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
//! \file     encode_hevc_vdenc_roi_forceqp.cpp
//! \brief    implementation of forceqp ROI

#include "mos_defs.h"
#include "encode_hevc_vdenc_roi_forceqp.h"
namespace encode
{

void ForceQPROI::SetQpRoiCtrlPerLcu(
    StreamInParams *streaminParams,
    HevcVdencStreamInState *data)
{
    data->DW7.QpEnable   = 0xf;
    data->DW14.ForceQp_0 = streaminParams->forceQp[0];
    data->DW14.ForceQp_1 = streaminParams->forceQp[1];
    data->DW14.ForceQp_2 = streaminParams->forceQp[2];
    data->DW14.ForceQp_3 = streaminParams->forceQp[3];
}

void ForceQPROI::SetRoiCtrlMode(
    uint32_t lcuIndex,
    uint32_t regionIndex,
    StreamInParams &streaminParams)
{
    int8_t forceQp = 0;

    // Calculate ForceQp
    forceQp = streaminParams.setQpRoiCtrl ?
        (m_qpY + m_roiRegions[regionIndex].PriorityLevelOrDQp + m_sliceQpDelta)
        : (m_qpY + m_sliceQpDelta);
    forceQp = (int8_t)CodecHal_Clip3(10, 51, forceQp);

    streaminParams.forceQp[0] = forceQp;
    streaminParams.forceQp[1] = forceQp;
    streaminParams.forceQp[2] = forceQp;
    streaminParams.forceQp[3] = forceQp;
}

}  // namespace encode
