/*
* Copyright (c) 2024, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     encode_check_huc_load_packet.cpp
//! \brief    
//!

#include "encode_check_huc_load_packet.h"

namespace encode
{
    MOS_STATUS EncodeCheckHucLoadPkt::Init()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_NULL_RETURN(m_allocator);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
        allocParamsForBufferLinear.dwBytes = sizeof(uint64_t);
        allocParamsForBufferLinear.pBufName = "Huc authentication status Buffer";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        PMOS_RESOURCE allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_hucAuthBuf = allocatedbuffer;

        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            // VDENC uses second level batch buffer
            MOS_ZeroMemory(&m_2ndLevelBB[j], sizeof(MHW_BATCH_BUFFER));
            m_2ndLevelBB[j].bSecondLevel = true;
            ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_hwInterface->GetOsInterface(),
                &m_2ndLevelBB[j],
                nullptr,
                CODECHAL_CACHELINE_SIZE));
        }

        return MOS_STATUS_SUCCESS;
    }

    EncodeCheckHucLoadPkt::~EncodeCheckHucLoadPkt()
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            MOS_STATUS eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_2ndLevelBB[j], nullptr);
            ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);
        }
    }

    MOS_STATUS EncodeCheckHucLoadPkt::AddForceWakeup(MOS_COMMAND_BUFFER *cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        auto &forceWakeupParams                     = m_miItf->MHW_GETPAR_F(MI_FORCE_WAKEUP)();
        forceWakeupParams                           = {};
        forceWakeupParams.bMFXPowerWellControl      = true;
        forceWakeupParams.bMFXPowerWellControlMask  = true;
        forceWakeupParams.bHEVCPowerWellControl     = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;

        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FORCE_WAKEUP)(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeCheckHucLoadPkt::SendPrologCmds(MOS_COMMAND_BUFFER *cmdBuffer)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_FUNC_CALL();
        bool mmcEnabled = false;
#ifdef _MMC_SUPPORTED
        EncodeMemComp *mmcState = m_pipeline->GetMmcState();
        ENCODE_CHK_NULL_RETURN(mmcState);
        mmcEnabled = mmcState->IsMmcEnabled();
        ENCODE_CHK_STATUS_RETURN(mmcState->SendPrologCmd(cmdBuffer, false));
#endif

        MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
        MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        genericPrologParams.pOsInterface = m_hwInterface->GetOsInterface();
        genericPrologParams.pvMiInterface = nullptr;
        genericPrologParams.bMmcEnabled   = mmcEnabled;
        ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(cmdBuffer, &genericPrologParams, m_miItf));

        return eStatus;
    }

    MOS_STATUS EncodeCheckHucLoadPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        // add media reset check 100ms, which equals to 1080p WDT threshold
        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(1920, 1080, true));

        bool firstTaskInPhase = packetPhase & firstPacket;
        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            ENCODE_CHK_STATUS_RETURN(AddForceWakeup(commandBuffer));
            // Send command buffer header at the beginning (OS dependent)
            ENCODE_CHK_STATUS_RETURN(SendPrologCmds(commandBuffer));
        }

        // program 2nd level chained BB for Huc auth
        m_batchBuf = &m_2ndLevelBB[m_pipeline->m_currRecycledBufIdx];
        ENCODE_CHK_NULL_RETURN(m_batchBuf);
        uint8_t *data = (uint8_t *)m_allocator->LockResourceForWrite(&(m_batchBuf->OsResource));
        ENCODE_CHK_NULL_RETURN(data);
        
        MOS_COMMAND_BUFFER hucAuthCmdBuffer = {};
        hucAuthCmdBuffer.pCmdBase   = (uint32_t *)data;
        hucAuthCmdBuffer.pCmdPtr    = hucAuthCmdBuffer.pCmdBase;
        hucAuthCmdBuffer.iRemaining = m_batchBuf->iSize;
        hucAuthCmdBuffer.OsResource = m_batchBuf->OsResource;
        hucAuthCmdBuffer.cmdBuf1stLvl = commandBuffer;

        //pak check huc status command
        ENCODE_CHK_STATUS_RETURN(PackHucAuthCmds(hucAuthCmdBuffer));

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(&(m_batchBuf->OsResource)));

        // BB start for 2nd level BB
        auto &miBatchBufferStartParams = m_miItf->MHW_GETPAR_F(MI_BATCH_BUFFER_START)();
        miBatchBufferStartParams       = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(commandBuffer, m_batchBuf));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeCheckHucLoadPkt::PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hucItf);
        auto mmioRegisters = m_hucItf->GetMmioRegisters(m_vdboxIndex);
        ENCODE_CHK_NULL_RETURN(mmioRegisters);

        // Write HuC Load Info Mask
        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_hucAuthBuf;
        storeDataParams.dwResourceOffset = 0;
        storeDataParams.dwValue          = m_hucLoadInfoMask;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

        // Store Huc Auth register
        auto &miStoreRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        miStoreRegMemParams                 = {};
        miStoreRegMemParams.presStoreBuffer = m_hucAuthBuf;
        miStoreRegMemParams.dwOffset        = sizeof(uint32_t);
        miStoreRegMemParams.dwRegister      = mmioRegisters->hucLoadInfoOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(&cmdBuffer));

        // Flush the engine to ensure memory written out
        auto &flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams       = {};
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));

        // Check Huc load status: if equals to 0 continue chained BB until reset, otherwise send BB end cmd.
        auto &conditionalBatchBufferEndParams                          = m_miItf->MHW_GETPAR_F(MI_CONDITIONAL_BATCH_BUFFER_END)();
        conditionalBatchBufferEndParams                                = {};
        conditionalBatchBufferEndParams.presSemaphoreBuffer            = m_hucAuthBuf;
        conditionalBatchBufferEndParams.dwOffset                       = 0;
        conditionalBatchBufferEndParams.dwValue                        = 0;
        conditionalBatchBufferEndParams.bDisableCompareMask            = false;
        conditionalBatchBufferEndParams.dwParamsType                   = mhw::mi::MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS;
        conditionalBatchBufferEndParams.enableEndCurrentBatchBuffLevel = true;
        conditionalBatchBufferEndParams.compareOperation               = COMPARE_OPERATION_MADEQUALIDD;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_CONDITIONAL_BATCH_BUFFER_END)(&cmdBuffer));

        // Chained BB loop
        auto &miBatchBufferStartParams                  = m_miItf->MHW_GETPAR_F(MI_BATCH_BUFFER_START)();
        miBatchBufferStartParams                        = {};
        miBatchBufferStartParams.secondLevelBatchBuffer = false;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_BATCH_BUFFER_START)(&cmdBuffer, m_batchBuf));

        return MOS_STATUS_SUCCESS;
    }
}
