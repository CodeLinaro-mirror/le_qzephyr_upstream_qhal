/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear*/
#include <libwifi.h>
#include <qwifi_api.h>
#include "qwifi_internal.h"
#include <zephyr/kernel.h>
#include <zephyr/net/net_if.h>

#include "uart_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <nt_logger_api.h>

#include <zephyr/sys/printk.h>

extern struct qwifi_hal_t gs_qwifi_hal[2];

enum {
	STA_DEVICE,
	AP_DEVICE,
	MAX_ROLE,
};

#define HAL_MAX_RX_RATEINDEX    (32)
typedef struct qintf_device {
	uint8_t role;
#ifdef LWIP
	struct netif *netif;
#endif
	uint8_t mac_address[NT_MAC_ADDR_SIZE];
	uint8_t wmi_devid;
	uint8_t occupied;
	uint8_t dev_id;
	uint8_t ip_address[NT_IPV4_ADDR_SIZE];

	/* list of sta entries */
	void *dev_sta;

    uint32_t  rx_rate_index_counter[HAL_MAX_RX_RATEINDEX];
} device_t;


// API called by libwifi
// Control if ftm code is linked in, so to reduce code size
NT_BOOL wmi_pdev_utf_cmd(wmi_msg_struct_t *msg)
{
#if CONFIG_FTM_MODE
    extern uint8_t ftm_parse_tlv_cmd(uint8_t * buf, uint32_t dataLength);
    ftm_parse_tlv_cmd((uint8_t *)msg->msg_struct.vo_data, msg->msg_struct.vo_data_len);
#else  /* CONFIG_FTM_MODE */
    (void)msg;
#endif /* CONFIG_FTM_MODE */
    return TRUE;
}

// Control if unit test code is linked in, so to reduce code size
void wmi_unit_test_cmd_handler(WMI_UNIT_TEST_CMD *cmd)
{
#ifdef UNIT_TEST_SUPPORT
    extern void wmi_unit_test_internal_cmd_handler(WMI_UNIT_TEST_CMD * cmd);
    wmi_unit_test_internal_cmd_handler(cmd);
#else  /* UNIT_TEST_SUPPORT */
    (void)cmd;
#endif /* UNIT_TEST_SUPPORT */
    return;
}

nt_status_t nt_dpm_forward_eth_packet_to_stack_ext(void *rx_buf, void *eth_frame, uint32_t length, void *ext)
{
    struct qwifi_hal_t *hal;

    (void)ext;
    device_t *device = ext;

    if (device && (device->role == STA_DEVICE)) {
        hal = &gs_qwifi_hal[0];
    } else {
        hal = &gs_qwifi_hal[1];
    }

    hal->rx_cb(hal->drv_intf_data, eth_frame, length, hal);
    nt_dpm_free_buffer_ext(rx_buf);
    return NT_OK;
}

void nt_dpm_network_init(void) {}

void nt_dpm_add_dev_to_stack(void *dev)
{
    device_t *device = dev;
    struct qwifi_hal_t *hal;

    if (device && (device->role == STA_DEVICE)) {
        hal = &gs_qwifi_hal[0];
    } else {
        hal = &gs_qwifi_hal[1];
    }

    hal->link_change(hal->drv_intf_data, Q_LINKCHANGE_ADD, device->mac_address);
}

void nt_dpm_remove_dev_from_stack(void *dev)
{
    device_t *device = dev;
    struct qwifi_hal_t *hal;

    if (device && (device->role == STA_DEVICE)) {
        hal = &gs_qwifi_hal[0];
    } else {
        hal = &gs_qwifi_hal[1];
    }

    hal->link_change(hal->drv_intf_data, Q_LINKCHANGE_REMOVE, device->mac_address);
}

void *nt_dpm_allocate_network_buffer_pool(uint32_t length) { return k_malloc(length); }

void *nt_dpm_allocate_network_buffer(uint32_t length) { return k_malloc(length); }

void nt_dpm_free_network_buffer(void *buf) { k_free(buf); }

// to-do
void nt_dpm_realloc_network_buffer(void *buf, uint32_t length)
{
    (void)buf;
    (void)length;
}

#if QCCSDK
void nt_dpm_notify_network_to_set_linkup(struct netif *netif)
{
    nt_dpm_netif_set_link_up(netif);
    return;
}

void nt_dpm_notify_network_to_set_linkdown(struct netif *netif)
{
    nt_dpm_netif_set_link_down(netif);
    return;
}
#endif

nt_status_t get_netif_hwaddr_from_netif_id(uint8_t netif_id, uint8_t *addr) { return NT_OK; }

app_mode_id_t nt_get_app_mode(void) { 
#if CONFIG_FTM_MODE
    return APP_MODE_FTM;
#else
    return APP_MODE_MM; 
#endif
}

