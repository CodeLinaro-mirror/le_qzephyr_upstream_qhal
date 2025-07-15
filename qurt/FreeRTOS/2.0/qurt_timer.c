/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * qurt_timer.c
 *
 *  Created on: May 12, 2020
 *      Author: Abhilash
 */
#include <stdlib.h>
#include <string.h>
#include "qurt_ext.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "nt_osal.h" //added for osal calls
/*=========================================================
                 EXTERNAL
 ==========================================================*/
extern void qurt_signal_set(qurt_signal_t *signal, uint32 mask);

/*=========================================================
                 MACROS
 ==========================================================*/
#define QURT_TIMER_NOTIFY_TYPE_CALLBACK 1
#define QURT_TIMER_NOTIFY_TYPE_SIGNAL 2
#define QURT_SYSTICK_CLOCK_HZ configTICK_RATE_HZ

/*=========================================================
                TYPEDEFS
 ==========================================================*/
// _qurt_timer_sig_info_t types
typedef struct qurt_timer_sig_info_t
{
    qurt_signal_t *sig;
    uint32 mask;
} _qurt_timer_sig_info_t;

// _qurt_timer_cb_info_t types
typedef struct qurt_timer_cb_info_t
{
    TimerCallbackFunction_t cbfunc;
    void *cbctxt;
} _qurt_timer_cb_info_t;

// timer_notify_info_t types
typedef struct timer_notify_info_t
{
    uint32 timer_notify_type;
    union
    {
        _qurt_timer_sig_info_t sig_obj;
        _qurt_timer_cb_info_t cb_obj;
    } notify_option;
} _timer_notify_info_t;

// qurt_timer_attr_t types
typedef struct _qurt_timer_attr_t
{
    qurt_time_t duration;
    qurt_time_t reload;
    qurt_time_t remaining;
    qurt_time_t start;
    uint32 option;
    uint32 expire_count;
    _timer_notify_info_t notify_info;
} _qurt_timer_attr_t;

#ifdef QURT_TIMER_T_TODO
// qurt internal timer object
typedef struct _qurt_timer_t
{
    TimerHandle_t free_rtos_timer_info;
    _qurt_timer_attr_t qurt_timer_info;
} _qurt_timer_t;
#endif

/*=============================================================
                 FUNCTIONS
 ==============================================================*/

void qurt_timer_cb(TimerHandle_t free_rtos_timer_info)
{
    _qurt_timer_attr_t *qurt_timer_info;
    //TODO: wrong code logic, but fortunetaly not used yet
    qurt_timer_info = (_qurt_timer_attr_t *)pvTimerGetTimerID(free_rtos_timer_info);
    if (NULL == qurt_timer_info)
    {
        return;
    }
    // change the period for subsequent expiry
    if (qurt_timer_info->expire_count == 0 && (qurt_timer_info->option & QURT_TIMER_PERIODIC))
    {
        if (pdPASS != xTimerChangePeriod(free_rtos_timer_info, qurt_timer_info->reload, portMAX_DELAY))
        {
            ASSERT(0);
        }
    }
    qurt_timer_info->expire_count++;
    if (QURT_TIMER_NOTIFY_TYPE_CALLBACK == qurt_timer_info->notify_info.timer_notify_type)
    {
        qurt_timer_info->notify_info.notify_option.cb_obj.cbfunc(qurt_timer_info->notify_info.notify_option.cb_obj.cbctxt);
    }
    else if (QURT_TIMER_NOTIFY_TYPE_SIGNAL == qurt_timer_info->notify_info.timer_notify_type)
    {
        qurt_signal_set(qurt_timer_info->notify_info.notify_option.sig_obj.sig, qurt_timer_info->notify_info.notify_option.sig_obj.mask);
    }
    else
    {
        return;
    }
}

void qurt_timer_attr_init(qurt_timer_attr_t *attr)
{
    memset(attr, 0, QURT_TIMER_OBJ_SIZE_BYTES);
}

void qurt_timer_attr_set_duration(qurt_timer_attr_t *attr, qurt_time_t duration)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    pattr->duration = duration;
}

void qurt_timer_attr_set_reload(qurt_timer_attr_t *attr, qurt_time_t reload_time)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    pattr->reload = reload_time;
}

void qurt_timer_attr_set_callback(qurt_timer_attr_t *attr, TimerCallbackFunction_t cbfunc, void *cbctxt)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    pattr->notify_info.timer_notify_type = QURT_TIMER_NOTIFY_TYPE_CALLBACK;
    pattr->notify_info.notify_option.cb_obj.cbfunc = cbfunc;
    pattr->notify_info.notify_option.cb_obj.cbctxt = cbctxt;
}

void qurt_timer_attr_set_signal(qurt_timer_attr_t *attr, qurt_signal_t *signal, uint32 mask)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    pattr->notify_info.timer_notify_type = QURT_TIMER_NOTIFY_TYPE_SIGNAL;
    pattr->notify_info.notify_option.sig_obj.sig = signal;
    pattr->notify_info.notify_option.sig_obj.mask = mask;
}

void qurt_timer_attr_set_option(qurt_timer_attr_t *attr, uint32 option)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    pattr->option = option;
}

TimerHandle_t qurt_timer_create(void *id, const qurt_timer_attr_t *attr, TimerCallbackFunction_t pxCallbackFunction)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;

    UBaseType_t reload;
    if (NULL == pattr)
    {
        return NULL;
    }
    if (pattr->option & QURT_TIMER_ONESHOT)
    {
        // if one shot timer and reload provided, return error
        if (pattr->reload)
        {
            return NULL;
        }
        reload = pdFALSE;
    }
    else if (pattr->option & QURT_TIMER_PERIODIC)
    {
        // IF PERIODIC TIMER AND ZERO RELOAD PROVIDED, RETURN ERROR
        if (!pattr->reload)
        {
            return NULL;
        }
        reload = pdTRUE;
    }
    else
    {
        reload = pdFALSE;
    }

    return (xTimerCreate("qurt_timer",
                         pattr->duration, reload,
                         id,
                         pxCallbackFunction));
}

int qurt_timer_start(TimerHandle_t timer, TickType_t block_time)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerStart(timer, block_time))
    {
        return QURT_EFAILED;
    }
    return QURT_EOK;
}

int qurt_timer_start_frm_isr(TimerHandle_t timer, long *const port_yield)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerStartFromISR(timer, port_yield))
    {
        return QURT_EFAILED;
    }
    return QURT_EOK;
}

/**
 * @brief    Function to stop NT timer from ISR
 *
 * @param[in]           timer       Timer handler
 * @param[in, out]      port_yield  Yield to high priority task
 */
int qurt_timer_stop_frm_isr(TimerHandle_t timer, long *const port_yield)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerStopFromISR(timer, port_yield))
    {
        return QURT_EFAILED;
    }
    return QURT_EOK;
}

int qurt_get_expiry_time(TimerHandle_t timer)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }
    return xTimerGetExpiryTime(timer);
}

int qurt_get_time_period(TimerHandle_t timer)
{
    if (NULL == timer)
    {
        return QURT_EINVALID;
    }
    return xTimerGetPeriod(timer);
}

int qurt_timer_Is_Active(TimerHandle_t timer)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerIsTimerActive(timer))
    {
        return QURT_EFAILED;
    }
    return QURT_EOK;
}

int qurt_timer_stop(TimerHandle_t timer, TickType_t block_time)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerStop(timer, block_time))
    {
        return QURT_EFAILED;
    }

    return QURT_EOK;
}

int qurt_timer_change_period(TimerHandle_t timer, TickType_t timer_period, TickType_t block_time)
{

    if (NULL == timer)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerChangePeriod(timer, timer_period, block_time))
    {
        return QURT_EFAILED;
    }

    return QURT_EOK;
}

#ifdef QURT_TIMER_T_TODO
int qurt_timer_restart(qurt_timer_t timer, qurt_time_t duration)
{
    _qurt_timer_t *qurt_timer_internal = (_qurt_timer_t *)timer;

    if (NULL == qurt_timer_internal)
    {
        return QURT_EINVALID;
    }

    if (pdPASS != xTimerChangePeriod(qurt_timer_internal->free_rtos_timer_info, duration, portMAX_DELAY))
    {
        return QURT_EFAILED;
    }
    qurt_timer_internal->qurt_timer_info.remaining = duration;
    qurt_timer_internal->qurt_timer_info.start = xTaskGetTickCount();
    return QURT_EOK;
}

qurt_time_t qurt_timer_remaining(qurt_timer_t timer)
{
    _qurt_timer_t *qurt_timer_internal = (_qurt_timer_t *)timer;
    TickType_t qurt_timer_initial_duration = 0;
    TickType_t qurt_timer_start_time = 0;
    TickType_t qurt_time_now = 0;
    TickType_t qurt_remaining_time = 0;
    TickType_t qurt_expiry_time = 0;

    qurt_timer_initial_duration = qurt_timer_internal->qurt_timer_info.duration;
    qurt_timer_start_time = qurt_timer_internal->qurt_timer_info.start;
    qurt_expiry_time = qurt_timer_initial_duration + qurt_timer_start_time;
    qurt_time_now = xTaskGetTickCount();

    /* Timer has not been activated yet, stopped or set for portMAX_DELAY */
    if (0 == qurt_timer_start_time || portMAX_DELAY == qurt_timer_initial_duration)
    {
        return qurt_timer_initial_duration;
    }

    /* Current time has exceeded the expiry time, timer would have expired or expiry would have overflowed*/
    if (qurt_expiry_time <= qurt_time_now)
    {
        /* Has the expiry time elapsed between the command to start/reset a timer was issued, and the time the command was processed? */
        if ((qurt_time_now - qurt_timer_start_time) >= qurt_timer_initial_duration)
        {
            /* The time between a command being issued and the command being processed actually exceeds the timers period. */
            qurt_remaining_time = 0;
        }
        else
        {
            /* qurt_remaining_time has become less then initial duration, will happen when expiry time has overflowed */
            qurt_remaining_time = portMAX_DELAY - qurt_time_now;
            qurt_remaining_time = qurt_remaining_time + qurt_expiry_time;
        }
    }
    else
    {
        /* Current time is less then expiry time, timer has not expired or tick count has overflowed */
        if ((qurt_time_now < qurt_timer_start_time) && (qurt_expiry_time >= qurt_timer_start_time))
        {
            /* If, since the command was issued, the tick count has overflowed but the expiry time has not, then the timer
            must have already passed its expiry time and should be processed immediately. */
            qurt_remaining_time = 0;
        }
        /* No overflow for current time or expiry time, normal case */
        else
        {
            qurt_remaining_time = qurt_expiry_time - qurt_time_now;
        }
    }
    return (qurt_time_t)qurt_remaining_time;
}

int qurt_timer_get_attr(qurt_timer_t timer, qurt_timer_attr_t *attr)
{
    _qurt_timer_t *qurt_timer_internal = (_qurt_timer_t *)timer;
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == qurt_timer_internal || NULL == pattr)
    {
        return QURT_EINVALID;
    }
    pattr->duration = qurt_timer_internal->qurt_timer_info.duration;
    pattr->notify_info = qurt_timer_internal->qurt_timer_info.notify_info;
    pattr->option = qurt_timer_internal->qurt_timer_info.option;
    pattr->reload = qurt_timer_internal->qurt_timer_info.reload;
    pattr->remaining = qurt_timer_remaining(timer);
    return QURT_EOK;
}
#endif

int qurt_timer_attr_get_duration(qurt_timer_attr_t *attr, qurt_time_t *duration)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == duration)
    {
        return QURT_EINVALID;
    }
    *duration = pattr->duration;
    return QURT_EOK;
}

int qurt_timer_attr_get_option(qurt_timer_attr_t *attr, uint32 *option)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == option)
    {
        return QURT_EINVALID;
    }
    *option = pattr->option;
    return QURT_EOK;
}

int qurt_timer_attr_get_remaining(qurt_timer_attr_t *attr, qurt_time_t *remaining)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == remaining)
    {
        return QURT_EINVALID;
    }
    *remaining = pattr->remaining;
    return QURT_EOK;
}

int qurt_timer_attr_get_reload(qurt_timer_attr_t *attr, qurt_time_t *reload_time)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == reload_time)
    {
        return QURT_EINVALID;
    }
    *reload_time = pattr->reload;
    return QURT_EOK;
}

int qurt_timer_delete(TimerHandle_t timer, TickType_t block_time)
{

    BaseType_t status;
    if (NULL == timer)
    {
        return QURT_EINVALID;
    }
    // FreeRTOS internally does not stop timer before deleting it
    status = xTimerStop(timer, block_time);
    if (pdPASS == status)
    {
        status = xTimerDelete(timer, block_time);
    }
    if (status != pdPASS)
    {
        return QURT_EINVALID;
    }
    return QURT_EOK;
}

qurt_time_t qurt_timer_get_ticks(void)
{
    return xTaskGetTickCount();
}

qurt_time_t qurt_timer_get_remaining(void)
{
    return (qurt_time_t)QURT_TIME_WAIT_FOREVER;
}

qurt_time_t qurt_timer_convert_time_to_ticks(qurt_time_t time, qurt_time_unit_t unit_of_time)
{
    switch (unit_of_time)
    {
    case QURT_TIME_TICK:
        return time;
    // Round up
    // 1ms - 10ms = 1 tick
    // 11ms - 20ms = 2 ticks
    case QURT_TIME_MSEC:
        return (((time * QURT_SYSTICK_CLOCK_HZ) + 999) / 1000);
    default:
        return QURT_TIME_WAIT_FOREVER;
    }
}

qurt_time_t qurt_timer_convert_ticks_to_time(qurt_time_t ticks, qurt_time_unit_t time_unit)
{
    switch (time_unit)
    {
    case QURT_TIME_TICK:
        return ticks;

    case QURT_TIME_MSEC:
        return ((ticks * 1000) / QURT_SYSTICK_CLOCK_HZ);
    default:
        return QURT_TIME_WAIT_FOREVER;
    }
}
