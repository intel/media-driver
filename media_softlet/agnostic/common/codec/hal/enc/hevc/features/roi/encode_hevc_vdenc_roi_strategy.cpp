/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \brief    implementation of ROI feature of HEVC VDENC

#include "mos_defs.h"
#include "encode_hevc_basic_feature.h"
#include "encode_hevc_vdenc_roi_overlap.h"
#include "encode_hevc_vdenc_roi_strategy.h"
#include "encode_hevc_vdenc_roi_native.h"
#include "encode_hevc_vdenc_roi_huc_forceqp.h"
#include "encode_hevc_vdenc_roi_forceqp.h"
#include "encode_hevc_vdenc_roi_dirty.h"
#include "encode_hevc_vdenc_roi_qpmap.h"
#include "encode_hevc_vdenc_roi_arb.h"
#include "encode_hevc_vdenc_roi_forcedeltaqp.h"
#include "encode_hevc_brc.h"
#include "encode_hevc_tile.h"

namespace encode
{
MOS_STATUS RoiStrategy::PrepareParams(
    SeqParams *hevcSeqParams,
    PicParams *hevcPicParams,
    SlcParams *hevcSlcParams)
{
    ENCODE_CHK_NULL_RETURN(hevcSeqParams);
    ENCODE_CHK_NULL_RETURN(hevcPicParams);
    ENCODE_CHK_NULL_RETURN(hevcSlcParams);

    m_numRoi     = hevcPicParams->NumROI;
    m_roiRegions = hevcPicParams->ROI;
    ENCODE_CHK_NULL_RETURN(m_roiRegions);

    m_targetUsage = hevcSeqParams->TargetUsage;

    m_qpY          = hevcPicParams->QpY;
    m_sliceQpDelta = hevcSlcParams->slice_qp_delta;

    m_isTileModeEnabled  = hevcPicParams->tiles_enabled_flag;
    m_minCodingBlockSize = hevcSeqParams->log2_min_coding_block_size_minus3 + 3;

    m_numDistinctDeltaQp = sizeof(hevcPicParams->ROIDistinctDeltaQp) / sizeof(int8_t);
    m_roiDistinctDeltaQp = hevcPicParams->ROIDistinctDeltaQp;
    ENCODE_CHK_NULL_RETURN(m_roiDistinctDeltaQp);

    return MOS_STATUS_SUCCESS;
}

void RoiStrategy::StreaminZigZagToLinearMap(
    uint32_t  streamInWidth,
    uint32_t  x,
    uint32_t  y,
    uint32_t *offset,
    uint32_t *xyOffset)
{
    ENCODE_FUNC_CALL();

    *offset          = streamInWidth * y;
    uint32_t yOffset = 0;
    uint32_t xOffset = 2 * x;

    //Calculate X Y Offset for the zig zag scan with in each 64x64 LCU
    //dwOffset gives the 64 LCU row
    if (y % 2)
    {
        *offset = streamInWidth * (y - 1);
        yOffset = 2;
    }

    if (x % 2)
    {
        xOffset = (2 * x) - 1;
    }

    *xyOffset = xOffset + yOffset;
}

void RoiStrategy::ZigZagToRaster(
    uint32_t streamInWidth,
    uint32_t lcuIndex,
    uint32_t &x,
    uint32_t &y)
{
    ENCODE_FUNC_CALL();

    uint32_t lcu64Index  = lcuIndex / 4;
    uint32_t OffsetInlcu64 = lcuIndex % 4;
    uint32_t XoffsetIn64 = lcu64Index % (streamInWidth / 2);
    x = XoffsetIn64 * 2 + OffsetInlcu64 % 2;
    uint32_t YoffsetIn64 = lcu64Index / (streamInWidth / 2);
    y = YoffsetIn64 * 2 + OffsetInlcu64 / 2;
}

void RoiStrategy::SetStreaminDataPerRegion(
    const UintVector &lcuVector,
    StreamInParams *streaminParams,
    void *streaminData)
{
    ENCODE_FUNC_CALL();

    uint8_t *data = (uint8_t *)streaminData;

    for (uint32_t lcu : lcuVector)
    {
        SetStreaminDataPerLcu(streaminParams, data + lcu * 64);
    }
}

void RoiStrategy::SetStreaminDataPerLcu(
    StreamInParams *streaminParams,
    void *          streaminData)
{
    ENCODE_FUNC_CALL();
    HevcVdencStreamInState *data = (HevcVdencStreamInState *)streaminData;

    data->DW0.MaxTuSize                = streaminParams->maxTuSize;
    data->DW0.MaxCuSize                = streaminParams->maxCuSize;
    data->DW0.NumImePredictors         = streaminParams->numImePredictors;
    data->DW0.PuTypeCtrl               = streaminParams->puTypeCtrl;
    data->DW6.NumMergeCandidateCu64x64 = streaminParams->numMergeCandidateCu64x64;
    data->DW6.NumMergeCandidateCu32x32 = streaminParams->numMergeCandidateCu32x32;
    data->DW6.NumMergeCandidateCu16x16 = streaminParams->numMergeCandidateCu16x16;
    data->DW6.NumMergeCandidateCu8x8   = streaminParams->numMergeCandidateCu8x8;
}

MOS_STATUS RoiStrategy::SetupRoi(RoiOverlap &overlap)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);

    uint32_t streamInWidth  = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32);
    uint32_t streamInHeight = (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32) + 8;
    int32_t  streamInNumCUs = streamInWidth * streamInHeight;

    //ROI higher priority for smaller index.
    bool cu64Align = true;

    for (int32_t i = m_numRoi - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)
            CodecHal_Clip3(0, (streamInHeight - 1), m_roiRegions[i].Top);
        uint16_t bottom = (uint16_t)
            CodecHal_Clip3(0, streamInHeight, m_roiRegions[i].Bottom);
        uint16_t left   = (uint16_t)
            CodecHal_Clip3(0, (streamInWidth - 1), m_roiRegions[i].Left);
        uint16_t right  = (uint16_t)
            CodecHal_Clip3(0, streamInWidth, m_roiRegions[i].Right);

        //Check if all the sides of ROI regions are aligned to 64CU
        if ((top % 2 == 1) ||
            (bottom % 2 == 1) ||
            (left % 2 == 1) ||
            (right % 2 == 1))
        {
            cu64Align = false;
            break;
        }
    }

    for (int32_t i = m_numRoi - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)
            CodecHal_Clip3(0, (streamInHeight - 1), m_roiRegions[i].Top);
        uint16_t bottom = (uint16_t)
            CodecHal_Clip3(0, streamInHeight, m_roiRegions[i].Bottom);
        uint16_t left   = (uint16_t)
            CodecHal_Clip3(0, (streamInWidth - 1), m_roiRegions[i].Left);
        uint16_t right  = (uint16_t)
            CodecHal_Clip3(0, streamInWidth, m_roiRegions[i].Right);

        UintVector lcuVector;
        GetLCUsInRoiRegion(streamInWidth, top, bottom, left, right, lcuVector);

        overlap.MarkLcus(lcuVector, cu64Align ? 
            RoiOverlap::mkRoi : RoiOverlap::mkRoiNone64Align, i);
    }

    for (auto i = 0; i < streamInNumCUs; i++)
    {
        overlap.MarkLcu(i, cu64Align ? 
            RoiOverlap::mkRoiBk : RoiOverlap::mkRoiBkNone64Align);
    }

    return eStatus;
}

MOS_STATUS RoiStrategy::WriteStreaminData(
    uint32_t lcuIndex,
    RoiOverlap::OverlapMarker marker,
    uint32_t roiRegionIndex,
    uint8_t *rawStreamIn)
{
    ENCODE_CHK_NULL_RETURN(rawStreamIn);

    StreamInParams streaminDataParams = {};

    bool cu64Align = true;

    switch (marker)
    {
    case RoiOverlap::mkRoi:
        cu64Align                       = true;
        streaminDataParams.setQpRoiCtrl = true;
        break;

    case RoiOverlap::mkRoiNone64Align:
        cu64Align                       = false;
        streaminDataParams.setQpRoiCtrl = true;
        break;

    case RoiOverlap::mkRoiBk:
        cu64Align = true;
        break;

    case RoiOverlap::mkRoiBkNone64Align:
        cu64Align = false;
        break;

    default:
        return MOS_STATUS_INVALID_PARAMETER;
    }

    SetRoiCtrlMode(lcuIndex, roiRegionIndex, streaminDataParams);
    SetQpRoiCtrlPerLcu(&streaminDataParams, (HevcVdencStreamInState *)(rawStreamIn + (lcuIndex * 64)));

    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));
    SetStreaminParamByTU(cu64Align, streaminDataParams);
    SetStreaminDataPerLcu(&streaminDataParams, rawStreamIn + (lcuIndex * 64));

    return MOS_STATUS_SUCCESS;
}

void RoiStrategy::SetStreaminParamByTU(
    bool cu64Align,
    StreamInParams &streaminDataParams)
{
    MOS_ZeroMemory(&streaminDataParams, sizeof(streaminDataParams));

    auto settings = static_cast<HevcVdencFeatureSettings *>(m_featureManager->GetFeatureSettings()->GetConstSettings());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(settings);

    for (const auto &lambda : settings->vdencStreaminStateSettings)
    {
        lambda(streaminDataParams, cu64Align);
    }
}

void RoiStrategy::GetLCUsInRoiRegionForTile(
    uint32_t    streamInWidth,
    uint32_t    top,
    uint32_t    bottom,
    uint32_t    left,
    uint32_t    right,
    UintVector &lcuVector)
{
    ENCODE_FUNC_CALL();

    auto tileFeature = dynamic_cast<HevcEncodeTile *>(m_featureManager->GetFeature(HevcFeatureIDs::encodeTile));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(tileFeature);

    uint32_t tileStartLcuX = 0, tileEndLcuX = 0;
    uint32_t tileStartLcuY = 0, tileEndLcuY = 0;
    uint32_t streamInBaseOffset = 0;

    tileFeature->GetTileInfo(left,
        top,
        tileStartLcuX,
        tileEndLcuX,
        tileStartLcuY,
        tileEndLcuY,
        streamInBaseOffset);

    for (auto y = top; y < bottom; y++)
    {
        for (auto x = left; x < right; x++)
        {
            if (x < (tileStartLcuX * 2) ||
                y < (tileStartLcuY * 2) ||
                x >= (tileEndLcuX * 2) ||
                y >= (tileEndLcuY * 2))
            {
                tileFeature->GetTileInfo(x,
                    y,
                    tileStartLcuX,
                    tileEndLcuX,
                    tileStartLcuY,
                    tileEndLcuY,
                    streamInBaseOffset);
            }

            auto xPositionInTile = x - (tileStartLcuX * 2);
            auto yPositionInTile = y - (tileStartLcuY * 2);
            auto tileWidthInLCU  = tileEndLcuX - tileStartLcuX;

            uint32_t offset = 0, xyOffset = 0;

            StreaminZigZagToLinearMap(
                tileWidthInLCU * 2,
                xPositionInTile,
                yPositionInTile,
                &offset,
                &xyOffset);

            lcuVector.push_back(streamInBaseOffset + offset + xyOffset);
        }
    }
}

void RoiStrategy::GetLCUsInRoiRegion(
    uint32_t    streamInWidth,
    uint32_t    top,
    uint32_t    bottom,
    uint32_t    left,
    uint32_t    right,
    UintVector &lcuVector)
{
    if (m_isTileModeEnabled)
    {
        GetLCUsInRoiRegionForTile(streamInWidth, top, bottom, left, right, lcuVector);
        return;
    }

    for (auto y = top; y < bottom; y++)
    {
        for (auto x = left; x < right; x++)
        {
            //Calculate X Y for the zig zag scan
            uint32_t offset = 0, xyOffset = 0;
            StreaminZigZagToLinearMap(streamInWidth, x, y, &offset, &xyOffset);

            lcuVector.push_back(offset + xyOffset);
        }
    }
}

/*******************************************************

    Following is for RoiStrategyFactory

*******************************************************/

RoiStrategy *RoiStrategyFactory::CreateStrategy(
    EncodeAllocator     *allocator,
    MediaFeatureManager *featureManager,
    PMOS_INTERFACE       m_osInterface,
    bool                 isArb,
    bool                 isDirtyRoi,
    bool                 isNativeRoi,
    bool                 isQPMap)
{
    RoiStrategy *strategy = nullptr;

    if (isArb)
    {
        strategy     = m_arbRoi == nullptr ? m_arbRoi = MOS_New(ArbROI, allocator, featureManager, m_osInterface) : m_arbRoi;
        m_currentRoi = strategy;
        return strategy;
    }

    if (isDirtyRoi)
    {
        strategy   = (m_dirtyRoi == nullptr) ?
            m_dirtyRoi = MOS_New(DirtyROI, allocator, featureManager, m_osInterface) : m_dirtyRoi;

        return strategy;
    }

    if (isNativeRoi)
    {
        strategy     = (m_nativeRoi == nullptr) ?
            m_nativeRoi = MOS_New(NativeROI, allocator, featureManager, m_osInterface) : m_nativeRoi;
        m_currentRoi = strategy;
        return strategy;
    }

    if (isQPMap)
    {
        strategy = (m_QPMapROI == nullptr) ?
            m_QPMapROI = MOS_New(QPMapROI, allocator, featureManager, m_osInterface) : m_QPMapROI;
        m_currentRoi = strategy;
        return strategy;
    }

    auto brcFeature = dynamic_cast<HEVCEncodeBRC *>
        (featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
    if (brcFeature != nullptr && brcFeature->IsVdencHucUsed())
    {
        strategy = (m_hucForceQpRoi == nullptr) ?
            m_hucForceQpRoi = MOS_New(HucForceQpROI, allocator, featureManager, m_osInterface) : m_hucForceQpRoi;

        m_currentRoi = strategy;
        return strategy;
    }

    strategy     = (m_forceQpRoi == nullptr) ?
        m_forceQpRoi = MOS_New(ForceQPROI, allocator, featureManager, m_osInterface) : m_forceQpRoi;

    m_currentRoi = strategy;
    return strategy;
}

RoiStrategy *RoiStrategyFactory::CreateStrategyForceDeltaQP(
    EncodeAllocator* allocator,
    MediaFeatureManager* featureManager,
    PMOS_INTERFACE m_osInterface)
{
    RoiStrategy *strategy = nullptr;

    strategy = (m_deltaQpRoi == nullptr) ?
        m_deltaQpRoi = MOS_New(ForceDeltaQPROI, allocator, featureManager, m_osInterface) : m_deltaQpRoi;
    m_currentRoi = strategy;

    return strategy;
}

RoiStrategyFactory::~RoiStrategyFactory()
{
    if (m_dirtyRoi != nullptr)
    {
        MOS_Delete(m_dirtyRoi);
        m_dirtyRoi = nullptr;
    }

    if (m_nativeRoi != nullptr)
    {
        MOS_Delete(m_nativeRoi);
        m_nativeRoi = nullptr;
    }

    if (m_arbRoi != nullptr)
    {
        MOS_Delete(m_arbRoi);
        m_arbRoi = nullptr;
    }

    if (m_deltaQpRoi != nullptr)
    {
        MOS_Delete(m_deltaQpRoi);
        m_deltaQpRoi = nullptr;
    }

    if (m_hucForceQpRoi != nullptr)
    {
        MOS_Delete(m_hucForceQpRoi);
        m_hucForceQpRoi = nullptr;
    }

    if (m_forceQpRoi != nullptr)
    {
        MOS_Delete(m_forceQpRoi);
        m_forceQpRoi = nullptr;
    }

    if (m_QPMapROI != nullptr)
    {
        MOS_Delete(m_QPMapROI);
        m_QPMapROI = nullptr;
    }
}

}  // namespace encode
