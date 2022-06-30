/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     vphal_render_vebox_xe_xpm_denoise.h
//! \brief    Head file for vebox xe xpm denoise renderer
//! \details  Head file for vebox xe xpm denoise renderer
//!
#ifndef __VPHAL_RENDER_VEBOX_XE_XPM_DENOISE_H__
#define __VPHAL_RENDER_VEBOX_XE_XPM_DENOISE_H__

#include "renderhal_legacy.h"

#if !EMUL
#include "vphal_mdf_wrapper.h"

// Human Vision System Hpm Based Denoise
class HVSDenoiseHpm : public VPCmRenderer
{
public:
    struct HVSDenoisePayloadHpm
    {
        VpCmSurfaceHolder<CmBuffer> *denoiseParam;
        uint16_t                     Mode;
        uint16_t                     Format;
        uint16_t                     Width;
        uint16_t                     Height;
        uint32_t                     Noise_level;
        uint32_t                     Noise_level_u;
        uint32_t                     Noise_level_v;
        uint32_t                     Sgne_Level;
        uint32_t                     Sgne_Level_u;
        uint32_t                     Sgne_Level_v;
        uint32_t                     Sgne_Count;
        uint32_t                     Sgne_Count_u;
        uint32_t                     Sgne_Count_v;
        uint32_t                     PrevNslvTemporal;
        uint32_t                     PrevNslvTemporal_u;
        uint32_t                     PrevNslvTemporal_v;
        uint16_t                     QP;
        uint16_t                     FirstFrame;
        uint16_t                     TGNE_firstFrame;
        uint16_t                     FallBack;
        uint16_t                     EnableChroma;
        uint16_t                     EnableTemporalGNE;
        uint16_t                     CodecID;
    };

    HVSDenoiseHpm(const PRENDERHAL_INTERFACE vphalRenderer, void *kernelBinary, int32_t kerneBinarySize, CmContext *cmContext);
    virtual ~HVSDenoiseHpm();

private:
    virtual void      AttachPayload(void *payload);
    virtual CmKernel *GetKernelToRun(std::string &name);
    virtual void      GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor);
    virtual void      PrepareKernel(CmKernel *kernel);
    virtual void      Dump();

    CmProgram *m_cmProgram = nullptr;
    CmKernel * m_cmKernel  = nullptr;

    HVSDenoisePayloadHpm *m_payload = nullptr;
};

class VphalHVSDenoiserHpm
{
public:
    explicit VphalHVSDenoiserHpm(const PRENDERHAL_INTERFACE vphalRenderer);
    VphalHVSDenoiserHpm(const VphalHVSDenoiserHpm &) = delete;
    VphalHVSDenoiserHpm &operator=(const VphalHVSDenoiserHpm &) = delete;
    virtual ~VphalHVSDenoiserHpm();

    // This InitKernelParams function needs to be called immediately after constructor function.
    void       InitKernelParams(void *kernelBinary, const int32_t kerneBinarySize);
    MOS_STATUS Render(const PVPHAL_DENOISE_PARAMS pDNParams);
    uint8_t *  GetDenoiseParams()
    {
        return m_hvsDenoiseParam;
    }

private:
    void AllocateResources(const uint32_t width, const uint32_t height);
    void FreeResources();

    EventManager *               m_eventManager        = nullptr;
    PRENDERHAL_INTERFACE         m_renderHal           = nullptr;
    VpCmSurfaceHolder<CmBuffer> *m_hvsDenoiseCmSurface = nullptr;
    // Denoise Parameters in CPU memory
    uint8_t *   m_hvsDenoiseParam = nullptr;
    HVSDenoiseHpm *m_hvsDenoise      = nullptr;
    CmContext * m_cmContext       = nullptr;

    uint16_t m_savedQP        = 0;
    uint16_t m_savedStrength  = 0;
    bool     m_initHVSDenoise = false;

    // It is defined in Media Kernel.
    const uint32_t m_denoiseBufferInBytes = 160; //32 + 8 int
    void *         m_kernelBinary         = nullptr;
    int32_t        m_kernelBinarySize     = 0;
};
#else
#include "vphal_common.h"

class VphalHVSDenoiserHpm
{
public:
    explicit VphalHVSDenoiserHpm(PRENDERHAL_INTERFACE vphalRenderer)
    {
        MOS_UNUSED(vphalRenderer);
    };
    VphalHVSDenoiserHpm(const VphalHVSDenoiserHpm &) = delete;
    VphalHVSDenoiserHpm &operator=(const VphalHVSDenoiserHpm &) = delete;
    virtual ~VphalHVSDenoiserHpm(){};

    // This InitKernelParams function needs to be called after construrctor immediately.
    void InitKernelParams(void *kernelBinary, const int32_t kerneBinarySize)
    {
        MOS_UNUSED(kernelBinary);
        MOS_UNUSED(kerneBinarySize);
    };

    MOS_STATUS Render(const PVPHAL_DENOISE_PARAMS pDNParams)
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