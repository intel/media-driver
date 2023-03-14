/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_picture_packet_g12.cpp
//! \brief    Defines the interface for av1 decode picture packet
//!
#include "codechal_utilities.h"
#include "decode_av1_picture_packet_g12.h"
#include "mhw_mi_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_avp_g12_X.h"

namespace decode
{
    MOS_STATUS Av1DecodePicPktG12::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(Av1DecodePicPkt_G12_Base::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktG12::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        // Send VD_CONTROL_STATE Pipe Initialization
        DECODE_CHK_STATUS(VdInit(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpPipeModeSelectCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpSurfacesCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpPipeBufAddrCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpIndObjBaseAddrCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpPicStateCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpInterPredStateCmd(cmdBuffer));
        DECODE_CHK_STATUS(AddAvpSegmentStateCmd(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktG12::VdInit(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        vdCtrlParam.initialization = true;
        vdCtrlParam.avpEnabled     = true;

        MhwMiInterfaceG12* miInterfaceG12 = static_cast<MhwMiInterfaceG12*>(m_miInterface);
        DECODE_CHK_NULL(miInterfaceG12);
        DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktG12::AddAvpPipeModeSelectCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
        pipeModeSelectParams ={};
        SetAvpPipeModeSelectParams(pipeModeSelectParams);
        DECODE_CHK_STATUS(m_avpInterface->AddAvpPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

        return MOS_STATUS_SUCCESS;
    }

    void Av1DecodePicPktG12::SetAvpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12& pipeModeSelectParams)
    {
        DECODE_FUNC_CALL();
        pipeModeSelectParams.bDeblockerStreamOutEnable = false;
    }

    MOS_STATUS Av1DecodePicPktG12::AddAvpPipeBufAddrCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpPipeBufAddrParams pipeBufAddrParams = {};
        DECODE_CHK_STATUS(Av1DecodePicPkt_G12_Base::SetAvpPipeBufAddrParams(pipeBufAddrParams));
    #ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(SetSurfaceMmcState(pipeBufAddrParams));
    #endif
        DECODE_CHK_STATUS(m_avpInterface->AddAvpPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktG12::SetSurfaceMmcState(MhwVdboxAvpPipeBufAddrParams& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

#ifdef _MMC_SUPPORTED
        if (m_mmcState && m_mmcState->IsMmcEnabled())
        {
            DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                &m_av1BasicFeature->m_destSurface,
                &pipeBufAddrParams.m_preDeblockSurfMmcState));
        }
        else
#endif
        {
            pipeBufAddrParams.m_preDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        }

        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS Av1DecodePicPktG12::AddAvpInterPredStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpPicStateParams picStateParams;
        DECODE_CHK_STATUS(SetAvpInterPredStateParams(picStateParams));
        DECODE_CHK_STATUS(m_avpInterface->AddAvpInterPredStateCmd(&cmdBuffer, &picStateParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktG12::AddAvpPicStateCmd(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MhwVdboxAvpPicStateParams picStateParams;
        DECODE_CHK_STATUS(SetAvpPicStateParams(picStateParams));
        DECODE_CHK_STATUS(m_avpInterface->AddAvpDecodePicStateCmd(&cmdBuffer, &picStateParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePicPktG12::CalculatePictureStateCommandSize()
    {
        MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
        stateCmdSizeParams.bShortFormat    = true;
        stateCmdSizeParams.bHucDummyStream = false;
        stateCmdSizeParams.bSfcInUse       = false;
        // Picture Level Commands
        DECODE_CHK_STATUS(m_hwInterface->GetAvpStateCommandSize(
                m_av1BasicFeature->m_mode,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                &stateCmdSizeParams));

        return MOS_STATUS_SUCCESS;
    }

}
