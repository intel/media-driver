/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     codec_hw_xe_lpm_plus_base.cpp
//! \brief    Implements HW interface layer for Xe_LPM_plus+ used on all OSs.
//! \details  Implements HW interface layer for CNL+ to be used on on all operating systems/DDIs, across CODECHAL components.
//!           This module must not contain any OS dependent code.
//!

#include "codec_hw_xe_lpm_plus_base.h"
#include "mhw_vdbox_xe_lpm_plus_base.h"

// Currently initialized with dummy values, just as an example. Will be updated later.
const CODECHAL_SSEU_SETTING CodechalHwInterfaceXe_Lpm_Plus_Base::m_defaultSsEuLutXeLpmPlus[CODECHAL_NUM_MEDIA_STATES_XE_LPM_PLUS_BASE] =
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

CodechalHwInterfaceXe_Lpm_Plus_Base::CodechalHwInterfaceXe_Lpm_Plus_Base(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfacesNext *mhwInterfacesNext,
    bool              disableScalability)
    : CodechalHwInterfaceNext(osInterface, codecFunction, mhwInterfacesNext, disableScalability)
{
    CODEC_HW_FUNCTION_ENTER;

    m_mediaSfcItf     = std::make_shared<MediaSfcInterface>(m_osInterface);

    InitCacheabilityControlSettings(codecFunction);

    m_isVdencSuperSliceEnabled = true;

    m_ssEuTable      = m_defaultSsEuLutXeLpmPlus;
    m_numMediaStates = CODECHAL_NUM_MEDIA_STATES_XE_LPM_PLUS_BASE;

    // Set platform dependent parameters
    m_sizeOfCmdBatchBufferEnd = m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
    m_sizeOfCmdMediaReset     = m_miItf->MHW_GETSIZE_F(MI_LOAD_REGISTER_IMM)() * 8;
    m_vdencBrcImgStateBufferSize = m_vdencItf->MHW_GETSIZE_F(VDENC_AVC_IMG_STATE)()
        + m_mfxItf->MHW_GETSIZE_F(MFX_AVC_IMG_STATE)()
        + m_vdencItf->MHW_GETSIZE_F(VDENC_CMD3)()
        + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    m_vdencBatchBuffer1stGroupSize = m_hcpItf->MHW_GETSIZE_F(HCP_PIPE_MODE_SELECT)()
                                     + m_miItf->MHW_GETSIZE_F(MFX_WAIT)() * 2
                                     + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    m_vdencBatchBuffer2ndGroupSize = m_vdencItf->MHW_GETSIZE_F(VDENC_CMD1)()
                                     + m_hcpItf->MHW_GETSIZE_F(HCP_PIC_STATE)()
                                     + m_vdencItf->MHW_GETSIZE_F(VDENC_CMD2)()
                                     + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    m_vdencReadBatchBufferSize =
    m_vdenc2ndLevelBatchBufferSize = m_vdencBatchBuffer1stGroupSize
                                     + m_vdencBatchBuffer2ndGroupSize
                                     + ENCODE_HEVC_VDENC_NUM_MAX_SLICES * (2 * m_hcpItf->MHW_GETSIZE_F(HCP_WEIGHTOFFSET_STATE)()
                                     + m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)()
                                     + (HEVC_MAX_NAL_UNIT_TYPE + 2) * m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)()
                                     + m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)()
                                     + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)()
                                     + 4 * ENCODE_VDENC_HEVC_PADDING_DW_SIZE);

    m_HucStitchCmdBatchBufferSize = 7 * 4 
                                    + 14 * 4 
                                    + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();
                                    
    // HCP_WEIGHTOFFSET_STATE_CMD cmds is planned to be added in near future
    m_vdencBatchBufferPerSliceConstSize = m_hcpItf->MHW_GETSIZE_F(HCP_SLICE_STATE)()
        + m_hcpItf->MHW_GETSIZE_F(HCP_PAK_INSERT_OBJECT)() // 1st PakInsertObject cmd is not always inserted for each slice, 2nd PakInsertObject cmd is always inserted for each slice
        + m_vdencItf->MHW_GETSIZE_F(VDENC_WEIGHTSOFFSETS_STATE)()
        + m_miItf->MHW_GETSIZE_F(MI_BATCH_BUFFER_END)();

    // No kernel used on xe lpm plus
    m_maxKernelLoadCmdSize = 0;

    m_sizeOfCmdMediaObject = 0;
    m_sizeOfCmdMediaStateFlush = 0;
}

MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::SetRowstoreCachingOffsets(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    CODEC_HW_CHK_STATUS_RETURN(CodechalHwInterfaceNext::SetRowstoreCachingOffsets(rowstoreParams));
    if (GetMfxInterfaceNext())
    {
        CODEC_HW_CHK_STATUS_RETURN(GetMfxInterfaceNext()->GetRowstoreCachingAddrs(rowstoreParams));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::GetAvpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    CODEC_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t    avpCommandsSize = 0;
    uint32_t    avpPatchListSize = 0;
    uint32_t    cpCmdsize        = 0;
    uint32_t    cpPatchListSize  = 0;

    if (GetAvpInterfaceNext())
    {
        CODEC_HW_CHK_STATUS_RETURN(GetAvpInterfaceNext()->GetAvpStateCmdSize(
            (uint32_t *)&avpCommandsSize,
            (uint32_t *)&avpPatchListSize,
            params));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize = avpCommandsSize + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::GetAvpPrimitiveCommandSize(
    uint32_t                    mode,
    uint32_t                    *commandsSize,
    uint32_t                    *patchListSize)
{
    CODEC_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t avpCommandsSize = 0;
    uint32_t avpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (GetAvpInterfaceNext())
    {
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        CODEC_HW_CHK_STATUS_RETURN(GetAvpInterfaceNext()->GetAvpPrimitiveCmdSize(
            (uint32_t*)&avpCommandsSize,
            (uint32_t*)&avpPatchListSize,
            &stateCmdSizeParams));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize = avpCommandsSize + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::GetHcpStateCommandSize(
    uint32_t                        mode,
    uint32_t *                      commandsSize,
    uint32_t *                      patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize  = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hcpItf && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODEC_HW_CHK_STATUS_RETURN(m_hcpItf->GetHcpStateCommandSize(
            mode, &hcpCommandsSize, &hcpPatchListSize, params));

        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hcpCommandsSize + cpCmdsize;
    *patchListSize = hcpPatchListSize + cpPatchListSize;

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::GetHcpPrimitiveCommandSize(
    uint32_t  mode,
    uint32_t *commandsSize,
    uint32_t *patchListSize,
    bool      modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    uint32_t hcpCommandsSize  = 0;
    uint32_t hcpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;

    if (m_hcpItf && (standard == CODECHAL_HEVC || standard == CODECHAL_VP9))
    {
        CODEC_HW_CHK_STATUS_RETURN(m_hcpItf->GetHcpPrimitiveCommandSize(
            mode, &hcpCommandsSize, &hcpPatchListSize, modeSpecific ? true : false));

        CODEC_HW_CHK_NULL_RETURN(m_cpInterface);
        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    *commandsSize  = hcpCommandsSize + cpCmdsize;
    *patchListSize = hcpPatchListSize + cpPatchListSize;

    return eStatus;
}


MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::SendCondBbEndCmd(
    PMOS_RESOURCE              resource,
    uint32_t                   offset,
    uint32_t                   compData,
    bool                       disableCompMask,
    bool                       enableEndCurrentBatchBuffLevel,
    uint32_t                   compareOperation,
    PMOS_COMMAND_BUFFER        cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&m_conditionalBbEndDummy))
    {
        auto &par = GetMiInterfaceNext()->GETPAR_MI_FLUSH_DW();
        par                             = {};
        par.postSyncOperation           = 1;
        par.pOsResource                 = &m_conditionalBbEndDummy;
        par.dwDataDW1                   = 0;
        CODEC_HW_CHK_STATUS_RETURN(GetMiInterfaceNext()->ADDCMD_MI_FLUSH_DW(cmdBuffer));
    }

    auto &par = GetMiInterfaceNext()->GETPAR_MI_CONDITIONAL_BATCH_BUFFER_END();
    par       = {};
    par.presSemaphoreBuffer                             = resource;
    par.dwOffset                                        = offset;
    par.dwValue                                         = compData;
    par.bDisableCompareMask                             = disableCompMask;
    par.dwParamsType                                    = mhw::mi::MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS;
    if (enableEndCurrentBatchBuffLevel)
    {
        par.enableEndCurrentBatchBuffLevel               = enableEndCurrentBatchBuffLevel;
        par.compareOperation                             = compareOperation;
    }
    eStatus = GetMiInterfaceNext()->ADDCMD_MI_CONDITIONAL_BATCH_BUFFER_END(cmdBuffer);

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceXe_Lpm_Plus_Base::Initialize(
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODEC_HW_FUNCTION_ENTER;

    CODEC_HW_CHK_STATUS_RETURN(CodechalHwInterfaceNext::Initialize(settings));

    //Initialize renderHal
    m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE));
    CODEC_HW_CHK_NULL_RETURN(m_renderHal);
    CODEC_HW_CHK_STATUS_RETURN(RenderHal_InitInterface(
        m_renderHal,
        &m_renderHalCpInterface,
        m_osInterface));

    RENDERHAL_SETTINGS RenderHalSettings;
    RenderHalSettings.iMediaStates = 32;
    CODEC_HW_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    //set SSEU table
    m_renderHal->sseuTable = m_ssEuTable;

    return eStatus;
}

CodechalHwInterfaceXe_Lpm_Plus_Base::~CodechalHwInterfaceXe_Lpm_Plus_Base()
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
            MOS_Delete(m_renderHalCpInterface);
            m_renderHalCpInterface = nullptr;
        }

        MOS_FreeMemory(m_renderHal);
        m_renderHal = nullptr;
    }
}
