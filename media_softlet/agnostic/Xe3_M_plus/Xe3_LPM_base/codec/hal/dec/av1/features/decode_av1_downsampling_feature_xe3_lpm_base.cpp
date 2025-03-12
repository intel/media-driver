/*
* Copyright (c) 2022-2025, Intel Corporation
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
//! \file     decode_av1_downsampling_feature_xe3_lpm_base.cpp
//! \brief    Defines the interface for Av1 decode downsampling feature
//! \details  The Av1 decode downsampling feature interface is maintaining the down sampling context.
//!
#include "decode_av1_downsampling_feature_xe3_lpm_base.h"
#include "decode_av1_basic_feature.h"
#include "decode_av1_reference_frames.h"
#include "decode_utils.h"
#include "codechal_debug.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
Av1DownSamplingFeatureXe3_Lpm_Base::Av1DownSamplingFeatureXe3_Lpm_Base(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface)
    : DecodeDownSamplingFeature(featureManager, allocator, osInterface)
{
}

Av1DownSamplingFeatureXe3_Lpm_Base::~Av1DownSamplingFeatureXe3_Lpm_Base()
{
    MOS_STATUS eStatus = m_allocator->Destroy(m_histogramBufferU);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Failed to free histogram internal buffer U!");
    }
    eStatus = m_allocator->Destroy(m_histogramBufferV);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Failed to free histogram internal buffer V!");
    }
}

MOS_STATUS Av1DownSamplingFeatureXe3_Lpm_Base::GetRefFrameList(std::vector<uint32_t> &refFrameList)
{
    DECODE_FUNC_CALL();
    Av1BasicFeature *av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);

    std::vector<uint32_t> refFrameIndexList;
    refFrameIndexList.clear();
    for (auto i = 0; i < av1TotalRefsPerFrame; i++)
    {
        uint8_t index = av1BasicFeature->m_av1PicParams->m_refFrameMap[i].FrameIdx;
        if (index < CODECHAL_MAX_DPB_NUM_AV1)
        {
            refFrameIndexList.push_back(index);
        }
    }

    refFrameList.clear();
    for (uint32_t frameIdx : refFrameIndexList)
    {
        refFrameList.push_back(frameIdx);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe3_Lpm_Base::GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height)
{
    width  = m_basicFeature->m_width;
    height = m_basicFeature->m_height;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe3_Lpm_Base::GetDecodeTargetFormat(MOS_FORMAT &format)
{
    DECODE_FUNC_CALL()

    auto av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);

    auto av1PicParams = av1BasicFeature->m_av1PicParams;
    DECODE_CHK_NULL(av1PicParams);

    if (av1PicParams->m_profile == 0)
    {
        if (av1PicParams->m_bitDepthIdx == 0)
        {
            format = Format_NV12;
        }
        else if (av1PicParams->m_bitDepthIdx == 1)
        {
            format = Format_P010;
        }
        else
        {
            DECODE_ASSERTMESSAGE("Unsupported sub-sampling format!");
        }
    }
    else
    {
        DECODE_ASSERTMESSAGE("The profile has not been supported yet!");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe3_Lpm_Base::UpdateDecodeTarget(MOS_SURFACE &surface)
{
    DECODE_FUNC_CALL();
    Av1BasicFeature *av1BasicFeature = dynamic_cast<Av1BasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(av1BasicFeature);

    DECODE_CHK_STATUS(av1BasicFeature->UpdateDestSurface(surface));

    Av1ReferenceFrames &refFrame = av1BasicFeature->m_refFrames;
    PCODEC_REF_LIST_AV1 pRefList = refFrame.m_currRefList;
    DECODE_CHK_NULL(pRefList);
    DECODE_CHK_STATUS(refFrame.UpdateCurResource(pRefList));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe3_Lpm_Base::Update(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    DecodeDownSamplingFeature::Update(params);

    if (m_enabled && m_histogramBuffer)
    {
        m_aqmHistogramEnable         = true;
    }
    else
    {
        m_histogramDestSurfU         = nullptr;
        m_histogramBufferU           = nullptr;
        m_histogramDestSurfV         = nullptr;
        m_histogramBufferV           = nullptr;
        m_histogramStatisticsSummary = nullptr;
        m_histogramMataDataStreamOut = nullptr;
        m_histogramMataDataStreamIn  = nullptr;

        return MOS_STATUS_SUCCESS;
    }

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;

    // Histogram U plane
    if (m_allocator->ResourceIsNull(&decodeParams->m_histogramSurfaceU.OsResource) && !m_histogramDebug)
    {
        m_histogramDestSurfU = nullptr;
        m_histogramBufferU   = nullptr;
    }
    else
    {
        m_histogramDestSurfU = &decodeParams->m_histogramSurfaceU;
        if (m_histogramBufferU == nullptr)
        {
            //m_histogramBufferU = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx);
            m_histogramBufferU = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram internal buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (m_histogramBufferU == nullptr ||
                m_allocator->ResourceIsNull(&m_histogramBufferU->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram internal buffer!");
            }
        }
        DECODE_CHK_NULL(m_histogramBufferU);
    }

    // Histogram V plane
    if (m_allocator->ResourceIsNull(&decodeParams->m_histogramSurfaceV.OsResource) && !m_histogramDebug)
    {
        m_histogramDestSurfV = nullptr;
        m_histogramBufferV   = nullptr;
    }
    else
    {
        m_histogramDestSurfV = &decodeParams->m_histogramSurfaceV;
        if (m_histogramBufferV == nullptr)
        {
            //m_histogramBufferV = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx);
            m_histogramBufferV = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram internal buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (m_histogramBufferV == nullptr ||
                m_allocator->ResourceIsNull(&m_histogramBufferV->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram internal buffer!");
            }
        }
        DECODE_CHK_NULL(m_histogramBufferV);
    }

    if ((m_histogramStatisticsSummary == nullptr) &&
        (!m_allocator->ResourceIsNull(&decodeParams->m_histogramSurface.OsResource) || m_histogramDebug))
    {
        m_histogramStatisticsSummary = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
            "Histogram internal buffer",
            resourceInternalReadWriteCache,
            lockableVideoMem,
            true,
            0,
            false);

        if (m_histogramStatisticsSummary == nullptr ||
            m_allocator->ResourceIsNull(&m_histogramStatisticsSummary->OsResource))
        {
            DECODE_ASSERTMESSAGE("Failed to allocate hsitogram internal m_histogramStatisticsSummary buffer!");
        }

        DECODE_CHK_NULL(m_histogramStatisticsSummary);
    }

    if ((m_histogramMataDataStreamOut == nullptr) &&
        (!m_allocator->ResourceIsNull(&decodeParams->m_histogramSurface.OsResource) || m_histogramDebug))
    {
        m_histogramMataDataStreamOut = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
            "Histogram internal buffer",
            resourceInternalReadWriteCache,
            lockableVideoMem,
            true,
            0,
            false);

        if (m_histogramMataDataStreamOut == nullptr ||
            m_allocator->ResourceIsNull(&m_histogramMataDataStreamOut->OsResource))
        {
            DECODE_ASSERTMESSAGE("Failed to allocate hsitogram internal m_histogramMataDataStreamOut buffer!");
        }

        DECODE_CHK_NULL(m_histogramMataDataStreamOut);
    }

    if ((m_histogramMataDataStreamIn == nullptr) &&
        (!m_allocator->ResourceIsNull(&decodeParams->m_histogramSurface.OsResource) || m_histogramDebug))
    {
        m_histogramMataDataStreamIn = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
            "Histogram internal buffer",
            resourceInternalReadWriteCache,
            lockableVideoMem,
            true,
            0,
            false);

        if (m_histogramMataDataStreamIn == nullptr ||
            m_allocator->ResourceIsNull(&m_histogramMataDataStreamIn->OsResource))
        {
            DECODE_ASSERTMESSAGE("Failed to allocate hsitogram internal m_histogramMataDataStreamIn buffer!");
        }

        DECODE_CHK_NULL(m_histogramMataDataStreamIn);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DownSamplingFeatureXe3_Lpm_Base::DumpSfcOutputs(CodechalDebugInterface *debugInterface)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(debugInterface);
    DECODE_CHK_NULL(m_basicFeature);

    DecodeDownSamplingFeature::DumpSfcOutputs(debugInterface);

    // Dump histogram U & V
    if ((m_histogramDestSurfU != nullptr || m_histogramDebug) &&
        m_histogramBufferU != nullptr &&
        !m_allocator->ResourceIsNull(&m_histogramBufferU->OsResource))
    {
        CODECHAL_DEBUG_TOOL(
            debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                &m_histogramBufferU->OsResource,
                CodechalDbgAttr::attrSfcHistogram,
                "_DEC_U",
                HISTOGRAM_BINCOUNT * m_histogramBinWidth));)
    }

    if ((m_histogramDestSurfV != nullptr || m_histogramDebug) &&
        m_histogramBufferV != nullptr &&
        !m_allocator->ResourceIsNull(&m_histogramBufferV->OsResource))
    {
        CODECHAL_DEBUG_TOOL(
            debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                &m_histogramBufferV->OsResource,
                CodechalDbgAttr::attrSfcHistogram,
                "_DEC_V",
                HISTOGRAM_BINCOUNT * m_histogramBinWidth));)
    }

    return MOS_STATUS_SUCCESS;
}


}  // namespace decode

#endif
