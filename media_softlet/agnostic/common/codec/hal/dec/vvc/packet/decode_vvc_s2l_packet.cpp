/*
* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     decode_vvc_s2l_packet.cpp
//! \brief    Defines the implementation of VVC decode S2L packet
//!
#include "decode_vvc_s2l_packet.h"
#include "decode_resource_auto_lock.h"
#include "mos_os_cp_interface_specific.h"

namespace decode
{
    MOS_STATUS VvcDecodeS2LPkt::AllocateResources()
    {
        m_sliceBsParamNumber = m_vvcBasicFeature->m_numSlices;
        m_sliceBsParamOffset = 0;  

        m_sliceBbParamNumber = m_vvcBasicFeature->m_numSlices;
        m_sliceBbParamOffset = MOS_ALIGN_CEIL((m_sliceBsParamNumber * sizeof(VvcS2lSliceBsParam) + m_sliceBsParamOffset), 4);

        if(m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            if (m_vvcPicParams->m_spsNumSubpicsMinus1 > 0 && m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag)
            {
                m_isMultiSubPicParam = true; //Rect scan mode and SubpicsMinus1 > 0 and SubpicInfoPresentFlag enabled, means subpic params valid.
            }
        }
        if (!m_isMultiSubPicParam)
        {
            m_subPicParamNumber = 0;
        }
        else
        {
            m_subPicParamNumber = m_vvcBasicFeature->m_vvcPicParams->m_spsNumSubpicsMinus1 + 1;
        }
        m_subPicParamOffset = MOS_ALIGN_CEIL((m_sliceBbParamNumber * sizeof(VvcS2lSliceBbParam) + m_sliceBbParamOffset), 4);

        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag) //Rect scan mode
        {
            if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag)
            {
                if (m_vvcPicParams->m_spsNumSubpicsMinus1 == 0 || !m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag)
                {
                    m_slicePartitionParamNumber = 1;
                }
                else
                {
                    m_slicePartitionParamNumber = m_vvcPicParams->m_spsNumSubpicsMinus1 + 1;  //have subpic param.
                }
            }
            else
            {
                m_slicePartitionParamNumber = m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1;
            }
        }
        else //Raster scan mode
        {
            m_slicePartitionParamNumber = m_vvcBasicFeature->m_numSlices;
        }

        m_sliceParamDynamicSize = m_subPicParamNumber * sizeof(VvcS2lSubpicParam) + m_sliceBsParamNumber * sizeof(VvcS2lSliceBsParam) + m_sliceBbParamNumber * sizeof(VvcS2lSliceBbParam);
        DECODE_CHK_NULL(m_allocator);
        m_dmemBufferSize = MOS_ALIGN_CEIL(sizeof(VvcS2lBss) + m_sliceParamDynamicSize, CODECHAL_CACHELINE_SIZE);
        if (m_vvcS2lDmemBufferArray == nullptr)
        {
            m_vvcS2lDmemBufferArray = m_allocator->AllocateBufferArray(m_dmemBufferSize, 
                "VVCDmemBuffer",
                CODECHAL_VVC_NUM_DMEM_BUFFERS,
                resourceInternalReadWriteCache,
                lockableVideoMem);
            DECODE_CHK_NULL(m_vvcS2lDmemBufferArray);
            PMOS_BUFFER &buf = m_vvcS2lDmemBufferArray->Fetch();
            DECODE_CHK_NULL(buf);
        }
        else
        {
            PMOS_BUFFER &buf = m_vvcS2lDmemBufferArray->Fetch();
            DECODE_CHK_NULL(buf);
            DECODE_CHK_STATUS(m_allocator->Resize(buf, m_dmemBufferSize, lockableVideoMem));
        }
        if (m_vvcS2lExtraBufferArray == nullptr)
        {
            m_vvcS2lExtraBufferArray = m_allocator->AllocateBufferArray(sizeof(VvcS2lExtraBss),
                "VVCExtraDataBuffer",
                CODECHAL_VVC_NUM_DMEM_BUFFERS,
                resourceInternalReadWriteCache,
                lockableVideoMem);
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::Destroy()
    {
        if (m_allocator && m_vvcS2lDmemBufferArray)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_vvcS2lDmemBufferArray));
        }
        if (m_allocator && m_vvcS2lExtraBufferArray)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_vvcS2lExtraBufferArray));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::Submit(
        MOS_COMMAND_BUFFER *cmdBuffer,
        uint8_t             packetPhase)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(cmdBuffer);
        // Send prolog since S2L packet always be first packet
        bool requestProlog = true;
        DECODE_CHK_STATUS(Execute(*cmdBuffer, requestProlog));

        CODECHAL_DEBUG_TOOL(
            DECODE_CHK_STATUS(DumpHucS2l());)

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_vvcPipeline);
        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_hucItf);
        DECODE_CHK_NULL(m_miItf);
        DECODE_CHK_NULL(m_vdencItf);
        m_basicFeature = dynamic_cast<DecodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_basicFeature);
        m_vvcBasicFeature = dynamic_cast<VvcBasicFeature *>(m_basicFeature);
        DECODE_CHK_NULL(m_vvcBasicFeature);

        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        stateCmdSizeParams.bShortFormat = true;
        DECODE_CHK_STATUS(m_hwInterface->GetHucStateCommandSize(m_vvcBasicFeature->m_mode, &m_pictureStatesSize, &m_picturePatchListSize, &stateCmdSizeParams));
        uint32_t cpCmdsize       = 0;
        uint32_t cpPatchListSize = 0;
        m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
        m_sliceStatesSize += cpCmdsize;
        m_slicePatchListSize += cpPatchListSize;

        CalculateVvcSliceLvlCmdSize();

        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS VvcDecodeS2LPkt::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_STATUS(AddCmd_HUC_IMEM_STATE(cmdBuffer));
        DECODE_CHK_STATUS(AddCmd_HUC_PIPE_MODE_SELECT(cmdBuffer));
        SETPAR_AND_ADDCMD(HUC_IND_OBJ_BASE_ADDR_STATE, m_hucItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, &cmdBuffer);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &par = m_vdencItf->GETPAR_VD_PIPELINE_FLUSH();
        par                        = {};
        par.waitDoneHEVC           = 1;
        par.flushHEVC              = 1;
        par.waitDoneVDCmdMsgParser = 1;
        m_vdencItf->ADDCMD_VD_PIPELINE_FLUSH(&cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        for (uint32_t i = 0; i < m_vvcBasicFeature->m_numSlices; i++)
        {
            //add CP huc State
            DECODE_CHK_STATUS(AddHucCpState(cmdBuffer, i, m_vvcSliceParams[i]));
            int32_t deltaSize = m_vvcBasicFeature->m_dataOffset + m_vvcBasicFeature->m_dataSize - (m_vvcSliceParams[i].m_bSNALunitDataLocation + m_vvcSliceParams[i].m_sliceBytesInBuffer);
            if (deltaSize <= 0)
            {
                m_tailingBsReadSize = 0;  //Cannot read more bytes if last slice
            }
            else
            {
                m_tailingBsReadSize = deltaSize > 10 ? 10 : deltaSize;
            }
            DECODE_CHK_STATUS(AddCmd_HUC_STREAM_OBJECT(cmdBuffer, m_vvcSliceParams[i]));
            if (i == (m_vvcBasicFeature->m_numSlices - 1))
            {
                DECODE_CHK_STATUS(StoreHucStatus2Register(cmdBuffer));
                DECODE_CHK_STATUS(AddCmd_HUC_START(cmdBuffer, true));
            }
            else
            {
                DECODE_CHK_STATUS(AddCmd_HUC_START(cmdBuffer, false));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::Prepare()
    {
        DECODE_FUNC_CALL();

        m_vvcPicParams = m_vvcBasicFeature->m_vvcPicParams;
        DECODE_CHK_NULL(m_vvcPicParams);
        m_vvcSliceParams = m_vvcBasicFeature->m_vvcSliceParams;
        DECODE_CHK_NULL(m_vvcSliceParams);

        DECODE_CHK_STATUS(AllocateResources());

        DECODE_CHK_STATUS(SetDmemBuffer());
        DECODE_CHK_STATUS(SetExtraDataBuffer());

        SetHucStatusMask(GetHucStatusVvcS2lFailureMask(),
            m_hucItf->GetHucStatus2ImemLoadedMask());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize      = CalculateCommandBufferSize();
        requestedPatchListSize = CalculatePatchListSize();

        return MOS_STATUS_SUCCESS;
    }

    uint32_t VvcDecodeS2LPkt::CalculateCommandBufferSize()
    {
        DECODE_FUNC_CALL();

        uint32_t commandBufferSize = m_pictureStatesSize +
                                     m_sliceStatesSize * (m_vvcBasicFeature->m_numSlices + 1);

        return (commandBufferSize + COMMAND_BUFFER_RESERVED_SPACE);
    }

    uint32_t VvcDecodeS2LPkt::CalculatePatchListSize()
    {
        DECODE_FUNC_CALL();

        if (!m_osInterface->bUsesPatchList)
        {
            return 0;
        }

        uint32_t requestedPatchListSize = m_picturePatchListSize +
                                          m_slicePatchListSize * (m_vvcBasicFeature->m_numSlices + 1);

        return requestedPatchListSize;
    }

    MOS_STATUS VvcDecodeS2LPkt::SetHucDmemPictureBss(HucVvcS2lPicBss &hucVvcS2LPicBss)
    {
        DECODE_FUNC_CALL();
        // SPS info
        hucVvcS2LPicBss.m_spsPicWidthMaxInLumaSamples  = m_vvcPicParams->m_spsPicWidthMaxInLumaSamples;   // [8..16888]
        hucVvcS2LPicBss.m_spsPicHeightMaxInLumaSamples = m_vvcPicParams->m_spsPicHeightMaxInLumaSamples;  // [8..16888]

        hucVvcS2LPicBss.m_spsNumSubpicsMinus1                 = m_vvcPicParams->m_spsNumSubpicsMinus1;                  // [0..599]
        hucVvcS2LPicBss.m_spsSubpicIdLenMinus1                = m_vvcPicParams->m_spsSubpicIdLenMinus1;                 // [0..15]
        hucVvcS2LPicBss.m_spsChromaFormatIdc                  = m_vvcPicParams->m_spsChromaFormatIdc;                   // [1]
        hucVvcS2LPicBss.m_spsBitdepthMinus8                   = m_vvcPicParams->m_spsBitdepthMinus8;                    // [0..2]
        hucVvcS2LPicBss.m_spsLog2CtuSizeMinus5                = m_vvcPicParams->m_spsLog2CtuSizeMinus5;                 // [0..2]
        hucVvcS2LPicBss.m_spsLog2MaxPicOrderCntLsbMinus4      = m_vvcPicParams->m_spsLog2MaxPicOrderCntLsbMinus4;       // [0..12]
        hucVvcS2LPicBss.m_spsLog2MinLumaCodingBlockSizeMinus2 = m_vvcPicParams->m_spsLog2MinLumaCodingBlockSizeMinus2;  // [0..4]
        hucVvcS2LPicBss.m_spsPocMsbCycleLenMinus1             = m_vvcPicParams->m_spsPocMsbCycleLenMinus1;              // [0..27]
        hucVvcS2LPicBss.m_numExtraPhBits                      = m_vvcPicParams->m_numExtraPhBits;                       // [0..15]
        hucVvcS2LPicBss.m_numExtraShBits                      = m_vvcPicParams->m_numExtraShBits;                       // [0..15]
        hucVvcS2LPicBss.m_spsLog2TransformSkipMaxSizeMinus2   = m_vvcPicParams->m_spsLog2TransformSkipMaxSizeMinus2;    // [0..3]

        for (uint32_t i = 0; i < 3; i++)
        {
            for (uint32_t j = 0; j < 76; j++)
            {
                hucVvcS2LPicBss.m_chromaQpTable[i][j] = m_vvcPicParams->m_chromaQpTable[i][j];  // [-12..63]
            }
        }
        
        hucVvcS2LPicBss.m_spsNumRefPicLists[0]                 = m_vvcPicParams->m_spsNumRefPicLists[0];                  // [0..64]
        hucVvcS2LPicBss.m_spsNumRefPicLists[1]                 = m_vvcPicParams->m_spsNumRefPicLists[1];                  // [0..64]
        hucVvcS2LPicBss.m_spsSixMinusMaxNumMergeCand           = m_vvcPicParams->m_spsSixMinusMaxNumMergeCand;            // [0..5]
        hucVvcS2LPicBss.m_spsFiveMinusMaxNumSubblockMergeCand  = m_vvcPicParams->m_spsFiveMinusMaxNumSubblockMergeCand;   // [0..5]
        hucVvcS2LPicBss.m_spsMaxNumMergeCandMinusMaxNumGpmCand = m_vvcPicParams->m_spsMaxNumMergeCandMinusMaxNumGpmCand;  // [0..4]
        hucVvcS2LPicBss.m_spsLog2ParallelMergeLevelMinus2      = m_vvcPicParams->m_spsLog2ParallelMergeLevelMinus2;       // [0..5]
        hucVvcS2LPicBss.m_spsMinQpPrimeTs                      = m_vvcPicParams->m_spsMinQpPrimeTs;                       // [0..8]
        hucVvcS2LPicBss.m_spsSixMinusMaxNumIbcMergeCand        = m_vvcPicParams->m_spsSixMinusMaxNumIbcMergeCand;         // [0..5]
        hucVvcS2LPicBss.m_spsNumLadfIntervalsMinus2            = m_vvcPicParams->m_spsNumLadfIntervalsMinus2;             // [0..3]
        hucVvcS2LPicBss.m_spsLadfLowestIntervalQpOffset        = m_vvcPicParams->m_spsLadfLowestIntervalQpOffset;         // [-63..63]
        for (uint32_t i = 0; i < 4; i++)
        {
            hucVvcS2LPicBss.m_spsLadfQpOffset[i]             = m_vvcPicParams->m_spsLadfQpOffset[i];
            hucVvcS2LPicBss.m_spsLadfDeltaThresholdMinus1[i] = m_vvcPicParams->m_spsLadfDeltaThresholdMinus1[i];
        }
        hucVvcS2LPicBss.m_spsNumVerVirtualBoundaries            = m_vvcPicParams->m_spsNumVerVirtualBoundaries;             // [0..3]
        hucVvcS2LPicBss.m_spsNumHorVirtualBoundaries            = m_vvcPicParams->m_spsNumHorVirtualBoundaries;             // [0..3]
        hucVvcS2LPicBss.m_spsLog2DiffMinQtMinCbIntraSliceLuma   = m_vvcPicParams->m_spsLog2DiffMinQtMinCbIntraSliceLuma;    // [0..4]
        hucVvcS2LPicBss.m_spsMaxMttHierarchyDepthIntraSliceLuma = m_vvcPicParams->m_spsMaxMttHierarchyDepthIntraSliceLuma;  // [0..10]
        for (uint32_t i = 0; i < 3; i++)
        {
            hucVvcS2LPicBss.m_spsVirtualBoundaryPosXMinus1[i] = m_vvcPicParams->m_spsVirtualBoundaryPosXMinus1[i];
            hucVvcS2LPicBss.m_spsVirtualBoundaryPosYMinus1[i] = m_vvcPicParams->m_spsVirtualBoundaryPosYMinus1[i];
        }
        hucVvcS2LPicBss.m_spsLog2DiffMaxBtMinQtIntraSliceLuma     = m_vvcPicParams->m_spsLog2DiffMaxBtMinQtIntraSliceLuma;      // [0..5]
        hucVvcS2LPicBss.m_spsLog2DiffMaxTtMinQtIntraSliceLuma     = m_vvcPicParams->m_spsLog2DiffMaxTtMinQtIntraSliceLuma;      // [0..4]
        hucVvcS2LPicBss.m_spsLog2DiffMinQtMinCbIntraSliceChroma   = m_vvcPicParams->m_spsLog2DiffMinQtMinCbIntraSliceChroma;    // [0..4]
        hucVvcS2LPicBss.m_spsMaxMttHierarchyDepthIntraSliceChroma = m_vvcPicParams->m_spsMaxMttHierarchyDepthIntraSliceChroma;  // [0..10]
        hucVvcS2LPicBss.m_spsLog2DiffMaxBtMinQtIntraSliceChroma   = m_vvcPicParams->m_spsLog2DiffMaxBtMinQtIntraSliceChroma;    // [0..4]
        hucVvcS2LPicBss.m_spsLog2DiffMaxTtMinQtIntraSliceChroma   = m_vvcPicParams->m_spsLog2DiffMaxTtMinQtIntraSliceChroma;    // [0..4]
        hucVvcS2LPicBss.m_spsLog2DiffMinQtMinCbInterSlice         = m_vvcPicParams->m_spsLog2DiffMinQtMinCbInterSlice;          // [0..4]
        hucVvcS2LPicBss.m_spsMaxMttHierarchyDepthInterSlice       = m_vvcPicParams->m_spsMaxMttHierarchyDepthInterSlice;        // [0..10]
        hucVvcS2LPicBss.m_spsLog2DiffMaxBtMinQtInterSlice         = m_vvcPicParams->m_spsLog2DiffMaxBtMinQtInterSlice;          // [0..5]
        hucVvcS2LPicBss.m_spsLog2DiffMaxTtMinQtInterSlice         = m_vvcPicParams->m_spsLog2DiffMaxTtMinQtInterSlice;          // [0..4]

        hucVvcS2LPicBss.m_picSpsFlags.m_spsSubpicInfoPresentFlag                   = m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag;                    // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsIndependentSubpicsFlag     = m_vvcPicParams->m_spsFlags0.m_fields.m_spsIndependentSubpicsFlag;                   // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSubpicSameSizeFlag                      = m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicSameSizeFlag;                       // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsEntropyCodingSyncEnabledFlag            = m_vvcPicParams->m_spsFlags0.m_fields.m_spsEntropyCodingSyncEnabledFlag;             // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsEntryPointOffsetsPresentFlag            = m_vvcPicParams->m_spsFlags0.m_fields.m_spsEntryPointOffsetsPresentFlag;             // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsPocMsbCycleFlag                         = m_vvcPicParams->m_spsFlags0.m_fields.m_spsPocMsbCycleFlag;                          // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsPartitionConstraintsOverrideEnabledFlag = m_vvcPicParams->m_spsFlags0.m_fields.m_spsPartitionConstraintsOverrideEnabledFlag;  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsQtbttDualTreeIntraFlag                  = m_vvcPicParams->m_spsFlags0.m_fields.m_spsQtbttDualTreeIntraFlag;                   // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsMaxLumaTransformSize64Flag              = m_vvcPicParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag;               // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsTransformSkipEnabledFlag                = m_vvcPicParams->m_spsFlags0.m_fields.m_spsTransformSkipEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsBdpcmEnabledFlag                        = m_vvcPicParams->m_spsFlags0.m_fields.m_spsBdpcmEnabledFlag;                         // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsMtsEnabledFlag                          = m_vvcPicParams->m_spsFlags0.m_fields.m_spsMtsEnabledFlag;                           // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsExplicitMtsIntraEnabledFlag             = m_vvcPicParams->m_spsFlags0.m_fields.m_spsExplicitMtsIntraEnabledFlag;              // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsExplicitMtsInterEnabledFlag             = m_vvcPicParams->m_spsFlags0.m_fields.m_spsExplicitMtsInterEnabledFlag;              // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsLfnstEnabledFlag                        = m_vvcPicParams->m_spsFlags0.m_fields.m_spsLfnstEnabledFlag;                         // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsJointCbcrEnabledFlag                    = m_vvcPicParams->m_spsFlags0.m_fields.m_spsJointCbcrEnabledFlag;                     // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSameQpTableForChromaFlag                = m_vvcPicParams->m_spsFlags0.m_fields.m_spsSameQpTableForChromaFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSaoEnabledFlag                          = m_vvcPicParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag;                           // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsAlfEnabledFlag                          = m_vvcPicParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag;                           // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsCcalfEnabledFlag                        = m_vvcPicParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag;                         // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsLmcsEnabledFlag                         = m_vvcPicParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag;                          // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsLongTermRefPicsFlag                     = m_vvcPicParams->m_spsFlags0.m_fields.m_spsLongTermRefPicsFlag;                      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsInterLayerPredictionEnabledFlag         = m_vvcPicParams->m_spsFlags0.m_fields.m_spsInterLayerPredictionEnabledFlag;          // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsIdrRplPresentFlag                       = m_vvcPicParams->m_spsFlags0.m_fields.m_spsIdrRplPresentFlag;                        // [0..1]

        hucVvcS2LPicBss.m_picSpsFlags.m_spsTemporalMvpEnabledFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsTemporalMvpEnabledFlag;          // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSbtmvpEnabledFlag      = m_vvcPicParams->m_spsFlags1.m_fields.m_spsSbtmvpEnabledFlag;               // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsAmvrEnabledFlag        = m_vvcPicParams->m_spsFlags1.m_fields.m_spsAmvrEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsBdofEnabledFlag        = m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsBdofControlPresentInPhFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofControlPresentInPhFlag;      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSmvdEnabledFlag            = m_vvcPicParams->m_spsFlags1.m_fields.m_spsSmvdEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsDmvrEnabledFlag            = m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsDmvrControlPresentInPhFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrControlPresentInPhFlag;      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsMmvdEnabledFlag            = m_vvcPicParams->m_spsFlags1.m_fields.m_spsMmvdEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsMmvdFullpelOnlyEnabledFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsMmvdFullpelOnlyEnabledFlag;      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSbtEnabledFlag             = m_vvcPicParams->m_spsFlags1.m_fields.m_spsSbtEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsAffineEnabledFlag          = m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineEnabledFlag;               // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_sps6paramAffineEnabledFlag    = m_vvcPicParams->m_spsFlags1.m_fields.m_sps6paramAffineEnabledFlag;         // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsAffineAmvrEnabledFlag      = m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineAmvrEnabledFlag;           // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsAffineProfEnabledFlag      = m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag;           // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsProfControlPresentInPhFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsProfControlPresentInPhFlag;      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsBcwEnabledFlag             = m_vvcPicParams->m_spsFlags1.m_fields.m_spsBcwEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsCiipEnabledFlag            = m_vvcPicParams->m_spsFlags1.m_fields.m_spsCiipEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsGpmEnabledFlag             = m_vvcPicParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsIspEnabledFlag             = m_vvcPicParams->m_spsFlags1.m_fields.m_spsIspEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsMrlEnabledFlag             = m_vvcPicParams->m_spsFlags1.m_fields.m_spsMrlEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsMipEnabledFlag             = m_vvcPicParams->m_spsFlags1.m_fields.m_spsMipEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsCclmEnabledFlag            = m_vvcPicParams->m_spsFlags1.m_fields.m_spsCclmEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsChromaHorizontalCollocatedFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsChromaHorizontalCollocatedFlag;  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsChromaVerticalCollocatedFlag   = m_vvcPicParams->m_spsFlags1.m_fields.m_spsChromaVerticalCollocatedFlag;    // [0..1]

        hucVvcS2LPicBss.m_picSpsFlags.m_spsPaletteEnabledFlag                   = m_vvcPicParams->m_spsFlags2.m_fields.m_spsPaletteEnabledFlag;                                  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsActEnabledFlag                       = m_vvcPicParams->m_spsFlags2.m_fields.m_spsActEnabledFlag;                                      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsIbcEnabledFlag                       = m_vvcPicParams->m_spsFlags2.m_fields.m_spsIbcEnabledFlag;                                      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsLadfEnabledFlag                      = m_vvcPicParams->m_spsFlags2.m_fields.m_spsLadfEnabledFlag;                                     // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsExplicitScalingListEnabledFlag       = m_vvcPicParams->m_spsFlags2.m_fields.m_spsExplicitScalingListEnabledFlag;                      // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsScalingMatrixForLfnstDisabledFlag    = m_vvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixForLfnstDisabledFlag;                   // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag = m_vvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag;  // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsScalingMatrixDesignatedColourSpaceFlag             = m_vvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixDesignatedColourSpaceFlag;              // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsDepQuantEnabledFlag                                = m_vvcPicParams->m_spsFlags2.m_fields.m_spsDepQuantEnabledFlag;                                 // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsSignDataHidingEnabledFlag                          = m_vvcPicParams->m_spsFlags2.m_fields.m_spsSignDataHidingEnabledFlag;                           // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsVirtualBoundariesEnabledFlag                       = m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag;                        // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsVirtualBoundariesPresentFlag                       = m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag;                        // [0..1]
        hucVvcS2LPicBss.m_picSpsFlags.m_spsWeightedPredFlag                                   = m_vvcPicParams->m_spsFlags2.m_fields.m_spsWeightedPredFlag;
        hucVvcS2LPicBss.m_picSpsFlags.m_spsWeightedBipredFlag                                 = m_vvcPicParams->m_spsFlags2.m_fields.m_spsWeightedBipredFlag;
        // PPS info
        hucVvcS2LPicBss.m_ppsPicWidthInLumaSamples  = m_vvcPicParams->m_ppsPicWidthInLumaSamples;  // [8..16888]
        hucVvcS2LPicBss.m_ppsPicHeightInLumaSamples = m_vvcPicParams->m_ppsPicHeightInLumaSamples;  // [8..16888]
        hucVvcS2LPicBss.m_numVerVirtualBoundaries   = m_vvcPicParams->m_numVerVirtualBoundaries;   // [0..3]
        hucVvcS2LPicBss.m_numHorVirtualBoundaries   = m_vvcPicParams->m_numHorVirtualBoundaries;   // [0..3]
        for (uint32_t i = 0; i < 3; i++)
        {
            hucVvcS2LPicBss.m_virtualBoundaryPosX[i] = m_vvcPicParams->m_virtualBoundaryPosX[i];  // [0..16880]
            hucVvcS2LPicBss.m_virtualBoundaryPosY[i] = m_vvcPicParams->m_virtualBoundaryPosY[i];  // [0..16880]
        }
        hucVvcS2LPicBss.m_ppsScalingWinLeftOffset   = m_vvcPicParams->m_ppsScalingWinLeftOffset;
        hucVvcS2LPicBss.m_ppsScalingWinRightOffset  = m_vvcPicParams->m_ppsScalingWinRightOffset;
        hucVvcS2LPicBss.m_ppsScalingWinTopOffset    = m_vvcPicParams->m_ppsScalingWinTopOffset;
        hucVvcS2LPicBss.m_ppsScalingWinBottomOffset = m_vvcPicParams->m_ppsScalingWinBottomOffset;

        hucVvcS2LPicBss.m_ppsNumExpTileColumnsMinus1 = m_vvcPicParams->m_ppsNumExpTileColumnsMinus1;
        hucVvcS2LPicBss.m_ppsNumExpTileRowsMinus1    = m_vvcPicParams->m_ppsNumExpTileRowsMinus1;
        hucVvcS2LPicBss.m_ppsNumSlicesInPicMinus1    = m_vvcPicParams->m_ppsNumSlicesInPicMinus1;
        for (uint32_t i = 0; i < 2; i++)
        {
            hucVvcS2LPicBss.m_ppsNumRefIdxDefaultActiveMinus1[i] = m_vvcPicParams->m_ppsNumRefIdxDefaultActiveMinus1[i];  // [0..16880]
        }
        hucVvcS2LPicBss.m_ppsPicWidthMinusWraparoundOffset = m_vvcPicParams->m_ppsPicWidthMinusWraparoundOffset;
        hucVvcS2LPicBss.m_ppsInitQpMinus26                 = m_vvcPicParams->m_ppsInitQpMinus26;
        hucVvcS2LPicBss.m_ppsCbQpOffset                    = m_vvcPicParams->m_ppsCbQpOffset;
        hucVvcS2LPicBss.m_ppsCrQpOffset                    = m_vvcPicParams->m_ppsCrQpOffset;
        hucVvcS2LPicBss.m_ppsJointCbcrQpOffsetValue        = m_vvcPicParams->m_ppsJointCbcrQpOffsetValue;
        hucVvcS2LPicBss.m_ppsChromaQpOffsetListLenMinus1   = m_vvcPicParams->m_ppsChromaQpOffsetListLenMinus1;
        for (uint32_t i = 0; i < 6; i++)
        {
            hucVvcS2LPicBss.m_ppsCbQpOffsetList[i]        = m_vvcPicParams->m_ppsCbQpOffsetList[i];
            hucVvcS2LPicBss.m_ppsCrQpOffsetList[i]        = m_vvcPicParams->m_ppsCrQpOffsetList[i];
            hucVvcS2LPicBss.m_ppsJointCbcrQpOffsetList[i] = m_vvcPicParams->m_ppsJointCbcrQpOffsetList[i];
        }
        hucVvcS2LPicBss.m_ppsLumaBetaOffsetDiv2                  = m_vvcPicParams->m_ppsLumaBetaOffsetDiv2;
        hucVvcS2LPicBss.m_ppsLumaTcOffsetDiv2                    = m_vvcPicParams->m_ppsLumaTcOffsetDiv2;
        hucVvcS2LPicBss.m_ppsCbBetaOffsetDiv2                    = m_vvcPicParams->m_ppsCbBetaOffsetDiv2;
        hucVvcS2LPicBss.m_ppsCbTcOffsetDiv2                      = m_vvcPicParams->m_ppsCbTcOffsetDiv2;
        hucVvcS2LPicBss.m_ppsCrBetaOffsetDiv2                    = m_vvcPicParams->m_ppsCrBetaOffsetDiv2;
        hucVvcS2LPicBss.m_ppsCrTcOffsetDiv2                      = m_vvcPicParams->m_ppsCrTcOffsetDiv2;
        hucVvcS2LPicBss.m_numScalingMatrixBuffers                = m_vvcPicParams->m_numScalingMatrixBuffers;
        hucVvcS2LPicBss.m_numAlfBuffers                          = m_vvcPicParams->m_numAlfBuffers;
        hucVvcS2LPicBss.m_numLmcsBuffers                         = m_vvcPicParams->m_numLmcsBuffers;
        hucVvcS2LPicBss.m_numRefPicListStructs                   = m_vvcPicParams->m_numRefPicListStructs;
        hucVvcS2LPicBss.m_numSliceStructsMinus1                  = m_vvcPicParams->m_numSliceStructsMinus1;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsOutputFlagPresentFlag = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsOutputFlagPresentFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsLoopFilterAcrossTilesEnabledFlag = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossTilesEnabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsRectSliceFlag                    = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsSingleSlicePerSubpicFlag         = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsLoopFilterAcrossSlicesEnabledFlag = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsLoopFilterAcrossSlicesEnabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsCabacInitPresentFlag              = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCabacInitPresentFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsRpl1IdxPresentFlag                = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRpl1IdxPresentFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsWeightedPredFlag                  = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedPredFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsWeightedBipredFlag                = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedBipredFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsRefWraparoundEnabledFlag          = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRefWraparoundEnabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsCuQpDeltaEnabledFlag              = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuQpDeltaEnabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsChroma_toolOffsetsPresentFlag     = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsSliceChromaQpOffsetsPresentFlag   = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSliceChromaQpOffsetsPresentFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsCuChromaQpOffsetListEnabledFlag   = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsDeblockingFilterOverrideEnabledFlag = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterOverrideEnabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsDeblockingFilterDisabledFlag        = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterDisabledFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsDbfInfoInPhFlag                     = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsRplInfoInPhFlag                     = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsSaoInfoInPhFlag                     = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsAlfInfoInPhFlag                     = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsWpInfoInPhFlag                      = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWpInfoInPhFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsQpDeltaInfoInPhFlag                 = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsQpDeltaInfoInPhFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsPictureHeaderExtensionPresentFlag   = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsPictureHeaderExtensionPresentFlag;
        hucVvcS2LPicBss.m_picPpsFlags.m_ppsSliceHeaderExtensionPresentFlag     = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSliceHeaderExtensionPresentFlag;

        // PH info
        hucVvcS2LPicBss.m_phNumAlfApsIdsLuma = m_vvcPicParams->m_phNumAlfApsIdsLuma;  // [0..7]
        for (uint32_t i = 0; i < 7; i++)
        {
            hucVvcS2LPicBss.m_phAlfApsIdLuma[i] = m_vvcPicParams->m_phAlfApsIdLuma[i];
        }
        hucVvcS2LPicBss.m_phAlfApsIdChroma                    = m_vvcPicParams->m_phAlfApsIdChroma;
        hucVvcS2LPicBss.m_phAlfCcCbApsId                      = m_vvcPicParams->m_phAlfCcCbApsId;
        hucVvcS2LPicBss.m_phAlfCcCrApsId                      = m_vvcPicParams->m_phAlfCcCrApsId;
        hucVvcS2LPicBss.m_phLmcsApsId                         = m_vvcPicParams->m_phLmcsApsId;
        hucVvcS2LPicBss.m_phScalingListApsId                  = m_vvcPicParams->m_phScalingListApsId;
        hucVvcS2LPicBss.m_phLog2DiffMinQtMinCbIntraSliceLuma  = m_vvcPicParams->m_phLog2DiffMinQtMinCbIntraSliceLuma;
        hucVvcS2LPicBss.m_phLog2DiffMaxBtMinQtIntraSliceLuma  = m_vvcPicParams->m_phLog2DiffMaxBtMinQtIntraSliceLuma;
        hucVvcS2LPicBss.m_phLog2DiffMax_ttMinQtIntraSliceLuma = m_vvcPicParams->m_phLog2DiffMax_ttMinQtIntraSliceLuma;
        hucVvcS2LPicBss.m_phCuChromaQpOffsetSubdivIntraSlice  = m_vvcPicParams->m_phCuChromaQpOffsetSubdivIntraSlice;
        hucVvcS2LPicBss.m_phLog2DiffMinQtMinCbInterSlice      = m_vvcPicParams->m_phLog2DiffMinQtMinCbInterSlice;
        hucVvcS2LPicBss.m_phMaxMtt_hierarchyDepthInterSlice   = m_vvcPicParams->m_phMaxMtt_hierarchyDepthInterSlice;
        hucVvcS2LPicBss.m_phLog2DiffMaxBtMinQtInterSlice      = m_vvcPicParams->m_phLog2DiffMaxBtMinQtInterSlice;
        hucVvcS2LPicBss.m_phLog2DiffMax_ttMinQtInterSlice     = m_vvcPicParams->m_phLog2DiffMax_ttMinQtInterSlice;
        hucVvcS2LPicBss.m_phCuQpDeltaSubdivInterSlice         = m_vvcPicParams->m_phCuQpDeltaSubdivInterSlice;
        hucVvcS2LPicBss.m_phCuChromaQpOffsetSubdivInterSlice  = m_vvcPicParams->m_phCuChromaQpOffsetSubdivInterSlice;
        hucVvcS2LPicBss.m_phCollocatedRefIdx                  = m_vvcPicParams->m_phCollocatedRefIdx;
        hucVvcS2LPicBss.m_phQpDelta                           = m_vvcPicParams->m_phQpDelta;
        hucVvcS2LPicBss.m_phLumaBetaOffsetDiv2                = m_vvcPicParams->m_phLumaBetaOffsetDiv2;
        hucVvcS2LPicBss.m_phLumaTcOffsetDiv2                  = m_vvcPicParams->m_phLumaTcOffsetDiv2;
        hucVvcS2LPicBss.m_phCbBetaOffsetDiv2                  = m_vvcPicParams->m_phCbBetaOffsetDiv2;
        hucVvcS2LPicBss.m_phCbTcOffsetDiv2                    = m_vvcPicParams->m_phCbTcOffsetDiv2;
        hucVvcS2LPicBss.m_phCrBetaOffsetDiv2                  = m_vvcPicParams->m_phCrBetaOffsetDiv2;
        hucVvcS2LPicBss.m_phCrTcOffsetDiv2                    = m_vvcPicParams->m_phCrTcOffsetDiv2;

        hucVvcS2LPicBss.m_picPhFlags.m_phNonRefPicFlag    = m_vvcPicParams->m_phFlags.m_fields.m_phNonRefPicFlag;                   // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phInterSliceAllowedFlag = m_vvcPicParams->m_phFlags.m_fields.m_phInterSliceAllowedFlag;           // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phAlfEnabledFlag        = m_vvcPicParams->m_phFlags.m_fields.m_phAlfEnabledFlag;                  // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phAlfCbEnabledFlag      = m_vvcPicParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag;                // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phAlfCrEnabledFlag      = m_vvcPicParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag;                // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phAlfCcCbEnabledFlag    = m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag;              // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phAlfCcCrEnabledFlag    = m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag;              // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phLmcsEnabledFlag       = m_vvcPicParams->m_phFlags.m_fields.m_phLmcsEnabledFlag;                 // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phChromaResidualScaleFlag = m_vvcPicParams->m_phFlags.m_fields.m_phChromaResidualScaleFlag;         // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phExplicitScalingListEnabledFlag = m_vvcPicParams->m_phFlags.m_fields.m_phExplicitScalingListEnabledFlag;  // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phVirtualBoundariesPresentFlag   = m_vvcPicParams->m_phFlags.m_fields.m_phVirtualBoundariesPresentFlag;    // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phTemporalMvpEnabledFlag         = m_vvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag;          // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_numRefEntries0RplIdx0LargerThan0 = m_vvcPicParams->m_phFlags.m_fields.m_numRefEntries0RplIdx0LargerThan0;  // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_numRefEntries1RplIdx1LargerThan0 = m_vvcPicParams->m_phFlags.m_fields.m_numRefEntries1RplIdx1LargerThan0;  // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phCollocatedFromL0Flag           = m_vvcPicParams->m_phFlags.m_fields.m_phCollocatedFromL0Flag;            // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phMmvdFullpelOnlyFlag            = m_vvcPicParams->m_phFlags.m_fields.m_phMmvdFullpelOnlyFlag;             // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phMvdL1ZeroFlag                  = m_vvcPicParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag;                   // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phBdofDisabledFlag               = m_vvcPicParams->m_phFlags.m_fields.m_phBdofDisabledFlag;                // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phDmvrDisabledFlag               = m_vvcPicParams->m_phFlags.m_fields.m_phDmvrDisabledFlag;                // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phProfDisabledFlag               = m_vvcPicParams->m_phFlags.m_fields.m_phProfDisabledFlag;                // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phJointCbcrSignFlag              = m_vvcPicParams->m_phFlags.m_fields.m_phJointCbcrSignFlag;               // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phSaoLumaEnabledFlag             = m_vvcPicParams->m_phFlags.m_fields.m_phSaoLumaEnabledFlag;              // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phSaoChromaEnabledFlag           = m_vvcPicParams->m_phFlags.m_fields.m_phSaoChromaEnabledFlag;            // [0..1]
        hucVvcS2LPicBss.m_picPhFlags.m_phDeblockingFilterDisabledFlag   = m_vvcPicParams->m_phFlags.m_fields.m_phDeblockingFilterDisabledFlag;    // [0..1]

        // reference lists
        hucVvcS2LPicBss.m_picOrderCntVal = m_vvcPicParams->m_picOrderCntVal;
        for (uint8_t j = 0; j < 15; j++)
        {
            hucVvcS2LPicBss.m_refFramePocList[j] = m_vvcPicParams->m_refFramePocList[j];  //This order need aligned with pipe buf ref frame order.
        }
        for (uint8_t i = 0; i < 2; i++)
        {
            for (uint8_t j = 0; j < 15; j++)
            {
                bool    rprConstraintFlag            = 0;
                uint8_t frameIdx                     = m_vvcPicParams->m_refFrameList[j].FrameIdx;
                m_vvcBasicFeature->m_refFrames.CalcRprConstraintsActiveFlag(frameIdx, rprConstraintFlag);
                hucVvcS2LPicBss.m_rprConstraintsActiveFlag[i][j] = rprConstraintFlag ? 1 : 0;
                hucVvcS2LPicBss.unavailableRefPic[i][j]          = (m_vvcPicParams->m_refFrameList[j].PicFlags == PICTURE_UNAVAILABLE_FRAME) ? true : false;
            }
        }
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag)
        {
            SetRefIdxStateCmd(hucVvcS2LPicBss.m_phRplInfoParam);
            for (uint8_t i = 0; i < 2; i++)
            {
                for (uint8_t j = 0; j < 15; j++)
                {
                    hucVvcS2LPicBss.m_phRplInfoParam.rprConstraintsActiveFlag[i][j] = (uint8_t)hucVvcS2LPicBss.m_rprConstraintsActiveFlag[i][j];
                    hucVvcS2LPicBss.m_phRplInfoParam.unavailableRefPic[i][j]        = (m_vvcPicParams->m_refFrameList[j].PicFlags == PICTURE_UNAVAILABLE_FRAME) ? true : false;
                }
            }
        }
        
        hucVvcS2LPicBss.m_numSlices      = m_vvcBasicFeature->m_numSlices;

        hucVvcS2LPicBss.m_picWidthInCtu = m_vvcBasicFeature->m_picWidthInCtu;
        hucVvcS2LPicBss.m_picHeightInCtu = m_vvcBasicFeature->m_picHeightInCtu;

        hucVvcS2LPicBss.tileCodingParam.m_tileCols = m_vvcBasicFeature->m_tileCols;
        hucVvcS2LPicBss.tileCodingParam.m_tileRows = m_vvcBasicFeature->m_tileRows;

        MOS_SecureMemcpy(hucVvcS2LPicBss.tileCodingParam.m_tileRow, sizeof(TileRowDesc) * vvcMaxTileRowNum, m_vvcBasicFeature->m_tileRow, sizeof(TileRowDesc) * vvcMaxTileRowNum);
        MOS_SecureMemcpy(hucVvcS2LPicBss.tileCodingParam.m_tileCol, sizeof(TileColDesc) * vvcMaxTileColNum, m_vvcBasicFeature->m_tileCol, sizeof(TileColDesc) * vvcMaxTileColNum);
        
        MOS_SecureMemcpy(&hucVvcS2LPicBss.m_vvcLmcsData, sizeof(CodecVvcLmcsData), &m_vvcBasicFeature->m_lmcsApsArray[m_vvcPicParams->m_phLmcsApsId], sizeof(CodecVvcLmcsData));
        MOS_SecureMemcpy(&hucVvcS2LPicBss.m_vvcLmcsShapeInfo, sizeof(ApsLmcsReshapeInfo), &m_vvcBasicFeature->m_lmcsReshaperInfo[m_vvcPicParams->m_phLmcsApsId], sizeof(ApsLmcsReshapeInfo));

        for (uint8_t i = 0; i < 8; i++)
        {
            hucVvcS2LPicBss.m_alfChromaNumAltFiltersMinus1[i] = m_vvcBasicFeature->m_alfApsArray[i].m_alfChromaNumAltFiltersMinus1;
            hucVvcS2LPicBss.m_alfCcCbFiltersSignalledMinus1[i] = m_vvcBasicFeature->m_alfApsArray[i].m_alfCcCbFiltersSignalledMinus1;
            hucVvcS2LPicBss.m_alfCcCrFiltersSignalledMinus1[i] = m_vvcBasicFeature->m_alfApsArray[i].m_alfCcCrFiltersSignalledMinus1;
        }
        hucVvcS2LPicBss.m_isMultiSubPicParam = m_isMultiSubPicParam;

        if (m_decodecp)
        {
            DECODE_CHK_STATUS(m_decodecp->SetHucDmemS2LPicBss(&hucVvcS2LPicBss.reserve, &(m_vvcBasicFeature->m_resDataBuffer.OsResource)));
        }
        else
        {
            hucVvcS2LPicBss.reserve.reserve_0 = 0;
            hucVvcS2LPicBss.reserve.reserve_1 = 0;
            hucVvcS2LPicBss.reserve.reserve_2 = 0;
            hucVvcS2LPicBss.reserve.reserve_3 = 0;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::SetRefIdxStateCmd(PicHeaderRplParam& phRplInfo)
    {
        DECODE_FUNC_CALL();
        MOS_ZeroMemory(&phRplInfo, sizeof(PicHeaderRplParam));
        CodecVvcRplStructure rpl = {};
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag) //Parse RefIdx State Cmd in driver when RPL info is in PH flag.
        {
            uint8_t rpl0EnableFrameCnt = 0;
            uint8_t rpl1EnableFrameCnt = 0;
            for (auto i = 0; i < vvcMaxNumRefFrame; i++)
            {
                if (m_vvcPicParams->m_refPicList[0][i].PicFlags != PICTURE_INVALID)
                {
                    rpl0EnableFrameCnt++;
                }
                if (m_vvcPicParams->m_refPicList[1][i].PicFlags != PICTURE_INVALID)
                {
                    rpl1EnableFrameCnt++;
                }
            }

            phRplInfo.numRefForList0 = m_vvcBasicFeature->m_numRefForList0;
            phRplInfo.numRefForList1 = m_vvcBasicFeature->m_numRefForList1;

            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < vvcMaxNumRefFrame; j++)
                {
                    phRplInfo.refPicListFrameIdx[i][j] = m_vvcPicParams->m_refPicList[i][j].FrameIdx;
                    phRplInfo.refPicListFrameFlag[i][j] = m_vvcPicParams->m_refPicList[i][j].PicFlags;
                }
            }
            FillPhRplInfoArray(LIST_0, rpl0EnableFrameCnt, phRplInfo);
            FillPhRplInfoArray(LIST_1, rpl1EnableFrameCnt, phRplInfo);
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::FillPhRplInfoArray(uint8_t listIdx, uint8_t entryCounts, PicHeaderRplParam &phRplInfo)
    {
        DECODE_FUNC_CALL();
        for (auto j = 0; j < entryCounts; j++)
        {
            // Short term reference flag
            phRplInfo.stRefPicFlag[listIdx][j] = (m_vvcPicParams->m_refPicList[listIdx][j].PicFlags == PICTURE_SHORT_TERM_REFERENCE) ? true : false;
            // DiffPicOrderCnt
            DECODE_ASSERT(m_vvcPicParams->m_refPicList[listIdx][j].FrameIdx < vvcMaxNumRefFrame);                              //TODO: unavailable? associated flag?
            int32_t refPoc                  = m_vvcPicParams->m_refFramePocList[m_vvcPicParams->m_refPicList[listIdx][j].FrameIdx];  //TODO: For error check, compare the poc from ref list and the value here.
            phRplInfo.diffPicOrderCnt[listIdx][j] = m_vvcPicParams->m_picOrderCntVal - refPoc;
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::SetDmemBuffer()
    {
        DECODE_FUNC_CALL();
        PMOS_BUFFER dmemBuf = m_vvcS2lDmemBufferArray->Peek();
        DECODE_CHK_NULL(dmemBuf);
        ResourceAutoLock resLock(m_allocator, &dmemBuf->OsResource);
        VvcS2lBss       *dmemBase = (VvcS2lBss *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(dmemBase);

        dmemBase->ProductFamily          = m_hucItf->GetHucProductFamily();
        dmemBase->RevId                  = m_hwInterface->GetPlatform().usRevId;
        dmemBase->isDmaCopyEnable        = DMAReadWriteEnabled;
        dmemBase->isCp                   = 0;

        if (m_osInterface != nullptr && m_osInterface->osCpInterface != nullptr)
        {
            dmemBase->isCp = m_osInterface->osCpInterface->IsCpEnabled() && m_osInterface->osCpInterface->IsHMEnabled();
        }
        else
        {
            dmemBase->isCp = 0;
        }

        DECODE_CHK_STATUS(ConstructLmcsReshaper());

        DECODE_CHK_STATUS(SetHucDmemPictureBss(dmemBase->VvcPictureBss));
        DECODE_CHK_STATUS(SetHucDmemSliceBss(dmemBase));
        
        auto offset1 = CODECHAL_OFFSETOF(HucVvcS2lPicBss, m_phCrTcOffsetDiv2);
        auto offset2 = CODECHAL_OFFSETOF(HucVvcS2lPicBss, m_picOrderCntVal);
        auto size    = offset2 - offset1;
        if (m_vvcBasicFeature->m_numSlices <= CODECHAL_VVC_MAX_NUM_SLICES_LVL_6)
        {
            m_dmemTransferSize = m_dmemBufferSize;
            m_dmemTransferSize = MOS_ALIGN_CEIL(m_dmemTransferSize, CODECHAL_CACHELINE_SIZE);
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::SetExtraDataBuffer()
    {
        DECODE_FUNC_CALL();
        m_vvcS2lExtraDataBuffer = m_vvcS2lExtraBufferArray->Fetch();
        DECODE_CHK_NULL(m_vvcS2lExtraDataBuffer);
        ResourceAutoLock resLock(m_allocator, &m_vvcS2lExtraDataBuffer->OsResource);
        VvcS2lExtraBss  *extraData = (VvcS2lExtraBss *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(extraData);
        //RPL INFO
        for (auto i = 0; i < vvcMaxRplNum; i++)
        {
            extraData->m_rplInfo[i].rplInfo.m_listIdx          = m_vvcBasicFeature->m_rplParams[i].m_listIdx;
            extraData->m_rplInfo[i].rplInfo.m_rplsIdx     = m_vvcBasicFeature->m_rplParams[i].m_rplsIdx;
            extraData->m_rplInfo[i].rplInfo.m_numRefEntries = m_vvcBasicFeature->m_rplParams[i].m_numRefEntries;
            extraData->m_rplInfo[i].rplInfo.m_ltrpInHeaderFlag = m_vvcBasicFeature->m_rplParams[i].m_ltrpInHeaderFlag;
            for (auto j = 0; j < 29; j++)
            {
                if ((m_vvcBasicFeature->m_vvcPicParams->m_spsFlags2.m_fields.m_spsWeightedPredFlag || m_vvcBasicFeature->m_vvcPicParams->m_spsFlags2.m_fields.m_spsWeightedBipredFlag) && j != 0 )
                {
                    extraData->m_rplInfo[i].rplInfo.m_absDeltaPocSt[j] = MOS_ABS(m_vvcBasicFeature->m_rplParams[i].m_deltaPocSt[j]);
                }
                else
                {
                    extraData->m_rplInfo[i].rplInfo.m_absDeltaPocSt[j] = MOS_ABS(m_vvcBasicFeature->m_rplParams[i].m_deltaPocSt[j]) - 1;
                } 
                extraData->m_rplInfo[i].rplInfo.m_strpEntrySignFlag[j] = m_vvcBasicFeature->m_rplParams[i].m_deltaPocSt[j] >= 0 ? 0 : 1;
            }
            MOS_SecureMemcpy(extraData->m_rplInfo[i].rplInfo.m_stRefPicFlag, sizeof(m_vvcBasicFeature->m_rplParams[i].m_stRefPicFlag), & m_vvcBasicFeature->m_rplParams[i].m_stRefPicFlag, sizeof(m_vvcBasicFeature->m_rplParams[i].m_stRefPicFlag));
            MOS_SecureMemcpy(extraData->m_rplInfo[i].rplInfo.m_rplsPocLsbLt, sizeof(m_vvcBasicFeature->m_rplParams[i].m_rplsPocLsbLt), & m_vvcBasicFeature->m_rplParams[i].m_rplsPocLsbLt, sizeof(m_vvcBasicFeature->m_rplParams[i].m_rplsPocLsbLt));
        }
        for (uint32_t i = 0; i < m_slicePartitionParamNumber; i++)
        {
            extraData->m_slicePartitionParam[i].partitionInfo.m_bottomSliceInTileFlag = m_vvcBasicFeature->m_sliceDesc[i].m_bottomSliceInTileFlag;
            extraData->m_slicePartitionParam[i].partitionInfo.m_multiSlicesInTileFlag = m_vvcBasicFeature->m_sliceDesc[i].m_multiSlicesInTileFlag;
            extraData->m_slicePartitionParam[i].partitionInfo.m_numCtusInCurrSlice    = m_vvcBasicFeature->m_sliceDesc[i].m_numCtusInCurrSlice;
            extraData->m_slicePartitionParam[i].partitionInfo.m_numSlicesInTile       = m_vvcBasicFeature->m_sliceDesc[i].m_numSlicesInTile;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceEndCtbx          = m_vvcBasicFeature->m_sliceDesc[i].m_sliceEndCtbx;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceEndCtby          = m_vvcBasicFeature->m_sliceDesc[i].m_sliceEndCtby;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceHeightInCtu      = m_vvcBasicFeature->m_sliceDesc[i].m_sliceHeightInCtu;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceHeightInTiles    = m_vvcBasicFeature->m_sliceDesc[i].m_sliceHeightInTiles;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceIdxInSubPic      = m_vvcBasicFeature->m_sliceDesc[i].m_sliceIdxInSubPic;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceStartCtbx        = m_vvcBasicFeature->m_sliceDesc[i].m_sliceStartCtbx;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceStartCtby        = m_vvcBasicFeature->m_sliceDesc[i].m_sliceStartCtby;
            extraData->m_slicePartitionParam[i].partitionInfo.m_sliceWidthInTiles     = m_vvcBasicFeature->m_sliceDesc[i].m_sliceWidthInTiles;
            extraData->m_slicePartitionParam[i].partitionInfo.m_startTileX            = m_vvcBasicFeature->m_sliceDesc[i].m_startTileX;
            extraData->m_slicePartitionParam[i].partitionInfo.m_startTileY            = m_vvcBasicFeature->m_sliceDesc[i].m_startTileY;
            extraData->m_slicePartitionParam[i].partitionInfo.m_subPicIdx             = m_vvcBasicFeature->m_sliceDesc[i].m_subPicIdx;
            extraData->m_slicePartitionParam[i].partitionInfo.m_tileIdx               = m_vvcBasicFeature->m_sliceDesc[i].m_tileIdx;
            extraData->m_slicePartitionParam[i].partitionInfo.m_topSliceInTileFlag    = m_vvcBasicFeature->m_sliceDesc[i].m_topSliceInTileFlag;
        }

        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWpInfoInPhFlag)
        {
            extraData->m_wpInfoinPH.paramField.m_lumaLog2WeightDenom        = m_vvcPicParams->m_wpInfo.m_lumaLog2WeightDenom;
            extraData->m_wpInfoinPH.paramField.m_deltaChromaLog2WeightDenom = m_vvcPicParams->m_wpInfo.m_deltaChromaLog2WeightDenom;
            extraData->m_wpInfoinPH.paramField.m_numL0Weights               = m_vvcPicParams->m_wpInfo.m_numL0Weights;
            extraData->m_wpInfoinPH.paramField.m_numL1Weights               = m_vvcPicParams->m_wpInfo.m_numL1Weights;
            for (auto i = 0; i < 15; i++)
            {
                extraData->m_wpInfoinPH.paramField.m_lumaWeightL0Flag[i]   = m_vvcPicParams->m_wpInfo.m_lumaWeightL0Flag[i];
                extraData->m_wpInfoinPH.paramField.m_chromaWeightL0Flag[i] = m_vvcPicParams->m_wpInfo.m_chromaWeightL0Flag[i];
                extraData->m_wpInfoinPH.paramField.m_deltaLumaWeightL0[i]  = m_vvcPicParams->m_wpInfo.m_deltaLumaWeightL0[i];
                extraData->m_wpInfoinPH.paramField.m_lumaOffsetL0[i]       = m_vvcPicParams->m_wpInfo.m_lumaOffsetL0[i];
                extraData->m_wpInfoinPH.paramField.m_lumaWeightL1Flag[i]   = m_vvcPicParams->m_wpInfo.m_lumaWeightL1Flag[i];
                extraData->m_wpInfoinPH.paramField.m_chromaWeightL1Flag[i] = m_vvcPicParams->m_wpInfo.m_chromaWeightL1Flag[i];
                extraData->m_wpInfoinPH.paramField.m_deltaLumaWeightL1[i]  = m_vvcPicParams->m_wpInfo.m_deltaLumaWeightL1[i];
                extraData->m_wpInfoinPH.paramField.m_lumaOffsetL1[i]       = m_vvcPicParams->m_wpInfo.m_lumaOffsetL1[i];
            }
            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < 15; j++)
                {
                    extraData->m_wpInfoinPH.paramField.m_deltaChromaWeightL0[j][i] = m_vvcPicParams->m_wpInfo.m_deltaChromaWeightL0[j][i];
                    extraData->m_wpInfoinPH.paramField.m_deltaChromaOffsetL0[j][i] = m_vvcPicParams->m_wpInfo.m_deltaChromaOffsetL0[j][i];
                    extraData->m_wpInfoinPH.paramField.m_deltaChromaWeightL1[j][i] = m_vvcPicParams->m_wpInfo.m_deltaChromaWeightL1[j][i];
                    extraData->m_wpInfoinPH.paramField.m_deltaChromaOffsetL1[j][i] = m_vvcPicParams->m_wpInfo.m_deltaChromaOffsetL1[j][i];
                }
            }
        }
        else
        {
            MOS_ZeroMemory(&extraData->m_wpInfoinPH, sizeof(extraData->m_wpInfoinPH));
        }

        return MOS_STATUS_SUCCESS;
    }

     MOS_STATUS VvcDecodeS2LPkt::ConstructLmcsReshaper() const
    {
        DECODE_FUNC_CALL()

        int32_t  reshapeLUTSize = 1 << (m_vvcPicParams->m_spsBitdepthMinus8 + 8);
        int32_t  pwlFwdLUTsize  = vvcPicCodeCwBins;
        int32_t  pwlFwdBinLen   = reshapeLUTSize / vvcPicCodeCwBins;
        uint16_t initCW         = (uint16_t)pwlFwdBinLen;

        CodecVvcLmcsData   *lmcsData               = &m_vvcBasicFeature->m_lmcsApsArray[m_vvcPicParams->m_phLmcsApsId];
        ApsLmcsReshapeInfo *sliceReshapeInfo       = &m_vvcBasicFeature->m_lmcsReshaperInfo[m_vvcPicParams->m_phLmcsApsId];
        uint32_t            reshaperModelMaxBinIdx = vvcPicCodeCwBins - 1 - lmcsData->m_lmcsDeltaMaxBinIdx;
        memset(sliceReshapeInfo->m_lmcsPivot, 0, sizeof(sliceReshapeInfo->m_lmcsPivot));

        if (m_vvcBasicFeature->m_activeLmcsMask & (1 << lmcsData->m_apsAdaptationParameterSetId))  // Only Transfer the data to dmem when LMCS array is available
        {
            for (int32_t i = 0; i < lmcsData->m_lmcsMinBinIdx; i++)
            {
                sliceReshapeInfo->m_lmcsCW[i] = 0;
            }
            for (auto i = reshaperModelMaxBinIdx + 1; i < vvcPicCodeCwBins; i++)
            {
                sliceReshapeInfo->m_lmcsCW[i] = 0;
            }
            for (auto i = lmcsData->m_lmcsMinBinIdx; i <= reshaperModelMaxBinIdx; i++)
            {
                sliceReshapeInfo->m_lmcsCW[i] = (uint16_t)(lmcsData->m_lmcsDeltaCW[i] + (int32_t)initCW);
            }

            for (auto i = 0; i < pwlFwdLUTsize; i++)
            {
                sliceReshapeInfo->m_lmcsPivot[i + 1] = sliceReshapeInfo->m_lmcsPivot[i] + sliceReshapeInfo->m_lmcsCW[i];
                sliceReshapeInfo->m_scaleCoeff[i]    = ((int32_t)sliceReshapeInfo->m_lmcsCW[i] * (1 << vvcFpPrec) + (1 << ((int32_t)log2(pwlFwdBinLen) - 1))) >> (int32_t)log2(pwlFwdBinLen);
                if (sliceReshapeInfo->m_lmcsCW[i] == 0)
                {
                    sliceReshapeInfo->m_invScaleCoeff[i]    = 0;
                    sliceReshapeInfo->m_chromaScaleCoeff[i] = 1 << vvcCscaleFpPrec;
                }
                else
                {
                    int32_t lmcsCwCrs = sliceReshapeInfo->m_lmcsCW[i] + lmcsData->m_lmcsDeltaCrs;
                    if (lmcsCwCrs < (initCW >> 3) || (lmcsCwCrs > (initCW << 3) - 1))
                    {
                        DECODE_ASSERTMESSAGE("Error concealed: force sh_lmcs_used_flag = 0 when (lmcsCW[ i ] + lmcsDeltaCrs) out of the range (OrgCW >> 3) to ((OrgCW << 3) - 1) inclusive.\n");
                    }
                    else
                    {
                        sliceReshapeInfo->m_invScaleCoeff[i]    = (int32_t)(initCW * (1 << vvcFpPrec) / sliceReshapeInfo->m_lmcsCW[i]);
                        sliceReshapeInfo->m_chromaScaleCoeff[i] = (int32_t)(initCW * (1 << vvcFpPrec) / (sliceReshapeInfo->m_lmcsCW[i] + lmcsData->m_lmcsDeltaCrs));
                    }
                }
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::AddHucCpState(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t index, CodecVvcSliceParams &sliceParams)
    {
        if (m_decodecp)
        {
            DECODE_CHK_STATUS(m_decodecp->AddHucState(&cmdBuffer,
                &(m_vvcBasicFeature->m_resDataBuffer.OsResource),
                sliceParams.m_sliceBytesInBuffer,
                sliceParams.m_byteOffsetToSliceData,
                index));
        }
        return MOS_STATUS_SUCCESS;
    }

    uint32_t VvcDecodeS2LPkt::GetSliceBatchOffset(uint32_t sliceNum)
    {
        DECODE_FUNC_CALL();
        uint32_t sliceLvlBufSize = 0;
        if (m_hwInterface != nullptr)
        {
            DECODE_CHK_STATUS(m_hwInterface->GetVvcpSliceLvlCmdSize(&sliceLvlBufSize));
            sliceLvlBufSize = MOS_ALIGN_CEIL(sliceLvlBufSize, 64);
        }
        return sliceLvlBufSize * sliceNum;
    }

    MOS_STATUS VvcDecodeS2LPkt::SetHucDmemSliceBss(
        VvcS2lBss* VvcS2lBss)
    {
        DECODE_FUNC_CALL();
        uint8_t* currentPointer = (uint8_t*)VvcS2lBss->sliceLvlParam;

        VvcS2lBss->sliceBbParamNumber = m_sliceBbParamNumber;
        VvcS2lBss->sliceBbParamOffset = m_sliceBbParamOffset;
        VvcS2lBss->sliceBsParamNumber = m_sliceBsParamNumber;
        VvcS2lBss->sliceBsParamOffset = m_sliceBsParamOffset;
        VvcS2lBss->slicePartitionParamNumber = m_slicePartitionParamNumber;
        VvcS2lBss->subPicParamNumber    = m_subPicParamNumber;
        VvcS2lBss->subPicParamOffset    = m_subPicParamOffset;

        currentPointer = (uint8_t*)(VvcS2lBss->sliceLvlParam) + m_sliceBsParamOffset;
        for (uint32_t i = 0; i < m_sliceBsParamNumber; i++)
        {
            VvcS2lSliceBsParam bsParam    = {};
            bsParam.BSNALunitDataLocation = m_vvcSliceParams[i].m_bSNALunitDataLocation;
            bsParam.SliceBytesInBuffer    = m_vvcSliceParams[i].m_sliceBytesInBuffer;
            
            if (m_decodecp)
            {
                DECODE_CHK_STATUS(m_decodecp->SetHucDmemS2LSliceBss(&bsParam.reserve, i, m_vvcSliceParams[i].m_sliceBytesInBuffer, m_vvcSliceParams[i].m_byteOffsetToSliceData));
            }
            MOS_SecureMemcpy(currentPointer, sizeof(VvcS2lSliceBsParam), &bsParam, sizeof(VvcS2lSliceBsParam));
            currentPointer += sizeof(VvcS2lSliceBsParam);
        }

        currentPointer = (uint8_t *)(VvcS2lBss->sliceLvlParam) + m_sliceBbParamOffset;
        
        for (uint32_t i = 0; i < m_sliceBbParamNumber; i++)
        {
            VvcS2lSliceBbParam bbParam  = {};
            uint32_t           cpSize      = 0;
            uint32_t           cpPatchSize = 0;
            m_hwInterface->GetCpInterface()->GetCpSliceLevelCmdSize(cpSize, cpPatchSize);
            bbParam.SliceCmdBatchOffset = GetSliceBatchOffset(i);
            MOS_SecureMemcpy(currentPointer, sizeof(VvcS2lSliceBbParam), &bbParam, sizeof(VvcS2lSliceBbParam));
            currentPointer += sizeof(VvcS2lSliceBbParam);
        }

        if (m_isMultiSubPicParam)  //SubPic param need be valid before copy.
        {
            currentPointer = (uint8_t *)(VvcS2lBss->sliceLvlParam) + m_subPicParamOffset;
            for (uint32_t i = 0; i < m_subPicParamNumber; i++)
            {
                VvcS2lSubpicParam subPicParam       = {};
                subPicParam.m_endCtbX               = m_vvcBasicFeature->m_subPicParams[i].m_endCtbX;
                subPicParam.m_endCtbY               = m_vvcBasicFeature->m_subPicParams[i].m_endCtbY;
                subPicParam.m_numSlices             = m_vvcBasicFeature->m_subPicParams[i].m_numSlices;
                subPicParam.m_spsSubpicCtuTopLeftX  = m_vvcBasicFeature->m_subPicParams[i].m_spsSubpicCtuTopLeftX;
                subPicParam.m_spsSubpicCtuTopLeftY  = m_vvcBasicFeature->m_subPicParams[i].m_spsSubpicCtuTopLeftY;
                subPicParam.m_spsSubpicHeightMinus1 = m_vvcBasicFeature->m_subPicParams[i].m_spsSubpicHeightMinus1;
                subPicParam.m_spsSubpicWidthMinus1  = m_vvcBasicFeature->m_subPicParams[i].m_spsSubpicWidthMinus1;
                subPicParam.m_subPicFlags.m_value   = m_vvcBasicFeature->m_subPicParams[i].m_subPicFlags.m_value;
                subPicParam.m_subpicIdVal           = m_vvcBasicFeature->m_subPicParams[i].m_subpicIdVal;
                MOS_SecureMemcpy(currentPointer, sizeof(VvcS2lSubpicParam), &subPicParam, sizeof(VvcS2lSubpicParam));
                currentPointer += sizeof(VvcS2lSubpicParam);
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void VvcDecodeS2LPkt::CalculateVvcSliceLvlCmdSize()
    {
        m_vvcpSliceCmdSize = m_vvcpItf->GETSIZE_VVCP_SLICE_STATE() +
                             m_vvcpItf->GETSIZE_VVCP_REF_IDX_STATE() * 2 +
                             m_vvcpItf->GETSIZE_VVCP_WEIGHTOFFSET_STATE() * 2 +
                             m_vvcpItf->GETSIZE_VVCP_BSD_OBJECT() +
                             m_miItf->GETSIZE_MI_BATCH_BUFFER_START();
    }

    MOS_STATUS VvcDecodeS2LPkt::AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();
        auto &par            = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
        par                  = {};
        par.kernelDescriptor = m_vdboxHucVvcS2lKernelDescriptor;
        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(&cmdBuffer));

        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();
        //for gen 11+, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        auto &par            = m_hucItf->MHW_GETPAR_F(HUC_PIPE_MODE_SELECT)();
        par                  = {};
        par.streamOutEnabled = false;
        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_PIPE_MODE_SELECT)(&cmdBuffer));

        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));
        return MOS_STATUS_SUCCESS;
    }

    PMOS_BUFFER VvcDecodeS2LPkt::GetS2lDmemBuffer()
    {
        if (m_vvcS2lDmemBufferArray == nullptr)
        {
            return nullptr;
        }
        return m_vvcS2lDmemBufferArray->Peek();
    }

    MHW_SETPAR_DECL_SRC(HUC_IND_OBJ_BASE_ADDR_STATE, VvcDecodeS2LPkt)
    {
        DECODE_FUNC_CALL();

        params.DataSize   = m_vvcBasicFeature->m_dataSize;
        params.DataOffset = m_vvcBasicFeature->m_dataOffset;
        params.DataBuffer = &(m_vvcBasicFeature->m_resDataBuffer.OsResource);
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, VvcDecodeS2LPkt)
    {
        DECODE_FUNC_CALL();
        
        PMHW_BATCH_BUFFER sliceBatchBuffer = m_vvcPipeline->GetSliceLvlCmdBuffer();
        DECODE_ASSERT(sliceBatchBuffer != nullptr);
        params.regionParams[0].presRegion = &sliceBatchBuffer->OsResource;
        params.regionParams[0].isWritable = true;
        params.regionParams[0].dwOffset   = 0;

        PMHW_BATCH_BUFFER tileBatchBuffer = m_vvcPipeline->GetTileLvlCmdBuffer();
        DECODE_ASSERT(tileBatchBuffer != nullptr);
        params.regionParams[1].presRegion = &tileBatchBuffer->OsResource;
        params.regionParams[1].isWritable = true;
        params.regionParams[1].dwOffset   = 0;

        DECODE_ASSERT(m_vvcS2lExtraDataBuffer != nullptr);
        params.regionParams[2].presRegion = &m_vvcS2lExtraDataBuffer->OsResource;
        params.regionParams[2].isWritable = false;
        params.regionParams[2].dwOffset   = 0;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, VvcDecodeS2LPkt)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_vvcS2lDmemBufferArray);
        auto dmemBuffer = m_vvcS2lDmemBufferArray->Peek();
        DECODE_CHK_NULL(dmemBuffer);
        params.hucDataSource = &dmemBuffer->OsResource;
        params.dataLength    = MOS_ALIGN_CEIL(m_dmemTransferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::AddCmd_HUC_STREAM_OBJECT(MOS_COMMAND_BUFFER &cmdBuffer, CodecVvcSliceParams sliceParams)
    {
        DECODE_FUNC_CALL();

        auto &par = m_hucItf->MHW_GETPAR_F(HUC_STREAM_OBJECT)();
        par                              = {};
        par.IndirectStreamInDataLength   = sliceParams.m_sliceBytesInBuffer + m_tailingBsReadSize;
        par.StreamOut                    = 0;
        par.IndirectStreamInStartAddress = sliceParams.m_bSNALunitDataLocation;
        par.HucProcessing                = true;

        par.HucBitstreamEnable             = 1;
        par.EmulationPreventionByteRemoval = 0;
        par.StartCodeSearchEngine          = 0;
        par.StartCodeByte0                 = 0;
        par.StartCodeByte1                 = 0;
        par.StartCodeByte2                 = 1;

        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_STREAM_OBJECT)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPkt::AddCmd_HUC_START(MOS_COMMAND_BUFFER &cmdBuffer, bool laststreamobject)
    {
        DECODE_FUNC_CALL();
        auto &par            = m_hucItf->MHW_GETPAR_F(HUC_START)();
        par                  = {};
        par.lastStreamObject = laststreamobject;
        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_START)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS VvcDecodeS2LPkt::DumpHucS2l()
    {
        DECODE_FUNC_CALL();

        int32_t currentPass = m_vvcPipeline->GetCurrentPass();

        CodechalDebugInterface *debugInterface = m_vvcPipeline->GetDebugInterface();
        DECODE_CHK_NULL(debugInterface);
        auto dmemBuffer = GetS2lDmemBuffer();
        DECODE_CHK_NULL(dmemBuffer);
        DECODE_CHK_STATUS(debugInterface->DumpHucDmem(
            &dmemBuffer->OsResource,
            m_dmemTransferSize,
            currentPass,
            hucRegionDumpDefault));

        return MOS_STATUS_SUCCESS;
    }
#endif

};
