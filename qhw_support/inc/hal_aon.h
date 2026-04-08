/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdint.h>

void qaon_init();

uint32_t qaon_get_counter32();

uint64_t qaon_get_counter64();

void qaon_start_count();

void qaon_stop_count();

void qaon_set_alarm(uint64_t expire);

void qaon_clear_interrupt();
