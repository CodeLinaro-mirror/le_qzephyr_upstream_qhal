# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT CONFIG_SOC_QCC730)
  return()
endif()

set(executable_name ${BOARD}_sbl)
add_executable(${executable_name})

# ===== 1) Compile-time definitions =====
target_compile_definitions(${executable_name} PRIVATE
  MBEDTLS_ARC4_C
  MBEDTLS_PLATFORM_MS_TIME_ALT # Enable custom mbedtls_ms_time() implementation
  ZEPHYR
  CONFIG_SOC_SERIES_QCC730=1
  SBL_BUILD
  P_DEBUG
  CONFIG_NON_OS
  CONFIG_BOARD_QCC730_QSPI_ENABLE
  __LINUX_ERRNO_EXTENSIONS__
)

# ===== 2) Linker script and map file =====
set(sbl_linker_script_path "${ZEPHYR_HAL_QCOM_MODULE_DIR}/qboot/frn_ioe_sbl.ld")
set(map_file_path          "${CMAKE_CURRENT_BINARY_DIR}/${executable_name}.map")
set(pbl_patch_obj_path     "${ZEPHYR_HAL_QCOM_MODULE_DIR}/qboot/src/pbl_patch.o")

# ===== 3) CPU/ABI flags (must match for compile and link) =====
set(CPU_ABI_FLAGS
  -mcpu=cortex-m4
  -mthumb
  -mfpu=fpv4-sp-d16
  -mfloat-abi=softfp
)

# ===== 4) Compile options =====
target_compile_options(${executable_name} PRIVATE
  ${CPU_ABI_FLAGS}
  -fdiagnostics-color=always
  -fno-strict-aliasing
  -ffunction-sections
  -fdata-sections
  -mfp16-format=ieee
  -Os
  -g
)

# ===== 5) Link options =====
target_link_options(${executable_name} PRIVATE
  ${CPU_ABI_FLAGS}
  -L ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qboot
  -T ${sbl_linker_script_path}
  -nostartfiles
  -nodefaultlibs
  -Wl,-Map=${map_file_path}
  -Wl,--gc-sections
  -Wl,--nmagic
)

# Force relink if linker script changes
set_property(TARGET ${executable_name} PROPERTY
  LINK_DEPENDS ${sbl_linker_script_path}
)

# ===== 6) Libraries and objects =====
target_link_libraries(${executable_name} PRIVATE
  ${pbl_patch_obj_path}
  -Wl,--start-group
  c_nano
  m
  gcc
  nosys
  -Wl,--end-group
)

# ===== 7) Include paths =====
target_include_directories(${executable_name} PRIVATE
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qsystem/common_inc
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qwifi/inc/libwifi
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qurt/zephyr/inc
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/lib/include
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qsystem/common_inc/hwio/TAPEOUT_07
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/hal/include
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/hal/qcc730/include
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/soc
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qwifi/zephyr
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qhw_support/inc
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/soc/qcc730
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qsystem/inc
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qpower/inc
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qprgd/src
  ${ZEPHYR_BASE}/../qcc730/soc/qcom/qcc730/
  ${ZEPHYR_BASE}/include
  ${ZEPHYR_MBEDTLS_MODULE_DIR}/include
  ${ZEPHYR_MBEDTLS_MODULE_DIR}/include/mbedtls
  ${ZEPHYR_MBEDTLS_MODULE_DIR}/include/psa
  ${ZEPHYR_CMSIS_MODULE_DIR}/CMSIS/Core/Include
  ${ZEPHYR_BINARY_DIR}/include/generated
  elf_loader
  include
  src
  common
  sbl_auth/inc
  sbl_auth/rollback
)

# ===== 8) Source files =====
target_sources(${executable_name} PRIVATE
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/lib/util/safeAPI.c
  ${ZEPHYR_MBEDTLS_MODULE_DIR}/library/platform_util.c
  ${ZEPHYR_MBEDTLS_MODULE_DIR}/library/sha256.c
  elf_loader/boot_elf_loader.c
  elf_loader/boot_elf_mem.c
  system/nt_bl_rram_dxe.c
  debug/boot_print.c
  debug/boot_log.c
  system/nt_bl_uart.c
  system/nt_bl_mem.c
  system/sbl_storage.c
  system/sbl_flash_fwd.c
  system/rram_fdt.c
  src/sbl_main.c
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/hal/src/ferm_qspi.c
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/hal/src/ferm_flash.c
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/hal/hal/src/ferm_flash_config.c
  sbl_auth/sbl_auth.c
  sbl_auth/rollback/boot_rollback_version.c
  sbl_auth/rollback/boot_rollback_version_impl.c
)

# ===== Post-build: run Python script for hex/strip/bin/hashed =====
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(POST_PY "${ZEPHYR_HAL_QCOM_MODULE_DIR}/tools/post_process.py")
set(HASH_SCRIPT "${ZEPHYR_HAL_QCOM_MODULE_DIR}/tools/qhash/sechash/createxbl.py")
set(SBL_TOOL_PREFIX "${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/bin/arm-zephyr-eabi")

set(BLOBS_DIR "${ZEPHYR_HAL_QCOM_MODULE_DIR}/zephyr/blobs")
set(HASHED_ELF_BASENAME "${executable_name}_HASHED.elf")

add_custom_command(TARGET ${executable_name} POST_BUILD
  COMMAND "${Python3_EXECUTABLE}" "${POST_PY}"
          --elf "$<TARGET_FILE:${executable_name}>"
          --tool-prefix "${SBL_TOOL_PREFIX}"
          --hash-script "${HASH_SCRIPT}"

  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE_DIR:${executable_name}>/${HASHED_ELF_BASENAME}"
          "${BLOBS_DIR}/${HASHED_ELF_BASENAME}"

  COMMENT "Post-process ELF and export hashed ELF to ${BLOBS_DIR}"
  VERBATIM)
