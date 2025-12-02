/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

void qwifi_init(void);

enum {
	Q_LINKCHANGE_ADD,
	Q_LINKCHANGE_REMOVE,
};

typedef qapi_Status_t (*qwifi_drv_eth_rx_cb_t)(void *drv_intf_data, void *bufp, uint16_t len, void *hal_data);
typedef void (*qwifi_link_change_handler)(void *drv_iface, uint32_t event, uint8_t* mac_addr);

qapi_Status_t qwifi_hal_tx(uint8_t device_ID, void *buffer, uint16_t len);

qapi_Status_t qwifi_hal_reg_rxcb(void *drv_intf_data, qwifi_drv_eth_rx_cb_t fn, qwifi_link_change_handler link_fn);
