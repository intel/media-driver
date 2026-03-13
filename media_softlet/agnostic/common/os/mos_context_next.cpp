/*
* Copyright (c) 2019-2026, Intel Corporation
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

#include "mos_context_specific_next.h"
#include "mos_os_mock_adaptor.h"
#include "mos_os_mock_adaptor_specific.h"
#include "mos_oca_rtlog_mgr.h"

class OsContextNext* OsContextNext::GetOsContextObject()
{
    MOS_OS_FUNCTION_ENTER;
    class OsContextNext* osContextPtr = MOS_New(OsContextSpecificNext);

    return osContextPtr;
}

void OsContextNext::CleanUp()
{
    MOS_OS_FUNCTION_ENTER;

    MOS_Delete(m_mosDecompression);

    MOS_Delete(m_mosMediaCopy);

    MosOcaRTLogMgr::UnRegisterContext(this);

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

    NullHwDestroy();

    Destroy();
}

MOS_STATUS OsContextNext::NullHwInit(MOS_CONTEXT_HANDLE osContext)
{
    MOS_OS_FUNCTION_ENTER;

    // Per-Context: Create a new MockAdaptor instance for this context
    if (m_mockAdaptor == nullptr)
    {
        m_mockAdaptor = MOS_New(MosMockAdaptorSpecific);
        if (m_mockAdaptor == nullptr)
        {
            return MOS_STATUS_NO_SPACE;
        }
    }
    
    // Initialize the per-context MockAdaptor
    return m_mockAdaptor->Init(osContext, this);
}

MOS_STATUS OsContextNext::NullHwDestroy()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_mockAdaptor != nullptr)
    {
        m_mockAdaptor->Destroy();
        MOS_Delete(m_mockAdaptor);
        m_mockAdaptor = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}
