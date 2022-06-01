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
//! \file     encode_hevc_vdenc_roi_arb.h
//! \brief    Defines of the ROI adaptive region boost
//!

#ifndef __ENCODE_HEVC_VDENC_ROI_ARB_H__
#define __ENCODE_HEVC_VDENC_ROI_ARB_H__

#include "encode_hevc_vdenc_roi_strategy.h"

namespace encode
{
class ArbROI : public RoiStrategy
{
public:
    ArbROI(EncodeAllocator *allocator, MediaFeatureManager *featureManager, PMOS_INTERFACE osInterface) :
        RoiStrategy(allocator, featureManager, osInterface) { }

    virtual ~ArbROI() = default;

    MOS_STATUS PrepareParams(
        SeqParams *hevcSeqParams,
        PicParams *hevcPicParams,
        SlcParams *hevcSlcParams) override;

protected:
    void SetRoiCtrlMode(
        uint32_t        lcuIndex,
        uint32_t        regionIndex,
        StreamInParams &streaminParams) override;

    void SetQpRoiCtrlPerLcu(
        StreamInParams *        streaminParams,
        HevcVdencStreamInState *data) override;

    void SetStreaminParamByTU(
        bool            cu64Align,
        StreamInParams &streaminDataParams) override;

protected:
    size_t  m_boostCycle = 0;
    size_t  m_boostIdx   = 0;
    uint8_t m_roiCtrl    = 0;
    uint8_t m_maxCuSize  = 0;

MEDIA_CLASS_DEFINE_END(encode__ArbROI)
};
}  // namespace encode

#endif  // __ENCODE_HEVC_VDENC_ROI_ARB_H__
