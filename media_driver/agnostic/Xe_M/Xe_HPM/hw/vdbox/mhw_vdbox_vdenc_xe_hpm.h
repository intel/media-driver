/*===================== begin_copyright_notice ==================================

INTEL CONFIDENTIAL
Copyright 2019-2021
Intel Corporation All Rights Reserved.

The source code contained or described herein and all documents related to the
source code ("Material") are owned by Intel Corporation or its suppliers or
licensors. Title to the Material remains with Intel Corporation or its suppliers
and licensors. The Material contains trade secrets and proprietary and confidential
information of Intel or its suppliers and licensors. The Material is protected by
worldwide copyright and trade secret laws and treaty provisions. No part of the
Material may be used, copied, reproduced, modified, published, uploaded, posted,
transmitted, distributed, or disclosed in any way without Intel's prior express
written permission.

No license under any patent, copyright, trade secret or other intellectual
property right is granted to or conferred upon you by disclosure or delivery
of the Materials, either expressly, by implication, inducement, estoppel
or otherwise. Any license under such intellectual property rights must be
express and approved by Intel in writing.

======================= end_copyright_notice ==================================*/

//! \file     mhw_vdbox_vdenc_xe_hpm.h
//! \details  Defines functions for constructing Vdbox Vdenc commands on DG2 platform
//!

#ifndef __MHW_VDBOX_VDENC_XE_HPM_H__
#define __MHW_VDBOX_VDENC_XE_HPM_H__

#ifdef IGFX_VDENC_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_vdenc_xe_hpm_cmd_ext.h"
#else

#include "mhw_vdbox_vdenc_hwcmd_xe_hpm.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "codec_def_common_av1.h"
#include "mhw_vdbox_vdenc_xe_xpm.h"
// Include ext head file here temporarily for accessing MHW_VDBOX_VDENC_HEVC_VP9_TILE_SLICE_STATE_PARAMS.
// MHW_VDBOX_VDENC_HEVC_VP9_TILE_SLICE_STATE_PARAMS cannot be put in mhw_vdbox_vdenc_xe_xpm.h since
// it is referenced in common file, such as as encode_tile.cpp, encode_hevc_tile.cpp and
// encode_hevc_vdenc_weighted_prediction.cpp. Need to put MHW_VDBOX_VDENC_HEVC_VP9_TILE_SLICE_STATE_PARAMS
// to the better place later.
#include "mhw_vdbox_vdenc_g12_X_ext.h"
#include "mhw_vdbox_vdenc_impl.h"

using PMHW_VDBOX_AVC_IMG_PARAMS_XE_HPM = MHW_VDBOX_AVC_IMG_PARAMS_XE_XPM *;

template<> inline uint32_t
MhwVdboxVdencInterfaceGeneric<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetVdencAvcCostStateSize()
{
    return mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_COSTS_STATE_CMD::byteSize;
}

template<> inline MOS_STATUS
MhwVdboxVdencInterfaceGeneric<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencConstQPStateCmd(
    PMOS_COMMAND_BUFFER                cmdBuffer,
    PMHW_VDBOX_VDENC_CQPT_STATE_PARAMS params)
{
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Explictly instantiate AddVdencPipeModeSelectCmd of MhwVdboxVdencInterfaceG12 for mhw::vdbox::vdenc::xe_hpm::Cmd
//!         to because programming of DW1.PakChromaSubSamplingType has changed
//!
template<> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(m_osInterface);

    auto paramsG12 = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(params);
    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD cmd;
    MEDIA_WA_TABLE *pWaTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_CHK_NULL_RETURN(pWaTable);

    cmd.DW1.StandardSelect  = (params->Mode == codechalEncodeModeAv1)? 3 : CodecHal_GetStandardFromMode(params->Mode);
    cmd.DW1.ScalabilityMode = !(paramsG12->MultiEngineMode == MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY);
    if (CODECHAL_ENCODE_MODE_HEVC == params->Mode || 
        CODECHAL_ENCODE_MODE_AVC  == params->Mode ||
        codechalEncodeModeAv1 == params->Mode)
    {
        cmd.DW1.FrameStatisticsStreamOutEnable = paramsG12->bBRCEnabled || paramsG12->bAdaptiveRounding || paramsG12->bLookaheadPass;
    }
    else
    {
        cmd.DW1.FrameStatisticsStreamOutEnable = 1;
    }
    cmd.DW1.VdencPakObjCmdStreamOutEnable = params->bVdencPakObjCmdStreamOutEnable;
    cmd.DW1.TlbPrefetchEnable             = 0;
    cmd.DW1.PakThresholdCheckEnable       = params->bDynamicSliceEnable;
    cmd.DW1.VdencStreamInEnable           = params->bVdencStreamInEnable;
    cmd.DW1.BitDepth                      = params->ucVdencBitDepthMinus8;
    cmd.DW1.PakChromaSubSamplingType      = params->ChromaType;

    // by default RGB to YUV using full to studio range
    // can add a DDI flag to control if needed
    cmd.DW1.OutputRangeControlAfterColorSpaceConversion = 1;

    // for tile encoding
    cmd.DW1.TileReplayEnable = paramsG12->bTileBasedReplayMode;
    cmd.DW1.IsRandomAccess = paramsG12->bIsRandomAccess;

    //BSpec restriction: When this pre-fetch is enabled,TLB Prefetch Enable in VDENC_PIPE_MODE select (DW 1 bit 7) should be disabled.
    cmd.DW2.HmeRegionPreFetchenable                      = 1;
    if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && !m_osInterface->bSimIsActive && !IsSimuless(m_osInterface->pOsContext) &&
        (codechalEncodeModeAv1 == params->Mode || ((CODECHAL_ENCODE_MODE_HEVC == params->Mode) && !params->bIBCEnabled) || CODECHAL_ENCODE_MODE_VP9 == params->Mode))
    {
        cmd.DW2.HmeRegionPreFetchenable = !(params->isIFrame);
    }
    cmd.DW2.Topprefetchenablemode                        = 0;
    cmd.DW2.LeftpreFetchatwraparound                     = 1;
    cmd.DW2.Verticalshift32Minus1                        = 2;
    cmd.DW2.Hzshift32Minus1                              = 3;
    cmd.DW2.NumVerticalReqMinus1                         = 6;
    cmd.DW2.Numhzreqminus1                               = 2;
    cmd.DW2.PreFetchOffsetForReferenceIn16PixelIncrement = 0;

    if(cmd.DW1.StandardSelect == CODECHAL_AVC && paramsG12->tuSettingsRevision == 1) {
        // override perf settings for AVC codec on B-stepping
        static const uint8_t latencyToleratePreFetchEnable[] = { 0, 0, 1, 1, 1, 1, 1 };

        cmd.DW2.Verticalshift32Minus1                        = 0;
        cmd.DW2.Hzshift32Minus1                              = 15;
        cmd.DW2.NumVerticalReqMinus1                         = 5;
        cmd.DW2.Numhzreqminus1                               = 0;

        cmd.DW5.LatencyToleratePreFetchEnable                = latencyToleratePreFetchEnable[paramsG12->tuMinus1];
    }

    // For RGB encoding
    if (paramsG12->bRGBEncodingMode)
    {
        cmd.DW1.RgbEncodingEnable = 1;
    }

    // For parallel encode from display
    if (paramsG12->bWirelessEncodeEnabled)
    {
        cmd.DW5.CaptureMode                       = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::CAPTURE_MODE_UNNAMED1;
        cmd.DW5.ParallelCaptureAndEncodeSessionId = paramsG12->ucWirelessSessionId;
        cmd.DW5.TailPointerReadFrequency          = 0x50;
    }

    if (paramsG12->bStreamingBufferEnabled)
    {
        cmd.DW1.StreamingBufferConfig = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::STREAMING_BUFFER_64;
        cmd.DW5.CaptureMode           = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::CAPTURE_MODE_CAMERA;
    }

    cmd.DW5.QuantizationPrecisionOptimization = paramsG12->ucQuantizationPrecision & 0x1;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

template<> inline uint32_t
MhwVdboxVdencInterfaceGeneric<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetVdencAvcImgStateSize()
{
    return mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_IMG_STATE_CMD::byteSize;
}

template <> inline uint32_t
MhwVdboxVdencInterfaceGeneric<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetVdencAvcSlcStateSize()
{
    return mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_SLICE_STATE_CMD::byteSize;
}

template<> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencAvcWeightsOffsetsStateCmd(
    PMOS_COMMAND_BUFFER                cmdBuffer,
    PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pAvcPicParams);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WEIGHTSOFFSETS_STATE_CMD cmd;
    auto avcPicParams = params->pAvcPicParams;

    if (avcPicParams->weighted_pred_flag)
    {
        cmd.DW1.WeightsForwardReference0  = params->Weights[0][0][0][0];
        cmd.DW1.OffsetForwardReference0   = params->Weights[0][0][0][1];
        cmd.DW1.WeightsForwardReference1  = params->Weights[0][1][0][0];
        cmd.DW1.OffsetForwardReference1   = params->Weights[0][1][0][1];
        cmd.DW2.WeightsForwardReference2  = params->Weights[0][2][0][0];
        cmd.DW2.OffsetForwardReference2   = params->Weights[0][2][0][1];
    }

    if (avcPicParams->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE)
    {
        cmd.DW2.WeightsBackwardReference0 = params->Weights[1][0][0][0];
        cmd.DW2.OffsetBackwardReference0  = params->Weights[1][0][0][1];
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

template<> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencWeightsOffsetsStateCmd(
    PMOS_COMMAND_BUFFER                   cmdBuffer,
    PMHW_BATCH_BUFFER                     batchBuffer,
    PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WEIGHTSOFFSETS_STATE_CMD cmd;

    // Luma Offsets and Weights
    if (params->bWeightedPredEnabled)
    {
        uint32_t  refPicListNum = 0;
        cmd.DW1.WeightsForwardReference0 = CodecHal_Clip3(-128, 127,
            params->LumaWeights[refPicListNum][0] + params->dwDenom);
        cmd.DW1.OffsetForwardReference0  = params->LumaOffsets[refPicListNum][0];
        cmd.DW1.WeightsForwardReference1 = CodecHal_Clip3(-128, 127,
            params->LumaWeights[refPicListNum][1] + params->dwDenom);
        cmd.DW1.OffsetForwardReference1  = params->LumaOffsets[refPicListNum][1];
        cmd.DW2.WeightsForwardReference2 = CodecHal_Clip3(-128, 127,
            params->LumaWeights[refPicListNum][2] + params->dwDenom);
        cmd.DW2.OffsetForwardReference2  = params->LumaOffsets[refPicListNum][2];

        cmd.DW2.WeightsBackwardReference0 = 0;
        cmd.DW2.OffsetBackwardReference0  = 0;
        if (!params->isLowDelay)
        {
            refPicListNum = 1;
            cmd.DW2.WeightsBackwardReference0 = CodecHal_Clip3(-128, 127,
                params->LumaWeights[refPicListNum][0] + params->dwDenom);
            cmd.DW2.OffsetBackwardReference0 = params->LumaOffsets[refPicListNum][0];
        }
        else
        {
            cmd.DW2.WeightsBackwardReference0 = cmd.DW1.WeightsForwardReference0;
            cmd.DW2.OffsetBackwardReference0 = cmd.DW1.OffsetForwardReference0;
        }

        // DWORD 3
        refPicListNum = 0;
        cmd.DW3.CbWeightsForwardReference0 = CodecHal_Clip3(-128, 127,
            params->ChromaWeights[refPicListNum][0][0] + params->dwChromaDenom);
        cmd.DW3.CbOffsetForwardReference0 = params->ChromaOffsets[refPicListNum][0][0];
        cmd.DW3.CbWeightsForwardReference1 = CodecHal_Clip3(-128, 127,
            params->ChromaWeights[refPicListNum][1][0] + params->dwChromaDenom);
        cmd.DW3.CbOffsetForwardReference1 = params->ChromaOffsets[refPicListNum][1][0];

        // DWORD 4
        cmd.DW4.CbWeightsForwardReference2 = CodecHal_Clip3(-128, 127,
            params->ChromaWeights[refPicListNum][2][0] + params->dwChromaDenom);
        cmd.DW4.CbOffsetForwardReference2 = params->ChromaOffsets[refPicListNum][2][0];
        if (!params->isLowDelay)
        {
            refPicListNum = 1;
            cmd.DW4.CbWeightsBackwardReference0 = CodecHal_Clip3(-128, 127,
                params->ChromaWeights[refPicListNum][0][0] + params->dwChromaDenom);
            cmd.DW4.CbOffsetBackwardReference0 = params->ChromaOffsets[refPicListNum][0][0];
        }
        else
        {
            cmd.DW4.CbWeightsBackwardReference0 = cmd.DW3.CbWeightsForwardReference0;
            cmd.DW4.CbOffsetBackwardReference0 = cmd.DW3.CbOffsetForwardReference0;
        }

        // DWORD 5
        refPicListNum = 0;
        cmd.DW5.CrWeightsForwardReference0 = CodecHal_Clip3(-128, 127,
            params->ChromaWeights[refPicListNum][0][1] + params->dwChromaDenom);
        cmd.DW5.CrOffsetForwardReference0 = params->ChromaOffsets[refPicListNum][0][1];
        cmd.DW5.CrWeightsForwardReference1 = CodecHal_Clip3(-128, 127,
            params->ChromaWeights[refPicListNum][1][1] + params->dwChromaDenom);
        cmd.DW5.CrOffsetForwardReference1 = params->ChromaOffsets[refPicListNum][1][1];

        // DWORD 6
        cmd.DW6.CrWeightsForwardReference2 = CodecHal_Clip3(-128, 127,
            params->ChromaWeights[refPicListNum][2][1] + params->dwChromaDenom);
        cmd.DW6.CrOffsetForwardReference2 = params->ChromaOffsets[refPicListNum][2][1];
        if (!params->isLowDelay)
        {
            refPicListNum = 1;
            cmd.DW6.CrWeightsBackwardReference0 = CodecHal_Clip3(-128, 127,
                params->ChromaWeights[refPicListNum][0][1] + params->dwChromaDenom);
            cmd.DW6.CrOffsetBackwardReference0 = params->ChromaOffsets[refPicListNum][0][1];
        }
        else
        {
            cmd.DW6.CrWeightsBackwardReference0 = cmd.DW5.CrWeightsForwardReference0;
            cmd.DW6.CrOffsetBackwardReference0 = cmd.DW5.CrOffsetForwardReference0;
        }
    }
    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

template<> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencImgStateCmd(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    PMHW_BATCH_BUFFER         batchBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS params)

{
    MHW_FUNCTION_ENTER;

    // Frame type related parameters
    static const uint32_t sadHaarThreshold0[] = { 0x3e8, 0x320, 0x258 };
    static const uint32_t sadHaarThreshold1[] = { 0x7d0, 0x640, 0x4b0 };
    static const uint32_t sadHaarThreshold2[] = { 0xbb8, 0x960, 0x708 };
    static const uint32_t midpointSadHaar[]   = { 0x7d0, 0x640, 0x4b0 };

    // Target usage related parameters
    static const uint8_t numberOfFractionalCandidatesChecked[] = { 4, 4, 2 ,2, 2, 1, 1 };
    static const uint8_t numberOfFractionalCandidatesRefB[]    = { 2, 2, 2 ,2, 2, 1, 1 };
    static const uint8_t imePredictorLargeSearchWindow[]       = { 0, 0, 0, 0, 0, 0, 0 };
    static const uint8_t hmeRefWindowSize[]                    = { 0, 0, 0, 0, 0, 2, 2 };
    static const uint8_t ImeLeftPredDep[]                      = { 2, 2, 1, 1, 1, 1, 1 };

    // Frame type and target usage related parameters
    static const uint8_t numImePredictors[][7]                       = { { 0, 0, 0, 0, 0, 0, 0 }, { 7, 7, 7, 7, 7, 4, 4 }, { 7, 7, 7, 7, 7, 4, 4 } };
    static const uint8_t initialMacroblockBudgetForTransform4X4[][7] = { { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0xff, 0xff }, { 0, 0, 0, 0, 0, 0xff, 0xff } };
    static const uint8_t intraTransform4X4Percentage[][7]            = { { 0, 0, 0, 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 3, 3 }, { 0, 0, 0, 0, 0, 3, 3 } };
    static const uint8_t LumaIntraPartitionMask[]                    = { 0, 0, 0, 0, 0, 4, 4 };

    // Revision 1 TU settings override
    static const uint8_t numImePredictors_v1[][7] = { { 0, 0, 0, 0, 0, 0, 0 }, { 7, 7, 7, 7, 7, 4, 4 }, { 7, 7, 4, 3, 3, 2, 2 } };
    static const uint8_t imePredictorLargeSearchWindow_v1[]    = { 1, 1, 0, 0, 0, 0, 0 };
    static const uint8_t ImeLeftPredDep_v1[]                   = { 0, 0, 1, 1, 1, 1, 1 };

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pEncodeAvcSeqParams);
    MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_IMG_STATE_CMD cmd;

    auto paramsG12 = static_cast<PMHW_VDBOX_AVC_IMG_PARAMS_XE_HPM>(params);
    auto seqParams = params->pEncodeAvcSeqParams;
    auto picParams = params->pEncodeAvcPicParams;
    auto slcParams = params->pEncodeAvcSliceParams;

    auto tu               = seqParams->TargetUsage;
    auto tuMinus1         = tu - 1;
    auto codingType       = picParams->CodingType;
    auto pictureType      = codingType - 1;
    uint32_t refPoc[2][3] = {};

    bool isRefB = (codingType == B_TYPE) && picParams->RefPicFlag;

    MHW_ASSERT(tu <= 7);
    MHW_ASSERT(pictureType < NUM_PIC_TYPES);

    cmd.DW1.LeftNeighborPixelMode                       = 1;
    cmd.DW1.PictureType                                 = pictureType;
    cmd.DW1.Transform8X8Flag                            = picParams->transform_8x8_mode_flag;
    cmd.DW1.SubpelMode                                  = 3;
    cmd.DW1.ImePredictorOverlapThreshold                = 3;
    cmd.DW1.MacroblockSizeEstimatedScalingRatioForIntra = 0;
    cmd.DW1.IntraMBHdrScaleFactor                       = 0;
    cmd.DW1.MacroblockSizeEstimatedScalingRatioForInter = 0;
    cmd.DW1.InterMBHdrScaleFactor                       = 0;
    cmd.DW1.colloc_mv_wr_en                             = paramsG12->colMVWriteEnable;

    cmd.DW2.NumImePredictors                    = paramsG12->tuSettingsRevision ? numImePredictors_v1[pictureType][tuMinus1] : numImePredictors[pictureType][tuMinus1];
    cmd.DW2.HmeRefWindowSize                    = hmeRefWindowSize[tuMinus1];
    cmd.DW2.ImeLeftPredDep                      = paramsG12->tuSettingsRevision ? ImeLeftPredDep_v1[tuMinus1] : ImeLeftPredDep[tuMinus1];

    cmd.DW2.NumberOfFractionalCandidatesChecked = (isRefB) ? numberOfFractionalCandidatesRefB[tuMinus1] : numberOfFractionalCandidatesChecked[tuMinus1];
    if (params->bRollingIRestrictFracCand && codingType != I_TYPE && picParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED && tu > 5)
    {
        cmd.DW2.NumberOfFractionalCandidatesChecked = 2;
    }

    cmd.DW2.ImePredictorLargeSearchWindow       = paramsG12->tuSettingsRevision ? imePredictorLargeSearchWindow_v1[tuMinus1] : imePredictorLargeSearchWindow[tuMinus1];
    cmd.DW2.InterSadAdjustment                  = 2;
    cmd.DW2.IntraSadAdjustment                  = 2;
    if (codingType == B_TYPE)
    {
        if (params->pEncodeAvcPicParams->weighted_bipred_idc == 2)
        {
            cmd.DW2.BidirectionalWeight = params->biWeight;
        }
        else
        {
            cmd.DW2.BidirectionalWeight = 0x20;
        }
    }
    cmd.DW2.colloc_mv_rd_en                     = paramsG12->colMVReadEnable;
    cmd.DW2.SubmacroblockPartitionMask          = (isRefB) ? 0x72 : 0x70;
    cmd.DW2.RdeChromaEnable                     = 0;

    cmd.DW3.PictureHeightMinusOne = params->wPicHeightInMb - 1;
    cmd.DW3.PictureWidth          = params->wPicWidthInMb;

    cmd.DW4.AvcIntra4X4ModeMask    = 0;
    cmd.DW4.LumaIntraPartitionMask = LumaIntraPartitionMask[tuMinus1];
    cmd.DW4.IntraChromaMask        = 0;
    cmd.DW4.AvcIntra8X8ModeMask    = 0;
    cmd.DW4.RdeIntraChromaSearch   = 0;
    cmd.DW4.IntraComputeType       = 0;
    cmd.DW4.AvcIntra16X16ModeMask  = 0;

    if (codingType != I_TYPE)
    {
        cmd.DW5.NumberOfL0ReferencesMinusOne = slcParams->num_ref_idx_l0_active_minus1;
        cmd.DW5.NumberOfL1ReferencesMinusOne = slcParams->num_ref_idx_l1_active_minus1;

        cmd.DW5.RefIdxToReferencePictureNonDefaultEnable = !paramsG12->oneOnOneMapping;
        if (cmd.DW5.RefIdxToReferencePictureNonDefaultEnable)
        {
            MHW_CHK_STATUS_RETURN(slcParams->num_ref_idx_l0_active_minus1 > 2);
            uint8_t fwdRefIdx[3] = {0xf, 0xf, 0xf};
            for (auto i = 0; i < slcParams->num_ref_idx_l0_active_minus1 + 1; i++)
            {
                auto id = slcParams->RefPicList[LIST_0][i].FrameIdx;
                id = params->pPicIdx[id].ucPicIdx;
                fwdRefIdx[i] = params->ppRefList[id]->ucFrameId;
                refPoc[0][i] = params->ppRefList[id]->iFieldOrderCnt[0];
            }
            cmd.DW5.FwdRefIdx0ReferencePicture = fwdRefIdx[0];
            cmd.DW5.FwdRefIdx1ReferencePicture = fwdRefIdx[1];
            cmd.DW5.FwdRefIdx2ReferencePicture = fwdRefIdx[2];

            if (codingType == B_TYPE)
            {
                auto id = slcParams->RefPicList[LIST_1][0].FrameIdx;
                id = params->pPicIdx[id].ucPicIdx;
                cmd.DW5.BwdRefIdx0ReferencePicture = params->ppRefList[id]->ucFrameId;
                refPoc[1][0]                       = params->ppRefList[id]->iFieldOrderCnt[0];
                cmd.DW5.TemporalBDirectMode        = 0;
            }
        }
    }

    // Rolling-I settings
    if (codingType != I_TYPE && picParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED)
    {
        cmd.DW6.IntraRefreshMbPos                = picParams->IntraRefreshMBNum;
        cmd.DW6.IntraRefreshMbSizeMinusOne       = picParams->IntraRefreshUnitinMB;
        cmd.DW6.IntraRefreshEnableRollingIEnable = picParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED ? 1 : 0;
        cmd.DW6.IntraRefreshMode                 = picParams->EnableRollingIntraRefresh == ROLLING_I_ROW ? 0 : 1;
        cmd.DW6.QpAdjustmentForRollingI          = picParams->IntraRefreshQPDelta;

        auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        MHW_MI_CHK_NULL(waTable);

        // WA to prevent error propagation from top-right direction.
        // Disable prediction modes 3, 7 for 4x4
        // and modes 0, 2, 3, 4, 5, 7 for 8x8 (due to filtering)
        if (picParams->EnableRollingIntraRefresh == ROLLING_I_COLUMN &&
            MEDIA_IS_WA(waTable, Wa_18011246551))
        {
            cmd.DW4.AvcIntra4X4ModeMask = 0x88;
            cmd.DW4.AvcIntra8X8ModeMask = 0xBD;
        }
    }

    cmd.DW7.PanicModeMbThreshold = 0;
    cmd.DW7.SmallMbSizeInWord    = 0xff;
    cmd.DW7.LargeMbSizeInWord    = 0xff;

    cmd.DW8.MaxHMvR = 0x2000;
    cmd.DW8.MaxVMvR = params->dwMaxVmvR;

    cmd.DW9.RoiQpAdjustmentForZone0                  = 0;
    cmd.DW9.RoiQpAdjustmentForZone1                  = 0;
    cmd.DW9.RoiQpAdjustmentForZone2                  = 0;
    cmd.DW9.RoiQpAdjustmentForZone3                  = 0;
    cmd.DW9.QpAdjustmentForShapeBestIntra4X4Winner   = 0;
    cmd.DW9.QpAdjustmentForShapeBestIntra8X8Winner   = 0;
    cmd.DW9.QpAdjustmentForShapeBestIntra16X16Winner = 0;

    cmd.DW10.BestdistortionQpAdjustmentForZone0 = 0;
    cmd.DW10.BestdistortionQpAdjustmentForZone1 = 0;
    cmd.DW10.BestdistortionQpAdjustmentForZone2 = 0;
    cmd.DW10.BestdistortionQpAdjustmentForZone3 = 0;
    cmd.DW10.SadHaarThreshold0                  = sadHaarThreshold0[pictureType];

    cmd.DW11.SadHaarThreshold1 = sadHaarThreshold1[pictureType];
    cmd.DW11.SadHaarThreshold2 = sadHaarThreshold2[pictureType];

    cmd.DW12.MinQp                                  = 0xa;
    cmd.DW12.MaxQp                                  = 0x33;
    cmd.DW12.InitialMacroblockBudgetForTransform4X4 = initialMacroblockBudgetForTransform4X4[pictureType][tuMinus1];
    cmd.DW12.MaxDeltaQp                             = 0xf;
    
    // VDEnc CQP case ROI settings, BRC ROI will be handled in HuC FW
    if (!params->bVdencBRCEnabled && picParams->NumROI && picParams->bNativeROI)
    {
        int8_t priorityLevelOrDQp[ENCODE_VDENC_AVC_MAX_ROI_NUMBER_ADV] = {0};

        for (uint8_t i = 0; i < picParams->NumROI; i++)
        {

            // clip delta qp roi to VDEnc supported range
            if (picParams->ROIDistinctDeltaQp[i] == 0)
            {
                break;
            }
            priorityLevelOrDQp[i] = (char)CodecHal_Clip3(
                ENCODE_VDENC_AVC_MIN_ROI_DELTA_QP_G9, ENCODE_VDENC_AVC_MAX_ROI_DELTA_QP_G9, picParams->ROIDistinctDeltaQp[i]);
        }

        cmd.DW13.RoiEnable = true;

        // Zone0 is reserved for non-ROI region
        cmd.DW9.RoiQpAdjustmentForZone1 = priorityLevelOrDQp[0];
        cmd.DW9.RoiQpAdjustmentForZone2 = priorityLevelOrDQp[1];
        cmd.DW9.RoiQpAdjustmentForZone3 = priorityLevelOrDQp[2];
    }

    cmd.DW13.RoiEnable                                 = params->bVdencStreamInEnabled && picParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED && 
                                                            (picParams->NumDirtyROI && params->bVdencBRCEnabled || picParams->NumROI && picParams->bNativeROI ||
                                                            (picParams->TargetFrameSize > 0 && !seqParams->LookaheadDepth));  // TCBRC (for AdaptiveRegionBoost)
    cmd.DW13.FwdPredictor0MvEnable                     = params->bVdencStreamInEnabled;
    cmd.DW13.BwdPredictor1MvEnable                     = 0;
    cmd.DW13.MbLevelQpEnable                           = (params->bVdencStreamInEnabled && picParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED)
                                                            && (picParams->NumROI && !picParams->bNativeROI || paramsG12->bStreamInMbQpEnabled);
    cmd.DW13.TargetSizeInWordsMbMaxSizeInWordsMbEnable = 0;
    cmd.DW13.MbLevelDeltaQpEnable                      = paramsG12->bStreamInMbQpEnabled && picParams->NumDeltaQpForNonRectROI;
    cmd.DW13.ZeroCbfSkipCheckEn                        = 0;
    cmd.DW13.ZeroCbfInterCheckEn                       = 0;
    cmd.DW13.CoefficientClampEnable                    = 0;
    const uint8_t refFrameId                           = slcParams->RefPicList[1][0].FrameIdx;
    cmd.DW13.LongtermReferenceFrameBwdRef0Indicator    = (refFrameId >= CODEC_AVC_MAX_NUM_REF_FRAME) ? 0 : CodecHal_PictureIsLongTermRef(picParams->RefFrameList[refFrameId]);
    cmd.DW13.ZMVPredictorDisable                       = 0;
    cmd.DW13.ToPRightIMEPredictorDisable               = 0;
    cmd.DW13.ToPIMEPredictorDisable                    = 0;
    cmd.DW13.MidpointSadHaar                           = midpointSadHaar[pictureType];

    cmd.DW14.QpPrimeY                    = picParams->QpY + slcParams->slice_qp_delta;
    cmd.DW14.ForceIpcmMinQp              = 0;
    cmd.DW14.TrellisTableSel             = 0;
    cmd.DW14.IpcmEnable                  = 0;
    cmd.DW14.TrellisQuantEn              = params->dwTqEnabled;
    cmd.DW14.CrePrefetchEnable           = 1;
    cmd.DW14.IntraTransform4X4Percentage = intraTransform4X4Percentage[pictureType][tuMinus1];
    cmd.DW14.Targetsizeinword            = 0xff;

    cmd.DW15.IpcmZone0Qp                = 2;
    cmd.DW15.PocNumberForCurrentPicture = paramsG12->ppRefList[paramsG12->pEncodeAvcPicParams->CurrReconstructedPic.FrameIdx]->iFieldOrderCnt[0];
    cmd.DW15.IpcmZone0Thr               = 0xbb8;

    cmd.DW16.IpcmZone1Qp         = 4;
    cmd.DW16.PocNumberForFwdRef0 = refPoc[0][0];
    cmd.DW16.IpcmZone1Thr        = 0xe10;

    cmd.DW17.IpcmZone2Qp         = 6;
    cmd.DW17.PocNumberForFwdRef1 = refPoc[0][1];
    cmd.DW17.IpcmZone2Thr        = 0x1388;

    cmd.DW18.IpcmZone3Qp         = 0xa;
    cmd.DW18.PocNumberForFwdRef2 = refPoc[0][2];
    cmd.DW18.IpcmZone3Thr        = 0x1f40;

    cmd.DW19.IpcmZone4Qp         = 0x12;
    cmd.DW19.PocNumberForBwdRef0 = refPoc[1][0];
    cmd.DW19.IpcmZone4Thr        = 0x2328;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Explictly instantiate GetAvcMaxSize of MhwVdboxVdencInterfaceG12 for mhw::vdbox::vdenc::xe_hpm::Cmd
//!         because there is no VDENC_CONST_QPT_STATE_CMD and VDENC_IMG_STATE has been changed to VDENC_AVC_IMG_STATE_CMD for DG2
//!
template<> inline uint32_t
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetAvcMaxSize(uint32_t waAddDelayInVDEncDynamicSlice)
{
    uint32_t maxSize =
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_SRC_SURFACE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_REF_SURFACE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_DS_REF_SURFACE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_BUF_ADDR_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_IMG_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WALKER_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_COSTS_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_SLICE_STATE_CMD::byteSize;

    if (waAddDelayInVDEncDynamicSlice)
    {
        maxSize += mhw::vdbox::vdenc::xe_hpm::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize * MHW_VDBOX_VDENC_DYNAMIC_SLICE_WA_COUNT;
    }

    return maxSize;
}

//!
//! \brief  Explictly instantiate GetAvcSliceMaxSize of MhwVdboxVdencInterfaceG12 for mhw::vdbox::vdenc::xe_hpm::Cmd,
//!         because there is VDENC_AVC_SLICE_STATE_CMD for DG2
//!
template<> inline uint32_t
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetAvcSliceMaxSize()
{
    uint32_t maxSize =
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_SLICE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WALKER_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize;

    return maxSize;
}

template<> inline uint32_t
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetReserved0TileMaxSize()
{
    uint32_t maxSize = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_HEVC_VP9_TILE_SLICE_STATE_CMD::byteSize +
        GetVdencCmd1Size() +
        GetVdencCmd2Size() +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WALKER_STATE_CMD::byteSize;

    return maxSize;
}

template<> inline uint32_t
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::GetReserved0MaxSize()
{
    uint maxSize =
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_CONTROL_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_MODE_SELECT_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_SRC_SURFACE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_REF_SURFACE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_DS_REF_SURFACE_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_BUF_ADDR_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WALKER_STATE_CMD::byteSize +
        mhw::vdbox::vdenc::xe_hpm::Cmd::VD_PIPELINE_FLUSH_CMD::byteSize;;

    return maxSize;
}

template<> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(m_osInterface);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_BUF_ADDR_STATE_CMD cmd;

    MOS_SURFACE details;
    uint8_t     refIdx;

    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_VDENC_PIPE_BUF_ADDR;
    MEDIA_WA_TABLE *pWaTable     = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_CHK_NULL_RETURN(pWaTable);

    if (params->psRawSurface != nullptr)
    {
        cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(params->RawSurfMmcState) ? 1 : 0;
        cmd.OriginalUncompressedPicture.PictureFields.DW0.CompressionType         = MmcIsRc(params->RawSurfMmcState) ? 1 : 0;
        cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Gen12_7.Index;
        cmd.OriginalUncompressedPicture.PictureFields.DW0.CompressionFormat = params->pRawSurfParam->dwCompressionFormat;

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.OriginalUncompressedPicture.LowerAddress);
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (this->m_vdencRowStoreCache.bEnabled)
    {
        cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.CacheSelect = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_Control_Bits_CMD::CACHE_SELECT_UNNAMED1;
        cmd.RowStoreScratchBuffer.LowerAddress.DW0.Value = this->m_vdencRowStoreCache.dwAddress << 6;
    }
    else if (params->presVdencIntraRowStoreScratchBuffer != nullptr)
    {
        cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC].Gen12_7.Index;

        resourceParams.presResource = params->presVdencIntraRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.RowStoreScratchBuffer.LowerAddress);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presVdencStreamOutBuffer != nullptr)
    {
        cmd.VdencStatisticsStreamout.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Gen12_7.Index;

        resourceParams.presResource = params->presVdencStreamOutBuffer;
        resourceParams.dwOffset = params->dwVdencStatsStreamOutOffset;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.VdencStatisticsStreamout.LowerAddress);
        resourceParams.dwLocationInCmd = 34;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presVdencStreamInBuffer != nullptr)
    {
        cmd.StreaminDataPicture.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Gen12_7.Index;

        resourceParams.presResource = params->presVdencStreamInBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.StreaminDataPicture.LowerAddress);
        resourceParams.dwLocationInCmd = 13;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Reference_Picture_CMD *FwdBwdRefs[] = { &cmd.FwdRef0, &cmd.FwdRef1, &cmd.FwdRef2, &cmd.BwdRef0 };
    mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Down_Scaled_Reference_Picture_CMD *DsFwdRefs[] = { &cmd.DsFwdRef0, &cmd.DsFwdRef1 };
    mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Down_Scaled_Reference_Picture_CMD *DsFwdRefs4X[] = {&cmd.DsFwdRef04X, &cmd.DsFwdRef14X, &cmd.Additional4xDsFwdRef};
    PMOS_SURFACE DsSurfStage1, DsSurfStage2;

    if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
    {
        DsSurfStage1 = params->ps4xDsSurface;
        DsSurfStage2 = nullptr;
    }
    else
    {
        DsSurfStage1 = params->ps8xDsSurface;
        DsSurfStage2 = params->ps4xDsSurface;
    }

    for (refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
    {
        if (params->presVdencReferences[refIdx])
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdencReferences[refIdx], &details));

            resourceParams.presResource = params->presVdencReferences[refIdx];
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = (refIdx * 3) + 22;
            resourceParams.bIsWritable = false;
            resourceParams.pdwCmd = (uint32_t*)&FwdBwdRefs[refIdx]->LowerAddress;

            mmcMode = (params->PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
                params->PostDeblockSurfMmcState : params->PreDeblockSurfMmcState;

            FwdBwdRefs[refIdx]->PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
            FwdBwdRefs[refIdx]->PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
            FwdBwdRefs[refIdx]->PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;
            FwdBwdRefs[refIdx]->PictureFields.DW0.CompressionFormat = params->pDecodedReconParam->dwCompressionFormat;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        PMOS_RESOURCE dsFwdRefsSurf, dsFwdRefs4XSurf;
        if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            dsFwdRefsSurf = params->presVdenc4xDsSurface[refIdx];
            dsFwdRefs4XSurf = nullptr;
        }
        else
        {
            if (MEDIA_IS_WA(pWaTable, Wa_22011549751) && params->isIFrame && !this->m_osInterface->bSimIsActive && !IsSimuless(this->m_osInterface->pOsContext) && 
                (codechalEncodeModeAv1 == params->Mode || ((CODECHAL_ENCODE_MODE_HEVC == params->Mode) && !params->bIBCEnabled) || CODECHAL_ENCODE_MODE_VP9 == params->Mode))
            {
                dsFwdRefsSurf   = &DsSurfStage1->OsResource;
                dsFwdRefs4XSurf = &DsSurfStage2->OsResource;
            }
            else
            {
                dsFwdRefsSurf   = params->presVdenc8xDsSurface[refIdx];
                dsFwdRefs4XSurf = params->presVdenc4xDsSurface[refIdx];
            }
        }
        if (refIdx <= 1 && dsFwdRefsSurf)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, dsFwdRefsSurf, &details));

            resourceParams.presResource = dsFwdRefsSurf;
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = (refIdx * 3) + 1;
            resourceParams.bIsWritable = false;
            resourceParams.pdwCmd = (uint32_t *)&(DsFwdRefs[refIdx]->LowerAddress);

            if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
            {
                mmcMode = params->Ps4xDsSurfMmcState;
            }
            else
            {
                mmcMode = params->Ps8xDsSurfMmcState;
            }

            DsFwdRefs[refIdx]->PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
            DsFwdRefs[refIdx]->PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
            DsFwdRefs[refIdx]->PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (refIdx <= 2 && dsFwdRefs4XSurf)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, dsFwdRefs4XSurf, &details));

            resourceParams.presResource = dsFwdRefs4XSurf;
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;

            if (!params->isLowDelayB && params->dwNumRefIdxL0ActiveMinus1 == 1 && params->dwNumRefIdxL1ActiveMinus1 == 0 && refIdx == 1 && params->presVdenc4xDsSurface[2] != nullptr)
            {
                resourceParams.dwLocationInCmd = 86;
                resourceParams.bIsWritable = false;
                resourceParams.pdwCmd      = (uint32_t *)&(DsFwdRefs4X[2]->LowerAddress);

                mmcMode = params->Ps4xDsSurfMmcState;

                DsFwdRefs4X[2]->PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                DsFwdRefs4X[2]->PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                DsFwdRefs4X[2]->PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
            else
            {
                if (refIdx == 2)
                {
                    resourceParams.dwLocationInCmd = 86;
                }
                else
                {
                    resourceParams.dwLocationInCmd = (refIdx * 3) + 37;
                }
                resourceParams.bIsWritable = false;
                resourceParams.pdwCmd = (uint32_t *)&(DsFwdRefs4X[refIdx]->LowerAddress);

                mmcMode = params->Ps4xDsSurfMmcState;

                DsFwdRefs4X[refIdx]->PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                DsFwdRefs4X[refIdx]->PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                DsFwdRefs4X[refIdx]->PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }
    }

    if ((!params->isLowDelayB && (params->Mode == CODECHAL_ENCODE_RESERVED_0)) || ((params->Mode == CODECHAL_ENCODE_MODE_HEVC)&&(!params->isLowDelayB || params->isPFrame)) || params->Mode == CODECHAL_ENCODE_MODE_AVC)
    {
        if (params->presVdencReferences[refIdx])
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdencReferences[refIdx], &details));

            resourceParams.presResource = params->presVdencReferences[refIdx];
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = OFFSET_OF(typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_BUF_ADDR_STATE_CMD, BwdRef0) / sizeof(uint32_t);
            resourceParams.bIsWritable = false;
            resourceParams.pdwCmd = (uint32_t *)&(cmd.BwdRef0.LowerAddress);

            mmcMode = (params->PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
                params->PostDeblockSurfMmcState : params->PreDeblockSurfMmcState;

            cmd.BwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
            cmd.BwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
            cmd.BwdRef0.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;
            cmd.BwdRef0.PictureFields.DW0.CompressionFormat = params->pDecodedReconParam->dwCompressionFormat;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        PMOS_RESOURCE DsBwdRef0Surf, DsBwdRef04XSurf;
        if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            DsBwdRef0Surf = params->presVdenc4xDsSurface[refIdx];
            DsBwdRef04XSurf = nullptr;
        }
        else
        {
            DsBwdRef0Surf = params->presVdenc8xDsSurface[refIdx];
            DsBwdRef04XSurf = params->presVdenc4xDsSurface[refIdx];
        }
        if (DsBwdRef0Surf)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, DsBwdRef0Surf, &details));

            resourceParams.presResource = DsBwdRef0Surf;
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = OFFSET_OF(typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_BUF_ADDR_STATE_CMD, DsBwdRef0) / sizeof(uint32_t);
            resourceParams.bIsWritable = false;
            resourceParams.pdwCmd = (uint32_t *)&(cmd.DsBwdRef0.LowerAddress);

            if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
            {
                mmcMode = params->Ps4xDsSurfMmcState;
            }
            else
            {
                mmcMode = params->Ps8xDsSurfMmcState;
            }

            cmd.DsBwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
            cmd.DsBwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
            cmd.DsBwdRef0.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (DsBwdRef04XSurf)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, DsBwdRef04XSurf, &details));

            resourceParams.presResource = DsBwdRef04XSurf;
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = OFFSET_OF(typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_PIPE_BUF_ADDR_STATE_CMD, DsBwdRef04X) / sizeof(uint32_t);
            resourceParams.bIsWritable = false;
            resourceParams.pdwCmd = (uint32_t*)&(cmd.DsBwdRef04X.LowerAddress);

            mmcMode = params->Ps4xDsSurfMmcState;

            cmd.DsBwdRef04X.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
            cmd.DsBwdRef04X.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
            cmd.DsBwdRef04X.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    if (params->presVdencColocatedMVReadBuffer != nullptr)
    {
        resourceParams.presResource    = params->presVdencColocatedMVReadBuffer;
        resourceParams.dwOffset        = 0;
        resourceParams.pdwCmd          = (uint32_t *)&(cmd.ColocatedMv.LowerAddress);
        resourceParams.dwLocationInCmd = 19;
        resourceParams.bIsWritable     = false;

        cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionEnable = 0;
        cmd.ColocatedMv.PictureFields.DW0.CompressionType         = 0;
        cmd.ColocatedMv.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presColMvTempBuffer[0] != nullptr && !params->isPFrame)
    {
        resourceParams.presResource = params->presColMvTempBuffer[0];
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.ColocatedMv.LowerAddress);
        resourceParams.dwLocationInCmd = 19;
        resourceParams.bIsWritable = true;

        cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionEnable = 0;
        cmd.ColocatedMv.PictureFields.DW0.CompressionType = 0;
        cmd.ColocatedMv.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (DsSurfStage1 != nullptr)
    {
        resourceParams.presResource = &DsSurfStage1->OsResource;
        resourceParams.dwOffset = DsSurfStage1->dwOffset;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.ScaledReferenceSurfaceStage1.LowerAddress);
        resourceParams.dwLocationInCmd = 49;
        resourceParams.bIsWritable = true;

        if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            mmcMode = params->Ps4xDsSurfMmcState;
        }
        else
        {
            mmcMode = params->Ps8xDsSurfMmcState;
        }

        cmd.ScaledReferenceSurfaceStage1.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
        cmd.ScaledReferenceSurfaceStage1.PictureFields.DW0.CompressionType = MmcIsRc(mmcMode) ? 1 : 0;
        cmd.ScaledReferenceSurfaceStage1.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (DsSurfStage2 != nullptr)
    {
        resourceParams.presResource = &DsSurfStage2->OsResource;
        resourceParams.dwOffset = DsSurfStage2->dwOffset;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.ScaledReferenceSurfaceStage2.LowerAddress);
        resourceParams.dwLocationInCmd = 52;
        resourceParams.bIsWritable = true;

        cmd.ScaledReferenceSurfaceStage2.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(params->Ps4xDsSurfMmcState) ? 1 : 0;
        cmd.ScaledReferenceSurfaceStage2.PictureFields.DW0.CompressionType         = MmcIsRc(params->Ps4xDsSurfMmcState) ? 1 : 0;
        cmd.ScaledReferenceSurfaceStage2.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presVdencPakObjCmdStreamOutBuffer)
    {
        resourceParams.presResource = params->presVdencPakObjCmdStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.VdencLcuPakObjCmdBuffer.LowerAddress);
        resourceParams.dwLocationInCmd = 46;
        resourceParams.bIsWritable = true;

        cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryCompressionEnable = 0;
        cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.CompressionType = 0;
        cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presSegmentMapStreamOut)
    {
        if (params->presSegmentMapStreamIn)
        {
            resourceParams.presResource    = params->presSegmentMapStreamIn;
        }
        else
        {
            resourceParams.presResource    = params->presSegmentMapStreamOut;
        }
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.Vp9SegmentationMapStreaminBuffer.LowerAddress);
        resourceParams.dwLocationInCmd = 55;
        resourceParams.bIsWritable = true;

        cmd.Vp9SegmentationMapStreaminBuffer.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));

        resourceParams.presResource = params->presSegmentMapStreamOut;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.Vp9SegmentationMapStreamoutBuffer.LowerAddress);
        resourceParams.dwLocationInCmd = 58;
        resourceParams.bIsWritable = true;

        cmd.Vp9SegmentationMapStreamoutBuffer.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Following Fulsim programming hard-coding the value 
    // The first 2 CLs(cacheline=64bytes) are ENC frame statistics data. 
    // The 3rd CL is for VDL1* stats (hits & misses which Fulsim doesn't model). 
    // Hence it's a dummy CL for us. Histogram stats start from 4th CL onwards. 
    cmd.DW61.WeightsHistogramStreamoutOffset = 3 * MHW_CACHELINE_SIZE;

    if (params->presVdencTileRowStoreBuffer != nullptr)
    {
        cmd.VdencTileRowStoreBuffer.BufferPictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC].Gen12_7.Index;

        resourceParams.presResource = params->presVdencTileRowStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t*)&(cmd.VdencTileRowStoreBuffer.LowerAddress);
        resourceParams.dwLocationInCmd = 62;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (this->m_vdencRowStoreCache.bEnabled && (params->Mode == CODECHAL_ENCODE_RESERVED_0 || params->Mode == CODECHAL_ENCODE_MODE_AVC))
    {
        cmd.IntraPredictionRowstoreBaseAddress.BufferPictureFields.DW0.CacheSelect = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_Control_Bits_CMD::CACHE_SELECT_UNNAMED1;
        cmd.IntraPredictionRowstoreBaseAddress.LowerAddress.DW0.Value = m_vdencIpdlRowstoreCache.dwAddress << 6;
    }

    else if (Mos_ResourceIsNull(params->presMfdIntraRowStoreScratchBuffer) == false)
    {
        cmd.IntraPredictionRowstoreBaseAddress.BufferPictureFields.DW0.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_7.Index;
        cmd.IntraPredictionRowstoreBaseAddress.BufferPictureFields.DW0.MemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

        resourceParams.presResource = params->presMfdIntraRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t *)&(cmd.IntraPredictionRowstoreBaseAddress);
        resourceParams.dwLocationInCmd = 77;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presVdencCumulativeCuCountStreamoutSurface != nullptr)
    {
        resourceParams.presResource = params->presVdencCumulativeCuCountStreamoutSurface;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (uint32_t *)&(cmd.VdencCumulativeCuCountStreamoutSurface.LowerAddress);
        resourceParams.dwLocationInCmd = 65;
        resourceParams.bIsWritable = true;

        cmd.VdencCumulativeCuCountStreamoutSurface.PictureFields.DW0.MemoryObjectControlState =
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Gen12_7.Index;

        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presVdencColocatedMVWriteBuffer != nullptr)
    {
        if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            resourceParams.presResource    = params->presVdencColocatedMVWriteBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t *)&(cmd.ColocatedMvAvcWriteBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = 83;
            resourceParams.bIsWritable     = true;

            cmd.ColocatedMvAvcWriteBuffer.PictureFields.DW0.MemoryCompressionEnable = 0;
            cmd.ColocatedMvAvcWriteBuffer.PictureFields.DW0.CompressionType         = 0;
            cmd.ColocatedMvAvcWriteBuffer.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_7.Index;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}
template<> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_WALKER_STATE_CMD cmd;

    if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
    {
        auto avcSeqParams = params->pAvcSeqParams;
        auto avcSlcParams = params->pAvcSlcParams;
        MHW_MI_CHK_NULL(avcSeqParams);
        MHW_MI_CHK_NULL(avcSlcParams);

        auto frameHeight = static_cast<uint32_t>(CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(avcSeqParams->FrameHeight));
        auto frameWidth = static_cast<uint32_t>(CODECHAL_GET_WIDTH_IN_MACROBLOCKS(avcSeqParams->FrameWidth));

        cmd.DW1.MbLcuStartYPosition = avcSlcParams->first_mb_in_slice / frameWidth;
        cmd.DW1.FirstSuperSlice = 1;

        auto nextsliceMbStartYPosition = (avcSlcParams->first_mb_in_slice + avcSlcParams->NumMbsForSlice) / frameWidth;
        cmd.DW2.NextsliceMbStartYPosition = nextsliceMbStartYPosition > frameHeight ? frameHeight : nextsliceMbStartYPosition;
    }
    else if (params->Mode == CODECHAL_ENCODE_MODE_HEVC)
    {
        auto paramsG12 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12>(params);
        MHW_MI_CHK_NULL(paramsG12);

        MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
        MHW_MI_CHK_NULL(params->pHevcEncPicParams);
        MHW_MI_CHK_NULL(params->pEncodeHevcSliceParams);

        auto seqParams   = params->pHevcEncSeqParams;
        auto picParams   = params->pHevcEncPicParams;
        auto sliceParams = params->pEncodeHevcSliceParams;

        uint32_t ctbSize     = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
        uint32_t widthInPix  = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameWidthInMinCbMinus1 + 1);
        uint32_t widthInCtb  = (widthInPix / ctbSize) + ((widthInPix % ctbSize) ? 1 : 0);  // round up
        uint32_t heightInPix = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameHeightInMinCbMinus1 + 1);
        uint32_t heightInCtb = (heightInPix / ctbSize) + ((heightInPix % ctbSize) ? 1 : 0);  // round up
        uint32_t shift       = seqParams->log2_max_coding_block_size_minus3 - seqParams->log2_min_coding_block_size_minus3;

        if (paramsG12->pTileCodingParams == nullptr)
        {
            // No tiling support
            cmd.DW1.MbLcuStartYPosition          = sliceParams->slice_segment_address / widthInCtb;
            cmd.DW2.NextsliceMbLcuStartXPosition = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / heightInCtb;
            cmd.DW2.NextsliceMbStartYPosition    = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / widthInCtb;
        }
        else
        {
            cmd.DW1.MbLcuStartXPosition = paramsG12->pTileCodingParams->TileStartLCUX;
            cmd.DW1.MbLcuStartYPosition = paramsG12->pTileCodingParams->TileStartLCUY;

            // In HEVC vdnec, always first super slice in each tile 
            cmd.DW1.FirstSuperSlice = 1;

            cmd.DW2.NextsliceMbLcuStartXPosition = paramsG12->pTileCodingParams->TileStartLCUX + (paramsG12->pTileCodingParams->TileWidthInMinCbMinus1 >> shift) + 1;
            cmd.DW2.NextsliceMbStartYPosition    = paramsG12->pTileCodingParams->TileStartLCUY + (paramsG12->pTileCodingParams->TileHeightInMinCbMinus1 >> shift) + 1;
        }
    }
    else if (params->Mode == CODECHAL_ENCODE_MODE_VP9)
    {
        auto paramsG12 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12>(params);
        MHW_MI_CHK_NULL(paramsG12);
        MHW_MI_CHK_NULL(params->pVp9EncPicParams);
        auto vp9PicParams = params->pVp9EncPicParams;
        auto tileCodingParams = paramsG12->pTileCodingParams;

        if (tileCodingParams == nullptr)
        {
            cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS(vp9PicParams->SrcFrameWidthMinus1, CODEC_VP9_SUPER_BLOCK_WIDTH);
            cmd.DW2.NextsliceMbStartYPosition = CODECHAL_GET_HEIGHT_IN_BLOCKS(vp9PicParams->SrcFrameHeightMinus1, CODEC_VP9_SUPER_BLOCK_HEIGHT);
            cmd.DW1.FirstSuperSlice = 1;
        }
        else
        {
            auto tileStartCtbX = tileCodingParams->TileStartLCUX * CODEC_VP9_SUPER_BLOCK_WIDTH;
            auto tileStartCtbY = tileCodingParams->TileStartLCUY * CODEC_VP9_SUPER_BLOCK_HEIGHT;

            auto tileWidth = ((tileCodingParams->TileWidthInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
            auto tileHeight = ((tileCodingParams->TileHeightInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

            cmd.DW1.MbLcuStartXPosition = tileCodingParams->TileStartLCUX;
            cmd.DW1.MbLcuStartYPosition = tileCodingParams->TileStartLCUY;
            cmd.DW1.FirstSuperSlice     = 1;

            cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS((tileStartCtbX + tileWidth + 1), CODEC_VP9_SUPER_BLOCK_WIDTH);
            cmd.DW2.NextsliceMbStartYPosition = CODECHAL_GET_HEIGHT_IN_BLOCKS((tileStartCtbY + tileHeight + 1), CODEC_VP9_SUPER_BLOCK_HEIGHT);
        }
    }
    else if (params->Mode == CODECHAL_ENCODE_RESERVED_0)
    {
        auto paramsG12 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12>(params);
        MHW_MI_CHK_NULL(paramsG12);
        auto tileCodingParams = paramsG12->pTileCodingParams;

        if (tileCodingParams == nullptr)
        {
            cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS(paramsG12->frameWidthMinus1, av1SuperBlockWidth);
            cmd.DW2.NextsliceMbStartYPosition    = CODECHAL_GET_HEIGHT_IN_BLOCKS(paramsG12->frameHeightMinus1, av1SuperBlockHeight);
        }
        else
        {
            auto tileStartCtbX = tileCodingParams->TileStartLCUX * av1SuperBlockWidth;
            auto tileStartCtbY = tileCodingParams->TileStartLCUY * av1SuperBlockHeight;

            auto tileWidth  = ((tileCodingParams->TileWidthInMinCbMinus1 + 1) * av1MinBlockWidth) - 1;
            auto tileHeight = ((tileCodingParams->TileHeightInMinCbMinus1 + 1) * av1MinBlockHeight) - 1;

            cmd.DW1.MbLcuStartXPosition = tileCodingParams->TileStartLCUX;
            cmd.DW1.MbLcuStartYPosition = tileCodingParams->TileStartLCUY;

            cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS((tileStartCtbX + tileWidth + 1), av1SuperBlockWidth);
            cmd.DW2.NextsliceMbStartYPosition    = CODECHAL_GET_HEIGHT_IN_BLOCKS((tileStartCtbY + tileHeight + 1), av1SuperBlockHeight);
        }
        cmd.DW1.FirstSuperSlice = 1;
     }

     MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

     return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Explictly instantiate AddVdencSrcSurfaceStateCmd of MhwVdboxVdencInterfaceG12 for mhw::vdbox::vdenc::xe_hpm::Cmd
//!         because VDENC_SRC_SURFACE_STATE_CMD has been changed for DG2
//!
template <> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencSrcSurfaceStateCmd(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->psSurface);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_SRC_SURFACE_STATE_CMD cmd;

    cmd.Dwords25.DW0.Width               = params->dwActualWidth - 1;
    cmd.Dwords25.DW0.Height              = params->dwActualHeight - 1;
    cmd.Dwords25.DW0.ColorSpaceSelection = params->bColorSpaceSelection;

    cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

    uint32_t tilemode         = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
    cmd.Dwords25.DW1.TileMode = tilemode;

    cmd.Dwords25.DW1.SurfaceFormat            = MosFormatToVdencSurfaceRawFormat(params->psSurface->Format);
    cmd.Dwords25.DW0.SurfaceFormatByteSwizzle = params->bDisplayFormatSwizzle;
    cmd.Dwords25.DW1.SurfacePitch             = params->psSurface->dwPitch - 1;
    cmd.Dwords25.DW2.YOffsetForUCb            = cmd.Dwords25.DW3.YOffsetForVCr =
        MOS_ALIGN_CEIL((params->psSurface->UPlaneOffset.iSurfaceOffset - params->psSurface->dwOffset) / params->psSurface->dwPitch + params->psSurface->RenderOffset.YUV.U.YOffset, MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9);

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Explictly instantiate AddVdencRefSurfaceStateCmd of MhwVdboxVdencInterfaceG12 for mhw::vdbox::vdenc::xe_hpm::Cmd
//!         because VDENC_REF_SURFACE_STATE_CMD has been changed for DG2
//!
template <> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencRefSurfaceStateCmd(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->psSurface);

    auto waTable = this->m_osInterface->pfnGetWaTable(this->m_osInterface);
    MHW_MI_CHK_NULL(waTable);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_REF_SURFACE_STATE_CMD cmd;

    if (params->bVdencDynamicScaling)
    {
        if (params->ucSurfaceStateId == CODECHAL_HCP_LAST_SURFACE_ID)
        {
            cmd.DW1.SurfaceId = 4;
        }
        else if (params->ucSurfaceStateId == CODECHAL_HCP_GOLDEN_SURFACE_ID)
        {
            cmd.DW1.SurfaceId = 5;
        }
        else if (params->ucSurfaceStateId == CODECHAL_HCP_ALTREF_SURFACE_ID)
        {
            cmd.DW1.SurfaceId = 6;
        }
    }

    if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0)
    {
        cmd.Dwords25.DW0.Width  = params->dwActualWidth - 1;
        cmd.Dwords25.DW0.Height = params->dwActualHeight - 1;
    }
    else
    {
        cmd.Dwords25.DW0.Width  = params->psSurface->dwWidth - 1;
        cmd.Dwords25.DW0.Height = params->psSurface->dwHeight - 1;
    }

    cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

    uint32_t tilemode         = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
    cmd.Dwords25.DW1.TileMode = tilemode;

    cmd.Dwords25.DW1.SurfaceFormat = MosFormatToVdencSurfaceReconFormat(params->psSurface->Format);

    if ((params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_MODE_VP9) && MEDIA_IS_WA(waTable, WaForceTile64ReconSurfaceToTile4) && params->psSurface->Format != Format_NV12 && tilemode == MOS_TILE_64_GMM)
    {
        cmd.Dwords25.DW1.TileMode = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_State_Fields_CMD::TILE_F;
    }

    if (cmd.Dwords25.DW1.SurfaceFormat == mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_P010)
    {
        cmd.Dwords25.DW1.SurfaceFormat = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_P010_VARIANT;
    }

    cmd.Dwords25.DW1.SurfacePitch  = params->psSurface->dwPitch - 1;
    cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr =
        (params->psSurface->UPlaneOffset.iSurfaceOffset - params->psSurface->dwOffset) / params->psSurface->dwPitch + params->psSurface->RenderOffset.YUV.U.YOffset;

    if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatY416Variant ||
        cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatAyuvVariant)
    {
        /* Y410/Y416 Reconstructed format handling */
        if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatY416Variant)
            cmd.Dwords25.DW1.SurfacePitch = params->psSurface->dwPitch / 2 - 1;
        /* AYUV Reconstructed format handling */
        if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatAyuvVariant)
            cmd.Dwords25.DW1.SurfacePitch = params->psSurface->dwPitch / 4 - 1;

        cmd.Dwords25.DW2.YOffsetForUCb = params->dwReconSurfHeight;
        cmd.Dwords25.DW3.YOffsetForVCr = params->dwReconSurfHeight << 1;
    }
    else if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatY216Variant ||
             cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatYuyvVariant)
    {
        cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr = params->dwReconSurfHeight;
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Explictly instantiate AddVdencDsRefSurfaceStateCmd of MhwVdboxVdencInterfaceG12 for mhw::vdbox::vdenc::xe_hpm::Cmd
//!         because VDENC_DS_REF_SURFACE_STATE_CMD has been changed for DG2
//!
template <> inline MOS_STATUS
MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>::AddVdencDsRefSurfaceStateCmd(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS params,
    uint8_t                   numSurfaces)
{
    uint32_t tilemode = 0;
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->psSurface);

    typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_DS_REF_SURFACE_STATE_CMD cmd;

    if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0)
    {
        cmd.Dwords25.DW0.Width  = params->dwActualWidth - 1;
        cmd.Dwords25.DW0.Height = params->dwActualHeight - 1;
    }
    else
    {
        cmd.Dwords25.DW0.Width  = params->psSurface->dwWidth - 1;
        cmd.Dwords25.DW0.Height = params->psSurface->dwHeight - 1;
    }
    cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

    tilemode                  = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
    cmd.Dwords25.DW1.TileMode = tilemode;

    cmd.Dwords25.DW1.SurfaceFormat = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8;
    cmd.Dwords25.DW1.SurfacePitch  = params->psSurface->dwPitch - 1;
    cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr =
        (params->psSurface->UPlaneOffset.iSurfaceOffset - params->psSurface->dwOffset) / params->psSurface->dwPitch + params->psSurface->RenderOffset.YUV.U.YOffset;

    // 2nd surface
    if (numSurfaces > 1)
    {
        params = params + 1;  // Increment pointer to move from 1st surface to 2nd surface.
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);

        if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0)
        {
            cmd.Dwords69.DW0.Width  = params->dwActualWidth - 1;
            cmd.Dwords69.DW0.Height = params->dwActualHeight - 1;
        }
        else
        {
            cmd.Dwords69.DW0.Width  = params->psSurface->dwWidth - 1;
            cmd.Dwords69.DW0.Height = params->psSurface->dwHeight - 1;
        }
        cmd.Dwords69.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

        tilemode                  = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
        cmd.Dwords69.DW1.TileMode = tilemode;

        cmd.Dwords69.DW1.SurfaceFormat = mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8;
        cmd.Dwords69.DW1.SurfacePitch  = params->psSurface->dwPitch - 1;
        cmd.Dwords69.DW2.YOffsetForUCb = cmd.Dwords69.DW3.YOffsetForVCr =
            (params->psSurface->UPlaneOffset.iSurfaceOffset - params->psSurface->dwOffset) / params->psSurface->dwPitch + params->psSurface->RenderOffset.YUV.U.YOffset;
    }

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

class MhwVdboxVdencInterfaceXe_Hpm : public MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>
{
public:

    //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceXe_Hpm(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceG12<mhw::vdbox::vdenc::xe_hpm::Cmd>(osInterface)
    {
        MHW_FUNCTION_ENTER;

        m_rhoDomainStatsEnabled = false;
        m_perfModeSupported = false;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceXe_Hpm()
    {
        MHW_FUNCTION_ENTER;
    }

    MOS_STATUS AddVdencAvcCostStateCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_BATCH_BUFFER         batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);

        typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_COSTS_STATE_CMD cmd;
        auto paramsDg2 = dynamic_cast<PMHW_VDBOX_AVC_IMG_PARAMS_XE_HPM>(params);
        auto pictureType    = paramsDg2->pEncodeAvcPicParams->CodingType - 1;
        auto codingType     = paramsDg2->pEncodeAvcPicParams->CodingType;
        auto isIPGOP        = paramsDg2->pEncodeAvcSeqParams->GopRefDist == 1;
        auto qp             = paramsDg2->pEncodeAvcPicParams->QpY +
            paramsDg2->pEncodeAvcSliceParams->slice_qp_delta;

        const auto &HmeCost          = paramsDg2->vdencCostTable->HmeCost;
        const auto &VDEnc_MV_Cost    = paramsDg2->vdencCostTable->VDEnc_MV_Cost;
        const auto &AVC_RD_Mode_Cost = paramsDg2->vdencCostTable->AVC_RD_Mode_Cost;
        const auto &VDEnc_Mode_Cost  = paramsDg2->vdencCostTable->VDEnc_Mode_Cost;
        const auto &qp_lambda        = paramsDg2->vdencCostTable->qp_lambda;

        int minQP = 10;
        int maxQP = 51;
        qp = qp > minQP ? qp : minQP;
        qp = qp < maxQP ? qp : maxQP;

        // Only do this lambda offset for lower resolution and high QP range.
        uint16_t gopP = (paramsDg2->pEncodeAvcSeqParams->GopRefDist) ? 
            ((paramsDg2->pEncodeAvcSeqParams->GopPicSize - 1) / paramsDg2->pEncodeAvcSeqParams->GopRefDist) : 0;
        uint16_t gopB = paramsDg2->pEncodeAvcSeqParams->GopPicSize - 1 - gopP;
        uint16_t numB = ((gopP > 0) ? (gopB / gopP) : 0);
        if ((paramsDg2->bVdencBRCEnabled == false) && (numB != 0))
        {
            int lambdaOffset = 0;
            if ((paramsDg2->wPicWidthInMb * 16 < 1920) && 
                (paramsDg2->wPicHeightInMb * 16 < 1080) && 
                (qp >= 32))
            {
                if (pictureType == 0)
                {
                    lambdaOffset = 0;
                }
                else if (pictureType == 1)
                {
                    lambdaOffset = 1;
                }
                else if (pictureType == 2)
                {
                    lambdaOffset = (paramsDg2->pEncodeAvcPicParams->RefPicFlag) ? 1 : 2;
                }
            }
            qp += lambdaOffset;
            qp = CodecHal_Clip3(0, 51, qp);
        }

        cmd.DW1_2.HmeMvCost0 = Map44LutValue((uint16_t)(HmeCost[0][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost1 = Map44LutValue((uint16_t)(HmeCost[1][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost2 = Map44LutValue((uint16_t)(HmeCost[2][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost3 = Map44LutValue((uint16_t)(HmeCost[3][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost4 = Map44LutValue((uint16_t)(HmeCost[4][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost5 = Map44LutValue((uint16_t)(HmeCost[5][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost6 = Map44LutValue((uint16_t)(HmeCost[6][qp]), 0x6f);
        cmd.DW1_2.HmeMvCost7 = Map44LutValue((uint16_t)(HmeCost[7][qp]), 0x6f);

        // Override costs for XE HPM for P and B frames
        if (codingType == P_TYPE || codingType == B_TYPE)
        {
            for (auto i = 0; i < 12; i++)
            {
                static const double sadmvcost_ptype[12] = { 1.0,   3.0,   5.0,   7.0,   9.0,  11.0,  13.0,  15.0,  17.0,  19.0,  21.0,  23.0 };
                static const double rdomvcost_ptype[12] = { 0.8,   2.4,   4.0,   5.6,   7.2,   8.8,  10.4,  12.0,  13.6,  15.2,  16.8,  18.4 };

                static const double sadmvcost_btype[12] = { 1.0,   3.0,   5.0,   7.0,   9.0,  11.0,  13.0,  15.0,  17.0,  19.0,  21.0,  23.0 };
                static const double rdomvcost_btype[12] = { 0.375, 1.125, 1.875, 2.625, 3.375, 4.125, 4.875, 5.625, 6.375, 7.125, 7.875, 8.625 };

                cmd.SadMvCost011[i] = GetU62ModeCost(codingType == P_TYPE ? sadmvcost_ptype[i]
                                                                          : sadmvcost_btype[i]);
                cmd.RdMvCost011[i]  = GetU62ModeCost(codingType == P_TYPE ? rdomvcost_ptype[i]
                                                                          : rdomvcost_btype[i]);
            }
        }
        else
        {
            for (auto i = 0; i < 12; i++)
            {
                cmd.SadMvCost011[i] = GetU62ModeCost(VDEnc_MV_Cost[pictureType][i]);
                cmd.RdMvCost011[i]  = cmd.SadMvCost011[i];
            }
        }

        // Override costs for XE HPM
        if (codingType == P_TYPE)
        {
            cmd.DW10.Skip16X16Cost  = GetU62ModeCost(0.0000);
            cmd.DW10.Merge16X16Cost = GetU62ModeCost(0.0000);

            cmd.DW12.ModeInter8X4   = GetU62ModeCost(isIPGOP ? 7.200 : 5.85);
            cmd.DW12.ModeInter16X16 = GetU62ModeCost(isIPGOP ? 1.500 : 1.60);
            cmd.DW12.ModeInter16X8  = GetU62ModeCost(isIPGOP ? 6.400 : 5.20);
            cmd.DW12.ModeInter8X8   = GetU62ModeCost(isIPGOP ? 4.000 : 3.25);

            cmd.DW13.ModeInterBidir = GetU62ModeCost(0.0);
            cmd.DW13.RefIdCost      = GetU62ModeCost(1.0);
            cmd.DW13.RdChromaIntraModeCost   = GetU62ModeCost(isIPGOP ? 0.0 : 0.5);
            cmd.DW13.SadChromaIntraModeCost  = GetU62ModeCost(isIPGOP ? 3.0 : 1.0);

            cmd.DW16.SadIntraNonPredModeCost = GetU71ModeCost(VDEnc_Mode_Cost[pictureType][LUTMODE_INTRA_NONPRED][qp]);

            cmd.DW17.RdModeIntraNonPred = GetU71ModeCost(3.0);
            cmd.DW17.ModeIntra4X4       = GetU71ModeCost(isIPGOP ? 26.400 : 22.00);
            cmd.DW17.ModeIntra16X16     = GetU71ModeCost(isIPGOP ?  8.000 :  8.00);
            cmd.DW17.ModeIntra8X8       = GetU71ModeCost(isIPGOP ? 16.000 :  9.00);
        }
        else if (codingType == B_TYPE)
        {

            cmd.DW10.Skip16X16Cost  = GetU62ModeCost(0.0000);
            cmd.DW10.Merge16X16Cost = GetU62ModeCost(0.0000);

            cmd.DW12.ModeInter8X4   = GetU62ModeCost(9.0750);
            cmd.DW12.ModeInter16X16 = GetU62ModeCost(3.0000);
            cmd.DW12.ModeInter16X8  = GetU62ModeCost(8.8000);
            cmd.DW12.ModeInter8X8   = GetU62ModeCost(6.0500);

            cmd.DW13.ModeInterBidir = GetU62ModeCost(1.0);
            cmd.DW13.RefIdCost      = GetU62ModeCost(1.0);
            cmd.DW13.RdChromaIntraModeCost   = GetU62ModeCost(0.5);
            cmd.DW13.SadChromaIntraModeCost  = GetU62ModeCost(1.0);

            cmd.DW16.SadIntraNonPredModeCost = GetU71ModeCost(7.0);

            cmd.DW17.RdModeIntraNonPred = GetU71ModeCost( 3.0);
            cmd.DW17.ModeIntra4X4       = GetU71ModeCost(20.8);
            cmd.DW17.ModeIntra16X16     = GetU71ModeCost( 8.0);
            cmd.DW17.ModeIntra8X8       = GetU71ModeCost(11.2);
        }
        else if (codingType == I_TYPE)
        {

            cmd.DW10.Skip16X16Cost  = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_SKIP_16X16]);
            cmd.DW10.Merge16X16Cost = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_DIRECT_16X16]);

            cmd.DW12.ModeInter8X4   = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_INTER_8X4]);
            cmd.DW12.ModeInter16X16 = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_INTER_16X16]);
            cmd.DW12.ModeInter16X8  = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_INTER_16X8]);
            cmd.DW12.ModeInter8X8   = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_INTER_8X8]);

            cmd.DW13.RdChromaIntraModeCost   = GetU62ModeCost(1.0);
            cmd.DW13.SadChromaIntraModeCost  = GetU62ModeCost(0.0);
            cmd.DW13.ModeInterBidir          = GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_INTER_BWD]);
            cmd.DW13.RefIdCost = paramsDg2->pEncodeAvcSliceParams->num_ref_idx_l0_active_minus1 == 0 ?
                0 : GetU62ModeCost(AVC_RD_Mode_Cost[pictureType][RD_MODE_REF_ID]);

            cmd.DW16.SadIntraNonPredModeCost = GetU71ModeCost(VDEnc_Mode_Cost[pictureType][LUTMODE_INTRA_NONPRED][qp]);

            cmd.DW17.RdModeIntraNonPred = GetU71ModeCost(3.0);
            cmd.DW17.ModeIntra4X4       = GetU71ModeCost(18.0000);
            cmd.DW17.ModeIntra16X16     = GetU71ModeCost(7.0000);
            cmd.DW17.ModeIntra8X8       = GetU71ModeCost(6.0000);
        }

        cmd.DW14.SadPenaltyForIntraNondc4X4PredMode   = 4;
        cmd.DW14.SadPenaltyForIntraNondc16X16PredMode = 36;
        cmd.DW14.SadPenaltyForIntraNondc8X8PredMode   = 12;

        cmd.DW15.RdPenaltyForIntraDc4X4PredMode      = 0;
        cmd.DW15.RdPenaltyForIntraNondc16X16PredMode = 0;
        cmd.DW15.RdPenaltyForIntraNondc8X8PredMode   = 0;

        cmd.DW19.TuDepthCost0 = 0;
        cmd.DW19.TuDepthCost1 = 0;
        cmd.DW19.TuDepthCost2 = 0;

        cmd.DW20.IntraTu4X4CbfCost = 0;
        cmd.DW20.IntraTu8X8CbfCost = 0;

        cmd.DW21.InterTu4X4CbfCost = 0;
        cmd.DW21.InterTu8X8CbfCost = 0;

        uint16_t RdQpLambda, SadQpLambda;
        uint8_t  idx12 = uint8_t(qp > 12 ? qp - 12 : 0);
        double   rdQpLambdaCoeff  = 4.0 * pow(2.0, idx12 / 3.0);
        double   sadQpLambdaCoeff = 4.0 * pow(2.0, idx12 / 6.0);
        if (codingType == P_TYPE)
        {
            if (isIPGOP)
            {
                RdQpLambda  = (uint16_t)(0.80 * rdQpLambdaCoeff);
                SadQpLambda = (uint16_t)(0.90 * sadQpLambdaCoeff);
            }
            else
            {
                RdQpLambda  = (uint16_t)(0.50 * rdQpLambdaCoeff);
                SadQpLambda = (uint16_t)(0.75 * sadQpLambdaCoeff);
            }
        }
        else if (codingType == B_TYPE)
        {
            if (paramsDg2->pEncodeAvcPicParams->RefPicFlag)
            {
                RdQpLambda  = (uint16_t)(0.90 * rdQpLambdaCoeff);
                SadQpLambda = (uint16_t)(0.90 * sadQpLambdaCoeff);
            }
            else
            {
                RdQpLambda  = (uint16_t)(1.20 * rdQpLambdaCoeff);
                SadQpLambda = (uint16_t)(1.10 * sadQpLambdaCoeff);
            }
        }
        else
        {
            RdQpLambda  = (uint16_t)(0.65 * rdQpLambdaCoeff);
            SadQpLambda = (uint16_t)(0.65 * sadQpLambdaCoeff);
        }

        cmd.DW22.RdQpLambda  = RdQpLambda;
        cmd.DW22.SadQpLambda = SadQpLambda;

        if (params->bPerMBStreamOut)
        {
            // Force the Intra cost as maximium to force intra MB as Inter MB for all P frames.
            // This is for PerMB StreamOut PAK statistics usage.
            cmd.DW13.RdChromaIntraModeCost = 155;
            cmd.DW13.SadChromaIntraModeCost = 155;
            cmd.DW14.SadPenaltyForIntraNondc4X4PredMode = 155;
            cmd.DW14.SadPenaltyForIntraNondc16X16PredMode = 155;
            cmd.DW14.SadPenaltyForIntraNondc8X8PredMode = 155;
            cmd.DW15.RdPenaltyForIntraDc4X4PredMode = 155;
            cmd.DW15.RdPenaltyForIntraNondc16X16PredMode = 155;
            cmd.DW15.RdPenaltyForIntraNondc8X8PredMode = 155;
            cmd.DW16.SadIntraNonPredModeCost = 155;
            cmd.DW17.RdModeIntraNonPred = 155;
            cmd.DW17.ModeIntra4X4 = 155;
            cmd.DW17.ModeIntra16X16 = 155;
            cmd.DW17.ModeIntra8X8 = 155;
            cmd.DW20.IntraTu4X4CbfCost = 155;
            cmd.DW20.IntraTu8X8CbfCost = 155;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencSliceStateCmd(
        PMOS_COMMAND_BUFFER        cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pEncodeAvcSliceParams);

        typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_AVC_SLICE_STATE_CMD cmd;

        auto sliceParams = params->pEncodeAvcSliceParams;
        auto picParams   = params->pEncodeAvcPicParams;

        cmd.DW1.RoundIntra       = params->dwRoundingIntraValue;
        cmd.DW1.RoundIntraEnable = 1;
        if (sliceParams->slice_type == 2 || sliceParams->slice_type == 4
            || sliceParams->slice_type == 7 || sliceParams->slice_type == 9) // I slice
        {
            cmd.DW1.RoundInter       = 2;
            cmd.DW1.RoundInterEnable = 0;
        }
        else // P/B slice
        {
            cmd.DW1.RoundInter       = params->dwRoundingValue;
            cmd.DW1.RoundInterEnable = params->bRoundingInterEnable;
        }

        cmd.DW3.Log2WeightDenomLuma = sliceParams->luma_log2_weight_denom;
        if (sliceParams->slice_type == 1 || sliceParams->slice_type == 6) // B slice
        {
            if (picParams->weighted_bipred_idc == IMPLICIT_WEIGHTED_INTER_PRED_MODE)
            {
                cmd.DW3.Log2WeightDenomLuma = 0;
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencControlStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer) override
    {
        typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_CONTROL_STATE_CMD cmd;

        cmd.DW1.VdencInitialization = 1;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencHevcTileSliceStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VDENC_HEVC_VP9_TILE_SLICE_STATE_PARAMS tileSlcParams)
    {
        MHW_FUNCTION_ENTER;

        typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_HEVC_VP9_TILE_SLICE_STATE_CMD cmd;

        MHW_MI_CHK_NULL(tileSlcParams);

        cmd.DW3.Log2WeightDenomLuma = cmd.DW3.HevcVp9Log2WeightDenomLuma = tileSlcParams->Log2WeightDenomLuma;
        cmd.DW3.Log2WeightDenomChroma = tileSlcParams->Log2WeightDenomChroma;

        if (tileSlcParams->pTileCodingParams == nullptr)
        {
            // No tiling support
            cmd.DW5.TileWidth  = tileSlcParams->widthInPix - 1;
            cmd.DW5.TileHeight = tileSlcParams->heightInPix - 1;
        }
        else
        {
            cmd.DW4.TileStartCtbX = tileSlcParams->pTileCodingParams->TileStartLCUX * tileSlcParams->ctbSize;
            cmd.DW4.TileStartCtbY = tileSlcParams->pTileCodingParams->TileStartLCUY * tileSlcParams->ctbSize;

            cmd.DW5.TileWidth                 = ((tileSlcParams->pTileCodingParams->TileWidthInMinCbMinus1 + 1) << tileSlcParams->minCodingBlkSize) - 1;
            cmd.DW5.TileHeight                = ((tileSlcParams->pTileCodingParams->TileHeightInMinCbMinus1 + 1) << tileSlcParams->minCodingBlkSize) - 1;

            // NumParEngine is not used by HW
            cmd.DW3.NumParEngine              = tileSlcParams->dwNumberOfPipes;

            cmd.DW3.TileNumber                = tileSlcParams->dwTileId;
            cmd.DW3.TileRowStoreSelect        = tileSlcParams->pTileCodingParams->TileRowStoreSelect;
            cmd.DW8.TileStreamoutOffsetEnable = 1;
            cmd.DW8.TileStreamoutOffset       = tileSlcParams->dwTileId * 19;

            cmd.DW6.StreaminOffsetEnable = 1;
            cmd.DW6.TileStreaminOffset   = tileSlcParams->pTileCodingParams->TileStreaminOffset;

            // PAK Object StreamOut Offset Computation
            uint32_t tileLCUStreamOutByteOffset = 0;
            if (tileSlcParams->pTileCodingParams->TileStartLCUX != 0 || tileSlcParams->pTileCodingParams->TileStartLCUY != 0)
            {
                uint32_t ctbSize            = tileSlcParams->ctbSize;
                uint32_t NumOfCUInLCU       = (ctbSize >> 3) * (ctbSize >> 3);  // Min CU size is 8
                uint32_t ImgWidthInLCU      = (tileSlcParams->widthInPix + ctbSize - 1) / ctbSize;
                uint32_t ImgHeightInLCU     = (tileSlcParams->heightInPix + ctbSize - 1) / ctbSize;
                uint32_t NumLCUsCurLocation = tileSlcParams->pTileCodingParams->TileStartLCUY * ImgWidthInLCU + tileSlcParams->pTileCodingParams->TileStartLCUX *
                    ((((tileSlcParams->pTileCodingParams->TileHeightInMinCbMinus1 + 1) << tileSlcParams->minCodingBlkSize) + ctbSize - 1) / ctbSize);
                //For PAKObject Surface
                tileLCUStreamOutByteOffset = 2 * BYTES_PER_DWORD * NumLCUsCurLocation * (NUM_PAK_DWS_PER_LCU + NumOfCUInLCU * NUM_DWS_PER_CU);
                //Add 1 CL for size info at the beginning of each tile
                tileLCUStreamOutByteOffset += MHW_CACHELINE_SIZE;
                //CL alignment at end of every tile
                tileLCUStreamOutByteOffset = MOS_ROUNDUP_DIVIDE(tileLCUStreamOutByteOffset, MHW_CACHELINE_SIZE);
            }

            cmd.DW9.TileLcuStreamOutOffset = tileLCUStreamOutByteOffset;
            cmd.DW9.LcuStreamOutOffsetEnable = 0x1;

            if (cmd.DW4.TileStartCtbY == 0)
            {
                //RowStore Offset Computation
                uint32_t num32x32InX         = (cmd.DW4.TileStartCtbX) / 32;
                cmd.DW7.RowStoreOffsetEnable = 1;
                cmd.DW7.TileRowstoreOffset   = num32x32InX;
            }
        }

        // For IBC
        // DW 11
        // Set to 0 by default, refine later
        cmd.DW11.Reserved360 = 0;

        // DW 12
        cmd.DW12.IbcControl = tileSlcParams->IBCControl;

        // For palette mode
        // DW12
        cmd.DW12.PaletteModeEnable     = tileSlcParams->PaletteModeEnable;
        cmd.DW12.Reserved408           = 1;  //to be aligned with C model

        // Get the slice QP
        uint32_t sliceQP = tileSlcParams->sliceQP;
        MHW_ASSERT(cmd.DW12.Reserved408 < 3);
        uint32_t index;
        // Get index with slice QP
        if (sliceQP <= 12)
        {
            index = 0;
        }
        else if (sliceQP > 12 && sliceQP <= 17)
        {
            index = 1;
        }
        else if (sliceQP > 17 && sliceQP <= 22)
        {
            index = 2;
        }
        else if (sliceQP > 22 && sliceQP <= 27)
        {
            index = 3;
        }
        else if (sliceQP > 27 && sliceQP <= 32)
        {
            index = 4;
        }
        else if (sliceQP > 32 && sliceQP <= 37)
        {
            index = 5;
        }
        else if (sliceQP > 37 && sliceQP <= 42)
        {
            index = 6;
        }
        else if (sliceQP > 42 && sliceQP <= 47)
        {
            index = 7;
        }
        else if (sliceQP > 47 && sliceQP <= 49)
        {
            index = 8;
        }
        else
        {
            index = 9;
        }

        const uint32_t table1[10] = {0x50001, 0x50001, 0x50001, 0x50002, 0x90002, 0x90002, 0x90002, 0xd0002, 0x190002, 0x210003};
        cmd.DW12.Value            = (cmd.DW12.Value & 0xff80fff8) | table1[index];
        const uint32_t table2[10] = {0x2000a, 0x2000a, 0x2000a, 0x4000a, 0x8000a, 0xc0010, 0xc0018, 0xc0018, 0x100020, 0x180030};
        cmd.DW13.Value            = table2[index];
        const uint32_t table3[10] = {0x101004, 0x101004, 0x101004, 0xc1008, 0x42004, 0x42006, 0x13f06, 0x13f06, 0x13f0c, 0x13006};
        cmd.DW14.Value            = (cmd.DW14.Value & 0xffe0c0c0) | table3[index];
        const uint32_t table4[10] = {0x100004, 0x100004, 0x100004, 0x100004, 0x200004, 0x300004, 0x400004, 0x600004, 0x800004, 0x1000004};
        cmd.DW15.Value            = (cmd.DW15.Value & 0xfc00) | table4[index];

        if (tileSlcParams->bit_depth_luma_minus8 > 0 && tileSlcParams->PaletteModeEnable)
        {
            uint32_t shift = tileSlcParams->bit_depth_luma_minus8;
            cmd.DW12.Reserved384 += shift;
            cmd.DW13 = (cmd.DW13 & 0x3FF03FF << shift) & 0x3FF03FF;
            if (cmd.DW15.Reserved496 >= 256)            {
                cmd.DW15.Reserved496 = 255;
            }
            cmd.DW15.Reserved496 <<= shift;
        }

        cmd.DW12.Reserved392 = 0;

        cmd.DW14.Value = (cmd.DW14.Value & 0x9fffff) | 0xc8400000;
        cmd.DW16.Value = (cmd.DW16.Value & 0xffffff) | 0xa6000000;
        cmd.DW16.ReservedForAdaptiveChannelThreshold = 6;

        //aligned with C model for speed mode
        if (tileSlcParams->TargetUsage == 7)
        {
            cmd.DW16.Value = (cmd.DW16.Value & 0xffc0c0c0) | 0x313131;
        }
        else
        {
            cmd.DW16.Value = (cmd.DW16.Value & 0xffc0c0c0) | 0x3f3f3f;
        }

        cmd.DW19.Reserved608 = tileSlcParams->RowStaticInfo_31_0;
        cmd.DW20.Reserved640 = tileSlcParams->RowStaticInfo_63_32;
        cmd.DW21.Reserved672 = tileSlcParams->RowStaticInfo_95_64;
        cmd.DW22.Reserved704 = tileSlcParams->RowStaticInfo_127_96;
        cmd.DW23.Reserved736 = tileSlcParams->RowStaticInfo_31_0;
        cmd.DW24.Reserved768 = tileSlcParams->RowStaticInfo_63_32;
        cmd.DW25.Reserved800 = tileSlcParams->RowStaticInfo_95_64;
        cmd.DW26.Reserved832 = tileSlcParams->RowStaticInfo_127_96;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencVp9TileSliceStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 params
    )
    {
        MHW_FUNCTION_ENTER;
        typename mhw::vdbox::vdenc::xe_hpm::Cmd::VDENC_HEVC_VP9_TILE_SLICE_STATE_CMD cmd;

        auto paramsG12 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12>(params);
        MHW_MI_CHK_NULL(paramsG12);

        //Tile height programmed should be n * 8 + 7, where n is integer greater than or equal to 15.
        //In other words it tile height is greater than or equal to 128 and it should be multiple of 8 pixels.
        //Maximum height of a tile is 8191
        auto tileCodingParams = paramsG12->pTileCodingParams;
        MHW_MI_CHK_NULL(params->pVp9EncPicParams);
        auto vp9PicParams = params->pVp9EncPicParams;

        if (tileCodingParams == nullptr)
        {
            cmd.DW5.TileWidth = vp9PicParams->SrcFrameWidthMinus1;
            cmd.DW5.TileHeight = vp9PicParams->SrcFrameHeightMinus1;
        }
        else
        {
            cmd.DW5.TileWidth  = ((tileCodingParams->TileWidthInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
            cmd.DW5.TileHeight = ((tileCodingParams->TileHeightInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

            cmd.DW4.TileStartCtbX = tileCodingParams->TileStartLCUX * CODEC_VP9_SUPER_BLOCK_WIDTH;
            cmd.DW4.TileStartCtbY = tileCodingParams->TileStartLCUY * CODEC_VP9_SUPER_BLOCK_HEIGHT;

            cmd.DW3.NumParEngine    = paramsG12->dwNumberOfPipes;
            cmd.DW3.TileNumber      = paramsG12->dwTileId;

            cmd.DW6.StreaminOffsetEnable = 1;
            cmd.DW6.TileStreaminOffset   = tileCodingParams->TileStreaminOffset;

            //Frame Stats Offset
            cmd.DW8.TileStreamoutOffsetEnable = 1;
            cmd.DW8.TileStreamoutOffset       = paramsG12->dwTileId * 19;  // 3 CLs or 48 DWs of statistics data + 16CLs or 256 DWs of Histogram data

            if (cmd.DW4.TileStartCtbY == 0)
            {
                //RowStore Offset Computation
                uint32_t num32x32sInX        = (cmd.DW4.TileStartCtbX) / 32;
                cmd.DW7.RowStoreOffsetEnable = 1;
                cmd.DW7.TileRowstoreOffset   = num32x32sInX;
            }

            cmd.DW9.LcuStreamOutOffsetEnable = 1;
            cmd.DW9.TileLcuStreamOutOffset   = tileCodingParams->TileLCUStreamOutOffset;

            cmd.DW17.Reserved544 = 1;
            cmd.DW17.Reserved550 = tileCodingParams->CumulativeCUTileOffset;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));
        return MOS_STATUS_SUCCESS;
    }
};

#endif // IGFX_VDENC_INTERFACE_EXT_SUPPORT
#endif // __MHW_VDBOX_VDENC_XE_HPM_H__
