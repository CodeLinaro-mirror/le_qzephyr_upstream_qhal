/**
 * \file ecp_alt.h
 *
 * \brief This file provides a Qualcomm-specific alternative implementation for Elliptic Curves over GF(P) (ECP) via the MBEDTLS_BIGNUM_ALT interface.
 *
 * The use of ECP in cryptography and TLS is defined in
 * <em>Standards for Efficient Cryptography Group (SECG): SEC1
 * Elliptic Curve Cryptography</em> and
 * <em>RFC-4492: Elliptic Curve Cryptography (ECC) Cipher Suites
 * for Transport Layer Security (TLS)</em>.
 *
 * <em>RFC-2409: The Internet Key Exchange (IKE)</em> defines ECP
 * group types.
 *
 */

/*
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0 OR GPL-2.0-or-later
 *
 *  Qualcomm Technologies, Inc. chooses to take subject only to the terms of the Apache-2.0 license.
 */

/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * NOT A CONTRIBUTION
 */

#ifndef MBEDTLS_ECP_ALT_H
#define MBEDTLS_ECP_ALT_H

#include "mbedtls/private_access.h"

#include "mbedtls/build_info.h"
#include "mbedtls/platform_util.h"

#include "mbedtls/bignum.h"

#if defined(MBEDTLS_ECP_ALT)

typedef struct mbedtls_ecp_group {
    mbedtls_ecp_group_id id;    /*!< An internal group identifier. */
    mbedtls_mpi P;              /*!< The prime modulus of the base field. */
    mbedtls_mpi A;              /*!< For Short Weierstrass: \p A in the equation. Note that
                                     \p A is not set to the authentic value in some cases.
                                     Refer to detailed description of ::mbedtls_ecp_group if
                                     using domain parameters in the structure.
                                     For Montgomery curves: <code>(A + 2) / 4</code>. */
    mbedtls_mpi B;              /*!< For Short Weierstrass: \p B in the equation.
                                     For Montgomery curves: unused. */
    mbedtls_ecp_point G;        /*!< The generator of the subgroup used. */
    mbedtls_mpi N;              /*!< The order of \p G. */
    size_t pbits;               /*!< The number of bits in \p P.*/
    size_t nbits;               /*!< For Short Weierstrass: The number of bits in \p P.
                                     For Montgomery curves: the number of bits in the
                                     private keys. */
    /* End of public fields */

    unsigned int MBEDTLS_PRIVATE(h);             /*!< \internal 1 if the constants are static. */
    int(*MBEDTLS_PRIVATE(modp))(mbedtls_mpi *);  /*!< The function for fast pseudo-reduction
                                                    mod \p P (see above).*/
    int(*MBEDTLS_PRIVATE(t_pre))(mbedtls_ecp_point *, void *);   /*!< Unused. */
    int(*MBEDTLS_PRIVATE(t_post))(mbedtls_ecp_point *, void *);  /*!< Unused. */
    void *MBEDTLS_PRIVATE(t_data);               /*!< Unused. */
    mbedtls_ecp_point *MBEDTLS_PRIVATE(T);       /*!< Pre-computed points for ecp_mul_comb(). */
    size_t MBEDTLS_PRIVATE(T_size);              /*!< The number of dynamic allocated pre-computed points. */
}
mbedtls_ecp_group;

/**
 * \name SECTION: Module settings
 *
 * The configuration options you can set for this module are in this section.
 * Either change them in mbedtls_config.h, or define them using the compiler command line.
 * \{
 */

#if !defined(MBEDTLS_ECP_WINDOW_SIZE)
/*
 * Maximum "window" size used for point multiplication.
 * Default: a point where higher memory usage yields diminishing performance
 *          returns.
 * Minimum value: 2. Maximum value: 7.
 *
 * Result is an array of at most ( 1 << ( MBEDTLS_ECP_WINDOW_SIZE - 1 ) )
 * points used for point multiplication. This value is directly tied to EC
 * peak memory usage, so decreasing it by one should roughly cut memory usage
 * by two (if large curves are in use).
 *
 * Reduction in size may reduce speed, but larger curves are impacted first.
 * Sample performances (in ECDHE handshakes/s, with FIXED_POINT_OPTIM = 1):
 *      w-size:     6       5       4       3       2
 *      521       145     141     135     120      97
 *      384       214     209     198     177     146
 *      256       320     320     303     262     226
 *      224       475     475     453     398     342
 *      192       640     640     633     587     476
 */
#define MBEDTLS_ECP_WINDOW_SIZE    4   /**< The maximum window size used. */
#endif /* MBEDTLS_ECP_WINDOW_SIZE */

#if !defined(MBEDTLS_ECP_FIXED_POINT_OPTIM)
/*
 * Trade code size for speed on fixed-point multiplication.
 *
 * This speeds up repeated multiplication of the generator (that is, the
 * multiplication in ECDSA signatures, and half of the multiplications in
 * ECDSA verification and ECDHE) by a factor roughly 3 to 4.
 *
 * For each n-bit Short Weierstrass curve that is enabled, this adds 4n bytes
 * of code size if n < 384 and 8n otherwise.
 *
 * Change this value to 0 to reduce code size.
 */
#define MBEDTLS_ECP_FIXED_POINT_OPTIM  1   /**< Enable fixed-point speed-up. */
#endif /* MBEDTLS_ECP_FIXED_POINT_OPTIM */

/** \} name SECTION: Module settings */
#endif /* MBEDTLS_ECP_ALT */

#ifdef __cplusplus
#endif

#endif /* ecp_alt.h */
