/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     decode_av1_picture_packet_xe2_lpm_base.cpp
//! \brief    Defines the interface for av1 decode picture packet
//!
#include "decode_av1_picture_packet_xe2_lpm_base.h"
#include "mhw_vdbox_xe2_lpm_base.h"
#include "decode_common_feature_defs.h"

using namespace mhw::vdbox::xe2_lpm_base;

namespace decode
{
    MOS_STATUS Av1DecodePicPktXe2_Lpm_Base::Init()
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

    MOS_STATUS Av1DecodePicPktXe2_Lpm_Base::InitAv1State(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL()

        // Send VD_CONTROL_STATE Pipe Initialization
        DECODE_CHK_STATUS(VdInit(cmdBuffer)); //TODO, need refine when MI MHW softlet enabled
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

    MOS_STATUS Av1DecodePicPktXe2_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
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

    MOS_STATUS Av1DecodePicPktXe2_Lpm_Base::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL()

        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par       = {};
        par.initialization     = true;
        par.avpEnabled     = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktXe2_Lpm_Base::CalculatePictureStateCommandSize()
    {
        DECODE_FUNC_CALL()

        MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE2_LPM_BASE stateCmdSizeParams;
        stateCmdSizeParams.bShortFormat    = true;
        stateCmdSizeParams.bHucDummyStream = false;
#ifdef _DECODE_PROCESSING_SUPPORTED
        DecodeDownSamplingFeature *decodeDownSampling =
        dynamic_cast<DecodeDownSamplingFeature *>(m_featureManager->GetFeature(DecodeFeatureIDs::decodeDownSampling));
        stateCmdSizeParams.bSfcInUse = (decodeDownSampling != nullptr);
#endif
        // Picture Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetAvpStateCommandSize(
                m_av1BasicFeature->m_mode,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }
}
