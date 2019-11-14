/*
* Copyright (c) 2019, Intel Corporation
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
//! \file    mos_gpucontextmgr_next.cpp
//! \brief   Container class for the basic gpu context manager
//!

#include "mos_gpucontextmgr_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_graphicsresource_specific_next.h"

GpuContextMgrNext::GpuContextMgrNext(GT_SYSTEM_INFO *gtSystemInfo, OsContextNext *osContext)
{
    MOS_OS_FUNCTION_ENTER;
    m_initialized = false;

    m_gpuContextArrayMutex = MosUtilities::MosCreateMutex();
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(m_gpuContextArrayMutex);

    MosUtilities::MosLockMutex(m_gpuContextArrayMutex);
    m_gpuContextArray.clear();
    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);

    if (gtSystemInfo)
    {
        MosUtilities::MosSecureMemcpy(&m_gtSystemInfo, sizeof(GT_SYSTEM_INFO), gtSystemInfo, sizeof(GT_SYSTEM_INFO));
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Input GTSystemInfo cannot be nullptr");
        return;
    }

    if (osContext)
    {
        m_osContext = osContext;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Input osContext cannot be nullptr");
        return;
    }

    m_initialized = true;
}

GpuContextMgrNext::~GpuContextMgrNext()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_gpuContextArrayMutex)
    {
        MosUtilities::MosDestroyMutex(m_gpuContextArrayMutex);
        m_gpuContextArrayMutex = nullptr;
    }
}

GpuContextMgrNext *GpuContextMgrNext::GetObject(
    GT_SYSTEM_INFO *gtSystemInfo,
    OsContextNext     *osContext)
{
    MOS_OS_FUNCTION_ENTER;
    if (gtSystemInfo == nullptr || osContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameters!");
        return nullptr;
    }
    return MOS_New(GpuContextMgrNext, gtSystemInfo, osContext);
}

void GpuContextMgrNext::CleanUp()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_initialized)
    {
        DestroyAllGpuContexts();

        MosUtilities::MosLockMutex(m_gpuContextArrayMutex);
        m_gpuContextArray.clear();
        MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);

        m_initialized = false;
    }

    return;
}

bool GpuContextMgrNext::ContextReuseNeeded()
{
    MOS_OS_FUNCTION_ENTER;

    // to be added after scalable design is nailed down
    return false;
}

GpuContextNext *GpuContextMgrNext::SelectContextToReuse()
{
    MOS_OS_FUNCTION_ENTER;

    // to be added after scalable design is nailed down
    return nullptr;
}

GpuContextNext *GpuContextMgrNext::CreateGpuContext(
    const MOS_GPU_NODE gpuNode,
    CmdBufMgrNext *    cmdBufMgr)
{
    MOS_OS_FUNCTION_ENTER;

    if (cmdBufMgr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("nullptr of cmdbufmgr.");
        return nullptr;
    }

    GpuContextNext *reusedContext = nullptr;
    if (ContextReuseNeeded())
    {
        reusedContext = SelectContextToReuse();
    }

    GpuContextNext *gpuContext = GpuContextNext::Create(gpuNode, cmdBufMgr, reusedContext);
    if (gpuContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("nullptr returned by GpuContext::Create.");
        return nullptr;
    }

    MosUtilities::MosLockMutex(m_gpuContextArrayMutex);

    GPU_CONTEXT_HANDLE gpuContextHandle = 0;

    if (m_noCycledGpuCxtMgmt)
    {
        // new created context at the end of m_gpuContextArray.
        gpuContextHandle = m_gpuContextArray.size() ? m_gpuContextArray.size() : 0;
    }
    else
    {
        // Directly replace nullptr with new created context in m_gpuContextArray.
        GpuContextNext *curGpuContext = nullptr;
        int         index         = 0;
        for (auto &curGpuContext : m_gpuContextArray)
        {
            if (curGpuContext == nullptr)
            {
                break;
            }
            index++;
        }
        gpuContextHandle = m_gpuContextArray.size() ? index : 0;
    }
    gpuContext->SetGpuContextHandle(gpuContextHandle);

    if (gpuContextHandle == m_gpuContextArray.size())
    {
        m_gpuContextArray.push_back(gpuContext);
    }
    else
    {
        m_gpuContextArray[gpuContextHandle] = gpuContext;
    }
    m_gpuContextCount++;

    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);

    return gpuContext;
}

GpuContextNext *GpuContextMgrNext::GetGpuContext(GPU_CONTEXT_HANDLE gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    if (gpuContextHandle == MOS_GPU_CONTEXT_INVALID_HANDLE)
    {
        MOS_OS_ASSERTMESSAGE("Input gpucontext handle cannot be MOS_GPU_CONTEXT_INVALID_HANDLE!");
        return nullptr;
    }

    if (!m_gpuContextArray.empty() && gpuContextHandle < m_gpuContextArray.size())
    {
        return m_gpuContextArray.at(gpuContextHandle);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("GPU context array is empty or got invalid index, something must be wrong!");
        return nullptr;
    }
}

void GpuContextMgrNext::DestroyGpuContext(GpuContextNext *gpuContext)
{
    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(gpuContext);

    GpuContextNext *curGpuContext = nullptr;
    bool        found         = false;

    MosUtilities::MosLockMutex(m_gpuContextArrayMutex);
    for (auto &curGpuContext : m_gpuContextArray)
    {
        if (curGpuContext == gpuContext)
        {
            found = true;
            // to keep original order, here should not erase gpucontext, replace with nullptr in array.
            MOS_Delete(curGpuContext);  // delete gpu context.
            m_gpuContextCount--;
            break;
        }
    }

    if (m_gpuContextCount == 0 && !m_noCycledGpuCxtMgmt)
    {
        m_gpuContextArray.clear();  // clear whole array
    }

    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);

    if (!found)
    {
        MOS_OS_ASSERTMESSAGE("cannot find specified gpuContext in the gpucontext pool, something must be wrong");
    }
}

void GpuContextMgrNext::DestroyAllGpuContexts()
{
    MOS_OS_FUNCTION_ENTER;

    GpuContextNext *curGpuContext = nullptr;

    MosUtilities::MosLockMutex(m_gpuContextArrayMutex);

    // delete each instance in m_gpuContextArray
    for (auto &curGpuContext : m_gpuContextArray)
    {
        MOS_Delete(curGpuContext);
    }

    m_gpuContextArray.clear();  // clear whole array

    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);
}
