# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

zephyr_include_directories(inc)
zephyr_include_directories(inc/libwifi)
zephyr_include_directories(inc/wlan_drv)
zephyr_include_directories(zephyr)

#zephyr_library()
zephyr_library_named(qwifi)

zephyr_library_sources(zephyr/qwifi.c)
zephyr_library_sources(src/wlan_drv/qapi_wlan_base.c)
zephyr_library_sources(src/wlan_drv/qapi_wlan_param_group.c)
zephyr_library_sources(src/wlan_drv/wlan_qapi_helper.c)
zephyr_library_sources(src/wlan_drv/wmi_api.c)
zephyr_library_sources(src/wlan_drv/wlan_drv.c)
zephyr_library_sources(zephyr/libwifi.c)

zephyr_link_libraries_ifdef(
    CONFIG_QWIFI_BUILD_WITH_BLOBS
    wifiqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)
