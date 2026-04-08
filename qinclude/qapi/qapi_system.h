/*
#Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/**
 * @file qapi_system.h
 *
 */

#ifndef __QAPI_SYSTEM_H__
#define __QAPI_SYSTEM_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "qapi_status.h"

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
/*-------------------------------------------------------------------------
 * Boot reason types
 *-----------------------------------------------------------------------*/
typedef enum
{
    QAPI_BOOT_REASON_COLD_BOOT = 0,
    QAPI_BOOT_REASON_DTIM_SLEEP = 1,
    QAPI_BOOT_REASON_DEEP_SLEEP = 2,
    QAPI_BOOT_REASON_UNKNOWN = 3
} qapi_boot_reason_t;


/*-------------------------------------------------------------------------
 * Function Declarations
 *-----------------------------------------------------------------------*/


/**
 *  Gets boot reason.
 *
 * @param[out] reason  Pointer to a qapi_boot_reason_t to receive the boot reason.
 *
 * @return #QAPI_OK on success, or #QAPI_ERROR if reason is NULL.
 */
qapi_Status_t qapi_core_obtain_boot_reason(qapi_boot_reason_t *reason);

#endif /* __QAPI_SYSTEM_H__ */
