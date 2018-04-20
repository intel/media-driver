/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mos_graphicsresource_specific.cpp
//! \brief   Container class for the linux/Android specfic graphic resource
//!

#include "mos_defs.h"
#include "mos_util_debug.h"
#ifdef ANDROID
#include <ufo/gralloc.h>
#endif

#include "mos_graphicsresource_specific.h"
#include "mos_context_specific.h"

GraphicsResourceSpecific::GraphicsResourceSpecific()
{
    MOS_OS_FUNCTION_ENTER;
}

GraphicsResourceSpecific::~GraphicsResourceSpecific()
{
    MOS_OS_FUNCTION_ENTER;
}

MOS_STATUS GraphicsResourceSpecific::SetSyncTag(OsContext* osContextPtr, SyncParams& params, uint32_t streamIndex)
{
    MOS_UNUSED(osContextPtr);
    MOS_UNUSED(params);
    MOS_UNUSED(streamIndex);
    return MOS_STATUS_SUCCESS;
}

bool GraphicsResourceSpecific::ResourceIsNull()
{
    return ((m_bo == nullptr)
#if (_DEBUG || _RELEASE_INTERNAL)
         && ((m_pData == nullptr) )
#endif // (_DEBUG || _RELEASE_INTERNAL)
    );

}

GMM_RESOURCE_FORMAT GraphicsResourceSpecific::ConvertMosFmtToGmmFmt(MOS_FORMAT format)
{
    switch (format)
    {
        case Format_Buffer      : return GMM_FORMAT_GENERIC_8BIT;
        case Format_Buffer_2D   : return GMM_FORMAT_GENERIC_8BIT;
        case Format_L8          : return GMM_FORMAT_GENERIC_8BIT;
        case Format_L16         : return GMM_FORMAT_L16_UNORM_TYPE;
        case Format_STMM        : return GMM_FORMAT_R8_UNORM_TYPE;
        case Format_AI44        : return GMM_FORMAT_GENERIC_8BIT;
        case Format_IA44        : return GMM_FORMAT_GENERIC_8BIT;
        case Format_R5G6B5      : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Format_X8R8G8B8    : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Format_A8R8G8B8    : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Format_X8B8G8R8    : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case Format_A8B8G8R8    : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Format_R32F        : return GMM_FORMAT_R32_FLOAT_TYPE;
        case Format_V8U8        : return GMM_FORMAT_GENERIC_16BIT;              // matching size as format
        case Format_YUY2        : return GMM_FORMAT_YUY2;
        case Format_UYVY        : return GMM_FORMAT_UYVY;
        case Format_P8          : return GMM_FORMAT_RENDER_8BIT_TYPE;           // matching size as format
        case Format_A8          : return GMM_FORMAT_A8_UNORM_TYPE;
        case Format_AYUV        : return GMM_FORMAT_R8G8B8A8_UINT_TYPE;
        case Format_NV12        : return GMM_FORMAT_NV12_TYPE;
        case Format_NV21        : return GMM_FORMAT_NV21_TYPE;
        case Format_YV12        : return GMM_FORMAT_YV12_TYPE;
        case Format_R32U        : return GMM_FORMAT_R32_UINT_TYPE;
        case Format_R32S        : return GMM_FORMAT_R32_SINT_TYPE;
        case Format_RAW         : return GMM_FORMAT_GENERIC_8BIT;
        case Format_444P        : return GMM_FORMAT_MFX_JPEG_YUV444_TYPE;
        case Format_422H        : return GMM_FORMAT_MFX_JPEG_YUV422H_TYPE;
        case Format_422V        : return GMM_FORMAT_MFX_JPEG_YUV422V_TYPE;
        case Format_IMC3        : return GMM_FORMAT_IMC3_TYPE;
        case Format_411P        : return GMM_FORMAT_MFX_JPEG_YUV411_TYPE;
        case Format_R8U         : return GMM_FORMAT_R8_UINT_TYPE;
        case Format_R16U        : return GMM_FORMAT_R16_UINT_TYPE;
        case Format_P010        : return GMM_FORMAT_P010_TYPE;
        default                 : return GMM_FORMAT_INVALID;
    }
}

MOS_STATUS GraphicsResourceSpecific::Allocate(OsContext* osContextPtr, CreateParams& params)
{
    MOS_OS_FUNCTION_ENTER;

    if (osContextPtr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
        return MOS_STATUS_INVALID_HANDLE;
    }

    if (osContextPtr->GetOsContextValid() == false)
    {
        MOS_OS_ASSERTMESSAGE("The OS context got is not valid.");
        return MOS_STATUS_INVALID_HANDLE;
    }

    OsContextSpecific *pOsContextSpecific  = static_cast<OsContextSpecific *>(osContextPtr);

    MOS_STATUS         status          = MOS_STATUS_SUCCESS;
    uint32_t           tileFormatLinux = I915_TILING_NONE;
    uint32_t           alignedHeight   = params.m_height;
    uint32_t           bufHeight       = params.m_height;
    GMM_RESOURCE_TYPE  resourceType    = RESOURCE_2D;

    GMM_RESCREATE_PARAMS    gmmParams;
    MOS_ZeroMemory(&gmmParams, sizeof(gmmParams));

    switch (params.m_format)
    {
        case Format_Buffer:
        case Format_RAW:
            resourceType    = RESOURCE_BUFFER;
            alignedHeight   = 1;
            //indicate buffer Restriction is Vertex.
            gmmParams.Flags.Gpu.State = true;
            break;
        case Format_L8:
        case Format_L16:
        case Format_STMM:
        case Format_AI44:
        case Format_IA44:
        case Format_R5G6B5:
        case Format_X8R8G8B8:
        case Format_A8R8G8B8:
        case Format_X8B8G8R8:
        case Format_A8B8G8R8:
        case Format_R32S:
        case Format_R32F:
        case Format_V8U8:
        case Format_YUY2:
        case Format_UYVY:
        case Format_P8:
        case Format_A8:
        case Format_AYUV:
        case Format_NV12:
        case Format_NV21:
        case Format_YV12:
        case Format_Buffer_2D:
        case Format_R32U:
        case Format_444P:
        case Format_422H:
        case Format_422V:
        case Format_IMC3:
        case Format_411P:
        case Format_R16U:
        case Format_R8U:
        case Format_P010:
            resourceType  = RESOURCE_2D;
            //indicate buffer Restriction is Planar surface restrictions.
            gmmParams.Flags.Gpu.Video = true;
            break;
        default:
            MOS_OS_ASSERTMESSAGE("Unsupported format");
            status = MOS_STATUS_UNIMPLEMENTED;
            return status;
    }

    // Create GmmResourceInfo
    gmmParams.BaseWidth    = params.m_width;
    gmmParams.BaseHeight   = alignedHeight;
    gmmParams.ArraySize    = 1;
    gmmParams.Type         = resourceType;
    gmmParams.Format       = ConvertMosFmtToGmmFmt(params.m_format);

    MOS_OS_CHECK_CONDITION(gmmParams.Format == GMM_FORMAT_INVALID,
                         "Unsupported format",
                         MOS_STATUS_UNKNOWN);

    MOS_TILE_TYPE tileformat = params.m_tileType;
    switch(tileformat)
    {
        case MOS_TILE_Y:
            gmmParams.Flags.Info.TiledY   = true;
            gmmParams.Flags.Gpu.MMC       = params.m_isCompressed;
            tileFormatLinux               = I915_TILING_Y;
            break;
        case MOS_TILE_X:
            gmmParams.Flags.Info.TiledX   = true;
            tileFormatLinux               = I915_TILING_X;
            break;
        default:
            gmmParams.Flags.Info.Linear   = true;
            tileFormatLinux               = I915_TILING_NONE;
    }

    GMM_RESOURCE_INFO*  gmmResourceInfoPtr = pOsContextSpecific->GetGmmClientContext()->CreateResInfoObject(&gmmParams);

    if (gmmResourceInfoPtr == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    switch (gmmResourceInfoPtr->GetTileType())
    {
        case GMM_TILED_X:
            tileformat      = MOS_TILE_X;
            tileFormatLinux = I915_TILING_X;
            break;
        case GMM_TILED_Y:
            tileformat      = MOS_TILE_Y;
            tileFormatLinux = I915_TILING_Y;
            break;
        case GMM_NOT_TILED:
        default:
            tileformat      = MOS_TILE_LINEAR;
            tileFormatLinux = I915_TILING_NONE;
            break;
    }

    if (params.m_tileType== MOS_TILE_Y)
    {
        GmmResSetMmcMode(gmmResourceInfoPtr, (GMM_RESOURCE_MMC_INFO)params.m_compressionMode, 0);
    }

    uint32_t bufPitch        = GFX_ULONG_CAST(gmmResourceInfoPtr->GetRenderPitch());
    uint32_t bufSize         = GmmResGetRenderSize(gmmResourceInfoPtr);
    bufHeight                = gmmResourceInfoPtr->GetBaseHeight();
    unsigned long linuxPitch = 0;
    MOS_LINUX_BO* boPtr      = nullptr;

    char bufName[m_maxBufNameLength];
    MOS_SecureStrcpy(bufName, m_maxBufNameLength, params.m_name.c_str());

#if defined(I915_PARAM_CREATE_VERSION)
    drm_i915_getparam_t gp;
    int32_t gpvalue;
    int32_t ret;
    gpvalue = 0;
    ret = -1;
    memset( &gp, 0, sizeof(gp) );
    gp.value = &gpvalue;
    gp.param = I915_PARAM_CREATE_VERSION;

    ret = drmIoctl(pOsContextSpecific->m_fd, DRM_IOCTL_I915_GETPARAM, &gp );

    if ((0 == ret) && ( tileFormatLinux != I915_TILING_NONE))
    {
        boPtr = mos_bo_alloc_tiled(pOsContextSpecific->m_bufmgr, bufName, bufPitch, bufSize/bufPitch, 1,
                 &tileFormatLinux, &linuxPitch, BO_ALLOC_STOLEN);
        if (nullptr == boPtr)
        {
            boPtr = mos_bo_alloc_tiled(pOsContextSpecific->m_bufmgr, bufName, bufPitch, bufSize/bufPitch, 1, &tileFormatLinux, &linuxPitch, 0);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Stolen memory is created sucessfully on Mos");
        }
        bufPitch = (uint32_t)linuxPitch;
    }
    else
#endif
    {
        // Only Linear and Y TILE supported
        if( tileFormatLinux == I915_TILING_NONE )
        {
            boPtr = mos_bo_alloc(pOsContextSpecific->m_bufmgr, bufName, bufSize, 4096);
        }
        else
        {
            boPtr = mos_bo_alloc_tiled(pOsContextSpecific->m_bufmgr, bufName, bufPitch, bufSize/bufPitch, 1, &tileFormatLinux, &linuxPitch, 0);
            bufPitch = (uint32_t)linuxPitch;
        }

    }

    m_mapped = false;
    if (boPtr)
    {
        m_format   = params.m_format;
        m_width    = params.m_width;
        m_height   = bufHeight;
        m_pitch    = bufPitch;
        m_qPitch   = gmmResourceInfoPtr->GetQPitch();
        m_count    = 0;
        m_bo       = boPtr;
        m_name     = params.m_name;
        m_pData    = (uint8_t*) boPtr->virt;

        m_gmmResInfo    = gmmResourceInfoPtr;
        m_mapped        = false;
        m_mmapOperation = MOS_MMAP_OPERATION_NONE;

        m_arraySize = 1;
        m_depth     = MOS_MAX(1, gmmResourceInfoPtr->GetBaseDepth());
        m_size      = GmmResGetRenderSize(gmmResourceInfoPtr);
        m_tileType  = tileformat;

#ifdef ANDROID
        intel_ufo_bo_datatype_t datatype;

        datatype.value = 0;
        mos_bo_get_datatype(boPtr, &datatype.value);
        datatype.compression_mode = (uint32_t)params.m_compressionMode;
        datatype.is_mmc_capable   = (uint32_t)gmmParams.Flags.Gpu.MMC;
        datatype.compression_hint = INTEL_UFO_BUFFER_HINT_MMC_COMPRESSED;
        mos_bo_set_datatype(boPtr, datatype.value);

        datatype.value = 0;
        mos_bo_get_datatype(boPtr, &datatype.value);
        m_compressible    = (datatype.is_mmc_capable && (datatype.compression_hint == INTEL_UFO_BUFFER_HINT_MMC_COMPRESSED));
        m_isCompressed    = datatype.compression_mode ? true : false;
        m_compressionMode = (MOS_RESOURCE_MMC_MODE)datatype.compression_mode;
#else
        m_compressible    = gmmParams.Flags.Gpu.MMC ?
            (GmmResGetMmcHint(gmmResourceInfoPtr, 0) == GMM_MMC_HINT_ON) : false;
        m_isCompressed    = GmmResIsMediaMemoryCompressed(gmmResourceInfoPtr, 0);
        m_compressionMode = (MOS_RESOURCE_MMC_MODE)GmmResGetMmcMode(gmmResourceInfoPtr, 0);
#endif

        MOS_OS_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource).",bufSize, params.m_width, bufHeight);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).",bufSize, params.m_width, params.m_height);
        status = MOS_STATUS_NO_SPACE;
    }

    m_memAllocCounterGfx++;
    return  status;
}

void GraphicsResourceSpecific::Free(OsContext* osContextPtr, uint32_t  freeFlag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_UNUSED(osContextPtr);
    MOS_UNUSED(freeFlag);

    OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(osContextPtr);

    MOS_LINUX_BO* boPtr = m_bo;

    if (boPtr)
    {
        mos_bo_unreference(boPtr);
        m_bo = nullptr;
        if (nullptr != m_gmmResInfo)
        {
            pOsContextSpecific->GetGmmClientContext()->DestroyResInfoObject(m_gmmResInfo);
            m_gmmResInfo = nullptr;
            m_memAllocCounterGfx--;
        }
    }
    return;
}

bool GraphicsResourceSpecific::IsEqual(GraphicsResource* toCompare)
{
    if  (toCompare == nullptr)
    {
        return false;
    }

    GraphicsResourceSpecific *resSpecificPtr = static_cast<GraphicsResourceSpecific *>(toCompare);

    return (m_bo == resSpecificPtr->m_bo);
}

bool GraphicsResourceSpecific::IsValid()
{
    return (m_bo != nullptr);
}

MOS_STATUS GraphicsResourceSpecific::ConvertToMosResource(MOS_RESOURCE* pMosResource)
{
    if (pMosResource == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    pMosResource->Format   = m_format;
    pMosResource->iWidth   = m_width;
    pMosResource->iHeight  = m_height;
    pMosResource->iPitch   = m_pitch;
    pMosResource->iDepth   = m_depth;
    pMosResource->TileType = m_tileType;
    pMosResource->iCount   = 0;
    pMosResource->pData    = m_pData;
    pMosResource->bufname  = m_name.c_str();
    pMosResource->bo       = m_bo;
    pMosResource->bMapped  = m_mapped;
    pMosResource->MmapOperation = m_mmapOperation;
    pMosResource->pGmmResInfo   = m_gmmResInfo;

    pMosResource->user_provided_va    = m_userProvidedVA;

    pMosResource->pGfxResource    = this;

    return  MOS_STATUS_SUCCESS;
}

void* GraphicsResourceSpecific::Lock(OsContext* osContextPtr, LockParams& params)
{
    MOS_OS_FUNCTION_ENTER;

    if (osContextPtr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
        return nullptr;
    }

    if (osContextPtr ->GetOsContextValid() == false)
    {
        MOS_OS_ASSERTMESSAGE("The OS context got is not valid.");
        return nullptr;
    }

    OsContextSpecific *pOsContextSpecific  = static_cast<OsContextSpecific *>(osContextPtr);

    void*   dataPtr     = nullptr;
    MOS_LINUX_BO* boPtr = m_bo;

    if (boPtr)
    {
        // Do decompression for a compressed surface before lock
        const auto pGmmResInfo = m_gmmResInfo;
        if (!params.m_noDecompress &&
             GmmResIsMediaMemoryCompressed(pGmmResInfo, 0))
        {
            if ((pOsContextSpecific->m_mediaMemDecompState == nullptr) ||
                (pOsContextSpecific->m_memoryDecompress    == nullptr))
            {
                MOS_OS_ASSERTMESSAGE("m_mediaMemDecompState/m_memoryDecompress is not valid.");
                return nullptr;
            }

            MOS_RESOURCE mosResource = {};
            ConvertToMosResource(&mosResource);
            pOsContextSpecific->m_memoryDecompress(pOsContextSpecific->m_mosContext, &mosResource);
        }

        if(false == m_mapped)
        {
            if (pOsContextSpecific->IsAtomSoc())
            {
                mos_gem_bo_map_gtt(boPtr);
            }
            else
            {
#ifdef ANDROID
                if (m_tileType != MOS_TILE_LINEAR ||params.m_uncached)
                {
                    mos_gem_bo_map_gtt(boPtr);
                }
                else
                {
                    mos_bo_map(boPtr, ( OSKM_LOCKFLAG_WRITEONLY & params.m_writeRequest ));
                }
#else
                if (m_tileType != MOS_TILE_LINEAR)
                {
                    mos_gem_bo_map_gtt(boPtr);
                    m_mmapOperation = MOS_MMAP_OPERATION_MMAP_GTT;
                }
                else if (params.m_uncached)
                {
                    mos_gem_bo_map_wc(boPtr);
                    m_mmapOperation = MOS_MMAP_OPERATION_MMAP_WC;
                }
                else
                {
                    mos_bo_map(boPtr, ( OSKM_LOCKFLAG_WRITEONLY & params.m_writeRequest ));
                    m_mmapOperation = MOS_MMAP_OPERATION_MMAP;
                }
#endif
            }
            m_mapped = true;
            m_pData  = (uint8_t *)boPtr->virt;
        }

        dataPtr = boPtr->virt;
    }

    MOS_OS_ASSERT(dataPtr);
    return dataPtr;
}

MOS_STATUS GraphicsResourceSpecific::Unlock(OsContext* osContextPtr)
{
    MOS_OS_FUNCTION_ENTER;

    if (osContextPtr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
        return MOS_STATUS_INVALID_HANDLE;
    }

    if (osContextPtr ->GetOsContextValid() == false)
    {
        MOS_OS_ASSERTMESSAGE("The OS context got is not valid.");
        return MOS_STATUS_INVALID_HANDLE;
    }

    OsContextSpecific *pOsContextSpecific  = static_cast<OsContextSpecific *>(osContextPtr);

    MOS_LINUX_BO* boPtr = m_bo;
    if (boPtr)
    {
        if (m_mapped)
        {
           if (pOsContextSpecific->IsAtomSoc())
           {
               mos_gem_bo_unmap_gtt(boPtr);
           }
           else
           {
#ifdef ANDROID

               if (m_tileType == MOS_TILE_LINEAR)
               {
                   mos_bo_unmap(boPtr);
               }
               else
               {
                   mos_gem_bo_unmap_gtt(boPtr);
               }
#else
               switch(m_mmapOperation)
               {
                   case MOS_MMAP_OPERATION_MMAP_GTT:
                        mos_gem_bo_unmap_gtt(boPtr);
                        break;
                   case MOS_MMAP_OPERATION_MMAP_WC:
                        mos_gem_bo_unmap_wc(boPtr);
                        break;
                   case MOS_MMAP_OPERATION_MMAP:
                        mos_bo_unmap(boPtr);
                        break;
                   default:
                        MOS_OS_ASSERTMESSAGE("Invalid mmap operation type");
                        break;
               }
#endif
            }

            m_mapped           = false;
            m_mmapOperation    = MOS_MMAP_OPERATION_NONE;

            boPtr->virt        = nullptr;
            m_bo = boPtr;
        }

        m_pData = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

