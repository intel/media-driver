/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     mos_context_specific_next.h
//! \brief    Container for Linux specific parameters shared across different GPU contexts of the same device instance
//!

#ifndef __MOS_CONTEXT_SPECIFIC_NEXT_H__
#define __MOS_CONTEXT_SPECIFIC_NEXT_H__

#include "mos_context_next.h"
#include "mos_auxtable_mgr.h"

class GraphicsResourceSpecificNext;
class CmdBufMgrNext;
class GpuContextMgrNext;

class OsContextSpecificNext : public OsContextNext
{
    friend class GraphicsResourceSpecificNext;

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
    OsContextSpecificNext();

    //!
    //! \brief  Destructor
    //!
    ~OsContextSpecificNext();

    //!
    //! \brief  Initialize the MOS Context
    //! \param  [in] pOsDriverContext
    //!         ptr to DDI_DEVICE_CONTEXT created inside DDI
    //! \return MOS_Success in pass case, MOS error status in fail cases
    //!
    MOS_STATUS Init(DDI_DEVICE_CONTEXT osDriverContext);

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
        MosUtilities::MosSecureMemcpy(&m_performanceInfo, sizeof(struct PerfInfo), &performanceInfo, sizeof(struct PerfInfo));
    }

    //!
    //! \brief  Return whether we need 64bit relocation
    //!
    bool Is64BitRelocUsed() { return m_use64BitRelocs; }

    AuxTableMgr* GetAuxTableMgr() { return m_auxTableMgr; }

    bool UseSwSwizzling() { return m_useSwSwizzling; }
    bool GetTileYFlag() { return m_tileYFlag; }

    MOS_BUFMGR *GetBufMgr()
    {
        return m_bufmgr;
    }

    int32_t GetFd()
    {
        return m_fd;
    }

    int GetDeviceType()
    {
        return m_deviceType;
    }

private:
    //!
    //! \brief  Performance specific switch for debug purpose
    //!
    struct PerfInfo     m_performanceInfo = {};

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
    bool                m_tileYFlag     = true;

    //!
    //! \brief  ptr to DRM bufmgr
    //!
    MOS_BUFMGR          *m_bufmgr       = nullptr;

    //!
    //! \brief  drm device fd
    //!
    int32_t             m_fd            = -1;

    //!
    //! \brief  device type
    //!
    int                 m_deviceType   = DEVICE_TYPE_COUNT;
    AuxTableMgr         *m_auxTableMgr = nullptr;
    PERF_DATA           *m_perfData =   nullptr;
MEDIA_CLASS_DEFINE_END(OsContextSpecificNext)
};
#endif // #ifndef __MOS_CONTEXT_SPECIFIC_NEXT_H__
