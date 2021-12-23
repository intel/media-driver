/*
// Copyright (C) 2020-2021 Intel Corporation
//
// Licensed under the Apache License,Version 2.0(the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
//!
//! \file     codechal_debug_xe_xpm_plus_ext.cpp
//! \brief    Defines the ext debug interface .
//! \details  The ext debug interface dumps that are used only for internal branches
//!

#include "codechal_debug.h"

#if USE_CODECHAL_DEBUG_TOOL

#include "codechal_debug_xe_xpm_plus_ext.h"
#include "codechal_utilities.h"
#include "codechal_hw_xe_xpm_plus.h"

CodechalDebugInterfaceXe_Xpm_Plus::CodechalDebugInterfaceXe_Xpm_Plus()
{

}

CodechalDebugInterfaceXe_Xpm_Plus::~CodechalDebugInterfaceXe_Xpm_Plus()
{

}

MOS_STATUS CodechalDebugInterfaceXe_Xpm_Plus::DumpBltOutput(
    PMOS_SURFACE              surface,
    const char *              attrName)
{
    if (!DumpIsEnabled(attrName))
    {
        return MOS_STATUS_SUCCESS;
    }

    bool copyMain = true;
    bool copyCcs = true;
    BltStateXe_Xpm *bltStateXehp = nullptr;
    CodechalHwInterfaceXe_Xpm_Plus* hwInterface = dynamic_cast<CodechalHwInterfaceXe_Xpm_Plus*>(m_hwInterface);
    CODECHAL_DEBUG_CHK_NULL(hwInterface);
    bltStateXehp = hwInterface->GetBltState();
    CODECHAL_DEBUG_CHK_NULL(bltStateXehp);
    const PMOS_SURFACE inputSurface = surface;
    MOS_ALLOC_GFXRES_PARAMS AllocParams = {};

    if (copyMain)
    {
        CODECHAL_DEBUG_CHK_STATUS_MESSAGE(
            DumpYUVSurface(
                inputSurface,
                attrName,
                "BltOutMainSurf"),
            "DumpYUVSurface BltOutMainSurf error");
    }

    if (copyCcs)
    {
        // dump ccs surface
        MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        AllocParams.TileType        = MOS_TILE_LINEAR;
        AllocParams.Type            = MOS_GFXRES_BUFFER;
        AllocParams.dwWidth         = (uint32_t)inputSurface->OsResource.pGmmResInfo->GetSizeMainSurface() / 256;
        AllocParams.dwHeight        = 1;
        AllocParams.Format          = Format_Buffer;
        AllocParams.bIsCompressible = false;
        AllocParams.CompressionMode = MOS_MMC_DISABLED;
        AllocParams.pBufName        = "TempCCS";
        AllocParams.dwArraySize     = 1;

        PMOS_SURFACE ccsSurface = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
        m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParams,
            &ccsSurface->OsResource);
        CODECHAL_DEBUG_CHK_STATUS(CodecHalGetResourceInfo(
            m_osInterface,
            ccsSurface));
        CODECHAL_DEBUG_VERBOSEMESSAGE("BLT CCS surface width %d, height %d, pitch %d, TileType %d, bIsCompressed %d, CompressionMode %d",
            ccsSurface->dwWidth, ccsSurface->dwHeight, ccsSurface->dwPitch, ccsSurface->TileType, ccsSurface->bIsCompressed, ccsSurface->CompressionMode);

        CODECHAL_DEBUG_CHK_STATUS_MESSAGE(
            bltStateXehp->GetCCS(inputSurface, ccsSurface),
            "BLT CopyMainSurface error");

        CODECHAL_DEBUG_CHK_STATUS_MESSAGE(
            DumpSurface(
                ccsSurface,
                attrName,
                "BltOutCcsSurf"),
            "DumpYUVSurface BltOutMainSurf error");

        m_osInterface->pfnFreeResource(m_osInterface, &ccsSurface->OsResource);
        MOS_FreeMemAndSetNull(ccsSurface);
    }

    return MOS_STATUS_SUCCESS;
}

#endif
