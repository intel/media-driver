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
//! \file     vphal_render_hdr_3dlut_g12.h
//! \brief    Common interface and structure used in HDR (3DLUT generation) for GEN12
//! \details  Common interface and structure used in HDR (3DLUT generation) for GEN12 Fixed Function Vebox 3DLut
//!
#ifndef __VPHAL_RENDER_HDR_3DLUT_G12_H__
#define __VPHAL_RENDER_HDR_3DLUT_G12_H__

#if !EMUL
#include "vphal_common.h"
#include "vphal_mdf_wrapper.h"
#include "vphal_render_hdr_g11.h"

class Hdr3DLutCmRenderG12 : public VPCmRenderer
{
public:
    struct Hdr3DLutPayload
    {
        VpCmSurfaceHolder<CmBuffer>         *hdr3DLutSurface;
        VpCmSurfaceHolder<CmSurface2D>      *hdrCoefSurface;
        uint16_t                            hdr3DLutSurfaceWidth;
        uint16_t                            hdr3DLutSurfaceHeight;
    };

    Hdr3DLutCmRenderG12(uint32_t *kernelBinary, uint32_t kernelSize, CmContext *cmContext);
    Hdr3DLutCmRenderG12(const Hdr3DLutCmRenderG12&) = delete;
    Hdr3DLutCmRenderG12& operator=(const Hdr3DLutCmRenderG12&) = delete;
    virtual ~Hdr3DLutCmRenderG12();

private:
    virtual void            AttachPayload(void *payload);
    virtual CmKernel        *GetKernelToRun(std::string &name);
    virtual void            GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor);
    virtual void            PrepareKernel(CmKernel *kernel);    

    CmProgram *m_cmProgram               = nullptr;
    CmKernel *m_cmKernel                 = nullptr;
    Hdr3DLutPayload *m_cmPayload         = nullptr;
};

class Hdr3DLutGeneratorG12
{
public:
    explicit Hdr3DLutGeneratorG12(PRENDERHAL_INTERFACE vphalRenderer, uint32_t *kernelBinary, uint32_t kernelSize);
    Hdr3DLutGeneratorG12(const Hdr3DLutGeneratorG12 &) = delete;
    Hdr3DLutGeneratorG12 &operator=(const Hdr3DLutGeneratorG12 &) = delete;
    virtual ~Hdr3DLutGeneratorG12();

    void Render(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode, const PVPHAL_SURFACE p3DLutSurface);
    bool IsObjectVaild();

private:
    void AllocateResources();
    void FreeResources();

    void InitCoefSurface(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode);
    void Init3DLutSurface();

    EventManager                        *m_eventManager        = nullptr;

    PRENDERHAL_INTERFACE                m_renderHal            = nullptr;
    VpCmSurfaceHolder<CmBuffer>         *m_hdr3DLutSurface     = nullptr;
    VpCmSurfaceHolder<CmSurface2D>      *m_hdrCoefSurface      = nullptr;
    Hdr3DLutCmRenderG12                 *m_hdr3DLutCmRender    = nullptr;
    float                               *m_hdrcoefBuffer       = nullptr;
    uint8_t                             *m_hdr3DLutSysBuffer   = nullptr;
    CmContext                           *m_cmContext           = nullptr;

    bool     m_bHdr3DLutInit            = false;
    uint32_t m_savedMaxDLL              = 1000;
    uint32_t m_savedMaxCLL              = 4000;
    VPHAL_HDR_MODE m_savedHdrMode       = VPHAL_HDR_MODE_NONE;

    // Default to 65^3
    const uint32_t    m_segSize  = 65;
    const uint32_t    m_mulSize  = 128;
    const uint32_t    m_lutSizeInBytes = m_segSize * m_segSize * m_mulSize * 4 * 2;

    uint32_t*         m_kernelBinary = nullptr;
    uint32_t          m_kernelSize   = 0;
};
#else
#include "vphal_render_hdr.h"
class Hdr3DLutGeneratorG12
{
public:
    explicit Hdr3DLutGeneratorG12(PRENDERHAL_INTERFACE vphalRenderer, uint32_t* kernelBinary, uint32_t kernelSize)
    {
        MOS_UNUSED(vphalRenderer);
        MOS_UNUSED(kernelBinary);
        MOS_UNUSED(kernelSize);
    };

    Hdr3DLutGeneratorG12(const Hdr3DLutGeneratorG12 &) = delete;
    Hdr3DLutGeneratorG12 &operator=(const Hdr3DLutGeneratorG12 &) = delete;
    virtual ~Hdr3DLutGeneratorG12() {};

    void Render(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode, const PVPHAL_SURFACE p3DLutSurface)
    {
        return ;
    };
    bool    IsObjectVaild() { return true; };
};
#endif
#endif // __VPHAL_RENDER_HDR_G12_H__