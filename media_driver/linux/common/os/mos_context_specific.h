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
//! \file     mos_context_specific.h
//! \brief    Container for Linux/Android specific parameters shared across different GPU contexts of the same device instance
//!

#ifndef __MOS_CONTEXT_SPECIFIC_H__
#define __MOS_CONTEXT_SPECIFIC_H__

#include "mos_os_specific.h"
#include "mos_context.h"
#include "mos_auxtable_mgr.h"

class GraphicsResourceSpecific;
class CmdBufMgr;
class GpuContextMgr;

class OsContextSpecific : public OsContext
{
    friend class GraphicsResourceSpecific;

    //!
    //! \brief SW semaphore key for share memory btw dual VDBox
    //!
    constexpr static uint32_t  m_dualVdboxKey= ('D'<<24|'V'<<8|'X'<<0);

    //!
    //! \brief SW semaphore key for share memory to store SSEU configration
    //!
    constexpr static uint32_t  m_sseuKey= ('S'<<24|'S'<<16|'E'<<8|'U'<<0);

    //!
    //! \brief shared memory size
    //!
    constexpr static uint32_t m_sseuShmSize = 256;

    //!
    //! \brief slice count value timeout critera in milliseconds
    //!
    constexpr static uint64_t m_sliceCountTimeoutMS = 1000;

#ifndef ANDROID
    //!
    //! \brief Initial share memory handle
    //!
    constexpr static void* MOS_LINUX_SHM_INVALID      = (void *)nullptr;

    //!
    //! \brief Initial share memory ID
    //!
    constexpr static int32_t MOS_LINUX_IPC_INVALID_ID = -1;

    //!
    //! \brief maximum number to try to get a valid semaphore
    //!
    constexpr static uint32_t MOS_LINUX_SEM_MAX_TRIES = 10;
#endif

public:
    //!
    //! \brief  Do not disable kmd watchdog, that is to say, pass
    //!         < 0: I915_EXEC_ENABLE_WATCHDOG flag to KMD;
    //!         < 1: Disable kmd watchdog;
    //!         that is to say, DO NOT pass I915_EXEC_ENABLE_WATCHDOG flag to KMD;
    struct PerfInfo {
        bool     m_disableKmdWatchdog;
        uint32_t m_enablePerfTag;
    } ;

    //!
    //! \brief  Constructor
    //!
    OsContextSpecific();

    //!
    //! \brief  Destructor
    //!
    ~OsContextSpecific();

    //!
    //! \brief  Initialize the MOS Context
    //! \param  [in] pOsDriverContext
    //!         ptr to MOS_CONTEXT created inside DDI 
    //! \return MOS_Success in pass case, MOS error status in fail cases
    //!
    MOS_STATUS Init(PMOS_CONTEXT osDriverContext);

    //!
    //! \brief  Destroy the os specific MOS context
    //!
    void Destroy();

    //!
    //! \brief  Get the performance information
    //!
    struct PerfInfo GetPerfInfo() { return m_performanceInfo; }

    //!
    //! \brief  Get the performance information
    //!
    void SetPerfInfo(const struct PerfInfo &performanceInfo)
    {
        MOS_SecureMemcpy(&m_performanceInfo, sizeof(struct PerfInfo), &performanceInfo, sizeof(struct PerfInfo));
    }

    //!
    //! \brief  Return whether we need 64bit relocation
    //!
    bool Is64BitRelocUsed() { return m_use64BitRelocs; }

    //!
    //! \brief  Return whether the KMD support the 2nd VCS
    //!
    bool IsKmdWithVcs2() { return m_kmdHasVCS2; }

    //!
    //! \brief  Return the semaphore ID we use to protect the IPC creation process
    //! \return sem id
    //!
    int32_t GetSemId() { return m_semId; }

    //!
    //! \brief  Return the shm ID for the IPC
    //! \return shm id
    int32_t GetShmId() { return m_shmId; }

    //!
    //! \brief  Return the shm object for the IPC
    //! \return shm id
    //!
    void *GetShmPtr() { return m_shm; }

    //!
    //! \brief  Return the function ptr for memory decompression function
    //!
    void *GetpfnMemoryDecompaddr() { return (void *)m_memoryDecompress; }

    MOS_LINUX_CONTEXT *GetDrmContext() { return m_intelContext; }

    GPU_CONTEXT_HANDLE GetGpuContextHandle(MOS_GPU_CONTEXT GpuContext)
    {
        return m_GpuContextHandle[GpuContext];
    }

    void SetGpuContextHandle(MOS_GPU_CONTEXT GpuContext, GPU_CONTEXT_HANDLE gpuContextHandle)
    {
        m_GpuContextHandle[GpuContext] = gpuContextHandle;
    }

    GPU_CONTEXT_HANDLE GetGpuContextHandleByIndex(uint32_t index)
    {
        return (index < MOS_GPU_CONTEXT_MAX) ? m_GpuContextHandle[index] : MOS_GPU_CONTEXT_INVALID_HANDLE;
    }

    void SetGpuContextHandleByIndex(uint32_t index, GPU_CONTEXT_HANDLE gpuContextHandle)
    {
        if (index < MOS_GPU_CONTEXT_MAX)
        {
            m_GpuContextHandle[index] = gpuContextHandle;
        }
    }

    GpuContextMgr *GetGpuContextMgr() { return m_gpuContextMgr; }

    CmdBufMgr* GetCmdBufMgr(){return m_cmdBufMgr;}

    GMM_CLIENT_CONTEXT*  GetGmmClientContext() { return m_pGmmClientContext; };

    AuxTableMgr* GetAuxTableMgr() { return m_auxTableMgr; }

    bool UseSwSwizzling() { return m_useSwSwizzling; }
    bool GetTileYFlag() { return m_tileYFlag; }

#ifndef ANDROID

    //!
    //! \brief  Set slice count to shared memory and KMD
    //! \param  [in,out] pSliceCount
    //!         Pointer to the slice count. Input the slice count for current
    //!         context, output the ruling slice count shared by all contexts.
    //!
    void SetSliceCount(uint32_t *pSliceCount);

#endif

private:
#ifndef ANDROID
    //!
    //! \brief  connect and create share memory for driver secure IPC
    //! \param  [in] key
    //!         used to generate key_value for share memory
    //! \param  [in] key
    //!         share memory size
    //! \param  [out] pShmid
    //!         ptr to int value for share memory id
    //! \param  [out] ppShm
    //!         ptr to ptr for share memory
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS ConnectCreateShm(long key, uint32_t size, int32_t * pShmid, void* *ppShm);

    //!
    //! \brief  destory the share memory
    //! \param  [in] pShmid
    //!         ptr to int value for share memory id
    //! \param  [in] ppShm
    //!         ptr to ptr for share memory
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS DetachDestroyShm(int32_t shmid, void* pShm);

    //!
    //! \brief  connect and create semaphore for driver secure IPC
    //! \param  [in] key
    //!         used to generate key_value for share memory
    //! \param  [out] pSemid
    //!         ptr to sem id created
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS ConnectCreateSemaphore(long key, int32_t *pSemid);

    //!
    //! \brief  create driver secure IPC
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS CreateIPC();

    //!
    //! \brief  unlock the semaphore used in driver IPC
    //! \param  [in] semid
    //!         semaphore id to be unlocked
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS UnLockSemaphore(int32_t semid);

    //!
    //! \brief  lock the semaphore used in driver IPC
    //! \param  [in] semid
    //!         semaphore id to be locked
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS LockSemaphore(int32_t semid);

    //!
    //! \brief  destroy the IPC instance
    //!
    void DestroyIPC();

    //!
    //! \brief  attach to the share memory instance
    //! \param   shmid
    //!          [in] share memory id to be attached
    //! \return  share memory attached
    //!
    short ShmAttachedNumber(unsigned int shmid);

    //!
    //! \brief  destroy the semaphore
    //! \param  shmid
    //!         [in] Semaphore id to be destoried
    //!
    MOS_STATUS DestroySemaphore(unsigned int semid);

    //!
    //! \brief  create driver secure IPC for SSEU setting
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS CreateSSEUIPC();

    //!
    //! \brief  destroy the SSEU IPC instance
    //!
    void DestroySSEUIPC();
#endif // #ifndef ANDROID

    //!
    //! \brief  Performance specific switch for debug purpose
    //!
    struct PerfInfo     m_performanceInfo = {};

    //!
    //! \brief  Performance specific information for debug purpose
    //!
    PERF_DATA           m_perfData = {};

    //!
    //! \brief  switch for 64bit KMD relocation
    //!
    bool                m_use64BitRelocs = false;

    //!
    //! \brief  tiling/untiling with CPU
    //!
    bool                m_useSwSwizzling = false;

    //!
    //! \brief Sku tile Y flag
    //!
    bool                m_tileYFlag = true;

    //!
    //! \brief  flag to mark the existance of the second VDBox
    //!
    bool                m_kmdHasVCS2 = false;
    //!
    //! \brief  Semophore ID for secure IPC
    //!
    int32_t            m_semId = 0;
    //!
    //! \brief  Share memory ID for secure IPC
    //!
    int32_t            m_shmId = 0;
    //!
    //! \brief  Share memory ptr for secure IPC
    //!
    void*               m_shm = nullptr;
    //!
    //! \brief  Support slice count set in KMD
    //!
    bool               m_sliceCountSetSupported = 0;
    //!
    //! \brief  Enable/Disable dynamic slice shutdown and static slice config
    //!         -1    Use timer-based dynamic slice shutdown
    //!         0   [default] Use default slices count
    //!         >0  Static slice shutdown, N for N slices
    //!
    int                m_enableDymanicSliceShutdown = 0;
    //!
    //! \brief  sseu for current context
    //!
    uint64_t            m_sseu = 0;
    //!
    //! \brief  Semophore ID for ruling SSEU configration
    //!
    int32_t            m_sseuSemId = 0;
    //!
    //! \brief  Share memory ID for ruling SSEU configration
    //!
    int32_t            m_sseuShmId = 0;
    //!
    //! \brief  Share memory ptr to the ruling SSEU configration
    //!
    void*               m_sseuShm = nullptr;
    //!
    //! \brief  Hybrid Decoder Multi-Threading Enable Flag
    //!
    bool                m_hybridDecMultiThreadEnabled = false;
    //!
    //! \brief  Flag to indicate if hybrid decoder is running
    //!
    bool                m_hybridDecoderRunningFlag = false;

    //!
    //! \brief  the function ptr for memory decompression function
    //!
    void (* m_memoryDecompress)(
        PMOS_CONTEXT                pOsContext,
        PMOS_RESOURCE               pOsResource) = nullptr;

    //!
    //! \brief  ptr to ptr of memory decompression state
    //!
    void*               *m_mediaMemDecompState = nullptr;

    //!
    //! \brief  ptr to mos context(kept for memory decompression function, to be cleaned up)
    //!
    PMOS_CONTEXT        m_mosContext = nullptr;

    //!
    //! \brief  the function ptr for memory decompression function
    //!
    uint32_t            *m_transcryptedKernels = nullptr;

    //!
    //! \brief  Size in bytes of the cached version of transcrypted and authenticated kernels
    //!
    uint32_t            m_transcryptedKernelsSize = 0;

    //!
    //! \brief  ptr to DRM bufmgr
    //!
    MOS_BUFMGR          *m_bufmgr       = nullptr;

    //!
    //! \brief  ptr to intel context
    //!
    MOS_LINUX_CONTEXT   *m_intelContext = nullptr;

    //!
    //! \brief  drm device fd
    //!
    uint32_t            m_fd             = 0;

    //!
    //!UMD specific ClientContext object in GMM
    //!
    GMM_CLIENT_CONTEXT   *m_pGmmClientContext = nullptr;

    AuxTableMgr          *m_auxTableMgr = nullptr;

    GPU_CONTEXT_HANDLE  m_GpuContextHandle[MOS_GPU_CONTEXT_MAX]; // Index to GPU Context (GpuContextHandles)

    GpuContextMgr      *m_gpuContextMgr = nullptr;
    CmdBufMgr          *m_cmdBufMgr = nullptr;
};
#endif // #ifndef __MOS_CONTEXT_SPECIFIC_H__
