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
/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "wmi.h"

#pragma once

typedef struct {
    uint32_t softoff_duration_ms;
    uint32_t softoff_wakeup_src;
    uint32_t s2ram_duration_ms;
    uint32_t s2ram_wakeup_src;
} qpower_param_t;

extern qpower_param_t gs_qpower_param;

/**
Data structure used by the api layer to pass lowpower configurations to the driver.
*/
typedef union {
    WMI_IMPS_CFG imps_cfg;
    /**< IMPS cfg, used in qapi_imps_cfg. */
    struct {
        WMI_BMPS_IDLE_TIME bmps_idle_time;
        /**< The idle timeout in ms, used in qapi_bmps_cfg. */
        WMI_BMPS_ENABLE bmps_enable;
        /**< To enable/disable BMPS, used in qapi_bmps_cfg. */
        WMI_BMPS_PWR_OPT_ENABLE bmps_pwr_opt_enable;
        /**< To enable/disable BMPS power optimization. */
        WMI_BMPS_CMPR_QOS_NULL_ENABLE bmps_cmpr_qos_null_enable;
        /**< To enable/disable compressing qos-null sending */
    } bmps_cfg;
    /**< BMPS cfg, used in qapi_bmps_cfg. */
    WMI_BMPS_IGNORE_BCMC bmps_ignore_bcmc;
    /**< To config ignore group-cast traffic during BMPS. */
    WMI_BMPS_TIMING_CFG bmps_timing;
    /**< Internal timing parameters in BMPS. */
    WMI_SLP_CLK_CAL_CFG slp_clk_cal;
    /**< Enable/disable 32k clock calibration in sleep mode. */
    WMI_SLP_CLK_CAL_ACT slp_clk_cal_act;
    /**< Enable/disable 32k clock calibration in active mode. */
    uint32_t force_dtim;
    /**< Force dtim period */
} lpr_wmi_t;
