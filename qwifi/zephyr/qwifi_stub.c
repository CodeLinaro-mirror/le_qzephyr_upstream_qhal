/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
  * SPDX-License-Identifier: BSD-3-Clause*/
#include <qwifi_api.h>

qapi_Status_t qapi_WLAN_Disconnect (uint8_t __attribute__((__unused__)) device_ID)
{
	return QAPI_OK;
}

qapi_Status_t qapi_WLAN_Set_Param (uint8_t __attribute__((__unused__)) device_ID, uint16_t group_ID, uint16_t param_ID, const void *data, uint32_t length,
        qapi_WLAN_Wait_For_Status_e __attribute__((__unused__)) wait_For_Status)
{
	(void)group_ID;
	(void)param_ID;
	(void)data;
	(void)length;
	return QAPI_OK;
}

qapi_Status_t qapi_WLAN_Get_Param (uint8_t __attribute__((__unused__)) device_ID, uint16_t group_ID, uint16_t param_ID, void *data, uint32_t *length)
{
	(void)group_ID;
	(void)param_ID;
	(void)data;
	(void)length;
	return QAPI_OK;
}

qapi_Status_t qapi_WLAN_Commit (uint8_t  __attribute__((__unused__)) device_ID)
{
	return QAPI_OK;
}

qapi_Status_t qapi_WLAN_Start_Scan(uint8_t device_ID, const qapi_WLAN_Start_Scan_Params_t *scan_Params)
{
	(void)device_ID;
	(void)scan_Params;
	return QAPI_OK;
}

qapi_Status_t qapi_WLAN_Enable (qapi_WLAN_Enable_e enable)
{
	(void)enable;
	return QAPI_OK;
}

void qwifi_init (void)
{
}

qapi_Status_t qapi_WLAN_Set_Callback (qapi_WLAN_Callback_t callback, void *application_Context)
{
	(void)application_Context;
	(void)callback;
	return QAPI_OK;
}
qapi_Status_t qwifi_hal_tx(uint8_t device_ID, void *buffer, uint16_t len)
{
	(void)device_ID;
	(void)buffer;
	(void)len;
	return QAPI_OK;
}

qapi_Status_t qwifi_hal_reg_rxcb(void *drv_intf_data, qwifi_drv_eth_rx_cb_t fn)
{
	(void)drv_intf_data;
	(void)fn;
	return QAPI_OK;
}

