/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/*********************************************************************************************
 * @file    wifi_fw_pwr_cb_infra.h
 * @brief   Declarations of Fermion Callback Infra
 *
 *
 ********************************************************************************************/
#ifndef _WIFI_FW_PWR_CB_INFRA_H_
#define _WIFI_FW_PWR_CB_INFRA_H_

#include "fwconfig_cmn.h"
#include "nt_flags.h"
#include "fdi.h"
#ifdef FEATURE_FDI_RMC
#include "fdi_rmc.h"
#endif
#include "stdio.h"
#include "stdlib.h"
#include <string.h>
#include <stddef.h>
#ifdef FEATURE_FPCI

#define FPCI_MAX_REG (16)
#define SORT_CONCAT_2(a, b) a##b
#define SORT_CONCAT_3(a, b, c) a##b##c

typedef enum fpci_err { FPCI_SUCCESS, FPCI_ERR } fpci_err_t;

/* @brief Register Power Event Mask as Enum */
typedef enum pwr_evt {
    PWR_EVT_WMAC_PRE_SLEEP = 1 << 1U,
    PWR_EVT_WMAC_POST_SLEEP = 1 << 2U,
    PWR_EVT_WMAC_PRE_AWAKE = 1 << 3U,
    PWR_EVT_WMAC_POST_AWAKE = 1 << 4U,
    PWR_EVT_WMAC_SLEEP_ABORT = 1 << 5U,
    PWR_EVT_PRE_IMPS_TRIGGER = 1 << 6U,
    PWR_EVT_WMAC_MAX = 1 << 7U,
} pwr_evt_t;

/* @brief Event Callback Typedef */
typedef void (*ps_evt_cb_t)(uint8_t evt, const void *p_args);

/* @brief Power Event Registration */
typedef struct pwr_evt_reg {
    uint16_t evt_mask;  /* OR Masked Event Registration flag */
    uint8_t priority;   /* Priority level of an evvent reg */
    ps_evt_cb_t evt_cb; /* Pointer to event callback */
    void *p_args;       /* Pointer to data that shall be referenced during the callback */
} pwr_evt_reg_t;

typedef enum sort_direction
{
    SORT_DIRECTION_ASSENDING  = 0,
    SORT_DIRECTION_DESSENDING = 1
} sort_direction_t;

typedef struct sort
{
    char *p_buffer;
    char *p_dummy_buff;
    size_t buffer_node_size;
    size_t size_buffer;
    size_t sort_param_offset;
    size_t sort_param_size;
} sort_t;

/*******************************************************************************
 * @brief Create a Sorting instance for user defined Structure/Unions
 *
 * @param _NAME     Name of the instance
 * @param _type     Structure data type
 * @param _buff     buffer to sort
 * @param _sort_param_offset_var    Name of parameter of structure to sort upon
 *
 *******************************************************************************/
#define SORT_INSTANCE_STRUCT(_NAME, _type, _buff, _sort_param_offset_var) \
    static _type SORT_CONCAT_3(sort_, _NAME, _dummy_buff)[sizeof(_type)]; \
    static sort_t SORT_CONCAT_3(sort_, _NAME, _inst) = {                  \
        .buffer_node_size = sizeof(_type),                                \
        .p_buffer = (char *)_buff,                                        \
        .size_buffer = sizeof(_buff),                                     \
        .sort_param_offset = offsetof(_type, _sort_param_offset_var),     \
        .sort_param_size = sizeof(((_type *)0)->_sort_param_offset_var),  \
        .p_dummy_buff = (char *)SORT_CONCAT_3(sort_, _NAME, _dummy_buff)};
 /******************************************************************************
  * @brief Create a Sorting instance for pre-defined data-types
  *
  * @param _NAME     Name of the instance
  * @param _type     data type
  * @param _buff     buffer to sort
  *
  *****************************************************************************/
#define SORT_INSTANCE_VAR(_NAME, _type, _buff)                           \
    static _type SORT_CONCAT_3(sort, _NAME, _dummy_buff)[sizeof(_type)]; \
    static sort_t SORT_CONCAT_3(sort_, _NAME, _inst) = {                 \
        .buffer_node_size = sizeof(_type),                               \
        .p_buffer = (char *)_buff,                                       \
        .size_buffer = sizeof(_buff),                                    \
        .sort_param_offset = 0,                                          \
        .sort_param_size = 0,                                            \
        .p_dummy_buff = (char *)SORT_CONCAT_3(sort_, _NAME, _dummy_buff)};

#define GET_SORT_INSTANCE(_Name) SORT_CONCAT_3(sort_, _Name, _inst)

fpci_err_t fpci_evt_cb_reg(ps_evt_cb_t cb, uint16_t evt_reg_mask, uint8_t priority, void *p_args);
fpci_err_t fpci_evt_cb_dereg(ps_evt_cb_t cb, uint16_t evt_mask);
fpci_err_t fpci_evt_dispatch(pwr_evt_t evt);

#if FPCI_DEBUG == 1
void fpci_register_test_cb(void);
#endif

#endif /* FEATURE_FPCI */
#endif /* _WIFI_FW_PWR_CB_INFRA_H_ */
