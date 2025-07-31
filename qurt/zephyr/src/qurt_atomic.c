
/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "zephyr/kernel.h"
#include "qurt_atomic.h"

unsigned int qurt_atomic_set(unsigned int *target, unsigned int value)
{
    atomic_set((atomic_t *)target, value);
    return value;
}

void qurt_atomic_and(unsigned int *target, unsigned int mask)
{
    atomic_and((atomic_t *)target, mask);
}

void qurt_atomic_or(unsigned int *target, unsigned int mask)
{
    atomic_or((atomic_t *)target, mask);
}

void qurt_atomic_xor(unsigned int *target, unsigned int mask)
{
    atomic_xor((atomic_t *)target, mask);
}

void qurt_atomic_set_bit(unsigned int *target, unsigned int bit)
{
    atomic_set_bit((atomic_t *)target, bit);
}

void qurt_atomic_clear_bit(unsigned int *target, unsigned int bit)
{
    atomic_clear_bit((atomic_t *)target, bit);
}

void qurt_atomic_add(unsigned int *target, unsigned int v)
{
    atomic_add((atomic_t *)target, v);
}

void qurt_atomic_sub(unsigned int *target, unsigned int v)
{
    atomic_sub((atomic_t *)target, v);
}

void qurt_atomic_inc(unsigned int *target)
{
    atomic_inc((atomic_t *)target);
}

unsigned int qurt_atomic_inc_return(unsigned int *target)
{
    return atomic_inc((atomic_t *)target) + 1;
}

void qurt_atomic_dec(unsigned int *target)
{
    atomic_dec((atomic_t *)target);
}

unsigned int qurt_atomic_dec_return(unsigned int *target)
{
    return atomic_dec((atomic_t *)target) - 1;
}

unsigned int qurt_atomic_compare_and_set(unsigned int *target, unsigned int old_val, unsigned int new_val)
{
    return atomic_cas((atomic_t *)target, old_val, new_val);
}
