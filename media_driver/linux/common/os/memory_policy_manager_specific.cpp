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
//! \file     memory_policy_manager_specific.cpp
//! \brief    Defines interfaces for media memory policy manager.

#include "memory_policy_manager.h"


int MemoryPolicyManager::UpdateMemoryPolicyWithWA(
    MemoryPolicyParameter* memPolicyPar,
    int& mem_type)
{
    if (!memPolicyPar || !memPolicyPar->skuTable || !memPolicyPar->resInfo)
    {
        MOS_OS_ASSERTMESSAGE("Null pointer");
        return 0;
    }

    if (MEDIA_IS_WA(memPolicyPar->waTable, WaForceAllocateLML2))
    {
        mem_type = MOS_MEMPOOL_DEVICEMEMORY;

        if (memPolicyPar->uiType == VAEncCodedBufferType && !MEDIA_IS_WA(memPolicyPar->waTable, Wa_14012254246))
        {
            mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
        }
    }

    if (MEDIA_IS_WA(memPolicyPar->waTable, WaForceAllocateLML3))
    {
        if(memPolicyPar->preferredMemType == MOS_MEMPOOL_VIDEOMEMORY)
        {
            mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
        }
    }

    if(memPolicyPar->isServer)
    {
        if (strcmp(memPolicyPar->resName, "MOS CmdBuf") == 0 ||
            strcmp(memPolicyPar->resName, "BatchBuffer") == 0
            )
        {
            mem_type = MOS_MEMPOOL_SYSTEMMEMORY;
        }
    }

    return 0;
}