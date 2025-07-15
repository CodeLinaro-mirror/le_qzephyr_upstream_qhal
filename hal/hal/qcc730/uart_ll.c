/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/


#include <uart_ll.h>
#include <pmu_ll.h>

typedef struct {
	uint32_t		baudrate;
	uint32_t		divisor;
} uart_baudrate;

#define DIVISOR_DLL(divisor)		(divisor & 0xff)
#define DIVISOR_DLH(divisor)		((divisor >> 8) & 0xff)
#define BAUDRATE_NUM_MAX			9

uart_hal * g_uart_hal = (uart_hal *)(QCC730V2_UART_BASE_BASE);

static const uart_baudrate baudrate_table[BAUDRATE_NUM_MAX] = {
	{921600, 0x04},
	{460800, 0x08},
	{230400, 0x10},
	{115200, 0x20},
	{57600,  0x40},
	{38400,  0x60},
	{19200,  0xc0},
	{9600,  0x180},
	{4800,  0x300},
};

void uart_ll_enable (uart_hal *hal, uint32_t enable)
{
	pmu_hal * pmu = g_pmu_hal;

	(void)(hal); //ARG_UNUSED(hal);
	if (enable) {
		//Power up PSS domain for UART
		pmu->PMU_CFG_AON_CNTL_MCU_SYSTEM_BOOT_COMPLETE_STATE_RESOURCE_REQ.bit.PD_PSS_CNTL_BIT = enable;
		pmu->PMU_ROOT_CLK_ENABLE.bit.UART_ROOT_CLK_ENABLE = enable;
		pmu->PMU_BOOT_STRAP_CONFIG_SECURE.reg = BOOT_STRAP_VALUE;
		pmu->PMU_BOOT_STRAP_CONFIGURATION_STATUS.bit.CFG_UART_ENABLE = enable;

		pmu->PMU_BOOT_STRAP_CONFIG_SECURE.reg = BOOT_STRAP_VALUE;
        pmu->PMU_BOOT_STRAP_CONFIGURATION_STATUS.bit.CFG_UART_OPTION = CONFIG_BOARD_QCC730_UART_GPIO_OPTION;
	} else {
		pmu->PMU_BOOT_STRAP_CONFIG_SECURE.reg = BOOT_STRAP_VALUE;
		pmu->PMU_BOOT_STRAP_CONFIGURATION_STATUS.bit.CFG_UART_ENABLE = enable;
		pmu->PMU_ROOT_CLK_ENABLE.bit.UART_ROOT_CLK_ENABLE = enable;
		pmu->PMU_CFG_AON_CNTL_MCU_SYSTEM_BOOT_COMPLETE_STATE_RESOURCE_REQ.bit.PD_PSS_CNTL_BIT = enable;
	}
}

void uart_ll_reset (uart_hal *hal)
{
	uart_hal *l_uart_hal = hal;
	l_uart_hal->UART_UART_DLH.reg = 0;
	l_uart_hal->UART_UART_MCR.reg = 0;
	l_uart_hal->UART_UART_IIR.reg = 0;
	uart_hal_loopback_enable(hal, 0);
}

void uart_ll_parity_set(uart_hal *hal, uart_parity parity)
{
	if (!hal) {
		return;
	}

	/* Configure parity */
	if (parity == UART_PARITY_EVEN) {
		uart_hal_parity_enable(hal, 1);
		uart_hal_event_parity_select(hal, 1);
	} else if (parity == UART_PARITY_ODD) {
		uart_hal_parity_enable(hal, 1);
		uart_hal_event_parity_select(hal, 0);
	} else {
		uart_hal_parity_enable(hal, 0);
	}
}
static int uart_divisor_by_baudrate(uint32_t baudrate, uint32_t *divisor)
{
	uint32_t i = 0;

	if (!divisor) {
		return -1;
	}

	while (i < (sizeof(baudrate_table) / sizeof(uart_baudrate))) {
		if (baudrate_table[i].baudrate == baudrate) {
			*divisor = baudrate_table[i].divisor;
			return 0;
		}

		i ++;
	}

	return -1;
}

static void uart_baudrate_divisor_set(uart_hal *hal, uint32_t divisor)
{
	uart_hal_divisor_access(hal, 1);
	uart_hal_divisor_low_cfg(hal, DIVISOR_DLL(divisor));
	uart_hal_divisor_high_cfg(hal, DIVISOR_DLH(divisor));
	uart_hal_divisor_access(hal, 0);
}

int uart_ll_baudrate_set (uart_hal *hal, uint32_t baudrate)
{
	uint32_t divisor = 0;

	if (uart_divisor_by_baudrate(baudrate, &divisor)) {
		return -1;
	}

	uart_baudrate_divisor_set(hal, divisor);
	return 0;
}

