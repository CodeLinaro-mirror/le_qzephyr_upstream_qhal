/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#pragma once

#include <stdint.h>

struct qwifi_wlan_lib_version {
    uint32_t major;
    uint32_t minor;
    uint32_t patch;
    uint32_t build;
};

/**
 * qwifi_get_wlan_lib_version - Populate @ver with the compile-time WLAN lib version.
 *
 * Returns 0 on success.  @ver must not be NULL.
 */
int qwifi_get_wlan_lib_version(struct qwifi_wlan_lib_version *ver);
