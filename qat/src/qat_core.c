 /*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <errno.h>
#include <cat.h>
#include "qat_api.h"
#include "ring_service.h"

LOG_MODULE_REGISTER(qat_core, LOG_LEVEL_INF);

/* Configuration macros */
#define QAT_WORK_BUF_SIZE QAT_RESPONSE_BUF_SIZE /* libcat working buffer size */
#define MAX_CMD_GROUPS 8                        /* Maximum number of command groups */
#define QAT_SERVICE_STACK_SIZE CONFIG_QAT_SERVICE_STACK_SIZE
#define QAT_SERVICE_PRIORITY 7                  /* Service thread priority */
#define QAT_SERVICE_POLL_MS 1                   /* Service polling interval in milliseconds */

/* External functions */
extern int qat_io_init(void);
extern struct cat_io_interface *qat_get_io_interface(void);

/* Working buffer for libcat */
static uint8_t qat_work_buf[QAT_WORK_BUF_SIZE];

/* libcat descriptor - support up to MAX_CMD_GROUPS command groups */
static struct cat_command_group *cmd_group_ptrs[MAX_CMD_GROUPS];
static uint8_t cmd_group_count = 0;
static K_MUTEX_DEFINE(cmd_group_mutex);

static struct cat_descriptor qat_desc = {.cmd_group = cmd_group_ptrs,
                                         .cmd_group_num = 0, /* Will be set dynamically */
                                         .buf = qat_work_buf,
                                         .buf_size = sizeof(qat_work_buf)};

/**
 * Register a command group with QAT
 * This function is called by command group modules during initialization
 */
int qat_register_cmd_group(struct cat_command_group *(*get_cmd_group)(void), const char *name)
{
    struct cat_command_group *group;
    int ret = 0;

    k_mutex_lock(&cmd_group_mutex, K_FOREVER);

    if (cmd_group_count >= MAX_CMD_GROUPS) {
        LOG_ERR("Maximum command groups (%d) reached, cannot register: %s", MAX_CMD_GROUPS, name);
        ret = -ENOMEM;
        goto out;
    }

    group = get_cmd_group();
    if (group == NULL) {
        LOG_ERR("Failed to get command group: %s", name);
        ret = -EINVAL;
        goto out;
    }

    cmd_group_ptrs[cmd_group_count] = group;
    cmd_group_count++;
    LOG_INF("Registered command group: %s (total: %d)", name, cmd_group_count);

out:
    k_mutex_unlock(&cmd_group_mutex);
    return ret;
}

/**
 * Get the list of registered command groups
 */
int qat_get_cmd_groups(struct cat_command_group ***groups, uint8_t *count)
{
    if (groups == NULL || count == NULL) {
        return -EINVAL;
    }

    k_mutex_lock(&cmd_group_mutex, K_FOREVER);
    *groups = cmd_group_ptrs;
    *count = cmd_group_count;
    k_mutex_unlock(&cmd_group_mutex);

    return 0;
}

/* libcat object */
static struct cat_object qat_cat;

struct cat_object *qat_get_cat_object(void)
{
    return &qat_cat;
}

/**
 * Output data to the AT command interface
 * This function writes data directly to the ring service
 *
 * @param Length Number of bytes to write
 * @param Buffer Pointer to the data buffer
 * @return 0 on success, negative error code on failure
 */
int QAT_Output(uint32_t Length, const char *Buffer)
{
    int ret;
    int retries = 1500; /* up to 1500 × 20 ms = 30 s total wait */

    if (Buffer == NULL || Length == 0) {
        return -EINVAL;
    }

    do {
        ret = ring_send(QAT_RING_ID, (const uint8_t *)Buffer, Length, K_MSEC(100));
        if (ret == 0) {
            break;
        }
        if (ret != -EAGAIN) {
            LOG_ERR("Failed to send %u bytes via ring: %d", Length, ret);
            return -EIO;
        }
        /* Ring full — nudge the host to drain, then wait for a descriptor to free */
        ring_notify_host(QAT_RING_ID);
        k_sleep(K_MSEC(20));
    } while (--retries > 0);

    if (ret < 0) {
        LOG_ERR("Failed to send %u bytes via ring after retries: %d", Length, ret);
        return -EIO;
    }

    LOG_DBG("Sent %u bytes via ring", Length);

    if (!k_is_in_isr())
        k_usleep(10);

    return 0;
}

/**
 * Send AT response data and return the corresponding cat_return_state.
 *
 * Sends only the buffer (with \r\n framing). The result string (OK/ERROR)
 * is NOT sent here — libcat handles it based on the returned cat_return_state.
 *
 * This function is designed to be used as the direct return value of a
 * libcat command callback:
 *
 *   return QAT_Response_Str(QAT_RC_OK, "+MYRESP: 1");
 *   // → sends "\r\n+MYRESP: 1\r\n", libcat appends "\r\nOK\r\n"
 *
 * For unsolicited use (outside libcat callbacks), use QAT_RC_QUIET:
 *   QAT_Response_Str(QAT_RC_QUIET, "+EVENT: data");
 *   // → sends "\r\n+EVENT: data\r\n", return value ignored
 */
cat_return_state QAT_Response_Str(QAT_Result_Enum_Type ret_code, const char *buffer)
{
    /* Response buffer sized to fit within QAT_TX_BUFFER_SIZE */
    char resp[QAT_RESPONSE_BUF_SIZE];
    int pos = 0;
    int buf_len = 0;

    if (ret_code >= QAT_RC_MAX) {
        return CAT_RETURN_STATE_ERROR;
    }

    if (buffer != NULL) {
        buf_len = strlen(buffer);
    }

    if (buf_len > 0) {
        if (ret_code == QAT_RC_QUIET_NO_CR) {
            /* Raw mode: send buffer with no \r\n framing. */
            if (buf_len < QAT_RESPONSE_BUF_SIZE) {
                memcpy(resp, buffer, buf_len);
                pos = buf_len;
            }
        } else {
            /* Standard framing: \r\n<buffer>\r\n */
            if ((2 + buf_len) < QAT_RESPONSE_BUF_SIZE) {
                resp[pos++] = '\r';
                resp[pos++] = '\n';
                memcpy(resp + pos, buffer, buf_len);
                pos += buf_len;
                // resp[pos++] = '\r';
                // resp[pos++] = '\n';
            }
        }

        if (pos > 0) {
            QAT_Output(pos, resp);
        }
    }

    /* Map ret_code to cat_return_state.
     * Error-class codes → ERROR; everything else → OK.
     * For QUIET modes the return value is ignored by the caller. */
    switch (ret_code) {
    case QAT_RC_ERROR:
    case QAT_RC_NO_CARRIER:
    case QAT_RC_NO_DIALTONE:
    case QAT_RC_BUSY:
    case QAT_RC_NO_ANSWER:
        return CAT_RETURN_STATE_ERROR;
    default:
        return CAT_RETURN_STATE_OK;
    }
}

/* Semaphore for timer-driven service execution */
K_SEM_DEFINE(qat_service_sem, 0, 1);

/**
 * QAT service timer callback - signals the service thread
 * This PM-aware timer allows the system to sleep while maintaining periodic service execution
 */
static void qat_service_timer_handler(struct k_timer *timer)
{
    ARG_UNUSED(timer);

    /* Signal the service thread to run cat_service() */
    k_sem_give(&qat_service_sem);
}
K_TIMER_DEFINE(qat_service_timer, qat_service_timer_handler, NULL);

/* Timer control functions */
int qat_start_service_timer(void)
{
    k_timer_start(&qat_service_timer, K_MSEC(QAT_SERVICE_POLL_MS), K_MSEC(QAT_SERVICE_POLL_MS));

    LOG_DBG("QAT service timer started");
    return 0;
}

int qat_stop_service_timer(void)
{
    k_timer_stop(&qat_service_timer);

    LOG_DBG("QAT service timer stopped");
    return 0;
}

bool qat_is_service_timer_running(void) { return k_timer_remaining_get(&qat_service_timer) > 0; }

/**
 * QAT service thread - waits for timer signal and calls cat_service()
 * This approach allows the system to enter sleep mode between timer events
 */
static void qat_service_thread(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    k_thread_name_set(k_current_get(), "qat_service");
    LOG_INF("QAT service thread started");

    while (1) {
        /* Wait for timer signal - thread sleeps here, allowing system to sleep */
        k_sem_take(&qat_service_sem, K_FOREVER);

        /* Execute cat_service when signaled */
        cat_service(&qat_cat);
    }
}

/* Define QAT service thread */
K_THREAD_DEFINE(qat_service_tid, QAT_SERVICE_STACK_SIZE, qat_service_thread, NULL, NULL, NULL, QAT_SERVICE_PRIORITY, 0,
                0);

/**
 * Initialize QAT module
 */
static int qat_init(void)
{
    int ret;

    LOG_INF("Initializing QAT module");

    /* Initialize QAT I/O adapter */
    ret = qat_io_init();
    if (ret != 0) {
        LOG_ERR("Failed to initialize QAT I/O: %d", ret);
        return ret;
    }

    /* Command groups are registered via SYS_INIT before this function runs */
    /* Update command group count */
    qat_desc.cmd_group_num = cmd_group_count;

    if (cmd_group_count == 0) {
        LOG_WRN("No command groups registered!");
    } else {
        LOG_INF("Total command groups registered: %d", cmd_group_count);
    }

    /* Initialize libcat */
    cat_init(&qat_cat, &qat_desc, qat_get_io_interface(), NULL);

    LOG_INF("QAT module initialized successfully");
    LOG_INF("Ready to process AT commands via Ring ID 0");

    return 0;
}

/* Initialize QAT at application level */
SYS_INIT(qat_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
