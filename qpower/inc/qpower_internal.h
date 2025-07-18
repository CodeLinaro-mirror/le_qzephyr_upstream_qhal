/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

/**
 * @file qpower.h
 * @brief System/hardware module for QCC730 processor
 * @version 0.1
 * @date 2025-06-18
 * 
 * 
 */

#pragma once

typedef struct {
    uint32_t softoff_duration_ms;
    uint32_t softoff_wakeup_src;
    uint32_t s2ram_duration_ms;
    uint32_t s2ram_wakeup_src;
} qpower_param_t;

extern qpower_param_t gs_qpower_param;

