# Copyright (c) 2025 River <riverh@qti.qualcomm.com>
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(qpower)
zephyr_include_directories(inc)
zephyr_library_sources(src/qpower.c)

zephyr_link_libraries(
    powerqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)
