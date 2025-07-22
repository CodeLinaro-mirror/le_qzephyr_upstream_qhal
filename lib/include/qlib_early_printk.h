/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCC_EARLY_PRINTK_H_
#define QCC_EARLY_PRINTK_H_

#define EARLY_PRINTK_BUF_SIZE 120

void early_printk_init(void);
void early_printk(const char *fmt, ...);

#endif /* QCC_EARLY_PRINTK_H_ */
