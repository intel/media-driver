/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_basic_feature.cpp
//! \brief    Defines the common interface for encode vp9 parameter
//!

#include "encode_utils.h"
#include "encode_allocator.h"

#include "encode_vp9_basic_feature.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "encode_vp9_brc.h"
#include "encode_vp9_segmentation.h"

#include "media_feature_manager.h"
#include "media_vp9_feature_defs.h"

namespace encode
{
MOS_STATUS Vp9BasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    EncodeBasicFeature::Init(setting);
    CodechalSetting *codecSettings = (CodechalSetting *)setting;

    // Need to convert from VP9_ENCODE_CHROMA_FORMAT to HCP_CHROMA_FORMAT_IDC type
    m_chromaFormat += 1;

    if (CodecHalUsesVideoEngine(m_codecFunction))
    {
        m_pakEnabled = true;
    }

    if (CodecHalUsesRenderEngine(m_codecFunction, m_standard))
    {
        m_encEnabled = true;
    }

    m_adaptiveRepakSupported  = true;

    // HME Scaling WxH
    m_downscaledWidthInMb4x   = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth   / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x       = m_downscaledWidthInMb4x  * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x      = m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth   / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x      = m_downscaledWidthInMb16x  * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x     = m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    m_minScaledDimension      = CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE;
    m_minScaledDimensionInMb  = (CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE + 15) >> 4;

    // Max tile numbers = max of number tiles for single pipe or max number of tiles for scalable pipes
    m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth,   CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH) *
                      CODECHAL_GET_HEIGHT_IN_BLOCKS(m_frameHeight, CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT);

    // Picture (width, height, size) in SB units
    m_picWidthInSb  = MOS_ROUNDUP_DIVIDE(m_oriFrameWidth,  CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_picHeightInSb = MOS_ROUNDUP_DIVIDE(m_oriFrameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    m_picSizeInSb   = m_picWidthInSb * m_picHeightInSb;

    // Application needs to pass the maxinum frame width/height
    m_maxPicWidth      = m_frameWidth;
    m_maxPicHeight     = m_frameHeight;
    m_maxPicWidthInSb  = MOS_ROUNDUP_DIVIDE(m_maxPicWidth,  CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_maxPicHeightInSb = MOS_ROUNDUP_DIVIDE(m_maxPicHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    m_maxPicSizeInSb   = m_maxPicWidthInSb * m_maxPicHeightInSb;

    if (m_pakEnabled)
    {
        // m_mvoffset looks not correct here, and corresponding setting of buffer offset in HCP_IND_OBJ, need to check with HW team.
        // keep current logic unchanged but increase the buffer size for now in case regression before we know how to correctly program these.
        m_mvOffset = MOS_ALIGN_CEIL((m_maxPicSizeInSb * 4 * sizeof(uint32_t)), CODECHAL_PAGE_SIZE);  // 3 uint32_t for HCP_PAK_OBJECT and 1 uint32_t for padding zero in kernel

        // We need additional buffer for
        // (1) 1 CL for size info at the beginning of each tile column (max of 4 vdbox in scalability mode)
        // (2) CL alignment at the end of every tile column for every SB of width
        // As a result, increase the height by 1 for allocation purposes
        uint32_t numOfLCU      = m_maxPicSizeInSb + m_maxPicWidthInSb;
        uint32_t maxNumCUInLCU = (m_maxLCUSize / m_minLCUSize) * (m_maxLCUSize / m_minLCUSize);
        m_mbCodeSize           = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * numOfLCU * (NUM_PAK_DWS_PER_LCU + 64 * NUM_DWS_PER_CU), CODECHAL_PAGE_SIZE);
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;

    // HUC enabled by default for VP9
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "VP9 Encode HUC Enable",
        MediaUserSetting::Group::Sequence);
    m_hucEnabled = outValue.Get<bool>();

    // Single pass dynamic scaling enabled by default
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "VP9 Encode Single Pass Dys Enable",
        MediaUserSetting::Group::Sequence);
    m_dysVdencMultiPassEnabled = !outValue.Get<bool>();

    // HME enabled by default for VP9
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "VP9 Encode HME",
        MediaUserSetting::Group::Sequence);
    m_hmeSupported = outValue.Get<bool>();

    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "VP9 Encode SuperHME",
        MediaUserSetting::Group::Sequence);
    m_16xMeSupported = outValue.Get<bool>();
#endif

    // Disable superHME when HME is disabled
    if (m_hmeSupported == false)
    {
        m_16xMeSupported = false;
    }

    ENCODE_CHK_STATUS_RETURN(m_ref.Init(this));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Update(params));

    EncoderParams *encodeParams = (EncoderParams *)params;

    m_vp9SeqParams = static_cast<PCODEC_VP9_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(m_vp9SeqParams);
    m_vp9PicParams = static_cast<PCODEC_VP9_ENCODE_PIC_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(m_vp9PicParams);
    m_vp9SegmentParams = static_cast<PCODEC_VP9_ENCODE_SEGMENT_PARAMS>(encodeParams->pSegmentParams);
    ENCODE_CHK_NULL_RETURN(m_vp9SegmentParams);

    m_nalUnitParams = encodeParams->ppNALUnitParams;
    ENCODE_CHK_NULL_RETURN(m_nalUnitParams);
    m_NumNalUnits = encodeParams->uiNumNalUnits;
    ENCODE_ASSERT(m_NumNalUnits == 1);

    m_targetUsage          = m_vp9SeqParams->TargetUsage;
    m_currOriginalPic      = m_vp9PicParams->CurrOriginalPic;
    m_currReconstructedPic = m_vp9PicParams->CurrReconstructedPic;
    m_pictureCodingType    = m_vp9PicParams->PicFlags.fields.frame_type == 0 ? I_TYPE : P_TYPE;

    m_bitstreamUpperBound = encodeParams->dwBitstreamSize;

    if (m_newSeq)
    {
        ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }

    // Set picture structs here
    ENCODE_CHK_STATUS_RETURN(SetPictureStructs())

    if (m_resolutionChanged)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
    }

    ENCODE_CHK_STATUS_RETURN(GetTrackedBuffers());

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp9BasicFeature::GetProfileLevelMaxFrameSize()
{
    ENCODE_FUNC_CALL();

    uint32_t profileLevelMaxFrame = m_frameWidth * m_frameHeight;
    if (m_vp9SeqParams->UserMaxFrameSize > 0)
    {
        profileLevelMaxFrame = MOS_MIN(profileLevelMaxFrame, m_vp9SeqParams->UserMaxFrameSize);
    }

    return profileLevelMaxFrame;
}

MOS_STATUS Vp9BasicFeature::Resize4x8xforDS(uint8_t bufIdx)
{
    ENCODE_FUNC_CALL();

    // Calculate the expected 4x dimensions
    uint32_t downscaledSurfaceWidth4x  = m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    uint32_t downscaledSurfaceHeight4x = ((m_downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
    downscaledSurfaceHeight4x          = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    // Calculate the expected 8x dimensions
    uint32_t downscaledSurfaceWidth8x  = downscaledSurfaceWidth4x >> 1;
    uint32_t downscaledSurfaceHeight8x = downscaledSurfaceHeight4x >> 1;

    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    // Get the 8x and 4x ds downscaled surfaces from tracked buffers
    auto trackedBuf8xDsReconSurface = m_trackedBuf->GetSurface(BufferType::ds8xSurface, bufIdx);
    auto trackedBuf4xDsReconSurface = m_trackedBuf->GetSurface(BufferType::ds4xSurface, bufIdx);

    ENCODE_CHK_NULL_RETURN(trackedBuf8xDsReconSurface);
    ENCODE_CHK_NULL_RETURN(trackedBuf4xDsReconSurface);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = Format_NV12;

    // If any dimensions of allocated surface are smaller, reallocation is needed
    if (trackedBuf8xDsReconSurface->dwWidth < downscaledSurfaceWidth8x || trackedBuf8xDsReconSurface->dwHeight < downscaledSurfaceHeight8x)
    {
        // Get the previously assigned dimensions to make sure we do not lower any dimension
        auto previous8xWidth  = trackedBuf8xDsReconSurface->dwWidth;
        auto previous8xHeight = trackedBuf8xDsReconSurface->dwHeight;

        auto new8xWidth  = MOS_MAX(previous8xWidth, downscaledSurfaceWidth8x);
        auto new8xHeight = MOS_MAX(previous8xHeight, downscaledSurfaceHeight8x);

        allocParamsForBuffer2D.dwWidth  = new8xWidth;
        allocParamsForBuffer2D.dwHeight = new8xHeight;
        allocParamsForBuffer2D.pBufName = "8xDSSurface";
        allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::ds8xSurface, allocParamsForBuffer2D));
    }

    if (trackedBuf4xDsReconSurface->dwWidth < downscaledSurfaceWidth4x || trackedBuf4xDsReconSurface->dwHeight < downscaledSurfaceHeight4x)
    {
        // Get the previously assigned dimensions to make sure we do not lower any dimension
        auto previous4xWidth  = trackedBuf4xDsReconSurface->dwWidth;
        auto previous4xHeight = trackedBuf4xDsReconSurface->dwHeight;

        auto new4xWidth  = MOS_MAX(previous4xWidth, downscaledSurfaceWidth4x);
        auto new4xHeight = MOS_MAX(previous4xHeight, downscaledSurfaceHeight4x);

        allocParamsForBuffer2D.dwWidth  = new4xWidth;
        allocParamsForBuffer2D.dwHeight = new4xHeight;
        allocParamsForBuffer2D.pBufName = "4xDSSurface";
        allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::ds4xSurface, allocParamsForBuffer2D));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::UpdateParameters()
{
    ENCODE_FUNC_CALL();

    m_prevFrameInfo.KeyFrame    = !m_vp9PicParams->PicFlags.fields.frame_type;
    m_prevFrameInfo.IntraOnly   = (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME) || m_vp9PicParams->PicFlags.fields.intra_only;
    m_prevFrameInfo.ShowFrame   = m_vp9PicParams->PicFlags.fields.show_frame;
    m_prevFrameInfo.FrameWidth  = m_oriFrameWidth;
    m_prevFrameInfo.FrameHeight = m_oriFrameHeight;

    m_lastMvTemporalBufferIndex = m_currMvTemporalBufferIndex;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::UpdateTrackedBufferParameters()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->OnSizeChange());

    m_mvDataSize = 0;

    uint32_t downscaledWidthInMb4x  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth   / SCALE_FACTOR_4x);
    uint32_t downscaledHeightInMb4x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);

    m_downscaledWidth4x = downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;

    // Account for field case, offset needs to be 4K aligned if tiled for DI surface state
    // Width will be allocated tile Y aligned, so also tile align height
    uint32_t downscaledSurfaceHeight4x = ((downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;

    m_downscaledHeight4x = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    // Segment ID buffer
    uint32_t sizeOfSegmentIdMap = m_maxPicSizeInSb * CODECHAL_CACHELINE_SIZE;
    if (sizeOfSegmentIdMap > 0)
    {
        allocParams.dwBytes  = sizeOfSegmentIdMap;
        allocParams.pBufName = "SegmentIdBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::segmentIdStreamOutBuffer, allocParams));
    }

    // Current MV temporal buffer
    uint32_t sizeOfMvTemporalBuffer = m_maxPicSizeInSb * 9 * CODECHAL_CACHELINE_SIZE;
    if (sizeOfMvTemporalBuffer > 0)
    {
        allocParams.dwBytes  = sizeOfMvTemporalBuffer;
        allocParams.pBufName = "mvTemporalBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::mvTemporalBuffer, allocParams));
    }

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::UpdateTrackedBufferParameters());

    ENCODE_CHK_STATUS_RETURN(ResizeDsReconSurfacesVdenc());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::GetTrackedBuffers()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_trackedBuf);
    ENCODE_CHK_NULL_RETURN(m_vp9PicParams);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    auto currRefList = m_ref.GetCurrRefList();
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->Acquire(currRefList, false, true));

    auto currIndex = m_trackedBuf->GetCurrIndex();

    m_resMbCodeBuffer = m_trackedBuf->GetBuffer(BufferType::mbCodedBuffer, currIndex);
    ENCODE_CHK_NULL_RETURN(m_resMbCodeBuffer);

    currRefList->ucMbCodeIdx        = currIndex;
    currRefList->resRefMbCodeBuffer = *m_resMbCodeBuffer;

    m_4xDSSurface = m_trackedBuf->GetSurface(BufferType::ds4xSurface, currIndex);
    ENCODE_CHK_NULL_RETURN(m_4xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_4xDSSurface));

    m_8xDSSurface = m_trackedBuf->GetSurface(BufferType::ds8xSurface, currIndex);
    ENCODE_CHK_NULL_RETURN(m_8xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_8xDSSurface));

    m_resMvTemporalBuffer = m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, currIndex);
    ENCODE_CHK_NULL_RETURN(m_resMvTemporalBuffer);
    m_currMvTemporalBufferIndex = currIndex;

    m_resSegmentIdBuffer = m_trackedBuf->GetBuffer(BufferType::segmentIdStreamOutBuffer, currIndex);
    ENCODE_CHK_NULL_RETURN(m_resSegmentIdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::SetSequenceStructs()
{
    ENCODE_FUNC_CALL();

    if (m_adaptiveRepakSupported)
    {
        ENCODE_CHK_STATUS_RETURN(CalculateRePakThresholds());
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::SetPictureStructs()
{
    ENCODE_FUNC_CALL();

    if (false == isHmeEnabledForTargetUsage(m_vp9SeqParams->TargetUsage))
    {
        m_hmeEnabled = m_16xMeSupported = m_32xMeSupported = false;
        m_16xMeEnabled                                     = false;
    }

    // Setup internal parameters
    uint32_t frameWidth  = m_vp9PicParams->SrcFrameWidthMinus1  + 1;
    uint32_t frameHeight = m_vp9PicParams->SrcFrameHeightMinus1 + 1;

    // dwOriFrameWidth and dwOriFrameHeight are encoded by resolution.
    // If dynamic scaling is enabled, current resolution may differ from the source resolution.
    // Only for the first frame
    if (m_frameNum == 0)
    {
        m_oriFrameWidth     = frameWidth;
        m_oriFrameHeight    = frameHeight;
        m_resolutionChanged = true;
    }
    else
    {
        // Check if there is a dynamic resolution change
        if ((m_oriFrameWidth  && (m_oriFrameWidth  != frameWidth)) ||
            (m_oriFrameHeight && (m_oriFrameHeight != frameHeight)))
        {
            m_resolutionChanged = true;
            m_oriFrameWidth     = frameWidth;
            m_oriFrameHeight    = frameHeight;
        }
        else
        {
            m_resolutionChanged = false;
        }
    }

    if (m_oriFrameWidth  == 0 || m_oriFrameWidth  > m_maxPicWidth ||
        m_oriFrameHeight == 0 || m_oriFrameHeight > m_maxPicHeight)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Picture (width, height, size) in SB units
    m_picWidthInSb  = MOS_ROUNDUP_DIVIDE(m_oriFrameWidth,  CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_picHeightInSb = MOS_ROUNDUP_DIVIDE(m_oriFrameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    m_picSizeInSb   = m_picWidthInSb * m_picHeightInSb;

    // Picture (width, height) in MB units
    m_picWidthInMb  = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
    m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
    m_frameWidth    = m_picWidthInMb  * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight   = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    // HME Scaling WxH
    m_downscaledWidthInMb4x  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth   / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x      = m_downscaledWidthInMb4x  * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x     = m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth   / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x      = m_downscaledWidthInMb16x  * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x     = m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    m_frameFieldHeight                  = m_frameHeight;
    m_frameFieldHeightInMb              = m_picHeightInMb;
    m_downscaledFrameFieldHeightInMb4x  = m_downscaledHeightInMb4x;
    m_downscaledFrameFieldHeightInMb16x = m_downscaledHeightInMb16x;

    MotionEstimationDisableCheck();

    if (m_vp9SeqParams->SeqFlags.fields.EnableDynamicScaling)
    {
        m_rawSurface.dwWidth  = MOS_ALIGN_CEIL(m_vp9PicParams->SrcFrameWidthMinus1  + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
        m_rawSurface.dwHeight = MOS_ALIGN_CEIL(m_vp9PicParams->SrcFrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_HEIGHT);
    }

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) &&
        (!m_vp9SeqParams->SeqFlags.fields.bUseRawReconRef || m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_dysBrc = false;
    m_dysCqp = false;

    // Update reference frames
    ENCODE_CHK_STATUS_RETURN(m_ref.Update());

    m_vdencPakonlyMultipassEnabled   = false;

    m_txMode = CODEC_VP9_TX_SELECTABLE;

    // For VDEnc disable HME if HME hasn't been disabled by reg key AND TU != TU1
    m_hmeSupported   = m_hmeSupported && isHmeEnabledForTargetUsage(m_vp9SeqParams->TargetUsage);
    m_16xMeSupported = m_16xMeSupported && m_hmeSupported;
    // Enable HME/SHME for frame
    m_hmeEnabled   = m_hmeSupported && m_pictureCodingType != I_TYPE && !m_vp9PicParams->PicFlags.fields.intra_only;
    m_16xMeEnabled = m_16xMeSupported && m_hmeEnabled;

    // We cannot use refresh_frame_context if HuC isn't enabled to update probs
    if (m_vp9PicParams->PicFlags.fields.refresh_frame_context && !m_hucEnabled)
    {
        ENCODE_ASSERTMESSAGE("Refresh_frame_context cannot be enabled while HuC is disabled.  HuC is needed for refresh_frame_context to be enabled.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

void Vp9BasicFeature::MotionEstimationDisableCheck()
{
    ENCODE_FUNC_CALL();

    if (m_downscaledWidth4x < m_minScaledDimension || m_downscaledWidthInMb4x < m_minScaledDimensionInMb ||
        m_downscaledHeight4x < m_minScaledDimension || m_downscaledHeightInMb4x < m_minScaledDimensionInMb)
    {
        m_32xMeSupported = false;
        m_16xMeSupported = false;
        if (m_downscaledWidth4x < m_minScaledDimension || m_downscaledWidthInMb4x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth4x     = m_minScaledDimension;
            m_downscaledWidthInMb4x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth4x);
        }
        if (m_downscaledHeight4x < m_minScaledDimension || m_downscaledHeightInMb4x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight4x     = m_minScaledDimension;
            m_downscaledHeightInMb4x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight4x);
        }
    }
    else if (m_downscaledWidth16x < m_minScaledDimension || m_downscaledWidthInMb16x < m_minScaledDimensionInMb ||
             m_downscaledHeight16x < m_minScaledDimension || m_downscaledHeightInMb16x < m_minScaledDimensionInMb)
    {
        m_32xMeSupported = false;
        if (m_downscaledWidth16x < m_minScaledDimension || m_downscaledWidthInMb16x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth16x     = m_minScaledDimension;
            m_downscaledWidthInMb16x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth16x);
        }
        if (m_downscaledHeight16x < m_minScaledDimension || m_downscaledHeightInMb16x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight16x     = m_minScaledDimension;
            m_downscaledHeightInMb16x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight16x);
        }
    }
    else
    {
        if (m_downscaledWidth32x < m_minScaledDimension || m_downscaledWidthInMb32x < m_minScaledDimensionInMb)
        {
            m_downscaledWidth32x     = m_minScaledDimension;
            m_downscaledWidthInMb32x = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_downscaledWidth32x);
        }
        if (m_downscaledHeight32x < m_minScaledDimension || m_downscaledHeightInMb32x < m_minScaledDimensionInMb)
        {
            m_downscaledHeight32x     = m_minScaledDimension;
            m_downscaledHeightInMb32x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_downscaledHeight32x);
        }
    }
}

MOS_STATUS Vp9BasicFeature::ResizeDsReconSurfacesVdenc()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    auto allocated8xWidth  = m_downscaledWidth4x >> 1;
    auto allocated8xHeight = MOS_ALIGN_CEIL(m_downscaledHeight4x >> 1, MOS_YTILE_H_ALIGNMENT) << 1;

    // Calculate the expected 4x dimensions
    uint32_t downscaledSurfaceWidth4x  = m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;

    // Account for field case, offset needs to be 4K aligned if tiled for DI surface state.
    // Width will be allocated tile Y aligned, so also tile align height.
    uint32_t downscaledSurfaceHeight4x = ((m_downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
    downscaledSurfaceHeight4x          = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    // Calculate the expected 8x dimensions
    uint32_t downscaledSurfaceWidth8x  = downscaledSurfaceWidth4x >> 1;
    uint32_t downscaledSurfaceHeight8x = downscaledSurfaceHeight4x >> 1;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = Format_NV12;

    // If any dimensions of allocated surface are smaller, reallocation is needed
    if ((downscaledSurfaceHeight4x != m_downscaledHeight4x) || (downscaledSurfaceWidth4x != m_downscaledWidth4x))
    {
        allocParamsForBuffer2D.dwWidth  = downscaledSurfaceWidth4x;
        allocParamsForBuffer2D.dwHeight = downscaledSurfaceHeight4x;
        allocParamsForBuffer2D.pBufName = "4xDSSurface";
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::ds4xSurface, allocParamsForBuffer2D));
    }

    if ((downscaledSurfaceHeight8x != allocated8xHeight) || (downscaledSurfaceWidth8x != allocated8xWidth))
    {
        allocParamsForBuffer2D.dwWidth  = downscaledSurfaceWidth8x;
        allocParamsForBuffer2D.dwHeight = downscaledSurfaceHeight8x;
        allocParamsForBuffer2D.pBufName = "8xDSSurface";
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::ds8xSurface, allocParamsForBuffer2D));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9BasicFeature::CalculateRePakThresholds()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_vp9SeqParams);

    int32_t repakSavingThreshold = 0;

    if (m_prevFrameInfo.FrameWidth  != m_oriFrameWidth ||
        m_prevFrameInfo.FrameHeight != m_oriFrameHeight)
    {
        if (TargetUsage::isQuality(m_oriTargetUsage))
        {
            repakSavingThreshold =  2; // Quality mode
        }
        else if (TargetUsage::isSpeed(m_oriTargetUsage)) 
        {
            repakSavingThreshold = 80; // Speed mode
        }
        else
        {
            repakSavingThreshold = 10; // Normal mode
        }

        int32_t scale = (m_oriFrameWidth * m_oriFrameHeight) / (176 * 144);
        if (!scale)
        {
            scale = 1;
        }

        for (auto i = 0; i < CODEC_VP9_QINDEX_RANGE; ++i)
        {
            double tempQp = i - 144.0;

            int32_t b                              = (int32_t)(92.5 * i);
            int32_t c                              = (int32_t)(1.6 * tempQp * tempQp);
            int32_t d                              = (int32_t)(0.01 * tempQp * tempQp * tempQp);
            int32_t threshold                      = (int32_t)((18630 - b + c - d) / 10);
            int32_t calculatedRepakSavingThreshold = repakSavingThreshold * scale;

            // To avoid overflow of the integer threshold, it must be (RepakSavingThreshold * scale) <= 40342
            if (calculatedRepakSavingThreshold > CODEC_VP9_MAX_REPAK_THRESHOLD)
            {
                calculatedRepakSavingThreshold = CODEC_VP9_MAX_REPAK_THRESHOLD;
            }
            m_rePakThreshold[i] = calculatedRepakSavingThreshold * threshold;
        }
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t Vp9BasicFeature::Convert2SignMagnitude(int32_t val, uint32_t signBitPos) const
{
    uint32_t retVal        = 0;
    uint32_t magnitudeMask = (1 << (signBitPos - 1)) - 1;

    if (val < 0)
    {
        retVal = ((magnitudeMask + 1) | (-val & magnitudeMask));
    }
    else
    {
        retVal = val & magnitudeMask;
    }
    return retVal;
}

bool Vp9BasicFeature::isHmeEnabledForTargetUsage(uint8_t targetUsage) const
{
    if (TargetUsage::QUALITY == targetUsage)
    {
        return true;
    }
    return false;
}

MHW_SETPAR_DECL_SRC(HCP_VP9_PIC_STATE, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    uint32_t curFrameWidth  = m_vp9PicParams->SrcFrameWidthMinus1  + 1;
    uint32_t curFrameHeight = m_vp9PicParams->SrcFrameHeightMinus1 + 1;

    params.frameWidthInPixelsMinus1   = MOS_ALIGN_CEIL(curFrameWidth,  CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    params.frameHeightInPixelsMinus1  = MOS_ALIGN_CEIL(curFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    params.frameType                  = m_vp9PicParams->PicFlags.fields.frame_type;
    params.adaptProbabilitiesFlag     = !m_vp9PicParams->PicFlags.fields.error_resilient_mode && !m_vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    params.intraOnlyFlag              = m_vp9PicParams->PicFlags.fields.intra_only;
    params.allowHiPrecisionMv         = m_vp9PicParams->PicFlags.fields.allow_high_precision_mv;
    params.mcompFilterType            = m_vp9PicParams->PicFlags.fields.mcomp_filter_type;
    params.hybridPredictionMode       = m_vp9PicParams->PicFlags.fields.comp_prediction_mode == 2;
    params.selectableTxMode           = m_txMode == 4;
    params.refreshFrameContext        = m_vp9PicParams->PicFlags.fields.refresh_frame_context;
    params.errorResilientMode         = m_vp9PicParams->PicFlags.fields.error_resilient_mode;
    params.frameParallelDecodingMode  = m_vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    params.filterLevel                = m_vp9PicParams->filter_level;
    params.sharpnessLevel             = m_vp9PicParams->sharpness_level;
    params.segmentationEnabled        = m_vp9PicParams->PicFlags.fields.segmentation_enabled;
    params.segmentationUpdateMap      = m_vp9PicParams->PicFlags.fields.segmentation_update_map;
    params.segmentationTemporalUpdate = m_vp9PicParams->PicFlags.fields.segmentation_temporal_update;
    params.losslessMode               = m_vp9PicParams->PicFlags.fields.LosslessFlag;

    params.log2TileRow                = m_vp9PicParams->log2_tile_rows;
    params.log2TileColumn             = m_vp9PicParams->log2_tile_columns;

    params.chromaSamplingFormat       = m_vp9SeqParams->SeqFlags.fields.EncodedFormat;
    params.bitdepthMinus8             = (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth == VP9_ENCODED_BIT_DEPTH_10) ? 2 : 0;

    params.baseQIndexSameAsLumaAc     = m_vp9PicParams->LumaACQIndex;
    params.headerInsertionEnable      = 1;

    params.chromaAcQIndexDelta        = Convert2SignMagnitude(m_vp9PicParams->ChromaACQIndexDelta, 5);
    params.chromaDcQIndexDelta        = Convert2SignMagnitude(m_vp9PicParams->ChromaDCQIndexDelta, 5);
    params.lumaDcQIndexDelta          = Convert2SignMagnitude(m_vp9PicParams->LumaDCQIndexDelta, 5);

    params.bitOffsetForLfRefDelta         = m_vp9PicParams->BitOffsetForLFRefDelta;
    params.bitOffsetForLfModeDelta        = m_vp9PicParams->BitOffsetForLFModeDelta;
    params.bitOffsetForLfLevel            = m_vp9PicParams->BitOffsetForLFLevel;
    params.bitOffsetForQIndex             = m_vp9PicParams->BitOffsetForQIndex;
    params.bitOffsetForFirstPartitionSize = m_vp9PicParams->BitOffsetForFirstPartitionSize;

    params.vdencPakOnlyPass = m_vdencPakonlyMultipassEnabled;

    m_ref.MHW_SETPAR_F(HCP_VP9_PIC_STATE)(params);

    return MOS_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////

static inline uint8_t getNumRefFrames(uint8_t frameType, uint8_t numRefFrames)
{
    return (frameType ? numRefFrames : 0);  // frame_type == 1 ? NON_KEY_FRAME : KEY_FRAME
}

static inline bool refIdxEqual(PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams)
{
    uint32_t lastRefIdx   = vp9PicParams->RefFlags.fields.LastRefIdx;
    uint32_t altRefIdx    = vp9PicParams->RefFlags.fields.AltRefIdx;
    uint32_t goldenRefIdx = vp9PicParams->RefFlags.fields.GoldenRefIdx;

    if (lastRefIdx == altRefIdx && altRefIdx == goldenRefIdx)
    {
        return true;
    }
    return false;
}

static inline bool isTemporalMvpEnabledForTargetUsage(uint8_t targetUsage)
{
    if (TargetUsage::QUALITY == targetUsage)
    {
        return false;
    }
    return true;
}

static inline bool isTemporalMvpEnabled(uint8_t targetUsage, uint32_t frameType, uint32_t lastFrameType, PCODEC_VP9_ENCODE_PIC_PARAMS vp9PicParams)
{
    return frameType && !lastFrameType;  // frame_type == 1 ? NON_KEY_FRAME : KEY_FRAME
}

////////////////////////////////////////////////////////////////////////////////

MHW_SETPAR_DECL_SRC(VDENC_CMD1, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();
    auto settings = static_cast<Vp9VdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd1Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, false));
    }
    return MOS_STATUS_SUCCESS;
}    

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    MHW_MI_CHK_NULL(m_vp9SeqParams);
    MHW_MI_CHK_NULL(m_rawSurfaceToPak);

    // DW2..5 - DW0

    params.width = MOS_ALIGN_CEIL(m_oriFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH);
    params.height = MOS_ALIGN_CEIL(m_oriFrameHeight, CODEC_VP9_MIN_BLOCK_HEIGHT);
    params.displayFormatSwizzle = m_vp9SeqParams->SeqFlags.fields.DisplayFormatSwizzle;

    auto waTable = m_osInterface == nullptr ? nullptr : m_osInterface->pfnGetWaTable(m_osInterface);
    if (waTable)
    {
        if (!MEDIA_IS_WA(waTable, Wa_Vp9UnalignedHeight))
        {
            params.height = m_oriFrameHeight;
        }
    }
    // DW2..5 - DW1

    params.tileType             = m_rawSurfaceToPak->TileType;
    params.tileModeGmm          = m_rawSurfaceToPak->TileModeGMM;
    params.format               = m_rawSurfaceToPak->Format;
    params.gmmTileEn            = m_rawSurfaceToPak->bGMMTileEnabled;
    params.pitch                = m_rawSurfaceToPak->dwPitch;
    params.chromaDownsampleFilterControl = 7;

    // DW2..5 - DW2

    params.uOffset              = m_rawSurfaceToPak->YoffsetForUplane;

    // DW2..5 - DW3

    params.vOffset              = m_rawSurfaceToPak->YoffsetForVplane;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    MHW_MI_CHK_NULL(m_8xDSSurface);

    // DW2..5

    params.pitchStage1       = m_8xDSSurface->dwPitch;
    params.tileTypeStage1    = m_8xDSSurface->TileType;
    params.tileModeGmmStage1 = m_8xDSSurface->TileModeGMM;
    params.gmmTileEnStage1   = m_8xDSSurface->bGMMTileEnabled;
    params.uOffsetStage1     = m_8xDSSurface->YoffsetForUplane;
    params.vOffsetStage1     = m_8xDSSurface->YoffsetForVplane;
    params.heightStage1      = m_8xDSSurface->dwHeight;
    params.widthStage1       = m_8xDSSurface->dwWidth;

    MHW_MI_CHK_NULL(m_4xDSSurface);

    // DW 6..9

    params.pitchStage2       = m_4xDSSurface->dwPitch;
    params.tileTypeStage2    = m_4xDSSurface->TileType;
    params.tileModeGmmStage2 = m_4xDSSurface->TileModeGMM;
    params.gmmTileEnStage2   = m_4xDSSurface->bGMMTileEnabled;
    params.uOffsetStage2     = m_4xDSSurface->YoffsetForUplane;
    params.vOffsetStage2     = m_4xDSSurface->YoffsetForVplane;
    params.heightStage2      = m_4xDSSurface->dwHeight;
    params.widthStage2       = m_4xDSSurface->dwWidth;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    MHW_MI_CHK_NULL(m_vp9PicParams);
    auto vp9PicParams = m_vp9PicParams;
    uint8_t frameType = vp9PicParams->PicFlags.fields.frame_type; // frame_type == 1 ? NON_KEY_FRAME : KEY_FRAME

    MHW_MI_CHK_NULL(m_vp9SeqParams);
    uint8_t targetUsage = m_vp9SeqParams->TargetUsage;

    ENCODE_CHK_COND_RETURN(!TargetUsage::isValid(targetUsage), "TargetUsage is invalid");

    uint32_t lastFrameType = m_prevFrameInfo.KeyFrame; // frame_type == 1 ? NON_KEY_FRAME : KEY_FRAME

    uint8_t numRefFrames = getNumRefFrames(frameType, m_ref.NumRefFrames());

    bool segmentationEnabled = vp9PicParams->PicFlags.fields.segmentation_enabled;

    // DW1
    params.width  = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameWidthMinus1  + 1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1; // FrameWidthInPixelsMinusOne
    params.height = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1; // FrameHeightInPixelsMinusOne

    // DW2
    params.transformSkip = 0;
    params.temporalMvp   = isTemporalMvpEnabled(targetUsage, frameType, lastFrameType, vp9PicParams);
    params.pictureType   = frameType;

    // DW3
    params.pocL1Ref1 = 0; // PocNumberForRefid1InL1
    params.pocL0Ref1 = 0; // PocNumberForRefid1InL0
    params.pocL1Ref0 = 0; // BwdPocNumberForRefid0InL1
    params.pocL0Ref0 = 0; // FwdPocNumberForRefid0InL0

    // DW4
    params.pocL1Ref3 = 0; // PocNumberForRefid3InL1
    params.pocL0Ref3 = 0; // PocNumberForRefid3InL0
    params.pocL1Ref2 = 0; // PocNumberForRefid2InL1
    params.pocL0Ref2 = 0; // PocNumberForRefid2InL0

    // DW5
    params.numRefL1 = 0;             // NumRefIdxL1Minus1 + 1
    params.numRefL0 = numRefFrames;  // NumRefIdxL0Minus1 + 1

    // DW7
    params.pakOnlyMultiPass     = m_vdencPakonlyMultipassEnabled;
    params.tiling               = (vp9PicParams->log2_tile_columns != 0) || (vp9PicParams->log2_tile_rows != 0); // TilingEnable
    params.segmentationTemporal = frameType ? m_prevFrameSegEnabled : 0;                                         // SegmentationMapTemporalPredictionEnable
    params.segmentation         = segmentationEnabled;                                                           // SegmentationEnable

    // DW16
    params.maxQp = 0xff; // MaxQp
    params.minQp = 0;    // MinQp

    // DW17
    params.temporalMvEnableForIntegerSearch = params.temporalMvp; // TemporalMVEnableForIntegerSearch

    // DW21
    params.qpAdjustmentForRollingI  = 0;
    params.intraRefresh             = 0;
    params.intraRefreshMbSizeMinus1 = 1;

    // DW26
    params.vp9DynamicSlice = ((m_ref.DysRefFrameFlags() != DYS_REF_NONE) && !m_dysVdencMultiPassEnabled);

    // DW27
    params.qpPrimeYAc = vp9PicParams->LumaACQIndex;
    params.qpPrimeYDc = params.qpPrimeYAc + vp9PicParams->LumaDCQIndexDelta;

    // DW61
    params.av1RefId[0][0] = 0;
    params.av1RefId[0][1] = 0;
    params.av1RefId[0][2] = 0;
    params.av1RefId[0][3] = 0; 
    params.av1RefId[1][0] = 0;
    params.av1RefId[1][1] = 0;
    params.av1RefId[1][2] = 0;
    params.av1RefId[1][3] = 0;

    auto settings = static_cast<Vp9VdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd2Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, false));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    MHW_MI_CHK_NULL(m_vp9SeqParams);

    // DW1

    params.chromaType = m_vp9SeqParams->SeqFlags.fields.EncodedFormat + 1;

    params.bitDepthMinus8 = 0;
    if (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth == VP9_ENCODED_BIT_DEPTH_10)
    {
        params.bitDepthMinus8 = 2;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    params.surfaceRaw      = m_rawSurfaceToPak;
    params.surfaceDsStage1 = m_8xDSSurface;
    params.surfaceDsStage2 = m_4xDSSurface;

    params.streamInBuffer  = m_recycleBuf->GetBuffer(RecycleResId::StreamInBuffer, m_frameNum);

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);

    if (m_mmcState->IsMmcEnabled())
    {
        params.mmcEnabled = true;
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_rawSurface), &params.mmcStateRaw));
    }
    else
    {
        params.mmcEnabled = false;
        params.mmcStateRaw         = MOS_MEMCOMP_DISABLED;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_WAIT, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    params.iStallVdboxPipeline = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_PIPE_MODE_SELECT, Vp9BasicFeature)
{
    ENCODE_FUNC_CALL();

    params.decoderShortFormatMode = 1;

    if (m_decodeInUse)
    {
        params.Mode        = CODECHAL_DECODE_MODE_AVCVLD;
        params.codecSelect = decoderCodec;
    }
    else
    {
        params.Mode                           = m_mode;
        params.codecSelect                    = encoderCodec;
        params.vdencMode                      = 1;
        params.scaledSurfaceEnable            = true;
        params.frameStatisticsStreamoutEnable = true;
    }

    params.standardSelect = CodecHal_GetStandardFromMode(params.Mode);

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
