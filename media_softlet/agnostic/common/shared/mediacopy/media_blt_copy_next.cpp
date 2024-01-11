/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     media_blt_copy_next.cpp
//! \brief    Common interface used in Blitter Engine
//! \details  Common interface used in Blitter Engine which are platform independent
//!

#define NOMINMAX
#include <algorithm>
#include "media_perf_profiler.h"
#include "media_blt_copy_next.h"
#define BIT( n )                            ( 1 << (n) )

#ifdef min
#undef min
#endif

//!
//! \brief    BltStateNext constructor
//! \details  Initialize the BltStateNext members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateNext::BltStateNext(PMOS_INTERFACE    osInterface) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_cpInterface(nullptr)
{
    MhwInterfacesNext::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_blt = 1;
    m_mhwInterfaces = MhwInterfacesNext::CreateFactory(params, osInterface);
    if (m_mhwInterfaces != nullptr)
    {
        m_miItf  = m_mhwInterfaces->m_miItf;
        m_bltItf = m_mhwInterfaces->m_bltItf;
    }
}

//!
//! \brief    BltStateNext constructor
//! \details  Initialize the BltStateNext members.
//! \param    osInterface
//!           [in] Pointer to MOS_INTERFACE.
//!
BltStateNext::BltStateNext(PMOS_INTERFACE    osInterface, MhwInterfacesNext* mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(nullptr),
    m_cpInterface(nullptr)
{
    m_miItf        = mhwInterfaces->m_miItf;
    m_bltItf       = mhwInterfaces->m_bltItf;
    m_cpInterface  = mhwInterfaces->m_cpInterface;
}


BltStateNext::~BltStateNext()
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
    //component interface will be relesed in media copy.
    if (m_mhwInterfaces != nullptr)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }
}

//!
//! \brief    BltStateNext initialize
//! \details  Initialize the BltStateNext, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::Initialize()
{
    return MOS_STATUS_SUCCESS;
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
MOS_STATUS BltStateNext::GetCCS(
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
MOS_STATUS BltStateNext::PutCCS(
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
MOS_STATUS BltStateNext::LockSurface(
    PMOS_SURFACE pSurface)
{
    MOS_STATUS eStatus    = MOS_STATUS_SUCCESS;
    void*      pTemp      = nullptr;
    do
    {
        if (pSurface==nullptr)
        {
            BLT_ASSERTMESSAGE("BLT: pSurface check nullptr fail in LockSurface.")
            break;
        }

        // Initialize for the first time
        if (!initialized)
        {
            if (Initialize() != MOS_STATUS_SUCCESS)
            {
                break;
            }
        }

        // Allocate internel resource
        if (AllocateResource (pSurface) != MOS_STATUS_SUCCESS)
        {
            break;
        }

        // Get main surface and CCS
        // Currentlt main surface copy will cause page fault, which cause crash.
        // BLT_CHK_STATUS(CopyMainSurface(pSurface, tempSurface));
        if (GetCCS(pSurface, tempAuxSurface) != MOS_STATUS_SUCCESS)
        {
            break;
        }

        MOS_LOCK_PARAMS LockFlags;
        LockFlags.Value        = 0;
        LockFlags.ReadOnly     = 1;
        LockFlags.TiledAsTiled = 1;
        LockFlags.NoDecompress = 1;

        // Lock main surface data
        pTemp = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &pSurface->OsResource,
            &LockFlags);
        if (pTemp == nullptr)
        {
            break;
        }

        MOS_SecureMemcpy(
            pMainSurface,
            surfaceSize,
            pTemp,
            surfaceSize);
        if (m_osInterface->pfnUnlockResource(m_osInterface, &pSurface->OsResource) != MOS_STATUS_SUCCESS)
        {
            break;
        }

        // Lock CCS data
        pTemp = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &tempAuxSurface->OsResource,
            &LockFlags);
        if (pTemp == nullptr)
        {
            break;
        }

        MOS_SecureMemcpy(
            pAuxSurface,
            auxSize,
            pTemp,
            auxSize);
        if (m_osInterface->pfnUnlockResource(m_osInterface, &tempAuxSurface->OsResource))
        {
            break;
        }

        return eStatus;
    } while (false);

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
MOS_STATUS BltStateNext::UnLockSurface()
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
MOS_STATUS BltStateNext::WriteCompressedSurface(
    void*        pSysMemory,
    uint32_t     dataSize,
    PMOS_SURFACE pSurface)
{
    MOS_STATUS eStatus  = MOS_STATUS_SUCCESS;
    void*      pTemp    = nullptr;
    uint32_t   sizeAux  = 0;
    do
    {
        if (pSurface == nullptr)
        {
            BLT_ASSERTMESSAGE("BLT: pSurface check nullptr fail in WriteCompressedSurface.")
            break;
        }

        // Initialize for the first time
        if (!initialized)
        {
            if (Initialize() != MOS_STATUS_SUCCESS)
            {
                break;
            }
        }

        // Allocate internel resource
        if (AllocateResource(pSurface) != MOS_STATUS_SUCCESS)
        {
            break;
        }

        sizeAux = dataSize / 257;

        MOS_LOCK_PARAMS LockFlags;
        LockFlags.Value        = 0;
        LockFlags.WriteOnly    = 1;
        LockFlags.TiledAsTiled = 1;
        LockFlags.NoDecompress = 1;

        // Lock temp main surface
        pTemp = (uint32_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &pSurface->OsResource,
            &LockFlags);
        // copy surface data to temp surface
        MOS_SecureMemcpy(
            pTemp,
            sizeAux * 256,
            pSysMemory,
            sizeAux * 256);
        if (m_osInterface->pfnUnlockResource(m_osInterface, &pSurface->OsResource) != MOS_STATUS_SUCCESS)
        {
            break;
        }

        // Lock temp aux surface
        pTemp = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &tempAuxSurface->OsResource,
            &LockFlags);
        // copy aux data to temp aux surface
        MOS_SecureMemcpy(
            pTemp,
            sizeAux,
            (uint8_t *)pSysMemory + sizeAux * 256,
            sizeAux);
        if (m_osInterface->pfnUnlockResource(m_osInterface, &tempAuxSurface->OsResource) != MOS_STATUS_SUCCESS)
        {
            break;
        }
        BLT_CHK_STATUS_RETURN(PutCCS(tempAuxSurface, pSurface));

        FreeResource();
        return eStatus;
    } while (false);

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
MOS_STATUS BltStateNext::AllocateResource(
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
MOS_STATUS BltStateNext::FreeResource()
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
MOS_STATUS BltStateNext::SetupCtrlSurfCopyBltParam(
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
//! \brief    Block copy buffer
//! \details  BLT engine will copy source buffer to destination buffer
//! \param    pBltStateParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::BlockCopyBuffer(PBLT_STATE_PARAM pBltStateParam)
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

//!
//! \brief    Copy main surface
//! \details  BLT engine will copy source surface to destination surface
//! \param    src
//!           [in] Pointer to source surface
//! \param    dst
//!           [in] Pointer to destination surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::CopyMainSurface(
    PMOS_SURFACE src,
    PMOS_SURFACE dst)
{
    BLT_CHK_STATUS_RETURN(CopyMainSurface(&src->OsResource, &dst->OsResource));
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Copy main surface
//! \details  BLT engine will copy source surface to destination surface
//! \param    src
//!           [in] Pointer to source resource
//! \param    dst
//!           [in] Pointer to destination resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::CopyMainSurface(
    PMOS_RESOURCE src,
    PMOS_RESOURCE dst)
{
    BLT_STATE_PARAM BltStateNextParam;

    BLT_CHK_NULL_RETURN(src);
    BLT_CHK_NULL_RETURN(dst);
    BLT_CHK_NULL_RETURN(src->pGmmResInfo);
    BLT_CHK_NULL_RETURN(dst->pGmmResInfo);
    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_START, nullptr, 0, nullptr, 0);

    MOS_ZeroMemory(&BltStateNextParam, sizeof(BLT_STATE_PARAM));
    BltStateNextParam.bCopyMainSurface = true;
    BltStateNextParam.pSrcSurface      = src;
    BltStateNextParam.pDstSurface      = dst;

    // A workaround for oversized buffers.
    // BLOCK_COPY_BLT can only receive (width-1) of up to 14 bits.
    // The width of the internal buffer of a staging texture may exceed that limit.
    if (m_blokCopyon &&
        (src->pGmmResInfo->GetResourceType() == RESOURCE_BUFFER) &&
        (dst->pGmmResInfo->GetResourceType() == RESOURCE_BUFFER) &&
        ((src->pGmmResInfo->GetBaseWidth() > MAX_BLT_BLOCK_COPY_WIDTH) || (dst->pGmmResInfo->GetBaseWidth() > MAX_BLT_BLOCK_COPY_WIDTH)))
    {
        BLT_CHK_STATUS_RETURN(BlockCopyBuffer(&BltStateNextParam));
    }
    else
    {
        BLT_CHK_STATUS_RETURN(SubmitCMD(&BltStateNextParam));
    }

    MOS_TraceEventExt(EVENT_MEDIA_COPY, EVENT_TYPE_END, nullptr, 0, nullptr, 0);
    return MOS_STATUS_SUCCESS;

}

//!
//! \brief    Setup fast copy parameters
//! \details  Setup fast copy parameters for BLT Engine
//! \param    mhwParams
//!           [in/out] Pointer to MHW_FAST_COPY_BLT_PARAM
//! \param    inputSurface
//!           [in] Pointer to input surface
//! \param    outputSurface
//!           [in] Pointer to output surface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::SetupBltCopyParam(
    PMHW_FAST_COPY_BLT_PARAM pMhwBltParams,
    PMOS_RESOURCE            inputSurface,
    PMOS_RESOURCE            outputSurface,
    int                      planeIndex)
{
    BLT_CHK_NULL_RETURN(pMhwBltParams);
    BLT_CHK_NULL_RETURN(inputSurface);
    BLT_CHK_NULL_RETURN(outputSurface);
    BLT_CHK_NULL_RETURN(outputSurface->pGmmResInfo);

    uint32_t          BytesPerTexel = 1;
    MOS_SURFACE       ResDetails;
    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(pMhwBltParams, sizeof(MHW_FAST_COPY_BLT_PARAM));
    ResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, inputSurface, &ResDetails));

    uint32_t inputHeight = ResDetails.dwHeight;
    uint32_t inputWidth  = ResDetails.dwWidth;
    uint32_t inputPitch  = ResDetails.dwPitch;

    if (inputSurface->TileType != MOS_TILE_LINEAR)
    { 
        pMhwBltParams->dwSrcPitch = ResDetails.dwPitch / 4;
    }
    else
    {
        pMhwBltParams->dwSrcPitch = ResDetails.dwPitch;
    }
    
    pMhwBltParams->dwSrcTop    = ResDetails.RenderOffset.YUV.Y.YOffset;
    pMhwBltParams->dwSrcLeft   = ResDetails.RenderOffset.YUV.Y.XOffset;

    MOS_ZeroMemory(&ResDetails, sizeof(MOS_SURFACE));
    ResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, outputSurface, &ResDetails));

    uint32_t outputHeight = ResDetails.dwHeight;
    uint32_t outputWidth  = ResDetails.dwWidth;
    uint32_t outputPitch  = ResDetails.dwPitch;

    if (outputSurface->TileType != MOS_TILE_LINEAR)
    {
        pMhwBltParams->dwDstPitch = ResDetails.dwPitch/4;
    }
    else
    {
        pMhwBltParams->dwDstPitch  = ResDetails.dwPitch;
    }
    pMhwBltParams->dwDstTop    = ResDetails.RenderOffset.YUV.Y.YOffset;
    pMhwBltParams->dwDstLeft   = ResDetails.RenderOffset.YUV.Y.XOffset;

    int planeNum = GetPlaneNum(ResDetails.Format);
    pMhwBltParams->dwDstRight = std::min(inputWidth, outputWidth);

    if (outputSurface->pGmmResInfo->GetResourceType() != RESOURCE_BUFFER)
    {
        BytesPerTexel = outputSurface->pGmmResInfo->GetBitsPerPixel() / 8;  // using Bytes.

        if (ResDetails.Format == Format_P010 || ResDetails.Format == Format_P016)
        {
            BytesPerTexel = 2;
        }
    }

    if (true == m_blokCopyon)
    {
        pMhwBltParams->dwColorDepth = GetBlkCopyColorDepth(outputSurface->pGmmResInfo->GetResourceFormat(), BytesPerTexel);
    }
    else
    {
        pMhwBltParams->dwColorDepth = GetFastCopyColorDepth(outputSurface->pGmmResInfo->GetResourceFormat(), BytesPerTexel);
    }

    if( 1 == planeNum )
    {// handle as whole memory
       pMhwBltParams->dwDstBottom = std::min(inputHeight, outputHeight);
       if (false == m_blokCopyon)
       {// fastcopy
           pMhwBltParams->dwDstRight   = std::min(inputPitch, outputPitch) / 4;  // Regard as 32 bit per pixel format, i.e. 4 byte per pixel.
           pMhwBltParams->dwColorDepth = 3;  //0:8bit 1:16bit 3:32bit 4:64bit
       }
       else
       {
           // Block copy
           pMhwBltParams->dwDstRight = std::min(inputPitch, outputPitch) / 4;  // Regard as 32 bit per pixel format, i.e. 4 byte per pixel.
           pMhwBltParams->dwColorDepth = 2;  //0:8bit 1:16bit 2:32bit
       }
    }
    else
    {
        int bytePerTexelScaling    = GetBytesPerTexelScaling(ResDetails.Format);
        pMhwBltParams->dwDstBottom = std::min(inputHeight, outputHeight);

        if (1 == planeIndex || 2 == planeIndex)
        {
           pMhwBltParams->dwDstBottom = pMhwBltParams->dwDstBottom / bytePerTexelScaling;
           if (ResDetails.Format == Format_I420 || ResDetails.Format == Format_YV12)
           {
               pMhwBltParams->dwDstPitch  = pMhwBltParams->dwDstPitch / 2;
               pMhwBltParams->dwSrcPitch  = pMhwBltParams->dwSrcPitch / 2;
               pMhwBltParams->dwDstRight  = pMhwBltParams->dwDstRight / 2;
               pMhwBltParams->dwDstBottom = pMhwBltParams->dwDstBottom / 2;
           }
        }
    }
    pMhwBltParams->pSrcOsResource = inputSurface;
    pMhwBltParams->pDstOsResource = outputSurface;
    MCPY_NORMALMESSAGE("BLT params: m_blokCopyon = %d, format %d, planeNum %d, planeIndex %d, dwColorDepth %d, dwSrcTop %d, dwSrcLeft %d, dwSrcPitch %d,"
                       "dwDstTop %d, dwDstLeft %d, dwDstRight %d, dwDstBottom %d, dwDstPitch %d", m_blokCopyon,
                       ResDetails.Format, planeNum, planeIndex, pMhwBltParams->dwColorDepth, pMhwBltParams->dwSrcTop, pMhwBltParams->dwSrcLeft,
                       pMhwBltParams->dwSrcPitch, pMhwBltParams->dwDstTop, pMhwBltParams->dwDstLeft, pMhwBltParams->dwDstRight, 
                       pMhwBltParams->dwDstBottom, pMhwBltParams->dwDstPitch);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Submit command2
//! \details  Submit BLT command2
//! \param    pBltStateParam
//!           [in] Pointer to BLT_STATE_PARAM
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS BltStateNext::SubmitCMD(
    PBLT_STATE_PARAM pBltStateParam)
{
    MOS_STATUS                   eStatus;
    MOS_COMMAND_BUFFER           cmdBuffer;
    MHW_FAST_COPY_BLT_PARAM      fastCopyBltParam;
    MHW_CTRL_SURF_COPY_BLT_PARAM ctrlSurfCopyBltParam;
    MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption = {};
    int                          planeNum = 1;

    BLT_CHK_NULL_RETURN(m_miItf);
    BLT_CHK_NULL_RETURN(m_bltItf);

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

    MOS_SURFACE       srcResDetails;
    MOS_SURFACE       dstResDetails;
    MOS_ZeroMemory(&srcResDetails, sizeof(MOS_SURFACE));
    MOS_ZeroMemory(&dstResDetails, sizeof(MOS_SURFACE));
    srcResDetails.Format = Format_Invalid;
    dstResDetails.Format = Format_Invalid;
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateParam->pSrcSurface, &srcResDetails));
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(m_osInterface, pBltStateParam->pDstSurface, &dstResDetails));

    if (srcResDetails.Format != dstResDetails.Format)
    {
        MCPY_ASSERTMESSAGE("BLT copy can't support CSC copy. input format = %d, output format = %d", srcResDetails.Format, dstResDetails.Format);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    planeNum = GetPlaneNum(dstResDetails.Format);
    m_osInterface->pfnSetPerfTag(m_osInterface, BLT_COPY);
    MediaPerfProfiler* perfProfiler = MediaPerfProfiler::Instance();
    BLT_CHK_NULL_RETURN(perfProfiler);
    BLT_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectStartCmd((void*)this, m_osInterface, m_miItf, &cmdBuffer));

    if (pBltStateParam->bCopyMainSurface)
    {
        BLT_CHK_STATUS_RETURN(SetupBltCopyParam(
            &fastCopyBltParam,
            pBltStateParam->pSrcSurface,
            pBltStateParam->pDstSurface,
            0));

        auto& Register = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        Register = {};
        Register.dwRegister = mhw_blt_state::BCS_SWCTRL_XE::REGISTER_OFFSET;
        mhw_blt_state::BCS_SWCTRL_XE swctrl;
        if (pBltStateParam->pSrcSurface->TileType != MOS_TILE_LINEAR)
        {
            swctrl.DW0.Tile4Source = 1;
        }
        if (pBltStateParam->pDstSurface->TileType != MOS_TILE_LINEAR)
        {//output tiled
            swctrl.DW0.Tile4Destination = 1;
        }
        Register.dwData = swctrl.DW0.Value;
        BLT_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(&cmdBuffer));

        if (m_blokCopyon)
        {
            BLT_CHK_STATUS_RETURN(m_bltItf->AddBlockCopyBlt(
                &cmdBuffer,
                &fastCopyBltParam,
                srcResDetails.YPlaneOffset.iSurfaceOffset,
                dstResDetails.YPlaneOffset.iSurfaceOffset));
        }
        else
        {
            BLT_CHK_STATUS_RETURN(m_bltItf->AddFastCopyBlt(
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
                BLT_CHK_STATUS_RETURN(m_bltItf->AddBlockCopyBlt(
                    &cmdBuffer,
                    &fastCopyBltParam,
                    srcResDetails.UPlaneOffset.iSurfaceOffset,
                    dstResDetails.UPlaneOffset.iSurfaceOffset));
            }
            else
            {
                BLT_CHK_STATUS_RETURN(m_bltItf->AddFastCopyBlt(
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
                    BLT_CHK_STATUS_RETURN(m_bltItf->AddBlockCopyBlt(
                        &cmdBuffer,
                        &fastCopyBltParam,
                        srcResDetails.VPlaneOffset.iSurfaceOffset,
                        dstResDetails.VPlaneOffset.iSurfaceOffset));
                }
                else
                {
                    BLT_CHK_STATUS_RETURN(m_bltItf->AddFastCopyBlt(
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
    BLT_CHK_STATUS_RETURN(perfProfiler->AddPerfCollectEndCmd((void*)this, m_osInterface, m_miItf, &cmdBuffer));

    // Add flush DW
    auto& flushDwParams = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams = {};
    auto skuTable       = m_osInterface->pfnGetSkuTable(m_osInterface);
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
         flushDwParams.bEnablePPCFlush = true;
    }
    BLT_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(&cmdBuffer));
    // Add Batch Buffer end
    BLT_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // Return unused command buffer space to OS
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    // Flush the command buffer
    BLT_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, false));

    return MOS_STATUS_SUCCESS;
}

uint32_t BltStateNext::GetBlkCopyColorDepth(
    GMM_RESOURCE_FORMAT dstFormat,
    uint32_t            BytesPerTexel)
{
    uint32_t bitsPerTexel = BytesPerTexel * BLT_BITS_PER_BYTE;

    switch (bitsPerTexel)
    {
     case 8:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_8BITCOLOR;
         break;
     case 16:
         switch (dstFormat)
         {
           case GMM_FORMAT_B5G5R5A1_UNORM:
               return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
               break;
           default:
               return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_16BITCOLOR;;
               break;
         }
         break;
     case 64:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_64BITCOLOR;
         break;
     case 96:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
         break;
     case 128:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_128BITCOLOR;
         break;
     default:
         return mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
         break;
    }
 }

uint32_t BltStateNext::GetFastCopyColorDepth(
     GMM_RESOURCE_FORMAT dstFormat,
     uint32_t            BytesPerTexel)
 {
     uint32_t bitsPerTexel = BytesPerTexel * BLT_BITS_PER_BYTE;

     switch (bitsPerTexel)
     {
     case 8:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_8BITCOLOR;
         break;
     case 16:
         switch (dstFormat)
         {
         case GMM_FORMAT_B5G5R5A1_UNORM:
             return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
             break;
         default:
             return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_16BITCOLOR_565;
             break;
         }
         break;
     case 64:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_64BITCOLOR_FOR64KBTILING;
         break;
     case 128:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_128BITCOLOR_FOR64KBTILING;
         break;
     default:
         return mhw_blt_state::XY_FAST_COPY_BLT_CMD::COLOR_DEPTH_32BITCOLOR;
         break;
     }
 }

 int BltStateNext::GetBytesPerTexelScaling(MOS_FORMAT format)
{
  int  dstBytesPerTexel = 1;

   switch (format)
   {
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            dstBytesPerTexel = 2;
           break;

       default:
           dstBytesPerTexel = 1;
    }
   return dstBytesPerTexel;
 }

int BltStateNext::GetPlaneNum(MOS_FORMAT format)
{

  int  planeNum = 1;

   switch (format)
   {
       case Format_NV12:
       case Format_P010:
       case Format_P016:
            planeNum = 2;
           break;
       case Format_YV12:
       case Format_I420:
       case Format_444P:
       case Format_RGBP:
       case Format_BGRP:
       case Format_IMC3:
       case Format_411P:
       case Format_422V:
       case Format_422H:
            planeNum = 3;
            break;
       default:
            planeNum = 1;
           break;
    }
   return planeNum;
 }