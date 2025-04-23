/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_avc_picture_packet_m12.cpp
//! \brief    Defines the interface for avc decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_avc_picture_packet_m12.h"
#include "mhw_mi_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_mfx_g12_X.h"

namespace decode
{
    MOS_STATUS AvcDecodePicPktM12::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(AvcDecodePicPktXe_M_Base::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
    {
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
        DECODE_CHK_STATUS(AddMfxBspBufBaseAddrCmd(cmdBuffer));
        if (m_avcPipeline->IsShortFormat())
        {
            DECODE_CHK_STATUS(AddMfdAvcDpbCmd(cmdBuffer));
        }
        DECODE_CHK_STATUS(AddMfdAvcPicidCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddMfxAvcImgCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddMfxQmCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddMfxAvcDirectmodeCmd(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktM12::AddMfxPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
        pipeModeSelectParams ={};
        SetMfxPipeModeSelectParams(pipeModeSelectParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktM12::AddMfxPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams = {};
        DECODE_CHK_STATUS(SetMfxPipeBufAddrParams(pipeBufAddrParams));
#ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(SetSurfaceMmcState(pipeBufAddrParams));
#endif
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktM12::SetSurfaceMmcState(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        if (m_mmcState->IsMmcEnabled())
        {
            pipeBufAddrParams.bMmcEnabled = true;
            if (m_avcBasicFeature->m_deblockingEnabled)
            {
                DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                    &m_avcBasicFeature->m_destSurface,
                    &pipeBufAddrParams.PostDeblockSurfMmcState));
            }
            else
            {
                DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                    &m_avcBasicFeature->m_destSurface,
                    &pipeBufAddrParams.PreDeblockSurfMmcState));
            }
        }
        else
        {
            pipeBufAddrParams.PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            pipeBufAddrParams.PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktM12::CalculatePictureStateCommandSize()
    {
        // Picture Level Commands
        DECODE_CHK_STATUS(static_cast<CodechalHwInterfaceG12*>(m_hwInterface)->GetMfxStateCommandsDataSize(
                CODECHAL_DECODE_MODE_AVCVLD,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                m_avcPipeline->IsShortFormat()));

        return MOS_STATUS_SUCCESS;
    }

}
