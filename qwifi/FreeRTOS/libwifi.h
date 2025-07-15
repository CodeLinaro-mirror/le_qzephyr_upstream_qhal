/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/


#pragma once

#include <nt_common.h>
#include <wmi.h>

extern uint32_t g__OTP_region_st_addr; //0x001a0000
extern uint32_t g__OTP_region_end_addr; //0x001a1000, len=0x1000
extern uint32_t g__rram_region_start_addr; //0x00200000
extern uint32_t g__rram_region_end_address; //0x00380000, len=0x180000
extern uint32_t g_ln_REGDB_Start_Addr; //0x0021a600
extern uint32_t g_ln_REGDB_Data_length; //0x000036e0
extern uint32_t g_ln_CAL_Start_Addr; //0x0021dce0
extern uint32_t g_ln_CAL_Data_length; //0x3000
extern uint32_t g_ln_RAM_start_addr_hw_desc__; //base address for hardware descriptors, 0x0002f780
extern uint32_t g_ln_RAM_end_addr_hw_desc__; //0x00034f68, len=0x57e8
extern uint32_t g_ln_RAM_start_addr_hw_pktmem__; //base address for packet memory, 0x00027780
extern uint32_t g_ln_RAM_end_addr_hw_pktmem__; //0x0002f780, len=0x8000

/* enum application mode */
typedef enum app_mode_id
{
    APP_MODE_MM = 0,
    APP_MODE_FTM = 1,
    APP_MODE_MAX = 2,
}app_mode_id_t;

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

extern struct libwifi_kconfig_t g_libwifi_kconfig;
extern struct libwifi_qos_null_kconfig_t g_libwifi_qos_null_kconfig_t;

//API provided by libwifi
extern uint32_t _tst_bmps_enter_f;
void ftm_task_init(void);
void wlan_init_early1 (void);
void wlan_init_early2 (uint32_t is_ftm, uint32_t *bdf_addr, uint32_t *regdb_addr);
void hal_phy_power_ftm_switch_to_listen(void);

void wmi_register_event_handler (wmi_evt_cb_t cb, void *cxt);

void *nt_dpm_memcpy(void *dst, const void *src, uint32_t length);
int32_t wlan_hal_get_rts_rate (void);
void wlan_hal_get_edca_param(uint8_t qid, uint8_t *aifs, uint16_t *cw_min, uint16_t *cw_max, uint16_t *txop_limit);
uint32_t wlan_hal_rts_cts_enabled (void);

void wlan_hal_get_ba_win_size(uint16_t *ack_timeout, uint16_t *delay);
void wlan_hal_get_slot_time(uint32_t *slot_time);
void wlan_hal_get_edcca_threshold(uint8_t *edcca_threshold);
void start_imps_cnx_wait_timer_ext(void);
void stop_imps_cnx_wait_timer(void);

void *nt_dpm_allocate_buffer_ext(uint32_t length);
void nt_dpm_free_buffer_ext(void *buf);
nt_status_t nt_dpm_process_eth_packet_from_stack_ext(void *frame, uint32_t length);
uint8_t *get_ap_dev_ic_myaddr (void *dev_p);
uint8_t get_ap_dev_numConn (void *dev_p);
uint32_t get_ap_dev_ic_flags (void *dev_p);
uint32_t get_ap_dev_ic_flags2 (void *dev_p);
uint8_t *get_dev_ic_myaddr (void);
uint8_t get_currently_enabled_powersave_ext(void);
uint8_t get_dev_phymode (void);
uint16_t wlan_get_listen_interval_ext(uint16_t beaconInterval);
uint16_t get_dev_bss_ni_intval (void);
boolean dev_is_up (void);
void dev_set_rts_enable (uint32_t enable);
void dev_set_rts_rate (uint32_t rate);
void dev_set_cw (uint8_t qid, uint8_t aifsn, uint16_t cw_min, uint16_t cw_max, uint16_t txop_limit);
void dev_set_per_upper_threshold (uint32_t threshold);
uint32_t dev_get_per_upper_threshold(void);
void dev_set_ba_win_size(uint16_t ack_timeout, uint16_t delay);
void dev_set_slot_time(uint32_t slot_time);
void dev_get_beacon_threshold_ext(uint8_t *count);
void hal_mac_sw_powerup(void);
void hal_mac_hw_ctrl(void);

//API called by libwifi
void wmi_unit_test_cmd_handler(WMI_UNIT_TEST_CMD *cmd);
NT_BOOL wmi_pdev_utf_cmd(wmi_msg_struct_t* msg);

//low power start
void nt_socpm_nop_delay(uint64_t n_nops);
void _socpm_slptmr_off(void);
void nt_socpm_mtusr_restore_mtu_time(void);
void nt_socpm_mtusr_save_mtu_time (void);
int nt_socpm_sleep_lst_delete(volatile int List_to_Del);
void nt_socpm_sleep_deregister(int list_idx);
#include "nt_socpm_sleep.h" //for nt_socpm_sleep_t & sleep_mode
int nt_socpm_sleep_register(nt_socpm_sleep_t * FunctionToRegister,volatile int List_no);
uint32_t get_sleep_exit_hw_delay (sleep_mode slp_mode);
void nt_socpm_enable(uint8_t socpm_state);
nt_status_t nt_update_clk_latency(void *buffer);
//low power end

int8_t nt_rram_write(uint32_t address,const void *wdata,uint32_t length);
int8_t nt_rram_read(uint32_t address, void *rdata, uint32_t length);
#ifdef RRAM_WRITE_VIA_DXE
extern uint8_t dxe_deinit;
#endif
int8_t nt_get_macid(uint8_t *macid );

nt_status_t nt_dpm_forward_eth_packet_to_stack_ext(void *rx_buf, void *eth_frame, uint32_t length, void *ext);
void nt_dpm_add_dev_to_stack(void *dev);
void nt_dpm_remove_dev_from_stack (void *dev);
void nt_dpm_stop_network_stack(void);
void nt_dpm_start_network_stack(void);
void *nt_dpm_allocate_network_buffer_pool(uint32_t length);
void *nt_dpm_allocate_network_buffer(uint32_t length);
void nt_dpm_free_network_buffer(void *buf);
void nt_dpm_realloc_network_buffer(void *buf, uint32_t length);
void nt_dpm_network_init(void);
nt_status_t get_netif_hwaddr_from_netif_id(uint8_t netif_id, uint8_t *addr);

#if QCCSDK
void nt_dpm_notify_network_to_set_linkup(struct netif * netif);
void nt_dpm_notify_network_to_set_linkdown(struct netif * netif);
#endif

/* Actions that can be performed when vTaskNotify() is called. */
typedef enum
{
    eNoAction = 0,            /* Notify the task without updating its notify value. */
    eSetBits,                 /* Set bits in the task's notification value. */
    eIncrement,               /* Increment the task's notification value. */
    eSetValueWithOverwrite,   /* Set the task's notification value to a specific value even if the previous value has not yet been read by the task. */
    eSetValueWithoutOverwrite /* Set the task's notification value if the previous value has been read by the task. */
} eNotifyAction;

#define xTaskNotifyFromISR( xTaskToNotify, ulValue, eAction, pxHigherPriorityTaskWoken )  (void)(pxHigherPriorityTaskWoken)
#define portYIELD_FROM_ISR( x )  ((void)(x))
#define xTaskNotify( xTaskToNotify, ulValue, eAction )   
#define xTaskNotifyWait( ulBitsToClearOnEntry, ulBitsToClearOnExit, pulNotificationValue, xTicksToWait )   0

#define	PROF_IRQ_ENTER()  
#define	PROF_IRQ_EXIT()  

//TickType_t
void hres_timer_us_delay(uint32_t time_us);
uint64_t hres_timer_curr_time_us(void);
uint32_t hres_timer_curr_time_ms(void);

app_mode_id_t nt_get_app_mode(void);


