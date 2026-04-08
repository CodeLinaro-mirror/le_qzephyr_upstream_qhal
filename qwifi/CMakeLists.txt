# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

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
zephyr_library_sources(platform/qapi_system.c)

zephyr_link_libraries(
    wifiqcc730
    -L${CMAKE_CURRENT_SOURCE_DIR}/../zephyr/blobs/
)


file(GLOB RAMFUNC_FILES
    # Network stack - sockets layer
    ${ZEPHYR_BASE}/subsys/net/lib/sockets/sockets_inet.c
    ${ZEPHYR_BASE}/subsys/net/lib/sockets/sockets.c
    ${ZEPHYR_BASE}/subsys/net/lib/sockets/sockets_misc.c
    
    # Network stack - L2 layer
    ${ZEPHYR_BASE}/subsys/net/l2/ethernet/ethernet.c
    ${ZEPHYR_BASE}/subsys/net/l2/ethernet/arp.c
    
    # Network stack - IP layer
    ${ZEPHYR_BASE}/subsys/net/ip/net_context.c
    ${ZEPHYR_BASE}/subsys/net/ip/net_core.c
    ${ZEPHYR_BASE}/subsys/net/ip/ipv4.c
    ${ZEPHYR_BASE}/subsys/net/ip/ipv6.c
    ${ZEPHYR_BASE}/subsys/net/ip/tcp.c
    ${ZEPHYR_BASE}/subsys/net/ip/udp.c
    ${ZEPHYR_BASE}/subsys/net/ip/net_if.c
    ${ZEPHYR_BASE}/subsys/net/ip/net_pkt.c
    ${ZEPHYR_BASE}/subsys/net/ip/utils.c
    ${ZEPHYR_BASE}/subsys/net/ip/connection.c
    
    # Network buffers
    ${ZEPHYR_BASE}/lib/net_buf/buf.c
    ${ZEPHYR_BASE}/lib/net_buf/buf_simple.c
    
    # Kernel - memory management
    ${ZEPHYR_BASE}/kernel/mempool.c
    ${ZEPHYR_BASE}/kernel/mem_slab.c
    ${ZEPHYR_BASE}/kernel/kheap.c
    
    # Kernel - synchronization
    ${ZEPHYR_BASE}/kernel/mutex.c
    ${ZEPHYR_BASE}/kernel/sem.c
    ${ZEPHYR_BASE}/kernel/queue.c
    
    # Kernel - work queue
    ${ZEPHYR_BASE}/kernel/work.c
    ${ZEPHYR_BASE}/kernel/system_work_q.c
    
    # Kernel - timing
    ${ZEPHYR_BASE}/kernel/timeout.c
    ${ZEPHYR_BASE}/kernel/timer.c
    
    # WiFi driver
    ${ZEPHYR_BASE}/../qcc730/drivers/wifi/qcom/qcom_wifi_drv.c
    ${ZEPHYR_BASE}/../modules/hal/qcom/qwifi/zephyr/qwifi.c
    ${ZEPHYR_BASE}/../modules/hal/qcom/qwifi/zephyr/libwifi.c
    )

if (CONFIG_DP_RAM_LOCATED)
	zephyr_code_relocate(FILES ${RAMFUNC_FILES} LOCATION RAM)
endif()
