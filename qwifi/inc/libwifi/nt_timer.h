/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SME_MLME_INC_NT_TIMER_H_
#define SME_MLME_INC_NT_TIMER_H_

/*Include Files*/
#include "nt_osal.h"

/*Timer Configuration*/ /*Move this configuration to sme header file*/
#define NT_SCAN_BEACON_WAIT_TIME		1000 // need to mention unit like milli sec or mili sec
#define NT_AUTH_RESPONSE_WAIT_TIME		1000
#define NT_ASSO_RESPONSE_WAIT_TIME		1000

#define NT_BEACON_TIMER_ID				0x01
#define NT_AUTHENTICATION_TIMER_ID		0x02
#define NT_ASSOCIATION_TIMER_ID		0x03

#define TIME_IS_GREATER_U64(a, b)       ((int64_t)((uint64_t)a - (uint64_t)b) > 0)
#define TIME_IS_GREATER_EQ_U64(a, b)       ((int64_t)((uint64_t)a - (uint64_t)b) >= 0)
#define TIME_IS_SMALLER_U64(a, b)          ((int64_t)((uint64_t)a - (uint64_t)b) < 0)
#define TIME_IS_SMALLER_EQ_U64(a, b)       ((int64_t)((uint64_t)a - (uint64_t)b) <= 0)
#define TIME_DIFF_U64(a, b)                ((uint64_t)((int64_t)(a) - (int64_t)(b)))
#define TIME_DIFF_WITH_WRAP_U64(a, b)  (TIME_IS_GREATER_U64(a,b)? \
                                                      TIME_DIFF_U64(a, b): \
                                                  (0xFFFFFFFFFFFFFFFF - (b) + (a) + 1))



#define TIME_IS_GREATER(a, b)          ((int32_t)((uint32_t)a - (uint32_t)b) > 0)
#define TIME_IS_GREATER_EQ(a, b)       ((int32_t)((uint32_t)a - (uint32_t)b) >= 0)
#define TIME_IS_SMALLER(a, b)          ((int32_t)((uint32_t)a - (uint32_t)b) < 0)
#define TIME_IS_SMALLER_EQ(a, b)       ((int32_t)((uint32_t)a - (uint32_t)b) <= 0)
#define TIME_DIFF(a, b)                ((uint32_t)((int32_t)(a) - (int32_t)(b)))
#define TIME_DIFF_WITH_WRAP_U32(a, b)  (TIME_IS_GREATER(a,b)? \
			               TIME_DIFF(a, b): \
				       (0xFFFFFFFF - (b) + (a) + 1))


/*Timer Function Declaration*/
int nt_delete_timer(TimerHandle_t timer_handle);
TickType_t nt_timer_get_tick_count();

//#if (defined NT_RCLI)
TickType_t nt_timer_get_expiry_time(TimerHandle_t timer_handle) ;
//#endif	// NT_RCLI

#endif /* SME_MLME_INC_NT_TIMER_H_ */
