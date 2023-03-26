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
//! \file     mos_oca_util_debug.h
//! \brief    Container class for OCA buffer manager wrapper
//! \details  Container class for OCA buffer manager wrapper
//!

#ifndef __MOS_OCA_UTIL_DEBUG_H__
#define __MOS_OCA_UTIL_DEBUG_H__
#if !EMUL
#include "mos_oca_rtlog_mgr_defs.h"
#include "oca_rtlog_section_mgr.h"

#define OCA_MT_ERR(id, p1, v1, cTpye, osStreamState)                                               \
    {                                                                                              \
        bool isErr = true;                                                                         \
        MT_PARAM param[] = {{p1, v1}};                                                             \
        OcaRtLogSectionMgr::InsertRTLog(cTpye, isErr, id, 1, param);                      \
        MT_ERR1(id, p1, v1);                                                                       \
    }

#define OCA_MT_LOG(id, p1, v1, cTpye, osStreamState)                                               \
    {                                                                                              \
        bool isErr = false;                                                                        \
        MT_PARAM param[] = {{p1, v1}};                                                             \
        OcaRtLogSectionMgr::InsertRTLog(cTpye, isErr, id, 1, param);                      \
        MT_LOG1(id, MT_NORMAL, p1,v1);                                                             \
    }

void OcaOnMosCriticalMessage(const PCCHAR functionName, int32_t lineNum);
#else
#define OCA_MT_ERR(id, p1, v1, cTpye, osStreamState)
#define OCA_MT_LOG(id, p1, v1, cTpye, osStreamState)
#define OcaOnMosCriticalMessage(functionName, lineNum)
#endif
#endif  // __MOS_OCA_UTIL_DEBUG_H__