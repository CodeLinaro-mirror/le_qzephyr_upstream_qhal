/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "qapi_rram.h"
#include "ferm_qspi.h"
#include "qurt_mutex.h"
#include "nt_mem.h"
#include "stdio.h"

#define RRAM_SHELL_INFO 1
#define RRAM_SHELL_GROUP_PRINTF_SUFFIX  "RRAM: "

#if RRAM_SHELL_INFO
#define rram_info(msg,...)     printf(RRAM_SHELL_GROUP_PRINTF_SUFFIX msg, ##__VA_ARGS__)
#define rram_err(msg,...)      printf(RRAM_SHELL_GROUP_PRINTF_SUFFIX msg, ##__VA_ARGS__)
#else
#define rram_info(args...)      do { } while (0)
#define rram_err(args...)       do { } while (0)
#endif

static qurt_mutex_t *rram_udpart_mutex;
static uint32_t rram_max_address = _LN_REGDB_START_ADDR;
static bool rram_udpart_init_done;
static uint32_t part_map[RRAM_MAX_PART_NUMBER];

/**
   @brief Verify if the given RRAM  address range is valid for an RRAM write/read
          operation.
   @param[in] start_address  Start address for the RRAM read/write operation.
   @param[in] length         Number of bytes to be read/written.
   @param[in] min_addr       Minimum allowed address.
   @param[in] max_addr       Maximum allowed address.

   @return true if the address range is valid or false if it is not.
*/
static bool rram_valid_address(uint32_t start_address, uint32_t length, uint32_t min_addr, uint32_t max_addr)
{
    return (start_address >= min_addr && start_address < max_addr && start_address + length <= max_addr);
}

/**
   @brief Initialize the rram module.

   This function must be called before any other rram functions. It initializes
   the mutex for thread safety, maps partition IDs to their corresponding addresses
   from the FDT partition table, and marks the initialization as complete.
*/
void rram_udpart_init()
{
    if (rram_udpart_init_done) {
        return ;
    }

    qurt_mutex_create(&rram_udpart_mutex);

    int array_size = sizeof(fdt_part) / sizeof(IDAddr);
    for (int i = 0; i < array_size; i++) {
        int id = fdt_part[i].id;
        if (id < RRAM_MAX_PART_NUMBER && fdt_part[i].addr) {
            part_map[id] = fdt_part[i].addr;
        }
    }

    rram_udpart_init_done = true;
}


/**
   @brief Deinitialize the rram module.

   This function cleans up resources used by the rram module, such as deleting
   the mutex and resetting the initialization flag. It should be called when
   the rram functionality is no longer needed.
*/
void rram_udpart_deinit()
{
    if(!rram_udpart_init_done)
       return ;

    qurt_mutex_delete(rram_udpart_mutex);
    rram_udpart_mutex = NULL;
    rram_udpart_init_done = false;
}

/**
   @brief Read data from the rram.

   @param[in]  partid    The partid to map the ud part base addrss in sbl.
   @param[in]  offset    The rram address to start to read from.
   @param[in]  len       Number of bytes to read.
   @param[out] buffer    Data buffer for a rram read operation.

   @return
   QAPI_OK -- If a read completed successfully. \n
   Error code -- If there is an error.
*/
qapi_Status_t qapi_rram_read(uint32_t partid, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    qapi_Status_t status;

    if (buffer == NULL || len == 0) {
        return QAPI_ERROR;
    }

    if (!rram_udpart_init_done) {
        return QAPI_ERROR;
    }

    if (partid >= RRAM_MAX_PART_NUMBER || !part_map[partid]) {
        rram_err("partiton id (%u): valid range 0-%d\n", partid, RRAM_MAX_PART_NUMBER - 1);
        return QAPI_ERR_BOUNDS;
    }

    uint32_t base_addr = part_map[partid];
    uint32_t dst_addr = base_addr + offset;

    if (!rram_valid_address(dst_addr, len, base_addr, rram_max_address)) {
        rram_err("fail to verify address: 0x%x %u\n", dst_addr, len);
        return QAPI_ERR_BOUNDS;
    }

    qurt_mutex_lock(rram_udpart_mutex);
    status = nt_rram_read(dst_addr, buffer, len);
    qurt_mutex_unlock(rram_udpart_mutex);

    if (status != QAPI_OK) {
        rram_err("rram read error: 0x%x %u\n", dst_addr, len);
    }

    return status;
}

/**
   @brief Write data to the rram.

   @param[in] partid    The partid to map the ud part base addrss in sbl.
   @param[in] offset    The rram address to start to write to.
   @param[in] buffer    Data buffer containing data to be written.
   @param[in] len       Number of bytes to write.

   @return
   QAPI_OK -- If blocking write completed successfully. \n
   Error code -- If there is an error.
*/
qapi_Status_t qapi_rram_write(uint32_t partid, uint32_t offset, uint8_t *buffer, uint32_t len)
{
    qapi_Status_t status;

    if (buffer == NULL || len == 0) {
        return QAPI_ERROR;
    }

    if (!rram_udpart_init_done) {
        return QAPI_ERROR;
    }

    if (partid >= RRAM_MAX_PART_NUMBER || !part_map[partid]) {
        rram_err("partiton id (%u): valid range 0-%d\n", partid, RRAM_MAX_PART_NUMBER - 1);
        return QAPI_ERR_BOUNDS;
    }

    uint32_t base_addr = part_map[partid];
    uint32_t dst_addr = base_addr + offset;

    if (!rram_valid_address(dst_addr, len, base_addr, rram_max_address)) {
        rram_err("fail to verify address: 0x%x %u\n", dst_addr, len);
        return QAPI_ERR_BOUNDS;
    }

    qurt_mutex_lock(rram_udpart_mutex);
    status = nt_rram_write(dst_addr, buffer, len);
    qurt_mutex_unlock(rram_udpart_mutex);

    if (status != QAPI_OK) {
        rram_err("rram write error: 0x%x %u\n", dst_addr, len);
    }

    return status;
}
