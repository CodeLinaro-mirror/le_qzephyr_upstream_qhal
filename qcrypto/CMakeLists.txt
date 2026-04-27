# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

zephyr_library_named(qcrypto)
zephyr_include_directories(inc)

if(CONFIG_QHW_SUPPORT)
  zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../qhw_support/inc)
endif()

zephyr_link_libraries(
    cryptoqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)

# libcryptoqcc730.a provides ALT implementations (ecdsa_alt, rsa_alt, …) that
# overlap with generic symbols in libmbedTLSBase.a.  The linker must accept
# multiple definitions and keep the first occurrence (libcryptoqcc730.a wins).
zephyr_ld_options(-Wl,--allow-multiple-definition)

zephyr_library_sources(
    src/qcrypto.c
    src/aead_get_custom.c
)

zephyr_compile_definitions(
    MBEDTLS_RSA_C
    MBEDTLS_PKCS1_V15
    MBEDTLS_GENPRIME
    MBEDTLS_RSA_ALT
    MBEDTLS_ECP_ALT
    MBEDTLS_ECDSA_ALT
    MBEDTLS_DHM_ALT
    # MBEDTLS_AES_ALT    # Build 38: disabled — hardware AES leaves semaphore dirty after 4WHS, TLS derive_keys deadlocks at state=9
    # MBEDTLS_CCM_ALT    # Build 38: disabled — CCM uses hardware AES internally, same deadlock risk
    MBEDTLS_SHA_ALT
    MBEDTLS_SHA1_ALT
    # MBEDTLS_SHA256_ALT # Build 33: disabled to use software SHA256 (hardware hangs at TLS state=9)
    MBEDTLS_CCM_C
    MBEDTLS_ENTROPY_HARDWARE_ALT
)