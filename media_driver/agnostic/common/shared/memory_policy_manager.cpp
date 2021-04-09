/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     memory_policy_manager.cpp
//! \brief    Defines interfaces for media memory policy manager.

#include "memory_policy_manager.h"

int MemoryPolicyManager::UpdateMemoryPolicy(
    MemoryPolicyParameter* memPolicyPar)
{
    int mem_type = MOS_MEMPOOL_VIDEOMEMORY;

    if(!memPolicyPar || !memPolicyPar->skuTable || !memPolicyPar->resInfo)
    {
        MOS_OS_ASSERTMESSAGE("Null pointer");
        return mem_type;
    }

    if(!MEDIA_IS_SKU(memPolicyPar->skuTable, FtrLocalMemory))
    {
        MOS_OS_VERBOSEMESSAGE("No FtrLocalMemory");
        return mem_type;
    }

    GMM_TILE_TYPE tile_type = memPolicyPar->resInfo->GetTileType();
    GMM_RESOURCE_FLAG& resFlag = memPolicyPar->resInfo->GetResFlags();
    GMM_RESOURCE_TYPE res_type = memPolicyPar->resInfo->GetResourceType();

    if (memPolicyPar->preferredMemType != MOS_MEMPOOL_VIDEOMEMORY &&
        memPolicyPar->preferredMemType != MOS_MEMPOOL_DEVICEMEMORY &&
        memPolicyPar->preferredMemType != MOS_MEMPOOL_SYSTEMMEMORY)
    {
        MOS_OS_ASSERTMESSAGE("Wrong preferredMemType %d", memPolicyPar->preferredMemType);
        return mem_type;
    }

    // Follow default setting, tiled resource in Video Memory, 1D linear resource in System Memory
    if (tile_type == GMM_NOT_TILED && res_type == RESOURCE_1D)
    {
        mem_type                  = MOS_MEMPOOL_SYSTEMMEMORY;
        resFlag.Info.LocalOnly    = 0;
        resFlag.Info.NonLocalOnly = 1;
    }
    else
    {
        mem_type                  = MOS_MEMPOOL_VIDEOMEMORY;
        resFlag.Info.LocalOnly    = 0;
        resFlag.Info.NonLocalOnly = 0;
    }

    // Override setting, depending on preferredMemType
    if ((memPolicyPar->preferredMemType & MOS_MEMPOOL_DEVICEMEMORY) && !(mem_type & MOS_MEMPOOL_DEVICEMEMORY))
    {
        mem_type                  = MOS_MEMPOOL_DEVICEMEMORY;
        resFlag.Info.LocalOnly    = 1;
        resFlag.Info.NonLocalOnly = 0;
    }

    if ((memPolicyPar->preferredMemType & MOS_MEMPOOL_SYSTEMMEMORY) && !(mem_type & MOS_MEMPOOL_SYSTEMMEMORY))
    {
        mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
        resFlag.Info.LocalOnly    = 0;
        resFlag.Info.NonLocalOnly = 1;
    }

    UpdateMemoryPolicyWithWA(memPolicyPar, mem_type);

    uint32_t surfSize = (uint32_t)memPolicyPar->resInfo->GetSizeSurface();

    if (1 == resFlag.Info.LocalOnly && 0 == resFlag.Info.NotLockable)
    {
        MOS_OS_ASSERTMESSAGE("Invalid setting! Local only memory with cpu visible.");
        MOS_OS_ASSERTMESSAGE("\"%s\" preferredMemType %d, mem_type %d, res_type %d, size %d", (memPolicyPar->resName ? memPolicyPar->resName : "Resource"), memPolicyPar->preferredMemType, mem_type, res_type, surfSize);
    }
    else
    {
        MOS_OS_NORMALMESSAGE("\"%s\" preferredMemType %d, mem_type %d, res_type %d, size %d", (memPolicyPar->resName ? memPolicyPar->resName : "Resource"), memPolicyPar->preferredMemType, mem_type, res_type, surfSize);
    }

    return mem_type;
}
