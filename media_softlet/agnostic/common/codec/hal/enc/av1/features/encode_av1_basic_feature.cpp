/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     encode_av1_basic_feature.cpp
//! \brief    Defines the common interface for encode av1 parameter
//!

#include "encode_av1_basic_feature.h"
#include "encode_utils.h"
#include "encode_allocator.h"
#include "encode_av1_vdenc_const_settings.h"
#include "mos_solo_generic.h"

namespace encode
{
MOS_STATUS Av1BasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    EncodeBasicFeature::Init(setting);
    CodechalSetting *codecSettings = (CodechalSetting*)setting;

    ENCODE_CHK_STATUS_RETURN(m_ref.Init(this));

    MEDIA_WA_TABLE* waTable = GetWaTable();
    ENCODE_CHK_NULL_RETURN(waTable);

    MediaUserSetting::Value outValue;

#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "AV1 Enable SW Back Annotation",
        MediaUserSetting::Group::Sequence);
    m_enableSWBackAnnotation = outValue.Get<bool>();

    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "AV1 Enable SW Stitching",
        MediaUserSetting::Group::Sequence,
        m_osInterface->pOsContext);
    m_enableSWStitching = outValue.Get<bool>();
    m_enableTileStitchByHW = !m_enableSWStitching;

    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "Encode Enable NonDefault Mapping",
        MediaUserSetting::Group::Sequence);
    m_enableNonDefaultMapping = outValue.Get<bool>();
#endif

    if (MEDIA_IS_WA(waTable, WaEnableOnlyASteppingFeatures))
    {
        m_enableNonDefaultMapping = false;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "AV1 Encode Adaptive Rounding Enable",
        MediaUserSetting::Group::Sequence);
    bool adaptiveRounding = outValue.Get<bool>();
    if (adaptiveRounding)
        m_roundingMethod = RoundingMethod::adaptiveRounding;
#endif
    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS CheckLrParams(const CODEC_AV1_ENCODE_PICTURE_PARAMS &m_av1PicParams)
{
    auto lr = m_av1PicParams.LoopRestorationFlags.fields;
    if (lr.cbframe_restoration_type || lr.crframe_restoration_type || lr.yframe_restoration_type)
    {
        // Check whether coding size for Luma is 64x64, and Chroma's coding size is half of Luma's
        if ((lr.lr_unit_shift != 0) || (lr.lr_uv_shift != 1))
        {
            ENCODE_ASSERTMESSAGE("Invalid Coding Size");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Update(params));

    EncoderParamsAV1 *encodeParams = (EncoderParamsAV1 *)params;

    m_av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(m_av1SeqParams);
    m_av1PicParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(m_av1PicParams);

    m_nalUnitParams = encodeParams->ppNALUnitParams;
    ENCODE_CHK_NULL_RETURN(m_nalUnitParams);
    m_NumNalUnits = encodeParams->uiNumNalUnits;
    if (m_NumNalUnits >= MAX_NUM_OBU_TYPES)
    {
        ENCODE_ASSERTMESSAGE("The num of OBU header exceeds the max value.");
        return MOS_STATUS_USER_CONTROL_MAX_DATA_SIZE;
    }
    m_AV1metaDataOffset = encodeParams->AV1metaDataOffset;

    m_appHdrSize = m_appHdrSizeExcludeFrameHdr = 0;
    m_targetUsage = m_av1SeqParams->TargetUsage;
    m_currOriginalPic = m_av1PicParams->CurrOriginalPic;

    if (IsRateControlBrc(m_av1SeqParams->RateControlMethod))
    {
        // As adaptive rounding need to lock statistics buffer, for CQP case, it was done in driver(check Av1EncodeTile's set VDENC_CMD2)
        // The lock operation will bring functional issue under "asyc != 1" condition, as well as pref issue need to be considered.
        // So we choose to by default disable adaptive rounding for CQP, but for BRC, statistics read operation will move to HuC. it's all 
        // fine by nature. so we will open adaptive rounding for BRC all the time as it will bring quality gain.
        m_roundingMethod = RoundingMethod::adaptiveRounding;
    }

    for (uint32_t i = 0; i < m_NumNalUnits; i++)
    {
        m_appHdrSize += m_nalUnitParams[i]->uiSize;
        if (IsFrameHeader(*(m_bsBuffer.pBase + m_nalUnitParams[i]->uiOffset)))
        {
            break;
        }
        m_appHdrSizeExcludeFrameHdr += m_nalUnitParams[i]->uiSize;
    }

    switch (m_av1PicParams->PicFlags.fields.frame_type)
    {
        case keyFrame:
        case intraOnlyFrame:
        {
            m_pictureCodingType = I_TYPE;
            break;
        }
        case interFrame:
        case sFrame:
        {
            m_pictureCodingType = P_TYPE;
            break;
        }
        default:
        {
            m_pictureCodingType = 0;
            break;
        }
    }

    uint32_t frameWidth = m_av1PicParams->frame_width_minus1 + 1;

    uint32_t frameHeight = m_av1PicParams->frame_height_minus1 + 1;

    // super block with 128x128 is not supported
    m_isSb128x128 = false;
    int32_t mibSizeLog2 = m_isSb128x128 ? av1MaxMibSizeLog2 : av1MinMibSizeLog2;
    int32_t miCols = MOS_ALIGN_CEIL(frameWidth, 8) >> av1MiSizeLog2;
    int32_t miRows = MOS_ALIGN_CEIL(frameHeight, 8) >> av1MiSizeLog2;

    m_miCols = MOS_ALIGN_CEIL(miCols, 1 << mibSizeLog2);
    m_miRows = MOS_ALIGN_CEIL(miRows, 1 << mibSizeLog2);

    m_picWidthInSb = m_miCols >> mibSizeLog2;
    m_picHeightInSb = m_miRows >> mibSizeLog2;

    // EnableFrameOBU thread safety
    if (m_av1PicParams->PicFlags.fields.EnableFrameOBU)
    {
        m_frameHdrOBUSizeByteOffset = m_av1PicParams->FrameHdrOBUSizeByteOffset;
    }

    // Only for first frame
    if (m_frameNum == 0)
    {
        m_oriFrameHeight = frameHeight;
        m_oriFrameWidth = frameWidth;
        m_resolutionChanged = true;
    }
    else
    {
        // check if there is a dynamic resolution change
        if ((m_oriFrameHeight && (m_oriFrameHeight != frameHeight)) ||
            (m_oriFrameWidth && (m_oriFrameWidth != frameWidth)))
        {
            m_resolutionChanged = true;
            m_oriFrameHeight = frameHeight;
            m_oriFrameWidth = frameWidth;
        }
        else
        {
            m_resolutionChanged = false;
        }
    }

    if (m_resolutionChanged)
    {
        m_picWidthInMb  = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
        m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
        m_frameWidth    = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
        m_frameHeight   = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;
        ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
    }

    ENCODE_CHK_STATUS_RETURN(CheckLrParams(*m_av1PicParams));

    m_enableCDEF = !(IsFrameLossless(*m_av1PicParams) 
        || m_av1PicParams->PicFlags.fields.allow_intrabc
        || !(m_av1SeqParams->CodingToolFlags.fields.enable_cdef));

    // Update reference frames
    ENCODE_CHK_STATUS_RETURN(m_ref.Update());

    // Reset stream in
    // N.B. Update for Stream In will be called by features which use it
    m_streamIn.Reset();

    ENCODE_CHK_STATUS_RETURN(UpdateDefaultCdfTable());

    ENCODE_CHK_STATUS_RETURN(GetTrackedBuffers());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeature::UpdateTrackedBufferParameters()
{
    ENCODE_FUNC_CALL();

    m_trackedBuf->OnSizeChange();

    // Here to calculate the max coded data size needed for one frame according to LCU and CU partitions
    //
    // 1.LCU and CU num:
    // LCU size is 64x64 so we can get the LCU num according to frame width and hight
    // The restrict min CU size is 8x8 so we can get totally max 64 CUs per LCU
    //
    // 2.LCU and CU data size:
    // LCU header size 5DW, CU size 8DW. And a DW size is 64bits(2 * sizeof(uint32_t))
    // 
    // 3.Calculate the max coded data size needed
    //
    // 4.Finally need to do page align for CL alignment at the end of the frame
    uint32_t numOfLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, av1SuperBlockWidth) * MOS_ROUNDUP_DIVIDE(m_frameHeight, av1SuperBlockHeight);
    m_mbCodeSize = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * (numOfLCU * 5 + numOfLCU * 64 * 8), CODECHAL_PAGE_SIZE);
    m_mvDataSize = 0;

    uint32_t downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    uint32_t downscaledHeightInMb4x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);

    m_downscaledWidth4x =
        downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;

    // Account for field case, offset needs to be 4K aligned if tiled for DI surface state.
    // Width will be allocated tile Y aligned, so also tile align height.
    uint32_t downscaledSurfaceHeight4x = ((downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;

    m_downscaledHeight4x = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type               = MOS_GFXRES_BUFFER;
    allocParams.TileType           = MOS_TILE_LINEAR;
    allocParams.Format             = Format_Buffer;
    allocParams.Flags.bNotLockable = !m_lockableResource;

    uint32_t totalSbPerFrame    = m_picWidthInSb * m_picHeightInSb;

    const uint16_t num4x4BlocksIn64x64Sb   = 256;
    const uint16_t num4x4BlocksIn128x128Sb = 1024;
    const uint32_t sizeOfSegmentIdMap = ((m_isSb128x128) ? num4x4BlocksIn128x128Sb : num4x4BlocksIn64x64Sb) * totalSbPerFrame;

    if (sizeOfSegmentIdMap > 0)
    {
        allocParams.dwBytes  = sizeOfSegmentIdMap;
        allocParams.pBufName = "segmentIdStreamOutBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::segmentIdStreamOutBuffer, allocParams));
    }

    allocParams.dwBytes  = MOS_ALIGN_CEIL(m_cdfMaxNumBytes, CODECHAL_PAGE_SIZE);
    allocParams.pBufName = "bwdAdaptCdfBuffer";
    allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::bwdAdaptCdfBuffer, allocParams));

    uint32_t sizeOfMvTemporalbuffer = CODECHAL_CACHELINE_SIZE * ((m_isSb128x128) ? 16 : 4) * totalSbPerFrame;
    if (sizeOfMvTemporalbuffer > 0)
    {
        allocParams.dwBytes  = sizeOfMvTemporalbuffer;
        allocParams.pBufName = "mvTemporalBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;

        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::mvTemporalBuffer, allocParams));
    }

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::UpdateTrackedBufferParameters());

    return MOS_STATUS_SUCCESS;
}

uint32_t Av1BasicFeature::GetProfileLevelMaxFrameSize()
{
    ENCODE_FUNC_CALL();

    //TBD

    return MOS_STATUS_SUCCESS;
}

uint32_t Av1BasicFeature::GetAppHdrSizeInBytes(bool excludeFrameHdr) const
{
    return excludeFrameHdr ? m_appHdrSizeExcludeFrameHdr : m_appHdrSize;
}

MOS_STATUS Av1BasicFeature::UpdateFormat(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams* encodeParams = (EncoderParams*)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS av1SeqParams = nullptr;

    av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    if (m_chromaFormat != AVP_CHROMA_FORMAT_YUV420)
    {
        ENCODE_ASSERTMESSAGE("Invalid output chromat format!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_outputChromaFormat = m_chromaFormat;

    // Set surface bit info according to surface format.
    switch(m_rawSurface.Format)
    {
    case Format_P010:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        m_is10Bit  = true;
        m_bitDepth = 10;
        break;
    case Format_NV12:
        m_is10Bit  = false;
        m_bitDepth = 8;
        break;
    default:
        m_is10Bit  = false;
        m_bitDepth = 8;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeature::UpdateDefaultCdfTable()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_av1PicParams);

    uint32_t cdfTableSize = MOS_ALIGN_CEIL(m_cdfMaxNumBytes, CODECHAL_CACHELINE_SIZE);

    if (!m_defaultFcInitialized)
    {
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type            = MOS_GFXRES_BUFFER;
        allocParams.TileType        = MOS_TILE_LINEAR;
        allocParams.Format          = Format_Buffer;
        allocParams.dwBytes         = cdfTableSize * 4;  // totally 4 cdf tables according to spec
        allocParams.pBufName        = "Av1CdfTablesBuffer";
        allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
        m_defaultCdfBuffers         = m_allocator->AllocateResource(allocParams, true);

        auto data = (uint16_t *)m_allocator->LockResourceForWrite(m_defaultCdfBuffers);
        ENCODE_CHK_NULL_RETURN(data);
        for (uint8_t index = 0; index < 4; index++)
        {
            ENCODE_CHK_STATUS_RETURN(InitDefaultFrameContextBuffer(data + cdfTableSize * index / sizeof(uint16_t), index));
        }
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_defaultCdfBuffers));

        if (!IsRateControlBrc(m_av1SeqParams->RateControlMethod))
        {
            m_defaultCdfBufferInUse = m_defaultCdfBuffers;
        }
        else
        {
            allocParams.dwBytes           = cdfTableSize;
            allocParams.pBufName          = "ActiveAv1CdfTableBuffer";
            allocParams.ResUsageType    = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ;
            m_defaultCdfBufferInUse       = m_allocator->AllocateResource(allocParams, true);
            m_defaultCdfBufferInUseOffset = 0;
        }

        m_defaultFcInitialized = true;//set only once, won't set again
    }

    if (m_av1PicParams->primary_ref_frame == av1PrimaryRefNone && !IsRateControlBrc(m_av1SeqParams->RateControlMethod))
    {
        //Calculate the current frame's Coeff CDF Q Context ID, that is the Coeff CDF Buffer index
        uint32_t curCoeffCdfQCtx = 0;
        if (m_av1PicParams->base_qindex <= 20)    curCoeffCdfQCtx = 0;
        else if (m_av1PicParams->base_qindex <= 60)    curCoeffCdfQCtx = 1;
        else if (m_av1PicParams->base_qindex <= 120)   curCoeffCdfQCtx = 2;
        else curCoeffCdfQCtx = 3;
        m_defaultCdfBufferInUseOffset = curCoeffCdfQCtx * cdfTableSize;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeature::GetTrackedBuffers()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);
    ENCODE_CHK_NULL_RETURN(m_av1PicParams);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    auto currRefList = m_ref.GetCurrRefList();
    m_trackedBuf->Acquire(currRefList, false);

    m_resMbCodeBuffer = m_trackedBuf->GetBuffer(BufferType::mbCodedBuffer, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_resMbCodeBuffer);

    m_4xDSSurface = m_trackedBuf->GetSurface(BufferType::ds4xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_4xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_4xDSSurface));

    m_8xDSSurface = m_trackedBuf->GetSurface(BufferType::ds8xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_8xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_8xDSSurface));

    m_resMvTemporalBuffer = m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_resMvTemporalBuffer);

    return MOS_STATUS_SUCCESS;
}

Av1StreamIn *Av1BasicFeature::GetStreamIn()
{
    m_streamIn.Init(this, this->m_allocator, this->m_osInterface);
    return &m_streamIn;
}

MOS_STATUS Av1BasicFeature::GetSurfaceMmcInfo(PMOS_SURFACE surface, MOS_MEMCOMP_STATE& mmcState, uint32_t& compressionFormat) const
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(surface);

#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(surface, &mmcState));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(surface, &compressionFormat));
    }
    else
    {
        mmcState = MOS_MEMCOMP_DISABLED;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS InitDefaultFrameContextBuffer(
    uint16_t                 *ctxBuffer,
    uint8_t                  index,
    Av1CdfTableSyntaxElement begin,
    Av1CdfTableSyntaxElement end)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(ctxBuffer);

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
    { 28,    21  ,    10 ,   (uint16_t *)&defaultSpatialPredSegTreeCdf[0][0] },   //    segment_id
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
    { 32,    5   ,    78 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf0[0][0] }, //   palette_uv_color_idx_0
    { 32,    10  ,    79 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf1[0][0] }, //   palette_uv_color_idx_1
    { 30,    15  ,    80 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf2[0][0] }, //   palette_uv_color_idx_2
    { 32,    20  ,    81 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf3[0][0] }, //   palette_uv_color_idx_3
    { 30,    25  ,    82 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf4[0][0] }, //   palette_uv_color_idx_4
    { 30,    30  ,    83 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf5[0][0] }, //   palette_uv_color_idx_5
    { 28,    35  ,    84 ,    (uint16_t *)&defaultPaletteUvColorIndexCdf6[0][0] }, //   palette_uv_color_idx_6
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
    { 28,    56  ,    200,    (uint16_t *)&defaultInterCompoundModeCdf[0][0] },   //    inter_compound_mode
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

    for (auto idx = (uint32_t)begin; idx < (uint32_t)end; idx++)
    {
        ENCODE_CHK_STATUS_RETURN(SyntaxElementCdfTableInit(
            ctxBuffer,
            syntaxElementsLayout[idx]));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SyntaxElementCdfTableInit(
    uint16_t                    *ctxBuffer,
    SyntaxElementCdfTableLayout SyntaxElement)
{
    ENCODE_CHK_NULL_RETURN(ctxBuffer);
    ENCODE_CHK_NULL_RETURN(SyntaxElement.m_srcInitBuffer);

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

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1BasicFeature)
{
    params.standardSelect    = 3;  // av1
    params.chromaType        = m_outputChromaFormat;
    params.bitDepthMinus8    = m_is10Bit ? 2 : 0;
    params.wirelessSessionId = 0;
    params.streamIn          = false;
    params.randomAccess      = !m_ref.IsLowDelay();

    params.rgbEncodingMode                       = m_rgbEncodingEnable;
    params.bt2020RGB2YUV                         = m_av1SeqParams->InputColorSpace == ECOLORSPACE_P2020;
    params.rgbInputStudioRange                   = params.bt2020RGB2YUV ? m_av1SeqParams->SeqFlags.fields.RGBInputStudioRange : 0;
    params.convertedYUVStudioRange               = params.bt2020RGB2YUV ? m_av1SeqParams->SeqFlags.fields.ConvertedYUVStudioRange : 0;
    if (m_captureModeEnable)
    {
        params.captureMode = 1;
        params.tailPointerReadFrequency = 0x50;
    }

    if (m_dualEncEnable)
    {
        params.scalabilityMode = true;
        params.tileBasedReplayMode = true;
    }

    params.frameStatisticsStreamOut = IsRateControlBrc(m_av1SeqParams->RateControlMethod) || m_roundingMethod == RoundingMethod::adaptiveRounding;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, Av1BasicFeature)
{
    params.pitch                         = m_rawSurfaceToEnc->dwPitch;
    params.tileType                      = m_rawSurfaceToEnc->TileType;
    params.tileModeGmm                   = m_rawSurfaceToEnc->TileModeGMM;
    params.format                        = m_rawSurfaceToEnc->Format;
    params.gmmTileEn                     = m_rawSurfaceToEnc->bGMMTileEnabled;
    params.uOffset                       = m_rawSurfaceToEnc->YoffsetForUplane;
    params.vOffset                       = m_rawSurfaceToEnc->YoffsetForVplane;
    params.displayFormatSwizzle          = m_av1SeqParams->SeqFlags.fields.DisplayFormatSwizzle;
    params.height                        = m_oriFrameHeight;
    params.width                         = m_oriFrameWidth;
    params.colorSpaceSelection           = (m_av1SeqParams->InputColorSpace == ECOLORSPACE_P709) ? true : false;
    params.chromaDownsampleFilterControl = 7;     // confirmed with arch, this should apply to all platforms

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, Av1BasicFeature)
{
    auto surface = &m_reconSurface;
    if (!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type))
    {
        surface = m_ref.GetEncRefSurface().front();
    }

    params.pitch       = surface->dwPitch;
    params.tileType    = surface->TileType;
    params.tileModeGmm = surface->TileModeGMM;
    params.format      = surface->Format;
    params.gmmTileEn   = surface->bGMMTileEnabled;
    params.uOffset     = surface->YoffsetForUplane;
    params.vOffset     = surface->YoffsetForVplane;
    params.height      = m_oriFrameHeight;
    params.width       = m_oriFrameWidth;

    // For 10 bit case, app may still allocates NV12 buffer but with doubled width, need to change it to P010
    if (m_is10Bit && params.format == Format_NV12)
    {
        params.format = Format_P010;
    }

    if (m_reconSurface.Format == Format_Y410 || m_reconSurface.Format == Format_444P || m_reconSurface.Format == Format_AYUV)
    {
        if (m_reconSurface.Format == Format_Y410)
        {
            params.pitch = m_reconSurface.dwPitch / 2;
        }
        else
        {
            params.pitch = m_reconSurface.dwPitch / 4;
        }
        params.uOffset = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
        params.vOffset = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8) << 1;
    }
    else if (m_reconSurface.Format == Format_Y216 || m_reconSurface.Format == Format_Y210 || m_reconSurface.Format == Format_YUY2)
    {
        params.uOffset = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
        params.vOffset = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, Av1BasicFeature)
{
    auto surface8x = m_8xDSSurface;
    if (!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type))
    {
        surface8x = m_ref.GetEnc8xRefSurface().front();
    }

    params.pitchStage1       = surface8x->dwPitch;
    params.tileTypeStage1    = surface8x->TileType;
    params.tileModeGmmStage1 = surface8x->TileModeGMM;
    params.gmmTileEnStage1   = surface8x->bGMMTileEnabled;
    params.uOffsetStage1     = surface8x->YoffsetForUplane;
    params.vOffsetStage1     = surface8x->YoffsetForVplane;
    params.heightStage1      = surface8x->dwHeight;
    params.widthStage1       = surface8x->dwWidth;

    auto surface4x = m_4xDSSurface;
    if (!AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type))
    {
        surface4x = m_ref.GetEnc4xRefSurface().front();
    }

    params.pitchStage2       = surface4x->dwPitch;
    params.tileTypeStage2    = surface4x->TileType;
    params.tileModeGmmStage2 = surface4x->TileModeGMM;
    params.gmmTileEnStage2   = surface4x->bGMMTileEnabled;
    params.uOffsetStage2     = surface4x->YoffsetForUplane;
    params.vOffsetStage2     = surface4x->YoffsetForVplane;
    params.heightStage2      = surface4x->dwHeight;
    params.widthStage2       = surface4x->dwWidth;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1BasicFeature)
{
#ifdef _MMC_SUPPORTED    
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    if (m_mmcState->IsMmcEnabled())
    {
        params.mmcEnabled = true;
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(m_rawSurfaceToEnc), &params.mmcStateRaw));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(const_cast<PMOS_SURFACE>(m_rawSurfaceToEnc), &params.compressionFormatRaw));
    }
    else
    {
        params.mmcEnabled           = false;
        params.mmcStateRaw          = MOS_MEMCOMP_DISABLED;
        params.compressionFormatRaw = GMM_FORMAT_INVALID;
    }
#endif

    params.surfaceRaw                    = m_rawSurfaceToEnc;
    params.surfaceDsStage1               = m_8xDSSurface;
    params.surfaceDsStage2               = m_4xDSSurface;
    params.pakObjCmdStreamOutBuffer      = m_resMbCodeBuffer;
    params.streamOutBuffer               = m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
    params.streamOutOffset               = 0;
    params.mfdIntraRowStoreScratchBuffer = m_resMfdIntraRowStoreScratchBuffer;

    // it's better to make m_ref and m_streamIn a feature
    m_ref.MHW_SETPAR_F(VDENC_PIPE_BUF_ADDR_STATE)(params);
    m_streamIn.MHW_SETPAR_F(VDENC_PIPE_BUF_ADDR_STATE)(params);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WEIGHTSOFFSETS_STATE, Av1BasicFeature)
{
    int8_t size = sizeof(params.weightsLuma) / sizeof(int8_t);
    memset(params.weightsLuma, 1, size);
    memset(params.offsetsLuma, 0, size);

    size = sizeof(params.weightsChroma) / sizeof(int8_t);
    memset(params.weightsChroma, 1, size);
    memset(params.offsetsChroma, 0, size);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD1, Av1BasicFeature)
{

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    for (const auto &lambda : setting->vdencCmd1Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, m_ref.IsLowDelay()));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1BasicFeature)
{
    params.tileWidth  = m_oriFrameWidth;
    params.tileHeight = m_oriFrameHeight;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1BasicFeature)
{
    ENCODE_FUNC_CALL();

    params.width       = m_oriFrameWidth;
    params.height      = m_oriFrameHeight;
    params.qpPrimeYDc  = (uint8_t)CodecHal_Clip3(0, 255, m_av1PicParams->base_qindex + m_av1PicParams->y_dc_delta_q);
    params.qpPrimeYAc  = (uint8_t)m_av1PicParams->base_qindex;
    params.tiling      = m_av1PicParams->tile_cols > 1 || m_av1PicParams->tile_rows > 1;
    params.temporalMvp = false;

    ENCODE_CHK_STATUS_RETURN(m_ref.MHW_SETPAR_F(VDENC_CMD2)(params));
    ENCODE_CHK_STATUS_RETURN(m_streamIn.MHW_SETPAR_F(VDENC_CMD2)(params));

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(setting);

    for (const auto& lambda : setting->vdencCmd2Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, m_ref.IsLowDelay()));
    }

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    ENCODE_CHK_NULL_RETURN(waTable);
    if (MEDIA_IS_WA(waTable, Wa_22011549751) &&
        !m_osInterface->bSimIsActive &&
        !Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext) &&
        m_av1PicParams->PicFlags.fields.frame_type == keyFrame)
    {
        params.pictureType = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1BasicFeature)
{
    // (1) from seq ddi params
    params.enableOrderHint     = m_av1SeqParams->CodingToolFlags.fields.enable_order_hint ? true : false;
    params.orderHintBitsMinus1 = m_av1SeqParams->order_hint_bits_minus_1;
    params.enableRestoration   = m_av1SeqParams->CodingToolFlags.fields.enable_restoration;

    // (2) from pic ddi params
    params.frameWidthMinus1     = m_av1PicParams->frame_width_minus1;
    params.frameHeightMinus1    = m_av1PicParams->frame_height_minus1;
    params.frameType            = m_av1PicParams->PicFlags.fields.frame_type;
    params.primaryRefFrame      = m_av1PicParams->primary_ref_frame;
    params.deltaQPresentFlag    = m_av1PicParams->dwModeControlFlags.fields.delta_q_present_flag ? true : false;
    params.log2DeltaQRes        = m_av1PicParams->dwModeControlFlags.fields.log2_delta_q_res;
    params.codedLossless        = IsFrameLossless(*m_av1PicParams);
    params.baseQindex           = static_cast<uint8_t>(m_av1PicParams->base_qindex);
    params.yDcDeltaQ            = m_av1PicParams->y_dc_delta_q;
    params.uDcDeltaQ            = m_av1PicParams->u_dc_delta_q;
    params.uAcDeltaQ            = m_av1PicParams->u_ac_delta_q;
    params.vDcDeltaQ            = m_av1PicParams->v_dc_delta_q;
    params.vAcDeltaQ            = m_av1PicParams->v_ac_delta_q;
    params.allowHighPrecisionMV = m_av1PicParams->PicFlags.fields.allow_high_precision_mv ? true : false;
    params.referenceSelect      = m_av1PicParams->dwModeControlFlags.fields.reference_mode == referenceModeSelect;
    params.interpFilter         = m_av1PicParams->interp_filter;
    params.currentOrderHint     = m_av1PicParams->order_hint;
    params.reducedTxSetUsed     = m_av1PicParams->PicFlags.fields.reduced_tx_set_used ? true : false;
    params.txMode               = m_av1PicParams->dwModeControlFlags.fields.tx_mode;
    params.skipModePresent      = m_av1PicParams->dwModeControlFlags.fields.skip_mode_present ? true : false;
    params.enableCDEF           = m_enableCDEF;
    
    for (uint8_t i = 0; i < 7; i++)
        params.globalMotionType[i] = static_cast<uint8_t>(m_av1PicParams->wm[i].wmtype);

    params.frameLevelGlobalMotionInvalidFlags =
        (m_av1PicParams->wm[0].invalid << 1) |
        (m_av1PicParams->wm[1].invalid << 2) |
        (m_av1PicParams->wm[2].invalid << 3) |
        (m_av1PicParams->wm[3].invalid << 4) |
        (m_av1PicParams->wm[4].invalid << 5) |
        (m_av1PicParams->wm[5].invalid << 6) |
        (m_av1PicParams->wm[6].invalid << 7);

    for (uint8_t i = 0; i < 8; i++)
        params.refFrameIdx[i] = m_av1PicParams->RefFrameList[i].FrameIdx == 0xff ? 0 : m_av1PicParams->RefFrameList[i].FrameIdx;

    // (3) from other sources
    params.skipModeFrame[0] = 0;
    params.skipModeFrame[1] = 0;

    params.bitDepthIdc  = (m_bitDepth - 8) >> 1;
    params.chromaFormat = m_chromaFormat;

    params.vdencPackOnlyPass = false;

    params.frameBitRateMaxReportMask = false;
    params.frameBitRateMinReportMask = false;
    params.frameBitRateMaxUnit       = 0;
    params.frameBitRateMax           = 0;
    params.frameBitRateMinUnit       = 0;
    params.frameBitRateMin           = 0;

    params.frameDeltaQindexMax[0]        = 0;
    params.frameDeltaQindexMax[1]        = 0;
    params.frameDeltaQindexMin           = 0;
    params.frameDeltaLFMax[0]            = 0;
    params.frameDeltaLFMax[1]            = 0;
    params.frameDeltaLFMin               = 0;
    params.frameDeltaQindexLFMaxRange[0] = 0;
    params.frameDeltaQindexLFMaxRange[1] = 0;
    params.frameDeltaQindexLFMinRange    = 0;

    const auto minFrameBytes = 512 / 8 + m_appHdrSize + m_tileGroupHeaderSize;
    params.minFramSizeUnits = 3;
    params.minFramSize      = MOS_ALIGN_CEIL(minFrameBytes, 16) / 16;

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    if (MEDIA_IS_WA(waTable, Wa_15013355402))
    {
        params.minFramSize = MOS_ALIGN_CEIL(13 * 64, 16) / 16;
    }

    params.bitOffsetForFirstPartitionSize = 0;

    params.class0_SSE_Threshold0 = 0;
    params.class0_SSE_Threshold1 = 0;

    params.sbMaxSizeReportMask = false;
    params.sbMaxBitSizeAllowed = 0;

    params.autoBistreamStitchingInHardware = !m_enableSWStitching && !m_dualEncEnable;

    // special fix to avoid zero padding for low resolution/bitrates and restore up to 20% BdRate quality
    if ((m_av1PicParams->tile_cols * m_av1PicParams->tile_rows == 1) || m_dualEncEnable || m_enableSWStitching)
    {
        params.minFramSize = 0;
        params.minFramSizeUnits                = 0;
        params.autoBistreamStitchingInHardware = false;
    }

    MHW_CHK_STATUS_RETURN(m_ref.MHW_SETPAR_F(AVP_PIC_STATE)(params));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_INLOOP_FILTER_STATE, Av1BasicFeature)
{
    params.loopFilterLevel[0] = m_av1PicParams->filter_level[0];
    params.loopFilterLevel[1] = m_av1PicParams->filter_level[1];
    params.loopFilterLevel[2] = m_av1PicParams->filter_level_u;
    params.loopFilterLevel[3] = m_av1PicParams->filter_level_v;

    params.loopFilterSharpness    = m_av1PicParams->cLoopFilterInfoFlags.fields.sharpness_level;
    params.loopFilterDeltaEnabled = m_av1PicParams->cLoopFilterInfoFlags.fields.mode_ref_delta_enabled ? true : false;
    params.deltaLfRes             = m_av1PicParams->dwModeControlFlags.fields.log2_delta_lf_res;
    params.deltaLfMulti           = m_av1PicParams->dwModeControlFlags.fields.delta_lf_multi;
    params.loopFilterDeltaUpdate  = m_av1PicParams->dwModeControlFlags.fields.delta_lf_present_flag ? true : false;

    for (uint8_t i = 0; i < AV1_NUM_OF_REF_LF_DELTAS; i++)
        params.loopFilterRefDeltas[i] = m_av1PicParams->ref_deltas[i];

    for (uint8_t i = 0; i < AV1_NUM_OF_MODE_LF_DELTAS; i++)
        params.loopFilterModeDeltas[i] = m_av1PicParams->mode_deltas[i];

    for (uint8_t i = 0; i < 8; i++)
    {
        params.cdefYStrength[i]  = m_av1PicParams->cdef_y_strengths[i];
        params.cdefUVStrength[i] = m_av1PicParams->cdef_uv_strengths[i];
    }

    params.cdefBits = m_av1PicParams->cdef_bits;
    params.cdefDampingMinus3 = m_av1PicParams->cdef_damping_minus_3;

    params.LoopRestorationType[0]              = m_av1PicParams->LoopRestorationFlags.fields.yframe_restoration_type;
    params.LoopRestorationType[1]              = m_av1PicParams->LoopRestorationFlags.fields.cbframe_restoration_type;
    params.LoopRestorationType[2]              = m_av1PicParams->LoopRestorationFlags.fields.crframe_restoration_type;
    params.UseSameLoopRestorationSizeForChroma = m_av1PicParams->LoopRestorationFlags.fields.lr_uv_shift ? false: true;

    if (params.LoopRestorationType[0] == 0 &&
        params.LoopRestorationType[1] == 0 &&
        params.LoopRestorationType[2] == 0)
    {
        params.LoopRestorationSizeLuma             = 0;
        params.UseSameLoopRestorationSizeForChroma = false;
    }
    else
    {
        // Intel Encoder PAK only support LRU size = SuperBlock size = 64x64 pixels.
        params.LoopRestorationSizeLuma = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIPE_BUF_ADDR_STATE, Av1BasicFeature)
{
    params.bsLineRowstoreBuffer            = m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer;
    params.intraPredLineRowstoreBuffer     = m_resMfdIntraRowStoreScratchBuffer;
    params.intraPredTileLineRowstoreBuffer = m_intraPredictionTileLineRowstoreReadWriteBuffer;
    params.spatialMVLineBuffer             = m_spatialMotionVectorLineReadWriteBuffer;
    params.spatialMVCodingTileLineBuffer   = m_spatialMotionVectorCodingTileLineReadWriteBuffer;
    params.lrMetaTileColumnBuffer          = m_loopRestorationMetaTileColumnReadWriteBuffer;
    params.lrTileLineYBuffer               = m_loopRestorationFilterTileReadWriteLineYBuffer;
    params.lrTileLineUBuffer               = m_loopRestorationFilterTileReadWriteLineUBuffer;
    params.lrTileLineVBuffer               = m_loopRestorationFilterTileReadWriteLineVBuffer;
    params.lrTileColumnYBuffer             = m_loopRestorationFilterTileColumnReadWriteYBuffer;
    params.lrTileColumnUBuffer             = m_loopRestorationFilterTileColumnReadWriteUBuffer;
    params.lrTileColumnVBuffer             = m_loopRestorationFilterTileColumnReadWriteVBuffer;
    params.lrTileColumnAlignBuffer         = m_loopRestorationFilterTileColumnAlignmentBuf;
    params.deblockLineYBuffer              = m_deblockerFilterLineReadWriteYBuffer;
    params.deblockLineUBuffer              = m_deblockerFilterLineReadWriteUBuffer;
    params.deblockLineVBuffer              = m_deblockerFilterLineReadWriteVBuffer;
    params.superResTileColumnYBuffer       = m_superResTileColumnReadWriteYBuffer;
    params.superResTileColumnUBuffer       = m_superResTileColumnReadWriteUBuffer;
    params.superResTileColumnVBuffer       = m_superResTileColumnReadWriteVBuffer;
    params.decodedFrameStatusErrorBuffer   = m_decodedFrameStatusErrorBuffer;
    params.decodedBlockDataStreamoutBuffer = m_decodedBlockDataStreamoutBuffer;

    params.tileStatisticsPakStreamoutBuffer = m_tileStatisticsPakStreamoutBuffer;
    params.cuStreamoutBuffer                = m_cuStreamoutBuffer;
    params.sseLineBuffer                    = m_sseLineReadWriteBuffer;
    params.sseTileLineBuffer                = m_sseTileLineReadWriteBuffer;

    CODEC_REF_LIST currRefList     = *m_ref.GetCurrRefList();
    MOS_SURFACE *  postCdefSurface = m_trackedBuf->GetSurface(
        BufferType::postCdefReconSurface, currRefList.ucScalingIdx);
    params.postCDEFpixelsBuffer = postCdefSurface;

    // code from SetAvpPipeBufAddr
 #ifdef _MMC_SUPPORTED    
    ENCODE_CHK_NULL_RETURN(m_mmcState);
    if (m_mmcState->IsMmcEnabled())
    {
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_reconSurface), &params.mmcStatePreDeblock));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_rawSurface), &params.mmcStateRawSurf));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(postCdefSurface, &params.postCdefSurfMmcState));
    }
    else
    {
        params.mmcStatePreDeblock    = MOS_MEMCOMP_DISABLED;
        params.mmcStateRawSurf       = MOS_MEMCOMP_DISABLED;
        params.postCdefSurfMmcState  = MOS_MEMCOMP_DISABLED;
    }
#endif

    params.decodedPic              = const_cast<PMOS_SURFACE>(&m_reconSurface);
    params.decodedPic->MmcState    = params.mmcStatePreDeblock;  // This is for MMC report only
    params.curMvTempBuffer         = m_resMvTemporalBuffer;
    params.originalPicSourceBuffer = const_cast<PMOS_RESOURCE>(&m_rawSurfaceToPak->OsResource);
    params.dsPictureSourceBuffer   = const_cast<PMOS_RESOURCE>(&m_rawSurfaceToEnc->OsResource);

    MHW_CHK_STATUS_RETURN(m_ref.MHW_SETPAR_F(AVP_PIPE_BUF_ADDR_STATE)(params));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_INTER_PRED_STATE, Av1BasicFeature)
{
    MHW_CHK_STATUS_RETURN(m_ref.MHW_SETPAR_F(AVP_INTER_PRED_STATE)(params));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_IND_OBJ_BASE_ADDR_STATE, Av1BasicFeature)
{
    params.Mode                = codechalEncodeModeAv1;
    params.mvObjectBuffer      = m_resMbCodeBuffer;
    params.pakBaseObjectBuffer = const_cast<PMOS_RESOURCE>(&m_resBitstreamBuffer);
    params.pakBaseObjectSize   = m_bitstreamSize;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1BasicFeature)
{
    MOS_MEMCOMP_STATE mmcState = {};
    switch (params.surfaceStateId)
    {
    case srcInputPic:
        params.pitch   = m_rawSurfaceToEnc->dwPitch;
        params.uOffset = m_rawSurfaceToEnc->YoffsetForUplane;
        params.vOffset = m_rawSurfaceToEnc->YoffsetForVplane;
        GetSurfaceMmcInfo(m_rawSurfaceToEnc, mmcState, params.compressionFormat);
        std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
        break;
    case origUpscaledSrc:
        params.pitch   = m_rawSurfaceToPak->dwPitch;
        params.uOffset = m_rawSurfaceToPak->YoffsetForUplane;
        params.vOffset = m_rawSurfaceToPak->YoffsetForVplane;
        GetSurfaceMmcInfo(m_rawSurfaceToPak, mmcState, params.compressionFormat);
        std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
        break;
    case reconPic:
        params.pitch   = m_reconSurface.dwPitch;
        params.uOffset = m_reconSurface.YoffsetForUplane;
        params.vOffset = m_reconSurface.YoffsetForVplane;
        GetSurfaceMmcInfo(const_cast<PMOS_SURFACE>(&m_reconSurface), mmcState, params.compressionFormat);
        std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
        break;
    case av1CdefPixelsStreamout:
    {
        CODEC_REF_LIST_AV1 currRefList     = *m_ref.GetCurrRefList();
        MOS_SURFACE *      postCdefSurface = m_trackedBuf->GetSurface(
            BufferType::postCdefReconSurface, currRefList.ucScalingIdx);
        MHW_CHK_NULL_RETURN(postCdefSurface);
        params.pitch   = postCdefSurface->dwPitch;
        params.uOffset = postCdefSurface->YoffsetForUplane;
        params.vOffset = postCdefSurface->YoffsetForVplane;
        GetSurfaceMmcInfo(postCdefSurface, mmcState, params.compressionFormat);
        std::fill(std::begin(params.mmcState), std::end(params.mmcState), mmcState);
        break;
    }
    case av1IntraFrame:
    case av1LastRef:
    case av1Last2Ref:
    case av1Last3Ref:
    case av1GoldRef:
    case av1BwdRef:
    case av1AltRef2:
    case av1AltRef:
        params.uvPlaneAlignment = 8;
        MHW_CHK_STATUS_RETURN(m_ref.MHW_SETPAR_F(AVP_SURFACE_STATE)(params));
        break;
    }

    params.bitDepthLumaMinus8 = m_is10Bit ? 2 : 0;

    return MOS_STATUS_SUCCESS;
}

enum CODEC_SELECT
{
    CODEC_SELECT_DECODE = 0,
    CODEC_SELECT_ENCODE = 1,
};

enum CODEC_STANDARD_SELECT
{
    CODEC_STANDARD_SELECT_HEVC = 0,
    CODEC_STANDARD_SELECT_VP9  = 1,
    CODEC_STANDARD_SELECT_AV1  = 2,
};

MHW_SETPAR_DECL_SRC(AVP_PIPE_MODE_SELECT, Av1BasicFeature)
{
    params.codecSelect         = CODEC_SELECT_ENCODE;
    params.codecStandardSelect = CODEC_STANDARD_SELECT_AV1;
    params.vdencMode           = true;

    params.cdefOutputStreamoutEnableFlag = false;
    params.lrOutputStreamoutEnableFlag   = false;

    params.frameReconDisable = m_av1PicParams->PicFlags.fields.DisableFrameRecon;
    params.tileStatsStreamoutEnable      = IsRateControlBrc(m_av1SeqParams->RateControlMethod);
    params.pakFrameLevelStreamOutEnable  = IsRateControlBrc(m_av1SeqParams->RateControlMethod);

    params.picStatusErrorReportEnable    = false;
    params.picStatusErrorReportId        = false;

    params.phaseIndicator = 0;

    params.frameReconDisable = false;

    params.motionCompMemoryTrackerCntEnable  = false;
    params.motionCompMemTrackerCounterEnable = false;

    params.srcPixelPrefetchEnable = true;
    params.srcPixelPrefetchLen    = 4;
    params.sseEnable              = false;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_TILE_CODING, Av1BasicFeature)
{
    params.tileRowIndependentFlag  = false;
    params.disableCdfUpdateFlag    = m_av1PicParams->PicFlags.fields.disable_cdf_update ? 1 : 0;
    params.numOfActiveBePipes      = 1;
    params.numOfTileColumnsInFrame = m_av1PicParams->tile_cols;
    params.numOfTileRowsInFrame    = m_av1PicParams->tile_rows;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_PIPE_MODE_SELECT, Av1BasicFeature)
{
    params.mode = m_mode;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(MFX_WAIT, Av1BasicFeature)
{
    params.iStallVdboxPipeline = true;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
