/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "pka.h"
#include "nt_prng.h"
#include <zephyr/init.h>

static int pka_sys_init(void)
{
    pka_init(&g_pka_ctxt);

    return 0;
}

static int prng_sys_init(void)
{
    return (int)nt_prng_init();
}

SYS_INIT(pka_sys_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
SYS_INIT(prng_sys_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
