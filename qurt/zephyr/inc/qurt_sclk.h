/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QURT_SCLK_H
#define QURT_SCLK_H

#include <stdint.h>

#include <zephyr/kernel.h>

#if CONFIG_TIMER_HAS_64BIT_CYCLE_COUNTER
#define QURT_SYSCLOCK_MAX_DURATION (INT64_MAX - 1)
#else
#define QURT_SYSCLOCK_MAX_DURATION (UINT32_MAX - 1)
#endif

#define QURT_TIMER_MAX_DURATION QURT_SYSCLOCK_MAX_DURATION
#define QURT_TIMER_MIN_DURATION (Z_HZ_us / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC)

/** @brief Macro to check if duration is within range. Evaluates to QURT_EOK
           on success, QURT_EINVALID on failure */
#define QURT_TIMER_IS_DURATION_VALID(duration)                                                                         \
    (((((duration) >= QURT_TIMER_MIN_DURATION) && ((duration) <= QURT_TIMER_MAX_DURATION)) != 0) ? QURT_EOK            \
                                                                                                 : QURT_EINVALID)

/*
 Conversion from timer ticks to microseconds at the nominal frequency.
*/
#define QURT_SYSCLOCK_TIMETICK_TO_US(ticks) qurt_sysclock_timetick_to_us(ticks)

#if CONFIG_QURT_SCLK

/**@ingroup func_qurt_sysclock_get_hw_ticks
  @xreflabel{sec:qurt_sysclock_get_hw_ticks}
  Gets the hardware tick count.\n
  Returns the current value of a 64-bit hardware counter. The value wraps around to zero
  when it exceeds the maximum value.
  @note1hang This operation must be used with care because of the wrap-around behavior.

  @return
  Integer -- Current value of 64-bit hardware counter.
  @dependencies
  None.
 */
static inline unsigned long long qurt_sysclock_get_hw_ticks(void) { return k_cycle_get_64(); }

/**@ingroup func_qurt_sysclock_timetick_to_us
  @xreflabel{sec:qurt_sysclock_timetick_to_us}
  Convert sys ticks into micro seconds

  @return
  Integer -- Value of time in micro seconds
  @dependencies
  None.
 */
static inline unsigned long long qurt_sysclock_timetick_to_us(unsigned long long ticks)
{
    return ticks * Z_HZ_us / CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC;
}

#endif // CONFIG_QURT_SCLK

#endif
