/*
* Copyright (c) 2009-2023, Intel Corporation
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
#include <dlfcn.h>
#include <stdlib.h>

#include "mos_os.h"
#include "mos_os_cp_interface_specific.h"
#include "mos_util_debug.h"
#include "mos_resource_defs.h"
#include "hwinfo_linux.h"
#include "mos_graphicsresource.h"
#include "mos_context_specific.h"
#include "mos_gpucontext_specific.h"
#include "mos_gpucontextmgr.h"

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

extern int32_t CreateCmDevice(MOS_CONTEXT *mosContext,
                              CMRT_UMD::CmDevice* &device,
                              uint32_t devCreateOption,
                              uint8_t  priority);

extern int32_t DestroyCmDevice(CMRT_UMD::CmDevice* &device);

extern MOS_STATUS InitCmOsDDIInterface(PCM_HAL_STATE cmState);

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

bool SetupApoDdiSwitch(int32_t fd, MediaUserSettingSharedPtr userSettingPtr)
{
    if (fd < 0)
    {
        return false;
    }

    //Read user feature to determine if apg mos is enabled.
    uint32_t    userfeatureValue = 0;
    MOS_STATUS estatus          = MosUtilities::MosReadApoDdiEnabledUserFeature(userfeatureValue, nullptr, userSettingPtr);

    return (userfeatureValue != 0);
}

bool SetupApoMosSwitch(int32_t fd, MediaUserSettingSharedPtr userSettingPtr)
{
    if (fd < 0)
    {
        return false;
    }

    //Read user feature to determine if apg mos is enabled.
    uint32_t    userfeatureValue = 0;
    MOS_STATUS  estatus          = MosUtilities::MosReadApoMosEnabledUserFeature(userfeatureValue, nullptr, userSettingPtr);

    if(estatus == MOS_STATUS_SUCCESS)
    {
        return (userfeatureValue != 0);
    }
    PRODUCT_FAMILY eProductFamily = IGFX_UNKNOWN;
    HWInfo_GetGfxProductFamily(fd, eProductFamily);

    if (eProductFamily >= IGFX_TIGERLAKE_LP)
    {
        return true;
    }

    return false;
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
    struct mos_drm_bo_alloc alloc;

    if ( pOsContext == nullptr ||
         pCmdBuffer == nullptr)
    {
        bResult = false;
        MOS_OS_ASSERTMESSAGE("Linux_GetCommandBuffer:pOsContext == nullptr || pCmdBuffer == NULL");
        goto finish;
    }

    // Allocate the command buffer from GEM
    alloc.name = "MOS CmdBuf";
    alloc.size = iSize;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
    cmd_bo = mos_bo_alloc(pOsContext->bufmgr, &alloc);     // Align to page boundary
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
    pCmdBuffer->iVeboxNodeIndex = MOS_VEBOX_NODE_INVALID;
    pCmdBuffer->is1stLvlBB = true;
    MOS_ZeroMemory(pCmdBuffer->pCmdBase, cmd_bo->size);
    pCmdBuffer->iSubmissionType = SUBMISSION_TYPE_SINGLE_PIPE;
    MOS_ZeroMemory(&pCmdBuffer->Attributes, sizeof(pCmdBuffer->Attributes));
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
        MOS_OS_ASSERTMESSAGE("Invalid input parameter pOsContext or pCmdBuffer.");
        goto finish;
    }

    if (GpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        goto finish;
    }

    pOsGpuContext = &pOsContext->OsGpuContext[GpuContext];
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Os gpucontext is null.");
        goto finish;
    }

    pOsGpuContext->pCB->iOffset    = pCmdBuffer->iOffset ;
    pOsGpuContext->pCB->iRemaining = pCmdBuffer->iRemaining;
    pOsGpuContext->pCB->pCmdPtr    = pCmdBuffer->pCmdPtr;
    pOsGpuContext->pCB->iVdboxNodeIndex = pCmdBuffer->iVdboxNodeIndex;
    pOsGpuContext->pCB->iVeboxNodeIndex = pCmdBuffer->iVeboxNodeIndex;
    pOsGpuContext->pCB->is1stLvlBB = pCmdBuffer->is1stLvlBB;

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
        MOS_OS_ASSERTMESSAGE("Invalid input parameter pOsContext.")
        goto finish;
    }

    if (GpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        goto finish;
    }

    pOsGpuContext = &pOsContext->OsGpuContext[GpuContext];
    if (pOsGpuContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid OsGpuContext");
        goto finish;
    }

    // Refresh command buffer usage
    pOsContext->pfnRefresh(pOsContext);

    pOsGpuContext->uiCurrentNumPatchLocations = 0;

    // CB already active
    pCurrCB = pOsGpuContext->pCurrentCB;
    if (pCurrCB == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid current CB in OsGpuContext.");
        goto finish;
    }

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
    int32_t         i;
    MOS_STATUS      eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsContext);

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

    MOS_OS_CHK_NULL(pOsContext);

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
    int32_t         index = 0;
    MOS_STATUS      eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsContext);
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
    MOS_ZeroMemory(&buf, sizeof(buf));

    if (shmid < 0)
    {
        return MOS_STATUS_UNKNOWN;
    }

    if ((pShm != MOS_LINUX_SHM_INVALID) && (pShm != nullptr) && shmdt(pShm) < 0)
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
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pShmid);
    MOS_OS_CHK_NULL_RETURN(ppShm);

    struct shmid_ds buf;
    int32_t         shmid     = -1;
    key_t           key_value = (key_t)key;
    void *          shmptr    = nullptr;
    MOS_ZeroMemory(&buf, sizeof(buf));

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
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pSemid);
    int32_t         semid = -1;
    struct sembuf   sop;
    struct semid_ds buf;
    uint32_t        i         = 0;
    key_t           key_value = (key_t)key;
    int32_t         val       = 0;
    MOS_ZeroMemory(&sop, sizeof(sop));
    MOS_ZeroMemory(&buf, sizeof(buf));

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
    int32_t nwait = -1;

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
    MOS_ZeroMemory(&buf, sizeof(buf));

    if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
        return -1;
    }

    return buf.shm_nattch;
}

static MOS_STATUS LockSemaphore(int32_t semid)
{
    struct sembuf op[2] = {};
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

    MOS_OS_CHK_NULL(pOsContext);
    pOsContext->semid = MOS_LINUX_IPC_INVALID_ID;
    pOsContext->shmid = MOS_LINUX_IPC_INVALID_ID;
    pOsContext->pShm = MOS_LINUX_SHM_INVALID;

    struct semid_ds buf;
    MOS_ZeroMemory(&buf, sizeof(buf));

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

        MosUtilities::MosSleep(1);  //wait and retry
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
    struct mos_drm_bo_alloc alloc;

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
    alloc.name = "GPU Status Buffer";
    alloc.size = sizeof(MOS_GPU_STATUS_DATA)  * MOS_GPU_CONTEXT_MAX;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
    bo = mos_bo_alloc(pOsContext->bufmgr, &alloc);     // Align to page boundary
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
        MOS_OS_ASSERTMESSAGE("Invalid pOsContext.");
        return 0;
    }

    if (GpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
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
        MOS_OS_ASSERTMESSAGE("Invalid pOsContext.");
        return;
    }

    if (GpuContext == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return;
    }

    uiGPUStatusTag = pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag;

    pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag = uiGPUStatusTag % UINT_MAX + 1;
    if(pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag == 0)
    {
        pOsContext->OsGpuContext[GpuContext].uiGPUStatusTag = 1;
    }
}

GMM_CLIENT_CONTEXT* Linux_GetGmmClientContext(PMOS_CONTEXT pOsContext)
{
    if (pOsContext == nullptr)
    {
        return nullptr;
    }
    return pOsContext->pGmmClientContext;
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
        
        if (pOsInterface->apoMosEnabled)
        {
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

        auto gpuContext = Linux_GetGpuContext(pOsInterface, handle);

        if (gpuContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot get corresponding gpu context!");
            return 0;
        }

        auto resource = gpuContext->GetStatusBufferResource();

        if (resource == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active resource.");
            return 0;
        }

        MOS_RESOURCE gpuStatusResource;
        MOS_OS_CHK_STATUS_RETURN(resource->ConvertToMosResource(&gpuStatusResource));
        auto gpuStatusData = (MOS_GPU_STATUS_DATA *)gpuStatusResource.pData;
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
    PCOMMAND_BUFFER pCurrCB = nullptr;
    PCOMMAND_BUFFER pNextCB = nullptr;
    int32_t         iSize   = 0;
    int             i       = 0;
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsContext is null.");
        return;
    }

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

    if (pOsContext->contextOffsetList.size())
    {
         pOsContext->contextOffsetList.clear();
         pOsContext->contextOffsetList.shrink_to_fit();
    }

    if (!MODSEnabled && (pOsContext->intel_context))
    {
        if (pOsContext->intel_context->vm_id != INVALID_VM)
        {
            mos_vm_destroy(pOsContext->intel_context->bufmgr,pOsContext->intel_context->vm_id);
            pOsContext->intel_context->vm_id = INVALID_VM;
        }
        mos_context_destroy(pOsContext->intel_context);
    }

    MOS_Delete(pOsContext);
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
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsContext is null.");
        return 0;
    }

    uint32_t dmaBufID = 0;
    if (pOsContext->pPerfData != nullptr)
    {
        dmaBufID = pOsContext->pPerfData->dmaBufID;
    }
    return dmaBufID;
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
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsContext is null.");
        return;
    }

    if (pOsContext->pPerfData != nullptr)
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
    if (pOsContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("OsContext is null.");
        return;
    }

    if (pOsContext->pPerfData != nullptr)
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
    int32_t    iDeviceId = -1;
    MOS_STATUS eStatus;
    int32_t    i = -1;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    if (nullptr == pContext ||
        nullptr == pOsDriverContext ||
        nullptr == pOsDriverContext->bufmgr ||
        (nullptr == pOsDriverContext->m_gpuContextMgr && modularizedGpuCtxEnabled && !pOsDriverContext->m_apoMosEnabled) ||
        (nullptr == pOsDriverContext->m_cmdBufMgr && modularizedGpuCtxEnabled && !pOsDriverContext->m_apoMosEnabled) ||
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
    pContext->m_auxTableMgr   = pOsDriverContext->m_auxTableMgr;
    pContext->m_userSettingPtr = pOsDriverContext->m_userSettingPtr;

    mos_bufmgr_enable_reuse(pOsDriverContext->bufmgr);

    // DDI layer can pass over the DeviceID.
    iDeviceId = pOsDriverContext->iDeviceId;
    if (0 == iDeviceId)
    {
        //Such as CP, it calls InitMosInterface() dretly without creating MediaContext.
        iDeviceId = mos_bufmgr_get_devid(pOsDriverContext->bufmgr);
        pOsDriverContext->iDeviceId = iDeviceId;

        MOS_OS_CHK_STATUS_MESSAGE(
            HWInfo_GetGfxInfo(pOsDriverContext->fd, pOsDriverContext->bufmgr, &pContext->m_platform, &pContext->m_skuTable, &pContext->m_waTable, &pContext->m_gtSystemInfo, pOsDriverContext->m_userSettingPtr),
            "Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization");

        pOsDriverContext->m_skuTable     = pContext->m_skuTable;
        pOsDriverContext->m_waTable      = pContext->m_waTable;
        pOsDriverContext->m_gtSystemInfo = pContext->m_gtSystemInfo;
        pOsDriverContext->m_platform     = pContext->m_platform;
        MOS_OS_NORMALMESSAGE("DeviceID was created DeviceID = %d, platform product %d", iDeviceId, pContext->m_platform.eProductFamily);
    }
    else
    {
        // pOsDriverContext's parameters were passed by CmCreateDevice.
        // Get SkuTable/WaTable/systemInfo/platform from OSDriver directly.
        pContext->m_skuTable     = pOsDriverContext->m_skuTable;
        pContext->m_waTable      = pOsDriverContext->m_waTable;
        pContext->m_gtSystemInfo = pOsDriverContext->m_gtSystemInfo;
        pContext->m_platform     = pOsDriverContext->m_platform;
    }

    pContext->bUse64BitRelocs = true;
    pContext->bUseSwSwizzling = pContext->bSimIsActive || MEDIA_IS_SKU(&pContext->m_skuTable, FtrUseSwSwizzling);
    pContext->bTileYFlag      = MEDIA_IS_SKU(&pContext->m_skuTable, FtrTileY);
    // when MODS enabled, intel_context will be created by pOsContextSpecific, should not recreate it here, or will cause memory leak.
    if (!MODSEnabled)
    {
       pContext->intel_context = mos_context_create_ext(pOsDriverContext->bufmgr, 0, pOsDriverContext->m_protectedGEMContext);
       if (!Mos_Solo_IsEnabled(nullptr) && pContext->intel_context)
       {
           pContext->intel_context->vm_id = mos_vm_create(pOsDriverContext->bufmgr);
           if (pContext->intel_context->vm_id == INVALID_VM)
           {
               MOS_OS_ASSERTMESSAGE("Failed to create vm.\n");
               return MOS_STATUS_UNKNOWN;
           }
       }
       else //try legacy context create ioctl if DRM_IOCTL_I915_GEM_CONTEXT_CREATE_EXT is not supported
       {
           pContext->intel_context = mos_context_create(pOsDriverContext->bufmgr);
           if (pContext->intel_context)
           {
               pContext->intel_context->vm_id = INVALID_VM;
           }
       }

       if (pContext->intel_context == nullptr)
       {
            MOS_OS_ASSERTMESSAGE("Failed to create drm intel context");
            return MOS_STATUS_UNKNOWN;
       }
    }

    pContext->intel_context->pOsContext = pContext;

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
        pContext->bKMDHasVCS2 = mos_has_bsd2(pContext->bufmgr);
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
    pContext->pfnMediaMemoryCopy        = pOsDriverContext->pfnMediaMemoryCopy;
    pContext->pfnMediaMemoryCopy2D      = pOsDriverContext->pfnMediaMemoryCopy2D;

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
    pContext->GetGmmClientContext        = Linux_GetGmmClientContext;

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

        if (pOsInterface->apoMosEnabled)
        {
            if(MEDIA_IS_SKU(&pOsInterface->pOsContext->m_skuTable, FtrProtectedGEMContextPatch))
            {
                GpuContextSpecificNext* GpuCtxSpecific = MosInterface::GetGpuContext(pOsInterface->osStreamState, pOsInterface->CurrentGpuContextHandle);
                MOS_OS_CHK_STATUS_RETURN(GpuCtxSpecific->PatchGPUContextProtection(pOsInterface->osStreamState));
            }
            MOS_OS_CHK_STATUS_RETURN(MosInterface::SetGpuContext(
                pOsInterface->osStreamState,
                pOsContextSpecific->GetGpuContextHandle(GpuContext)));
        }
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

        if (osInterface->apoMosEnabled)
        {
            MOS_OS_CHK_STATUS_RETURN(
                MosInterface::SetGpuContext(osInterface->osStreamState,
                                            contextHandle));
        }
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

    if (Mos_Solo_IsEnabled(nullptr)) return MOS_STATUS_SUCCESS;

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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetGpuContext(pOsInterface->osStreamState, gpuContextHandle);
    }

    OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
    if(!pOsContextSpecific)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return nullptr;
    }

    GpuContextMgr *gpuContextMgr = pOsContextSpecific->GetGpuContextMgr();
    if (!gpuContextMgr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return nullptr;
    }

    GpuContext *gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);
    if (!gpuContext)
    {
        MOS_OS_ASSERTMESSAGE("Invalid nullptr");
        return nullptr;
    }

    return (void *)gpuContext;
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetGmmClientContext(pOsInterface->osStreamState);
    }

    if (pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
        if (pOsContextSpecific)
        {
            return pOsContextSpecific->GetGmmClientContext();
        }
    }
    else
    {
        if (pOsInterface->pOsContext)
        {
            return pOsInterface->pOsContext->GetGmmClientContext(pOsInterface->pOsContext);
        }
    }
    return nullptr;
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

    if (pOsInterface->apoMosEnabled)
    {
        // apo wrapper
        auto platform = MosInterface::GetPlatform(pOsInterface->osStreamState);
        if (platform)
        {
            *pPlatform = *platform;
        }
        return;
    }

    *pPlatform = pOsInterface->pOsContext->m_platform;

finish:
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

    if (pOsInterface->apoMosEnabled)
    {
        MOS_STATUS status = Mos_DestroyInterface(pOsInterface);
        if (status != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Mos Destroy Interface failed.");
        }
        return;
    }

    if (pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        OsContext* pOsContext = pOsInterface->osContextPtr;
        if (pOsContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Unable to get the active OS context.");
            return;
        }

        // APO MOS destory GPU contexts here instead of inside osContextPtr
        if (pOsInterface->apoMosEnabled)
        {
            OsContextSpecific* pOsContextSpecific = static_cast<OsContextSpecific*>(pOsContext);
            if (pOsInterface->osStreamState == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("osStreamState is null when destroy GpuContext");
                return;
            }
            if (pOsInterface->osStreamState->osDeviceContext == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("osDeviceContext is null when destroy GpuContext");
                return;
            }
            auto gpuContextMgr = pOsInterface->osStreamState->osDeviceContext->GetGpuContextMgr();
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

    if (pOsInterface &&
        pOsInterface->pOsContext &&
        pOsInterface->pOsContext->bFreeContext)
    {
        pOsInterface->pOsContext->m_skuTable.reset();
        pOsInterface->pOsContext->m_waTable.reset();
        Mos_Specific_ClearGpuContext(pOsInterface->pOsContext);
        bool modularizedGpuCtxEnabled = pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr);
        pOsInterface->pOsContext->pfnDestroy(pOsInterface->pOsContext, pOsInterface->modulizedMosEnabled, modularizedGpuCtxEnabled);
        pOsInterface->pOsContext = nullptr;
    }
    if (pOsInterface->pVEInterf)
    {
        if (pOsInterface->apoMosEnabled && pOsInterface->pVEInterf->veInterface)
        {
            pOsInterface->pVEInterf->veInterface->Destroy();
            MOS_Delete(pOsInterface->pVEInterf->veInterface);
        }
        MOS_FreeMemAndSetNull(pOsInterface->pVEInterf);
    }

    if (pOsInterface->apoMosEnabled)
    {
        auto status = MosInterface::DestroyOsStreamState(pOsInterface->osStreamState);
        if (status != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to destroy stream state.");
            return;
        }
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
    if (pOsInterface && pOsInterface->apoMosEnabled)
    {
        // apo wrapper
        return MosInterface::GetSkuTable(pOsInterface->osStreamState);
    }

    if (pOsInterface && pOsInterface->pOsContext)
    {
        return &pOsInterface->pOsContext->m_skuTable;
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
    if (pOsInterface && pOsInterface->apoMosEnabled)
    {
        // apo wrapper
        return MosInterface::GetWaTable(pOsInterface->osStreamState);
    }

    if (pOsInterface && pOsInterface->pOsContext)
    {
        return &pOsInterface->pOsContext->m_waTable;
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

    if (pOsInterface->apoMosEnabled)
    {
        // apo wrapper
        return MosInterface::GetGtSystemInfo(pOsInterface->osStreamState);
    }

    if( nullptr == pOsInterface->pOsContext)
    {
        MOS_OS_ASSERTMESSAGE("pOsContext is NULL.");
        return  nullptr;
    }

    return &pOsInterface->pOsContext->m_gtSystemInfo;
}

MOS_STATUS Mos_Specific_GetMediaEngineInfo(
    PMOS_INTERFACE pOsInterface, MEDIA_ENGINE_INFO &info)
{
    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid pointer!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (pOsInterface->apoMosEnabled)
    {
        // apo wrapper
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetMediaEngineInfo(pOsInterface->osStreamState, info));
        return MOS_STATUS_SUCCESS;
    }

    auto systemInfo = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(systemInfo);
    MosUtilities::MosZeroMemory(&info, sizeof(info));
    info.VDBoxInfo = systemInfo->VDBoxInfo;
    info.VEBoxInfo = systemInfo->VEBoxInfo;

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

    if (pOsInterface->apoMosEnabled)
    {
        auto status = MosInterface::ResetCommandBuffer(pOsInterface->osStreamState, 0);
        if (MOS_FAILED(status))
        {
            MOS_OS_ASSERTMESSAGE("ResetCommandBuffer failed.");
        }
        return;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        if (gpuContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("GPU Context pointer is nullptr!");
            return;
        }

        gpuContext->ResetGpuContextStatus();
        return;
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
    void *               caller = nullptr;
    const char *         bufname;
    void *               ptr;
    int32_t              iSize = 0;
    int32_t              iPitch = 0;
    MOS_STATUS           eStatus;
    MOS_LINUX_BO *       bo = nullptr;
    MOS_TILE_TYPE        tileformat;
    uint32_t             tileformat_linux = 0;
    int32_t              iHeight = 0;
    int32_t              iAlignedHeight = 0;
    GMM_RESCREATE_PARAMS GmmParams;
    GMM_RESOURCE_INFO *  pGmmResourceInfo = nullptr;
    GMM_RESOURCE_TYPE    resourceType = RESOURCE_INVALID;
    int                  mem_type = MOS_MEMPOOL_VIDEOMEMORY;

    MOS_OS_FUNCTION_ENTER;

    MOS_ZeroMemory(&GmmParams, sizeof(GmmParams));

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

    bufname                                = pParams->pBufName;
    pOsResource->bConvertedFromDDIResource = false;

    bool osContextValid = false;
    if (pOsInterface->osContextPtr != nullptr)
    {
        if (pOsInterface->osContextPtr->GetOsContextValid() == true)
        {
            osContextValid = true;
        }
    }

    if (pOsInterface->apoMosEnabled)
    {
        pParams->bBypassMODImpl = !((pOsInterface->modulizedMosEnabled) && (!Mos_Solo_IsEnabled(nullptr)) && (osContextValid == true));

        MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);

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

    if ((pOsInterface->modulizedMosEnabled) && (!Mos_Solo_IsEnabled(nullptr)) && (osContextValid == true))
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
        if (MosUtilities::m_mosMemAllocCounterGfx != nullptr)
        {
            GraphicsResource::SetMemAllocCounterGfx(*MosUtilities::m_mosMemAllocCounterGfx);
        }
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
        if (MosUtilities::m_mosMemAllocCounterGfx != nullptr)
        {
            *MosUtilities::m_mosMemAllocCounterGfx = GraphicsResource::GetMemAllocCounterGfx();
        }
        MOS_OS_CHK_NULL(pOsResource->pGmmResInfo);
        MOS_MEMNINJA_GFX_ALLOC_MESSAGE(pOsResource->pGmmResInfo, bufname, pOsInterface->Component,
            (uint32_t)pOsResource->pGmmResInfo->GetSizeSurface(), pParams->dwArraySize, functionName, filename, line);

        return eStatus;
    }

    caller              = nullptr;
    tileformat_linux    = TILING_NONE;
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
    GmmParams.Format                = MosInterface::MosFmtToGmmFmt(pParams->Format);

    MOS_OS_CHECK_CONDITION(GmmParams.Format == GMM_FORMAT_INVALID,
                         "Unsupported format",
                         MOS_STATUS_UNKNOWN);

    switch(tileformat)
    {
        case MOS_TILE_Y:
            tileformat_linux               = TILING_Y;
            if (pParams->bIsCompressible                                             && 
                MEDIA_IS_SKU(&pOsInterface->pOsContext->m_skuTable, FtrE2ECompression) &&
                MEDIA_IS_SKU(&pOsInterface->pOsContext->m_skuTable, FtrCompressibleSurfaceDefault))
            {
                GmmParams.Flags.Gpu.MMC = true;
                GmmParams.Flags.Info.MediaCompressed = 1;
                GmmParams.Flags.Gpu.CCS = 1;
                GmmParams.Flags.Gpu.RenderTarget = 1;
                GmmParams.Flags.Gpu.UnifiedAuxSurface = 1;

                if (pParams->CompressionMode == MOS_MMC_RC)
                {
                    GmmParams.Flags.Info.MediaCompressed = 0;
                    GmmParams.Flags.Info.RenderCompressed = 1;
                }
                else
                {
                    GmmParams.Flags.Info.MediaCompressed = 1;
                    GmmParams.Flags.Info.RenderCompressed = 0;
                }

                if(MEDIA_IS_SKU(&pOsInterface->pOsContext->m_skuTable, FtrFlatPhysCCS))
                {
                    GmmParams.Flags.Gpu.UnifiedAuxSurface = 0;
                }
            }
            break;
        case MOS_TILE_X:
            GmmParams.Flags.Info.TiledX    = true;
            tileformat_linux               = TILING_X;
            break;
        default:
            GmmParams.Flags.Info.Linear    = true;
            tileformat_linux               = TILING_NONE;
    }
    GmmParams.Flags.Info.LocalOnly = MEDIA_IS_SKU(&pOsInterface->pOsContext->m_skuTable, FtrLocalMemory);

    pOsResource->pGmmResInfo = pGmmResourceInfo = pOsInterface->pOsContext->pGmmClientContext->CreateResInfoObject(&GmmParams);

    MOS_OS_CHK_NULL(pGmmResourceInfo);

    switch (pGmmResourceInfo->GetTileType())
    {
        case GMM_TILED_X:
            tileformat = MOS_TILE_X;
            tileformat_linux               = TILING_X;
            break;
        case GMM_TILED_Y:
            tileformat = MOS_TILE_Y;
            tileformat_linux               = TILING_Y;
            break;
        case GMM_NOT_TILED:
            tileformat = MOS_TILE_LINEAR;
            tileformat_linux               = TILING_NONE;
            break;
        default:
            tileformat = MOS_TILE_Y;
            tileformat_linux               = TILING_Y;
            break;
    }

    if (pParams->TileType == MOS_TILE_Y)
    {
        pGmmResourceInfo->SetMmcMode((GMM_RESOURCE_MMC_INFO)pParams->CompressionMode, 0);
    }

    iPitch      = GFX_ULONG_CAST(pGmmResourceInfo->GetRenderPitch());
    iSize       = GFX_ULONG_CAST(pGmmResourceInfo->GetSizeSurface());
    iHeight     = pGmmResourceInfo->GetBaseHeight();

    MemoryPolicyParameter memPolicyPar;
    MOS_ZeroMemory(&memPolicyPar, sizeof(MemoryPolicyParameter));

    memPolicyPar.skuTable         = &pOsInterface->pOsContext->m_skuTable;
    memPolicyPar.waTable          = &pOsInterface->pOsContext->m_waTable;
    memPolicyPar.resInfo          = pGmmResourceInfo;
    memPolicyPar.resName          = pParams->pBufName;
    memPolicyPar.preferredMemType = pParams->dwMemType;

    mem_type = MemoryPolicyManager::UpdateMemoryPolicy(&memPolicyPar);

    // Only Linear and Y TILE supported
    if( tileformat_linux == TILING_NONE )
    {
        struct mos_drm_bo_alloc alloc;
        alloc.name = bufname;
        alloc.size = iSize;
        alloc.alignment = 4096;
        alloc.ext.mem_type = mem_type;
        bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, &alloc);
    }
    else
    {
        struct mos_drm_bo_alloc_tiled alloc_tiled;
        alloc_tiled.name = bufname;
        alloc_tiled.x = iPitch;
        alloc_tiled.y = iSize/iPitch;
        alloc_tiled.cpp = 1;
        alloc_tiled.ext.tiling_mode = tileformat_linux;
        alloc_tiled.ext.mem_type = mem_type;

        bo = mos_bo_alloc_tiled(pOsInterface->pOsContext->bufmgr, &alloc_tiled);
        iPitch = (int32_t)alloc_tiled.pitch;;
    }

    pOsResource->bMapped = false;
    if (bo)
    {
        pOsResource->Format          = pParams->Format;
        pOsResource->iWidth          = pParams->dwWidth;
        pOsResource->iHeight         = iHeight;
        pOsResource->iPitch          = iPitch;
        pOsResource->iCount          = 0;
        pOsResource->bufname         = bufname;
        pOsResource->bo              = bo;
        pOsResource->TileType        = tileformat;
        pOsResource->TileModeGMM     = (MOS_TILE_MODE_GMM)pGmmResourceInfo->GetTileModeSurfaceState();
        pOsResource->bGMMTileEnabled = true;
        pOsResource->pData           = (uint8_t *)bo->virt;  //It is useful for batch buffer to fill commands
        if (pParams->ResUsageType == MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC ||
            pParams->ResUsageType >= MOS_HW_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS)
        {
            pOsResource->memObjCtrlState = MosInterface::GetCachePolicyMemoryObject(pOsInterface->pOsContext->pGmmClientContext, MOS_MP_RESOURCE_USAGE_DEFAULT);
            pOsResource->mocsMosResUsageType = MOS_MP_RESOURCE_USAGE_DEFAULT;
        }
        else
        {
            pOsResource->memObjCtrlState = MosInterface::GetCachePolicyMemoryObject(pOsInterface->pOsContext->pGmmClientContext, pParams->ResUsageType);
            pOsResource->mocsMosResUsageType = pParams->ResUsageType;
        }

        MOS_OS_VERBOSEMESSAGE("Alloc %7d bytes (%d x %d resource).",iSize, pParams->dwWidth, iHeight);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Fail to Alloc %7d bytes (%d x %d resource).",iSize, pParams->dwWidth, pParams->dwHeight);
        eStatus = MOS_STATUS_NO_SPACE;
    }

    MosUtilities::MosAtomicIncrement(MosUtilities::m_mosMemAllocCounterGfx);
    MOS_MEMNINJA_GFX_ALLOC_MESSAGE(pOsResource->pGmmResInfo, bufname, pOsInterface->Component,
        (uint32_t)pOsResource->pGmmResInfo->GetSizeSurface(), pParams->dwArraySize, functionName, filename, line);

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
    GMM_RESOURCE_INFO * pGmmResourceInfo = nullptr;
    GMM_DISPLAY_FRAME   gmmChannel = GMM_DISPLAY_FRAME_MAX;
    GMM_REQ_OFFSET_INFO reqInfo[3] = {};
    GMM_RESOURCE_FLAG   GmmFlags = {};
    MOS_STATUS          eStatus;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pResDetails);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetResourceInfo(pOsInterface->osStreamState, pOsResource, *pResDetails);
    }

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    GmmFlags = pGmmResourceInfo->GetResFlags();

    pResDetails->dwWidth         = GFX_ULONG_CAST(pGmmResourceInfo->GetBaseWidth());
    pResDetails->dwHeight        = pGmmResourceInfo->GetBaseHeight();
    pResDetails->dwPitch         = GFX_ULONG_CAST(pGmmResourceInfo->GetRenderPitch());
    pResDetails->dwSize          = GFX_ULONG_CAST(pGmmResourceInfo->GetSizeSurface());
    pResDetails->dwDepth         = MOS_MAX(1, pGmmResourceInfo->GetBaseDepth());
    pResDetails->dwLockPitch     = GFX_ULONG_CAST(pGmmResourceInfo->GetRenderPitch());
    if (GFX_GET_CURRENT_RENDERCORE(pOsInterface->pfnGetGmmClientContext(pOsInterface)->GetPlatformInfo().Platform) < IGFX_GEN8_CORE)
    {
        pResDetails->bArraySpacing = pGmmResourceInfo->IsArraySpacingSingleLod();
    }
    if (GFX_GET_CURRENT_RENDERCORE(pOsInterface->pfnGetGmmClientContext(pOsInterface)->GetPlatformInfo().Platform) >= IGFX_GEN9_CORE)
    {
        pResDetails->dwQPitch = pGmmResourceInfo->GetQPitch();
    }

    pResDetails->bCompressible   = GmmFlags.Gpu.MMC ?
        (pGmmResourceInfo->GetMmcHint(0) == GMM_MMC_HINT_ON) : false;
    pResDetails->bIsCompressed   = pGmmResourceInfo->IsMediaMemoryCompressed(0);
    pResDetails->CompressionMode = (MOS_RESOURCE_MMC_MODE)pGmmResourceInfo->GetMmcMode(0);

    if (0 == pResDetails->dwPitch)
    {
        MOS_OS_ASSERTMESSAGE("Pitch from GmmResource is 0, unexpected.");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    // check resource's tile type
    pResDetails->TileModeGMM       = (MOS_TILE_MODE_GMM)pGmmResourceInfo->GetTileModeSurfaceState();
    pResDetails->bGMMTileEnabled   = true;
    switch (pGmmResourceInfo->GetTileType())
    {
    case GMM_TILED_Y:
          if (GmmFlags.Info.TiledYf)
          {
              pResDetails->TileType = MOS_TILE_YF;
          }
          else if (GmmFlags.Info.TiledYs)
          {
              pResDetails->TileType = MOS_TILE_YS;
          }
          else
          {
              pResDetails->TileType = MOS_TILE_Y;
          }
          break;
    case GMM_TILED_X:
          pResDetails->TileType = MOS_TILE_X;
          break;
    case GMM_NOT_TILED:
          pResDetails->TileType = MOS_TILE_LINEAR;
          break;
    default:
          pResDetails->TileType = MOS_TILE_Y;
          break;
    }
    pResDetails->Format   = pOsResource->Format;

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

    if (pOsInterface->apoMosEnabled)
    {
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

    if ((pOsInterface->modulizedMosEnabled)
     && (!pOsResource->bConvertedFromDDIResource)
     && (osContextValid == true)
     && (!Mos_Solo_IsEnabled(nullptr))
     && (pOsResource->pGfxResource))
    {
        if (pOsInterface->osContextPtr == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("invalid osContextPtr.");
            return;
        }
        if (MosUtilities::m_mosMemAllocCounterGfx != nullptr)
        {
            GraphicsResource::SetMemAllocCounterGfx(*MosUtilities::m_mosMemAllocCounterGfx);
        }
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
        if (MosUtilities::m_mosMemAllocCounterGfx != nullptr)
        {
            *MosUtilities::m_mosMemAllocCounterGfx = GraphicsResource::GetMemAllocCounterGfx();
        }
        MOS_MEMNINJA_GFX_FREE_MESSAGE(pOsResource->pGmmResInfo, functionName, filename, line);
        MOS_ZeroMemory(pOsResource, sizeof(*pOsResource));
        return;
    }

    if (pOsResource && pOsResource->bo)
    {
        OsContextSpecific *osCtx = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
        if(osCtx == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("osCtx is nullptr!");
            return;
        }
        else
        {
            AuxTableMgr *auxTableMgr = osCtx->GetAuxTableMgr();

            // Unmap Resource from Aux Table
            if (auxTableMgr)
            {
                auxTableMgr->UnmapResource(pOsResource->pGmmResInfo, pOsResource->bo);
            }
        }

        mos_bo_unreference((MOS_LINUX_BO *)(pOsResource->bo));

        if ( pOsInterface->pOsContext != nullptr && pOsInterface->pOsContext->contextOffsetList.size()) 
        {
          MOS_CONTEXT *pOsCtx = pOsInterface->pOsContext;
          auto item_ctx = pOsCtx->contextOffsetList.begin();

          for (;item_ctx != pOsCtx->contextOffsetList.end();)
          {
             if (item_ctx->target_bo == pOsResource->bo)
             {
                item_ctx = pOsCtx->contextOffsetList.erase(item_ctx);
             }
             else
             {
                item_ctx++;
             }
          }
        }

        pOsResource->bo = nullptr;
        if (pOsResource->pGmmResInfo != nullptr && 
            pOsInterface->pOsContext != nullptr &&
            pOsInterface->pOsContext->pGmmClientContext != nullptr)
        {
            MosUtilities::MosAtomicDecrement(MosUtilities::m_mosMemAllocCounterGfx);
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::LockMosResource(pOsInterface->osStreamState, pOsResource, pLockFlags);
    }

    if ((pOsInterface->modulizedMosEnabled)
       && (!pOsResource->bConvertedFromDDIResource)
       && (osContextValid == true)
       && (!Mos_Solo_IsEnabled(nullptr))
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
    if (pOsResource && pOsResource->bo && pOsResource->pGmmResInfo)
    {
        MOS_LINUX_BO *bo = pOsResource->bo;
        GMM_RESOURCE_FLAG GmmFlags;

        MOS_ZeroMemory(&GmmFlags, sizeof(GmmFlags));
        GmmFlags = pOsResource->pGmmResInfo->GetResFlags();

        // Do decompression for a compressed surface before lock
        if (!pLockFlags->NoDecompress &&
            (((GmmFlags.Gpu.MMC || GmmFlags.Gpu.CCS) && GmmFlags.Gpu.UnifiedAuxSurface)||
             pOsResource->pGmmResInfo->IsMediaMemoryCompressed(0)))
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
                mos_bo_map_gtt(bo);
            }
            else
            {
                if (pOsResource->TileType != MOS_TILE_LINEAR && !pLockFlags->TiledAsTiled)
                {
                    if (pContext->bUseSwSwizzling)
                    {
                        mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY&pLockFlags->WriteOnly));
                        pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
                        if (pOsResource->pSystemShadow == nullptr)
                        {
                            pOsResource->pSystemShadow = (uint8_t *)MOS_AllocMemory(bo->size);
                            MOS_OS_CHECK_CONDITION((pOsResource->pSystemShadow == nullptr), "Failed to allocate shadow surface", nullptr);
                        }
                        if (pOsResource->pSystemShadow)
                        {
                            int32_t flags = pContext->bTileYFlag ? 0 : 1;
                            MOS_OS_CHECK_CONDITION((pOsResource->TileType != MOS_TILE_Y), "Unsupported tile type", nullptr);
                            MOS_OS_CHECK_CONDITION((bo->size <= 0 || pOsResource->iPitch <= 0), "Invalid BO size or pitch", nullptr);
                            Mos_SwizzleData((uint8_t*)bo->virt, pOsResource->pSystemShadow, 
                                    MOS_TILE_Y, MOS_TILE_LINEAR, bo->size / pOsResource->iPitch, pOsResource->iPitch, flags);
                        }
                    }
                    else
                    {
                        mos_bo_map_gtt(bo);
                        pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP_GTT;
                    }
                }
                else if (pLockFlags->Uncached)
                {
                    mos_bo_map_wc(bo);
                    pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP_WC;
                }
                else
                {
                    mos_bo_map(bo, (OSKM_LOCKFLAG_WRITEONLY&pLockFlags->WriteOnly));
                    pOsResource->MmapOperation = MOS_MMAP_OPERATION_MMAP;
                }
            }
            pOsResource->pData   = pOsResource->pSystemShadow ? pOsResource->pSystemShadow : (uint8_t*)bo->virt;
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::UnlockMosResource(pOsInterface->osStreamState, pOsResource);
    }

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
     && (!Mos_Solo_IsEnabled(nullptr))
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
               mos_bo_unmap_gtt(pOsResource->bo);
           }
           else
           {
               if (pOsResource->pSystemShadow)
               {
                   int32_t flags = pContext->bTileYFlag ? 0 : 1;
                   Mos_SwizzleData(pOsResource->pSystemShadow, (uint8_t*)pOsResource->bo->virt, 
                           MOS_TILE_LINEAR, MOS_TILE_Y, pOsResource->bo->size / pOsResource->iPitch, pOsResource->iPitch, flags);
                   MOS_FreeMemory(pOsResource->pSystemShadow);
                   pOsResource->pSystemShadow = nullptr;
               }

               switch(pOsResource->MmapOperation)
               {
                   case MOS_MMAP_OPERATION_MMAP_GTT:
                        mos_bo_unmap_gtt(pOsResource->bo);
                        break;
                   case MOS_MMAP_OPERATION_MMAP_WC:
                        mos_bo_unmap_wc(pOsResource->bo);
                        break;
                   case MOS_MMAP_OPERATION_MMAP:
                        mos_bo_unmap(pOsResource->bo);
                        break;
                   default:
                        MOS_OS_ASSERTMESSAGE("Invalid mmap operation type");
                        break;
               }
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::DecompResource(pOsInterface->osStreamState, pOsResource);
    }

    pContext = pOsInterface->pOsContext;

    if (pOsResource && pOsResource->bo && pOsResource->pGmmResInfo)
    {
        MOS_LINUX_BO *bo = pOsResource->bo;
        if (pOsResource->pGmmResInfo->IsMediaMemoryCompressed(0))
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
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CONTEXT          *pContext = nullptr;

    //---------------------------------------
    MOS_OS_CHK_NULL(osInterface);
    MOS_OS_CHK_NULL(inputOsResource);
    MOS_OS_CHK_NULL(outputOsResource);
    //---------------------------------------

    if (osInterface->apoMosEnabled)
    {
        return MosInterface::DoubleBufferCopyResource(osInterface->osStreamState, inputOsResource, outputOsResource, bOutputCompressed);
    }

    pContext = osInterface->pOsContext;

    if (inputOsResource && inputOsResource->bo && inputOsResource->pGmmResInfo &&
        outputOsResource && outputOsResource->bo && outputOsResource->pGmmResInfo)
    {
        // Double Buffer Copy can support any tile status surface with/without compression
        pContext->pfnMediaMemoryCopy(pContext, inputOsResource, outputOsResource, bOutputCompressed);
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
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
    uint32_t              bpp,
    bool                  bOutputCompressed)
{
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CONTEXT          *pContext = nullptr;

    //---------------------------------------
    MOS_OS_CHK_NULL(osInterface);
    MOS_OS_CHK_NULL(inputOsResource);
    MOS_OS_CHK_NULL(outputOsResource);
    //---------------------------------------
    
    if (osInterface->apoMosEnabled)
    {
        return MosInterface::MediaCopyResource2D(osInterface->osStreamState, inputOsResource, outputOsResource,
            copyWidth, copyHeight, bpp, bOutputCompressed);
    }

    pContext = osInterface->pOsContext;

    if (inputOsResource && inputOsResource->bo && inputOsResource->pGmmResInfo &&
        outputOsResource && outputOsResource->bo && outputOsResource->pGmmResInfo)
    {
        // Double Buffer Copy can support any tile status surface with/without compression
        pContext->pfnMediaMemoryCopy2D(pContext, inputOsResource, outputOsResource, copyWidth, copyHeight, 0, 0, bpp, bOutputCompressed);
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Copy mono picture
//! \details  This is the copy function dedicated to mono picture copy
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
MOS_STATUS Mos_Specific_MonoSurfaceCopy(
    PMOS_INTERFACE osInterface,
    PMOS_RESOURCE  inputOsResource,
    PMOS_RESOURCE  outputOsResource,
    uint32_t       copyWidth,
    uint32_t       copyHeight,
    uint32_t       copyInputOffset,
    uint32_t       copyOutputOffset,
    bool           bOutputCompressed)
{
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CONTEXT          *pContext = nullptr;

    //---------------------------------------
    MOS_OS_CHK_NULL_RETURN(osInterface);
    MOS_OS_CHK_NULL_RETURN(inputOsResource);
    MOS_OS_CHK_NULL_RETURN(outputOsResource);
    //---------------------------------------
    
    if (osInterface->apoMosEnabled)
    {
        return MosInterface::MonoSurfaceCopy(osInterface->osStreamState, inputOsResource, outputOsResource, copyWidth, copyHeight, copyInputOffset, copyOutputOffset, bOutputCompressed);
    }

    pContext = osInterface->pOsContext;

    if (inputOsResource && inputOsResource->bo && inputOsResource->pGmmResInfo &&
        outputOsResource && outputOsResource->bo && outputOsResource->pGmmResInfo)
    {
        // Double Buffer Copy can support any tile status surface with/without compression
        pContext->pfnMediaMemoryCopy2D(pContext, inputOsResource, outputOsResource, copyWidth, copyHeight, 0, 0, 16, bOutputCompressed);
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
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
    if (osInterface->apoMosEnabled)
    {
        void *apo_mos_context = nullptr;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetperStreamParameters(osInterface->osStreamState, &apo_mos_context));
        *mosContext = (PMOS_CONTEXT)apo_mos_context;
    }
    else
    {
        *mosContext = (PMOS_CONTEXT)osInterface->pOsContext;
    }

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
    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::SetPatchEntry(pOsInterface->osStreamState, pParams);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
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
    pPatchList[pOsGpuContext->uiCurrentNumPatchLocations].uiWriteOperation    = pParams->bWrite ? true : false;

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
    MOS_OS_CHK_NULL_RETURN(pOsResource);

    if (pOsInterface->apoMosEnabled)
    {
#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
        GpuCmdResInfoDumpNext::GetInstance(pOsInterface->pOsContext)->StoreCmdResPtr(pOsInterface, (const void *)pOsResource);
#endif  // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

        return MosInterface::RegisterResource(
            pOsInterface->osStreamState,
            pOsResource,
            bWrite);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

    #if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
        GpuCmdResInfoDump::GetInstance(pOsInterface->pOsContext)->StoreCmdResPtr(pOsInterface, (const void *)pOsResource);
    #endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

        return (gpuContext->RegisterResource(pOsResource, bWrite));
    }

    PMOS_OS_CONTEXT     pOsContext = nullptr;
    PMOS_RESOURCE       pResources = nullptr;
    uint32_t            uiAllocation = 0;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_GPU_CONTEXT  *pOsGpuContext = nullptr;
    MOS_UNUSED(bWritebSetResourceSyncTag);

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);

    pOsContext          = pOsInterface->pOsContext;

    if (pOsInterface->CurrentGpuContextOrdinal == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("CurrentGpuContextOrdinal exceed max.");
        return  MOS_STATUS_INVALID_PARAMETER;
    }
    pOsGpuContext       = &pOsContext->OsGpuContext[pOsInterface->CurrentGpuContextOrdinal];

    // Find previous registration
    pResources = pOsGpuContext->pResources;
    if( nullptr == pResources)
    {
        MOS_OS_ASSERTMESSAGE("pResource is NULL.");
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::VerifyCommandBufferSize(pOsInterface->osStreamState, 0, dwRequestedSize, dwFlags);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->VerifyCommandBufferSize(dwRequestedSize));
    }

    PMOS_OS_CONTEXT    pOsContext = nullptr;
    MOS_OS_GPU_CONTEXT OsGpuContext;

    //---------------------------------------
    MOS_UNUSED(dwFlags);
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);
    //---------------------------------------

    if (pOsInterface->CurrentGpuContextOrdinal == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("CurrentGpuContextOrdinal exceed max.");
        return  MOS_STATUS_INVALID_PARAMETER;
    }

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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pCmdBuffer);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetCommandBuffer(pOsInterface->osStreamState, pCmdBuffer, dwFlags);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->GetCommandBuffer(pCmdBuffer, dwFlags));
    }

    PMOS_OS_CONTEXT         pOsContext = nullptr;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
    PMOS_OS_GPU_CONTEXT     pOsGpuContext = nullptr;
    uint32_t                uiCommandBufferSize = 0;

    MOS_UNUSED(dwFlags);
    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pOsInterface->pOsContext);
    MOS_OS_CHK_NULL(pCmdBuffer);

    if (pOsInterface->CurrentGpuContextOrdinal == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::SetupIndirectState(pOsInterface->osStreamState, uSize);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        MOS_OS_CHK_STATUS_RETURN(gpuContext->SetIndirectStateSize(uSize));
    }

    PMOS_CONTEXT   pOsContext = nullptr;
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

    if (pOsInterface->apoMosEnabled)
    {
        uint32_t offset = 0;
        uint32_t size   = 0;
        auto eStatus = MosInterface::GetIndirectState(pOsInterface->osStreamState, nullptr, offset, size);
        *puOffset = offset;
        *puSize   = size;
        return eStatus;
    }

   if (pOsInterface->CurrentGpuContextHandle == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->GetIndirectState(puOffset, puSize));
    }

    PMOS_CONTEXT       pOsContext = nullptr;
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
    PMOS_OS_CONTEXT     pOsContext = nullptr;
    MOS_OS_GPU_CONTEXT  OsGpuContext = {};
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pIndirectState);

    if (pOsInterface->apoMosEnabled)
    {
        uint32_t offset = 0;
        uint32_t size   = 0;
        return MosInterface::GetIndirectState(pOsInterface->osStreamState, pIndirectState, offset, size);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return (gpuContext->GetIndirectStatePointer(pIndirectState));
    }

    eStatus    = MOS_STATUS_UNKNOWN;

    pOsContext = (pOsInterface) ? pOsInterface->pOsContext : nullptr;

    if (pOsContext)
    {
        if (pOsInterface->CurrentGpuContextHandle == MOS_GPU_CONTEXT_INVALID_HANDLE)
        {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
        }

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

    // Need to ensure 4K extra padding for HW requirement.
    if (pCmdBuffer && pCmdBuffer->iRemaining < EXTRA_PADDING_NEEDED)
    {
        MOS_OS_ASSERTMESSAGE("Need to ensure 4K extra padding for HW requirement.");
    }

    if (pOsInterface->apoMosEnabled)
    {
        auto status = MosInterface::ReturnCommandBuffer(pOsInterface->osStreamState, pCmdBuffer, dwFlags);
        if (MOS_FAILED(status))
        {
            MOS_OS_ASSERTMESSAGE("ReturnCommandBuffer failed.");
        }
        return;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
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

    PMOS_OS_CONTEXT         pOsContext = nullptr;
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
    uint32_t *buf = nullptr;
    MOS_LINUX_BO* bo = nullptr;

    j = 0;

    if(pOsInterface == nullptr || pOsInterface->pOsContext == nullptr)
    {
        return nullptr;
    }

    struct mos_drm_bo_alloc alloc;
    alloc.name = "NOP_CMD_BO";
    alloc.size = 4096;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
    bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, &alloc);
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
    uint32_t *buf = nullptr;
    MOS_LINUX_BO* bo = nullptr;

    j = 0;

    if(pOsInterface == nullptr || pOsInterface->pOsContext == nullptr)
    {
        return nullptr;
    }

    struct mos_drm_bo_alloc alloc;
    alloc.name = "BAD_CMD_BO";
    alloc.size = 4096;
    alloc.alignment = 4096;
    alloc.ext.mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
    bo = mos_bo_alloc(pOsInterface->pOsContext->bufmgr, &alloc);
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
    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pCmdBuffer);
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::SubmitCommandBuffer(pOsInterface->osStreamState, pCmdBuffer, bNullRendering);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

    #if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
        GpuCmdResInfoDump::GetInstance(pOsInterface->pOsContext)->Dump(pOsInterface);
        GpuCmdResInfoDump::GetInstance(pOsInterface->pOsContext)->ClearCmdResPtrs(pOsInterface);
    #endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

        return (gpuContext->SubmitCommandBuffer(pOsInterface, pCmdBuffer, bNullRendering));
    }
    MOS_OS_ASSERTMESSAGE("Not support this path");
    return MOS_STATUS_UNIMPLEMENTED;
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
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pCmdBuffer);
    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
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
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pResource);
    
    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetResourceGfxAddress(pOsInterface->osStreamState, pResource);
    }

    if (!mos_bo_is_softpin(pResource->bo))
    {
        mos_bo_set_softpin(pResource->bo);
    }
    return pResource->bo->offset64;
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::ResizeCommandBufferAndPatchList(pOsInterface->osStreamState, 0, dwRequestedCommandBufferSize, dwRequestedPatchListSize, dwFlags);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
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

            if (mos_hweight8(pOsInterface->pOsContext->intel_context, sseu.subslice_mask) > createOption->packed.SubSliceCount)
            {
                sseu.subslice_mask = mos_switch_off_n_bits(pOsInterface->pOsContext->intel_context, sseu.subslice_mask,
                        mos_hweight8(pOsInterface->pOsContext->intel_context, sseu.subslice_mask)-createOption->packed.SubSliceCount);
            }

            if (mos_set_context_param_sseu(pOsInterface->pOsContext->intel_context, sseu))
            {
                MOS_OS_ASSERTMESSAGE("Failed to set sseu configuration.");
                return MOS_STATUS_UNKNOWN;
            };
        }

        createOption->gpuNode = GpuNode;
        if (pOsInterface->apoMosEnabled)
        {
            MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);
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

        if (pOsContextSpecific->GetGpuContextHandle(mosGpuCxt) == MOS_GPU_CONTEXT_INVALID_HANDLE)
        {
            auto gpuContext = gpuContextMgr->CreateGpuContext(GpuNode, cmdBufMgr, mosGpuCxt);
            MOS_OS_CHK_NULL_RETURN(gpuContext);

            auto gpuContextSpecific  = static_cast<GpuContextSpecific *>(gpuContext);
            MOS_OS_CHK_NULL_RETURN(gpuContextSpecific);

            MOS_OS_CHK_STATUS_RETURN(gpuContextSpecific->Init(gpuContextMgr->GetOsContext(), pOsInterface, GpuNode, createOption));

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

        if (osInterface->apoMosEnabled)
        {
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
        else
        {
            auto os_context_specific
                    = static_cast<OsContextSpecific*>(osInterface->osContextPtr);
            if (nullptr == os_context_specific)
            {
                return MOS_GPU_CONTEXT_INVALID_HANDLE;
            }
            auto gpu_context_mgr = os_context_specific->GetGpuContextMgr();
            if (nullptr == gpu_context_mgr)
            {
                return MOS_GPU_CONTEXT_INVALID_HANDLE;
            }
            auto cmd_buffer_mgr = os_context_specific->GetCmdBufMgr();
            if (nullptr == cmd_buffer_mgr)
            {
                return MOS_GPU_CONTEXT_INVALID_HANDLE;
            }

            GpuContext *gpu_context
                    = gpu_context_mgr->CreateGpuContext(MOS_GPU_NODE_COMPUTE,
                                                        cmd_buffer_mgr,
                                                        contextName);
            auto *context_specific = static_cast<GpuContextSpecific*>(gpu_context);
            if (nullptr == context_specific)
            {
                return MOS_GPU_CONTEXT_INVALID_HANDLE;
            }
            MOS_STATUS status
                    = context_specific->Init(gpu_context_mgr->GetOsContext(),
                                             osInterface, MOS_GPU_NODE_COMPUTE,
                                             createOption);
            if (MOS_STATUS_SUCCESS != status)
            {
                return MOS_GPU_CONTEXT_INVALID_HANDLE;
            }
            return context_specific->GetGpuContextHandle();
        }
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

        if (pOsInterface->apoMosEnabled)
        {
            return MosInterface::DestroyGpuContext(pOsInterface->osStreamState, gpuContextHandle);
        }

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
//! \brief    Get GPU context Manager
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   GpuContextMgr
//!           GPU context Manager got from Os interface
//!
GpuContextMgr *Mos_Specific_GetGpuContextMgr(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_FUNCTION_ENTER;

    if (pOsInterface && pOsInterface->osContextPtr)
    {
        auto pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
        return pOsContextSpecific->GetGpuContextMgr();
    }
    else
    {
        return nullptr;
    }
}

//!
//! \brief    Destroy GPU context by handle
//! \details  Destroy GPU context by handle for legacy
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    GPU_CONTEXT_HANDLE gpuContextHandle
//!           [in] GPU Context handle
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_DestroyGpuContextByHandle(
    PMOS_INTERFACE        pOsInterface,
    GPU_CONTEXT_HANDLE    gpuContextHandle)
{
    if (Mos_Solo_IsEnabled(nullptr)) return MOS_STATUS_SUCCESS;
    if (pOsInterface && pOsInterface->apoMosEnabled)
    {
        return MosInterface::DestroyGpuContext(pOsInterface->osStreamState, gpuContextHandle);
    }

    auto gpuContextMgr = Mos_Specific_GetGpuContextMgr(pOsInterface);
                
    if (gpuContextMgr == nullptr)
    {
        //No need to destory GPU context when adv_gpucontext not enabled in Os context
        MOS_OS_NORMALMESSAGE("There is no Gpu context manager, adv gpu context not enabled, no need to destory GPU contexts.");
        return MOS_STATUS_NULL_POINTER;
    }
    else
    {
        auto gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);
        if (gpuContext != nullptr)
        {
            gpuContextMgr->DestroyGpuContext(gpuContext);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Not found gpu Context to destory, something must be wrong");
            return MOS_STATUS_NULL_POINTER;
        }
    }
    return MOS_STATUS_SUCCESS;
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

    if (osInterface->apoMosEnabled)
    {
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

    GpuContextMgr *gpuContextMgr = pOsContextSpecific->GetGpuContextMgr();
    MOS_OS_CHK_NULL_RETURN(gpuContextMgr);
    GpuContext *gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);
    MOS_OS_CHK_NULL_RETURN(gpuContext);

    MOS_GPU_CONTEXT gpuContextName = gpuContext->GetCpuContextID();
    if(gpuContextName != MOS_GPU_CONTEXT_CM_COMPUTE && gpuContextName != MOS_GPU_CONTEXT_COMPUTE)
    {
        MOS_OS_ASSERTMESSAGE("It is not compute gpu context and it will be destroyed in osInterface destroy.");
        return MOS_STATUS_SUCCESS;
    }

    gpuContextMgr->DestroyGpuContext(gpuContext);
    return MOS_STATUS_SUCCESS;
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

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface->osStreamState);
        pOsInterface->osStreamState->component = pOsInterface->Component;
        return MosInterface::SetPerfTag(pOsInterface->osStreamState, PerfTag);
    }

    PMOS_CONTEXT pOsContext =(PMOS_CONTEXT)pOsInterface->pOsContext;

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
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::ResetPerfBufferID(pOsInterface->osStreamState);
    }

    PMOS_CONTEXT pOsContext = (PMOS_CONTEXT)pOsInterface->pOsContext;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsContext);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsContext->pPerfData);
    pOsContext->pPerfData->bufferID = 0;
    return;
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::IncPerfBufferID(pOsInterface->osStreamState);
    }

    PMOS_CONTEXT pOsContext = (PMOS_CONTEXT)pOsInterface->pOsContext;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsContext);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsContext->pPerfData);

    pOsContext->pPerfData->bufferID++;
    return;
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::IncPerfFrameID(pOsInterface->osStreamState);
    }

    PMOS_CONTEXT pOsContext = (PMOS_CONTEXT)pOsInterface->pOsContext;

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsContext);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsContext->pPerfData);

    pOsContext->pPerfData->frameID++;

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

    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface invalid nullptr");
        return 0;
    }

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetPerfTag(pOsInterface->osStreamState);
    }

    PMOS_CONTEXT osContext = (PMOS_CONTEXT)pOsInterface->pOsContext;

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

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::SetPerfHybridKernelID(pOsInterface->osStreamState, KernelID);
    }

    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface->pOsContext);

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
    int32_t      bRet = false;
    PMOS_CONTEXT pOsContext;

    if(pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("pOsInterface invalid nullptr");
        return bRet;
    }

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::IsPerfTagSet(pOsInterface->osStreamState);
    }

    MOS_OS_ASSERT(pOsInterface->pOsContext);

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
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetGpuStatusBufferResource(pOsInterface->osStreamState, pOsResource, pOsInterface->osStreamState->currentGpuContextHandle);
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        auto resource = gpuContext->GetStatusBufferResource();
        MOS_OS_CHK_NULL_RETURN(resource);

        pOsResource = gpuContext->GetStatusBufferMosResource();
        MOS_OS_CHK_NULL_RETURN(pOsResource);
        MOS_ZeroMemory(pOsResource, sizeof(MOS_RESOURCE));

        MOS_OS_CHK_STATUS_RETURN(resource->ConvertToMosResource(pOsResource));

        return MOS_STATUS_SUCCESS;
    }

    PMOS_CONTEXT pOsContext;

    MOS_OS_ASSERT(pOsInterface);
    MOS_OS_ASSERT(pOsInterface->pOsContext);

    pOsContext = pOsInterface->pOsContext;

    pOsResource = pOsContext->pGPUStatusBuffer;
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

    if (pOsInterface->apoMosEnabled)
    {
        // always 0 in apo
        return 0;
    }
    // A gobal status buffer for all GPU contexts is no longer used when modulized GPU context enabled,
    // replace with separate buffer corresponding to each GPU context and the offset will be 0
    if (!pOsInterface->modularizedGpuCtxEnabled || Mos_Solo_IsEnabled(nullptr))
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

    MOS_OS_ASSERT(pOsInterface);

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

        if (pOsInterface->apoMosEnabled)
        {
            return MosInterface::GetGpuStatusTag(pOsInterface->osStreamState, handle);
        }

        auto gpuContext = Linux_GetGpuContext(pOsInterface, handle);
        MOS_OS_CHK_NULL_RETURN(gpuContext);

        return gpuContext->GetGpuStatusTag();
    }

    PMOS_CONTEXT pOsContext;

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

        if (pOsInterface->apoMosEnabled)
        {
            MosInterface::IncrementGpuStatusTag(pOsInterface->osStreamState, handle);
            return;
        }

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
        *pOsResource->ppReferenceFrameSemaphore = MosUtilities::MosCreateSemaphore(1, 1);
    }

    if(*pOsResource->ppCurrentFrameSemaphore == nullptr)
    {
        *pOsResource->ppCurrentFrameSemaphore = MosUtilities::MosCreateSemaphore(1, 1);
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

    MOS_OS_CHK_STATUS(MosUtilities::MosWaitSemaphore(*pOsResource->ppCurrentFrameSemaphore, INFINITE));

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

    MOS_OS_CHK_STATUS(MosUtilities::MosPostSemaphore(*pOsResource->ppCurrentFrameSemaphore, 1));

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

    MOS_OS_CHK_NULL(pFlag);
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
//!           [in] Time to wait (ms)
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
    usleep(1000 * uiTimeOut);

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
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //---------------------------------------
    MOS_OS_CHK_NULL(pOsResource);
    //---------------------------------------

    mos_bo_set_object_async(pOsResource->bo);

finish:
    return eStatus;
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

    if (pOsInterface->apoMosEnabled)
    {
        // No APO MOS interface support for this func, implement in wrapper
        auto streamState = pOsInterface->osStreamState;

        MOS_OS_CHK_NULL_RETURN(streamState);
        MOS_OS_CHK_NULL_RETURN(streamState->osDeviceContext);

        auto osDeviceContext = streamState->osDeviceContext;

        auto gpuContextMgr = osDeviceContext->GetGpuContextMgr();
        if (gpuContextMgr)
        {
            auto gpuCtx = gpuContextMgr->GetGpuContext(streamState->currentGpuContextHandle);

            auto gpuCtxSpecific = static_cast<GpuContextSpecificNext *>(gpuCtx);
            MOS_OS_CHK_NULL_RETURN(gpuCtxSpecific);
            return (gpuCtxSpecific->VerifyPatchListSize(dwRequestedSize));
        }
    }
    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
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

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::ResetCommandBuffer(pOsInterface->osStreamState, pCmdBuffer);
    }

    if (pOsInterface->CurrentGpuContextOrdinal == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameter GpuContext.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
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
    PGMM_RESOURCE_INFO      pGmmResourceInfo;
    GMM_RESOURCE_FLAG       flags;
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(pOsResource);
    MOS_OS_CHK_NULL_RETURN(pResMmcMode);
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    MediaFeatureTable* skuTable = nullptr;
    skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(skuTable);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetMemoryCompressionMode(pOsInterface->osStreamState, pOsResource, *pResMmcMode);
    }

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);

    flags = pOsResource->pGmmResInfo->GetResFlags();

    if (!flags.Gpu.MMC || !flags.Gpu.CCS)
    {
        *pResMmcMode = MOS_MEMCOMP_DISABLED;
        return MOS_STATUS_SUCCESS;
    }

    if (flags.Info.MediaCompressed || flags.Info.RenderCompressed)
    {
        *pResMmcMode = flags.Info.RenderCompressed ? MOS_MEMCOMP_RC : MOS_MEMCOMP_MC;
    }
    else
    {
        switch  (pGmmResourceInfo->GetMmcMode(0))
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
    }

    uint32_t          MmcFormat = 0;
    GMM_RESOURCE_FORMAT gmmResFmt;

    gmmResFmt = pGmmResourceInfo->GetResourceFormat();

    if (*pResMmcMode == MOS_MEMCOMP_MC         &&
       (!MEDIA_IS_SKU(skuTable, FtrFlatPhysCCS)))
    {
        MmcFormat = static_cast<uint32_t>(pOsInterface->pfnGetGmmClientContext(pOsInterface)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));
        *pResMmcMode = (MmcFormat != 0) ? *pResMmcMode : MOS_MEMCOMP_DISABLED;
    }

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
    PGMM_RESOURCE_INFO      pGmmResourceInfo = nullptr;
    GMM_RESOURCE_MMC_INFO   GmmResMmcMode = GMM_MMC_DISABLED;
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::SetMemoryCompressionMode(pOsInterface->osStreamState, pOsResource, ResMmcMode);
    }

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

    pGmmResourceInfo->SetMmcMode(GmmResMmcMode, 0);

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
    PGMM_RESOURCE_INFO      pGmmResourceInfo = nullptr;
    uint32_t                uiArrayIndex = 0;
    MOS_STATUS              eStatus;
    MOS_OS_FUNCTION_ENTER;
    eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pOsInterface);

     if (pOsInterface->apoMosEnabled)
     {
         return MosInterface::SetMemoryCompressionHint(pOsInterface->osStreamState, pOsResource, bHintOn);
     }

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    pGmmResourceInfo->SetMmcHint(bHintOn ? GMM_MMC_HINT_ON : GMM_MMC_HINT_OFF, uiArrayIndex);

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
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

    PGMM_RESOURCE_INFO      pGmmResourceInfo;
    MOS_STATUS              eStatus = MOS_STATUS_UNKNOWN;
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL(pOsResource);
    MOS_OS_CHK_NULL(pOsInterface);
    MOS_OS_CHK_NULL(pResMmcFormat);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetMemoryCompressionFormat(pOsInterface->osStreamState, pOsResource, pResMmcFormat);
    }

    // Get Gmm resource info
    pGmmResourceInfo = (GMM_RESOURCE_INFO*)pOsResource->pGmmResInfo;
    MOS_OS_CHK_NULL(pGmmResourceInfo);

    MOS_OS_CHK_NULL(pOsInterface->pfnGetGmmClientContext(pOsInterface));
    // Get compression format from GMM RESOURCE FORMAT
    GMM_RESOURCE_FORMAT gmmResFmt;
    gmmResFmt = pGmmResourceInfo->GetResourceFormat();
    MOS_MEMCOMP_STATE   MmcMode;
    uint32_t            MmcFormat;
    Mos_Specific_GetMemoryCompressionMode(pOsInterface, pOsResource, &MmcMode);
    switch (MmcMode)
    {
    case MOS_MEMCOMP_MC:
        MmcFormat = static_cast<uint32_t>(pOsInterface->pfnGetGmmClientContext(pOsInterface)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));
        break;
    case MOS_MEMCOMP_RC:
        MmcFormat = static_cast<uint32_t>(pOsInterface->pfnGetGmmClientContext(pOsInterface)->GetSurfaceStateCompressionFormat(gmmResFmt));
        break;
    default:
        MmcFormat = 0;
    }

    if (MmcFormat > 0x1F)
    {
        MOS_OS_ASSERTMESSAGE("Get a incorrect Compression format(%d) from GMM", MmcFormat);
    }
    else
    {
        *pResMmcFormat = MmcFormat;
        MOS_OS_VERBOSEMESSAGE("GMM compression mode %d, compression format %d", MmcMode, MmcFormat);
    }
    
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
   
    if (pOsContext->semid == MOS_LINUX_IPC_INVALID_ID)
    {
        MOS_OS_ASSERTMESSAGE("Invalid semid in OsContext.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    LockSemaphore(pOsContext->semid);

    pVDBoxWorkLoad = (PVDBOX_WORKLOAD)pOsContext->pShm;
    MOS_OS_ASSERT(pVDBoxWorkLoad);

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
    char  *error;
    MOS_UNUSED(pOsInterface);
    //---------------------------------
    MOS_OS_ASSERT(pFileName);
    //---------------------------------

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
    int32_t bStatus;

    //---------------------------------
    MOS_OS_ASSERT(hInstance);
    //---------------------------------

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
    uint32_t dwResetCount = 0;
    uint32_t dwActiveBatch = 0;
    uint32_t dwPendingBatch = 0;
    int32_t bResult = false;
    int32_t ret = 0;

    if (pOsInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Mos_Specific_IsGPUHung: pOsInterface == NULL");
        return false;
    }

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::IsGPUHung(pOsInterface->osStreamState);
    }

    MOS_LINUX_CONTEXT *intel_i915_ctx = pOsInterface->pOsContext->intel_context;
    if (pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(nullptr))
    {
        auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
        if(gpuContext != nullptr && gpuContext->GetI915Context(0) != nullptr)
        {
            intel_i915_ctx = gpuContext->GetI915Context(0);
        }
    }

    dwResetCount = dwActiveBatch = dwPendingBatch = 0;

    ret = mos_get_reset_stats(intel_i915_ctx, &dwResetCount,
                                &dwActiveBatch, &dwPendingBatch);
    if (ret)
    {
        MOS_OS_NORMALMESSAGE("mos_get_reset_stats return error(%d)\n", ret);
        goto finish;
    }

    //Chip reset will count this value, but maybe caused by ctx in other process.
    //In this case, reset has no impact on current process
    if (dwResetCount != pOsInterface->dwGPUResetCount)
    {
        pOsInterface->dwGPUResetCount   = dwResetCount;
    }

    //Thses two values are ctx specific, it must reset in current process if either changes
    if  (dwActiveBatch != pOsInterface->dwGPUActiveBatch ||
        dwPendingBatch != pOsInterface->dwGPUPendingBatch)
    {
        pOsInterface->dwGPUActiveBatch  = dwActiveBatch;
        pOsInterface->dwGPUPendingBatch = dwPendingBatch;
        bResult = true;
    }

finish:
    return bResult;
}

uint64_t Mos_Specific_GetAuxTableBaseAddr(
    PMOS_INTERFACE              osInterface)
{
    if (osInterface == nullptr)
    {
        MOS_OS_NORMALMESSAGE("Invalid osInterface");
        return 0;
    }
    if(osInterface->apoMosEnabled)
    {
        return MosInterface::GetAuxTableBaseAddr(osInterface->osStreamState);
    }

    if (osInterface->osContextPtr == nullptr)
    {
        MOS_OS_NORMALMESSAGE("Invalid osInterface");
        return 0;
    }
    auto osCtx = static_cast<OsContextSpecific *>(osInterface->osContextPtr);
    AuxTableMgr *auxTableMgr = osCtx->GetAuxTableMgr();

    return auxTableMgr ? auxTableMgr->GetAuxTableBase() : 0;
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
    MOS_OS_ASSERT(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::GetGpuPriority(pOsInterface->osStreamState, pPriority);
    }

    if (pOsInterface->osContextPtr)
    {
        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
        osCxtSpecific->GetGpuPriority(pPriority);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("OS context is nullptr.");
    }
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
    MOS_OS_ASSERT(pOsInterface);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::SetGpuPriority(pOsInterface->osStreamState, priority);
    }

    if (pOsInterface->osContextPtr)
    {
        auto osCxtSpecific = static_cast<OsContextSpecific*>(pOsInterface->osContextPtr);
        osCxtSpecific->SetGpuPriority(priority);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("OS context is nullptr.");
    }
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

void Mos_Specific_NotifyStreamIndexSharing(
    PMOS_INTERFACE              pOsInterface)
{
    MOS_UNUSED(pOsInterface);
}

MOS_STATUS Mos_Specific_CheckVirtualEngineSupported(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_CHK_NULL_RETURN(osInterface);

    auto skuTable = osInterface->pfnGetSkuTable(osInterface);
    MOS_OS_CHK_NULL_RETURN(skuTable);
    if (MEDIA_IS_SKU(skuTable, FtrContextBasedScheduling))
    {
        osInterface->bSupportVirtualEngine = true;
    }
    else
    {
        osInterface->bSupportVirtualEngine = false;
    }

    return MOS_STATUS_SUCCESS;
}

MediaUserSettingSharedPtr Mos_Specific_GetUserSettingInstance(
    PMOS_INTERFACE osInterface)
{
    MOS_OS_FUNCTION_ENTER;
    if (!osInterface)
    {
        MOS_OS_ASSERTMESSAGE("Invalid osInterface ptr");
        return nullptr;
    }

    if (osInterface->apoMosEnabled)
    {
        return MosInterface::MosGetUserSettingInstance(osInterface->osStreamState);
    }
    return MosInterface::MosGetUserSettingInstance(osInterface->pOsContext);
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

    if (GFX_IS_GEN_11_OR_LATER(Platform) || Mos_Solo_IsEnabled(nullptr))
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
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_FUNCTION_ENTER;

    return false;
}

MOS_STATUS Mos_Specific_pfnSetMultiEngineEnabled(
    PMOS_INTERFACE pOsInterface,
    MOS_COMPONENT  component,
    bool           enabled)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_pfnGetMultiEngineStatus(
    PMOS_INTERFACE pOsInterface,
    PLATFORM      *platform,
    MOS_COMPONENT  component,
    bool          &isMultiDevices,
    bool          &isMultiEngine)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_STATUS_SUCCESS;
}

MOS_GPU_NODE Mos_Specific_pfnGetLatestVirtualNode(
    PMOS_INTERFACE pOsInterface,
    MOS_COMPONENT  component)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_GPU_NODE_MAX;
}

void Mos_Specific_pfnSetLatestVirtualNode(
    PMOS_INTERFACE pOsInterface,
    MOS_GPU_NODE   node)
{
    MOS_OS_FUNCTION_ENTER;
}

MOS_GPU_NODE Mos_Specific_pfnGetDecoderVirtualNodePerStream(
    PMOS_INTERFACE pOsInterface)
{
    MOS_OS_FUNCTION_ENTER;

    return MOS_GPU_NODE_MAX;
}

void Mos_Specific_pfnSetDecoderVirtualNodePerStream(
    PMOS_INTERFACE pOsInterface,
    MOS_GPU_NODE   node)
{
    MOS_OS_FUNCTION_ENTER;
}

bool Mos_Specific_IsAsyncDevice(PMOS_INTERFACE pOsInterface)
{
    return false;
}

bool Mos_Specific_IsGpuSyncByCmd(
    PMOS_INTERFACE osInterface)
{
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
    MediaUserSettingSharedPtr       userSettingPtr           = nullptr;
    bool                            bSimIsActive = false;
    bool                            useCustomerValue = false;
    uint32_t                        regValue = 0;
    char                            *pMediaWatchdog = nullptr;
    long int                        watchdog = 0;

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
    pOsInterface->pfnMonoSurfaceCopy                        = Mos_Specific_MonoSurfaceCopy;
    pOsInterface->pfnGetMosContext                          = Mos_Specific_GetMosContext;
    pOsInterface->pfnUpdateResourceUsageType                = Mos_Specific_UpdateResourceUsageType;
    pOsInterface->pfnRegisterResource                       = Mos_Specific_RegisterResource;
    pOsInterface->pfnResetResourceAllocationIndex           = Mos_Specific_ResetResourceAllocationIndex;
    pOsInterface->pfnGetResourceAllocationIndex             = Mos_Specific_GetResourceAllocationIndex;
    pOsInterface->pfnGetResourceGfxAddress                  = Mos_Specific_GetResourceGfxAddress;
    pOsInterface->pfnGetResourceClearAddress                = Mos_Specific_GetResourceClearAddress;
    pOsInterface->pfnGetCommandBuffer                       = Mos_Specific_GetCommandBuffer;
    pOsInterface->pfnResetCommandBuffer                     = Mos_Specific_ResetCommandBuffer;
    pOsInterface->pfnReturnCommandBuffer                    = Mos_Specific_ReturnCommandBuffer;
    pOsInterface->pfnSubmitCommandBuffer                    = Mos_Specific_SubmitCommandBuffer;
    pOsInterface->pfnWaitAndReleaseCmdBuffer                = Mos_Specific_WaitAndReleaseCmdBuffer;
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
    pOsInterface->pfnCheckVirtualEngineSupported            = Mos_Specific_CheckVirtualEngineSupported;

    //GPU context and synchronization functions
    pOsInterface->pfnCreateGpuContext                       = Mos_Specific_CreateGpuContext;
    pOsInterface->pfnCreateGpuComputeContext                = Mos_Specific_CreateGpuComputeContext;
    pOsInterface->pfnDestroyGpuContext                      = Mos_Specific_DestroyGpuContext;
    pOsInterface->pfnDestroyGpuContextByHandle              = Mos_Specific_DestroyGpuContextByHandle;
    pOsInterface->pfnDestroyGpuComputeContext               = Mos_Specific_DestroyGpuComputeContext;
    pOsInterface->pfnIsGpuContextValid                      = Mos_Specific_IsGpuContextValid;
    pOsInterface->pfnSyncOnResource                         = Mos_Specific_SyncOnResource;
    pOsInterface->pfnSyncGpuContext                         = Mos_Specific_SyncGpuContext;
    pOsInterface->pfnGetGpuStatusBufferResource             = Mos_Specific_GetGpuStatusBufferResource;
    pOsInterface->pfnGetGpuStatusTagOffset                  = Mos_Specific_GetGpuStatusTagOffset;
    pOsInterface->pfnGetGpuStatusTag                        = Mos_Specific_GetGpuStatusTag;
    pOsInterface->pfnIncrementGpuStatusTag                  = Mos_Specific_IncrementGpuStatusTag;
    pOsInterface->pfnGetGpuStatusSyncTag                    = Mos_Specific_GetGpuStatusSyncTag;
    pOsInterface->pfnSetResourceSyncTag                     = Mos_Specific_SetResourceSyncTag;
    pOsInterface->pfnPerformOverlaySync                     = Mos_Specific_PerformOverlaySync;
    pOsInterface->pfnEngineSignal                           = Mos_Specific_EngineSignal;
    pOsInterface->pfnEngineWait                             = Mos_Specific_EngineWait;
    pOsInterface->pfnWaitAllCmdCompletion                   = Mos_Specific_WaitAllCmdCompletion_Os;
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
    pOsInterface->pfnSetSliceCount                          = Mos_Specific_SetSliceCount;
    pOsInterface->pfnGetResourceIndex                       = Mos_Specific_GetResourceIndex;
    pOsInterface->pfnGetGpuPriority                         = Mos_Specific_GetGpuPriority;
    pOsInterface->pfnSetGpuPriority                         = Mos_Specific_SetGpuPriority;
    pOsInterface->pfnIsSetMarkerEnabled                     = Mos_Specific_IsSetMarkerEnabled;
    pOsInterface->pfnGetMarkerResource                      = Mos_Specific_GetMarkerResource;
    pOsInterface->pfnNotifyStreamIndexSharing               = Mos_Specific_NotifyStreamIndexSharing;

    pOsInterface->pfnSetGpuContextHandle                    = Mos_Specific_SetGpuContextHandle;
    pOsInterface->pfnGetGpuContextbyHandle                  = Mos_Specific_GetGpuContextbyHandle;

    pOsInterface->pfnGetUserSettingInstance                 = Mos_Specific_GetUserSettingInstance;

    pOsInterface->pfnIsMismatchOrderProgrammingSupported    = Mos_Specific_IsMismatchOrderProgrammingSupported;
    pOsInterface->pfnIsMultipleCodecDevicesInUse            = Mos_Specific_pfnIsMultipleCodecDevicesInUse;
    pOsInterface->pfnSetMultiEngineEnabled                  = Mos_Specific_pfnSetMultiEngineEnabled;
    pOsInterface->pfnGetMultiEngineStatus                   = Mos_Specific_pfnGetMultiEngineStatus;
    pOsInterface->pfnGetLatestVirtualNode                   = Mos_Specific_pfnGetLatestVirtualNode;
    pOsInterface->pfnSetLatestVirtualNode                   = Mos_Specific_pfnSetLatestVirtualNode;
    pOsInterface->pfnGetDecoderVirtualNodePerStream         = Mos_Specific_pfnGetDecoderVirtualNodePerStream;
    pOsInterface->pfnSetDecoderVirtualNodePerStream         = Mos_Specific_pfnSetDecoderVirtualNodePerStream;

    pOsInterface->pfnGetResType                             = GetResType;
    pOsInterface->pfnGetTsFrequency                         = Mos_Specific_GetTsFrequency;
    pOsInterface->pfnSetHintParams                          = Mos_Specific_SetHintParams;
    pOsInterface->pfnIsResourceReleasable                   = Mos_Specific_IsResourceReleasable;
    pOsInterface->pfnVirtualEngineInit                      = Mos_Specific_Virtual_Engine_Init;
    pOsInterface->pfnDestroyVeInterface                     = Mos_Specific_DestroyVeInterface;
    pOsInterface->pfnVirtualEngineInterfaceInitialize       = Mos_VirtualEngineInterface_Initialize;

    pOsInterface->pfnCreateCmDevice                         = CreateCmDevice;
    pOsInterface->pfnDestroyCmDevice                        = DestroyCmDevice;
    pOsInterface->pfnInitCmInterface                        = InitCmOsDDIInterface;

    pOsInterface->pfnIsAsynDevice                           = Mos_Specific_IsAsyncDevice;
    pOsInterface->pfnIsGpuSyncByCmd                         = Mos_Specific_IsGpuSyncByCmd;

#if (_DEBUG || _RELEASE_INTERNAL)
    pOsInterface->pfnGetEngineLogicId                       = Mos_Specific_GetEngineLogicId;
#endif
    pOsContext              = nullptr;
    pOsUserFeatureInterface = (PMOS_USER_FEATURE_INTERFACE)&pOsInterface->UserFeatureInterface;

    MOS_OS_NORMALMESSAGE("mm:Mos_Specific_InitInterface called.");

    pOsInterface->modularizedGpuCtxEnabled  = true;
    pOsInterface->veDefaultEnable           = true;
    pOsInterface->phasedSubmission          = true;

    pOsInterface->apoMosEnabled             = pOsDriverContext->m_apoMosEnabled;
    if (pOsInterface->apoMosEnabled)
    {
        MOS_OS_CHK_NULL(pOsDriverContext->m_osDeviceContext);
        pOsInterface->bNullHwIsEnabled  = pOsDriverContext->m_osDeviceContext->GetNullHwIsEnabled();
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
    }
    else
    {
        // Create Linux OS Context
        pOsContext = MOS_New(MOS_OS_CONTEXT);
        MOS_OS_CHK_NULL_RETURN(pOsContext);
    }

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

        //ApoMos do it in CreateOsStreamState
        if (!pOsInterface->apoMosEnabled)
        {
            OsContextSpecific *pOsContextSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);
            pOsContext->intel_context             = pOsContextSpecific->GetDrmContext();
            pOsContext->pGmmClientContext         = pOsDriverContext->pGmmClientContext;
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
    if (!pOsInterface->apoMosEnabled)
    {
        pOsContext->bSimIsActive = pOsInterface->bSimIsActive;
    }

    // Initialize
    if (!pOsInterface->apoMosEnabled)
    {
        modularizedGpuCtxEnabled = pOsInterface->modularizedGpuCtxEnabled && !Mos_Solo_IsEnabled(pOsContext);
        eStatus = Linux_InitContext(pOsContext, pOsDriverContext, pOsInterface->modulizedMosEnabled && !Mos_Solo_IsEnabled(pOsContext), modularizedGpuCtxEnabled);
        if( MOS_STATUS_SUCCESS != eStatus )
        {
            MOS_OS_ASSERTMESSAGE("Unable to initialize context.");
            goto finish;
        }

        pOsContext->bFreeContext = true;

        //Added by Ben for video memory allocation
        pOsContext->bufmgr = pOsDriverContext->bufmgr;
        mos_bufmgr_enable_reuse(pOsDriverContext->bufmgr);
    }

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
    pOsInterface->trinityPath         = TRINITY_DISABLED;
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

    // Check SKU table to detect if simulation environment (HAS) is enabled
    pSkuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    MOS_OS_CHK_NULL(pSkuTable);

    // disable Media Reset for non-xe platform who has no media reset support on Linux
    if(!MEDIA_IS_SKU(pSkuTable, FtrSWMediaReset))
    {
        pOsInterface->bMediaReset         = false;
        pOsInterface->umdMediaResetEnable = false;
    }

    // initialize MOS_CP interface
    pOsInterface->osCpInterface = Create_MosCpInterface(pOsInterface);
    if (pOsInterface->osCpInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("fail to create osCpInterface.");
        return MOS_STATUS_UNKNOWN;
    }

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

//!
//! \brief    Checks whether the requested resource is releasable
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if requested can be released, otherwise MOS_STATUS_UNKNOWN
//!
MOS_STATUS Mos_Specific_IsResourceReleasable(
    PMOS_INTERFACE         pOsInterface,
    PMOS_RESOURCE          pOsResource)
{
    return MOS_STATUS_SUCCESS;
}

MOS_USER_FEATURE_KEY_PATH_INFO *Mos_GetDeviceUfPathInfo(
    PMOS_CONTEXT mosContext)
{
    MOS_UNUSED(mosContext);
    //NOT IMPLEMENTED 
    return nullptr;
}

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
struct GpuCmdResInfoDump::GpuCmdResInfo
{
    int32_t             iWidth;
    int32_t             iHeight;
    int32_t             iSize;
    int32_t             iPitch;
    int32_t             iDepth;
    MOS_FORMAT          Format;
    int32_t             iCount;
    int32_t             iAllocationIndex[MOS_GPU_CONTEXT_MAX];
    uint32_t            dwGfxAddress;
    const char          *bufname;
    uint32_t            isTiled;
    MOS_TILE_TYPE       TileType;
    uint32_t            bMapped;
    uint32_t            name;
    uint64_t            user_provided_va;
    bool                bConvertedFromDDIResource;
};

void GpuCmdResInfoDump::StoreCmdResPtr(PMOS_INTERFACE pOsInterface, const void *pRes) const
{
    if (!m_dumpEnabled)
    {
        return;
    }
    auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_ASSERT(gpuContext != nullptr);

    auto pResTmp1 = (const MOS_RESOURCE *)(pRes);
    auto pResTmp2 = (GpuCmdResInfo *)MOS_AllocMemory(sizeof(GpuCmdResInfo));

    pResTmp2->iWidth                    = pResTmp1->iWidth;
    pResTmp2->iHeight                   = pResTmp1->iHeight;
    pResTmp2->iSize                     = pResTmp1->iSize;
    pResTmp2->iPitch                    = pResTmp1->iPitch;
    pResTmp2->iDepth                    = pResTmp1->iDepth;
    pResTmp2->Format                    = pResTmp1->Format;
    pResTmp2->iCount                    = pResTmp1->iCount;
    for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        pResTmp2->iAllocationIndex[i] = pResTmp1->iAllocationIndex[i];
    }
    pResTmp2->dwGfxAddress              = pResTmp1->dwGfxAddress;
    pResTmp2->bufname                   = pResTmp1->bufname;
    pResTmp2->isTiled                   = pResTmp1->isTiled;
    pResTmp2->TileType                  = pResTmp1->TileType;
    pResTmp2->bMapped                   = pResTmp1->bMapped;
    pResTmp2->name                      = pResTmp1->name;
    pResTmp2->user_provided_va          = pResTmp1->user_provided_va;
    pResTmp2->bConvertedFromDDIResource = pResTmp1->bConvertedFromDDIResource;

    gpuContext->PushCmdResPtr((const void *)pResTmp2);
}

void GpuCmdResInfoDump::ClearCmdResPtrs(PMOS_INTERFACE pOsInterface) const
{
    if (!m_dumpEnabled)
    {
        return;
    }

    auto gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_ASSERT(gpuContext != nullptr);

    auto &cmdResInfoPtrs = gpuContext->GetCmdResPtrs();

    for (auto e : cmdResInfoPtrs)
    {
        MOS_FreeMemory((void *)e);
    }

    gpuContext->ClearCmdResPtrs();
}

void GpuCmdResInfoDump::Dump(const void *cmdResInfoPtr, std::ofstream &outputFile) const
{
    using std::endl;

    auto pRes = (const GpuCmdResInfo *)(cmdResInfoPtr);

    outputFile << "Gpu Resource Pointer      : " << pRes << endl;
    outputFile << "iWidth                    : " << pRes->iWidth << endl;
    outputFile << "iHeight                   : " << pRes->iHeight << endl;
    outputFile << "iSize                     : " << pRes->iSize << endl;
    outputFile << "iPitch                    : " << pRes->iPitch << endl;
    outputFile << "iDepth                    : " << pRes->iDepth << endl;
    outputFile << "Format                    : " << (int32_t)pRes->Format << endl;
    outputFile << "iCount                    : " << pRes->iCount << endl;
    outputFile << "iAllocationIndex          : ";
    for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        outputFile << pRes->iAllocationIndex[i] << " ";
    }
    outputFile << endl;
    outputFile << "dwGfxAddress              : " << pRes->dwGfxAddress << endl;
    outputFile << "bufname                   : " << pRes->bufname << endl;
    outputFile << "isTiled                   : " << pRes->isTiled << endl;
    outputFile << "TileType                  : " << GetTileType(pRes->TileType) << endl;
    outputFile << "bMapped                   : " << pRes->bMapped << endl;
    outputFile << "name                      : " << pRes->name << endl;
    outputFile << "user_provided_va          : " << pRes->user_provided_va << endl;
    outputFile << "bConvertedFromDDIResource : " << pRes->bConvertedFromDDIResource << endl;
    outputFile << endl;
}

const std::vector<const void *> &GpuCmdResInfoDump::GetCmdResPtrs(PMOS_INTERFACE pOsInterface) const
{
    const auto *gpuContext = Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_ASSERT(gpuContext != nullptr);

    return gpuContext->GetCmdResPtrs();
}
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

MOS_STATUS Mos_Specific_Virtual_Engine_Init(
    PMOS_INTERFACE pOsInterface,
    PMOS_VIRTUALENGINE_HINT_PARAMS* veHitParams,
    MOS_VIRTUALENGINE_INIT_PARAMS&  veInParams)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    if (pOsInterface->apoMosEnabled)
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);
        MOS_VE_HANDLE veState = nullptr;
        MOS_OS_CHK_STATUS_RETURN(MosInterface::CreateVirtualEngineState(
            pOsInterface->osStreamState, &veInParams, veState));

        MOS_OS_CHK_STATUS_RETURN(MosInterface::GetVeHintParams(pOsInterface->osStreamState, veInParams.bScalabilitySupported, veHitParams));
    }
    else
    {
        MOS_OS_CHK_STATUS_RETURN(Mos_VirtualEngineInterface_Initialize(pOsInterface, &veInParams));
        PMOS_VIRTUALENGINE_INTERFACE veInterface = pOsInterface->pVEInterf;
        MOS_OS_CHK_NULL_RETURN(veInterface);
        if (veInterface->pfnVEGetHintParams)
        {
            MOS_OS_CHK_STATUS_RETURN(veInterface->pfnVEGetHintParams(veInterface, veInParams.bScalabilitySupported, veHitParams));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_SetHintParams(
    PMOS_INTERFACE                pOsInterface,
    PMOS_VIRTUALENGINE_SET_PARAMS veParams)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    if (pOsInterface->apoMosEnabled)
    {
        MOS_OS_CHK_NULL_RETURN(pOsInterface->osStreamState);
        MOS_OS_CHK_STATUS_RETURN(MosInterface::SetVeHintParams(pOsInterface->osStreamState, veParams));
    }
    else
    {
        PMOS_VIRTUALENGINE_INTERFACE veInterface = pOsInterface->pVEInterf;
        MOS_OS_CHK_NULL_RETURN(veInterface);
        if (veInterface->pfnVESetHintParams)
        {
            MOS_OS_CHK_STATUS_RETURN(veInterface->pfnVESetHintParams(veInterface, veParams));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mos_Specific_DestroyVeInterface(
    PMOS_VIRTUALENGINE_INTERFACE *veInterface)
{
    MOS_OS_FUNCTION_ENTER;
    if (*veInterface)
    {
        if ((*veInterface)->pfnVEDestroy)
        {
            (*veInterface)->pfnVEDestroy(*veInterface);
        }
        MOS_FreeMemAndSetNull(*veInterface);
    }

    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS Mos_Specific_GetEngineLogicId(
    PMOS_INTERFACE pOsInterface,
    uint8_t& id)
{
    if (pOsInterface->apoMosEnabled)
    {
        if (MosInterface::GetVeEngineCount(pOsInterface->osStreamState) != 1)
        {
            MOS_OS_ASSERTMESSAGE("VeEngineCount is not equal to 1.");
        }
        id = MosInterface::GetEngineLogicId(pOsInterface->osStreamState, 0);
    }
    else
    {
        PMOS_VIRTUALENGINE_INTERFACE veInterface = pOsInterface->pVEInterf;
        if (veInterface->ucEngineCount != 1)
        {
            MOS_OS_ASSERTMESSAGE("ucEngineCount is not equal to 1.");
        }
        id = veInterface->EngineLogicId[0];
    }

    return MOS_STATUS_SUCCESS;
}
#endif
