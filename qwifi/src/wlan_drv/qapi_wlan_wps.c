/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * WPS scan QAPI and WMI layer implementation.
 *
 * This file is compiled only when CONFIG_WIFI_QCOM_WPS is enabled.
 * All WPS-scan-related QAPI functions, WMI command senders and WMI
 * event handlers are consolidated here so that enabling/disabling the
 * feature touches only this file and its CMakeLists entry.
 */


#include "wlan_drv.h"
#include "wmi_api.h"
#include "wlan_qapi_helper.h"
#include "safeAPI.h"
#include "qapi_wlan_base.h"

/* QAPI wsc_ie[] must fit within the WMI IE payload cap so wmi_wps_scan_ap_result_event()
 * never needs to grow the QAPI buffer when truncating wsc_ie_len. */
_Static_assert(sizeof(((qapi_WLAN_WPS_Scan_AP_Result_t *)0)->wsc_ie) <= WMI_WPS_SCAN_IE_MAX_LEN,
               "QAPI wsc_ie buffer must not exceed WMI_WPS_SCAN_IE_MAX_LEN");

/* ------------------------------------------------------------------ */
/* WMI event handlers (called from wmi_api.c event dispatch)           */
/* ------------------------------------------------------------------ */

/*
 * wmi_wps_scan_ap_result_event — WMI_WPS_SCAN_RESULT_EVTID
 * Delivered once per WPS-capable AP found during a scan pass.
 */
void wmi_wps_scan_ap_result_event(void *msg)
{
    wlan_qapi_cxt_t *p_cxt = gp_wlan_qapi_cxt;
    WMI_WPS_SCAN_AP_RESULT *res = (WMI_WPS_SCAN_AP_RESULT *)msg;

    if (!msg) {
        warn_printf("wmi_wps_scan_ap_result_event: NULL msg\n");
        return;
    }

    qurt_mutex_lock(p_cxt->wlan_qapi_cxt_mutex);
    memcpy(p_cxt->wps_scan_ap, res, sizeof(WMI_WPS_SCAN_AP_RESULT));
    /* wps_scan_ap is passed to the callback as qapi_WLAN_WPS_Scan_AP_Result_t
     * whose wsc_ie[] is 192 bytes vs WMI_WPS_SCAN_IE_MAX_LEN (210).
     * Truncate wsc_ie_len so the callback never reads past the QAPI buffer. */
    qapi_WLAN_WPS_Scan_AP_Result_t *ap_qapi =
        (qapi_WLAN_WPS_Scan_AP_Result_t *)p_cxt->wps_scan_ap;
    if (ap_qapi->wsc_ie_len > sizeof(ap_qapi->wsc_ie))
        ap_qapi->wsc_ie_len = sizeof(ap_qapi->wsc_ie);
    if (p_cxt->qapi_event_handler) {
        p_cxt->qapi_event_handler(QCOM_DEV_STA_ID,
                                  QAPI_WLAN_WPS_SCAN_AP_CB_E,
                                  p_cxt->event_application_Context,
                                  p_cxt->wps_scan_ap,
                                  sizeof(WMI_WPS_SCAN_AP_RESULT));
    }
    qurt_mutex_unlock(p_cxt->wlan_qapi_cxt_mutex);
}

/*
 * wmi_wps_scan_comp_event — WMI_WPS_SCAN_COMP_EVTID
 * Delivered once when a full WPS scan round completes.
 */
void wmi_wps_scan_comp_event(void *msg)
{
    wlan_qapi_cxt_t *p_cxt = gp_wlan_qapi_cxt;
    WMI_WPS_SCAN_COMP_RESULT *res = (WMI_WPS_SCAN_COMP_RESULT *)msg;

    if (!msg) {
        warn_printf("wmi_wps_scan_comp_event: NULL msg\n");
        return;
    }

    qurt_mutex_lock(p_cxt->wlan_qapi_cxt_mutex);
    memcpy(p_cxt->wps_scan_comp, res, sizeof(WMI_WPS_SCAN_COMP_RESULT));
    p_cxt->wps_scan_in_progress = false;
    if (p_cxt->wlan_wps_scan_block_mode) {
        qurt_signal_set(p_cxt->wlan_cmd_done,
                        WLAN_WMI_CMD_SIG_MASK_WPS_SCAN_COMP);
    }
    if (p_cxt->qapi_event_handler) {
        p_cxt->qapi_event_handler(QCOM_DEV_STA_ID,
                                  QAPI_WLAN_WPS_SCAN_COMP_CB_E,
                                  p_cxt->event_application_Context,
                                  p_cxt->wps_scan_comp,
                                  sizeof(WMI_WPS_SCAN_COMP_RESULT));
    }
    qurt_mutex_unlock(p_cxt->wlan_qapi_cxt_mutex);
}

/* ------------------------------------------------------------------ */
/* WMI command senders                                                  */
/* ------------------------------------------------------------------ */

qapi_Status_t wmi_wps_scan(uint8_t __attribute__((__unused__)) device_ID,
                            const qapi_WLAN_WPS_Scan_Params_t *params)
{
    wlan_qapi_cxt_t *p_cxt = gp_wlan_qapi_cxt;
    WMI_WPS_SCAN_CMD cmd = {0};

    switch (params->op) {
    case QAPI_WLAN_WPS_SCAN_START_PBC_E:
        cmd.op       = WPS_SCAN_OP_START;
        cmd.wps_mode = (uint8_t)QAPI_WLAN_WPS_PBC_MODE_E;
        break;
    case QAPI_WLAN_WPS_SCAN_START_PIN_E:
        cmd.op       = WPS_SCAN_OP_START;
        cmd.wps_mode = (uint8_t)QAPI_WLAN_WPS_PIN_MODE_E;
        break;
    case QAPI_WLAN_WPS_SCAN_STOP_E:
        cmd.op       = WPS_SCAN_OP_STOP;
        cmd.wps_mode = 0;
        break;
    default:
        log_printf("wmi_wps_scan: unknown op %d\n", (int)params->op);
        return QAPI_WLAN_ERROR;
    }

    if (cmd.op == WPS_SCAN_OP_START) {
        /* channel list */
        if (params->channel_count > 0) {
            uint8_t n = (params->channel_count > WMI_WPS_SCAN_MAX_CHANNELS)
                        ? WMI_WPS_SCAN_MAX_CHANNELS : params->channel_count;
            cmd.channel_count = n;
            memscpy(cmd.channels, n * sizeof(uint16_t),
                    params->channels, n * sizeof(uint16_t));
        }
        /* BSSID filter */
        memscpy(cmd.filter_bssid, sizeof(cmd.filter_bssid),
                params->bssid, sizeof(params->bssid));

        qurt_mutex_lock(p_cxt->wlan_qapi_cxt_mutex);
        if (p_cxt->wps_scan_in_progress) {
            qurt_mutex_unlock(p_cxt->wlan_qapi_cxt_mutex);
            log_printf("wmi_wps_scan: scan already in progress\n");
            return QAPI_WLAN_ERROR;
        }
        p_cxt->wps_scan_in_progress = true;
        qurt_mutex_unlock(p_cxt->wlan_qapi_cxt_mutex);
    }

    wmi_cmd_send(WMI_WPS_SCAN_CMDID, &cmd, sizeof(cmd));

    if (cmd.op == WPS_SCAN_OP_START && p_cxt->wlan_wps_scan_block_mode) {
        qurt_signal_wait(p_cxt->wlan_cmd_done,
                         WLAN_WMI_CMD_SIG_MASK_WPS_SCAN_COMP,
                         QURT_SIGNAL_ATTR_CLEAR_MASK);
    }
    return QAPI_OK;
}

/* ------------------------------------------------------------------ */
/* Public QAPI                                                          */
/* ------------------------------------------------------------------ */

/**
 * qapi_WLAN_WPS_Scan — start or stop a WPS scan.
 *
 * params->op == QAPI_WLAN_WPS_SCAN_START_PBC_E : start PBC mode scan
 * params->op == QAPI_WLAN_WPS_SCAN_START_PIN_E : start PIN mode scan
 * params->op == QAPI_WLAN_WPS_SCAN_STOP_E      : stop in-progress scan
 *
 * For start operations, params->channel_count > 0 restricts the scan to the
 * listed 802.11 channel numbers.  params->bssid non-zero filters AP results
 * to the specified BSSID.  Both fields are ignored for STOP.
 */
qapi_Status_t qapi_WLAN_WPS_Scan(uint8_t device_ID,
                                   const qapi_WLAN_WPS_Scan_Params_t *params)
{
    qapi_Status_t ret = QAPI_WLAN_ERROR;

    if (!params) {
        log_printf("qapi_WLAN_WPS_Scan: NULL params\n");
        return QAPI_WLAN_ERROR;
    }

    WLAN_QAPI_LOCK();
    switch (params->op) {
    case QAPI_WLAN_WPS_SCAN_START_PBC_E:
    case QAPI_WLAN_WPS_SCAN_START_PIN_E:
    case QAPI_WLAN_WPS_SCAN_STOP_E:
        ret = wmi_wps_scan(device_ID, params);
        break;
    default:
        log_printf("qapi_WLAN_WPS_Scan: unknown op %d\n", (int)params->op);
        ret = QAPI_WLAN_ERROR;
        break;
    }
    WLAN_QAPI_UNLOCK();
    return ret;
}
