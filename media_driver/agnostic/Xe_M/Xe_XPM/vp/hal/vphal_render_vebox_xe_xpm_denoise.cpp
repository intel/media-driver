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
//! \file     vphal_render_vebox_xe_xpm_denoise.cpp
//! \brief    Handles rendering operations for vebox denoise
//! \details  Handles rendering operations for vebox denoise
//!
#include "vphal_render_vebox_xe_xpm_denoise.h"
#include <cstring>
#include <new>
#include <string>
#include <vector>

#if !EMUL
const std::string DumpRoot("/temp/HVS/");
const std::string OutputDumpDirectory(DumpRoot + "Output/");

HVSDenoiseHpm::HVSDenoiseHpm(const PRENDERHAL_INTERFACE vphalRenderer, void *kernelBinary, int32_t kerneBinarySize, CmContext *cmContext) : VPCmRenderer("HVSDenoise", cmContext),
                                                                                                                                      m_cmProgram(nullptr),
                                                                                                                                      m_cmKernel(nullptr),
                                                                                                                                      m_payload(nullptr)
{
    m_cmProgram = LoadProgram(kernelBinary, kerneBinarySize);
    if (!m_cmProgram)
    {
        VPHAL_RENDER_ASSERTMESSAGE("CM LoadProgram error\n");
    }

    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(cmContext);
    CmDevice *dev    = cmContext->GetCmDevice();
    int       result = dev->CreateKernel(m_cmProgram, _NAME(UpdateDNDITable), m_cmKernel);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM CreateKernel error %d\n", mName.c_str(), result);
    }
}

HVSDenoiseHpm::~HVSDenoiseHpm()
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

    m_payload = nullptr;
}

void HVSDenoiseHpm::AttachPayload(void *payload)
{
    m_payload = static_cast<HVSDenoisePayloadHpm *>(payload);
}

CmKernel *HVSDenoiseHpm::GetKernelToRun(std::string &name)
{
    name = "UpdateDNDITable";
    return m_cmKernel;
}

void HVSDenoiseHpm::GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor)
{
    tsWidth = tsHeight = tsColor = 1;
}

void HVSDenoiseHpm::PrepareKernel(CmKernel *kernel)
{
    uint16_t value_QP = 0;
    uint16_t value_Mode = 0;

    if (m_payload->Mode == HVSDENOISE_MANUAL)
    {
        if (m_payload->QP <= 18)
        {
            value_QP = 0;
        }
        else if (m_payload->QP <= 22)
        {
            value_QP = 1;
        }
        else if (m_payload->QP <= 27)
        {
            value_QP = 2;
        }
        else if (m_payload->QP <= 32)
        {
            value_QP = 3;
        }
        else if (m_payload->QP <= 37)
        {
            value_QP = 4;
        }
    }
    else
    {
        value_QP = m_payload->QP;
    }

    if (m_payload->Mode == HVSDENOISE_AUTO_BDRATE)
    {
        value_Mode = 0;
    }
    else if (m_payload->Mode == HVSDENOISE_AUTO_SUBJECTIVE)
    {
        value_Mode = 1;
    }
    else if (m_payload->Mode == HVSDENOISE_MANUAL)
    {
        value_Mode = 2;
    }

    kernel->SetKernelArg(0, sizeof(SurfaceIndex), m_payload->denoiseParam->GetCmSurfaceIndex());
    kernel->SetKernelArg(1, sizeof(uint16_t),     &value_Mode);
    kernel->SetKernelArg(2, sizeof(uint16_t),     &m_payload->Format);
    kernel->SetKernelArg(3, sizeof(uint16_t),     &m_payload->Width);
    kernel->SetKernelArg(4, sizeof(uint16_t),     &m_payload->Height);
    kernel->SetKernelArg(5, sizeof(uint32_t),     &m_payload->Noise_level);
    kernel->SetKernelArg(6, sizeof(uint32_t),     &m_payload->Noise_level_u);
    kernel->SetKernelArg(7, sizeof(uint32_t),     &m_payload->Noise_level_v);
    kernel->SetKernelArg(8, sizeof(uint32_t),     &m_payload->Sgne_Level);
    kernel->SetKernelArg(9, sizeof(uint32_t),     &m_payload->Sgne_Level_u);
    kernel->SetKernelArg(10, sizeof(uint32_t),    &m_payload->Sgne_Level_v);
    kernel->SetKernelArg(11, sizeof(uint32_t),    &m_payload->Sgne_Count);
    kernel->SetKernelArg(12, sizeof(uint32_t),    &m_payload->Sgne_Count_u);
    kernel->SetKernelArg(13, sizeof(uint32_t),    &m_payload->Sgne_Count_v);
    kernel->SetKernelArg(14, sizeof(uint32_t),    &m_payload->PrevNslvTemporal);
    kernel->SetKernelArg(15, sizeof(uint32_t),    &m_payload->PrevNslvTemporal_u);
    kernel->SetKernelArg(16, sizeof(uint32_t),    &m_payload->PrevNslvTemporal_v);
    kernel->SetKernelArg(17, sizeof(uint16_t),    &value_QP);
    kernel->SetKernelArg(18, sizeof(uint16_t),    &m_payload->FirstFrame);
    kernel->SetKernelArg(19, sizeof(uint16_t),    &m_payload->TGNE_firstFrame);
    kernel->SetKernelArg(20, sizeof(uint16_t),    &m_payload->FallBack);
    kernel->SetKernelArg(21, sizeof(uint16_t),    &m_payload->EnableChroma);
    kernel->SetKernelArg(22, sizeof(uint16_t),    &m_payload->EnableTemporalGNE);
}

void HVSDenoiseHpm::Dump()
{
    int width = 0, height = 0, depth = 0;
    m_payload->denoiseParam->DumpSurfaceToFile(OutputDumpDirectory + std::to_string(width) + "x" + std::to_string(height) + ".dat");
}

VphalHVSDenoiserHpm::VphalHVSDenoiserHpm(PRENDERHAL_INTERFACE renderHal) : m_eventManager(nullptr),
                                                                     m_renderHal(renderHal),
                                                                     m_hvsDenoiseCmSurface(nullptr),
                                                                     m_hvsDenoiseParam(nullptr),
                                                                     m_hvsDenoise(nullptr),
                                                                     m_savedQP(0),
                                                                     m_savedStrength(0),
                                                                     m_initHVSDenoise(false)
{
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_renderHal);
    VPHAL_RENDER_CHK_NULL_NO_STATUS_RETURN(m_renderHal->pOsInterface);
    m_cmContext    = MOS_New(CmContext, m_renderHal->pOsInterface);
    m_eventManager = MOS_New(EventManager, "HVSEventManager", m_cmContext);
    VPHAL_RENDER_NORMALMESSAGE("Constructor!");
}

VphalHVSDenoiserHpm::~VphalHVSDenoiserHpm()
{
    FreeResources();
    MOS_Delete(m_hvsDenoise);
    MOS_Delete(m_eventManager);
    MOS_Delete(m_cmContext);
    VPHAL_RENDER_NORMALMESSAGE("Destructor!");
}

void VphalHVSDenoiserHpm::InitKernelParams(void *kernelBinary, const int32_t kerneBinarySize)
{
    m_kernelBinary     = kernelBinary;
    m_kernelBinarySize = kerneBinarySize;
}

void VphalHVSDenoiserHpm::AllocateResources(const uint32_t width, const uint32_t height)
{
    uint32_t size = width * height;

    m_hvsDenoiseCmSurface = MOS_New(VpCmSurfaceHolder<CmBuffer>, size, 1, 1, GMM_FORMAT_A8_UNORM_TYPE, m_cmContext);
    if (nullptr == m_hvsDenoiseCmSurface)
    {
        VPHAL_RENDER_NORMALMESSAGE("[0x%x] Failed to Allocate m_hvsDenoiseCmSurface(gpu memory) GMM_FORMAT_A8_UNORM_TYPE %d*%d!", this, width, height);
    }
    m_hvsDenoiseParam = MOS_NewArray(uint8_t, size);
    if (nullptr == m_hvsDenoiseParam)
    {
        VPHAL_RENDER_NORMALMESSAGE("[0x%x] Failed to Allocate m_hvsDenoiseParam(cpu memory) uint8_t %d*%d!", this, width, height);
    }
}

void VphalHVSDenoiserHpm::FreeResources()
{
    MOS_DeleteArray(m_hvsDenoiseParam);
    MOS_Delete(m_hvsDenoiseCmSurface);
}

MOS_STATUS VphalHVSDenoiserHpm::Render(const PVPHAL_DENOISE_PARAMS pDNParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL_RETURN(pDNParams);

    if (nullptr == m_hvsDenoise)
    {
        VPHAL_RENDER_CHK_NULL_RETURN(m_cmContext);

        m_hvsDenoise = MOS_New(HVSDenoiseHpm, m_renderHal, m_kernelBinary, m_kernelBinarySize, m_cmContext);
        AllocateResources(m_denoiseBufferInBytes, 1);

        VPHAL_RENDER_NORMALMESSAGE("[0x%x] Init HVSDenoise[0x%x] and Allocate necessary resource!", this, m_hvsDenoise);
    }

    HVSDenoiseHpm::HVSDenoisePayloadHpm denoisePayload = {0};
    denoisePayload.denoiseParam                  = m_hvsDenoiseCmSurface;
    denoisePayload.QP                            = pDNParams->HVSDenoise.QP;
    denoisePayload.EnableTemporalGNE             = pDNParams->HVSDenoise.EnableTemporalGNE;
    denoisePayload.FallBack                      = pDNParams->HVSDenoise.Fallback;
    denoisePayload.FirstFrame                    = pDNParams->HVSDenoise.FirstFrame;
    denoisePayload.Mode                          = pDNParams->HVSDenoise.Mode;
    denoisePayload.TGNE_firstFrame               = pDNParams->HVSDenoise.TgneFirstFrame;
    denoisePayload.Format                        = 0;                                     //Default YUV Format
    denoisePayload.EnableChroma                  = pDNParams->HVSDenoise.EnableChroma;
    denoisePayload.Sgne_Count                    = pDNParams->HVSDenoise.Sgne_Count;
    denoisePayload.Sgne_Count_u                  = pDNParams->HVSDenoise.Sgne_CountU;
    denoisePayload.Sgne_Count_v                  = pDNParams->HVSDenoise.Sgne_CountV;
    denoisePayload.Sgne_Level                    = pDNParams->HVSDenoise.Sgne_Level;
    denoisePayload.Sgne_Level_u                  = pDNParams->HVSDenoise.Sgne_LevelU;
    denoisePayload.Sgne_Level_v                  = pDNParams->HVSDenoise.Sgne_LevelV;
    denoisePayload.PrevNslvTemporal              = pDNParams->HVSDenoise.PrevNslvTemporal;
    denoisePayload.PrevNslvTemporal_u            = pDNParams->HVSDenoise.PrevNslvTemporalU;
    denoisePayload.PrevNslvTemporal_v            = pDNParams->HVSDenoise.PrevNslvTemporalV;
    denoisePayload.Width                         = pDNParams->HVSDenoise.Width;
    denoisePayload.Height                        = pDNParams->HVSDenoise.Height;

    if (pDNParams->HVSDenoise.Mode == HVSDENOISE_MANUAL)
    {
        denoisePayload.Noise_level   = uint32_t(pDNParams->HVSDenoise.Strength);
        denoisePayload.Noise_level_u = uint32_t(pDNParams->HVSDenoise.Strength);
        denoisePayload.Noise_level_v = uint32_t(pDNParams->HVSDenoise.Strength);
    }
    else
    {
        denoisePayload.Noise_level   = pDNParams->HVSDenoise.dwGlobalNoiseLevel;
        denoisePayload.Noise_level_u = pDNParams->HVSDenoise.dwGlobalNoiseLevelU;
        denoisePayload.Noise_level_v = pDNParams->HVSDenoise.dwGlobalNoiseLevelV;
    }

    VPHAL_RENDER_CHK_NULL_RETURN(m_cmContext);
    m_cmContext->ConnectEventListener(m_eventManager);
    m_hvsDenoise->Render(&denoisePayload);
    m_cmContext->FlushBatchTask(false);
    m_cmContext->ConnectEventListener(nullptr);

    m_hvsDenoiseCmSurface->GetCmSurface()->ReadSurface((uint8_t *)m_hvsDenoiseParam, nullptr, m_denoiseBufferInBytes);

    VPHAL_RENDER_NORMALMESSAGE("Render qp %d, HVS mode %d!, Noise_level %d, Noise_level_u %d, Noise_level_v %d, FirstFrame %d, TGNE_firstFrame %d, EnableTemporalTGNE %d, FallBack %d, EnableChroma %d",
        pDNParams->HVSDenoise.QP,
        pDNParams->HVSDenoise.Mode,
        denoisePayload.Noise_level,
        denoisePayload.Noise_level_u,
        denoisePayload.Noise_level_v,
        pDNParams->HVSDenoise.FirstFrame,
        pDNParams->HVSDenoise.TgneFirstFrame,
        pDNParams->HVSDenoise.EnableTemporalGNE,
        pDNParams->HVSDenoise.Fallback,
        pDNParams->HVSDenoise.EnableChroma);

    VPHAL_RENDER_NORMALMESSAGE("Sgne_Count %d, Sgne_CountU %d!, Sgne_CountV %d, Sgne_Level %d, Sgne_LevelU %d, Sgne_LevelV %d, PrevNslvTemporal %d, PrevNslvTemporalU %d, PrevNslvTemporalV %d, Width %d, Height %d",
        denoisePayload.Sgne_Count,
        denoisePayload.Sgne_Count_u,
        denoisePayload.Sgne_Count_v,
        denoisePayload.Sgne_Level,
        denoisePayload.Sgne_Level_u,
        denoisePayload.Sgne_Level_v,
        denoisePayload.PrevNslvTemporal,
        denoisePayload.PrevNslvTemporal_u,
        denoisePayload.PrevNslvTemporal_v,
        denoisePayload.Width,
        denoisePayload.Height);
    return eStatus;
}

#endif  //!EMUL
