/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     mos_graphicsresource_specific_next.cpp
//! \brief   Container class for the linux/Android specfic graphic resource
//!

#include "mos_defs.h"
#include "mos_util_debug.h"

#include "mos_graphicsresource_specific_next.h"
#include "mos_context_specific_next.h"
#include "memory_policy_manager.h"

GraphicsResourceSpecificNext::GraphicsResourceSpecificNext()
{
    MOS_OS_FUNCTION_ENTER;
}

GraphicsResourceSpecificNext::~GraphicsResourceSpecificNext()
{
    MOS_OS_FUNCTION_ENTER;
}

bool GraphicsResourceSpecificNext::ResourceIsNull()
{
    return ((m_bo == nullptr)
#if (_DEBUG || _RELEASE_INTERNAL)
         && ((m_pData == nullptr) )
#endif // (_DEBUG || _RELEASE_INTERNAL)
    );

}

MOS_STATUS GraphicsResourceSpecificNext::Allocate(OsContextNext* osContextPtr, CreateParams& params)
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

    OsContextSpecificNext *pOsContextSpecific = static_cast<OsContextSpecificNext *>(osContextPtr);

    GMM_CLIENT_CONTEXT    *gmmClientContext = pOsContextSpecific->GetGmmClientContext();
    if (nullptr == gmmClientContext)
    {
        MOS_OS_ASSERTMESSAGE("Get GMM Client Context failed.");
        return MOS_STATUS_INVALID_HANDLE;
    }

    MOS_STATUS         status          = MOS_STATUS_SUCCESS;
    uint32_t           tileFormatLinux = TILING_NONE;
    uint32_t           alignedHeight   = params.m_height;
    uint32_t           bufHeight       = params.m_height;
    GMM_RESOURCE_TYPE  resourceType    = RESOURCE_2D;
    int32_t            mem_type        = MOS_MEMPOOL_VIDEOMEMORY;

    GMM_RESCREATE_PARAMS    gmmParams;
    MosUtilities::MosZeroMemory(&gmmParams, sizeof(gmmParams));

    gmmParams.Usage   = params.m_gmmResUsageType;

    switch (params.m_type)
    {
        case MOS_GFXRES_BUFFER:
        case MOS_GFXRES_SCRATCH:
          gmmParams.Type = RESOURCE_BUFFER;
          gmmParams.Flags.Gpu.State = true;
          alignedHeight = 1;
          break;

        case MOS_GFXRES_2D:
            gmmParams.Type = RESOURCE_2D;
            gmmParams.Flags.Gpu.Video = true;
            break;

        case MOS_GFXRES_VOLUME:
            gmmParams.Type = RESOURCE_3D;
            gmmParams.Flags.Gpu.Video = true;
            gmmParams.Depth = params.m_depth;
            break;

        default:
            MOS_OS_ASSERTMESSAGE("Unknown surface type");
            return MOS_STATUS_UNKNOWN;
    }

    // Create GmmResourceInfo
    gmmParams.Format = MosInterface::MosFmtToGmmFmt(params.m_format);
    if (gmmParams.Format == GMM_FORMAT_INVALID)
    {
        MOS_OS_ASSERTMESSAGE("Unsupported format");
        return MOS_STATUS_UNIMPLEMENTED;
    }
    gmmParams.BaseWidth = params.m_width;
    gmmParams.BaseHeight = alignedHeight;
    gmmParams.ArraySize = 1;

     if (MEDIA_IS_SKU(pOsContextSpecific->GetSkuTable(), FtrXe2Compression))
     {
        // Init NotCompressed flag as true to default Create as uncompressed surface on Xe2 Compression.
        gmmParams.Flags.Info.NotCompressed = 1;
     }

    MOS_TILE_TYPE tileformat = params.m_tileType;
    MOS_OS_NORMALMESSAGE("tilemode: tileformat = %d for %s, tileModeByForce = %d", tileformat, params.m_name.c_str(), params.m_tileModeByForce);
    switch (tileformat)
    {
        case MOS_TILE_Y:
            tileFormatLinux               = TILING_Y;
            if (params.m_isCompressible                                            &&
                MEDIA_IS_SKU(pOsContextSpecific->GetSkuTable(), FtrE2ECompression) &&
                MEDIA_IS_SKU(pOsContextSpecific->GetSkuTable(), FtrCompressibleSurfaceDefault))
            {
                gmmParams.Flags.Gpu.MMC  = 1;
                
                if (params.m_compressionMode == MOS_MMC_RC)
                {
                    gmmParams.Flags.Info.MediaCompressed = 0;
                    gmmParams.Flags.Info.RenderCompressed = 1;
                }
                else
                {
                    gmmParams.Flags.Info.MediaCompressed = 1;
                    gmmParams.Flags.Info.RenderCompressed = 0;
                }

                gmmParams.Flags.Gpu.CCS = 1;
                gmmParams.Flags.Gpu.UnifiedAuxSurface = 1;
                gmmParams.Flags.Gpu.RenderTarget = 1;

                if(MEDIA_IS_SKU(pOsContextSpecific->GetSkuTable(), FtrFlatPhysCCS))
                {
                    gmmParams.Flags.Gpu.UnifiedAuxSurface = 0;
                }

                if(MEDIA_IS_SKU(pOsContextSpecific->GetSkuTable(), FtrXe2Compression))
                {
                    gmmParams.Flags.Info.NotCompressed = 0;
                }
            }
            SetTileModebyForce(gmmParams, params.m_tileModeByForce);
            break;
        case MOS_TILE_X:
            gmmParams.Flags.Info.TiledX   = true;
            tileFormatLinux               = TILING_X;
            break;
        default:
            gmmParams.Flags.Info.Linear   = true;
            tileFormatLinux               = TILING_NONE;
    }

    if (nullptr != params.m_pSystemMemory)
    {
        // If user provides a system memory pointer, the gfx resource is backed
        // by the system memory pages. The resource is required to be linear.
        gmmParams.Flags.Info.Linear     = true;
        gmmParams.Flags.Info.Cacheable  = true;
        gmmParams.NoGfxMemory           = true;
        GMM_RESOURCE_INFO *tmpGmmResInfoPtr = pOsContextSpecific
                ->GetGmmClientContext()->CreateResInfoObject(&gmmParams);
        if (tmpGmmResInfoPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Create GmmResInfo failed");
            return MOS_STATUS_UNKNOWN;
        }

        gmmParams.ExistingSysMemSize = GFX_ULONG_CAST(tmpGmmResInfoPtr->GetSizeSurface());
        gmmParams.pExistingSysMem = (GMM_VOIDPTR64)params.m_pSystemMemory;
        gmmParams.NoGfxMemory = false;
        gmmParams.Flags.Info.ExistingSysMem = true;

        pOsContextSpecific->GetGmmClientContext()
                ->DestroyResInfoObject(tmpGmmResInfoPtr);
    }
    else
    {
        gmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(pOsContextSpecific->GetSkuTable(), FtrLocalMemory);
    }

    GMM_RESOURCE_INFO*  gmmResourceInfoPtr = pOsContextSpecific->GetGmmClientContext()->CreateResInfoObject(&gmmParams);

    if (gmmResourceInfoPtr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Get gmmResourceInfoPtr failed.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    switch (gmmResourceInfoPtr->GetTileType())
    {
        case GMM_TILED_X:
            tileformat      = MOS_TILE_X;
            tileFormatLinux = TILING_X;
            break;
        case GMM_TILED_Y:
            tileformat      = MOS_TILE_Y;
            tileFormatLinux = TILING_Y;
            break;
        case GMM_NOT_TILED:
            tileformat      = MOS_TILE_LINEAR;
            tileFormatLinux = TILING_NONE;
            break;
        default:
            tileformat      = MOS_TILE_Y;
            tileFormatLinux = TILING_Y;
            break;
    }

    if (params.m_tileType== MOS_TILE_Y)
    {
        gmmResourceInfoPtr->SetMmcMode((GMM_RESOURCE_MMC_INFO)params.m_compressionMode, 0);
    }

    if(!params.m_pSystemMemory)
    {
        MemoryPolicyParameter memPolicyPar;
        MOS_ZeroMemory(&memPolicyPar, sizeof(MemoryPolicyParameter));

        memPolicyPar.skuTable         = pOsContextSpecific->GetSkuTable();
        memPolicyPar.waTable          = pOsContextSpecific->GetWaTable();
        memPolicyPar.resInfo          = gmmResourceInfoPtr;
        memPolicyPar.resName          = params.m_name.c_str();
        memPolicyPar.preferredMemType = params.m_memType;
        memPolicyPar.isServer         = PLATFORM_INFORMATION_IS_SERVER & mos_get_platform_information(pOsContextSpecific->GetBufMgr());

        mem_type = MemoryPolicyManager::UpdateMemoryPolicy(&memPolicyPar);
    }

    uint32_t bufPitch        = GFX_ULONG_CAST(gmmResourceInfoPtr->GetRenderPitch());
    uint32_t bufSize         = GFX_ULONG_CAST(gmmResourceInfoPtr->GetSizeSurface());
    bufHeight                = gmmResourceInfoPtr->GetBaseHeight();
    MOS_LINUX_BO* boPtr      = nullptr;

    char bufName[m_maxBufNameLength];
    MosUtilities::MosSecureStrcpy(bufName, m_maxBufNameLength, params.m_name.c_str());

    unsigned int patIndex = MosInterface::GetPATIndexFromGmm(gmmClientContext, gmmResourceInfoPtr);
    bool isCpuCacheable   = gmmResourceInfoPtr->GetResFlags().Info.Cacheable;

    MOS_TraceEventExt(EVENT_RESOURCE_ALLOCATE, EVENT_TYPE_START, nullptr, 0, nullptr, 0);
    if (nullptr != params.m_pSystemMemory)
    {
        struct mos_drm_bo_alloc_userptr alloc_uptr;
        alloc_uptr.name = bufName;
        alloc_uptr.addr = params.m_pSystemMemory;
        alloc_uptr.tiling_mode = tileFormatLinux;
        alloc_uptr.stride = bufPitch;
        alloc_uptr.size = bufSize;
        alloc_uptr.pat_index = patIndex;

        boPtr = mos_bo_alloc_userptr(pOsContextSpecific->m_bufmgr, &alloc_uptr);
    }
    // Only Linear and Y TILE supported
    else if (tileFormatLinux == TILING_NONE)
    {
        struct mos_drm_bo_alloc alloc;
        alloc.name = bufName;
        alloc.size = bufSize;
        alloc.alignment = 4096;
        alloc.ext.mem_type = mem_type;
        alloc.ext.pat_index = patIndex;
        alloc.ext.cpu_cacheable = isCpuCacheable;
        boPtr = mos_bo_alloc(pOsContextSpecific->m_bufmgr, &alloc);
    }
    else
    {
        struct mos_drm_bo_alloc_tiled alloc_tiled;
        alloc_tiled.name = bufName;
        alloc_tiled.x = bufPitch;
        alloc_tiled.y = bufSize/bufPitch;
        alloc_tiled.cpp = 1;
        alloc_tiled.ext.tiling_mode = tileFormatLinux;
        alloc_tiled.ext.mem_type = mem_type;
        alloc_tiled.ext.pat_index = patIndex;
        alloc_tiled.ext.cpu_cacheable = isCpuCacheable;

        boPtr = mos_bo_alloc_tiled(pOsContextSpecific->m_bufmgr, &alloc_tiled);
        bufPitch = (uint32_t)alloc_tiled.pitch;
    }

    m_mapped = false;
    if (boPtr)
    {
        m_format   = params.m_format;
        m_width    = params.m_width;
        m_height   = bufHeight;
        m_pitch    = bufPitch;
        m_count    = 0;
        m_bo       = boPtr;
        m_name     = params.m_name;
        m_pData    = (uint8_t*) boPtr->virt;

        m_gmmResInfo    = gmmResourceInfoPtr;
        m_mapped        = false;
        m_mmapOperation = MOS_MMAP_OPERATION_NONE;

        m_arraySize = 1;
        m_depth     = MOS_MAX(1, gmmResourceInfoPtr->GetBaseDepth());
        m_size      = (uint32_t)gmmResourceInfoPtr->GetSizeSurface();
        m_tileType  = tileformat;
        m_tileModeGMM           = (MOS_TILE_MODE_GMM)gmmResourceInfoPtr->GetTileModeSurfaceState();
        m_isGMMTileEnabled      = true;

        m_compressible    = gmmParams.Flags.Gpu.MMC ?
            (gmmResourceInfoPtr->GetMmcHint(0) == GMM_MMC_HINT_ON) : false;
        m_isCompressed    = gmmResourceInfoPtr->IsMediaMemoryCompressed(0);
        m_compressionMode = (MOS_RESOURCE_MMC_MODE)gmmResourceInfoPtr->GetMmcMode(0);
        m_memObjCtrlState = MosInterface::GetCachePolicyMemoryObject(pOsContextSpecific->GetGmmClientContext(), params.m_mocsMosResUsageType);

        MOS_OS_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource), tile encoding %d.", bufSize, params.m_width, bufHeight, m_tileModeGMM);

        struct {
            uint32_t m_handle;
            uint32_t m_resFormat;
            uint32_t m_baseWidth;
            uint32_t m_baseHeight;
            uint32_t m_pitch;
            uint32_t m_size;
            uint32_t m_resTileType;
            GMM_RESOURCE_FLAG m_resFlag;
            uint32_t          m_reserve;
        } eventData;

        eventData.m_handle       = boPtr->handle;
        eventData.m_baseWidth    = m_width;
        eventData.m_baseHeight   = m_height;
        eventData.m_pitch        = m_pitch;
        eventData.m_size         = m_size;
        eventData.m_resFormat    = m_format;
        eventData.m_resTileType  = m_tileType;
        eventData.m_resFlag      = gmmResourceInfoPtr->GetResFlags();
        eventData.m_reserve      = 0;
        MOS_TraceEventExt(EVENT_RESOURCE_ALLOCATE,
            EVENT_TYPE_INFO,
            &eventData,
            sizeof(eventData),
            params.m_name.c_str(),
            params.m_name.size() + 1);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).",bufSize, params.m_width, params.m_height);
        status = MOS_STATUS_NO_SPACE;
    }
    MOS_TraceEventExt(EVENT_RESOURCE_ALLOCATE, EVENT_TYPE_END, &status, sizeof(status), nullptr, 0);

    m_memAllocCounterGfx++;
    return  status;
}

void GraphicsResourceSpecificNext::Free(OsContextNext* osContextPtr, uint32_t  freeFlag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_UNUSED(osContextPtr);
    MOS_UNUSED(freeFlag);

    OsContextSpecificNext *pOsContextSpecific = static_cast<OsContextSpecificNext *>(osContextPtr);

    MOS_LINUX_BO* boPtr = m_bo;

    if (boPtr)
    {
        AuxTableMgr *auxTableMgr = pOsContextSpecific->GetAuxTableMgr();
        if (auxTableMgr)
        {
            auxTableMgr->UnmapResource(m_gmmResInfo, boPtr);
        }
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

bool GraphicsResourceSpecificNext::IsEqual(GraphicsResourceNext* toCompare)
{
    if  (toCompare == nullptr)
    {
        return false;
    }

    GraphicsResourceSpecificNext *resSpecificPtr = static_cast<GraphicsResourceSpecificNext *>(toCompare);

    return (m_bo == resSpecificPtr->m_bo);
}

bool GraphicsResourceSpecificNext::IsValid()
{
    return (m_bo != nullptr);
}

MOS_STATUS GraphicsResourceSpecificNext::ConvertToMosResource(MOS_RESOURCE* pMosResource)
{
    if (pMosResource == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    pMosResource->Format   = m_format;
    pMosResource->iWidth   = m_width;
    pMosResource->iHeight  = m_height;
    pMosResource->iSize    = m_size;
    pMosResource->iPitch   = m_pitch;
    pMosResource->iDepth   = m_depth;
    pMosResource->TileType = m_tileType;
    pMosResource->TileModeGMM = m_tileModeGMM;
    pMosResource->bGMMTileEnabled = m_isGMMTileEnabled;
    pMosResource->iCount   = 0;
    pMosResource->pData    = m_pData;
    pMosResource->bufname  = m_name.c_str();
    pMosResource->bo       = m_bo;
    pMosResource->bMapped  = m_mapped;
    pMosResource->MmapOperation = m_mmapOperation;
    pMosResource->pGmmResInfo   = m_gmmResInfo;

    pMosResource->user_provided_va  = m_userProvidedVA;

    pMosResource->memObjCtrlState   = m_memObjCtrlState;
    pMosResource->mocsMosResUsageType = m_mocsMosResUsageType;

    pMosResource->pGfxResourceNext  = this;

    return  MOS_STATUS_SUCCESS;
}

void* GraphicsResourceSpecificNext::Lock(OsContextNext* osContextPtr, LockParams& params)
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

    OsContextSpecificNext *pOsContextSpecific = static_cast<OsContextSpecificNext *>(osContextPtr);

    void*   dataPtr     = nullptr;
    MOS_LINUX_BO* boPtr = m_bo;

    if (boPtr)
    {
        // Do decompression for a compressed surface before lock
        const auto pGmmResInfo = m_gmmResInfo;
        MOS_OS_ASSERT(pGmmResInfo);
        GMM_RESOURCE_FLAG GmmFlags = pGmmResInfo->GetResFlags();

        if (!params.m_noDecompress &&
            (((GmmFlags.Gpu.MMC || GmmFlags.Gpu.CCS) && GmmFlags.Info.MediaCompressed) ||
             pGmmResInfo->IsMediaMemoryCompressed(0)))
        {
            MOS_RESOURCE mosResource = {};
            ConvertToMosResource(&mosResource);

            MosDecompression *mosDecompression = pOsContextSpecific->GetMosDecompression();
            if (nullptr == mosDecompression)
            {
                MOS_OS_ASSERTMESSAGE("mosDecompression is NULL.");
                return nullptr;
            }
            mosDecompression->MemoryDecompress(&mosResource);
        }

        if(false == m_mapped)
        {
            if (pOsContextSpecific->IsAtomSoc())
            {
                mos_bo_map_gtt(boPtr);
            }
            else
            {
                if (m_tileType != MOS_TILE_LINEAR && !params.m_tileAsTiled)
                {
                    if (pOsContextSpecific->UseSwSwizzling())
                    {
                        mos_bo_map(boPtr, ( OSKM_LOCKFLAG_WRITEONLY & params.m_writeRequest ));
                        m_mmapOperation = MOS_MMAP_OPERATION_MMAP;
                        if (m_systemShadow == nullptr)
                        {
                            m_systemShadow = (uint8_t *)MOS_AllocMemory(boPtr->size);
                            MOS_OS_CHECK_CONDITION((m_systemShadow == nullptr), "Failed to allocate shadow surface", nullptr);
                        }
                        if (m_systemShadow)
                        {
                            int32_t flags = pOsContextSpecific->GetTileYFlag() ? 0 : 1;
                            uint64_t surfSize = m_gmmResInfo->GetSizeMainSurface();
                            MOS_OS_CHECK_CONDITION((m_tileType != MOS_TILE_Y), "Unsupported tile type", nullptr);
                            MOS_OS_CHECK_CONDITION((boPtr->size <= 0 || m_pitch <= 0), "Invalid BO size or pitch", nullptr);
                            MosUtilities::MosSwizzleData((uint8_t*)boPtr->virt, m_systemShadow,
                                            MOS_TILE_Y, MOS_TILE_LINEAR,
                                            (int32_t)(surfSize / m_pitch), m_pitch, flags);
                        }
                    }
                    else
                    {
                        mos_bo_map_gtt(boPtr);
                        m_mmapOperation = MOS_MMAP_OPERATION_MMAP_GTT;
                    }
                }
                else if (params.m_uncached)
                {
                    mos_bo_map_wc(boPtr);
                    m_mmapOperation = MOS_MMAP_OPERATION_MMAP_WC;
                }
                else
                {
                    mos_bo_map(boPtr, ( OSKM_LOCKFLAG_WRITEONLY & params.m_writeRequest ));
                    m_mmapOperation = MOS_MMAP_OPERATION_MMAP;
                }
            }
            m_mapped = true;
            m_pData  = m_systemShadow ? m_systemShadow : (uint8_t *)boPtr->virt;
        }

        dataPtr = m_pData;
    }

    MOS_OS_ASSERT(dataPtr);
    return dataPtr;
}

MOS_STATUS GraphicsResourceSpecificNext::Unlock(OsContextNext* osContextPtr)
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

    OsContextSpecificNext *pOsContextSpecific = static_cast<OsContextSpecificNext *>(osContextPtr);

    MOS_LINUX_BO* boPtr = m_bo;
    if (boPtr)
    {
        if (m_mapped)
        {
           if (pOsContextSpecific->IsAtomSoc())
           {
               mos_bo_unmap_gtt(boPtr);
           }
           else
           {

               if (m_systemShadow)
               {
                   int32_t flags = pOsContextSpecific->GetTileYFlag() ? 0 : 1;
                   uint64_t surfSize = m_gmmResInfo->GetSizeMainSurface();
                   MosUtilities::MosSwizzleData(m_systemShadow, (uint8_t*)boPtr->virt,
                                   MOS_TILE_LINEAR, MOS_TILE_Y,
                                   (int32_t)(surfSize / m_pitch), m_pitch, flags);
                   MOS_FreeMemory(m_systemShadow);
                   m_systemShadow = nullptr;
               }

               switch(m_mmapOperation)
               {
                   case MOS_MMAP_OPERATION_MMAP_GTT:
                        mos_bo_unmap_gtt(boPtr);
                        break;
                   case MOS_MMAP_OPERATION_MMAP_WC:
                        mos_bo_unmap_wc(boPtr);
                        break;
                   case MOS_MMAP_OPERATION_MMAP:
                        mos_bo_unmap(boPtr);
                        break;
                   default:
                        MOS_OS_ASSERTMESSAGE("Invalid mmap operation type");
                        break;
               }
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

MOS_STATUS GraphicsResourceSpecificNext::AllocateExternalResource(
    MOS_STREAM_HANDLE streamState,
    PMOS_ALLOC_GFXRES_PARAMS params,
    MOS_RESOURCE_HANDLE& resource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    
    const char *bufname = params->pBufName;;
    int32_t iSize = 0;
    int32_t iPitch = 0;
    MOS_LINUX_BO *bo = nullptr;
    MOS_TILE_TYPE tileformat = params->TileType;
    uint32_t tileformat_linux = TILING_NONE;
    int32_t iHeight = params->dwHeight;
    int32_t iAlignedHeight = 0;
    GMM_RESCREATE_PARAMS gmmParams;
    GMM_RESOURCE_INFO *gmmResourceInfo = nullptr;
    GMM_RESOURCE_TYPE resourceType = RESOURCE_2D;
    unsigned int patIndex = PAT_INDEX_INVALID;
    bool isCpuCacheable = true;

    MosUtilities::MosZeroMemory(&gmmParams, sizeof(gmmParams));

    MOS_OS_CHK_NULL_RETURN(streamState->perStreamParameters);
    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    gmmParams.Usage = MosInterface::GetGmmResourceUsageType(params->ResUsageType);

    switch (params->Format)
    {
    case Format_Buffer:
    case Format_RAW:
        resourceType   = RESOURCE_BUFFER;
        iAlignedHeight = 1;
        //indicate buffer Restriction is Vertex.
        gmmParams.Flags.Gpu.State = true;
        break;
    case Format_L8:
    case Format_L16:
    case Format_STMM:
    case Format_AI44:
    case Format_IA44:
    case Format_R5G6B5:
    case Format_R8G8B8:
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
    case Format_411R:
    case Format_RGBP:
    case Format_BGRP:
    case Format_R16U:
    case Format_R8U:
    case Format_R8UN:
    case Format_P010:
    case Format_P016:
    case Format_Y216:
    case Format_Y416:
    case Format_P208:
    case Format_Y210:
    case Format_Y410:
    case Format_R16F:
        resourceType = RESOURCE_2D;
        //indicate buffer Restriction is Planar surface restrictions.
        gmmParams.Flags.Gpu.Video = true;
        break;
    default:
        MOS_OS_ASSERTMESSAGE("Unsupported format");
        eStatus = MOS_STATUS_UNIMPLEMENTED;
        return eStatus;
    }

    // Create GmmResourceInfo
    gmmParams.BaseWidth  = params->dwWidth;
    gmmParams.BaseHeight = iAlignedHeight;
    gmmParams.ArraySize  = 1;
    gmmParams.Type       = resourceType;
    gmmParams.Format     = MosInterface::MosFmtToGmmFmt(params->Format);

    MOS_OS_CHECK_CONDITION(gmmParams.Format == GMM_FORMAT_INVALID,
        "Unsupported format",
        MOS_STATUS_UNKNOWN);

    switch (tileformat)
    {
    case MOS_TILE_Y:
        gmmParams.Flags.Gpu.MMC = params->bIsCompressible;
        tileformat_linux        = TILING_Y;
        break;
    case MOS_TILE_X:
        gmmParams.Flags.Info.TiledX = true;
        tileformat_linux            = TILING_X;
        break;
    default:
        gmmParams.Flags.Info.Linear = true;
        tileformat_linux            = TILING_NONE;
    }
    gmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(&perStreamParameters->m_skuTable, FtrLocalMemory);

    MOS_OS_CHK_NULL_RETURN(perStreamParameters->pGmmClientContext);
    resource->pGmmResInfo = gmmResourceInfo = perStreamParameters->pGmmClientContext->CreateResInfoObject(&gmmParams);

    MOS_OS_CHK_NULL_RETURN(gmmResourceInfo);

    switch (gmmResourceInfo->GetTileType())
    {
    case GMM_TILED_X:
        tileformat       = MOS_TILE_X;
        tileformat_linux = TILING_X;
        break;
    case GMM_TILED_Y:
        tileformat       = MOS_TILE_Y;
        tileformat_linux = TILING_Y;
        break;
    case GMM_NOT_TILED:
        tileformat       = MOS_TILE_LINEAR;
        tileformat_linux = TILING_NONE;
        break;
    default:
        tileformat       = MOS_TILE_Y;
        tileformat_linux = TILING_Y;
        break;
    }

    if (params->TileType == MOS_TILE_Y)
    {
        gmmResourceInfo->SetMmcMode((GMM_RESOURCE_MMC_INFO)params->CompressionMode, 0);
    }

    iPitch  = GFX_ULONG_CAST(gmmResourceInfo->GetRenderPitch());
    iSize   = GFX_ULONG_CAST(gmmResourceInfo->GetSizeSurface());
    iHeight = gmmResourceInfo->GetBaseHeight();

    patIndex = MosInterface::GetPATIndexFromGmm(perStreamParameters->pGmmClientContext, gmmResourceInfo);
    isCpuCacheable = gmmResourceInfo->GetResFlags().Info.Cacheable;

    // Only Linear and Y TILE supported
    if (tileformat_linux == TILING_NONE)
    {
        struct mos_drm_bo_alloc alloc;
        alloc.name = bufname;
        alloc.size = iSize;
        alloc.alignment = 4096;
        alloc.ext.mem_type = MOS_MEMPOOL_VIDEOMEMORY;
        alloc.ext.pat_index = patIndex;
        alloc.ext.cpu_cacheable = isCpuCacheable;
        bo = mos_bo_alloc(perStreamParameters->bufmgr, &alloc);
    }
    else
    {
        struct mos_drm_bo_alloc_tiled alloc_tiled;
        alloc_tiled.name = bufname;
        alloc_tiled.x = iPitch;
        alloc_tiled.y = iSize / iPitch;
        alloc_tiled.cpp = 1;
        alloc_tiled.ext.tiling_mode = tileformat_linux;
        alloc_tiled.ext.mem_type = MOS_MEMPOOL_VIDEOMEMORY;
        alloc_tiled.ext.pat_index = patIndex;
        alloc_tiled.ext.cpu_cacheable = isCpuCacheable;

        bo = mos_bo_alloc_tiled(perStreamParameters->bufmgr, &alloc_tiled);
        iPitch = (int32_t)alloc_tiled.pitch;
    }

    resource->bMapped = false;
    if (bo)
    {
        resource->Format   = params->Format;
        resource->iWidth   = params->dwWidth;
        resource->iHeight  = iHeight;
        resource->iPitch   = iPitch;
        resource->iCount   = 0;
        resource->bufname  = bufname;
        resource->bo       = bo;
        resource->TileType = tileformat;
        resource->TileModeGMM     = (MOS_TILE_MODE_GMM)gmmResourceInfo->GetTileModeSurfaceState();
        resource->bGMMTileEnabled = true;
        resource->pData    = (uint8_t *)bo->virt;  //It is useful for batch buffer to fill commands
        if (params->ResUsageType == MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC ||
            params->ResUsageType >= MOS_HW_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS)
        {
            resource->memObjCtrlState = MosInterface::GetCachePolicyMemoryObject(perStreamParameters->pGmmClientContext, MOS_MP_RESOURCE_USAGE_DEFAULT);
            resource->mocsMosResUsageType = MOS_MP_RESOURCE_USAGE_DEFAULT;
        }
        else
        {
            resource->memObjCtrlState = MosInterface::GetCachePolicyMemoryObject(perStreamParameters->pGmmClientContext, params->ResUsageType);
            resource->mocsMosResUsageType = params->ResUsageType;
        }
        MOS_OS_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource), tile encoding.", iSize, params->dwWidth, iHeight, resource->TileModeGMM);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).", iSize, params->dwWidth, params->dwHeight);
        eStatus = MOS_STATUS_NO_SPACE;
    }

    return eStatus;
}

MOS_STATUS GraphicsResourceSpecificNext::FreeExternalResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    uint32_t            flag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    
    if (resource && resource->bo)
    {
        OsContextSpecificNext *osCtx = static_cast<OsContextSpecificNext *>(streamState->osDeviceContext);
        if (osCtx == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("osCtx is nullptr!");
            return MOS_STATUS_NULL_POINTER;
        }
        else
        {
            AuxTableMgr *auxTableMgr = osCtx->GetAuxTableMgr();

            // Unmap Resource from Aux Table
            if (auxTableMgr)
            {
                auxTableMgr->UnmapResource(resource->pGmmResInfo, resource->bo);
            }
        }

        mos_bo_unreference((MOS_LINUX_BO *)(resource->bo));

        MOS_OS_CHK_NULL_RETURN(streamState->perStreamParameters);
        auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

        if (perStreamParameters != nullptr && perStreamParameters->contextOffsetList.size())
        {
            MOS_CONTEXT *pOsCtx   = perStreamParameters;
            auto         item_ctx = pOsCtx->contextOffsetList.begin();

            for (; item_ctx != pOsCtx->contextOffsetList.end();)
            {
                if (item_ctx->target_bo == resource->bo)
                {
                    item_ctx = pOsCtx->contextOffsetList.erase(item_ctx);
                }
                else
                {
                    item_ctx++;
                }
            }
        }

        resource->bo = nullptr;
    }
    
    return eStatus;
}

void* GraphicsResourceSpecificNext::LockExternalResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource,
    PMOS_LOCK_PARAMS    flags)
{
    MOS_OS_FUNCTION_ENTER;

    void *pData = nullptr;
    if (nullptr == streamState)
    {
        MOS_OS_ASSERTMESSAGE("input parameter streamState is NULL.");
        return nullptr;
    }

    if (nullptr == resource)
    {
        MOS_OS_ASSERTMESSAGE("input parameter resource is NULL.");
        return nullptr;
    }
    
    if (streamState->perStreamParameters == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("perStreamParameters is nullptr, skip lock");
        return nullptr;
    }
    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;
    if (resource && resource->bo && resource->pGmmResInfo)
    {
        MOS_LINUX_BO *bo = resource->bo;
        GMM_RESOURCE_FLAG GmmFlags = resource->pGmmResInfo->GetResFlags();

        // Do decompression for a compressed surface before lock
        if (!flags->NoDecompress &&
            (((GmmFlags.Gpu.MMC || GmmFlags.Gpu.CCS) && GmmFlags.Info.MediaCompressed) ||
            resource->pGmmResInfo->IsMediaMemoryCompressed(0)))
        {            
            MosDecompression   *mosDecompression = nullptr;
            MOS_STATUS status = MosInterface::GetMosDecompressionFromStreamState(streamState, mosDecompression);
            if (status != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Get Mos Decompression From StreamState failed, skip lock");
                return nullptr;
            }
            if (nullptr == mosDecompression)
            {
                MOS_OS_ASSERTMESSAGE("mosDecompression is NULL.");
                return nullptr;
            }
            mosDecompression->MemoryDecompress(resource);
        }

        if (false == resource->bMapped)
        {
            if (perStreamParameters->bIsAtomSOC)
            {
                mos_bo_map_gtt(bo);
            }
            else
            {
                if (resource->TileType != MOS_TILE_LINEAR && !flags->TiledAsTiled)
                {
                    if (perStreamParameters->bUseSwSwizzling)
                    {
                        mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY & flags->WriteOnly));
                        resource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
                        if (resource->pSystemShadow == nullptr)
                        {
                            resource->pSystemShadow = (uint8_t *)MOS_AllocMemory(bo->size);
                            MOS_OS_CHECK_CONDITION((resource->pSystemShadow == nullptr), "Failed to allocate shadow surface", nullptr);
                        }
                        if (resource->pSystemShadow)
                        {
                            int32_t swizzleflags = perStreamParameters->bTileYFlag ? 0 : 1;
                            MOS_OS_CHECK_CONDITION((resource->TileType != MOS_TILE_Y), "Unsupported tile type", nullptr);
                            MOS_OS_CHECK_CONDITION((bo->size <= 0 || resource->iPitch <= 0), "Invalid BO size or pitch", nullptr);
                            MosUtilities::MosSwizzleData((uint8_t *)bo->virt, resource->pSystemShadow, MOS_TILE_Y, MOS_TILE_LINEAR, bo->size / resource->iPitch, resource->iPitch, swizzleflags);
                        }
                    }
                    else
                    {
                        mos_bo_map_gtt(bo);
                        resource->MmapOperation = MOS_MMAP_OPERATION_MMAP_GTT;
                    }
                }
                else if (flags->Uncached)
                {
                    mos_bo_map_wc(bo);
                    resource->MmapOperation = MOS_MMAP_OPERATION_MMAP_WC;
                }
                else
                {
                    mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY & flags->WriteOnly));
                    resource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
                }
            }
            resource->pData   = resource->pSystemShadow ? resource->pSystemShadow : (uint8_t *)bo->virt;
            resource->bMapped = true;
        }

        pData = resource->pData;
    }

    MOS_OS_ASSERT(pData);
    return pData;
}

MOS_STATUS GraphicsResourceSpecificNext::UnlockExternalResource(
    MOS_STREAM_HANDLE   streamState,
    MOS_RESOURCE_HANDLE resource)
{
    MOS_OS_FUNCTION_ENTER;
    
    MOS_OS_CHK_NULL_RETURN(resource);
    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);
    
    MOS_OS_CHK_NULL_RETURN(streamState->perStreamParameters);
    auto perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    if (resource->bo)
    {
        if (true == resource->bMapped)
        {
            if (perStreamParameters->bIsAtomSOC)
            {
                mos_bo_unmap_gtt(resource->bo);
            }
            else
            {
                if (resource->pSystemShadow)
                {
                    int32_t flags = perStreamParameters->bTileYFlag ? 0 : 1;
                    MosUtilities::MosSwizzleData(resource->pSystemShadow, (uint8_t *)resource->bo->virt, MOS_TILE_LINEAR, MOS_TILE_Y, resource->bo->size / resource->iPitch, resource->iPitch, flags);
                    MOS_FreeMemory(resource->pSystemShadow);
                    resource->pSystemShadow = nullptr;
                }

                switch (resource->MmapOperation)
                {
                case MOS_MMAP_OPERATION_MMAP_GTT:
                    mos_bo_unmap_gtt(resource->bo);
                    break;
                case MOS_MMAP_OPERATION_MMAP_WC:
                    mos_bo_unmap_wc(resource->bo);
                    break;
                case MOS_MMAP_OPERATION_MMAP:
                    mos_bo_unmap(resource->bo);
                    break;
                default:
                    MOS_OS_ASSERTMESSAGE("Invalid mmap operation type");
                    break;
                }
            }
            resource->bo->virt = nullptr;
            resource->bMapped  = false;
        }
        resource->pData = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GraphicsResourceSpecificNext::SetTileModebyForce(GMM_RESCREATE_PARAMS &gmmParams, MOS_TILE_MODE_GMM tileMode)
{
    if (tileMode == MOS_TILE_64_GMM)
    {
        gmmParams.Flags.Info.Tile64 = true;
    }
    else if (tileMode == MOS_TILE_4_GMM)
    {
        gmmParams.Flags.Info.Tile4 = true;
    }
    return MOS_STATUS_SUCCESS;
}
