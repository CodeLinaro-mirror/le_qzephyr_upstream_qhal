/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __PRINTF_EXT__
#define __PRINTF_EXT__

#include "qdefconfig.h"
#include <zephyr/sys/printk.h>

#if CONFIG_PRINTF_ERR
#define err_printf(msg, ...) printk(CONFIG_ERROR_PREFIX msg, ##__VA_ARGS__)
#else
#define err_printf(args...)                                                                                            \
    do {                                                                                                               \
    } while (0)
#endif

#if CONFIG_PRINTF_WARN
#define warn_printf(msg, ...) printk(CONFIG_WARN_PREFIX msg, ##__VA_ARGS__)
#else
#define warn_printf(args...)                                                                                           \
    do {                                                                                                               \
    } while (0)
#endif

#if CONFIG_PRINTF_INFO
#define info_printf(msg, ...) printk(CONFIG_INFO_PREFIX msg, ##__VA_ARGS__)
#else
#define info_printf(args...)                                                                                           \
    do {                                                                                                               \
    } while (0)
#endif

#if CONFIG_PRINTF_LOG
#define log_printf(msg, ...) printk(CONFIG_LOG_PREFIX msg, ##__VA_ARGS__)
#else
#define log_printf(args...)                                                                                            \
    do {                                                                                                               \
    } while (0)
#endif

#if CONFIG_PRINTF_DUMP
#define dump_printf(msg, ...) printk(CONFIG_DUMP_PREFIX msg, ##__VA_ARGS__)
#else
#define dump_printf(args...)                                                                                           \
    do {                                                                                                               \
    } while (0)
#endif

#define PRINT_ERR_NOT_SUPPORTED err_printf("Not supported yet\n")
#define PRINT_ERR_INVALID_PARAM err_printf("Invalid paramter\n")
#define PRINT_ERR_INVALID_PARAM1(msg, argx) err_printf("Invalid paramter: " msg "=0x%x\n", argx)
#define PRINT_ERR_ALREADY_EXIST err_printf("Already exist\n")
#define PRINT_ERR_WMI_CMD_SEND_FAILED err_printf("WMI command send failed\n")
#define PRINT_ERR_NO_RESOURCE err_printf("No resource\n")

#define PRINT_WARN_SKIP warn_printf("Skip\n")

#define PRINT_LOG_FUNC_LINE log_printf("%s %d\n", __FUNCTION__, __LINE__)
#define PRINT_LOG_FUNC_LINE_ENTRY log_printf("%s %d entry\n", __FUNCTION__, __LINE__)
#define PRINT_LOG_FUNC_LINE_EXIT log_printf("%s %d exit\n", __FUNCTION__, __LINE__)

#endif //__PRINTF_EXT__
