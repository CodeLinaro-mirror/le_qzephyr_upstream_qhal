/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

/* ----------------------------- Hardware Packet Memory ------------------------------------- */
/* Warning : Linker script reserves the memory for HW Packet Memory.
* The address is configured in hal_int_mmap.h file. */

SECTION_PROLOGUE(__sect_hw_pktmem, (NOLOAD), ALIGN(0x80))
{
	_ln_RAM_start_addr_hw_pktmem__ = ABSOLUTE(.);
	. += 32768;
	_ln_RAM_end_addr_hw_pktmem__ = ABSOLUTE(.);
} GROUP_LINK_IN(RAMABLE_REGION)

/* --------------------------- Hardware Descriptors/Data structures --------------------------- */
/* Warning : Linker script reserves the memory for HW Descriptors.
* The address is configured in hal_int_mmap.h file. */

/* DPU descriptors + keys + mickeys + replay counters + TPE descriptors + stats + RPE descriptors
 * + BTQM + SW templates + CTS to self SW templates + PS poll SW templates + QOS NULL SW templates
 * + Data NULL SW templates + TEMP SW templates + Beacon template + RXP address table + RRI region
 */

SECTION_PROLOGUE(__sect_hw_desc, (NOLOAD), ALIGN(4))
{
	_ln_RAM_start_addr_hw_desc__ = ABSOLUTE(.);
	. += 22504;
	_ln_RAM_end_addr_hw_desc__ = ABSOLUTE(.);
} GROUP_LINK_IN(RAMABLE_REGION)
