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
//! \file     encode_avc_vdenc_brc_roi_feature.cpp
//! \brief    implementation of BRC ROI features of AVC VDENC

#include "encode_avc_vdenc_brc_roi_feature.h"

namespace encode
{

    AvcVdencBrcRoiFeature::AvcVdencBrcRoiFeature(
        MediaFeatureManager* featureManager,
        EncodeAllocator* allocator,
        CodechalHwInterfaceNext* hwInterface,
        void* constSettings,
        SupportedModes& supportedModes) :
        AvcVdencRoiInterface(featureManager, allocator, hwInterface, constSettings, supportedModes)
    {
    }

    MOS_STATUS AvcVdencBrcRoiFeature::Update(void* params)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(params);
        if (!m_brcFeature->IsVdencBrcEnabled())
        {
            return MOS_STATUS_SUCCESS;
        }

        // Allocate buffer if it doesn't exists
        if (m_nonNativeBrcRoiSupported &&
            !m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::HucRoiMapBuffer, m_basicFeature->m_frameNum))
        {
            MOS_ALLOC_GFXRES_PARAMS allocParams;
            MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParams.Type = MOS_GFXRES_BUFFER;
            allocParams.TileType = MOS_TILE_LINEAR;
            allocParams.Format = Format_Buffer;
            allocParams.dwBytes = MOS_ALIGN_CEIL(m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb, CODECHAL_CACHELINE_SIZE);
            allocParams.pBufName = "VDENC BRC ROI Map Buffer";
            allocParams.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            allocParams.bIsPersistent = true;
            allocParams.dwMemType = MOS_MEMPOOL_VIDEOMEMORY;
            ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_recycleBuf->RegisterResource(RecycleResId::HucRoiMapBuffer, allocParams));
        }

        return AvcVdencRoiInterface::Update(params);
    }

    MOS_STATUS AvcVdencBrcRoiFeature::SetupROI()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Enable());
        ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Clear());

        int32_t dqpIdx;
        uint32_t curX, curY;

        if (m_picParam->bNativeROI)
        {
            ENCODE_NORMALMESSAGE("Setup BRC Native ROI");

            auto pData = m_vdencStreamInFeature->Lock();
            ENCODE_CHK_NULL_RETURN(pData);

            for (int32_t i = m_picParam->NumROI - 1; i >= 0; i--)
            {
                ENCODE_CHK_STATUS_MESSAGE_RETURN(GetDeltaQPIndex(m_maxNumNativeRoi, m_picParam->ROI[i].PriorityLevelOrDQp, dqpIdx), "dQP index not found");
                for (curY = m_picParam->ROI[i].Top; curY < m_picParam->ROI[i].Bottom; curY++)
                {
                    for (curX = m_picParam->ROI[i].Left; curX < m_picParam->ROI[i].Right; curX++)
                    {
                        (pData + (m_basicFeature->m_picWidthInMb * curY + curX))->DW0.RegionOfInterestSelection = dqpIdx + 1;  // Shift ROI by 1
                    }
                }
            }
            m_vdencStreamInFeature->Unlock();
        }
        else
        {
            ENCODE_NORMALMESSAGE("Setup BRC Non-Native ROI");

            MOS_RESOURCE* pResRoiMapBuffer = m_basicFeature->m_recycleBuf->GetBuffer(RecycleResId::HucRoiMapBuffer, m_basicFeature->m_frameNum);
            ENCODE_CHK_NULL_RETURN(pResRoiMapBuffer);

            ROIMapBuffer* pRoiMapBuffer = (ROIMapBuffer*)m_allocator->LockResourceForWrite(pResRoiMapBuffer);
            ENCODE_CHK_NULL_RETURN(pRoiMapBuffer);

            uint32_t picSizeInMb = m_basicFeature->m_picHeightInMb * m_basicFeature->m_picWidthInMb;
            MOS_ZeroMemory(pRoiMapBuffer, picSizeInMb);
            for (int32_t i = m_picParam->NumROI - 1; i >= 0; i--)
            {
                ENCODE_CHK_STATUS_MESSAGE_RETURN(GetDeltaQPIndex(m_maxNumBrcRoi, m_picParam->ROI[i].PriorityLevelOrDQp, dqpIdx), "dQP index not found");
                for (curY = m_picParam->ROI[i].Top; curY < m_picParam->ROI[i].Bottom; curY++)
                {
                    for (curX = m_picParam->ROI[i].Left; curX < m_picParam->ROI[i].Right; curX++)
                    {
                        (pRoiMapBuffer + (m_basicFeature->m_picWidthInMb * curY + curX))->roiIndex = dqpIdx + 1; // Shift ROI by 1
                    }
                }
            }
            m_allocator->UnLock(pResRoiMapBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencBrcRoiFeature::SetupDirtyROI()
    {
        ENCODE_FUNC_CALL();
        ENCODE_NORMALMESSAGE("CRITICAL: DirtyROI feature setup will be bypassed");
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcVdencBrcRoiFeature::SetupArbROI()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Enable());
        ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Clear());

        auto pData = m_vdencStreamInFeature->Lock();
        ENCODE_CHK_NULL_RETURN(pData);

        uint16_t rowOffset[8] = {0, 3, 5, 2, 7, 4, 1, 6};
        uint16_t boostIdx     = rowOffset[m_basicFeature->m_frameNum & 7];

        for (uint16_t y = 0; y < m_basicFeature->m_picHeightInMb; y++)
        {
            if ((y & 7) == boostIdx)
            {
                for (uint16_t x = 0; x < m_basicFeature->m_picWidthInMb; x++)
                {
                    pData->DW0.RegionOfInterestSelection = 1;
                    pData++;
                }
            }
            else
            {
                pData += m_basicFeature->m_picWidthInMb;
            }
        }

        m_vdencStreamInFeature->Unlock();

        return MOS_STATUS_SUCCESS;
    }
}
