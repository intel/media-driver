/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_hevc_picture_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for hevc decode picture packet
//!
#include "decode_hevc_picture_packet.h"
#include "decode_hevc_picture_packet_xe_lpm_plus_base.h"
#include "decode_hevc_mem_compression_xe_lpm_plus_base.h"
#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "codec_hw_xe_lpm_plus_base.h"
#include "decode_common_feature_defs.h"
#include "mhw_mi_hwcmd_xe_lpm_plus_base_next.h"

using namespace mhw::vdbox::xe_lpm_plus_base;

namespace decode
{

    HevcDecodePicPktXe_Lpm_Plus_Base::~HevcDecodePicPktXe_Lpm_Plus_Base()
    {
    }

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(HevcDecodePicPkt::Init());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
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
        DECODE_CHK_STATUS(HevcDecodePicPktXe_Lpm_Plus_Base::MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params));
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

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par       = {};
        par.initialization = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par       = {};
        par.scalableModePipeLock = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, HevcDecodePicPktXe_Lpm_Plus_Base)
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

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::AddAllCmds_HCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
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

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcDecodePicPktXe_Lpm_Plus_Base)
    {
        DECODE_FUNC_CALL();

        params = {};
        DECODE_CHK_STATUS(HevcDecodePicPkt::MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params));

#ifdef _MMC_SUPPORTED
        HevcDecodeMemComp *hevcDecodeMemComp = dynamic_cast<HevcDecodeMemComp *>(m_mmcState);
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

    MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcDecodePicPktXe_Lpm_Plus_Base)
    {
        DECODE_FUNC_CALL();

        params = {};
        HevcDecodePicPkt::MHW_SETPAR_F(HCP_PIC_STATE)(params);  // Call into common part, then add command in the end

        params.pHevcExtPicParams                       = m_hevcRextPicParams;
        params.pHevcSccPicParams                       = m_hevcSccPicParams;
        params.ibcMotionCompensationBufferReferenceIdc = m_hevcBasicFeature->m_refFrames.m_IBCRefIdx;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE_LPM_PLUS_BASE stateCmdSizeParams;

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
        DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceXe_Lpm_Plus_Base *>(m_hwInterface)->GetHcpStateCommandSize(
            m_hevcBasicFeature->m_mode,
            &commandBufferSize,
            &requestedPatchListSize,
            &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodePicPktXe_Lpm_Plus_Base::ValidateCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_resCABACStreamOutSizeBuffer);
        DECODE_CHK_NULL(m_resCABACSyntaxStreamOutBuffer);

        CodechalHwInterfaceXe_Lpm_Plus_Base *hwInterface = dynamic_cast<CodechalHwInterfaceXe_Lpm_Plus_Base *>(m_hwInterface);
        DECODE_CHK_NULL(hwInterface);

        uint32_t compareOperation = mhw::mi::xe_lpm_plus_base_next::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION::COMPARE_OPERATION_MADLESSTHANIDD;
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
