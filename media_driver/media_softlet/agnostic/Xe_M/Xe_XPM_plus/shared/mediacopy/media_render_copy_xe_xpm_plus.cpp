/*===================== begin_copyright_notice ==================================

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

======================= end_copyright_notice ==================================*/
//!
//! \file       media_render_copy_xe_xpm_plus.cpp
//! \brief      implementation of Gen11 hardware functions
//! \details    Render functions
//!

#include "media_render_copy_xe_xpm_plus.h"
#include "igvpkrn_xe_xpm_plus.h"

#define RENDER_COPY_XE_XPM_PLUS_THREADS_MAX  0
#define RENDER_COPY_XE_XPM_PLUS_NUM          9

// Kernel Params ---------------------------------------------------------------
const RENDERHAL_KERNEL_PARAM g_rendercopy_KernelParam_xe_xpm_plus[RENDER_COPY_XE_XPM_PLUS_NUM] =
{
///*  GRF_Count
//    |  BT_Count
//    |  |    Sampler_Count
//    |  |    |  Thread_Count
//    |  |    |  |                             GRF_Start_Register
//    |  |    |  |                             |   CURBE_Length
//    |  |    |  |                             |   |   block_width
//    |  |    |  |                             |   |   |    block_height
//    |  |    |  |                             |   |   |    |   blocks_x
//    |  |    |  |                             |   |   |    |   |   blocks_y
//    |  |    |  |                             |   |   |    |   |   |*/
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_1D_to_2D_NV12
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_2D_to_1D_NV12
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_2D_to_2D_NV12
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_1D_to_2D_Planar
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_2D_to_1D_Planar
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_2D_to_2D_Planar
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_1D_to_2D_Packed
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_2D_to_1D_Packed
    { 4, 34,  0, RENDER_COPY_XE_XPM_PLUS_THREADS_MAX,  0,  0,  64,  8,  1,  1 },    // CopyKernel_2D_to_2D_Packed
};

RenderCopy_Xe_Xpm_Plus::RenderCopy_Xe_Xpm_Plus(PMOS_INTERFACE  osInterface, MhwInterfaces *mhwInterfaces):
    RenderCopyState(osInterface, mhwInterfaces)
{
    MOS_NULL_RENDERING_FLAGS        NullRenderingFlags;
    m_RenderData.pKernelParam = (PRENDERHAL_KERNEL_PARAM)g_rendercopy_KernelParam_xe_xpm_plus;
    Mos_SetVirtualEngineSupported(osInterface, true);
    Mos_CheckVirtualEngineSupported(osInterface, true, false);

    NullRenderingFlags = osInterface->pfnGetNullHWRenderFlags(osInterface);

    m_bNullHwRenderCopy =
     NullRenderingFlags.VPComp ||
     NullRenderingFlags.VPGobal;
}

RenderCopy_Xe_Xpm_Plus:: ~RenderCopy_Xe_Xpm_Plus()
{
    // Destroy Kernel DLL objects (cache, hash table, states)
    if (m_pKernelDllState)
    {
        KernelDll_ReleaseStates(m_pKernelDllState);
    }
}

int32_t RenderCopy_Xe_Xpm_Plus::GetBytesPerPixel(
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
        MCPY_ASSERTMESSAGE("XE_XPM_PLUS can't support formats.");
        break;
    }

    return iBytePerPixelPerPlane;
}


MOS_STATUS RenderCopy_Xe_Xpm_Plus::GetCurentKernelID( )
{
    int32_t iBytePerPixelPerPlane = GetBytesPerPixel(m_Source.Format);

    if ((iBytePerPixelPerPlane < 1) && (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("XE_XPM_PLUS GetCurentKernelID wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // This scheme is temporary
    // for a !MOS_TILE_LINEAR plane surface, it is 2D surface.
    // for a MOS_TILE_LINEAR plane surface, if (dwWidth * bytes_per_pixel < dwPitch) it is 2D surface
    // if (dwWidth * bytes_per_pixel == dwPitch) it is a 1D surface.

    if ((m_Source.Format == Format_NV12) || (m_Source.Format == Format_P010) || (m_Source.Format == Format_P016))
    {
        if ((m_Source.TileType == MOS_TILE_LINEAR && m_Source.dwWidth * iBytePerPixelPerPlane == m_Source.dwPitch)
            && ((m_Target.TileType == MOS_TILE_LINEAR) && (m_Target.dwWidth * iBytePerPixelPerPlane < m_Target.dwPitch)
            || (m_Target.TileType != MOS_TILE_LINEAR)))
        {
            m_currKernelId = KERNEL_CopyKernel_1D_to_2D_NV12;
        }
        else if ((m_Target.TileType == MOS_TILE_LINEAR && m_Target.dwWidth * iBytePerPixelPerPlane == m_Target.dwPitch)
            && ((m_Source.TileType == MOS_TILE_LINEAR) && (m_Source.dwWidth * iBytePerPixelPerPlane < m_Source.dwPitch)
            || (m_Source.TileType != MOS_TILE_LINEAR)))
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_1D_NV12;
        }
        else if (m_Source.TileType == MOS_TILE_Y ||
            (m_Source.TileType == MOS_TILE_LINEAR && m_Source.dwWidth * iBytePerPixelPerPlane < m_Source.dwPitch))
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_2D_NV12;
        }
        else
        {
             m_currKernelId = KERNEL_CopyKernel_MAX;
             MCPY_ASSERTMESSAGE("XE_XPM_PLUS kernel can't support it.");
             return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if (m_Source.Format == Format_RGBP)
    {
        if ((m_Source.TileType == MOS_TILE_LINEAR && m_Source.dwWidth == m_Source.dwPitch)
            && ((m_Target.TileType == MOS_TILE_LINEAR) && (m_Target.dwWidth < m_Target.dwPitch)
            || (m_Target.TileType != MOS_TILE_LINEAR)))
        {
            m_currKernelId = KERNEL_CopyKernel_1D_to_2D_Planar;
        }
        else if ((m_Target.TileType == MOS_TILE_LINEAR && m_Target.dwWidth == m_Target.dwPitch)
            && ((m_Source.TileType == MOS_TILE_LINEAR) && (m_Source.dwWidth < m_Source.dwPitch)
            || (m_Source.TileType != MOS_TILE_LINEAR)))
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_1D_Planar;
        }
        else if (m_Source.TileType == MOS_TILE_Y ||
            (m_Source.TileType == MOS_TILE_LINEAR && m_Source.dwWidth < m_Source.dwPitch))
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_2D_Planar;
        }
        else
        {
            m_currKernelId = KERNEL_CopyKernel_MAX;
            MCPY_ASSERTMESSAGE("XE_XPM_PLUS kernel can't support it.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if ((m_Source.Format == Format_YUY2) || (m_Source.Format == Format_Y210) || (m_Source.Format == Format_Y216)
              || (m_Source.Format == Format_AYUV) || (m_Source.Format == Format_Y410) || (m_Source.Format == Format_Y416)
              || (m_Source.Format == Format_A8R8G8B8))
    {
        if ((m_Source.TileType == MOS_TILE_LINEAR && m_Source.dwWidth * iBytePerPixelPerPlane == m_Source.dwPitch)
            && ((m_Target.TileType == MOS_TILE_LINEAR) && (m_Target.dwWidth * iBytePerPixelPerPlane < m_Target.dwPitch)
            || (m_Target.TileType != MOS_TILE_LINEAR)))
        {
            m_currKernelId = KERNEL_CopyKernel_1D_to_2D_Packed;
        }
        else if ((m_Target.TileType == MOS_TILE_LINEAR && m_Target.dwWidth * iBytePerPixelPerPlane == m_Target.dwPitch)
            && ((m_Source.TileType == MOS_TILE_LINEAR) && (m_Source.dwWidth * iBytePerPixelPerPlane < m_Source.dwPitch)
            || (m_Source.TileType != MOS_TILE_LINEAR)))
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_1D_Packed;
        }
        else if (m_Source.TileType != MOS_TILE_LINEAR ||
            (m_Source.TileType == MOS_TILE_LINEAR && m_Source.dwWidth * iBytePerPixelPerPlane < m_Source.dwPitch))
        {
            m_currKernelId = KERNEL_CopyKernel_2D_to_2D_Packed;
        }
        else
        {
             m_currKernelId = KERNEL_CopyKernel_MAX;
             MCPY_ASSERTMESSAGE("XE_XPM_PLUS kernel can't support it.");
             return MOS_STATUS_INVALID_PARAMETER;
        }

    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCopy_Xe_Xpm_Plus::CopySurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    VPHAL_GET_SURFACE_INFO  Info;
    MOS_ZeroMemory(&Info, sizeof(VPHAL_GET_SURFACE_INFO));
    m_Source.OsResource = *src;
    MCPY_CHK_STATUS_RETURN(VpHal_GetSurfaceInfo(
       m_osInterface,
       &Info,
       &m_Source));
    m_Source.rcSrc.right = m_Source.dwWidth;
    m_Source.rcSrc.bottom = m_Source.dwHeight;
    m_Source.rcDst.right = m_Source.dwWidth;
    m_Source.rcDst.bottom = m_Source.dwHeight;
    m_Source.rcMaxSrc.right = m_Source.dwWidth;
    m_Source.rcMaxSrc.bottom = m_Source.dwHeight;

    m_Target.OsResource = *dst;
    MCPY_CHK_STATUS_RETURN(VpHal_GetSurfaceInfo(
       m_osInterface,
       &Info,
       &m_Target));
    m_Target.rcSrc.right = m_Target.dwWidth;
    m_Target.rcSrc.bottom = m_Target.dwHeight;
    m_Target.rcDst.right = m_Target.dwWidth;
    m_Target.rcDst.bottom = m_Target.dwHeight;
    m_Target.rcMaxSrc.right = m_Target.dwWidth;
    m_Target.rcMaxSrc.bottom = m_Target.dwHeight;

    if ((m_Target.Format != Format_RGBP) && (m_Target.Format != Format_NV12) && (m_Target.Format != Format_RGB)
        && (m_Target.Format != Format_P010) && (m_Target.Format != Format_P016) && (m_Target.Format != Format_YUY2)
        && (m_Target.Format != Format_Y210)  && (m_Target.Format != Format_Y216)  && (m_Target.Format != Format_AYUV)
        && (m_Target.Format != Format_Y410)  && (m_Target.Format != Format_Y416)  && (m_Target.Format != Format_A8R8G8B8))
    {
        MCPY_ASSERTMESSAGE("Can't suppport format %d ", m_Target.Format);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MCPY_CHK_STATUS_RETURN(GetCurentKernelID());
    return SubmitCMD();
}

MOS_STATUS RenderCopy_Xe_Xpm_Plus::SubmitCMD( )
{
    PRENDERHAL_INTERFACE        pRenderHal;
    PMOS_INTERFACE              pOsInterface;
    MHW_KERNEL_PARAM            MhwKernelParam;
    int32_t                     iKrnAllocation;
    int32_t                     iCurbeOffset;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    PRenderCopy_Xe_Xpm_Plus         pRenderCopy = this;
    PMEDIACOPY_RENDER_DATA      pRenderData = &(pRenderCopy->m_RenderData);
    MHW_WALKER_PARAMS           WalkerParams = {0};
    PMHW_WALKER_PARAMS          pWalkerParams = nullptr;
    MHW_GPGPU_WALKER_PARAMS     ComputeWalkerParams = {0};
    PMHW_GPGPU_WALKER_PARAMS    pComputeWalkerParams = nullptr;
    MOS_GPUCTX_CREATOPTIONS     createOption;

    pRenderHal   = pRenderCopy->m_renderHal;
    pOsInterface = pRenderCopy->m_osInterface;
    // no gpucontext will be created if the gpu context has been created before.
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_COMPUTE,
        MOS_GPU_NODE_COMPUTE,
        &createOption));

    // Set GPU Context to Render Engine
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnSetGpuContext(pOsInterface, MOS_GPU_CONTEXT_COMPUTE));

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(pOsInterface);

    // Register the resource of GSH
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnReset(pRenderHal));

    // Register the input resource;
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(
        pOsInterface,
        (PMOS_RESOURCE)&pRenderCopy->m_KernelResource,
        true,
        true));

    // Ensure input can be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface,
        &pRenderCopy->m_Source.OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        false);

    // Ensure Output can be read
    pOsInterface->pfnSyncOnResource(
        pOsInterface,
        &pRenderCopy->m_Target.OsResource,
        pOsInterface->CurrentGpuContextOrdinal,
        false);


    // Set copy kernel
    pRenderCopy->SetupKernel(m_currKernelId);

    //----------------------------------
    // Allocate and reset media state
    //----------------------------------
     pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_RENDER_COPY);
     MCPY_CHK_NULL_RETURN(pRenderData->pMediaState);

    // Allocate and reset SSH instance
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnAssignSshInstance(pRenderHal));

    // Assign and Reset Binding Table
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnAssignBindingTable(
            pRenderHal,
            &pRenderData->iBindingTable));

    // Setup surface states
    MCPY_CHK_STATUS_RETURN(SetupSurfaceStates())

    // load static data
    MCPY_CHK_STATUS_RETURN(LoadStaticData(
            &iCurbeOffset));

    //----------------------------------
    // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
    //----------------------------------
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnSetVfeStateParams(
        pRenderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        pRenderData->pKernelParam->Thread_Count,
        pRenderData->iCurbeLength,
        pRenderData->iInlineLength,
        nullptr));

    //----------------------------------
    // Load kernel to GSH
    //----------------------------------
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, &pRenderData->KernelEntry);
    iKrnAllocation = pRenderHal->pfnLoadKernel(
        pRenderHal,
        pRenderData->pKernelParam,
        &MhwKernelParam,
        nullptr);
    if (iKrnAllocation < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    //----------------------------------
    // Allocate Media ID, link to kernel
    //----------------------------------
    pRenderData->iMediaID = pRenderHal->pfnAllocateMediaID(
        pRenderHal,
        iKrnAllocation,
        pRenderData->iBindingTable,
        iCurbeOffset,
        pRenderData->pKernelParam->CURBE_Length << 5,
        0,
        nullptr);
    if (pRenderData->iMediaID < 0)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Set Perf Tag
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(
        pOsInterface,
        pRenderData->PerfTag);

    // Setup Compute Walker
    pWalkerParams = nullptr;
    pComputeWalkerParams = &ComputeWalkerParams;

    RenderCopyComputerWalker(
       &ComputeWalkerParams);

    // Submit all states to render the kernel
    MCPY_CHK_STATUS_RETURN(VpHal_RndrCommonSubmitCommands(
        pRenderHal,
        nullptr,
        pRenderCopy->m_bNullHwRenderCopy,
        pWalkerParams,
        pComputeWalkerParams,
        (VpKernelID)kernelRenderCopy,
        true));

finish:
    return eStatus;
}


MOS_STATUS RenderCopy_Xe_Xpm_Plus::SetupKernel(
    int32_t iKDTIndex)
{
    Kdll_CacheEntry             *pCacheEntryTable;                              // Kernel Cache Entry table
    int32_t                     iKUID = 0 ;                                     // Kernel Unique ID (DNDI uses combined kernels)
    int32_t                     iInlineLength;                                  // Inline data length
    int32_t                     iTotalRows;                                     // Total number of row in statistics surface
    int32_t                     iTotalColumns;                                  // Total number of columns in statistics surface
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;                   // Return code
    uint32_t                    dwKernelBinSize;
    PRenderCopy_Xe_Xpm_Plus           pRenderCopy = this;
    PMEDIACOPY_RENDER_DATA      pRenderData = &(pRenderCopy->m_RenderData);
    void                        *pKernelBin;

    if (iKDTIndex == KERNEL_CopyKernel_1D_to_2D_NV12)
    {
        iKUID = IDR_VP_CopyKernel_1D_to_2D_NV12_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_2D_NV12)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_2D_NV12_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_1D_NV12)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_1D_NV12_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_1D_to_2D_Planar)
    {
        iKUID = IDR_VP_CopyKernel_1D_to_2D_RGBP_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_2D_Planar)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_2D_RGBP_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_1D_Planar)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_1D_RGBP_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_1D_to_2D_Packed)
    {
        iKUID = IDR_VP_CopyKernel_1D_to_2D_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_2D_Packed)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_2D_genx;
    }
    else if (iKDTIndex == KERNEL_CopyKernel_2D_to_1D_Packed)
    {
        iKUID = IDR_VP_CopyKernel_2D_to_1D_genx;
    }
    else
    {
        MCPY_ASSERTMESSAGE("Can't find the right kernel.");
        return MOS_STATUS_UNKNOWN;
    }

    pcKernelBin = (const void*)IGVPKRN_XE_XPM_PLUS;
    dwKernelBinSize = IGVPKRN_XE_XPM_PLUS_SIZE;

    pKernelBin = MOS_AllocMemory(dwKernelBinSize);
    MCPY_CHK_NULL_RETURN(pKernelBin);
    MOS_SecureMemcpy(pKernelBin,
                     dwKernelBinSize,
                     pcKernelBin,
                     dwKernelBinSize);

    // Allocate KDLL state (Kernel Dynamic Linking)
    m_pKernelDllState =  KernelDll_AllocateStates(
                                            pKernelBin,
                                            dwKernelBinSize,
                                            nullptr,
                                            0,
                                            nullptr,
                                            nullptr);
    if (!m_pKernelDllState)
    {
        MCPY_ASSERTMESSAGE("Failed to allocate KDLL state.");
        if (pKernelBin)
        {
            MOS_SafeFreeMemory(pKernelBin);
            if (m_pKernelDllState && m_pKernelDllState->ComponentKernelCache.pCache == pKernelBin)
            {
               m_pKernelDllState->ComponentKernelCache.pCache = nullptr;
            }
            pKernelBin = nullptr;
        }
        return MOS_STATUS_NULL_POINTER;
    }

    pCacheEntryTable =
          m_pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Set the Kernel Parameters
    pRenderData->pKernelParam = (PRENDERHAL_KERNEL_PARAM)&g_rendercopy_KernelParam_xe_xpm_plus[iKDTIndex];
    pRenderData->PerfTag = VPHAL_NONE;

    // Set Kernel entry
    pRenderData->KernelEntry.iKUID = iKUID;
    pRenderData->KernelEntry.iKCID = -1;
    pRenderData->KernelEntry.iSize = pCacheEntryTable[iKUID].iSize;
    pRenderData->KernelEntry.pBinary = pCacheEntryTable[iKUID].pBinary;

    return eStatus;
}

//!
//! \brief    setup surface states
//! \details  Setup surface states for fast 1toN
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS RenderCopy_Xe_Xpm_Plus::SetupSurfaceStates()
{
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParams;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    uint32_t                        index;
    uint32_t                        width  = 0;
    MOS_FORMAT                      format = Format_NV12;
    int32_t                         iBTEntry;
    PRENDERHAL_INTERFACE            pRenderHal;
    PRenderCopy_Xe_Xpm_Plus             pRenderCopy = this;
    PMEDIACOPY_RENDER_DATA          pRenderData = &(pRenderCopy->m_RenderData);

    pRenderHal     = pRenderCopy->m_renderHal;
    // Source surface
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));

    pRenderData->SurfMemObjCtl.SourceSurfMemObjCtl =
         pRenderHal->pOsInterface->pfnCachePolicyGetMemoryObject(
         MOS_MP_RESOURCE_USAGE_SurfaceState_FF,
         pRenderHal->pOsInterface->pfnGetGmmClientContext(pRenderHal->pOsInterface)).DwordValue;

    pRenderData->SurfMemObjCtl.TargetSurfMemObjCtl = pRenderData->SurfMemObjCtl.SourceSurfMemObjCtl;

    SurfaceParams.bAVS              = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_SRCRECT;
    SurfaceParams.bRenderTarget     = false;
    SurfaceParams.MemObjCtl         = pRenderData->SurfMemObjCtl.SourceSurfMemObjCtl;

    SurfaceParams.Type              = RENDERHAL_SURFACE_TYPE_G10;
    SurfaceParams.bWidthInDword_Y   = false;
    SurfaceParams.bWidthInDword_UV  = false;
    SurfaceParams.bWidth16Align     = false;

    if (m_currKernelId == KERNEL_CopyKernel_1D_to_2D_NV12
        || m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Planar
        || m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed)
    {
        format = m_Source.Format;
        width = m_Source.dwWidth;
        m_Source.Format = Format_RAW;
       #if defined(LINUX)
        if ((format == Format_NV12) || (format == Format_P010) || (format == Format_P016))
        {
           m_Source.dwWidth = (m_Source.dwHeight * m_Source.dwPitch) * 3 / 2;
        }
        else if ((format == Format_RGBP) || (format == Format_Y410) || (format == Format_Y416))
        {
           m_Source.dwWidth = (m_Source.dwHeight * m_Source.dwPitch) * 3;
        }
        else if ((format == Format_YUY2) || (format == Format_Y210) || (format == Format_Y216)
                 || (format == Format_AYUV) || (format == Format_Y410) || (format == Format_Y416)
                 || (format == Format_A8R8G8B8))
        {
           m_Source.dwWidth = m_Source.dwHeight * m_Source.dwPitch;
        }

       #endif
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
    SurfaceParams.bRenderTarget     = true;
    SurfaceParams.bAVS              = false;
    SurfaceParams.Boundary          = RENDERHAL_SS_BOUNDARY_DSTRECT;

    if (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_NV12
        || m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Planar
        || m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed)
    {
        format = m_Target.Format;
        width = m_Target.dwWidth;
        m_Target.Format = Format_RAW;

       #if defined(LINUX)
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

       #endif
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


MOS_STATUS RenderCopy_Xe_Xpm_Plus::LoadStaticData(
    int32_t*                        piCurbeOffset)
{
    PRENDERHAL_INTERFACE                    pRenderHal;
    DP_RENDERCOPY_NV12_STATIC_DATA          WalkerNV12Static;
    DP_RENDERCOPY_RGBP_STATIC_DATA          WalkerPlanarStatic;
    DP_RENDERCOPY_PACKED_STATIC_DATA        WalkerSinglePlaneStatic;

    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    int32_t                                 iCurbeLength = 0;
    int32_t                                 iBytePerPixelPerPlane = GetBytesPerPixel(m_Target.Format);

    PRenderCopy_Xe_Xpm_Plus                     pRenderCopy = this;
    PMEDIACOPY_RENDER_DATA                  pRenderData = &(pRenderCopy->m_RenderData);

    pRenderHal = pRenderCopy->m_renderHal;

    if ((iBytePerPixelPerPlane < 1) && (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("XE_XPM_PLUS LoadStaticData wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((m_Target.Format == Format_NV12) || ((m_Target.Format == Format_P010) ||  (m_Target.Format == Format_P016)))
    {
        // Set relevant static data
        MOS_ZeroMemory(&WalkerNV12Static, sizeof(DP_RENDERCOPY_NV12_STATIC_DATA));

        WalkerNV12Static.DW0.Inputsurfaceindex = RENDERCOPY_SRC_INDEX;
        WalkerNV12Static.DW1.Outputsurfaceindex = RENDERCOPY_DST_INDEX;
        WalkerNV12Static.DW2.Widthdword = m_Target.dwWidth * iBytePerPixelPerPlane / 4;
        WalkerNV12Static.DW3.Height = m_Target.dwHeight;
        WalkerNV12Static.DW4.ShiftLeftOffsetInBytes = m_Target.dwOffset;
        WalkerNV12Static.DW5.Widthstride = m_Target.dwWidth * iBytePerPixelPerPlane;
        WalkerNV12Static.DW6.Heightstride = m_Target.dwHeight;

        iCurbeLength = sizeof(DP_RENDERCOPY_NV12_STATIC_DATA);

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

        if( m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Planar)
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
        WalkerPlanarStatic.DW6.Widthdword = m_Target.dwWidth / 4;
        WalkerPlanarStatic.DW7.Height = m_Target.dwHeight;
        WalkerPlanarStatic.DW8.ShiftLeftOffsetInBytes = m_Source.dwOffset;
        WalkerPlanarStatic.DW9.Dst2DStartX = 0;
        WalkerPlanarStatic.DW10.Dst2DStartY = 0;

        iCurbeLength = sizeof(DP_RENDERCOPY_RGBP_STATIC_DATA);

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
        WalkerSinglePlaneStatic.DW2.WidthDWord = m_Target.dwWidth * iBytePerPixelPerPlane / 4;

        WalkerSinglePlaneStatic.DW3.Height = m_Target.dwHeight;
        WalkerSinglePlaneStatic.DW4.ShiftLeftOffsetInBytes = m_Target.dwOffset;

        if ((m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed) || (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed))
        {
            WalkerSinglePlaneStatic.DW5.ThreadHeight = m_Target.dwHeight / 32;
        }
        else if (m_currKernelId == KERNEL_CopyKernel_2D_to_2D_Packed)
        {
            WalkerSinglePlaneStatic.DW5.ThreadHeight = m_Target.dwHeight / 8;
        }
        else
        {
            MCPY_ASSERTMESSAGE("XE_XPM_PLUS LoadStaticData wrong kernel file.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        WalkerSinglePlaneStatic.DW6.Dst2DStartX = 0;
        WalkerSinglePlaneStatic.DW7.Dst2DStartY = 0;

        iCurbeLength = sizeof(DP_RENDERCOPY_PACKED_STATIC_DATA);

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
 MOS_STATUS RenderCopy_Xe_Xpm_Plus::RenderCopyComputerWalker(
 PMHW_GPGPU_WALKER_PARAMS    pWalkerParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    PRenderCopy_Xe_Xpm_Plus                     pRenderCopy = this;
    PMEDIACOPY_RENDER_DATA                  pRenderData = &(pRenderCopy->m_RenderData);
    RECT                                    AlignedRect;
    int32_t                                 iBytePerPixelPerPlane = GetBytesPerPixel(m_Target.Format);

    if ((iBytePerPixelPerPlane < 1) && (iBytePerPixelPerPlane > 8))
    {
        MCPY_ASSERTMESSAGE("XE_XPM_PLUS RenderCopyComputerWalker wrong pixel size.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((m_Target.Format == Format_YUY2) || (m_Target.Format == Format_Y210) || (m_Target.Format == Format_Y216)
        || (m_Target.Format == Format_AYUV) || (m_Target.Format == Format_Y410) || (m_Target.Format == Format_Y416)
        || (m_Target.Format == Format_A8R8G8B8))
    {
        if ((m_currKernelId == KERNEL_CopyKernel_1D_to_2D_Packed) || (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed))
        {
            m_WalkerHeightBlockSize = 32;
        }
        else if (m_currKernelId == KERNEL_CopyKernel_2D_to_2D_Packed)
        {
            m_WalkerHeightBlockSize = 8;
        }
        else
        {
            MCPY_ASSERTMESSAGE("XE_XPM_PLUS RenderCopyComputerWalker wrong kernel file.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        m_WalkerHeightBlockSize = 8;
    }

    // Set walker cmd params - Rasterscan
    MOS_ZeroMemory(pWalkerParams, sizeof(*pWalkerParams));


    AlignedRect.left   = 0;
    AlignedRect.top    = 0;
    AlignedRect.right  = m_Target.dwWidth;
    AlignedRect.bottom = m_Target.dwHeight;
    // Calculate aligned output area in order to determine the total # blocks
   // to process in case of non-16x16 aligned target.
    AlignedRect.right += m_WalkerWidthBlockSize - 1;
    AlignedRect.bottom += m_WalkerHeightBlockSize - 1;
    AlignedRect.left -= AlignedRect.left % m_WalkerWidthBlockSize;
    AlignedRect.top -= AlignedRect.top % m_WalkerHeightBlockSize;
    AlignedRect.right -= AlignedRect.right % m_WalkerWidthBlockSize;
    AlignedRect.bottom -= AlignedRect.bottom % m_WalkerHeightBlockSize;

    pWalkerParams->InterfaceDescriptorOffset = pRenderData->iMediaID;

    pWalkerParams->GroupStartingX = (AlignedRect.left / m_WalkerWidthBlockSize);
    pWalkerParams->GroupStartingY = (AlignedRect.top / m_WalkerHeightBlockSize);

    // Set number of blocks
    pRenderData->iBlocksX =
        (AlignedRect.right - AlignedRect.left) * iBytePerPixelPerPlane / m_WalkerWidthBlockSize;
    pRenderData->iBlocksY =
        (AlignedRect.bottom - AlignedRect.top) / m_WalkerHeightBlockSize;

    // Set number of blocks, block size is m_WalkerWidthBlockSize x m_WalkerHeightBlockSize.
    if ((m_Target.Format == Format_YUY2) || (m_Target.Format == Format_Y210) || (m_Target.Format == Format_Y216)
        || (m_Target.Format == Format_AYUV) || (m_Target.Format == Format_Y410) || (m_Target.Format == Format_Y416)
        || (m_Target.Format == Format_A8R8G8B8))
    {
        pWalkerParams->GroupWidth = ((m_Target.dwWidth * iBytePerPixelPerPlane + m_WalkerWidthBlockSize - 1)
                                      & (~(m_WalkerWidthBlockSize - 1))) /  m_WalkerWidthBlockSize;
    }
    else
    {
        pWalkerParams->GroupWidth = pRenderData->iBlocksX;
    }

    pWalkerParams->GroupHeight = pRenderData->iBlocksY; // hight/m_WalkerWidthBlockSize

    pWalkerParams->ThreadWidth = 1;
    pWalkerParams->ThreadHeight = 1;
    pWalkerParams->ThreadDepth = 1;
    pWalkerParams->IndirectDataStartAddress = pRenderData->iCurbeOffset;
    // Indirect Data Length is a multiple of 64 bytes (size of L3 cacheline). Bits [5:0] are zero.
    pWalkerParams->IndirectDataLength = MOS_ALIGN_CEIL(pRenderData->iCurbeLength, 1 << MHW_COMPUTE_INDIRECT_SHIFT);
    pWalkerParams->BindingTableID = pRenderData->iBindingTable;

    return eStatus;
}
