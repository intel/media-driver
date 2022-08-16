/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_debug.cpp
//! \brief    Implementation of functions for debugging VPHAL
//! \details  This file contains the Implementation of functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#include "vp_debug.h"

// Max number of source info to be dumped into OCA.
#define MAX_NUMBER_OF_SOURCE_INFO_IN_OCA_BUFFER 8
// Max number of target info to be dumped into OCA.
#define MAX_NUMBER_OF_TARGET_INFO_IN_OCA_BUFFER 4

VphalOcaDumper::VphalOcaDumper()
{
}

VphalOcaDumper::~VphalOcaDumper()
{
    MOS_DeleteArray(m_pOcaRenderParam);
}

void VphalOcaDumper::Delete(void *&p)
{
    VphalOcaDumper *pOcaDumpter =  (VphalOcaDumper *)p;
    MOS_Delete(pOcaDumpter);
    p = nullptr;
}

void VphalOcaDumper::SetRenderParam(VPHAL_RENDER_PARAMS *pRenderParams)
{
    if (nullptr == pRenderParams)
    {
        return;
    }

    uint32_t uSrcCountDumped = MOS_MIN(pRenderParams->uSrcCount, MAX_NUMBER_OF_SOURCE_INFO_IN_OCA_BUFFER);
    uint32_t uDstCountDumped = MOS_MIN(pRenderParams->uDstCount, MAX_NUMBER_OF_TARGET_INFO_IN_OCA_BUFFER);

    uint32_t size = sizeof(VPHAL_OCA_RENDER_PARAM) + uSrcCountDumped * sizeof(VPHAL_OCA_SOURCE_INFO) +
        uDstCountDumped * sizeof(VPHAL_OCA_TARGET_INFO);
    uint32_t allocSize = size;

    if (m_pOcaRenderParam)
    {
        if (allocSize > m_pOcaRenderParam->Header.allocSize)
        {
            MOS_DeleteArray(m_pOcaRenderParam);
        }
        else
        {
            // Reuse previous buffer.
            allocSize = m_pOcaRenderParam->Header.allocSize;
        }
    }

    if (nullptr == m_pOcaRenderParam)
    {
        m_pOcaRenderParam = (VPHAL_OCA_RENDER_PARAM *)MOS_NewArray(char, allocSize);
        if (nullptr == m_pOcaRenderParam)
        {
            return;
        }
    }
    MOS_ZeroMemory(m_pOcaRenderParam, size);

    m_pOcaRenderParam->Header.size          = size;
    m_pOcaRenderParam->Header.allocSize     = allocSize;
    m_pOcaRenderParam->Component            = pRenderParams->Component;

    if (pRenderParams->uSrcCount > 0 && pRenderParams->pSrc[0])
    {
        m_pOcaRenderParam->FrameID = pRenderParams->pSrc[0]->FrameID;
    }

    m_pOcaRenderParam->Pid = MosUtilities::MosGetPid();

    m_pOcaRenderParam->uSrcCount = pRenderParams->uSrcCount;
    m_pOcaRenderParam->uDstCount = pRenderParams->uDstCount;
    m_pOcaRenderParam->uSrcCountDumped = uSrcCountDumped;
    m_pOcaRenderParam->uDstCountDumped = uDstCountDumped;

    if (pRenderParams->pColorFillParams)
    {
        m_pOcaRenderParam->ColorFillParams.params = *pRenderParams->pColorFillParams;
        m_pOcaRenderParam->ColorFillParams.bValid = true;
    }

    uint32_t offset = sizeof(VPHAL_OCA_RENDER_PARAM);
    VPHAL_OCA_SOURCE_INFO *pSource = uSrcCountDumped > 0 ? (VPHAL_OCA_SOURCE_INFO *)((char*)m_pOcaRenderParam + offset) : nullptr;
    offset += uSrcCountDumped * sizeof(VPHAL_OCA_SOURCE_INFO);
    VPHAL_OCA_TARGET_INFO *pTarget = uDstCountDumped > 0 ? (VPHAL_OCA_TARGET_INFO *)((char*)m_pOcaRenderParam + offset) : nullptr;

    if (pSource)
    {
        for (uint32_t i = 0; i < uSrcCountDumped; ++i)
        {
            if (pRenderParams->pSrc[i])
            {
                InitSourceInfo(pSource[i], *(VPHAL_SURFACE *)pRenderParams->pSrc[i]);
            }
        }
    }

    if (pTarget)
    {
        for (uint32_t i = 0; i < uDstCountDumped; ++i)
        {
            if (pRenderParams->pTarget[i])
            {
                InitTargetInfo(pTarget[i], *(VPHAL_SURFACE *)pRenderParams->pTarget[i]);
            }
        }
    }
}

void VphalOcaDumper::InitSurfInfo(VPHAL_OCA_SURFACE_INFO &surfInfo, VPHAL_SURFACE &surf)
{
    surfInfo.Format        = surf.Format;
    surfInfo.SurfType      = surf.SurfType;
    surfInfo.SampleType    = surf.SampleType;
    surfInfo.ColorSpace    = surf.ColorSpace;
    surfInfo.ScalingMode   = surf.ScalingMode;
    surfInfo.TileType      = surf.TileType;
    surfInfo.dwWidth       = surf.dwWidth;
    surfInfo.dwHeight      = surf.dwHeight;
    surfInfo.dwPitch       = surf.dwPitch;
    surfInfo.rcSrc         = surf.rcSrc;
    surfInfo.rcDst         = surf.rcDst;
}

void VphalOcaDumper::InitSourceInfo(VPHAL_OCA_SOURCE_INFO &sourceInfo, VPHAL_SURFACE &source)
{
    InitSurfInfo(sourceInfo.surfInfo, source);
    sourceInfo.Rotation         = source.Rotation;
    sourceInfo.iPalette         = source.iPalette;
    sourceInfo.PaletteParams    = source.Palette;

    if (source.pBlendingParams)
    {
        sourceInfo.BlendingParams.params = *source.pBlendingParams;
        sourceInfo.BlendingParams.bValid = true;
    }

    if (source.pLumaKeyParams)
    {
        sourceInfo.LumaKeyParams.params = *source.pLumaKeyParams;
        sourceInfo.LumaKeyParams.bValid = true;
    }

    if (source.pProcampParams)
    {
        sourceInfo.ProcampParams.params = *source.pProcampParams;
        sourceInfo.ProcampParams.bValid = true;
    }

    if (source.pIEFParams)
    {
        sourceInfo.IEFParams.params = *source.pIEFParams;
        sourceInfo.IEFParams.bValid = true;
    }

    if (source.pDeinterlaceParams)
    {
        sourceInfo.DIParams.params = *source.pDeinterlaceParams;
        sourceInfo.DIParams.bValid = true;
    }

    if (source.pDenoiseParams)
    {
        sourceInfo.DNParams.params = *source.pDenoiseParams;
        sourceInfo.DNParams.bValid = true;
    }

    if (source.pColorPipeParams)
    {
        sourceInfo.ColorPipeParams.params = *source.pColorPipeParams;
        sourceInfo.ColorPipeParams.bValid = true;
    }

    if (source.uBwdRefCount > 0)
    {
        sourceInfo.BwdRefInfo.uBwdRefCount  = source.uBwdRefCount;
        sourceInfo.BwdRefInfo.bValid        = true;
    }

    if (source.uFwdRefCount > 0)
    {
        sourceInfo.FwdRefInfo.uFwdRefCount  = source.uFwdRefCount;
        sourceInfo.FwdRefInfo.bValid        = true;
    }

    if (source.pHDRParams)
    {
        sourceInfo.HDRParams.params = *source.pHDRParams;
        sourceInfo.HDRParams.bValid = true;
    }
}

void VphalOcaDumper::InitTargetInfo(VPHAL_OCA_TARGET_INFO &targetInfo, VPHAL_SURFACE &target)
{
    InitSurfInfo(targetInfo.surfInfo, target);

    if (target.pHDRParams)
    {
        targetInfo.HDRParams.params = *target.pHDRParams;
        targetInfo.HDRParams.bValid = true;
    }
}
