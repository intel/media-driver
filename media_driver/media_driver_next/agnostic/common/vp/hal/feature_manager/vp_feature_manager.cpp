/*
* Copyright (c) 2019-2020, Intel Corporation
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
using namespace vp;

/****************************************************************************************************/
/*                                      VpFeatureManagerNext                                        */
/****************************************************************************************************/

VpFeatureManagerNext::VpFeatureManagerNext(VpAllocator &allocator, VpResourceManager *resourceManager, PVP_MHWINTERFACE pHwInterface) :
    m_vpInterface(pHwInterface, allocator, resourceManager), m_Policy(m_vpInterface)
{
}

VpFeatureManagerNext::~VpFeatureManagerNext()
{
}

MOS_STATUS VpFeatureManagerNext::Initialize()
{
    return m_Policy.Initialize();
}

MOS_STATUS VpFeatureManagerNext::CreateHwFilterPipe(VP_PIPELINE_PARAMS &params, HwFilterPipe *&pHwFilterPipe)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    pHwFilterPipe = nullptr;

    SwFilterPipe * pSwFilterPipe = nullptr;
    status = m_vpInterface.GetSwFilterPipeFactory().Create(params, pSwFilterPipe);

    VP_PUBLIC_CHK_STATUS_RETURN(status);
    VP_PUBLIC_CHK_NULL_RETURN(pSwFilterPipe);

    status = m_vpInterface.GetHwFilterPipeFactory().Create(*pSwFilterPipe, m_Policy, pHwFilterPipe);
    m_vpInterface.GetSwFilterPipeFactory().Destory(pSwFilterPipe);

    VP_PUBLIC_CHK_STATUS_RETURN(status);
    VP_PUBLIC_CHK_NULL_RETURN(pHwFilterPipe);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpFeatureManagerNext::InitPacketPipe(VP_PIPELINE_PARAMS &params,
                PacketPipe &packetPipe)
{
    HwFilterPipe *pHwFilterPipe = nullptr;

    MOS_STATUS status = CreateHwFilterPipe(params, pHwFilterPipe);

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
    return hwFilterPipe.UpdateResources();
}

/****************************************************************************************************/
/*                                      VPFeatureManager                                            */
/****************************************************************************************************/

VPFeatureManager::VPFeatureManager(
    PVP_MHWINTERFACE  hwInterface) :
    MediaFeatureManager(),
    m_hwInterface(hwInterface)
{

}

MOS_STATUS VPFeatureManager::CheckFeatures(void * params, bool &bApgFuncSupported)
{
    VP_PUBLIC_CHK_NULL_RETURN(params);

    PVP_PIPELINE_PARAMS pvpParams = (PVP_PIPELINE_PARAMS)params;
    bApgFuncSupported = false;

    // APG only support single frame input/output
    if (pvpParams->uSrcCount != 1 ||
        pvpParams->uDstCount != 1)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pSrc[0]->SurfType != SURF_IN_PRIMARY)
    {
        return MOS_STATUS_SUCCESS;
    }

    // align rectangle of surface
    VP_PUBLIC_CHK_STATUS_RETURN(RectSurfaceAlignment(pvpParams->pSrc[0], pvpParams->pTarget[0]->Format));
    VP_PUBLIC_CHK_STATUS_RETURN(RectSurfaceAlignment(pvpParams->pTarget[0], pvpParams->pTarget[0]->Format));


    if (IsHdrNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Check whether VEBOX is available
    // VTd doesn't support VEBOX
    if (!MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrVERing))
    {
        return MOS_STATUS_SUCCESS;
    }

    // Check if the Surface size is greater than 64x16 which is the minimum Width and Height VEBOX can handle
    if (pvpParams->pSrc[0]->dwWidth < MHW_VEBOX_MIN_WIDTH || pvpParams->pSrc[0]->dwHeight < MHW_VEBOX_MIN_HEIGHT)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (pvpParams->pSrc[0]->pDenoiseParams                  ||
        pvpParams->pSrc[0]->pDeinterlaceParams              ||
        pvpParams->pSrc[0]->pBlendingParams                 ||
        pvpParams->pSrc[0]->pColorPipeParams                ||
        pvpParams->pSrc[0]->pHDRParams                      ||
        pvpParams->pSrc[0]->pLumaKeyParams                  ||
        pvpParams->pSrc[0]->pProcampParams                  ||
        pvpParams->pConstriction)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (Is2PassesCSCNeeded(pvpParams->pSrc[0], pvpParams->pTarget[0]))
    {
        return MOS_STATUS_SUCCESS;
    }

    // for now, Temp removed ARGB input for APG
    if (pvpParams->pSrc[0]->Format == Format_A8R8G8B8 ||
        pvpParams->pSrc[0]->Format == Format_X8R8G8B8)
    {
        return MOS_STATUS_SUCCESS;
    }

    bool bVeboxNeeded = false;
    bool bSFCNeeded   = IsSfcOutputFeasible(pvpParams);

    if (IsVeboxOutFeasible(pvpParams) ||
        !bSFCNeeded)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (!bVeboxNeeded &&
        pvpParams->pSrc[0]->ScalingPreference == VPHAL_SCALING_PREFER_SFC_FOR_VEBOX)
    {
        VP_PUBLIC_NORMALMESSAGE("DDI choose to use SFC only for VEBOX, and since VEBOX is not required, change to Composition.");
        return MOS_STATUS_SUCCESS;
    }

    bApgFuncSupported = true;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VPFeatureManager::CheckFeatures(void * params)
{
    bool bApgFuncSupported = false;
    return CheckFeatures(params, bApgFuncSupported);
}

bool VPFeatureManager::IsHdrNeeded(
    PVPHAL_SURFACE              pSrc,
    PVPHAL_SURFACE              pRenderTarget)
{
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
    bool    bRet = false;

    // Vebox Comp Bypass is on by default
    uint32_t dwCompBypassMode = VP_COMP_BYPASS_DISABLED;

    VP_PUBLIC_CHK_NULL_NO_STATUS(params);
    VP_PUBLIC_CHK_NULL_NO_STATUS(params->pSrc[0]);
    VP_PUBLIC_CHK_NULL_NO_STATUS(params->pTarget[0]);

    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    // Read user feature key to get the Composition Bypass mode
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

    // Vebox Comp Bypass is on by default
    UserFeatureData.u32Data = VP_COMP_BYPASS_ENABLED;

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_BYPASS_COMPOSITION_ID,
        &UserFeatureData));
    dwCompBypassMode = UserFeatureData.u32Data;

    if (dwCompBypassMode  != VP_COMP_BYPASS_DISABLED                           &&
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
    bool    bRet = false;
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pSrcSurface);

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
        !IS_PA_FORMAT(pSrcSurface->Format)/* &&
        !IS_RGB64_FLOAT_FORMAT(pSrcSurface->Format)*/)
    {
        VPHAL_RENDER_NORMALMESSAGE("Unsupported Source Format '0x%08x' for VEBOX.", pSrcSurface->Format);
        goto finish;
    }

    bRet = true;

finish:
    return bRet;
}
bool VPFeatureManager::IsVeboxRTFormatSupport(
    PVPHAL_SURFACE pSrcSurface,
    PVPHAL_SURFACE pRTSurface)
{
    bool bRet = false;

    if ((nullptr == pSrcSurface) || (nullptr == pRTSurface))
    {
        VPHAL_RENDER_ASSERTMESSAGE(" invalid surface");
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
bool VPFeatureManager::IsSfcOutputFeasible(PVP_PIPELINE_PARAMS params)
{
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
    bool                        bRet = false;
    float                       fScaleX = 0.0f, fScaleY = 0.0f;
    bool                        disableSFC = false;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(params);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(params->pTarget[0]);

    if (MEDIA_IS_SKU(m_hwInterface->m_skuTable, FtrSFCPipe))
    {
        // Read user feature key to Disable SFC
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
            nullptr,
            __VPHAL_VEBOX_DISABLE_SFC_ID,
            &UserFeatureData));

        disableSFC = UserFeatureData.bData ? true : false;

        if (disableSFC)
        {
            VPHAL_RENDER_NORMALMESSAGE("SFC is disabled.");
            bRet = false;
            return bRet;
        }
    }

    // params->pSrc[0] == nullptr is valid for color fill case on SFC.
    if (params->pSrc[0] && !IsVeboxInputFormatSupport(params->pSrc[0]))
    {
        VPHAL_RENDER_NORMALMESSAGE("The input format %d is not supported by vebox.", params->pSrc[0]->Format);
        bRet = false;
        return bRet;
    }

    if (params->pTarget[0] && !IsOutputFormatSupported(params->pTarget[0]))
    {
        VPHAL_RENDER_NORMALMESSAGE("The output format %d is not supported by vebox.", params->pSrc[0]->Format);
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

    if (OUT_OF_BOUNDS(params->pSrc[0]->dwWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
        OUT_OF_BOUNDS(params->pSrc[0]->dwHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
        OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
        OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
        OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
        OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
        OUT_OF_BOUNDS(params->pTarget[0]->dwWidth, dwSfcMinWidth, dwSfcMaxWidth) ||
        OUT_OF_BOUNDS(params->pTarget[0]->dwHeight, dwSfcMinHeight, dwSfcMaxHeight))
    {
        VPHAL_RENDER_NORMALMESSAGE("Surface dimensions not supported by SFC Pipe.");
        bRet = false;
        return bRet;
    }

    // Size of the Output Region over the Render Target
    dwOutputRegionHeight = MOS_MIN(dwOutputRegionHeight, params->pTarget[0]->dwHeight);
    dwOutputRegionWidth = MOS_MIN(dwOutputRegionWidth, params->pTarget[0]->dwWidth);

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    if (params->pSrc[0]->Rotation == VPHAL_ROTATION_IDENTITY ||
        params->pSrc[0]->Rotation == VPHAL_ROTATION_180 ||
        params->pSrc[0]->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        params->pSrc[0]->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        fScaleX = (float)dwOutputRegionWidth / (float)dwSourceRegionWidth;
        fScaleY = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;
    }
    else
    {
        // VPHAL_ROTATION_90 || VPHAL_ROTATION_270 || VPHAL_ROTATE_90_MIRROR_VERTICAL || VPHAL_ROTATE_90_MIRROR_HORIZONTAL
        fScaleX = (float)dwOutputRegionHeight / (float)dwSourceRegionWidth;
        fScaleY = (float)dwOutputRegionWidth / (float)dwSourceRegionHeight;
    }

    // SFC scaling range is [0.125, 8] for both X and Y direction.
    if ((fScaleX < 0.125F) || (fScaleX > 8.0F) ||
        (fScaleY < 0.125F) || (fScaleY > 8.0F))
    {
        VPHAL_RENDER_NORMALMESSAGE("Scaling factor not supported by SFC Pipe.");
            bRet = false;
            return bRet;
    }

    // Check if the input/output combination is supported, given certain alpha fill mode.
    // So far SFC only supports filling constant alpha.
    if (params->pCompAlpha                                                  &&
       (params->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_NONE         ||
        params->pCompAlpha->AlphaMode == VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM))
    {
        if ((params->pTarget[0]->Format == Format_A8R8G8B8    ||
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

bool VPFeatureManager::IsOutputFormatSupported(PVPHAL_SURFACE outSurface)
{
    bool ret = true;

    if (!IS_RGB32_FORMAT(outSurface->Format) &&
        // Remove RGB565 support due to quality issue, may reopen this after root cause in the future.
        //!IS_RGB16_FORMAT(outSurface->Format)   &&
        outSurface->Format != Format_YUY2 &&
        outSurface->Format != Format_UYVY &&
        outSurface->Format != Format_AYUV &&
        outSurface->Format != Format_Y210 &&
        outSurface->Format != Format_Y410 &&
        outSurface->Format != Format_Y216 &&
        outSurface->Format != Format_Y416)
    {
        if (outSurface->TileType == MOS_TILE_Y    &&
            (outSurface->Format == Format_P010    ||
             outSurface->Format == Format_P016    ||
             outSurface->Format == Format_NV12))
        {
            ret = true;
        }
        else
        {
            VPHAL_RENDER_NORMALMESSAGE("Unsupported Render Target Format '0x%08x' for SFC Pipe.", outSurface->Format);
            ret = false;
        }
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
    uint16_t   wWidthAlignUnit;
    uint16_t   wHeightAlignUnit;
    uint16_t   wWidthAlignUnitForDstRect;
    uint16_t   wHeightAlignUnitForDstRect;
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
        VPHAL_RENDER_ASSERTMESSAGE("Surface Parameter is invalid.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}
