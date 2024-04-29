/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_copy_xe_lpm_plus_base.cpp
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!
#include "media_copy_xe_lpm_plus_base.h"
#include "mhw_render.h"
#include "media_debug_dumper.h"

MediaCopyStateXe_Lpm_Plus_Base::MediaCopyStateXe_Lpm_Plus_Base() :
    MediaCopyBaseState()
{

}

MOS_STATUS MediaCopyStateXe_Lpm_Plus_Base::Initialize(PMOS_INTERFACE  osInterface, MhwInterfacesNext *mhwInterfaces)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MCPY_CHK_NULL_RETURN(osInterface);
    MCPY_CHK_NULL_RETURN(mhwInterfaces);
    MEDIA_FEATURE_TABLE* pSkuTable;

    m_osInterface   = osInterface;
    m_mhwInterfaces = mhwInterfaces;
    pSkuTable       = osInterface->pfnGetSkuTable(osInterface);

    MCPY_CHK_STATUS_RETURN(MediaCopyBaseState::Initialize(osInterface));

    if (MEDIA_IS_SKU(pSkuTable, FtrCCSNode))
    {
        // render copy init
        if (nullptr == m_renderCopy)
        {
            m_renderCopy = MOS_New(RenderCopyXe_LPM_Plus_Base, m_osInterface, m_mhwInterfaces);
            MCPY_CHK_NULL_RETURN(m_renderCopy);
            MCPY_CHK_STATUS_RETURN(m_renderCopy->Initialize());
        }
    }
    else
    {
        MCPY_NORMALMESSAGE(" Rendercopy don't support due to no CCS Ring ");
    }
    // blt copy init
    if (nullptr == m_bltState)
    {
        m_bltState = MOS_New(BltStateXe_Lpm_Plus_Base, m_osInterface, m_mhwInterfaces);
        MCPY_CHK_NULL_RETURN(m_bltState);
        MCPY_CHK_STATUS_RETURN(m_bltState->Initialize());
    }

    // vebox init
    if ( nullptr == m_veboxCopyState)
    {
        m_veboxCopyState = MOS_New(VeboxCopyStateXe_Lpm_Plus_Base, m_osInterface, m_mhwInterfaces);
        MCPY_CHK_NULL_RETURN(m_veboxCopyState);
        MCPY_CHK_STATUS_RETURN(m_veboxCopyState->Initialize());
    }

    return eStatus;
}

MediaCopyStateXe_Lpm_Plus_Base::~MediaCopyStateXe_Lpm_Plus_Base()
{
    //MOS_Delete(m_renderCopy);
    MOS_Delete(m_bltState);
    MOS_Delete(m_veboxCopyState);
    MOS_Delete(m_renderCopy);
    if (m_mhwInterfaces != nullptr)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }
}

bool MediaCopyStateXe_Lpm_Plus_Base::RenderFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    MOS_STATUS              eStatus1, eStatus2;
    MOS_SURFACE             Source = {0};
    MOS_SURFACE             Target = {0};

    Source.OsResource = *src;
    Source.Format = Format_Invalid;
    eStatus1 = m_osInterface->pfnGetResourceInfo(m_osInterface, src, &Source);

    Target.OsResource = *dst;
    Target.Format = Format_Invalid;
    eStatus2 = m_osInterface->pfnGetResourceInfo(m_osInterface, dst, &Target);


    if ((Source.Format != Target.Format) || (eStatus1 != MOS_STATUS_SUCCESS) || (eStatus2 != MOS_STATUS_SUCCESS))
    {
         MCPY_ASSERTMESSAGE("wrong input parmaters or src format don't match the dest format ");
         return false;
    }

     if ((Source.Format != Format_RGBP) && (Source.Format != Format_NV12) && (Source.Format != Format_RGB)
     && (Source.Format != Format_P010) && (Source.Format != Format_P016) && (Source.Format != Format_YUY2)
     && (Source.Format != Format_Y210) && (Source.Format != Format_Y216) && (Source.Format != Format_AYUV)
     && (Source.Format != Format_Y410) && (Source.Format != Format_A8R8G8B8))
    {
         MCPY_NORMALMESSAGE("render copy doesn't suppport format %d ", Source.Format);
         return false;
    }

    if ((MOS_TILE_LINEAR == Source.TileType) && (MOS_TILE_LINEAR == Target.TileType))
    {
         MCPY_NORMALMESSAGE("render copy doesn't suppport  linear to linear copy");
         return false;
    }

    return true;
}

MOS_STATUS MediaCopyStateXe_Lpm_Plus_Base::FeatureSupport(PMOS_RESOURCE src, PMOS_RESOURCE dst, MCPY_ENGINE_CAPS & caps)
{
    caps.engineVebox  = true;
    caps.engineBlt    = true;
    caps.engineRender = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe_Lpm_Plus_Base::MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    if (m_bltState != nullptr)
    {
        return m_bltState->CopyMainSurface(src, dst);
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
}

MOS_STATUS MediaCopyStateXe_Lpm_Plus_Base::MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    if (m_veboxCopyState != nullptr)
    {
        return m_veboxCopyState->CopyMainSurface(src, dst);
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
}

MOS_STATUS MediaCopyStateXe_Lpm_Plus_Base::MediaRenderCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    if (m_renderCopy != nullptr)
    {
        return m_renderCopy->CopySurface(src, dst);
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
}

bool MediaCopyStateXe_Lpm_Plus_Base::IsVeboxCopySupported(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    bool supported = false;

    if (m_osInterface &&
        !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrVERing))
    {
        return false;
    }

    if (m_veboxCopyState)
    {
        supported = m_veboxCopyState->IsSurfaceSupported(src) && m_veboxCopyState->IsSurfaceSupported(dst);
    }

    if (src->TileType == MOS_TILE_LINEAR &&
        dst->TileType == MOS_TILE_LINEAR)
    {
        supported = false;
    }

    return supported;
}

MOS_STATUS MediaCopyStateXe_Lpm_Plus_Base::CopyEnigneSelect(MCPY_METHOD &preferMethod, MCPY_ENGINE &mcpyEngine, MCPY_ENGINE_CAPS &caps)
{
    if (preferMethod == MCPY_METHOD_DEFAULT)
    {
        preferMethod = MCPY_METHOD_PERFORMANCE;
    }
    MediaCopyBaseState::CopyEnigneSelect(preferMethod, mcpyEngine, caps);
    return MOS_STATUS_SUCCESS;
}