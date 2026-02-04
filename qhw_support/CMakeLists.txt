# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause


zephyr_library_named(qhw_support)

zephyr_link_libraries(
    sensorqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)

zephyr_include_directories(inc)
zephyr_include_directories(${ZEPHYR_BASE}/../modules/hal/qcom/qwifi/inc/libwifi)
zephyr_library_sources(src/wifi_fw_pmic_driver.c)
zephyr_library_sources(src/nt_wifi_driver.c)
zephyr_library_sources(src/nt_sys_monitoring.c)
zephyr_library_sources(src/wifi_fw_cpr_driver.c)
zephyr_library_sources(src/nt_mem.c)
zephyr_library_sources(src/hal_dxe.c)
zephyr_library_sources(src/nt_cc_battery_driver.c)
#for power
zephyr_library_sources(src/wlan_sleep_clk_cal.c)
zephyr_library_sources(src/nt_gpio.c)
zephyr_library_sources(src/wifi_fw_ext_intr.c)
zephyr_library_sources(src/wifi_fw_init.c)
zephyr_library_sources(src/hal_aon.c)

zephyr_linker_sources(SECTIONS src/hwmem.ld)

#to-do
#ferm_hkadc_drv.c
#nt_prng.c
#wifi_fw_pwr_cb_infra.c
#wifi_fw_pmu_ts_cfg.c