# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause-Clear


zephyr_library_named(qutil)

zephyr_include_directories(include)

zephyr_library_sources_ifdef(CONFIG_QLIB_EARLY_PRINTK	util/qlib_early_printk.c)
zephyr_library_sources(util/qlib_util.c)
zephyr_library_sources(util/safeAPI.c)
