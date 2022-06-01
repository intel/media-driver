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
//! \file     encode_hevc_vdenc_roi_dirty.h
//! \brief    Defines of the dirty ROI
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_DIRTY_H__
#define __CODECHAL_HEVC_VDENC_ROI_DIRTY_H__

#include "encode_hevc_vdenc_roi_strategy.h"

namespace encode
{

class DirtyROI : public RoiStrategy
{
public:
    DirtyROI(EncodeAllocator *allocator, MediaFeatureManager *featureManager, PMOS_INTERFACE osInterface) :
        RoiStrategy(allocator, featureManager, osInterface)
    {
    }

    virtual ~DirtyROI() {}

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

    //!
    //! \brief    Setup the ROI regione
    //!
    //! \param    [in] overlap
    //!           Overlap between ROI and dirty ROI
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupRoi(RoiOverlap &overlap) override;

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
        uint32_t lcuIndex,
        RoiOverlap::OverlapMarker marker,
        uint32_t roiRegionIndex,
        uint8_t *rawStreamIn) override;

private:
    //!
    //! \brief    Setup stream-in surface for a dirty rectangle
    //!
    //! \param    [in] streamInWidth, top, bottom, left, right, maxcu
    //!           StreamInWidth, dirtyRect corner locations, maxCuSize
    //! \param    [out] streaminData
    //!           Pointer to streaminData
    //!
    //! \return   void
    //!
    void StreaminSetDirtyRectRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        bool     cu64Align,
        RoiOverlap &overlap);

    //!
    //! \brief    Setup stream-in for border of non-64 aligned region
    //!
    //! \param    [in] streamInWidth, top, bottom, left, right
    //!           StreamInWidth, dirtyRect corner locations
    //! \param    [out] streaminData
    //!           Pointer to streaminData
    //!
    //! \return   void
    //!
    void StreaminSetBorderNon64AlignStaticRegion(
        uint32_t streamInWidth,
        uint32_t top,
        uint32_t bottom,
        uint32_t left,
        uint32_t right,
        RoiOverlap &overlap);

    //!
    //! \brief    Handle not 64 CU align for right border.
    //!
    //! \param    [in] right
    //!           Right of the region
    //! \param    [in] top
    //!           Top of the region
    //! \param    [in] bottom
    //!           Bottom of the region
    //! \param    [in] streamInWidth
    //!           Width of the streamin
    //! \param    [out] overlap
    //!           Roi overlap
    //! \param    [out] dirtyrect_right
    //!           64 CU aligned right border
    //!
    //! \return   void
    //!
    void HandleRightNot64CuAligned(
        uint16_t  right,
        uint16_t  top,
        uint16_t  bottom,
        uint32_t  streamInWidth,
        RoiOverlap &overlap,
        uint16_t &dirtyrect_right);

    //!
    //! \brief    Handle not 64 CU align for left border.
    //!
    //! \param    [in] left
    //!           Left of the region
    //! \param    [in] top
    //!           Top of the region
    //! \param    [in] bottom
    //!           Bottom of the region
    //! \param    [in] streamInWidth
    //!           Width of the streamin
    //! \param    [out] overlap
    //!           Roi overlap
    //! \param    [out] dirtyrect_left
    //!           64 CU aligned left border
    //!
    //! \return   void
    //!
    void HandleLeftNot64CuAligned(
        uint16_t  left,
        uint16_t  top,
        uint16_t  bottom,
        uint32_t  streamInWidth,
        RoiOverlap &overlap,
        uint16_t &dirtyrect_left);

    //!
    //! \brief    Handle not 64 CU align for bottom border.
    //!
    //! \param    [in] bottom
    //!           Bottom of the region
    //! \param    [in] left
    //!           Left of the region
    //! \param    [in] right
    //!           Right of the region
    //! \param    [in] streamInWidth
    //!           Width of the streamin
    //! \param    [out] overlap
    //!           Roi overlap
    //! \param    [out] dirtyrect_bottom
    //!           64 CU aligned bottom border
    //!
    //! \return   void
    //!
    void HandleBottomNot64CuAligned(
        uint16_t  bottom,
        uint16_t  left,
        uint16_t  right,
        uint32_t  streamInWidth,
        RoiOverlap &overlap,
        uint16_t &dirtyrect_bottom);

    //!
    //! \brief    Handle not 64 CU align for top border.
    //!
    //! \param    [in] top
    //!           Top of the region
    //! \param    [in] left
    //!           Left of the region
    //! \param    [in] right
    //!           Right of the region
    //! \param    [in] streamInWidth
    //!           Width of the streamin
    //! \param    [out] overlap
    //!           Roi overlap
    //! \param    [out] dirtyrect_top
    //!           64 CU aligned top border
    //!
    //! \return   void
    //!
    void HandleTopNot64CuAligned(
        uint16_t  top,
        uint16_t  left,
        uint16_t  right,
        uint32_t  streamInWidth,
        RoiOverlap &overlap,
        uint16_t &dirtyrect_top);

    //!
    //! \brief    Set the background data for dirty ROI in streamin.
    //!
    //! \param    [in] cu64Align
    //!           Is 64 CU aligned
    //! \param    [in] streaminDataParams
    //!           Streamin data parameters
    //!
    //! \return   void
    //!
    void SetStreaminBackgroundData(bool cu64Align,
        StreamInParams &                streaminDataParams);

protected:
    uint8_t    m_numDirtyRects = 0;
    CODEC_ROI *m_dirtyRegions  = nullptr;

MEDIA_CLASS_DEFINE_END(encode__DirtyROI)
};

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_DIRTY_H__