/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_avc_picture_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for avc decode picture packet
//!
#include "decode_avc_picture_packet_xe_lpm_plus_base.h"

using namespace mhw::vdbox::xe_lpm_plus_base;

namespace decode
{
   MOS_STATUS AvcDecodePicPktXe_Lpm_Plus_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(AvcDecodePicPkt::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);

        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

#ifdef _DECODE_PROCESSING_SUPPORTED
        if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr)
        {
            DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
        }
#endif
        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_BSP_BUF_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        if (m_avcPipeline->IsShortFormat())
        {
            SETPAR_AND_ADDCMD(MFD_AVC_DPB_STATE, m_mfxItf, &cmdBuffer);
        }
        SETPAR_AND_ADDCMD(MFD_AVC_PICID_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_AVC_IMG_STATE, m_mfxItf, &cmdBuffer);
        AddAllCmds_MFX_QM_STATE(&cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_AVC_DIRECTMODE_STATE, m_mfxItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }


    MOS_STATUS AvcDecodePicPktXe_Lpm_Plus_Base::SetSurfaceMmcState(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        if (m_mmcState->IsMmcEnabled())
        {
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

    MOS_STATUS AvcDecodePicPktXe_Lpm_Plus_Base::CalculatePictureStateCommandSize()
    {
        // Picture Level Commands
        DECODE_CHK_STATUS(
            static_cast<CodechalHwInterfaceXe_Lpm_Plus_Base*>(m_hwInterface)->GetMfxInterfaceNext()->GetMfxStateCommandsDataSize(
                CODECHAL_DECODE_MODE_AVCVLD,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                m_avcBasicFeature->m_shortFormatInUse));

        return MOS_STATUS_SUCCESS;
    }
}
