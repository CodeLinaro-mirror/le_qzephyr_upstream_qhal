/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QCC730_PMU_LL_H
#define QCC730_PMU_LL_H

#include <qcom_soc.h>

typedef volatile PMU_BASE_pmu_Type pmu_hal;
extern pmu_hal *g_pmu_hal;

#define BOOT_STRAP_VALUE 0x63887466

static inline uint32_t pmu_hal_get_boot_status(pmu_hal *hal) { return hal->PMU_SYSTEM_STATUS.reg; }

#endif /* QCC730_PMU_LL_H */
