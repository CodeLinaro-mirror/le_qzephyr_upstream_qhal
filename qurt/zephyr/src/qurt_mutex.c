/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*==============================================================================

qurt_mutex.c

GENERAL DESCRIPTION
    QuRT Shim Layer functions for Mutex APIs

EXTERNAL FUNCTIONS
    qurt_rmutex_init
    qurt_rmutex_destroy
    qurt_rmutex_lock
    qurt_rmutex_unlock
    qurt_rmutex_lock_timed

    qurt_mutex_init
    qurt_mutex_destroy
    qurt_mutex_lock
    qurt_mutex_lock_timed
    qurt_mutex_unlock

    qurt_pimutex_init
    qurt_pimutex_destroy
    qurt_pimutex_lock
    qurt_pimutex_lock_timed
    qurt_pimutex_unlock

INITIALIZATION AND SEQUENCING REQUIREMENTS
    None.

==============================================================================*/

#include <zephyr/kernel.h>
#include <qurt_error.h>
#include <qurt_clock.h>
#include <qurt_mutex.h>
#include "qurt_error.h"

static int qurt_mutex_lock_timed_impl(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    int ret = QURT_EOK;
    struct k_mutex *mutex = (struct k_mutex *)lock;

    int retVal = k_mutex_lock(mutex, K_USEC(duration_in_us));
    switch (retVal) {
    case 0: {
        ret = QURT_EOK;
    } break;
    case -EAGAIN:
    case -ETIMEDOUT: {
        ret = QURT_EFAILED_TIMEOUT;
    } break;
    case -EBUSY:
    default: {
        ret = QURT_EFATAL;
    } break;
    }
    return ret;
}

void qurt_rmutex_init(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;

    k_mutex_init(mutex);
}

void qurt_rmutex_destroy(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    k_free(mutex);
}

void qurt_rmutex_lock(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    k_mutex_lock(mutex, K_FOREVER);
}

void qurt_rmutex_unlock(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    k_mutex_unlock(mutex);
}

int qurt_rmutex_lock_timed(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    return qurt_mutex_lock_timed_impl(lock, duration_in_us);
}

void qurt_mutex_init(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;

    k_mutex_init(mutex);
}

void qurt_mutex_destroy(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    k_free(mutex);
}

void qurt_mutex_lock(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;

    int ret_val = k_mutex_lock(mutex, K_FOREVER);
    ARG_UNUSED(ret_val);
}

int qurt_mutex_lock_timed(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    return qurt_mutex_lock_timed_impl(lock, duration_in_us);
}

void qurt_mutex_unlock(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    int ret_val = k_mutex_unlock(mutex);
    ARG_UNUSED(ret_val);
}

void qurt_pimutex_init(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    int ret_val = k_mutex_init(mutex);
    ARG_UNUSED(ret_val);
}

void qurt_pimutex_destroy(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    mutex->lock_count = 0;
}

void qurt_pimutex_lock(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    int ret_val = k_mutex_lock(mutex, K_FOREVER);
    ARG_UNUSED(ret_val);
}

int qurt_pimutex_lock_timed(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    return qurt_mutex_lock_timed_impl(lock, duration_in_us);
}

void qurt_pimutex_unlock(qurt_mutex_t *lock)
{
    struct k_mutex *mutex = (struct k_mutex *)lock;
    int ret_val = k_mutex_unlock(mutex);
    ARG_UNUSED(ret_val);
}

int qurt_mutex_create(qurt_mutex_t **lock)
{
    struct k_mutex *mutex = k_calloc(sizeof(*mutex), 1);
    if (!mutex) {
        return QURT_EMEM;
    }

    *lock = (qurt_mutex_t *)mutex;
    qurt_mutex_init(*lock);

    return QURT_EOK;
}
