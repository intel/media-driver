/*===================== begin_copyright_notice ==================================
# Copyright (c) 2020-2024, Intel Corporation
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
======================= end_copyright_notice ==================================*/
//!
//! \file     codechal_hw_xe_hpm.cpp
//! \brief    Implements HW interface layer for dg2 used on all OSs.
//! \details  Implements HW interface layer for dg2 to be used on on all operating systems/DDIs, across CODECHAL components.
//!           This module must not contain any OS dependent code.
//!

#include "codechal_hw_next_xe_hpm.h"
#include "codechal_hw_g12_X.h"
#include "mhw_render_g12_X.h"
#include "mhw_vdbox_hcp_hwcmd_xe_hpm.h"  // temporary include for calculating size of various hardware commands
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "media_interfaces_xehp_sdv.h"// temporary include for getting avp interface
#include "media_interfaces_dg2.h"

CodechalHwInterfaceNextXe_Hpm::~CodechalHwInterfaceNextXe_Hpm()
{
    if (m_renderHal != nullptr && m_renderHal->pfnDestroy != nullptr)
    {
        MOS_STATUS eStatus = m_renderHal->pfnDestroy(m_renderHal);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
        }

        if (m_renderHalCpInterface)
        {
            if (m_osInterface)
            {
                m_osInterface->pfnDeleteMhwCpInterface(m_renderHalCpInterface);
                m_renderHalCpInterface = nullptr;
            }
            else
            {
                MHW_ASSERTMESSAGE("Failed to destroy renderHalCpInterface.");
            }
        }
    }

    if (m_renderHal != nullptr)
    {
        MOS_FreeMemory(m_renderHal);
        m_renderHal = nullptr;
    }
    if (m_renderInterface != nullptr)
    {
        MOS_Delete(m_renderInterface);
    }
}

void CodechalHwInterfaceNextXe_Hpm::PrepareCmdSize(CODECHAL_FUNCTION codecFunction)
{
    InitCacheabilityControlSettings(codecFunction);

    // Set platform dependent parameters
    m_sizeOfCmdBatchBufferEnd    = mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_sizeOfCmdMediaReset        = mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 8;
    m_vdencBrcImgStateBufferSize = 80 + mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize + 92 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer1stGroupSize = mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PIPE_MODE_SELECT_CMD::byteSize + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 2 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer2ndGroupSize = 132 + mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PIC_STATE_CMD::byteSize + 248 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

   m_vdencReadBatchBufferSize =
        m_vdenc2ndLevelBatchBufferSize = m_vdencBatchBuffer1stGroupSize + m_vdencBatchBuffer2ndGroupSize + ENCODE_HEVC_VDENC_NUM_MAX_SLICES * (2 * mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_WEIGHTOFFSET_STATE_CMD::byteSize + mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_SLICE_STATE_CMD::byteSize + 3 * mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PAK_INSERT_OBJECT_CMD::byteSize + 28 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize + 4 * ENCODE_VDENC_HEVC_PADDING_DW_SIZE);

    m_HucStitchCmdBatchBufferSize = 7 * 4 + 14 * 4 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    // HCP_WEIGHTOFFSET_STATE_CMD cmds is planned to be added in near future
    m_vdencBatchBufferPerSliceConstSize = mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_SLICE_STATE_CMD::byteSize + mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PAK_INSERT_OBJECT_CMD::byteSize  // 1st PakInsertObject cmd is not always inserted for each slice, 2nd PakInsertObject cmd is always inserted for each slice
                                          + 28 + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;
}

CodechalHwInterfaceNextXe_Hpm::CodechalHwInterfaceNextXe_Hpm(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfacesNext *mhwInterfacesNext,
    bool              disableScalability)
    : CodechalHwInterfaceNext(osInterface, codecFunction, mhwInterfacesNext, disableScalability)
{
    CODECHAL_HW_FUNCTION_ENTER;
    PrepareCmdSize(codecFunction);
    m_miInterface     = static_cast<MhwInterfacesDg2_Next*>(mhwInterfacesNext)->m_miInterface;
    m_renderInterface = static_cast<MhwInterfacesDg2_Next*>(mhwInterfacesNext)->m_renderInterface;
    //prevent double free
    static_cast<MhwInterfacesDg2_Next *>(mhwInterfacesNext)->m_renderInterface = nullptr;
}

bool CodechalHwInterfaceNextXe_Hpm::UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard)
{
    if (codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK && standard == CODECHAL_AV1)
        return false;
    else
        return UsesRenderEngine(codecFunction, standard);
}

MOS_STATUS CodechalHwInterfaceNextXe_Hpm::GetAvpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    CODECHAL_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t    avpCommandsSize = 0;
    uint32_t    avpPatchListSize = 0;
    uint32_t    cpCmdsize        = 0;
    uint32_t    cpPatchListSize  = 0;


    if (GetAvpInterfaceNext())
    {
        CODECHAL_HW_CHK_STATUS_RETURN(GetAvpInterfaceNext()->GetAvpStateCmdSize(
            (uint32_t *)&avpCommandsSize,
            (uint32_t *)&avpPatchListSize,
            params));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpStateLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize  = avpCommandsSize  + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}


MOS_STATUS CodechalHwInterfaceNextXe_Hpm::GetAvpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize)
{
    CODECHAL_HW_FUNCTION_ENTER;

    //calculate AVP related commands size
    uint32_t avpCommandsSize = 0;
    uint32_t avpPatchListSize = 0;
    uint32_t cpCmdsize        = 0;
    uint32_t cpPatchListSize  = 0;


    if (GetAvpInterfaceNext())
    {
        MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
        CODECHAL_HW_CHK_STATUS_RETURN(GetAvpInterfaceNext()->GetAvpPrimitiveCmdSize(
            (uint32_t*)&avpCommandsSize,
            (uint32_t*)&avpPatchListSize,
            &stateCmdSizeParams));
    }

    if (m_cpInterface)
    {
        m_cpInterface->GetCpSliceLevelCmdSize(cpCmdsize, cpPatchListSize);
    }

    //Calc final command size
    *commandsSize  = avpCommandsSize  + cpCmdsize;
    *patchListSize = avpPatchListSize + cpPatchListSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceNextXe_Hpm::Initialize(
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    CODECHAL_HW_CHK_STATUS_RETURN(CodechalHwInterfaceNext::Initialize(settings));
    // Added isRenderHalNeeded flag into settings 
    // Indicate whether RenderHal is needed or not 
    if (settings->isRenderHalNeeded)
    {
        //Initialize renderHal
        m_renderHal = (PRENDERHAL_INTERFACE_LEGACY)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE_LEGACY));
        CODECHAL_HW_CHK_NULL_RETURN(m_renderHal);
        CODECHAL_HW_CHK_STATUS_RETURN(RenderHal_InitInterface_Legacy(
            (PRENDERHAL_INTERFACE_LEGACY)m_renderHal,
            &m_renderHalCpInterface,
            m_osInterface));

        RENDERHAL_SETTINGS_LEGACY RenderHalSettings;
        RenderHalSettings.iMediaStates = 32;
        CODECHAL_HW_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

        //set SSEU table
        m_renderHal->sseuTable = m_ssEuTable;
    }
    return eStatus;
}
#if USE_CODECHAL_DEBUG_TOOL
MediaCopyBaseState *CodechalHwInterfaceNextXe_Hpm::CreateMediaCopy(PMOS_INTERFACE mosInterface)
{
    CODECHAL_HW_FUNCTION_ENTER;
    MediaCopyBaseState *mediaCopy   = nullptr;
    PMOS_CONTEXT        mos_context = nullptr;

    if (mosInterface && mosInterface->pfnGetMosContext)
    {
        mosInterface->pfnGetMosContext(mosInterface, &mos_context);
    }
    mediaCopy = static_cast<MediaCopyBaseState *>(McpyDevice::CreateFactory(mos_context));

    return mediaCopy;
}
#endif