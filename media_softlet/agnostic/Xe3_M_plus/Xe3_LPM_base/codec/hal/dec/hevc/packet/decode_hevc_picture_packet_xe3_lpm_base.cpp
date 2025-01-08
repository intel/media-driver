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
//! \file     decode_hevc_picture_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for hevc decode picture packet
//!
#include "decode_hevc_picture_packet.h"
#include "decode_hevc_picture_packet_xe3_lpm_base.h"
#include "decode_hevc_mem_compression_xe3_lpm_base.h"
#include "mhw_vdbox_xe3_lpm_base.h"
#include "codec_hw_xe3_lpm_base.h"
#include "decode_common_feature_defs.h"
#include "mhw_mi_hwcmd_xe3_lpm_base.h"
#include "mhw_sfc_hwcmd_xe3_lpm_base.h"

using namespace mhw::vdbox::xe3_lpm_base;

namespace decode
{

HevcDecodePicPktXe3_Lpm_Base::~HevcDecodePicPktXe3_Lpm_Base()
{
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(HevcDecodePicPkt::Init());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
{
    // Send VD_CONTROL_STATE Pipe Initialization
    DECODE_CHK_STATUS(VdInit(cmdBuffer));

    DECODE_CHK_STATUS(AddAllCmds_HCP_PIPE_MODE_SELECT(cmdBuffer));

    if (IsRealTilePhase() || IsBackEndPhase())
    {
        VdScalabPipeLock(cmdBuffer);
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr &&
        m_downSamplingFeature->IsEnabled())
    {
        if (!IsFrontEndPhase())
        {
            DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
        }
    }
#endif

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PIPE_BUF_ADDR_STATE)();
    params       = {};
    HevcDecodePicPktXe3_Lpm_Base::MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params);
    DECODE_CHK_STATUS(AddAllCmds_HCP_SURFACE_STATE(cmdBuffer));
    DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_PIPE_BUF_ADDR_STATE)(&cmdBuffer));
    SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpItf, &cmdBuffer);
    DECODE_CHK_STATUS(AddAllCmds_HCP_QM_STATE(cmdBuffer));
    SETPAR_AND_ADDCMD(HCP_PIC_STATE, m_hcpItf, &cmdBuffer);

    if (m_hevcPicParams->tiles_enabled_flag == 1)
    {
        SETPAR_AND_ADDCMD(HCP_TILE_STATE, m_hcpItf, &cmdBuffer);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
{
    auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
    par       = {};
    par.initialization = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer)
{
    auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
    par       = {};
    par.scalableModePipeLock = true;

    DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcDecodePicPktXe3_Lpm_Base)
{
    DECODE_FUNC_CALL();

    params = {};
    HevcDecodePicPkt::MHW_SETPAR_F(HCP_PIPE_MODE_SELECT)(params);

    uint32_t pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    uint32_t multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;

    if (m_phase != nullptr)
    {
        m_phase->GetMode(pipeWorkMode, multiEngineMode);
    }
    params.pipeWorkMode    = static_cast<MHW_VDBOX_HCP_PIPE_WORK_MODE>(pipeWorkMode);
    params.multiEngineMode = static_cast<MHW_VDBOX_HCP_MULTI_ENGINE_MODE>(multiEngineMode);

    auto decodeMode = m_hevcPipeline->GetDecodeMode();
    if (decodeMode == HevcPipeline::realTileDecodeMode)
    {
        if (m_hevcPipeline->IsFirstPass())
        {
            params.ucPhaseIndicator = MHW_VDBOX_HCP_RT_FIRST_PHASE;
        }
        else if (m_hevcPipeline->IsLastPass())
        {
            params.ucPhaseIndicator = MHW_VDBOX_HCP_RT_LAST_PHASE;
        }
        else
        {
            params.ucPhaseIndicator = MHW_VDBOX_HCP_RT_MIDDLE_PHASE;
        }
    }

    params.bHEVCSeparateTileProgramming =
        (m_hevcPipeline->GetDecodeMode() == HevcPipeline::separateTileDecodeMode);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::AddAllCmds_HCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_hcpItf);

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_PIPE_MODE_SELECT)();
    params       = {};

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after HCP Pipemode select...
    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    SETPAR_AND_ADDCMD(HCP_PIPE_MODE_SELECT, m_hcpItf, &cmdBuffer);

    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcDecodePicPktXe3_Lpm_Base)
{
    DECODE_FUNC_CALL();

    params = {};
    HevcDecodePicPkt::MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params);

#ifdef _MMC_SUPPORTED
    HevcDecodeMemCompXe3_Lpm_Base *hevcDecodeMemComp = dynamic_cast<HevcDecodeMemCompXe3_Lpm_Base *>(m_mmcState);
    DECODE_CHK_NULL(hevcDecodeMemComp);
    DECODE_CHK_STATUS(hevcDecodeMemComp->CheckReferenceList(*m_hevcBasicFeature, params.PostDeblockSurfMmcState, params.PreDeblockSurfMmcState, params.presReferences));
#endif

    auto decodeMode = m_hevcPipeline->GetDecodeMode();
    if (decodeMode == HevcPipeline::virtualTileDecodeMode ||
        decodeMode == HevcPipeline::realTileDecodeMode)
    {
        params.presSliceStateStreamOutBuffer        = &(m_resSliceStateStreamOutBuffer->OsResource);
        params.presMvUpRightColStoreBuffer          = &(m_resMvUpRightColStoreBuffer->OsResource);
        params.presIntraPredUpRightColStoreBuffer   = &(m_resIntraPredUpRightColStoreBuffer->OsResource);
        params.presIntraPredLeftReconColStoreBuffer = &(m_resIntraPredLeftReconColStoreBuffer->OsResource);
        params.presCABACSyntaxStreamOutBuffer       = &(m_resCABACSyntaxStreamOutBuffer->OsResource);
    }
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcDecodePicPktXe3_Lpm_Base)
{
    DECODE_FUNC_CALL();

    params = {};
    HevcDecodePicPkt::MHW_SETPAR_F(HCP_PIC_STATE)(params);  // Call into common part, then add command in the end

    params.pHevcExtPicParams                       = m_hevcRextPicParams;
    params.pHevcSccPicParams                       = m_hevcSccPicParams;
    params.ibcMotionCompensationBufferReferenceIdc = m_hevcBasicFeature->m_refFrames.m_IBCRefIdx;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::GetHevcStateCommandSize(
    uint32_t                        mode,
    uint32_t                       *commandsSize,
    uint32_t                       *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t maxSize          = 0;
    uint32_t patchListMaxSize = 0;
    MHW_CHK_NULL_RETURN(params);
    auto par = dynamic_cast<mhw::vdbox::xe3_lpm_base::PMHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE>(params);
    MHW_CHK_NULL_RETURN(par);
    maxSize =
        m_vdencItf->GETSIZE_VD_PIPELINE_FLUSH() +
        m_miItf->GETSIZE_MI_FLUSH_DW() +
        m_hcpItf->GETSIZE_HCP_PIPE_MODE_SELECT() +
        m_hcpItf->GETSIZE_HCP_SURFACE_STATE() +
        m_hcpItf->GETSIZE_HCP_PIPE_BUF_ADDR_STATE() +
        m_hcpItf->GETSIZE_HCP_IND_OBJ_BASE_ADDR_STATE() +
        m_miItf->GETSIZE_MI_LOAD_REGISTER_REG() * 8 +
        m_miItf->GETSIZE_VD_CONTROL_STATE() * 2 +
        m_hcpItf->GETSIZE_HCP_QM_STATE() * 20 +
        m_hcpItf->GETSIZE_HCP_PIC_STATE() +
        m_hcpItf->GETSIZE_HCP_TILE_STATE();

    patchListMaxSize =
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::VD_PIPELINE_FLUSH_CMD) +
        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
        20 * PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_QM_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIC_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_TILE_STATE_CMD);

    if (params->bSfcInUse)
    {
        maxSize +=
            mhw::sfc::xe3_lpm_base::Cmd::SFC_LOCK_CMD::byteSize +
            m_miItf->GETSIZE_VD_CONTROL_STATE() * 2 +
            mhw::sfc::xe3_lpm_base::Cmd::SFC_AVS_STATE_CMD::byteSize +
            mhw::sfc::xe3_lpm_base::Cmd::SFC_AVS_LUMA_Coeff_Table_CMD::byteSize +
            mhw::sfc::xe3_lpm_base::Cmd::SFC_AVS_CHROMA_Coeff_Table_CMD::byteSize +
            mhw::sfc::xe3_lpm_base::Cmd::SFC_IEF_STATE_CMD::byteSize +
            mhw::sfc::xe3_lpm_base::Cmd::SFC_FRAME_START_CMD::byteSize;

        patchListMaxSize +=
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_STATE_CMD_NUMBER_OF_ADDRESSES +
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_CHROMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_LUMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_STATE_CMD_NUMBER_OF_ADDRESSES +
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_FRAME_START_CMD_NUMBER_OF_ADDRESSES +
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_IEF_STATE_CMD_NUMBER_OF_ADDRESSES +
            mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_LOCK_CMD_NUMBER_OF_ADDRESSES;
    }

    if (par->bScalableMode)
    {
        // VD_CONTROL_STATE Hcp lock and unlock
        maxSize += 2 * mhw::mi::xe3_lpm_base::Cmd::VD_CONTROL_STATE_CMD::byteSize;

        // Due to the fact that there is no slice level command in BE status, we mainly consider commands in FE.
        maxSize +=
            4 * mhw::mi::xe3_lpm_base::Cmd::MI_ATOMIC_CMD::byteSize +                        // used to reset semaphore in BEs
            2 * mhw::mi::xe3_lpm_base::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +  // 1 Conditional BB END for FE hang, 1 for streamout buffer writing over allocated size
            3 * mhw::mi::xe3_lpm_base::Cmd::MI_SEMAPHORE_WAIT_CMD::byteSize +                // for FE & BE0, BEs sync
            15 * mhw::mi::xe3_lpm_base::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +               // for placeholder cmds to resolve the hazard between BEs sync
            3 * mhw::mi::xe3_lpm_base::Cmd::MI_STORE_DATA_IMM_CMD::byteSize +                // for FE status set and clear
            3 * mhw::mi::xe3_lpm_base::Cmd::MI_LOAD_REGISTER_IMM_CMD::byteSize +             // for FE status set
            2 * mhw::mi::xe3_lpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize +                      // 2 needed for command flush in slice level
            2 * mhw::mi::xe3_lpm_base::Cmd::MI_STORE_REGISTER_MEM_CMD::byteSize +            // store the carry flag of reported size in FE
            4 * sizeof(MHW_MI_ALU_PARAMS) +                                                  // 4 ALU commands needed for substract opertaion in FE
            mhw::mi::xe3_lpm_base::Cmd::MI_MATH_CMD::byteSize +                              // 1 needed for FE status set
            mhw::mi::xe3_lpm_base::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;                  // 1 needed for FE status set
        mhw::mi::xe3_lpm_base::Cmd::MI_MATH_CMD::byteSize +                                  // 1 needed for FE status set
            mhw::mi::xe3_lpm_base::Cmd::MI_LOAD_REGISTER_REG_CMD::byteSize;                  // 1 needed for FE status set

        patchListMaxSize +=
            4 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_ATOMIC_CMD) +
            2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
            3 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_SEMAPHORE_WAIT_CMD) +
            18 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD) +
            2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD);

        if (params->bSfcInUse)
        {
            maxSize +=
                mhw::sfc::xe3_lpm_base::Cmd::SFC_LOCK_CMD::byteSize +
                m_miItf->GETSIZE_VD_CONTROL_STATE() * 2 +
                mhw::sfc::xe3_lpm_base::Cmd::SFC_STATE_CMD::byteSize +
                mhw::sfc::xe3_lpm_base::Cmd::SFC_AVS_STATE_CMD::byteSize +
                mhw::sfc::xe3_lpm_base::Cmd::SFC_AVS_LUMA_Coeff_Table_CMD::byteSize +
                mhw::sfc::xe3_lpm_base::Cmd::SFC_AVS_CHROMA_Coeff_Table_CMD::byteSize +
                mhw::sfc::xe3_lpm_base::Cmd::SFC_IEF_STATE_CMD::byteSize +
                mhw::sfc::xe3_lpm_base::Cmd::SFC_FRAME_START_CMD::byteSize;

            patchListMaxSize +=
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_STATE_CMD_NUMBER_OF_ADDRESSES +
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_CHROMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_LUMA_Coeff_Table_CMD_NUMBER_OF_ADDRESSES +
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_AVS_STATE_CMD_NUMBER_OF_ADDRESSES +
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_FRAME_START_CMD_NUMBER_OF_ADDRESSES +
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_IEF_STATE_CMD_NUMBER_OF_ADDRESSES +
                mhw::sfc::Itf::CommandsNumberOfAddresses::SFC_LOCK_CMD_NUMBER_OF_ADDRESSES;
        }
    }
    *commandsSize  = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE stateCmdSizeParams;
    uint32_t                                    cpCmdsize  = 0;
    uint32_t                                    cpPatchListSize = 0;
    auto decodeMode = m_hevcPipeline->GetDecodeMode();
    if (decodeMode == HevcPipeline::virtualTileDecodeMode ||
        decodeMode == HevcPipeline::realTileDecodeMode)
    {
        stateCmdSizeParams.bScalableMode = true;
    }

#ifdef _DECODE_PROCESSING_SUPPORTED
    DecodeDownSamplingFeature *decodeDownSampling =
        dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
    stateCmdSizeParams.bSfcInUse = (decodeDownSampling != nullptr);
#endif

    // Picture Level Commands
    DECODE_CHK_STATUS(GetHevcStateCommandSize(
        m_hevcBasicFeature->m_mode,
        &commandBufferSize,
        &requestedPatchListSize,
        &stateCmdSizeParams));

    m_hwInterface->GetCpInterface()->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);

    commandBufferSize += cpCmdsize;
    requestedPatchListSize += cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodePicPktXe3_Lpm_Base::ValidateCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);
    DECODE_CHK_NULL(m_resCABACSyntaxStreamOutBuffer);

    CodechalHwInterfaceXe3_Lpm_Base *hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(m_hwInterface);
    DECODE_CHK_NULL(hwInterface);

    uint32_t compareOperation = mhw::mi::xe3_lpm_base::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION::COMPARE_OPERATION_MADLESSTHANIDD;
    DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
        &m_resCABACStreamOutSizeBuffer->OsResource,
        0,
        m_resCABACSyntaxStreamOutBuffer->size,
        true,
        true,
        compareOperation,
        &cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

}
