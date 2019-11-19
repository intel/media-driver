/*
* Copyright (c) 2019, Intel Corporation
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
//! \file    mos_graphicsresource_next.h
//! \brief   Container class for the basic gpu resource
//!

#ifndef __MOS_GRAPHICS_RESOURCE_NEXT_H__
#define __MOS_GRAPHICS_RESOURCE_NEXT_H__

#include "mos_os_next.h"
#include "mos_resource_defs.h"
#include <string>
#include "mos_context_next.h"

class GraphicsResourceNext
{
public:
    enum ResourceType
    {
        undefinedResource    = 0,    //!< Undefined Resource Type
        osSpecificResource   = 1,    //!< Os Specific Resource Type
        soloResource         = 2,    //!< MediaSolo based Resource Type
        invalidResource      = 0xff  //!< Ceil Guard for Resource Type
    };

    constexpr static uint32_t  m_maxBufNameLength = 256;

    //!
    //! \brief  Structure to Resource allocation parameters
    //!
    struct CreateParams
    {
    public:
        //!
        //! \brief  0,1: 1 element. >1: N elements
        //!
        uint32_t m_arraySize = 1;

        //!
        //! \brief   Compression mode.
        //!
        MOS_RESOURCE_MMC_MODE m_compressionMode = MOS_MMC_DISABLED;

        //!
        //! \brief   depth information.
        //!
        uint32_t m_depth     = 0;

        //!
        //! \brief   Pixel format.
        //!
        MOS_FORMAT m_format = Format_Invalid;

        //!
        //! \brief   Type == 2D || VOLUME, height in rows. Type == BUFFER, n/a
        //!
        uint32_t m_height   = 1;

        //!
        //! \brief   Resource is compressible or not.
        //!
        bool m_isCompressible = false;

        //!
        //! \brief   Optional parameter. Used to indicate that resource
        //!              can not be evicted
        //!
        bool m_isPersistent = false;

        //!
        //! \brief   Optional parameter only used in Linux. A string indicates the buffer
        //!              name and is used for debugging. NULL is OK.
        //!
        std::string m_name  = "";

        //!
        //! \brief   Optional parameter. If non null, TileType must be set to linear.
        //!
        void* m_pSystemMemory = nullptr;

        //!
        //! \brief  Whether SVM is in use.
        //!
        bool m_svm = false;

        //!
        //! \brief   Defines the layout of a physical page. Optimal choice depends on usage
        //!              model.
        //!
        MOS_TILE_TYPE m_tileType = MOS_TILE_INVALID;

        //!
        //! \brief   Basic resource geometry
        //!
        MOS_GFXRES_TYPE m_type = MOS_GFXRES_INVALID;

        //!
        //! \brief   Flags to describe attributes
        //!
        MOS_GFXRES_FLAGS m_flags = {};

        //!
        //! \brief   width in pixels
        //!
        uint32_t m_width = 0;

        //!
        //! \brief   Create the graphics buffer from a PMOS_ALLOC_GFXRES_PARAMS, for wrapper usage, to be deleted
        //!
        CreateParams(PMOS_ALLOC_GFXRES_PARAMS pParams)
        {
            m_arraySize       = pParams->dwArraySize;
            m_compressionMode = pParams->CompressionMode;
            m_depth           = pParams->dwDepth;
            m_format          = pParams->Format;
            m_height          = pParams->dwHeight;
            m_isCompressible  = (pParams->bIsCompressible == 1) ? true : false;
            m_isPersistent    = (pParams->bIsPersistent == 1) ? true : false;
            if (pParams->pBufName != nullptr)
            {
                 m_name = pParams->pBufName;
            }
            m_pSystemMemory   = pParams->pSystemMemory;
            m_tileType        = pParams->TileType;
            m_type            = pParams->Type;
            m_flags           = pParams->Flags;
            m_width           = pParams->dwWidth;
        };

        CreateParams()
        {
        };
    };

    //!
    //! \brief  Structure to Structure to Lock params
    //!
    struct LockParams
    {
    public:
        bool m_forceCached  = false;
        bool m_noDecompress = false;
        bool m_readRequest  = false;
        bool m_tileAsTiled  = false;
        bool m_uncached     = false;
        bool m_writeRequest = false;
        bool m_noOverWrite  = false;

        //!
        //! \brief   For wrapper usage, to be removed
        //!
        LockParams(PMOS_LOCK_PARAMS pLockFlags)
        {
            m_forceCached  = pLockFlags->ForceCached;
            m_noDecompress = pLockFlags->NoDecompress;
            m_readRequest  = pLockFlags->ReadOnly;
            m_tileAsTiled  = pLockFlags->TiledAsTiled;
            m_uncached     = pLockFlags->Uncached;
            m_writeRequest = pLockFlags->WriteOnly;
            m_noOverWrite  = pLockFlags->NoOverWrite;
        };

        LockParams()
        {
        };
    };

    //!
    //! \brief Structure to OS sync parameters
    //!
    struct SyncParams
    {
        MOS_GPU_CONTEXT         m_gpuContext      = {};             //!< GPU context you would like to signal on or wait in
        GraphicsResourceNext*   m_resSyncResource = nullptr;        //!< Has 2 meanings:
                                                                    //!< 1) a resource that requires sync, like a destination surface
                                                                    //!< 2) a resource used by HW/OS to sync between engines, like for MI_SEMAPHORE_MBOX
        uint32_t                m_semaphoreCount  = 0;              //!< Semaphore count
        uint32_t                m_semaphoreValue  = 0;              //!< Tag value in the case of resource tagging
        uint32_t                m_semaphoreOffset = 0;              //!< Offset into the sync resource for value read/write
        bool                    m_readOnly        = false;          //!< Marks the resource as read or write for future waits
        bool                    m_disableDecodeSyncLock   = false;  //!< Disable the lock function for decode.
        bool                    m_disableLockForTranscode = false;  //!< Disable the lock function for transcode perf.

        //!
        //! \brief   For wrapper usage, to be removed
        //!
        SyncParams(PMOS_SYNC_PARAMS pParams)
        {
            m_gpuContext              = pParams->GpuContext;
            m_resSyncResource         = pParams->presSyncResource->pGfxResourceNext;
            m_semaphoreCount          = pParams->uiSemaphoreCount;
            m_semaphoreValue          = pParams->uiSemaphoreValue;
            m_semaphoreOffset         = pParams->uiSemaphoreOffset;
            m_readOnly                = (pParams->bReadOnly == 1) ? true : false;
            m_disableDecodeSyncLock   = (pParams->bDisableDecodeSyncLock == 1) ? true : false;
            m_disableLockForTranscode = (pParams->bDisableLockForTranscode == 1) ? true : false;
        };

        SyncParams()
        {
        };
    } ;

protected:
    //!
    //! \brief  Constructor
    //! \return N/A
    //!
    GraphicsResourceNext();

public:
    //!
    //! \brief  Create Graphic resource based on specific resource type
    //! \param  [in] resourceType
    //!         Resource type, either os specific resoure or solo resource
    //! \return One specific Graphic Resource
    //!
    static class GraphicsResourceNext *CreateGraphicResource(ResourceType resourceType);

    //!
    //! \brief  Destructor
    //! \return N/A
    //!
    virtual ~GraphicsResourceNext();

    //!
    //! \brief  Get the size of the graphic resource
    //! \return buf size of the graphic resource
    //!
    uint32_t GetSize() { return m_size; };

    //!
    //! \brief  Get the locked address of resource
    //! \return address locked
    //!
    uint8_t* GetLockedAddr() {return m_pData; };

    //!
    //! \brief  Get allocation index of resource
    //! \param  [in] gpuContextHandle
    //!         Gpu context handle used to get the alloc index
    //! \return index got from current resource
    //!
    int32_t GetAllocationIndex(GPU_CONTEXT_HANDLE gpuContextHandle);

    //!
    //! \brief  Dump the content of the graphic resource into a specific file
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] overrideOffset
    //!         the offset inside the dump file
    //! \param  [in] overrideSize
    //!         the dump length
    //! \param  [in] outputFileName
    //!         the dump file name
    //! \param  [in] outputPath
    //!         the dump file path
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    MOS_STATUS Dump(OsContextNext* osContextPtr, uint32_t overrideOffset, uint32_t overrideSize, std::string outputFileName, std::string outputPath);

    //!
    //! \brief  Add a sync tag to the graphic resource
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] params
    //!         Parameters to do the synchronization
    //! \param  [in] streamIndex
    //!         Stream index to indicate which stream this resource belongs to
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    virtual MOS_STATUS SetSyncTag(OsContextNext* osContextPtr, SyncParams& params, uint32_t streamIndex) = 0;

    //!
    //! \brief  Check whether the resource is nullptr
    //! \param  [in] pResource
    //!         ptr to the graphics resource to be checked
    //! \return ture if the resource is nullptr, false on other cases
    //!
    virtual bool ResourceIsNull() = 0;

    //!
    //! \brief  Allocate the graphic memory to back up the graphic resource
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] params
    //!         Resource creation Params
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    virtual MOS_STATUS Allocate(OsContextNext* osContextPtr, CreateParams& params) = 0;

    //!
    //! \brief  Frees the specified resource with specific flag, if locked, unlocks it.
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] freeFlag
    //!         specific flag to free the resource
    //!
    virtual void Free(OsContextNext* osContextPtr, uint32_t  freeFlag = 0) = 0;

    //!
    //! \brief  check whether the current graphic resource is equal to another one
    //! \param  [in] toCompare
    //!         ptr to the graphics resource to be compared with 
    //! \return ture if equal, false if not
    //!
    virtual bool IsEqual(GraphicsResourceNext* toCompare) = 0;

    //!
    //! \brief  Check whether the current graphic resouce is valid
    //! \return Returns true if the two resources are equal and false otherwise.
    //!
    virtual bool IsValid() = 0;

    //!
    //! \brief  Locks a resource and returns a mapped system memory pointer.
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] params
    //!         Resource lock Params
    //! \return CPU side lock address in success case, nullptr in fail cases
    //!
    virtual void* Lock(OsContextNext* osContextPtr, LockParams& params) = 0;

    //!
    //! \brief  Unlocks a resource that has already been locked, if no lock has
    //!         occurred, this function does nothing
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    virtual MOS_STATUS Unlock(OsContextNext* osContextPtr) = 0;

    //!
    //! \brief  Converts an OS specific resource to a MOS resource.
    //! \param  [out] pMosResource
    //!         ptr to the MOS_RESOURCE as the convertion result
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    virtual MOS_STATUS ConvertToMosResource(MOS_RESOURCE* pMosResource) = 0;

    //!
    //! \brief  For MemNinja support.
    //! \return Global memory alloction counter
    //!
    static uint32_t GetMemAllocCounterGfx() { return m_memAllocCounterGfx; };

    //!
    //! \brief  For MemNinja support.
    //! \return Global memory alloction counter
    //!
    static void  SetMemAllocCounterGfx(uint32_t memAllocCounterGfx) { m_memAllocCounterGfx = memAllocCounterGfx; };

    //!
    //! \brief  Reset allocation index of all Gpu context in the resource
    //! \return void
    //!
    void ResetResourceAllocationIndex();

protected:
    //!
    //! \brief  Global graphic resouce counter
    //!
    static uint32_t m_memAllocCounterGfx;

    //!
    //! \brief  buf name of the graphic resource
    //!
    std::string m_name = "";

    //!
    //! \brief  CPU side lock address for the graphic resource
    //!
    uint8_t* m_pData = nullptr;

    //!
    //! \brief  0,1: 1 element. >1: N elements
    //!
    uint32_t m_arraySize = 0;

    //!
    //! \brief  Graphic resource compressiable or not
    //!
    bool m_compressible = false;

    //!
    //! \brief  compression mode for the Graphic resource
    //!
    MOS_RESOURCE_MMC_MODE m_compressionMode = MOS_MMC_DISABLED;

    //!MOS_MMC_DISABLED
    //! \brief  0: Implies 2D resource. >=1: volume resource
    //!
    uint32_t m_depth = 0;

    //!
    //! \brief  wheather the flip chain is in use
    //!
    bool m_flipChain = false;

    //!
    //! \brief  Pixel format
    //!
    MOS_FORMAT m_format = Format_Invalid;

    //!
    //! \brief  Type == 2D || VOLUME, height in rows. Type == BUFFER, n/a
    //!
    uint32_t m_height = 0;

    //!
    //! \brief  the Graphic resource is compressed or not
    //!
    bool m_isCompressed = false;

    //!
    //! \brief  the flag to indicate if overlay is in use
    //!
    bool m_overlay = false;

    //!
    //! \brief   < RenderPitch > pitch in bytes used for programming HW
    //!
    uint32_t m_pitch = 0;

    //!
    //! \brief   distance in rows between R-Planes used for programming HW
    //!
    uint32_t m_qPitch = 0;

    //!
    //! \brief   the active m_count for the graphic resource
    //!
    uint32_t m_count = 0;

    //!
    //! \brief   the active S3D_CHANNEL for the graphic resource
    //!
    MOS_S3D_CHANNEL m_s3dChannel = MOS_S3D_NONE;

    //!
    //! \brief   the size of the graphic resource
    //!
    uint32_t m_size = 0;

    //!
    //! \brief   Type == VOLUME, byte offset to next slice. Type != VOLUME, n/a
    //!
    uint32_t m_slicePitch = 0;

    //!
    //! \brief   Defines the layout of a physical page. Optimal choice depends on usage
    //!
    MOS_TILE_TYPE m_tileType = MOS_TILE_INVALID;

    //!
    //! \brief   Transparent GMM Tiletype specifying in hwcmd finally
    //!
    MOS_TILE_MODE_GMM   m_tileModeGMM = MOS_TILE_LINEAR_GMM;

    //!
    //! \brief   GMM defined Tile mode flag
    //!
    bool                m_isGMMTileEnabled = false;

    //!
    //! \brief   Basic resource geometry
    //!
    MOS_GFXRES_TYPE m_type = MOS_GFXRES_INVALID;

    //!
    //! \brief   U surface plane offset
    //!
    MOS_PLANE_OFFSET m_uPlaneOffset = {};

    //!
    //! \brief   V surface plane offset
    //!
    MOS_PLANE_OFFSET m_vPlaneOffset = {};

    //!
    //! \brief   Type == 2D || VOLUME, width in pixels.
    //!
    uint32_t m_width = 0;

    //!
    //! \brief   Y surface plane offset or RGB
    //!
    MOS_PLANE_OFFSET m_yPlaneOffset = {};

    //!
    //! \brief  This is used by MDF when a wrapper MOS Resource is used to 
    //!         set surface state for a given VA, not necessary from start,
    //!         in another actual MOS resource
    //!
    uint64_t                m_userProvidedVA = 0;

    //!
    //! \brief  Array of Gpu context and alloctaion index tuple.
    //!
    std::vector <std::tuple<GPU_CONTEXT_HANDLE, int32_t>>  m_allocationIndexArray;

    //! \brief   Mutex for allocation index array
    PMOS_MUTEX m_allocationIndexMutex = nullptr;
};
#endif // #ifndef __MOS_GRAPHICS_RESOURCE_NEXT_H__

