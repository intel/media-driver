/*
* Copyright (c) 2017-2019, Intel Corporation
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
#include "mhw_vdbox_mfx_hwcmd_g11_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "media_interfaces_g12_tgllp.h"

// Currently initialized with dummy values, just as an example. Will be updated later.
const CODECHAL_SSEU_SETTING CodechalHwInterfaceG12::m_defaultSsEuLutG12[CODECHAL_NUM_MEDIA_STATES] =
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
    { 1,        0,        8,         0 }     // CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT
};

CodechalHwInterfaceG12::CodechalHwInterfaceG12(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfaces     *mhwInterfaces)
    : CodechalHwInterface(osInterface, codecFunction, mhwInterfaces)
{
    CODECHAL_HW_FUNCTION_ENTER;

    PLATFORM platform;
    osInterface->pfnGetPlatform(osInterface, &platform);

    InitCacheabilityControlSettings(codecFunction);

    m_isVdencSuperSliceEnabled = true;

    m_ssEuTable = m_defaultSsEuLutG12;

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
            + 3 * mhw_vdbox_hcp_g12_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize
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

MOS_STATUS CodechalHwInterfaceG12::InitL3CacheSettings()
{
    // Get default L3 cache settings
    CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->EnableL3Caching(nullptr));

#if (_DEBUG || _RELEASE_INTERNAL)
    // Override default L3 cache settings
    auto l3CacheConfig =
        m_renderInterface->GetL3CacheConfig();
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12 l3Overrides;
    l3Overrides.dwTcCntlReg =
        static_cast<MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12*>(l3CacheConfig)->dwL3CacheTcCntlReg_Setting;
    l3Overrides.dwAllocReg =
        static_cast<MHW_RENDER_ENGINE_L3_CACHE_CONFIG_G12*>(l3CacheConfig)->dwL3CacheAllocReg_Setting;
    CODECHAL_HW_CHK_STATUS_RETURN(InitL3ControlUserFeatureSettings(
        l3CacheConfig,
        &l3Overrides));
    CODECHAL_HW_CHK_STATUS_RETURN(m_renderInterface->EnableL3Caching(
        &l3Overrides));
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

    return eStatus;
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
