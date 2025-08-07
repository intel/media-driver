/*
* Copyright (c) 2020-2022, Intel Corporation
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
#include <iomanip>
#include "vp_render_kernel_obj.h"
#include "vp_dumper.h"
#include "hal_oca_interface_next.h"

using namespace vp;

VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    m_hwInterface(hwInterface),
    m_allocator(allocator)
{
}

VpRenderKernelObj::VpRenderKernelObj(PVP_MHWINTERFACE hwInterface, VpKernelID kernelId, uint32_t kernelIndex, std::string kernelName, PVpAllocator allocator) :
    m_hwInterface(hwInterface), m_allocator(allocator), m_kernelName(kernelName), m_kernelId(kernelId), m_kernelIndex(kernelIndex)
{
    VP_RENDER_NORMALMESSAGE("kernel name is %s, kernel ID is %d", kernelName.c_str(), kernelId);
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
    entry.iPaddingSize  = m_kernelPaddingSize; 
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

    walkerParam.curbeResourceList      = m_curbeResourceList.data();
    walkerParam.curbeResourceListSize  = m_curbeResourceList.size();
    walkerParam.inlineResourceList     = m_inlineResourceList.data();
    walkerParam.inlineResourceListSize = m_inlineResourceList.size();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetSamplerStates(KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup)
{
    VP_FUNC_CALL();
    return MOS_STATUS_SUCCESS;
}

void VpRenderKernelObj::OcaDumpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext)
{
    HalOcaInterfaceNext::DumpVpKernelInfo(cmdBuffer, (MOS_CONTEXT_HANDLE)&mosContext, m_kernelId, 0, nullptr);
}

MOS_STATUS VpRenderKernelObj::InitRenderHalSurfaceCMF(MOS_SURFACE* src, PRENDERHAL_SURFACE renderHalSurface)
{
    PMOS_INTERFACE        osInterface = m_hwInterface->m_osInterface;
    VP_RENDER_CHK_NULL_RETURN(osInterface);
#if !EMUL
    PGMM_RESOURCE_INFO pGmmResourceInfo;
    pGmmResourceInfo = (GMM_RESOURCE_INFO *)src->OsResource.pGmmResInfo;
    MOS_OS_CHK_NULL_RETURN(pGmmResourceInfo);

    GMM_RESOURCE_FORMAT gmmResFmt;
    gmmResFmt = pGmmResourceInfo->GetResourceFormat();
    uint32_t          MmcFormat = 0;

    MmcFormat = static_cast<uint32_t>(osInterface->pfnGetGmmClientContext(osInterface)->GetMediaSurfaceStateCompressionFormat(gmmResFmt));

    if (MmcFormat > 0x1F)
    {
        MOS_OS_ASSERTMESSAGE("Get a incorrect Compression format(%d) from GMM", MmcFormat);
    }
    else
    {
        renderHalSurface->OsSurface.CompressionFormat = MmcFormat;
        MOS_OS_VERBOSEMESSAGE("Render Enigien compression format %d", MmcFormat);
    }
#endif
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderKernelObj::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetTuningFlag(PKERNEL_TUNING_PARAMS tuningParams)
{
    VP_FUNC_CALL();
    m_kernelTuningParams = tuningParams;
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS VpRenderKernelObj::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelStatefulSurfaces(KERNEL_ARG_INDEX_SURFACE_MAP &statefulSurfaces)
{
    m_argIndexSurfMap = statefulSurfaces;
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderKernelObj::SetKernelConfigs(
    KERNEL_PARAMS& kernelParams,
    VP_SURFACE_GROUP& surfaces,
    KERNEL_SAMPLER_STATE_GROUP& samplerStateGroup,
    KERNEL_CONFIGS& kernelConfigs,
    VP_PACKET_SHARED_CONTEXT* sharedContext)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(SetKernelConfigs(kernelConfigs));

    VP_RENDER_CHK_STATUS_RETURN(SetKernelArgs(kernelParams.kernelArgs, sharedContext));

    VP_RENDER_CHK_STATUS_RETURN(SetKernelStatefulSurfaces(kernelParams.kernelStatefulSurfaces));

    VP_RENDER_CHK_STATUS_RETURN(SetProcessSurfaceGroup(surfaces));

    // when UseIndependentSamplerGroup is true, each kernel will set their own sampler state group in VpRenderCmdPacket::SetupSamplerStates()
    if (!UseIndependentSamplerGroup())
    {
        VP_RENDER_CHK_STATUS_RETURN(SetSamplerStates(samplerStateGroup));
    }

    VP_RENDER_CHK_STATUS_RETURN(SetWalkerSetting(kernelParams.kernelThreadSpace, kernelParams.syncFlag,kernelParams.flushL1));

    VP_RENDER_CHK_STATUS_RETURN(SetTuningFlag(&kernelParams.kernelTuningParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs)
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::InitKernel(void* binary, uint32_t size, KERNEL_CONFIGS& kernelConfigs,
                                        VP_SURFACE_GROUP& surfacesGroup, VP_RENDER_CACHE_CNTL& surfMemCacheCtl)
{
    VP_FUNC_CALL();

    if (kernelCombinedFc != m_kernelId)
    {
        VP_RENDER_CHK_NULL_RETURN(binary);
    }
    // m_kernelBinary and m_kernelSize being nullptr and 0 for FC case.
    m_kernelBinary = binary;
    m_kernelSize = size;
    m_curbeResourceList.clear();
    m_inlineResourceList.clear();
    m_curbeLocation = {};
    SetCacheCntl(&surfMemCacheCtl);
    VP_RENDER_CHK_STATUS_RETURN(SetKernelConfigs(kernelConfigs));
    VP_RENDER_CHK_STATUS_RETURN(SetProcessSurfaceGroup(surfacesGroup));

    VP_RENDER_NORMALMESSAGE("Kernel %d is in use.", m_kernelId);
    MT_LOG1(MT_VP_KERNEL_Init, MT_NORMAL, MT_VP_KERNEL_ID, m_kernelId);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::CpPrepareResources()
{
    VP_RENDER_NORMALMESSAGE("Not prepare reousces for CP in kernel %d.", m_kernelId);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetProcessSurfaceGroup(VP_SURFACE_GROUP &surfaces)
{
    m_surfaceGroup = &surfaces;
    VP_RENDER_CHK_STATUS_RETURN(InitBindlessResources());
    VP_RENDER_CHK_STATUS_RETURN(SetupSurfaceState());
    VP_RENDER_CHK_STATUS_RETURN(CpPrepareResources());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS GetSurfaceSize(
    VP_SURFACE    *pSurface,
    uint32_t       iBpp,
    uint32_t      *piWidthInBytes,
    uint32_t      *piHeightInRows)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus;
    uint32_t   iWidthInBytes;
    uint32_t   iHeightInRows;

    //-------------------------------------------
    VP_DEBUG_ASSERT(pSurface->osSurface->dwWidth >= 1);
    VP_DEBUG_ASSERT(pSurface->osSurface->dwHeight >= 1);
    VP_DEBUG_ASSERT(pSurface->osSurface->dwPitch >= 1);
    //-------------------------------------------

    eStatus = MOS_STATUS_SUCCESS;

    switch (pSurface->osSurface->Format)
    {
    // Packed Formats
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_R5G6B5:
    case Format_R8G8B8:
    case Format_R32U:
    case Format_R32F:
    case Format_AYUV:
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_AI44:
    case Format_IA44:
    case Format_P8:
    case Format_A8P8:
    case Format_A8:
    case Format_L8:
    case Format_A4L4:
    case Format_A8L8:
    case Format_V8U8:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
    case Format_Y410:
    case Format_Y416:
    case Format_Y210:
    case Format_Y216:
    case Format_R16F:
        iWidthInBytes = pSurface->osSurface->dwWidth * iBpp / 8;
        iHeightInRows = pSurface->osSurface->dwHeight;
        break;

    // 4:2:0 (12-bits per pixel)
    // IMC1                           // IMC3
    // ----------------->             // ----------------->
    // ________________________       // ________________________
    //|Y0|Y1|                  |      //|Y0|Y1|                  |
    //|__|__|                  |      //|__|__|                  |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|________________________|      //|________________________|
    //|V0|V1|      |           |      //|U0|U1|      |           |
    //|__|__|      |           |      //|__|__|      |           |
    //|            |           |      //|            |           |
    //|____________|  PAD      |      //|____________|  PAD      |
    //|U0|U1|      |           |      //|V0|V1|      |           |
    //|__|__|      |           |      //|__|__|      |           |
    //|            |           |      //|            |           |
    //|____________|___________|      //|____________|___________|
    case Format_IMC1:
    case Format_IMC3:
        iWidthInBytes = pSurface->osSurface->dwWidth;
        iHeightInRows = pSurface->osSurface->dwHeight * 2;
        break;

    // 4:0:0 (8-bits per pixel)
    // 400P
    // ----------------->
    // ________________________
    //|Y0|Y1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    case Format_400P:
    case Format_Buffer:
    case Format_RAW:
        iWidthInBytes = pSurface->osSurface->dwWidth;
        iHeightInRows = pSurface->osSurface->dwHeight;
        break;

    // 4:1:1 (12-bits per pixel)      // 4:2:2 (16-bits per pixel)
    // 411P                           // 422H
    // ----------------->             // ----------------->
    // ________________________       // ________________________
    //|Y0|Y1|                  |      //|Y0|Y1|                  |
    //|__|__|                  |      //|__|__|                  |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|                        |      //|                        |
    //|________________________|      //|________________________|
    //|U0|U1||                 |      //|U0|U1|      |           |
    //|__|__||                 |      //|__|__|      |           |
    //|      |                 |      //|            |           |
    //|      |      PAD        |      //|            |    PAD    |
    //|      |                 |      //|            |           |
    //|      |                 |      //|            |           |
    //|      |                 |      //|            |           |
    //|______|_________________|      //|____________|___________|
    //|V0|V1||                 |      //|V0|V1|      |           |
    //|__|__||                 |      //|__|__|      |           |
    //|      |                 |      //|            |           |
    //|      |      PAD        |      //|            |    PAD    |
    //|      |                 |      //|            |           |
    //|      |                 |      //|            |           |
    //|      |                 |      //|            |           |
    //|______|_________________|      //|____________|___________|

    // 4:4:4 (24-bits per pixel)
    // 444P
    // ----------------->
    // ________________________
    //|Y0|Y1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|U0|U1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|V0|V1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|

    // 4:4:4 (24-bits per pixel)
    // RGBP
    // ----------------->
    // ________________________
    //|R0|R1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|G0|G1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|B0|B1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    case Format_RGBP:

    // 4:4:4 (24-bits per pixel)
    // BGRP
    // ----------------->
    // ________________________
    //|B0|B1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|G0|G1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|R0|R1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    case Format_BGRP:
    case Format_411P:
    case Format_422H:
    case Format_444P:
        iWidthInBytes = pSurface->osSurface->dwWidth;
        iHeightInRows = pSurface->osSurface->dwHeight * 3;
        break;

    // 4:1:1 (12-bits per pixel)
    // 411R
    // ----------------->
    // ________________________
    //|Y0|Y1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|U0|U1|                  |
    //|__|__|                  |
    //|________________________|
    //|V0|V1|                  |
    //|__|__|                  |
    //|________________________|
    case Format_411R:
        iWidthInBytes = pSurface->osSurface->dwWidth;
        iHeightInRows = (pSurface->osSurface->dwHeight * 3) / 2;
        break;

    // 4:2:2V (16-bits per pixel)
    // 422V
    // ----------------->
    // ________________________
    //|Y0|Y1|                  |
    //|__|__|                  |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|                        |
    //|________________________|
    //|U0|U1|                  |
    //|__|__|                  |
    //|                        |
    //|________________________|
    //|V0|V1|                  |
    //|__|__|                  |
    //|                        |
    //|________________________|
    case Format_422V:
        iWidthInBytes = pSurface->osSurface->dwWidth;
        iHeightInRows = pSurface->osSurface->dwHeight * 2;
        break;

        // 4:2:0 (12-bits per pixel)
        // IMC2                          // IMC4
        // ----------------->            // ----------------->
        // ________________________      // ________________________
        //|Y0|Y1|                  |     //|Y0|Y1|                  |
        //|__|__|                  |     //|__|__|                  |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|________________________|     //|________________________|
        //|V0|V1|      |U0|U1|     |     //|U0|U1|      |V0|V1|     |
        //|__|__|      |__|__|     |     //|__|__|      |__|__|     |
        //|            |           |     //|            |           |
        //|____________|___________|     //|____________|___________|

        // NV12                          // YV12
        // ----------------->            // ----------------->
        // ________________________      // ________________________
        //|Y0|Y1|                  |     //|Y0|Y1|                  |
        //|__|__|                  |     //|__|__|                  |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|                        |     //|                        |
        //|________________________|     //|________________________|
        //|U0|V0|U1|V1|            |     //|V0|V1|                  |
        //|__|__|__|__|            |     //|__|__|__________________|
        //|                        |     //|U0|U1|                  |
        //|________________________|     //|__|__|__________________|

    case Format_IMC2:
    case Format_IMC4:
    case Format_NV12:
    case Format_YV12:
    case Format_I420:
    case Format_IYUV:
    case Format_YVU9:
        iWidthInBytes = pSurface->osSurface->dwWidth;
        iHeightInRows = pSurface->osSurface->dwHeight * iBpp / 8;
        break;

    case Format_P010:
    case Format_P016:
        iWidthInBytes = pSurface->osSurface->dwWidth * 2;
        iHeightInRows = pSurface->osSurface->dwHeight * 3 / 2;
        break;

    case Format_A16R16G16B16:
    case Format_A16B16G16R16:
        iWidthInBytes = pSurface->osSurface->dwWidth * 8;
        iHeightInRows = pSurface->osSurface->dwHeight;
        break;

    case Format_P210:
    case Format_P216:
        iWidthInBytes = pSurface->osSurface->dwWidth * 2;
        iHeightInRows = pSurface->osSurface->dwHeight * 2;
        break;
    default:
        VP_RENDER_ASSERTMESSAGE("Format %d not supported.", pSurface->osSurface->Format);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    *piWidthInBytes = iWidthInBytes;
    *piHeightInRows = iHeightInRows;

finish:
    return eStatus;
}

void VpRenderKernelObj::DumpSurface(VP_SURFACE* pSurface, PCCHAR fileName)
{
    uint8_t* pData;
    char                    sPath[MAX_PATH];
    uint8_t* pDst;
    uint8_t* pTmpDst;
    uint8_t* pTmpSrc;
    uint32_t                iWidthInBytes = 0;
    uint32_t                iHeightInRows = 0;
    uint32_t                iBpp = 0;
    uint32_t                iSize = 0;
    uint32_t                iY = 0;
    MOS_LOCK_PARAMS         LockFlags;

    VP_FUNC_CALL();
#if !EMUL
    PMOS_INTERFACE        pOsInterface = m_hwInterface->m_osInterface;

    pDst = nullptr;
    MOS_ZeroMemory(sPath, MAX_PATH);

    // get bits per pixel for the format
    pOsInterface->pfnGetBitsPerPixel(pOsInterface, pSurface->osSurface->Format, &iBpp);

    GetSurfaceSize(
        pSurface,
        iBpp,
        &iWidthInBytes,
        &iHeightInRows);

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
        "c:\\dump\\f[%08d]_%s_w[%d]_h[%d]_p[%d].%s",
        1,
        fileName,
        pSurface->osSurface->dwWidth,
        pSurface->osSurface->dwHeight,
        pSurface->osSurface->dwPitch,
        VP_GET_FORMAT_STRING(pSurface->osSurface->Format));

    // Write the data to file
    if (pSurface->osSurface->dwPitch == iWidthInBytes)
    {
        MosUtilities::MosWriteFileFromPtr((const char*)sPath, pData, iSize);
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

        MosUtilities::MosWriteFileFromPtr((const char*)sPath, pDst, iSize);
    }

    if (pDst)
    {
        MOS_FreeMemory(pDst);
    }

    MOS_STATUS status = m_allocator->UnLock(&pSurface->osSurface->OsResource);
    if (MOS_FAILED(status))
    {
        VP_RENDER_ASSERTMESSAGE("Unlock resource failed!");
    }
#endif
}

MOS_STATUS VpRenderKernelObj::UpdateCurbeStateHeapInfo(PMOS_RESOURCE stateHeap, uint8_t *statePtr, uint32_t offset)
{
    VP_RENDER_CHK_NULL_RETURN(stateHeap);
    VP_RENDER_CHK_NULL_RETURN(statePtr);

    m_curbeLocation.offset    = offset;
    m_curbeLocation.stateHeap = stateHeap;
    m_curbeLocation.statePtr  = statePtr;

    for (MHW_INDIRECT_STATE_RESOURCE_PARAMS &resourceParam : m_curbeResourceList)
    {
        resourceParam.stateHeap    = stateHeap;
        resourceParam.stateBasePtr = statePtr;
        resourceParam.stateOffset += offset;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetInlineDataParameter(KRN_ARG arg, uint8_t* inlineData)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(inlineData);
    if (arg.implicitArgType == IndirectDataPtr)
    {
        VP_RENDER_CHK_NULL_RETURN(m_curbeLocation.stateHeap);
        MHW_INDIRECT_STATE_RESOURCE_PARAMS params = {};
        params.isWrite                            = false;
        params.resource                           = m_curbeLocation.stateHeap;
        params.resourceOffset                     = m_curbeLocation.offset;
        params.stateOffset                        = arg.uOffsetInPayload;
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
        params.dumpName        = "KERNEL_CURBE";
        params.needDump        = true;
        params.dumpSize        = m_curbeLocation.size;
        params.resourceBasePtr = m_curbeLocation.statePtr;
#endif
        m_inlineResourceList.push_back(params);
        VP_RENDER_NORMALMESSAGE("Setting Indirect State Data Inline Data KernelID %d, index %d , argKind %d", m_kernelId, arg.uIndex, arg.eArgKind);
    }
    else if (arg.implicitArgType == ValueType)
    {
        if (arg.pData != nullptr)
        {
            MOS_SecureMemcpy(inlineData + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
            VP_RENDER_NORMALMESSAGE("Setting Inline Data KernelID %d, index %d , value %d, argKind %d", m_kernelId, arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
        }
        else
        {
            VP_RENDER_NORMALMESSAGE("KernelID %d, index %d, argKind %d is empty", m_kernelId, arg.uIndex, arg.eArgKind);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetBindlessSamplerToResourceList(KRN_ARG &arg, uint32_t samplerIndex)
{
    auto it = m_bindlessSamperArray.find(samplerIndex);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(it, &m_bindlessSamperArray);
    RENDERHAL_STATE_LOCATION &samplerStateLocation = it->second;
    VP_PUBLIC_CHK_NULL_RETURN(samplerStateLocation.stateHeap);
    
    MHW_INDIRECT_STATE_RESOURCE_PARAMS params = {};
    params.isWrite                            = false;
    params.resource                           = samplerStateLocation.stateHeap;
    params.resourceOffset                     = samplerStateLocation.offset;
    params.stateOffset                        = arg.uOffsetInPayload;
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    params.dumpName        = "KERNEL_SAMPLER";
    params.needDump        = true;
    params.dumpSize        = samplerStateLocation.size;
    params.resourceBasePtr = samplerStateLocation.statePtr;
#endif
    m_curbeResourceList.push_back(params);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetBindlessSurfaceStateToResourceList(KRN_ARG &arg)
{
    auto surfMapHandle = m_argIndexSurfMap.find(arg.uIndex);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(surfMapHandle, &m_argIndexSurfMap);
    if (surfMapHandle->second.surfType == SurfaceTypeInvalid)
    {
        //for invalid surface type, it means the surface in not used in this senario
        return MOS_STATUS_SUCCESS;
    }
    auto bindlessAddressHandle = m_bindlessSurfaceArray.find(surfMapHandle->second.surfType);
    VP_PUBLIC_CHK_NOT_FOUND_RETURN(bindlessAddressHandle, &m_bindlessSurfaceArray);
    if (surfMapHandle->second.planeIndex >= bindlessAddressHandle->second.size())
    {
        //for those surfaces plane number less than kernel interface max, skip these sub planes
        return MOS_STATUS_SUCCESS;
    }
    RENDERHAL_STATE_LOCATION& surfStateLocation = bindlessAddressHandle->second.at(surfMapHandle->second.planeIndex);
    VP_PUBLIC_CHK_NULL_RETURN(surfStateLocation.stateHeap);

    MHW_INDIRECT_STATE_RESOURCE_PARAMS params = {};
    params.isWrite                            = false;
    params.resource                           = surfStateLocation.stateHeap;
    params.resourceOffset                     = surfStateLocation.offset;     //this is the offset of surface state in SurfStateHep
    params.stateOffset                        = arg.uOffsetInPayload;   //this is the offset of curbe state in GSH
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    params.dumpName        = "BINDLESS_SURFACE_STATE";
    params.needDump        = true;
    params.dumpSize        = surfStateLocation.size;
    params.resourceBasePtr = surfStateLocation.statePtr;
#endif
    m_curbeResourceList.push_back(params);
    
    return MOS_STATUS_SUCCESS;
}

bool VpRenderKernelObj::IsLocalIdGeneratedByRuntime(KRN_EXECUTE_ENV &krnEnv, KRN_PER_THREAD_ARG_INFO &perThreadInfo, uint32_t localWidth, uint32_t localHeight, uint32_t localDepth)
{
    return (krnEnv.uiSlmSize > 0 &&
            krnEnv.uSimdSize == 1 &&
            perThreadInfo.packedLocalIdSize > 0 &&
            (localWidth > 1 || localHeight > 1 || localDepth > 1));
}

MOS_STATUS VpRenderKernelObj::PaddingPerThreadCurbe(uint32_t& curbeSize, uint32_t localWidth, uint32_t localHeight, uint32_t localDepth)
{
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_PUBLIC_CHK_VALUE_RETURN(m_hwInterface->m_renderHal->grfSize > 0, true);

    curbeSize = MOS_ALIGN_CEIL(curbeSize, 32);
    uint32_t localSize = MOS_MAX(localWidth, 1) * MOS_MAX(localHeight, 1) * MOS_MAX(localDepth, 1);  //this is the local thread number
    curbeSize += localSize * m_hwInterface->m_renderHal->grfSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderKernelObj::SetPerThreadCurbe(uint8_t *curbe, uint32_t offset, uint32_t curbeSize, KRN_PER_THREAD_ARG_INFO &perThreadInfo, uint32_t localWidth, uint32_t localHeight, uint32_t localDepth)
{
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_PUBLIC_CHK_VALUE_RETURN(m_hwInterface->m_renderHal->grfSize > 0, true);

    uint32_t totalSize     = MOS_ALIGN_CEIL(offset, 32) + perThreadInfo.packedLocalIdOffset + perThreadInfo.packedLocalIdSize;
    uint8_t *perThreadData = curbe + MOS_ALIGN_CEIL(offset, 32) + perThreadInfo.packedLocalIdOffset;
    
    for (uint16_t threadX = 0; threadX < MOS_MAX(localWidth, 1); ++threadX)
    {
        for (uint16_t threadY = 0; threadY < MOS_MAX(localHeight, 1); ++threadY)
        {
            for (uint16_t threadZ = 0; threadZ < MOS_MAX(localDepth, 1); ++threadZ)
            {
                VP_PUBLIC_CHK_VALUE_RETURN(totalSize <= curbeSize, true);
                reinterpret_cast<uint16_t *>(perThreadData)[0] = threadX;
                reinterpret_cast<uint16_t *>(perThreadData)[1] = threadY;
                reinterpret_cast<uint16_t *>(perThreadData)[2] = threadZ;
                perThreadData += m_hwInterface->m_renderHal->grfSize;
                totalSize += m_hwInterface->m_renderHal->grfSize;
                VP_RENDER_NORMALMESSAGE("Setting Per Thread Data X %d, Y %d, Z %d", threadX, threadY, threadZ);
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}