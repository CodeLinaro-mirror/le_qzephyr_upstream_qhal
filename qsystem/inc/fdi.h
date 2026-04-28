/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/******************************************************************************
 * @file    fdi.h
 * @brief   Interface for using Fermion Debug Infra
 *
 *
 *****************************************************************************/
#ifndef _FDI_H_
#define _FDI_H_

#include "nt_flags.h"

#include "wifi_fw_dbg_infra.h"
#include "wifi_fw_dbg_infra_cmn.h"
#ifdef FEATURE_FDI_RMC
#include "fdi_rmc.h"
#endif
#ifdef FEATURE_FDI
#include "nt_common.h"

/* To Enable FDI DBG Codes*/
#define FDI_DBG (FDI_RESET)

/****************************************
 * @brief Register all debug Nodes
 *
 ***************************************/
void fdi_reg_all_nodes(void);

#endif /* FEATURE_FDI */

/* Unique per-use suffix lets --gc-sections drop unreferenced entries. */
#define _FDI_STR_(x) #x
#define _FDI_STR(x) _FDI_STR_(x)
#define FDI_PS_TXT  __attribute__((section(".__sect_ps_txt."  _FDI_STR(__COUNTER__))))
#define FDI_PS_DATA __attribute__((section(".__sect_ps_data." _FDI_STR(__COUNTER__))))

#endif /* _FDI_H_ */
