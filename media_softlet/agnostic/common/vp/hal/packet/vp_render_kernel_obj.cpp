/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_render_kernel_obj.cpp
//! \brief    vp render kernel base object.
//! \details  vp render kernel base object will provided interface where sub kernels processing ways
//!
#include "vp_render_kernel_obj.h"
#include "vp_dumper.h"

using namespace vp;

VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
}

VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, VpKernelID kernelId, uint32_t kernelIndex) :
    m_hwInterface(hwInterface), m_kernelId(kernelId), m_kernelIndex(kernelIndex)
{
}

VpRenderKernelObj::~VpRenderKernelObj()
{
}

// For Adv kernel
MOS_STATUS VpRenderKernelObj::Init(VpRenderKernel& kernel)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    return MOS_STATUS_SUCCESS;
}

uint32_t VpRenderKernelObj::GetKernelBinaryID()
{
    VP_FUNC_CALL();

    return m_kernelBinaryID;
}

MOS_STATUS VpRenderKernelObj::GetKernelEntry(Kdll_CacheEntry &entry)
{
    VP_FUNC_CALL();

    // Set Parameters for Kernel Entry
    entry.iKUID         = m_kernelBinaryID;
    entry.iKCID         = -1;
    entry.iFilterSize   = 2;
    entry.pFilter       = nullptr;
    entry.iSize         = m_kernelSize;
    entry.pBinary       = (uint8_t *)m_kernelBinary;
    return MOS_STATUS_SUCCESS;
}

uint32_t VpRenderKernelObj::GetKernelIndex()
{
    VP_FUNC_CALL();

    return m_kernelIndex;
}

MOS_STATUS VpRenderKernelObj::GetWalkerSetting(KERNEL_WALKER_PARAMS& walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();
    walkerParam.iBindingTable   = renderData.bindingTable;
    walkerParam.iMediaID        = renderData.mediaID;
    walkerParam.iCurbeOffset    = renderData.iCurbeOffset;
    // Should use renderData.iCurbeLength instead of kernelSettings.CURBE_Length.
    // kernelSettings.CURBE_Length is 32 aligned with 5 bits shift.
    // renderData.iCurbeLength is RENDERHAL_CURBE_BLOCK_ALIGN(64) aligned.
    walkerParam.iCurbeLength    = renderData.iCurbeLength;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup)
{
    VP_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderKernelObj::SetWalkerSetting(KERNEL_THREAD_SPACE& threadSpace, bool bSyncFlag)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelArgs(KERNEL_ARGS& kernelArgs)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderKernelObj::SetKernelConfigs(
    KERNEL_PARAMS& kernelParams,
    VP_SURFACE_GROUP& surfaces,
    KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(SetKernelArgs(kernelParams.kernelArgs));

    VP_RENDER_CHK_STATUS_RETURN(SetProcessSurfaceGroup(surfaces));

    VP_RENDER_CHK_STATUS_RETURN(SetSamplerStates(samplerStateGroup));

    VP_RENDER_CHK_STATUS_RETURN(SetWalkerSetting(kernelParams.kernelThreadSpace, kernelParams.syncFlag));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs)
{
    VP_FUNC_CALL();

    //For legacy kernel usage
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::InitKernel(void* binary, uint32_t size, KERNEL_CONFIGS& kernelConfigs, VP_SURFACE_GROUP& surfacesGroup)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(binary);
    // m_kernelBinary and m_kernelSize being nullptr and 0 for FC case.
    m_kernelBinary = binary;
    m_kernelSize = size;

    VP_RENDER_CHK_STATUS_RETURN(SetKernelConfigs(kernelConfigs));
    VP_RENDER_CHK_STATUS_RETURN(SetProcessSurfaceGroup(surfacesGroup));

    return MOS_STATUS_SUCCESS;
}

void VpRenderKernelObj::DumpSurface(VP_SURFACE* pSurface, PCCHAR fileName)
{
    uint8_t* pData;
    char                    sPath[MAX_PATH];
    uint8_t* pDst;
    uint8_t* pTmpDst;
    uint8_t* pTmpSrc;
    uint32_t                iWidthInBytes;
    uint32_t                iHeightInRows;
    uint32_t                iBpp;
    uint32_t                iSize;
    uint32_t                iY;
    MOS_LOCK_PARAMS         LockFlags;

    VP_FUNC_CALL();

    PMOS_INTERFACE        pOsInterface = m_hwInterface->m_osInterface;

    pDst = nullptr;
    MOS_ZeroMemory(sPath, MAX_PATH);

    // get bits per pixel for the format
    pOsInterface->pfnGetBitsPerPixel(pOsInterface, pSurface->osSurface->Format, &iBpp);

    iWidthInBytes = pSurface->osSurface->dwWidth;
    iHeightInRows = pSurface->osSurface->dwHeight;

    iSize = iWidthInBytes * iHeightInRows;

    // Write original image to file
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.ReadOnly = 1;

    pData = (uint8_t*)m_allocator->Lock(
        &pSurface->osSurface->OsResource,
        &LockFlags);

    if (pData == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("pData == nullptr");
        return;
    }

    MOS_SecureStringPrint(
        sPath,
        MAX_PATH,
        sizeof(sPath),
        "c:\\dump\\f[%08I64x]_%s_w[%d]_h[%d]_p[%d].%s",
        1,
        fileName,
        pSurface->osSurface->dwWidth,
        pSurface->osSurface->dwHeight,
        pSurface->osSurface->dwPitch,
        VP_GET_FORMAT_STRING(pSurface->osSurface->Format));

    // Write the data to file
    if (pSurface->osSurface->dwPitch == iWidthInBytes)
    {
        MOS_WriteFileFromPtr((const char*)sPath, pData, iSize);
    }
    else
    {
        pDst = (uint8_t*)MOS_AllocAndZeroMemory(iSize);
        pTmpSrc = pData;
        pTmpDst = pDst;

        for (iY = 0; iY < iHeightInRows; iY++)
        {
            MOS_SecureMemcpy(pTmpDst, iSize, pTmpSrc, iWidthInBytes);
            pTmpSrc += pSurface->osSurface->dwPitch;
            pTmpDst += iWidthInBytes;
        }

        MOS_WriteFileFromPtr((const char*)sPath, pDst, iSize);
    }

    if (pDst)
    {
        MOS_FreeMemory(pDst);
    }

    m_allocator->UnLock(&pSurface->osSurface->OsResource);
}
