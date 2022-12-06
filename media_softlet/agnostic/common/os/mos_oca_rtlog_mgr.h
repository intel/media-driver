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
#include "mos_utilities.h"
#include <vector>

class OsContextNext;
class OsContextSpecificNext;

struct MOS_OCA_RTLOG_RES_AND_INTERFACE
{
    PMOS_RESOURCE  ocaRTLogResource = nullptr;
    PMOS_INTERFACE osInterface      = nullptr;
};

class MosOcaRtLogSectionMgr
{
public:
    MosOcaRtLogSectionMgr();
    void Init(uint8_t *logSysMem, uint32_t size, uint32_t componentSize, uint32_t offset);
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
    bool                         m_IsInitialized = false;    //!< ture if current heap object has been initialized.
    uint32_t                     m_Offset        = 0;
    int32_t                      m_HeapHandle    = 0;
    int32_t                      m_EntryCount    = 0;

    MosOcaRtLogSectionMgr &operator=(MosOcaRtLogSectionMgr &)
    {
        return *this;
    }

MEDIA_CLASS_DEFINE_END(MosOcaRtLogSectionMgr)
};

class MosOcaRTLogMgr
{
public:
    MOS_STATUS InsertRTLog(MOS_OCA_RTLOG_COMPONENT_TPYE componentType, bool isErr, int32_t id, uint32_t paramCount, const void *param);

    bool IsMgrInitialized() { return m_isMgrInitialized; };

    uint32_t GetRtlogHeapSize() { return m_heapSize; };

    static void RegisterContext(OsContextNext *osDriverContext, MOS_CONTEXT *osContext);

    static void UnRegisterContext(OsContextNext *osDriverContext);

    static bool IsOcaRTLogEnabled() { return m_enableOcaRTLog; };

    MosOcaRTLogMgr();
    MosOcaRTLogMgr(MosOcaRTLogMgr &);
    virtual ~MosOcaRTLogMgr();
    MosOcaRTLogMgr &operator=(MosOcaRTLogMgr &);
    static MosOcaRTLogMgr &GetInstance();

protected:
    virtual MOS_STATUS RegisterCtx(OsContextNext *osDriverContext, MOS_CONTEXT *osContext);
    virtual MOS_STATUS UnRegisterCtx(OsContextNext *osDriverContext);
    virtual MOS_STATUS RegisterRes(OsContextNext *osDriverContext, MOS_OCA_RTLOG_RES_AND_INTERFACE *resInterface, MOS_CONTEXT *osContext);
    virtual void       UnregisterRes(OsContextNext *osDriverContext);
    virtual MOS_STATUS MapGfxVa(PMOS_RESOURCE ocaRTLogResource, OsContextNext *osDriverContext);
    int32_t GetGlobleIndex();

    OsContextNext                       *m_osContext = nullptr;
    MosOcaRtLogSectionMgr                m_rtLogSectionMgr[MOS_OCA_RTLOG_COMPONENT_MAX] = {};
    int32_t                              m_globleIndex = -1;
    bool                                 m_isMgrInitialized = false;
    std::map<OsContextNext *, MOS_OCA_RTLOG_RES_AND_INTERFACE> m_resMap;
    uint32_t                             m_heapSize = MAX_OCA_RT_SIZE;
    uint8_t                             *m_heapAddr = nullptr;

    static bool                          m_enableOcaRTLog;
    static uint8_t                       s_localSysMem[MAX_OCA_RT_POOL_SIZE];
    static MosMutex                      s_ocaMutex;

MEDIA_CLASS_DEFINE_END(MosOcaRTLogMgr)
};

#endif //__MOS_OCA_RTLOG_MGR_H__