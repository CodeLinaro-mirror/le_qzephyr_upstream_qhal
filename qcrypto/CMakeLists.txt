# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

zephyr_library_named(qcrypto)
zephyr_include_directories(inc)

if(CONFIG_QHW_SUPPORT)
  zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../qhw_support/inc)
endif()

zephyr_link_libraries(
    cryptoqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)

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
    MBEDTLS_DHM_ALT
    MBEDTLS_AES_ALT
    MBEDTLS_CCM_ALT
    MBEDTLS_SHA_ALT
    MBEDTLS_SHA1_ALT
    MBEDTLS_SHA256_ALT
    MBEDTLS_CCM_C
    MBEDTLS_ENTROPY_HARDWARE_ALT
)