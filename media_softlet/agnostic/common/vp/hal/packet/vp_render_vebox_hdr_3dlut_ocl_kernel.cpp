/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_render_vebox_hdr_3dlut_ocl_kernel.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_render_vebox_hdr_3dlut_ocl_kernel.h"
#include "vp_dumper.h"
#include "vp_kernelset.h"

using namespace vp;

static const float ccm_identity[12]               = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f};
static float       color_matrix_calculation[3][4] = {0.0f};

template <typename T>
void mat_3by3_inv(T *m1, T *m2)
{
    double a11, a12, a13, a21, a22, a23, a31, a32, a33;
    double det, det_inv;

    a11 = *m1;
    a12 = *(m1 + 1);
    a13 = *(m1 + 2);
    a21 = *(m1 + 3);
    a22 = *(m1 + 4);
    a23 = *(m1 + 5);
    a31 = *(m1 + 6);
    a32 = *(m1 + 7);
    a33 = *(m1 + 8);

    det = a11 * (a33 * a22 - a32 * a23) - a21 * (a33 * a12 - a32 * a13) + a31 * (a12 * a23 - a13 * a22);

    det_inv = 1 / det;

    *m2       = (float)(det_inv * (a33 * a22 - a32 * a23));
    *(m2 + 1) = (float)(-det_inv * (a33 * a12 - a32 * a13));
    *(m2 + 2) = (float)(det_inv * (a12 * a23 - a13 * a22));
    *(m2 + 3) = (float)(-det_inv * (a33 * a21 - a31 * a23));
    *(m2 + 4) = (float)(det_inv * (a33 * a11 - a31 * a13));
    *(m2 + 5) = (float)(-det_inv * (a23 * a11 - a21 * a13));
    *(m2 + 6) = (float)(det_inv * (a32 * a21 - a31 * a22));
    *(m2 + 7) = (float)(-det_inv * (a32 * a11 - a31 * a12));
    *(m2 + 8) = (float)(det_inv * (a22 * a11 - a21 * a12));
}

template <typename T>
void mat_mul_float(T *m1, T *m2, T *m3, short m_size, short n_size, short l_size)
{
    int    i, j, k, Tjn, Tjl, off3;
    float *m1_offs, *m2_offs, *m2_k_offs, *m3_offs;

    for (j = 0; j < m_size; j++)
    {
        Tjn = j * n_size;
        Tjl = j * l_size;
        for (k = 0; k < l_size; k++)
        {
            off3      = Tjl + k;
            m3_offs   = m3 + off3;
            *m3_offs  = 0;
            m2_k_offs = m2 + k;
            m1_offs   = m1 + Tjn;
            for (i = 0; i < n_size; i++)
            {
                m2_offs = m2_k_offs + i * l_size;
                *m3_offs += *m1_offs * *m2_offs;
                m1_offs++;
            }
        }
    }
}
static void CalcCCMMatrix()
{
    float mat_rgb2xyz_src[3][3], mat_rgb2xyz_dst[3][3];
    float mat_xyz[3][3], mat_invxyz[3][3];
    float matW[3];
    float matE[3] = {};

    float fccmSrcZR, fccmSrcZG, fccmSrcZB, fccmSrcZW;
    float fccmDstZR, fccmDstZG, fccmDstZB, fccmDstZW;

    const float fccmSrcXR = 0.708000000000000f;
    const float fccmSrcYR = 0.292000000000000f;
    const float fccmSrcXG = 0.170000000000000f;
    const float fccmSrcYG = 0.797000000000000f;
    const float fccmSrcXB = 0.131000000000000f;
    const float fccmSrcYB = 0.046000000000000f;
    const float fccmSrcXW = 0.312700000000000f;
    const float fccmSrcYW = 0.329000000000000f;
    const float fccmDstXR = 0.640000000000000f;
    const float fccmDstYR = 0.330000000000000f;
    const float fccmDstXG = 0.300000000000000f;
    const float fccmDstYG = 0.600000000000000f;
    const float fccmDstXB = 0.150000000000000f;
    const float fccmDstYB = 0.060000000000000f;
    const float fccmDstXW = 0.312700000000000f;
    const float FccmDstYW = 0.329000000000000f;

    fccmSrcZR = 1 - fccmSrcXR - fccmSrcYR;
    fccmSrcZG = 1 - fccmSrcXG - fccmSrcYG;
    fccmSrcZB = 1 - fccmSrcXB - fccmSrcYB;
    fccmSrcZW = 1 - fccmSrcXW - fccmSrcYW;

    mat_xyz[0][0] = fccmSrcXR;
    mat_xyz[0][1] = fccmSrcXG;
    mat_xyz[0][2] = fccmSrcXB;
    mat_xyz[1][0] = fccmSrcYR;
    mat_xyz[1][1] = fccmSrcYG;
    mat_xyz[1][2] = fccmSrcYB;
    mat_xyz[2][0] = fccmSrcZR;
    mat_xyz[2][1] = fccmSrcZG;
    mat_xyz[2][2] = fccmSrcZB;

    mat_3by3_inv(mat_xyz[0], mat_invxyz[0]);

    matW[0] = fccmSrcXW / fccmSrcYW;
    matW[1] = 1;
    matW[2] = fccmSrcZW / fccmSrcYW;

    mat_mul_float(mat_invxyz[0], matW, matE, 3, 3, 1);

    mat_rgb2xyz_src[0][0] = matE[0] * fccmSrcXR;
    mat_rgb2xyz_src[1][0] = matE[0] * fccmSrcYR;
    mat_rgb2xyz_src[2][0] = matE[0] * fccmSrcZR;
    mat_rgb2xyz_src[0][1] = matE[1] * fccmSrcXG;
    mat_rgb2xyz_src[1][1] = matE[1] * fccmSrcYG;
    mat_rgb2xyz_src[2][1] = matE[1] * fccmSrcZG;
    mat_rgb2xyz_src[0][2] = matE[2] * fccmSrcXB;
    mat_rgb2xyz_src[1][2] = matE[2] * fccmSrcYB;
    mat_rgb2xyz_src[2][2] = matE[2] * fccmSrcZB;

    fccmDstZR = 1 - fccmDstXR - fccmDstYR;
    fccmDstZG = 1 - fccmDstXG - fccmDstYG;
    fccmDstZB = 1 - fccmDstXB - fccmDstYB;
    fccmDstZW = 1 - fccmDstXW - FccmDstYW;

    mat_xyz[0][0] = fccmDstXR;
    mat_xyz[0][1] = fccmDstXG;
    mat_xyz[0][2] = fccmDstXB;
    mat_xyz[1][0] = fccmDstYR;
    mat_xyz[1][1] = fccmDstYG;
    mat_xyz[1][2] = fccmDstYB;
    mat_xyz[2][0] = fccmDstZR;
    mat_xyz[2][1] = fccmDstZG;
    mat_xyz[2][2] = fccmDstZB;

    mat_3by3_inv(mat_xyz[0], mat_invxyz[0]);

    matW[0] = fccmDstXW / FccmDstYW;
    matW[1] = 1;
    matW[2] = fccmDstZW / FccmDstYW;

    mat_mul_float(mat_invxyz[0], matW, matE, 3, 3, 1);

    mat_rgb2xyz_dst[0][0] = matE[0] * fccmDstXR;
    mat_rgb2xyz_dst[1][0] = matE[0] * fccmDstYR;
    mat_rgb2xyz_dst[2][0] = matE[0] * fccmDstZR;
    mat_rgb2xyz_dst[0][1] = matE[1] * fccmDstXG;
    mat_rgb2xyz_dst[1][1] = matE[1] * fccmDstYG;
    mat_rgb2xyz_dst[2][1] = matE[1] * fccmDstZG;
    mat_rgb2xyz_dst[0][2] = matE[2] * fccmDstXB;
    mat_rgb2xyz_dst[1][2] = matE[2] * fccmDstYB;
    mat_rgb2xyz_dst[2][2] = matE[2] * fccmDstZB;

    float mat_invrgb2xyz_dst[3][3];
    mat_3by3_inv(mat_rgb2xyz_dst[0], mat_invrgb2xyz_dst[0]);

    float CCMmat[3][3];
    mat_mul_float(mat_invrgb2xyz_dst[0], mat_rgb2xyz_src[0], CCMmat[0], 3, 3, 3);

    color_matrix_calculation[0][0] = CCMmat[0][0];
    color_matrix_calculation[0][1] = CCMmat[0][1];
    color_matrix_calculation[0][2] = CCMmat[0][2];
    color_matrix_calculation[0][3] = 0.0f;
    color_matrix_calculation[1][0] = CCMmat[1][0];
    color_matrix_calculation[1][1] = CCMmat[1][1];
    color_matrix_calculation[1][2] = CCMmat[1][2];
    color_matrix_calculation[1][3] = 0.0f;
    color_matrix_calculation[2][0] = CCMmat[2][0];
    color_matrix_calculation[2][1] = CCMmat[2][1];
    color_matrix_calculation[2][2] = CCMmat[2][2];
    color_matrix_calculation[2][3] = 0.0f;
}

// Used by OCL kernel
VpRenderHdr3DLutOclKernel::VpRenderHdr3DLutOclKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    VpRenderKernelObj(hwInterface, (VpKernelID)kernelHdr3DLutCalcOcl, 0, VP_HDR_KERNEL_NAME_OCL_3DLUT, allocator)
{
    VP_FUNC_CALL();
    m_kernelBinaryID = VP_ADV_KERNEL_BINARY_ID(kernelHdr3DLutCalcOcl);
    m_isAdvKernel    = true;
}

VpRenderHdr3DLutOclKernel::~VpRenderHdr3DLutOclKernel()
{
    MOS_SafeFreeMemory(m_curbe);
    m_curbe = nullptr;
}

MOS_STATUS VpRenderHdr3DLutOclKernel::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();

    VP_RENDER_NORMALMESSAGE("Initializing SR krn %s", kernel.GetKernelName().c_str());

    m_kernelSize = kernel.GetKernelSize();

    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();

    m_kernelArgs = kernel.GetKernelArgs();

    for (auto arg : m_kernelArgs)
    {
        arg.pData = nullptr;
    }

    m_kernelBtis = kernel.GetKernelBtis();

    m_kernelEnv = kernel.GetKernelExeEnv();

    m_curbeSize = kernel.GetCurbeSize();

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS VpRenderHdr3DLutOclKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_STATUS_RETURN(InitCoefSurface(m_maxDisplayLum, m_maxContentLevelLum, m_hdrMode));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutOclKernel::CpPrepareResources()
{
    VP_FUNC_CALL();

    PMOS_RESOURCE source[VPHAL_MAX_SOURCES] = {nullptr};
    PMOS_RESOURCE target[VPHAL_MAX_TARGETS] = {nullptr};

    if ((nullptr != m_hwInterface->m_osInterface) &&
        (nullptr != m_hwInterface->m_osInterface->osCpInterface))
    {
        auto        it   = m_surfaceGroup->find(SurfaceType3DLutCoef);
        VP_SURFACE *surf = (m_surfaceGroup->end() != it) ? it->second : nullptr;
        VP_RENDER_CHK_NULL_RETURN(surf);
        source[0] = &(surf->osSurface->OsResource);

        it   = m_surfaceGroup->find(SurfaceType3DLut);
        surf = (m_surfaceGroup->end() != it) ? it->second : nullptr;
        VP_RENDER_CHK_NULL_RETURN(surf);

        target[0] = &(surf->osSurface->OsResource);
        m_hwInterface->m_osInterface->osCpInterface->PrepareResources((void **)source, 1, (void **)target, 1);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutOclKernel::SetupStatelessBuffer()
{
    VP_FUNC_CALL();
    m_statelessArray.clear();
    VP_RENDER_CHK_STATUS_RETURN(SetupStatelessBufferResource(SurfaceType3DLutCoef));
    VP_RENDER_CHK_STATUS_RETURN(SetupStatelessBufferResource(SurfaceType3DLut));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutOclKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = m_curbeSize;

    VP_RENDER_NORMALMESSAGE("KernelID %d, Curbe Size %d\n", m_kernelId, curbeLength);
    if (curbeLength == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint8_t *pCurbe = (uint8_t *)MOS_AllocAndZeroMemory(curbeLength);
    VP_RENDER_CHK_NULL_RETURN(pCurbe);
    MOS_FreeMemAndSetNull(m_curbe);
    m_curbe = pCurbe;

    for (auto &arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_GENERAL)
        {
            if (arg.pData != nullptr)
            {
                MOS_SecureMemcpy(pCurbe + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
                VP_RENDER_NORMALMESSAGE("Setting Curbe State KernelID %d, index %d , value %d, argKind %d", m_kernelId, arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("KernelID %d, index %d, argKind %d is empty", m_kernelId, arg.uIndex, arg.eArgKind);
            }
        }
        else if (arg.eArgKind == ARG_KIND_SURFACE)
        {
            if (arg.addressMode == AddressingModeStateless && arg.pData != nullptr)
            {
                for (uint32_t idx = 0; idx < arg.uSize / sizeof(SurfaceType); idx++)
                {
                    uint32_t   *pSurfaceindex  = (uint32_t *)(arg.pData) + idx;
                    SurfaceType surf           = (SurfaceType)*pSurfaceindex;

                    if (surf != SurfaceTypeInvalid)
                    {
                        auto it = m_statelessArray.find(surf);
                        uint64_t ui64GfxAddress                              = (m_statelessArray.end() != it) ? it->second : 0xFFFF;
                        *((uint64_t *)(pCurbe + arg.uOffsetInPayload) + idx) = ui64GfxAddress;
                        break;
                    }
                    else
                    {
                        *((uint64_t *)(pCurbe + arg.uOffsetInPayload) + idx) = 0xFFFF;
                    }
                }
            }
        }
        else if (arg.eArgKind == ARG_KIND_INLINE)
        {
            VP_RENDER_NORMALMESSAGE("Skip inline data here");
        }
        else
        {
            return MOS_STATUS_UNIMPLEMENTED;
        }
    }

    curbe = pCurbe;

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS VpRenderHdr3DLutOclKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{

    VP_FUNC_CALL();

    walkerParam = m_walkerParam;
    walkerParam.iBindingTable = renderData.bindingTable;
    walkerParam.iMediaID      = renderData.mediaID;
    walkerParam.iCurbeOffset  = renderData.iCurbeOffset;
    // Should use renderData.iCurbeLength instead of kernelSettings.CURBE_Length.
    // kernelSettings.CURBE_Length is 32 aligned with 5 bits shift.
    // renderData.iCurbeLength is RENDERHAL_CURBE_BLOCK_ALIGN(64) aligned.
    walkerParam.iCurbeLength = renderData.iCurbeLength;
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderHdr3DLutOclKernel::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
{
    VP_FUNC_CALL();
    MOS_ZeroMemory(&m_walkerParam, sizeof(KERNEL_WALKER_PARAMS));

    m_walkerParam.iBlocksX          = threadSpace.uWidth;
    m_walkerParam.iBlocksY          = threadSpace.uHeight;
    m_walkerParam.threadWidth       = threadSpace.uLocalWidth;
    m_walkerParam.threadHeight      = threadSpace.uLocalHeight;
    m_walkerParam.threadDepth       = 1;
    m_walkerParam.isVerticalPattern = false;
    m_walkerParam.bSyncFlag         = bSyncFlag;

    m_walkerParam.pipeControlParams.bUpdateNeeded              = true;
    m_walkerParam.pipeControlParams.bEnableDataPortFlush       = true;
    m_walkerParam.pipeControlParams.bUnTypedDataPortCacheFlush = true;
    m_walkerParam.pipeControlParams.bFlushRenderTargetCache    = false;
    m_walkerParam.pipeControlParams.bInvalidateTextureCache    = false;

    for (auto &arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_INLINE)
        {
            if (arg.pData != nullptr)
            {
                MOS_SecureMemcpy(m_inlineData + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
                VP_RENDER_NORMALMESSAGE("Setting Inline Data KernelID %d, index %d , value %d, argKind %d", m_kernelId, arg.uIndex, *(uint32_t *)arg.pData, arg.eArgKind);
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("KernelID %d, index %d, argKind %d is empty", m_kernelId, arg.uIndex, arg.eArgKind);
            }
        }
    }
    m_walkerParam.inlineDataLength = sizeof(m_inlineData);
    m_walkerParam.inlineData       = m_inlineData;

    if (m_kernelEnv.uSimdSize != 1)
    {
        m_walkerParam.isEmitInlineParameter = true;
        m_walkerParam.isGenerateLocalID     = true;
        m_walkerParam.emitLocal             = MHW_EMIT_LOCAL_XYZ;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutOclKernel::InitCoefSurface(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode)
{
    VP_FUNC_CALL();
    float  *hdrcoefBuffer = nullptr;
    int32_t oetfCurve = 0, tmMode = 0, tmSrcType = 0;
    float   *ccmMatrix = m_ccmMatrix;
    float   tmMaxCLL = 0.0f, tmMaxDLL = 0.0f;

    MOS_ZeroMemory(m_ccmMatrix, sizeof(m_ccmMatrix));

    // Get surface addr
    auto        it   = m_surfaceGroup->find(SurfaceType3DLutCoef);
    VP_SURFACE *surf = (m_surfaceGroup->end() != it) ? it->second : nullptr;
    VP_RENDER_CHK_NULL_RETURN(surf);

    tmMaxCLL = (float)maxCLL;
    tmMaxDLL = (float)maxDLL;

    // Lock surface
    uint8_t *lockedAddr = (uint8_t *)m_allocator->LockResourceForWrite(&surf->osSurface->OsResource);

    VP_RENDER_CHK_NULL_RETURN(lockedAddr);

    hdrcoefBuffer = (float *)lockedAddr;

    if (hdrMode == VPHAL_HDR_MODE_TONE_MAPPING)  // H2S
    {
        CalcCCMMatrix();
        MOS_SecureMemcpy(ccmMatrix, sizeof(float) * 12, color_matrix_calculation, sizeof(float) * 12);

        if (maxDLL > 800)
        {
            tmMode = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2H;
        }
        else
        {
            tmMode = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2S;
        }
        oetfCurve = (OETF_CURVE_TYPE)OETF_SRGB;
        tmSrcType = (TONE_MAPPING_SOURCE_TYPE)TONE_MAPPING_SOURCE_PSEUDO_Y_BT709;
    }
    else  // H2H
    {
        MOS_SecureMemcpy(ccmMatrix, sizeof(float) * 12, ccm_identity, sizeof(float) * 12);

        tmMode    = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2H;
        oetfCurve = (OETF_CURVE_TYPE)OETF_CURVE_HDR_2084;
        tmSrcType = (TONE_MAPPING_SOURCE_TYPE)TONE_MAPPING_SOURCE_PSEUDO_Y_BT709;
    }

    // Fill Coefficient Surface: Media kernel define the layout of coefficients. Please don't change it.
    const uint32_t pos_coef[17] = {7, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 54, 55, 62, 63};

    // OETF curve
    ((int *)hdrcoefBuffer)[pos_coef[0]] = oetfCurve;
    // CCM
    for (uint32_t i = 0; i < VP_CCM_MATRIX_SIZE; ++i)
    {
        hdrcoefBuffer[pos_coef[i + 1]] = ccmMatrix[i];
    }
    // TM Source Type
    ((int *)hdrcoefBuffer)[pos_coef[13]] = tmSrcType;
    // TM Mode
    ((int *)hdrcoefBuffer)[pos_coef[14]] = tmMode;
    // Max CLL and DLL
    hdrcoefBuffer[pos_coef[15]] = tmMaxCLL;
    hdrcoefBuffer[pos_coef[16]] = tmMaxDLL;

    //Unlock
    VP_RENDER_CHK_STATUS_RETURN(m_allocator->UnLock(&surf->osSurface->OsResource));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutOclKernel::SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs)
{
    VP_FUNC_CALL();
    auto it = kernelConfigs.find((VpKernelID)kernelHdr3DLutCalcOcl);

    if (kernelConfigs.end() == it || nullptr == it->second)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    PRENDER_HDR_3DLUT_CAL_PARAMS params = (PRENDER_HDR_3DLUT_CAL_PARAMS)it->second;

    if (m_maxDisplayLum == params->maxDisplayLum && m_maxContentLevelLum == params->maxContentLevelLum &&
        m_hdrMode == params->hdrMode && m_hdrLutSize == params->threadWidth)
    {
        // For such case, 3DLut calculation should be skipped in Policy::GetHdrExecutionCaps.
        VP_RENDER_ASSERTMESSAGE("No change in 3D Lut parameters!");
    }
    else
    {
        m_maxDisplayLum      = params->maxDisplayLum;
        m_maxContentLevelLum = params->maxContentLevelLum;
        m_hdrMode            = params->hdrMode;
        m_hdrLutSize         = params->threadWidth;
        VP_RENDER_NORMALMESSAGE("Maximum Display Luminance %d, Maximum Content Level Luminance %d, HDR mode %d, Lut size %d",
            m_maxDisplayLum,
            m_maxContentLevelLum,
            m_hdrMode,
            m_hdrLutSize);
    }

    return MOS_STATUS_SUCCESS;
}



MOS_STATUS VpRenderHdr3DLutOclKernel::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();

    //All pData will be free in VpSrFilter::Destroy so no need to free here
    for (KRN_ARG &srcArg : kernelArgs)
    {
        for (KRN_ARG &dstArg : m_kernelArgs)
        {
            if (srcArg.uIndex == dstArg.uIndex)
            {
                if (dstArg.eArgKind == ARG_KIND_GENERAL || dstArg.eArgKind == ARG_KIND_INLINE || srcArg.eArgKind == ARG_KIND_SURFACE)
                {
                    if (srcArg.pData == nullptr)
                    {
                        VP_RENDER_ASSERTMESSAGE("The Kernel Argument General Data is null! KernelID %d, argIndex %d", m_kernelId, dstArg.uIndex);
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        dstArg.eArgKind = srcArg.eArgKind;
                        dstArg.pData    = srcArg.pData;
                        srcArg.pData    = nullptr;
                    }
                }
            }
        }

        if (srcArg.pData != nullptr)
        {
            srcArg.pData = nullptr;
            VP_RENDER_ASSERTMESSAGE("The Kernel Argument is set but not used. KernelID %d, argIndex %d", m_kernelId, srcArg.uIndex);
        }
    }

    return MOS_STATUS_SUCCESS;
}

void VpRenderHdr3DLutOclKernel::DumpSurfaces()
{
    VP_FUNC_CALL();
    for (auto &arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_SURFACE)
        {
            for (uint32_t idx = 0; idx < arg.uSize / (sizeof(SurfaceType) * 2); idx++)
            {
                uint32_t   *pSurfaceindex = (uint32_t *)(arg.pData) + idx;
                SurfaceType surfType      = (SurfaceType)*pSurfaceindex;
                if (surfType == SurfaceTypeInvalid)
                {
                    VP_RENDER_ASSERTMESSAGE("Surf type was invalid");
                    return;
                }
                auto surf = m_surfaceGroup->find(surfType);
                if (m_surfaceGroup->end() == surf)
                {
                    VP_RENDER_ASSERTMESSAGE("Surf was not found");
                    return;
                }

                char bufName[MAX_PATH] = {};

                MOS_SecureStringPrint(
                    bufName,
                    MAX_PATH,
                    sizeof(bufName),
                    "k_%d_%s_argi_%d",
                    m_kernelIndex,
                    m_kernelName.c_str(),
                    idx);

                if (surf->second == nullptr)
                {
                    return;
                }
                if (surf->second->osSurface == nullptr)
                {
                    return;
                }

                DumpSurface(surf->second, bufName);

            }
        }
    }

    return;
}