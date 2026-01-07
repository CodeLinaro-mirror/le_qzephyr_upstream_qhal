# Copyright (c) 2025 Qualcomm Innovation Center, Inc. All rights reserved.
# SPDX-License-Identifier: BSD-3-Clause-Clear

if(CONFIG_RING_SERVICE)
  zephyr_library_named(qring)
  
  zephyr_library_sources(src/ring_service_qcc730.c)

  zephyr_include_directories(inc)

  zephyr_linker_sources(SECTIONS src/ring.ld)
endif()
