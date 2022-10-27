/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_vp8_basic_feature.cpp
//! \brief    Defines the common interface for decode vp8 parameter
//!

#include "decode_vp8_basic_feature.h"
#include "decode_utils.h"
#include "decode_allocator.h"
#include "decode_resource_auto_lock.h"
#include <iostream>

namespace decode
{
Vp8BasicFeature::Vp8BasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) : DecodeBasicFeature(allocator, hwInterface, osInterface)
{
    if (osInterface != nullptr)
    {
        m_osInterface = osInterface;
    }

    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(&m_vp8FrameHead, sizeof(m_vp8FrameHead));
    MOS_ZeroMemory(&m_resCoefProbBufferExternal, sizeof(m_resCoefProbBufferExternal));
}

Vp8BasicFeature::~Vp8BasicFeature()
{
    if (m_allocator != nullptr)
    {
        m_allocator->Destroy(m_resCoefProbBufferInternal);
    }
}

MOS_STATUS Vp8BasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(setting);

    DECODE_CHK_STATUS(DecodeBasicFeature::Init(setting));

    DECODE_CHK_STATUS(m_refFrames.Init(this, *m_allocator));

    CodechalSetting *codecSettings  = (CodechalSetting *)setting;
    m_shortFormatInUse              = codecSettings->shortFormatInUse ? true : false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8BasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(params);
    DECODE_CHK_STATUS(DecodeBasicFeature::Update(params));

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;
    m_coefProbSize                     = decodeParams->m_coefProbSize;

    m_vp8PicParams                     = static_cast<PCODEC_VP8_PIC_PARAMS>(decodeParams->m_picParams);
    m_vp8SliceParams                   = static_cast<PCODEC_VP8_SLICE_PARAMS>(decodeParams->m_sliceParams);
    m_vp8IqMatrixParams                = static_cast<PCODEC_VP8_IQ_MATRIX_PARAMS>(decodeParams->m_iqMatrixBuffer);
    DECODE_CHK_NULL(m_vp8IqMatrixParams);

    DECODE_CHK_STATUS(SetPictureStructs(decodeParams));

    DECODE_CHK_STATUS(m_refFrames.UpdatePicture(*m_vp8PicParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp8BasicFeature::SetPictureStructs(CodechalDecodeParams *decodeParams)
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_CHK_NULL(decodeParams->m_destSurface);
    DECODE_CHK_NULL(decodeParams->m_dataBuffer);

    m_coefProbSize              = decodeParams->m_coefProbSize;
    m_LastRefSurface            = decodeParams->m_presNoneRegLastRefFrame;
    m_AltRefSurface             = decodeParams->m_presNoneRegAltRefFrame;
    m_GoldenRefSurface          = decodeParams->m_presNoneRegGoldenRefFrame;
    m_bitstreamLockingInUse     = decodeParams->m_bitstreamLockingInUse;

    DECODE_CHK_NULL(m_vp8PicParams);

    m_deblockingEnabled = !m_vp8PicParams->LoopFilterDisable ? true : false;

    if (decodeParams->m_bitstreamLockingInUse)
    {
        DECODE_CHK_STATUS(AllocateCoefProbBuffer());
        if (decodeParams->m_bitstreamLockable)
        {
            ResourceAutoLock resLock(m_allocator, &m_resDataBuffer.OsResource);
            auto             bitstreamBuffer = (uint8_t *)resLock.LockResourceForRead();

            DECODE_CHK_NULL(bitstreamBuffer);

            DECODE_CHK_STATUS(ParseFrameHead(bitstreamBuffer + m_dataOffset, m_dataSize));
        }
        else
        {
            // in some case, need to do bitstream copy, skipped for no usage now.
            DECODE_ASSERTMESSAGE("Not support VP8 bitstream without Lockable!")
        }

        decodeParams->m_coefProbSize = sizeof(m_vp8FrameHead.FrameContext.CoefProbs);
    }
    else
    {
        m_resCoefProbBufferExternal = *(decodeParams->m_coefProbBuffer);
    }

    return eStatus;
}

MOS_STATUS Vp8BasicFeature::AllocateCoefProbBuffer()
{
    DECODE_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Coef Prob
    if (m_resCoefProbBufferInternal == nullptr)
    {
        m_resCoefProbBufferInternal = m_allocator->AllocateBuffer(
            m_coefProbSize, "VP8_Coef_Prob", resourceInternalReadWriteCache, lockableVideoMem);
    }
    else
    {
        DECODE_CHK_STATUS(m_allocator->Resize(
            m_resCoefProbBufferInternal, m_coefProbSize, lockableVideoMem));
    }

    return eStatus;
}

MOS_STATUS Vp8BasicFeature::ParseFrameHead(uint8_t *bitstreamBuffer, uint32_t bitstreamBufferSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(bitstreamBuffer);

    m_vp8EntropyState.Initialize(&m_vp8FrameHead, bitstreamBuffer, bitstreamBufferSize);

    eStatus = m_vp8EntropyState.ParseFrameHead(m_vp8PicParams);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        DECODE_ASSERTMESSAGE("Fail to parse VP8 Frame Head");
        return eStatus;
    }

    // Loop Filter
    for (int32_t i = 0; i < VP8_MAX_MB_SEGMENTS; i++)
    {
        int32_t segmentLvl = m_vp8PicParams->ucFilterLevel;

        if (m_vp8PicParams->segmentation_enabled)
        {
            if (m_vp8PicParams->mb_segement_abs_delta == 1)
            {
                m_vp8PicParams->ucLoopFilterLevel[i] = segmentLvl = m_vp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_LF][i];
            }
            else
            {
                segmentLvl += m_vp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_LF][i];
                m_vp8PicParams->ucLoopFilterLevel[i] = segmentLvl = (segmentLvl > 0) ? ((segmentLvl > 63) ? 63 : segmentLvl) : 0;
            }
        }
        else
        {
            m_vp8PicParams->ucLoopFilterLevel[i] = m_vp8PicParams->ucFilterLevel;
        }
    }

    // Quant Matrix
    int32_t vp8QIndex[VP8_MAX_MB_SEGMENTS];
    if (m_vp8PicParams->segmentation_enabled)
    {
        for (int32_t i = 0; i < 4; i++)
        {
            if (m_vp8PicParams->mb_segement_abs_delta == 1)
            {
                vp8QIndex[i] = (int32_t)m_vp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_Q][i];
            }
            else
            {
                vp8QIndex[i] = (int32_t)m_vp8PicParams->ucBaseQIndex + (int32_t)m_vp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_Q][i];
                vp8QIndex[i] = (vp8QIndex[i] >= 0) ? ((vp8QIndex[i] <= VP8_MAX_Q) ? vp8QIndex[i] : VP8_MAX_Q) : 0;    // Clamp to valid range
            }
        }
    }
    else
    {
        vp8QIndex[0] = (int32_t)m_vp8PicParams->ucBaseQIndex;
        vp8QIndex[1] = 0;
        vp8QIndex[2] = 0;
        vp8QIndex[3] = 0;
    }

    m_vp8EntropyState.FrameHeadQuantUpdate(m_vp8PicParams);

    m_vp8IqMatrixParams->quantization_values[0][0] = m_vp8FrameHead.Y1DeQuant[vp8QIndex[0]][0];
    m_vp8IqMatrixParams->quantization_values[0][1] = m_vp8FrameHead.Y1DeQuant[vp8QIndex[0]][1];
    m_vp8IqMatrixParams->quantization_values[0][2] = m_vp8FrameHead.UVDeQuant[vp8QIndex[0]][0];
    m_vp8IqMatrixParams->quantization_values[0][3] = m_vp8FrameHead.UVDeQuant[vp8QIndex[0]][1];
    m_vp8IqMatrixParams->quantization_values[0][4] = m_vp8FrameHead.Y2DeQuant[vp8QIndex[0]][0];
    m_vp8IqMatrixParams->quantization_values[0][5] = m_vp8FrameHead.Y2DeQuant[vp8QIndex[0]][1];

    if (m_vp8FrameHead.u8SegmentationEnabled)
    {
        for (int32_t i = 1; i < 4; i++)
        {
            m_vp8IqMatrixParams->quantization_values[i][0] = m_vp8FrameHead.Y1DeQuant[vp8QIndex[i]][0];
            m_vp8IqMatrixParams->quantization_values[i][1] = m_vp8FrameHead.Y1DeQuant[vp8QIndex[i]][1];
            m_vp8IqMatrixParams->quantization_values[i][2] = m_vp8FrameHead.UVDeQuant[vp8QIndex[i]][0];
            m_vp8IqMatrixParams->quantization_values[i][3] = m_vp8FrameHead.UVDeQuant[vp8QIndex[i]][1];
            m_vp8IqMatrixParams->quantization_values[i][4] = m_vp8FrameHead.Y2DeQuant[vp8QIndex[i]][0];
            m_vp8IqMatrixParams->quantization_values[i][5] = m_vp8FrameHead.Y2DeQuant[vp8QIndex[i]][1];
        }
    }
    else
    {
        for (int32_t i = 1; i < 4; i++)
        {
            for (int32_t j = 0; j < 6; j++)
            {
                m_vp8IqMatrixParams->quantization_values[i][j] = 0;
            }
        }
    }

    ResourceAutoLock resLock(m_allocator, &m_resCoefProbBufferInternal->OsResource);
    auto             data = (uint8_t *)resLock.LockResourceForWrite();

    DECODE_CHK_NULL(data);

    MOS_SecureMemcpy(
        data,
        m_resCoefProbBufferInternal->size,
        (void *)&(m_vp8FrameHead.FrameContext.CoefProbs),
        sizeof(m_vp8FrameHead.FrameContext.CoefProbs));

    m_vp8FrameHead.bNotFirstCall = true;

    return eStatus;
}

MOS_STATUS Vp8BasicFeature::SetRequiredBitstreamSize(uint32_t requiredSize)
{
    DECODE_FUNC_CALL();

    if (requiredSize > m_dataSize)
    {
        m_dataOffset = 0;
        m_dataSize   = MOS_ALIGN_CEIL(requiredSize, MHW_CACHELINE_SIZE);
    }

    DECODE_NORMALMESSAGE("Estimate bitstream size in this Frame: %u", requiredSize);
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode
