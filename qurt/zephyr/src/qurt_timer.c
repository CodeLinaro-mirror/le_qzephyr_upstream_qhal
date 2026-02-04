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
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/sys_clock.h>
#include <timeout_q.h>
#include "qurt_timer.h"
#include "qurt_error.h"
#include "fermion_reg.h"

#define QURT_TIMER_NOTIFY_TYPE_CALLBACK 1
#define QURT_TIMER_NOTIFY_TYPE_SIGNAL 2

typedef struct _qurt_timer_attr_t {
    qurt_time_t duration;
    qurt_time_t reload;
    qurt_time_t remaining;
    qurt_time_t start;
    uint32 option;
    uint32 expire_count;
    void *id;
} _qurt_timer_attr_t;

typedef struct _qurt_timer_t {
    struct k_timer timer;
    _qurt_timer_attr_t qurt_timer_info;
    struct k_work work;
    TimerCallbackFunction_t user_callback;
} _qurt_timer_t;

static void timer_work_handler(struct k_work *work)
{
    _qurt_timer_t *_qtimer = CONTAINER_OF(work, _qurt_timer_t, work);

    if (_qtimer->user_callback) {
        _qtimer->user_callback(&_qtimer->timer);
    }
}

static void k_timer_expiry_wrapper(struct k_timer *timer)
{
    _qurt_timer_t *_qtimer = CONTAINER_OF(timer, _qurt_timer_t, timer);

    k_work_submit(&_qtimer->work);
}


TickType_t qurt_timer_ms_to_ticks(uint32_t ms)
{
    return Z_TIMEOUT_MS_TICKS(ms);
}

void qurt_timer_attr_init(qurt_timer_attr_t *attr) { memset(attr, 0, QURT_TIMER_OBJ_SIZE_BYTES); }

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

void qurt_timer_attr_set_option(qurt_timer_attr_t *attr, uint32 option)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    pattr->option = option;
}

TimerHandle_t qurt_timer_create(void *id, const qurt_timer_attr_t *attr, TimerCallbackFunction_t expiry_fn)
{
    _qurt_timer_t *_qtimer = NULL;
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;

    if (NULL == pattr) {
        return NULL;
    }

    if (pattr->option & QURT_TIMER_ONESHOT) {
        if (pattr->reload) {
            return NULL;
        }
    } else if (pattr->option & QURT_TIMER_PERIODIC) {
        if (!pattr->reload) {
            return NULL;
        }
    }

    _qtimer = k_calloc(sizeof(_qurt_timer_t), 1);
    if (!_qtimer) {
        return NULL;
    }

    _qtimer->qurt_timer_info.duration = pattr->duration;
    _qtimer->qurt_timer_info.reload = pattr->reload;
    _qtimer->qurt_timer_info.id = id;
    _qtimer->user_callback = expiry_fn;

    k_work_init(&_qtimer->work, timer_work_handler);

    k_timer_init(&_qtimer->timer, k_timer_expiry_wrapper, NULL);

    return &_qtimer->timer;
}

int qurt_timer_start(TimerHandle_t qtimer, TickType_t block_time)
{
    _qurt_timer_t *_qtimer = (_qurt_timer_t *)qtimer;
    (void)block_time;

    if (NULL == qtimer) {
        return QURT_EINVALID;
    }

    k_timer_start(qtimer, K_TICKS(_qtimer->qurt_timer_info.duration), K_TICKS(_qtimer->qurt_timer_info.reload));

    return QURT_EOK;
}

int qurt_timer_start_frm_isr(TimerHandle_t timer, long *const port_yield)
{
    (void)port_yield;
    return qurt_timer_start(timer, 0);
}

int qurt_get_expiry_time(TimerHandle_t timer)
{
    if (NULL == timer) {
        return QURT_EINVALID;
    }

    return k_timer_expires_ticks(timer);
}

int qurt_get_time_period(TimerHandle_t timer)
{
    _qurt_timer_t *_qtimer = (_qurt_timer_t *)timer;

    if (NULL == timer) {
        return QURT_EINVALID;
    }

    return _qtimer->qurt_timer_info.reload;
}

bool qurt_timer_Is_Active(TimerHandle_t timer)
{
    if (NULL == timer) {
        return false;
    }

    if (z_is_inactive_timeout(&timer->timeout)) {
        return false;
    }
    
    return true;
}

int qurt_timer_stop(TimerHandle_t timer, TickType_t block_time)
{
    (void)block_time;
    if (NULL == timer) {
        return QURT_EINVALID;
    }

    k_timer_stop(timer);

    return QURT_EOK;
}

int qurt_timer_change_period(TimerHandle_t timer, TickType_t period, TickType_t block_time)
{
    _qurt_timer_t *_qtimer = (_qurt_timer_t *)timer;
    (void)block_time;

    if (NULL == timer) {
        return QURT_EINVALID;
    }

    if (_qtimer->qurt_timer_info.option & QURT_TIMER_ONESHOT) {
        assert(0);
        return QURT_EINVALID;
    }

    _qtimer->qurt_timer_info.reload = period;
    _qtimer->qurt_timer_info.duration = period;
    k_timer_start(timer, K_TICKS(_qtimer->qurt_timer_info.duration), K_TICKS(_qtimer->qurt_timer_info.reload));

    return QURT_EOK;
}

int qurt_timer_attr_get_duration(qurt_timer_attr_t *attr, qurt_time_t *duration)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == duration) {
        return QURT_EINVALID;
    }
    *duration = pattr->duration;
    return QURT_EOK;
}

int qurt_timer_attr_get_option(qurt_timer_attr_t *attr, uint32 *option)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == option) {
        return QURT_EINVALID;
    }
    *option = pattr->option;
    return QURT_EOK;
}

int qurt_timer_attr_get_remaining(qurt_timer_attr_t *attr, qurt_time_t *remaining)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == remaining) {
        return QURT_EINVALID;
    }
    *remaining = pattr->remaining;
    return QURT_EOK;
}

int qurt_timer_attr_get_reload(qurt_timer_attr_t *attr, qurt_time_t *reload_time)
{
    _qurt_timer_attr_t *pattr = (_qurt_timer_attr_t *)attr;
    if (NULL == pattr || NULL == reload_time) {
        return QURT_EINVALID;
    }
    *reload_time = pattr->reload;
    return QURT_EOK;
}

/* Weak function declaration that points to pm_timer_unregister */
__weak void pm_timer_unregister_internal(TimerHandle_t k_timer_handle)
{
    /* Empty implementation when pm_timer_unregister doesn't exist */
}

int qurt_timer_delete(TimerHandle_t timer, TickType_t block_time)
{
    if (NULL == timer) {
        return QURT_EINVALID;
    }

    _qurt_timer_t *_qtimer = (_qurt_timer_t *)timer;

    pm_timer_unregister_internal(timer); 

    k_timer_stop(timer);
    k_work_cancel(&_qtimer->work);
    k_work_flush(&_qtimer->work, NULL);
    k_free(timer);

    return QURT_EOK;
}

// k_uptime_ticks
qurt_time_t qurt_timer_get_ticks(void) { return sys_clock_tick_get_32(); }

qurt_time_t qurt_timer_get_remaining(void) { return (qurt_time_t)QURT_TIME_WAIT_FOREVER; }

qurt_time_t qurt_timer_convert_time_to_ticks(qurt_time_t time, qurt_time_unit_t unit_of_time)
{
    switch (unit_of_time) {
    case QURT_TIME_TICK:
        return time;
    // Round up
    // 1ms - 10ms = 1 tick
    // 11ms - 20ms = 2 ticks
    case QURT_TIME_MSEC:
        return Z_TIMEOUT_MS_TICKS(time);
    default:
        return QURT_TIME_WAIT_FOREVER;
    }
}

qurt_time_t qurt_timer_convert_ticks_to_time(qurt_time_t ticks, qurt_time_unit_t time_unit)
{
    switch (time_unit) {
    case QURT_TIME_TICK:
        return ticks;

    case QURT_TIME_MSEC:
        return k_ticks_to_ms_ceil32(ticks);
    default:
        return QURT_TIME_WAIT_FOREVER;
    }
}

TimerHandle_t nt_qurt_timer_create(char *pcTimerName, const TickType_t xTimerPeriodInTicks,
                                   const UBaseType_t uxAutoReload, void *pvTimerID,
                                   TimerCallbackFunction_t pxCallbackFunction)
{
    qurt_timer_attr_t timer_create;
    qurt_timer_attr_init(&timer_create);
    qurt_timer_attr_set_duration(&timer_create, xTimerPeriodInTicks);

    if (uxAutoReload == 0) {
        qurt_timer_attr_set_option(&timer_create, QURT_TIMER_ONESHOT);
    } else {
        qurt_timer_attr_set_option(&timer_create, QURT_TIMER_PERIODIC);
        qurt_timer_attr_set_reload(&timer_create, xTimerPeriodInTicks);
    }

    return qurt_timer_create(pvTimerID, &timer_create, pxCallbackFunction);
}

#define TIMER_NAME "sme_timer"

/*Timer Functions*/
// target to call back function
TimerHandle_t nt_create_timer(void *call_back_function, void *timer_id, TickType_t time_countdown,
                              UBaseType_t auto_reload)
{
    return nt_qurt_timer_create(TIMER_NAME, time_countdown, auto_reload, timer_id, call_back_function);
}

/* Weak function declaration that points to pm_timer_register */
__weak void pm_timer_register_internal(char *pcTimerName,TimerHandle_t k_timer_handle)
{
    /* Empty implementation when pm_timer_register doesn't exist */
}

TimerHandle_t nt_create_pm_timer(char *pcTimerName,void *call_back_function, void *timer_id, TickType_t time_countdown, UBaseType_t auto_reload)
{
    TimerHandle_t k_timer_handle = NULL;

    k_timer_handle = nt_qurt_timer_create(pcTimerName, time_countdown, auto_reload, timer_id, call_back_function);

    if(k_timer_handle && pcTimerName) {
        /* Call the weak function that will link to pm_timer_register if it exists */
        pm_timer_register_internal(pcTimerName, k_timer_handle);
    }

    return k_timer_handle;
}


int nt_start_timer(TimerHandle_t timer_handle) { return qurt_timer_start(timer_handle, 100); }

int nt_stop_timer(TimerHandle_t timer_handle) { return qurt_timer_stop(timer_handle, 100); }

int nt_timer_change_time_period(TimerHandle_t timer, TickType_t period)
{
    return qurt_timer_change_period(timer, period, 0);
    ;
}

void *nt_get_timeout_arg(TimerHandle_t timer_handle)
{
    _qurt_timer_t *_qtimer = (_qurt_timer_t *)timer_handle;
    if (_qtimer) {
        return _qtimer->qurt_timer_info.id;
    } else {
        return NULL;
    }
}

#if 0
void hres_timer_us_delay(uint32_t time_us)
{
    uint64_t curr_time = hres_timer_curr_time_us();
    uint64_t target_time = (curr_time + time_us);

    while(curr_time < target_time)
    {
        curr_time = hres_timer_curr_time_us();
    }
}
#else
void hres_timer_us_delay(uint32_t time_us) { k_busy_wait(time_us); }
#endif

uint64_t hres_timer_curr_time_us(void)
{
#if 0
#ifdef SUPPORT_HIGH_RES_TIMER
    uint64_t curr_time_us;
    timer_cvt_from_tick64(hres_timer_timetick_get(), T_USEC, &curr_time_us);
    return curr_time_us;
#else  /* SUPPORT_HIGH_RES_TIMER */
    return k_uptime_get()*1000;
#endif /* SUPPORT_HIGH_RES_TIMER */
#else
    uint64_t current_ticks;
    uint32_t count_lo, count_hi;
    uint64_t cntr_freq_hz = 38400000u;
    uint64_t curr_time_us;

    count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T4_OFFSET);
    count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T4_OFFSET);
    current_ticks = (((uint64_t)count_hi << 32) | count_lo);
    curr_time_us = (current_ticks * 1000000) / cntr_freq_hz;
    return curr_time_us;
#endif
}

/**
 * @brief   Get frequency of QTMR counter
 * @return  qtmr counter frequency
 */
uint32_t qtmr_get_freq(void)
{
    return QTMR_CNTR_FREQ_HZ;
}

/**
 * @brief   Get the current timer counter value.
 * @return  Current time in ticks
 */
uint64_t qtmr_get_time64(qtmr_frame_t frame)
{
    uint64_t current_ticks;
    uint32_t count_lo, count_hi;

    switch (frame) {
        case QTMR_FRAME_PHYSICAL_0:
            count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T0_OFFSET);
            count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T0_OFFSET);
            break;
        case QTMR_FRAME_PHYSICAL_1:
            count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T1_OFFSET);
            count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T1_OFFSET);
            break;
        case QTMR_FRAME_PHYSICAL_2:
            count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T2_OFFSET);
            count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T2_OFFSET);
            break;
        case QTMR_FRAME_PHYSICAL_3:
            count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T3_OFFSET);
            count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T3_OFFSET);
            break;
        case QTMR_FRAME_PHYSICAL_4:
            count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T4_OFFSET);
            count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T4_OFFSET);
            break;
        default:
            count_hi = HWIO_QTMR_V1_QTMR_V1_CNTPCT_HI_IN(SEQ_WCSS_QTMR_V1_T4_OFFSET);
            count_lo = HWIO_QTMR_V1_QTMR_V1_CNTPCT_LO_IN(SEQ_WCSS_QTMR_V1_T4_OFFSET);
            break;
    }

    current_ticks = (((uint64_t)count_hi << 32) | count_lo);

    return current_ticks;
}

/**
 * hres_timer_timetick_get
 *
 * @brief This function gets the timetick and stores the value in global variable
 * Timer module should be initialized before calling this function
 *
 * @return timetick value
 */
time_timetick_type  hres_timer_timetick_get(void)
{
    return TIMER_GET_TIME64();
}

/**
 * timer_cvt_to_tick64
 *
 * @brief Converts time to ticks
 *
 * @param time Timer to convert
 * @param unit Time unit type
 * @param pTimeRet Pointer to store the result
 *
 * @return Success or failure after conversion
 */
int8_t timer_cvt_to_tick64(uint64_t time, time_unit_type unit, uint64_t *pTimeRet)
{
    uint64_t cntr_freq_hz = qtmr_get_freq();

    switch (unit) {
        case T_SEC:
            /* Seconds to ticks */
            time *= cntr_freq_hz;
            break;

        case T_MSEC:
            /* Milliseconds to ticks */
            time = (time * cntr_freq_hz) / 1000;
            break;

        case T_USEC:
            /* Microseconds to ticks */
            time = ((time * cntr_freq_hz) / 1000000);
            break;

        case T_TICK:
            /* Time already in tick */
            break;

        default:
            return -1;
    }

    *pTimeRet = time;

    return 0;

} /* timer_cvt_to_tick64 */

uint32_t hres_timer_curr_time_ms(void)
{
#ifdef SUPPORT_HIGH_RES_TIMER
    uint64_t curr_time_ms;
    timer_cvt_from_tick64(hres_timer_timetick_get(), T_MSEC, &curr_time_ms);
    return curr_time_ms;
#else  /* SUPPORT_HIGH_RES_TIMER */
     return k_uptime_get_32();
#endif /* SUPPORT_HIGH_RES_TIMER */
}

