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
//! \file     decode_vp9_picture_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for vp9 decode picture packet
//!
#include "decode_vp9_picture_packet_xe3_lpm_base.h"
#include "decode_vp9_mem_compression_xe3_lpm_base.h"
#include "mhw_vdbox_xe2_lpm_base.h"
#include "decode_common_feature_defs.h"
#include "codec_hw_xe3_lpm_base.h"
#include "mhw_vdbox_hcp_hwcmd_xe3_lpm.h"
#include "mhw_mi_hwcmd_xe3_lpm_base.h"
#include "mhw_vdbox_xe3_lpm_base.h"
#include "mhw_sfc_hwcmd_xe3_lpm_base.h"
#include "decode_resource_auto_lock.h"

using namespace mhw::vdbox::xe3_lpm_base;

namespace decode
{
MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(Vp9DecodePicPkt::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        // Send VD_CONTROL_STATE Pipe Initialization
        DECODE_CHK_STATUS(VdInit(cmdBuffer));

        DECODE_CHK_STATUS(AddAllCmds_HCP_PIPE_MODE_SELECT(cmdBuffer));
        if (IsBackEndPhase())
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

        DECODE_CHK_STATUS(AddAllCmds_HCP_SURFACE_STATE(cmdBuffer));

        bool isMismatch   = m_osInterface->pfnIsMismatchOrderProgrammingSupported();
        bool isInterFrame = (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_INTER_FRAME) &&
                            !m_vp9PicParams->PicFlags.fields.intra_only;

        if (isMismatch && isInterFrame)
        {
            // Ping BB: cur=MvBuf[0], col=MvBuf[1]; executes when stateBuffer==1
            m_batchBufForMvPing = m_pingPongMvBBArray->Fetch();
            DECODE_CHK_NULL(m_batchBufForMvPing);
            {
                ResourceAutoLock resLock(m_allocator, &m_batchBufForMvPing->OsResource);
                uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                DECODE_CHK_NULL(batchBufBase);
                DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBufForMvPing, batchBufBase));
                m_picStateCmdBuffer.cmdBuf1stLvl     = &cmdBuffer;
                m_vp9BasicFeature->m_curMvTempBufIdx = 0;
                m_vp9BasicFeature->m_colMvTempBufIdx = 1;
                DECODE_CHK_STATUS(PackMvBufAddrCmds(m_picStateCmdBuffer, true));
                DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picStateCmdBuffer, nullptr));
            }
            DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBufForMvPing));

            // Pong BB: cur=MvBuf[1], col=MvBuf[0]; executes when stateBuffer==0
            m_batchBufForMvPong = m_pingPongMvBBArray->Fetch();
            DECODE_CHK_NULL(m_batchBufForMvPong);
            {
                ResourceAutoLock resLock(m_allocator, &m_batchBufForMvPong->OsResource);
                uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                DECODE_CHK_NULL(batchBufBase);
                DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBufForMvPong, batchBufBase));
                m_picStateCmdBuffer.cmdBuf1stLvl     = &cmdBuffer;
                m_vp9BasicFeature->m_curMvTempBufIdx = 1;
                m_vp9BasicFeature->m_colMvTempBufIdx = 0;
                DECODE_CHK_STATUS(PackMvBufAddrCmds(m_picStateCmdBuffer, false));
                DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picStateCmdBuffer, nullptr));
            }
            DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBufForMvPong));

            // Two conditional copy BBs propagate the resolved ping/pong state to stateBuffer:
            // BB0: skip-if(nextState==1) -> STORE state=0 (Ping ran)
            // BB1: skip-if(nextState==0) -> STORE state=1 (Pong ran)
            m_batchBufCopyState0 = m_copyStateBBArray->Fetch();
            DECODE_CHK_NULL(m_batchBufCopyState0);
            {
                ResourceAutoLock resLock(m_allocator, &m_batchBufCopyState0->OsResource);
                uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                DECODE_CHK_NULL(batchBufBase);
                DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBufCopyState0, batchBufBase));
                m_picStateCmdBuffer.cmdBuf1stLvl = &cmdBuffer;
                DECODE_CHK_STATUS(BuildCopyStateBB(m_picStateCmdBuffer, 1, 0));
                DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picStateCmdBuffer, nullptr));
            }
            DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBufCopyState0));

            m_batchBufCopyState1 = m_copyStateBBArray->Fetch();
            DECODE_CHK_NULL(m_batchBufCopyState1);
            {
                ResourceAutoLock resLock(m_allocator, &m_batchBufCopyState1->OsResource);
                uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                DECODE_CHK_NULL(batchBufBase);
                DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBufCopyState1, batchBufBase));
                m_picStateCmdBuffer.cmdBuf1stLvl = &cmdBuffer;
                DECODE_CHK_STATUS(BuildCopyStateBB(m_picStateCmdBuffer, 0, 1));
                DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picStateCmdBuffer, nullptr));
            }
            DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBufCopyState1));
        }
        else
        {
            if (isMismatch)
            {
                // GPU-init on every key frame: reset state=1, nextState=0.
                auto &st            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
                st                  = {};
                st.pOsResource      = &m_vp9BasicFeature->m_resVp9MVPingPongStateBuffer->OsResource;
                st.dwResourceOffset = 0;
                st.dwValue          = 1;
                DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

                st                  = {};
                st.pOsResource      = &m_vp9BasicFeature->m_resVp9MVNextStateBuffer->OsResource;
                st.dwResourceOffset = 0;
                st.dwValue          = 0;
                DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));
            }
            SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpItf, &cmdBuffer);
        }

        SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpItf, &cmdBuffer);
        DECODE_CHK_STATUS(AddAllCmds_HCP_VP9_SEGMENT_STATE(cmdBuffer));
        if (!m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            SETPAR_AND_ADDCMD(HCP_VP9_PIC_STATE, m_hcpItf, &cmdBuffer);
        }
        else
        {
            // prepare 2nd level chained BB for two frame types
            {
                m_batchBufferForKeyFrame = m_secondLevelBBArray->Fetch();
                DECODE_CHK_NULL(m_batchBufferForKeyFrame);
                {
                    ResourceAutoLock resLock(m_allocator, &m_batchBufferForKeyFrame->OsResource);
                    uint8_t         *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                    DECODE_CHK_NULL(batchBufBase);
                    DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBufferForKeyFrame, batchBufBase));

                    m_picStateCmdBuffer.cmdBuf1stLvl                        = &cmdBuffer;
                    m_vp9BasicFeature->m_prevFrameParams.fields.VkFrameType = 1;
                    DECODE_CHK_STATUS(PackPicState2ndLevelCmds(m_picStateCmdBuffer, false));
                    DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picStateCmdBuffer, nullptr));
                }
                DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBufferForKeyFrame));
            }

            {
                m_batchBufferForNonKeyFrame = m_secondLevelBBArray->Fetch();
                DECODE_CHK_NULL(m_batchBufferForNonKeyFrame);
                {
                    ResourceAutoLock resLock(m_allocator, &m_batchBufferForNonKeyFrame->OsResource);
                    uint8_t         *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                    DECODE_CHK_NULL(batchBufBase);
                    DECODE_CHK_STATUS(Init2ndLevelCmdBuffer(*m_batchBufferForNonKeyFrame, batchBufBase));

                    m_picStateCmdBuffer.cmdBuf1stLvl                        = &cmdBuffer;
                    m_vp9BasicFeature->m_prevFrameParams.fields.VkFrameType = 0;
                    DECODE_CHK_STATUS(PackPicState2ndLevelCmds(m_picStateCmdBuffer, true));
                    DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picStateCmdBuffer, nullptr));
                }
                DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBufferForNonKeyFrame));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::Init2ndLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase)
    {
        DECODE_FUNC_CALL();

        auto &cmdBuffer = m_picStateCmdBuffer;
        MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));
        cmdBuffer.pCmdBase   = (uint32_t *)batchBufBase;
        cmdBuffer.pCmdPtr    = cmdBuffer.pCmdBase;
        cmdBuffer.iRemaining = batchBuffer.iSize;
        cmdBuffer.OsResource = batchBuffer.OsResource;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::PackPicState2ndLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer, bool bLastIsKeyFrame)
    {
        DECODE_FUNC_CALL();

        CodechalHwInterfaceXe3_Lpm_Base *hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(m_hwInterface);
        DECODE_CHK_NULL(hwInterface);

        uint32_t compareOperation = mhw::mi::xe3_lpm_base::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION::COMPARE_OPERATION_MADEQUALIDD;

        if (bLastIsKeyFrame)
        {
            DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
                &m_vp9BasicFeature->m_resVp9FrameStatusBuffer->OsResource, 0, 1, true, true, compareOperation, &cmdBuffer));
        }
        else
        {
            DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
                &m_vp9BasicFeature->m_resVp9FrameStatusBuffer->OsResource, 0, 0, true, true, compareOperation, &cmdBuffer));
        }
        SETPAR_AND_ADDCMD(HCP_VP9_PIC_STATE, m_hcpItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::PackMvBufAddrCmds(MOS_COMMAND_BUFFER &cmdBuffer, bool isPing)
    {
        DECODE_FUNC_CALL();

        CodechalHwInterfaceXe3_Lpm_Base *hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(m_hwInterface);
        DECODE_CHK_NULL(hwInterface);

        uint32_t compareOperation = mhw::mi::xe3_lpm_base::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION::COMPARE_OPERATION_MADEQUALIDD;

        // Skip this BB if stateBuffer does not select it: Ping skips when stateBuffer==0 (Pong's turn)
        uint32_t skipIfValue = isPing ? 0 : 1;
        DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
            &m_vp9BasicFeature->m_resVp9MVPingPongStateBuffer->OsResource,
            0, skipIfValue, true, true, compareOperation, &cmdBuffer));

        // Emit full HCP_PIPE_BUF_ADDR_STATE with the indices pre-set by Execute()
        SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpItf, &cmdBuffer);

        // Signal which BB ran this frame via nextStateBuffer
        uint32_t nextState           = isPing ? 0 : 1;
        auto &storeParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        storeParams                  = {};
        storeParams.pOsResource      = &m_vp9BasicFeature->m_resVp9MVNextStateBuffer->OsResource;
        storeParams.dwResourceOffset = 0;
        storeParams.dwValue          = nextState;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::BuildCopyStateBB(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t skipIfValue, uint32_t writeValue)
    {
        DECODE_FUNC_CALL();

        CodechalHwInterfaceXe3_Lpm_Base *hwInterface = dynamic_cast<CodechalHwInterfaceXe3_Lpm_Base *>(m_hwInterface);
        DECODE_CHK_NULL(hwInterface);

        uint32_t compareOperation = mhw::mi::xe3_lpm_base::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION::COMPARE_OPERATION_MADEQUALIDD;

        // Read nextStateBuffer; skip this BB if it indicates the wrong Ping/Pong ran
        DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
            &m_vp9BasicFeature->m_resVp9MVNextStateBuffer->OsResource,
            0, skipIfValue, true, true, compareOperation, &cmdBuffer));

        // Propagate the resolved state to stateBuffer for the next frame
        auto &st            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
        st                  = {};
        st.pOsResource      = &m_vp9BasicFeature->m_resVp9MVPingPongStateBuffer->OsResource;
        st.dwResourceOffset = 0;
        st.dwValue          = writeValue;
        DECODE_CHK_STATUS(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par                = {};
        par.initialization = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par       = {};
        par.scalableModePipeLock = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9DecodePicPktXe3_Lpm_Base)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(Vp9DecodePicPkt::MHW_SETPAR_F(HCP_PIPE_MODE_SELECT)(params));

        uint32_t pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
        uint32_t multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        if (m_phase != nullptr)
        {
            m_phase->GetMode(pipeWorkMode, multiEngineMode);
        }
        params.pipeWorkMode    = static_cast<MHW_VDBOX_HCP_PIPE_WORK_MODE>(pipeWorkMode);
        params.multiEngineMode = static_cast<MHW_VDBOX_HCP_MULTI_ENGINE_MODE>(multiEngineMode);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::AddAllCmds_HCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

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

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9DecodePicPktXe3_Lpm_Base)
    {
        DECODE_FUNC_CALL();
        params = {};

        DECODE_CHK_STATUS(Vp9DecodePicPkt::MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params));

        Vp9DecodeMemCompXe3_Lpm_Base *vp9DecodeMemComp = dynamic_cast<Vp9DecodeMemCompXe3_Lpm_Base *>(m_mmcState);
        DECODE_CHK_NULL(vp9DecodeMemComp);
        DECODE_CHK_STATUS(vp9DecodeMemComp->CheckReferenceList(*m_vp9BasicFeature, params.PostDeblockSurfMmcState, params.PreDeblockSurfMmcState, params.presReferences));

        if (m_vp9Pipeline->GetDecodeMode() == Vp9Pipeline::virtualTileDecodeMode)
        {
            params.presCABACSyntaxStreamOutBuffer       = &(m_resCABACSyntaxStreamOutBuffer->OsResource);
            params.presIntraPredUpRightColStoreBuffer   = &(m_resIntraPredUpRightColStoreBuffer->OsResource);
            params.presIntraPredLeftReconColStoreBuffer = &(m_resIntraPredLeftReconColStoreBuffer->OsResource);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_VP9_PIC_STATE, Vp9DecodePicPktXe3_Lpm_Base)
    {
        DECODE_FUNC_CALL();
        params             = {};
        params.dWordLength = mhw::vdbox::hcp::xe3_lpm_base::xe3_lpm::Cmd::GetOpLength(12);  // VP9_PIC_STATE command is common for both Decoder and Encoder. Decoder uses only 12 DWORDS of the generated 33 DWORDS
        DECODE_CHK_STATUS(Vp9DecodePicPkt::MHW_SETPAR_F(HCP_VP9_PIC_STATE)(params));

        return MOS_STATUS_SUCCESS;
    }
    
    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::GetVp9StateCommandSize(
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
            m_hcpItf->GETSIZE_HCP_VP9_SEGMENT_STATE() * 8 +
            m_hcpItf->GETSIZE_HCP_BSD_OBJECT() +
            m_miItf->GETSIZE_MI_LOAD_REGISTER_REG() * 8 +
            m_hcpItf->GETSIZE_HCP_VP9_PIC_STATE() +
            m_miItf->GETSIZE_VD_CONTROL_STATE() * 2;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_SURFACE_STATE_CMD) * 4 +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_VP9_SEGMENT_STATE_CMD) * 8 +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_BSD_OBJECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::hcp::Itf::HCP_VP9_PIC_STATE_CMD);

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

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::CalculatePictureStateCommandSize()
    {
        mhw::vdbox::xe3_lpm_base::MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE stateCmdSizeParams;
        uint32_t                                                              cpCmdsize = 0;
        uint32_t                                                              cpPatchListSize = 0;
        stateCmdSizeParams.bHucDummyStream = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *decodeDownSampling =
            dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        stateCmdSizeParams.bSfcInUse = (decodeDownSampling != nullptr);
#endif
        stateCmdSizeParams.bScalableMode = (m_vp9Pipeline->GetDecodeMode() == Vp9Pipeline::virtualTileDecodeMode);

        // Picture Level Commands
        DECODE_CHK_STATUS(GetVp9StateCommandSize(
                m_vp9BasicFeature->m_mode,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                &stateCmdSizeParams));

        m_hwInterface->GetCpInterface()->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);

        m_pictureStatesSize += cpCmdsize;
        m_picturePatchListSize += cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe3_Lpm_Base::ValidateCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
    {
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
