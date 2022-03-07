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

finish:
    return eStatus;
}

