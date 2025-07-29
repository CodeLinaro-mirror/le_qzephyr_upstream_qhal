/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause*/
#include <libwifi.h>
#include <qwifi_api.h>
#include "qwifi_internal.h"
#include <zephyr/kernel.h>

#include "uart_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <nt_logger_api.h>

#include <zephyr/sys/printk.h>

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
    struct qwifi_hal_t *hal = &gs_qwifi_hal;

    (void)ext;

    hal->rx_cb(hal->drv_intf_data, eth_frame, length, hal);
    nt_dpm_free_buffer_ext(rx_buf);
    return NT_OK;
}

void nt_dpm_network_init(void) {}

void nt_dpm_add_dev_to_stack(void *dev) { (void)dev; }

void nt_dpm_remove_dev_from_stack(void *dev) { (void)dev; }

void nt_dpm_stop_network_stack(void) {}

void nt_dpm_start_network_stack(void) {}

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

app_mode_id_t nt_get_app_mode(void) { return APP_MODE_MM; }

int32_t pmu_ts_get_current_temperature(void)
{
    // room temperature in degree
    return 25;
}

