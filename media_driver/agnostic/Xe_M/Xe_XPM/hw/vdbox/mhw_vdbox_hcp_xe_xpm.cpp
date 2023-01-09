/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

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
//! \file     mhw_vdbox_hcp_xe_xpm.cpp
//! \brief    Constructs VdBox HCP commands on Xe_XPM platforms

#include "mhw_vdbox_hcp_xe_xpm.h"
#include "mhw_vdbox_hcp_hwcmd_xe_xpm.h"
#include "mhw_utilities_xe_xpm.h"

MhwVdboxHcpInterfaceXe_Xpm::MhwVdboxHcpInterfaceXe_Xpm(
    PMOS_INTERFACE  osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool            decodeInUse)
    : MhwVdboxHcpInterfaceG12(osInterface, miInterface, cpInterface, decodeInUse)
{
    MHW_FUNCTION_ENTER;
}

MhwVdboxHcpInterfaceXe_Xpm ::~MhwVdboxHcpInterfaceXe_Xpm()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MhwVdboxHcpInterfaceXe_Xpm::AddHcpEncodePicStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE       params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
    MHW_MI_CHK_NULL(params->pHevcEncPicParams);
 
    auto paramsG12 = dynamic_cast<PMHW_VDBOX_HEVC_PIC_STATE_G12>(params);
    MHW_MI_CHK_NULL(paramsG12);

    PMHW_BATCH_BUFFER                       batchBuffer = nullptr;
    mhw_vdbox_hcp_xe_xpm::HCP_PIC_STATE_CMD  cmd;

    auto hevcSeqParams = params->pHevcEncSeqParams;
    auto hevcPicParams = params->pHevcEncPicParams;

    if (params->bBatchBufferInUse)
    {
        MHW_MI_CHK_NULL(params->pBatchBuffer);
        batchBuffer = params->pBatchBuffer;
    }

    cmd.DW1.Framewidthinmincbminus1     = hevcSeqParams->wFrameWidthInMinCbMinus1;
    cmd.DW1.PakTransformSkipEnable      = cmd.DW4.TransformSkipEnabledFlag = params->bTransformSkipEnable;
    cmd.DW1.Frameheightinmincbminus1    = hevcSeqParams->wFrameHeightInMinCbMinus1;

    cmd.DW2.Mincusize                   = hevcSeqParams->log2_min_coding_block_size_minus3;
    cmd.DW2.CtbsizeLcusize              = hevcSeqParams->log2_max_coding_block_size_minus3;
    cmd.DW2.Maxtusize                   = hevcSeqParams->log2_max_transform_block_size_minus2;
    cmd.DW2.Mintusize                   = hevcSeqParams->log2_min_transform_block_size_minus2;
    cmd.DW2.Minpcmsize                  = 0; 
    cmd.DW2.Maxpcmsize                  = 0; 

    cmd.DW3.Colpicisi                   = 0; 
    cmd.DW3.Curpicisi                   = 0; 

    cmd.DW4.SampleAdaptiveOffsetEnabledFlag         = params->bSAOEnable;
    cmd.DW4.PcmEnabledFlag                          = 0; // Not supported in CNL
    cmd.DW4.CuQpDeltaEnabledFlag                    = hevcPicParams->cu_qp_delta_enabled_flag; // In VDENC mode, this field should always be set to 1.
    cmd.DW4.DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth  = hevcPicParams->diff_cu_qp_delta_depth;
    cmd.DW4.PcmLoopFilterDisableFlag                = hevcSeqParams->pcm_loop_filter_disable_flag;
    cmd.DW4.ConstrainedIntraPredFlag                = 0; 
    cmd.DW4.Log2ParallelMergeLevelMinus2            = 0; 
    cmd.DW4.SignDataHidingFlag                      = 0; // currently not supported in encoder
    cmd.DW4.LoopFilterAcrossTilesEnabledFlag        = 0; 
    cmd.DW4.EntropyCodingSyncEnabledFlag            = 0; // not supported as per Dimas notes. PAK restriction
    cmd.DW4.TilesEnabledFlag                        = 0; // not supported in encoder
    cmd.DW4.WeightedPredFlag                        = hevcPicParams->weighted_pred_flag;
    cmd.DW4.WeightedBipredFlag                      = hevcPicParams->weighted_bipred_flag;
    cmd.DW4.Fieldpic                                = 0; 
    cmd.DW4.Bottomfield                             = 0; 
    cmd.DW4.AmpEnabledFlag                          = hevcSeqParams->amp_enabled_flag;
    cmd.DW4.TransquantBypassEnableFlag              = hevcPicParams->transquant_bypass_enabled_flag;
    cmd.DW4.StrongIntraSmoothingEnableFlag          = hevcSeqParams->strong_intra_smoothing_enable_flag;
    cmd.DW4.CuPacketStructure                       = 0; // output from HW VME, 1/2 CL per CU

    cmd.DW5.PicCbQpOffset                           = hevcPicParams->pps_cb_qp_offset & 0x1f;
    cmd.DW5.PicCrQpOffset                           = hevcPicParams->pps_cr_qp_offset & 0x1f;
    cmd.DW5.MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra = hevcSeqParams->max_transform_hierarchy_depth_intra;
    cmd.DW5.MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter = hevcSeqParams->max_transform_hierarchy_depth_inter;
    cmd.DW5.PcmSampleBitDepthChromaMinus1                           = hevcSeqParams->pcm_sample_bit_depth_chroma_minus1;
    cmd.DW5.PcmSampleBitDepthLumaMinus1                             = hevcSeqParams->pcm_sample_bit_depth_luma_minus1;
    cmd.DW5.BitDepthChromaMinus8                                    = hevcSeqParams->bit_depth_chroma_minus8;
    cmd.DW5.BitDepthLumaMinus8                                      = hevcSeqParams->bit_depth_luma_minus8;

    cmd.DW6.LcuMaxBitsizeAllowed                                    = hevcPicParams->LcuMaxBitsizeAllowed & 0xffff;
    cmd.DW6.Nonfirstpassflag                                        = 0; // needs to be updated for HEVC VDEnc
    cmd.DW6.LcuMaxBitSizeAllowedMsb2its                             = (hevcPicParams->LcuMaxBitsizeAllowed & 0x00030000) >> 16;
    cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask                   = 0;
    cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask            = 0;
    cmd.DW6.FrameszunderstatusenFramebitrateminreportmask           = 0;
    cmd.DW6.LoadSlicePointerFlag                                    = 0; // must be set to 0 for encoder

    cmd.DW19.RdoqEnable                                             = params->bHevcRdoqEnabled;
    //only 420 format support SSE in DP encode
    cmd.DW19.SseEnable                                              = (hevcSeqParams->chroma_format_idc == 2) ? 0 : (params->bUseVDEnc || params->sseEnabledInVmeEncode);
    // only for VDEnc
    cmd.DW19.RhodomainRateControlEnable                             = params->bUseVDEnc || params->rhodomainRCEnable;   // DW19[6] diff with G12
                                                                // RhoDomainFrameLevelQP: This QP is used for RhoDomain Frame level statistics.
    cmd.DW19.Rhodomainframelevelqp                                  = cmd.DW19.RhodomainRateControlEnable ? hevcPicParams->QpY : 0;  // DW19[13:8]
    cmd.DW19.FractionalQpAdjustmentEnable                           = params->bUseVDEnc;   // DW19[17]

    cmd.DW19.FirstSliceSegmentInPicFlag                             = 1;
    cmd.DW19.Nalunittypeflag                                        = 0;

    // For HEVC VDEnc Dynamic Slice Control
    if (hevcSeqParams->SliceSizeControl == 1)
    {
        cmd.DW19.PakDynamicSliceModeEnable      = 1;
        cmd.DW19.SlicePicParameterSetId         = hevcPicParams->slice_pic_parameter_set_id;
        cmd.DW19.Nalunittypeflag                = (hevcPicParams->nal_unit_type >= HEVC_NAL_UT_BLA_W_LP) &&
            (hevcPicParams->nal_unit_type <= HEVC_NAL_UT_RSV_IRAP_VCL23);
        cmd.DW19.FirstSliceSegmentInPicFlag     = 1;
        cmd.DW19.NoOutputOfPriorPicsFlag        = hevcPicParams->no_output_of_prior_pics_flag;

        cmd.DW21.SliceSizeThresholdInBytes      = hevcPicParams->MaxSliceSizeInBytes;  // HuC FW is expected to update this
        cmd.DW22.TargetSliceSizeInBytes         = hevcPicParams->MaxSliceSizeInBytes;
    }
  
    cmd.DW4.TilesEnabledFlag                 = hevcPicParams->tiles_enabled_flag;
    cmd.DW2.ChromaSubsampling                = hevcSeqParams->chroma_format_idc;
    cmd.DW4.LoopFilterAcrossTilesEnabledFlag = hevcPicParams->loop_filter_across_tiles_flag;

    // Disable HEVC RDOQ for Intra blocks
    cmd.DW20.Intratucountbasedrdoqdisable   = params->bRDOQIntraTUDisable;
    cmd.DW37.Rdoqintratuthreshold           = params->wRDOQIntraTUThreshold;


    // new command for GEN12 to enable SCC
    // DW34
    cmd.DW34.IntraBoundaryFilteringDisabledFlag      = hevcSeqParams->intra_boundary_filtering_disabled_flag;
    cmd.DW34.MotionVectorResolutionControlIdc        = hevcSeqParams->motion_vector_resolution_control_idc;
    cmd.DW34.PpsCurrPicRefEnabledFlag                = hevcPicParams->pps_curr_pic_ref_enabled_flag;
    cmd.DW34.IbcMotionCompensationBufferReferenceIdc = paramsG12->ucRecNotFilteredID;

    // DW35 
    // Enable both TBC and LBC by default, refine later
    cmd.DW35.IbcConfiguration                        = hevcPicParams->pps_curr_pic_ref_enabled_flag ?
                                                       paramsG12->IBCControl :
                                                       HCP_PIC_STATE_IBC_CONFIGURATION_IN_VDENC_MODE_UNNAMED0;
    // Enable palette mode
    cmd.DW35.PaletteModeEnabledFlag                  = hevcSeqParams->palette_mode_enabled_flag;
    cmd.DW35.MonochromePaletteFlag                   = 0;  //Encoder does not support this
    cmd.DW35.PaletteMaxSize                          = hevcSeqParams->palette_mode_enabled_flag ? 64 : 0; // Encoder only supports default value
    cmd.DW35.DeltaPaletteMaxPredictorSize            = hevcSeqParams->palette_mode_enabled_flag ? 32 : 0; // Encoder only supports default value
    cmd.DW35.LumaBitDepthEntryMinus8                 = hevcSeqParams->bit_depth_luma_minus8;
    cmd.DW35.ChromaBitDepthEntryMinus8               = hevcSeqParams->bit_depth_chroma_minus8;

    // Enable Partial Frame Update
    cmd.DW19.PartialFrameUpdateMode  = paramsG12->PartialFrameUpdateEnable ? 1 : 0;
    cmd.DW19.TemporalMvPredDisable   = !hevcSeqParams->sps_temporal_mvp_enable_flag;
    if (cmd.DW19.PartialFrameUpdateMode)
    {
        cmd.DW19.TemporalMvPredDisable = 1;
    }

#ifdef _ENCODE_RESERVED
    if (hevcSeqParams->RateControlMethod == RATECONTROL_CBR && params->maxFrameSize > 0)
    {
        cmd.DW18.Value = params->maxFrameSize;
    }
#endif

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceXe_Xpm::AddHcpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_xe_xpm::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_xe_xpm::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    //Reuse tgllp base class now, since HCP_SURFACE_STATE_CMD changes very little.
    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceG12::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    cmd->DW4.CompressionFormat = params->dwCompressionFormat;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceXe_Xpm::AddHcpEncodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_xe_xpm::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_xe_xpm::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    //Reuse tgllp base class now, since HCP_SURFACE_STATE_CMD changes very little.
    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceG12::AddHcpEncodeSurfaceStateCmd(cmdBuffer, params));

    cmd->DW4.CompressionFormat = params->dwCompressionFormat;

    return eStatus;
}


MOS_STATUS MhwVdboxHcpInterfaceXe_Xpm::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params)
{
    mhw_vdbox_hcp_xe_xpm::HCP_PIPE_BUF_ADDR_STATE_CMD *cmd =
        (mhw_vdbox_hcp_xe_xpm::HCP_PIPE_BUF_ADDR_STATE_CMD *)cmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwVdboxHcpInterfaceG12::AddHcpPipeBufAddrCmd(cmdBuffer, params));

    MOS_SURFACE details;
    MOS_ZeroMemory(&details, sizeof(details));
    details.Format = Format_Invalid;
    MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &params->psPreDeblockSurface->OsResource, &details));
    cmd->DecodedPictureMemoryAddressAttributes.DW0.TileMode = MosGetHWTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

    for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        // All Reference Picture Buffer share same memory address attribute
        // As Reference Surface attribute should be aligned with Recon surface
        // Reference surface tilemode info won't get from GMM but just eqaul to DecodedPicture
        if (params->presReferences[i] != nullptr)
        {
            cmd->ReferencePictureBaseAddressMemoryAddressAttributes.DW0.TileMode = cmd->DecodedPictureMemoryAddressAttributes.DW0.TileMode;
            break;
        }
    }

    if (params->psRawSurface != nullptr)
    {
        MOS_ZeroMemory(&details, sizeof(details));
        details.Format = Format_Invalid;
        MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, &params->psRawSurface->OsResource, &details));
        cmd->OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.TileMode = MosGetHWTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
    }
    return MOS_STATUS_SUCCESS;
}
