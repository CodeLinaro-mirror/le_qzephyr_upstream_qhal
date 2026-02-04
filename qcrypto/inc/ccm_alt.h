/**
 * \file ccm_alt.h
 *
 * \brief   This file contains qualcomm hardware AES-CCM definitions and functions.
 */
/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MBEDTLS_CCM_ALT_H
#define MBEDTLS_CCM_ALT_H

#if !defined(MBEDTLS_CONFIG_FILE)
#include "mbedtls/mbedtls_config.h"
#else
#include MBEDTLS_CONFIG_FILE
#endif

#include <stddef.h>
#include <stdint.h>


#include <crypto_port.h>
#include "qccaes.h"

#include <CeML.h>

typedef crypto_aes_ccm_qcc_t mbedtls_ccm_context;

#endif

