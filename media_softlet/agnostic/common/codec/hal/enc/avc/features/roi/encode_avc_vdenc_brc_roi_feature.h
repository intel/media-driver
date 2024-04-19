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
//! \file     encode_avc_vdenc_brc_roi_feature.h
//! \brief    Define implementation of the BRC ROI features of AVC VDENC
//!

#ifndef __CODECHAL_AVC_VDENC_BRC_ROI_FEATURE_H__
#define __CODECHAL_AVC_VDENC_BRC_ROI_FEATURE_H__

#include "encode_avc_vdenc_roi_interface.h"

namespace encode
{

//!
//! \class    AvcVdencBrcRoiFeature
//!
//! \brief    AvcVdencBrcRoiFeature is implementation of BRC VDEnc ROI and Dirty ROI features.
//!
//! \detail   This class is an implementation for AVC native and non-native ROI 
//!           in both ForceQP and DeltaQP modes and Dirty ROI.
class AvcVdencBrcRoiFeature : public AvcVdencRoiInterface
{
public:
    //!
    //! \struct    ROIMapBuffer
    //! \brief     This struct is defined for BRC Update HUC kernel
    //!            region 8 - ROI Map buffer
    //!            The index ranges in [0; 7]
    //!
    struct ROIMapBuffer
    {
        uint8_t roiIndex;
    };

    AvcVdencBrcRoiFeature(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings,
        SupportedModes& supportedModes);

    virtual ~AvcVdencBrcRoiFeature() {}

    virtual MOS_STATUS Update(void* params) override;

protected:

    virtual MOS_STATUS SetupROI() override;

    virtual MOS_STATUS SetupDirtyROI() override;

    virtual MOS_STATUS SetupArbROI() override;

    const bool    m_nonNativeBrcRoiSupported = true;  //!< Non native ROI in BRC mode enable flag.
    const uint8_t m_maxNumBrcRoi             = 7;     //!< VDEnc maximum number of BRC ROI with different dQP supported by HuC (without non-ROI zone0)

MEDIA_CLASS_DEFINE_END(encode__AvcVdencBrcRoiFeature)
};

}  // namespace encode
#endif  //<! __CODECHAL_AVC_VDENC_BRC_ROI_FEATURE_H__
