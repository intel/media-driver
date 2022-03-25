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
//! \file     decode_mpeg2_picture_packet_m12.cpp
//! \brief    Defines the interface for mpeg2 decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_mpeg2_picture_packet_m12.h"
#include "mhw_mi_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_mfx_g12_X.h"
#include "decode_mpeg2_mem_compression_m12.h"

namespace decode{

MOS_STATUS Mpeg2DecodePicPktM12::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(Mpeg2DecodePicPktXe_M_Base::Init());
    DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(AddMfxPipeModeSelectCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxSurfacesCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxPipeBufAddrCmd(cmdBuffer));
    DECODE_CHK_STATUS(AddMfxIndObjBaseAddrCmd(cmdBuffer));

    if (CodecHalIsDecodeModeVLD(m_mpeg2BasicFeature->m_mode))
    {
        DECODE_CHK_STATUS(AddMfxBspBufBaseAddrCmd(cmdBuffer));
    }

    DECODE_CHK_STATUS(AddMfxMpeg2PicCmd(cmdBuffer));

    if (CodecHalIsDecodeModeVLD(m_mpeg2BasicFeature->m_mode))
    {
        DECODE_CHK_STATUS(AddMfxQmCmd(cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPktM12::AddMfxPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
    pipeModeSelectParams ={};
    SetMfxPipeModeSelectParams(pipeModeSelectParams);
    DECODE_CHK_STATUS(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPktM12::AddMfxPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams = {};
    DECODE_CHK_STATUS(SetMfxPipeBufAddrParams(pipeBufAddrParams));

#ifdef _MMC_SUPPORTED
    Mpeg2DecodeMemCompM12 *mpeg2DecodeMemComp = dynamic_cast<Mpeg2DecodeMemCompM12 *>(m_mmcState);
    DECODE_CHK_NULL(mpeg2DecodeMemComp);
    DECODE_CHK_STATUS(mpeg2DecodeMemComp->CheckReferenceList(*m_mpeg2BasicFeature, pipeBufAddrParams.PreDeblockSurfMmcState, pipeBufAddrParams.PostDeblockSurfMmcState));
#endif

    DECODE_CHK_STATUS(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Mpeg2DecodePicPktM12::CalculatePictureStateCommandSize()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->GetMfxStateCommandsDataSize(
        m_mpeg2BasicFeature->m_mode,
        &m_pictureStatesSize,
        &m_picturePatchListSize,
        0));

    return MOS_STATUS_SUCCESS;
}

}
