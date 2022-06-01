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
//! \file     encode_hevc_vdenc_roi_qpmap.h
//! \brief    Defines of the qpmap ROI by software
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_QPMAP_H__
#define __CODECHAL_HEVC_VDENC_ROI_QPMAP_H__

#include "encode_hevc_vdenc_roi_strategy.h"

namespace encode
{

    class QPMapROI : public RoiStrategy
    {
    public:
        QPMapROI(EncodeAllocator* allocator,
            MediaFeatureManager* featureManager,
            PMOS_INTERFACE osInterface) :
            RoiStrategy(allocator, featureManager, osInterface)
        {

        }

        virtual ~QPMapROI() {}

    protected:
        //!
        //! \brief    Set the ROI ctrol mode(Native/ForceQP/MBQPMap)
        //!
        //! \param    [in] lcuIndex
        //!           Index of LCU
        //! \param    [in] w_in16
        //!           width of frame in 16x16 block
        //! \param    [in] h_in16
        //!           hight of frame in 16x16 block
        //! \param    [in] Pitch
        //!           Pitch of QpDataSurface
        //! \param    [in] QpData
        //!           Raw data buffer of QpDataSurface
        //! \param    [out] streaminDataParams
        //!           Streamin data parameters
        //!
        //! \return   void
        //!
        virtual void SetRoiCtrlMode(
            uint32_t        lcuIndex,
            StreamInParams &streaminParams,
            uint32_t        w_in16,
            uint32_t        h_in16,
            uint32_t        Pitch,
            uint8_t *       QpData);
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
            StreamInParams* streaminParams,
            HevcVdencStreamInState* data) override;

        //!
        //! \brief    Write the Streamin data according to marker.
        //! \param    [in] lcuIndex
        //!           Index of LCU
        //! \param    [in] marker
        //!           overlap marker
        //! \param    [in] roiRegionIndex
        //!           Index of ROI region
        //! \param    [out] streamInBuffer
        //!           Streamin buffer
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS WriteStreaminData(
            uint32_t                  lcuIndex,
            RoiOverlap::OverlapMarker marker,
            uint32_t                  roiRegionIndex,
            uint8_t *                 rawStreamIn) override;

    private:


    MEDIA_CLASS_DEFINE_END(encode__QPMapROI)
    };

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_QPMAP_H__