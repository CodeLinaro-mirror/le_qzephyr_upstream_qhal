 /*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <cat.h>
#include "qat_api.h"

LOG_MODULE_REGISTER(qat_io, LOG_LEVEL_INF);

/* Configuration macros */
#define QAT_RX_BUFFER_SIZE QAT_RESPONSE_BUF_SIZE /* RX ring buffer size */
#define QAT_TX_BUFFER_SIZE QAT_RX_BUFFER_SIZE    /* TX buffer size for accumulating responses */
#define QAT_RX_TEMP_BUF_SIZE QAT_RX_BUFFER_SIZE  /* Temporary buffer size for ring_recv */
#define QAT_RING_SEND_TIMEOUT 100                /* Ring send timeout in milliseconds */
#define QAT_EXIT_SEQ "+++"                       /* Exit sequence for online data mode */

/* RX ring buffer for AT commands */
static uint8_t rx_buffer[QAT_RX_BUFFER_SIZE];
static size_t rx_head = 0;
static size_t rx_tail = 0;
static size_t rx_count = 0;

/* TX buffer for accumulating response */
static uint8_t tx_buffer[QAT_TX_BUFFER_SIZE];
static size_t tx_count = 0;

/* Transfer mode state */
static struct {
    QAT_Transfer_Mode_t mode;
    qat_passthrough_callback_t callback;
} transfer_state = {
    .mode = QAT_Transfer_Mode_AT_COMMAND_E,
    .callback = NULL,
};

/* Synchronization */
static K_MUTEX_DEFINE(rx_mutex);
static K_MUTEX_DEFINE(tx_mutex);
static K_MUTEX_DEFINE(mode_mutex);
static K_SEM_DEFINE(rx_sem, 0, 1);

/**
 * Check if data is the exit sequence (+++
)
 * Returns true if data is "+++" optionally followed by \r or \r\n
 */
static bool check_exit_sequence(const uint8_t *data, size_t len)
{
    /* Check for various valid exit sequence formats:
     * - "+++" (3 bytes)
     * - "+++\r" (4 bytes)
     * - "+++\n" (4 bytes)
     * - "+++\r\n" (5 bytes)
     */

    if (len < 3 || len > 5) {
        return false;
    }

    /* First 3 bytes must be +++ */
    if (data[0] != '+' || data[1] != '+' || data[2] != '+') {
        return false;
    }

    /* Check optional line endings */
    if (len == 3) {
        /* Just +++ */
        LOG_DBG("Detected exit sequence: +++");
        return true;
    } else if (len == 4) {
        /* +++\r or +++\n */
        if (data[3] == '\r' || data[3] == '\n') {
            LOG_DBG("Detected exit sequence: +++\\%c", data[3] == '\r' ? 'r' : 'n');
            return true;
        }
    } else if (len == 5) {
        /* +++\r\n */
        if (data[3] == '\r' && data[4] == '\n') {
            LOG_DBG("Detected exit sequence: +++\\r\\n");
            return true;
        }
    }

    return false;
}

/**
 * Handle online data mode
 */
static void handle_online_data(const uint8_t *data, size_t len)
{
    k_mutex_lock(&mode_mutex, K_FOREVER);

    if (transfer_state.mode != QAT_Transfer_Mode_ONLINE_DATA_E || !transfer_state.callback) {
        k_mutex_unlock(&mode_mutex);
        return;
    }

    /* Check for exit sequence +++ */
    if (check_exit_sequence(data, len)) {
        LOG_DBG("Exit sequence detected, switching to AT command mode");

        /* Switch back to AT command mode */
        transfer_state.mode = QAT_Transfer_Mode_AT_COMMAND_E;
        transfer_state.callback = NULL;

        k_mutex_unlock(&mode_mutex);

        /* Send OK response */
        QAT_Response_Str(QAT_RC_QUIET, "OK\r\n");
        return;
    }

    /* Call user callback */
    int ret = transfer_state.callback(data, len);
    if (ret < 0) {
        LOG_ERR("Online data callback error: %d", ret);
    }

    k_mutex_unlock(&mode_mutex);
}

/**
 * Ring Service callback - receives data from Host
 */
static void qat_rx_callback(uint8_t ring_id, void *user_data)
{
    uint8_t temp_buf[QAT_RX_TEMP_BUF_SIZE];
    int ret;
    QAT_Transfer_Mode_t current_mode;

    if (ring_id != QAT_RING_ID) {
        return;
    }

    /* Check current mode */
    current_mode = QAT_Transfer_Mode_get();

    /* Read data from Ring Service */
    while (true) {
        ret = ring_recv(ring_id, temp_buf, sizeof(temp_buf), K_NO_WAIT);
        if (ret <= 0) {
            break;
        }

        LOG_DBG("RX: received %d bytes, mode=%d", ret, current_mode);

        /* Handle based on current mode */
        if (current_mode == QAT_Transfer_Mode_ONLINE_DATA_E) {
            /* Online data mode - pass to callback */
            handle_online_data(temp_buf, ret);
        } else {
            /* AT command mode - buffer for libcat */
            k_mutex_lock(&rx_mutex, K_FOREVER);

            for (int i = 0; i < ret && rx_count < QAT_RX_BUFFER_SIZE; i++) {
                rx_buffer[rx_tail] = temp_buf[i];
                rx_tail = (rx_tail + 1) % QAT_RX_BUFFER_SIZE;
                rx_count++;
            }

            k_mutex_unlock(&rx_mutex);

            /* Signal that data is available */
            if (rx_count > 0) {
                k_sem_give(&rx_sem);
            }
        }
    }

    /* Add newline for AT command mode if no more data */
    if (current_mode == QAT_Transfer_Mode_AT_COMMAND_E) {
        k_mutex_lock(&rx_mutex, K_FOREVER);
        if (rx_count < QAT_RX_BUFFER_SIZE) {
            rx_buffer[rx_tail] = '\n';
            rx_tail = (rx_tail + 1) % QAT_RX_BUFFER_SIZE;
            rx_count++;
        }
        k_mutex_unlock(&rx_mutex);
        if (!qat_is_service_timer_running()) {
            qat_start_service_timer();
            LOG_DBG("Timer started for AT command processing");
        }
    }
}

/**
 * libcat read interface - read one character from ring buffer
 */
static int qat_read_char(char *ch)
{
    k_mutex_lock(&rx_mutex, K_FOREVER);

    if (rx_count == 0) {
        k_mutex_unlock(&rx_mutex);
        if (qat_is_service_timer_running()) {
            qat_stop_service_timer();
            LOG_DBG("Timer stoped for AT command processing");
        }
        return 0; /* No data available */
    }

    *ch = rx_buffer[rx_head];
    rx_head = (rx_head + 1) % QAT_RX_BUFFER_SIZE;
    rx_count--;

    k_mutex_unlock(&rx_mutex);

    return 1; /* Successfully read one character */
}

/**
 * Flush TX buffer - send accumulated data via Ring Service
 */
static int qat_flush_tx_buffer(void)
{
    int ret;

    if (tx_count == 0) {
        return 0;
    }

    ret = ring_send(QAT_RING_ID, tx_buffer, tx_count, K_MSEC(QAT_RING_SEND_TIMEOUT));
    if (ret < 0) {
        LOG_ERR("Failed to send %zu bytes: %d", tx_count, ret);
        tx_count = 0; /* Reset buffer even on error */
        return ret;
    }

    LOG_DBG("TX: sent %zu bytes", tx_count);
    tx_count = 0;

    /* Add small delay to allow host to process data and prevent ring buffer overflow
     * This is especially important when sending multiple responses quickly (e.g., AT+CMD?)
     * The delay gives the host time to read from the ring before we send the next packet */
    k_usleep(50);

    return ret;
}
static bool tx_has_content = false; /* Track if buffer has actual content*/

static int qat_write_char(char ch)
{
    int ret = 1;

    k_mutex_lock(&tx_mutex, K_FOREVER);

    /* Add character to TX buffer */
    if (tx_count < QAT_TX_BUFFER_SIZE) {
        tx_buffer[tx_count++] = ch;

        /* Mark that we have actual content (not just line endings) */
        if (ch != '\r' && ch != '\n') {
            tx_has_content = true;
        }
    } else {
        LOG_ERR("TX buffer overflow");
        tx_has_content = false;
        tx_count = 0;
        ret = 0;
        goto unlock;
    }

    /* Flush on newline ONLY if we have actual content
     * This prevents splitting "\r\nOK\r\n" into two packets:
     * - Without this check: packet1="\r\n", packet2="OK\r\n"
     * - With this check: packet1="\r\nOK\r\n" (single packet)
     */
    if (ch == '\n' && tx_has_content) {
        ret = qat_flush_tx_buffer();
        tx_has_content = false; /* Reset flag after flush */
        if (ret < 0) {
            ret = 0; /* libcat expects 0 on error, 1 on success */
        } else {
            ret = 1;
        }
    }

unlock:
    k_mutex_unlock(&tx_mutex);
    return ret;
}

/* libcat I/O interface */
static struct cat_io_interface qat_io = {.read = qat_read_char, .write = qat_write_char};

/**
 * Set QAT transfer mode
 */
int QAT_Transfer_Mode_set(QAT_Transfer_Mode_t mode, qat_passthrough_callback_t callback)
{
    k_mutex_lock(&mode_mutex, K_FOREVER);

    if (mode == QAT_Transfer_Mode_ONLINE_DATA_E) {
        if (callback == NULL) {
            k_mutex_unlock(&mode_mutex);
            return -EINVAL;
        }
        transfer_state.mode = QAT_Transfer_Mode_ONLINE_DATA_E;
        transfer_state.callback = callback;
        LOG_DBG("Switched to ONLINE_DATA mode");
    } else {
        transfer_state.mode = QAT_Transfer_Mode_AT_COMMAND_E;
        transfer_state.callback = NULL;
        LOG_DBG("Switched to AT_COMMAND mode");
    }

    k_mutex_unlock(&mode_mutex);

    return 0;
}

/**
 * Get current QAT transfer mode
 */
QAT_Transfer_Mode_t QAT_Transfer_Mode_get(void)
{
    QAT_Transfer_Mode_t mode;

    k_mutex_lock(&mode_mutex, K_FOREVER);
    mode = transfer_state.mode;
    k_mutex_unlock(&mode_mutex);

    return mode;
}

/**
 * Initialize QAT I/O adapter
 */
int qat_io_init(void)
{
    int ret;

    LOG_INF("Initializing QAT I/O adapter");

    /* Register Ring Service callback */
    ret = ring_register_callback(qat_rx_callback, NULL);
    if (ret != 0) {
        LOG_ERR("Failed to register ring callback: %d", ret);
        return ret;
    }

    LOG_INF("QAT I/O adapter initialized successfully");
    return 0;
}

/**
 * Get I/O interface for libcat
 */
struct cat_io_interface *qat_get_io_interface(void) { return &qat_io; }
