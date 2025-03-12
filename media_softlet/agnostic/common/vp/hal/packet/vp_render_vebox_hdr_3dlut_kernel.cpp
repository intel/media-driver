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
//! \file     vp_render_vebox_hdr_3dlut_kernel.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!
#include "vp_render_vebox_hdr_3dlut_kernel.h"
#include "vp_dumper.h"
#include "vp_kernelset.h"

using namespace vp;

//!
//! \brief Binding Table Index for HDR 3DLut kernel
//!
#define BI_VEBOX_HDR_3DLUT_3DLUT 1
#define BI_VEBOX_HDR_3DLUT_COEF  2

#define BI_VEBOX_HDR_3DLUT_3DLUT_CM 0
#define BI_VEBOX_HDR_3DLUT_COEF_CM  1

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

// Used by ISA kernel
VpRenderHdr3DLutKernel::VpRenderHdr3DLutKernel(PVP_MHWINTERFACE hwInterface, VpKernelID kernelId, uint32_t kernelIndex, std::string kernelName, PVpAllocator allocator) :
    VpRenderKernelObj(hwInterface, kernelId, kernelIndex, VP_HDR_KERNEL_NAME, allocator)
{
    VP_FUNC_CALL();
}

// Used by L0 kernel
VpRenderHdr3DLutKernel::VpRenderHdr3DLutKernel(PVP_MHWINTERFACE hwInterface, PVpAllocator allocator) :
    VpRenderKernelObj(hwInterface, (VpKernelID)kernelHdr3DLutCalc, 0, VP_HDR_KERNEL_NAME_L0, allocator)
{
    VP_FUNC_CALL();
    m_kernelBinaryID = VP_ADV_KERNEL_BINARY_ID(kernelHdr3DLutCalc);
}

VpRenderHdr3DLutKernel::~VpRenderHdr3DLutKernel()
{
}

MOS_STATUS VpRenderHdr3DLutKernel::SetupSurfaceState()
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_surfaceGroup);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);

    PRENDERHAL_INTERFACE renderHal   = m_hwInterface->m_renderHal;
    PMOS_INTERFACE       osInterface = m_hwInterface->m_osInterface;
    m_surfaceBindingIndex.clear();
    m_surfaceState.clear();
    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam            = {};
    // Only need to specify binding index in surface parameters.
    kernelSurfaceParam.surfaceOverwriteParams.updatedSurfaceParams = true; 
    kernelSurfaceParam.surfaceOverwriteParams.bindedKernel   = true;
    kernelSurfaceParam.surfaceOverwriteParams.bufferResource = true;

    UpdateCurbeBindingIndex(SurfaceType3DLut, BI_VEBOX_HDR_3DLUT_3DLUT);
    kernelSurfaceParam.isOutput                         = true;
    m_surfaceState.insert(std::make_pair(SurfaceType3DLut, kernelSurfaceParam));
    UpdateCurbeBindingIndex(SurfaceType3DLutCoef, BI_VEBOX_HDR_3DLUT_COEF);
    kernelSurfaceParam.isOutput                         = false;
    m_surfaceState.insert(std::make_pair(SurfaceType3DLutCoef, kernelSurfaceParam));

    VP_RENDER_CHK_STATUS_RETURN(InitCoefSurface(m_maxDisplayLum, m_maxContentLevelLum, m_hdrMode));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernel::CpPrepareResources()
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

MOS_STATUS VpRenderHdr3DLutKernel::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    // init the hdr 3dlut static data
    auto bindingMap3DLut = GetSurfaceBindingIndex(SurfaceType3DLut);
    auto bindingMapCoef  = GetSurfaceBindingIndex(SurfaceType3DLutCoef);
    if (bindingMap3DLut.empty() || bindingMapCoef.empty())
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    MOS_ZeroMemory(&m_curbe, sizeof(m_curbe));
    m_curbe.DW02.hdr3DLutSurface       = *bindingMap3DLut.begin();
    m_curbe.DW04.hdrCoefSurface        = *bindingMapCoef.begin();
    m_curbe.DW06.hdr3DLutSurfaceWidth  = m_hdrLutSize;
    m_curbe.DW06.hdr3DLutSurfaceHeight = m_hdrLutSize;

    curbeLength = sizeof(VEBOX_HDR_3DLUT_STATIC_DATA);
    curbe = (uint8_t *) & m_curbe;
    VP_RENDER_NORMALMESSAGE("HDR 3DLut Kernel curbelength %d", curbeLength);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernel::SetKernelConfigs(KERNEL_CONFIGS &kernelConfigs)
{
    VP_FUNC_CALL();
    auto it = kernelConfigs.find((VpKernelID)kernelHdr3DLutCalc);

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


MOS_STATUS VpRenderHdr3DLutKernel::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{

    VP_FUNC_CALL();
    RENDERHAL_KERNEL_PARAM kernelSettings;

    VP_RENDER_CHK_STATUS_RETURN(GetKernelSettings(kernelSettings));
    MOS_ZeroMemory(&walkerParam, sizeof(KERNEL_WALKER_PARAMS));

    VP_RENDER_CHK_STATUS_RETURN(VpRenderKernelObj::GetWalkerSetting(m_walkerParam, renderData));
    m_walkerParam.iBlocksX = m_hdrLutSize;
    m_walkerParam.iBlocksY = m_hdrLutSize;
    m_walkerParam.isVerticalPattern = false;
    m_walkerParam.bSyncFlag         = true;
    walkerParam = m_walkerParam;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernel::InitCoefSurface(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode)
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
            tmMode = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2E;
            VP_RENDER_NORMALMESSAGE("Change curve to H2E, maxDLL %d", maxDLL);
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

MOS_STATUS VpRenderHdr3DLutKernel::SetPerfTag()
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    auto osInterface = m_hwInterface->m_osInterface;
    VP_RENDER_CHK_NULL_RETURN(osInterface);
    VP_RENDER_CHK_NULL_RETURN(osInterface->pfnSetPerfTag);

    osInterface->pfnSetPerfTag(osInterface, VPHAL_EU3DLUT);
    return MOS_STATUS_SUCCESS;
}

VpRenderHdr3DLutKernelCM::VpRenderHdr3DLutKernelCM(PVP_MHWINTERFACE hwInterface, VpKernelID kernelID, uint32_t kernelIndex, PVpAllocator allocator) : 
    VpRenderHdr3DLutKernel(hwInterface, kernelID, kernelIndex, VP_HDR_KERNEL_NAME, allocator)
{
    m_kernelBinaryID = VP_ADV_KERNEL_BINARY_ID(kernelID);
    m_isAdvKernel    = true;
}

VpRenderHdr3DLutKernelCM::~VpRenderHdr3DLutKernelCM()
{
    // No need to destroy dstArg.pData, which points to the local variable
    // in VpHdrFilter.
}

MOS_STATUS VpRenderHdr3DLutKernelCM::Init(VpRenderKernel &kernel)
{
    VP_FUNC_CALL();
    m_kernelSize = kernel.GetKernelSize() + KERNEL_BINARY_PADDING_SIZE;
    m_kernelPaddingSize = KERNEL_BINARY_PADDING_SIZE;
    uint8_t *pKernelBin = (uint8_t *)kernel.GetKernelBinPointer();
    VP_RENDER_CHK_NULL_RETURN(pKernelBin);

    m_kernelBinary = pKernelBin + kernel.GetKernelBinOffset();
    m_kernelArgs = kernel.GetKernelArgs();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernelCM::GetWalkerSetting(KERNEL_WALKER_PARAMS &walkerParam, KERNEL_PACKET_RENDER_DATA &renderData)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_STATUS_RETURN(VpRenderKernelObj::GetWalkerSetting(m_walkerParam, renderData));

    walkerParam = m_walkerParam;
    return MOS_STATUS_SUCCESS;
}

// Only for Adv kernels.
MOS_STATUS VpRenderHdr3DLutKernelCM::SetWalkerSetting(KERNEL_THREAD_SPACE &threadSpace, bool bSyncFlag, bool flushL1)
{
    VP_FUNC_CALL();
    MOS_ZeroMemory(&m_walkerParam, sizeof(KERNEL_WALKER_PARAMS));

    m_walkerParam.iBlocksX          = threadSpace.uWidth;
    m_walkerParam.iBlocksY          = threadSpace.uHeight;
    m_walkerParam.isVerticalPattern = false;
    m_walkerParam.bSyncFlag         = bSyncFlag;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernelCM::SetKernelArgs(KERNEL_ARGS &kernelArgs, VP_PACKET_SHARED_CONTEXT *sharedContext)
{
    VP_FUNC_CALL();
    if (kernelArgs.size() != m_kernelArgs.size())
    {
        VP_RENDER_ASSERTMESSAGE("The Kernel Arguments is not aligned!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < m_kernelArgs.size(); ++i)
    {
        if (i >= kernelArgs.size())
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        KRN_ARG &srcArg = kernelArgs[i];
        KRN_ARG &dstArg = m_kernelArgs[i];

        if (srcArg.uIndex   != dstArg.uIndex    ||
            srcArg.uSize    != dstArg.uSize     ||
            srcArg.eArgKind != dstArg.eArgKind  &&
            dstArg.eArgKind != (srcArg.eArgKind & ~SURFACE_MASK)    ||
            srcArg.pData == nullptr)
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        dstArg.eArgKind = srcArg.eArgKind;
        dstArg.pData    = srcArg.pData;
        srcArg.pData    = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernelCM::GetCurbeState(void *&curbe, uint32_t &curbeLength)
{
    VP_FUNC_CALL();
    curbeLength = 0;
    for (auto arg : m_kernelArgs)
    {
        curbeLength += arg.uSize;
    }

    if (sizeof(m_curbe) != curbeLength)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    uint8_t *data = (uint8_t *)&m_curbe;

    for (auto &arg : m_kernelArgs)
    {
        if (arg.eArgKind == ARG_KIND_SURFACE)
        {
            // Resource need be added.
            uint32_t *pSurfaceindex = static_cast<uint32_t *>(arg.pData);
            auto      bindingMap    = GetSurfaceBindingIndex((SurfaceType)*pSurfaceindex);
            if (bindingMap.empty())
            {
                VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            *((uint32_t *)(data + arg.uOffsetInPayload)) = *bindingMap.begin();
        }
        else if (arg.eArgKind == ARG_KIND_GENERAL)
        {
            MOS_SecureMemcpy(data + arg.uOffsetInPayload, arg.uSize, arg.pData, arg.uSize);
        }
        else
        {
            VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
        }
    }

    curbe       = data;
    VP_RENDER_NORMALMESSAGE("HDR 3DLut Kernel curbelength %d", curbeLength);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernelCM::SetupSurfaceState()
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_surfaceGroup);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);

    PRENDERHAL_INTERFACE renderHal = m_hwInterface->m_renderHal;
    PMOS_INTERFACE osInterface = m_hwInterface->m_osInterface;
    m_surfaceBindingIndex.clear();

    KERNEL_SURFACE_STATE_PARAM kernelSurfaceParam            = {};
    kernelSurfaceParam.surfaceOverwriteParams.bindedKernel   = true;
    kernelSurfaceParam.surfaceOverwriteParams.bufferResource = true;

    UpdateCurbeBindingIndex(SurfaceType3DLut, BI_VEBOX_HDR_3DLUT_3DLUT_CM);
    kernelSurfaceParam.isOutput                         = true;
    m_surfaceState.insert(std::make_pair(SurfaceType3DLut, kernelSurfaceParam));
    UpdateCurbeBindingIndex(SurfaceType3DLutCoef, BI_VEBOX_HDR_3DLUT_COEF_CM);
    kernelSurfaceParam.isOutput                         = false;
    m_surfaceState.insert(std::make_pair(SurfaceType3DLutCoef, kernelSurfaceParam));
    
    VP_RENDER_CHK_STATUS_RETURN(InitCoefSurface(m_maxDisplayLum, m_maxContentLevelLum, m_hdrMode));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernelCM::CpPrepareResources()
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

MOS_STATUS VpRenderHdr3DLutKernelCM::SetKernelConfigs(KERNEL_CONFIGS& kernelConfigs)
{
    VP_FUNC_CALL();
    auto it = kernelConfigs.find((VpKernelID)kernelHdr3DLutCalc);

    if (kernelConfigs.end() == it || nullptr == it->second)
    {
        VP_RENDER_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    PRENDER_HDR_3DLUT_CAL_PARAMS params = (PRENDER_HDR_3DLUT_CAL_PARAMS)it->second;

    if (m_maxDisplayLum == params->maxDisplayLum && m_maxContentLevelLum == params->maxContentLevelLum &&
        m_hdrMode == params->hdrMode)
    {
        // For such case, 3DLut calculation should be skipped in Policy::GetHdrExecutionCaps.
        VP_RENDER_ASSERTMESSAGE("No change in 3D Lut parameters!");
    }
    else
    {
        m_maxDisplayLum         = params->maxDisplayLum;
        m_maxContentLevelLum    = params->maxContentLevelLum;
        m_hdrMode               = params->hdrMode;
        VP_RENDER_NORMALMESSAGE("Maximum Display Luminance %d, Maximum Content Level Luminance %d, HDR mode %d",
            m_maxDisplayLum, m_maxContentLevelLum, m_hdrMode);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpRenderHdr3DLutKernelCM::SetPerfTag()
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    auto osInterface = m_hwInterface->m_osInterface;
    VP_RENDER_CHK_NULL_RETURN(osInterface);
    VP_RENDER_CHK_NULL_RETURN(osInterface->pfnSetPerfTag);

    osInterface->pfnSetPerfTag(osInterface, VPHAL_EU3DLUT);
    return MOS_STATUS_SUCCESS;
}