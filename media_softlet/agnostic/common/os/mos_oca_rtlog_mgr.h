/*
* Copyright (c) 2022-2023, Intel Corporation
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

#include "mos_oca_rtlog_mgr_base.h"

class MosOcaRTLogMgr : public MosOcaRTLogMgrBase
{
public:
    static void RegisterContext(OsContextNext *osDriverContext, MOS_CONTEXT *osContext);
    static void UnRegisterContext(OsContextNext *osDriverContext);

    MosOcaRTLogMgr();
    MosOcaRTLogMgr(MosOcaRTLogMgr &);
    virtual ~MosOcaRTLogMgr();
    MosOcaRTLogMgr &operator=(MosOcaRTLogMgr &);
    static MosOcaRTLogMgr *GetInstance();

protected:
    virtual MOS_STATUS RegisterCtx(OsContextNext *osDriverContext, MOS_CONTEXT *osContext);
    virtual MOS_STATUS UnRegisterCtx(OsContextNext *osDriverContext);
    virtual MOS_STATUS RegisterRes(OsContextNext *osDriverContext, MOS_OCA_RTLOG_RES_AND_INTERFACE *resInterface, MOS_CONTEXT *osContext);
    virtual void       UnregisterRes(OsContextNext *osDriverContext);
    virtual MOS_STATUS MapGfxVa(PMOS_RESOURCE ocaRTLogResource, OsContextNext *osDriverContext);

    std::map<OsContextNext *, MOS_OCA_RTLOG_RES_AND_INTERFACE> m_resMap;

MEDIA_CLASS_DEFINE_END(MosOcaRTLogMgr)
};

#endif //__MOS_OCA_RTLOG_MGR_H__