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
//! \file      mos_os_specific.c
//! \brief     Common interface used in MOS LINUX OS
//! \details   Common interface used in MOS LINUX OS
//!
#include "mos_os_specific_next.h"

#include "mos_os_next.h"
#include "mos_util_debug_next.h"
#include <unistd.h>
#include <dlfcn.h>
#include "hwinfo_linux.h"
#include "media_fourcc.h"
#include <stdlib.h>

#include "mos_graphicsresource_next.h"
#include "mos_context_specific.h"
#include "mos_gpucontext_specific.h"
#include "mos_gpucontextmgr_next.h"

#if MOS_MEDIASOLO_SUPPORTED
#include "mos_os_solo.h"
#endif // MOS_MEDIASOLO_SUPPORTED
#include "mos_solo_generic.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

GpuContextSpecific * MosOsSpecificNext::Linux_GetGpuContext(PMOS_INTERFACE pOsInterface, uint32_t gpuContextHandle)
{
    MOS_OS_FUNCTION_ENTER;

    if (pOsInterface == nullptr || pOsInterface->osContextPtr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("invalid input parameters!");
        return nullptr;
    }

    auto osCxtSpecific = static_cast<OsContextSpecific *>(pOsInterface->osContextPtr);

    auto gpuContextMgr = osCxtSpecific->GetGpuContextMgr();
    if (gpuContextMgr == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("m_gpuContextMgr cannot be nullptr");
        return nullptr;
    }

    auto gpuContext = gpuContextMgr->GetGpuContext(gpuContextHandle);
    if (gpuContext == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("cannot find the gpuContext corresponding to the active gpuContextHandle");
        return nullptr;
    }

    auto gpuContextSpecific = static_cast<GpuContextSpecific *>(gpuContext);

    return gpuContextSpecific;
}

GMM_RESOURCE_FORMAT MosOsSpecificNext::Mos_Specific_ConvertMosFmtToGmmFmt(
    MOS_FORMAT format)
{
    switch (format)
    {
        case Format_Buffer      : return GMM_FORMAT_GENERIC_8BIT;
        case Format_Buffer_2D   : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_L8          : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_L16         : return GMM_FORMAT_L16_UNORM_TYPE;
        case Format_STMM        : return GMM_FORMAT_R8_UNORM_TYPE;              // matching size as format
        case Format_AI44        : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_IA44        : return GMM_FORMAT_GENERIC_8BIT;               // matching size as format
        case Format_R5G6B5      : return GMM_FORMAT_B5G6R5_UNORM_TYPE;
        case Format_R8G8B8      : return GMM_FORMAT_R8G8B8_UNORM;
        case Format_X8R8G8B8    : return GMM_FORMAT_B8G8R8X8_UNORM_TYPE;
        case Format_A8R8G8B8    : return GMM_FORMAT_B8G8R8A8_UNORM_TYPE;
        case Format_X8B8G8R8    : return GMM_FORMAT_R8G8B8X8_UNORM_TYPE;
        case Format_A8B8G8R8    : return GMM_FORMAT_R8G8B8A8_UNORM_TYPE;
        case Format_R32F        : return GMM_FORMAT_R32_FLOAT_TYPE;
        case Format_V8U8        : return GMM_FORMAT_GENERIC_16BIT;              // matching size as format
        case Format_YUY2        : return GMM_FORMAT_YUY2;
        case Format_UYVY        : return GMM_FORMAT_UYVY;
        case Format_P8          : return GMM_FORMAT_RENDER_8BIT_TYPE;           // matching size as format
        case Format_A8          : return GMM_FORMAT_A8_UNORM_TYPE;
        case Format_AYUV        : return GMM_FORMAT_R8G8B8A8_UINT_TYPE;
        case Format_NV12        : return GMM_FORMAT_NV12_TYPE;
        case Format_NV21        : return GMM_FORMAT_NV21_TYPE;
        case Format_YV12        : return GMM_FORMAT_YV12_TYPE;
        case Format_R32U        : return GMM_FORMAT_R32_UINT_TYPE;
        case Format_R32S        : return GMM_FORMAT_R32_SINT_TYPE;
        case Format_RAW         : return GMM_FORMAT_GENERIC_8BIT;
        case Format_444P        : return GMM_FORMAT_MFX_JPEG_YUV444_TYPE;
        case Format_422H        : return GMM_FORMAT_MFX_JPEG_YUV422H_TYPE;
        case Format_422V        : return GMM_FORMAT_MFX_JPEG_YUV422V_TYPE;
        case Format_IMC3        : return GMM_FORMAT_IMC3_TYPE;
        case Format_411P        : return GMM_FORMAT_MFX_JPEG_YUV411_TYPE;
        case Format_411R        : return GMM_FORMAT_MFX_JPEG_YUV411R_TYPE;
        case Format_RGBP        : return GMM_FORMAT_RGBP_TYPE;
        case Format_BGRP        : return GMM_FORMAT_BGRP_TYPE;
        case Format_R8U         : return GMM_FORMAT_R8_UINT_TYPE;
        case Format_R16U        : return GMM_FORMAT_R16_UINT_TYPE;
        case Format_R16F        : return GMM_FORMAT_R16_FLOAT_TYPE;
        case Format_P010        : return GMM_FORMAT_P010_TYPE;
        case Format_P016        : return GMM_FORMAT_P016_TYPE;
        case Format_Y216        : return GMM_FORMAT_Y216_TYPE;
        case Format_Y416        : return GMM_FORMAT_Y416_TYPE;
        case Format_P208        : return GMM_FORMAT_P208_TYPE;
        case Format_Y210        : return GMM_FORMAT_Y210_TYPE;
        case Format_Y410        : return GMM_FORMAT_Y410_TYPE;
        default                 : return GMM_FORMAT_INVALID;
    }
}


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
    auto gpuContext = MosOsSpecificNext::Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_ASSERT(gpuContext != nullptr);

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

    auto gpuContext = MosOsSpecificNext::Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_ASSERT(gpuContext != nullptr);

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
    const auto *gpuContext = MosOsSpecificNext::Linux_GetGpuContext(pOsInterface, pOsInterface->CurrentGpuContextHandle);
    MOS_OS_ASSERT(gpuContext != nullptr);

    return gpuContext->GetCmdResPtrs();
}
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
