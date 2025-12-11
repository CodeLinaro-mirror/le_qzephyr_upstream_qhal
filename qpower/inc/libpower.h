
#pragma once

struct libpower_kconfig_t {
    uint32_t reserved;
};
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

