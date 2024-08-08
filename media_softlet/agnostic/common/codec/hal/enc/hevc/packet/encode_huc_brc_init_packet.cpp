/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     encode_huc_brc_init_packet.cpp
//! \brief    Defines the interface for huc brc init/reset packet
//!
#include "encode_huc_brc_init_packet.h"
#include "mhw_vdbox.h"
#include "encode_hevc_brc.h"
#include "encode_hevc_vdenc_lpla_enc.h"

namespace encode {
    MOS_STATUS HucBrcInitPkt::Init()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::Init());
        ENCODE_CHK_NULL_RETURN(m_featureManager);

        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcInitPkt::AllocateResources()
    {
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        // initiate allocation paramters and lock flags
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
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

    MOS_STATUS HucBrcInitPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_STATUS_RETURN(m_miItf->SetWatchdogTimerThreshold(m_basicFeature->m_frameWidth, m_basicFeature->m_frameHeight, true));
        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        SetPerfTag(CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET, (uint16_t)m_basicFeature->m_mode, m_basicFeature->m_pictureCodingType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }
        auto brcFeature = dynamic_cast<HEVCEncodeBRC*>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);
        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_INIT));

        // Disable Brc Init/reset  here after init cmd executed, APP will re-trigger the reset by DDI params m_hevcSeqParams->bResetBRC
        RUN_FEATURE_INTERFACE_NO_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature,
            DisableBrcInitReset);

        CODECHAL_DEBUG_TOOL
        (
            ENCODE_CHK_STATUS_RETURN(DumpHucBrcInit());
        )

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HucBrcInitPkt)
    {
        params.kernelDescriptor = m_vdboxHucHevcBrcInitKernelDescriptor;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucBrcInitPkt)
    {
        ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

        params.function      = BRC_INIT;
        params.hucDataSource = const_cast<PMOS_RESOURCE>(&m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx]);
        params.dataLength    = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucBrcInitPkt)
    {
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

        params.function                   = BRC_INIT;
        params.regionParams[0].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencBRCHistoryBuffer, m_basicFeature->m_frameNum);
        params.regionParams[0].isWritable = true;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcInitPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        stateCmdSizeParams.uNumMfxWait      = 3;
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcInitPkt::SetDmemBuffer() const
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Setup BrcInit DMEM
        auto hucVdencBrcInitDmem = (VdencHevcHucBrcInitDmem *)
        m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx]));
        ENCODE_CHK_NULL_RETURN(hucVdencBrcInitDmem);
        MOS_ZeroMemory(hucVdencBrcInitDmem, sizeof(VdencHevcHucBrcInitDmem));

        bool enableTileReplay = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsTileReplayEnabled, enableTileReplay);

        hucVdencBrcInitDmem->BRCFunc_U32 = enableTileReplay ? 1 : 0 << 7;  //bit0 0: Init; 1: Reset, bit7 0: frame-based; 1: tile-based
        hucVdencBrcInitDmem->InitBufFull_U32   = MOS_MIN(m_basicFeature->m_hevcSeqParams->InitVBVBufferFullnessInBit, m_basicFeature->m_hevcSeqParams->VBVBufferSizeInBit);
        hucVdencBrcInitDmem->BufSize_U32       = m_basicFeature->m_hevcSeqParams->VBVBufferSizeInBit;
        hucVdencBrcInitDmem->MinRate_U32 = 0;
        hucVdencBrcInitDmem->FrameRateM_U32    = m_basicFeature->m_hevcSeqParams->FrameRate.Numerator;
        hucVdencBrcInitDmem->FrameRateD_U32    = m_basicFeature->m_hevcSeqParams->FrameRate.Denominator;
        hucVdencBrcInitDmem->ACQP_U32          = 0;

        auto CalculatedMaxFrame                         = m_basicFeature->GetProfileLevelMaxFrameSize();
        hucVdencBrcInitDmem->UserMaxFrame               = m_basicFeature->m_hevcSeqParams->UserMaxIFrameSize > 0 ? MOS_MIN(m_basicFeature->m_hevcSeqParams->UserMaxIFrameSize, CalculatedMaxFrame) : CalculatedMaxFrame;
        hucVdencBrcInitDmem->ProfileLevelMaxFramePB_U32 = m_basicFeature->m_hevcSeqParams->UserMaxPBFrameSize > 0 ? MOS_MIN(m_basicFeature->m_hevcSeqParams->UserMaxPBFrameSize, CalculatedMaxFrame) : CalculatedMaxFrame;

        hucVdencBrcInitDmem->SSCFlag = m_basicFeature->m_hevcSeqParams->SliceSizeControl;

        // LDB case, NumP=0 & NumB=100, but GopP=100 & GopB=0

        hucVdencBrcInitDmem->FrameWidth_U16 = (uint16_t)m_basicFeature->m_oriFrameWidth;
        hucVdencBrcInitDmem->FrameHeight_U16 = (uint16_t)m_basicFeature->m_oriFrameHeight;

        hucVdencBrcInitDmem->MinQP_U8 = m_basicFeature->m_hevcPicParams->BRCMinQp < 10 ? 10 : m_basicFeature->m_hevcPicParams->BRCMinQp;                                           // Setting values from arch spec
        hucVdencBrcInitDmem->MaxQP_U8 = m_basicFeature->m_hevcPicParams->BRCMaxQp < 10 ? 51 : (m_basicFeature->m_hevcPicParams->BRCMaxQp > 51 ? 51 : m_basicFeature->m_hevcPicParams->BRCMaxQp);   // Setting values from arch spec

        hucVdencBrcInitDmem->BRCPyramidEnable_U8 = 0;

        //QP modulation settings
        m_basicFeature->m_hevcSeqParams->GopRefDist = 
                        m_basicFeature->m_hevcSeqParams->GopRefDist == 0 ? 1 : 
                        m_basicFeature->m_hevcSeqParams->GopRefDist;
        bool bAllowedPyramid = m_basicFeature->m_hevcSeqParams->GopRefDist != 3;
        uint16_t intraPeriod = m_basicFeature->m_hevcSeqParams->GopPicSize > 4001 ? 4000 : m_basicFeature->m_hevcSeqParams->GopPicSize - 1;
        intraPeriod = ((intraPeriod + m_basicFeature->m_hevcSeqParams->GopRefDist - 1) / m_basicFeature->m_hevcSeqParams->GopRefDist) * m_basicFeature->m_hevcSeqParams->GopRefDist;
        if (m_basicFeature->m_hevcSeqParams->HierarchicalFlag && bAllowedPyramid)
        {
            hucVdencBrcInitDmem->GopP_U16 = intraPeriod/m_basicFeature->m_hevcSeqParams->GopRefDist;
            hucVdencBrcInitDmem->GopB_U16 = (hucVdencBrcInitDmem->GopP_U16)*(m_basicFeature->m_hevcSeqParams->GopRefDist>1);
            hucVdencBrcInitDmem->GopB1_U16 = hucVdencBrcInitDmem->GopP_U16 * ((m_basicFeature->m_hevcSeqParams->GopRefDist > 2) + (m_basicFeature->m_hevcSeqParams->GopRefDist == 4 || m_basicFeature->m_hevcSeqParams->GopRefDist > 5));
            hucVdencBrcInitDmem->GopB2_U16 = (intraPeriod - hucVdencBrcInitDmem->GopP_U16 - hucVdencBrcInitDmem->GopB_U16 - hucVdencBrcInitDmem->GopB1_U16) * (m_basicFeature->m_hevcSeqParams->GopRefDist > 3);
            hucVdencBrcInitDmem->MaxBRCLevel_U8      = hucVdencBrcInitDmem->GopB1_U16 == 0 ? HEVC_BRC_FRAME_TYPE_B : (hucVdencBrcInitDmem->GopB2_U16 == 0 ? HEVC_BRC_FRAME_TYPE_B1 : HEVC_BRC_FRAME_TYPE_B2);
            hucVdencBrcInitDmem->BRCPyramidEnable_U8 = 1;
        }
        else //FlatB or LDB
        {
            hucVdencBrcInitDmem->GopP_U16 = intraPeriod/m_basicFeature->m_hevcSeqParams->GopRefDist;
            hucVdencBrcInitDmem->GopB_U16 = intraPeriod - hucVdencBrcInitDmem->GopP_U16;
            hucVdencBrcInitDmem->MaxBRCLevel_U8 = hucVdencBrcInitDmem->GopB_U16 == 0 ? HEVC_BRC_FRAME_TYPE_P_OR_LB : HEVC_BRC_FRAME_TYPE_B;
        }

        hucVdencBrcInitDmem->LumaBitDepth_U8   = m_basicFeature->m_hevcSeqParams->bit_depth_luma_minus8 + 8;
        hucVdencBrcInitDmem->ChromaBitDepth_U8 = m_basicFeature->m_hevcSeqParams->bit_depth_chroma_minus8 + 8;

        if (m_basicFeature->m_hevcSeqParams->SourceBitDepth == ENCODE_HEVC_BIT_DEPTH_10)
        {
            hucVdencBrcInitDmem->LumaBitDepth_U8 = 10;
            hucVdencBrcInitDmem->ChromaBitDepth_U8 = 10;
        }

        RUN_FEATURE_INTERFACE_RETURN(HevcVdencRoi, HevcFeatureIDs::hevcVdencRoiFeature, SetDmemHuCBrcInitReset, hucVdencBrcInitDmem);

        if ((m_basicFeature->m_hevcSeqParams->SlidingWindowSize != 0) && (m_basicFeature->m_hevcSeqParams->MaxBitRatePerSlidingWindow != 0))
        {
            if (m_basicFeature->m_hevcSeqParams->TargetBitRate == 0)
            {
                ENCODE_ASSERTMESSAGE("TargetBitRate is zero!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            hucVdencBrcInitDmem->SlidingWindow_Size_U32     = MOS_MIN((uint32_t)m_basicFeature->m_hevcSeqParams->SlidingWindowSize, 60);
            hucVdencBrcInitDmem->SLIDINGWINDOW_MaxRateRatio = (uint8_t)((uint64_t)m_basicFeature->m_hevcSeqParams->MaxBitRatePerSlidingWindow * 100 / m_basicFeature->m_hevcSeqParams->TargetBitRate);
        }
        else
        {
            if (m_basicFeature->m_hevcSeqParams->FrameRate.Denominator == 0)
            {
                ENCODE_ASSERTMESSAGE("FrameRate.Deminator is zero!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            uint32_t framerate = m_basicFeature->m_hevcSeqParams->FrameRate.Numerator / m_basicFeature->m_hevcSeqParams->FrameRate.Denominator;
            hucVdencBrcInitDmem->SlidingWindow_Size_U32 = MOS_MIN(framerate, 60);
            hucVdencBrcInitDmem->SLIDINGWINDOW_MaxRateRatio = 120;
        }

        // Tile Row based BRC 
        if (enableTileReplay)
        {
            hucVdencBrcInitDmem->SlideWindowRC    = 0; //Reserved for now
            hucVdencBrcInitDmem->MaxLogCUSize     = m_basicFeature->m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
            hucVdencBrcInitDmem->FrameWidthInLCU  = (m_basicFeature->m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) >> (m_basicFeature->m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_basicFeature->m_hevcSeqParams->log2_min_coding_block_size_minus3);
            hucVdencBrcInitDmem->FrameHeightInLCU = (m_basicFeature->m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) >> (m_basicFeature->m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_basicFeature->m_hevcSeqParams->log2_min_coding_block_size_minus3);
        }

        // Long term reference
        hucVdencBrcInitDmem->LongTermRefEnable_U8  = true;
        hucVdencBrcInitDmem->LongTermRefMsdk_U8 = true;
        hucVdencBrcInitDmem->IsLowDelay_U8 = m_basicFeature->m_ref.IsLowDelay();

        hucVdencBrcInitDmem->LookaheadDepth_U8 = m_basicFeature->m_hevcSeqParams->LookaheadDepth;
        RUN_FEATURE_INTERFACE_RETURN(HEVCVdencLplaEnc, HevcFeatureIDs::hevcVdencLplaEncFeature,
            SetDmemForInit, hucVdencBrcInitDmem);

        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature,
            SetDmemForInit, hucVdencBrcInitDmem);

        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx])));

        return eStatus;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HucBrcInitPkt::DumpHucBrcInit()
    {
        ENCODE_FUNC_CALL();
        int32_t currentPass = m_pipeline->GetCurrentPass();

        HevcBasicFeature *hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcBasicFeature);

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
            &m_vdencBrcInitDmemBuffer[m_pipeline->m_currRecycledBufIdx],
            m_vdencBrcInitDmemBufferSize,
            currentPass,
            hucRegionDumpInit));

        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_History", true, hucRegionDumpInit));

        return MOS_STATUS_SUCCESS;
    }
#endif
}
