/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * NOT A CONTRIBUTION
 */


#if defined(MBEDTLS_DHM_ALT)

/**
 * \brief          The DHM context structure.
 */
typedef struct mbedtls_dhm_context {
    mbedtls_mpi MBEDTLS_PRIVATE(P);      /*!<  The prime modulus. */
    mbedtls_mpi MBEDTLS_PRIVATE(G);      /*!<  The generator. */
    mbedtls_mpi MBEDTLS_PRIVATE(X);      /*!<  Our secret value. */
    mbedtls_mpi MBEDTLS_PRIVATE(GX);     /*!<  Our public key = \c G^X mod \c P. */
    mbedtls_mpi MBEDTLS_PRIVATE(GY);     /*!<  The public key of the peer = \c G^Y mod \c P. */
    mbedtls_mpi MBEDTLS_PRIVATE(K);      /*!<  The shared secret = \c G^(XY) mod \c P. */
    mbedtls_mpi MBEDTLS_PRIVATE(RP);     /*!<  The cached value = \c R^2 mod \c P. */
    mbedtls_mpi MBEDTLS_PRIVATE(Vi);     /*!<  The blinding value. */
    mbedtls_mpi MBEDTLS_PRIVATE(Vf);     /*!<  The unblinding value. */
    mbedtls_mpi MBEDTLS_PRIVATE(pX);     /*!<  The previous \c X. */
}
mbedtls_dhm_context;

#endif /* MBEDTLS_DHM_ALT */