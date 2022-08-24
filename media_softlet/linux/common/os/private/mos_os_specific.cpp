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
//! \file      mos_os_specific.c
//! \brief     Common interface used in MOS LINUX OS
//! \details   Common interface used in MOS LINUX OS
//!

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_resource_defs.h"
#include <unistd.h>
#include <dlfcn.h>
#include "hwinfo_linux.h"
#include <stdlib.h>

#include "mos_context_specific.h"

#include "mos_graphicsresource_next.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_gpucontextmgr_next.h"
#include "mos_interface.h"

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"

#ifndef ANDROID
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#endif

#include "mos_os_virtualengine.h"
#include "mos_util_user_interface.h"

#include "mos_os_virtualengine_singlepipe_next.h"
#include "mos_os_virtualengine_scalability_next.h"

#include "memory_policy_manager.h"
#include "mos_oca_interface_specific.h"
#include "mos_os_next.h"

//!
//! \brief DRM VMAP patch
//!
#define Y_TILE_WIDTH  128
#define Y_TILE_HEIGHT 32
#define X_TILE_WIDTH  512
#define X_TILE_HEIGHT 8

#define MI_BATCHBUFFER_END 0x05000000

//!
//! \brief Two VDBOX shared memory key
//!
#define DUAL_VDBOX_KEY ('D'<<24|'V'<<8|'X'<<0)

//============= PRIVATE FUNCTIONS <BEGIN>=========================================

bool SetupMediaSoloSwitch()
{
    bool mediaSoloEnabled = false;
    MosUtilities::MosReadMediaSoloEnabledUserFeature(mediaSoloEnabled);
    return mediaSoloEnabled;
}

//!
//! \brief    Clear Gpu Context
//! \details  OS GPU context clear
//! \param    PMOS_RESOURCE pContext
//!           [in] Pointer to OS context
//! \return   void
//!           Return NONE
//!
void Mos_Specific_ClearGpuContext(MOS_CONTEXT *pContext)
{
    int32_t iLoop = 0;

    MOS_OS_FUNCTION_ENTER;

    if (nullptr == pContext)
    {
        MOS_OS_ASSERTMESSAGE("Input mos context is null");
        return;
    }

    for (iLoop = 0; iLoop < MOS_GPU_CONTEXT_MAX; iLoop++)
    {
        if (pContext->OsGpuContext[iLoop].pCB != nullptr)
        {
            MOS_FreeMemory(pContext->OsGpuContext[iLoop].pCB);
            pContext->OsGpuContext[iLoop].pCB = nullptr;
        }

        if (pContext->OsGpuContext[iLoop].pAllocationList != nullptr)
        {
            MOS_FreeMemory(pContext->OsGpuContext[iLoop].pAllocationList);
            pContext->OsGpuContext[iLoop].pAllocationList = nullptr;
        }

        if (pContext->OsGpuContext[iLoop].pPatchLocationList)
        {
            MOS_FreeMemory(pContext->OsGpuContext[iLoop].pPatchLocationList);
            pContext->OsGpuContext[iLoop].pPatchLocationList = nullptr;
        }

        if (pContext->OsGpuContext[iLoop].pResources != nullptr)
        {
            MOS_FreeMemory(pContext->OsGpuContext[iLoop].pResources);
            pContext->OsGpuContext[iLoop].pResources = nullptr;
        }

        if (pContext->OsGpuContext[iLoop].pbWriteMode != nullptr)
        {
            MOS_FreeMemory(pContext->OsGpuContext[iLoop].pbWriteMode);
            pContext->OsGpuContext[iLoop].pbWriteMode = nullptr;
        }

        pContext->OsGpuContext[iLoop].uiMaxNumAllocations    = 0;
        pContext->OsGpuContext[iLoop].uiMaxPatchLocationsize = 0;
    }
}

//!
//! \brief    Wait and release command buffer
//! \details  Command buffer Wait and release
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \param    int32_t index
//!           [in] Command buffer's index in Command buffer pool
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_WaitAndReleaseCmdBuffer(
    PMOS_CONTEXT   pOsContext,
    int32_t        index)
{
    MOS_LINUX_BO   *cmd_bo;
    MOS_STATUS     eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsContext);

    if (index < 0 || index >= MAX_CMD_BUF_NUM)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // According to the logic of CmdBufferPool now, the command buffer is used in a circular way.
    // The input index always points to the next(oldest) buffer after the latest(newest) buffer.
    // If the next buffer is not empty (!= nullptr), all the buffers in the pool will also be not empty.
    // So it's not necessary to check all buffers to see whether there's empty buffer.
    cmd_bo = pOsContext->CmdBufferPool.pCmd_bo[index];
    if (cmd_bo != nullptr)
    {
        mos_bo_wait_rendering(cmd_bo);
        mos_bo_unreference(cmd_bo);
        pOsContext->CmdBufferPool.pCmd_bo[index] = nullptr;
    }

finish:
    return eStatus;
}

//!
//! \brief    Release command buffer pool
//! \details  Release command buffer pool until all of commands are finished.
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_ReleaseCmdBufferPool(PMOS_CONTEXT   pOsContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsContext);

    for (int32_t i = 0; i < MAX_CMD_BUF_NUM; i++)
    {
        MOS_OS_CHK_STATUS_RETURN(Linux_WaitAndReleaseCmdBuffer(pOsContext, i));
    }

    return MOS_STATUS_SUCCESS;
}

MosOcaInterface* Linux_GetOcaInterface()
{
    return nullptr;
}

//!
//! \brief    Get GPU tag for the given GPU context from the status buffer
//! \details  Get GPU tag for the given GPU context from the status buffer
//! \param    MOS_CONTEXT * pOsContext
//!           [in] Pointer to OS context structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           GPU tag
//!
uint32_t Linux_GetGPUTag(
        PMOS_INTERFACE             pOsInterface,
        MOS_GPU_CONTEXT            mosGpuCtx)
{
    MOS_OS_FUNCTION_ENTER;

    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("invalid input parameters!");
        return 0;
    }

    if (mosGpuCtx == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return 0;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        if (pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid input parameters!");
            return 0;
        }

        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

        auto handle = osCxtSpecific->GetGpuContextHandle(mosGpuCtx);

        PMOS_RESOURCE gpuStatusResource = nullptr;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetGpuStatusBufferResource(pOsInterface->osStreamState, gpuStatusResource, handle));
        MOS_OS_CHK_NULL_RETURN(gpuStatusResource);
        auto gpuStatusData = (MOS_GPU_STATUS_DATA *)gpuStatusResource->pData;
        if (gpuStatusData == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot find ");
            return 0;
        }
        return gpuStatusData->GPUTag;
    }

    MOS_GPU_STATUS_DATA  *pGPUStatusData = nullptr;

    if ( pOsInterface->pOsContext == nullptr ||
         pOsInterface->pOsContext->pGPUStatusBuffer == nullptr ||
         pOsInterface->pOsContext->pGPUStatusBuffer->pData == nullptr)
    {
        return 0;
    }

    pGPUStatusData = (MOS_GPU_STATUS_DATA *)(pOsInterface->pOsContext->pGPUStatusBuffer->pData + (sizeof(MOS_GPU_STATUS_DATA) * mosGpuCtx));
    if (pGPUStatusData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active resource");
        return 0;
    }
    return pGPUStatusData->GPUTag;
}

//!
//! \brief    Get Buffer Type
//! \details  Returns the type of buffer, 1D, 2D or volume
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   GFX resource Type
//!
MOS_GFXRES_TYPE GetResType(PMOS_RESOURCE pOsResource)
{
    GMM_RESOURCE_INFO *pGmmResourceInfo = nullptr;
    MOS_GFXRES_TYPE    ResType          = MOS_GFXRES_INVALID;
    GMM_RESOURCE_TYPE  GMMResType       = RESOURCE_INVALID;

    if (pOsResource == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsContext is null.");
        return MOS_GFXRES_INVALID;
    }

    pGmmResourceInfo = (GMM_RESOURCE_INFO *)pOsResource->pGmmResInfo;
    GMMResType = pGmmResourceInfo->GetResourceType();

    switch (GMMResType)
    {
    case RESOURCE_BUFFER:
        ResType = MOS_GFXRES_BUFFER;
        break;
    case RESOURCE_3D:
        ResType = MOS_GFXRES_VOLUME;
        break;
    case RESOURCE_2D:
        ResType = MOS_GFXRES_2D;
        break;
    default:
        break;
    }
    return ResType;
}

//============= PRIVATE FUNCTIONS <END>=========================================

//!
//! \brief    Set GPU context
//! \details  Set GPU context for the following rendering operations
//! \param    MOS_OS_CONTEXT * pContext
//!           [in] Pointer to OS context structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetGpuContext(
    PMOS_INTERFACE     pOsInterface,
    MOS_GPU_CONTEXT    GpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (GpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Set GPU context handle
    pOsInterface->CurrentGpuContextOrdinal = GpuContext;

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

        auto pOsContextSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

        // Set GPU context handle
        pOsInterface->CurrentGpuContextHandle = pOsContextSpecific->GetGpuContextHandle(GpuContext);

        MOS_OS_CHK_STATUS_RETURN(MosInterface::SetGpuContext(
            pOsInterface->osStreamState,
            pOsContextSpecific->GetGpuContextHandle(GpuContext)));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_SetGpuContextFromHandle(MOS_INTERFACE *osInterface,
                                                MOS_GPU_CONTEXT contextName,
                                                GPU_CONTEXT_HANDLE contextHandle)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (MOS_GPU_CONTEXT_INVALID_HANDLE == contextName)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter contextName.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Set GPU context handle
    osInterface->CurrentGpuContextOrdinal = contextName;

    if (osInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_CHK_NULL_RETURN(osInterface->osContextPtr);

        auto os_context_specific
                = static_cast<OsContextSpecific*>(osInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(os_context_specific);

        // Set GPU context handle
        osInterface->CurrentGpuContextHandle = contextHandle;

        MOS_OS_CHK_STATUS_RETURN(
            MosInterface::SetGpuContext(osInterface->osStreamState,
                                        contextHandle));
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get current GPU context
//! \details  Get current GPU context for the following rendering operations
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   MOS_GPU_CONTEXT
//!           Return current GPU context
//!
MOS_GPU_CONTEXT Mos_Specific_GetGpuContext(
    PMOS_INTERFACE     pOsInterface)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsInterface is null.");
        return MOS_GPU_CONTEXT_INVALID_HANDLE;
    }

    return pOsInterface->CurrentGpuContextOrdinal;
}

//!
//! \brief    Set GPU context Handle
//! \details  Set GPU context handle for the following rendering operations
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    GPU_CONTEXT_HANDLE gpuContextHandle
//!           [in] GPU Context Handle
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetGpuContextHandle(
    PMOS_INTERFACE     pOsInterface,
    GPU_CONTEXT_HANDLE gpuContextHandle,
    MOS_GPU_CONTEXT    GpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

    auto pOsContextSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
    MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

    pOsContextSpecific->SetGpuContextHandle(GpuContext, gpuContextHandle);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get GPU context pointer
//! \details  Get GPU context pointer
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    GPU_CONTEXT_HANDLE gpuContextHandle
//!           [in] GPU Context Handle
//! \return   void *
//!           a pointer to a gpu context
//!
void *Mos_Specific_GetGpuContextbyHandle(
    PMOS_INTERFACE     pOsInterface,
    GPU_CONTEXT_HANDLE gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    if (!pOsInterface)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return nullptr;
    }

    return MosInterface::GetGpuContext(pOsInterface->osStreamState, gpuContextHandle);
}

//!
//! \brief    Get current GMM client context
//! \details  Get current GMM client context
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   GMM_CLIENT_CONTEXT
//!           Return current GMM client context
//!
GMM_CLIENT_CONTEXT *Mos_Specific_GetGmmClientContext(
    PMOS_INTERFACE pOsInterface)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsInterface is null.");
        return nullptr;
    }

    return MosInterface::GetGmmClientContext(pOsInterface->osStreamState);
}

//!
//! \brief    Get Platform
//! \details  Get platform info
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PLATFORM pPlatform
//!           [out] Pointer to platform
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void Mos_Specific_GetPlatform(
    PMOS_INTERFACE          pOsInterface,
    PLATFORM                *pPlatform)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pPlatform);

    auto platform = MosInterface::GetPlatform(pOsInterface->osStreamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(platform);
    *pPlatform = *platform;
    return;
}

MOS_STATUS Mos_DestroyInterface(PMOS_INTERFACE pOsInterface)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    MOS_STREAM_HANDLE streamState = pOsInterface->osStreamState;
    MOS_OS_CHK_NULL_RETURN(streamState);

    auto deviceContext = streamState->osDeviceContext;
    MOS_OS_CHK_NULL_RETURN(deviceContext);

    if (!Mos_Solo_IsEnabled((PMOS_CONTEXT)streamState->perStreamParameters))
    {
        OsContext *pOsContext = pOsInterface->osContextPtr;
        MOS_OS_CHK_NULL_RETURN(pOsContext);
        // APO MOS destory GPU contexts here instead of inside osContextPtr
        OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsContext);

        auto gpuContextMgr = deviceContext->GetGpuContextMgr();
        for (uint32_t i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
        {
            if (pOsContextSpecific->GetGpuContextHandleByIndex(i) != MOS_GPU_CONTEXT_INVALID_HANDLE)
            {
                if (gpuContextMgr == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("GpuContextMgr is null when destroy GpuContext");
                    break;
                }
                auto gpuContext = gpuContextMgr->GetGpuContext(pOsContextSpecific->GetGpuContextHandleByIndex(i));
                if (gpuContext == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active gpuContextHandle");
                    continue;
                }
                gpuContextMgr->DestroyGpuContext(gpuContext);
                pOsContextSpecific->SetGpuContextHandleByIndex(i, MOS_GPU_CONTEXT_INVALID_HANDLE);
            }
        }

        pOsContext->CleanUp();

        MOS_Delete(pOsContext);
        pOsInterface->osContextPtr = nullptr;
    }

    if (pOsInterface->osCpInterface)
    {
        Delete_MosCpInterface(pOsInterface->osCpInterface);
        pOsInterface->osCpInterface = NULL;
    }

    PMOS_CONTEXT perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    if (perStreamParameters && perStreamParameters->bFreeContext)
    {
        perStreamParameters->SkuTable.reset();
        perStreamParameters->WaTable.reset();
        Mos_Specific_ClearGpuContext(perStreamParameters);

        if (perStreamParameters->contextOffsetList.size())
        {
            perStreamParameters->contextOffsetList.clear();
            perStreamParameters->contextOffsetList.shrink_to_fit();
        }

        if(Mos_Solo_IsEnabled(perStreamParameters))
        {
            Linux_ReleaseCmdBufferPool(perStreamParameters);
            PCOMMAND_BUFFER pCurrCB = nullptr;
            PCOMMAND_BUFFER pNextCB = nullptr;
            for (uint32_t i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
            {
                MOS_FreeMemAndSetNull(perStreamParameters->OsGpuContext[i].pCB);

                pCurrCB = perStreamParameters->OsGpuContext[i].pStartCB;
                for (; (pCurrCB); pCurrCB = pNextCB)
                {
                    pNextCB = pCurrCB->pNext;
                    MOS_FreeMemAndSetNull(pCurrCB);
                }
            }
            Linux_ReleaseGPUStatus(perStreamParameters);
        }
        if (perStreamParameters->intel_context)
        {
            if (perStreamParameters->intel_context->vm)
            {
                mos_gem_vm_destroy(perStreamParameters->intel_context->bufmgr, perStreamParameters->intel_context->vm);
                perStreamParameters->intel_context->vm = nullptr;
            }
            mos_gem_context_destroy(perStreamParameters->intel_context);
            perStreamParameters->intel_context = nullptr;
        }
        MOS_Delete(perStreamParameters);
        streamState->perStreamParameters = nullptr;
    }

    MosInterface::DestroyVirtualEngineState(streamState);
    MOS_FreeMemAndSetNull(pOsInterface->pVEInterf);

    MOS_OS_CHK_STATUS_RETURN(MosInterface::DestroyOsStreamState(streamState));
    pOsInterface->osStreamState = nullptr;
    return MOS_STATUS_SUCCESS;
}
//!
//! \brief    Destroys OS specific allocations
//! \details  Destroys OS specific allocations including destroying OS context
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    int32_t bDestroyVscVppDeviceTag
//!           [in] Destroy VscVppDeviceTagId Flag, no use in Linux
//! \return   void
//!
void Mos_Specific_Destroy(
    PMOS_INTERFACE pOsInterface,
    int32_t        bDestroyVscVppDeviceTag)
{
    MOS_UNUSED(bDestroyVscVppDeviceTag);

    if (nullptr == pOsInterface)
    {
        MOS_OS_ASSERTMESSAGE("input parameter, pOsInterface is NULL.");
        return;
    }

    MOS_STATUS status = Mos_DestroyInterface(pOsInterface);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Mos Destroy Interface failed.");
    }
    return;
}

//!
//! \brief    Gets the SKU table
//! \details  Gets the SKU table for the platform
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   MEDIA_FEATURE_TABLE *
//!           Returns the pointer to sku table
//!
MEDIA_FEATURE_TABLE *Mos_Specific_GetSkuTable(
    PMOS_INTERFACE pOsInterface)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface found be nullptr.");
        return nullptr;
    }
    return MosInterface::GetSkuTable(pOsInterface->osStreamState);
}

//!
//! \brief    Gets the WA table
//! \details  Gets the WA table for the platform
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   MEDIA_WA_TABLE *
//!           Returns the pointer to WA table
//!
MEDIA_WA_TABLE *Mos_Specific_GetWaTable(
    PMOS_INTERFACE pOsInterface)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface found be nullptr.");
        return nullptr;
    }
    return MosInterface::GetWaTable(pOsInterface->osStreamState);
}

//!
//! \brief    Gets the GT System Info
//! \details  Gets the GT System Info
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   MEDIA_SYSTEM_INFO *
//!           pointer to the GT System Info
//!
MEDIA_SYSTEM_INFO *Mos_Specific_GetGtSystemInfo(
    PMOS_INTERFACE     pOsInterface)
{
    if( nullptr == pOsInterface)
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return  nullptr;
    }

    return MosInterface::GetGtSystemInfo(pOsInterface->osStreamState);
}

MOS_STATUS Mos_Specific_GetMediaEngineInfo(
    PMOS_INTERFACE pOsInterface, MEDIA_ENGINE_INFO &info)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMediaEngineInfo(pOsInterface->osStreamState, info));
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Resets OS States
//! \details  Resets OS States for linux
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   void
//!
void Mos_Specific_ResetOsStates(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_FUNCTION_ENTER;

    if (pOsInterface == nullptr)
    {
        return;
    }

    auto status = MosInterface::ResetCommandBuffer(pOsInterface->osStreamState, 0);
    if (MOS_FAILED(status))
    {
        MOS_OS_ASSERTMESSAGE("ResetCommandBuffer failed.");
    }
    return;
 }

//!
//! \brief    Allocate resource
//! \details  To Allocate Buffer, pass Format as Format_Buffer and set the iWidth as size of the buffer.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_ALLOC_GFXRES_PARAMS pParams
//!           [in] Pointer to resource params
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] Pointer to OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_AllocateResource(
    PMOS_INTERFACE           pOsInterface,
    PMOS_ALLOC_GFXRES_PARAMS pParams,
#if MOS_MESSAGES_ENABLED
    PCCHAR                   functionName,
    PCCHAR                   filename,
    int32_t                  line,
#endif // MOS_MESSAGES_ENABLED
    PMOS_RESOURCE            pOsResource)
{
    MOS_STATUS  eStatus         = MOS_STATUS_UNKNOWN;
    bool        osContextValid  = false;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsResource);
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);

    pOsResource->bConvertedFromDDIResource = false;

    if (pOsInterface->osContextPtr != nullptr)
    {
        if (pOsInterface->osContextPtr->GetOsContextValid() == true)
        {
            osContextValid = true;
        }
    }

    pParams->bBypassMODImpl = !((pOsInterface->modulizedMosEnabled) && (!Mos_Solo_IsEnabled(nullptr)) && (osContextValid == true));

    pOsInterface->osStreamState->component = pOsInterface->Component;

    eStatus = MosInterface::AllocateResource(
        pOsInterface->osStreamState,
        pParams,
        pOsResource
#if MOS_MESSAGES_ENABLED
        ,
        functionName,
        filename,
        line
#endif  // MOS_MESSAGES_ENABLED
    );

    return eStatus;
}

//!
//! \brief    Get Resource Information
//! \details  Linux get resource info
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \param    PMOS_SURFACE pResDetails
//!           [out] Pointer to output resource information details
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetResourceInfo(
    PMOS_INTERFACE              pOsInterface,
    PMOS_RESOURCE               pOsResource,
    PMOS_SURFACE                pResDetails)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pResDetails);
    return MosInterface::GetResourceInfo(pOsInterface->osStreamState, pOsResource, *pResDetails);
}

//!
//! \brief    Free the resource
//! \details  Free the allocated resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \return   void
//!
void Mos_Specific_FreeResource(
    PMOS_INTERFACE   pOsInterface,
#if MOS_MESSAGES_ENABLED
    PCCHAR           functionName,
    PCCHAR           filename,
    int32_t          line,
#endif // MOS_MESSAGES_ENABLED
    PMOS_RESOURCE    pOsResource)
{
    MOS_OS_FUNCTION_ENTER;

    if( nullptr == pOsInterface )
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return;
    }

    if( nullptr == pOsResource )
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsResource is NULL.");
        return;
    }

    MosInterface::FreeResource(
        pOsInterface->osStreamState,
        pOsResource,
        0
#if MOS_MESSAGES_ENABLED
        ,
        functionName,
        filename,
        line
#endif
    );

    return;
}

//!
//! \brief    Free OS resource
//! \details  Free OS resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] OS Resource to be freed
//! \param    uint32_t uiFlag
//!           [in] Flag to free resources. This one is useless on Linux, just for compatibility.
//! \return   void
//!
void Mos_Specific_FreeResourceWithFlag(
    PMOS_INTERFACE    pOsInterface,
    PMOS_RESOURCE     pOsResource,
#if MOS_MESSAGES_ENABLED
    PCCHAR            functionName,
    PCCHAR            filename,
    int32_t           line,
#endif // MOS_MESSAGES_ENABLED
    uint32_t          uiFlag)
{
    MOS_UNUSED(uiFlag);

    if (pOsInterface == nullptr ||
        pOsResource == nullptr)
    {
        return;
    }

#if MOS_MESSAGES_ENABLED
    Mos_Specific_FreeResource(pOsInterface, functionName, filename, line, pOsResource);
#else
    Mos_Specific_FreeResource(pOsInterface, pOsResource);
#endif // MOS_MESSAGES_ENABLED
}

//!
//! \brief    Locks Resource request
//! \details  Locks Resource request
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] Resource object
//! \param    PMOS_LOCK_PARAMS pLockFlags
//!           [in] Lock Flags
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_LockSyncRequest(
    PMOS_INTERFACE        pOsInterface,
    PMOS_RESOURCE         pOsResource,
    PMOS_LOCK_PARAMS      pLockFlags)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pOsInterface);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Lock resource
//! \details  Lock allocated resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \param    PMOS_LOCK_PARAMS pLockFlags
//!           [in] Lock Flags - MOS_LOCKFLAG_* flags
//! \return   void *
//!
void  *Mos_Specific_LockResource(
    PMOS_INTERFACE     pOsInterface,
    PMOS_RESOURCE      pOsResource,
    PMOS_LOCK_PARAMS   pLockFlags)
{
    void                *pData = nullptr;
    MOS_OS_CONTEXT      *pContext = nullptr;

    MOS_OS_FUNCTION_ENTER;

    if( nullptr == pOsInterface )
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return nullptr;
    }

    if( nullptr == pOsResource)
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return nullptr;
    }

    return MosInterface::LockMosResource(pOsInterface->osStreamState, pOsResource, pLockFlags);
}

//!
//! \brief    Unlock resource
//! \details  Unlock the locked resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_UnlockResource(
    PMOS_INTERFACE       pOsInterface,
    PMOS_RESOURCE        pOsResource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pOsInterface);

    return MosInterface::UnlockMosResource(pOsInterface->osStreamState, pOsResource);
}

//!
//! \brief    Decompress Resource
//! \details  Decompress Resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] Resource object
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_DecompResource(
    PMOS_INTERFACE        pOsInterface,
    PMOS_RESOURCE         pOsResource)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    return MosInterface::DecompResource(pOsInterface->osStreamState, pOsResource);
}

//!
//! \brief    Set auxiliary sync resource
//! \details  Set auxiliary resource to sync with decompression
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] Resource object
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_SetDecompSyncRes(
    PMOS_INTERFACE pOsInterface,
    PMOS_RESOURCE  syncResource)
{
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Decompress and Copy Resource to Another Buffer
//! \details  Decompress and Copy Resource to Another Buffer
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE inputOsResource
//!           [in] Input Resource object
//! \param    PMOS_RESOURCE outputOsResource
//!           [out] output Resource object
//! \param    [in] bOutputCompressed
//!            true means apply compression on output surface, else output uncompressed surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_DoubleBufferCopyResource(
    PMOS_INTERFACE        osInterface,
    PMOS_RESOURCE         inputOsResource,
    PMOS_RESOURCE         outputOsResource,
    bool                  bOutputCompressed)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::DoubleBufferCopyResource(osInterface->osStreamState, inputOsResource, outputOsResource, bOutputCompressed);
}

//!
//! \brief    Decompress and Copy Resource to Another Buffer
//! \details  Decompress and Copy Resource to Another Buffer
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE inputOsResource
//!           [in] Input Resource object
//! \param    PMOS_RESOURCE outputOsResource
//!           [out] output Resource object
//! \param    [in] copyWidth
//!            The 2D surface Width
//! \param    [in] copyHeight
//!            The 2D surface height
//! \param    [in] copyInputOffset
//!            The offset of copied surface from
//! \param    [in] copyOutputOffset
//!            The offset of copied to
//! \param    [in] bOutputCompressed
//!            true means apply compression on output surface, else output uncompressed surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_MediaCopyResource2D(
    PMOS_INTERFACE        osInterface,
    PMOS_RESOURCE         inputOsResource,
    PMOS_RESOURCE         outputOsResource,
    uint32_t              copyWidth,
    uint32_t              copyHeight,
    uint32_t              copyInputOffset,
    uint32_t              copyOutputOffset,
    uint32_t              bpp,
    bool                  bOutputCompressed)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::MediaCopyResource2D(osInterface->osStreamState, inputOsResource, outputOsResource,
        copyWidth, copyHeight, copyInputOffset, copyOutputOffset, bpp, bOutputCompressed);
}

//!
//! \brief    Get Mos Context
//! \details  Get Mos Context info
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    mosContext void **
//!           [out] pointer of mos_context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetMosContext(
        PMOS_INTERFACE        osInterface,
        PMOS_CONTEXT*         mosContext)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(mosContext);
    void *apo_mos_context = nullptr;
    MOS_OS_CHK_STATUS_RETURN(MosInterface::GetperStreamParameters(osInterface->osStreamState, &apo_mos_context));
    *mosContext = (PMOS_CONTEXT)apo_mos_context;
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set patch entry
//! \details  Sets the patch entry in MS's patch list
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_PATCH_ENTRY_PARAMS pParams
//!           [in] patch entry params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetPatchEntry(
    PMOS_INTERFACE              pOsInterface,
    PMOS_PATCH_ENTRY_PARAMS     pParams)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pParams);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (!pParams->bUpperBoundPatch && pParams->presResource)
    {
        uint32_t handle = pParams->presResource->bo?pParams->presResource->bo->handle:0;
        uint32_t eventData[] = {handle, pParams->uiResourceOffset,
                                pParams->uiPatchOffset, pParams->bWrite,
                                pParams->HwCommandType, pParams->forceDwordOffset,
                                pParams->patchType};
        MOS_TraceEventExt(EVENT_RESOURCE_PATCH, EVENT_TYPE_INFO2,
                          &eventData, sizeof(eventData),
                          nullptr, 0);
    }
#endif
    return MosInterface::SetPatchEntry(pOsInterface->osStreamState, pParams);
}

//!
//! \brief    Update resource usage type
//! \details  update the resource usage for cache policy
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] Pointer to OS Resource
//! \param    MOS_HW_RESOURCE_DEF resUsageType
//!           [in] resosuce usage type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_UpdateResourceUsageType(
    PMOS_RESOURCE           pOsResource,
    MOS_HW_RESOURCE_DEF     resUsageType)
{
    return MosInterface::UpdateResourceUsageType(pOsResource, resUsageType);
}

//!
//! \brief    Registers Resource
//! \details  Set the Allocation Index in OS resource structure
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in/out] Pointer to OS Resource
//! \param    int32_t bWrite
//!           [in] Write Flag
//! \param    int32_t bWritebSetResourceSyncTag
//!           [in] Resource registration Flag, no use for Linux
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_RegisterResource (
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pOsResource,
    int32_t             bWrite,
    int32_t             bWritebSetResourceSyncTag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
    auto dumper = GpuCmdResInfoDumpNext::GetInstance(pOsInterface->pOsContext);
    if(dumper)
    {
        dumper->StoreCmdResPtr(pOsInterface, (const void*)pOsResource);
    }
#endif  // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

    return MosInterface::RegisterResource(
        pOsInterface->osStreamState,
        pOsResource,
        bWrite);
}

//!
//! \brief    Verify command buffer size
//! \details  Verifys the buffer to be used for rendering GPU commands is large enough
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t dwRequestedSize
//!           [in] Buffer size requested
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful (command buffer will be large enough to hold dwMaxSize)
//!           otherwise failed
//!
MOS_STATUS Mos_Specific_VerifyCommandBufferSize(
    PMOS_INTERFACE          pOsInterface,
    uint32_t                dwRequestedSize,
    uint32_t                dwFlags)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::VerifyCommandBufferSize(pOsInterface->osStreamState, 0, dwRequestedSize, dwFlags);
}

//!
//! \brief    Resets Resource Allocation
//! \details  Resets Resource Allocation
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \return   void
//!
void Mos_Specific_ResetResourceAllocationIndex (
    PMOS_INTERFACE   pOsInterface,
    PMOS_RESOURCE    pOsResource)
{
    int32_t i;
    MOS_UNUSED(pOsInterface);

    if( nullptr == pOsResource)
    {
        MOS_OS_ASSERTMESSAGE("pOsResource is NULL.");
        return;
    }

    for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        pOsResource->iAllocationIndex[i] = MOS_INVALID_ALLOC_INDEX;
    }
}

//!
//! \brief    Get command buffer
//! \details  Retrieves buffer to be used for rendering GPU commands
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [out] Pointer to Command Buffer control structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetCommandBuffer(
    PMOS_INTERFACE          pOsInterface,
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    uint32_t                dwFlags)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::GetCommandBuffer(pOsInterface->osStreamState, pCmdBuffer, dwFlags);
}

//!
//! \brief    Set indirect state size
//! \details  Sets indirect state size to be used for rendering purposes
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t uSize
//!           [in] State size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetIndirectStateSize(
    PMOS_INTERFACE              pOsInterface,
    uint32_t                    uSize)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::SetupIndirectState(pOsInterface->osStreamState, uSize);
}

//!
//! \brief    Set indirect state size
//! \details  Retrieves indirect state to be used for rendering purposes
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t *puOffset
//!           [out] Pointer to indirect buffer offset
//! \param    uint32_t *puSize
//!           [out] Pointer to indirect buffer size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetIndirectState(
    PMOS_INTERFACE           pOsInterface,
    uint32_t                *puOffset,
    uint32_t                *puSize)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(puOffset);
    MOS_OS_CHK_NULL_RETURN(puSize);

    uint32_t offset = 0;
    uint32_t size   = 0;
    auto eStatus = MosInterface::GetIndirectState(pOsInterface->osStreamState, nullptr, offset, size);
    *puOffset = offset;
    *puSize   = size;
    return eStatus;
}

//!
//! \brief    Get Resource Allocation Index
//! \details  Get Resource Allocation Index
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pResource
//!           [in] Pointer to input OS resource
//! \return   int32_t
//!           return the allocation index
//!
int32_t Mos_Specific_GetResourceAllocationIndex(
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pResource)
{
    MOS_OS_FUNCTION_ENTER;

    if (pResource && pOsInterface)
    {

       if (pOsInterface->CurrentGpuContextHandle == MOS_GPU_CONTEXT_INVALID_HANDLE)
       {
           MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
           return MOS_STATUS_INVALID_PARAMETER;
       }

        return(pResource->iAllocationIndex[pOsInterface->CurrentGpuContextOrdinal]);
    }

    return MOS_INVALID_ALLOC_INDEX;

}

//!
//! \brief    Get Indirect State Pointer
//! \details  Get Indirect State Pointer
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint8_t **pIndirectState
//!           [out] Pointer to Indirect State Buffer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetIndirectStatePointer(
    PMOS_INTERFACE      pOsInterface,
    uint8_t             **pIndirectState)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    uint32_t offset = 0;
    uint32_t size   = 0;
    return MosInterface::GetIndirectState(pOsInterface->osStreamState, pIndirectState, offset, size);
}

//!
//! \brief    Return command buffer space
//! \details  Return unused command buffer space
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [out] Pointer to Command Buffer control structure
//! \return   void
//!
void Mos_Specific_ReturnCommandBuffer(
    PMOS_INTERFACE          pOsInterface,
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    uint32_t                dwFlags)
{
    MOS_OS_FUNCTION_ENTER;

    if (pOsInterface == nullptr || pCmdBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameters.");
        return;
    }

    // Need to ensure 4K extra padding for HW requirement.
    if (pCmdBuffer && pCmdBuffer->iRemaining < EXTRA_PADDING_NEEDED)
    {
        MOS_OS_ASSERTMESSAGE("Need to ensure 4K extra padding for HW requirement.");
    }

    auto status = MosInterface::ReturnCommandBuffer(pOsInterface->osStreamState, pCmdBuffer, dwFlags);
    if (MOS_FAILED(status))
    {
        MOS_OS_ASSERTMESSAGE("ReturnCommandBuffer failed.");
    }
    return;
}

//!
//! \brief    Submit command buffer
//! \details  Submit the command buffer
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer control structure
//! \param    int32_t bNullRendering
//!           [in] boolean null rendering
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SubmitCommandBuffer(
    PMOS_INTERFACE        pOsInterface,
    PMOS_COMMAND_BUFFER   pCmdBuffer,
    int32_t               bNullRendering)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    return MosInterface::SubmitCommandBuffer(pOsInterface->osStreamState, pCmdBuffer, bNullRendering);
}

//!
//! \brief    Get resource gfx address
//! \details  Get resource gfx address
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pResource
//!           [in] Pointer to resource
//! \return   uint64_t
//!           Return resource gfx address
//!
uint64_t Mos_Specific_GetResourceGfxAddress(
    PMOS_INTERFACE   pOsInterface,
    PMOS_RESOURCE    pResource)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::GetResourceGfxAddress(pOsInterface->osStreamState, pResource);
}

//!
//! \brief    Resizes the buffer to be used for rendering GPU commands
//! \details  return true if succeeded - command buffer will be large enough to hold dwMaxSize
//!           return false if failed or invalid parameters
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t dwRequestedCommandBufferSize
//!           [in] requested command buffer size
//! \param    uint32_t dwRequestedPatchListSize
//!           [in] requested patch list size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_ResizeCommandBufferAndPatchList(
    PMOS_INTERFACE          pOsInterface,
    uint32_t                dwRequestedCommandBufferSize,
    uint32_t                dwRequestedPatchListSize,
    uint32_t                dwFlags)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::ResizeCommandBufferAndPatchList(pOsInterface->osStreamState, 0, dwRequestedCommandBufferSize, dwRequestedPatchListSize, dwFlags);
}

//!
//! \brief    Create GPU context
//! \details  Create GPU context
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \param    MOS_GPU_NODE GpuNode
//!           [in] GPU Node
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_CreateGpuContext(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       mosGpuCxt,
    MOS_GPU_NODE          GpuNode,
    PMOS_GPUCTX_CREATOPTIONS createOption)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (mosGpuCxt == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

        auto pOsContextSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

        auto gpuContextMgr = pOsContextSpecific->GetGpuContextMgr();
        if (!pOsInterface->apoMosEnabled)
        {
            MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
        }

        auto cmdBufMgr = pOsContextSpecific->GetCmdBufMgr();
        if (!pOsInterface->apoMosEnabled)
        {
            MOS_OS_CHK_NULL_RETURN(cmdBufMgr);
        }

        MOS_OS_CHK_NULL_RETURN(createOption);
        if (GpuNode == MOS_GPU_NODE_3D && createOption->SSEUValue != 0)
        {
            struct drm_i915_gem_context_param_sseu sseu;
            MOS_ZeroMemory(&sseu, sizeof(sseu));
            sseu.engine.engine_class = I915_ENGINE_CLASS_RENDER;
            sseu.engine.engine_instance = 0;

            if (mos_get_context_param_sseu(pOsInterface->pOsContext->intel_context, &sseu))
            {
                MOS_OS_ASSERTMESSAGE("Failed to get sseu configuration.");
                return MOS_STATUS_UNKNOWN;
            };

            if (mos_hweight8(sseu.subslice_mask) > createOption->packed.SubSliceCount)
            {
                sseu.subslice_mask = mos_switch_off_n_bits(sseu.subslice_mask,
                        mos_hweight8(sseu.subslice_mask)-createOption->packed.SubSliceCount);
            }

            if (mos_set_context_param_sseu(pOsInterface->pOsContext->intel_context, sseu))
            {
                MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
                return MOS_STATUS_UNKNOWN;
            };
        }

        createOption->gpuNode = GpuNode;
        // Update ctxBasedScheduling from legacy OsInterface
        pOsInterface->osStreamState->ctxBasedScheduling = pOsInterface->ctxBasedScheduling;
        if (pOsContextSpecific->GetGpuContextHandle(mosGpuCxt) == MOS_GPU_CONTEXT_INVALID_HANDLE)
        {
            auto osDeviceContext = pOsInterface->osStreamState->osDeviceContext;
            MOS_OS_CHK_NULL_RETURN(osDeviceContext);
            auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
            MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
            auto cmdBufMgr = osDeviceContext->GetCmdBufferMgr();
            MOS_OS_CHK_NULL_RETURN(cmdBufMgr);

            auto gpuContext = gpuContextMgr->CreateGpuContext(GpuNode, cmdBufMgr);
            MOS_OS_CHK_NULL_RETURN(gpuContext);

            auto gpuContextSpecific  = static_cast<GpuContextSpecificNext *>(gpuContext);
            MOS_OS_CHK_NULL_RETURN(gpuContextSpecific);

            MOS_OS_CHK_STATUS_RETURN(gpuContextSpecific->Init(gpuContextMgr->GetOsContext(), pOsInterface->osStreamState, createOption));
            gpuContextSpecific->SetGpuContext(mosGpuCxt);

            pOsContextSpecific->SetGpuContextHandle(mosGpuCxt, gpuContextSpecific->GetGpuContextHandle());
        }

        return MOS_STATUS_SUCCESS;

    }

    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(mosGpuCxt);
    MOS_UNUSED(GpuNode);
    MOS_UNUSED(createOption);
    return MOS_STATUS_SUCCESS;
}

GPU_CONTEXT_HANDLE
Mos_Specific_CreateGpuComputeContext(MOS_INTERFACE *osInterface,
                                     MOS_GPU_CONTEXT contextName,
                                     MOS_GPUCTX_CREATOPTIONS *createOption)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (MOS_GPU_CONTEXT_CM_COMPUTE != contextName
        && MOS_GPU_CONTEXT_COMPUTE != contextName)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter contextName.");
        return MOS_GPU_CONTEXT_INVALID_HANDLE;
    }

    if (osInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        if (nullptr == createOption)
        {
            MOS_OS_ASSERTMESSAGE("Invalid pointer (nullptr).");
            return MOS_GPU_CONTEXT_INVALID_HANDLE;
        }

        if (nullptr == osInterface->osStreamState)
        {
            MOS_OS_ASSERTMESSAGE("Invalid pointer (nullptr).");
            return MOS_GPU_CONTEXT_INVALID_HANDLE;
        }

        // Update ctxBasedScheduling from legacy OsInterface
        osInterface->osStreamState->ctxBasedScheduling
                = osInterface->ctxBasedScheduling;

        // Only wrapper will contain re-creation check based on stream Index and MOS_GPU_CONTEXT
        createOption->gpuNode = MOS_GPU_NODE_COMPUTE;
        GPU_CONTEXT_HANDLE context_handle = MOS_GPU_CONTEXT_INVALID_HANDLE;
        MOS_STATUS status
                = MosInterface::CreateGpuContext(osInterface->osStreamState,
                                                    *createOption, context_handle);
        if (MOS_FAILED(status))
        {
            return MOS_GPU_CONTEXT_INVALID_HANDLE;
        }

        auto os_device_context = osInterface->osStreamState->osDeviceContext;
        auto gpu_context_mgr = os_device_context->GetGpuContextMgr();
        if (nullptr == gpu_context_mgr)
        {
            return MOS_GPU_CONTEXT_INVALID_HANDLE;
        }
        GpuContextNext *context_next
                = gpu_context_mgr->GetGpuContext(context_handle);
        auto context_specific_next
                = static_cast<GpuContextSpecificNext*>(context_next);
        if (nullptr == context_specific_next)
        {
            return MOS_GPU_CONTEXT_INVALID_HANDLE;
        }
        context_specific_next->SetGpuContext(contextName);
        return context_handle;
    }
    return MOS_GPU_CONTEXT_INVALID_HANDLE;
}

//!
//! \brief    Destroy GPU context
//! \details  Destroy GPU context
//!           [in] Pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_DestroyGpuContext(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       mosGpuCxt)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (mosGpuCxt == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

        OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

        GPU_CONTEXT_HANDLE gpuContextHandle = pOsContextSpecific->GetGpuContextHandle(mosGpuCxt);

        return MosInterface::DestroyGpuContext(pOsInterface->osStreamState, gpuContextHandle);
    }

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(mosGpuCxt);
    return eStatus;
}

//!
//! \brief    Destroy Compute GPU context
//! \details  Destroy Compute GPU context
//!           [in] Pointer to OS interface structure
//! \param    GPU_CONTEXT_HANDLE gpuContextHandle
//!           [in] GPU Context handle
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_DestroyGpuComputeContext(
    PMOS_INTERFACE        osInterface,
    GPU_CONTEXT_HANDLE    gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    if(MOS_GPU_CONTEXT_INVALID_HANDLE == gpuContextHandle)
    {
        MOS_OS_ASSERTMESSAGE("Invalid compute gpu context handle.");
        return MOS_STATUS_INVALID_HANDLE;
    }
    if (!osInterface->modularizedGpuCtxEnabled || Mos_Solo_IsEnabled(nullptr))
    {
        return MOS_STATUS_SUCCESS;
    }

    OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(osInterface->osContextPtr);
    MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

    GPU_CONTEXT_HANDLE iGpuContextHandle = pOsContextSpecific->GetGpuContextHandle(MOS_GPU_CONTEXT_CM_COMPUTE);
    if(iGpuContextHandle == gpuContextHandle)
    {
        MOS_OS_ASSERTMESSAGE("It will be destroyed in osInterface destroy.");
        return MOS_STATUS_SUCCESS;
    }

    iGpuContextHandle = pOsContextSpecific->GetGpuContextHandle(MOS_GPU_CONTEXT_COMPUTE);
    if(iGpuContextHandle == gpuContextHandle)
    {
        MOS_OS_ASSERTMESSAGE("It will be destroyed in osInterface destroy.");
        return MOS_STATUS_SUCCESS;
    }

    auto gpuContext = MosInterface::GetGpuContext(osInterface->osStreamState, gpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_GPU_CONTEXT gpuContextName = gpuContext->GetCpuContextID();
    if(gpuContextName != MOS_GPU_CONTEXT_CM_COMPUTE && gpuContextName != MOS_GPU_CONTEXT_COMPUTE)
    {
        MOS_OS_ASSERTMESSAGE("It is not compute gpu context and it will be destroyed in osInterface destroy.");
        return MOS_STATUS_SUCCESS;
    }
    return MosInterface::DestroyGpuContext(osInterface->osStreamState, gpuContextHandle);
 }

//!
//! \brief    Sets the perf tag
//! \details  Sets the perf tag
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t PerfTag
//!           [in] Perf tag
//! \return   void
//!
void Mos_Specific_SetPerfTag(
    PMOS_INTERFACE    pOsInterface,
    uint32_t          PerfTag)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface->osStreamState);
    pOsInterface->osStreamState->component = pOsInterface->Component;
    return MosInterface::SetPerfTag(pOsInterface->osStreamState, PerfTag);
}

//!
//! \brief    Resets the perf tag
//! \details  Resets the perf tag
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \return   void
//!
void Mos_Specific_ResetPerfBufferID(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    return MosInterface::ResetPerfBufferID(pOsInterface->osStreamState);
}

//!
//! \brief    Increment the perf tag for buffer ID
//! \details  Increment the perf tag for buffer ID
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \return   void
//!
void Mos_Specific_IncPerfBufferID(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);
    return MosInterface::IncPerfBufferID(pOsInterface->osStreamState);
}

//!
//! \brief    Increment the perf tag for Frame ID
//! \details  Increment the perf tag for frame ID
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \return   void
//!
void Mos_Specific_IncPerfFrameID(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    return MosInterface::IncPerfFrameID(pOsInterface->osStreamState);
}

//!
//! \brief    Gets the perf tag
//! \details  Gets the perf tag
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \return   uint32_t
//!           Return perf tag
//!
uint32_t Mos_Specific_GetPerfTag(
    PMOS_INTERFACE pOsInterface)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface invalid nullptr");
        return 0;
    }

    return MosInterface::GetPerfTag(pOsInterface->osStreamState);
}

//!
//! \brief    Set Hybrid Kernel ID
//! \details  Set Hybrid Kernel ID
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    uint32_t KernelID
//!           [in] Hybrid Decoder Kernel ID
//! \return   void
//!
void Mos_Specific_SetPerfHybridKernelID(
    PMOS_INTERFACE     pOsInterface,
    uint32_t           KernelID)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    return MosInterface::SetPerfHybridKernelID(pOsInterface->osStreamState, KernelID);
}

//!
//! \brief    Check if Perf Tag is already set
//! \details  Check if Perf Tag is already set
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   int32_t
//!
int32_t Mos_Specific_IsPerfTagSet(
    PMOS_INTERFACE     pOsInterface)
{
    uint32_t     ComponentTag;
    int32_t      bRet = false;
    PMOS_CONTEXT pOsContext;

    if(pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface invalid nullptr");
        return bRet;
    }

    return MosInterface::IsPerfTagSet(pOsInterface->osStreamState);
}

//!
//! \brief    Check for GPU context valid
//! \details  Always returns MOS_STATUS_SUCCESS on Linux.
//            This interface is implemented for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to Os interface structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] Gpu Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_IsGpuContextValid(
    PMOS_INTERFACE         pOsInterface,
    MOS_GPU_CONTEXT        GpuContext)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(GpuContext);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Unified OS Resources sync
//! \details  Tag based synchronization at the resource level
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \param    MOS_GPU_CONTEXT requestorGPUCtx
//!           [in] GPU  Context
//! \param    int32_t bWriteOperation
//!           [in] true if write operation
//! \return   void
//!
void Mos_Specific_SyncOnResource(
    PMOS_INTERFACE          pOsInterface,
    PMOS_RESOURCE           pOsResource,
    MOS_GPU_CONTEXT         requestorGPUCtx,
    int32_t                 bWriteOperation)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pOsResource);
    MOS_UNUSED(requestorGPUCtx);
    MOS_UNUSED(bWriteOperation);
}

//!
//! \brief    Checks for HW enabled
//! \details  Checks for HW enabled
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to Os interface structure
//! \return   int32_t
//!           Returns true id HW enabled if CP mode
//!
int32_t Mos_Specific_IsNullHWEnabled(
    PMOS_INTERFACE     pOsInterface)
{
    MOS_UNUSED(pOsInterface);
    return false;
}

//!
//! \brief    Get GPU status buffer
//! \details  Gets the status buffer for the resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [out] Pointer to OS Resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetGpuStatusBufferResource(
    PMOS_INTERFACE         pOsInterface,
    PMOS_RESOURCE          &pOsResource)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);

    return MosInterface::GetGpuStatusBufferResource(pOsInterface->osStreamState, pOsResource, pOsInterface->osStreamState->currentGpuContextHandle);
}

//!
//! \brief    Get GPU status tag offset
//! \details  Gets the status tag offset
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           Returns the tag offset
//!
uint32_t Mos_Specific_GetGpuStatusTagOffset(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       GpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    return 0;
}

//!
//! \brief    Get GPU status tag
//! \details  Gets the status tag
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           Returns the tag
//!
uint32_t Mos_Specific_GetGpuStatusTag(
    PMOS_INTERFACE            pOsInterface,
    MOS_GPU_CONTEXT           mosGpuCtx)
{
    MOS_OS_FUNCTION_ENTER;

    if(pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface == nullptr");
        return -1;
    }

    if (mosGpuCtx == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return -1;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        if (pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid input parameters!");
            return 0;
        }

        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

        auto handle = osCxtSpecific->GetGpuContextHandle(mosGpuCtx);

        return MosInterface::GetGpuStatusTag(pOsInterface->osStreamState, handle);
    }
    MOS_OS_ASSERTMESSAGE("Invalid input parameter.");
    return -1;
}

//!
//! \brief    Increment GPU status tag
//! \details  Increment the status tag
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   void
//!
void Mos_Specific_IncrementGpuStatusTag(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       mosGpuCtx)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(pOsInterface);

    if (mosGpuCtx == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        if (pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid input parameters!");
            return;
        }

        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

        auto handle = osCxtSpecific->GetGpuContextHandle(mosGpuCtx);

        MosInterface::IncrementGpuStatusTag(pOsInterface->osStreamState, handle);
        return;
    }

    MOS_OS_ASSERTMESSAGE("Invalid input parameter.");
    return;
}

//!
//! \brief    Get GPU status Sync tag
//! \details  This function will return the GPU status tag which is updated when corresponding 
//!           GPU packet is done. User can use this flag to check if some GPU packet is done.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           Returns the tag
//!
uint32_t Mos_Specific_GetGpuStatusSyncTag(
    PMOS_INTERFACE            pOsInterface,
    MOS_GPU_CONTEXT           gpuContext)
{
    MOS_OS_FUNCTION_ENTER;
    PMOS_CONTEXT pOsContext = nullptr;

    if(!pOsInterface || !pOsInterface->pOsContext)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return 0;
    }
    pOsContext = pOsInterface->pOsContext;

    return pOsContext->GetGPUTag(pOsInterface, gpuContext);
}

//!
//! \brief    Sets the resource sync tag
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   void
//!
void Mos_Specific_SetResourceSyncTag(
    PMOS_INTERFACE         pOsInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pParams);
    return ;
}

//!
//! \brief    Perform overlay sync
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_PerformOverlaySync(
    PMOS_INTERFACE         pOsInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pParams);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Wait on resource
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_ResourceWait(
    PMOS_INTERFACE         pOsInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pParams);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Registers a complete notification event
//! \details  Registers a complete notification event
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_RegisterBBCompleteNotifyEvent(
    PMOS_INTERFACE     pOsInterface,
    MOS_GPU_CONTEXT    GpuContext)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(GpuContext);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Waits on a complete notification event
//! \details  Waits on a complete notification event
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    uint32_t uiTimeOut
//!           [in] Time to wait
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_WaitForBBCompleteNotifyEvent(
    PMOS_INTERFACE     pOsInterface,
    MOS_GPU_CONTEXT    GpuContext,
    uint32_t           uiTimeOut)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(GpuContext);
    usleep(uiTimeOut);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_WaitAllCmdCompletion_Os(
    PMOS_INTERFACE pOsInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

//!
//! \brief    Get interface version
//! \details  Get interface version
//!
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//!
//! \return   uint32_t
//!           Read-only OS runtime interface version, it's meaning diff from OS and API
//!
uint32_t Mos_Specific_GetInterfaceVersion(
    PMOS_INTERFACE        pOsInterface)
{
    MOS_UNUSED(pOsInterface);
    return 0;
}

//!
//! \brief    Sets the flags to skip the sync on a particular resource
//! \details  This is required to avoid updating resource tags/masks for a given resource
//!           This way we dont end up putting unnecessary sync points by syncing on this particular resource
//! \param    PMOS_ALLOC_GFXRES_PARAMS pAllocParams
//!           [in] allocation parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if success else failure reason
//!
MOS_STATUS Mos_Specific_SkipResourceSync(
    PMOS_RESOURCE               pOsResource)
{
    MOS_OS_CHK_NULL_RETURN(pOsResource);

    mos_bo_set_object_async(pOsResource->bo);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Gets the HW rendering flags
//! \details  Gets the HW rendering flags
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   MOS_NULL_RENDERING_FLAGS 
//!           Returns the null rendering flags
//!
MOS_NULL_RENDERING_FLAGS  Mos_Specific_GetNullHWRenderFlags(
    PMOS_INTERFACE         pOsInterface)
{
    return pOsInterface->NullHWAccelerationEnable;
}

//!
//! \brief    Debug hook to note type of surface state or sampler state being
//!           used.
//! \details  Sets the Command buffer debug info
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    int32_t bSamplerState
//!           [in] Sampler state
//! \param    int32_t bSurfaceState
//!           [in] Surface state
//! \param    uint32_t dwStateIndex
//!           [in] State index
//! \param    uint32_t dwType
//!           [in] dword type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetCmdBufferDebugInfo(
    PMOS_INTERFACE              pOsInterface,
    int32_t                     bSamplerState,
    int32_t                     bSurfaceState,
    uint32_t                    dwStateIndex,
    uint32_t                    dwType)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(bSamplerState);
    MOS_UNUSED(bSurfaceState);
    MOS_UNUSED(dwStateIndex);
    MOS_UNUSED(dwType);
    // stub function. implemented for simulation but not driver.
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Command buffer debug
//! \details  Debug hook to get type of surface state or sampler state being
//!           used.
//! \param    PMOS_INTERFACE pOsInterface
//! \param    int32_t bSamplerState
//! \param    int32_t bSurfaceState
//! \param    uint32_t dwStateIndex
//! \return   uint32_t
//!
uint32_t Mos_Specific_GetCmdBufferDebugInfo(
    PMOS_INTERFACE              pOsInterface,
    int32_t                     bSamplerState,
    int32_t                     bSurfaceState,
    uint32_t                    dwStateIndex)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(bSamplerState);
    MOS_UNUSED(bSurfaceState);
    MOS_UNUSED(dwStateIndex);
    // stub function. implemented for simulation but not driver.
    return 0;
}

//!
//! \brief    Set PAK/MFX context for Encoder which can be used for Synchronization
//! \details  On Linux, the synchronization is handled in KMD, no job in UMD
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void Mos_Specific_SetEncodePakContext(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       GpuContext)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(GpuContext);
    return;
}

//!
//! \brief    Set VME/ENC context for Encoder which can be used for Synchronization
//! \details  On Linux, the synchronization is handled in KMD, no job in UMD
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void Mos_Specific_SetEncodeEncContext(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       GpuContext)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(GpuContext);
    return;
}

//!
//! \brief    Verifys the patch list to be used for rendering GPU commands is large enough
//! \details
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    uint32_t dwRequestedSize
//!           [in] patch list size to be verified
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_VerifyPatchListSize(
    PMOS_INTERFACE          pOsInterface,
    uint32_t                dwRequestedSize)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    // No APO MOS interface support for this func, implement in wrapper
    auto streamState = pOsInterface->osStreamState;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    auto osDeviceContext = streamState->osDeviceContext;

    auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);

    auto gpuCtx = gpuContextMgr->GetGpuContext(streamState->currentGpuContextHandle);

    auto gpuCtxSpecific = static_cast<GpuContextSpecificNext *>(gpuCtx);
    MOS_OS_CHK_NULL_RETURN(gpuCtxSpecific);
    return (gpuCtxSpecific->VerifyPatchListSize(dwRequestedSize));
}

//!
//! \brief    reset command buffer space
//! \details  resets the command buffer space
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] pointer to command buffer structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_ResetCommandBuffer(
    PMOS_INTERFACE          pOsInterface,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::ResetCommandBuffer(pOsInterface->osStreamState, pCmdBuffer);
}

//!
//! \brief    Get the memory compression mode
//! \details  Gets the memory compression mode from GMM
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] pointer to input OS resource
//! \param    PMOS_MEMCOMP_STATE pResMmcMode
//!           [out] the memory compression mode gotten from OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetMemoryCompressionMode(
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pOsResource,
    PMOS_MEMCOMP_STATE  pResMmcMode)
{
    MOS_OS_CHK_NULL_RETURN(pResMmcMode);
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::GetMemoryCompressionMode(pOsInterface->osStreamState, pOsResource, *pResMmcMode);
}

//!
//! \brief    Set the memory compression mode in GMM
//! \details  Set the memory compression mode
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] pointer to input OS resource
//! \param    MOS_MEMCOMP_STATE ResMmcMode
//!           [in] the memory compression mode to be set into OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetMemoryCompressionMode(
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pOsResource,
    MOS_MEMCOMP_STATE   resMmcMode)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::SetMemoryCompressionMode(pOsInterface->osStreamState, pOsResource, resMmcMode);
}

//!
//! \brief    Set the memory compression hint in GMM on Linux or Gralloc on Android
//! \details  Indicate if the surface is compressible 
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] pointer to input OS resource
//! \param    int32_t bHintOn
//!           [in] the memory compression hint to be set
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetMemoryCompressionHint(
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pOsResource,
    int32_t             bHintOn)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::SetMemoryCompressionHint(pOsInterface->osStreamState, pOsResource, bHintOn);
}

//!
//! \brief    Get the memory compression format
//! \details  Gets the memory compression format from GMM
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] pointer to input OS resource
//! \param    uint32_t *pResMmcFormat
//!           [out] the memory compression format gotten from GMM resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetMemoryCompressionFormat(
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pOsResource,
    uint32_t            *pResMmcFormat)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    return MosInterface::GetMemoryCompressionFormat(pOsInterface->osStreamState, pOsResource, pResMmcFormat);
}

//!
//! \brief    Create GPU node association.
//! \details  Create GPU node association.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    MOS_MEDIA_OPERATION MediaOperation
//!           [in] Media operation
//! \param    MOS_GPU_NODE *pVideoNodeOrdinal
//!           [out] VCS node ordinal
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS Mos_Specific_CreateVideoNodeAssociation(
    PMOS_INTERFACE      pOsInterface,
    int32_t             bSetVideoNode,
    MOS_GPU_NODE        *pVideoNodeOrdinal)
{
    PMOS_OS_CONTEXT         pOsContext      = nullptr;
    PVDBOX_WORKLOAD         pVDBoxWorkLoad  = nullptr;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pVideoNodeOrdinal);
    MOS_OS_CHK_NULL_RETURN(pOsInterface->pOsContext);

    pOsContext = pOsInterface->pOsContext;

    if (false == pOsContext->bKMDHasVCS2)
    {
        *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO;
        return MOS_STATUS_SUCCESS;
    }

    // If node selection is forced or we have only one VDBox, turn balancing off.
    // After that check debug flags.
    if (pOsInterface->bEnableVdboxBalancing)
    {
        pOsContext->bPerCmdBufferBalancing = !bSetVideoNode && pOsContext->bKMDHasVCS2 && pOsInterface->pfnGetVdboxNodeId;
    }
    else 
    {
        pOsContext->bPerCmdBufferBalancing = 0;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pOsInterface->eForceVdbox == MOS_FORCE_VDBOX_1)
    {
        bSetVideoNode = true;
        *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO;
        pOsContext->bPerCmdBufferBalancing = 0;
    }
    else if (pOsInterface->eForceVdbox == MOS_FORCE_VDBOX_2)
    {
        bSetVideoNode = true;
        *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO2;
        pOsContext->bPerCmdBufferBalancing = 0;
    }
#endif // _DEBUG || _RELEASE_INTERNAL
   
    if (pOsContext->semid == MOS_LINUX_IPC_INVALID_ID)
    {
        MOS_OS_ASSERTMESSAGE("Invalid semid in OsContext.");
        return MOS_STATUS_UNKNOWN;
    }

    LockSemaphore(pOsContext->semid);

    pVDBoxWorkLoad = (PVDBOX_WORKLOAD)pOsContext->pShm;
    if(pVDBoxWorkLoad == nullptr)
    {
        UnLockSemaphore(pOsContext->semid);
        MOS_OS_ASSERTMESSAGE("VDBoxWorkLoad not set.");
        return MOS_STATUS_UNKNOWN;
    }

    if (bSetVideoNode)
    {
        if (*pVideoNodeOrdinal == MOS_GPU_NODE_VIDEO)
        {
            pVDBoxWorkLoad->uiVDBoxCount[0]++;
        }
        else if (*pVideoNodeOrdinal == MOS_GPU_NODE_VIDEO2)
        {
            pVDBoxWorkLoad->uiVDBoxCount[1]++;
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("VDBoxWorkLoad not set.");
        }
    }
    else
    {
        if (pVDBoxWorkLoad->uiVDBoxCount[0] < pVDBoxWorkLoad->uiVDBoxCount[1])
        {
            *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO;
            pVDBoxWorkLoad->uiVDBoxCount[0]++;
        }
        else if (pVDBoxWorkLoad->uiVDBoxCount[0] == pVDBoxWorkLoad->uiVDBoxCount[1])
        {
            // this ping-pong method improves much performance for multi-session HD to HD xcode
            if (pVDBoxWorkLoad->uiRingIndex == 0)
            {
                *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO;
                pVDBoxWorkLoad->uiVDBoxCount[0]++;
                pVDBoxWorkLoad->uiRingIndex = 1;
            }
            else
            {
                *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO2;
                pVDBoxWorkLoad->uiVDBoxCount[1]++;
                pVDBoxWorkLoad->uiRingIndex = 0;
            }
        }
        else
        {
            *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO2;
            pVDBoxWorkLoad->uiVDBoxCount[1]++;
        }
    }

    UnLockSemaphore(pOsContext->semid);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroy GPU node association.
//! \details  Destroy GPU node association.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    MOS_GPU_NODE VideoNodeOrdinal
//!           [in] VCS node ordinal
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS Mos_Specific_DestroyVideoNodeAssociation(
    PMOS_INTERFACE     pOsInterface,
    MOS_GPU_NODE       VideoNodeOrdinal)
{
    PMOS_OS_CONTEXT         pOsContext = nullptr;
    PVDBOX_WORKLOAD         pVDBoxWorkLoad = nullptr;

    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsInterface->pOsContext);
    pOsContext    = pOsInterface->pOsContext;

    // not do workload balancing in UMD just return;
    if (pOsContext->bKMDHasVCS2 == false)
    {
        return MOS_STATUS_SUCCESS;
    }

     if (pOsContext->semid == MOS_LINUX_IPC_INVALID_ID)
     {
         MOS_OS_ASSERTMESSAGE("Invalid semid in OsContext.");
         return MOS_STATUS_UNKNOWN;
     }

    LockSemaphore(pOsContext->semid);

    pVDBoxWorkLoad = (PVDBOX_WORKLOAD)pOsContext->pShm;
    MOS_OS_ASSERT(pVDBoxWorkLoad);

    if (VideoNodeOrdinal == MOS_GPU_NODE_VIDEO)
    {
        pVDBoxWorkLoad->uiVDBoxCount[0]--;
    }
    else
    {
        pVDBoxWorkLoad->uiVDBoxCount[1]--;
    }

    UnLockSemaphore(pOsContext->semid);

    return MOS_STATUS_SUCCESS;
}

MOS_VDBOX_NODE_IND Mos_Specific_GetVdboxNodeId(
    PMOS_INTERFACE pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer)
{
    MOS_VDBOX_NODE_IND idx = MOS_VDBOX_NODE_INVALID;

    MOS_OS_CHK_NULL_NO_STATUS(pCmdBuffer);
    MOS_OS_CHK_NULL_NO_STATUS(pOsInterface);
    MOS_OS_CHK_NULL_NO_STATUS(pOsInterface->pOsContext);

    // If we have assigned vdbox index for the given cmdbuf, return it immediately
    if (MOS_VDBOX_NODE_INVALID != pCmdBuffer->iVdboxNodeIndex) {
        idx = pCmdBuffer->iVdboxNodeIndex;
        return idx;
    }

finish:
    return idx;
}

//!
//! \brief    Get the memory object
//! \details  Get the memory object for cache policy
//! \param    MOS_HW_RESOURCE_DEF MosUsage
//!           [in] HW resource
//!           [in] Gmm client context
//! \return   MEMORY_OBJECT_CONTROL_STATE
//!           Return the memory object
//!
MEMORY_OBJECT_CONTROL_STATE Mos_Specific_CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF         MosUsage,
    GMM_CLIENT_CONTEXT          *pGmmClientContext)
{
   // Force convert to stream handle for wrapper
    return MosInterface::GetCachePolicyMemoryObject(pGmmClientContext, MosUsage);
}

//!
//! \brief    Get the L1 config
//! \details  Get the L1 config for cache policy
//! \param    MOS_HW_RESOURCE_DEF MosUsage
//!           [in] HW resource
//!           [in] Gmm client context
//! \return   uint8_t
//!           L1_CACHE_CONTROL
//!
uint8_t Mos_Specific_CachePolicyGetL1Config(
    MOS_HW_RESOURCE_DEF         MosUsage,
    GMM_CLIENT_CONTEXT          *pGmmClientContext)
{
    return 0;
}


//*-----------------------------------------------------------------------------
//| Purpose   : Loads library
//| Returns   : Instance to handle
//*-----------------------------------------------------------------------------
//!
//! \brief
//! \details
//! \param    const char  *pFileName
//! \return   HINSTANCE
//!
MOS_STATUS Mos_Specific_LoadLibrary(
    PMOS_INTERFACE              pOsInterface,
    PCCHAR                      pFileName,
    void                        **ppvModule)
{
    MOS_UNUSED(pOsInterface);

    return MosInterface::MosLoadLibrary(nullptr, pFileName, ppvModule);
}

//*-----------------------------------------------------------------------------
//| Purpose   : Frees library
//| Returns   : Result of the operation
//*-----------------------------------------------------------------------------
//!
//! \brief
//! \details
//! \param    HINSTANCE hInstance
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_FreeLibrary(
    void  *hInstance)
{
    return MosInterface::MosFreeLibrary(hInstance);
}

//!
//! \brief    Determines if the GPU Hung
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   int32_t
//!           Return if the GPU Hung
//!
int32_t Mos_Specific_IsGPUHung(
    PMOS_INTERFACE              pOsInterface)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Mos_Specific_IsGPUHung: pOsInterface == NULL");
        return false;
    }

    return MosInterface::IsGPUHung(pOsInterface->osStreamState);
}

uint64_t Mos_Specific_GetAuxTableBaseAddr(
    PMOS_INTERFACE              osInterface)
{
    if (osInterface == nullptr)
    {
        MOS_OS_NORMALMESSAGE("Invalid osInterface");
        return 0;
    }
    return MosInterface::GetAuxTableBaseAddr(osInterface->osStreamState);
}

//!
//! \brief  Get gpu context priority from KMD
//! \param  [in] pOsInterface
//!         Pointer to OS interface
//! \param  [out] pPriority
//!         the priority of the  current context.
//!
void Mos_Specific_GetGpuPriority(
        PMOS_INTERFACE              pOsInterface,
        int32_t*                    pPriority)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    return MosInterface::GetGpuPriority(pOsInterface->osStreamState, pPriority);
}

//!
//! \brief  Set gpu priority to KMD
//! \param  [in] pOsInterface
//!         Pointer to OS interface
//! \param  [in] priority
//!         the priority set to current context.
//!
void Mos_Specific_SetGpuPriority(
        PMOS_INTERFACE              pOsInterface,
        int32_t                     priority)
{
    if (!pOsInterface)
    {
        MOS_OS_ASSERTMESSAGE("nullptr");
        return;
    }
    return MosInterface::SetGpuPriority(pOsInterface->osStreamState, priority);
}

MediaUserSettingSharedPtr Mos_Specific_GetUserSettingInstance(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_FUNCTION_ENTER;
    if (!osInterface)
    {
        MOS_OS_ASSERTMESSAGE("Invalid mosContext ptr");
        return nullptr;
    }

    return MosInterface::MosGetUserSettingInstance(osInterface->osStreamState);
}

static MOS_STATUS Mos_Specific_InitInterface_Ve(
    PMOS_INTERFACE osInterface)
{
    PLATFORM                            Platform;
    MOS_STATUS                          eStatus;
    MOS_STATUS                          eStatusUserFeature;
    uint32_t                            regValue = 0;
    MediaUserSettingSharedPtr           userSettingPtr = nullptr;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;
    eStatusUserFeature = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);

    // Get platform information
    memset(&Platform, 0, sizeof(PLATFORM));
    if (!Mos_Solo_IsEnabled(nullptr))
    {
        osInterface->pfnGetPlatform(osInterface, &Platform);
    }

    {
        //keep this as false until VE is enabled by all media components
        osInterface->bSupportVirtualEngine = false;
        osInterface->bUseHwSemaForResSyncInVE = false;
        osInterface->pVEInterf = nullptr;
        osInterface->VEEnable = false;

        auto skuTable = osInterface->pfnGetSkuTable(osInterface);
        MOS_OS_CHK_NULL_RETURN(skuTable);
        if (MEDIA_IS_SKU(skuTable, FtrGucSubmission))
        {
            osInterface->bGucSubmission = true;
        }

        //Read Scalable/Legacy Decode mode on Gen11+
        //1:by default for scalable decode mode
        //0:for legacy decode mode
        regValue = 0;
        eStatusUserFeature = ReadUserSetting(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_HCP_SCALABILITY_DECODE,
            MediaUserSetting::Group::Device);

        osInterface->bHcpDecScalabilityMode = regValue ? MOS_SCALABILITY_ENABLE_MODE_DEFAULT : MOS_SCALABILITY_ENABLE_MODE_FALSE;
        if(osInterface->bHcpDecScalabilityMode
            && (eStatusUserFeature == MOS_STATUS_SUCCESS))
        {
            //user's value to enable scalability
            osInterface->bHcpDecScalabilityMode = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE;
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        osInterface->frameSplit                  = false;
        ReadUserSetting(
            userSettingPtr,
            osInterface->frameSplit,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_LINUX_FRAME_SPLIT,
            MediaUserSetting::Group::Device);

        regValue = 0;
        ReadUserSettingForDebug(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_GUC_SUBMISSION,
            MediaUserSetting::Group::Device);
        osInterface->bGucSubmission = osInterface->bGucSubmission && regValue;

        // read the "Force VEBOX" user feature key
        // 0: not force
        regValue = 0;
        ReadUserSettingForDebug(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_FORCE_VEBOX,
            MediaUserSetting::Group::Device);
        osInterface->eForceVebox = (MOS_FORCE_VEBOX)regValue;

        //KMD Virtual Engine DebugOverride
        // 0: not Override
        ReadUserSettingForDebug(
            userSettingPtr,
            osInterface->bEnableDbgOvrdInVE,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VE_DEBUG_OVERRIDE,
            MediaUserSetting::Group::Device);
#endif

        // UMD Vebox Virtual Engine Scalability Mode
        // 0: disable. can set to 1 only when KMD VE is enabled.
        regValue = 0;
        eStatusUserFeature = ReadUserSetting(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE,
            MediaUserSetting::Group::Device);

        osInterface->bVeboxScalabilityMode = regValue ? MOS_SCALABILITY_ENABLE_MODE_DEFAULT : MOS_SCALABILITY_ENABLE_MODE_FALSE;

#if (_DEBUG || _RELEASE_INTERNAL)
        if(osInterface->bVeboxScalabilityMode
            && (eStatusUserFeature == MOS_STATUS_SUCCESS))
        {
            //user's value to enable scalability
            osInterface->bVeboxScalabilityMode = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE;

            if (osInterface->eForceVebox == MOS_FORCE_VEBOX_NONE)
            {
                osInterface->eForceVebox = MOS_FORCE_VEBOX_1_2;
            }
        }
        else if ((!osInterface->bVeboxScalabilityMode)
            && (eStatusUserFeature == MOS_STATUS_SUCCESS))
        {
            osInterface->eForceVebox        = MOS_FORCE_VEBOX_NONE;
        }

        // read the "Force VEBOX" user feature key
        // 0: not force
        regValue = 0;
        ReadUserSettingForDebug(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_FORCE_VEBOX,
            MediaUserSetting::Group::Device);
        osInterface->eForceVebox = (MOS_FORCE_VEBOX)regValue;
#endif
    }

    return eStatus;
}

bool Mos_Specific_IsMismatchOrderProgrammingSupported()
{
    return MosInterface::IsMismatchOrderProgrammingSupported();
}

bool Mos_Specific_pfnIsMultipleCodecDevicesInUse(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_FUNCTION_ENTER;

    return false;
}

//! \brief    Unified OS Initializes OS Linux Interface
//! \details  Linux OS Interface initilization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_CONTEXT pOsDriverContext
//!           [in] Pointer to Driver context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_InitInterface(
    PMOS_INTERFACE     pOsInterface,
    PMOS_CONTEXT       pOsDriverContext)
{
    PMOS_OS_CONTEXT                 pOsContext = nullptr;
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface = nullptr;
    MOS_STATUS                      eStatus = MOS_STATUS_UNKNOWN;
    MediaFeatureTable              *pSkuTable = nullptr;
    uint32_t                        dwResetCount = 0;
    int32_t                         ret = 0;
    bool                            modularizedGpuCtxEnabled = false;
    MediaUserSettingSharedPtr       userSettingPtr = nullptr;
    bool                            bSimIsActive = false;
    bool                            useCustomerValue = false;
    uint32_t                        regValue = 0;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;

    char *pMediaWatchdog = nullptr;
    long int watchdog = 0;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsDriverContext);
    userSettingPtr = pOsDriverContext->m_userSettingPtr;

    // Initialize OS interface functions
    pOsInterface->pfnSetGpuContext                          = Mos_Specific_SetGpuContext;
    pOsInterface->pfnSetGpuContextFromHandle                = Mos_Specific_SetGpuContextFromHandle;
    pOsInterface->pfnGetGpuContext                          = Mos_Specific_GetGpuContext;
    pOsInterface->pfnSetEncodePakContext                    = Mos_Specific_SetEncodePakContext;
    pOsInterface->pfnSetEncodeEncContext                    = Mos_Specific_SetEncodeEncContext;
    pOsInterface->pfnGetGmmClientContext                    = Mos_Specific_GetGmmClientContext;

    pOsInterface->pfnGetPlatform                            = Mos_Specific_GetPlatform;
    pOsInterface->pfnDestroy                                = Mos_Specific_Destroy;
    pOsInterface->pfnGetSkuTable                            = Mos_Specific_GetSkuTable;
    pOsInterface->pfnGetWaTable                             = Mos_Specific_GetWaTable;
    pOsInterface->pfnGetGtSystemInfo                        = Mos_Specific_GetGtSystemInfo;
    pOsInterface->pfnGetMediaEngineInfo                     = Mos_Specific_GetMediaEngineInfo;
    pOsInterface->pfnResetOsStates                          = Mos_Specific_ResetOsStates;
    pOsInterface->pfnAllocateResource                       = Mos_Specific_AllocateResource;
    pOsInterface->pfnGetResourceInfo                        = Mos_Specific_GetResourceInfo;
    pOsInterface->pfnFreeResource                           = Mos_Specific_FreeResource;
    pOsInterface->pfnFreeResourceWithFlag                   = Mos_Specific_FreeResourceWithFlag;
    pOsInterface->pfnLockSyncRequest                        = Mos_Specific_LockSyncRequest;
    pOsInterface->pfnLockResource                           = Mos_Specific_LockResource;
    pOsInterface->pfnUnlockResource                         = Mos_Specific_UnlockResource;
    pOsInterface->pfnDecompResource                         = Mos_Specific_DecompResource;
    pOsInterface->pfnSetDecompSyncRes                       = Mos_Specific_SetDecompSyncRes;
    pOsInterface->pfnDoubleBufferCopyResource               = Mos_Specific_DoubleBufferCopyResource;
    pOsInterface->pfnMediaCopyResource2D                    = Mos_Specific_MediaCopyResource2D;
    pOsInterface->pfnGetMosContext                          = Mos_Specific_GetMosContext;
    pOsInterface->pfnUpdateResourceUsageType                = Mos_Specific_UpdateResourceUsageType;
    pOsInterface->pfnRegisterResource                       = Mos_Specific_RegisterResource;
    pOsInterface->pfnResetResourceAllocationIndex           = Mos_Specific_ResetResourceAllocationIndex;
    pOsInterface->pfnGetResourceAllocationIndex             = Mos_Specific_GetResourceAllocationIndex;
    pOsInterface->pfnGetResourceGfxAddress                  = Mos_Specific_GetResourceGfxAddress;
    pOsInterface->pfnGetCommandBuffer                       = Mos_Specific_GetCommandBuffer;
    pOsInterface->pfnResetCommandBuffer                     = Mos_Specific_ResetCommandBuffer;
    pOsInterface->pfnReturnCommandBuffer                    = Mos_Specific_ReturnCommandBuffer;
    pOsInterface->pfnSubmitCommandBuffer                    = Mos_Specific_SubmitCommandBuffer;
    pOsInterface->pfnVerifyCommandBufferSize                = Mos_Specific_VerifyCommandBufferSize;
    pOsInterface->pfnResizeCommandBufferAndPatchList        = Mos_Specific_ResizeCommandBufferAndPatchList;
    pOsInterface->pfnSetPerfTag                             = Mos_Specific_SetPerfTag;
    pOsInterface->pfnResetPerfBufferID                      = Mos_Specific_ResetPerfBufferID;
    pOsInterface->pfnIncPerfFrameID                         = Mos_Specific_IncPerfFrameID;
    pOsInterface->pfnIncPerfBufferID                        = Mos_Specific_IncPerfBufferID;
    pOsInterface->pfnGetPerfTag                             = Mos_Specific_GetPerfTag;
    pOsInterface->pfnSetPerfHybridKernelID                  = Mos_Specific_SetPerfHybridKernelID;
    pOsInterface->pfnIsPerfTagSet                           = Mos_Specific_IsPerfTagSet;
    pOsInterface->pfnSetIndirectStateSize                   = Mos_Specific_SetIndirectStateSize;
    pOsInterface->pfnGetIndirectState                       = Mos_Specific_GetIndirectState;
    pOsInterface->pfnGetIndirectStatePointer                = Mos_Specific_GetIndirectStatePointer;
    pOsInterface->pfnSetPatchEntry                          = Mos_Specific_SetPatchEntry;
    pOsInterface->pfnGetInterfaceVersion                    = Mos_Specific_GetInterfaceVersion;

    pOsInterface->pfnLoadLibrary                            = Mos_Specific_LoadLibrary;
    pOsInterface->pfnFreeLibrary                            = Mos_Specific_FreeLibrary;

    //GPU context and synchronization functions
    pOsInterface->pfnCreateGpuContext                       = Mos_Specific_CreateGpuContext;
    pOsInterface->pfnCreateGpuComputeContext                = Mos_Specific_CreateGpuComputeContext;
    pOsInterface->pfnDestroyGpuContext                      = Mos_Specific_DestroyGpuContext;
    pOsInterface->pfnDestroyGpuComputeContext               = Mos_Specific_DestroyGpuComputeContext;
    pOsInterface->pfnIsGpuContextValid                      = Mos_Specific_IsGpuContextValid;
    pOsInterface->pfnSyncOnResource                         = Mos_Specific_SyncOnResource;
    pOsInterface->pfnGetGpuStatusBufferResource             = Mos_Specific_GetGpuStatusBufferResource;
    pOsInterface->pfnGetGpuStatusTagOffset                  = Mos_Specific_GetGpuStatusTagOffset;
    pOsInterface->pfnGetGpuStatusTag                        = Mos_Specific_GetGpuStatusTag;
    pOsInterface->pfnIncrementGpuStatusTag                  = Mos_Specific_IncrementGpuStatusTag;
    pOsInterface->pfnGetGpuStatusSyncTag                    = Mos_Specific_GetGpuStatusSyncTag;
    pOsInterface->pfnSetResourceSyncTag                     = Mos_Specific_SetResourceSyncTag;
    pOsInterface->pfnPerformOverlaySync                     = Mos_Specific_PerformOverlaySync;
    pOsInterface->pfnWaitAllCmdCompletion                   = Mos_Specific_WaitAllCmdCompletion_Os;
    pOsInterface->pfnResourceWait                           = Mos_Specific_ResourceWait;

    pOsInterface->pfnCachePolicyGetMemoryObject             = Mos_Specific_CachePolicyGetMemoryObject;
    pOsInterface->pfnVerifyPatchListSize                    = Mos_Specific_VerifyPatchListSize;
    pOsInterface->pfnGetMemoryCompressionMode               = Mos_Specific_GetMemoryCompressionMode;
    pOsInterface->pfnSetMemoryCompressionMode               = Mos_Specific_SetMemoryCompressionMode;
    pOsInterface->pfnSetMemoryCompressionHint               = Mos_Specific_SetMemoryCompressionHint;
    pOsInterface->pfnGetMemoryCompressionFormat             = Mos_Specific_GetMemoryCompressionFormat;
    pOsInterface->pfnCreateVideoNodeAssociation             = Mos_Specific_CreateVideoNodeAssociation;
    pOsInterface->pfnDestroyVideoNodeAssociation            = Mos_Specific_DestroyVideoNodeAssociation;
    pOsInterface->pfnGetVdboxNodeId                         = Mos_Specific_GetVdboxNodeId;

    pOsInterface->pfnGetNullHWRenderFlags                   = Mos_Specific_GetNullHWRenderFlags;
    pOsInterface->pfnSetCmdBufferDebugInfo                  = Mos_Specific_SetCmdBufferDebugInfo;
    pOsInterface->pfnGetCmdBufferDebugInfo                  = Mos_Specific_GetCmdBufferDebugInfo;

    pOsInterface->pfnRegisterBBCompleteNotifyEvent          = Mos_Specific_RegisterBBCompleteNotifyEvent;
    pOsInterface->pfnWaitForBBCompleteNotifyEvent           = Mos_Specific_WaitForBBCompleteNotifyEvent;
    pOsInterface->pfnCachePolicyGetMemoryObject             = Mos_Specific_CachePolicyGetMemoryObject;
    pOsInterface->pfnCachePolicyGetL1Config                 = Mos_Specific_CachePolicyGetL1Config;
    pOsInterface->pfnSkipResourceSync                       = Mos_Specific_SkipResourceSync;
    pOsInterface->pfnIsGPUHung                              = Mos_Specific_IsGPUHung;
    pOsInterface->pfnGetAuxTableBaseAddr                    = Mos_Specific_GetAuxTableBaseAddr;
    pOsInterface->pfnGetResourceIndex                       = Mos_Specific_GetResourceIndex;
    pOsInterface->pfnGetGpuPriority                         = Mos_Specific_GetGpuPriority;
    pOsInterface->pfnSetGpuPriority                         = Mos_Specific_SetGpuPriority;
    pOsInterface->pfnIsSetMarkerEnabled                     = Mos_Specific_IsSetMarkerEnabled;
    pOsInterface->pfnGetMarkerResource                      = Mos_Specific_GetMarkerResource;

    pOsInterface->pfnSetGpuContextHandle                    = Mos_Specific_SetGpuContextHandle;
    pOsInterface->pfnGetGpuContextbyHandle                  = Mos_Specific_GetGpuContextbyHandle;

    pOsInterface->pfnGetUserSettingInstance                 = Mos_Specific_GetUserSettingInstance;

    pOsInterface->pfnIsMismatchOrderProgrammingSupported    = Mos_Specific_IsMismatchOrderProgrammingSupported;

    pOsInterface->pfnIsMultipleCodecDevicesInUse            = Mos_Specific_pfnIsMultipleCodecDevicesInUse;

    pOsContext              = nullptr;
    pOsUserFeatureInterface = (PMOS_USER_FEATURE_INTERFACE)&pOsInterface->UserFeatureInterface;

    MOS_OS_NORMALMESSAGE("mm:Mos_Specific_InitInterface called.");

    pOsInterface->modularizedGpuCtxEnabled  = true;
    pOsInterface->veDefaultEnable           = true;
    pOsInterface->phasedSubmission          = true;
    pOsInterface->apoMosEnabled             = true;

    pOsInterface->streamStateIniter = true;
    MOS_OS_CHK_STATUS(MosInterface::CreateOsStreamState(
        &pOsInterface->osStreamState,
        (MOS_DEVICE_HANDLE)pOsDriverContext->m_osDeviceContext,
        (MOS_INTERFACE_HANDLE)pOsInterface,
        pOsInterface->Component,
        pOsDriverContext));

    // Set interface functions for legacy HAL
    pOsContext                          = (PMOS_OS_CONTEXT)pOsInterface->osStreamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(pOsContext);

    pOsContext->GetGPUTag                  = Linux_GetGPUTag;


    if (pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        OsContext *osContextPtr = OsContext::GetOsContextObject();
        if (osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        pOsInterface->osContextPtr = osContextPtr;

        if (pOsInterface->osContextPtr->GetOsContextValid() == false)
        {
            eStatus = pOsInterface->osContextPtr->Init(pOsDriverContext);
            if (MOS_STATUS_SUCCESS != eStatus)
            {
                MOS_OS_ASSERTMESSAGE("Unable to initialize MODS context.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }
    }
    else
    {
        pOsContext->pGmmClientContext = pOsDriverContext->pGmmClientContext;
    }

#if MOS_MEDIASOLO_SUPPORTED
    if (pOsInterface->bSoloInUse)
    {
        bSimIsActive = pOsInterface->bSimIsActive;
        useCustomerValue = true;
    }
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
    ReadUserSettingForDebug(
        userSettingPtr,
        pOsInterface->bSimIsActive,
        __MEDIA_USER_FEATURE_VALUE_SIM_ENABLE,
        MediaUserSetting::Group::Device,
        bSimIsActive,
        useCustomerValue);
#endif
    pOsInterface->pOsContext                  = pOsContext;

    pOsInterface->bUsesPatchList              = true;
    pOsInterface->bUsesGfxAddress             = false;
    pOsInterface->bNoParsingAssistanceInKmd   = true;
    pOsInterface->bUsesCmdBufHeaderInResize   = false;
    pOsInterface->bUsesCmdBufHeader           = false;
    pOsInterface->dwNumNalUnitBytesIncluded   = MOS_NAL_UNIT_LENGTH - MOS_NAL_UNIT_STARTCODE_LENGTH;

    pOsInterface->bInlineCodecStatusUpdate    = true;
    pOsInterface->bAllowExtraPatchToSameLoc   = false;

    pOsUserFeatureInterface->bIsNotificationSupported   = false;
    pOsUserFeatureInterface->pOsInterface               = pOsInterface;

    pOsUserFeatureInterface->pfnEnableNotification      = MosUtilities::MosUserFeatureEnableNotification;
    pOsUserFeatureInterface->pfnDisableNotification     = MosUtilities::MosUserFeatureDisableNotification;
    pOsUserFeatureInterface->pfnParsePath               = MosUtilities::MosUserFeatureParsePath;

    // Init reset count for the context
    ret = mos_get_reset_stats(pOsInterface->pOsContext->intel_context, &dwResetCount, nullptr, nullptr);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        dwResetCount = 0;
    }
    pOsInterface->dwGPUResetCount   = dwResetCount;
    pOsInterface->dwGPUActiveBatch  = 0;
    pOsInterface->dwGPUPendingBatch = 0;

    // enable it on Linux
    pOsInterface->bMediaReset         = true;
    pOsInterface->umdMediaResetEnable = true;

    pMediaWatchdog = getenv("INTEL_MEDIA_RESET_WATCHDOG");
    if (pMediaWatchdog != nullptr)
    {
        watchdog = strtol(pMediaWatchdog, nullptr, 0);
        if (watchdog == 0)
        {
            pOsInterface->bMediaReset         = false;
            pOsInterface->umdMediaResetEnable = false;
        }
    }

    // initialize MOS_CP interface
    pOsInterface->osCpInterface = Create_MosCpInterface(pOsInterface);
    if (pOsInterface->osCpInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("fail to create osCpInterface.");
        return MOS_STATUS_UNKNOWN;
    }

    // Check SKU table to detect if simulation environment (HAS) is enabled
    pSkuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    MOS_OS_CHK_NULL(pSkuTable);

#if (_DEBUG || _RELEASE_INTERNAL)
    // read the "Force VDBOX" user feature key
    // 0: not force
    ReadUserSettingForDebug(
        userSettingPtr,
        pOsInterface->eForceVdbox,
        __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX,
        MediaUserSetting::Group::Device);

    // Force TileYf/Ys
    // 0: Tile Y  1: Tile Yf   2 Tile Ys
    ReadUserSettingForDebug(
        userSettingPtr,
        pOsInterface->dwForceTileYfYs,
        __MEDIA_USER_FEATURE_VALUE_FORCE_YFYS,
        MediaUserSetting::Group::Device);

    // Null HW Driver
    // 0: Disable
    ReadUserSettingForDebug(
        userSettingPtr,
        pOsInterface->NullHWAccelerationEnable.Value,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE,
        MediaUserSetting::Group::Device);

#endif // (_DEBUG || _RELEASE_INTERNAL)

#if MOS_MEDIASOLO_SUPPORTED
    Mos_Solo_Initialize(pOsInterface);
#endif // MOS_MEDIASOLO_SUPPORTED
    if (!pOsInterface->apoMosEnabled)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        // read the "Disable KMD Watchdog" user feature key
        regValue = 0;
        ReadUserSettingForDebug(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_DISABLE_KMD_WATCHDOG,
            MediaUserSetting::Group::Device);

        pOsContext->bDisableKmdWatchdog = regValue ? true : false;

        // read "Linux PerformanceTag Enable" user feature key
        regValue = 0;
        ReadUserSettingForDebug(
            userSettingPtr,
            regValue,
            __MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE,
            MediaUserSetting::Group::Device);

        pOsContext->uEnablePerfTag = regValue;
#endif
    }
    eStatus = Mos_Specific_InitInterface_Ve(pOsInterface);
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        goto finish;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    if( MOS_STATUS_SUCCESS != eStatus && nullptr != pOsContext )
    {
        MOS_Delete(pOsContext);
    }
    return eStatus;
}

//!
//! \brief    Convert to MOS tile type
//! \details  Convert from Linux to MOS tile type
//! \param    uint32_t type
//!           [in] tile type
//! \return   MOS_TILE_TYPE
//!           Return MOS tile type
//!
MOS_TILE_TYPE LinuxToMosTileType(uint32_t type)
{
    switch (type) {
        case I915_TILING_NONE:
            return MOS_TILE_LINEAR;
        case I915_TILING_X:
            return MOS_TILE_X;
        case I915_TILING_Y:
            return MOS_TILE_Y;
        default:
            return MOS_TILE_INVALID;
    }
};

//!
//! \brief    Get resource index
//! \details  Get resource index of MOS_RESOURCE
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to OS resource
//! \return   uint32_t
//!           Resource index
//!
uint32_t Mos_Specific_GetResourceIndex(
    PMOS_RESOURCE           osResource)
{
    return 0;
}

uint32_t Mos_Specific_GetResourcePitch(
    PMOS_RESOURCE               pOsResource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(pOsResource);

    return pOsResource->iPitch;
}

void Mos_Specific_SetResourceWidth(
    PMOS_RESOURCE               pOsResource,
    uint32_t                    dwWidth)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(pOsResource);

    pOsResource->iWidth = dwWidth;
}

void Mos_Specific_SetResourceFormat(
    PMOS_RESOURCE               pOsResource,
    MOS_FORMAT                  mosFormat)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(pOsResource);

    pOsResource->Format = mosFormat;
}

//!
//! \brief    Get SetMarker enabled flag
//! \details  Get SetMarker enabled flag from OsInterface
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   bool
//!           SetMarker enabled flag
//!
bool Mos_Specific_IsSetMarkerEnabled(
    PMOS_INTERFACE         pOsInterface)
{
    return false;
}

//!
//! \brief    Get SetMarker resource address
//! \details  Get SetMarker resource address from OsInterface
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   PMOS_RESOURCE
//!           SetMarker resource address
//!
PMOS_RESOURCE Mos_Specific_GetMarkerResource(
    PMOS_INTERFACE         pOsInterface)
{
    return 0;
}

//!
//! \brief    Get TimeStamp frequency base
//! \details  Get TimeStamp frequency base from OsInterface
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   uint32_t
//!           time stamp frequency base
//!
uint32_t Mos_Specific_GetTsFrequency(PMOS_INTERFACE osInterface)
{
    int32_t freq = 0;
    drm_i915_getparam_t gp;
    MOS_ZeroMemory(&gp, sizeof(gp));
    gp.param = I915_PARAM_CS_TIMESTAMP_FREQUENCY;
    gp.value = &freq;
    int ret = drmIoctl(osInterface->pOsContext->fd, DRM_IOCTL_I915_GETPARAM, &gp);
    if(ret == 0)
    {
        return freq;
    }
    else
    {
        // fail to query it from KMD
        return 0;
    }
}

MOS_USER_FEATURE_KEY_PATH_INFO *Mos_GetDeviceUfPathInfo(
    PMOS_CONTEXT mosContext)
{
    MOS_UNUSED(mosContext);
    //NOT IMPLEMENTED 
    return nullptr;
}
