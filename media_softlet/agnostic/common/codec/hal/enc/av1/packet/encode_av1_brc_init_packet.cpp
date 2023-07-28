/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_av1_brc_init_packet.cpp
//! \brief    Defines the interface for av1 brc init/reset packet
//!
#include "encode_av1_brc_init_packet.h"
#include "mhw_vdbox.h"

namespace encode {
    MOS_STATUS Av1BrcInitPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());
        ENCODE_CHK_NULL_RETURN(m_featureManager);

        m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcInitPkt::AllocateResources()
    {
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        // initiate allocation paramters
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        MOS_RESOURCE *allocatedbuffer;
        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            // BRC init/reset DMEM
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC BrcInit DmemBuffer";
            allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencBrcInitDmemBuffer[k] = *allocatedbuffer;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcInitPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;
        auto brcFeature = dynamic_cast<Av1Brc*>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        uint16_t pictureType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 0 : (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? 1 : 3) : 2);
        SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET, (uint16_t)m_basicFeature->m_mode, pictureType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }
        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, brcFeature->IsBRCResetRequired()? BRC_RESET : BRC_INIT));

        // Disable Brc Init/reset  here after init cmd executed, APP will re-trigger the reset by DDI params seqParams->resetBRC
        RUN_FEATURE_INTERFACE_NO_RETURN(Av1Brc, Av1FeatureIDs::av1BrcFeature,
            DisableBrcInitReset);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcInitPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Av1BrcInitPkt::DumpInput()
    {
        ENCODE_FUNC_CALL();
        int32_t currentPass = m_pipeline->GetCurrentPass();

        CodechalDebugInterface* debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            &m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx],
            m_vdencBrcInitDmemBufferSize,
            currentPass,
            hucRegionDumpInit));

        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_BrcHistory", true, hucRegionDumpInit, 6080));
        return MOS_STATUS_SUCCESS;
    }
#endif

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, Av1BrcInitPkt)
    {
        params.kernelDescriptor = m_vdboxHucAv1BrcInitKernelDescriptor;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, Av1BrcInitPkt)
    {
        params.function      = BRC_INIT;
        params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx]);
        params.dataLength    = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, Av1BrcInitPkt)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

        params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, 0);
        params.regionParams[0].isWritable = true;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VD_PIPELINE_FLUSH, Av1BrcInitPkt)
    {
        params.waitDoneVDCmdMsgParser = true;
        params.waitDoneAV1 = true;
        params.flushAV1 = true;

        return MOS_STATUS_SUCCESS;
    }
}
