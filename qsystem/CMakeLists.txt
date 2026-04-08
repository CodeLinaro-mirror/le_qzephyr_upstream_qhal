# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear

execute_process(
    COMMAND
        ${PYTHON_EXECUTABLE}
        "${ZEPHYR_HAL_QCOM_MODULE_DIR}/tools/qbdf/BDF_txt2bin.py"
        "${ZEPHYR_HAL_QCOM_MODULE_DIR}/tools/qbdf/bdf_file/"
        "${ZEPHYR_HAL_QCOM_MODULE_DIR}/zephyr/blobs/"
)

execute_process(
	COMMAND
		${PYTHON_EXECUTABLE}
		"${CMAKE_CURRENT_SOURCE_DIR}/../tools/devcfg/propgen.py"
		"--XmlFile=master_xml.xml" "--DirName=${CMAKE_CURRENT_SOURCE_DIR}/inc/"
		"--ConfigFile=nt_devcfg_from_master_xml.h"
		"--DevcfgDataFile=nt_devcfg_data.h"
		"--ConfigType=devcfg_xml"
)

execute_process(
	COMMAND
		${PYTHON_EXECUTABLE}
		"${CMAKE_CURRENT_SOURCE_DIR}/../tools/devcfg/dev_xml_cfg_debug.py"
		"${CMAKE_CURRENT_SOURCE_DIR}/inc/"
)

zephyr_library_named(qsystem)
zephyr_include_directories(common_inc)
zephyr_include_directories(inc)
zephyr_include_directories(common_inc/hwio/TAPEOUT_07)

zephyr_library_sources(src/nt_devcfg_parser.c)

zephyr_library_sources(src/unpa.c)
zephyr_library_sources(src/unpa_resource.c)
zephyr_library_sources(src/unpa_client.c)
zephyr_library_sources(src/wifi_fw_pwr_cb_infra.c)