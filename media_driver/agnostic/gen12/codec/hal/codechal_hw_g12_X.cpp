/*
* Copyright (c) 2017-2022, Intel Corporation
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
//! \file     codechal_hw_g12_X.cpp
//! \brief    Implements HW interface layer for Gen12+ used on all OSs.
//! \details  Implements HW interface layer for CNL+ to be used on on all operating systems/DDIs, across CODECHAL components.
//!           This module must not contain any OS dependent code.
//!

#include "codechal_hw_g12_X.h"
#include "mhw_render_g12_X.h"
#include "mhw_mi_hwcmd_g12_X.h"
#include "mhw_vdbox_hcp_hwcmd_g12_X.h"  // temporary include for calculating size of various hardware commands
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "media_interfaces_g12_tgllp.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#include "codechal_utilities.h"
#include "codeckrnheader.h"

// Currently initialized with dummy values, just as an example. Will be updated later.
const CODECHAL_SSEU_SETTING CodechalHwInterfaceG12::m_defaultSsEuLutG12[CODECHAL_NUM_MEDIA_STATES_G12] =
    {
    // Slice    Sub-Slice   EU      Rsvd(freq)
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_OLP
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_NORMAL
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_PERFORMANCE
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_QUALITY
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_32X_SCALING
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_16X_SCALING
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_4X_SCALING
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_32X_ME
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_16X_ME
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_4X_ME
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_BRC_INIT_RESET
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_BRC_UPDATE
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_BRC_BLOCK_COPY
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HYBRID_PAK_P1
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HYBRID_PAK_P2
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_I_FRAME_CHROMA
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_I_FRAME_LUMA
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_MPU_FHB
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_TPU_FHB
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_PA_COPY
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_PL2_COPY
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_ADV
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_2X_SCALING
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_16x16_PU_SAD
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_8x8_PU
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_8x8_PU_FMODE
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HEVC_B_MBENC
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_RESET_VLINE_STRIDE
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HEVC_B_PAK
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_P
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_TX
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_RECON
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_RECON
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_DEBLOCK_MASK
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_DEBLOCK
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_DEBLOCK
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_MC_PRED
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_LUMA_RECON
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_CHROMA_RECON
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_LUMA_RECON
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_CHROMA_RECON
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_PREPROC
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_ENC_WP
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HEVC_I_MBENC
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_CSC_DS_COPY
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_2X_4X_SCALING
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_MB_BRC_UPDATE
    { 1,        0,        8,         0 },    // CODECHAL_MEDIA_STATE_STATIC_FRAME_DETECTION
    { 1,        0,        8,         0 },
    { 1,        0,        8,         0 },
    { 1,        0,        8,         0 },
    { 1,        0,        8,         0 },
    { 2,        3,        8,         0 },
    { 2,        3,        8,         0 },
    { 2,        3,        8,         0 },
    { 2,        3,        8,         0 },
};

void CodechalHwInterfaceG12::InternalInit(CODECHAL_FUNCTION codecFunction)
{
    InitCacheabilityControlSettings(codecFunction);
    m_isVdencSuperSliceEnabled = true;
    m_ssEuTable = m_defaultSsEuLutG12;
    m_numMediaStates = CODECHAL_NUM_MEDIA_STATES_G12;

    PrepareCmdSize(codecFunction);
}

void CodechalHwInterfaceG12::PrepareCmdSize(CODECHAL_FUNCTION codecFunction)
{
    // Set platform dependent parameters
    m_sizeOfCmdBatchBufferEnd = mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_sizeOfCmdMediaReset = mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 8;
    m_vdencBrcImgStateBufferSize = mhw_vdbox_vdenc_g12_X::VDENC_IMG_STATE_CMD::byteSize + mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize +
        mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;;

    m_vdencBatchBuffer1stGroupSize = mhw_vdbox_hcp_g12_X::HCP_PIPE_MODE_SELECT_CMD::byteSize
        + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 2
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer2ndGroupSize = 124
        + mhw_vdbox_hcp_g12_X::HCP_PIC_STATE_CMD::byteSize
        + 204
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencReadBatchBufferSize =
    m_vdenc2ndLevelBatchBufferSize = m_vdencBatchBuffer1stGroupSize
        + m_vdencBatchBuffer2ndGroupSize
        + ENCODE_HEVC_VDENC_NUM_MAX_SLICES
        * (2 * mhw_vdbox_hcp_g12_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize
            + mhw_vdbox_hcp_g12_X::HCP_SLICE_STATE_CMD::byteSize
            + (HEVC_MAX_NAL_UNIT_TYPE + 2) * mhw_vdbox_hcp_g12_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize
            + mhw_vdbox_vdenc_g12_X::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize
            + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize
            + 4 * ENCODE_VDENC_HEVC_PADDING_DW_SIZE);

    m_HucStitchCmdBatchBufferSize = 7 * 4 
                                    + 14 * 4 
                                    + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;
                                    
    // HCP_WEIGHTOFFSET_STATE_CMD cmds is planned to be added in near future
    m_vdencBatchBufferPerSliceConstSize = mhw_vdbox_hcp_g12_X::HCP_SLICE_STATE_CMD::byteSize
        + mhw_vdbox_hcp_g12_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize          // 1st PakInsertObject cmd is not always inserted for each slice, 2nd PakInsertObject cmd is always inserted for each slice
        + mhw_vdbox_vdenc_g12_X::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    // Set to size of the BRC update command buffer, since it is larger than BRC Init/ PAK integration commands
    m_hucCommandBufferSize = mhw_vdbox_huc_g12_X::HUC_IMEM_STATE_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_PIPE_MODE_SELECT_CMD::byteSize
        + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 3
        + mhw_vdbox_huc_g12_X::HUC_DMEM_STATE_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_VIRTUAL_ADDR_STATE_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_STREAM_OBJECT_CMD::byteSize
        + mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize
        + mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_START_CMD::byteSize
        + mhw_vdbox_vdenc_g12_X::VD_PIPELINE_FLUSH_CMD::byteSize
        + mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize
        + mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize * 2
        + mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 2
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_maxKernelLoadCmdSize =
        mhw_mi_g12_X::PIPE_CONTROL_CMD::byteSize +
        mhw_render_g12_X::PIPELINE_SELECT_CMD::byteSize +
        mhw_render_g12_X::MEDIA_OBJECT_CMD::byteSize +
        mhw_render_g12_X::STATE_BASE_ADDRESS_CMD::byteSize +
        mhw_render_g12_X::MEDIA_VFE_STATE_CMD::byteSize +
        mhw_render_g12_X::MEDIA_CURBE_LOAD_CMD::byteSize +
        mhw_render_g12_X::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD::byteSize +
        mhw_mi_g12_X::MI_BATCH_BUFFER_START_CMD::byteSize +
        mhw_render_g12_X::MEDIA_OBJECT_WALKER_CMD::byteSize +
        mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize;

    m_sizeOfCmdMediaObject = mhw_render_g12_X::MEDIA_OBJECT_CMD::byteSize;
    m_sizeOfCmdMediaStateFlush = mhw_mi_g12_X::MEDIA_STATE_FLUSH_CMD::byteSize;
}

CodechalHwInterfaceG12::CodechalHwInterfaceG12(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfaces     *mhwInterfaces,
    bool              disableScalability)
    : CodechalHwInterface(osInterface, codecFunction, mhwInterfaces, disableScalability)
{
    CODECHAL_HW_FUNCTION_ENTER;

    MhwInterfacesG12Tgllp *mhwItfG12Tgllp = static_cast<MhwInterfacesG12Tgllp *>(mhwInterfaces);
    CODECHAL_HW_ASSERT(mhwItfG12Tgllp);
    m_avpInterface                        = mhwItfG12Tgllp->m_avpInterface;
    //Set the original m_avpInterface to nullptr to avoid double free and wild pointer
    mhwItfG12Tgllp->m_avpInterface        = nullptr;

    m_mediaSfcItf  = std::make_shared<MediaSfcInterfaceLegacy>(m_osInterface);

    InternalInit(codecFunction);
}

MOS_STATUS CodechalHwInterfaceG12::InitL3CacheSettings()
{
    // Get default L3 cache settings
    if (m_renderInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->EnableL3Caching(nullptr));
    }
    else
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hwInterfaceNext->GetRenderInterfaceNext()->EnableL3Caching(nullptr));
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    // Override default L3 cache settings
    if (m_renderInterface)
    {
        auto l3CacheConfig = m_renderInterface->GetL3CacheConfig();
        MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12 l3Overrides;
        l3Overrides.dwTcCntlReg =
            static_cast<MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12 *>(l3CacheConfig)->dwL3CacheTcCntlReg_Setting;
        l3Overrides.dwAllocReg =
            static_cast<MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12 *>(l3CacheConfig)->dwL3CacheAllocReg_Setting;
        CODECHAL_HW_CHK_STATUS_RETURN(InitL3ControlUserFeatureSettings(
            l3CacheConfig,
            &l3Overrides));
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->EnableL3Caching(
            &l3Overrides));
    }
    else
    {
        auto                                             l3CacheConfig = m_hwInterfaceNext->GetRenderInterfaceNext()->GetL3CacheConfig();
        mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS l3Overrides;
        l3Overrides.dwTcCntlReg =
            static_cast<mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG *>(l3CacheConfig)->dwL3CacheTcCntlReg_Setting;
        l3Overrides.dwAllocReg =
            static_cast<mhw::render::MHW_RENDER_ENGINE_L3_CACHE_CONFIG *>(l3CacheConfig)->dwL3CacheAllocReg_Setting;
        CODECHAL_HW_CHK_STATUS_RETURN(InitL3ControlUserFeatureSettings(
            (MHW_RENDER_ENGINE_L3_CACHE_CONFIG *)l3CacheConfig,
            (MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12 *)&l3Overrides));
        CODECHAL_HW_CHK_STATUS_RETURN(m_hwInterfaceNext->GetRenderInterfaceNext()->EnableL3Caching(
            &l3Overrides));

    }

#endif // (_DEBUG || _RELEASE_INTERNAL)

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceG12::GetStreamoutCommandSize(
    uint32_t   *commandsSize,
    uint32_t   *patchListSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;

    stateCmdSizeParams.bShortFormat = false;
    stateCmdSizeParams.bHucDummyStream = MEDIA_IS_WA(m_waTable, WaHucStreamoutEnable);
    CODECHAL_HW_CHK_STATUS_RETURN(GetHxxStateCommandSize(
        CODECHAL_DECODE_MODE_CENC,  // For CENC phase
        commandsSize,
        patchListSize,
        &stateCmdSizeParams));

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceG12::SetCacheabilitySettings(
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
    if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_hucInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->SetCacheabilitySettings(cacheabilitySettings));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceG12::SetRowstoreCachingOffsets(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalHwInterface::SetRowstoreCachingOffsets(rowstoreParams));

    if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->GetRowstoreCachingAddrs(rowstoreParams));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceG12::GetAvpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    CODECHAL_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t    avpCommandsSize = 0;
    uint32_t    avpPatchListSize = 0;
    uint32_t    cpCmdsize        = 0;
    uint32_t    cpPatchListSize  = 0;

    if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->GetAvpStateCommandSize(
            (uint32_t *)&avpCommandsSize,
            (uint32_t *)&avpPatchListSize,
            params));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize  = avpCommandsSize  + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceG12::GetAvpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize)
{
    CODECHAL_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t avpCommandsSize = 0;
    uint32_t avpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->GetAvpPrimitiveCommandSize(
            (uint32_t*)&avpCommandsSize,
            (uint32_t*)&avpPatchListSize));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize  = avpCommandsSize  + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CodechalHwInterfaceG12::SendCondBbEndCmd(
    PMOS_RESOURCE              resource,
    uint32_t                   offset,
    uint32_t                   compData,
    bool                       disableCompMask,
    bool                       enableEndCurrentBatchBuffLevel,
    uint32_t                   compareOperation,
    PMOS_COMMAND_BUFFER        cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&m_conditionalBbEndDummy))
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.postSyncOperation = 1;
        flushDwParams.pOsResource       = &m_conditionalBbEndDummy;
        flushDwParams.dwDataDW1         = 0;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

    MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS conditionalBatchBufferEndParams;
    MOS_ZeroMemory(&conditionalBatchBufferEndParams, sizeof(conditionalBatchBufferEndParams));
    conditionalBatchBufferEndParams.presSemaphoreBuffer = resource;
    conditionalBatchBufferEndParams.dwOffset            = offset;
    conditionalBatchBufferEndParams.dwValue             = compData;
    conditionalBatchBufferEndParams.bDisableCompareMask = disableCompMask;
    conditionalBatchBufferEndParams.dwParamsType        = MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS;
    if (enableEndCurrentBatchBuffLevel)
    {
        conditionalBatchBufferEndParams.enableEndCurrentBatchBuffLevel = enableEndCurrentBatchBuffLevel;
        conditionalBatchBufferEndParams.compareOperation               = compareOperation;
    }
    eStatus = m_miInterface->AddMiConditionalBatchBufferEndCmd(cmdBuffer, &conditionalBatchBufferEndParams);

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceG12::Initialize(
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalHwInterface::Initialize(settings));
    // Added isRenderHalNeeded flag into settings 
    // Indicate whether RenderHal is needed or not 
    if (settings->isRenderHalNeeded || 
        (settings->codecFunction == CODECHAL_FUNCTION_DECODE && 
        settings->standard == CODECHAL_AV1))
    {
        //Initialize renderHal
        m_renderHal = (PRENDERHAL_INTERFACE_LEGACY)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE_LEGACY));
        CODECHAL_HW_CHK_NULL_RETURN(m_renderHal);
        CODECHAL_HW_CHK_STATUS_RETURN(RenderHal_InitInterface_Legacy(
            (PRENDERHAL_INTERFACE_LEGACY)m_renderHal,
            &m_renderHalCpInterface,
            m_osInterface));

        RENDERHAL_SETTINGS_LEGACY RenderHalSettings;
        RenderHalSettings.iMediaStates = 32;
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

        //set SSEU table
        m_renderHal->sseuTable = m_ssEuTable;
    }
    return eStatus;
}

MOS_STATUS CodechalHwInterfaceG12::ReadAvpStatus(MHW_VDBOX_NODE_IND vdboxIndex, const EncodeStatusReadParams &params, PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    std::shared_ptr<mhw::mi::Itf> m_miItf = GetMiInterfaceNext();
    if (m_miInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }
    else
    {
        auto &parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush       = {};
        CODECHAL_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));
    }

    std::shared_ptr<mhw::vdbox::avp::Itf> m_avpItf = GetAvpInterfaceNext();
    CODECHAL_HW_CHK_NULL_RETURN(m_avpItf);
    auto mmioRegisters = m_avpItf->GetMmioRegisters(vdboxIndex);
    if (m_miInterface)
    {
        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
        miStoreRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1BitstreamByteCountTileRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = params.resQpStatusCount;
        miStoreRegMemParams.dwOffset        = params.qpStatusCountOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1QpStatusCountRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    }
    else
    {
        auto &storeRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = params.resBitstreamByteCountPerFrame;
        storeRegMemParams.dwOffset        = params.bitstreamByteCountPerFrameOffset;
        storeRegMemParams.dwRegister      = mmioRegisters->avpAv1BitstreamByteCountTileRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = params.resQpStatusCount;
        storeRegMemParams.dwOffset        = params.qpStatusCountOffset;
        storeRegMemParams.dwRegister      = mmioRegisters->avpAv1QpStatusCountRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
    }
    return eStatus;
}

MOS_STATUS CodechalHwInterfaceG12::ReadImageStatusForAvp(MHW_VDBOX_NODE_IND vdboxIndex, const EncodeStatusReadParams &params, PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_HW_CHK_COND_RETURN((vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()),"ERROR - vdbox index exceed the maximum");

    std::shared_ptr<mhw::vdbox::avp::Itf> m_avpItf = GetAvpInterfaceNext();
    CODECHAL_HW_CHK_NULL_RETURN(m_avpItf);
    auto mmioRegisters = GetAvpInterfaceNext()->GetMmioRegisters(vdboxIndex);
    std::shared_ptr<mhw::mi::Itf> m_miItf       = GetMiInterfaceNext();
    if (m_miInterface)
    {
        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = params.resImageStatusMask;
        miStoreRegMemParams.dwOffset        = params.imageStatusMaskOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1ImageStatusMaskRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = params.resImageStatusCtrl;
        miStoreRegMemParams.dwOffset        = params.imageStatusCtrlOffset;
        miStoreRegMemParams.dwRegister      = mmioRegisters->avpAv1ImageStatusControlRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }
    else
    {
        auto &storeRegMemParams           = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = params.resImageStatusMask;
        storeRegMemParams.dwOffset        = params.imageStatusMaskOffset;
        storeRegMemParams.dwRegister      = mmioRegisters->avpAv1ImageStatusMaskRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
        storeRegMemParams                 = {};
        storeRegMemParams.presStoreBuffer = params.resImageStatusCtrl;
        storeRegMemParams.dwOffset        = params.imageStatusCtrlOffset;
        storeRegMemParams.dwRegister      = mmioRegisters->avpAv1ImageStatusControlRegOffset;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(cmdBuffer));
        auto &parFlush = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        parFlush       = {};
        CODECHAL_HW_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));
    }
    return eStatus;
}

CodechalHwInterfaceG12::~CodechalHwInterfaceG12()
{
    if (m_renderHal != nullptr && m_renderHal->pfnDestroy != nullptr)
    {
        MOS_STATUS eStatus = m_renderHal->pfnDestroy(m_renderHal);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
        }

        if (m_renderHalCpInterface)
        {
            if (m_osInterface)
            {
                m_osInterface->pfnDeleteMhwCpInterface(m_renderHalCpInterface);
                m_renderHalCpInterface = nullptr;
            }
            else
            {
                MHW_ASSERTMESSAGE("Failed to destroy renderHalCpInterface.");
            }
        }
    }

    if (m_renderHal != nullptr)
    {
        MOS_FreeMemory(m_renderHal);
        m_renderHal = nullptr;
    }

    if (m_avpInterface)
    {
        MOS_Delete(m_avpInterface);
    }
}

MOS_STATUS CodechalHwInterfaceG12::GetFilmGrainKernelInfo(
    uint8_t*& kernelBase,
    uint32_t& kernelSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    uint8_t* kernelArrayBase = nullptr;

    auto kuidCommon        = IDR_CODEC_VID_ApplyNoise_Combined;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    kernelArrayBase = (uint8_t*)IGCODECKRN_G12;
    CODECHAL_HW_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(kernelArrayBase,
        kuidCommon,
        &kernelBase,
        &kernelSize));
#else
    kernelBase = nullptr;
    kernelSize = 0;
#endif

    return eStatus;
}
