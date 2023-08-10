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
//! \file     decode_vp9_picture_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for vp9 decode picture packet
//!
#include "decode_vp9_picture_packet_xe_lpm_plus_base.h"
#include "decode_vp9_mem_compression_xe_lpm_plus_base.h"
#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "decode_common_feature_defs.h"
#include "codec_hw_xe_lpm_plus_base.h"
#include "mhw_vdbox_hcp_hwcmd_xe_lpm_plus.h"
#include "mhw_mi_hwcmd_xe_lpm_plus_base_next.h"

using namespace mhw::vdbox::xe_lpm_plus_base;

namespace decode
{
    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(Vp9DecodePicPkt::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
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
        SETPAR_AND_ADDCMD(HCP_PIPE_BUF_ADDR_STATE, m_hcpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(HCP_IND_OBJ_BASE_ADDR_STATE, m_hcpItf, &cmdBuffer);
        DECODE_CHK_STATUS(AddAllCmds_HCP_VP9_SEGMENT_STATE(cmdBuffer));
        SETPAR_AND_ADDCMD(HCP_VP9_PIC_STATE, m_hcpItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par                        = {};
        par.initialization         = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::VdScalabPipeLock(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par                              = {};
        par.scalableModePipeLock         = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_MODE_SELECT, Vp9DecodePicPktXe_Lpm_Plus_Base)
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

    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::AddAllCmds_HCP_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer)
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

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, Vp9DecodePicPktXe_Lpm_Plus_Base)
    {
        DECODE_FUNC_CALL();
        params = {};

        DECODE_CHK_STATUS(Vp9DecodePicPkt::MHW_SETPAR_F(HCP_PIPE_BUF_ADDR_STATE)(params));

#ifdef _MMC_SUPPORTED
        Vp9DecodeMemComp *vp9DecodeMemComp = dynamic_cast<Vp9DecodeMemComp *>(m_mmcState);
        DECODE_CHK_NULL(vp9DecodeMemComp);
        DECODE_CHK_STATUS(vp9DecodeMemComp->CheckReferenceList(*m_vp9BasicFeature, params.PostDeblockSurfMmcState, params.PreDeblockSurfMmcState, params.presReferences));
#endif

        if (m_vp9Pipeline->GetDecodeMode() == Vp9Pipeline::virtualTileDecodeMode)
        {
            params.presCABACSyntaxStreamOutBuffer       = &(m_resCABACSyntaxStreamOutBuffer->OsResource);
            params.presIntraPredUpRightColStoreBuffer   = &(m_resIntraPredUpRightColStoreBuffer->OsResource);
            params.presIntraPredLeftReconColStoreBuffer = &(m_resIntraPredLeftReconColStoreBuffer->OsResource);
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_VP9_PIC_STATE, Vp9DecodePicPktXe_Lpm_Plus_Base)
    {
        DECODE_FUNC_CALL();
        params = {};
        params.dWordLength = mhw::vdbox::hcp::xe_lpm_plus_base::v0::Cmd::GetOpLength(12); // VP9_PIC_STATE command is common for both Decoder and Encoder. Decoder uses only 12 DWORDS of the generated 33 DWORDS
        DECODE_CHK_STATUS(Vp9DecodePicPkt::MHW_SETPAR_F(HCP_VP9_PIC_STATE)(params));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::CalculatePictureStateCommandSize()
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE_LPM_PLUS_BASE stateCmdSizeParams;

        stateCmdSizeParams.bHucDummyStream = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *decodeDownSampling =
            dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        stateCmdSizeParams.bSfcInUse = (decodeDownSampling != nullptr);
#endif

        stateCmdSizeParams.bScalableMode = (m_vp9Pipeline->GetDecodeMode() == Vp9Pipeline::virtualTileDecodeMode);

        // Picture Level Commands
        DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceXe_Lpm_Plus_Base *>(m_hwInterface)->GetHcpStateCommandSize(
                m_vp9BasicFeature->m_mode,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp9DecodePicPktXe_Lpm_Plus_Base::ValidateCabacStreamOutSize(MOS_COMMAND_BUFFER &cmdBuffer)
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
