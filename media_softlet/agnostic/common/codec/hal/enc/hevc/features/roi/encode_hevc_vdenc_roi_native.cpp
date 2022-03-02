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
//! \file     encode_hevc_vdenc_roi.h
//! \brief    implementation of ROI feature of HEVC VDENC

#include "mos_defs.h"
#include "encode_hevc_vdenc_roi_native.h"
namespace encode
{
MOS_STATUS NativeROI::PrepareParams(
    SeqParams *hevcSeqParams,
    PicParams *hevcPicParams,
    SlcParams *hevcSlcParams)
{
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    ENCODE_CHK_STATUS_RETURN(
        RoiStrategy::PrepareParams(hevcSeqParams, hevcPicParams, hevcSlcParams));

    m_numDistinctDeltaQp = 
        sizeof(hevcPicParams->ROIDistinctDeltaQp) / sizeof(int8_t);
    m_roiDistinctDeltaQp = hevcPicParams->ROIDistinctDeltaQp;
    ENCODE_CHK_NULL_RETURN(m_roiDistinctDeltaQp);

    return MOS_STATUS_SUCCESS;
}

void NativeROI::SetQpRoiCtrlPerLcu(
    StreamInParams *streaminParams,
    HevcVdencStreamInState *data)
{
    if (streaminParams->setQpRoiCtrl != true)
    {
        return;
    }
    data->DW0.RoiCtrl = streaminParams->roiCtrl;
}

void NativeROI::SetRoiCtrlMode(
    uint32_t lcuIndex,
    uint32_t regionIndex,
    StreamInParams &streaminParams)
{
    if (regionIndex > m_numRoi || streaminParams.setQpRoiCtrl != true)
    {
        return;
    }
    // For native ROI, determine Region ID based on distinct
    // delta Qps and set ROI control
    uint8_t roiCtrl = 0;
    for (auto j = 0; j < m_maxNumNativeRoi; j++)
    {
        if (m_roiDistinctDeltaQp[j] == 
                m_roiRegions[regionIndex].PriorityLevelOrDQp)
        {
            // All four 16x16 blocks within the 32x32 blocks
            // should share the same region ID j
            roiCtrl = j + 1;
            for (auto k = 0; k < 3; k++)
            {
                roiCtrl = roiCtrl << 2;
                roiCtrl = roiCtrl + j + 1;
            }
            break;
        }
    }

    streaminParams.roiCtrl = roiCtrl;
}

}  // namespace encode