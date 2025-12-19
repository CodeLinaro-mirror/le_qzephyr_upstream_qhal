/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "nt_common.h"
#include <stdint.h>
#include "nt_osal.h"
#include <zephyr/autoconf.h>
#include <zephyr/kernel.h>

/**
 * <!-- nt_delay -->
 *
 * @brief Normal delay function
 * @param time: Delay time needed in milli seconds
 * @return: void
 */
void nt_normal_delay(uint32_t time)
{
    volatile int32_t i, j, value;
    value = (volatile int32_t)(time * (10 ^ 6) * 3);
    for (i = value; i >= 0; i--) {
        for (j = 100; j >= 0; j--) {
            __asm volatile("nop");
        }
    }
}

int tickless_idle_enabled(void)
{
#ifdef CONFIG_PM
    return 1;
#else
    return 0;
#endif
}

// NT_LOG_LVL_WARN
uint8_t min_loglvl = 1;

uint8_t nt_log_write(
    /*!@module id like SME,MLME,HAL.etc...*/
    uint8_t mod_id,
    /*!@ loglevel like info,warning.etc...*/
    uint8_t loglvl,
    /*@ for file name*/
    char *fn,
    /*@ for line number*/
    uint16_t ln,
    /*!@ data */
    const char *msg,
    /* user provided parameters */
    uint32_t p1, uint32_t p2, uint32_t p3

)
{
    (void)mod_id;

#if 1

    if (loglvl < min_loglvl) {
        return NT_ECANCELED;
    }

    printk("[%s %d]: %s [%d] [%d] [%d]\r\n", fn, ln, msg, p1, p2, p3);
#endif
    return NT_OK;
}

#define MSGBUF_LEN 200
char my1pbuf[MSGBUF_LEN];

uint8_t nt_log_printf(uint8_t mod_id, uint8_t loglvl, char *func_name,
                      /*@ for line number*/
                      uint16_t ln, const char *fmt, uint8_t num, ...)
{
#if 1
    va_list argp;
    (void)mod_id;

    if (loglvl < min_loglvl) {
        return NT_ECANCELED;
    }

    memset(my1pbuf, 0, MSGBUF_LEN);
    va_start(argp, num);
    vsnprintf(my1pbuf, sizeof(my1pbuf), fmt, argp);
    va_end(argp);
#if CONFIG_FTM_MODE
    printk("[%s %d]: %s\r\n", func_name, ln, my1pbuf);
#else
    printk("[%s %d]: ", func_name, ln);
    uart_hal_poll_out_str_ext(my1pbuf);
    uart_hal_poll_out_str_ext("\r\n");
#endif
#endif
    return NT_OK;
}

uint8_t nt_log_array_printf(uint8_t mod_id, uint8_t loglvl, char *func_name,
                            /*@ for line number*/
                            uint16_t ln, const char *s, const uint8_t *ptr, const uint16_t len)
{
    (void)mod_id;
    (void)loglvl;
    (void)func_name;
    (void)ln;
    (void)s;
    (void)ptr;
    (void)len;
    return NT_OK;
}

void *nt_osal_allocate_memory(size_t size)
{
    return k_malloc(size);
}

void nt_osal_free_memory(void *ptr)
{
    k_free(ptr);
}

void *nt_osal_calloc(size_t nmemb, size_t size)
{
    return k_calloc(nmemb, size);
}

const char *nt_osal_get_current_task_name()
{
    return k_thread_name_get(k_current_get());
}

void taskENTER_CRITICAL()
{
    k_sched_lock();
}

void taskEXIT_CRITICAL()
{
    k_sched_unlock();
}
