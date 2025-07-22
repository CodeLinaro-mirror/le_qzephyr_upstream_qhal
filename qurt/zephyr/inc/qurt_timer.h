/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "qurt_types.h"
#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>

#define QURT_TIMER_OBJ_SIZE_BYTES 128

/* TIMER ATTRIBUTES  */

#define QURT_TIMER_ONESHOT 0x01       /**< one short timer  .*/
#define QURT_TIMER_PERIODIC 0x02      /**< periodic timer  .*/
#define QURT_TIMER_NO_AUTO_START 0x04 /**< no Auto deactivate  .*/
#define QURT_TIMER_AUTO_START 0x08    /**< Default, Auto Activate  .*/

#define NT_MS_TO_TICKS(ms) Z_TIMEOUT_MS_TICKS(ms)
#define TICKS_TO_MS(ticks) k_ticks_to_ms_ceil32(ticks)

/** qurt_time_unit_t types  */
typedef enum {
    QURT_TIME_TICK,             // return time in ticks
    QURT_TIME_MSEC,             // return time in ms
    QURT_TIME_NONE = 0XFFFFFFFF // identifier to use if no particular return type is needed
} qurt_time_unit_t;

/* qurt_timer_t types */

/* qurt_timer_cb_func_t types */
typedef void (*qurt_timer_callback_func_t)(void *);

typedef struct qurt_timer_attr /* 8 byte aligned */
{
    unsigned long long _bSpace[QURT_TIMER_OBJ_SIZE_BYTES / sizeof(unsigned long long)];

} qurt_timer_attr_t;

typedef struct k_timer *TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t timer);

int qurt_get_expiry_time(TimerHandle_t timer);
int qurt_timer_Is_Active(TimerHandle_t timer);
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

TimerHandle_t nt_create_timer(void *call_back_function, void *timer_id, uint32_t time_countdown,
                              UBaseType_t auto_reload);
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
