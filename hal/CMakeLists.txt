# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

#zephyr_library()
zephyr_library_named(qhal)

zephyr_include_directories(soc)
zephyr_include_directories(soc/${SOC_SERIES})
zephyr_include_directories(hal/include)
zephyr_include_directories(hal/${SOC_SERIES}/include)

zephyr_library_sources_ifdef(CONFIG_UART_HAL	hal/uart_hal.c)
zephyr_library_sources_ifdef(CONFIG_UART_QCC730_LL	hal/${SOC_SERIES}/uart_ll.c)
zephyr_library_sources(hal/${SOC_SERIES}/pmu_ll.c)

if(CONFIG_FLASH_QCC730)
  zephyr_library_sources(
    hal/src/ferm_qspi.c
    hal/src/ferm_flash_config.c
  )
endif()
