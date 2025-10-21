# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause


if(NOT CONFIG_SOC_FAMILY_QCOM)
	return()
endif()

zephyr_compile_options(-DZEPHYR)

add_subdirectory_ifdef(CONFIG_QTEST tests)
add_subdirectory_ifdef(CONFIG_QWIFI qwifi)
add_subdirectory_ifdef(CONFIG_QURT_ABSTRACTION_LAYER qurt)
add_subdirectory_ifdef(CONFIG_QSYSTEM qsystem)
add_subdirectory_ifdef(CONFIG_QHW_SUPPORT qhw_support)
add_subdirectory_ifdef(CONFIG_QPOWER qpower)
add_subdirectory(hal)
add_subdirectory(lib)
add_subdirectory(qinclude)
add_subdirectory(qboot)
