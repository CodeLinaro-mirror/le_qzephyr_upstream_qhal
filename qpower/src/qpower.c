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
#include "zephyr/sys/util_macro.h"

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
#include <zephyr/linker/sections.h>
#include <zephyr/linker/linker-defs.h>
#include <zephyr/drivers/timer/system_timer.h>
#include <zephyr/sys_clock.h>
#include <zephyr/sys/util.h>
#include <zephyr/irq.h>
#include "uart_hal.h"
#include "nt_hw_support.h"
#include "pmu_ll.h"
#include <zephyr/drivers/timer/system_timer.h>

#define ARRAY_SIZE_IN_TYPE(type, member) sizeof(((type *)(0))->member)

typedef struct {
    /* NVIC components stored into RAM. */
    uint32_t ISER[ARRAY_SIZE_IN_TYPE(NVIC_Type, ISER)];
    uint32_t ISPR[ARRAY_SIZE_IN_TYPE(NVIC_Type, ISPR)];
    uint8_t IP[ARRAY_SIZE_IN_TYPE(NVIC_Type, IP)];
    uint8_t SHP[ARRAY_SIZE_IN_TYPE(SCB_Type, SHP)];
} _nvic_context_t;

struct backup {
    _nvic_context_t nvic_context;
};

static __noinit struct backup backup_data;

qpower_param_t gs_qpower_param;

static void aon_set_alarm(uint64_t us)
{
    /* !!! Caution: remove it if automatic suspend is implemented. */
    sys_clock_set_timeout(k_us_to_ticks_ceil64(us), true);
}

qapi_Status_t qapi_pmu_init(void)
{
    PRINT_LOG_FUNC_LINE_ENTRY;

    gs_qpower_param.softoff_duration_ms = DEFAULT_SOFTOFF_DURATION_MS;
    gs_qpower_param.softoff_wakeup_src = DEFAULT_SOFTOFF_WAKEUP_SRC;
    gs_qpower_param.s2ram_duration_ms = 0;
    gs_qpower_param.s2ram_wakeup_src = DEFAULT_S2RAM_WAKEUP_SRC;

    /* dev cfg should be the first to get initialized */
    nt_devcfg_parse(); // devcfg parser function call to fill the common devcfg structure
    // nt_devcfg_byte_seq_parse(); // byte_sequence :: devcfg parser function call to fill the common devcfg structure
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

    nt_socpm_init();
    PRINT_LOG_FUNC_LINE;

#if defined(SUPPORT_HIGH_RES_TIMER)
    hres_timer_init_setup();
    PRINT_LOG_FUNC_LINE;
#if defined(HRES_TIMER_UNIT_TEST)
    // TO-DO: should be moved to POST_KERNEL due to task creation
    hres_timer_test_create_task();
    PRINT_LOG_FUNC_LINE;
#endif
#endif

    nt_socpm_secondary_init();
    PRINT_LOG_FUNC_LINE;

#ifdef NT_SOPCM_CHANGE
    enum error_no reason = wifi_pdc_init();
    if (reason != pdc_init_success) {
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
    if ((uint8_t)nt_socpm_cpr_flag_state_get(CPR_EN)) {
        nt_cpr_init();
        PRINT_LOG_FUNC_LINE;
    }
#endif // NT_FN_CPR

#ifdef NT_FN_SYSMON
    nt_sysmon_threshold_init(); // initializing the thresholds for voltage and temperature
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

    __disable_fault_irq();
    nt_socpm_enable(1);
    if (p_qpower_param->softoff_duration_ms
            && IS_BIT_SET(p_qpower_param->softoff_wakeup_src, WKUP_AON_TIMER)
            && IS_BIT_SET(p_qpower_param->softoff_wakeup_src, WKUP_EXT_PIN)) {
        early_printk("%s wakeup by timer %d ms or gpio\n", __FUNCTION__, p_qpower_param->softoff_duration_ms);
        nt_enable_standby(((uint64_t)(p_qpower_param->softoff_duration_ms))*1000);
    } else if (IS_BIT_SET(p_qpower_param->softoff_wakeup_src, WKUP_EXT_PIN)) {
        early_printk("%s only wakeup by gpio\n", __FUNCTION__);
        nt_enable_indef_deepsleep();
    } else {
        early_printk("%s no valid wakeup source, skip\n", __FUNCTION__);
        goto exit;
    }
exit:
    __enable_fault_irq();
}

static void nvic_suspend(_nvic_context_t *backup)
{
    memcpy(backup->ISER, (uint32_t *)NVIC->ISER, sizeof(NVIC->ISER));
    memcpy(backup->ISPR, (uint32_t *)NVIC->ISPR, sizeof(NVIC->ISPR));
    memcpy(backup->IP, (uint32_t *)NVIC->IP, sizeof(NVIC->IP));
    memcpy(backup->SHP, SCB->SHP, sizeof(SCB->SHP));
}

static void nvic_resume(_nvic_context_t *backup)
{
    memcpy((uint32_t *)NVIC->ISER, backup->ISER, sizeof(NVIC->ISER));
    memcpy((uint32_t *)NVIC->ISPR, backup->ISPR, sizeof(NVIC->ISPR));
    memcpy((uint32_t *)NVIC->IP, backup->IP, sizeof(NVIC->IP));
    memcpy(SCB->SHP, backup->SHP, sizeof(SCB->SHP));
}

static void mcusleep_init_vector_table(void)
{
#define VECTOR_ADDRESS 0

    size_t vector_size = (size_t)_vector_end - (size_t)_vector_start;
    (void)memcpy(VECTOR_ADDRESS, _vector_start, vector_size);
    *(uint32_t *)(VECTOR_ADDRESS + 0x4) = (void *)ram_minimum_code;
    SCB->VTOR = VECTOR_ADDRESS & SCB_VTOR_TBLOFF_Msk;
}

static void mcusleep_restore_vector_table(void) { SCB->VTOR = ((size_t)_vector_start) & SCB_VTOR_TBLOFF_Msk; }

/* Function called during local domain suspend to RAM. */
static int mcu_sleep_enter(void)
{
    //early_printk("%s %d entry\r\n", __FUNCTION__, __LINE__);
    g_socpm_struct.aon_cmnss_wlan_slp_tmr_int_processed = 0;
#ifdef FIRMWARE_APPS_INFORMED_WAKE
    aon_ext_interrupt_wake_up_processed = 0;
#endif
    g_socpm_struct.woken_src = WKUP_UNKNOWN;
    g_socpm_struct.slept_time_ms = 0;
    nvic_suspend(&backup_data.nvic_context);
    mcusleep_init_vector_table();
    //early_printk("vecotr pointed to SRAM 0 \r\n");
    dead_loop_cond1();

    // test_sleep_cb
    q_sleep_wifi_enter(NT_PMU_CFG_WIFI_SLEEP_OFFSET);

    if (gs_qpower_param.s2ram_duration_ms) {
        early_printk("To set sleep timer=%d ms\r\n", gs_qpower_param.s2ram_duration_ms);
        aon_set_alarm(((uint64_t)gs_qpower_param.s2ram_duration_ms) * 1000);
    }

    /* This function performs sleep recipe as per the sleep mode specified */
#ifdef SLEEP_CLK_CAL_IN_SLEEP_MODE
    socpm_slp_clk_cal_presleep_activites(((uint64_t)gs_qpower_param.s2ram_duration_ms) * 1000);
#endif /* SLEEP_CLK_CAL_IN_SLEEP_MODE */

    _socpm_slpcfg_mcuslp();

    dead_loop_cond1();
    _tst_sleep_enter();
    dead_loop();
    // CODE_UNREACHABLE;
    /*
     * We might reach this point is k_cpu_idle returns (there is a pre sleep hook that
     * can abort sleeping.
     */
    return NT_FAIL;
}

static void mcu_sleep_wakeup(void)
{
#ifdef SLEEP_CLK_CAL_IN_SLEEP_MODE
    socpm_slp_clk_cal_postawake_activities();
#endif /* SLEEP_CLK_CAL_IN_SLEEP_MODE */

    // enable systick int
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;
    // sys_clock_announce(Z_TIMEOUT_MS_TICKS(nt_socpm_slp_time_total));

    // hres_timer_post_sleep();

    // request xip
    g_pmu_hal->PMU_CFG_AON_CNTL_MCU_SYSTEM_BOOT_COMPLETE_STATE_RESOURCE_REQ.bit.PD_XIP_CNTL_BIT = 1;

    uart_hal_enable_intr_rx_ext();

    mcusleep_restore_vector_table();
    nvic_resume(&backup_data.nvic_context);

    early_printk("%s %d exit\r\n", __FUNCTION__, __LINE__);
}

void qapi_enter_suspend2ram(void)
{
    qpower_param_t *p_qpower_param = &gs_qpower_param;

    __disable_fault_irq();
    early_printk("%s %d entry\r\n", __FUNCTION__, __LINE__);
    if (p_qpower_param->s2ram_duration_ms
            && IS_BIT_SET(p_qpower_param->s2ram_wakeup_src, WKUP_AON_TIMER)
            && IS_BIT_SET(p_qpower_param->s2ram_wakeup_src, WKUP_EXT_PIN)) {
        early_printk("%s wakeup by timer %d ms or gpio\r\n", __FUNCTION__, p_qpower_param->s2ram_duration_ms);
    } else if (IS_BIT_SET(p_qpower_param->s2ram_wakeup_src, WKUP_EXT_PIN)) {
        early_printk("%s only wakeup by gpio\r\n", __FUNCTION__);
    } else {
        early_printk("%s no valid wakeup source, skip\n", __FUNCTION__);
        goto exit;
    }
    arch_pm_s2ram_suspend(mcu_sleep_enter);
    /* On resuming or error we return exactly *HERE* */
    //ram_minimum_code();
    mcu_sleep_wakeup();
exit:
    early_printk("%s %d exit\r\n", __FUNCTION__, __LINE__);
    __enable_fault_irq();
}

void qapi_suspend2ram_exit_post_ops(void)
{
    static uint32_t mcusleep_cnt = 0;

    mcusleep_cnt++;
    early_printk("%s %d mcusleep_cnt=%d\r\n", __FUNCTION__, __LINE__, mcusleep_cnt);
    dead_loop_cond2();
    irq_unlock(0);
}

void qapi_slp_tmr_set(uint64_t slp_us)
{
    sys_clock_set_timeout( k_us_to_ticks_ceil32(slp_us), true);
}
