/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/*========================================================================
 * @file fwconfig_cmn.h
 * @brief feature flag definitions of NT code base required for Fermion
 * ======================================================================*/
#ifndef _FWCONFIG_CMN_H_
#define _FWCONFIG_CMN_H_
/*------------------------------------------------------------------------
 * Include Files
 * ----------------------------------------------------------------------*/
#include "qdefconfig.h"

/* None*/
#ifdef QCCSDK
#include "autoconf.h"
#endif
#if (FERMION_CHIP_VERSION == 2)
#include "fwconfig_QCP7321.h"
#else
#include "fwconfig_QCP5321.h"
#endif

#endif
