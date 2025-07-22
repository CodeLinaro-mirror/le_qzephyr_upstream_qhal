/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*=============================================================================
    File: qal_memory.c
=============================================================================*/

#include "stdlib.h"
#include <zephyr/kernel.h>
#include <zephyr/cache.h>
#include "qurt_mem.h"

int qurt_mem_cache_clean(qurt_addr_t addr, qurt_size_t size, qurt_mem_cache_op_t opcode, qurt_mem_cache_type_t type)
{

    return QURT_EOK;
}
