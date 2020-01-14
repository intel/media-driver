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
//! \file     vp_csc_filter.h
//! \brief    Defines the common interface for CSC
//!           this file is for the base interface which is shared by all CSC in driver.
//!

#include "vp_csc_filter.h"
#include "vp_vebox_cmd_packet.h"
#include "hw_filter.h"
#include "sw_filter_pipe.h"

namespace vp {
VpCscFilter::VpCscFilter(PVP_MHWINTERFACE vpMhwInterface) :
    VpFilter(vpMhwInterface)
{

}

MOS_STATUS VpCscFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_sfcCSCParams)
    {
        MOS_FreeMemory(m_sfcCSCParams);
        m_sfcCSCParams = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetExecuteEngineCaps(
    FeatureParamCsc &cscParams,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_cscParams = cscParams;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();

    if (m_executeCaps.bSFC)
    {
        if (!m_sfcCSCParams)
        {
            m_sfcCSCParams = (PSFC_CSC_PARAMS)MOS_AllocAndZeroMemory(sizeof(SFC_CSC_PARAMS));

            if (m_sfcCSCParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("sfc CSC Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            MOS_ZeroMemory(m_sfcCSCParams, sizeof(SFC_CSC_PARAMS));
        }

        m_sfcCSCParams->bIEFEnable = (m_cscParams.pIEFParams                 &&
                                      m_cscParams.pIEFParams->bEnabled       &&
                                      m_cscParams.pIEFParams->fIEFFactor > 0.0F) ? true : false;

        if (m_sfcCSCParams->bIEFEnable)
        {
            m_sfcCSCParams->iefParams = m_cscParams.pIEFParams;
        }

        m_sfcCSCParams->inputColorSpcase = m_cscParams.colorSpaceInput;
        m_sfcCSCParams->inputFormat      = m_executeCaps.bDI ? Format_YUY2 : m_cscParams.formatInput;
        m_sfcCSCParams->outputFormat     = m_cscParams.formatOutput;

        if (m_sfcCSCParams->inputColorSpcase != m_cscParams.colorSpaceOutput)
        {
            m_sfcCSCParams->bCSCEnabled = true;
        }

        if (IS_RGB_CSPACE(m_cscParams.colorSpaceInput))
        {
            m_sfcCSCParams->bInputColorSpace = true;
        }
        else
        {
            m_sfcCSCParams->bInputColorSpace = false;
        }
        // Set Chromasting Params
        VP_RENDER_CHK_STATUS_RETURN(SetChromaParams(m_executeCaps));
    }
    else
    {
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetChromaParams(
    VP_EXECUTE_CAPS         vpExecuteCaps)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcCSCParams);

    // Setup General params
    // Set chroma subsampling type according to the Vebox output, but
    // when Vebox is bypassed, set it according to the source surface format.
    m_sfcCSCParams->inputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_400;
    m_sfcCSCParams->b8tapChromafiltering   = false;

    if (vpExecuteCaps.bIECP)
    {
        m_sfcCSCParams->inputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;
        m_sfcCSCParams->sfcSrcChromaSiting     = MHW_CHROMA_SITING_HORZ_LEFT | MHW_CHROMA_SITING_VERT_TOP;
        m_sfcCSCParams->b8tapChromafiltering = true;
    }
    else if (vpExecuteCaps.bDI)
    {
        m_sfcCSCParams->inputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H;
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(SetSubSampling());
    }
    
    m_sfcCSCParams->chromaDownSamplingVerticalCoef = (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_VERT_CENTER) ?
        MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8;
    m_sfcCSCParams->chromaDownSamplingHorizontalCoef = (m_cscParams.chromaSitingOutput & MHW_CHROMA_SITING_HORZ_CENTER) ?
        MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_4_OVER_8 : MEDIASTATE_SFC_CHROMA_DOWNSAMPLING_COEF_0_OVER_8;

    m_sfcCSCParams->bChromaUpSamplingEnable = IsChromaUpSamplingNeeded();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCscFilter::SetSubSampling()
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_RETURN(m_sfcCSCParams);

    if (m_sfcCSCParams->inputFormat  == Format_NV12 ||
        (m_sfcCSCParams->inputFormat == Format_P010) ||
        (m_sfcCSCParams->inputFormat == Format_P016))
    {
        m_sfcCSCParams->inputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_420;
    }
    else if (VpHal_GetSurfaceColorPack(m_sfcCSCParams->inputFormat) == VPHAL_COLORPACK_422)
    {
        m_sfcCSCParams->inputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_422H;
    }
    else if (VpHal_GetSurfaceColorPack(m_sfcCSCParams->inputFormat) == VPHAL_COLORPACK_444)
    {
        m_sfcCSCParams->inputChromaSubSampling = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_444;
        m_sfcCSCParams->b8tapChromafiltering = true;
    }

    m_sfcCSCParams->sfcSrcChromaSiting = m_cscParams.chromaSitingInput;

    if (m_sfcCSCParams->sfcSrcChromaSiting == MHW_CHROMA_SITING_NONE)
    {
        m_sfcCSCParams->sfcSrcChromaSiting = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
    }

    switch (VpHal_GetSurfaceColorPack(m_sfcCSCParams->inputFormat))
    {
    case VPHAL_COLORPACK_422:
        m_sfcCSCParams->sfcSrcChromaSiting = (m_sfcCSCParams->sfcSrcChromaSiting & 0x7) | CHROMA_SITING_VERT_TOP;
        break;
    case VPHAL_COLORPACK_444:
        m_sfcCSCParams->sfcSrcChromaSiting = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
        break;
    default:
        break;
    }

    // Prevent invalid input for output surface and format
    if (m_cscParams.chromaSitingOutput == MHW_CHROMA_SITING_NONE)
    {
        m_cscParams.chromaSitingOutput = (CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_CENTER);
    }
    switch (VpHal_GetSurfaceColorPack(m_cscParams.formatOutput))
    {
    case VPHAL_COLORPACK_422:
        m_cscParams.chromaSitingOutput = (m_cscParams.chromaSitingOutput & 0x7) | CHROMA_SITING_VERT_TOP;
        break;
    case VPHAL_COLORPACK_444:
        m_cscParams.chromaSitingOutput = CHROMA_SITING_HORZ_LEFT | CHROMA_SITING_VERT_TOP;
        break;
    default:
        break;
    }

    return MOS_STATUS_SUCCESS;
}

bool VpCscFilter::IsChromaUpSamplingNeeded()
{
    bool                  bChromaUpSampling = false;
    VPHAL_COLORPACK       srcColorPack, dstColorPack;

    bChromaUpSampling = false;

    srcColorPack = VpHal_GetSurfaceColorPack(m_cscParams.formatInput);
    dstColorPack = VpHal_GetSurfaceColorPack(m_cscParams.formatOutput);

    if ((srcColorPack == VPHAL_COLORPACK_420 &&
        (dstColorPack == VPHAL_COLORPACK_422 || dstColorPack == VPHAL_COLORPACK_444)) ||
        (srcColorPack == VPHAL_COLORPACK_422 && dstColorPack == VPHAL_COLORPACK_444))
    {
        bChromaUpSampling = true;
    }

    return bChromaUpSampling;
}

/****************************************************************************************************/
/*                                   HwFilter Csc Parameter                                         */
/****************************************************************************************************/
HwFilterParameter *HwFilterCscParameter::Create(HW_FILTER_CSC_PARAM &param, FeatureType featureType)
{
    HwFilterCscParameter *p = MOS_New(HwFilterCscParameter, featureType);
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

HwFilterCscParameter::HwFilterCscParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterCscParameter::~HwFilterCscParameter()
{
}

MOS_STATUS HwFilterCscParameter::ConfigParams(HwFilter &hwFilter)
{
    return hwFilter.ConfigCscParam(m_Params);
}

MOS_STATUS HwFilterCscParameter::Initialize(HW_FILTER_CSC_PARAM &param)
{
    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Sfc Csc Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpSfcCscParameter::Create(HW_FILTER_CSC_PARAM &param)
{
    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpSfcCscParameter *p = dynamic_cast<VpSfcCscParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
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

VpSfcCscParameter::VpSfcCscParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) :
    VpPacketParameter(packetParamFactory), m_CscFilter(pHwInterface)
{
}
VpSfcCscParameter::~VpSfcCscParameter() {}

bool VpSfcCscParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VpVeboxCmdPacket *pVeboxPacket = dynamic_cast<VpVeboxCmdPacket *>(pPacket);
    if (nullptr == pVeboxPacket)
    {
        return false;
    }

    SFC_CSC_PARAMS *pParams = m_CscFilter.GetSfcParams();
    if (nullptr == pParams)
    {
        return false;
    }
    return MOS_SUCCEEDED(pVeboxPacket->SetSfcCSCParams(pParams));
}

MOS_STATUS VpSfcCscParameter::Initialize(HW_FILTER_CSC_PARAM &params)
{
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.SetExecuteEngineCaps(params.cscParams, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_CscFilter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Sfc Csc Handler                                         */
/****************************************************************************************************/
PolicySfcCscHandler::PolicySfcCscHandler()
{
    m_Type = FeatureTypeCscOnSfc;
}
PolicySfcCscHandler::~PolicySfcCscHandler()
{
}

bool PolicySfcCscHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bSfcCsc;
}

HwFilterParameter *PolicySfcCscHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    if (IsFeatureEnabled(vpExecuteCaps))
    {
        if (SwFilterPipeType1To1 != swFilterPipe.GetSwFilterPipeType())
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Sfc only support 1To1 swFilterPipe!");
            return nullptr;
        }

        SwFilterCsc *swFilter = dynamic_cast<SwFilterCsc *>(swFilterPipe.GetSwFilter(true, 0, FeatureTypeCscOnSfc));

        if (nullptr == swFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! Feature enabled in vpExecuteCaps but no swFilter exists!");
            return nullptr;
        }

        FeatureParamCsc &param = swFilter->GetSwFilterParams();

        HW_FILTER_CSC_PARAM paramCsc = {};
        paramCsc.type = m_Type;
        paramCsc.pHwInterface = pHwInterface;
        paramCsc.vpExecuteCaps = vpExecuteCaps;
        paramCsc.pPacketParamFactory = &m_PacketParamFactory;
        paramCsc.cscParams = param;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterCscParameter*)pHwFilterParam)->Initialize(paramCsc)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterCscParameter::Create(paramCsc, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}
}
