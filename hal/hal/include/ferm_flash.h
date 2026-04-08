/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file ferm_flash.h
   @brief Flash Services Interface definition.
   This module provides flash operation APIs.
*/

/** @addtogroup peripherals_flash

  The QSPI Flash module provides flash operation APIs.

  drv_flash_init() initializes the flash module and must be called
  before any other function in this module. The input parameter - Config -
  must be set based on the specific flash used.

  Flash read/write/erase/writereg operations cannot be performed simultaneously.
  There can be only one operation ongoing at a time. A mutex is used to protect it.

  Flash read/write/erase/writereg support blocking operations on Fermion.

  For a blocking operation, the related function returns until the
  required number of data/status are read/written/erased successfully, or
  there is an error. If the required number is large, the related function
  may take some time to return. It should be taken into consideration if
  the product is power and time sensitive.
*/

#ifndef __FERM_FLASH_H__
#define __FERM_FLASH_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "ferm_flash_config.h"

#define PAGE_SIZE_IN_BYTES  256
#define BLOCK_SIZE_IN_BYTES 4096
#define FLASH_16MB_IN_BYTES 0x1000000

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/**
   @brief Prototype for a function called after a non-blocking flash
	  read/write/erase/writereg operation is complete.

   This function is called from a flash operation task.

   @param[in] Status     Flash operation result.
   @param[in] UserParam  User-specified parameter provided when the callback
			 is registered.
*/
typedef void (*flash_operation_cb_t)(int status, void *user_param);

/*-------------------------------------------------------------------------
 * Preprocessor Definitions and Constants
 *-----------------------------------------------------------------------*/

#define QSPI_TRANS_MODE QSPI_PIO_MODE_E

#define WRITE_STATUS_POLLING_USEC      100
#define WRITE_TIMEOUT                  5000000 /**< Time unit is usec. */
#define ERASE_STATUS_POLLING_MSEC      5
#define ERASE_TIMEOUT                  5000000 /**< Time unit is usec. */
#define READ_STATUS_POLLING_USEC       20
#define READ_STATUS_TIMEOUT            5000000 /**< Time unit is usec. */
#define CHIP_ERASE_STATUS_POLLING_MSEC 1000
#define CHIP_ERASE_TIMEOUT             500000000 /**< Time unit is usec. */

#define WRITE_ENABLE_CMD        0x06
#define READ_STATUS_CMD         0x05
#define READ_STATUS_2_CMD       0x3F
#define READ_CFG_REG_CMD        0x15
#define READ_CFG1_CMD           0x35
#define WRITE_STATUS_CMD        0x01
#define ENTER_4B_ADDR_CMD       0xB7
#define WRITE_STATUS_2_CMD      0x3E
#define READ_IDENTIFICATION_CMD 0x9F

#define STATUS_WR_EN_MASK          0x02
#define PROG_ERASE_WRITE_BUSY_BMSK 0x01
#define READ_STATUS_BUSY_MASK      0x01

#define FLASH_PID2VID(__pid__) ((uint8_t)(__pid__))

/* Flash Manufacturer ID, from the lowest byte of device id */
#define MANUFACTURER_ID_MACRONIX 0xC2 /**< Macronix. */
#define MANUFACTURER_ID_WINBOND  0xEF /**< Winbond. */
#define MANUFACTURER_ID_ISSI     0x9D /**< ISSI. */
#define MANUFACTURER_ID_GD       0xC8 /**< GD. */
#define MANUFACTURER_ID_GT       0xC4 /**< GT. */

/* Winbond SPI Command */
#define WINBOND_READ_STATUS_2_CMD  0x35
#define WINBOND_READ_STATUS_3_CMD  0x15
#define WINBOND_WRITE_STATUS_2_CMD 0x31
#define WINBOND_WRITE_STATUS_3_CMD 0x11

#if CONFIG_FLASH_QCC730_QSPI_QUAD_MODE
/* Quad enable mode. */
#define ENABLE_QUAD_MODE_0 0x0
#define ENABLE_QUAD_MODE_1 0x1
#define ENABLE_QUAD_MODE_2 0x2
#define ENABLE_QUAD_MODE_3 0x3
#define ENABLE_QUAD_MODE_4 0x4
#define ENABLE_QUAD_MODE_5 0x5
#endif

#define WRITE_OPERATION 0x0
#define ERASE_OPERATION 0x1
#define OTHER_OPERATION 0x2

#define VALID_RW_MODE(__mode__)                                                                    \
	(((__mode__) >= FLASH_RW_MODE_SDR_SINGLE && (__mode__) <= FLASH_RW_MODE_SDR_QUAD) ||       \
	 ((__mode__) == FLASH_RW_MODE_DDR_QUAD))

#define IS_QUAD_MODE(__mode__) (((__mode__) & 0x3) == 0x3)

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/

/**
   Structure representing flash non-blocking operation parameters.
*/
typedef struct flash_operation_param_s {
	flash_operation_cb_t
		operation_cb; /**< The callback function for non-blocking flash operation. */
	void *user_param;     /**< The user specified parameter for the callback function. */
	qspi_cmd_t qspi_cmd;  /**< The qspi cmd for read/writ/erase. */
	uint32_t address;     /**< The start address for read/writ/erase. */
	uint32_t byte_cnt;    /**< The total number of data for read/write/erase. */
	uint32_t tried_cnt; /**< The number of data tried to read/write/erase in last operation. */
	uint8_t *buffer;    /**< The buffer pointer for read/write. */
} flash_operation_param_t;

/**
   Structure representing context for flash module.
*/
typedef struct flash_context_s {
	uint8_t state;                            /**< Flash state. */
	flash_config_data_t *config;              /**< Flash specific configurations. */
	flash_operation_param_t *operation_param; /**< Flash operation parameters. */
	uint8_t timer_comparator;                 /**< Flash timer comparator. */
	uint64_t polling_start; /**< Record the start time of polling flash operation status. */
} flash_context_t;

/*-------------------------------------------------------------------------
 * Variables
 *-----------------------------------------------------------------------*/

/* default flash type. with default clock 4Mhz.
 * 30Mhz doesn't work for now - HW bug?  */
static uint8_t default_qspi_clock = FLASH_CLOCK_4MHZ;

#endif
