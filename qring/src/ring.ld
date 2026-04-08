/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

SECTION_PROLOGUE(.ring_ctrl, (NOLOAD),)
{
    . = ALIGN(4);
    __ring_ctrl_start = .;
    KEEP(*(.ring_ctrl))
    . = ALIGN(4);
    __ring_ctrl_end = .;
} GROUP_LINK_IN(RING_CTRL_RAM)

