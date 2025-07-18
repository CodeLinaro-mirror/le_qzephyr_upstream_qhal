/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
*/

#include <qwifi_api.h>
#include "qwifi_internal.h"

#include "wlan_drv.h"
#include "libwifi.h"

#include <zephyr/logging/log.h>
#include "printfext.h"

LOG_MODULE_DECLARE(soc, CONFIG_SOC_LOG_LEVEL);

#if CONFIG_LIBWIFIQCC730_SRC

//reference: BACKUP_SRAM

//OTP total LEN=0X1000
#define   __OTP_REGION_START_ADDR 0X001A0000
#define   __OTP_REGION_END_ADDR 0X001A1000

//RRAM total LEN=0X180000
#define   __RRAM_REGION_START_ADDR 0X00200000
#define   __RRAM_REGION_END_ADDRESS 0X00380000

//RRAM from end to start
#define BDF_LEN 0x6000
#define BDF_END_ADDR __RRAM_REGION_END_ADDRESS
#define BDF_START_ADDR (BDF_END_ADDR - BDF_LEN) //0x37a000

#define   _LN_CAL_DATA_LENGTH 0x3000
#define   _LN_CAL_END_ADDR BDF_START_ADDR
#define   _LN_CAL_START_ADDR (_LN_CAL_END_ADDR - _LN_CAL_DATA_LENGTH) //0x377000

#define   _LN_REGDB_DATA_LENGTH 0x4000 //read len 0X000036E0
#define   _LN_REGDB_END_ADDR _LN_CAL_START_ADDR
#define   _LN_REGDB_START_ADDR (_LN_REGDB_END_ADDR - _LN_REGDB_DATA_LENGTH) //0x373000

//SRAM total LEN=0xA0000
#define SRAM_END_ADDR 0xA0000

//SRAM from end to start
#define _LN_RAM_HW_DESC__LEN 0x6000  //real len 0X57E8
#define   _LN_RAM_END_ADDR_HW_DESC_ SRAM_END_ADDR
#define   _LN_RAM_START_ADDR_HW_DESC__ (_LN_RAM_END_ADDR_HW_DESC_ - _LN_RAM_HW_DESC__LEN)

#define  _LN_RAM_HW_PKTMEM__LEN 0X8000
#define   _LN_RAM_END_ADDR_HW_PKTMEM__ _LN_RAM_START_ADDR_HW_DESC__
#define  _LN_RAM_START_ADDR_HW_PKTMEM__ (_LN_RAM_END_ADDR_HW_PKTMEM__ - _LN_RAM_HW_PKTMEM__LEN)

const uint32_t g__OTP_region_st_addr = __OTP_REGION_START_ADDR; //0x001a0000
const uint32_t g__OTP_region_end_addr = __OTP_REGION_END_ADDR; //0x001a1000, len=0x1000
const uint32_t g__rram_region_start_addr = __RRAM_REGION_START_ADDR; //0x00200000
const uint32_t g__rram_region_end_address = __RRAM_REGION_END_ADDRESS; //0x00380000, len=0x180000
const uint32_t g_ln_REGDB_Start_Addr = _LN_REGDB_START_ADDR; //0x0021a600
const uint32_t g_ln_REGDB_Data_length = _LN_REGDB_DATA_LENGTH; //0x000036e0
const uint32_t g_ln_CAL_Start_Addr = _LN_CAL_START_ADDR; //0x0021dce0
const uint32_t g_ln_CAL_Data_length = _LN_CAL_DATA_LENGTH; //0x3000
const uint32_t g_ln_RAM_start_addr_hw_desc__ = _LN_RAM_START_ADDR_HW_DESC__; //base address for hardware descriptors, 0x0002f780
const uint32_t g_ln_RAM_end_addr_hw_desc__ = _LN_RAM_END_ADDR_HW_DESC_; //0x00034f68, len=0x57e8
const uint32_t g_ln_RAM_start_addr_hw_pktmem__ = _LN_RAM_START_ADDR_HW_PKTMEM__; //base address for packet memory, 0x00027780
const uint32_t g_ln_RAM_end_addr_hw_pktmem__ = _LN_RAM_END_ADDR_HW_PKTMEM__; //0x0002f780, len=0x8000

struct libwifi_kconfig_t g_libwifi_kconfig;
struct libwifi_qos_null_kconfig_t g_libwifi_qos_null_kconfig_t;

static void libwifi_kconfig_install(void)
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
#endif

void qwifi_init (void)
{
    PRINT_LOG_FUNC_LINE_ENTRY;

#if CONFIG_LIBWIFIQCC730_SRC
    uint32_t is_ftm = 0;

#if CONFIG_FTM_MODE
    ftm_task_init();
    is_ftm = 1;
#endif
    PRINT_LOG_FUNC_LINE;

    wlan_init_early1();
    PRINT_LOG_FUNC_LINE;

    libwifi_kconfig_install();
    PRINT_LOG_FUNC_LINE;

    //TODO: uint32_t *bdf_addr, uint32_t *regdb_addr
    wlan_init_early2(is_ftm, BDF_START_ADDR, g_ln_REGDB_Start_Addr);
    PRINT_LOG_FUNC_LINE;

#ifdef PHY_POWER_SWITCH
    /* In FTM configure PHY in RXB_LISTEN mode */
#if CONFIG_FTM_MODE
    hal_phy_power_ftm_switch_to_listen();
#endif
#endif /* PHY_POWER_SWITCH */
    PRINT_LOG_FUNC_LINE;
#endif

    wlan_qapi_init();
    PRINT_LOG_FUNC_LINE_EXIT;
}

qapi_Status_t qwifi_hal_tx(uint8_t device_ID, void *buffer, uint16_t len)
{
    nt_status_t err;
    uint8_t *buf_ptr = NULL;
    qapi_Status_t ret = QAPI_OK;

    PRINT_LOG_FUNC_LINE_ENTRY;
    buf_ptr = nt_dpm_allocate_buffer_ext((uint32_t)len);
    nt_dpm_memcpy(buf_ptr, buffer, len);
    err = nt_dpm_process_eth_packet_from_stack_ext(buf_ptr, len);
    if (err != NT_OK ) {
        nt_dpm_free_buffer_ext(buf_ptr);
        if (err == NT_ECONN) {
            ret = QAPI_ERR_NO_ENTRY;                                                       /* using LWIP ERROR constant to notify LWIP stack for state of connection */
        } else if ((err == NT_ENOMEM) || (err == NT_ENORES) || (err == NT_ETXFAIL)) {
            ret = QAPI_ERR_NO_MEMORY;                                                          /* using LWIP ERROR constant to notify LWIP stack for error in transmitting */
        } else {
            ret = QAPI_ERROR; //TODO: any error log should be added
        }
    }
    PRINT_LOG_FUNC_LINE_EXIT;
    return ret;
}

struct qwifi_hal_t gs_qwifi_hal;

qapi_Status_t qwifi_hal_reg_rxcb(void *drv_intf_data, qwifi_drv_eth_rx_cb_t fn)
{
    struct qwifi_hal_t *hal = &gs_qwifi_hal;

    PRINT_LOG_FUNC_LINE_ENTRY;
    hal->drv_intf_data = drv_intf_data;
    hal->rx_cb = fn;
    PRINT_LOG_FUNC_LINE_EXIT;
    return QAPI_OK;
}


