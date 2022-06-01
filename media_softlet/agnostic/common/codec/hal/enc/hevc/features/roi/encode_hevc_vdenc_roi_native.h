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
//! \brief    Defines of the ROI feature of HEVC VDENC
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_NATIVE_H__
#define __CODECHAL_HEVC_VDENC_ROI_NATIVE_H__

#include "encode_hevc_vdenc_roi_strategy.h"

namespace encode
{

class NativeROI : public RoiStrategy
{
public:
    NativeROI(EncodeAllocator *allocator, MediaFeatureManager *featureManager, PMOS_INTERFACE osInterface) :
        RoiStrategy(allocator, featureManager, osInterface)
    {

    }

    virtual ~NativeROI() {}

    //!
    //! \brief    Prepare parameters
    //!
    //! \param    [in] hevcSeqParams
    //!           pointer of sequence parameters
    //! \param    [in] hevcPicParams
    //!           pointer of picture parameters
    //! \param    [in] hevcSlcParams
    //!           pointer of slice parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareParams(
        SeqParams *hevcSeqParams,
        PicParams *hevcPicParams,
        SlcParams *hevcSlcParams) override;

protected:
    //!
    //! \brief    Set the ROI ctrol mode(Native/ForceQP)
    //!
    //! \param    [in] roiCtrl
    //!           ROI control
    //! \param    [in] forceQp
    //!           force QP value
    //! \param    [out] streaminDataParams
    //!           Streamin data parameters
    //!
    //! \return   void
    //!
    virtual void SetRoiCtrlMode(
        uint32_t lcuIndex,
        uint32_t regionIndex,
        StreamInParams &streaminParams) override;

    //!
    //! \brief    Set ROI Control/Force QP Data per LCU
    //!
    //! \param    [in] streaminDataParams
    //!           Streamin data parameters
    //! \param    [out] data
    //!           Streamin data
    //!
    //! \return   void
    //!
    virtual void SetQpRoiCtrlPerLcu(
        StreamInParams *streaminParams,
        HevcVdencStreamInState *data) override;

MEDIA_CLASS_DEFINE_END(encode__NativeROI)
};

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_NATIVE_H__