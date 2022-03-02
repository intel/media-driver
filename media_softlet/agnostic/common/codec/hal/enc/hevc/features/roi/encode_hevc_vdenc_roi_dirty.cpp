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
//! \file     encode_hevc_vdenc_roi_dirty.cpp
//! \brief    implementation of dirty ROI

#include "mos_defs.h"
#include "encode_hevc_vdenc_roi_dirty.h"

namespace encode
{
MOS_STATUS DirtyROI::PrepareParams(
    SeqParams *hevcSeqParams,
    PicParams *hevcPicParams,
    SlcParams *hevcSlcParams)
{
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    ENCODE_CHK_STATUS_RETURN(RoiStrategy::PrepareParams(
        hevcSeqParams, hevcPicParams, hevcSlcParams));

    m_numDirtyRects = hevcPicParams->NumDirtyRects;
    m_dirtyRegions  = hevcPicParams->pDirtyRect;
    ENCODE_CHK_NULL_RETURN(m_dirtyRegions);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DirtyROI::SetupRoi(RoiOverlap &overlap)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    uint32_t streamInWidth  = 
        (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32);
    uint32_t streamInHeight = 
        (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32);
    int32_t  streamInNumCUs = streamInWidth * streamInHeight;

    for (auto i = 0; i < streamInNumCUs; i++)
    {
        overlap.MarkLcu(i, RoiOverlap::mkDirtyRoiBk);
    }

    uint32_t streamInWidthNo64Align  = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 32) / 32);
    uint32_t streamInHeightNo64Align = (MOS_ALIGN_CEIL(m_basicFeature->m_oriFrameHeight, 32) / 32);

    bool bActualWidth32Align  = (m_basicFeature->m_frameWidth % 32) == 0;
    bool bActualHeight32Align = (m_basicFeature->m_oriFrameHeight % 32) == 0;

    // Set the static region when the width is not 64 CU aligned.
    if (streamInWidthNo64Align != streamInWidth || !bActualWidth32Align)
    {
        auto border_top    = 0;
        auto border_bottom = streamInHeight;
        auto border_left   = streamInWidthNo64Align - 1;
        auto border_right  = streamInWidth;

        if (!bActualWidth32Align)
        {
            StreaminSetDirtyRectRegion(streamInWidth,
                border_top,
                border_bottom,
                border_left,
                border_right,
                true,
                overlap);

            if (streamInWidthNo64Align == streamInWidth)
            {
                StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
                    border_top,
                    border_bottom,
                    border_left - 1,
                    border_right - 1,
                    overlap);
            }
        }
        else
        {
            StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
                border_top,
                border_bottom,
                border_left,
                border_right,
                overlap);
        }
    }

    // Set the static region when the height is not 64 CU aligned.
    if (streamInHeightNo64Align != streamInHeight || !bActualHeight32Align)
    {
        auto border_top    = streamInHeightNo64Align - 1;
        auto border_bottom = streamInHeight;
        auto border_left   = 0;
        auto border_right  = streamInWidth;

        if (!bActualHeight32Align)
        {
            StreaminSetDirtyRectRegion(streamInWidth,
                border_top,
                border_bottom,
                border_left,
                border_right,
                true,
                overlap);

            if (streamInHeightNo64Align == streamInHeight)
            {
                StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
                    border_top - 1,
                    border_bottom - 1,
                    border_left,
                    border_right,
                    overlap);
            }
        }
        else
        {
            StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
                border_top,
                border_bottom,
                border_left,
                border_right,
                overlap);
        }
    }

    for (int i = m_numDirtyRects - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)
            CodecHal_Clip3(0, (streamInHeight - 1), m_dirtyRegions[i].Top);
        uint16_t bottom = (uint16_t)
            CodecHal_Clip3(0, (streamInHeight - 1), m_dirtyRegions[i].Bottom) + 1;
        uint16_t left   = (uint16_t)
            CodecHal_Clip3(0, (streamInWidth - 1), m_dirtyRegions[i].Left);
        uint16_t right  = (uint16_t)
            CodecHal_Clip3(0, (streamInWidth - 1), m_dirtyRegions[i].Right) + 1;

        auto dirtyrect_top    = top;
        auto dirtyrect_bottom = bottom;
        auto dirtyrect_left   = left;
        auto dirtyrect_right  = right;

        // If the border of the DirtyRect is not aligned with 64 CU,
        // different setting in the border
        HandleTopNot64CuAligned(
            top, left, right, streamInWidth, overlap, dirtyrect_top);
        HandleBottomNot64CuAligned(
            bottom, left, right, streamInWidth, overlap, dirtyrect_bottom);
        HandleLeftNot64CuAligned(
            left, top, bottom, streamInWidth, overlap, dirtyrect_left);
        HandleRightNot64CuAligned(
            right, top, bottom, streamInWidth, overlap, dirtyrect_right);

        StreaminSetDirtyRectRegion(streamInWidth,
            dirtyrect_top,
            dirtyrect_bottom,
            dirtyrect_left,
            dirtyrect_right,
            true,
            overlap);
    }

    return eStatus;
}

MOS_STATUS DirtyROI::WriteStreaminData(
    uint32_t lcuIndex,
    RoiOverlap::OverlapMarker marker,
    uint32_t roiRegionIndex,
    uint8_t *rawStreamIn)
{
    ENCODE_CHK_NULL_RETURN(rawStreamIn);

    StreamInParams streaminDataParams;
    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));

    switch (marker)
    {
    case RoiOverlap::mkDirtyRoi:
        SetStreaminParamByTU(true, streaminDataParams);
        break;
    case RoiOverlap::mkDirtyRoiNone64Align:
        SetStreaminParamByTU(false, streaminDataParams);
        break;
    case RoiOverlap::mkDirtyRoiBk:
        SetStreaminBackgroundData(true, streaminDataParams);
        break;
    case RoiOverlap::mkDirtyRoiBkNone64Align:
        SetStreaminBackgroundData(false, streaminDataParams);
        
        break;
    default:
        return MOS_STATUS_INVALID_PARAMETER;
    }

    SetStreaminDataPerLcu(&streaminDataParams, rawStreamIn + (lcuIndex * 64));
    return MOS_STATUS_SUCCESS;
}

void DirtyROI::HandleRightNot64CuAligned(
    uint16_t  right,
    uint16_t  top,
    uint16_t  bottom,
    uint32_t  streamInWidth,
    RoiOverlap &overlap,
    uint16_t &dirtyrect_right)
{
    if (right % 2 != 0)
    {
        auto border_top    = top;
        auto border_bottom = bottom;
        auto border_left   = right - 1;
        auto border_right  = right;

        StreaminSetDirtyRectRegion(
            streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            false,
            overlap);

        border_top    = (top % 2 != 0) ? top - 1 : top;
        border_bottom = (bottom % 2 != 0) ? bottom + 1 : bottom;
        border_left   = right;
        border_right  = right + 1;

        StreaminSetBorderNon64AlignStaticRegion(
            streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            overlap);
        dirtyrect_right = right - 1;
    }
}

void DirtyROI::HandleLeftNot64CuAligned(
    uint16_t left,
    uint16_t top,
    uint16_t bottom,
    uint32_t streamInWidth,
    RoiOverlap &overlap,
    uint16_t &dirtyrect_left)
{
    if (left % 2 != 0)
    {
        auto border_top    = top;
        auto border_bottom = bottom;
        auto border_left   = left;
        auto border_right  = left + 1;

        StreaminSetDirtyRectRegion(
            streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            false,
            overlap);

        border_top    = (top % 2 != 0) ? top - 1 : top;
        border_bottom = (bottom % 2 != 0) ? bottom + 1 : bottom;
        border_left   = left - 1;
        border_right  = left;

        StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            overlap);

        dirtyrect_left = left + 1;
    }
}

void DirtyROI::HandleBottomNot64CuAligned(
    uint16_t bottom,
    uint16_t left,
    uint16_t right,
    uint32_t streamInWidth,
    RoiOverlap &overlap,
    uint16_t &dirtyrect_bottom)
{
    if (bottom % 2 != 0)
    {
        auto border_top    = bottom - 1;
        auto border_bottom = bottom;
        auto border_left   = left;
        auto border_right  = right;

        StreaminSetDirtyRectRegion(streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            false,
            overlap);

        border_top    = bottom;
        border_bottom = bottom + 1;
        border_left   = (left % 2 != 0) ? left - 1 : left;
        border_right  = (right % 2 != 0) ? right + 1 : right;

        StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            overlap);

        dirtyrect_bottom = bottom - 1;
    }
}

void DirtyROI::HandleTopNot64CuAligned(
    uint16_t top,
    uint16_t left,
    uint16_t right,
    uint32_t streamInWidth,
    RoiOverlap &overlap,
    uint16_t &dirtyrect_top)
{
    if (top % 2 != 0)
    {
        auto border_top    = top;
        auto border_bottom = top + 1;
        auto border_left   = left;
        auto border_right  = right;

        StreaminSetDirtyRectRegion(streamInWidth,
            border_top,
            border_bottom,
            border_left,
            border_right,
            false,
            overlap);

        border_top    = top - 1;
        border_bottom = top;
        border_left   = (left % 2 != 0) ? left - 1 : left;
        border_right  = (right % 2 != 0) ? right + 1 : right;

        StreaminSetBorderNon64AlignStaticRegion(streamInWidth,
            border_top, border_bottom, border_left, border_right, overlap);

        dirtyrect_top = top + 1;
    }
}

void DirtyROI::StreaminSetDirtyRectRegion(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    bool cu64Align,
    RoiOverlap &overlap)
{
    ENCODE_FUNC_CALL();

    std::vector<uint32_t> lcuVector;
    GetLCUsInRoiRegion(streamInWidth, top, bottom, left, right, lcuVector);

    overlap.MarkLcus(lcuVector, cu64Align ? 
        RoiOverlap::mkDirtyRoi : 
        RoiOverlap::mkDirtyRoiNone64Align);
}

void DirtyROI::StreaminSetBorderNon64AlignStaticRegion(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    RoiOverlap &overlap)
{
    ENCODE_FUNC_CALL();

    std::vector<uint32_t> lcuVector;
    GetLCUsInRoiRegion(streamInWidth, top, bottom, left, right, lcuVector);

    for (uint32_t lcu : lcuVector)
    {
        overlap.MarkLcus(lcuVector, RoiOverlap::mkDirtyRoiBkNone64Align);
    }
}

void DirtyROI::SetStreaminBackgroundData(
    bool cu64Align, StreamInParams &streaminDataParams)
{
    streaminDataParams.maxTuSize                = 3;
    streaminDataParams.maxCuSize                = cu64Align ? 3 : 2;
    streaminDataParams.numMergeCandidateCu64x64 = cu64Align ? 1 : 0; // MergeCand setting for Force MV
    streaminDataParams.numMergeCandidateCu32x32 = cu64Align ? 0 : 1;
    streaminDataParams.numMergeCandidateCu16x16 = 0;
    streaminDataParams.numMergeCandidateCu8x8   = 0;
    streaminDataParams.numImePredictors         = 0;
    streaminDataParams.puTypeCtrl               = 0xff;  //Force MV
}

}