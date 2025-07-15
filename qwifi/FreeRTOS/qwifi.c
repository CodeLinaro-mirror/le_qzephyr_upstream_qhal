/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
  * SPDX-License-Identifier: BSD-3-Clause*/
#include "autoconf.h"
#include "fwconfig_cmn.h"
#include "nt_flags.h"
#include "nt_osal.h"
#include "nt_common.h"
#include "wmi.h"
#include <stdint.h>

extern uint32_t __OTP_region_st_addr; //0x001a0000
extern uint32_t __OTP_region_end_addr; //0x001a1000, len=0x1000
extern uint32_t __rram_region_start_addr; //0x00200000
extern uint32_t __rram_region_end_address; //0x00380000, len=0x180000
extern uint32_t _ln_REGDB_Start_Addr; //0x0021a600
extern uint32_t _ln_REGDB_Data_length; //0x000036e0
extern uint32_t _ln_CAL_Start_Addr; //0x0021dce0
extern uint32_t _ln_CAL_Data_length; //0x3000
extern uint32_t _ln_RAM_start_addr_hw_desc__; //base address for hardware descriptors, 0x0002f780
extern uint32_t _ln_RAM_end_addr_hw_desc__; //0x00034f68, len=0x57e8
extern uint32_t _ln_RAM_start_addr_hw_pktmem__; //base address for packet memory, 0x00027780
extern uint32_t _ln_RAM_end_addr_hw_pktmem__; //0x0002f780, len=0x8000

uint32_t g__OTP_region_st_addr = (uint32_t)&__OTP_region_st_addr; //0x001a0000
uint32_t g__OTP_region_end_addr = (uint32_t)&__OTP_region_end_addr; //0x001a1000, len=0x1000
uint32_t g__rram_region_start_addr = (uint32_t)&__rram_region_start_addr; //0x00200000
uint32_t g__rram_region_end_address = (uint32_t)&__rram_region_end_address; //0x00380000, len=0x180000
uint32_t g_ln_REGDB_Start_Addr = (uint32_t)&_ln_REGDB_Start_Addr; //0x0021a600
uint32_t g_ln_REGDB_Data_length = (uint32_t)&_ln_REGDB_Data_length; //0x000036e0
uint32_t g_ln_CAL_Start_Addr = (uint32_t)&_ln_CAL_Start_Addr; //0x0021dce0
uint32_t g_ln_CAL_Data_length = (uint32_t)&_ln_CAL_Data_length; //0x3000
uint32_t g_ln_RAM_start_addr_hw_desc__ = (uint32_t)&_ln_RAM_start_addr_hw_desc__; //base address for hardware descriptors, 0x0002f780
uint32_t g_ln_RAM_end_addr_hw_desc__ = (uint32_t)&_ln_RAM_end_addr_hw_desc__; //0x00034f68, len=0x57e8
uint32_t g_ln_RAM_start_addr_hw_pktmem__ = (uint32_t)&_ln_RAM_start_addr_hw_pktmem__; //base address for packet memory, 0x00027780
uint32_t g_ln_RAM_end_addr_hw_pktmem__ = (uint32_t)&_ln_RAM_end_addr_hw_pktmem__; //0x0002f780, len=0x8000

qapi_Status_t qwifi_hal_tx(uint8_t device_ID, void *buffer, uint16_t len)
{
	nt_status_t err;
	void *buf = NULL;
	uint8_t *buf_ptr;

	buf = nt_dpm_allocate_buffer((uint32_t)(NT_TX_BUFFER_OFFSET + len));
	buf_ptr = (uint8_t *)buf + NT_TX_BUFFER_OFFSET;
	nt_dpm_memcpy(buf_ptr, buffer, len);
	err = nt_dpm_process_eth_packet_from_stack(buf, len);
	if (err != NT_OK ) {
		nt_dpm_free_buffer(buf);
		if (err == NT_ECONN) {
			return QAPI_ERR_NO_ENTRY;														/* using LWIP ERROR constant to notify LWIP stack for state of connection */
		} else if ((err == NT_ENOMEM) || (err == NT_ENORES) || (err == NT_ETXFAIL)) {
			return QAPI_ERR_NO_MEMORY;															/* using LWIP ERROR constant to notify LWIP stack for error in transmitting */
		} else {
			return QAPI_ERROR; //TODO: any error log should be added
		}
	}
	return QAPI_OK;
}

struct qwifi_hal_t {
	void *drv_intf_data;
	qwifi_drv_eth_rx_cb_t rx_cb;
};

struct qwifi_hal_t gs_qwifi_hal;

qapi_Status_t qwifi_hal_reg_rxcb(void *drv_intf_data, qwifi_drv_eth_rx_cb_t fn)
{
	struct qwifi_hal_t *hal = &gs_qwifi_hal;

	hal->drv_intf_data = drv_intf_data;
	hal->rx_cb = fn;
	return QAPI_OK;
}

uint8_t nt_dpm_forward_eth_packet_to_stack(p_ndpA ad, void *rx_frame, void *eth_frame, uint32_t length, device_t *dev)
{
	struct qwifi_hal_t *hal = &gs_qwifi_hal;

	(void)ad;
	(void)dev;

	hal->rx_cb(hal->drv_intf_data, eth_frame, length, hal);
	nt_dpm_free_buffer(rx_frame);
	return QAPI_OK;
}

struct libwifi_kconfig_t{
    uint32_t hc_11a_0_2g;
    uint32_t hc_11a_1_2g;
    uint32_t hc_11n_2g;
    uint32_t hc_11a_0_5g;
    uint32_t hc_11a_1_5g;
    uint32_t hc_11n_5g;
    uint8_t srrc_band_edge_enable;
};

struct libwifi_qos_null_kconfig_t{
    uint8_t enable;
    uint8_t retry_count;
    uint16_t socmp_nop_delay;
};

struct libwifi_kconfig_t g_libwifi_kconfig;
struct libwifi_qos_null_kconfig_t g_libwifi_qos_null_kconfig_t;


void libwifi_kconfig_install(void)
{
#ifdef CONFIG_HEAVY_CLIP_LEVEL_2G
    g_libwifi_kconfig.hc_11a_0_2g = CONFIG_PEAKW_11A_0_2G;
    g_libwifi_kconfig.hc_11a_1_2g = CONFIG_PEAKW_11A_1_2G;
    g_libwifi_kconfig.hc_11n_2g = CONFIG_PEAKW_11N_0_2G;
#else
    g_libwifi_kconfig.hc_11a_0_2g = 0x564E4739;
    g_libwifi_kconfig.hc_11a_1_2g = 0x5D;
    g_libwifi_kconfig.hc_11n_2g = 0x5D47473C;
#endif

#ifdef CONFIG_HEAVY_CLIP_LEVEL_5G
    g_libwifi_kconfig.hc_11a_0_5g = CONFIG_PEAKW_11A_0_5G;
    g_libwifi_kconfig.hc_11a_1_5g = CONFIG_PEAKW_11A_1_5G;
    g_libwifi_kconfig.hc_11n_5g = CONFIG_PEAKW_11N_0_5G;
#else
    g_libwifi_kconfig.hc_11a_0_5g = 0x60523F3F;
    g_libwifi_kconfig.hc_11a_1_5g = 0x62;
    g_libwifi_kconfig.hc_11n_5g = 0x62514D3F;
#endif

#ifdef CONFIG_ACK_TIMEOUT_MODIFY_ENABLE
    g_libwifi_qos_null_kconfig_t.enable = TRUE;
#else
    g_libwifi_qos_null_kconfig_t.enable = FALSE;
#endif
    g_libwifi_qos_null_kconfig_t.retry_count = CONFIG_QOS_NULL_DATA_MAX_RETRY_COUNT;
    g_libwifi_qos_null_kconfig_t.socmp_nop_delay = CONFIG_QOS_NULL_DATA_RETRY_DELAY;

#ifdef CONFIG_SRRC_BAND_EDGE_SUPPORT
    g_libwifi_kconfig.srrc_band_edge_enable = TRUE;
#else
    g_libwifi_kconfig.srrc_band_edge_enable = FALSE;
#endif
}

NT_BOOL wmi_pdev_utf_cmd(wmi_msg_struct_t* msg)
{
#if CONFIG_FTM_MODE
    extern uint8_t ftm_parse_tlv_cmd(uint8_t * buf, uint32_t dataLength);
    ftm_parse_tlv_cmd((uint8_t*)msg->msg_struct.vo_data, msg->msg_struct.vo_data_len);
#else /* CONFIG_FTM_MODE */
    (void)msg;
#endif /* CONFIG_FTM_MODE */
    return TRUE;
}
void wmi_unit_test_cmd_handler(WMI_UNIT_TEST_CMD *cmd)
{
#ifdef UNIT_TEST_SUPPORT
    extern void wmi_unit_test_internal_cmd_handler(WMI_UNIT_TEST_CMD *cmd);
    wmi_unit_test_internal_cmd_handler(cmd);
#else /* UNIT_TEST_SUPPORT */
    (void)cmd;
#endif /* UNIT_TEST_SUPPORT */
    return;
}
