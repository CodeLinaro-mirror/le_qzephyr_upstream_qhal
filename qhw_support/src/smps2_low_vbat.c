/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/**********************************************************************************************
 * @file smps2_low_vbat.c
 * @brief SMPS2 Low Vbatt (2.0V~2.6V) Support — Implementation
 *
 * See smps2_low_vbat.h for full feature description.
 *
 *********************************************************************************************/

/*----------------------------------------------------------------------------
 * Include Files
 *--------------------------------------------------------------------------*/

#include "smps2_low_vbat.h"
#include "HALhwio.h"
#include "Fermion_seq_hwioreg.h"
#include "nt_logger_api.h"
#include "nt_socpm_sleep.h"    /* nt_socpm_nop_delay */
#include "qcc730v2.h"          /* RPMU_BASE_rpmu_Type, QCC730V2_RPMU_BASE_BASE */
#include "qurt_signal.h"
#include "qurt_thread.h"       /* nt_qurt_thread_create */
#include "wifi_fw_pmu_ts_cfg.h" /* pmu_ts_get_current_temperature, tv_monitor_get_vbat_mV */

/*----------------------------------------------------------------------------
 * PFM Usability Lookup Table
 *
 * Rows  : Vbatt bands  — index 0 = lowest (2.0V), index 3 = highest (2.6V+)
 * Cols  : Temperature  — index 0 = 30°C, index 1 = 50°C, index 2 = 70°C, index 3 = 90°C
 *
 * Value 1 = PFM safe, 0 = must stay in PWM.
 *
 * Raw HW data (FR202397, no guard-band):
 *   Vbatt >= 2.6V : Y Y Y Y
 *   Vbatt  = 2.4V : Y Y Y N
 *   Vbatt  = 2.2V : Y Y N N
 *   Vbatt  = 2.0V : Y N N N
 *--------------------------------------------------------------------------*/

/* Number of Vbatt / temperature breakpoints in the table */
#define PFM_TABLE_VBAT_ROWS   4
#define PFM_TABLE_TEMP_COLS   4

/*
 * Vbatt lower-bound for each row (mV).
 * Row i covers: pfm_vbat_min[i] <= vbat < pfm_vbat_min[i+1]
 * Row 3 covers: vbat >= pfm_vbat_min[3]  (i.e. >= 2600 mV)
 */
static const uint32_t pfm_vbat_min_mv[PFM_TABLE_VBAT_ROWS] = {
    2000, 2200, 2400, 2600
};

/*
 * Temperature upper-bound for each column (°C).
 * Col j covers: temp <= pfm_temp_max[j]
 * Col 0 covers: temp <= 30°C
 * Col 1 covers: 30°C < temp <= 50°C
 * Col 2 covers: 50°C < temp <= 70°C
 * Col 3 covers: 70°C < temp <= 90°C  (anything above 90°C is treated as col 3 = worst case)
 *
 * This array is mutable so that it can be overridden at runtime via the
 * smps2_set_pfm_temp_thresholds() API (e.g. from a shell command for testing).
 */
static int32_t pfm_temp_max_c[PFM_TABLE_TEMP_COLS] = {
    30, 50, 70, 90
};

/*
 * pfm_table[vbat_row][temp_col]
 *   1 = PFM safe
 *   0 = must stay in PWM
 *
 * vbat_row 0 = 2.0V, 1 = 2.2V, 2 = 2.4V, 3 = 2.6V+
 * temp_col 0 = <=30C, 1 = <=50C, 2 = <=70C, 3 = <=90C
 */
static const uint8_t pfm_table[PFM_TABLE_VBAT_ROWS][PFM_TABLE_TEMP_COLS] = {
    /* 30C  50C  70C  90C */
    {  1,   0,   0,   0  },  /* Vbatt = 2.0V */
    {  1,   1,   0,   0  },  /* Vbatt = 2.2V */
    {  1,   1,   1,   0  },  /* Vbatt = 2.4V */
    {  1,   1,   1,   1  },  /* Vbatt >= 2.6V */
};

/*----------------------------------------------------------------------------
 * Guard-band values (runtime-configurable)
 *
 * These are applied in smps2_can_use_pfm() before the table lookup:
 *   effective_temp = actual_temp + g_pfm_temp_guardband_c
 *   effective_vbat = actual_vbat - g_pfm_vbat_guardband_mv
 *
 * Defaults match the compile-time constants in smps2_low_vbat.h.
 *
 * To find the current addresses of these variables after a build, run:
 *   arm-zephyr-eabi-nm zephyr.elf | grep -E 'pfm_temp_max_c|g_pfm_temp_guardband|g_pfm_vbat_guardband'
 * or use the helper script: modules/hal/qcom/qhw_support/scripts/smps2_pfm_addr.py
 *--------------------------------------------------------------------------*/

static int32_t  g_pfm_temp_guardband_c  = SMPS2_PFM_TEMP_GUARDBAND_C;
static uint32_t g_pfm_vbat_guardband_mv = SMPS2_PFM_VBAT_GUARDBAND_MV;

/*----------------------------------------------------------------------------
 * Verbose logging control
 *
 * g_smps2_log_verbose = 0 (default): decision/state-change logs use INFO
 * g_smps2_log_verbose = 1          : decision/state-change logs use ERR
 *                                    (ERR is always printed regardless of
 *                                     compile-time log level filter)
 *
 * Set via shell: qbmps smps2_set_verbose <0|1>
 * Set via address: use smps2_pfm_addr.py to find g_smps2_log_verbose address.
 *--------------------------------------------------------------------------*/
static uint32_t g_smps2_log_verbose = SMPS2_LOG_VERBOSE_OFF;

/*
 * SMPS2_LOG(fmt, ...) — log at ERR if verbose, INFO otherwise.
 * Use this for all decision/state-change messages in this file.
 */
#define SMPS2_LOG(fmt, ...) \
    do { \
        if (g_smps2_log_verbose) { \
            NT_LOG_PRINT(SOCPM, ERR, fmt, ##__VA_ARGS__); \
        } else { \
            NT_LOG_PRINT(SOCPM, INFO, fmt, ##__VA_ARGS__); \
        } \
    } while (0)

/*----------------------------------------------------------------------------
 * PFM Monitor Task — signal and task handle
 *
 * The task waits on g_smps2_pfm_signal.  The Vbatt-done ISR
 * (pmu_ccpu_vbat_mon_done_intr) calls smps2_low_vbat_notify_vbat_updated()
 * which sets SMPS2_PFM_MONITOR_SIGNAL_MASK to wake the task.
 * The task then reads the latest Vbatt/Temp values and decides whether to
 * switch SMPS2 from PWM to PFM.
 *--------------------------------------------------------------------------*/

#define SMPS2_PFM_MONITOR_SIGNAL_MASK   0x00000001U
#define SMPS2_PFM_MONITOR_TASK_STACK    512U
/*
 * Priority 8: lower than all WiFi tasks (nt_wlan_evt=5, nt_wlan=6, data_path=7)
 * so it only runs when WiFi tasks are blocked/sleeping.
 * PFM switching is not time-critical; it is fine to run at background priority.
 * Valid range: 0 (highest) to CONFIG_NUM_PREEMPT_PRIORITIES-1 = 14 (lowest).
 */
#define SMPS2_PFM_MONITOR_TASK_PRIORITY 8U

static qurt_signal_t   *g_smps2_pfm_signal    = NULL;
static TaskHandle_t     g_smps2_pfm_task_hnd  = (TaskHandle_t)NULL;

/*----------------------------------------------------------------------------
 * Module State
 *--------------------------------------------------------------------------*/

static smps2_low_vbat_state_t g_smps2_low_vbat_state = {
    .low_vbat_regs_applied  = false,
    .auto_switch_configured = false,
};

/*----------------------------------------------------------------------------
 * Private Helper: RPMU register accessor
 *--------------------------------------------------------------------------*/

static inline RPMU_BASE_rpmu_Type *_rpmu(void)
{
    return (RPMU_BASE_rpmu_Type *)QCC730V2_RPMU_BASE_BASE;
}

/*----------------------------------------------------------------------------
 * smps2_can_use_pfm
 *--------------------------------------------------------------------------*/

bool smps2_can_use_pfm(uint32_t vbat_mV, int32_t temp_C)
{
    /* Apply guard-band: be pessimistic about both Vbat and temperature */
    int32_t  eff_temp = temp_C + g_pfm_temp_guardband_c;
    uint32_t eff_vbat = (vbat_mV > g_pfm_vbat_guardband_mv)
                        ? (vbat_mV - g_pfm_vbat_guardband_mv)
                        : 0;

    /* Find Vbatt row: highest row whose lower-bound <= eff_vbat */
    int vbat_row = 0;
    for (int i = 0; i < PFM_TABLE_VBAT_ROWS; i++) {
        if (eff_vbat >= pfm_vbat_min_mv[i]) {
            vbat_row = i;
        }
    }

    /* Find temperature column: first column whose upper-bound >= eff_temp */
    int temp_col = PFM_TABLE_TEMP_COLS - 1; /* default: worst case */
    for (int j = 0; j < PFM_TABLE_TEMP_COLS; j++) {
        if (eff_temp <= pfm_temp_max_c[j]) {
            temp_col = j;
            break;
        }
    }

    bool result = (pfm_table[vbat_row][temp_col] == 1);

    if (result) {
        SMPS2_LOG("smps2_can_use_pfm: vbat=%umV(eff=%u) temp=%dC(eff=%d) "
                  "row=%d col=%d -> PFM",
                  vbat_mV, eff_vbat, temp_C, eff_temp, vbat_row, temp_col);
    } else {
        SMPS2_LOG("smps2_can_use_pfm: vbat=%umV(eff=%u) temp=%dC(eff=%d) "
                  "row=%d col=%d -> PWM",
                  vbat_mV, eff_vbat, temp_C, eff_temp, vbat_row, temp_col);
    }

    return result;
}

/*----------------------------------------------------------------------------
 * smps2_set_low_vbat_regs
 *--------------------------------------------------------------------------*/

void smps2_set_low_vbat_regs(bool low_vbat)
{
    RPMU_BASE_rpmu_Type *p_rpmu = _rpmu();

    if (low_vbat) {
        /*
         * Vbatt < 2.6V: apply conservative values from the 1.8-2.4V band.
         * Per HW spec:
         *   PMIN_ON = 6  (min PMOS on-time = 48 ns)
         *   NMIN_ON = 5  (min NMOS on-time = 40 ns)
         *   CL_ILIM_MIN = 4
         */
        p_rpmu->RPMU_R_PMU_SMPS2_4.bit.SMPS2_PMIN_ON    = SMPS2_PMIN_ON_VBAT_LOW;
        p_rpmu->RPMU_R_PMU_SMPS2_4.bit.SMPS2_NMIN_ON    = SMPS2_NMIN_ON_VBAT_LOW;
        p_rpmu->RPMU_R_PMU_SMPS2_4.bit.SMPS2_CL_ILIM_MIN = SMPS2_CL_ILIM_MIN_LOW_VBAT;

        SMPS2_LOG("smps2_set_low_vbat_regs: LOW vbat regs applied "
                  "(pmin=%d nmin=%d cl_ilim_min=%d)",
                  SMPS2_PMIN_ON_VBAT_LOW,
                  SMPS2_NMIN_ON_VBAT_LOW,
                  SMPS2_CL_ILIM_MIN_LOW_VBAT);

        g_smps2_low_vbat_state.low_vbat_regs_applied = true;
    } else {
        /*
         * Vbatt >= 2.6V: restore chip defaults (3.0-3.6V band values).
         * Per HW spec:
         *   PMIN_ON = 4  (min PMOS on-time = 32 ns)
         *   NMIN_ON = 3  (min NMOS on-time = 24 ns)
         *   CL_ILIM_MIN = 11 (register default 0x2C >> 2)
         */
        p_rpmu->RPMU_R_PMU_SMPS2_4.bit.SMPS2_PMIN_ON    = SMPS2_DEFAULT_PMIN_ON;
        p_rpmu->RPMU_R_PMU_SMPS2_4.bit.SMPS2_NMIN_ON    = SMPS2_DEFAULT_NMIN_ON;
        p_rpmu->RPMU_R_PMU_SMPS2_4.bit.SMPS2_CL_ILIM_MIN = SMPS2_DEFAULT_CL_ILIM_MIN;

        SMPS2_LOG("smps2_set_low_vbat_regs: DEFAULT regs restored "
                  "(pmin=%d nmin=%d cl_ilim_min=%d)",
                  SMPS2_DEFAULT_PMIN_ON,
                  SMPS2_DEFAULT_NMIN_ON,
                  SMPS2_DEFAULT_CL_ILIM_MIN);

        g_smps2_low_vbat_state.low_vbat_regs_applied = false;
    }
}

/*----------------------------------------------------------------------------
 * smps2_init_auto_switch
 *--------------------------------------------------------------------------*/

void smps2_init_auto_switch(void)
{
    if (g_smps2_low_vbat_state.auto_switch_configured) {
        return; /* idempotent */
    }

    RPMU_BASE_rpmu_Type *p_rpmu = _rpmu();

    /* smps2_ea_hold = 0 (RPMU_R_PMU_SMPS2_3) */
    p_rpmu->RPMU_R_PMU_SMPS2_3.bit.SMPS2_EA_HOLD = SMPS2_AUTO_SWITCH_EA_HOLD;

    /* smps2_pfm2pwm_hfrc = 7, smps2_pfm2pwm_thres = 1 (RPMU_R_PMU_SMPS2_6) */
    p_rpmu->RPMU_R_PMU_SMPS2_6.bit.SMPS2_PFM2PWM_HFRC  = SMPS2_AUTO_SWITCH_PFM2PWM_HFRC;
    p_rpmu->RPMU_R_PMU_SMPS2_6.bit.SMPS2_PFM2PWM_THRES = SMPS2_AUTO_SWITCH_PFM2PWM_THRES;

    /* smps2_lpm_ovr = 2: enable HW auto-switch (PFM->PWM by HW) (RPMU_R_PMU_SMPS2_5) */
    p_rpmu->RPMU_R_PMU_SMPS2_5.bit.SMPS2_LPM_OVR = SMPS2_LPM_OVR_AUTO_SWITCH;

    g_smps2_low_vbat_state.auto_switch_configured = true;

    NT_LOG_PRINT(SOCPM, INFO,
                 "smps2_init_auto_switch: ea_hold=%d pfm2pwm_hfrc=%d "
                 "pfm2pwm_thres=%d lpm_ovr=%d",
                 SMPS2_AUTO_SWITCH_EA_HOLD,
                 SMPS2_AUTO_SWITCH_PFM2PWM_HFRC,
                 SMPS2_AUTO_SWITCH_PFM2PWM_THRES,
                 SMPS2_LPM_OVR_AUTO_SWITCH);
}

/*----------------------------------------------------------------------------
 * smps2_get_fsm_state
 *--------------------------------------------------------------------------*/

uint32_t smps2_get_fsm_state(void)
{
    RPMU_BASE_rpmu_Type *p_rpmu = _rpmu();
    return (uint32_t)p_rpmu->RPMU_R_RO_PMU_SMPS2R_1.bit.RO_SMPS2_FSM;
}

/*----------------------------------------------------------------------------
 * smps2_force_pfm_then_auto
 *--------------------------------------------------------------------------*/

bool smps2_force_pfm_then_auto(void)
{
    RPMU_BASE_rpmu_Type *p_rpmu = _rpmu();
    uint32_t fsm;
    uint32_t timeout_ms = SMPS2_FSM_TRANSITION_TIMEOUT_MS;

    /* Step 1: force SMPS2 to PFM */
    p_rpmu->RPMU_R_PMU_SMPS2_5.bit.SMPS2_LPM_OVR = SMPS2_LPM_OVR_FORCE_PFM;

    /* Step 2: poll ro_smps2_fsm until PFM or timeout */
    do {
        fsm = smps2_get_fsm_state();
        if (fsm == SMPS2_FSM_PFM) {
            break;
        }
        /* ~1 ms busy-wait using nop delay (calibrated elsewhere in the codebase) */
        nt_socpm_nop_delay(1000);
        timeout_ms--;
    } while (timeout_ms > 0);

    if (fsm != SMPS2_FSM_PFM) {
        NT_LOG_PRINT(SOCPM, WARN,
                     "smps2_force_pfm_then_auto: timeout waiting for PFM "
                     "(fsm=%u)", fsm);
        /*
         * Even on timeout, re-enable auto-switch so HW can recover.
         * Do NOT leave lpm_ovr=3 permanently.
         */
        p_rpmu->RPMU_R_PMU_SMPS2_5.bit.SMPS2_LPM_OVR = SMPS2_LPM_OVR_AUTO_SWITCH;
        return false;
    }

    /* Step 3: re-enable HW auto-switch (PFM->PWM by HW) */
    p_rpmu->RPMU_R_PMU_SMPS2_5.bit.SMPS2_LPM_OVR = SMPS2_LPM_OVR_AUTO_SWITCH;

    SMPS2_LOG("smps2_force_pfm_then_auto: switched to PFM, auto-switch re-enabled");
    return true;
}

/*----------------------------------------------------------------------------
 * smps2_pfm_monitor_task
 *
 * Waits for SMPS2_PFM_MONITOR_SIGNAL_MASK, then reads the latest Vbatt/Temp
 * values from the PMU TS module and decides whether to switch SMPS2 to PFM.
 * This task is woken by smps2_low_vbat_notify_vbat_updated(), which is called
 * from the Vbatt-done ISR (pmu_ccpu_vbat_mon_done_intr) after each periodic
 * Vbatt measurement completes.
 *--------------------------------------------------------------------------*/

static void smps2_pfm_monitor_task(void __attribute__((__unused__)) *arg)
{
    for (;;) {
        qurt_signal_wait(g_smps2_pfm_signal,
                         SMPS2_PFM_MONITOR_SIGNAL_MASK,
                         QURT_SIGNAL_ATTR_WAIT_ANY | QURT_SIGNAL_ATTR_CLEAR_MASK);

        uint32_t vbat_mV = tv_monitor_get_vbat_mV();
        int32_t  temp_C  = pmu_ts_get_current_temperature();

        uint32_t fsm = smps2_get_fsm_state();
        SMPS2_LOG("smps2_pfm_monitor_task: vbat_mV=%umV temp=%dC fsm=%u",
                  vbat_mV, temp_C, fsm);

        /*apply Vbatt-dependent register values */
        if (vbat_mV < SMPS2_LOW_VBAT_THRESHOLD_MV &&
            g_smps2_low_vbat_state.low_vbat_regs_applied == false) {
            smps2_set_low_vbat_regs(true);
        }
        else if(vbat_mV >= SMPS2_LOW_VBAT_THRESHOLD_MV &&
                g_smps2_low_vbat_state.low_vbat_regs_applied == true){
            smps2_set_low_vbat_regs(false);
        }

        if (smps2_can_use_pfm(vbat_mV, temp_C)) {

            if (fsm != SMPS2_FSM_PWM) {
                SMPS2_LOG("smps2_pfm_monitor_task: already in PFM (fsm=%u) — nothing to do",
                          fsm);
                continue;
            }
            SMPS2_LOG("smps2_pfm_monitor_task: PWM->PFM switch "
                      "(vbat=%umV temp=%dC)", vbat_mV, temp_C);
            (void)smps2_force_pfm_then_auto();
        }
    }
}

/*----------------------------------------------------------------------------
 * smps2_low_vbat_notify_vbat_updated
 *
 * Called from the Vbatt-done ISR (pmu_ccpu_vbat_mon_done_intr) to wake the
 * PFM monitor task after a new Vbatt measurement is available.
 * Safe to call from ISR context.
 *--------------------------------------------------------------------------*/

void smps2_low_vbat_notify_vbat_updated(void)
{
    if (g_smps2_pfm_signal) {
        qurt_signal_set(g_smps2_pfm_signal, SMPS2_PFM_MONITOR_SIGNAL_MASK);
    }
}

/*----------------------------------------------------------------------------
 * smps2_low_vbat_init
 *--------------------------------------------------------------------------*/

void smps2_low_vbat_init(uint32_t vbat_mV, int32_t temp_C)
{
    uint32_t fsm = smps2_get_fsm_state();

    NT_LOG_PRINT(SOCPM, ERR,
                 "smps2_low_vbat_init: vbat=%umV temp=%dC", vbat_mV, temp_C);

    NT_LOG_PRINT(SOCPM, ERR,
                 "smps2_low_vbat_init: SMPS mode is %d", fsm);
    /*apply Vbatt-dependent register values */
    smps2_set_low_vbat_regs(vbat_mV < SMPS2_LOW_VBAT_THRESHOLD_MV);

    /*one-time auto-switch configuration */
    smps2_init_auto_switch();

    /*decide initial PFM/PWM mode */
    if (smps2_can_use_pfm(vbat_mV, temp_C)) {
        (void)smps2_force_pfm_then_auto();
        NT_LOG_PRINT(SOCPM, ERR,"smps2_low_vbat_init: conditions allow PFM -> forcing PFM, auto switch enabled");
    } else {
         NT_LOG_PRINT(SOCPM, ERR,"smps2_low_vbat_init: conditions require PWM -> staying in PWM");
        /*
         * Cold-boot default is already PWM.
         * auto-switch (lpm_ovr=2) was set in smps2_init_auto_switch(),
         * so HW will automatically switch to PWM if temperature rises.
         */
    }

    /*
     * NOTE: Task creation is intentionally NOT done here.
     * smps2_low_vbat_init() is called from nt_socpm_secondary_init() which runs
     * in the Zephyr pre-kernel SYS_INIT phase, before the scheduler is started.
     * Creating a thread at that point puts it in the ready queue but the scheduler
     * never dispatches it.
     *
     * smps2_low_vbat_start_monitor_task() must be called separately from a task
     * context (e.g. at the end of pmu_ts_init(), which runs inside nt_wlan_main_task).
     */
}

/*----------------------------------------------------------------------------
 * smps2_low_vbat_start_monitor_task
 *
 * Creates the PFM monitor task and its signal.
 * Must be called from a task context (after the Zephyr scheduler has started),
 * NOT from a SYS_INIT / pre-kernel context.
 *
 * Intended call site: end of pmu_ts_init() in wifi_fw_pmu_ts_cfg.c.
 *--------------------------------------------------------------------------*/

void smps2_low_vbat_start_monitor_task(void)
{
    if (g_smps2_pfm_task_hnd != (TaskHandle_t)NULL) {
        return; /* already created */
    }

    if (qurt_signal_create(&g_smps2_pfm_signal) != 0) {
        NT_LOG_PRINT(SOCPM, WARN,
                     "smps2_low_vbat_start_monitor_task: qurt_signal_create failed");
        return;
    }

    nt_qurt_thread_create(smps2_pfm_monitor_task,
                          "smps2_pfm_mon",
                          SMPS2_PFM_MONITOR_TASK_STACK,
                          NULL,
                          SMPS2_PFM_MONITOR_TASK_PRIORITY,
                          &g_smps2_pfm_task_hnd);

    SMPS2_LOG("smps2_low_vbat_start_monitor_task: PFM monitor task created");
}

/*----------------------------------------------------------------------------
 * smps2_set_pfm_temp_thresholds
 *--------------------------------------------------------------------------*/

bool smps2_set_pfm_temp_thresholds(int32_t t0, int32_t t1, int32_t t2, int32_t t3)
{
    /* Validate: thresholds must be strictly increasing */
    if (t0 >= t1 || t1 >= t2 || t2 >= t3) {
        NT_LOG_PRINT(SOCPM, WARN,
                     "smps2_set_pfm_temp_thresholds: invalid thresholds "
                     "(%d %d %d %d) — must be strictly increasing",
                     t0, t1, t2, t3);
        return false;
    }

    pfm_temp_max_c[0] = t0;
    pfm_temp_max_c[1] = t1;
    pfm_temp_max_c[2] = t2;
    pfm_temp_max_c[3] = t3;

    NT_LOG_PRINT(SOCPM, INFO,
                 "smps2_set_pfm_temp_thresholds: updated to [%d, %d, %d, %d]",
                 pfm_temp_max_c[0], pfm_temp_max_c[1],
                 pfm_temp_max_c[2], pfm_temp_max_c[3]);
    return true;
}

/*----------------------------------------------------------------------------
 * smps2_get_pfm_temp_thresholds
 *--------------------------------------------------------------------------*/

void smps2_get_pfm_temp_thresholds(int32_t out[PFM_TEMP_COLS])
{
    for (int i = 0; i < PFM_TABLE_TEMP_COLS; i++) {
        out[i] = pfm_temp_max_c[i];
    }
}

/*----------------------------------------------------------------------------
 * smps2_set_pfm_guardband
 *--------------------------------------------------------------------------*/

void smps2_set_pfm_guardband(int32_t temp_gb_c, uint32_t vbat_gb_mv)
{
    g_pfm_temp_guardband_c  = temp_gb_c;
    g_pfm_vbat_guardband_mv = vbat_gb_mv;

    NT_LOG_PRINT(SOCPM, INFO,
                 "smps2_set_pfm_guardband: temp_gb=%dC vbat_gb=%umV",
                 g_pfm_temp_guardband_c, g_pfm_vbat_guardband_mv);
}

/*----------------------------------------------------------------------------
 * smps2_get_pfm_guardband
 *--------------------------------------------------------------------------*/

void smps2_get_pfm_guardband(int32_t *out_temp_gb_c, uint32_t *out_vbat_gb_mv)
{
    if (out_temp_gb_c) {
        *out_temp_gb_c = g_pfm_temp_guardband_c;
    }
    if (out_vbat_gb_mv) {
        *out_vbat_gb_mv = g_pfm_vbat_guardband_mv;
    }
}

/*----------------------------------------------------------------------------
 * smps2_set_log_verbose
 *--------------------------------------------------------------------------*/

void smps2_set_log_verbose(bool enable)
{
    if (enable) {
        g_smps2_log_verbose = SMPS2_LOG_VERBOSE_ON;
    }else{
        g_smps2_log_verbose = SMPS2_LOG_VERBOSE_OFF;
    }
}

/*----------------------------------------------------------------------------
 * smps2_get_log_verbose
 *--------------------------------------------------------------------------*/

bool smps2_get_log_verbose(void)
{
    if (g_smps2_log_verbose) {
        return true;
    }else{
        return false;
    }
}

