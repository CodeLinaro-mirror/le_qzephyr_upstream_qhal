/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef __QAPI_RRAM_H__
#define __QAPI_RRAM_H__

#include "qapi_types.h"
#include "qapi_status.h"
#include "ferm_qspi.h"

#define RRAM_MAX_PART_NUMBER     10    /**< part map size */

typedef struct {
    uint32_t id;
    uint32_t addr;
} IDAddr;

typedef enum {
    RRAM_OFFSET_ERROR = -2,
    RRAM_ADDRESS_ERROR = -3,
    RRAM_OK = 1,
} rram_status_t;

extern IDAddr fdt_part[RRAM_MAX_PART_NUMBER];

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
qapi_Status_t qapi_rram_read(uint32_t partid, uint32_t offset, uint8_t *buffer, uint32_t len);

/**
   @brief Write data to the rram.

   @param[in] partid    The partid to map the ud part base addrss in sbl.
   @param[in] offset    The rram address to start to write to.
   @param[in] len       Number of bytes to write.
   @param[in] buffer    Data buffer containing data to be written.

   @return
   QAPI_OK -- If blocking write completed successfully. \n
   Error code -- If there is an error.
*/
qapi_Status_t qapi_rram_write(uint32_t partid, uint32_t offset, uint8_t *buffer, uint32_t len);

#endif /* __QAPI_RRAM_H__ */
