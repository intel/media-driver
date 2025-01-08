/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_basic_feature_g12.cpp
//! \brief    Defines the common interface for decode av1 parameter g12
//! 

#include "decode_av1_basic_feature_g12.h"
#include "decode_utils.h"
#include "codechal_utilities.h"
#include "decode_allocator.h"

namespace decode
{
    Av1BasicFeatureG12::~Av1BasicFeatureG12()
    {
        for (uint8_t i = 0; i < av1DefaultCdfTableNum; i++)
        {
            if (!m_allocator->ResourceIsNull(&m_defaultCdfBuffers[i]->OsResource))
            {
                m_allocator->Destroy(m_defaultCdfBuffers[i]);
            }
        }
        if (m_usingDummyWl == true)
        {
            m_allocator->Destroy(m_destSurfaceForDummyWL);
        }
        if (m_fgInternalSurf != nullptr && !m_allocator->ResourceIsNull(&m_fgInternalSurf->OsResource))
        {
            m_allocator->Destroy(m_fgInternalSurf);
        }
    }

    MOS_STATUS Av1BasicFeatureG12::Init(void *setting)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(setting);

        DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));
        CodechalSetting *codecSettings = (CodechalSetting*)setting;

        if (m_osInterface != nullptr)
        {
            MEDIA_WA_TABLE* waTable = m_osInterface->pfnGetWaTable(m_osInterface);

            m_usingDummyWl = ((waTable != nullptr) ? MEDIA_IS_WA(waTable, Wa_1508208842) : false)
                && !m_osInterface->bSimIsActive;
            if (m_usingDummyWl == true)
            {
                //Allocate a dest surface for dummy WL
                m_destSurfaceForDummyWL = m_allocator->AllocateSurface(
                    16,
                    16,
                    "Dummy Decode Output Frame Buffer",
                    Format_NV12,
                    false,
                    resourceOutputPicture,
                    notLockableVideoMem);
                DECODE_CHK_NULL(m_destSurfaceForDummyWL);
            }
        }

        DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));
        DECODE_CHK_STATUS(m_tempBuffers.Init(m_hwInterface, *m_allocator, *this, CODEC_NUM_REF_AV1_TEMP_BUFFERS));
        DECODE_CHK_STATUS(m_tileCoding.Init(this, codecSettings));
        DECODE_CHK_STATUS(m_internalTarget.Init(*m_allocator));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::Update(void *params)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(params);

        DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

        CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;
        m_dataSize = decodeParams->m_dataSize;
        m_av1PicParams  = static_cast<CodecAv1PicParams*>(decodeParams->m_picParams);
        DECODE_CHK_NULL(m_av1PicParams);

        if (decodeParams->m_destSurface->OsResource.Format == Format_P010 &&
            m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            m_bitDepth = 10;
        }

        // Do error detection and concealment
        DECODE_CHK_STATUS(ErrorDetectAndConceal());

        if (m_av1PicParams->m_bitDepthIdx == 0) m_av1DepthIndicator = 0;
        if (m_av1PicParams->m_bitDepthIdx == 1) m_av1DepthIndicator = 1;
        if (m_av1PicParams->m_bitDepthIdx == 2) m_av1DepthIndicator = 2;

        m_pictureCodingType = m_av1PicParams->m_picInfoFlags.m_fields.m_frameType ? P_TYPE : I_TYPE;

        // Derive Large Scale Tile output frame width/height
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile &&
            !m_av1PicParams->m_anchorFrameInsertion &&
            m_av1PicParams->m_outputFrameWidthInTilesMinus1 == 0xffff &&
            m_av1PicParams->m_outputFrameHeightInTilesMinus1 == 0xffff)
        {
            m_av1PicParams->m_outputFrameWidthInTilesMinus1  = (uint16_t)((m_destSurface.dwWidth / (m_av1PicParams->m_widthInSbsMinus1[0] + 1)) 
                                                                >> (m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? 7 : 6)) - 1;
            m_av1PicParams->m_outputFrameHeightInTilesMinus1 = (uint16_t)((m_destSurface.dwHeight / (m_av1PicParams->m_heightInSbsMinus1[0] + 1))
                                                                >> (m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock ? 7 : 6)) - 1;
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile && m_av1PicParams->m_anchorFrameInsertion)
        {
            DECODE_CHK_STATUS(m_refFrames.InsertAnchorFrame(*m_av1PicParams));
            return MOS_STATUS_SUCCESS;
        }

        m_av1TileParams = static_cast<CodecAv1TileParams*>(decodeParams->m_sliceParams);
        DECODE_CHK_NULL(m_av1TileParams);

        m_segmentParams = &m_av1PicParams->m_av1SegData;
        DECODE_CHK_NULL(m_segmentParams);

        m_tileCoding.m_numTiles = decodeParams->m_numSlices;

        m_filmGrainEnabled = m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain;

        DECODE_CHK_STATUS(SetPictureStructs(decodeParams));
        DECODE_CHK_STATUS(SetTileStructs());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::ErrorDetectAndConceal()
    {
        DECODE_FUNC_CALL()
        DECODE_CHK_NULL(m_av1PicParams);

        // Frame Width/Frame Height, valid range is [15, 16383]
        if (m_av1PicParams->m_frameWidthMinus1 < 15 || m_av1PicParams->m_frameHeightMinus1 < 15)
        {
            DECODE_ASSERTMESSAGE(" Frame Width/Height is invalid, out of [15, 16383].");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Current FrameIdx
        if (m_av1PicParams->m_currPic.FrameIdx >= CODECHAL_MAX_DPB_NUM_LST_AV1)
        {
            DECODE_ASSERTMESSAGE("CurrPic.FrameIdx is invalid.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Mono Chrome
        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_monoChrome != 0)
        {
            DECODE_ASSERTMESSAGE("AV1 doesn't support monoChrome!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        
        // Reference Frame
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame && 
            m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != intraOnlyFrame)
        {
            for (int i = 0; i < 8; i++)
            {
                if (m_av1PicParams->m_refFrameMap[i].FrameIdx >= CODECHAL_MAX_DPB_NUM_LST_AV1)
                {
                    DECODE_ASSERTMESSAGE("ref_frame_map index is invalid!");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            for (int i = 0; i < 7; i++)
            {
                if (m_av1PicParams->m_refFrameIdx[i] > av1TotalRefsPerFrame - 1)
                {
                    DECODE_ASSERTMESSAGE("ref_frame_list index is invalid!");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
        }

        // Primary Ref Frame
        if (m_av1PicParams->m_primaryRefFrame > av1TotalRefsPerFrame - 1)
        {
            DECODE_ASSERTMESSAGE("primary ref index (should be in [0,7]) is invald!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Superres Scale Denominator
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres && (m_av1PicParams->m_superresScaleDenominator != av1ScaleNumerator))
        {
            if ((m_av1PicParams->m_superresScaleDenominator < 9) || (m_av1PicParams->m_superresScaleDenominator > 16))
            {
                DECODE_ASSERTMESSAGE("Invalid superres denominator (should be in [9, 16]) in pic parameter!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        // Deblocking Filter
        m_av1PicParams->m_interpFilter   = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_interpFilter,   0, 4);
        m_av1PicParams->m_filterLevel[0] = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_filterLevel[0], 0, 63);
        m_av1PicParams->m_filterLevel[1] = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_filterLevel[1], 0, 63);
        m_av1PicParams->m_filterLevelU   = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_filterLevelU,   0, 63);
        m_av1PicParams->m_filterLevelV   = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_filterLevelV,   0, 63);

        // Ref & Mode Deltas
        for (int i = 0; i < 8; i++)
        {
            m_av1PicParams->m_refDeltas[i]  = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_refDeltas[i],  -64, 63);
        }
        for (int j = 0; j < 2; j++)
        {
            m_av1PicParams->m_modeDeltas[j] = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_modeDeltas[j], -64, 63);
        }

        // QMatrix
        m_av1PicParams->m_yDcDeltaQ = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_yDcDeltaQ, -64, 63);
        m_av1PicParams->m_uDcDeltaQ = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_uDcDeltaQ, -64, 63);
        m_av1PicParams->m_uAcDeltaQ = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_uAcDeltaQ, -64, 63);
        m_av1PicParams->m_vDcDeltaQ = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_vDcDeltaQ, -64, 63);
        m_av1PicParams->m_vAcDeltaQ = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_vAcDeltaQ, -64, 63);

        // Segment Data
        if (!m_av1PicParams->m_av1SegData.m_enabled)
        {
            // Error concealment for segmentation
            m_av1PicParams->m_av1SegData.m_segmentInfoFlags = 0;
            memset(m_av1PicParams->m_av1SegData.m_featureMask, 0, 8);
            memset(&m_av1PicParams->m_av1SegData.m_featureData, 0, 8 * 8 * sizeof(int16_t));
        }

        // Tile Col & Row Number
        if (m_av1PicParams->m_tileCols > av1MaxTileRow || m_av1PicParams->m_tileRows > av1MaxTileColumn)
        {
            DECODE_ASSERTMESSAGE("tile row_num or col_num is invald!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // CDF Bits & Strength
        if (m_av1PicParams->m_cdefBits > 3)
        {
            DECODE_ASSERTMESSAGE("Invalid cdef_bits (should be in [0, 3]) in pic parameter!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        for (auto i = 0; i < (1 << m_av1PicParams->m_cdefBits); i++)
        {
            m_av1PicParams->m_cdefYStrengths[i]  = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_cdefYStrengths[i],  0, 63);
            m_av1PicParams->m_cdefUvStrengths[i] = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_cdefUvStrengths[i], 0, 63);
        }

        // LR Unit Shift
        m_av1PicParams->m_loopRestorationFlags.m_fields.m_lrUnitShift = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_loopRestorationFlags.m_fields.m_lrUnitShift, 0, 2);

        // Profile and Subsampling
        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_monoChrome ||
            m_av1PicParams->m_profile != 0 ||
            !(m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX ==1 &&
                m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 1))
        {
            DECODE_ASSERTMESSAGE("Only 4:2:0 8bit and 10bit are supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if(m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc &&
            (!(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame ||
                m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame) ||
                !m_av1PicParams->m_picInfoFlags.m_fields.m_allowScreenContentTools ||
                m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres))
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Tx Mode
        if(m_av1PicParams->m_losslessMode &&
            m_av1PicParams->m_modeControlFlags.m_fields.m_txMode != (uint32_t)CodecAv1TxType::ONLY_4X4)
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec! Coded Lossless only allows TX_4X4.\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_forceIntegerMv &&
            m_av1PicParams->m_picInfoFlags.m_fields.m_allowHighPrecisionMv &&
            !(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame ||
             m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame))
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_av1PicParams->m_picInfoFlags.m_fields.m_forceIntegerMv &&
            (!m_av1PicParams->m_picInfoFlags.m_fields.m_allowScreenContentTools &&
             !(m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame
                 || m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame)))
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Order Hint
        if (!m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint &&
            (m_av1PicParams->m_seqInfoFlags.m_fields.m_enableJntComp ||
                m_av1PicParams->m_picInfoFlags.m_fields.m_useRefFrameMvs))
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (!m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint &&
            (m_av1PicParams->m_orderHint != 0))
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // CDF Upate
        if (!m_av1PicParams->m_picInfoFlags.m_fields.m_disableFrameEndUpdateCdf &&
            m_av1PicParams->m_picInfoFlags.m_fields.m_disableCdfUpdate)
        {
            DECODE_ASSERTMESSAGE("Illegal Cdf update params combination!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Reference Mode
        if ((m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame ||
            m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame) &&
                (m_av1PicParams->m_modeControlFlags.m_fields.m_referenceMode != singleReference))
        {
            DECODE_ASSERTMESSAGE("Reference mode shouldn't be singleReference for keyFrame or intraOnlyFrame.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Skip Mode
        if (((m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame ||
                m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame ||
                m_av1PicParams->m_modeControlFlags.m_fields.m_referenceMode == singleReference) ||
                !m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint) &&
                m_av1PicParams->m_modeControlFlags.m_fields.m_skipModePresent)
        {
            DECODE_ASSERTMESSAGE("SkipModePresent should be 0 for keyFrame, intraOnlyFrame or singleReference.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if ((m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == keyFrame ||
            m_av1PicParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame) &&
            (m_av1PicParams->m_picInfoFlags.m_fields.m_allowWarpedMotion ||
                (m_av1PicParams->m_primaryRefFrame != av1PrimaryRefNone)))
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_enableOrderHint &&
            m_av1PicParams->m_orderHintBitsMinus1 > 7)
        {
            DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Film grain parameter check
        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_filmGrainParamsPresent &&
            m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
        {
            // Check film grain parameter of the luma component
            if (m_av1PicParams->m_filmGrainParams.m_numYPoints > 14)
            {
                DECODE_ASSERTMESSAGE("Invalid film grain num_y_points (should be in [0, 14]) in pic parameter!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            for (auto i = 1; i < m_av1PicParams->m_filmGrainParams.m_numYPoints; i++)
            {
                if (m_av1PicParams->m_filmGrainParams.m_pointYValue[i] <= m_av1PicParams->m_filmGrainParams.m_pointYValue[i - 1])
                {
                    DECODE_ASSERTMESSAGE("Invalid film grain point_y_value (point_y_value[%d] should be greater than point_y_value[%d]) in pic parameter!", i, i - 1);
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            // Check film grain parameter of the cb component
            if (m_av1PicParams->m_filmGrainParams.m_numCbPoints > 10)
            {
                DECODE_ASSERTMESSAGE("Invalid film grain num_cb_points (should be in [0, 10]) in pic parameter!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            for (auto i = 1; i < m_av1PicParams->m_filmGrainParams.m_numCbPoints; i++)
            {
                if (m_av1PicParams->m_filmGrainParams.m_pointCbValue[i] <= m_av1PicParams->m_filmGrainParams.m_pointCbValue[i - 1])
                {
                    DECODE_ASSERTMESSAGE("Invalid film grain point_cb_value (point_cb_value[%d] should be greater than point_cb_value[%d]) in pic parameter!", i, i - 1);
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            // Check film grain parameter of the cr component
            if (m_av1PicParams->m_filmGrainParams.m_numCrPoints > 10)
            {
                DECODE_ASSERTMESSAGE("Invalid film grain num_cr_points (should be in [0, 10]) in pic parameter!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
            for (auto i = 1; i < m_av1PicParams->m_filmGrainParams.m_numCrPoints; i++)
            {
                if (m_av1PicParams->m_filmGrainParams.m_pointCrValue[i] <= m_av1PicParams->m_filmGrainParams.m_pointCrValue[i - 1])
                {
                    DECODE_ASSERTMESSAGE("Invalid film grain point_cr_value (point_cr_value[%d] should be greater than point_cr_value[%d]) in pic parameter!", i, i - 1);
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }

            m_av1PicParams->m_filmGrainParams.m_cbOffset = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_filmGrainParams.m_cbOffset, 0, 512);
            m_av1PicParams->m_filmGrainParams.m_crOffset = MOS_CLAMP_MIN_MAX(m_av1PicParams->m_filmGrainParams.m_crOffset, 0, 512);
        }

        // Error Concealment for CDEF
        if (m_av1PicParams->m_losslessMode ||
            m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc ||
            !m_av1PicParams->m_seqInfoFlags.m_fields.m_enableCdef)
        {
            m_av1PicParams->m_cdefBits            = 0;
            m_av1PicParams->m_cdefYStrengths[0]   = 0;
            m_av1PicParams->m_cdefUvStrengths[0]  = 0;
            m_av1PicParams->m_cdefDampingMinus3   = 0;
        }

        // Error Concealment for Loop Filter
        if (m_av1PicParams->m_losslessMode ||
            m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            m_av1PicParams->m_filterLevel[0] = 0;
            m_av1PicParams->m_filterLevel[1] = 0;

            m_av1PicParams->m_refDeltas[intraFrame]     = 1;
            m_av1PicParams->m_refDeltas[lastFrame]      = 0;
            m_av1PicParams->m_refDeltas[last2Frame]     = 0;
            m_av1PicParams->m_refDeltas[last3Frame]     = 0;
            m_av1PicParams->m_refDeltas[bwdRefFrame]    = 0;
            m_av1PicParams->m_refDeltas[goldenFrame]    = -1;
            m_av1PicParams->m_refDeltas[altRef2Frame]   = -1;
            m_av1PicParams->m_refDeltas[altRefFrame]    = -1;

            m_av1PicParams->m_modeDeltas[0] = 0;
            m_av1PicParams->m_modeDeltas[1] = 0;

            m_av1PicParams->m_loopFilterInfoFlags.m_value = 0;
        }

        // Error Concealment for Loop Restoration
        if ((!m_av1PicParams->m_picInfoFlags.m_fields.m_useSuperres &&
            m_av1PicParams->m_losslessMode) ||
            m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
        {
            m_av1PicParams->m_loopRestorationFlags.m_value = 0;
        }

        // LRU Luma Size cannot < Superblock Size. If Superblock Size is 128x128, LRU Luma Size cannot be 64x64.
        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_use128x128Superblock)
        {
            if (m_av1PicParams->m_loopRestorationFlags.m_fields.m_lrUnitShift == 0)
            {
                DECODE_ASSERTMESSAGE("Conflict with AV1 Spec!");
                m_av1PicParams->m_loopRestorationFlags.m_value = 0;
            }
        }

        // Error Concealment for DeltaLF and DeltaQ
        if (m_av1PicParams->m_baseQindex == 0)
        {
            m_av1PicParams->m_modeControlFlags.m_fields.m_deltaQPresentFlag = 0;
        }
        if (m_av1PicParams->m_modeControlFlags.m_fields.m_deltaQPresentFlag)
        {
            if (m_av1PicParams->m_picInfoFlags.m_fields.m_allowIntrabc)
            {
                m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfPresentFlag = 0;
            }
        }
        else
        {
            m_av1PicParams->m_modeControlFlags.m_fields.m_log2DeltaQRes = 0;
            m_av1PicParams->m_modeControlFlags.m_fields.m_log2DeltaLfRes = 0;
            m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfPresentFlag = 0;
            m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfMulti = 0;
        }
        if (m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfPresentFlag == 0)
        {
            m_av1PicParams->m_modeControlFlags.m_fields.m_log2DeltaLfRes = 0;
            m_av1PicParams->m_modeControlFlags.m_fields.m_deltaLfMulti = 0;
        }

        // Error Concealment for Film Grain
        if (!m_av1PicParams->m_seqInfoFlags.m_fields.m_filmGrainParamsPresent ||
            !(m_av1PicParams->m_picInfoFlags.m_fields.m_showFrame ||
              m_av1PicParams->m_picInfoFlags.m_fields.m_showableFrame))
        {
            memset(&m_av1PicParams->m_filmGrainParams, 0, sizeof(CodecAv1FilmGrainParams));
        }

        // Error Concealment for Reference List
        if (m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != keyFrame && m_av1PicParams->m_picInfoFlags.m_fields.m_frameType != intraOnlyFrame)
        {
            DECODE_CHK_STATUS(m_refFrames.ErrorConcealment(*m_av1PicParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    //Currently, m_bsBytesInBuffer of current bitstream buffer is not passed by application.
    MOS_STATUS Av1BasicFeatureG12::SetRequiredBitstreamSize(uint32_t requiredSize)
    {
        DECODE_FUNC_CALL();
        m_dataSize = requiredSize;
        DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::GetDecodeTargetFormat(MOS_FORMAT& format)
    {
        if (m_av1PicParams->m_profile == 0)
        {
            if (m_av1PicParams->m_bitDepthIdx == 0)
            {
                format = Format_NV12;
            }
            else if (m_av1PicParams->m_bitDepthIdx == 1)
            {
                format = Format_P010;
            }
            else
            {
                DECODE_ASSERTMESSAGE("Unsupported sub-sampling format!");
                return MOS_STATUS_UNKNOWN;
            }
        }
        else
        {
            DECODE_ASSERTMESSAGE("The profile has not been supported yet!");
            return MOS_STATUS_UNKNOWN;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::SetPictureStructs(CodechalDecodeParams *decodeParams)
    {
        DECODE_FUNC_CALL();
        m_curRenderPic                  = m_av1PicParams->m_currPic;
        m_width                         = MOS_MAX(m_width, (uint32_t)m_av1PicParams->m_frameWidthMinus1 + 1);
        m_height                        = MOS_MAX(m_height, (uint32_t)m_av1PicParams->m_frameHeightMinus1 + 1);
        m_frameWidthAlignedMinBlk       = MOS_ALIGN_CEIL(m_av1PicParams->m_frameWidthMinus1 + 1, av1MinBlockWidth);
        m_frameHeightAlignedMinBlk      = MOS_ALIGN_CEIL(m_av1PicParams->m_frameHeightMinus1 + 1, av1MinBlockHeight);

        m_refFrameIndexList.clear();
        for (auto i = 0; i < av1TotalRefsPerFrame; i++)
        {
            uint8_t index = m_av1PicParams->m_refFrameMap[i].FrameIdx;
            if (index < CODECHAL_MAX_DPB_NUM_AV1)
            {
                m_refFrameIndexList.push_back(index);
            }
        }

        DECODE_CHK_STATUS(m_internalTarget.UpdateRefList(m_av1PicParams->m_currPic.FrameIdx, m_refFrameIndexList));

        if (m_filmGrainEnabled)
        {
            m_filmGrainProcParams = (FilmGrainProcParams*)&decodeParams->m_filmGrainProcParams;

            MOS_SURFACE surface = {};
            MOS_ZeroMemory(&surface, sizeof(surface));
            // if SFC enabled
#ifdef _DECODE_PROCESSING_SUPPORTED
            if (decodeParams->m_procParams != nullptr)
            {
                surface.dwWidth  = m_width;
                surface.dwHeight = m_height;
                DECODE_CHK_STATUS(GetDecodeTargetFormat(surface.Format));

                auto procParams = (DecodeProcessingParams *)decodeParams->m_procParams;
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(procParams->m_outputSurface));
                surface.TileModeGMM = procParams->m_outputSurface->TileModeGMM;
            }
            else
            {
#endif
                surface = m_destSurface;
#ifdef _DECODE_PROCESSING_SUPPORTED
            }
#endif
            if (m_filmGrainProcParams->m_inputSurface == nullptr)
            {
                DECODE_CHK_STATUS(m_internalTarget.ActiveCurSurf(
                    m_av1PicParams->m_currPic.FrameIdx,
                    &surface,
                    IsMmcEnabled(), resourceOutputPicture, notLockableVideoMem));

                m_filmGrainProcParams->m_inputSurface = m_internalTarget.GetCurSurf();
            }
            else
            {
                DECODE_CHK_NULL(m_filmGrainProcParams->m_inputSurface);
                DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_filmGrainProcParams->m_inputSurface));
            }
            m_filmGrainProcParams->m_inputSurface->UPlaneOffset.iYOffset
                = (m_filmGrainProcParams->m_inputSurface->UPlaneOffset.iSurfaceOffset - m_filmGrainProcParams->m_inputSurface->dwOffset) / m_filmGrainProcParams->m_inputSurface->dwPitch
                  + m_filmGrainProcParams->m_inputSurface->RenderOffset.YUV.U.YOffset;

            // For AVP+FilmGrain+SFC scenario, SFC will be the final unit,
            // set temp surface for film grain output
#ifdef _DECODE_PROCESSING_SUPPORTED
            if (decodeParams->m_procParams != nullptr)
            {
                if (m_fgInternalSurf == nullptr || m_allocator->ResourceIsNull(&m_fgInternalSurf->OsResource))
                {
                    m_fgInternalSurf = m_allocator->AllocateSurface(
                        m_width, m_height, "Internal film grain target surface", surface.Format, IsMmcEnabled(),
                        resourceOutputPicture, notLockableVideoMem, surface.TileModeGMM);
                }
                else
                {
                    DECODE_CHK_STATUS(m_allocator->Resize(m_fgInternalSurf, m_width, MOS_ALIGN_CEIL(m_height, 8),
                        notLockableVideoMem, false, "Internal film grain target surface"));
                }
                DECODE_CHK_NULL(m_fgInternalSurf);

                m_filmGrainProcParams->m_outputSurface = m_fgInternalSurf;
            }
#endif
            m_destSurface = *m_filmGrainProcParams->m_inputSurface;

            DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_filmGrainProcParams->m_outputSurface));
            m_fgOutputSurf = *m_filmGrainProcParams->m_outputSurface;
        }

        DECODE_CHK_STATUS(UpdateDefaultCdfTable());
        DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_av1PicParams));
        if (m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            for (auto &refFrameIdx : m_refFrameIndexList)
            {
                DECODE_CHK_STATUS(m_tempBuffers.ActiveCurBuffer(refFrameIdx));
            }
            DECODE_CHK_STATUS(m_tempBuffers.ReActiveCurBuffer(m_av1PicParams->m_currPic.FrameIdx, m_refFrameIndexList));
        }
        else
        {
            DECODE_CHK_STATUS(m_tempBuffers.UpdatePicture(m_av1PicParams->m_currPic.FrameIdx, m_refFrameIndexList));
        }
        DECODE_CHK_STATUS(SetSegmentData(*m_av1PicParams));

        DECODE_CHK_STATUS(CalculateGlobalMotionParams());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::SetTileStructs()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(m_tileCoding.Update(*m_av1PicParams, m_av1TileParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::SetSegmentData(CodecAv1PicParams &picParams)
    {
        DECODE_FUNC_CALL();

        // Configure Segment ID read buffer SegmentMapIsZeroFlag
        picParams.m_av1SegData.m_segmentMapIsZeroFlag    = false;
        picParams.m_av1SegData.m_segIdBufStreamInEnable  = false;
        picParams.m_av1SegData.m_segIdBufStreamOutEnable = false;
        uint8_t prevFrameIdx = m_refFrames.GetPrimaryRefIdx();

        if (picParams.m_av1SegData.m_enabled && 
            (picParams.m_av1SegData.m_temporalUpdate || !picParams.m_av1SegData.m_updateMap))
        {
            if(picParams.m_av1SegData.m_temporalUpdate)
            {
                DECODE_ASSERT((picParams.m_primaryRefFrame != av1PrimaryRefNone) &&
                                        picParams.m_av1SegData.m_updateMap);
            }

            if (!picParams.m_av1SegData.m_updateMap)
            {
                DECODE_ASSERT((picParams.m_primaryRefFrame != av1PrimaryRefNone) &&
                                       !picParams.m_av1SegData.m_temporalUpdate);
            }

            if (m_refFrames.CheckSegForPrimFrame(*m_av1PicParams))
            {
                picParams.m_av1SegData.m_segmentMapIsZeroFlag = false;
                picParams.m_av1SegData.m_segIdBufStreamInEnable = true;
            }
            else
            {
                picParams.m_av1SegData.m_segmentMapIsZeroFlag = true;
                picParams.m_av1SegData.m_segIdBufStreamInEnable = false;
            }
        }

        if(picParams.m_av1SegData.m_enabled && picParams.m_av1SegData.m_updateMap)
        {
            picParams.m_av1SegData.m_segIdBufStreamOutEnable = true;
        }

        // Calculate m_lastActiveSegmentId/m_preSkipSegmentIdFlag
        if (!picParams.m_av1SegData.m_enabled)
        {
            picParams.m_av1SegData.m_lastActiveSegmentId = 0;
            picParams.m_av1SegData.m_preSkipSegmentIdFlag = 0;
        }
        else if (picParams.m_av1SegData.m_updateData)
        {
            picParams.m_av1SegData.m_lastActiveSegmentId = 0;
            picParams.m_av1SegData.m_preSkipSegmentIdFlag = 0;

            for (uint8_t seg = 0; seg < av1MaxSegments; seg++)
            {
                for (int lvl = 0; lvl < segLvlMax; lvl++)
                {
                    if (picParams.m_av1SegData.m_featureMask[seg] & (1 << lvl))
                    {
                        picParams.m_av1SegData.m_preSkipSegmentIdFlag |= (lvl >= segLvlRefFrame);
                        picParams.m_av1SegData.m_lastActiveSegmentId = seg;
                    }
                }
            }
        }
        else if (picParams.m_primaryRefFrame != av1PrimaryRefNone)//copy from primary_ref_frame
        {
            picParams.m_av1SegData.m_lastActiveSegmentId = m_refFrames.m_refList[prevFrameIdx]->m_lastActiveSegmentId;
            picParams.m_av1SegData.m_preSkipSegmentIdFlag = m_refFrames.m_refList[prevFrameIdx]->m_preSkipSegmentIdFlag;
        }

        //record m_lastActiveSegmentId/m_preSkipSegmentIdFlag into DPB for future frame decoding
        m_refFrames.m_currRefList->m_lastActiveSegmentId = picParams.m_av1SegData.m_lastActiveSegmentId;
        m_refFrames.m_currRefList->m_preSkipSegmentIdFlag = picParams.m_av1SegData.m_preSkipSegmentIdFlag;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::CalculateGlobalMotionParams()
    {
        DECODE_FUNC_CALL();

        for (uint32_t ref = (uint32_t)lastFrame; ref <= (uint32_t)altRefFrame; ref++)
        {
            if (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype >= rotzoom)
            {
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[2] -= (1 << av1WarpedModelPrecBits);
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[2] >>= av1GmAlphaPrecDiff;
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[3] >>= av1GmAlphaPrecDiff;
            }

            if (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype == affine)
            {
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[4] >>= av1GmAlphaPrecDiff;
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[5] -= (1 << av1WarpedModelPrecBits);
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[5] >>= av1GmAlphaPrecDiff;
            }
            else
            {
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[4] = -m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[3];
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[5] = m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[2];
            }

            if (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype >= translation)
            {
                int32_t transDecFactorShift = (m_av1PicParams->m_wm[ref - lastFrame].m_wmtype == translation) ?
                    (av1GmTransOnlyPrecDiff + (m_av1PicParams->m_picInfoFlags.m_fields.m_allowHighPrecisionMv ? 0 : 1)) : av1GmTransPrecDiff;

                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[0] >>= transDecFactorShift;
                m_av1PicParams->m_wm[ref - lastFrame].m_wmmat[1] >>= transDecFactorShift;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::InitDefaultFrameContextBuffer(
        uint16_t              *ctxBuffer,
        uint8_t               index)
    {
        DECODE_CHK_NULL(ctxBuffer);

        //initialize the layout and default table info for each syntax element
        struct SyntaxElementCdfTableLayout syntaxElementsLayout[syntaxElementMax] =
        {
        //m_entryCountPerCL, m_entryCountTotal, m_startCL, *m_srcInitBuffer
        //PartI: Intra
        { 30,    12  ,    0  ,    (uint16_t *)&defaultPartitionCdf8x8[0][0] },        //    partition_8x8
        { 27,    108 ,    1  ,    (uint16_t *)&defaultPartitionCdfNxN[0][0] },        //    partition
        { 28,    28  ,    5  ,    (uint16_t *)&defaultPartitionCdf128x128[0][0] },    //    partition_128x128
        { 32,    3   ,    6  ,    (uint16_t *)&defaultSkipCdfs[0][0] },               //    skip
        { 30,    3   ,    7  ,    (uint16_t *)&defaultDeltaQCdf[0] },                 //    delta_q
        { 30,    3   ,    8  ,    (uint16_t *)&defaultDeltaLfCdf[0] },                //    delta_lf
        { 30,    12  ,    9  ,    (uint16_t *)&defaultDeltaLfMultiCdf[0][0] },        //    delta_lf_multi
        { 28,    21  ,    10 ,    (uint16_t *)&defaultSpatialPredSegTreeCdf[0][0] },  //    segment_id
        { 24,    300 ,    11 ,    (uint16_t *)&defaultKfYModeCdf[0][0][0] },          //    intra_y_mode
        { 24,    156 ,    24 ,    (uint16_t *)&defaultUvModeCdf0[0][0] },             //    uv_mode_0
        { 26,    169 ,    31 ,    (uint16_t *)&defaultUvModeCdf1[0][0] },             //    uv_mode_1
        { 32,    21  ,    38 ,    (uint16_t *)&defaultPaletteYModeCdf[0][0][0] },     //    palette_y_mode
        { 32,    2   ,    39 ,    (uint16_t *)&defaultPaletteUvModeCdf[0][0] },       //    palette_uv_mode
        { 30,    42  ,    40 ,    (uint16_t *)&defaultPaletteYSizeCdf[0][0] },        //    palette_y_size
        { 30,    42  ,    42 ,    (uint16_t *)&defaultPaletteUvSizeCdf[0][0] },       //    palette_uv_size
        { 30,    312 ,    44 ,    (uint16_t *)&defaultIntraExtTxCdf1[0][0][0] },      //    intra_tx_type_1
        { 32,    208 ,    55 ,    (uint16_t *)&defaultIntraExtTxCdf2[0][0][0] },      //    intra_tx_type_2
        { 32,    3   ,    62 ,    (uint16_t *)&defaultTxSizeCdf0[0][0] },             //    depth_0
        { 32,    18  ,    63 ,    (uint16_t *)&defaultTxSizeCdf[0][0][0] },           //    depth
        { 28,    7   ,    64 ,    (uint16_t *)&defaultCflSignCdf[0] },                //    cfl_joint_sign
        { 30,    90  ,    65 ,    (uint16_t *)&defaultCflAlphaCdf[0][0] },            //    cdf_alpha
        { 30,    48  ,    68 ,    (uint16_t *)&defaultAngleDeltaCdf[0][0] },          //    angle_delta
        { 32,    5   ,    70 ,    (uint16_t *)&defaultPaletteYColorIndexCdf0[0][0] }, //    palette_y_color_idx_0
        { 32,    10  ,    71 ,    (uint16_t *)&defaultPaletteYColorIndexCdf1[0][0] }, //    palette_y_color_idx_1
        { 30,    15  ,    72 ,    (uint16_t *)&defaultPaletteYColorIndexCdf2[0][0] }, //    palette_y_color_idx_2
        { 32,    20  ,    73 ,    (uint16_t *)&defaultPaletteYColorIndexCdf3[0][0] }, //    palette_y_color_idx_3
        { 30,    25  ,    74 ,    (uint16_t *)&defaultPaletteYColorIndexCdf4[0][0] }, //    palette_y_color_idx_4
        { 30,    30  ,    75 ,    (uint16_t *)&defaultPaletteYColorIndexCdf5[0][0] }, //    palette_y_color_idx_5
        { 28,    35  ,    76 ,    (uint16_t *)&defaultPaletteYColorIndexCdf6[0][0] }, //    palette_y_color_idx_6
        { 32,    5   ,    78 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf0[0][0] }, //    palette_uv_color_idx_0
        { 32,    10  ,    79 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf1[0][0] }, //    palette_uv_color_idx_1
        { 30,    15  ,    80 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf2[0][0] }, //    palette_uv_color_idx_2
        { 32,    20  ,    81 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf3[0][0] }, //    palette_uv_color_idx_3
        { 30,    25  ,    82 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf4[0][0] }, //    palette_uv_color_idx_4
        { 30,    30  ,    83 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf5[0][0] }, //    palette_uv_color_idx_5
        { 28,    35  ,    84 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf6[0][0] }, //    palette_uv_color_idx_6
        //coeff cdfs addressed by index
        { 32,    65  ,    86 ,    (uint16_t *)&av1DefaultTxbSkipCdfs[index][0][0][0] },               //    txb_skip
        { 32,    16  ,    89 ,    (uint16_t *)&av1DefaultEobMulti16Cdfs[index][0][0][0] },            //    eob_pt_0
        { 30,    20  ,    90 ,    (uint16_t *)&av1DefaultEobMulti32Cdfs[index][0][0][0] },            //    eob_pt_1
        { 30,    24  ,    91 ,    (uint16_t *)&av1DefaultEobMulti64Cdfs[index][0][0][0] },            //    eob_pt_2
        { 28,    28  ,    92 ,    (uint16_t *)&av1DefaultEobMulti128Cdfs[index][0][0][0] },           //    eob_pt_3
        { 32,    32  ,    93 ,    (uint16_t *)&av1DefaultEobMulti256Cdfs[index][0][0][0] },           //    eob_pt_4
        { 27,    36  ,    94 ,    (uint16_t *)&av1DefaultEobMulti512Cdfs[index][0][0][0] },           //    eob_pt_5
        { 30,    40  ,    96 ,    (uint16_t *)&av1DefaultEobMulti1024Cdfs[index][0][0][0] },          //    eob_pt_6
        { 32,    90  ,    98 ,    (uint16_t *)&av1DefaultEobExtraCdfs[index][0][0][0][0] },           //    eob_extra
        { 32,    80  ,    101,    (uint16_t *)&av1DefaultCoeffBaseEobMultiCdfs[index][0][0][0][0] },  //    coeff_base_eob
        { 30,    1260,    104,    (uint16_t *)&av1DefaultCoeffBaseMultiCdfs[index][0][0][0][0] },     //    coeff_base
        { 32,    6   ,    146,    (uint16_t *)&av1DefaultDcSignCdfs[index][0][0][0] },                //    dc_sign
        { 30,    630 ,    147,    (uint16_t *)&av1DefaultCoeffLpsMultiCdfs[index][0][0][0][0] },      //    coeff_br
        { 32,    2   ,    168,    (uint16_t *)&defaultSwitchableRestoreCdf[0] },  //    switchable_restore
        { 32,    1   ,    169,    (uint16_t *)&defaultWienerRestoreCdf[0] },      //    wiener_restore
        { 32,    1   ,    170,    (uint16_t *)&defaultSgrprojRestoreCdf[0] },     //    sgrproj_restore
        { 32,    1   ,    171,    (uint16_t *)&defaultIntrabcCdf[0] },            //    use_intrabc
        { 32,    22  ,    172,    (uint16_t *)&default_filter_intra_cdfs[0][0] }, //    use_filter_intra
        { 32,    4   ,    173,    (uint16_t *)&defaultFilterIntraModeCdf[0] },    //    filter_intra_mode
        { 30,    3   ,    174,    (uint16_t *)&defaultJointCdf[0] },              //    dv_joint_type
        { 32,    2   ,    175,    (uint16_t *)&defaultSignCdf[0][0] },            //    dv_sign
        { 32,    20  ,    176,    (uint16_t *)&defaultBitsCdf[0][0][0] },         //    dv_sbits
        { 30,    20  ,    177,    (uint16_t *)&defaultClassesCdf[0][0] },         //    dv_class
        { 32,    2   ,    178,    (uint16_t *)&defaultClass0Cdf[0][0] },          //    dv_class0
        { 30,    6   ,    179,    (uint16_t *)&defaultFpCdf[0][0] },              //    dv_fr
        { 30,    12  ,    180,    (uint16_t *)&defaultClass0FpCdf[0][0][0] },     //    dv_class0_fr
        { 32,    2   ,    181,    (uint16_t *)&defaultHpCdf[0][0] },              //    dv_hp
        { 32,    2   ,    182,    (uint16_t *)&defaultClass0HpCdf[0][0] },        //    dv_class0_hp
        //PartII: Inter
        { 32,    3   ,    183,    (uint16_t *)&defaultSkipModeCdfs[0][0] },           //    skip_mode
        { 32,    3   ,    184,    (uint16_t *)&defaultSegmentPredCdf[0][0] },         //    pred_seg_id
        { 24,    48  ,    185,    (uint16_t *)&defaultIfYModeCdf[0][0] },             //    y_mode
        { 30,    60  ,    187,    (uint16_t *)&defaultInterExtTxCdf1[0][0] },         //    inter_tx_type_1
        { 22,    44  ,    189,    (uint16_t *)&defaultInterExtTxCdf2[0][0] },         //    inter_tx_type_2
        { 32,    4   ,    191,    (uint16_t *)&defaultInterExtTxCdf3[0][0] },         //    inter_tx_type_3
        { 32,    4   ,    192,    (uint16_t *)&defaultIntraInterCdf[0][0] },          //    is_inter
        { 32,    21  ,    193,    (uint16_t *)&defaultTxfmPartitionCdf[0][0] },       //    tx_split
        { 32,    5   ,    194,    (uint16_t *)&defaultCompInterCdf[0][0] },           //    ref_mode
        { 32,    5   ,    195,    (uint16_t *)&defaultCompRefTypeCdf[0][0] },         //    comp_ref_type
        { 32,    9   ,    196,    (uint16_t *)&defaultUniCompRefCdf[0][0][0] },       //    unidir_comp_ref
        { 32,    9   ,    197,    (uint16_t *)&defaultCompRefCdf[0][0][0] },          //    ref_bit
        { 32,    6   ,    198,    (uint16_t *)&defaultCompBwdrefCdf[0][0][0] },       //    ref_bit_bwd
        { 32,    18  ,    199,    (uint16_t *)&defaultSingleRefCdf[0][0][0] },        //    single_ref_bit
        { 28,    56  ,    200,    (uint16_t *)&defaultInterCompoundModeCdf[0][0] },   // inter_compound_mode
        { 32,    6   ,    202,    (uint16_t *)&defaultNewmvCdf[0][0] },               //    is_newmv
        { 32,    2   ,    203,    (uint16_t *)&defaultZeromvCdf[0][0] },              //    is_zeromv
        { 32,    6   ,    204,    (uint16_t *)&defaultRefmvCdf[0][0] },               //    is_refmv
        { 30,    3   ,    205,    (uint16_t *)&defaultJointCdf[0] },                  //    mv_joint_type
        { 32,    2   ,    206,    (uint16_t *)&defaultSignCdf[0][0] },                //    mv_sign
        { 32,    20  ,    207,    (uint16_t *)&defaultBitsCdf[0][0][0] },             //    mv_sbits
        { 30,    20  ,    208,    (uint16_t *)&defaultClassesCdf[0][0] },             //    mv_class
        { 32,    2   ,    209,    (uint16_t *)&defaultClass0Cdf[0][0] },              //    mv_class0
        { 30,    6   ,    210,    (uint16_t *)&defaultFpCdf[0][0] },                  //    mv_fr
        { 30,    12  ,    211,    (uint16_t *)&defaultClass0FpCdf[0][0][0] },         //    mv_class0_fr
        { 32,    2   ,    212,    (uint16_t *)&defaultHpCdf[0][0] },                  //    mv_hp
        { 32,    2   ,    213,    (uint16_t *)&defaultClass0HpCdf[0][0] },            //    mv_class0_hp
        { 32,    4   ,    214,    (uint16_t *)&defaultInterintraCdf[0][0] },          //    interintra
        { 30,    12  ,    215,    (uint16_t *)&defaultInterintraModeCdf[0][0] },      //    interintra_mode
        { 32,    22  ,    216,    (uint16_t *)&defaultWedgeInterintraCdf[0][0] },     //    use_wedge_interintra
        { 30,    330 ,    217,    (uint16_t *)&defaultWedgeIdxCdf[0][0] },            //    wedge_index
        { 32,    3   ,    228,    (uint16_t *)&defaultDrlCdf[0][0] },                 //    drl_idx
        { 32,    22  ,    229,    (uint16_t *)&defaultObmcCdf[0][0] },                //    obmc_motion_mode
        { 32,    44  ,    230,    (uint16_t *)&defaultMotionModeCdf[0][0] },          //    non_obmc_motion_mode
        { 32,    6   ,    232,    (uint16_t *)&defaultCompGroupIdxCdfs[0][0] },       //    comp_group_idx
        { 32,    6   ,    233,    (uint16_t *)&defaultCompoundIdxCdfs[0][0] },        //    compound_idx
        { 32,    22  ,    234,    (uint16_t *)&defaultCompoundTypeCdf[0][0] },        //    interinter_compound_type
        { 32,    32  ,    235,    (uint16_t *)&defaultSwitchableInterpCdf[0][0] },    //    switchable_interp
        };

        for (auto idx = (uint32_t)partition8x8; idx < (uint32_t)syntaxElementMax; idx++)
        {
            DECODE_CHK_STATUS(SyntaxElementCdfTableInit(
                ctxBuffer,
                syntaxElementsLayout[idx]));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::SyntaxElementCdfTableInit(
        uint16_t                    *ctxBuffer,
        SyntaxElementCdfTableLayout SyntaxElement)
    {
        DECODE_CHK_NULL(SyntaxElement.m_srcInitBuffer);

        uint16_t    entryCountPerCL = SyntaxElement.m_entryCountPerCL;  //one entry means one uint16_t value
        uint16_t    entryCountTotal = SyntaxElement.m_entryCountTotal;  //total number of entrie for this Syntax element's CDF tables
        uint16_t    startCL         = SyntaxElement.m_startCL;

        uint16_t *src = SyntaxElement.m_srcInitBuffer;
        uint16_t *dst = ctxBuffer + startCL * 32;   //one CL equals to 32 uint16_t
        uint16_t entryCountLeft = entryCountTotal;
        while (entryCountLeft >= entryCountPerCL)
        {
            //copy one CL
            MOS_SecureMemcpy(dst, entryCountPerCL * sizeof(uint16_t), src, entryCountPerCL * sizeof(uint16_t));
            entryCountLeft -= entryCountPerCL;

            //go to next CL
            src += entryCountPerCL;
            dst += 32;
        };
        //copy the remaining which are less than a CL
        if (entryCountLeft > 0)
        {
            MOS_SecureMemcpy(dst, entryCountLeft * sizeof(uint16_t), src, entryCountLeft * sizeof(uint16_t));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureG12::UpdateDefaultCdfTable()
    {
        DECODE_FUNC_CALL();

        if (!m_defaultFcInitialized)
        {
            for (uint8_t index = 0; index < av1DefaultCdfTableNum; index++)
            {
                m_defaultCdfBuffers[index] = m_allocator->AllocateBuffer(
                    MOS_ALIGN_CEIL(m_cdfMaxNumBytes, CODECHAL_PAGE_SIZE), "TempCdfTableBuffer",
                    resourceInternalRead, lockableVideoMem);
                DECODE_CHK_NULL(m_defaultCdfBuffers[index]);

                auto data = (uint16_t *)m_allocator->LockResourceForWrite(&m_defaultCdfBuffers[index]->OsResource);
                DECODE_CHK_NULL(data);
                
                // reset all CDF tables to default values
                DECODE_CHK_STATUS(InitDefaultFrameContextBuffer(data, index));
            }

            m_defaultFcInitialized = true;//set only once, won't set again
        }

        //Calculate the current frame's Coeff CDF Q Context ID, that is the Coeff CDF Buffer index
        if (m_av1PicParams->m_baseQindex <= 20)    m_curCoeffCdfQCtx = 0;
        else if (m_av1PicParams->m_baseQindex <= 60)    m_curCoeffCdfQCtx = 1;
        else if (m_av1PicParams->m_baseQindex <= 120)   m_curCoeffCdfQCtx = 2;
        else m_curCoeffCdfQCtx = 3;

        m_defaultCdfBufferInUse = m_defaultCdfBuffers[m_curCoeffCdfQCtx];

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
