/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * qurt_pipe.c
 *
 *  Created on: May 11, 2020
 *      Author: Abhilash
 */
#include "qurt_ext.h"
#include "FreeRTOS.h"
#include "queue.h"
/*==================================================
                   EXTERNAL
 ===================================================*/

extern uint32 qurt_system_get_ipsr(void);

/*==================================================
                 TYPEDEFS
 ===================================================*/
//pipe attributes
typedef struct _qurt_pipe_attr {
	UBaseType_t elements;   //number of elements
	UBaseType_t element_size; //size of element
}_qurt_pipe_attr;

/*==================================================
               FUNCTIONS
 ===================================================*/

void qurt_pipe_attr_init (qurt_pipe_attr_t *attr)
{
_qurt_pipe_attr *pattr = (_qurt_pipe_attr *)attr;
pattr->elements = 0;
pattr->element_size = 0;
}

void qurt_pipe_attr_set_elements (qurt_pipe_attr_t *attr, uint32 elements)
{
	_qurt_pipe_attr	*pattr = (_qurt_pipe_attr *)attr;
	pattr->elements = elements;
}

void qurt_pipe_attr_set_element_size (qurt_pipe_attr_t *attr, uint32 element_size)
{
	_qurt_pipe_attr	*pattr = (_qurt_pipe_attr *)attr;
	pattr->element_size = element_size;
}

int qurt_pipe_create(qurt_pipe_t *pipe, qurt_pipe_attr_t *attr)
{
	_qurt_pipe_attr *pattr = (_qurt_pipe_attr *)attr;
	QueueHandle_t xQueue = NULL;
	if(NULL == pattr)
	{
		return QURT_EINVALID;
	}
	xQueue = xQueueCreate( pattr->elements, pattr->element_size);
    if(NULL == xQueue)
    {
    	return QURT_EFAILED;
    }
    *pipe = (qurt_pipe_t)xQueue;
    return QURT_EOK;
}

void qurt_pipe_delete (qurt_pipe_t pipe)
{
	//BaseType_t ret;
	if( NULL != pipe)
	{
		 vQueueDelete((QueueHandle_t)pipe);
	}
}

int qurt_pipe_send_timed(qurt_pipe_t pipe, void *data, qurt_time_t timeout)
{
BaseType_t ret;
    if( NULL == pipe)
    {
	return QURT_EINVALID;
    }
    /* send an item over pipe is not supported from ISR */
    ASSERT(0 == qurt_system_get_ipsr());
    ret = xQueueSendToBack((QueueHandle_t)pipe, (const void *)data, timeout);
    if(pdPASS != ret)
    {
    	return QURT_EFAILED_TIMEOUT;
    }
    return QURT_EOK;
}

void qurt_pipe_send(qurt_pipe_t pipe, void *data)
{
	/* send an item over pipe is not supported from ISR */
	ASSERT(0 == qurt_system_get_ipsr());
	qurt_pipe_send_timed(pipe,data, portMAX_DELAY);
}

int qurt_pipe_try_send(qurt_pipe_t pipe, void *data,BaseType_t * timeout)
{
     BaseType_t ret;
     /*higher_prio_task_woken must be initialized to pdFALSE */
     BaseType_t higher_prio_task_woken = *timeout;
     if( NULL == pipe)
     {
    	 return QURT_EINVALID;
     }
     /* if running in thread mode */
     if( 0== qurt_system_get_ipsr())
     {
    	 ret = xQueueSendToBack((QueueHandle_t)pipe, (const void *)data, 0);
     }
     else
     {
    	 ret = xQueueSendToBackFromISR((QueueHandle_t)pipe, (const void *)data,&higher_prio_task_woken);
     }
     if(pdPASS != ret )
     {
    	 return QURT_EFAILED;
     }
     /* CAlling xQueueSendToBackFromISR within an ISR routine can potentially cause
      cause a task that was blocked on queue to leave the blocked state. A context
      switch should be performed if unblocked task has priority equal or greater then task executing
      before interrupt was taken. If higher_prio_task_woken is true then a context switch should be
      requested
      */
     portYIELD_FROM_ISR(higher_prio_task_woken);
     return QURT_EOK;
}

int qurt_pipe_receive_timed(qurt_pipe_t pipe, void * const data, qurt_time_t timeout)
{
	BaseType_t ret;
	if( NULL == pipe)
	{
		return QURT_EINVALID;
	}
	/* receive an item over pipe is not supported from ISR */
	ret = xQueueReceive((QueueHandle_t)pipe, data,timeout);
	if(pdPASS != ret )
	{
		return QURT_EFAILED_TIMEOUT;
	}
	return QURT_EOK;
}

void qurt_pipe_receive(qurt_pipe_t pipe, void *data)
{
    /* receive an item over pipe is not supported from ISR */
	ASSERT(0 == qurt_system_get_ipsr());
	qurt_pipe_receive_timed(pipe, data,portMAX_DELAY);
}

int qurt_pipe_try_receive(qurt_pipe_t pipe, void * const data,BaseType_t* timeout)
{
    BaseType_t ret;
    /* higher_prio_task_woken must be initialized to pdFALSE */
    BaseType_t higher_prio_task_woken  = *timeout;
    if( NULL == pipe)
    {
    	return QURT_EINVALID;
    }
    if( 0 == qurt_system_get_ipsr())
    {
    	ret = xQueueReceive((QueueHandle_t)pipe,data,0);
    }
    else
    {
    	ret = xQueueReceiveFromISR((QueueHandle_t)pipe,data, &higher_prio_task_woken);
    }
    if( pdPASS != ret )
    {
    	return QURT_EFAILED;
    }
    /* calling xQueueReceiveFromISR within an ISR routine can potentially cause a task
     that was blocked on queue to leave the blocked state. A context switch should be performed
     if higher_prio_task_woken is true then a context switch should be requested
     */
    portYIELD_FROM_ISR(higher_prio_task_woken);
    return QURT_EOK;
}

int qurt_pipe_flush(qurt_pipe_t pipe)
{
    if(NULL == pipe)
    {
    	return QURT_EINVALID;
    }
    if(pdPASS != xQueueReset((QueueHandle_t)pipe))
    {
    	return QURT_EFAILED;
    }
    return QURT_EOK;
}

uint32 qurt_system_get_ipsr( void )
{
    __asm volatile
    (
        "  mrs r0, ipsr  \n"
        "  bx r14        \n"
    );



}
