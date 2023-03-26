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
//! \file     mos_oca_util_debug.cpp
//! \brief    OCA Util Debug
//!
#if !EMUL
#include "mos_oca_util_debug.h"
#include "mos_os_trace_event.h"

void OcaOnMosCriticalMessage(const PCCHAR functionName, int32_t lineNum)
{
    bool isErr = true;
    uint32_t subStrLen = 0;

    PCCHAR subStr = functionName;

    while (subStr[subStrLen])
    {
        ++subStrLen;
        if (8 == subStrLen)
        {
            MT_PARAM param[] = {{MT_FUNC_NAME, *(int64_t*)subStr}};
            OcaRtLogSectionMgr::InsertRTLog(MOS_OCA_RTLOG_COMPONENT_COMMON, isErr, MT_ERR_CRITICAL_MESSAGE, 1, param);
            subStr += subStrLen;
            subStrLen = 0;
        }
    }

    if (subStrLen > 0)
    {
        int64_t name = 0;
        for (uint32_t i = 0; i < subStrLen; ++i)
        {
            name |= ((uint64_t)subStr[i] << (i * 8));
        }
        MT_PARAM param[] = {{MT_FUNC_NAME, name}};
        OcaRtLogSectionMgr::InsertRTLog(MOS_OCA_RTLOG_COMPONENT_COMMON, isErr, MT_ERR_CRITICAL_MESSAGE, 1, param);
    }

    MT_PARAM param[] = {{MT_FUNC_LINE, lineNum}};
    OcaRtLogSectionMgr::InsertRTLog(MOS_OCA_RTLOG_COMPONENT_COMMON, isErr, MT_ERR_CRITICAL_MESSAGE, 1, param);
}

#endif