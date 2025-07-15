/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * qurt_event_groups.c
 *
 *  Created on: May 13, 2020
 *      Author: Abhilash
 */

#include "qurt_ext.h"
#include "FreeRTOS.h"
#include "event_groups.h"

/*====================================================
             EXTERNAL
 =====================================================*/
extern uint32 qurt_system_get_ipsr(void);

/*====================================================
            MACROS
 =====================================================*/
#define QURT_SIGNAL_MASK_CONTROL_BYTES   0xff000000U

/*====================================================
             TYOEDEFS
 =====================================================*/

/*====================================================
             FUNCTIONS
 =====================================================*/

int qurt_signal_create(qurt_signal_t *signal)
{
	EventGroupHandle_t sig_group;
	if(NULL != signal)
	{
		sig_group = xEventGroupCreate();
		if(NULL != sig_group)
		{
			*signal = (qurt_signal_t)sig_group;
			return QURT_EOK;
		}
		else
		{
			return QURT_EFAILED;
		}
	}
	return QURT_EINVALID;
}

int qurt_signal_wait_timed(qurt_signal_t *signal, uint32 mask,
		uint32 attribute,uint32 *curr_signals, qurt_time_t timeout)
{
	BaseType_t clear_on_exit = pdFALSE;
	BaseType_t wait_all_bits = pdFALSE;
	EventBits_t set_signal = 0;
	uint32 sig_mask;
	EventGroupHandle_t sig_group;
	if((NULL == signal) ||(!*signal) )
	{
		return QURT_EINVALID;
	}
	sig_group = (EventGroupHandle_t)*signal;
	// signal wait is not supported from ISR
	ASSERT( 0 == qurt_system_get_ipsr());
	if(attribute & QURT_SIGNAL_ATTR_CLEAR_MASK)
	{
		clear_on_exit = pdTRUE;
	}
	if(attribute & QURT_SIGNAL_ATTR_WAIT_ALL)
	{
		wait_all_bits = pdTRUE;
	}
	// wait forever
	set_signal = xEventGroupWaitBits (sig_group,mask,
			clear_on_exit,wait_all_bits,timeout);
	*curr_signals = (uint32)set_signal;
	sig_mask = set_signal & mask;
	if(wait_all_bits && (sig_mask ^ mask))
	{
		return QURT_EFAILED_TIMEOUT;
	}
	else
	{
		if(sig_mask)
		{
			return QURT_EOK;
		}
	}
	return QURT_EFAILED_TIMEOUT;
}

uint32 qurt_signal_wait(qurt_signal_t *signal, uint32 mask, uint32 attribute )
{
	uint32 set_signal = 0;
	// signal wait is not supported from ISR
	ASSERT (0 == qurt_system_get_ipsr());
	qurt_signal_wait_timed(signal,mask,attribute, &set_signal,portMAX_DELAY);
	return set_signal;
}

void qurt_signal_set(qurt_signal_t *signal, uint32 mask)
{
	EventGroupHandle_t sig_group;
	BaseType_t result;
	if((NULL == signal) || (!*signal))
	{
		return;
	}
	sig_group = (EventGroupHandle_t)*signal;
	// assert if application attempts to set reserved control masks
	ASSERT(!(mask & QURT_SIGNAL_MASK_CONTROL_BYTES));
	// if running in thread mode
	if( 0 == qurt_system_get_ipsr())
	{
		result = (BaseType_t)xEventGroupSetBits(sig_group,mask);
	}
	else
	{
		//higher_prio_task_woken must be initialized to pdFALSE
		BaseType_t higher_prio_task_woken = pdFALSE;
		result = xEventGroupSetBitsFromISR(sig_group, mask,&higher_prio_task_woken);
		if(result != pdFAIL)
		{
			//if higher_prio_task_woken is now set to pdTRUE then
			//a context switch should be requested
			portYIELD_FROM_ISR(higher_prio_task_woken);
		}
	}
}

void qurt_signal_clear(qurt_signal_t *signal, uint32 mask)
{
	EventGroupHandle_t sig_group;
	if((NULL == signal) || (!*signal))
	{
		return;
	}
	sig_group = (EventGroupHandle_t)*signal;
	// assert if application attempts to clear reserved control masks
	ASSERT(!(mask & QURT_SIGNAL_MASK_CONTROL_BYTES));
	// if running in thread mode
	if( 0 == qurt_system_get_ipsr())
	{
		xEventGroupClearBits(sig_group,mask);
	}
	else
	{
		xEventGroupClearBitsFromISR(sig_group,mask);
	}
}

uint32 qurt_signal_get(qurt_signal_t *signal)
{
	EventBits_t get_signal = 0;
	EventGroupHandle_t sig_group;
	if((NULL == signal) || (!*signal))
	{
		return (uint32_t)QURT_EINVALID;

	}
	sig_group = (EventGroupHandle_t)*signal;
	// if running in thread mode
	if(0 == qurt_system_get_ipsr())
	{
		get_signal = xEventGroupGetBits(sig_group);
	}
	else
	{
		get_signal = xEventGroupGetBitsFromISR(sig_group);
	}
	return (uint32)get_signal;
}

void qurt_signal_delete(qurt_signal_t *signal)
{
	EventGroupHandle_t sig_group;
	if((NULL == signal) || (!*signal))
	{
		return;
	}
	sig_group = (EventGroupHandle_t)*signal;
	vEventGroupDelete(sig_group);

}

void qurt_kernel_start()
{
	vTaskStartScheduler();
}

