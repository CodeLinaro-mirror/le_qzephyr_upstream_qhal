/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*==============================================================================

qurt_sem.c

GENERAL DESCRIPTION
    QuRT Shim Layer functions for Semaphore APIs

EXTERNAL FUNCTIONS
    qurt_signal_init
    qurt_signal_destroy
    qurt_signal_wait
    qurt_signal_wait_timed
    qurt_signal_set
    qurt_signal_get
    qurt_signal_clear

INITIALIZATION AND SEQUENCING REQUIREMENTS
    None.

==============================================================================*/

#include <zephyr/kernel.h>
#include <qurt_error.h>
#include <qurt_clock.h>
#include <qurt_signal.h>
#include "qurt_error.h"

void qurt_signal_init(qurt_signal_t *signal)
{
    struct k_event *event = (struct k_event *)signal;
    k_event_init(event);
}

void qurt_signal_destroy(qurt_signal_t *signal)
{
    struct k_event *event = (struct k_event *)signal;
    k_event_clear(event, UINT32_MAX);
    k_free(event);
}

static inline int qurt_signal_wait_impl(qurt_signal_t *signal, unsigned int mask, unsigned int attribute,
                                        unsigned int *out_signals, k_timeout_t timeout)
{
    struct k_event *event = (struct k_event *)signal;

    if (mask == 0) {
        *out_signals = 0;
        return 0;
    }

    uint32_t signals_received;
    const bool auto_reset = false;
    if (attribute & QURT_SIGNAL_ATTR_WAIT_ALL) {
        signals_received = k_event_wait_all(event, mask, auto_reset, timeout);
    } else {
        signals_received = k_event_wait(event, mask, auto_reset, timeout);
    }

    // if signals_received == 0 => timedout
    // if signals_received != 0 => signal mask is received
    if (signals_received == 0) {
        return -ETIMEDOUT;
    } else {
        if (attribute & QURT_SIGNAL_ATTR_CLEAR_MASK) {
            k_event_clear(event, signals_received);
        }
        *out_signals = signals_received;
        return 0;
    }
}

unsigned int qurt_signal_wait(qurt_signal_t *signal, unsigned int mask, unsigned int attribute)
{
    unsigned int out_signal = 0;
    int ret_val = qurt_signal_wait_impl(signal, mask, attribute, &out_signal, K_FOREVER);
    ARG_UNUSED(ret_val);
    return out_signal;
}

int qurt_signal_wait_timed(qurt_signal_t *signal, unsigned int mask, unsigned int attribute, unsigned int *out_signals,
                           unsigned long long int duration_in_us)
{
    int ret_val = qurt_signal_wait_impl(signal, mask, attribute, out_signals, K_USEC(duration_in_us));
    switch (ret_val) {
    case 0: {
        return QURT_EOK;
    } break;
    case -EAGAIN:
    case -ETIMEDOUT: {
        return QURT_EFAILED_TIMEOUT;
    } break;
    case -EBUSY:
    default: {
        return QURT_EFATAL;
    } break;
    }
}

void qurt_signal_set(qurt_signal_t *signal, unsigned int mask)
{
    struct k_event *event = (struct k_event *)signal;

    k_event_post(event, mask);
}

unsigned int qurt_signal_get(qurt_signal_t *signal)
{
    struct k_event * event = (struct k_event *)signal;
    return event->events;
}

void qurt_signal_clear(qurt_signal_t *signal, unsigned int mask)
{
    struct k_event * event = (struct k_event *)signal;
    k_event_clear(event, mask);
}

unsigned int qurt_anysignal_set(qurt_anysignal_t *signal, unsigned int mask)
{
    unsigned int prev_signals = qurt_signal_get(signal);
    qurt_signal_set(signal, mask);
    return prev_signals;
}

unsigned int qurt_anysignal_clear(qurt_anysignal_t *signal, unsigned int mask)
{
    unsigned int prev_signals = qurt_signal_get(signal);
    qurt_signal_clear(signal, mask);
    return prev_signals;
}

int qurt_signal_create(qurt_signal_t **signal)
{
    struct k_event *event = k_calloc(1, sizeof(*event));
    if (!event) {
        return QURT_EMEM;
    }

    *signal = (qurt_signal_t *)event;
    qurt_signal_init(*signal);

    return QURT_EOK;
}
