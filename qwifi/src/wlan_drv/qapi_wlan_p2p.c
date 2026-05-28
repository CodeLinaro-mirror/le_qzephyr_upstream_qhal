/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Driver-side P2P WMI wrappers. Mirrors the relevant portion of
 * fermion_p2p drivers/wlan/qapi_wlan_p2p.c so the host hostap layer
 * can drive a real firmware-side scan via qapi_WLAN_P2P_Find().
 */

#include <string.h>
#include "wlan_drv.h"
#include "wmi_api.h"
#include "wmi.h"
#include "qapi_wlan_p2p.h"

qapi_Status_t qapi_WLAN_P2P_Set_Config(uint8_t device_ID,
                                       const qapi_WLAN_P2P_Set_Config_t *cfg)
{
    WMI_P2P_FW_SET_CONFIG_CMD wmi_cfg;

    (void)device_ID;
    if (cfg == NULL) {
        return QAPI_ERROR;
    }

    memset(&wmi_cfg, 0, sizeof(wmi_cfg));
    wmi_cfg.go_intent      = cfg->go_intent;
    wmi_cfg.reg_class      = cfg->reg_class;
    wmi_cfg.listen_channel = cfg->listen_channel;
    wmi_cfg.op_reg_class   = cfg->op_reg_class;
    wmi_cfg.op_channel     = cfg->op_channel;
    wmi_cfg.node_age_to    = cfg->node_age_to;
    wmi_cfg.max_node_count = cfg->max_node_count;

    return wmi_cmd_send(WMI_P2P_SET_CONFIG_CMDID, &wmi_cfg, sizeof(wmi_cfg));
}

/* Static backing buffer for WMI_P2P_FIND_CMDID. The WMI message queue is
 * fire-and-forget: wmi_cmd_send hands a pointer to the worker task and
 * returns; the caller must keep that buffer live until the worker has
 * finished processing it. Because P2P find is single-instance (a second
 * find before the first is dispatched is racy at the hostap level too),
 * a single static buffer is enough — same pattern other variable-length
 * WMI helpers in this driver use.
 */
#define P2P_FIND_BUF_SIZE  (sizeof(WMI_P2P_FW_FIND_CMD)         \
                            + (255u * sizeof(uint16_t))         \
                            + WMI_P2P_FIND_EXTRA_IE_MAX         \
                            + 32u)
static uint8_t s_find_buf[P2P_FIND_BUF_SIZE];

qapi_Status_t qapi_WLAN_P2P_Find(uint8_t device_ID,
                                 const qapi_WLAN_P2P_Scan_Params_t *params)
{
    WMI_P2P_FW_FIND_CMD *cmd;
    uint8_t      *tail;
    uint8_t       num_freqs = 0;
    uint16_t      ies_len   = 0;
    uint8_t       ssid_len  = 0;
    size_t        total_len;

    (void)device_ID;
    if (params == NULL) {
        return QAPI_ERROR;
    }

    /* Count freqs (driver convention: null-terminated int list). Clamp at
     * 0xff so num_freqs (uint8_t) cannot overflow.
     */
    if (params->freqs != NULL) {
        while (params->freqs[num_freqs] != 0 && num_freqs < 0xff) {
            num_freqs++;
        }
    }

    if (params->extra_ies != NULL && params->extra_ies_len > 0) {
        if (params->extra_ies_len > WMI_P2P_FIND_EXTRA_IE_MAX) {
            return QAPI_ERROR;
        }
        ies_len = (uint16_t)params->extra_ies_len;
    }
    if (params->ssid != NULL && params->ssid_len > 0) {
        if (params->ssid_len > 32) {
            return QAPI_ERROR;
        }
        ssid_len = (uint8_t)params->ssid_len;
    }

    total_len = sizeof(*cmd) + (size_t)num_freqs * sizeof(uint16_t)
              + ies_len + ssid_len;
    if (total_len > sizeof(s_find_buf)) {
        return QAPI_ERROR;
    }

    cmd = (WMI_P2P_FW_FIND_CMD *)s_find_buf;
    memset(cmd, 0, total_len);

    cmd->timeout       = params->timeout_In_Secs;
    cmd->type          = (uint8_t)params->disc_type;
    cmd->p2p_probe     = params->p2p_probe ? 1 : 0;
    cmd->include_6ghz  = params->include_6ghz ? 1 : 0;
    cmd->num_freqs     = num_freqs;
    cmd->extra_ies_len = ies_len;
    cmd->ssid_len      = ssid_len;

    /* Tail layout: freqs[] -> extra_ies[] -> ssid[]. uint16_t freqs are
     * little-endian on this platform; matches the rest of the WMI ABI.
     */
    tail = (uint8_t *)(cmd + 1);
    for (uint8_t i = 0; i < num_freqs; i++) {
        uint16_t v = (uint16_t)params->freqs[i];
        memcpy(tail, &v, sizeof(v));
        tail += sizeof(v);
    }
    if (ies_len > 0) {
        memcpy(tail, params->extra_ies, ies_len);
        tail += ies_len;
    }
    if (ssid_len > 0) {
        memcpy(tail, params->ssid, ssid_len);
    }

    return wmi_cmd_send(WMI_P2P_FIND_CMDID, cmd, (uint32_t)total_len);
}

qapi_Status_t qapi_WLAN_P2P_Stop_Find(uint8_t device_ID)
{
    (void)device_ID;
    return wmi_cmd_send(WMI_P2P_STOP_FIND_CMDID, NULL, 0);
}

/* Static buffer for WMI_P2P_LISTEN_CMDID. Same fire-and-forget queue
 * lifetime requirement as the find buffer above — the worker copies/uses
 * the payload after this function returns, so static storage is safer
 * (and cheaper) than alloc/free.
 */
#define P2P_LISTEN_BUF_SIZE  (sizeof(WMI_P2P_FW_LISTEN_CMD)        \
                              + WMI_P2P_LISTEN_EXTRA_IE_MAX)
static uint8_t s_listen_buf[P2P_LISTEN_BUF_SIZE];

qapi_Status_t qapi_WLAN_P2P_Listen(uint8_t device_ID,
                                   uint16_t freq_mhz, uint16_t duration_ms,
                                   const uint8_t *probe_resp_ies,
                                   size_t          probe_resp_ies_len)
{
    WMI_P2P_FW_LISTEN_CMD *cmd;
    uint16_t ies_len = 0;
    size_t   total_len;

    (void)device_ID;
    if (freq_mhz == 0 || duration_ms == 0) {
        return QAPI_ERROR;
    }
    if (probe_resp_ies != NULL && probe_resp_ies_len > 0) {
        if (probe_resp_ies_len > WMI_P2P_LISTEN_EXTRA_IE_MAX) {
            return QAPI_ERROR;
        }
        ies_len = (uint16_t)probe_resp_ies_len;
    }

    total_len = sizeof(*cmd) + ies_len;

    cmd = (WMI_P2P_FW_LISTEN_CMD *)s_listen_buf;
    memset(cmd, 0, total_len);
    cmd->freq          = freq_mhz;
    cmd->duration_ms   = duration_ms;
    cmd->extra_ies_len = ies_len;
    if (ies_len > 0) {
        memcpy((uint8_t *)(cmd + 1), probe_resp_ies, ies_len);
    }

    return wmi_cmd_send(WMI_P2P_LISTEN_CMDID, cmd, (uint32_t)total_len);
}

qapi_Status_t qapi_WLAN_P2P_Cancel_Listen(uint8_t device_ID)
{
    (void)device_ID;
    return wmi_cmd_send(WMI_P2P_CANCEL_LISTEN_CMDID, NULL, 0);
}

/* Static backing for WMI_SEND_RAW driven from P2P. Same fire-and-forget
 * queue lifetime as find/listen — SEND_RAW_FRAME carries a raw data
 * pointer that fw dereferences on the worker task after this function
 * returns, so both the command struct and the frame body must live in
 * static storage. Single-instance is fine (hostap fires the next
 * send_action only after tx_done fires or wait_time expires). */
#define P2P_ACTION_FRAME_MAX  512u
static SEND_RAW_FRAME s_p2p_action_cmd;
static uint8_t        s_p2p_action_frame[P2P_ACTION_FRAME_MAX];

qapi_Status_t qapi_WLAN_P2P_Send_Action(uint8_t device_ID,
    const qapi_WLAN_P2P_Send_Action_Params_t *params)
{
    (void)device_ID;
    if (params == NULL || params->data == NULL ||
        params->data_len == 0 ||
        params->data_len > sizeof(s_p2p_action_frame)) {
        return QAPI_ERROR;
    }
    if (params->freq == 0 || params->wait_time_ms == 0 ||
        params->num_tries == 0 || params->num_tries > 14) {
        return QAPI_ERROR;
    }

    memcpy(s_p2p_action_frame, params->data, params->data_len);

    memset(&s_p2p_action_cmd, 0, sizeof(s_p2p_action_cmd));
    s_p2p_action_cmd.deviceId     = 2;                   /* STA vdev */
    s_p2p_action_cmd.rate_Index   = 0;
    s_p2p_action_cmd.num_Tries    = params->num_tries;
    s_p2p_action_cmd.payload_Size = params->data_len;
    s_p2p_action_cmd.header_Type  = HDR_TYPE_SELF_DEF;
    s_p2p_action_cmd.seq          = 0;
    memcpy(s_p2p_action_cmd.addr1, params->dst,   6);
    memcpy(s_p2p_action_cmd.addr2, params->src,   6);
    memcpy(s_p2p_action_cmd.addr3, params->bssid, 6);
    s_p2p_action_cmd.data         = s_p2p_action_frame;
    s_p2p_action_cmd.data_Length  = params->data_len;
    s_p2p_action_cmd.p2p_freq     = params->freq;
    s_p2p_action_cmd.p2p_wait_ms  = params->wait_time_ms;

    return wmi_cmd_send(WMI_SEND_RAW,
                        &s_p2p_action_cmd, sizeof(s_p2p_action_cmd));
}
