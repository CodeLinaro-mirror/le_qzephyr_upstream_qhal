/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QCC_UTIL_H_
#define QCC_UTIL_H_

#include <stdint.h>
extern volatile uint32_t g32_dead_loop_1;
extern volatile uint32_t g32_dead_loop_2;
void dead_loop(void);
void dead_loop_cond1(void);
void dead_loop_cond2(void);
void nop_delay(uint32_t n);

#endif /* QCC_UTIL_H_ */
