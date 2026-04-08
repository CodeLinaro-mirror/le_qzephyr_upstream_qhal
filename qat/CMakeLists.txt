# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

if(CONFIG_QAT)

zephyr_library()

# QAT Zephyr integration sources
zephyr_library_sources(
    src/qat_io.c
    src/qat_core.c
)

zephyr_library_include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc
)

endif()
