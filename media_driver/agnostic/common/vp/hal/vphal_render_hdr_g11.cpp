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
//! \file     vphal_render_hdr_g11.cpp
//! \brief    Common interface and structure used in HDR for GEN11
//! \details  Common interface and structure used in HDR for GEN11 Fixed Function Vebox 3DLut
//!
#if !EMUL

#include "vphal_render_hdr_g11.h"
#include "renderhal_legacy.h"
#include "mos_interface.h"

static const std::string DumpRoot("C:\\temp\\");
static const std::string OutputDumpDirectory(DumpRoot + "Output\\");
static const std::string Hdr3DLutKernelName("hdr_3dlut");

static const bool enableDump        = false;
static const int32_t lutWidth       = 65*2;
static const int32_t lutHeight      = 65 * 128;

static const float ccm_identity[12] = { 1.0f, 0.0f, 0.0f, 0.0f,
                                        0.0f, 1.0f, 0.0f, 0.0f,
                                        0.0f, 0.0f, 1.0f, 0.0f };

static float color_matrix_calculation[3][4] = {0.0f};

template <typename T>void mat_3by3_inv(T *m1, T *m2)
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

    det = a11 * (a33*a22 - a32 * a23) - a21 * (a33*a12 - a32 * a13) + a31 * (a12*a23 - a13 * a22);

    det_inv = 1 / det;

    *m2 = (float)(det_inv*(a33*a22 - a32 * a23));
    *(m2 + 1) = (float)(-det_inv * (a33*a12 - a32 * a13));
    *(m2 + 2) = (float)(det_inv*(a12*a23 - a13 * a22));
    *(m2 + 3) = (float)(-det_inv * (a33*a21 - a31 * a23));
    *(m2 + 4) = (float)(det_inv*(a33*a11 - a31 * a13));
    *(m2 + 5) = (float)(-det_inv * (a23*a11 - a21 * a13));
    *(m2 + 6) = (float)(det_inv*(a32*a21 - a31 * a22));
    *(m2 + 7) = (float)(-det_inv * (a32*a11 - a31 * a12));
    *(m2 + 8) = (float)(det_inv*(a22*a11 - a21 * a12));
}

template <typename T>void mat_mul_float(T *m1, T *m2, T *m3, short m_size, short n_size, short l_size)
{
    int i, j, k, Tjn, Tjl, off3;
    float  *m1_offs, *m2_offs, *m2_k_offs, *m3_offs;

    for (j = 0; j < m_size; j++)
    {
        Tjn = j * n_size;
        Tjl = j * l_size;
        for (k = 0; k < l_size; k++)
        {
            off3 = Tjl + k;
            m3_offs = m3 + off3;
            *m3_offs = 0;
            m2_k_offs = m2 + k;
            m1_offs = m1 + Tjn;
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
    float mat_rgb2xyz_src[3][3] = {}, mat_rgb2xyz_dst[3][3] = {};
    float mat_xyz[3][3] = {}, mat_invxyz[3][3] = {};
    float matW[3] = {}, matE[3] = {};

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

Hdr3DLutCmRender::Hdr3DLutCmRender(uint32_t *kernelBinary, uint32_t kernelSize, CmContext *cmContext) :
    VPCmRenderer("Hdr3DLutCmRender", cmContext),
    m_cmProgram(nullptr),
    m_cmKernel(nullptr),
    m_cmPayload(nullptr)
{
    m_cmProgram = LoadProgram(kernelBinary, kernelSize);

    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(cmContext);
    if (!m_cmProgram)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Hdr3DLutCmRender [%s]: CM LoadProgram error %d\n");
        return;
    }

    CmDevice *dev = cmContext->GetCmDevice();
    int result = dev->CreateKernel(m_cmProgram, _NAME(hdr_3dlut), m_cmKernel);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Hdr3DLutCmRender [%s]: CM CreateKernel error %d\n", mName.c_str(), result);
    }
}

Hdr3DLutCmRender::~Hdr3DLutCmRender()
{
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_cmContext);
    CmDevice *dev = m_cmContext->GetCmDevice();
    if (m_cmKernel)
    {
        dev->DestroyKernel(m_cmKernel);
    }

    if (m_cmProgram)
    {
        dev->DestroyProgram(m_cmProgram);
    }
}

void Hdr3DLutCmRender::AttachPayload(void *payload)
{
    m_cmPayload = static_cast<Hdr3DLutPayload *>(payload);
}

CmKernel* Hdr3DLutCmRender::GetKernelToRun(std::string &name)
{
    name = Hdr3DLutKernelName;
    return m_cmKernel;
}

void Hdr3DLutCmRender::GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor)
{
#define BLOCK_WIDTH 16
#define BLOCK_HEIGHT 8

    tsWidth  = (lutWidth + BLOCK_WIDTH - 1) / BLOCK_WIDTH;
    tsHeight = (lutHeight + BLOCK_HEIGHT - 1) / BLOCK_HEIGHT;
    tsColor  = 1;
}

void Hdr3DLutCmRender::PrepareKernel(CmKernel *kernel)
{
    kernel->SetKernelArg(0, sizeof(SurfaceIndex), m_cmPayload->hdr3DLutSurface->GetCmSurfaceIndex());
    kernel->SetKernelArg(1, sizeof(SurfaceIndex), m_cmPayload->hdrCoefSurface->GetCmSurfaceIndex());
    kernel->SetKernelArg(2, sizeof(uint16_t), &m_cmPayload->hdr3DLutSurfaceWidth);
    kernel->SetKernelArg(3, sizeof(uint16_t), &m_cmPayload->hdr3DLutSurfaceHeight);
}

Hdr3DLutGenerator::Hdr3DLutGenerator(PRENDERHAL_INTERFACE renderHal, uint32_t *kernelBinary, uint32_t kernelSize) :
    m_renderHal(renderHal),
    m_hdr3DLutSurface(nullptr),
    m_hdrCoefSurface(nullptr),
    m_hdr3DLutCmRender(nullptr),
    m_hdrcoefBuffer(nullptr),
    m_bHdr3DLutInit(false),
    m_savedMaxDLL(1000),
    m_savedMaxCLL(4000),
    m_savedHdrMode(VPHAL_HDR_MODE_NONE)
{
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_renderHal);
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_renderHal->pOsInterface);
    m_cmContext    = MOS_New(CmContext, m_renderHal->pOsInterface);

    VPHAL_RENDER_NORMALMESSAGE("Hdr3DLutGenerator Constructor!");

    m_kernelBinary = kernelBinary;
    m_kernelSize   = kernelSize;
}

Hdr3DLutGenerator::~Hdr3DLutGenerator()
{
    FreeResources();

    MOS_Delete(m_hdr3DLutCmRender);

    MOS_Delete(m_eventManager);

    MOS_Delete(m_cmContext);

    VPHAL_RENDER_NORMALMESSAGE("Hdr3DLutGenerator Destructor!");
}

void Hdr3DLutGenerator::AllocateResources()
{
    const int32_t coefWidth     = 8;
    const int32_t coefHeight    = 8;
    const int32_t lutWidth      = 65*2;
    const int32_t lutHeight     = 65 * 128;

    // Allocate 3DLut buffer in CPU memory to init 3DLut Surface
    m_hdr3DLutSysBuffer = MOS_NewArray(uint8_t, m_lutSizeInBytes);
    Init3DLutSurface();
    // Allocate 3DLut Surface
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_renderHal);
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_renderHal->pOsInterface);
    m_hdr3DLutSurface = MOS_New(VpCmSurfaceHolder<CmSurface2D>, lutWidth, lutHeight, 1, m_renderHal->pOsInterface->pfnMosFmtToGmmFmt(Format_A8R8G8B8), m_cmContext);
    m_hdr3DLutSurface->GetCmSurface()->WriteSurface(m_hdr3DLutSysBuffer, nullptr);

    // Allocate Coefficient Surface in GPU memory
    m_hdrCoefSurface = MOS_New(VpCmSurfaceHolder<CmSurface2D>, coefWidth, coefHeight, 1, GMM_FORMAT_B8G8R8A8_UNORM_TYPE, m_cmContext);
    // Allocate Coefficient Surface in CPU memory
    m_hdrcoefBuffer     = MOS_NewArray(float, coefWidth * coefHeight);
    return;
}

void Hdr3DLutGenerator::FreeResources()
{
    MOS_Delete(m_hdr3DLutSurface);
    MOS_Delete(m_hdrCoefSurface);
    MOS_DeleteArray(m_hdrcoefBuffer);
    MOS_DeleteArray(m_hdr3DLutSysBuffer);
}

void Hdr3DLutGenerator::Init3DLutSurface()
{
    int in_prec = 16;
    int max_input_level = ((1 << in_prec) - 1);
    int R = 0, G = 0, B = 0;
    int lutIndex = 0;

    uint16_t *pLut = (uint16_t *)m_hdr3DLutSysBuffer;

    int lutMul = m_mulSize;
    int lutSeg = m_segSize;
    for (int rr = 0; rr<lutSeg; rr++) {
        for (int gg = 0; gg<lutSeg; gg++) {
            for (int bb = 0; bb<lutMul; bb++) {
                //--- convert fixed point to floating point                

                if (bb >= lutSeg) {
                    pLut[lutIndex + 0] = 0;
                    pLut[lutIndex + 1] = 0;
                    pLut[lutIndex + 2] = 0;
                    pLut[lutIndex + 3] = 0;
                    lutIndex += 4;
                    continue;
                }

                if (rr == (lutSeg - 1))
                    R = max_input_level;
                else
                    R = rr * ((max_input_level + 1) / (lutSeg - 1));

                if (gg == (lutSeg - 1))
                    G = max_input_level;
                else
                    G = gg * ((max_input_level + 1) / (lutSeg - 1));

                if (bb == (lutSeg - 1))
                    B = max_input_level;
                else
                    B = bb * ((max_input_level + 1) / (lutSeg - 1));

                pLut[lutIndex + 0] = (unsigned short)R;
                pLut[lutIndex + 1] = (unsigned short)G;
                pLut[lutIndex + 2] = (unsigned short)B;
                pLut[lutIndex + 3] = 0;
                lutIndex += 4;
            }
        }
    }
}

void Hdr3DLutGenerator::InitCoefSurface(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode)
{
    int32_t oetfCurve = 0, tmMode = 0, tmSrcType = 0;
    float ccmMatrix[12] = { 0.0 };
    float tmMaxCLL = 0.0f, tmMaxDLL = 0.0f;

    tmMaxCLL      = (float)maxCLL;
    tmMaxDLL      = (float)maxDLL;

    if (hdrMode == VPHAL_HDR_MODE_TONE_MAPPING)    // H2S
    {
        CalcCCMMatrix();
        MOS_SecureMemcpy(ccmMatrix, sizeof(float) * 12, color_matrix_calculation, sizeof(float) * 12);
        if (maxDLL > 800)
        {
            tmMode = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2H;
            VPHAL_RENDER_NORMALMESSAGE("Change curve, maxDLL %d, hdrMode: %d!", maxDLL, tmMode);
        }
        else
        {
            tmMode = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2S;
        }
        oetfCurve       = (OETF_CURVE_TYPE)OETF_SRGB;
        tmSrcType       = (TONE_MAPPING_SOURCE_TYPE)TONE_MAPPING_SOURCE_PSEUDO_Y_BT709;
    }
    else    // H2H
    {
        MOS_SecureMemcpy(ccmMatrix, sizeof(float) * 12, ccm_identity, sizeof(float) * 12);

        tmMode          = (TONE_MAPPING_MODE)TONE_MAPPING_MODE_H2H;
        oetfCurve       = (OETF_CURVE_TYPE)OETF_CURVE_HDR_2084;
        tmSrcType       = (TONE_MAPPING_SOURCE_TYPE)TONE_MAPPING_SOURCE_PSEUDO_Y_BT709;
    }
    
    // Fill Coefficient Surface: Media kernel define the layout of coefficients. Please don't change it.
    const uint32_t pos_coef[17] = { 7, 16, 17, 18, 19, 20, 21, 24, 25, 26, 27, 28, 29, 54, 55, 62, 63 };

    // OETF curve
    ((int*)m_hdrcoefBuffer)[pos_coef[0]] = oetfCurve;
    // CCM
    m_hdrcoefBuffer[pos_coef[1]]     = ccmMatrix[0];
    m_hdrcoefBuffer[pos_coef[2]]     = ccmMatrix[1];
    m_hdrcoefBuffer[pos_coef[3]]     = ccmMatrix[2];
    m_hdrcoefBuffer[pos_coef[4]]     = ccmMatrix[3];
    m_hdrcoefBuffer[pos_coef[5]]     = ccmMatrix[4];
    m_hdrcoefBuffer[pos_coef[6]]     = ccmMatrix[5];
    m_hdrcoefBuffer[pos_coef[7]]     = ccmMatrix[6];
    m_hdrcoefBuffer[pos_coef[8]]     = ccmMatrix[7];
    m_hdrcoefBuffer[pos_coef[9]]     = ccmMatrix[8];
    m_hdrcoefBuffer[pos_coef[10]]    = ccmMatrix[9];
    m_hdrcoefBuffer[pos_coef[11]]    = ccmMatrix[10];
    m_hdrcoefBuffer[pos_coef[12]]    = ccmMatrix[11];
    // TM Source Type
    ((int*)m_hdrcoefBuffer)[pos_coef[13]]               = tmSrcType;
    // TM Mode
    ((int*)m_hdrcoefBuffer)[pos_coef[14]]               = tmMode;
    // Max CLL and DLL
    m_hdrcoefBuffer[pos_coef[15]]                       = tmMaxCLL;
    m_hdrcoefBuffer[pos_coef[16]]                       = tmMaxDLL;
}

void Hdr3DLutGenerator::Render(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode, const PVPHAL_SURFACE p3DLutSurface)
{
    MOS_STATUS eStatus                      = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE        pOsInterface      = nullptr;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(m_renderHal);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(p3DLutSurface);

    pOsInterface = m_renderHal->pOsInterface;
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pOsInterface);

    if (nullptr == m_hdr3DLutCmRender)
    {
        m_eventManager = MOS_New(EventManager, "EventManager", m_cmContext);
        VPHAL_RENDER_CHK_NULL_NO_STATUS(m_cmContext);

        m_hdr3DLutCmRender = MOS_New(Hdr3DLutCmRender, m_kernelBinary, m_kernelSize, m_cmContext);
        AllocateResources();

        VPHAL_RENDER_NORMALMESSAGE("Hdr3DLutGenerator Init Hdr3DLutCmRender and Allocate Necessary Resources!");
    }

    if (maxCLL != m_savedMaxCLL || maxDLL != m_savedMaxDLL || hdrMode != m_savedHdrMode)
    {
        m_savedMaxCLL = maxCLL;
        m_savedMaxDLL = maxDLL;
        m_savedHdrMode = hdrMode;

        InitCoefSurface(maxDLL, maxCLL, hdrMode);
        m_hdrCoefSurface->GetCmSurface()->WriteSurface((uint8_t*)m_hdrcoefBuffer, nullptr);

        Hdr3DLutCmRender::Hdr3DLutPayload hdr3DLutPayload = { 0 };
        hdr3DLutPayload.hdr3DLutSurface = m_hdr3DLutSurface;
        hdr3DLutPayload.hdrCoefSurface = m_hdrCoefSurface;
        hdr3DLutPayload.hdr3DLutSurfaceWidth = lutWidth;
        hdr3DLutPayload.hdr3DLutSurfaceHeight = lutHeight;

        VPHAL_RENDER_CHK_NULL_NO_STATUS(m_cmContext);
        m_cmContext->ConnectEventListener(m_eventManager);
        m_hdr3DLutCmRender->Render(&hdr3DLutPayload);
        m_cmContext->FlushBatchTask(false);
        m_cmContext->ConnectEventListener(nullptr);

        {
            MOS_LOCK_PARAMS         LockFlags;
            LockFlags.NoOverWrite = 1;
            uint8_t   *pVebox3DLutBuffer = (uint8_t*)m_renderHal->pOsInterface->pfnLockResource(
                pOsInterface,
                &p3DLutSurface->OsResource,
                &LockFlags);
            if (pVebox3DLutBuffer)
            {                
                m_hdr3DLutSurface->GetCmSurface()->ReadSurface(pVebox3DLutBuffer, nullptr);
            }
            else
            {
                VPHAL_RENDER_NORMALMESSAGE("Hdr3DLutGenerator::Render 3DLut Surface Lock Failed!");
            }
            pOsInterface->pfnUnlockResource(
                pOsInterface,
                &p3DLutSurface->OsResource);
        }

        if (enableDump)
        {
            // Dump 3DLut Surface
            int32_t width = 0, height = 0, depth = 0;
            m_hdr3DLutSurface->GetSurfaceDimentions(width, height, depth);
            m_hdr3DLutSurface->DumpSurfaceToFile(OutputDumpDirectory + "3DLutSurface" + std::to_string(width) + "x" + std::to_string(height) + ".dat");
            // Dump Coefficient Surface(including CCM, Tone Mapping Type etc.)
            m_hdrCoefSurface->GetSurfaceDimentions(width, height, depth);
            m_hdrCoefSurface->DumpSurfaceToFile(OutputDumpDirectory + "CoffSurface" + std::to_string(width) + "x" + std::to_string(height) + ".dat");
        }
    }

    VPHAL_RENDER_NORMALMESSAGE("Hdr3DLutGenerator Render maxCLL %d, maxDLL %d, hdrMode: %d!", maxCLL, maxDLL, hdrMode);

finish:
    return;
}
#endif
