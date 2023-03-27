/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     meida_sfc_render.cpp
//! \brief    Common interface for sfc
//! \details  Common interface for sfc
//!
#include "media_sfc_interface.h"
#include "media_sfc_render_legacy.h"
#include "vp_feature_manager.h"
#include "mhw_vebox.h"
#include "vp_common.h"
#include "vp_platform_interface.h"
#include "vp_pipeline.h"
#include "media_vdbox_sfc_render.h"
#include "media_interfaces_vphal.h"
#include "mos_os.h"
#include "renderhal_legacy.h"
#include "media_mem_compression.h"
#include "media_interfaces_mhw.h"
#include "media_interfaces_mhw_next.h"
#include "renderhal_platform_interface.h"

using namespace vp;

typedef MediaFactory<uint32_t, VphalDevice> VphalFactory;

MediaSfcRenderLegacy::MediaSfcRenderLegacy(PMOS_INTERFACE osInterface, MEDIA_SFC_INTERFACE_MODE mode, MediaMemComp *mmc) :
    MediaSfcRender(osInterface, mode, mmc)
{
}

MediaSfcRenderLegacy::~MediaSfcRenderLegacy()
{
    Destroy();
}

void MediaSfcRenderLegacy::Destroy()
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    MOS_Delete(m_sfcInterface);

    if (m_veboxInterface)
    {
        status = m_veboxInterface->DestroyHeap();
        if (MOS_FAILED(status))
        {
            VP_PUBLIC_ASSERTMESSAGE("Failed to destroy vebox heap, eStatus:%d.\n", status);
        }
        MOS_Delete(m_veboxInterface);
    }
}

//!
//! \brief    MediaSfcInterface initialize
//! \details  Initialize the BltState, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MediaSfcRenderLegacy::Initialize()
{
    if (m_initialized)
    {
        return MOS_STATUS_SUCCESS;
    }

    VphalDevice         *vphalDevice = nullptr;
    PLATFORM            platform = {};
    MOS_STATUS          status = MOS_STATUS_SUCCESS;
    MEDIA_FEATURE_TABLE *skuTable = nullptr;
    MEDIA_WA_TABLE      *waTable = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface->pfnGetPlatform);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface->pfnGetSkuTable);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface->pfnGetWaTable);

    skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    waTable = m_osInterface->pfnGetWaTable(m_osInterface);

    VP_PUBLIC_CHK_NULL_RETURN(skuTable);
    VP_PUBLIC_CHK_NULL_RETURN(waTable);

    // Check whether SFC supported.
    if (!MEDIA_IS_SKU(skuTable, FtrSFCPipe))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Clean the garbage data if any.
    Destroy();

    m_statusTable = MOS_New(VPHAL_STATUS_TABLE);
    VP_PUBLIC_CHK_NULL_RETURN(m_statusTable);

    // Create platform interface and vp pipeline by vphalDevice.
    m_osInterface->pfnGetPlatform(m_osInterface, &platform);
    vphalDevice = VphalFactory::Create(platform.eProductFamily);
    VP_PUBLIC_CHK_NULL_RETURN(vphalDevice);

    if (m_mode.veboxSfcEnabled)
    {
        if (vphalDevice->Initialize(m_osInterface, false, &status) != MOS_STATUS_SUCCESS)
        {
            vphalDevice->Destroy();
            MOS_Delete(vphalDevice);
            return status;
        }
        if (nullptr == vphalDevice->m_vpPipeline || nullptr == vphalDevice->m_vpPlatformInterface)
        {
            vphalDevice->Destroy();
            MOS_Delete(vphalDevice);
            return status;
        }
    }
    else
    {
        if (vphalDevice->CreateVpPlatformInterface(m_osInterface, &status) != MOS_STATUS_SUCCESS)
        {
            vphalDevice->Destroy();
            MOS_Delete(vphalDevice);
            return status;
        }
        if (nullptr == vphalDevice->m_vpPlatformInterface)
        {
            vphalDevice->Destroy();
            MOS_Delete(vphalDevice);
            return status;
        }
    }

    m_vpPipeline = vphalDevice->m_vpPipeline;
    m_vpPlatformInterface = vphalDevice->m_vpPlatformInterface;
    MOS_Delete(vphalDevice);

    // Create mhw interfaces.
    MhwInterfacesNext::CreateParams paramsNext  = {};
    paramsNext.Flags.m_sfc                      = MEDIA_IS_SKU(skuTable, FtrSFCPipe);
    paramsNext.Flags.m_vebox                    = MEDIA_IS_SKU(skuTable, FtrVERing);
    MhwInterfacesNext *mhwInterfacesNext        = MhwInterfacesNext::CreateFactory(paramsNext, m_osInterface);

    if (!mhwInterfacesNext)
    {
        MhwInterfaces::CreateParams params      = {};
        params.Flags.m_sfc                      = MEDIA_IS_SKU(skuTable, FtrSFCPipe);
        params.Flags.m_vebox                    = MEDIA_IS_SKU(skuTable, FtrVERing);
        MhwInterfaces *mhwInterfaces            = MhwInterfaces::CreateFactory(params, m_osInterface);

        VP_PUBLIC_CHK_NULL_RETURN(mhwInterfaces);

        m_sfcInterface                          = mhwInterfaces->m_sfcInterface;
        m_veboxInterface                        = mhwInterfaces->m_veboxInterface;
        m_sfcItf                                = nullptr;
        m_veboxItf                              = nullptr;

        // mi interface and cp interface will always be created during MhwInterfaces::CreateFactory.
        // Delete them here since they will also be created by RenderHal_InitInterface.
        m_osInterface->pfnDeleteMhwCpInterface(mhwInterfaces->m_cpInterface);
        MOS_Delete(mhwInterfaces);
    }
    else
    {
        VP_PUBLIC_CHK_NULL_RETURN(mhwInterfacesNext);

        m_sfcInterface                          = mhwInterfacesNext->m_sfcInterface;
        m_veboxInterface                        = mhwInterfacesNext->m_veboxInterface;
        m_sfcItf                                = mhwInterfacesNext->m_sfcItf;
        m_veboxItf                              = mhwInterfacesNext->m_veboxItf;

        // mi interface and cp interface will always be created during MhwInterfaces::CreateFactory.
        // Delete them here since they will also be created by RenderHal_InitInterface.
        m_osInterface->pfnDeleteMhwCpInterface(mhwInterfacesNext->m_cpInterface);
        MOS_Delete(mhwInterfacesNext);

        if (m_sfcInterface == nullptr)
        {
            VP_PUBLIC_NORMALMESSAGE("m_sfcInterface is null. Return to use APO path !");
            return MOS_STATUS_UNIMPLEMENTED;
        }
    }

    if (m_veboxItf)
    {
        const MHW_VEBOX_HEAP* veboxHeap = nullptr;
        m_veboxItf->GetVeboxHeapInfo(&veboxHeap);
        uint32_t uiNumInstances = m_veboxItf->GetVeboxNumInstances();

        if (uiNumInstances > 0 &&
            veboxHeap == nullptr)
        {
            // Allocate VEBOX Heap
            VP_PUBLIC_CHK_STATUS_RETURN(m_veboxItf->CreateHeap());
        }
    }
    else if (m_veboxInterface &&
        m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
        m_veboxInterface->m_veboxHeap == nullptr)
    {
        // Allocate VEBOX Heap
        VP_PUBLIC_CHK_STATUS_RETURN(m_veboxInterface->CreateHeap());
    }

    // Initialize render hal.
    m_renderHal = (PRENDERHAL_INTERFACE_LEGACY)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE_LEGACY));
    VP_PUBLIC_CHK_NULL_RETURN(m_renderHal);
    VP_PUBLIC_CHK_STATUS_RETURN(RenderHal_InitInterface_Legacy(
        (PRENDERHAL_INTERFACE_LEGACY)m_renderHal,
        &m_cpInterface,
        m_osInterface));
    RENDERHAL_SETTINGS_LEGACY  RenderHalSettings = {};
    RenderHalSettings.iMediaStates = 32; // Init MEdia state values
    VP_PUBLIC_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));
    m_miItf = m_renderHal->pRenderHalPltInterface->GetMhwMiItf();

    // Initialize vpPipeline.
    m_vpMhwinterface = MOS_New(VP_MHWINTERFACE);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpMhwinterface);
    MOS_ZeroMemory(m_vpMhwinterface, sizeof(VP_MHWINTERFACE));
    m_osInterface->pfnGetPlatform(m_osInterface, &m_vpMhwinterface->m_platform);
    m_vpMhwinterface->m_waTable                = waTable;
    m_vpMhwinterface->m_skuTable               = skuTable;
    m_vpMhwinterface->m_osInterface            = m_osInterface;
    m_vpMhwinterface->m_renderHal              = m_renderHal;
    m_vpMhwinterface->m_veboxInterface         = m_veboxInterface;
    m_vpMhwinterface->m_sfcInterface           = m_sfcInterface;
    m_vpMhwinterface->m_cpInterface            = m_cpInterface;
    m_vpMhwinterface->m_mhwMiInterface         = m_renderHal->pMhwMiInterface;
    m_vpMhwinterface->m_statusTable            = m_statusTable;
    m_vpMhwinterface->m_vpPlatformInterface    = m_vpPlatformInterface;
    m_vpMhwinterface->m_settings               = nullptr;
    m_vpMhwinterface->m_reporting              = nullptr;
    m_vpPlatformInterface->SetMhwSfcItf(m_sfcItf);
    m_vpPlatformInterface->SetMhwVeboxItf(m_veboxItf);
    m_vpPlatformInterface->SetMhwMiItf(m_miItf);
    m_vpMhwinterface->m_vpPlatformInterface = m_vpPlatformInterface;

    if (m_mode.veboxSfcEnabled)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpPipeline->Init(m_vpMhwinterface));
    }
    else
    {
        MOS_Delete(m_vpPipeline);
    }

    if (m_mode.vdboxSfcEnabled)
    {
        m_vdboxSfcRender = MOS_New(MediaVdboxSfcRender);
        VP_PUBLIC_CHK_NULL_RETURN(m_vdboxSfcRender);
        VP_PUBLIC_CHK_STATUS_RETURN(m_vdboxSfcRender->Initialize(*m_vpMhwinterface, m_mmc));
    }

    m_initialized = true;

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS MediaSfcRenderLegacy::IsParameterSupported(
    VDBOX_SFC_PARAMS                    &sfcParam)
{
    if (!m_mode.vdboxSfcEnabled)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNINITIALIZED);
    }

    VP_PUBLIC_CHK_NULL_RETURN(sfcParam.output.surface);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_vdboxSfcRender);

    VpScalingFilter scalingFilter(m_vpMhwinterface);
    FeatureParamScaling scalingParams = {};

    VP_PUBLIC_CHK_STATUS_RETURN(InitScalingParams(scalingParams, sfcParam));

    VP_EXECUTE_CAPS vpExecuteCaps   = {};
    vpExecuteCaps.bSFC              = 1;
    vpExecuteCaps.bSfcCsc           = 1;
    vpExecuteCaps.bSfcScaling       = 1;
    vpExecuteCaps.bSfcRotMir        = 1;

    VP_PUBLIC_CHK_STATUS_RETURN(scalingFilter.Init(sfcParam.videoParams.codecStandard, sfcParam.videoParams.jpeg.jpegChromaType));
    VP_PUBLIC_CHK_STATUS_RETURN(scalingFilter.SetExecuteEngineCaps(scalingParams, vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(scalingFilter.CalculateEngineParams());

    SFC_SCALING_PARAMS *params = scalingFilter.GetSfcParams();
    VP_PUBLIC_CHK_NULL_RETURN(params);

    // Check original input size (for JPEG)
    if (!MOS_WITHIN_RANGE(sfcParam.input.width, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(sfcParam.input.height, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check input size
    if (!MOS_WITHIN_RANGE(params->dwInputFrameWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(params->dwInputFrameHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check output size
    if (!MOS_WITHIN_RANGE(params->dwOutputFrameWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(params->dwOutputFrameHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check output region rectangles
    if ((scalingParams.input.rcDst.bottom - scalingParams.input.rcDst.top > (int32_t)scalingParams.output.dwHeight) ||
        (scalingParams.input.rcDst.right - scalingParams.input.rcDst.left > (int32_t)scalingParams.output.dwWidth))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check scaling ratio
    if (!MOS_WITHIN_RANGE(params->fAVSXScalingRatio, m_sfcInterface->m_minScalingRatio, m_sfcInterface->m_maxScalingRatio) ||
        !MOS_WITHIN_RANGE(params->fAVSYScalingRatio, m_sfcInterface->m_minScalingRatio, m_sfcInterface->m_maxScalingRatio))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check input and output format (limited only to current decode processing usage)
    if (!m_vdboxSfcRender->IsVdboxSfcFormatSupported(sfcParam.videoParams.codecStandard, sfcParam.input.format, sfcParam.output.surface->Format, sfcParam.output.surface->TileType))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaSfcRenderLegacy::IsParameterSupported(
    VEBOX_SFC_PARAMS                    &sfcParam)
{
    if (!m_mode.veboxSfcEnabled)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNINITIALIZED);
    }

    VP_PUBLIC_CHK_NULL_RETURN(sfcParam.input.surface);
    VP_PUBLIC_CHK_NULL_RETURN(sfcParam.output.surface);
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcInterface);

    VpScalingFilter scalingFilter(m_vpMhwinterface);
    FeatureParamScaling scalingParams = {};

    VP_PUBLIC_CHK_STATUS_RETURN(InitScalingParams(scalingParams, sfcParam));

    VP_EXECUTE_CAPS vpExecuteCaps   = {};
    vpExecuteCaps.bSFC              = 1;
    vpExecuteCaps.bSfcCsc           = 1;
    vpExecuteCaps.bSfcScaling       = 1;
    vpExecuteCaps.bSfcRotMir        = 1;

    VP_PUBLIC_CHK_STATUS_RETURN(scalingFilter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(scalingFilter.SetExecuteEngineCaps(scalingParams, vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(scalingFilter.CalculateEngineParams());

    SFC_SCALING_PARAMS *params = scalingFilter.GetSfcParams();
    VP_PUBLIC_CHK_NULL_RETURN(params);

    // Check input size
    if (!MOS_WITHIN_RANGE(params->dwInputFrameWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(params->dwInputFrameHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check output size
    if (!MOS_WITHIN_RANGE(params->dwOutputFrameWidth, m_sfcInterface->m_minWidth, m_sfcInterface->m_maxWidth) ||
        !MOS_WITHIN_RANGE(params->dwOutputFrameHeight, m_sfcInterface->m_minHeight, m_sfcInterface->m_maxHeight))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check input region rectangles
    if ((scalingParams.input.rcSrc.bottom - scalingParams.input.rcSrc.top > (int32_t)scalingParams.input.dwHeight) ||
        (scalingParams.input.rcSrc.right - scalingParams.input.rcSrc.left > (int32_t)scalingParams.input.dwWidth))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check output region rectangles
    if ((scalingParams.input.rcDst.bottom - scalingParams.input.rcDst.top > (int32_t)scalingParams.output.dwHeight) ||
        (scalingParams.input.rcDst.right - scalingParams.input.rcDst.left > (int32_t)scalingParams.output.dwWidth))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check scaling ratio
    if (!MOS_WITHIN_RANGE(params->fAVSXScalingRatio, m_sfcInterface->m_minScalingRatio, m_sfcInterface->m_maxScalingRatio) ||
        !MOS_WITHIN_RANGE(params->fAVSYScalingRatio, m_sfcInterface->m_minScalingRatio, m_sfcInterface->m_maxScalingRatio))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    // Check input and output format
    if (!m_vpPipeline->IsVeboxSfcFormatSupported(sfcParam.input.surface->Format, sfcParam.output.surface->Format))
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    return MOS_STATUS_SUCCESS;
}
