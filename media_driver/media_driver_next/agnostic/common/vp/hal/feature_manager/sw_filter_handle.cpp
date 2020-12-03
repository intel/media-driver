/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     sw_filter_handle.cpp
//! \brief    Factories for vp sw filter handle creation.
//!

#include "sw_filter_handle.h"

using namespace vp;
/****************************************************************************************************/
/*                                      SwFilterFeatureHandler                                      */
/****************************************************************************************************/

SwFilterFeatureHandler::SwFilterFeatureHandler(VpInterface& vpInterface, FeatureType type) : m_vpInterface(vpInterface), m_type(type)
{}

SwFilterFeatureHandler::~SwFilterFeatureHandler()
{}

bool SwFilterFeatureHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (isInputSurf && (uint32_t)surfIndex >= params.uSrcCount ||
        !isInputSurf && (uint32_t)surfIndex >= params.uDstCount)
    {
        // Invalid parameters.
        VP_PUBLIC_ASSERTMESSAGE("Surface index is bigger than surface count!");
        return false;
    }
    return true;
}

MOS_STATUS SwFilterFeatureHandler::CreateSwFilter(SwFilter*& swFilter, VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    swFilter = nullptr;
    if (!IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        // nullptr == swFilter means no such feature in params, which is also the valid case.
        return MOS_STATUS_SUCCESS;
    }
    swFilter = CreateSwFilter();
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);
    MOS_STATUS status = swFilter->Configure(params, isInputSurf, surfIndex);
    if (MOS_FAILED(status))
    {
        Destory(swFilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }
    return MOS_STATUS_SUCCESS;
}

bool SwFilterFeatureHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return false;
}

MOS_STATUS SwFilterFeatureHandler::CreateSwFilter(SwFilter*& swFilter, VEBOX_SFC_PARAMS& params)
{
    swFilter = nullptr;
    if (!IsFeatureEnabled(params))
    {
        // nullptr == swFilter means no such feature in params, which is also the valid case.
        return MOS_STATUS_SUCCESS;
    }
    swFilter = CreateSwFilter();;
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);
    MOS_STATUS status = swFilter->Configure(params);
    if (MOS_FAILED(status))
    {
        Destory(swFilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterCscHandler                                          */
/****************************************************************************************************/

SwFilterCscHandler::SwFilterCscHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeCsc),
    m_swFilterFactory(vpInterface)
{
}

SwFilterCscHandler::~SwFilterCscHandler()
{
}

bool SwFilterCscHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType) ||
        !isInputSurf && SwFilterPipeType1ToN == pipeType)
    {
        return true;
    }
    return false;
}

SwFilter* SwFilterCscHandler::CreateSwFilter()
{
    SwFilterCsc* swFilter = nullptr;
    swFilter = dynamic_cast<SwFilterCsc*>(m_swFilterFactory.Create());
    if (swFilter)
    {
        MOS_STATUS status = swFilter->SetFeatureType(FeatureTypeCsc);
        if (MOS_FAILED(status))
        {
            m_swFilterFactory.Destory(swFilter);
            return nullptr;
        }
        return swFilter;
    }
    return nullptr;
}

bool SwFilterCscHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return true;
}

void SwFilterCscHandler::Destory(SwFilter*& swFilter)
{
    SwFilterCsc* filter = nullptr;
    filter = dynamic_cast<SwFilterCsc*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterRotMirHandler                                       */
/****************************************************************************************************/

SwFilterRotMirHandler::SwFilterRotMirHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeRotMir),
    m_swFilterFactory(vpInterface)
{}
SwFilterRotMirHandler::~SwFilterRotMirHandler()
{}

bool SwFilterRotMirHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType) ||
        !isInputSurf && SwFilterPipeType1ToN == pipeType)
    {
        return true;
    }
    return false;
}

SwFilter* SwFilterRotMirHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeRotMir);
    }

    return swFilter;
}

bool SwFilterRotMirHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return true;
}

void SwFilterRotMirHandler::Destory(SwFilter*& swFilter)
{
    SwFilterRotMir* filter = nullptr;
    filter = dynamic_cast<SwFilterRotMir*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterScalingHandler                                      */
/****************************************************************************************************/

SwFilterScalingHandler::SwFilterScalingHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeScaling),
    m_swFilterFactory(vpInterface)
{}
SwFilterScalingHandler::~SwFilterScalingHandler()
{}

bool SwFilterScalingHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType) ||
        !isInputSurf && SwFilterPipeType1ToN == pipeType)
    {
        return true;
    }
    return false;
}

SwFilter* SwFilterScalingHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeScaling);
    }

    return swFilter;
}

bool SwFilterScalingHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return true;
}

int SwFilterScalingHandler::GetPipeCountForProcessing(VP_PIPELINE_PARAMS& params)
{
    // For interlaced scaling field-to-interleave mode, need two submission for top field and bottom field,
    // thus we need 2 pipe to handle it.
    if (params.pSrc[0]->InterlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED &&
        params.pSrc[0]->pBwdRef != nullptr)
    {
        return 2;
    }
    return 1;
}

MOS_STATUS SwFilterScalingHandler::UpdateParamsForProcessing(VP_PIPELINE_PARAMS& params, int index)
{
    if (index >= GetPipeCountForProcessing(params))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // For second submission of field-to-interleaved mode, we will take second field as input surface,
    // second field is stored in pBwdRef.
    if (params.pSrc[0]->InterlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED && index == 1)
    {
        if (params.pSrc[0] && params.pSrc[0]->pBwdRef)
        {
            params.pSrc[0]->pBwdRef->ScalingMode = params.pSrc[0]->ScalingMode;
            params.pSrc[0] = params.pSrc[0]->pBwdRef;
        }
    }

    return MOS_STATUS_SUCCESS;
}

void SwFilterScalingHandler::Destory(SwFilter*& swFilter)
{
    SwFilterScaling* filter = nullptr;
    filter = dynamic_cast<SwFilterScaling*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterDnHandler                                      */
/****************************************************************************************************/

SwFilterDnHandler::SwFilterDnHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeDn),
    m_swFilterFactory(vpInterface)
{}
SwFilterDnHandler::~SwFilterDnHandler()
{}

bool SwFilterDnHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pDenoiseParams &&
        (vphalSurf->pDenoiseParams->bEnableLuma || vphalSurf->pDenoiseParams->bEnableHVSDenoise))
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterDnHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeDn);
    }

    return swFilter;
}

void SwFilterDnHandler::Destory(SwFilter*& swFilter)
{
    SwFilterDenoise* filter = nullptr;
    filter = dynamic_cast<SwFilterDenoise*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterDiHandler                                           */
/****************************************************************************************************/

SwFilterDiHandler::SwFilterDiHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeDi),
    m_swFilterFactory(vpInterface)
{}
SwFilterDiHandler::~SwFilterDiHandler()
{}

bool SwFilterDiHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pDeinterlaceParams && vphalSurf->SampleType != SAMPLE_PROGRESSIVE)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterDiHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeDi);
    }

    return swFilter;
}

void SwFilterDiHandler::Destory(SwFilter*& swFilter)
{
    SwFilterDeinterlace* filter = nullptr;
    filter = dynamic_cast<SwFilterDeinterlace*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterSteHandler                                      */
/****************************************************************************************************/

SwFilterSteHandler::SwFilterSteHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeSte),
    m_swFilterFactory(vpInterface)
{}
SwFilterSteHandler::~SwFilterSteHandler()
{}

bool SwFilterSteHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pColorPipeParams &&
        vphalSurf->pColorPipeParams->bEnableSTE)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterSteHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeSte);
    }

    return swFilter;
}

void SwFilterSteHandler::Destory(SwFilter*& swFilter)
{
    SwFilterSte* filter = nullptr;
    filter = dynamic_cast<SwFilterSte*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterTccHandler                                      */
/****************************************************************************************************/

SwFilterTccHandler::SwFilterTccHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeTcc),
    m_swFilterFactory(vpInterface)
{}
SwFilterTccHandler::~SwFilterTccHandler()
{}

bool SwFilterTccHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pColorPipeParams &&
        vphalSurf->pColorPipeParams->bEnableTCC)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterTccHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeTcc);
    }

    return swFilter;
}

void SwFilterTccHandler::Destory(SwFilter*& swFilter)
{
    SwFilterTcc* filter = nullptr;
    filter = dynamic_cast<SwFilterTcc*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterProcampHandler                                      */
/****************************************************************************************************/

SwFilterProcampHandler::SwFilterProcampHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeProcamp),
    m_swFilterFactory(vpInterface)
{}
SwFilterProcampHandler::~SwFilterProcampHandler()
{}

bool SwFilterProcampHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pProcampParams &&
        vphalSurf->pProcampParams->bEnabled)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterProcampHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeProcamp);
    }

    return swFilter;
}

void SwFilterProcampHandler::Destory(SwFilter*& swFilter)
{
    SwFilterProcamp* filter = nullptr;
    filter = dynamic_cast<SwFilterProcamp*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterHdrHandler                                          */
/****************************************************************************************************/

SwFilterHdrHandler::SwFilterHdrHandler(VpInterface &vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeHdr),
    m_swFilterFactory(vpInterface)
{}
SwFilterHdrHandler::~SwFilterHdrHandler()
{}

bool SwFilterHdrHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    // Avoid recheck when it is output or surfIndex > 0
    if (!isInputSurf || surfIndex > 0)
    {
        return false;
    }
    // if  target surf is invalid, return false;
    PVPHAL_SURFACE pSrc            = params.pSrc[0];
    PVPHAL_SURFACE pRenderTarget   = params.pTarget[0];
    if (!pSrc || !pRenderTarget)
    {
        return false;
    }

    bool bBt2020Output       = false;
    bool bToneMapping        = false;
    bool bMultiLayerBt2020   = false;
    bool bFP16               = false;
    // Need to use HDR to process BT601/BT709->BT2020
    if (IS_COLOR_SPACE_BT2020(pRenderTarget->ColorSpace) &&
        !IS_COLOR_SPACE_BT2020(pSrc->ColorSpace))
    {
        bBt2020Output = true;
    }

    if ((pSrc->pHDRParams && (pSrc->pHDRParams->EOTF != VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR)) ||
        (pRenderTarget->pHDRParams && (pRenderTarget->pHDRParams->EOTF != VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR)))
    {
        bToneMapping = true;
    }

    if ((pSrc->Format == Format_A16B16G16R16F) || (pSrc->Format == Format_A16R16G16B16F))
    {
        bFP16 = true;
    }

    bFP16 = bFP16 || (pRenderTarget->Format == Format_A16B16G16R16F) || (pRenderTarget->Format == Format_A16R16G16B16F);

    // Temorary solution for menu/FBI not show up : route all S2S uage to HDR kernel path, need to consider RenderBlockedFromCp

    return (bBt2020Output || bToneMapping || bMultiLayerBt2020);
}

SwFilter *SwFilterHdrHandler::CreateSwFilter()
{
    SwFilter *swFilter = nullptr;
    swFilter           = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeHdr);
    }

    return swFilter;
}

void SwFilterHdrHandler::Destory(SwFilter *&swFilter)
{
    SwFilterHdr *filter = nullptr;
    filter              = dynamic_cast<SwFilterHdr *>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}
