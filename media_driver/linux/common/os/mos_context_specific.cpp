/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mos_context_specific.cpp
//! \brief    Container for Linux/Android specific parameters shared across different GPU contexts of the same device instance 
//!

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_resource_defs.h"
#include <unistd.h>
#include <dlfcn.h>
#include "hwinfo_linux.h"
#include <stdlib.h>

#ifndef ANDROID
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <time.h>
#endif

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"

#include "mos_context_specific.h"
#include "mos_gpucontextmgr.h"
#include "mos_cmdbufmgr.h"

OsContextSpecific::OsContextSpecific()
{
    for (int i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        m_GpuContextHandle[i] = MOS_GPU_CONTEXT_INVALID_HANDLE;
    }

    MOS_OS_FUNCTION_ENTER;
}

OsContextSpecific::~OsContextSpecific()
{
    MOS_OS_FUNCTION_ENTER;
}

#ifndef ANDROID
MOS_STATUS OsContextSpecific::LockSemaphore(int32_t semid)
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

MOS_STATUS OsContextSpecific::UnLockSemaphore(int32_t semid)
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

int16_t OsContextSpecific::ShmAttachedNumber(unsigned int shmid)
{
    struct shmid_ds buf;
    MOS_ZeroMemory(&buf, sizeof(buf));

    if (shmctl(shmid, IPC_STAT, &buf) < 0)
    {
        return -1;
    }

    return buf.shm_nattch;
}

MOS_STATUS OsContextSpecific::DestroySemaphore(unsigned int semid)
{
    int32_t nwait = 0;

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

MOS_STATUS OsContextSpecific::ConnectCreateShm(long key, uint32_t size, int32_t *pShmid, void* *ppShm)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pShmid);
    MOS_OS_CHK_NULL_RETURN(ppShm);

    struct shmid_ds buf;
    int32_t         shmid = 0;
    key_t           key_value = (key_t)key;
    void            *shmptr = nullptr;
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

MOS_STATUS OsContextSpecific::DetachDestroyShm(int32_t shmid, void  *pShm)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pShm);

    struct shmid_ds buf;
    MOS_ZeroMemory(&buf, sizeof(buf));

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

MOS_STATUS OsContextSpecific::ConnectCreateSemaphore(long key, int32_t *pSemid)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pSemid);

    int32_t         semid = 0;
    struct sembuf   sop;
    struct semid_ds buf;
    key_t           key_value = (key_t)key;
    int32_t         val = 0;

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

MOS_STATUS OsContextSpecific::CreateIPC()
{
    MOS_STATUS eStatus;

    m_semId = MOS_LINUX_IPC_INVALID_ID;
    m_shmId = MOS_LINUX_IPC_INVALID_ID;
    m_shm   = MOS_LINUX_SHM_INVALID;

    struct semid_ds buf;
    MOS_ZeroMemory(&buf, sizeof(buf));
    //wait and retry till to get a valid semphore
    for(int i = 0; i < MOS_LINUX_SEM_MAX_TRIES; i++)
    {
        ConnectCreateSemaphore(m_dualVdboxKey, &m_semId);

        if (m_semId == MOS_LINUX_IPC_INVALID_ID)
        {
            return MOS_STATUS_UNKNOWN;
        }
        //check wether the semid is initialized or not
        if (semctl(m_semId, 0, IPC_STAT, &buf) == -1)
        {
            return MOS_STATUS_UNKNOWN;
        }
        if (buf.sem_otime != 0)
        {
            break;
        }

        MOS_Sleep(1); //wait and retry
    }

    LockSemaphore(m_semId);
    eStatus = ConnectCreateShm(m_dualVdboxKey, sizeof(VDBOX_WORKLOAD), &(m_shmId), &(m_shm));
    UnLockSemaphore(m_semId);
    MOS_CHK_STATUS_SAFE(eStatus);

finish:
    return eStatus;
}

void OsContextSpecific::DestroyIPC()
{
    if (MOS_LINUX_IPC_INVALID_ID != m_semId)
    {
        int16_t iAttachedNum = 0;

        if (MOS_LINUX_IPC_INVALID_ID != m_shmId)
        {
            LockSemaphore(m_semId);
            iAttachedNum = ShmAttachedNumber(m_shmId);

            DetachDestroyShm(m_shmId, m_shm);
            m_shmId = MOS_LINUX_IPC_INVALID_ID;
            m_shm   = MOS_LINUX_SHM_INVALID;

            if (iAttachedNum)
            {
                --iAttachedNum;
            }
            UnLockSemaphore(m_semId);
        }
    }
}

MOS_STATUS OsContextSpecific::CreateSSEUIPC()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_sseuSemId = MOS_LINUX_IPC_INVALID_ID;
    m_sseuShmId = MOS_LINUX_IPC_INVALID_ID;
    m_sseuShm   = MOS_LINUX_SHM_INVALID;

    return eStatus;
}

void OsContextSpecific::DestroySSEUIPC()
{
    if (MOS_LINUX_IPC_INVALID_ID != m_sseuSemId)
    {
        short iAttachedNum = 0;

        if (MOS_LINUX_IPC_INVALID_ID != m_sseuShmId)
        {
            LockSemaphore(m_sseuSemId);
            iAttachedNum = ShmAttachedNumber(m_sseuShmId);

            DetachDestroyShm(m_sseuShmId, m_sseuShm);
            m_sseuShmId = MOS_LINUX_IPC_INVALID_ID;
            m_sseuShm   = MOS_LINUX_SHM_INVALID;

            if (iAttachedNum)
            {
                --iAttachedNum;
            }
            UnLockSemaphore(m_sseuSemId);
        }
    }
}

void OsContextSpecific::SetSliceCount(uint32_t *pSliceCount)
{
    if (pSliceCount == nullptr)
        MOS_OS_ASSERTMESSAGE("pSliceCount is NULL.");
}

#endif //#ifndef ANDROID

MOS_STATUS OsContextSpecific::Init(PMOS_CONTEXT pOsDriverContext)
{
    uint32_t      iDeviceId = 0;
    MOS_STATUS    eStatus;
    uint32_t      i = 0;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_STATUS_SUCCESS;

    if (GetOsContextValid() == false)
    {
        if( nullptr == pOsDriverContext         ||
            nullptr == pOsDriverContext->bufmgr ||
            0 >= pOsDriverContext->fd )
        {
            MOS_OS_ASSERT(false);
            return MOS_STATUS_INVALID_HANDLE;
        }
    
        m_bufmgr        = pOsDriverContext->bufmgr;
        m_gpuContextMgr = static_cast<GpuContextMgr *>(pOsDriverContext->m_gpuContextMgr);
        m_cmdBufMgr     = static_cast<CmdBufMgr *>(pOsDriverContext->m_cmdBufMgr);
        m_fd            = pOsDriverContext->fd;
        MOS_SecureMemcpy(&m_perfData, sizeof(PERF_DATA), pOsDriverContext->pPerfData, sizeof(PERF_DATA));
        mos_bufmgr_gem_enable_reuse(pOsDriverContext->bufmgr);
        m_pGmmClientContext = pOsDriverContext->pGmmClientContext;
        m_auxTableMgr = pOsDriverContext->m_auxTableMgr;
    
        // DDI layer can pass over the DeviceID.
        iDeviceId = pOsDriverContext->iDeviceId;
        if (0 == iDeviceId)
        {
            PLATFORM           platformInfo;
            MEDIA_FEATURE_TABLE  skuTable;
            MEDIA_WA_TABLE       waTable;
            MEDIA_SYSTEM_INFO    gtSystemInfo;
    
            MOS_ZeroMemory(&platformInfo, sizeof(platformInfo));
            MOS_ZeroMemory(&skuTable, sizeof(skuTable));
            MOS_ZeroMemory(&waTable, sizeof(waTable));
            MOS_ZeroMemory(&gtSystemInfo, sizeof(gtSystemInfo));
            eStatus = HWInfo_GetGfxInfo(pOsDriverContext->fd, &platformInfo, &skuTable, &waTable, &gtSystemInfo);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Fatal error - unsuccesfull Sku/Wa/GtSystemInfo initialization");
                return eStatus;
            }
    
            MOS_SecureMemcpy(&m_platformInfo, sizeof(PLATFORM), &platformInfo, sizeof(PLATFORM));
            MOS_SecureMemcpy(&m_gtSystemInfo, sizeof(MEDIA_SYSTEM_INFO), &gtSystemInfo, sizeof(MEDIA_SYSTEM_INFO));
    
            pOsDriverContext->iDeviceId      = platformInfo.usDeviceID;
            m_skuTable = skuTable;
            m_waTable  = waTable;
    
            pOsDriverContext->SkuTable       = skuTable;
            pOsDriverContext->WaTable        = waTable;
            pOsDriverContext->gtSystemInfo   = gtSystemInfo;
            pOsDriverContext->platform       = platformInfo;
    
            MOS_OS_NORMALMESSAGE("DeviceID was created DeviceID = %d, platform product %d", iDeviceId, platformInfo.eProductFamily);
        }
        else
        {
            // pOsDriverContext's parameters were passed by CmCreateDevice.
            // Get SkuTable/WaTable/systemInfo/platform from OSDriver directly.
            MOS_SecureMemcpy(&m_platformInfo, sizeof(PLATFORM), &(pOsDriverContext->platform), sizeof(PLATFORM));
            MOS_SecureMemcpy(&m_gtSystemInfo, sizeof(MEDIA_SYSTEM_INFO), &(pOsDriverContext->gtSystemInfo), sizeof(MEDIA_SYSTEM_INFO));
    
            m_skuTable = pOsDriverContext->SkuTable;
            m_waTable  = pOsDriverContext->WaTable;
        }

        m_use64BitRelocs = true;
        m_useSwSwizzling = MEDIA_IS_SKU(&m_skuTable, FtrSimulationMode)
                        || MEDIA_IS_SKU(&m_skuTable, FtrUseSwSwizzling);
        m_tileYFlag      = MEDIA_IS_SKU(&m_skuTable, FtrTileY);
    
        if (!Mos_Solo_IsEnabled() && MEDIA_IS_SKU(&m_skuTable,FtrContextBasedScheduling))
        {
            m_intelContext = mos_gem_context_create_ext(pOsDriverContext->bufmgr,0);
            if (m_intelContext)
            {
                m_intelContext->vm = mos_gem_vm_create(pOsDriverContext->bufmgr);
                if (m_intelContext->vm == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("Failed to create vm.\n");
                    return MOS_STATUS_UNKNOWN;
                }
            }
        }
        else //use legacy context create ioctl for pre-gen11 platforms
        {
           m_intelContext = mos_gem_context_create(pOsDriverContext->bufmgr);
           if (m_intelContext)
           {
               m_intelContext->vm = nullptr;
           }
        }

        if (m_intelContext == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to create drm intel context");
            return MOS_STATUS_UNKNOWN;
        }

        m_isAtomSOC = IS_ATOMSOC(iDeviceId);
    
    #ifndef ANDROID
    
        if ((m_gtSystemInfo.VDBoxInfo.IsValid) && (m_gtSystemInfo.VDBoxInfo.NumberOfVDBoxEnabled > 1))
        {
            m_kmdHasVCS2 = true;
        }
        else
        {
            m_kmdHasVCS2 = false;
        }
    
        if (m_kmdHasVCS2)
        {
            eStatus = CreateIPC();
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Fatal error - create IPC failed");
                return eStatus;
            }
        }
    
        eStatus = CreateSSEUIPC();
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Fatal error - Failed to create shared memory for SSEU configuration.");
            return eStatus;
        }
    #endif
    
        m_transcryptedKernels       = nullptr;
        m_transcryptedKernelsSize   = 0;
    
        // For Media Memory compression
        m_mediaMemDecompState       = pOsDriverContext->ppMediaMemDecompState;
        m_memoryDecompress       = pOsDriverContext->pfnMemoryDecompress;
        m_mosContext                = pOsDriverContext;
    
        m_noParsingAssistanceInKmd  = true;
        m_numNalUnitBytesIncluded   = MOS_NAL_UNIT_LENGTH - MOS_NAL_UNIT_STARTCODE_LENGTH;
    
        // Init reset count for the context
        uint32_t dwResetCount       = 0;
        mos_get_reset_stats(m_intelContext, &dwResetCount, nullptr, nullptr);
        m_gpuResetCount             = dwResetCount;
        m_gpuActiveBatch            = 0;
        m_gpuPendingBatch           = 0;
    
        m_usesPatchList             = true;
        m_usesGfxAddress            = false;
    
        m_inlineCodecStatusUpdate   = true;
    
    #if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
        CommandBufferDumpInit();
    #endif
    
        SetOsContextValid(true);
    }
    return eStatus;
}

void OsContextSpecific::Destroy()
{
    MOS_OS_FUNCTION_ENTER;

    if (GetOsContextValid() == true)
    {
        // APO MOS will destory each stream's GPU context at different place
        if (!g_apoMosEnabled)
        {
            for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
            {
                if (m_GpuContextHandle[i] != MOS_GPU_CONTEXT_INVALID_HANDLE)
                {
                    if (m_gpuContextMgr == nullptr)
                    {
                        MOS_OS_ASSERTMESSAGE("GpuContextMgr is null when destroy GpuContext");
                        break;
                    }
                    auto gpuContext = m_gpuContextMgr->GetGpuContext(m_GpuContextHandle[i]);
                    if (gpuContext == nullptr)
                    {
                        MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active gpuContextHandle");
                        continue;
                    }
                    m_gpuContextMgr->DestroyGpuContext(gpuContext);
                }
            }
        }
    
     #ifndef ANDROID
        if (m_kmdHasVCS2)
        {
            DestroyIPC();
        }
        DestroySSEUIPC();
     #endif
        m_skuTable.reset();
        m_waTable.reset();
        if (m_intelContext && m_intelContext->vm)
        {
            mos_gem_vm_destroy(m_intelContext->bufmgr, m_intelContext->vm);
        }
        if (m_intelContext)
        {
            mos_gem_context_destroy(m_intelContext);
        }
        SetOsContextValid(false);
    }
}

