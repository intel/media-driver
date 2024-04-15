/*
* Copyright (c) 2020-2024, Intel Corporation
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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    return false;
}

MOS_STATUS SwFilterFeatureHandler::CreateSwFilter(SwFilter*& swFilter, VEBOX_SFC_PARAMS& params)
{
    VP_FUNC_CALL();

    swFilter = nullptr;
    if (!IsFeatureEnabled(params))
    {
        // nullptr == swFilter means no such feature in params, which is also the valid case.
        return MOS_STATUS_SUCCESS;
    }
    swFilter = CreateSwFilter();
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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    return true;
}

void SwFilterCscHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    return true;
}

void SwFilterRotMirHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    return true;
}

int SwFilterScalingHandler::GetPipeCountForProcessing(VP_PIPELINE_PARAMS& params)
{
    VP_FUNC_CALL();

    // For interlaced scaling field-to-interleave mode, need two submission for top field and bottom field,
    // thus we need 2 pipe to handle it.
    if (params.pSrc[0] && params.pSrc[0]->InterlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED &&
        params.pSrc[0]->pBwdRef != nullptr)
    {
        return 2;
    }
    return 1;
}

MOS_STATUS SwFilterScalingHandler::UpdateParamsForProcessing(VP_PIPELINE_PARAMS& params, int index)
{
    VP_FUNC_CALL();

    // For second submission of field-to-interleaved mode, we will take second field as input surface,
    // second field is stored in pBwdRef.
    if (params.pSrc[0] && params.pSrc[0]->InterlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED && index == 1)
    {
        if (index >= GetPipeCountForProcessing(params))
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        if (params.pSrc[0] && params.pSrc[0]->pBwdRef)
        {
            params.pSrc[0]->pBwdRef->ScalingMode = params.pSrc[0]->ScalingMode;
            params.pSrc[0]->pBwdRef->SurfType    = params.pSrc[0]->SurfType;
            params.pSrc[0]->pBwdRef->InterlacedScalingType = params.pSrc[0]->InterlacedScalingType;
            if (params.pSrc[0]->SampleType == SAMPLE_SINGLE_TOP_FIELD)
            {
                params.pSrc[0]->pBwdRef->SampleType = SAMPLE_SINGLE_BOTTOM_FIELD;
                params.pTarget[0]->SampleType       = SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD;
            }
            else
            {
                params.pSrc[0]->pBwdRef->SampleType = SAMPLE_SINGLE_TOP_FIELD;
                params.pTarget[0]->SampleType       = SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD;
            }
            if (params.pSrc[0]->pBwdRef->pDeinterlaceParams)
            {
                MOS_FreeMemAndSetNull(params.pSrc[0]->pBwdRef->pDeinterlaceParams);
            }
            params.pSrc[0] = params.pSrc[0]->pBwdRef;
        }
    }

    return MOS_STATUS_SUCCESS;
}

void SwFilterScalingHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    PVP_MHWINTERFACE hwInterface = m_vpInterface.GetHwInterface();
    // secure mode
    if (hwInterface->m_osInterface->osCpInterface &&
        (hwInterface->m_osInterface->osCpInterface->IsHMEnabled() 
            || hwInterface->m_osInterface->osCpInterface->IsIDMEnabled()))
    {
        VP_PUBLIC_NORMALMESSAGE("Dn is disabled in secure mode.");
        return false;
    }
    // clear mode
    else
    {
        auto userFeatureControl = hwInterface->m_userFeatureControl;
        if (userFeatureControl != nullptr)
        {
            bool disableDn = userFeatureControl->IsDisableDn();
            if (disableDn)
            {
                VP_PUBLIC_NORMALMESSAGE("Dn is disabled in clear mode.");
                return false;
            }
        }
       
    }

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE inputSurface = params.pSrc[surfIndex];

    if (inputSurface                               &&
        (inputSurface->Format == Format_A8R8G8B8   ||
        inputSurface->Format == Format_A16R16G16B16))
    {
        VP_PUBLIC_NORMALMESSAGE("Unsupported Format '0x%08x' which DN will not supported", inputSurface->Format);
        return false;
    }

    // Disable VP features DN for resolution > 4K
    if (inputSurface &&
        (inputSurface->rcSrc.bottom > inputSurface->rcSrc.top + VPHAL_RNDR_4K_MAX_HEIGHT ||
         inputSurface->rcSrc.right > inputSurface->rcSrc.left + VPHAL_RNDR_4K_MAX_WIDTH))
    {
        VP_PUBLIC_NORMALMESSAGE("DN is disabled for 4K+ res");
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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pDeinterlaceParams && vphalSurf->SampleType != SAMPLE_PROGRESSIVE)
    {
        return true;
    }

    if (vphalSurf && vphalSurf->bQueryVariance &&
        vphalSurf->Format != Format_P010 &&
        vphalSurf->Format != Format_P016)
    {
        VP_PUBLIC_NORMALMESSAGE("Query Variance is enabled, but APG didn't support this feature yet");
    }

    return false;
}

SwFilter* SwFilterDiHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pColorPipeParams &&
            (vphalSurf->pColorPipeParams->bEnableSTE ||
        vphalSurf->pColorPipeParams->bEnableSTD))
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterSteHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pProcampParams &&
        !IS_RGB_FORMAT(vphalSurf->Format)      &&
        vphalSurf->pProcampParams->bEnabled)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterProcampHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    // Avoid recheck when it is output or surfIndex > 0
    if (!isInputSurf)
    {
        return false;
    }
    // if  target surf is invalid, return false;
    PVPHAL_SURFACE pSrc            = params.pSrc[0];
    PVPHAL_SURFACE pRenderTarget   = params.pTarget[0];
    auto           userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    if (!pSrc || !pRenderTarget)
    {
        return false;
    }

    bool bBt2020Output       = false;
    bool bToneMapping        = false;
    bool bMultiLayerBt2020   = false;
    bool bFP16Format         = false;
    // Not all FP16 input / output need HDR processing, e.g, FP16 by pass, FP16 csc etc.
    bool bFP16HdrProcessing  = false;
    bool isExternal3DLutEnabled     = false;
    if (pSrc->p3DLutParams && userFeatureControl->IsExternal3DLutSupport())
    {
        isExternal3DLutEnabled = true;
    }
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
        bFP16Format = true;
    }

    bFP16Format = bFP16Format || (pRenderTarget->Format == Format_A16B16G16R16F) || (pRenderTarget->Format == Format_A16R16G16B16F);

    if (bFP16Format && !pSrc->p3DLutParams)
    {
        // Check if input/output gamma is same(TBD)
        // Check if input/output color space is same
        bool bColorSpaceConversion = true;
        if (IS_COLOR_SPACE_BT2020(pRenderTarget->ColorSpace) &&
            IS_COLOR_SPACE_BT2020(pSrc->ColorSpace))
        {
            bColorSpaceConversion = false;
        }
        if ((pRenderTarget->ColorSpace == CSpace_sRGB || pRenderTarget->ColorSpace == CSpace_stRGB) &&
            (pSrc->ColorSpace == CSpace_BT709 || pSrc->ColorSpace == CSpace_BT709_FullRange))
        {
            bColorSpaceConversion = false;
        }
        if ((pRenderTarget->ColorSpace == CSpace_sRGB || pRenderTarget->ColorSpace == CSpace_stRGB) &&
            (pSrc->ColorSpace == CSpace_BT601 || pSrc->ColorSpace == CSpace_BT601_FullRange))
        {
            bColorSpaceConversion = false;
        }
        bFP16HdrProcessing = bColorSpaceConversion;
    }

    // Temorary solution for menu/FBI not show up : route all S2S uage to HDR kernel path, need to consider RenderBlockedFromCp

    return (bBt2020Output || bToneMapping || bMultiLayerBt2020 || bFP16HdrProcessing || isExternal3DLutEnabled);
}

SwFilter *SwFilterHdrHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    SwFilterHdr *filter = nullptr;
    filter              = dynamic_cast<SwFilterHdr *>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterLumakeyHandler                                      */
/****************************************************************************************************/

SwFilterLumakeyHandler::SwFilterLumakeyHandler(VpInterface &vpInterface, FeatureType featureType) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeLumakey),
    m_swFilterFactory(vpInterface)
{}
SwFilterLumakeyHandler::~SwFilterLumakeyHandler()
{}

bool SwFilterLumakeyHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE surf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (surf && surf->pLumaKeyParams)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterLumakeyHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(m_type);
    }

    return swFilter;
}

void SwFilterLumakeyHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

    SwFilterLumakey* filter = nullptr;
    filter = dynamic_cast<SwFilterLumakey*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterBlendingHandler                                     */
/****************************************************************************************************/

SwFilterBlendingHandler::SwFilterBlendingHandler(VpInterface &vpInterface, FeatureType featureType) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeBlending),
    m_swFilterFactory(vpInterface)
{}
SwFilterBlendingHandler::~SwFilterBlendingHandler()
{}

bool SwFilterBlendingHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE surf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (surf && surf->pBlendingParams)
    {
        if (!isInputSurf)
        {
            VP_PUBLIC_NORMALMESSAGE("Skip blending parameters on target.");
            return false;
        }
        return true;
    }

    return false;
}

SwFilter* SwFilterBlendingHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(m_type);
    }

    return swFilter;
}

void SwFilterBlendingHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

    SwFilterBlending* filter = nullptr;
    filter = dynamic_cast<SwFilterBlending*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}


/****************************************************************************************************/
/*                                      SwFilterColorFillHandler                                    */
/****************************************************************************************************/

SwFilterColorFillHandler::SwFilterColorFillHandler(VpInterface &vpInterface, FeatureType featureType) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeColorFill),
    m_swFilterFactory(vpInterface)
{}
SwFilterColorFillHandler::~SwFilterColorFillHandler()
{}

bool SwFilterColorFillHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (!isInputSurf && params.pColorFillParams)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterColorFillHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(m_type);
    }

    return swFilter;
}

void SwFilterColorFillHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

    SwFilterColorFill* filter = nullptr;
    filter = dynamic_cast<SwFilterColorFill*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterAlphaHandler                                        */
/****************************************************************************************************/

SwFilterAlphaHandler::SwFilterAlphaHandler(VpInterface &vpInterface, FeatureType featureType) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeAlpha),
    m_swFilterFactory(vpInterface)
{}
SwFilterAlphaHandler::~SwFilterAlphaHandler()
{}

bool SwFilterAlphaHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (!isInputSurf && params.pCompAlpha)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterAlphaHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(m_type);
    }

    return swFilter;
}

void SwFilterAlphaHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

    SwFilterAlpha* filter = nullptr;
    filter = dynamic_cast<SwFilterAlpha*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterCgcHandler                                          */
/****************************************************************************************************/

SwFilterCgcHandler::SwFilterCgcHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeCgc),
    m_swFilterFactory(vpInterface)
{}
SwFilterCgcHandler::~SwFilterCgcHandler()
{}

bool SwFilterCgcHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    VP_FUNC_CALL();

    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    // BT2020YUV->BT601/709YUV enable GC in Vebox for BT2020 to RGB process
    // SFC for other format conversion. For such case, add Cgc only to input
    // pipe for 1 to 1 or N to 1 case.
    if (isInputSurf && (SwFilterPipeType1ToN == pipeType) ||
        !isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE inputSurf  = static_cast<PVPHAL_SURFACE>(isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0]);
    PVPHAL_SURFACE outputSurf = static_cast<PVPHAL_SURFACE>(isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex]);

    if (inputSurf && outputSurf &&
        IS_COLOR_SPACE_BT2020_YUV(inputSurf->ColorSpace) &&
      (!(inputSurf->pHDRParams && 
        (inputSurf->pHDRParams->EOTF != VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR) &&
       !(outputSurf->pHDRParams && 
        (outputSurf->pHDRParams->EOTF != VPHAL_HDR_EOTF_TRADITIONAL_GAMMA_SDR))))) // When HDR Enabled, GC should always be turn off, not to create the sw filter
    {
        if ((outputSurf->ColorSpace == CSpace_BT601) ||
            (outputSurf->ColorSpace == CSpace_BT709) ||
            (outputSurf->ColorSpace == CSpace_BT601_FullRange) ||
            (outputSurf->ColorSpace == CSpace_BT709_FullRange) ||
            (outputSurf->ColorSpace == CSpace_stRGB) ||
            (outputSurf->ColorSpace == CSpace_sRGB))
        {
            return true;
        }
    }

    return false;
}

SwFilter* SwFilterCgcHandler::CreateSwFilter()
{
    VP_FUNC_CALL();

    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeCgc);
    }

    return swFilter;
}

void SwFilterCgcHandler::Destory(SwFilter*& swFilter)
{
    VP_FUNC_CALL();

    SwFilterCgc* filter = nullptr;
    filter = dynamic_cast<SwFilterCgc*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}
