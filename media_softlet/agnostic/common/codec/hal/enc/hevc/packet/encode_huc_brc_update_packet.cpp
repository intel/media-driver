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
//! \file     encode_huc_brc_update_packet.cpp
//! \brief    Defines the implementation of huc update packet 
//!

#include "encode_huc_brc_update_packet.h"
#include "codechal_debug.h"
#include "encode_hevc_vdenc_weighted_prediction.h"
#include "encode_hevc_brc.h"
#include "encode_hevc_vdenc_scc.h"
#include "encode_vdenc_lpla_analysis.h"
#include "encode_hevc_vdenc_lpla_enc.h"
#include "encode_hevc_header_packer.h"

namespace encode
{
    MOS_STATUS HucBrcUpdatePkt::Init()
    {
        ENCODE_FUNC_CALL();
        m_hwInterface->m_vdencBatchBuffer1stGroupSize = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBatchBuffer1stGroupSize, CODECHAL_CACHELINE_SIZE);
        m_hwInterface->m_vdencBatchBuffer2ndGroupSize = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBatchBuffer2ndGroupSize, CODECHAL_CACHELINE_SIZE);
        m_hwInterface->m_vdencReadBatchBufferSize =
        m_hwInterface->m_vdenc2ndLevelBatchBufferSize = m_hwInterface->m_vdencBatchBuffer1stGroupSize
                                         + m_hwInterface->m_vdencBatchBuffer2ndGroupSize
                                         + ENCODE_HEVC_VDENC_NUM_MAX_SLICES * MOS_ALIGN_CEIL((2 * m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)()
                                         + m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)()
                                         + 2 * m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)()
                                         + sizeof (HevcSlice)
                                         + m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)()
                                         + 2 * m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)()), CODECHAL_CACHELINE_SIZE);
        m_hwInterface->m_vdencBatchBufferPerSliceConstSize = m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)()
        + m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)()
        + m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)()
        + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)() * 2;

        HUC_CHK_STATUS_RETURN(EncodeHucPkt::Init());

        ENCODE_CHK_NULL_RETURN(m_pipeline);
        m_allocator = m_pipeline->GetEncodeAllocator();
        ENCODE_CHK_NULL_RETURN(m_allocator);

        ENCODE_CHK_NULL_RETURN(m_featureManager);
        m_basicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(HevcFeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(m_basicFeature);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();

        auto osInterface = m_hwInterface->GetOsInterface();
        ENCODE_CHK_NULL_RETURN(osInterface);

        uint32_t hucCommandsSize = 0;
        uint32_t hucPatchListSize = 0;
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

        stateCmdSizeParams.uNumStoreDataImm = 2;
        stateCmdSizeParams.uNumStoreReg     = 4;
        stateCmdSizeParams.uNumMfxWait      = 3;
        stateCmdSizeParams.uNumAddConBBEnd  = 1;
        ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucStateCommandSize(
            m_basicFeature->m_mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, &stateCmdSizeParams));

        commandBufferSize = hucCommandsSize;
        requestedPatchListSize = osInterface->bUsesPatchList ? hucPatchListSize : 0;

        if (m_pipeline->IsSingleTaskPhaseSupported())
        {
            commandBufferSize *= m_pipeline->GetPassNum();
        }

        // 4K align since allocation is in chunks of 4K bytes.
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::DumpOutput()
    {
        ENCODE_FUNC_CALL();

#if USE_CODECHAL_DEBUG_TOOL
        DumpHucBrcUpdate(false);
#endif
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // HuC FW Region 6: Data Buffer of Current Picture
        // Data (1024 bytes) for current
        // Data (1024 bytes) for ref0
        // Data (1024 bytes) for ref1
        // Data (1024 bytes) for ref2
        allocParamsForBufferLinear.dwBytes = CODECHAL_PAGE_SIZE * 4;
        allocParamsForBufferLinear.pBufName = "Data from Pictures Buffer for Weighted Prediction";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        MOS_RESOURCE *allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_dataFromPicsBuffer = *allocatedbuffer;

        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            // Const Data buffer
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencBrcConstDataBuffer[k] = *allocatedbuffer;

            for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
            {
                // VDEnc read batch buffer (input for HuC FW)
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer Origin";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
                allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencReadBatchBufferOrigin[k][i] = *allocatedbuffer;

                // VDEnc read batch buffer (input for HuC FW)
                allocParamsForBufferLinear.dwBytes      = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName     = "VDENC Read Batch Buffer TU7";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
                allocatedbuffer                         = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencReadBatchBufferTU7[k][i] = *allocatedbuffer;

                // BRC update DMEM
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
                allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencBrcUpdateDmemBuffer[k][i] = *allocatedbuffer;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_STATE, HucBrcUpdatePkt)
    {
        if (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)  // Low Delay BRC
        {
            params.kernelDescriptor = m_vdboxHucHevcBrcLowdelayKernelDescriptor;
        }
        else
        {
            params.kernelDescriptor = m_vdboxHucHevcBrcUpdateKernelDescriptor;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_DMEM_STATE, HucBrcUpdatePkt)
    {
        ENCODE_CHK_STATUS_RETURN(SetDmemBuffer());

        params.function      = BRC_UPDATE;
        params.passNum       = static_cast<uint8_t>(m_pipeline->GetPassNum());
        params.currentPass   = static_cast<uint8_t> (m_pipeline->GetCurrentPass());
        params.hucDataSource = const_cast<PMOS_RESOURCE> (&m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]);
        params.dataLength    = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        params.dmemOffset    = HUC_DMEM_OFFSET_RTOS_GEMS;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HUC_VIRTUAL_ADDR_STATE, HucBrcUpdatePkt)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        int32_t currentPass = m_pipeline->GetCurrentPass();
        if (currentPass < 0)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);

        params.function = BRC_UPDATE;

        ENCODE_CHK_STATUS_RETURN(SetConstDataHuCBrcUpdate());

        // Add Virtual addr
        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature,
            SetCurrRecycledBufIdx, m_pipeline->m_currRecycledBufIdx);

        params.regionParams[2].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(FrameStatStreamOutBuffer, 0);      // Region 2  PAK Statistics Buffer (Input) - MFX_PAK_FRAME_STATISTICS
        params.regionParams[3].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencReadBatchBufferOrigin[m_pipeline->m_currRecycledBufIdx][currentPass]);    // Region 3 - Input SLB Buffer (Input Origin)
        params.regionParams[4].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]);              // Region 4 - Constant Data (Input)
        params.regionParams[6].presRegion = const_cast<PMOS_RESOURCE>(&m_dataFromPicsBuffer);                   // Region 6 - Data Buffer of Current and Reference Pictures for Weighted Prediction (Input/Output)
        params.regionParams[6].isWritable = true;
        params.regionParams[8].presRegion =
            m_basicFeature->m_recycleBuf->GetBuffer(PakInfo, 0);                        // Region 8 - PAK Information (Input)

        params.regionParams[7].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(LcuBaseAddressBuffer, 0);   // Region 7  Slice Stat Streamout (Input)

        bool tileEnabled = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsEnabled, tileEnabled);
        if (tileEnabled)
        {
            MOS_RESOURCE *resHuCPakAggregatedFrameStatsBuffer = nullptr;
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetHucPakAggregatedFrameStatsBuffer, resHuCPakAggregatedFrameStatsBuffer);
            HevcTileStatusInfo tileStatsOffset = {};
            HevcTileStatusInfo frameStatsOffset = {};
            HevcTileStatusInfo statsSize = {};
            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, GetTileStatusInfo, tileStatsOffset, frameStatsOffset, statsSize);
            params.regionParams[1].presRegion = resHuCPakAggregatedFrameStatsBuffer;
            params.regionParams[1].dwOffset = frameStatsOffset.vdencStatistics;

            if (m_pipeline->GetPipeNum() > 1)
            {
                params.regionParams[2].presRegion = resHuCPakAggregatedFrameStatsBuffer;  // Region 2  PAK Statistics Buffer (Input) - MFX_PAK_FRAME_STATISTICS
                params.regionParams[2].dwOffset = frameStatsOffset.hevcPakStatistics;
                params.regionParams[7].presRegion = resHuCPakAggregatedFrameStatsBuffer;  // Region 7  Slice Stat Streamout (Input)
                params.regionParams[7].dwOffset = frameStatsOffset.hevcSliceStreamout;
                // In scalable-mode, use PAK Integration kernel output to get bistream size
                MOS_RESOURCE *resBrcDataBuffer = nullptr;
                RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, GetBrcDataBuffer, resBrcDataBuffer);
                params.regionParams[8].presRegion = resBrcDataBuffer;
            }
        }
        else
        {
            params.regionParams[1].presRegion = m_basicFeature->m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);  // Region 1  VDEnc Statistics Buffer (Input) - VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT
        }

        if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
        {
            params.regionParams[12].presRegion = const_cast<PMOS_RESOURCE>(&m_vdencReadBatchBufferTU7[m_pipeline->m_currRecycledBufIdx][currentPass]);  // Region 12 - Input SLB Buffer (Input TU7)
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::SetCommonDmemBuffer(VdencHevcHucBrcUpdateDmem *hucVdencBrcUpdateDmem)
    {
        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature, SetDmemForUpdate, hucVdencBrcUpdateDmem);

        hucVdencBrcUpdateDmem->TARGETSIZE_U32 =
            (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) ? m_basicFeature->m_hevcSeqParams->InitVBVBufferFullnessInBit : MOS_MIN(m_basicFeature->m_hevcSeqParams->InitVBVBufferFullnessInBit, m_basicFeature->m_hevcSeqParams->VBVBufferSizeInBit);
        hucVdencBrcUpdateDmem->FrameID_U32                   = m_basicFeature->m_frameNum;  // frame number
        hucVdencBrcUpdateDmem->TargetSliceSize_U16           = (uint16_t)m_basicFeature->m_hevcPicParams->MaxSliceSizeInBytes;
        hucVdencBrcUpdateDmem->SLB_Data_SizeInBytes          = (uint16_t)m_slbDataSizeInBytes;
        hucVdencBrcUpdateDmem->PIPE_MODE_SELECT_StartInBytes = 0xFFFF;  // HuC need not modify the pipe mode select command in Gen11+
        hucVdencBrcUpdateDmem->CMD1_StartInBytes             = (uint16_t)m_hwInterface->m_vdencBatchBuffer1stGroupSize;
        hucVdencBrcUpdateDmem->PIC_STATE_StartInBytes        = (uint16_t)m_basicFeature->m_picStateCmdStartInBytes;
        hucVdencBrcUpdateDmem->CMD2_StartInBytes             = (uint16_t)m_cmd2StartInBytes;

        if (m_basicFeature->m_prevStoreData != m_basicFeature->m_frameNum)
        {
            m_basicFeature->m_prevStoreData = m_basicFeature->m_frameNum;

            int32_t  oldestIdx    = -1;
            int32_t  selectedSlot = -1;
            uint32_t oldestAge    = 0;
            for (int i = 0; i < CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER; i++)
            {
                if (slotInfo[i].isUsed == true && slotInfo[i].isRef)
                {
                    slotInfo[i].age++;
                    if (slotInfo[i].age >= oldestAge)
                    {
                        oldestAge = slotInfo[i].age;
                        oldestIdx = i;
                    }
                }
                if ((selectedSlot == -1) && (slotInfo[i].isUsed == false || !slotInfo[i].isRef))
                {
                    selectedSlot = i;
                }
            }

            if (selectedSlot == -1)
            {
                selectedSlot = oldestIdx;
            }

            if (selectedSlot == -1)
            {
                ENCODE_ASSERTMESSAGE("No valid ref slot index");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            slotInfo[selectedSlot].age    = 0;
            slotInfo[selectedSlot].poc    = m_basicFeature->m_hevcPicParams->CurrPicOrderCnt;
            slotInfo[selectedSlot].isUsed = true;
            slotInfo[selectedSlot].isRef  = m_basicFeature->m_hevcPicParams->bUsedAsRef;

            m_curPicSlot = selectedSlot;
        }

        hucVdencBrcUpdateDmem->Current_Data_Offset = m_curPicSlot * m_weightHistSize;

        for (uint8_t refIdx = 0; refIdx <= m_basicFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = m_basicFeature->m_hevcSliceParams->RefPicList[LIST_0][refIdx];
            auto          refPOC = m_basicFeature->m_hevcPicParams->RefFramePOCList[refPic.FrameIdx];
            for (int i = 0; i < CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER; i++)
            {
                if (slotInfo[i].poc == refPOC)
                {
                    hucVdencBrcUpdateDmem->Ref_Data_Offset[refIdx] = i * m_weightHistSize;
                    break;
                }
            }
        }

        for (uint8_t refIdx = 0; refIdx <= m_basicFeature->m_hevcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = m_basicFeature->m_hevcSliceParams->RefPicList[LIST_1][refIdx];
            auto          refPOC = m_basicFeature->m_hevcPicParams->RefFramePOCList[refPic.FrameIdx];
            for (int i = 0; i < CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER; i++)
            {
                if (slotInfo[i].poc == refPOC)
                {
                    hucVdencBrcUpdateDmem->Ref_Data_Offset[refIdx + m_basicFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1] = i * m_weightHistSize;
                    break;
                }
            }
        }

        hucVdencBrcUpdateDmem->MaxNumSliceAllowed_U16 = (uint16_t)GetMaxAllowedSlices(m_basicFeature->m_hevcSeqParams->Level);

        hucVdencBrcUpdateDmem->OpMode_U8 = 0x4;

        bool enableTileReplay = false;
        RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, IsTileReplayEnabled, enableTileReplay);
        if (enableTileReplay)
        {
            hucVdencBrcUpdateDmem->OpMode_U8 = 0x8;
        }
        else
        {
            RUN_FEATURE_INTERFACE_RETURN(
                HevcVdencWeightedPred,
                HevcFeatureIDs::hevcVdencWpFeature,
                SetHucBrcUpdateDmemBuffer,
                m_pipeline->IsFirstPass(),
                *hucVdencBrcUpdateDmem);
        }

        bool bAllowedPyramid = m_basicFeature->m_hevcSeqParams->GopRefDist != 3;

        if (m_basicFeature->m_pictureCodingType == I_TYPE)
        {
            hucVdencBrcUpdateDmem->CurrentFrameType_U8 = HEVC_BRC_FRAME_TYPE_I;
        }
        else if (m_basicFeature->m_hevcSeqParams->HierarchicalFlag && bAllowedPyramid)
        {
            if (m_basicFeature->m_hevcPicParams->HierarchLevelPlus1 > 0)
            {
                std::map<int, HEVC_BRC_FRAME_TYPE> hierchLevelPlus1_to_brclevel{
                    {1, HEVC_BRC_FRAME_TYPE_P_OR_LB},
                    {2, HEVC_BRC_FRAME_TYPE_B},
                    {3, HEVC_BRC_FRAME_TYPE_B1},
                    {4, HEVC_BRC_FRAME_TYPE_B2}};
                hucVdencBrcUpdateDmem->CurrentFrameType_U8 = hierchLevelPlus1_to_brclevel.count(m_basicFeature->m_hevcPicParams->HierarchLevelPlus1) ? hierchLevelPlus1_to_brclevel[m_basicFeature->m_hevcPicParams->HierarchLevelPlus1] : HEVC_BRC_FRAME_TYPE_INVALID;
                //Invalid HierarchLevelPlus1 or LBD frames at level 3 eror check.
                if ((hucVdencBrcUpdateDmem->CurrentFrameType_U8 == HEVC_BRC_FRAME_TYPE_INVALID) ||
                    (m_basicFeature->m_hevcSeqParams->LowDelayMode && hucVdencBrcUpdateDmem->CurrentFrameType_U8 == HEVC_BRC_FRAME_TYPE_B2))
                {
                    ENCODE_ASSERTMESSAGE("HEVC_BRC_FRAME_TYPE_INVALID or LBD picture doesn't support Level 4\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else if (!m_basicFeature->m_hevcSeqParams->LowDelayMode)  //RA
            {
                //if L0/L1 both points to previous frame, then its LBD otherwise its is level 1 RA B.
                auto                               B_or_LDB_brclevel = m_basicFeature->m_ref.IsLowDelay() ? HEVC_BRC_FRAME_TYPE_P_OR_LB : HEVC_BRC_FRAME_TYPE_B;
                std::map<int, HEVC_BRC_FRAME_TYPE> codingtype_to_brclevel{
                    {P_TYPE, HEVC_BRC_FRAME_TYPE_P_OR_LB},
                    {B_TYPE, B_or_LDB_brclevel},
                    {B1_TYPE, HEVC_BRC_FRAME_TYPE_B1},
                    {B2_TYPE, HEVC_BRC_FRAME_TYPE_B2}};
                hucVdencBrcUpdateDmem->CurrentFrameType_U8 = codingtype_to_brclevel.count(m_basicFeature->m_pictureCodingType) ? codingtype_to_brclevel[m_basicFeature->m_pictureCodingType] : HEVC_BRC_FRAME_TYPE_INVALID;
                //Invalid CodingType.
                if (hucVdencBrcUpdateDmem->CurrentFrameType_U8 == HEVC_BRC_FRAME_TYPE_INVALID)
                {
                    ENCODE_ASSERTMESSAGE("Invalid CodingType\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else  //LDB
            {
                hucVdencBrcUpdateDmem->CurrentFrameType_U8 = HEVC_BRC_FRAME_TYPE_P_OR_LB;  //No Hierarchical info for LDB, treated as flat case
            }
        }
        else
        {
            hucVdencBrcUpdateDmem->CurrentFrameType_U8 = m_basicFeature->m_ref.IsLowDelay() ? HEVC_BRC_FRAME_TYPE_P_OR_LB : HEVC_BRC_FRAME_TYPE_B;
        }

        // Num_Ref_L1 should be always same as Num_Ref_L0
        hucVdencBrcUpdateDmem->Num_Ref_L0_U8 = m_basicFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
        hucVdencBrcUpdateDmem->Num_Ref_L1_U8 = m_basicFeature->m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;
        hucVdencBrcUpdateDmem->Num_Slices    = (uint8_t)m_basicFeature->m_hevcPicParams->NumSlices;

        // CQP_QPValue_U8 setting is needed since ACQP is also part of ICQ
        hucVdencBrcUpdateDmem->CQP_QPValue_U8 = m_basicFeature->m_hevcPicParams->QpY + m_basicFeature->m_hevcSliceParams->slice_qp_delta;
        hucVdencBrcUpdateDmem->CQP_FracQP_U8  = 0;
        if (m_basicFeature->m_hevcPicParams->BRCPrecision == 1)
        {
            hucVdencBrcUpdateDmem->MaxNumPass_U8 = 1;
        }
        else
        {
            hucVdencBrcUpdateDmem->MaxNumPass_U8 = VDENC_BRC_NUM_OF_PASSES;
        }

        hucVdencBrcUpdateDmem->IPAverageCoeff_U8 = (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) ? 0 : 64;
        hucVdencBrcUpdateDmem->CurrentPass_U8    = (uint8_t)m_pipeline->GetCurrentPass();

        // chroma weights are not confirmed to be supported from HW team yet
        hucVdencBrcUpdateDmem->DisabledFeature_U8 = 0;  // bit mask, 1 (bit0): disable chroma weight setting

        hucVdencBrcUpdateDmem->SlidingWindow_Enable_U8 = (m_basicFeature->m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
        hucVdencBrcUpdateDmem->LOG_LCU_Size_U8         = 6;

        RUN_FEATURE_INTERFACE_RETURN(
            HevcVdencScc,
            HevcFeatureIDs::hevcVdencSccFeature,
            SetHucBrcUpdateDmem,
            hucVdencBrcUpdateDmem);

        // reset skip frame statistics
        //m_numSkipFrames = 0;
        //m_sizeSkipFrames = 0;

        // For tile row based BRC
        //if (m_basicFeature->m_TileRowLevelBRC)
        //{
        //    hucVdencBrcUpdateDmem->MaxNumTileHuCCallMinus1 = m_basicFeature->m_hevcPicParams->num_tile_rows_minus1 + 1;
        //    hucVdencBrcUpdateDmem->TileHucCallIndex        = (uint8_t)m_CurrentTileRow;
        //    hucVdencBrcUpdateDmem->TileHuCCallPassIndex    = m_CurrentPassForTileReplay + 1;
        //    hucVdencBrcUpdateDmem->TileHuCCallPassMax      = m_NumPassesForTileReplay;

        //    hucVdencBrcUpdateDmem->TxSizeInBitsPerFrame    = 0; //threshold for the min frame size

        //    uint32_t numTileColumns = m_basicFeature->m_hevcPicParams->num_tile_columns_minus1 + 1;
        //    uint32_t startIdx       = m_CurrentTileRow * numTileColumns;
        //    uint32_t endIdx         = startIdx + numTileColumns - 1;
        //    uint32_t LCUsInTile     = 0;

        //    for (uint32_t idx = 0; idx < numTileColumns; idx ++)
        //    {
        //        LCUsInTile += m_basicFeature->m_hevcPicParams->tile_row_height[m_CurrentTileRow] * m_hevcPicParams->tile_column_width[idx];
        //    }

        //    hucVdencBrcUpdateDmem->StartTileIdx            = (uint8_t)startIdx;
        //    hucVdencBrcUpdateDmem->EndTileIdx              = (uint8_t)endIdx;
        //    hucVdencBrcUpdateDmem->TileSizeInLCU           = (uint16_t)LCUsInTile;
        //}

        // Long term reference
        hucVdencBrcUpdateDmem->IsLongTermRef = CodecHal_PictureIsLongTermRef(m_basicFeature->m_currReconstructedPic);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::SetExtDmemBuffer(VdencHevcHucBrcUpdateDmem *hucVdencBrcUpdateDmem) const
    {
        ENCODE_FUNC_CALL();

        // TCBRC
        hucVdencBrcUpdateDmem->FrameSizeBoostForSceneChange = m_tcbrcQualityBoost;
        hucVdencBrcUpdateDmem->TargetFrameSize              = m_basicFeature->m_hevcPicParams->TargetFrameSize << 3;

        auto CalculatedMaxFrame                   = m_basicFeature->GetProfileLevelMaxFrameSize();
        hucVdencBrcUpdateDmem->UPD_UserMaxFrame   = m_basicFeature->m_hevcSeqParams->UserMaxIFrameSize > 0 ? MOS_MIN(m_basicFeature->m_hevcSeqParams->UserMaxIFrameSize, CalculatedMaxFrame) : CalculatedMaxFrame;
        hucVdencBrcUpdateDmem->UPD_UserMaxFramePB = m_basicFeature->m_hevcSeqParams->UserMaxPBFrameSize > 0 ? MOS_MIN(m_basicFeature->m_hevcSeqParams->UserMaxPBFrameSize, CalculatedMaxFrame) : CalculatedMaxFrame;

        auto UserMaxFrame = m_basicFeature->m_hevcPicParams->CodingType == I_TYPE ? hucVdencBrcUpdateDmem->UPD_UserMaxFrame : hucVdencBrcUpdateDmem->UPD_UserMaxFramePB;
        if (!(UserMaxFrame < hucVdencBrcUpdateDmem->TargetFrameSize / 4)  
            && !(hucVdencBrcUpdateDmem->FrameSizeBoostForSceneChange == 2) 
            && (m_basicFeature->m_hevcSeqParams->LookaheadDepth == 0))
        {
            hucVdencBrcUpdateDmem->TargetFrameSize = 0;
        }
        else if (m_basicFeature->m_hevcPicParams->CodingType == I_TYPE && (m_basicFeature->m_hevcSeqParams->LookaheadDepth == 0))
        {
            hucVdencBrcUpdateDmem->TargetFrameSize += m_basicFeature->m_hevcPicParams->TargetFrameSize;
        }

        hucVdencBrcUpdateDmem->ROMCurrent                   = 8;
        hucVdencBrcUpdateDmem->ROMZero                      = 0;
        
        // LPLA
        RUN_FEATURE_INTERFACE_RETURN(
            HEVCVdencLplaEnc,
            HevcFeatureIDs::hevcVdencLplaEncFeature,
            SetHucBrcUpdateExtBuffer,
            hucVdencBrcUpdateDmem,
            m_pipeline->IsLastPass());

        // CQM
        hucVdencBrcUpdateDmem->CqmEnable = m_basicFeature->m_hevcSeqParams->scaling_list_enable_flag || m_basicFeature->m_hevcPicParams->scaling_list_data_present_flag;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::SetDmemBuffer() const
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        // Program update DMEM
        auto hucVdencBrcUpdateDmem =
            (VdencHevcHucBrcUpdateDmem *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));
        ENCODE_CHK_NULL_RETURN(hucVdencBrcUpdateDmem);
        MOS_ZeroMemory(hucVdencBrcUpdateDmem, sizeof(VdencHevcHucBrcUpdateDmem));

        const_cast<HucBrcUpdatePkt* const>(this)->SetCommonDmemBuffer(hucVdencBrcUpdateDmem);
        SetExtDmemBuffer(hucVdencBrcUpdateDmem);

        m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::SetConstLambdaHucBrcUpdate(void *params) const 
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        auto hucConstData = (VdencHevcHucBrcConstantData *)params;

        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature,
            SetConstLambdaForUpdate, hucConstData, true);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::SetConstDataHuCBrcUpdate() const
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        auto hucConstData = (VdencHevcHucBrcConstantData *)m_allocator->LockResourceForWrite(const_cast<MOS_RESOURCE*>(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]));
        ENCODE_CHK_NULL_RETURN(hucConstData);

        ENCODE_CHK_STATUS_RETURN(SetConstLambdaHucBrcUpdate(hucConstData));
        RUN_FEATURE_INTERFACE_RETURN(HEVCEncodeBRC, HevcFeatureIDs::hevcBrcFeature,
            SetConstForUpdate, hucConstData);

        // starting location in batch buffer for each slice
        uint32_t baseLocation = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
        uint32_t currentLocation = baseLocation;

        auto slcData = m_basicFeature->m_slcData;
        // HCP_WEIGHTSOFFSETS_STATE + HCP_SLICE_STATE + HCP_PAK_INSERT_OBJECT + VDENC_WEIGHT_OFFSETS_STATE
        for (uint32_t slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            m_basicFeature->m_curNumSlices = slcCount;
            auto hevcSlcParams = &m_basicFeature->m_hevcSliceParams[slcCount];
            // HuC FW require unit in Bytes
            hucConstData->Slice[slcCount].SizeOfCMDs
                = (uint16_t)(m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount]);

            // HCP_WEIGHTOFFSET_STATE cmd
            RUN_FEATURE_INTERFACE_RETURN(
                HevcVdencWeightedPred,
                HevcFeatureIDs::hevcVdencWpFeature,
                SetHucBrcUpdateConstData,
                *hevcSlcParams,
                slcCount,
                m_hcpWeightOffsetStateCmdSize,
                currentLocation,
                *hucConstData);

            // HCP_SLICE_STATE cmd
            hucConstData->Slice[slcCount].SliceState_StartInBytes = (uint16_t)currentLocation;  // HCP_WEIGHTOFFSET is not needed
            currentLocation += m_hcpSliceStateCmdSize;

            // VDENC_WEIGHT_OFFSETS_STATE cmd
            hucConstData->Slice[slcCount].VdencWeightOffset_StartInBytes                      // VdencWeightOffset cmd is the last one expect BatchBufferEnd cmd when adaptive tu disabled
                = (uint16_t)(baseLocation + hucConstData->Slice[slcCount].SizeOfCMDs - m_vdencWeightOffsetStateCmdSize - m_miBatchBufferEndCmdSize - m_alignSize[slcCount]);

            currentLocation += m_miBatchBufferEndCmdSize;

            // logic from PakInsertObject cmd
            uint32_t bitSize         = (m_basicFeature->m_hevcSeqParams->SliceSizeControl) ? (hevcSlcParams->BitLengthSliceHeaderStartingPortion) : slcData[slcCount].BitSize;  // 40 for HEVC VDEnc Dynamic Slice
            uint32_t byteSize = (bitSize + 7) >> 3;
            uint32_t sliceHeaderSizeInBytes = (bitSize + 7) >> 3;
            // 1st PakInsertObject cmd with AU, SPS, PPS headers only exists for the first slice
            if (slcCount == 0)
            {
                // assumes that there is no 3rd PakInsertObject cmd for SSC
                currentLocation += m_1stPakInsertObjectCmdSize;
            }

            hucConstData->Slice[slcCount].SliceHeaderPIO_StartInBytes = (uint16_t)currentLocation;

            // HuC FW requires true slice header size in bits without byte alignment
            hucConstData->Slice[slcCount].SliceHeader_SizeInBits = (uint16_t)(sliceHeaderSizeInBytes * 8);
            if (!m_pipeline->IsFirstPass())
            {
                PBSBuffer bsBuffer = &(m_basicFeature->m_bsBuffer);
                ENCODE_CHK_NULL_RETURN(bsBuffer);
                ENCODE_CHK_NULL_RETURN(bsBuffer->pBase);
                uint8_t *sliceHeaderLastByte = (uint8_t*)(bsBuffer->pBase + slcData[slcCount].SliceOffset + sliceHeaderSizeInBytes - 1);
                for (auto i = 0; i < 8; i++)
                {
                    uint8_t mask = 1 << i;
                    if (*sliceHeaderLastByte & mask)
                    {
                        hucConstData->Slice[slcCount].SliceHeader_SizeInBits -= (i + 1);
                        break;
                    }
                }
            }

            baseLocation += hucConstData->Slice[slcCount].SizeOfCMDs;
            currentLocation = baseLocation;
        }

        m_allocator->UnLock(const_cast<MOS_RESOURCE*>(&m_vdencBrcConstDataBuffer[m_pipeline->m_currRecycledBufIdx]));

        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcPicParams);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);

        ENCODE_CHK_STATUS_RETURN(SetTcbrcMode());

        if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0 && m_bufConstSizeFlagForAdaptiveTU == false)
        {
            m_bufConstSizeFlagForAdaptiveTU = true;
            m_hwInterface->m_vdencBatchBufferPerSliceConstSize += m_vdencItf->MHW_GETSIZE_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)();
        }

        ENCODE_CHK_STATUS_RETURN(ConstructBatchBufferHuCBRC(&m_vdencReadBatchBufferOrigin[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));

        if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
        {
            auto original_TU = m_basicFeature->m_targetUsage;
            m_basicFeature->m_targetUsage = m_basicFeature->m_hevcSeqParams->TargetUsage = 7;
            ENCODE_CHK_STATUS_RETURN(ConstructBatchBufferHuCBRC(&m_vdencReadBatchBufferTU7[m_pipeline->m_currRecycledBufIdx][m_pipeline->GetCurrentPass()]));
            m_basicFeature->m_targetUsage = m_basicFeature->m_hevcSeqParams->TargetUsage = original_TU;
        }

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        uint16_t perfTag = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_SECOND_PASS;
        uint16_t pictureType = m_basicFeature->m_pictureCodingType;
        if (m_basicFeature->m_pictureCodingType == B_TYPE && m_basicFeature->m_ref.IsLowDelay())
        {
            pictureType = 0;
        }
        SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_UPDATE));

#if _SW_BRC
    if (!m_swBrc || !m_swBrc->SwBrcEnabled())
    {
#endif
        // Write HUC_STATUS mask: DW1 (mask value)
        auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeDataParams                  = {};
        storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        storeDataParams.dwResourceOffset = sizeof(uint32_t);
        storeDataParams.dwValue          = CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

        // store HUC_STATUS register: DW0 (actual value)
        ENCODE_CHK_COND_RETURN((m_vdboxIndex > MHW_VDBOX_NODE_1), "ERROR - vdbox index exceed the maximum");
        auto mmioRegisters              = m_hucItf->GetMmioRegisters(m_vdboxIndex);
        auto &storeRegParams            = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeDataParams                 = {};
        storeRegParams.presStoreBuffer  = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
        storeRegParams.dwOffset         = 0;
        storeRegParams.dwRegister       = mmioRegisters->hucStatusRegOffset;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
#if _SW_BRC
    }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        ENCODE_CHK_NULL_RETURN(batchBuffer);

        m_batchbufferAddr = (uint8_t *)m_allocator->LockResourceForWrite(batchBuffer);
        ENCODE_CHK_NULL_RETURN(m_batchbufferAddr);

        ENCODE_CHK_STATUS_RETURN(ConstructGroup1Cmds());
        ENCODE_CHK_STATUS_RETURN(ConstructGroup2Cmds());
        ENCODE_CHK_STATUS_RETURN(ConstructGroup3Cmds());

        m_allocator->UnLock(batchBuffer);

        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePkt::ConstructGroup1Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

        // 1st Group : PIPE_MODE_SELECT
        // set PIPE_MODE_SELECT command
        // This is not needed for GEN11/GEN12 as single pass SAO is supported
        // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
        auto &mfxWaitParams                 = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&constructedCmdBuf));

        SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &constructedCmdBuf);

        mfxWaitParams                       = {};
        mfxWaitParams.iStallVdboxPipeline   = true;
        ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&constructedCmdBuf));

        MHW_BATCH_BUFFER TempBatchBuffer = {};
        TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
        TempBatchBuffer.pData       = m_batchbufferAddr;

        // set MI_BATCH_BUFFER_END command
        int32_t cmdBufOffset = constructedCmdBuf.iOffset;

        TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
        TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
        constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
        constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
        constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

        m_miBatchBufferEndCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

        uint32_t alignSize = m_hwInterface->m_vdencBatchBuffer1stGroupSize - constructedCmdBuf.iOffset;
        if (alignSize)
        {
            for (uint32_t i = 0; i < (alignSize / 4); i++)
            {
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
            }
        }
        ENCODE_CHK_COND_RETURN(
            (m_hwInterface->m_vdencBatchBuffer1stGroupSize != constructedCmdBuf.iOffset), 
            "ERROR - constructed cmd size is mismatch with calculated");
 
        return eStatus;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HucBrcUpdatePkt)
    {
        params.bNotFirstPass = !m_pipeline->IsFirstPass();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::ConstructGroup2Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
        constructedCmdBuf.pCmdPtr = constructedCmdBuf.pCmdBase + constructedCmdBuf.iOffset / 4;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE) - constructedCmdBuf.iOffset;

        SETPAR_AND_ADDCMD(VDENC_CMD1, m_vdencItf, &constructedCmdBuf);
        m_basicFeature->m_picStateCmdStartInBytes = constructedCmdBuf.iOffset;

        // set HCP_PIC_STATE command
        SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &constructedCmdBuf);
        m_cmd2StartInBytes = constructedCmdBuf.iOffset;

        SETPAR_AND_ADDCMD(VDENC_CMD2, m_vdencItf, &constructedCmdBuf);

        // set MI_BATCH_BUFFER_END command
        MHW_BATCH_BUFFER TempBatchBuffer = {};
        TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
        TempBatchBuffer.pData       = m_batchbufferAddr;

        TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
        TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
        ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
        constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
        constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
        constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

        uint32_t alignSize = m_hwInterface->m_vdencBatchBuffer2ndGroupSize + m_hwInterface->m_vdencBatchBuffer1stGroupSize - constructedCmdBuf.iOffset;
        if (alignSize)
        {
            for (uint32_t i = 0; i < (alignSize / 4); i++)
            {
                ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
            }
        }
        ENCODE_CHK_COND_RETURN(
            (m_hwInterface->m_vdencBatchBuffer2ndGroupSize + m_hwInterface->m_vdencBatchBuffer1stGroupSize != constructedCmdBuf.iOffset), 
            "ERROR - constructed cmd size is mismatch with calculated");

        return eStatus;
    }

    MOS_STATUS HucBrcUpdatePkt::ConstructGroup3Cmds()
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        uint32_t cmdBufOffset = 0;

        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_slcData);

        MOS_COMMAND_BUFFER constructedCmdBuf;
        MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
        constructedCmdBuf.pCmdBase = (uint32_t *)m_batchbufferAddr;
        constructedCmdBuf.iOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
        constructedCmdBuf.pCmdPtr = constructedCmdBuf.pCmdBase + constructedCmdBuf.iOffset / 4;
        constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE) - constructedCmdBuf.iOffset;

        // slice level cmds for each slice
        PCODEC_ENCODER_SLCDATA slcData = m_basicFeature->m_slcData;
        for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_basicFeature->m_numSlices; slcCount++)
        {
            m_basicFeature->m_curNumSlices    = slcCount;
            m_basicFeature->m_lastSliceInTile = true;

            if (m_pipeline->IsFirstPass())
            {
                slcData[slcCount].CmdOffset = startLCU * (m_hcpItf->GetHcpPakObjSize()) * sizeof(uint32_t);
            }

            RUN_FEATURE_INTERFACE_RETURN(HevcEncodeTile, HevcFeatureIDs::encodeTile, SetCurrentTileFromSliceIndex, slcCount, m_pipeline);

            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount] = 0;
            m_basicFeature->m_vdencBatchBufferPerSlicePart2Size[slcCount] = 0;

            // set HCP_WEIGHTOFFSET_STATE command
            // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
            //        If zero, then this command is not issued.
            AddAllCmds_HCP_WEIGHTOFFSET_STATE(&constructedCmdBuf);

            // set HCP_SLICE_STATE command
            cmdBufOffset = constructedCmdBuf.iOffset;
            SETPAR_AND_ADDCMD(HCP_SLICE_STATE, m_hcpItf, &constructedCmdBuf);
            m_hcpSliceStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

            // set MI_BATCH_BUFFER_END command
            MHW_BATCH_BUFFER TempBatchBuffer = {};
            TempBatchBuffer.iSize            = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
            TempBatchBuffer.pData            = m_batchbufferAddr;

            TempBatchBuffer.iCurrent   = constructedCmdBuf.iOffset;
            TempBatchBuffer.iRemaining = constructedCmdBuf.iRemaining;
            ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
            constructedCmdBuf.pCmdPtr += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
            constructedCmdBuf.iOffset    = TempBatchBuffer.iCurrent;
            constructedCmdBuf.iRemaining = TempBatchBuffer.iRemaining;

            m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[slcCount] = constructedCmdBuf.iOffset;

            AddAllCmds_HCP_PAK_INSERT_OBJECT_SLICE(&constructedCmdBuf);

            cmdBufOffset = constructedCmdBuf.iOffset;
            SETPAR_AND_ADDCMD(VDENC_WEIGHTSOFFSETS_STATE, m_vdencItf, &constructedCmdBuf);
            if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
            {
                SETPAR_AND_ADDCMD(VDENC_HEVC_VP9_TILE_SLICE_STATE, m_vdencItf, &constructedCmdBuf);
            }
            m_vdencWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

            // set MI_BATCH_BUFFER_END command
            TempBatchBuffer = {};
            TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
            TempBatchBuffer.pData       = m_batchbufferAddr;

            TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
            TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
            ENCODE_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
            constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
            constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
            constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

            m_alignSize[slcCount] = MOS_ALIGN_CEIL(constructedCmdBuf.iOffset, 64) - constructedCmdBuf.iOffset;
            if (m_alignSize[slcCount] > 0)
            {
                for (uint32_t i = 0; i < (m_alignSize[slcCount] / 4); i++)
                {
                    ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_NOOP)(&constructedCmdBuf));
                }
            }
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[slcCount] += m_alignSize[slcCount];
            m_basicFeature->m_vdencBatchBufferPerSlicePart2Size[slcCount] = constructedCmdBuf.iOffset - m_basicFeature->m_vdencBatchBufferPerSlicePart2Start[slcCount];
            startLCU += m_basicFeature->m_hevcSliceParams[slcCount].NumLCUsInSlice;
        }

        m_slbDataSizeInBytes = constructedCmdBuf.iOffset;

        return eStatus;
    }

    uint32_t HucBrcUpdatePkt::GetMaxAllowedSlices(uint8_t levelIdc) const 
    {
        uint32_t maxAllowedNumSlices = 0;

        switch (levelIdc)
        {
        case 10:
        case 20:
            maxAllowedNumSlices = 16;
            break;
        case 21:
            maxAllowedNumSlices = 20;
            break;
        case 30:
            maxAllowedNumSlices = 30;
            break;
        case 31:
            maxAllowedNumSlices = 40;
            break;
        case 40:
        case 41:
            maxAllowedNumSlices = 75;
            break;
        case 50:
        case 51:
        case 52:
            maxAllowedNumSlices = 200;
            break;
        case 60:
        case 61:
        case 62:
            maxAllowedNumSlices = 600;
            break;
        default:
            maxAllowedNumSlices = 0;
            break;
        }

        return maxAllowedNumSlices;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS HucBrcUpdatePkt::DumpHucBrcUpdate(bool isInput)
    {
        ENCODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        ENCODE_CHK_NULL_RETURN(debugInterface);

        auto virtualAddrParams = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();
        int32_t currentPass = m_pipeline->GetCurrentPass();

        if (isInput)
        {
            //Dump HucBrcUpdate input buffers
            ENCODE_CHK_STATUS_RETURN(debugInterface->DumpHucDmem(
                &m_vdencBrcUpdateDmemBuffer[m_pipeline->m_currRecycledBufIdx][currentPass],
                m_vdencBrcUpdateDmemBufferSize,
                currentPass,
                hucRegionDumpUpdate));

            // Region 1 - VDENC Statistics Buffer dump
            HevcBasicFeature *hevcBasicFeature  = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
            ENCODE_CHK_NULL_RETURN(hevcBasicFeature);
            uint32_t vdencBRCStatsBufferSize = 1216;
            uint32_t size = MOS_ALIGN_CEIL(vdencBRCStatsBufferSize * hevcBasicFeature->m_maxTileNumber, CODECHAL_PAGE_SIZE);
            ENCODE_CHK_STATUS_RETURN(DumpRegion(1, "_VdencStats", true, hucRegionDumpUpdate, size));

            // Region 2 - PAK Statistics Buffer dump
            size = MOS_ALIGN_CEIL(HevcBasicFeature::m_sizeOfHcpPakFrameStats * hevcBasicFeature->m_maxTileNumber, CODECHAL_PAGE_SIZE);
            ENCODE_CHK_STATUS_RETURN(DumpRegion(2, "_PakStats", true, hucRegionDumpUpdate, size));

            // Region 3 - Input Origin SLB Buffer
            ENCODE_CHK_STATUS_RETURN(DumpRegion(3, "_Slb_Origin", true, hucRegionDumpUpdate, m_hwInterface->m_vdencReadBatchBufferSize));

            // Region 4 - Constant Data Buffer dump
            ENCODE_CHK_STATUS_RETURN(DumpRegion(4, "_ConstData", true, hucRegionDumpUpdate, m_vdencBrcConstDataBufferSize));

            // Region 7 - Slice Stat Streamout (Input)
            ENCODE_CHK_STATUS_RETURN(DumpRegion(7, "_SliceStat", true, hucRegionDumpUpdate, CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6 * CODECHAL_CACHELINE_SIZE));

            // Region 8 - PAK MMIO Buffer dump
            ENCODE_CHK_STATUS_RETURN(DumpRegion(8, "_PakMmio", true, hucRegionDumpUpdate, sizeof(CodechalVdencHevcPakInfo)));

            // Region 9 - Streamin Buffer for ROI (Input)
            auto streamInBufferSize = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;
            ENCODE_CHK_STATUS_RETURN(DumpRegion(9, "_RoiStreamin", true, hucRegionDumpUpdate, streamInBufferSize));

            // Region 10 - Delta QP for ROI Buffer
            auto vdencDeltaQpBuffer = virtualAddrParams.regionParams[10].presRegion;
            if (vdencDeltaQpBuffer)
            {
                ENCODE_CHK_STATUS_RETURN(DumpRegion(10, "_DeltaQp", true, hucRegionDumpUpdate, vdencDeltaQpBuffer->iSize));
            }

            // Region 12 - Input TU7 SLB Buffer
            if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
            {      
                ENCODE_CHK_STATUS_RETURN(DumpRegion(12, "_Slb_TU7", true, hucRegionDumpUpdate, m_hwInterface->m_vdencReadBatchBufferSize));
            }
        }
        else
        {
            // Region 5 - Output SLB Buffer
            ENCODE_CHK_STATUS_RETURN(DumpRegion(5, "_Slb", false, hucRegionDumpUpdate, m_hwInterface->m_vdenc2ndLevelBatchBufferSize));

            // Region 11 - Output ROI Streamin Buffer
            auto vdencOutputROIStreaminBuffer = virtualAddrParams.regionParams[11].presRegion;
            if (vdencOutputROIStreaminBuffer)
            {
                ENCODE_CHK_STATUS_RETURN(DumpRegion(11, "_RoiStreamin", false, hucRegionDumpUpdate, vdencOutputROIStreaminBuffer->iSize));
            }
        }

        // Region 0 - History Buffer dump (Input/Output)
        ENCODE_CHK_STATUS_RETURN(DumpRegion(0, "_History", isInput, hucRegionDumpUpdate));

        // Region 6 - Data from Pictures for Weighted Prediction (Input/Output)
        ENCODE_CHK_STATUS_RETURN(DumpRegion(6, "_PicsData", isInput, hucRegionDumpUpdate, CODECHAL_PAGE_SIZE * 4));

        // Region 15 - Debug Output
        auto debugBuffer = virtualAddrParams.regionParams[15].presRegion;
        if (debugBuffer)
        {
            ENCODE_CHK_STATUS_RETURN(DumpRegion(15, "_Debug", isInput, hucRegionDumpUpdate, debugBuffer->iSize));
        }
        
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::DumpInput()
    {
        ENCODE_FUNC_CALL();
        DumpHucBrcUpdate(true);
        return MOS_STATUS_SUCCESS;
    }
#endif

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HucBrcUpdatePkt)
    {
        params.codecStandardSelect = CodecHal_GetStandardFromMode(m_basicFeature->m_mode) - CODECHAL_HCP_BASE;
        params.bStreamOutEnabled   = true;
        params.bVdencEnabled       = true;
        params.multiEngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        params.pipeWorkMode        = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;

        MhwCpInterface *cpInterface     = m_hwInterface->GetCpInterface();
        bool            twoPassScalable = params.multiEngineMode != MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY && !params.bTileBasedReplayMode;

        ENCODE_CHK_NULL_RETURN(cpInterface);
        params.setProtectionSettings = [=](uint32_t *data) { return cpInterface->SetProtectionSettingsForHcpPipeModeSelect(data, twoPassScalable); };

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HucBrcUpdatePkt)
    {
        ENCODE_FUNC_CALL();

        params.intrareffetchdisable = m_basicFeature->m_pakOnlyPass;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::AddAllCmds_HCP_PAK_INSERT_OBJECT_SLICE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PAK_INSERT_OBJECT)();

        uint32_t                   cmdBufOffset    = 0;
        uint32_t                   bitSize         = 0;
        uint32_t                   offSet          = 0;
        PCODECHAL_NAL_UNIT_PARAMS *ppNalUnitParams = (CODECHAL_NAL_UNIT_PARAMS **)m_basicFeature->m_nalUnitParams;
        PBSBuffer                  pBsBuffer       = &(m_basicFeature->m_bsBuffer);

        params = {};
        // Insert slice header
        params.bLastHeader              = true;
        params.bEmulationByteBitsInsert = true;

        // App does the slice header packing, set the skip count passed by the app
        PCODEC_ENCODER_SLCDATA slcData    = m_basicFeature->m_slcData;
        uint32_t               currSlcIdx = m_basicFeature->m_curNumSlices;

        params.uiSkipEmulationCheckCount = slcData[currSlcIdx].SkipEmulationByteCount;
        bitSize                          = slcData[currSlcIdx].BitSize;
        offSet                           = slcData[currSlcIdx].SliceOffset;

        if (m_basicFeature->m_hevcSeqParams->SliceSizeControl)
        {
            params.bLastHeader                = false;
            params.bEmulationByteBitsInsert   = false;
            bitSize                           = m_basicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            params.bResetBitstreamStartingPos = true;
            params.dwPadding                  = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw           = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }

            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            uint32_t byteSize = (bitSize + 7) >> 3;
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }

            // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
            params.bLastHeader = true;
            bitSize            = slcData[currSlcIdx].BitSize - m_basicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            offSet += ((m_basicFeature->m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);  // Skips the first 5 bytes which is Start Code + Nal Unit Header
            params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }
            params.bResetBitstreamStartingPos = true;
            cmdBufOffset                      = cmdBuffer->iOffset;
            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            byteSize = (bitSize + 7) >> 3;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += (cmdBuffer->iOffset - cmdBufOffset);
        }
        else
        {
            params.dwPadding        = (MOS_ALIGN_CEIL((bitSize + 7) >> 3, sizeof(uint32_t))) / sizeof(uint32_t);
            params.dataBitsInLastDw = bitSize % 32;
            if (params.dataBitsInLastDw == 0)
            {
                params.dataBitsInLastDw = 32;
            }
            m_hcpItf->MHW_ADDCMD_F(HCP_PAK_INSERT_OBJECT)(cmdBuffer);
            uint32_t byteSize = (bitSize + 7) >> 3;
            m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;
            if (byteSize)
            {
                MHW_MI_CHK_NULL(pBsBuffer);
                MHW_MI_CHK_NULL(pBsBuffer->pBase);
                uint8_t *data = (uint8_t *)(pBsBuffer->pBase + offSet);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, nullptr, data, byteSize));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::AddAllCmds_HCP_WEIGHTOFFSET_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(cmdBuffer);

        auto &params                 = m_hcpItf->MHW_GETPAR_F(HCP_WEIGHTOFFSET_STATE)();
        params                       = {};
        auto wpFeature = dynamic_cast<HevcVdencWeightedPred *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcVdencWpFeature));
        ENCODE_CHK_NULL_RETURN(wpFeature);

        if (wpFeature->IsEnabled() && m_basicFeature->m_hevcPicParams->bEnableGPUWeightedPrediction)
        {
            MHW_CHK_STATUS_RETURN(wpFeature->MHW_SETPAR_F(HCP_WEIGHTOFFSET_STATE)(params));

            uint32_t cmdBufOffset = 0;
            // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
            if (m_basicFeature->m_hevcSliceParams->slice_type == encodeHevcPSlice || m_basicFeature->m_hevcSliceParams->slice_type == encodeHevcBSlice)
            {
                params.ucList = LIST_0;
                cmdBufOffset = cmdBuffer->iOffset;
                m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(cmdBuffer);
                m_hcpWeightOffsetStateCmdSize = cmdBuffer->iOffset - cmdBufOffset;
                // 1st HcpWeightOffset cmd is not always inserted (except weighted prediction + P, B slices)
                m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += m_hcpWeightOffsetStateCmdSize;
            }

            // 2nd HCP_WEIGHTOFFSET_STATE cmd - B only
            if (m_basicFeature->m_hevcSliceParams->slice_type == encodeHevcBSlice)
            {
                params.ucList = LIST_1;
                cmdBufOffset = cmdBuffer->iOffset;
                m_hcpItf->MHW_ADDCMD_F(HCP_WEIGHTOFFSET_STATE)(cmdBuffer);
                m_hcpWeightOffsetStateCmdSize = cmdBuffer->iOffset - cmdBufOffset;
                // 1st HcpWeightOffset cmd is not always inserted (except weighted prediction + P, B slices)
                m_basicFeature->m_vdencBatchBufferPerSliceVarSize[m_basicFeature->m_curNumSlices] += m_hcpWeightOffsetStateCmdSize;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucBrcUpdatePkt::SetTcbrcMode()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);
        ENCODE_CHK_NULL_RETURN(m_hwInterface);

        if (m_basicFeature->m_newSeq)
        {
            //tcbrc mode set by scenarioInfo
            uint8_t tcbrcQualityBoostFromScenario = 0;
            switch (m_basicFeature->m_hevcSeqParams->ScenarioInfo)
            {
            case ESCENARIO_REMOTEGAMING:
                tcbrcQualityBoostFromScenario = 0;
                break;
            case ESCENARIO_VIDEOCONFERENCE:
                tcbrcQualityBoostFromScenario = 1;
                break;
            default:
                tcbrcQualityBoostFromScenario = 0;
            }

#if (_DEBUG || _RELEASE_INTERNAL)
            //tcbrc mode override by reg key
            uint8_t tcbrcQualityBoostFromRegkey = 3;

            MediaUserSetting::Value outValue;
            ReadUserSetting(m_userSettingPtr,
                outValue,
                "TCBRC Quality Boost Mode",
                MediaUserSetting::Group::Sequence);
            tcbrcQualityBoostFromRegkey = static_cast<uint8_t>(outValue.Get<int32_t>());
            //if FrameSizeBoostForSceneChange is set by regkey, then override it
            if (tcbrcQualityBoostFromRegkey == 0 || tcbrcQualityBoostFromRegkey == 1 || tcbrcQualityBoostFromRegkey == 2)
            {
                m_tcbrcQualityBoost = tcbrcQualityBoostFromRegkey;
                ENCODE_VERBOSEMESSAGE("TCBRC FrameSizeBoostForSceneChange is override by regkey!");
            }
            else
#endif
            {
                m_tcbrcQualityBoost = tcbrcQualityBoostFromScenario;
            }
#if (_DEBUG || _RELEASE_INTERNAL)
            ReportUserSettingForDebug(m_userSettingPtr,
                "TCBRC Quality Boost Mode",
                m_tcbrcQualityBoost,
                MediaUserSetting::Group::Sequence);
#endif
        }
        return MOS_STATUS_SUCCESS;
    }

    }
