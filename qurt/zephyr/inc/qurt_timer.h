/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#pragma once

#include "qurt_types.h"
#include <stdbool.h>

#define QURT_TIMER_OBJ_SIZE_BYTES 128

/* TIMER ATTRIBUTES  */

#define QURT_TIMER_ONESHOT 0x01       /**< one short timer  .*/
#define QURT_TIMER_PERIODIC 0x02      /**< periodic timer  .*/
#define QURT_TIMER_NO_AUTO_START 0x04 /**< no Auto deactivate  .*/
#define QURT_TIMER_AUTO_START 0x08    /**< Default, Auto Activate  .*/

#define NT_MS_TO_TICKS(ms) qurt_timer_ms_to_ticks(ms)

#define MS_TO_US(x) ((x) * 1000)    /* millisecond to microsecond */
#define US_TO_MS(x) ((x) / 1000)    /* microsecond to millisecond */
#define MS_TO_TU(x) (((x) * 1000) >> 10)
#define TU_TO_MS(x) ((x) * 1024 / 1000)
#define TU_TO_US(x) ((x) << 10)
#define SEC_TO_MSEC(x)        (x * 1000)

/** qurt_time_unit_t types  */
typedef enum {
    QURT_TIME_TICK,             // return time in ticks
    QURT_TIME_MSEC,             // return time in ms
    QURT_TIME_NONE = 0XFFFFFFFF // identifier to use if no particular return type is needed
} qurt_time_unit_t;

//*****for qtmr start*/

#define FRAME_n QTMR_FRAME_PHYSICAL_4
/* Get current time */
#define TIMER_GET_TIME64() qtmr_get_time64(FRAME_n)

#define QTMR_CNTR_FREQ_HZ (38400000u)

/**
 * Various units supported by the timetick module
 */
typedef enum {
    T_TICK, /**< -- Return time in Ticks */
    T_USEC, /**< -- Return time in Microseconds */
    T_MSEC, /**< -- Return time in Milliseconds */
    T_SEC,  /**< -- Return time in Seconds */

    T_NONE = T_TICK /**< -- use if no paticular return type is needed */
} time_unit_type;

typedef uint64_t time_timetick_type;

/**
 * Qtimer frame type
 */
typedef enum {
    QTMR_FRAME_PHYSICAL_0, /**< Qtimer Physical Frame-0 */
    QTMR_FRAME_PHYSICAL_1, /**< Qtimer Physical Frame-1 */
    QTMR_FRAME_PHYSICAL_2, /**< Qtimer Physical Frame-2 */
    QTMR_FRAME_PHYSICAL_3, /**< Qtimer Physical Frame-3 */
    QTMR_FRAME_PHYSICAL_4, /**< Qtimer Physical Frame-4 */
} qtmr_frame_t;
//*****for qtmr end*/

/* qurt_timer_t types */

/* qurt_timer_cb_func_t types */
typedef void (*qurt_timer_callback_func_t)(void *);

typedef struct qurt_timer_attr /* 8 byte aligned */
{
    unsigned long long _bSpace[QURT_TIMER_OBJ_SIZE_BYTES / sizeof(unsigned long long)];

} qurt_timer_attr_t;

#ifdef INC_FREERTOS_H
#include "timers.h"
#else
typedef struct k_timer *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t timer);
#endif

TickType_t qurt_timer_ms_to_ticks(uint32_t ms);

int qurt_get_expiry_time(TimerHandle_t timer);
bool qurt_timer_Is_Active(TimerHandle_t timer);
int qurt_get_time_period(TimerHandle_t timer);

void qurt_timer_attr_init(qurt_timer_attr_t *attr);
void qurt_timer_attr_set_duration(qurt_timer_attr_t *attr, qurt_time_t duration);
void qurt_timer_attr_set_option(qurt_timer_attr_t *attr, uint32 option);

qurt_time_t qurt_timer_get_ticks(void);
void qurt_timer_cb(TimerHandle_t timer);
int qurt_timer_delete(TimerHandle_t timer, TickType_t block_time);
void qurt_timer_attr_set_reload(qurt_timer_attr_t *attr, qurt_time_t reload_time);
TimerHandle_t qurt_timer_create(void *id, const qurt_timer_attr_t *attr, TimerCallbackFunction_t pxCallbackFunction);
void qurt_timer_attr_set_callback(qurt_timer_attr_t *attr, TimerCallbackFunction_t cbfunc, void *cbctxt);
int qurt_timer_stop(TimerHandle_t timer, TickType_t block_time);
int qurt_timer_start(TimerHandle_t timer, TickType_t block_time);

qurt_time_t qurt_timer_get_ticks(void);
/*
 * Brief : Changes the period of existing timer
 * Parameters : timer : timer is a timer handler to which we intend to change period
 *              timer_period: new time period that we want for the timer
 *              block_time: the maximum amount of block time if the message passing queue is full.
 * return : QURT_EINVALID if timer is not already created
 *          QURT_EFAILED  if the could not change the period
 *          QURT_OK on successfully processing message
 */
int qurt_timer_change_period(TimerHandle_t timer, TickType_t timer_period, TickType_t block_time);

qurt_time_t qurt_timer_convert_time_to_ticks(qurt_time_t time, qurt_time_unit_t unit_of_time);
qurt_time_t qurt_timer_convert_ticks_to_time(qurt_time_t ticks, qurt_time_unit_t time_unit);

TimerHandle_t nt_create_timer(void *call_back_function, void *timer_id, uint32_t time_countdown,
                              UBaseType_t auto_reload);
TimerHandle_t nt_create_pm_timer(char *pcTimerName,void *call_back_function, void *timer_id, TickType_t time_countdown, UBaseType_t auto_reload);

int nt_start_timer(TimerHandle_t timer_handle);
int nt_stop_timer(TimerHandle_t timer_handle);

TimerHandle_t nt_qurt_timer_create(char *pcTimerName, const TickType_t xTimerPeriodInTicks,
                                   const UBaseType_t uxAutoReload,
                                   void *pvTimerID, // const
                                   TimerCallbackFunction_t pxCallbackFunction);

#define NT_TIMER_SUCCESS 0
#define NT_TIMER_FAILURE 1

int nt_timer_change_time_period(TimerHandle_t timer_handle, TickType_t new_period);
void *nt_get_timeout_arg(TimerHandle_t timer_handle);

//*****for qtmr start*/
// TickType_t
void hres_timer_us_delay(uint32_t time_us);
uint64_t hres_timer_curr_time_us(void);
uint32_t hres_timer_curr_time_ms(void);
int8_t timer_cvt_to_tick64(uint64_t time, time_unit_type unit, uint64_t *pTimeRet);
uint64_t qtmr_get_time64(qtmr_frame_t frame);
time_timetick_type  hres_timer_timetick_get(void);
uint32_t qtmr_get_freq(void);
//*****for qtmr end*/
