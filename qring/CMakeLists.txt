# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

if(CONFIG_RING_SERVICE)
  zephyr_library_named(qring)
  
  zephyr_library_sources(src/ring_service_qcc730.c)

  zephyr_include_directories(inc)

  zephyr_linker_sources(SECTIONS src/ring.ld)
endif()
