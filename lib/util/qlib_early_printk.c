/**  
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/
#include <qlib_early_printk.h>
#include <uart_hal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static char early_printk_buf[EARLY_PRINTK_BUF_SIZE];

void early_printk(const char *fmt, ...)
{
    va_list argp;

    memset((uint8_t *)(early_printk_buf), 0, sizeof(early_printk_buf));
    va_start(argp, fmt);
    vsnprintf(early_printk_buf, sizeof(early_printk_buf), fmt, argp);
    uart_hal_poll_out_str(g_uart_hal, early_printk_buf);
    va_end(argp);

    return;
}

void early_printk_init (void)
{
    uart_hal *l_uart_hal = g_uart_hal;

    uart_hal_init(l_uart_hal, 0);
	uart_hal_baudrate_set(l_uart_hal, 115200);
    uart_hal_parity_enable(l_uart_hal, UART_PARITY_NONE);
    uart_hal_data_bits_cfg(l_uart_hal, UART_DATA_BITS_8);
    uart_hal_stop_bits_cfg(l_uart_hal, UART_STOP_BITS_1);
}

//#define UART_ECHO_TEST

#ifdef UART_ECHO_TEST
#include <zephyr/irq.h>

struct dbg_uart {
	uint32_t uart_irq;
	uint32_t irq_raw;
	uint8_t ch;
};

struct dbg_uart g_dbg_uart = {0};

void uart_echo (uint32_t is_rx)
{
	uart_hal * l_uart_hal = g_uart_hal;
	uint32_t exit_cnt = 10;
	uint32_t rx_cnt = 0;
	uint32_t key = 0;
	uint32_t tx_cnt = 0;
	uint32_t unknown_irq_cnt = 0;
	uint32_t rx_zero_cnt = 0;
	struct dbg_uart *p_dbg_uart = &g_dbg_uart;

	early_printk("%s loop until UART char cnt==%d\r\n", __FUNCTION__, exit_cnt);
	if (is_rx) {
		early_printk("Please input UART by single char\r\n");
	} else {
		early_printk("Please output UART by writing reg on single char\r\n");
	}

	key = irq_lock();
	irq_enable(UART_IRQn);
	if (is_rx) {
		uart_hal_enable_intr_rx(l_uart_hal);
	} else {		
		uart_hal_enable_intr_tx(l_uart_hal);
	}

	//g32_dead_loop_1 = 1;
	//dead_loop_cond1();
	do {
		memset(p_dbg_uart, 0, sizeof(struct dbg_uart));
		p_dbg_uart->uart_irq = NVIC_GetPendingIRQ(UART_IRQn);
		if (!p_dbg_uart->uart_irq) {
			continue;
		}
		NVIC_ClearPendingIRQ(UART_IRQn);

		p_dbg_uart->irq_raw = uart_hal_intr_get(l_uart_hal) & IIR_IID_MASK;
		if (p_dbg_uart->irq_raw == IIR_THR_EMPTY) {
			tx_cnt++;
		} else if (p_dbg_uart->irq_raw == IIR_NO_INTR_PENDING) {
			p_dbg_uart->ch = uart_hal_rx_read(l_uart_hal);
			if (p_dbg_uart->ch) {
				uart_hal_poll_out(l_uart_hal, p_dbg_uart->ch);
				rx_cnt++;
			} else {
				rx_zero_cnt++;
			}
		} else {
			unknown_irq_cnt++;
		}

		if (is_rx) {
			if (rx_cnt >= exit_cnt) {
				break;
			}
		} else {
			if (tx_cnt >= exit_cnt) {
				break;
			}
		}
	} while (rx_cnt < exit_cnt);

	if (is_rx) {
		uart_hal_disable_intr_rx(l_uart_hal);
	} else {
		uart_hal_disable_intr_tx(l_uart_hal);
	}
	irq_disable(UART_IRQn);
	irq_unlock(key);

	early_printk("tx_cnt=%d rx_cnt=%d rx_zero_cnt=%d unknown_irq_cnt=%d \r\n", tx_cnt, rx_cnt, rx_zero_cnt, unknown_irq_cnt);
	early_printk("UART done\r\n");
}
#endif

