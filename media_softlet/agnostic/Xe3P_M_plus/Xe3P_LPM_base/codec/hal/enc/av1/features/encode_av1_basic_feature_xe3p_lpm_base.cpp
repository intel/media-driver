/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_av1_basic_feature_Xe3P_Lpm_base.cpp
//! \brief    Defines the Xe3P_LPM_Base common class for encode av1 basic feature
//!

#include "encode_av1_basic_feature_xe3p_lpm_base.h"
#include "encode_av1_vdenc_const_settings_xe3p_lpm_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_scc.h"

using namespace mhw::vdbox;

namespace encode
{
Av1BasicFeatureXe3P_Lpm_Base::~Av1BasicFeatureXe3P_Lpm_Base()
{
    ENCODE_FUNC_CALL();
    
    if (m_hwInterface != nullptr)
    {
        for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
        {
            MOS_STATUS eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_vdenc2ndLevelBatchBuffer[j], nullptr);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to free m_vdenc2ndLevelBatchBuffer[%d], status = 0x%x", j, eStatus);
            }
            eStatus = Mhw_FreeBb(m_hwInterface->GetOsInterface(), &m_vdenc2ndLevelBatchBufferTU7[j], nullptr);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                ENCODE_ASSERTMESSAGE("Failed to free m_vdenc2ndLevelBatchBufferTU7[%d], status = 0x%x", j, eStatus);
            }
        }
    }
}

MOS_STATUS Av1BasicFeatureXe3P_Lpm_Base::UpdateFormat(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS  av1SeqParams  = nullptr;

    av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    if (m_chromaFormat != AVP_CHROMA_FORMAT_YUV420 && m_chromaFormat != AVP_CHROMA_FORMAT_YUV444
        && m_chromaFormat != AVP_CHROMA_FORMAT_YUV422)
    {
        ENCODE_ASSERTMESSAGE("Invalid output chromat format!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_outputChromaFormat = m_chromaFormat;

    // Set surface bit info according to surface format.
    switch (m_rawSurface.Format)
    {
    case Format_P010:
    case Format_Y410:
    case Format_Y210:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        m_is10Bit  = true;
        m_bitDepth = 10;
        break;
    case Format_NV12:
    case Format_AYUV:
    case Format_YUY2:
        m_is10Bit  = false;
        m_bitDepth = 8;
        break;
    default:
        m_is10Bit  = false;
        m_bitDepth = 8;
        break;
    }

    if (m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV422)
    {
        ENCODE_CHK_COND_RETURN(
            m_rawSurface.Format != Format_YUY2 && m_rawSurface.Format != Format_Y210,
            "4:2:2 only supports YUY2 and Y210 as input!");

        ENCODE_CHK_COND_RETURN(
            m_reconSurface.Format != Format_YUY2 && m_reconSurface.Format != Format_Y210 && m_reconSurface.Format != Format_Y216,
            "4:2:2 recon surface must be YUY2, Y210 or Y216!");

        ENCODE_CHK_COND_RETURN(
            m_reconSurface.dwPitch != MOS_ALIGN_CEIL(m_reconSurface.dwPitch, 128),
            "4:2:2 recon surface pitch (%d) is not 128 aligned!",
            m_reconSurface.dwPitch);

        ENCODE_CHK_COND_RETURN(
            m_reconSurface.dwHeight != MOS_ALIGN_CEIL(m_reconSurface.dwHeight, 8),
            "4:2:2 recon surface height (%d) is not 8 aligned!",
            m_reconSurface.dwHeight);

        ENCODE_CHK_COND_RETURN(
            m_reconSurface.Format == Format_YUY2 && m_reconSurface.dwPitch < m_rawSurface.dwWidth,
            "4:2:2 YUY2 recon surface pitch (%d) should >= input surface width (%d)!",
            m_reconSurface.dwPitch,
            m_rawSurface.dwWidth);

        ENCODE_CHK_COND_RETURN(
            (m_reconSurface.Format == Format_Y216 || m_reconSurface.Format == Format_Y210) && m_reconSurface.dwPitch < 2 * m_rawSurface.dwWidth,
            "4:2:2 Y216/Y210 recon surface pitch (%d) should >= 2 x input surface width (%d)!",
            m_reconSurface.dwPitch,
            m_rawSurface.dwWidth);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeatureXe3P_Lpm_Base::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::Update(params));

    auto superResFeature = dynamic_cast<Av1SuperRes *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SuperRes));
    ENCODE_CHK_NULL_RETURN(superResFeature);
    if (superResFeature->IsEnabled())
    {
        m_rawSurfaceToEnc          = superResFeature->GetRawSurfaceToEnc();
        m_postCdefReconSurfaceFlag = true;
    }

    if (m_roundingMethod == RoundingMethod::adaptiveRounding)
    {
        m_roundingMethod = RoundingMethod::lookUpTableRounding;
    }

    ENCODE_CHK_COND_RETURN(
        m_av1PicParams->PicFlags.fields.use_superres &&
            (m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV444 || m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV422),
        "Super-res does not support 4:4:4 or 4:2:2!");

    ENCODE_CHK_COND_RETURN(
        m_av1SeqParams->CodingToolFlags.fields.enable_restoration &&
            (m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV444 || m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV422),
        "Loop restoration does not support 4:4:4 or 4:2:2!");

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeatureXe3P_Lpm_Base::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::Init(setting));
    ENCODE_CHK_NULL_RETURN(m_hwInterface);

    auto avpItf   = std::static_pointer_cast<mhw::vdbox::avp::Itf>(m_hwInterface->GetAvpInterfaceNext());
    auto vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
    auto miItf    = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
    ENCODE_CHK_NULL_RETURN(avpItf);
    ENCODE_CHK_NULL_RETURN(vdencItf);
    ENCODE_CHK_NULL_RETURN(miItf);

    uint32_t bbEndSize = miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    // Group1: AVP_SEGMENT_STATE × av1MaxSegments + AVP_INLOOP_FILTER_STATE + BB_END
    uint32_t group1Size = av1MaxSegments * avpItf->MHW_GETSIZE_F(AVP_SEGMENT_STATE)()
        + avpItf->MHW_GETSIZE_F(AVP_INLOOP_FILTER_STATE)()
        + bbEndSize;

    // Group2: VDENC_CMD1 + BB_END
    uint32_t group2Size = vdencItf->MHW_GETSIZE_F(VDENC_CMD1)() + bbEndSize;

    // Group3: VDENC_CMD2 + BB_END
    uint32_t group3Size = vdencItf->MHW_GETSIZE_F(VDENC_CMD2)() + bbEndSize;

    // Group4: (AVP_PIC_STATE + BB_END) × max tile columns
    uint32_t group4Size = av1MaxTileColumn
        * (avpItf->MHW_GETSIZE_F(AVP_PIC_STATE)() + bbEndSize);

    // Group5: (VDENC_HEVC_VP9_TILE_SLICE_STATE + BB_END) × max tile columns (palette mode)
    uint32_t group5Size = av1MaxTileColumn
        * (vdencItf->MHW_GETSIZE_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)() + bbEndSize);

    m_slbbBufferSize = MOS_ALIGN_CEIL(
        group1Size + group2Size + group3Size + group4Size + group5Size,
        CODECHAL_PAGE_SIZE);

    ENCODE_CHK_STATUS_RETURN(AllocateVdencBatchBuffers());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1BasicFeatureXe3P_Lpm_Base::AllocateVdencBatchBuffers()
{
    ENCODE_FUNC_CALL();

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
        {
            // VDEnc read batch buffer Origin
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_slbbBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer Origin";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            MOS_RESOURCE *allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencReadBatchBufferOrigin[k][i] = *allocatedbuffer;

            // VDEnc read batch buffer TU7
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_slbbBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer TU7";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencReadBatchBufferTU7[k][i] = *allocatedbuffer;
        }
    }

    // VDEnc 2nd level batch buffer for SLBB update output
    for (auto j = 0; j < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; j++)
    {
        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBuffer[j], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBuffer[j].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_vdenc2ndLevelBatchBuffer[j],
            nullptr,
            m_slbbBufferSize));

        MOS_ZeroMemory(&m_vdenc2ndLevelBatchBufferTU7[j], sizeof(MHW_BATCH_BUFFER));
        m_vdenc2ndLevelBatchBufferTU7[j].bSecondLevel = true;
        ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_hwInterface->GetOsInterface(),
            &m_vdenc2ndLevelBatchBufferTU7[j],
            nullptr,
            m_slbbBufferSize));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_RESOURCE* Av1BasicFeatureXe3P_Lpm_Base::GetVdencReadBatchBufferOrigin(uint32_t recycledBufIdx, uint32_t brcPass)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM || brcPass >= VDENC_BRC_NUM_OF_PASSES)
    {
        return nullptr;
    }
    return &m_vdencReadBatchBufferOrigin[recycledBufIdx][brcPass];
}

MOS_RESOURCE* Av1BasicFeatureXe3P_Lpm_Base::GetVdencReadBatchBufferTU7(uint32_t recycledBufIdx, uint32_t brcPass)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM || brcPass >= VDENC_BRC_NUM_OF_PASSES)
    {
        return nullptr;
    }
    return &m_vdencReadBatchBufferTU7[recycledBufIdx][brcPass];
}

MHW_BATCH_BUFFER* Av1BasicFeatureXe3P_Lpm_Base::GetVdenc2ndLevelBatchBuffer(uint32_t recycledBufIdx)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
    {
        return nullptr;
    }
    return &m_vdenc2ndLevelBatchBuffer[recycledBufIdx];
}

MHW_BATCH_BUFFER* Av1BasicFeatureXe3P_Lpm_Base::GetVdenc2ndLevelBatchBufferTU7(uint32_t recycledBufIdx)
{
    if (recycledBufIdx >= CODECHAL_ENCODE_RECYCLED_BUFFER_NUM)
    {
        return nullptr;
    }
    return &m_vdenc2ndLevelBatchBufferTU7[recycledBufIdx];
}

MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1BasicFeatureXe3P_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_SURFACE_STATE)(params));

    if (m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV420)
    {
        if (!m_is10Bit)
        {
            params.srcFormat = avp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
        }
        else
        {
            if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
            {
                params.srcFormat = avp::SURFACE_FORMAT::SURFACE_FORMAT_P010;
            }
            else
            {
                params.srcFormat = avp::SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
            }
        }
    }
#ifdef _MEDIA_RESERVED
    else if (m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV444)
    {
        if (!m_is10Bit)
        {
            if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_AYUV4444FORMAT);
            }
            else
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_AYUVVARIANT);
                params.pitch     = m_reconSurface.dwPitch / 4;
                params.uOffset   = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
                params.vOffset   = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8) << 1;
            }
        }
        else
        {
            if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_Y410FORMAT);
            }
            else
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_Y410VARIANT);
                params.pitch     = m_reconSurface.dwPitch / 2;
                params.uOffset   = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
                params.vOffset   = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8) << 1;
            }
        }
    }
    else if (m_outputChromaFormat == AVP_CHROMA_FORMAT_YUV422)
    {
        if (!m_is10Bit)
        {
            if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_YUY2FORMAT);
            }
            else
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_YUY2VARIANT);
                params.uOffset = params.vOffset = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
            }
        }
        else
        {
            if (params.surfaceStateId == srcInputPic || params.surfaceStateId == origUpscaledSrc)
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_Y216Y210FORMAT);
            }
            else
            {
                params.srcFormat = static_cast<avp::SURFACE_FORMAT>(avp::SURFACE_FORMAT_EXT::SURFACE_FORMAT_Y210VARIANT);
                params.uOffset = params.vOffset = MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
            }
        }
    }
#endif
    else
    {
        ENCODE_ASSERTMESSAGE("Invalid output chromat format!");
    }

    return MOS_STATUS_SUCCESS;
}

#if _MEDIA_RESERVED
#include "encode_av1_basic_feature_ext.h"
#else
static uint32_t ComputeRdMult(uint16_t qIndex, bool is10Bit)
{
    return RdMultLUT[is10Bit][qIndex];
}
#endif

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1BasicFeatureXe3P_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_PIC_STATE)(params));

    params.enableIntraEdgeFilter = m_av1SeqParams->CodingToolFlags.fields.enable_intra_edge_filter;

    params.rdmult = ComputeRdMult(params.baseQindex, m_is10Bit);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_INLOOP_FILTER_STATE, Av1BasicFeatureXe3P_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_INLOOP_FILTER_STATE)(params));

    bool enableCDEF = !(IsFrameLossless(*m_av1PicParams) || m_av1PicParams->PicFlags.fields.allow_intrabc
        || !(m_av1SeqParams->CodingToolFlags.fields.enable_cdef));

    //restriction: When CDEF filter is disabled, all its filter parameters should be reset to 0.
    if (!enableCDEF)
    {
        memset(params.cdefYStrength,  0, sizeof(params.cdefYStrength));
        memset(params.cdefUVStrength, 0, sizeof(params.cdefUVStrength));
        params.cdefBits          = 0;
        params.cdefDampingMinus3 = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1BasicFeatureXe3P_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params));

    params.VdencPipeModeSelectPar8 = 1;

    params.chromaPrefetchDisable = m_chromaPrefetchDisable;

    params.verticalShift32Minus1 = 0;
    params.numVerticalReqMinus1  = 11;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
