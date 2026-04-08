/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <string.h>
#include <uart_hal.h>
#include <zephyr/irq.h>

void uart_hal_init(uart_hal *hal, uint32_t uart_num)
{
    irq_disable(uart_intr);
    uart_ll_enable(hal, 1);
    uart_ll_reset(hal);
    uart_hal_disable_intr_tx(hal);
    uart_hal_disable_intr_rx(hal);
}

void uart_hal_parity_set(uart_hal *hal, uart_parity parity) { uart_ll_parity_set(hal, parity); }

int uart_hal_baudrate_set(uart_hal *hal, uint32_t baudrate) { return uart_ll_baudrate_set(hal, baudrate); }

void uart_hal_poll_out(uart_hal *hal, uint8_t ch)
{
    uint16_t timeout = 0;
    volatile uint32_t tx_empty = 0;
    uart_hal *l_uart_hal = hal;
    // Check the transmit holding register empty bit
    do {
        timeout++;
        tx_empty = uart_hal_tx_empty(l_uart_hal);
    } while ((tx_empty == 0x00) && (timeout < 1000));
    // Write to the transmit holding register
    uart_hal_tx_write(l_uart_hal, ch);
}

void uart_hal_poll_out_ext(uint8_t ch) { uart_hal_poll_out(g_uart_hal, ch); }

void uart_hal_poll_outs(uart_hal *hal, const char *data, uint32_t length)
{
    while (length--) {
        uart_hal_poll_out(hal, *data++);
    }
}

void uart_hal_poll_out_str(uart_hal *hal, const char *str) { uart_hal_poll_outs(hal, str, strlen(str)); }

void uart_hal_poll_out_str_ext(const char *str) { uart_hal_poll_out_str(g_uart_hal, str); }

void uart_hal_enable_intr_rx_ext(void) { uart_hal_enable_intr_rx(g_uart_hal); }
