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

#ifndef QPOWER
#include "nt_socpm_sleep.h"
#endif
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

#if 0
void hres_timer_us_delay(uint32_t time_us)
{
    uint64_t curr_time = hres_timer_curr_time_us();
    uint64_t target_time = (curr_time + time_us);

    while(curr_time < target_time)
    {
        curr_time = hres_timer_curr_time_us();
    }
}
#else
void hres_timer_us_delay(uint32_t time_us) { k_busy_wait(time_us); }
#endif

uint32_t nt_hal_get_curr_time(void);

uint64_t hres_timer_curr_time_us(void)
{
#ifdef SUPPORT_HIGH_RES_TIMER
    uint64_t curr_time_us;
    timer_cvt_from_tick64(hres_timer_timetick_get(), T_USEC, &curr_time_us);
    return curr_time_us;
#else  /* SUPPORT_HIGH_RES_TIMER */
    return nt_hal_get_curr_time();
#endif /* SUPPORT_HIGH_RES_TIMER */
}

uint32_t hres_timer_curr_time_ms(void)
{
#ifdef SUPPORT_HIGH_RES_TIMER
    uint64_t curr_time_ms;
    timer_cvt_from_tick64(hres_timer_timetick_get(), T_MSEC, &curr_time_ms);
    return curr_time_ms;
#else  /* SUPPORT_HIGH_RES_TIMER */
    return (nt_hal_get_curr_time() / 1000);
#endif /* SUPPORT_HIGH_RES_TIMER */
}

// NT_LOG_LVL_WARN
extern uint8_t min_loglvl;

uint8_t nt_log_write(
    /*!@module id like SME,MLME,HAL.etc...*/
    uint8_t mod_id,
    /*!@ loglevel like info,warning.etc...*/
    uint8_t loglvl,
    /*@ for file name*/
    char *fn,
    /*@ for line number*/
    uint16_t ln,
    /*!@ data */
    const char *msg,
    /* user provided parameters */
    uint32_t p1, uint32_t p2, uint32_t p3

)
{
    (void)mod_id;

#if 1

    if (loglvl < min_loglvl) {
        return NT_ECANCELED;
    }

    printk("[%s %d]: %s [%d] [%d] [%d]\r\n", fn, ln, msg, p1, p2, p3);
#endif
    return NT_OK;
}

#define MSGBUF_LEN 200
char my1pbuf[MSGBUF_LEN];

uint8_t nt_log_printf(uint8_t mod_id, uint8_t loglvl, char *func_name,
                      /*@ for line number*/
                      uint16_t ln, const char *fmt, uint8_t num, ...)
{
#if 1
    va_list argp;
    (void)mod_id;

    if (loglvl < min_loglvl) {
        return NT_ECANCELED;
    }

    memset(my1pbuf, 0, MSGBUF_LEN);
    va_start(argp, num);
    vsnprintf(my1pbuf, sizeof(my1pbuf), fmt, argp);
    va_end(argp);
    printk("[%s %d]: ", func_name, ln);
    uart_hal_poll_out_str_ext(my1pbuf);
    uart_hal_poll_out_str_ext("\r\n");
#endif
    return NT_OK;
}

uint8_t nt_log_array_printf(uint8_t mod_id, uint8_t loglvl, char *func_name,
                            /*@ for line number*/
                            uint16_t ln, const char *s, const uint8_t *ptr, const uint16_t len)
{
    (void)mod_id;
    (void)loglvl;
    (void)func_name;
    (void)ln;
    (void)s;
    (void)ptr;
    (void)len;
    return NT_OK;
}

app_mode_id_t nt_get_app_mode(void) { return APP_MODE_MM; }

int32_t pmu_ts_get_current_temperature(void)
{
    // room temperature in degree
    return 25;
}

#ifndef QPOWER
void nt_socpm_nop_delay(uint64_t n_nops)
{
    for (uint64_t i = 0; i < n_nops; i++)
        __asm volatile(" nop \n");
}

uint32_t get_sleep_exit_hw_delay(sleep_mode slp_mode)
{
    (void)slp_mode;
    return 0;
}

int nt_socpm_sleep_register(nt_socpm_sleep_t *FunctionToRegister, volatile int List_no)
{
    (void)FunctionToRegister;
    (void)List_no;
    return -1;
}

void nt_socpm_sleep_deregister(int list_idx) { (void)list_idx; }

int nt_socpm_sleep_lst_delete(volatile int List_to_Del)
{
    (void)List_to_Del;
    return 0;
}

void _socpm_slptmr_off(void) {}

void nt_socpm_mtusr_save_mtu_time(void) {}

void nt_socpm_mtusr_restore_mtu_time(void) {}

uint64_t freertosdefaultminimum(uint32_t wkup_delay_us)
{
    (void)wkup_delay_us;
    return 0;
}

void nt_socpm_enable(uint8_t socpm_state) { (void)socpm_state; }
nt_status_t nt_update_clk_latency(void *buffer)
{
    (void)buffer;
    return 0;
}
#endif
