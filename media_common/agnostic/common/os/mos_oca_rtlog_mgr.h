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
//! \file        mos_oca_buffer_mgr.h 
//! \brief 
//!
//!
//! \file     mos_oca_buffer_mgr.h
//! \brief    Container class for OCA buffer manager wrapper
//! \details  Container class for OCA buffer manager wrapper
//!

#ifndef __MOS_OCA_RTLOG_MGR_H__
#define __MOS_OCA_RTLOG_MGR_H__

#include "mos_oca_rtlog_mgr_defs.h"
#include "mos_os_specific.h"
#include "mos_interface.h"
#include "mos_util_debug.h"

class MosOcaRtLogSectionMgr
{
public:
    MosOcaRtLogSectionMgr(MOS_OCA_RTLOG_HEAP ocaRtHeap, uint32_t size, uint32_t offset);
    virtual ~MosOcaRtLogSectionMgr();
    virtual MOS_STATUS InsertData(MOS_OCA_RTLOG_HEADER header, const void *param);
    virtual MOS_STATUS InsertUid(MOS_OCA_RTLOG_SECTION_HEADER sectionHeader);
    virtual bool       IsInitialized() { return m_IsInitialized; };
    virtual uint64_t   GetHeapSize() { return m_HeapSize; }
    virtual void      *GetLockHeap() { return m_LockedHeap; }
    virtual int32_t    AllocHeapHandle();

protected:
    uint32_t                     m_HeapSize      = 0;        //!< Ring size in bytes.
    void                        *m_LockedHeap    = nullptr;  //!< System (logical) address for state heap.
    uint64_t                     m_HeapGpuVa     = 0;        //!< Gpu virtual address to current heap.
    bool                         m_IsInitialized = false;    //!< ture if current heap object has been initialized.
    uint32_t                     m_Offset        = 0;
    int32_t                      m_HeapHandle    = 0;
    int32_t                      m_EntryCount    = 0;

private:
    MosOcaRtLogSectionMgr &operator=(MosOcaRtLogSectionMgr &)
    {
        return *this;
    }
};

class MosOcaRTLogMgr
{
public:
    MOS_STATUS InsertRTLog(MOS_OCA_RTLOG_COMPONENT_TPYE componentType, bool isErr, int32_t id, uint8_t paramCount, const void *param);

    void Initialize(void *osDriverContext, void *osMemoryMgr);

    void Uninitialize();

    MOS_RESOURCE GetOcaRTlogResource() { return m_ocaRTLogResource; }

    int32_t GetGlobleIndex();

    static void InitMgr(void *osDriverContext, void *osMemoryMgr);

    static void UninitMgr();

    MOS_STATUS Clean();

    MOS_STATUS Reset();

    PMOS_MUTEX GetMutex()
    {
        return m_ocaMutex;
    }

    static MosOcaRTLogMgr &GetInstance();
    MosOcaRTLogMgr();
    MosOcaRTLogMgr(MosOcaRTLogMgr &);
    virtual ~MosOcaRTLogMgr();
    MosOcaRTLogMgr &operator=(MosOcaRTLogMgr &);

protected:
    MOS_INTERFACE m_OsInterface = {};
    MOS_RESOURCE m_ocaRTLogResource = {};

public:
    PMOS_MUTEX                           m_ocaMutex    = nullptr;
    MOS_OCA_RTLOG_HEAP                   m_ocaRtHeap   = {};
    std::vector<MosOcaRtLogSectionMgr *> m_RTLogSectionMgr;

    static int32_t                      m_globleIndex;
    static bool                         m_IsMgrInitialized;
};

#endif //__MOS_OCA_RTLOG_MGR_H__