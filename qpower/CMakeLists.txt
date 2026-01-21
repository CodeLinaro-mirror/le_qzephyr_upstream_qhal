# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

zephyr_library_named(qpower)
zephyr_include_directories(inc)
zephyr_include_directories(${ZEPHYR_HAL_QCOM_MODULE_DIR}/qwifi/inc/libwifi)
zephyr_include_directories(${ZEPHYR_HAL_QCOM_MODULE_DIR}/qwifi/inc/wlan_drv)
zephyr_include_directories(${ZEPHYR_HAL_QCOM_MODULE_DIR}/qinclude/qapi)
zephyr_library_sources(src/qpower.c)
zephyr_library_sources(src/qapi_lowpower.c)

zephyr_link_libraries(
    powerqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)
