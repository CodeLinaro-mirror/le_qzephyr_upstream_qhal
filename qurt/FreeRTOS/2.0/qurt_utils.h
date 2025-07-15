/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/*
 * qurt_utils.h
 *
 *  Created on: Dec 30, 2022
 *      Author: Bhagyashree
 */

#ifndef BUILD_FREERTOS_COMMON_CONFIG_FILES_QURT_UTILS_H_
#define BUILD_FREERTOS_COMMON_CONFIG_FILES_QURT_UTILS_H_


#include "FreeRTOS.h"
#include <string.h>

#include "autoconf.h"

#include "projdefs.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "time.h"
#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "qurt_ext.h"
#include <stdlib.h>

QueueHandle_t nt_qurt_pipe_create( const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize );

BaseType_t nt_qurt_thread_create(	TaskFunction_t pxTaskCode,
		const char * const pcName,		/*lint !e971 Unqualified char types are allowed for strings and single characters only. */
		const configSTACK_DEPTH_TYPE usStackDepth,
		void * const pvParameters,
		UBaseType_t uxPriority,
		TaskHandle_t * const pxCreatedTask );

TimerHandle_t nt_qurt_timer_create(	char * pcTimerName,
		const TickType_t xTimerPeriodInTicks,
		const UBaseType_t uxAutoReload,
		void *  pvTimerID,//const
		TimerCallbackFunction_t pxCallbackFunction );

TaskHandle_t nt_qurt_thread_get_id(void);

#endif /* BUILD_FREERTOS_COMMON_CONFIG_FILES_QURT_UTILS_H_ */
