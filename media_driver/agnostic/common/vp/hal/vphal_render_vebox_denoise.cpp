/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vphal_render_vebox_denoise.cpp
//! \brief    Handles rendering operations for vebox denoise
//! \details  Handles rendering operations for vebox denoise
//!
#include "vphal_render_vebox_denoise.h"
#include <cstring>
#include <new>
#include <string>
#include <vector>

#if !EMUL
const std::string DumpRoot("C:\\temp\\HVS\\");
const std::string OutputDumpDirectory(DumpRoot + "Output\\");

HVSDenoise::HVSDenoise(const PRENDERHAL_INTERFACE vphalRenderer, void *kernelBinary, int32_t kerneBinarySize, CmContext *cmContext) :
    VPCmRenderer("HVSDenoise", cmContext),
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
    CmDevice *dev = cmContext->GetCmDevice();
    int result = dev->CreateKernel(m_cmProgram, _NAME(getDenoiseFactor), m_cmKernel);
    if (result != CM_SUCCESS)
    {
        VPHAL_RENDER_ASSERTMESSAGE("[%s]: CM CreateKernel error %d\n", mName.c_str(), result);
    }
}

HVSDenoise::~HVSDenoise()
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

    m_payload           = nullptr;
}

void HVSDenoise::AttachPayload(void *payload)
{
    m_payload = static_cast<HVSDenoisePayload *>(payload);
}

CmKernel* HVSDenoise::GetKernelToRun(std::string &name)
{
    name = "HVSDenoise";
    return m_cmKernel;
}

void HVSDenoise::GetThreadSpaceDimension(int &tsWidth, int &tsHeight, int &tsColor)
{
    tsWidth = tsHeight = tsColor = 1;
}

void HVSDenoise::PrepareKernel(CmKernel *kernel)
{
    uint16_t value_QP = 0;

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

    kernel->SetKernelArg(0, sizeof(SurfaceIndex), m_payload->denoiseParam->GetCmSurfaceIndex());
    kernel->SetKernelArg(1, sizeof(uint16_t),     &value_QP);
    kernel->SetKernelArg(2, sizeof(uint16_t),     &m_payload->Strength);
}

void HVSDenoise::Dump()
{
    int width = 0, height = 0, depth = 0;
    m_payload->denoiseParam->DumpSurfaceToFile(OutputDumpDirectory + std::to_string(width) + "x" + std::to_string(height) + ".dat");
}

VphalHVSDenoiser::VphalHVSDenoiser(PRENDERHAL_INTERFACE renderHal) :
    m_eventManager(nullptr),
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

VphalHVSDenoiser::~VphalHVSDenoiser()
{
    FreeResources();
    MOS_Delete(m_hvsDenoise);
    MOS_Delete(m_eventManager);
    MOS_Delete(m_cmContext);
    VPHAL_RENDER_NORMALMESSAGE("Destructor!");
}

void VphalHVSDenoiser::InitKernelParams(void *kernelBinary, const int32_t kerneBinarySize)
{
    m_kernelBinary          = kernelBinary;
    m_kernelBinarySize      = kerneBinarySize;
}

void VphalHVSDenoiser::AllocateResources(const uint32_t width, const uint32_t height)
{
    uint32_t size         = width * height;

    m_hvsDenoiseCmSurface = MOS_New(VpCmSurfaceHolder<CmBuffer>, size, 1, 1, GMM_FORMAT_A8_UNORM_TYPE, m_cmContext);
    if (nullptr == m_hvsDenoiseCmSurface)
    {
        VPHAL_RENDER_NORMALMESSAGE("[0x%x] Failed to Allocate m_hvsDenoiseCmSurface(gpu memory) GMM_FORMAT_A8_UNORM_TYPE %d*%d!", this, width, height);
    }
    m_hvsDenoiseParam     = MOS_NewArray(uint8_t, size);
    if (nullptr == m_hvsDenoiseParam)
    {
        VPHAL_RENDER_NORMALMESSAGE("[0x%x] Failed to Allocate m_hvsDenoiseParam(cpu memory) uint8_t %d*%d!", this, width, height);
    }
}

void VphalHVSDenoiser::FreeResources()
{
    MOS_DeleteArray(m_hvsDenoiseParam);
    MOS_Delete(m_hvsDenoiseCmSurface);
}

MOS_STATUS VphalHVSDenoiser::Render(const PVPHAL_SURFACE pSrcSuface)
{
    MOS_STATUS          eStatus         = MOS_STATUS_SUCCESS;
    uint16_t            qp              = 18;
    uint16_t            strength        = 10;

    VPHAL_RENDER_CHK_NULL_RETURN(pSrcSuface);
    VPHAL_RENDER_CHK_NULL_RETURN(pSrcSuface->pDenoiseParams);

    qp          = pSrcSuface->pDenoiseParams->HVSDenoise.QP;
    strength    = pSrcSuface->pDenoiseParams->HVSDenoise.Strength;

    VPHAL_RENDER_NORMALMESSAGE("qp: %d, strength: %d", qp, strength);

    if (nullptr == m_hvsDenoise)
    {
        VPHAL_RENDER_CHK_NULL_RETURN(m_cmContext);

        m_hvsDenoise = MOS_New(HVSDenoise, m_renderHal, m_kernelBinary, m_kernelBinarySize, m_cmContext);
        AllocateResources(m_denoiseBufferInBytes, 1);

        VPHAL_RENDER_NORMALMESSAGE("[0x%x] Init HVSDenoise[0x%x] and Allocate necessary resource!", this, m_hvsDenoise);
    }

    if (qp != m_savedQP || strength != m_savedStrength)
    {
        HVSDenoise::HVSDenoisePayload denoisePayload    = {0};
        denoisePayload.denoiseParam                     = m_hvsDenoiseCmSurface;
        denoisePayload.Strength                         = strength;
        denoisePayload.QP                               = qp;

        VPHAL_RENDER_CHK_NULL_RETURN(m_cmContext);
        m_cmContext->ConnectEventListener(m_eventManager);
        m_hvsDenoise->Render(&denoisePayload);
        m_cmContext->FlushBatchTask(false);
        m_cmContext->ConnectEventListener(nullptr);

        m_hvsDenoiseCmSurface->GetCmSurface()->ReadSurface((uint8_t *)m_hvsDenoiseParam, nullptr, m_denoiseBufferInBytes);

        m_savedQP           = qp;
        m_savedStrength     = strength;

        VPHAL_RENDER_NORMALMESSAGE("Render qp %d, strength %d!", qp, strength);
    }

    return eStatus;
}

#endif  //!EMUL
