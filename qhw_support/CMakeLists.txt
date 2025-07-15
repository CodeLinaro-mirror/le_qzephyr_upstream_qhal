# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause


zephyr_library_named(qhw_support)
zephyr_include_directories(inc)

set(QHW_SUPPORT_STUB ON)

if(CONFIG_LIBWIFIQCC730_SRC)
	zephyr_library_sources(src/wifi_fw_pmic_driver.c)
	#add nt_socpm_sleep.c, also need to enable QPOWER
	zephyr_library_sources(src/nt_socpm_sleep.c)
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
endif()

if(QHW_SUPPORT_STUB)
	message("QHW_SUPPORT enable in debug mode")
	return()
endif()


zephyr_library_sources(src/nt_socpm_rtos_api.c)



