
#pragma once

#include "nt_socpm_sleep.h"
#include "nt_common.h"

struct libpower_kconfig_t {
    uint32_t reserved;
};

typedef void (*wifi_pre_sleep_ulpsmps2_cb)(void);
typedef void (*pmu_ts_init_cb)(void);
typedef  void (*pmu_ts_configure_cb)(void);
typedef nt_status_t (*slp_clk_cal_enable_cb)(slp_clk_cal_mode_t mode);
typedef void (*set_sleep_exit_reason_cb)(void *reason);
typedef void (*qtmr_init_cb)(void);
typedef void (*nt_hal_complete_rri_restore_op_cb)(void);
typedef void (*nt_socpm_mtusr_restore_mtu_time_cb)(void);
typedef void (*watchdog_feed_cb)(void);
typedef struct libpower_ifc {
    wifi_pre_sleep_ulpsmps2_cb ulpsmps2;
    pmu_ts_init_cb  ts_init;
    pmu_ts_configure_cb ts_configure;
    slp_clk_cal_enable_cb slp_clk_cal_enable;
    set_sleep_exit_reason_cb set_sleep_exit_reason;
    qtmr_init_cb qtmr_init;
    nt_hal_complete_rri_restore_op_cb nt_hal_complete_rri_restore_op;
    nt_socpm_mtusr_restore_mtu_time_cb nt_socpm_mtusr_restore_mtu_time;
    watchdog_feed_cb watchdog_feed;
} libpower_ifc_t;

#define LIBPOWER_IFC_CALL(func, ...) \
    do { \
        if (libpower_ifc.func) { \
            libpower_ifc.func(__VA_ARGS__); \
        } \
    } while (0)

extern struct libpower_kconfig_t g_libpower_kconfig;

void nt_enable_standby(uint64_t sleep_time);
void nt_enable_indef_deepsleep(void);

void q_sleep_wifi_enter (uint32_t state);
void _socpm_slpcfg_mcuslp(void);
void _tst_sleep_enter(void);
void ram_minimum_code(void);

/*function to clear the sleep timer interrupt*/
void nt_socpm_slp_tmr_set(uint64_t sleep_time);
void _socpm_slptmr_off(void);
void _socpm_slptmr_set_max_expire (void);
void reg_libpower_ifc(libpower_ifc_t *ifc_t);

void nt_hal_complete_rri_restore_op(void);
extern void qtmr_init();