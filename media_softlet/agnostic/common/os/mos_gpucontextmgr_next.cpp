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
#include "mos_context_next.h"
#include "mos_oca_rtlog_mgr.h"

GpuContextMgrNext::GpuContextMgrNext(OsContextNext *osContext)
{
    MOS_OS_FUNCTION_ENTER;
    m_initialized = false;

    if (osContext)
    {
        m_osContext = osContext;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Input osContext cannot be nullptr");
        return;
    }
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

MOS_STATUS GpuContextMgrNext::Initialize()
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    m_gpuContextArrayMutex = MosUtilities::MosCreateMutex();
    MOS_OS_CHK_NULL_RETURN(m_gpuContextArrayMutex);

    MosUtilities::MosLockMutex(m_gpuContextArrayMutex);
    m_gpuContextArray.clear();
    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);

    m_initialized = true;
    return status;
}


GpuContextMgrNext *GpuContextMgrNext::GetObject(
    OsContextNext     *osContext)
{
    MOS_OS_FUNCTION_ENTER;
    if (osContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid input parameters!");
        return nullptr;
    }

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    GpuContextMgrNext* pGpuContext = MOS_New(GpuContextMgrNext, osContext);
    if (!pGpuContext)
    {
        return nullptr;
    }
    status = pGpuContext->Initialize();
    if (MOS_FAILED(status))
    {
        MOS_Delete(pGpuContext);
        return nullptr;
    }
    return pGpuContext;
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

    if (cmdBufMgr == nullptr && !m_osContext->IsAynchronous())
    {
        MOS_OS_ASSERTMESSAGE("nullptr of cmdbufmgr in normal mode. nullptr can only be applied in Async mode");
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

    MT_LOG5(MT_MOS_GPUCXT_CREATE, MT_NORMAL, MT_MOS_GPUCXT_MGR_PTR, (int64_t)this, MT_MOS_GPUCXT_PTR, (int64_t)gpuContext,
        MT_MOS_GPUCXT_COUNT, m_gpuContextCount, MT_MOS_GPU_NODE, gpuNode, MT_MOS_GPUCXT_HANDLE, gpuContextHandle);

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

    GpuContextNext *gpuContext = nullptr;
    MosUtilities::MosLockMutex(m_gpuContextArrayMutex);
    if (!m_gpuContextArray.empty() && gpuContextHandle < m_gpuContextArray.size())
    {
        gpuContext = m_gpuContextArray.at(gpuContextHandle);
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("GPU context array is empty or got invalid index, something must be wrong!");
        MT_ERR2(MT_MOS_GPUCXT_GET, MT_MOS_GPUCXT_MGR_PTR, (int64_t)this, MT_MOS_GPUCXT_HANDLE, gpuContextHandle);
        gpuContext = nullptr;
    }
    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);
    return gpuContext;
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
            curGpuContext = nullptr;
            m_gpuContextCount--;
            break;
        }
    }

    if (m_gpuContextCount == 0 && !m_noCycledGpuCxtMgmt)
    {
        m_gpuContextArray.clear();  // clear whole array
    }

    MT_LOG3(MT_MOS_GPUCXT_DESTROY, MT_NORMAL, MT_MOS_GPUCXT_MGR_PTR, (int64_t)this, MT_MOS_GPUCXT_PTR, (int64_t)gpuContext, MT_MOS_GPUCXT_COUNT, m_gpuContextCount);
    OCA_MT_ERR1(MT_MOS_GPUCXT_DESTROY, MT_MOS_GPUCXT_MGR_PTR, (int64_t)this, MOS_OCA_RTLOG_COMPONENT_COMMON);
    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);

    if (found)
    {
        MOS_Delete(gpuContext);  // delete gpu context.
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("cannot find specified gpuContext in the gpucontext pool, something must be wrong");
        MT_ERR3(MT_MOS_GPUCXT_DESTROY, MT_MOS_GPUCXT_MGR_PTR, (int64_t)this, MT_MOS_GPUCXT_PTR, (int64_t)gpuContext, MT_CODE_LINE, __LINE__);
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
        MT_LOG2(MT_MOS_GPUCXT_DESTROY, MT_NORMAL, MT_MOS_GPUCXT_MGR_PTR, (int64_t)this, MT_MOS_GPUCXT_PTR, (int64_t)curGpuContext);
        MOS_Delete(curGpuContext);
    }

    m_gpuContextArray.clear();  // clear whole array

    MosUtilities::MosUnlockMutex(m_gpuContextArrayMutex);
}
