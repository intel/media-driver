/*
* Copyright (c) 2009-2018, Intel Corporation
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
#include "media_fourcc.h"
#include <stdlib.h>

#include "mos_graphicsresource.h"
#include "mos_context_specific.h"
#include "mos_gpucontext_specific.h"
#include "mos_gpucontextmgr.h"

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"

#ifdef ANDROID
#include <ufo/gralloc.h>
#endif

#ifndef ANDROID
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#endif

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

//MemNinja graphics counter
extern int32_t MosMemAllocCounterGfx;

//============= PRIVATE FUNCTIONS <BEGIN>=========================================

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
    int32_t iLoop;

    MOS_OS_FUNCTION_ENTER;

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
//! \brief    Unified OS get command buffer
//! \details  Return the pointer to the next available space in Cmd Buffer
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS Context
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [out] Pointer to Command Buffer
//! \param    int32_t iSize
//!           [out] Size of command in bytes
//! \return   int32_t
//!           Return true is there is space
//!
int32_t Linux_GetCommandBuffer(
    PMOS_CONTEXT            pOsContext,
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    int32_t                 iSize)
{
    int32_t                bResult  = false;
    MOS_LINUX_BO           *cmd_bo = nullptr;

    if ( pOsContext == nullptr ||
         pCmdBuffer == nullptr)
    {
        bResult = false;
        MOS_OS_ASSERTMESSAGE("Linux_GetCommandBuffer:pOsContext == nullptr || pCmdBuffer == NULL");
        goto finish;
    }

    // Allocate the command buffer from GEM
    cmd_bo = mos_bo_alloc(pOsContext->bufmgr,"MOS CmdBuf",iSize,4096);     // Align to page boundary
    if (cmd_bo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Allocation of command buffer failed.");
        bResult = false;
        goto finish;
    }
    //MOS_OS_NORMALMESSAGE("alloc CMB, bo is 0x%x.", cmd_bo);

    // Map command buffer to user virtual address
    if (mos_bo_map(cmd_bo,1) != 0) // Write enable set
    {
        MOS_OS_ASSERTMESSAGE("Mapping of command buffer failed.");
        bResult = false;
        goto finish;
    }

    Mos_ResetResource(&pCmdBuffer->OsResource);

    // Fill in resource information
    pCmdBuffer->OsResource.Format = Format_Buffer;
    pCmdBuffer->OsResource.iWidth = cmd_bo->size;
    pCmdBuffer->OsResource.iHeight = 1;
    pCmdBuffer->OsResource.iPitch = cmd_bo->size;
    pCmdBuffer->OsResource.iSize =  pCmdBuffer->OsResource.iPitch * pCmdBuffer->OsResource.iHeight;
    pCmdBuffer->OsResource.iCount = 1;
    pCmdBuffer->OsResource.pData = (uint8_t*)cmd_bo->virt;
    pCmdBuffer->OsResource.TileType = MOS_TILE_LINEAR;
    pCmdBuffer->OsResource.bo = cmd_bo;
    pCmdBuffer->OsResource.bMapped  = true;

    // for MOS wrapper to avoid memory leak
    pCmdBuffer->OsResource.bConvertedFromDDIResource = true;

    pCmdBuffer->pCmdBase    = (uint32_t*)cmd_bo->virt;
    pCmdBuffer->pCmdPtr     = (uint32_t*)cmd_bo->virt;
    pCmdBuffer->iOffset     = 0;
    pCmdBuffer->iRemaining  = cmd_bo->size;
    pCmdBuffer->iCmdIndex   = -1;
    pCmdBuffer->iVdboxNodeIndex = MOS_VDBOX_NODE_INVALID;

    MOS_ZeroMemory(pCmdBuffer->pCmdBase, cmd_bo->size);
    bResult = true;

finish:
    if ((false == bResult)&&(nullptr != cmd_bo)){
        //need to unreference command buffer allocated.
        mos_bo_unreference(cmd_bo);
    }
    return bResult;
}

//!
//! \brief    Get unused command buffer space
//! \details  Return unused command buffer space
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS Context
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU context
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [out] Pointer to Command buffer
//! \return   void
//!
void Linux_ReturnCommandBuffer(
    PMOS_CONTEXT            pOsContext,
    MOS_GPU_CONTEXT         GpuContext,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MOS_OS_GPU_CONTEXT *pOsGpuContext;

    if (pOsContext == nullptr || pCmdBuffer == nullptr ||
        Mos_ResourceIsNull(&(pCmdBuffer->OsResource)))
    {
        goto finish;
    }
    pOsGpuContext = &pOsContext->OsGpuContext[GpuContext];

    pOsGpuContext->pCB->iOffset    = pCmdBuffer->iOffset ;
    pOsGpuContext->pCB->iRemaining = pCmdBuffer->iRemaining;
    pOsGpuContext->pCB->pCmdPtr    = pCmdBuffer->pCmdPtr;
    pOsGpuContext->pCB->iVdboxNodeIndex = pCmdBuffer->iVdboxNodeIndex;

finish:
    return;
}

//!
//! \brief    Flush Command Buffer
//! \details  Flush Command Buffer space
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS Context
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU context
//! \return   int32_t
//!           true if succeeded, false if failed or invalid parameters
//!
int32_t Linux_FlushCommandBuffer(
    PMOS_CONTEXT           pOsContext,
    MOS_GPU_CONTEXT        GpuContext)
{
    PCOMMAND_BUFFER pCurrCB;
    int32_t         bResult = false;
    PMOS_OS_GPU_CONTEXT pOsGpuContext;

    if (pOsContext == nullptr)
    {
        goto finish;
    }

    pOsGpuContext = &pOsContext->OsGpuContext[GpuContext];
    // Refresh command buffer usage
    pOsContext->pfnRefresh(pOsContext);

    pOsGpuContext->uiCurrentNumPatchLocations = 0;

    // CB already active
    pCurrCB = pOsGpuContext->pCurrentCB;
    if (pCurrCB->bActive)
    {
        goto finish;
    }

    pCurrCB->bActive  = true;
    bResult = true;

finish:
    return bResult;
}

//!
//! \brief    Init command buffer pool
//! \details  Initilize the command buffer pool
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS Context
//! \return   void
//!
void Linux_InitCmdBufferPool(
    PMOS_CONTEXT   pOsContext)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_ZeroMemory(&pOsContext->CmdBufferPool, sizeof(CMD_BUFFER_BO_POOL));
    pOsContext->CmdBufferPool.iFetch = 0;
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
    int32_t         i;
    MOS_STATUS      eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    for (i = 0; i < MAX_CMD_BUF_NUM; i++)
    {
        MOS_OS_CHK_STATUS(Linux_WaitAndReleaseCmdBuffer(pOsContext, i));
    }

finish:
    return eStatus;
}

//!
//! \brief    Wait for the fetch command
//! \details  Wait for the fetch command bo until it is available
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_WaitForAvailableCmdBo(
    PMOS_CONTEXT   pOsContext)
{
    int32_t         index;
    MOS_STATUS      eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    index = pOsContext->CmdBufferPool.iFetch;
    MOS_OS_CHK_STATUS(Linux_WaitAndReleaseCmdBuffer(pOsContext, index));

finish:
    return eStatus;
}

//!
//! \brief    Insert command buffer
//! \details  Insert command buffer into pool
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \param    PMOS_COMMAND_BUFFER    pCmdBuffer
//!           [in] Pointer to command buffer struct
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_InsertCmdBufferToPool(
    PMOS_CONTEXT           pOsContext,
    PMOS_COMMAND_BUFFER    pCmdBuffer)
{
    int32_t         index;
    MOS_STATUS      eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pCmdBuffer);
    MOS_OS_CHK_STATUS(Linux_WaitForAvailableCmdBo(pOsContext));

    index = pOsContext->CmdBufferPool.iFetch;

    pOsContext->CmdBufferPool.pCmd_bo[index] = pCmdBuffer->OsResource.bo;
    pCmdBuffer->iCmdIndex = index;

    pOsContext->CmdBufferPool.iFetch++;
    if (pOsContext->CmdBufferPool.iFetch >= MAX_CMD_BUF_NUM)
    {
        pOsContext->CmdBufferPool.iFetch     = 0;
    }

finish:
    return eStatus;
}

//!
//! \brief    Linux Refresh
//! \details  Linux Refresh
//! \param    MOS_CONTEXT * pOsContext
//!           [in] Pointer to OS context structure
//! \return   int32_t
//!           true always
//!
int32_t Linux_Refresh(MOS_CONTEXT *pOsContext)
{
    MOS_UNUSED(pOsContext);
    return true;
}

#ifndef ANDROID

#define MOS_LINUX_IPC_INVALID_ID -1
#define MOS_LINUX_SHM_INVALID (void *)-1
#define MOS_LINUX_SEM_MAX_TRIES 10

static MOS_STATUS DetachDestroyShm(int32_t shmid, void  *pShm)
{
    struct shmid_ds buf;

    if (shmid < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    if (pShm != MOS_LINUX_SHM_INVALID && shmdt(pShm) < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    if (buf.shm_nattch == 0)
    {
        if (shmctl(shmid, IPC_RMID, nullptr) < 0)
        {
            return MOS_STATUS_UNKNOWN;
        }
    }
    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS ConnectCreateShm(long key, uint32_t size, int32_t *pShmid, void  **ppShm)
{
    struct shmid_ds buf;
    int32_t         shmid;
    key_t           key_value = (key_t)key;
    void            *shmptr = nullptr;

    shmid = shmget(key_value, size, IPC_CREAT | 0666);
    if (shmid < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    shmptr = shmat(shmid, 0, 0);
    if (shmptr == MOS_LINUX_SHM_INVALID)
    {
        DetachDestroyShm(shmid, shmptr);
        return MOS_STATUS_UNKNOWN;
    }

    if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
        // can't get any status info
        DetachDestroyShm(shmid, shmptr);
        return MOS_STATUS_UNKNOWN;
    }

    *ppShm = shmptr;
    *pShmid = shmid;

    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS ConnectCreateSemaphore(long key, int32_t *pSemid)
{
    int32_t         semid;
    struct sembuf   sop;
    struct semid_ds buf;
    uint32_t        i;
    key_t           key_value = (key_t)key;
    int32_t         val = 0;

    semid = semget(key, 1, IPC_CREAT | IPC_EXCL | 0666);

    if (semid != MOS_LINUX_IPC_INVALID_ID)
    {
        // initialize it to 0
        if (semctl(semid, 0, SETVAL, val) == -1)
        {
            return MOS_STATUS_UNKNOWN;
        }

        // Perform a "no-op" semaphore operation - changes sem_otime
        // so other processes can see we've initialized the set.
        sop.sem_num = 0;
        sop.sem_op  = 0; //Wait for value to equal 0
        sop.sem_flg = 0;
        if (semop(semid, &sop, 1) == -1)
        {
            return MOS_STATUS_UNKNOWN;
        }

    }
    else
    {
        // errno EEXIST
        semid = semget(key, 1, 0666);
        if (semid == MOS_LINUX_IPC_INVALID_ID)
        {
            return MOS_STATUS_UNKNOWN;
        }
    }

    *pSemid = semid;

    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS DestroySemaphore(int32_t semid)
{
    int32_t nwait;

    if (semid < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    nwait = semctl(semid, 0, GETZCNT, 0);

    if (nwait > 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (semctl(semid, 0, IPC_RMID, nullptr) < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

static int16_t ShmAttachedNumber(int32_t shmid)
{
    struct shmid_ds buf;

    if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
        return -1;
    }

    return buf.shm_nattch;
}

static MOS_STATUS LockSemaphore(int32_t semid)
{
    struct sembuf op[2];
    op[0].sem_num = 0; // wait for [0] to be 0
    op[0].sem_op  = 0;
    op[0].sem_flg = 0;
    op[1].sem_num = 0;
    op[1].sem_op  = 1; // increment
    op[1].sem_flg = SEM_UNDO;

    if (semid < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    if (semop(semid, op, 2) < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS UnLockSemaphore(int32_t semid)
{
    struct sembuf op;
    op.sem_num = 0;
    op.sem_op  = -1; // decrement back to 0
    op.sem_flg = SEM_UNDO;

    if (semid < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    if (semop(semid, &op, 1) < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

void DestroyIPC(PMOS_CONTEXT pOsContext)
{
    if (MOS_LINUX_IPC_INVALID_ID != pOsContext->semid)
    {
        int16_t iAttachedNum = 0;

        if (MOS_LINUX_IPC_INVALID_ID != pOsContext->shmid)
        {
            LockSemaphore(pOsContext->semid);
            iAttachedNum = ShmAttachedNumber(pOsContext->shmid);

            DetachDestroyShm(pOsContext->shmid, pOsContext->pShm);
            pOsContext->shmid = MOS_LINUX_IPC_INVALID_ID;
            pOsContext->pShm = MOS_LINUX_SHM_INVALID;

            if (iAttachedNum) --iAttachedNum;
            UnLockSemaphore(pOsContext->semid);
        }
    }
}

MOS_STATUS CreateIPC(PMOS_CONTEXT pOsContext)
{
    MOS_STATUS eStatus;

    pOsContext->semid = MOS_LINUX_IPC_INVALID_ID;
    pOsContext->shmid = MOS_LINUX_IPC_INVALID_ID;
    pOsContext->pShm = MOS_LINUX_SHM_INVALID;

    struct semid_ds buf;
    //wait and retry untill to get a valid semaphore
    for (int i = 0; i < MOS_LINUX_SEM_MAX_TRIES; i ++)
    {
        ConnectCreateSemaphore(DUAL_VDBOX_KEY, &pOsContext->semid);

        //check whether the semid is initialized or not
        if (semctl(pOsContext->semid, 0, IPC_STAT, &buf) == -1)
        {
            return MOS_STATUS_UNKNOWN;
        }
        if (buf.sem_otime != 0)
        {
            break;
        }

        MOS_Sleep(1); //wait and retry
    }

    LockSemaphore(pOsContext->semid);
    eStatus = ConnectCreateShm(DUAL_VDBOX_KEY, sizeof(VDBOX_WORKLOAD), &pOsContext->shmid, &pOsContext->pShm);
    UnLockSemaphore(pOsContext->semid);
    MOS_CHK_STATUS_SAFE(eStatus);

finish:
    return eStatus;
}
#endif

GpuContextSpecific* Linux_GetGpuContext(PMOS_INTERFACE pOsInterface, uint32_t gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    if (pOsInterface == nullptr || pOsInterface->osContextPtr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("invalid input parameters!");
        return nullptr;
    }

    auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

    auto gpuContextMgr = osCxtSpecific->GetGpuContextMgr();
    if (gpuContextMgr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("m_gpuContextMgr cannot be nullptr");
        return nullptr;
    }

    auto gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);
    if (gpuContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active gpuContextHandle");
        return nullptr;
    }

    auto gpuContextSpecific = static_cast<GpuContextSpecific *>(gpuContext);

    return gpuContextSpecific;
}

//!
//! \brief    Initialize the GPU Status Buffer
//! \details  Initialize the GPU Status Buffer
//! \param    MOS_CONTEXT * pOsContext
//!           [in, out] Pointer to OS context structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_InitGPUStatus(
    PMOS_CONTEXT    pOsContext)
{
    MOS_LINUX_BO    *bo     = nullptr;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Linux_InitGPUStatus:pOsContext == NULL");
        eStatus = MOS_STATUS_NULL_POINTER;
        goto finish;
    }

    pOsContext->pGPUStatusBuffer      =
                (MOS_RESOURCE*)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * MOS_GPU_CONTEXT_MAX);
    if (nullptr == pOsContext->pGPUStatusBuffer)
    {
        MOS_OS_ASSERTMESSAGE("pContext->pGPUStatusBuffer malloc failed.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Allocate the command buffer from GEM
    bo = mos_bo_alloc(pOsContext->bufmgr,"GPU Status Buffer", sizeof(MOS_GPU_STATUS_DATA)  * MOS_GPU_CONTEXT_MAX, 4096);     // Align to page boundary
    if (bo == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Allocation of GPU Status Buffer failed.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    // Map command buffer to user virtual address
    if (mos_bo_map(bo, 1) != 0) // Write enable set
    {
        MOS_OS_ASSERTMESSAGE("Mapping of GPU Status Buffer failed.");
        eStatus = MOS_STATUS_INVALID_HANDLE;
        goto finish;
    }

    Mos_ResetResource(pOsContext->pGPUStatusBuffer);

    // Fill in resource information
    pOsContext->pGPUStatusBuffer->Format   = Format_Buffer;
    pOsContext->pGPUStatusBuffer->iWidth   = bo->size;
    pOsContext->pGPUStatusBuffer->iHeight  = 1;
    pOsContext->pGPUStatusBuffer->iPitch   = bo->size;
    pOsContext->pGPUStatusBuffer->iCount   = 1;
    pOsContext->pGPUStatusBuffer->pData    = (uint8_t*)bo->virt;
    pOsContext->pGPUStatusBuffer->TileType = MOS_TILE_LINEAR;
    pOsContext->pGPUStatusBuffer->bo       = bo;
    pOsContext->pGPUStatusBuffer->bMapped  = true;

    MOS_ZeroMemory(pOsContext->pGPUStatusBuffer->pData, bo->size);

finish:
    return eStatus;
}

//!
//! \brief    Release the GPU Status Buffer
//! \details  Release the GPU Status Buffer
//! \param    MOS_CONTEXT * pOsContext
//!           [in, out] Pointer to OS context structure
//! \return   void
//!
void Linux_ReleaseGPUStatus(
    PMOS_CONTEXT    pOsContext)
{
    MOS_LINUX_BO    *bo = nullptr;

    if ( pOsContext == nullptr || pOsContext->pGPUStatusBuffer == nullptr)
    {
        return;
    }

    bo = pOsContext->pGPUStatusBuffer->bo;
    if (bo != nullptr)
    {
        mos_bo_unmap(bo);
        mos_bo_wait_rendering(bo);
        mos_bo_unreference(bo);
    }
    pOsContext->pGPUStatusBuffer->bo = nullptr;

    MOS_FreeMemAndSetNull(pOsContext->pGPUStatusBuffer);
}

//!
//! \brief    Get GPU status tag for the given GPU context
//! \details  Get GPU status tag for the given GPU context
//! \param    MOS_CONTEXT * pOsContext
//!           [in] Pointer to OS context structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in, out] GPU Context
//! \return   uint32_t
//!           GPU status tag
//!
uint32_t Linux_GetGpuCtxBufferTag(
    PMOS_CONTEXT    pOsContext,
    MOS_GPU_CONTEXT GpuContext)
{
    if ( pOsContext == nullptr)
    {
        return 0;
    }

    return pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag;
}

//!
//! \brief    Increment GPU status tag for the given GPU context
//! \details  Increment GPU status tag for the given GPU context
//! \param    MOS_CONTEXT * pOsContext
//!           [in] Pointer to OS context structure
//! \param    MOS_GPU_CONTEXT GpuContext
//!           [in] GPU Context
//! \return   void
//!
void Linux_IncGpuCtxBufferTag(
        PMOS_CONTEXT               pOsContext,
        MOS_GPU_CONTEXT            GpuContext)
{
    uint32_t uiGPUStatusTag;

    if ( pOsContext == nullptr)
    {
        return;
    }

    uiGPUStatusTag = pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag;

    pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag = uiGPUStatusTag % UINT_MAX + 1;
    if(pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag == 0)
    {
        pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag = 1;
    }
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        if (pOsInterface == nullptr || pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid input parameters!");
            return 0;
        }

        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

        auto handle = osCxtSpecific->GetGpuContextHandle(mosGpuCtx);

        auto gpuContext = Linux_GetGpuContext(pOsInterface, handle);

        if (gpuContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot get corresponding gpu context!");
            return 0;
        }

        auto resource = gpuContext->GetStatusBufferResource();

        if (resource == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active resource");
            return 0;
        }

        MOS_RESOURCE gpuStatusResource;
        MOS_OS_CHK_STATUS_RETURN(resource->ConvertToMosResource(&gpuStatusResource));
        auto gpuStatusData = (MOS_GPU_STATUS_DATA*)gpuStatusResource.pData;

        return gpuStatusData->GPUTag;
    }

    MOS_GPU_STATUS_DATA  *pGPUStatusData = nullptr;

    if ( pOsInterface == nullptr ||
         pOsInterface->pOsContext == nullptr ||
         pOsInterface->pOsContext->pGPUStatusBuffer == nullptr ||
         pOsInterface->pOsContext->pGPUStatusBuffer->pData == nullptr)
    {
        return 0;
    }

    pGPUStatusData = (MOS_GPU_STATUS_DATA*)(pOsInterface->pOsContext->pGPUStatusBuffer->pData + (sizeof(MOS_GPU_STATUS_DATA) * mosGpuCtx));

    return pGPUStatusData->GPUTag;
}

//!
//! \brief    Destroy Linux context
//! \details  Destroy Linux context
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \return   void
//!
void Linux_Destroy(
    PMOS_CONTEXT       pOsContext,
    int32_t            MODSEnabled,
    int32_t            modularizedGpuCtxEnabled)
{
    PCOMMAND_BUFFER pCurrCB, pNextCB;
    int32_t iSize;
    int i =0;

 #ifndef ANDROID
    if (pOsContext->bKMDHasVCS2)
    {
        DestroyIPC(pOsContext);
    }
 #endif

    if (!modularizedGpuCtxEnabled)
    {
        Linux_ReleaseCmdBufferPool(pOsContext);

        for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
        {
            MOS_FreeMemAndSetNull(pOsContext->OsGpuContext[i].pCB);

            pCurrCB = pOsContext->OsGpuContext[i].pStartCB;
            for (; (pCurrCB); pCurrCB = pNextCB)
            {
                pNextCB = pCurrCB->pNext;
                MOS_FreeMemAndSetNull(pCurrCB);
            }
        }

        Linux_ReleaseGPUStatus(pOsContext);
    }

#ifndef ANDROID
    if (pOsContext->contextOffsetList.size())
    {
         pOsContext->contextOffsetList.clear();
         pOsContext->contextOffsetList.shrink_to_fit();
    }
#endif

    if (!MODSEnabled && (pOsContext->intel_context))
    {
        mos_gem_context_destroy(pOsContext->intel_context);
    }

    GmmDeleteClientContext(pOsContext->pGmmClientContext);

    MOS_FreeMemAndSetNull(pOsContext);
}

//!
//! \brief    Get DMA Buffer ID
//! \details  Get the DMA perf Buffer ID
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \return   uint32_t
//!           Return the buffer ID
//!
uint32_t Linux_GetDmaBufID (PMOS_CONTEXT pOsContext )
{
    uint32_t dmaBufID =0;
    if(pOsContext->pPerfData != nullptr)
    {
        dmaBufID = pOsContext->pPerfData->dmaBufID;
    }
    return dmaBufID;
}

//!
//! \brief    Set the DMA Buffer ID
//! \details  Sets the buffer ID in perf data
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \param    uint32_t dwDmaBufID
//!           [in] Buffer ID to be set
//! \return   void
//!
void Linux_SetDmaBufID ( PMOS_CONTEXT pOsContext, uint32_t dwDmaBufID )
{
    if(pOsContext->pPerfData != nullptr)
    {
        pOsContext->pPerfData->dmaBufID = dwDmaBufID;
    }
}

//!
//! \brief    Set the DMA Kernel ID
//! \details  Sets the Kernel ID in perf data
//! \param    PMOS_CONTEXT pOsContext
//!           [in] Pointer to OS context structure
//! \param    uint32_t KernelID
//!           [in] Buffer ID to be set
//! \return   void
//!
void Linux_SetPerfHybridKernelID ( PMOS_CONTEXT pOsContext, uint32_t KernelID)
{
    if(pOsContext->pPerfData != nullptr)
    {
        pOsContext->pPerfData->dmaBufID = (pOsContext->pPerfData->dmaBufID & 0xF0FF) | ((KernelID <<8) & 0x0F00);
    }
}

//!
//! \brief    Init Linux context
//! \details  Initialize the linux context
//! \param    MOS_OS_CONTEXT * pContext
//!           [in] Pointer to OS context structure
//! \param    PMOS_CONTEXT pOsDriverContext
//!           [in] Pointer to OS Driver context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Linux_InitContext(
    MOS_OS_CONTEXT      *pContext,
    PMOS_CONTEXT         pOsDriverContext,
    int32_t              MODSEnabled,
    int32_t              modularizedGpuCtxEnabled)
{
    int32_t              iDeviceId;
    MOS_STATUS           eStatus;
    int32_t              i;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    if (nullptr == pContext ||
        nullptr == pOsDriverContext ||
        nullptr == pOsDriverContext->bufmgr ||
        nullptr == pOsDriverContext->m_gpuContextMgr ||
        nullptr == pOsDriverContext->m_cmdBufMgr ||
        0 >= pOsDriverContext->fd)
    {
        MOS_OS_ASSERT(false);
        return MOS_STATUS_INVALID_HANDLE;
    }
    pContext->bufmgr          = pOsDriverContext->bufmgr;
    pContext->m_gpuContextMgr = pOsDriverContext->m_gpuContextMgr;
    pContext->m_cmdBufMgr     = pOsDriverContext->m_cmdBufMgr;
    pContext->fd              = pOsDriverContext->fd;
    pContext->pPerfData       = pOsDriverContext->pPerfData;
    mos_bufmgr_gem_enable_reuse(pOsDriverContext->bufmgr);
    pContext->pCpContext = pOsDriverContext->pCpContext;

    // DDI layer can pass over the DeviceID.
    iDeviceId = pOsDriverContext->iDeviceId;
    if (0 == iDeviceId)
    {
        //Such as CP, it calls InitMosInterface() dretly without creating MediaContext.
        iDeviceId = mos_bufmgr_gem_get_devid(pOsDriverContext->bufmgr);
        pOsDriverContext->iDeviceId = iDeviceId;

        MOS_OS_CHK_STATUS_MESSAGE(
            HWInfo_GetGfxInfo(pOsDriverContext->fd, &pContext->platform, &pContext->SkuTable, &pContext->WaTable, &pContext->gtSystemInfo),
            "Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization");

        pOsDriverContext->SkuTable     = pContext->SkuTable;
        pOsDriverContext->WaTable      = pContext->WaTable;
        pOsDriverContext->gtSystemInfo = pContext->gtSystemInfo;
        pOsDriverContext->platform     = pContext->platform;
        MOS_OS_NORMALMESSAGE("DeviceID was created DeviceID = %d, platform product %d", iDeviceId, pContext->platform.eProductFamily);
    }
    else
    {
        // pOsDriverContext's parameters were passed by CmCreateDevice.
        // Get SkuTable/WaTable/systemInfo/platform from OSDriver directly.
        pContext->SkuTable     = pOsDriverContext->SkuTable;
        pContext->WaTable      = pOsDriverContext->WaTable;
        pContext->gtSystemInfo = pOsDriverContext->gtSystemInfo;
        pContext->platform     = pOsDriverContext->platform;
    }

    pContext->bUse64BitRelocs = true;
#ifndef ANDROID
    // when MODS enabled, intel_context will be created by pOsContextSpecific, should not recreate it here, or will cause memory leak.
    if (!MODSEnabled)
    {
       pContext->intel_context = mos_gem_context_create(pOsDriverContext->bufmgr);

       if (pContext->intel_context == nullptr)
       {
            MOS_OS_ASSERTMESSAGE("Failed to create drm intel context");
            return MOS_STATUS_UNKNOWN;
       }
    }

    pContext->intel_context->pOsContext = pContext;
#else
    pContext->intel_context = nullptr;
#endif

    pContext->bIsAtomSOC = IS_ATOMSOC(iDeviceId);

    if(!modularizedGpuCtxEnabled)
    {
        Linux_InitCmdBufferPool(pContext);

        // Initialize GPU Status Buffer
        eStatus = Linux_InitGPUStatus(pContext);
        if (MOS_STATUS_SUCCESS != eStatus)
        {
            goto finish;
        }

        for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
        {
            pContext->OsGpuContext[i].pStartCB            = nullptr;
            pContext->OsGpuContext[i].pCurrentCB          = nullptr;
            pContext->OsGpuContext[i].bCBFlushed          = true;
            pContext->OsGpuContext[i].uiCommandBufferSize = COMMAND_BUFFER_SIZE;
            pContext->OsGpuContext[i].pCB                 =
                (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));

            if (nullptr == pContext->OsGpuContext[i].pCB)
            {
                MOS_OS_ASSERTMESSAGE("No More Avaliable Memory");
                eStatus = MOS_STATUS_NO_SPACE;
                goto finish;
            }

            // each thread has its own GPU context, so do not need any lock as guarder here
            pContext->OsGpuContext[i].pAllocationList =
                (ALLOCATION_LIST*)MOS_AllocAndZeroMemory(sizeof(ALLOCATION_LIST) * ALLOCATIONLIST_SIZE);
            if (nullptr == pContext->OsGpuContext[i].pAllocationList)
            {
                MOS_OS_ASSERTMESSAGE("pContext->OsGpuContext[%d].pAllocationList malloc failed.", i);
                eStatus = MOS_STATUS_NO_SPACE;
                goto finish;
            }
            pContext->OsGpuContext[i].uiMaxNumAllocations = ALLOCATIONLIST_SIZE;

            pContext->OsGpuContext[i].pPatchLocationList =
                (PATCHLOCATIONLIST*)MOS_AllocAndZeroMemory(sizeof(PATCHLOCATIONLIST) * PATCHLOCATIONLIST_SIZE);
            if (nullptr == pContext->OsGpuContext[i].pPatchLocationList)
            {
                MOS_OS_ASSERTMESSAGE("pContext->OsGpuContext[%d].pPatchLocationList malloc failed.", i);
                eStatus = MOS_STATUS_NO_SPACE;
                goto finish;
            }
            pContext->OsGpuContext[i].uiMaxPatchLocationsize = PATCHLOCATIONLIST_SIZE;

            pContext->OsGpuContext[i].pResources    =
                (PMOS_RESOURCE)MOS_AllocAndZeroMemory(sizeof(MOS_RESOURCE) * ALLOCATIONLIST_SIZE);
            if (nullptr == pContext->OsGpuContext[i].pResources)
            {
                MOS_OS_ASSERTMESSAGE("pContext->OsGpuContext[%d].pResources malloc failed.", i);
                eStatus = MOS_STATUS_NO_SPACE;
                goto finish;
            }

            pContext->OsGpuContext[i].pbWriteMode    =
                (int32_t*)MOS_AllocAndZeroMemory(sizeof(int32_t) * ALLOCATIONLIST_SIZE);
            if (nullptr == pContext->OsGpuContext[i].pbWriteMode)
            {
                MOS_OS_ASSERTMESSAGE("pContext->OsGpuContext[%d].pbWriteMode malloc failed.", i);
                eStatus = MOS_STATUS_NO_SPACE;
                goto finish;
            }

            pContext->OsGpuContext[i].uiGPUStatusTag = 1;
        }
    }

#ifndef ANDROID
    {
        drm_i915_getparam_t     gp;
        int32_t                 ret;
        int32_t                 value = 0;

        //KMD support VCS2?
        gp.value = &value;
        gp.param = I915_PARAM_HAS_BSD2;
        ret = drmIoctl(pContext->fd, DRM_IOCTL_I915_GETPARAM, &gp);
        if (ret == 0 && value != 0)
        {
            pContext->bKMDHasVCS2 = true;
        }
        else
        {
            pContext->bKMDHasVCS2 = false;
        }
    }
    if (pContext->bKMDHasVCS2)
    {
        eStatus = CreateIPC(pContext);
        MOS_CHK_STATUS_SAFE(eStatus);
    }
#endif

    pContext->pTranscryptedKernels      = nullptr;
    pContext->uiTranscryptedKernelsSize = 0;

    // For Media Memory compression
    pContext->ppMediaMemDecompState     = pOsDriverContext->ppMediaMemDecompState;
    pContext->pfnMemoryDecompress       = pOsDriverContext->pfnMemoryDecompress;

    // Set interface functions
    pContext->pfnDestroy                 = Linux_Destroy;
    pContext->pfnGetCommandBuffer        = Linux_GetCommandBuffer;
    pContext->pfnReturnCommandBuffer     = Linux_ReturnCommandBuffer;
    pContext->pfnFlushCommandBuffer      = Linux_FlushCommandBuffer;
    pContext->pfnInsertCmdBufferToPool   = Linux_InsertCmdBufferToPool;
    pContext->pfnWaitAndReleaseCmdBuffer = Linux_WaitAndReleaseCmdBuffer;
    pContext->pfnRefresh                 = Linux_Refresh;
    pContext->GetDmaBufID                = Linux_GetDmaBufID;
    pContext->SetDmaBufID                = Linux_SetDmaBufID;
    pContext->SetPerfHybridKernelID      = Linux_SetPerfHybridKernelID;
    pContext->pfnGetGpuCtxBufferTag      = Linux_GetGpuCtxBufferTag;
    pContext->pfnIncGpuCtxBufferTag      = Linux_IncGpuCtxBufferTag;
    pContext->GetGPUTag                  = Linux_GetGPUTag;

finish:
    if (!modularizedGpuCtxEnabled)
    {
        // init context failed, roll back
        if (eStatus != MOS_STATUS_SUCCESS)
            Mos_Specific_ClearGpuContext(pContext);
    }

    return eStatus;
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
    // Set GPU context handle
    pOsInterface->CurrentGpuContextOrdinal = GpuContext;

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

        auto pOsContextSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

        // Set GPU context handle
        pOsInterface->CurrentGpuContextHandle = pOsContextSpecific->GetGpuContextHandle(GpuContext);
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
    return pOsInterface->CurrentGpuContextOrdinal;
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
    MOS_STATUS          eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);
    MOS_OS_CHK_NULL(pPlatform);

    *pPlatform = pOsInterface->pOsContext->platform;

finish:
    return;
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

    if( nullptr == pOsInterface )
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return;
    }

    if (pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled())
    {
        OsContext* pOsContext = pOsInterface->osContextPtr;
        if (pOsContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
            return;
        }

        pOsContext->CleanUp();

        MOS_Delete(pOsContext);
        pOsInterface->osContextPtr = nullptr;
    }

    if (pOsInterface->osCpInterface)
    {
        MOS_Delete(pOsInterface->osCpInterface);
        pOsInterface->osCpInterface = nullptr;
    }

    if (pOsInterface &&
        pOsInterface->pOsContext &&
        pOsInterface->pOsContext->bFreeContext &&
        !Mos_Solo_IsEnabled())
    {
        pOsInterface->pOsContext->SkuTable.reset();
        pOsInterface->pOsContext->WaTable.reset();
        Mos_Specific_ClearGpuContext(pOsInterface->pOsContext);
        bool modularizedGpuCtxEnabled = pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled();
        pOsInterface->pOsContext->pfnDestroy(pOsInterface->pOsContext, pOsInterface->modulizedMosEnabled, modularizedGpuCtxEnabled);
        pOsInterface->pOsContext = nullptr;
    }
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
    if (pOsInterface && pOsInterface->pOsContext)
    {
        return &pOsInterface->pOsContext->SkuTable;
    }
    return nullptr;
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
    if (pOsInterface && pOsInterface->pOsContext)
    {
        return &pOsInterface->pOsContext->WaTable;
    }
    return nullptr;
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
    if( nullptr == pOsInterface->pOsContext)
    {
        MOS_OS_ASSERTMESSAGE("pOsContext is NULL.");
        return  nullptr;
    }

    return &pOsInterface->pOsContext->gtSystemInfo;
}

//!
//! \brief    Resets OS States
//! \details  Resets OS States for linux
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   void
//!
void Mos_Specific_ResetOsStates(
    PMOS_INTERFACE pOsInterface)                                           // [in] OS Interface
{
    MOS_OS_FUNCTION_ENTER;

    PMOS_OS_CONTEXT     pOsContext;
    PMOS_OS_GPU_CONTEXT pOsGpuContext;

    if (pOsInterface == nullptr ||
        pOsInterface->pOsContext == nullptr)
    {
        return;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        if (gpuContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("GPU Context pointer is nullptr!");
            return;
        }

        return gpuContext->ResetGpuContextStatus();
    }

    pOsContext = pOsInterface->pOsContext;
    pOsGpuContext = &pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];
    // Reset resource allocation
    pOsGpuContext->uiNumAllocations = 0;
    MOS_ZeroMemory(pOsGpuContext->pAllocationList, sizeof(ALLOCATION_LIST) * pOsGpuContext->uiMaxNumAllocations);
    pOsGpuContext->uiCurrentNumPatchLocations = 0;
    MOS_ZeroMemory(pOsGpuContext->pPatchLocationList, sizeof(PATCHLOCATIONLIST) * pOsGpuContext->uiMaxPatchLocationsize);
    pOsGpuContext->uiResCount = 0;

    MOS_ZeroMemory(pOsGpuContext->pResources, sizeof(MOS_RESOURCE) * pOsGpuContext->uiMaxNumAllocations);
    MOS_ZeroMemory(pOsGpuContext->pbWriteMode, sizeof(int32_t) * pOsGpuContext->uiMaxNumAllocations);

    if((pOsGpuContext->bCBFlushed == true) && pOsGpuContext->pCB->OsResource.bo)
    {
        pOsGpuContext->pCB->OsResource.bo = nullptr;
    }

 }

//!
//! \brief    Convert MOS format to gmm format
//! \details  convert MOS format to gmm format
//! \param    MOS_FORMAT format
//!           [in] Surface
//! \return   MEDIA_WA_TABLE *
//!           Returns the pointer to WA table
//!
GMM_RESOURCE_FORMAT Mos_Specific_ConvertMosFmtToGmmFmt(
    MOS_FORMAT format)
{
    switch (format)
    {
        case Format_Buffer      : return GMM_FORMAT_GENERIC_8BIT;
        case Format_Buffer_2D   : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_L8          : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_L16         : return GMM_FORMAT_L16_UNORM_TYPE;
        case Format_STMM        : return GMM_FORMAT_R8_UNORM_TYPE;              // matching size as format
        case Format_AI44        : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_IA44        : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_R5G6B5      : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Format_R8G8B8      : return GMM_FORMAT_R8G8B8_UNORM;
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
        case Format_P208        : return GMM_FORMAT_P208_TYPE;
        default                 : return GMM_FORMAT_INVALID;
    }
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
    void                    *caller;
    const char              *bufname;
    void                    *ptr;
    int32_t                 iSize;
    int32_t                 iPitch;
    unsigned long           ulPitch;
    MOS_STATUS              eStatus;
    MOS_LINUX_BO            *bo;
    MOS_TILE_TYPE           tileformat;
    uint32_t                tileformat_linux;
    int32_t                 iHeight;
    int32_t                 iAlignedHeight;
    GMM_RESCREATE_PARAMS    GmmParams;
    GMM_RESOURCE_INFO       *pGmmResourceInfo;
    GMM_RESOURCE_TYPE       resourceType;

    MOS_OS_FUNCTION_ENTER;

    if( nullptr == pOsResource)
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOSResource is NULL.");
        return  MOS_STATUS_INVALID_PARAMETER;
    }

    if( nullptr == pOsInterface)
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return  MOS_STATUS_INVALID_PARAMETER;
    }

    pOsResource->bConvertedFromDDIResource = false;

    bool osContextValid = false;
    if (pOsInterface->osContextPtr != nullptr)
    {
        if (pOsInterface->osContextPtr->GetOsContextValid() == true)
        {
            osContextValid = true;
        }
    }

    if ((pOsInterface->modulizedMosEnabled) && (!Mos_Solo_IsEnabled()) && (osContextValid == true))
    {
        pOsResource->pGfxResource = GraphicsResource::CreateGraphicResource(GraphicsResource::osSpecificResource);
        if (pOsResource->pGfxResource == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("input nullptr returned by GraphicsResource::CreateGraphicResource.");
            return MOS_STATUS_INVALID_HANDLE;
        }

        if (pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid osContextPtr.");
            return MOS_STATUS_INVALID_HANDLE;
        }

        GraphicsResource::SetMemAllocCounterGfx(MosMemAllocCounterGfx);

        GraphicsResource::CreateParams params(pParams);
        eStatus = pOsResource->pGfxResource->Allocate(pOsInterface->osContextPtr, params);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Allocate graphic resource failed");
            return MOS_STATUS_INVALID_HANDLE;
        }

        eStatus = pOsResource->pGfxResource->ConvertToMosResource(pOsResource);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Convert graphic resource failed");
            return MOS_STATUS_INVALID_HANDLE;
        }

        MosMemAllocCounterGfx = GraphicsResource::GetMemAllocCounterGfx();
        MOS_MEMNINJA_GFX_ALLOC_MESSAGE(pOsResource->pGmmResInfo, GmmResGetRenderSize(pOsResource->pGmmResInfo), functionName, filename, line);

        return eStatus;
    }

    caller              = nullptr;
    bufname             = pParams->pBufName;
    tileformat_linux    = I915_TILING_NONE;
    iAlignedHeight      = iHeight = pParams->dwHeight;
    eStatus             = MOS_STATUS_SUCCESS;
    resourceType        = RESOURCE_2D;
    tileformat          = pParams->TileType;

    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));

    if( nullptr == pOsInterface->pOsContext )
    {
        MOS_OS_ASSERTMESSAGE("input parameter pOsInterface is NULL.");
        return  MOS_STATUS_INVALID_PARAMETER;
    }
    switch (pParams->Format)
    {
        case Format_Buffer:
        case Format_RAW:
            resourceType                = RESOURCE_BUFFER;
            iAlignedHeight              = 1;
            //indicate buffer Restriction is Vertex.
            GmmParams.Flags.Gpu.State   = true;
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
        case Format_R16U:
        case Format_R8U:
        case Format_P010:
        case Format_P208:
            resourceType                = RESOURCE_2D;
            //indicate buffer Restriction is Planar surface restrictions.
            GmmParams.Flags.Gpu.Video   = true;
            break;
        default:
            MOS_OS_ASSERTMESSAGE("Unsupported format");
            eStatus = MOS_STATUS_UNIMPLEMENTED;
            goto finish;
    }

    // Create GmmResourceInfo
    GmmParams.BaseWidth             = pParams->dwWidth;
    GmmParams.BaseHeight            = iAlignedHeight;
    GmmParams.ArraySize             = 1;
    GmmParams.Type                  = resourceType;
    GmmParams.Format                = Mos_Specific_ConvertMosFmtToGmmFmt(pParams->Format);

    MOS_OS_CHECK_CONDITION(GmmParams.Format == GMM_FORMAT_INVALID,
                         "Unsupported format",
                         MOS_STATUS_UNKNOWN);

    switch(tileformat)
    {
        case MOS_TILE_Y:
            GmmParams.Flags.Info.TiledY    = true;
            GmmParams.Flags.Gpu.MMC        = pParams->bIsCompressed;
            tileformat_linux               = I915_TILING_Y;
            break;
        case MOS_TILE_X:
            GmmParams.Flags.Info.TiledX    = true;
            tileformat_linux               = I915_TILING_X;
            break;
        default:
            GmmParams.Flags.Info.Linear    = true;
            tileformat_linux               = I915_TILING_NONE;
    }

    pOsResource->pGmmResInfo = pGmmResourceInfo = pOsInterface->pOsContext->pGmmClientContext->CreateResInfoObject(&GmmParams);

    MOS_OS_CHK_NULL(pGmmResourceInfo);

    switch (pGmmResourceInfo->GetTileType())
    {
        case GMM_TILED_X:
            tileformat = MOS_TILE_X;
            tileformat_linux               = I915_TILING_X;
            break;
        case GMM_TILED_Y:
            tileformat = MOS_TILE_Y;
            tileformat_linux               = I915_TILING_Y;
            break;
        case GMM_NOT_TILED:
        default:
            tileformat = MOS_TILE_LINEAR;
            tileformat_linux               = I915_TILING_NONE;
            break;
    }

    if (pParams->TileType == MOS_TILE_Y)
    {
        GmmResSetMmcMode(pGmmResourceInfo, (GMM_RESOURCE_MMC_INFO)pParams->CompressionMode, 0);
    }

    iPitch      = GFX_ULONG_CAST(pGmmResourceInfo->GetRenderPitch());
    iSize       = GmmResGetRenderSize(pGmmResourceInfo);
    iHeight     = pGmmResourceInfo->GetBaseHeight();

#if defined(I915_PARAM_CREATE_VERSION)
    drm_i915_getparam_t gp;
    int32_t gpvalue;
    int32_t ret;
    gpvalue = 0;
    ret = -1;
    memset( &gp, 0, sizeof(gp) );
    gp.value = &gpvalue;
    gp.param = I915_PARAM_CREATE_VERSION;
    ret = drmIoctl(pOsInterface->pOsContext->fd, DRM_IOCTL_I915_GETPARAM, &gp );

    if ((0 == ret) && ( tileformat_linux != I915_TILING_NONE))
    {
        bo = mos_bo_alloc_tiled(pOsInterface->pOsContext->bufmgr, bufname, iPitch, iSize/iPitch, 1,
                 &tileformat_linux, &ulPitch, BO_ALLOC_STOLEN);
        if (nullptr == bo)
        {
            bo = mos_bo_alloc_tiled(pOsInterface->pOsContext->bufmgr, bufname, iPitch, iSize/iPitch, 1, &tileformat_linux, &ulPitch, 0);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Stolen memory is created sucessfully on Mos");
        }
        iPitch = (int32_t)ulPitch;
    } else
#endif
    {
        // Only Linear and Y TILE supported
        if( tileformat_linux == I915_TILING_NONE )
        {
            bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, bufname, iSize, 4096);
        }
        else
        {
            bo = mos_bo_alloc_tiled(pOsInterface->pOsContext->bufmgr, bufname, iPitch, iSize/iPitch, 1, &tileformat_linux, &ulPitch, 0);
            iPitch = (int32_t)ulPitch;
        }

    }

    pOsResource->bMapped = false;
    if (bo)
    {
#ifdef ANDROID
        intel_ufo_bo_datatype_t datatype;

        datatype.value = 0;
        mos_bo_get_datatype(bo, &datatype.value);
        datatype.compression_mode = (uint32_t)pParams->CompressionMode;
        datatype.is_mmc_capable   = (uint32_t)GmmParams.Flags.Gpu.MMC;
        datatype.compression_hint = INTEL_UFO_BUFFER_HINT_MMC_COMPRESSED;
        mos_bo_set_datatype(bo, datatype.value);
#endif

        pOsResource->Format       = pParams->Format;
        pOsResource->iWidth       = pParams->dwWidth;
        pOsResource->iHeight      = iHeight;
        pOsResource->iPitch       = iPitch;
        pOsResource->iCount       = 0;
        pOsResource->bufname      = bufname;
        pOsResource->bo           = bo;
        pOsResource->TileType     = tileformat;
        pOsResource->pData        = (uint8_t*) bo->virt; //It is useful for batch buffer to fill commands
        MOS_OS_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource).",iSize, pParams->dwWidth, iHeight);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).",iSize, pParams->dwWidth, pParams->dwHeight);
        eStatus = MOS_STATUS_NO_SPACE;
    }

    MosMemAllocCounterGfx++;
    MOS_MEMNINJA_GFX_ALLOC_MESSAGE(pOsResource->pGmmResInfo, GmmResGetRenderSize(pOsResource->pGmmResInfo), functionName, filename, line);

finish:
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
#ifdef ANDROID
    intel_ufo_bo_datatype_t datatype;
#endif
    GMM_RESOURCE_INFO       *pGmmResourceInfo;
    GMM_DISPLAY_FRAME       gmmChannel;
    GMM_REQ_OFFSET_INFO     reqInfo[3];
    GMM_RESOURCE_FLAG       GmmFlags;
    MOS_STATUS              eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pResDetails);

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    GmmFlags = pGmmResourceInfo->GetResFlags();

    // Get resource information
    pResDetails->dwWidth         = GFX_ULONG_CAST(pGmmResourceInfo->GetBaseWidth());
    pResDetails->dwHeight        = pGmmResourceInfo->GetBaseHeight();
    pResDetails->dwPitch         = GFX_ULONG_CAST(pGmmResourceInfo->GetRenderPitch());
    pResDetails->dwDepth         = MOS_MAX(1, pGmmResourceInfo->GetBaseDepth());
    pResDetails->dwLockPitch     = GmmResGetLockPitch(pGmmResourceInfo);
    if (GFX_GET_CURRENT_RENDERCORE(pGmmGlobalContext->GetPlatformInfo().Platform) < IGFX_GEN8_CORE)
    {
        pResDetails->bArraySpacing = GmmResIsArraySpacingSingleLod(pGmmResourceInfo);
    }
    if (GFX_GET_CURRENT_RENDERCORE(pGmmGlobalContext->GetPlatformInfo().Platform) >= IGFX_GEN9_CORE)
    {
        pResDetails->dwQPitch = pGmmResourceInfo->GetQPitch();
    }

#ifdef ANDROID
    if(pOsResource->bo)
    {
        datatype.value = 0;
        mos_bo_get_datatype(pOsResource->bo, &datatype.value);
        pResDetails->bCompressible   =
            (datatype.is_mmc_capable && (datatype.compression_hint == INTEL_UFO_BUFFER_HINT_MMC_COMPRESSED)) ? true : false;
        pResDetails->bIsCompressed   = datatype.compression_mode ? true : false;
        pResDetails->CompressionMode = (MOS_RESOURCE_MMC_MODE)datatype.compression_mode;
    }
    else
    {
        pResDetails->bCompressible   = false;
        pResDetails->bIsCompressed   = false;
        pResDetails->CompressionMode = MOS_MMC_DISABLED;
    }
#else
    pResDetails->bCompressible   = GmmFlags.Gpu.MMC ?
        (GmmResGetMmcHint(pGmmResourceInfo, 0) == GMM_MMC_HINT_ON) : false;
    pResDetails->bIsCompressed   = GmmResIsMediaMemoryCompressed(pGmmResourceInfo, 0);
    pResDetails->CompressionMode = (MOS_RESOURCE_MMC_MODE)GmmResGetMmcMode(pGmmResourceInfo, 0);
#endif

    if (0 == pResDetails->dwPitch)
    {
        MOS_OS_ASSERTMESSAGE("Pitch from GmmResource is 0, unexpected.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    // check resource's tile type
    if( GmmFlags.Info.TiledY )
    {
        if( GmmFlags.Info.TiledYf )
        {
            pResDetails->TileType = MOS_TILE_YF;
        }
        else if( GmmFlags.Info.TiledYs )
        {
            pResDetails->TileType = MOS_TILE_YS;
        }
        else
        {
            pResDetails->TileType = MOS_TILE_Y;
        }
    }
    else if( GmmFlags.Info.TiledX )
    {
        pResDetails->TileType = MOS_TILE_X;
    }
    else
    {
        pResDetails->TileType = MOS_TILE_LINEAR;
    }

    pResDetails->Format   = pOsResource->Format;

    // Get planes
    MOS_ZeroMemory(reqInfo, sizeof(reqInfo));
    gmmChannel = GMM_DISPLAY_BASE;
    // Get the base offset of the surface (plane Y)
    reqInfo[2].ReqRender = true;
    reqInfo[2].Plane     = GMM_PLANE_Y;
    reqInfo[2].Frame     = gmmChannel;
    reqInfo[2].CubeFace  = __GMM_NO_CUBE_MAP;
    reqInfo[2].ArrayIndex = 0;
    pGmmResourceInfo->GetOffset(reqInfo[2]);
    pResDetails->RenderOffset.YUV.Y.BaseOffset = reqInfo[2].Render.Offset;

    // Get U/UV plane information (plane offset, X/Y offset)
    reqInfo[0].ReqRender = true;
    reqInfo[0].Plane     = GMM_PLANE_U;
    reqInfo[0].Frame     = gmmChannel;
    reqInfo[0].CubeFace  = __GMM_NO_CUBE_MAP;
    reqInfo[0].ArrayIndex = 0;
    pGmmResourceInfo->GetOffset(reqInfo[0]);

    pResDetails->RenderOffset.YUV.U.BaseOffset = reqInfo[0].Render.Offset;
    pResDetails->RenderOffset.YUV.U.XOffset    = reqInfo[0].Render.XOffset;
    pResDetails->RenderOffset.YUV.U.YOffset    = reqInfo[0].Render.YOffset;

    // Get V plane information (plane offset, X/Y offset)
    reqInfo[1].ReqRender = true;
    reqInfo[1].Plane     = GMM_PLANE_V;
    reqInfo[1].Frame     = gmmChannel;
    reqInfo[1].CubeFace  = __GMM_NO_CUBE_MAP;
    reqInfo[1].ArrayIndex = 0;
    pGmmResourceInfo->GetOffset(reqInfo[1]);

    pResDetails->RenderOffset.YUV.V.BaseOffset = reqInfo[1].Render.Offset;
    pResDetails->RenderOffset.YUV.V.XOffset    = reqInfo[1].Render.XOffset;
    pResDetails->RenderOffset.YUV.V.YOffset    = reqInfo[1].Render.YOffset;

finish:
    return eStatus;
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

    bool osContextValid = false;
    if (pOsInterface->osContextPtr != nullptr)
    {
        if (pOsInterface->osContextPtr->GetOsContextValid() == true)
        {
            osContextValid = true;
        }
    }

    if ((pOsInterface->modulizedMosEnabled)
     && (!pOsResource->bConvertedFromDDIResource)
     && (osContextValid == true)
     && (!Mos_Solo_IsEnabled())
     && (pOsResource->pGfxResource))
    {
        if (pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid osContextPtr.");
            return;
        }

        GraphicsResource::SetMemAllocCounterGfx(MosMemAllocCounterGfx);

        if (pOsResource && pOsResource->pGfxResource)
        {
            pOsResource->pGfxResource->Free(pOsInterface->osContextPtr);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Received an empty Graphics Resource, skip free");
        }
        MOS_Delete(pOsResource->pGfxResource);
        pOsResource->pGfxResource = nullptr;

        MosMemAllocCounterGfx = GraphicsResource::GetMemAllocCounterGfx();
        MOS_MEMNINJA_GFX_FREE_MESSAGE(pOsResource->pGmmResInfo, functionName, filename, line);
        MOS_ZeroMemory(pOsResource, sizeof(*pOsResource));
        return;
    }

    if (pOsResource && pOsResource->bo)
    {
        mos_bo_unreference((MOS_LINUX_BO *)(pOsResource->bo));

#ifndef ANDROID
        if ( pOsInterface->pOsContext != nullptr && pOsInterface->pOsContext->contextOffsetList.size()) {
          MOS_CONTEXT *pOsCtx = pOsInterface->pOsContext;
          auto item_ctx = pOsCtx->contextOffsetList.begin();

          for (;item_ctx != pOsCtx->contextOffsetList.end();)
          {
             if (item_ctx->target_bo == pOsResource->bo)
                item_ctx = pOsCtx->contextOffsetList.erase(item_ctx);
             else
                item_ctx++;
          }
        }
#endif
        pOsResource->bo = nullptr;
        if (nullptr != pOsResource->pGmmResInfo)
        {
            MosMemAllocCounterGfx--;
            MOS_MEMNINJA_GFX_FREE_MESSAGE(pOsResource->pGmmResInfo, functionName, filename, line);

            pOsInterface->pOsContext->pGmmClientContext->DestroyResInfoObject(pOsResource->pGmmResInfo);
            pOsResource->pGmmResInfo = nullptr;
        }
    }

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

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsResource);
    MOS_OS_ASSERT(pOsInterface->pOsContext);

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

    bool osContextValid = false;
    if (pOsInterface->osContextPtr != nullptr)
    {
        if (pOsInterface->osContextPtr->GetOsContextValid() == true)
        {
            osContextValid = true;
        }
    }

    if ((pOsInterface->modulizedMosEnabled)
       && (!pOsResource->bConvertedFromDDIResource)
       && (osContextValid == true)
       && (!Mos_Solo_IsEnabled())
       && (pOsResource->pGfxResource))
    {
        if (nullptr == pOsInterface->osContextPtr)
        {
            MOS_OS_ASSERTMESSAGE("invalid osContextPtr, skip lock");
        }

        if (pOsResource->pGfxResource)
        {
            GraphicsResource::LockParams params(pLockFlags);
            pData = pOsResource->pGfxResource->Lock(pOsInterface->osContextPtr, params);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Received an empty Graphics Resource, skip lock");
        }
        return pData;
    }

    pContext = pOsInterface->pOsContext;
    if (pOsResource && pOsResource->bo)
    {
        MOS_LINUX_BO *bo = pOsResource->bo;

        // Do decompression for a compressed surface before lock
        if (!pLockFlags->NoDecompress &&
             GmmResIsMediaMemoryCompressed(pOsResource->pGmmResInfo, 0))
        {
            PMOS_CONTEXT pOsContext = pOsInterface->pOsContext;

            MOS_OS_ASSERT(pOsContext);
            MOS_OS_ASSERT(pOsContext->ppMediaMemDecompState);
            MOS_OS_ASSERT(pOsContext->pfnMemoryDecompress);
            pOsContext->pfnMemoryDecompress(pOsContext, pOsResource);
        }

        if(false == pOsResource->bMapped)
        {
            if (pContext->bIsAtomSOC)
            {
                mos_gem_bo_map_gtt(bo);
            }
            else
            {
#ifdef ANDROID
                if (pOsResource->TileType != MOS_TILE_LINEAR ||pLockFlags->Uncached)
                {
                    mos_gem_bo_map_gtt(bo);
                }
                else
                {
                    mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY&pLockFlags->WriteOnly));
                }
#else
                if (pOsResource->TileType != MOS_TILE_LINEAR && !pLockFlags->TiledAsTiled)
                {
                    mos_gem_bo_map_gtt(bo);
                    pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP_GTT;
                }
                else if (pLockFlags->Uncached)
                {
                    mos_gem_bo_map_wc(bo);
                    pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP_WC;
                }
                else
                {
                    mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY&pLockFlags->WriteOnly));
                    pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
                }
#endif
            }
            pOsResource->pData   = (uint8_t*)bo->virt;
            pOsResource->bMapped = true;
        }

        pData = pOsResource->pData;

    }

finish:
    MOS_OS_ASSERT(pData);
    return pData;
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
    MOS_STATUS           eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CONTEXT      *pContext = nullptr;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);

    bool osContextValid;
    osContextValid = false;
    if (pOsInterface->osContextPtr != nullptr)
    {
        if (pOsInterface->osContextPtr->GetOsContextValid() == true)
        {
            osContextValid = true;
        }
    }

    if ((pOsInterface->modulizedMosEnabled)
     && (!pOsResource->bConvertedFromDDIResource)
     && (osContextValid == true)
     && (!Mos_Solo_IsEnabled())
     && (pOsResource->pGfxResource))
    {
        if (nullptr == pOsInterface->osContextPtr)
        {
            MOS_OS_ASSERTMESSAGE("invalid osContextPtr, skip lock");
        }

        if (pOsResource->pGfxResource)
        {
            eStatus = pOsResource->pGfxResource->Unlock(pOsInterface->osContextPtr);
        }
        else
        {
            MOS_OS_VERBOSEMESSAGE("Received an empty Graphics Resource, skip unlock");
        }
        return eStatus;
    }

    pContext = pOsInterface->pOsContext;

    if(pOsResource->bo)
    {
        if(true == pOsResource->bMapped)
        {
           if (pContext->bIsAtomSOC)
           {
               mos_gem_bo_unmap_gtt(pOsResource->bo);
           }
           else
           {
#ifdef ANDROID
               if (pOsResource->TileType == MOS_TILE_LINEAR)
               {
                   mos_bo_unmap(pOsResource->bo);
               }
               else
               {
                   mos_gem_bo_unmap_gtt(pOsResource->bo);
               }
#else
               switch(pOsResource->MmapOperation)
               {
                   case MOS_MMAP_OPERATION_MMAP_GTT:
                        mos_gem_bo_unmap_gtt(pOsResource->bo);
                        break;
                   case MOS_MMAP_OPERATION_MMAP_WC:
                        mos_gem_bo_unmap_wc(pOsResource->bo);
                        break;
                   case MOS_MMAP_OPERATION_MMAP:
                        mos_bo_unmap(pOsResource->bo);
                        break;
                   default:
                        MOS_OS_ASSERTMESSAGE("Invalid mmap operation type");
                        break;
               }
#endif
           }
           pOsResource->bo->virt = nullptr;
           pOsResource->bMapped  = false;
        }
        pOsResource->pData       = nullptr;
    }

finish:
    return eStatus;
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
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CONTEXT          *pContext = nullptr;

    //---------------------------------------
    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);
    //---------------------------------------

    pContext = pOsInterface->pOsContext;

    if (pOsResource && pOsResource->bo)
    {
        MOS_LINUX_BO *bo = pOsResource->bo;
        if (GmmResIsMediaMemoryCompressed(pOsResource->pGmmResInfo, 0))
        {
            PMOS_CONTEXT pOsContext = pOsInterface->pOsContext;

            MOS_OS_CHK_NULL(pOsContext);
            MOS_OS_CHK_NULL(pOsContext->ppMediaMemDecompState);
            MOS_OS_CHK_NULL(pOsContext->pfnMemoryDecompress);
            pOsContext->pfnMemoryDecompress(pOsContext, pOsResource);
        }
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->SetPatchEntry(pOsInterface, pParams));
    }

    PMOS_OS_CONTEXT         pOsContext;
    MOS_OS_GPU_CONTEXT      *pOsGpuContext;
    PPATCHLOCATIONLIST      pPatchList;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    pOsContext      = pOsInterface->pOsContext;
    pOsGpuContext   = &pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];
    pPatchList      = pOsGpuContext->pPatchLocationList;

    pPatchList[pOsGpuContext->uiCurrentNumPatchLocations].AllocationIndex     = pParams->uiAllocationIndex;
    pPatchList[pOsGpuContext->uiCurrentNumPatchLocations].AllocationOffset    = pParams->uiResourceOffset;
    pPatchList[pOsGpuContext->uiCurrentNumPatchLocations].PatchOffset         = pParams->uiPatchOffset;
    pPatchList[pOsGpuContext->uiCurrentNumPatchLocations].uiWriteOperation    = pParams->bWrite;

    if (pOsInterface->osCpInterface &&
        pOsInterface->osCpInterface->IsHMEnabled())
    {
        pOsInterface->osCpInterface->RegisterPatchForHM(
            (uint32_t*)(pParams->cmdBufBase + pParams->uiPatchOffset),
            pParams->bWrite,
            pParams->HwCommandType,
            pParams->forceDwordOffset,
            pParams->presResource,
            &pPatchList[pOsGpuContext->uiCurrentNumPatchLocations]);
    }

    pOsGpuContext->uiCurrentNumPatchLocations++;

    return eStatus;
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
    MOS_OS_CHK_NULL_RETURN(pOsResource);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

    #if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
        gpuContext->PushCmdResPtr(static_cast<const void *>(pOsResource));
    #endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

        return (gpuContext->RegisterResource(pOsResource, bWrite));
    }

    PMOS_OS_CONTEXT     pOsContext;
    PMOS_RESOURCE       pResources;
    uint32_t            uiAllocation;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_GPU_CONTEXT  *pOsGpuContext;
    MOS_UNUSED(bWritebSetResourceSyncTag);

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);

    pOsContext          = pOsInterface->pOsContext;
    pOsGpuContext       = &pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];

    // Find previous registration
    pResources = pOsGpuContext->pResources;
    if( nullptr == pResources)
    {
        MOS_OS_ASSERTMESSAGE("pResouce is NULL.");
        return MOS_STATUS_SUCCESS;
    }
    for (uiAllocation = 0;
         uiAllocation < pOsGpuContext->uiResCount;
         uiAllocation++, pResources++)
    {
        if (pOsResource->bo == pResources->bo) break;
    }
    // Allocation list to be updated
    if (uiAllocation < pOsGpuContext->uiMaxNumAllocations)
    {
        // New buffer
        if (uiAllocation == pOsGpuContext->uiResCount)
        {
            pOsGpuContext->uiResCount++;
        }

        // Set allocation
        pOsResource->iAllocationIndex[pOsInterface->CurrentGpuContextOrdinal] = uiAllocation;
        pOsGpuContext->pResources[uiAllocation]                     = *pOsResource;
        pOsGpuContext->pbWriteMode[uiAllocation]                   |= bWrite;
        pOsGpuContext->pAllocationList[uiAllocation].hAllocation    = &pOsGpuContext->pResources[uiAllocation];
        pOsGpuContext->pAllocationList[uiAllocation].WriteOperation|= bWrite;
        pOsGpuContext->uiNumAllocations                             = pOsGpuContext->uiResCount;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Reached max # registrations.");
        eStatus = MOS_STATUS_UNKNOWN;
    }
    return eStatus;
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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->VerifyCommandBufferSize(dwRequestedSize));
    }

    PMOS_OS_CONTEXT    pOsContext;
    MOS_OS_GPU_CONTEXT OsGpuContext;

    //---------------------------------------
    MOS_UNUSED(dwFlags);
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //---------------------------------------

    pOsContext      = pOsInterface->pOsContext;
    OsGpuContext    = pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];

    if (OsGpuContext.uiCommandBufferSize < dwRequestedSize)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pCmdBuffer);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->GetCommandBuffer(pCmdBuffer, dwFlags));
    }

    PMOS_OS_CONTEXT         pOsContext;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PMOS_OS_GPU_CONTEXT     pOsGpuContext;
    uint32_t                uiCommandBufferSize;

    MOS_UNUSED(dwFlags);
    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);
    MOS_OS_CHK_NULL(pCmdBuffer);

    // Activate software context
    pOsContext = pOsInterface->pOsContext;
    pOsGpuContext        = &pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];
    uiCommandBufferSize = pOsGpuContext->uiCommandBufferSize;

    if(pOsGpuContext->bCBFlushed == true)
    {
        if (pOsContext->pfnGetCommandBuffer(pOsContext,
            pCmdBuffer,
            uiCommandBufferSize))
        {
            MOS_OS_CHK_STATUS(pOsContext->pfnInsertCmdBufferToPool(pOsContext, pCmdBuffer));
            pOsGpuContext->bCBFlushed = false;
            eStatus = MOS_SecureMemcpy(pOsGpuContext->pCB, sizeof(MOS_COMMAND_BUFFER), pCmdBuffer, sizeof(MOS_COMMAND_BUFFER));
            MOS_OS_CHECK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "Failed to copy command buffer", eStatus);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Failed to activate command buffer.");
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }

    MOS_OS_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface, &(pOsGpuContext->pCB->OsResource), false, false));
    eStatus = MOS_SecureMemcpy(pCmdBuffer, sizeof(MOS_COMMAND_BUFFER), pOsGpuContext->pCB, sizeof(MOS_COMMAND_BUFFER));
    MOS_OS_CHECK_CONDITION((eStatus != MOS_STATUS_SUCCESS), "Failed to copy command buffer", eStatus);

finish:
    return eStatus;
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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        MOS_OS_CHK_STATUS_RETURN(gpuContext->SetIndirectStateSize(uSize));
    }

    PMOS_CONTEXT   pOsContext;
    MOS_STATUS     eStatus;

    MOS_OS_CHK_NULL(pOsInterface);

    eStatus = MOS_STATUS_SUCCESS;

    pOsContext = pOsInterface->pOsContext;
    MOS_OS_CHK_NULL(pOsContext);

    pOsContext->uIndirectStateSize = uSize;

finish:
    return eStatus;
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->GetIndirectState(puOffset, puSize));
    }

    PMOS_CONTEXT       pOsContext;
    MOS_OS_GPU_CONTEXT OsGpuContext;

    pOsContext = pOsInterface->pOsContext;
    if (pOsContext)
    {
        OsGpuContext = pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];

        if (puOffset)
        {
            *puOffset = OsGpuContext.uiCommandBufferSize - pOsContext->uIndirectStateSize;
        }

        if (puSize)
        {
            *puSize = pOsContext->uIndirectStateSize;
        }
    }
    return MOS_STATUS_SUCCESS;
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

    if (pResource)
    {
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
    PMOS_OS_CONTEXT     pOsContext;
    MOS_OS_GPU_CONTEXT  OsGpuContext;
    MOS_STATUS          eStatus;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pIndirectState);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->GetIndirectStatePointer(pIndirectState));
    }

    eStatus    = MOS_STATUS_UNKNOWN;

    pOsContext = (pOsInterface) ? pOsInterface->pOsContext : nullptr;

    if (pOsContext)
    {
        OsGpuContext = pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];

        if (OsGpuContext.pCB && OsGpuContext.pCB->pCmdBase)
        {
            *pIndirectState =
                (uint8_t*)OsGpuContext.pCB->pCmdBase   +
                OsGpuContext.uiCommandBufferSize    -
                pOsContext->uIndirectStateSize;

            eStatus = MOS_STATUS_SUCCESS;
        }
    }

    return eStatus;
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);

        if (gpuContext)
        {
            gpuContext->ReturnCommandBuffer(pCmdBuffer, dwFlags);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Cannot get valid Gpu context!");
        }

        return;
    }

    PMOS_OS_CONTEXT         pOsContext;
    int32_t                 bResult;

    MOS_UNUSED(dwFlags);

    bResult = false;

    // Validate parameters
    pOsContext = (pOsInterface) ? pOsInterface->pOsContext : nullptr;
    if (pOsContext == nullptr || pCmdBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameters.");
        goto finish;
    }

    pOsContext->pfnReturnCommandBuffer(pOsContext,pOsInterface->CurrentGpuContextOrdinal, pCmdBuffer);

finish:
        return;
}
#if (_DEBUG || _RELEASE_INTERNAL)
MOS_LINUX_BO * Mos_GetNopCommandBuffer_Linux(
    PMOS_INTERFACE        pOsInterface)
{
    int j;
    uint32_t *buf;
    MOS_LINUX_BO* bo;

    j = 0;

    if(pOsInterface == nullptr || pOsInterface->pOsContext == nullptr)
    {
        return nullptr;
    }

    bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, "NOP_CMD_BO", 4096, 4096);
    if(bo == nullptr)
    {
        return nullptr;
    }

    mos_bo_map(bo, 1);
    buf = (uint32_t*)bo->virt;
    if(buf == nullptr)
    {
        mos_bo_unreference(bo);
        return nullptr;
    }

    buf[j++] = 0x05000000; // MI_BATCH_BUFFER_END

    mos_bo_unmap(bo);

    return bo;
}

MOS_LINUX_BO * Mos_GetBadCommandBuffer_Linux(
    PMOS_INTERFACE        pOsInterface)
{
    int j;
    uint32_t *buf;
    MOS_LINUX_BO* bo;

    j = 0;

    if(pOsInterface == nullptr || pOsInterface->pOsContext == nullptr)
    {
        return nullptr;
    }

    bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, "BAD_CMD_BO", 4096, 4096);
    if(bo == nullptr)
    {
        return nullptr;
    }

    mos_bo_map(bo, 1);
    buf = (uint32_t*)bo->virt;
    if(buf == nullptr)
    {
        mos_bo_unreference(bo);
        return nullptr;
    }

    /* Hanging batch buffer */
    // Semaphore setup:
    // Set up GTT buffer, polling mode with
    // comparison: address > op data (0xffffffff),
    // which should never resolve since nothing
    // is bigger than 0xffffffff. This semaphore
    // should result in a hang that TDR will have
    // to handle.
    //
    // 31:29 = 0
    // 28:23 = 0x1c
    // ---> 31:24 = 0x0e, 23=0x0
    // 22:22 = 0x1    (GTT, must be secure batch)
    // 21:16 = 0x0
    // ---> 23:20 = 0x4, 19:16=0x0
    // 15:15 = 0x1 (polling mode)
    // 14:12 = 0x0: memory data > op data (set op data = 0xffffffff and we should never have a match... if the comparison is unsigned)
    // ---> 15:12 = 0x8
    // 11:08 = 0x0
    // 07:00 = 0x2
    // ---> 07:04 = 0x0, 03:00 = 0x2
    // 31:0 = 0xffffffff
    //
    buf[j++]    = 0x0e008002;

    // semaphore data
    //
    buf[j++] = 0xffffffff;

    // semaphore address
    //
    buf[j++] = 0x0;
    buf[j++] = 0x0;

    buf[j++] = 0x0; /* MI_NOOP */
    buf[j++] = 0x05000000; // MI_BATCH_BUFFER_END

    mos_bo_unmap(bo);

    return bo;
}
#endif // _DEBUG || _RELEASE_INTERNAL

uint32_t Mos_Specific_GetVcsExecFlag(PMOS_INTERFACE pOsInterface,
                            PMOS_COMMAND_BUFFER pCmdBuffer,
                            MOS_GPU_NODE GpuNode)
{
    uint32_t VcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;

    if (MOS_VDBOX_NODE_INVALID == pCmdBuffer->iVdboxNodeIndex)
    {
       // That's those case when BB did not have any VDBOX# specific commands.
       // Thus, we need to select VDBOX# here. Alternatively we can rely on KMD
       // to make balancing for us, i.e. rely on Virtual Engine support.
       pCmdBuffer->iVdboxNodeIndex = pOsInterface->pfnGetVdboxNodeId(pOsInterface, pCmdBuffer);
       if (MOS_VDBOX_NODE_INVALID == pCmdBuffer->iVdboxNodeIndex)
       {
           pCmdBuffer->iVdboxNodeIndex = (GpuNode == MOS_GPU_NODE_VIDEO)?
               MOS_VDBOX_NODE_1: MOS_VDBOX_NODE_2;
       }
     }

     if (MOS_VDBOX_NODE_1 == pCmdBuffer->iVdboxNodeIndex)
     {
         VcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
     }
     else if (MOS_VDBOX_NODE_2 == pCmdBuffer->iVdboxNodeIndex)
     {
         VcsExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
     }

     return VcsExecFlag;
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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pCmdBuffer);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

    #if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
        auto &cmdPtrs = gpuContext->GetCmdResPtrs();
        pOsInterface->gpuCmdResInfoDump->SetTitle("PerfTag: " + std::to_string(pOsInterface->pfnGetPerfTag(pOsInterface)));
        pOsInterface->gpuCmdResInfoDump->Dump(cmdPtrs);
        gpuContext->ClearCmdResPtrs();
    #endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

        return (gpuContext->SubmitCommandBuffer(pOsInterface, pCmdBuffer, bNullRendering));
    }

    PMOS_CONTEXT            pOsContext;
    PMOS_RESOURCE           pResource;
    PMOS_OS_GPU_CONTEXT     pOsGpuContext;
    MOS_GPU_CONTEXT         GpuContext;
    MOS_GPU_NODE            GpuNode;
    uint32_t                PatchIndex, AllocationIndex, CmdBufferSize;
    MOS_STATUS              eStatus;
    PPATCHLOCATIONLIST      pPatchList,pCurrentPatch;
    MOS_LINUX_BO            *alloc_bo, *cmd_bo;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_LINUX_BO            *bad_cmd_bo;
    MOS_LINUX_BO            *nop_cmd_bo;
    uint32_t                dwComponentTag;
    uint32_t                dwCallType;
#endif // (_DEBUG || _RELEASE_INTERNAL)
    int32_t                 ResourceOffset,PatchOffset;
    uint32_t                dwBatchBufferEndCmd;
    MOS_CP_COMMAND_PROPERTIES           cpCmdProps;
    uint32_t                            dwAddCb2;
    PLATFORM                            platform;
    int32_t                             PerfData;
    uint32_t                            ExecFlag;
    drm_clip_rect_t                     *cliprects;
    int32_t                             num_cliprects;
    int32_t                             DR4, ret;
#ifndef ANDROID
    uint64_t                            boOffset;

    boOffset = 0;
#endif
    dwAddCb2 = 0xffffffff;
    eStatus  = MOS_STATUS_SUCCESS;
    ret      = 0;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->osCpInterface);

    pOsContext = pOsInterface->pOsContext;
    MOS_OS_CHK_NULL(pOsContext);

    GpuContext = pOsInterface->CurrentGpuContextOrdinal;

    GpuNode = OSKMGetGpuNode(GpuContext);
    ExecFlag = GpuNode;

    MOS_OS_CHK_NULL(pOsContext->OsGpuContext);
    pOsGpuContext = &pOsContext->OsGpuContext[GpuContext];
    MOS_OS_CHK_NULL(pOsGpuContext);

    pPatchList = pOsGpuContext->pPatchLocationList;
    MOS_OS_CHK_NULL(pPatchList);

    pOsInterface->pfnGetPlatform(pOsInterface,&platform);

    // Allocate command buffer from video memory
    CmdBufferSize = (pCmdBuffer->pCmdPtr - pCmdBuffer->pCmdBase)*4;// pCmdBuffer->OsResource.iPitch;        // ??? Not 100% sure about this ...

    // Command buffer object DRM pointer
    pOsGpuContext->bCBFlushed = true;
    cmd_bo = pCmdBuffer->OsResource.bo;

    // Now, the patching will be done, based on the patch list.
    for (PatchIndex = 0; PatchIndex <  pOsGpuContext->uiCurrentNumPatchLocations; PatchIndex++)
    {
        pCurrentPatch   = &pPatchList[PatchIndex];
        MOS_OS_CHK_NULL(pCurrentPatch);

        AllocationIndex = pCurrentPatch->AllocationIndex;
        ResourceOffset  = pCurrentPatch->AllocationOffset;
        PatchOffset     = pCurrentPatch->PatchOffset;

        // This is the resource for which patching will be done
        pResource       = (PMOS_RESOURCE)pOsGpuContext->pAllocationList[AllocationIndex].hAllocation;
        MOS_OS_CHK_NULL(pResource);

        // For now, we'll assume the system memory's DRM bo pointer
        // is NULL.  If nullptr is detected, then the resource has been
        // placed inside the command buffer's indirect state area.
        // We'll simply set alloc_bo to the command buffer's bo pointer.
        MOS_OS_ASSERT(pResource->bo);

        alloc_bo = (pResource->bo) ? pResource->bo : cmd_bo;

        MOS_OS_CHK_STATUS(pOsInterface->osCpInterface->PermeatePatchForHM(
            cmd_bo->virt,
            pCurrentPatch,
            pResource));

#ifndef ANDROID
        boOffset = alloc_bo->offset64;
        if (alloc_bo != cmd_bo)
        {
          auto item_ctx = pOsContext->contextOffsetList.begin();
          for (; item_ctx != pOsContext->contextOffsetList.end(); item_ctx++)
          {
             if (item_ctx->intel_context == pOsContext->intel_context && item_ctx->target_bo == alloc_bo)
             {
               boOffset = item_ctx->offset64;
               break;
             }
          }
        }
        if (pOsContext->bUse64BitRelocs)
        {
            *((uint64_t*)((uint8_t*)cmd_bo->virt + PatchOffset)) = boOffset + ResourceOffset;
        }
        else
        {
            *((uint32_t*)((uint8_t*)cmd_bo->virt + PatchOffset)) = boOffset + ResourceOffset;
        }

        // This call will patch the command buffer with the offsets of the indirect state region of the command buffer
        ret = mos_bo_emit_reloc2(
                          cmd_bo,                                                              // Command buffer
                          PatchOffset,                                                         // Offset in the command buffer
                          alloc_bo,                                                            // Allocation object for which the patch will be made.
                          ResourceOffset,                                                      // Offset to the indirect state
                          I915_GEM_DOMAIN_RENDER,                                              // Read domain
                          (pCurrentPatch->uiWriteOperation) ? I915_GEM_DOMAIN_RENDER : 0x0,   // Write domain
                          boOffset);
#else
        if (pOsContext->bUse64BitRelocs)
        {
            *((uint64_t*)((uint8_t*)cmd_bo->virt + PatchOffset)) = alloc_bo->offset64 + ResourceOffset;
        }
        else
        {
            *((uint32_t*)((uint8_t*)cmd_bo->virt + PatchOffset)) = alloc_bo->offset64 + ResourceOffset;
        }

        // This call will patch the command buffer with the offsets of the indirect state region of the command buffer
        ret = mos_bo_emit_reloc(
                          cmd_bo,                                                              // Command buffer
                          PatchOffset,                                                         // Offset in the command buffer
                          alloc_bo,                                                            // Allocation object for which the patch will be made.
                          ResourceOffset,                                                      // Offset to the indirect state
                          I915_GEM_DOMAIN_RENDER,                                              // Read domain
                          (pCurrentPatch->uiWriteOperation) ? I915_GEM_DOMAIN_RENDER : 0x0);   // Write domain

#endif
        if (ret != 0)
        {
            MOS_OS_ASSERTMESSAGE("Error patching alloc_bo = 0x%x, cmd_bo = 0x%x.",
                               (uintptr_t)alloc_bo,(uintptr_t)cmd_bo);
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
        }
    }

    //Add Batch buffer End Command
    dwBatchBufferEndCmd = MI_BATCHBUFFER_END;
    if (MOS_FAILED(Mos_AddCommand(
                       pCmdBuffer,
                       &dwBatchBufferEndCmd,
                       sizeof(uint32_t))))
    {
        MOS_OS_ASSERTMESSAGE("Inserting BB_END failed!");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    // Now, we can unmap the video command buffer, since we don't need CPU access anymore.
    mos_bo_unmap(cmd_bo);

    if(pOsContext->pPerfData != nullptr)
    {
        PerfData = *(int32_t *)(pOsContext->pPerfData);
    }
    else
    {
        PerfData = 0;
    }

    cliprects = nullptr;
    num_cliprects = 0;
    DR4 = pOsContext->uEnablePerfTag ? PerfData : 0;

    if (GpuNode == I915_EXEC_RENDER)
    {
         if (true == pOsInterface->osCpInterface->IsHMEnabled())
         {
              cliprects = (drm_clip_rect*)(&dwAddCb2);
              num_cliprects = sizeof(dwAddCb2);
         }
    }
    else
    {
        if (pOsContext->bKMDHasVCS2)
        {
            if (pOsContext->bPerCmdBufferBalancing && pOsInterface->pfnGetVdboxNodeId)
            {
                ExecFlag = Mos_Specific_GetVcsExecFlag(pOsInterface, pCmdBuffer, GpuNode);
            }
            else if (GpuNode == MOS_GPU_NODE_VIDEO)
            {
                ExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING1;
            }
            else if (GpuNode == MOS_GPU_NODE_VIDEO2)
            {
                ExecFlag = I915_EXEC_BSD | I915_EXEC_BSD_RING2;
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // trigger GPU HANG if bTriggerCodecHang is set
    bad_cmd_bo = nullptr;

    //dwComponentTag 3: decode,5: vpp,6: encode
    //dwCallType     8: PAK(CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE)
    //               34:PREENC
    //               5: VPP
    dwComponentTag = (PerfData & 0xF000) >> 12;
    dwCallType     = (PerfData & 0xFC) >> 2;

    if(pOsInterface->bTriggerCodecHang &&
        (dwComponentTag == 3 || (dwComponentTag == 6 && dwCallType == 8) ||
        (dwComponentTag == 6 && dwCallType == 34) ||
        (dwComponentTag == 5 && dwCallType == 5))
      )
    {
        bad_cmd_bo = Mos_GetBadCommandBuffer_Linux(pOsInterface);
        if(bad_cmd_bo)
        {
            ret = mos_bo_mrb_exec(bad_cmd_bo,
                                        4096,
                                        nullptr,
                                        0,
                                        0,
                                        ExecFlag);
        }
    }
    else if (pOsInterface->bTriggerVPHang == true)
    {
        bad_cmd_bo = Mos_GetBadCommandBuffer_Linux(pOsInterface);

        if (bad_cmd_bo)
        {
            ret = mos_bo_mrb_exec(bad_cmd_bo,
                                        4096,
                                        nullptr,
                                        0,
                                        0,
                                        ExecFlag);
        }

        pOsInterface->bTriggerVPHang = false;
    }

    nop_cmd_bo = nullptr;
    if (bNullRendering == true)
    {
        nop_cmd_bo = Mos_GetNopCommandBuffer_Linux(pOsInterface);

        if(nop_cmd_bo)
        {
            ret = mos_bo_mrb_exec(nop_cmd_bo,
                                        4096,
                                        nullptr,
                                        0,
                                        0,
                                        ExecFlag);
        }
    }

#endif //(_DEBUG || _RELEASE_INTERNAL)

    if (GpuNode != I915_EXEC_RENDER &&
        pOsInterface->osCpInterface->IsTearDownHappen())
    {
        // to skip PAK command when CP tear down happen to avoid of GPU hang
        // conditonal batch buffer start PoC is in progress
    }
    else if (bNullRendering == false)
    {
#ifdef ANDROID
     if (pOsContext->uEnablePerfTag == 2)
     {
         ret = mos_gem_bo_tag_exec(cmd_bo,
                                        pOsGpuContext->uiCommandBufferSize,
                                        pOsContext->intel_context,
                                        cliprects,
                                        num_cliprects,
                                        0,
                                        ExecFlag,
                                        PerfData);
     }
     else
     {
           ret = mos_bo_mrb_exec(cmd_bo,
                                       pOsGpuContext->uiCommandBufferSize,
                                       cliprects,
                                       num_cliprects,
                                       DR4,
                                       ExecFlag);
     }
#else
     ret = mos_gem_bo_context_exec2(cmd_bo,
                                       pOsGpuContext->uiCommandBufferSize,
                                       pOsContext->intel_context,
                                       cliprects,
                                       num_cliprects,
                                       DR4,
                                       ExecFlag);
#endif

      if (ret != 0) {
          eStatus = MOS_STATUS_UNKNOWN;
      }
    }

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Command buffer submission failed!");
    }

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    if (pOsInterface->bDumpCommandBuffer)
    {
        mos_bo_map(cmd_bo, 0);
        pOsInterface->pfnDumpCommandBuffer(pOsInterface, pCmdBuffer);
        mos_bo_unmap(cmd_bo);
    }
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)
    if(bad_cmd_bo)
    {
        mos_bo_wait_rendering(bad_cmd_bo);
        mos_bo_unreference(bad_cmd_bo);
    }
    if(nop_cmd_bo)
    {
        mos_bo_unreference(nop_cmd_bo);
    }
#endif //(_DEBUG || _RELEASE_INTERNAL)

    //clear command buffer relocations to fix memory leak issue
    mos_gem_bo_clear_relocs(cmd_bo, 0);

    // Reset resource allocation
    pOsGpuContext->uiNumAllocations = 0;
    MOS_ZeroMemory(pOsGpuContext->pAllocationList, sizeof(ALLOCATION_LIST) * pOsGpuContext->uiMaxNumAllocations);
    pOsGpuContext->uiCurrentNumPatchLocations = 0;
    MOS_ZeroMemory(pOsGpuContext->pPatchLocationList, sizeof(PATCHLOCATIONLIST) * pOsGpuContext->uiMaxPatchLocationsize);
    pOsGpuContext->uiResCount = 0;

    MOS_ZeroMemory(pOsGpuContext->pbWriteMode, sizeof(int32_t) * pOsGpuContext->uiMaxNumAllocations);
finish:
    return eStatus;
}

//!
//! \brief    Wait and release command buffer
//! \details  Wait and release command buffer
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer control structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_WaitAndReleaseCmdBuffer(
    PMOS_INTERFACE        pOsInterface,
    PMOS_COMMAND_BUFFER   pCmdBuffer)
{
    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto cmd_bo = pCmdBuffer->OsResource.bo;
        MOS_OS_CHK_NULL_RETURN(cmd_bo);

        // only wait rendering here, release will covered by gpucontext
        mos_bo_wait_rendering(cmd_bo);

        return MOS_STATUS_SUCCESS;
    }

    PMOS_CONTEXT            pOsContext;
    MOS_STATUS              eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pCmdBuffer);

    pOsContext = pOsInterface->pOsContext;
    MOS_OS_CHK_NULL(pOsContext);

    MOS_OS_CHK_STATUS(pOsContext->pfnWaitAndReleaseCmdBuffer(pOsContext, pCmdBuffer->iCmdIndex));

finish:
    return eStatus;
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
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pResource);
    return 0;
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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->ResizeCommandBufferAndPatchList(dwRequestedCommandBufferSize, dwRequestedPatchListSize, dwFlags));
    }

    PMOS_CONTEXT            pOsContext;
    PMOS_OS_GPU_CONTEXT     pOsGpuContext;
    PPATCHLOCATIONLIST      pNewPatchList;
    MOS_STATUS              eStatus;

    MOS_OS_FUNCTION_ENTER;

    //---------------------------------------
    MOS_UNUSED(dwFlags);
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //---------------------------------------

    eStatus = MOS_STATUS_SUCCESS;

    pOsContext    = pOsInterface->pOsContext;
    pOsGpuContext = &(pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal]);

    // uiCommandBufferSize is used for allocate command buffer and submit command buffer, in this moment, command buffer has not allocated yet.
    // Linux KMD requires command buffer size align to 8 bytes, or it will not execute the commands.
    pOsGpuContext->uiCommandBufferSize = MOS_ALIGN_CEIL(dwRequestedCommandBufferSize, 8);

    if (dwRequestedPatchListSize > pOsGpuContext->uiMaxPatchLocationsize)
    {
        pNewPatchList = (PPATCHLOCATIONLIST)realloc(
            pOsGpuContext->pPatchLocationList,
            sizeof(PATCHLOCATIONLIST) * dwRequestedPatchListSize);
        if (nullptr == pNewPatchList)
        {
            MOS_OS_ASSERTMESSAGE("pOsGpuContext->pPatchLocationList realloc failed.");
            eStatus = MOS_STATUS_NO_SPACE;
            goto finish;
        }

        pOsGpuContext->pPatchLocationList = pNewPatchList;

        // now zero the extended portion
        MOS_ZeroMemory(
            (pOsGpuContext->pPatchLocationList + pOsGpuContext->uiMaxPatchLocationsize),
            sizeof(PATCHLOCATIONLIST) * (dwRequestedPatchListSize - pOsGpuContext->uiMaxPatchLocationsize));
        pOsGpuContext->uiMaxPatchLocationsize = dwRequestedPatchListSize;
    }

finish:
    return eStatus;
}

//!
//! \brief    Resizes the buffer to be used for rendering GPU commands
//! \details  return true if succeeded - command buffer will be large enough to hold dwMaxSize
//!           return false if failed or invalid parameters
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    uint32_t dwRequestedSize
//!           [in] requested size
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_ResizeCommandBuffer(
    PMOS_INTERFACE          pOsInterface,
    uint32_t                dwRequestedSize)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->ResizeCommandBuffer(dwRequestedSize));
    }

    PMOS_CONTEXT          pOsContext;
    PMOS_OS_GPU_CONTEXT   pOsGpuContext;
    MOS_GPU_CONTEXT       GpuContext;
    MOS_STATUS            eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);

    pOsContext = &pOsInterface->pOsContext[pOsInterface->CurrentGpuContextOrdinal];
    MOS_OS_CHK_NULL(pOsContext);

    GpuContext = pOsInterface->CurrentGpuContextOrdinal;

    MOS_OS_CHK_NULL(pOsContext->OsGpuContext);
    pOsGpuContext = &pOsContext->OsGpuContext[GpuContext];
    MOS_OS_CHK_NULL(pOsGpuContext);

    pOsGpuContext->uiCommandBufferSize = dwRequestedSize;

finish:
    return eStatus;
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

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

        auto pOsContextSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

        auto gpuContextMgr = pOsContextSpecific->GetGpuContextMgr();
        MOS_OS_CHK_NULL_RETURN(gpuContextMgr);

        auto cmdBufMgr = pOsContextSpecific->GetCmdBufMgr();
        MOS_OS_CHK_NULL_RETURN(cmdBufMgr);

        auto gpuContext = gpuContextMgr->CreateGpuContext(GpuNode, cmdBufMgr, mosGpuCxt);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        auto gpuContextSpecific  = static_cast<GpuContextSpecific *>(gpuContext);
        MOS_OS_CHK_NULL_RETURN(gpuContextSpecific);

        MOS_OS_CHK_STATUS_RETURN(gpuContextSpecific->Init(gpuContextMgr->GetOsContext()));

        pOsContextSpecific->SetGpuContextHandle(mosGpuCxt, gpuContextSpecific->GetGpuContextHandle());

        return MOS_STATUS_SUCCESS;
    }

    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(mosGpuCxt);
    MOS_UNUSED(GpuNode);
    MOS_UNUSED(createOption);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroy GPU context
//! \details  Destroy GPU context
//! \param    PMOS_INTERFACE pOsInterface
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osContextPtr);

        OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
        MOS_OS_CHK_NULL_RETURN(pOsContextSpecific);

        GPU_CONTEXT_HANDLE gpuContextHandle = pOsContextSpecific->GetGpuContextHandle(mosGpuCxt);

        GpuContextMgr *gpuContextMgr = pOsContextSpecific->GetGpuContextMgr();
        MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
        GpuContext *gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        gpuContextMgr->DestroyGpuContext(gpuContext);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(mosGpuCxt);
    return eStatus;
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
    uint32_t ComponentTag;
    PMOS_CONTEXT pOsContext = (pOsInterface) ? (PMOS_CONTEXT)pOsInterface->pOsContext : nullptr;

    if(pOsContext)
    {
        switch (pOsInterface->Component)
        {
            case COMPONENT_VPreP:
            case COMPONENT_VPCommon:
                ComponentTag = PERFTAG_VPREP;
                break;

            case COMPONENT_LibVA:
                ComponentTag = PERFTAG_LIBVA;
                break;

            case COMPONENT_CM:
                ComponentTag = PERFTAG_CM;
                break;

            case COMPONENT_Decode:
                ComponentTag = PERFTAG_DECODE;
                break;

            case COMPONENT_Encode:
                ComponentTag = PERFTAG_ENCODE;
                break;

            default:
                ComponentTag = 0xF000 & pOsContext->GetDmaBufID(pOsContext);
                break;
        }

        pOsContext->SetDmaBufID(pOsContext, ComponentTag | (PerfTag&0x0fff));
    }

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
    PMOS_CONTEXT pOsContext = (pOsInterface) ? (PMOS_CONTEXT)pOsInterface->pOsContext : nullptr;

    if (pOsContext == nullptr)
        return;

    if (pOsContext->pPerfData != nullptr)
    {
        pOsContext->pPerfData->bufferID = 0;
    }
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
    PMOS_CONTEXT pOsContext = (pOsInterface) ? (PMOS_CONTEXT)pOsInterface->pOsContext : nullptr;

    if (pOsContext == nullptr)
        return;

    if (pOsContext->pPerfData != nullptr)
    {
        pOsContext->pPerfData->bufferID++;
    }
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
    PMOS_CONTEXT pOsContext = (pOsInterface) ? (PMOS_CONTEXT)pOsInterface->pOsContext : nullptr;

    if (pOsContext == nullptr)
        return;

    if (pOsContext->pPerfData != nullptr)
    {
        pOsContext->pPerfData->frameID++;
    }
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
    uint32_t                perfTag;

    PMOS_CONTEXT osContext = (pOsInterface) ? (PMOS_CONTEXT)pOsInterface->pOsContext : nullptr;

    if (osContext == nullptr)
    {
        return 0;
    }

    perfTag = *(uint32_t *)(osContext->pPerfData);
    return perfTag;
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
    PMOS_CONTEXT pOsContext;

    //---------------------------------------
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //------------------------------------

    pOsContext = (PMOS_CONTEXT)pOsInterface->pOsContext;
    pOsContext->SetPerfHybridKernelID(pOsContext,KernelID);
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
    int32_t      bRet;
    PMOS_CONTEXT pOsContext;

    //---------------------------------------
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //------------------------------------

    pOsContext = (pOsInterface) ? (PMOS_CONTEXT)pOsInterface->pOsContext : nullptr;
    bRet       = false;

    if (pOsContext)
    {
        ComponentTag = 0xF000 & pOsContext->GetDmaBufID(pOsContext);

        switch (ComponentTag)
        {
            case PERFTAG_ENCODE:
            case PERFTAG_DECODE:
                bRet = true;
                break;

            default:
                bRet = false;
                break;
        }
    }

    return bRet;
}

//!
//! \brief    Convet from OS to Mos format
//! \details  need to refactor this function after libva ddi interface done, or after OS format defined
//!          for Linux/Android
//! \param    MOS_OS_FORMAT format
//!           [in] OS surface format
//! \return   MOS_FORMAT
//!           Return Mos format
//!
MOS_FORMAT Mos_Specific_FmtOsToMos(
    MOS_OS_FORMAT   format)
{
    switch (static_cast<int>(format))
    {
        case DDI_FORMAT_A8B8G8R8     : return Format_A8R8G8B8;
        case DDI_FORMAT_X8B8G8R8     : return Format_X8R8G8B8;
        case DDI_FORMAT_R32F         : return Format_R32F;
        case DDI_FORMAT_A8R8G8B8     : return Format_A8R8G8B8;
        case DDI_FORMAT_X8R8G8B8     : return Format_X8R8G8B8;
        case DDI_FORMAT_R5G6B5       : return Format_R5G6B5;
        case DDI_FORMAT_YUY2         : return Format_YUY2;
        case DDI_FORMAT_P8           : return Format_P8;
        case DDI_FORMAT_A8P8         : return Format_A8P8;
        case DDI_FORMAT_A8           : return Format_A8;
        case DDI_FORMAT_L8           : return Format_L8;
        case DDI_FORMAT_L16          : return Format_L16;
        case DDI_FORMAT_A4L4         : return Format_A4L4;
        case DDI_FORMAT_A8L8         : return Format_A8L8;
        case DDI_FORMAT_V8U8         : return Format_V8U8;
        case DDI_FORMAT_A16B16G16R16 : return Format_A16B16G16R16;
        case FOURCC_YVYU             : return Format_YVYU;
        case FOURCC_UYVY             : return Format_UYVY;
        case FOURCC_VYUY             : return Format_VYUY;
        case FOURCC_AYUV             : return Format_AYUV;
        case FOURCC_NV12             : return Format_NV12;
        case FOURCC_NV21             : return Format_NV21;
        case FOURCC_NV11             : return Format_NV11;
        case FOURCC_P208             : return Format_P208;
        case FOURCC_IMC1             : return Format_IMC1;
        case FOURCC_IMC2             : return Format_IMC2;
        case FOURCC_IMC3             : return Format_IMC3;
        case FOURCC_IMC4             : return Format_IMC4;
        case FOURCC_I420             : return Format_I420;
        case FOURCC_IYUV             : return Format_IYUV;
        case FOURCC_YV12             : return Format_YV12;
        case FOURCC_YVU9             : return Format_YVU9;
        case FOURCC_AI44             : return Format_AI44;
        case FOURCC_IA44             : return Format_IA44;
        case FOURCC_400P             : return Format_400P;
        case FOURCC_411P             : return Format_411P;
        case FOURCC_422H             : return Format_422H;
        case FOURCC_422V             : return Format_422V;
        case FOURCC_444P             : return Format_444P;
        case FOURCC_RGBP             : return Format_RGBP;
        case FOURCC_BGRP             : return Format_BGRP;
        case FOURCC_P010             : return Format_P010;
        default                      : return Format_Invalid;
    }
}

//!
//! \brief    Convet from MOS OS to OS format
//! \details  need to refactor this function after libva ddi interface done, or after OS format defined
//!          for Linux/Android
//! \param    MOS_FORMAT format
//!           [in] MOS OS surface format
//! \return   MOS_OS_FORMAT
//!           Return OS format
//!
MOS_OS_FORMAT Mos_Specific_FmtMosToOs(
    MOS_FORMAT     format)
{
    switch (format)
    {
    case Format_A8R8G8B8     : return (MOS_OS_FORMAT)DDI_FORMAT_A8R8G8B8;
    case Format_X8R8G8B8     : return (MOS_OS_FORMAT)DDI_FORMAT_X8R8G8B8;
    case Format_A8B8G8R8     : return (MOS_OS_FORMAT)DDI_FORMAT_A8B8G8R8;
    case Format_R32U         : return (MOS_OS_FORMAT)DDI_FORMAT_R32F;
    case Format_R32F         : return (MOS_OS_FORMAT)DDI_FORMAT_R32F;
    case Format_R5G6B5       : return (MOS_OS_FORMAT)DDI_FORMAT_R5G6B5;
    case Format_YUY2         : return (MOS_OS_FORMAT)DDI_FORMAT_YUY2;
    case Format_P8           : return (MOS_OS_FORMAT)DDI_FORMAT_P8;
    case Format_A8P8         : return (MOS_OS_FORMAT)DDI_FORMAT_A8P8;
    case Format_A8           : return (MOS_OS_FORMAT)DDI_FORMAT_A8;
    case Format_L8           : return (MOS_OS_FORMAT)DDI_FORMAT_L8;
    case Format_L16          : return (MOS_OS_FORMAT)DDI_FORMAT_L16;
    case Format_A4L4         : return (MOS_OS_FORMAT)DDI_FORMAT_A4L4;
    case Format_A8L8         : return (MOS_OS_FORMAT)DDI_FORMAT_A8L8;
    case Format_V8U8         : return (MOS_OS_FORMAT)DDI_FORMAT_V8U8;
    case Format_YVYU         : return (MOS_OS_FORMAT)FOURCC_YVYU;
    case Format_UYVY         : return (MOS_OS_FORMAT)FOURCC_UYVY;
    case Format_VYUY         : return (MOS_OS_FORMAT)FOURCC_VYUY;
    case Format_AYUV         : return (MOS_OS_FORMAT)FOURCC_AYUV;
    case Format_NV12         : return (MOS_OS_FORMAT)FOURCC_NV12;
    case Format_NV21         : return (MOS_OS_FORMAT)FOURCC_NV21;
    case Format_NV11         : return (MOS_OS_FORMAT)FOURCC_NV11;
    case Format_P208         : return (MOS_OS_FORMAT)FOURCC_P208;
    case Format_IMC1         : return (MOS_OS_FORMAT)FOURCC_IMC1;
    case Format_IMC2         : return (MOS_OS_FORMAT)FOURCC_IMC2;
    case Format_IMC3         : return (MOS_OS_FORMAT)FOURCC_IMC3;
    case Format_IMC4         : return (MOS_OS_FORMAT)FOURCC_IMC4;
    case Format_I420         : return (MOS_OS_FORMAT)FOURCC_I420;
    case Format_IYUV         : return (MOS_OS_FORMAT)FOURCC_IYUV;
    case Format_YV12         : return (MOS_OS_FORMAT)FOURCC_YV12;
    case Format_YVU9         : return (MOS_OS_FORMAT)FOURCC_YVU9;
    case Format_AI44         : return (MOS_OS_FORMAT)FOURCC_AI44;
    case Format_IA44         : return (MOS_OS_FORMAT)FOURCC_IA44;
    case Format_400P         : return (MOS_OS_FORMAT)FOURCC_400P;
    case Format_411P         : return (MOS_OS_FORMAT)FOURCC_411P;
    case Format_422H         : return (MOS_OS_FORMAT)FOURCC_422H;
    case Format_422V         : return (MOS_OS_FORMAT)FOURCC_422V;
    case Format_444P         : return (MOS_OS_FORMAT)FOURCC_444P;
    case Format_RGBP         : return (MOS_OS_FORMAT)FOURCC_RGBP;
    case Format_BGRP         : return (MOS_OS_FORMAT)FOURCC_BGRP;
    case Format_STMM         : return (MOS_OS_FORMAT)DDI_FORMAT_P8;
    case Format_P010         : return (MOS_OS_FORMAT)FOURCC_P010;
    case Format_A16B16G16R16 : return (MOS_OS_FORMAT)DDI_FORMAT_A16B16G16R16;
    default                  : return (MOS_OS_FORMAT)DDI_FORMAT_UNKNOWN;
    }
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
//! \brief    Synchronize GPU context
//! \details  Synchronize GPU context
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    MOS_GPU_CONTEXT busyGPUCtx
//!           [in] Busy context id being requested to sync with
//! \param    MOS_GPU_CONTEXT requestorGPUCtx
//!           [in] Requestor context id
//! \return   void
//!
void Mos_Specific_SyncGpuContext(
    PMOS_INTERFACE        pOsInterface,
    MOS_GPU_CONTEXT       busyGPUCtx,
    MOS_GPU_CONTEXT       requestorGPUCtx)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(busyGPUCtx);
    MOS_UNUSED(requestorGPUCtx);
}

//!
//! \brief    Synchronize 3d GPU context
//! \details  Synchronize 3d GPU context
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    PMOS_SYNC_PARAMS pSyncParams
//!           [in] Sync parameters
//! \return   void
//!
void Mos_Specific_SyncWith3DContext(
    PMOS_INTERFACE        pOsInterface,
    PMOS_SYNC_PARAMS      pSyncParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pSyncParams);
}

//!
//! \brief    Sleep
//! \details  Sleeps for the specified amount
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] pointer to Os interface structure
//! \param    uint32_t dwWaitMs
//!           [in] sleep time in ms
//! \return   void
//!
void Mos_Specific_SleepMs(
    PMOS_INTERFACE      pOsInterface,
    uint32_t            dwWaitMs)
{
    MOS_UNUSED(pOsInterface);
    usleep(dwWaitMs);
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
    PMOS_RESOURCE          pOsResource)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsResource);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        auto resource = gpuContext->GetStatusBufferResource();
        MOS_OS_CHK_NULL_RETURN(resource);

        MOS_OS_CHK_STATUS_RETURN(resource->ConvertToMosResource(pOsResource));
        return MOS_STATUS_SUCCESS;
    }

    PMOS_CONTEXT pOsContext;

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);

    pOsContext = pOsInterface->pOsContext;

    MOS_ZeroMemory(pOsResource, sizeof(*pOsResource));

    *pOsResource = *(pOsContext->pGPUStatusBuffer);
    return MOS_STATUS_SUCCESS;
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

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    uint32_t offset = 0;

    // A gobal status buffer for all GPU contexts is no longer used when modulized GPU context enabled,
    // replace with separate buffer corresponding to each GPU context and the offset will be 0
    if (!pOsInterface->modularizedGpuCtxEnabled || Mos_Solo_IsEnabled())
    {
        offset = sizeof(MOS_GPU_STATUS_DATA) * GpuContext;
    }

    return offset;
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

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        if (pOsInterface == nullptr || pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid input parameters!");
            return 0;
        }

        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

        auto handle = osCxtSpecific->GetGpuContextHandle(mosGpuCtx);

        auto gpuContext = Linux_GetGpuContext(pOsInterface, handle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return gpuContext->GetGpuStatusTag();
    }

    PMOS_CONTEXT pOsContext;

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //------------------------------------

    pOsContext = pOsInterface->pOsContext;

    return pOsContext->pfnGetGpuCtxBufferTag(pOsContext, mosGpuCtx);
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        if (pOsInterface == nullptr || pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid input parameters!");
            return;
        }

        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);

        auto handle = osCxtSpecific->GetGpuContextHandle(mosGpuCtx);

        auto gpuContext = Linux_GetGpuContext(pOsInterface, handle);

        if (gpuContext)
        {
            gpuContext->IncrementGpuStatusTag();
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Cannot get valid Gpu context!");
        }
    }

    PMOS_CONTEXT pOsContext;

    //------------------------------------
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //------------------------------------

    pOsContext = pOsInterface->pOsContext;

    pOsContext->pfnIncGpuCtxBufferTag(pOsContext, mosGpuCtx);
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
    MOS_GPU_CONTEXT           GpuContext)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(GpuContext);
    return 0;
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
//! \brief    Signal OS Engine
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_EngineSignal(
    PMOS_INTERFACE         pOsInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pParams);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Wait on sync
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_EngineWait(
    PMOS_INTERFACE         pOsInterface,
    PMOS_SYNC_PARAMS       pParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pParams);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Signal on resource
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_SYNC_PARAMS pParams
//!           [in] Pointer to sync params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_ResourceSignal(
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
//! \brief    Create Sync Resource
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_CreateSyncResource(
    PMOS_INTERFACE        pOsInterface,
    PMOS_RESOURCE         pOsResource)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pOsResource);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Destroy Sync resource
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS
//!
MOS_STATUS Mos_Specific_DestroySyncResource(
    PMOS_INTERFACE      pOsInterface,
    PMOS_RESOURCE       pOsResource)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pOsResource);
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Initializes the tags for Hybrid Decode multi-threading sync
//! \details  Initializes the tags for Hybrid Decode multi-threading sync
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS resource
//! \param    uint8_t ucRenderTargetIndex
//!           [in] Index of render target in pSemHandleList for pOsResource
//! \param    PMOS_SEMAPHORE *pCurFrmSem
//!           [in] List of semaphore handles used for current frame sync
//! \param    PMOS_SEMAPHORE *pRefFrmSem
//!           [in] List of semaphore handles used for reference frame sync
//! \param    PMOS_MUTEX *pFrmMutex
//!           [in] List of mutex handles of frame lock mutex
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_InitializeMultiThreadingSyncTags(
    PMOS_INTERFACE          pOsInterface,
    PMOS_RESOURCE           pOsResource,
    uint8_t                 ucRenderTargetIndex,
    PMOS_SEMAPHORE          *pCurFrmSem,
    PMOS_SEMAPHORE          *pRefFrmSem,
    PMOS_MUTEX              *pFrmMutex)
{
    MOS_STATUS       eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pCurFrmSem);
    MOS_OS_CHK_NULL(pRefFrmSem);

    if(*pOsResource->ppReferenceFrameSemaphore == nullptr)
    {
        *pOsResource->ppReferenceFrameSemaphore = MOS_CreateSemaphore(1, 1);
    }

    if(*pOsResource->ppCurrentFrameSemaphore == nullptr)
    {
        *pOsResource->ppCurrentFrameSemaphore = MOS_CreateSemaphore(1, 1);
    }

    if((*pOsResource->ppReferenceFrameSemaphore != nullptr) && (*pOsResource->ppCurrentFrameSemaphore != nullptr))
    {
        pCurFrmSem[ucRenderTargetIndex] = *pOsResource->ppCurrentFrameSemaphore;
        pRefFrmSem[ucRenderTargetIndex] = *pOsResource->ppReferenceFrameSemaphore;
        pFrmMutex[ucRenderTargetIndex]  = nullptr;
    }

finish:
    return eStatus;
}

//!
//! \brief    Wait current frame's semaphore for Hybrid Decode multi-threading sync
//! \details  Wait current frame's semaphore for Hybrid Decode multi-threading sync
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS resource
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_MultiThreadingWaitCurrentFrame(
    PMOS_RESOURCE   pOsResource)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsResource);

    MOS_OS_CHK_STATUS(MOS_WaitSemaphore(*pOsResource->ppCurrentFrameSemaphore, INFINITE));

finish:
    return eStatus;
}

//!
//! \brief    Post current frame's semaphore for Hybrid Decode multi-threading sync
//! \details  Post current frame's semaphore for Hybrid Decode multi-threading sync
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS resource
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_MultiThreadingPostCurrentFrame(
    PMOS_RESOURCE   pOsResource)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsResource);

    MOS_OS_CHK_STATUS(MOS_PostSemaphore(*pOsResource->ppCurrentFrameSemaphore, 1));

finish:
    return eStatus;
}

//!
//! \brief    Sets hybrid decoder running flag
//! \details  Sets hybrid decoder running flag to indicate there is a hybrid decoder
//!           is running
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    int32_t bFlag
//!           [in] hybrid decoder (HEVC or VP9) running flag
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_SetHybridDecoderRunningFlag(
    PMOS_INTERFACE          pOsInterface,
    int32_t                 bFlag)
{
    PMOS_CONTEXT          pOsContext;
    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);

    pOsContext = pOsInterface->pOsContext;
    pOsContext->bHybridDecoderRunningFlag = bFlag;

finish:
    return eStatus;
}

//!
//! \brief    Gets hybrid decoder running flag
//! \details  Gets hybrid decoder running flag to decide if hybrid decoder is being used
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    int32_t bFlag
//!           [in] hybrid decoder (HEVC or VP9) running flag
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_GetHybridDecoderRunningFlag(
    PMOS_INTERFACE          pOsInterface,
    int32_t                 *pFlag)
{
    PMOS_CONTEXT          pOsContext;
    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;

    *pFlag = false;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);

    pOsContext = pOsInterface->pOsContext;
    *pFlag = pOsContext->bHybridDecoderRunningFlag;

finish:
    return eStatus;
}

//!
//! \brief    Multi thread resource sync method
//! \details  Dummy implementation on Linux for compatibility.
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS resource
//! \return   MOS_STATUS
//!
MOS_STATUS Mos_Specific_MultiThreadResourceSync(
    PMOS_INTERFACE          pOsInterface,
    PMOS_RESOURCE           pOsResource)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pOsResource);
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

//!
//! \brief    Determines if the resource should be CPU cacheable during allocation
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_ALLOC_GFXRES_PARAMS pAllocParams
//!           [in] allocation parameters
//! \return   int32_t
//!           Return if resource should be CPU cacheable
//!
int32_t Mos_Specific_SetCpuCacheability(
    PMOS_INTERFACE              pOsInterface,
    PMOS_ALLOC_GFXRES_PARAMS    pAllocParams)
{
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(pAllocParams);
    return false;
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
    MOS_UNUSED(pOsResource);
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

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->VerifyPatchListSize(dwRequestedSize));
    }

    MOS_STATUS              eStatus;
    PMOS_OS_CONTEXT         pOsContext;
    PMOS_OS_GPU_CONTEXT     pOsGpuContext;

    MOS_OS_CHK_NULL(pOsInterface);

    eStatus       = MOS_STATUS_SUCCESS;

    pOsContext    = pOsInterface->pOsContext;
    MOS_OS_CHK_NULL(pOsContext);

    pOsGpuContext = &(pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal]);
    if (dwRequestedSize > pOsGpuContext->uiMaxPatchLocationsize)
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

finish:
    return eStatus;
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
    PMOS_OS_CONTEXT         pOsContext;
    PMOS_OS_GPU_CONTEXT     pOsGpuContext;
    MOS_UNUSED(pCmdBuffer);
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pCmdBuffer);

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled())
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return gpuContext->ResetCommandBuffer();
    }

    pOsContext    = pOsInterface->pOsContext;
    pOsGpuContext = &pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];

    pOsGpuContext->bCBFlushed = true;

finish:
    return MOS_STATUS_SUCCESS;
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
#ifdef ANDROID
    intel_ufo_bo_datatype_t datatype;
#else
    PGMM_RESOURCE_INFO      pGmmResourceInfo;
#endif
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_UNUSED(pOsInterface);
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pOsResource);

#ifdef ANDROID
    MOS_OS_CHK_NULL(pOsResource->bo);

    datatype.value = 0;
    mos_bo_get_datatype(pOsResource->bo, &datatype.value);
    *pResMmcMode = (MOS_MEMCOMP_STATE)datatype.compression_mode;
#else
    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    switch (GmmResGetMmcMode(pGmmResourceInfo, 0))
    {
        case GMM_MMC_HORIZONTAL:
            *pResMmcMode = MOS_MEMCOMP_HORIZONTAL;
            break;
        case GMM_MMC_VERTICAL:
            *pResMmcMode = MOS_MEMCOMP_VERTICAL;
            break;
        case GMM_MMC_DISABLED:
        default:
            *pResMmcMode = MOS_MEMCOMP_DISABLED;
            break;
    }
#endif

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
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
    MOS_MEMCOMP_STATE   ResMmcMode)
{
#ifdef ANDROID
    intel_ufo_bo_datatype_t datatype;
#else
    PGMM_RESOURCE_INFO      pGmmResourceInfo;
    GMM_RESOURCE_MMC_INFO   GmmResMmcMode = GMM_MMC_DISABLED;
#endif
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_UNUSED(pOsInterface);
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL(pOsResource);

#ifdef ANDROID
    MOS_OS_CHK_NULL(pOsResource->bo);

    datatype.value = 0;
    mos_bo_get_datatype(pOsResource->bo, &datatype.value);

    datatype.compression_mode = (uint32_t)ResMmcMode;
    mos_bo_set_datatype(pOsResource->bo, datatype.value);
#else
    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    switch (ResMmcMode)
    {
        case MOS_MEMCOMP_HORIZONTAL:
            GmmResMmcMode = GMM_MMC_HORIZONTAL;
            break;
        case MOS_MEMCOMP_VERTICAL:
            GmmResMmcMode = GMM_MMC_VERTICAL;
            break;
        case MOS_MEMCOMP_DISABLED:
        default:
            GmmResMmcMode = GMM_MMC_DISABLED;
            break;
    }

    GmmResSetMmcMode(pGmmResourceInfo, GmmResMmcMode, 0);
#endif

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
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
#ifdef ANDROID
    intel_ufo_bo_datatype_t datatype;
#else
    PGMM_RESOURCE_INFO      pGmmResourceInfo;
    uint32_t                uiArrayIndex = 0;
#endif
    MOS_STATUS              eStatus;
    MOS_UNUSED(pOsInterface);
    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CHK_NULL(pOsResource);

#ifdef ANDROID
    MOS_OS_CHK_NULL(pOsResource->bo);

    datatype.value = 0;
    mos_bo_get_datatype(pOsResource->bo, &datatype.value);

    datatype.compression_hint = (uint32_t) (bHintOn ? INTEL_UFO_BUFFER_HINT_MMC_COMPRESSED : INTEL_UFO_BUFFER_HINT_MMC_UNCOMPRESSED);
    mos_bo_set_datatype(pOsResource->bo, datatype.value);
#else
    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    GmmResSetMmcHint(pGmmResourceInfo, bHintOn ? GMM_MMC_HINT_ON : GMM_MMC_HINT_OFF, uiArrayIndex);
#endif

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

#ifdef ANDROID
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
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(bSetVideoNode);
    MOS_OS_ASSERT(pVideoNodeOrdinal);

    // return VDBox #1 for now.
    *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO;

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
    MOS_UNUSED(pOsInterface);
    MOS_UNUSED(VideoNodeOrdinal);
    return MOS_STATUS_SUCCESS;
}

#else

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
    PMOS_OS_CONTEXT         pOsContext;
    PVDBOX_WORKLOAD         pVDBoxWorkLoad = nullptr;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pVideoNodeOrdinal);

    pOsContext = pOsInterface->pOsContext;

    if (false == pOsContext->bKMDHasVCS2)
    {
        *pVideoNodeOrdinal = MOS_GPU_NODE_VIDEO;
        goto finish;
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

    LockSemaphore(pOsContext->semid);

    pVDBoxWorkLoad = (PVDBOX_WORKLOAD)pOsContext->pShm;

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

finish:
    return eStatus;
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
    PMOS_OS_CONTEXT         pOsContext;
    PVDBOX_WORKLOAD         pVDBoxWorkLoad;

    MOS_OS_FUNCTION_ENTER;

    pOsContext    = pOsInterface->pOsContext;

    // not do workload balancing in UMD just return;
    if (pOsContext->bKMDHasVCS2 == false)
    {
        return MOS_STATUS_SUCCESS;
    }

    LockSemaphore(pOsContext->semid);

    pVDBoxWorkLoad = (PVDBOX_WORKLOAD)pOsContext->pShm;

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
#endif

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

#ifndef ANDROID
    if (pOsInterface->pOsContext->bPerCmdBufferBalancing)
    {
        int32_t ret;

        // Query KMD for VDBox load counters;
        drm_i915_ring_load_info vdbox_load[] = {
            {.ring_id = I915_EXEC_BSD | I915_EXEC_BSD_RING1, .load_cnt = 0},
            {.ring_id = I915_EXEC_BSD | I915_EXEC_BSD_RING2, .load_cnt = 0},
        };

        drm_i915_ring_load_query vdbox_load_query = {
            .query_size = sizeof(vdbox_load) / sizeof(vdbox_load[0]),
            .load_info  = &vdbox_load[0],
        };

        ret = drmIoctl(pOsInterface->pOsContext->fd, DRM_IOCTL_I915_LOAD_BALANCING_HINT,
            &vdbox_load_query);

        if (ret) {
            MOS_OS_ASSERTMESSAGE("Failed to query KMD for balancing hint:"
                " error %d (falling back to ctx assigment)", ret);
        } else {
            int32_t cnt1 = vdbox_load[0].load_cnt;
            int32_t cnt2 = vdbox_load[1].load_cnt;

            // Assign task to VDBox with smaller load counter.
            // Give small priority to VDBox 2;
            if (0 == cnt2)
            {
                idx = MOS_VDBOX_NODE_2;
            } else if (0 == cnt1)
            {
                idx = MOS_VDBOX_NODE_1;
            } else if (cnt1 < cnt2)
            {
                idx = MOS_VDBOX_NODE_1;
            } else {
                idx = MOS_VDBOX_NODE_2;
            }

            // Save assigned VDBox number inside cmdbuf
            pCmdBuffer->iVdboxNodeIndex = idx;
        }
    }
#endif //#ifndef ANDROID

finish:
    return idx;
}

//!
//! \brief    Get the memory object
//! \details  Get the memory object for cache policy
//! \param    MOS_HW_RESOURCE_DEF MosUsage
//!           [in] HW resource
//! \return   MEMORY_OBJECT_CONTROL_STATE
//!           Return the memory object
//!
MEMORY_OBJECT_CONTROL_STATE Mos_Specific_CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF         MosUsage)
{
    return Mos_CachePolicyGetMemoryObject(MosUsage);
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
    char  *error;
    MOS_UNUSED(pOsInterface);
    //---------------------------------
    MOS_OS_ASSERT(pFileName);
    //---------------------------------

    *ppvModule = dlopen(pFileName, RTLD_LAZY);
    if (!(*ppvModule))
    {
#ifdef ANDROID
        error = (char *)dlerror();
        ALOGD("%s", error);
#endif
    }

    return ((*ppvModule) ? MOS_STATUS_SUCCESS : MOS_STATUS_LOAD_LIBRARY_FAILED);
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
    int32_t bStatus;

    //---------------------------------
    MOS_OS_ASSERT(hInstance);
    //---------------------------------

    bStatus = dlclose(hInstance);
    return (bStatus != 0) ? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}

//!
//! \brief
//! \details
//! \param    char  *pData
//! \return   void
//!
void  *Mos_Specific_GetProcAddress(
    void        *hInstance,
    char        *pModuleName)
{
    //---------------------------------
    MOS_OS_ASSERT(hInstance);
    MOS_OS_ASSERT(pModuleName);
    //---------------------------------

    return dlsym(hInstance, pModuleName);
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
    uint32_t dwResetCount;
    uint32_t dwActiveBatch;
    uint32_t dwPendingBatch;
    int32_t bResult;
    int32_t ret;

    bResult = false;
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Mos_Specific_IsGPUHung: pOsInterface == NULL");
        goto finish;
    }

    dwResetCount = dwActiveBatch = dwPendingBatch = 0;

    ret = mos_get_reset_stats(pOsInterface->pOsContext->intel_context, &dwResetCount,
                                &dwActiveBatch, &dwPendingBatch);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        goto finish;
    }

    if (dwResetCount != pOsInterface->dwGPUResetCount   ||
        dwActiveBatch != pOsInterface->dwGPUActiveBatch ||
        dwPendingBatch != pOsInterface->dwGPUPendingBatch)
    {
        pOsInterface->dwGPUResetCount   = dwResetCount;
        pOsInterface->dwGPUActiveBatch  = dwActiveBatch;
        pOsInterface->dwGPUPendingBatch = dwPendingBatch;
        bResult = true;
    }

finish:
    return bResult;
}

uint64_t Mos_Specific_GetAuxTableBaseAddr(
    PMOS_INTERFACE              pOsInterface)
{
    return 0;
}

//!
//! \brief  Set slice count to shared memory and KMD
//! \param  [in] pOsInterface
//!         Pointer to OS interface
//! \param  [in,out] pSliceCount
//!         Pointer to the slice count. Input the slice count for current
//!         context, output the ruling slice count shared by all contexts.
//!
void Mos_Specific_SetSliceCount(
        PMOS_INTERFACE              pOsInterface,
        uint32_t *pSliceCount)
{
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pSliceCount);

    if (pOsInterface->osContextPtr)
    {
        pOsInterface->osContextPtr->SetSliceCount(pSliceCount);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("OS context is nullptr.");
    }
}

//!
//! \brief
//! \details
//! \param    const char  *pFileName
//! \return   HINSTANCE
//!
void Mos_Specific_LogData(
    char       *pData)
{
#ifdef Android
    ALOGD(pData);
#else
    MOS_UNUSED(pData);
#endif

    return;
}

void Mos_Specific_NotifyStreamIndexSharing(
    PMOS_INTERFACE              pOsInterface)
{
    MOS_UNUSED(pOsInterface);
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
    PMOS_OS_CONTEXT                 pOsContext;
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface;
    MOS_STATUS                      eStatus;
    MediaFeatureTable              *pSkuTable;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    int32_t                         iDeviceId;
    uint32_t                        dwResetCount;
    int32_t                         ret;
    bool                            modularizedGpuCtxEnabled;

    MOS_OS_FUNCTION_ENTER;

    eStatus                 = MOS_STATUS_SUCCESS;
    pOsContext              = nullptr;
    pOsUserFeatureInterface = (PMOS_USER_FEATURE_INTERFACE)&pOsInterface->UserFeatureInterface;

    MOS_OS_NORMALMESSAGE("mm:Mos_Specific_InitInterface called.");

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsDriverContext);

    pOsInterface->modularizedGpuCtxEnabled    = true;

    // Create Linux OS Context
    pOsContext = (PMOS_OS_CONTEXT)MOS_AllocAndZeroMemory(sizeof(MOS_OS_CONTEXT));
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Unable to allocate memory.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pOsContext->intel_context = nullptr;
    if (pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled())
    {
        OsContext*  osContextPtr = OsContext::GetOsContextObject();
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
            if( MOS_STATUS_SUCCESS != eStatus )
            {
                MOS_OS_ASSERTMESSAGE("Unable to initialize MODS context.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }
        }

        OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
        pOsContext->intel_context             = pOsContextSpecific->GetDrmContext();
        pOsContext->pGmmClientContext         = nullptr;
    }
    else
    {
        pOsContext->pGmmClientContext = GmmCreateClientContext((GMM_CLIENT)GMM_LIBVA_LINUX);
    }

    // Initialize
    modularizedGpuCtxEnabled = pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled();
    eStatus = Linux_InitContext(pOsContext, pOsDriverContext, pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled(), modularizedGpuCtxEnabled);
    if( MOS_STATUS_SUCCESS != eStatus )
    {
        MOS_OS_ASSERTMESSAGE("Unable to initialize context.");
        goto finish;
    }

    iDeviceId                                 = pOsDriverContext->iDeviceId;
    pOsContext->bFreeContext                  = true;
    pOsInterface->pOsContext                  = pOsContext;
    pOsInterface->bUsesPatchList              = true;
    pOsInterface->bUsesGfxAddress             = false;
    pOsInterface->bNoParsingAssistanceInKmd   = true;
    pOsInterface->bUsesCmdBufHeaderInResize   = false;
    pOsInterface->bUsesCmdBufHeader           = false;
    pOsInterface->dwNumNalUnitBytesIncluded   = MOS_NAL_UNIT_LENGTH - MOS_NAL_UNIT_STARTCODE_LENGTH;

    pOsInterface->bInlineCodecStatusUpdate    = true;

    //Added by Ben for video memory allocation
    pOsInterface->pOsContext->bufmgr = pOsDriverContext->bufmgr;
    mos_bufmgr_gem_enable_reuse(pOsDriverContext->bufmgr);

    // Initialize OS interface functions
    pOsInterface->pfnSetGpuContext                          = Mos_Specific_SetGpuContext;
    pOsInterface->pfnGetGpuContext                          = Mos_Specific_GetGpuContext;
    pOsInterface->pfnSetEncodePakContext                    = Mos_Specific_SetEncodePakContext;
    pOsInterface->pfnSetEncodeEncContext                    = Mos_Specific_SetEncodeEncContext;

    pOsInterface->pfnGetPlatform                            = Mos_Specific_GetPlatform;
    pOsInterface->pfnDestroy                                = Mos_Specific_Destroy;
    pOsInterface->pfnGetSkuTable                            = Mos_Specific_GetSkuTable;
    pOsInterface->pfnGetWaTable                             = Mos_Specific_GetWaTable;
    pOsInterface->pfnGetGtSystemInfo                        = Mos_Specific_GetGtSystemInfo;
    pOsInterface->pfnResetOsStates                          = Mos_Specific_ResetOsStates;
    pOsInterface->pfnAllocateResource                       = Mos_Specific_AllocateResource;
    pOsInterface->pfnGetResourceInfo                        = Mos_Specific_GetResourceInfo;
    pOsInterface->pfnFreeResource                           = Mos_Specific_FreeResource;
    pOsInterface->pfnFreeResourceWithFlag                   = Mos_Specific_FreeResourceWithFlag;
    pOsInterface->pfnLockSyncRequest                        = Mos_Specific_LockSyncRequest;
    pOsInterface->pfnLockResource                           = Mos_Specific_LockResource;
    pOsInterface->pfnUnlockResource                         = Mos_Specific_UnlockResource;
    pOsInterface->pfnDecompResource                         = Mos_Specific_DecompResource;
    pOsInterface->pfnRegisterResource                       = Mos_Specific_RegisterResource;
    pOsInterface->pfnResetResourceAllocationIndex           = Mos_Specific_ResetResourceAllocationIndex;
    pOsInterface->pfnGetResourceAllocationIndex             = Mos_Specific_GetResourceAllocationIndex;
    pOsInterface->pfnGetResourceGfxAddress                  = Mos_Specific_GetResourceGfxAddress;
    pOsInterface->pfnGetCommandBuffer                       = Mos_Specific_GetCommandBuffer;
    pOsInterface->pfnResetCommandBuffer                     = Mos_Specific_ResetCommandBuffer;
    pOsInterface->pfnReturnCommandBuffer                    = Mos_Specific_ReturnCommandBuffer;
    pOsInterface->pfnSubmitCommandBuffer                    = Mos_Specific_SubmitCommandBuffer;
    pOsInterface->pfnWaitAndReleaseCmdBuffer                = Mos_Specific_WaitAndReleaseCmdBuffer;
    pOsInterface->pfnVerifyCommandBufferSize                = Mos_Specific_VerifyCommandBufferSize;
    pOsInterface->pfnResizeCommandBufferAndPatchList        = Mos_Specific_ResizeCommandBufferAndPatchList;
    pOsInterface->pfnFmt_OsToMos                            = Mos_Specific_FmtOsToMos;
    pOsInterface->pfnFmt_MosToOs                            = Mos_Specific_FmtMosToOs;
    pOsInterface->pfnFmt_MosToGmm                           = Mos_Specific_ConvertMosFmtToGmmFmt;
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

    pOsInterface->pfnSleepMs                                = Mos_Specific_SleepMs;

    pOsInterface->pfnLoadLibrary                            = Mos_Specific_LoadLibrary;
    pOsInterface->pfnFreeLibrary                            = Mos_Specific_FreeLibrary;
    pOsInterface->pfnGetProcAddress                         = Mos_Specific_GetProcAddress;
    pOsInterface->pfnLogData                                = Mos_Specific_LogData;

    //GPU context and synchronization functions
    pOsInterface->pfnCreateGpuContext                       = Mos_Specific_CreateGpuContext;
    pOsInterface->pfnDestroyGpuContext                      = Mos_Specific_DestroyGpuContext;
    pOsInterface->pfnIsGpuContextValid                      = Mos_Specific_IsGpuContextValid;
    pOsInterface->pfnSyncOnResource                         = Mos_Specific_SyncOnResource;
    pOsInterface->pfnSyncGpuContext                         = Mos_Specific_SyncGpuContext;
    pOsInterface->pfnSyncWith3DContext                      = Mos_Specific_SyncWith3DContext;
    pOsInterface->pfnGetGpuStatusBufferResource             = Mos_Specific_GetGpuStatusBufferResource;
    pOsInterface->pfnGetGpuStatusTagOffset                  = Mos_Specific_GetGpuStatusTagOffset;
    pOsInterface->pfnGetGpuStatusTag                        = Mos_Specific_GetGpuStatusTag;
    pOsInterface->pfnIncrementGpuStatusTag                  = Mos_Specific_IncrementGpuStatusTag;
    pOsInterface->pfnGetGpuStatusSyncTag                    = Mos_Specific_GetGpuStatusSyncTag;
    pOsInterface->pfnSetResourceSyncTag                     = Mos_Specific_SetResourceSyncTag;
    pOsInterface->pfnPerformOverlaySync                     = Mos_Specific_PerformOverlaySync;
    pOsInterface->pfnEngineSignal                           = Mos_Specific_EngineSignal;
    pOsInterface->pfnEngineWait                             = Mos_Specific_EngineWait;
    pOsInterface->pfnResourceSignal                         = Mos_Specific_ResourceSignal;
    pOsInterface->pfnResourceWait                           = Mos_Specific_ResourceWait;
    pOsInterface->pfnCreateSyncResource                     = Mos_Specific_CreateSyncResource;
    pOsInterface->pfnDestroySyncResource                    = Mos_Specific_DestroySyncResource;
    pOsInterface->pfnInitializeMultiThreadingSyncTags       = Mos_Specific_InitializeMultiThreadingSyncTags;
    pOsInterface->pfnMultiThreadingWaitCurrentFrame         = Mos_Specific_MultiThreadingWaitCurrentFrame;
    pOsInterface->pfnMultiThreadingPostCurrentFrame         = Mos_Specific_MultiThreadingPostCurrentFrame;
    pOsInterface->pfnSetHybridDecoderRunningFlag            = Mos_Specific_SetHybridDecoderRunningFlag;
    pOsInterface->pfnGetHybridDecoderRunningFlag            = Mos_Specific_GetHybridDecoderRunningFlag;
    pOsInterface->pfnMultiThreadResourceSync                = Mos_Specific_MultiThreadResourceSync;

    pOsInterface->pfnCachePolicyGetMemoryObject             = Mos_Specific_CachePolicyGetMemoryObject;
    pOsInterface->pfnVerifyPatchListSize                    = Mos_Specific_VerifyPatchListSize;
    pOsInterface->pfnGetMemoryCompressionMode               = Mos_Specific_GetMemoryCompressionMode;
    pOsInterface->pfnSetMemoryCompressionMode               = Mos_Specific_SetMemoryCompressionMode;
    pOsInterface->pfnSetMemoryCompressionHint               = Mos_Specific_SetMemoryCompressionHint;
    pOsInterface->pfnCreateVideoNodeAssociation             = Mos_Specific_CreateVideoNodeAssociation;
    pOsInterface->pfnDestroyVideoNodeAssociation            = Mos_Specific_DestroyVideoNodeAssociation;
    pOsInterface->pfnGetVdboxNodeId                         = Mos_Specific_GetVdboxNodeId;

    pOsInterface->pfnGetNullHWRenderFlags                   = Mos_Specific_GetNullHWRenderFlags;
    pOsInterface->pfnSetCmdBufferDebugInfo                  = Mos_Specific_SetCmdBufferDebugInfo;
    pOsInterface->pfnGetCmdBufferDebugInfo                  = Mos_Specific_GetCmdBufferDebugInfo;

    pOsInterface->pfnRegisterBBCompleteNotifyEvent          = Mos_Specific_RegisterBBCompleteNotifyEvent;
    pOsInterface->pfnWaitForBBCompleteNotifyEvent           = Mos_Specific_WaitForBBCompleteNotifyEvent;
    pOsInterface->pfnCachePolicyGetMemoryObject             = Mos_Specific_CachePolicyGetMemoryObject;
    pOsInterface->pfnSetCpuCacheability                     = Mos_Specific_SetCpuCacheability;
    pOsInterface->pfnSkipResourceSync                       = Mos_Specific_SkipResourceSync;
    pOsInterface->pfnIsGPUHung                              = Mos_Specific_IsGPUHung;
    pOsInterface->pfnGetAuxTableBaseAddr                    = Mos_Specific_GetAuxTableBaseAddr;
    pOsInterface->pfnSetSliceCount                          = Mos_Specific_SetSliceCount;
    pOsInterface->pfnGetResourceIndex                       = Mos_Specific_GetResourceIndex;
    pOsInterface->pfnSetSliceCount                          = Mos_Specific_SetSliceCount;
    pOsInterface->pfnIsSetMarkerEnabled                     = Mos_Specific_IsSetMarkerEnabled;
    pOsInterface->pfnGetMarkerResource                      = Mos_Specific_GetMarkerResource;
    pOsInterface->pfnNotifyStreamIndexSharing               = Mos_Specific_NotifyStreamIndexSharing;

    pOsUserFeatureInterface->bIsNotificationSupported   = false;
    pOsUserFeatureInterface->pOsInterface               = pOsInterface;
    pOsUserFeatureInterface->pfnReadValue               = MOS_UserFeature_ReadValue;
    pOsUserFeatureInterface->pfnEnableNotification      = MOS_UserFeature_EnableNotification;
    pOsUserFeatureInterface->pfnDisableNotification     = MOS_UserFeature_DisableNotification;
    pOsUserFeatureInterface->pfnParsePath               = MOS_UserFeature_ParsePath;

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

    // disable it on Linux
    pOsInterface->bMediaReset         = false;
    pOsInterface->umdMediaResetEnable = false;

    // initialize MOS_CP interface
    pOsInterface->osCpInterface = MOS_New(MosCpInterface, pOsInterface);
    if (pOsInterface->osCpInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("fail to create osCpInterface.");
        return MOS_STATUS_UNKNOWN;
    }
    pOsInterface->pOsExt            = nullptr;

    // Check SKU table to detect if simulation environment (HAS) is enabled
    pSkuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    MOS_OS_CHK_NULL(pSkuTable);
    pOsInterface->bSimIsActive = MEDIA_IS_SKU(pSkuTable, FtrSimulationMode);

#if (_DEBUG || _RELEASE_INTERNAL)
    // read the "Force VDBOX" user feature key
    // 0: not force
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX_ID,
        &UserFeatureData);
    pOsInterface->eForceVdbox = (MOS_FORCE_VDBOX)UserFeatureData.u32Data;

    // Force TileYf/Ys
    // 0: Tile Y  1: Tile Yf   2 Tile Ys
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_FORCE_YFYS_ID,
        &UserFeatureData);
    pOsInterface->dwForceTileYfYs = (uint32_t)UserFeatureData.i32Data;

    // Null HW Driver
    // 0: Disable
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE_ID,
        &UserFeatureData));
    pOsInterface->NullHWAccelerationEnable.Value = UserFeatureData.u32Data;
#endif // (_DEBUG || _RELEASE_INTERNAL)

#if MOS_MEDIASOLO_SUPPORTED
    Mos_Solo_Initialize(pOsInterface);
#endif // MOS_MEDIASOLO_SUPPORTED

    // read the "Disable VDBox load balancing" user feature key
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VDBOX_BALANCING_ID,
        &UserFeatureData);
    pOsInterface->bEnableVdboxBalancing = (bool)UserFeatureData.u32Data;

    // read the "Disable KMD Watchdog" user feature key
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_KMD_WATCHDOG_ID,
        &UserFeatureData);
    pOsContext->bDisableKmdWatchdog = (UserFeatureData.i32Data) ? true : false;

    // read "Linux PerformanceTag Enable" user feature key
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE_ID,
        &UserFeatureData);
    pOsContext->uEnablePerfTag = UserFeatureData.i32Data;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    if( MOS_STATUS_SUCCESS != eStatus && nullptr != pOsContext )
    {
        MOS_FreeMemAndSetNull(pOsContext);
    }
    return eStatus;
}

//!
//! \brief    Check if OS resource is nullptr
//! \details  Check if OS resource is nullptr
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   int32_t
//!           Return true if nullptr, otherwise false
//!
int32_t Mos_ResourceIsNull(
    PMOS_RESOURCE   pOsResource)
{
    //---------------------
    MOS_OS_ASSERT(pOsResource);
    //---------------------

    return ((pOsResource->bo == nullptr)
#if (_DEBUG || _RELEASE_INTERNAL)
         && ((pOsResource->pData == nullptr) )
#endif // (_DEBUG || _RELEASE_INTERNAL)
    );

}

//!
//! \brief    OS reset resource
//! \details  Resets the OS resource
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   void
//!           Return NONE
//!
void Mos_ResetResource(
    PMOS_RESOURCE   pOsResource)
{
    int32_t i;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_ASSERT(pOsResource);

    MOS_ZeroMemory(pOsResource, sizeof(MOS_RESOURCE));
    pOsResource->Format  = Format_None;
    for (i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        pOsResource->iAllocationIndex[i] = MOS_INVALID_ALLOC_INDEX;
    }
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

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
void GpuCmdResInfoDump::Dump(const void *cmdResInfoPtr)
{
    using std::endl;

    auto pRes = static_cast<const MOS_RESOURCE *>(cmdResInfoPtr);

    m_outputFile << "Gpu Resource Pointer " << pRes << endl;
    m_outputFile << "iWidth                    : " << pRes->iWidth << endl;
    m_outputFile << "iHeight                   : " << pRes->iHeight << endl;
    m_outputFile << "iSize                     : " << pRes->iSize << endl;
    m_outputFile << "iPitch                    : " << pRes->iPitch << endl;
    m_outputFile << "iDepth                    : " << pRes->iDepth << endl;
    m_outputFile << "Format                    : " << (uint32_t)pRes->Format << endl;
    m_outputFile << "iCount                    : " << pRes->iCount << endl;
    m_outputFile << "iAllocationIndex          : ";
    for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        m_outputFile << pRes->iAllocationIndex[i] << " ";
    }
    m_outputFile << endl;
    m_outputFile << "dwGfxAddress              : " << pRes->dwGfxAddress << endl;
    m_outputFile << "pData                     : " << pRes->pData << endl;
    m_outputFile << "bufname                   : " << pRes->bufname << endl;
    m_outputFile << "isTiled                   : " << pRes->isTiled << endl;
    m_outputFile << "TileType                  : " << (uint32_t)pRes->TileType << endl;
    m_outputFile << "bMapped                   : " << pRes->bMapped << endl;
    m_outputFile << "bo                        : " << pRes->bo << endl;
    m_outputFile << "name                      : " << pRes->name << endl;
    m_outputFile << "pGmmResInfo               : " << pRes->pGmmResInfo << endl;
    m_outputFile << "MmapOperation             : " << (uint32_t)pRes->MmapOperation << endl;
    m_outputFile << "user_provided_va          : " << pRes->user_provided_va << endl;
    m_outputFile << "pGfxResource              : " << pRes->pGfxResource << endl;
    m_outputFile << "bConvertedFromDDIResource : " << pRes->bConvertedFromDDIResource << endl;
    m_outputFile << endl;
}
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
