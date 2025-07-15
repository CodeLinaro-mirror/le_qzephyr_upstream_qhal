/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/
#include <qlib_util.h>

SCB_Type * g_SCB = SCB;
NVIC_Type * g_NVIC = NVIC;
SysTick_Type *g_SysTick = SysTick;

volatile uint32_t g32_dead_loop_1 = 0;
volatile uint32_t g32_dead_loop_2 = 0;
volatile uint32_t g32_bss_test;

void dead_loop (void)
{
    while (1);
}

void dead_loop_cond1 (void)
{
    while (g32_dead_loop_1);
}

void dead_loop_cond2 (void)
{
    while (g32_dead_loop_2);
}

void nop_delay( uint32_t n )
{
   uint32_t nop_count = 0;
   for( nop_count = 0; nop_count < n; nop_count++)
   {
       __asm volatile(" nop \n");
   }
}

