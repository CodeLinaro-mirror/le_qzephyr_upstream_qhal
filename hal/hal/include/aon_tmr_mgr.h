/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __DRV_AON_TMR_H__
#define __DRV_AON_TMR_H__

#include <stdint.h>
#include <stdbool.h>


/*============================================================================
 * Type and Enum Definitions
 *==========================================================================*/

/**
 * @brief Function pointer to get the underlying, monotonically increasing absolute time in microseconds.
 * 
 * @details point to a hardware counter read function.
 * @return The current absolute time in microseconds as a 64-bit integer.
 */
typedef uint64_t (*get_abs_us_fn)(void);

/**
 * @brief Callback function type for an expired timer.
 * @param user_data A pointer to user-defined context provided during registration.
 */
typedef uint64_t (*aon_expiry_fn)(void *user_data);

/**
 * @brief Identifiers for timer clients. Used as an index into the aon_clients array.
 */
typedef enum {
    AON_CLIENT_OS = 0,
    AON_CLIENT_BMPS_IN_SLEEP_MODE,
    AON_CLIENT_BMPS_IN_ACTIVE_MODE,
    /*Add other clients like AON_CLIENT_BT here...*/
    AON_CLIENT_MAX,
    AON_CLIENT_NONE = 0xFF  // Special value to indicate no client
} aon_client_id_t;

/**
 * @brief Control block for a single AON timer client.
 */
typedef struct {
    /** The absolute expiry timestamp in microseconds when this timer should fire. */
    volatile uint64_t expiry_abs_us;
    /** Flag indicating if the timer is active. */
    volatile bool enabled;
    /** Callback function to execute on expiry. */
    aon_expiry_fn callback;
    /** User-defined context for the callback. */
    void *user_data;
} aon_timer_t;

/**
 * @brief Structure to return both sleep duration and client ID.
 */
typedef struct {
    /** The sleep duration in microseconds. */
    uint64_t sleep_us;
    /** The ID of the client with the earliest expiry time. */
    aon_client_id_t client_id;
} aon_sleep_info_t;

void aon_manager_init(get_abs_us_fn us_provider);
void aon_timer_register(aon_client_id_t id, aon_expiry_fn cb, void *data);
void aon_timer_deregister(aon_client_id_t id);
void aon_timer_set(aon_client_id_t id, uint64_t duration_us);
void aon_get_min_expiry(aon_sleep_info_t* info);
void aon_disable_all_timers(void);
bool aon_process_specific_client(aon_client_id_t client_id, uint64_t* slp_back);
#endif // __DRV_AON_TMR_H__