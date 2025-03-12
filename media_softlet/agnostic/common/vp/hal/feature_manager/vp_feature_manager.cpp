/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     vp_feature_manager.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "vp_feature_manager.h"
#include "sw_filter_handle.h"
using namespace vp;

/****************************************************************************************************/
/*                                      VpFeatureManagerNext                                        */
/****************************************************************************************************/

VpFeatureManagerNext::VpFeatureManagerNext(VpInterface &vpInterface) :
    m_vpInterface(vpInterface)
{
    m_vpInterface.SetSwFilterHandlers(m_featureHandler);
}

VpFeatureManagerNext::~VpFeatureManagerNext()
{
    UnregisterFeatures();
    MOS_Delete(m_policy);
}

MOS_STATUS VpFeatureManagerNext::Init(void* settings)
{
    VP_FUNC_CALL();

    if (!m_policy)
    {
        m_policy = MOS_New(Policy, m_vpInterface);
    }
    VP_PUBLIC_CHK_NULL_RETURN(m_policy);
    vp::VpPlatformInterface *vpPlatformInterface = m_vpInterface.GetHwInterface()->m_vpPlatformInterface;
    VP_PUBLIC_CHK_NULL_RETURN(vpPlatformInterface);
    VP_PUBLIC_CHK_STATUS_RETURN(vpPlatformInterface->InitVpFeatureSupportBits());
    VP_PUBLIC_CHK_STATUS_RETURN(RegisterFeatures());
    return m_policy->Initialize();
}

bool VpFeatureManagerNext::IsVeboxSfcFormatSupported(MOS_FORMAT formatInput, MOS_FORMAT formatOutput)
{
    VP_FUNC_CALL();

    if (m_policy)
    {
        return m_policy->IsVeboxSfcFormatSupported(formatInput, formatOutput);
    }

    return false;
}

MOS_STATUS VpFeatureManagerNext::CreateHwFilterPipe(SwFilterPipe &swFilterPipe, HwFilterPipe *&pHwFilterPipe)
{
    VP_FUNC_CALL();

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    pHwFilterPipe = nullptr;

    status = m_vpInterface.GetHwFilterPipeFactory().Create(swFilterPipe, *m_policy, pHwFilterPipe);

    VP_PUBLIC_CHK_STATUS_RETURN(status);
    VP_PUBLIC_CHK_NULL_RETURN(pHwFilterPipe);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFeatureManagerNext::InitPacketPipe(SwFilterPipe &swFilterPipe,
                PacketPipe &packetPipe)
{
    VP_FUNC_CALL();

    HwFilterPipe *pHwFilterPipe = nullptr;

    MOS_STATUS status = CreateHwFilterPipe(swFilterPipe, pHwFilterPipe);

    VP_PUBLIC_CHK_STATUS_RETURN(status);
    VP_PUBLIC_CHK_NULL_RETURN(pHwFilterPipe);

    status = UpdateResources(*pHwFilterPipe);

    if (MOS_FAILED(status))
    {
        m_vpInterface.GetHwFilterPipeFactory().Destory(pHwFilterPipe);
        return status;
    }

    status = pHwFilterPipe->InitPacketPipe(packetPipe);

    m_vpInterface.GetHwFilterPipeFactory().Destory(pHwFilterPipe);

    return status;
}

MOS_STATUS VpFeatureManagerNext::UpdateResources(HwFilterPipe &hwFilterPipe)
{
    VP_FUNC_CALL();

    return hwFilterPipe.UpdateResources();
}

MOS_STATUS VpFeatureManagerNext::RegisterFeatures()
{
    VP_FUNC_CALL();

    if (m_isFeatureRegistered)
    {
        return MOS_STATUS_SUCCESS;
    }

    // Clear m_featureHandler to avoid any garbage data.
    UnregisterFeatures();

    // Vebox/Sfc features.
    SwFilterFeatureHandler *p = MOS_New(SwFilterCscHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeCsc, p));

    p = MOS_New(SwFilterRotMirHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeRotMir, p));

    p = MOS_New(SwFilterScalingHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeScaling, p));

    p = MOS_New(SwFilterDnHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeDn, p));

    p = MOS_New(SwFilterSteHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeSte, p));

    p = MOS_New(SwFilterTccHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeTcc, p));

    p = MOS_New(SwFilterProcampHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeProcamp, p));

    p = MOS_New(SwFilterHdrHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeHdr, p));

    p = MOS_New(SwFilterDiHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeDi, p));

    p = MOS_New(SwFilterLumakeyHandler, m_vpInterface, FeatureTypeLumakey);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeLumakey, p));

    p = MOS_New(SwFilterBlendingHandler, m_vpInterface, FeatureTypeBlending);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeBlending, p));

    p = MOS_New(SwFilterColorFillHandler, m_vpInterface, FeatureTypeColorFill);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeColorFill, p));

    p = MOS_New(SwFilterAlphaHandler, m_vpInterface, FeatureTypeAlpha);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeAlpha, p));

    p = MOS_New(SwFilterCgcHandler, m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_featureHandler.insert(std::make_pair(FeatureTypeCgc, p));

    //TODO for AI Feature Developer: Register sw filter handler derived from SwFilterAiBaseHanlder for new AI Feature

    m_isFeatureRegistered = true;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFeatureManagerNext::UnregisterFeatures()
{
    VP_FUNC_CALL();

    while (!m_featureHandler.empty())
    {
        auto it = m_featureHandler.begin();
        SwFilterFeatureHandler* p = it->second;
        MOS_Delete(p);
        m_featureHandler.erase(it);
    }
    m_isFeatureRegistered = false;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      VPFeatureManager                                            */
/****************************************************************************************************/

VPFeatureManager::VPFeatureManager(
    PVP_MHWINTERFACE  hwInterface) :
    MediaFeatureManager(),
    m_hwInterface(hwInterface)
{
    if (m_hwInterface && m_hwInterface->m_osInterface)
    {
        m_userSettingPtr = m_hwInterface->m_osInterface->pfnGetUserSettingInstance(m_hwInterface->m_osInterface);
    }
    if (m_hwInterface)
    {
        m_vpUserFeatureControl = m_hwInterface->m_userFeatureControl;
    }
}

MOS_STATUS VPFeatureManager::CheckFeatures(void * params, bool &bApgFuncSupported)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);

    PVP_PIPELINE_PARAMS pvpParams = (PVP_PIPELINE_PARAMS)params;
    bApgFuncSupported = false;

    // Color fill does not need to check src params.
    if (0 == pvpParams->uSrcCount)
    {
        bApgFuncSupported = true;
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(pvpParams->pSrc[0]);
    VP_PUBLIC_CHK_NULL_RETURN(pvpParams->pTarget[0]);

    // align rectangle of surface
    VP_PUBLIC_CHK_STATUS_RETURN(RectSurfaceAlignment(pvpParams->pSrc[0], pvpParams->pTarget[0]->Format));
    VP_PUBLIC_CHK_STATUS_RETURN(RectSurfaceAlignment(pvpParams->pTarget[0], pvpParams->pTarget[0]->Format));

    //Force 8K to render. Handle this case in APG path after render path being enabled.
    if (pvpParams->bDisableVeboxFor8K &&
        ((pvpParams->pSrc[0]->dwWidth >= VPHAL_RNDR_8K_WIDTH || pvpParams->pSrc[0]->dwHeight >= VPHAL_RNDR_8K_HEIGHT) ||
         (pvpParams->pTarget[0]->dwWidth >= VPHAL_RNDR_8K_WIDTH || pvpParams->pTarget[0]->dwHeight >= VPHAL_RNDR_8K_HEIGHT)))
    {
        VP_PUBLIC_NORMALMESSAGE("Disable VEBOX/SFC for 8k resolution");
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pConstriction)
    {
        return MOS_STATUS_SUCCESS;
    }

    bApgFuncSupported = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VPFeatureManager::CheckFeatures(void * params)
{
    VP_FUNC_CALL();

    bool bApgFuncSupported = false;
    return CheckFeatures(params, bApgFuncSupported);
}

bool VPFeatureManager::IsCroppingNeeded(
    PVPHAL_SURFACE pSrc)
{
    VP_FUNC_CALL();

    if (!pSrc)
    {
        return false;
    }
    bool bCropping = false;
    // use comp for cropping
    if (pSrc->rcSrc.left != 0 || pSrc->rcDst.left != 0 ||
        pSrc->rcSrc.top != 0 || pSrc->rcDst.top != 0)
    {
        bCropping = true;
    }
    return bCropping;
}

bool VPFeatureManager::IsHdrNeeded(
    PVPHAL_SURFACE              pSrc,
    PVPHAL_SURFACE              pRenderTarget)
{
    VP_FUNC_CALL();

    if (!pSrc || !pRenderTarget)
    {
        return false;
    }

    if (!MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrHDR))
    {
        return false;
    }

    bool bBt2020Output = false;
    bool bToneMapping = false;
    bool bMultiLayerBt2020 = false;
    bool bBDUHD = false;
    bool bFP16 = false;
    bool bRouteSdrUsageToHdr = false;
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

    //if (IS_COLOR_SPACE_BT2020(pSrc->ColorSpace) && pRenderParams->uSrcCount > 1)
    //{
    //    bMultiLayerBt2020 = true;
    //}
    if (pSrc->pHDRParams)
    {

        if (pSrc->pHDRParams->bPathKernel || (pRenderTarget->pHDRParams && pRenderTarget->pHDRParams->bPathKernel))
        {
            bBDUHD = true;
        }
    }

    if ((pSrc->Format == Format_A16B16G16R16F) || (pSrc->Format == Format_A16R16G16B16F))
    {
        bFP16 = true;
    }

    bFP16 = bFP16 || (pRenderTarget->Format == Format_A16B16G16R16F) || (pRenderTarget->Format == Format_A16R16G16B16F);

    // Temorary solution for menu/FBI not show up : route all S2S uage to HDR kernel path
    if (m_hwInterface->m_osInterface &&
        m_hwInterface->m_osInterface->osCpInterface->RenderBlockedFromCp())
    {
        bRouteSdrUsageToHdr = true;
    }
    return  (bBt2020Output || bToneMapping || bMultiLayerBt2020 || bRouteSdrUsageToHdr);

}

bool VPFeatureManager::Is2PassesCSCNeeded(PVPHAL_SURFACE pSrc, PVPHAL_SURFACE pRenderTarget)
{
    VP_FUNC_CALL();

    bool bRet = false;
    bool b2PassesCSCNeeded = false;

    VP_PUBLIC_CHK_NULL_NO_STATUS(pSrc);
    VP_PUBLIC_CHK_NULL_NO_STATUS(pRenderTarget);

    // 2 Passes CSC is used in BT2020YUV->BT601/709YUV
    // Isolate decoder require SFC output, but SFC can not support RGB input,
    // so sRGB need two pass, that same as original logic.
    if (IS_COLOR_SPACE_BT2020_YUV(pSrc->ColorSpace))
    {
        if ((pRenderTarget->ColorSpace == CSpace_BT601) ||
            (pRenderTarget->ColorSpace == CSpace_BT709) ||
            (pRenderTarget->ColorSpace == CSpace_BT601_FullRange) ||
            (pRenderTarget->ColorSpace == CSpace_BT709_FullRange) ||
            (pRenderTarget->ColorSpace == CSpace_stRGB) ||
            (pRenderTarget->ColorSpace == CSpace_sRGB))
        {
            b2PassesCSCNeeded = true;
        }
    }

    bRet = b2PassesCSCNeeded;

finish:
    return bRet;
}
bool VPFeatureManager::IsVeboxOutFeasible(
    PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();

    bool    bRet = false;
    bool disableVeboxOutput = false;

    VP_PUBLIC_CHK_NULL_NO_STATUS(params);
    VP_PUBLIC_CHK_NULL_NO_STATUS(params->pSrc[0]);
    VP_PUBLIC_CHK_NULL_NO_STATUS(params->pTarget[0]);

    // Read user feature key to get the Composition Bypass mode
    // Vebox Comp Bypass is on by default
    VP_PUBLIC_CHK_NULL_NO_STATUS(m_vpUserFeatureControl);
    disableVeboxOutput  = m_vpUserFeatureControl->IsVeboxOutputDisabled();

    if (!disableVeboxOutput                                                    &&
        params->uDstCount ==1                                                  &&
        SAME_SIZE_RECT(params->pSrc[0]->rcSrc, params->pSrc[0]->rcDst)         &&
        RECT1_CONTAINS_RECT2(params->pSrc[0]->rcMaxSrc, params->pSrc[0]->rcSrc) &&
        params->pSrc[0]->rcSrc.top == 0                                        &&
        params->pSrc[0]->rcSrc.left == 0                                       &&
        SAME_SIZE_RECT(params->pSrc[0]->rcDst, params->pTarget[0]->rcDst)      &&
        params->pSrc[0]->pIEFParams == nullptr                                 &&
        params->pSrc[0]->SampleType                 == SAMPLE_PROGRESSIVE      &&
        params->pSrc[0]->Rotation                   == VPHAL_ROTATION_IDENTITY &&
        params->pSrc[0]->bQueryVariance             == false                   &&
        IsVeboxInputFormatSupport(params->pSrc[0])                             &&
        IsVeboxRTFormatSupport(params->pSrc[0], params->pTarget[0])            &&
        (params->pCompAlpha == nullptr                                         ||
         params->pCompAlpha->AlphaMode != VPHAL_ALPHA_FILL_MODE_BACKGROUND)    &&
        params->pSrc[0]->rcDst.top  == 0                                       &&
        params->pSrc[0]->rcDst.left == 0)
    {
        bRet = true;
    }

finish:
    return bRet;
}
bool VPFeatureManager::IsVeboxInputFormatSupport(PVPHAL_SURFACE pSrcSurface)
{
    VP_FUNC_CALL();

    if (nullptr == pSrcSurface)
    {
        VP_PUBLIC_ASSERTMESSAGE("nullptr == pSrcSurface");
        return false;
    }

    // Check if Sample Format is supported
    // Vebox only support P016 format, P010 format can be supported by faking it as P016
    if (pSrcSurface->Format != Format_NV12 &&
        pSrcSurface->Format != Format_AYUV &&
        pSrcSurface->Format != Format_P010 &&
        pSrcSurface->Format != Format_P016 &&
        pSrcSurface->Format != Format_P210 &&
        pSrcSurface->Format != Format_P216 &&
        pSrcSurface->Format != Format_Y8 &&
        pSrcSurface->Format != Format_Y16U &&
        pSrcSurface->Format != Format_Y16S &&
        !IS_PA_FORMAT(pSrcSurface->Format) &&
        (pSrcSurface->Format != Format_A8B8G8R8) &&
        (pSrcSurface->Format != Format_X8B8G8R8) &&
        (pSrcSurface->Format != Format_A8R8G8B8) &&
        (pSrcSurface->Format != Format_X8R8G8B8)/* &&
        !IS_RGB64_FLOAT_FORMAT(pSrcSurface->Format)*/)
    {
        VP_PUBLIC_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX.", pSrcSurface->Format);
        return false;
    }

    return true;
}
bool VPFeatureManager::IsVeboxRTFormatSupport(
    PVPHAL_SURFACE pSrcSurface,
    PVPHAL_SURFACE pRTSurface)
{
    VP_FUNC_CALL();

    bool bRet = false;

    if ((nullptr == pSrcSurface) || (nullptr == pRTSurface))
    {
        VP_RENDER_ASSERTMESSAGE(" invalid surface");
        return false;
    }

    // Check if RT Format is supported by Vebox
    if (IS_PA_FORMAT(pRTSurface->Format) ||
        pRTSurface->Format == Format_NV12 ||
        pRTSurface->Format == Format_AYUV ||
        pRTSurface->Format == Format_P010 ||
        pRTSurface->Format == Format_P016 ||
        pRTSurface->Format == Format_P210 ||
        pRTSurface->Format == Format_P216 ||
        pRTSurface->Format == Format_Y8 ||
        pRTSurface->Format == Format_Y16U ||
        pRTSurface->Format == Format_Y16S)
    {
        // Supported Vebox Render Target format. Vebox Pipe Output can be selected.
        bRet = true;
    }

    if ((pSrcSurface->ColorSpace == CSpace_BT2020) &&
        ((pSrcSurface->Format == Format_P010) ||
        (pSrcSurface->Format == Format_P016)) &&
        // YuvChannelSwap is no longer supported from GEN10+, so we only accept 32-bits no swapped format.
            (IS_RGB32_FORMAT(pRTSurface->Format) && IS_RGB_NO_SWAP(pRTSurface->Format)))
    {
        bRet = true;
    }

    return bRet;
}

bool VPFeatureManager::IsVeboxSupported(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();

    VP_RENDER_CHK_NULL_NO_STATUS(params);
    VP_RENDER_CHK_NULL_NO_STATUS(params->pSrc[0]);

    if ((nullptr != params->pSrc[0]->pDenoiseParams && true == params->pSrc[0]->pDenoiseParams->bEnableLuma) ||
        (nullptr != params->pSrc[0]->pProcampParams && true == params->pSrc[0]->pProcampParams->bEnabled) ||
        (nullptr != params->pSrc[0]->pColorPipeParams &&
        (true == params->pSrc[0]->pColorPipeParams->bEnableACE ||
         true == params->pSrc[0]->pColorPipeParams->bEnableSTE ||
         true == params->pSrc[0]->pColorPipeParams->bEnableTCC)))
    {
        return true;
    }

finish:
    return false;
}

bool VPFeatureManager::IsSfcOutputFeasible(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();

    uint32_t                    dwSfcMaxWidth = 0;
    uint32_t                    dwSfcMaxHeight = 0;
    uint32_t                    dwSfcMinWidth = 0;
    uint32_t                    dwSfcMinHeight = 0;
    uint16_t                    wWidthAlignUnit = 0;
    uint16_t                    wHeightAlignUnit = 0;
    uint32_t                    dwSourceRegionWidth = 0;
    uint32_t                    dwSourceRegionHeight = 0;
    uint32_t                    dwOutputRegionWidth = 0;
    uint32_t                    dwOutputRegionHeight = 0;
    uint32_t                    dwTargetMinWidth     = 0;
    uint32_t                    dwTargetMinHeight    = 0;
    uint32_t                    dwOutputMinWidth     = 0;
    uint32_t                    dwOutputMinHeight    = 0;
    bool                        isInterlaced         = false;
    bool                        bRet = false;
    float                       fScaleX = 0.0f, fScaleY = 0.0f;
    float                       minRatio = 0.125f, maxRatio = 8.0f;
    bool                        disableSFC = false;
    VP_POLICY_RULES             rules = {};

    VP_RENDER_CHK_NULL_NO_STATUS(params);
    VP_RENDER_CHK_NULL_NO_STATUS(params->pTarget[0]);

    if (MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrSFCPipe))
    {
        // Read user feature key to Disable SFC
        VP_PUBLIC_CHK_NULL_NO_STATUS(m_vpUserFeatureControl);
        disableSFC = m_vpUserFeatureControl->IsSfcDisabled();
        if (disableSFC)
        {
            VP_RENDER_NORMALMESSAGE("SFC is disabled.");
            bRet = false;
            return bRet;
        }
    }

    // params->pSrc[0] == nullptr is valid for color fill case on SFC.
    if (params->pSrc[0] && !IsVeboxInputFormatSupport(params->pSrc[0]))
    {
        VP_RENDER_NORMALMESSAGE("The input format %d is not supported by vebox.", params->pSrc[0]->Format);
        bRet = false;
        return bRet;
    }

    if (params->pTarget[0] && !IsOutputFormatSupported(params->pTarget[0]))
    {
        VP_RENDER_NORMALMESSAGE("The output format %d is not supported by vebox.", params->pSrc[0]->Format);
        bRet = false;
        return bRet;
    }

    dwSfcMaxWidth       = 16 * 1024;
    dwSfcMaxHeight      = 16 * 1024;
    dwSfcMinWidth       = MHW_SFC_MIN_WIDTH;
    dwSfcMinHeight      = MHW_SFC_MIN_HEIGHT;
    wWidthAlignUnit     = 1;
    wHeightAlignUnit    = 1;

    // Apply alignment restriction to the source and scaled regions.
    switch (params->pTarget[0]->Format)
    {
    case Format_NV12:
        wWidthAlignUnit = 2;
        wHeightAlignUnit = 2;
        break;
    case Format_YUY2:
    case Format_UYVY:
        wWidthAlignUnit = 2;
        break;
    default:
        break;
    }

    // Region of the input frame which needs to be processed by SFC
    dwSourceRegionHeight = MOS_ALIGN_FLOOR(
        (uint32_t)(params->pSrc[0]->rcSrc.bottom - params->pSrc[0]->rcSrc.top),
        wHeightAlignUnit);
    dwSourceRegionWidth = MOS_ALIGN_FLOOR(
        (uint32_t)(params->pSrc[0]->rcSrc.right - params->pSrc[0]->rcSrc.left),
        wWidthAlignUnit);

    // Size of the Output Region over the Render Target
    dwOutputRegionHeight = MOS_ALIGN_CEIL(
        (uint32_t)(params->pSrc[0]->rcDst.bottom - params->pSrc[0]->rcDst.top),
        wHeightAlignUnit);
    dwOutputRegionWidth = MOS_ALIGN_CEIL(
        (uint32_t)(params->pSrc[0]->rcDst.right - params->pSrc[0]->rcDst.left),
        wWidthAlignUnit);

    dwTargetMinWidth  = dwSfcMinWidth;
    dwTargetMinHeight = dwSfcMinHeight;
    dwOutputMinWidth  = dwSfcMinWidth;
    dwOutputMinHeight = dwSfcMinHeight;
    switch (params->pSrc[0]->InterlacedScalingType)
    {
    case ISCALING_INTERLEAVED_TO_INTERLEAVED:
        isInterlaced      = true;
        break;
    case ISCALING_FIELD_TO_INTERLEAVED:
        dwTargetMinWidth  = dwSfcMinWidth * 2;
        dwTargetMinHeight = dwSfcMinHeight * 2;
        isInterlaced      = true;
        break;
    case ISCALING_INTERLEAVED_TO_FIELD:
        dwOutputMinWidth  = dwSfcMinWidth / 2;
        dwOutputMinHeight = dwSfcMinHeight / 2;
        dwTargetMinWidth  = dwSfcMinWidth / 2;
        dwTargetMinHeight = dwSfcMinHeight / 2;
        isInterlaced      = true;
        break;
    default:
        isInterlaced      = false;
        break;
    }

    if (OUT_OF_BOUNDS(params->pSrc[0]->dwWidth, dwSfcMinWidth, dwSfcMaxWidth)       ||
        OUT_OF_BOUNDS(params->pSrc[0]->dwHeight, dwSfcMinHeight, dwSfcMaxHeight)    ||
        OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)            ||
        OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight)         ||
        OUT_OF_BOUNDS(dwOutputRegionWidth, dwOutputMinWidth, dwSfcMaxWidth)         ||
        OUT_OF_BOUNDS(dwOutputRegionHeight, dwOutputMinHeight, dwSfcMaxHeight)      ||
        OUT_OF_BOUNDS(params->pTarget[0]->dwWidth, dwTargetMinWidth, dwSfcMaxWidth) ||
        OUT_OF_BOUNDS(params->pTarget[0]->dwHeight, dwTargetMinHeight, dwSfcMaxHeight))
    {
        VP_RENDER_NORMALMESSAGE("Surface dimensions not supported by SFC Pipe");
        bRet = false;
        return bRet;
    }
    if (isInterlaced)
    {
        if (params->pSrc[0]->Rotation != VPHAL_ROTATION_IDENTITY)
        {
            VP_RENDER_NORMALMESSAGE("Interlaced scaling cannot support rotate or mirror by SFC pipe.");
            bRet = false;
            return bRet;
        }

        if (params->pSrc[0]->rcSrc.left != 0 ||
            params->pSrc[0]->rcSrc.top  != 0 ||
            params->pSrc[0]->rcDst.left != 0 ||
            params->pSrc[0]->rcDst.top  != 0)
        {
            VP_RENDER_NORMALMESSAGE("Interlaced scaling cannot support offset by SFC pipe.");
            bRet = false;
            return bRet;
        }
    }

    // Size of the Output Region over the Render Target
    dwOutputRegionHeight = MOS_MIN(dwOutputRegionHeight, params->pTarget[0]->dwHeight);
    dwOutputRegionWidth = MOS_MIN(dwOutputRegionWidth, params->pTarget[0]->dwWidth);

    if ((params->pSrc[0]->Rotation > VPHAL_ROTATION_IDENTITY && params->pSrc[0]->Rotation != VPHAL_MIRROR_HORIZONTAL) &&
        params->pTarget[0]->TileType != MOS_TILE_Y)
    {
        VP_RENDER_NORMALMESSAGE("non TileY output mirror and rotation not supported by SFC Pipe.");
        bRet = false;
        return bRet;
    }

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    if (params->pSrc[0]->Rotation == VPHAL_ROTATION_IDENTITY ||
        params->pSrc[0]->Rotation == VPHAL_ROTATION_180 ||
        params->pSrc[0]->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        params->pSrc[0]->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        if (params->pSrc[0]->InterlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
        {
            fScaleX = (float)dwOutputRegionWidth / (float)dwSourceRegionWidth;
            fScaleY = (float)dwOutputRegionHeight * 2.0F / (float)dwSourceRegionHeight;
        }
        else
        {
            fScaleX = (float)dwOutputRegionWidth / (float)dwSourceRegionWidth;
            fScaleY = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;
        }
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || VPHAL_ROTATE_90_MIRROR_VERTICAL || VPHAL_ROTATE_90_MIRROR_HORIZONTAL
        fScaleX = (float)dwOutputRegionHeight / (float)dwSourceRegionWidth;
        fScaleY = (float)dwOutputRegionWidth / (float)dwSourceRegionHeight;
    }

    m_hwInterface->m_vpPlatformInterface->InitPolicyRules(rules);

    if (rules.sfcMultiPassSupport.scaling.enable)
    {
        minRatio *= rules.sfcMultiPassSupport.scaling.downScaling.minRatioEnlarged;
        maxRatio *= rules.sfcMultiPassSupport.scaling.upScaling.maxRatioEnlarged;
    }

    // SFC scaling range is [0.125, 8] for both X and Y direction.
    if ((fScaleX < minRatio) || (fScaleX > maxRatio) ||
        (fScaleY < minRatio) || (fScaleY > maxRatio))
    {
        VP_RENDER_NORMALMESSAGE("Scaling factor not supported by SFC Pipe.");
        bRet = false;
        return bRet;
    }

    // Check if the input/output combination is supported, given certain alpha fill mode.
    // So far SFC only supports filling constant alpha.
    if (params->pCompAlpha                                                  &&
       (params->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_NONE         ||
        params->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM))
    {
        //No Alpha DDI for LIBVA, Always allow SFC to do detail feature on GEN12+ on linux
        //No matter what the current alpha mode is.
        if (params->pSrc[0]->bIEF == true)
        {
            params->pCompAlpha->AlphaMode = VPHAL_ALPHA_FILL_MODE_NONE;
            params->pCompAlpha->fAlpha    = 1.0;
            bRet                          = true;
            return bRet;
        }
        else if ((params->pTarget[0]->Format == Format_A8R8G8B8    ||
                 params->pTarget[0]->Format == Format_A8B8G8R8     ||
                 params->pTarget[0]->Format == Format_R10G10B10A2  ||
                 params->pTarget[0]->Format == Format_B10G10R10A2  ||
                 params->pTarget[0]->Format == Format_Y410         ||
                 params->pTarget[0]->Format == Format_Y416         ||
                 params->pTarget[0]->Format == Format_AYUV)        &&
                (params->pSrc[0]->Format == Format_A8B8G8R8        ||
                 params->pSrc[0]->Format == Format_A8R8G8B8        ||
                 params->pSrc[0]->Format == Format_Y410            ||
                 params->pSrc[0]->Format == Format_Y416            ||
                 params->pSrc[0]->Format == Format_AYUV))
        {
            bRet = false;
            return bRet;
        }
    }

    bRet = true;

finish:
    return bRet;
}

bool VPFeatureManager::IsRGBOutputFormatSupported(PVPHAL_SURFACE outSurface)
{
    if (nullptr == outSurface)
    {
        VP_RENDER_ASSERTMESSAGE(" invalid outputsurface");
        return false;
    }

    if (IS_RGB32_FORMAT(outSurface->Format)) // Remove RGB565 support due to quality issue. IS_RGB16_FORMAT(outSurface->Format)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPFeatureManager::IsNV12P010OutputFormatSupported(PVPHAL_SURFACE outSurface)
{
    if (nullptr == outSurface)
    {
        VP_RENDER_ASSERTMESSAGE(" invalid outputsurface");
        return false;
    }

    if (outSurface->TileType == MOS_TILE_Y &&
        (outSurface->Format == Format_P010 ||
         outSurface->Format == Format_P016 ||
         outSurface->Format == Format_NV12))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPFeatureManager::IsOutputFormatSupported(PVPHAL_SURFACE outSurface)
{
    VP_FUNC_CALL();
    if (nullptr == outSurface)
    {
        VP_RENDER_ASSERTMESSAGE(" invalid outputsurface");
        return false;
    }

    bool ret = true;

    if (IsRGBOutputFormatSupported(outSurface) ||
        outSurface->Format == Format_YUY2 ||
        outSurface->Format == Format_UYVY ||
        outSurface->Format == Format_AYUV ||
        outSurface->Format == Format_Y210 ||
        outSurface->Format == Format_Y410 ||
        outSurface->Format == Format_Y216 ||
        outSurface->Format == Format_Y416)
    {
        ret = true;
    }
    else if (IsNV12P010OutputFormatSupported(outSurface))
    {
        ret = true;
    }
    else
    {
        VP_RENDER_NORMALMESSAGE("Unsupported Render Target Format '0x%08x' for SFC Pipe.", outSurface->Format);
        ret = false;
    }

    return ret;
}

//!
//! \brief    Get the aligned the surface height and width unit
//! \details  According to the format of the surface, get the aligned unit for the surface
//!           width and height
//! \param    [in,out] pwWidthAlignUnit
//!           Pointer to the surface width alignment unit
//! \param    [in,out] pwHeightAlignUnit
//!           Pointer to the surface height alignment unit
//! \param    [in] format
//!           The format of the surface
//! \return   void
//!
void VPFeatureManager::GetAlignUnit(
    uint16_t        &wWidthAlignUnit,
    uint16_t        &wHeightAlignUnit,
    MOS_FORMAT      format)
{
    VP_FUNC_CALL();

    switch (format)
    {
        case Format_YV12:
        case Format_I420:
        case Format_IYUV:
        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
        case Format_NV12:
        case Format_P010:
        case Format_P016:
            wWidthAlignUnit = 2;
            wHeightAlignUnit = 2;
            break;

        case Format_YVU9:
            wWidthAlignUnit = 4;
            wHeightAlignUnit = 4;
            break;

        case Format_YUY2:
        case Format_UYVY:
        case Format_YUYV:
        case Format_YVYU:
        case Format_VYUY:
        case Format_P208:
        case Format_Y210:
        case Format_Y216:
            wWidthAlignUnit = 2;
            wHeightAlignUnit = 1;
            break;

        case Format_NV11:
            wWidthAlignUnit = 4;
            wHeightAlignUnit = 1;
            break;

        default:
            wWidthAlignUnit = 1;
            wHeightAlignUnit = 1;
            break;
    }
}

//!
//! \brief    Align the src/dst surface rectangle and surface width/height
//! \details  The surface rects and width/height need to be aligned according to the surface format
//! \param    [in,out] pSurface
//!           Pointer to the surface
//! \param    [in] formatForDstRect
//!           Format for Dst Rect
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPFeatureManager::RectSurfaceAlignment(
    PVPHAL_SURFACE       pSurface,
    MOS_FORMAT           formatForDstRect)
{
    VP_FUNC_CALL();

    uint16_t   wWidthAlignUnit            = 0;
    uint16_t   wHeightAlignUnit           = 0;
    uint16_t   wWidthAlignUnitForDstRect  = 0;
    uint16_t   wHeightAlignUnitForDstRect = 0;
    MOS_STATUS eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    GetAlignUnit(wWidthAlignUnit, wHeightAlignUnit, pSurface->Format);
    GetAlignUnit(wWidthAlignUnitForDstRect, wHeightAlignUnitForDstRect, formatForDstRect);

    // The source rectangle is floored to the aligned unit to
    // get rid of invalid data(ex: an odd numbered src rectangle with NV12 format
    // will have invalid UV data for the last line of Y data).
    pSurface->rcSrc.bottom = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcSrc.bottom, wHeightAlignUnit);
    pSurface->rcSrc.right  = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcSrc.right, wWidthAlignUnit);

    pSurface->rcSrc.top    = MOS_ALIGN_CEIL((uint32_t)pSurface->rcSrc.top, wHeightAlignUnit);
    pSurface->rcSrc.left   = MOS_ALIGN_CEIL((uint32_t)pSurface->rcSrc.left, wWidthAlignUnit);

    // The Destination rectangle is rounded to the upper alignment unit to prevent the loss of
    // data which was present in the source rectangle
    pSurface->rcDst.bottom = MOS_ALIGN_CEIL((uint32_t)pSurface->rcDst.bottom, wHeightAlignUnitForDstRect);
    pSurface->rcDst.right  = MOS_ALIGN_CEIL((uint32_t)pSurface->rcDst.right, wWidthAlignUnitForDstRect);

    pSurface->rcDst.top    = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcDst.top, wHeightAlignUnitForDstRect);
    pSurface->rcDst.left   = MOS_ALIGN_FLOOR((uint32_t)pSurface->rcDst.left, wWidthAlignUnitForDstRect);

    if (pSurface->SurfType == SURF_OUT_RENDERTARGET)
    {
        pSurface->dwHeight = MOS_ALIGN_CEIL(pSurface->dwHeight, wHeightAlignUnit);
        pSurface->dwWidth  = MOS_ALIGN_CEIL(pSurface->dwWidth, wWidthAlignUnit);
    }
    else
    {
        pSurface->dwHeight = MOS_ALIGN_FLOOR(pSurface->dwHeight, wHeightAlignUnit);
        pSurface->dwWidth  = MOS_ALIGN_FLOOR(pSurface->dwWidth, wWidthAlignUnit);
    }

    if ((pSurface->rcSrc.top  == pSurface->rcSrc.bottom) ||
        (pSurface->rcSrc.left == pSurface->rcSrc.right)  ||
        (pSurface->rcDst.top  == pSurface->rcDst.bottom) ||
        (pSurface->rcDst.left == pSurface->rcDst.right)  ||
        (pSurface->dwWidth    == 0)                      ||
        (pSurface->dwHeight   == 0))
    {
        VP_RENDER_ASSERTMESSAGE("Surface Parameter is invalid.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

bool VPFeatureManager::IsDiFormatSupported(MOS_FORMAT format)
{
    VP_FUNC_CALL();

    if (format != Format_AYUV         &&
        format != Format_Y416         &&
        format != Format_Y410         &&
        format != Format_A8B8G8R8     &&
        format != Format_A8R8G8B8     &&
        format != Format_B10G10R10A2  &&
        format != Format_R10G10B10A2  &&
        format != Format_A16B16G16R16 &&
        format != Format_A16R16G16B16)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool VPFeatureManager::IsVeboxSurfaceHeightAligned(VPHAL_SURFACE &surf)
{
    VP_FUNC_CALL();

    return MOS_IS_ALIGNED(MOS_MIN((uint32_t)surf.dwHeight, (uint32_t)surf.rcSrc.bottom), 4);
}
