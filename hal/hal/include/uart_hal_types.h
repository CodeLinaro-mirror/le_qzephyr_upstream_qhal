/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef UART_HAL_TYPES_H
#define UART_HAL_TYPES_H

#include <qcom_soc.h>

typedef enum {
    UART_STOP_BITS_1,
    UART_STOP_BITS_1_5_OR_2,
} uart_stop_bits;

typedef enum {
    UART_DATA_BITS_5,
    UART_DATA_BITS_6,
    UART_DATA_BITS_7,
    UART_DATA_BITS_8,
} uart_data_bits;

typedef enum {
    UART_PARITY_NONE,
    UART_PARITY_ODD,
    UART_PARITY_EVEN,
} uart_parity;

#endif /* UART_HAL_TYPES_H */
