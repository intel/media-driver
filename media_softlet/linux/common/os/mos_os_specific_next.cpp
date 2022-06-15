/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file      mos_os_specific.c
//! \brief     Common interface used in MOS LINUX OS
//! \details   Common interface used in MOS LINUX OS
//!

#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#include "mos_os_next.h"
#include "mos_util_debug.h"
#include "hwinfo_linux.h"
#include "media_fourcc.h"
#include "mos_graphicsresource_next.h"
#include "mos_gpucontext_specific_next.h"
#include "mos_gpucontextmgr_next.h"

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"


#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
struct GpuCmdResInfoDumpNext::GpuCmdResInfo
{
    int32_t             iWidth;
    int32_t             iHeight;
    int32_t             iSize;
    int32_t             iPitch;
    int32_t             iDepth;
    MOS_FORMAT          Format;
    int32_t             iCount;
    int32_t             iAllocationIndex[MOS_GPU_CONTEXT_MAX];
    uint32_t            dwGfxAddress;
    const char          *bufname;
    uint32_t            isTiled;
    MOS_TILE_TYPE       TileType;
    uint32_t            bMapped;
    uint32_t            name;
    uint64_t            user_provided_va;
    bool                bConvertedFromDDIResource;
};

void GpuCmdResInfoDumpNext::StoreCmdResPtr(PMOS_INTERFACE pOsInterface, const void *pRes) const
{
    if (!m_dumpEnabled)
    {
        return;
    }

    auto gpuContext = MosInterface::GetGpuContext(pOsInterface->osStreamState, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(gpuContext);

    auto pResTmp1 = (const MOS_RESOURCE *)(pRes);
    auto pResTmp2 = (GpuCmdResInfo *)MOS_AllocMemory(sizeof(GpuCmdResInfo));

    pResTmp2->iWidth                    = pResTmp1->iWidth;
    pResTmp2->iHeight                   = pResTmp1->iHeight;
    pResTmp2->iSize                     = pResTmp1->iSize;
    pResTmp2->iPitch                    = pResTmp1->iPitch;
    pResTmp2->iDepth                    = pResTmp1->iDepth;
    pResTmp2->Format                    = pResTmp1->Format;
    pResTmp2->iCount                    = pResTmp1->iCount;
    for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        pResTmp2->iAllocationIndex[i] = pResTmp1->iAllocationIndex[i];
    }
    pResTmp2->dwGfxAddress              = pResTmp1->dwGfxAddress;
    pResTmp2->bufname                   = pResTmp1->bufname;
    pResTmp2->isTiled                   = pResTmp1->isTiled;
    pResTmp2->TileType                  = pResTmp1->TileType;
    pResTmp2->bMapped                   = pResTmp1->bMapped;
    pResTmp2->name                      = pResTmp1->name;
    pResTmp2->user_provided_va          = pResTmp1->user_provided_va;
    pResTmp2->bConvertedFromDDIResource = pResTmp1->bConvertedFromDDIResource;

    gpuContext->PushCmdResPtr((const void *)pResTmp2);
}

void GpuCmdResInfoDumpNext::ClearCmdResPtrs(PMOS_INTERFACE pOsInterface) const
{
    if (!m_dumpEnabled)
    {
        return;
    }

    auto gpuContext = MosInterface::GetGpuContext(pOsInterface->osStreamState, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(gpuContext);

    auto &cmdResInfoPtrs = gpuContext->GetCmdResPtrs();

    for (auto e : cmdResInfoPtrs)
    {
        MOS_FreeMemory((void *)e);
    }

    gpuContext->ClearCmdResPtrs();
}

void GpuCmdResInfoDumpNext::Dump(const void *cmdResInfoPtr, std::ofstream &outputFile) const
{
    using std::endl;

    auto pRes = (const GpuCmdResInfo *)(cmdResInfoPtr);

    outputFile << "Gpu Resource Pointer      : " << pRes << endl;
    outputFile << "iWidth                    : " << pRes->iWidth << endl;
    outputFile << "iHeight                   : " << pRes->iHeight << endl;
    outputFile << "iSize                     : " << pRes->iSize << endl;
    outputFile << "iPitch                    : " << pRes->iPitch << endl;
    outputFile << "iDepth                    : " << pRes->iDepth << endl;
    outputFile << "Format                    : " << (int32_t)pRes->Format << endl;
    outputFile << "iCount                    : " << pRes->iCount << endl;
    outputFile << "iAllocationIndex          : ";
    for (auto i = 0; i < MOS_GPU_CONTEXT_MAX; i++)
    {
        outputFile << pRes->iAllocationIndex[i] << " ";
    }
    outputFile << endl;
    outputFile << "dwGfxAddress              : " << pRes->dwGfxAddress << endl;
    outputFile << "bufname                   : " << pRes->bufname << endl;
    outputFile << "isTiled                   : " << pRes->isTiled << endl;
    outputFile << "TileType                  : " << GetTileType(pRes->TileType) << endl;
    outputFile << "bMapped                   : " << pRes->bMapped << endl;
    outputFile << "name                      : " << pRes->name << endl;
    outputFile << "user_provided_va          : " << pRes->user_provided_va << endl;
    outputFile << "bConvertedFromDDIResource : " << pRes->bConvertedFromDDIResource << endl;
    outputFile << endl;
}

const std::vector<const void *> &GpuCmdResInfoDumpNext::GetCmdResPtrs(PMOS_INTERFACE pOsInterface) const
{
    const auto *gpuContext = MosInterface::GetGpuContext(pOsInterface->osStreamState, pOsInterface->CurrentGpuContextHandle);
    if(gpuContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("gpuContext == nullptr");
        static const std::vector<const void*> dummyVec = {nullptr};
        return dummyVec;
    }

    return gpuContext->GetCmdResPtrs();
}
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
