/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/
/**
 * @file soc.c
 * @brief System/hardware module for QCA402x processor
 * @version 0.1
 * @date 2025-06-18
 * 
 */

#include "printfext.h"
#include "qpower.h"
#include "qpower_internal.h"

#include "libpower.h"
#include "qlib_early_printk.h"
#include "qlib_util.h"

#include "nt_common.h"
#include "nt_socpm_sleep.h"

#include "nt_devcfg_structure.h"
#ifdef NT_NEUTRINO_1_0_SYS_MAC
#include "nt_pmic_driver.h"
#endif
#include "wifi_fw_pmic_driver.h"
#include "nt_socpm_sleep.h"
#include "unpa.h"
#include "nt_wifi_driver.h"
#ifdef NT_FN_CPR
#include "nt_cpr_driver.h"
#endif
#include "nt_sys_monitoring.h"
#include "wifi_fw_cpr_driver.h"

#ifdef NT_GPIO_FLAG
#include "nt_gpio_api.h"
#include "wifi_fw_internal_api.h"
#endif

#ifdef FIRMWARE_APPS_INFORMED_WAKE
#include "wifi_fw_ext_intr.h"
#endif

#include <zephyr/toolchain.h>
#include <zephyr/pm/policy.h>
#include <zephyr/arch/cpu.h>
#include <zephyr/arch/common/pm_s2ram.h>

qpower_param_t gs_qpower_param;

qapi_Status_t qapi_pmu_init (void)
{
    PRINT_LOG_FUNC_LINE_ENTRY;

    gs_qpower_param.softoff_duration_ms = DEFAULT_SOFTOFF_DURATION_MS;
    gs_qpower_param.softoff_wakeup_src = DEFAULT_SOFTOFF_WAKEUP_SRC;
    gs_qpower_param.s2ram_duration_ms = DEFAULT_S2RAM_DURATION_MS;
    gs_qpower_param.s2ram_wakeup_src = DEFAULT_S2RAM_WAKEUP_SRC;

    /* dev cfg should be the first to get initialized */
    nt_devcfg_parse();      // devcfg parser function call to fill the common devcfg structure
    //nt_devcfg_byte_seq_parse(); // byte_sequence :: devcfg parser function call to fill the common devcfg structure
    PRINT_LOG_FUNC_LINE;

#ifdef FEATURE_FDI
    fdi_init();
    fdi_reg_all_nodes();
    PRINT_LOG_FUNC_LINE;
#endif /* FEATURE_FDI */

#if FPCI_DEBUG
    fpci_register_test_cb();
    PRINT_LOG_FUNC_LINE;
#endif /* FPCI_DEBUG */

    nt_socpm_init_soc_cfg();
    PRINT_LOG_FUNC_LINE;

#ifdef NT_NEUTRINO_1_0_SYS_MAC
    nt_pmic_init();
    PRINT_LOG_FUNC_LINE;
#endif

    wifi_fw_pmic_init(cpr_openloop);
    PRINT_LOG_FUNC_LINE;

#ifndef CBC_CX_VOLTAGE_WAR
    wifi_fw_cpr_init();
    PRINT_LOG_FUNC_LINE;
#endif /*CBC_CX_VOLTAGE_WAR */
#ifdef QPOWER
    nt_socpm_init();
    PRINT_LOG_FUNC_LINE;
#endif

#if defined(SUPPORT_HIGH_RES_TIMER)
    hres_timer_init_setup();
    PRINT_LOG_FUNC_LINE;
#if defined(HRES_TIMER_UNIT_TEST)
    //TO-DO: should be moved to POST_KERNEL due to task creation
    hres_timer_test_create_task();
    PRINT_LOG_FUNC_LINE;
#endif
#endif

    nt_socpm_secondary_init();
    PRINT_LOG_FUNC_LINE;

#ifdef NT_SOPCM_CHANGE
    enum error_no reason=wifi_pdc_init();
    if(reason != pdc_init_success ) {
        WLAN_DBG0_PRINT("WIFI Resource Creation failed");
    }
    PRINT_LOG_FUNC_LINE;
#else
    unpa_init();
    PRINT_LOG_FUNC_LINE;
    nt_pdc_driver_init();
    PRINT_LOG_FUNC_LINE;
#ifdef NT_FN_PDC_
    nt_pdc_init();
    PRINT_LOG_FUNC_LINE;
#endif
#endif

#ifdef NT_FN_CC_MGMT
    nt_cc_battery_mgmt_init();
    PRINT_LOG_FUNC_LINE;
#endif

#ifdef NT_FN_CPR
    if((uint8_t) nt_socpm_cpr_flag_state_get(CPR_EN)) {
        nt_cpr_init();
        PRINT_LOG_FUNC_LINE;
    }
#endif //NT_FN_CPR

#ifdef NT_FN_SYSMON
    nt_sysmon_threshold_init();// initializing the thresholds for voltage and temperature
    PRINT_LOG_FUNC_LINE;
#endif

#ifdef NT_GPIO_FLAG
    nt_gpio_init();
    PRINT_LOG_FUNC_LINE;
#endif

    wifi_fw_pmic_init(cpr_closeloop);
    PRINT_LOG_FUNC_LINE;

#ifdef CBC_CX_VOLTAGE_WAR
    wifi_fw_cpr_init();
    PRINT_LOG_FUNC_LINE;
#endif // CBC_CX_VOLTAGE_WAR

#ifdef NT_GPIO_FLAG
    wifi_fw_gpio_init(FALSE);

    PRINT_LOG_FUNC_LINE;
#endif

#ifdef FIRMWARE_APPS_INFORMED_WAKE
    /* Initialize A2F interrupt */
    init_aon_ext_wakeup_int();
    PRINT_LOG_FUNC_LINE;

#ifdef SUPPORT_RING_IF
    /* F2A signal on cold boot */
    wifi_fw_ext_cold_boot_f2a_signal();
    PRINT_LOG_FUNC_LINE;
#endif
#else
    /** Disable the external wakeup interrupt when the feature is not enabled
    * as it prevents SOC from entering sleep state.
    */
    disable_aon_ext_wakeup_int();
    PRINT_LOG_FUNC_LINE;

#ifdef SUPPORT_RING_IF
    wifi_fw_f2a_interrupt();
    PRINT_LOG_FUNC_LINE;
#endif
#endif /* FIRMWARE_APPS_INFORMED_WAKE */

    PRINT_LOG_FUNC_LINE_EXIT;

    return QAPI_OK;
}

qapi_Status_t qapi_power_set_parameter(uint32_t type, uint32_t val)
{
    qapi_Status_t ret = QAPI_OK;

    log_printf("%s, type:%d val:%d\n", __func__, type, val);
    switch (type) {
        case __QAPI_POWER_SOFTOFF_DURATION_MS:
            log_printf("softoff_duration_ms %d=>%d\n", gs_qpower_param.softoff_duration_ms, val);
            gs_qpower_param.softoff_duration_ms = val;
            break;
        case __QAPI_POWER_SUSPEND2RAM_DURATION_MS:
            log_printf("s2ram_duration_ms %d=>%d\n", gs_qpower_param.s2ram_duration_ms, val);
            gs_qpower_param.s2ram_duration_ms = val;
            break;
        default:
            ret = QAPI_ERR_NOT_SUPPORTED;
            break;
    }
    return ret;
}

qapi_Status_t qapi_power_get_parameter(uint32_t type, uint32_t *val)
{
    qapi_Status_t ret = QAPI_OK;

    if (!val) {
        ret = QAPI_ERR_INVALID_PARAM;
        goto exit;
    }

    log_printf("%s, type:%d\n", __func__, type);
    switch (type) {
        case __QAPI_POWER_SOFTOFF_DURATION_MS:
            *val = gs_qpower_param.softoff_duration_ms;
            break;
        case __QAPI_POWER_SUSPEND2RAM_DURATION_MS:
            *val = gs_qpower_param.s2ram_duration_ms;
            break;
        default:
            ret = QAPI_ERR_NOT_SUPPORTED;
            break;
    }

exit:
    return ret;
}

void qapi_enter_softoff(void)
{
    qpower_param_t *p_qpower_param = &gs_qpower_param;

    PRINT_LOG_FUNC_LINE_ENTRY;
    nt_socpm_enable(1);
    PRINT_LOG_FUNC_LINE;
    if (p_qpower_param->softoff_duration_ms
            && IS_BIT_SET(p_qpower_param->softoff_wakeup_src, WKUP_AON_TIMER)
            && IS_BIT_SET(p_qpower_param->softoff_wakeup_src, WKUP_EXT_PIN)) {
        log_printf("%s wakeup by timer %d ms or gpio\n", __FUNCTION__, p_qpower_param->softoff_duration_ms);
        nt_enable_standby(((uint64_t)(p_qpower_param->softoff_duration_ms))*1000);
    } else if (IS_BIT_SET(p_qpower_param->softoff_wakeup_src, WKUP_EXT_PIN)) {
        log_printf("%s only wakeup by gpio\n", __FUNCTION__);
        nt_enable_indef_deepsleep();
    }
    PRINT_LOG_FUNC_LINE_EXIT;
}

void __enter_suspend2ram (void)
{
    __disable_irq();
    early_printk("%s %d entry\r\n", __FUNCTION__, __LINE__);
    arch_pm_s2ram_suspend(mcu_sleep_enter);
    /* On resuming or error we return exactly *HERE* */
    ram_minimum_code();
    mcu_sleep_wakeup();
    early_printk("%s %d exit\r\n", __FUNCTION__, __LINE__);
    __enable_irq();
}

void qapi_enter_suspend2ram(void)
{
    qpower_param_t *p_qpower_param = &gs_qpower_param;

    PRINT_LOG_FUNC_LINE_ENTRY;
    if (p_qpower_param->s2ram_duration_ms
            && IS_BIT_SET(p_qpower_param->s2ram_wakeup_src, WKUP_AON_TIMER)
            && IS_BIT_SET(p_qpower_param->s2ram_wakeup_src, WKUP_EXT_PIN)) {
        log_printf("%s wakeup by timer %d ms or gpio\n", __FUNCTION__, p_qpower_param->s2ram_duration_ms);
        __enter_suspend2ram();
    } else if (IS_BIT_SET(p_qpower_param->s2ram_wakeup_src, WKUP_EXT_PIN)) {
        log_printf("%s only wakeup by gpio\n", __FUNCTION__);
        __enter_suspend2ram();
        return;
    }
}

void qapi_suspend2ram_exit_post_ops(void)
{
    static uint32_t mcusleep_cnt = 0;

    mcusleep_cnt++;
    early_printk("%s %d mcusleep_cnt=%d\r\n", __FUNCTION__, __LINE__, mcusleep_cnt);
    dead_loop_cond2();
    irq_unlock(0);
}

