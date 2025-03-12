/*
* Copyright (c) 2020, Intel Corporation
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
//! \file       media_render_copy_next.cpp
//! \brief      render copy implement file
//! \details    render copy implement file
//!

#include "media_render_copy_next.h"
#include "media_interfaces_mhw_next.h"
#include "media_render_common.h"

//!
//! \brief Initialize MHW Kernel Param struct for loading Kernel
//!
#define INIT_MHW_KERNEL_PARAM(MhwKernelParam, _pKernelEntry)                        \
    do                                                                              \
    {                                                                               \
        MOS_ZeroMemory(&(MhwKernelParam), sizeof(MhwKernelParam));                  \
        (MhwKernelParam).pBinary  = (_pKernelEntry)->pBinary;                       \
        (MhwKernelParam).iSize    = (_pKernelEntry)->iSize;                         \
        (MhwKernelParam).iKUID    = (_pKernelEntry)->iKUID;                         \
        (MhwKernelParam).iKCID    = (_pKernelEntry)->iKCID;                         \
        (MhwKernelParam).iPaddingSize = (_pKernelEntry)->iPaddingSize;              \
    } while(0)


RenderCopyStateNext::RenderCopyStateNext(PMOS_INTERFACE osInterface, MhwInterfacesNext *mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(mhwInterfaces)
{
    if (nullptr == osInterface)
    {
        MCPY_ASSERTMESSAGE("osInterface is nullptr");
        return;
    }
    m_RenderData.pKernelParam = (PRENDERHAL_KERNEL_PARAM)g_rendercopy_KernelParam;
    Mos_SetVirtualEngineSupported(osInterface, true);
    osInterface->pfnVirtualEngineSupported(osInterface, true, false);

    MOS_NULL_RENDERING_FLAGS        NullRenderingFlags;
    NullRenderingFlags = osInterface->pfnGetNullHWRenderFlags(osInterface);

    m_bNullHwRenderCopy =
     NullRenderingFlags.VPComp ||
     NullRenderingFlags.VPGobal;
}

RenderCopyStateNext:: ~RenderCopyStateNext()
{
    if (m_renderHal != nullptr)
    {
        if (m_renderHal->pfnDestroy)
        {
            MOS_STATUS eStatus = m_renderHal->pfnDestroy(m_renderHal);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MCPY_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
            }
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
            MCPY_ASSERTMESSAGE("Failed to destroy cpInterface.");
        }
    }

    // Destroy Kernel DLL objects (cache, hash table, states)
    if (m_pKernelDllState)
    {
       KernelDll_ReleaseStates(m_pKernelDllState);
       m_pKernelBin = nullptr;
    }
}

MOS_STATUS RenderCopyStateNext::Initialize()
{
    RENDERHAL_SETTINGS      RenderHalSettings;

    MCPY_CHK_NULL_RETURN(m_osInterface);

    m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(*m_renderHal));
    MCPY_CHK_NULL_RETURN(m_renderHal);
    MCPY_CHK_STATUS_RETURN(RenderHal_InitInterface(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));

    // Allocate and initialize HW states
    RenderHalSettings.iMediaStates = 32;
    MCPY_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    m_renderHal->sseuTable              = defaultSSEUTable;
    m_renderHal->forceDisablePreemption = true;

    return MOS_STATUS_SUCCESS;
}

int32_t RenderCopyStateNext::GetBytesPerPixelPerPlane(
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

MOS_STATUS RenderCopyStateNext::SetupKernel(
    int32_t iKDTIndex)
{
    Kdll_CacheEntry* pCacheEntryTable;                              // Kernel Cache Entry table
    int32_t                     iKUID = 0;                                     // Kernel Unique ID
    int32_t                     iInlineLength;                                  // Inline data length
    int32_t                     iTotalRows;                                     // Total number of row in statistics surface
    int32_t                     iTotalColumns;                                  // Total number of columns in statistics surface
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;                   // Return code
    uint32_t                    dwKernelBinSize;
    PMEDIACOPY_RENDER_DATA      pRenderData = &m_RenderData;
    const void* pcKernelBin = nullptr;

    MCPY_CHK_NULL_RETURN(pRenderData);
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

    pcKernelBin = m_KernelBin;
    dwKernelBinSize = m_KernelBinSize;

    if (nullptr == m_pKernelBin)
    {
        m_pKernelBin = MOS_AllocMemory(dwKernelBinSize);
    }

    MCPY_CHK_NULL_RETURN(m_pKernelBin);
    MOS_SecureMemcpy(m_pKernelBin,
        dwKernelBinSize,
        pcKernelBin,
        dwKernelBinSize);

    // Allocate KDLL state (Kernel Dynamic Linking)
    if (nullptr == m_pKernelDllState)
    {
        m_pKernelDllState = KernelDll_AllocateStates(
            m_pKernelBin,
            dwKernelBinSize,
            nullptr,
            0,
            nullptr,
            nullptr);
    }
    if (nullptr == m_pKernelDllState)
    {
        MCPY_ASSERTMESSAGE("Failed to allocate KDLL state.");
        if (m_pKernelBin)
        {
            MOS_SafeFreeMemory(m_pKernelBin);
            m_pKernelBin = nullptr;
        }
        return MOS_STATUS_NULL_POINTER;
    }
    pCacheEntryTable =
        m_pKernelDllState->ComponentKernelCache.pCacheEntries;

    // Set the Kernel Parameters
    pRenderData->pKernelParam = (PRENDERHAL_KERNEL_PARAM)&g_rendercopy_KernelParam[iKDTIndex];
    pRenderData->PerfTag = VPHAL_NONE;

    // Set Kernel entry
    pRenderData->KernelEntry.iKUID = iKUID;
    pRenderData->KernelEntry.iKCID = -1;
    pRenderData->KernelEntry.iSize = pCacheEntryTable[iKUID].iSize;
    pRenderData->KernelEntry.pBinary = pCacheEntryTable[iKUID].pBinary;

    return eStatus;
}

MOS_STATUS RenderCopyStateNext::SubmitCMD()
{
    PRENDERHAL_INTERFACE        pRenderHal;
    PMOS_INTERFACE              pOsInterface;
    MHW_KERNEL_PARAM            MhwKernelParam;
    int32_t                     iKrnAllocation;
    int32_t                     iCurbeOffset;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    PMEDIACOPY_RENDER_DATA      pRenderData = &m_RenderData;
    MHW_WALKER_PARAMS           WalkerParams = {0};
    PMHW_WALKER_PARAMS          pWalkerParams = nullptr;
    MHW_GPGPU_WALKER_PARAMS     ComputeWalkerParams = {0};
    PMHW_GPGPU_WALKER_PARAMS    pComputeWalkerParams = nullptr;
    MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption = {};

    pRenderHal = m_renderHal;
    pOsInterface = m_osInterface;
    // no gpucontext will be created if the gpu context has been created before.
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_COMPUTE,
        MOS_GPU_NODE_COMPUTE,
        &createOption));

    // Register context with the Batch Buffer completion event
    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_COMPUTE));

    // Set GPU Context to Render Engine
    MCPY_CHK_STATUS_RETURN(pOsInterface->pfnSetGpuContext(pOsInterface, MOS_GPU_CONTEXT_COMPUTE));

    // Reset allocation list and house keeping
    m_osInterface->pfnResetOsStates(pOsInterface);

    // Register the resource of GSH
    MCPY_CHK_STATUS_RETURN(pRenderHal->pfnReset(pRenderHal));

    // Set copy kernel
    SetupKernel(m_currKernelId);

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
        return eStatus;
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
        return eStatus;
    }

    // Set Perf Tag
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, RENDER_COPY);

    // Setup Compute Walker
    pWalkerParams = nullptr;
    pComputeWalkerParams = &ComputeWalkerParams;

    RenderCopyComputerWalker(
        &ComputeWalkerParams);

    // Submit all states to render the kernel
    //VpHal_RndrCommonSubmitCommands(
    MediaRenderCommon::EukernelSubmitCommands(
        pRenderHal,
        nullptr,
        m_bNullHwRenderCopy,
        pWalkerParams,
        pComputeWalkerParams,
        (VpKernelID)kernelRenderCopy,
        true);

    return eStatus;
}

MOS_STATUS RenderCopyStateNext::GetCurentKernelID( )
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
MOS_STATUS RenderCopyStateNext::SetupSurfaceStates()
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
    RENDERHAL_SURFACE               RenderHalTarget = {};
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
        // set NV12 as 2 plane because render copy only uses DP reading&writing.
        RenderHalSource.SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
        RenderHalTarget.SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
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
        MCPY_CHK_STATUS_RETURN(MediaRenderCommon::Set1DSurfaceForHwAccess(
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
        MCPY_CHK_STATUS_RETURN(MediaRenderCommon::Set2DSurfaceForHwAccess(
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
        MCPY_CHK_STATUS_RETURN(MediaRenderCommon::Set1DSurfaceForHwAccess(
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
        MCPY_CHK_STATUS_RETURN(MediaRenderCommon::Set2DSurfaceForHwAccess(
            pRenderHal,
            &m_Target,
            &RenderHalTarget,
            &SurfaceParams,
            pRenderData->iBindingTable,
            RENDERCOPY_DST_INDEX,
            true));

    }

    return eStatus;
}


MOS_STATUS RenderCopyStateNext::LoadStaticData(
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
 MOS_STATUS RenderCopyStateNext::RenderCopyComputerWalker(
 PMHW_GPGPU_WALKER_PARAMS    pWalkerParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    PMEDIACOPY_RENDER_DATA                  pRenderData = &m_RenderData;
    RECT                                    AlignedRect;
    int32_t                                 iBytePerPixelPerPlane = GetBytesPerPixelPerPlane(m_Target.Format);

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
            m_WalkerHeightBlockSize = 32;
        }
        else if (m_currKernelId == KERNEL_CopyKernel_2D_to_2D_Packed)
        {
            m_WalkerHeightBlockSize = 8;
        }
        else
        {
            MCPY_ASSERTMESSAGE("RenderCopyComputerWalker wrong kernel file.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        m_WalkerHeightBlockSize = 8;
    }

    if ((m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Packed) ||
        (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_NV12) ||
        (m_currKernelId == KERNEL_CopyKernel_2D_to_1D_Planar))
    {
        m_WalkerWidthBlockSize = 16;
    }
    else
    {
        m_WalkerWidthBlockSize = 128;
    }

    // Set walker cmd params - Rasterscan
    MOS_ZeroMemory(pWalkerParams, sizeof(*pWalkerParams));


    AlignedRect.left   = 0;
    AlignedRect.top    = 0;
    AlignedRect.right  = (m_Source.dwPitch < m_Target.dwPitch) ? m_Source.dwPitch : m_Target.dwPitch;
    AlignedRect.bottom = (m_Source.dwHeight < m_Target.dwHeight) ? m_Source.dwHeight : m_Target.dwHeight;
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
        ((AlignedRect.right - AlignedRect.left) + m_WalkerWidthBlockSize - 1) / m_WalkerWidthBlockSize;
    pRenderData->iBlocksY =
        ((AlignedRect.bottom - AlignedRect.top) + m_WalkerHeightBlockSize -1)/ m_WalkerHeightBlockSize;

    // Set number of blocks, block size is m_WalkerWidthBlockSize x m_WalkerHeightBlockSize.
    pWalkerParams->GroupWidth = pRenderData->iBlocksX;
    pWalkerParams->GroupHeight = pRenderData->iBlocksY; // hight/m_WalkerWidthBlockSize

    pWalkerParams->ThreadWidth = 1;
    pWalkerParams->ThreadHeight = 1;
    pWalkerParams->ThreadDepth = 1;
    pWalkerParams->IndirectDataStartAddress = pRenderData->iCurbeOffset;
    // Indirect Data Length is a multiple of 64 bytes (size of L3 cacheline). Bits [5:0] are zero.
    pWalkerParams->IndirectDataLength = MOS_ALIGN_CEIL(pRenderData->iCurbeLength, 1 << MHW_COMPUTE_INDIRECT_SHIFT);
    pWalkerParams->BindingTableID = pRenderData->iBindingTable;
    MCPY_NORMALMESSAGE("WidthBlockSize %d, HeightBlockSize %d, Widththreads %d, Heightthreads%d",
        m_WalkerWidthBlockSize, m_WalkerHeightBlockSize, pWalkerParams->GroupWidth, pWalkerParams->GroupHeight);

    return eStatus;
}

 MOS_STATUS RenderCopyStateNext::CopySurface(
     PMOS_RESOURCE src,
     PMOS_RESOURCE dst)
 {
     MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
     m_Source.OsResource = *src;
     m_Source.Format = Format_Invalid;
     m_osInterface->pfnGetResourceInfo(m_osInterface, src, &m_Source);

     m_Target.OsResource = *dst;
     m_Target.Format = Format_Invalid;
     m_osInterface->pfnGetResourceInfo(m_osInterface, dst, &m_Target);

     if ((m_Target.Format != Format_RGBP) && (m_Target.Format != Format_NV12) && (m_Target.Format != Format_RGB)
         && (m_Target.Format != Format_P010) && (m_Target.Format != Format_P016) && (m_Target.Format != Format_YUY2)
         && (m_Target.Format != Format_Y210) && (m_Target.Format != Format_Y216) && (m_Target.Format != Format_AYUV)
         && (m_Target.Format != Format_Y410) && (m_Target.Format != Format_Y416) && (m_Target.Format != Format_A8R8G8B8))
     {
         MCPY_ASSERTMESSAGE("Can't suppport format %d ", m_Target.Format);
         return MOS_STATUS_INVALID_PARAMETER;
     }

     MCPY_CHK_STATUS_RETURN(GetCurentKernelID());
     return SubmitCMD();
 }