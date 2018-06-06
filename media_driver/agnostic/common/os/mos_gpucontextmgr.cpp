/*
* Copyright (c) 2018, Intel Corporation
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
//! \file    mos_gpucontextmgr.cpp
//! \brief   Container class for the basic gpu context manager
//!

#include "mos_gpucontextmgr.h"
#include "mos_gpucontext_specific.h"
#include "mos_graphicsresource_specific.h"

GpuContextMgr::GpuContextMgr(GT_SYSTEM_INFO *gtSystemInfo, OsContext *osContext)
{
    MOS_OS_FUNCTION_ENTER;

    m_gpuContextArrayMutex = MOS_CreateMutex();

    MOS_LockMutex(m_gpuContextArrayMutex);
    m_gpuContextArray.clear();
    MOS_UnlockMutex(m_gpuContextArrayMutex);

    if (gtSystemInfo)
    {
        MOS_SecureMemcpy(&m_gtSystemInfo, sizeof(GT_SYSTEM_INFO), gtSystemInfo, sizeof(GT_SYSTEM_INFO));
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Input GTSystemInfo cannot be nullptr");
    }

    if (osContext)
    {
        m_osContext = osContext;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Input osContext cannot be nullptr");
    }
}

GpuContextMgr::~GpuContextMgr()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_gpuContextArrayMutex)
    {
        MOS_DestroyMutex(m_gpuContextArrayMutex);
    }
}

GpuContextMgr* GpuContextMgr::GetObject(
    GT_SYSTEM_INFO *gtSystemInfo,
    OsContext      *osContext)
{
    MOS_OS_FUNCTION_ENTER;
    if (gtSystemInfo == nullptr || osContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameters!");
        return nullptr;
    }
    return MOS_New(GpuContextMgr,gtSystemInfo, osContext);
}

void GpuContextMgr::CleanUp()
{
    MOS_OS_FUNCTION_ENTER;

    DestroyAllGpuContexts();

    MOS_LockMutex(m_gpuContextArrayMutex);
    m_gpuContextArray.clear();
    MOS_UnlockMutex(m_gpuContextArrayMutex);

    return;
}

bool GpuContextMgr::ContextReuseNeeded()
{
    MOS_OS_FUNCTION_ENTER;

    // to be added after scalable design is nailed down
    return false;
}

GpuContext *GpuContextMgr::SelectContextToReuse()
{
    MOS_OS_FUNCTION_ENTER;

    // to be added after scalable design is nailed down
    return nullptr;
}

GpuContext *GpuContextMgr::CreateGpuContext(
    const MOS_GPU_NODE gpuNode,
    CmdBufMgr         *cmdBufMgr,
    MOS_GPU_CONTEXT    mosGpuCtx)
{
    MOS_OS_FUNCTION_ENTER;

    GpuContext *reusedContext = nullptr;
    if (ContextReuseNeeded())
    {
        reusedContext = SelectContextToReuse();
    }

    GpuContext *gpuContext = GpuContext::Create(gpuNode, mosGpuCtx, cmdBufMgr, reusedContext);
    if (gpuContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("nullptr returned by GpuContext::Create.");
        return nullptr;
    }

    MOS_LockMutex(m_gpuContextArrayMutex);

    // m_gpuContextArray always increases, directly use size before push back as search index.
    GPU_CONTEXT_HANDLE gpuContextHandle = m_gpuContextArray.size();
    gpuContext->SetGpuContextHandle(gpuContextHandle);
    m_gpuContextArray.push_back(gpuContext);
    m_gpuContextCount++;

    MOS_UnlockMutex(m_gpuContextArrayMutex);

    return gpuContext;
}

GpuContext *GpuContextMgr::GetGpuContext(GPU_CONTEXT_HANDLE gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    if (!m_gpuContextArray.empty() && gpuContextHandle <= m_gpuContextArray.size())
    {
        return m_gpuContextArray.at(gpuContextHandle);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("GPU context array is empty or got invalid index, something must be wrong!");
        return nullptr;
    }
}

void GpuContextMgr::DestroyGpuContext(GpuContext *gpuContext)
{
    MOS_OS_FUNCTION_ENTER;

    GpuContext*       curGpuContext = nullptr;
    bool              found         = false;

    MOS_LockMutex(m_gpuContextArrayMutex);
    for (auto& curGpuContext : m_gpuContextArray)
    {
        if (curGpuContext == gpuContext)
        {
            found = true;
            // to keep original order, here should not erase gpucontext, replace with nullptr in array.
            MOS_Delete(curGpuContext); // delete gpu context.
            m_gpuContextCount--;
            break;
        }
    }
    
    if(m_gpuContextCount == 0)
    {
       m_gpuContextArray.clear(); // clear whole array     
    }

    MOS_UnlockMutex(m_gpuContextArrayMutex);

    if (!found)
    {
        MOS_OS_ASSERTMESSAGE("cannot find specified gpuContext in the gpucontext pool, something must be wrong");
    }
}

void GpuContextMgr::DestroyAllGpuContexts()
{
    MOS_OS_FUNCTION_ENTER;

    GpuContext *curGpuContext = nullptr;

    MOS_LockMutex(m_gpuContextArrayMutex);

    // delete each instance in m_gpuContextArray
    for (auto& curGpuContext : m_gpuContextArray)
    {
        MOS_Delete(curGpuContext);
    }

    m_gpuContextArray.clear(); // clear whole array

    MOS_UnlockMutex(m_gpuContextArrayMutex);
}
