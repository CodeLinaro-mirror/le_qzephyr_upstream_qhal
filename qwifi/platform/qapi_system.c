/*
#Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
#SPDX-License-Identifier: BSD-3-Clause-Clear
 */
/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "qapi_system.h"
#include "qcc730v2_posmask.h"
#include "nt_hw.h"

/*-------------------------------------------------------------------------
 * Function Definition
 *-----------------------------------------------------------------------*/

/**
 *  Gets boot reason.
 *
 * @param[out] reason  Pointer to a qapi_boot_reason_t to receive the boot reason.
 *
 * @return #QAPI_OK on success, or #QAPI_ERROR if reason is NULL.
 */
qapi_Status_t qapi_core_obtain_boot_reason(qapi_boot_reason_t *reason)
{
    if (reason == NULL)
    {
        return QAPI_ERROR;
    }

    volatile uint32_t status = *(volatile uint32_t *)QWLAN_PMU_SYSTEM_STATUS_REG;

    if (0 == (status & PMU_BASE_pmu_PMU_SYSTEM_STATUS_COLD_WARM_BOOT_Msk)) {
        *reason = QAPI_BOOT_REASON_COLD_BOOT;
    }
    /* Deep sleep warm boot (includes indefinite deep sleep) */
    else if (QWLAN_PMU_SYSTEM_STATUS_WARM_BOOT_FROM_DEEPSLEEP_MASK ==
		    (uint32_t)(status & QWLAN_PMU_SYSTEM_STATUS_WARM_BOOT_FROM_DEEPSLEEP_MASK))
    {
        *reason = QAPI_BOOT_REASON_DEEP_SLEEP;
    }
    /* DTIM sleep wake indicators */
    else if (QWLAN_PMU_SYSTEM_STATUS_WARM_BOOT_FROM_SLEEP_MASK ==
		    (uint32_t)(status & QWLAN_PMU_SYSTEM_STATUS_WARM_BOOT_FROM_SLEEP_MASK))
    {
        *reason = QAPI_BOOT_REASON_DTIM_SLEEP;
    }
    else
    {
        *reason = QAPI_BOOT_REASON_UNKNOWN;
    }

    return QAPI_OK;
}
