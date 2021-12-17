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
//! \file     decode_jpeg_picture_packet_m12.cpp
//! \brief    Defines the interface for jpeg decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_jpeg_picture_packet_m12.h"
#include "mhw_mi_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_mfx_g12_X.h"

namespace decode{

MOS_STATUS JpegDecodePicPktM12::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(JpegDecodePicPktXe_M_Base::Init());
    DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(AddMfxPipeModeSelectCmd(cmdBuffer));
#ifdef _DECODE_PROCESSING_SUPPORTED
    if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr &&
        m_downSamplingFeature->IsEnabled())
    {
        DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
    }
#endif
    DECODE_CHK_STATUS(AddMfxSurfacesCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxPipeBufAddrCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxIndObjBaseAddrCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxJpegPicCmd(cmdBuffer));

    DECODE_CHK_STATUS(AddMfxQmCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxJpegHuffTableCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxBsdObjectParams(cmdBuffer));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktM12::AddMfxPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
    pipeModeSelectParams ={};
    SetMfxPipeModeSelectParams(pipeModeSelectParams);
    DECODE_CHK_STATUS(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktM12::AddMfxPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams = {};
    DECODE_CHK_STATUS(SetMfxPipeBufAddrParams(pipeBufAddrParams));

    DECODE_CHK_STATUS(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePicPktM12::CalculatePictureStateCommandSize()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->GetMfxStateCommandsDataSize(
        m_jpegBasicFeature->m_mode,
        &m_pictureStatesSize,
        &m_picturePatchListSize,
        0));

    return MOS_STATUS_SUCCESS;
}

}
