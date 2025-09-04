/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <zephyr/toolchain.h>
#include <fermion_hw_reg.h>

static ALWAYS_INLINE uint32_t qaon_hal_read_32(unsigned long address)
{
	uint32_t v = (*(volatile uint32_t *)address);
	__asm volatile("dsb" ::: "memory");
	return v;
}

static ALWAYS_INLINE void qaon_hal_write_32(unsigned long address, uint32_t v)
{
	(*(volatile uint32_t *)address) = v;
	__asm volatile("dsb" ::: "memory");
}

uint32_t qaon_get_counter32()
{
	return qaon_hal_read_32(QWLAN_PMU_SLP_TMR_VAL_LSB_REG);
}

uint64_t qaon_get_counter64()
{
	uint64_t v = qaon_hal_read_32(QWLAN_PMU_SLP_TMR_VAL_MSB_REG);
	v = (v << 32) | qaon_hal_read_32(QWLAN_PMU_SLP_TMR_VAL_LSB_REG);
	return v;
}

void qaon_start_count()
{
	qaon_hal_write_32(QWLAN_PMU_SLP_TMR_CTL_REG, QWLAN_PMU_SLP_TMR_CTL_SLP_TMR_EN_MASK | QWLAN_PMU_SLP_TMR_CTL_SLP_TMR_LEGACY_MODE_MASK);
}

void qaon_stop_count()
{
	uint32_t v = qaon_hal_read_32(QWLAN_PMU_SLP_TMR_CTL_REG);
	v &= ~QWLAN_PMU_SLP_TMR_CTL_SLP_TMR_EN_MASK;
	qaon_hal_write_32(QWLAN_PMU_SLP_TMR_CTL_REG, v);
}

void qaon_set_alarm(uint64_t expire)
{
	qaon_hal_write_32(QWLAN_PMU_WLAN_SLP_TMR_EXP_MSB_REG, expire >> 32);
	qaon_hal_write_32(QWLAN_PMU_WLAN_SLP_TMR_EXP_LSB_REG, expire & 0xffffffff);
}

void qaon_clear_interrupt()
{
	/* write 1, then 0, to clear interrupt. */
	qaon_hal_write_32(QWLAN_PMU_AON_SLP_TIMER_INT_CLR_REG, QWLAN_PMU_AON_SLP_TIMER_INT_CLR_AON_SLP_TIMER_INT_CLR_MASK);
	__asm__ volatile("nop \r\n");
	qaon_hal_write_32(QWLAN_PMU_AON_SLP_TIMER_INT_CLR_REG, QWLAN_PMU_AON_SLP_TIMER_INT_CLR_DEFAULT);
}

void qaon_init()
{
	/* enable wlan sleep timer interrupt */
	uint32_t v = qaon_hal_read_32(QWLAN_PMU_WLAN_SLP_TMR_CTL_REG);
	v |= QWLAN_PMU_WLAN_SLP_TMR_CTL_WLAN_SLP_TMR_INT_EN_MASK;
	qaon_hal_write_32(QWLAN_PMU_WLAN_SLP_TMR_CTL_REG, v);

	v = qaon_hal_read_32(QWLAN_PMU_AON_LIC_INT_EN_REG);
	v |= QWLAN_PMU_AON_LIC_INT_EN_WLAN_WAKEUP_INTR_EN_MASK;
	qaon_hal_write_32(QWLAN_PMU_AON_LIC_INT_EN_REG, v);
}
