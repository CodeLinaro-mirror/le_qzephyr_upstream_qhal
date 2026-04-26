/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 *
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/pm/device.h>
#include <stdlib.h>
#include <string.h>
#include "ring_service.h"

LOG_MODULE_REGISTER(ring_service, CONFIG_RING_SERVICE_LOG_LEVEL);

/* GPIO device for triggering Host interrupt */
static const struct device *gpio_dev;
/* Use GPIO 8 to trigger Host interrupt */
#define PIN_INT_TO_HOST 8

/* Control block in fixed memory (defined via linker script) */
__attribute__((section(".ring_ctrl"))) struct ring_control_block g_ring_ctrl_block;

/* Work queue for deferred processing */
static struct k_work_q ring_work_q;
static K_THREAD_STACK_DEFINE(ring_work_stack, CONFIG_RING_SERVICE_WORKQ_STACK_SIZE);

/* Work item context for each ring */
struct ring_work_context {
    struct k_work work;
    uint8_t ring_id;
};

/* Work items for RX event processing (one per ring) */
static struct ring_work_context rx_works[MAX_RINGS];

/* Per-ring instance data */
struct ring_instance {
    struct k_mutex tx_lock;
    struct k_mutex rx_lock;
    struct k_sem tx_sem;
    struct k_sem rx_sem;
    struct ring_stats stats;
};

/* Global ring service state */
static struct {
    bool initialized;
    uint32_t num_rings;
    struct ring_instance rings[MAX_RINGS];
    ring_event_callback_t callback;
    void *callback_data;
} g_ring_service;

/* Configure 3 rings: Ring 0 (AT Commands), Ring 1 (Data), Ring 2 (Loopback) */
struct ring_config configs[] = {
    {
        .ring_id = RING_0,
        .desc_count = CONFIG_RING0_DESC_COUNT,
        .buf_size = CONFIG_RING0_BUF_SIZE,
    },
    {
        .ring_id = RING_1,
        .desc_count = CONFIG_RING1_DESC_COUNT,
        .buf_size = CONFIG_RING1_BUF_SIZE,
    },
    {
        .ring_id = RING_2,
        .desc_count = CONFIG_RING2_DESC_COUNT,
        .buf_size = CONFIG_RING2_BUF_SIZE,
    },
};

/**
 * @brief Work handler for RX event processing (deferred from interrupt context)
 */
static void rx_work_handler(struct k_work *work)
{
    struct ring_work_context *ctx = CONTAINER_OF(work, struct ring_work_context, work);

    int rx_avail = ring_get_rx_available(ctx->ring_id);
    if (rx_avail <= 0) {
        /* No data available for this ring, skip callback */
        LOG_DBG("rx_work_handler: ring %d has no data (avail=%d)", ctx->ring_id, rx_avail);
        return;
    }

    LOG_DBG("rx_work_handler: ring %d has %d descriptors available", ctx->ring_id, rx_avail);

    /* Call the registered callback in thread context with ring_id */
    if (g_ring_service.callback) {
        g_ring_service.callback(ctx->ring_id, g_ring_service.callback_data);
    }
}

/**
 * @brief Helper function to free ring resources
 */
static void free_ring_resources(struct ring_control_block *ctrl, uint8_t ring_id)
{
    if (ctrl->tx_desc_base[ring_id]) {
        free((void *)ctrl->tx_desc_base[ring_id]);
        ctrl->tx_desc_base[ring_id] = 0;
    }
    if (ctrl->tx_buf_base[ring_id]) {
        free((void *)ctrl->tx_buf_base[ring_id]);
        ctrl->tx_buf_base[ring_id] = 0;
    }
    if (ctrl->rx_desc_base[ring_id]) {
        free((void *)ctrl->rx_desc_base[ring_id]);
        ctrl->rx_desc_base[ring_id] = 0;
    }
    if (ctrl->rx_buf_base[ring_id]) {
        free((void *)ctrl->rx_buf_base[ring_id]);
        ctrl->rx_buf_base[ring_id] = 0;
    }
}

/**
 * @brief ring service initialization (supports multiple rings)
 */
int ring_service_slave_init(const struct ring_config *configs, uint32_t num_rings)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    int ret;

    if (g_ring_service.initialized) {
        LOG_WRN("Ring service already initialized");
        return -EALREADY;
    }

    /* Validate parameters */
    if (!configs || num_rings == 0 || num_rings > MAX_RINGS) {
        LOG_ERR("Invalid parameters: configs=%p, num_rings=%u", configs, num_rings);
        return -EINVAL;
    }

    LOG_INF("Initializing ring service: num_rings=%u", num_rings);

    /* Validate each ring configuration */
    for (uint32_t i = 0; i < num_rings; i++) {
        if (configs[i].ring_id >= MAX_RINGS) {
            LOG_ERR("Invalid ring_id: %u", configs[i].ring_id);
            return -EINVAL;
        }
        if (configs[i].desc_count == 0 || configs[i].desc_count > 256) {
            LOG_ERR("Invalid desc_count for ring %u: %u", configs[i].ring_id, configs[i].desc_count);
            return -EINVAL;
        }
        if (configs[i].buf_size == 0 || configs[i].buf_size > 1500) {
            LOG_ERR("Invalid buf_size for ring %u: %u", configs[i].ring_id, configs[i].buf_size);
            return -EINVAL;
        }
        LOG_INF("  Ring %u: desc_count=%u, buf_size=%u", configs[i].ring_id, configs[i].desc_count,
                configs[i].buf_size);
    }

    /* Initialize control block */
    memset(ctrl, 0, sizeof(struct ring_control_block));
    ctrl->magic = RING_MAGIC_NUMBER;
    ctrl->version = RING_VERSION;
    ctrl->num_rings = num_rings;
    ctrl->status = RING_STATUS_INITIALIZED;

    /* Allocate memory for each ring */
    for (uint32_t i = 0; i < num_rings; i++) {
        uint8_t ring_id = configs[i].ring_id;
        uint32_t desc_count = configs[i].desc_count;
        uint32_t buf_size = configs[i].buf_size;
        struct ring_descriptor *tx_desc = NULL;
        void *tx_buf_pool = NULL;
        struct ring_descriptor *rx_desc = NULL;
        void *rx_buf_pool = NULL;

        LOG_INF("Allocating memory for ring %u", ring_id);

        /* 1. Allocate TX ring descriptor array */
        tx_desc = malloc(desc_count * sizeof(struct ring_descriptor));
        if (!tx_desc) {
            LOG_ERR("Failed to allocate TX descriptors for ring %u", ring_id);
            ret = -ENOMEM;
            goto cleanup;
        }
        ctrl->tx_desc_base[ring_id] = (uint32_t)tx_desc;
        ctrl->tx_desc_count[ring_id] = desc_count;

        /* 2. Allocate TX ring buffer pool */
        tx_buf_pool = malloc(desc_count * buf_size);
        if (!tx_buf_pool) {
            LOG_ERR("Failed to allocate TX buffer pool for ring %u", ring_id);
            ret = -ENOMEM;
            goto cleanup;
        }
        ctrl->tx_buf_base[ring_id] = (uint32_t)tx_buf_pool;
        ctrl->tx_buf_size[ring_id] = buf_size;

        /* 3. Allocate RX ring descriptor array */
        rx_desc = malloc(desc_count * sizeof(struct ring_descriptor));
        if (!rx_desc) {
            LOG_ERR("Failed to allocate RX descriptors for ring %u", ring_id);
            ret = -ENOMEM;
            goto cleanup;
        }
        ctrl->rx_desc_base[ring_id] = (uint32_t)rx_desc;
        ctrl->rx_desc_count[ring_id] = desc_count;

        /* 4. Allocate RX ring buffer pool */
        rx_buf_pool = malloc(desc_count * buf_size);
        if (!rx_buf_pool) {
            LOG_ERR("Failed to allocate RX buffer pool for ring %u", ring_id);
            ret = -ENOMEM;
            goto cleanup;
        }
        ctrl->rx_buf_base[ring_id] = (uint32_t)rx_buf_pool;
        ctrl->rx_buf_size[ring_id] = buf_size;

        /* Initialize indices */
        ctrl->tx_wr_idx[ring_id] = 0;
        ctrl->tx_rd_idx[ring_id] = 0;
        ctrl->rx_wr_idx[ring_id] = 0;
        ctrl->rx_rd_idx[ring_id] = 0;

        /* Initialize all descriptors */
        for (uint32_t j = 0; j < desc_count; j++) {
            tx_desc[j].buffer_addr = ctrl->tx_buf_base[ring_id] + j * buf_size;
            tx_desc[j].length = 0;
            tx_desc[j].flags = 0;

            rx_desc[j].buffer_addr = ctrl->rx_buf_base[ring_id] + j * buf_size;
            rx_desc[j].length = 0;
            rx_desc[j].flags = 0;
        }

        ctrl->ring_status[ring_id] = RING_STATUS_VALID;

        LOG_INF("Ring %u memory allocated: TX desc=0x%08x, buf=0x%08x, RX desc=0x%08x, buf=0x%08x", ring_id,
                ctrl->tx_desc_base[ring_id], ctrl->tx_buf_base[ring_id], ctrl->rx_desc_base[ring_id],
                ctrl->rx_buf_base[ring_id]);
    }

    /* Initialize GPIO for interrupt to host */
    gpio_dev = DEVICE_DT_GET(DT_NODELABEL(gpioa));

    if (!device_is_ready(gpio_dev)) {
        LOG_ERR("GPIO device not ready");
        ret = -ENODEV;
        goto cleanup;
    }

    ret = gpio_pin_configure(gpio_dev, PIN_INT_TO_HOST, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        LOG_ERR("Failed to configure GPIO pin %d", ret);
        goto cleanup;
    }

    /* Initialize synchronization objects for each ring */
    for (uint32_t i = 0; i < num_rings; i++) {
        uint8_t ring_id = configs[i].ring_id;
        uint32_t desc_count = configs[i].desc_count;

        ret = k_mutex_init(&g_ring_service.rings[ring_id].tx_lock);
        if (ret < 0) {
            LOG_ERR("Failed to initialize TX mutex for ring %u: %d", ring_id, ret);
            goto cleanup;
        }

        ret = k_mutex_init(&g_ring_service.rings[ring_id].rx_lock);
        if (ret < 0) {
            LOG_ERR("Failed to initialize RX mutex for ring %u: %d", ring_id, ret);
            goto cleanup;
        }

        ret = k_sem_init(&g_ring_service.rings[ring_id].tx_sem, 0, desc_count);
        if (ret < 0) {
            LOG_ERR("Failed to initialize TX semaphore for ring %u: %d", ring_id, ret);
            goto cleanup;
        }

        ret = k_sem_init(&g_ring_service.rings[ring_id].rx_sem, 0, desc_count);
        if (ret < 0) {
            LOG_ERR("Failed to initialize RX semaphore for ring %u: %d", ring_id, ret);
            goto cleanup;
        }

        /* Initialize statistics */
        memset(&g_ring_service.rings[ring_id].stats, 0, sizeof(g_ring_service.rings[ring_id].stats));

        LOG_INF("Ring %u synchronization objects initialized", ring_id);
    }

    /* Initialize work queue for deferred processing */
    k_work_queue_init(&ring_work_q);
    k_work_queue_start(&ring_work_q, ring_work_stack, K_THREAD_STACK_SIZEOF(ring_work_stack), K_PRIO_COOP(7), NULL);
    k_thread_name_set(&ring_work_q.thread, "ring_work_q");

    /* Initialize work items for each ring */
    for (uint32_t i = 0; i < num_rings; i++) {
        uint8_t ring_id = configs[i].ring_id;
        rx_works[ring_id].ring_id = ring_id;
        k_work_init(&rx_works[ring_id].work, rx_work_handler);
    }

    LOG_INF("Work queue and work items initialized");

    g_ring_service.num_rings = num_rings;
    g_ring_service.initialized = true;

    LOG_INF("Ring service initialized successfully");
    LOG_INF("  Control block: 0x%08x", (uint32_t)ctrl);
    LOG_INF("  Number of rings: %u", num_rings);

    return 0;

cleanup:
    LOG_ERR("Initialization failed, cleaning up resources");

    /* Free allocated memory for all rings */
    for (uint32_t i = 0; i < num_rings; i++) {
        uint8_t ring_id = configs[i].ring_id;
        free_ring_resources(ctrl, ring_id);
    }

    /* Reset control block */
    memset(ctrl, 0, sizeof(struct ring_control_block));

    return ret;
}

/**
 * @brief Deinitialize ring service
 */
void ring_service_deinit(void)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;

    if (!g_ring_service.initialized) {
        return;
    }

    /* Free allocated memory only for rings that were initialized */
    for (uint32_t ring_id = 0; ring_id < MAX_RINGS; ring_id++) {
        if (ctrl->ring_status[ring_id] == RING_STATUS_VALID) {
            free_ring_resources(ctrl, ring_id);
        }
    }

    /* Clear control block */
    memset(ctrl, 0, sizeof(*ctrl));

    g_ring_service.initialized = false;

    LOG_INF("Ring service deinitialized");
}

/**
 * @brief Register event callback
 */
int ring_register_callback(ring_event_callback_t callback, void *user_data)
{
    if (!g_ring_service.initialized) {
        return -ENODEV;
    }

    g_ring_service.callback = callback;
    g_ring_service.callback_data = user_data;

    return 0;
}

/**
 * @brief Get ring statistics
 */
int ring_get_stats(uint8_t ring_id, struct ring_stats *stats)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    if (!g_ring_service.initialized) {
        return -ENODEV;
    }

    if (ring_id >= MAX_RINGS) {
        LOG_ERR("Invalid ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (ctrl->ring_status[ring_id] != RING_STATUS_VALID) {
        LOG_ERR("Unconfigure ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (!stats) {
        return -EINVAL;
    }

    memcpy(stats, &g_ring_service.rings[ring_id].stats, sizeof(*stats));

    return 0;
}

/**
 * @brief Get available ring count
 */
int ring_get_num(void) { return g_ring_service.num_rings; }

/**
 * @brief Get available TX descriptors count
 */
int ring_get_tx_available(uint8_t ring_id)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    uint32_t wr_idx, rd_idx;

    if (!g_ring_service.initialized) {
        return -ENODEV;
    }

    if (ring_id >= MAX_RINGS) {
        LOG_ERR("Invalid ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (ctrl->ring_status[ring_id] != RING_STATUS_VALID) {
        LOG_ERR("Unconfigure ring_id: %u", ring_id);
        return -EINVAL;
    }

    /* For Slave, TX is actually RX ring (Slave→Host) */
    wr_idx = ctrl->rx_wr_idx[ring_id];
    rd_idx = ctrl->rx_rd_idx[ring_id];

    /* Calculate available space */
    if (rd_idx > wr_idx) {
        return rd_idx - wr_idx - 1;
    } else {
        return ctrl->rx_desc_count[ring_id] - (wr_idx - rd_idx) - 1;
    }
}

/**
 * @brief Get available RX descriptors count
 */
int ring_get_rx_available(uint8_t ring_id)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    uint32_t wr_idx, rd_idx;

    if (!g_ring_service.initialized) {
        return -ENODEV;
    }

    if (ring_id >= MAX_RINGS) {
        LOG_ERR("Invalid ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (ctrl->ring_status[ring_id] != RING_STATUS_VALID) {
        LOG_ERR("Unconfigure ring_id: %u", ring_id);
        return -EINVAL;
    }

    /* For Slave, RX is actually TX ring (Host→Slave) */
    wr_idx = ctrl->tx_wr_idx[ring_id];
    rd_idx = ctrl->tx_rd_idx[ring_id];

    /* Calculate available data */
    if (wr_idx >= rd_idx) {
        return wr_idx - rd_idx;
    } else {
        return ctrl->tx_desc_count[ring_id] - (rd_idx - wr_idx);
    }
}

/**
 * @brief Send data through ring buffer (Slave→Host)
 *
 * For Slave, sending means writing to RX ring (from Host's perspective)
 */
int ring_send(uint8_t ring_id, const uint8_t *data, size_t len, k_timeout_t timeout)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    struct ring_descriptor *rx_desc;
    uint32_t wr_idx, rd_idx, next_wr_idx;
    uint32_t avail;
    uint8_t *buf_addr;
    int ret;

    if (!g_ring_service.initialized) {
        LOG_ERR("Ring service not initialized");
        return -ENODEV;
    }

    if (ring_id >= MAX_RINGS) {
        LOG_ERR("Invalid ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (ctrl->ring_status[ring_id] != RING_STATUS_VALID) {
        LOG_ERR("Unconfigure ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (!data || len == 0) {
        LOG_ERR("Invalid parameters");
        return -EINVAL;
    }

    if (len > ctrl->rx_buf_size[ring_id]) {
        LOG_ERR("Data too large: %zu > %u", len, ctrl->rx_buf_size[ring_id]);
        return -EMSGSIZE;
    }

    /* Lock TX (for Slave, TX is RX ring) */
    ret = k_mutex_lock(&g_ring_service.rings[ring_id].tx_lock, timeout);
    if (ret < 0) {
        LOG_ERR("Failed to lock TX mutex: %d", ret);
        return ret;
    }

    /* Read current indices */
    wr_idx = ctrl->rx_wr_idx[ring_id];
    rd_idx = ctrl->rx_rd_idx[ring_id];

    /* Calculate available space */
    if (rd_idx > wr_idx) {
        avail = rd_idx - wr_idx - 1;
    } else {
        avail = ctrl->rx_desc_count[ring_id] - (wr_idx - rd_idx) - 1;
    }

    if (avail == 0) {
        k_mutex_unlock(&g_ring_service.rings[ring_id].tx_lock);
        return -EAGAIN; /* No space available */
    }

    /* Get descriptor */
    rx_desc = (struct ring_descriptor *)ctrl->rx_desc_base[ring_id];
    rx_desc = &rx_desc[wr_idx];

    /* Get buffer address */
    buf_addr = (uint8_t *)rx_desc->buffer_addr;

    /* Copy data to buffer (local memory access) */
    memcpy(buf_addr, data, len);

    /* Update descriptor */
    rx_desc->length = len;
    rx_desc->flags = RING_DESC_FLAG_VALID;

    /* Update write index */
    next_wr_idx = (wr_idx + 1) % ctrl->rx_desc_count[ring_id];
    ctrl->rx_wr_idx[ring_id] = next_wr_idx;

    /* Update statistics */
    g_ring_service.rings[ring_id].stats.tx_count++;

    k_mutex_unlock(&g_ring_service.rings[ring_id].tx_lock);
#ifdef CONFIG_SPI
    /* Prevent BMPS sleep until host reads the data */
    const struct device *spi_dev = DEVICE_DT_GET(DT_NODELABEL(qcspi));
    if (device_is_ready(spi_dev))
        pm_device_busy_set(spi_dev);
#endif

    ret = gpio_pin_set(gpio_dev, PIN_INT_TO_HOST, 0);
    if (ret < 0) {
        LOG_ERR("Failed to set GPIO low: %d", ret);
    }

    ret = gpio_pin_set(gpio_dev, PIN_INT_TO_HOST, 1);
    if (ret < 0) {
        LOG_ERR("Failed to set GPIO high: %d", ret);
    }

    return 0;
}

/**
 * @brief Receive data from ring buffer (Host→Slave)
 *
 * For Slave, receiving means reading from TX ring (from Host's perspective)
 */
int ring_recv(uint8_t ring_id, uint8_t *data, size_t max_len, k_timeout_t timeout)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    struct ring_descriptor *tx_desc;
    uint32_t wr_idx, rd_idx, next_rd_idx;
    uint32_t avail;
    uint8_t *buf_addr;
    uint16_t data_len;
    int ret;

    if (!g_ring_service.initialized) {
        LOG_ERR("Ring service not initialized");
        return -ENODEV;
    }

    if (ring_id >= MAX_RINGS) {
        LOG_ERR("Invalid ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (ctrl->ring_status[ring_id] != RING_STATUS_VALID) {
        LOG_ERR("Unconfigure ring_id: %u", ring_id);
        return -EINVAL;
    }

    if (!data || max_len == 0) {
        LOG_ERR("Invalid parameters");
        return -EINVAL;
    }

    /* Lock RX (for Slave, RX is TX ring) */
    ret = k_mutex_lock(&g_ring_service.rings[ring_id].rx_lock, timeout);
    if (ret < 0) {
        LOG_ERR("Failed to lock RX mutex: %d", ret);
        return ret;
    }

    /* Read current indices */
    wr_idx = ctrl->tx_wr_idx[ring_id];
    rd_idx = ctrl->tx_rd_idx[ring_id];

    /* Calculate available data */
    if (wr_idx >= rd_idx) {
        avail = wr_idx - rd_idx;
    } else {
        avail = ctrl->tx_desc_count[ring_id] - (rd_idx - wr_idx);
    }

    /* Check if data available */
    if (avail == 0) {
        if (K_TIMEOUT_EQ(timeout, K_NO_WAIT)) {
            k_mutex_unlock(&g_ring_service.rings[ring_id].rx_lock);
            return 0;
        }

        /* Wait for data */
        k_mutex_unlock(&g_ring_service.rings[ring_id].rx_lock);
        ret = k_sem_take(&g_ring_service.rings[ring_id].rx_sem, timeout);
        if (ret < 0) {
            LOG_WRN("Timeout waiting for RX data");
            return 0;
        }

        /* Re-acquire lock */
        ret = k_mutex_lock(&g_ring_service.rings[ring_id].rx_lock, K_FOREVER);
        if (ret < 0) {
            return ret;
        }

        /* Re-read indices */
        wr_idx = ctrl->tx_wr_idx[ring_id];
        rd_idx = ctrl->tx_rd_idx[ring_id];
    }

    /* Get descriptor */
    tx_desc = (struct ring_descriptor *)ctrl->tx_desc_base[ring_id];
    tx_desc = &tx_desc[rd_idx];

    /* Check descriptor validity */
    if (!(tx_desc->flags & RING_DESC_FLAG_VALID)) {
        LOG_ERR("Invalid descriptor at index %u", rd_idx);
        g_ring_service.rings[ring_id].stats.rx_errors++;
        k_mutex_unlock(&g_ring_service.rings[ring_id].rx_lock);
        return -EINVAL;
    }

    /* Get data length */
    data_len = tx_desc->length;
    if (data_len > max_len) {
        LOG_ERR("Buffer too small: %u > %zu", data_len, max_len);
        g_ring_service.rings[ring_id].stats.rx_errors++;
        k_mutex_unlock(&g_ring_service.rings[ring_id].rx_lock);
        return -EMSGSIZE;
    }

    /* Get buffer address */
    buf_addr = (uint8_t *)tx_desc->buffer_addr;

    /* Copy data from buffer (local memory access) */
    memcpy(data, buf_addr, data_len);

    /* Clear descriptor */
    tx_desc->flags = 0;
    tx_desc->length = 0;

    /* Update read index */
    next_rd_idx = (rd_idx + 1) % ctrl->tx_desc_count[ring_id];
    ctrl->tx_rd_idx[ring_id] = next_rd_idx;

    /* Update statistics */
    g_ring_service.rings[ring_id].stats.rx_count++;

    k_mutex_unlock(&g_ring_service.rings[ring_id].rx_lock);

    LOG_DBG("Received %u bytes (rd_idx: %u -> %u)", data_len, rd_idx, next_rd_idx);

    return data_len;
}

/**
 * @brief ring rx interrupt handler - called when Host sends data
 *
 * Polls all configuration rings and submits work items for rings with data
 */
void ring_rx_handler(void)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;
    if (!g_ring_service.initialized) {
        return;
    }

    /* Poll all configured rings by checking ring_status */
    for (uint32_t ring_id = 0; ring_id < MAX_RINGS; ring_id++) {
        if (ctrl->ring_status[ring_id] != RING_STATUS_VALID) {
            continue;
        }

        /* Signal semaphore for this ring */
        k_sem_give(&g_ring_service.rings[ring_id].rx_sem);

        /* Submit work item for this ring */
        if (g_ring_service.callback) {
            k_work_submit_to_queue(&ring_work_q, &rx_works[ring_id].work);
        }
    }
}

int init_qring(void)
{
    int ret;

    ret = ring_service_slave_init(configs, sizeof(configs) / sizeof(configs[0]));
    if (ret < 0) {
        printk("Failed to init ring service: %d\n", ret);
        return ret;
    }

    LOG_WRN("%s", __FUNCTION__);
    return 0;
}

SYS_INIT(init_qring, POST_KERNEL, 50);

/**
 * @brief Check if all TX data has been consumed by host
 *
 * Used by SPI driver to determine if pm_device_busy can be cleared.
 *
 * @return true if all ring TX data has been read by host, false otherwise
 */
bool ring_all_tx_consumed(void)
{
    struct ring_control_block *ctrl = &g_ring_ctrl_block;

    if (!g_ring_service.initialized) {
        return true;
    }

    for (int i = 0; i < g_ring_service.num_rings; i++) {
        if (ctrl->ring_status[i] != RING_STATUS_VALID) {
            continue;
        }
        if (ctrl->rx_wr_idx[i] != ctrl->rx_rd_idx[i]) {
            return false;
        }
    }
    return true;
}
