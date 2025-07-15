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
#ifndef _QPOWER_H_
#define _QPOWER_H_

typedef enum {
    WKUP_AON_TIMER = 1,
    WKUP_EXT_PIN = 2,
    WKUP_UNKNOWN = 3,
} wkup_src_t;

void config_deepsleep(int wkup_src, int slp_time);
void deepsleep(void);
void mcusleep(void);

#endif /* _QPOWER_H_ */
