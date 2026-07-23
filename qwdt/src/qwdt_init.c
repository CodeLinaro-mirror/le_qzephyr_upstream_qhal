/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/init.h>
#include <zephyr/logging/log.h>
#include <qurt_timer.h>
#include "printfext.h"
#include <soc.h>

LOG_MODULE_REGISTER(qwdt, CONFIG_WDT_LOG_LEVEL);

/* Watchdog register definitions for direct hardware access */
#define PMU_BASE_ADDR                           DT_REG_ADDR(DT_NODELABEL(pmu))
#define QWLAN_PMU_AON_WDOG_CTL_REG             (PMU_BASE_ADDR + 0xCE0)
#define QWLAN_PMU_AON_WDOG_CTL_WDOG_RESET_MASK (1 << 0)

static const struct device *g_wdt_dev = NULL;
static int g_wdt_channel_id = -1;
static bool wdt_enabled = false;

TimerHandle_t wdt_timer_handle = NULL;

#define WDOG_TIMER_NAME  "wdt_feed"

#ifdef CONFIG_QWDT
static uint32_t bark_time;
static uint32_t bite_time;

/**
 * @brief Watchdog feed timer callback
 */
static void qwdt_timer_call_back(struct k_timer *timer)
{
	ARG_UNUSED(timer);

	if (wdt_enabled && g_wdt_dev && g_wdt_channel_id >= 0) {
		int ret = wdt_feed(g_wdt_dev, g_wdt_channel_id);
		if (ret < 0) {
			LOG_ERR("Failed to feed watchdog: %d", ret);
		} else {
			LOG_DBG("Watchdog fed automatically");
		}
	}
}
#endif

/**
 * @brief Initialize and start watchdog timer with auto-feed
 */
static int qwdt_init(void)
{
#ifndef CONFIG_QWDT
    LOG_INF("QWDT disabled by CONFIG_QWDT=n");
    g_wdt_dev = NULL;
    g_wdt_channel_id = -1;
    wdt_enabled = false;
    wdt_timer_handle = NULL;
    return 0;
#else
	int ret;
    PRINT_LOG_FUNC_LINE;

	bark_time = CONFIG_WATCH_DOG_BITE_TIME * 1000;
	bite_time = CONFIG_WATCH_DOG_BITE_TIME * 1000;

	/* Create PM timer for automatic suspend/resume during power save */
	wdt_timer_handle = nt_create_pm_timer(
		WDOG_TIMER_NAME,
		qwdt_timer_call_back,
		NULL,
		NT_MS_TO_TICKS((bark_time - 1000)),
		1  /* auto_reload */
	);

	if (!wdt_timer_handle) {
		LOG_ERR("Failed to create watchdog timer");
		return -ENOMEM;
	}

	g_wdt_dev = DEVICE_DT_GET(DT_NODELABEL(watchdog0));
	if (!device_is_ready(g_wdt_dev)) {
		LOG_ERR("Watchdog device not ready");
		return -ENODEV;
	}

	struct wdt_timeout_cfg wdt_config = {
		.flags = WDT_FLAG_RESET_SOC,
		.window.min = 0,
		.window.max = bite_time,
		.callback = NULL,
	};

	g_wdt_channel_id = wdt_install_timeout(g_wdt_dev, &wdt_config);
	if (g_wdt_channel_id < 0) {
		LOG_ERR("Failed to install timeout: %d", g_wdt_channel_id);
		return g_wdt_channel_id;
	}

	/* Start watchdog */
	ret = wdt_setup(g_wdt_dev, 0);
	if (ret < 0) {
		LOG_ERR("Failed to setup watchdog: %d", ret);
		return ret;
	}

	wdt_enabled = true;

	LOG_WRN("Watchdog configured: bite=%u ms, bark=%u ms", bite_time, bark_time);

	qurt_timer_start(wdt_timer_handle, (TickType_t)100);

	LOG_WRN("Watchdog auto-feed started: interval=%u ms", bark_time - 1000);

	return 0;
#endif
}

/**
 * @brief Manually feed the watchdog
 */
int qwdt_feed_now(void)
{
#ifndef CONFIG_QWDT
    return 0;
#endif
	if (!g_wdt_dev || g_wdt_channel_id < 0) {
		return -EINVAL;
	}

	return wdt_feed(g_wdt_dev, g_wdt_channel_id);
}

/**
 * @brief Feed watchdog by directly accessing registers
 */
int qwdt_feed_now_direct(void)
{
#ifndef CONFIG_QWDT
    return 0;
#else
    volatile uint32_t *wdog_ctl_reg = (volatile uint32_t *)QWLAN_PMU_AON_WDOG_CTL_REG;

    /* Toggle WDOG_RESET bit: set to 1 then clear to 0 */
    *wdog_ctl_reg |= QWLAN_PMU_AON_WDOG_CTL_WDOG_RESET_MASK;
    *wdog_ctl_reg &= ~QWLAN_PMU_AON_WDOG_CTL_WDOG_RESET_MASK;

    return 0;
#endif
}

SYS_INIT(qwdt_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
