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