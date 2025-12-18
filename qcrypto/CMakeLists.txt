# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

zephyr_library_named(qcrypto)
zephyr_include_directories(inc)

zephyr_link_libraries(
    cryptoqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)
