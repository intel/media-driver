/*
* Copyright (c) 2022 - 2025, Intel Corporation
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
//! \file     encode_av1_basic_feature_Xe3_Lpm_base.cpp
//! \brief    Defines the Xe3_LPM+ common class for encode av1 basic feature
//!

#include "encode_av1_basic_feature_xe3_lpm_base.h"
#include "encode_av1_vdenc_const_settings_xe3_lpm_base.h"
#include "encode_av1_superres.h"
#include "encode_av1_scc.h"

using namespace mhw::vdbox;

namespace encode
{
MOS_STATUS Av1BasicFeatureXe3_Lpm_Base::UpdateFormat(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = (EncoderParams *)params;

    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS  av1SeqParams  = nullptr;

    av1SeqParams = static_cast<PCODEC_AV1_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(av1SeqParams);

    if (m_chromaFormat != AVP_CHROMA_FORMAT_YUV420 && m_chromaFormat != AVP_CHROMA_FORMAT_YUV444)
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
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        m_is10Bit  = true;
        m_bitDepth = 10;
        break;
    case Format_NV12:
    case Format_AYUV:
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

MOS_STATUS Av1BasicFeatureXe3_Lpm_Base::Update(void *params)
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

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, Av1BasicFeatureXe3_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(VDENC_CMD2)(params));

#if _MEDIA_RESERVED
    if (IsFrameLossless(*m_av1PicParams))
    {
        params.vdencCmd2Par156 = 0;
        params.vdencCmd2Par101 = 0;
        params.vdencCmd2Par142 = 1;
        params.vdencCmd2Par143 = 1;
        params.vdencCmd2Par144 = 1;
    }

    if (m_chromaFormat == AVP_CHROMA_FORMAT_YUV444)
    {
        params.vdencCmd2Par102 = false;
        params.vdencCmd2Par101 = false;
    }
#else
    params.extSettings.emplace_back([this](uint32_t *data){
        if (IsFrameLossless(*m_av1PicParams))
        {
            data[64]               = data[64] & 0xfffffffc;
            data[54]               = data[54] & 0xffffffbf;
            data[63]               = (data[63] & 0xff00ffff) | 0x10000;
            data[63]               = (data[63] & 0xffff00ff) | 0x100;
            data[63]               = (data[63] & 0xffffff00) | 0x1;
        }

        if (m_chromaFormat == AVP_CHROMA_FORMAT_YUV444)
        {
            data[54]               = data[54] & 0xffffff7f;
            data[54]               = data[54] & 0xffffffbf;
        }

        return MOS_STATUS_SUCCESS;
    });
#endif

    params.av1EnableIntraEdgeFilter = m_av1SeqParams->CodingToolFlags.fields.enable_intra_edge_filter;

    return MOS_STATUS_SUCCESS;
}


MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1BasicFeatureXe3_Lpm_Base)
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
                params.uOffset   = (uint16_t)MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
                params.vOffset   = (uint16_t)MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8) << 1;
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
                params.uOffset   = (uint16_t)MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8);
                params.vOffset   = (uint16_t)MOS_ALIGN_CEIL(m_rawSurfaceToPak->dwHeight, 8) << 1;
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

MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1BasicFeatureXe3_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_PIC_STATE)(params));

    params.enableIntraEdgeFilter = m_av1SeqParams->CodingToolFlags.fields.enable_intra_edge_filter;

    params.rdmult = ComputeRdMult(params.baseQindex, m_is10Bit);

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AVP_INLOOP_FILTER_STATE, Av1BasicFeatureXe3_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_INLOOP_FILTER_STATE)(params));

    bool enableCDEF = !(IsFrameLossless(*m_av1PicParams) || m_av1PicParams->PicFlags.fields.allow_intrabc
        || !(m_av1SeqParams->CodingToolFlags.fields.enable_cdef));

    if (!enableCDEF)
    {
        memset(params.cdefYStrength,  0, sizeof(params.cdefYStrength));
        memset(params.cdefUVStrength, 0, sizeof(params.cdefUVStrength));
        params.cdefBits          = 0;
        params.cdefDampingMinus3 = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1BasicFeatureXe3_Lpm_Base)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params));

    params.VdencPipeModeSelectPar8 = 1;

    params.chromaPrefetchDisable = m_chromaPrefetchDisable;

    params.verticalShift32Minus1 = 0;
    params.numVerticalReqMinus1 = 11;

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
