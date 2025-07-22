/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
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

#include <qurt_error.h>
#include <qurt_sclk.h>
#include <qurt_mutex.h>

static int qurt_mutex_lock_timed_impl(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    int ret = QURT_EOK;
    if (QURT_TIMER_IS_DURATION_VALID(duration_in_us) != QURT_EOK) {
        return QURT_EINVALID;
    }

    int retVal = k_mutex_lock(lock, K_USEC(duration_in_us));
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
    int ret_val = k_mutex_init(lock);
    ARG_UNUSED(ret_val);
}

void qurt_rmutex_destroy(qurt_mutex_t *lock) { lock->lock_count = 0; }

void qurt_rmutex_lock(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_lock(lock, K_FOREVER);
    ARG_UNUSED(ret_val);
}

void qurt_rmutex_unlock(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_unlock(lock);
    ARG_UNUSED(ret_val);
}

int qurt_rmutex_lock_timed(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    return qurt_mutex_lock_timed_impl(lock, duration_in_us);
}

void qurt_mutex_init(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_init(lock);
    ARG_UNUSED(ret_val);
}

void qurt_mutex_destroy(qurt_mutex_t *lock) { lock->lock_count = 0; }

void qurt_mutex_lock(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_lock(lock, K_FOREVER);
    ARG_UNUSED(ret_val);
}

int qurt_mutex_lock_timed(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    return qurt_mutex_lock_timed_impl(lock, duration_in_us);
}

void qurt_mutex_unlock(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_unlock(lock);
    ARG_UNUSED(ret_val);
}

void qurt_pimutex_init(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_init(lock);
    ARG_UNUSED(ret_val);
}

void qurt_pimutex_destroy(qurt_mutex_t *lock) { lock->lock_count = 0; }

void qurt_pimutex_lock(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_lock(lock, K_FOREVER);
    ARG_UNUSED(ret_val);
}

int qurt_pimutex_lock_timed(qurt_mutex_t *lock, unsigned long long int duration_in_us)
{
    return qurt_mutex_lock_timed_impl(lock, duration_in_us);
}

void qurt_pimutex_unlock(qurt_mutex_t *lock)
{
    int ret_val = k_mutex_unlock(lock);
    ARG_UNUSED(ret_val);
}

int qurt_mutex_create(qurt_mutex_t *lock)
{
    qurt_mutex_init(lock);
    return QURT_EOK;
}
