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
//! \file        mos_oca_buffer_mgr_base.h 
//! \brief 
//!
//!
//! \file     mos_oca_buffer_mgr.h
//! \brief    Container class for OCA buffer manager wrapper
//! \details  Container class for OCA buffer manager wrapper
//!

#ifndef __MOS_OCA_RTLOG_MGR_BASE_H__
#define __MOS_OCA_RTLOG_MGR_BASE_H__

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

class MosOcaRTLogMgrBase
{
public:
    bool IsMgrInitialized() { return m_isMgrInitialized; };

    uint32_t GetRtlogHeapSize() { return m_heapSize; };

    static bool IsOcaRTLogEnabled() { return s_enableOcaRTLog; };

protected:
    int32_t GetGlobleIndex();

    OsContextNext                       *m_osContext = nullptr;
    int32_t                              m_globleIndex = -1;
    bool                                 m_isMgrInitialized = false;
    uint32_t                             m_heapSize = MAX_OCA_RT_SIZE;
    uint8_t                             *m_heapAddr = nullptr;

    static bool                          s_enableOcaRTLog;
    static MosMutex                      s_ocaMutex;
    static bool                          s_isOcaRtlogMgrDestoryed;

MEDIA_CLASS_DEFINE_END(MosOcaRTLogMgrBase)
};

#endif //__MOS_OCA_RTLOG_MGR_BASE_H__