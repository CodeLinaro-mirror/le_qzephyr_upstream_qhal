/**
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "stddef.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#include "dxe.h"
#include "nt_hw.h"
#include "nt_osal.h"
// #include "data_path_sys.h"
#include "nt_logger_api.h"
// #include "data_path.h"
// #include "hal_int_interrupt.h"
// #include "hal_int_modules.h"
#ifdef NT_TST_TIME_STAMP_ENABLE
#include "hal_int_sys.h"
#endif
// #include "hal_int_powersave.h"
#include <assert.h>
#include <libwifi.h>
#include "nt_hw_support.h"
#include "qurt_isr.h"
#include "qcc730v2.h"

// #include "ferm_prof.h"

#define DXE_WAR_FOR_DATA_STALL 1
#define DXE_MAX_RETRY (10)

pHalDxe halDxe = NULL;
volatile uint32_t dxe_reg;

#ifdef DEBUG
volatile uint32_t g_dxe_suspend, g_dxe_resume, g_dxe_stop_time, g_suspend_in_progress;
#endif

extern uint8_t rx_amsdu_enabled;

#ifdef MEM_CPY_VIA_DXE
static volatile uint8_t copy_ongoing = 0;
void nt_dxe_cpy_done_handler(uint32_t tx_type)
{
    if (tx_type == H2H) {
        copy_ongoing = 0;
    } else {
        NT_LOG_DPM_ERR("Invalid TX Type in Interrupt Handler\r\n", 0, 0, 0);
    }
    return;
}

void *nt_dxe_memcpy(void *dst, const void *src, uint32_t length)
{
    eRet_t ret = NDXE_SUCCESS;

    while (copy_ongoing)
        ;

    taskENTER_CRITICAL();
    copy_ongoing = 1;
    taskEXIT_CRITICAL();

    ret = nt_ndxe_write_frame_to_transfer(H2H, src, length, dst);
    if (ret != NDXE_SUCCESS) {
        copy_ongoing = 0;
        return NULL;
    }

    while (copy_ongoing)
        ;

    return dst;
}

#endif /* MEM_CPY_VIA_DXE */

/* Start/Reset DXE */
void nt_ndxe_start(void)
{
    volatile uint32_t regVal;

    qurt_isr_register_3(DXE_qgic2_per_channel_int_0, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_1, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_2, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_3, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_4, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_5, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_6, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_7, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_8, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_9, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_10, nt_dxe_interrupt_handler);
    qurt_isr_register_3(DXE_qgic2_per_channel_int_11, nt_dxe_interrupt_handler);

    // Reset DXE : Not reset in pronto code <--  Check if required ?
    regVal = rRead(QWLAN_CCU_R_CCU_SOFT_RESET_REG);
    rWrite(QWLAN_CCU_R_CCU_SOFT_RESET_REG, regVal | QWLAN_CCU_R_CCU_SOFT_RESET_DXE_SOFT_RESET_MASK);

    // while (++i < 0xFFFF)
    regVal = rRead(QWLAN_CCU_R_CCU_SOFT_RESET_REG); // Add some delay

    rWrite(QWLAN_CCU_R_CCU_SOFT_RESET_REG, regVal & (~QWLAN_CCU_R_CCU_SOFT_RESET_DXE_SOFT_RESET_MASK));
    regVal = rRead(QWLAN_CCU_R_CCU_SOFT_RESET_REG); // Add some delay

    rWrite(QWLAN_DXE_0_DMA_CSR_REG, 0);

    rWrite(QWLAN_DXE_0_INT_MSK_REG, 0xFFF);

    // De-assert RESET, enable DXE, enable channel counter and configure DXE in LITTLE-ENDIAN mode.
    // This is not enough. we need to enable per channel counters. Pay attention to to destroy CTRL_SEL bits in CH_CTRL
    // registers
    regVal = rRead(QWLAN_DXE_0_DMA_CSR_REG);
    regVal = QWLAN_DXE_0_DMA_CSR_EN_MASK | QWLAN_DXE_0_DMA_CSR_ECTR_EN_MASK | QWLAN_DXE_0_DMA_CSR_H2H_SYNC_EN_MASK |
             QWLAN_DXE_0_DMA_CSR_TSTMP_EN_MASK |
             ((NT_DXE_CH_TSTMP_H2B_TSTMP_OFFSET << QWLAN_DXE_0_DMA_CSR_H2B_TSTMP_OFF_OFFSET) &
              QWLAN_DXE_0_DMA_CSR_H2B_TSTMP_OFF_MASK) |
             ((NT_DXE_CH_TSTMP_B2H_TSTMP_OFFSET << QWLAN_DXE_0_DMA_CSR_B2H_TSTMP_OFF_OFFSET) &
              QWLAN_DXE_0_DMA_CSR_B2H_TSTMP_OFF_MASK);
#ifdef PLATFORM_FERMION
    regVal |= QWLAN_DXE_0_DMA_CSR_RRAM_WRITE_DLY_DEFAULT;
#endif

    rWrite(QWLAN_DXE_0_DMA_CSR_REG, regVal);

    // Clear DXE channel counters
    rWrite(QWLAN_DXE_0_CTR_CLR_REG, 0x7f);
    // Enable DXE interrupts
    rWrite(NT_NVIC_ISER0, ENABLE_DXE_IRQ);
    rWrite(NT_NVIC_ISER1, ENABLE_DXE_IRQ1);
    return;
}

void nt_ndxe_deinit(void)
{
    volatile DescCB_t *desc_cb;

    volatile DxeCCB_t *pDxeCCB = NULL;
    uint32_t regVal, i = 0, j = 0;

    // Clear DXE channel counters
    rWrite(QWLAN_DXE_0_CTR_CLR_REG, 0x7f);

    // Reset DXE : Not reset in pronto code <--  Check if required ?
    regVal = rRead(QWLAN_CCU_R_CCU_SOFT_RESET_REG);
    rWrite(QWLAN_CCU_R_CCU_SOFT_RESET_REG, regVal | QWLAN_CCU_R_CCU_SOFT_RESET_DXE_SOFT_RESET_MASK);

    while (++i < 100) {
        regVal = rRead(QWLAN_CCU_R_CCU_SOFT_RESET_REG); // Add some delay
    }

    rWrite(QWLAN_CCU_R_CCU_SOFT_RESET_REG, regVal & (~QWLAN_CCU_R_CCU_SOFT_RESET_DXE_SOFT_RESET_MASK));

    rWrite(QWLAN_DXE_0_DMA_CSR_REG, 0);

    if (halDxe == NULL || !(halDxe->Configured))
        return;

    for (i = 0; i < DXE_CHANNEL_MAX; i++) {
        pDxeCCB = &(halDxe->DxeCCB[i]);

        if (pDxeCCB->chConfigured) {
            if (NT_DXE_XFR_BMU_TO_HOST == pDxeCCB->xfrType) {
                if (pDxeCCB->rx_buf) {
                    if (pDxeCCB->buffer_type == NT_DXE_BUF_HEAP) {
                        nt_osal_free_memory(pDxeCCB->rx_buf);
                    }
                }
            }

            if (NT_DXE_XFR_HOST_TO_HOST != pDxeCCB->xfrType) {
                desc_cb = pDxeCCB->pRingFreeHead;
                for (j = 0; j < pDxeCCB->nDescs; j++) {
                    if (desc_cb->StagingBuffer) {
                        if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
                            nt_dpm_free_network_buffer((void *)desc_cb->StagingBuffer);
                        } else {
                            nt_osal_free_memory((void *)desc_cb->StagingBuffer);
                        }
                    }
                    desc_cb = desc_cb->next;
                }
            }
        }
    }

    memset((void *)halDxe->pXfrDescPool, 0, NT_DXE_TOTAL_DESC_NO * sizeof(DescCB_t));
    memset((void *)halDxe->pDXEDescPool, 0, NT_DXE_TOTAL_DESC_NO * sizeof(DXEDesc_t));
    nt_osal_free_memory((void *)halDxe->pXfrDescPool);
    nt_osal_free_memory((void *)halDxe->pDXEDescPool);

    memset((void *)halDxe, 0, sizeof(HalDxe_t));
    nt_osal_free_memory(halDxe);
    halDxe = NULL;
}

/* Allocate DXE Descriptor Pool and Staging Buffers*/
static eRet_t nt_ndxe_allocate_desc()
{
    void *pPhysMem;
    uint32_t phyRun;
    uint32_t i;
    DXEDesc_t *pDXEDescRun;
    DescCB_t *pDCB;
    DescCB_t *pPrevDCB;
    eRet_t retVal = NDXE_FAIL;

    if (halDxe == NULL) {
        NT_LOG_DPM_ERR("Dxe not Initialized", 0, 0, 0);
        return retVal;
    }

    if (halDxe->Configured == 1) {
        return retVal;
    }

    do {
        memset((void *)halDxe->pXfrDescPool, 0, NT_DXE_TOTAL_DESC_NO * sizeof(DescCB_t));

        // Should be DWORD Aligned
        pPhysMem = (void *)halDxe->pDXEDescPool;

        memset(pPhysMem, 0, NT_DXE_TOTAL_DESC_NO * sizeof(DXEDesc_t));
        halDxe->pFreeXfrDescPoolHead =
            (volatile DescCB_t *)(halDxe->pXfrDescPool); // Allocated Tx desc starting pointer

        pDCB = (DescCB_t *)(halDxe->pXfrDescPool); // DCB running ptr
        pPrevDCB = pDCB;                           // DCB running ptr

        phyRun = (uint32_t)pPhysMem;                       // DXE Desc PHYS running ptr
        pDXEDescRun = (DXEDesc_t *)(halDxe->pDXEDescPool); // DXE Desc Virt running ptr

        for (i = 0; i < NT_DXE_TOTAL_DESC_NO; i++) {
            pPrevDCB = pDCB;

            pDCB->next = 0;
            pDCB->DXEDescAddr = pDXEDescRun;
            pDCB->physDescAddr = phyRun;

            phyRun += sizeof(DXEDesc_t);

            pDXEDescRun++;
            pDCB++;
            pPrevDCB->next = pDCB;
        }

        pPrevDCB->next = 0;

        halDxe->pFreeDescPoolCount = NT_DXE_TOTAL_DESC_NO;

        retVal = NDXE_SUCCESS;

    } while (0);

    halDxe->Configured = 1;

    return retVal;
}

/* Configure DXE Channels and set up DXE Ring Descriptors per channel*/
void nt_ndxe_config_channel(e_dxe_channel channel, p_dxe_channel_cfg_t pcfg)
{
    uint32_t regVal;
    uint32_t nDesc;
    uint32_t i;
    DescCB_t *pDCB = NULL;
    DescCB_t *pPrevDCB = NULL;
    DescCB_t *pNextDCB = NULL;
    DescCB_t *pHead = NULL;
    volatile DxeCCB_t *pDxeCCB = NULL;
    uint32_t phyRun;
    DXEDesc_t *pDxeDesc = 0;
    DXEDesc_t *pDXEDesc = NULL;
    uint32_t phyStagingBufRun;

    if (channel >= DXE_CHANNEL_MAX) {
        NT_LOG_PRINT(DPM, ERR, "asicDXEConfigChannel failed - channel %d greater than max chan# %d.\n", channel,
                     DXE_CHANNEL_MAX);
        return;
    }

    pDxeCCB = &(halDxe->DxeCCB[channel]);

    if (pDxeCCB->chConfigured) {
        NT_LOG_DPM_ERR("asicDXEConfigChannel failed - channel already configured.\n", channel, 0, 0);
        return;
    }

    // Initialize data structure and channel registers based on parameters
    pDxeCCB->noXfrDescUsed = 0;
    pDxeCCB->nDescs = pcfg->nDescs;
    pDxeCCB->bdPresent = pcfg->bdPresent;

    pDxeCCB->BDTXIdx = pcfg->BDTXIdx; // Suraj
    pDxeCCB->cbfn = pcfg->cbfn;       // Suraj
    pDxeCCB->arg = pcfg->arg;         // Suraj

    pDxeCCB->buffer_type = pcfg->buffer_type;

    pDxeCCB->use_short_desc_fmt = pcfg->useshortdescfmt;

    pDxeCCB->chDXEBaseAddr = QWLAN_DXE_0_CH0_CTRL_REG + NT_DXE_CH_REG_SIZE * channel;
    pDxeCCB->channel = channel;
    pDxeCCB->chDXECtrlRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_CTRL_REG;

    pDxeCCB->bmuThdSel = pcfg->bmuThdSel;
    pDxeCCB->bmuThdSel_mask = pcfg->bmuThdSel << QWLAN_DXE_0_CH0_CTRL_BTHLD_SEL_OFFSET;

    pDxeCCB->chPriority = pcfg->chPriority;
    pDxeCCB->xfrType = pcfg->xfrType;

    pDxeCCB->chDXETimestampRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_TSTMP_REG;
    pDxeCCB->chDXEStatusRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_STATUS_REG;
    pDxeCCB->chDXEDesclRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCL_REG;
    pDxeCCB->chDXEDeschRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCH_REG;
    pDxeCCB->chDXELstDesclRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_LST_DESCL_REG;
    pDxeCCB->chDXESzRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_SZ_REG;

    pDxeCCB->chDXESadrlRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_SADRL_REG;
    pDxeCCB->chDXESadrhRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_SADRH_REG;
    pDxeCCB->chDXEDadrlRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DADRL_REG;
    pDxeCCB->chDXEDadrhRegAddr = pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DADRH_REG;
    pDxeCCB->chk_size_mask = pcfg->chk_size << QWLAN_DXE_0_CH0_SZ_CHK_SZ_OFFSET;
    pDxeCCB->chk_size = pcfg->chk_size;

    pDxeCCB->cw_ctrl_b2h =
        (NT_DXE_DESC_CTRL_SIQ | NT_DXE_DESC_CTRL_XTYPE_B2H | NT_DXE_DESC_CTRL_EOP |
         ((pDxeCCB->bdPresent) ? 0 : NT_DXE_DESC_CTRL_BDH) |
         ((pDxeCCB->chPriority << QWLAN_DXE_0_CH0_CTRL_PRIO_OFFSET) & NT_DXE_DESC_CTRL_PRIO) |
         (pDxeCCB->bmuThdSel_mask) | NT_DXE_DESC_CTRL_PDU_REL | NT_DXE_DESC_CTRL_INT | NT_DXE_DESC_CTRL_BDT_SWAP);

    pDxeCCB->cw_ctrl_h2bh = ((pDxeCCB->xfrType << QWLAN_DXE_0_CH0_CTRL_XTYPE_OFFSET) |
                             ((NT_DXE_XFR_HOST_TO_HOST == pDxeCCB->xfrType) ? 0 : NT_DXE_DESC_CTRL_DIQ) |
                             ((pDxeCCB->bdPresent) ? 0 : NT_DXE_DESC_CTRL_BDH) |
                             ((pDxeCCB->chPriority << QWLAN_DXE_0_CH0_CTRL_PRIO_OFFSET) & NT_DXE_DESC_CTRL_PRIO) |
                             (pDxeCCB->bmuThdSel_mask) | NT_DXE_DESC_CTRL_INT | NT_DXE_DESC_CTRL_EOP);

    if (NT_DXE_XFR_HOST_TO_BMU == pDxeCCB->xfrType) {
        pDxeCCB->cw_ctrl_h2bh |= NT_DXE_DESC_CTRL_BDT_SWAP;
    } else if (NT_DXE_XFR_HOST_TO_HOST == pDxeCCB->xfrType) {
        pDxeCCB->cw_ctrl_h2bh |= NT_DXE_DESC_CTRL_ENDIANNESS;
    }

    if (NT_DXE_XFR_BMU_TO_HOST == pDxeCCB->xfrType) {
        pDxeCCB->cw_ctrl_valid = pDxeCCB->cw_ctrl_b2h | NT_SA_DXE_DESC_CTRL_VALID;
    } else {
        pDxeCCB->cw_ctrl_valid = pDxeCCB->cw_ctrl_h2bh | NT_SA_DXE_DESC_CTRL_VALID;
    }

    pDxeCCB->chan_mask = QWLAN_DXE_0_CH0_CTRL_EDVEN_MASK | (channel << QWLAN_DXE_0_CH0_CTRL_CTR_SEL_OFFSET) |
                         QWLAN_DXE_0_CH0_CTRL_EDEN_MASK | QWLAN_DXE_0_CH0_CTRL_INE_DONE_MASK |
                         QWLAN_DXE_0_CH0_CTRL_INE_ERR_MASK | QWLAN_DXE_0_CH0_CTRL_INE_ED_MASK |
                         ((pDxeCCB->use_short_desc_fmt) ? 0 : QWLAN_DXE_0_CH0_CTRL_DFMT_MASK) |
                         QWLAN_DXE_0_CH0_CTRL_EN_MASK;

    pDxeCCB->refWQ = pcfg->refWQ;

    if (!(pDxeCCB->nDescs)) {
        NT_LOG_DPM_ERR("No of descriptors set to zero\n", channel, pDxeCCB->nDescs, 0);
        return;
    }

    if (halDxe->pFreeDescPoolCount >= pDxeCCB->nDescs) {
        nDesc = pDxeCCB->nDescs;
        pHead = (DescCB_t *)halDxe->pFreeXfrDescPoolHead;
        pDCB = pHead;
        pPrevDCB = pDCB;
        for (i = 0; i < nDesc; i++) {
            pNextDCB = pDCB->next;
            pDxeDesc = (DXEDesc_t *)(pDCB->DXEDescAddr);

            if (NT_DXE_XFR_BMU_TO_HOST == pDxeCCB->xfrType) {
                if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
                    if (channel == RX_DATA)
                        phyStagingBufRun = (uint32_t)nt_dpm_allocate_network_buffer_pool(NT_MAX_STAGING_BUFFER_SIZE);
                    else
                        phyStagingBufRun = (uint32_t)nt_dpm_allocate_network_buffer(NT_MAX_STAGING_BUFFER_SIZE);
                } else {
                    phyStagingBufRun = (uint32_t)nt_osal_allocate_memory(NT_MAX_STAGING_BUFFER_SIZE);
                }
                if (phyStagingBufRun == 0) {
                    NT_LOG_DPM_CRIT("DXE Rx Buffer Allocation Failed\r\n", 0, 0, 0);
                    return;
                }
                pDCB->StagingBuffer = phyStagingBufRun;
            }

            pDxeDesc->ctrl = 0;

            if (NULL != pNextDCB) {
                phyRun = pNextDCB->physDescAddr;

                if (pDxeCCB->use_short_desc_fmt) {
                    pDxeDesc->dxedesc.dxe_short_desc.phyNextL = phyRun;
                } else {
                    pDxeDesc->dxedesc.dxe_long_desc.phyNextL = phyRun;
                    pDxeDesc->dxedesc.dxe_long_desc.phyNextH = 0;
                    pDxeDesc->dxedesc.dxe_long_desc.srcMemAddrH = 0;
                    pDxeDesc->dxedesc.dxe_long_desc.dstMemAddrH = 0;
                }
            } else {
                pPrevDCB = pDCB;
                pDCB = NULL;
                break;
            }
            pPrevDCB = pDCB;
            pDCB = pNextDCB;
        }

        // Update global free list and count
        halDxe->pFreeXfrDescPoolHead = pDCB; // The new head
        halDxe->pFreeDescPoolCount -= nDesc;
        assert(pPrevDCB);
        // Set up the allocated chain
        pPrevDCB->next = pHead; // Make DCB a ring

        phyRun = pHead->physDescAddr;

        pDxeDesc = (DXEDesc_t *)(pPrevDCB->DXEDescAddr);
        if (pDxeCCB->use_short_desc_fmt) {
            pDxeDesc->dxedesc.dxe_short_desc.phyNextL = phyRun;
        } else {
            pDxeDesc->dxedesc.dxe_long_desc.phyNextL = phyRun;
            pDxeDesc->dxedesc.dxe_long_desc.phyNextH = 0;
            pDxeDesc->dxedesc.dxe_long_desc.srcMemAddrH = 0;
            pDxeDesc->dxedesc.dxe_long_desc.dstMemAddrH = 0;
        }
        pDxeCCB->pRingFreeHead = pHead;
        pDxeCCB->pRingUsedHead = pHead;
    } else {
        NT_LOG_DPM_ERR("Enough descriptors not available in free pool...exiting.\r\n", 0, 0, 0);
        return;
    }

    // Set starting DXE descriptor
    if (pDxeCCB->use_short_desc_fmt) {
        rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DADRH_REG, 0);
        rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_SADRH_REG, 0);
        rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCH_REG, 0);
    } else {
        rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCH_REG, 0);
    }

    rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCL_REG, pDxeCCB->pRingFreeHead->physDescAddr);

    // Configure the correct BD/PDU threshold for this channel
    regVal = rRead(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_CTRL_REG);

#if 0
    //Suraj: Configure BD Template Index
    regVal = (regVal & ~QWLAN_DXE_0_CH0_CTRL_BDT_IDX_MASK) |
    		((pDxeCCB->BDTXIdx << QWLAN_DXE_0_CH0_CTRL_BDT_IDX_OFFSET) & QWLAN_DXE_0_CH0_CTRL_BDT_IDX_MASK);
#endif

    rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_CTRL_REG,
           (regVal & ~QWLAN_DXE_0_CH0_CTRL_BTHLD_SEL_MASK) | (pDxeCCB->bmuThdSel_mask));

    regVal = pDxeCCB->chk_size_mask;
    rWrite(pDxeCCB->chDXESzRegAddr, regVal);

    pDxeCCB->chConfigured = 1;

    if (NT_DXE_XFR_BMU_TO_HOST == pDxeCCB->xfrType) {
        if (pDxeCCB->buffer_type == NT_DXE_BUF_HEAP) {
            pDxeCCB->rx_buf = nt_osal_allocate_memory(NT_MAX_STAGING_BUFFER_SIZE_AMSDU);
        }
        pDCB = pDxeCCB->pRingFreeHead;

        for (i = 0; i < pDxeCCB->nDescs; i++) {
            pDXEDesc = (DXEDesc_t *)(pDCB->DXEDescAddr);

            if (pDxeCCB->use_short_desc_fmt) {
                pDXEDesc->dxedesc.dxe_short_desc.srcMemAddrL = pDxeCCB->refWQ;
            } else {
                pDXEDesc->dxedesc.dxe_long_desc.srcMemAddrL = pDxeCCB->refWQ;
            }

            if (pDxeCCB->use_short_desc_fmt) {
                pDXEDesc->dxedesc.dxe_short_desc.dstMemAddrL = pDCB->StagingBuffer;
            } else {
                pDXEDesc->dxedesc.dxe_long_desc.dstMemAddrL = pDCB->StagingBuffer;
            }

            // Set the DXE descriptor to VALID
            ((DXEDesc_t *)pDCB->DXEDescAddr)->ctrl = pDxeCCB->cw_ctrl_valid;
            pDCB = pDCB->next;
        }
        // Read it back, to ensure that the posted write transaction gets flushed
        // before the DXE channel is enabled.
        dxe_reg = (volatile uint32_t)((DXEDesc_t *)pDCB->DXEDescAddr)->ctrl;

        // Write to the CH_CTRL register only the first time, to enable the channel.
        // For subsequent attempts, to enable the channel, use the DMA_ENCH register, to avoid the possibility
        // of overwriting the CTRL DWORD loaded from the DXE descriptor, into the CH_CTRL register.
        if (pDxeCCB->chEnabled == 0) {
            rWrite(pDxeCCB->chDXECtrlRegAddr, pDxeCCB->chan_mask);
        } else {
            rWrite(QWLAN_DXE_0_DMA_ENCH_REG, (1 << pDxeCCB->channel));
        }
        pDxeCCB->chEnabled = 1; // Channel enabled
    }
}

#if 0
void nt_ndxe_channel_change(e_dxe_channel channel, uint8_t amsdu)
{
	DescCB_t *pDCB;
	volatile DxeCCB_t *pDxeCCB;
	uint32_t phyStagingBufRun = 0;
	uint32_t i;
	volatile uint32_t regVal;

	// Check Channel Busy. If yes, wait for completion
	do { // wait for completion
		regVal = rRead(QWLAN_DXE_0_CH6_STATUS_REG);
	} while ((regVal & QWLAN_DXE_0_CH6_STATUS_BUSY_MASK));

	//Disable Channel
	rWrite(QWLAN_DXE_0_CH6_CTRL_REG, (1 << QWLAN_DXE_0_CH6_CTRL_ABORT_OFFSET));

	do {
		regVal = rRead(QWLAN_DXE_0_CH6_STATUS_REG);
	} while ((regVal & QWLAN_DXE_0_CH6_STATUS_ABORT_REQ_MASK));


	regVal = rRead(QWLAN_DXE_0_CH6_STATUS_REG);
	rWrite(QWLAN_DXE_0_CH6_STATUS_REG, regVal |  (1 << QWLAN_DXE_0_CH6_STATUS_ERR_OFFSET));

	rWrite(QWLAN_DXE_0_INT_ERR_CLR_REG, (1 << QWLAN_DXE_0_INT_ERR_CLR_CH6_INT_ERR_CLR_OFFSET));

	//If Transfer Complete,set channel change notification and return
	if (nt_dxe_get_pending_pkt_count(DXE_CHANNEL_6) != 0) {
		nt_dpm_rx_notify();
		nt_ndxe_channel_change_notify(amsdu);
		return;
	}
	NT_LOG_DPM_INFO("DXE channel configuration change",amsdu,0,0);

	if (channel == DXE_CHANNEL_6) {
		pDxeCCB = &(halDxe->DxeCCB[channel]);
		pDCB = pDxeCCB->pRingFreeHead;

		if (!(NT_DXE_XFR_BMU_TO_HOST == pDxeCCB->xfrType)) {
			return;
		}

		for (i=0; i< pDxeCCB->nDescs; i++) {
			if (pDCB->StagingBuffer) {
				if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
					nt_dpm_free_buffer((void *)pDCB->StagingBuffer);
				} else {
					nt_osal_free_memory((void *)pDCB->StagingBuffer);
				}
			}

			if (rx_amsdu_enabled) {
				if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
					phyStagingBufRun = (uint32_t)nt_dpm_allocate_buffer(NT_MAX_STAGING_BUFFER_SIZE_AMSDU);
				} else {
					phyStagingBufRun = (uint32_t)nt_osal_allocate_memory(NT_MAX_STAGING_BUFFER_SIZE_AMSDU);
				}

				if (phyStagingBufRun == 0) {
					NT_LOG_DPM_CRIT("DXE Rx Buffer Allocation Failed\r\n",0,0,0);
					return;
				}
				//NT_LOG_DPM_INFO("staging buffer size for add ba::",phyStagingBufRun,0,0);
			} else {
				if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
					phyStagingBufRun = (uint32_t)nt_dpm_allocate_buffer_pool(NT_MAX_STAGING_BUFFER_SIZE);
				} else {
					phyStagingBufRun = (uint32_t)nt_osal_allocate_memory(NT_MAX_STAGING_BUFFER_SIZE);
				}

				if (phyStagingBufRun == 0) {
					NT_LOG_DPM_CRIT("DXE Rx Buffer Allocation Failed\r\n",0,0,0);
					return;
				}
				//NT_LOG_DPM_INFO("staging buffer size for del ba::",phyStagingBufRun,0,0);
			}

			pDCB->StagingBuffer = phyStagingBufRun;


			if (pDxeCCB->use_short_desc_fmt) {
				((DXEDesc_t *)(pDCB->DXEDescAddr))->dxedesc.dxe_short_desc.dstMemAddrL = pDCB->StagingBuffer;
			} else {
				((DXEDesc_t *)(pDCB->DXEDescAddr))->dxedesc.dxe_long_desc.dstMemAddrL = pDCB->StagingBuffer;
			}

			pDCB = pDCB->next;
		}

		//HW Programming
		// Set starting DXE descriptor
		if (pDxeCCB->use_short_desc_fmt) {
			rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DADRH_REG, 0);
			rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_SADRH_REG, 0);
			rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCH_REG, 0);
		} else {
			rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCH_REG, 0);
		}

		rWrite(pDxeCCB->chDXEBaseAddr + NT_DXE_CH_DESCL_REG, pDxeCCB->pRingFreeHead->physDescAddr);

		regVal = pDxeCCB->chk_size_mask;
		rWrite( pDxeCCB->chDXESzRegAddr, regVal);

		rWrite(pDxeCCB->chDXECtrlRegAddr, pDxeCCB->chan_mask);
	}
}
#endif

/* Set the current descriptor as valid and enable the channel. The DXE processes the descriptor for transfer*/
static void ndxe_enable_dxe_desc(volatile DxeCCB_t *pDxeCCB, DescCB_t *pDCB)
{
    // Set the first DXE descriptor to VALID
    ((DXEDesc_t *)pDCB->DXEDescAddr)->ctrl = pDxeCCB->cw_ctrl_valid;

    // Read it back, to ensure that the posted write transaction gets flushed
    // before the DXE channel is enabled.
    (volatile void)((DXEDesc_t *)pDCB->DXEDescAddr)->ctrl;

    // Write to the CH_CTRL register only the first time, to enable the channel.
    // For subsequent attempts, to enable the channel, use the DMA_ENCH register, to avoid the possibility
    // of overwriting the CTRL DWORD loaded from the DXE descriptor, into the CH_CTRL register.
    if (pDxeCCB->chEnabled == 0) {
        rWrite(pDxeCCB->chDXECtrlRegAddr, pDxeCCB->chan_mask);
    } else {
        rWrite(QWLAN_DXE_0_DMA_ENCH_REG, (1 << pDxeCCB->channel));
    }
    pDxeCCB->chEnabled = 1; // Channel enabled
}

#ifdef DXE_WAR_FOR_DATA_STALL
static void nt_dxe_update_descctrl_in_lst(volatile DxeCCB_t *pDxeCCB)
{
    uint32_t i;

    volatile DescCB_t *pDCB = NULL;
    volatile DXEDesc_t *pDXEDesc = NULL;
    volatile uint32_t desc_ctrl;
    volatile uint32_t chDescLReg;

    uint32_t nDesc;
    uint32_t xfr_type;

    if (pDxeCCB == NULL) {
        return;
    }

    pDCB = pDxeCCB->pRingFreeHead;
    nDesc = pDxeCCB->nDescs;
    xfr_type = pDxeCCB->xfrType;

    if (((xfr_type != NT_DXE_XFR_HOST_TO_BMU) && (xfr_type != NT_DXE_XFR_BMU_TO_HOST)) || (pDCB == NULL)) {
        return;
    }

    chDescLReg = rRead(pDxeCCB->chDXEDesclRegAddr);
    if (pDCB->physDescAddr == chDescLReg) {
        return;
    }

    for (i = 0; i < nDesc; i++) {
        pDXEDesc = (DXEDesc_t *)(pDCB->DXEDescAddr);
        desc_ctrl = (volatile uint32_t)pDXEDesc->ctrl;

        if ((desc_ctrl & NT_DXE_DESC_CTRL_VALID) == 0) {
            ((DXEDesc_t *)pDCB->DXEDescAddr)->ctrl |= NT_DXE_DESC_CTRL_VALID;
            (volatile void)((DXEDesc_t *)pDCB->DXEDescAddr)->ctrl;

            // introduce some delay
            // dxe_reg = (volatile uint32_t) rRead(pDxeCCB->chDXELstDesclRegAddr);
        }
        pDCB = pDCB->next;
    }

    return;
}

#endif /* DXE_WAR_FOR_DATA_STALL */

/* Write frame for transfer from the Host. Used for H2B and H2H transfer */
eRet_t __attribute__((section(".after_ram_vectors")))
nt_ndxe_write_frame_to_transfer(e_dxe_channel channel, const void *frame, uint32_t length, void *h2hdst)
{
    DescCB_t *pDCB;
    DescCB_t *pCurrDCB;
    volatile DXEDesc_t *pDXEDesc;
    volatile DxeCCB_t *pDxeCCB;
    uint32_t StagingBuf = 0;
    volatile uint32_t desc_ctrl;
    static uint32_t dxe_halt_count = 0;
    uint32_t H2HDstPhysAddr = 0;

    if (frame == NULL) {
        NT_LOG_DPM_ERR("Frame not present", 0, 0, 0);
        return NDXE_FAIL;
    }

    if (length > NT_MAX_STAGING_BUFFER_SIZE_AMSDU) {
        NT_LOG_DPM_ERR("Frame Too Large bytes", length, 0, 0);
        return NDXE_FAIL;
    }

    if (length == 0) {
        NT_LOG_DPM_ERR(" Zero Length Packet", 0, 0, 0);
        return NDXE_FAIL;
    }

    if (channel >= DXE_CHANNEL_MAX) {
        NT_LOG_DPM_ERR(" Invalid channel...exiting.", 0, 0, 0);
        return NDXE_FAIL;
    }

    pDxeCCB = &(halDxe->DxeCCB[channel]);

    if (!pDxeCCB->chConfigured) {
        NT_LOG_DPM_ERR("DXE channel not configured...exiting.", 0, 0, 0);
        return NDXE_FAIL;
    }

    if ((pDxeCCB->xfrType != NT_DXE_XFR_HOST_TO_BMU) && (pDxeCCB->xfrType != NT_DXE_XFR_HOST_TO_HOST)) {
        NT_LOG_DPM_ERR("Invalid transfer type...exiting.", 0, 0, 0);
        return NDXE_FAIL;
    }
#ifdef MEM_CPY_VIA_DXE
    if ((pDxeCCB->xfrType != NT_DXE_XFR_HOST_TO_HOST) && (((uint32_t)frame % 4) != 0)) {
        NT_LOG_DPM_CRIT(" Frame not DWORD Aligned\r\n", (uint32_t)frame, 0, 0);
        return NDXE_FAIL;
    }

    if (halDxe->dxe_suspend) {
        NT_LOG_PRINT(DPM, ERR, "pkt queued aftr suspend - dxech:%d", channel);
        pDxeCCB->pkts_after_suspend++;
        return NDXE_NO_FREE_DESC;
    }

#else /* MEM_CPY_VIA_DXE */

    if (((uint32_t)frame % 4) != 0) {
        NT_LOG_DPM_CRIT(" Frame not DWORD Aligned\r\n", 0, 0, 0);
        return NDXE_FAIL;
    }

    if (halDxe->dxe_suspend) {
        // NT_LOG_PRINT(DPM, ERR, "pkt queued aftr suspend - dxech:%d", channel);
        pDxeCCB->pkts_after_suspend++;
        return NDXE_FAIL;
    }

#endif /* MEM_CPY_VIA_DXE */

    if (NT_DXE_XFR_HOST_TO_HOST == pDxeCCB->xfrType) {
        if (h2hdst == NULL) {
            return NDXE_INVAILD_PARAMS;
        }
        H2HDstPhysAddr = (uint32_t)h2hdst;
    }

    pDCB = pDxeCCB->pRingFreeHead;
    pCurrDCB = pDCB;

    pDXEDesc = (DXEDesc_t *)pCurrDCB->DXEDescAddr;

    // As a workaround for the delayed VALID descriptor updation on FPGA generate some read
    // activity on the bus and this should flush any stale FIFOs
    dxe_reg = (volatile uint32_t)rRead(pDxeCCB->chDXELstDesclRegAddr);

    desc_ctrl = (volatile uint32_t)pDXEDesc->ctrl;

    if (desc_ctrl & NT_SA_DXE_DESC_CTRL_VALID) {
        dxe_halt_count++;
        /*Update DXE Descctrl when dropping 100 frames since DXE may have HALT*/
        if (dxe_halt_count < 100) {
            return NDXE_NO_FREE_DESC;
        }

#ifdef DXE_WAR_FOR_DATA_STALL
        nt_dxe_update_descctrl_in_lst(pDxeCCB);
        /*NOTE:in some corner with powersaving enabled,DXE may have HALT but buffer been write to description*/
        rWrite(QWLAN_DXE_0_DMA_ENCH_REG, (1 << pDxeCCB->channel));
#endif /* DXE_WAR_FOR_DATA_STALL */
        dxe_halt_count = 0;
        return NDXE_NO_FREE_DESC;
    }
#ifdef NT_TST_TIME_STAMP_ENABLE
    if ((nt_dpm_tm.tx_stat[DXE_TX].valid == 0) &&
        (*(uint32_t *)((uint8_t *)frame + NT_TX_BUFFER_OFFSET + nt_dpm_tm.tx_stat[LOW_LVL_OUTPUT].offset) ==
         nt_dpm_tm.tx_marker)) {
        nt_dpm_tm.tx_stat[DXE_TX].value = nt_hal_get_curr_time();
        nt_dpm_tm.tx_stat[DXE_TX].offset = (uint32_t)pDXEDesc;
        nt_dpm_tm.tx_stat[DXE_TX].valid = 1;
    }
#endif
    dxe_halt_count = 0;
    if (H2HDstPhysAddr == 0) {
        StagingBuf = pCurrDCB->StagingBuffer;
    }

    pCurrDCB->StagingBuffer = (uint32_t)frame;

    if (StagingBuf != 0) {
        if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
            nt_dpm_free_network_buffer((void *)StagingBuf);
        } else {
            nt_osal_free_memory((void *)StagingBuf);
        }
    }

    StagingBuf = pCurrDCB->StagingBuffer;

    pDXEDesc->xfrSize = (length & 0x3FFF);

    if (pDxeCCB->use_short_desc_fmt) {
        pDXEDesc->dxedesc.dxe_short_desc.srcMemAddrL = StagingBuf;
    } else {
        pDXEDesc->dxedesc.dxe_long_desc.srcMemAddrL = StagingBuf;
    }

    if (H2HDstPhysAddr) {
        if (pDxeCCB->use_short_desc_fmt) {
            pDXEDesc->dxedesc.dxe_short_desc.dstMemAddrL = H2HDstPhysAddr;
        } else {
            pDXEDesc->dxedesc.dxe_long_desc.dstMemAddrL = H2HDstPhysAddr;
        }
    } else {
        if (pDxeCCB->use_short_desc_fmt) {
            pDXEDesc->dxedesc.dxe_short_desc.dstMemAddrL = pDxeCCB->refWQ;
        } else {
            pDXEDesc->dxedesc.dxe_long_desc.dstMemAddrL = pDxeCCB->refWQ;
        }
    }

    pCurrDCB = pCurrDCB->next;
    pDxeCCB->pRingFreeHead = pCurrDCB;

    (pDxeCCB->pkt_count)++;

    ndxe_enable_dxe_desc(pDxeCCB, pDCB);

    return NDXE_SUCCESS;
}

#if 0
/* Write frame for transfer from the Host. Used for H2B and H2H transfer */
eRet_t nt_ndxe_write_frame_to_transfer_single(e_dxe_channel channel, void *frame, uint32_t length, void *h2hdst)
{
	volatile DxeCCB_t *pDxeCCB;
	volatile uint32_t StagingBuf;
	volatile uint32_t dxe_ctrl = 0;
	volatile uint32_t xfr_size = 0;

	(void) h2hdst;

    if (frame == NULL) {
		printf("(%s,%d): Frame not present\n",__func__, __LINE__);
        return NDXE_FAIL;
    }

    if (length > NT_MAX_STAGING_BUFFER_SIZE) {
		printf("(%s,%d): Frame Too Large\n",__func__, __LINE__);
        return NDXE_FAIL;
    }

    if (length == 0) {
		printf("(%s,%d): Zero Length Packet\n",__func__, __LINE__);
        return NDXE_FAIL;
    }

    if (channel >= DXE_CHANNEL_MAX) {
		printf("(%s,%d): Invalid channel...exiting.\n",__func__, __LINE__);
        return NDXE_FAIL;
	}

    pDxeCCB = &(halDxe->DxeCCB[channel]);

    if (!pDxeCCB->chConfigured) {
		printf("(%s,%d): DXE channel not configured...exiting.\n",__func__, __LINE__);
        return NDXE_FAIL;
	}

    if((pDxeCCB->xfrType != NT_DXE_XFR_HOST_TO_BMU) &&
	   (pDxeCCB->xfrType != NT_DXE_XFR_HOST_TO_HOST)) {
		printf("(%s,%d): Invalid transfer type...exiting.\n",__func__, __LINE__);
        return NDXE_FAIL;
	}

	StagingBuf = (uint32_t)frame;

	if ((StagingBuf % 4) != 0) {
		//ADD Hack if not DWORD Aligned
		NT_LOG_DPM_CRIT("StagingBuffer not DWORD Aligned\n.",0,0,0);
		return NDXE_FAIL;
	}

	xfr_size = (length & 0x3FFF);
	rWrite( pDxeCCB->chDXESzRegAddr, xfr_size);

	rWrite( pDxeCCB->chDXESadrlRegAddr, StagingBuf);
	rWrite( pDxeCCB->chDXESadrhRegAddr, 0);

	rWrite( pDxeCCB->chDXEDadrlRegAddr, 6);
	rWrite( pDxeCCB->chDXEDadrhRegAddr, 0);

    dxe_ctrl = ((NT_DXE_XFR_HOST_TO_BMU << QWLAN_DXE_0_CH0_CTRL_XTYPE_OFFSET) |
				(NT_DXE_DESC_CTRL_DIQ) |
				((pDxeCCB->chPriority << QWLAN_DXE_0_CH0_CTRL_PRIO_OFFSET) & NT_DXE_DESC_CTRL_PRIO) |
				(pDxeCCB->bmuThdSel_mask) |
//				DXE_DESC_CTRL_ENDIANNESS |
				NT_SA_DXE_DESC_CTRL_INT_N |
				NT_SA_DXE_DESC_CTRL_INT_ERR |
				NT_SA_DXE_DESC_CTRL_EOP |
				NT_DXE_CTRL_BDT_SWAP |
				QWLAN_DXE_0_CH0_CTRL_EN_MASK);

	rWrite( pDxeCCB->chDXECtrlRegAddr, dxe_ctrl);

    return NDXE_SUCCESS;
}
#endif

/* Obtain the frame from the DXE Ring Descriptor after transfer to Staging Buffer */
eRet_t nt_ndxe_get_single_received_frame(e_dxe_channel channel, void **frame)
{
    DescCB_t *pDCB;
    volatile DxeCCB_t *pDxeCCB;
    uint32_t desc_ctrl;
    uint32_t stagBuff;

    *frame = 0;

    if (channel >= DXE_CHANNEL_MAX) {
        NT_LOG_DPM_ERR("Invalid channel...exiting.", 0, 0, 0);
        return NDXE_INVALID_CHANNEL;
    }

    pDxeCCB = &(halDxe->DxeCCB[channel]);

    if (!pDxeCCB->chConfigured) {
        NT_LOG_DPM_ERR("DXE channel not configured...exiting.", 0, 0, 0);
        return NDXE_FAIL;
    }

    if (pDxeCCB->xfrType != NT_DXE_XFR_BMU_TO_HOST) {
        NT_LOG_DPM_ERR("Invalid transfer type...exiting.", 0, 0, 0);
        return NDXE_FAIL;
    }

    if (halDxe->dxe_suspend) {
        // NT_LOG_PRINT(DPM, ERR, "pkt recv aftr suspend - dxech:%d", channel);
        pDxeCCB->pkts_after_suspend++;
        // return NDXE_FAIL;
    }

    pDCB = pDxeCCB->pRingFreeHead;

    // As a workaround for the delayed VALID descriptor updation on FPGA generate some read
    // activity on the bus and this should flush any stale FIFOs
    dxe_reg = (volatile uint32_t)rRead(pDxeCCB->chDXELstDesclRegAddr);

    // Check for VALID bit clear
    desc_ctrl = ((DXEDesc_t *)(pDCB->DXEDescAddr))->ctrl;

    if (desc_ctrl & NT_SA_DXE_DESC_CTRL_VALID) {
        // nt_dxe_update_descctrl_in_lst(pDxeCCB);
        /*NOTE:in some corner with powersaving enabled,DXE may have HALT but buffer been write to description*/
        // rWrite(QWLAN_DXE_0_DMA_ENCH_REG, (1 << pDxeCCB->channel));
        return NDXE_NO_PKTS_AVAILABLE;
    }

    if (pDxeCCB->buffer_type == NT_DXE_BUF_PBUF) {
        if (rx_amsdu_enabled) {
            if (channel == RX_DATA)
                stagBuff = (uint32_t)nt_dpm_allocate_network_buffer(NT_MAX_STAGING_BUFFER_SIZE_AMSDU);
            else
                stagBuff = (uint32_t)nt_dpm_allocate_network_buffer(NT_MAX_STAGING_BUFFER_SIZE_AMSDU);
        } else {
            if (channel == RX_DATA)
                stagBuff = (uint32_t)nt_dpm_allocate_network_buffer_pool(NT_MAX_STAGING_BUFFER_SIZE);
            else
                stagBuff = (uint32_t)nt_dpm_allocate_network_buffer(NT_MAX_STAGING_BUFFER_SIZE);
        }
    } else {
        stagBuff = (uint32_t)pDxeCCB->rx_buf;
    }

    if (stagBuff == (uint32_t)NULL) {
        // NT_LOG_DPM_ERR("Rx buff Null error",0,0,0);
        return NDXE_NO_MEM_AVAILABLE;
    }

    pDxeCCB->pRingFreeHead = pDCB->next;

    *frame = (void *)(pDCB->StagingBuffer);
    if (pDCB->StagingBuffer == (uint32_t)NULL) {
        NT_LOG_DPM_ERR("Staging buff Null error", 0, 0, 0);
        return NDXE_NO_MEM_AVAILABLE;
    }

    // Assign the RX frame here
    pDCB->StagingBuffer = stagBuff;
    if (pDxeCCB->buffer_type == NT_DXE_BUF_HEAP) {
        pDxeCCB->rx_buf = *frame;
    }

    if (pDxeCCB->use_short_desc_fmt) {
        ((DXEDesc_t *)(pDCB->DXEDescAddr))->dxedesc.dxe_short_desc.srcMemAddrL = pDxeCCB->refWQ;
    } else {
        ((DXEDesc_t *)(pDCB->DXEDescAddr))->dxedesc.dxe_long_desc.srcMemAddrL = pDxeCCB->refWQ;
    }

    if (pDxeCCB->use_short_desc_fmt) {
        ((DXEDesc_t *)(pDCB->DXEDescAddr))->dxedesc.dxe_short_desc.dstMemAddrL = pDCB->StagingBuffer;
    } else {
        ((DXEDesc_t *)(pDCB->DXEDescAddr))->dxedesc.dxe_long_desc.dstMemAddrL = pDCB->StagingBuffer;
    }

    (pDxeCCB->pkt_count)++;

    ndxe_enable_dxe_desc(pDxeCCB, pDCB);

    return NDXE_SUCCESS;
}

/*To be Used Later*/
eRet_t ndxe_free_packet_from_desc_ring(e_dxe_channel channel)
{
    DescCB_t *pDCB;
    volatile DxeCCB_t *pDxeCCB;
    DXEDesc_t *p_dxe_desc;
    uint32_t desc_ctrl;
    eRet_t ret = NDXE_SUCCESS;

    if (channel >= DXE_CHANNEL_MAX) {
        NT_LOG_DPM_ERR(" Invalid channel...exiting.", 0, 0, 0);
        return NDXE_INVALID_CHANNEL;
    }

    pDxeCCB = &(halDxe->DxeCCB[channel]);

    pDCB = pDxeCCB->pRingUsedHead;

    p_dxe_desc = (DXEDesc_t *)pDCB->DXEDescAddr;

    desc_ctrl = p_dxe_desc->ctrl;

    while ((desc_ctrl & NT_DXE_DESC_CTRL_VALID) == 0) {
    }

    return ret;
}

/* Initialize DXE module*/
eRet_t nt_ndxe_init()
{
    eRet_t status;

    if ((halDxe) && (halDxe->Configured)) {
        NT_LOG_DPM_INFO("DXE init done", 0, 0, 0);
        return NDXE_SUCCESS;
    }

    /* Allocating halDxe Structure*/
    halDxe = nt_osal_allocate_memory(sizeof(HalDxe_t));
    if (halDxe == NULL) {
        NT_LOG_DPM_ERR("Failed to allocate Dxe Structure", 0, 0, 0);
        return NDXE_BUFF_ALLOC_FAILED;
    }
    memset((void *)halDxe, 0, sizeof(HalDxe_t));

    /* Allocating the DXE Transfer Pools*/
    halDxe->pXfrDescPool = nt_osal_calloc(NT_DXE_TOTAL_DESC_NO, sizeof(DescCB_t));
    if (halDxe->pXfrDescPool == NULL) {
        NT_LOG_DPM_ERR("Failed to allocate Dxe Desc Pool", 0, 0, 0);
        return NDXE_BUFF_ALLOC_FAILED;
    }

    /* Allocating the DXE Descriptor Pools*/
    halDxe->pDXEDescPool = nt_osal_calloc(NT_DXE_TOTAL_DESC_NO, sizeof(DXEDesc_t));
    if (halDxe->pDXEDescPool == NULL) {
        NT_LOG_DPM_ERR("Failed to allocate Dxe Desc Pool", 0, 0, 0);
        return NDXE_BUFF_ALLOC_FAILED;
    }

    /* Configure the DXE Descriptors*/
    status = nt_ndxe_allocate_desc();

    if (status != NDXE_SUCCESS) {
        NT_LOG_DPM_ERR("DXE configuration failed", 0, 0, 0);
        return status;
    }

    /* Reset and Start the DXE */
    nt_ndxe_start();
    NT_LOG_DPM_INFO("DXE Init Success", 0, 0, 0);
    return status;
}
uint8_t g_dxe_error_int = 0;
uint32_t dxe_err_cnt[12];
eRet_t __attribute__((section(".after_ram_vectors"))) ndxe_irq_handler()
{
    uint32_t regVal;
    volatile DxeCCB_t *pDxeCCB;

    regVal = rRead(QWLAN_DXE_0_INT_ERR_SRC_REG);
    if (regVal & ~QWLAN_DXE_0_INT_ERR_SRC_RSVD_MASK) {
        //		NT_LOG_DPM_CRIT("DXE interrupt error occured",0,0,0);
        // First clear DXE interrupts

        for (uint32_t i = 0; i < DXE_CHANNEL_MAX; i++) {
            if (regVal & (1 << i)) {
                dxe_err_cnt[i]++;
                // uint32_t status = rRead((QWLAN_DXE_0_CH0_STATUS_REG + i * 0x40));
                // NT_LOG_DPM_ERR("DXE_error_int:channel/status",i,status,0);
                g_dxe_error_int++;
            }
        }
        rWrite(QWLAN_DXE_0_INT_CLR_REG, regVal & ~QWLAN_DXE_0_INT_ERR_SRC_RSVD_MASK);
        // printf("(%s:%d): DXE_0_INT_ERR_SRC = 0x%08x, DXE_0_CH1_STATUS = 0x%08x, DXE_0_CH0_STATUS_REG = 0x%08x\n",
        // __func__, __LINE__, regVal, inp(DXE_0_CH1_STATUS_REG), inp(DXE_0_CH0_STATUS_REG)); return
        // NDXE_ERROR_INTERRUPT;
    }

    regVal = rRead(QWLAN_DXE_0_INT_SRC_RAW_REG);
    if (regVal & ~QWLAN_DXE_0_INT_SRC_RAW_RSVD_MASK) {

        // Channel 0
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH0_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH0_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_0]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 1
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH1_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH1_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_1]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 2
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH2_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH2_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_2]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 3
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH3_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH3_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_3]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 4
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH4_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH4_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_4]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 5
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH5_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH5_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_5]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 6
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH6_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH6_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_6]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 7
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH7_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH7_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_7]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 8
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH8_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH8_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_8]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 9
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH9_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH9_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_9]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 10
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH10_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH10_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_10]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }

        // Channel 11
        if (regVal & QWLAN_DXE_0_INT_SRC_RAW_CH11_INT_MASK) {
            rWrite(QWLAN_DXE_0_INT_CLR_REG, QWLAN_DXE_0_INT_CLR_CH11_INT_CLR_MASK);
            pDxeCCB = (volatile DxeCCB_t *)&(halDxe->DxeCCB[DXE_CHANNEL_11]);
            if (pDxeCCB->cbfn) {
                (pDxeCCB->cbfn)(pDxeCCB->arg);
            }
        }
    }

    return NDXE_SUCCESS;
}

void __attribute__((section(".after_ram_vectors"))) nt_dxe_interrupt_handler(void)
{
    eRet_t ret = 0;

    ret = ndxe_irq_handler();
    if (ret != NDXE_SUCCESS) {
        // NT_LOG_DPM_CRIT("DXE interrupt error occured",0,0,0);
    }
}

uint32_t nt_dxe_get_pending_pkt_count(e_dxe_channel channel)
{
    uint32_t i, count = 0;
    DescCB_t *pDCB = NULL;
    DXEDesc_t *pDXEDesc = NULL;
    uint32_t desc_ctrl;
    uint32_t nDesc;
    uint32_t xfr_type;
    volatile DxeCCB_t *pDxeCCB = NULL;

    pDxeCCB = &(halDxe->DxeCCB[channel]);
    pDCB = pDxeCCB->pRingUsedHead;
    nDesc = pDxeCCB->nDescs;
    xfr_type = pDxeCCB->xfrType;
    for (i = 0; i < nDesc; i++) {
        pDXEDesc = (DXEDesc_t *)(pDCB->DXEDescAddr);
        desc_ctrl = pDXEDesc->ctrl;
        if ((xfr_type == NT_DXE_XFR_HOST_TO_BMU) && ((desc_ctrl & NT_DXE_DESC_CTRL_VALID) == 1)) {
            count++;
        } else if ((xfr_type == NT_DXE_XFR_BMU_TO_HOST) && ((desc_ctrl & NT_DXE_DESC_CTRL_VALID) == 0)) {
            count++;
        }

        pDCB = pDCB->next;
    }

    return count;
}

#ifdef NT_TST_TIME_STAMP_ENABLE
void *nt_dxe_get_tm_tx_pkt(e_dxe_channel channel)
{
    volatile DxeCCB_t *pDxeCCB = NULL;
    volatile DescCB_t *pDCB = NULL;
    volatile DXEDesc_t *pDXEDesc = NULL;

    pDxeCCB = &(halDxe->DxeCCB[channel]);
    pDCB = pDxeCCB->pRingUsedHead;

    if (nt_dpm_tm.tx_stat[DXE_TX].valid == 0)
        return NULL;

    while ((uint32_t)pDCB->DXEDescAddr != nt_dpm_tm.tx_stat[DXE_TX].offset) {
        pDCB = pDCB->next;
        if (pDCB == pDxeCCB->pRingUsedHead) {
            return NULL;
        }
    }

    pDXEDesc = pDCB->DXEDescAddr;

    dxe_reg = (volatile uint32_t)rRead(pDxeCCB->chDXELstDesclRegAddr);

    if ((volatile uint32_t)pDXEDesc->ctrl & NT_SA_DXE_DESC_CTRL_VALID) {
        return NULL;
    }

    if (pDxeCCB->use_short_desc_fmt) {
        return (void *)(pDXEDesc->dxedesc.dxe_short_desc.srcMemAddrL);
    } else {
        return (void *)(pDXEDesc->dxedesc.dxe_long_desc.srcMemAddrL);
    }
}
#endif

void *nt_dxe_get_last_tx_pkt(e_dxe_channel channel)
{
    volatile DxeCCB_t *pDxeCCB = NULL;
    volatile DXEDesc_t *pDXEDesc = NULL;

    pDxeCCB = &(halDxe->DxeCCB[channel]);
    pDXEDesc = (volatile DXEDesc_t *)rRead(pDxeCCB->chDXELstDesclRegAddr);

    if (pDxeCCB->use_short_desc_fmt) {
        return (void *)(pDXEDesc->dxedesc.dxe_short_desc.srcMemAddrL);
    } else {
        return (void *)(pDXEDesc->dxedesc.dxe_long_desc.srcMemAddrL);
    }
}

void *nt_dxe_get_last_rx_pkt(e_dxe_channel channel)
{
    volatile DxeCCB_t *pDxeCCB = NULL;
    volatile DXEDesc_t *pDXEDesc = NULL;
    DescCB_t *pDescCB;

    pDxeCCB = &(halDxe->DxeCCB[channel]);
    pDXEDesc = (volatile DXEDesc_t *)rRead(pDxeCCB->chDXELstDesclRegAddr);

    pDescCB = pDxeCCB->pRingFreeHead;

    while (pDescCB->next->DXEDescAddr != pDXEDesc) {
        pDescCB = pDescCB->next;
        if (pDescCB == pDxeCCB->pRingFreeHead) {
            return NULL;
        }
    }

    pDXEDesc = pDescCB->DXEDescAddr;

    if (pDxeCCB->use_short_desc_fmt) {
        return (void *)(pDXEDesc->dxedesc.dxe_short_desc.dstMemAddrL);
    } else {
        return (void *)(pDXEDesc->dxedesc.dxe_long_desc.dstMemAddrL);
    }
}

uint32_t nt_dxe_get_dxe_timestamp(e_dxe_channel channel)
{
    volatile DxeCCB_t *pDxeCCB = NULL;
    pDxeCCB = &(halDxe->DxeCCB[channel]);
    volatile uint32_t reg_val;

    reg_val = rRead(pDxeCCB->chDXETimestampRegAddr);
    return reg_val;
}

uint32_t hal_dxe_suspend()
{
    uint32_t wait_count = 0;
    uint32_t return_val = NDXE_SUCCESS;

    volatile uint32_t regVal;

    NT_LOG_DPM_INFO("dxe_suspend", 0, 0, 0);
#ifdef DEBUG
    uint32_t start = HAL_REG_RD(QWLAN_MTU_MTU_GLOBAL_TIMER_REG);
    ++g_dxe_suspend;
#endif

    regVal = HAL_REG_RD(QWLAN_DXE_0_DMA_CSR_REG);
    rWrite(QWLAN_DXE_0_DMA_CSR_REG, regVal | QWLAN_DXE_0_DMA_CSR_PAUSE_MASK);
    while (wait_count++ < DXE_MAX_RETRY) {
        regVal = HAL_REG_RD(QWLAN_DXE_0_DMA_CSR_REG);
        if (regVal & QWLAN_DXE_0_DMA_CSR_PAUSED_MASK) {
            NT_LOG_DPM_INFO("dxe_suspend_succ", 0, 0, 0);
            halDxe->dxe_suspend = 1;
            break;
        }
        // Replacing taskdelay with for loop
        nt_socpm_nop_delay(10);
    }

#ifdef DEBUG
    g_dxe_stop_time = rRead(QWLAN_MTU_MTU_GLOBAL_TIMER_REG);
    g_dxe_stop_time -= start;
#endif

    if (wait_count == DXE_MAX_RETRY) {
        //	NT_LOG_PRINT(DPM, ERR, "dpm stopped failed dma_csr: 0x%x wmac_status: 0x%x wait_cnt: %d",
        //    HAL_REG_RD(QWLAN_DXE_0_DMA_CSR_REG), HAL_REG_RD(QWLAN_MCU_MCU_WMAC_STATUS_REG), wait_count);

        NT_LOG_DPM_ERR("dpm stopped failed", HAL_REG_RD(QWLAN_DXE_0_DMA_CSR_REG),
                       HAL_REG_RD(QWLAN_MCU_MCU_WMAC_STATUS_REG), wait_count);
        return_val = NDXE_FAIL;
    }

    return return_val;
}
uint32_t hal_dxe_resume()
{
    uint32_t regVal = HAL_REG_RD(QWLAN_DXE_0_DMA_CSR_REG);
    uint32_t wait_count = 0;
    regVal = regVal & (~QWLAN_DXE_0_DMA_CSR_PAUSE_MASK);

    // if( halDxe->dxe_suspend )
    {
        HAL_REG_WR(QWLAN_DXE_0_DMA_CSR_REG, regVal);
#ifdef DEBUG
        ++g_dxe_resume;
#endif
        // halDxe->dxe_suspend = 0;
        NT_LOG_DPM_INFO("hal_dxe_resume", 0, 0, 0);

        while (wait_count++ < DXE_MAX_RETRY) {
            regVal = HAL_REG_RD(QWLAN_DXE_0_DMA_CSR_REG);
            NT_LOG_DPM_INFO("dxe_r", regVal, QWLAN_DXE_0_DMA_CSR_PAUSE_MASK, 0);
            if ((regVal & (QWLAN_DXE_0_DMA_CSR_PAUSE_MASK)) == 0) {
                NT_LOG_DPM_INFO("dxe_resume_succ", 0, 0, 0);
                halDxe->dxe_suspend = 0;
                break;
            }
            // Replacing taskdelay with for loop
            nt_socpm_nop_delay(100);
        }
    }
    return NDXE_SUCCESS;
}

void nt_dxe_update_intr_cnt(e_dxe_channel channel)
{
    volatile DxeCCB_t *pDxeCCB = NULL;
    pDxeCCB = &(halDxe->DxeCCB[channel]);
    pDxeCCB->int_cnt++;
}

/*
 * @brief  : Dxe Descriptors reconfiguration based on the Transfer type H2B / B2H.
 *           Note: This function should be invoked only when the s/w is
 *           about to enter sleep mode.
 * @param  : channel - Dxe Channel
 * @return : None
 */
void __attribute__((section(".after_ram_vectors"))) hal_dxe_desc_reconfig(e_dxe_channel channel)
{
    uint8_t desc_idx;
    DescCB_t *pCurrDCB;
    volatile DXEDesc_t *pDXEDesc;
    volatile DxeCCB_t *pDxeCCB;
    volatile uint32_t desc_ctrl;
    pDxeCCB = &(halDxe->DxeCCB[channel]);
    pCurrDCB = pDxeCCB->pRingFreeHead;

    /* If the channel is not configured, return */
    if (!pDxeCCB->chConfigured) {
        return;
    }

    /* If B2H channel, point the freehead to first desc_ctrl_valid bit set to 0,
     * as the buffers will be available in s/w memory to process post wakeup
     * Explanation:
     * -----------
     *     Whenever there is a channel abort request made and if transfer is ongoing
     *     DXE will abort the transfer or completes the transfer and set the desc_ctrl_valid bit to 0
     *     but wont raise an interrupt.
     *     In such cases, freehead will point to invalid buffer and post wakeup, Rx will stall.
     */
    if (NT_DXE_XFR_BMU_TO_HOST == pDxeCCB->xfrType) {
        for (desc_idx = 0; desc_idx < pDxeCCB->nDescs; desc_idx++) {
            pDXEDesc = (DXEDesc_t *)pCurrDCB->DXEDescAddr;
            desc_ctrl = (volatile uint32_t)pDXEDesc->ctrl;
            if ((desc_ctrl & NT_SA_DXE_DESC_CTRL_VALID) == 0) {
                pDxeCCB->pRingFreeHead = pCurrDCB;
                if (desc_idx)
                    NT_LOG_PRINT(DPM, ERR, "DXE CHAN %d pRingFreeHead modified %d", channel, desc_idx);
                break;
            }
            pCurrDCB = pCurrDCB->next;
        }
    }
    /* If H2B channel, desc_ctrl_valid bit should be set to 0, for all the decriptors */
    else if (NT_DXE_XFR_HOST_TO_BMU == pDxeCCB->xfrType) {
        for (desc_idx = 0; desc_idx < pDxeCCB->nDescs; desc_idx++) {
            pDXEDesc = (DXEDesc_t *)pCurrDCB->DXEDescAddr;
            desc_ctrl = (volatile uint32_t)pDXEDesc->ctrl;
            if (desc_ctrl & NT_SA_DXE_DESC_CTRL_VALID) {
                desc_ctrl = desc_ctrl & (~(NT_SA_DXE_DESC_CTRL_VALID));
                pDXEDesc->ctrl = desc_ctrl;
            }
            pCurrDCB = pCurrDCB->next;
        }
    }
    /* No Handling required for H2H transfer channels */
    else {
        NT_LOG_PRINT(DPM, ERR, "Invalid Dxe channel %d for reconfiguration", channel);
    }
}
