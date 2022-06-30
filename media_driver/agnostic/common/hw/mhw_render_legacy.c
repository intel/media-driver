/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_render_legacy.c
//! \brief    MHW interface for constructing commands for the render engine
//! \details  Impelements the functionalities common across all platforms for MHW_RENDER
//!

#include "mhw_render_legacy.h"

//!
//! \brief    Allocates the MHW render interface internal parameters
//! \details  Internal MHW function to allocate all parameters needed for the
//!           render interface including the state heap interface
//! \param    MHW_STATE_HEAP_SETTINGS stateHeapSettings
//!           [in] Setting used to initialize the state heap interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS MhwRenderInterface::AllocateHeaps(
    MHW_STATE_HEAP_SETTINGS         stateHeapSettings)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_stateHeapInterface);

    if ((stateHeapSettings.dwIshSize > 0 ||
        stateHeapSettings.dwDshSize > 0 ) &&
        stateHeapSettings.dwNumSyncTags > 0)
    {
        MHW_MI_CHK_STATUS(m_stateHeapInterface->pfnCreate(
            &m_stateHeapInterface,
            stateHeapSettings));
    }

    return eStatus;
}

void MhwRenderInterface::InitPlatformCaps(
    MEDIA_SYSTEM_INFO         *gtSystemInfo)
{
    if (gtSystemInfo == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid input pointer provided");
        return;
    }

    MOS_ZeroMemory(&m_hwCaps, sizeof(MHW_RENDER_ENGINE_CAPS));

    m_hwCaps.dwMaxUnormSamplers       = MHW_RENDER_ENGINE_SAMPLERS_MAX;
    m_hwCaps.dwMaxAVSSamplers         = MHW_RENDER_ENGINE_SAMPLERS_AVS_MAX;
    m_hwCaps.dwMaxBTIndex             = MHW_RENDER_ENGINE_SSH_SURFACES_PER_BT_MAX - 1;
    m_hwCaps.dwMaxThreads             = gtSystemInfo->ThreadCount;
    m_hwCaps.dwMaxMediaPayloadSize    = MHW_RENDER_ENGINE_MEDIA_PALOAD_SIZE_MAX;
    m_hwCaps.dwMaxURBSize             = MHW_RENDER_ENGINE_URB_SIZE_MAX;
    m_hwCaps.dwMaxURBEntries          = MHW_RENDER_ENGINE_URB_ENTRIES_MAX;
    m_hwCaps.dwMaxSubslice            = gtSystemInfo->MaxSubSlicesSupported;
    m_hwCaps.dwMaxEUIndex             = MHW_RENDER_ENGINE_EU_INDEX_MAX;
    m_hwCaps.dwNumThreadsPerEU        = (gtSystemInfo->EUCount > 0) ?
        gtSystemInfo->ThreadCount / gtSystemInfo->EUCount : 0;
    m_hwCaps.dwSizeRegistersPerThread = MHW_RENDER_ENGINE_SIZE_REGISTERS_PER_THREAD;

    m_hwCaps.dwMaxInterfaceDescriptorEntries  = MHW_RENDER_ENGINE_INTERFACE_DESCRIPTOR_ENTRIES_MAX;
    m_hwCaps.dwMaxURBEntryAllocationSize      =
        m_hwCaps.dwMaxCURBEAllocationSize     =
        m_hwCaps.dwMaxURBSize - m_hwCaps.dwMaxInterfaceDescriptorEntries;
}

void MhwRenderInterface::InitPreemption()
{
    auto m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);

    if (m_skuTable == nullptr || waTable == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid SKU or WA table acquired");
        return;
    }

    if (MEDIA_IS_SKU(m_skuTable, FtrMediaThreadGroupLevelPreempt) ||
        MEDIA_IS_SKU(m_skuTable, FtrMediaMidBatchPreempt))
    {
        m_preemptionEnabled = true;

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MEDIA_PREEMPTION_ENABLE_ID,
            &UserFeatureData,
            m_osInterface->pOsContext);
        m_preemptionEnabled = (UserFeatureData.i32Data) ? true : false;
#endif
    }

    if (MEDIA_IS_SKU(m_skuTable, FtrPerCtxtPreemptionGranularityControl))
    {
        m_preemptionCntlRegisterOffset = MHW_RENDER_ENGINE_PREEMPTION_CONTROL_OFFSET;

        if (MEDIA_IS_SKU(m_skuTable, FtrMediaMidThreadLevelPreempt))
        {
            m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE;
        }
        else if (MEDIA_IS_SKU(m_skuTable, FtrMediaThreadGroupLevelPreempt))
        {
            m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
        }
        else if (MEDIA_IS_SKU(m_skuTable, FtrMediaMidBatchPreempt))
        {
            m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
        }

        // Set it to Mid Batch Pre-emption level (command level) to avoid render engine hang after preemption is turned on in ring buffer
        if (MEDIA_IS_WA(waTable, WaMidBatchPreemption))
        {
            m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
        }
    }
}

MOS_STATUS MhwRenderInterface::EnablePreemption(
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(m_miInterface);

    MEDIA_FEATURE_TABLE *m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    MHW_MI_CHK_NULL(m_skuTable);

    if (MEDIA_IS_SKU(m_skuTable, FtrPerCtxtPreemptionGranularityControl))
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterParams;
        MOS_ZeroMemory(&loadRegisterParams, sizeof(loadRegisterParams));
        loadRegisterParams.dwRegister = m_preemptionCntlRegisterOffset;
        loadRegisterParams.dwData     = m_preemptionCntlRegisterValue;
        MHW_MI_CHK_STATUS(m_miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegisterParams));
    }

    return eStatus;
}
