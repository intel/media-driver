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
//! \file     vp_pipeline_adapter_base.cpp
//! \brief    vp pipeline adapter base clarification
//! \details  vp pipeline adapter base clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#include "vp_pipeline_adapter_base.h"
#include "media_interfaces_vphal.h"
#include "vp_platform_interface.h"
#include "vp_debug.h"
#include "vp_user_setting.h"
#include "renderhal_platform_interface.h"
#include "vp_user_feature_control.h"

VpPipelineAdapterBase::VpPipelineAdapterBase(
    vp::VpPlatformInterface &vpPlatformInterface,
    MOS_STATUS              &eStatus,
    bool                    clearViewMode) :
    m_vpPlatformInterface(vpPlatformInterface)
{
    m_osInterface = m_vpPlatformInterface.GetOsInterface();
    if (m_osInterface)
    {
        m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);
    }
    VpUserSetting::InitVpUserSetting(m_userSettingPtr, clearViewMode);

    eStatus = MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipelineAdapterBase::GetVpMhwInterface(
    VP_MHWINTERFACE &vpMhwinterface)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    bool       sfcNeeded                          = false;
    bool       veboxNeeded                        = false;
    std::shared_ptr<mhw::vebox::Itf> veboxItf     = nullptr;
    std::shared_ptr<mhw::sfc::Itf>   sfcItf       = nullptr;
    std::shared_ptr<mhw::mi::Itf>    miItf        = nullptr;
    m_osInterface = m_vpPlatformInterface.GetOsInterface();
    if (m_osInterface == nullptr)
    {
        eStatus = MOS_STATUS_NULL_POINTER;
        return eStatus;
    }

    // Initialize platform, sku, wa tables
    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);
    m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable  = m_osInterface->pfnGetWaTable(m_osInterface);

    m_vprenderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(*m_vprenderHal));
    if (m_vprenderHal == nullptr)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
    }

    eStatus = RenderHal_InitInterface(
        m_vprenderHal,
        &m_cpInterface,
        m_osInterface);

    VPHAL_DBG_OCA_DUMPER_CREATE(m_vprenderHal);

    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("VpPipelineAdapterBase construct failed due to base class returned failure: eStatus = %d.", eStatus);
        return eStatus;
    }

    veboxNeeded = MEDIA_IS_SKU(m_skuTable, FtrVERing);
    sfcNeeded   = MEDIA_IS_SKU(m_skuTable, FtrSFCPipe);
    SetMhwMiItf(m_vprenderHal->pRenderHalPltInterface->GetMhwMiItf());
    if ((veboxNeeded || sfcNeeded) && !m_clearVideoViewMode)
    {
        eStatus = VphalDevice::CreateVPMhwInterfaces(sfcNeeded, veboxNeeded, veboxItf, sfcItf, miItf, m_osInterface);
        if (eStatus == MOS_STATUS_SUCCESS)
        {
            SetMhwVeboxItf(veboxItf);
            SetMhwSfcItf(sfcItf);
        }
        else
        {
            VP_PUBLIC_ASSERTMESSAGE("Allocate MhwInterfaces failed");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NO_SPACE);
        }
    }

    vpMhwinterface.m_platform       = m_platform;
    vpMhwinterface.m_waTable        = m_waTable;
    vpMhwinterface.m_skuTable       = m_skuTable;
    vpMhwinterface.m_osInterface    = m_osInterface;
    vpMhwinterface.m_renderHal      = m_vprenderHal;
    vpMhwinterface.m_cpInterface    = m_cpInterface;
    vpMhwinterface.m_statusTable    = &m_statusTable;
    m_vpPlatformInterface.SetMhwSfcItf(m_sfcItf);
    m_vpPlatformInterface.SetMhwVeboxItf(m_veboxItf);
    m_vpPlatformInterface.SetMhwMiItf(m_miItf);
    vpMhwinterface.m_vpPlatformInterface = &m_vpPlatformInterface;

    return eStatus;
}

VpPipelineAdapterBase::~VpPipelineAdapterBase()
{
    MOS_STATUS eStatus;

    // Wait for all cmd before destroy gpu resource
    if (m_osInterface && m_osInterface->pfnWaitAllCmdCompletion && m_osInterface->bDeallocateOnExit)
    {
        VP_PUBLIC_NORMALMESSAGE("WaitAllCmdCompletion in VpPipelineAdapterBase::~VpPipelineAdapterBase");
        m_osInterface->pfnWaitAllCmdCompletion(m_osInterface);
    }

    if (m_vprenderHal)
    {
        VPHAL_DBG_OCA_DUMPER_DESTORY(m_vprenderHal);
        if (m_vprenderHal->pfnDestroy)
        {
            eStatus = m_vprenderHal->pfnDestroy(m_vprenderHal);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VP_PUBLIC_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
            }
        }
        MOS_FreeMemory(m_vprenderHal);
    }

    if (m_cpInterface)
    {
        if (m_osInterface)
        {
            m_osInterface->pfnDeleteMhwCpInterface(m_cpInterface);
            m_cpInterface = nullptr;
        }
        else
        {
            VP_PUBLIC_ASSERTMESSAGE("Failed to destroy cpInterface.");
        }
    }

    if (m_sfcItf)
    {
        m_sfcItf = nullptr;
    }

    if (m_veboxItf)
    {
        eStatus    = m_veboxItf->DestroyHeap();
        m_veboxItf = nullptr;
    }

    // Destroy OS interface objects (CBs, etc)
    if (m_osInterface)
    {
        if (m_osInterface->bDeallocateOnExit)
        {
            m_osInterface->pfnDestroy(m_osInterface, true);

            // Deallocate OS interface structure (except if externally provided)
            MOS_FreeMemory(m_osInterface);
        }
    }

    vp::VpPlatformInterface *pIntf = &m_vpPlatformInterface;
    MOS_Delete(pIntf);
}

//!
//! \brief    Get Status Report
//! \details  Get Status Report, will return back to app indicating if related frame id is done by gpu
//! \param    [out] pQueryReport
//!           Pointer to pQueryReport, the status query report array.
//! \param    [in] wStatusNum
//!           The size of array pQueryReport.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
MOS_STATUS VpPipelineAdapterBase::GetStatusReport(
    PQUERY_STATUS_REPORT_APP pQueryReport,
    uint16_t                 wStatusNum)
{
    VP_FUNC_CALL();
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if (!EMUL)  // this function is dummy for emul
    uint32_t            i;
    uint32_t            uiTableLen;
    PVPHAL_STATUS_TABLE pStatusTable;
    PMOS_CONTEXT        pOsContext;
    uint32_t            uiIndex;
    uint32_t            uiNewHead;
    PVPHAL_STATUS_ENTRY pStatusEntry;
    bool                bMarkNotReadyForRemains = false;

    VP_PUBLIC_CHK_NULL(pQueryReport);
    VP_PUBLIC_CHK_NULL(m_osInterface);
    VP_PUBLIC_CHK_NULL(m_osInterface->pOsContext);

    // it should be ok if we don't consider the null render
    // eNullRender = m_pOsInterface->pfnGetNullHWRenderFlags(m_pOsInterface);

    pOsContext   = m_osInterface->pOsContext;
    pStatusTable = &m_statusTable;
    uiNewHead    = pStatusTable->uiHead;  // uiNewHead start from previous head value
    // entry length from head to tail
    if (pStatusTable->uiCurrent < pStatusTable->uiHead)
    {
        uiTableLen = pStatusTable->uiCurrent + VPHAL_STATUS_TABLE_MAX_SIZE - pStatusTable->uiHead;
    }
    else
    {
        uiTableLen = pStatusTable->uiCurrent - pStatusTable->uiHead;
    }

    // step 1 - update pStatusEntry from driver if command associated with the dwTag is done by gpu
    for (i = 0; i < wStatusNum && i < uiTableLen; i++)
    {
        uint32_t dwGpuTag;  // hardware tag updated by gpu command pipectl
        bool     bDoneByGpu;
        bool     bFailedOnSubmitCmd;

        uiIndex      = (pStatusTable->uiHead + i) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        pStatusEntry = &pStatusTable->aTableEntries[uiIndex];

        // for tasks using CM, different streamIndexes may be used
        uint32_t oldStreamIndex = m_osInterface->streamIndex;
        if (pStatusEntry->isStreamIndexSet)
        {
            m_osInterface->streamIndex = pStatusEntry->streamIndex;
        }

        if (bMarkNotReadyForRemains)
        {
            // the status is set as VPREP_NOTREADY while submitting commands
            pQueryReport[i].dwStatus         = pStatusEntry->dwStatus;
            pQueryReport[i].StatusFeedBackID = pStatusEntry->StatusFeedBackID;
            continue;
        }

        dwGpuTag            = m_osInterface->pfnGetGpuStatusSyncTag(m_osInterface, pStatusEntry->GpuContextOrdinal);
        bDoneByGpu          = (dwGpuTag >= pStatusEntry->dwTag);
        bFailedOnSubmitCmd  = (pStatusEntry->dwStatus == VPREP_ERROR);

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_NULL_RENDERING_FLAGS NullRender = m_osInterface->pfnGetNullHWRenderFlags(m_osInterface);
        if (NullRender.Value != 0)
        {
            bDoneByGpu = true;
        }
#endif

        if (bFailedOnSubmitCmd)
        {
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        }
        else if (bDoneByGpu)
        {
            pStatusEntry->dwStatus = VPREP_OK;
            uiNewHead              = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        }
        else
        {  // here we have the first not ready entry.
#if (LINUX || ANDROID)
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
#else
            uiNewHead = uiIndex;
#endif

            bMarkNotReadyForRemains = true;
        }

        if (m_osInterface->pfnIsGPUHung(m_osInterface))
        {
            pStatusEntry->dwStatus = VPREP_NOTREADY;
        }

        pQueryReport[i].dwStatus         = pStatusEntry->dwStatus;
        pQueryReport[i].StatusFeedBackID = pStatusEntry->StatusFeedBackID;

        if (pStatusEntry->isStreamIndexSet)
        {
            m_osInterface->streamIndex = oldStreamIndex;
        }
    }
    pStatusTable->uiHead = uiNewHead;

    // step 2 - mark VPREP_NOTAVAILABLE for unused entry
    for (/* continue from previous i */; i < wStatusNum; i++)
    {
        pQueryReport[i].dwStatus         = VPREP_NOTAVAILABLE;
        pQueryReport[i].StatusFeedBackID = 0;
    }

finish:
#else
    MOS_UNUSED(pQueryReport);
    MOS_UNUSED(wStatusNum);
#endif  // end (!EMUL && !ANDROID)
    return eStatus;
}

MOS_STATUS VpPipelineAdapterBase::GetStatusReportEntryLength(
    uint32_t*                      puiLength)
{
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;
#if(!EMUL)        // this function is dummy for emul
    PVPHAL_STATUS_TABLE            pStatusTable;

    VP_PUBLIC_CHK_NULL(puiLength);

    pStatusTable = &m_statusTable;

    // entry length from head to tail
    if (pStatusTable->uiCurrent < pStatusTable->uiHead)
    {
        *puiLength = pStatusTable->uiCurrent + VPHAL_STATUS_TABLE_MAX_SIZE - pStatusTable->uiHead;
    }
    else
    {
        *puiLength = pStatusTable->uiCurrent - pStatusTable->uiHead;
    }
    
finish:
#else
    MOS_UNUSED(puiLength);
#endif
    return eStatus;
}
