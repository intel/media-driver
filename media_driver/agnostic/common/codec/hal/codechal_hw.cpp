/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     codechal_hw.cpp
//! \brief    This modules implements HW interface layer to be used on all platforms on     all operating systems/DDIs, across CODECHAL components.
//!
#include "codechal_hw.h"
#include "codechal_setting.h"
#include "mhw_cmd_reader.h"

#define VDBOX_HUC_VDENC_BRC_INIT_KERNEL_DESCRIPTOR 4

//| HW parameter initializers
const MOS_SYNC_PARAMS     g_cInitSyncParams =
{
    MOS_GPU_CONTEXT_RENDER,         // GpuContext
    nullptr,                        // presSyncResource
    1,                              // uiSemaphoreCount
    0,                              // uiSemaphoreValue
    0,                              // uiSemaphoreOffset
    false,                          // bReadOnly
    true,                           // bDisableDecodeSyncLock
    false,                          // bDisableLockForTranscode
};

CodechalHwInterface::CodechalHwInterface(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfaces     *mhwInterfaces)
{
    CODECHAL_HW_FUNCTION_ENTER;

    // Basic intialization
    m_osInterface = osInterface;

    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);

    m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable = m_osInterface->pfnGetWaTable(m_osInterface);

    CODECHAL_HW_ASSERT(m_skuTable);
    CODECHAL_HW_ASSERT(m_waTable);

    // Init sub-interfaces
    m_cpInterface = mhwInterfaces->m_cpInterface;
    m_mfxInterface = mhwInterfaces->m_mfxInterface;
    m_hcpInterface = mhwInterfaces->m_hcpInterface;
    m_hucInterface = mhwInterfaces->m_hucInterface;
    m_vdencInterface = mhwInterfaces->m_vdencInterface;
    m_veboxInterface = mhwInterfaces->m_veboxInterface;
    m_sfcInterface = mhwInterfaces->m_sfcInterface;
    m_miInterface = mhwInterfaces->m_miInterface;
    m_renderInterface = mhwInterfaces->m_renderInterface;

    m_stateHeapSettings = MHW_STATE_HEAP_SETTINGS();

    MOS_ZeroMemory(&m_hucDmemDummy, sizeof(m_hucDmemDummy));
    MOS_ZeroMemory(&m_dummyStreamIn, sizeof(m_dummyStreamIn));
    MOS_ZeroMemory(&m_dummyStreamOut, sizeof(m_dummyStreamOut));
    MOS_ZeroMemory(&m_conditionalBbEndDummy, sizeof(m_conditionalBbEndDummy));
}

MOS_STATUS CodechalHwInterface::SetCacheabilitySettings(
    MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mfxInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_hcpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_vdencInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_vdencInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_hucInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    return eStatus;
}

MOS_STATUS CodechalHwInterface::SetRowstoreCachingOffsets(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_vdencInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_vdencInterface->GetRowstoreCachingAddrs(rowstoreParams));
    }
    if (m_mfxInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->GetRowstoreCachingAddrs(rowstoreParams));
    }
    if (m_hcpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->GetRowstoreCachingAddrs(rowstoreParams));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterface::InitCacheabilityControlSettings(
    CODECHAL_FUNCTION codecFunction)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(m_osInterface);

    for (uint32_t i = MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC + 1; i < MOS_CODEC_RESOURCE_USAGE_END_CODEC; i++)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(CachePolicyGetMemoryObject(
            (MOS_HW_RESOURCE_DEF)i));
    }

    SetCacheabilitySettings(m_cacheabilitySettings);

    // This code needs to be removed
    PMHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings = &m_cacheabilitySettings[0];
    bool l3CachingEnabled = !m_osInterface->bSimIsActive;

    if (m_checkTargetCache)
    {
        l3CachingEnabled =
            ((cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Gen8.TargetCache == 0x3) ||
                (cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Gen8.TargetCache == 0x3) ||
                (cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Gen8.TargetCache == 0x3) ||
                (cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Gen8.TargetCache == 0x3));
    }

    if (m_checkBankCount)
    {
        CODECHAL_HW_CHK_NULL_RETURN(m_osInterface);
        auto gtSysInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
        CODECHAL_HW_CHK_NULL_RETURN(gtSysInfo);

        l3CachingEnabled = (gtSysInfo->L3BankCount != 0);
    }

    if (l3CachingEnabled)
    {
        InitL3CacheSettings();
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterface::InitL3CacheSettings()
{

        // Get default L3 cache settings
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->EnableL3Caching(nullptr));

#if (_DEBUG || _RELEASE_INTERNAL)
        // Override default L3 cache settings
        auto l3CacheConfig =
            m_renderInterface->GetL3CacheConfig();
        MHW_RENDER_ENGINE_L3_CACHE_SETTINGS l3Overrides;
        CODECHAL_HW_CHK_STATUS_RETURN(InitL3ControlUserFeatureSettings(
            l3CacheConfig,
            &l3Overrides));
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->EnableL3Caching(
            &l3Overrides));
#endif // (_DEBUG || _RELEASE_INTERNAL)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterface::CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF mosUsage)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_cacheabilitySettings[mosUsage].Value =
        (m_osInterface->pfnCachePolicyGetMemoryObject(
            mosUsage, 
            m_osInterface->pfnGetGmmClientContext(m_osInterface))).DwordValue;

    m_cacheabilitySettings[mosUsage].Value = ComposeSurfaceCacheabilityControl(
        mosUsage,
        codechalUncacheableWa);

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetMfxStateCommandsDataSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    bool                            shortFormat)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;

    if (m_mfxInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->GetMfxStateCommandsDataSize(
            mode, (uint32_t*)commandsSize, (uint32_t*)patchListSize, shortFormat ? true : false));

        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }
    *commandsSize += (uint32_t)cpCmdsize;
    *patchListSize += (uint32_t)cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetMfxPrimitiveCommandsDataSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;

    if (m_mfxInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->GetMfxPrimitiveCommandsDataSize(
            mode, (uint32_t*)commandsSize, (uint32_t*)patchListSize, modeSpecific ? true : false));

        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize += (uint32_t)cpCmdsize;
    *patchListSize += (uint32_t)cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetHxxStateCommandSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t   standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;

    if (m_hcpInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->GetHcpStateCommandSize(
            mode, (uint32_t*)&hcpCommandsSize, (uint32_t*)&hcpPatchListSize, params));

        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    uint32_t hucCommandsSize = 0;
    uint32_t hucPatchListSize = 0;

    if (m_hucInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_CENC || standard == CODECHAL_VP9 || standard == CODECHAL_AVC))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->GetHucStateCommandSize(
            mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize, params));
    }

    *commandsSize = hcpCommandsSize + hucCommandsSize + (uint32_t)cpCmdsize;
    *patchListSize = hcpPatchListSize + hucPatchListSize + (uint32_t)cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetHxxPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize = 0;
    uint32_t cpPatchListSize = 0;

    if (m_hcpInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->GetHcpPrimitiveCommandSize(
            mode, (uint32_t*)&hcpCommandsSize, (uint32_t*)&hcpPatchListSize, modeSpecific ? true : false));

        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    uint32_t hucCommandsSize = 0;
    uint32_t hucPatchListSize = 0;

    if (m_hucInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_CENC || standard == CODECHAL_VP9))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->GetHucPrimitiveCommandSize(
            mode, (uint32_t*)&hucCommandsSize, (uint32_t*)&hucPatchListSize));
    }

    *commandsSize = hcpCommandsSize + hucCommandsSize + (uint32_t)cpCmdsize;
    *patchListSize = hcpPatchListSize + hucPatchListSize + (uint32_t)cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetHcpStateCommandSize(
    uint32_t                        mode,
    uint32_t *                      commandsSize,
    uint32_t *                      patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize  = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hcpInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->GetHcpStateCommandSize(
            mode, &hcpCommandsSize, &hcpPatchListSize, params));

        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hcpCommandsSize + cpCmdsize;
    *patchListSize = hcpPatchListSize + cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetHcpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize  = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hcpInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->GetHcpPrimitiveCommandSize(
            mode, &hcpCommandsSize, &hcpPatchListSize, modeSpecific ? true : false));

        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hcpCommandsSize + cpCmdsize;
    *patchListSize = hcpPatchListSize + cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetHucStateCommandSize(
    uint32_t                        mode,
    uint32_t *                      commandsSize,
    uint32_t *                      patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t standard         = CodecHal_GetStandardFromMode(mode);
    uint32_t hucCommandsSize  = 0;
    uint32_t hucPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hucInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_CENC || standard == CODECHAL_VP9 || standard == CODECHAL_AVC))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->GetHucStateCommandSize(
            mode, &hucCommandsSize, &hucPatchListSize, params));

        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hucCommandsSize + cpCmdsize;
    *patchListSize = hucPatchListSize + cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetHucPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t standard         = CodecHal_GetStandardFromMode(mode);
    uint32_t hucCommandsSize  = 0;
    uint32_t hucPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hucInterface && (standard == CODECHAL_HEVC || standard == CODECHAL_CENC || standard == CODECHAL_VP9))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->GetHucPrimitiveCommandSize(
            mode, &hucCommandsSize, &hucPatchListSize));
        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hucCommandsSize + cpCmdsize;
    *patchListSize = hucPatchListSize + cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterface::GetVdencStateCommandsDataSize(
    uint32_t                    mode,
    uint32_t                   *commandsSize,
    uint32_t                   *patchListSize)
{
    CODECHAL_HW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_miInterface);
    MHW_MI_CHK_NULL(m_hcpInterface);
    MHW_MI_CHK_NULL(m_vdencInterface);

    uint32_t commands = 0;
    uint32_t patchList = 0;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    MHW_MI_CHK_STATUS(m_vdencInterface->GetVdencStateCommandsDataSize(
        mode,
        0,
        &commands,
        &patchList));

    commands += m_sizeOfCmdMediaReset;

    if (standard == CODECHAL_AVC)
    {
        commands += m_miInterface->GetMiFlushDwCmdSize();
        commands += m_miInterface->GetMiBatchBufferStartCmdSize();
        commands += m_sizeOfCmdBatchBufferEnd;
    }
    else if (standard == CODECHAL_HEVC)
    {
        commands += m_miInterface->GetMiFlushDwCmdSize();
        commands += m_miInterface->GetMiBatchBufferStartCmdSize();
        commands += m_hcpInterface->GetHcpHevcVp9RdoqStateCommandSize();
        commands += m_sizeOfCmdBatchBufferEnd;
    }
    else if (standard == CODECHAL_VP9)
    {
        commands += m_miInterface->GetMiFlushDwCmdSize();
        commands += m_miInterface->GetMiBatchBufferStartCmdSize();
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported encode mode.");
        return MOS_STATUS_UNKNOWN;
    }

    *commandsSize = commands;
    *patchListSize = patchList;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterface::GetVdencPictureSecondLevelCommandsSize(
    uint32_t                    mode,
    uint32_t                   *commandsSize)
{
    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t commands = 0;

    MHW_MI_CHK_NULL(m_hcpInterface);
    MHW_MI_CHK_NULL(m_vdencInterface);

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_VP9)
    {
        commands += m_hcpInterface->GetHcpVp9PicStateCommandSize();
        commands += m_hcpInterface->GetHcpVp9SegmentStateCommandSize() * 8;
        commands += 128;
        commands += 220;
        commands += m_sizeOfCmdBatchBufferEnd;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported encode mode.");
        return MOS_STATUS_UNKNOWN;
    }

    *commandsSize = commands;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterface::GetStreamoutCommandSize(
    uint32_t   *commandsSize,
    uint32_t   *patchListSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;

    stateCmdSizeParams.bShortFormat = false;
    stateCmdSizeParams.bHucDummyStream = MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable);
    CODECHAL_HW_CHK_STATUS_RETURN(GetHxxStateCommandSize(
        CODECHAL_DECODE_MODE_CENC,  // For cenc phase
        commandsSize,
        patchListSize,
        &stateCmdSizeParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::Initialize(
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (CodecHalUsesRenderEngine(settings->codecFunction, settings->standard) ||
        CodecHalIsEnableFieldScaling(settings->codecFunction, settings->standard, settings->downsamplingHinted))
    {
        CODECHAL_HW_CHK_NULL_RETURN(m_renderInterface);

        m_stateHeapSettings.m_ishBehavior = HeapManager::Behavior::clientControlled;
        m_stateHeapSettings.m_dshBehavior = HeapManager::Behavior::destructiveExtend;
        // As a performance optimization keep the DSH locked always,
        // the ISH is only accessed at device creation and thus does not need to be locked
        m_stateHeapSettings.m_keepDshLocked = true;
        m_stateHeapSettings.dwDshIncrement = 2 * MOS_PAGE_SIZE;

        if (m_stateHeapSettings.dwIshSize > 0 &&
            m_stateHeapSettings.dwDshSize > 0 &&
            m_stateHeapSettings.dwNumSyncTags > 0)
        {
            CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->AllocateHeaps(
                m_stateHeapSettings));
        }
    }

    m_enableCodecMmc = !MEDIA_IS_WA(GetWaTable(), WaDisableCodecMmc) && settings->enableCodecMmc;

    return eStatus;
}

uint32_t CodechalHwInterface::GetMediaObjectBufferSize(
    uint32_t numMbs,
    uint32_t inlineDataSize)
{
    uint32_t maxSize = 0;

    maxSize +=
        (m_sizeOfCmdMediaObject + inlineDataSize) * numMbs +
        m_sizeOfCmdMediaStateFlush +
        m_sizeOfCmdBatchBufferEnd +
        128; // extra padding needed for scaling

    return maxSize;
}

MOS_STATUS CodechalHwInterface::AddVdencBrcImgBuffer(
    PMOS_RESOURCE               vdencBrcImgBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS   params)
{
    CODECHAL_HW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(m_mfxInterface);
    MHW_MI_CHK_NULL(m_vdencInterface);

    uint8_t                     *data = nullptr;
    MOS_LOCK_PARAMS             lockFlags;

    uint32_t mfxAvcImgStateSize = m_mfxInterface->GetAvcImgStateSize();
    uint32_t vdencAvcCostStateSize = m_vdencInterface->GetVdencAvcCostStateSize();
    uint32_t vdencAvcImgStateSize = m_vdencInterface->GetVdencAvcImgStateSize();

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, vdencBrcImgBuffer, &lockFlags);
    MHW_MI_CHK_NULL(data);

    MOS_COMMAND_BUFFER          constructedCmdBuf;

    MOS_ZeroMemory(&constructedCmdBuf, sizeof(MOS_COMMAND_BUFFER));
    constructedCmdBuf.pCmdBase = (uint32_t *)data;
    constructedCmdBuf.iRemaining = mfxAvcImgStateSize + vdencAvcCostStateSize + vdencAvcImgStateSize;

    // Set MFX_IMAGE_STATE command
    constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iOffset = 0;
    MHW_MI_CHK_STATUS(m_mfxInterface->AddMfxAvcImgCmd(&constructedCmdBuf, nullptr, params));

    // Set VDENC_COST_STATE command
    constructedCmdBuf.pCmdPtr = (uint32_t *)(data + mfxAvcImgStateSize);
    constructedCmdBuf.iOffset = mfxAvcImgStateSize;
    m_vdencInterface->AddVdencAvcCostStateCmd(&constructedCmdBuf, nullptr, params);

    // Set VDENC_IMAGE_STATE command
    constructedCmdBuf.pCmdPtr = (uint32_t *)(data + mfxAvcImgStateSize + vdencAvcCostStateSize);
    constructedCmdBuf.iOffset = mfxAvcImgStateSize + vdencAvcCostStateSize;

    MHW_MI_CHK_STATUS(m_vdencInterface->AddVdencImgStateCmd(&constructedCmdBuf, nullptr, params));

    // Add batch buffer end insertion flag
    m_miInterface->AddBatchBufferEndInsertionFlag(constructedCmdBuf);

    OVERRIDE_CMD_DATA(constructedCmdBuf.pCmdBase, constructedCmdBuf.iOffset);

    if (data)
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            vdencBrcImgBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterface::AddVdencSfdImgBuffer(
    PMOS_RESOURCE               vdencSfdImgBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS   params)
{
    CODECHAL_HW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(m_vdencInterface);

    uint8_t                     *data = nullptr;
    MOS_LOCK_PARAMS             lockFlags;

    uint32_t vdencAvcImgStateSize = m_vdencInterface->GetVdencAvcImgStateSize();

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, vdencSfdImgBuffer, &lockFlags);
    MHW_MI_CHK_NULL(data);

    MOS_COMMAND_BUFFER          constructedCmdBuf;

    MOS_ZeroMemory(&constructedCmdBuf, sizeof(MOS_COMMAND_BUFFER));
    constructedCmdBuf.pCmdBase = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iOffset = 0;
    constructedCmdBuf.iRemaining = vdencAvcImgStateSize + m_sizeOfCmdBatchBufferEnd;

    MHW_MI_CHK_STATUS(m_vdencInterface->AddVdencImgStateCmd(&constructedCmdBuf, nullptr, params));

    // Add batch buffer end insertion flag
    constructedCmdBuf.pCmdPtr = (uint32_t *)(data + vdencAvcImgStateSize);
    constructedCmdBuf.iOffset = vdencAvcImgStateSize;
    constructedCmdBuf.iRemaining = m_sizeOfCmdBatchBufferEnd;
    m_miInterface->AddBatchBufferEndInsertionFlag(constructedCmdBuf);

    if (data)
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            vdencSfdImgBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

uint32_t CodechalHwInterface::GetKernelLoadCommandSize(
    uint32_t maxNumSurfaces)
{
    CODECHAL_HW_FUNCTION_ENTER;

    MOS_UNUSED(maxNumSurfaces);

    return m_maxKernelLoadCmdSize;
}

MOS_STATUS CodechalHwInterface::ResizeCommandBufferAndPatchList(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    CODECHAL_HW_FUNCTION_ENTER;

    if (m_osInterface->bUsesCmdBufHeaderInResize)
    {
        return ResizeCommandBufferAndPatchListCmd(requestedCommandBufferSize, requestedPatchListSize);
    }
    else
    {
        return ResizeCommandBufferAndPatchListOs(requestedCommandBufferSize, requestedPatchListSize);
    }
}

MOS_STATUS CodechalHwInterface::ResizeCommandBufferAndPatchListCmd(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(m_miInterface);

    MOS_COMMAND_BUFFER cmdBuffer;

    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiNoop(&cmdBuffer, nullptr));
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);
    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(m_osInterface, requestedCommandBufferSize, requestedPatchListSize, 0));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ResizeCommandBufferAndPatchListOs(
    uint32_t                    requestedCommandBufferSize,
    uint32_t                    requestedPatchListSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnResizeCommandBufferAndPatchList(m_osInterface, requestedCommandBufferSize, requestedPatchListSize, 0));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::WriteSyncTagToResource(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    PMOS_SYNC_PARAMS            syncParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(m_miInterface);
    MOS_UNUSED(syncParams);

    MHW_MI_STORE_DATA_PARAMS        params;
    MOS_RESOURCE                    globalGpuContextSyncTagBuffer;
    uint32_t                        offset = 0;
    uint32_t                        value = 0;

    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
        m_osInterface,
        &globalGpuContextSyncTagBuffer));

    offset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
    value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);

    params.pOsResource = &globalGpuContextSyncTagBuffer;
    params.dwResourceOffset = offset;
    params.dwValue = value;

    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &params));

    // Increment GPU Context Tag for next use
    m_osInterface->pfnIncrementGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);

    return eStatus;
}

uint32_t CodechalHwInterface::ComposeSurfaceCacheabilityControl(
    uint32_t                cacheabiltySettingIdx,
    uint32_t                cacheabilityTypeRequested)
{
    CODECHAL_HW_FUNCTION_ENTER;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheSetting = m_cacheabilitySettings[cacheabiltySettingIdx];

    if (m_noSeparateL3LlcCacheabilitySettings)
    {
        if (cacheabilityTypeRequested == codechalUncacheableWa)
        {
            if (cacheSetting.Gen8.TargetCache == CODECHAL_MO_TARGET_CACHE_ELLC)
            {
                //Check if the SKU doesn't have EDRAM
                if (!MEDIA_IS_SKU(m_skuTable, FtrEDram))
                {
                    // No eDRAM, set the caching to uncache
                    cacheSetting.Gen8.CacheControl = 1;
                }
            }
        }
        // No separate LLC and L3 cacheability settings in the memory control object, so just return the value
        return (uint32_t)cacheSetting.Value;
    }

    return (uint32_t)m_cacheabilitySettings[cacheabiltySettingIdx].Value;
}

MOS_STATUS CodechalHwInterface::AddHucDummyStreamOut(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable))
    {
        return eStatus;
    }

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_HW_CHK_NULL_RETURN(m_miInterface);

    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   pipeModeSelectParams;
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  indObjParams;
    MHW_VDBOX_HUC_STREAM_OBJ_PARAMS     streamObjParams;

    if (Mos_ResourceIsNull(&m_dummyStreamOut))
    {
        MOS_LOCK_PARAMS         lockFlags;
        uint8_t*                data;
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        m_dmemBufSize = MHW_CACHELINE_SIZE;

        allocParamsForBufferLinear.dwBytes = m_dmemBufSize;
        allocParamsForBufferLinear.pBufName = "HucDmemBufferDummy";
        CODECHAL_HW_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_hucDmemDummy));
        // set lock flag to WRITE_ONLY
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        data =
            (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_hucDmemDummy, &lockFlags);
        CODECHAL_HW_CHK_NULL_RETURN(data);
        MOS_ZeroMemory(data, m_dmemBufSize);
        *data = 8;
        m_osInterface->pfnUnlockResource(m_osInterface, &m_hucDmemDummy);

        allocParamsForBufferLinear.dwBytes = CODECHAL_CACHELINE_SIZE;

        allocParamsForBufferLinear.pBufName = "HucDummyStreamInBuffer";
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_dummyStreamIn));
        allocParamsForBufferLinear.pBufName = "HucDummyStreamOutBuffer";
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_dummyStreamOut));
    }

    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    // pipe mode select
    pipeModeSelectParams.dwMediaSoftResetCounterValue = 2400;

    // pass bit-stream buffer by Ind Obj Addr command. Set size to 1 for dummy stream
    MOS_ZeroMemory(&indObjParams, sizeof(indObjParams));
    indObjParams.presDataBuffer = &m_dummyStreamIn;
    indObjParams.dwDataSize = 1;
    indObjParams.presStreamOutObjectBuffer = &m_dummyStreamOut;
    indObjParams.dwStreamOutObjectSize = 1;

    // set stream object with stream out enabled
    MOS_ZeroMemory(&streamObjParams, sizeof(streamObjParams));
    streamObjParams.dwIndStreamInLength = 1;
    streamObjParams.dwIndStreamInStartAddrOffset = 0;
    streamObjParams.bHucProcessing = true;
    streamObjParams.dwIndStreamOutStartAddrOffset = 0;
    streamObjParams.bStreamOutEnable = 1;

    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    MHW_VDBOX_HUC_IMEM_STATE_PARAMS     imemParams;
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS     dmemParams;
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS   virtualAddrParams;

    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = VDBOX_HUC_VDENC_BRC_INIT_KERNEL_DESCRIPTOR;

    // set HuC DMEM param
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_hucDmemDummy;
    dmemParams.dwDataLength = m_dmemBufSize;
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;

    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_dummyStreamOut;

    streamObjParams.bHucProcessing = true;
    streamObjParams.bStreamInEnable = 1;

    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(cmdBuffer, &imemParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(cmdBuffer, &dmemParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucIndObjBaseAddrStateCmd(cmdBuffer, &indObjParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucStreamObjectCmd(cmdBuffer, &streamObjParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(cmdBuffer, true));

    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;

}

MOS_STATUS CodechalHwInterface::PerformHucStreamOut(
    CodechalHucStreamoutParams  *hucStreamOutParams,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;
    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    if (MEDIA_IS_SKU(m_skuTable, FtrEnableMediaKernels) && MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(AddHucDummyStreamOut(cmdBuffer));
    }

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   pipeModeSelectParams;
    pipeModeSelectParams.Mode = hucStreamOutParams->mode;
    pipeModeSelectParams.dwMediaSoftResetCounterValue = 2400;
    pipeModeSelectParams.bStreamObjectUsed = true;
    pipeModeSelectParams.bStreamOutEnabled = true;
    if (hucStreamOutParams->segmentInfo == nullptr && m_osInterface->osCpInterface->IsCpEnabled())
    {
        // Disable protection control setting in huc drm
        pipeModeSelectParams.disableProtectionSetting = true;
    }

    // Enlarge the stream in/out data size to avoid upper bound hit assert in HuC
    hucStreamOutParams->dataSize += hucStreamOutParams->inputRelativeOffset;
    hucStreamOutParams->streamOutObjectSize += hucStreamOutParams->outputRelativeOffset;

    // pass bit-stream buffer by Ind Obj Addr command
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  indObjParams;
    MOS_ZeroMemory(&indObjParams, sizeof(indObjParams));
    indObjParams.presDataBuffer = hucStreamOutParams->dataBuffer;
    indObjParams.dwDataSize = MOS_ALIGN_CEIL(hucStreamOutParams->dataSize, MHW_PAGE_SIZE);
    indObjParams.dwDataOffset = hucStreamOutParams->dataOffset;
    indObjParams.presStreamOutObjectBuffer = hucStreamOutParams->streamOutObjectBuffer;
    indObjParams.dwStreamOutObjectSize = MOS_ALIGN_CEIL(hucStreamOutParams->streamOutObjectSize, MHW_PAGE_SIZE);
    indObjParams.dwStreamOutObjectOffset = hucStreamOutParams->streamOutObjectOffset;

    // set stream object with stream out enabled
    MHW_VDBOX_HUC_STREAM_OBJ_PARAMS     streamObjParams;
    MOS_ZeroMemory(&streamObjParams, sizeof(streamObjParams));
    streamObjParams.dwIndStreamInLength = hucStreamOutParams->indStreamInLength;
    streamObjParams.dwIndStreamInStartAddrOffset = hucStreamOutParams->inputRelativeOffset;
    streamObjParams.dwIndStreamOutStartAddrOffset = hucStreamOutParams->outputRelativeOffset;
    streamObjParams.bHucProcessing = true;
    streamObjParams.bStreamInEnable = true;
    streamObjParams.bStreamOutEnable = true;

    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucIndObjBaseAddrStateCmd(cmdBuffer, &indObjParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->AddHucStreamObjectCmd(cmdBuffer, &streamObjParams));

    // This flag is always false if huc fw is not loaded.
    if (MEDIA_IS_SKU(m_skuTable, FtrEnableMediaKernels) &&
        MEDIA_IS_WA(m_waTable, WaHucStreamoutOnlyDisable))
    {
        CODECHAL_HW_CHK_STATUS_RETURN(AddHucDummyStreamOut(cmdBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterface::UpdateSSEuForCmdBuffer(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    bool                                singleTaskPhaseSupported,
    bool                                lastTaskInPhase)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    if (singleTaskPhaseSupported && lastTaskInPhase)
    {
        PMOS_COMMAND_BUFFER_ATTRIBUTES  attributes = &cmdBuffer->Attributes;    // Base of the command buffer is where the command buffer header would be

        // Update the SSEU realted values in the comand buffer header header with the final values in the HwInterface structure
        attributes->dwNumRequestedEUSlices = m_numRequestedEuSlices;
        attributes->dwNumRequestedSubSlices = m_numRequestedSubSlices;
        attributes->dwNumRequestedEUs = m_numRequestedEus;
        CODECHAL_HW_VERBOSEMESSAGE("CB Attributes Updated: S/SS/EU: %d/%d/%d.", m_numRequestedEuSlices, m_numRequestedSubSlices, m_numRequestedEus);
    }

    // Clear the SSEU setting in HwInterface always for non-singletaskphase mode and after updating command buffer header for singletaskphase mode
    if ((!singleTaskPhaseSupported) || (singleTaskPhaseSupported && lastTaskInPhase))
    {
        m_numRequestedSubSlices = 0;
        m_numRequestedEus = 0;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterface::GetDefaultSSEuSetting(
    CODECHAL_MEDIA_STATE_TYPE           mediaStateType,
    bool                                setRequestedSlices,
    bool                                setRequestedSubSlices,
    bool                                setRequestedEus)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MOS_UNUSED(setRequestedSlices);

    if (m_ssEuTable == nullptr) // SSEUTable may be nullptr if not initialized(for newer platforms) or not supported(on HSW). Its not an error, just return success.
    {
        return eStatus;
    }

    if (mediaStateType >= CODECHAL_NUM_MEDIA_STATES)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    CODECHAL_SSEU_SETTING const *ssEutable = m_ssEuTable + mediaStateType;

    if (m_numRequestedEuSlices != CODECHAL_SLICE_SHUTDOWN_DEFAULT)        // Num Slices will be set based on table only if it is not set to default (HALO)
    {
        if (m_numRequestedEuSlices < ssEutable->ui8NumSlices)
        {
            m_numRequestedEuSlices = ssEutable->ui8NumSlices;
        }
    }
    if (!setRequestedSubSlices)                     // If num Sub-Slices is already programmed, then don't change it
    {
        if (m_numRequestedSubSlices < ssEutable->ui8NumSubSlices)
        {
            m_numRequestedSubSlices = ssEutable->ui8NumSubSlices;
        }
    }
    if (!setRequestedEus)                           // If num EUs is already programmed, then don't change it
    {
        if (m_numRequestedEus < ssEutable->ui8NumEUs)
        {
            m_numRequestedEus = ssEutable->ui8NumEUs;
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE_ID,
        &userFeatureData);
    if (userFeatureData.i32Data != 0xDEADC0DE)
    {
        m_numRequestedEuSlices = userFeatureData.i32Data & 0xFF;              // Bits 0-7
        m_numRequestedSubSlices = (userFeatureData.i32Data >> 8) & 0xFF;       // Bits 8-15
        m_numRequestedEus = (userFeatureData.i32Data >> 16) & 0xFFFF;    // Bits 16-31
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalHwInterface::CopyDataSourceWithDrv(
    CodechalDataCopyParams          *dataCopyParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;
    CODECHAL_HW_CHK_NULL_RETURN(dataCopyParams);
    CODECHAL_HW_CHK_NULL_RETURN(dataCopyParams->srcResource);
    CODECHAL_HW_CHK_NULL_RETURN(dataCopyParams->dstResource)

    uint8_t*                            src = nullptr;
    uint8_t*                            dst = nullptr;
    MOS_LOCK_PARAMS                     lockFlags;

    // initiate lock flags
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    src = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, dataCopyParams->srcResource, &lockFlags);
    if (nullptr != src)
    {
        src += dataCopyParams->srcOffset;

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        dst = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, dataCopyParams->dstResource, &lockFlags);
        if (nullptr != dst)
        {
            dst += dataCopyParams->dstOffset;
            eStatus = MOS_SecureMemcpy(dst, dataCopyParams->dstSize, src, dataCopyParams->srcSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_HW_ASSERTMESSAGE("Failed to copy memory.");
                return eStatus;
            }

            m_osInterface->pfnUnlockResource(m_osInterface, dataCopyParams->dstResource);
        }

        m_osInterface->pfnUnlockResource(m_osInterface, dataCopyParams->srcResource);
    }

    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodechalHwInterface::InitL3ControlUserFeatureSettings(
    MHW_RENDER_ENGINE_L3_CACHE_CONFIG   *l3CacheConfig,
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *l3Overrides)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(l3CacheConfig);
    CODECHAL_HW_CHK_NULL_RETURN(l3Overrides);

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data = l3CacheConfig->dwL3CacheCntlReg_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG_OVERRIDE_ID,
        &userFeatureData);
    l3Overrides->dwCntlReg = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data = l3CacheConfig->dwL3CacheCntlReg2_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG2_OVERRIDE_ID,
        &userFeatureData);
    l3Overrides->dwCntlReg2 = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data = l3CacheConfig->dwL3CacheCntlReg3_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG3_OVERRIDE_ID,
        &userFeatureData);
    l3Overrides->dwCntlReg3 = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data = l3CacheConfig->dwL3CacheSqcReg1_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG1_OVERRIDE_ID,
        &userFeatureData);
    l3Overrides->dwSqcReg1 = (uint32_t)userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.u32Data = l3CacheConfig->dwL3CacheSqcReg4_Setting;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG4_OVERRIDE_ID,
        &userFeatureData);
    l3Overrides->dwSqcReg4 = (uint32_t)userFeatureData.i32Data;

    if (l3CacheConfig->bL3LRA1Reset)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.u32Data = l3CacheConfig->dwL3LRA1Reg_Setting;
        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_LRA_1_REG1_OVERRIDE_ID,
            &userFeatureData);
        l3Overrides->dwLra1Reg = (uint32_t)userFeatureData.i32Data;
    }

    return MOS_STATUS_SUCCESS;
}
#endif // _DEBUG || _RELEASE_INTERNAL

MOS_STATUS CodechalHwInterface::SendHwSemaphoreWaitCmd(
    PMOS_RESOURCE                               semaMem,
    uint32_t                                    semaData,
    MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION   opCode,
    PMOS_COMMAND_BUFFER                         cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MHW_MI_SEMAPHORE_WAIT_PARAMS       miSemaphoreWaitParams;
    MOS_ZeroMemory((&miSemaphoreWaitParams), sizeof(miSemaphoreWaitParams));
    miSemaphoreWaitParams.presSemaphoreMem = semaMem;
    miSemaphoreWaitParams.bPollingWaitMode = true;
    miSemaphoreWaitParams.dwSemaphoreData = semaData;
    miSemaphoreWaitParams.CompareOperation = opCode;
    eStatus = m_miInterface->AddMiSemaphoreWaitCmd(cmdBuffer, &miSemaphoreWaitParams);

    return eStatus;
}

MOS_STATUS CodechalHwInterface::SendMiAtomicDwordCmd(
    PMOS_RESOURCE               resource,
    uint32_t                    immData,
    MHW_COMMON_MI_ATOMIC_OPCODE opCode,
    PMOS_COMMAND_BUFFER         cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(m_miInterface);

    MHW_MI_ATOMIC_PARAMS atomicParams;
    MOS_ZeroMemory((&atomicParams), sizeof(atomicParams));
    atomicParams.pOsResource = resource;
    atomicParams.dwDataSize = sizeof(uint32_t);
    atomicParams.Operation = opCode;
    atomicParams.bInlineData = true;
    atomicParams.dwOperand1Data[0] = immData;
    eStatus = m_miInterface->AddMiAtomicCmd(cmdBuffer, &atomicParams);

    return eStatus;
}

MOS_STATUS CodechalHwInterface::SendCondBbEndCmd(
    PMOS_RESOURCE              resource,
    uint32_t                   offset,
    uint32_t                   compData,
    bool                       disableCompMask,
    PMOS_COMMAND_BUFFER        cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&m_conditionalBbEndDummy))
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.postSyncOperation = 1;
        flushDwParams.pOsResource = &m_conditionalBbEndDummy;
        flushDwParams.dwDataDW1 = 0;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS conditionalBatchBufferEndParams;
    MOS_ZeroMemory(&conditionalBatchBufferEndParams, sizeof(conditionalBatchBufferEndParams));
    conditionalBatchBufferEndParams.presSemaphoreBuffer = resource;
    conditionalBatchBufferEndParams.dwOffset = offset;
    conditionalBatchBufferEndParams.dwValue = compData;
    conditionalBatchBufferEndParams.bDisableCompareMask = disableCompMask;
    eStatus = m_miInterface->AddMiConditionalBatchBufferEndCmd(cmdBuffer, &conditionalBatchBufferEndParams);

    return eStatus;
}

MOS_STATUS CodechalHwInterface::MhwInitISH(
    PMHW_STATE_HEAP_INTERFACE   stateHeapInterface,
    PMHW_KERNEL_STATE           kernelState)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(stateHeapInterface);
    CODECHAL_HW_CHK_NULL_RETURN(kernelState);

    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_HW_CHK_STATUS_RETURN(stateHeapInterface->pfnAssignSpaceInStateHeap(
        stateHeapInterface,
        MHW_ISH_TYPE,
        kernelState,
        kernelState->KernelParams.iSize,
        true,
        false));

    CODECHAL_HW_CHK_STATUS_RETURN(kernelState->m_ishRegion.AddData(
        kernelState->KernelParams.pBinary,
        0,
        kernelState->KernelParams.iSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterface::AssignDshAndSshSpace(
    PMHW_STATE_HEAP_INTERFACE   stateHeapInterface,
    PMHW_KERNEL_STATE           kernelState,
    bool                        noDshSpaceRequested,
    uint32_t                    forcedDshSize,
    bool                        noSshSpaceRequested,
    uint32_t                    currCmdBufId)
{
    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(stateHeapInterface);
    CODECHAL_HW_CHK_NULL_RETURN(kernelState);

    kernelState->m_currTrackerId = currCmdBufId;

    if (!noDshSpaceRequested)
    {
        uint32_t dshSize = 0;
        if (forcedDshSize != 0)
        {
            dshSize = forcedDshSize;
        }
        else
        {
            dshSize = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData()+
                MOS_ALIGN_CEIL(kernelState->KernelParams.iCurbeLength,
                stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        }

        CODECHAL_HW_CHK_STATUS_RETURN(stateHeapInterface->pfnAssignSpaceInStateHeap(
            stateHeapInterface,
            MHW_DSH_TYPE,
            kernelState,
            dshSize,
            false,
            true));
    }

    if (!noSshSpaceRequested)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(stateHeapInterface->pfnAssignSpaceInStateHeap(
            stateHeapInterface,
            MHW_SSH_TYPE,
            kernelState,
            kernelState->dwSshSize,
            false,
            false));
    }

    return MOS_STATUS_SUCCESS;
}

MmioRegistersMfx * CodechalHwInterface::SelectVdboxAndGetMmioRegister(
    MHW_VDBOX_NODE_IND index,
    PMOS_COMMAND_BUFFER pCmdBuffer)
{
    if (m_getVdboxNodeByUMD)
    {
        pCmdBuffer->iVdboxNodeIndex = m_osInterface->pfnGetVdboxNodeId(m_osInterface, pCmdBuffer);
        switch (pCmdBuffer->iVdboxNodeIndex)
        {
                case MOS_VDBOX_NODE_1:
                    index = MHW_VDBOX_NODE_1;
                    break;
                case MOS_VDBOX_NODE_2:
                    index = MHW_VDBOX_NODE_2;
                    break;
                case MOS_VDBOX_NODE_INVALID:
                    // That's a legal case meaning that we were not assigned with per-bb index because
                    // balancing algorithm can't work (forcedly diabled or miss kernel support).
                    // If that's the case we just proceed with the further static context assignment.
                    break;
                default:
                    // That's the case when MHW and MOS enumerations mismatch. We again proceed with the
                    // best effort (static context assignment, but provide debug note).
                    MHW_ASSERTMESSAGE("MOS and MHW VDBOX enumerations mismatch! Adjust HW description!");
                    break;
        }
    } 

    return m_mfxInterface->GetMmioRegisters(index);
}

MOS_STATUS CodechalHwInterface::SendMiStoreDataImm(
    PMOS_RESOURCE       resource,
    uint32_t            immData,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(resource);
    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = resource;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue = immData;

    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ReadMfcStatus(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const EncodeStatusReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    MmioRegistersMfx* mmioRegisters = SelectVdboxAndGetMmioRegister(vdboxIndex, cmdBuffer);

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
    miStoreRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.resBitstreamSyntaxElementOnlyBitCount;
    miStoreRegMemParams.dwOffset        = params.bitstreamSyntaxElementOnlyBitCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.resQpStatusCount;
    miStoreRegMemParams.dwOffset        = params.qpStatusCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcQPStatusCountOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    if (mmioRegisters->mfcAvcNumSlicesRegOffset > 0)
    {
        //read MFC_AVC_NUM_SLICES register to status report
        miStoreRegMemParams.presStoreBuffer = params.resNumSlices;
        miStoreRegMemParams.dwOffset        = params.numSlicesOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->mfcAvcNumSlicesRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
    }

    if (params.vdencBrcEnabled)
    {
        // Store PAK FrameSize MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame.
        for (int i = 0; i < 2; i++)
        {
            if (params.resVdencBrcUpdateDmemBufferPtr[i])
            {
                miStoreRegMemParams.presStoreBuffer = params.resVdencBrcUpdateDmemBufferPtr[i];
                miStoreRegMemParams.dwOffset        = 5 * sizeof(uint32_t);
                miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
                CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

                if (params.vdencBrcNumOfSliceOffset)
                {
                    miStoreRegMemParams.presStoreBuffer = params.resVdencBrcUpdateDmemBufferPtr[i];
                    miStoreRegMemParams.dwOffset        = params.vdencBrcNumOfSliceOffset;
                    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcAvcNumSlicesRegOffset;
                    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
                }
            }
        }
    }

    CODECHAL_HW_CHK_STATUS_RETURN(ReadImageStatus(vdboxIndex, params, cmdBuffer));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ReadImageStatus(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const EncodeStatusReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    MmioRegistersMfx* mmioRegisters = SelectVdboxAndGetMmioRegister(vdboxIndex, cmdBuffer);

    MOS_RESOURCE *osResource;
    uint32_t     offset;

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.resImageStatusMask;
    miStoreRegMemParams.dwOffset        = params.imageStatusMaskOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusMaskRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.resImageStatusCtrl;
    miStoreRegMemParams.dwOffset        = params.imageStatusCtrlOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusCtrlRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // VDEnc dynamic slice overflow semaphore, DW0 is SW programmed mask(MFX_IMAGE_MASK does not support), DW1 is MFX_IMAGE_STATUS_CONTROL
    if (params.vdencBrcEnabled)
    {
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;

        // Added for VDEnc slice overflow bit in MFC_IMAGE_STATUS_CONTROL
        // The bit is connected on the non-AVC encoder side of MMIO register.
        // Need a dummy MFX_PIPE_MODE_SELECT to decoder and read this register.
        if (params.waReadVDEncOverflowStatus)
        {
            MOS_ZeroMemory(&pipeModeSelectParams, sizeof(pipeModeSelectParams));
            pipeModeSelectParams.Mode               = CODECHAL_DECODE_MODE_AVCVLD;
            m_mfxInterface->SetDecodeInUse(true);
            CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));
        }

        // Store MFC_IMAGE_STATUS_CONTROL MMIO to DMEM for HuC next BRC pass of current frame and first pass of next frame.
        for (int i = 0; i < 2; i++)
        {
            if (params.resVdencBrcUpdateDmemBufferPtr[i])
            {
                miStoreRegMemParams.presStoreBuffer    = params.resVdencBrcUpdateDmemBufferPtr[i];
                miStoreRegMemParams.dwOffset           = 7 * sizeof(uint32_t); // offset of SliceSizeViolation in HUC_BRC_UPDATE_DMEM
                miStoreRegMemParams.dwRegister         = mmioRegisters->mfcImageStatusCtrlRegOffset;
                CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
            }
        }

        // Restore MFX_PIPE_MODE_SELECT to encode mode
        if (params.waReadVDEncOverflowStatus)
        {
            MOS_ZeroMemory(&pipeModeSelectParams, sizeof(pipeModeSelectParams));
            pipeModeSelectParams.Mode               = params.mode;
            pipeModeSelectParams.bVdencEnabled      = true;
            m_mfxInterface->SetDecodeInUse(false);
            CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));
        }
    }

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ReadBrcPakStatistics(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const BrcPakStatsReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_HW_CHK_NULL_RETURN(params.presBrcPakStatisticBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");
    MmioRegistersMfx* mmioRegisters = SelectVdboxAndGetMmioRegister(vdboxIndex, cmdBuffer);

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset        = 0;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountFrameRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params.presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset        = sizeof(uint32_t);
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcBitstreamBytecountSliceRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource         = params.presBrcPakStatisticBuffer;
    storeDataParams.dwResourceOffset    = sizeof(uint32_t) * 2;
    storeDataParams.dwValue             = params.ucPass + 1;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    storeDataParams.pOsResource         = params.presStatusBuffer;
    storeDataParams.dwResourceOffset    = params.dwStatusBufNumPassesOffset;
    storeDataParams.dwValue             = params.ucPass + 1;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    miStoreRegMemParams.presStoreBuffer = params.presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset        = sizeof(uint32_t) * (4 + params.ucPass);
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfcImageStatusCtrlRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ReadHcpStatus(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const EncodeStatusReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(vdboxIndex);

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
    miStoreRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.resBitstreamSyntaxElementOnlyBitCount;
    miStoreRegMemParams.dwOffset        = params.bitstreamSyntaxElementOnlyBitCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.resQpStatusCount;
    miStoreRegMemParams.dwOffset        = params.qpStatusCountOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncQpStatusCountRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ReadImageStatusForHcp(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const EncodeStatusReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(vdboxIndex);

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.resImageStatusMask;
    miStoreRegMemParams.dwOffset        = params.imageStatusMaskOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusMaskRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.resImageStatusCtrl;
    miStoreRegMemParams.dwOffset        = params.imageStatusCtrlOffset;
    miStoreRegMemParams.dwRegister      = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::ReadBrcPakStatisticsForHcp(
    MHW_VDBOX_NODE_IND vdboxIndex,
    const BrcPakStatsReadParams &params,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_HW_CHK_NULL_RETURN(params.presBrcPakStatisticBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(vdboxIndex);

    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = params.bitstreamBytecountFrameOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = params.bitstreamBytecountFrameNoHeaderOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
    miStoreRegMemParams.presStoreBuffer = params.presBrcPakStatisticBuffer;
    miStoreRegMemParams.dwOffset = params.imageStatusCtrlOffset;
    miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    storeDataParams.pOsResource = params.presStatusBuffer;
    storeDataParams.dwResourceOffset = params.dwStatusBufNumPassesOffset;
    storeDataParams.dwValue = params.ucPass;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterface::SetStatusTagByPipeCtrl(
    PMOS_RESOURCE osResource,
    uint32_t offset,
    uint32_t tag,
    bool needFlushCache,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    MHW_PIPE_CONTROL_PARAMS pipeControlParams;
    MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));

    if (!needFlushCache)
    {
        pipeControlParams.presDest                  = osResource;
        pipeControlParams.dwPostSyncOp              = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
        pipeControlParams.dwResourceOffset          = offset;
        pipeControlParams.dwDataDW1                 = tag;
        result = m_miInterface->AddPipeControl(
            cmdBuffer,
            nullptr,
            &pipeControlParams);

        return result;
    }

    pipeControlParams.dwFlushMode  = MHW_FLUSH_WRITE_CACHE;
    pipeControlParams.bGenericMediaStateClear = true;
    CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(cmdBuffer, nullptr, &pipeControlParams));
    
    if (MEDIA_IS_WA(m_waTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS vfeStateParams;
    
        MOS_ZeroMemory(&vfeStateParams, sizeof(vfeStateParams));
        vfeStateParams.dwNumberofURBEntries = 1;
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->AddMediaVfeCmd(cmdBuffer, &vfeStateParams));
    }

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(MHW_MI_STORE_DATA_PARAMS));
    storeDataParams.pOsResource      = osResource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = tag;
    result = m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams);

    return result;
}

MOS_STATUS CodechalHwInterface::SetStatusTagByMiCommand(
    PMOS_RESOURCE osResource,
    uint32_t offset,
    uint32_t tag,
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS result = MOS_STATUS_SUCCESS;

    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(MHW_MI_STORE_DATA_PARAMS));
    storeDataParams.pOsResource      = osResource;
    storeDataParams.dwResourceOffset = offset;
    storeDataParams.dwValue          = tag;

    result = m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams);

    return result;
}
