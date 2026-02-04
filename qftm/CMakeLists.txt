# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

zephyr_library_named(qftm)
zephyr_include_directories(inc)

zephyr_library_sources(src/ftm_uart_polling.c)

zephyr_link_libraries(
    wifiqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)
