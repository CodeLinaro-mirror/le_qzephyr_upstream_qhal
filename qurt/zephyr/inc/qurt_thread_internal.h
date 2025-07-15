/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#ifndef QURT_THREAD_INTERNAL_H
#define QURT_THREAD_INTERNAL_H

#include <zephyr/kernel.h>
#include <zephyr/kernel/thread.h>

typedef struct _qal_thread
{
    union
    {
        struct k_thread zthread;
        struct k_work_q z_work_q;
    };

    qurt_thread_t qurt_tid;
    k_tid_t zephyr_tid;

    qurt_thread_attr_t qurt_attr;

    struct _qal_thread *joiner;
    int joinee_exit_status;

    sys_snode_t node;
} qal_thread_t;

int _qal_work_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr);
qal_thread_t *_qal_get_thread(qurt_thread_t thread_id);
int _qal_thread_delete(qal_thread_t *thread_node);

#endif /* QURT_THREAD_INTERNAL_H */
