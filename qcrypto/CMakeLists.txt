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

# NOTE: RSA_C/PKCS1_V15/GENPRIME/CCM_C/ENTROPY_HARDWARE_ALT are also #define'd
# (with an empty body) by the mbedTLS config headers.  Force them here with an
# empty value ("X=" rather than "X") so the command-line token matches the
# header token and the compiler no longer emits "redefined" warnings.  These
# must stay defined globally because the prebuilt *_alt sources rely on them.
zephyr_compile_definitions(
    MBEDTLS_RSA_C=
    MBEDTLS_PKCS1_V15=
    MBEDTLS_GENPRIME=
    MBEDTLS_RSA_ALT
    MBEDTLS_ECP_ALT
    MBEDTLS_ECDSA_ALT
    MBEDTLS_DHM_ALT
    # Hardware AES/CCM/SHA256 ALT re-enabled: blob rebuilt with clearReg() flush
    # and 500k-iteration CE polling timeout (fixes Build 33/38 deadlock root cause).
    MBEDTLS_AES_ALT
    MBEDTLS_CCM_ALT
    MBEDTLS_SHA_ALT
    MBEDTLS_SHA1_ALT
    MBEDTLS_SHA256_ALT
    MBEDTLS_CCM_C=
    MBEDTLS_ENTROPY_HARDWARE_ALT=
)