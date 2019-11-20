/*
* Copyright (c) 2013-2017, Intel Corporation
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
//! \file     codechal_memdecomp.cpp
//! \brief    This module sets up a kernel for media memory decompression.

#include "codechal_memdecomp.h"
#include "codeckrnheader.h"

//!
//! \class MediaObjectCopyCurbe
//! \brief Media object memory decompress copy knernel curbe.
//!        Note: Cube data DW0-6 must be defined at the begining of the class.
//!
class MediaObjectCopyCurbe
{
public:
    // DW 0
    union
    {
        struct
        {
            uint32_t srcSurface0Index;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw0;

    // DW 1
    union
    {
        struct
        {
            uint32_t srcSurface1Index;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw1;

    // DW 2
    union
    {
        struct
        {
            uint32_t srcSurface2Index;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw2;

    // DW 3
    union
    {
        struct
        {
            uint32_t dstSurface0Index;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw3;

    // DW 4
    union
    {
        struct
        {
            uint32_t dstSurface1Index;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw4;

    // DW 5
    union
    {
        struct
        {
            uint32_t dstSurface2Index;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw5;

    // DW 6
    union
    {
        struct
        {
            uint32_t surfaceWidth;
        };
        struct
        {
            uint32_t value;
        };
    } m_dw6;

    //!
    //! \brief    Constructor
    //!
    MediaObjectCopyCurbe();

    //!
    //! \brief    Destructor
    //!
    ~MediaObjectCopyCurbe(){};

    static const size_t m_byteSize = 28; //!< Byte size of cube data DW0-6.
} ;

MediaObjectCopyCurbe::MediaObjectCopyCurbe()
{
    MOS_ZeroMemory(this, m_byteSize);
}

MediaMemDecompState::~MediaMemDecompState()
{
    MHW_FUNCTION_ENTER;

    Delete_MhwCpInterface(m_cpInterface); 
    m_cpInterface = nullptr;

    if (m_cmdBufIdGlobal)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resCmdBufIdGlobal);
        m_osInterface->pfnFreeResource(m_osInterface, &m_resCmdBufIdGlobal);
        m_cmdBufIdGlobal = nullptr;
    }

    if (m_miInterface)
    {
        MOS_Delete(m_miInterface);
        m_miInterface = nullptr;
    }

    if (m_renderInterface)
    {
        MOS_Delete(m_renderInterface);
        m_renderInterface = nullptr;
    }

    if (m_osInterface)
    {
        m_osInterface->pfnDestroy(m_osInterface, false);
        MOS_FreeMemory(m_osInterface);
        m_osInterface = nullptr;
    }
}

MediaMemDecompState::MediaMemDecompState() :
    MediaMemDecompBaseState(),
    m_currCmdBufId(0)
{
    MHW_FUNCTION_ENTER;
    m_stateHeapSettings.m_ishBehavior = HeapManager::Behavior::clientControlled;
    m_stateHeapSettings.m_dshBehavior = HeapManager::Behavior::destructiveExtend;
    m_stateHeapSettings.m_keepDshLocked = true;
    m_stateHeapSettings.dwDshIncrement = 2 * MOS_PAGE_SIZE;

    MOS_ZeroMemory(&m_renderContext, sizeof(m_renderContext));
    MOS_ZeroMemory(&m_krnUniId, sizeof(m_krnUniId));
    MOS_ZeroMemory(&m_kernelSize, sizeof(m_kernelSize));
    MOS_ZeroMemory(&m_resCmdBufIdGlobal, sizeof(m_resCmdBufIdGlobal));

    for (uint8_t idx = decompKernelStatePa; idx < decompKernelStateMax; idx++)
    {
        m_kernelBinary[idx] = nullptr;
        m_kernelStates[idx] = MHW_KERNEL_STATE();
    }

     m_krnUniId[decompKernelStatePa] = IDR_CODEC_ALLPACopy;
     m_krnUniId[decompKernelStatePl2] = IDR_CODEC_ALLPL2Copy;

}

MOS_STATUS MediaMemDecompState::GetKernelBinaryAndSize(
    uint8_t  *kernelBase,
    uint32_t krnUniId,
    uint8_t  **kernelBinary,
    uint32_t *kernelSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL_RETURN(kernelBase);
    MHW_CHK_NULL_RETURN(kernelBinary);
    MHW_CHK_NULL_RETURN(kernelSize);

    if (krnUniId >= IDR_CODEC_TOTAL_NUM_KERNELS)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    uint32_t *kernelOffsetTable = (uint32_t*)kernelBase;
    uint8_t  *base              = (uint8_t*)(kernelOffsetTable + IDR_CODEC_TOTAL_NUM_KERNELS + 1);

    *kernelSize =
        kernelOffsetTable[krnUniId + 1] -
        kernelOffsetTable[krnUniId];
    *kernelBinary =
        ((*kernelSize) > 0) ? (base + kernelOffsetTable[krnUniId]) : nullptr;

    return eStatus;
}

MOS_STATUS MediaMemDecompState::InitKernelState(
    uint32_t                 kernelStateIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (kernelStateIdx >= decompKernelStateMax)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    uint8_t **kernelBase  = &m_kernelBinary[kernelStateIdx];
    uint32_t *kernelSize = &m_kernelSize[kernelStateIdx];

    MHW_CHK_STATUS_RETURN(GetKernelBinaryAndSize(
        m_kernelBase,
        m_krnUniId[kernelStateIdx],
        kernelBase,
        kernelSize));

    m_stateHeapSettings.dwIshSize +=
        MOS_ALIGN_CEIL(*kernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));
    m_stateHeapSettings.dwDshSize += MHW_CACHELINE_SIZE* m_numMemDecompSyncTags;
    m_stateHeapSettings.dwNumSyncTags += m_numMemDecompSyncTags;

    return eStatus;
}

MOS_STATUS MediaMemDecompState::MemoryDecompress(
    PMOS_RESOURCE targetResource)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL_RETURN(targetResource);

    MOS_SURFACE targetSurface;
    MOS_ZeroMemory(&targetSurface, sizeof(MOS_SURFACE));
    targetSurface.Format     = Format_Invalid;
    targetSurface.OsResource = *targetResource;
    MHW_CHK_STATUS_RETURN(GetResourceInfo(&targetSurface));

    //Set context before proceeding
    auto gpuContext = m_osInterface->CurrentGpuContextOrdinal;
    m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
    m_osInterface->pfnResetOsStates(m_osInterface);

    DecompKernelStateIdx kernelStateIdx;
    bool                 useUVPlane;
    if ((targetSurface.Format == Format_YUY2) || (targetSurface.Format == Format_UYVY))
    {
        kernelStateIdx = decompKernelStatePa;
        useUVPlane     = false;
    }
    else if ((targetSurface.Format == Format_NV12) || (targetSurface.Format == Format_P010))
    {
        kernelStateIdx = decompKernelStatePl2;
        useUVPlane     = true;
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    auto kernelState = &m_kernelStates[kernelStateIdx];
    kernelState->m_currTrackerId = m_currCmdBufId;

    // preprocess in cp first
    m_osInterface->osCpInterface->PrepareResources((void **)&targetResource, 1, nullptr, 0);

    if (kernelStateIdx == decompKernelStatePl2)
    {
        if (m_osInterface->osCpInterface->IsSMEnabled())
        {
            uint32_t *kernelBase = nullptr;
            uint32_t  kernelSize = 0;
            m_osInterface->osCpInterface->GetTK(
                &kernelBase,
                &kernelSize,
                nullptr);
            if (nullptr == kernelBase || 0 == kernelSize)
            {
                MHW_ASSERT("Could not get TK kernels for MMC!");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }

            kernelState->KernelParams.pBinary = (uint8_t *)kernelBase;
        }
        else
        {
            kernelState->KernelParams.pBinary = m_kernelBinary[kernelStateIdx];
        }
        MHW_CHK_STATUS_RETURN(kernelState->m_ishRegion.AddData(
            kernelState->KernelParams.pBinary,
            0,
            kernelState->KernelParams.iSize));
    }

    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        m_stateHeapInterface,
        kernelState->KernelParams.iBTCount));

    uint32_t dshSize = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() +
        MOS_ALIGN_CEIL(kernelState->KernelParams.iCurbeLength,
        m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());

    eStatus = m_stateHeapInterface->pfnAssignSpaceInStateHeap(
        m_stateHeapInterface,
        MHW_DSH_TYPE,
        kernelState,
        dshSize,
        false,
        true);

    if (eStatus == MOS_STATUS_CLIENT_AR_NO_SPACE)
    {
        MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnAssignSpaceInStateHeap(
            m_stateHeapInterface,
            MHW_DSH_TYPE,
            kernelState,
            dshSize,
            false,
            true));
    }
    else if (eStatus != MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }

    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnAssignSpaceInStateHeap(
        m_stateHeapInterface,
        MHW_SSH_TYPE,
        kernelState,
        kernelState->dwSshSize,
        false,
        false));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    MHW_CHK_STATUS_RETURN(SetMediaObjectCopyCurbe(kernelStateIdx));

    MOS_COMMAND_BUFFER cmdBuffer;
    // Send HW commands (including SSH)
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface        = m_osInterface;
    genericPrologParams.pvMiInterface       = m_miInterface;
    genericPrologParams.bMmcEnabled         = true;
    MHW_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(&cmdBuffer, &genericPrologParams));

    MHW_CHK_NULL_RETURN(m_renderInterface);
    if (m_renderInterface->GetL3CacheConfig()->bL3CachingEnabled)
    {
        MHW_CHK_STATUS_RETURN(m_renderInterface->SetL3Cache(&cmdBuffer));
    }

    MHW_CHK_STATUS_RETURN(m_renderInterface->EnablePreemption(&cmdBuffer));

    MHW_CHK_STATUS_RETURN(m_renderInterface->AddPipelineSelectCmd(&cmdBuffer, false));

    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    MHW_RCS_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.dwNumPlanes = useUVPlane ? 2 : 1;  // Y+UV : Y
    surfaceParams.psSurface   = &targetSurface;
    // Y Plane
    surfaceParams.dwBindingTableOffset[MHW_Y_PLANE] = copySurfaceSrcY;

    if (surfaceParams.psSurface->Format == Format_YUY2)
    {
        surfaceParams.ForceSurfaceFormat[MHW_Y_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_NORMAL;
    }
    else if (surfaceParams.psSurface->Format == Format_UYVY)
    {
        surfaceParams.ForceSurfaceFormat[MHW_Y_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPY;
    }
    else if (surfaceParams.psSurface->Format == Format_P010)
    {
        surfaceParams.ForceSurfaceFormat[MHW_Y_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM;
    }
    else  //NV12
    {
        surfaceParams.ForceSurfaceFormat[MHW_Y_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM;
    }

    uint32_t widthInBytes = GetSurfaceWidthInBytes(surfaceParams.psSurface);
    surfaceParams.dwWidthToUse[MHW_Y_PLANE] = WIDTH_IN_DW(widthInBytes);

    // UV Plane
    if (useUVPlane)
    {
        surfaceParams.dwBindingTableOffset[MHW_U_PLANE] = copySurfaceSrcU;
        if (surfaceParams.psSurface->Format == Format_P010)
        {
            surfaceParams.ForceSurfaceFormat[MHW_U_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUVY;
        }
        else  //NV12
        {
            surfaceParams.ForceSurfaceFormat[MHW_U_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT;
        }
        surfaceParams.dwBaseAddrOffset[MHW_U_PLANE] =
            targetSurface.dwPitch *
            MOS_ALIGN_FLOOR(targetSurface.UPlaneOffset.iYOffset, MOS_YTILE_H_ALIGNMENT);
        surfaceParams.dwWidthToUse[MHW_U_PLANE]  = WIDTH_IN_DW(widthInBytes);
        surfaceParams.dwHeightToUse[MHW_U_PLANE] = surfaceParams.psSurface->dwHeight / 2;
        surfaceParams.dwYOffset[MHW_U_PLANE] =
            (targetSurface.UPlaneOffset.iYOffset % MOS_YTILE_H_ALIGNMENT);
    }
    m_osInterface->pfnGetMemoryCompressionMode(
        m_osInterface, &targetSurface.OsResource, (PMOS_MEMCOMP_STATE)&surfaceParams.psSurface->CompressionMode);
    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetSurfaceState(
        m_stateHeapInterface,
        kernelState,
        &cmdBuffer,
        1,
        &surfaceParams));

    //In place decompression: src shares the same surface with dst.
    surfaceParams.bIsWritable                       = true;
    surfaceParams.dwBindingTableOffset[MHW_Y_PLANE] = copySurfaceDstY;
    if (useUVPlane)
    {
        surfaceParams.dwBindingTableOffset[MHW_U_PLANE] = copySurfaceDstU;
    }
    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetSurfaceState(
        m_stateHeapInterface,
        kernelState,
        &cmdBuffer,
        1,
        &surfaceParams));

    MHW_STATE_BASE_ADDR_PARAMS stateBaseAddrParams;
    MOS_ZeroMemory(&stateBaseAddrParams, sizeof(stateBaseAddrParams));
    MOS_RESOURCE *dsh = nullptr, *ish = nullptr;
    MHW_CHK_NULL_RETURN(dsh = kernelState->m_dshRegion.GetResource());
    MHW_CHK_NULL_RETURN(ish = kernelState->m_ishRegion.GetResource());
    stateBaseAddrParams.presDynamicState = dsh;
    stateBaseAddrParams.dwDynamicStateSize = kernelState->m_dshRegion.GetHeapSize();
    stateBaseAddrParams.presInstructionBuffer = ish;
    stateBaseAddrParams.dwInstructionBufferSize = kernelState->m_ishRegion.GetHeapSize();
    MHW_CHK_STATUS_RETURN(m_renderInterface->AddStateBaseAddrCmd(
        &cmdBuffer,
        &stateBaseAddrParams));

    MHW_VFE_PARAMS vfeParams = {};
    vfeParams.pKernelState = kernelState;
    auto waTable          = m_osInterface->pfnGetWaTable(m_osInterface);

    vfeParams.eVfeSliceDisable = MHW_VFE_SLICE_ALL;

    MHW_CHK_STATUS_RETURN(m_renderInterface->AddMediaVfeCmd(
        &cmdBuffer,
        &vfeParams));

    MHW_CURBE_LOAD_PARAMS curbeLoadParams;
    MOS_ZeroMemory(&curbeLoadParams, sizeof(curbeLoadParams));
    curbeLoadParams.pKernelState = kernelState;
    MHW_CHK_STATUS_RETURN(m_renderInterface->AddMediaCurbeLoadCmd(
        &cmdBuffer,
        &curbeLoadParams));

    MHW_ID_LOAD_PARAMS idLoadParams;
    MOS_ZeroMemory(&idLoadParams, sizeof(idLoadParams));
    idLoadParams.pKernelState = kernelState;
    idLoadParams.dwNumKernelsLoaded = 1;
    MHW_CHK_STATUS_RETURN(m_renderInterface->AddMediaIDLoadCmd(
        &cmdBuffer,
        &idLoadParams));

    uint32_t resolutionX;
    if (kernelStateIdx == decompKernelStatePa)  // Format_YUY2, Format_UYVY
    {
        resolutionX = MOS_ROUNDUP_DIVIDE(targetSurface.dwWidth * 2, 32);
    }
    else  // DecompKernelStatePl2: Format_NV12, Format_P010
    {
        if (targetSurface.Format == Format_P010)  // Format_P010
        {
            resolutionX = MOS_ROUNDUP_DIVIDE(targetSurface.dwWidth * 2, 32);
        }
        else  // Format_NV12
        {
            resolutionX = MOS_ROUNDUP_DIVIDE(targetSurface.dwWidth, 32);
        }
    }
    uint32_t resolutionY = MOS_ROUNDUP_DIVIDE(targetSurface.dwHeight, 16);

    MHW_WALKER_PARAMS walkerParams;
    MOS_ZeroMemory(&walkerParams, sizeof(walkerParams));
    walkerParams.WalkerMode               = MHW_WALKER_MODE_SINGLE;
    walkerParams.BlockResolution.x        = resolutionX;
    walkerParams.BlockResolution.y        = resolutionY;
    walkerParams.GlobalResolution.x       = resolutionX;
    walkerParams.GlobalResolution.y       = resolutionY;
    walkerParams.GlobalOutlerLoopStride.x = resolutionX;
    walkerParams.GlobalOutlerLoopStride.y = 0;
    walkerParams.GlobalInnerLoopUnit.x    = 0;
    walkerParams.GlobalInnerLoopUnit.y    = resolutionY;
    walkerParams.dwLocalLoopExecCount     = 0xFFFF;  //MAX VALUE
    walkerParams.dwGlobalLoopExecCount    = 0xFFFF;  //MAX VALUE

    // No dependency
    walkerParams.ScoreboardMask = 0;
    // Raster scan walking pattern
    walkerParams.LocalOutLoopStride.x = 0;
    walkerParams.LocalOutLoopStride.y = 1;
    walkerParams.LocalInnerLoopUnit.x = 1;
    walkerParams.LocalInnerLoopUnit.y = 0;
    walkerParams.LocalEnd.x           = resolutionX - 1;
    walkerParams.LocalEnd.y           = 0;

    MHW_CHK_STATUS_RETURN(m_renderInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    // Check if destination surface needs to be synchronized, before command buffer submission
    MOS_SYNC_PARAMS    syncParams;
    MOS_ZeroMemory(&syncParams, sizeof(syncParams));
    syncParams.uiSemaphoreCount         = 1;
    syncParams.GpuContext               = m_renderContext;
    syncParams.presSyncResource         = &targetSurface.OsResource;
    syncParams.bReadOnly                = false;
    syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    MHW_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync)
    {
        MHW_PIPE_CONTROL_PARAMS pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));

        pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        MHW_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(
            &cmdBuffer,
            nullptr,
            &pipeControlParams));

        MHW_CHK_STATUS_RETURN(WriteSyncTagToResourceCmd(&cmdBuffer));
    }

    MHW_MI_STORE_DATA_PARAMS        miStoreDataParams;
    MOS_ZeroMemory(&miStoreDataParams, sizeof(miStoreDataParams));
    miStoreDataParams.pOsResource = &m_resCmdBufIdGlobal;
    miStoreDataParams.dwValue = m_currCmdBufId;
    MHW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        &cmdBuffer,
        &miStoreDataParams));

    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
        m_stateHeapInterface));

    // Add PipeControl to invalidate ISP and MediaState to avoid PageFault issue
    // This code is temporal and it will be moved to batch buffer end in short
    PLATFORM platform;
    m_osInterface->pfnGetPlatform(m_osInterface, &platform);
    if (GFX_IS_GEN_9_OR_LATER(platform))
    {
        MHW_PIPE_CONTROL_PARAMS pipeControlParams;

        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        pipeControlParams.bGenericMediaStateClear = true;
        pipeControlParams.bIndirectStatePointersDisable = true;
        pipeControlParams.bDisableCSStall = false;
        MHW_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(&cmdBuffer, NULL, &pipeControlParams));

        if (MEDIA_IS_WA(m_osInterface->pfnGetWaTable(m_osInterface), WaSendDummyVFEafterPipelineSelect))
        {
            MHW_VFE_PARAMS vfeStateParams = {};
            vfeStateParams.dwNumberofURBEntries = 1;
            MHW_CHK_STATUS_RETURN(m_renderInterface->AddMediaVfeCmd(&cmdBuffer, &vfeStateParams));
        }
    }

    MHW_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    MHW_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_renderContextUsesNullHw));

    // Update the compression mode
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnSetMemoryCompressionMode(
        m_osInterface,
        targetResource,
        MOS_MEMCOMP_DISABLED));
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnSetMemoryCompressionHint(
        m_osInterface,
        targetResource,
        false));

    //Update CmdBufId...
    m_currCmdBufId++;
    if (m_currCmdBufId == MemoryBlock::m_invalidTrackerId)
    {
        m_currCmdBufId++;
    }

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

    if (gpuContext != m_renderContext)
    {
        m_osInterface->pfnSetGpuContext(m_osInterface, gpuContext);
    }

    return eStatus;
}

MOS_STATUS MediaMemDecompState::GetResourceInfo(
    PMOS_SURFACE   surface)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL_RETURN(m_osInterface);
    MHW_CHK_NULL_RETURN(surface);

    MOS_SURFACE details;
    MOS_ZeroMemory(&details, sizeof(details));
    details.Format = Format_Invalid;

    MHW_CHK_STATUS_RETURN(m_osInterface->pfnGetResourceInfo(
        m_osInterface,
        &surface->OsResource,
        &details));

    surface->Format                      = details.Format;
    surface->dwWidth                     = details.dwWidth;
    surface->dwHeight                    = details.dwHeight;
    surface->dwPitch                     = details.dwPitch;
    surface->dwDepth                     = details.dwDepth;
    surface->bArraySpacing               = details.bArraySpacing;
    surface->TileType                    = details.TileType;
    surface->TileModeGMM                 = details.TileModeGMM;
    surface->bGMMTileEnabled             = details.bGMMTileEnabled;
    surface->dwOffset                    = details.RenderOffset.YUV.Y.BaseOffset;
    surface->UPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.U.BaseOffset;
    surface->UPlaneOffset.iXOffset       = details.RenderOffset.YUV.U.XOffset;
    surface->UPlaneOffset.iYOffset =
        (surface->UPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
        details.RenderOffset.YUV.U.YOffset;
    surface->VPlaneOffset.iSurfaceOffset = details.RenderOffset.YUV.V.BaseOffset;
    surface->VPlaneOffset.iXOffset       = details.RenderOffset.YUV.V.XOffset;
    surface->VPlaneOffset.iYOffset =
        (surface->VPlaneOffset.iSurfaceOffset - surface->dwOffset) / surface->dwPitch +
        details.RenderOffset.YUV.V.YOffset;
    surface->bCompressible   = details.bCompressible;
    surface->bIsCompressed   = details.bIsCompressed;
    surface->CompressionMode = details.CompressionMode;

    return eStatus;
}

uint32_t MediaMemDecompState::GetSurfaceWidthInBytes(PMOS_SURFACE surface)
{
    uint32_t widthInBytes;

    switch (surface->Format)
    {
    case Format_IMC1:
    case Format_IMC3:
    case Format_IMC2:
    case Format_IMC4:
    case Format_NV12:
    case Format_YV12:
    case Format_I420:
    case Format_IYUV:
    case Format_400P:
    case Format_411P:
    case Format_422H:
    case Format_422V:
    case Format_444P:
    case Format_RGBP:
    case Format_BGRP:
        widthInBytes = surface->dwWidth;
        break;
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_P010:
        widthInBytes = surface->dwWidth << 1;
        break;
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
        widthInBytes = surface->dwWidth << 2;
        break;
    default:
        widthInBytes = surface->dwWidth;
        break;
    }

    return widthInBytes;
}

MOS_STATUS MediaMemDecompState::WriteSyncTagToResourceCmd(
    PMOS_COMMAND_BUFFER   cmdBuffer)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MOS_RESOURCE globalGpuContextSyncTagBuffer;
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
        m_osInterface,
        &globalGpuContextSyncTagBuffer));

    uint32_t offset = m_osInterface->pfnGetGpuStatusTagOffset(
        m_osInterface,
        m_osInterface->CurrentGpuContextOrdinal);
    uint32_t value  = m_osInterface->pfnGetGpuStatusTag(
        m_osInterface,
        m_osInterface->CurrentGpuContextOrdinal);

    MHW_MI_STORE_DATA_PARAMS params;
    params.pOsResource      = &globalGpuContextSyncTagBuffer;
    params.dwResourceOffset = offset;
    params.dwValue          = value;

    MHW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &params));

    // Increment GPU Context Tag for next use
    m_osInterface->pfnIncrementGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);

    return eStatus;
}

MOS_STATUS MediaMemDecompState::SetMediaObjectCopyCurbe(
    DecompKernelStateIdx kernelStateIdx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if ((kernelStateIdx >= decompKernelStateMax))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MediaObjectCopyCurbe cmd;

    cmd.m_dw0.srcSurface0Index = copySurfaceSrcY;
    cmd.m_dw3.dstSurface0Index = copySurfaceDstY;

    if (kernelStateIdx == decompKernelStatePl2)
    {
        cmd.m_dw1.srcSurface1Index = copySurfaceSrcU;
        cmd.m_dw4.dstSurface1Index = copySurfaceDstU;
    }

    MHW_CHK_STATUS_RETURN(m_kernelStates[kernelStateIdx].m_dshRegion.AddData(
        &cmd,
        m_kernelStates[kernelStateIdx].dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MediaMemDecompState::SetKernelStateParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL_RETURN(m_renderInterface->GetHwCaps());

    for (uint32_t krnIdx = 0; krnIdx < decompKernelStateMax; krnIdx++)
    {
        auto kernelState = &m_kernelStates[krnIdx];
        kernelState->KernelParams.pBinary = m_kernelBinary[krnIdx];
        kernelState->KernelParams.iSize   = m_kernelSize[krnIdx];
        kernelState->KernelParams.iBTCount     = copySurfaceNum;
        kernelState->KernelParams.iThreadCount = m_renderInterface->GetHwCaps()->dwMaxThreads;
        kernelState->KernelParams.iCurbeLength = MOS_ALIGN_CEIL(
            MediaObjectCopyCurbe::m_byteSize,
            m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        kernelState->KernelParams.iBlockWidth  = 32;
        kernelState->KernelParams.iBlockHeight = 16;
        kernelState->KernelParams.iIdCount     = 1;

        kernelState->dwCurbeOffset =
            m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();

        MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelState->KernelParams.iBTCount,
            &kernelState->dwSshSize,
            &kernelState->dwBindingTableSize));

        kernelState->dwKernelBinaryOffset = 0;

        eStatus = m_stateHeapInterface->pfnAssignSpaceInStateHeap(
            m_stateHeapInterface,
            MHW_ISH_TYPE,
            kernelState,
            kernelState->KernelParams.iSize,
            true,
            false);

        if (eStatus == MOS_STATUS_CLIENT_AR_NO_SPACE)
        {
            MHW_ASSERTMESSAGE("CodecHal does not handle this case");
            return eStatus;
        }
        else if (eStatus != MOS_STATUS_SUCCESS)
        {
            return eStatus;
        }

        MHW_CHK_STATUS_RETURN(kernelState->m_ishRegion.AddData(
            kernelState->KernelParams.pBinary,
            0,
            kernelState->KernelParams.iSize));
    }

    return eStatus;
}

MOS_STATUS MediaMemDecompState::Initialize(
    PMOS_INTERFACE                  osInterface,
    MhwCpInterface                  *cpInterface,
    MhwMiInterface                  *miInterface,
    MhwRenderInterface              *renderInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL_RETURN(osInterface);
    MHW_CHK_NULL_RETURN(cpInterface);
    MHW_CHK_NULL_RETURN(miInterface);
    MHW_CHK_NULL_RETURN(renderInterface);

    m_osInterface = osInterface;
    m_cpInterface = cpInterface;
    m_miInterface = miInterface;
    m_renderInterface = renderInterface;

    for (uint8_t kernelIdx = decompKernelStatePa; kernelIdx < decompKernelStateMax; kernelIdx++)
    {
        MHW_CHK_STATUS_RETURN(InitKernelState(kernelIdx));
    }

    if (m_stateHeapSettings.dwIshSize > 0 &&
        m_stateHeapSettings.dwDshSize > 0 &&
        m_stateHeapSettings.dwNumSyncTags > 0)
    {
        MHW_CHK_STATUS_RETURN(m_renderInterface->AllocateHeaps(
            m_stateHeapSettings));
    }

    m_stateHeapInterface = m_renderInterface->m_stateHeapInterface;
    MHW_CHK_NULL_RETURN(m_stateHeapInterface);

    if (m_osInterface->pfnIsGpuContextValid(m_osInterface, MOS_GPU_CONTEXT_RENDER) == MOS_STATUS_SUCCESS)
    {
        m_renderContext = MOS_GPU_CONTEXT_RENDER;
    }
    else
    {
        MOS_GPUCTX_CREATOPTIONS createOption;
        MHW_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_RENDER,
            MOS_GPU_NODE_3D,
            &createOption));

        m_renderContext = MOS_GPU_CONTEXT_RENDER;
    }

    MOS_NULL_RENDERING_FLAGS nullHWAccelerationEnable;
    nullHWAccelerationEnable.Value = 0;
    m_disableDecodeSyncLock        = false;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE_ID,
        &userFeatureData);
    nullHWAccelerationEnable.Value = userFeatureData.u32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DECODE_LOCK_DISABLE_ID,
        &userFeatureData);
    m_disableDecodeSyncLock = userFeatureData.u32Data ? true : false;
#endif  // _DEBUG || _RELEASE_INTERNAL

    m_disableLockForTranscode =
        MEDIA_IS_WA(m_osInterface->pfnGetWaTable(m_osInterface), WaDisableLockForTranscodePerf);

    MHW_CHK_STATUS_RETURN(SetKernelStateParams());

    m_renderContextUsesNullHw =
        ((m_renderContext == MOS_GPU_CONTEXT_RENDER) ? nullHWAccelerationEnable.CtxRender : nullHWAccelerationEnable.CtxRender2) ||
        nullHWAccelerationEnable.Mmc;

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(allocParams));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = MHW_CACHELINE_SIZE;
    allocParams.pBufName = "CmdBufIdGlobal";
    MHW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        &m_resCmdBufIdGlobal));
    m_currCmdBufId = MemoryBlock::m_invalidTrackerId + 1;

    MOS_LOCK_PARAMS lockParams;
    MOS_ZeroMemory(&lockParams, sizeof(lockParams));
    lockParams.WriteOnly = 1;
    m_cmdBufIdGlobal = (uint32_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_resCmdBufIdGlobal,
        &lockParams);
    MHW_CHK_NULL_RETURN(m_cmdBufIdGlobal);
    MOS_ZeroMemory(m_cmdBufIdGlobal, allocParams.dwBytes);

    MHW_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetCmdBufStatusPtr(
        m_stateHeapInterface,
        m_cmdBufIdGlobal));

    return eStatus;
}
