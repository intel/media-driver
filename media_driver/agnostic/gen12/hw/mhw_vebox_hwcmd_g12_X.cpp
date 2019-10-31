/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file   mhw_vebox_hwcmd_g12_X.cpp
//! \brief  Auto-generated definitions for MHW commands and states.
//!

#include "mhw_vebox_hwcmd_g12_X.h"
#include "mos_utilities.h"

mhw_vebox_g12_X::VEBOX_ACE_LACE_STATE_CMD::VEBOX_ACE_LACE_STATE_CMD()
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

    DW13.Value                                       = 0;
    DW14.Value                                       = 0;
    DW15.Value                                       = 0;
    DW16.Value                                       = 0;
    DW17.Value                                       = 0;
    DW18.Value                                       = 0;
    DW19.Value                                       = 0;
    DW20.Value                                       = 0;
    DW21.Value                                       = 0;
    DW22.Value                                       = 0;
    DW23.Value                                       = 0;
    DW24.Value                                       = 0;
    DW25.Value                                       = 0;
    DW26.Value                                       = 0;
    DW27.Value                                       = 0;
    DW28.Value                                       = 0;
    DW29.Value                                       = 0;
    DW30.Value                                       = 0;
}

mhw_vebox_g12_X::VEBOX_ALPHA_AOI_STATE_CMD::VEBOX_ALPHA_AOI_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.AlphaFromStateSelect                         = ALPHA_FROM_STATE_SELECT_ALPHAISTAKENFROMMESSAGE;

    DW1.Value                                        = 0;        
    DW1.AoiMinX                                      = AOI_MIN_X_UNNAMED0;
    DW1.AoiMaxX                                      = AOI_MAX_X_UNNAMED3;

    DW2.Value                                        = 0;        
    DW2.AoiMinY                                      = AOI_MIN_Y_UNNAMED0;
    DW2.AoiMaxY                                      = AOI_MAX_Y_UNNAMED3;

}

mhw_vebox_g12_X::VEBOX_CAPTURE_PIPE_STATE_CMD::VEBOX_CAPTURE_PIPE_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.GoodPixelNeighborThreshold                   = GOOD_PIXEL_NEIGHBOR_THRESHOLD_UNNAMED35;
    DW0.AverageColorThreshold                        = AVERAGE_COLOR_THRESHOLD_UNNAMED255;
    DW0.GreenImbalanceThreshold                      = GREEN_IMBALANCE_THRESHOLD_UNNAMED1;
    DW0.ShiftMinCost                                 = SHIFT_MIN_COST_UNNAMED1;
    DW0.GoodPixelThreshold                           = GOOD_PIXEL_THRESHOLD_UNNAMED5;
    DW0.DirmapScale                                  = DIRMAP_SCALE_UNNAMED2;

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

mhw_vebox_g12_X::VEBOX_CCM_STATE_CMD::VEBOX_CCM_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.C1                                           = C1_1825665536;

    DW1.Value                                        = 0;        
    DW1.C0                                           = C0_4467265536;

    DW2.Value                                        = 0;        
    DW2.C3                                           = C3_113665536;

    DW3.Value                                        = 0;        
    DW3.C2                                           = C2_54465536;

    DW4.Value                                        = 0;        
    DW4.C5                                           = C5_83265536;

    DW5.Value                                        = 0;        
    DW5.C4                                           = C4_5380865536;

    DW6.Value                                        = 0;        
    DW6.C7                                           = C7_268865536;

    DW7.Value                                        = 0;        
    DW7.C6                                           = C6_19265536;

    DW8.Value                                        = 0;        
    DW8.C8                                           = C8_5494465536;

    DW9.Value                                        = 0;        
    DW9.OffsetInR                                    = OFFSET_IN_R_UNNAMED0;

    DW10.Value                                       = 0;        
    DW10.OffsetInG                                   = OFFSET_IN_G_UNNAMED0;

    DW11.Value                                       = 0;        
    DW11.OffsetInB                                   = OFFSET_IN_B_UNNAMED0;

    DW12.Value                                       = 0;        
    DW12.OffsetOutR                                  = OFFSET_OUT_R_UNNAMED0;

    DW13.Value                                       = 0;        
    DW13.OffsetOutG                                  = OFFSET_OUT_G_UNNAMED0;

    DW14.Value                                       = 0;        
    DW14.OffsetOutB                                  = OFFSET_OUT_B_UNNAMED0;

}

mhw_vebox_g12_X::VEBOX_CSC_STATE_CMD::VEBOX_CSC_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.C0                                           = C0_OR10;

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

mhw_vebox_g12_X::VEBOX_DNDI_STATE_CMD::VEBOX_DNDI_STATE_CMD()
{
    DW0.Value                                        = 0;        

    DW1.Value                                        = 0;        
    DW1.DenoiseHistoryIncrease                       = DENOISE_HISTORY_INCREASE_UNNAMED8;

    DW2.Value                                        = 0;        
    DW2.InitialDenoiseHistory                        = INITIAL_DENOISE_HISTORY_UNNAMED32;

    DW3.Value                                        = 0;        

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        
    DW6.BlockNoiseEstimateEdgeThreshold              = BLOCK_NOISE_ESTIMATE_EDGE_THRESHOLD_UNNAMED80;

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

    DW9.Value                                        = 0;        

    DW10.Value                                       = 0;        

    DW11.Value                                       = 0;        

    DW12.Value                                       = 0;        

    DW13.Value                                       = 0;        

    DW14.Value                                       = 0;        

    DW15.Value                                       = 0;        

    DW16.Value                                       = 0;        

    DW17.Value                                       = 0;        

    DW18.Value                                       = 0;        

    DW19.Value                                       = 0;        

    DW20.Value                                       = 0;        

    DW21.Value                                       = 0;        

    DW22.Value                                       = 0;        

    DW23.Value                                       = 0;        

    DW24.Value                                       = 0;        

    DW25.Value                                       = 0;        

    DW26.Value                                       = 0;        

    DW27.Value                                       = 0;        

    DW28.Value                                       = 0;        

    DW29.Value                                       = 0;        

    DW30.Value                                       = 0;        
    DW30.ValidPixelThreshold                         = VALID_PIXEL_THRESHOLD_UNNAMED480;
    DW30.EightDirectionEdgeThreshold                 = EIGHT_DIRECTION_EDGE_THRESHOLD_UNNAMED1024;

    DW31.Value                                       = 0;        
    DW31.LargeSobelThreshold                         = LARGE_SOBEL_THRESHOLD_UNNAMED2400;
    DW31.SmallSobelThreshold                         = SMALL_SOBEL_THRESHOLD_UNNAMED480;

    DW32.Value                                       = 0;        
    DW32.BlockSigmaDiffThreshold                     = BLOCK_SIGMA_DIFF_THRESHOLD_UNNAMED480;

    DW33.Value                                       = 0;        
    DW33.MaxSobelThreshold                           = MAX_SOBEL_THRESHOLD_UNNAMED1440;

    DW34.Value                                       = 0;        
    DW34.SadTightThreshold                           = SAD_TIGHT_THRESHOLD_UNNAMED5;
    DW34.ContentAdaptiveThresholdSlope               = CONTENT_ADAPTIVE_THRESHOLD_SLOPE_UNNAMED9;

    DW35.Value                                       = 0;        
    DW35.StmmBlendingConstantSelect                  = STMM_BLENDING_CONSTANT_SELECT_UNNAMED0;

    DW36.Value                                       = 0;        
    DW36.LumatdmWt                                   = LUMATDM_WT_UNNAMED4;
    DW36.ChromatdmWt                                 = CHROMATDM_WT_UNNAMED0;
    DW36.StmmShiftUp                                 = STMM_SHIFT_UP_SHIFTBY6;
    DW36.StmmShiftDown                               = STMM_SHIFT_DOWN_SHIFTBY4;

    DW37.Value                                       = 0;        
    DW37.CoringThresholdForSvcm                      = CORING_THRESHOLD_FOR_SVCM_UNNAMED255;
    DW37.DeltabitValueForSvcm                        = DELTABIT_VALUE_FOR_SVCM_UNNAMED5;
    DW37.CoringThresholdForShcm                      = CORING_THRESHOLD_FOR_SHCM_UNNAMED255;
    DW37.DeltabitValueForShcm                        = DELTABIT_VALUE_FOR_SHCM_UNNAMED5;

    DW38.Value                                       = 0;        
    DW38.CatThreshold                                = CAT_THRESHOLD_UNNAMED0;

    DW39.Value                                       = 0;        
    DW39.SadTha                                      = SAD_THA_UNNAMED5;
    DW39.SadThb                                      = SAD_THB_UNNAMED10;
    DW39.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame = PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_1ST_FIELD_OF_CURRENT_FRAME_DEINTERLACE;
    DW39.McPixelConsistencyThreshold                 = MC_PIXEL_CONSISTENCY_THRESHOLD_UNNAMED25;
    DW39.ProgressiveCadenceReconstructionFor2NdFieldOfPreviousFrame = PROGRESSIVE_CADENCE_RECONSTRUCTION_FOR_2ND_FIELD_OF_PREVIOUS_FRAME_DEINTERLACE;
    DW39.NeighborPixelThreshold                      = NEIGHBOR_PIXEL_THRESHOLD_UNNAMED10;
    DW39.Fastercovergence                            = FASTERCOVERGENCE_UNNAMED0;

    DW40.Value                                       = 0;        
    DW40.SadWt0                                      = SAD_WT0_DEFAULTFORNATURALANDSYNTHETIC;
    DW40.SadWt1                                      = SAD_WT1_DEFAULTFORNATURAL;
    DW40.SadWt2                                      = SAD_WT2_DEFAULTFORNATURAL;
    DW40.SadWt3                                      = SAD_WT3_DEFAULTFORNATURAL;

    DW41.Value                                       = 0;        
    DW41.SadWt4                                      = SAD_WT4_DEFAULTFORNATURAL;
    DW41.SadWt6                                      = SAD_WT6_DEFAULTFORNATURAL;
    DW41.CoringThresholdForLumaSadCalculation        = CORING_THRESHOLD_FOR_LUMA_SAD_CALCULATION_UNNAMED0;
    DW41.CoringThresholdForChromaSadCalculation      = CORING_THRESHOLD_FOR_CHROMA_SAD_CALCULATION_UNNAMED0;

    DW42.Value                                       = 0;        
    DW42.ParDiffcheckslackthreshold                  = PAR_DIFFCHECKSLACKTHRESHOLD_UNNAMED15;
    DW42.ParTearinghighthreshold                     = PAR_TEARINGHIGHTHRESHOLD_UNNAMED100;
    DW42.ParTearinglowthreshold                      = PAR_TEARINGLOWTHRESHOLD_UNNAMED20;
    DW42.ParDirectioncheckth                         = PAR_DIRECTIONCHECKTH_UNNAMED3;
    DW42.ParSyntheticcontentcheck                    = PAR_SYNTHETICCONTENTCHECK_UNNAMED0;
    DW42.ParLocalcheck                               = PAR_LOCALCHECK_UNNAMED1;
    DW42.ParUsesyntheticcontentmedian                = PAR_USESYNTHETICCONTENTMEDIAN_UNNAMED0;

    DW43.Value                                       = 0;        
    DW43.Lpfwtlut0                                   = LPFWTLUT0_UNNAMED0;
    DW43.Lpfwtlut1                                   = LPFWTLUT1_UNNAMED0;
    DW43.Lpfwtlut2                                   = LPFWTLUT2_UNNAMED0;
    DW43.Lpfwtlut3                                   = LPFWTLUT3_UNNAMED0;

    DW44.Value                                       = 0;        
    DW44.Lpfwtlut4                                   = LPFWTLUT4_UNNAMED32;
    DW44.Lpfwtlut5                                   = LPFWTLUT5_UNNAMED64;
    DW44.Lpfwtlut6                                   = LPFWTLUT6_UNNAMED128;
    DW44.Lpfwtlut7                                   = LPFWTLUT7_UNNAMED255;

    DW45.Value                                       = 0;
    DW45.TdmUvThreshold                              = TDM_UV_THRESHOLD_UNNAMED100;
    DW45.HvUvThreshold                               = HV_UV_THRESHOLD_UNNAMED30;
    DW45.TdmHarmonicFactorSynthetic                  = TDM_HARMONIC_FACTOR_SYNTHETIC_UNNAMED4;
    DW45.TdmHarmonicFactorNatural                    = TDM_HARMONIC_FACTOR_NATURAL_UNNAMED4;
    DW45.SynthticFrame                               = SYNTHTIC_FRAME_UNNAMED0;
    DW45.SyntheticContentThreshold                   = SYNTHETIC_CONTENT_THRESHOLD_UNNAMED1500;

    DW46.Value                                       = 0;
    DW46.SvcmHarmonicFactorSynthetic                 = SVCM_HARMONIC_FACTOR_SYNTHETIC_UNNAMED4;
    DW46.ShcmHarmonicFactorSynthetic                 = SHCM_HARMONIC_FACTOR_SYNTHETIC_UNNAMED4;
    DW46.SvcmHarmonicFactorNatural                   = SVCM_HARMONIC_FACTOR_NATURAL_UNNAMED6;
    DW46.ShcmHarmonicFactorNatural                   = SHCM_HARMONIC_FACTOR_NATURAL_UNNAMED6;
    DW46.HarmonicCounterThreshold                    = HARMONIC_COUNTER_THRESHOLD_UNNAMED8;
    DW46.MaxHarmonicCounterThreshold                 = MAX_HARMONIC_COUNTER_THRESHOLD_UNNAMED15;
    DW46.NaturalContentThreshold                     = NATURAL_CONTENT_THRESHOLD_UNNAMED200;

    DW47.Value                                       = 0;
    DW47.MaximumValue                                = MAXIMUM_VALUE_UNNAMED67108863;

    DW48.Value                                       = 0;
    DW48.ShiftingValue                               = SHIFTING_VALUE_UNNAMED32;   
    DW48.HvYThreshold                                = HV_Y_THRESHOLD_UNNAMED50;  
    DW48.NumInlinerNumeratorThreshold                = NUM_INLINER_NUMERATOR_THRESHOLD_UNNAMED3;
    DW48.NumInlinerDenominatorThreshold              = NUM_INLINER_DENOMINATOR_THRESHOLD_UNNAMED4; 
}

mhw_vebox_g12_X::VEBOX_FRONT_END_CSC_STATE_CMD::VEBOX_FRONT_END_CSC_STATE_CMD()
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

mhw_vebox_g12_X::VEBOX_GAMUT_CONTROL_STATE_CMD::VEBOX_GAMUT_CONTROL_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.C1                                           = C1_1825665536;
    DW0.GlobalModeEnable                             = GLOBAL_MODE_ENABLE_ADVANCEMODE;
    DW0.AR                                           = AR_UNNAMED436;

    DW1.Value                                        = 0;        
    DW1.C0                                           = C0_4467265536;

    DW2.Value                                        = 0;        
    DW2.C3                                           = C3_113665536;

    DW3.Value                                        = 0;        
    DW3.C2                                           = C2_54465536;

    DW4.Value                                        = 0;        
    DW4.C5                                           = C5_83265536;

    DW5.Value                                        = 0;        
    DW5.C4                                           = C4_5380865536;

    DW6.Value                                        = 0;        
    DW6.C7                                           = C7_268865536;

    DW7.Value                                        = 0;        
    DW7.C6                                           = C6_19265536;

    DW8.Value                                        = 0;        
    DW8.C8                                           = C8_5494465536;

    DW9.Value                                        = 0;        
    DW9.OffsetInR                                    = OFFSET_IN_R_UNNAMED0;

    DW10.Value                                       = 0;        
    DW10.OffsetInG                                   = OFFSET_IN_G_UNNAMED0;

    DW11.Value                                       = 0;        
    DW11.OffsetInB                                   = OFFSET_IN_B_UNNAMED0;

    DW12.Value                                       = 0;        
    DW12.OffsetOutR                                  = OFFSET_OUT_R_UNNAMED0;

    DW13.Value                                       = 0;        
    DW13.OffsetOutG                                  = OFFSET_OUT_G_UNNAMED0;

    DW14.Value                                       = 0;        
    DW14.OffsetOutB                                  = OFFSET_OUT_B_UNNAMED0;

    DW15.Value                                       = 0;        
    DW15.D1Out                                       = D1OUT_UNNAMED287;
    DW15.DOutDefault                                 = DOUT_DEFAULT_UNNAMED164;
    DW15.DInDefault                                  = DINDEFAULT_UNNAMED205;
    DW15.Fullrangemappingenable                      = FULLRANGEMAPPINGENABLE_BASICMODE;

    DW16.Value                                       = 0;        
    DW16.D1In                                        = D1IN_UNNAMED820;
    DW16.Compressionlineshift                        = COMPRESSIONLINESHIFT_UNNAMED3;
    DW16.Xvyccdecencenable                           = XVYCCDECENCENABLE_BOTHXVYCCDECODEANDXVYCCENCODEAREENABLED;

    DW17.Value                                       = 0;        
    DW17.CpiOverride                                 = CPI_OVERRIDE_UNNAMED0;
    DW17.Lumachormaonlycorrection                    = LUMACHORMAONLYCORRECTION_LUMAONLYCORRECTION;
    DW17.GccBasicmodeselection                       = GCC_BASICMODESELECTION_DEFAULT;

}

mhw_vebox_g12_X::VEBOX_STD_STE_STATE_CMD::VEBOX_STD_STE_STATE_CMD()
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
    DW14.SkinTypesThresh                             = SKIN_TYPES_THRESH_UNNAMED120;
    DW14.SkinTypesMargin                             = SKIN_TYPES_MARGIN_UNNAMED20;

    DW15.Value                                       = 0;        
    DW15.Satp1                                       = SATP1_UNNAMED6;
    DW15.Satp2                                       = SATP2_UNNAMED6;
    DW15.Satp3                                       = SATP3_UNNAMED31;
    DW15.Satb1                                       = SATB1_UNNAMED8;

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

mhw_vebox_g12_X::VEBOX_TCC_STATE_CMD::VEBOX_TCC_STATE_CMD()
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

mhw_vebox_g12_X::VEBOX_PROCAMP_STATE_CMD::VEBOX_PROCAMP_STATE_CMD()
{
    DW0.Value                                        = 0;        
    DW0.Brightness                                   = BRIGHTNESS_OR00;
    DW0.Contrast                                     = CONTRAST_10INFIXEDPOINTU47;

    DW1.Value                                        = 0;        
    DW1.SinCS                                        = SIN_C_S_UNNAMED0;
    DW1.CosCS                                        = COS_C_S_UNNAMED256;

}

mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD::VEBOX_IECP_STATE_CMD()
{
}

mhw_vebox_g12_X::VEBOX_Ch_Dir_Filter_Coefficient_CMD::VEBOX_Ch_Dir_Filter_Coefficient_CMD()
{
    DW0.Value = 0;
    DW1.Value = 0;
}

mhw_vebox_g12_X::VEBOX_STATE_CMD::VEBOX_STATE_CMD()
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
    DW1.DiOutputFrames                               = DI_OUTPUT_FRAMES_OUTPUTBOTHFRAMES;
    DW1.SinglePipeEnable                             = SINGLE_PIPE_ENABLE_DEFAULT;
    DW1.GamutExpansionPosition                       = GAMUT_EXPANSION_POSITION_GAMUTEXPANSIONATTHEBACKENDOFIECPPIPE;

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

    DW16.Value                                       = 0;        

    DW17.Value                                       = 0;        
    DW17.ArbitrationPriorityControlForLut3D          = ARBITRATION_PRIORITY_CONTROL_FOR_3D_LUT_HIGHESTPRIORITY;

    DW18.Value                                       = 0;        
    DW18.ChromaUpsamplingCoSitedHorizontalOffset     = CHROMA_UPSAMPLING_CO_SITED_HORIZONTAL_OFFSET_UNNAMED0;
    DW18.ChromaUpsamplingCoSitedVerticalOffset       = CHROMA_UPSAMPLING_CO_SITED_VERTICAL_OFFSET_UNNAMED0;
    DW18.ChromaDownsamplingCoSitedHorizontalOffset   = CHROMA_DOWNSAMPLING_CO_SITED_HORIZONTAL_OFFSET_UNNAMED0;
    DW18.ChromaDownsamplingCoSitedVerticalOffset     = CHROMA_DOWNSAMPLING_CO_SITED_VERTICAL_OFFSET_UNNAMED0;
    DW18.BypassChromaUpsampling                      = BYPASS_CHROMA_UPSAMPLING_UNNAMED0;
    DW18.BypassChromaDownsampling                    = BYPASS_CHROMA_DOWNSAMPLING_UNNAMED0;
    DW18.Lut3DSize                                   = _3D_LUT_SIZE_33X33X33;

}

mhw_vebox_g12_X::VEBOX_SURFACE_STATE_CMD::VEBOX_SURFACE_STATE_CMD()
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
    DW3.BayerInputAlignment                          = BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
    DW3.BayerPatternFormat                           = BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
    DW3.BayerPatternOffset                           = BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
    DW3.SurfaceFormat                                = SURFACE_FORMAT_YCRCBNORMAL;

    DW4.Value                                        = 0;        

    DW5.Value                                        = 0;        

    DW6.Value                                        = 0;        

    DW7.Value                                        = 0;        

    DW8.Value                                        = 0;        

}

mhw_vebox_g12_X::VEBOX_TILING_CONVERT_CMD::VEBOX_TILING_CONVERT_CMD()
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

mhw_vebox_g12_X::VEB_DI_IECP_CMD::VEB_DI_IECP_CMD()
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

mhw_vebox_g12_X::Gamut_Expansion_Gamma_Correction_CMD::Gamut_Expansion_Gamma_Correction_CMD()
{
    DW0.Value                                        = 0;        
    DW0.InverseGChGammaCorrectedValue0               = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_0_UNNAMED0;
    DW0.InverseBChGammaCorrectedValue0               = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_0_UNNAMED0;

    DW1.Value                                        = 0;        
    DW1.InversePixelValue0                           = INVERSE_PIXEL_VALUE_0_UNNAMED0;
    DW1.InverseRChGammaCorrectedValue0               = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_0_UNNAMED0;

    DW2.Value                                        = 0;        
    DW2.ForwardGChGammaCorrectedValue0               = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_0_UNNAMED0;
    DW2.ForwardBChGammaCorrectedValue0               = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_0_UNNAMED0;

    DW3.Value                                        = 0;        
    DW3.ForwardPixelValue0                           = FORWARD_PIXEL_VALUE_0_UNNAMED0;
    DW3.ForwardRChGammaCorrectedValue0               = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_0_UNNAMED0;

    DW4.Value                                        = 0;        
    DW4.InverseGChGammaCorrectedValue1               = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_1_UNNAMED256;
    DW4.InverseBChGammaCorrectedValue1               = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_1_UNNAMED256;

    DW5.Value                                        = 0;        
    DW5.InversePixelValue1                           = INVERSE_PIXEL_VALUE_1_UNNAMED256;
    DW5.InverseRChGammaCorrectedValue1               = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_1_UNNAMED256;

    DW6.Value                                        = 0;        
    DW6.ForwardGChGammaCorrectedValue1               = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_1_UNNAMED256;
    DW6.ForwardBChGammaCorrectedValue1               = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_1_UNNAMED256;

    DW7.Value                                        = 0;        
    DW7.ForwardPixelValue1                           = FORWARD_PIXEL_VALUE_1_UNNAMED256;
    DW7.ForwardRChGammaCorrectedValue1               = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_1_UNNAMED256;

    DW8.Value                                        = 0;        
    DW8.InverseGChGammaCorrectedValue2               = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_2_UNNAMED512;
    DW8.InverseBChGammaCorrectedValue2               = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_2_UNNAMED512;

    DW9.Value                                        = 0;        
    DW9.InversePixelValue2                           = INVERSE_PIXEL_VALUE_2_UNNAMED512;
    DW9.InverseRChGammaCorrectedValue2               = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_2_UNNAMED512;

    DW10.Value                                       = 0;        
    DW10.ForwardGChGammaCorrectedValue2              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_2_UNNAMED512;
    DW10.ForwardBChGammaCorrectedValue2              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_2_UNNAMED512;

    DW11.Value                                       = 0;        
    DW11.ForwardPixelValue2                          = FORWARD_PIXEL_VALUE_2_UNNAMED512;
    DW11.ForwardRChGammaCorrectedValue2              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_2_UNNAMED512;

    DW12.Value                                       = 0;        
    DW12.InverseGChGammaCorrectedValue3              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_3_UNNAMED768;
    DW12.InverseBChGammaCorrectedValue3              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_3_UNNAMED768;

    DW13.Value                                       = 0;        
    DW13.InversePixelValue3                          = INVERSE_PIXEL_VALUE_3_UNNAMED768;
    DW13.InverseRChGammaCorrectedValue3              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_3_UNNAMED768;

    DW14.Value                                       = 0;        
    DW14.ForwardGChGammaCorrectedValue3              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_3_UNNAMED768;
    DW14.ForwardBChGammaCorrectedValue3              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_3_UNNAMED768;

    DW15.Value                                       = 0;        
    DW15.ForwardPixelValue3                          = FORWARD_PIXEL_VALUE_3_UNNAMED768;
    DW15.ForwardRChGammaCorrectedValue3              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_3_UNNAMED768;

    DW16.Value                                       = 0;        
    DW16.InverseGChGammaCorrectedValue4              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_4_UNNAMED1024;
    DW16.InverseBChGammaCorrectedValue4              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_4_UNNAMED1024;

    DW17.Value                                       = 0;        
    DW17.InversePixelValue4                          = INVERSE_PIXEL_VALUE_4_UNNAMED1024;
    DW17.InverseRChGammaCorrectedValue4              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_4_UNNAMED1024;

    DW18.Value                                       = 0;        
    DW18.ForwardGChGammaCorrectedValue4              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_4_UNNAMED1024;
    DW18.ForwardBChGammaCorrectedValue4              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_4_UNNAMED1024;

    DW19.Value                                       = 0;        
    DW19.ForwardPixelValue4                          = FORWARD_PIXEL_VALUE_4_UNNAMED1024;
    DW19.ForwardRChGammaCorrectedValue4              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_4_UNNAMED1024;

    DW20.Value                                       = 0;        
    DW20.InverseGChGammaCorrectedValue5              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_5_UNNAMED1280;
    DW20.InverseBChGammaCorrectedValue5              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_5_UNNAMED1280;

    DW21.Value                                       = 0;        
    DW21.InversePixelValue5                          = INVERSE_PIXEL_VALUE_5_UNNAMED1280;
    DW21.InverseRChGammaCorrectedValue5              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_5_UNNAMED1280;

    DW22.Value                                       = 0;        
    DW22.ForwardGChGammaCorrectedValue5              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_5_UNNAMED1280;
    DW22.ForwardBChGammaCorrectedValue5              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_5_UNNAMED1280;

    DW23.Value                                       = 0;        
    DW23.ForwardPixelValue5                          = FORWARD_PIXEL_VALUE_5_UNNAMED1280;
    DW23.ForwardRChGammaCorrectedValue5              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_5_UNNAMED1280;

    DW24.Value                                       = 0;        
    DW24.InverseGChGammaCorrectedValue6              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_6_UNNAMED1536;
    DW24.InverseBChGammaCorrectedValue6              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_6_UNNAMED1536;

    DW25.Value                                       = 0;        
    DW25.InversePixelValue6                          = INVERSE_PIXEL_VALUE_6_UNNAMED1536;
    DW25.InverseRChGammaCorrectedValue6              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_6_UNNAMED1536;

    DW26.Value                                       = 0;        
    DW26.ForwardGChGammaCorrectedValue6              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_6_UNNAMED1536;
    DW26.ForwardBChGammaCorrectedValue6              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_6_UNNAMED1536;

    DW27.Value                                       = 0;        
    DW27.ForwardPixelValue6                          = FORWARD_PIXEL_VALUE_6_UNNAMED1536;
    DW27.ForwardRChGammaCorrectedValue6              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_6_UNNAMED1536;

    DW28.Value                                       = 0;        
    DW28.InverseGChGammaCorrectedValue7              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_7_UNNAMED1792;
    DW28.InverseBChGammaCorrectedValue7              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_7_UNNAMED1792;

    DW29.Value                                       = 0;        
    DW29.InversePixelValue7                          = INVERSE_PIXEL_VALUE_7_UNNAMED1792;
    DW29.InverseRChGammaCorrectedValue7              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_7_UNNAMED1792;

    DW30.Value                                       = 0;        
    DW30.ForwardGChGammaCorrectedValue7              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_7_UNNAMED1792;
    DW30.ForwardBChGammaCorrectedValue7              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_7_UNNAMED1792;

    DW31.Value                                       = 0;        
    DW31.ForwardPixelValue7                          = FORWARD_PIXEL_VALUE_7_UNNAMED1792;
    DW31.ForwardRChGammaCorrectedValue7              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_7_UNNAMED1792;

    DW32.Value                                       = 0;        
    DW32.InverseGChGammaCorrectedValue8              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_8_UNNAMED2048;
    DW32.InverseBChGammaCorrectedValue8              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_8_UNNAMED2048;

    DW33.Value                                       = 0;        
    DW33.InversePixelValue8                          = INVERSE_PIXEL_VALUE_8_UNNAMED2048;
    DW33.InverseRChGammaCorrectedValue8              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_8_UNNAMED2048;

    DW34.Value                                       = 0;        
    DW34.ForwardGChGammaCorrectedValue8              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_8_UNNAMED2048;
    DW34.ForwardBChGammaCorrectedValue8              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_8_UNNAMED2048;

    DW35.Value                                       = 0;        
    DW35.ForwardPixelValue8                          = FORWARD_PIXEL_VALUE_8_UNNAMED2048;
    DW35.ForwardRChGammaCorrectedValue8              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_8_UNNAMED2048;

    DW36.Value                                       = 0;        
    DW36.InverseGChGammaCorrectedValue9              = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_9_UNNAMED2304;
    DW36.InverseBChGammaCorrectedValue9              = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_9_UNNAMED2304;

    DW37.Value                                       = 0;        
    DW37.InversePixelValue9                          = INVERSE_PIXEL_VALUE_9_UNNAMED2304;
    DW37.InverseRChGammaCorrectedValue9              = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_9_UNNAMED2304;

    DW38.Value                                       = 0;        
    DW38.ForwardGChGammaCorrectedValue9              = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_9_UNNAMED2304;
    DW38.ForwardBChGammaCorrectedValue9              = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_9_UNNAMED2304;

    DW39.Value                                       = 0;        
    DW39.ForwardPixelValue9                          = FORWARD_PIXEL_VALUE_9_UNNAMED2304;
    DW39.ForwardRChGammaCorrectedValue9              = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_9_UNNAMED2304;

    DW40.Value                                       = 0;        
    DW40.InverseGChGammaCorrectedValue10             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_10_UNNAMED2560;
    DW40.InverseBChGammaCorrectedValue10             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_10_UNNAMED2560;

    DW41.Value                                       = 0;        
    DW41.InversePixelValue10                         = INVERSE_PIXEL_VALUE_10_UNNAMED2560;
    DW41.InverseRChGammaCorrectedValue10             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_10_UNNAMED2560;

    DW42.Value                                       = 0;        
    DW42.ForwardGChGammaCorrectedValue10             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_10_UNNAMED2560;
    DW42.ForwardBChGammaCorrectedValue10             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_10_UNNAMED2560;

    DW43.Value                                       = 0;        
    DW43.ForwardPixelValue10                         = FORWARD_PIXEL_VALUE_10_UNNAMED2560;
    DW43.ForwardRChGammaCorrectedValue10             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_10_UNNAMED2560;

    DW44.Value                                       = 0;        
    DW44.InverseGChGammaCorrectedValue11             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_11_UNNAMED2816;
    DW44.InverseBChGammaCorrectedValue11             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_11_UNNAMED2816;

    DW45.Value                                       = 0;        
    DW45.InversePixelValue11                         = INVERSE_PIXEL_VALUE_11_UNNAMED2816;
    DW45.InverseRChGammaCorrectedValue11             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_11_UNNAMED2816;

    DW46.Value                                       = 0;        
    DW46.ForwardGChGammaCorrectedValue11             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_11_UNNAMED2816;
    DW46.ForwardBChGammaCorrectedValue11             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_11_UNNAMED2816;

    DW47.Value                                       = 0;        
    DW47.ForwardPixelValue11                         = FORWARD_PIXEL_VALUE_11_UNNAMED2816;
    DW47.ForwardRChGammaCorrectedValue11             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_11_UNNAMED2816;

    DW48.Value                                       = 0;        
    DW48.InverseGChGammaCorrectedValue12             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_12_UNNAMED3072;
    DW48.InverseBChGammaCorrectedValue12             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_12_UNNAMED3072;

    DW49.Value                                       = 0;        
    DW49.InversePixelValue12                         = INVERSE_PIXEL_VALUE_12_UNNAMED3072;
    DW49.InverseRChGammaCorrectedValue12             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_12_UNNAMED3072;

    DW50.Value                                       = 0;        
    DW50.ForwardGChGammaCorrectedValue12             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_12_UNNAMED3072;
    DW50.ForwardBChGammaCorrectedValue12             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_12_UNNAMED3072;

    DW51.Value                                       = 0;        
    DW51.ForwardPixelValue12                         = FORWARD_PIXEL_VALUE_12_UNNAMED3072;
    DW51.ForwardRChGammaCorrectedValue12             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_12_UNNAMED3072;

    DW52.Value                                       = 0;        
    DW52.InverseGChGammaCorrectedValue13             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_13_UNNAMED3328;
    DW52.InverseBChGammaCorrectedValue13             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_13_UNNAMED3328;

    DW53.Value                                       = 0;        
    DW53.InversePixelValue13                         = INVERSE_PIXEL_VALUE_13_UNNAMED3328;
    DW53.InverseRChGammaCorrectedValue13             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_13_UNNAMED3328;

    DW54.Value                                       = 0;        
    DW54.ForwardGChGammaCorrectedValue13             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_13_UNNAMED3328;
    DW54.ForwardBChGammaCorrectedValue13             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_13_UNNAMED3328;

    DW55.Value                                       = 0;        
    DW55.ForwardPixelValue13                         = FORWARD_PIXEL_VALUE_13_UNNAMED3328;
    DW55.ForwardRChGammaCorrectedValue13             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_13_UNNAMED3328;

    DW56.Value                                       = 0;        
    DW56.InverseGChGammaCorrectedValue14             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_14_UNNAMED3584;
    DW56.InverseBChGammaCorrectedValue14             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_14_UNNAMED3584;

    DW57.Value                                       = 0;        
    DW57.InversePixelValue14                         = INVERSE_PIXEL_VALUE_14_UNNAMED3584;
    DW57.InverseRChGammaCorrectedValue14             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_14_UNNAMED3584;

    DW58.Value                                       = 0;        
    DW58.ForwardGChGammaCorrectedValue14             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_14_UNNAMED3584;
    DW58.ForwardBChGammaCorrectedValue14             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_14_UNNAMED3584;

    DW59.Value                                       = 0;        
    DW59.ForwardPixelValue14                         = FORWARD_PIXEL_VALUE_14_UNNAMED3584;
    DW59.ForwardRChGammaCorrectedValue14             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_14_UNNAMED3584;

    DW60.Value                                       = 0;        
    DW60.InverseGChGammaCorrectedValue15             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_15_UNNAMED3840;
    DW60.InverseBChGammaCorrectedValue15             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_15_UNNAMED3840;

    DW61.Value                                       = 0;        
    DW61.InversePixelValue15                         = INVERSE_PIXEL_VALUE_15_UNNAMED3840;
    DW61.InverseRChGammaCorrectedValue15             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_15_UNNAMED3840;

    DW62.Value                                       = 0;        
    DW62.ForwardGChGammaCorrectedValue15             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_15_UNNAMED3840;
    DW62.ForwardBChGammaCorrectedValue15             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_15_UNNAMED3840;

    DW63.Value                                       = 0;        
    DW63.ForwardPixelValue15                         = FORWARD_PIXEL_VALUE_15_UNNAMED3840;
    DW63.ForwardRChGammaCorrectedValue15             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_15_UNNAMED3840;

    DW64.Value                                       = 0;        
    DW64.InverseGChGammaCorrectedValue16             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_16_UNNAMED4096;
    DW64.InverseBChGammaCorrectedValue16             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_16_UNNAMED4096;

    DW65.Value                                       = 0;        
    DW65.InversePixelValue16                         = INVERSE_PIXEL_VALUE_16_UNNAMED4096;
    DW65.InverseRChGammaCorrectedValue16             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_16_UNNAMED4096;

    DW66.Value                                       = 0;        
    DW66.ForwardGChGammaCorrectedValue16             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_16_UNNAMED4096;
    DW66.ForwardBChGammaCorrectedValue16             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_16_UNNAMED4096;

    DW67.Value                                       = 0;        
    DW67.ForwardPixelValue16                         = FORWARD_PIXEL_VALUE_16_UNNAMED4096;
    DW67.ForwardRChGammaCorrectedValue16             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_16_UNNAMED4096;

    DW68.Value                                       = 0;        
    DW68.InverseGChGammaCorrectedValue17             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_17_UNNAMED4352;
    DW68.InverseBChGammaCorrectedValue17             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_17_UNNAMED4352;

    DW69.Value                                       = 0;        
    DW69.InversePixelValue17                         = INVERSE_PIXEL_VALUE_17_UNNAMED4352;
    DW69.InverseRChGammaCorrectedValue17             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_17_UNNAMED4352;

    DW70.Value                                       = 0;        
    DW70.ForwardGChGammaCorrectedValue17             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_17_UNNAMED4352;
    DW70.ForwardBChGammaCorrectedValue17             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_17_UNNAMED4352;

    DW71.Value                                       = 0;        
    DW71.ForwardPixelValue17                         = FORWARD_PIXEL_VALUE_17_UNNAMED4352;
    DW71.ForwardRChGammaCorrectedValue17             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_17_UNNAMED4352;

    DW72.Value                                       = 0;        
    DW72.InverseGChGammaCorrectedValue18             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_18_UNNAMED4608;
    DW72.InverseBChGammaCorrectedValue18             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_18_UNNAMED4608;

    DW73.Value                                       = 0;        
    DW73.InversePixelValue18                         = INVERSE_PIXEL_VALUE_18_UNNAMED4608;
    DW73.InverseRChGammaCorrectedValue18             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_18_UNNAMED4608;

    DW74.Value                                       = 0;        
    DW74.ForwardGChGammaCorrectedValue18             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_18_UNNAMED4608;
    DW74.ForwardBChGammaCorrectedValue18             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_18_UNNAMED4608;

    DW75.Value                                       = 0;        
    DW75.ForwardPixelValue18                         = FORWARD_PIXEL_VALUE_18_UNNAMED4608;
    DW75.ForwardRChGammaCorrectedValue18             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_18_UNNAMED4608;

    DW76.Value                                       = 0;        
    DW76.InverseGChGammaCorrectedValue19             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_19_UNNAMED4864;
    DW76.InverseBChGammaCorrectedValue19             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_19_UNNAMED4864;

    DW77.Value                                       = 0;        
    DW77.InversePixelValue19                         = INVERSE_PIXEL_VALUE_19_UNNAMED4864;
    DW77.InverseRChGammaCorrectedValue19             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_19_UNNAMED4864;

    DW78.Value                                       = 0;        
    DW78.ForwardGChGammaCorrectedValue19             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_19_UNNAMED4864;
    DW78.ForwardBChGammaCorrectedValue19             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_19_UNNAMED4864;

    DW79.Value                                       = 0;        
    DW79.ForwardPixelValue19                         = FORWARD_PIXEL_VALUE_19_UNNAMED4864;
    DW79.ForwardRChGammaCorrectedValue19             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_19_UNNAMED4864;

    DW80.Value                                       = 0;        
    DW80.InverseGChGammaCorrectedValue20             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_20_UNNAMED5120;
    DW80.InverseBChGammaCorrectedValue20             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_20_UNNAMED5120;

    DW81.Value                                       = 0;        
    DW81.InversePixelValue20                         = INVERSE_PIXEL_VALUE_20_UNNAMED5120;
    DW81.InverseRChGammaCorrectedValue20             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_20_UNNAMED5120;

    DW82.Value                                       = 0;        
    DW82.ForwardGChGammaCorrectedValue20             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_20_UNNAMED5120;
    DW82.ForwardBChGammaCorrectedValue20             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_20_UNNAMED5120;

    DW83.Value                                       = 0;        
    DW83.ForwardPixelValue20                         = FORWARD_PIXEL_VALUE_20_UNNAMED5120;
    DW83.ForwardRChGammaCorrectedValue20             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_20_UNNAMED5120;

    DW84.Value                                       = 0;        
    DW84.InverseGChGammaCorrectedValue21             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_21_UNNAMED5376;
    DW84.InverseBChGammaCorrectedValue21             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_21_UNNAMED5376;

    DW85.Value                                       = 0;        
    DW85.InversePixelValue21                         = INVERSE_PIXEL_VALUE_21_UNNAMED5376;
    DW85.InverseRChGammaCorrectedValue21             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_21_UNNAMED5376;

    DW86.Value                                       = 0;        
    DW86.ForwardGChGammaCorrectedValue21             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_21_UNNAMED5376;
    DW86.ForwardBChGammaCorrectedValue21             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_21_UNNAMED5376;

    DW87.Value                                       = 0;        
    DW87.ForwardPixelValue21                         = FORWARD_PIXEL_VALUE_21_UNNAMED5376;
    DW87.ForwardRChGammaCorrectedValue21             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_21_UNNAMED5376;

    DW88.Value                                       = 0;        
    DW88.InverseGChGammaCorrectedValue22             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_22_UNNAMED5632;
    DW88.InverseBChGammaCorrectedValue22             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_22_UNNAMED5632;

    DW89.Value                                       = 0;        
    DW89.InversePixelValue22                         = INVERSE_PIXEL_VALUE_22_UNNAMED5632;
    DW89.InverseRChGammaCorrectedValue22             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_22_UNNAMED5632;

    DW90.Value                                       = 0;        
    DW90.ForwardGChGammaCorrectedValue22             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_22_UNNAMED5632;
    DW90.ForwardBChGammaCorrectedValue22             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_22_UNNAMED5632;

    DW91.Value                                       = 0;        
    DW91.ForwardPixelValue22                         = FORWARD_PIXEL_VALUE_22_UNNAMED5632;
    DW91.ForwardRChGammaCorrectedValue22             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_22_UNNAMED5632;

    DW92.Value                                       = 0;        
    DW92.InverseGChGammaCorrectedValue23             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_23_UNNAMED5888;
    DW92.InverseBChGammaCorrectedValue23             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_23_UNNAMED5888;

    DW93.Value                                       = 0;        
    DW93.InversePixelValue23                         = INVERSE_PIXEL_VALUE_23_UNNAMED5888;
    DW93.InverseRChGammaCorrectedValue23             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_23_UNNAMED5888;

    DW94.Value                                       = 0;        
    DW94.ForwardGChGammaCorrectedValue23             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_23_UNNAMED5888;
    DW94.ForwardBChGammaCorrectedValue23             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_23_UNNAMED5888;

    DW95.Value                                       = 0;        
    DW95.ForwardPixelValue23                         = FORWARD_PIXEL_VALUE_23_UNNAMED5888;
    DW95.ForwardRChGammaCorrectedValue23             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_23_UNNAMED5888;

    DW96.Value                                       = 0;        
    DW96.InverseGChGammaCorrectedValue24             = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_24_UNNAMED6144;
    DW96.InverseBChGammaCorrectedValue24             = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_24_UNNAMED6144;

    DW97.Value                                       = 0;        
    DW97.InversePixelValue24                         = INVERSE_PIXEL_VALUE_24_UNNAMED6144;
    DW97.InverseRChGammaCorrectedValue24             = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_24_UNNAMED6144;

    DW98.Value                                       = 0;        
    DW98.ForwardGChGammaCorrectedValue24             = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_24_UNNAMED6144;
    DW98.ForwardBChGammaCorrectedValue24             = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_24_UNNAMED6144;

    DW99.Value                                       = 0;        
    DW99.ForwardPixelValue24                         = FORWARD_PIXEL_VALUE_24_UNNAMED6144;
    DW99.ForwardRChGammaCorrectedValue24             = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_24_UNNAMED6144;

    DW100.Value                                      = 0;        
    DW100.InverseGChGammaCorrectedValue25            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_25_UNNAMED6400;
    DW100.InverseBChGammaCorrectedValue25            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_25_UNNAMED6400;

    DW101.Value                                      = 0;        
    DW101.InversePixelValue25                        = INVERSE_PIXEL_VALUE_25_UNNAMED6400;
    DW101.InverseRChGammaCorrectedValue25            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_25_UNNAMED6400;

    DW102.Value                                      = 0;        
    DW102.ForwardGChGammaCorrectedValue25            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_25_UNNAMED6400;
    DW102.ForwardBChGammaCorrectedValue25            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_25_UNNAMED6400;

    DW103.Value                                      = 0;        
    DW103.ForwardPixelValue25                        = FORWARD_PIXEL_VALUE_25_UNNAMED6400;
    DW103.ForwardRChGammaCorrectedValue25            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_25_UNNAMED6400;

    DW104.Value                                      = 0;        
    DW104.InverseGChGammaCorrectedValue26            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_26_UNNAMED6656;
    DW104.InverseBChGammaCorrectedValue26            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_26_UNNAMED6656;

    DW105.Value                                      = 0;        
    DW105.InversePixelValue26                        = INVERSE_PIXEL_VALUE_26_UNNAMED6656;
    DW105.InverseRChGammaCorrectedValue26            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_26_UNNAMED6656;

    DW106.Value                                      = 0;        
    DW106.ForwardGChGammaCorrectedValue26            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_26_UNNAMED6656;
    DW106.ForwardBChGammaCorrectedValue26            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_26_UNNAMED6656;

    DW107.Value                                      = 0;        
    DW107.ForwardPixelValue26                        = FORWARD_PIXEL_VALUE_26_UNNAMED6656;
    DW107.ForwardRChGammaCorrectedValue26            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_26_UNNAMED6656;

    DW108.Value                                      = 0;        
    DW108.InverseGChGammaCorrectedValue27            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_27_UNNAMED6912;
    DW108.InverseBChGammaCorrectedValue27            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_27_UNNAMED6912;

    DW109.Value                                      = 0;        
    DW109.InversePixelValue27                        = INVERSE_PIXEL_VALUE_27_UNNAMED6912;
    DW109.InverseRChGammaCorrectedValue27            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_27_UNNAMED6912;

    DW110.Value                                      = 0;        
    DW110.ForwardGChGammaCorrectedValue27            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_27_UNNAMED6912;
    DW110.ForwardBChGammaCorrectedValue27            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_27_UNNAMED6912;

    DW111.Value                                      = 0;        
    DW111.ForwardPixelValue27                        = FORWARD_PIXEL_VALUE_27_UNNAMED6912;
    DW111.ForwardRChGammaCorrectedValue27            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_27_UNNAMED6912;

    DW112.Value                                      = 0;        
    DW112.InverseGChGammaCorrectedValue28            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_28_UNNAMED7168;
    DW112.InverseBChGammaCorrectedValue28            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_28_UNNAMED7168;

    DW113.Value                                      = 0;        
    DW113.InversePixelValue28                        = INVERSE_PIXEL_VALUE_28_UNNAMED7168;
    DW113.InverseRChGammaCorrectedValue28            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_28_UNNAMED7168;

    DW114.Value                                      = 0;        
    DW114.ForwardGChGammaCorrectedValue28            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_28_UNNAMED7168;
    DW114.ForwardBChGammaCorrectedValue28            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_28_UNNAMED7168;

    DW115.Value                                      = 0;        
    DW115.ForwardPixelValue28                        = FORWARD_PIXEL_VALUE_28_UNNAMED7168;
    DW115.ForwardRChGammaCorrectedValue28            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_28_UNNAMED7168;

    DW116.Value                                      = 0;        
    DW116.InverseGChGammaCorrectedValue29            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_29_UNNAMED7424;
    DW116.InverseBChGammaCorrectedValue29            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_29_UNNAMED7424;

    DW117.Value                                      = 0;        
    DW117.InversePixelValue29                        = INVERSE_PIXEL_VALUE_29_UNNAMED7424;
    DW117.InverseRChGammaCorrectedValue29            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_29_UNNAMED7424;

    DW118.Value                                      = 0;        
    DW118.ForwardGChGammaCorrectedValue29            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_29_UNNAMED7424;
    DW118.ForwardBChGammaCorrectedValue29            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_29_UNNAMED7424;

    DW119.Value                                      = 0;        
    DW119.ForwardPixelValue29                        = FORWARD_PIXEL_VALUE_29_UNNAMED7424;
    DW119.ForwardRChGammaCorrectedValue29            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_29_UNNAMED7424;

    DW120.Value                                      = 0;        
    DW120.InverseGChGammaCorrectedValue30            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_30_UNNAMED7680;
    DW120.InverseBChGammaCorrectedValue30            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_30_UNNAMED7680;

    DW121.Value                                      = 0;        
    DW121.InversePixelValue30                        = INVERSE_PIXEL_VALUE_30_UNNAMED7680;
    DW121.InverseRChGammaCorrectedValue30            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_30_UNNAMED7680;

    DW122.Value                                      = 0;        
    DW122.ForwardGChGammaCorrectedValue30            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_30_UNNAMED7680;
    DW122.ForwardBChGammaCorrectedValue30            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_30_UNNAMED7680;

    DW123.Value                                      = 0;        
    DW123.ForwardPixelValue30                        = FORWARD_PIXEL_VALUE_30_UNNAMED7680;
    DW123.ForwardRChGammaCorrectedValue30            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_30_UNNAMED7680;

    DW124.Value                                      = 0;        
    DW124.InverseGChGammaCorrectedValue31            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_31_UNNAMED7936;
    DW124.InverseBChGammaCorrectedValue31            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_31_UNNAMED7936;

    DW125.Value                                      = 0;        
    DW125.InversePixelValue31                        = INVERSE_PIXEL_VALUE_31_UNNAMED7936;
    DW125.InverseRChGammaCorrectedValue31            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_31_UNNAMED7936;

    DW126.Value                                      = 0;        
    DW126.ForwardGChGammaCorrectedValue31            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_31_UNNAMED7936;
    DW126.ForwardBChGammaCorrectedValue31            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_31_UNNAMED7936;

    DW127.Value                                      = 0;        
    DW127.ForwardPixelValue31                        = FORWARD_PIXEL_VALUE_31_UNNAMED7936;
    DW127.ForwardRChGammaCorrectedValue31            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_31_UNNAMED7936;

    DW128.Value                                      = 0;        
    DW128.InverseGChGammaCorrectedValue32            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_32_UNNAMED8192;
    DW128.InverseBChGammaCorrectedValue32            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_32_UNNAMED8192;

    DW129.Value                                      = 0;        
    DW129.InversePixelValue32                        = INVERSE_PIXEL_VALUE_32_UNNAMED8192;
    DW129.InverseRChGammaCorrectedValue32            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_32_UNNAMED8192;

    DW130.Value                                      = 0;        
    DW130.ForwardGChGammaCorrectedValue32            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_32_UNNAMED8192;
    DW130.ForwardBChGammaCorrectedValue32            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_32_UNNAMED8192;

    DW131.Value                                      = 0;        
    DW131.ForwardPixelValue32                        = FORWARD_PIXEL_VALUE_32_UNNAMED8192;
    DW131.ForwardRChGammaCorrectedValue32            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_32_UNNAMED8192;

    DW132.Value                                      = 0;        
    DW132.InverseGChGammaCorrectedValue33            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_33_UNNAMED8448;
    DW132.InverseBChGammaCorrectedValue33            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_33_UNNAMED8448;

    DW133.Value                                      = 0;        
    DW133.InversePixelValue33                        = INVERSE_PIXEL_VALUE_33_UNNAMED8448;
    DW133.InverseRChGammaCorrectedValue33            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_33_UNNAMED8448;

    DW134.Value                                      = 0;        
    DW134.ForwardGChGammaCorrectedValue33            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_33_UNNAMED8448;
    DW134.ForwardBChGammaCorrectedValue33            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_33_UNNAMED8448;

    DW135.Value                                      = 0;        
    DW135.ForwardPixelValue33                        = FORWARD_PIXEL_VALUE_33_UNNAMED8448;
    DW135.ForwardRChGammaCorrectedValue33            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_33_UNNAMED8448;

    DW136.Value                                      = 0;        
    DW136.InverseGChGammaCorrectedValue34            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_34_UNNAMED8704;
    DW136.InverseBChGammaCorrectedValue34            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_34_UNNAMED8704;

    DW137.Value                                      = 0;        
    DW137.InversePixelValue34                        = INVERSE_PIXEL_VALUE_34_UNNAMED8704;
    DW137.InverseRChGammaCorrectedValue34            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_34_UNNAMED8704;

    DW138.Value                                      = 0;        
    DW138.ForwardGChGammaCorrectedValue34            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_34_UNNAMED8704;
    DW138.ForwardBChGammaCorrectedValue34            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_34_UNNAMED8704;

    DW139.Value                                      = 0;        
    DW139.ForwardPixelValue34                        = FORWARD_PIXEL_VALUE_34_UNNAMED8704;
    DW139.ForwardRChGammaCorrectedValue34            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_34_UNNAMED8704;

    DW140.Value                                      = 0;        
    DW140.InverseGChGammaCorrectedValue35            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_35_UNNAMED8960;
    DW140.InverseBChGammaCorrectedValue35            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_35_UNNAMED8960;

    DW141.Value                                      = 0;        
    DW141.InversePixelValue35                        = INVERSE_PIXEL_VALUE_35_UNNAMED8960;
    DW141.InverseRChGammaCorrectedValue35            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_35_UNNAMED8960;

    DW142.Value                                      = 0;        
    DW142.ForwardGChGammaCorrectedValue35            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_35_UNNAMED8960;
    DW142.ForwardBChGammaCorrectedValue35            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_35_UNNAMED8960;

    DW143.Value                                      = 0;        
    DW143.ForwardPixelValue35                        = FORWARD_PIXEL_VALUE_35_UNNAMED8960;
    DW143.ForwardRChGammaCorrectedValue35            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_35_UNNAMED8960;

    DW144.Value                                      = 0;        
    DW144.InverseGChGammaCorrectedValue36            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_36_UNNAMED9216;
    DW144.InverseBChGammaCorrectedValue36            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_36_UNNAMED9216;

    DW145.Value                                      = 0;        
    DW145.InversePixelValue36                        = INVERSE_PIXEL_VALUE_36_UNNAMED9216;
    DW145.InverseRChGammaCorrectedValue36            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_36_UNNAMED9216;

    DW146.Value                                      = 0;        
    DW146.ForwardGChGammaCorrectedValue36            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_36_UNNAMED9216;
    DW146.ForwardBChGammaCorrectedValue36            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_36_UNNAMED9216;

    DW147.Value                                      = 0;        
    DW147.ForwardPixelValue36                        = FORWARD_PIXEL_VALUE_36_UNNAMED9216;
    DW147.ForwardRChGammaCorrectedValue36            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_36_UNNAMED9216;

    DW148.Value                                      = 0;        
    DW148.InverseGChGammaCorrectedValue37            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_37_UNNAMED9472;
    DW148.InverseBChGammaCorrectedValue37            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_37_UNNAMED9472;

    DW149.Value                                      = 0;        
    DW149.InversePixelValue37                        = INVERSE_PIXEL_VALUE_37_UNNAMED9472;
    DW149.InverseRChGammaCorrectedValue37            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_37_UNNAMED9472;

    DW150.Value                                      = 0;        
    DW150.ForwardGChGammaCorrectedValue37            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_37_UNNAMED9472;
    DW150.ForwardBChGammaCorrectedValue37            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_37_UNNAMED9472;

    DW151.Value                                      = 0;        
    DW151.ForwardPixelValue37                        = FORWARD_PIXEL_VALUE_37_UNNAMED9472;
    DW151.ForwardRChGammaCorrectedValue37            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_37_UNNAMED9472;

    DW152.Value                                      = 0;        
    DW152.InverseGChGammaCorrectedValue38            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_38_UNNAMED9728;
    DW152.InverseBChGammaCorrectedValue38            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_38_UNNAMED9728;

    DW153.Value                                      = 0;        
    DW153.InversePixelValue38                        = INVERSE_PIXEL_VALUE_38_UNNAMED9728;
    DW153.InverseRChGammaCorrectedValue38            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_38_UNNAMED9728;

    DW154.Value                                      = 0;        
    DW154.ForwardGChGammaCorrectedValue38            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_38_UNNAMED9728;
    DW154.ForwardBChGammaCorrectedValue38            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_38_UNNAMED9728;

    DW155.Value                                      = 0;        
    DW155.ForwardPixelValue38                        = FORWARD_PIXEL_VALUE_38_UNNAMED9728;
    DW155.ForwardRChGammaCorrectedValue38            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_38_UNNAMED9728;

    DW156.Value                                      = 0;        
    DW156.InverseGChGammaCorrectedValue39            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_39_UNNAMED9984;
    DW156.InverseBChGammaCorrectedValue39            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_39_UNNAMED9984;

    DW157.Value                                      = 0;        
    DW157.InversePixelValue39                        = INVERSE_PIXEL_VALUE_39_UNNAMED9984;
    DW157.InverseRChGammaCorrectedValue39            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_39_UNNAMED9984;

    DW158.Value                                      = 0;        
    DW158.ForwardGChGammaCorrectedValue39            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_39_UNNAMED9984;
    DW158.ForwardBChGammaCorrectedValue39            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_39_UNNAMED9984;

    DW159.Value                                      = 0;        
    DW159.ForwardPixelValue39                        = FORWARD_PIXEL_VALUE_39_UNNAMED9984;
    DW159.ForwardRChGammaCorrectedValue39            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_39_UNNAMED9984;

    DW160.Value                                      = 0;        
    DW160.InverseGChGammaCorrectedValue40            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_40_UNNAMED10240;
    DW160.InverseBChGammaCorrectedValue40            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_40_UNNAMED10240;

    DW161.Value                                      = 0;        
    DW161.InversePixelValue40                        = INVERSE_PIXEL_VALUE_40_UNNAMED10240;
    DW161.InverseRChGammaCorrectedValue40            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_40_UNNAMED10240;

    DW162.Value                                      = 0;        
    DW162.ForwardGChGammaCorrectedValue40            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_40_UNNAMED10240;
    DW162.ForwardBChGammaCorrectedValue40            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_40_UNNAMED10240;

    DW163.Value                                      = 0;        
    DW163.ForwardPixelValue40                        = FORWARD_PIXEL_VALUE_40_UNNAMED10240;
    DW163.ForwardRChGammaCorrectedValue40            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_40_UNNAMED10240;

    DW164.Value                                      = 0;        
    DW164.InverseGChGammaCorrectedValue41            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_41_UNNAMED10496;
    DW164.InverseBChGammaCorrectedValue41            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_41_UNNAMED10496;

    DW165.Value                                      = 0;        
    DW165.InversePixelValue41                        = INVERSE_PIXEL_VALUE_41_UNNAMED10496;
    DW165.InverseRChGammaCorrectedValue41            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_41_UNNAMED10496;

    DW166.Value                                      = 0;        
    DW166.ForwardGChGammaCorrectedValue41            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_41_UNNAMED10496;
    DW166.ForwardBChGammaCorrectedValue41            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_41_UNNAMED10496;

    DW167.Value                                      = 0;        
    DW167.ForwardPixelValue41                        = FORWARD_PIXEL_VALUE_41_UNNAMED10496;
    DW167.ForwardRChGammaCorrectedValue41            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_41_UNNAMED10496;

    DW168.Value                                      = 0;        
    DW168.InverseGChGammaCorrectedValue42            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_42_UNNAMED10752;
    DW168.InverseBChGammaCorrectedValue42            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_42_UNNAMED10752;

    DW169.Value                                      = 0;        
    DW169.InversePixelValue42                        = INVERSE_PIXEL_VALUE_42_UNNAMED10752;
    DW169.InverseRChGammaCorrectedValue42            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_42_UNNAMED10752;

    DW170.Value                                      = 0;        
    DW170.ForwardGChGammaCorrectedValue42            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_42_UNNAMED10752;
    DW170.ForwardBChGammaCorrectedValue42            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_42_UNNAMED10752;

    DW171.Value                                      = 0;        
    DW171.ForwardPixelValue42                        = FORWARD_PIXEL_VALUE_42_UNNAMED10752;
    DW171.ForwardRChGammaCorrectedValue42            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_42_UNNAMED10752;

    DW172.Value                                      = 0;        
    DW172.InverseGChGammaCorrectedValue43            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_43_UNNAMED11008;
    DW172.InverseBChGammaCorrectedValue43            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_43_UNNAMED11008;

    DW173.Value                                      = 0;        
    DW173.InversePixelValue43                        = INVERSE_PIXEL_VALUE_43_UNNAMED11008;
    DW173.InverseRChGammaCorrectedValue43            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_43_UNNAMED11008;

    DW174.Value                                      = 0;        
    DW174.ForwardGChGammaCorrectedValue43            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_43_UNNAMED11008;
    DW174.ForwardBChGammaCorrectedValue43            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_43_UNNAMED11008;

    DW175.Value                                      = 0;        
    DW175.ForwardPixelValue43                        = FORWARD_PIXEL_VALUE_43_UNNAMED11008;
    DW175.ForwardRChGammaCorrectedValue43            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_43_UNNAMED11008;

    DW176.Value                                      = 0;        
    DW176.InverseGChGammaCorrectedValue44            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_44_UNNAMED11264;
    DW176.InverseBChGammaCorrectedValue44            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_44_UNNAMED11264;

    DW177.Value                                      = 0;        
    DW177.InversePixelValue44                        = INVERSE_PIXEL_VALUE_44_UNNAMED11264;
    DW177.InverseRChGammaCorrectedValue44            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_44_UNNAMED11264;

    DW178.Value                                      = 0;        
    DW178.ForwardGChGammaCorrectedValue44            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_44_UNNAMED11264;
    DW178.ForwardBChGammaCorrectedValue44            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_44_UNNAMED11264;

    DW179.Value                                      = 0;        
    DW179.ForwardPixelValue44                        = FORWARD_PIXEL_VALUE_44_UNNAMED11264;
    DW179.ForwardRChGammaCorrectedValue44            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_44_UNNAMED11264;

    DW180.Value                                      = 0;        
    DW180.InverseGChGammaCorrectedValue45            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_45_UNNAMED11520;
    DW180.InverseBChGammaCorrectedValue45            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_45_UNNAMED11520;

    DW181.Value                                      = 0;        
    DW181.InversePixelValue45                        = INVERSE_PIXEL_VALUE_45_UNNAMED11520;
    DW181.InverseRChGammaCorrectedValue45            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_45_UNNAMED11520;

    DW182.Value                                      = 0;        
    DW182.ForwardGChGammaCorrectedValue45            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_45_UNNAMED11520;
    DW182.ForwardBChGammaCorrectedValue45            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_45_UNNAMED11520;

    DW183.Value                                      = 0;        
    DW183.ForwardPixelValue45                        = FORWARD_PIXEL_VALUE_45_UNNAMED11520;
    DW183.ForwardRChGammaCorrectedValue45            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_45_UNNAMED11520;

    DW184.Value                                      = 0;        
    DW184.InverseGChGammaCorrectedValue46            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_46_UNNAMED11776;
    DW184.InverseBChGammaCorrectedValue46            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_46_UNNAMED11776;

    DW185.Value                                      = 0;        
    DW185.InversePixelValue46                        = INVERSE_PIXEL_VALUE_46_UNNAMED11776;
    DW185.InverseRChGammaCorrectedValue46            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_46_UNNAMED11776;

    DW186.Value                                      = 0;        
    DW186.ForwardGChGammaCorrectedValue46            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_46_UNNAMED11776;
    DW186.ForwardBChGammaCorrectedValue46            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_46_UNNAMED11776;

    DW187.Value                                      = 0;        
    DW187.ForwardPixelValue46                        = FORWARD_PIXEL_VALUE_46_UNNAMED11776;
    DW187.ForwardRChGammaCorrectedValue46            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_46_UNNAMED11776;

    DW188.Value                                      = 0;        
    DW188.InverseGChGammaCorrectedValue47            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_47_UNNAMED12032;
    DW188.InverseBChGammaCorrectedValue47            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_47_UNNAMED12032;

    DW189.Value                                      = 0;        
    DW189.InversePixelValue47                        = INVERSE_PIXEL_VALUE_47_UNNAMED12032;
    DW189.InverseRChGammaCorrectedValue47            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_47_UNNAMED12032;

    DW190.Value                                      = 0;        
    DW190.ForwardGChGammaCorrectedValue47            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_47_UNNAMED12032;
    DW190.ForwardBChGammaCorrectedValue47            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_47_UNNAMED12032;

    DW191.Value                                      = 0;        
    DW191.ForwardPixelValue47                        = FORWARD_PIXEL_VALUE_47_UNNAMED12032;
    DW191.ForwardRChGammaCorrectedValue47            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_47_UNNAMED12032;

    DW192.Value                                      = 0;        
    DW192.InverseGChGammaCorrectedValue48            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_48_UNNAMED12288;
    DW192.InverseBChGammaCorrectedValue48            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_48_UNNAMED12288;

    DW193.Value                                      = 0;        
    DW193.InversePixelValue48                        = INVERSE_PIXEL_VALUE_48_UNNAMED12288;
    DW193.InverseRChGammaCorrectedValue48            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_48_UNNAMED12288;

    DW194.Value                                      = 0;        
    DW194.ForwardGChGammaCorrectedValue48            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_48_UNNAMED12288;
    DW194.ForwardBChGammaCorrectedValue48            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_48_UNNAMED12288;

    DW195.Value                                      = 0;        
    DW195.ForwardPixelValue48                        = FORWARD_PIXEL_VALUE_48_UNNAMED12288;
    DW195.ForwardRChGammaCorrectedValue48            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_48_UNNAMED12288;

    DW196.Value                                      = 0;        
    DW196.InverseGChGammaCorrectedValue49            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_49_UNNAMED12544;
    DW196.InverseBChGammaCorrectedValue49            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_49_UNNAMED12544;

    DW197.Value                                      = 0;        
    DW197.InversePixelValue49                        = INVERSE_PIXEL_VALUE_49_UNNAMED12544;
    DW197.InverseRChGammaCorrectedValue49            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_49_UNNAMED12544;

    DW198.Value                                      = 0;        
    DW198.ForwardGChGammaCorrectedValue49            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_49_UNNAMED12544;
    DW198.ForwardBChGammaCorrectedValue49            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_49_UNNAMED12544;

    DW199.Value                                      = 0;        
    DW199.ForwardPixelValue49                        = FORWARD_PIXEL_VALUE_49_UNNAMED12544;
    DW199.ForwardRChGammaCorrectedValue49            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_49_UNNAMED12544;

    DW200.Value                                      = 0;        
    DW200.InverseGChGammaCorrectedValue50            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_50_UNNAMED12800;
    DW200.InverseBChGammaCorrectedValue50            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_50_UNNAMED12800;

    DW201.Value                                      = 0;        
    DW201.InversePixelValue50                        = INVERSE_PIXEL_VALUE_50_UNNAMED12800;
    DW201.InverseRChGammaCorrectedValue50            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_50_UNNAMED12800;

    DW202.Value                                      = 0;        
    DW202.ForwardGChGammaCorrectedValue50            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_50_UNNAMED12800;
    DW202.ForwardBChGammaCorrectedValue50            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_50_UNNAMED12800;

    DW203.Value                                      = 0;        
    DW203.ForwardPixelValue50                        = FORWARD_PIXEL_VALUE_50_UNNAMED12800;
    DW203.ForwardRChGammaCorrectedValue50            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_50_UNNAMED12800;

    DW204.Value                                      = 0;        
    DW204.InverseGChGammaCorrectedValue51            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_51_UNNAMED13056;
    DW204.InverseBChGammaCorrectedValue51            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_51_UNNAMED13056;

    DW205.Value                                      = 0;        
    DW205.InversePixelValue51                        = INVERSE_PIXEL_VALUE_51_UNNAMED13056;
    DW205.InverseRChGammaCorrectedValue51            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_51_UNNAMED13056;

    DW206.Value                                      = 0;        
    DW206.ForwardGChGammaCorrectedValue51            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_51_UNNAMED13056;
    DW206.ForwardBChGammaCorrectedValue51            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_51_UNNAMED13056;

    DW207.Value                                      = 0;        
    DW207.ForwardPixelValue51                        = FORWARD_PIXEL_VALUE_51_UNNAMED13056;
    DW207.ForwardRChGammaCorrectedValue51            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_51_UNNAMED13056;

    DW208.Value                                      = 0;        
    DW208.InverseGChGammaCorrectedValue52            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_52_UNNAMED13312;
    DW208.InverseBChGammaCorrectedValue52            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_52_UNNAMED13312;

    DW209.Value                                      = 0;        
    DW209.InversePixelValue52                        = INVERSE_PIXEL_VALUE_52_UNNAMED13312;
    DW209.InverseRChGammaCorrectedValue52            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_52_UNNAMED13312;

    DW210.Value                                      = 0;        
    DW210.ForwardGChGammaCorrectedValue52            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_52_UNNAMED13312;
    DW210.ForwardBChGammaCorrectedValue52            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_52_UNNAMED13312;

    DW211.Value                                      = 0;        
    DW211.ForwardPixelValue52                        = FORWARD_PIXEL_VALUE_52_UNNAMED13312;
    DW211.ForwardRChGammaCorrectedValue52            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_52_UNNAMED13312;

    DW212.Value                                      = 0;        
    DW212.InverseGChGammaCorrectedValue53            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_53_UNNAMED13568;
    DW212.InverseBChGammaCorrectedValue53            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_53_UNNAMED13568;

    DW213.Value                                      = 0;        
    DW213.InversePixelValue53                        = INVERSE_PIXEL_VALUE_53_UNNAMED13568;
    DW213.InverseRChGammaCorrectedValue53            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_53_UNNAMED13568;

    DW214.Value                                      = 0;        
    DW214.ForwardGChGammaCorrectedValue53            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_53_UNNAMED13568;
    DW214.ForwardBChGammaCorrectedValue53            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_53_UNNAMED13568;

    DW215.Value                                      = 0;        
    DW215.ForwardPixelValue53                        = FORWARD_PIXEL_VALUE_53_UNNAMED13568;
    DW215.ForwardRChGammaCorrectedValue53            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_53_UNNAMED13568;

    DW216.Value                                      = 0;        
    DW216.InverseGChGammaCorrectedValue54            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_54_UNNAMED13824;
    DW216.InverseBChGammaCorrectedValue54            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_54_UNNAMED13824;

    DW217.Value                                      = 0;        
    DW217.InversePixelValue54                        = INVERSE_PIXEL_VALUE_54_UNNAMED13824;
    DW217.InverseRChGammaCorrectedValue54            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_54_UNNAMED13824;

    DW218.Value                                      = 0;        
    DW218.ForwardGChGammaCorrectedValue54            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_54_UNNAMED13824;
    DW218.ForwardBChGammaCorrectedValue54            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_54_UNNAMED13824;

    DW219.Value                                      = 0;        
    DW219.ForwardPixelValue54                        = FORWARD_PIXEL_VALUE_54_UNNAMED13824;
    DW219.ForwardRChGammaCorrectedValue54            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_54_UNNAMED13824;

    DW220.Value                                      = 0;        
    DW220.InverseGChGammaCorrectedValue55            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_55_UNNAMED14080;
    DW220.InverseBChGammaCorrectedValue55            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_55_UNNAMED14080;

    DW221.Value                                      = 0;        
    DW221.InversePixelValue55                        = INVERSE_PIXEL_VALUE_55_UNNAMED14080;
    DW221.InverseRChGammaCorrectedValue55            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_55_UNNAMED14080;

    DW222.Value                                      = 0;        
    DW222.ForwardGChGammaCorrectedValue55            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_55_UNNAMED14080;
    DW222.ForwardBChGammaCorrectedValue55            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_55_UNNAMED14080;

    DW223.Value                                      = 0;        
    DW223.ForwardPixelValue55                        = FORWARD_PIXEL_VALUE_55_UNNAMED14080;
    DW223.ForwardRChGammaCorrectedValue55            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_55_UNNAMED14080;

    DW224.Value                                      = 0;        
    DW224.InverseGChGammaCorrectedValue56            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_56_UNNAMED14336;
    DW224.InverseBChGammaCorrectedValue56            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_56_UNNAMED14336;

    DW225.Value                                      = 0;        
    DW225.InversePixelValue56                        = INVERSE_PIXEL_VALUE_56_UNNAMED14336;
    DW225.InverseRChGammaCorrectedValue56            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_56_UNNAMED14336;

    DW226.Value                                      = 0;        
    DW226.ForwardGChGammaCorrectedValue56            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_56_UNNAMED14336;
    DW226.ForwardBChGammaCorrectedValue56            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_56_UNNAMED14336;

    DW227.Value                                      = 0;        
    DW227.ForwardPixelValue56                        = FORWARD_PIXEL_VALUE_56_UNNAMED14336;
    DW227.ForwardRChGammaCorrectedValue56            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_56_UNNAMED14336;

    DW228.Value                                      = 0;        
    DW228.InverseGChGammaCorrectedValue57            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_57_UNNAMED14592;
    DW228.InverseBChGammaCorrectedValue57            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_57_UNNAMED14592;

    DW229.Value                                      = 0;        
    DW229.InversePixelValue57                        = INVERSE_PIXEL_VALUE_57_UNNAMED14592;
    DW229.InverseRChGammaCorrectedValue57            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_57_UNNAMED14592;

    DW230.Value                                      = 0;        
    DW230.ForwardGChGammaCorrectedValue57            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_57_UNNAMED14592;
    DW230.ForwardBChGammaCorrectedValue57            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_57_UNNAMED14592;

    DW231.Value                                      = 0;        
    DW231.ForwardPixelValue57                        = FORWARD_PIXEL_VALUE_57_UNNAMED14592;
    DW231.ForwardRChGammaCorrectedValue57            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_57_UNNAMED14592;

    DW232.Value                                      = 0;        
    DW232.InverseGChGammaCorrectedValue58            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_58_UNNAMED14848;
    DW232.InverseBChGammaCorrectedValue58            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_58_UNNAMED14848;

    DW233.Value                                      = 0;        
    DW233.InversePixelValue58                        = INVERSE_PIXEL_VALUE_58_UNNAMED14848;
    DW233.InverseRChGammaCorrectedValue58            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_58_UNNAMED14848;

    DW234.Value                                      = 0;        
    DW234.ForwardGChGammaCorrectedValue58            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_58_UNNAMED14848;
    DW234.ForwardBChGammaCorrectedValue58            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_58_UNNAMED14848;

    DW235.Value                                      = 0;        
    DW235.ForwardPixelValue58                        = FORWARD_PIXEL_VALUE_58_UNNAMED14848;
    DW235.ForwardRChGammaCorrectedValue58            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_58_UNNAMED14848;

    DW236.Value                                      = 0;        
    DW236.InverseGChGammaCorrectedValue59            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_59_UNNAMED15104;
    DW236.InverseBChGammaCorrectedValue59            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_59_UNNAMED15104;

    DW237.Value                                      = 0;        
    DW237.InversePixelValue59                        = INVERSE_PIXEL_VALUE_59_UNNAMED15104;
    DW237.InverseRChGammaCorrectedValue59            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_59_UNNAMED15104;

    DW238.Value                                      = 0;        
    DW238.ForwardGChGammaCorrectedValue59            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_59_UNNAMED15104;
    DW238.ForwardBChGammaCorrectedValue59            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_59_UNNAMED15104;

    DW239.Value                                      = 0;        
    DW239.ForwardPixelValue59                        = FORWARD_PIXEL_VALUE_59_UNNAMED15104;
    DW239.ForwardRChGammaCorrectedValue59            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_59_UNNAMED15104;

    DW240.Value                                      = 0;        
    DW240.InverseGChGammaCorrectedValue60            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_60_UNNAMED15360;
    DW240.InverseBChGammaCorrectedValue60            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_60_UNNAMED15360;

    DW241.Value                                      = 0;        
    DW241.InversePixelValue60                        = INVERSE_PIXEL_VALUE_60_UNNAMED15360;
    DW241.InverseRChGammaCorrectedValue60            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_60_UNNAMED15360;

    DW242.Value                                      = 0;        
    DW242.ForwardGChGammaCorrectedValue60            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_60_UNNAMED15360;
    DW242.ForwardBChGammaCorrectedValue60            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_60_UNNAMED15360;

    DW243.Value                                      = 0;        
    DW243.ForwardPixelValue60                        = FORWARD_PIXEL_VALUE_60_UNNAMED15360;
    DW243.ForwardRChGammaCorrectedValue60            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_60_UNNAMED15360;

    DW244.Value                                      = 0;        
    DW244.InverseGChGammaCorrectedValue61            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_61_UNNAMED15616;
    DW244.InverseBChGammaCorrectedValue61            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_61_UNNAMED15616;

    DW245.Value                                      = 0;        
    DW245.InversePixelValue61                        = INVERSE_PIXEL_VALUE_61_UNNAMED15616;
    DW245.InverseRChGammaCorrectedValue61            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_61_UNNAMED15616;

    DW246.Value                                      = 0;        
    DW246.ForwardGChGammaCorrectedValue61            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_61_UNNAMED15616;
    DW246.ForwardBChGammaCorrectedValue61            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_61_UNNAMED15616;

    DW247.Value                                      = 0;        
    DW247.ForwardPixelValue61                        = FORWARD_PIXEL_VALUE_61_UNNAMED15616;
    DW247.ForwardRChGammaCorrectedValue61            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_61_UNNAMED15616;

    DW248.Value                                      = 0;        
    DW248.InverseGChGammaCorrectedValue62            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_62_UNNAMED15872;
    DW248.InverseBChGammaCorrectedValue62            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_62_UNNAMED15872;

    DW249.Value                                      = 0;        
    DW249.InversePixelValue62                        = INVERSE_PIXEL_VALUE_62_UNNAMED15872;
    DW249.InverseRChGammaCorrectedValue62            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_62_UNNAMED15872;

    DW250.Value                                      = 0;        
    DW250.ForwardGChGammaCorrectedValue62            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_62_UNNAMED15872;
    DW250.ForwardBChGammaCorrectedValue62            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_62_UNNAMED15872;

    DW251.Value                                      = 0;        
    DW251.ForwardPixelValue62                        = FORWARD_PIXEL_VALUE_62_UNNAMED15872;
    DW251.ForwardRChGammaCorrectedValue62            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_62_UNNAMED15872;

    DW252.Value                                      = 0;        
    DW252.InverseGChGammaCorrectedValue63            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_63_UNNAMED16128;
    DW252.InverseBChGammaCorrectedValue63            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_63_UNNAMED16128;

    DW253.Value                                      = 0;        
    DW253.InversePixelValue63                        = INVERSE_PIXEL_VALUE_63_UNNAMED16128;
    DW253.InverseRChGammaCorrectedValue63            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_63_UNNAMED16128;

    DW254.Value                                      = 0;        
    DW254.ForwardGChGammaCorrectedValue63            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_63_UNNAMED16128;
    DW254.ForwardBChGammaCorrectedValue63            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_63_UNNAMED16128;

    DW255.Value                                      = 0;        
    DW255.ForwardPixelValue63                        = FORWARD_PIXEL_VALUE_63_UNNAMED16128;
    DW255.ForwardRChGammaCorrectedValue63            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_63_UNNAMED16128;

    DW256.Value                                      = 0;        
    DW256.InverseGChGammaCorrectedValue64            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_64_UNNAMED16384;
    DW256.InverseBChGammaCorrectedValue64            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_64_UNNAMED16384;

    DW257.Value                                      = 0;        
    DW257.InversePixelValue64                        = INVERSE_PIXEL_VALUE_64_UNNAMED16384;
    DW257.InverseRChGammaCorrectedValue64            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_64_UNNAMED16384;

    DW258.Value                                      = 0;        
    DW258.ForwardGChGammaCorrectedValue64            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_64_UNNAMED16384;
    DW258.ForwardBChGammaCorrectedValue64            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_64_UNNAMED16384;

    DW259.Value                                      = 0;        
    DW259.ForwardPixelValue64                        = FORWARD_PIXEL_VALUE_64_UNNAMED16384;
    DW259.ForwardRChGammaCorrectedValue64            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_64_UNNAMED16384;

    DW260.Value                                      = 0;        
    DW260.InverseGChGammaCorrectedValue65            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_65_UNNAMED16640;
    DW260.InverseBChGammaCorrectedValue65            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_65_UNNAMED16640;

    DW261.Value                                      = 0;        
    DW261.InversePixelValue65                        = INVERSE_PIXEL_VALUE_65_UNNAMED16640;
    DW261.InverseRChGammaCorrectedValue65            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_65_UNNAMED16640;

    DW262.Value                                      = 0;        
    DW262.ForwardGChGammaCorrectedValue65            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_65_UNNAMED16640;
    DW262.ForwardBChGammaCorrectedValue65            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_65_UNNAMED16640;

    DW263.Value                                      = 0;        
    DW263.ForwardPixelValue65                        = FORWARD_PIXEL_VALUE_65_UNNAMED16640;
    DW263.ForwardRChGammaCorrectedValue65            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_65_UNNAMED16640;

    DW264.Value                                      = 0;        
    DW264.InverseGChGammaCorrectedValue66            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_66_UNNAMED16896;
    DW264.InverseBChGammaCorrectedValue66            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_66_UNNAMED16896;

    DW265.Value                                      = 0;        
    DW265.InversePixelValue66                        = INVERSE_PIXEL_VALUE_66_UNNAMED16896;
    DW265.InverseRChGammaCorrectedValue66            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_66_UNNAMED16896;

    DW266.Value                                      = 0;        
    DW266.ForwardGChGammaCorrectedValue66            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_66_UNNAMED16896;
    DW266.ForwardBChGammaCorrectedValue66            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_66_UNNAMED16896;

    DW267.Value                                      = 0;        
    DW267.ForwardPixelValue66                        = FORWARD_PIXEL_VALUE_66_UNNAMED16896;
    DW267.ForwardRChGammaCorrectedValue66            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_66_UNNAMED16896;

    DW268.Value                                      = 0;        
    DW268.InverseGChGammaCorrectedValue67            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_67_UNNAMED17152;
    DW268.InverseBChGammaCorrectedValue67            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_67_UNNAMED17152;

    DW269.Value                                      = 0;        
    DW269.InversePixelValue67                        = INVERSE_PIXEL_VALUE_67_UNNAMED17152;
    DW269.InverseRChGammaCorrectedValue67            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_67_UNNAMED17152;

    DW270.Value                                      = 0;        
    DW270.ForwardGChGammaCorrectedValue67            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_67_UNNAMED17152;
    DW270.ForwardBChGammaCorrectedValue67            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_67_UNNAMED17152;

    DW271.Value                                      = 0;        
    DW271.ForwardPixelValue67                        = FORWARD_PIXEL_VALUE_67_UNNAMED17152;
    DW271.ForwardRChGammaCorrectedValue67            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_67_UNNAMED17152;

    DW272.Value                                      = 0;        
    DW272.InverseGChGammaCorrectedValue68            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_68_UNNAMED17408;
    DW272.InverseBChGammaCorrectedValue68            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_68_UNNAMED17408;

    DW273.Value                                      = 0;        
    DW273.InversePixelValue68                        = INVERSE_PIXEL_VALUE_68_UNNAMED17408;
    DW273.InverseRChGammaCorrectedValue68            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_68_UNNAMED17408;

    DW274.Value                                      = 0;        
    DW274.ForwardGChGammaCorrectedValue68            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_68_UNNAMED17408;
    DW274.ForwardBChGammaCorrectedValue68            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_68_UNNAMED17408;

    DW275.Value                                      = 0;        
    DW275.ForwardPixelValue68                        = FORWARD_PIXEL_VALUE_68_UNNAMED17408;
    DW275.ForwardRChGammaCorrectedValue68            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_68_UNNAMED17408;

    DW276.Value                                      = 0;        
    DW276.InverseGChGammaCorrectedValue69            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_69_UNNAMED17664;
    DW276.InverseBChGammaCorrectedValue69            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_69_UNNAMED17664;

    DW277.Value                                      = 0;        
    DW277.InversePixelValue69                        = INVERSE_PIXEL_VALUE_69_UNNAMED17664;
    DW277.InverseRChGammaCorrectedValue69            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_69_UNNAMED17664;

    DW278.Value                                      = 0;        
    DW278.ForwardGChGammaCorrectedValue69            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_69_UNNAMED17664;
    DW278.ForwardBChGammaCorrectedValue69            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_69_UNNAMED17664;

    DW279.Value                                      = 0;        
    DW279.ForwardPixelValue69                        = FORWARD_PIXEL_VALUE_69_UNNAMED17664;
    DW279.ForwardRChGammaCorrectedValue69            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_69_UNNAMED17664;

    DW280.Value                                      = 0;        
    DW280.InverseGChGammaCorrectedValue70            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_70_UNNAMED17920;
    DW280.InverseBChGammaCorrectedValue70            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_70_UNNAMED17920;

    DW281.Value                                      = 0;        
    DW281.InversePixelValue70                        = INVERSE_PIXEL_VALUE_70_UNNAMED17920;
    DW281.InverseRChGammaCorrectedValue70            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_70_UNNAMED17920;

    DW282.Value                                      = 0;        
    DW282.ForwardGChGammaCorrectedValue70            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_70_UNNAMED17920;
    DW282.ForwardBChGammaCorrectedValue70            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_70_UNNAMED17920;

    DW283.Value                                      = 0;        
    DW283.ForwardPixelValue70                        = FORWARD_PIXEL_VALUE_70_UNNAMED17920;
    DW283.ForwardRChGammaCorrectedValue70            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_70_UNNAMED17920;

    DW284.Value                                      = 0;        
    DW284.InverseGChGammaCorrectedValue71            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_71_UNNAMED18176;
    DW284.InverseBChGammaCorrectedValue71            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_71_UNNAMED18176;

    DW285.Value                                      = 0;        
    DW285.InversePixelValue71                        = INVERSE_PIXEL_VALUE_71_UNNAMED18176;
    DW285.InverseRChGammaCorrectedValue71            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_71_UNNAMED18176;

    DW286.Value                                      = 0;        
    DW286.ForwardGChGammaCorrectedValue71            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_71_UNNAMED18176;
    DW286.ForwardBChGammaCorrectedValue71            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_71_UNNAMED18176;

    DW287.Value                                      = 0;        
    DW287.ForwardPixelValue71                        = FORWARD_PIXEL_VALUE_71_UNNAMED18176;
    DW287.ForwardRChGammaCorrectedValue71            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_71_UNNAMED18176;

    DW288.Value                                      = 0;        
    DW288.InverseGChGammaCorrectedValue72            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_72_UNNAMED18432;
    DW288.InverseBChGammaCorrectedValue72            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_72_UNNAMED18432;

    DW289.Value                                      = 0;        
    DW289.InversePixelValue72                        = INVERSE_PIXEL_VALUE_72_UNNAMED18432;
    DW289.InverseRChGammaCorrectedValue72            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_72_UNNAMED18432;

    DW290.Value                                      = 0;        
    DW290.ForwardGChGammaCorrectedValue72            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_72_UNNAMED18432;
    DW290.ForwardBChGammaCorrectedValue72            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_72_UNNAMED18432;

    DW291.Value                                      = 0;        
    DW291.ForwardPixelValue72                        = FORWARD_PIXEL_VALUE_72_UNNAMED18432;
    DW291.ForwardRChGammaCorrectedValue72            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_72_UNNAMED18432;

    DW292.Value                                      = 0;        
    DW292.InverseGChGammaCorrectedValue73            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_73_UNNAMED18688;
    DW292.InverseBChGammaCorrectedValue73            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_73_UNNAMED18688;

    DW293.Value                                      = 0;        
    DW293.InversePixelValue73                        = INVERSE_PIXEL_VALUE_73_UNNAMED18688;
    DW293.InverseRChGammaCorrectedValue73            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_73_UNNAMED18688;

    DW294.Value                                      = 0;        
    DW294.ForwardGChGammaCorrectedValue73            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_73_UNNAMED18688;
    DW294.ForwardBChGammaCorrectedValue73            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_73_UNNAMED18688;

    DW295.Value                                      = 0;        
    DW295.ForwardPixelValue73                        = FORWARD_PIXEL_VALUE_73_UNNAMED18688;
    DW295.ForwardRChGammaCorrectedValue73            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_73_UNNAMED18688;

    DW296.Value                                      = 0;        
    DW296.InverseGChGammaCorrectedValue74            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_74_UNNAMED18944;
    DW296.InverseBChGammaCorrectedValue74            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_74_UNNAMED18944;

    DW297.Value                                      = 0;        
    DW297.InversePixelValue74                        = INVERSE_PIXEL_VALUE_74_UNNAMED18944;
    DW297.InverseRChGammaCorrectedValue74            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_74_UNNAMED18944;

    DW298.Value                                      = 0;        
    DW298.ForwardGChGammaCorrectedValue74            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_74_UNNAMED18944;
    DW298.ForwardBChGammaCorrectedValue74            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_74_UNNAMED18944;

    DW299.Value                                      = 0;        
    DW299.ForwardPixelValue74                        = FORWARD_PIXEL_VALUE_74_UNNAMED18944;
    DW299.ForwardRChGammaCorrectedValue74            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_74_UNNAMED18944;

    DW300.Value                                      = 0;        
    DW300.InverseGChGammaCorrectedValue75            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_75_UNNAMED19200;
    DW300.InverseBChGammaCorrectedValue75            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_75_UNNAMED19200;

    DW301.Value                                      = 0;        
    DW301.InversePixelValue75                        = INVERSE_PIXEL_VALUE_75_UNNAMED19200;
    DW301.InverseRChGammaCorrectedValue75            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_75_UNNAMED19200;

    DW302.Value                                      = 0;        
    DW302.ForwardGChGammaCorrectedValue75            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_75_UNNAMED19200;
    DW302.ForwardBChGammaCorrectedValue75            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_75_UNNAMED19200;

    DW303.Value                                      = 0;        
    DW303.ForwardPixelValue75                        = FORWARD_PIXEL_VALUE_75_UNNAMED19200;
    DW303.ForwardRChGammaCorrectedValue75            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_75_UNNAMED19200;

    DW304.Value                                      = 0;        
    DW304.InverseGChGammaCorrectedValue76            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_76_UNNAMED19456;
    DW304.InverseBChGammaCorrectedValue76            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_76_UNNAMED19456;

    DW305.Value                                      = 0;        
    DW305.InversePixelValue76                        = INVERSE_PIXEL_VALUE_76_UNNAMED19456;
    DW305.InverseRChGammaCorrectedValue76            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_76_UNNAMED19456;

    DW306.Value                                      = 0;        
    DW306.ForwardGChGammaCorrectedValue76            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_76_UNNAMED19456;
    DW306.ForwardBChGammaCorrectedValue76            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_76_UNNAMED19456;

    DW307.Value                                      = 0;        
    DW307.ForwardPixelValue76                        = FORWARD_PIXEL_VALUE_76_UNNAMED19456;
    DW307.ForwardRChGammaCorrectedValue76            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_76_UNNAMED19456;

    DW308.Value                                      = 0;        
    DW308.InverseGChGammaCorrectedValue77            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_77_UNNAMED19712;
    DW308.InverseBChGammaCorrectedValue77            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_77_UNNAMED19712;

    DW309.Value                                      = 0;        
    DW309.InversePixelValue77                        = INVERSE_PIXEL_VALUE_77_UNNAMED19712;
    DW309.InverseRChGammaCorrectedValue77            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_77_UNNAMED19712;

    DW310.Value                                      = 0;        
    DW310.ForwardGChGammaCorrectedValue77            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_77_UNNAMED19712;
    DW310.ForwardBChGammaCorrectedValue77            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_77_UNNAMED19712;

    DW311.Value                                      = 0;        
    DW311.ForwardPixelValue77                        = FORWARD_PIXEL_VALUE_77_UNNAMED19712;
    DW311.ForwardRChGammaCorrectedValue77            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_77_UNNAMED19712;

    DW312.Value                                      = 0;        
    DW312.InverseGChGammaCorrectedValue78            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_78_UNNAMED19968;
    DW312.InverseBChGammaCorrectedValue78            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_78_UNNAMED19968;

    DW313.Value                                      = 0;        
    DW313.InversePixelValue78                        = INVERSE_PIXEL_VALUE_78_UNNAMED19968;
    DW313.InverseRChGammaCorrectedValue78            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_78_UNNAMED19968;

    DW314.Value                                      = 0;        
    DW314.ForwardGChGammaCorrectedValue78            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_78_UNNAMED19968;
    DW314.ForwardBChGammaCorrectedValue78            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_78_UNNAMED19968;

    DW315.Value                                      = 0;        
    DW315.ForwardPixelValue78                        = FORWARD_PIXEL_VALUE_78_UNNAMED19968;
    DW315.ForwardRChGammaCorrectedValue78            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_78_UNNAMED19968;

    DW316.Value                                      = 0;        
    DW316.InverseGChGammaCorrectedValue79            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_79_UNNAMED20224;
    DW316.InverseBChGammaCorrectedValue79            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_79_UNNAMED20224;

    DW317.Value                                      = 0;        
    DW317.InversePixelValue79                        = INVERSE_PIXEL_VALUE_79_UNNAMED20224;
    DW317.InverseRChGammaCorrectedValue79            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_79_UNNAMED20224;

    DW318.Value                                      = 0;        
    DW318.ForwardGChGammaCorrectedValue79            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_79_UNNAMED20224;
    DW318.ForwardBChGammaCorrectedValue79            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_79_UNNAMED20224;

    DW319.Value                                      = 0;        
    DW319.ForwardPixelValue79                        = FORWARD_PIXEL_VALUE_79_UNNAMED20224;
    DW319.ForwardRChGammaCorrectedValue79            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_79_UNNAMED20224;

    DW320.Value                                      = 0;        
    DW320.InverseGChGammaCorrectedValue80            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_80_UNNAMED20480;
    DW320.InverseBChGammaCorrectedValue80            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_80_UNNAMED20480;

    DW321.Value                                      = 0;        
    DW321.InversePixelValue80                        = INVERSE_PIXEL_VALUE_80_UNNAMED20480;
    DW321.InverseRChGammaCorrectedValue80            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_80_UNNAMED20480;

    DW322.Value                                      = 0;        
    DW322.ForwardGChGammaCorrectedValue80            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_80_UNNAMED20480;
    DW322.ForwardBChGammaCorrectedValue80            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_80_UNNAMED20480;

    DW323.Value                                      = 0;        
    DW323.ForwardPixelValue80                        = FORWARD_PIXEL_VALUE_80_UNNAMED20480;
    DW323.ForwardRChGammaCorrectedValue80            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_80_UNNAMED20480;

    DW324.Value                                      = 0;        
    DW324.InverseGChGammaCorrectedValue81            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_81_UNNAMED20736;
    DW324.InverseBChGammaCorrectedValue81            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_81_UNNAMED20736;

    DW325.Value                                      = 0;        
    DW325.InversePixelValue81                        = INVERSE_PIXEL_VALUE_81_UNNAMED20736;
    DW325.InverseRChGammaCorrectedValue81            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_81_UNNAMED20736;

    DW326.Value                                      = 0;        
    DW326.ForwardGChGammaCorrectedValue81            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_81_UNNAMED20736;
    DW326.ForwardBChGammaCorrectedValue81            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_81_UNNAMED20736;

    DW327.Value                                      = 0;        
    DW327.ForwardPixelValue81                        = FORWARD_PIXEL_VALUE_81_UNNAMED20736;
    DW327.ForwardRChGammaCorrectedValue81            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_81_UNNAMED20736;

    DW328.Value                                      = 0;        
    DW328.InverseGChGammaCorrectedValue82            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_82_UNNAMED20992;
    DW328.InverseBChGammaCorrectedValue82            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_82_UNNAMED20992;

    DW329.Value                                      = 0;        
    DW329.InversePixelValue82                        = INVERSE_PIXEL_VALUE_82_UNNAMED20992;
    DW329.InverseRChGammaCorrectedValue82            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_82_UNNAMED20992;

    DW330.Value                                      = 0;        
    DW330.ForwardGChGammaCorrectedValue82            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_82_UNNAMED20992;
    DW330.ForwardBChGammaCorrectedValue82            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_82_UNNAMED20992;

    DW331.Value                                      = 0;        
    DW331.ForwardPixelValue82                        = FORWARD_PIXEL_VALUE_82_UNNAMED20992;
    DW331.ForwardRChGammaCorrectedValue82            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_82_UNNAMED20992;

    DW332.Value                                      = 0;        
    DW332.InverseGChGammaCorrectedValue83            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_83_UNNAMED21248;
    DW332.InverseBChGammaCorrectedValue83            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_83_UNNAMED21248;

    DW333.Value                                      = 0;        
    DW333.InversePixelValue83                        = INVERSE_PIXEL_VALUE_83_UNNAMED21248;
    DW333.InverseRChGammaCorrectedValue83            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_83_UNNAMED21248;

    DW334.Value                                      = 0;        
    DW334.ForwardGChGammaCorrectedValue83            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_83_UNNAMED21248;
    DW334.ForwardBChGammaCorrectedValue83            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_83_UNNAMED21248;

    DW335.Value                                      = 0;        
    DW335.ForwardPixelValue83                        = FORWARD_PIXEL_VALUE_83_UNNAMED21248;
    DW335.ForwardRChGammaCorrectedValue83            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_83_UNNAMED21248;

    DW336.Value                                      = 0;        
    DW336.InverseGChGammaCorrectedValue84            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_84_UNNAMED21504;
    DW336.InverseBChGammaCorrectedValue84            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_84_UNNAMED21504;

    DW337.Value                                      = 0;        
    DW337.InversePixelValue84                        = INVERSE_PIXEL_VALUE_84_UNNAMED21504;
    DW337.InverseRChGammaCorrectedValue84            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_84_UNNAMED21504;

    DW338.Value                                      = 0;        
    DW338.ForwardGChGammaCorrectedValue84            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_84_UNNAMED21504;
    DW338.ForwardBChGammaCorrectedValue84            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_84_UNNAMED21504;

    DW339.Value                                      = 0;        
    DW339.ForwardPixelValue84                        = FORWARD_PIXEL_VALUE_84_UNNAMED21504;
    DW339.ForwardRChGammaCorrectedValue84            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_84_UNNAMED21504;

    DW340.Value                                      = 0;        
    DW340.InverseGChGammaCorrectedValue85            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_85_UNNAMED21760;
    DW340.InverseBChGammaCorrectedValue85            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_85_UNNAMED21760;

    DW341.Value                                      = 0;        
    DW341.InversePixelValue85                        = INVERSE_PIXEL_VALUE_85_UNNAMED21760;
    DW341.InverseRChGammaCorrectedValue85            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_85_UNNAMED21760;

    DW342.Value                                      = 0;        
    DW342.ForwardGChGammaCorrectedValue85            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_85_UNNAMED21760;
    DW342.ForwardBChGammaCorrectedValue85            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_85_UNNAMED21760;

    DW343.Value                                      = 0;        
    DW343.ForwardPixelValue85                        = FORWARD_PIXEL_VALUE_85_UNNAMED21760;
    DW343.ForwardRChGammaCorrectedValue85            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_85_UNNAMED21760;

    DW344.Value                                      = 0;        
    DW344.InverseGChGammaCorrectedValue86            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_86_UNNAMED22016;
    DW344.InverseBChGammaCorrectedValue86            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_86_UNNAMED22016;

    DW345.Value                                      = 0;        
    DW345.InversePixelValue86                        = INVERSE_PIXEL_VALUE_86_UNNAMED22016;
    DW345.InverseRChGammaCorrectedValue86            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_86_UNNAMED22016;

    DW346.Value                                      = 0;        
    DW346.ForwardGChGammaCorrectedValue86            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_86_UNNAMED22016;
    DW346.ForwardBChGammaCorrectedValue86            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_86_UNNAMED22016;

    DW347.Value                                      = 0;        
    DW347.ForwardPixelValue86                        = FORWARD_PIXEL_VALUE_86_UNNAMED22016;
    DW347.ForwardRChGammaCorrectedValue86            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_86_UNNAMED22016;

    DW348.Value                                      = 0;        
    DW348.InverseGChGammaCorrectedValue87            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_87_UNNAMED22272;
    DW348.InverseBChGammaCorrectedValue87            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_87_UNNAMED22272;

    DW349.Value                                      = 0;        
    DW349.InversePixelValue87                        = INVERSE_PIXEL_VALUE_87_UNNAMED22272;
    DW349.InverseRChGammaCorrectedValue87            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_87_UNNAMED22272;

    DW350.Value                                      = 0;        
    DW350.ForwardGChGammaCorrectedValue87            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_87_UNNAMED22272;
    DW350.ForwardBChGammaCorrectedValue87            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_87_UNNAMED22272;

    DW351.Value                                      = 0;        
    DW351.ForwardPixelValue87                        = FORWARD_PIXEL_VALUE_87_UNNAMED22272;
    DW351.ForwardRChGammaCorrectedValue87            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_87_UNNAMED22272;

    DW352.Value                                      = 0;        
    DW352.InverseGChGammaCorrectedValue88            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_88_UNNAMED22528;
    DW352.InverseBChGammaCorrectedValue88            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_88_UNNAMED22528;

    DW353.Value                                      = 0;        
    DW353.InversePixelValue88                        = INVERSE_PIXEL_VALUE_88_UNNAMED22528;
    DW353.InverseRChGammaCorrectedValue88            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_88_UNNAMED22528;

    DW354.Value                                      = 0;        
    DW354.ForwardGChGammaCorrectedValue88            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_88_UNNAMED22528;
    DW354.ForwardBChGammaCorrectedValue88            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_88_UNNAMED22528;

    DW355.Value                                      = 0;        
    DW355.ForwardPixelValue88                        = FORWARD_PIXEL_VALUE_88_UNNAMED22528;
    DW355.ForwardRChGammaCorrectedValue88            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_88_UNNAMED22528;

    DW356.Value                                      = 0;        
    DW356.InverseGChGammaCorrectedValue89            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_89_UNNAMED22784;
    DW356.InverseBChGammaCorrectedValue89            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_89_UNNAMED22784;

    DW357.Value                                      = 0;        
    DW357.InversePixelValue89                        = INVERSE_PIXEL_VALUE_89_UNNAMED22784;
    DW357.InverseRChGammaCorrectedValue89            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_89_UNNAMED22784;

    DW358.Value                                      = 0;        
    DW358.ForwardGChGammaCorrectedValue89            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_89_UNNAMED22784;
    DW358.ForwardBChGammaCorrectedValue89            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_89_UNNAMED22784;

    DW359.Value                                      = 0;        
    DW359.ForwardPixelValue89                        = FORWARD_PIXEL_VALUE_89_UNNAMED22784;
    DW359.ForwardRChGammaCorrectedValue89            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_89_UNNAMED22784;

    DW360.Value                                      = 0;        
    DW360.InverseGChGammaCorrectedValue90            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_90_UNNAMED23040;
    DW360.InverseBChGammaCorrectedValue90            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_90_UNNAMED23040;

    DW361.Value                                      = 0;        
    DW361.InversePixelValue90                        = INVERSE_PIXEL_VALUE_90_UNNAMED23040;
    DW361.InverseRChGammaCorrectedValue90            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_90_UNNAMED23040;

    DW362.Value                                      = 0;        
    DW362.ForwardGChGammaCorrectedValue90            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_90_UNNAMED23040;
    DW362.ForwardBChGammaCorrectedValue90            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_90_UNNAMED23040;

    DW363.Value                                      = 0;        
    DW363.ForwardPixelValue90                        = FORWARD_PIXEL_VALUE_90_UNNAMED23040;
    DW363.ForwardRChGammaCorrectedValue90            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_90_UNNAMED23040;

    DW364.Value                                      = 0;        
    DW364.InverseGChGammaCorrectedValue91            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_91_UNNAMED23296;
    DW364.InverseBChGammaCorrectedValue91            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_91_UNNAMED23296;

    DW365.Value                                      = 0;        
    DW365.InversePixelValue91                        = INVERSE_PIXEL_VALUE_91_UNNAMED23296;
    DW365.InverseRChGammaCorrectedValue91            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_91_UNNAMED23296;

    DW366.Value                                      = 0;        
    DW366.ForwardGChGammaCorrectedValue91            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_91_UNNAMED23296;
    DW366.ForwardBChGammaCorrectedValue91            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_91_UNNAMED23296;

    DW367.Value                                      = 0;        
    DW367.ForwardPixelValue91                        = FORWARD_PIXEL_VALUE_91_UNNAMED23296;
    DW367.ForwardRChGammaCorrectedValue91            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_91_UNNAMED23296;

    DW368.Value                                      = 0;        
    DW368.InverseGChGammaCorrectedValue92            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_92_UNNAMED23552;
    DW368.InverseBChGammaCorrectedValue92            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_92_UNNAMED23552;

    DW369.Value                                      = 0;        
    DW369.InversePixelValue92                        = INVERSE_PIXEL_VALUE_92_UNNAMED23552;
    DW369.InverseRChGammaCorrectedValue92            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_92_UNNAMED23552;

    DW370.Value                                      = 0;        
    DW370.ForwardGChGammaCorrectedValue92            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_92_UNNAMED23552;
    DW370.ForwardBChGammaCorrectedValue92            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_92_UNNAMED23552;

    DW371.Value                                      = 0;        
    DW371.ForwardPixelValue92                        = FORWARD_PIXEL_VALUE_92_UNNAMED23552;
    DW371.ForwardRChGammaCorrectedValue92            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_92_UNNAMED23552;

    DW372.Value                                      = 0;        
    DW372.InverseGChGammaCorrectedValue93            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_93_UNNAMED23808;
    DW372.InverseBChGammaCorrectedValue93            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_93_UNNAMED23808;

    DW373.Value                                      = 0;        
    DW373.InversePixelValue93                        = INVERSE_PIXEL_VALUE_93_UNNAMED23808;
    DW373.InverseRChGammaCorrectedValue93            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_93_UNNAMED23808;

    DW374.Value                                      = 0;        
    DW374.ForwardGChGammaCorrectedValue93            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_93_UNNAMED23808;
    DW374.ForwardBChGammaCorrectedValue93            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_93_UNNAMED23808;

    DW375.Value                                      = 0;        
    DW375.ForwardPixelValue93                        = FORWARD_PIXEL_VALUE_93_UNNAMED23808;
    DW375.ForwardRChGammaCorrectedValue93            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_93_UNNAMED23808;

    DW376.Value                                      = 0;        
    DW376.InverseGChGammaCorrectedValue94            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_94_UNNAMED24064;
    DW376.InverseBChGammaCorrectedValue94            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_94_UNNAMED24064;

    DW377.Value                                      = 0;        
    DW377.InversePixelValue94                        = INVERSE_PIXEL_VALUE_94_UNNAMED24064;
    DW377.InverseRChGammaCorrectedValue94            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_94_UNNAMED24064;

    DW378.Value                                      = 0;        
    DW378.ForwardGChGammaCorrectedValue94            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_94_UNNAMED24064;
    DW378.ForwardBChGammaCorrectedValue94            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_94_UNNAMED24064;

    DW379.Value                                      = 0;        
    DW379.ForwardPixelValue94                        = FORWARD_PIXEL_VALUE_94_UNNAMED24064;
    DW379.ForwardRChGammaCorrectedValue94            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_94_UNNAMED24064;

    DW380.Value                                      = 0;        
    DW380.InverseGChGammaCorrectedValue95            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_95_UNNAMED24320;
    DW380.InverseBChGammaCorrectedValue95            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_95_UNNAMED24320;

    DW381.Value                                      = 0;        
    DW381.InversePixelValue95                        = INVERSE_PIXEL_VALUE_95_UNNAMED24320;
    DW381.InverseRChGammaCorrectedValue95            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_95_UNNAMED24320;

    DW382.Value                                      = 0;        
    DW382.ForwardGChGammaCorrectedValue95            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_95_UNNAMED24320;
    DW382.ForwardBChGammaCorrectedValue95            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_95_UNNAMED24320;

    DW383.Value                                      = 0;        
    DW383.ForwardPixelValue95                        = FORWARD_PIXEL_VALUE_95_UNNAMED24320;
    DW383.ForwardRChGammaCorrectedValue95            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_95_UNNAMED24320;

    DW384.Value                                      = 0;        
    DW384.InverseGChGammaCorrectedValue96            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_96_UNNAMED24576;
    DW384.InverseBChGammaCorrectedValue96            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_96_UNNAMED24576;

    DW385.Value                                      = 0;        
    DW385.InversePixelValue96                        = INVERSE_PIXEL_VALUE_96_UNNAMED24576;
    DW385.InverseRChGammaCorrectedValue96            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_96_UNNAMED24576;

    DW386.Value                                      = 0;        
    DW386.ForwardGChGammaCorrectedValue96            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_96_UNNAMED24576;
    DW386.ForwardBChGammaCorrectedValue96            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_96_UNNAMED24576;

    DW387.Value                                      = 0;        
    DW387.ForwardPixelValue96                        = FORWARD_PIXEL_VALUE_96_UNNAMED24576;
    DW387.ForwardRChGammaCorrectedValue96            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_96_UNNAMED24576;

    DW388.Value                                      = 0;        
    DW388.InverseGChGammaCorrectedValue97            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_97_UNNAMED24832;
    DW388.InverseBChGammaCorrectedValue97            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_97_UNNAMED24832;

    DW389.Value                                      = 0;        
    DW389.InversePixelValue97                        = INVERSE_PIXEL_VALUE_97_UNNAMED24832;
    DW389.InverseRChGammaCorrectedValue97            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_97_UNNAMED24832;

    DW390.Value                                      = 0;        
    DW390.ForwardGChGammaCorrectedValue97            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_97_UNNAMED24832;
    DW390.ForwardBChGammaCorrectedValue97            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_97_UNNAMED24832;

    DW391.Value                                      = 0;        
    DW391.ForwardPixelValue97                        = FORWARD_PIXEL_VALUE_97_UNNAMED24832;
    DW391.ForwardRChGammaCorrectedValue97            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_97_UNNAMED24832;

    DW392.Value                                      = 0;        
    DW392.InverseGChGammaCorrectedValue98            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_98_UNNAMED25088;
    DW392.InverseBChGammaCorrectedValue98            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_98_UNNAMED25088;

    DW393.Value                                      = 0;        
    DW393.InversePixelValue98                        = INVERSE_PIXEL_VALUE_98_UNNAMED25088;
    DW393.InverseRChGammaCorrectedValue98            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_98_UNNAMED25088;

    DW394.Value                                      = 0;        
    DW394.ForwardGChGammaCorrectedValue98            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_98_UNNAMED25088;
    DW394.ForwardBChGammaCorrectedValue98            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_98_UNNAMED25088;

    DW395.Value                                      = 0;        
    DW395.ForwardPixelValue98                        = FORWARD_PIXEL_VALUE_98_UNNAMED25088;
    DW395.ForwardRChGammaCorrectedValue98            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_98_UNNAMED25088;

    DW396.Value                                      = 0;        
    DW396.InverseGChGammaCorrectedValue99            = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_99_UNNAMED25344;
    DW396.InverseBChGammaCorrectedValue99            = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_99_UNNAMED25344;

    DW397.Value                                      = 0;        
    DW397.InversePixelValue99                        = INVERSE_PIXEL_VALUE_99_UNNAMED25344;
    DW397.InverseRChGammaCorrectedValue99            = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_99_UNNAMED25344;

    DW398.Value                                      = 0;        
    DW398.ForwardGChGammaCorrectedValue99            = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_99_UNNAMED25344;
    DW398.ForwardBChGammaCorrectedValue99            = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_99_UNNAMED25344;

    DW399.Value                                      = 0;        
    DW399.ForwardPixelValue99                        = FORWARD_PIXEL_VALUE_99_UNNAMED25344;
    DW399.ForwardRChGammaCorrectedValue99            = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_99_UNNAMED25344;

    DW400.Value                                      = 0;        
    DW400.InverseGChGammaCorrectedValue100           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_100_UNNAMED25600;
    DW400.InverseBChGammaCorrectedValue100           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_100_UNNAMED25600;

    DW401.Value                                      = 0;        
    DW401.InversePixelValue100                       = INVERSE_PIXEL_VALUE_100_UNNAMED25600;
    DW401.InverseRChGammaCorrectedValue100           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_100_UNNAMED25600;

    DW402.Value                                      = 0;        
    DW402.ForwardGChGammaCorrectedValue100           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_100_UNNAMED25600;
    DW402.ForwardBChGammaCorrectedValue100           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_100_UNNAMED25600;

    DW403.Value                                      = 0;        
    DW403.ForwardPixelValue100                       = FORWARD_PIXEL_VALUE_100_UNNAMED25600;
    DW403.ForwardRChGammaCorrectedValue100           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_100_UNNAMED25600;

    DW404.Value                                      = 0;        
    DW404.InverseGChGammaCorrectedValue101           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_101_UNNAMED25856;
    DW404.InverseBChGammaCorrectedValue101           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_101_UNNAMED25856;

    DW405.Value                                      = 0;        
    DW405.InversePixelValue101                       = INVERSE_PIXEL_VALUE_101_UNNAMED25856;
    DW405.InverseRChGammaCorrectedValue101           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_101_UNNAMED25856;

    DW406.Value                                      = 0;        
    DW406.ForwardGChGammaCorrectedValue101           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_101_UNNAMED25856;
    DW406.ForwardBChGammaCorrectedValue101           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_101_UNNAMED25856;

    DW407.Value                                      = 0;        
    DW407.ForwardPixelValue101                       = FORWARD_PIXEL_VALUE_101_UNNAMED25856;
    DW407.ForwardRChGammaCorrectedValue101           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_101_UNNAMED25856;

    DW408.Value                                      = 0;        
    DW408.InverseGChGammaCorrectedValue102           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_102_UNNAMED26112;
    DW408.InverseBChGammaCorrectedValue102           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_102_UNNAMED26112;

    DW409.Value                                      = 0;        
    DW409.InversePixelValue102                       = INVERSE_PIXEL_VALUE_102_UNNAMED26112;
    DW409.InverseRChGammaCorrectedValue102           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_102_UNNAMED26112;

    DW410.Value                                      = 0;        
    DW410.ForwardGChGammaCorrectedValue102           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_102_UNNAMED26112;
    DW410.ForwardBChGammaCorrectedValue102           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_102_UNNAMED26112;

    DW411.Value                                      = 0;        
    DW411.ForwardPixelValue102                       = FORWARD_PIXEL_VALUE_102_UNNAMED26112;
    DW411.ForwardRChGammaCorrectedValue102           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_102_UNNAMED26112;

    DW412.Value                                      = 0;        
    DW412.InverseGChGammaCorrectedValue103           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_103_UNNAMED26368;
    DW412.InverseBChGammaCorrectedValue103           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_103_UNNAMED26368;

    DW413.Value                                      = 0;        
    DW413.InversePixelValue103                       = INVERSE_PIXEL_VALUE_103_UNNAMED26368;
    DW413.InverseRChGammaCorrectedValue103           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_103_UNNAMED26368;

    DW414.Value                                      = 0;        
    DW414.ForwardGChGammaCorrectedValue103           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_103_UNNAMED26368;
    DW414.ForwardBChGammaCorrectedValue103           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_103_UNNAMED26368;

    DW415.Value                                      = 0;        
    DW415.ForwardPixelValue103                       = FORWARD_PIXEL_VALUE_103_UNNAMED26368;
    DW415.ForwardRChGammaCorrectedValue103           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_103_UNNAMED26368;

    DW416.Value                                      = 0;        
    DW416.InverseGChGammaCorrectedValue104           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_104_UNNAMED26624;
    DW416.InverseBChGammaCorrectedValue104           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_104_UNNAMED26624;

    DW417.Value                                      = 0;        
    DW417.InversePixelValue104                       = INVERSE_PIXEL_VALUE_104_UNNAMED26624;
    DW417.InverseRChGammaCorrectedValue104           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_104_UNNAMED26624;

    DW418.Value                                      = 0;        
    DW418.ForwardGChGammaCorrectedValue104           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_104_UNNAMED26624;
    DW418.ForwardBChGammaCorrectedValue104           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_104_UNNAMED26624;

    DW419.Value                                      = 0;        
    DW419.ForwardPixelValue104                       = FORWARD_PIXEL_VALUE_104_UNNAMED26624;
    DW419.ForwardRChGammaCorrectedValue104           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_104_UNNAMED26624;

    DW420.Value                                      = 0;        
    DW420.InverseGChGammaCorrectedValue105           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_105_UNNAMED26880;
    DW420.InverseBChGammaCorrectedValue105           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_105_UNNAMED26880;

    DW421.Value                                      = 0;        
    DW421.InversePixelValue105                       = INVERSE_PIXEL_VALUE_105_UNNAMED26880;
    DW421.InverseRChGammaCorrectedValue105           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_105_UNNAMED26880;

    DW422.Value                                      = 0;        
    DW422.ForwardGChGammaCorrectedValue105           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_105_UNNAMED26880;
    DW422.ForwardBChGammaCorrectedValue105           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_105_UNNAMED26880;

    DW423.Value                                      = 0;        
    DW423.ForwardPixelValue105                       = FORWARD_PIXEL_VALUE_105_UNNAMED26880;
    DW423.ForwardRChGammaCorrectedValue105           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_105_UNNAMED26880;

    DW424.Value                                      = 0;        
    DW424.InverseGChGammaCorrectedValue106           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_106_UNNAMED27136;
    DW424.InverseBChGammaCorrectedValue106           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_106_UNNAMED27136;

    DW425.Value                                      = 0;        
    DW425.InversePixelValue106                       = INVERSE_PIXEL_VALUE_106_UNNAMED27136;
    DW425.InverseRChGammaCorrectedValue106           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_106_UNNAMED27136;

    DW426.Value                                      = 0;        
    DW426.ForwardGChGammaCorrectedValue106           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_106_UNNAMED27136;
    DW426.ForwardBChGammaCorrectedValue106           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_106_UNNAMED27136;

    DW427.Value                                      = 0;        
    DW427.ForwardPixelValue106                       = FORWARD_PIXEL_VALUE_106_UNNAMED27136;
    DW427.ForwardRChGammaCorrectedValue106           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_106_UNNAMED27136;

    DW428.Value                                      = 0;        
    DW428.InverseGChGammaCorrectedValue107           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_107_UNNAMED27392;
    DW428.InverseBChGammaCorrectedValue107           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_107_UNNAMED27392;

    DW429.Value                                      = 0;        
    DW429.InversePixelValue107                       = INVERSE_PIXEL_VALUE_107_UNNAMED27392;
    DW429.InverseRChGammaCorrectedValue107           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_107_UNNAMED27392;

    DW430.Value                                      = 0;        
    DW430.ForwardGChGammaCorrectedValue107           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_107_UNNAMED27392;
    DW430.ForwardBChGammaCorrectedValue107           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_107_UNNAMED27392;

    DW431.Value                                      = 0;        
    DW431.ForwardPixelValue107                       = FORWARD_PIXEL_VALUE_107_UNNAMED27392;
    DW431.ForwardRChGammaCorrectedValue107           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_107_UNNAMED27392;

    DW432.Value                                      = 0;        
    DW432.InverseGChGammaCorrectedValue108           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_108_UNNAMED27648;
    DW432.InverseBChGammaCorrectedValue108           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_108_UNNAMED27648;

    DW433.Value                                      = 0;        
    DW433.InversePixelValue108                       = INVERSE_PIXEL_VALUE_108_UNNAMED27648;
    DW433.InverseRChGammaCorrectedValue108           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_108_UNNAMED27648;

    DW434.Value                                      = 0;        
    DW434.ForwardGChGammaCorrectedValue108           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_108_UNNAMED27648;
    DW434.ForwardBChGammaCorrectedValue108           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_108_UNNAMED27648;

    DW435.Value                                      = 0;        
    DW435.ForwardPixelValue108                       = FORWARD_PIXEL_VALUE_108_UNNAMED27648;
    DW435.ForwardRChGammaCorrectedValue108           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_108_UNNAMED27648;

    DW436.Value                                      = 0;        
    DW436.InverseGChGammaCorrectedValue109           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_109_UNNAMED27904;
    DW436.InverseBChGammaCorrectedValue109           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_109_UNNAMED27904;

    DW437.Value                                      = 0;        
    DW437.InversePixelValue109                       = INVERSE_PIXEL_VALUE_109_UNNAMED27904;
    DW437.InverseRChGammaCorrectedValue109           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_109_UNNAMED27904;

    DW438.Value                                      = 0;        
    DW438.ForwardGChGammaCorrectedValue109           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_109_UNNAMED27904;
    DW438.ForwardBChGammaCorrectedValue109           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_109_UNNAMED27904;

    DW439.Value                                      = 0;        
    DW439.ForwardPixelValue109                       = FORWARD_PIXEL_VALUE_109_UNNAMED27904;
    DW439.ForwardRChGammaCorrectedValue109           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_109_UNNAMED27904;

    DW440.Value                                      = 0;        
    DW440.InverseGChGammaCorrectedValue110           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_110_UNNAMED28160;
    DW440.InverseBChGammaCorrectedValue110           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_110_UNNAMED28160;

    DW441.Value                                      = 0;        
    DW441.InversePixelValue110                       = INVERSE_PIXEL_VALUE_110_UNNAMED28160;
    DW441.InverseRChGammaCorrectedValue110           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_110_UNNAMED28160;

    DW442.Value                                      = 0;        
    DW442.ForwardGChGammaCorrectedValue110           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_110_UNNAMED28160;
    DW442.ForwardBChGammaCorrectedValue110           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_110_UNNAMED28160;

    DW443.Value                                      = 0;        
    DW443.ForwardPixelValue110                       = FORWARD_PIXEL_VALUE_110_UNNAMED28160;
    DW443.ForwardRChGammaCorrectedValue110           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_110_UNNAMED28160;

    DW444.Value                                      = 0;        
    DW444.InverseGChGammaCorrectedValue111           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_111_UNNAMED28416;
    DW444.InverseBChGammaCorrectedValue111           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_111_UNNAMED28416;

    DW445.Value                                      = 0;        
    DW445.InversePixelValue111                       = INVERSE_PIXEL_VALUE_111_UNNAMED28416;
    DW445.InverseRChGammaCorrectedValue111           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_111_UNNAMED28416;

    DW446.Value                                      = 0;        
    DW446.ForwardGChGammaCorrectedValue111           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_111_UNNAMED28416;
    DW446.ForwardBChGammaCorrectedValue111           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_111_UNNAMED28416;

    DW447.Value                                      = 0;        
    DW447.ForwardPixelValue111                       = FORWARD_PIXEL_VALUE_111_UNNAMED28416;
    DW447.ForwardRChGammaCorrectedValue111           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_111_UNNAMED28416;

    DW448.Value                                      = 0;        
    DW448.InverseGChGammaCorrectedValue112           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_112_UNNAMED28672;
    DW448.InverseBChGammaCorrectedValue112           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_112_UNNAMED28672;

    DW449.Value                                      = 0;        
    DW449.InversePixelValue112                       = INVERSE_PIXEL_VALUE_112_UNNAMED28672;
    DW449.InverseRChGammaCorrectedValue112           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_112_UNNAMED28672;

    DW450.Value                                      = 0;        
    DW450.ForwardGChGammaCorrectedValue112           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_112_UNNAMED28672;
    DW450.ForwardBChGammaCorrectedValue112           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_112_UNNAMED28672;

    DW451.Value                                      = 0;        
    DW451.ForwardPixelValue112                       = FORWARD_PIXEL_VALUE_112_UNNAMED28672;
    DW451.ForwardRChGammaCorrectedValue112           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_112_UNNAMED28672;

    DW452.Value                                      = 0;        
    DW452.InverseGChGammaCorrectedValue113           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_113_UNNAMED28928;
    DW452.InverseBChGammaCorrectedValue113           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_113_UNNAMED28928;

    DW453.Value                                      = 0;        
    DW453.InversePixelValue113                       = INVERSE_PIXEL_VALUE_113_UNNAMED28928;
    DW453.InverseRChGammaCorrectedValue113           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_113_UNNAMED28928;

    DW454.Value                                      = 0;        
    DW454.ForwardGChGammaCorrectedValue113           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_113_UNNAMED28928;
    DW454.ForwardBChGammaCorrectedValue113           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_113_UNNAMED28928;

    DW455.Value                                      = 0;        
    DW455.ForwardPixelValue113                       = FORWARD_PIXEL_VALUE_113_UNNAMED28928;
    DW455.ForwardRChGammaCorrectedValue113           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_113_UNNAMED28928;

    DW456.Value                                      = 0;        
    DW456.InverseGChGammaCorrectedValue114           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_114_UNNAMED29184;
    DW456.InverseBChGammaCorrectedValue114           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_114_UNNAMED29184;

    DW457.Value                                      = 0;        
    DW457.InversePixelValue114                       = INVERSE_PIXEL_VALUE_114_UNNAMED29184;
    DW457.InverseRChGammaCorrectedValue114           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_114_UNNAMED29184;

    DW458.Value                                      = 0;        
    DW458.ForwardGChGammaCorrectedValue114           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_114_UNNAMED29184;
    DW458.ForwardBChGammaCorrectedValue114           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_114_UNNAMED29184;

    DW459.Value                                      = 0;        
    DW459.ForwardPixelValue114                       = FORWARD_PIXEL_VALUE_114_UNNAMED29184;
    DW459.ForwardRChGammaCorrectedValue114           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_114_UNNAMED29184;

    DW460.Value                                      = 0;        
    DW460.InverseGChGammaCorrectedValue115           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_115_UNNAMED29440;
    DW460.InverseBChGammaCorrectedValue115           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_115_UNNAMED29440;

    DW461.Value                                      = 0;        
    DW461.InversePixelValue115                       = INVERSE_PIXEL_VALUE_115_UNNAMED29440;
    DW461.InverseRChGammaCorrectedValue115           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_115_UNNAMED29440;

    DW462.Value                                      = 0;        
    DW462.ForwardGChGammaCorrectedValue115           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_115_UNNAMED29440;
    DW462.ForwardBChGammaCorrectedValue115           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_115_UNNAMED29440;

    DW463.Value                                      = 0;        
    DW463.ForwardPixelValue115                       = FORWARD_PIXEL_VALUE_115_UNNAMED29440;
    DW463.ForwardRChGammaCorrectedValue115           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_115_UNNAMED29440;

    DW464.Value                                      = 0;        
    DW464.InverseGChGammaCorrectedValue116           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_116_UNNAMED29696;
    DW464.InverseBChGammaCorrectedValue116           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_116_UNNAMED29696;

    DW465.Value                                      = 0;        
    DW465.InversePixelValue116                       = INVERSE_PIXEL_VALUE_116_UNNAMED29696;
    DW465.InverseRChGammaCorrectedValue116           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_116_UNNAMED29696;

    DW466.Value                                      = 0;        
    DW466.ForwardGChGammaCorrectedValue116           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_116_UNNAMED29696;
    DW466.ForwardBChGammaCorrectedValue116           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_116_UNNAMED29696;

    DW467.Value                                      = 0;        
    DW467.ForwardPixelValue116                       = FORWARD_PIXEL_VALUE_116_UNNAMED29696;
    DW467.ForwardRChGammaCorrectedValue116           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_116_UNNAMED29696;

    DW468.Value                                      = 0;        
    DW468.InverseGChGammaCorrectedValue117           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_117_UNNAMED29952;
    DW468.InverseBChGammaCorrectedValue117           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_117_UNNAMED29952;

    DW469.Value                                      = 0;        
    DW469.InversePixelValue117                       = INVERSE_PIXEL_VALUE_117_UNNAMED29952;
    DW469.InverseRChGammaCorrectedValue117           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_117_UNNAMED29952;

    DW470.Value                                      = 0;        
    DW470.ForwardGChGammaCorrectedValue117           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_117_UNNAMED29952;
    DW470.ForwardBChGammaCorrectedValue117           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_117_UNNAMED29952;

    DW471.Value                                      = 0;        
    DW471.ForwardPixelValue117                       = FORWARD_PIXEL_VALUE_117_UNNAMED29952;
    DW471.ForwardRChGammaCorrectedValue117           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_117_UNNAMED29952;

    DW472.Value                                      = 0;        
    DW472.InverseGChGammaCorrectedValue118           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_118_UNNAMED30208;
    DW472.InverseBChGammaCorrectedValue118           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_118_UNNAMED30208;

    DW473.Value                                      = 0;        
    DW473.InversePixelValue118                       = INVERSE_PIXEL_VALUE_118_UNNAMED30208;
    DW473.InverseRChGammaCorrectedValue118           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_118_UNNAMED30208;

    DW474.Value                                      = 0;        
    DW474.ForwardGChGammaCorrectedValue118           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_118_UNNAMED30208;
    DW474.ForwardBChGammaCorrectedValue118           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_118_UNNAMED30208;

    DW475.Value                                      = 0;        
    DW475.ForwardPixelValue118                       = FORWARD_PIXEL_VALUE_118_UNNAMED30208;
    DW475.ForwardRChGammaCorrectedValue118           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_118_UNNAMED30208;

    DW476.Value                                      = 0;        
    DW476.InverseGChGammaCorrectedValue119           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_119_UNNAMED30464;
    DW476.InverseBChGammaCorrectedValue119           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_119_UNNAMED30464;

    DW477.Value                                      = 0;        
    DW477.InversePixelValue119                       = INVERSE_PIXEL_VALUE_119_UNNAMED30464;
    DW477.InverseRChGammaCorrectedValue119           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_119_UNNAMED30464;

    DW478.Value                                      = 0;        
    DW478.ForwardGChGammaCorrectedValue119           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_119_UNNAMED30464;
    DW478.ForwardBChGammaCorrectedValue119           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_119_UNNAMED30464;

    DW479.Value                                      = 0;        
    DW479.ForwardPixelValue119                       = FORWARD_PIXEL_VALUE_119_UNNAMED30464;
    DW479.ForwardRChGammaCorrectedValue119           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_119_UNNAMED30464;

    DW480.Value                                      = 0;        
    DW480.InverseGChGammaCorrectedValue120           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_120_UNNAMED30720;
    DW480.InverseBChGammaCorrectedValue120           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_120_UNNAMED30720;

    DW481.Value                                      = 0;        
    DW481.InversePixelValue120                       = INVERSE_PIXEL_VALUE_120_UNNAMED30720;
    DW481.InverseRChGammaCorrectedValue120           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_120_UNNAMED30720;

    DW482.Value                                      = 0;        
    DW482.ForwardGChGammaCorrectedValue120           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_120_UNNAMED30720;
    DW482.ForwardBChGammaCorrectedValue120           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_120_UNNAMED30720;

    DW483.Value                                      = 0;        
    DW483.ForwardPixelValue120                       = FORWARD_PIXEL_VALUE_120_UNNAMED30720;
    DW483.ForwardRChGammaCorrectedValue120           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_120_UNNAMED30720;

    DW484.Value                                      = 0;        
    DW484.InverseGChGammaCorrectedValue121           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_121_UNNAMED30976;
    DW484.InverseBChGammaCorrectedValue121           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_121_UNNAMED30976;

    DW485.Value                                      = 0;        
    DW485.InversePixelValue121                       = INVERSE_PIXEL_VALUE_121_UNNAMED30976;
    DW485.InverseRChGammaCorrectedValue121           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_121_UNNAMED30976;

    DW486.Value                                      = 0;        
    DW486.ForwardGChGammaCorrectedValue121           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_121_UNNAMED30976;
    DW486.ForwardBChGammaCorrectedValue121           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_121_UNNAMED30976;

    DW487.Value                                      = 0;        
    DW487.ForwardPixelValue121                       = FORWARD_PIXEL_VALUE_121_UNNAMED30976;
    DW487.ForwardRChGammaCorrectedValue121           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_121_UNNAMED30976;

    DW488.Value                                      = 0;        
    DW488.InverseGChGammaCorrectedValue122           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_122_UNNAMED31232;
    DW488.InverseBChGammaCorrectedValue122           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_122_UNNAMED31232;

    DW489.Value                                      = 0;        
    DW489.InversePixelValue122                       = INVERSE_PIXEL_VALUE_122_UNNAMED31232;
    DW489.InverseRChGammaCorrectedValue122           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_122_UNNAMED31232;

    DW490.Value                                      = 0;        
    DW490.ForwardGChGammaCorrectedValue122           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_122_UNNAMED31232;
    DW490.ForwardBChGammaCorrectedValue122           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_122_UNNAMED31232;

    DW491.Value                                      = 0;        
    DW491.ForwardPixelValue122                       = FORWARD_PIXEL_VALUE_122_UNNAMED31232;
    DW491.ForwardRChGammaCorrectedValue122           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_122_UNNAMED31232;

    DW492.Value                                      = 0;        
    DW492.InverseGChGammaCorrectedValue123           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_123_UNNAMED31488;
    DW492.InverseBChGammaCorrectedValue123           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_123_UNNAMED31488;

    DW493.Value                                      = 0;        
    DW493.InversePixelValue123                       = INVERSE_PIXEL_VALUE_123_UNNAMED31488;
    DW493.InverseRChGammaCorrectedValue123           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_123_UNNAMED31488;

    DW494.Value                                      = 0;        
    DW494.ForwardGChGammaCorrectedValue123           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_123_UNNAMED31488;
    DW494.ForwardBChGammaCorrectedValue123           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_123_UNNAMED31488;

    DW495.Value                                      = 0;        
    DW495.ForwardPixelValue123                       = FORWARD_PIXEL_VALUE_123_UNNAMED31488;
    DW495.ForwardRChGammaCorrectedValue123           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_123_UNNAMED31488;

    DW496.Value                                      = 0;        
    DW496.InverseGChGammaCorrectedValue124           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_124_UNNAMED31744;
    DW496.InverseBChGammaCorrectedValue124           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_124_UNNAMED31744;

    DW497.Value                                      = 0;        
    DW497.InversePixelValue124                       = INVERSE_PIXEL_VALUE_124_UNNAMED31744;
    DW497.InverseRChGammaCorrectedValue124           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_124_UNNAMED31744;

    DW498.Value                                      = 0;        
    DW498.ForwardGChGammaCorrectedValue124           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_124_UNNAMED31744;
    DW498.ForwardBChGammaCorrectedValue124           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_124_UNNAMED31744;

    DW499.Value                                      = 0;        
    DW499.ForwardPixelValue124                       = FORWARD_PIXEL_VALUE_124_UNNAMED31744;
    DW499.ForwardRChGammaCorrectedValue124           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_124_UNNAMED31744;

    DW500.Value                                      = 0;        
    DW500.InverseGChGammaCorrectedValue125           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_125_UNNAMED32000;
    DW500.InverseBChGammaCorrectedValue125           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_125_UNNAMED32000;

    DW501.Value                                      = 0;        
    DW501.InversePixelValue125                       = INVERSE_PIXEL_VALUE_125_UNNAMED32000;
    DW501.InverseRChGammaCorrectedValue125           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_125_UNNAMED32000;

    DW502.Value                                      = 0;        
    DW502.ForwardGChGammaCorrectedValue125           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_125_UNNAMED32000;
    DW502.ForwardBChGammaCorrectedValue125           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_125_UNNAMED32000;

    DW503.Value                                      = 0;        
    DW503.ForwardPixelValue125                       = FORWARD_PIXEL_VALUE_125_UNNAMED32000;
    DW503.ForwardRChGammaCorrectedValue125           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_125_UNNAMED32000;

    DW504.Value                                      = 0;        
    DW504.InverseGChGammaCorrectedValue126           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_126_UNNAMED32256;
    DW504.InverseBChGammaCorrectedValue126           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_126_UNNAMED32256;

    DW505.Value                                      = 0;        
    DW505.InversePixelValue126                       = INVERSE_PIXEL_VALUE_126_UNNAMED32256;
    DW505.InverseRChGammaCorrectedValue126           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_126_UNNAMED32256;

    DW506.Value                                      = 0;        
    DW506.ForwardGChGammaCorrectedValue126           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_126_UNNAMED32256;
    DW506.ForwardBChGammaCorrectedValue126           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_126_UNNAMED32256;

    DW507.Value                                      = 0;        
    DW507.ForwardPixelValue126                       = FORWARD_PIXEL_VALUE_126_UNNAMED32256;
    DW507.ForwardRChGammaCorrectedValue126           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_126_UNNAMED32256;

    DW508.Value                                      = 0;        
    DW508.InverseGChGammaCorrectedValue127           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_127_UNNAMED32512;
    DW508.InverseBChGammaCorrectedValue127           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_127_UNNAMED32512;

    DW509.Value                                      = 0;        
    DW509.InversePixelValue127                       = INVERSE_PIXEL_VALUE_127_UNNAMED32512;
    DW509.InverseRChGammaCorrectedValue127           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_127_UNNAMED32512;

    DW510.Value                                      = 0;        
    DW510.ForwardGChGammaCorrectedValue127           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_127_UNNAMED32512;
    DW510.ForwardBChGammaCorrectedValue127           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_127_UNNAMED32512;

    DW511.Value                                      = 0;        
    DW511.ForwardPixelValue127                       = FORWARD_PIXEL_VALUE_127_UNNAMED32512;
    DW511.ForwardRChGammaCorrectedValue127           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_127_UNNAMED32512;

    DW512.Value                                      = 0;        
    DW512.InverseGChGammaCorrectedValue128           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_128_UNNAMED32768;
    DW512.InverseBChGammaCorrectedValue128           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_128_UNNAMED32768;

    DW513.Value                                      = 0;        
    DW513.InversePixelValue128                       = INVERSE_PIXEL_VALUE_128_UNNAMED32768;
    DW513.InverseRChGammaCorrectedValue128           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_128_UNNAMED32768;

    DW514.Value                                      = 0;        
    DW514.ForwardGChGammaCorrectedValue128           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_128_UNNAMED32768;
    DW514.ForwardBChGammaCorrectedValue128           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_128_UNNAMED32768;

    DW515.Value                                      = 0;        
    DW515.ForwardPixelValue128                       = FORWARD_PIXEL_VALUE_128_UNNAMED32768;
    DW515.ForwardRChGammaCorrectedValue128           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_128_UNNAMED32768;

    DW516.Value                                      = 0;        
    DW516.InverseGChGammaCorrectedValue129           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_129_UNNAMED33024;
    DW516.InverseBChGammaCorrectedValue129           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_129_UNNAMED33024;

    DW517.Value                                      = 0;        
    DW517.InversePixelValue129                       = INVERSE_PIXEL_VALUE_129_UNNAMED33024;
    DW517.InverseRChGammaCorrectedValue129           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_129_UNNAMED33024;

    DW518.Value                                      = 0;        
    DW518.ForwardGChGammaCorrectedValue129           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_129_UNNAMED33024;
    DW518.ForwardBChGammaCorrectedValue129           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_129_UNNAMED33024;

    DW519.Value                                      = 0;        
    DW519.ForwardPixelValue129                       = FORWARD_PIXEL_VALUE_129_UNNAMED33024;
    DW519.ForwardRChGammaCorrectedValue129           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_129_UNNAMED33024;

    DW520.Value                                      = 0;        
    DW520.InverseGChGammaCorrectedValue130           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_130_UNNAMED33280;
    DW520.InverseBChGammaCorrectedValue130           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_130_UNNAMED33280;

    DW521.Value                                      = 0;        
    DW521.InversePixelValue130                       = INVERSE_PIXEL_VALUE_130_UNNAMED33280;
    DW521.InverseRChGammaCorrectedValue130           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_130_UNNAMED33280;

    DW522.Value                                      = 0;        
    DW522.ForwardGChGammaCorrectedValue130           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_130_UNNAMED33280;
    DW522.ForwardBChGammaCorrectedValue130           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_130_UNNAMED33280;

    DW523.Value                                      = 0;        
    DW523.ForwardPixelValue130                       = FORWARD_PIXEL_VALUE_130_UNNAMED33280;
    DW523.ForwardRChGammaCorrectedValue130           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_130_UNNAMED33280;

    DW524.Value                                      = 0;        
    DW524.InverseGChGammaCorrectedValue131           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_131_UNNAMED33536;
    DW524.InverseBChGammaCorrectedValue131           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_131_UNNAMED33536;

    DW525.Value                                      = 0;        
    DW525.InversePixelValue131                       = INVERSE_PIXEL_VALUE_131_UNNAMED33536;
    DW525.InverseRChGammaCorrectedValue131           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_131_UNNAMED33536;

    DW526.Value                                      = 0;        
    DW526.ForwardGChGammaCorrectedValue131           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_131_UNNAMED33536;
    DW526.ForwardBChGammaCorrectedValue131           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_131_UNNAMED33536;

    DW527.Value                                      = 0;        
    DW527.ForwardPixelValue131                       = FORWARD_PIXEL_VALUE_131_UNNAMED33536;
    DW527.ForwardRChGammaCorrectedValue131           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_131_UNNAMED33536;

    DW528.Value                                      = 0;        
    DW528.InverseGChGammaCorrectedValue132           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_132_UNNAMED33792;
    DW528.InverseBChGammaCorrectedValue132           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_132_UNNAMED33792;

    DW529.Value                                      = 0;        
    DW529.InversePixelValue132                       = INVERSE_PIXEL_VALUE_132_UNNAMED33792;
    DW529.InverseRChGammaCorrectedValue132           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_132_UNNAMED33792;

    DW530.Value                                      = 0;        
    DW530.ForwardGChGammaCorrectedValue132           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_132_UNNAMED33792;
    DW530.ForwardBChGammaCorrectedValue132           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_132_UNNAMED33792;

    DW531.Value                                      = 0;        
    DW531.ForwardPixelValue132                       = FORWARD_PIXEL_VALUE_132_UNNAMED33792;
    DW531.ForwardRChGammaCorrectedValue132           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_132_UNNAMED33792;

    DW532.Value                                      = 0;        
    DW532.InverseGChGammaCorrectedValue133           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_133_UNNAMED34048;
    DW532.InverseBChGammaCorrectedValue133           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_133_UNNAMED34048;

    DW533.Value                                      = 0;        
    DW533.InversePixelValue133                       = INVERSE_PIXEL_VALUE_133_UNNAMED34048;
    DW533.InverseRChGammaCorrectedValue133           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_133_UNNAMED34048;

    DW534.Value                                      = 0;        
    DW534.ForwardGChGammaCorrectedValue133           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_133_UNNAMED34048;
    DW534.ForwardBChGammaCorrectedValue133           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_133_UNNAMED34048;

    DW535.Value                                      = 0;        
    DW535.ForwardPixelValue133                       = FORWARD_PIXEL_VALUE_133_UNNAMED34048;
    DW535.ForwardRChGammaCorrectedValue133           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_133_UNNAMED34048;

    DW536.Value                                      = 0;        
    DW536.InverseGChGammaCorrectedValue134           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_134_UNNAMED34304;
    DW536.InverseBChGammaCorrectedValue134           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_134_UNNAMED34304;

    DW537.Value                                      = 0;        
    DW537.InversePixelValue134                       = INVERSE_PIXEL_VALUE_134_UNNAMED34304;
    DW537.InverseRChGammaCorrectedValue134           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_134_UNNAMED34304;

    DW538.Value                                      = 0;        
    DW538.ForwardGChGammaCorrectedValue134           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_134_UNNAMED34304;
    DW538.ForwardBChGammaCorrectedValue134           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_134_UNNAMED34304;

    DW539.Value                                      = 0;        
    DW539.ForwardPixelValue134                       = FORWARD_PIXEL_VALUE_134_UNNAMED34304;
    DW539.ForwardRChGammaCorrectedValue134           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_134_UNNAMED34304;

    DW540.Value                                      = 0;        
    DW540.InverseGChGammaCorrectedValue135           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_135_UNNAMED34560;
    DW540.InverseBChGammaCorrectedValue135           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_135_UNNAMED34560;

    DW541.Value                                      = 0;        
    DW541.InversePixelValue135                       = INVERSE_PIXEL_VALUE_135_UNNAMED34560;
    DW541.InverseRChGammaCorrectedValue135           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_135_UNNAMED34560;

    DW542.Value                                      = 0;        
    DW542.ForwardGChGammaCorrectedValue135           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_135_UNNAMED34560;
    DW542.ForwardBChGammaCorrectedValue135           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_135_UNNAMED34560;

    DW543.Value                                      = 0;        
    DW543.ForwardPixelValue135                       = FORWARD_PIXEL_VALUE_135_UNNAMED34560;
    DW543.ForwardRChGammaCorrectedValue135           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_135_UNNAMED34560;

    DW544.Value                                      = 0;        
    DW544.InverseGChGammaCorrectedValue136           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_136_UNNAMED34816;
    DW544.InverseBChGammaCorrectedValue136           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_136_UNNAMED34816;

    DW545.Value                                      = 0;        
    DW545.InversePixelValue136                       = INVERSE_PIXEL_VALUE_136_UNNAMED34816;
    DW545.InverseRChGammaCorrectedValue136           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_136_UNNAMED34816;

    DW546.Value                                      = 0;        
    DW546.ForwardGChGammaCorrectedValue136           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_136_UNNAMED34816;
    DW546.ForwardBChGammaCorrectedValue136           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_136_UNNAMED34816;

    DW547.Value                                      = 0;        
    DW547.ForwardPixelValue136                       = FORWARD_PIXEL_VALUE_136_UNNAMED34816;
    DW547.ForwardRChGammaCorrectedValue136           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_136_UNNAMED34816;

    DW548.Value                                      = 0;        
    DW548.InverseGChGammaCorrectedValue137           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_137_UNNAMED35072;
    DW548.InverseBChGammaCorrectedValue137           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_137_UNNAMED35072;

    DW549.Value                                      = 0;        
    DW549.InversePixelValue137                       = INVERSE_PIXEL_VALUE_137_UNNAMED35072;
    DW549.InverseRChGammaCorrectedValue137           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_137_UNNAMED35072;

    DW550.Value                                      = 0;        
    DW550.ForwardGChGammaCorrectedValue137           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_137_UNNAMED35072;
    DW550.ForwardBChGammaCorrectedValue137           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_137_UNNAMED35072;

    DW551.Value                                      = 0;        
    DW551.ForwardPixelValue137                       = FORWARD_PIXEL_VALUE_137_UNNAMED35072;
    DW551.ForwardRChGammaCorrectedValue137           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_137_UNNAMED35072;

    DW552.Value                                      = 0;        
    DW552.InverseGChGammaCorrectedValue138           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_138_UNNAMED35328;
    DW552.InverseBChGammaCorrectedValue138           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_138_UNNAMED35328;

    DW553.Value                                      = 0;        
    DW553.InversePixelValue138                       = INVERSE_PIXEL_VALUE_138_UNNAMED35328;
    DW553.InverseRChGammaCorrectedValue138           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_138_UNNAMED35328;

    DW554.Value                                      = 0;        
    DW554.ForwardGChGammaCorrectedValue138           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_138_UNNAMED35328;
    DW554.ForwardBChGammaCorrectedValue138           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_138_UNNAMED35328;

    DW555.Value                                      = 0;        
    DW555.ForwardPixelValue138                       = FORWARD_PIXEL_VALUE_138_UNNAMED35328;
    DW555.ForwardRChGammaCorrectedValue138           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_138_UNNAMED35328;

    DW556.Value                                      = 0;        
    DW556.InverseGChGammaCorrectedValue139           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_139_UNNAMED35584;
    DW556.InverseBChGammaCorrectedValue139           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_139_UNNAMED35584;

    DW557.Value                                      = 0;        
    DW557.InversePixelValue139                       = INVERSE_PIXEL_VALUE_139_UNNAMED35584;
    DW557.InverseRChGammaCorrectedValue139           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_139_UNNAMED35584;

    DW558.Value                                      = 0;        
    DW558.ForwardGChGammaCorrectedValue139           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_139_UNNAMED35584;
    DW558.ForwardBChGammaCorrectedValue139           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_139_UNNAMED35584;

    DW559.Value                                      = 0;        
    DW559.ForwardPixelValue139                       = FORWARD_PIXEL_VALUE_139_UNNAMED35584;
    DW559.ForwardRChGammaCorrectedValue139           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_139_UNNAMED35584;

    DW560.Value                                      = 0;        
    DW560.InverseGChGammaCorrectedValue140           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_140_UNNAMED35840;
    DW560.InverseBChGammaCorrectedValue140           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_140_UNNAMED35840;

    DW561.Value                                      = 0;        
    DW561.InversePixelValue140                       = INVERSE_PIXEL_VALUE_140_UNNAMED35840;
    DW561.InverseRChGammaCorrectedValue140           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_140_UNNAMED35840;

    DW562.Value                                      = 0;        
    DW562.ForwardGChGammaCorrectedValue140           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_140_UNNAMED35840;
    DW562.ForwardBChGammaCorrectedValue140           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_140_UNNAMED35840;

    DW563.Value                                      = 0;        
    DW563.ForwardPixelValue140                       = FORWARD_PIXEL_VALUE_140_UNNAMED35840;
    DW563.ForwardRChGammaCorrectedValue140           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_140_UNNAMED35840;

    DW564.Value                                      = 0;        
    DW564.InverseGChGammaCorrectedValue141           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_141_UNNAMED36096;
    DW564.InverseBChGammaCorrectedValue141           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_141_UNNAMED36096;

    DW565.Value                                      = 0;        
    DW565.InversePixelValue141                       = INVERSE_PIXEL_VALUE_141_UNNAMED36096;
    DW565.InverseRChGammaCorrectedValue141           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_141_UNNAMED36096;

    DW566.Value                                      = 0;        
    DW566.ForwardGChGammaCorrectedValue141           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_141_UNNAMED36096;
    DW566.ForwardBChGammaCorrectedValue141           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_141_UNNAMED36096;

    DW567.Value                                      = 0;        
    DW567.ForwardPixelValue141                       = FORWARD_PIXEL_VALUE_141_UNNAMED36096;
    DW567.ForwardRChGammaCorrectedValue141           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_141_UNNAMED36096;

    DW568.Value                                      = 0;        
    DW568.InverseGChGammaCorrectedValue142           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_142_UNNAMED36352;
    DW568.InverseBChGammaCorrectedValue142           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_142_UNNAMED36352;

    DW569.Value                                      = 0;        
    DW569.InversePixelValue142                       = INVERSE_PIXEL_VALUE_142_UNNAMED36352;
    DW569.InverseRChGammaCorrectedValue142           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_142_UNNAMED36352;

    DW570.Value                                      = 0;        
    DW570.ForwardGChGammaCorrectedValue142           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_142_UNNAMED36352;
    DW570.ForwardBChGammaCorrectedValue142           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_142_UNNAMED36352;

    DW571.Value                                      = 0;        
    DW571.ForwardPixelValue142                       = FORWARD_PIXEL_VALUE_142_UNNAMED36352;
    DW571.ForwardRChGammaCorrectedValue142           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_142_UNNAMED36352;

    DW572.Value                                      = 0;        
    DW572.InverseGChGammaCorrectedValue143           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_143_UNNAMED36608;
    DW572.InverseBChGammaCorrectedValue143           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_143_UNNAMED36608;

    DW573.Value                                      = 0;        
    DW573.InversePixelValue143                       = INVERSE_PIXEL_VALUE_143_UNNAMED36608;
    DW573.InverseRChGammaCorrectedValue143           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_143_UNNAMED36608;

    DW574.Value                                      = 0;        
    DW574.ForwardGChGammaCorrectedValue143           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_143_UNNAMED36608;
    DW574.ForwardBChGammaCorrectedValue143           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_143_UNNAMED36608;

    DW575.Value                                      = 0;        
    DW575.ForwardPixelValue143                       = FORWARD_PIXEL_VALUE_143_UNNAMED36608;
    DW575.ForwardRChGammaCorrectedValue143           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_143_UNNAMED36608;

    DW576.Value                                      = 0;        
    DW576.InverseGChGammaCorrectedValue144           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_144_UNNAMED36864;
    DW576.InverseBChGammaCorrectedValue144           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_144_UNNAMED36864;

    DW577.Value                                      = 0;        
    DW577.InversePixelValue144                       = INVERSE_PIXEL_VALUE_144_UNNAMED36864;
    DW577.InverseRChGammaCorrectedValue144           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_144_UNNAMED36864;

    DW578.Value                                      = 0;        
    DW578.ForwardGChGammaCorrectedValue144           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_144_UNNAMED36864;
    DW578.ForwardBChGammaCorrectedValue144           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_144_UNNAMED36864;

    DW579.Value                                      = 0;        
    DW579.ForwardPixelValue144                       = FORWARD_PIXEL_VALUE_144_UNNAMED36864;
    DW579.ForwardRChGammaCorrectedValue144           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_144_UNNAMED36864;

    DW580.Value                                      = 0;        
    DW580.InverseGChGammaCorrectedValue145           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_145_UNNAMED37120;
    DW580.InverseBChGammaCorrectedValue145           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_145_UNNAMED37120;

    DW581.Value                                      = 0;        
    DW581.InversePixelValue145                       = INVERSE_PIXEL_VALUE_145_UNNAMED37120;
    DW581.InverseRChGammaCorrectedValue145           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_145_UNNAMED37120;

    DW582.Value                                      = 0;        
    DW582.ForwardGChGammaCorrectedValue145           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_145_UNNAMED37120;
    DW582.ForwardBChGammaCorrectedValue145           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_145_UNNAMED37120;

    DW583.Value                                      = 0;        
    DW583.ForwardPixelValue145                       = FORWARD_PIXEL_VALUE_145_UNNAMED37120;
    DW583.ForwardRChGammaCorrectedValue145           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_145_UNNAMED37120;

    DW584.Value                                      = 0;        
    DW584.InverseGChGammaCorrectedValue146           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_146_UNNAMED37376;
    DW584.InverseBChGammaCorrectedValue146           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_146_UNNAMED37376;

    DW585.Value                                      = 0;        
    DW585.InversePixelValue146                       = INVERSE_PIXEL_VALUE_146_UNNAMED37376;
    DW585.InverseRChGammaCorrectedValue146           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_146_UNNAMED37376;

    DW586.Value                                      = 0;        
    DW586.ForwardGChGammaCorrectedValue146           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_146_UNNAMED37376;
    DW586.ForwardBChGammaCorrectedValue146           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_146_UNNAMED37376;

    DW587.Value                                      = 0;        
    DW587.ForwardPixelValue146                       = FORWARD_PIXEL_VALUE_146_UNNAMED37376;
    DW587.ForwardRChGammaCorrectedValue146           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_146_UNNAMED37376;

    DW588.Value                                      = 0;        
    DW588.InverseGChGammaCorrectedValue147           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_147_UNNAMED37632;
    DW588.InverseBChGammaCorrectedValue147           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_147_UNNAMED37632;

    DW589.Value                                      = 0;        
    DW589.InversePixelValue147                       = INVERSE_PIXEL_VALUE_147_UNNAMED37632;
    DW589.InverseRChGammaCorrectedValue147           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_147_UNNAMED37632;

    DW590.Value                                      = 0;        
    DW590.ForwardGChGammaCorrectedValue147           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_147_UNNAMED37632;
    DW590.ForwardBChGammaCorrectedValue147           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_147_UNNAMED37632;

    DW591.Value                                      = 0;        
    DW591.ForwardPixelValue147                       = FORWARD_PIXEL_VALUE_147_UNNAMED37632;
    DW591.ForwardRChGammaCorrectedValue147           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_147_UNNAMED37632;

    DW592.Value                                      = 0;        
    DW592.InverseGChGammaCorrectedValue148           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_148_UNNAMED37888;
    DW592.InverseBChGammaCorrectedValue148           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_148_UNNAMED37888;

    DW593.Value                                      = 0;        
    DW593.InversePixelValue148                       = INVERSE_PIXEL_VALUE_148_UNNAMED37888;
    DW593.InverseRChGammaCorrectedValue148           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_148_UNNAMED37888;

    DW594.Value                                      = 0;        
    DW594.ForwardGChGammaCorrectedValue148           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_148_UNNAMED37888;
    DW594.ForwardBChGammaCorrectedValue148           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_148_UNNAMED37888;

    DW595.Value                                      = 0;        
    DW595.ForwardPixelValue148                       = FORWARD_PIXEL_VALUE_148_UNNAMED37888;
    DW595.ForwardRChGammaCorrectedValue148           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_148_UNNAMED37888;

    DW596.Value                                      = 0;        
    DW596.InverseGChGammaCorrectedValue149           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_149_UNNAMED38144;
    DW596.InverseBChGammaCorrectedValue149           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_149_UNNAMED38144;

    DW597.Value                                      = 0;        
    DW597.InversePixelValue149                       = INVERSE_PIXEL_VALUE_149_UNNAMED38144;
    DW597.InverseRChGammaCorrectedValue149           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_149_UNNAMED38144;

    DW598.Value                                      = 0;        
    DW598.ForwardGChGammaCorrectedValue149           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_149_UNNAMED38144;
    DW598.ForwardBChGammaCorrectedValue149           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_149_UNNAMED38144;

    DW599.Value                                      = 0;        
    DW599.ForwardPixelValue149                       = FORWARD_PIXEL_VALUE_149_UNNAMED38144;
    DW599.ForwardRChGammaCorrectedValue149           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_149_UNNAMED38144;

    DW600.Value                                      = 0;        
    DW600.InverseGChGammaCorrectedValue150           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_150_UNNAMED38400;
    DW600.InverseBChGammaCorrectedValue150           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_150_UNNAMED38400;

    DW601.Value                                      = 0;        
    DW601.InversePixelValue150                       = INVERSE_PIXEL_VALUE_150_UNNAMED38400;
    DW601.InverseRChGammaCorrectedValue150           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_150_UNNAMED38400;

    DW602.Value                                      = 0;        
    DW602.ForwardGChGammaCorrectedValue150           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_150_UNNAMED38400;
    DW602.ForwardBChGammaCorrectedValue150           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_150_UNNAMED38400;

    DW603.Value                                      = 0;        
    DW603.ForwardPixelValue150                       = FORWARD_PIXEL_VALUE_150_UNNAMED38400;
    DW603.ForwardRChGammaCorrectedValue150           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_150_UNNAMED38400;

    DW604.Value                                      = 0;        
    DW604.InverseGChGammaCorrectedValue151           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_151_UNNAMED38656;
    DW604.InverseBChGammaCorrectedValue151           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_151_UNNAMED38656;

    DW605.Value                                      = 0;        
    DW605.InversePixelValue151                       = INVERSE_PIXEL_VALUE_151_UNNAMED38656;
    DW605.InverseRChGammaCorrectedValue151           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_151_UNNAMED38656;

    DW606.Value                                      = 0;        
    DW606.ForwardGChGammaCorrectedValue151           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_151_UNNAMED38656;
    DW606.ForwardBChGammaCorrectedValue151           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_151_UNNAMED38656;

    DW607.Value                                      = 0;        
    DW607.ForwardPixelValue151                       = FORWARD_PIXEL_VALUE_151_UNNAMED38656;
    DW607.ForwardRChGammaCorrectedValue151           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_151_UNNAMED38656;

    DW608.Value                                      = 0;        
    DW608.InverseGChGammaCorrectedValue152           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_152_UNNAMED38912;
    DW608.InverseBChGammaCorrectedValue152           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_152_UNNAMED38912;

    DW609.Value                                      = 0;        
    DW609.InversePixelValue152                       = INVERSE_PIXEL_VALUE_152_UNNAMED38912;
    DW609.InverseRChGammaCorrectedValue152           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_152_UNNAMED38912;

    DW610.Value                                      = 0;        
    DW610.ForwardGChGammaCorrectedValue152           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_152_UNNAMED38912;
    DW610.ForwardBChGammaCorrectedValue152           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_152_UNNAMED38912;

    DW611.Value                                      = 0;        
    DW611.ForwardPixelValue152                       = FORWARD_PIXEL_VALUE_152_UNNAMED38912;
    DW611.ForwardRChGammaCorrectedValue152           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_152_UNNAMED38912;

    DW612.Value                                      = 0;        
    DW612.InverseGChGammaCorrectedValue153           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_153_UNNAMED39168;
    DW612.InverseBChGammaCorrectedValue153           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_153_UNNAMED39168;

    DW613.Value                                      = 0;        
    DW613.InversePixelValue153                       = INVERSE_PIXEL_VALUE_153_UNNAMED39168;
    DW613.InverseRChGammaCorrectedValue153           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_153_UNNAMED39168;

    DW614.Value                                      = 0;        
    DW614.ForwardGChGammaCorrectedValue153           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_153_UNNAMED39168;
    DW614.ForwardBChGammaCorrectedValue153           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_153_UNNAMED39168;

    DW615.Value                                      = 0;        
    DW615.ForwardPixelValue153                       = FORWARD_PIXEL_VALUE_153_UNNAMED39168;
    DW615.ForwardRChGammaCorrectedValue153           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_153_UNNAMED39168;

    DW616.Value                                      = 0;        
    DW616.InverseGChGammaCorrectedValue154           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_154_UNNAMED39424;
    DW616.InverseBChGammaCorrectedValue154           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_154_UNNAMED39424;

    DW617.Value                                      = 0;        
    DW617.InversePixelValue154                       = INVERSE_PIXEL_VALUE_154_UNNAMED39424;
    DW617.InverseRChGammaCorrectedValue154           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_154_UNNAMED39424;

    DW618.Value                                      = 0;        
    DW618.ForwardGChGammaCorrectedValue154           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_154_UNNAMED39424;
    DW618.ForwardBChGammaCorrectedValue154           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_154_UNNAMED39424;

    DW619.Value                                      = 0;        
    DW619.ForwardPixelValue154                       = FORWARD_PIXEL_VALUE_154_UNNAMED39424;
    DW619.ForwardRChGammaCorrectedValue154           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_154_UNNAMED39424;

    DW620.Value                                      = 0;        
    DW620.InverseGChGammaCorrectedValue155           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_155_UNNAMED39680;
    DW620.InverseBChGammaCorrectedValue155           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_155_UNNAMED39680;

    DW621.Value                                      = 0;        
    DW621.InversePixelValue155                       = INVERSE_PIXEL_VALUE_155_UNNAMED39680;
    DW621.InverseRChGammaCorrectedValue155           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_155_UNNAMED39680;

    DW622.Value                                      = 0;        
    DW622.ForwardGChGammaCorrectedValue155           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_155_UNNAMED39680;
    DW622.ForwardBChGammaCorrectedValue155           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_155_UNNAMED39680;

    DW623.Value                                      = 0;        
    DW623.ForwardPixelValue155                       = FORWARD_PIXEL_VALUE_155_UNNAMED39680;
    DW623.ForwardRChGammaCorrectedValue155           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_155_UNNAMED39680;

    DW624.Value                                      = 0;        
    DW624.InverseGChGammaCorrectedValue156           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_156_UNNAMED39936;
    DW624.InverseBChGammaCorrectedValue156           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_156_UNNAMED39936;

    DW625.Value                                      = 0;        
    DW625.InversePixelValue156                       = INVERSE_PIXEL_VALUE_156_UNNAMED39936;
    DW625.InverseRChGammaCorrectedValue156           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_156_UNNAMED39936;

    DW626.Value                                      = 0;        
    DW626.ForwardGChGammaCorrectedValue156           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_156_UNNAMED39936;
    DW626.ForwardBChGammaCorrectedValue156           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_156_UNNAMED39936;

    DW627.Value                                      = 0;        
    DW627.ForwardPixelValue156                       = FORWARD_PIXEL_VALUE_156_UNNAMED39936;
    DW627.ForwardRChGammaCorrectedValue156           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_156_UNNAMED39936;

    DW628.Value                                      = 0;        
    DW628.InverseGChGammaCorrectedValue157           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_157_UNNAMED40192;
    DW628.InverseBChGammaCorrectedValue157           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_157_UNNAMED40192;

    DW629.Value                                      = 0;        
    DW629.InversePixelValue157                       = INVERSE_PIXEL_VALUE_157_UNNAMED40192;
    DW629.InverseRChGammaCorrectedValue157           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_157_UNNAMED40192;

    DW630.Value                                      = 0;        
    DW630.ForwardGChGammaCorrectedValue157           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_157_UNNAMED40192;
    DW630.ForwardBChGammaCorrectedValue157           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_157_UNNAMED40192;

    DW631.Value                                      = 0;        
    DW631.ForwardPixelValue157                       = FORWARD_PIXEL_VALUE_157_UNNAMED40192;
    DW631.ForwardRChGammaCorrectedValue157           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_157_UNNAMED40192;

    DW632.Value                                      = 0;        
    DW632.InverseGChGammaCorrectedValue158           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_158_UNNAMED40448;
    DW632.InverseBChGammaCorrectedValue158           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_158_UNNAMED40448;

    DW633.Value                                      = 0;        
    DW633.InversePixelValue158                       = INVERSE_PIXEL_VALUE_158_UNNAMED40448;
    DW633.InverseRChGammaCorrectedValue158           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_158_UNNAMED40448;

    DW634.Value                                      = 0;        
    DW634.ForwardGChGammaCorrectedValue158           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_158_UNNAMED40448;
    DW634.ForwardBChGammaCorrectedValue158           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_158_UNNAMED40448;

    DW635.Value                                      = 0;        
    DW635.ForwardPixelValue158                       = FORWARD_PIXEL_VALUE_158_UNNAMED40448;
    DW635.ForwardRChGammaCorrectedValue158           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_158_UNNAMED40448;

    DW636.Value                                      = 0;        
    DW636.InverseGChGammaCorrectedValue159           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_159_UNNAMED40704;
    DW636.InverseBChGammaCorrectedValue159           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_159_UNNAMED40704;

    DW637.Value                                      = 0;        
    DW637.InversePixelValue159                       = INVERSE_PIXEL_VALUE_159_UNNAMED40704;
    DW637.InverseRChGammaCorrectedValue159           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_159_UNNAMED40704;

    DW638.Value                                      = 0;        
    DW638.ForwardGChGammaCorrectedValue159           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_159_UNNAMED40704;
    DW638.ForwardBChGammaCorrectedValue159           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_159_UNNAMED40704;

    DW639.Value                                      = 0;        
    DW639.ForwardPixelValue159                       = FORWARD_PIXEL_VALUE_159_UNNAMED40704;
    DW639.ForwardRChGammaCorrectedValue159           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_159_UNNAMED40704;

    DW640.Value                                      = 0;        
    DW640.InverseGChGammaCorrectedValue160           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_160_UNNAMED40960;
    DW640.InverseBChGammaCorrectedValue160           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_160_UNNAMED40960;

    DW641.Value                                      = 0;        
    DW641.InversePixelValue160                       = INVERSE_PIXEL_VALUE_160_UNNAMED40960;
    DW641.InverseRChGammaCorrectedValue160           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_160_UNNAMED40960;

    DW642.Value                                      = 0;        
    DW642.ForwardGChGammaCorrectedValue160           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_160_UNNAMED40960;
    DW642.ForwardBChGammaCorrectedValue160           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_160_UNNAMED40960;

    DW643.Value                                      = 0;        
    DW643.ForwardPixelValue160                       = FORWARD_PIXEL_VALUE_160_UNNAMED40960;
    DW643.ForwardRChGammaCorrectedValue160           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_160_UNNAMED40960;

    DW644.Value                                      = 0;        
    DW644.InverseGChGammaCorrectedValue161           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_161_UNNAMED41216;
    DW644.InverseBChGammaCorrectedValue161           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_161_UNNAMED41216;

    DW645.Value                                      = 0;        
    DW645.InversePixelValue161                       = INVERSE_PIXEL_VALUE_161_UNNAMED41216;
    DW645.InverseRChGammaCorrectedValue161           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_161_UNNAMED41216;

    DW646.Value                                      = 0;        
    DW646.ForwardGChGammaCorrectedValue161           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_161_UNNAMED41216;
    DW646.ForwardBChGammaCorrectedValue161           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_161_UNNAMED41216;

    DW647.Value                                      = 0;        
    DW647.ForwardPixelValue161                       = FORWARD_PIXEL_VALUE_161_UNNAMED41216;
    DW647.ForwardRChGammaCorrectedValue161           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_161_UNNAMED41216;

    DW648.Value                                      = 0;        
    DW648.InverseGChGammaCorrectedValue162           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_162_UNNAMED41472;
    DW648.InverseBChGammaCorrectedValue162           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_162_UNNAMED41472;

    DW649.Value                                      = 0;        
    DW649.InversePixelValue162                       = INVERSE_PIXEL_VALUE_162_UNNAMED41472;
    DW649.InverseRChGammaCorrectedValue162           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_162_UNNAMED41472;

    DW650.Value                                      = 0;        
    DW650.ForwardGChGammaCorrectedValue162           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_162_UNNAMED41472;
    DW650.ForwardBChGammaCorrectedValue162           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_162_UNNAMED41472;

    DW651.Value                                      = 0;        
    DW651.ForwardPixelValue162                       = FORWARD_PIXEL_VALUE_162_UNNAMED41472;
    DW651.ForwardRChGammaCorrectedValue162           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_162_UNNAMED41472;

    DW652.Value                                      = 0;        
    DW652.InverseGChGammaCorrectedValue163           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_163_UNNAMED41728;
    DW652.InverseBChGammaCorrectedValue163           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_163_UNNAMED41728;

    DW653.Value                                      = 0;        
    DW653.InversePixelValue163                       = INVERSE_PIXEL_VALUE_163_UNNAMED41728;
    DW653.InverseRChGammaCorrectedValue163           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_163_UNNAMED41728;

    DW654.Value                                      = 0;        
    DW654.ForwardGChGammaCorrectedValue163           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_163_UNNAMED41728;
    DW654.ForwardBChGammaCorrectedValue163           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_163_UNNAMED41728;

    DW655.Value                                      = 0;        
    DW655.ForwardPixelValue163                       = FORWARD_PIXEL_VALUE_163_UNNAMED41728;
    DW655.ForwardRChGammaCorrectedValue163           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_163_UNNAMED41728;

    DW656.Value                                      = 0;        
    DW656.InverseGChGammaCorrectedValue164           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_164_UNNAMED41984;
    DW656.InverseBChGammaCorrectedValue164           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_164_UNNAMED41984;

    DW657.Value                                      = 0;        
    DW657.InversePixelValue164                       = INVERSE_PIXEL_VALUE_164_UNNAMED41984;
    DW657.InverseRChGammaCorrectedValue164           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_164_UNNAMED41984;

    DW658.Value                                      = 0;        
    DW658.ForwardGChGammaCorrectedValue164           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_164_UNNAMED41984;
    DW658.ForwardBChGammaCorrectedValue164           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_164_UNNAMED41984;

    DW659.Value                                      = 0;        
    DW659.ForwardPixelValue164                       = FORWARD_PIXEL_VALUE_164_UNNAMED41984;
    DW659.ForwardRChGammaCorrectedValue164           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_164_UNNAMED41984;

    DW660.Value                                      = 0;        
    DW660.InverseGChGammaCorrectedValue165           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_165_UNNAMED42240;
    DW660.InverseBChGammaCorrectedValue165           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_165_UNNAMED42240;

    DW661.Value                                      = 0;        
    DW661.InversePixelValue165                       = INVERSE_PIXEL_VALUE_165_UNNAMED42240;
    DW661.InverseRChGammaCorrectedValue165           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_165_UNNAMED42240;

    DW662.Value                                      = 0;        
    DW662.ForwardGChGammaCorrectedValue165           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_165_UNNAMED42240;
    DW662.ForwardBChGammaCorrectedValue165           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_165_UNNAMED42240;

    DW663.Value                                      = 0;        
    DW663.ForwardPixelValue165                       = FORWARD_PIXEL_VALUE_165_UNNAMED42240;
    DW663.ForwardRChGammaCorrectedValue165           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_165_UNNAMED42240;

    DW664.Value                                      = 0;        
    DW664.InverseGChGammaCorrectedValue166           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_166_UNNAMED42496;
    DW664.InverseBChGammaCorrectedValue166           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_166_UNNAMED42496;

    DW665.Value                                      = 0;        
    DW665.InversePixelValue166                       = INVERSE_PIXEL_VALUE_166_UNNAMED42496;
    DW665.InverseRChGammaCorrectedValue166           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_166_UNNAMED42496;

    DW666.Value                                      = 0;        
    DW666.ForwardGChGammaCorrectedValue166           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_166_UNNAMED42496;
    DW666.ForwardBChGammaCorrectedValue166           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_166_UNNAMED42496;

    DW667.Value                                      = 0;        
    DW667.ForwardPixelValue166                       = FORWARD_PIXEL_VALUE_166_UNNAMED42496;
    DW667.ForwardRChGammaCorrectedValue166           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_166_UNNAMED42496;

    DW668.Value                                      = 0;        
    DW668.InverseGChGammaCorrectedValue167           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_167_UNNAMED42752;
    DW668.InverseBChGammaCorrectedValue167           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_167_UNNAMED42752;

    DW669.Value                                      = 0;        
    DW669.InversePixelValue167                       = INVERSE_PIXEL_VALUE_167_UNNAMED42752;
    DW669.InverseRChGammaCorrectedValue167           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_167_UNNAMED42752;

    DW670.Value                                      = 0;        
    DW670.ForwardGChGammaCorrectedValue167           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_167_UNNAMED42752;
    DW670.ForwardBChGammaCorrectedValue167           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_167_UNNAMED42752;

    DW671.Value                                      = 0;        
    DW671.ForwardPixelValue167                       = FORWARD_PIXEL_VALUE_167_UNNAMED42752;
    DW671.ForwardRChGammaCorrectedValue167           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_167_UNNAMED42752;

    DW672.Value                                      = 0;        
    DW672.InverseGChGammaCorrectedValue168           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_168_UNNAMED43008;
    DW672.InverseBChGammaCorrectedValue168           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_168_UNNAMED43008;

    DW673.Value                                      = 0;        
    DW673.InversePixelValue168                       = INVERSE_PIXEL_VALUE_168_UNNAMED43008;
    DW673.InverseRChGammaCorrectedValue168           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_168_UNNAMED43008;

    DW674.Value                                      = 0;        
    DW674.ForwardGChGammaCorrectedValue168           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_168_UNNAMED43008;
    DW674.ForwardBChGammaCorrectedValue168           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_168_UNNAMED43008;

    DW675.Value                                      = 0;        
    DW675.ForwardPixelValue168                       = FORWARD_PIXEL_VALUE_168_UNNAMED43008;
    DW675.ForwardRChGammaCorrectedValue168           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_168_UNNAMED43008;

    DW676.Value                                      = 0;        
    DW676.InverseGChGammaCorrectedValue169           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_169_UNNAMED43264;
    DW676.InverseBChGammaCorrectedValue169           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_169_UNNAMED43264;

    DW677.Value                                      = 0;        
    DW677.InversePixelValue169                       = INVERSE_PIXEL_VALUE_169_UNNAMED43264;
    DW677.InverseRChGammaCorrectedValue169           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_169_UNNAMED43264;

    DW678.Value                                      = 0;        
    DW678.ForwardGChGammaCorrectedValue169           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_169_UNNAMED43264;
    DW678.ForwardBChGammaCorrectedValue169           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_169_UNNAMED43264;

    DW679.Value                                      = 0;        
    DW679.ForwardPixelValue169                       = FORWARD_PIXEL_VALUE_169_UNNAMED43264;
    DW679.ForwardRChGammaCorrectedValue169           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_169_UNNAMED43264;

    DW680.Value                                      = 0;        
    DW680.InverseGChGammaCorrectedValue170           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_170_UNNAMED43520;
    DW680.InverseBChGammaCorrectedValue170           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_170_UNNAMED43520;

    DW681.Value                                      = 0;        
    DW681.InversePixelValue170                       = INVERSE_PIXEL_VALUE_170_UNNAMED43520;
    DW681.InverseRChGammaCorrectedValue170           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_170_UNNAMED43520;

    DW682.Value                                      = 0;        
    DW682.ForwardGChGammaCorrectedValue170           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_170_UNNAMED43520;
    DW682.ForwardBChGammaCorrectedValue170           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_170_UNNAMED43520;

    DW683.Value                                      = 0;        
    DW683.ForwardPixelValue170                       = FORWARD_PIXEL_VALUE_170_UNNAMED43520;
    DW683.ForwardRChGammaCorrectedValue170           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_170_UNNAMED43520;

    DW684.Value                                      = 0;        
    DW684.InverseGChGammaCorrectedValue171           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_171_UNNAMED43776;
    DW684.InverseBChGammaCorrectedValue171           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_171_UNNAMED43776;

    DW685.Value                                      = 0;        
    DW685.InversePixelValue171                       = INVERSE_PIXEL_VALUE_171_UNNAMED43776;
    DW685.InverseRChGammaCorrectedValue171           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_171_UNNAMED43776;

    DW686.Value                                      = 0;        
    DW686.ForwardGChGammaCorrectedValue171           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_171_UNNAMED43776;
    DW686.ForwardBChGammaCorrectedValue171           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_171_UNNAMED43776;

    DW687.Value                                      = 0;        
    DW687.ForwardPixelValue171                       = FORWARD_PIXEL_VALUE_171_UNNAMED43776;
    DW687.ForwardRChGammaCorrectedValue171           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_171_UNNAMED43776;

    DW688.Value                                      = 0;        
    DW688.InverseGChGammaCorrectedValue172           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_172_UNNAMED44032;
    DW688.InverseBChGammaCorrectedValue172           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_172_UNNAMED44032;

    DW689.Value                                      = 0;        
    DW689.InversePixelValue172                       = INVERSE_PIXEL_VALUE_172_UNNAMED44032;
    DW689.InverseRChGammaCorrectedValue172           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_172_UNNAMED44032;

    DW690.Value                                      = 0;        
    DW690.ForwardGChGammaCorrectedValue172           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_172_UNNAMED44032;
    DW690.ForwardBChGammaCorrectedValue172           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_172_UNNAMED44032;

    DW691.Value                                      = 0;        
    DW691.ForwardPixelValue172                       = FORWARD_PIXEL_VALUE_172_UNNAMED44032;
    DW691.ForwardRChGammaCorrectedValue172           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_172_UNNAMED44032;

    DW692.Value                                      = 0;        
    DW692.InverseGChGammaCorrectedValue173           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_173_UNNAMED44288;
    DW692.InverseBChGammaCorrectedValue173           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_173_UNNAMED44288;

    DW693.Value                                      = 0;        
    DW693.InversePixelValue173                       = INVERSE_PIXEL_VALUE_173_UNNAMED44288;
    DW693.InverseRChGammaCorrectedValue173           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_173_UNNAMED44288;

    DW694.Value                                      = 0;        
    DW694.ForwardGChGammaCorrectedValue173           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_173_UNNAMED44288;
    DW694.ForwardBChGammaCorrectedValue173           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_173_UNNAMED44288;

    DW695.Value                                      = 0;        
    DW695.ForwardPixelValue173                       = FORWARD_PIXEL_VALUE_173_UNNAMED44288;
    DW695.ForwardRChGammaCorrectedValue173           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_173_UNNAMED44288;

    DW696.Value                                      = 0;        
    DW696.InverseGChGammaCorrectedValue174           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_174_UNNAMED44544;
    DW696.InverseBChGammaCorrectedValue174           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_174_UNNAMED44544;

    DW697.Value                                      = 0;        
    DW697.InversePixelValue174                       = INVERSE_PIXEL_VALUE_174_UNNAMED44544;
    DW697.InverseRChGammaCorrectedValue174           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_174_UNNAMED44544;

    DW698.Value                                      = 0;        
    DW698.ForwardGChGammaCorrectedValue174           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_174_UNNAMED44544;
    DW698.ForwardBChGammaCorrectedValue174           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_174_UNNAMED44544;

    DW699.Value                                      = 0;        
    DW699.ForwardPixelValue174                       = FORWARD_PIXEL_VALUE_174_UNNAMED44544;
    DW699.ForwardRChGammaCorrectedValue174           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_174_UNNAMED44544;

    DW700.Value                                      = 0;        
    DW700.InverseGChGammaCorrectedValue175           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_175_UNNAMED44800;
    DW700.InverseBChGammaCorrectedValue175           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_175_UNNAMED44800;

    DW701.Value                                      = 0;        
    DW701.InversePixelValue175                       = INVERSE_PIXEL_VALUE_175_UNNAMED44800;
    DW701.InverseRChGammaCorrectedValue175           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_175_UNNAMED44800;

    DW702.Value                                      = 0;        
    DW702.ForwardGChGammaCorrectedValue175           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_175_UNNAMED44800;
    DW702.ForwardBChGammaCorrectedValue175           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_175_UNNAMED44800;

    DW703.Value                                      = 0;        
    DW703.ForwardPixelValue175                       = FORWARD_PIXEL_VALUE_175_UNNAMED44800;
    DW703.ForwardRChGammaCorrectedValue175           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_175_UNNAMED44800;

    DW704.Value                                      = 0;        
    DW704.InverseGChGammaCorrectedValue176           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_176_UNNAMED45056;
    DW704.InverseBChGammaCorrectedValue176           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_176_UNNAMED45056;

    DW705.Value                                      = 0;        
    DW705.InversePixelValue176                       = INVERSE_PIXEL_VALUE_176_UNNAMED45056;
    DW705.InverseRChGammaCorrectedValue176           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_176_UNNAMED45056;

    DW706.Value                                      = 0;        
    DW706.ForwardGChGammaCorrectedValue176           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_176_UNNAMED45056;
    DW706.ForwardBChGammaCorrectedValue176           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_176_UNNAMED45056;

    DW707.Value                                      = 0;        
    DW707.ForwardPixelValue176                       = FORWARD_PIXEL_VALUE_176_UNNAMED45056;
    DW707.ForwardRChGammaCorrectedValue176           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_176_UNNAMED45056;

    DW708.Value                                      = 0;        
    DW708.InverseGChGammaCorrectedValue177           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_177_UNNAMED45312;
    DW708.InverseBChGammaCorrectedValue177           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_177_UNNAMED45312;

    DW709.Value                                      = 0;        
    DW709.InversePixelValue177                       = INVERSE_PIXEL_VALUE_177_UNNAMED45312;
    DW709.InverseRChGammaCorrectedValue177           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_177_UNNAMED45312;

    DW710.Value                                      = 0;        
    DW710.ForwardGChGammaCorrectedValue177           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_177_UNNAMED45312;
    DW710.ForwardBChGammaCorrectedValue177           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_177_UNNAMED45312;

    DW711.Value                                      = 0;        
    DW711.ForwardPixelValue177                       = FORWARD_PIXEL_VALUE_177_UNNAMED45312;
    DW711.ForwardRChGammaCorrectedValue177           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_177_UNNAMED45312;

    DW712.Value                                      = 0;        
    DW712.InverseGChGammaCorrectedValue178           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_178_UNNAMED45568;
    DW712.InverseBChGammaCorrectedValue178           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_178_UNNAMED45568;

    DW713.Value                                      = 0;        
    DW713.InversePixelValue178                       = INVERSE_PIXEL_VALUE_178_UNNAMED45568;
    DW713.InverseRChGammaCorrectedValue178           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_178_UNNAMED45568;

    DW714.Value                                      = 0;        
    DW714.ForwardGChGammaCorrectedValue178           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_178_UNNAMED45568;
    DW714.ForwardBChGammaCorrectedValue178           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_178_UNNAMED45568;

    DW715.Value                                      = 0;        
    DW715.ForwardPixelValue178                       = FORWARD_PIXEL_VALUE_178_UNNAMED45568;
    DW715.ForwardRChGammaCorrectedValue178           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_178_UNNAMED45568;

    DW716.Value                                      = 0;        
    DW716.InverseGChGammaCorrectedValue179           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_179_UNNAMED45824;
    DW716.InverseBChGammaCorrectedValue179           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_179_UNNAMED45824;

    DW717.Value                                      = 0;        
    DW717.InversePixelValue179                       = INVERSE_PIXEL_VALUE_179_UNNAMED45824;
    DW717.InverseRChGammaCorrectedValue179           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_179_UNNAMED45824;

    DW718.Value                                      = 0;        
    DW718.ForwardGChGammaCorrectedValue179           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_179_UNNAMED45824;
    DW718.ForwardBChGammaCorrectedValue179           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_179_UNNAMED45824;

    DW719.Value                                      = 0;        
    DW719.ForwardPixelValue179                       = FORWARD_PIXEL_VALUE_179_UNNAMED45824;
    DW719.ForwardRChGammaCorrectedValue179           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_179_UNNAMED45824;

    DW720.Value                                      = 0;        
    DW720.InverseGChGammaCorrectedValue180           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_180_UNNAMED46080;
    DW720.InverseBChGammaCorrectedValue180           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_180_UNNAMED46080;

    DW721.Value                                      = 0;        
    DW721.InversePixelValue180                       = INVERSE_PIXEL_VALUE_180_UNNAMED46080;
    DW721.InverseRChGammaCorrectedValue180           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_180_UNNAMED46080;

    DW722.Value                                      = 0;        
    DW722.ForwardGChGammaCorrectedValue180           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_180_UNNAMED46080;
    DW722.ForwardBChGammaCorrectedValue180           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_180_UNNAMED46080;

    DW723.Value                                      = 0;        
    DW723.ForwardPixelValue180                       = FORWARD_PIXEL_VALUE_180_UNNAMED46080;
    DW723.ForwardRChGammaCorrectedValue180           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_180_UNNAMED46080;

    DW724.Value                                      = 0;        
    DW724.InverseGChGammaCorrectedValue181           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_181_UNNAMED46336;
    DW724.InverseBChGammaCorrectedValue181           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_181_UNNAMED46336;

    DW725.Value                                      = 0;        
    DW725.InversePixelValue181                       = INVERSE_PIXEL_VALUE_181_UNNAMED46336;
    DW725.InverseRChGammaCorrectedValue181           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_181_UNNAMED46336;

    DW726.Value                                      = 0;        
    DW726.ForwardGChGammaCorrectedValue181           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_181_UNNAMED46336;
    DW726.ForwardBChGammaCorrectedValue181           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_181_UNNAMED46336;

    DW727.Value                                      = 0;        
    DW727.ForwardPixelValue181                       = FORWARD_PIXEL_VALUE_181_UNNAMED46336;
    DW727.ForwardRChGammaCorrectedValue181           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_181_UNNAMED46336;

    DW728.Value                                      = 0;        
    DW728.InverseGChGammaCorrectedValue182           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_182_UNNAMED46592;
    DW728.InverseBChGammaCorrectedValue182           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_182_UNNAMED46592;

    DW729.Value                                      = 0;        
    DW729.InversePixelValue182                       = INVERSE_PIXEL_VALUE_182_UNNAMED46592;
    DW729.InverseRChGammaCorrectedValue182           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_182_UNNAMED46592;

    DW730.Value                                      = 0;        
    DW730.ForwardGChGammaCorrectedValue182           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_182_UNNAMED46592;
    DW730.ForwardBChGammaCorrectedValue182           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_182_UNNAMED46592;

    DW731.Value                                      = 0;        
    DW731.ForwardPixelValue182                       = FORWARD_PIXEL_VALUE_182_UNNAMED46592;
    DW731.ForwardRChGammaCorrectedValue182           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_182_UNNAMED46592;

    DW732.Value                                      = 0;        
    DW732.InverseGChGammaCorrectedValue183           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_183_UNNAMED46848;
    DW732.InverseBChGammaCorrectedValue183           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_183_UNNAMED46848;

    DW733.Value                                      = 0;        
    DW733.InversePixelValue183                       = INVERSE_PIXEL_VALUE_183_UNNAMED46848;
    DW733.InverseRChGammaCorrectedValue183           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_183_UNNAMED46848;

    DW734.Value                                      = 0;        
    DW734.ForwardGChGammaCorrectedValue183           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_183_UNNAMED46848;
    DW734.ForwardBChGammaCorrectedValue183           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_183_UNNAMED46848;

    DW735.Value                                      = 0;        
    DW735.ForwardPixelValue183                       = FORWARD_PIXEL_VALUE_183_UNNAMED46848;
    DW735.ForwardRChGammaCorrectedValue183           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_183_UNNAMED46848;

    DW736.Value                                      = 0;        
    DW736.InverseGChGammaCorrectedValue184           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_184_UNNAMED47104;
    DW736.InverseBChGammaCorrectedValue184           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_184_UNNAMED47104;

    DW737.Value                                      = 0;        
    DW737.InversePixelValue184                       = INVERSE_PIXEL_VALUE_184_UNNAMED47104;
    DW737.InverseRChGammaCorrectedValue184           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_184_UNNAMED47104;

    DW738.Value                                      = 0;        
    DW738.ForwardGChGammaCorrectedValue184           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_184_UNNAMED47104;
    DW738.ForwardBChGammaCorrectedValue184           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_184_UNNAMED47104;

    DW739.Value                                      = 0;        
    DW739.ForwardPixelValue184                       = FORWARD_PIXEL_VALUE_184_UNNAMED47104;
    DW739.ForwardRChGammaCorrectedValue184           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_184_UNNAMED47104;

    DW740.Value                                      = 0;        
    DW740.InverseGChGammaCorrectedValue185           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_185_UNNAMED47360;
    DW740.InverseBChGammaCorrectedValue185           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_185_UNNAMED47360;

    DW741.Value                                      = 0;        
    DW741.InversePixelValue185                       = INVERSE_PIXEL_VALUE_185_UNNAMED47360;
    DW741.InverseRChGammaCorrectedValue185           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_185_UNNAMED47360;

    DW742.Value                                      = 0;        
    DW742.ForwardGChGammaCorrectedValue185           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_185_UNNAMED47360;
    DW742.ForwardBChGammaCorrectedValue185           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_185_UNNAMED47360;

    DW743.Value                                      = 0;        
    DW743.ForwardPixelValue185                       = FORWARD_PIXEL_VALUE_185_UNNAMED47360;
    DW743.ForwardRChGammaCorrectedValue185           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_185_UNNAMED47360;

    DW744.Value                                      = 0;        
    DW744.InverseGChGammaCorrectedValue186           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_186_UNNAMED47616;
    DW744.InverseBChGammaCorrectedValue186           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_186_UNNAMED47616;

    DW745.Value                                      = 0;        
    DW745.InversePixelValue186                       = INVERSE_PIXEL_VALUE_186_UNNAMED47616;
    DW745.InverseRChGammaCorrectedValue186           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_186_UNNAMED47616;

    DW746.Value                                      = 0;        
    DW746.ForwardGChGammaCorrectedValue186           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_186_UNNAMED47616;
    DW746.ForwardBChGammaCorrectedValue186           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_186_UNNAMED47616;

    DW747.Value                                      = 0;        
    DW747.ForwardPixelValue186                       = FORWARD_PIXEL_VALUE_186_UNNAMED47616;
    DW747.ForwardRChGammaCorrectedValue186           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_186_UNNAMED47616;

    DW748.Value                                      = 0;        
    DW748.InverseGChGammaCorrectedValue187           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_187_UNNAMED47872;
    DW748.InverseBChGammaCorrectedValue187           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_187_UNNAMED47872;

    DW749.Value                                      = 0;        
    DW749.InversePixelValue187                       = INVERSE_PIXEL_VALUE_187_UNNAMED47872;
    DW749.InverseRChGammaCorrectedValue187           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_187_UNNAMED47872;

    DW750.Value                                      = 0;        
    DW750.ForwardGChGammaCorrectedValue187           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_187_UNNAMED47872;
    DW750.ForwardBChGammaCorrectedValue187           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_187_UNNAMED47872;

    DW751.Value                                      = 0;        
    DW751.ForwardPixelValue187                       = FORWARD_PIXEL_VALUE_187_UNNAMED47872;
    DW751.ForwardRChGammaCorrectedValue187           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_187_UNNAMED47872;

    DW752.Value                                      = 0;        
    DW752.InverseGChGammaCorrectedValue188           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_188_UNNAMED48128;
    DW752.InverseBChGammaCorrectedValue188           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_188_UNNAMED48128;

    DW753.Value                                      = 0;        
    DW753.InversePixelValue188                       = INVERSE_PIXEL_VALUE_188_UNNAMED48128;
    DW753.InverseRChGammaCorrectedValue188           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_188_UNNAMED48128;

    DW754.Value                                      = 0;        
    DW754.ForwardGChGammaCorrectedValue188           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_188_UNNAMED48128;
    DW754.ForwardBChGammaCorrectedValue188           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_188_UNNAMED48128;

    DW755.Value                                      = 0;        
    DW755.ForwardPixelValue188                       = FORWARD_PIXEL_VALUE_188_UNNAMED48128;
    DW755.ForwardRChGammaCorrectedValue188           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_188_UNNAMED48128;

    DW756.Value                                      = 0;        
    DW756.InverseGChGammaCorrectedValue189           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_189_UNNAMED48384;
    DW756.InverseBChGammaCorrectedValue189           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_189_UNNAMED48384;

    DW757.Value                                      = 0;        
    DW757.InversePixelValue189                       = INVERSE_PIXEL_VALUE_189_UNNAMED48384;
    DW757.InverseRChGammaCorrectedValue189           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_189_UNNAMED48384;

    DW758.Value                                      = 0;        
    DW758.ForwardGChGammaCorrectedValue189           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_189_UNNAMED48384;
    DW758.ForwardBChGammaCorrectedValue189           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_189_UNNAMED48384;

    DW759.Value                                      = 0;        
    DW759.ForwardPixelValue189                       = FORWARD_PIXEL_VALUE_189_UNNAMED48384;
    DW759.ForwardRChGammaCorrectedValue189           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_189_UNNAMED48384;

    DW760.Value                                      = 0;        
    DW760.InverseGChGammaCorrectedValue190           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_190_UNNAMED48640;
    DW760.InverseBChGammaCorrectedValue190           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_190_UNNAMED48640;

    DW761.Value                                      = 0;        
    DW761.InversePixelValue190                       = INVERSE_PIXEL_VALUE_190_UNNAMED48640;
    DW761.InverseRChGammaCorrectedValue190           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_190_UNNAMED48640;

    DW762.Value                                      = 0;        
    DW762.ForwardGChGammaCorrectedValue190           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_190_UNNAMED48640;
    DW762.ForwardBChGammaCorrectedValue190           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_190_UNNAMED48640;

    DW763.Value                                      = 0;        
    DW763.ForwardPixelValue190                       = FORWARD_PIXEL_VALUE_190_UNNAMED48640;
    DW763.ForwardRChGammaCorrectedValue190           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_190_UNNAMED48640;

    DW764.Value                                      = 0;        
    DW764.InverseGChGammaCorrectedValue191           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_191_UNNAMED48896;
    DW764.InverseBChGammaCorrectedValue191           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_191_UNNAMED48896;

    DW765.Value                                      = 0;        
    DW765.InversePixelValue191                       = INVERSE_PIXEL_VALUE_191_UNNAMED48896;
    DW765.InverseRChGammaCorrectedValue191           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_191_UNNAMED48896;

    DW766.Value                                      = 0;        
    DW766.ForwardGChGammaCorrectedValue191           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_191_UNNAMED48896;
    DW766.ForwardBChGammaCorrectedValue191           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_191_UNNAMED48896;

    DW767.Value                                      = 0;        
    DW767.ForwardPixelValue191                       = FORWARD_PIXEL_VALUE_191_UNNAMED48896;
    DW767.ForwardRChGammaCorrectedValue191           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_191_UNNAMED48896;

    DW768.Value                                      = 0;        
    DW768.InverseGChGammaCorrectedValue192           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_192_UNNAMED49152;
    DW768.InverseBChGammaCorrectedValue192           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_192_UNNAMED49152;

    DW769.Value                                      = 0;        
    DW769.InversePixelValue192                       = INVERSE_PIXEL_VALUE_192_UNNAMED49152;
    DW769.InverseRChGammaCorrectedValue192           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_192_UNNAMED49152;

    DW770.Value                                      = 0;        
    DW770.ForwardGChGammaCorrectedValue192           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_192_UNNAMED49152;
    DW770.ForwardBChGammaCorrectedValue192           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_192_UNNAMED49152;

    DW771.Value                                      = 0;        
    DW771.ForwardPixelValue192                       = FORWARD_PIXEL_VALUE_192_UNNAMED49152;
    DW771.ForwardRChGammaCorrectedValue192           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_192_UNNAMED49152;

    DW772.Value                                      = 0;        
    DW772.InverseGChGammaCorrectedValue193           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_193_UNNAMED49408;
    DW772.InverseBChGammaCorrectedValue193           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_193_UNNAMED49408;

    DW773.Value                                      = 0;        
    DW773.InversePixelValue193                       = INVERSE_PIXEL_VALUE_193_UNNAMED49408;
    DW773.InverseRChGammaCorrectedValue193           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_193_UNNAMED49408;

    DW774.Value                                      = 0;        
    DW774.ForwardGChGammaCorrectedValue193           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_193_UNNAMED49408;
    DW774.ForwardBChGammaCorrectedValue193           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_193_UNNAMED49408;

    DW775.Value                                      = 0;        
    DW775.ForwardPixelValue193                       = FORWARD_PIXEL_VALUE_193_UNNAMED49408;
    DW775.ForwardRChGammaCorrectedValue193           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_193_UNNAMED49408;

    DW776.Value                                      = 0;        
    DW776.InverseGChGammaCorrectedValue194           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_194_UNNAMED49664;
    DW776.InverseBChGammaCorrectedValue194           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_194_UNNAMED49664;

    DW777.Value                                      = 0;        
    DW777.InversePixelValue194                       = INVERSE_PIXEL_VALUE_194_UNNAMED49664;
    DW777.InverseRChGammaCorrectedValue194           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_194_UNNAMED49664;

    DW778.Value                                      = 0;        
    DW778.ForwardGChGammaCorrectedValue194           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_194_UNNAMED49664;
    DW778.ForwardBChGammaCorrectedValue194           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_194_UNNAMED49664;

    DW779.Value                                      = 0;        
    DW779.ForwardPixelValue194                       = FORWARD_PIXEL_VALUE_194_UNNAMED49664;
    DW779.ForwardRChGammaCorrectedValue194           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_194_UNNAMED49664;

    DW780.Value                                      = 0;        
    DW780.InverseGChGammaCorrectedValue195           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_195_UNNAMED49920;
    DW780.InverseBChGammaCorrectedValue195           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_195_UNNAMED49920;

    DW781.Value                                      = 0;        
    DW781.InversePixelValue195                       = INVERSE_PIXEL_VALUE_195_UNNAMED49920;
    DW781.InverseRChGammaCorrectedValue195           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_195_UNNAMED49920;

    DW782.Value                                      = 0;        
    DW782.ForwardGChGammaCorrectedValue195           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_195_UNNAMED49920;
    DW782.ForwardBChGammaCorrectedValue195           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_195_UNNAMED49920;

    DW783.Value                                      = 0;        
    DW783.ForwardPixelValue195                       = FORWARD_PIXEL_VALUE_195_UNNAMED49920;
    DW783.ForwardRChGammaCorrectedValue195           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_195_UNNAMED49920;

    DW784.Value                                      = 0;        
    DW784.InverseGChGammaCorrectedValue196           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_196_UNNAMED50176;
    DW784.InverseBChGammaCorrectedValue196           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_196_UNNAMED50176;

    DW785.Value                                      = 0;        
    DW785.InversePixelValue196                       = INVERSE_PIXEL_VALUE_196_UNNAMED50176;
    DW785.InverseRChGammaCorrectedValue196           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_196_UNNAMED50176;

    DW786.Value                                      = 0;        
    DW786.ForwardGChGammaCorrectedValue196           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_196_UNNAMED50176;
    DW786.ForwardBChGammaCorrectedValue196           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_196_UNNAMED50176;

    DW787.Value                                      = 0;        
    DW787.ForwardPixelValue196                       = FORWARD_PIXEL_VALUE_196_UNNAMED50176;
    DW787.ForwardRChGammaCorrectedValue196           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_196_UNNAMED50176;

    DW788.Value                                      = 0;        
    DW788.InverseGChGammaCorrectedValue197           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_197_UNNAMED50432;
    DW788.InverseBChGammaCorrectedValue197           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_197_UNNAMED50432;

    DW789.Value                                      = 0;        
    DW789.InversePixelValue197                       = INVERSE_PIXEL_VALUE_197_UNNAMED50432;
    DW789.InverseRChGammaCorrectedValue197           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_197_UNNAMED50432;

    DW790.Value                                      = 0;        
    DW790.ForwardGChGammaCorrectedValue197           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_197_UNNAMED50432;
    DW790.ForwardBChGammaCorrectedValue197           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_197_UNNAMED50432;

    DW791.Value                                      = 0;        
    DW791.ForwardPixelValue197                       = FORWARD_PIXEL_VALUE_197_UNNAMED50432;
    DW791.ForwardRChGammaCorrectedValue197           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_197_UNNAMED50432;

    DW792.Value                                      = 0;        
    DW792.InverseGChGammaCorrectedValue198           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_198_UNNAMED50688;
    DW792.InverseBChGammaCorrectedValue198           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_198_UNNAMED50688;

    DW793.Value                                      = 0;        
    DW793.InversePixelValue198                       = INVERSE_PIXEL_VALUE_198_UNNAMED50688;
    DW793.InverseRChGammaCorrectedValue198           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_198_UNNAMED50688;

    DW794.Value                                      = 0;        
    DW794.ForwardGChGammaCorrectedValue198           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_198_UNNAMED50688;
    DW794.ForwardBChGammaCorrectedValue198           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_198_UNNAMED50688;

    DW795.Value                                      = 0;        
    DW795.ForwardPixelValue198                       = FORWARD_PIXEL_VALUE_198_UNNAMED50688;
    DW795.ForwardRChGammaCorrectedValue198           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_198_UNNAMED50688;

    DW796.Value                                      = 0;        
    DW796.InverseGChGammaCorrectedValue199           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_199_UNNAMED50944;
    DW796.InverseBChGammaCorrectedValue199           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_199_UNNAMED50944;

    DW797.Value                                      = 0;        
    DW797.InversePixelValue199                       = INVERSE_PIXEL_VALUE_199_UNNAMED50944;
    DW797.InverseRChGammaCorrectedValue199           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_199_UNNAMED50944;

    DW798.Value                                      = 0;        
    DW798.ForwardGChGammaCorrectedValue199           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_199_UNNAMED50944;
    DW798.ForwardBChGammaCorrectedValue199           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_199_UNNAMED50944;

    DW799.Value                                      = 0;        
    DW799.ForwardPixelValue199                       = FORWARD_PIXEL_VALUE_199_UNNAMED50944;
    DW799.ForwardRChGammaCorrectedValue199           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_199_UNNAMED50944;

    DW800.Value                                      = 0;        
    DW800.InverseGChGammaCorrectedValue200           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_200_UNNAMED51200;
    DW800.InverseBChGammaCorrectedValue200           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_200_UNNAMED51200;

    DW801.Value                                      = 0;        
    DW801.InversePixelValue200                       = INVERSE_PIXEL_VALUE_200_UNNAMED51200;
    DW801.InverseRChGammaCorrectedValue200           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_200_UNNAMED51200;

    DW802.Value                                      = 0;        
    DW802.ForwardGChGammaCorrectedValue200           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_200_UNNAMED51200;
    DW802.ForwardBChGammaCorrectedValue200           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_200_UNNAMED51200;

    DW803.Value                                      = 0;        
    DW803.ForwardPixelValue200                       = FORWARD_PIXEL_VALUE_200_UNNAMED51200;
    DW803.ForwardRChGammaCorrectedValue200           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_200_UNNAMED51200;

    DW804.Value                                      = 0;        
    DW804.InverseGChGammaCorrectedValue201           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_201_UNNAMED51456;
    DW804.InverseBChGammaCorrectedValue201           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_201_UNNAMED51456;

    DW805.Value                                      = 0;        
    DW805.InversePixelValue201                       = INVERSE_PIXEL_VALUE_201_UNNAMED51456;
    DW805.InverseRChGammaCorrectedValue201           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_201_UNNAMED51456;

    DW806.Value                                      = 0;        
    DW806.ForwardGChGammaCorrectedValue201           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_201_UNNAMED51456;
    DW806.ForwardBChGammaCorrectedValue201           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_201_UNNAMED51456;

    DW807.Value                                      = 0;        
    DW807.ForwardPixelValue201                       = FORWARD_PIXEL_VALUE_201_UNNAMED51456;
    DW807.ForwardRChGammaCorrectedValue201           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_201_UNNAMED51456;

    DW808.Value                                      = 0;        
    DW808.InverseGChGammaCorrectedValue202           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_202_UNNAMED51712;
    DW808.InverseBChGammaCorrectedValue202           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_202_UNNAMED51712;

    DW809.Value                                      = 0;        
    DW809.InversePixelValue202                       = INVERSE_PIXEL_VALUE_202_UNNAMED51712;
    DW809.InverseRChGammaCorrectedValue202           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_202_UNNAMED51712;

    DW810.Value                                      = 0;        
    DW810.ForwardGChGammaCorrectedValue202           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_202_UNNAMED51712;
    DW810.ForwardBChGammaCorrectedValue202           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_202_UNNAMED51712;

    DW811.Value                                      = 0;        
    DW811.ForwardPixelValue202                       = FORWARD_PIXEL_VALUE_202_UNNAMED51712;
    DW811.ForwardRChGammaCorrectedValue202           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_202_UNNAMED51712;

    DW812.Value                                      = 0;        
    DW812.InverseGChGammaCorrectedValue203           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_203_UNNAMED51968;
    DW812.InverseBChGammaCorrectedValue203           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_203_UNNAMED51968;

    DW813.Value                                      = 0;        
    DW813.InversePixelValue203                       = INVERSE_PIXEL_VALUE_203_UNNAMED51968;
    DW813.InverseRChGammaCorrectedValue203           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_203_UNNAMED51968;

    DW814.Value                                      = 0;        
    DW814.ForwardGChGammaCorrectedValue203           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_203_UNNAMED51968;
    DW814.ForwardBChGammaCorrectedValue203           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_203_UNNAMED51968;

    DW815.Value                                      = 0;        
    DW815.ForwardPixelValue203                       = FORWARD_PIXEL_VALUE_203_UNNAMED51968;
    DW815.ForwardRChGammaCorrectedValue203           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_203_UNNAMED51968;

    DW816.Value                                      = 0;        
    DW816.InverseGChGammaCorrectedValue204           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_204_UNNAMED52224;
    DW816.InverseBChGammaCorrectedValue204           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_204_UNNAMED52224;

    DW817.Value                                      = 0;        
    DW817.InversePixelValue204                       = INVERSE_PIXEL_VALUE_204_UNNAMED52224;
    DW817.InverseRChGammaCorrectedValue204           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_204_UNNAMED52224;

    DW818.Value                                      = 0;        
    DW818.ForwardGChGammaCorrectedValue204           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_204_UNNAMED52224;
    DW818.ForwardBChGammaCorrectedValue204           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_204_UNNAMED52224;

    DW819.Value                                      = 0;        
    DW819.ForwardPixelValue204                       = FORWARD_PIXEL_VALUE_204_UNNAMED52224;
    DW819.ForwardRChGammaCorrectedValue204           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_204_UNNAMED52224;

    DW820.Value                                      = 0;        
    DW820.InverseGChGammaCorrectedValue205           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_205_UNNAMED52480;
    DW820.InverseBChGammaCorrectedValue205           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_205_UNNAMED52480;

    DW821.Value                                      = 0;        
    DW821.InversePixelValue205                       = INVERSE_PIXEL_VALUE_205_UNNAMED52480;
    DW821.InverseRChGammaCorrectedValue205           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_205_UNNAMED52480;

    DW822.Value                                      = 0;        
    DW822.ForwardGChGammaCorrectedValue205           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_205_UNNAMED52480;
    DW822.ForwardBChGammaCorrectedValue205           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_205_UNNAMED52480;

    DW823.Value                                      = 0;        
    DW823.ForwardPixelValue205                       = FORWARD_PIXEL_VALUE_205_UNNAMED52480;
    DW823.ForwardRChGammaCorrectedValue205           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_205_UNNAMED52480;

    DW824.Value                                      = 0;        
    DW824.InverseGChGammaCorrectedValue206           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_206_UNNAMED52736;
    DW824.InverseBChGammaCorrectedValue206           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_206_UNNAMED52736;

    DW825.Value                                      = 0;        
    DW825.InversePixelValue206                       = INVERSE_PIXEL_VALUE_206_UNNAMED52736;
    DW825.InverseRChGammaCorrectedValue206           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_206_UNNAMED52736;

    DW826.Value                                      = 0;        
    DW826.ForwardGChGammaCorrectedValue206           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_206_UNNAMED52736;
    DW826.ForwardBChGammaCorrectedValue206           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_206_UNNAMED52736;

    DW827.Value                                      = 0;        
    DW827.ForwardPixelValue206                       = FORWARD_PIXEL_VALUE_206_UNNAMED52736;
    DW827.ForwardRChGammaCorrectedValue206           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_206_UNNAMED52736;

    DW828.Value                                      = 0;        
    DW828.InverseGChGammaCorrectedValue207           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_207_UNNAMED52992;
    DW828.InverseBChGammaCorrectedValue207           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_207_UNNAMED52992;

    DW829.Value                                      = 0;        
    DW829.InversePixelValue207                       = INVERSE_PIXEL_VALUE_207_UNNAMED52992;
    DW829.InverseRChGammaCorrectedValue207           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_207_UNNAMED52992;

    DW830.Value                                      = 0;        
    DW830.ForwardGChGammaCorrectedValue207           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_207_UNNAMED52992;
    DW830.ForwardBChGammaCorrectedValue207           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_207_UNNAMED52992;

    DW831.Value                                      = 0;        
    DW831.ForwardPixelValue207                       = FORWARD_PIXEL_VALUE_207_UNNAMED52992;
    DW831.ForwardRChGammaCorrectedValue207           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_207_UNNAMED52992;

    DW832.Value                                      = 0;        
    DW832.InverseGChGammaCorrectedValue208           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_208_UNNAMED53248;
    DW832.InverseBChGammaCorrectedValue208           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_208_UNNAMED53248;

    DW833.Value                                      = 0;        
    DW833.InversePixelValue208                       = INVERSE_PIXEL_VALUE_208_UNNAMED53248;
    DW833.InverseRChGammaCorrectedValue208           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_208_UNNAMED53248;

    DW834.Value                                      = 0;        
    DW834.ForwardGChGammaCorrectedValue208           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_208_UNNAMED53248;
    DW834.ForwardBChGammaCorrectedValue208           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_208_UNNAMED53248;

    DW835.Value                                      = 0;        
    DW835.ForwardPixelValue208                       = FORWARD_PIXEL_VALUE_208_UNNAMED53248;
    DW835.ForwardRChGammaCorrectedValue208           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_208_UNNAMED53248;

    DW836.Value                                      = 0;        
    DW836.InverseGChGammaCorrectedValue209           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_209_UNNAMED53504;
    DW836.InverseBChGammaCorrectedValue209           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_209_UNNAMED53504;

    DW837.Value                                      = 0;        
    DW837.InversePixelValue209                       = INVERSE_PIXEL_VALUE_209_UNNAMED53504;
    DW837.InverseRChGammaCorrectedValue209           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_209_UNNAMED53504;

    DW838.Value                                      = 0;        
    DW838.ForwardGChGammaCorrectedValue209           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_209_UNNAMED53504;
    DW838.ForwardBChGammaCorrectedValue209           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_209_UNNAMED53504;

    DW839.Value                                      = 0;        
    DW839.ForwardPixelValue209                       = FORWARD_PIXEL_VALUE_209_UNNAMED53504;
    DW839.ForwardRChGammaCorrectedValue209           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_209_UNNAMED53504;

    DW840.Value                                      = 0;        
    DW840.InverseGChGammaCorrectedValue210           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_210_UNNAMED53760;
    DW840.InverseBChGammaCorrectedValue210           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_210_UNNAMED53760;

    DW841.Value                                      = 0;        
    DW841.InversePixelValue210                       = INVERSE_PIXEL_VALUE_210_UNNAMED53760;
    DW841.InverseRChGammaCorrectedValue210           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_210_UNNAMED53760;

    DW842.Value                                      = 0;        
    DW842.ForwardGChGammaCorrectedValue210           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_210_UNNAMED53760;
    DW842.ForwardBChGammaCorrectedValue210           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_210_UNNAMED53760;

    DW843.Value                                      = 0;        
    DW843.ForwardPixelValue210                       = FORWARD_PIXEL_VALUE_210_UNNAMED53760;
    DW843.ForwardRChGammaCorrectedValue210           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_210_UNNAMED53760;

    DW844.Value                                      = 0;        
    DW844.InverseGChGammaCorrectedValue211           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_211_UNNAMED54016;
    DW844.InverseBChGammaCorrectedValue211           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_211_UNNAMED54016;

    DW845.Value                                      = 0;        
    DW845.InversePixelValue211                       = INVERSE_PIXEL_VALUE_211_UNNAMED54016;
    DW845.InverseRChGammaCorrectedValue211           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_211_UNNAMED54016;

    DW846.Value                                      = 0;        
    DW846.ForwardGChGammaCorrectedValue211           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_211_UNNAMED54016;
    DW846.ForwardBChGammaCorrectedValue211           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_211_UNNAMED54016;

    DW847.Value                                      = 0;        
    DW847.ForwardPixelValue211                       = FORWARD_PIXEL_VALUE_211_UNNAMED54016;
    DW847.ForwardRChGammaCorrectedValue211           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_211_UNNAMED54016;

    DW848.Value                                      = 0;        
    DW848.InverseGChGammaCorrectedValue212           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_212_UNNAMED54272;
    DW848.InverseBChGammaCorrectedValue212           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_212_UNNAMED54272;

    DW849.Value                                      = 0;        
    DW849.InversePixelValue212                       = INVERSE_PIXEL_VALUE_212_UNNAMED54272;
    DW849.InverseRChGammaCorrectedValue212           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_212_UNNAMED54272;

    DW850.Value                                      = 0;        
    DW850.ForwardGChGammaCorrectedValue212           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_212_UNNAMED54272;
    DW850.ForwardBChGammaCorrectedValue212           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_212_UNNAMED54272;

    DW851.Value                                      = 0;        
    DW851.ForwardPixelValue212                       = FORWARD_PIXEL_VALUE_212_UNNAMED54272;
    DW851.ForwardRChGammaCorrectedValue212           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_212_UNNAMED54272;

    DW852.Value                                      = 0;        
    DW852.InverseGChGammaCorrectedValue213           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_213_UNNAMED54528;
    DW852.InverseBChGammaCorrectedValue213           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_213_UNNAMED54528;

    DW853.Value                                      = 0;        
    DW853.InversePixelValue213                       = INVERSE_PIXEL_VALUE_213_UNNAMED54528;
    DW853.InverseRChGammaCorrectedValue213           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_213_UNNAMED54528;

    DW854.Value                                      = 0;        
    DW854.ForwardGChGammaCorrectedValue213           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_213_UNNAMED54528;
    DW854.ForwardBChGammaCorrectedValue213           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_213_UNNAMED54528;

    DW855.Value                                      = 0;        
    DW855.ForwardPixelValue213                       = FORWARD_PIXEL_VALUE_213_UNNAMED54528;
    DW855.ForwardRChGammaCorrectedValue213           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_213_UNNAMED54528;

    DW856.Value                                      = 0;        
    DW856.InverseGChGammaCorrectedValue214           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_214_UNNAMED54784;
    DW856.InverseBChGammaCorrectedValue214           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_214_UNNAMED54784;

    DW857.Value                                      = 0;        
    DW857.InversePixelValue214                       = INVERSE_PIXEL_VALUE_214_UNNAMED54784;
    DW857.InverseRChGammaCorrectedValue214           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_214_UNNAMED54784;

    DW858.Value                                      = 0;        
    DW858.ForwardGChGammaCorrectedValue214           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_214_UNNAMED54784;
    DW858.ForwardBChGammaCorrectedValue214           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_214_UNNAMED54784;

    DW859.Value                                      = 0;        
    DW859.ForwardPixelValue214                       = FORWARD_PIXEL_VALUE_214_UNNAMED54784;
    DW859.ForwardRChGammaCorrectedValue214           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_214_UNNAMED54784;

    DW860.Value                                      = 0;        
    DW860.InverseGChGammaCorrectedValue215           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_215_UNNAMED55040;
    DW860.InverseBChGammaCorrectedValue215           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_215_UNNAMED55040;

    DW861.Value                                      = 0;        
    DW861.InversePixelValue215                       = INVERSE_PIXEL_VALUE_215_UNNAMED55040;
    DW861.InverseRChGammaCorrectedValue215           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_215_UNNAMED55040;

    DW862.Value                                      = 0;        
    DW862.ForwardGChGammaCorrectedValue215           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_215_UNNAMED55040;
    DW862.ForwardBChGammaCorrectedValue215           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_215_UNNAMED55040;

    DW863.Value                                      = 0;        
    DW863.ForwardPixelValue215                       = FORWARD_PIXEL_VALUE_215_UNNAMED55040;
    DW863.ForwardRChGammaCorrectedValue215           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_215_UNNAMED55040;

    DW864.Value                                      = 0;        
    DW864.InverseGChGammaCorrectedValue216           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_216_UNNAMED55296;
    DW864.InverseBChGammaCorrectedValue216           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_216_UNNAMED55296;

    DW865.Value                                      = 0;        
    DW865.InversePixelValue216                       = INVERSE_PIXEL_VALUE_216_UNNAMED55296;
    DW865.InverseRChGammaCorrectedValue216           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_216_UNNAMED55296;

    DW866.Value                                      = 0;        
    DW866.ForwardGChGammaCorrectedValue216           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_216_UNNAMED55296;
    DW866.ForwardBChGammaCorrectedValue216           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_216_UNNAMED55296;

    DW867.Value                                      = 0;        
    DW867.ForwardPixelValue216                       = FORWARD_PIXEL_VALUE_216_UNNAMED55296;
    DW867.ForwardRChGammaCorrectedValue216           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_216_UNNAMED55296;

    DW868.Value                                      = 0;        
    DW868.InverseGChGammaCorrectedValue217           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_217_UNNAMED55552;
    DW868.InverseBChGammaCorrectedValue217           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_217_UNNAMED55552;

    DW869.Value                                      = 0;        
    DW869.InversePixelValue217                       = INVERSE_PIXEL_VALUE_217_UNNAMED55552;
    DW869.InverseRChGammaCorrectedValue217           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_217_UNNAMED55552;

    DW870.Value                                      = 0;        
    DW870.ForwardGChGammaCorrectedValue217           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_217_UNNAMED55552;
    DW870.ForwardBChGammaCorrectedValue217           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_217_UNNAMED55552;

    DW871.Value                                      = 0;        
    DW871.ForwardPixelValue217                       = FORWARD_PIXEL_VALUE_217_UNNAMED55552;
    DW871.ForwardRChGammaCorrectedValue217           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_217_UNNAMED55552;

    DW872.Value                                      = 0;        
    DW872.InverseGChGammaCorrectedValue218           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_218_UNNAMED55808;
    DW872.InverseBChGammaCorrectedValue218           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_218_UNNAMED55808;

    DW873.Value                                      = 0;        
    DW873.InversePixelValue218                       = INVERSE_PIXEL_VALUE_218_UNNAMED55808;
    DW873.InverseRChGammaCorrectedValue218           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_218_UNNAMED55808;

    DW874.Value                                      = 0;        
    DW874.ForwardGChGammaCorrectedValue218           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_218_UNNAMED55808;
    DW874.ForwardBChGammaCorrectedValue218           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_218_UNNAMED55808;

    DW875.Value                                      = 0;        
    DW875.ForwardPixelValue218                       = FORWARD_PIXEL_VALUE_218_UNNAMED55808;
    DW875.ForwardRChGammaCorrectedValue218           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_218_UNNAMED55808;

    DW876.Value                                      = 0;        
    DW876.InverseGChGammaCorrectedValue219           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_219_UNNAMED56064;
    DW876.InverseBChGammaCorrectedValue219           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_219_UNNAMED56064;

    DW877.Value                                      = 0;        
    DW877.InversePixelValue219                       = INVERSE_PIXEL_VALUE_219_UNNAMED56064;
    DW877.InverseRChGammaCorrectedValue219           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_219_UNNAMED56064;

    DW878.Value                                      = 0;        
    DW878.ForwardGChGammaCorrectedValue219           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_219_UNNAMED56064;
    DW878.ForwardBChGammaCorrectedValue219           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_219_UNNAMED56064;

    DW879.Value                                      = 0;        
    DW879.ForwardPixelValue219                       = FORWARD_PIXEL_VALUE_219_UNNAMED56064;
    DW879.ForwardRChGammaCorrectedValue219           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_219_UNNAMED56064;

    DW880.Value                                      = 0;        
    DW880.InverseGChGammaCorrectedValue220           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_220_UNNAMED56320;
    DW880.InverseBChGammaCorrectedValue220           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_220_UNNAMED56320;

    DW881.Value                                      = 0;        
    DW881.InversePixelValue220                       = INVERSE_PIXEL_VALUE_220_UNNAMED56320;
    DW881.InverseRChGammaCorrectedValue220           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_220_UNNAMED56320;

    DW882.Value                                      = 0;        
    DW882.ForwardGChGammaCorrectedValue220           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_220_UNNAMED56320;
    DW882.ForwardBChGammaCorrectedValue220           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_220_UNNAMED56320;

    DW883.Value                                      = 0;        
    DW883.ForwardPixelValue220                       = FORWARD_PIXEL_VALUE_220_UNNAMED56320;
    DW883.ForwardRChGammaCorrectedValue220           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_220_UNNAMED56320;

    DW884.Value                                      = 0;        
    DW884.InverseGChGammaCorrectedValue221           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_221_UNNAMED56576;
    DW884.InverseBChGammaCorrectedValue221           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_221_UNNAMED56576;

    DW885.Value                                      = 0;        
    DW885.InversePixelValue221                       = INVERSE_PIXEL_VALUE_221_UNNAMED56576;
    DW885.InverseRChGammaCorrectedValue221           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_221_UNNAMED56576;

    DW886.Value                                      = 0;        
    DW886.ForwardGChGammaCorrectedValue221           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_221_UNNAMED56576;
    DW886.ForwardBChGammaCorrectedValue221           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_221_UNNAMED56576;

    DW887.Value                                      = 0;        
    DW887.ForwardPixelValue221                       = FORWARD_PIXEL_VALUE_221_UNNAMED56576;
    DW887.ForwardRChGammaCorrectedValue221           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_221_UNNAMED56576;

    DW888.Value                                      = 0;        
    DW888.InverseGChGammaCorrectedValue222           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_222_UNNAMED56832;
    DW888.InverseBChGammaCorrectedValue222           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_222_UNNAMED56832;

    DW889.Value                                      = 0;        
    DW889.InversePixelValue222                       = INVERSE_PIXEL_VALUE_222_UNNAMED56832;
    DW889.InverseRChGammaCorrectedValue222           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_222_UNNAMED56832;

    DW890.Value                                      = 0;        
    DW890.ForwardGChGammaCorrectedValue222           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_222_UNNAMED56832;
    DW890.ForwardBChGammaCorrectedValue222           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_222_UNNAMED56832;

    DW891.Value                                      = 0;        
    DW891.ForwardPixelValue222                       = FORWARD_PIXEL_VALUE_222_UNNAMED56832;
    DW891.ForwardRChGammaCorrectedValue222           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_222_UNNAMED56832;

    DW892.Value                                      = 0;        
    DW892.InverseGChGammaCorrectedValue223           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_223_UNNAMED57088;
    DW892.InverseBChGammaCorrectedValue223           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_223_UNNAMED57088;

    DW893.Value                                      = 0;        
    DW893.InversePixelValue223                       = INVERSE_PIXEL_VALUE_223_UNNAMED57088;
    DW893.InverseRChGammaCorrectedValue223           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_223_UNNAMED57088;

    DW894.Value                                      = 0;        
    DW894.ForwardGChGammaCorrectedValue223           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_223_UNNAMED57088;
    DW894.ForwardBChGammaCorrectedValue223           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_223_UNNAMED57088;

    DW895.Value                                      = 0;        
    DW895.ForwardPixelValue223                       = FORWARD_PIXEL_VALUE_223_UNNAMED57088;
    DW895.ForwardRChGammaCorrectedValue223           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_223_UNNAMED57088;

    DW896.Value                                      = 0;        
    DW896.InverseGChGammaCorrectedValue224           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_224_UNNAMED57344;
    DW896.InverseBChGammaCorrectedValue224           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_224_UNNAMED57344;

    DW897.Value                                      = 0;        
    DW897.InversePixelValue224                       = INVERSE_PIXEL_VALUE_224_UNNAMED57344;
    DW897.InverseRChGammaCorrectedValue224           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_224_UNNAMED57344;

    DW898.Value                                      = 0;        
    DW898.ForwardGChGammaCorrectedValue224           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_224_UNNAMED57344;
    DW898.ForwardBChGammaCorrectedValue224           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_224_UNNAMED57344;

    DW899.Value                                      = 0;        
    DW899.ForwardPixelValue224                       = FORWARD_PIXEL_VALUE_224_UNNAMED57344;
    DW899.ForwardRChGammaCorrectedValue224           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_224_UNNAMED57344;

    DW900.Value                                      = 0;        
    DW900.InverseGChGammaCorrectedValue225           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_225_UNNAMED57600;
    DW900.InverseBChGammaCorrectedValue225           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_225_UNNAMED57600;

    DW901.Value                                      = 0;        
    DW901.InversePixelValue225                       = INVERSE_PIXEL_VALUE_225_UNNAMED57600;
    DW901.InverseRChGammaCorrectedValue225           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_225_UNNAMED57600;

    DW902.Value                                      = 0;        
    DW902.ForwardGChGammaCorrectedValue225           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_225_UNNAMED57600;
    DW902.ForwardBChGammaCorrectedValue225           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_225_UNNAMED57600;

    DW903.Value                                      = 0;        
    DW903.ForwardPixelValue225                       = FORWARD_PIXEL_VALUE_225_UNNAMED57600;
    DW903.ForwardRChGammaCorrectedValue225           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_225_UNNAMED57600;

    DW904.Value                                      = 0;        
    DW904.InverseGChGammaCorrectedValue226           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_226_UNNAMED57856;
    DW904.InverseBChGammaCorrectedValue226           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_226_UNNAMED57856;

    DW905.Value                                      = 0;        
    DW905.InversePixelValue226                       = INVERSE_PIXEL_VALUE_226_UNNAMED57856;
    DW905.InverseRChGammaCorrectedValue226           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_226_UNNAMED57856;

    DW906.Value                                      = 0;        
    DW906.ForwardGChGammaCorrectedValue226           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_226_UNNAMED57856;
    DW906.ForwardBChGammaCorrectedValue226           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_226_UNNAMED57856;

    DW907.Value                                      = 0;        
    DW907.ForwardPixelValue226                       = FORWARD_PIXEL_VALUE_226_UNNAMED57856;
    DW907.ForwardRChGammaCorrectedValue226           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_226_UNNAMED57856;

    DW908.Value                                      = 0;        
    DW908.InverseGChGammaCorrectedValue227           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_227_UNNAMED58112;
    DW908.InverseBChGammaCorrectedValue227           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_227_UNNAMED58112;

    DW909.Value                                      = 0;        
    DW909.InversePixelValue227                       = INVERSE_PIXEL_VALUE_227_UNNAMED58112;
    DW909.InverseRChGammaCorrectedValue227           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_227_UNNAMED58112;

    DW910.Value                                      = 0;        
    DW910.ForwardGChGammaCorrectedValue227           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_227_UNNAMED58112;
    DW910.ForwardBChGammaCorrectedValue227           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_227_UNNAMED58112;

    DW911.Value                                      = 0;        
    DW911.ForwardPixelValue227                       = FORWARD_PIXEL_VALUE_227_UNNAMED58112;
    DW911.ForwardRChGammaCorrectedValue227           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_227_UNNAMED58112;

    DW912.Value                                      = 0;        
    DW912.InverseGChGammaCorrectedValue228           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_228_UNNAMED58368;
    DW912.InverseBChGammaCorrectedValue228           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_228_UNNAMED58368;

    DW913.Value                                      = 0;        
    DW913.InversePixelValue228                       = INVERSE_PIXEL_VALUE_228_UNNAMED58368;
    DW913.InverseRChGammaCorrectedValue228           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_228_UNNAMED58368;

    DW914.Value                                      = 0;        
    DW914.ForwardGChGammaCorrectedValue228           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_228_UNNAMED58368;
    DW914.ForwardBChGammaCorrectedValue228           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_228_UNNAMED58368;

    DW915.Value                                      = 0;        
    DW915.ForwardPixelValue228                       = FORWARD_PIXEL_VALUE_228_UNNAMED58368;
    DW915.ForwardRChGammaCorrectedValue228           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_228_UNNAMED58368;

    DW916.Value                                      = 0;        
    DW916.InverseGChGammaCorrectedValue229           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_229_UNNAMED58624;
    DW916.InverseBChGammaCorrectedValue229           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_229_UNNAMED58624;

    DW917.Value                                      = 0;        
    DW917.InversePixelValue229                       = INVERSE_PIXEL_VALUE_229_UNNAMED58624;
    DW917.InverseRChGammaCorrectedValue229           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_229_UNNAMED58624;

    DW918.Value                                      = 0;        
    DW918.ForwardGChGammaCorrectedValue229           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_229_UNNAMED58624;
    DW918.ForwardBChGammaCorrectedValue229           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_229_UNNAMED58624;

    DW919.Value                                      = 0;        
    DW919.ForwardPixelValue229                       = FORWARD_PIXEL_VALUE_229_UNNAMED58624;
    DW919.ForwardRChGammaCorrectedValue229           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_229_UNNAMED58624;

    DW920.Value                                      = 0;        
    DW920.InverseGChGammaCorrectedValue230           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_230_UNNAMED58880;
    DW920.InverseBChGammaCorrectedValue230           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_230_UNNAMED58880;

    DW921.Value                                      = 0;        
    DW921.InversePixelValue230                       = INVERSE_PIXEL_VALUE_230_UNNAMED58880;
    DW921.InverseRChGammaCorrectedValue230           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_230_UNNAMED58880;

    DW922.Value                                      = 0;        
    DW922.ForwardGChGammaCorrectedValue230           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_230_UNNAMED58880;
    DW922.ForwardBChGammaCorrectedValue230           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_230_UNNAMED58880;

    DW923.Value                                      = 0;        
    DW923.ForwardPixelValue230                       = FORWARD_PIXEL_VALUE_230_UNNAMED58880;
    DW923.ForwardRChGammaCorrectedValue230           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_230_UNNAMED58880;

    DW924.Value                                      = 0;        
    DW924.InverseGChGammaCorrectedValue231           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_231_UNNAMED59136;
    DW924.InverseBChGammaCorrectedValue231           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_231_UNNAMED59136;

    DW925.Value                                      = 0;        
    DW925.InversePixelValue231                       = INVERSE_PIXEL_VALUE_231_UNNAMED59136;
    DW925.InverseRChGammaCorrectedValue231           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_231_UNNAMED59136;

    DW926.Value                                      = 0;        
    DW926.ForwardGChGammaCorrectedValue231           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_231_UNNAMED59136;
    DW926.ForwardBChGammaCorrectedValue231           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_231_UNNAMED59136;

    DW927.Value                                      = 0;        
    DW927.ForwardPixelValue231                       = FORWARD_PIXEL_VALUE_231_UNNAMED59136;
    DW927.ForwardRChGammaCorrectedValue231           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_231_UNNAMED59136;

    DW928.Value                                      = 0;        
    DW928.InverseGChGammaCorrectedValue232           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_232_UNNAMED59392;
    DW928.InverseBChGammaCorrectedValue232           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_232_UNNAMED59392;

    DW929.Value                                      = 0;        
    DW929.InversePixelValue232                       = INVERSE_PIXEL_VALUE_232_UNNAMED59392;
    DW929.InverseRChGammaCorrectedValue232           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_232_UNNAMED59392;

    DW930.Value                                      = 0;        
    DW930.ForwardGChGammaCorrectedValue232           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_232_UNNAMED59392;
    DW930.ForwardBChGammaCorrectedValue232           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_232_UNNAMED59392;

    DW931.Value                                      = 0;        
    DW931.ForwardPixelValue232                       = FORWARD_PIXEL_VALUE_232_UNNAMED59392;
    DW931.ForwardRChGammaCorrectedValue232           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_232_UNNAMED59392;

    DW932.Value                                      = 0;        
    DW932.InverseGChGammaCorrectedValue233           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_233_UNNAMED59648;
    DW932.InverseBChGammaCorrectedValue233           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_233_UNNAMED59648;

    DW933.Value                                      = 0;        
    DW933.InversePixelValue233                       = INVERSE_PIXEL_VALUE_233_UNNAMED59648;
    DW933.InverseRChGammaCorrectedValue233           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_233_UNNAMED59648;

    DW934.Value                                      = 0;        
    DW934.ForwardGChGammaCorrectedValue233           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_233_UNNAMED59648;
    DW934.ForwardBChGammaCorrectedValue233           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_233_UNNAMED59648;

    DW935.Value                                      = 0;        
    DW935.ForwardPixelValue233                       = FORWARD_PIXEL_VALUE_233_UNNAMED59648;
    DW935.ForwardRChGammaCorrectedValue233           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_233_UNNAMED59648;

    DW936.Value                                      = 0;        
    DW936.InverseGChGammaCorrectedValue234           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_234_UNNAMED59904;
    DW936.InverseBChGammaCorrectedValue234           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_234_UNNAMED59904;

    DW937.Value                                      = 0;        
    DW937.InversePixelValue234                       = INVERSE_PIXEL_VALUE_234_UNNAMED59904;
    DW937.InverseRChGammaCorrectedValue234           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_234_UNNAMED59904;

    DW938.Value                                      = 0;        
    DW938.ForwardGChGammaCorrectedValue234           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_234_UNNAMED59904;
    DW938.ForwardBChGammaCorrectedValue234           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_234_UNNAMED59904;

    DW939.Value                                      = 0;        
    DW939.ForwardPixelValue234                       = FORWARD_PIXEL_VALUE_234_UNNAMED59904;
    DW939.ForwardRChGammaCorrectedValue234           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_234_UNNAMED59904;

    DW940.Value                                      = 0;        
    DW940.InverseGChGammaCorrectedValue235           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_235_UNNAMED60160;
    DW940.InverseBChGammaCorrectedValue235           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_235_UNNAMED60160;

    DW941.Value                                      = 0;        
    DW941.InversePixelValue235                       = INVERSE_PIXEL_VALUE_235_UNNAMED60160;
    DW941.InverseRChGammaCorrectedValue235           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_235_UNNAMED60160;

    DW942.Value                                      = 0;        
    DW942.ForwardGChGammaCorrectedValue235           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_235_UNNAMED60160;
    DW942.ForwardBChGammaCorrectedValue235           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_235_UNNAMED60160;

    DW943.Value                                      = 0;        
    DW943.ForwardPixelValue235                       = FORWARD_PIXEL_VALUE_235_UNNAMED60160;
    DW943.ForwardRChGammaCorrectedValue235           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_235_UNNAMED60160;

    DW944.Value                                      = 0;        
    DW944.InverseGChGammaCorrectedValue236           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_236_UNNAMED60416;
    DW944.InverseBChGammaCorrectedValue236           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_236_UNNAMED60416;

    DW945.Value                                      = 0;        
    DW945.InversePixelValue236                       = INVERSE_PIXEL_VALUE_236_UNNAMED60416;
    DW945.InverseRChGammaCorrectedValue236           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_236_UNNAMED60416;

    DW946.Value                                      = 0;        
    DW946.ForwardGChGammaCorrectedValue236           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_236_UNNAMED60416;
    DW946.ForwardBChGammaCorrectedValue236           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_236_UNNAMED60416;

    DW947.Value                                      = 0;        
    DW947.ForwardPixelValue236                       = FORWARD_PIXEL_VALUE_236_UNNAMED60416;
    DW947.ForwardRChGammaCorrectedValue236           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_236_UNNAMED60416;

    DW948.Value                                      = 0;        
    DW948.InverseGChGammaCorrectedValue237           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_237_UNNAMED60672;
    DW948.InverseBChGammaCorrectedValue237           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_237_UNNAMED60672;

    DW949.Value                                      = 0;        
    DW949.InversePixelValue237                       = INVERSE_PIXEL_VALUE_237_UNNAMED60672;
    DW949.InverseRChGammaCorrectedValue237           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_237_UNNAMED60672;

    DW950.Value                                      = 0;        
    DW950.ForwardGChGammaCorrectedValue237           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_237_UNNAMED60672;
    DW950.ForwardBChGammaCorrectedValue237           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_237_UNNAMED60672;

    DW951.Value                                      = 0;        
    DW951.ForwardPixelValue237                       = FORWARD_PIXEL_VALUE_237_UNNAMED60672;
    DW951.ForwardRChGammaCorrectedValue237           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_237_UNNAMED60672;

    DW952.Value                                      = 0;        
    DW952.InverseGChGammaCorrectedValue238           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_238_UNNAMED60928;
    DW952.InverseBChGammaCorrectedValue238           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_238_UNNAMED60928;

    DW953.Value                                      = 0;        
    DW953.InversePixelValue238                       = INVERSE_PIXEL_VALUE_238_UNNAMED60928;
    DW953.InverseRChGammaCorrectedValue238           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_238_UNNAMED60928;

    DW954.Value                                      = 0;        
    DW954.ForwardGChGammaCorrectedValue238           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_238_UNNAMED60928;
    DW954.ForwardBChGammaCorrectedValue238           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_238_UNNAMED60928;

    DW955.Value                                      = 0;        
    DW955.ForwardPixelValue238                       = FORWARD_PIXEL_VALUE_238_UNNAMED60928;
    DW955.ForwardRChGammaCorrectedValue238           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_238_UNNAMED60928;

    DW956.Value                                      = 0;        
    DW956.InverseGChGammaCorrectedValue239           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_239_UNNAMED61184;
    DW956.InverseBChGammaCorrectedValue239           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_239_UNNAMED61184;

    DW957.Value                                      = 0;        
    DW957.InversePixelValue239                       = INVERSE_PIXEL_VALUE_239_UNNAMED61184;
    DW957.InverseRChGammaCorrectedValue239           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_239_UNNAMED61184;

    DW958.Value                                      = 0;        
    DW958.ForwardGChGammaCorrectedValue239           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_239_UNNAMED61184;
    DW958.ForwardBChGammaCorrectedValue239           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_239_UNNAMED61184;

    DW959.Value                                      = 0;        
    DW959.ForwardPixelValue239                       = FORWARD_PIXEL_VALUE_239_UNNAMED61184;
    DW959.ForwardRChGammaCorrectedValue239           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_239_UNNAMED61184;

    DW960.Value                                      = 0;        
    DW960.InverseGChGammaCorrectedValue240           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_240_UNNAMED61440;
    DW960.InverseBChGammaCorrectedValue240           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_240_UNNAMED61440;

    DW961.Value                                      = 0;        
    DW961.InversePixelValue240                       = INVERSE_PIXEL_VALUE_240_UNNAMED61440;
    DW961.InverseRChGammaCorrectedValue240           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_240_UNNAMED61440;

    DW962.Value                                      = 0;        
    DW962.ForwardGChGammaCorrectedValue240           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_240_UNNAMED61440;
    DW962.ForwardBChGammaCorrectedValue240           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_240_UNNAMED61440;

    DW963.Value                                      = 0;        
    DW963.ForwardPixelValue240                       = FORWARD_PIXEL_VALUE_240_UNNAMED61440;
    DW963.ForwardRChGammaCorrectedValue240           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_240_UNNAMED61440;

    DW964.Value                                      = 0;        
    DW964.InverseGChGammaCorrectedValue241           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_241_UNNAMED61696;
    DW964.InverseBChGammaCorrectedValue241           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_241_UNNAMED61696;

    DW965.Value                                      = 0;        
    DW965.InversePixelValue241                       = INVERSE_PIXEL_VALUE_241_UNNAMED61696;
    DW965.InverseRChGammaCorrectedValue241           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_241_UNNAMED61696;

    DW966.Value                                      = 0;        
    DW966.ForwardGChGammaCorrectedValue241           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_241_UNNAMED61696;
    DW966.ForwardBChGammaCorrectedValue241           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_241_UNNAMED61696;

    DW967.Value                                      = 0;        
    DW967.ForwardPixelValue241                       = FORWARD_PIXEL_VALUE_241_UNNAMED61696;
    DW967.ForwardRChGammaCorrectedValue241           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_241_UNNAMED61696;

    DW968.Value                                      = 0;        
    DW968.InverseGChGammaCorrectedValue242           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_242_UNNAMED61952;
    DW968.InverseBChGammaCorrectedValue242           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_242_UNNAMED61952;

    DW969.Value                                      = 0;        
    DW969.InversePixelValue242                       = INVERSE_PIXEL_VALUE_242_UNNAMED61952;
    DW969.InverseRChGammaCorrectedValue242           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_242_UNNAMED61952;

    DW970.Value                                      = 0;        
    DW970.ForwardGChGammaCorrectedValue242           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_242_UNNAMED61952;
    DW970.ForwardBChGammaCorrectedValue242           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_242_UNNAMED61952;

    DW971.Value                                      = 0;        
    DW971.ForwardPixelValue242                       = FORWARD_PIXEL_VALUE_242_UNNAMED61952;
    DW971.ForwardRChGammaCorrectedValue242           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_242_UNNAMED61952;

    DW972.Value                                      = 0;        
    DW972.InverseGChGammaCorrectedValue243           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_243_UNNAMED62208;
    DW972.InverseBChGammaCorrectedValue243           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_243_UNNAMED62208;

    DW973.Value                                      = 0;        
    DW973.InversePixelValue243                       = INVERSE_PIXEL_VALUE_243_UNNAMED62208;
    DW973.InverseRChGammaCorrectedValue243           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_243_UNNAMED62208;

    DW974.Value                                      = 0;        
    DW974.ForwardGChGammaCorrectedValue243           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_243_UNNAMED62208;
    DW974.ForwardBChGammaCorrectedValue243           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_243_UNNAMED62208;

    DW975.Value                                      = 0;        
    DW975.ForwardPixelValue243                       = FORWARD_PIXEL_VALUE_243_UNNAMED62208;
    DW975.ForwardRChGammaCorrectedValue243           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_243_UNNAMED62208;

    DW976.Value                                      = 0;        
    DW976.InverseGChGammaCorrectedValue244           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_244_UNNAMED62464;
    DW976.InverseBChGammaCorrectedValue244           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_244_UNNAMED62464;

    DW977.Value                                      = 0;        
    DW977.InversePixelValue244                       = INVERSE_PIXEL_VALUE_244_UNNAMED62464;
    DW977.InverseRChGammaCorrectedValue244           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_244_UNNAMED62464;

    DW978.Value                                      = 0;        
    DW978.ForwardGChGammaCorrectedValue244           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_244_UNNAMED62464;
    DW978.ForwardBChGammaCorrectedValue244           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_244_UNNAMED62464;

    DW979.Value                                      = 0;        
    DW979.ForwardPixelValue244                       = FORWARD_PIXEL_VALUE_244_UNNAMED62464;
    DW979.ForwardRChGammaCorrectedValue244           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_244_UNNAMED62464;

    DW980.Value                                      = 0;        
    DW980.InverseGChGammaCorrectedValue245           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_245_UNNAMED62720;
    DW980.InverseBChGammaCorrectedValue245           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_245_UNNAMED62720;

    DW981.Value                                      = 0;        
    DW981.InversePixelValue245                       = INVERSE_PIXEL_VALUE_245_UNNAMED62720;
    DW981.InverseRChGammaCorrectedValue245           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_245_UNNAMED62720;

    DW982.Value                                      = 0;        
    DW982.ForwardGChGammaCorrectedValue245           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_245_UNNAMED62720;
    DW982.ForwardBChGammaCorrectedValue245           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_245_UNNAMED62720;

    DW983.Value                                      = 0;        
    DW983.ForwardPixelValue245                       = FORWARD_PIXEL_VALUE_245_UNNAMED62720;
    DW983.ForwardRChGammaCorrectedValue245           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_245_UNNAMED62720;

    DW984.Value                                      = 0;        
    DW984.InverseGChGammaCorrectedValue246           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_246_UNNAMED62976;
    DW984.InverseBChGammaCorrectedValue246           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_246_UNNAMED62976;

    DW985.Value                                      = 0;        
    DW985.InversePixelValue246                       = INVERSE_PIXEL_VALUE_246_UNNAMED62976;
    DW985.InverseRChGammaCorrectedValue246           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_246_UNNAMED62976;

    DW986.Value                                      = 0;        
    DW986.ForwardGChGammaCorrectedValue246           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_246_UNNAMED62976;
    DW986.ForwardBChGammaCorrectedValue246           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_246_UNNAMED62976;

    DW987.Value                                      = 0;        
    DW987.ForwardPixelValue246                       = FORWARD_PIXEL_VALUE_246_UNNAMED62976;
    DW987.ForwardRChGammaCorrectedValue246           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_246_UNNAMED62976;

    DW988.Value                                      = 0;        
    DW988.InverseGChGammaCorrectedValue247           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_247_UNNAMED63232;
    DW988.InverseBChGammaCorrectedValue247           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_247_UNNAMED63232;

    DW989.Value                                      = 0;        
    DW989.InversePixelValue247                       = INVERSE_PIXEL_VALUE_247_UNNAMED63232;
    DW989.InverseRChGammaCorrectedValue247           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_247_UNNAMED63232;

    DW990.Value                                      = 0;        
    DW990.ForwardGChGammaCorrectedValue247           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_247_UNNAMED63232;
    DW990.ForwardBChGammaCorrectedValue247           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_247_UNNAMED63232;

    DW991.Value                                      = 0;        
    DW991.ForwardPixelValue247                       = FORWARD_PIXEL_VALUE_247_UNNAMED63232;
    DW991.ForwardRChGammaCorrectedValue247           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_247_UNNAMED63232;

    DW992.Value                                      = 0;        
    DW992.InverseGChGammaCorrectedValue248           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_248_UNNAMED63488;
    DW992.InverseBChGammaCorrectedValue248           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_248_UNNAMED63488;

    DW993.Value                                      = 0;        
    DW993.InversePixelValue248                       = INVERSE_PIXEL_VALUE_248_UNNAMED63488;
    DW993.InverseRChGammaCorrectedValue248           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_248_UNNAMED63488;

    DW994.Value                                      = 0;        
    DW994.ForwardGChGammaCorrectedValue248           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_248_UNNAMED63488;
    DW994.ForwardBChGammaCorrectedValue248           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_248_UNNAMED63488;

    DW995.Value                                      = 0;        
    DW995.ForwardPixelValue248                       = FORWARD_PIXEL_VALUE_248_UNNAMED63488;
    DW995.ForwardRChGammaCorrectedValue248           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_248_UNNAMED63488;

    DW996.Value                                      = 0;        
    DW996.InverseGChGammaCorrectedValue249           = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_249_UNNAMED63744;
    DW996.InverseBChGammaCorrectedValue249           = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_249_UNNAMED63744;

    DW997.Value                                      = 0;        
    DW997.InversePixelValue249                       = INVERSE_PIXEL_VALUE_249_UNNAMED63744;
    DW997.InverseRChGammaCorrectedValue249           = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_249_UNNAMED63744;

    DW998.Value                                      = 0;        
    DW998.ForwardGChGammaCorrectedValue249           = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_249_UNNAMED63744;
    DW998.ForwardBChGammaCorrectedValue249           = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_249_UNNAMED63744;

    DW999.Value                                      = 0;        
    DW999.ForwardPixelValue249                       = FORWARD_PIXEL_VALUE_249_UNNAMED63744;
    DW999.ForwardRChGammaCorrectedValue249           = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_249_UNNAMED63744;

    DW1000.Value                                     = 0;        
    DW1000.InverseGChGammaCorrectedValue250          = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_250_UNNAMED64000;
    DW1000.InverseBChGammaCorrectedValue250          = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_250_UNNAMED64000;

    DW1001.Value                                     = 0;        
    DW1001.InversePixelValue250                      = INVERSE_PIXEL_VALUE_250_UNNAMED64000;
    DW1001.InverseRChGammaCorrectedValue250          = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_250_UNNAMED64000;

    DW1002.Value                                     = 0;        
    DW1002.ForwardGChGammaCorrectedValue250          = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_250_UNNAMED64000;
    DW1002.ForwardBChGammaCorrectedValue250          = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_250_UNNAMED64000;

    DW1003.Value                                     = 0;        
    DW1003.ForwardPixelValue250                      = FORWARD_PIXEL_VALUE_250_UNNAMED64000;
    DW1003.ForwardRChGammaCorrectedValue250          = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_250_UNNAMED64000;

    DW1004.Value                                     = 0;        
    DW1004.InverseGChGammaCorrectedValue251          = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_251_UNNAMED64256;
    DW1004.InverseBChGammaCorrectedValue251          = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_251_UNNAMED64256;

    DW1005.Value                                     = 0;        
    DW1005.InversePixelValue251                      = INVERSE_PIXEL_VALUE_251_UNNAMED64256;
    DW1005.InverseRChGammaCorrectedValue251          = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_251_UNNAMED64256;

    DW1006.Value                                     = 0;        
    DW1006.ForwardGChGammaCorrectedValue251          = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_251_UNNAMED64256;
    DW1006.ForwardBChGammaCorrectedValue251          = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_251_UNNAMED64256;

    DW1007.Value                                     = 0;        
    DW1007.ForwardPixelValue251                      = FORWARD_PIXEL_VALUE_251_UNNAMED64256;
    DW1007.ForwardRChGammaCorrectedValue251          = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_251_UNNAMED64256;

    DW1008.Value                                     = 0;        
    DW1008.InverseGChGammaCorrectedValue252          = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_252_UNNAMED64512;
    DW1008.InverseBChGammaCorrectedValue252          = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_252_UNNAMED64512;

    DW1009.Value                                     = 0;        
    DW1009.InversePixelValue252                      = INVERSE_PIXEL_VALUE_252_UNNAMED64512;
    DW1009.InverseRChGammaCorrectedValue252          = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_252_UNNAMED64512;

    DW1010.Value                                     = 0;        
    DW1010.ForwardGChGammaCorrectedValue252          = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_252_UNNAMED64512;
    DW1010.ForwardBChGammaCorrectedValue252          = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_252_UNNAMED64512;

    DW1011.Value                                     = 0;        
    DW1011.ForwardPixelValue252                      = FORWARD_PIXEL_VALUE_252_UNNAMED64512;
    DW1011.ForwardRChGammaCorrectedValue252          = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_252_UNNAMED64512;

    DW1012.Value                                     = 0;        
    DW1012.InverseGChGammaCorrectedValue253          = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_253_UNNAMED64768;
    DW1012.InverseBChGammaCorrectedValue253          = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_253_UNNAMED64768;

    DW1013.Value                                     = 0;        
    DW1013.InversePixelValue253                      = INVERSE_PIXEL_VALUE_253_UNNAMED64768;
    DW1013.InverseRChGammaCorrectedValue253          = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_253_UNNAMED64768;

    DW1014.Value                                     = 0;        
    DW1014.ForwardGChGammaCorrectedValue253          = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_253_UNNAMED64768;
    DW1014.ForwardBChGammaCorrectedValue253          = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_253_UNNAMED64768;

    DW1015.Value                                     = 0;        
    DW1015.ForwardPixelValue253                      = FORWARD_PIXEL_VALUE_253_UNNAMED64768;
    DW1015.ForwardRChGammaCorrectedValue253          = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_253_UNNAMED64768;

    DW1016.Value                                     = 0;        
    DW1016.InverseGChGammaCorrectedValue254          = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_254_UNNAMED65024;
    DW1016.InverseBChGammaCorrectedValue254          = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_254_UNNAMED65024;

    DW1017.Value                                     = 0;        
    DW1017.InversePixelValue254                      = INVERSE_PIXEL_VALUE_254_UNNAMED65024;
    DW1017.InverseRChGammaCorrectedValue254          = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_254_UNNAMED65024;

    DW1018.Value                                     = 0;        
    DW1018.ForwardGChGammaCorrectedValue254          = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_254_UNNAMED65024;
    DW1018.ForwardBChGammaCorrectedValue254          = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_254_UNNAMED65024;

    DW1019.Value                                     = 0;        
    DW1019.ForwardPixelValue254                      = FORWARD_PIXEL_VALUE_254_UNNAMED65024;
    DW1019.ForwardRChGammaCorrectedValue254          = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_254_UNNAMED65024;

    DW1020.Value                                     = 0;        
    DW1020.InverseGChGammaCorrectedValue255          = INVERSE_G_CH_GAMMA_CORRECTED_VALUE_255_UNNAMED65535;
    DW1020.InverseBChGammaCorrectedValue255          = INVERSE_B_CH_GAMMA_CORRECTED_VALUE_255_UNNAMED65535;

    DW1021.Value                                     = 0;        
    DW1021.InversePixelValue255                      = INVERSE_PIXEL_VALUE_255_UNNAMED65535;
    DW1021.InverseRChGammaCorrectedValue255          = INVERSE_R_CH_GAMMA_CORRECTED_VALUE_255_UNNAMED65535;

    DW1022.Value                                     = 0;        
    DW1022.ForwardGChGammaCorrectedValue255          = FORWARD_G_CH_GAMMA_CORRECTED_VALUE_255_UNNAMED65535;
    DW1022.ForwardBChGammaCorrectedValue255          = FORWARD_B_CH_GAMMA_CORRECTED_VALUE_255_UNNAMED65535;

    DW1023.Value                                     = 0;        
    DW1023.ForwardPixelValue255                      = FORWARD_PIXEL_VALUE_255_UNNAMED65535;
    DW1023.ForwardRChGammaCorrectedValue255          = FORWARD_R_CH_GAMMA_CORRECTED_VALUE_255_UNNAMED65535;

}

mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD()
{
    DW0.Value                                        = 0;
    DW0.CompressionType                              = MEMORY_COMPRESSION_TYPE_MEDIA_COMPRESSION_ENABLE;
    DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TILED_RESOURCE_MODE_FOR_OUTPUT_FRAME_SURFACE_BASE_ADDRESS_TRMODENONE;

}

mhw_vebox_g12_X::VEBOX_HDR_FWD_GAMMA_CORRECTION_STATE_CMD::VEBOX_HDR_FWD_GAMMA_CORRECTION_STATE_CMD()
{
    DW0.Value = 0x00000000;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

}

mhw_vebox_g12_X::VEBOX_HDR_INV_GAMMA_CORRECTION_STATE_CMD::VEBOX_HDR_INV_GAMMA_CORRECTION_STATE_CMD()
{
    DW0.Value = 0x00000000;

    DW1.Value = 0x00000000;

    DW2.Value = 0x00000000;

    DW3.Value = 0x00000000;

}

mhw_vebox_g12_X::VEBOX_HDR_TONE_MAPPING_CORRECTION_STATE_CMD::VEBOX_HDR_TONE_MAPPING_CORRECTION_STATE_CMD()
{
    DW0.Value = 0x00000000;

    DW1.Value = 0x00000000;

}
mhw_vebox_g12_X::VEBOX_HDR_STATE_CMD::VEBOX_HDR_STATE_CMD()
{
    DW17440.Value = 0x00000000;

    DW17441.Value = 0x00000000;

    DW17442.Value = 0x00000000;

}