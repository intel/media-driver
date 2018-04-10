/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      codechal_hw_g10_X.cpp 
//! \brief         This modules implements HW interface layer for CNL+ to be used on on all operating systems/DDIs, across CODECHAL components. 
//!
#include "codechal_hw_g10_X.h"
#include "mhw_state_heap_g10.h"
#include "mhw_render_hwcmd_g10_X.h"
#include "mhw_vdbox_mfx_hwcmd_g10_X.h"
#include "mhw_mi_hwcmd_g10_X.h"
#include "mhw_vdbox_hcp_hwcmd_g10_X.h"  // temporary include for calculating size of various hardware commands
#include "mhw_vdbox_vdenc_hwcmd_g10_X.h"  // temporary include for calculating size of various hardware commands

// Currently initialized with dummy values, just as an example. Will be updated later.
const CODECHAL_SSEU_SETTING CodechalHwInterfaceG10::m_defaultSsEuLutG10[CODECHAL_NUM_MEDIA_STATES] =
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

CodechalHwInterfaceG10::CodechalHwInterfaceG10(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfaces *mhwInterfaces)
    : CodechalHwInterface(osInterface, codecFunction, mhwInterfaces)
{
    CODECHAL_HW_FUNCTION_ENTER;

    m_checkBankCount = true;
    InitCacheabilityControlSettings(codecFunction);

    m_isVdencSuperSliceEnabled = true;

    if (osInterface->bEnableVdboxBalancing)
    {
       bEnableVdboxBalancingbyUMD = true;
       // Enabled VDbox Balancing, the HuC will be disabled.
       m_noHuC = true;
    }

    m_ssEuTable = m_defaultSsEuLutG10;

    // Set platform dependent parameters
    m_sizeOfCmdBatchBufferEnd = mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_sizeOfCmdMediaReset = mhw_mi_g10_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 8;
    m_vdencBrcImgStateBufferSize = mhw_vdbox_vdenc_g10_X::VDENC_IMG_STATE_CMD::byteSize + mhw_vdbox_mfx_g10_X::MFX_AVC_IMG_STATE_CMD::byteSize
        + mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_vdencBatchBuffer1stGroupSize = mhw_vdbox_hcp_g10_X::HCP_PIPE_MODE_SELECT_CMD::byteSize
        + mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_vdencBatchBuffer2ndGroupSize = 120
        + mhw_vdbox_hcp_g10_X::HCP_PIC_STATE_CMD::byteSize
        + 148
        + mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_vdencReadBatchBufferSize =
        m_vdenc2ndLevelBatchBufferSize = m_vdencBatchBuffer1stGroupSize
        + m_vdencBatchBuffer2ndGroupSize
        + ENCODE_HEVC_VDENC_NUM_MAX_SLICES
        * (2 * mhw_vdbox_hcp_g10_X::HCP_WEIGHTOFFSET_STATE_CMD::byteSize
            + mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD::byteSize
            + 3 * mhw_vdbox_hcp_g10_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize
            + mhw_vdbox_vdenc_g10_X::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize
            + mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize);

    // mhw_vdbox_hcp_g10_X::HCP_WEIGHTOFFSET_STATE_CMD cmds is planned to be added in near future
    m_vdencBatchBufferPerSliceConstSize = mhw_vdbox_hcp_g10_X::HCP_SLICE_STATE_CMD::byteSize
        + mhw_vdbox_hcp_g10_X::HCP_PAK_INSERT_OBJECT_CMD::byteSize         // 1st PakInsertObject cmd is not always inserted for each slice, 2nd PakInsertObject cmd is always inserted for each slice
        + mhw_vdbox_vdenc_g10_X::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize
        + mhw_mi_g10_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_maxKernelLoadCmdSize =
        mhw_mi_g10_X::PIPE_CONTROL_CMD::byteSize +
        mhw_render_g10_X::PIPELINE_SELECT_CMD::byteSize +
        mhw_render_g10_X::MEDIA_OBJECT_CMD::byteSize +
        mhw_render_g10_X::STATE_BASE_ADDRESS_CMD::byteSize +
        mhw_render_g10_X::MEDIA_VFE_STATE_CMD::byteSize +
        mhw_render_g10_X::MEDIA_CURBE_LOAD_CMD::byteSize +
        mhw_render_g10_X::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD::byteSize +
        mhw_mi_g10_X::MI_BATCH_BUFFER_START_CMD::byteSize +
        mhw_render_g10_X::MEDIA_OBJECT_WALKER_CMD::byteSize +
        mhw_mi_g10_X::MI_STORE_DATA_IMM_CMD::byteSize;

    m_sizeOfCmdMediaObject = mhw_render_g10_X::MEDIA_OBJECT_CMD::byteSize;
    m_sizeOfCmdMediaStateFlush = mhw_mi_g10_X::MEDIA_STATE_FLUSH_CMD::byteSize;
}
