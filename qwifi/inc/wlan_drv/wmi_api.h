/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear*/

#ifndef __WMI_API_H__
#define __WMI_API_H__

#define WLAN_WMI_CMD_SIG_MASK_ENABLE_DONE 0x01
#define WLAN_WMI_CMD_SIG_MASK_DISABLE_DONE 0x02
#define WLAN_WMI_CMD_SIG_MASK_STARTED_SCAN 0x04
#define WLAN_WMI_CMD_SIG_MASK_SCAN_COMPLETED 0x08
#define WLAN_WMI_CMD_SIG_MASK_CONNECT_COMPLETED 0x10
#define WLAN_WMI_CMD_SIG_MASK_DISCONNECTED 0x20
#define WLAN_WMI_CMD_SIG_MASK_IF_ADDED 0x40
#define WLAN_WMI_CMD_SIG_MASK_SET_MODE 0x80
#define WLAN_WMI_CMD_SIG_MASK_GET_STAT 0x100
#define WLAN_WMI_CMD_SIG_MASK_SET_PARAM 0x200
#define WLAN_WMI_CMD_SIG_MASK_GET_REG 0x400
#define WLAN_WMI_CMD_SIG_MASK_SET_RATE 0x800
#define WLAN_WMI_CMD_SIG_MASK_GET_RATE 0x1000
#define WLAN_WMI_CMD_SIG_MASK_SEND_RAW 0x2000
#define WLAN_WMI_CMD_SIG_MASK_SET_MGMT_FILTER 0x4000
#define WLAN_WMI_CMD_SIG_MASK_GET_TX_POWER 0x8000
#ifdef CONFIG_WPS
#define WLAN_WMI_CMD_SIG_MASK_STARTED_WPS_PROCESS 0x10000
#define WLAN_WMI_CMD_SIG_MASK_STOPPED_SCAN 0x20000
#endif
#define WLAN_WMI_CMD_SIG_MASK_GET_STATUS 0x40000
#define WLAN_WMI_CMD_SIG_MASK_SUSPEND 0x80000
#define WLAN_WMI_CMD_SIG_MASK_RESUME 0x100000
#define WLAN_WMI_CMD_SIG_MASK_SAP_CSA_STATUS 0x200000

extern qapi_Status_t wmi_cmd_send(WMI_COMMAND_ID cmd_id, void *p_data, uint32_t data_len);
extern qapi_Status_t wmi_dev_cmd_send(WMI_COMMAND_ID cmd_id, uint8_t dev_id, void *p_data, uint32_t data_len);

extern void wmi_event_relay(uint32_t if_id, uint32_t event_id, void *data, uint32_t data_length,
                            void __attribute__((__unused__)) * cxt);

extern qapi_Status_t wmi_on(void);
extern qapi_Status_t wmi_off(void);
extern qapi_Status_t wmi_add_device(uint8_t __attribute__((__unused__)) device_ID);
extern qapi_Status_t wmi_start_scan(uint8_t __attribute__((__unused__)) device_ID,
                                    const qapi_WLAN_Start_Scan_Params_t *scan_Params);
extern qapi_Status_t wlan_get_scan_results(uint8_t __attribute__((__unused__)) device_ID,
                                           qapi_WLAN_Scan_Comp_Evt_t *scan_Res, int16_t *num_Bss);
extern qapi_Status_t wmi_set_passphrase(uint8_t vdev_id);
extern qapi_Status_t wmi_connect(uint8_t vdev_id);
extern qapi_Status_t wmi_disconnect(uint8_t vdev_id);
extern qapi_Status_t wmi_ap_disconnect_station(uint8_t device_ID, const uint8_t *mac_addr, uint32_t len);
extern qapi_Status_t wmi_set_op_mode(uint8_t device_ID);
extern qapi_Status_t wmi_wlan_get_statistics(uint8_t device_ID);
extern qapi_Status_t wmi_wlan_get_regulatory(void);
extern qapi_Status_t wmi_set_rate(void);
extern qapi_Status_t wmi_get_rate(void);
extern qapi_Status_t wmi_send_raw(void);
extern qapi_Status_t wmi_set_mgmt_filter(void);
extern qapi_Status_t wmi_get_tx_power(void);
#ifdef CONFIG_WPS
extern qapi_Status_t wmi_stop_scan(void);
qapi_Status_t wmi_start_wps_process(uint8_t __attribute__((__unused__)) device_ID,
                                    qapi_WLAN_WPS_Connect_Action_e connect_Action, qapi_WLAN_WPS_Mode_e mode,
                                    const char *pin, uint8_t auth_floor);

#endif

qapi_Status_t wlan_get_rts_rate(uint32_t *rate);
extern qapi_Status_t wlan_get_edca_param(uint8_t qid, uint8_t *aifs, uint16_t *cw_min, uint16_t *cw_max,
                                         uint16_t *txop_limit);
extern qapi_Status_t  wmi_suspend(void);
extern qapi_Status_t  wmi_resume(void);
extern qapi_Status_t wmi_get_wifi_status(uint8_t dev_id, WMI_WIFI_STATUS *status);
extern qapi_Status_t wmi_wlan_sap_csa(uint8_t device_ID, uint8_t switch_mode, uint16_t channel,
                                      uint8_t is_6g, uint8_t switch_count);


#endif //__WMI_API_H__
