/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QAL_MEMORY_H
#define QAL_MEMORY_H

#include <qurt_error.h>

typedef unsigned int qurt_addr_t; /**< QuRT address type.*/
typedef unsigned int qurt_paddr_t;
typedef unsigned int qurt_size_t; /**< QuRT size type. */

/** QuRT cache operation code type. */
typedef enum {
    QURT_MEM_CACHE_FLUSH,                  /**< Flush. */
    QURT_MEM_CACHE_INVALIDATE,             /**< Invalidate */
    QURT_MEM_CACHE_FLUSH_INVALIDATE,       /**< Flush invalidate. */
    QURT_MEM_CACHE_FLUSH_ALL,              /**< Flush all. */
    QURT_MEM_CACHE_FLUSH_INVALIDATE_ALL,   /**< Flush invalidate all. */
    QURT_MEM_CACHE_TABLE_FLUSH_INVALIDATE, /**< Table flush invalidate. */
    QURT_MEM_CACHE_FLUSH_INVALIDATE_L2,    /**< */
} qurt_mem_cache_op_t;

/** @cond rest_reg_dist*/
/** QuRT cache type; specifies data cache or instruction cache. */
typedef enum {
    QURT_MEM_ICACHE, /**< Instruction cache.*/
    QURT_MEM_DCACHE  /**< Data cache.*/
} qurt_mem_cache_type_t;

/*=====================================================================
 Functions
======================================================================*/

/**@ingroup func_qurt_mem_cache_clean
  * -- Not Supported yet -- *

  Performs a cache clean operation on the data stored in the specified memory area.
  Peforms a syncht on all the data cache operations when the Hexagon processor version is V60 or greater.
  @note1hang Perform the flush all operation only on the data cache.
  @note1cont This operation flushes and invalidates the contents of all cache lines from start address
             to end address (start address + size). The contents of the adjoining buffer can be
             flushed and invalidated if it falls in any of the cache line.
  @datatypes
  #qurt_addr_t \n
  #qurt_size_t \n
  #qurt_mem_cache_op_t \n
  #qurt_mem_cache_type_t
  @param[in] addr      Address of data to flush.
  @param[in] size      Size (in bytes) of data to flush.
  @param[in] opcode    Type of cache clean operation. Values:
                       - #QURT_MEM_CACHE_FLUSH
                       - #QURT_MEM_CACHE_INVALIDATE
                       - #QURT_MEM_CACHE_FLUSH_INVALIDATE
                       - #QURT_MEM_CACHE_FLUSH_ALL\n
                       @note1 #QURT_MEM_CACHE_FLUSH_ALL is valid only when the type is #QURT_MEM_DCACHE @tablebulletend
  @param[in] type          Cache type. Values:
                       - #QURT_MEM_ICACHE
                       - #QURT_MEM_DCACHE  @tablebulletend

  @return
  #QURT_EOK -- Cache operation performed successfully.\n
  #QURT_EVAL -- Invalid cache type.\n
  # -ENOTSUP  -- Not Supported
  # -errno -- negative error numbers for failures
  @dependencies
  None.
*/
int qurt_mem_cache_clean(qurt_addr_t addr, qurt_size_t size, qurt_mem_cache_op_t opcode, qurt_mem_cache_type_t type);

static inline void qurt_mem_barrier(void) { ; }

#endif
