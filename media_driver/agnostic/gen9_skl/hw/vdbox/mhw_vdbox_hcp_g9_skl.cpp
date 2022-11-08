/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_hcp_g9_skl.cpp
//! \brief    Defines functions for constructing Vdbox HCP commands on G9 SKL
//!

#include "mos_os_cp_interface_specific.h"
#include "mhw_vdbox_hcp_g9_skl.h"
#include "mhw_mi_hwcmd_g9_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_skl.h"

MOS_STATUS MhwVdboxHcpInterfaceG9Skl::GetHcpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;
    uint32_t            standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_HEVC)
    {
        maxSize =
            mhw_vdbox_vdenc_g9_skl::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD);

        if (mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            /* HCP_QM_STATE_CMD may be issued up to 20 times: 3x Colour Component plus 2x intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components.
            HCP_FQP_STATE_CMD may be issued up to 8 times: 4 scaling list per intra and inter. */
            maxSize +=
                mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD::byteSize + // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
                20 * mhw_vdbox_hcp_g9_skl::HCP_QM_STATE_CMD::byteSize +
                8 * mhw_vdbox_hcp_g9_skl::HCP_FQM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_PIC_STATE_CMD::byteSize +
                2 * mhw_mi_g9_X::MI_STORE_DATA_IMM_CMD::byteSize + // Slice level commands
                2 * mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize + // need for Status report, Mfc Status and
                10 * mhw_mi_g9_X::MI_STORE_REGISTER_MEM_CMD::byteSize + // 8 for BRCStatistics and 2 for RC6 WAs
                mhw_mi_g9_X::MI_LOAD_REGISTER_MEM_CMD::byteSize + // 1 for RC6
                2 * mhw_vdbox_hcp_g9_skl::HCP_PAK_INSERT_OBJECT_CMD::byteSize + // Two PAK insert object commands are for headers before the slice header and the header for the end of stream
                4 * mhw_mi_g9_X::MI_STORE_DATA_IMM_CMD::byteSize + // two (BRC+reference frame) for clean-up HW semaphore memory and another two for signal it
                17 * mhw_mi_g9_X::MI_SEMAPHORE_WAIT_CMD::byteSize + // Use HW wait command for each reference and one wait for current semaphore object
                mhw_mi_g9_X::MI_SEMAPHORE_WAIT_CMD::byteSize;        // Use HW wait command for each BRC pass

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                8 * PATCH_LIST_COMMAND(HCP_FQM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) + // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
                2 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) + // Slice level commands
                2 * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) + // need for Status report, Mfc Status and
                11 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) +// 8 for BRCStatistics and 3 for RC6 WAs
                22 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD); // Use HW wait commands plus its memory clean-up and signal (4 + 16 +1 + 1)
        }
        else
        {
            maxSize +=
                20 * mhw_vdbox_hcp_g9_skl::HCP_QM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_TILE_STATE_CMD::byteSize;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_TILE_STATE_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)     // VP9 Clear Decode
    {
        maxSize =
            mhw_vdbox_vdenc_g9_skl::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD::byteSize * 4 +
            mhw_vdbox_hcp_g9_skl::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_skl::HCP_BSD_OBJECT_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) * 4 +
            PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
            PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Skl::GetHcpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t            standard = CodecHal_GetStandardFromMode(mode);
    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;

    if (standard == CODECHAL_HEVC)
    {
        if (mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            maxSize =
                2 * mhw_vdbox_hcp_g9_skl::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g9_skl::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_START_CMD::byteSize;

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PAK_INSERT_OBJECT_CMD) +
                2 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD); // One is for the PAK command and another one is for the BB when BRC and single task mode are on
        }
        else
        {
            maxSize =
                2 * mhw_vdbox_hcp_g9_skl::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g9_skl::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_skl::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)      // VP9 Clear decode does not require primitive level commands. VP9 DRM does.
    {
        if (modeSpecific)                  // VP9 DRM
        {
            maxSize +=
                mhw_vdbox_hcp_g9_skl::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

 MOS_STATUS MhwVdboxHcpInterfaceG9Skl::AddHcpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_skl::HCP_PIPE_MODE_SELECT_CMD  *cmd =
        (mhw_vdbox_hcp_g9_skl::HCP_PIPE_MODE_SELECT_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceG9<mhw_vdbox_hcp_g9_skl>::AddHcpPipeModeSelectCmd(cmdBuffer, params));

    m_cpInterface->SetProtectionSettingsForMfxPipeModeSelect((uint32_t *)cmd);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Skl::AddHcpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_skl>::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    MHW_ASSERT(params->psSurface->Format == Format_NV12);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Skl::AddHcpEncodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_skl::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_skl>::AddHcpEncodeSurfaceStateCmd(cmdBuffer, params));

    MHW_ASSERT(params->psSurface->Format == Format_NV12);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Skl::AddHcpDecodeSliceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcSliceParams);

    auto hevcPicParams = hevcSliceState->pHevcPicParams;
    auto hevcSliceParams = hevcSliceState->pHevcSliceParams;

    mhw_vdbox_hcp_g9_skl::HCP_SLICE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_skl::HCP_SLICE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_skl>::AddHcpDecodeSliceStateCmd(cmdBuffer, hevcSliceState));

    cmd->DW3.Sliceqp = hevcSliceParams->slice_qp_delta +
        hevcPicParams->init_qp_minus26 + 26;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Skl::AddHcpVp9SegmentStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_SEGMENT_STATE     params)
{
    return MOS_STATUS_SUCCESS;
}