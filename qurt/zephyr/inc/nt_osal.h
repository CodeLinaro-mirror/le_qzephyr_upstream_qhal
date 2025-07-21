/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef OS_INC_NT_OSAL_H_
#define OS_INC_NT_OSAL_H_

#include <zephyr/kernel.h>
#include <string.h>
#include "qurt_ext.h"
#include "zephyr/sys/__assert.h"
#include "zephyr/kernel/thread.h"

/*Semaphore ID identifies the semaphore*/
typedef qurt_sem_t *nt_osal_semaphore_handle_t;

/*Thread ID identifies the thread*/
typedef TaskHandle_t nt_osal_task_handle_t;

/*Message Queue ID identifies the message queue*/
typedef QueueHandle_t nt_osal_queue_handle_t;

/*Timer ID identifies the Timer*/
typedef TimerHandle_t nt_osal_timer_handle_t;

typedef TickType_t nt_osal_tick_type_t;

/******************************MACRO DEFINITION***************************/
#define NT_OSAL_NAME			"Zephyr"	/* Internally which OS we are using right now 	*/
#define NT_OSAL_VERSION         0x00001     /* OSAL version definition 					*/
#define pdFALSE                                  ( ( BaseType_t ) 0 )
#define pdTRUE                                   ( ( BaseType_t ) 1 )
#define pdPASS                                   ( pdTRUE )
#define pdFAIL                                   ( pdFALSE )
#define nt_pass 				(QURT_EOK)
#define nt_fail 				(pdFAIL)
#define NT_QUEUE_SUCCESS   		(QURT_EOK)
#define NT_QUEUE_FAIL      		(QURT_EFAILED_TIMEOUT)

#define nt_osal_thread_create(ptr_function, name, stack_size, argument, priority, handler)	\
		nt_qurt_thread_create(ptr_function, name, stack_size, argument, priority, handler)

/*Delete the task*/
#define nt_osal_thread_delete(handler) \
		qurt_thread_join(handler, 0)

/*Put a Message to a Queue*/

#define nt_osal_queue_send(queue_id, message, tick_to_wait)		\
		qurt_pipe_send_timed(queue_id,	message,tick_to_wait)

#define nt_osal_queue_send_from_isr(queue_id, message, tasktoken) \
		qurt_pipe_try_send(queue_id, message, tasktoken)

#define nt_osal_queue_msg_receive(queue_handle, msg_buffer, block_time) \
		qurt_pipe_receive_timed(queue_handle, msg_buffer, block_time)

#define nt_osal_queue_msg_receive_from_isr(queue_handle, msg_buffer, block_time) \
	qurt_pipe_try_receive(queue_handle, msg_buffer,block_time)

#define nt_osal_delay(mil_sec) \
			qurt_thread_sleep_ms(mil_sec)

#define qurt_sleep_ms qurt_thread_sleep_ms

/*Create and Initialize a Semaphore object*/
#define nt_osal_semaphore_create_binary(sem)	\
    qurt_sem_create(&sem)

#define nt_osal_semaphore_delete(sem) \
               qurt_sem_destroy(sem)

#define nt_osal_higher_priority_task_woken xHigherPriorityTaskWoken

#define nt_osal_yield_from_isr(nt_osal_higher_priority_task_woken)	\
	k_yield()


/*Change Time Period*/
#define nt_osal_timer_change_period(timer_handle, period) \
        qurt_timer_change_period(timer_handle, period, 0)


/*allocating heap memory*/
#define nt_osal_allocate_memory(size) \
			k_malloc(size)

#define nt_osal_free_memory(ptr) \
			k_free(ptr)

#define nt_osal_calloc(count, size) \
			k_calloc(count, size)

/*to check is timer active or not*/
#define nt_osal_is_timer_active(timer_handle) \
			qurt_timer_Is_Active(timer_handle)

/*to get timer period*/
#define nt_osal_get_time_period(timer_handle) \
			qurt_get_time_period(timer_handle)

#define nt_osal_get_current_task_name() \
			k_thread_name_get(k_current_get())

/*Release a Semaphore token from isr*/
#define nt_osal_semaphore_give_from_isr(sem, target_task) \
			qurt_sem_up(sem, target_task)

#define taskENTER_CRITICAL()	\
	do {						\
		k_sched_lock();			\
	} while (0)

#define taskEXIT_CRITICAL()		\
	do {						\
		k_sched_unlock();		\
	} while (0)

/* expr == 0, trigger assert action*/
#define configASSERT(expr) 		\
    __ASSERT(expr, "")

/**
 * <!-- nt_normal_delay -->
 *
 * @brief Normal delay function
 * @param time: Delay time needed in milli seconds
 * @return: void
 */
void nt_normal_delay(uint32_t time);
int tickless_idle_enabled (void);

#define xTaskNotify2evt(xTaskToNotify) evt##xTaskToNotify

#define xTaskNotify( xTaskToNotify, ulValue, eAction )   qurt_signal_set(&(xTaskNotify2evt(xTaskToNotify)), (ulValue))
#define xTaskNotifyFromISR( xTaskToNotify, ulValue, eAction, pxHigherPriorityTaskWoken )  do { \
    xTaskNotify(xTaskToNotify, (ulValue), (eAction)); \
    (void)(pxHigherPriorityTaskWoken); \
    } while (0)
#define portYIELD_FROM_ISR( x )  ((void)(x))

#define xTaskNotifyWait( ulBitsToClearOnEntry, ulBitsToClearOnExit, pulNotificationValue, xTicksToWait )   0

#endif /* OS_INC_NT_OSAL_H_ */
