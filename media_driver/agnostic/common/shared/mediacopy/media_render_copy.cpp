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
//! \file       media_render_copy.cpp
//! \brief      render copy implement file
//! \details    render copy implement file
//!

#include "media_render_copy.h"
#include "hal_kerneldll.h"
#include "media_copy.h"
#include "media_interfaces_mhw.h"
#include "mhw_cp_interface.h"
#include "mhw_state_heap.h"
#include "mos_defs_specific.h"
#include "mos_os_hw.h"
#include "mos_utilities.h"
#include "vphal_render_common.h"

RenderCopyState::RenderCopyState(PMOS_INTERFACE osInterface, MhwInterfaces *mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(mhwInterfaces)
{
    if (nullptr == osInterface)
    {
        MCPY_ASSERTMESSAGE("osInterface is nullptr");
        return;
    }
    m_renderInterface = mhwInterfaces->m_renderInterface;
    m_RenderData.pKernelParam = (PRENDERHAL_KERNEL_PARAM)g_rendercopy_KernelParam;
    Mos_SetVirtualEngineSupported(osInterface, true);
    osInterface->pfnVirtualEngineSupported(osInterface, true, false);

    MOS_NULL_RENDERING_FLAGS        NullRenderingFlags;
    NullRenderingFlags = osInterface->pfnGetNullHWRenderFlags(osInterface);

    m_bNullHwRenderCopy =
     NullRenderingFlags.VPComp ||
     NullRenderingFlags.VPGobal;
}

RenderCopyState:: ~RenderCopyState()
{
    if (m_renderHal != nullptr)
    {
       MOS_STATUS eStatus = m_renderHal->pfnDestroy(m_renderHal);
       if (eStatus != MOS_STATUS_SUCCESS)
       {
           MCPY_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
       }
       MOS_FreeMemAndSetNull(m_renderHal);
    }

    if (m_cpInterface != nullptr)
    {
        if (m_osInterface)
        {
            m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
            m_cpInterface = nullptr;
        }
        else
        {
            MCPY_ASSERTMESSAGE("Failed to destroy vpInterface.");
        }
    }

    // Destroy Kernel DLL objects (cache, hash table, states)
    if (m_pKernelDllState)
    {
       KernelDll_ReleaseStates(m_pKernelDllState);
       m_pKernelBin = nullptr;
    }
}

MOS_STATUS RenderCopyState::Initialize()
{
    RENDERHAL_SETTINGS_LEGACY RenderHalSettings;

    MCPY_CHK_NULL_RETURN(m_osInterface);

    m_renderHal = (PRENDERHAL_INTERFACE_LEGACY)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE_LEGACY));
    MCPY_CHK_NULL_RETURN(m_renderHal);
    MCPY_CHK_STATUS_RETURN(RenderHal_InitInterface_Legacy(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));

    // Allocate and initialize HW states
    RenderHalSettings.iMediaStates = 32;
    MCPY_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    m_renderHal->sseuTable              = VpDefaultSSEUTable;
    m_renderHal->forceDisablePreemption = true;

    return MOS_STATUS_SUCCESS;
}

int32_t RenderCopyState::GetBytesPerPixelPerPlane(
    MOS_FORMAT        Format)
{
    int32_t     iBytePerPixelPerPlane = 0;

    switch(Format)
    {
    case Format_NV12:
    case Format_RGBP:
        iBytePerPixelPerPlane = 1;
        break;
    case Format_YUY2:
    case Format_P010:
    case Format_P016:
        iBytePerPixelPerPlane = 2;
        break;
    case Format_Y210:
    case Format_Y216:
    case Format_Y410:
    case Format_AYUV:
    case Format_A8R8G8B8:
        iBytePerPixelPerPlane = 4;
        break;
    case Format_Y416:
        iBytePerPixelPerPlane = 8;
        break;
    default:
        MCPY_ASSERTMESSAGE("can't support formats %d for render copy", Format);
        break;
    }

    return iBytePerPixelPerPlane;
}

MOS_STATUS RenderCopyState::SubmitCMD( )
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCopyState::GetCurentKernelID( )
{
    int32_t iBytePerPixelPerPlane = GetBytesPerPixelPerPlane(m_Source.Format);

    if ((iBytePerPixelPerPlane < 1) || (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("GetCurentKernelID wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // This scheme is temporary
    // for a !MOS_TILE_LINEAR plane surface, it is 2D surface.
    // for a MOS_TILE_LINEAR plane surface, if (dwWidth * bytes_per_pixel < dwPitch) it is 2D surface
    // if (dwWidth * bytes_per_pixel == dwPitch) it is a 1D surface.

    if ((m_Source.Format == Format_NV12) || (m_Source.Format == Format_P010) || (m_Source.Format == Format_P016))
    {
        if (m_Source.TileType == MOS_TILE_LINEAR && m_Target.TileType != MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_1D_to_2D_NV12;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR && m_Target.TileType == MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_1D_NV12;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR && m_Target.TileType != MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_2D_NV12;
        }
        else
        {
             m_currKernelId = KERNEL_CopyKernel_MAX;
             MCPY_ASSERTMESSAGE("Can't find right kernel to support, pls help to check input parameters.");
             return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if (m_Source.Format == Format_RGBP)
    {
        if (m_Source.TileType == MOS_TILE_LINEAR && m_Target.TileType != MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_1D_to_2D_Planar;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR && m_Target.TileType == MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_1D_Planar;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR && m_Target.TileType != MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_2D_Planar;
        }
        else
        {
            m_currKernelId = KERNEL_CopyKernel_MAX;
            MCPY_ASSERTMESSAGE("kernel can't support it.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if ((m_Source.Format == Format_YUY2) || (m_Source.Format == Format_Y210) || (m_Source.Format == Format_Y216)
              || (m_Source.Format == Format_AYUV) || (m_Source.Format == Format_Y410) || (m_Source.Format == Format_Y416)
              || (m_Source.Format == Format_A8R8G8B8))
    {
        if (m_Source.TileType == MOS_TILE_LINEAR && m_Target.TileType != MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_1D_to_2D_Packed;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR && m_Target.TileType == MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_1D_Packed;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR && m_Target.TileType != MOS_TILE_LINEAR)
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_2D_Packed;
        }
        else
        {
             m_currKernelId = KERNEL_CopyKernel_MAX;
             MCPY_ASSERTMESSAGE("kernel can't support it.");
             return MOS_STATUS_INVALID_PARAMETER;
        }

    }
    MCPY_NORMALMESSAGE("Used Render copy and currentKernel id = %d.", m_currKernelId);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    setup surface states
//! \details  Setup surface states for fast 1toN
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS RenderCopyState::SetupSurfaceStates()
{
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    uint32_t                        index;
    uint32_t                        width  = 0;
    MOS_FORMAT                      format = Format_NV12;
    int32_t                         iBTEntry;
    PRENDERHAL_INTERFACE            pRenderHal  = m_renderHal;
    PMEDIACOPY_RENDER_DATA          pRenderData = &m_RenderData;
    RENDERHAL_SURFACE               RenderHalSource = {};  // source for mhw
    RENDERHAL_SURFACE               RenderHalTarget = {};  // target for mhw
    MCPY_CHK_NULL_RETURN(pRenderHal);
    MCPY_CHK_NULL_RETURN(pRenderData);
    // Source surface
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    pRenderData->SurfMemObjCtl.SourceSurfMemObjCtl =
         pRenderHal->pOsInterface->pfnCachePolicyGetMemoryObject(
         MOS_MP_RESOURCE_USAGE_SurfaceState_RCS,
         pRenderHal->pOsInterface->pfnGetGmmClientContext(pRenderHal->pOsInterface)).DwordValue;

    pRenderData->SurfMemObjCtl.TargetSurfMemObjCtl = pRenderData->SurfMemObjCtl.SourceSurfMemObjCtl;

    SurfaceParams.bAVS              = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_SRCRECT;
    SurfaceParams.isOutput     = false;
    SurfaceParams.MemObjCtl         = pRenderData->SurfMemObjCtl.SourceSurfMemObjCtl;

    SurfaceParams.Type              = RENDERHAL_SURFACE_TYPE_G10;
    SurfaceParams.bWidthInDword_Y   = false;
    SurfaceParams.bWidthInDword_UV  = false;
    SurfaceParams.bWidth16Align     = false;

    if (Format_NV12 == m_Target.Format)
    {
        m_Target.SurfType = SURF_OUT_RENDERTARGET;
        m_Source.SurfType = SURF_OUT_RENDERTARGET;
    }

    if (m_currKernelId == KERNEL_CopyKernel_1D_to_2D_NV12
        || m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Planar
        || m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed)
    {
        format = m_Source.Format;
        width = m_Source.dwWidth;
        m_Source.Format = Format_RAW;

        if ((format == Format_NV12) || (format == Format_P010) || (format == Format_P016))
        {
           m_Source.dwWidth = (m_Source.dwHeight * m_Source.dwPitch) * 3 / 2;
        }
        else if (format == Format_RGBP)
        {
           m_Source.dwWidth = (m_Source.dwHeight * m_Source.dwPitch) * 3;
        }
        else if ((format == Format_YUY2) || (format == Format_Y210) || (format == Format_Y216)
                 || (format == Format_AYUV) || (format == Format_Y410) || (format == Format_Y416)
                 || (format == Format_A8R8G8B8))
        {
           m_Source.dwWidth = m_Source.dwHeight * m_Source.dwPitch;
        }

        m_Source.dwWidth = MOS_ALIGN_CEIL(m_Source.dwWidth, 128);
        //1D surfaces
        VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
             pRenderHal,
             &m_Source,
             &RenderHalSource,
             &SurfaceParams,
             pRenderData->iBindingTable,
             RENDERCOPY_SRC_INDEX,
             false));
        m_Source.Format = format;
        m_Source.dwWidth = width;
    }
    else {
        //2D surfaces
        VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
            pRenderHal,
            &m_Source,
            &RenderHalSource,
            &SurfaceParams,
            pRenderData->iBindingTable,
            RENDERCOPY_SRC_INDEX,
            false));
    }

    // Target surface
    SurfaceParams.MemObjCtl         = pRenderData->SurfMemObjCtl.TargetSurfMemObjCtl;
    SurfaceParams.Type              = pRenderHal->SurfaceTypeDefault;
    SurfaceParams.isOutput     = true;
    SurfaceParams.bAVS              = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_DSTRECT;

    if (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_NV12
        || m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Planar
        || m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed)
    {
        format = m_Target.Format;
        width = m_Target.dwWidth;
        m_Target.Format = Format_RAW;

          if ((format == Format_NV12) || (format == Format_P010) || (format == Format_P016))
          {
             m_Target.dwWidth = (m_Target.dwHeight * m_Target.dwPitch) * 3 / 2;
          }
          else if (format == Format_RGBP)
          {
             m_Target.dwWidth = (m_Target.dwHeight * m_Target.dwPitch) * 3;
          }
          else if ((format == Format_YUY2) || (format == Format_Y210) || (format == Format_Y216)
                   || (format == Format_AYUV) || (format == Format_Y410) || (format == Format_Y416)
                   || (format == Format_A8R8G8B8))
          {
             m_Target.dwWidth = m_Target.dwHeight * m_Target.dwPitch;
          }

        m_Target.dwWidth = MOS_ALIGN_CEIL(m_Target.dwWidth, 128);

        //1D surface.
        VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetBufferSurfaceForHwAccess(
            pRenderHal,
            &m_Target,
            &RenderHalTarget,
            &SurfaceParams,
            pRenderData->iBindingTable,
            RENDERCOPY_DST_INDEX,
            true));
        m_Target.Format = format;
        m_Target.dwWidth = width;
    }
    else
    {
        //2D surface.
        VPHAL_RENDER_CHK_STATUS(VpHal_CommonSetSurfaceForHwAccess(
            pRenderHal,
            &m_Target,
            &RenderHalTarget,
            &SurfaceParams,
            pRenderData->iBindingTable,
            RENDERCOPY_DST_INDEX,
            true));

    }
finish:
    return eStatus;
}


MOS_STATUS RenderCopyState::LoadStaticData(
    int32_t*                        piCurbeOffset)
{
    DP_RENDERCOPY_NV12_STATIC_DATA          WalkerNV12Static;
    DP_RENDERCOPY_RGBP_STATIC_DATA          WalkerPlanarStatic;
    DP_RENDERCOPY_PACKED_STATIC_DATA        WalkerSinglePlaneStatic;

    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    int32_t                                 iCurbeLength = 0;
    int32_t                                 iBytePerPixelPerPlane = GetBytesPerPixelPerPlane(m_Target.Format);
    PRENDERHAL_INTERFACE                    pRenderHal  = m_renderHal;
    PMEDIACOPY_RENDER_DATA                  pRenderData = &m_RenderData;

    MCPY_CHK_NULL_RETURN(pRenderHal);
    MCPY_CHK_NULL_RETURN(pRenderData);
    if ((iBytePerPixelPerPlane < 1) || (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("LoadStaticData wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    int32_t srcResourceOffset = (int32_t)(m_Source.OsResource.pGmmResInfo->GetPlanarXOffset(GMM_NO_PLANE));
    int32_t dstResourceOffset = (int32_t)(m_Target.OsResource.pGmmResInfo->GetPlanarXOffset(GMM_NO_PLANE));
    
    if (srcResourceOffset)
    {
        m_Source.dwOffset -= srcResourceOffset;
    }
    
    if (dstResourceOffset)
    {
        m_Target.dwOffset -= dstResourceOffset;
    }

    if ((m_Target.Format == Format_NV12) || ((m_Target.Format == Format_P010) || (m_Target.Format == Format_P016)))
    {
        // Set relevant static data
        MOS_ZeroMemory(&WalkerNV12Static, sizeof(DP_RENDERCOPY_NV12_STATIC_DATA));

        WalkerNV12Static.DW0.Inputsurfaceindex = RENDERCOPY_SRC_INDEX;
        WalkerNV12Static.DW1.Outputsurfaceindex = RENDERCOPY_DST_INDEX;

        if (m_currKernelId == KERNEL_CopyKernel_1D_to_2D_NV12)
        {
            WalkerNV12Static.DW2.Widthdword = (m_Source.dwWidth * iBytePerPixelPerPlane + 3) / 4;
            WalkerNV12Static.DW3.Height = m_Source.dwHeight;
            WalkerNV12Static.DW4.ShiftLeftOffsetInBytes = m_Source.dwOffset;
            WalkerNV12Static.DW5.Widthstride = m_Source.dwPitch;
            WalkerNV12Static.DW6.Heightstride = m_Source.dwHeight;
        }
        else
        {
            WalkerNV12Static.DW2.Widthdword = (m_Source.dwWidth < m_Target.dwWidth) ? m_Source.dwWidth : m_Target.dwWidth;
            WalkerNV12Static.DW2.Widthdword = (WalkerNV12Static.DW2.Widthdword * iBytePerPixelPerPlane + 3) / 4;
            WalkerNV12Static.DW3.Height = (m_Source.dwHeight < m_Target.dwHeight) ? m_Source.dwHeight:m_Target.dwHeight;
            WalkerNV12Static.DW4.ShiftLeftOffsetInBytes = m_Target.dwOffset;
            WalkerNV12Static.DW5.Widthstride = m_Target.dwPitch;
            WalkerNV12Static.DW6.Heightstride = m_Target.dwHeight;
        }

        iCurbeLength = sizeof(DP_RENDERCOPY_NV12_STATIC_DATA);
        MCPY_NORMALMESSAGE("Load Curbe data: DW0.Inputsurfaceindex = %d, DW1.Outputsurfaceindex = %d, DW2.WidthDWord= %d, DW3.Height= %d,"
            "DW4.ShiftLeftOffsetInBytes= %d,DW5.Widthstride = %d, DW6.Heightstride = % d.",
            WalkerNV12Static.DW0.Inputsurfaceindex,
            WalkerNV12Static.DW1.Outputsurfaceindex,
            WalkerNV12Static.DW2.Widthdword,
            WalkerNV12Static.DW3.Height,
            WalkerNV12Static.DW4.ShiftLeftOffsetInBytes,
            WalkerNV12Static.DW5.Widthstride,
            WalkerNV12Static.DW6.Heightstride);
        *piCurbeOffset = pRenderHal->pfnLoadCurbeData(
         pRenderHal,
         pRenderData->pMediaState,
         &WalkerNV12Static,
         iCurbeLength);
    }
    else if (m_Target.Format == Format_RGBP)
    {
        // Set relevant static data
        MOS_ZeroMemory(&WalkerPlanarStatic, sizeof(DP_RENDERCOPY_RGBP_STATIC_DATA));

        if (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Planar)
        {
            WalkerPlanarStatic.DW0.InputsurfaceRindex = RENDERCOPY_SRC_INDEX + 2;
            WalkerPlanarStatic.DW1.InputsurfaceGindex = RENDERCOPY_SRC_INDEX;
            WalkerPlanarStatic.DW2.InputsurfaceBindex = RENDERCOPY_SRC_INDEX + 1;
        }
        else
        {
            WalkerPlanarStatic.DW0.InputsurfaceRindex = RENDERCOPY_SRC_INDEX;
            WalkerPlanarStatic.DW1.InputsurfaceGindex = RENDERCOPY_SRC_INDEX + 1;
            WalkerPlanarStatic.DW2.InputsurfaceBindex = RENDERCOPY_SRC_INDEX + 2;
        }

        if (m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Planar)
        {
            WalkerPlanarStatic.DW3.OutputsurfaceRindex = RENDERCOPY_DST_INDEX + 2;
            WalkerPlanarStatic.DW4.OutputsurfaceGindex = RENDERCOPY_DST_INDEX;
            WalkerPlanarStatic.DW5.OutputsurfaceBindex = RENDERCOPY_DST_INDEX + 1;
        }
        else
        {
            WalkerPlanarStatic.DW3.OutputsurfaceRindex = RENDERCOPY_DST_INDEX;
            WalkerPlanarStatic.DW4.OutputsurfaceGindex = RENDERCOPY_DST_INDEX + 1;
            WalkerPlanarStatic.DW5.OutputsurfaceBindex = RENDERCOPY_DST_INDEX + 2;
        }

        if (m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Planar)
        {
            WalkerPlanarStatic.DW6.Widthdword = m_Source.dwPitch / 4;
            WalkerPlanarStatic.DW7.Height = m_Source.dwHeight;
            WalkerPlanarStatic.DW8.ShiftLeftOffsetInBytes = m_Source.dwOffset;
        }
        else
        {
            WalkerPlanarStatic.DW6.Widthdword = m_Target.dwPitch / 4;
            WalkerPlanarStatic.DW7.Height = m_Target.dwHeight;
            WalkerPlanarStatic.DW8.ShiftLeftOffsetInBytes = m_Target.dwOffset;
        }

        WalkerPlanarStatic.DW9.WidthdwordNoPadding = (m_Source.dwWidth < m_Target.dwWidth) ? m_Source.dwWidth : m_Target.dwWidth;
        WalkerPlanarStatic.DW9.WidthdwordNoPadding = (WalkerPlanarStatic.DW9.WidthdwordNoPadding * iBytePerPixelPerPlane + 3) / 4;
        WalkerPlanarStatic.DW10.Dst2DStartX = 0;
        WalkerPlanarStatic.DW11.Dst2DStartY = 0;

        iCurbeLength = sizeof(DP_RENDERCOPY_RGBP_STATIC_DATA);
        MCPY_NORMALMESSAGE("Load Curbe data: DW0.InputsurfaceRindex = %d, DW1.InputsurfaceGindex = %d, DW2.InputsurfaceBindex= %d, DW3.Height= %d,"
            "DW4.OutputsurfaceGindex = %d, DW5.OutputsurfaceBindex = %d, DW6.Widthdword = %d, DW7.Height = %d, DW8.ShiftLeftOffsetInByte= %d,"
            "DW9.WidthdwordNoPadding = %d, WalkerPlanarStatic.DW10.Dst2DStartX = %d, WalkerPlanarStatic.DW11.Dst2DStartY = %d.",
            WalkerPlanarStatic.DW0.InputsurfaceRindex,
            WalkerPlanarStatic.DW1.InputsurfaceGindex,
            WalkerPlanarStatic.DW2.InputsurfaceBindex,
            WalkerPlanarStatic.DW3.OutputsurfaceRindex,
            WalkerPlanarStatic.DW4.OutputsurfaceGindex,
            WalkerPlanarStatic.DW5.OutputsurfaceBindex,
            WalkerPlanarStatic.DW6.Widthdword,
            WalkerPlanarStatic.DW7.Height,
            WalkerPlanarStatic.DW8.ShiftLeftOffsetInBytes,
            WalkerPlanarStatic.DW9.WidthdwordNoPadding,
            WalkerPlanarStatic.DW10.Dst2DStartX,
            WalkerPlanarStatic.DW11.Dst2DStartY);

        *piCurbeOffset = pRenderHal->pfnLoadCurbeData(
                                         pRenderHal,
                                         pRenderData->pMediaState,
                                         &WalkerPlanarStatic,
                                         iCurbeLength);
    }
    else if ((m_Target.Format == Format_YUY2) || (m_Target.Format == Format_Y210) || (m_Target.Format == Format_Y216)
              || (m_Target.Format == Format_AYUV) || (m_Target.Format == Format_Y410) || (m_Target.Format == Format_Y416)
              || (m_Target.Format == Format_A8R8G8B8))
    {
        // Set relevant static data
        MOS_ZeroMemory(&WalkerSinglePlaneStatic, sizeof(WalkerSinglePlaneStatic));

        WalkerSinglePlaneStatic.DW0.InputSurfaceIndex = RENDERCOPY_SRC_INDEX;
        WalkerSinglePlaneStatic.DW1.OutputSurfaceIndex = RENDERCOPY_DST_INDEX;
        if (m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed)
        {
            WalkerSinglePlaneStatic.DW2.WidthDWord = m_Source.dwPitch / 4;
            WalkerSinglePlaneStatic.DW3.Height = m_Source.dwHeight;
            WalkerSinglePlaneStatic.DW4.ShiftLeftOffsetInBytes = m_Source.dwOffset;
        }
        else
        {
            WalkerSinglePlaneStatic.DW2.WidthDWord = m_Target.dwPitch / 4;
            WalkerSinglePlaneStatic.DW3.Height = m_Target.dwHeight;
            WalkerSinglePlaneStatic.DW4.ShiftLeftOffsetInBytes = m_Target.dwOffset;
        }

        if ((m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed) || (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed))
        {
            WalkerSinglePlaneStatic.DW5.ThreadHeight = (m_Source.dwHeight < m_Target.dwHeight) ? m_Source.dwHeight : m_Target.dwHeight;
            WalkerSinglePlaneStatic.DW5.ThreadHeight = (WalkerSinglePlaneStatic.DW5.ThreadHeight + 32 - 1) / 32;
        }
        else if (m_currKernelId == KERNEL_CopyKernel_2D_to_2D_Packed)
        {
            WalkerSinglePlaneStatic.DW5.ThreadHeight = (m_Source.dwHeight + 8 - 1) / 8;
        }
        else
        {
            MCPY_ASSERTMESSAGE("LoadStaticData wrong kernel file.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        WalkerSinglePlaneStatic.DW6.WidthdwordNoPadding = (m_Source.dwWidth < m_Target.dwWidth) ? m_Source.dwWidth : m_Target.dwWidth;
        WalkerSinglePlaneStatic.DW6.WidthdwordNoPadding = (WalkerSinglePlaneStatic.DW6.WidthdwordNoPadding * iBytePerPixelPerPlane + 3) / 4;
        WalkerSinglePlaneStatic.DW7.Dst2DStartX = 0;
        WalkerSinglePlaneStatic.DW8.Dst2DStartY = 0;

        iCurbeLength = sizeof(DP_RENDERCOPY_PACKED_STATIC_DATA);
        MCPY_NORMALMESSAGE("Load Curbe data: DW0.InputSurfaceIndex = %d, DW1.OutputSurfaceIndex = %d, DW2.WidthDWord= %d, DW3.Height= %d,"
            "DW4.ShiftLeftOffsetInBytes= %d,DW5.ThreadHeight = %d, DW6.WidthdwordNoPadding = %d, DW7.Dst2DStartX = %d, DW8.Dst2DStartY = %d.",
            WalkerSinglePlaneStatic.DW0.InputSurfaceIndex,
            WalkerSinglePlaneStatic.DW1.OutputSurfaceIndex,
            WalkerSinglePlaneStatic.DW2.WidthDWord,
            WalkerSinglePlaneStatic.DW3.Height,
            WalkerSinglePlaneStatic.DW4.ShiftLeftOffsetInBytes,
            WalkerSinglePlaneStatic.DW5.ThreadHeight,
            WalkerSinglePlaneStatic.DW6.WidthdwordNoPadding,
            WalkerSinglePlaneStatic.DW7.Dst2DStartX,
            WalkerSinglePlaneStatic.DW8.Dst2DStartY);

        *piCurbeOffset = pRenderHal->pfnLoadCurbeData(
         pRenderHal,
         pRenderData->pMediaState,
         &WalkerSinglePlaneStatic,
         iCurbeLength);
    }
    else
    {
         MCPY_ASSERTMESSAGE("can't support Target format %d", m_Target.Format);
    }

    if (*piCurbeOffset < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    pRenderData->iCurbeLength = iCurbeLength;

    return eStatus;
}

 //!
 //! \brief    Render copy omputer walker setup
 //! \details  Computer walker setup for render copy
 //! \param    PMHW_WALKER_PARAMS pWalkerParams
 //!           [in/out] Pointer to Walker params
 //! \return   MOS_STATUS
 //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
 //!
 MOS_STATUS RenderCopyState::RenderCopyComputerWalker(
 PMHW_GPGPU_WALKER_PARAMS    pWalkerParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    PMEDIACOPY_RENDER_DATA                  pRenderData = &m_RenderData;
    RECT                                    AlignedRect;
    int32_t                                 iBytePerPixelPerPlane = GetBytesPerPixelPerPlane(m_Target.Format);
    uint32_t                                WalkerWidthBlockSize = 128;
    uint32_t                                WalkerHeightBlockSize = 8;

    MCPY_CHK_NULL_RETURN(pRenderData);

    if ((iBytePerPixelPerPlane < 1) || (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("RenderCopyComputerWalker wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((m_Target.Format == Format_YUY2) || (m_Target.Format == Format_Y210) || (m_Target.Format == Format_Y216)
        || (m_Target.Format == Format_AYUV) || (m_Target.Format == Format_Y410) || (m_Target.Format == Format_Y416)
        || (m_Target.Format == Format_A8R8G8B8))
    {
        if ((m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed) || (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed))
        {
            WalkerHeightBlockSize = 32;
        }
        else if (m_currKernelId == KERNEL_CopyKernel_2D_to_2D_Packed)
        {
            WalkerHeightBlockSize = 8;
        }
        else
        {
            MCPY_ASSERTMESSAGE("RenderCopyComputerWalker wrong kernel file.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        WalkerHeightBlockSize = 8;
    }

    // Set walker cmd params - Rasterscan
    MOS_ZeroMemory(pWalkerParams, sizeof(*pWalkerParams));


    AlignedRect.left   = 0;
    AlignedRect.top    = 0;
    AlignedRect.right  = (m_Source.dwPitch < m_Target.dwPitch) ? m_Source.dwPitch : m_Target.dwPitch;
    AlignedRect.bottom = (m_Source.dwHeight < m_Target.dwHeight) ? m_Source.dwHeight : m_Target.dwHeight;
    // Calculate aligned output area in order to determine the total # blocks
   // to process in case of non-16x16 aligned target.
    AlignedRect.right += WalkerWidthBlockSize - 1;
    AlignedRect.bottom += WalkerHeightBlockSize - 1;
    AlignedRect.left -= AlignedRect.left % WalkerWidthBlockSize;
    AlignedRect.top -= AlignedRect.top % WalkerHeightBlockSize;
    AlignedRect.right -= AlignedRect.right % WalkerWidthBlockSize;
    AlignedRect.bottom -= AlignedRect.bottom % WalkerHeightBlockSize;

    pWalkerParams->InterfaceDescriptorOffset = pRenderData->iMediaID;

    pWalkerParams->GroupStartingX = (AlignedRect.left / WalkerWidthBlockSize);
    pWalkerParams->GroupStartingY = (AlignedRect.top / WalkerHeightBlockSize);

    // Set number of blocks
    pRenderData->iBlocksX =
        ((AlignedRect.right - AlignedRect.left) + WalkerWidthBlockSize - 1) / WalkerWidthBlockSize;
    pRenderData->iBlocksY =
        ((AlignedRect.bottom - AlignedRect.top) + WalkerHeightBlockSize -1)/ WalkerHeightBlockSize;

    // Set number of blocks, block size is WalkerWidthBlockSize x WalkerHeightBlockSize.
    pWalkerParams->GroupWidth = pRenderData->iBlocksX;
    pWalkerParams->GroupHeight = pRenderData->iBlocksY; // hight/WalkerWidthBlockSize

    pWalkerParams->ThreadWidth = 1;
    pWalkerParams->ThreadHeight = 1;
    pWalkerParams->ThreadDepth = 1;
    pWalkerParams->IndirectDataStartAddress = pRenderData->iCurbeOffset;
    // Indirect Data Length is a multiple of 64 bytes (size of L3 cacheline). Bits [5:0] are zero.
    pWalkerParams->IndirectDataLength = MOS_ALIGN_CEIL(pRenderData->iCurbeLength, 1 << MHW_COMPUTE_INDIRECT_SHIFT);
    pWalkerParams->BindingTableID = pRenderData->iBindingTable;
    MCPY_NORMALMESSAGE("this = %p, WidthBlockSize %d, HeightBlockSize %d, Widththreads %d, Heightthreads%d",
        this, WalkerWidthBlockSize, WalkerHeightBlockSize, pWalkerParams->GroupWidth, pWalkerParams->GroupHeight);

    return eStatus;
}
