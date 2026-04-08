# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

# Build when CONFIG_WATCHDOG is enabled
if(CONFIG_WATCHDOG)
  zephyr_library()
  zephyr_library_sources(src/qwdt_init.c)
endif()
if(CONFIG_QWDT)
    zephyr_library_compile_definitions(CONFIG_QWDT=1)
endif()