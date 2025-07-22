/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef UART_HAL_H
#define UART_HAL_H

#include <uart_ll.h>

void uart_hal_init(uart_hal *hal, uint32_t uart_num);
void uart_hal_parity_set(uart_hal *hal, uart_parity parity);
int uart_hal_baudrate_set(uart_hal *hal, uint32_t baudrate);

void uart_hal_poll_out(uart_hal *hal, uint8_t ch);
void uart_hal_poll_out_ext(uint8_t ch);

void uart_hal_poll_out_str(uart_hal *hal, const char *str);
void uart_hal_poll_out_str_ext(const char *str);
void uart_hal_enable_intr_rx_ext(void);

#endif /* UART_HAL_H */
