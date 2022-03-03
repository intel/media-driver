/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_copy_xe_xpm_base.cpp
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#include "media_copy_xe_xpm_base.h"

MediaCopyStateXe_Xpm_Base::MediaCopyStateXe_Xpm_Base() :
    MediaCopyBaseState()
{

}

MOS_STATUS MediaCopyStateXe_Xpm_Base::Initialize(  PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MCPY_CHK_NULL_RETURN(osInterface);
    MCPY_CHK_NULL_RETURN(mhwInterfaces);

    m_osInterface   = osInterface;
    m_mhwInterfaces = mhwInterfaces;

    MCPY_CHK_STATUS_RETURN(MediaCopyBaseState::Initialize(osInterface, mhwInterfaces));

    // blt init
    m_bltState = MOS_New(BltStateXe_Xpm, m_osInterface, m_mhwInterfaces);
    MCPY_CHK_NULL_RETURN(m_bltState);
    MCPY_CHK_STATUS_RETURN(m_bltState->Initialize());

    // vebox init
    if (nullptr == m_veboxCopyState)
    {
        m_veboxCopyState = MOS_New(VeboxCopyState, m_osInterface, m_mhwInterfaces);
        MCPY_CHK_NULL_RETURN(m_veboxCopyState);
        MCPY_CHK_STATUS_RETURN(m_veboxCopyState->Initialize());
    }
    return eStatus;
}

//!
//! \brief    surface copy pre process.
//! \details  pre process before doing surface copy.
//! \param    preferMethod
//!           [in] Preferred media copy method
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
//!
MOS_STATUS MediaCopyStateXe_Xpm_Base::PreProcess(MCPY_METHOD preferMethod)
{
    if ((preferMethod == MCPY_METHOD_POWERSAVING)
        && m_mcpyEngineCaps.engineBlt
        && (m_mcpySrc.CpMode == MCPY_CPMODE_CP)
        && (m_mcpyDst.CpMode == MCPY_CPMODE_CLEAR))
    {
        //Allow blt engine to do copy when dst buffer is staging buffer and allocate in system mem, since protection off with blt engine.
        m_allowCPBltCopy = true;
    }
    else
    {
        m_allowCPBltCopy = false;
    }

    return MOS_STATUS_SUCCESS;
}

MediaCopyStateXe_Xpm_Base::~MediaCopyStateXe_Xpm_Base()
{
    MOS_Delete(m_bltState);
    MOS_Delete(m_veboxCopyState);
}

bool MediaCopyStateXe_Xpm_Base::RenderFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    bool eStatus = false;
    //eStatus = checkinpuforamt(src);
    //eStatus = checkoutputformat(dst);

    return eStatus;
}

MOS_STATUS MediaCopyStateXe_Xpm_Base::FeatureSupport(PMOS_RESOURCE src, PMOS_RESOURCE dst,
            MCPY_STATE_PARAMS& mcpy_src, MCPY_STATE_PARAMS& mcpy_dst, MCPY_ENGINE_CAPS& caps)
{
    // TGL has full hw enigne.
    // check CP COPYIN case. ??
    if (mcpy_src.CompressionMode == MOS_MMC_DISABLED &&
        mcpy_dst.CompressionMode == MOS_MMC_RC       &&
        mcpy_dst.CpMode   == MCPY_CPMODE_CP)
    {
        caps.engineBlt = false;
    }

    caps.engineRender = false; // currently, Xe_XPM+ render fast copy still uses MDF kernel.

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe_Xpm_Base::MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    MCPY_CHK_NULL_RETURN(m_veboxCopyState);
    return m_veboxCopyState->CopyMainSurface(src, dst);
}

bool MediaCopyStateXe_Xpm_Base::IsVeboxCopySupported(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    bool supported = false;

    if (m_osInterface &&
        !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrVERing))
    {
        return false;
    }

    if (m_veboxCopyState)
    {
        supported = m_veboxCopyState->IsFormatSupported(src) && m_veboxCopyState->IsFormatSupported(dst);
    }

    if (src->TileType == MOS_TILE_LINEAR &&
        dst->TileType == MOS_TILE_LINEAR)
    {
        supported = false;
    }

    return supported;
}

MOS_STATUS MediaCopyStateXe_Xpm_Base::MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    MCPY_CHK_STATUS_RETURN(m_bltState->CopyMainSurface(src, dst));
    return MOS_STATUS_SUCCESS;

}

MOS_STATUS MediaCopyStateXe_Xpm_Base::MediaRenderCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // not implement yet
    return MOS_STATUS_UNIMPLEMENTED;
}