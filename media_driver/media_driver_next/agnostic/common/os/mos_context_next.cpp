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
//! \file     mos_context_next.cpp
//! \brief    Container for parameters shared across different GPU contexts of the same device instance
//!

#include "mos_context_next.h"
#include "mos_context_specific_next.h"
#include "mediamemdecomp.h"
#include "mos_util_debug_next.h"
#include <new>

class OsContextNext* OsContextNext::GetOsContextObject()
{
    MOS_OS_FUNCTION_ENTER;
    class OsContextNext* osContextPtr = MOS_New(OsContextSpecificNext);

    return osContextPtr;
}

MOS_STATUS OsContextNext::Init(DDI_DEVICE_CONTEXT osDriverContext)
{
    //Read user feature key here for Per Utility Tool Enabling
    MOS_UNUSED(osDriverContext);
#if _RELEASE_INTERNAL
    if (!g_perfutility->bPerfUtilityKey)
    {
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            NULL,
            __MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE_ID,
            &UserFeatureData);
        g_perfutility->dwPerfUtilityIsEnabled = UserFeatureData.i32Data;

        char                        sFilePath[MOS_MAX_PERF_FILENAME_LEN + 1] = "";
        MOS_USER_FEATURE_VALUE_DATA perfFilePath;
        MOS_STATUS                  eStatus_Perf = MOS_STATUS_SUCCESS;

        MOS_ZeroMemory(&perfFilePath, sizeof(perfFilePath));
        perfFilePath.StringData.pStringData = sFilePath;
        eStatus_Perf                        = MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY_ID,
            &perfFilePath);
        if (eStatus_Perf == MOS_STATUS_SUCCESS)
        {
            g_perfutility->setupFilePath(sFilePath);
        }
        else
        {
            g_perfutility->setupFilePath();
        }

        g_perfutility->bPerfUtilityKey = true;
    }
#endif
    return MOS_STATUS_SUCCESS;
}

void OsContextNext::CleanUp()
{
    MOS_OS_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    MOS_Delete(m_mediaMemDecompState);
#endif
    if (m_gpuContextMgr != nullptr)
    {
        m_gpuContextMgr->CleanUp();
        MOS_Delete(m_gpuContextMgr);
        m_gpuContextMgr = nullptr;
    }

    if (m_cmdBufMgr != nullptr)
    {
        m_cmdBufMgr->CleanUp();
        MOS_Delete(m_cmdBufMgr);
        m_cmdBufMgr = nullptr;
    }

    Destroy();
}