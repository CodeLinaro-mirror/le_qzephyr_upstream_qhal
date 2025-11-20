# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT CONFIG_SOC_QCC730)
  return()
endif()

set(executable_name ${BOARD}_prg)
add_executable(${executable_name})

# ===== 1) Compile-time definitions =====
target_compile_definitions(${executable_name} PRIVATE
  CONFIG_NON_OS
)

# ===== 2) Linker script and map file =====
set(linker_script_path "${ZEPHYR_HAL_QCOM_MODULE_DIR}/qprgd/frn_nvm_programmer.ld")
set(map_file_path          "${CMAKE_CURRENT_BINARY_DIR}/${executable_name}.map")

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
  -mfp16-format=ieee
  -Os
  -g
)

# ===== 5) Link options =====
target_link_options(${executable_name} PRIVATE
  ${CPU_ABI_FLAGS}
  -T ${linker_script_path}
  -nostartfiles
  -nodefaultlibs
  -Wl,-Map=${map_file_path}
  -Wl,--gc-sections
  -Wl,--nmagic
)

# Force relink if linker script changes
set_property(TARGET ${executable_name} PROPERTY
  LINK_DEPENDS ${linker_script_path}
)

# ===== 6) Libraries and objects =====
target_link_libraries(${executable_name} PRIVATE
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
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/lib/include
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qsystem/common_inc/hwio/TAPEOUT_07
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/qinclude/qapi/common
  ${ZEPHYR_BINARY_DIR}/include/generated
)

# ===== 8) Source files =====
target_sources(${executable_name} PRIVATE
  src/nvm_programmer.c
  src/nvm_rram.c
  src/nvm_flash.c
  src/uart_print.c
  ${ZEPHYR_HAL_QCOM_MODULE_DIR}/lib/util/safeAPI.c
)

# ===== Post-build: run Python script for hex/strip/bin/hashed =====
find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(POST_PY "${ZEPHYR_HAL_QCOM_MODULE_DIR}/tools/post_process.py")
set(HASH_SCRIPT "${ZEPHYR_HAL_QCOM_MODULE_DIR}/tools/qhash/sechash/createxbl.py")
set(SBL_TOOL_PREFIX "${ZEPHYR_SDK_INSTALL_DIR}/arm-zephyr-eabi/bin/arm-zephyr-eabi")

set(BLOBS_DIR "${ZEPHYR_HAL_QCOM_MODULE_DIR}/zephyr/blobs")
set(ELF_BASENAME "${executable_name}.elf")

add_custom_command(TARGET ${executable_name} POST_BUILD
  COMMAND "${Python3_EXECUTABLE}" "${POST_PY}"
          --elf "$<TARGET_FILE:${executable_name}>"
          --tool-prefix "${SBL_TOOL_PREFIX}"
          --hash-script "${HASH_SCRIPT}"

  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          "$<TARGET_FILE_DIR:${executable_name}>/${ELF_BASENAME}"
          "${BLOBS_DIR}/${ELF_BASENAME}"

  COMMENT "Post-process ELF and export ELF to ${BLOBS_DIR}"
  VERBATIM)
