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
//! \brief    implementation of ROI feature of HEVC VDENC

#include "encode_hevc_vdenc_roi_huc_forceqp.h"
#include "mos_defs.h"

namespace encode
{

HucForceQpROI::HucForceQpROI(
    EncodeAllocator *allocator, MediaFeatureManager *featureManager, PMOS_INTERFACE osInterface) :
        RoiStrategy(allocator, featureManager, osInterface)
{
    if (m_recycle != nullptr)
    {
        MOS_ALLOC_GFXRES_PARAMS allocParams;
        MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParams.Type     = MOS_GFXRES_BUFFER;
        allocParams.TileType = MOS_TILE_LINEAR;
        allocParams.Format   = Format_Buffer;
        allocParams.dwBytes  = m_deltaQpRoiBufferSize;
        allocParams.pBufName = "Delta QP for ROI Buffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
        m_recycle->RegisterResource(
            RecycleResId::HucRoiDeltaQpBuffer, allocParams);

        allocParams.dwBytes  = m_roiStreamInBufferSize;
        allocParams.pBufName = "Output ROI Streamin Buffer";
        allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        m_recycle->RegisterResource(
            RecycleResId::HucRoiOutputBuffer, allocParams);
    }
}
MOS_STATUS HucForceQpROI::SetupRoi(RoiOverlap &overlap)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_allocator);
    ENCODE_CHK_NULL_RETURN(m_basicFeature);
    ENCODE_CHK_NULL_RETURN(m_recycle);

    uint32_t frameIndex = m_basicFeature->m_frameNum;

    m_deltaQpBuffer = m_recycle->GetBuffer(
        RecycleResId::HucRoiDeltaQpBuffer, frameIndex);
    ENCODE_CHK_NULL_RETURN(m_deltaQpBuffer);

    m_hucRoiOutput = m_recycle->GetBuffer(
        RecycleResId::HucRoiOutputBuffer, frameIndex);
    ENCODE_CHK_NULL_RETURN(m_hucRoiOutput);

    DeltaQpForRoi *deltaQpData = (DeltaQpForRoi *)
        m_allocator->LockResourceForWrite(m_deltaQpBuffer);
    ENCODE_CHK_NULL_RETURN(deltaQpData);

    MOS_ZeroMemory(deltaQpData, m_deltaQpRoiBufferSize);

    uint32_t streamInWidth    = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 64) / 32);
    uint32_t streamInHeight   = (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 64) / 32);
    uint32_t deltaQpBufWidth  = (MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, 32) / 32);
    uint32_t deltaQpBufHeight = (MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, 32) / 32);
    bool     cu64Align        = true;

    for (auto i = m_numRoi - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)
            CodecHal_Clip3(0, (deltaQpBufHeight - 1), m_roiRegions[i].Top);
        uint16_t bottom = (uint16_t)
            CodecHal_Clip3(0, deltaQpBufHeight, m_roiRegions[i].Bottom);
        uint16_t left   = (uint16_t)
            CodecHal_Clip3(0, (deltaQpBufWidth - 1), m_roiRegions[i].Left);
        uint16_t right  = (uint16_t)
            CodecHal_Clip3(0, deltaQpBufWidth, m_roiRegions[i].Right);

        //Check if all the sides of ROI regions are aligned to 64CU
        if ((top % 2 == 1) ||
            (bottom % 2 == 1) ||
            (left % 2 == 1) ||
            (right % 2 == 1))
        {
            cu64Align = false;
        }
    }

    for (auto i = m_numRoi - 1; i >= 0; i--)
    {
        //Check if the region is with in the borders
        uint16_t top    = (uint16_t)
            CodecHal_Clip3(0, (deltaQpBufHeight - 1), m_roiRegions[i].Top);
        uint16_t bottom = (uint16_t)
            CodecHal_Clip3(0, deltaQpBufHeight, m_roiRegions[i].Bottom);
        uint16_t left   = (uint16_t)
            CodecHal_Clip3(0, (deltaQpBufWidth - 1), m_roiRegions[i].Left);
        uint16_t right  = (uint16_t)
            CodecHal_Clip3(0, deltaQpBufWidth, m_roiRegions[i].Right);

        std::vector<uint32_t> lcuVector;
        GetLCUsInRoiRegion(streamInWidth, top, bottom, left, right, lcuVector);

        // Set ROI Delta QP
        for (uint32_t lcuOffset : lcuVector)
        {
            (deltaQpData + lcuOffset)->iDeltaQp = 
                m_roiRegions[i].PriorityLevelOrDQp;
        }
    }

    ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_deltaQpBuffer));


    int32_t streamInNumCUs = streamInWidth * streamInHeight;
    for (auto i = 0; i < streamInNumCUs; i++)
    {
        overlap.MarkLcu(i, cu64Align ? 
            RoiOverlap::mkRoiBk : RoiOverlap::mkRoiBkNone64Align);
    }

    return eStatus;
}

}