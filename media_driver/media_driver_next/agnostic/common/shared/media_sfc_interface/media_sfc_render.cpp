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
//! \file     meida_sfc_render.cpp
//! \brief    Common interface for sfc
//! \details  Common interface for sfc
//!
#include "media_sfc_interface.h"
#include "media_sfc_render.h"
#include "vp_feature_manager.h"
#include "mhw_vebox.h"
#include "vphal_common.h"
#include "vp_platform_interface.h"
#include "vp_pipeline.h"
#include "media_vdbox_sfc_render.h"
#include "media_interfaces_vphal.h"
#include "mos_os.h"
#include "renderhal.h"

using namespace vp;

typedef MediaInterfacesFactory<VphalDevice> VphalFactory;

MediaSfcRender::MediaSfcRender(PMOS_INTERFACE osInterface) : m_osInterface(osInterface)
{
}

MediaSfcRender::~MediaSfcRender()
{
    Destroy();
}

void MediaSfcRender::Destroy()
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MOS_Delete(m_vdboxSfcRender);
    MOS_Delete(m_vpPipeline);
    MOS_Delete(m_vpPlatformInterface);

    if (m_renderHal)
    {
        if (m_renderHal->pfnDestroy)
        {
            status = m_renderHal->pfnDestroy(m_renderHal);
            if (MOS_STATUS_SUCCESS != status)
            {
                VP_PUBLIC_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", status);
            }
        }
        MOS_FreeMemory(m_renderHal);
    }

    Delete_MhwCpInterface(m_cpInterface);
    m_cpInterface = nullptr;
    MOS_Delete(m_sfcInterface);

    if (m_veboxInterface)
    {
        status = m_veboxInterface->DestroyHeap();
        if (MOS_STATUS_SUCCESS != status)
        {
            VP_PUBLIC_ASSERTMESSAGE("Failed to destroy vebox heap, eStatus:%d.\n", status);
        }
        MOS_Delete(m_veboxInterface);
    }

    MOS_Delete(m_statusTable);
}

MOS_STATUS MediaSfcRender::Render(VEBOX_SFC_PARAMS &sfcParam)
{
    if (!m_initialized)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNINITIALIZED);
    }
    VP_PARAMS params = {};
    params.type = PIPELINE_PARAM_TYPE_MEDIA_SFC_INTERFACE;
    params.sfcParams = &sfcParam;
    VP_PUBLIC_CHK_STATUS_RETURN(m_vpPipeline->Prepare(&params));
    VP_PUBLIC_CHK_STATUS_RETURN(m_vpPipeline->Execute());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaSfcRender::Render(MOS_COMMAND_BUFFER *cmdBuffer, VDBOX_SFC_PARAMS &param)
{
    if (!m_initialized)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNINITIALIZED);
    }
    VP_PUBLIC_CHK_NULL_RETURN(m_vdboxSfcRender);
    VP_PUBLIC_CHK_NULL_RETURN(cmdBuffer);
    VP_PUBLIC_CHK_STATUS_RETURN(m_vdboxSfcRender->AddSfcStates(cmdBuffer, param));
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    MediaSfcInterface initialize
//! \details  Initialize the BltState, create BLT context.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MediaSfcRender::Initialize()
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
    vphalDevice = VphalFactory::CreateHal(platform.eProductFamily);
    VP_PUBLIC_CHK_NULL_RETURN(vphalDevice);

    if (vphalDevice->Initialize(m_osInterface, m_osInterface->pOsContext, false, &status) != MOS_STATUS_SUCCESS)
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

    m_vpPipeline = vphalDevice->m_vpPipeline;
    m_vpPlatformInterface = vphalDevice->m_vpPlatformInterface;
    MOS_Delete(vphalDevice);

    // Create mhw interfaces.
    MhwInterfaces::CreateParams params      = {};
    params.Flags.m_sfc                      = MEDIA_IS_SKU(skuTable, FtrSFCPipe);
    params.Flags.m_vebox                    = MEDIA_IS_SKU(skuTable, FtrVERing);
    MhwInterfaces *mhwInterfaces            = MhwInterfaces::CreateFactory(params, m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(mhwInterfaces);

    m_sfcInterface                          = mhwInterfaces->m_sfcInterface;
    m_veboxInterface                        = mhwInterfaces->m_veboxInterface;

    // mi interface and cp interface will always be created during MhwInterfaces::CreateFactory.
    // Delete them here since they will also be created by RenderHal_InitInterface.
    MOS_Delete(mhwInterfaces->m_miInterface);
    Delete_MhwCpInterface(mhwInterfaces->m_cpInterface);
    MOS_Delete(mhwInterfaces);

    if (m_veboxInterface &&
        m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
        m_veboxInterface->m_veboxHeap == nullptr)
    {
        // Allocate VEBOX Heap
        VP_PUBLIC_CHK_STATUS_RETURN(m_veboxInterface->CreateHeap());
    }

    // Initialize render hal.
    m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE));
    VP_PUBLIC_CHK_NULL_RETURN(m_renderHal);
    VP_PUBLIC_CHK_STATUS_RETURN(RenderHal_InitInterface(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));
    RENDERHAL_SETTINGS  RenderHalSettings = {};
    RenderHalSettings.iMediaStates = 32; // Init MEdia state values
    VP_PUBLIC_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    // Initialize vpPipeline.
    VP_MHWINTERFACE vpMhwinterface          = {};
    m_osInterface->pfnGetPlatform(m_osInterface, &vpMhwinterface.m_platform);
    vpMhwinterface.m_waTable                = waTable;
    vpMhwinterface.m_skuTable               = skuTable;
    vpMhwinterface.m_osInterface            = m_osInterface;
    vpMhwinterface.m_renderHal              = m_renderHal;
    vpMhwinterface.m_veboxInterface         = m_veboxInterface;
    vpMhwinterface.m_sfcInterface           = m_sfcInterface;
    vpMhwinterface.m_renderer               = nullptr;
    vpMhwinterface.m_cpInterface            = m_cpInterface;
    vpMhwinterface.m_mhwMiInterface         = m_renderHal->pMhwMiInterface;
    vpMhwinterface.m_statusTable            = m_statusTable;
    vpMhwinterface.m_vpPlatformInterface    = m_vpPlatformInterface;

    VP_PUBLIC_CHK_STATUS_RETURN(m_vpPipeline->Init(&vpMhwinterface));
    m_vdboxSfcRender = MOS_New(MediaVdboxSfcRender);
    VP_PUBLIC_CHK_NULL_RETURN(m_vdboxSfcRender);
    VP_PUBLIC_CHK_STATUS_RETURN(m_vdboxSfcRender->Initialize(vpMhwinterface));

    m_initialized = true;

    return MOS_STATUS_SUCCESS;
}