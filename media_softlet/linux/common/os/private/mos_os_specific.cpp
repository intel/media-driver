/*
* Copyright (c) 2022-2023, Intel Corporation
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
#include <unistd.h>
#include <cstdlib>
#include <dlfcn.h>

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_resource_defs.h"
#include "hwinfo_linux.h"

#include "mos_graphicsresource_next.h"
#include "mos_context_specific_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_gpucontextmgr_next.h"
#include "mos_interface.h"

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"


#include "mos_os_virtualengine_singlepipe_next.h"
#include "mos_os_virtualengine_scalability_next.h"

#include "memory_policy_manager.h"
#include "mos_oca_interface_specific.h"
#include "mos_os_next.h"
#include "mos_os_cp_interface_specific.h"

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
//! \param    PMOS_RESOURCE context
//!           [in] Pointer to OS context
//! \return   void
//!           Return NONE
//!
void Mos_Specific_ClearGpuContext(MOS_CONTEXT *context)
{
    int32_t iLoop = 0;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(context);

    for (iLoop = 0; iLoop < MOS_GPU_CONTEXT_MAX; iLoop++)
    {
        if (context->OsGpuContext[iLoop].pCB != nullptr)
        {
            MOS_FreeMemory(context->OsGpuContext[iLoop].pCB);
            context->OsGpuContext[iLoop].pCB = nullptr;
        }

        if (context->OsGpuContext[iLoop].pAllocationList != nullptr)
        {
            MOS_FreeMemory(context->OsGpuContext[iLoop].pAllocationList);
            context->OsGpuContext[iLoop].pAllocationList = nullptr;
        }

        if (context->OsGpuContext[iLoop].pPatchLocationList)
        {
            MOS_FreeMemory(context->OsGpuContext[iLoop].pPatchLocationList);
            context->OsGpuContext[iLoop].pPatchLocationList = nullptr;
        }

        if (context->OsGpuContext[iLoop].pResources != nullptr)
        {
            MOS_FreeMemory(context->OsGpuContext[iLoop].pResources);
            context->OsGpuContext[iLoop].pResources = nullptr;
        }

        if (context->OsGpuContext[iLoop].pbWriteMode != nullptr)
        {
            MOS_FreeMemory(context->OsGpuContext[iLoop].pbWriteMode);
            context->OsGpuContext[iLoop].pbWriteMode = nullptr;
        }

        context->OsGpuContext[iLoop].uiMaxNumAllocations    = 0;
        context->OsGpuContext[iLoop].uiMaxPatchLocationsize = 0;
    }
}

//!
//! \brief    Unified OS get command buffer
//! \details  Return the pointer to the next available space in Cmd Buffer
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS Context
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [out] Pointer to Command Buffer
//! \param    int32_t iSize
//!           [out] Size of command in bytes
//! \return   int32_t
//!           Return true is there is space
//!
int32_t Linux_GetCommandBuffer(
    PMOS_CONTEXT            osContext,
    PMOS_COMMAND_BUFFER     cmdBuffer,
    int32_t                 size)
{
    MOS_LINUX_BO           *cmd_bo = nullptr;

    if ( osContext == nullptr ||
         cmdBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Linux_GetCommandBuffer:osContext == nullptr || cmdBuffer == NULL");
        return false;
    }

    // Allocate the command buffer from GEM
    struct mos_drm_bo_alloc alloc;
    alloc.name = "MOS CmdBuf";
    alloc.size = size;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
    cmd_bo = mos_bo_alloc(osContext->bufmgr, &alloc);     // Align to page boundary
    MOS_OS_CHK_NULL_RETURN_VALUE(cmd_bo, false);

    //MOS_OS_NORMALMESSAGE("alloc CMB, bo is 0x%x.", cmd_bo);

    // Map command buffer to user virtual address
    if (mos_bo_map(cmd_bo,1) != 0) // Write enable set
    {
        MOS_OS_ASSERTMESSAGE("Mapping of command buffer failed.");
        mos_bo_unreference(cmd_bo);
        return false;
    }

    Mos_ResetResource(&cmdBuffer->OsResource);

    // Fill in resource information
    cmdBuffer->OsResource.Format = Format_Buffer;
    cmdBuffer->OsResource.iWidth = cmd_bo->size;
    cmdBuffer->OsResource.iHeight = 1;
    cmdBuffer->OsResource.iPitch = cmd_bo->size;
    cmdBuffer->OsResource.iSize =  cmdBuffer->OsResource.iPitch * cmdBuffer->OsResource.iHeight;
    cmdBuffer->OsResource.iCount = 1;
    cmdBuffer->OsResource.pData = (uint8_t*)cmd_bo->virt;
    cmdBuffer->OsResource.TileType = MOS_TILE_LINEAR;
    cmdBuffer->OsResource.bo = cmd_bo;
    cmdBuffer->OsResource.bMapped  = true;

    // for MOS wrapper to avoid memory leak
    cmdBuffer->OsResource.bConvertedFromDDIResource = true;

    cmdBuffer->pCmdBase    = (uint32_t*)cmd_bo->virt;
    cmdBuffer->pCmdPtr     = (uint32_t*)cmd_bo->virt;
    cmdBuffer->iOffset     = 0;
    cmdBuffer->iRemaining  = cmd_bo->size;
    cmdBuffer->iCmdIndex   = -1;
    cmdBuffer->iVdboxNodeIndex = MOS_VDBOX_NODE_INVALID;
    cmdBuffer->iVeboxNodeIndex = MOS_VEBOX_NODE_INVALID;
    cmdBuffer->is1stLvlBB = true;
    MOS_ZeroMemory(cmdBuffer->pCmdBase, cmd_bo->size);
    cmdBuffer->iSubmissionType = SUBMISSION_TYPE_SINGLE_PIPE;
    MOS_ZeroMemory(&cmdBuffer->Attributes, sizeof(cmdBuffer->Attributes));

    return true;
}

//!
//! \brief    Get unused command buffer space
//! \details  Return unused command buffer space
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS Context
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU context
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [out] Pointer to Command buffer
//! \return   void
//!
void Linux_ReturnCommandBuffer(
    PMOS_CONTEXT            osContext,
    MOS_GPU_CONTEXT         gpuContext,
    PMOS_COMMAND_BUFFER     cmdBuffer)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osContext);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(cmdBuffer);

    if (Mos_ResourceIsNull(&(cmdBuffer->OsResource)))
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter osContext or cmdBuffer.");
        return;
    }

    if (gpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return;
    }

    MOS_OS_GPU_CONTEXT &osGpuContext = osContext->OsGpuContext[gpuContext];

    osGpuContext.pCB->iOffset    = cmdBuffer->iOffset;
    osGpuContext.pCB->iRemaining = cmdBuffer->iRemaining;
    osGpuContext.pCB->pCmdPtr    = cmdBuffer->pCmdPtr;
    osGpuContext.pCB->iVdboxNodeIndex = cmdBuffer->iVdboxNodeIndex;
    osGpuContext.pCB->iVeboxNodeIndex = cmdBuffer->iVeboxNodeIndex;
    osGpuContext.pCB->is1stLvlBB = cmdBuffer->is1stLvlBB;

    return;
}

//!
//! \brief    Flush Command Buffer
//! \details  Flush Command Buffer space
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS Context
//! \param    MOS_GPU_CONTEXT gpucontext
//!           [in] GPU context
//! \return   int32_t
//!           true if succeeded, false if failed or invalid parameters
//!
int32_t Linux_FlushCommandBuffer(
    PMOS_CONTEXT           osContext,
    MOS_GPU_CONTEXT        gpucontext)
{
    PCOMMAND_BUFFER currCB;

    MOS_OS_CHK_NULL_RETURN_VALUE(osContext, false);

    if (gpucontext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpucontext.");
        return false;
    }

    MOS_OS_GPU_CONTEXT &osGpuContext  = osContext->OsGpuContext[gpucontext];

    osGpuContext.uiCurrentNumPatchLocations = 0;

    // CB already active
    currCB = osGpuContext.pCurrentCB;
    MOS_OS_CHK_NULL_RETURN_VALUE(currCB, false);

    if (currCB->bActive)
    {
        return false;
    }

    currCB->bActive  = true;
    return true;
}

//!
//! \brief    Init command buffer pool
//! \details  Initilize the command buffer pool
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS Context
//! \return   void
//!
void Linux_InitCmdBufferPool(
    PMOS_CONTEXT   osContext)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osContext);

    MOS_ZeroMemory(&osContext->CmdBufferPool, sizeof(CMD_BUFFER_BO_POOL));
    osContext->CmdBufferPool.iFetch = 0;
}

//!
//! \brief    Wait and release command buffer
//! \details  Command buffer Wait and release
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS context structure
//! \param    int32_t index
//!           [in] Command buffer's index in Command buffer pool
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_WaitAndReleaseCmdBuffer(
    PMOS_CONTEXT   osContext,
    int32_t        index)
{
    MOS_LINUX_BO   *cmd_bo;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osContext);

    if (index < 0 || index >= MAX_CMD_BUF_NUM)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // According to the logic of CmdBufferPool now, the command buffer is used in a circular way.
    // The input index always points to the next(oldest) buffer after the latest(newest) buffer.
    // If the next buffer is not empty (!= nullptr), all the buffers in the pool will also be not empty.
    // So it's not necessary to check all buffers to see whether there's empty buffer.
    cmd_bo = osContext->CmdBufferPool.pCmd_bo[index];
    if (cmd_bo != nullptr)
    {
        mos_bo_wait_rendering(cmd_bo);
        mos_bo_unreference(cmd_bo);
        osContext->CmdBufferPool.pCmd_bo[index] = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Release command buffer pool
//! \details  Release command buffer pool until all of commands are finished.
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS context structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_ReleaseCmdBufferPool(PMOS_CONTEXT osContext)
{
    MOS_OS_FUNCTION_ENTER;

    for (int32_t i = 0; i < MAX_CMD_BUF_NUM; i++)
    {
        MOS_OS_CHK_STATUS_RETURN(Linux_WaitAndReleaseCmdBuffer(osContext, i));
    }
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Wait for the fetch command
//! \details  Wait for the fetch command bo until it is available
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS context structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_WaitForAvailableCmdBo(
    PMOS_CONTEXT   osContext)
{
    int32_t         index = 0;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osContext);

    index = osContext->CmdBufferPool.iFetch;
    MOS_OS_CHK_STATUS_RETURN(Linux_WaitAndReleaseCmdBuffer(osContext, index));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Insert command buffer
//! \details  Insert command buffer into pool
//! \param    PMOS_CONTEXT osContext
//!           [in] Pointer to OS context structure
//! \param    PMOS_COMMAND_BUFFER    cmdBuffer
//!           [in] Pointer to command buffer struct
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_InsertCmdBufferToPool(
    PMOS_CONTEXT           osContext,
    PMOS_COMMAND_BUFFER    cmdBuffer)
{
    int32_t         index = 0;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osContext);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);
    MOS_OS_CHK_STATUS_RETURN(Linux_WaitForAvailableCmdBo(osContext));

    index = osContext->CmdBufferPool.iFetch;

    osContext->CmdBufferPool.pCmd_bo[index] = cmdBuffer->OsResource.bo;
    cmdBuffer->iCmdIndex = index;

    osContext->CmdBufferPool.iFetch++;
    if (osContext->CmdBufferPool.iFetch >= MAX_CMD_BUF_NUM)
    {
        osContext->CmdBufferPool.iFetch     = 0;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Initialize the GPU Status Buffer
//! \details  Initialize the GPU Status Buffer
//! \param    MOS_CONTEXT * osContext
//!           [in, out] Pointer to OS context structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_InitGPUStatus(
    PMOS_CONTEXT    osContext)
{
    MOS_LINUX_BO    *bo     = nullptr;

    MOS_OS_CHK_NULL_RETURN(osContext);

    osContext->pGPUStatusBuffer      =
                (MOS_RESOURCE*)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * MOS_GPU_CONTEXT_MAX);

    MOS_OS_CHK_NULL_RETURN(osContext->pGPUStatusBuffer);

    // Allocate the command buffer from GEM
    struct mos_drm_bo_alloc alloc;
    alloc.name = "GPU Status Buffer";
    alloc.size = sizeof(MOS_GPU_STATUS_DATA)  * MOS_GPU_CONTEXT_MAX;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
    bo = mos_bo_alloc(osContext->bufmgr, &alloc);     // Align to page boundary
    if (bo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Allocation of GPU Status Buffer failed.");
        MOS_FreeMemAndSetNull(osContext->pGPUStatusBuffer);
        return MOS_STATUS_NO_SPACE;
    }

    // Map command buffer to user virtual address
    if (mos_bo_map(bo, 1) != 0) // Write enable set
    {
        MOS_OS_ASSERTMESSAGE("Mapping of GPU Status Buffer failed.");
        mos_bo_unreference(bo);
        MOS_FreeMemAndSetNull(osContext->pGPUStatusBuffer);
        return MOS_STATUS_INVALID_HANDLE;
    }

    Mos_ResetResource(osContext->pGPUStatusBuffer);

    // Fill in resource information
    osContext->pGPUStatusBuffer->Format   = Format_Buffer;
    osContext->pGPUStatusBuffer->iWidth   = bo->size;
    osContext->pGPUStatusBuffer->iHeight  = 1;
    osContext->pGPUStatusBuffer->iPitch   = bo->size;
    osContext->pGPUStatusBuffer->iCount   = 1;
    osContext->pGPUStatusBuffer->pData    = (uint8_t*)bo->virt;
    osContext->pGPUStatusBuffer->TileType = MOS_TILE_LINEAR;
    osContext->pGPUStatusBuffer->bo       = bo;
    osContext->pGPUStatusBuffer->bMapped  = true;

    MOS_ZeroMemory(osContext->pGPUStatusBuffer->pData, bo->size);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Release the GPU Status Buffer
//! \details  Release the GPU Status Buffer
//! \param    MOS_CONTEXT * osContext
//!           [in, out] Pointer to OS context structure
//! \return   void
//!
void Linux_ReleaseGPUStatus(
    PMOS_CONTEXT    osContext)
{
    MOS_LINUX_BO    *bo = nullptr;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osContext);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osContext->pGPUStatusBuffer);

    bo = osContext->pGPUStatusBuffer->bo;
    if (bo != nullptr)
    {
        mos_bo_unmap(bo);
        mos_bo_wait_rendering(bo);
        mos_bo_unreference(bo);
    }
    osContext->pGPUStatusBuffer->bo = nullptr;

    MOS_FreeMemAndSetNull(osContext->pGPUStatusBuffer);
}

//!
//! \brief    Get GPU status tag for the given GPU context
//! \details  Get GPU status tag for the given GPU context
//! \param    MOS_CONTEXT * osContext
//!           [in] Pointer to OS context structure
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in, out] GPU Context
//! \return   uint32_t
//!           GPU status tag
//!
uint32_t Linux_GetGpuCtxBufferTag(
    PMOS_CONTEXT    osContext,
    MOS_GPU_CONTEXT gpuContext)
{
    MOS_OS_CHK_NULL_RETURN_VALUE(osContext, 0);

    if (gpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return 0;
    }

    return osContext->OsGpuContext[gpuContext].uiGPUStatusTag;
}

//!
//! \brief    Increment GPU status tag for the given GPU context
//! \details  Increment GPU status tag for the given GPU context
//! \param    MOS_CONTEXT * osContext
//!           [in] Pointer to OS context structure
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   void
//!
void Linux_IncGpuCtxBufferTag(
    PMOS_CONTEXT    osContext,
    MOS_GPU_CONTEXT gpuContext)
{
    uint32_t uiGPUStatusTag;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osContext);

    if (gpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return;
    }

    uiGPUStatusTag = osContext->OsGpuContext[gpuContext].uiGPUStatusTag;

    osContext->OsGpuContext[gpuContext].uiGPUStatusTag = uiGPUStatusTag % UINT_MAX + 1;
    if (osContext->OsGpuContext[gpuContext].uiGPUStatusTag == 0)
    {
        osContext->OsGpuContext[gpuContext].uiGPUStatusTag = 1;
    }
}

//!
//! \brief    Get Buffer Type
//! \details  Returns the type of buffer, 1D, 2D or volume
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to OS Resource
//! \return   GFX resource Type
//!
MOS_GFXRES_TYPE GetResType(PMOS_RESOURCE osResource)
{
    GMM_RESOURCE_INFO *gmmResourceInfo = nullptr;
    MOS_GFXRES_TYPE    resType          = MOS_GFXRES_INVALID;
    GMM_RESOURCE_TYPE  gmmResType       = RESOURCE_INVALID;

    MOS_OS_CHK_NULL_RETURN_VALUE(osResource, MOS_GFXRES_INVALID);

    gmmResourceInfo = (GMM_RESOURCE_INFO *)osResource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN_VALUE(gmmResourceInfo, MOS_GFXRES_INVALID);
    gmmResType = gmmResourceInfo->GetResourceType();

    switch (gmmResType)
    {
    case RESOURCE_BUFFER:
        resType = MOS_GFXRES_BUFFER;
        break;
    case RESOURCE_3D:
        resType = MOS_GFXRES_VOLUME;
        break;
    case RESOURCE_2D:
        resType = MOS_GFXRES_2D;
        break;
    default:
        break;
    }
    return resType;
}

//!
//! \brief    Init Linux context
//! \details  Initialize the linux context
//! \param    MOS_OS_CONTEXT * context
//!           [in] Pointer to OS context structure
//! \param    PMOS_CONTEXT osDriverContext
//!           [in] Pointer to OS Driver context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_InitContext(
    MOS_OS_CONTEXT       *context,
    PMOS_CONTEXT         osDriverContext)
{
    MOS_STATUS eStatus;
    int32_t    i = -1;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    Linux_InitCmdBufferPool(context);

    // Initialize GPU Status Buffer
    eStatus = Linux_InitGPUStatus(context);
    MOS_OS_CHK_STATUS_RETURN(eStatus);

    for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        context->OsGpuContext[i].pStartCB            = nullptr;
        context->OsGpuContext[i].pCurrentCB          = nullptr;
        context->OsGpuContext[i].bCBFlushed          = true;
        context->OsGpuContext[i].uiCommandBufferSize = COMMAND_BUFFER_SIZE;
        context->OsGpuContext[i].pCB                 =
            (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));

        if (nullptr == context->OsGpuContext[i].pCB)
        {
            MOS_OS_ASSERTMESSAGE("No More Avaliable Memory");
            Mos_Specific_ClearGpuContext(context);
            return MOS_STATUS_NO_SPACE;
        }

        // each thread has its own GPU context, so do not need any lock as guarder here
        context->OsGpuContext[i].pAllocationList =
            (ALLOCATION_LIST*)MOS_AllocAndZeroMemory(sizeof(ALLOCATION_LIST) * ALLOCATIONLIST_SIZE);
        if (nullptr == context->OsGpuContext[i].pAllocationList)
        {
            MOS_OS_ASSERTMESSAGE("context->OsGpuContext[%d].pAllocationList malloc failed.", i);
            Mos_Specific_ClearGpuContext(context);
            return MOS_STATUS_NO_SPACE;
        }
        context->OsGpuContext[i].uiMaxNumAllocations = ALLOCATIONLIST_SIZE;

        context->OsGpuContext[i].pPatchLocationList =
            (PATCHLOCATIONLIST*)MOS_AllocAndZeroMemory(sizeof(PATCHLOCATIONLIST) * PATCHLOCATIONLIST_SIZE);
        if (nullptr == context->OsGpuContext[i].pPatchLocationList)
        {
            MOS_OS_ASSERTMESSAGE("context->OsGpuContext[%d].pPatchLocationList malloc failed.", i);
            Mos_Specific_ClearGpuContext(context);
            return MOS_STATUS_NO_SPACE;
        }
        context->OsGpuContext[i].uiMaxPatchLocationsize = PATCHLOCATIONLIST_SIZE;

        context->OsGpuContext[i].pResources    =
            (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * ALLOCATIONLIST_SIZE);
        if (nullptr == context->OsGpuContext[i].pResources)
        {
            MOS_OS_ASSERTMESSAGE("context->OsGpuContext[%d].pResources malloc failed.", i);
            Mos_Specific_ClearGpuContext(context);
            return MOS_STATUS_NO_SPACE;
        }

        context->OsGpuContext[i].pbWriteMode    =
            (int32_t*)MOS_AllocAndZeroMemory(sizeof(int32_t) * ALLOCATIONLIST_SIZE);
        if (nullptr == context->OsGpuContext[i].pbWriteMode)
        {
            MOS_OS_ASSERTMESSAGE("context->OsGpuContext[%d].pbWriteMode malloc failed.", i);
            Mos_Specific_ClearGpuContext(context);
            return MOS_STATUS_NO_SPACE;
        }

        context->OsGpuContext[i].uiGPUStatusTag = 1;
    }


    context->pTranscryptedKernels      = nullptr;
    context->uiTranscryptedKernelsSize = 0;

    // For Media Memory compression
    context->ppMediaMemDecompState     = osDriverContext->ppMediaMemDecompState;
    context->pfnMemoryDecompress       = osDriverContext->pfnMemoryDecompress;
    context->pfnMediaMemoryCopy        = osDriverContext->pfnMediaMemoryCopy;
    context->pfnMediaMemoryCopy2D      = osDriverContext->pfnMediaMemoryCopy2D;

    // Set interface functions
    return eStatus;
}

//============= PRIVATE FUNCTIONS <END>=========================================

//!
//! \brief    Set GPU context
//! \details  Set GPU context for the following rendering operations
//! \param    PMOS_INTERFACE * osInterface
//!           [in] Pointer to OS interface
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetGpuContext(
    PMOS_INTERFACE     osInterface,
    MOS_GPU_CONTEXT    gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (gpuContext >= MOS_GPU_CONTEXT_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Set GPU context handle
    osInterface->CurrentGpuContextOrdinal = gpuContext;

    if (Mos_Solo_IsEnabled(nullptr))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Set GPU context handle
    osInterface->CurrentGpuContextHandle = osInterface->m_GpuContextHandleMap[gpuContext];

    MOS_OS_CHK_STATUS_RETURN(MosInterface::SetGpuContext(
        osInterface->osStreamState,
        osInterface->m_GpuContextHandleMap[gpuContext]));

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

    if (Mos_Solo_IsEnabled(nullptr))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Set GPU context handle
    osInterface->CurrentGpuContextHandle = contextHandle;

    MOS_OS_CHK_STATUS_RETURN(
        MosInterface::SetGpuContext(osInterface->osStreamState,
                                    contextHandle));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get current GPU context
//! \details  Get current GPU context for the following rendering operations
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   MOS_GPU_CONTEXT
//!           Return current GPU context
//!
MOS_GPU_CONTEXT Mos_Specific_GetGpuContext(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, MOS_GPU_CONTEXT_INVALID_HANDLE);

    return osInterface->CurrentGpuContextOrdinal;
}

//!
//! \brief    Set GPU context Handle
//! \details  Set GPU context handle for the following rendering operations
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    GPU_CONTEXT_HANDLE gpuContextHandle
//!           [in] GPU Context Handle
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetGpuContextHandle(
    PMOS_INTERFACE     osInterface,
    GPU_CONTEXT_HANDLE gpuContextHandle,
    MOS_GPU_CONTEXT    gpuContext)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(osInterface);
    if (gpuContext >= MOS_GPU_CONTEXT_MAX)
    {
        MOS_OS_ASSERTMESSAGE("gpuContext %x >=   MOS_GPU_CONTEXT_MAX", gpuContext);
        return MOS_STATUS_INVALID_PARAMETER;
    }
    osInterface->m_GpuContextHandleMap[gpuContext] = gpuContextHandle;
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get GPU context pointer
//! \details  Get GPU context pointer
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    GPU_CONTEXT_HANDLE gpuContextHandle
//!           [in] GPU Context Handle
//! \return   void *
//!           a pointer to a gpu context
//!
void *Mos_Specific_GetGpuContextbyHandle(
    PMOS_INTERFACE     osInterface,
    GPU_CONTEXT_HANDLE gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, nullptr);

    return MosInterface::GetGpuContext(osInterface->osStreamState, gpuContextHandle);
}

//!
//! \brief    Get current GMM client context
//! \details  Get current GMM client context
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   GMM_CLIENT_CONTEXT
//!           Return current GMM client context
//!
GMM_CLIENT_CONTEXT *Mos_Specific_GetGmmClientContext(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, nullptr);

    return MosInterface::GetGmmClientContext(osInterface->osStreamState);
}

//!
//! \brief    Get Platform
//! \details  Get platform info
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PLATFORM pPlatform
//!           [out] Pointer to platform
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void Mos_Specific_GetPlatform(
    PMOS_INTERFACE          osInterface,
    PLATFORM                *pPlatform)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pPlatform);

    auto platform = MosInterface::GetPlatform(osInterface->osStreamState);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(platform);
    *pPlatform = *platform;
    return;
}

MOS_STATUS Mos_DestroyInterface(PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    MOS_STREAM_HANDLE streamState = osInterface->osStreamState;
    MOS_OS_CHK_NULL_RETURN(streamState);

    auto deviceContext = streamState->osDeviceContext;
    MOS_OS_CHK_NULL_RETURN(deviceContext);
    auto gpuContextMgr = deviceContext->GetGpuContextMgr();

    if (!Mos_Solo_IsEnabled((PMOS_CONTEXT)streamState->perStreamParameters) && gpuContextMgr)
    {
        for (uint32_t i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
        {
            if (osInterface->m_GpuContextHandleMap[i] != MOS_GPU_CONTEXT_INVALID_HANDLE)
            {
                auto gpuContext = gpuContextMgr->GetGpuContext(osInterface->m_GpuContextHandleMap[i]);
                if (gpuContext == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active gpuContextHandle");
                    continue;
                }
                gpuContextMgr->DestroyGpuContext(gpuContext);
                osInterface->m_GpuContextHandleMap[i] = MOS_GPU_CONTEXT_INVALID_HANDLE;
            }
        }
    }

    if (osInterface->osCpInterface)
    {
        Delete_MosCpInterface(osInterface->osCpInterface);
        osInterface->osCpInterface = nullptr;
    }

    PMOS_CONTEXT perStreamParameters = (PMOS_CONTEXT)streamState->perStreamParameters;

    if (perStreamParameters && perStreamParameters->bFreeContext)
    {
        perStreamParameters->m_skuTable.reset();
        perStreamParameters->m_waTable.reset();
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
            if (perStreamParameters->intel_context->vm_id != INVALID_VM)
            {
                mos_vm_destroy(perStreamParameters->intel_context->bufmgr, perStreamParameters->intel_context->vm_id);
                perStreamParameters->intel_context->vm_id = INVALID_VM;
            }
            mos_context_destroy(perStreamParameters->intel_context);
            perStreamParameters->intel_context = nullptr;
        }
        MOS_Delete(perStreamParameters);
        streamState->perStreamParameters = nullptr;
    }

    MosInterface::DestroyVirtualEngineState(streamState);
    MOS_FreeMemAndSetNull(osInterface->pVEInterf);

    MOS_OS_CHK_STATUS_RETURN(MosInterface::DestroyOsStreamState(streamState));
    osInterface->osStreamState = nullptr;
    return MOS_STATUS_SUCCESS;
}
//!
//! \brief    Destroys OS specific allocations
//! \details  Destroys OS specific allocations including destroying OS context
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    int32_t destroyVscVppDeviceTag
//!           [in] Destroy VscVppDeviceTagId Flag, no use in Linux
//! \return   void
//!
void Mos_Specific_Destroy(
    PMOS_INTERFACE osInterface,
    int32_t        destroyVscVppDeviceTag)
{
    MOS_UNUSED(destroyVscVppDeviceTag);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    MOS_STATUS status = Mos_DestroyInterface(osInterface);
    if (status != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Mos Destroy Interface failed.");
    }
    return;
}

//!
//! \brief    Gets the SKU table
//! \details  Gets the SKU table for the platform
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   MEDIA_FEATURE_TABLE *
//!           Returns the pointer to sku table
//!
MEDIA_FEATURE_TABLE *Mos_Specific_GetSkuTable(
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("osInterface found be nullptr.");
        return nullptr;
    }
    return MosInterface::GetSkuTable(osInterface->osStreamState);
}

//!
//! \brief    Gets the WA table
//! \details  Gets the WA table for the platform
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   MEDIA_WA_TABLE *
//!           Returns the pointer to WA table
//!
MEDIA_WA_TABLE *Mos_Specific_GetWaTable(
    PMOS_INTERFACE osInterface)
{
    if (osInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("osInterface found be nullptr.");
        return nullptr;
    }
    return MosInterface::GetWaTable(osInterface->osStreamState);
}

//!
//! \brief    Gets the GT System Info
//! \details  Gets the GT System Info
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \return   MEDIA_SYSTEM_INFO *
//!           pointer to the GT System Info
//!
MEDIA_SYSTEM_INFO *Mos_Specific_GetGtSystemInfo(
    PMOS_INTERFACE osInterface)
{
    if (nullptr == osInterface)
    {
        MOS_OS_ASSERTMESSAGE("input parameter osInterface is NULL.");
        return  nullptr;
    }

    return MosInterface::GetGtSystemInfo(osInterface->osStreamState);
}

MOS_STATUS Mos_Specific_GetMediaEngineInfo(
    PMOS_INTERFACE      osInterface,
    MEDIA_ENGINE_INFO   &info)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMediaEngineInfo(osInterface->osStreamState, info));
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Resets OS States
//! \details  Resets OS States for linux
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   void
//!
void Mos_Specific_ResetOsStates(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_FUNCTION_ENTER;

    if (osInterface == nullptr)
    {
        return;
    }
    if (!Mos_Solo_IsEnabled(nullptr))
    {
        auto status = MosInterface::ResetCommandBuffer(osInterface->osStreamState, 0);
        if (MOS_FAILED(status))
        {
            MOS_OS_ASSERTMESSAGE("ResetCommandBuffer failed.");
        }
        return;
    }

    PMOS_OS_CONTEXT     osContext = osInterface->pOsContext;
    MOS_OS_GPU_CONTEXT  &osGpuContext = osContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];
    // Reset resource allocation
    osGpuContext.uiNumAllocations = 0;
    MOS_ZeroMemory(osGpuContext.pAllocationList, sizeof(ALLOCATION_LIST) * osGpuContext.uiMaxNumAllocations);
    osGpuContext.uiCurrentNumPatchLocations = 0;
    MOS_ZeroMemory(osGpuContext.pPatchLocationList, sizeof(PATCHLOCATIONLIST) * osGpuContext.uiMaxPatchLocationsize);
    osGpuContext.uiResCount = 0;

    MOS_ZeroMemory(osGpuContext.pResources, sizeof(MOS_RESOURCE) * osGpuContext.uiMaxNumAllocations);
    MOS_ZeroMemory(osGpuContext.pbWriteMode, sizeof(int32_t) * osGpuContext.uiMaxNumAllocations);

    if ((osGpuContext.bCBFlushed == true) && osGpuContext.pCB->OsResource.bo)
    {
        osGpuContext.pCB->OsResource.bo = nullptr;
    }

    return;
 }

//!
//! \brief    Allocate resource
//! \details  To Allocate Buffer, pass Format as Format_Buffer and set the iWidth as size of the buffer.
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_ALLOC_GFXRES_PARAMS params
//!           [in] Pointer to resource params
//! \param    PMOS_RESOURCE osResource
//!           [in/out] Pointer to OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_AllocateResource(
    PMOS_INTERFACE           osInterface,
    PMOS_ALLOC_GFXRES_PARAMS params,
#if MOS_MESSAGES_ENABLED
    PCCHAR                   functionName,
    PCCHAR                   filename,
    int32_t                  line,
#endif // MOS_MESSAGES_ENABLED
    PMOS_RESOURCE            osResource)
{
    MOS_STATUS  eStatus         = MOS_STATUS_UNKNOWN;
    bool        osContextValid  = false;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osResource);
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(osInterface->osStreamState);

    if(Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Mos_Solo_IsEnabled, Invalid resource allocation request");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    osResource->bConvertedFromDDIResource = false;
    params->bBypassMODImpl = false;
    osInterface->osStreamState->component = osInterface->Component;

    eStatus = MosInterface::AllocateResource(
        osInterface->osStreamState,
        params,
        osResource
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
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to input OS resource
//! \param    PMOS_SURFACE pResDetails
//!           [out] Pointer to output resource information details
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetResourceInfo(
    PMOS_INTERFACE              osInterface,
    PMOS_RESOURCE               osResource,
    PMOS_SURFACE                resDetails)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(resDetails);
    return MosInterface::GetResourceInfo(osInterface->osStreamState, osResource, *resDetails);
}

//!
//! \brief    Free the resource
//! \details  Free the allocated resource
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to input OS resource
//! \return   void
//!
void Mos_Specific_FreeResource(
    PMOS_INTERFACE   osInterface,
#if MOS_MESSAGES_ENABLED
    PCCHAR           functionName,
    PCCHAR           filename,
    int32_t          line,
#endif // MOS_MESSAGES_ENABLED
    PMOS_RESOURCE    osResource)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osResource);

    MosInterface::FreeResource(
        osInterface->osStreamState,
        osResource,
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
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE osResource
//!           [in/out] OS Resource to be freed
//! \param    uint32_t uiFlag
//!           [in] Flag to free resources. This one is useless on Linux, just for compatibility.
//! \return   void
//!
void Mos_Specific_FreeResourceWithFlag(
    PMOS_INTERFACE    osInterface,
    PMOS_RESOURCE     osResource,
#if MOS_MESSAGES_ENABLED
    PCCHAR            functionName,
    PCCHAR            filename,
    int32_t           line,
#endif // MOS_MESSAGES_ENABLED
    uint32_t          uiFlag)
{
    MOS_UNUSED(uiFlag);

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osResource);

#if MOS_MESSAGES_ENABLED
    Mos_Specific_FreeResource(osInterface, functionName, filename, line, osResource);
#else
    Mos_Specific_FreeResource(osInterface, osResource);
#endif // MOS_MESSAGES_ENABLED
}

//!
//! \brief    Locks Resource request
//! \details  Locks Resource request
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in/out] Resource object
//! \param    PMOS_LOCK_PARAMS lockFlags
//!           [in] Lock Flags
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_LockSyncRequest(
    PMOS_INTERFACE        osInterface,
    PMOS_RESOURCE         osResource,
    PMOS_LOCK_PARAMS      lockFlags)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(osResource);
    MOS_UNUSED(lockFlags);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Lock resource
//! \details  Lock allocated resource
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to input OS resource
//! \param    PMOS_LOCK_PARAMS lockFlags
//!           [in] Lock Flags - MOS_LOCKFLAG_* flags
//! \return   void *
//!
void  *Mos_Specific_LockResource(
    PMOS_INTERFACE     osInterface,
    PMOS_RESOURCE      osResource,
    PMOS_LOCK_PARAMS   lockFlags)
{
    MOS_OS_CONTEXT      *context = nullptr;

    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, nullptr);
    MOS_OS_CHK_NULL_RETURN_VALUE(osResource, nullptr);
    if(!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::LockMosResource(osInterface->osStreamState, osResource, lockFlags);
    }

    context = osInterface->pOsContext;
    MOS_OS_CHK_NULL_RETURN_VALUE(context, nullptr);
    MOS_OS_CHK_NULL_RETURN_VALUE(osResource->bo, nullptr);
    MOS_OS_CHK_NULL_RETURN_VALUE(osResource->pGmmResInfo, nullptr);

    {
        MOS_LINUX_BO *bo = osResource->bo;
        GMM_RESOURCE_FLAG gmmFlags = {};

        gmmFlags = osResource->pGmmResInfo->GetResFlags();

        // Do decompression for a compressed surface before lock
        if (!lockFlags->NoDecompress &&
            (((gmmFlags.Gpu.MMC || gmmFlags.Gpu.CCS) && gmmFlags.Gpu.UnifiedAuxSurface)||
             osResource->pGmmResInfo->IsMediaMemoryCompressed(0)))
        {
            if(context->pfnMemoryDecompress)
            {
                context->pfnMemoryDecompress(context, osResource);
            }
        }
        if(osResource->bMapped)
        {
            return osResource->pData;
        }

        if (osResource->TileType != MOS_TILE_LINEAR && !lockFlags->TiledAsTiled)
        {
            if (context->bUseSwSwizzling)
            {
                mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY&lockFlags->WriteOnly));
                osResource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
                if (osResource->pSystemShadow == nullptr)
                {
                    osResource->pSystemShadow = (uint8_t *)MOS_AllocMemory(bo->size);
                    MOS_OS_CHECK_CONDITION((osResource->pSystemShadow == nullptr), "Failed to allocate shadow surface", nullptr);
                }
                if (osResource->pSystemShadow)
                {
                    int32_t flags = context->bTileYFlag ? 0 : 1;
                    MOS_OS_CHECK_CONDITION((osResource->TileType != MOS_TILE_Y), "Unsupported tile type", nullptr);
                    MOS_OS_CHECK_CONDITION((bo->size <= 0 || osResource->iPitch <= 0), "Invalid BO size or pitch", nullptr);
                    Mos_SwizzleData((uint8_t*)bo->virt, osResource->pSystemShadow, 
                            MOS_TILE_Y, MOS_TILE_LINEAR, bo->size / osResource->iPitch, osResource->iPitch, flags);
                }
            }
            else
            {
                mos_bo_map_gtt(bo);
                osResource->MmapOperation = MOS_MMAP_OPERATION_MMAP_GTT;
            }
        }
        else if (lockFlags->Uncached)
        {
            mos_bo_map_wc(bo);
            osResource->MmapOperation = MOS_MMAP_OPERATION_MMAP_WC;
        }
        else
        {
            mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY&lockFlags->WriteOnly));
            osResource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
        }
        osResource->pData   = osResource->pSystemShadow ? osResource->pSystemShadow : (uint8_t*)bo->virt;
        osResource->bMapped = true;
    }
    return osResource->pData;
}

//!
//! \brief    Unlock resource
//! \details  Unlock the locked resource
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to input OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_UnlockResource(
    PMOS_INTERFACE       osInterface,
    PMOS_RESOURCE        osResource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    if(!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::UnlockMosResource(osInterface->osStreamState, osResource);
    }

    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);
    if(false == osResource->bMapped)
    {
        osResource->pData       = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    if(osResource->bo)
    {
        if (osResource->pSystemShadow)
        {
            int32_t flags = osInterface->pOsContext->bTileYFlag ? 0 : 1;
            Mos_SwizzleData(osResource->pSystemShadow, (uint8_t*)osResource->bo->virt, 
                    MOS_TILE_LINEAR, MOS_TILE_Y, osResource->bo->size / osResource->iPitch, osResource->iPitch, flags);
            MOS_FreeMemory(osResource->pSystemShadow);
            osResource->pSystemShadow = nullptr;
        }

        switch(osResource->MmapOperation)
        {
            case MOS_MMAP_OPERATION_MMAP_GTT:
                mos_bo_unmap_gtt(osResource->bo);
                break;
            case MOS_MMAP_OPERATION_MMAP_WC:
                mos_bo_unmap_wc(osResource->bo);
                break;
            case MOS_MMAP_OPERATION_MMAP:
                mos_bo_unmap(osResource->bo);
                break;
            default:
                MOS_OS_ASSERTMESSAGE("Invalid mmap operation type");
                break;
        }
        osResource->bo->virt = nullptr;
    }

    osResource->bMapped  = false;
    osResource->pData   = nullptr;
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Decompress Resource
//! \details  Decompress Resource
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in/out] Resource object
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_DecompResource(
    PMOS_INTERFACE        osInterface,
    PMOS_RESOURCE         osResource)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    return MosInterface::DecompResource(osInterface->osStreamState, osResource);
}

//!
//! \brief    Decompress and Copy Resource to Another Buffer
//! \details  Decompress and Copy Resource to Another Buffer
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS Interface structure
//! \param    PMOS_RESOURCE inputOsResource
//!           [in] Input Resource object
//! \param    PMOS_RESOURCE outputOsResource
//!           [out] output Resource object
//! \param    [in] outputCompressed
//!            true means apply compression on output surface, else output uncompressed surface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS Mos_Specific_DoubleBufferCopyResource(
    PMOS_INTERFACE        osInterface,
    PMOS_RESOURCE         inputOsResource,
    PMOS_RESOURCE         outputOsResource,
    bool                  outputCompressed)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::DoubleBufferCopyResource(osInterface->osStreamState, inputOsResource, outputOsResource, outputCompressed);
}

//!
//! \brief    Decompress and Copy Resource to Another Buffer
//! \details  Decompress and Copy Resource to Another Buffer
//! \param    PMOS_INTERFACE osInterface
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
    uint32_t              bpp,
    bool                  outputCompressed)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::MediaCopyResource2D(osInterface->osStreamState, inputOsResource, outputOsResource,
        copyWidth, copyHeight, bpp, outputCompressed);
}

//!
//! \brief    Get Mos Context
//! \details  Get Mos Context info
//! \param    PMOS_INTERFACE osInterface
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
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_PATCH_ENTRY_PARAMS params
//!           [in] patch entry params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetPatchEntry(
    PMOS_INTERFACE              osInterface,
    PMOS_PATCH_ENTRY_PARAMS     params)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(params);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (!params->bUpperBoundPatch && params->presResource)
    {
        uint32_t handle = params->presResource->bo?params->presResource->bo->handle:0;
        uint32_t eventData[] = {handle, params->uiResourceOffset,
                                params->uiPatchOffset, params->bWrite,
                                params->HwCommandType, params->forceDwordOffset,
                                params->patchType};
        MOS_TraceEventExt(EVENT_RESOURCE_PATCH, EVENT_TYPE_INFO2,
                          &eventData, sizeof(eventData),
                          nullptr, 0);
    }
#endif
    if (Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Invalid solo function call");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MosInterface::SetPatchEntry(osInterface->osStreamState, params);
}

//!
//! \brief    Update resource usage type
//! \details  update the resource usage for cache policy
//! \param    PMOS_RESOURCE osResource
//!           [in/out] Pointer to OS Resource
//! \param    MOS_HW_RESOURCE_DEF resUsageType
//!           [in] resosuce usage type
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_UpdateResourceUsageType(
    PMOS_RESOURCE           osResource,
    MOS_HW_RESOURCE_DEF     resUsageType)
{
    return MosInterface::UpdateResourceUsageType(osResource, resUsageType);
}

//!
//! \brief    Registers Resource
//! \details  Set the Allocation Index in OS resource structure
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE osResource
//!           [in/out] Pointer to OS Resource
//! \param    int32_t bWrite
//!           [in] Write Flag
//! \param    int32_t bWritebSetResourceSyncTag
//!           [in] Resource registration Flag, no use for Linux
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_RegisterResource (
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       osResource,
    int32_t             write,
    int32_t             writebSetResourceSyncTag)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Invalid solo function call");
        return MOS_STATUS_INVALID_PARAMETER;
    }

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
    auto dumper = GpuCmdResInfoDumpNext::GetInstance(osInterface->pOsContext);
    if (dumper)
    {
        dumper->StoreCmdResPtr(osInterface, (const void *)osResource);
    }
#endif  // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

    return MosInterface::RegisterResource(
        osInterface->osStreamState,
        osResource,
        write);
}

//!
//! \brief    Verify command buffer size
//! \details  Verifys the buffer to be used for rendering GPU commands is large enough
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t dwRequestedSize
//!           [in] Buffer size requested
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful (command buffer will be large enough to hold dwMaxSize)
//!           otherwise failed
//!
MOS_STATUS Mos_Specific_VerifyCommandBufferSize(
    PMOS_INTERFACE          osInterface,
    uint32_t                requestedSize,
    uint32_t                flags)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::VerifyCommandBufferSize(osInterface->osStreamState, 0, requestedSize, flags);
    }

    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);
    if (osInterface->CurrentGpuContextOrdinal == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("CurrentGpuContextOrdinal exceed max.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_OS_GPU_CONTEXT &osGpuContext = osInterface->pOsContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];

    if (osGpuContext.uiCommandBufferSize < requestedSize)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Resets Resource Allocation
//! \details  Resets Resource Allocation
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to input OS resource
//! \return   void
//!
void Mos_Specific_ResetResourceAllocationIndex (
    PMOS_INTERFACE   osInterface,
    PMOS_RESOURCE    osResource)
{
    int32_t i = 0;
    MOS_UNUSED(osInterface);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osResource);

    for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        osResource->iAllocationIndex[i] = MOS_INVALID_ALLOC_INDEX;
    }
}

//!
//! \brief    Get command buffer
//! \details  Retrieves buffer to be used for rendering GPU commands
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [out] Pointer to Command Buffer control structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetCommandBuffer(
    PMOS_INTERFACE          osInterface,
    PMOS_COMMAND_BUFFER     cmdBuffer,
    uint32_t                flags)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    if (!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::GetCommandBuffer(osInterface->osStreamState, cmdBuffer, flags);
    }

    MOS_STATUS          eStatus             = MOS_STATUS_SUCCESS;
    uint32_t            uiCommandBufferSize = 0;

    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);
    MOS_OS_CHK_NULL_RETURN(cmdBuffer);

    if (osInterface->CurrentGpuContextOrdinal == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Activate software context
    MOS_OS_GPU_CONTEXT &osGpuContext  = osInterface->pOsContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];
    uiCommandBufferSize = osGpuContext.uiCommandBufferSize;

    if (osGpuContext.bCBFlushed == true)
    {
        if (Linux_GetCommandBuffer(osInterface->pOsContext,
                cmdBuffer,
                uiCommandBufferSize))
        {
            MOS_OS_CHK_STATUS_RETURN(Linux_InsertCmdBufferToPool(osInterface->pOsContext, cmdBuffer));
            osGpuContext.bCBFlushed = false;
            eStatus                   = MOS_SecureMemcpy(osGpuContext.pCB, sizeof(MOS_COMMAND_BUFFER), cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
            MOS_OS_CHECK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "Failed to copy command buffer", eStatus);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Failed to activate command buffer.");
            return MOS_STATUS_UNKNOWN;
        }
    }

    MOS_OS_CHK_STATUS_RETURN(osInterface->pfnRegisterResource(osInterface, &(osGpuContext.pCB->OsResource), false, false));
    eStatus = MOS_SecureMemcpy(cmdBuffer, sizeof(MOS_COMMAND_BUFFER), osGpuContext.pCB, sizeof(MOS_COMMAND_BUFFER));
    MOS_OS_CHECK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "Failed to copy command buffer", eStatus);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set indirect state size
//! \details  Sets indirect state size to be used for rendering purposes
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t uSize
//!           [in] State size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetIndirectStateSize(
    PMOS_INTERFACE              osInterface,
    uint32_t                    size)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::SetupIndirectState(osInterface->osStreamState, size);
    }

    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);

    osInterface->pOsContext->uIndirectStateSize = size;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Set indirect state size
//! \details  Retrieves indirect state to be used for rendering purposes
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t *puOffset
//!           [out] Pointer to indirect buffer offset
//! \param    uint32_t *puSize
//!           [out] Pointer to indirect buffer size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetIndirectState(
    PMOS_INTERFACE           osInterface,
    uint32_t                *puOffset,
    uint32_t                *puSize)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(puOffset);
    MOS_OS_CHK_NULL_RETURN(puSize);

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        uint32_t offset  = 0;
        uint32_t size    = 0;
        auto     eStatus = MosInterface::GetIndirectState(osInterface->osStreamState, nullptr, offset, size);
        *puOffset        = offset;
        *puSize          = size;
        return eStatus;
    }

    if (osInterface->pOsContext)
    {
        MOS_OS_GPU_CONTEXT &osGpuContext = osInterface->pOsContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];

        *puOffset = osGpuContext.uiCommandBufferSize - osInterface->pOsContext->uIndirectStateSize;

        *puSize = osInterface->pOsContext->uIndirectStateSize;
    }
    return MOS_STATUS_SUCCESS;

}

//!
//! \brief    Get Resource Allocation Index
//! \details  Get Resource Allocation Index
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pResource
//!           [in] Pointer to input OS resource
//! \return   int32_t
//!           return the allocation index
//!
int32_t Mos_Specific_GetResourceAllocationIndex(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       resource)
{
    MOS_OS_FUNCTION_ENTER;

    if (!resource || !osInterface || osInterface->CurrentGpuContextHandle == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
       return MOS_INVALID_ALLOC_INDEX;
    }

    return(resource->iAllocationIndex[osInterface->CurrentGpuContextOrdinal]);
}

//!
//! \brief    Get Indirect State Pointer
//! \details  Get Indirect State Pointer
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    uint8_t **indirectState
//!           [out] Pointer to Indirect State Buffer
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetIndirectStatePointer(
    PMOS_INTERFACE      osInterface,
    uint8_t             **indirectState)
{
    MOS_OS_FUNCTION_ENTER;
    PMOS_OS_CONTEXT     osContext = nullptr;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(indirectState);

    uint32_t offset = 0;
    uint32_t size   = 0;
    return MosInterface::GetIndirectState(osInterface->osStreamState, indirectState, offset, size);
}

//!
//! \brief    Return command buffer space
//! \details  Return unused command buffer space
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [out] Pointer to Command Buffer control structure
//! \return   void
//!
void Mos_Specific_ReturnCommandBuffer(
    PMOS_INTERFACE          osInterface,
    PMOS_COMMAND_BUFFER     cmdBuffer,
    uint32_t                flags)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(cmdBuffer);

    // Need to ensure 4K extra padding for HW requirement.
    if (cmdBuffer && cmdBuffer->iRemaining < EXTRA_PADDING_NEEDED)
    {
        MOS_OS_ASSERTMESSAGE("Need to ensure 4K extra padding for HW requirement.");
    }

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        auto status = MosInterface::ReturnCommandBuffer(osInterface->osStreamState, cmdBuffer, flags);
        if (MOS_FAILED(status))
        {
            MOS_OS_ASSERTMESSAGE("ReturnCommandBuffer failed.");
        }
        return;
    }

    Linux_ReturnCommandBuffer(osInterface->pOsContext,osInterface->CurrentGpuContextOrdinal, cmdBuffer);
    return;
}

//!
//! \brief    Submit command buffer
//! \details  Submit the command buffer
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [in] Pointer to Command Buffer control structure
//! \param    int32_t bNullRendering
//!           [in] boolean null rendering
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SubmitCommandBuffer(
    PMOS_INTERFACE        osInterface,
    PMOS_COMMAND_BUFFER   cmdBuffer,
    int32_t               bNullRendering)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    return MosInterface::SubmitCommandBuffer(osInterface->osStreamState, cmdBuffer, bNullRendering);
}

//!
//! \brief    Get resource gfx address
//! \details  Get resource gfx address
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pResource
//!           [in] Pointer to resource
//! \return   uint64_t
//!           Return resource gfx address
//!
uint64_t Mos_Specific_GetResourceGfxAddress(
    PMOS_INTERFACE   osInterface,
    PMOS_RESOURCE    resource)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::GetResourceGfxAddress(osInterface->osStreamState, resource);
}

//!
//! \brief    Get Clear Color Address
//! \details  The clear color address
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE pResource
//!           [in] OS resource structure
//! \return   uint64_t
//!           The clear color address
//!
uint64_t Mos_Specific_GetResourceClearAddress(
    PMOS_INTERFACE pOsInterface,
    PMOS_RESOURCE  pResource)
{
    uint64_t ui64ClearColorAddress = 0;
    return ui64ClearColorAddress;
}

//!
//! \brief    Resizes the buffer to be used for rendering GPU commands
//! \details  return true if succeeded - command buffer will be large enough to hold dwMaxSize
//!           return false if failed or invalid parameters
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t dwRequestedCommandBufferSize
//!           [in] requested command buffer size
//! \param    uint32_t dwRequestedPatchListSize
//!           [in] requested patch list size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_ResizeCommandBufferAndPatchList(
    PMOS_INTERFACE          osInterface,
    uint32_t                dwRequestedCommandBufferSize,
    uint32_t                dwRequestedPatchListSize,
    uint32_t                dwFlags)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    if (!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::ResizeCommandBufferAndPatchList(osInterface->osStreamState, 0, dwRequestedCommandBufferSize, dwRequestedPatchListSize, dwFlags);
    }

    PMOS_CONTEXT            osContext = osInterface->pOsContext;
    MOS_OS_CHK_NULL_RETURN(osContext);

    MOS_OS_GPU_CONTEXT &osGpuContext  = osContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];

    // uiCommandBufferSize is used for allocate command buffer and submit command buffer, in this moment, command buffer has not allocated yet.
    // Linux KMD requires command buffer size align to 8 bytes, or it will not execute the commands.
    osGpuContext.uiCommandBufferSize = MOS_ALIGN_CEIL(dwRequestedCommandBufferSize, 8);

    if (dwRequestedPatchListSize > osGpuContext.uiMaxPatchLocationsize)
    {
        PPATCHLOCATIONLIST pNewPatchList = (PPATCHLOCATIONLIST)realloc(
            osGpuContext.pPatchLocationList,
            sizeof(PATCHLOCATIONLIST) * dwRequestedPatchListSize);
        MOS_OS_CHK_NULL_RETURN(pNewPatchList);

        osGpuContext.pPatchLocationList = pNewPatchList;

        // now zero the extended portion
        MOS_ZeroMemory(
            (osGpuContext.pPatchLocationList + osGpuContext.uiMaxPatchLocationsize),
            sizeof(PATCHLOCATIONLIST) * (dwRequestedPatchListSize - osGpuContext.uiMaxPatchLocationsize));
        osGpuContext.uiMaxPatchLocationsize = dwRequestedPatchListSize;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Create GPU context
//! \details  Create GPU context
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \param    MOS_GPU_NODE GpuNode
//!           [in] GPU Node
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_CreateGpuContext(
    PMOS_INTERFACE        osInterface,
    MOS_GPU_CONTEXT       mosGpuCxt,
    MOS_GPU_NODE          gpuNode,
    PMOS_GPUCTX_CREATOPTIONS createOption)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (mosGpuCxt >= MOS_GPU_CONTEXT_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_CHK_NULL_RETURN(createOption);
        if (gpuNode == MOS_GPU_NODE_3D && createOption->SSEUValue != 0)
        {
            struct drm_i915_gem_context_param_sseu sseu;
            MOS_ZeroMemory(&sseu, sizeof(sseu));
            sseu.engine.engine_class = I915_ENGINE_CLASS_RENDER;
            sseu.engine.engine_instance = 0;

            if (mos_get_context_param_sseu(osInterface->pOsContext->intel_context, &sseu))
            {
                MOS_OS_ASSERTMESSAGE("Failed to get sseu configuration.");
                return MOS_STATUS_UNKNOWN;
            };

            if (mos_hweight8(osInterface->pOsContext->intel_context, sseu.subslice_mask) > createOption->packed.SubSliceCount)
            {
                sseu.subslice_mask = mos_switch_off_n_bits(osInterface->pOsContext->intel_context, sseu.subslice_mask,
                        mos_hweight8(osInterface->pOsContext->intel_context, sseu.subslice_mask)-createOption->packed.SubSliceCount);
            }

            if (mos_set_context_param_sseu(osInterface->pOsContext->intel_context, sseu))
            {
                MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
                return MOS_STATUS_UNKNOWN;
            }
        }

        createOption->gpuNode = gpuNode;
        MOS_OS_CHK_NULL_RETURN(osInterface->osStreamState);
        // Update ctxBasedScheduling from legacy OsInterface
        osInterface->osStreamState->ctxBasedScheduling = osInterface->ctxBasedScheduling;
        if (osInterface->m_GpuContextHandleMap[mosGpuCxt] == MOS_GPU_CONTEXT_INVALID_HANDLE)
        {
            auto osDeviceContext = osInterface->osStreamState->osDeviceContext;
            MOS_OS_CHK_NULL_RETURN(osDeviceContext);
            auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
            MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
            auto cmdBufMgr = osDeviceContext->GetCmdBufferMgr();
            MOS_OS_CHK_NULL_RETURN(cmdBufMgr);

            auto gpuContext = gpuContextMgr->CreateGpuContext(gpuNode, cmdBufMgr);
            MOS_OS_CHK_NULL_RETURN(gpuContext);

            auto gpuContextSpecific  = static_cast<GpuContextSpecificNext *>(gpuContext);
            MOS_OS_CHK_NULL_RETURN(gpuContextSpecific);

            MOS_OS_CHK_STATUS_RETURN(gpuContextSpecific->Init(gpuContextMgr->GetOsContext(), osInterface->osStreamState, createOption));
            gpuContextSpecific->SetGpuContext(mosGpuCxt);
            osInterface->m_GpuContextHandleMap[mosGpuCxt] = gpuContextSpecific->GetGpuContextHandle();
        }

        return MOS_STATUS_SUCCESS;

    }

    MOS_UNUSED(osInterface);
    MOS_UNUSED(mosGpuCxt);
    MOS_UNUSED(gpuNode);
    MOS_UNUSED(createOption);
    return MOS_STATUS_SUCCESS;
}

GPU_CONTEXT_HANDLE
Mos_Specific_CreateGpuComputeContext(MOS_INTERFACE   *osInterface,
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

    if (!Mos_Solo_IsEnabled(nullptr))
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
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_DestroyGpuContext(
    PMOS_INTERFACE        osInterface,
    MOS_GPU_CONTEXT       mosGpuCxt)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);

    if (mosGpuCxt >= MOS_GPU_CONTEXT_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        auto status = MosInterface::DestroyGpuContext(osInterface->osStreamState, osInterface->m_GpuContextHandleMap[mosGpuCxt]);
        osInterface->m_GpuContextHandleMap[mosGpuCxt] = MOS_GPU_CONTEXT_INVALID_HANDLE;
        return status;
    }

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(osInterface);
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
    if (Mos_Solo_IsEnabled(nullptr))
    {
        return MOS_STATUS_SUCCESS;
    }

    GPU_CONTEXT_HANDLE iGpuContextHandle = osInterface->m_GpuContextHandleMap[MOS_GPU_CONTEXT_CM_COMPUTE];
    if (iGpuContextHandle == gpuContextHandle)
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
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t PerfTag
//!           [in] Perf tag
//! \return   void
//!
void Mos_Specific_SetPerfTag(
    PMOS_INTERFACE    osInterface,
    uint32_t          perfTag)
{
    uint32_t        componentTag    = 0;
    PMOS_CONTEXT    osContext       = nullptr;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface->osStreamState);
    osInterface->osStreamState->component = osInterface->Component;
    return MosInterface::SetPerfTag(osInterface->osStreamState, perfTag);
}

//!
//! \brief    Resets the perf tag
//! \details  Resets the perf tag
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \return   void
//!
void Mos_Specific_ResetPerfBufferID(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    return MosInterface::ResetPerfBufferID(osInterface->osStreamState);
}

//!
//! \brief    Increment the perf tag for buffer ID
//! \details  Increment the perf tag for buffer ID
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \return   void
//!
void Mos_Specific_IncPerfBufferID(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    return MosInterface::IncPerfBufferID(osInterface->osStreamState);
}

//!
//! \brief    Increment the perf tag for Frame ID
//! \details  Increment the perf tag for frame ID
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \return   void
//!
void Mos_Specific_IncPerfFrameID(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    return MosInterface::IncPerfFrameID(osInterface->osStreamState);
}

//!
//! \brief    Gets the perf tag
//! \details  Gets the perf tag
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS interface structure
//! \return   uint32_t
//!           Return perf tag
//!
uint32_t Mos_Specific_GetPerfTag(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, 0);

    return MosInterface::GetPerfTag(osInterface->osStreamState);
}

//!
//! \brief    Check if Perf Tag is already set
//! \details  Check if Perf Tag is already set
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \return   int32_t
//!
int32_t Mos_Specific_IsPerfTagSet(
    PMOS_INTERFACE     osInterface)
{
    uint32_t     ComponentTag;
    int32_t      bRet = false;
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, false);
    return MosInterface::IsPerfTagSet(osInterface->osStreamState);
}

//!
//! \brief    Check for GPU context valid
//! \details  Always returns MOS_STATUS_SUCCESS on Linux.
//            This interface is implemented for compatibility.
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to Os interface structure
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] Gpu Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_IsGpuContextValid(
    PMOS_INTERFACE         osInterface,
    MOS_GPU_CONTEXT        gpuContext)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(gpuContext);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Unified OS Resources sync
//! \details  Tag based synchronization at the resource level
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to OS Resource
//! \param    MOS_GPU_CONTEXT requestorGPUCtx
//!           [in] GPU  Context
//! \param    int32_t bWriteOperation
//!           [in] true if write operation
//! \return   void
//!
void Mos_Specific_SyncOnResource(
    PMOS_INTERFACE          osInterface,
    PMOS_RESOURCE           osResource,
    MOS_GPU_CONTEXT         requestorGPUCtx,
    int32_t                 bWriteOperation)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(osResource);
    MOS_UNUSED(requestorGPUCtx);
    MOS_UNUSED(bWriteOperation);
}

//!
//! \brief    Checks for HW enabled
//! \details  Checks for HW enabled
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to Os interface structure
//! \return   int32_t
//!           Returns true id HW enabled if CP mode
//!
int32_t Mos_Specific_IsNullHWEnabled(
    PMOS_INTERFACE     osInterface)
{
    MOS_UNUSED(osInterface);
    return false;
}

//!
//! \brief    Get GPU status buffer
//! \details  Gets the status buffer for the resource
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE osResource
//!           [out] Pointer to OS Resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetGpuStatusBufferResource(
    PMOS_INTERFACE         osInterface,
    PMOS_RESOURCE          &osResource)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(osInterface->osStreamState);

    if(!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::GetGpuStatusBufferResource(osInterface->osStreamState, osResource, osInterface->osStreamState->currentGpuContextHandle);
    }

    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);

    osResource = osInterface->pOsContext->pGPUStatusBuffer;
    return MOS_STATUS_SUCCESS;

}

//!
//! \brief    Get GPU status tag offset
//! \details  Gets the status tag offset
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           Returns the tag offset
//!
uint32_t Mos_Specific_GetGpuStatusTagOffset(
    PMOS_INTERFACE        osInterface,
    MOS_GPU_CONTEXT       gpuContext)
{
    MOS_OS_FUNCTION_ENTER;
    if(Mos_Solo_IsEnabled(nullptr))
    {
        return sizeof(MOS_GPU_STATUS_DATA) * gpuContext;
    }

    return 0;
}

//!
//! \brief    Get GPU status tag
//! \details  Gets the status tag
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           Returns the tag
//!
uint32_t Mos_Specific_GetGpuStatusTag(
    PMOS_INTERFACE            osInterface,
    MOS_GPU_CONTEXT           mosGpuCtx)
{
    MOS_OS_FUNCTION_ENTER;

    if (osInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("osInterface == nullptr");
        return -1;
    }

    if (mosGpuCtx >= MOS_GPU_CONTEXT_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return -1;
    }

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        auto handle = osInterface->m_GpuContextHandleMap[mosGpuCtx];

        return MosInterface::GetGpuStatusTag(osInterface->osStreamState, handle);
    }

    return Linux_GetGpuCtxBufferTag(osInterface->pOsContext, mosGpuCtx);
}

//!
//! \brief    Increment GPU status tag
//! \details  Increment the status tag
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   void
//!
void Mos_Specific_IncrementGpuStatusTag(
    PMOS_INTERFACE        osInterface,
    MOS_GPU_CONTEXT       mosGpuCtx)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    if (mosGpuCtx >= MOS_GPU_CONTEXT_MAX)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter gpuContext.");
        return;
    }

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        auto handle = osInterface->m_GpuContextHandleMap[mosGpuCtx];

        MosInterface::IncrementGpuStatusTag(osInterface->osStreamState, handle);
        return;
    }

    return Linux_IncGpuCtxBufferTag(osInterface->pOsContext, mosGpuCtx);
}

//!
//! \brief    Get GPU status Sync tag
//! \details  This function will return the GPU status tag which is updated when corresponding 
//!           GPU packet is done. User can use this flag to check if some GPU packet is done.
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   uint32_t
//!           Returns the tag
//!
uint32_t Mos_Specific_GetGpuStatusSyncTag(
    PMOS_INTERFACE            osInterface,
    MOS_GPU_CONTEXT           gpuContext)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, 0);
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface->pOsContext, 0);

    if (!Mos_Solo_IsEnabled(nullptr))
    {
        auto handle = osInterface->m_GpuContextHandleMap[gpuContext];

        PMOS_RESOURCE gpuStatusResource = nullptr;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetGpuStatusBufferResource(osInterface->osStreamState, gpuStatusResource, handle));
        MOS_OS_CHK_NULL_RETURN(gpuStatusResource);
        auto gpuStatusData = (MOS_GPU_STATUS_DATA *)gpuStatusResource->pData;
        if (gpuStatusData == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot find ");
            return 0;
        }
        return gpuStatusData->GPUTag;
    }
    else
    {
        MOS_GPU_STATUS_DATA  *pGPUStatusData = nullptr;

        if ( osInterface->pOsContext->pGPUStatusBuffer == nullptr ||
             osInterface->pOsContext->pGPUStatusBuffer->pData == nullptr)
        {
            return 0;
        }

        pGPUStatusData = (MOS_GPU_STATUS_DATA *)(osInterface->pOsContext->pGPUStatusBuffer->pData + (sizeof(MOS_GPU_STATUS_DATA) * gpuContext));
        if (pGPUStatusData == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active resource");
            return 0;
        }
        return pGPUStatusData->GPUTag;
    }
}

//!
//! \brief    Sets the resource sync tag
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   void
//!
void Mos_Specific_SetResourceSyncTag(
    PMOS_INTERFACE         osInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(pParams);
    return ;
}

//!
//! \brief    Perform overlay sync
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_PerformOverlaySync(
    PMOS_INTERFACE         osInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(pParams);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Wait on resource
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_ResourceWait(
    PMOS_INTERFACE         osInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(pParams);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Registers a complete notification event
//! \details  Registers a complete notification event
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU Context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_RegisterBBCompleteNotifyEvent(
    PMOS_INTERFACE     osInterface,
    MOS_GPU_CONTEXT    gpuContext)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(gpuContext);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Waits on a complete notification event
//! \details  Waits on a complete notification event
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    uint32_t uiTimeOut
//!           [in] Time to wait (ms)
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_WaitForBBCompleteNotifyEvent(
    PMOS_INTERFACE     osInterface,
    MOS_GPU_CONTEXT    gpuContext,
    uint32_t           uiTimeOut)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(gpuContext);
    usleep(1000 * uiTimeOut);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_WaitAllCmdCompletion_Os(
    PMOS_INTERFACE osInterface)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

//!
//! \brief    Get interface version
//! \details  Get interface version
//!
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//!
//! \return   uint32_t
//!           Read-only OS runtime interface version, it's meaning diff from OS and API
//!
uint32_t Mos_Specific_GetInterfaceVersion(
    PMOS_INTERFACE        osInterface)
{
    MOS_UNUSED(osInterface);
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
    PMOS_RESOURCE               osResource)
{
    MOS_OS_CHK_NULL_RETURN(osResource);

    mos_bo_set_object_async(osResource->bo);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Gets the HW rendering flags
//! \details  Gets the HW rendering flags
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   MOS_NULL_RENDERING_FLAGS 
//!           Returns the null rendering flags
//!
MOS_NULL_RENDERING_FLAGS  Mos_Specific_GetNullHWRenderFlags(
    PMOS_INTERFACE         osInterface)
{
    return osInterface->NullHWAccelerationEnable;
}

//!
//! \brief    Debug hook to note type of surface state or sampler state being
//!           used.
//! \details  Sets the Command buffer debug info
//! \param    PMOS_INTERFACE osInterface
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
    PMOS_INTERFACE              osInterface,
    int32_t                     bSamplerState,
    int32_t                     bSurfaceState,
    uint32_t                    dwStateIndex,
    uint32_t                    dwType)
{
    MOS_UNUSED(osInterface);
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
//! \param    PMOS_INTERFACE osInterface
//! \param    int32_t bSamplerState
//! \param    int32_t bSurfaceState
//! \param    uint32_t dwStateIndex
//! \return   uint32_t
//!
uint32_t Mos_Specific_GetCmdBufferDebugInfo(
    PMOS_INTERFACE              osInterface,
    int32_t                     bSamplerState,
    int32_t                     bSurfaceState,
    uint32_t                    dwStateIndex)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(bSamplerState);
    MOS_UNUSED(bSurfaceState);
    MOS_UNUSED(dwStateIndex);
    // stub function. implemented for simulation but not driver.
    return 0;
}

//!
//! \brief    Set PAK/MFX context for Encoder which can be used for Synchronization
//! \details  On Linux, the synchronization is handled in KMD, no job in UMD
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void Mos_Specific_SetEncodePakContext(
    PMOS_INTERFACE        osInterface,
    MOS_GPU_CONTEXT       gpuContext)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(gpuContext);
    return;
}

//!
//! \brief    Set VME/ENC context for Encoder which can be used for Synchronization
//! \details  On Linux, the synchronization is handled in KMD, no job in UMD
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    MOS_GPU_CONTEXT gpuContext
//!           [in] GPU context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void Mos_Specific_SetEncodeEncContext(
    PMOS_INTERFACE        osInterface,
    MOS_GPU_CONTEXT       gpuContext)
{
    MOS_UNUSED(osInterface);
    MOS_UNUSED(gpuContext);
    return;
}

//!
//! \brief    Verifys the patch list to be used for rendering GPU commands is large enough
//! \details
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    uint32_t dwRequestedSize
//!           [in] patch list size to be verified
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_VerifyPatchListSize(
    PMOS_INTERFACE          osInterface,
    uint32_t                dwRequestedSize)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);

    // No APO MOS interface support for this func, implement in wrapper
    auto streamState = osInterface->osStreamState;

    MOS_OS_CHK_NULL_RETURN(streamState);
    MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

    auto osDeviceContext = streamState->osDeviceContext;

    if (Mos_Solo_IsEnabled(nullptr))
    {
        MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);
        MOS_OS_GPU_CONTEXT &osGpuContext = osInterface->pOsContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];
        if (dwRequestedSize > osGpuContext.uiMaxPatchLocationsize)
        {
            return MOS_STATUS_UNKNOWN;
        }
        return MOS_STATUS_SUCCESS;
    }

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
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER cmdBuffer
//!           [in] pointer to command buffer structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_ResetCommandBuffer(
    PMOS_INTERFACE          osInterface,
    PMOS_COMMAND_BUFFER     cmdBuffer)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    if (!Mos_Solo_IsEnabled(nullptr))
    {
        return MosInterface::ResetCommandBuffer(osInterface->osStreamState, cmdBuffer);
    }

    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);
    MOS_OS_GPU_CONTEXT &osGpuContext = osInterface->pOsContext->OsGpuContext[osInterface->CurrentGpuContextOrdinal];

    osGpuContext.bCBFlushed = true;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Get the memory compression mode
//! \details  Gets the memory compression mode from GMM
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] pointer to input OS resource
//! \param    PMOS_MEMCOMP_STATE pResMmcMode
//!           [out] the memory compression mode gotten from OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetMemoryCompressionMode(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       osResource,
    PMOS_MEMCOMP_STATE  pResMmcMode)
{
    MOS_OS_CHK_NULL_RETURN(pResMmcMode);
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::GetMemoryCompressionMode(osInterface->osStreamState, osResource, *pResMmcMode);
}

//!
//! \brief    Set the memory compression mode in GMM
//! \details  Set the memory compression mode
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] pointer to input OS resource
//! \param    MOS_MEMCOMP_STATE ResMmcMode
//!           [in] the memory compression mode to be set into OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetMemoryCompressionMode(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       osResource,
    MOS_MEMCOMP_STATE   resMmcMode)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::SetMemoryCompressionMode(osInterface->osStreamState, osResource, resMmcMode);
}

//!
//! \brief    Set the memory compression hint in GMM on Linux or Gralloc on Android
//! \details  Indicate if the surface is compressible 
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] pointer to input OS resource
//! \param    int32_t bHintOn
//!           [in] the memory compression hint to be set
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_SetMemoryCompressionHint(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       osResource,
    int32_t             bHintOn)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    return MosInterface::SetMemoryCompressionHint(osInterface->osStreamState, osResource, bHintOn);
}

//!
//! \brief    Get the memory compression format
//! \details  Gets the memory compression format from GMM
//! \param    PMOS_INTERFACE osInterface
//!           [in] pointer to OS interface structure
//! \param    PMOS_RESOURCE osResource
//!           [in] pointer to input OS resource
//! \param    uint32_t *pResMmcFormat
//!           [out] the memory compression format gotten from GMM resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetMemoryCompressionFormat(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       osResource,
    uint32_t            *pResMmcFormat)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);
    return MosInterface::GetMemoryCompressionFormat(osInterface->osStreamState, osResource, pResMmcFormat);
}

//!
//! \brief    Create GPU node association.
//! \details  In i915 it already implements coarse ping-pong based on fd.
//!  Set the I915_EXEC_BSD without I915_EXEC_BSD_RING0 and I915_EXEC_BSD_RING1, it will be dispatched to the corresponding node.
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \param    MOS_MEDIA_OPERATION MediaOperation
//!           [in] Media operation
//! \param    MOS_GPU_NODE *videoNodeOrdinal
//!           [out] VCS node ordinal
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS Mos_Specific_CreateVideoNodeAssociation(
    PMOS_INTERFACE      osInterface,
    int32_t             bSetVideoNode,
    MOS_GPU_NODE        *videoNodeOrdinal)
{
    PMOS_OS_CONTEXT osContext   = nullptr;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(videoNodeOrdinal);
    MOS_OS_CHK_NULL_RETURN(osInterface->pOsContext);

    osContext = osInterface->pOsContext;
    *videoNodeOrdinal = MOS_GPU_NODE_VIDEO;

    if (false == osContext->bKMDHasVCS2)
    {
        return MOS_STATUS_SUCCESS;
    }

    // If node selection is forced or we have only one VDBox, turn balancing off.
    // After that check debug flags.
    if (osInterface->bEnableVdboxBalancing)
    {
        osContext->bPerCmdBufferBalancing = osInterface->pfnGetVdboxNodeId;
    }
    else 
    {
        osContext->bPerCmdBufferBalancing = 0;
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroy GPU node association.
//! \details  Destroy GPU node association.
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \param    MOS_GPU_NODE VideoNodeOrdinal
//!           [in] VCS node ordinal
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, otherwise error code
//!
MOS_STATUS Mos_Specific_DestroyVideoNodeAssociation(
    PMOS_INTERFACE     osInterface,
    MOS_GPU_NODE       VideoNodeOrdinal)
{
    return MOS_STATUS_SUCCESS;
}

MOS_VDBOX_NODE_IND Mos_Specific_GetVdboxNodeId(
    PMOS_INTERFACE osInterface,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_VDBOX_NODE_IND idx = MOS_VDBOX_NODE_INVALID;

    MOS_OS_CHK_NULL_RETURN_VALUE(cmdBuffer, MOS_VDBOX_NODE_INVALID);
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, MOS_VDBOX_NODE_INVALID);
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface->pOsContext, MOS_VDBOX_NODE_INVALID);

    // If we have assigned vdbox index for the given cmdbuf, return it immediately
    return cmdBuffer->iVdboxNodeIndex;
}

//!
//! \brief    Get the memory object
//! \details  Get the memory object for cache policy
//! \param    MOS_HW_RESOURCE_DEF mosUsage
//!           [in] HW resource
//!           [in] Gmm client context
//! \return   MEMORY_OBJECT_CONTROL_STATE
//!           Return the memory object
//!
MEMORY_OBJECT_CONTROL_STATE Mos_Specific_CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF         mosUsage,
    GMM_CLIENT_CONTEXT          *gmmClientContext)
{
   // Force convert to stream handle for wrapper
    return MosInterface::GetCachePolicyMemoryObject(gmmClientContext, mosUsage);
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
    MOS_HW_RESOURCE_DEF         mosUsage,
    GMM_CLIENT_CONTEXT          *gmmClientContext)
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
    PMOS_INTERFACE              osInterface,
    PCCHAR                      pFileName,
    void                        **ppvModule)
{
    MOS_UNUSED(osInterface);

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
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \return   int32_t
//!           Return if the GPU Hung
//!
int32_t Mos_Specific_IsGPUHung(
    PMOS_INTERFACE              osInterface)
{
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, false);
    return MosInterface::IsGPUHung(osInterface->osStreamState);
}

uint64_t Mos_Specific_GetAuxTableBaseAddr(
    PMOS_INTERFACE              osInterface)
{
    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, 0);
    return MosInterface::GetAuxTableBaseAddr(osInterface->osStreamState);
}

//!
//! \brief  Get gpu context priority from KMD
//! \param  [in] osInterface
//!         Pointer to OS interface
//! \param  [out] pPriority
//!         the priority of the  current context.
//!
void Mos_Specific_GetGpuPriority(
        PMOS_INTERFACE              osInterface,
        int32_t*                    pPriority)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);

    return MosInterface::GetGpuPriority(osInterface->osStreamState, pPriority);
}

//!
//! \brief  Set gpu priority to KMD
//! \param  [in] osInterface
//!         Pointer to OS interface
//! \param  [in] priority
//!         the priority set to current context.
//!
void Mos_Specific_SetGpuPriority(
        PMOS_INTERFACE              osInterface,
        int32_t                     priority)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osInterface);
    return MosInterface::SetGpuPriority(osInterface->osStreamState, priority);
}

MOS_STATUS Mos_Specific_CheckVirtualEngineSupported(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    osInterface->bSupportVirtualEngine = true;

    return MOS_STATUS_SUCCESS;
}

MediaUserSettingSharedPtr Mos_Specific_GetUserSettingInstance(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN_VALUE(osInterface, nullptr);

    return MosInterface::MosGetUserSettingInstance(osInterface->osStreamState);
}

static MOS_STATUS Mos_Specific_InitInterface_Ve(
    PMOS_INTERFACE osInterface)
{
    MOS_STATUS                          eStatus;
    MOS_STATUS                          eStatusUserFeature;
    uint32_t                            regValue = 0;
    MediaUserSettingSharedPtr           userSettingPtr = nullptr;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;
    eStatusUserFeature = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);

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
            osInterface->bParallelSubmission = true;
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
        osInterface->bParallelSubmission = osInterface->bParallelSubmission && regValue;

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
    PMOS_INTERFACE osInterface)
{
    MOS_OS_FUNCTION_ENTER;

    return false;
}

bool Mos_Specific_IsAsyncDevice(PMOS_INTERFACE osInterface)
{
    return false;
}

bool Mos_Specific_IsGpuSyncByCmd(
    PMOS_INTERFACE osInterface)
{
    return false;
}

MOS_STATUS Mos_Specific_LoadFunction(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    osInterface->pfnSetGpuContext            = Mos_Specific_SetGpuContext;
    osInterface->pfnSetGpuContextFromHandle  = Mos_Specific_SetGpuContextFromHandle;
    osInterface->pfnGetGpuContext            = Mos_Specific_GetGpuContext;
    osInterface->pfnSetEncodePakContext      = Mos_Specific_SetEncodePakContext;
    osInterface->pfnSetEncodeEncContext      = Mos_Specific_SetEncodeEncContext;
    osInterface->pfnGetGmmClientContext      = Mos_Specific_GetGmmClientContext;

    osInterface->pfnGetPlatform                     = Mos_Specific_GetPlatform;
    osInterface->pfnDestroy                         = Mos_Specific_Destroy;
    osInterface->pfnGetSkuTable                     = Mos_Specific_GetSkuTable;
    osInterface->pfnGetWaTable                      = Mos_Specific_GetWaTable;
    osInterface->pfnGetGtSystemInfo                 = Mos_Specific_GetGtSystemInfo;
    osInterface->pfnGetMediaEngineInfo              = Mos_Specific_GetMediaEngineInfo;
    osInterface->pfnResetOsStates                   = Mos_Specific_ResetOsStates;
    osInterface->pfnAllocateResource                = Mos_Specific_AllocateResource;
    osInterface->pfnGetResourceInfo                 = Mos_Specific_GetResourceInfo;
    osInterface->pfnFreeResource                    = Mos_Specific_FreeResource;
    osInterface->pfnFreeResourceWithFlag            = Mos_Specific_FreeResourceWithFlag;
    osInterface->pfnLockSyncRequest                 = Mos_Specific_LockSyncRequest;
    osInterface->pfnLockResource                    = Mos_Specific_LockResource;
    osInterface->pfnUnlockResource                  = Mos_Specific_UnlockResource;
    osInterface->pfnDecompResource                  = Mos_Specific_DecompResource;
    osInterface->pfnDoubleBufferCopyResource        = Mos_Specific_DoubleBufferCopyResource;
    osInterface->pfnMediaCopyResource2D             = Mos_Specific_MediaCopyResource2D;
    osInterface->pfnGetMosContext                   = Mos_Specific_GetMosContext;
    osInterface->pfnUpdateResourceUsageType         = Mos_Specific_UpdateResourceUsageType;
    osInterface->pfnRegisterResource                = Mos_Specific_RegisterResource;
    osInterface->pfnResetResourceAllocationIndex    = Mos_Specific_ResetResourceAllocationIndex;
    osInterface->pfnGetResourceAllocationIndex      = Mos_Specific_GetResourceAllocationIndex;
    osInterface->pfnGetResourceGfxAddress           = Mos_Specific_GetResourceGfxAddress;
    osInterface->pfnGetResourceClearAddress         = Mos_Specific_GetResourceClearAddress;
    osInterface->pfnGetCommandBuffer                = Mos_Specific_GetCommandBuffer;
    osInterface->pfnResetCommandBuffer              = Mos_Specific_ResetCommandBuffer;
    osInterface->pfnReturnCommandBuffer             = Mos_Specific_ReturnCommandBuffer;
    osInterface->pfnSubmitCommandBuffer             = Mos_Specific_SubmitCommandBuffer;
    osInterface->pfnVerifyCommandBufferSize         = Mos_Specific_VerifyCommandBufferSize;
    osInterface->pfnResizeCommandBufferAndPatchList = Mos_Specific_ResizeCommandBufferAndPatchList;
    osInterface->pfnSetPerfTag                      = Mos_Specific_SetPerfTag;
    osInterface->pfnResetPerfBufferID               = Mos_Specific_ResetPerfBufferID;
    osInterface->pfnIncPerfFrameID                  = Mos_Specific_IncPerfFrameID;
    osInterface->pfnIncPerfBufferID                 = Mos_Specific_IncPerfBufferID;
    osInterface->pfnGetPerfTag                      = Mos_Specific_GetPerfTag;
    osInterface->pfnIsPerfTagSet                    = Mos_Specific_IsPerfTagSet;
    osInterface->pfnSetIndirectStateSize            = Mos_Specific_SetIndirectStateSize;
    osInterface->pfnGetIndirectState                = Mos_Specific_GetIndirectState;
    osInterface->pfnGetIndirectStatePointer         = Mos_Specific_GetIndirectStatePointer;
    osInterface->pfnSetPatchEntry                   = Mos_Specific_SetPatchEntry;
    osInterface->pfnGetInterfaceVersion             = Mos_Specific_GetInterfaceVersion;

    osInterface->pfnLoadLibrary                     = Mos_Specific_LoadLibrary;
    osInterface->pfnFreeLibrary                     = Mos_Specific_FreeLibrary;
    osInterface->pfnCheckVirtualEngineSupported     = Mos_Specific_CheckVirtualEngineSupported;

    //GPU context and synchronization functions
    osInterface->pfnCreateGpuContext           = Mos_Specific_CreateGpuContext;
    osInterface->pfnCreateGpuComputeContext    = Mos_Specific_CreateGpuComputeContext;
    osInterface->pfnDestroyGpuContext          = Mos_Specific_DestroyGpuContext;
    osInterface->pfnDestroyGpuComputeContext   = Mos_Specific_DestroyGpuComputeContext;
    osInterface->pfnIsGpuContextValid          = Mos_Specific_IsGpuContextValid;
    osInterface->pfnSyncOnResource             = Mos_Specific_SyncOnResource;
    osInterface->pfnGetGpuStatusBufferResource = Mos_Specific_GetGpuStatusBufferResource;
    osInterface->pfnGetGpuStatusTagOffset      = Mos_Specific_GetGpuStatusTagOffset;
    osInterface->pfnGetGpuStatusTag            = Mos_Specific_GetGpuStatusTag;
    osInterface->pfnIncrementGpuStatusTag      = Mos_Specific_IncrementGpuStatusTag;
    osInterface->pfnGetGpuStatusSyncTag        = Mos_Specific_GetGpuStatusSyncTag;
    osInterface->pfnSetResourceSyncTag         = Mos_Specific_SetResourceSyncTag;
    osInterface->pfnPerformOverlaySync         = Mos_Specific_PerformOverlaySync;
    osInterface->pfnWaitAllCmdCompletion       = Mos_Specific_WaitAllCmdCompletion_Os;
    osInterface->pfnResourceWait               = Mos_Specific_ResourceWait;

    osInterface->pfnCachePolicyGetMemoryObject  = Mos_Specific_CachePolicyGetMemoryObject;
    osInterface->pfnVerifyPatchListSize         = Mos_Specific_VerifyPatchListSize;
    osInterface->pfnGetMemoryCompressionMode    = Mos_Specific_GetMemoryCompressionMode;
    osInterface->pfnSetMemoryCompressionMode    = Mos_Specific_SetMemoryCompressionMode;
    osInterface->pfnSetMemoryCompressionHint    = Mos_Specific_SetMemoryCompressionHint;
    osInterface->pfnGetMemoryCompressionFormat  = Mos_Specific_GetMemoryCompressionFormat;
    osInterface->pfnCreateVideoNodeAssociation  = Mos_Specific_CreateVideoNodeAssociation;
    osInterface->pfnDestroyVideoNodeAssociation = Mos_Specific_DestroyVideoNodeAssociation;
    osInterface->pfnGetVdboxNodeId              = Mos_Specific_GetVdboxNodeId;

    osInterface->pfnGetNullHWRenderFlags  = Mos_Specific_GetNullHWRenderFlags;
    osInterface->pfnSetCmdBufferDebugInfo = Mos_Specific_SetCmdBufferDebugInfo;
    osInterface->pfnGetCmdBufferDebugInfo = Mos_Specific_GetCmdBufferDebugInfo;

    osInterface->pfnRegisterBBCompleteNotifyEvent = Mos_Specific_RegisterBBCompleteNotifyEvent;
    osInterface->pfnWaitForBBCompleteNotifyEvent  = Mos_Specific_WaitForBBCompleteNotifyEvent;
    osInterface->pfnCachePolicyGetMemoryObject    = Mos_Specific_CachePolicyGetMemoryObject;
    osInterface->pfnSkipResourceSync              = Mos_Specific_SkipResourceSync;
    osInterface->pfnIsGPUHung                     = Mos_Specific_IsGPUHung;
    osInterface->pfnGetAuxTableBaseAddr           = Mos_Specific_GetAuxTableBaseAddr;
    osInterface->pfnGetResourceIndex              = Mos_Specific_GetResourceIndex;
    osInterface->pfnGetGpuPriority                = Mos_Specific_GetGpuPriority;
    osInterface->pfnSetGpuPriority                = Mos_Specific_SetGpuPriority;
    osInterface->pfnIsSetMarkerEnabled            = Mos_Specific_IsSetMarkerEnabled;
    osInterface->pfnGetMarkerResource             = Mos_Specific_GetMarkerResource;
    osInterface->pfnGetResType                    = GetResType;
    osInterface->pfnGetTsFrequency                = Mos_Specific_GetTsFrequency;
    osInterface->pfnSetHintParams                 = Mos_Specific_SetHintParams;
    osInterface->pfnVirtualEngineInit             = Mos_Specific_Virtual_Engine_Init;
    osInterface->pfnDestroyVeInterface            = Mos_Specific_DestroyVeInterface;
#if (_DEBUG || _RELEASE_INTERNAL)
    osInterface->pfnGetEngineLogicId              = Mos_Specific_GetEngineLogicId;
#endif
    osInterface->pfnSetGpuContextHandle   = Mos_Specific_SetGpuContextHandle;
    osInterface->pfnGetGpuContextbyHandle = Mos_Specific_GetGpuContextbyHandle;

    osInterface->pfnGetUserSettingInstance = Mos_Specific_GetUserSettingInstance;

    osInterface->pfnIsMismatchOrderProgrammingSupported = Mos_Specific_IsMismatchOrderProgrammingSupported;

    osInterface->pfnIsMultipleCodecDevicesInUse = Mos_Specific_pfnIsMultipleCodecDevicesInUse;
    osInterface->pfnIsAsynDevice               = Mos_Specific_IsAsyncDevice;

    return MOS_STATUS_SUCCESS;
}

//! \brief    Unified OS Initializes OS Linux Interface
//! \details  Linux OS Interface initilization
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_CONTEXT osDriverContext
//!           [in] Pointer to Driver context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_InitInterface(
    PMOS_INTERFACE     osInterface,
    PMOS_CONTEXT       osDriverContext)
{
    PMOS_OS_CONTEXT                 osContext = nullptr;
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface = nullptr;
    MOS_STATUS                      eStatus = MOS_STATUS_UNKNOWN;
    uint32_t                        dwResetCount = 0;
    int32_t                         ret = 0;
    MediaUserSettingSharedPtr       userSettingPtr = nullptr;
    bool                            bSimIsActive = false;
    uint32_t                        regValue = 0;
    char                            *pMediaWatchdog = nullptr;
    long int                        watchdog = 0;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(osDriverContext);

    Mos_Specific_LoadFunction(osInterface);

    userSettingPtr = osDriverContext->m_userSettingPtr;

    // Initialize OS interface functions
    pOsUserFeatureInterface = (PMOS_USER_FEATURE_INTERFACE)&osInterface->UserFeatureInterface;

    osInterface->modularizedGpuCtxEnabled   = true;
    osInterface->veDefaultEnable            = true;
    osInterface->phasedSubmission           = true;
    osInterface->apoMosEnabled              = true;

    MOS_OS_CHK_NULL_RETURN(osDriverContext->m_osDeviceContext);
    osInterface->bNullHwIsEnabled  = osDriverContext->m_osDeviceContext->GetNullHwIsEnabled();

    osInterface->streamStateIniter = true;
    eStatus = MosInterface::CreateOsStreamState(
        &osInterface->osStreamState,
        (MOS_DEVICE_HANDLE)osDriverContext->m_osDeviceContext,
        (MOS_INTERFACE_HANDLE)osInterface,
        osInterface->Component,
        osDriverContext);

    if(MOS_STATUS_SUCCESS != eStatus)
    {
        MOS_OS_ASSERTMESSAGE("MosInterface::CreateOsStreamState FAILED");
        Mos_DestroyInterface(osInterface);
        return eStatus;
    }

    // Set interface functions for legacy HAL
    osContext = (PMOS_OS_CONTEXT)osInterface->osStreamState->perStreamParameters;
    MOS_OS_CHK_NULL_RETURN(osContext);

#if MOS_MEDIASOLO_SUPPORTED
    if (Mos_Solo_IsEnabled(osContext))
    {
        eStatus = Linux_InitContext(osContext, osDriverContext);
        if( MOS_STATUS_SUCCESS != eStatus )
        {
            MOS_OS_ASSERTMESSAGE("Unable to initialize context.");
            Mos_DestroyInterface(osInterface);
            return eStatus;
        }

        osContext->bFreeContext = true;

        //Added by Ben for video memory allocation
        osContext->bufmgr = osDriverContext->bufmgr;
        mos_bufmgr_enable_reuse(osDriverContext->bufmgr);
    }
#endif
    osInterface->pOsContext = osContext;

    osInterface->bUsesPatchList               = true;
    osInterface->bUsesGfxAddress              = false;
    osInterface->bNoParsingAssistanceInKmd    = true;
    osInterface->bUsesCmdBufHeaderInResize    = false;
    osInterface->bUsesCmdBufHeader            = false;
    osInterface->dwNumNalUnitBytesIncluded    = MOS_NAL_UNIT_LENGTH - MOS_NAL_UNIT_STARTCODE_LENGTH;

    osInterface->bInlineCodecStatusUpdate   = true;
    osInterface->bAllowExtraPatchToSameLoc = false;

    pOsUserFeatureInterface->bIsNotificationSupported   = false;
    pOsUserFeatureInterface->pOsInterface               = osInterface;

    pOsUserFeatureInterface->pfnEnableNotification      = MosUtilities::MosUserFeatureEnableNotification;
    pOsUserFeatureInterface->pfnDisableNotification     = MosUtilities::MosUserFeatureDisableNotification;
    pOsUserFeatureInterface->pfnParsePath               = MosUtilities::MosUserFeatureParsePath;

    // Init reset count for the context
    ret = mos_get_reset_stats(osInterface->pOsContext->intel_context, &dwResetCount, nullptr, nullptr);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        dwResetCount = 0;
    }
    osInterface->dwGPUResetCount    = dwResetCount;
    osInterface->dwGPUActiveBatch   = 0;
    osInterface->dwGPUPendingBatch  = 0;

    // enable it on Linux
    osInterface->bMediaReset         = true;
    osInterface->trinityPath         = TRINITY_DISABLED;
    osInterface->umdMediaResetEnable = true;

    // disable Media Reset for non-xe platform who has no media reset support on Linux
    auto skuTable = osInterface->pfnGetSkuTable(osInterface);
    MOS_OS_CHK_NULL_RETURN(skuTable);
    if(!MEDIA_IS_SKU(skuTable, FtrSWMediaReset))
    {
        osInterface->bMediaReset         = false;
        osInterface->umdMediaResetEnable = false;
    }

    pMediaWatchdog = getenv("INTEL_MEDIA_RESET_WATCHDOG");
    if (pMediaWatchdog != nullptr)
    {
        watchdog = strtol(pMediaWatchdog, nullptr, 0);
        if (watchdog == 0)
        {
            osInterface->bMediaReset          = false;
            osInterface->umdMediaResetEnable = false;
        }
    }

    // initialize MOS_CP interface
    osInterface->osCpInterface = Create_MosCpInterface(osInterface);
    if (osInterface->osCpInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("fail to create osCpInterface.");
        Mos_DestroyInterface(osInterface);
        return MOS_STATUS_UNKNOWN;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // read the "Force VDBOX" user feature key
    // 0: not force
    ReadUserSettingForDebug(
        userSettingPtr,
        osInterface->eForceVdbox,
        __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX,
        MediaUserSetting::Group::Device);

    // Force TileYf/Ys
    // 0: Tile Y  1: Tile Yf   2 Tile Ys
    ReadUserSettingForDebug(
        userSettingPtr,
        osInterface->dwForceTileYfYs,
        __MEDIA_USER_FEATURE_VALUE_FORCE_YFYS,
        MediaUserSetting::Group::Device);

    // Null HW Driver
    // 0: Disable
    ReadUserSettingForDebug(
        userSettingPtr,
        osInterface->NullHWAccelerationEnable.Value,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE,
        MediaUserSetting::Group::Device);
#endif // (_DEBUG || _RELEASE_INTERNAL)

#if MOS_MEDIASOLO_SUPPORTED
    Mos_Solo_Initialize(osInterface);
#endif // MOS_MEDIASOLO_SUPPORTED

    eStatus = Mos_Specific_InitInterface_Ve(osInterface);
    if(eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Mos_Specific_InitInterface_Ve failed.");
        Mos_DestroyInterface(osInterface);
        return eStatus;
    }

    return MOS_STATUS_SUCCESS;
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
        case TILING_NONE:
            return MOS_TILE_LINEAR;
        case TILING_X:
            return MOS_TILE_X;
        case TILING_Y:
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
    PMOS_RESOURCE               osResource)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN_VALUE(osResource, 0);

    return osResource->iPitch;
}

void Mos_Specific_SetResourceWidth(
    PMOS_RESOURCE               osResource,
    uint32_t                    dwWidth)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osResource);

    osResource->iWidth = dwWidth;
}

void Mos_Specific_SetResourceFormat(
    PMOS_RESOURCE               osResource,
    MOS_FORMAT                  mosFormat)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(osResource);

    osResource->Format = mosFormat;
}

//!
//! \brief    Get SetMarker enabled flag
//! \details  Get SetMarker enabled flag from OsInterface
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \return   bool
//!           SetMarker enabled flag
//!
bool Mos_Specific_IsSetMarkerEnabled(
    PMOS_INTERFACE         osInterface)
{
    return false;
}

//!
//! \brief    Get SetMarker resource address
//! \details  Get SetMarker resource address from OsInterface
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \return   PMOS_RESOURCE
//!           SetMarker resource address
//!
PMOS_RESOURCE Mos_Specific_GetMarkerResource(
    PMOS_INTERFACE         osInterface)
{
    return 0;
}

//!
//! \brief    Get TimeStamp frequency base
//! \details  Get TimeStamp frequency base from OsInterface
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \return   uint32_t
//!           time stamp frequency base
//!
uint32_t Mos_Specific_GetTsFrequency(PMOS_INTERFACE osInterface)
{
    uint32_t freq = 0;
    int ret = mos_get_ts_frequency(osInterface->pOsContext->bufmgr, &freq);
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

MOS_STATUS Mos_Specific_Virtual_Engine_Init(
    PMOS_INTERFACE pOsInterface,
    PMOS_VIRTUALENGINE_HINT_PARAMS* veHitParams,
    MOS_VIRTUALENGINE_INIT_PARAMS&  veInParams)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);
    MOS_VE_HANDLE veState = nullptr;
    MOS_OS_CHK_STATUS_RETURN(MosInterface::CreateVirtualEngineState(
        pOsInterface->osStreamState, &veInParams, veState));

    MOS_OS_CHK_STATUS_RETURN(MosInterface::GetVeHintParams(pOsInterface->osStreamState, veInParams.bScalabilitySupported, veHitParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_SetHintParams(
    PMOS_INTERFACE                osInterface,
    PMOS_VIRTUALENGINE_SET_PARAMS veParams)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(osInterface->osStreamState);
    MOS_OS_CHK_STATUS_RETURN(MosInterface::SetVeHintParams(osInterface->osStreamState, veParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_DestroyVeInterface(
    PMOS_VIRTUALENGINE_INTERFACE *veInterface)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS Mos_Specific_GetEngineLogicId(
    PMOS_INTERFACE osInterface,
    uint8_t& id)
{
    if (MosInterface::GetVeEngineCount(osInterface->osStreamState) != 1)
    {
        MOS_OS_ASSERTMESSAGE("VeEngineCount is not equal to 1.");
    }
    id = MosInterface::GetEngineLogicId(osInterface->osStreamState, 0);
    return MOS_STATUS_SUCCESS;
}
#endif
