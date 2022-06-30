/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     vphal_render_vebox_denoise.h
//! \brief    Head file for vebox denoise renderer
//! \details  Head file for vebox denoise renderer
//!
#ifndef __VPHAL_RENDER_VEBOX_DENOISE_H__
#define __VPHAL_RENDER_VEBOX_DENOISE_H__

#include "renderhal_legacy.h"

#if !EMUL
#include "vphal_mdf_wrapper.h"

// Human Vision System Based Denoise
class HVSDenoise : public VPCmRenderer
{
public:
    struct HVSDenoisePayload
    {
        VpCmSurfaceHolder<CmBuffer> *denoiseParam;
        uint16_t                     QP;
        uint16_t                     CodecID;
        uint16_t                     Strength;
    };

    HVSDenoise(const PRENDERHAL_INTERFACE vphalRenderer, void *kernelBinary, int32_t kerneBinarySize, CmContext *cmContext);
    virtual ~HVSDenoise();

private:
    virtual void      AttachPayload(void *payload);
    virtual CmKernel *GetKernelToRun(std::string &name);
    virtual void      GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor);
    virtual void      PrepareKernel(CmKernel *kernel);
    virtual void      Dump();

    CmProgram *m_cmProgram              = nullptr;
    CmKernel *m_cmKernel                = nullptr;

    HVSDenoisePayload *m_payload        = nullptr;
};

class VphalHVSDenoiser
{
public:
    explicit VphalHVSDenoiser(const PRENDERHAL_INTERFACE vphalRenderer);
    VphalHVSDenoiser(const VphalHVSDenoiser &) = delete;
    VphalHVSDenoiser &operator=(const VphalHVSDenoiser &) = delete;
    virtual ~VphalHVSDenoiser();

    // This InitKernelParams function needs to be called immediately after constructor function.
    void InitKernelParams(void *kernelBinary, const int32_t kerneBinarySize);
    MOS_STATUS Render(const PVPHAL_SURFACE pSrcSuface);
    uint8_t*  GetDenoiseParams() 
    { 
        return m_hvsDenoiseParam; 
    }

private:
    void AllocateResources(const uint32_t width, const uint32_t height);
    void FreeResources();

    EventManager*                m_eventManager            = nullptr;
    PRENDERHAL_INTERFACE         m_renderHal               = nullptr;
    VpCmSurfaceHolder<CmBuffer> *m_hvsDenoiseCmSurface     = nullptr;
    // Denoise Parameters in CPU memory
    uint8_t *                    m_hvsDenoiseParam         = nullptr;
    HVSDenoise *                 m_hvsDenoise              = nullptr;
    CmContext *                  m_cmContext               = nullptr;

    uint16_t m_savedQP              = 0;
    uint16_t m_savedStrength        = 0;
    bool     m_initHVSDenoise       = false;

    // It is defined in Media Kernel.
    const uint32_t    m_denoiseBufferInBytes        = 64;
    void              *m_kernelBinary               = nullptr;
    int32_t           m_kernelBinarySize            = 0;
};
#else
#include "vphal_common.h"

class VphalHVSDenoiser
{
public:
    explicit VphalHVSDenoiser(PRENDERHAL_INTERFACE vphalRenderer)
    { 
        MOS_UNUSED(vphalRenderer); 
    };
    VphalHVSDenoiser(const VphalHVSDenoiser &) = delete;
    VphalHVSDenoiser &operator=(const VphalHVSDenoiser &) = delete;
    virtual ~VphalHVSDenoiser() 
    {
    };

    // This InitKernelParams function needs to be called after construrctor immediately.
    void InitKernelParams(void *kernelBinary, const int32_t kerneBinarySize)
    {
        MOS_UNUSED(kernelBinary);
        MOS_UNUSED(kerneBinarySize);
    };

    MOS_STATUS Render(const PVPHAL_SURFACE pSrcSuface)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    uint8_t *GetDenoiseParams()
    {
        return nullptr;
    }
};
#endif  // !EMUL
#endif  // __VPHAL_RENDER_VEBOX_DENOISE_H__