/*
* Copyright (c) 2020-2025, Intel Corporation
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
//! \file     decode_downsampling_feature.cpp
//! \brief    Defines the common interface for decode downsampling features
//! \details  The decode downsampling feature interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#include "decode_downsampling_feature.h"
#include "decode_utils.h"
#include "codechal_debug.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
DecodeDownSamplingFeature::DecodeDownSamplingFeature(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface) :
    m_osInterface(osInterface), m_allocator(allocator)
{
    m_featureManager = featureManager;
}

DecodeDownSamplingFeature::~DecodeDownSamplingFeature()
{
    FreeHistogramBuffer();
}

MOS_STATUS DecodeDownSamplingFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_internalTargets.Init(*m_allocator));

    m_basicFeature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    MOS_ZeroMemory(&m_outputSurface, sizeof(m_outputSurface));

    DECODE_CHK_NULL(m_osInterface);
    m_userSettingPtr = m_osInterface->pfnGetUserSettingInstance(m_osInterface);

#if (_DEBUG || _RELEASE_INTERNAL)
    m_histogramDebug = ReadUserFeature(m_userSettingPtr, "Decode Histogram Debug", MediaUserSetting::Group::Sequence).Get<bool>();
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeDownSamplingFeature::Update(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;

    if (decodeParams->m_procParams == nullptr)
    {
        m_inputSurface               = nullptr;
        m_histogramDestSurf          = nullptr;
        m_histogramBuffer            = nullptr;
        m_histogramDestSurfU         = nullptr;
        m_histogramBufferU           = nullptr;
        m_histogramDestSurfV         = nullptr;
        m_histogramBufferV           = nullptr;
        m_histogramStatisticsSummary = nullptr;
        m_histogramMetaDataStreamOut = nullptr;
        m_histogramMetaDataStreamIn  = nullptr;
        m_enabled                    = false;

        return MOS_STATUS_SUCCESS;
    }
    else
    {
        m_enabled = true;
    }

    DecodeProcessingParams *procParams = (DecodeProcessingParams *)decodeParams->m_procParams;

    m_chromaSitingType             = procParams->m_chromaSitingType;
    m_rotationState                = procParams->m_rotationState;
    m_blendState                   = procParams->m_blendState;
    m_mirrorState                  = procParams->m_mirrorState;
    m_scalingMode                  = procParams->m_scalingMode;
    m_isReferenceOnlyPattern       = procParams->m_isReferenceOnlyPattern;

    if (m_isReferenceOnlyPattern)
    {
        m_enabled = false;
        m_inputSurface = procParams->m_inputSurface;
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(procParams->m_outputSurface);
    m_outputSurface = *(procParams->m_outputSurface);
    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&m_outputSurface));

    m_outputSurfaceRegion.m_x      = procParams->m_outputSurfaceRegion.m_x;
    m_outputSurfaceRegion.m_y      = procParams->m_outputSurfaceRegion.m_y;
    m_outputSurfaceRegion.m_width  = (procParams->m_outputSurfaceRegion.m_width == 0) ?
        m_outputSurface.dwWidth : procParams->m_outputSurfaceRegion.m_width;
    m_outputSurfaceRegion.m_height = (procParams->m_outputSurfaceRegion.m_height == 0) ?
        m_outputSurface.dwHeight : procParams->m_outputSurfaceRegion.m_height;

    if (procParams->m_inputSurface != nullptr)
    {
        m_inputSurface = procParams->m_inputSurface;
        DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_inputSurface));

        m_inputSurfaceRegion.m_x      = procParams->m_inputSurfaceRegion.m_x;
        m_inputSurfaceRegion.m_y      = procParams->m_inputSurfaceRegion.m_y;
        m_inputSurfaceRegion.m_width  = (procParams->m_inputSurfaceRegion.m_width == 0) ?
            m_inputSurface->dwWidth : procParams->m_inputSurfaceRegion.m_width;
        m_inputSurfaceRegion.m_height = (procParams->m_inputSurfaceRegion.m_height == 0) ?
            m_inputSurface->dwHeight : procParams->m_inputSurfaceRegion.m_height;
    }
    else
    {
        if (m_basicFeature->m_curRenderPic.FrameIdx >= decodeParams->m_refFrameCnt)
        {
            DECODE_ASSERTMESSAGE("Invalid Downsampling Reference Frame Index !");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        DECODE_CHK_STATUS(UpdateInternalTargets(*m_basicFeature));

        m_inputSurface = m_internalTargets.GetCurSurf();
        DECODE_CHK_NULL(m_inputSurface);

        m_inputSurfaceRegion.m_x      = 0;
        m_inputSurfaceRegion.m_y      = 0;
        m_inputSurfaceRegion.m_width  = (procParams->m_inputSurfaceRegion.m_width == 0 || procParams->m_inputSurfaceRegion.m_width > m_basicFeature->m_width) ?
            m_basicFeature->m_width : procParams->m_inputSurfaceRegion.m_width;
        m_inputSurfaceRegion.m_height = (procParams->m_inputSurfaceRegion.m_height == 0 || procParams->m_inputSurfaceRegion.m_height > m_basicFeature->m_height) ?
            m_basicFeature->m_height : procParams->m_inputSurfaceRegion.m_height;
    }

    // Histogram
    if (!m_allocator->ResourceIsNull(&decodeParams->m_histogramSurface.OsResource) || m_histogramDebug)
    {
        m_histogramDestSurf = &decodeParams->m_histogramSurface;
        m_histogramBuffer   = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx, HistogramY);
        DECODE_CHK_NULL(m_histogramBuffer);
    }
    else
    {
        m_histogramDestSurf = nullptr;
        m_histogramBuffer   = nullptr;
    }

    // Histogram U plane
    if (!m_allocator->ResourceIsNull(&decodeParams->m_histogramSurfaceU.OsResource) || m_histogramDebug)
    {
        m_histogramDestSurfU = &decodeParams->m_histogramSurfaceU;
        m_histogramBufferU   = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx, HistogramU);
        DECODE_CHK_NULL(m_histogramBufferU);
    }
    else
    {
        m_histogramDestSurfU = nullptr;
        m_histogramBufferU   = nullptr;
    }

    // Histogram V plane
    if (!m_allocator->ResourceIsNull(&decodeParams->m_histogramSurfaceV.OsResource) || m_histogramDebug)
    {
        m_histogramDestSurfV = &decodeParams->m_histogramSurfaceV;
        m_histogramBufferV   = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx, HistogramV);
        DECODE_CHK_NULL(m_histogramBufferV);
    }
    else
    {
        m_histogramDestSurfV = nullptr;
        m_histogramBufferV   = nullptr;
    }

    if (m_osInterface && MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrVDAQMHistogram) &&
        (m_histogramBuffer != nullptr || m_histogramBufferU != nullptr || m_histogramBufferV != nullptr))
    {
        m_aqmHistogramEnable = true;
    }
    else
    {
        m_aqmHistogramEnable = false;
    }

    if (m_aqmHistogramEnable)
    {
        m_histogramStatisticsSummary = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx, HistogramStatistics);
        DECODE_CHK_NULL(m_histogramStatisticsSummary);
        m_histogramMetaDataStreamIn = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx, HistogramStreamIn);
        DECODE_CHK_NULL(m_histogramMetaDataStreamIn);
        m_histogramMetaDataStreamOut = AllocateHistogramBuffer(m_basicFeature->m_curRenderPic.FrameIdx, HistogramStreamOut);
        DECODE_CHK_NULL(m_histogramMetaDataStreamOut);
    }

    // Update decode output in basic feature
    if (!m_aqmHistogramEnable)
    {
        DECODE_CHK_STATUS(UpdateDecodeTarget(*m_inputSurface));
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_outputSurfaceList[m_basicFeature->m_curRenderPic.FrameIdx] = m_outputSurface;
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeDownSamplingFeature::UpdateInternalTargets(DecodeBasicFeature &basicFeature)
{
    DECODE_FUNC_CALL();

    uint32_t curFrameIdx = basicFeature.m_curRenderPic.FrameIdx;

    std::vector<uint32_t> refFrameList;
    DECODE_CHK_STATUS(GetRefFrameList(refFrameList));
    DECODE_CHK_STATUS(m_internalTargets.UpdateRefList(curFrameIdx, refFrameList));

    MOS_SURFACE surface;
    MOS_ZeroMemory(&surface, sizeof(surface));
    DECODE_CHK_STATUS(GetDecodeTargetSize(surface.dwWidth, surface.dwHeight));
    DECODE_CHK_STATUS(GetDecodeTargetFormat(surface.Format));
    DECODE_CHK_STATUS(m_internalTargets.ActiveCurSurf(
        curFrameIdx, &surface, basicFeature.IsMmcEnabled(), resourceOutputPicture, notLockableVideoMem));

    return MOS_STATUS_SUCCESS;
}

PMOS_BUFFER DecodeDownSamplingFeature::AllocateHistogramBuffer(uint8_t frameIndex, HistogramBufferType bufferType)
{
    DECODE_FUNC_CALL();

    if (frameIndex >= DecodeBasicFeature::m_maxFrameIndex)
    {
        return nullptr;
    }

    switch (bufferType)
    {
    case HistogramY:
        if (m_histogramBufferList[frameIndex] == nullptr)
        {
            auto histogramBuffer = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram internal buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (histogramBuffer == nullptr ||
                m_allocator->ResourceIsNull(&histogramBuffer->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram internal buffer!");
            }

            m_histogramBufferList[frameIndex] = histogramBuffer;

            return m_histogramBufferList[frameIndex];
        }
        break;
    case HistogramU:
        if (m_histogramBufferUList[frameIndex] == nullptr)
        {
            auto histogramBufferU = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram U-Plane buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (histogramBufferU == nullptr ||
                m_allocator->ResourceIsNull(&histogramBufferU->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram U-Plane internal buffer!");
            }

            m_histogramBufferUList[frameIndex] = histogramBufferU;

            return m_histogramBufferUList[frameIndex];
        }
        break;
    case HistogramV:
        if (m_histogramBufferVList[frameIndex] == nullptr)
        {
            auto histogramBufferV = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram V-Plane buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (histogramBufferV == nullptr ||
                m_allocator->ResourceIsNull(&histogramBufferV->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram V-Plane internal buffer!");
            }

            m_histogramBufferVList[frameIndex] = histogramBufferV;

            return m_histogramBufferVList[frameIndex];
        }
        break;
    case HistogramStatistics:
        if (m_histogramBufferStatisticsSummaryList[frameIndex] == nullptr)
        {
            auto histogramBufferStatistics = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram statistics summary buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (histogramBufferStatistics == nullptr ||
                m_allocator->ResourceIsNull(&histogramBufferStatistics->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram statistics internal buffer!");
            }

            m_histogramBufferStatisticsSummaryList[frameIndex] = histogramBufferStatistics;

            return m_histogramBufferStatisticsSummaryList[frameIndex];
        }
        break;
    case HistogramStreamIn:
        if (m_histogramBufferMetaDataStreamInList[frameIndex] == nullptr)
        {
            auto histogramBufferStreamIn = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram stream in buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (histogramBufferStreamIn == nullptr ||
                m_allocator->ResourceIsNull(&histogramBufferStreamIn->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram StreamIn internal buffer!");
            }

            m_histogramBufferMetaDataStreamInList[frameIndex] = histogramBufferStreamIn;

            return m_histogramBufferMetaDataStreamInList[frameIndex];
        }
        break;
    case HistogramStreamOut:
        if (m_histogramBufferMetaDataStreamOutList[frameIndex] == nullptr)
        {
            auto histogramBufferStreamOut = m_allocator->AllocateBuffer(HISTOGRAM_BINCOUNT * m_histogramBinWidth,
                "Histogram stream out buffer",
                resourceInternalReadWriteCache,
                lockableVideoMem,
                true,
                0,
                false);

            if (histogramBufferStreamOut == nullptr ||
                m_allocator->ResourceIsNull(&histogramBufferStreamOut->OsResource))
            {
                DECODE_ASSERTMESSAGE("Failed to allocate hsitogram StreamOut internal buffer!");
            }

            m_histogramBufferMetaDataStreamOutList[frameIndex] = histogramBufferStreamOut;

            return m_histogramBufferMetaDataStreamOutList[frameIndex];
        }
        break;
    default:
        DECODE_ASSERTMESSAGE("Failed to allocate hsitogram buffer due to invalid buffer type %d !", bufferType);
        break;
    }

    return nullptr;
}

void DecodeDownSamplingFeature::FreeHistogramBuffer()
{
    DECODE_FUNC_CALL();

    PMOS_BUFFER *histogramBufferList = nullptr;

    for (int k = HistogramY; k < HistogramStreamMax; k++)
    {
        switch (k)
        {
        case HistogramY:
            histogramBufferList = m_histogramBufferList;
            break;
        case HistogramU:
            histogramBufferList = m_histogramBufferUList;
            break;
        case HistogramV:
            histogramBufferList = m_histogramBufferVList;
            break;
        case HistogramStatistics:
            histogramBufferList = m_histogramBufferStatisticsSummaryList;
            break;
        case HistogramStreamIn:
            histogramBufferList = m_histogramBufferMetaDataStreamInList;
            break;
        case HistogramStreamOut:
            histogramBufferList = m_histogramBufferMetaDataStreamOutList;
            break;
        default:
            histogramBufferList = nullptr;
            break;
        }

        if (histogramBufferList != nullptr && m_allocator != nullptr)
        {
            for (auto i = 0; i < DecodeBasicFeature::m_maxFrameIndex; i++)
            {
                MOS_BUFFER *histogramBuffer = histogramBufferList[i];
                if (histogramBuffer == nullptr ||
                    m_allocator->ResourceIsNull(&histogramBuffer->OsResource))
                {
                    continue;
                }
                MOS_STATUS eStatus = m_allocator->Destroy(histogramBuffer);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    DECODE_ASSERTMESSAGE("Failed to free histogram internal buffer!");
                }
            }
        }
    }
}

MOS_STATUS DecodeDownSamplingFeature::DumpSfcOutputs(CodechalDebugInterface* debugInterface)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(debugInterface);
    DECODE_CHK_NULL(m_basicFeature);

    // Dump histogram
    if ((m_histogramDestSurf != nullptr || m_histogramDebug) &&
        m_histogramBuffer != nullptr &&
        !m_allocator->ResourceIsNull(&m_histogramBuffer->OsResource))
    {
        CODECHAL_DEBUG_TOOL(
            debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                &m_histogramBuffer->OsResource,
                m_aqmHistogramEnable ? CodechalDbgAttr::attrAqmHistogram : CodechalDbgAttr::attrSfcHistogram,
                "_DEC_Hist",
                HISTOGRAM_BINCOUNT * m_histogramBinWidth));)
    }

    // Dump histogram U & V
    if ((m_histogramDestSurfU != nullptr || m_histogramDebug) &&
        m_histogramBufferU != nullptr &&
        !m_allocator->ResourceIsNull(&m_histogramBufferU->OsResource))
    {
        CODECHAL_DEBUG_TOOL(
            debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
            DECODE_CHK_STATUS(debugInterface->DumpBuffer(
                &m_histogramBufferU->OsResource,
                m_aqmHistogramEnable ? CodechalDbgAttr::attrAqmHistogram : CodechalDbgAttr::attrSfcHistogram,
                "_DEC_Hist_U",
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
                m_aqmHistogramEnable ? CodechalDbgAttr::attrAqmHistogram : CodechalDbgAttr::attrSfcHistogram,
                "_DEC_Hist_V",
                HISTOGRAM_BINCOUNT * m_histogramBinWidth));)
    }

    // Dump SFC
    if (!m_allocator->ResourceIsNull(&m_outputSurface.OsResource) &&
        m_inputSurface != nullptr &&
        !m_aqmHistogramEnable)
    {
        CODECHAL_DEBUG_TOOL(
            debugInterface->m_bufferDumpFrameNum = m_basicFeature->m_frameNum;
            DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                &m_outputSurface,
                CodechalDbgAttr::attrSfcOutputSurface,
                "_SFCSurf"));)
    }

    return MOS_STATUS_SUCCESS;
}
}

#endif
