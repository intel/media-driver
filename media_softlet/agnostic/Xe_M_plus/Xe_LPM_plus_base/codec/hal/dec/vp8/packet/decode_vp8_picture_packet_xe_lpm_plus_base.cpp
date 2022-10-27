/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_vp8_picture_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for vp8 decode picture packet
//!
#include "decode_vp8_picture_packet.h"
#include "decode_vp8_picture_packet_xe_lpm_plus_base.h"
//#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "decode_common_feature_defs.h"
#include "codec_hw_xe_lpm_plus_base.h"

namespace decode
{
    MOS_STATUS Vp8DecodePicPktXe_Lpm_Plus_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(Vp8DecodePicPkt::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Vp8DecodePicPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(AddMiForceWakeupCmd(cmdBuffer));

        DECODE_CHK_STATUS(AddAllCmds_MFX_PIPE_MODE_SELECT(cmdBuffer));
        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_BSP_BUF_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_VP8_PIC_STATE, m_mfxItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }
    
    MOS_STATUS Vp8DecodePicPktXe_Lpm_Plus_Base::CalculatePictureStateCommandSize()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceXe_Lpm_Plus_Base*>(m_hwInterface)->GetMfxInterfaceNext()->GetMfxStateCommandsDataSize(m_vp8BasicFeature->m_mode, &m_pictureStatesSize, &m_picturePatchListSize, 0));

        return MOS_STATUS_SUCCESS;
    }
}
