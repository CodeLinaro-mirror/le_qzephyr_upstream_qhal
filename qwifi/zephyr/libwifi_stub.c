/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
  * SPDX-License-Identifier: BSD-3-Clause*/
#include <libwifi.h>

#include "qurt_pipe.h"

//API provided by libwifi
qurt_pipe_t msg_wfm_wmi_id;

void wmi_register_event_handler (wmi_evt_cb_t cb, void *cxt)
{
	(void)cb;
	(void)cxt;
}

void *nt_dpm_memcpy(void *dst, const void *src, uint32_t length)
{
	(void)dst;
	(void)src;
	(void)length;
	return NULL;
}

int32_t wlan_hal_get_rts_rate (void)
{
	return NT_FAIL;
}

void wlan_hal_get_edca_param(uint8_t qid, uint8_t *aifs, uint16_t *cw_min, uint16_t *cw_max, uint16_t *txop_limit)
{
	(void)qid;
	(void)aifs;
	(void)cw_min;
	(void)cw_max;
	(void)txop_limit;
}

void wlan_hal_get_ba_win_size(uint16_t *ack_timeout, uint16_t *delay)
{
	(void)ack_timeout;
	(void)delay;
}

void wlan_hal_get_slot_time(uint32_t *slot_time)
{
	(void)slot_time;
}

void wlan_hal_get_edcca_threshold(uint8_t *edcca_threshold)
{
	(void)edcca_threshold;
}

uint32_t wlan_hal_rts_cts_enabled (void)
{
	return 0;
}

void start_imps_cnx_wait_timer_ext(void)
{
}

void stop_imps_cnx_wait_timer(void)
{
}

void *nt_dpm_allocate_buffer_ext(uint32_t length)
{
	(void)length;
	return NULL;
}

void nt_dpm_free_buffer_ext(void *buf)
{
	(void)buf;
	return;
}

nt_status_t nt_dpm_process_eth_packet_from_stack_ext(void *frame, uint32_t length)
{
	(void)frame;
	(void)length;
	return NT_OK;
}

uint8_t *get_ap_dev_ic_myaddr (void *dev_p)
{
	(void)dev_p;
	return NULL;
}

uint8_t get_ap_dev_numConn (void *dev_p)
{
	(void)dev_p;
	return 0;
}

uint32_t get_ap_dev_ic_flags (void *dev_p)
{
	(void)dev_p;
	return 0;
}

uint32_t get_ap_dev_ic_flags2 (void *dev_p)
{
	(void)dev_p;
	return 0;
}

uint8_t *get_dev_ic_myaddr (void)
{
	return NULL;
}

uint8_t get_currently_enabled_powersave_ext(void)
{
	return 0;
}

uint8_t get_dev_phymode (void)
{
	return 0;
}

uint16_t wlan_get_listen_interval_ext(uint16_t beaconInterval)
{
	(void)beaconInterval;
	return 0;
}

uint16_t get_dev_bss_ni_intval (void)
{
	return 0;
}

boolean dev_is_up (void)
{
	return 1;
}

void dev_set_rts_enable (uint32_t enable)
{
	(void)enable;
}

void dev_set_rts_rate (uint32_t rate)
{
	(void)rate;
}

void dev_set_cw (uint8_t qid, uint8_t aifsn, uint16_t cw_min, uint16_t cw_max, uint16_t txop_limit)
{
	(void)qid;
	(void)aifsn;
	(void)cw_min;
	(void)cw_max;
	(void)txop_limit;
}

void dev_set_per_upper_threshold (uint32_t threshold)
{
	(void)threshold;
}

uint32_t dev_get_per_upper_threshold(void)
{
	return 0;
}

void dev_set_ba_win_size(uint16_t ack_timeout, uint16_t delay)
{
	(void)ack_timeout;
	(void)delay;
}

void dev_set_slot_time(uint32_t slot_time)
{
	(void)slot_time;
}

void dev_get_beacon_threshold_ext(uint8_t *count){
	(void)count;
}

void hal_mac_sw_powerup(void)
{
}

void hal_mac_hw_ctrl()
{
}

