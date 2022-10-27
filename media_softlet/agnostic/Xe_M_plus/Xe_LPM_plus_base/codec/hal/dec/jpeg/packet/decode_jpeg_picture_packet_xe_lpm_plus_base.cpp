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
//! \file     decode_jpeg_picture_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for jpeg decode picture packet
//!
#include "decode_jpeg_picture_packet_xe_lpm_plus_base.h"
#include "mhw_vdbox_xe_lpm_plus_base.h"
#include "codec_hw_xe_lpm_plus_base.h"

namespace decode{

MOS_STATUS JpegDecodePicPktXe_Lpm_Plus_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(JpegDecodePicPkt::Init());
    DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);

    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr &&
        m_downSamplingFeature->IsEnabled())
    {
        DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
    }
#endif
    SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(MFX_JPEG_PIC_STATE, m_mfxItf, &cmdBuffer);

    AddAllCmds_MFX_QM_STATE(&cmdBuffer);
    AddAllCmds_MFX_JPEG_HUFF_TABLE_STATE(&cmdBuffer);
    AddAllCmds_MFD_JPEG_BSD_OBJECT(&cmdBuffer);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktXe_Lpm_Plus_Base::CalculatePictureStateCommandSize()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(m_hwInterface->GetMfxInterfaceNext()->GetMfxStateCommandsDataSize(
        m_jpegBasicFeature->m_mode,
        &m_pictureStatesSize,
        &m_picturePatchListSize,
        0));

    return MOS_STATUS_SUCCESS;
}

}
