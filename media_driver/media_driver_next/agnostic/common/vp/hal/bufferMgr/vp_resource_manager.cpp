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
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "vp_resource_manager.h"

using namespace std;
using namespace vp;

VpResourceAllocator::VpResourceAllocator(MOS_INTERFACE &osInterface, VpAllocator &allocator) : m_OsInterface(osInterface), m_Allocator(allocator)
{
}

VpResourceAllocator::~VpResourceAllocator()
{
}

MOS_RESOURCE *vp::VpResourceAllocator::AllocateResource(MOS_ALLOC_GFXRES_PARAMS &allocParam)
{
    list<VP_RESOURCE_INFO *>::iterator it = m_IdleResPool.begin();
    for (; it != m_IdleResPool.end(); ++it)
    {
        if (allocParam == (*it)->AllocParam)
        {
            break;
        }
    }

    if (it != m_IdleResPool.end())
    {
        VP_RESOURCE_INFO *p = *it;
        m_IdleResPool.erase(it);
        m_InuseResPool.push_back(p);
        return &p->OsResource;
    }

    VP_RESOURCE_INFO *pResInfo = MOS_New(VP_RESOURCE_INFO);
    MOS_ZeroMemory(pResInfo, sizeof(VP_RESOURCE_INFO));

    pResInfo->AllocParam = allocParam;

    m_OsInterface.pfnAllocateResource(&m_OsInterface, &pResInfo->AllocParam, &pResInfo->OsResource);
    return &pResInfo->OsResource;
}

void VpResourceAllocator::ReleaseResource(MOS_RESOURCE *res)
{
    list<VP_RESOURCE_INFO *>::iterator it = m_InuseResPool.begin();
    for (; it != m_InuseResPool.end(); ++it)
    {
        if (res == &(*it)->OsResource)
        {
            break;
        }
    }

    if (it == m_InuseResPool.end())
    {
        MOS_OS_ASSERTMESSAGE("Invalid Resource!");
        return;
    }

    VP_RESOURCE_INFO *p = *it;
    m_InuseResPool.erase(it);
    m_IdleResPool.push_front(p);

    if (m_IdleResPool.size() > MAX_IDLE_RESOURCE_COUNT)
    {
        VP_RESOURCE_INFO *pInfo = m_IdleResPool.back();
        m_IdleResPool.pop_back();
        delete pInfo;
    }
}