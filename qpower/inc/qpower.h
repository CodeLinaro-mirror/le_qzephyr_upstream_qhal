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

#include "qapi_status.h"

typedef enum {
    WKUP_AON_TIMER = 1,
    WKUP_EXT_PIN = 2,
    WKUP_UNKNOWN = 3,
} wkup_src_t;

#ifndef BIT
#define BIT(x) (1<<(x))
#endif

#define DEFAULT_SOFTOFF_DURATION_MS 5000
#define DEFAULT_SOFTOFF_WAKEUP_SRC (BIT(WKUP_AON_TIMER) | BIT(WKUP_EXT_PIN))
#define DEFAULT_S2RAM_DURATION_MS 5000
#define DEFAULT_S2RAM_WAKEUP_SRC (BIT(WKUP_AON_TIMER) | BIT(WKUP_EXT_PIN))

#define __QAPI_POWER_SOFTOFF_DURATION_MS 1
#define __QAPI_POWER_SUSPEND2RAM_DURATION_MS 2

qapi_Status_t qapi_pmu_init(void);
qapi_Status_t qapi_power_set_parameter(uint32_t type, uint32_t val);
qapi_Status_t qapi_power_get_parameter(uint32_t type, uint32_t *val);
void qapi_enter_softoff(void);
void qapi_enter_suspend2ram(void);
void qapi_suspend2ram_exit_post_ops(void);
void qapi_slp_tmr_set(uint64_t);
void set_sleep_exit_reason(void *reason);
