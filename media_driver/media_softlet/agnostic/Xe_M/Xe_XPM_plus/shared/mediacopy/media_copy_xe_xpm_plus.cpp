/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     media_copy_xe_xpm_plus.cpp
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!
#include "media_copy_xe_xpm_plus.h"
#include "media_blt_copy_xe_xpm_plus.h"
#include "media_render_copy_xe_xpm_plus.h"
#include "media_skuwa_specific.h"
#include "mos_os.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"
#include "vp_common.h"
class MhwInterfaces;

MediaCopyStateXe_Xpm_Plus::MediaCopyStateXe_Xpm_Plus() :
    MediaCopyBaseState()
{

}

MOS_STATUS MediaCopyStateXe_Xpm_Plus::Initialize(  PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MCPY_CHK_NULL_RETURN(osInterface);
    MCPY_CHK_NULL_RETURN(mhwInterfaces);
    MEDIA_FEATURE_TABLE* pSkuTable;

    m_osInterface   = osInterface;
    m_mhwInterfacesXeXpmPlus = mhwInterfaces;

    MCPY_CHK_STATUS_RETURN(MediaCopyBaseState::Initialize(osInterface));

    pSkuTable = osInterface->pfnGetSkuTable(osInterface);
    if (MEDIA_IS_SKU(pSkuTable, FtrCCSNode))
    {
        // render copy init
        if (nullptr == m_renderCopy )
        {
            m_renderCopy = MOS_New(RenderCopy_Xe_Xpm_Plus, m_osInterface, m_mhwInterfacesXeXpmPlus);
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
        m_bltState = MOS_New(BltStateXe_Xpm_Plus, m_osInterface, m_mhwInterfacesXeXpmPlus);
        MCPY_CHK_NULL_RETURN(m_bltState);
        MCPY_CHK_STATUS_RETURN(m_bltState->Initialize());
    }
    return eStatus;
}

MediaCopyStateXe_Xpm_Plus::~MediaCopyStateXe_Xpm_Plus()
{
    MOS_Delete(m_renderCopy);
    MOS_Delete(m_bltState);
    if (m_mhwInterfacesXeXpmPlus)
    {
        if (m_mhwInterfacesXeXpmPlus->m_cpInterface)
        {
            if (m_osInterface)
            {
                m_osInterface->pfnDeleteMhwCpInterface(m_mhwInterfacesXeXpmPlus->m_cpInterface);
                m_mhwInterfacesXeXpmPlus->m_cpInterface = nullptr;
            }
            else
            {
                MCPY_ASSERTMESSAGE("Failed to destroy cpInterface.");
            }
        }
        MOS_Delete(m_mhwInterfacesXeXpmPlus->m_miInterface);
        MOS_Delete(m_mhwInterfacesXeXpmPlus->m_veboxInterface);
        MOS_Delete(m_mhwInterfacesXeXpmPlus->m_bltInterface);
        MOS_Delete(m_mhwInterfacesXeXpmPlus->m_renderInterface);
        MOS_Delete(m_mhwInterfacesXeXpmPlus);
        m_mhwInterfacesXeXpmPlus = nullptr;
    }
}

bool MediaCopyStateXe_Xpm_Plus::RenderFormatSupportCheck(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    MOS_STATUS              eStatus1, eStatus2;
    VPHAL_GET_SURFACE_INFO  Info;
    VPHAL_SURFACE           Source;
    VPHAL_SURFACE           Target;
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));
    MOS_ZeroMemory(&Source, sizeof(VPHAL_SURFACE));
    MOS_ZeroMemory(&Target, sizeof(VPHAL_SURFACE));

    Source.OsResource = *src;
    eStatus1 = VpHal_GetSurfaceInfo(
                   m_osInterface,
                   &Info,
                   &Source);
    Target.OsResource = *dst;
    eStatus2 = VpHal_GetSurfaceInfo(
                   m_osInterface,
                   &Info,
                   &Target);

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

MOS_STATUS MediaCopyStateXe_Xpm_Plus::FeatureSupport(PMOS_RESOURCE src, PMOS_RESOURCE dst, MCPY_ENGINE_CAPS & caps)
{
    caps.engineVebox  = false; // PVC doesn't have vebox.
    caps.engineBlt    = true; // enable blt engine after hw cmd adds.
    caps.engineRender = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe_Xpm_Plus::AuxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // CCS only could be copied by blt engine.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe_Xpm_Plus::MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
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

MOS_STATUS MediaCopyStateXe_Xpm_Plus::MediaRenderCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
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

MOS_STATUS MediaCopyStateXe_Xpm_Plus::CapabilityCheck(
    MOS_FORMAT         format,
    MCPY_STATE_PARAMS &mcpySrc,
    MCPY_STATE_PARAMS &mcpyDst,
    MCPY_ENGINE_CAPS  &caps,
    MCPY_METHOD        preferMethod)
{
    // init hw enigne caps.pvc doesn't have vebox
    caps.engineBlt    = 1;
    caps.engineRender = 1;
    caps.engineVebox = 0;

    // derivate class specific check. include HW engine avaliable check.
    MCPY_CHK_STATUS_RETURN(FeatureSupport(mcpySrc.OsRes, mcpyDst.OsRes, caps));

    // common policy check
    // legal check
    // Blt engine does not support protection, allow the copy if dst is staging buffer in system mem
    if (preferMethod == MCPY_METHOD_POWERSAVING && 
        (mcpySrc.CpMode == MCPY_CPMODE_CP || mcpyDst.CpMode == MCPY_CPMODE_CP))
    {
        MCPY_ASSERTMESSAGE("illegal usage");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Eu cap check.
    if (!RenderFormatSupportCheck(mcpySrc.OsRes, mcpyDst.OsRes) || // format check, implemented on Gen derivate class.
        mcpySrc.bAuxSuface)
    {
        caps.engineRender = false;
    }

    if (!caps.engineVebox && !caps.engineBlt && !caps.engineRender)
    {
        return MOS_STATUS_INVALID_PARAMETER; // unsupport copy on each hw engine.
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe_Xpm_Plus::CopyEnigneSelect(MCPY_METHOD& preferMethod, MCPY_ENGINE& mcpyEngine, MCPY_ENGINE_CAPS& caps)
{
    // assume perf render > vebox > blt. blt data should be measured.
    // driver should make sure there is at least one he can process copy even customer choice doesn't match caps.
    switch (preferMethod)
    {
        case MCPY_METHOD_PERFORMANCE:
        case MCPY_METHOD_DEFAULT:
            mcpyEngine = caps.engineRender ? MCPY_ENGINE_RENDER:MCPY_ENGINE_BLT;
            break;
        default:
            mcpyEngine = MCPY_ENGINE_BLT;
            break;
    }

    return MOS_STATUS_SUCCESS;
}
