/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_vdenc_cqp_roi_feature.cpp
//! \brief    implementation of CQP ROI features of AVC VDENC

#include "encode_avc_vdenc_cqp_roi_feature.h"

#include <array>
#include <algorithm>

namespace encode
{

AvcVdencCqpRoiFeature::AvcVdencCqpRoiFeature(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings,
    SupportedModes& supportedModes) :
    AvcVdencRoiInterface(featureManager, allocator, hwInterface, constSettings, supportedModes)
{
}

MOS_STATUS AvcVdencCqpRoiFeature::Update(void* params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);

    if (m_brcFeature->IsVdencBrcEnabled())
    {
        return MOS_STATUS_SUCCESS;
    }

    return AvcVdencRoiInterface::Update(params);
}

MOS_STATUS AvcVdencCqpRoiFeature::SetupROI()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Enable());
    ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Clear());

    auto pData = m_vdencStreamInFeature->Lock();
    ENCODE_CHK_NULL_RETURN(pData);

    // legacy AVC ROI[n]->VDEnc ROI[n+1], ROI 1 has higher priority than 2 and so on
    if (m_picParam->bNativeROI)
    {
        ENCODE_NORMALMESSAGE("Setup CQP Native ROI");

        for (int32_t i = m_picParam->NumROI - 1; i >= 0; i--)
        {
            int32_t dqpidx = -1;
            for (int32_t j = 0; j < m_maxNumNativeRoi; j++)
            {
                if (m_picParam->ROIDistinctDeltaQp[j] == m_picParam->ROI[i].PriorityLevelOrDQp)
                {
                    dqpidx = j;
                    break;
                }
            }
            if (dqpidx == -1)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }

            uint32_t curX, curY;
            for (curY = m_picParam->ROI[i].Top; curY < m_picParam->ROI[i].Bottom; curY++)
            {
                for (curX = m_picParam->ROI[i].Left; curX < m_picParam->ROI[i].Right; curX++)
                {
                    (pData + (m_basicFeature->m_picWidthInMb * curY + curX))->DW0.RegionOfInterestSelection = dqpidx + 1;  //Shift ROI by 1
                }
            }
        }
    }
    else
    {
        ENCODE_NORMALMESSAGE("Setup CQP Non-Native ROI");

        int8_t qpPrimeY = (int8_t)CodecHal_Clip3(10, 51, m_picParam->QpY + m_basicFeature->m_sliceParams->slice_qp_delta);
        for (int32_t i = 0; i < m_basicFeature->m_picWidthInMb * m_basicFeature->m_picHeightInMb; i++)
        {
            (pData + i)->DW1.QpPrimeY = qpPrimeY;
        }
        for (int32_t i = m_picParam->NumROI - 1; i >= 0; i--)
        {
            uint32_t curX, curY;
            int8_t   newQp = (int8_t)CodecHal_Clip3(10, 51, qpPrimeY + m_picParam->ROI[i].PriorityLevelOrDQp);
            for (curY = m_picParam->ROI[i].Top; curY < m_picParam->ROI[i].Bottom; curY++)
            {
                for (curX = m_picParam->ROI[i].Left; curX < m_picParam->ROI[i].Right; curX++)
                {
                    (pData + (m_basicFeature->m_picWidthInMb * curY + curX))->DW1.QpPrimeY = newQp;
                }
            }
        }
    }

    m_vdencStreamInFeature->Unlock();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencCqpRoiFeature::SetupDirtyROI()
{
    ENCODE_FUNC_CALL();
    ENCODE_NORMALMESSAGE("CRITICAL: DirtyROI feature setup will be bypassed");
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencCqpRoiFeature::SetupMBQP()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Enable());
    ENCODE_CHK_STATUS_RETURN(m_vdencStreamInFeature->Clear());

    auto pData = m_vdencStreamInFeature->Lock();
    ENCODE_CHK_NULL_RETURN(pData);

    auto pInputData = (uint8_t*)m_allocator->LockResourceForRead(&m_basicFeature->m_mbQpDataSurface.OsResource);
    ENCODE_CHK_NULL_RETURN(pInputData);

    // MBQP in ForceQP mode
    if (m_picParam->NumDeltaQpForNonRectROI == 0)
    {
        ENCODE_NORMALMESSAGE("Setup MBQP in ForceQP mode");
        for (uint32_t curY = 0; curY < m_basicFeature->m_picHeightInMb; curY++)
        {
            for (uint32_t curX = 0; curX < m_basicFeature->m_picWidthInMb; curX++)
            {
                pData->DW1.QpPrimeY = *pInputData;
                pData++;
                pInputData++;
            }
            pInputData += m_basicFeature->m_mbQpDataSurface.dwPitch - m_basicFeature->m_mbQpDataSurface.dwWidth;
        }
    }
    else // MBQP in DeltaQP mode
    {
        ENCODE_NORMALMESSAGE("Setup MBQP in DeltaQP mode");

        std::array<char, sizeof(CODEC_AVC_ENCODE_PIC_PARAMS::NonRectROIDeltaQpList) + 1> nonRectROIDeltaQpList;
        nonRectROIDeltaQpList[0] = 0;
        std::copy(
            std::begin(m_picParam->NonRectROIDeltaQpList),
            std::end(m_picParam->NonRectROIDeltaQpList),
            nonRectROIDeltaQpList.begin() + 1);
        for (uint32_t curY = 0; curY < m_basicFeature->m_picHeightInMb; curY++)
        {
            for (uint32_t curX = 0; curX < m_basicFeature->m_picWidthInMb; curX++)
            {
                pData->DW1.QpPrimeY = nonRectROIDeltaQpList[*pInputData];
                pData++;
                pInputData++;
            }
            pInputData += m_basicFeature->m_mbQpDataSurface.dwPitch - m_basicFeature->m_mbQpDataSurface.dwWidth;
        }
    }

    m_allocator->UnLock(&m_basicFeature->m_mbQpDataSurface.OsResource);
    m_vdencStreamInFeature->Unlock();
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(VDENC_AVC_IMG_STATE, AvcVdencCqpRoiFeature)
{
    // VDEnc CQP case ROI settings, BRC ROI will be handled in HuC FW
    if (!m_brcFeature->IsVdencBrcEnabled() && m_picParam->NumROI && m_picParam->bNativeROI)
    {
        int8_t priorityLevelOrDQp[ENCODE_VDENC_AVC_MAX_ROI_NUMBER_ADV] = {0};

        for (uint8_t i = 0; i < m_picParam->NumROI; i++)
        {

            // clip delta qp roi to VDEnc supported range
            if (m_picParam->ROIDistinctDeltaQp[i] == 0)
            {
                break;
            }
            priorityLevelOrDQp[i] = (char)CodecHal_Clip3(
                ENCODE_VDENC_AVC_MIN_ROI_DELTA_QP_G9, ENCODE_VDENC_AVC_MAX_ROI_DELTA_QP_G9, m_picParam->ROIDistinctDeltaQp[i]);
        }

        params.roiEnable = true;

        // Zone0 is reserved for non-ROI region
        params.roiQpAdjustmentForZone1 = priorityLevelOrDQp[0];
        params.roiQpAdjustmentForZone2 = priorityLevelOrDQp[1];
        params.roiQpAdjustmentForZone3 = priorityLevelOrDQp[2];
    }

    return MOS_STATUS_SUCCESS;
}

}
