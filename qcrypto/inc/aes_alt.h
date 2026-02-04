/**
 * \file aes_alt.h
 *
 * \brief   This file contains qualcomm hardware AES definitions and functions.
 */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MBEDTLS_AES_ALT_H
#define MBEDTLS_AES_ALT_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <stddef.h>
#include <stdint.h>


#include "crypto_port.h"
#include "qccaes.h"

#include <CeML.h>

typedef crypto_aes_hw_t mbedtls_aes_context;

#if defined(MBEDTLS_CIPHER_MODE_XTS)
typedef struct {
    mbedtls_aes_context crypt;
    mbedtls_aes_context tweak;
} mbedtls_aes_xts_context;
#endif

#endif

