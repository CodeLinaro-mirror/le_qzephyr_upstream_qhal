/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * qurt_utils.c
 *
 *  Created on: Dec 30, 2022
 *      Author: Bhagyashree
 */


#include "nt_osal.h"
#include "FreeRTOS.h"
#include "projdefs.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "time.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "qurt_ext.h"

#define TASK_FAILED           -1

#if(NT_FN_QURT == 1)
QueueHandle_t queue_str = NULL;
void *queue_handle = (void *)&queue_str;
QueueHandle_t nt_qurt_pipe_create( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize )
{

		qurt_pipe_attr_t cmd_msg_queue;
		qurt_pipe_attr_init(&cmd_msg_queue);
		qurt_pipe_attr_set_elements(&cmd_msg_queue,uxQueueLength);
		qurt_pipe_attr_set_element_size(&cmd_msg_queue,uxItemSize);
		if(0 == qurt_pipe_create(queue_handle,&cmd_msg_queue))
		{
	//	return (QueueHandle_t)queue_handle;
		return queue_str;
		}
		else
		{
			return NULL;
		}
}

BaseType_t nt_qurt_thread_create( TaskFunction_t pxTaskCode,
		const char * const pcName,		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
		const configSTACK_DEPTH_TYPE usStackDepth,
		void * const pvParameters,
		UBaseType_t uxPriority,
		TaskHandle_t * const pxCreatedTask )
{
	    BaseType_t ret;
		qurt_thread_t *handle = (qurt_thread_t*)pxCreatedTask;
	    qurt_thread_attr_t consoleattributes;
	    qurt_thread_attr_init(&consoleattributes);
	    qurt_thread_attr_set_name(&consoleattributes,pcName);//nt_osal_thread_create
	    qurt_thread_attr_set_priority(&consoleattributes,uxPriority);
	    qurt_thread_attr_set_stack_size(&consoleattributes,usStackDepth);
	    ret = qurt_thread_create(handle,&consoleattributes,pxTaskCode,pvParameters);
	    if(ret == 0)
	    {
	    	return pdPASS;
	    }
	    else
	    {
	    	return TASK_FAILED;
	    }
}

TaskHandle_t nt_qurt_thread_get_id(void)
{
    return qurt_thread_get_id();
}


TimerHandle_t nt_qurt_timer_create(	char * pcTimerName,
		const TickType_t xTimerPeriodInTicks,
		const UBaseType_t uxAutoReload,
		void *  pvTimerID,//const
		TimerCallbackFunction_t pxCallbackFunction )
{

	    qurt_timer_attr_t timer_create ;
	    qurt_timer_attr_init(&timer_create);
	    qurt_timer_attr_set_duration(&timer_create, xTimerPeriodInTicks);

        if( uxAutoReload == 0)
        {
	        qurt_timer_attr_set_option(&timer_create, 1);
        }
        else
        {
        	qurt_timer_attr_set_option(&timer_create, 2);
            qurt_timer_attr_set_reload(&timer_create,xTimerPeriodInTicks);
        }

        qurt_timer_attr_set_callback(&timer_create,pxCallbackFunction, ( void*)&pcTimerName);
        return(qurt_timer_create (pvTimerID, &timer_create,pxCallbackFunction));

}

#endif //NT_FN_QURT



