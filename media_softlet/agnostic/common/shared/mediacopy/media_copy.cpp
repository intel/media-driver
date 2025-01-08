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
//! \file     media_copy.cpp
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#include "media_copy.h"
#include "media_copy_common.h"
#include "media_debug_dumper.h"
#include "mhw_cp_interface.h"
#include "mos_utilities.h"
#include "mos_util_debug.h"

#define BLT_MAX_WIDTH  (1 << 16) - 1
#define BLT_MAX_HEIGHT (1 << 16) - 1
#define BLT_MAX_PITCH  (1 << 18) - 1

#define VE_MIN_WIDTH  64
#define VE_MIN_HEIGHT 32

#define RENDER_MIN_WIDTH  16
#define RENDER_MIN_HEIGHT 16

MediaCopyBaseState::MediaCopyBaseState():
    m_osInterface(nullptr)
{

}

MediaCopyBaseState::~MediaCopyBaseState()
{
    if (m_osInterface)
    {
        m_osInterface->pfnDestroy(m_osInterface, false);
        MOS_FreeMemory(m_osInterface);
        m_osInterface = nullptr;
    }

    if (m_inUseGPUMutex)
    {
        MosUtilities::MosDestroyMutex(m_inUseGPUMutex);
        m_inUseGPUMutex = nullptr;
    }

   #if (_DEBUG || _RELEASE_INTERNAL)
    if (m_surfaceDumper != nullptr)
    {
       MOS_Delete(m_surfaceDumper);
       m_surfaceDumper = nullptr;
    }
   #endif
}

//!
//! \brief    init Media copy
//! \details  init func.
//! \param    none
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if success, otherwise return failed.
//!
MOS_STATUS MediaCopyBaseState::Initialize(PMOS_INTERFACE osInterface)
{
    if (m_inUseGPUMutex == nullptr)
    {
        m_inUseGPUMutex     = MosUtilities::MosCreateMutex();
        MCPY_CHK_NULL_RETURN(m_inUseGPUMutex);
    }
    MCPY_CHK_NULL_RETURN(m_osInterface);
    Mos_SetVirtualEngineSupported(m_osInterface, true);
    m_osInterface->pfnVirtualEngineSupported(m_osInterface, true, true);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_surfaceDumper == nullptr)
    {
       m_surfaceDumper = MOS_New(CommonSurfaceDumper, osInterface);
       MOS_OS_CHK_NULL_RETURN(m_surfaceDumper);
    }
    MediaUserSettingSharedPtr           userSettingPtr = nullptr;

    m_surfaceDumper->GetSurfaceDumpLocation(m_dumpLocation_in, mcpy_in);
    m_surfaceDumper->GetSurfaceDumpLocation(m_dumpLocation_out, mcpy_out);
    if (m_osInterface)
    {
        userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
        ReadUserSettingForDebug(
            userSettingPtr,
            m_MCPYForceMode,
            __MEDIA_USER_FEATURE_SET_MCPY_FORCE_MODE,
            MediaUserSetting::Group::Device);

        ReadUserSettingForDebug(
            userSettingPtr,
            m_enableVeCopySmallRes,
            __MEDIA_USER_FEATURE_ENABLE_VECOPY_SMALL_RESOLUTION,
            MediaUserSetting::Group::Device);
    }
#endif
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    check copy capability.
//! \details  to determine surface copy is supported or not.
//! \param    none
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
//!
MOS_STATUS MediaCopyBaseState::CapabilityCheck(
    MOS_FORMAT         format,
    MCPY_STATE_PARAMS &mcpySrc,
    MCPY_STATE_PARAMS &mcpyDst,
    MCPY_ENGINE_CAPS  &caps,
    MCPY_METHOD        preferMethod)
{
    // derivate class specific check. include HW engine avaliable check.
    MCPY_CHK_STATUS_RETURN(FeatureSupport(mcpySrc.OsRes, mcpyDst.OsRes, mcpySrc, mcpyDst, caps));

    // common policy check
    // legal check
    // Blt engine does not support protection, allow the copy if dst is staging buffer in system mem
    if (preferMethod == MCPY_METHOD_POWERSAVING &&
        (mcpySrc.CpMode == MCPY_CPMODE_CP || mcpyDst.CpMode == MCPY_CPMODE_CP))
    {
        MCPY_ASSERTMESSAGE("illegal usage");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (MCPY_METHOD_PERFORMANCE == preferMethod && !caps.engineRender)
    {
        MCPY_ASSERTMESSAGE("No cap support for render copy, so app could not prefer to use EU copy");
        return MOS_STATUS_INVALID_PARAMETER;            // Bypass media copy, and let APP handle it.
    }
    // vebox cap check.
    if (!IsVeboxCopySupported(mcpySrc.OsRes, mcpyDst.OsRes) || // format check, implemented on Gen derivate class.
        mcpySrc.bAuxSuface)
    {
        caps.engineVebox = false;
        // temp solution for FP16 enabling on new platform
        if (format == Format_A16B16G16R16F || format == Format_A16R16G16B16F)
        {
            return MOS_STATUS_UNIMPLEMENTED;
        }
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

//!
//! \brief    surface copy pre process.
//! \details  pre process before doing surface copy.
//! \param    none
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
//!
MOS_STATUS MediaCopyBaseState::PreCheckCpCopy(
    MCPY_STATE_PARAMS src, MCPY_STATE_PARAMS dest, MCPY_METHOD preferMethod)
{
    if (preferMethod == MCPY_METHOD_POWERSAVING &&
        (src.CpMode == MCPY_CPMODE_CP || dest.CpMode == MCPY_CPMODE_CP))
    {
        MCPY_ASSERTMESSAGE("BLT Copy with CP is not supported");
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    dispatch copy task if support.
//! \details  dispatch copy task to HW engine (vebox, EU, Blt) based on customer and default.
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
//!
MOS_STATUS MediaCopyBaseState::CopyEnigneSelect(MCPY_METHOD& preferMethod, MCPY_ENGINE& mcpyEngine, MCPY_ENGINE_CAPS& caps)
{
    // driver should make sure there is at least one he can process copy even customer choice doesn't match caps.
    switch (preferMethod)
    {
        case MCPY_METHOD_PERFORMANCE:
            mcpyEngine = caps.engineRender?MCPY_ENGINE_RENDER:(caps.engineBlt ? MCPY_ENGINE_BLT : MCPY_ENGINE_VEBOX);
            break;
        case MCPY_METHOD_BALANCE:
            mcpyEngine = caps.engineVebox?MCPY_ENGINE_VEBOX:(caps.engineBlt?MCPY_ENGINE_BLT:MCPY_ENGINE_RENDER);
            break;
        case MCPY_METHOD_POWERSAVING:
        case MCPY_METHOD_DEFAULT:
            mcpyEngine = caps.engineBlt?MCPY_ENGINE_BLT:(caps.engineVebox?MCPY_ENGINE_VEBOX:MCPY_ENGINE_RENDER);
            break;
        default:
            break;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (MCPY_METHOD_PERFORMANCE == m_MCPYForceMode)
    {
        mcpyEngine = MCPY_ENGINE_RENDER;
    }
    else if (MCPY_METHOD_POWERSAVING == m_MCPYForceMode)
    {
        mcpyEngine = MCPY_ENGINE_BLT;
    }
    else if (MCPY_METHOD_BALANCE == m_MCPYForceMode)
    {
        mcpyEngine = MCPY_ENGINE_VEBOX;
    }
    else if (MCPY_METHOD_DEFAULT != m_MCPYForceMode)
    {
        return MOS_STATUS_INVALID_PARAMETER; // bypass copy engine, just let APP handle it.
    }
#endif
    return MOS_STATUS_SUCCESS;
}

uint32_t GetMinRequiredSurfaceSizeInBytes(uint32_t pitch, uint32_t height, MOS_FORMAT format)
{
    uint32_t nBytes = 0;
    switch (format)
    {
    case Format_NV12:
    case Format_YV12:
    case Format_I420:
    case Format_P010:
    case Format_P016:
        nBytes = pitch * height + (pitch >> 1) * (height >> 1) + (pitch >> 1) * (height >> 1);
        break;
    case Format_RGBP:
    case Format_BGRP:
        nBytes = pitch * height + pitch * height + pitch * height;
        break;
    case Format_Y410:
    case Format_Y416:
    case Format_Y210:
    case Format_Y216:
    case Format_YUY2:
    case Format_R5G6B5:
    case Format_R8G8B8:
    case Format_A8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8R8G8B8:
    case Format_X8B8G8R8:
    case Format_AYUV:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_P8:
    case Format_L8:
    case Format_A8:
    case Format_Y16U:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_IRW0:
    case Format_IRW1:
    case Format_IRW2:
    case Format_IRW3:
        nBytes = pitch * height;
        break;
    default:
        MCPY_ASSERTMESSAGE("Unsupported format!");
        break;
    }
    return nBytes;
}

MOS_STATUS MediaCopyBaseState::CheckResourceSizeValidForCopy(const MOS_SURFACE &res, const MCPY_ENGINE method)
{
    if (res.TileType != MOS_TILE_LINEAR)
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t nBytes = GetMinRequiredSurfaceSizeInBytes(res.dwPitch, res.dwHeight, res.Format);
    if (nBytes == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (res.dwSize < nBytes)
    {
        MT_ERR2(MT_MEDIA_COPY, 
        MT_MEDIA_COPY_DATASIZE, nBytes,
        MT_MEDIA_COPY_DATASIZE, res.dwSize);

        return MOS_STATUS_INVALID_PARAMETER;
    }
    
    if (method == MCPY_ENGINE_BLT)
    {
        if (res.dwPitch > BLT_MAX_PITCH || res.dwHeight > BLT_MAX_HEIGHT || res.dwWidth > BLT_MAX_WIDTH)
        {
            MT_ERR3(MT_MEDIA_COPY,
                MT_SURF_WIDTH, res.dwWidth,
                MT_SURF_HEIGHT, res.dwHeight,
                MT_SURF_PITCH, res.dwPitch);
            MCPY_ASSERTMESSAGE("Surface size overflow! pitch %d, height %d, width %d", res.dwPitch, res.dwHeight, res.dwWidth);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    if (method == MCPY_ENGINE_RENDER)
    {
        if (res.dwHeight < RENDER_MIN_HEIGHT || res.dwWidth < RENDER_MIN_WIDTH)
        {
            MT_ERR2(MT_MEDIA_COPY,
                MT_SURF_WIDTH, res.dwWidth,
                MT_SURF_HEIGHT, res.dwHeight);
            MCPY_ASSERTMESSAGE("Surface size not meet min requirement! height %d, width %d", res.dwHeight, res.dwWidth);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    if (method == MCPY_ENGINE_VEBOX)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        if (!m_enableVeCopySmallRes)
#endif
        {
            if (res.dwHeight < VE_MIN_HEIGHT || res.dwWidth < VE_MIN_WIDTH)
            {
                MT_ERR2(MT_MEDIA_COPY,
                    MT_SURF_WIDTH, res.dwWidth,
                    MT_SURF_HEIGHT, res.dwHeight);
                MCPY_ASSERTMESSAGE("Surface size not meet min requirement! height %d, width %d", res.dwHeight, res.dwWidth);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyBaseState::ValidateResource(const MOS_SURFACE &src, const MOS_SURFACE &dst, MCPY_ENGINE method)
{
    // For CP buffer copy, CP will handle the overflown size, skip size check
    if (src.OsResource.pGmmResInfo->GetResourceType() == RESOURCE_BUFFER &&
        dst.OsResource.pGmmResInfo->GetResourceType() == RESOURCE_BUFFER &&
        method == MCPY_ENGINE_BLT)
    {
        return MOS_STATUS_SUCCESS;
    }

    MCPY_CHK_STATUS_RETURN(CheckResourceSizeValidForCopy(src, method));
    MCPY_CHK_STATUS_RETURN(CheckResourceSizeValidForCopy(dst, method));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    surface copy func.
//! \details  copy surface.
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
//!
MOS_STATUS MediaCopyBaseState::SurfaceCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst, MCPY_METHOD preferMethod)
{
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_SURFACE SrcResDetails, DstResDetails;
    MOS_ZeroMemory(&SrcResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&DstResDetails, sizeof(MOS_SURFACE));
    SrcResDetails.Format     = Format_Invalid;
    SrcResDetails.OsResource = *src;
    DstResDetails.Format     = Format_Invalid;
    DstResDetails.OsResource = *dst;

    MCPY_STATE_PARAMS     mcpySrc = {nullptr, MOS_MMC_DISABLED, MOS_TILE_LINEAR, MCPY_CPMODE_CLEAR, false};
    MCPY_STATE_PARAMS     mcpyDst = {nullptr, MOS_MMC_DISABLED, MOS_TILE_LINEAR, MCPY_CPMODE_CLEAR, false};
    MCPY_ENGINE           mcpyEngine = MCPY_ENGINE_BLT;
    MCPY_ENGINE_CAPS      mcpyEngineCaps = {1, 1, 1, 1};

    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, src, &SrcResDetails));
    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, src, (PMOS_MEMCOMP_STATE)&(mcpySrc.CompressionMode)));
    mcpySrc.CpMode   = src->pGmmResInfo->GetSetCpSurfTag(false, 0)?MCPY_CPMODE_CP:MCPY_CPMODE_CLEAR;
    mcpySrc.TileMode = SrcResDetails.TileType;
    mcpySrc.OsRes    = src;
    MCPY_NORMALMESSAGE("input surface's format %d, width %d; hight %d, pitch %d, tiledmode %d, mmc mode %d, cp mode %d, ResType %d, preferMethod %d",
        SrcResDetails.Format,
        SrcResDetails.dwWidth,
        SrcResDetails.dwHeight,
        SrcResDetails.dwPitch,
        mcpySrc.TileMode,
        mcpySrc.CompressionMode,
        mcpySrc.CpMode,
        src->pGmmResInfo->GetResourceType(),
        preferMethod);
    MT_LOG7(MT_MEDIA_COPY, MT_NORMAL, 
        MT_SURF_PITCH,          SrcResDetails.dwPitch,
        MT_SURF_HEIGHT,         SrcResDetails.dwHeight,
        MT_SURF_WIDTH,          SrcResDetails.dwWidth,
        MT_SURF_MOS_FORMAT,     SrcResDetails.Format,
        MT_MEDIA_COPY_DATASIZE, SrcResDetails.dwSize,
        MT_SURF_TILE_TYPE,      SrcResDetails.TileType,
        MT_SURF_COMP_MODE,      mcpySrc.CompressionMode);

    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, dst, &DstResDetails));
    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,dst, (PMOS_MEMCOMP_STATE) &(mcpyDst.CompressionMode)));
    mcpyDst.CpMode   = dst->pGmmResInfo->GetSetCpSurfTag(false, 0)?MCPY_CPMODE_CP:MCPY_CPMODE_CLEAR;
    mcpyDst.TileMode = DstResDetails.TileType;
    mcpyDst.OsRes    = dst;
    MCPY_NORMALMESSAGE("Output surface's format %d, width %d; hight %d, pitch %d, tiledmode %d, mmc mode %d,cp mode %d, ResType %d",
        DstResDetails.Format,
        DstResDetails.dwWidth,
        DstResDetails.dwHeight,
        DstResDetails.dwPitch,
        mcpyDst.TileMode,
        mcpyDst.CompressionMode,
        mcpyDst.CpMode,
        dst->pGmmResInfo->GetResourceType());
    MT_LOG7(MT_MEDIA_COPY, MT_NORMAL, 
        MT_SURF_PITCH,          DstResDetails.dwPitch,
        MT_SURF_HEIGHT,         DstResDetails.dwHeight,
        MT_SURF_WIDTH,          DstResDetails.dwWidth,
        MT_SURF_MOS_FORMAT,     DstResDetails.Format,
        MT_MEDIA_COPY_DATASIZE, DstResDetails.dwSize,
        MT_SURF_TILE_TYPE,      DstResDetails.TileType,
        MT_SURF_COMP_MODE,      mcpyDst.CompressionMode);

#if (_DEBUG || _RELEASE_INTERNAL) && !defined(LINUX)
    TRACEDATA_MEDIACOPY eventData = {0};
    TRACEDATA_MEDIACOPY_INIT(
        eventData,
        src->AllocationInfo.m_AllocationHandle,
        SrcResDetails.dwWidth,
        SrcResDetails.dwHeight,
        SrcResDetails.Format,
        *((int64_t *)&src->pGmmResInfo->GetResFlags().Gpu),
        *((int64_t *)&src->pGmmResInfo->GetResFlags().Info),
        src->pGmmResInfo->GetSetCpSurfTag(0, 0),
        dst->AllocationInfo.m_AllocationHandle,
        DstResDetails.dwWidth,
        DstResDetails.dwHeight,
        DstResDetails.Format,
        *((int64_t *)&dst->pGmmResInfo->GetResFlags().Gpu),
        *((int64_t *)&dst->pGmmResInfo->GetResFlags().Info),
        dst->pGmmResInfo->GetSetCpSurfTag(0, 0)
    );

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_INFO, &eventData, sizeof(eventData), nullptr, 0);
#endif

    MCPY_CHK_STATUS_RETURN(PreCheckCpCopy(mcpySrc, mcpyDst, preferMethod));

    MCPY_CHK_STATUS_RETURN(CapabilityCheck(SrcResDetails.Format,
        mcpySrc, mcpyDst,
        mcpyEngineCaps, preferMethod));

    MCPY_CHK_STATUS_RETURN(CopyEnigneSelect(preferMethod, mcpyEngine, mcpyEngineCaps));

    MCPY_CHK_STATUS_RETURN(ValidateResource(SrcResDetails, DstResDetails, mcpyEngine));

    MCPY_CHK_STATUS_RETURN(TaskDispatch(mcpySrc, mcpyDst, mcpyEngine));

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return eStatus;
}

MOS_STATUS MediaCopyBaseState::TaskDispatch(MCPY_STATE_PARAMS mcpySrc, MCPY_STATE_PARAMS mcpyDst, MCPY_ENGINE mcpyEngine)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;


#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_SURFACE sourceSurface = {};
    MOS_SURFACE targetSurface = {};

    targetSurface.Format = Format_Invalid;
    targetSurface.OsResource = *mcpyDst.OsRes;

#if !defined(LINUX) && !defined(ANDROID) && !EMUL
    MOS_ZeroMemory(&targetSurface.OsResource.AllocationInfo, sizeof(SResidencyInfo));
#endif

    sourceSurface.Format = Format_Invalid;
    sourceSurface.OsResource = *mcpySrc.OsRes;

    m_osInterface->pfnGetResourceInfo(m_osInterface, &sourceSurface.OsResource, &sourceSurface);
    m_osInterface->pfnGetResourceInfo(m_osInterface, &targetSurface.OsResource, &targetSurface);

    // Set the dump location like "dumpLocation before MCPY=path_to_dump_folder" in user feature configure file
    // Otherwise, the surface may not be dumped
    // Only dump linear surface
    if (m_surfaceDumper && mcpySrc.TileMode == MOS_TILE_LINEAR)
    {
        if ((*m_dumpLocation_in == '\0') || (*m_dumpLocation_in == ' '))
        {
            MCPY_NORMALMESSAGE("Invalid dump location set, the surface will not be dumped");
        }
        else
        {
            m_surfaceDumper->DumpSurfaceToFile(m_osInterface, &sourceSurface, m_dumpLocation_in, m_surfaceDumper->m_frameNum, true, false, nullptr);
        }
    }
#endif

    MosUtilities::MosLockMutex(m_inUseGPUMutex);
    switch(mcpyEngine)
    {
        case MCPY_ENGINE_VEBOX:
            eStatus = MediaVeboxCopy(mcpySrc.OsRes, mcpyDst.OsRes);
            break;
        case MCPY_ENGINE_BLT:
            if ((mcpyDst.TileMode != MOS_TILE_LINEAR) && (mcpyDst.CompressionMode == MOS_MMC_RC))
            {
                MCPY_NORMALMESSAGE("mmc on, mcpyDst.TileMode= %d, mcpyDst.CompressionMode = %d", mcpyDst.TileMode, mcpyDst.CompressionMode);
                eStatus = m_osInterface->pfnDecompResource(m_osInterface, mcpyDst.OsRes);
                if (MOS_STATUS_SUCCESS != eStatus)
                {
                    MosUtilities::MosUnlockMutex(m_inUseGPUMutex);
                    MCPY_CHK_STATUS_RETURN(eStatus);
                }
            }
            eStatus = MediaBltCopy(mcpySrc.OsRes, mcpyDst.OsRes);
            break;
        case MCPY_ENGINE_RENDER:
            eStatus = MediaRenderCopy(mcpySrc.OsRes, mcpyDst.OsRes);
            break;
        default:
            break;
    }
    MosUtilities::MosUnlockMutex(m_inUseGPUMutex);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_bRegReport)
    {
        std::string copyEngine = mcpyEngine ?(mcpyEngine == MCPY_ENGINE_BLT?"BLT":"Render"):"VeBox";
        MediaUserSettingSharedPtr userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
        ReportUserSettingForDebug(
            userSettingPtr,
            __MEDIA_USER_FEATURE_MCPY_MODE,
            copyEngine,
            MediaUserSetting::Group::Device);
    }

    // Set the dump location like "dumpLocation after MCPY=path_to_dump_folder" in user feature configure file
    // Otherwise, the surface may not be dumped
    // Only dump linear surface
    if (m_surfaceDumper && mcpyDst.TileMode == MOS_TILE_LINEAR)
    {
        if ((*m_dumpLocation_out == '\0') || (*m_dumpLocation_out == ' '))
        {
            MCPY_NORMALMESSAGE("Invalid dump location set, the surface will not be dumped");
        }
        else
        {
            m_surfaceDumper->DumpSurfaceToFile(m_osInterface, &targetSurface, m_dumpLocation_out, m_surfaceDumper->m_frameNum, true, false, nullptr);
        }
        m_surfaceDumper->m_frameNum++;
    }
#endif
    MCPY_NORMALMESSAGE("Media Copy works on %s Engine", mcpyEngine ?(mcpyEngine == MCPY_ENGINE_BLT?"BLT":"Render"):"VeBox");

    return eStatus;
}

//!
//! \brief    aux surface copy.
//! \details  copy surface.
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if support, otherwise return unspoort.
//!
MOS_STATUS MediaCopyBaseState::AuxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // only support form Gen12+, will implement on deriavete class.
    MCPY_ASSERTMESSAGE("doesn't support");
    return MOS_STATUS_INVALID_HANDLE;
}

PMOS_INTERFACE MediaCopyBaseState::GetMosInterface()
{
    return m_osInterface;
}