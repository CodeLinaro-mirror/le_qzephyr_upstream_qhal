/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/**********************************************************************************************
 * @file smps2_low_vbat.h
 * @brief SMPS2 Low Vbatt (2.0V~2.6V) Support
 *
 * This module handles SMPS2 PFM/PWM mode switching when Vbatt is below 2.6V.
 *
 * Background:
 *   SMPS2 (CX power rail) normally requires Vbatt >= 2.6V.
 *   This feature extends support down to 2.0V by:
 *     1. Applying Vbatt-range-dependent PMU register values when Vbatt < 2.6V
 *     2. Configuring HW auto-switch (PFM->PWM) feature
 *     3. SW-controlled PWM->PFM switching based on Temp/Vbat lookup table
 *
 * SMPS2 FSM states (ro_smps2_fsm):
 *   3 = PWM
 *   4 = PFM
 *
 * smps2_lpm_ovr values:
 *   2 = enable HW auto-switch (PFM->PWM by HW, PWM->PFM by SW)
 *   3 = force PFM
 *
 * SMPS2_PMIN_ON / SMPS2_NMIN_ON register definition:
 *   Minimum PMOS on-time = 8ns * PMIN_ON value
 *   Minimum NMOS on-time = 8ns * NMIN_ON value
 *
 *   Recommended values by Vbatt range (from HW spec):
 *     Vbatt 1.8V ~ 2.4V : PMIN_ON = 6, NMIN_ON = 5
 *     Vbatt 2.4V ~ 3.0V : PMIN_ON = 5, NMIN_ON = 4
 *     Vbatt 3.0V ~ 3.6V : PMIN_ON = 4, NMIN_ON = 3  (chip default)
 *
 *   The email spec says "pmin_on=6, nmin_on=5 for Vbatt < 2.6V".
 *   Per the HW register definition above, this is the conservative choice
 *   covering the entire sub-2.6V range (both 1.8-2.4V and 2.4-3.0V bands).
 *   SW applies the stricter 1.8-2.4V values for the whole < 2.6V range as
 *   a safe guard-band until HW provides finer-grained characterisation data.
 *
 *********************************************************************************************/

#ifndef _SMPS2_LOW_VBAT_H_
#define _SMPS2_LOW_VBAT_H_

#include <stdint.h>
#include <stdbool.h>

/*-----------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *----------------------------------------------------------------------------*/

/** Vbatt threshold (mV) below which non-default SMPS2 register values are applied */
#define SMPS2_LOW_VBAT_THRESHOLD_MV     2600

/**
 * SMPS2 PMIN_ON / NMIN_ON values by Vbatt range.
 *
 * HW register definition:
 *   SMPS2_PMIN_ON: Minimum PMOS on-time = 8ns * value
 *   SMPS2_NMIN_ON: Minimum NMOS on-time = 8ns * value
 *
 * Recommended by HW spec:
 *   Vbatt 1.8V ~ 2.4V  ->  PMIN_ON=6, NMIN_ON=5
 *   Vbatt 2.4V ~ 3.0V  ->  PMIN_ON=5, NMIN_ON=4
 *   Vbatt 3.0V ~ 3.6V  ->  PMIN_ON=4, NMIN_ON=3  (chip default)
 *
 * SW policy (conservative, pending skewed-parts data from HW):
 *   Apply the 1.8-2.4V values (pmin=6, nmin=5) for the entire Vbatt < 2.6V
 *   range, matching the email spec requirement.
 */
#define SMPS2_PMIN_ON_VBAT_LOW          6   /* Vbatt 1.8V ~ 2.4V (also used for < 2.6V per email spec) */
#define SMPS2_NMIN_ON_VBAT_LOW          5   /* Vbatt 1.8V ~ 2.4V (also used for < 2.6V per email spec) */

#define SMPS2_PMIN_ON_VBAT_MID          5   /* Vbatt 2.4V ~ 3.0V */
#define SMPS2_NMIN_ON_VBAT_MID          4   /* Vbatt 2.4V ~ 3.0V */

#define SMPS2_PMIN_ON_VBAT_HIGH         4   /* Vbatt 3.0V ~ 3.6V (chip default) */
#define SMPS2_NMIN_ON_VBAT_HIGH         3   /* Vbatt 3.0V ~ 3.6V (chip default) */

/** CL_ILIM_MIN non-default value for Vbatt < 2.6V (from HW spec) */
#define SMPS2_CL_ILIM_MIN_LOW_VBAT      4

/** SMPS2 default register values (Vbatt >= 2.6V) */
#define SMPS2_DEFAULT_PMIN_ON           SMPS2_PMIN_ON_VBAT_MID  /* = 5 */
#define SMPS2_DEFAULT_NMIN_ON           SMPS2_NMIN_ON_VBAT_MID  /* = 4 */
#define SMPS2_DEFAULT_CL_ILIM_MIN       0x7  /* SMPS2_CL_ILIM_MIN 4/ 7/ 11d for Vbatt 1.8-2.4V/ 2.4-3.0V/ 3.0-3.6 V */

/** One-time auto-switch configuration values (from HW spec) */
#define SMPS2_AUTO_SWITCH_EA_HOLD       0
#define SMPS2_AUTO_SWITCH_PFM2PWM_HFRC  7
#define SMPS2_AUTO_SWITCH_PFM2PWM_THRES 1

/** smps2_lpm_ovr values */
#define SMPS2_LPM_OVR_AUTO_SWITCH      2   /* Enable HW auto-switch (PFM->PWM by HW) */
#define SMPS2_LPM_OVR_FORCE_PFM        3   /* Force SMPS2 to PFM */

/** ro_smps2_fsm read-only status values */
#define SMPS2_FSM_PWM                   3
#define SMPS2_FSM_PFM                   4

/** Timeout (ms) to wait for SMPS2 FSM to reach expected state after lpm_ovr write */
#define SMPS2_FSM_TRANSITION_TIMEOUT_MS 10

/*-----------------------------------------------------------------------------
 * PFM Usability Lookup Table
 *
 * Based on HW characterization data.
 * Table indicates whether SMPS2 can operate in PFM at given Vbatt and temperature.
 * Guard-band will be added after HW provides data from skewed parts.
 *
 * Vbatt breakpoints (mV): 2000, 2200, 2400, 2600+
 * Temp  breakpoints (°C): 30,   50,   70,   90
 *
 * Entry = 1 means PFM is safe; 0 means must stay in PWM.
 *
 * Raw HW data (no guard-band yet):
 *   Vbatt=2.6V: Y Y Y Y  (all temps OK)
 *   Vbatt=2.4V: Y Y Y N  (fail at 90C)
 *   Vbatt=2.2V: Y Y N N  (fail at 70C+)
 *   Vbatt=2.0V: Y N N N  (fail at 50C+)
 *
 * Guard-band applied (conservative, pending skewed-parts data):
 *   Temperature: add SMPS2_PFM_TEMP_GUARDBAND_C to measured temp before lookup
 *   Vbatt:       subtract SMPS2_PFM_VBAT_GUARDBAND_MV from measured Vbat before lookup
 *
 * Example with guard-band (temp_gb = temp + 5, vbat_gb = vbat - 50):
 *   At Vbat=2.4V, Temp=80C -> effective (2350mV, 85C) -> lookup row 2.2V, col 90C -> FAIL -> stay PWM
 *   At Vbat=2.4V, Temp=60C -> effective (2350mV, 65C) -> lookup row 2.2V, col 70C -> FAIL -> stay PWM
 *   At Vbat=2.4V, Temp=40C -> effective (2350mV, 45C) -> lookup row 2.2V, col 50C -> PASS -> can PFM
 *----------------------------------------------------------------------------*/

/** Guard-band values — to be tuned after HW provides skewed-parts data */
#define SMPS2_PFM_TEMP_GUARDBAND_C      5   /* °C margin: use (actual_temp + 5C) for lookup */
#define SMPS2_PFM_VBAT_GUARDBAND_MV     50  /* mV margin: use (actual_vbat - 50mV) for lookup */

/** Number of temperature threshold columns in the PFM lookup table */
#define PFM_TEMP_COLS                   4

/*-----------------------------------------------------------------------------
 * Verbose / Debug Logging Control
 *
 * g_smps2_log_verbose controls the log level used by smps2_low_vbat.c:
 *   0 (default) : use INFO  — quiet, only configuration messages
 *   1           : use ERR   — verbose, all decision/state-change messages
 *
 * Can be set at runtime via the shell command:
 *   qbmps smps2_set_verbose <0|1>
 *
 * Can also be written directly by address (use smps2_pfm_addr.py to find it):
 *   write32(<addr_of_g_smps2_log_verbose>, 1)   # enable verbose
 *   write32(<addr_of_g_smps2_log_verbose>, 0)   # disable verbose
 *----------------------------------------------------------------------------*/
#define SMPS2_LOG_VERBOSE_OFF   0   /**< Use INFO level (default, quiet) */
#define SMPS2_LOG_VERBOSE_ON    1   /**< Use ERR  level (verbose debug)  */

/*-----------------------------------------------------------------------------
 * Type Declarations
 *----------------------------------------------------------------------------*/

/**
 * @brief SMPS2 low-Vbatt state tracking structure
 */
typedef struct {
    bool low_vbat_regs_applied;   /**< true if non-default regs for Vbat<2.6V are active */
    bool auto_switch_configured;  /**< true if one-time auto-switch setup has been done */
} smps2_low_vbat_state_t;

/*-----------------------------------------------------------------------------
 * Function Declarations
 *----------------------------------------------------------------------------*/

/**
 * @brief  Query whether SMPS2 can safely operate in PFM at given conditions.
 *
 * Uses the HW characterization lookup table with guard-band applied:
 *   effective_temp = temp_C + SMPS2_PFM_TEMP_GUARDBAND_C
 *   effective_vbat = vbat_mV - SMPS2_PFM_VBAT_GUARDBAND_MV
 *
 * @param  vbat_mV  Vbatt in millivolts (from tv_monitor_get_vbat_mV())
 * @param  temp_C   Temperature in degrees Celsius (from pmu_ts_get_current_temperature())
 * @return true if PFM is safe, false if must stay in PWM
 */
bool smps2_can_use_pfm(uint32_t vbat_mV, int32_t temp_C);

/**
 * @brief  Apply or restore SMPS2 PMIN_ON / NMIN_ON / CL_ILIM_MIN based on Vbatt level.
 *
 * Register values applied per Vbatt range (HW spec):
 *   Vbatt < 2.6V  : PMIN_ON=6, NMIN_ON=5, CL_ILIM_MIN=4  (conservative 1.8-2.4V values)
 *   Vbatt >= 2.6V : PMIN_ON=4, NMIN_ON=3, CL_ILIM_MIN=11 (chip defaults)
 *
 * Note: SW applies the stricter 1.8-2.4V band values for the entire < 2.6V range
 * as a safe guard-band, matching the email spec (pmin=6, nmin=5 for Vbatt < 2.6V).
 *
 * @param  low_vbat  true = apply low-Vbat non-default values (Vbat < 2.6V)
 *                   false = restore default values (Vbat >= 2.6V)
 */
void smps2_set_low_vbat_regs(bool low_vbat);

/**
 * @brief  One-time initialization of SMPS2 auto-switch feature.
 *
 * Writes:
 *   smps2_ea_hold       = 0  (RPMU_R_PMU_SMPS2_3)
 *   smps2_pfm2pwm_hfrc  = 7  (RPMU_R_PMU_SMPS2_6)
 *   smps2_pfm2pwm_thres = 1  (RPMU_R_PMU_SMPS2_6)
 *   smps2_lpm_ovr       = 2  (RPMU_R_PMU_SMPS2_5, enable HW auto-switch)
 *
 * Should be called once during system init (after cold boot decisions).
 * Idempotent: subsequent calls are no-ops (guarded by auto_switch_configured flag).
 */
void smps2_init_auto_switch(void);

/**
 * @brief  Force SMPS2 to PFM, then re-enable auto-switch.
 *
 * Sequence:
 *   1. Write smps2_lpm_ovr = 3 (force PFM)
 *   2. Poll ro_smps2_fsm until == SMPS2_FSM_PFM (or SMPS2_FSM_TRANSITION_TIMEOUT_MS)
 *   3. Write smps2_lpm_ovr = 2 (re-enable HW auto-switch)
 *
 * @return true if transition to PFM succeeded, false on timeout
 */
bool smps2_force_pfm_then_auto(void);

/**
 * @brief  Read current SMPS2 FSM state from read-only register.
 *
 * @return SMPS2_FSM_PWM (3), SMPS2_FSM_PFM (4), or 0 on read error
 */
uint32_t smps2_get_fsm_state(void);

/**
 * @brief  Top-level init called after cold boot Vbatt/Temp measurement.
 *
 * Performs:
 *   1. If Vbatt < SMPS2_LOW_VBAT_THRESHOLD_MV: apply low-Vbat register values
 *   2. Configure auto-switch feature (one-time)
 *   3. Decide initial PFM/PWM mode:
 *        - If smps2_can_use_pfm() -> force PFM then re-enable auto-switch
 *        - Otherwise             -> stay in PWM (cold-boot default), auto-switch enabled
 *
 * @param  vbat_mV  Bootup Vbatt in millivolts  (from pmu_ts_init measurement)
 * @param  temp_C   Bootup temperature in °C    (from pmu_ts_init measurement)
 */
void smps2_low_vbat_init(uint32_t vbat_mV, int32_t temp_C);

/**
 * @brief  Create the PFM monitor task and its signal object.
 *
 * MUST be called from a task context (after the Zephyr scheduler has started).
 * Do NOT call from SYS_INIT / pre-kernel context — the scheduler will not
 * dispatch threads created before it starts.
 *
 * Intended call site: end of pmu_ts_init() in wifi_fw_pmu_ts_cfg.c, which
 * runs inside nt_wlan_main_task (a proper task context).
 *
 * Idempotent: subsequent calls are no-ops if the task was already created.
 */
void smps2_low_vbat_start_monitor_task(void);

/**
 * @brief  Notify the PFM monitor task that a new Vbatt measurement is available.
 *
 * This function is intended to be called from the Vbatt-done ISR
 * (pmu_ccpu_vbat_mon_done_intr) after each periodic Vbatt measurement cycle.
 * It sets a signal to wake the smps2_pfm_monitor_task, which will then read
 * the latest Vbatt/Temp values and decide whether to switch SMPS2 from PWM
 * to PFM.
 *
 * Safe to call from ISR context (qurt_signal_set is ISR-safe).
 */
void smps2_low_vbat_notify_vbat_updated(void);

/**
 * @brief  Override the PFM temperature threshold table at runtime.
 *
 * The four values define the upper-bound temperature (°C) for each column of
 * the PFM usability lookup table:
 *   col 0: temp <= t0
 *   col 1: t0 < temp <= t1
 *   col 2: t1 < temp <= t2
 *   col 3: t2 < temp <= t3  (anything above t3 is treated as worst-case col 3)
 *
 * Default values: t0=30, t1=50, t2=70, t3=90
 *
 * Constraints: t0 < t1 < t2 < t3  (strictly increasing)
 *
 * @param  t0  Upper-bound temperature for column 0 (°C)
 * @param  t1  Upper-bound temperature for column 1 (°C)
 * @param  t2  Upper-bound temperature for column 2 (°C)
 * @param  t3  Upper-bound temperature for column 3 (°C)
 * @return true on success, false if thresholds are not strictly increasing
 */
bool smps2_set_pfm_temp_thresholds(int32_t t0, int32_t t1, int32_t t2, int32_t t3);

/**
 * @brief  Read back the current PFM temperature threshold table.
 *
 * @param  out  Output array of PFM_TEMP_COLS elements; filled with current thresholds.
 */
void smps2_get_pfm_temp_thresholds(int32_t out[PFM_TEMP_COLS]);

/**
 * @brief  Set the PFM guard-band values at runtime.
 *
 * Guard-bands are applied in smps2_can_use_pfm() before the table lookup:
 *   effective_temp = actual_temp + temp_gb_c
 *   effective_vbat = actual_vbat - vbat_gb_mv
 *
 * A larger guard-band makes the PFM decision more conservative (harder to enter PFM).
 * Setting both to 0 disables the guard-band entirely.
 *
 * Defaults: temp_gb_c = SMPS2_PFM_TEMP_GUARDBAND_C (5),
 *           vbat_gb_mv = SMPS2_PFM_VBAT_GUARDBAND_MV (50)
 *
 * @param  temp_gb_c   Temperature guard-band in °C (added to measured temp; may be negative)
 * @param  vbat_gb_mv  Vbatt guard-band in mV (subtracted from measured Vbat; unsigned)
 */
void smps2_set_pfm_guardband(int32_t temp_gb_c, uint32_t vbat_gb_mv);

/**
 * @brief  Read back the current PFM guard-band values.
 *
 * @param  out_temp_gb_c   Pointer to receive temperature guard-band (°C); may be NULL.
 * @param  out_vbat_gb_mv  Pointer to receive Vbatt guard-band (mV); may be NULL.
 */
void smps2_get_pfm_guardband(int32_t *out_temp_gb_c, uint32_t *out_vbat_gb_mv);

/**
 * @brief  Set the smps2 log verbose enable/disable.
 *
 * @param  enable   enable/disable flag.
 */
void smps2_set_log_verbose(bool enable);

/**
 * @brief  Get the smps2 log verbose enable/disable flag.
 *
 */
bool smps2_get_log_verbose(void);
#endif /* _SMPS2_LOW_VBAT_H_ */
