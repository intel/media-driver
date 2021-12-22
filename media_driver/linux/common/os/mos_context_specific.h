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
    //! \brief slice count value timeout critera in milliseconds
    //!
    constexpr static uint64_t m_sliceCountTimeoutMS = 1000;

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

    //!
    //! \brief  Get the context priority from KMD
    //! \param  [in, out] pPriority
    //!         Pointer to the priority of current gpu context.
    //!
    void GetGpuPriority(int32_t *pPriority);

    //!
    //! \brief  Get the context priority from KMD
    //! \param  [in] priority
    //!         the priority set to  current gpu context.
    //!
    void SetGpuPriority(int32_t priority); 

private:
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
    //! \brief  Enable/Disable dynamic slice shutdown and static slice config
    //!         -1    Use timer-based dynamic slice shutdown
    //!         0   [default] Use default slices count
    //!         >0  Static slice shutdown, N for N slices
    //!
    int                m_enableDymanicSliceShutdown = 0;

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
    //! \brief  the function ptr for surface copy function
    //!
    void  (* m_mediaMemCopy )(
        PMOS_CONTEXT       pOsContext,
        PMOS_RESOURCE      pInputResource,
        PMOS_RESOURCE      pOutputResource,
        bool               bOutputCompressed) = nullptr;

    //!
    //! \brief  the function ptr for Media Memory 2D copy function
    //!
    void (* m_mediaMemCopy2D)(
        PMOS_CONTEXT       pOsContext,
        PMOS_RESOURCE      pInputResource,
        PMOS_RESOURCE      pOutputResource,
        uint32_t           copyWidth,
        uint32_t           copyHeight,
        uint32_t           copyInputOffset,
        uint32_t           copyOutputOffset,
        uint32_t           bpp,
        bool               bOutputCompressed) = nullptr;

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
    bool                m_apoMosEnabled = false;
};
#endif // #ifndef __MOS_CONTEXT_SPECIFIC_H__
