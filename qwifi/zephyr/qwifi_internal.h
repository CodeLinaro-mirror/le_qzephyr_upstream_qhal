/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#pragma once

#include "qwifi.h"

struct qwifi_hal_t {
    void *drv_intf_data;
    qwifi_drv_eth_rx_cb_t rx_cb;
    qwifi_link_change_handler link_change;
};

extern struct qwifi_hal_t gs_qwifi_hal[2];
