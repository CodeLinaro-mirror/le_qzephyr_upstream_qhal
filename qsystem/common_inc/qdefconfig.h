/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#define FERMION_CHIP_VERSION 2

// only FTM app set 1
// #define CONFIG_FTM_MODE 0

#define CONFIG_PLATFORM_QAPI 1
#define CONFIG_WFM_EN 1
#define CONFIG_6GHZ 1
#define CONFIG_WIFILIB_6GHZ 1
#define CONFIG_ACK_TIMEOUT_MODIFY_ENABLE 1
#define CONFIG_SRRC_BAND_EDGE_SUPPORT 1

#define CONFIG_PBL_PREES_RESET_FOR_DTIM 1
#define CONFIG_PBL_PREES_GPIO_RESTORE 1

#define SCALE_INCLUDES
#define IMAGE_FERMION
#define DFU_BUILD
#define PLATFORM_FERMION
#define PHYDEVLIB_PRODUCT_FERMION
#define FERMION_SILICON
#define FEMION_IOT
#define SILICON_BUILD
#define USE_FERMION_HALMAC
#define NT_DEBUG
#define DEBUG
#define CHIP_HALPHY_FULL_DEBUG

#define CONFIG_PRINTF_ERR 1
#define CONFIG_PRINTF_WARN 1
#define CONFIG_PRINTF_INFO 1
#define CONFIG_PRINTF_LOG 1
#define CONFIG_PRINTF_DUMP 0

#define CONFIG_ERROR_PREFIX "[ERROR] "
#define CONFIG_WARN_PREFIX "[WARNING] "
#define CONFIG_INFO_PREFIX "[INFO] "
#define CONFIG_LOG_PREFIX "[LOG] "
#define CONFIG_DUMP_PREFIX "[DUMP] "

#define CONFIG_QOS_NULL_DATA_MAX_RETRY_COUNT 0x10
#define CONFIG_QOS_NULL_DATA_RETRY_DELAY 0x3000

#define CONFIG_OMAC1_AES_128
#define CONFIG_PBKDF2_SHA1

// for qcc730evbx
#define CONFIG_BOARD_QCC730_GPIO_DEFAULT_PU_STATE 0xAAB0A
#define CONFIG_BOARD_QCC730_GPIO_DEFAULT_PD_STATE 0x1554F0

/* The below will enable the functionality related to cMEM minimal build and it
 * will disable the PBL as well as SBL functionality.
 * */
// #define NT_CMEM_BUILD
#ifndef NT_CHIP_VERSION
#define NT_CHIP_VERSION 1
#endif

// #define TEST_NT_DPM_SEND_AMSDU_PKT

// for FTM
// #define FTM_OVER_UART

// #define WMI_WLAN_TARGET_RESET_CMDID_ENABLE

#include "fwconfig_wlan.h"

#include "wifi_cmn.h"

#include "nt_flags.h"
