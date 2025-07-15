/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdint.h>
#include "nt_osal.h"
#include <zephyr/autoconf.h>

/**
 * <!-- nt_delay -->
 *
 * @brief Normal delay function
 * @param time: Delay time needed in milli seconds
 * @return: void
 */
void nt_normal_delay(uint32_t time)
{
    volatile int32_t i,j, value;
    value = (volatile int32_t)(time * (10^6)*3);
    for(i = value ; i>=0 ; i--)
    {
       for(j = 100;j>=0;j--)
       {
         __asm volatile("nop");
       }
    }
}

int tickless_idle_enabled (void)
{
#ifdef CONFIG_PM
    return 1;
#else
    return 0;
#endif
}

