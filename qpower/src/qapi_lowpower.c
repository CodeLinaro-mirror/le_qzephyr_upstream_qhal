/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "qapi_lowpower.h"
#include "nt_socpm_sleep.h"
#include "qpower_internal.h"
#include "wifi_fw_pwr_cb_infra.h"
#include "wmi.h"
#include "wmi_api.h"
#include "HALhwio.h"
#include <stdlib.h>
#include <string.h>

lpr_wmi_t g_lowpower_wmi;
extern bool (*wakeup_cb_dtim)(uint16_t type, bool bm_cast,void* pbuf,uint16_t len);

/**
 *  Register the callback to filter BC/MC packets
 *
 * @param[in] bmps_cb  the callback to filter the BC/MC packets
 * @param[in] net_cb   not used currently
 *
 * @return #QAPI_OK on success, or #QAPI_ERR_INVALID_PARAM if bmps_cb is NULL.
 */
qapi_Status_t qapi_bmps_bcmc_rx_filter_cb_register(qapi_bmps_rx_filter_cb bmps_cb, qapi_bmps_rx_filter_cb net_cb)
{
    if(!bmps_cb)
    {
        return QAPI_ERR_INVALID_PARAM;
    }

    if(bmps_cb)
    {
        wakeup_cb_dtim = bmps_cb;
    }

    /** wakeup_cb_net = net_cb; */
    

   return QAPI_OK;
}

/**
 *  Enable/disable the power optimization when in active mode. By enabling this feature, BMPS will also works when in active mode.
 *
 * @param[in] enable  1 for enable, 0 for disable.
 *
 * @return #QAPI_OK on success, or #QAPI_ERR_INVALID_PARAM if enable is not 1 or 0.
 */
qapi_Status_t qapi_bmps_power_optimization_enable(uint8_t enable)
{
    if (enable != 0 && enable != 1) {
        return QAPI_ERR_INVALID_PARAM;
    }

    WMI_BMPS_PWR_OPT_ENABLE *pdata = (WMI_BMPS_PWR_OPT_ENABLE *)&g_lowpower_wmi.bmps_cfg.bmps_pwr_opt_enable;
    memset(pdata, 0, sizeof(*pdata));
    pdata->enable = enable;
    wmi_cmd_send(WMI_BMPS_PWR_OPT_ENABLE_CMDID, pdata, sizeof(*pdata));
}

/**
 *  Enable/disable the compressing of qos-null sending. By enabling this feature, QoS-Null will not be sent when unnecessary during BMPS..
 *
 * @param[in] enable  1 for enable, 0 for disable.
 *
 * @return #QAPI_OK on success, or #QAPI_ERR_INVALID_PARAM if enable is not 1 or 0.
 */
qapi_Status_t qapi_bmps_compress_qos_null_enable(uint8_t enable)
{
    if (enable != 0 && enable != 1) {
        return QAPI_ERR_INVALID_PARAM;
    }

    WMI_BMPS_CMPR_QOS_NULL_ENABLE *pdata = (WMI_BMPS_CMPR_QOS_NULL_ENABLE *)&g_lowpower_wmi.bmps_cfg.bmps_cmpr_qos_null_enable;
    memset(pdata, 0, sizeof(*pdata));
    pdata->enable = enable;
    wmi_cmd_send(WMI_BMPS_CMPR_QOS_NULL_ENABLE_CMDID, pdata, sizeof(*pdata));
}