/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*==============================================================================

qurt_isr.c

GENERAL DESCRIPTION
    QuRT Shim Layer functions for ISR APIs

INITIALIZATION AND SEQUENCING REQUIREMENTS
    None.

==============================================================================*/

#include <stdlib.h>
#include <qurt_isr.h>
#include <qurt_thread.h>
#include <qurt_error.h>

#include "qurt_thread_internal.h"

typedef struct _qal_isr {
    struct k_work_q *z_work_q;
    void (*isr)(void *, int);
    void *arg;
    struct k_work work;

    int int_num;

    // This flag is for restoring the interrupt enabled / disabled status
    //  in the work function
    // If the client disables the interrupt using qurt_interrupt_disable,
    //  then restore_interrupt will be false, and we don't enable the interrupt
    //  after returning back from the client callback
    bool restore_interrupt;
} qal_isr_t;

#ifdef IRQ_TABLE_SIZE
#define QAL_IRQ_TABLE_SIZE IRQ_TABLE_SIZE
#elif defined(CONFIG_MAX_IRQ_LINES)
#define QAL_IRQ_TABLE_SIZE CONFIG_MAX_IRQ_LINES
#else
#error Interrupts not supported for given board
#endif

#ifndef CONFIG_GEN_IRQ_START_VECTOR
#define CONFIG_GEN_IRQ_START_VECTOR 0
#endif

static qal_isr_t qal_isr_table[QAL_IRQ_TABLE_SIZE];

// Following code reference till _get_qal_index
//  is taken from zephyr/arch/common/sw_isr_common.c

/**
 * @brief helper function to get effective index in qal_isr_table for a given
 *        interrupt number
 *
 * @param int_num interrupt number passed to QuRT APIs
 * @return value >= 0 table offset
 *         value < 0 invalid interrupt
 */
#ifndef CONFIG_ZTEST
static inline
#endif
    int
    _get_qal_index(unsigned int irq)
{
    int table_idx;

#ifdef CONFIG_2ND_LEVEL_INTERRUPTS
    unsigned int level = irq_get_level(irq);
    if (level == 2U) {
        unsigned int parent_irq = irq_parent_level_2(irq);
        if (parent_irq == CONFIG_2ND_LVL_INTR_00_OFFSET) {
            table_idx = CONFIG_2ND_LVL_ISR_TBL_OFFSET + irq_from_level_2(irq);
        } else {
            table_idx = -1;
        }
    }
#ifdef CONFIG_3RD_LEVEL_INTERRUPTS
#error QuRT ISR does not support 3rd level interrupts
#endif /* CONFIG_3RD_LEVEL_INTERRUPTS */
    else {
        table_idx = irq;
    }

    table_idx -= CONFIG_GEN_IRQ_START_VECTOR;
#else
    table_idx = irq - CONFIG_GEN_IRQ_START_VECTOR;
#endif /* CONFIG_MULTI_LEVEL_INTERRUPTS */

    return table_idx;
}

static void _qal_isr_handler(const void *arg)
{
    qal_isr_t *this_isr = (qal_isr_t *)arg;
    if (this_isr->z_work_q != NULL) {
        // disable this interrupt, otherwise this interrupt handler will be
        // invoked after exiting from interrupt handling and no other threads
        // would be run
        // this interrupt can be enabled after the work thread is scheduled
        irq_disable(this_isr->int_num);
        k_work_submit_to_queue(this_isr->z_work_q, &this_isr->work);
    }
}

void _qal_isr_work(struct k_work *item)
{
    qal_isr_t *this_isr = CONTAINER_OF(item, qal_isr_t, work);
    if (this_isr->isr != NULL) {
        this_isr->isr(this_isr->arg, this_isr->int_num);
    }

    if (this_isr->restore_interrupt) {
        // enable the interrupt back which was disabled in the isr handler
        irq_enable(this_isr->int_num);
    }
}

int qurt_isr_create(qurt_thread_t *thread_id, qurt_thread_attr_t *pAttr)
{
    _qal_work_thread_create(thread_id, pAttr);
    return QURT_EOK;
}

int qurt_isr_register2(qurt_thread_t isr_thread_id, int int_num, unsigned short prio, unsigned short flags,
                       unsigned int int_type, void (*isr)(void *, int), void *arg)
{
    int qal_index;

    if (isr == NULL) {
        return QURT_EINT;
    }

    qal_index = _get_qal_index(int_num);
    if ((qal_index < 0) || (qal_index >= QAL_IRQ_TABLE_SIZE)) {
        return QURT_EINT;
    }

    qal_thread_t *work_thread = _qal_get_thread(isr_thread_id);
    if (work_thread == NULL) {
        return QURT_EINVALID;
    }

    qal_isr_t *qal_isr = &qal_isr_table[qal_index];

    if (qal_isr->isr != NULL) {
        return QURT_EDUPLICATE;
    }

    qal_isr->isr = isr;
    qal_isr->arg = arg;
    qal_isr->int_num = int_num;

    k_work_init(&qal_isr->work, _qal_isr_work);

    qal_isr->z_work_q = &work_thread->z_work_q;
    irq_connect_dynamic(int_num, prio, _qal_isr_handler, qal_isr, flags);
    irq_enable(int_num);
    qal_isr->restore_interrupt = true;

    return QURT_EOK;
}

int qurt_isr_deregister2(int int_num)
{
    int qal_index;

    qal_index = _get_qal_index(int_num);
    if ((qal_index < 0) || (qal_index >= QAL_IRQ_TABLE_SIZE)) {
        return QURT_EINT;
    }

    qal_isr_t *qal_isr = &qal_isr_table[qal_index];

    qal_isr->isr = NULL;
    qal_isr->arg = NULL;
    qal_isr->z_work_q = NULL;
    qal_isr->restore_interrupt = false;

    irq_disable(int_num);

    return QURT_EOK;
}

int qurt_isr_delete(qurt_thread_t isr_tid)
{
    qal_thread_t *work_thread = _qal_get_thread(isr_tid);
    return _qal_thread_delete(work_thread);
}

unsigned int qurt_interrupt_disable(int int_num)
{
    int qal_index;

    qal_index = _get_qal_index(int_num);
    if ((qal_index < 0) || (qal_index >= QAL_IRQ_TABLE_SIZE)) {
        return QURT_EINT;
    }

    qal_isr_table[qal_index].restore_interrupt = false;
    irq_disable(int_num);
    return QURT_EOK;
}

unsigned int qurt_interrupt_enable(int int_num)
{
    int qal_index;

    qal_index = _get_qal_index(int_num);
    if ((qal_index < 0) || (qal_index >= QAL_IRQ_TABLE_SIZE)) {
        return QURT_EINT;
    }

    qal_isr_table[qal_index].restore_interrupt = true;
    irq_enable(int_num);
    return QURT_EOK;
}
