/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_vvc_basic_feature.cpp
//! \brief    Defines the common interface for decode vvc parameter
//!

#include "decode_vvc_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"

namespace decode
{
    VvcBasicFeature::~VvcBasicFeature() { }

    MOS_STATUS VvcBasicFeature::Init(void *setting)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(setting);

        DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));
        CodechalSetting *codecSettings = (CodechalSetting*)setting;
        m_shortFormatInUse = codecSettings->shortFormatInUse;

        DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));
        DECODE_CHK_STATUS(m_mvBuffers.Init(m_hwInterface, *m_allocator, *this,
                                       vvcNumInitialMvBuffers));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::UpdateAPS(void *params)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(params);

        CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;

        // ALF APS
        if (m_vvcPicParams->m_numAlfBuffers > 0 && decodeParams->m_deblockData != nullptr)
        {
            uint32_t actualBufNum = decodeParams->m_deblockDataSize / sizeof(CodecVvcAlfData);
            if (m_vvcPicParams->m_numAlfBuffers > actualBufNum)
            {
                DECODE_ASSERTMESSAGE("Received ALF buffer size < claimed buffer size in pic params buffer.\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            CodecVvcAlfData* alfData = (CodecVvcAlfData*)decodeParams->m_deblockData;
            for (uint32_t i = 0; i < actualBufNum; i++)
            {
                if (alfData->m_apsAdaptationParameterSetId >= vvcMaxAlfNum)
                {
                    DECODE_ASSERTMESSAGE("ALF: Invalid APS set ID from App.\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                m_concealAlfMask &= ~(1 << alfData->m_apsAdaptationParameterSetId);
                if (MOS_STATUS_INVALID_PARAMETER == CheckAlfRange(alfData))
                {
                    m_concealAlfMask |= (1 << alfData->m_apsAdaptationParameterSetId);
                }

                MOS_SecureMemcpy(&m_alfApsArray[alfData->m_apsAdaptationParameterSetId], sizeof(CodecVvcAlfData), alfData, sizeof(CodecVvcAlfData));
                m_activeAlfMask |= (1 << alfData->m_apsAdaptationParameterSetId);

                alfData++;
            }

            //calc accumulated valid ALF number
            m_numAlf = 0;
            uint8_t alfFlag = m_activeAlfMask;
            for (auto i = 0; i < vvcMaxAlfNum; i++)
            {
                m_numAlf += (alfFlag >> i) & 0x1;
            }
        }
        else if(m_vvcPicParams->m_numAlfBuffers > 0 && decodeParams->m_deblockData == nullptr)
        {
            DECODE_ASSERTMESSAGE("Inconsistent between pic params ALF num and actual ALF buffer\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // decide ALF error concealment
        if(m_concealAlfMask)
        {
            DECODE_ASSERTMESSAGE("Error concealed: Disable ALF since out-of-range ALF parameters detected.\n");
            m_vvcPicParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag    = 0;
            m_vvcPicParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag  = 0;
            m_vvcPicParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag    = 1;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfEnabledFlag       = 0;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag     = 0;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag     = 0;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag   = 0;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag   = 0;
        }

        // LMCS APS
        if (m_vvcPicParams->m_numLmcsBuffers > 0 && decodeParams->m_macroblockParams != nullptr)
        {
            if (m_vvcPicParams->m_numLmcsBuffers > decodeParams->m_numMacroblocks)
            {
                DECODE_ASSERTMESSAGE("Received LMCS buffer size < claimed buffer size in pic params buffer.\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            CodecVvcLmcsData *lmcsData = (CodecVvcLmcsData*)decodeParams->m_macroblockParams;
            for (uint32_t i = 0; i < decodeParams->m_numMacroblocks; i++)
            {
                if (lmcsData->m_apsAdaptationParameterSetId >= vvcMaxLmcsNum)
                {
                    DECODE_ASSERTMESSAGE("LMCS: Invalid APS set ID from App.\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                MOS_SecureMemcpy(&m_lmcsApsArray[lmcsData->m_apsAdaptationParameterSetId], sizeof(CodecVvcLmcsData), lmcsData, sizeof(CodecVvcLmcsData));
                m_activeLmcsMask |= (1 << lmcsData->m_apsAdaptationParameterSetId);
                m_lmcsReshaperReady &= ~(1 << lmcsData->m_apsAdaptationParameterSetId);//reset the flag to indicate reshape info not derived yet

                lmcsData++;
            }

            //calc accumulated valid LMCS number
            m_numLmcs = 0;
            uint8_t lmcsMask = m_activeLmcsMask;
            for (auto i = 0; i < vvcMaxLmcsNum; i++)
            {
                m_numLmcs += (lmcsMask >> i) & 0x1;
            }
        }
        else if(m_vvcPicParams->m_numLmcsBuffers > 0 && decodeParams->m_macroblockParams == nullptr)
        {
            DECODE_ASSERTMESSAGE("Inconsistent between pic params LMCS num and actual LMCS buffer\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Scaling List APS
        if (m_vvcPicParams->m_numScalingMatrixBuffers > 0 && decodeParams->m_iqMatrixBuffer != nullptr)
        {
            uint32_t actualBufNum = decodeParams->m_iqMatrixSize / sizeof(CodecVvcQmData);
            if (m_vvcPicParams->m_numScalingMatrixBuffers > actualBufNum)
            {
                DECODE_ASSERTMESSAGE("Received Scaling List APS buffer size < claimed buffer size in pic params buffer.\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            CodecVvcQmData* scalingListData = (CodecVvcQmData*)decodeParams->m_iqMatrixBuffer;
            for (uint32_t i = 0; i < actualBufNum; i++)
            {
                if (scalingListData->m_apsAdaptationParameterSetId >= vvcMaxScalingMatrixNum)
                {
                    DECODE_ASSERTMESSAGE("Scaling List: Invalid APS set ID from App.\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                MOS_SecureMemcpy(&m_scalingListArray[scalingListData->m_apsAdaptationParameterSetId], sizeof(CodecVvcQmData), scalingListData, sizeof(CodecVvcQmData));
                m_activeScalingListMask |= (1 << scalingListData->m_apsAdaptationParameterSetId);

                scalingListData++;
            }

            //calc accumulated valid scaling list number
            m_numScalingList = 0;
            uint8_t scalingListMask = m_activeScalingListMask;
            for (auto i = 0; i < vvcMaxScalingMatrixNum; i++)
            {
                m_numScalingList += (scalingListMask >> i) & 0x1;
            }
        }
        else if(m_vvcPicParams->m_numScalingMatrixBuffers > 0 && decodeParams->m_iqMatrixBuffer == nullptr)
        {
            DECODE_ASSERTMESSAGE("Scaling List APS: Inconsistent between pic params Scaling List num and actual Scaling List buffer\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::Update(void *params)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(params);

        DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

        CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
        m_dataSize = decodeParams->m_dataSize;
        m_vvcPicParams  = static_cast<CodecVvcPicParams*>(decodeParams->m_picParams);
        DECODE_CHK_NULL(m_vvcPicParams);
        m_vvcSliceParams  = static_cast<CodecVvcSliceParams*>(decodeParams->m_sliceParams);
        DECODE_CHK_NULL(m_vvcSliceParams);
        m_curSlice = 0;

        DECODE_CHK_STATUS(SetPictureStructs(decodeParams));

        // APS update
        DECODE_CHK_STATUS(UpdateAPS(params));

        // Partition & RPL update
        m_subPicParams      = static_cast<CodecVvcSubpicParam*>(decodeParams->m_extPicParams);
        if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
        {
            DECODE_CHK_NULL(m_subPicParams);
        }
        m_sliceStructParams = static_cast<CodecVvcSliceStructure*>(decodeParams->m_extSliceParams);
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag && m_vvcPicParams->m_numSliceStructsMinus1 > 0)
        {
            DECODE_CHK_NULL(m_sliceStructParams);
        }
        m_rplParams         = static_cast<CodecVvcRplStructure*>(decodeParams->m_refParams);
        m_tileParams        = static_cast<CodecVvcTileParam*>(decodeParams->m_tileParams);

        if (m_shortFormatInUse)
        {
            DECODE_CHK_STATUS(UpdateNumRefForList());
        }

        DECODE_CHK_STATUS(ReconstructPartition(decodeParams));

        // Error detection and concealment
        DECODE_CHK_STATUS(ErrorDetectAndConceal());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::CheckAlfRange(CodecVvcAlfData* alfData)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(alfData);
        MOS_STATUS res = MOS_STATUS_SUCCESS;

        // Luma
        if (alfData->m_alfFlags.m_fields.m_alfLumaFilterSignalFlag)
        {
            CHECK_RANGE(alfData->m_alfLumaNumFiltersSignalledMinus1, 0, 24);
            for (auto i = 0; i < 25; i++)
            {
                CHECK_RANGE(alfData->m_alfLumaCoeffDeltaIdx[i], 0, 24);
            }
            for (auto i = 0; i <= alfData->m_alfLumaNumFiltersSignalledMinus1; i++)
            {
                for (auto j = 0; j < 12; j++)
                {
                    CHECK_RANGE(alfData->m_alfLumaClipIdx[i][j], 0, 3);
                }
            }
        }

        // Chroma
        if (alfData->m_alfFlags.m_fields.m_alfChromaFilterSignalFlag)
        {
            CHECK_RANGE(alfData->m_alfChromaNumAltFiltersMinus1, 0, 7);
            for (auto i = 0; i <= alfData->m_alfChromaNumAltFiltersMinus1; i++)
            {
                for (auto j = 0; j < 6; j++)
                {
                    CHECK_RANGE(alfData->m_alfChromaClipIdx[i][j], 0, 3);
                }
            }
        }

        // CC Cb
        if (alfData->m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag)
        {
            CHECK_RANGE(alfData->m_alfCcCbFiltersSignalledMinus1, 0, 3);
            for (auto i = 0; i <= alfData->m_alfCcCbFiltersSignalledMinus1; i++)
            {
                for (auto j = 0; j < 7; j++)
                {
                    CHECK_RANGE(alfData->m_ccAlfApsCoeffCb[i][j], -64, 64);
                }
            }
        }

        // CC Cr
        if (alfData->m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag)
        {
            CHECK_RANGE(alfData->m_alfCcCrFiltersSignalledMinus1, 0, 3);
            for (auto i = 0; i <= alfData->m_alfCcCrFiltersSignalledMinus1; i++)
            {
                for (auto j = 0; j < 7; j++)
                {
                    CHECK_RANGE(alfData->m_ccAlfApsCoeffCr[i][j], -64, 64);
                }
            }
        }

        return res;
    }

    MOS_STATUS VvcBasicFeature::SliceErrorHandlingLF()
    {
        DECODE_FUNC_CALL()
        DECODE_CHK_NULL(m_vvcPicParams);
        DECODE_CHK_NULL(m_vvcSliceParams);

        m_sliceIdxInOrder.clear();
        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            m_sliceIdxInOrder.push_back(0);
            for (uint16_t i = 1; i < m_numSlices; i++)
            {
                CodecVvcSliceParams temp = m_vvcSliceParams[i];
                uint16_t startTile = temp.m_shSliceAddress;
                int16_t j = i - 1;
                for (; (j >= 0) && (m_vvcSliceParams[m_sliceIdxInOrder[j]].m_shSliceAddress > startTile); j--) {}

                if (j == -1)
                {
                    m_sliceIdxInOrder.insert(m_sliceIdxInOrder.begin(), i);
                }
                else if (m_vvcSliceParams[m_sliceIdxInOrder[j]].m_shSliceAddress == startTile) // duplicated slices detected, keep the one with bigger bitstream size
                {
                    if (m_vvcSliceParams[m_sliceIdxInOrder[j]].m_sliceBytesInBuffer - m_vvcSliceParams[m_sliceIdxInOrder[j]].m_byteOffsetToSliceData < temp.m_sliceBytesInBuffer - temp.m_byteOffsetToSliceData)
                    {
                        m_sliceIdxInOrder[j] = i;
                    }
                }
                else
                {
                    m_sliceIdxInOrder.insert(m_sliceIdxInOrder.begin() + j + 1, i);
                }
            }
        }
        else
        {
            for (uint16_t i = 0; i < m_numSlices; i++)
            {
                uint16_t subPicIdx = GetSubPicIdxFromSubPicId(m_vvcSliceParams[i].m_shSubpicId);
                uint16_t sliceIdx = 0;

                if(m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
                {
                    sliceIdx = m_subPicParams[subPicIdx].m_sliceIdx[m_vvcSliceParams[i].m_shSliceAddress];
                }
                else
                {
                    DECODE_CHK_COND(subPicIdx != 0, "Error detected: incorrect subpic index\n");
                    sliceIdx = m_vvcSliceParams[i].m_shSliceAddress;
                }

                if (!m_sliceDesc[sliceIdx].m_sliceAvailableFlag)
                {
                    m_sliceDesc[sliceIdx].m_sliceAvailableFlag = true;
                    m_sliceDesc[sliceIdx].m_sliceCtrlIdx = i;
                }
                else if(m_vvcSliceParams[m_sliceDesc[sliceIdx].m_sliceCtrlIdx].m_sliceBytesInBuffer - m_vvcSliceParams[m_sliceDesc[sliceIdx].m_sliceCtrlIdx].m_byteOffsetToSliceData < m_vvcSliceParams[i].m_sliceBytesInBuffer - m_vvcSliceParams[i].m_byteOffsetToSliceData)
                {
                    m_sliceDesc[sliceIdx].m_sliceCtrlIdx = i;
                }
            }

            uint16_t numSliceDesc = (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag) ?
                                    ((m_vvcPicParams->m_spsNumSubpicsMinus1 == 0 || !m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag) ? 1 : (m_vvcPicParams->m_spsNumSubpicsMinus1 + 1)) : (m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1);
            for (auto i = 0; i < numSliceDesc; i++)
            {
                if (m_sliceDesc[i].m_sliceAvailableFlag)
                {
                    m_sliceIdxInOrder.push_back(m_sliceDesc[i].m_sliceCtrlIdx);
                }
            }
        }

        //Override with valid slice number
        m_numSlices = m_sliceIdxInOrder.size();

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::CheckProfileCaps()
    {
        DECODE_FUNC_CALL();

        // Check chroma format
        if (m_vvcPicParams->m_spsChromaFormatIdc  != 1 || // 4:2:0
            !(m_vvcPicParams->m_spsBitdepthMinus8 == 0 || // 8 bit
            m_vvcPicParams->m_spsBitdepthMinus8   == 2))  // 10 bit
        {
            DECODE_ASSERTMESSAGE("Only 4:2:0 8bit and 10bit are supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::ErrorDetectAndConceal()
    {
        DECODE_FUNC_CALL()
        DECODE_CHK_NULL(m_vvcPicParams);

        DECODE_CHK_STATUS(CheckProfileCaps());

        // Error Detection
        if (m_vvcPicParams->m_spsLog2CtuSizeMinus5 > 2)
        {
            DECODE_ASSERTMESSAGE("pps_log2_ctu_size_minus5 must be less than or equal to 2.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        //slice number limit
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag && !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag)
        {
            if (m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1 > vvcMaxSliceNum)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        //Slice number check
        int16_t numSlices = 0;
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            numSlices = (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag)? (m_vvcPicParams->m_spsNumSubpicsMinus1 + 1):(m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1);
            if (m_numSlices != numSlices)
            {
                DECODE_ASSERTMESSAGE("Rect Slice: Slice number is incorrect.\n");
            }
        }

        //Concealment for bitstream size
        if (m_numSlices > 0)
        {
            CodecVvcSliceParams *lastSlice = m_vvcSliceParams + (m_numSlices - 1);
            DECODE_CHK_STATUS(SetRequiredBitstreamSize(lastSlice->m_bSNALunitDataLocation + lastSlice->m_sliceBytesInBuffer));
        }

        if ((m_vvcPicParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag == 1) && (m_vvcPicParams->m_spsLog2CtuSizeMinus5 == 0))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force sps_max_luma_transform_size_64_flag = 0 when sps_log2_ctu_size_minus5 = 0.\n");
            m_vvcPicParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag = 0;
        }

        if ((m_vvcPicParams->m_spsFlags0.m_fields.m_spsTransformSkipEnabledFlag == 0) &&
            (m_vvcPicParams->m_spsLog2TransformSkipMaxSizeMinus2 > 0 || m_vvcPicParams->m_spsFlags0.m_fields.m_spsBdpcmEnabledFlag > 0))
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_transform_skip_enabled_flag == 0) && (sps_log2_transform_skip_max_size_minus2 > 0 || sps_bdpcm_enabled_flag >0).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if ((m_vvcPicParams->m_spsFlags0.m_fields.m_spsMtsEnabledFlag == 0) &&
            (m_vvcPicParams->m_spsFlags0.m_fields.m_spsExplicitMtsIntraEnabledFlag == 1 || m_vvcPicParams->m_spsFlags0.m_fields.m_spsExplicitMtsInterEnabledFlag == 1))
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_mts_enabled_flag == 0) &&  (sps_explicit_mts_intra_enabled_flag == 1 || sps_explicit_mts_inter_enabled_flag == 1).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if ((m_vvcPicParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag == 0 || m_vvcPicParams->m_spsChromaFormatIdc == 0) &&
            (m_vvcPicParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag == 1))
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_alf_enabled_flag == 0 || sps_chroma_format_idc == 0) && (sps_ccalf_enabled_flag == 1).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRefWraparoundEnabledFlag) && m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag)
        {
            uint16_t picWidthMaxInCtus = MOS_ROUNDUP_SHIFT(m_vvcPicParams->m_spsPicWidthMaxInLumaSamples, m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 5);
            uint16_t picWidthInCtus = MOS_ROUNDUP_SHIFT(m_vvcPicParams->m_ppsPicWidthInLumaSamples, m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 5);
            if (m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
            {
                for (int16_t sp = 0; sp < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; sp++)
                {
                    if ((m_subPicParams[sp].m_spsSubpicWidthMinus1 + 1 != picWidthMaxInCtus) &&
                         m_subPicParams[sp].m_subPicFlags.m_fields.m_spsSubpicTreatedAsPicFlag)
                    {
                        DECODE_ASSERTMESSAGE("Error detected: (sps_ref_wraparound_enabled_flag == 1) && (at least one SubPic with sps_subpic_treated_as_pic_flag[i] == 1 && sps_subpic_width_minus1[i] + 1 != (sps_pic_width_max_in_luma_samples + CtbSizeY - 1 ) >> CtbLog2SizeY )).\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                }
            }
            else
            {
                if (picWidthInCtus != picWidthMaxInCtus)
                {
                    DECODE_ASSERTMESSAGE("Error detected: (sps_ref_wraparound_enabled_flag == 1) && (at least one SubPic with sps_subpic_treated_as_pic_flag[i] == 1 && sps_subpic_width_minus1[i] + 1 != (sps_pic_width_max_in_luma_samples + CtbSizeY - 1 ) >> CtbLog2SizeY )).\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsTemporalMvpEnabledFlag && m_vvcPicParams->m_spsFlags1.m_fields.m_spsSbtmvpEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_temporal_mvp_enabled_flag == 0 && sps_sbtmvp_enabled_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofEnabledFlag && m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofControlPresentInPhFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_bdof_enabled_flag == 0 && sps_bdof_control_present_in_ph_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrEnabledFlag && m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrControlPresentInPhFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_dmvr_enabled_flag == 0 && sps_dmvr_control_present_in_ph_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsMmvdEnabledFlag && m_vvcPicParams->m_spsFlags1.m_fields.m_spsMmvdFullpelOnlyEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_mmvd_enabled_flag == 0) && (sps_mmvd_fullpel_only_enabled_flag == 1).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineEnabledFlag &&
            (m_vvcPicParams->m_spsFiveMinusMaxNumSubblockMergeCand != 0 ||
             m_vvcPicParams->m_spsFlags1.m_fields.m_sps6paramAffineEnabledFlag != 0 ||
             m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineAmvrEnabledFlag == 1 ||
             m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag == 1 ||
             m_vvcPicParams->m_spsFlags1.m_fields.m_spsProfControlPresentInPhFlag == 1))
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_affine_enabled_flag == 0 && (sps_five_minus_max_num_subblock_merge_cand != 0 || sps_6param_affine_enabled_flag != 0 || sps_affine_amvr_enabled_flag == 1 || sps_affine_prof_enabled_flag == 1 || sps_prof_control_present_in_ph_flag  == 1).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsAmvrEnabledFlag && m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineAmvrEnabledFlag == 1)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_amvr_enabled_flag == 0 && sps_affine_amvr_enabled_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag && m_vvcPicParams->m_spsFlags1.m_fields.m_spsProfControlPresentInPhFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_affine_prof_enabled_flag == 0 && sps_prof_control_present_in_ph_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_vvcPicParams->m_spsSixMinusMaxNumMergeCand == 5 && m_vvcPicParams->m_spsFlags1.m_fields.m_spsGpmEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_six_minus_max_num_merge_cand == 5 && sps_gpm_enabled_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_vvcPicParams->m_spsChromaFormatIdc == 0 && m_vvcPicParams->m_spsFlags1.m_fields.m_spsCclmEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_chroma_format_idc ==  0 && sps_cclm_enabled_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_vvcPicParams->m_spsChromaFormatIdc != 1 &&
            (m_vvcPicParams->m_spsFlags1.m_fields.m_spsChromaHorizontalCollocatedFlag ||
             m_vvcPicParams->m_spsFlags1.m_fields.m_spsChromaVerticalCollocatedFlag))
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_chroma_format_idc != 1) && ((sps_chroma_horizontal_collocated_flag != 0) || (sps_chroma_vertical_collocated_flag!=0)).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if ((m_vvcPicParams->m_spsChromaFormatIdc != 3 || m_vvcPicParams->m_spsFlags0.m_fields.m_spsMaxLumaTransformSize64Flag) &&
            m_vvcPicParams->m_spsFlags2.m_fields.m_spsActEnabledFlag != 0)
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_chroma_format_idc  !=  3  ||  sps_max_luma_transform_size_64_flag ==1) && (sps_act_enabled_flag != 0).\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (!m_vvcPicParams->m_spsFlags0.m_fields.m_spsTransformSkipEnabledFlag &&
            !m_vvcPicParams->m_spsFlags2.m_fields.m_spsPaletteEnabledFlag &&
            m_vvcPicParams->m_spsMinQpPrimeTs == 1)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_transform_skip_enabled_flag ==0 &&  sps_palette_enabled_flag == 0 && sps_min_qp_prime_ts == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if ((!m_vvcPicParams->m_spsFlags2.m_fields.m_spsActEnabledFlag || !m_vvcPicParams->m_spsFlags2.m_fields.m_spsExplicitScalingListEnabledFlag) &&
            m_vvcPicParams->m_spsFlags2.m_fields.m_spsScalingMatrixForAlternativeColourSpaceDisabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: (sps_act_enabled_flag == 0 ||  sps_explicit_scaling_list_enabled_flag == 0 ) && sps_scaling_matrix_for_alternative_colour_space_disabled_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (!m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag && m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_virtual_boundaries_enabled_flag ==0 && sps_virtual_boundaries_present_flag==1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag &&
            m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag &&
            !m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force sps_virtual_boundaries_enabled_flag from 1 to 0.\n");
            m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag = 0;
        }

        if (!m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag && m_vvcPicParams->m_spsNumVerVirtualBoundaries > 0)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_virtual_boundaries_present_flag == 0 && sps_num_ver_virtual_boundaries > 0.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && (m_vvcPicParams->m_ppsPicWidthInLumaSamples != m_vvcPicParams->m_spsPicWidthMaxInLumaSamples ||
             m_vvcPicParams->m_ppsPicHeightInLumaSamples != m_vvcPicParams->m_spsPicHeightMaxInLumaSamples))
        {
            if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag)
            {
                DECODE_ASSERTMESSAGE("Error detected: (pps_pic_width_in_luma_samples != sps_pic_width_max_in_luma_samples || pps_pic_height_in_luma_samples != sps_pic_height_max_in_luma_samples) && (sps_virtual_boundaries_present_flag == 1 || sps_subpic_info_present_flag == 1).\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag)
            {
                DECODE_ASSERTMESSAGE("Error concealed: force sps_virtual_boundaries_present_flag from 1 to 0 when (pps_pic_width_in_luma_samples != sps_pic_width_max_in_luma_samples || pps_pic_height_in_luma_samples != sps_pic_height_max_in_luma_samples).\n");
                m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag = 0;
            }
        }

        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRefWraparoundEnabledFlag)
        {
            if ((1 << (m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 3 - m_vvcPicParams->m_spsLog2MinLumaCodingBlockSizeMinus2)) > ((m_vvcPicParams->m_ppsPicWidthInLumaSamples >> (m_vvcPicParams->m_spsLog2MinLumaCodingBlockSizeMinus2 + 2)) - 1))
            {
                DECODE_ASSERTMESSAGE("Error detected: pps_ref_wraparound_enabled_flag == 1 && ( CtbSizeY / MinCbSizeY + 1 ) > ( pps_pic_width_in_luma_samples / MinCbSizeY - 1 ).\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        
        if (!m_shortFormatInUse)
        {
            // Scaling Window
            int8_t subWidthC = (m_vvcPicParams->m_spsChromaFormatIdc == 0 || m_vvcPicParams->m_spsChromaFormatIdc == 3) ? 1 : 2;
            int8_t subHeightC = (m_vvcPicParams->m_spsChromaFormatIdc == 0 || m_vvcPicParams->m_spsChromaFormatIdc == 3) ? 1 : ((m_vvcPicParams->m_spsChromaFormatIdc == 1) ? 2 : 1);

            int32_t value1 = subWidthC * m_vvcPicParams->m_ppsScalingWinLeftOffset;
            int32_t value2 = subWidthC * m_vvcPicParams->m_ppsScalingWinRightOffset;
            int32_t min = -15 * m_vvcPicParams->m_ppsPicWidthInLumaSamples;
            int32_t max = m_vvcPicParams->m_ppsPicWidthInLumaSamples;
            bool scalWinOutOfHorRange = false;

            if ((value1 < min || value1 >= max) ||
                (value2 < min || value2 >= max) ||
                (value1 + value2 < min || value1 + value2 >= max))
            {
                scalWinOutOfHorRange = true;
            }

            value1 = subHeightC * m_vvcPicParams->m_ppsScalingWinTopOffset;
            value2 = subHeightC * m_vvcPicParams->m_ppsScalingWinBottomOffset;
            min = -15 * m_vvcPicParams->m_ppsPicHeightInLumaSamples;
            max = m_vvcPicParams->m_ppsPicHeightInLumaSamples;
            bool scalWinOutOfVerRange = false;

            if ((value1 < min || value1 >= max) ||
                (value2 < min || value2 >= max) ||
                (value1 + value2 < min || value1 + value2 >= max))
            {
                scalWinOutOfVerRange = true;
            }

            VvcRefFrameAttributes curFrameAttr;
            DECODE_CHK_STATUS(m_refFrames.GetRefAttrByFrameIndex(
                m_vvcPicParams->m_currPic.FrameIdx,
                    &curFrameAttr));

            for (uint32_t slc = 0; slc < m_numSlices; slc++)
            {
                CodecVvcSliceParams* curSliceParams = &m_vvcSliceParams[slc];
                if (curSliceParams->m_shSliceType != vvcSliceI)
                {
                    if (scalWinOutOfHorRange == true)
                    {
                        DECODE_ASSERTMESSAGE("Error detected: pps_scaling_win_left_offset or pps_scaling_win_right_offset out of range.\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    if (scalWinOutOfVerRange == true)
                    {
                        DECODE_ASSERTMESSAGE("Error detected: pps_scaling_win_top_offset or pps_scaling_win_bottom_offset out of range.\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }

                    for (auto i = 0; i < 2; i++)
                    {
                        for (auto j = 0; j < curSliceParams->m_numRefIdxActive[i]; j++)
                        {
                            uint8_t refPicIdx = curSliceParams->m_refPicList[i][j].FrameIdx;
                            if (refPicIdx < vvcMaxNumRefFrame)
                            {
                                VvcRefFrameAttributes       refFrameAttr;
                                uint8_t refFrameIdx;
                                if (m_vvcPicParams->m_refFrameList[refPicIdx].PicFlags != PICTURE_INVALID)
                                {
                                    refFrameIdx = m_vvcPicParams->m_refFrameList[refPicIdx].FrameIdx;
                                    DECODE_CHK_STATUS(m_refFrames.GetRefAttrByFrameIndex(
                                        refFrameIdx,
                                        &refFrameAttr));

                                    if (((curFrameAttr.m_currPicScalWinWidthL << 1) < refFrameAttr.m_currPicScalWinWidthL) ||
                                        ((curFrameAttr.m_currPicScalWinHeightL << 1) < refFrameAttr.m_currPicScalWinHeightL) ||
                                        (curFrameAttr.m_currPicScalWinWidthL > (refFrameAttr.m_currPicScalWinWidthL << 3)) ||
                                        (curFrameAttr.m_currPicScalWinHeightL > (refFrameAttr.m_currPicScalWinHeightL << 3)))
                                    {
                                        DECODE_ASSERTMESSAGE("Error detected: current frame's scaling window width/height out of range against ref frame.\n");
                                        return MOS_STATUS_INVALID_PARAMETER;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_subpic_info_present_flag == 1 && pps_rect_slice_flag == 0.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && m_vvcPicParams->m_spsChromaFormatIdc == 0 && m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag)
        {
            DECODE_ASSERTMESSAGE("Error detected: sps_chroma_format_idc == 0 &&  pps_chroma_tool_offsets_present_flag == 1.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && (m_vvcPicParams->m_spsChromaFormatIdc == 0 || !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag) &&
            (m_vvcPicParams->m_ppsCbQpOffset != 0 ||
                m_vvcPicParams->m_ppsCrQpOffset != 0 ||
                m_vvcPicParams->m_ppsJointCbcrQpOffsetValue != 0 ||
                m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSliceChromaQpOffsetsPresentFlag != 0 ||
                m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag != 0))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force chroma qp offsets/flags to 0 for (sps_chroma_format_idc == 0 || pps_chroma_tool_offsets_present_flag == 0).\n");
            m_vvcPicParams->m_ppsCbQpOffset = 0;
            m_vvcPicParams->m_ppsCrQpOffset = 0;
            m_vvcPicParams->m_ppsJointCbcrQpOffsetValue = 0;
            m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSliceChromaQpOffsetsPresentFlag = 0;
            m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag = 0;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag)
        {
            for (auto i = 0; i < 6; i++)
            {
                if (m_vvcPicParams->m_ppsJointCbcrQpOffsetList[i] != 0)
                {
                    DECODE_ASSERTMESSAGE("Error concealed: force pps_joint_cbcr_qp_offset_list[i] to 0 since pps_chroma_tool_offsets_present_flag == 0.\n");
                    m_vvcPicParams->m_ppsJointCbcrQpOffsetList[i] = 0;
                }
            }
        }

        bool partitionFlag = ((m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0) ||
            (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag && m_vvcPicParams->m_ppsNumSlicesInPicMinus1 > 0) ||
            m_tileRows > 0 || m_tileCols > 0 );
        if ((!partitionFlag || (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterOverrideEnabledFlag == 0)) &&
            (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag == 1))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force pps_dbf_info_in_ph_flag to 0 when (!pps_no_pic_partition_flag && pps_deblocking_filter_override_enabled_flag) is false.\n");
            m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag = 0;
        }

        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDeblockingFilterDisabledFlag &&
            (m_vvcPicParams->m_ppsLumaBetaOffsetDiv2 != 0 || m_vvcPicParams->m_ppsLumaTcOffsetDiv2 != 0))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force luma_beta_offset_div2 and luma_tc_offset_div2 to 0 when pps_deblocking_filter_disabled_flag is true.\n");
            m_vvcPicParams->m_ppsLumaBetaOffsetDiv2 = 0;
            m_vvcPicParams->m_ppsLumaTcOffsetDiv2 = 0;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag &&
            (m_vvcPicParams->m_phCbBetaOffsetDiv2 != m_vvcPicParams->m_phLumaBetaOffsetDiv2 ||
             m_vvcPicParams->m_phCbTcOffsetDiv2 != m_vvcPicParams->m_phLumaTcOffsetDiv2 ||
             m_vvcPicParams->m_phCrBetaOffsetDiv2 != m_vvcPicParams->m_phLumaBetaOffsetDiv2 ||
             m_vvcPicParams->m_phCrTcOffsetDiv2 != m_vvcPicParams->m_phLumaTcOffsetDiv2))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force chroma beta/tc offsets to the same syntax of luma when pps_chroma_tool_offsets_present_flag is 0.\n");
            m_vvcPicParams->m_phCbBetaOffsetDiv2    = m_vvcPicParams->m_phLumaBetaOffsetDiv2;
            m_vvcPicParams->m_phCbTcOffsetDiv2      = m_vvcPicParams->m_phLumaTcOffsetDiv2;
            m_vvcPicParams->m_phCrBetaOffsetDiv2    = m_vvcPicParams->m_phLumaBetaOffsetDiv2;
            m_vvcPicParams->m_phCrTcOffsetDiv2      = m_vvcPicParams->m_phLumaTcOffsetDiv2;
        }

        if (m_shortFormatInUse && ((!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedPredFlag && !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWeightedBipredFlag) ||
            !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag) && m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWpInfoInPhFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force pps_wp_info_in_ph_flag = 0 when ((pps_weighted_pred_flag == 0 &&  pps_weighted_bipred_flag ==0) || pps_rpl_info_in_ph_flag == 0 ).\n");
            m_vvcPicParams->m_ppsFlags.m_fields.m_ppsWpInfoInPhFlag = 0;
        }

        if ((!m_vvcPicParams->m_spsFlags0.m_fields.m_spsAlfEnabledFlag ||
            !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsAlfInfoInPhFlag) &&
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_alf_enabled_flag = 0 when (sps_alf_enabled_flag == 0 || pps_alf_info_in_ph_flag == 0) is true\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfEnabledFlag = 0;
        }

        if (!m_vvcPicParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag)
        {
            for (auto i = 0; i < vvcMaxAlfNum; i++)
            {
                if (m_activeAlfMask & (1 << i))
                {
                    if (m_alfApsArray[i].m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag ||
                        m_alfApsArray[i].m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag)
                    {
                        DECODE_ASSERTMESSAGE("Error concealed: force alf_cc_cb_filter_signal_flag = 0, alf_cc_cr_filter_signal_flag = 0 when sps_ccalf_enabled_flag is 0\n");
                        m_alfApsArray[i].m_alfFlags.m_fields.m_alfCcCbFilterSignalFlag = 0;
                        m_alfApsArray[i].m_alfFlags.m_fields.m_alfCcCrFilterSignalFlag = 0;
                    }
                }
            }
        }

        if (!m_vvcPicParams->m_phFlags.m_fields.m_phAlfEnabledFlag &&
            (m_vvcPicParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag ||
             m_vvcPicParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_alf_cb_enabled_flag = 0, ph_alf_cr_enabled_flag = 0 when ph_alf_enabled_flag = 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCbEnabledFlag = 0;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCrEnabledFlag = 0;
        }

        if (!m_vvcPicParams->m_spsFlags0.m_fields.m_spsCcalfEnabledFlag &&
            (m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag ||
             m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_alf_cc_cb_enabled_flag = 0, ph_alf_cc_cr_enabled_flag = 0 when sps_ccalf_enabled_flag = 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCbEnabledFlag = 0;
            m_vvcPicParams->m_phFlags.m_fields.m_phAlfCcCrEnabledFlag = 0;
        }

        if (!m_vvcPicParams->m_spsFlags0.m_fields.m_spsLmcsEnabledFlag && m_vvcPicParams->m_phFlags.m_fields.m_phLmcsEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_lmcs_enabled_flag = 0 when sps_lmcs_enabled_flag = 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phLmcsEnabledFlag = 0;
        }

        if (!m_vvcPicParams->m_phFlags.m_fields.m_phLmcsEnabledFlag && m_vvcPicParams->m_phFlags.m_fields.m_phChromaResidualScaleFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_chroma_residual_scale_flag = 0 when ph_lmcs_enabled_flag = 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phChromaResidualScaleFlag = 0;
        }

        if (!m_vvcPicParams->m_spsFlags2.m_fields.m_spsExplicitScalingListEnabledFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phExplicitScalingListEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_explicit_scaling_list_enabled_flag = 0 when sps_explicit_scaling_list_enabled_flag = 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phExplicitScalingListEnabledFlag = 0;
        }

        if ((!m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesEnabledFlag ||
             m_vvcPicParams->m_spsFlags2.m_fields.m_spsVirtualBoundariesPresentFlag) &&
            m_vvcPicParams->m_phFlags.m_fields.m_phVirtualBoundariesPresentFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_virtual_boundaries_present_flag = 0 when (sps_virtual_boundaries_enabled_flag == 0 || sps_virtual_boundaries_present_flag == 1) is true.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phVirtualBoundariesPresentFlag = 0;
        }

        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuQpDeltaEnabledFlag &&
            m_vvcPicParams->m_phCuQpDeltaSubdivIntraSlice != 0)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_cu_qp_delta_subdiv_intra_slice = 0 when pps_cu_qp_delta_enabled_flag is 0.\n");
            m_vvcPicParams->m_phCuQpDeltaSubdivIntraSlice = 0;
        }

        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag &&
            m_vvcPicParams->m_phCuChromaQpOffsetSubdivIntraSlice != 0)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_cu_chroma_qp_offset_subdiv_intra_slice = 0 when pps_cu_chroma_qp_offset_list_enabled_flag is 0.\n");
            m_vvcPicParams->m_phCuChromaQpOffsetSubdivIntraSlice = 0;
        }

        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuQpDeltaEnabledFlag &&
            m_vvcPicParams->m_phCuQpDeltaSubdivInterSlice != 0)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_cu_qp_delta_subdiv_inter_slice = 0 when pps_cu_qp_delta_enabled_flag is 0.\n");
            m_vvcPicParams->m_phCuQpDeltaSubdivInterSlice = 0;
        }

        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsCuChromaQpOffsetListEnabledFlag &&
            m_vvcPicParams->m_phCuChromaQpOffsetSubdivInterSlice != 0)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_cu_chroma_qp_offset_subdiv_inter_slice = 0 when pps_cu_chroma_qp_offset_list_enabled_flag is 0.\n");
            m_vvcPicParams->m_phCuChromaQpOffsetSubdivInterSlice = 0;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsTemporalMvpEnabledFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_temporal_mvp_enabled_flag = 0 when sps_temporal_mvp_enabled_flag is 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag = 0;
        }

        if (m_vvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag == 1)
        {
            VvcRefFrameAttributes curFrameAttr;
            DECODE_CHK_STATUS(m_refFrames.GetRefAttrByFrameIndex(
                m_vvcPicParams->m_currPic.FrameIdx,
                    &curFrameAttr));

            bool allDiffFlag = true;
            uint8_t frameIdx = 0;
            VvcRefFrameAttributes refFrameAttr;
            for (uint8_t i = 0; i < vvcMaxNumRefFrame; i++)
            {
                if (m_vvcPicParams->m_refFrameList[i].PicFlags != PICTURE_INVALID)
                {
                    frameIdx = m_vvcPicParams->m_refFrameList[i].FrameIdx;
                    DECODE_CHK_STATUS(m_refFrames.GetRefAttrByFrameIndex(
                        frameIdx,
                        &refFrameAttr));

                    allDiffFlag &= (curFrameAttr.m_refpicwidth               != refFrameAttr.m_refpicwidth) ||
                                   (curFrameAttr.m_refpicheight              != refFrameAttr.m_refpicheight) ||
                                   (curFrameAttr.m_refscalingwinbottomoffset != refFrameAttr.m_refscalingwinbottomoffset) ||
                                   (curFrameAttr.m_refscalingwintopoffset    != refFrameAttr.m_refscalingwintopoffset) ||
                                   (curFrameAttr.m_refscalingwinleftoffset   != refFrameAttr.m_refscalingwinleftoffset) ||
                                   (curFrameAttr.m_refscalingwinrightoffset  != refFrameAttr.m_refscalingwinrightoffset);

                    if (!allDiffFlag)
                    {
                        DECODE_NORMALMESSAGE("Found a ref pic in the DPB has the same spatial resolution and the same scaling window offsets as the current picture.\n");
                        break;
                    }
                }
            }

            if (allDiffFlag)
            {
                DECODE_ASSERTMESSAGE("Error concealed: force ph_temporal_mvp_enabled_flag = 0 if no reference picture in the DPB has the same spatial resolution and the same scaling window offsets as the current picture.\n");
                m_vvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag = 0;
            }
        }

        if (m_shortFormatInUse && m_vvcPicParams->m_phFlags.m_fields.m_phInterSliceAllowedFlag && m_vvcPicParams->m_phFlags.m_fields.m_phTemporalMvpEnabledFlag &&
            m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag &&
            !m_vvcPicParams->m_phFlags.m_fields.m_numRefEntries1RplIdx1LargerThan0 &&
            !m_vvcPicParams->m_phFlags.m_fields.m_phCollocatedFromL0Flag)
        {
            DECODE_ASSERTMESSAGE("Error detected: ph_collocated_from_l0_flag = 0 when (ph_temporal_mvp_enabled_flag == 1 && pps_rpl_info_in_ph_flag == 1 && num_ref_entries[ 1 ][ RplsIdx[ 1 ] ] == 0) is true.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsMmvdFullpelOnlyEnabledFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phMmvdFullpelOnlyFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_mmvd_fullpel_only_flag = 0 when sps_mmvd_fullpel_only_enabled_flag is 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phMmvdFullpelOnlyFlag = 0;
        }

        uint32_t bdofDisabledFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofControlPresentInPhFlag ? 1 : (1 - m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofEnabledFlag);
        uint32_t dmvrDisabledFlag = m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrControlPresentInPhFlag ? 1 : (1-m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrEnabledFlag);

        if (m_shortFormatInUse && m_vvcPicParams->m_phFlags.m_fields.m_phInterSliceAllowedFlag && m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag &&
            !m_vvcPicParams->m_phFlags.m_fields.m_numRefEntries1RplIdx1LargerThan0 &&
            (!m_vvcPicParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag ||
             m_vvcPicParams->m_phFlags.m_fields.m_phBdofDisabledFlag != bdofDisabledFlag ||
             m_vvcPicParams->m_phFlags.m_fields.m_phDmvrDisabledFlag != dmvrDisabledFlag))
        {
            if (!m_vvcPicParams->m_phFlags.m_fields.m_phMvdL1ZeroFlag)
            {
                DECODE_ASSERTMESSAGE("Error detected: ph_mvd_l1_zero_flag = 0 when pps_rpl_info_in_ph_flag = 1 && num_ref_entries[ 1 ][ RplsIdx[ 1 ] ] = 0;\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            else
            {
                DECODE_ASSERTMESSAGE("Error concealed: since pps_rpl_info_in_ph_flag = 1 && num_ref_entries[ 1 ][ RplsIdx[ 1 ] ] = 0, do concealment:\n\
                Force ph_bdof_disabled_flag = sps_bdof_control_present_in_ph_flag == 0? 1 - sps_bdof_enabled_flag : 1;\n\
                Force ph_dmvr_disabled_flag = sps_dmvr_control_present_in_ph_flag == 0 ? 1 - sps_dmvr_enabled_flag : 1;\n");

                m_vvcPicParams->m_phFlags.m_fields.m_phBdofDisabledFlag = bdofDisabledFlag;
                m_vvcPicParams->m_phFlags.m_fields.m_phDmvrDisabledFlag = dmvrDisabledFlag;
            }
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsBdofControlPresentInPhFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phBdofDisabledFlag != bdofDisabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: Force ph_bdof_disabled_flag = sps_bdof_control_present_in_ph_flag == 0? 1 - sps_bdof_enabled_flag : 1;\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phBdofDisabledFlag = bdofDisabledFlag;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsDmvrControlPresentInPhFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phDmvrDisabledFlag != dmvrDisabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_dmvr_disabled_flag = sps_dmvr_control_present_in_ph_flag == 0 ? 1 - sps_dmvr_enabled_flag : 1;\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phDmvrDisabledFlag = dmvrDisabledFlag;
        }

        if (m_shortFormatInUse && !m_vvcPicParams->m_spsFlags1.m_fields.m_spsProfControlPresentInPhFlag &&
            m_vvcPicParams->m_phFlags.m_fields.m_phProfDisabledFlag != 1 - m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_prof_disabled_flag = 1-sps_affine_prof_enabled_flag when sps_prof_control_present_in_ph_flag = 0.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phProfDisabledFlag = 1 - m_vvcPicParams->m_spsFlags1.m_fields.m_spsAffineProfEnabledFlag;
        }

        if ((!m_vvcPicParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag || !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag) &&
            m_vvcPicParams->m_phFlags.m_fields.m_phSaoLumaEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_sao_luma_enabled_flag = 0 when (sps_sao_enabled_flag == 0 ||  pps_sao_info_in_ph_flag == 0) is true.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phSaoLumaEnabledFlag = 0;
        }

        if ((!m_vvcPicParams->m_spsFlags0.m_fields.m_spsSaoEnabledFlag ||
            !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSaoInfoInPhFlag ||
            m_vvcPicParams->m_spsChromaFormatIdc == 0) &&
            m_vvcPicParams->m_phFlags.m_fields.m_phSaoChromaEnabledFlag)
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_sao_chroma_enabled_flag = 0 when (sps_sao_enabled_flag == 0 ||  pps_sao_info_in_ph_flag == 0 || sps_chroma_format_idc  ==  0) is true.\n");
            m_vvcPicParams->m_phFlags.m_fields.m_phSaoChromaEnabledFlag = 0;
        }

        // Deblocking filter - luma
        if(m_shortFormatInUse && (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag ||
            m_vvcPicParams->m_phFlags.m_fields.m_phDeblockingFilterDisabledFlag) &&
           (m_vvcPicParams->m_phLumaBetaOffsetDiv2 != m_vvcPicParams->m_ppsLumaBetaOffsetDiv2 ||
            m_vvcPicParams->m_phLumaTcOffsetDiv2 != m_vvcPicParams->m_ppsLumaTcOffsetDiv2))
        {
            DECODE_ASSERTMESSAGE("Error concealed: force ph_luma_beta_offset_div2=pps_luma_beta_offset_div2, ph_luma_tc_offset_div2 = pps_luma_tc_offset_div2 when (pps_dbf_info_in_ph_flag == 0 || ph_deblocking_filter_disabled_flag == 1) is true.\n");
            m_vvcPicParams->m_phLumaBetaOffsetDiv2 = m_vvcPicParams->m_ppsLumaBetaOffsetDiv2;
            m_vvcPicParams->m_phLumaTcOffsetDiv2 = m_vvcPicParams->m_ppsLumaTcOffsetDiv2;
        }

        if (m_shortFormatInUse)
        {
            // Deblocking filter - chroma
            int8_t cbBetaOffsetDiv2 = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag ? m_vvcPicParams->m_ppsCbBetaOffsetDiv2 : m_vvcPicParams->m_phLumaBetaOffsetDiv2;
            int8_t cbTcOffsetDiv2   = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag ? m_vvcPicParams->m_ppsCbTcOffsetDiv2 : m_vvcPicParams->m_phLumaTcOffsetDiv2;
            int8_t crBetaOffsetDiv2 = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag ? m_vvcPicParams->m_ppsCrBetaOffsetDiv2 : m_vvcPicParams->m_phLumaBetaOffsetDiv2;
            int8_t crTcOffsetDiv2   = m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag ? m_vvcPicParams->m_ppsCrTcOffsetDiv2 : m_vvcPicParams->m_phLumaTcOffsetDiv2;

            if ((!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsDbfInfoInPhFlag ||
                    m_vvcPicParams->m_phFlags.m_fields.m_phDeblockingFilterDisabledFlag ||
                    !m_vvcPicParams->m_ppsFlags.m_fields.m_ppsChroma_toolOffsetsPresentFlag) &&
                (m_vvcPicParams->m_phCbBetaOffsetDiv2 != cbBetaOffsetDiv2 ||
                    m_vvcPicParams->m_phCbTcOffsetDiv2 != cbTcOffsetDiv2 ||
                    m_vvcPicParams->m_phCrBetaOffsetDiv2 != crBetaOffsetDiv2 ||
                    m_vvcPicParams->m_phCrTcOffsetDiv2 != crTcOffsetDiv2))
            {
                DECODE_ASSERTMESSAGE(
                    "Error concealed: force the following since (pps_dbf_info_in_ph_flag == 0 || ph_deblocking_filter_disabled_flag == 1 || pps_chroma_tool_offsets_present_flag == 0) is true: \n\
                ph_cb_beta_offset_div2  = pps_chroma_tool_offsets_present_flag == 1? pps_cb_beta_offset_div2 : ph_luma_beta_offset_div2;\n\
                ph_cb_tc_offset_div2    = pps_chroma_tool_offsets_present_flag == 1? pps_cb_tc_offset_div2 : ph_luma_tc_offset_div2;\n\
                ph_cr_beta_offset_div2  = pps_chroma_tool_offsets_present_flag == 1 ? pps_cr_beta_offset_div2 : ph_luma_beta_offset_div2;\n\
                ph_cr_tc_offset_div2    = pps_chroma_tool_offsets_present_flag == 1 ? pps_cr_tc_offset_div2 : ph_luma_tc_offset_div2;\n");
                m_vvcPicParams->m_phCbBetaOffsetDiv2 = cbBetaOffsetDiv2;
                m_vvcPicParams->m_phCbTcOffsetDiv2   = cbTcOffsetDiv2;
                m_vvcPicParams->m_phCrBetaOffsetDiv2 = crBetaOffsetDiv2;
                m_vvcPicParams->m_phCrTcOffsetDiv2   = crTcOffsetDiv2;
            }
        }
        // detection + concealment for slice duplication + reorder for Long Format decoding
        if (!m_shortFormatInUse)
        {
            DECODE_CHK_STATUS(SliceErrorHandlingLF());
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
    {
        DECODE_FUNC_CALL();
        if (requiredSize > m_dataSize)
        {
            m_dataOffset = 0;
            m_dataSize   = MOS_ALIGN_CEIL(requiredSize, CODEC_BITSTREAM_ALIGN_SIZE_VVC);
        }
        DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
        return MOS_STATUS_SUCCESS;
    }

    int16_t VvcBasicFeature::GetSubPicIdxFromSubPicId(uint16_t subPicId)
    {
        DECODE_FUNC_CALL();

        if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag &&
            m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
        {
            DECODE_CHK_NULL(m_subPicParams);

            for (int16_t idx = 0; idx < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; idx++)
            {
                if(m_subPicParams[idx].m_subpicIdVal == subPicId)
                {
                    return idx;
                }
            }
        }

        return 0;
    }

    MOS_STATUS VvcBasicFeature::SetSubPicStruct()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vvcPicParams);
        DECODE_CHK_NULL(m_subPicParams);

        if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
        {
            DECODE_CHK_NULL(m_subPicParams);
            DECODE_CHK_COND(m_vvcPicParams->m_spsNumSubpicsMinus1 >= vvcMaxSubpicNum, "Error detected: subpic number out of range!\n");

            MOS_ZeroMemory(m_sliceIdxInPicScanOrder, sizeof(m_sliceIdxInPicScanOrder));
            uint32_t accNumCtu = 0;
            for (auto i = 0; i < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; i++)
            {
                CodecVvcSubpicParam* subPic = &m_subPicParams[i];
                subPic->m_endCtbX = subPic->m_spsSubpicCtuTopLeftX + subPic->m_spsSubpicWidthMinus1;
                subPic->m_endCtbY = subPic->m_spsSubpicCtuTopLeftY + subPic->m_spsSubpicHeightMinus1;
                subPic->m_numSlices = 0;
                subPic->m_sliceIdx = &m_sliceIdxInPicScanOrder[0];

                accNumCtu += (subPic->m_spsSubpicWidthMinus1 + 1) * (subPic->m_spsSubpicHeightMinus1 + 1);
                DECODE_CHK_COND(subPic->m_endCtbX > m_picWidthInCtu - 1, "Error detected: subpic hor boundary out of pic!\n");
                DECODE_CHK_COND(subPic->m_endCtbY > m_picHeightInCtu - 1, "Error detected: subpic vert boundary out of pic!\n");
            }
            DECODE_CHK_COND(accNumCtu != m_picWidthInCtu * m_picHeightInCtu, "Error detected: subpictures have gap/overlap!\n");
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::ReconstructTile()
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(m_tileRow, sizeof(m_tileRow));
        MOS_ZeroMemory(m_tileCol, sizeof(m_tileCol));

        uint16_t accWidth = 0, accHeight = 0;
        uint16_t col = 0, row = 0;
        uint16_t ctuSize = 1 << (m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 5);

        //column
        for (col = 0; col <= m_vvcPicParams->m_ppsNumExpTileColumnsMinus1; col++)
        {
            m_tileCol[col].m_startCtbX = accWidth;
            m_tileCol[col].m_widthInCtb = m_tileParams[col].m_tileDimension + 1;
            m_tileCol[col].m_endCtbX = m_tileCol[col].m_startCtbX + m_tileCol[col].m_widthInCtb - 1;
            accWidth += m_tileParams[col].m_tileDimension + 1;
            DECODE_CHK_COND(accWidth > m_picWidthInCtu, "Error Detected: Tile hor boundary out of pic.\n");
        }

        uint16_t uniformWidthInCtb = m_tileParams[m_vvcPicParams->m_ppsNumExpTileColumnsMinus1].m_tileDimension + 1;
        for (; accWidth + uniformWidthInCtb <= m_picWidthInCtu; col++)
        {
            m_tileCol[col].m_startCtbX = accWidth;
            m_tileCol[col].m_widthInCtb = uniformWidthInCtb;
            m_tileCol[col].m_endCtbX = m_tileCol[col].m_startCtbX + m_tileCol[col].m_widthInCtb - 1;
            accWidth += uniformWidthInCtb;
        }
        if (accWidth < m_picWidthInCtu)
        {
            m_tileCol[col].m_startCtbX = accWidth;
            m_tileCol[col].m_widthInCtb = m_picWidthInCtu - accWidth;
            m_tileCol[col].m_endCtbX = m_picWidthInCtu - 1;
            col++;
        }

        m_tileCols = col;
        DECODE_CHK_COND(m_tileCols > vvcMaxTileColNum || m_tileCols == 0, "Error detected: tile column number out of range!\n");

        // row
        for (row = 0; row <= m_vvcPicParams->m_ppsNumExpTileRowsMinus1; row++)
        {
            m_tileRow[row].m_startCtbY   = accHeight;
            m_tileRow[row].m_heightInCtb = m_tileParams[m_vvcPicParams->m_ppsNumExpTileColumnsMinus1 + 1 + row].m_tileDimension + 1;
            m_tileRow[row].m_endCtbY     = m_tileRow[row].m_startCtbY + m_tileRow[row].m_heightInCtb - 1;
            accHeight += m_tileParams[m_vvcPicParams->m_ppsNumExpTileColumnsMinus1 + 1 + row].m_tileDimension + 1;
            DECODE_CHK_COND(accHeight > m_picHeightInCtu, "Error Detected: Tile vert boundary out of pic.\n");
        }
        uint16_t lastIdx = m_vvcPicParams->m_ppsNumExpTileRowsMinus1 + m_vvcPicParams->m_ppsNumExpTileColumnsMinus1 + 1;
        uint16_t uniformHeightInCtb = m_tileParams[lastIdx].m_tileDimension + 1;
        for (; accHeight + uniformHeightInCtb <= m_picHeightInCtu; row++)
        {
            m_tileRow[row].m_startCtbY   = accHeight;
            m_tileRow[row].m_heightInCtb = uniformHeightInCtb;
            m_tileRow[row].m_endCtbY     = m_tileRow[row].m_startCtbY + m_tileRow[row].m_heightInCtb - 1;
            accHeight += uniformHeightInCtb;
        }
        if (accHeight < m_picHeightInCtu)
        {
            m_tileRow[row].m_startCtbY = accHeight;
            m_tileRow[row].m_heightInCtb = m_picHeightInCtu - accHeight;
            m_tileRow[row].m_endCtbY = m_picHeightInCtu - 1;
            row++;
        }
        m_tileRows = row;
        DECODE_CHK_COND((m_tileRows * m_tileCols > vvcMaxTileNum) || m_tileRows == 0, "Error detected: tile number out of range!\n");

        m_maxTileWidth = 0;
        for (col = 0; col <= m_vvcPicParams->m_ppsNumExpTileColumnsMinus1; col++)
        {
            if (m_tileParams[col].m_tileDimension + 1 > m_maxTileWidth)
            {
                m_maxTileWidth = m_tileParams[col].m_tileDimension + 1;
            }
        }

        if (((ctuSize == vvcCtu32) && (m_maxTileWidth > vvcMaxTileWCtb32)) ||
            ((ctuSize == vvcCtu64) && (m_maxTileWidth > vvcMaxTileWCtb64)) ||
            ((ctuSize == vvcCtu128) && (m_maxTileWidth > vvcMaxTileWCtb128)))
        {
            DECODE_ASSERTMESSAGE("Tile width exceeds maximum allowed value.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::ReconstructSlice()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vvcPicParams);
        DECODE_CHK_NULL(m_vvcSliceParams);

        MOS_ZeroMemory(m_sliceDesc, sizeof(m_sliceDesc));

        // Partition - reconstruct Slices
        if (!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRectSliceFlag)
        {
            //Category1: Raster Scan Slice
            uint16_t col, row;
            for (uint32_t i = 0; i < m_numSlices; i++)
            {
                uint16_t start = m_vvcSliceParams[i].m_shSliceAddress;
                uint16_t end = start + m_vvcSliceParams[i].m_shNumTilesInSliceMinus1;

                uint32_t numCtus = 0;
                for (auto idx = start; idx <= end; idx++)
                {
                    col = idx % m_tileCols;
                    row = idx / m_tileCols;

                    numCtus += m_tileCol[col].m_widthInCtb * m_tileRow[row].m_heightInCtb;
                }
                m_sliceDesc[i].m_numCtusInCurrSlice = numCtus;

                col = start % m_tileCols;
                row = start / m_tileCols;
                m_sliceDesc[i].m_startTileX     = col;
                m_sliceDesc[i].m_startTileY     = row;
                m_sliceDesc[i].m_sliceStartCtbx = m_tileCol[col].m_startCtbX;
                m_sliceDesc[i].m_sliceStartCtby = m_tileRow[row].m_startCtbY;
            }
        }
        else if(!m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag)
        {
            // Category2: Most general rect slice case,  subpic v.s. slice partition is not the same
            DECODE_CHK_NULL(m_sliceStructParams);

            uint16_t tileIdx = 0;
            int32_t slcStructIdx = -1;//index to slice structure
            CodecVvcSliceStructure* slcStruct = nullptr;
            DECODE_ASSERT(m_sliceStructParams[0].m_sliceTopLeftTileIdx == 0);

            // Go through rectangular slice parameters
            int32_t i = 0;
            for( i = 0; i < m_vvcPicParams->m_ppsNumSlicesInPicMinus1; i++ )
            {
                slcStructIdx++;
                if (slcStructIdx > m_vvcPicParams->m_numSliceStructsMinus1)
                {
                    DECODE_ASSERTMESSAGE("Incomplete slice structures received.\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                slcStruct = &m_sliceStructParams[slcStructIdx];
                DECODE_CHK_COND(slcStruct->m_sliceTopLeftTileIdx >= m_tileCols * m_tileRows,"Error Detected: error slice structure params, SliceTopLeftTileIdx exceeds the max tile index in the picture!\n");
                tileIdx = slcStruct->m_sliceTopLeftTileIdx;
                m_sliceDesc[i].m_tileIdx = tileIdx;

                // complete tiles within a single slice
                DECODE_CHK_COND((tileIdx % m_tileCols) + slcStruct->m_ppsSliceWidthInTilesMinus1 >= m_tileCols, "Error Detected: rect slice right border exceeds picture boundary.\n");
                DECODE_CHK_COND((tileIdx / m_tileCols) + slcStruct->m_ppsSliceHeightInTilesMinus1 >= m_tileRows, "Error Detected: rect slice bottom border exceeds picture boundary.\n");
                m_sliceDesc[i].m_sliceWidthInTiles = slcStruct->m_ppsSliceWidthInTilesMinus1 + 1;
                m_sliceDesc[i].m_sliceHeightInTiles = slcStruct->m_ppsSliceHeightInTilesMinus1 + 1;

                // Derived variables on slice v.s. CTU
                m_sliceDesc[i].m_sliceStartCtbx = m_tileCol[tileIdx % m_tileCols].m_startCtbX;
                m_sliceDesc[i].m_sliceStartCtby = m_tileRow[tileIdx / m_tileCols].m_startCtbY;

                // multiple slices within a single tile special case
                if( m_sliceDesc[i].m_sliceWidthInTiles == 1 && m_sliceDesc[i].m_sliceHeightInTiles == 1)
                {
                    if((m_tileRow[tileIdx/m_tileCols].m_heightInCtb == 1) ||
                        (slcStruct->m_ppsExpSliceHeightInCtusMinus1 + 1 == m_tileRow[tileIdx / m_tileCols].m_heightInCtb))
                    {
                        m_sliceDesc[i].m_numSlicesInTile = 1;
                        m_sliceDesc[i].m_sliceHeightInCtu = slcStruct->m_ppsExpSliceHeightInCtusMinus1 + 1;
                    }
                    else
                    {
                        uint16_t remTileRowHeight = m_tileRow[tileIdx / m_tileCols].m_heightInCtb;
                        int j = 0;
                        for (;;j++)
                        {
                            DECODE_CHK_COND(slcStruct->m_ppsExpSliceHeightInCtusMinus1 + 1 > remTileRowHeight || slcStruct->m_ppsExpSliceHeightInCtusMinus1 + 1 == 0, "Error Detected: Accumulated rect slice height of multiple slices in one tile case exceeds the tile height.\n");
                            m_sliceDesc[i+j].m_sliceHeightInCtu = slcStruct->m_ppsExpSliceHeightInCtusMinus1 + 1;
                            remTileRowHeight -= m_sliceDesc[i+j].m_sliceHeightInCtu;

                            slcStructIdx++;
                            if (slcStructIdx > m_vvcPicParams->m_numSliceStructsMinus1)
                            {
                                slcStructIdx--;
                                break;
                            }
                            slcStruct = &m_sliceStructParams[slcStructIdx];
                            if (slcStruct->m_sliceTopLeftTileIdx != tileIdx)
                            {
                                slcStructIdx--;
                                break;
                            }
                        }

                        uint16_t uniformSliceHeight = m_sliceDesc[i + j].m_sliceHeightInCtu;

                        uint16_t deriveNum = MOS_ROUNDUP_DIVIDE(remTileRowHeight, uniformSliceHeight);
                        DECODE_CHK_COND(i + j + deriveNum > m_vvcPicParams->m_ppsNumSlicesInPicMinus1, "Error detected: Derived slice index exceeds the the last slice index in pic!\n");

                        while( remTileRowHeight >= uniformSliceHeight )
                        {
                            j++;
                            m_sliceDesc[i + j].m_sliceHeightInCtu = uniformSliceHeight;
                            remTileRowHeight -= uniformSliceHeight;
                        }
                        if( remTileRowHeight > 0 )
                        {
                            j++;
                            m_sliceDesc[i + j].m_sliceHeightInCtu = remTileRowHeight;
                        }

                        for( int k = 0; k < j + 1; k++ )
                        {
                            m_sliceDesc[i + k].m_numSlicesInTile = j + 1;
                            m_sliceDesc[i + k].m_sliceWidthInTiles = 1;
                            m_sliceDesc[i + k].m_sliceHeightInTiles = 1;
                            m_sliceDesc[i + k].m_tileIdx = tileIdx;

                            //derived variables
                            if (k > 0)
                            {
                                m_sliceDesc[i + k].m_sliceStartCtby = m_sliceDesc[i + k - 1].m_sliceStartCtby + m_sliceDesc[i + k - 1].m_sliceHeightInCtu;
                            }
                            m_sliceDesc[i + k].m_sliceStartCtbx = m_tileCol[tileIdx % m_tileCols].m_startCtbX;
                        }
                        m_sliceDesc[i].m_topSliceInTileFlag = 1;
                        m_sliceDesc[i + j].m_bottomSliceInTileFlag = 1;
                        i += j;
                    }
                }
            }

            // The last slice not covered in multi-slice tile
            if (i == m_vvcPicParams->m_ppsNumSlicesInPicMinus1)
            {
                slcStructIdx++;
                if (slcStructIdx != m_vvcPicParams->m_numSliceStructsMinus1)
                {
                    DECODE_ASSERTMESSAGE("Incorrect slice structures received.\n");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                slcStruct = &m_sliceStructParams[slcStructIdx];
                m_sliceDesc[i].m_tileIdx = slcStruct->m_sliceTopLeftTileIdx;

                //Derive other params
                m_sliceDesc[i].m_numSlicesInTile    = 1;
                m_sliceDesc[i].m_sliceStartCtbx     = m_tileCol[m_sliceDesc[i].m_tileIdx % m_tileCols].m_startCtbX;
                m_sliceDesc[i].m_sliceStartCtby     = m_tileRow[m_sliceDesc[i].m_tileIdx / m_tileCols].m_startCtbY;
                m_sliceDesc[i].m_sliceHeightInCtu   = m_picHeightInCtu - m_sliceDesc[i].m_sliceStartCtby;
                m_sliceDesc[i].m_sliceHeightInTiles = m_tileRows - (m_sliceDesc[i].m_tileIdx / m_tileCols);
                m_sliceDesc[i].m_sliceWidthInTiles  = m_tileCols - (m_sliceDesc[i].m_tileIdx % m_tileCols);
            }

            //Derived variables on SubPic v.s. Slice
            if (m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag && m_vvcPicParams->m_spsNumSubpicsMinus1 > 0)
            {
                for (auto slc = 0; slc < m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1; slc++)
                {
                    for (uint16_t sp = 0; sp < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; sp++)
                    {
                        if ((m_sliceDesc[slc].m_sliceStartCtbx >= m_subPicParams[sp].m_spsSubpicCtuTopLeftX) &&
                            (m_sliceDesc[slc].m_sliceStartCtbx <= m_subPicParams[sp].m_endCtbX) &&
                            (m_sliceDesc[slc].m_sliceStartCtby >= m_subPicParams[sp].m_spsSubpicCtuTopLeftY) &&
                            (m_sliceDesc[slc].m_sliceStartCtby <= m_subPicParams[sp].m_endCtbY))
                        {
                            m_sliceDesc[slc].m_subPicIdx = sp;
                            m_sliceDesc[slc].m_sliceIdxInSubPic = m_subPicParams[sp].m_numSlices;

                            m_subPicParams[sp].m_numSlices++;

                            break;
                        }
                    }
                }
                int32_t accSlice = 0;
                for (auto sp = 0; sp < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; sp++)
                {
                    m_subPicParams[sp].m_sliceIdx = &m_sliceIdxInPicScanOrder[accSlice];
                    accSlice += m_subPicParams[sp].m_numSlices;
                }
                for (auto sp = 0; sp < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; sp++)
                {
                    m_subPicParams[sp].m_numSlices = 0;
                }
                for (uint16_t slc = 0; slc < m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1; slc++)
                {
                    int32_t sp = m_sliceDesc[slc].m_subPicIdx;

                    m_subPicParams[sp].m_numSlices++;
                    m_subPicParams[sp].m_sliceIdx[m_subPicParams[sp].m_numSlices - 1] = slc;
                }
            }

            for (uint16_t slc = 0; slc < m_vvcPicParams->m_ppsNumSlicesInPicMinus1 + 1; slc++)
            {
                //Derive other params
                m_sliceDesc[slc].m_multiSlicesInTileFlag = (m_sliceDesc[slc].m_numSlicesInTile > 1) ? 1 : 0;
                m_sliceDesc[slc].m_startTileX = m_sliceDesc[slc].m_tileIdx % m_tileCols;
                m_sliceDesc[slc].m_startTileY = m_sliceDesc[slc].m_tileIdx / m_tileCols;

                uint16_t startTile = 0;
                uint16_t endTile = 0;
                uint16_t sliceWidthInCtu = 0;
                uint16_t sliceHeightInCtu = 0;
                if (m_sliceDesc[slc].m_numSlicesInTile > 1)
                {
                    startTile = m_sliceDesc[slc].m_startTileX;
                    sliceWidthInCtu = m_tileCol[startTile].m_widthInCtb;
                    sliceHeightInCtu = m_sliceDesc[slc].m_sliceHeightInCtu;
                }
                else
                {
                    startTile = m_sliceDesc[slc].m_startTileX;
                    endTile = m_sliceDesc[slc].m_sliceWidthInTiles + startTile - 1;
                    sliceWidthInCtu = m_tileCol[endTile].m_endCtbX + 1 - m_tileCol[startTile].m_startCtbX;

                    startTile = m_sliceDesc[slc].m_startTileY;
                    endTile = m_sliceDesc[slc].m_sliceHeightInTiles + startTile - 1;
                    sliceHeightInCtu = m_tileRow[endTile].m_endCtbY + 1 - m_tileRow[startTile].m_startCtbY;
                    m_sliceDesc[slc].m_sliceHeightInCtu = sliceHeightInCtu;
                }
                m_sliceDesc[slc].m_numCtusInCurrSlice = sliceWidthInCtu * sliceHeightInCtu;
            }
        }
        else if(m_vvcPicParams->m_ppsFlags.m_fields.m_ppsSingleSlicePerSubpicFlag)
        {
            //Category3/4: slice & subpic have the same partition
            if (m_vvcPicParams->m_spsNumSubpicsMinus1 == 0 || !m_vvcPicParams->m_spsFlags0.m_fields.m_spsSubpicInfoPresentFlag)
            {
                //Category4: only one slice/subpic
                m_sliceDesc[0].m_numCtusInCurrSlice     = m_picWidthInCtu * m_picHeightInCtu;
                m_sliceDesc[0].m_tileIdx                = 0;
                m_sliceDesc[0].m_sliceWidthInTiles      = m_tileCols;
                m_sliceDesc[0].m_sliceHeightInTiles     = m_tileRows;
                m_sliceDesc[0].m_numSlicesInTile        = 1;
                m_sliceDesc[0].m_sliceHeightInCtu       = m_picHeightInCtu;
                m_sliceDesc[0].m_sliceStartCtbx         = 0;
                m_sliceDesc[0].m_sliceStartCtby         = 0;
                m_sliceDesc[0].m_subPicIdx              = 0;
                m_sliceDesc[0].m_sliceIdxInSubPic       = 0;
                m_sliceDesc[0].m_multiSlicesInTileFlag  = 0;
                m_sliceDesc[0].m_startTileX             = 0;
                m_sliceDesc[0].m_startTileY             = 0;
            }
            else
            {
                //Category3: multiple slice/subpic
                uint16_t startCtu = 0, endCtu = 0;
                int16_t startTileX = 0, startTileY = 0, endTile = 0;
                for (uint16_t i = 0; i < m_vvcPicParams->m_spsNumSubpicsMinus1 + 1; i++)
                {
                    startCtu   = m_subPicParams[i].m_spsSubpicCtuTopLeftX;
                    endCtu     = m_subPicParams[i].m_endCtbX;
                    m_sliceDesc[i].m_sliceWidthInTiles = GetSubpicWidthInTile(startCtu, endCtu, startTileX, endTile);

                    if ((m_sliceDesc[i].m_sliceWidthInTiles == -1) || (startTileX == -1) || (endTile == -1))
                    {
                        DECODE_ASSERTMESSAGE("Incorrect derived results.\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }

                    startCtu    = m_subPicParams[i].m_spsSubpicCtuTopLeftY;
                    endCtu      = m_subPicParams[i].m_endCtbY;
                    m_sliceDesc[i].m_sliceHeightInTiles = GetSubpicHeightInTile(startCtu, endCtu, startTileY, endTile);

                    if ((m_sliceDesc[i].m_sliceHeightInTiles == -1) || (startTileY == -1) || (endTile == -1))
                    {
                        DECODE_ASSERTMESSAGE("Incorrect derived results.\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }

                    if (m_sliceDesc[i].m_sliceHeightInTiles == 1 &&
                        m_subPicParams[i].m_spsSubpicHeightMinus1 + 1 < m_tileRow[startTileY].m_heightInCtb)
                    {
                        m_sliceDesc[i].m_multiSlicesInTileFlag = 1;
                        m_sliceDesc[i].m_topSliceInTileFlag    = (startCtu == m_tileRow[startTileY].m_startCtbY) ? true : false;
                        m_sliceDesc[i].m_bottomSliceInTileFlag = (endCtu == m_tileRow[endTile].m_endCtbY) ? true : false;
                    }
                    else
                    {
                        m_sliceDesc[i].m_multiSlicesInTileFlag = 0;
                        m_sliceDesc[i].m_topSliceInTileFlag    = 0;
                        m_sliceDesc[i].m_bottomSliceInTileFlag = 0;
                    }

                    m_sliceDesc[i].m_startTileX         = startTileX;
                    m_sliceDesc[i].m_startTileY         = startTileY;
                    m_sliceDesc[i].m_tileIdx            = startTileX + startTileY * m_tileCols;
                    m_sliceDesc[i].m_sliceHeightInCtu   = m_subPicParams[i].m_spsSubpicHeightMinus1 + 1;
                    m_sliceDesc[i].m_sliceStartCtbx     = m_subPicParams[i].m_spsSubpicCtuTopLeftX;
                    m_sliceDesc[i].m_sliceStartCtby     = m_subPicParams[i].m_spsSubpicCtuTopLeftY;
                    m_sliceDesc[i].m_subPicIdx          = i;
                    m_sliceDesc[i].m_sliceIdxInSubPic   = 0;

                    //Derive other params
                    m_sliceDesc[i].m_numCtusInCurrSlice = (m_subPicParams[i].m_spsSubpicHeightMinus1 + 1) * (m_subPicParams[i].m_spsSubpicWidthMinus1 + 1);
                    m_sliceDesc[i].m_numSlicesInTile    = 0;
                    m_subPicParams[i].m_sliceIdx        = &m_sliceIdxInPicScanOrder[i];
                    m_subPicParams[i].m_sliceIdx[0]     = i;
                    m_subPicParams[i].m_numSlices       = 1;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::ReconstructPartition(CodechalDecodeParams* decodeParams)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vvcPicParams);
        DECODE_CHK_NULL(m_vvcSliceParams);

        // Tile
        DECODE_CHK_STATUS(ReconstructTile());

        // SubPic
        DECODE_CHK_STATUS(SetSubPicStruct());

        // Slice
        DECODE_CHK_STATUS(ReconstructSlice());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::SetPictureStructs(CodechalDecodeParams *decodeParams)
    {
        DECODE_FUNC_CALL();

        m_curRenderPic                  = m_vvcPicParams->m_currPic;
        m_width                         = (uint32_t)m_vvcPicParams->m_ppsPicWidthInLumaSamples;
        m_height                        = (uint32_t)m_vvcPicParams->m_ppsPicHeightInLumaSamples;
        m_picWidthInCtu                 = MOS_ROUNDUP_SHIFT(m_vvcPicParams->m_ppsPicWidthInLumaSamples, m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 5);
        m_picHeightInCtu                = MOS_ROUNDUP_SHIFT(m_vvcPicParams->m_ppsPicHeightInLumaSamples, m_vvcPicParams->m_spsLog2CtuSizeMinus5 + 5);
        m_frameWidthAlignedMinBlk       = MOS_ALIGN_CEIL(m_vvcPicParams->m_ppsPicWidthInLumaSamples, vvcMinBlockWidth);
        m_frameHeightAlignedMinBlk      = MOS_ALIGN_CEIL(m_vvcPicParams->m_ppsPicHeightInLumaSamples, vvcMinBlockHeight);

        m_refFrameIndexList.clear();
        for (auto i = 0; i < vvcMaxNumRefFrame; i++)
        {
            uint8_t index = m_vvcPicParams->m_refFrameList[i].FrameIdx;
            if (index < CODEC_MAX_DPB_NUM_VVC)
            {
                m_refFrameIndexList.push_back(index);
            }
        }

        DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_vvcPicParams));
        DECODE_CHK_STATUS(m_mvBuffers.UpdatePicture(m_vvcPicParams->m_currPic.FrameIdx, m_refFrameIndexList));

        m_pictureCodingType = m_vvcPicParams->m_picMiscFlags.m_fields.m_intraPicFlag ? I_TYPE : MIXED_TYPE;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcBasicFeature::UpdateNumRefForList()
    {
        DECODE_FUNC_CALL();
        if (m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag > 0)
        {
            //List0:
            if (!m_vvcPicParams->m_phFlags.m_fields.m_rplSpsFlag0)
            {
                m_numRefForList0 = m_rplParams[vvcPhRpl0Offset].m_numRefEntries;
            }
            else
            {
                m_numRefForList0 = m_rplParams[m_vvcPicParams->m_rplSpsIndex0].m_numRefEntries;
            }
            //List1:
            if (!m_vvcPicParams->m_phFlags.m_fields.m_rplSpsFlag1)
            {
                m_numRefForList1 = m_rplParams[vvcPhRpl1Offset].m_numRefEntries;
            }
            else
            {
                m_numRefForList1 = m_rplParams[m_vvcPicParams->m_rplSpsIndex1 + vvcSpsCandidateRpl1Offset].m_numRefEntries;
            }
        }

        if (m_vvcPicParams->m_phFlags.m_fields.m_phInterSliceAllowedFlag && m_vvcPicParams->m_ppsFlags.m_fields.m_ppsRplInfoInPhFlag)
        {
            m_vvcPicParams->m_phFlags.m_fields.m_numRefEntries0RplIdx0LargerThan0 = m_numRefForList0 > 0 ? 1 : 0;
            m_vvcPicParams->m_phFlags.m_fields.m_numRefEntries1RplIdx1LargerThan0 = m_numRefForList1 > 0 ? 1 : 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    int16_t VvcBasicFeature::GetSubpicWidthInTile(uint16_t startCtu, uint16_t endCtu, int16_t &startTile, int16_t &endTile)
    {
        DECODE_FUNC_CALL();

        if ((endCtu < startCtu) ||
            (endCtu > m_picWidthInCtu) ||
            (startCtu > m_picWidthInCtu))
        {
            DECODE_ASSERT(endCtu >= startCtu);
        }

        startTile = -1;
        for (int16_t i = 0; i < m_tileCols; i++)
        {
            if (startCtu >= m_tileCol[i].m_startCtbX && startCtu <= m_tileCol[i].m_endCtbX)
            {
                startTile = i;
                break;
            }
        }

        endTile = -1;
        if ((startTile > -1))
        {
            for (int16_t i = startTile; i < m_tileCols; i++)
            {
                if (endCtu >= m_tileCol[i].m_startCtbX && endCtu <= m_tileCol[i].m_endCtbX)
                {
                    endTile = i;
                    break;
                }
            }
        }

        return ((endTile > -1) ? (endTile + 1 - startTile) : 1);
    }

    int16_t VvcBasicFeature::GetSubpicHeightInTile(uint16_t startCtu, uint16_t endCtu, int16_t &startTile, int16_t &endTile)
    {
        DECODE_FUNC_CALL();

        if ((endCtu < startCtu) ||
            (endCtu > m_picHeightInCtu) ||
            (startCtu > m_picHeightInCtu))
        {
            DECODE_ASSERT(endCtu >= startCtu);
        }

        startTile = -1;
        for (int16_t i = 0; i < m_tileRows; i++)
        {
            if (startCtu >= m_tileRow[i].m_startCtbY && startCtu <= m_tileRow[i].m_endCtbY)
            {
                startTile = i;
                break;
            }
        }

        endTile = -1;
        if ((startTile > -1))
        {
            for (int16_t i = startTile; i < m_tileRows; i++)
            {
                if (endCtu >= m_tileRow[i].m_startCtbY && endCtu <= m_tileRow[i].m_endCtbY)
                {
                    endTile = i;
                    break;
                }
            }
        }

        return ((endTile > -1) ? (endTile + 1 - startTile) : 1);
    }

}  // namespace decode
