/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_av1_basic_feature_xe_hpm.cpp
//! \brief    Defines the common interface for encode av1 parameter
//!

#include "encode_av1_basic_feature_xe_hpm.h"
#include "mos_solo_generic.h"

namespace encode
{

MOS_STATUS Av1BasicFeatureXe_Hpm::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::Update(params));

    ENCODE_CHK_STATUS_RETURN(m_ref.SetPostCdefAsEncRef(true));

    m_postCdefReconSurfaceFlag = true;

    return MOS_STATUS_SUCCESS;

}

MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1BasicFeatureXe_Hpm)
{
    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(AVP_SURFACE_STATE)(params));

    if (!m_is10Bit)
    {
        params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
    }
    else
    {
        if (params.surfaceStateId == av1CdefPixelsStreamout)
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010VARIANT;
        }
        else
        {
            params.srcFormat = mhw::vdbox::avp::SURFACE_FORMAT::SURFACE_FORMAT_P010;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, Av1BasicFeatureXe_Hpm)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(VDENC_PIPE_MODE_SELECT)(params));

    params.hmeRegionPrefetch     = 1;
    params.VdencPipeModeSelectPar0 = 1;

    MEDIA_WA_TABLE* pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_CHK_NULL_RETURN(pWaTable);
    if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && !m_osInterface->bSimIsActive && !Mos_Solo_Extension(m_osInterface->pOsContext))
    {
        params.hmeRegionPrefetch = !(m_av1PicParams->PicFlags.fields.frame_type == keyFrame);
    }

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, Av1BasicFeatureXe_Hpm)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(VDENC_HEVC_VP9_TILE_SLICE_STATE)(params));

    params.VdencHEVCVP9TileSlicePar12    = 0x3f;
    params.VdencHEVCVP9TileSlicePar13    = 2;
    params.VdencHEVCVP9TileSlicePar17[0] = 0x3f;
    params.VdencHEVCVP9TileSlicePar17[1] = 0x3f;
    params.VdencHEVCVP9TileSlicePar17[2] = 0x3f;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, Av1BasicFeatureXe_Hpm)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(Av1BasicFeature::MHW_SETPAR_F(VDENC_PIPE_BUF_ADDR_STATE)(params));

    MEDIA_WA_TABLE* pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_CHK_NULL_RETURN(pWaTable);

    const auto frame_type = static_cast<Av1FrameType>(m_av1PicParams->PicFlags.fields.frame_type);
    if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && frame_type == keyFrame && !this->m_osInterface->bSimIsActive && !Mos_Solo_Extension(this->m_osInterface->pOsContext))
    {
        params.refsDsStage1[0] = &m_8xDSSurface->OsResource;
        params.refsDsStage2[0] = &m_4xDSSurface->OsResource;
        params.numActiveRefL0  = 1;
    }

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
