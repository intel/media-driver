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
//! \file     mhw_vdbox_hcp_g9_bxt.cpp
//! \brief    Defines functions for constructing Vdbox HCP commands on G9 BXT platform
//!

#include "mos_os_cp_interface_specific.h"
#include "mhw_vdbox_hcp_g9_bxt.h"
#include "mhw_mi_hwcmd_g9_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_bxt.h"

void MhwVdboxHcpInterfaceG9Bxt::InitRowstoreUserFeatureSettings()
{
    MhwVdboxHcpInterfaceG9::InitRowstoreUserFeatureSettings(m_osInterface->pOsContext);

    if (m_rowstoreCachingSupported && m_decodeInUse)
    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;

        PLATFORM  platform;
        m_osInterface->pfnGetPlatform(m_osInterface, &platform);

        if (platform.usRevId >= 3)
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_VP9_HVDROWSTORECACHE_DISABLE_ID,
                &userFeatureData,
                m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
            m_vp9HvdRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_VP9_DFROWSTORECACHE_DISABLE_ID,
                &userFeatureData,
                m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
            m_vp9DfRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;
        }
    }
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::GetHcpStateCommandSize(
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
            mhw_vdbox_vdenc_g9_bxt::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize;

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
                mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD::byteSize + // encoder needs two surface state commands. One is for raw and another one is for recon surfaces.
                20 * mhw_vdbox_hcp_g9_bxt::HCP_QM_STATE_CMD::byteSize +
                8 * mhw_vdbox_hcp_g9_bxt::HCP_FQM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_PIC_STATE_CMD::byteSize +
                2 * mhw_mi_g9_X::MI_STORE_DATA_IMM_CMD::byteSize + // Slice level commands
                2 * mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize + // need for Status report, Mfc Status and
                10 * mhw_mi_g9_X::MI_STORE_REGISTER_MEM_CMD::byteSize + // 8 for BRCStatistics and 2 for RC6 WAs
                mhw_mi_g9_X::MI_LOAD_REGISTER_MEM_CMD::byteSize + // 1 for RC6
                2 * mhw_vdbox_hcp_g9_bxt::HCP_PAK_INSERT_OBJECT_CMD::byteSize; // Two PAK insert object commands are for headers before the slice header and the header for the end of stream

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                8 * PATCH_LIST_COMMAND(HCP_FQM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) +   // When BRC is on, HCP_PIC_STATE_CMD command is in the BB
                2 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) +   // Slice level commands
                2 * PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +   // need for Status report, Mfc Status and
                11 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD); // 8 for BRCStatistics and 3 for RC6 WAs
        }
        else
        {
            maxSize +=
                20 * mhw_vdbox_hcp_g9_bxt::HCP_QM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_TILE_STATE_CMD::byteSize;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_TILE_STATE_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)     // VP9 Clear Decode
    {
        maxSize =
            mhw_vdbox_vdenc_g9_bxt::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD::byteSize * 4 +
            mhw_vdbox_hcp_g9_bxt::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
            mhw_vdbox_hcp_g9_bxt::HCP_VP9_PIC_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_bxt::HCP_BSD_OBJECT_CMD::byteSize;

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

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::GetHcpPrimitiveCommandSize(
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
                2 * mhw_vdbox_hcp_g9_bxt::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g9_bxt::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
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
                2 * mhw_vdbox_hcp_g9_bxt::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g9_bxt::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_BSD_OBJECT_CMD::byteSize +
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
                mhw_vdbox_hcp_g9_bxt::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw_vdbox_hcp_g9_bxt::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_bxt::HCP_BSD_OBJECT_CMD::byteSize +
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

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_bxt::HCP_PIPE_MODE_SELECT_CMD   *cmd =
        (mhw_vdbox_hcp_g9_bxt::HCP_PIPE_MODE_SELECT_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceG9<mhw_vdbox_hcp_g9_bxt>::AddHcpPipeModeSelectCmd(cmdBuffer, params));

    m_cpInterface->SetProtectionSettingsForHcpPipeModeSelect((uint32_t *)cmd);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_bxt>::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_PLANAR_420_8;
    }
    else
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_P010;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpEncodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_bxt::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_bxt>::AddHcpEncodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_PLANAR_420_8;
    }
    else
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_P010;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpDecodePicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcPicParams);

    mhw_vdbox_hcp_g9_bxt::HCP_PIC_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_bxt::HCP_PIC_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_bxt>::AddHcpDecodePicStateCmd(cmdBuffer, params));

    auto hevcPicParams = params->pHevcPicParams;
    PLATFORM  platform;
    m_osInterface->pfnGetPlatform(m_osInterface, &platform);

    // HCP is changed from B-STEPPING for HEVC MAIN 10 decode
    if (platform.usRevId > 2)
    {
        cmd->DW5.BitDepthChromaMinus8 = hevcPicParams->bit_depth_chroma_minus8;
        cmd->DW5.BitDepthLumaMinus8 = hevcPicParams->bit_depth_luma_minus8;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpDecodeSliceStateCmd(
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

    mhw_vdbox_hcp_g9_bxt::HCP_SLICE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_bxt::HCP_SLICE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_bxt>::AddHcpDecodeSliceStateCmd(cmdBuffer, hevcSliceState));

    PLATFORM  platform;
    m_osInterface->pfnGetPlatform(m_osInterface, &platform);

    // HCP is changed from B-STEPPING for HEVC MAIN 10 decode
    if (platform.usRevId > 2)
    {
        int32_t sliceQP = hevcSliceParams->slice_qp_delta + hevcPicParams->init_qp_minus26 + 26;
        cmd->DW3.SliceqpSignFlag = (sliceQP >= 0) ? 0 : 1;
        cmd->DW3.Sliceqp = ABS(sliceQP);
    }
    else
    {
        cmd->DW3.Sliceqp = hevcSliceParams->slice_qp_delta +
            hevcPicParams->init_qp_minus26 + 26;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpVp9PicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_PIC_STATE         params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);

    mhw_vdbox_hcp_g9_bxt::HCP_VP9_PIC_STATE_CMD cmd;
    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList = params->ppVp9RefList;

    uint32_t curFrameWidth                      = vp9PicParams->FrameWidthMinus1 + 1;
    uint32_t curFrameHeight                     = vp9PicParams->FrameHeightMinus1 + 1;
    bool isScaling                              = (curFrameWidth == params->dwPrevFrmWidth) && (curFrameHeight == params->dwPrevFrmHeight) ? false : true;

    cmd.DW1.FrameWidthInPixelsMinus1            = MOS_ALIGN_CEIL(curFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    cmd.DW1.FrameHeightInPixelsMinus1           = MOS_ALIGN_CEIL(curFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    cmd.DW2.FrameType                           = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag              = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag                       = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.RefreshFrameContext                 = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode                  = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode           = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                         = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel                      = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled                 = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap               = cmd.DW2.SegmentationEnabled && vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.LosslessMode                        = vp9PicParams->PicFlags.fields.LosslessFlag;
    cmd.DW2.SegmentIdStreamoutEnable            = cmd.DW2.SegmentationUpdateMap;

    uint8_t segmentIDStreaminEnable = 0;
    if (vp9PicParams->PicFlags.fields.intra_only ||
        (vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME)) {
        segmentIDStreaminEnable = 1;
    } else if (vp9PicParams->PicFlags.fields.segmentation_enabled) {
        if (!vp9PicParams->PicFlags.fields.segmentation_update_map)
            segmentIDStreaminEnable = 1;
        else if (vp9PicParams->PicFlags.fields.segmentation_temporal_update)
            segmentIDStreaminEnable = 1;
    }
    if (vp9PicParams->PicFlags.fields.error_resilient_mode) {
            segmentIDStreaminEnable = 1;
    }
    // Resolution change will reset the segment ID buffer
    if (isScaling)
    {
        segmentIDStreaminEnable = 1;
    }

    cmd.DW2.SegmentIdStreaminEnable       = segmentIDStreaminEnable;

    cmd.DW3.Log2TileRow                         = vp9PicParams->log2_tile_rows;        // No need to minus 1 here.
    cmd.DW3.Log2TileColumn                      = vp9PicParams->log2_tile_columns;     // No need to minus 1 here.

    cmd.DW10.UncompressedHeaderLengthInBytes70  = vp9PicParams->UncompressedHeaderLengthInBytes;
    cmd.DW10.FirstPartitionSizeInBytes150       = vp9PicParams->FirstPartitionSize;

    if (vp9PicParams->PicFlags.fields.frame_type && !vp9PicParams->PicFlags.fields.intra_only)
    {
        PCODEC_PICTURE refFrameList     = &(vp9PicParams->RefFrameList[0]);

        uint8_t  lastRefPicIndex        = refFrameList[vp9PicParams->PicFlags.fields.LastRefIdx].FrameIdx;
        uint32_t lastRefFrameWidth      = vp9RefList[lastRefPicIndex]->dwFrameWidth;
        uint32_t lastRefFrameHeight     = vp9RefList[lastRefPicIndex]->dwFrameHeight;

        uint8_t  goldenRefPicIndex      = refFrameList[vp9PicParams->PicFlags.fields.GoldenRefIdx].FrameIdx;
        uint32_t goldenRefFrameWidth    = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
        uint32_t goldenRefFrameHeight   = vp9RefList[goldenRefPicIndex]->dwFrameHeight;

        uint8_t  altRefPicIndex         = refFrameList[vp9PicParams->PicFlags.fields.AltRefIdx].FrameIdx;
        uint32_t altRefFrameWidth       = vp9RefList[altRefPicIndex]->dwFrameWidth;
        uint32_t altRefFrameHeight      = vp9RefList[altRefPicIndex]->dwFrameHeight;

        cmd.DW2.AllowHiPrecisionMv              = vp9PicParams->PicFlags.fields.allow_high_precision_mv;
        cmd.DW2.McompFilterType                 = vp9PicParams->PicFlags.fields.mcomp_filter_type;
        cmd.DW2.SegmentationTemporalUpdate      = cmd.DW2.SegmentationUpdateMap && vp9PicParams->PicFlags.fields.segmentation_temporal_update;

        cmd.DW2.RefFrameSignBias02              = vp9PicParams->PicFlags.fields.LastRefSignBias | 
                                                  (vp9PicParams->PicFlags.fields.GoldenRefSignBias << 1) | 
                                                  (vp9PicParams->PicFlags.fields.AltRefSignBias << 2);

        cmd.DW2.LastFrameType                   = !params->PrevFrameParams.fields.KeyFrame;

        // Reset UsePrevInFindMvReferences to zero if last picture has a different size,
        // Current picture is error-resilient mode, Last picture was intra_only or keyframe,
        // Last picture was not a displayed picture.
        cmd.DW2.UsePrevInFindMvReferences       =
                 !(vp9PicParams->PicFlags.fields.error_resilient_mode ||
                 params->PrevFrameParams.fields.KeyFrame ||
                 params->PrevFrameParams.fields.IntraOnly ||
                 !params->PrevFrameParams.fields.Display);
        // Reset UsePrevInFindMvReferences in case of resolution change on inter frames
        if (isScaling) {
            cmd.DW2.UsePrevInFindMvReferences   = 0;
        }

        cmd.DW4.HorizontalScaleFactorForLast    = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW4.VerticalScaleFactorForLast      = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW5.HorizontalScaleFactorForGolden  = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW5.VerticalScaleFactorForGolden    = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW6.HorizontalScaleFactorForAltref  = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW6.VerticalScaleFactorForAltref    = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW7.LastFrameWidthInPixelsMinus1    = lastRefFrameWidth - 1;
        cmd.DW7.LastFrameHieghtInPixelsMinus1   = lastRefFrameHeight - 1;

        cmd.DW8.GoldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
        cmd.DW8.GoldenFrameHieghtInPixelsMinus1 = goldenRefFrameHeight - 1;

        cmd.DW9.AltrefFrameWidthInPixelsMinus1  = altRefFrameWidth - 1;
        cmd.DW9.AltrefFrameHieghtInPixelsMinus1 = altRefFrameHeight - 1;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Bxt::AddHcpVp9SegmentStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_SEGMENT_STATE     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_bxt::HCP_VP9_SEGMENT_STATE_CMD  cmd;
    void*  segData = nullptr;

    cmd.DW1.SegmentId = params->ucCurrentSegmentId;

    if (!this->m_decodeInUse)
    {
        CODEC_VP9_ENCODE_SEG_PARAMS             vp9SegData;

        vp9SegData = params->pVp9EncodeSegmentParams->SegData[params->ucCurrentSegmentId];

        if (params->pbSegStateBufferPtr)   // Use the seg data from this buffer (output of BRC)
        {
            segData = params->pbSegStateBufferPtr;
        }
        else    // Prepare the seg data
        {
            cmd.DW2.SegmentSkipped = vp9SegData.SegmentFlags.fields.SegmentSkipped;
            cmd.DW2.SegmentReference = vp9SegData.SegmentFlags.fields.SegmentReference;
            cmd.DW2.SegmentReferenceEnabled = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

            segData = &cmd;
        }
    }
    else
    {
        CODEC_VP9_SEG_PARAMS            vp9SegData;
        vp9SegData = params->pVp9SegmentParams->SegData[params->ucCurrentSegmentId];

        cmd.DW2.SegmentSkipped = vp9SegData.SegmentFlags.fields.SegmentReferenceSkipped;
        cmd.DW2.SegmentReference = vp9SegData.SegmentFlags.fields.SegmentReference;
        cmd.DW2.SegmentReferenceEnabled = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

        cmd.DW3.Filterlevelref0Mode0 = vp9SegData.FilterLevel[0][0];
        cmd.DW3.Filterlevelref0Mode1 = vp9SegData.FilterLevel[0][1];
        cmd.DW3.Filterlevelref1Mode0 = vp9SegData.FilterLevel[1][0];
        cmd.DW3.Filterlevelref1Mode1 = vp9SegData.FilterLevel[1][1];

        cmd.DW4.Filterlevelref2Mode0 = vp9SegData.FilterLevel[2][0];
        cmd.DW4.Filterlevelref2Mode1 = vp9SegData.FilterLevel[2][1];
        cmd.DW4.Filterlevelref3Mode0 = vp9SegData.FilterLevel[3][0];
        cmd.DW4.Filterlevelref3Mode1 = vp9SegData.FilterLevel[3][1];

        cmd.DW5.LumaDcQuantScaleDecodeModeOnly = vp9SegData.LumaDCQuantScale;
        cmd.DW5.LumaAcQuantScaleDecodeModeOnly = vp9SegData.LumaACQuantScale;

        cmd.DW6.ChromaDcQuantScaleDecodeModeOnly = vp9SegData.ChromaDCQuantScale;
        cmd.DW6.ChromaAcQuantScaleDecodeModeOnly = vp9SegData.ChromaACQuantScale;

        segData = &cmd;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, segData, cmd.byteSize));

    return eStatus;
}
