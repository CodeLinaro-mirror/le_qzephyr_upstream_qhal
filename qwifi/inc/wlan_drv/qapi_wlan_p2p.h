/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 *
 * Driver-side P2P find / set-config WMI wrappers. Modeled on
 * fermion_p2p drivers/wlan/qapi_wlan_p2p.c, trimmed to just the
 * commands needed for hostap p2p_find().
 */

#ifndef __QAPI_WLAN_P2P_H__
#define __QAPI_WLAN_P2P_H__

#include <stdint.h>
#include <stddef.h>
#include "qapi_status.h"

/* Discovery type values exposed to the caller. Match the encoding used
 * in WMI_P2P_FW_FIND_CMD::type (see wmi.h).
 */
typedef enum {
    QAPI_WLAN_P2P_DISC_START_WITH_FULL_E = 0,
    QAPI_WLAN_P2P_DISC_ONLY_SOCIAL_E     = 1,
    QAPI_WLAN_P2P_DISC_PROGRESSIVE_E     = 2,
} qapi_WLAN_P2P_Disc_Type_e;

typedef struct {
    uint8_t  go_intent;
    uint8_t  reg_class;
    uint8_t  listen_channel;
    uint8_t  op_reg_class;
    uint8_t  op_channel;
    uint32_t node_age_to;
    uint8_t  max_node_count;
} qapi_WLAN_P2P_Set_Config_t;

/* Mirrors the upper-half of struct wpa_driver_scan_params, reduced to the
 * fields the firmware-offloaded P2P scan actually consumes. The host glue
 * (qcom_p2p_scan) fills this in the same way wpas_p2p_scan() fills its
 * scan_params, then hands it to qapi_WLAN_P2P_Find which serialises it
 * into a variable-length WMI_P2P_FW_FIND_CMD payload.
 *
 * Lifetime: caller owns all pointed-to buffers. qapi_WLAN_P2P_Find copies
 * everything before returning; freqs / extra_ies / ssid may be freed
 * immediately on return.
 */
typedef struct {
    qapi_WLAN_P2P_Disc_Type_e disc_type;       /* legacy hint (full/social) */
    uint32_t       timeout_In_Secs;            /* fw-side timeout (0 = none) */
    uint8_t        p2p_probe;                  /* 1 = P2P probe */
    uint8_t        include_6ghz;
    /* freqs[] — null-terminated list of frequencies in MHz (matches the
     * wpa_driver_scan_params convention). NULL or empty = let fw scan all
     * 2.4 GHz channels.
     */
    const int     *freqs;
    /* Opaque IE blob built by host (WPS Probe Req IE + P2P IE) to be
     * inserted into outgoing probe requests. Must fit in
     * WMI_P2P_FIND_EXTRA_IE_MAX.
     */
    const uint8_t *extra_ies;
    size_t         extra_ies_len;
    /* SSID for active probe. NULL/0 lets fw use the legacy wildcard. */
    const uint8_t *ssid;
    size_t         ssid_len;
} qapi_WLAN_P2P_Scan_Params_t;

qapi_Status_t qapi_WLAN_P2P_Set_Config(uint8_t device_ID,
                                       const qapi_WLAN_P2P_Set_Config_t *cfg);
qapi_Status_t qapi_WLAN_P2P_Find(uint8_t device_ID,
                                 const qapi_WLAN_P2P_Scan_Params_t *params);
qapi_Status_t qapi_WLAN_P2P_Stop_Find(uint8_t device_ID);

/* Park the radio on `freq_mhz` for `duration_ms` listening for incoming
 * Probe Requests. Optional probe_resp_ies blob is the IE template the
 * firmware should reply with; pass NULL/0 to skip (caller-managed memory,
 * copied before return). Firmware fires WMI_P2P_LISTEN_DONE_EVTID when
 * the listen window ends.
 */
qapi_Status_t qapi_WLAN_P2P_Listen(uint8_t device_ID,
                                   uint16_t freq_mhz, uint16_t duration_ms,
                                   const uint8_t *probe_resp_ies,
                                   size_t          probe_resp_ies_len);
qapi_Status_t qapi_WLAN_P2P_Cancel_Listen(uint8_t device_ID);

/* Off-channel action-frame TX for P2P negotiation. Firmware switches to
 * `freq`, transmits the fully-formed 802.11 mgmt frame in `data`, dwells
 * `wait_time_ms` so the peer's action-frame response can be received on
 * that channel, then restores the previous channel and fires
 * WMI_SEND_RAW_FRAME_EVTID. `data` must contain the complete 24-byte
 * mgmt header followed by the action body; addresses are also passed
 * separately so fw can key its ADDR1/2/3 filter without re-parsing.
 * Caller-managed memory: qapi_WLAN_P2P_Send_Action copies before return.
 */
typedef struct {
    uint32_t       freq;         /* MHz */
    uint32_t       wait_time_ms; /* dwell after TX */
    uint8_t        num_tries;    /* 1..14 */
    uint8_t        dst[6];
    uint8_t        src[6];
    uint8_t        bssid[6];
    const uint8_t *data;         /* 24B mgmt hdr + action body */
    uint32_t       data_len;
} qapi_WLAN_P2P_Send_Action_Params_t;

qapi_Status_t qapi_WLAN_P2P_Send_Action(uint8_t device_ID,
    const qapi_WLAN_P2P_Send_Action_Params_t *params);

#endif /* __QAPI_WLAN_P2P_H__ */
