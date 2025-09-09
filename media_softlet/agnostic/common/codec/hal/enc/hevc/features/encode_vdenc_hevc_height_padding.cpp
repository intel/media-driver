/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     encode_hevc_vdenc_height_padding.cpp
//! \brief    Defines the Xe3_LPM+ Base interface for hevc encode height padding features
//!

#include "encode_vdenc_hevc_height_padding.h"
#include "encode_hevc_vdenc_feature_manager.h"

namespace encode
{

MOS_STATUS HevcVdencHeightPadding::Update(void *params)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(params);
    EncoderParams *encodeParams = static_cast<EncoderParams *>(params);
    m_hevcSeqParams             = static_cast<PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS>(encodeParams->pSeqParams);
    ENCODE_CHK_NULL_RETURN(m_hevcSeqParams);
    m_hevcPicParams = static_cast<PCODEC_HEVC_ENCODE_PICTURE_PARAMS>(encodeParams->pPicParams);
    ENCODE_CHK_NULL_RETURN(m_hevcPicParams);

    uint32_t oriHeight = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3);
    m_enabled          = oriHeight % 8 == 0 && oriHeight % 16 != 0 
        && m_hevcSeqParams->ScenarioInfo == ESCENARIO_VIDEOCONFERENCE 
        && !m_hevcPicParams->tiles_enabled_flag 
        && m_hevcPicParams->TargetFrameSize == 0;
    if (m_enabled)
    {
        ENCODE_CHK_NULL_RETURN(encodeParams->psRawSurface);
        PMOS_SURFACE rawSurface = encodeParams->psRawSurface;
        ENCODE_CHK_NULL_RETURN(encodeParams->psReconSurface);
        PMOS_SURFACE reconSurface = encodeParams->psReconSurface;  
        m_alignedHeight = MOS_ALIGN_CEIL(oriHeight, 16);
        ENCODE_CHK_NULL_RETURN(m_allocator);
        ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(rawSurface));
        ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(reconSurface));
        if (rawSurface->Format != Format_NV12 || rawSurface->YoffsetForUplane < m_alignedHeight || reconSurface->YoffsetForUplane < m_alignedHeight)
        {
            m_enabled = false;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcVdencHeightPadding)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.height = m_alignedHeight;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcVdencHeightPadding)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    // currently no tiling supports
    params.tileHeight = m_alignedHeight;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcVdencHeightPadding)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.frameheightinmincbminus1 = (m_alignedHeight >> (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) - 1;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(AQM_PIC_STATE, HevcVdencHeightPadding)
{
    ENCODE_FUNC_CALL();

    if (!m_enabled)
    {
        return MOS_STATUS_SUCCESS;
    }

    params.FrameHeightInPixelMinus1 = m_alignedHeight - 1;

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
