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
//! \file     vp_hdr_filter.cpp
//! \brief    Defines the common interface for Adaptive Contrast Enhancement
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#include "vp_hdr_filter.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_render_cmd_packet.h"
#include "igvp3dlut_args.h"

namespace vp
{
VpHdrFilter::VpHdrFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface), m_3DLutSurfaceWidth(LUT65_SEG_SIZE * 2), m_3DLutSurfaceHeight(LUT65_SEG_SIZE * LUT65_MUL_SIZE)
{
}

MOS_STATUS VpHdrFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrFilter::Destroy()
{
    VP_FUNC_CALL();
    for (auto &handle : m_renderHdr3DLutOclParams)
    {
        KRN_ARG &krnArg = handle.second;
        MOS_FreeMemAndSetNull(krnArg.pData);
    }
    return MOS_STATUS_SUCCESS;
}

void _RENDER_HDR_3DLUT_CAL_PARAMS::Init()
{
    maxDisplayLum      = 0;      
    maxContentLevelLum = 0;
    hdrMode            = VPHAL_HDR_MODE_NONE;
    kernelId           = kernelCombinedFc;
    threadWidth        = 0;
    threadHeight       = 0;
    kernelArgs.clear();
}

MOS_STATUS VpHdrFilter::SetExecuteEngineCaps(
        SwFilterPipe    *executedPipe,
        VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executedPipe  = executedPipe;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpHdrFilter::CalculateEngineParams(
    FeatureParamHdr &hdrParams,
    VP_EXECUTE_CAPS  vpExecuteCaps)
{
    VP_FUNC_CALL();
    if (vpExecuteCaps.bVebox)
    {
        // create a filter Param buffer
        MOS_ZeroMemory(&m_veboxHdrParams, sizeof(VEBOX_HDR_PARAMS));
        m_veboxHdrParams.uiMaxDisplayLum      = hdrParams.uiMaxDisplayLum;
        m_veboxHdrParams.uiMaxContentLevelLum = hdrParams.uiMaxContentLevelLum;
        m_veboxHdrParams.hdrMode              = hdrParams.hdrMode;
        m_veboxHdrParams.srcColorSpace        = hdrParams.srcColorSpace;
        m_veboxHdrParams.dstColorSpace        = hdrParams.dstColorSpace;
        m_veboxHdrParams.dstFormat            = hdrParams.formatOutput;
        m_veboxHdrParams.stage                = hdrParams.stage;
        m_veboxHdrParams.lutSize              = hdrParams.lutSize;
        m_veboxHdrParams.isFp16Enable         = (hdrParams.formatInput == Format_A16B16G16R16F) ? true : false;
        m_veboxHdrParams.external3DLutParams  = hdrParams.external3DLutParams;
    }
    else if (vpExecuteCaps.bRender && HDR_STAGE_3DLUT_KERNEL == hdrParams.stage)
    {
        // create a filter Param buffer
        m_renderHdr3DLutParams.Init();
        m_renderHdr3DLutParams.maxDisplayLum       = hdrParams.uiMaxDisplayLum;
        m_renderHdr3DLutParams.maxContentLevelLum  = hdrParams.uiMaxContentLevelLum;
        m_renderHdr3DLutParams.hdrMode             = hdrParams.hdrMode;

        m_renderHdr3DLutParams.threadWidth  = hdrParams.lutSize;
        m_renderHdr3DLutParams.threadHeight = hdrParams.lutSize;

        if (hdrParams.isOclKernelEnabled == false)
        {
            KRN_ARG krnArg  = {};
            krnArg.uIndex   = 0;
            krnArg.eArgKind = ARG_KIND_SURFACE;
            krnArg.uSize    = 4;
            krnArg.pData    = &m_surfType3DLut;
            m_renderHdr3DLutParams.kernelArgs.push_back(krnArg);

            krnArg.uIndex   = 1;
            krnArg.eArgKind = ARG_KIND_SURFACE;
            krnArg.uSize    = 4;
            krnArg.pData    = &m_surfType3DLutCoef;
            m_renderHdr3DLutParams.kernelArgs.push_back(krnArg);

            krnArg.uIndex   = 2;
            krnArg.eArgKind = ARG_KIND_GENERAL;
            krnArg.uSize    = 2;
            krnArg.pData    = &m_3DLutSurfaceWidth;
            m_renderHdr3DLutParams.kernelArgs.push_back(krnArg);

            krnArg.uIndex                   = 3;
            krnArg.eArgKind                 = ARG_KIND_GENERAL;
            krnArg.uSize                    = 2;
            krnArg.pData                    = &m_3DLutSurfaceHeight;
            m_renderHdr3DLutParams.kernelId = (VpKernelID)kernelHdr3DLutCalc;
            m_renderHdr3DLutParams.kernelArgs.push_back(krnArg);
        }
        else
        {
            VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
            VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);

            auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find("fillLutTable_3dlut");
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
            KERNEL_ARGS kernelArgs             = handle->second.GetKernelArgs();
            uint32_t    localWidth             = 128;
            uint32_t    localHeight            = 1;
            uint32_t    localDepth             = 1;
            m_renderHdr3DLutParams.localWidth  = localWidth;
            m_renderHdr3DLutParams.localHeight = localHeight;
            m_renderHdr3DLutParams.kernelId    = (VpKernelID)kernelHdr3DLutCalcOcl;

            //step 1: setting curbe arguments
            for (auto const &kernelArg : kernelArgs)
            {
                uint32_t uIndex    = kernelArg.uIndex;
                auto     argHandle = m_renderHdr3DLutOclParams.find(uIndex);
                if (argHandle == m_renderHdr3DLutOclParams.end())
                {
                    KRN_ARG krnArg = {};
                    argHandle      = m_renderHdr3DLutOclParams.insert(std::make_pair(uIndex, krnArg)).first;
                    VP_PUBLIC_CHK_NOT_FOUND_RETURN(argHandle, &m_renderHdr3DLutOclParams);
                }
                KRN_ARG &krnArg = argHandle->second;
                krnArg.uIndex   = uIndex;
                bool bInit      = true;
                if (krnArg.pData == nullptr)
                {
                    if (kernelArg.uSize > 0)
                    {
                        krnArg.uSize = kernelArg.uSize;
                        krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
                    }
                }
                else
                {
                    VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
                    MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
                }
                uint16_t mulSize = hdrParams.lutSize == 65 ? 128 : 64;
                krnArg.eArgKind  = kernelArg.eArgKind;
                switch (krnArg.uIndex)
                {
                case LUT_FILLLUTTABLE_IOLUTINDEX:
                    VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                    *(uint32_t *)krnArg.pData = SurfaceType3DLut;
                    break;
                case LUT_FILLLUTTABLE_ICOEFINDEX:
                    VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                    *(uint32_t *)krnArg.pData = SurfaceType3DLutCoef;
                    break;
                case LUT_FILLLUTTABLE_LUTSIZE:
                    VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                    MOS_SecureMemcpy(krnArg.pData, kernelArg.uSize, &hdrParams.lutSize, sizeof(uint16_t));
                    break;
                case LUT_FILLLUTTABLE_MULSIZE:
                    VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                    MOS_SecureMemcpy(krnArg.pData, kernelArg.uSize, &mulSize, sizeof(uint16_t));
                    break;
                case LUT_FILLLUTTABLE_LOCAL_SIZE:
                    VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                    static_cast<uint32_t *>(krnArg.pData)[0] = localWidth;
                    static_cast<uint32_t *>(krnArg.pData)[1] = localHeight;
                    static_cast<uint32_t *>(krnArg.pData)[2] = localDepth;
                    break;
                default:
                    bInit = false;
                    break;
                }
                if (bInit)
                {
                    m_renderHdr3DLutParams.kernelArgs.push_back(krnArg);
                }
            }
        }
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Not implement on render path for Hdr yet");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   HwFilter Hdr Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterHdrParameter::Create(HW_FILTER_HDR_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterHdrParameter *p = MOS_New(HwFilterHdrParameter, featureType);
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            MOS_Delete(p);
            return nullptr;
        }
    }
    return p;
}

HwFilterHdrParameter::HwFilterHdrParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterHdrParameter::~HwFilterHdrParameter()
{
}

MOS_STATUS HwFilterHdrParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterHdrParameter::Initialize(HW_FILTER_HDR_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Vebox Hdr Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpVeboxHdrParameter::Create(HW_FILTER_HDR_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpVeboxHdrParameter *p = dynamic_cast<VpVeboxHdrParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter *pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}

VpVeboxHdrParameter::VpVeboxHdrParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_HdrFilter(pHwInterface)
{
}
VpVeboxHdrParameter::~VpVeboxHdrParameter() {}

bool VpVeboxHdrParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VEBOX_HDR_PARAMS *pParams = m_HdrFilter.GetVeboxParams();
    if (nullptr == pParams)
    {
        VP_PUBLIC_ASSERTMESSAGE("Failed to get vebox hdr params");
        return false;
    }

    VpVeboxCmdPacketBase *packet = dynamic_cast<VpVeboxCmdPacketBase *>(pPacket);
    if (packet)
    {
        return MOS_SUCCEEDED(packet->SetHdrParams(pParams));
    }

    VP_PUBLIC_ASSERTMESSAGE("Invalid packet for Vebox hdr");
    return false;

}

MOS_STATUS VpVeboxHdrParameter::Initialize(HW_FILTER_HDR_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.CalculateEngineParams(params.hdrParams, params.vpExecuteCaps));
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Vebox Hdr Handler                                         */
/****************************************************************************************************/
PolicyVeboxHdrHandler::PolicyVeboxHdrHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeHdrOnVebox;
}
PolicyVeboxHdrHandler::~PolicyVeboxHdrHandler()
{
}

bool PolicyVeboxHdrHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    //Need to check if other path activated
    return vpExecuteCaps.bHDR3DLUT;
}

HwFilterParameter *PolicyVeboxHdrHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterHdr *swFilter = dynamic_cast<SwFilterHdr *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeHdrOnVebox));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamHdr &param = swFilter->GetSwFilterParams();

        HW_FILTER_HDR_PARAM paramHdr = {};
        paramHdr.type                = m_Type;
        paramHdr.pHwInterface        = pHwInterface;
        paramHdr.vpExecuteCaps       = vpExecuteCaps;
        paramHdr.pPacketParamFactory = &m_PacketParamFactory;
        paramHdr.hdrParams           = param;
        paramHdr.pfnCreatePacketParam = PolicyVeboxHdrHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterHdrParameter *)pHwFilterParam)->Initialize(paramHdr)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterHdrParameter::Create(paramHdr, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}


/****************************************************************************************************/
/*                      Packet Render Hdr 3DLut Caculation Parameter                                */
/****************************************************************************************************/
VpPacketParameter *VpRenderHdr3DLutCalParameter::Create(HW_FILTER_HDR_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderHdr3DLutCalParameter *p = dynamic_cast<VpRenderHdr3DLutCalParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter *pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}

VpRenderHdr3DLutCalParameter::VpRenderHdr3DLutCalParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_HdrFilter(pHwInterface)
{
}
VpRenderHdr3DLutCalParameter::~VpRenderHdr3DLutCalParameter() {}

bool VpRenderHdr3DLutCalParameter::SetPacketParam(VpCmdPacket *packet)
{
    VP_FUNC_CALL();

    VpRenderCmdPacket *pRenderPacket = dynamic_cast<VpRenderCmdPacket *>(packet);
    if (nullptr == pRenderPacket)
    {
        return false;
    }

    RENDER_HDR_3DLUT_CAL_PARAMS *params = m_HdrFilter.GetRenderHdr3DLutParams();
    if (nullptr == params)
    {
        return false;
    }
    return MOS_SUCCEEDED(pRenderPacket->SetHdr3DLutParams(params));
}

MOS_STATUS VpRenderHdr3DLutCalParameter::Initialize(HW_FILTER_HDR_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_HdrFilter.CalculateEngineParams(params.hdrParams, params.vpExecuteCaps));
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                            Policy Render Hdr 3DLut Caculation Handler                            */
/****************************************************************************************************/
PolicyRenderHdr3DLutCalHandler::PolicyRenderHdr3DLutCalHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeHdr3DLutCalOnRender;
}
PolicyRenderHdr3DLutCalHandler::~PolicyRenderHdr3DLutCalHandler()
{
}

bool PolicyRenderHdr3DLutCalHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    //Need to check if other path activated
    return vpExecuteCaps.b3DLutCalc;
}

HwFilterParameter *PolicyRenderHdr3DLutCalHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterHdr *swFilter = dynamic_cast<SwFilterHdr *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeHdr3DLutCalOnRender));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamHdr &param = swFilter->GetSwFilterParams();

        HW_FILTER_HDR_PARAM paramHdr  = {};
        paramHdr.type                 = m_Type;
        paramHdr.pHwInterface         = pHwInterface;
        paramHdr.vpExecuteCaps        = vpExecuteCaps;
        paramHdr.pPacketParamFactory  = &m_PacketParamFactory;
        paramHdr.hdrParams            = param;
        paramHdr.pfnCreatePacketParam = PolicyRenderHdr3DLutCalHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterHdrParameter *)pHwFilterParam)->Initialize(paramHdr)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterHdrParameter::Create(paramHdr, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyRenderHdr3DLutCalHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterHdr *featureHdr = dynamic_cast<SwFilterHdr *>(&feature);
    VP_PUBLIC_CHK_NULL_RETURN(featureHdr);

    // only stage in 3DLUT_KERNEL need update engine caps
    if (caps.b3DLutCalc && featureHdr->GetSwFilterParams().stage == HDR_STAGE_3DLUT_KERNEL)
    {
        SwFilterHdr *filter2ndPass = featureHdr;
        SwFilterHdr *filter1ndPass = (SwFilterHdr *)feature.Clone();

        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        VP_PUBLIC_CHK_NULL_RETURN(filter2ndPass);

        filter1ndPass->GetFilterEngineCaps() = filter2ndPass->GetFilterEngineCaps();
        filter1ndPass->SetFeatureType(filter2ndPass->GetFeatureType());

        FeatureParamHdr &params2ndPass = filter2ndPass->GetSwFilterParams();
        FeatureParamHdr &params1stPass = filter1ndPass->GetSwFilterParams();

        params2ndPass.stage = HDR_STAGE_VEBOX_3DLUT_UPDATE;

        // Clear engine caps for filter in 2nd pass.
        filter2ndPass->SetFeatureType(FeatureTypeHdr);
        filter2ndPass->SetRenderTargetType(RenderTargetTypeSurface);
        filter2ndPass->GetFilterEngineCaps().bEnabled     = 1;
        filter2ndPass->GetFilterEngineCaps().RenderNeeded = 0;
        filter2ndPass->GetFilterEngineCaps().VeboxNeeded  = 1;
        filter2ndPass->GetFilterEngineCaps().isolated     = 0;
        if (featureHdr->GetSwFilterParams().is3DLutKernelOnly)
        {
            filter2ndPass->GetFilterEngineCaps().forceBypassWorkload = 1;
        }
        if (featureHdr->GetSwFilterParams().formatOutput == Format_A8B8G8R8 || featureHdr->GetSwFilterParams().formatOutput == Format_A8R8G8B8)
        {
            filter2ndPass->GetFilterEngineCaps().VeboxARGBOut = 1;
        }
        else if (featureHdr->GetSwFilterParams().formatOutput == Format_B10G10R10A2 || featureHdr->GetSwFilterParams().formatOutput == Format_R10G10B10A2)
        {
            filter2ndPass->GetFilterEngineCaps().VeboxARGB10bitOutput = 1;
        }

        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }

    return MOS_STATUS_SUCCESS;
}
}  // namespace vp
