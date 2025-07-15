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
#include "qurt_pipe.h"
#include "qurt_error.h"
#include "zephyr/sys_clock.h"
#include <zephyr/kernel.h>

/*==================================================
                 TYPEDEFS
 ===================================================*/
//pipe attributes
typedef struct _qurt_pipe_attr {
	uint32_t elements;   //number of elements
	uint32_t element_size; //size of element
}_qurt_pipe_attr;

/*==================================================
               FUNCTIONS
 ===================================================*/

void qurt_pipe_attr_init(qurt_pipe_attr_t *attr)
{
    _qurt_pipe_attr *pattr = (_qurt_pipe_attr *)attr;
    pattr->elements = 0;
    pattr->element_size = 0;
}

void qurt_pipe_attr_set_elements(qurt_pipe_attr_t *attr, uint32 elements)
{
	_qurt_pipe_attr	*pattr = (_qurt_pipe_attr *)attr;
	pattr->elements = elements;
}

void qurt_pipe_attr_set_element_size(qurt_pipe_attr_t *attr, uint32 element_size)
{
	_qurt_pipe_attr	*pattr = (_qurt_pipe_attr *)attr;
	pattr->element_size = element_size;
}

int qurt_pipe_create(qurt_pipe_t *pipe, qurt_pipe_attr_t *attr)
{
	_qurt_pipe_attr *pattr = (_qurt_pipe_attr *)attr;
	struct k_msgq *msg_q = k_calloc(sizeof(struct k_msgq), 1);

	if(NULL == pattr || !msg_q) {
		return QURT_EINVALID;
	}
	
    int ret = k_msgq_alloc_init(msg_q, pattr->element_size, pattr->elements);
    if(ret) {
        k_free(msg_q);
    	return QURT_EFAILED;
    }

    *pipe = msg_q;

    return ret;
}

void qurt_pipe_delete(qurt_pipe_t pipe)
{
	if(NULL != pipe) {
        return ;
    }

    k_msgq_purge(pipe);
    k_msgq_cleanup(pipe);
    k_free(pipe);
}

int qurt_pipe_send_timed(qurt_pipe_t pipe, void *data, qurt_time_t q_timeout)
{
    int ret;
    if (NULL == pipe) {
        return QURT_EINVALID;
    }

    /* send an item over pipe is not supported from ISR */
    ret = k_msgq_put(pipe, (const void *)data, K_TICKS(q_timeout));
    if (ret) {
      return QURT_EFAILED_TIMEOUT;
    }
    return QURT_EOK;
}

void qurt_pipe_send(qurt_pipe_t pipe, void *data)
{
	k_msgq_put(pipe, data, K_NO_WAIT);
}

int qurt_pipe_try_send(qurt_pipe_t pipe, void *data, BaseType_t *timeout)
{
    int ret;
    (void)timeout;

    if (NULL == pipe) {
        return QURT_EINVALID;
    }

    ret = k_msgq_put((struct k_msgq *)pipe, (const void *)data, K_NO_WAIT);
    if (ret) {
        return QURT_EFAILED;
    }

    return ret;
}

int qurt_pipe_receive_timed(qurt_pipe_t pipe, void * const data, qurt_time_t q_timeout)
{
	int ret;

	if( NULL == pipe) {
		return QURT_EINVALID;
	}

	/* receive an item over pipe is not supported from ISR */
	ret = k_msgq_get(pipe, data, K_TICKS(q_timeout));
	if(ret) {
		return QURT_EFAILED_TIMEOUT;
	}

	return QURT_EOK;
}

void qurt_pipe_receive(qurt_pipe_t pipe, void *data)
{
	k_msgq_get(pipe, data, K_FOREVER);
}

int qurt_pipe_try_receive(qurt_pipe_t pipe, void * const data, BaseType_t* q_timeout)
{
    int ret;
    (void)q_timeout;

    if(NULL == pipe) {
    	return QURT_EINVALID;
    }

    ret = k_msgq_get(pipe, data, K_NO_WAIT);
    if(ret) {
    	return QURT_EFAILED;
    }

    return QURT_EOK;
}

QueueHandle_t nt_qurt_pipe_create(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize )
{
	QueueHandle_t p_queue = NULL;
	qurt_pipe_attr_t cmd_msg_queue = {0};

	qurt_pipe_attr_init(&cmd_msg_queue);
	qurt_pipe_attr_set_elements(&cmd_msg_queue, uxQueueLength);
	qurt_pipe_attr_set_element_size(&cmd_msg_queue, uxItemSize);

	qurt_pipe_create(&p_queue, &cmd_msg_queue);

	return p_queue;
}


