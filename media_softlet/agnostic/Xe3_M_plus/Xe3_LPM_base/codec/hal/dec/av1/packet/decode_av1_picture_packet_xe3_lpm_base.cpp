/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     decode_av1_picture_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for av1 decode picture packet
//!
#include "decode_av1_picture_packet_xe3_lpm_base.h"
#include "mhw_vdbox_xe3_lpm_base.h"
#include "decode_common_feature_defs.h"
#include "mhw_sfc_hwcmd_xe3_lpm_base.h"
#include "mhw_vdbox_avp_hwcmd_xe3_lpm.h"

using namespace mhw::vdbox::xe3_lpm_base;
using namespace mhw::vdbox::avp::xe3_lpm_base::xe3_lpm;

namespace decode
{
    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::Init()
    {
        DECODE_FUNC_CALL()
        DECODE_CHK_STATUS(Av1DecodePicPkt::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
#ifdef _DECODE_PROCESSING_SUPPORTED
        m_downSamplingFeature      = dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        DecodeSubPacket *subPacket = m_av1Pipeline->GetSubPacket(DecodePacketId(m_av1Pipeline, downSamplingSubPacketId));
        m_downSamplingPkt          = dynamic_cast<DecodeDownSamplingPkt *>(subPacket);
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::GetChromaFormat()
    {
        DECODE_FUNC_CALL();

        m_av1PicParams = m_av1BasicFeature->m_av1PicParams;

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 1)
        {
            chromaSamplingFormat = av1ChromaFormatYuv420;
        }
        else if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 0 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0)
        {
            chromaSamplingFormat = av1ChromaFormatYuv444;
        }
        else
        {
            DECODE_ASSERTMESSAGE("Invalid Chroma sampling format!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::InitAv1State(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL()

        // Send VD_CONTROL_STATE Pipe Initialization
        DECODE_CHK_STATUS(VdInit(cmdBuffer));
        DECODE_CHK_STATUS(AddAllCmds_AVP_PIPE_MODE_SELECT(cmdBuffer));

#ifdef _DECODE_PROCESSING_SUPPORTED
        if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr)
        {
            if (m_downSamplingFeature->IsEnabled() && !(m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))  //LST can't support SFC
            {
                DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
            }
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_STATUS(AddAllCmds_AVP_SURFACE_STATE(cmdBuffer));
        SETPAR_AND_ADDCMD(AVP_PIPE_BUF_ADDR_STATE, m_avpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AVP_IND_OBJ_BASE_ADDR_STATE, m_avpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AVP_PIC_STATE, m_avpItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(AVP_INTER_PRED_STATE, m_avpItf, &cmdBuffer);
        DECODE_CHK_STATUS(AddAllCmds_AVP_SEGMENT_STATE(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL()

        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par       = {};
        par.initialization     = true;
        par.avpEnabled     = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::GetAvpStateCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) 
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(commandsSize);
        DECODE_CHK_NULL(patchListSize);

        uint32_t maxSize          = 0;
        uint32_t patchListMaxSize = 0;

        maxSize =
            m_vdencItf->GETSIZE_VD_PIPELINE_FLUSH() +
            m_miItf->GETSIZE_MI_FLUSH_DW() +
            m_avpItf->GETSIZE_AVP_PIPE_MODE_SELECT() +
            m_avpItf->GETSIZE_AVP_SURFACE_STATE() * 11 +
            m_avpItf->GETSIZE_AVP_PIPE_BUF_ADDR_STATE() +
            m_avpItf->GETSIZE_AVP_IND_OBJ_BASE_ADDR_STATE() +
            m_avpItf->GETSIZE_AVP_SEGMENT_STATE() * 8 +
            m_avpItf->GETSIZE_AVP_INLOOP_FILTER_STATE() +
            m_avpItf->GETSIZE_AVP_INTER_PRED_STATE() +
            m_avpItf->GETSIZE_AVP_FILM_GRAIN_STATE() +
            m_avpItf->GETSIZE_AVP_PIC_STATE() +
            m_miItf->GETSIZE_VD_CONTROL_STATE() * 2;

        patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SURFACE_STATE_CMD) * 11 +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SEGMENT_STATE_CMD) * 8 +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INTER_PRED_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INLOOP_FILTER_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_FILM_GRAIN_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIC_STATE_CMD);

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
        }
        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::GetAvpStateCommandSize(
        uint32_t                        mode,
        uint32_t                       *commandsSize,
        uint32_t                       *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
    {
        //calculate AVP related commands size
        uint32_t avpCommandsSize  = 0;
        uint32_t avpPatchListSize = 0;
        uint32_t cpCmdsize        = 0;
        uint32_t cpPatchListSize  = 0;

        CODEC_HW_CHK_STATUS_RETURN(GetAvpStateCmdSize(
            (uint32_t *)&avpCommandsSize,
            (uint32_t *)&avpPatchListSize,
            params));

        if (m_hwInterface->GetCpInterface() != nullptr)
        {
            m_hwInterface->GetCpInterface()->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
        }

        //Calc final command size
        *commandsSize  = avpCommandsSize + cpCmdsize;
        *patchListSize = avpPatchListSize + cpPatchListSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe3_Lpm_Base::CalculatePictureStateCommandSize()
    {
        DECODE_FUNC_CALL()

        MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE stateCmdSizeParams;
        stateCmdSizeParams.bShortFormat    = true;
        stateCmdSizeParams.bHucDummyStream = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *decodeDownSampling =
        dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        stateCmdSizeParams.bSfcInUse = (decodeDownSampling != nullptr);
#endif
        // Picture Level Commands
        DECODE_CHK_STATUS(GetAvpStateCommandSize(
                m_av1BasicFeature->m_mode,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_PIC_STATE, Av1DecodePicPktXe3_Lpm_Base)
    {
        DECODE_FUNC_CALL();

        params = {};
        Av1DecodePicPkt::MHW_SETPAR_F(AVP_PIC_STATE)(params);

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 0 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0)
        {
            // 4:4:4
            params.chromaFormat = Cmd::AVP_PIC_STATE_CMD::SEQUENCE_CHROMA_SUBSAMPLING_FORMAT_444;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(AVP_SURFACE_STATE, Av1DecodePicPktXe3_Lpm_Base)
    {
        DECODE_FUNC_CALL();

        params = {};
        Av1DecodePicPkt::MHW_SETPAR_F(AVP_SURFACE_STATE)(params);

#ifdef IGFX_AVP_INTERFACE_EXT_SUPPORT
        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX == 0 && m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0)  // 4:4:4
        {
            if (params.bitDepthLumaMinus8 == 0)
            {
                params.srcFormat = static_cast<mhw::vdbox::avp::SURFACE_FORMAT>(SURFACE_FORMAT_EXT::SURFACE_FORMAT_AYUV4444FORMAT);
            }
            else
            {
                params.srcFormat = static_cast<mhw::vdbox::avp::SURFACE_FORMAT>(SURFACE_FORMAT_EXT::SURFACE_FORMAT_Y410FORMAT);
            }
        }
#endif
        return MOS_STATUS_SUCCESS;
    }

}
