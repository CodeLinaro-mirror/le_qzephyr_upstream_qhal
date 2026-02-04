/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pka.h"
#include "nt_prng.h"
#include <zephyr/init.h>

static int pka_sys_init(void)
{
    pka_init(&g_pka_ctxt);

    return 0;
}

SYS_INIT(pka_sys_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
SYS_INIT(nt_prng_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);