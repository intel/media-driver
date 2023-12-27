/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_preenc_basic_feature.cpp
//! \brief    Defines the common interface for encode preenc parameter
//!

#include "encode_preenc_basic_feature.h"
#include "encode_utils.h"
#include "encode_allocator.h"
#include "media_sfc_interface.h"

using namespace mhw::vdbox;

namespace encode
{
MOS_STATUS PreEncBasicFeature::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);
    ENCODE_CHK_NULL_RETURN(m_allocator);

#if (_DEBUG || _RELEASE_INTERNAL)
    MediaUserSetting::Value outValue;
    ReadUserSetting(m_userSettingPtr,
        outValue,
        "Set Media Encode Mode",
        MediaUserSetting::Group::Sequence);
    m_encodeMode = outValue.Get<uint32_t>();
#endif

    if ((m_encodeMode & PRE_ENC_PASS) == PRE_ENC_PASS)
    {
        m_enabled = true;
    }

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_STATUS_RETURN(m_preEncConstSettings->PrepareConstSettings());

    EncodeBasicFeature::Init(setting);

    ENCODE_CHK_STATUS_RETURN(InitPreEncSize());

    if (((m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC) || (m_encodeMode == MediaEncodeMode::SINGLE_PRE_FULL_ENC)) && EncodePreencBasicMember6 > 0)
    {
        m_oriFrameWidth  = m_preEncSrcWidth;
        m_oriFrameHeight = m_preEncSrcHeight;
        m_frameHeight    = MOS_ALIGN_CEIL(m_oriFrameHeight, CODECHAL_MACROBLOCK_WIDTH);
        m_frameWidth     = MOS_ALIGN_CEIL(m_oriFrameWidth, CODECHAL_MACROBLOCK_WIDTH);
    }

    ENCODE_CHK_STATUS_RETURN(AllocateResources());

    return MOS_STATUS_SUCCESS;
}

PreEncBasicFeature::~PreEncBasicFeature()
{
#if USE_CODECHAL_DEBUG_TOOL
    if (pfile0 != nullptr)
    {
        fclose(pfile0);
        pfile0 = nullptr;
    }
    if (pfile1 != nullptr)
    {
        fclose(pfile1);
        pfile1 = nullptr;
    }
#endif
    MOS_Delete(m_preEncConstSettings);
    m_preEncConstSettings = nullptr;
};

MOS_STATUS PreEncBasicFeature::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_hcpItf);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    uint32_t              bufSize       = 0;
    hcp::HcpBufferSizePar hcpBufSizePar = {};
    hcpBufSizePar.ucMaxBitDepth         = m_bitDepth;
    hcpBufSizePar.ucChromaFormat        = m_chromaFormat;
    // We should move the buffer allocation to picture level if the size is dependent on LCU size
    hcpBufSizePar.dwCtbLog2SizeY = 6;  //assume Max LCU size
    hcpBufSizePar.dwPicWidth     = MOS_ALIGN_CEIL(m_frameWidth, m_maxLCUSize);
    hcpBufSizePar.dwPicHeight    = MOS_ALIGN_CEIL(m_frameHeight, m_maxLCUSize);

    auto AllocateResource = [&](PMOS_RESOURCE &res, const hcp::HCP_INTERNAL_BUFFER_TYPE bufferType, const char *bufferName) {
        hcpBufSizePar.bufferType = bufferType;
        eStatus                  = m_hcpItf->GetHcpBufSize(hcpBufSizePar, bufSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Buffer.");
            return eStatus;
        }
        allocParamsForBufferLinear.dwBytes  = bufSize;
        allocParamsForBufferLinear.pBufName = bufferName;
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        res                                 = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        return MOS_STATUS_SUCCESS;
    };

    // Deblocking Filter Row Store Scratch data surface
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resDeblockingFilterRowStoreScratchBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::DBLK_LINE, "DeblockingScratchBuffer"));
    // Deblocking Filter Tile Row Store Scratch data surface
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resDeblockingFilterTileRowStoreScratchBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_LINE, "DeblockingTileRowScratchBuffer"));
    // Deblocking Filter Column Row Store Scratch data surface
    ENCODE_CHK_STATUS_RETURN(AllocateResource(m_resDeblockingFilterColumnRowStoreScratchBuffer, hcp::HCP_INTERNAL_BUFFER_TYPE::DBLK_TILE_COL, "DeblockingColumnScratchBuffer"));

    if (EncodePreencBasicMember2 > 0)
    {
        allocParamsForBufferLinear.dwBytes = EncodePreencBasicMember2 * sizeof(EncodePreencDef1);
        allocParamsForBufferLinear.pBufName = "pre_Ref0";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        ENCODE_CHK_STATUS_RETURN(m_recycleBuf->RegisterResource(encode::RecycleResId::PreEncRef0, allocParamsForBufferLinear));
        allocParamsForBufferLinear.pBufName = "pre_Ref1";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        ENCODE_CHK_STATUS_RETURN(m_recycleBuf->RegisterResource(encode::RecycleResId::PreEncRef1, allocParamsForBufferLinear));
    }

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = m_is10Bit ? Format_P010 : Format_NV12;
    allocParamsForBuffer2D.dwWidth  = m_frameWidth;
    allocParamsForBuffer2D.dwHeight = m_frameHeight;
    allocParamsForBuffer2D.pBufName = "preRawDsSurface";
    allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INPUT_RAW;
    ENCODE_CHK_STATUS_RETURN(m_recycleBuf->RegisterResource(encode::RecycleResId::PreEncRawSurface, allocParamsForBuffer2D));

    return eStatus;
}

MOS_STATUS PreEncBasicFeature::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    ENCODE_CHK_STATUS_RETURN(PreparePreEncConfig(params));

    ENCODE_CHK_STATUS_RETURN(EncodeBasicFeature::Update(params));

    ENCODE_CHK_STATUS_RETURN(SetPictureStructs());
    ENCODE_CHK_STATUS_RETURN(SetSliceStructs());

    if (m_frameNum == 0)
    {
        m_resolutionChanged = true;
    }
    else
    {
        m_resolutionChanged = false;
    }

    if (m_resolutionChanged)
    {
        ENCODE_CHK_STATUS_RETURN(UpdateTrackedBufferParameters());
    }
    ENCODE_CHK_STATUS_RETURN(GetRecycleBuffers());
    ENCODE_CHK_STATUS_RETURN(GetTrackedBuffers());

    return MOS_STATUS_SUCCESS;
}

// This function may be used by fullenc feature.
// Please don't use class member variables in the function.
MOS_STATUS PreEncBasicFeature::CalculatePreEncInfo(uint32_t width, uint32_t height, PreEncInfo &preEncInfo)
{
    ENCODE_FUNC_CALL();

    preEncInfo.EncodePreEncInfo2 = 0;
    if (width >= 7680 && height >= 4320)
        preEncInfo.EncodePreEncInfo2 = 2;  //8x8
    else if (width >= 3840 && height >= 2160)
        preEncInfo.EncodePreEncInfo2 = 1;  //16x16
    else if (width >= 1920 && height >= 1080)
        preEncInfo.EncodePreEncInfo2 = 0;  //32x32

    preEncInfo.EncodePreEncInfo3 = 0;
    if (width >= 7680 && height >= 4320)
        preEncInfo.EncodePreEncInfo3 = 3;  //8x
    else if (width >= 3840 && height >= 2160)
        preEncInfo.EncodePreEncInfo3 = 2;  //4x
    else //if (width >= 1920 && height >= 1080)
        preEncInfo.EncodePreEncInfo3 = 1;  //2x

    preEncInfo.EncodePreEncInfo0 = 0;
    if (width >= 7680 && height >= 4320)
        preEncInfo.EncodePreEncInfo0 = 1;  //2x
    else if (width >= 3840 && height >= 2160)
        preEncInfo.EncodePreEncInfo0 = 0;  //1x
    else                                      //if (OrgSrcWidth >= 1920 && OrgSrcHeight >= 1080)
        preEncInfo.EncodePreEncInfo0 = 3;  //0.5x

    uint8_t vdencFactor = MAX(0, (5 - preEncInfo.EncodePreEncInfo2) + (preEncInfo.EncodePreEncInfo0 == 3 ? -1 : preEncInfo.EncodePreEncInfo0) - 4);

    uint32_t offset          = (1 << preEncInfo.EncodePreEncInfo3) - 1;
    uint32_t preEncSrcWidth  = (width + offset) >> preEncInfo.EncodePreEncInfo3;
    uint32_t preEncSrcHeight = (height + offset) >> preEncInfo.EncodePreEncInfo3;

    preEncSrcWidth  = ((preEncSrcWidth + 7) >> 3) << 3;
    preEncSrcHeight = ((preEncSrcHeight + 7) >> 3) << 3;

    preEncInfo.preEncSrcWidth  = preEncSrcWidth;
    preEncInfo.preEncSrcHeight = preEncSrcHeight;

    //pitch be 64 aligned and height be 64 aligned
    uint16_t vdencInfoStride = (uint16_t)((((preEncSrcWidth + 63) >> 6) << 6) >> (5 - preEncInfo.EncodePreEncInfo2));
    uint16_t vdencInfoHeight = (uint16_t)((((preEncSrcHeight + 63) >> 6) << 6) >> (5 - preEncInfo.EncodePreEncInfo2));

    vdencInfoStride = ((vdencInfoStride + 7) >> 3) << 3;

    vdencInfoStride <<= vdencFactor;
    vdencInfoHeight <<= vdencFactor;

    preEncInfo.EncodePreEncInfo1 = vdencInfoStride * vdencInfoHeight;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::InitPreEncSize()
{
    ENCODE_FUNC_CALL();

    if (m_encodeMode == MediaEncodeMode::MANUAL_RES_PRE_ENC)
    {
        MediaUserSetting::Value outValue;
        ReadUserSetting(m_userSettingPtr,
            outValue,
            "Set Media Encode Downscaled Ratio",
            MediaUserSetting::Group::Sequence);
        uint32_t downscaledRatio = outValue.Get<uint32_t>();
        uint32_t orgFrameWidth   = 0;
        uint32_t orgFrameHeight  = 0;
        if (downscaledRatio != 0)
        {
            orgFrameWidth  = m_frameWidth * downscaledRatio;
            orgFrameHeight = m_frameHeight * downscaledRatio;
        }
        else
        {
            orgFrameWidth  = m_frameWidth;
            orgFrameHeight = m_frameHeight;
        }
        ENCODE_CHK_STATUS_RETURN(CalculatePreEncInfo(orgFrameWidth, orgFrameHeight, m_preEncInfo));
    }
    else
    {
        ENCODE_CHK_STATUS_RETURN(CalculatePreEncInfo(m_frameWidth, m_frameHeight, m_preEncInfo));
    }

    EncodePreencBasicMember5 = m_preEncInfo.EncodePreEncInfo2;
    EncodePreencBasicMember6 = m_preEncInfo.EncodePreEncInfo3;
    EncodePreencBasicMember1 = m_preEncInfo.EncodePreEncInfo0;
    EncodePreencBasicMember2 = m_preEncInfo.EncodePreEncInfo1;
    m_preEncSrcWidth         = m_preEncInfo.preEncSrcWidth;
    m_preEncSrcHeight        = m_preEncInfo.preEncSrcHeight;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::GetPreEncInfo(PreEncInfo &preEncInfo)
{
    if (!m_enabled)
        return MOS_STATUS_INVALID_PARAMETER;
    preEncInfo = m_preEncInfo;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::SetPictureStructs()
{
    ENCODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //Create flat scaling list
    memset(&m_hevcIqMatrixParams, 0x10, sizeof(m_hevcIqMatrixParams));

    m_pictureCodingType = m_preEncConfig.CodingType;

    uint16_t log2_max_coding_block_size = 6;

    rawCTUBits = (1 << (2 * log2_max_coding_block_size));
    rawCTUBits = rawCTUBits * 3 / 2;
    rawCTUBits = rawCTUBits * (m_preEncConfig.BitDepthLumaMinus8 + 8);
    rawCTUBits = (5 * rawCTUBits / 3);

    return eStatus;
}

static MOS_STATUS SetSurfaceMMCParams(EncodeMemComp &mmcState, MOS_SURFACE &surf)
{
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcMode(&surf));
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcState(&surf));
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcFormat(&surf));
    surf.bIsCompressed = surf.CompressionMode != MOS_MMC_DISABLED;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::UpdateTrackedBufferParameters()
{
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    uint32_t numOfLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, m_maxLCUSize) * (MOS_ROUNDUP_DIVIDE(m_frameHeight, m_maxLCUSize) + 1);

    m_mbCodeSize = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * (numOfLCU * 5 + numOfLCU * 64 * 8), CODECHAL_PAGE_SIZE);

    uint32_t downscaledWidthInMb4x     = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    uint32_t downscaledHeightInMb4x    = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    uint32_t downscaledSurfaceHeight4x = ((downscaledHeightInMb4x + 1) >> 1) * CODECHAL_MACROBLOCK_HEIGHT;
    m_downscaledWidth4x                = downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x               = MOS_ALIGN_CEIL(downscaledSurfaceHeight4x, MOS_YTILE_H_ALIGNMENT) << 1;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = m_is10Bit ? Format_P010 : Format_NV12;

    allocParamsForBuffer2D.dwWidth  = m_frameWidth;
    allocParamsForBuffer2D.dwHeight = m_frameHeight;
    allocParamsForBuffer2D.pBufName = "preRefSurface";
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::preRefSurface, allocParamsForBuffer2D));

    if (m_downscaledWidth4x > 0 && m_downscaledHeight4x > 0)
    {
        allocParamsForBuffer2D.Format   = Format_NV12;
        allocParamsForBuffer2D.dwWidth  = m_downscaledWidth4x;
        allocParamsForBuffer2D.dwHeight = m_downscaledHeight4x;
        allocParamsForBuffer2D.pBufName = "Pre4xDSSurface";
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::preDs4xSurface, allocParamsForBuffer2D));

        allocParamsForBuffer2D.dwWidth  = m_downscaledWidth4x >> 1;
        allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(m_downscaledHeight4x >> 1, MOS_YTILE_H_ALIGNMENT) << 1;
        allocParamsForBuffer2D.pBufName = "Pre8xDSSurface";
        ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(encode::BufferType::preDs8xSurface, allocParamsForBuffer2D));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::GetRecycleBuffers()
{
    ENCODE_CHK_NULL_RETURN(m_recycleBuf);
    auto bIdx = m_preEncConfig.CurrOriginalPic.FrameIdx;
    if ((m_encodeMode == MediaEncodeMode::SINGLE_PRE_FULL_ENC || m_encodeMode == MediaEncodeMode::AUTO_RES_PRE_ENC) && (EncodePreencBasicMember6 > 0 || m_preEncConfig.BitDepthLumaMinus8 > 0))
    {
        m_rawDsSurface = m_recycleBuf->GetSurface(encode::RecycleResId::PreEncRawSurface, bIdx);
        ENCODE_CHK_NULL_RETURN(m_rawDsSurface);
        ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_rawDsSurface));
    }

    EncodePreencBasicMember3 = m_recycleBuf->GetBuffer(encode::RecycleResId::PreEncRef0, bIdx);
    ENCODE_CHK_NULL_RETURN(EncodePreencBasicMember3);
    EncodePreencBasicMember4 = m_recycleBuf->GetBuffer(encode::RecycleResId::PreEncRef1, bIdx);
    ENCODE_CHK_NULL_RETURN(EncodePreencBasicMember4);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::GetTrackedBuffers()
{
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);
    ENCODE_CHK_NULL_RETURN(m_allocator);

    PMOS_SURFACE reconSurface = m_trackedBuf->GetSurface(BufferType::preRefSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(reconSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(reconSurface));
    m_reconSurface = *(reconSurface);

    m_4xDSSurface = m_trackedBuf->GetSurface(BufferType::preDs4xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_4xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_4xDSSurface));

    m_8xDSSurface = m_trackedBuf->GetSurface(BufferType::preDs8xSurface, m_trackedBuf->GetCurrIndex());
    ENCODE_CHK_NULL_RETURN(m_8xDSSurface);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_8xDSSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::EncodePreencBasicFuntion0(PMOS_RESOURCE& Buffer0, PMOS_RESOURCE& Buffer1)
{
    ENCODE_FUNC_CALL();
    Buffer0 = EncodePreencBasicMember3;
    Buffer1 = EncodePreencBasicMember4;
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, PreEncBasicFeature)
{
    params.bitDepthMinus8    = m_preEncConfig.BitDepthLumaMinus8;
    params.chromaType        = 1;
    params.wirelessSessionId = 0;
    params.randomAccess      = !IsLowDelay();

    params.VdencPipeModeSelectPar2 = 1;
    if (m_preEncConfig.CodingType != I_TYPE && !IsLowDelay())
    {
        params.VdencPipeModeSelectPar4 = 3;
    }
    else
    {
        params.VdencPipeModeSelectPar4 = 1;
    }
    params.VdencPipeModeSelectPar5 = EncodePreencBasicMember6;
    params.VdencPipeModeSelectPar7 = EncodePreencBasicMember5;
    params.VdencPipeModeSelectPar6 = EncodePreencBasicMember1;

    if (m_preEncConfig.CodingType == I_TYPE)
    {
        params.VdencPipeModeSelectPar2 = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_SRC_SURFACE_STATE, PreEncBasicFeature)
{
    params.pitch                         = m_preEncRawSurface->dwPitch;
    params.tileType                      = m_preEncRawSurface->TileType;
    params.tileModeGmm                   = m_preEncRawSurface->TileModeGMM;
    params.format                        = m_preEncRawSurface->Format;
    params.gmmTileEn                     = m_preEncRawSurface->bGMMTileEnabled;
    params.uOffset                       = m_preEncRawSurface->YoffsetForUplane;
    params.vOffset                       = m_preEncRawSurface->YoffsetForVplane;
    params.height                        = m_oriFrameHeight;
    params.width                         = m_oriFrameWidth;
    params.colorSpaceSelection           = (m_preEncConfig.InputColorSpace == ECOLORSPACE_P709) ? 1 : 0;
    params.chromaDownsampleFilterControl = 7;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_REF_SURFACE_STATE, PreEncBasicFeature)
{
    params.pitch       = m_reconSurface.dwPitch;
    params.tileType    = m_reconSurface.TileType;
    params.tileModeGmm = m_reconSurface.TileModeGMM;
    params.format      = m_reconSurface.Format;
    params.gmmTileEn   = m_reconSurface.bGMMTileEnabled;
    params.uOffset     = m_reconSurface.YoffsetForUplane;
    params.vOffset     = m_reconSurface.YoffsetForVplane;
    params.height      = m_oriFrameHeight;
    params.width       = m_oriFrameWidth;

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
        params.uOffset = m_preEncRawSurface->dwHeight;
        params.vOffset = m_preEncRawSurface->dwHeight << 1;
    }
    else if (m_reconSurface.Format == Format_Y216 || m_reconSurface.Format == Format_YUY2 || m_reconSurface.Format == Format_YUYV)
    {
        params.uOffset = m_preEncRawSurface->dwHeight;
        params.vOffset = m_preEncRawSurface->dwHeight;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_DS_REF_SURFACE_STATE, PreEncBasicFeature)
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

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, PreEncBasicFeature)
{
    params.surfaceRaw               = m_preEncRawSurface;
    params.surfaceDsStage1          = m_8xDSSurface;
    params.surfaceDsStage2          = m_4xDSSurface;

    params.numActiveRefL0 = 1;
    params.numActiveRefL1 = 1;
    if (m_preEncConfig.CodingType == I_TYPE)
    {
        params.numActiveRefL0 = 0;
        params.numActiveRefL1 = 0;
    }

    // for reference
    auto l0RefFrameList = m_preEncConfig.RefPicList[LIST_0];

    CODEC_PICTURE refPic = l0RefFrameList[0];

    if (!CodecHal_PictureIsInvalid(refPic) && m_preEncConfig.PicIdx[refPic.FrameIdx].bValid)
    {
        // L0 references
        uint8_t refPicIdx = m_preEncConfig.PicIdx[refPic.FrameIdx].ucPicIdx;

        // 1x/4x/8x DS surface for VDEnc
        uint8_t scaledIdx = m_preEncConfig.RefList[refPicIdx]->ucScalingIdx;

        auto vdencRefSurface = m_trackedBuf->GetSurface(BufferType::preRefSurface, scaledIdx);
        ENCODE_CHK_NULL_RETURN(vdencRefSurface);
        auto vdenc4xDsSurface = m_trackedBuf->GetSurface(BufferType::preDs4xSurface, scaledIdx);
        ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
        auto vdenc8xDsSurface = m_trackedBuf->GetSurface(BufferType::preDs8xSurface, scaledIdx);
        ENCODE_CHK_NULL_RETURN(vdenc8xDsSurface);
        params.refs[0]         = &(vdencRefSurface->OsResource);
        params.refsDsStage2[0] = &vdenc4xDsSurface->OsResource;
        params.refsDsStage1[0] = &vdenc8xDsSurface->OsResource;
    }

    const CODEC_PICTURE *l1RefFrameList = m_preEncConfig.isPToB ? m_preEncConfig.RefPicList[LIST_0] : m_preEncConfig.RefPicList[LIST_1];

    refPic = l1RefFrameList[0];

    if (!CodecHal_PictureIsInvalid(refPic) && m_preEncConfig.PicIdx[refPic.FrameIdx].bValid)
    {
        // L1 references
        uint8_t refPicIdx = m_preEncConfig.PicIdx[refPic.FrameIdx].ucPicIdx;

        // 1x/4x/8x DS surface for VDEnc
        uint8_t scaledIdx = m_preEncConfig.RefList[refPicIdx]->ucScalingIdx;

        auto vdencRefSurface = m_trackedBuf->GetSurface(BufferType::preRefSurface, scaledIdx);
        ENCODE_CHK_NULL_RETURN(vdencRefSurface);
        auto vdenc4xDsSurface = m_trackedBuf->GetSurface(BufferType::preDs4xSurface, scaledIdx);
        ENCODE_CHK_NULL_RETURN(vdenc4xDsSurface);
        auto vdenc8xDsSurface = m_trackedBuf->GetSurface(BufferType::preDs8xSurface, scaledIdx);
        ENCODE_CHK_NULL_RETURN(vdenc8xDsSurface);
        params.refs[1]         = &(vdencRefSurface->OsResource);
        params.refsDsStage2[1] = &vdenc4xDsSurface->OsResource;
        params.refsDsStage1[1] = &vdenc8xDsSurface->OsResource;
    }

    params.lowDelayB = m_lowDelay;

    params.vdencPipeBufAddrStatePar0 = EncodePreencBasicMember3;
    params.vdencPipeBufAddrStatePar1 = EncodePreencBasicMember4;

    return MOS_STATUS_SUCCESS;
}

#define CLIP3(MIN_, MAX_, X) (((X) < (MIN_)) ? (MIN_) : (((X) > (MAX_)) ? (MAX_) : (X)))

MHW_SETPAR_DECL_SRC(VDENC_CMD1, PreEncBasicFeature)
{
    ENCODE_CHK_NULL_RETURN(m_preEncConstSettings);
    auto settings = static_cast<PreEncFeatureSettings *>(m_preEncConstSettings->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd1Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, IsLowDelay(), m_preEncConfig));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, PreEncBasicFeature)
{
    params.width  = m_oriFrameWidth;
    params.height = m_oriFrameHeight;

    params.pictureType = (m_preEncConfig.CodingType == I_TYPE) ? 0 : (IsLowDelay() ? 3 : 2);
    params.temporalMvp = 0;

    if (m_preEncConfig.CodingType != I_TYPE)
    {
        params.numRefL0 = 1;
        params.numRefL1 = 1;
    }

    params.tiling = 0;

    if (m_preEncConfig.CodingType != I_TYPE)
    {
        uint8_t refFrameId;
        int8_t  diffPoc;

        refFrameId                      = m_preEncConfig.RefPicList[0][0].FrameIdx;
        diffPoc                         = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_preEncConfig.RefFramePOCList[refFrameId]) - m_preEncConfig.CurrPicOrderCnt;
        params.pocL0Ref0                = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL0 = (refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_preEncConfig.RefFrameList[refFrameId]);
        refFrameId                      = m_preEncConfig.RefPicList[0][1].FrameIdx;
        diffPoc                         = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_preEncConfig.RefFramePOCList[refFrameId]) - m_preEncConfig.CurrPicOrderCnt;
        params.pocL0Ref1                = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL0 |= ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_preEncConfig.RefFrameList[refFrameId])) << 1;
        refFrameId       = m_preEncConfig.RefPicList[0][2].FrameIdx;
        diffPoc          = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_preEncConfig.RefFramePOCList[refFrameId]) - m_preEncConfig.CurrPicOrderCnt;
        params.pocL0Ref2 = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL0 |= ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_preEncConfig.RefFrameList[refFrameId])) << 2;

        refFrameId                      = m_preEncConfig.isPToB ? m_preEncConfig.RefPicList[0][0].FrameIdx : m_preEncConfig.RefPicList[1][0].FrameIdx;
        diffPoc                         = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : m_preEncConfig.RefFramePOCList[refFrameId]) - m_preEncConfig.CurrPicOrderCnt;
        params.pocL1Ref0                = (int8_t)CodecHal_Clip3(-16, 16, -diffPoc);
        params.longTermReferenceFlagsL1 = (refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : CodecHal_PictureIsLongTermRef(m_preEncConfig.RefFrameList[refFrameId]);

        params.pocL1Ref1 = params.pocL0Ref1;
        params.pocL1Ref2 = params.pocL0Ref2;
    }
    else
    {
        params.pocL0Ref0 = params.pocL0Ref1 = params.pocL1Ref0 = params.pocL1Ref1 = 0;
        params.pocL0Ref2 = params.pocL0Ref3 = params.pocL1Ref2 = params.pocL1Ref3 = 0;
    }
    params.minQp = 0x0a;
    params.maxQp = 0x33;

    params.qpPrimeYAc = m_QP;

    uint8_t frameIdxForL0L1[4] = {0x7, 0x7, 0x7, 0x7};

    MHW_MI_CHK_NULL(m_refIdxMapping);
    for (int i = 0; i < 1; i++)
    {
        uint8_t refFrameIDx = m_preEncConfig.RefPicList[0][i].FrameIdx;
        if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            frameIdxForL0L1[i] = *(m_refIdxMapping + refFrameIDx);
        }
    }

    if (!IsLowDelay())
    {
        uint8_t refFrameIDx = m_preEncConfig.RefPicList[1][0].FrameIdx;
        if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
        {
            frameIdxForL0L1[3] = *(m_refIdxMapping + refFrameIDx);
        }
    }

    params.frameIdxL0Ref0 = frameIdxForL0L1[0];
    params.frameIdxL0Ref1 = frameIdxForL0L1[1];
    params.frameIdxL0Ref2 = frameIdxForL0L1[2];
    params.frameIdxL1Ref0 = frameIdxForL0L1[3];

    ENCODE_CHK_NULL_RETURN(m_preEncConstSettings);
    auto settings = static_cast<PreEncFeatureSettings *>(m_preEncConstSettings->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    for (const auto &lambda : settings->vdencCmd2Settings)
    {
        ENCODE_CHK_STATUS_RETURN(lambda(params, IsLowDelay(), m_preEncConfig));
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WALKER_STATE, PreEncBasicFeature)
{
    uint32_t ctbSize        = 64;
    uint32_t widthInPix     = m_oriFrameWidth;
    uint32_t widthInCtb     = (widthInPix / ctbSize) + ((widthInPix % ctbSize) ? 1 : 0);  // round up
    uint32_t heightInPix    = m_oriFrameHeight;
    uint32_t heightInCtb    = (heightInPix / ctbSize) + ((heightInPix % ctbSize) ? 1 : 0);  // round up
    uint32_t numLCUsInSlice = widthInCtb * heightInCtb;

    params.firstSuperSlice = 0;
    // No tiling support
    params.tileSliceStartLcuMbY     = 0;
    params.nextTileSliceStartLcuMbX = numLCUsInSlice / heightInCtb;
    params.nextTileSliceStartLcuMbY = numLCUsInSlice / widthInCtb;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, PreEncBasicFeature)
{
    params.ctbSize                    = 64;
    params.tileWidth                  = m_oriFrameWidth;
    params.tileHeight                 = m_oriFrameHeight;
    params.log2WeightDenomLuma        = 0;
    params.hevcVp9Log2WeightDenomLuma = 0;
    params.log2WeightDenomChroma      = 0;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_WEIGHTSOFFSETS_STATE, PreEncBasicFeature)
{
    params.denomLuma = 1;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::SetSliceStructs()
{
    ENCODE_FUNC_CALL();

    m_lowDelay    = true;
    m_sameRefList = true;

    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        m_refIdxMapping[i]  = -1;
        m_currUsedRefPic[i] = false;
    }

    // To obtain current "used" reference frames. The number of current used reference frames cannot be greater than 8
    for (uint8_t ll = 0; ll < 2; ll++)
    {
        CODEC_PICTURE refPic = m_preEncConfig.RefPicList[ll][0];
        if (!CodecHal_PictureIsInvalid(refPic) &&
            !CodecHal_PictureIsInvalid(m_preEncConfig.RefFrameList[refPic.FrameIdx]))
        {
            m_currUsedRefPic[refPic.FrameIdx] = true;
        }
    }

    for (uint8_t i = 0, refIdx = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
    {
        if (!m_currUsedRefPic[i])
        {
            continue;
        }

        uint8_t index         = m_preEncConfig.RefFrameList[i].FrameIdx;
        bool    duplicatedIdx = false;
        for (uint8_t ii = 0; ii < i; ii++)
        {
            if (m_currUsedRefPic[i] && index == m_preEncConfig.RefFrameList[ii].FrameIdx)
            {
                // We find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                // In other words, RefFrameList[i] and RefFrameList[ii] have the same surface Id
                duplicatedIdx      = true;
                m_refIdxMapping[i] = m_refIdxMapping[ii];
                break;
            }
        }

        if (duplicatedIdx)
        {
            continue;
        }

        if (refIdx >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
        {
            // Total number of distingushing reference frames cannot be geater than 8.
            ENCODE_ASSERT(false);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Map reference frame index [0-15] into a set of unique IDs within [0-7]
        m_refIdxMapping[i] = refIdx;
        refIdx++;
    }

    ENCODE_CHK_STATUS_RETURN(ValidateLowDelayBFrame());
    ENCODE_CHK_STATUS_RETURN(ValidateSameRefInL0L1());

    if (m_lowDelay && !m_sameRefList)
    {
        ENCODE_NORMALMESSAGE("Attention: LDB frame but with different L0/L1 list !");
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS PreEncBasicFeature::ValidateSameRefInL0L1()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    if (m_sameRefList)
    {
        for (int refIdx = 0; refIdx < 1; refIdx++)
        {
            CODEC_PICTURE refPicL0 = m_preEncConfig.RefPicList[0][refIdx];
            CODEC_PICTURE refPicL1 = m_preEncConfig.RefPicList[1][refIdx];

            if (!CodecHal_PictureIsInvalid(refPicL0) && !CodecHal_PictureIsInvalid(refPicL1) && refPicL0.FrameIdx != refPicL1.FrameIdx)
            {
                m_sameRefList = false;
                break;
            }
        }
    }

    return eStatus;
}

MOS_STATUS PreEncBasicFeature::ValidateLowDelayBFrame()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    // Examine if now it is B type
    if (m_preEncConfig.SliceType == encodeHevcBSlice)
    {
        // forward
        for (int refIdx = 0; (refIdx < 1) && m_lowDelay; refIdx++)
        {
            CODEC_PICTURE refPic = m_preEncConfig.RefPicList[0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && m_preEncConfig.RefFramePOCList[refPic.FrameIdx] > m_preEncConfig.CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }

        // backward
        for (int refIdx = 0; (refIdx < 1) && m_lowDelay; refIdx++)
        {
            CODEC_PICTURE refPic = m_preEncConfig.RefPicList[1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && m_preEncConfig.RefFramePOCList[refPic.FrameIdx] > m_preEncConfig.CurrPicOrderCnt)
            {
                m_lowDelay = false;
            }
        }
    }

    return eStatus;
}

bool PreEncBasicFeature::IsCurrentUsedAsRef(uint8_t idx) const
{
    ENCODE_FUNC_CALL();

    if (idx < CODEC_MAX_NUM_REF_FRAME_HEVC &&
        m_preEncConfig.PicIdx[idx].bValid && m_currUsedRefPic[idx])
    {
        return true;
    }
    else
    {
        return false;
    }
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, PreEncBasicFeature)
{
    ENCODE_FUNC_CALL();

    //add for B frame support
    if (m_pictureCodingType != I_TYPE)
    {
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (IsCurrentUsedAsRef(i))
            {
                uint8_t idx       = m_preEncConfig.PicIdx[i].ucPicIdx;
                uint8_t scaledIdx = m_preEncConfig.RefList[idx]->ucScalingIdx;

                uint8_t frameStoreId = m_refIdxMapping[i];
                auto    refSurface   = m_trackedBuf->GetSurface(BufferType::preRefSurface, scaledIdx);
                ENCODE_CHK_NULL_RETURN(refSurface);
                params.presReferences[frameStoreId] = &(refSurface->OsResource);
                if (m_preEncConfig.isPToB)
                {
                    params.presReferences[frameStoreId + 1] = &(refSurface->OsResource);
                }
            }
        }
    }

    params.presDeblockingFilterTileRowStoreScratchBuffer   = m_resDeblockingFilterTileRowStoreScratchBuffer;
    params.presDeblockingFilterColumnRowStoreScratchBuffer = m_resDeblockingFilterColumnRowStoreScratchBuffer;
    params.presMfdDeblockingFilterRowStoreScratchBuffer    = m_resDeblockingFilterRowStoreScratchBuffer;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, PreEncBasicFeature)
{
    ENCODE_FUNC_CALL();

    params.bRdoqEnable = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HEVC_VP9_RDOQ_STATE, PreEncBasicFeature)
{
    uint8_t bitDepthLumaMinus8   = m_preEncConfig.BitDepthLumaMinus8;
    uint8_t bitDepthChromaMinus8 = m_preEncConfig.BitDepthChromaMinus8;
    uint8_t codingType           = m_preEncConfig.CodingType;
    auto    settings             = static_cast<PreEncFeatureSettings *>(m_preEncConstSettings->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

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
            qpTemp                     = (double)qp - bitdepthLumaQpScaleLuma - shiftQP;
            lambdaDouble               = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble               = lambdaDouble * 16 + 0.5;
            lambdaDouble               = (lambdaDouble > 65535) ? 65535 : lambdaDouble;
            lambda                     = (uint32_t)floor(lambdaDouble);
            params.lambdaTab[0][0][qp] = (uint16_t)lambda;
        }
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleChroma; qp++)
        {
            qpTemp                     = (double)qp - bitdepthLumaQpScaleChroma - shiftQP;
            lambdaDouble               = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble               = lambdaDouble * 16 + 0.5;
            lambdaDouble               = (lambdaDouble > 65535) ? 65535 : lambdaDouble;
            lambda                     = (uint32_t)floor(lambdaDouble);
            params.lambdaTab[0][1][qp] = (uint16_t)lambda;
        }

        ////Inter lambda
        qpFactor = 0.55;
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleLuma; qp++)
        {
            qpTemp       = (double)qp - bitdepthLumaQpScaleLuma - shiftQP;
            lambdaDouble = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble *= MOS_MAX(1.00, MOS_MIN(1.6, 1.0 + 0.6 / 12.0 * (qpTemp - 10.0)));
            lambdaDouble               = lambdaDouble * 16 + 0.5;
            lambda                     = (uint32_t)floor(lambdaDouble);
            lambda                     = CodecHal_Clip3(0, 0xffff, lambda);
            params.lambdaTab[1][0][qp] = (uint16_t)lambda;
        }
        for (uint8_t qp = 0; qp < 52 + bitdepthLumaQpScaleChroma; qp++)
        {
            qpTemp       = (double)qp - bitdepthLumaQpScaleChroma - shiftQP;
            lambdaDouble = qpFactor * pow(2.0, qpTemp / 3.0);
            lambdaDouble *= MOS_MAX(0.95, MOS_MIN(1.20, 0.25 / 12.0 * (qpTemp - 10.0) + 0.95));
            lambdaDouble               = lambdaDouble * 16 + 0.5;
            lambda                     = (uint32_t)floor(lambdaDouble);
            lambda                     = CodecHal_Clip3(0, 0xffff, lambda);
            params.lambdaTab[1][1][qp] = (uint16_t)lambda;
        }
    }

    params.disableHtqPerformanceFix0 = true;
    params.disableHtqPerformanceFix1 = true;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, PreEncBasicFeature)
{
    params.framewidthinmincbminus1         = (uint16_t)(m_oriFrameWidth >> 3) - 1;
    params.transformSkipEnabled            = true;
    params.frameheightinmincbminus1        = (uint16_t)(m_oriFrameHeight >> 3) - 1;
    params.ctbsizeLcusize                  = 3;
    params.maxtusize                       = 3;
    params.mintusize                       = 0;
    params.cuQpDeltaEnabledFlag            = 1;  // In VDENC mode, this field should always be set to 1.
    params.diffCuQpDeltaDepth              = 3;
    params.pcmLoopFilterDisableFlag        = 1;
    params.ampEnabledFlag                  = 1;
    params.maxTransformHierarchyDepthIntra = 2;
    params.maxTransformHierarchyDepthInter = 2;
    params.pcmSampleBitDepthChromaMinus1   = 0;
    params.pcmSampleBitDepthLumaMinus1     = 0;
    params.bitDepthChromaMinus8            = m_preEncConfig.BitDepthChromaMinus8;
    params.bitDepthLumaMinus8              = m_preEncConfig.BitDepthLumaMinus8;
    params.lcuMaxBitsizeAllowed            = rawCTUBits & 0xffff;
    params.lcuMaxBitSizeAllowedMsb2its     = (rawCTUBits & 0x00030000) >> 16;
    params.rdoqEnable                      = true;

    params.chromaSubsampling            = 1;
    params.loopFilterAcrossTilesEnabled = 0;

    // Disable HEVC RDOQ for Intra blocks
    params.intratucountbasedrdoqdisable = 1;
    params.rdoqintratuthreshold         = 0xffff;

    params.partialFrameUpdateMode = false;
    params.temporalMvPredDisable  = true;
    params.sseEnable              = false;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SLICE_STATE, PreEncBasicFeature)
{
    params.slicestartctbxOrSliceStartLcuXEncoder = 0;
    params.slicestartctbyOrSliceStartLcuYEncoder = 0;

    params.nextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
    params.nextslicestartctbyOrNextSliceStartLcuYEncoder = 0;

    params.sliceType          = (m_preEncConfig.SliceType == encodeHevcISlice) ? 2 : 0;
    params.lastsliceofpic     = 1;
    params.sliceqpSignFlag    = 0;
    params.dependentSliceFlag = false;

    params.sliceqp         = m_QP;
    params.sliceCbQpOffset = 0;
    params.sliceCrQpOffset = 0;

    params.loopFilterAcrossSlicesEnabled = 1;
    params.mvdL1ZeroFlag                 = 0;
    params.isLowDelay                    = IsLowDelay();

    params.collocatedFromL0Flag  = 0;
    params.chromalog2Weightdenom = 0;
    params.lumaLog2WeightDenom   = 0;

    params.cabacInitFlag = 0;
    params.maxmergeidx   = 4;

    params.collocatedrefidx = 0;

    params.sliceheaderlength = 0;

    params.emulationbytesliceinsertenable = 1;
    params.slicedataEnable                = 1;
    params.headerInsertionEnable          = 1;

    // Transform skip related parameters

    int slcQP = m_QP;

    int qpIdx = 0;
    if (slcQP <= 22)
    {
        qpIdx = 0;
    }
    else if (slcQP <= 27)
    {
        qpIdx = 1;
    }
    else if (slcQP <= 32)
    {
        qpIdx = 2;
    }
    else
    {
        qpIdx = 3;
    }

    auto settings = static_cast<PreEncFeatureSettings *>(m_preEncConstSettings->GetConstSettings());
    ENCODE_CHK_NULL_RETURN(settings);

    params.transformskiplambda = settings->transformSkipLambdaTable[slcQP];

    if (m_pictureCodingType == I_TYPE)
    {
        params.transformskipNumzerocoeffsFactor0    = settings->transformSkipCoeffsTable[qpIdx][0][0][0][0];
        params.transformskipNumzerocoeffsFactor1    = settings->transformSkipCoeffsTable[qpIdx][0][0][1][0];
        params.transformskipNumnonzerocoeffsFactor0 = settings->transformSkipCoeffsTable[qpIdx][0][0][0][1] + 32;
        params.transformskipNumnonzerocoeffsFactor1 = settings->transformSkipCoeffsTable[qpIdx][0][0][1][1] + 32;
    }
    else
    {
        params.transformskipNumzerocoeffsFactor0    = settings->transformSkipCoeffsTable[qpIdx][1][0][0][0];
        params.transformskipNumzerocoeffsFactor1    = settings->transformSkipCoeffsTable[qpIdx][1][0][1][0];
        params.transformskipNumnonzerocoeffsFactor0 = settings->transformSkipCoeffsTable[qpIdx][1][0][0][1] + 32;
        params.transformskipNumnonzerocoeffsFactor1 = settings->transformSkipCoeffsTable[qpIdx][1][0][1][1] + 32;
    }

    params.roundinter = 4;
    params.roundintra = 10;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, PreEncBasicFeature)
{
    PMOS_SURFACE psSurface            = nullptr;
    uint8_t      ucBitDepthLumaMinus8 = 0;
    uint8_t      chromaType           = 0;
    uint32_t     reconSurfHeight      = 0;

    ucBitDepthLumaMinus8 = m_preEncConfig.BitDepthLumaMinus8;
    chromaType           = m_outputChromaFormat;
    switch (params.surfaceStateId)
    {
    case CODECHAL_HCP_SRC_SURFACE_ID:
        psSurface = m_preEncRawSurface;
        break;
    case CODECHAL_HCP_DECODED_SURFACE_ID:
        psSurface       = const_cast<PMOS_SURFACE>(&m_reconSurface);
        reconSurfHeight = m_preEncRawSurface->dwHeight;
        break;
    case CODECHAL_HCP_REF_SURFACE_ID:
        psSurface       = const_cast<PMOS_SURFACE>(&m_reconSurface);
        reconSurfHeight = m_preEncRawSurface->dwHeight;
        break;
    }

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
        params.yOffsetForUCbInPixel = params.yOffsetForVCr = (uint16_t)reconSurfHeight;
    }

    return MOS_STATUS_SUCCESS;
}


#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS PreEncBasicFeature::EncodePreencBasicFuntion1()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    MediaUserSetting::Value outValue;
    outValue = std::string();
    eStatus = ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Preenc file0 path",
        MediaUserSetting::Group::Sequence);

    int size = outValue.Size();
    if (eStatus != MOS_STATUS_SUCCESS)
        size = 0;

    if (size == MOS_MAX_PATH_LENGTH + 1)
    {
        size = 0;
    }

    ENCODE_CHK_COND_RETURN(size == 0, "PATH LENGTH OF FILE IS TOO LONG");

    std::string path_file = outValue.Get<std::string>();
    if (path_file[size - 2] != MOS_DIRECTORY_DELIMITER)
    {
        path_file.push_back('\0');
    }
    std::string filePath0 = path_file;

    outValue = std::string();
    eStatus = ReadUserSetting(
        m_userSettingPtr,
        outValue,
        "Preenc file1 path",
        MediaUserSetting::Group::Sequence);

    size = outValue.Size();

    if (eStatus != MOS_STATUS_SUCCESS)
        size = 0;

    if (size == MOS_MAX_PATH_LENGTH + 1)
    {
        size = 0;
    }

    ENCODE_CHK_COND_RETURN(size == 0, "PATH LENGTH OF FILE IS TOO LONG");

    path_file = outValue.Get<std::string>();
    if (path_file[size - 2] != MOS_DIRECTORY_DELIMITER)
    {
        path_file.push_back('\0');
    }
    std::string filePath1 = path_file;

    if (m_preEncConfig.CodingType > I_TYPE)
    {
        void *data0 = m_allocator->LockResourceForRead(EncodePreencBasicMember3);

        if (pfile0 == nullptr)
        {
            MosUtilities::MosSecureFileOpen(&pfile0, filePath0.c_str(), "wb");
        }

        if (pfile0 != nullptr)
        {
            fwrite(data0, sizeof(EncodePreencDef1), EncodePreencBasicMember2, pfile0);
        }

        eStatus = m_allocator->UnLock(EncodePreencBasicMember3);
    }

    if (m_preEncConfig.CodingType != I_TYPE && !IsLowDelay())
    {
        void *data1 = m_allocator->LockResourceForRead(EncodePreencBasicMember4);

        if (pfile1 == nullptr)
        {
            MosUtilities::MosSecureFileOpen(&pfile1, filePath1.c_str(), "wb");
        }

        if (pfile1 != nullptr)
        {
            fwrite(data1, sizeof(EncodePreencDef1), EncodePreencBasicMember2, pfile1);
        }

        eStatus = m_allocator->UnLock(EncodePreencBasicMember4);
    }

    return eStatus;
}
#endif

}  // namespace encode
