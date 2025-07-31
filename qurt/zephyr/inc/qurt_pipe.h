/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include "qurt_types.h"

#define QURT_PIPE_ATTR_OBJ_SIZE_BYTES 16

/* Represents pipes  */
struct qurt_pipe;
typedef struct qurt_pipe *qurt_pipe_t;

/* Represents pipe attributes */
typedef struct qurt_pipe_attr /* 8 byte aligned */
{
    unsigned long long _bSpace[QURT_PIPE_ATTR_OBJ_SIZE_BYTES / sizeof(unsigned long long)];
} qurt_pipe_attr_t;

void qurt_pipe_attr_init(qurt_pipe_attr_t *attr);
void qurt_pipe_attr_set_elements(qurt_pipe_attr_t *attr, uint32 elements);
void qurt_pipe_attr_set_element_size(qurt_pipe_attr_t *attr, uint32 element_size);
int qurt_pipe_create(qurt_pipe_t *pipe, qurt_pipe_attr_t *attr);
void qurt_pipe_delete(qurt_pipe_t pipe);
int qurt_pipe_send_timed(qurt_pipe_t pipe, void *data, qurt_time_t timeout);
void qurt_pipe_send(qurt_pipe_t pipe, void *data);
int qurt_pipe_try_send(qurt_pipe_t pipe, void *data, BaseType_t *timeout);
int qurt_pipe_receive_timed(qurt_pipe_t pipe, void *const data, qurt_time_t timeout);
void qurt_pipe_receive(qurt_pipe_t pipe, void *data);
int qurt_pipe_try_receive(qurt_pipe_t pipe, void *const data, BaseType_t *timeout);
int qurt_pipe_flush(qurt_pipe_t pipe);

typedef qurt_pipe_t QueueHandle_t;
QueueHandle_t nt_qurt_pipe_create(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize);
