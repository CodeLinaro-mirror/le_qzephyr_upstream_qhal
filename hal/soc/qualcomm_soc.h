/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
*/


#ifndef QUALCOMM_SOC_H
#define QUALCOMM_SOC_H

#if CONFIG_SOC_SERIES_QCC730
    //version TAPEOUT_02
    #include "qcc730v2.h"
    #include "qcc730v2_posmask.h"
#else
    #error "SOC must be defined. See qualcomm_soc.h."
#endif

#endif //QUALCOMM_SOC_H
