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
//! \file     encode_hevc_vdenc_roi_overlap.cpp
//! \brief    Implemetation of the ROI overlap
//!

#include "encode_hevc_vdenc_roi_strategy.h"
#include "encode_hevc_vdenc_roi_overlap.h"

namespace encode
{
RoiOverlap::~RoiOverlap()
{
    MOS_FreeMemory(m_overlapMap);
    m_overlapMap = nullptr;
}

void RoiOverlap::Update(uint32_t lcuNumber)
{
    if (m_lcuNumber < lcuNumber)
    {
        MOS_FreeMemory(m_overlapMap);
        m_overlapMap = nullptr;
        m_lcuNumber  = lcuNumber;
    }

    if (m_overlapMap == nullptr)
    {
        m_overlapMap = (uint16_t *)
            MOS_AllocMemory(lcuNumber * sizeof(uint16_t));
    }

    MOS_ZeroMemory(m_overlapMap, m_lcuNumber * sizeof(uint16_t));
}

bool RoiOverlap::CanWriteMark(uint32_t lcu, OverlapMarker marker)
{
    if (m_overlapMap == nullptr || lcu >= m_lcuNumber)
    {
        return false;
    }

    // Avoid ROI background overwrite the marker write by 
    // others(Dirty ROI or ROI forground)
    if ((marker == mkRoiBk || marker == mkRoiBkNone64Align) &&
        m_overlapMap[lcu] > 0)
    {
        return false;
    }

    // Avoid ROI forground overwrite Dirty ROI forground data
    if ((marker == mkRoi || marker == mkRoiNone64Align) &&
        (m_overlapMap[lcu] == mkDirtyRoi || 
            m_overlapMap[lcu] == mkDirtyRoiNone64Align))
    {
        return false;
    }

    return true;
}

void RoiOverlap::MarkLcu(
    uint32_t lcu,
    OverlapMarker marker,
    int32_t roiRegionIndex)
{
    if (CanWriteMark(lcu, marker))
    {
        m_overlapMap[lcu] = 
            ((marker & m_maskOverlapMarker) | 
            ((roiRegionIndex & m_maskRoiRegionIndex) << 
                m_bitNumberOfOverlapMarker));
    }
}
void RoiOverlap::MarkLcu(uint32_t lcu, OverlapMarker marker)
{
    if (CanWriteMark(lcu, marker))
    {
        m_overlapMap[lcu] = 
            ((marker & m_maskOverlapMarker) | 
            (m_maskRoiRegionIndex << m_bitNumberOfOverlapMarker));
    }
}

MOS_STATUS RoiOverlap::WriteStreaminData(
    RoiStrategy *roi,
    RoiStrategy *dirtyRoi,
    uint8_t *streaminBuffer)
{
    ENCODE_CHK_NULL_RETURN(streaminBuffer);
    ENCODE_CHK_NULL_RETURN(m_overlapMap);

    for (uint32_t i = 0; i < m_lcuNumber; i++)
    {
        OverlapMarker marker = GetMarker(m_overlapMap[i]);
        uint32_t      roiRegionIndex = GetRoiRegionIndex(m_overlapMap[i]);

        if (IsRoiMarker(marker))
        {
            ENCODE_CHK_NULL_RETURN(roi);

            roi->WriteStreaminData(
                i, marker, roiRegionIndex, streaminBuffer);

        }
        else if (IsDirtyRoiMarker(marker))
        {
            ENCODE_CHK_NULL_RETURN(dirtyRoi);
            dirtyRoi->WriteStreaminData(
                i, marker, roiRegionIndex, streaminBuffer);
        }
    }
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode