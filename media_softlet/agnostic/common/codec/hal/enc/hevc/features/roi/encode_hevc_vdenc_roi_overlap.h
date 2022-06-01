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
//! \file     encode_hevc_vdenc_roi_overlap.h
//! \brief    Defines of the ROI overlap
//! \brief    Defines of the ROI overlap
//!

#ifndef __CODECHAL_HEVC_VDENC_ROI_OVERLAP_H__
#define __CODECHAL_HEVC_VDENC_ROI_OVERLAP_H__

namespace encode
{

using UintVector = std::vector<uint32_t>;

class RoiStrategy;

//!
//! \class    RoiOverlap
//!
//! \brief    Handle the overlap between ROI and Dirty ROI.
//!
//! \detail   The main purpose of this class is handling the overlap between
//!           ROI and dirty ROI. But in order to deal with all cases in a same
//!           way, we will use this class although there is no overlap between
//!           ROI and dirty ROI.
//!           In this class will hold a overlap map to store the description of
//!           each LCU. The description include overlap marker and ROI region
//!           which the LCU belong to. For detail of overlap mark, please check
//!           the definition of OverlapMarker.
//!
class RoiOverlap
{
public:
    enum OverlapMarker
    {
        mkRoi = 1,
        mkRoiNone64Align,
        mkRoiBk,
        mkRoiBkNone64Align,
        mkDirtyRoi,
        mkDirtyRoiNone64Align,
        mkDirtyRoiBk,
        mkDirtyRoiBkNone64Align
    };


    RoiOverlap() = default;

    ~RoiOverlap();

    //!
    //! \brief  Update the state of overlap
    //!
    //! \param  [in] lcuNumber
    //!         Number of LCU in stream-in buffer
    //! \return void
    //!
    void Update(uint32_t lcuNumber);

    //!
    //! \brief  Save the index of LCUs
    //!
    //! \param  [in] lcus
    //!         the vector of LCU which keeping the LCUs' index
    //! \return void
    //!
    void MarkLcus(
        const UintVector lcus, 
        OverlapMarker marker, 
        int32_t roiRegionIndex = m_maskRoiRegionIndex)
    {
        for (auto lcu : lcus)
        {
            MarkLcu(lcu, marker, roiRegionIndex);
        }
    }

    //!
    //! \brief  mark the specific LCU with provided marker
    //!
    //! \param  [in] lcus
    //!         Index of LCU
    //! \param  [in] marker
    //!         overlap marker
    //! \return void
    //!
    void MarkLcu(uint32_t lcu, OverlapMarker marker);

    //!
    //! \brief  Write streamin data according to the overlap map
    //!
    //! \param  [in] roi
    //!         ROI strategy
    //! \param  [in] dirtyRoi
    //!         Dirty ROI strategy
    //! \param  [in, out] streaminBuffer
    //!         streamin buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WriteStreaminData(
        RoiStrategy *roi,
        RoiStrategy *dirtyRoi,
        uint8_t *streaminBuffer);

private:
    //!
    //! \brief  mark the specific LCU with provided marker and region index
    //!
    //! \param  [in] lcus
    //!         Index of LCU
    //! \param  [in] marker
    //!         overlap marker
    //! \param  [in] roiRegionIndex
    //!         Index of ROI region
    //! \return void
    //!
    void MarkLcu(uint32_t lcu, OverlapMarker marker, int32_t roiRegionIndex);

    //!
    //! \brief  Check whether the marker can be written to the specific LCU.
    //!
    //! \param  [in] lcu
    //!         index of LCU
    //! \param  [in] marker
    //!         overlap marker
    //! \return bool
    //!         true if can write the marker, otherwise false
    //!
    bool CanWriteMark(uint32_t lcu, OverlapMarker marker);

    //!
    //! \brief  Get the ROI region index from the overlap map data.
    //!
    //! \param  [in] data
    //!         overlap map data
    //! \return uint32_t
    //!         ROI region index
    //!
    uint32_t GetRoiRegionIndex(uint16_t data)
    {
        return (data >> m_bitNumberOfOverlapMarker) & m_maskRoiRegionIndex;
    }

    //!
    //! \brief  Check whether the specific mark is for ROI or not
    //!
    //! \param  [in] marker
    //!         overlap marker
    //! \return bool
    //!         true if the marker is for ROI, otherwise false
    //!
    bool IsRoiMarker(OverlapMarker marker)
    {
        return (marker == mkRoi || 
                marker == mkRoiBk || 
                marker == mkRoiNone64Align || 
                marker == mkRoiBkNone64Align);
    }

    //!
    //! \brief  Check whether the specific mark is for dirty ROI or not
    //!
    //! \param  [in] marker
    //!         overlap marker
    //! \return bool
    //!         true if the marker is for dirty ROI, otherwise false
    //!
    bool IsDirtyRoiMarker(OverlapMarker marker)
    {
        return (marker == mkDirtyRoi || 
            marker == mkDirtyRoiBk || 
            marker == mkDirtyRoiNone64Align || 
            marker == mkDirtyRoiBkNone64Align);
    }

    static const uint16_t m_maskRoiRegionIndex = 0x7FF; //<! Mask for ROI region index in overlap map
    static const uint16_t m_maskOverlapMarker  = 0x1F;  //<! Mask for overlap marker in overlap map
    static const uint8_t  m_bitNumberOfOverlapMarker  = 5;   //<! Bit number of overlap marker in overlap map

    uint32_t   m_lcuNumber  = 0;       //<! Number of LCU

protected:
    //! This map is a array of LCU description. The description is a unsigned 
    //! 16 bit integer data, In each description includes overlap marker and
    //! which ROI region the LCU belong to. The structure of each descrioption
    //! as Following.
    //! 
    //! 15 14 13 12 11 10 9 8 7 6 5 | 4 3 2 1 0
    //!   ROI     region     index  |  Marker
    //! 
    //! We can use GetRoiRegionIndex and GetMarker functions to get the region
    //! index marker from description
    //!
    uint16_t *m_overlapMap = nullptr;  //<! Overlap map buffer

    //!
    //! \brief  Get the marker from the overlap map data.
    //!
    //! \param  [in] data
    //!         overlap map data
    //! \return OverlapMarker
    //!         overlap marker
    //!
    OverlapMarker GetMarker(uint16_t data)
    {
        return (OverlapMarker)(data & m_maskOverlapMarker);
    }

MEDIA_CLASS_DEFINE_END(encode__RoiOverlap)
};

}  // namespace encode
#endif  //<! __CODECHAL_HEVC_VDENC_ROI_OVERLAP_H__