# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
execute_process(
    COMMAND
        ${PYTHON_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/frn_fwd_table.py"
        --fdtxml "${CMAKE_CURRENT_SOURCE_DIR}/curr_age_fdt/frn_curr_age_with_app_bin.xml"
        --fdtbin "${ZEPHYR_HAL_QCOM_MODULE_DIR}/zephyr/blobs/frn_curr_age_with_app_bin.bin"
        --ctype 0

    COMMAND
        ${PYTHON_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/frn_fwd_table.py"
        --fdtxml "${CMAKE_CURRENT_SOURCE_DIR}/curr_age_fdt/frn_curr_age_default.xml"
        --fdtbin "${ZEPHYR_HAL_QCOM_MODULE_DIR}/zephyr/blobs/frn_curr_age_default.bin"
        --ctype 0

    COMMAND
        ${PYTHON_EXECUTABLE}
        "${CMAKE_CURRENT_SOURCE_DIR}/frn_fwd_table.py"
        --fdtxml "${CMAKE_CURRENT_SOURCE_DIR}/curr_gold_age_fdt/frn_curr_gold_age_default.xml"
        --fdtbin "${ZEPHYR_HAL_QCOM_MODULE_DIR}/zephyr/blobs/frn_curr_gold_age_default.bin"
        --ctype 0
)
