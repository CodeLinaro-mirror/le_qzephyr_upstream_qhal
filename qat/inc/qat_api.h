/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QAT_API_H
#define QAT_API_H

#include "cat.h"
#include <zephyr/init.h>
#include "ring_service.h"

/*-------------------------------------------------------------------------
 * Type Declarations
 *-----------------------------------------------------------------------*/
typedef enum {
    /*-------------------------------------------------------------------------
      Numbered per ITU-T V.25 ter
    -------------------------------------------------------------------------*/
    QAT_RC_OK = 0,          /*  AT: OK            */
    QAT_RC_CONNECT = 1,     /*  AT: CONNECT       */
    QAT_RC_RING = 2,        /*  AT: RING          */
    QAT_RC_NO_CARRIER = 3,  /*  AT: NO CARRIER    */
    QAT_RC_ERROR = 4,       /*  AT: ERROR         */
    QAT_RC_NO_DIALTONE = 6, /*  AT: NO DIAL TONE  */
    QAT_RC_BUSY = 7,        /*  AT: BUSY          */
    QAT_RC_NO_ANSWER = 8,   /*  AT: NO ANSWER     */

    QAT_RC_CONNECT_W_PARAMETER = 9, /*	AT: CONNECT <parameter>	  */
    QAT_RC_QUIET = 10,              /*  AT: No Messages           */
    QAT_RC_QUIET_NO_CR = 11,        /*  AT: No Messages           */

    QAT_RC_MAX, /*	AT: Max Num   */

} QAT_Result_Enum_Type;

/* Configuration macros */
#define QAT_RING_ID RING_0                          /* Ring Service ID for all communication */
#define QAT_RESPONSE_BUF_SIZE CONFIG_RING0_BUF_SIZE /* Response buffer size for QAT_Response_Str */

/**
 * @brief Register a command group with QAT
 *
 * This function is called by command group modules to register themselves.
 *
 * @param get_cmd_group Function that returns the command group
 * @param name Name of the command group (for logging)
 * @return 0 on success, negative error code on failure
 */
int qat_register_cmd_group(struct cat_command_group *(*get_cmd_group)(void), const char *name);

/**
 * @brief Get the list of registered command groups
 *
 * @param groups Pointer to receive the array of command group pointers
 * @param count Pointer to receive the number of registered groups
 * @return 0 on success, negative error code on failure
 */
int qat_get_cmd_groups(struct cat_command_group ***groups, uint8_t *count);

/**
 * @brief Output data to the AT command interface
 *
 * This function writes data directly to the IO interface, allowing
 * command handlers to send custom response data before the OK/ERROR response.
 *
 * @param Length Number of bytes to write
 * @param Buffer Pointer to the data buffer
 * @return 0 on success, negative error code on failure
 */
int QAT_Output(uint32_t Length, const char *Buffer);

/**
 * @brief Send AT response data and return the corresponding cat_return_state.
 *
 * This function is designed to be used directly as the return value of a
 * libcat command callback (read/write/run/test handler).
 *
 * Behaviour:
 *   - Sends only the optional buffer (with \r\n framing) via QAT_Output.
 *   - Does NOT send the result string (OK/ERROR) — libcat does that based
 *     on the returned cat_return_state.
 *   - QAT_RC_QUIET_NO_CR: sends buffer raw (no \r\n framing).
 *   - QAT_RC_QUIET:       sends \r\n[buffer\r\n], returns CAT_RETURN_STATE_OK
 *                         (intended for use outside libcat callbacks; caller
 *                          ignores the return value).
 *
 * Return value mapping:
 *   QAT_RC_ERROR / NO_CARRIER / NO_DIALTONE / BUSY / NO_ANSWER
 *                              → CAT_RETURN_STATE_ERROR  (libcat sends ERROR)
 *   all other codes            → CAT_RETURN_STATE_OK     (libcat sends OK)
 *
 * Usage inside a libcat read callback:
 * @code
 *   cat_return_state my_read(const struct cat_command *cmd,
 *                            uint8_t *data, size_t *data_size,
 *                            const size_t max_data_size)
 *   {
 *       return QAT_Response_Str(QAT_RC_OK, "+MYRESP: 1");
 *       // sends "\r\n+MYRESP: 1\r\n", libcat appends "\r\nOK\r\n"
 *   }
 * @endcode
 *
 * Usage outside libcat (unsolicited / no result code needed):
 * @code
 *   QAT_Response_Str(QAT_RC_QUIET, "+EVENT: data");
 *   // sends "\r\n+EVENT: data\r\n", return value ignored
 * @endcode
 *
 * @param ret_code  Result code (QAT_Result_Enum_Type)
 * @param buffer    Optional data string to send before the result code
 *                  (NULL if not needed)
 * @return cat_return_state to be returned directly to libcat
 */
cat_return_state QAT_Response_Str(QAT_Result_Enum_Type ret_code, const char *buffer);

/**
 * @brief QAT transfer mode enumeration
 */
typedef enum {
    QAT_Transfer_Mode_AT_COMMAND_E,  /**< Process AT command strings */
    QAT_Transfer_Mode_ONLINE_DATA_E, /**< Online data state (passthrough) */
} QAT_Transfer_Mode_t;

/**
 * @brief Data passthrough callback function type
 *
 * This callback is invoked when data is received in online data mode.
 * The upper layer can process the data as needed.
 *
 * @param data Pointer to received data
 * @param len Length of received data
 * @return Number of bytes processed, or negative error code
 */
typedef int (*qat_passthrough_callback_t)(const uint8_t *data, size_t len);

/**
 * @brief Set QAT transfer mode
 *
 * @param mode Transfer mode (AT_COMMAND or ONLINE_DATA)
 * @param callback Callback function for online data mode (NULL for AT command mode)
 * @return 0 on success, negative error code on failure
 */
int QAT_Transfer_Mode_set(QAT_Transfer_Mode_t mode, qat_passthrough_callback_t callback);

/**
 * @brief Get current QAT transfer mode
 *
 * @return Current transfer mode
 */
QAT_Transfer_Mode_t QAT_Transfer_Mode_get(void);

/**
 * @brief Start QAT service timer
 * @return 0 on success, negative error code on failure
 */
int qat_start_service_timer(void);

/**
 * @brief Stop QAT service timer
 * @return 0 on success, negative error code on failure
 */
int qat_stop_service_timer(void);

/**
 * @brief Return service timer status
 * @return true on run, false on stop
 */
bool qat_is_service_timer_running(void);

/**
 * @brief Register a command group with QAT using auto-init
 *
 * This macro creates an initialization function that automatically registers
 * the command group during system initialization.
 *
 * Usage example:
 * @code
 * // In your command group source file (e.g., qat_common.c)
 * struct cat_command_group *qat_common_get_command_group(void)
 * {
 *     return &common_cmd_group;
 * }
 *
 * QAT_REGISTER_CMD_GROUP(qat_common_get_command_group, "COMMON");
 * @endcode
 *
 * @param _get_func Function that returns pointer to cat_command_group
 * @param _name String name for the command group (for logging)
 */
#define QAT_REGISTER_CMD_GROUP(_get_func, _name)                                                                       \
    static int _CONCAT(_get_func, _register)(void) { return qat_register_cmd_group(_get_func, _name); }                \
    SYS_INIT(_CONCAT(_get_func, _register), APPLICATION, 80)

/**
 * @brief Exit libcat hold state with OK response.
 *
 * Call this from a data-mode callback after the data transfer completes
 * successfully. libcat will send "\r\nOK\r\n" and return to idle.
 */
void qat_hold_exit_ok(void);

/**
 * @brief Exit libcat hold state with ERROR response.
 */
void qat_hold_exit_error(void);

#endif /* QAT_API_H */
