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
//! \file     media_blt_copy_xe_hpm.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#include "media_blt_copy_xe_hpm.h"
#include "mhw_cp_interface.h"
#include "media_copy.h"
#include "mhw_mi.h"
#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_utilities.h"
#include "media_perf_profiler.h"
#include "mos_os_cp_interface_specific.h"

#define MAX_BLT_BLOCK_COPY_WIDTH 0x4000u

class MhwInterfaces;

//!
//! \brief    BltState_Xe_Hpm constructor
//! \details  Initialize the BltState_Xe_Hpm members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltState_Xe_Hpm::BltState_Xe_Hpm(PMOS_INTERFACE    osInterface) :
    BltState(osInterface),
    initialized(false),
    allocated(false),
    tempSurface(nullptr),
    tempAuxSurface(nullptr),
    surfaceSize(0),
    auxSize(0),
    pMainSurface(nullptr),
    pAuxSurface(nullptr)
{

}

//!
//! \brief    BltState_Xe_Hpm constructor
//! \details  Initialize the BltState_Xe_Hpm members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltState_Xe_Hpm::BltState_Xe_Hpm(PMOS_INTERFACE    osInterface, MhwInterfaces *mhwInterfaces) :
    BltState(osInterface, mhwInterfaces),
    initialized(false),
    allocated(false),
    tempSurface(nullptr),
    tempAuxSurface(nullptr),
    surfaceSize(0),
    auxSize(0),
    pMainSurface(nullptr),
    pAuxSurface(nullptr)
{

}

BltState_Xe_Hpm::~BltState_Xe_Hpm()
{
    FreeResource();
    if (pMainSurface)
    {
        MOS_FreeMemAndSetNull(pMainSurface);
    }
    if (pAuxSurface)
    {
        MOS_FreeMemAndSetNull(pAuxSurface);
    }
}

//!
//! \brief    BltState_Xe_Hpm initialize
//! \details  Initialize the BltState_Xe_Hpm, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::Initialize()
{
    initialized  = true;
    m_blokCopyon = true;
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    CopyProtectResource
//! \param    src
//!           [in] Pointer to source resource
//! \param    dst
//!           [out] Pointer to destination resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::CopyProtectResource(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    BLT_CHK_NULL_RETURN(m_osInterface);

    MOS_MEMCOMP_STATE mmcState;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, dst, &mmcState));

    BLT_CHK_STATUS_RETURN(m_osInterface->pfnDoubleBufferCopyResource(
        m_osInterface,
        src,
        dst,
        ((mmcState == MOS_MEMCOMP_MC) || (mmcState == MOS_MEMCOMP_RC)) ? true : false));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    CopyProtectSurface
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination buffer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::CopyProtectSurface(
    PMOS_SURFACE src,
    PMOS_SURFACE dst)
{
    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    BLT_CHK_NULL_RETURN(m_miInterface);
    BLT_CHK_NULL_RETURN(m_cpInterface);

    MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption = {};
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_VIDEO,
        MOS_GPU_NODE_VIDEO,
        &createOption));

    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VIDEO));

    MHW_ADD_CP_COPY_PARAMS cpCopyParams;
    MOS_ZeroMemory(&cpCopyParams, sizeof(cpCopyParams));
    cpCopyParams.size    = src->dwWidth * src->dwHeight;
    cpCopyParams.presSrc = &src->OsResource;
    cpCopyParams.presDst = &dst->OsResource;
    cpCopyParams.offset  = 0;
    MOS_COMMAND_BUFFER cmdBuffer;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // add CP prolog
    BLT_CHK_STATUS_RETURN(m_miInterface->AddProtectedProlog(&cmdBuffer));

    BLT_CHK_STATUS_RETURN(m_cpInterface->AddCpCopy(m_osInterface, &cmdBuffer, &cpCopyParams));

    //MI flush cmd
    MHW_MI_FLUSH_DW_PARAMS FlushDwParams;
    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &FlushDwParams));

    //BatchBuffer end
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    // Flush the command buffer
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    MapTileType
//! \param    flags
//!           [in] GMM resource flags
//! \param    type
//!           [in] GMM tile type
//! \return   MOS_TILE_TYPE
//!           Return MOS tile type for given GMM resource and tile flags
//!
MOS_TILE_TYPE BltState_Xe_Hpm::MapTileType(GMM_RESOURCE_FLAG flags, GMM_TILE_TYPE type)
{
    switch (type)
    {
        case GMM_TILED_Y:
            if (flags.Info.TiledYf)
            {
                return MOS_TILE_YF;
            }
            else if (flags.Info.TiledYs)
            {
                return MOS_TILE_YS;
            }
            else
            {
                return MOS_TILE_Y;
            }
            break;
        case GMM_TILED_X:
            return MOS_TILE_X;
            break;
        case GMM_TILED_4:
        case GMM_TILED_64:
            return MOS_TILE_Y;
            break;
        case GMM_NOT_TILED:
        default:
            return MOS_TILE_LINEAR;
            break;
    }
}


//!
//! \brief    Get control surface
//! \details  BLT engine will copy aux data of source surface to destination
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination buffer is created for aux data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::GetCCS(
    PMOS_SURFACE src,
    PMOS_SURFACE dst)
{
    BLT_STATE_PARAM bltStateParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    BLT_CHK_NULL_RETURN(&src->OsResource);
    BLT_CHK_NULL_RETURN(&dst->OsResource);

    MOS_ZeroMemory(&bltStateParam, sizeof(BLT_STATE_PARAM));
    bltStateParam.bCopyCCS = true;
    bltStateParam.ccsFlag  = CCS_READ;
    bltStateParam.pSrcCCS  = src;
    bltStateParam.pDstCCS  = dst;

    BLT_CHK_STATUS_RETURN(SubmitCMD(&bltStateParam));

    // sync
    MOS_LOCK_PARAMS flag;
    flag.Value     = 0;
    flag.WriteOnly = 1;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnLockSyncRequest(m_osInterface, &dst->OsResource, &flag));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Put control surface
//! \details  BLT engine will copy aux data of source surface to destination
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination buffer is created for aux data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::PutCCS(
    PMOS_SURFACE src,
    PMOS_SURFACE dst)
{
    BLT_STATE_PARAM bltStateParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    BLT_CHK_NULL_RETURN(&src->OsResource);
    BLT_CHK_NULL_RETURN(&dst->OsResource);

    MOS_ZeroMemory(&bltStateParam, sizeof(BLT_STATE_PARAM));
    bltStateParam.bCopyCCS = true;
    bltStateParam.ccsFlag  = CCS_WRITE;
    bltStateParam.pSrcCCS  = src;
    bltStateParam.pDstCCS  = dst;

    BLT_CHK_STATUS_RETURN(SubmitCMD(&bltStateParam));

    // sync
    MOS_LOCK_PARAMS flag;
    flag.Value     = 0;
    flag.WriteOnly = 1;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnLockSyncRequest(m_osInterface, &dst->OsResource, &flag));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Lock surface
//! \details  Lock surface to get main surface and aux data
//! \param    pSrcSurface
//!           [in] Pointer to source surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS BltState_Xe_Hpm::LockSurface(
    PMOS_SURFACE pSurface)
{
    MOS_STATUS eStatus    = MOS_STATUS_SUCCESS;
    void*      pTemp      = nullptr;

    BLT_CHK_NULL(pSurface);

    // Initialize for the first time
    if (!initialized)
    {
        BLT_CHK_STATUS(Initialize());
    }

    // Allocate internel resource
    BLT_CHK_STATUS(AllocateResource(pSurface));

    // Get main surface and CCS
    // Currentlt main surface copy will cause page fault, which cause crash.
    // BLT_CHK_STATUS(CopyMainSurface(pSurface, tempSurface));
    BLT_CHK_STATUS(GetCCS(pSurface, tempAuxSurface));

    MOS_LOCK_PARAMS LockFlags;
    LockFlags.Value        = 0;
    LockFlags.ReadOnly     = 1;
    LockFlags.TiledAsTiled = 1;
    LockFlags.NoDecompress = 1;

    // Lock main surface data
    pTemp = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &pSurface->OsResource,
        &LockFlags);
    BLT_CHK_NULL(pTemp);

    MOS_SecureMemcpy(
        pMainSurface,
        surfaceSize,
        pTemp,
        surfaceSize);
    BLT_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, &pSurface->OsResource));

    // Lock CCS data
    pTemp = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &tempAuxSurface->OsResource,
        &LockFlags);
    BLT_CHK_NULL(pTemp);

    MOS_SecureMemcpy(
        pAuxSurface,
        auxSize,
        pTemp,
        auxSize);
    BLT_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, &tempAuxSurface->OsResource));

    return eStatus;

finish:
    BLT_ASSERTMESSAGE("BLT: Lock surface failed.");
    FreeResource();
    return eStatus;
}

//!
//! \brief    Unlock surface
//! \details  Free resource created by lockSurface, must be called once call LockSurface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS BltState_Xe_Hpm::UnLockSurface()
{
    FreeResource();
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Write compressed surface
//! \details  Write compressed surface data from system memory to GPU memory
//! \param    pSysMemory
//!           [in] Pointer to system memory
//! \param    dataSize
//!           [in] data size, including main surface data and aux data
//! \param    pSurface
//!           [in] Pointer to the destination surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS BltState_Xe_Hpm::WriteCompressedSurface(
    void*        pSysMemory,
    uint32_t     dataSize,
    PMOS_SURFACE pSurface)
{
    MOS_STATUS eStatus  = MOS_STATUS_SUCCESS;
    void*      pTemp    = nullptr;
    uint32_t   sizeAux  = 0;
    BLT_CHK_NULL(pSurface);

    // Initialize for the first time
    if (!initialized)
    {
        BLT_CHK_STATUS(Initialize());
    }

    // Allocate internel resource
    BLT_CHK_STATUS(AllocateResource(pSurface));

    sizeAux = dataSize / 257;

    MOS_LOCK_PARAMS LockFlags;
    LockFlags.Value     = 0;
    LockFlags.WriteOnly = 1;
    LockFlags.TiledAsTiled = 1;
    LockFlags.NoDecompress = 1;

    // Lock temp main surface
    pTemp = (uint32_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &pSurface->OsResource,
        &LockFlags);
    // copy surface data to temp surface
    MOS_SecureMemcpy(
        pTemp,
        sizeAux*256,
        pSysMemory,
        sizeAux*256);
    BLT_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, &pSurface->OsResource));

    // Lock temp aux surface
    pTemp = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &tempAuxSurface->OsResource,
        &LockFlags);
    // copy aux data to temp aux surface
    MOS_SecureMemcpy(
        pTemp,
        sizeAux,
        (uint8_t*)pSysMemory+sizeAux*256,
        sizeAux);
    BLT_CHK_STATUS(m_osInterface->pfnUnlockResource(m_osInterface, &tempAuxSurface->OsResource));
    BLT_CHK_STATUS_RETURN(PutCCS(tempAuxSurface, pSurface));

    FreeResource();
    return eStatus;

finish:
    BLT_ASSERTMESSAGE("BLT: Write compressed surface failed.");
    FreeResource();
    return eStatus;
}

//!
//! \brief    Allocate resource
//! \details  Allocate internel resource
//! \param    pSrcSurface
//!           [in] Pointer to source surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS BltState_Xe_Hpm::AllocateResource(
    PMOS_SURFACE pSurface)
{
    MOS_ALLOC_GFXRES_PARAMS AllocParams;

    tempSurface = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
    tempAuxSurface     = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
    BLT_CHK_NULL_RETURN(tempSurface);
    BLT_CHK_NULL_RETURN(tempAuxSurface);

    // Always allocate the temp surface as compressible surface to make sure the size is correct.
    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParams.TileType        = pSurface->TileType;
    AllocParams.Type            = MOS_GFXRES_2D;
    AllocParams.dwWidth         = pSurface->dwWidth;
    AllocParams.dwHeight        = pSurface->dwHeight;
    AllocParams.Format          = pSurface->Format;
    AllocParams.bIsCompressible = true;
    AllocParams.CompressionMode = pSurface->CompressionMode;
    AllocParams.pBufName        = "TempOutSurface";
    AllocParams.dwArraySize     = 1;

    BLT_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParams,
        &tempSurface->OsResource));

    tempSurface->dwPitch = pSurface->dwPitch;
    tempSurface->dwWidth = pSurface->dwWidth;
    tempSurface->dwHeight = pSurface->dwHeight;
    tempSurface->Format   = pSurface->Format;
    tempSurface->TileType = pSurface->TileType;

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParams.TileType        = MOS_TILE_LINEAR;
    AllocParams.Type            = MOS_GFXRES_BUFFER;
    AllocParams.dwWidth         = (uint32_t)tempSurface->OsResource.pGmmResInfo->GetSizeMainSurface() / 256;
    AllocParams.dwHeight        = 1;
    AllocParams.Format          = Format_Buffer;
    AllocParams.bIsCompressible = false;
    AllocParams.CompressionMode = MOS_MMC_DISABLED;
    AllocParams.pBufName        = "TempCCS";
    AllocParams.dwArraySize     = 1;

    BLT_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParams,
        &tempAuxSurface->OsResource));

    surfaceSize  = (uint32_t)tempSurface->OsResource.pGmmResInfo->GetSizeMainSurface();
    auxSize      = surfaceSize / 256;
    pMainSurface = MOS_AllocAndZeroMemory(surfaceSize);
    pAuxSurface  = MOS_AllocAndZeroMemory(auxSize);
    BLT_CHK_NULL_RETURN(pMainSurface);
    BLT_CHK_NULL_RETURN(pAuxSurface);

    allocated    = true;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Free resource
//! \details  Free internel resource, must be called once call AllocateResource
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS BltState_Xe_Hpm::FreeResource()
{
    if (allocated)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &tempSurface->OsResource);
        m_osInterface->pfnFreeResource(m_osInterface, &tempAuxSurface->OsResource);
        allocated = false;
    }
    if (tempSurface)
    {
        MOS_FreeMemAndSetNull(tempSurface);
    }
    if (tempAuxSurface)
    {
        MOS_FreeMemAndSetNull(tempAuxSurface);
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Setup control surface copy parameters
//! \details  Setup control surface copy parameters for BLT Engine
//! \param    mhwParams
//!           [in/out] Pointer to MHW_CTRL_SURF_COPY_BLT_PARAM
//! \param    inputSurface
//!           [in] Pointer to input surface
//! \param    outputSurface
//!           [in] Pointer to output surface
//! \param    flag
//!           [in] Flag for read/write CCS
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::SetupCtrlSurfCopyBltParam(
    PMHW_CTRL_SURF_COPY_BLT_PARAM pMhwBltParams,
    PMOS_SURFACE                  inputSurface,
    PMOS_SURFACE                  outputSurface,
    uint32_t                      flag)
{
    BLT_CHK_NULL_RETURN(pMhwBltParams);
    BLT_CHK_NULL_RETURN(inputSurface);
    BLT_CHK_NULL_RETURN(outputSurface);

    if (flag == CCS_READ)
    {
        pMhwBltParams->dwSrcMemoryType = 0;
        pMhwBltParams->dwDstMemoryType = 1;
        pMhwBltParams->dwSizeofControlSurface = (uint32_t)inputSurface->OsResource.pGmmResInfo->GetSizeMainSurface() / 65536;
    }
    else
    {
        pMhwBltParams->dwSrcMemoryType = 1;
        pMhwBltParams->dwDstMemoryType = 0;
        pMhwBltParams->dwSizeofControlSurface = (uint32_t)outputSurface->OsResource.pGmmResInfo->GetSizeMainSurface() / 65536;
    }

    pMhwBltParams->pSrcOsResource  = &inputSurface->OsResource;
    pMhwBltParams->pDstOsResource  = &outputSurface->OsResource;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Submit command
//! \details  Submit BLT command
//! \param    pBltStateParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::SubmitCMD(
    PBLT_STATE_PARAM pBltStateParam)
{
    MOS_COMMAND_BUFFER           cmdBuffer;
    MHW_FAST_COPY_BLT_PARAM      fastCopyBltParam;
    MHW_CTRL_SURF_COPY_BLT_PARAM ctrlSurfCopyBltParam;
    MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption = {};
    int                          planeNum = 1;
    PMHW_BLT_INTERFACE_XE_HP     pbltInterface = dynamic_cast<PMHW_BLT_INTERFACE_XE_HP>(m_bltInterface);

    BLT_CHK_NULL_RETURN(pbltInterface);
    BLT_CHK_NULL_RETURN(pBltStateParam);
    BLT_CHK_NULL_RETURN(m_osInterface);
    // need consolidate both input/output surface information to decide cp context.
    PMOS_RESOURCE surfaceArray[2];
    surfaceArray[0] = pBltStateParam->pSrcSurface;
    surfaceArray[1] = pBltStateParam->pDstSurface;
    if (m_osInterface->osCpInterface)
    {
        m_osInterface->osCpInterface->PrepareResources((void **)&surfaceArray, sizeof(surfaceArray) / sizeof(PMOS_RESOURCE), nullptr, 0);
    }
    // no gpucontext will be created if the gpu context has been created before.
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        MOS_GPU_CONTEXT_BLT,
        MOS_GPU_NODE_BLT,
        &createOption));
    // Set GPU context
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_BLT));
    // Register context with the Batch Buffer completion event
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        MOS_GPU_CONTEXT_BLT));

    // Initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    BLT_CHK_STATUS_RETURN(SetPrologParamsforCmdbuffer(&cmdBuffer));

    MOS_SURFACE       srcResDetails;
    MOS_SURFACE       dstResDetails;
    MOS_ZeroMemory(&srcResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&dstResDetails, sizeof(MOS_SURFACE));
    srcResDetails.Format = Format_Invalid;
    dstResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateParam->pSrcSurface, &srcResDetails));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateParam->pDstSurface, &dstResDetails));
    m_osInterface->pfnSetPerfTag(m_osInterface, BLT_COPY);

    if (srcResDetails.Format != dstResDetails.Format)
    {
        MCPY_ASSERTMESSAGE("BLT copy can't support CSC copy. input format = %d, output format = %d", srcResDetails.Format, dstResDetails.Format);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    planeNum = GetPlaneNum(dstResDetails.Format);

    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    BLT_CHK_NULL_RETURN(perfProfiler);
    BLT_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd((void*)this, m_osInterface, m_miInterface, &cmdBuffer));
    if (pBltStateParam->bCopyMainSurface)
    {
        BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
            &fastCopyBltParam,
            pBltStateParam->pSrcSurface,
            pBltStateParam->pDstSurface,
            0));

        MHW_MI_LOAD_REGISTER_IMM_PARAMS RegisterDwParams;
        MOS_ZeroMemory(&RegisterDwParams, sizeof(RegisterDwParams));
        RegisterDwParams.dwRegister = mhw_blt_state::BCS_SWCTRL_CMD::REGISTER_OFFSET;

        mhw_blt_state_xe_hp_base::BCS_SWCTRL_CMD swctrl;
        if (pBltStateParam->pSrcSurface->TileType != MOS_TILE_LINEAR)
        {
            swctrl.DW0.Tile4Source = 1;
        }
        if (pBltStateParam->pDstSurface->TileType != MOS_TILE_LINEAR)
        {//output tiled
            swctrl.DW0.Tile4Destination = 1;
        }

        RegisterDwParams.dwData = swctrl.DW0.Value;
        BLT_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&cmdBuffer, &RegisterDwParams));

        if (m_blokCopyon)
        {
            BLT_CHK_STATUS_RETURN(m_bltInterface->AddBlockCopyBlt(
                &cmdBuffer,
                &fastCopyBltParam,
                srcResDetails.YPlaneOffset.iSurfaceOffset,
                dstResDetails.YPlaneOffset.iSurfaceOffset));
        }
        else
        {
            BLT_CHK_STATUS_RETURN(m_bltInterface->AddFastCopyBlt(
                &cmdBuffer,
                &fastCopyBltParam,
                srcResDetails.YPlaneOffset.iSurfaceOffset,
                dstResDetails.YPlaneOffset.iSurfaceOffset));
        }
        if (planeNum >= 2)
        {
            BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
             &fastCopyBltParam,
             pBltStateParam->pSrcSurface,
             pBltStateParam->pDstSurface,
             1));
            if (m_blokCopyon)
            {
                BLT_CHK_STATUS_RETURN(m_bltInterface->AddBlockCopyBlt(
                    &cmdBuffer,
                    &fastCopyBltParam,
                    srcResDetails.UPlaneOffset.iSurfaceOffset,
                    dstResDetails.UPlaneOffset.iSurfaceOffset));
            }
            else
            {
                BLT_CHK_STATUS_RETURN(m_bltInterface->AddFastCopyBlt(
                    &cmdBuffer,
                    &fastCopyBltParam,
                    srcResDetails.UPlaneOffset.iSurfaceOffset,
                    dstResDetails.UPlaneOffset.iSurfaceOffset));
            }
            if (planeNum == 3)
            {
                BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
                    &fastCopyBltParam,
                    pBltStateParam->pSrcSurface,
                    pBltStateParam->pDstSurface,
                    2));
                if (m_blokCopyon)
                {
                    BLT_CHK_STATUS_RETURN(m_bltInterface->AddBlockCopyBlt(
                        &cmdBuffer,
                        &fastCopyBltParam,
                        srcResDetails.VPlaneOffset.iSurfaceOffset,
                        dstResDetails.VPlaneOffset.iSurfaceOffset));
                }
                else
                {
                    BLT_CHK_STATUS_RETURN(m_bltInterface->AddFastCopyBlt(
                        &cmdBuffer,
                        &fastCopyBltParam,
                        srcResDetails.VPlaneOffset.iSurfaceOffset,
                        dstResDetails.VPlaneOffset.iSurfaceOffset));
                }
            }
            else if(planeNum > 3)
            {
                MCPY_ASSERTMESSAGE("illegal usage");
                return MOS_STATUS_INVALID_PARAMETER;
            }
         }

    }

    if (pBltStateParam->bCopyCCS)
    {
        BLT_CHK_STATUS_RETURN(SetupCtrlSurfCopyBltParam(
            &ctrlSurfCopyBltParam,
            pBltStateParam->pSrcCCS,
            pBltStateParam->pDstCCS,
            pBltStateParam->ccsFlag));
        BLT_CHK_STATUS_RETURN(pbltInterface->AddCtrlSurfCopyBlt(
            &cmdBuffer,
            &ctrlSurfCopyBltParam));
    }
    BLT_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd((void*)this, m_osInterface, m_miInterface, &cmdBuffer));

    // Add flush DW
    MHW_MI_FLUSH_DW_PARAMS FlushDwParams;
    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &FlushDwParams));

    // Add Batch Buffer end
    BLT_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // Flush the command buffer
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Block copy buffer
//! \details  BLT engine will copy source buffer to destination buffer
//! \param    pBltStateParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltState_Xe_Hpm::BlockCopyBuffer(PBLT_STATE_PARAM pBltStateParam)
{
    PMOS_RESOURCE src = nullptr;
    PMOS_RESOURCE dst = nullptr;
    BLT_CHK_NULL_RETURN(pBltStateParam);
    BLT_CHK_NULL_RETURN(pBltStateParam->pSrcSurface);
    BLT_CHK_NULL_RETURN(pBltStateParam->pDstSurface);
    BLT_CHK_NULL_RETURN(pBltStateParam->pSrcSurface->pGmmResInfo);
    BLT_CHK_NULL_RETURN(pBltStateParam->pDstSurface->pGmmResInfo);

    src = pBltStateParam->pSrcSurface;
    dst = pBltStateParam->pDstSurface;

    if ((src->pGmmResInfo->GetSizeMainSurface() > MAX_BLT_BLOCK_COPY_WIDTH * MAX_BLT_BLOCK_COPY_WIDTH) &&
        (dst->pGmmResInfo->GetSizeMainSurface() > MAX_BLT_BLOCK_COPY_WIDTH * MAX_BLT_BLOCK_COPY_WIDTH))
    {
        BLT_ASSERTMESSAGE("Buffer size too large");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((src->pGmmResInfo->GetSizeMainSurface() % 4096 != 0) &&
        (src->pGmmResInfo->GetSizeMainSurface() % 4096 != 16))
    {
        BLT_ASSERTMESSAGE("Src buffer is not aligned to 4K");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((dst->pGmmResInfo->GetSizeMainSurface() % 4096 != 0) &&
        (dst->pGmmResInfo->GetSizeMainSurface() % 4096 != 16))
    {
        BLT_ASSERTMESSAGE("Dst buffer is not aligned to 4K");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    GMM_RESOURCE_FORMAT backupSrcFormat = src->pGmmResInfo->GetResourceFormat();
    GMM_GFX_SIZE_T      backupSrcWidth  = src->pGmmResInfo->GetBaseWidth();
    uint32_t            backupSrcHeight = src->pGmmResInfo->GetBaseHeight();
    GMM_RESOURCE_FORMAT backupDstFormat = dst->pGmmResInfo->GetResourceFormat();
    GMM_GFX_SIZE_T      backupDstWidth  = dst->pGmmResInfo->GetBaseWidth();
    uint32_t            backupDstHeight = dst->pGmmResInfo->GetBaseHeight();

    uint32_t pitch = 4096;
    uint32_t size  = static_cast<uint32_t>((std::min)(src->pGmmResInfo->GetSizeMainSurface(), dst->pGmmResInfo->GetSizeMainSurface()));
    uint32_t height = static_cast<uint32_t>(size / pitch);
    while (height > MAX_BLT_BLOCK_COPY_WIDTH)
    {
        pitch += 4096;
        height = static_cast<uint32_t>(size / pitch);
    }

    src->pGmmResInfo->OverrideSurfaceFormat(GMM_FORMAT_R8_UINT);
    src->pGmmResInfo->OverrideSurfaceType(RESOURCE_2D);
    src->pGmmResInfo->OverrideBaseWidth(pitch);
    src->pGmmResInfo->OverrideBaseHeight(height);
    src->pGmmResInfo->OverridePitch(pitch);

    dst->pGmmResInfo->OverrideSurfaceFormat(GMM_FORMAT_R8_UINT);
    dst->pGmmResInfo->OverrideSurfaceType(RESOURCE_2D);
    dst->pGmmResInfo->OverrideBaseWidth(pitch);
    dst->pGmmResInfo->OverrideBaseHeight(height);
    dst->pGmmResInfo->OverridePitch(pitch);

    MOS_STATUS status = SubmitCMD(pBltStateParam);

    src->pGmmResInfo->OverrideSurfaceFormat(backupSrcFormat);
    src->pGmmResInfo->OverrideSurfaceType(RESOURCE_BUFFER);
    src->pGmmResInfo->OverrideBaseWidth(backupSrcWidth);
    src->pGmmResInfo->OverrideBaseHeight(backupSrcHeight);
    src->pGmmResInfo->OverridePitch(backupSrcWidth);

    dst->pGmmResInfo->OverrideSurfaceFormat(backupDstFormat);
    dst->pGmmResInfo->OverrideSurfaceType(RESOURCE_BUFFER);
    dst->pGmmResInfo->OverrideBaseWidth(backupDstWidth);
    dst->pGmmResInfo->OverrideBaseHeight(backupDstHeight);
    dst->pGmmResInfo->OverridePitch(backupDstWidth);

    BLT_CHK_STATUS_RETURN(status);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS BltState_Xe_Hpm::CopyMainSurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    BLT_STATE_PARAM bltStateParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    BLT_CHK_NULL_RETURN(src->pGmmResInfo);
    BLT_CHK_NULL_RETURN(dst->pGmmResInfo);

    MOS_ZeroMemory(&bltStateParam, sizeof(BLT_STATE_PARAM));
    bltStateParam.bCopyMainSurface = true;
    bltStateParam.pSrcSurface      = src;
    bltStateParam.pDstSurface      = dst;

    // A workaround for oversized buffers.
    // BLOCK_COPY_BLT can only receive (width-1) of up to 14 bits.
    // The width of the internal buffer of a staging texture may exceed that limit.
    if (m_blokCopyon &&
        (src->pGmmResInfo->GetResourceType() == RESOURCE_BUFFER) &&
        (dst->pGmmResInfo->GetResourceType() == RESOURCE_BUFFER) &&
        ((src->pGmmResInfo->GetBaseWidth() > MAX_BLT_BLOCK_COPY_WIDTH) || (dst->pGmmResInfo->GetBaseWidth() > MAX_BLT_BLOCK_COPY_WIDTH)))
    {
        BLT_CHK_STATUS_RETURN(BlockCopyBuffer(&bltStateParam));
    }
    else
    {
        BLT_CHK_STATUS_RETURN(SubmitCMD(&bltStateParam));
    }

    return MOS_STATUS_SUCCESS;
}