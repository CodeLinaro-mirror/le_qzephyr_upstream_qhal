/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear*/
// $QTI_LICENSE_QDN_C$

#ifndef __QAPI_WLAN_PARAM_GROUP_H__
#define __QAPI_WLAN_PARAM_GROUP_H__

/**
 * @file qapi_wlan_param_group.h
 *
 * @brief WLAN param group definitions
 *
 * @details This section provides APIs, macros definitions, enumerations and data structures
 *          for applications to perform WLAN control operations.
 */

#include "qapi_types.h"
#include "qapi_status.h"

/**
Macro that indicates the group ID that can be used to configure wireless parameters of
the WLAN subsystem.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS 1

/**
Macro that indicates the group ID that can be used to configure security parameters of
the WLAN subsystem.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_SECURITY 2

/**
Command ID to set/get the operating mode of a given virtual device in the WLAN subsystem.

Mode-specific parameters should be set only after setting the mode (AP/STA) using
this command.

If a concurrent mode of operation is enabled, the SAP can only be operated in virtual
device 0, and the STA mode of operation can only operate in virtual device 1.

In the case of a single device mode of operation, device 0 can be used for both STA and
SAP mode of operation.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] opMode  Address of the variable type qapi_WLAN_Dev_Mode_e

@sa
qapi_WLAN_Dev_Mode_e
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE 1

/**
Command ID to set/get the operating wireless channel of a given virtual device in
the WLAN subsystem.

For set/get operations, channel values are set in numbers (channel number 1-14, 36-165)
and not in frequency values.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] uint32_t    Variable that holds the channel number.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_CHANNEL 2

/**
Command ID to set/get the transmit power in dBm of a given virtual device.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] uint32_t        Address of the variable that holds the power value.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_TX_POWER_IN_DBM 4

/**
Command ID to set/get the SSID of/for a given virtual device in the WLAN subsystem.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] uint8_t[] Unsigned byte array of size __QAPI_WLAN_MAX_SSID_LENGTH.

@sa
__QAPI_WLAN_MAX_SSID_LENGTH
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_SSID 5

/**
Command ID to set/get the BSSID of/for a given virtual device in the WLAN subsystem when operation in STA mode.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] uint8_t[] Unsigned byte array of size __QAPI_WLAN_MAC_LEN.

@sa
__QAPI_WLAN_MAC_LEN

*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_BSSID 6

/**
Command ID to set/get the wireless PHY mode of/for a given virtual device.

The Set operation for this should be done before establishing a connection.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] qapi_WLAN_Phy_Mode_e    Required PHY mode should be specified.

@sa
qapi_WLAN_Phy_Mode_e
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_PHY_MODE 7

/**
Command ID to allow/disallow aggregation for Tx and Rx on a TID basis.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] qapi_WLAN_Aggregation_Params_t  Strucuture populated with
                                           required aggregation parameters
                                           for Tx and Rx.

@sa
qapi_WLAN_Aggregation_Params_t
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_ALLOW_TX_RX_AGGR_SET_TID 9

/**
Command ID to set/get the virtual device power mode.

The supported modes are Power Save mode (also known as REC_POWER) and Performance mode (MAX_PERF).
Applications are recommended to configure virtual devices in Performance mode
when more than one virtual device is connected (concurrency enabled).

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] qapi_WLAN_Power_Mode_Params_t         Power mode configurations.

@sa
qapi_WLAN_Power_Mode_Params_t
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_POWER_MODE_PARAMS 14

/** @cond */
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_COUNTRY_CODE 16
/** @endcond */

/** @cond */
/**
Used to set device MAC address.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_MAC_ADDRESS 23
/** @endcond */

/** @cond */
/**
Used to set application information element in outgoing frames.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_APP_IE 28
/** @endcond */

/**
Command ID to configure the 802.11 listen interval when operating in Station mode.
This value will be used in the listen interval field of the association request frame.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t        Listen interval in multiples of beacon intervals
                           (a value of 1 corresponds to 1 beacon interval).
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_LISTEN_INTERVAL_IN_TU 32

/**
Command ID to get the RSSI of the associated peer.

@note1hang This parameter can only be used with qapi_WLAN_Get_Param().

@param[out] uint8_t         RSSI value variable received from the firmware.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_RSSI 33

/**
Command ID to set reiceive AMSDU enable or disable.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t      Set to TRUE to enable AMSDU receive mode, FALSE otherwise.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_AMSDU_RX 39

/**
Command ID to set the beacon interval (in time units) when operating in SoftAP mode.
One TU = 1024 microseconds.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t      Number of TUs between every beacon in SoftAP mode.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_BEACON_INTERVAL_IN_TU 40

/**
Command ID to enable/disable the hidden SSID feature when operating a virtual device
in SoftAP mode.

This should be done after setting the operating mode as AP and before
committing the AP profile using qapi_WLAN_Commit().

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t        Set to TRUE to enable the hidden SSID feature, FALSE otherwise.

@dependencies
Should be set after setting the operating mode to AP by issuing __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE.\n
Should be set before invoking qapi_WLAN_Commit() to start SoftAP.

@sa
__QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE\n
qapi_WLAN_Commit()
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_ENABLE_HIDDEN_MODE 41

/**
Command ID to set an AP's inactivity period in seconds.

If no keepalive frames are received from an associated station during
this period, the AP deassociates that station.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t        Inactivity interval for associated stations in
                           seconds.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_INACTIVITY_TIME_IN_SECONDS 43

/**
Command ID to change the DTIM interval when operating a virtual device
in SoftAP mode.

This setting should be done before committing the AP profile using qapi_WLAN_Commit().

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t        DTIM interval in multiples of the beacon interval.

@dependencies
Should be set after setting the operating mode to AP by issuing __QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE.\n
Should be set before invoking qapi_WLAN_Commit() to start SoftAP.

@sa
__QAPI_WLAN_PARAM_GROUP_WIRELESS_OPERATION_MODE\n
qapi_WLAN_Commit
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_AP_DTIM_INTERVAL 45

/**
Command ID to set wireless 11n HT parameters of a given virtual device. The set
operation for this should be done before establishing a connection.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] qapi_WLAN_11n_HT_Config_e    Required 11n HT configuration must be specified.

@sa
qapi_WLAN_11n_HT_Config_e
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_11N_HT 60

/**
Command ID to set Beacon Miss configuration.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in,out] qapi_WLAN_Sta_Config_Bmiss_Config_t    Beacon Miss parameters to be set.

@sa
qapi_WLAN_Sta_Config_Bmiss_Config_t
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_STA_BMISS_CONFIG 61

/**
Command ID to set/get the concurrency mode of device in the WLAN subsystem.

@note1hang This parameter can be used with qapi_WLAN_Get_Param().

@param[in,out] concurrency mode  Address of the variable type qapi_WLAN_DEV_Mode_e

@sa
qapi_WLAN_DEV_Mode_e
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_CONCURRENCY_MODE 82

/**
Command ID to enable/disable RTS/CTS protection when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t        Set 1 to enable RTS/CTS protection, 0 to be disabled.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_RTS 84

/**
Command ID to fix RTS rate in 2G when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t        0: 1Mbps  1: 6Mbps
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_RTS_RATE_2G 85

/**
Command ID to adjust edca parameters when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] qapi_WLAN_Edca_Params_t  Set edca parameters for queue 0-7.

@sa
#qapi_WLAN_Edca_Params_t
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_EDCA_PARAM 86

/**
Command ID to adjust PER upper threshold when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t  Set PER upper threshold to 0-100.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_PER_UPPER_THRESHOLD 87

/**
Command ID to adjust BA window size when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] qapi_WLAN_BA_Window_Params_t  Set BA window size.

@sa
#qapi_WLAN_BA_Window_Params_t
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_BA_WINDOW 88

/**
Command ID to adjust BA window size when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint32_t  change slot time to 9us/20us.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_SLOT_TIME 89

/**
Command ID to adjust EDCCA threshold when operating in Station mode.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

Set EDCCA threshold to 0-100.
*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_EDCCA_THRESHOLD 90

/**
Command ID to get Wi-Fi status that used by Zephyr.

@note1hang This parameter can only be used with qapi_WLAN_Get_Param().

@param[out] qapi_WLAN_Status_t get Wi-Fi status.

*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_WIFI_STATUS 91

/**
Command ID to set active device for concurrency mode

@note1hang This parameter can only be used with qapi_WLAN_Get_Param().

Set actice device id to 0/1(SAP/STA).

*/
#define __QAPI_WLAN_PARAM_GROUP_WIRELESS_DEVICE_ID 92


#define __QAPI_WLAN_PARAM_GROUP_SECURITY_AUTH_MODE 0

/**
Command ID to set/get the encryption mode for an upcoming association operation.

@note1hang This parameter can be used with qapi_WLAN_Set_Param() and qapi_WLAN_Get_Param().

@param[in,out] qapi_WLAN_Crypt_Type_e   Encryption mode to be set.

@dependencies
Encryption mode must be set before connecting to the peer.

@sa
qapi_WLAN_Crypt_Type_e
*/
#define __QAPI_WLAN_PARAM_GROUP_SECURITY_ENCRYPTION_TYPE 1

/**
Command ID to set the passphrase for the upcoming WPA/WPA2 association procedure.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] uint8_t[]       Passphrase to be set. The passphrase length
                           should not exceed __QAPI_WLAN_PASSPHRASE_LEN.
@dependencies
This should be done before initiating an association.

@sa
__QAPI_WLAN_PASSPHRASE_LEN
*/
#define __QAPI_WLAN_PARAM_GROUP_SECURITY_PASSPHRASE 3

/**
Command ID to set WPS credentials received after WPS negotiation with the peer.
These are the credentials that will be used for secure association with the peer.

@note1hang This parameter can only be used with qapi_WLAN_Set_Param().

@param[in] qapi_WLAN_WPS_Credentials_t     WPS credential information to be used
                                           for a secure association.

@dependencies
This should be done after WPS negotiation is completed and before performing a
secure association.

@sa
qapi_WLAN_WPS_Credentials_t
*/
#define __QAPI_WLAN_PARAM_GROUP_SECURITY_WPS_CREDENTIALS 6

#endif /* __QAPI_WLAN_PARAM_GROUP_H__ */
