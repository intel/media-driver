/*
* Copyright (c) 2023, Intel Corporation
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
//! \file        oca_rtlog_section_mgr.h 
//! \brief       OCA rtlog section manager
//!

#ifndef __OCA_RTLOG_SECTION_MGR_H__
#define __OCA_RTLOG_SECTION_MGR_H__
#include <atomic>
#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_oca_rtlog_mgr_defs.h"

class OcaRtLogSectionMgr
{
public:
    OcaRtLogSectionMgr();
    virtual ~OcaRtLogSectionMgr();
    static uint8_t *GetMemAddress();

    static MOS_STATUS InsertRTLog(
        MOS_OCA_RTLOG_COMPONENT_TPYE componentType,
        bool                         isErr,
        int32_t                      id,
        uint32_t                     paramCount,
        const void                   *param);

protected:
    uint32_t                     m_HeapSize      = 0;        //!< Ring size in bytes.
    void                        *m_LockedHeap    = nullptr;  //!< System (logical) address for state heap.
    std::atomic<bool>            m_IsInitialized {false};    //!< ture if current heap object has been initialized.
    uint32_t                     m_Offset        = 0;
    uint32_t                     m_HeapHandle    = 0;
    uint32_t                     m_EntryCount    = 0;

    OcaRtLogSectionMgr &operator=(OcaRtLogSectionMgr &)
    {
        return *this;
    }

private:
    static OcaRtLogSectionMgr  s_rtLogSectionMgr[MOS_OCA_RTLOG_COMPONENT_MAX];
    static uint8_t             s_localSysMem[MAX_OCA_RT_POOL_SIZE];

    static uint8_t *InitSectionMgrAndGetAddress();

    void       Init(uint8_t *logSysMem, uint32_t size, uint32_t componentSize, uint32_t offset);
    uint32_t   AllocHeapHandle();
    MOS_STATUS InsertData(MOS_OCA_RTLOG_HEADER header, const void *param);
    MOS_STATUS InsertUid(MOS_OCA_RTLOG_SECTION_HEADER sectionHeader);
    bool       IsInitialized() { return m_IsInitialized; }
    uint64_t   GetHeapSize() { return m_HeapSize; }
    void       *GetLockHeap() { return m_LockedHeap; }
MEDIA_CLASS_DEFINE_END(OcaRtLogSectionMgr)
};
#endif //__OCA_RTLOG_SECTION_MGR_H__