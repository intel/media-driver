/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mos_context.cpp
//! \brief    Container for parameters shared across different GPU contexts of the same device instance
//!

#include "mos_context.h"
#include "mos_context_specific.h"
#include "mos_util_debug.h"
#include <new>

class OsContext* OsContext::GetOsContextObject()
{
    MOS_OS_FUNCTION_ENTER;
    class OsContext* osContextPtr = MOS_New(OsContextSpecific);

    return osContextPtr;
}

void OsContext::CleanUp()
{
    MOS_OS_FUNCTION_ENTER;

    Destroy();
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
MOS_STATUS OsContext::CommandBufferDumpInit(PMOS_CONTEXT mosCtx)
{
    MOS_OS_FUNCTION_ENTER;
    char                                sFileName[MOS_MAX_HLT_FILENAME_LEN];
    MOS_STATUS                          eStatus = MOS_STATUS_UNKNOWN;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;

    // Check if command buffer dump was enabled in user feature.
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE_ID,
        &UserFeatureData,
        mosCtx);
    m_dumpCommandBuffer           = (UserFeatureData.i32Data != 0);
    m_dumpCommandBufferToFile     = ((UserFeatureData.i32Data & 1) != 0);
    m_dumpCommandBufferAsMessages = ((UserFeatureData.i32Data & 2) != 0);

    return MOS_STATUS_SUCCESS;
}
#endif // #if MOS_COMMAND_BUFFER_DUMP_SUPPORTED