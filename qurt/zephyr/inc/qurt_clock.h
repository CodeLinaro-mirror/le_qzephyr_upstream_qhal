/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _QURT_CLOCK_H_
#define _QURT_CLOCK_H_

#include <stdint.h>

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
unsigned long long qurt_sysclock_get_hw_ticks(void);

/**@ingroup func_qurt_sysclock_timetick_to_us
  @xreflabel{sec:qurt_sysclock_timetick_to_us}
  Convert sys ticks into micro seconds

  @return
  Integer -- Value of time in micro seconds
  @dependencies
  None.
 */
unsigned long long qurt_sysclock_timetick_to_us(unsigned long long ticks);

#endif /* CONFIG_QURT_SCLK */

#endif /* QURT_SCLK_H */
