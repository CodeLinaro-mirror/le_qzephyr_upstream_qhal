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

/* mbedtls_rsa_get_bitlen() was added in mbedtls 3.x and is not present in the
 * older libcryptoqcc730.a (MBEDTLS_RSA_ALT).  Provide a compatibility stub:
 * for any well-formed RSA key the highest bit of N is always set, so the
 * bit-length equals the byte-length multiplied by 8 exactly.
 */
#if defined(MBEDTLS_RSA_ALT) && defined(MBEDTLS_RSA_C)
#include <mbedtls/rsa.h>
size_t mbedtls_rsa_get_bitlen(const mbedtls_rsa_context *ctx)
{
    return mbedtls_rsa_get_len(ctx) * 8;
}
#endif
