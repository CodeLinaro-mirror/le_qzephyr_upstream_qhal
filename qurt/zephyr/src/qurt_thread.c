/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*==============================================================================

qurt_thread.c

GENERAL DESCRIPTION
    QuRT Shim Layer functions for Thread APIs

INITIALIZATION AND SEQUENCING REQUIREMENTS
    None.

==============================================================================*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <zephyr/kernel.h>
#include <zephyr/init.h>

#include <qurt_error.h>
#include <qurt_sclk.h>
#include <qurt_thread.h>

#include "qurt_thread_internal.h"

static sys_slist_t qal_thread_slist;
static struct k_spinlock qal_thread_slist_lock;

static sys_slist_t qal_freethread_slist;
static struct k_spinlock qal_freethread_slist_lock;

// TODO: Add limit check for uint32_t, since atomic_t can be 64bit
static atomic_t g_thread_num = 1;

// mock structure to for joining from non qurt thread
static qal_thread_t g_mock_thread;

static int qal_thread_init()
{
    sys_slist_init(&qal_thread_slist);
    sys_slist_init(&qal_freethread_slist);
    return QURT_EOK;
}

static inline qal_thread_t *_qal_get_current_thread()
{
    k_spinlock_key_t key;
    qal_thread_t *thread_node;
    k_tid_t zephyr_tid = k_current_get();
    key = k_spin_lock(&qal_thread_slist_lock);
    SYS_SLIST_FOR_EACH_CONTAINER(&qal_thread_slist, thread_node, node)
    {
        if (thread_node->zephyr_tid == zephyr_tid) {
            k_spin_unlock(&qal_thread_slist_lock, key);
            return thread_node;
        }
    }
    k_spin_unlock(&qal_thread_slist_lock, key);
    return NULL;
}

static inline qal_thread_t *_qal_get_thread_by_name(char *name)
{
    k_spinlock_key_t key;
    qal_thread_t *thread_node;
    key = k_spin_lock(&qal_thread_slist_lock);
    SYS_SLIST_FOR_EACH_CONTAINER(&qal_thread_slist, thread_node, node)
    {
        if (strncmp(thread_node->qurt_attr.name, name, QURT_THREAD_ATTR_NAME_MAXLEN) == 0) {
            k_spin_unlock(&qal_thread_slist_lock, key);
            return thread_node;
        }
    }

    k_spin_unlock(&qal_thread_slist_lock, key);
    return NULL;
}

qal_thread_t *_qal_get_thread(qurt_thread_t thread_id)
{
    k_spinlock_key_t key;
    qal_thread_t *thread_node;
    key = k_spin_lock(&qal_thread_slist_lock);
    SYS_SLIST_FOR_EACH_CONTAINER(&qal_thread_slist, thread_node, node)
    {
        if (thread_node->qurt_tid == thread_id) {
            k_spin_unlock(&qal_thread_slist_lock, key);
            return thread_node;
        }
    }
    k_spin_unlock(&qal_thread_slist_lock, key);
    return NULL;
}

void _qal_free_stale_tcbs()
{
    sys_snode_t *node;

    k_spinlock_key_t key;
    key = k_spin_lock(&qal_freethread_slist_lock);
    while ((node = sys_slist_get(&qal_freethread_slist))) {
        qal_thread_t *thread_node = CONTAINER_OF(node, qal_thread_t, node);
        k_free(thread_node->qurt_attr.stack_addr);
        k_free(thread_node);
    }

    k_spin_unlock(&qal_freethread_slist_lock, key);
}

qurt_thread_t qurt_thread_get_id(void)
{
    qal_thread_t *thread_node = _qal_get_current_thread();
    if (thread_node != 0) {
        return thread_node->qurt_tid;
    } else {
        return (qurt_thread_t)QURT_EOK;
    }
}

int qurt_thread_get_thread_id(qurt_thread_t *thread_id, char *name)
{
    qal_thread_t *thread_node = _qal_get_thread_by_name(name);
    if (thread_node != 0) {
        *thread_id = thread_node->qurt_tid;
        return 0;
    } else {
        return QURT_EINVALID;
    }
}

int qurt_thread_attr_get(qurt_thread_t thread_id, qurt_thread_attr_t *attr)
{
    qal_thread_t *thread_node = _qal_get_thread(thread_id);
    if (thread_node != NULL) {
        *attr = thread_node->qurt_attr;
        return QURT_EOK;
    } else {
        return QURT_EINVALID;
    }
}

void qurt_thread_exit(int status)
{
    k_tid_t zephyr_tid = NULL;
    qal_thread_t *thread_node = _qal_get_current_thread();
    if (thread_node) {
        zephyr_tid = thread_node->zephyr_tid;
        if (thread_node->joiner) {
            thread_node->joiner->joinee_exit_status = status;
        }
        _qal_thread_delete(thread_node);
    } else {
        zephyr_tid = k_current_get();
    }

    k_thread_abort(zephyr_tid);
}

void qurt_thread_get_name(char *name, unsigned char max_len)
{
#ifdef CONFIG_THREAD_NAME
    int ret_val;
    k_tid_t kthread;
    unsigned char len;
    len = max_len < QURT_THREAD_ATTR_NAME_MAXLEN ? max_len : QURT_THREAD_ATTR_NAME_MAXLEN;

    if (name == NULL) {
        return;
    }

    kthread = k_current_get();
    ret_val = k_thread_name_copy(kthread, name, len - 1);
    ARG_UNUSED(ret_val);
#else
    ARG_UNUSED(name);
    ARG_UNUSED(max_len);
#endif
}

static void qal_thread_wrapper(void *arg1, void *arg2, void *arg3)
{
    void *(*fun_ptr)(void *) = arg3;

    fun_ptr(arg1);
    qurt_thread_exit(0);
}

static inline bool _qal_isvalid_priority(int priority)
{
    if ((priority > QURT_THREAD_ATTR_PRIORITY_MAX) || (priority < QURT_THREAD_ATTR_PRIORITY_MIN)) {
        return false;
    } else {
        return true;
    }
}

int qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void (*entrypoint)(void *), void *arg)
{
    int ret_val = QURT_EOK;
    k_tid_t zephyr_tid;
    qal_thread_t *newthread;
    k_spinlock_key_t key;
    void *stack_addr = NULL;

    _qal_free_stale_tcbs();

    if (!_qal_isvalid_priority(attr->priority)) {
        return QURT_EINVALID;
    }

    newthread = k_malloc(sizeof(qal_thread_t));
    if (newthread == NULL) {
        return QURT_EFAILED;
    }

    // K_KERNEL_STACK_DEFINE only can declare stack as global array
    // stack_addr should be 8-bytes alligned, better to be 16 bytes alligned
    stack_addr = k_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, attr->stack_size);
    if (!stack_addr) {
        ret_val = QURT_EMEM;
        goto exit_free_thread;
    }
    attr->stack_addr = stack_addr;

    zephyr_tid =
        k_thread_create(&newthread->zthread, attr->stack_addr, attr->stack_size, (k_thread_entry_t)qal_thread_wrapper,
                        (void *)arg, NULL, entrypoint, attr->priority, 0, K_FOREVER);
    ret_val = k_thread_name_set(zephyr_tid, attr->name);

    newthread->zephyr_tid = zephyr_tid;
    newthread->qurt_tid = atomic_inc(&g_thread_num);
    newthread->qurt_attr = *attr;
    newthread->joiner = NULL;
    *thread_id = newthread->qurt_tid;
    key = k_spin_lock(&qal_thread_slist_lock);
    sys_slist_append(&qal_thread_slist, &newthread->node);
    k_spin_unlock(&qal_thread_slist_lock, key);

    k_thread_start(zephyr_tid);
    return ret_val;

exit_free_thread:
    k_free(newthread);
    return ret_val;
}

int _qal_work_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr)
{
    k_spinlock_key_t key;
    struct k_work_queue_config cfg = {.name = attr->name};
    void *stack_addr = NULL;
    int ret_val = QURT_EFAILED;

    if (!_qal_isvalid_priority(attr->priority)) {
        return QURT_EINVALID;
    }

    qal_thread_t *this_work_thread = k_malloc(sizeof(qal_thread_t));
    if (this_work_thread == NULL) {
        return QURT_EMEM;
    }

    stack_addr = k_aligned_alloc(Z_KERNEL_STACK_OBJ_ALIGN, attr->stack_size);
    if (!stack_addr) {
        ret_val = QURT_EMEM;
        goto exit_free_thread;
    }
    attr->stack_addr = stack_addr;

    memset(this_work_thread, 0x00, sizeof(qal_thread_t));

    k_work_queue_init(&this_work_thread->z_work_q);
    k_work_queue_start(&this_work_thread->z_work_q, attr->stack_addr, attr->stack_size, attr->priority, &cfg);

    this_work_thread->zephyr_tid = k_work_queue_thread_get(&this_work_thread->z_work_q);
    this_work_thread->qurt_tid = atomic_inc(&g_thread_num);
    this_work_thread->qurt_attr = *attr;
    *thread_id = this_work_thread->qurt_tid;
    key = k_spin_lock(&qal_thread_slist_lock);
    sys_slist_append(&qal_thread_slist, &this_work_thread->node);
    k_spin_unlock(&qal_thread_slist_lock, key);

    return this_work_thread->qurt_tid;

exit_free_thread:
    k_free(this_work_thread);
    return ret_val;
}

// Before this call, task should call qurt_thread_exit by self to put thread to freeq
int qurt_thread_join(unsigned int tid, int *status)
{
    int retval = -1;
    qal_thread_t *joinee = _qal_get_thread(tid);
    qal_thread_t *joiner = _qal_get_current_thread();

    if (joinee != NULL) {
        joiner = joiner ? joiner : &g_mock_thread;
        joinee->joiner = joiner;

        int k_ret_val = k_thread_join(joinee->zephyr_tid, K_FOREVER);
        if (status != 0) {
            *status = joiner->joinee_exit_status;
        }

        ARG_UNUSED(k_ret_val);
        retval = 0;
    } else {
        retval = QURT_ENOTHREAD;
    }

    _qal_free_stale_tcbs();

    return retval;
}

int _qal_thread_delete(qal_thread_t *thread_node)
{
    int retval = -1;
    bool node_removed = false;

    if (thread_node != NULL) {
        // remove node from qurt thread list
        k_spinlock_key_t key = k_spin_lock(&qal_thread_slist_lock);
        node_removed = sys_slist_find_and_remove(&qal_thread_slist, &thread_node->node);
        k_spin_unlock(&qal_thread_slist_lock, key);

        if (node_removed) {
            // check if removed thread node is current running thread or not
            k_tid_t current_ztid = k_current_get();
            if (thread_node->zephyr_tid == current_ztid) {
                // add the removed node into free thread list
                k_spinlock_key_t freer_key;
                freer_key = k_spin_lock(&qal_freethread_slist_lock);
                sys_slist_append(&qal_freethread_slist, &thread_node->node);
                k_spin_unlock(&qal_freethread_slist_lock, freer_key);
            } else {
                // since the thread_node is not the current running thread
                // this can be freed immediately
                k_thread_abort(thread_node->zephyr_tid);
                k_free(thread_node->qurt_attr.stack_addr);
                k_free(thread_node);
            }

            retval = 0;
        }
    }

    return retval;
}

void qurt_thread_sleep(TickType_t sleep_time) { k_sleep(K_TICKS(sleep_time)); }

int qurt_thread_get_priority(qurt_thread_t qurt_tid)
{
    qal_thread_t *thread_node = _qal_get_thread(qurt_tid);
    if (thread_node != NULL) {
        return thread_node->qurt_attr.priority;
    } else {
        return QURT_EFATAL;
    }
}

int qurt_thread_set_priority(qurt_thread_t qurt_tid, unsigned short newprio)
{
    if (!_qal_isvalid_priority(newprio)) {
        return QURT_EFATAL;
    }

    qal_thread_t *thread_node = _qal_get_thread(qurt_tid);
    if (thread_node != NULL) {
        thread_node->qurt_attr.priority = newprio;
        k_thread_priority_set(thread_node->zephyr_tid, newprio);
        return QURT_EOK;
    } else {
        return QURT_EFATAL;
    }
}

BaseType_t nt_qurt_thread_create(
    TaskFunction_t pxTaskCode,
    const char *const pcName, /*lint !e971 Unqualified char types are allowed for strings and single characters only. */
    const configSTACK_DEPTH_TYPE usStackDepth, void *const pvParameters, UBaseType_t uxPriority,
    TaskHandle_t *const pxCreatedTask)
{
    qurt_thread_t *handle = (qurt_thread_t *)pxCreatedTask;
    qurt_thread_attr_t thread_attr = {0};

    qurt_thread_attr_init(&thread_attr);
    qurt_thread_attr_set_name(&thread_attr, (char *)pcName);
    qurt_thread_attr_set_priority(&thread_attr, uxPriority);
    qurt_thread_attr_set_stack_size(&thread_attr, usStackDepth);

    return qurt_thread_create(handle, &thread_attr, pxTaskCode, pvParameters);
}

TaskHandle_t nt_qurt_thread_get_id(void) { return qurt_thread_get_id(); }

SYS_INIT(qal_thread_init, POST_KERNEL, 10);
