/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

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

#include "codechal_hw_xe_hpm.h"
#include "codechal_hw_g12_X.h"
#include "mhw_render_g12_X.h"
#include "mhw_vdbox_hcp_hwcmd_xe_hpm.h"  // temporary include for calculating size of various hardware commands
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "media_interfaces_xehp_sdv.h"// temporary include for getting avp interface
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "Xe_Hpm_Film_Grain.h"
#endif

void CodechalHwInterfaceXe_Hpm::PrepareCmdSize(CODECHAL_FUNCTION codecFunction)
{
    m_bltState = MOS_New(BltStateXe_Xpm, m_osInterface);
    if(m_bltState != nullptr)
    {
        m_bltState->Initialize();
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid(nullptr) BltStateXe_Xpm!");
    }

    InitCacheabilityControlSettings(codecFunction);

    m_isVdencSuperSliceEnabled = true;

    m_ssEuTable = m_defaultSsEuLutG12;

    // Set platform dependent parameters
    m_sizeOfCmdBatchBufferEnd = mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;
    m_sizeOfCmdMediaReset = mhw_mi_g12_X::MI_LOAD_REGISTER_IMM_CMD::byteSize * 8;
    m_vdencBrcImgStateBufferSize = 80
        + mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize
        + 92
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer1stGroupSize = mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PIPE_MODE_SELECT_CMD::byteSize
        + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 2
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencBatchBuffer2ndGroupSize = 132
        + mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PIC_STATE_CMD::byteSize
        + 248
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_vdencReadBatchBufferSize =
    m_vdenc2ndLevelBatchBufferSize = m_vdencBatchBuffer1stGroupSize
        + m_vdencBatchBuffer2ndGroupSize
        + ENCODE_HEVC_VDENC_NUM_MAX_SLICES
        * (2 * mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_WEIGHTOFFSET_STATE_CMD::byteSize
            + mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_SLICE_STATE_CMD::byteSize
            + 3 * mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PAK_INSERT_OBJECT_CMD::byteSize
            + 28
            + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize
            + 4 * ENCODE_VDENC_HEVC_PADDING_DW_SIZE);

    m_HucStitchCmdBatchBufferSize = 7 * 4 
                                    + 14 * 4 
                                    + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    // HCP_WEIGHTOFFSET_STATE_CMD cmds is planned to be added in near future
    m_vdencBatchBufferPerSliceConstSize = mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_SLICE_STATE_CMD::byteSize
        + mhw::vdbox::hcp::xe_xpm_base::xe_hpm::Cmd::HCP_PAK_INSERT_OBJECT_CMD::byteSize          // 1st PakInsertObject cmd is not always inserted for each slice, 2nd PakInsertObject cmd is always inserted for each slice
        + 28
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    // Set to size of the BRC update command buffer, since it is larger than BRC Init/ PAK integration commands
    m_hucCommandBufferSize = mhw_vdbox_huc_g12_X::HUC_IMEM_STATE_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_PIPE_MODE_SELECT_CMD::byteSize
        + mhw_mi_g12_X::MFX_WAIT_CMD::byteSize * 3
        + mhw_vdbox_huc_g12_X::HUC_DMEM_STATE_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_VIRTUAL_ADDR_STATE_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_STREAM_OBJECT_CMD::byteSize
        + mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize
        + mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize
        + mhw_vdbox_huc_g12_X::HUC_START_CMD::byteSize
        + mhw_vdbox_vdenc_g12_X::VD_PIPELINE_FLUSH_CMD::byteSize
        + mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize
        + mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize * 2
        + mhw_mi_g12_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 2
        + mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

    m_maxKernelLoadCmdSize =
        mhw_mi_g12_X::PIPE_CONTROL_CMD::byteSize +
        mhw_render_g12_X::PIPELINE_SELECT_CMD::byteSize +
        mhw_render_g12_X::MEDIA_OBJECT_CMD::byteSize +
        mhw_render_g12_X::STATE_BASE_ADDRESS_CMD::byteSize +
        mhw_render_g12_X::MEDIA_VFE_STATE_CMD::byteSize +
        mhw_render_g12_X::MEDIA_CURBE_LOAD_CMD::byteSize +
        mhw_render_g12_X::MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD::byteSize +
        mhw_mi_g12_X::MI_BATCH_BUFFER_START_CMD::byteSize +
        mhw_render_g12_X::MEDIA_OBJECT_WALKER_CMD::byteSize +
        mhw_mi_g12_X::MI_STORE_DATA_IMM_CMD::byteSize;

    m_sizeOfCmdMediaObject = mhw_render_g12_X::MEDIA_OBJECT_CMD::byteSize;
    m_sizeOfCmdMediaStateFlush = mhw_mi_g12_X::MEDIA_STATE_FLUSH_CMD::byteSize;
}

CodechalHwInterfaceXe_Hpm::CodechalHwInterfaceXe_Hpm(
    PMOS_INTERFACE    osInterface,
    CODECHAL_FUNCTION codecFunction,
    MhwInterfaces     *mhwInterfaces,
    bool              disableScalability)
    : CodechalHwInterfaceG12(osInterface, codecFunction, mhwInterfaces, disableScalability)
{
    CODECHAL_HW_FUNCTION_ENTER;

    PrepareCmdSize(codecFunction);
}

MOS_STATUS CodechalHwInterfaceXe_Hpm::GetVdencPictureSecondLevelCommandsSize(
    uint32_t  mode,
    uint32_t *commandsSize)
{
    CODECHAL_HW_FUNCTION_ENTER;

    uint32_t commands = 0;

    MHW_MI_CHK_NULL(m_hcpInterface);
    MHW_MI_CHK_NULL(m_vdencInterface);

    uint32_t standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_VP9)
    {
        commands += m_hcpInterface->GetHcpVp9PicStateCommandSize();
        commands += m_hcpInterface->GetHcpVp9SegmentStateCommandSize() * 8;
        commands += 132;
        commands += 248;
        commands += m_sizeOfCmdBatchBufferEnd;
        commands += 24;  // padding for alignment on 64
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported encode mode.");
        return MOS_STATUS_UNKNOWN;
    }

    *commandsSize = commands;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalHwInterfaceXe_Hpm::GetFilmGrainKernelInfo(
    uint8_t*& kernelBase,
    uint32_t& kernelSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    kernelBase = (uint8_t*)XE_HPM_FILM_GRAIN;
    kernelSize = XE_HPM_FILM_GRAIN_SIZE;
#else
    kernelBase = nullptr;
    kernelSize = 0;
#endif

    return eStatus;
}

bool CodechalHwInterfaceXe_Hpm::UsesRenderEngine(CODECHAL_FUNCTION codecFunction, uint32_t standard)
{
    if (codecFunction == CODECHAL_FUNCTION_ENC_VDENC_PAK && standard == CODECHAL_AV1)
        return false;
    else
        return CodechalHwInterface::UsesRenderEngine(codecFunction, standard);
}

MOS_STATUS CodechalHwInterfaceXe_Hpm::SetCacheabilitySettings(
    MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (m_mfxInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_mfxInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (GetHcpInterfaceNext())
    {
        CODECHAL_HW_CHK_STATUS_RETURN(GetHcpInterfaceNext()->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (m_hcpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hcpInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (GetVdencInterfaceNext())
    {
        CODECHAL_HW_CHK_STATUS_RETURN(GetVdencInterfaceNext()->SetCacheabilitySettings(cacheabilitySettings));
    }
    else if (m_vdencInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_vdencInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (GetAvpInterfaceNext())
    {
        CODECHAL_HW_CHK_STATUS_RETURN(GetAvpInterfaceNext()->SetCacheabilitySettings(cacheabilitySettings));
    }
    else if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->SetCacheabilitySettings(cacheabilitySettings));
    }
    if (GetHucInterfaceNext())
    {
        CODECHAL_HW_CHK_STATUS_RETURN(GetHucInterfaceNext()->SetCacheabilitySettings(cacheabilitySettings));
    }
    else if (m_hucInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_hucInterface->SetCacheabilitySettings(cacheabilitySettings));
    }

    return eStatus;
}

MOS_STATUS CodechalHwInterfaceXe_Hpm::GetAvpStateCommandSize(
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
    else if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->GetAvpStateCommandSize(
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


MOS_STATUS CodechalHwInterfaceXe_Hpm::GetAvpPrimitiveCommandSize(
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
    else if (m_avpInterface)
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_avpInterface->GetAvpPrimitiveCommandSize(
            (uint32_t*)&avpCommandsSize,
            (uint32_t*)&avpPatchListSize));
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

MediaCopyBaseState* CodechalHwInterfaceXe_Hpm::CreateMediaCopy(PMOS_INTERFACE mosInterface)
{
    VP_FUNC_CALL();

    MediaCopyBaseState* mediaCopy = nullptr;
    PMOS_CONTEXT       mos_context = nullptr;

    if (mosInterface && mosInterface->pfnGetMosContext)
    {
        mosInterface->pfnGetMosContext(mosInterface, &mos_context);
    }
    mediaCopy = static_cast<MediaCopyBaseState*>(McpyDevice::CreateFactory(mos_context));

    return mediaCopy;
}