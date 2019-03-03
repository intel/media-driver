/*
* Copyright (c) 2018, Intel Corporation
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
//! \file   mhw_vebox_hwcmd_g9_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_vebox_hwcmd_g9_X.h"
#include "mos_utilities.h"

mhw_vebox_g9_X::VEBOX_ACE_LACE_STATE_CMD::VEBOX_ACE_LACE_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.SkinThreshold                                = SKIN_THRESHOLD_UNNAMED26;
    DW0.LaceHistogramEnable                          = LACE_HISTOGRAM_ENABLE_UNNAMED0;
    DW0.LaceHistogramSize                            = LACE_HISTOGRAM_SIZE_128_BINHISTOGRAM;
    DW0.LaceSingleHistogramSet                       = LACE_SINGLE_HISTOGRAM_SET_CURRENT;

    DW1.Value                                        = 0;
    DW1.Ymin                                         = YMIN_UNNAMED16;
    DW1.Y1                                           = Y1_UNNAMED36;
    DW1.Y2                                           = Y2_UNNAMED56;
    DW1.Y3                                           = Y3_UNNAMED76;

    DW2.Value                                        = 0;
    DW2.Y4                                           = Y4_UNNAMED96;
    DW2.Y5                                           = Y5_UNNAMED116;
    DW2.Y6                                           = Y6_UNNAMED136;
    DW2.Y7                                           = Y7_UNNAMED156;

    DW3.Value                                        = 0;
    DW3.Y8                                           = Y8_UNNAMED176;
    DW3.Y9                                           = Y9_UNNAMED196;
    DW3.Y10                                          = Y10_UNNAMED216;
    DW3.Ymax                                         = YMAX_UNNAMED235;

    DW4.Value                                        = 0;
    DW4.B1                                           = B1_UNNAMED36;
    DW4.B2                                           = B2_UNNAMED56;
    DW4.B3                                           = B3_UNNAMED76;
    DW4.B4                                           = B4_UNNAMED96;

    DW5.Value                                        = 0;
    DW5.B5                                           = B5_UNNAMED116;
    DW5.B6                                           = B6_UNNAMED136;
    DW5.B7                                           = B7_UNNAMED156;
    DW5.B8                                           = B8_UNNAMED176;

    DW6.Value                                        = 0;
    DW6.B9                                           = B9_UNNAMED196;
    DW6.B10                                          = B10_UNNAMED216;

    DW7.Value                                        = 0;
    DW7.S0                                           = S0_UNNAMED1024;
    DW7.S1                                           = S1_UNNAMED1024;

    DW8.Value                                        = 0;
    DW8.S2                                           = S2_UNNAMED1024;
    DW8.S3                                           = S3_UNNAMED1024;

    DW9.Value                                        = 0;
    DW9.S4                                           = S4_UNNAMED1024;
    DW9.S5                                           = S5_UNNAMED1024;

    DW10.Value                                       = 0;
    DW10.S6                                          = S6_UNNAMED1024;
    DW10.S7                                          = S7_UNNAMED1024;

    DW11.Value                                       = 0;
    DW11.S8                                          = S8_UNNAMED1024;
    DW11.S9                                          = S9_UNNAMED1024;

    DW12.Value                                       = 0;
    DW12.S10                                         = S10_UNNAMED1024;

}

mhw_vebox_g9_X::VEBOX_ALPHA_AOI_STATE_CMD::VEBOX_ALPHA_AOI_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.AlphaFromStateSelect                         = ALPHA_FROM_STATE_SELECT_ALPHAISTAKENFROMMESSAGE;
    DW0.FullImageHistogram                           = FULL_IMAGE_HISTOGRAM_UNNAMED0;

    DW1.Value                                        = 0;
    DW1.AoiMinX                                      = AOI_MIN_X_UNNAMED0;
    DW1.AoiMaxX                                      = AOI_MAX_X_UNNAMED3;

    DW2.Value                                        = 0;
    DW2.AoiMinY                                      = AOI_MIN_Y_UNNAMED0;
    DW2.AoiMaxY                                      = AOI_MAX_Y_UNNAMED3;

}

mhw_vebox_g9_X::VEBOX_CAPTURE_PIPE_STATE_CMD::VEBOX_CAPTURE_PIPE_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.GoodPixelNeighborThreshold                   = GOOD_PIXEL_NEIGHBOR_THRESHOLD_UNNAMED35;
    DW0.AverageColorThreshold                        = AVERAGE_COLOR_THRESHOLD_UNNAMED255;
    DW0.GreenImbalanceThreshold                      = GREEN_IMBALANCE_THRESHOLD_UNNAMED1;
    DW0.ShiftMinCost                                 = SHIFT_MIN_COST_UNNAMED1;
    DW0.GoodPixelThreshold                           = GOOD_PIXEL_THRESHOLD_UNNAMED5;

    DW1.Value                                        = 0;
    DW1.BadColorThreshold3                           = BAD_COLOR_THRESHOLD_3_UNNAMED10;
    DW1.NumberBigPixelThreshold                      = NUMBER_BIG_PIXEL_THRESHOLD_UNNAMED10;
    DW1.BadColorThreshold2                           = BAD_COLOR_THRESHOLD_2_UNNAMED175;
    DW1.BadColorThreshold1                           = BAD_COLOR_THRESHOLD_1_UNNAMED100;
    DW1.GoodIntesityThreshold                        = GOOD_INTESITY_THRESHOLD_UNNAMED10;
    DW1.ScaleForMinCost                              = SCALE_FOR_MIN_COST_UNNAMED10;

    DW2.Value                                        = 0;
    DW2.VignetteCorrectionFormat                     = VIGNETTE_CORRECTION_FORMAT_U88;
    DW2.UvThresholdValue                             = UV_THRESHOLD_VALUE_UNNAMED64;
    DW2.YOutlierValue                                = Y_OUTLIER_VALUE_UNNAMED253;
    DW2.YBrightValue                                 = Y_BRIGHT_VALUE_UNNAMED230;

    DW3.Value                                        = 0;
    DW3.BlackPointOffsetGreenTop                     = BLACK_POINT_OFFSET_GREEN_TOP_UNNAMED0;
    DW3.BlackPointOffsetRed                          = BLACK_POINT_OFFSET_RED_UNNAMED0;

    DW4.Value                                        = 0;
    DW4.BlackPointOffsetGreenBottom                  = BLACK_POINT_OFFSET_GREEN_BOTTOM_UNNAMED0;
    DW4.BlackPointOffsetBlue                         = BLACK_POINT_OFFSET_BLUE_UNNAMED0;

    DW5.Value                                        = 0;

    DW6.Value                                        = 0;

}

mhw_vebox_g9_X::VEBOX_CCM_STATE_CMD::VEBOX_CCM_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.C1                                           = C1_11414096;

    DW1.Value                                        = 0;
    DW1.C0                                           = C0_27924096;

    DW2.Value                                        = 0;
    DW2.C3                                           = C3_714096;

    DW3.Value                                        = 0;
    DW3.C2                                           = C2_344096;

    DW4.Value                                        = 0;
    DW4.C5                                           = C5_524096;

    DW5.Value                                        = 0;
    DW5.C4                                           = C4_33634096;

    DW6.Value                                        = 0;
    DW6.C7                                           = C7_1684096;

    DW7.Value                                        = 0;
    DW7.C6                                           = C6_124096;

    DW8.Value                                        = 0;
    DW8.C8                                           = C8_34344096;

}

mhw_vebox_g9_X::VEBOX_CSC_STATE_CMD::VEBOX_CSC_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.C0                                           = C0_OR10;
    DW0.YuvChannelSwap                               = YUV_CHANNEL_SWAP_UNNAMED0;

    DW1.Value                                        = 0;
    DW1.C1                                           = C1_UNNAMED0;

    DW2.Value                                        = 0;
    DW2.C2                                           = C2_UNNAMED0;

    DW3.Value                                        = 0;
    DW3.C3                                           = C3_UNNAMED0;

    DW4.Value                                        = 0;
    DW4.C4                                           = C4_OR10;

    DW5.Value                                        = 0;
    DW5.C5                                           = C5_UNNAMED0;

    DW6.Value                                        = 0;
    DW6.C6                                           = C6_UNNAMED0;

    DW7.Value                                        = 0;
    DW7.C7                                           = C7_UNNAMED0;

    DW8.Value                                        = 0;
    DW8.C8                                           = C8_OR10;

    DW9.Value                                        = 0;
    DW9.OffsetIn1                                    = OFFSET_IN_1_UNNAMED0;
    DW9.OffsetOut1                                   = OFFSET_OUT_1_UNNAMED0;

    DW10.Value                                       = 0;
    DW10.OffsetIn2                                   = OFFSET_IN_2_UNNAMED0;
    DW10.OffsetOut2                                  = OFFSET_OUT_2_UNNAMED0;

    DW11.Value                                       = 0;
    DW11.OffsetIn3                                   = OFFSET_IN_3_UNNAMED0;
    DW11.OffsetOut3                                  = OFFSET_OUT_3_UNNAMED0;

}

mhw_vebox_g9_X::VEBOX_DNDI_STATE_CMD::VEBOX_DNDI_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.DenoiseHistoryIncrease                       = DENOISE_HISTORY_INCREASE_UNNAMED8;

    DW1.Value                                        = 0;

    DW2.Value                                        = 0;
    DW2.InitialDenoiseHistory                        = INITIAL_DENOISE_HISTORY_UNNAMED32;
    DW2.ProgressiveDn                                = PROGRESSIVE_DN_UNNAMED0;

    DW3.Value                                        = 0;
    DW3.BlockNoiseEstimateEdgeThreshold              = BLOCK_NOISE_ESTIMATE_EDGE_THRESHOLD_UNNAMED16;

    DW4.Value                                        = 0;
    DW4.ChromaDenoiseEnable                          = CHROMA_DENOISE_ENABLE_UNNAMED1;

    DW5.Value                                        = 0;

    DW6.Value                                        = 0;

    DW7.Value                                        = 0;

    DW8.Value                                        = 0;

    DW9.Value                                        = 0;

    DW10.Value                                       = 0;

    DW11.Value                                       = 0;

    DW12.Value                                       = 0;
    DW12.SadTightThreshold                           = SAD_TIGHT_THRESHOLD_UNNAMED5;
    DW12.ContentAdaptiveThresholdSlope               = CONTENT_ADAPTIVE_THRESHOLD_SLOPE_UNNAMED9;

    DW13.Value                                       = 0;
    DW13.StmmBlendingConstantSelect                  = STMM_BLENDING_CONSTANT_SELECT_UNNAMED0;

    DW14.Value                                       = 0;
    DW14.StmmShiftUp                                 = STMM_SHIFT_UP_SHIFTBY6;
    DW14.StmmShiftDown                               = STMM_SHIFT_DOWN_SHIFTBY4;

    DW15.Value                                       = 0;

    DW16.Value                                       = 0;
    DW16.DnDiTopFirst                                = DNDI_TOP_FIRST_UNNAMED0;
    DW16.CatThreshold                                = CAT_THRESHOLD_UNNAMED0;

    DW17.Value                                       = 0;
    DW17.SadTha                                      = SAD_THA_UNNAMED5;
    DW17.SadThb                                      = SAD_THB_UNNAMED10;
    DW17.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame = PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME_DEINTERLACE;
    DW17.McPixelConsistencyThreshold                 = MC_PIXEL_CONSISTENCY_THRESHOLD_UNNAMED25;
    DW17.ProgressiveCadenceReconstructionFor2NdFieldOfPreviousFrame = PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME_DEINTERLACE;
    DW17.NeighborPixelThreshold                      = NEIGHBOR_PIXEL_THRESHOLD_UNNAMED10;

}

mhw_vebox_g9_X::VEBOX_FRONT_END_CSC_STATE_CMD::VEBOX_FRONT_END_CSC_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.FecscC0TransformCoefficient                  = FECSC_C0_TRANSFORM_COEFFICIENT_OR10;

    DW1.Value                                        = 0;
    DW1.FecscC1TransformCoefficient                  = FECSC_C1_TRANSFORM_COEFFICIENT_OR00;

    DW2.Value                                        = 0;
    DW2.FecscC2TransformCoefficient                  = FECSC_C2_TRANSFORM_COEFFICIENT_OR00;

    DW3.Value                                        = 0;
    DW3.FecscC3TransformCoefficient                  = FECSC_C3_TRANSFORM_COEFFICIENT_OR00;

    DW4.Value                                        = 0;
    DW4.FecscC4TransformCoefficient                  = FECSC_C4_TRANSFORM_COEFFICIENT_OR10;

    DW5.Value                                        = 0;
    DW5.FecscC5TransformCoefficient                  = FECSC_C5_TRANSFORM_COEFFICIENT_OR00;

    DW6.Value                                        = 0;
    DW6.FecscC6TransformCoefficient                  = FECSC_C6_TRANSFORM_COEFFICIENT_OR00;

    DW7.Value                                        = 0;
    DW7.FecscC7TransformCoefficient                  = FECSC_C7_TRANSFORM_COEFFICIENT_OR00;

    DW8.Value                                        = 0;
    DW8.FecscC8TransformCoefficient                  = FECSC_C8_TRANSFORM_COEFFICIENT_OR10;

    DW9.Value                                        = 0;
    DW9.FecScOffsetIn1OffsetInForYR                  = FEC_SC_OFFSET_IN_1_OFFSET_IN_FOR_YR_UNNAMED0;
    DW9.FecScOffsetOut1OffsetOutForYR                = FEC_SC_OFFSET_OUT_1_OFFSET_OUT_FOR_YR_UNNAMED0;

    DW10.Value                                       = 0;
    DW10.FecScOffsetIn2OffsetOutForUG                = FEC_SC_OFFSET_IN_2_OFFSET_OUT_FOR_UG_UNNAMED0;
    DW10.FecScOffsetOut2OffsetOutForUG               = FEC_SC_OFFSET_OUT_2_OFFSET_OUT_FOR_UG_UNNAMED0;

    DW11.Value                                       = 0;
    DW11.FecScOffsetIn3OffsetOutForVB                = FEC_SC_OFFSET_IN_3_OFFSET_OUT_FOR_VB_UNNAMED0;
    DW11.FecScOffsetOut3OffsetOutForVB               = FEC_SC_OFFSET_OUT_3_OFFSET_OUT_FOR_VB_UNNAMED0;

}

mhw_vebox_g9_X::VEBOX_GAMUT_STATE_CMD::VEBOX_GAMUT_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.GlobalModeEnable                             = GLOBAL_MODE_ENABLE_ADVANCEMODE;
    DW0.AR                                           = AR_UNNAMED436;

    DW1.Value                                        = 0;

    DW2.Value                                        = 0;

    DW3.Value                                        = 0;

    DW4.Value                                        = 0;

    DW5.Value                                        = 0;

    DW6.Value                                        = 0;

    DW7.Value                                        = 0;

    DW8.Value                                        = 0;
    DW8.PwlGammaPoint1                               = PWL_GAMMA_POINT_1_UNNAMED1;
    DW8.PwlGammaPoint2                               = PWL_GAMMA_POINT_2_UNNAMED2;
    DW8.PwlGammaPoint3                               = PWL_GAMMA_POINT_3_UNNAMED5;
    DW8.PwlGammaPoint4                               = PWL_GAMMA_POINT_4_UNNAMED9;

    DW9.Value                                        = 0;
    DW9.PwlGammaPoint5                               = PWL_GAMMA_POINT_5_UNNAMED16;
    DW9.PwlGammaPoint6                               = PWL_GAMMA_POINT_6_UNNAMED26;
    DW9.PwlGammaPoint7                               = PWL_GAMMA_POINT_7_UNNAMED42;
    DW9.PwlGammaPoint8                               = PWL_GAMMA_POINT_8_UNNAMED65;

    DW10.Value                                       = 0;
    DW10.PwlGammaPoint9                              = PWL_GAMMA_POINT_9_UNNAMED96;
    DW10.PwlGammaPoint10                             = PWL_GAMMA_POINT_10_UNNAMED136;
    DW10.PwlGammaPoint11                             = PWL_GAMMA_POINT_11_UNNAMED187;

    DW11.Value                                       = 0;
    DW11.PwlGammaBias1                               = PWL_GAMMA_BIAS_1_UNNAMED13;
    DW11.PwlGammaBias2                               = PWL_GAMMA_BIAS_2_UNNAMED23;
    DW11.PwlGammaBias3                               = PWL_GAMMA_BIAS_3_UNNAMED38;
    DW11.PwlGammaBias4                               = PWL_GAMMA_BIAS_4_UNNAMED53;

    DW12.Value                                       = 0;
    DW12.PwlGammaBias5                               = PWL_GAMMA_BIAS_5_UNNAMED71;
    DW12.PwlGammaBias6                               = PWL_GAMMA_BIAS_6_UNNAMED91;
    DW12.PwlGammaBias7                               = PWL_GAMMA_BIAS_7_UNNAMED114;
    DW12.PwlGammaBias8                               = PWL_GAMMA_BIAS_8_UNNAMED139;

    DW13.Value                                       = 0;
    DW13.PwlGammaBias9                               = PWL_GAMMA_BIAS_9_UNNAMED165;
    DW13.PwlGammaBias10                              = PWL_GAMMA_BIAS_10_UNNAMED193;
    DW13.PwlGammaBias11                              = PWL_GAMMA_BIAS_11_UNNAMED223;

    DW14.Value                                       = 0;

    DW15.Value                                       = 0;

    DW16.Value                                       = 0;

    DW17.Value                                       = 0;

    DW18.Value                                       = 0;

    DW19.Value                                       = 0;

    DW20.Value                                       = 0;
    DW20.PwlInvGammaPoint1                           = PWL_INV_GAMMA_POINT_1_UNNAMED30;
    DW20.PwlInvGammaPoint2                           = PWL_INV_GAMMA_POINT_2_UNNAMED55;
    DW20.PwlInvGammaPoint3                           = PWL_INV_GAMMA_POINT_3_UNNAMED79;
    DW20.PwlInvGammaPoint4                           = PWL_INV_GAMMA_POINT_4_UNNAMED101;

    DW21.Value                                       = 0;
    DW21.PwlInvGammaPoint5                           = PWL_INV_GAMMA_POINT_5_UNNAMED122;
    DW21.PwlInvGammaPoint6                           = PWL_INV_GAMMA_POINT_6_UNNAMED141;
    DW21.PwlInvGammaPoint7                           = PWL_INV_GAMMA_POINT_7_UNNAMED162;
    DW21.PwlInvGammaPoint8                           = PWL_INV_GAMMA_POINT_8_UNNAMED181;

    DW22.Value                                       = 0;
    DW22.PwlInvGammaPoint9                           = PWL_INV_GAMMA_POINT_9_UNNAMED200;
    DW22.PwlInvGammaPoint10                          = PWL_INV_GAMMA_POINT_10_UNNAMED219;
    DW22.PwlInvGammaPoint11                          = PWL_INV_GAMMA_POINT_11_UNNAMED237;

    DW23.Value                                       = 0;
    DW23.PwlInvGammaBias1                            = PWL_INV_GAMMA_BIAS_1_UNNAMED3;
    DW23.PwlInvGammaBias2                            = PWL_INV_GAMMA_BIAS_2_UNNAMED10;
    DW23.PwlInvGammaBias3                            = PWL_INV_GAMMA_BIAS_3_UNNAMED20;
    DW23.PwlInvGammaBias4                            = PWL_INV_GAMMA_BIAS_4_UNNAMED33;

    DW24.Value                                       = 0;
    DW24.PwlInvGammaBias5                            = PWL_INV_GAMMA_BIAS_5_UNNAMED49;
    DW24.PwlInvGammaBias6                            = PWL_INV_GAMMA_BIAS_6_UNNAMED67;
    DW24.PwlInvGammaBias7                            = PWL_INV_GAMMA_BIAS_7_UNNAMED92;
    DW24.PwlInvGammaBias8                            = PWL_INV_GAMMA_BIAS_8_UNNAMED117;

    DW25.Value                                       = 0;
    DW25.PwlInvGammaBias9                            = PWL_INV_GAMMA_BIAS_9_UNNAMED147;
    DW25.PwlInvGammaBias10                           = PWL_INV_GAMMA_BIAS_10_UNNAMED180;
    DW25.PwlInvGammaBias11                           = PWL_INV_GAMMA_BIAS_11_UNNAMED215;

    DW26.Value                                       = 0;

    DW27.Value                                       = 0;

    DW28.Value                                       = 0;

    DW29.Value                                       = 0;

    DW30.Value                                       = 0;

    DW31.Value                                       = 0;

    DW32.Value                                       = 0;
    DW32.OffsetInR                                   = OFFSET_IN_R_UNNAMED0;
    DW32.OffsetInG                                   = OFFSET_IN_G_UNNAMED0;

    DW33.Value                                       = 0;
    DW33.OffsetInB                                   = OFFSET_IN_B_UNNAMED0;

    DW34.Value                                       = 0;

    DW35.Value                                       = 0;
    DW35.D1Out                                       = D1OUT_UNNAMED287;
    DW35.DOutDefault                                 = DOUT_DEFAULT_UNNAMED164;
    DW35.DInDefault                                  = DINDEFAULT_UNNAMED205;
    DW35.Fullrangemappingenable                      = FULLRANGEMAPPINGENABLE_BASICMODE;

    DW36.Value                                       = 0;
    DW36.D1In                                        = D1IN_UNNAMED820;
    DW36.Compressionlineshift                        = COMPRESSIONLINESHIFT_UNNAMED3;
    DW36.Xvyccdecencenable                           = XVYCCDECENCENABLE_BOTHXVYCCDECODEANDXVYCCENCODEAREENABLED;

    DW37.Value                                       = 0;
    DW37.CpiOverride                                 = CPI_OVERRIDE_UNNAMED0;
    DW37.Lumachormaonlycorrection                    = LUMACHORMAONLYCORRECTION_LUMAONLYCORRECTION;
    DW37.GccBasicmodeselection                       = GCC_BASICMODESELECTION_DEFAULT;

}

mhw_vebox_g9_X::VEBOX_STD_STE_STATE_CMD::VEBOX_STD_STE_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.OutputControl                                = OUTPUT_CONTROL_OUTPUTPIXELS;
    DW0.SatMax                                       = SAT_MAX_UNNAMED31;
    DW0.HueMax                                       = HUE_MAX_UNNAMED14;
    DW0.UMid                                         = U_MID_UNNAMED110;
    DW0.VMid                                         = V_MID_UNNAMED154;

    DW1.Value                                        = 0;
    DW1.Sin                                          = SIN_UNNAMED101;
    DW1.Cos                                          = COS_UNNAMED79;
    DW1.HsMargin                                     = HS_MARGIN_UNNAMED3;
    DW1.DiamondDu                                    = DIAMOND_DU_UNNAMED0;
    DW1.DiamondMargin                                = DIAMOND_MARGIN_UNNAMED4;

    DW2.Value                                        = 0;
    DW2.DiamondDv                                    = DIAMOND_DV_UNNAMED0;
    DW2.DiamondTh                                    = DIAMOND_TH_UNNAMED35;
    DW2.DiamondAlpha                                 = DIAMOND_ALPHA_UNNAMED100;

    DW3.Value                                        = 0;
    DW3.YPoint1                                      = Y_POINT_1_UNNAMED46;
    DW3.YPoint2                                      = Y_POINT_2_UNNAMED47;
    DW3.YPoint3                                      = Y_POINT_3_UNNAMED254;

    DW4.Value                                        = 0;
    DW4.YPoint4                                      = Y_POINT_4_UNNAMED255;
    DW4.YSlope1                                      = Y_SLOPE_1_UNNAMED31;
    DW4.YSlope2                                      = Y_SLOPE_2_UNNAMED31;

    DW5.Value                                        = 0;
    DW5.InvSkinTypesMargin                           = INV_SKIN_TYPES_MARGIN_SKINTYPEMARGIN;

    DW6.Value                                        = 0;
    DW6.InvMarginVyu                                 = INV_MARGIN_VYU_UNNAMED1600;
    DW6.P0L                                          = P0L_UNNAMED46;
    DW6.P1L                                          = P1L_UNNAMED216;

    DW7.Value                                        = 0;
    DW7.P2L                                          = P2L_UNNAMED236;
    DW7.P3L                                          = P3L_UNNAMED236;
    DW7.B0L                                          = B0L_UNNAMED133;
    DW7.B1L                                          = B1L_UNNAMED130;

    DW8.Value                                        = 0;
    DW8.B2L                                          = B2L_UNNAMED130;
    DW8.B3L                                          = B3L_UNNAMED130;
    DW8.S0L                                          = S0L_UNNAMED2043;

    DW9.Value                                        = 0;
    DW9.S1L                                          = S1L_UNNAMED0;
    DW9.S2L                                          = S2L_UNNAMED0;

    DW10.Value                                       = 0;
    DW10.S3L                                         = S3L_UNNAMED0;
    DW10.P0U                                         = P0U_UNNAMED46;
    DW10.P1U                                         = P1U_UNNAMED66;

    DW11.Value                                       = 0;
    DW11.P2U                                         = P2U_UNNAMED150;
    DW11.P3U                                         = P3U_UNNAMED236;
    DW11.B0U                                         = B0U_UNNAMED143;
    DW11.B1U                                         = B1U_UNNAMED163;

    DW12.Value                                       = 0;
    DW12.B2U                                         = B2U_UNNAMED200;
    DW12.B3U                                         = B3U_UNNAMED200;
    DW12.S0U                                         = S0U_UNNAMED256;

    DW13.Value                                       = 0;
    DW13.S1U                                         = S1U_UNNAMED113;
    DW13.S2U                                         = S2U_UNNAMED1869;

    DW14.Value                                       = 0;
    DW14.S3U                                         = S3U_UNNAMED0;
    DW14.SkinTypesEnable                             = SKIN_TYPES_ENABLE_DISABLE;
    DW14.SkinTypesThresh                             = SKIN_TYPES_THRESH_UNNAMED120;
    DW14.SkinTypesMargin                             = SKIN_TYPES_MARGIN_UNNAMED20;

    DW15.Value                                       = 0;
    DW15.Satp1                                       = SATP1_UNNAMED122;
    DW15.Satp2                                       = SATP2_UNNAMED6;
    DW15.Satp3                                       = SATP3_UNNAMED31;
    DW15.Satb1                                       = SATB1_UNNAMED1016;

    DW16.Value                                       = 0;
    DW16.Satb2                                       = SATB2_UNNAMED8;
    DW16.Satb3                                       = SATB3_UNNAMED124;
    DW16.Sats0                                       = SATS0_UNNAMED297;

    DW17.Value                                       = 0;
    DW17.Sats1                                       = SATS1_UNNAMED85;
    DW17.Sats2                                       = SATS2_UNNAMED297;

    DW18.Value                                       = 0;
    DW18.Sats3                                       = SATS3_UNNAMED256;
    DW18.Huep1                                       = HUEP1_6;
    DW18.Huep2                                       = HUEP2_UNNAMED6;
    DW18.Huep3                                       = HUEP3_UNNAMED14;

    DW19.Value                                       = 0;
    DW19.Hueb1                                       = HUEB1_UNNAMED8;
    DW19.Hueb2                                       = HUEB2_UNNAMED8;
    DW19.Hueb3                                       = HUEB3_UNNAMED56;

    DW20.Value                                       = 0;
    DW20.Hues0                                       = HUES0_UNNAMED384;
    DW20.Hues1                                       = HUES1_UNNAMED85;

    DW21.Value                                       = 0;
    DW21.Hues2                                       = HUES2_UNNAMED384;
    DW21.Hues3                                       = HUES3_UNNAMED256;

    DW22.Value                                       = 0;
    DW22.Satp1Dark                                   = SATP1_DARK_UNNAMED123;
    DW22.Satp2Dark                                   = SATP2_DARK_UNNAMED31;
    DW22.Satp3Dark                                   = SATP3_DARK_UNNAMED31;
    DW22.Satb1Dark                                   = SATB1_DARK_UNNAMED0;

    DW23.Value                                       = 0;
    DW23.Satb2Dark                                   = SATB2_DARK_UNNAMED124;
    DW23.Satb3Dark                                   = SATB3_DARK_UNNAMED124;
    DW23.Sats0Dark                                   = SATS0_DARK_UNNAMED397;

    DW24.Value                                       = 0;
    DW24.Sats1Dark                                   = SATS1_DARK_UNNAMED189;
    DW24.Sats2Dark                                   = SATS2_DARK_UNNAMED256;

    DW25.Value                                       = 0;
    DW25.Sats3Dark                                   = SATS3_DARK_UNNAMED256;
    DW25.Huep1Dark                                   = HUEP1_DARK_UNNAMED0;
    DW25.Huep2Dark                                   = HUEP2_DARK_UNNAMED2;
    DW25.Huep3Dark                                   = HUEP3_DARK_UNNAMED14;

    DW26.Value                                       = 0;
    DW26.Hueb1Dark                                   = HUEB1_DARK_UNNAMED0;
    DW26.Hueb2Dark                                   = HUEB2_DARK_UNNAMED0;
    DW26.Hueb3Dark                                   = HUEB3_DARK_UNNAMED56;

    DW27.Value                                       = 0;
    DW27.Hues0Dark                                   = HUES0_DARK_UNNAMED299;
    DW27.Hues1Dark                                   = HUES1_DARK_UNNAMED256;

    DW28.Value                                       = 0;
    DW28.Hues2Dark                                   = HUES2_DARK_UNNAMED299;
    DW28.Hues3Dark                                   = HUES3_DARK_UNNAMED256;

}

mhw_vebox_g9_X::VEBOX_TCC_STATE_CMD::VEBOX_TCC_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.Satfactor1                                   = SATFACTOR1_UNNAMED220;
    DW0.Satfactor2                                   = SATFACTOR2_UNNAMED220;
    DW0.Satfactor3                                   = SATFACTOR3_UNNAMED220;

    DW1.Value                                        = 0;
    DW1.Satfactor4                                   = SATFACTOR4_UNNAMED220;
    DW1.Satfactor5                                   = SATFACTOR5_UNNAMED220;
    DW1.Satfactor6                                   = SATFACTOR6_UNNAMED220;

    DW2.Value                                        = 0;
    DW2.Basecolor1                                   = BASECOLOR1_UNNAMED145;
    DW2.Basecolor2                                   = BASECOLOR2_UNNAMED307;
    DW2.Basecolor3                                   = BASECOLOR3_UNNAMED483;

    DW3.Value                                        = 0;
    DW3.Basecolo4                                    = BASECOLO4_UNNAMED657;
    DW3.Basecolor5                                   = BASECOLOR5_UNNAMED819;
    DW3.Basecolor6                                   = BASECOLOR6_UNNAMED995;

    DW4.Value                                        = 0;
    DW4.Colortransitslope2                           = COLORTRANSITSLOPE2_UNNAMED405;
    DW4.Colortransitslope23                          = COLORTRANSITSLOPE23_UNNAMED744;

    DW5.Value                                        = 0;
    DW5.Colortransitslope34                          = COLORTRANSITSLOPE34_UNNAMED1131;
    DW5.Colortransitslope45                          = COLORTRANSITSLOPE45_UNNAMED407;

    DW6.Value                                        = 0;
    DW6.Colortransitslope56                          = COLORTRANSITSLOPE56_UNNAMED372;
    DW6.Colortransitslope61                          = COLORTRANSITSLOPE61_UNNAMED377;

    DW7.Value                                        = 0;
    DW7.Colorbias1                                   = COLORBIAS1_UNNAMED0;
    DW7.Colorbias2                                   = COLORBIAS2_UNNAMED150;
    DW7.Colorbias3                                   = COLORBIAS3_UNNAMED0;

    DW8.Value                                        = 0;
    DW8.Colorbias4                                   = COLORBIAS4_UNNAMED0;
    DW8.Colorbias5                                   = COLORBIAS5_UNNAMED0;
    DW8.Colorbias6                                   = COLORBIAS6_UNNAMED0;

    DW9.Value                                        = 0;
    DW9.SteSlopeBits                                 = STE_SLOPE_BITS_UNNAMED0;
    DW9.SteThreshold                                 = STE_THRESHOLD_UNNAMED0;
    DW9.UvThresholdBits                              = UV_THRESHOLD_BITS_UNNAMED3;
    DW9.UvThreshold                                  = UV_THRESHOLD_UNNAMED3;

    DW10.Value                                       = 0;
    DW10.Uvmaxcolor                                  = UVMAXCOLOR_UNNAMED448;
    DW10.InvUvmaxcolor                               = INV_UVMAXCOLOR_UNNAMED146;

}

mhw_vebox_g9_X::VEBOX_PROCAMP_STATE_CMD::VEBOX_PROCAMP_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.Brightness                                   = BRIGHTNESS_OR00;
    DW0.Contrast                                     = CONTRAST_10INFIXEDPOINTU47;

    DW1.Value                                        = 0;
    DW1.SinCS                                        = SIN_C_S_UNNAMED0;
    DW1.CosCS                                        = COS_C_S_UNNAMED256;

}

mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD::VEBOX_IECP_STATE_CMD()
{
}

mhw_vebox_g9_X::VEBOX_STATE_CMD::VEBOX_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED2;
    DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    DW0.CommandOpcode                                = COMMAND_OPCODE_VEBOX;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;
    DW1.DnEnable                                     = DN_ENABLE_DONOTDENOISEFRAME;
    DW1.DiEnable                                     = DI_ENABLE_DONOTCALCULATEDI;
    DW1.DnDiFirstFrame                               = DNDI_FIRST_FRAME_NOTFIRSTFIELD_PREVIOUSCLEANSURFACESTATEISVALID;
    DW1.DownsampleMethod422to420                     = _422_420_DOWNSAMPLE_METHOD_DROPLOWERCHROMAOFTHEPAIR;
    DW1.DownsampleMethod444to422                     = _444_422_DOWNSAMPLE_METHOD_DROPRIGHTCHROMAOFTHEPAIR;
    DW1.DiOutputFrames                               = DI_OUTPUT_FRAMES_OUTPUTBOTHFRAMES;
    DW1.SingleSliceVeboxEnable                       = SINGLE_SLICE_VEBOX_ENABLE_BOTHSLICESENABLED;
    DW1.SinglePipeEnable                             = SINGLE_PIPE_ENABLE_DEFAULT;

    DW2.Value                                        = 0;

    DW3.Value                                        = 0;

    DW4.Value                                        = 0;

    DW5.Value                                        = 0;

    DW6.Value                                        = 0;

    DW7.Value                                        = 0;

    DW8.Value                                        = 0;

    DW9.Value                                        = 0;

    DW10.Value                                       = 0;

    DW11.Value                                       = 0;

    DW12.Value                                       = 0;

    DW13.Value                                       = 0;
    DW13.ArbitrationPriorityControlForLaceLut        = ARBITRATION_PRIORITY_CONTROL_FOR_LACE_LUT_HIGHESTPRIORITY;

    DW14_15.Value[0] = DW14_15.Value[1]              = 0;

}

mhw_vebox_g9_X::VEBOX_SURFACE_STATE_CMD::VEBOX_SURFACE_STATE_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.SubopcodeB                                   = SUBOPCODE_B_VEBOX;
    DW0.SubopcodeA                                   = SUBOPCODE_A_VEBOX;
    DW0.MediaCommandOpcode                           = MEDIA_COMMAND_OPCODE_VEBOX;
    DW0.MediaCommandPipeline                         = MEDIA_COMMAND_PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;
    DW1.SurfaceIdentification                        = SURFACE_IDENTIFICATION_INPUTSURFACEANDDENOISEDCURRENTOUTPUTSURFACE;

    DW2.Value                                        = 0;

    DW3.Value                                        = 0;
    DW3.TileWalk                                     = TILE_WALK_TILEWALKXMAJOR;
    DW3.TiledSurface                                 = TILED_SURFACE_FALSE;
    DW3.BayerPatternFormat                           = BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
    DW3.BayerPatternOffset                           = BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
    DW3.SurfaceFormat                                = SURFACE_FORMAT_YCRCBNORMAL;

    DW4.Value                                        = 0;

    DW5.Value                                        = 0;

    DW6.Value                                        = 0;

    DW7.Value                                        = 0;

    DW8.Value                                        = 0;

}

mhw_vebox_g9_X::VEBOX_TILING_CONVERT_CMD::VEBOX_TILING_CONVERT_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.SubopcodeB                                   = SUBOPCODE_B_UNNAMED1;
    DW0.SubopcodeA                                   = SUBOPCODE_A_UNNAMED0;
    DW0.CommandOpcode                                = COMMAND_OPCODE_VEBOX;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1_2.Value[0] = DW1_2.Value[1]                  = 0;

    DW3_4.Value[0] = DW3_4.Value[1]                  = 0;

}

mhw_vebox_g9_X::VEB_DI_IECP_CMD::VEB_DI_IECP_CMD()
{
    DW0.Value                                        = 0;
    DW0.DwordLength                                  = GetOpLength(dwSize);
    DW0.Subopb                                       = SUBOPB_VEBDIIECP;
    DW0.Subopa                                       = SUBOPA_VEBDIIECP;
    DW0.Opcode                                       = OPCODE_VEBOX;
    DW0.Pipeline                                     = PIPELINE_MEDIA;
    DW0.CommandType                                  = COMMAND_TYPE_PARALLELVIDEOPIPE;

    DW1.Value                                        = 0;

    DW2.Value                                        = 0;

    DW3.Value                                        = 0;
    DW3.CurrentFrameInputSurfaceArbitrationPriorityControl = CURRENT_FRAME_INPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW4.Value                                        = 0;

    DW5.Value                                        = 0;
    DW5.PreviousFrameInputSurfaceArbitrationPriorityControl = PREVIOUS_FRAME_INPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW6.Value                                        = 0;

    DW7.Value                                        = 0;
    DW7.StmmInputSurfaceArbitrationPriorityControl   = STMM_INPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW8.Value                                        = 0;

    DW9.Value                                        = 0;
    DW9.StmmOutputSurfaceArbitrationPriorityControl  = STMM_OUTPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW10.Value                                       = 0;

    DW11.Value                                       = 0;
    DW11.DenoisedCurrentOutputSurfaceArbitrationPriorityControl = DENOISED_CURRENT_OUTPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW12.Value                                       = 0;

    DW13.Value                                       = 0;
    DW13.CurrentFrameOutputSurfaceArbitrationPriorityControl = CURRENT_FRAME_OUTPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW14.Value                                       = 0;

    DW15.Value                                       = 0;
    DW15.PreviousFrameOutputSurfaceArbitrationPriorityControl = PREVIOUS_FRAME_OUTPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW16.Value                                       = 0;

    DW17.Value                                       = 0;
    DW17.StatisticsOutputSurfaceArbitrationPriorityControl = STATISTICS_OUTPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW18.Value                                       = 0;

    DW19.Value                                       = 0;
    DW19.AlphaVignetteCorrectionSurfaceArbitrationPriorityControl = ALPHAVIGNETTE_CORRECTION_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW20.Value                                       = 0;

    DW21.Value                                       = 0;
    DW21.LaceAceRgbHistogramSurfaceArbitrationPriorityControl = LACEACERGB_HISTOGRAM_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

    DW22.Value                                       = 0;

    DW23.Value                                       = 0;
    DW23.SkinScoreOutputSurfaceArbitrationPriorityControl = SKIN_SCORE_OUTPUT_SURFACE_ARBITRATION_PRIORITY_CONTROL_HIGHESTPRIORITY;

}

mhw_vebox_g9_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD()
{
    DW0.Value                                        = 0;
    DW0.MemoryCompressionMode                        = MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE;
    DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TILED_RESOURCE_MODE_FOR_OUTPUT_FRAME_SURFACE_BASE_ADDRESS_TRMODENONE;

}