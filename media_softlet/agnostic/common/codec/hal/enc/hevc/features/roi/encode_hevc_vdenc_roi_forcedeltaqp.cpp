/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_roi_forcedeltaqp.cpp
//! \brief    implementation of forcedeltaqp ROI

#include "mos_defs.h"
#include "encode_hevc_vdenc_roi_forcedeltaqp.h"
namespace encode
{

void ForceDeltaQPROI::SetQpRoiCtrlPerLcu(
    StreamInParams *streaminParams,
    HevcVdencStreamInState *data)
{
    if (streaminParams->setQpRoiCtrl != true)
    {
        data->DW14.ForceQp_0 = data->DW14.ForceQp_1
            = data->DW14.ForceQp_2
            = data->DW14.ForceQp_3 = 0;
        return;
    }
    data->DW14.ForceQp_0 = streaminParams->forceQp[0];
    data->DW14.ForceQp_1 = streaminParams->forceQp[1];
    data->DW14.ForceQp_2 = streaminParams->forceQp[2];
    data->DW14.ForceQp_3 = streaminParams->forceQp[3];
}

void ForceDeltaQPROI::SetRoiCtrlMode(
    uint32_t lcuIndex,
    uint32_t regionIndex,
    StreamInParams &streaminParams)
{
    if (regionIndex > m_numRoi || streaminParams.setQpRoiCtrl != true)
    {
        return;
    }

    //Send the delta qp to to stream in buffer directly for force delta qp roi.
    char forceDeltaQp = m_roiRegions[regionIndex].PriorityLevelOrDQp;
    streaminParams.forceQp[0] = forceDeltaQp;
    streaminParams.forceQp[1] = forceDeltaQp;
    streaminParams.forceQp[2] = forceDeltaQp;
    streaminParams.forceQp[3] = forceDeltaQp;
}

}  // namespace encode
