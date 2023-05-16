/*===================== begin_copyright_notice ==================================

* Copyright (c) 2021, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vebox_xe_hpm.cpp
//! \brief    Constructs vebox commands on Xe_HPM based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_vebox_xe_hpm.h"
#include "mhw_utilities_xe_xpm.h"
#include "mos_solo_generic.h"

MhwVeboxInterfaceXe_Hpm::MhwVeboxInterfaceXe_Hpm(PMOS_INTERFACE pOsInterface)
    : MhwVeboxInterfaceXe_Xpm(pOsInterface)
{
    MHW_FUNCTION_ENTER;

    dwLumaStadTh         = 3200;
    dwChromaStadTh       = 1600;
    bTGNEEnable          = false;
    bHVSAutoBdrateEnable = false;
    dw4X4TGNEThCnt       = 576;

    if (pOsInterface && pOsInterface->pfnGetSkuTable)
    {
        MEDIA_FEATURE_TABLE *m_skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);

        if (m_skuTable)
        {
            m_veboxScalabilitywith4K = MEDIA_IS_SKU(m_skuTable, FtrVeboxScalabilitywith4K);
        }
        else
        {
            MHW_ASSERTMESSAGE("m_skuTable is null ptr");
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("pOsInterface or pfnGetSkuTable is null ptr");
    }
}

MhwVeboxInterfaceXe_Hpm::~MhwVeboxInterfaceXe_Hpm()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MhwVeboxInterfaceXe_Hpm::ForceGNEParams(uint8_t *pDnDiSate)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD* pVeboxDndiState = (mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *)pDnDiSate;

    MHW_CHK_NULL(pDnDiSate);

    //used by both SGNE and TGNE
    pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold  = 900;
    pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
    pVeboxDndiState->DW30.EightDirectionEdgeThreshold     = 1800; 
    
    //SGNE
    pVeboxDndiState->DW31.LargeSobelThreshold             = 1290;
    pVeboxDndiState->DW33.MaxSobelThreshold               = 1440;
    pVeboxDndiState->DW31.SmallSobelThreshold             = 480;
    pVeboxDndiState->DW32.BlockSigmaDiffThreshold         = dwBSDThreshold;
    pVeboxDndiState->DW31.SmallSobelCountThreshold        = 6;
    pVeboxDndiState->DW32.LargeSobelCountThreshold        = 6;
    pVeboxDndiState->DW32.MedianSobelCountThreshold       = 40;

    //TGNE
    pVeboxDndiState->DW50.LumaUniformityLowTh1            = 1;
    pVeboxDndiState->DW50.LumaUniformityLowTh2            = 1;
    pVeboxDndiState->DW50.LumaUniformityHighTh1           = 6;
    pVeboxDndiState->DW50.LumaUniformityHighTh2           = 0;
    pVeboxDndiState->DW49.LumaStadTh                      = 250;

    //Chroma
    pVeboxDndiState->DW8.ChromaDenoiseMovingPixelThreshold               = 2;  //m_chromaParams.dwHotPixelThresholdChromaV;
    pVeboxDndiState->DW8.ChromaDenoiseAsdThreshold                       = 512;
    pVeboxDndiState->DW8.ChromaDenoiseThresholdForSumOfComplexityMeasure = 512;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceXe_Hpm::DumpDNDIStates(uint8_t *pDnDiSate)
{
    MOS_STATUS                              eStatus         = MOS_STATUS_SUCCESS;
    mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *pVeboxDndiState = (mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *)pDnDiSate;

    MHW_CHK_NULL(pDnDiSate);

    MHW_VERBOSEMESSAGE("VeboxDndiState, DW 34-47 is DI related DW, others is DN.");
    MHW_VERBOSEMESSAGE("DW0:DenoiseMaximumHistory %d, DenoiseStadThreshold %d", pVeboxDndiState->DW0.DenoiseMaximumHistory, pVeboxDndiState->DW0.DenoiseStadThreshold);
    MHW_VERBOSEMESSAGE("DW1:DenoiseAsdThreshold %d, DenoiseHistoryIncrease %d, DenoiseMovingPixelThreshold %d", pVeboxDndiState->DW1.DenoiseAsdThreshold, pVeboxDndiState->DW1.DenoiseHistoryIncrease, pVeboxDndiState->DW1.DenoiseMovingPixelThreshold);
    MHW_VERBOSEMESSAGE("DW2:InitialDenoiseHistory %d, TemporalDifferenceThreshold %d", pVeboxDndiState->DW2.InitialDenoiseHistory, pVeboxDndiState->DW2.TemporalDifferenceThreshold);
    MHW_VERBOSEMESSAGE("DW3:HotPixelCountLuma %d, LowTemporalDifferenceThreshold %d, ProgressiveDn %d, TemporalGneEnable %d", pVeboxDndiState->DW3.HotPixelCountLuma, pVeboxDndiState->DW3.LowTemporalDifferenceThreshold, pVeboxDndiState->DW3.ProgressiveDn, pVeboxDndiState->DW3.TemporalGneEnable);
    MHW_VERBOSEMESSAGE("DW4:BlockNoiseEstimateNoiseThreshold %d, DenoiseThresholdForSumOfComplexityMeasureLuma %d, DW4.HotPixelThresholdLuma %d", pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold, pVeboxDndiState->DW4.DenoiseThresholdForSumOfComplexityMeasureLuma, pVeboxDndiState->DW4.HotPixelThresholdLuma);
    MHW_VERBOSEMESSAGE("DW5:ChromaDenoiseStadThreshold %d, HotPixelCountChromaU %d, HotPixelThresholdChromaU %d", pVeboxDndiState->DW5.ChromaDenoiseStadThreshold, pVeboxDndiState->DW5.HotPixelCountChromaU, pVeboxDndiState->DW5.HotPixelThresholdChromaU);
    MHW_VERBOSEMESSAGE("DW6:BlockNoiseEstimateEdgeThreshold %d, ChromaDenoiseEnable %d, ChromaTemporalDifferenceThreshold %d", pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold, pVeboxDndiState->DW6.ChromaDenoiseEnable, pVeboxDndiState->DW6.ChromaTemporalDifferenceThreshold);
    MHW_VERBOSEMESSAGE("DW7:ChromaLowTemporalDifferenceThreshold %d, HotPixelCountChromaV %d, HotPixelThresholdChromaV %d", pVeboxDndiState->DW7.ChromaLowTemporalDifferenceThreshold, pVeboxDndiState->DW7.HotPixelCountChromaV, pVeboxDndiState->DW7.HotPixelThresholdChromaV);
    MHW_VERBOSEMESSAGE("DW8:ChromaDenoiseAsdThreshold %d, ChromaDenoiseMovingPixelThreshold %d, ChromaDenoiseThresholdForSumOfComplexityMeasure %d", pVeboxDndiState->DW8.ChromaDenoiseAsdThreshold, pVeboxDndiState->DW8.ChromaDenoiseMovingPixelThreshold, pVeboxDndiState->DW8.ChromaDenoiseThresholdForSumOfComplexityMeasure);
    MHW_VERBOSEMESSAGE("DW9:DnyWr040 %d, DnyWr140 %d, DnyWr240 %d, DnyWr340 %d, DnyWr440 %d, DnyWr540 %d", pVeboxDndiState->DW9.DnyWr040, pVeboxDndiState->DW9.DnyWr140, pVeboxDndiState->DW9.DnyWr240, pVeboxDndiState->DW9.DnyWr340, pVeboxDndiState->DW9.DnyWr440, pVeboxDndiState->DW9.DnyWr540);
    MHW_VERBOSEMESSAGE("DW10:DnyThmax120 %d, DnyThmin120 %d, DW11: DnyDynThmin120 %d, DnyPrt5120 %d", pVeboxDndiState->DW10.DnyThmax120, pVeboxDndiState->DW10.DnyThmin120, pVeboxDndiState->DW11.DnyDynThmin120, pVeboxDndiState->DW11.DnyPrt5120);
    MHW_VERBOSEMESSAGE("DW12:DnyPrt3120 %d, DnyPrt4120 %d, DW13:DnyPrt1120 %d, DnyPrt2120 %d", pVeboxDndiState->DW12.DnyPrt3120, pVeboxDndiState->DW12.DnyPrt4120, pVeboxDndiState->DW13.DnyPrt1120, pVeboxDndiState->DW13.DnyPrt2120);
    MHW_VERBOSEMESSAGE("DW14:DnyPrt0120 %d, DnyWd2040 %d, DnyWd2140 %d, DnyWd2240 %d", pVeboxDndiState->DW14.DnyPrt0120, pVeboxDndiState->DW14.DnyWd2040, pVeboxDndiState->DW14.DnyWd2140, pVeboxDndiState->DW14.DnyWd2240);
    MHW_VERBOSEMESSAGE("DW15:DnyWd0040 %d, DnyWd0140 %d, DnyWd0240 %d, DnyWd1040 %d, DnyWd1140 %d, DnyWd1240 %d", pVeboxDndiState->DW15.DnyWd0040, pVeboxDndiState->DW15.DnyWd0140, pVeboxDndiState->DW15.DnyWd0240, pVeboxDndiState->DW15.DnyWd1040, pVeboxDndiState->DW15.DnyWd1140, pVeboxDndiState->DW15.DnyWd1240);
    MHW_VERBOSEMESSAGE("DW16:DnuWr040 %d, DnuWr140 %d, DnuWr240 %d, DnuWr340 %d, DnuWr440 %d, DnuWr540 %d", pVeboxDndiState->DW16.DnuWr040, pVeboxDndiState->DW16.DnuWr140, pVeboxDndiState->DW16.DnuWr240, pVeboxDndiState->DW16.DnuWr340, pVeboxDndiState->DW16.DnuWr440, pVeboxDndiState->DW16.DnuWr540);
    MHW_VERBOSEMESSAGE("DW17:DnuThmax120 %d, DnuThmin120 %d", pVeboxDndiState->DW17.DnuThmax120, pVeboxDndiState->DW17.DnuThmin120);
    MHW_VERBOSEMESSAGE("DW18:DnuDynThmin120 %d, DnuPrt5120 %d", pVeboxDndiState->DW18.DnuDynThmin120, pVeboxDndiState->DW18.DnuPrt5120);
    MHW_VERBOSEMESSAGE("DW19:DnuPrt3120 %d, DnuPrt4120 %d, DW20:DnuPrt1120 %d, DnuPrt2120 %d", pVeboxDndiState->DW19.DnuPrt3120, pVeboxDndiState->DW19.DnuPrt4120, pVeboxDndiState->DW20.DnuPrt1120, pVeboxDndiState->DW20.DnuPrt2120);
    MHW_VERBOSEMESSAGE("DW21:DnuPrt0120 %d, DnuWd2040 %d, DnuWd2140 %d, DnuWd2240 %d", pVeboxDndiState->DW21.DnuPrt0120, pVeboxDndiState->DW21.DnuWd2040, pVeboxDndiState->DW21.DnuWd2140, pVeboxDndiState->DW21.DnuWd2240);
    MHW_VERBOSEMESSAGE("DW22:DnuWd0040 %d, DnuWd0140 %d, DnuWd0240 %d, DnuWd1040 %d, DnuWd1140 %d, DnuWd1240 %d", pVeboxDndiState->DW22.DnuWd0040, pVeboxDndiState->DW22.DnuWd0140, pVeboxDndiState->DW22.DnuWd0240, pVeboxDndiState->DW22.DnuWd1040, pVeboxDndiState->DW22.DnuWd1140, pVeboxDndiState->DW22.DnuWd1240);
    MHW_VERBOSEMESSAGE("DW23:DnvWr040 %d, DnvWr240 %d, DnvWr340 %d, DnvWr440 %d, DnvWr5140 %d, DnvWr540", pVeboxDndiState->DW23.DnvWr040, pVeboxDndiState->DW23.DnvWr240, pVeboxDndiState->DW23.DnvWr340, pVeboxDndiState->DW23.DnvWr440, pVeboxDndiState->DW23.DnvWr5140, pVeboxDndiState->DW23.DnvWr540);
    MHW_VERBOSEMESSAGE("DW24:DnvThmax120 %d, DnvThmin120 %d, DW25:DnvDynThmin120 %d, DnvPrt5120 %d", pVeboxDndiState->DW24.DnvThmax120, pVeboxDndiState->DW24.DnvThmin120, pVeboxDndiState->DW25.DnvDynThmin120, pVeboxDndiState->DW25.DnvPrt5120);
    MHW_VERBOSEMESSAGE("DW26:DnvPrt3120 %d, DnvPrt4120 %d, DW27:DnvPrt1120 %d, DnvPrt2120 %d", pVeboxDndiState->DW26.DnvPrt3120, pVeboxDndiState->DW26.DnvPrt4120, pVeboxDndiState->DW27.DnvPrt1120, pVeboxDndiState->DW27.DnvPrt2120);
    MHW_VERBOSEMESSAGE("DW28:DnvPrt0120 %d, DnvWd2040 %d, DnvWd2140 %d, DnvWd2240 %d", pVeboxDndiState->DW28.DnvPrt0120, pVeboxDndiState->DW28.DnvWd2040, pVeboxDndiState->DW28.DnvWd2140, pVeboxDndiState->DW28.DnvWd2240);
    MHW_VERBOSEMESSAGE("DW29:DnvWd0040 %d, DnvWd0140 %d, DnvWd0240 %d, DnvWd1040 %d, DnvWd1140 %d, DnvWd1240 %d", pVeboxDndiState->DW29.DnvWd0040, pVeboxDndiState->DW29.DnvWd0140, pVeboxDndiState->DW29.DnvWd0240, pVeboxDndiState->DW29.DnvWd1040, pVeboxDndiState->DW29.DnvWd1140, pVeboxDndiState->DW29.DnvWd1240);
    MHW_VERBOSEMESSAGE("DW30:EightDirectionEdgeThreshold %d, ValidPixelThreshold %d", pVeboxDndiState->DW30.EightDirectionEdgeThreshold, pVeboxDndiState->DW30.ValidPixelThreshold);
    MHW_VERBOSEMESSAGE("DW31:LargeSobelThreshold %d, SmallSobelCountThreshold %d, SmallSobelThreshold %d", pVeboxDndiState->DW31.LargeSobelThreshold, pVeboxDndiState->DW31.SmallSobelCountThreshold, pVeboxDndiState->DW31.SmallSobelThreshold);
    MHW_VERBOSEMESSAGE("DW32:BlockSigmaDiffThreshold %d, LargeSobelCountThreshold %d, MedianSobelCountThreshold %d, DW33:MaxSobelThreshold %d", pVeboxDndiState->DW32.BlockSigmaDiffThreshold, pVeboxDndiState->DW32.LargeSobelCountThreshold, pVeboxDndiState->DW32.MedianSobelCountThreshold, pVeboxDndiState->DW33.MaxSobelThreshold);
    MHW_VERBOSEMESSAGE("DW34:SmoothMvThreshold %d, SadTightThreshold %d, ContentAdaptiveThresholdSlope %d, StmmC2 %d, SignBitForSmoothMvThreshold %d, SignBitForMaximumStmm %d, SignBitForMinimumStmm %d, Reserved1104 %d",
        pVeboxDndiState->DW34.SmoothMvThreshold,pVeboxDndiState->DW34.SadTightThreshold,pVeboxDndiState->DW34.ContentAdaptiveThresholdSlope,pVeboxDndiState->DW34.StmmC2,pVeboxDndiState->DW34.SignBitForSmoothMvThreshold,pVeboxDndiState->DW34.SignBitForMaximumStmm, pVeboxDndiState->DW34.SignBitForMinimumStmm,pVeboxDndiState->DW34.Reserved1104);
    MHW_VERBOSEMESSAGE("DW35:MaximumStmm %d, MultiplierForVecm %d, Reserved1134 %d, BlendingConstantAcrossTimeForSmallValuesOfStmm %d, BlendingConstantAcrossTimeForLargeValuesOfStmm %d, StmmBlendingConstantSelect %d",
        pVeboxDndiState->DW35.MaximumStmm,pVeboxDndiState->DW35.MultiplierForVecm,pVeboxDndiState->DW35.Reserved1134,pVeboxDndiState->DW35.BlendingConstantAcrossTimeForSmallValuesOfStmm,pVeboxDndiState->DW35.BlendingConstantAcrossTimeForLargeValuesOfStmm,pVeboxDndiState->DW35.StmmBlendingConstantSelect);
    MHW_VERBOSEMESSAGE("DW36:FmdTemporalDifferenceThreshold %d, LumatdmWt %d, ChromatdmWt %d, StmmOutputShift %d, StmmShiftUp %d, StmmShiftDown %d, MinimumStmm %d", 
        pVeboxDndiState->DW36.FmdTemporalDifferenceThreshold,pVeboxDndiState->DW36.LumatdmWt,pVeboxDndiState->DW36.ChromatdmWt,pVeboxDndiState->DW36.StmmOutputShift,pVeboxDndiState->DW36.StmmShiftUp,pVeboxDndiState->DW36.StmmShiftDown,pVeboxDndiState->DW36.MinimumStmm);
    MHW_VERBOSEMESSAGE("DW37:CoringThresholdForSvcm %d, DeltabitValueForSvcm %d, Reserved1196 %d, CoringThresholdForShcm %d, DeltabitValueForShcm %d, Reserved1212 %d", 
        pVeboxDndiState->DW37.CoringThresholdForSvcm,pVeboxDndiState->DW37.DeltabitValueForSvcm,pVeboxDndiState->DW37.Reserved1196,pVeboxDndiState->DW37.CoringThresholdForShcm,pVeboxDndiState->DW37.DeltabitValueForShcm,pVeboxDndiState->DW37.Reserved1212);
    MHW_VERBOSEMESSAGE("DW38:Reserved1216 %d, DnDiTopFirst %d, Reserved1220 %d, McdiEnable %d, FmdTearThreshold %d, CatThreshold %d, Fmd2VerticalDifferenceThreshold %d, Fmd1VerticalDifferenceThreshold %d", 
        pVeboxDndiState->DW38.Reserved1216,pVeboxDndiState->DW38.DnDiTopFirst,pVeboxDndiState->DW38.Reserved1220,pVeboxDndiState->DW38.McdiEnable,pVeboxDndiState->DW38.FmdTearThreshold,pVeboxDndiState->DW38.CatThreshold,pVeboxDndiState->DW38.Fmd2VerticalDifferenceThreshold,pVeboxDndiState->DW38.Fmd1VerticalDifferenceThreshold);
    MHW_VERBOSEMESSAGE("DW39:SadTha %d, SadThb %d, ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame %d, McPixelConsistencyThreshold %d, ProgressiveCadenceReconstructionForSecondFieldOfPreviousFrame %d, Reserved1266 %d, NeighborPixelThreshold %d, ChromaSmallerWindowForTdm %d, LumaSmallerWindowForTdm %d, Fastercovergence %d, Reserved1274 %d", 
        pVeboxDndiState->DW39.SadTha,pVeboxDndiState->DW39.SadThb,pVeboxDndiState->DW39.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame,pVeboxDndiState->DW39.McPixelConsistencyThreshold,pVeboxDndiState->DW39.ProgressiveCadenceReconstructionForSecondFieldOfPreviousFrame,
        pVeboxDndiState->DW39.Reserved1266,pVeboxDndiState->DW39.NeighborPixelThreshold,pVeboxDndiState->DW39.ChromaSmallerWindowForTdm, pVeboxDndiState->DW39.LumaSmallerWindowForTdm,pVeboxDndiState->DW39.Fastercovergence,pVeboxDndiState->DW39.Reserved1274);
    MHW_VERBOSEMESSAGE("DW40:SadWt0 %d, SadWt1 %d, SadWt2 %d, SadWt3 %d", pVeboxDndiState->DW40.SadWt0, pVeboxDndiState->DW40.SadWt1, pVeboxDndiState->DW40.SadWt2, pVeboxDndiState->DW40.SadWt3);
    MHW_VERBOSEMESSAGE("DW41:SadWt4 %d, SadWt6 %d, CoringThresholdForLumaSadCalculation %d, CoringThresholdForChromaSadCalculation %d", pVeboxDndiState->DW41.SadWt4, pVeboxDndiState->DW41.SadWt6, pVeboxDndiState->DW41.CoringThresholdForLumaSadCalculation, pVeboxDndiState->DW41.CoringThresholdForChromaSadCalculation);
    MHW_VERBOSEMESSAGE("DW42:ParDiffcheckslackthreshold %d, ParTearinghighthreshold %d, ParTearinglowthreshold %d, ParDirectioncheckth %d, ParSyntheticcontentcheck %d, ParLocalcheck %d, ParUsesyntheticcontentmedian %d, BypassDeflicker %d, Reserved1375 %d", 
        pVeboxDndiState->DW42.ParDiffcheckslackthreshold,pVeboxDndiState->DW42.ParTearinghighthreshold,pVeboxDndiState->DW42.ParTearinglowthreshold,pVeboxDndiState->DW42.ParDirectioncheckth,pVeboxDndiState->DW42.ParSyntheticcontentcheck,
        pVeboxDndiState->DW42.ParLocalcheck,pVeboxDndiState->DW42.ParUsesyntheticcontentmedian,pVeboxDndiState->DW42.BypassDeflicker, pVeboxDndiState->DW42.Reserved1375);
    MHW_VERBOSEMESSAGE("DW43:Lpfwtlut0 %d, Lpfwtlut1 %d, Lpfwtlut2 %d, Lpfwtlut3 %d", pVeboxDndiState->DW43.Lpfwtlut0, pVeboxDndiState->DW43.Lpfwtlut1, pVeboxDndiState->DW43.Lpfwtlut2, pVeboxDndiState->DW43.Lpfwtlut3);
    MHW_VERBOSEMESSAGE("DW44:Lpfwtlut4 %d, Lpfwtlut5 %d, Lpfwtlut6 %d, Lpfwtlut7 %d", pVeboxDndiState->DW44.Lpfwtlut4, pVeboxDndiState->DW44.Lpfwtlut5, pVeboxDndiState->DW44.Lpfwtlut6, pVeboxDndiState->DW44.Lpfwtlut7);
    MHW_VERBOSEMESSAGE("DW45:TdmUvThreshold %d, HvUvThreshold %d, TdmHarmonicFactorSynthetic %d, TdmHarmonicFactorNatural %d, SynthticFrame %d, SyntheticContentThreshold %d", pVeboxDndiState->DW45.TdmUvThreshold, pVeboxDndiState->DW45.HvUvThreshold, pVeboxDndiState->DW45.TdmHarmonicFactorSynthetic, pVeboxDndiState->DW45.TdmHarmonicFactorNatural, pVeboxDndiState->DW45.SynthticFrame, pVeboxDndiState->DW45.SyntheticContentThreshold);
    MHW_VERBOSEMESSAGE("DW46:SvcmHarmonicFactorSynthetic %d, ShcmHarmonicFactorSynthetic %d, SvcmHarmonicFactorNatural %d, ShcmHarmonicFactorNatural %d, HarmonicCounterThreshold %d, MaxHarmonicCounterThreshold %d, NaturalContentThreshold %d, Reserved1501 %d",
        pVeboxDndiState->DW46.SvcmHarmonicFactorSynthetic, pVeboxDndiState->DW46.ShcmHarmonicFactorSynthetic, pVeboxDndiState->DW46.SvcmHarmonicFactorNatural, pVeboxDndiState->DW46.ShcmHarmonicFactorNatural, pVeboxDndiState->DW46.HarmonicCounterThreshold, pVeboxDndiState->DW46.MaxHarmonicCounterThreshold, pVeboxDndiState->DW46.NaturalContentThreshold, pVeboxDndiState->DW46.Reserved1501);
    MHW_VERBOSEMESSAGE("DW47:MaximumValue %d, DW48:ShiftingValue %d, HvYThreshold %d, NumInlinerNumeratorThreshold %d, NumInlinerDenominatorThreshold %d, Reserved1556 %d", pVeboxDndiState->DW47.MaximumValue, pVeboxDndiState->DW48.ShiftingValue, pVeboxDndiState->DW48.HvYThreshold, pVeboxDndiState->DW48.NumInlinerNumeratorThreshold, pVeboxDndiState->DW48.NumInlinerDenominatorThreshold, pVeboxDndiState->DW48.Reserved1556);
    MHW_VERBOSEMESSAGE("DW49:ChromaStadTh %d, LumaStadTh %d", pVeboxDndiState->DW49.ChromaStadTh, pVeboxDndiState->DW49.LumaStadTh);
    MHW_VERBOSEMESSAGE("DW50:LumaUniformityHighTh1 %d, LumaUniformityHighTh2 %d, LumaUniformityLowTh1 %d, LumaUniformityLowTh2 %d", pVeboxDndiState->DW50.LumaUniformityHighTh1, pVeboxDndiState->DW50.LumaUniformityHighTh2, pVeboxDndiState->DW50.LumaUniformityLowTh1, pVeboxDndiState->DW50.LumaUniformityLowTh2);
    MHW_VERBOSEMESSAGE("DW51: ChromaUniformityHighTh1 %d, ChromaUniformityHighTh2 %d, ChromaUniformityLowTh1 %d, ChromaUniformityLowTh2 %d", pVeboxDndiState->DW51.ChromaUniformityHighTh1, pVeboxDndiState->DW51.ChromaUniformityHighTh2, pVeboxDndiState->DW51.ChromaUniformityLowTh1, pVeboxDndiState->DW51.ChromaUniformityLowTh2);
    MHW_VERBOSEMESSAGE("DW52:_4X4TemporalGneThresholdCount %d", pVeboxDndiState->DW52._4X4TemporalGneThresholdCount);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceXe_Hpm::AddVeboxDndiState(
    PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams)
{
    PMHW_VEBOX_HEAP pVeboxHeap;
    uint32_t        uiOffset;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, mVeboxDndiState;

    MHW_CHK_NULL(pVeboxDndiParams);
    MHW_CHK_NULL(m_veboxHeap);
    pVeboxHeap = m_veboxHeap;

    uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxDndiState =
        (mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                    pVeboxHeap->uiDndiStateOffset +
                                                    uiOffset);
    MHW_CHK_NULL(pVeboxDndiState);
    *pVeboxDndiState = mVeboxDndiState;

    eStatus = MhwVeboxInterfaceXe_Xpm::AddVeboxDndiState(pVeboxDndiParams);

    if (bHVSAutoBdrateEnable)
    {
        if (bTGNEEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = bTGNEEnable;
            pVeboxDndiState->DW30.ValidPixelThreshold             = 336;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = dw4X4TGNEThCnt;
            pVeboxDndiState->DW2.InitialDenoiseHistory            = dwHistoryInit;
            pVeboxDndiState->DW33.MaxSobelThreshold               = 448; //for SGNE
            //for chroma
            pVeboxDndiState->DW49.ChromaStadTh                    = dwChromaStadTh;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 9;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 2;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 1;
            
            ForceGNEParams((uint8_t*)pVeboxDndiState);
            pVeboxDndiState->DW2.InitialDenoiseHistory = dwHistoryInit;
        }
        else
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = 0;
            pVeboxDndiState->DW30.ValidPixelThreshold             = 336;
            pVeboxDndiState->DW33.MaxSobelThreshold               = 448;
            pVeboxDndiState->DW2.InitialDenoiseHistory            = dwHistoryInit;
            
            pVeboxDndiState->DW49.ChromaStadTh                    = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 0;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = 0;

            ForceGNEParams((uint8_t *)pVeboxDndiState);

            pVeboxDndiState->DW49.LumaStadTh            = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh2 = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh1 = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh2  = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh1  = 0;
        }
        
    }
    else if (bHVSAutoSubjectiveEnable)
    {
        if (bTGNEEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = bTGNEEnable;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
            pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold  = 200;
            pVeboxDndiState->DW30.EightDirectionEdgeThreshold     = 3200;
            pVeboxDndiState->DW30.ValidPixelThreshold             = 336;
            pVeboxDndiState->DW33.MaxSobelThreshold               = 1440;
            pVeboxDndiState->DW49.ChromaStadTh                    = dwChromaStadTh;
            pVeboxDndiState->DW49.LumaStadTh                      = dwLumaStadTh;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 50;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 15;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 2;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 2;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 30;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 15;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 2;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 1;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = dw4X4TGNEThCnt;
        }
        else
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = 0;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 720;
            pVeboxDndiState->DW6.BlockNoiseEstimateEdgeThreshold  = 200;
            pVeboxDndiState->DW30.EightDirectionEdgeThreshold     = 3200;
            pVeboxDndiState->DW30.ValidPixelThreshold             = 336;
            pVeboxDndiState->DW33.MaxSobelThreshold               = 1440;
            pVeboxDndiState->DW49.ChromaStadTh                    = 0;
            pVeboxDndiState->DW49.LumaStadTh                      = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 0;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = 0;
        }
    }
    else
    {
        if (bTGNEEnable)
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = bTGNEEnable;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 320;
            pVeboxDndiState->DW49.ChromaStadTh                    = dwChromaStadTh;
            pVeboxDndiState->DW49.LumaStadTh                      = dwLumaStadTh;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 50;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 10;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 2;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 1;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 30;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 15;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 2;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 1;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = dw4X4TGNEThCnt;
        }
        else
        {
            pVeboxDndiState->DW3.TemporalGneEnable                = 0;
            pVeboxDndiState->DW4.BlockNoiseEstimateNoiseThreshold = 320;
            pVeboxDndiState->DW49.ChromaStadTh                    = 0;
            pVeboxDndiState->DW49.LumaStadTh                      = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh2           = 0;
            pVeboxDndiState->DW50.LumaUniformityHighTh1           = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh2            = 0;
            pVeboxDndiState->DW50.LumaUniformityLowTh1            = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh2         = 0;
            pVeboxDndiState->DW51.ChromaUniformityHighTh1         = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh2          = 0;
            pVeboxDndiState->DW51.ChromaUniformityLowTh1          = 0;
            pVeboxDndiState->DW52._4X4TemporalGneThresholdCount   = 0;
        }
    }
    DumpDNDIStates((uint8_t *)pVeboxDndiState);
finish:
    return eStatus;
}

