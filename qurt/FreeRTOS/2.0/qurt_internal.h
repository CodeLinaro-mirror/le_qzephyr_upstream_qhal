/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * qurt_internal.h
 *
 *  Created on: May 8, 2020
 *      Author: Abhilash
 */

#ifndef BUILD_FREERTOS_COMMON_CONFIG_FILES_QURT_INTERNAL_H_
#define BUILD_FREERTOS_COMMON_CONFIG_FILES_QURT_INTERNAL_H_

#ifndef QURT_INTERNAL_H
#define QURT_INTERNAL_H


#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include <stdlib.h>
#include "qurt_utils.h"
#include "qurt_signal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*====================================================
     CONSTANT AND MACROS
 ====================================================*/
#undef ASSERT
#define ASSERT( x ) if( ( x )  == 0) { taskDISABLE_INTERRUPTS(); for( ;; ); }
/*===================================================
    Constants
===================================================== */
#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE                                1 //Boolean true value
#define FALSE                               0 //Boolean false value

#ifndef NULL
#define NULL  ( (void * ) 0)
#endif
/* Time */
#define QURT_TIME_NO_WAIT            0x00000000
#define QURT_TIME_WAIT_FOREVER       0xFFFFFFFF

/* OS object size */
#define QURT_INFO_OBJ_SIZE_BYTES            64
#define QURT_MUTEX_OBJ_SIZE_BYTES           64
#define QURT_SIGNAL_OBJ_SIZE_BYTES          64
#define QURT_THREAD_ATTR_OBJ_SIZE_BYTES    128
#define QURT_TIMER_OBJ_SIZE_BYTES          128
#define QURT_PIPE_ATTR_OBJ_SIZE_BYTES      16

/* THREAD ATTRIBUTES */
#define QURT_THREAD_ATTR_PRIORITY_MASK       0
#define QURT_THREAD_ATTR_PRIORITY_DEFAULT    16
#define QURT_THREAD_ATTR_PRIORITY_MIN        31
#define QURT_THREAD_ATTR_NAME_MAXLEN         10
#define QURT_THREAD_DEFAULT_STACK_SIZE      128
#define QURT_THREAD_MAX_PRIORITIES           20

/* TIMER ATTRIBUTES  */

#define QURT_TIMER_ONESHOT                 0x01 /**< one short timer  .*/
#define QURT_TIMER_PERIODIC                0x02 /**< periodic timer  .*/
#define QURT_TIMER_NO_AUTO_START           0x04 /**< no Auto deactivate  .*/
#define QURT_TIMER_AUTO_START              0x08 /**< Default, Auto Activate  .*/

/* Error Codes       */
#define QURT_EOK                              0 //operation successful changed
#define QURT_EFATAL                          -1 //fatal error which should never happen
#define QURT_EVAL                            -2 //wrong values for parameters. The specified page does not exist
#define QURT_EMEM                            -3 //not enough memory to perform operation
#define QURT_EINVALID                        -4 //Invalid arg value. Invalid key
#define QURT_EFAILED_TIMEOUT                 -5 //time out
#define QURT_EUNKNOWN                        -6 //Defined but never used in BLAST
#define QURT_EFAILED                         -7 //operation failed
#define QURT_ENOMSGS                         -8 //message queue empty
#define QURT_ENOTALLOWED                     -9 //operation not allowed

/*===============================================================
                        Standard Types
 ================================================================*/

/* The following definitions are the same across platforms */
#ifndef _ARM_ASM
#ifndef   _BOOLEAN_DEFINED
typedef  unsigned char boolean; /* Boolean value type */
#define   _BOOLEAN_DEFINED
#endif

#ifndef  _UINT32_DEFINED
typedef unsigned long int uint32; //Unsigned 32 bit value
#define  _UINT32_DEFINED
#endif

#ifndef  _UINT16_DEFINED
typedef unsigned short    uint16; //Unsigned 16 bit value
#define  _UINT16_DEFINED
#endif

#ifndef  _UINT8_DEFINED
typedef unsigned char      uint8; //Unsigned 8 bit value
#define  _UINT8_DEFINED
#endif

#ifndef  _INT32_DEFINED
typedef signed long int   int32; //signed 32 bit value
#define  _INT32_DEFINED
#endif

#ifndef  _INT16_DEFINED
typedef signed short       int16; //signed 16 bit value
#define  _INT16_DEFINED
#endif

#ifndef  _INT8_DEFINED
typedef signed char         int8; //signed 8 bit value
#define  _INT8_DEFINED
#endif

#ifndef  _INT64_DEFINED
typedef long long          int64;
#define  _INT64_DEFINED
#endif

#ifndef  _UINT64_DEFINED
typedef unsigned long long   uint64;
#define  _UINT64_DEFINED
#endif

#endif /* ARM_ASM */
#endif
/*=============================================================
                  TYPEDEFS
 ==============================================================*/

/* qurt info */
typedef struct _qurt_info_t
{
	void (*idlehook) (uint32);
	void (*bsphook)(void);
	uint8 *rtos_heap_start;
	unsigned long rtos_heap_size;
	uint32 idle_time;
}_qurt_info_t;

/* qurt_interrupt_handler_func_ptr type */
typedef void (*qurt_interrupt_handler_func_ptr_t) ( uint32 n_irq );

/** qurt_time_t types   */
typedef uint32 qurt_time_t;

/** qurt_time_unit_t types  */
typedef enum {
	QURT_TIME_TICK ,             // return time in ticks
	QURT_TIME_MSEC ,             // return time in ms
	QURT_TIME_NONE=0XFFFFFFFF    // identifier to use if no particular return type is needed
}qurt_time_unit_t;

/* qurt init data types   */
typedef struct qurt_data_s
{
	/*Pointer to the heap usedc by rtos */
	void *hPtr;
	/*Reserved pointer for future use */
	void *rPtr;
}qurt_data_t;

/* Qurt info type  */
typedef struct qurt_info /*8 bit aligned */
{
	unsigned long long _bSpace[QURT_INFO_OBJ_SIZE_BYTES/sizeof(unsigned long long)];
}qurt_info_t;

/* Qurt mutex type  */
typedef unsigned int qurt_mutex_t;

/* Thread ID typed   */
typedef unsigned long qurt_thread_t;

/* Thread attributes structure   */
typedef struct qurt_thread_attr  /*8 byte aligned */
{
	unsigned long long _bSpace[QURT_THREAD_ATTR_OBJ_SIZE_BYTES/sizeof(unsigned long long)];
}qurt_thread_attr_t;

/* qurt_timer_t types */
typedef unsigned long qurt_timer_t;

/* qurt_timer_cb_func_t types */
typedef void (*qurt_timer_callback_func_t)(void *);

typedef struct qurt_timer_attr  /* 8 byte aligned */
{
	unsigned long long _bSpace[QURT_TIMER_OBJ_SIZE_BYTES/sizeof(unsigned long long)];

}qurt_timer_attr_t;

/* Represents pipes  */
typedef void * qurt_pipe_t;

/* Represents pipe attributes */
typedef struct qurt_pipe_attr /* 8 byte aligned */
{
	unsigned long long _bSpace[QURT_PIPE_ATTR_OBJ_SIZE_BYTES/sizeof(unsigned long long)];
}qurt_pipe_attr_t;

/*========================================================================
              FUNCTIONS
 =========================================================================*/
void qurt_thread_attr_init (qurt_thread_attr_t *attr);
void qurt_thread_attr_set_name (qurt_thread_attr_t *attr, const char *name);
void qurt_thread_attr_set_priority (qurt_thread_attr_t *attr, UBaseType_t priority);
void qurt_thread_attr_set_stack_size (qurt_thread_attr_t *attr, uint16 stack_size);
long qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void(*entrypoint)(void *), void *arg);
void qurt_thread_sleep (qurt_thread_t duration);
int qurt_thread_set_priority (qurt_thread_t thread_id, UBaseType_t newprio);
TaskHandle_t qurt_thread_get_id (void);

const char *qurt_get_parameters( const char *pcCommandString, UBaseType_t uxWantedParameter, BaseType_t *pxParameterStringLength );
char* qurt_thread_get_name (TaskHandle_t handle);
void qurt_kernel_start();
void qurt_thread_stop ();

void qurt_pipe_attr_init (qurt_pipe_attr_t *attr);
void qurt_pipe_attr_set_elements (qurt_pipe_attr_t *attr, uint32 elements);
void qurt_pipe_attr_set_element_size (qurt_pipe_attr_t *attr, uint32 element_size);
int qurt_pipe_create(qurt_pipe_t *pipe, qurt_pipe_attr_t *attr);
void qurt_pipe_delete (qurt_pipe_t pipe);
int qurt_pipe_send_timed(qurt_pipe_t pipe, void *data, qurt_time_t timeout);
void qurt_pipe_send(qurt_pipe_t pipe, void *data);
int qurt_pipe_try_send(qurt_pipe_t pipe, void *data,BaseType_t* timeout);
int qurt_pipe_receive_timed(qurt_pipe_t pipe, void * const data, qurt_time_t timeout);
void qurt_pipe_receive(qurt_pipe_t pipe, void *data);
int qurt_pipe_try_receive(qurt_pipe_t pipe, void * const data,BaseType_t* timeout);
int qurt_pipe_flush(qurt_pipe_t pipe);
int qurt_get_expiry_time(TimerHandle_t timer);
int qurt_timer_Is_Active(TimerHandle_t timer);
int qurt_get_time_period(TimerHandle_t timer);

qurt_time_t qurt_timer_get_ticks(void);
void qurt_timer_cb (TimerHandle_t free_rtos_timer_info);
int qurt_timer_delete (TimerHandle_t timer_handle, TickType_t block_time);
void qurt_timer_attr_init(qurt_timer_attr_t *attr);
void qurt_timer_attr_set_duration(qurt_timer_attr_t *attr, qurt_time_t duration);
void qurt_timer_attr_set_option(qurt_timer_attr_t *attr, uint32 option);
void qurt_timer_attr_set_reload(qurt_timer_attr_t *attr,qurt_time_t reload_time);
TimerHandle_t qurt_timer_create (void *id, const qurt_timer_attr_t *attr,TimerCallbackFunction_t pxCallbackFunction);
void qurt_timer_attr_set_callback(qurt_timer_attr_t *attr, TimerCallbackFunction_t cbfunc, void *cbctxt);
int qurt_timer_stop(TimerHandle_t timer,TickType_t block_time);
int qurt_timer_start(TimerHandle_t timer,TickType_t block_time);
int qurt_timer_start_frm_isr(TimerHandle_t timer, long *const port_yield);
int qurt_timer_stop_frm_isr(TimerHandle_t timer, long *const port_yield);
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
int qurt_timer_change_period(TimerHandle_t timer, TickType_t timer_period,TickType_t block_time);

uint32 qurt_system_get_ipsr (void) __attribute__ (( naked ));


#define qurt_thread_notify(task, value, action) xTaskNotify(task, value, action)
#ifdef __cplusplus
}
#endif



#endif /* BUILD_FREERTOS_COMMON_CONFIG_FILES_QURT_INTERNAL_H_ */
