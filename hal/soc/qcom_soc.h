/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef QCOM_SOC_H
#define QCOM_SOC_H

#if CONFIG_SOC_SERIES_QCC730
// version TAPEOUT_02
#include "qcc730v2.h"
#include "qcc730v2_posmask.h"
#else
#error "SOC must be defined. See qcom_soc.h."
#endif

#endif // QCOM_SOC_H
