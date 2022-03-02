/*
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
*/
//!
//! \file     encode_hevc_vdenc_roi_arb.cpp
//! \brief    Defines of the ROI adaptive region boost
//!

#include "encode_hevc_vdenc_roi_arb.h"

using namespace encode;

MOS_STATUS ArbROI::PrepareParams(SeqParams *hevcSeqParams, PicParams *hevcPicParams, SlcParams *hevcSlcParams)
{
    ENCODE_CHK_STATUS_RETURN(RoiStrategy::PrepareParams(hevcSeqParams, hevcPicParams, hevcSlcParams));

    m_boostCycle = m_FeatureSettings->arbSettings.m_rowOffsetsForBoost.size();
    m_boostIdx   = m_FeatureSettings->arbSettings.m_rowOffsetsForBoost[m_basicFeature->m_frameNum % m_boostCycle];
    m_roiCtrl    = m_FeatureSettings->arbSettings.m_roiCtrl;
    m_maxCuSize  = m_FeatureSettings->arbSettings.m_maxCuSize;

    return MOS_STATUS_SUCCESS;
}

void ArbROI::SetRoiCtrlMode(uint32_t lcuIndex, uint32_t regionIndex, StreamInParams &streaminParams)
{
    // streamin LCU is 32x32, CTU is 64x64
    // streamin LCU is in zigzag order of a CTU
    uint32_t ctuIdx      = lcuIndex / 4;
    uint32_t lcuIdxInCtu = lcuIndex % 4;
    uint32_t ctuY        = ctuIdx / (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 64);
    uint32_t lcuY        = (ctuY << 1) + (lcuIdxInCtu > 1 ? 1 : 0);

    if ((lcuY % m_boostCycle) == 0)
    {
        streaminParams.setQpRoiCtrl = true;
        streaminParams.roiCtrl      = m_roiCtrl;
    }
    else
    {
        streaminParams.setQpRoiCtrl = false;
    }
}

void ArbROI::SetQpRoiCtrlPerLcu(StreamInParams *streaminParams, HevcVdencStreamInState *data)
{
    if (streaminParams->setQpRoiCtrl != true)
    {
        return;
    }
    data->DW0.RoiCtrl = streaminParams->roiCtrl;
}

void ArbROI::SetStreaminParamByTU(bool cu64Align, StreamInParams &streaminDataParams)
{
    RoiStrategy::SetStreaminParamByTU(cu64Align, streaminDataParams);
    streaminDataParams.maxCuSize = m_maxCuSize;
}
