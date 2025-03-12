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
//! \file     vphal_render_hdr_g11.h
//! \brief    Common interface and structure used in HDR for GEN11
//! \details  Common interface and structure used in HDR for GEN11 Fixed Function Vebox 3DLut
//!
#ifndef __VPHAL_RENDER_HDR_G11_H__
#define __VPHAL_RENDER_HDR_G11_H__

#if !EMUL
#include "vphal_common.h"
#include "vphal_mdf_wrapper.h"

//!
//! \brief    Tone Mapping Source Type, Please don't change the Enmu Value.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!
typedef enum _TONE_MAPPING_SOURCE_TYPE
{
    TONE_MAPPING_SOURCE_NONE                = 0,
    TONE_MAPPING_SOURCE_RGB                 = 1,
    TONE_MAPPING_SOURCE_PSEUDO_Y_BT2020     = 2,
    TONE_MAPPING_SOURCE_FULL_Y_BT2020       = 3,
    TONE_MAPPING_SOURCE_FULL_Y_BT709        = 4,
    TONE_MAPPING_SOURCE_PSEUDO_Y_BT709      = 5
} TONE_MAPPING_SOURCE_TYPE;

//!
//! \brief    Tone Mapping Mode.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!
typedef enum _TONE_MAPPING_MODE
{
    TONE_MAPPING_MODE_H2H  = 0,
    TONE_MAPPING_MODE_H2S = 1,
    TONE_MAPPING_MODE_H2E = 2
} TONE_MAPPING_MODE;

//!
//! \brief    OETF Type.
//! \details  These Enmu Values are passed to media kernel for further processing.
//!
typedef enum _OETF_CURVE_TYPE
{
    OETF_CURVE_SDR_709     = 0,
    OETF_CURVE_HDR_2084    = 1,
    OETF_SRGB              = 2
} OETF_CURVE_TYPE;

class Hdr3DLutCmRender : public VPCmRenderer
{
public:
    struct Hdr3DLutPayload
    {
        VpCmSurfaceHolder<CmSurface2D>      *hdr3DLutSurface;
        VpCmSurfaceHolder<CmSurface2D>      *hdrCoefSurface;
        uint16_t                            hdr3DLutSurfaceWidth;
        uint16_t                            hdr3DLutSurfaceHeight;
    };

    Hdr3DLutCmRender(uint32_t *kernelBinary, uint32_t kernelSize, CmContext *cmContext);
    Hdr3DLutCmRender(const Hdr3DLutCmRender&) = delete;
    Hdr3DLutCmRender& operator=(const Hdr3DLutCmRender&) = delete;
    virtual ~Hdr3DLutCmRender();

private:
    virtual void            AttachPayload(void *payload);
    virtual CmKernel        *GetKernelToRun(std::string &name);
    virtual void            GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor);
    virtual void            PrepareKernel(CmKernel *kernel);    

    CmProgram *m_cmProgram               = nullptr;
    CmKernel *m_cmKernel                 = nullptr;
    Hdr3DLutPayload *m_cmPayload         = nullptr;
};

class Hdr3DLutGenerator
{
public:
    explicit Hdr3DLutGenerator(PRENDERHAL_INTERFACE vphalRenderer, uint32_t *kernelBinary, uint32_t kernelSize);
    Hdr3DLutGenerator(const Hdr3DLutGenerator &) = delete;
    Hdr3DLutGenerator &operator=(const Hdr3DLutGenerator &) = delete;
    virtual ~Hdr3DLutGenerator();

    void Render(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode, const PVPHAL_SURFACE p3DLutSurface);

private:
    void AllocateResources();
    void FreeResources();

    void InitCoefSurface(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode);
    void Init3DLutSurface();

    EventManager                        *m_eventManager        = nullptr;

    PRENDERHAL_INTERFACE                m_renderHal            = nullptr;
    VpCmSurfaceHolder<CmSurface2D>      *m_hdr3DLutSurface     = nullptr;
    VpCmSurfaceHolder<CmSurface2D>      *m_hdrCoefSurface      = nullptr;
    Hdr3DLutCmRender                    *m_hdr3DLutCmRender    = nullptr;
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
class Hdr3DLutGenerator
{
public:
    explicit Hdr3DLutGenerator(PRENDERHAL_INTERFACE vphalRenderer, uint32_t* kernelBinary, uint32_t kernelSize)
    {
        MOS_UNUSED(vphalRenderer);
        MOS_UNUSED(kernelBinary);
        MOS_UNUSED(kernelSize);
    };

    Hdr3DLutGenerator(const Hdr3DLutGenerator &) = delete;
    Hdr3DLutGenerator &operator=(const Hdr3DLutGenerator &) = delete;
    virtual ~Hdr3DLutGenerator() {};

    void Render(const uint32_t maxDLL, const uint32_t maxCLL, const VPHAL_HDR_MODE hdrMode, const PVPHAL_SURFACE p3DLutSurface)
    {
        return ;
    };
};
#endif
#endif // __VPHAL_RENDER_HDR_G11_H__