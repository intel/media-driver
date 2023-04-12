/*
* Copyright (c) 2020 - 2023, Intel Corporation
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
//! \file     encode_av1_superres.cpp
//! \brief    Super-res feature
//!

#include "encode_av1_superres.h"
#include "encode_av1_tile.h"

namespace encode
{
Av1SuperRes::Av1SuperRes(MediaFeatureManager *featureManager,
    EncodeAllocator                          *allocator,
    void                                     *constSettings):
    MediaFeature(constSettings)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_NO_STATUS_RETURN(featureManager);
    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);

    m_allocator = allocator;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_allocator);
}

MOS_STATUS Av1SuperRes::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    m_trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_RETURN(m_trackedBuf);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperRes::Update(void *params)
{
    ENCODE_FUNC_CALL();

    EncoderParams *encodeParams = (EncoderParams *)params;
    ENCODE_CHK_NULL_RETURN(encodeParams);

    auto av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    auto av1PicParams = static_cast<PCODEC_AV1_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    m_oriFrameHeight       = av1PicParams->frame_height_minus1 + 1;
    m_oriAlignedFrameWidth = av1PicParams->frame_width_minus1 + 1;

    uint16_t picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
    m_frameHeight          = picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    m_enabled     = av1SeqParams->CodingToolFlags.fields.enable_superres;
    m_useSuperRes = av1PicParams->PicFlags.fields.use_superres;

    ENCODE_CHK_COND_RETURN(!m_enabled && m_useSuperRes == true, "Super-res disabled in SPS, but enabled in PPS!");
    ENCODE_CHK_COND_RETURN(m_enabled && !m_useSuperRes && av1PicParams->superres_scale_denominator != av1ScaleNumerator, "Super-res not used in current frame, but scale denominator is not 8!");

    if (m_enabled && m_useSuperRes)
    {
        m_superResDenom = av1PicParams->superres_scale_denominator;
        ENCODE_CHK_COND_RETURN(m_superResDenom < 9 || m_superResDenom > 16, "Wrong value of super-res denominator, should be in range [9, 16]!");

        // HW restriction of SuperRes denominator for SuperRes + LoopRestoration
        ENCODE_CHK_COND_RETURN(av1SeqParams->CodingToolFlags.fields.enable_restoration && m_superResDenom % 2 == 1, "When both SuperRes and LoopRestoration are enabled, only valid SuperRes denominator values are 10, 12, 14 and 16");

        m_frameWidthDs = ((m_oriAlignedFrameWidth << 3) + (m_superResDenom >> 1)) / m_superResDenom;

        // update DDI width so that subsequent features will use downscaled width
        av1PicParams->frame_width_minus1 = m_frameWidthDs - 1;
    }
    else
    {
        m_frameWidthDs  = m_oriAlignedFrameWidth;
        m_superResDenom = av1ScaleNumerator;
    }

    int8_t subsamplingX = /*SequenceChromaSubSamplingFormat != chromaIdc444 ? 1 : 0*/ 1;
    m_subsamplingX[0]   = 0;
    m_subsamplingX[1]   = subsamplingX;
    m_subsamplingX[2]   = subsamplingX;

    ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.UpdateRefFrameSize(m_oriAlignedFrameWidth, av1PicParams->frame_height_minus1 + 1));

    if (m_enabled)
    {
        m_basicFeature->m_ref.SetPostCdefAsEncRef(true);

        m_widthChanged = m_prevDsWidth != av1PicParams->frame_width_minus1 + 1;

        ENCODE_CHK_NULL_RETURN(encodeParams->psRawSurface);
        m_raw.resource = encodeParams->psRawSurface;
        m_allocator->GetSurfaceInfo(m_raw.resource);

        ENCODE_CHK_STATUS_RETURN(PrepareRawSurface());

        PrepareVeSfcDownscalingParam(m_raw, m_rawDs, m_downScalingParams);

        m_prevDsWidth = m_frameWidthDs;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperRes::InitMMCState(EncodeMemComp *mmcState)
{
    ENCODE_CHK_NULL_RETURN(mmcState);
    m_mmcState = mmcState;
    return MOS_STATUS_SUCCESS;
}

static inline MOS_STATUS SetSurfaceMMCParams(EncodeMemComp &mmcState, MOS_SURFACE &surf)
{
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcMode(&surf));
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcState(&surf));
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcFormat(&surf));
    surf.bIsCompressed = surf.CompressionMode != MOS_MMC_DISABLED;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperRes::PrepareRawSurface()
{
    ENCODE_FUNC_CALL();

    if (m_enabled)
    {
        m_raw.unalignedWidth  = GetUpscaledWidth();
        m_raw.unalignedHeight = m_oriFrameHeight;

        ENCODE_CHK_NULL_RETURN(m_mmcState);

        if (m_widthChanged)
        {
            if (!Mos_ResourceIsNull(&m_rawDs.resource->OsResource))
            {
                m_allocator->DestroySurface(m_rawDs.resource);
            }

            MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
            MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));

            allocParamsForBuffer2D.Type         = MOS_GFXRES_2D;
            allocParamsForBuffer2D.TileType     = MOS_TILE_Y;
            allocParamsForBuffer2D.Format       = m_raw.resource->Format;
            allocParamsForBuffer2D.dwWidth      = MOS_ALIGN_CEIL(m_frameWidthDs, av1SuperBlockWidth);
            allocParamsForBuffer2D.dwHeight     = MOS_ALIGN_CEIL(m_frameHeight, av1SuperBlockHeight);
            allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            allocParamsForBuffer2D.pBufName     = "superResEncRawSurface";

            if (m_mmcState->IsMmcEnabled())
            {
                allocParamsForBuffer2D.CompressionMode = MOS_MMC_MC;
                allocParamsForBuffer2D.bIsCompressible = true;
            }

            m_rawDs.resource = m_allocator->AllocateSurface(allocParamsForBuffer2D, false);
            ENCODE_CHK_NULL_RETURN(m_rawDs.resource);
            ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_rawDs.resource));

            m_rawDs.unalignedWidth  = m_frameWidthDs;
            m_rawDs.unalignedHeight = m_oriFrameHeight;
        }

        if (m_mmcState->IsMmcEnabled())
        {
            SetSurfaceMMCParams(*m_mmcState, *m_raw.resource);
            SetSurfaceMMCParams(*m_mmcState, *m_rawDs.resource);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperRes::PrepareVeSfcDownscalingParam(const SURFACE &inSurf, const SURFACE &outSurf, VEBOX_SFC_PARAMS &params)
{
    ENCODE_FUNC_CALL();

    params.input.surface      = inSurf.resource;
    params.input.chromaSiting = 0;
    params.input.rcSrc        = {0, 0, (long)inSurf.unalignedWidth, (long)inSurf.unalignedHeight};
    params.input.rotation     = ROTATION_IDENTITY;

    params.output.surface      = outSurf.resource;
    params.output.chromaSiting = 0;
    params.output.rcDst        = {0, 0, (long)outSurf.unalignedWidth, (long)outSurf.unalignedHeight};

    switch (inSurf.resource->Format)
    {
    case Format_NV12:
    case Format_P010:
        params.input.colorSpace  = CSpace_Any;
        params.output.colorSpace = CSpace_Any;
        break;
    case Format_A8R8G8B8:
    case Format_A8B8G8R8:
        params.input.colorSpace  = CSpace_sRGB;
        params.output.colorSpace = CSpace_sRGB;
        break;
    default:
        params.input.colorSpace  = CSpace_Any;
        params.output.colorSpace = CSpace_Any;
        break;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1SuperRes)
{
    params.enableSuperres = m_useSuperRes;

    return MOS_STATUS_SUCCESS;
}

static constexpr int32_t RS_SUBPEL_BITS       = 6;
static constexpr int32_t RS_SCALE_SUBPEL_BITS = 14;
static constexpr int32_t RS_SCALE_EXTRA_BITS  = RS_SCALE_SUBPEL_BITS - RS_SUBPEL_BITS;
static constexpr int32_t RS_SCALE_EXTRA_OFF   = 1 << (RS_SCALE_EXTRA_BITS - 1);
static constexpr int32_t RS_SCALE_SUBPEL_MASK = (1 << RS_SCALE_SUBPEL_BITS) - 1;

static inline int32_t GetUpscaleConvolveStep(int32_t inLength, int32_t outLength)
{
    return ((inLength << RS_SCALE_SUBPEL_BITS) + outLength / 2) / outLength;
}

static inline int32_t GetUpscaleConvolveX0(int32_t inLength, int32_t outLength, int32_t xStepQn)
{
    const int32_t err = outLength * xStepQn - (inLength << RS_SCALE_SUBPEL_BITS);
    const int32_t x0  = (-((outLength - inLength) << (RS_SCALE_SUBPEL_BITS - 1)) + outLength / 2) / outLength +
                       RS_SCALE_EXTRA_OFF - err / 2;

    return static_cast<int32_t>(static_cast<uint32_t>(x0) & RS_SCALE_SUBPEL_MASK);
}

MHW_SETPAR_DECL_SRC(AVP_INLOOP_FILTER_STATE, Av1SuperRes)
{
    params.superresUpscaledWidthMinus1 = m_oriAlignedFrameWidth - 1;
    params.superresDenom               = m_superResDenom;

    int32_t xStepQn[3];
    int32_t x0Qn[3];

    if (m_enabled && m_useSuperRes)
    {
        for (int32_t plane = 0; plane < 2; plane++)
        {
            int32_t downscaledPlaneWidth = ROUND_POWER_OF_TWO(m_frameWidthDs, m_subsamplingX[plane]);
            int32_t upscaledPlaneWidth   = ROUND_POWER_OF_TWO(m_oriAlignedFrameWidth, m_subsamplingX[plane]);

            xStepQn[plane] = GetUpscaleConvolveStep(downscaledPlaneWidth, upscaledPlaneWidth);
            x0Qn[plane]    = GetUpscaleConvolveX0(downscaledPlaneWidth, upscaledPlaneWidth, xStepQn[plane]);
        }

        xStepQn[2] = xStepQn[1];
        x0Qn[2]    = x0Qn[1];

    params.lumaPlaneXStepQn   = xStepQn[0];
    params.chromaPlaneXStepQn = xStepQn[1];
    }

    auto tileFeature = dynamic_cast<Av1EncodeTile *>(m_featureManager->GetFeature(Av1FeatureIDs::encodeTile));
    ENCODE_CHK_NULL_RETURN(tileFeature);

    EncodeTileData tileData = {};
    tileFeature->GetCurrentTile(tileData);

    if (m_enabled && m_useSuperRes)
    {
        /* This calculation is the same with CModel but will cause hang issue for multi-tile tests on EMU
        params.lumaPlaneX0Qn   = x0Qn[0] + params.lumaPlaneXStepQn * lumaXPos * m_superResDenom / av1ScaleNumerator;
        params.chromaPlaneX0Qn = x0Qn[1] + params.chromaPlaneXStepQn * chromaXPos * m_superResDenom / av1ScaleNumerator;
        */
        params.lumaPlaneX0Qn   = x0Qn[0];
        params.chromaPlaneX0Qn = x0Qn[1];
    }

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
