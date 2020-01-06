/*
* Copyright (c) 2007-2019, Intel Corporation
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
//! \file      cm_queue_rt_os.cpp
//! \brief     This file contains CmQueueRT implementations specific to Linux system.
//!

#include "cm_queue_rt.h"

#include "cm_mem.h"

namespace CMRT_UMD
{
MOS_STATUS CmQueueRT::CreateSyncBuffer(CM_HAL_STATE *halState)
{
    if (INVALID_SYNC_BUFFER_HANDLE != m_syncBufferHandle)
    {
        return MOS_STATUS_SUCCESS;  // Buffer has been created.
    }
    CM_HAL_BUFFER_PARAM buffer_param;
    CmSafeMemSet(&buffer_param, 0, sizeof(buffer_param));
    buffer_param.size = halState->cmHalInterface->GetTimeStampResourceSize();
    buffer_param.type = CM_BUFFER_N;
    buffer_param.isAllocatedbyCmrtUmd = true;
    MOS_STATUS result = halState->pfnAllocateBuffer(halState, &buffer_param);
    if (MOS_STATUS_SUCCESS == result)
    {
        m_syncBufferHandle = buffer_param.handle;
    }
    return result;
}

MOS_STATUS CmQueueRT::SelectSyncBuffer(CM_HAL_STATE *halState)
{
    return halState->pfnSelectSyncBuffer(halState, m_syncBufferHandle);
}

MOS_STATUS CmQueueRT::ReleaseSyncBuffer(CM_HAL_STATE *halState)
{
    if (INVALID_SYNC_BUFFER_HANDLE == m_syncBufferHandle)
    {
        return MOS_STATUS_SUCCESS;
    }
    CM_CHK_MOSSTATUS_RETURN(halState->pfnFreeBuffer(halState, m_syncBufferHandle));
    m_syncBufferHandle = INVALID_SYNC_BUFFER_HANDLE;
    return halState->pfnSelectSyncBuffer(halState, INVALID_SYNC_BUFFER_HANDLE);
}
}// namespace
