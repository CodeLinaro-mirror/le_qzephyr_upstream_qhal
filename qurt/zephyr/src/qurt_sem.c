/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*==============================================================================

qurt_sem.c

GENERAL DESCRIPTION
    QuRT Shim Layer functions for Semaphore APIs

EXTERNAL FUNCTIONS
    qurt_sem_init
    qurt_sem_init_val
    qurt_sem_up
    qurt_sem_down
    qurt_sem_try_down
    qurt_sem_destroy
    qurt_sem_get_val
    qurt_sem_down_timed

INITIALIZATION AND SEQUENCING REQUIREMENTS
    None.

==============================================================================*/

#include <assert.h>

#include <qurt_error.h>
#include <qurt_sclk.h>
#include <qurt_sem.h>

void qurt_sem_create(qurt_sem_t **sem)
{
    qurt_sem_t *qsem = NULL;

    if (!sem) {
        return;
    }

    qsem = k_calloc(sizeof(qurt_sem_t), 1);
    if (!qsem) {
        return;
    }

    k_sem_init(qsem, 0, 1);
    k_sem_give(qsem);
    *sem = qsem;
}

int qurt_sem_up(qurt_sem_t *sem)
{
    k_sem_give(sem);
    return 0;
}

int qurt_sem_down(qurt_sem_t *sem)
{
    return k_sem_take(sem, K_FOREVER);
}

int qurt_sem_try_down(qurt_sem_t *sem)
{
    int ret = k_sem_take(sem, K_NO_WAIT);
    if (ret == -EBUSY)
    {
        // setting same return value as QuRT
        ret = QURT_EFATAL;
    }

    return ret;
}

void qurt_sem_destroy(qurt_sem_t *sem)
{
    k_sem_reset(sem);
    k_free(sem);
}

unsigned int qurt_sem_get_val(qurt_sem_t *sem)
{
    return k_sem_count_get(sem);
}

int qurt_sem_down_timed(qurt_sem_t *sem, TickType_t block_time)
{
    int ret = QURT_EOK;
    int ret_val = k_sem_take(sem, K_TICKS(block_time));
    switch (ret_val)
    {
    case 0:
    {
        ret = QURT_EOK;
    }
    break;
    case -EAGAIN:
    case -ETIMEDOUT:
    {
        ret = QURT_EFAILED_TIMEOUT;
    }
    break;
    case -EBUSY:
    default:
    {
        ret = QURT_EFATAL;
    }
    break;
    }
    return ret;
}
