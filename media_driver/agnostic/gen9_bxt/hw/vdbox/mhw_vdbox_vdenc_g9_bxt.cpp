/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_g9_bxt.cpp
//! \brief    Defines functions for constructing Vdbox Vdenc commands on G9 BXT
//!

#include "mhw_vdbox_vdenc_g9_bxt.h"

MOS_STATUS MhwVdboxVdencInterfaceG9Bxt::AddVdencSrcSurfaceStateCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS            params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->psSurface);

    mhw_vdbox_vdenc_g9_bxt::VDENC_SRC_SURFACE_STATE_CMD  cmd;

    cmd.Dwords25.DW0.Width                       = params->psSurface->dwWidth - 1;
    cmd.Dwords25.DW0.Height                      = params->psSurface->dwHeight - 1;
    cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;
    cmd.Dwords25.DW1.TiledSurface                = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

    if (cmd.Dwords25.DW1.TiledSurface)
    {
        cmd.Dwords25.DW1.TileWalk = (params->psSurface->TileType);
    }
    cmd.Dwords25.DW1.SurfaceFormat    = MosToMediaStateFormat(params->psSurface->Format); //dwSurfaceFormat;  should be 4
    cmd.Dwords25.DW1.InterleaveChroma = 1;
    cmd.Dwords25.DW1.SurfacePitch     = params->psSurface->dwPitch - 1;
    cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr =
        MOS_ALIGN_CEIL(params->psSurface->UPlaneOffset.iYOffset, MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9);

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxVdencInterfaceG9Bxt::AddVdencImgStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_AVC_IMG_PARAMS        params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pEncodeAvcSeqParams);
    MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);

    mhw_vdbox_vdenc_g9_bxt::VDENC_IMG_STATE_CMD  cmd;

    auto avcSeqParams   = params->pEncodeAvcSeqParams;
    auto avcPicParams   = params->pEncodeAvcPicParams;
    auto avcSliceParams = params->pEncodeAvcSliceParams;

    // initialize
    cmd.DW2.UnidirectionalMixDisable             = false;
    cmd.DW4.IntraSadMeasureAdjustment            = 2;
    cmd.DW8.BilinearFilterEnable                 = false;
    cmd.DW9.Mode0Cost                            = 10;
    cmd.DW9.Mode1Cost                            = 0;
    cmd.DW9.Mode2Cost                            = 3;
    cmd.DW9.Mode3Cost                            = 30;
    cmd.DW20.PenaltyForIntra16X16NondcPrediction = 36;
    cmd.DW20.PenaltyForIntra8X8NondcPrediction   = 12;
    cmd.DW20.PenaltyForIntra4X4NondcPrediction   = 4;
    cmd.DW22.Smallmbsizeinword                   = 0xff;
    cmd.DW22.Largembsizeinword                   = 0xff;
    cmd.DW27.MaxHmvR                             = 0x2000;
    cmd.DW27.MaxVmvR                             = 0x200;
    cmd.DW33.Maxdeltaqp                          = 0x0f;

    // initialize for P frame
    if (avcPicParams->CodingType != I_TYPE)
    {
        cmd.DW2.BidirectionalWeight           = 0x20;
        cmd.DW4.SubPelMode                    = 3;
        cmd.DW4.BmeDisableForFbrMessage       = 1;
        cmd.DW4.InterSadMeasureAdjustment     = 2;
        cmd.DW4.SubMacroblockSubPartitionMask = 0x70;
        cmd.DW5.CrePrefetchEnable             = 1;
        cmd.DW8.NonSkipZeroMvCostAdded        = 1;
        cmd.DW8.NonSkipMbModeCostAdded        = 1;
        cmd.DW9.Mode0Cost                     = 7;
        cmd.DW9.Mode1Cost                     = 26;
        cmd.DW9.Mode2Cost                     = 30;
        cmd.DW9.Mode3Cost                     = 57;
        cmd.DW10.Mode4Cost                    = 8;
        cmd.DW10.Mode5Cost                    = 2;
        cmd.DW10.Mode6Cost                    = 4;
        cmd.DW10.Mode7Cost                    = 6;
        cmd.DW11.Mode8Cost                    = 5;
        cmd.DW11.Mode9Cost                    = 0;
        cmd.DW11.RefIdCost                    = 4;
        cmd.DW12.MvCost0                      = 0;
        cmd.DW12.MvCost1                      = 6;
        cmd.DW12.MvCost2                      = 6;
        cmd.DW12.MvCost3                      = 9;
        cmd.DW13.MvCost4                      = 10;
        cmd.DW13.MvCost5                      = 13;
        cmd.DW13.MvCost6                      = 14;
        cmd.DW13.MvCost7                      = 24;
        cmd.DW31.SadHaarThreshold0            = 800;
        cmd.DW32.SadHaarThreshold1            = 1600;
        cmd.DW32.SadHaarThreshold2            = 2400;
        cmd.DW34.MidpointSadHaar              = 0x640;
    }

    cmd.DW1.VdencPerfmode                   = params->bVDEncPerfModeEnabled;
    cmd.DW1.Transform8X8Flag                = avcPicParams->transform_8x8_mode_flag;
    cmd.DW3.PictureWidth                    = params->wPicWidthInMb;
    cmd.DW4.ForwardTransformSkipCheckEnable = m_vdencFTQEnabled[avcSeqParams->TargetUsage];
    cmd.DW4.BlockBasedSkipEnabled           = m_vdencBlockBasedSkipEnabled[avcSeqParams->TargetUsage];
    cmd.DW5.CrePrefetchEnable               = params->bCrePrefetchEnable;
    cmd.DW5.PictureHeightMinusOne           = params->wPicHeightInMb - 1;
    cmd.DW5.PictureType                     = avcPicParams->CodingType - 1;
    cmd.DW5.ConstrainedIntraPredictionFlag  = avcPicParams->constrained_intra_pred_flag;

    // HME Ref1 Disable should be set as 0 when VDEnc Perf Mode is enabled
    if ((avcPicParams->CodingType != I_TYPE) &&
        (!params->pEncodeAvcSliceParams->num_ref_idx_l0_active_minus1) &&
        (!params->bVDEncPerfModeEnabled))
    {
        cmd.DW5.HmeRef1Disable = true;
    }

    if (avcSeqParams->EnableSliceLevelRateCtrl)
    {
        cmd.DW5.MbSliceThresholdValue = params->dwMbSlcThresholdValue;
    }

    cmd.DW6.SliceMacroblockHeightMinusOne = params->wSlcHeightInMb - 1;

    cmd.DW8.LumaIntraPartitionMask = avcPicParams->transform_8x8_mode_flag ? 0 : mhw_vdbox_vdenc_g9_bxt::VDENC_IMG_STATE_CMD::LUMA_INTRA_PARTITION_MASK_UNNAMED2;

    cmd.DW14.QpPrimeY = avcPicParams->QpY + avcSliceParams->slice_qp_delta;

    if (params->pVDEncModeCost)
    {
        cmd.DW9.Mode0Cost  = *(params->pVDEncModeCost);
        cmd.DW9.Mode1Cost  = *(params->pVDEncModeCost + 1);
        cmd.DW9.Mode2Cost  = *(params->pVDEncModeCost + 2);
        cmd.DW9.Mode3Cost  = *(params->pVDEncModeCost + 3);

        cmd.DW10.Mode4Cost = *(params->pVDEncModeCost + 4);
        cmd.DW10.Mode5Cost = *(params->pVDEncModeCost + 5);
        cmd.DW10.Mode6Cost = *(params->pVDEncModeCost + 6);
        cmd.DW10.Mode7Cost = *(params->pVDEncModeCost + 7);

        cmd.DW11.Mode8Cost = *(params->pVDEncModeCost + 8);
        cmd.DW11.RefIdCost = *(params->pVDEncModeCost + 10);
    }
    if (params->pVDEncMvCost)
    {
        cmd.DW12.MvCost0 = *(params->pVDEncMvCost);
        cmd.DW12.MvCost1 = *(params->pVDEncMvCost + 1);
        cmd.DW12.MvCost2 = *(params->pVDEncMvCost + 2);
        cmd.DW12.MvCost3 = *(params->pVDEncMvCost + 3);
        cmd.DW13.MvCost4 = *(params->pVDEncMvCost + 4);
        cmd.DW13.MvCost5 = *(params->pVDEncMvCost + 5);
        cmd.DW13.MvCost6 = *(params->pVDEncMvCost + 6);
        cmd.DW13.MvCost7 = *(params->pVDEncMvCost + 7);
    }

    cmd.DW27.MaxVmvR = params->dwMaxVmvR;

    if (params->pVDEncHmeMvCost)
    {
        cmd.DW28.HmeMvCost0 = *(params->pVDEncHmeMvCost);
        cmd.DW28.HmeMvCost1 = *(params->pVDEncHmeMvCost + 1);
        cmd.DW28.HmeMvCost2 = *(params->pVDEncHmeMvCost + 2);
        cmd.DW28.HmeMvCost3 = *(params->pVDEncHmeMvCost + 3);
        cmd.DW29.HmeMvCost4 = *(params->pVDEncHmeMvCost + 4);
        cmd.DW29.HmeMvCost5 = *(params->pVDEncHmeMvCost + 5);
        cmd.DW29.HmeMvCost6 = *(params->pVDEncHmeMvCost + 6);
        cmd.DW29.HmeMvCost7 = *(params->pVDEncHmeMvCost + 7);
    }

    cmd.DW34.ImageStateQpOverride = avcSeqParams->RateControlMethod == RATECONTROL_CQP ? 1 : 0;

    // Rolling-I settings
    if ((avcPicParams->CodingType != I_TYPE) && (avcPicParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED))
    {
        cmd.DW1.VdencExtendedPakObjCmdEnable      = 1;
        cmd.DW21.IntraRefreshEnableRollingIEnable = avcPicParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED ? 1 : 0;
        cmd.DW21.IntraRefreshMode                 = avcPicParams->EnableRollingIntraRefresh == ROLLING_I_ROW ? 0 : 1;        // 0->Row based ; 1->Column based
        cmd.DW21.IntraRefreshMBPos                = avcPicParams->IntraRefreshMBNum;
        cmd.DW21.IntraRefreshMBSizeMinusOne       = avcPicParams->IntraRefreshUnitinMB;
        cmd.DW21.QpAdjustmentForRollingI          = avcPicParams->IntraRefreshQPDelta;

        auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
        MHW_MI_CHK_NULL(waTable);

        // WA to prevent error propagation from top-right direction.
        // Disable prediction modes 3, 7 for 4x4
        // and modes 0, 2, 3, 4, 5, 7 for 8x8 (due to filtering)
        if (avcPicParams->EnableRollingIntraRefresh == ROLLING_I_COLUMN &&
            MEDIA_IS_WA(waTable, Wa_18011246551))
        {
            cmd.DW17.AvcIntra4X4ModeMask = 0x88;
            cmd.DW17.AvcIntra8X8ModeMask = 0xBD;
        }
    }

    // Setting MinMaxQP values if they are presented
    if (avcPicParams->ucMaximumQP && avcPicParams->ucMinimumQP)
    {
        cmd.DW33.MaxQp = avcPicParams->ucMaximumQP;
        cmd.DW33.MinQp = avcPicParams->ucMinimumQP;
    }
    else
    {
        // Set default values
        cmd.DW33.MaxQp = 0x33;
        cmd.DW33.MinQp = 0x0a;
    }

    // VDEnc CQP case ROI settings, BRC ROI will be handled in HuC FW
    if (!params->bVdencBRCEnabled && avcPicParams->NumROI)
    {
        MHW_ASSERT(avcPicParams->NumROI < 4);

        int8_t priorityLevelOrDQp[ENCODE_VDENC_AVC_MAX_ROI_NUMBER_G9] = { 0 };

        for (uint8_t i = 0; i < avcPicParams->NumROI; i++)
        {
            int8_t dQpRoi = avcPicParams->ROIDistinctDeltaQp[i];

            // clip delta qp roi to VDEnc supported range
            priorityLevelOrDQp[i] = (char)CodecHal_Clip3(
                ENCODE_VDENC_AVC_MIN_ROI_DELTA_QP_G9, ENCODE_VDENC_AVC_MAX_ROI_DELTA_QP_G9, dQpRoi);
        }

        cmd.DW34.RoiEnable = true;

        // Zone0 is reserved for non-ROI region
        cmd.DW30.RoiQpAdjustmentForZone1 = priorityLevelOrDQp[0];
        cmd.DW30.RoiQpAdjustmentForZone2 = priorityLevelOrDQp[1];
        cmd.DW30.RoiQpAdjustmentForZone3 = priorityLevelOrDQp[2];
    }

    if (params->bVdencBRCEnabled && avcPicParams->NumDirtyROI && params->bVdencStreamInEnabled)
    {
        cmd.DW34.RoiEnable = true;
    }

    if (params->bVdencStreamInEnabled)
    {
        cmd.DW34.FwdPredictor0MvEnable = 1;
        cmd.DW34.PpmvDisable           = 1;
    }

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxVdencInterfaceG9Bxt::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_vdenc_g9_bxt::VDENC_WALKER_STATE_CMD  cmd;

    // MB start X/Y posistion set to 0
    cmd.DW1.MbLcuStartXPosition = 0;
    cmd.DW1.MbLcuStartYPosition = 0;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}
