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
#ifndef _VP_RESOURCE_MANAGER_H__
#define _VP_RESOURCE_MANAGER_H__

#include <map>

namespace vp {

struct VP_RESOURCE_INFO
{
    MOS_ALLOC_GFXRES_PARAMS AllocParam;
    MOS_RESOURCE            OsResource;
};

inline bool operator==(MOS_ALLOC_GFXRES_PARAMS &allocParam1, MOS_ALLOC_GFXRES_PARAMS &allocParam2)
{
    return allocParam1.Type == allocParam2.Type &&
        0 == memcmp(&allocParam1.Flags, &allocParam2.Flags, sizeof(allocParam1.Flags)),
        allocParam1.dwWidth == allocParam2.dwWidth &&
        allocParam1.dwHeight == allocParam2.dwHeight &&
        allocParam1.dwDepth == allocParam2.dwDepth &&
        allocParam1.dwArraySize == allocParam2.dwArraySize &&
        allocParam1.TileType == allocParam2.TileType &&
        allocParam1.Format == allocParam2.Format &&
        allocParam1.pSystemMemory == allocParam2.pSystemMemory &&
        nullptr == allocParam1.pSystemMemory &&
        allocParam1.bIsCompressible == allocParam2.bIsCompressible &&
        allocParam1.CompressionMode == allocParam2.CompressionMode &&
        allocParam1.bIsPersistent == allocParam2.bIsPersistent;
}

#define MAX_IDLE_RESOURCE_COUNT 10

class VpResourceAllocator
{
public:
    VpResourceAllocator(MOS_INTERFACE &osInterface, VpAllocator &allocator);
    virtual ~VpResourceAllocator();
    MOS_RESOURCE *AllocateResource(MOS_ALLOC_GFXRES_PARAMS &allocParam);
    void ReleaseResource(MOS_RESOURCE *res);

private:
    std::list<VP_RESOURCE_INFO *> m_IdleResPool = {}; 
    std::list<VP_RESOURCE_INFO *> m_InuseResPool = {};
    MOS_INTERFACE                &m_OsInterface;
    VpAllocator                  &m_Allocator;
};
}
#endif // !PacketPipe
