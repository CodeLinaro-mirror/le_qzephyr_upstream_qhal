/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QURT_TYPES_H
#define QURT_TYPES_H

#include <stdint.h>
#include <stddef.h>

/*
 */

/**

@file qurt_types.h

@brief  definition of basic types, constants, preprocessor macros
*/

// #include "qurt_stddef.h"

/*=============================================================================
                        CONSTANTS AND MACROS
=============================================================================*/
/** @addtogroup qurt_types
@{ */

#define QURT_TIME_NO_WAIT 0x00000000      /**< Return immediately without any waiting. */
#define QURT_TIME_WAIT_FOREVER 0xFFFFFFFF /**< Block until the operation is successful. */

#define portMAX_DELAY (TickType_t)0xffffffffUL

/*=============================================================================
                        TYPEDEFS
=============================================================================*/

/** QuRT time types. */
typedef uint32_t qurt_time_t;

typedef uint32_t TickType_t;

typedef long BaseType_t;
typedef unsigned long UBaseType_t;

/** QuRT time unit types. */

/*===============================================================
                        Standard Types
 ================================================================*/

/* The following definitions are the same across platforms */
#ifndef _ARM_ASM
#ifndef _BOOLEAN_DEFINED
typedef unsigned char boolean; /* Boolean value type */
#define _BOOLEAN_DEFINED
#endif

#ifndef _UINT32_DEFINED
typedef unsigned long int uint32; // Unsigned 32 bit value
#define _UINT32_DEFINED
#endif

#ifndef _UINT16_DEFINED
typedef unsigned short uint16; // Unsigned 16 bit value
#define _UINT16_DEFINED
#endif

#ifndef _UINT8_DEFINED
typedef unsigned char uint8; // Unsigned 8 bit value
#define _UINT8_DEFINED
#endif

#ifndef _INT32_DEFINED
typedef signed long int int32; // signed 32 bit value
#define _INT32_DEFINED
#endif

#ifndef _INT16_DEFINED
typedef signed short int16; // signed 16 bit value
#define _INT16_DEFINED
#endif

#ifndef _INT8_DEFINED
typedef signed char int8; // signed 8 bit value
#define _INT8_DEFINED
#endif

#ifndef _INT64_DEFINED
typedef long long int64;
#define _INT64_DEFINED
#endif

#ifndef _UINT64_DEFINED
typedef unsigned long long uint64;
#define _UINT64_DEFINED
#endif

#endif /* ARM_ASM */

/** @} */ /* end_addtogroup qurt_types */
#endif    /* QURT_TYPES_H */
