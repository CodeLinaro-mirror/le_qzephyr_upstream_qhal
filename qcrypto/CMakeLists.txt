# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

zephyr_library_named(qcrypto)
zephyr_include_directories(inc)

zephyr_link_libraries(
    cryptoqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)

zephyr_library_sources(src/qcrypto.c)

zephyr_compile_definitions(
    MBEDTLS_RSA_C
    MBEDTLS_PKCS1_V15
    MBEDTLS_GENPRIME
    MBEDTLS_RSA_ALT
    MBEDTLS_ECP_ALT
    MBEDTLS_DHM_ALT
)