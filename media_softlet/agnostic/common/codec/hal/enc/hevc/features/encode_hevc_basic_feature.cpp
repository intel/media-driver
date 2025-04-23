/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     encode_hevc_basic_feature.cpp
//! \brief    Defines the common interface for encode hevc parameter
//!

#include "encode_hevc_basic_feature.h"
#include "encode_utils.h"
#include "encode_allocator.h"
#include "encode_hevc_header_packer.h"
#include "encode_hevc_vdenc_const_settings.h"
#include "mos_solo_generic.h"
#include "mos_os_cp_interface_specific.h"

using namespace mhw::vdbox;
namespace encode
{
HevcBasicFeature::~HevcBasicFeature()
{
    if (m_422State)
    {
        MOS_Delete(m_422State);
        m_422State = nullptr;
    }
}

MOS_STATUS HevcBasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    EncodeBasicFeature::Init(setting);

    // It is assumed to be frame-mode always
    m_frameFieldHeight     = m_frameHeight;
    m_frameFieldHeightInMb = m_picHeightInMb;
    m_picWidthInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_LCU_SIZE);        //assume smallest LCU to get max width;
    m_picHeightInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameHeight, CODECHAL_HEVC_MIN_LCU_SIZE);      //assume smallest LCU to get max height
    m_widthAlignedMaxLCU  = MOS_ALIGN_CEIL(m_frameWidth, m_maxLCUSize);
    m_heightAlignedMaxLCU = MOS_ALIGN_CEIL(m_frameHeight, m_maxLCUSize);
    m_sizeOfSseSrcPixelRowStoreBufferPerLcu = CODECHAL_CACHELINE_SIZE * (4 + 4) << 1;

    //cannot remove as it used by reference frame
    m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE) *
        CODECHAL_GET_HEIGHT_IN_BLOCKS(m_frameHeight, CODECHAL_HEVC_MIN_TILE_SIZE);

    MOS_ALLOC_GFXRES_PARAMS allocParams{};
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    allocParams.dwBytes  = MOS_ALIGN_CEIL(m_sizeOfHcpPakFrameStats * m_maxTileNumber, CODECHAL_PAGE_SIZE);
    allocParams.pBufName = "FrameStatStreamOutBuffer";
    allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
    m_recycleBuf->RegisterResource(FrameStatStreamOutBuffer, allocParams, 1);

    allocParams.dwBytes  = MOS_ALIGN_CEIL(1216 * m_maxTileNumber, CODECHAL_PAGE_SIZE);
    allocParams.pBufName = "vdencStats";
    allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
    m_recycleBuf->RegisterResource(VdencStatsBuffer, allocParams, 1);

    uint32_t numOfLCU    = MOS_ROUNDUP_DIVIDE(m_frameWidth, m_maxLCUSize) * (MOS_ROUNDUP_DIVIDE(m_frameHeight, m_maxLCUSize) + 1);
    allocParams.dwBytes  = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * (numOfLCU * 5 + numOfLCU * 64 * 8), CODECHAL_PAGE_SIZE);
    allocParams.pBufName = "CuRecordStreamOutBuffer";
    allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_WRITE;
    ENCODE_NORMALMESSAGE("osCpInterface = %p\n", m_osInterface->osCpInterface);
    if (m_osInterface->osCpInterface != nullptr)
    {
        ENCODE_NORMALMESSAGE("IsCpEnabled = %u\n", m_osInterface->osCpInterface->IsCpEnabled());
    }
    if (m_osInterface->osCpInterface == nullptr || !m_osInterface->osCpInterface->IsCpEnabled())
    {
        allocParams.dwMemType = MOS_MEMPOOL_SYSTEMMEMORY;
    }
    allocParams.Flags.bCacheable = true;
    m_recycleBuf->RegisterResource(CuRecordStreamOutBuffer, allocParams, m_maxSyncDepth);

    ENCODE_CHK_STATUS_RETURN(m_ref.Init(this, m_allocator));

    MediaUserSetting::Value outValue;
#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        m_userSettingPtr,
        outValue,
        "Disable HEVC RDOQ Perf",
        MediaUserSetting::Group::Sequence);
#endif  // _DEBUG || _RELEASE_INTERNAL
    m_hevcRDOQPerfDisabled = outValue.Get<bool>();

    ENCODE_CHK_STATUS_RETURN(Init422State());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Update(params));

    EncoderParams* encodeParams = (EncoderParams*)params;

    m_hevcSeqParams = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
    m_hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(m_hevcPicParams);
    m_hevcSliceParams = static_cast<PCODEC_HEVC_ENCODE_SLICE_PARAMS>(encodeParams->pSliceParams);
    ENCODE_CHK_NULL_RETURN(m_hevcSliceParams);
    m_hevcIqMatrixParams = static_cast<PCODECHAL_HEVC_IQ_MATRIX_PARAMS>(encodeParams->pIQMatrixBuffer);
    ENCODE_CHK_NULL_RETURN(m_hevcIqMatrixParams);
    m_nalUnitParams = encodeParams->ppNALUnitParams;
    ENCODE_CHK_NULL_RETURN(m_nalUnitParams);
    m_NumNalUnits   = encodeParams->uiNumNalUnits;
    m_bEnableSubPelMode = encodeParams->bEnableSubPelMode;
    m_SubPelMode        = encodeParams->SubPelMode;

    if (m_422State && m_422State->GetFeature422Flag())
    {
        ENCODE_CHK_STATUS_RETURN(m_422State->Update422Format(m_hevcSeqParams, m_outputChromaFormat, m_reconSurface.Format, m_is10Bit));
    }

    if (encodeParams->bAcceleratorHeaderPackingCaps)
    {
        HevcHeaderPacker Packer;
        Packer.SliceHeaderPacker(encodeParams);  //after here, slice parameters will be modified; here is th elast place to deliver encodeParams
    }

    ENCODE_CHK_STATUS_RETURN(SetPictureStructs());
    ENCODE_CHK_STATUS_RETURN(SetSliceStructs());

#if (_DEBUG || _RELEASE_INTERNAL)
    // To enable rounding precision here
    MediaUserSetting::Value outValue;
    ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "HEVC VDEnc Rounding Enable",
        MediaUserSetting::Group::Sequence);
    m_hevcVdencRoundingPrecisionEnabled = outValue.Get<bool>();
    ReportUserSettingForDebug(
        m_userSettingPtr,
        "HEVC VDEnc Rounding Enable",
        m_hevcVdencRoundingPrecisionEnabled,
        MediaUserSetting::Group::Sequence);
#endif

    ENCODE_CHK_STATUS_RETURN(SetRoundingValues());

    uint32_t frameWidth = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);

    uint32_t frameHeight = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);

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
        ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
    }

    ENCODE_CHK_STATUS_RETURN(GetTrackedBuffers());
    ENCODE_CHK_STATUS_RETURN(GetRecycleBuffers());

    if (m_hevcSeqParams->LowDelayMode)
    {
        m_lambdaType = 1;
        m_qpFactors  = {0.578, 0.3524, 0.3524};
    }
    else
    {
        m_lambdaType = 2;
        m_qpFactors  = {0.442, 0.3536, 0.3536, 0.68}; // seems not used;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::CalcLCUMaxCodingSize()
{
    ENCODE_FUNC_CALL();

    uint16_t log2_max_coding_block_size = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
    uint32_t rawCTUBits = (1 << (2 * log2_max_coding_block_size));

    switch (m_hevcSeqParams->chroma_format_idc)
    {
        // 420
    case 1:
        rawCTUBits = rawCTUBits * 3 / 2;
        break;
        // 422
    case 2:
        rawCTUBits = rawCTUBits * 2;
        break;
        // 444
    case 3:
        rawCTUBits = rawCTUBits * 3;
        break;
    default:
        break;
    };

    rawCTUBits = rawCTUBits * (m_hevcSeqParams->bit_depth_luma_minus8 + 8);
    rawCTUBits = (5 * rawCTUBits / 3);

    if (m_hevcPicParams->LcuMaxBitsizeAllowed == 0 || m_hevcPicParams->LcuMaxBitsizeAllowed > rawCTUBits)
    {
        m_hevcPicParams->LcuMaxBitsizeAllowed = rawCTUBits;
    }

    return MOS_STATUS_SUCCESS;
}

void HevcBasicFeature::CreateFlatScalingList()
{
    ENCODE_FUNC_CALL();

    for (auto i = 0; i < 6; i++)
    {
        memset(&(m_hevcIqMatrixParams->ucScalingLists0[i][0]),
            0x10,
            sizeof(m_hevcIqMatrixParams->ucScalingLists0[i]));

        memset(&(m_hevcIqMatrixParams->ucScalingLists1[i][0]),
            0x10,
            sizeof(m_hevcIqMatrixParams->ucScalingLists1[i]));

        memset(&(m_hevcIqMatrixParams->ucScalingLists2[i][0]),
            0x10,
            sizeof(m_hevcIqMatrixParams->ucScalingLists2[i]));
    }

    memset(&(m_hevcIqMatrixParams->ucScalingLists3[0][0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[0]));

    memset(&(m_hevcIqMatrixParams->ucScalingLists3[1][0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[1]));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3));
}

void HevcBasicFeature::CreateDefaultScalingList()
{
    ENCODE_FUNC_CALL();

    const uint8_t flatScalingList4x4[16] =
    {
        16,16,16,16,
        16,16,16,16,
        16,16,16,16,
        16,16,16,16
    };

    const uint8_t defaultScalingList8x8[2][64] =
    {
        {
            16,16,16,16,17,18,21,24,
            16,16,16,16,17,19,22,25,
            16,16,17,18,20,22,25,29,
            16,16,18,21,24,27,31,36,
            17,17,20,24,30,35,41,47,
            18,19,22,27,35,44,54,65,
            21,22,25,31,41,54,70,88,
            24,25,29,36,47,65,88,115
        },
        {
            16,16,16,16,17,18,20,24,
            16,16,16,17,18,20,24,25,
            16,16,17,18,20,24,25,28,
            16,17,18,20,24,25,28,33,
            17,18,20,24,25,28,33,41,
            18,20,24,25,28,33,41,54,
            20,24,25,28,33,41,54,71,
            24,25,28,33,41,54,71,91
        }
    };

    for (auto i = 0; i < 6; i++)
    {
        memcpy(&(m_hevcIqMatrixParams->ucScalingLists0[i][0]),
            flatScalingList4x4,
            sizeof(m_hevcIqMatrixParams->ucScalingLists0[i]));
    }

    for (auto i = 0; i < 3; i++)
    {
        memcpy(&(m_hevcIqMatrixParams->ucScalingLists1[i][0]),
            defaultScalingList8x8[0],
            sizeof(m_hevcIqMatrixParams->ucScalingLists1[i]));

        memcpy(&(m_hevcIqMatrixParams->ucScalingLists1[3 + i][0]),
            defaultScalingList8x8[1],
            sizeof(m_hevcIqMatrixParams->ucScalingLists1[3 + i]));

        memcpy(&(m_hevcIqMatrixParams->ucScalingLists2[i][0]),
            defaultScalingList8x8[0],
            sizeof(m_hevcIqMatrixParams->ucScalingLists2[i]));

        memcpy(&(m_hevcIqMatrixParams->ucScalingLists2[3 + i][0]),
            defaultScalingList8x8[1],
            sizeof(m_hevcIqMatrixParams->ucScalingLists2[3 + i]));
    }

    memcpy(&(m_hevcIqMatrixParams->ucScalingLists3[0][0]),
        defaultScalingList8x8[0],
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[0]));

    memcpy(&(m_hevcIqMatrixParams->ucScalingLists3[1][0]),
        defaultScalingList8x8[1],
        sizeof(m_hevcIqMatrixParams->ucScalingLists3[1]));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID2));

    memset(&(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3[0]),
        0x10,
        sizeof(m_hevcIqMatrixParams->ucScalingListDCCoefSizeID3));
}


MOS_STATUS HevcBasicFeature::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    m_targetUsage                = m_hevcSeqParams->TargetUsage;
    m_lastPicInSeq               = m_hevcPicParams->bLastPicInSeq;
    m_lastPicInStream            = m_hevcPicParams->bLastPicInStream;
    m_currOriginalPic            = m_hevcPicParams->CurrOriginalPic;
    m_currReconstructedPic       = m_hevcPicParams->CurrReconstructedPic;

    ENCODE_CHK_STATUS_RETURN(m_ref.UpdatePicture());
    m_pictureCodingType = m_ref.GetPictureCodingType();

    if (m_hevcPicParams->QpY > m_maxSliceQP)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) &&
        (!m_hevcPicParams->bUseRawPicForRef || m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_hevcSeqParams->scaling_list_enable_flag && !m_hevcPicParams->scaling_list_data_present_flag)
    {
        CreateDefaultScalingList();
    }
    else if (!m_hevcSeqParams->scaling_list_enable_flag)
    {
        CreateFlatScalingList();
    }

    ENCODE_CHK_STATUS_RETURN(CalcLCUMaxCodingSize());

    // EOS is not working on GEN12, disable it by setting below to false (WA), only needed by HEVC VDENC
    m_lastPicInSeq = false;
    m_lastPicInStream = false;

    return eStatus;
}

MOS_STATUS HevcBasicFeature::SetSliceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(m_ref.UpdateSlice());

    if(m_hevcPicParams->bEnableRollingIntraRefresh != 0)
    {
        ENCODE_CHK_STATUS_RETURN(m_ref.UpdateRollingIReferenceLocation());
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::UpdateTrackedBufferParameters()
{
    // The MB/MV code size here, are just for HEVC VDENC, maybe different in Dual Pipe(VME) cases
    uint32_t numOfLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, m_maxLCUSize) * (MOS_ROUNDUP_DIVIDE(m_frameHeight, m_maxLCUSize) + 1);

    m_mbCodeSize = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * (numOfLCU * 5 + numOfLCU * 64 * 8), CODECHAL_PAGE_SIZE);

    m_mvDataSize = 0;

    uint32_t mvt_size        = MOS_ALIGN_CEIL(((m_frameWidth + 63) >> 6) * ((m_frameHeight + 15) >> 4), 2) * CODECHAL_CACHELINE_SIZE;
    uint32_t mvtb_size       = MOS_ALIGN_CEIL(((m_frameWidth + 31) >> 5) * ((m_frameHeight + 31) >> 5), 2) * CODECHAL_CACHELINE_SIZE;
    m_sizeOfMvTemporalBuffer = MOS_MAX(mvt_size, mvtb_size);

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
    allocParams.Type     = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format   = Format_Buffer;

    if (m_sizeOfMvTemporalBuffer > 0)
    {
        allocParams.dwBytes  = m_sizeOfMvTemporalBuffer;
        allocParams.pBufName = "mvTemporalBuffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::mvTemporalBuffer, allocParams));
    }

    if (m_422State && m_422State->GetFeature422Flag())
    {
        ENCODE_CHK_STATUS_RETURN(m_422State->RegisterMbCodeBuffer(m_trackedBuf, m_isMbCodeRegistered, m_mbCodeSize));
    }

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::UpdateTrackedBufferParameters());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::GetMaxMBPS(uint32_t levelIdc, uint32_t* maxMBPS, uint64_t* maxBytePerPic)
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_CHK_NULL_RETURN(maxMBPS);
    ENCODE_CHK_NULL_RETURN(maxBytePerPic);

    switch (levelIdc)
    {
    case 30:
        *maxMBPS = 552960;
        *maxBytePerPic = 36864;
        break;
    case 60:
        *maxMBPS = 3686400;
        *maxBytePerPic = 122880;
        break;
    case 63:
        *maxMBPS = 7372800;
        *maxBytePerPic = 245760;
        break;
    case 90:
        *maxMBPS = 16588800;
        *maxBytePerPic = 552760;
        break;
    case 93:
        *maxMBPS = 33177600;
        *maxBytePerPic = 983040;
        break;
    case 120:
        *maxMBPS = 66846720;
        *maxBytePerPic = 2228224;
        break;
    case 123:
        *maxMBPS = 133693440;
        *maxBytePerPic = 2228224;
        break;
    case 150:
        *maxMBPS = 267386880;
        *maxBytePerPic = 8912896;
        break;
    case 153:
        *maxMBPS = 534773760;
        *maxBytePerPic = 8912896;
        break;
    case 156:
        *maxMBPS = 1069547520;
        *maxBytePerPic = 8912896;
        break;
    case 180:
        *maxMBPS = 1069547520;
        *maxBytePerPic = 35651584;
        break;
    case 183:
        *maxMBPS = 2139095040;
        *maxBytePerPic = 35651584;
        break;
    case 186:
        *maxMBPS = 4278190080;
        *maxBytePerPic = 35651584;
        break;
    default:
        *maxMBPS = 16588800;
        *maxBytePerPic = 552760; // CModel defaults to level 3.0 value if not found,
                                 // we can do the same, just output that the issue exists and continue
        ENCODE_ASSERTMESSAGE("Unsupported LevelIDC setting for HEVC");
        break;
    }

    return eStatus;
}

uint32_t HevcBasicFeature::GetProfileLevelMaxFrameSize()
{
    ENCODE_FUNC_CALL();

    uint8_t         minCR = 2;
    float_t         formatFactor = 1.5;
    float_t         fminCrScale = 1.0;
    int32_t         levelIdc = m_hevcSeqParams->Level * 3;

    if (levelIdc == 186 || levelIdc == 150)
    {
        minCR = 6;
    }
    else if (levelIdc >150)
    {
        minCR = 8;
    }
    else if (levelIdc >93)
    {
        minCR = 4;
    }

    if (m_hevcSeqParams->chroma_format_idc == 0)
    {
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 0)
            formatFactor = 1.0;
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 8)
            formatFactor = 2.0;
    }
    else if (m_hevcSeqParams->chroma_format_idc == 1)
    {
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
        {
            formatFactor = 1.875;
        }
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 4)
        {
            formatFactor = 2.25;
        }
    }
    else if (m_hevcSeqParams->chroma_format_idc == 2)
    {
        fminCrScale = 0.5;
        if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
        {
            formatFactor = 2.5;
        }
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 4)
        {
            formatFactor = 3.0;
        }
    }
    else
    {
        fminCrScale = 0.5;
        formatFactor = 3.0;

        if (m_hevcSeqParams->bit_depth_luma_minus8 == 2)
        {
            formatFactor = 3.75;
        }
        else if (m_hevcSeqParams->bit_depth_luma_minus8 == 4)
        {
            formatFactor = 4.5;
        }
    }

    fminCrScale *= minCR;
    formatFactor /= fminCrScale;

    uint32_t        maxMBPS = 0;
    uint64_t        maxBytePerPic = 0;
    GetMaxMBPS(levelIdc, &maxMBPS, &maxBytePerPic);
    auto     maxBytePerPicNot0    = (uint64_t)((((float_t)maxMBPS * (float_t)m_hevcSeqParams->FrameRate.Denominator) / (float_t)m_hevcSeqParams->FrameRate.Numerator) * formatFactor);
    uint32_t profileLevelMaxFrame = 0;

    profileLevelMaxFrame = (uint32_t)MOS_MIN(maxBytePerPicNot0, maxBytePerPic);
    profileLevelMaxFrame = (uint32_t)MOS_MIN((m_frameHeight * m_frameWidth), profileLevelMaxFrame);

    return profileLevelMaxFrame;
}

MOS_STATUS HevcBasicFeature::GetTrackedBuffers()
{
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);
    ENCODE_CHK_NULL_RETURN(m_hevcPicParams);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    auto currRefList = m_ref.GetCurrRefList();
    m_trackedBuf->Acquire(currRefList, false);

    m_resMbCodeBuffer = m_trackedBuf->GetBuffer(BufferType::mbCodedBuffer, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_resMbCodeBuffer);

    m_resMvTemporalBuffer = m_trackedBuf->GetBuffer(BufferType::mvTemporalBuffer, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_resMvTemporalBuffer);

    m_4xDSSurface = m_trackedBuf->GetSurface(BufferType::ds4xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_4xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_4xDSSurface));

    m_8xDSSurface = m_trackedBuf->GetSurface(BufferType::ds8xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_8xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_8xDSSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::GetRecycleBuffers()
{
    ENCODE_CHK_NULL_RETURN(m_recycleBuf);

    uint32_t recycleBufferIdx = -1;
    for (uint32_t i = 0; i < m_maxSyncDepth; i++)
    {
        auto pos = find(m_recycleBufferIdxes.begin(), m_recycleBufferIdxes.end(), i);
        if (pos == m_recycleBufferIdxes.end())
        {
            recycleBufferIdx = i;
            break;
        }
    }

    if (recycleBufferIdx == -1)
    {
        return MOS_STATUS_SUCCESS;
    }

    m_resMbCodeBuffer = m_recycleBuf->GetBuffer(RecycleResId::CuRecordStreamOutBuffer, recycleBufferIdx);
    ENCODE_CHK_NULL_RETURN(m_resMbCodeBuffer);

    m_recycleBufferIdxes.push_back(recycleBufferIdx);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::SetRoundingValues()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra)
    {
        m_roundingIntra = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetIntra;
    }
    else
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            m_roundingIntra = 10;
        }
        else if (m_hevcSeqParams->HierarchicalFlag && m_hevcPicParams->HierarchLevelPlus1 > 0)
        {
            //Hierachical GOP
            if (m_hevcPicParams->HierarchLevelPlus1 == 1)
            {
                m_roundingIntra = 10;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
            {
                m_roundingIntra = 9;
            }
            else
            {
                m_roundingIntra = 8;
            }
        }
        else
        {
            m_roundingIntra = 10;
        }
    }

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter)
    {
        m_roundingInter = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetInter;
    }
    else
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            m_roundingInter = 4;
        }
        else if (m_hevcSeqParams->HierarchicalFlag && m_hevcPicParams->HierarchLevelPlus1 > 0)
        {
            //Hierachical GOP
            if (m_hevcPicParams->HierarchLevelPlus1 == 1)
            {
                m_roundingInter = 4;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
            {
                m_roundingInter = 3;
            }
            else
            {
                m_roundingInter = 2;
            }
        }
        else
        {
            m_roundingInter = 4;
        }
    }

    return eStatus;
}

MOS_STATUS HevcBasicFeature::Init422State()
{
    ENCODE_FUNC_CALL();

    m_422State = MOS_New(HevcBasicFeature422);
    ENCODE_CHK_NULL_RETURN(m_422State);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcBasicFeature::GetSurfaceMmcInfo(PMOS_SURFACE surface, MOS_MEMCOMP_STATE &mmcState, uint32_t &compressionFormat) const
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

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcBasicFeature)
{
    params.frameStatisticsStreamOut              = m_hevcPicParams->StatusReportEnable.fields.FrameStats;
    params.bitDepthMinus8                        = m_hevcSeqParams->bit_depth_luma_minus8;
    params.chromaType                            = m_hevcSeqParams->chroma_format_idc;
    params.wirelessSessionId                     = 0;
    params.randomAccess                          = !m_ref.IsLowDelay();
    params.bt2020RGB2YUV                         = m_hevcSeqParams->InputColorSpace == ECOLORSPACE_P2020;
    params.rgbInputStudioRange                   = params.bt2020RGB2YUV ? m_hevcSeqParams->RGBInputStudioRange : 0;
    params.convertedYUVStudioRange               = params.bt2020RGB2YUV ? m_hevcSeqParams->ConvertedYUVStudioRange : 0;

    if (m_captureModeEnable)
    {
        params.captureMode              = 1;
        params.tailPointerReadFrequency = 0x50;
    }

    if (m_hevcSeqParams->EnableStreamingBufferLLC || m_hevcSeqParams->EnableStreamingBufferDDR)
    {
        params.streamingBufferConfig = 1;
        params.captureMode           = 2;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, HevcBasicFeature)
{
    params.pitch                = m_rawSurfaceToPak->dwPitch;
    params.tileType             = m_rawSurfaceToPak->TileType;
    params.tileModeGmm          = m_rawSurfaceToPak->TileModeGMM;
    params.format               = m_rawSurfaceToPak->Format;
    params.gmmTileEn            = m_rawSurfaceToPak->bGMMTileEnabled;
    params.uOffset              = m_rawSurfaceToPak->YoffsetForUplane;
    params.vOffset              = m_rawSurfaceToPak->YoffsetForVplane;
    params.displayFormatSwizzle = m_hevcPicParams->bDisplayFormatSwizzle;
    params.height               = ((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    params.width                = ((m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    params.colorSpaceSelection  = (m_hevcSeqParams->InputColorSpace == ECOLORSPACE_P709) ? 1 : 0;
    params.chromaDownsampleFilterControl = 7;
    return MOS_STATUS_SUCCESS;
}

static inline uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
{
    uint32_t tileMode = 0;

    if (gmmTileEnabled)
    {
        return tileModeGMM;
    }

    switch (tileType)
    {
    case MOS_TILE_LINEAR:
        tileMode = 0;
        break;
    case MOS_TILE_YS:
        tileMode = 1;
        break;
    case MOS_TILE_X:
        tileMode = 2;
        break;
    default:
        tileMode = 3;
        break;
    }

    return tileMode;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, HevcBasicFeature)
{
    params.pitch       = m_reconSurface.dwPitch;
    params.tileType    = m_reconSurface.TileType;
    params.tileModeGmm = m_reconSurface.TileModeGMM;
    params.format      = m_reconSurface.Format;
    params.gmmTileEn   = m_reconSurface.bGMMTileEnabled;
    params.uOffset     = m_reconSurface.YoffsetForUplane;
    params.vOffset     = m_reconSurface.YoffsetForVplane;
    params.height      = ((m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    params.width       = ((m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3));
    uint32_t tileMode   = GetHwTileType(params.tileType, params.tileModeGmm, params.gmmTileEn);

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
        params.uOffset = m_rawSurfaceToPak->dwHeight;
        params.vOffset = m_rawSurfaceToPak->dwHeight << 1;
    }
    else if (m_reconSurface.Format == Format_Y216 || m_reconSurface.Format == Format_YUY2 || m_reconSurface.Format == Format_YUYV)
    {
        params.uOffset = m_rawSurfaceToPak->dwHeight;
        params.vOffset = m_rawSurfaceToPak->dwHeight;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, HevcBasicFeature)
{
    params.pitchStage1       = m_8xDSSurface->dwPitch;
    params.tileTypeStage1    = m_8xDSSurface->TileType;
    params.tileModeGmmStage1 = m_8xDSSurface->TileModeGMM;
    params.gmmTileEnStage1   = m_8xDSSurface->bGMMTileEnabled;
    params.uOffsetStage1     = m_8xDSSurface->YoffsetForUplane;
    params.vOffsetStage1     = m_8xDSSurface->YoffsetForVplane;
    params.heightStage1      = m_8xDSSurface->dwHeight;
    params.widthStage1       = m_8xDSSurface->dwWidth;

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

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcBasicFeature)
{
#ifdef _MMC_SUPPORTED
    ENCODE_CHK_NULL_RETURN(m_mmcState);   
    if (m_mmcState->IsMmcEnabled())
    {
        params.mmcEnabled = true;
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcState(const_cast<PMOS_SURFACE>(&m_rawSurface), &params.mmcStateRaw));
        ENCODE_CHK_STATUS_RETURN(m_mmcState->GetSurfaceMmcFormat(const_cast<PMOS_SURFACE>(&m_rawSurface), &params.compressionFormatRaw));
    }
    else
    {
        params.mmcEnabled           = false;
        params.mmcStateRaw          = MOS_MEMCOMP_DISABLED;
        params.compressionFormatRaw = 0;
    }
#endif

    params.surfaceRaw               = m_rawSurfaceToPak;
    params.surfaceDsStage1          = m_8xDSSurface;
    params.surfaceDsStage2          = m_4xDSSurface;
    params.pakObjCmdStreamOutBuffer = m_resMbCodeBuffer;
    params.streamOutBuffer          = m_recycleBuf->GetBuffer(VdencStatsBuffer, 0);
    params.streamOutOffset          = 0;

    params.numActiveRefL0           = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    params.numActiveRefL1           = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;
    if (m_hevcPicParams->CodingType == I_TYPE)
    {
        params.numActiveRefL0 = 0;
        params.numActiveRefL1 = 0;
    }

    if (m_hevcPicParams->CodingType == P_TYPE)
    {
        params.isPFrame = true;
    }

    //make m_ref and m_streamIn a feature
    m_ref.MHW_SETPAR_F(VDENC_PIPE_BUF_ADDR_STATE)(params);

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    ENCODE_CHK_NULL_RETURN(waTable);

    if (MEDIA_IS_WA(waTable, Wa_22011549751) &&
        m_hevcPicParams->CodingType == I_TYPE &&
        !m_osInterface->bSimIsActive &&
        !Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext) &&
        !m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
    {
        params.numActiveRefL0  = 1;
        params.numActiveRefL1  = 1;
        params.refsDsStage1[0] = &m_8xDSSurface->OsResource;
        params.refsDsStage2[0] = &m_4xDSSurface->OsResource;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD1, HevcBasicFeature)
{
    auto settings = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd1Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, m_ref.IsLowDelay()));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcBasicFeature)
{
    params.width  = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);
    params.height = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);

    params.pictureType                      = (m_hevcPicParams->CodingType == I_TYPE) ? 0 : ((m_hevcPicParams->CodingType == P_TYPE || m_ref.IsLowDelay()) ? 3 : 2);
    params.temporalMvp                      = (m_hevcPicParams->CodingType == I_TYPE || m_hevcPicParams->CodingType == P_TYPE) ? 0 : m_hevcSliceParams->slice_temporal_mvp_enable_flag;
    params.temporalMvEnableForIntegerSearch = m_ref.IsLowDelay() ? params.temporalMvp : 0;

    if (m_hevcPicParams->CodingType != I_TYPE)
    {
        params.numRefL0 = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
        params.numRefL1 = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;
    }

    params.tiling = m_hevcPicParams->tiles_enabled_flag;

    if (m_hevcPicParams->CodingType != I_TYPE)
    {
        uint8_t refFrameId;
        int8_t  diffPoc;

        refFrameId                        = m_hevcSliceParams->RefPicList[0][0].FrameIdx;
        diffPoc                           = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_hevcPicParams->RefFramePOCList[refFrameId]) - m_hevcPicParams->CurrPicOrderCnt;
        params.pocL0Ref0                 = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL0  = (refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_hevcPicParams->RefFrameList[refFrameId]);
        refFrameId                        = m_hevcSliceParams->RefPicList[0][1].FrameIdx;
        diffPoc                           = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_hevcPicParams->RefFramePOCList[refFrameId]) - m_hevcPicParams->CurrPicOrderCnt;
        params.pocL0Ref1                 = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL0 |= ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_hevcPicParams->RefFrameList[refFrameId])) << 1;
        refFrameId                        = m_hevcSliceParams->RefPicList[0][2].FrameIdx;
        diffPoc                           = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_hevcPicParams->RefFramePOCList[refFrameId]) - m_hevcPicParams->CurrPicOrderCnt;
        params.pocL0Ref2                 = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL0 |= ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_hevcPicParams->RefFrameList[refFrameId])) << 2;

        refFrameId                        = m_hevcSliceParams->RefPicList[1][0].FrameIdx;
        diffPoc                           = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_hevcPicParams->RefFramePOCList[refFrameId]) - m_hevcPicParams->CurrPicOrderCnt;
        params.pocL1Ref0                 = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL1  = (refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_hevcPicParams->RefFrameList[refFrameId]);

        params.pocL1Ref1 = params.pocL0Ref1;
        params.pocL1Ref2 = params.pocL0Ref2;
    }
    else
    {
        params.pocL0Ref0 = params.pocL0Ref1 = params.pocL1Ref0 = params.pocL1Ref1 = 0;
        params.pocL0Ref2 = params.pocL0Ref3 = params.pocL1Ref2 = params.pocL1Ref3 = 0;
    }

    if ((m_hevcPicParams->bEnableRollingIntraRefresh) && (m_hevcPicParams->CodingType != I_TYPE))
    {
        uint32_t rollingILimit = (m_hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ? MOS_ROUNDUP_DIVIDE(params.height, 32) : MOS_ROUNDUP_DIVIDE(params.width, 32);

        params.intraRefresh                = 1;
        params.qpAdjustmentForRollingI     = MOS_CLAMP_MIN_MAX(m_hevcPicParams->QpDeltaForInsertedIntra, -8, 7);
        params.intraRefreshMode            = (m_hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ? 0 : 1;
        params.intraRefreshMbSizeMinus1    = m_hevcPicParams->IntraInsertionSize - 1;
        params.intraRefreshPos            = m_hevcPicParams->IntraInsertionLocation;
        params.intraRefreshBoundary[0]     = CodecHal_Clip3(0, rollingILimit, m_hevcPicParams->RollingIntraReferenceLocation[0] - 1);
        params.intraRefreshBoundary[1]     = CodecHal_Clip3(0, rollingILimit, m_hevcPicParams->RollingIntraReferenceLocation[1] - 1);
        params.intraRefreshBoundary[2]     = CodecHal_Clip3(0, rollingILimit, m_hevcPicParams->RollingIntraReferenceLocation[2] - 1);
    }
    else
    {
        params.intraRefreshMbSizeMinus1 = 0;
    }

    params.qpPrimeYAc = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;

    // For IBC
    if (m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
    {
        params.numRefL0++;

        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            params.numRefL0                  = 0;
            params.pocL0Ref0                 = 0;
            params.longTermReferenceFlagsL0  = 1;
        }
        else
        {
            // For LDB here
            switch (params.numRefL0 - 1)
            {
            case 0:
                params.pocL0Ref0 = 0;
                params.longTermReferenceFlagsL0 |= 1;
                break;
            case 1:
                params.pocL0Ref1 = 0;
                params.longTermReferenceFlagsL0 |= 2;
                break;
            case 2:
                params.pocL0Ref2 = 0;
                params.longTermReferenceFlagsL0 |= 4;
                break;
            case 3:
                params.pocL0Ref3 = 0;
                params.longTermReferenceFlagsL0 |= 0;
                break;
            default:
                MHW_ASSERTMESSAGE("Invalid NumRefIdxL0");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }

    uint8_t frameIdxForL0L1[4] = {0x7, 0x7, 0x7, 0x7};

    MHW_MI_CHK_NULL(m_ref.GetRefIdxMapping());
    for (int i = 0; i < m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1 && i < 3; i++)
    {
        uint8_t refFrameIDx = m_hevcSliceParams->RefPicList[0][i].FrameIdx;
        if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            frameIdxForL0L1[i] = *(m_ref.GetRefIdxMapping() + refFrameIDx);
        }
    }

    if (!m_ref.IsLowDelay())
    {
        uint8_t refFrameIDx = m_hevcSliceParams->RefPicList[1][0].FrameIdx;
        if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            frameIdxForL0L1[3] = *(m_ref.GetRefIdxMapping() + refFrameIDx);
        }
    }

    params.frameIdxL0Ref0 = frameIdxForL0L1[0];
    params.frameIdxL0Ref1 = frameIdxForL0L1[1];
    params.frameIdxL0Ref2 = frameIdxForL0L1[2];
    params.frameIdxL1Ref0 = frameIdxForL0L1[3];

    params.minQp = m_hevcPicParams->BRCMinQp < 0x0a ? 0x0a : m_hevcPicParams->BRCMinQp;
    params.maxQp = m_hevcPicParams->BRCMaxQp < 0x0a ? 0x33 : (m_hevcPicParams->BRCMaxQp > 0x33 ? 0x33 : m_hevcPicParams->BRCMaxQp);

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    ENCODE_CHK_NULL_RETURN(waTable);

    if (MEDIA_IS_WA(waTable, Wa_22011549751) &&
        m_hevcPicParams->CodingType == I_TYPE &&
        !m_osInterface->bSimIsActive &&
        !Mos_Solo_Extension((MOS_CONTEXT_HANDLE)m_osInterface->pOsContext) &&
        !m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
    {
        params.pictureType = 3;
        params.frameIdxL0Ref0 = 0;
        params.frameIdxL1Ref0 = 0;
    }

    ENCODE_CHK_COND_RETURN(m_SubPelMode > 3, "Invalid subPelMode");
    params.subPelMode = m_bEnableSubPelMode ? m_SubPelMode : 3;
    auto settings = static_cast<HevcVdencFeatureSettings *>(m_constSettings);
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd2Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, m_ref.IsLowDelay()));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcBasicFeature)
{
    params.framewidthinmincbminus1         = m_hevcSeqParams->wFrameWidthInMinCbMinus1;
    params.frameheightinmincbminus1        = m_hevcSeqParams->wFrameHeightInMinCbMinus1;
    params.mincusize                       = m_hevcSeqParams->log2_min_coding_block_size_minus3;
    params.ctbsizeLcusize                  = m_hevcSeqParams->log2_max_coding_block_size_minus3;
    params.maxtusize                       = m_hevcSeqParams->log2_max_transform_block_size_minus2;
    params.mintusize                       = m_hevcSeqParams->log2_min_transform_block_size_minus2;
    params.cuQpDeltaEnabledFlag            = m_hevcPicParams->cu_qp_delta_enabled_flag;  // In VDENC mode, this field should always be set to 1.
    params.diffCuQpDeltaDepth              = m_hevcPicParams->diff_cu_qp_delta_depth;
    params.pcmLoopFilterDisableFlag        = m_hevcSeqParams->pcm_loop_filter_disable_flag;
    params.weightedPredFlag                = m_hevcPicParams->weighted_pred_flag;
    params.weightedBipredFlag              = m_hevcPicParams->weighted_bipred_flag;
    params.ampEnabledFlag                  = m_hevcSeqParams->amp_enabled_flag;
    params.transquantBypassEnableFlag      = m_hevcPicParams->transquant_bypass_enabled_flag;
    params.strongIntraSmoothingEnableFlag  = m_hevcSeqParams->strong_intra_smoothing_enable_flag;
    params.picCbQpOffset                   = m_hevcPicParams->pps_cb_qp_offset & 0x1f;
    params.picCrQpOffset                   = m_hevcPicParams->pps_cr_qp_offset & 0x1f;
    params.maxTransformHierarchyDepthIntra = m_hevcSeqParams->max_transform_hierarchy_depth_intra;
    params.maxTransformHierarchyDepthInter = m_hevcSeqParams->max_transform_hierarchy_depth_inter;
    params.pcmSampleBitDepthChromaMinus1   = m_hevcSeqParams->pcm_sample_bit_depth_chroma_minus1;
    params.pcmSampleBitDepthLumaMinus1     = m_hevcSeqParams->pcm_sample_bit_depth_luma_minus1;
    params.bitDepthChromaMinus8            = m_hevcSeqParams->bit_depth_chroma_minus8;
    params.bitDepthLumaMinus8              = m_hevcSeqParams->bit_depth_luma_minus8;
    params.lcuMaxBitsizeAllowed            = m_hevcPicParams->LcuMaxBitsizeAllowed & 0xffff;
    params.lcuMaxBitSizeAllowedMsb2its     = (m_hevcPicParams->LcuMaxBitsizeAllowed & 0x00030000) >> 16;
    if (m_hevcSeqParams->SliceSizeControl == 1)
    {
        params.pakDynamicSliceModeEnable = 1;
        params.slicePicParameterSetId    = m_hevcPicParams->slice_pic_parameter_set_id;
        params.nalunittypeflag           = (m_hevcPicParams->nal_unit_type >= HEVC_NAL_UT_BLA_W_LP) && (m_hevcPicParams->nal_unit_type <= HEVC_NAL_UT_RSV_IRAP_VCL23);
        params.noOutputOfPriorPicsFlag   = m_hevcPicParams->no_output_of_prior_pics_flag;
        params.sliceSizeThresholdInBytes = m_hevcPicParams->MaxSliceSizeInBytes;
        params.targetSliceSizeInBytes    = m_hevcPicParams->MaxSliceSizeInBytes;
    }
    params.tilesEnabledFlag             = m_hevcPicParams->tiles_enabled_flag;
    params.chromaSubsampling            = m_hevcSeqParams->chroma_format_idc;
    params.loopFilterAcrossTilesEnabled = m_hevcPicParams->loop_filter_across_tiles_flag;
    params.partialFrameUpdateMode       = false;
    params.temporalMvPredDisable        = !m_hevcSeqParams->sps_temporal_mvp_enable_flag;

    if (m_hevcSeqParams->chroma_format_idc == 2)
    {
        params.sseEnable = false;
    }
    params.constrainedIntraPredFlag = m_hevcPicParams->constrained_intra_pred_flag;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HEVC_VP9_RDOQ_STATE, HevcBasicFeature)
{
    uint8_t bitDepthLumaMinus8   = m_hevcSeqParams->bit_depth_luma_minus8;
    uint8_t bitDepthChromaMinus8 = m_hevcSeqParams->bit_depth_chroma_minus8;
    uint8_t codingType           = m_hevcPicParams->CodingType;
    auto    settings             = static_cast<HevcVdencFeatureSettings *>(m_constSettings);

    if (bitDepthLumaMinus8 < 8)
    {
        uint32_t sliceTypeIdx = (codingType == I_TYPE) ? 0 : 1;

        //Intra lambda
        MOS_ZeroMemory(params.lambdaTab, sizeof(params.lambdaTab));
        if (bitDepthLumaMinus8 == 0)
        {
            std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][0][0].begin(),
                settings->rdoqLamdas8bits[sliceTypeIdx][0][0].end(),
                std::begin(params.lambdaTab[0][0]));

            std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][0][1].begin(),
                settings->rdoqLamdas8bits[sliceTypeIdx][0][1].end(),
                std::begin(params.lambdaTab[0][1]));

            std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][1][0].begin(),
                settings->rdoqLamdas8bits[sliceTypeIdx][1][0].end(),
                std::begin(params.lambdaTab[1][0]));

            std::copy(settings->rdoqLamdas8bits[sliceTypeIdx][1][1].begin(),
                settings->rdoqLamdas8bits[sliceTypeIdx][1][1].end(),
                std::begin(params.lambdaTab[1][1]));
        }
        else if (bitDepthLumaMinus8 == 2)
        {
            std::copy(settings->rdoqLamdas10bits[sliceTypeIdx][0][0].begin(),
                settings->rdoqLamdas10bits[sliceTypeIdx][0][0].end(),
                std::begin(params.lambdaTab[0][0]));

            std::copy(settings->rdoqLamdas10bits[sliceTypeIdx][0][1].begin(),
                settings->rdoqLamdas10bits[sliceTypeIdx][0][1].end(),
                std::begin(params.lambdaTab[0][1]));

            std::copy(settings->rdoqLamdas10bits[sliceTypeIdx][1][0].begin(),
                settings->rdoqLamdas10bits[sliceTypeIdx][1][0].end(),
                std::begin(params.lambdaTab[1][0]));

            std::copy(settings->rdoqLamdas10bits[sliceTypeIdx][1][1].begin(),
                settings->rdoqLamdas10bits[sliceTypeIdx][1][1].end(),
                std::begin(params.lambdaTab[1][1]));
        }
        else if (bitDepthLumaMinus8 == 4)
        {
            std::copy(settings->rdoqLamdas12bits[sliceTypeIdx][0][0].begin(),
                settings->rdoqLamdas12bits[sliceTypeIdx][0][0].end(),
                std::begin(params.lambdaTab[0][0]));

            std::copy(settings->rdoqLamdas12bits[sliceTypeIdx][0][1].begin(),
                settings->rdoqLamdas12bits[sliceTypeIdx][0][1].end(),
                std::begin(params.lambdaTab[0][1]));

            std::copy(settings->rdoqLamdas12bits[sliceTypeIdx][1][0].begin(),
                settings->rdoqLamdas12bits[sliceTypeIdx][1][0].end(),
                std::begin(params.lambdaTab[1][0]));

            std::copy(settings->rdoqLamdas12bits[sliceTypeIdx][1][1].begin(),
                settings->rdoqLamdas12bits[sliceTypeIdx][1][1].end(),
                std::begin(params.lambdaTab[1][1]));
        }
    }
    else
    {
        int32_t shiftQP = 12;
#if INTRACONF
        double lambdaScale = 1.8;  //Intra
#else
        double lambdaScale = 1.0 - 0.35;  //LD or RA
#endif
        double   qpTemp       = 0;
        double   lambdaDouble = 0;
        uint32_t lambda       = 0;
        double   qpFactor     = 0.55;

        MOS_ZeroMemory(params.lambdaTab, sizeof(params.lambdaTab));

        int32_t bitdepthLumaQpScaleLuma   = 6 * bitDepthLumaMinus8;
        int32_t bitdepthLumaQpScaleChroma = 6 * bitDepthChromaMinus8;

        //Intra lambda
        qpFactor = 0.25 * lambdaScale;
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleLuma; qp++)
        {
            qpTemp              = (double)qp - bitdepthLumaQpScaleLuma - shiftQP;
            lambdaDouble        = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble        = lambdaDouble * 16 + 0.5;
            lambdaDouble        = (lambdaDouble > 65535) ? 65535 : lambdaDouble;
            lambda              = (uint32_t)floor(lambdaDouble);
            params.lambdaTab[0][0][qp] = (uint16_t)lambda;
        }
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleChroma; qp++)
        {
            qpTemp              = (double)qp - bitdepthLumaQpScaleChroma - shiftQP;
            lambdaDouble        = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble        = lambdaDouble * 16 + 0.5;
            lambdaDouble        = (lambdaDouble > 65535) ? 65535 : lambdaDouble;
            lambda              = (uint32_t)floor(lambdaDouble);
            params.lambdaTab[0][1][qp] = (uint16_t)lambda;
        }

        ////Inter lambda
        qpFactor = 0.55;
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleLuma; qp++)
        {
            qpTemp       = (double)qp - bitdepthLumaQpScaleLuma - shiftQP;
            lambdaDouble = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble *= MOS_MAX(1.00, MOS_MIN(1.6, 1.0 + 0.6 / 12.0 * (qpTemp - 10.0)));
            lambdaDouble        = lambdaDouble * 16 + 0.5;
            lambda              = (uint32_t)floor(lambdaDouble);
            lambda              = CodecHal_Clip3(0, 0xffff, lambda);
            params.lambdaTab[1][0][qp] = (uint16_t)lambda;
        }
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleChroma; qp++)
        {
            qpTemp       = (double)qp - bitdepthLumaQpScaleChroma - shiftQP;
            lambdaDouble = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble *= MOS_MAX(0.95, MOS_MIN(1.20, 0.25 / 12.0 * (qpTemp - 10.0) + 0.95));
            lambdaDouble        = lambdaDouble * 16 + 0.5;
            lambda              = (uint32_t)floor(lambdaDouble);
            lambda              = CodecHal_Clip3(0, 0xffff, lambda);
            params.lambdaTab[1][1][qp] = (uint16_t)lambda;
        }
    }

    if (m_hevcRDOQPerfDisabled)
    {
        params.disableHtqPerformanceFix0 = true;
        params.disableHtqPerformanceFix1 = true;
    }
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, HevcBasicFeature)
{
    PMOS_SURFACE psSurface            = nullptr;
    uint8_t      ucBitDepthLumaMinus8 = 0;
    uint8_t      chromaType           = 0;
    uint32_t     reconSurfHeight      = 0;

    ucBitDepthLumaMinus8 = m_hevcSeqParams->bit_depth_luma_minus8;
    chromaType           = m_outputChromaFormat;
    switch (params.surfaceStateId)
    {
    case CODECHAL_HCP_SRC_SURFACE_ID:
        psSurface = m_rawSurfaceToPak;
        break;
    case CODECHAL_HCP_DECODED_SURFACE_ID:
        psSurface                 = const_cast<PMOS_SURFACE>(&m_reconSurface);
        reconSurfHeight = m_rawSurfaceToPak->dwHeight;
        break;
    case CODECHAL_HCP_REF_SURFACE_ID:
        psSurface                 = const_cast<PMOS_SURFACE>(&m_reconSurface);
        reconSurfHeight = m_rawSurfaceToPak->dwHeight;
        break;
    }
#ifdef _MMC_SUPPORTED
    GetSurfaceMmcInfo(psSurface, params.mmcState, params.dwCompressionFormat);
    if (params.surfaceStateId == CODECHAL_HCP_REF_SURFACE_ID)
    {
        m_ref.MHW_SETPAR_F(HCP_SURFACE_STATE)(params);
    }
#endif

    ENCODE_CHK_NULL_RETURN(psSurface);
    params.surfacePitchMinus1 = psSurface->dwPitch - 1;

    /* Handling of reconstructed surface is different for Y410 & AYUV formats */
    if ((params.surfaceStateId != CODECHAL_HCP_SRC_SURFACE_ID) &&
        (psSurface->Format == Format_Y410))
        params.surfacePitchMinus1 = psSurface->dwPitch / 2 - 1;

    if ((params.surfaceStateId != CODECHAL_HCP_SRC_SURFACE_ID) &&
        (psSurface->Format == Format_AYUV))
        params.surfacePitchMinus1 = psSurface->dwPitch / 4 - 1;

    bool surf10bit = (psSurface->Format == Format_P010) ||
                     (psSurface->Format == Format_P210) ||
                     (psSurface->Format == Format_Y210) ||
                     (psSurface->Format == Format_Y410) ||
                     (psSurface->Format == Format_R10G10B10A2) ||
                     (psSurface->Format == Format_B10G10R10A2) ||
                     (psSurface->Format == Format_P016) ||
                     (psSurface->Format == Format_Y216);

    if (chromaType == HCP_CHROMA_FORMAT_YUV422)
    {
        if (ucBitDepthLumaMinus8 > 0)
        {
            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = surf10bit ? hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y216Y210FORMAT : hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2FORMAT;
            }
            else
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y216VARIANT;
            }
        }
        else
        {
            params.surfaceFormat = (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID) ? hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2FORMAT : hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2VARIANT;
        }
    }
    else if (chromaType == HCP_CHROMA_FORMAT_YUV444)
    {
        if (ucBitDepthLumaMinus8 == 0)
        {
            params.surfaceFormat = params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID ? hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444FORMAT : hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444VARIANT;
        }
        else if (ucBitDepthLumaMinus8 <= 2)
        {
            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = surf10bit ? hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y410FORMAT : hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444FORMAT;
            }
            else
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y416VARIANT;
            }
        }
        else
        {
            params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y416FORMAT;
        }
    }
    else  //chromaType == HCP_CHROMA_FORMAT_YUV420
    {
        if (ucBitDepthLumaMinus8 > 0)
        {
            if (params.surfaceStateId == CODECHAL_HCP_SRC_SURFACE_ID)
            {
                params.surfaceFormat = surf10bit ? hcp::SURFACE_FORMAT::SURFACE_FORMAT_P010 : hcp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
            }
            else
            {
                params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
            }
        }
        else
        {
            params.surfaceFormat = hcp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
        }
    }

    params.yOffsetForUCbInPixel = params.yOffsetForVCr =
        (uint16_t)((psSurface->UPlaneOffset.iSurfaceOffset - psSurface->dwOffset) / psSurface->dwPitch + psSurface->RenderOffset.YUV.U.YOffset);

    //Set U/V offsets for Variant surfaces
    if (params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y416VARIANT ||
        params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_AYUV4444VARIANT)
    {
        params.yOffsetForUCbInPixel = (uint16_t)reconSurfHeight;
        params.yOffsetForVCr        = (uint16_t)reconSurfHeight << 1;
    }
    else if (params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_Y216VARIANT ||
             params.surfaceFormat == hcp::SURFACE_FORMAT::SURFACE_FORMAT_YUY2VARIANT)
    {
        params.yOffsetForUCbInPixel = params.yOffsetForVCr = MOS_ALIGN_CEIL((uint16_t)reconSurfHeight, 8);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, HevcBasicFeature)
{
    ENCODE_CHK_NULL_RETURN(m_hevcSliceParams);
    PCODEC_HEVC_ENCODE_SLICE_PARAMS pEncodeHevcSliceParams = (CODEC_HEVC_ENCODE_SLICE_PARAMS *) &m_hevcSliceParams[m_curNumSlices];
    uint32_t ctbSize    = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t widthInPix = (1 << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) *
                          (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1);
    uint32_t widthInCtb = (widthInPix / ctbSize) +
                          ((widthInPix % ctbSize) ? 1 : 0);  // round up

    uint32_t ctbAddr = pEncodeHevcSliceParams->slice_segment_address;

    params.slicestartctbxOrSliceStartLcuXEncoder = ctbAddr % widthInCtb;
    params.slicestartctbyOrSliceStartLcuYEncoder = ctbAddr / widthInCtb;

    if (m_curNumSlices == m_numSlices - 1)
    {
        params.nextslicestartctbxOrNextSliceStartLcuXEncoder   = 0;
        params.nextslicestartctbyOrNextSliceStartLcuYEncoder  = 0;
    }
    else
    {
        if (m_hevcPicParams->tiles_enabled_flag)
        {
            params.nextslicestartctbxOrNextSliceStartLcuXEncoder = pEncodeHevcSliceParams[1].slice_segment_address % widthInCtb;
            params.nextslicestartctbyOrNextSliceStartLcuYEncoder = pEncodeHevcSliceParams[1].slice_segment_address / widthInCtb;
        }
        else
        {
            ctbAddr                                               = pEncodeHevcSliceParams->slice_segment_address + pEncodeHevcSliceParams->NumLCUsInSlice;
            params.nextslicestartctbxOrNextSliceStartLcuXEncoder = ctbAddr % widthInCtb;
            params.nextslicestartctbyOrNextSliceStartLcuYEncoder = ctbAddr / widthInCtb;
        }
    }
    params.sliceType = pEncodeHevcSliceParams->slice_type;
    params.lastsliceofpic = (m_curNumSlices == m_numSlices - 1);
    params.sliceqpSignFlag = ((pEncodeHevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY) >= 0) ? 0 : 1;
    params.dependentSliceFlag = false;
    params.sliceTemporalMvpEnableFlag = pEncodeHevcSliceParams->slice_temporal_mvp_enable_flag;
    if (m_hevcPicParams->CodingType == I_TYPE)
    {
        params.sliceTemporalMvpEnableFlag = 0;
    }
    params.sliceqp = (uint8_t)abs(pEncodeHevcSliceParams->slice_qp_delta + m_hevcPicParams->QpY);
    params.sliceCbQpOffset = pEncodeHevcSliceParams->slice_cb_qp_offset;
    params.sliceCrQpOffset = pEncodeHevcSliceParams->slice_cr_qp_offset;

    params.loopFilterAcrossSlicesEnabled = m_hevcPicParams->loop_filter_across_slices_flag;
    params.mvdL1ZeroFlag                 = 0;
    params.isLowDelay                    = m_ref.IsLowDelay();

    params.collocatedFromL0Flag = pEncodeHevcSliceParams->collocated_from_l0_flag;
    params.chromalog2Weightdenom = (m_hevcPicParams->weighted_pred_flag || m_hevcPicParams->weighted_bipred_flag) ? (m_hevcPicParams->bEnableGPUWeightedPrediction ? 6 : pEncodeHevcSliceParams->luma_log2_weight_denom + pEncodeHevcSliceParams->delta_chroma_log2_weight_denom) : 0;
    params.lumaLog2WeightDenom   = (m_hevcPicParams->weighted_pred_flag || m_hevcPicParams->weighted_bipred_flag) ? (m_hevcPicParams->bEnableGPUWeightedPrediction ? 6 : pEncodeHevcSliceParams->luma_log2_weight_denom) : 0;

    params.cabacInitFlag = pEncodeHevcSliceParams->cabac_init_flag;
    params.maxmergeidx   = pEncodeHevcSliceParams->MaxNumMergeCand - 1;

    if (params.sliceTemporalMvpEnableFlag)
    {
        if (params.sliceType == 2)
        {
            params.collocatedrefidx = 0;
        }
        else
        {
            // need to check with Ce for DDI issues
            uint8_t collocatedFromL0Flag = params.collocatedFromL0Flag;

            uint8_t collocatedRefIndex = m_hevcPicParams->CollocatedRefPicIndex;
            MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

            const int8_t *pRefIdxMapping     = m_ref.GetRefIdxMapping();
            uint8_t       collocatedFrameIdx = pRefIdxMapping[collocatedRefIndex];
            MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

            params.collocatedrefidx = collocatedFrameIdx;
        }
    }
    else
    {
        params.collocatedrefidx = 0;
    }

    params.sliceheaderlength = 0;

    params.emulationbytesliceinsertenable = 1;
    params.slicedataEnable                = 1;
    params.headerInsertionEnable          = 1;

    if (m_useDefaultRoundingForHcpSliceState)
    {
        params.roundinter = m_roundingInter;
        params.roundintra = m_roundingIntra;
    }
    else
    {
        params.roundinter = 4;
        params.roundintra = 10;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
