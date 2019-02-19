/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_decode_vp8.cpp
//! \brief    Implements the decode interface extension for VP8.
//! \details  Implements all functions required by CodecHal for VP8 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_vp8.h"
#include "codec_def_vp8_probs.h"
#include "codechal_mmc_decode_vp8.h"
#include "hal_oca_interface.h"
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

void Vp8EntropyState::DecodeFill()
{
    int32_t        shift       = m_bdValueSize - 8 - (m_count + 8);
    uint32_t       bytesLeft   = (uint32_t)(m_bufferEnd - m_buffer);
    uint32_t       bitsLeft   = bytesLeft * CHAR_BIT;
    int32_t        num         = (int32_t)(shift + CHAR_BIT - bitsLeft);
    int32_t        loopEnd     = 0;

    if (num >= 0)
    {
        m_count += m_lotsOfBits;
        loopEnd = num;
    }

    if (num < 0 || bitsLeft)
    {
        while (shift >= loopEnd)
        {
            m_count += CHAR_BIT;
            m_value |= (uint32_t)*m_buffer << shift;
            ++m_buffer;
            shift -= CHAR_BIT;
        }
    }
}

uint32_t Vp8EntropyState::DecodeBool(int32_t probability)
{
    uint32_t split     = 1 + (((m_range - 1) * probability) >> 8);
    uint32_t bigSplit  = (uint32_t)split << (m_bdValueSize - 8);
    uint32_t origRange = m_range;
    m_range            = split;

    uint32_t bit = 0;
    if (m_value >= bigSplit)
    {
        m_range = origRange - split;
        m_value = m_value - bigSplit;
        bit = 1;
    }

    int32_t shift = Norm[m_range];
    m_range <<= shift;
    m_value <<= shift;
    m_count -= shift;

    if (m_count < 0)
    {
        DecodeFill();
    }

    return bit;
}

int32_t Vp8EntropyState::DecodeValue(int32_t bits)
{
    int32_t retValue = 0;

    for (int32_t iBit = bits - 1; iBit >= 0; iBit--)
    {
        retValue |= (DecodeBool(0x80) << iBit);
    }

    return retValue;
}

void Vp8EntropyState::ParseFrameHeadInit()
{
    if (m_frameHead->iFrameType == m_keyFrame)
    {
        MOS_SecureMemcpy(m_frameHead->FrameContext.MvContext, sizeof(DefaultMvContext), DefaultMvContext, sizeof(DefaultMvContext));
        MOS_SecureMemcpy(m_frameHead->FrameContext.YModeProb, sizeof(KfYModeProb), KfYModeProb, sizeof(KfYModeProb));
        MOS_SecureMemcpy(m_frameHead->FrameContext.UVModeProb, sizeof(KfUVModeProb), KfUVModeProb, sizeof(KfUVModeProb));
        MOS_SecureMemcpy(m_frameHead->FrameContext.CoefProbs, sizeof(DefaultCoefProbs), DefaultCoefProbs, sizeof(DefaultCoefProbs));

        MOS_SecureMemcpy(m_frameHead->YModeProbs, sizeof(KfYModeProb), KfYModeProb, sizeof(KfYModeProb));
        MOS_SecureMemcpy(m_frameHead->UVModeProbs, sizeof(KfUVModeProb), KfUVModeProb, sizeof(KfUVModeProb));
        MOS_SecureMemcpy(m_frameHead->YModeProbs, sizeof(YModeProb), YModeProb, sizeof(YModeProb));
        MOS_SecureMemcpy(m_frameHead->UVModeProbs, sizeof(UVModeProb), UVModeProb, sizeof(UVModeProb));

        memset(m_frameHead->SegmentFeatureData, 0, sizeof(m_frameHead->SegmentFeatureData));
        m_frameHead->u8MbSegementAbsDelta = 0;

        memset(m_frameHead->RefLFDeltas, 0, sizeof(m_frameHead->RefLFDeltas));
        memset(m_frameHead->ModeLFDeltas, 0, sizeof(m_frameHead->ModeLFDeltas));

        m_frameHead->iRefreshGoldenFrame = 1;
        m_frameHead->iRefreshAltFrame    = 1;
        m_frameHead->iCopyBufferToGolden = 0;
        m_frameHead->iCopyBufferToAlt    = 0;

        m_frameHead->iLastFrameBufferCurrIdx   = m_frameHead->iNewFrameBufferIdx;
        m_frameHead->iGoldenFrameBufferCurrIdx = m_frameHead->iNewFrameBufferIdx;
        m_frameHead->iAltFrameBufferCurrIdx    = m_frameHead->iNewFrameBufferIdx;

        m_frameHead->RefFrameSignBias[VP8_GOLDEN_FRAME] = 0;
        m_frameHead->RefFrameSignBias[VP8_ALTREF_FRAME] = 0;
    }
}

int32_t Vp8EntropyState::StartEntropyDecode()
{
    m_bufferEnd = m_dataBufferEnd;
    m_buffer    = m_dataBuffer;
    m_value     = 0;
    m_count     = -8;
    m_range     = 255;

    if ((m_bufferEnd - m_buffer) > 0 && m_buffer == nullptr)
    {
        return 1;
    }

    DecodeFill();

    return 0;
}

void Vp8EntropyState::SegmentationEnabled()
{
    m_frameHead->u8SegmentationEnabled = (uint8_t)DecodeBool(m_probHalf);

    if (m_frameHead->u8SegmentationEnabled)
    {
        m_frameHead->u8UpdateMbSegmentationMap  = (uint8_t)DecodeBool(m_probHalf);
        m_frameHead->u8UpdateMbSegmentationData = (uint8_t)DecodeBool(m_probHalf);

        if (m_frameHead->u8UpdateMbSegmentationData)
        {
            m_frameHead->u8MbSegementAbsDelta = (uint8_t)DecodeBool(m_probHalf);

            memset(m_frameHead->SegmentFeatureData, 0, sizeof(m_frameHead->SegmentFeatureData));

            for (int32_t i = 0; i < VP8_MB_LVL_MAX; i++)
            {
                for (int32_t j = 0; j < VP8_MAX_MB_SEGMENTS; j++)
                {
                    if (DecodeBool(m_probHalf))
                    {
                        m_frameHead->SegmentFeatureData[i][j] = (int8_t)DecodeValue(MbFeatureDataBits[i]);

                        if (DecodeBool(m_probHalf))
                        {
                            m_frameHead->SegmentFeatureData[i][j] = -m_frameHead->SegmentFeatureData[i][j];
                        }
                    }
                    else
                    {
                        m_frameHead->SegmentFeatureData[i][j] = 0;
                    }
                }
            }
        }

        if (m_frameHead->u8UpdateMbSegmentationMap)
        {
            memset(m_frameHead->MbSegmentTreeProbs, 255, sizeof(m_frameHead->MbSegmentTreeProbs));

            for (int32_t i = 0; i < VP8_MB_SEGMENT_TREE_PROBS; i++)
            {
                if (DecodeBool(m_probHalf))
                {
                    m_frameHead->MbSegmentTreeProbs[i] = (uint8_t)DecodeValue(8);
                }
            }
        }
    }
    else
    {
        m_frameHead->u8UpdateMbSegmentationMap  = 0;
        m_frameHead->u8UpdateMbSegmentationData = 0;
    }
}

void Vp8EntropyState::LoopFilterInit(int32_t defaultFilterLvl)
{
    for (int32_t segmentNum = 0; segmentNum < VP8_MAX_MB_SEGMENTS; segmentNum++)
    {
        int32_t segmentLvl = defaultFilterLvl;

        if (m_frameHead->u8SegmentationEnabled)
        {
            if (m_frameHead->u8MbSegementAbsDelta == 1)
            {
                m_frameHead->LoopFilterLevel[segmentNum] = segmentLvl = m_frameHead->SegmentFeatureData[VP8_MB_LVL_ALT_LF][segmentNum];
            }
            else
            {
                segmentLvl += m_frameHead->SegmentFeatureData[VP8_MB_LVL_ALT_LF][segmentNum];
                m_frameHead->LoopFilterLevel[segmentNum] = segmentLvl = (segmentLvl > 0) ? ((segmentLvl > 63) ? 63 : segmentLvl) : 0;
            }
        }
    }
}

void Vp8EntropyState::LoopFilterEnabled()
{
    m_frameHead->FilterType      = (VP8_LF_TYPE)DecodeBool(m_probHalf);
    m_frameHead->iFilterLevel    = DecodeValue(6);
    m_frameHead->iSharpnessLevel = DecodeValue(3);

    m_frameHead->u8ModeRefLfDeltaUpdate  = 0;
    m_frameHead->u8ModeRefLfDeltaEnabled = (uint8_t)DecodeBool(m_probHalf);

    if (m_frameHead->u8ModeRefLfDeltaEnabled)
    {
        m_frameHead->u8ModeRefLfDeltaUpdate = (uint8_t)DecodeBool(m_probHalf);

        if (m_frameHead->u8ModeRefLfDeltaUpdate)
        {
            for (int32_t i = 0; i < VP8_MAX_REF_LF_DELTAS; i++)
            {
                if (DecodeBool(m_probHalf))
                {
                    m_frameHead->RefLFDeltas[i] = (int8_t)DecodeValue(6);

                    if (DecodeBool(m_probHalf))
                    {
                        m_frameHead->RefLFDeltas[i] = m_frameHead->RefLFDeltas[i] * (-1);
                    }
                }
            }

            for (int32_t i = 0; i < VP8_MAX_MODE_LF_DELTAS; i++)
            {
                if (DecodeBool(m_probHalf))
                {
                    m_frameHead->ModeLFDeltas[i] = (int8_t)DecodeValue(6);

                    if (DecodeBool(m_probHalf))
                    {
                        m_frameHead->ModeLFDeltas[i] = m_frameHead->ModeLFDeltas[i] * (-1);
                    }
                }
            }
        }
    }

    if (m_frameHead->iFilterLevel)
    {
        LoopFilterInit(m_frameHead->iFilterLevel);
    }
}

int32_t Vp8EntropyState::GetDeltaQ(int32_t prevVal, int32_t *qupdate)
{
    int32_t retVal = 0;

    if (DecodeBool(m_probHalf))
    {
        retVal = DecodeValue(4);

        if (DecodeBool(m_probHalf))
        {
            retVal = -retVal;
        }
    }

    if (retVal != prevVal)
    {
        *qupdate = 1;
    }

    return retVal;
}

int32_t Vp8EntropyState::DcQuant(int32_t qindex, int32_t delta)
{
    int32_t retVal;

    qindex = qindex + delta;

    if (qindex > 127)
    {
        qindex = 127;
    }
    else if (qindex < 0)
    {
        qindex = 0;
    }

    retVal = DcQLookup[qindex];
    return retVal;
}

int32_t Vp8EntropyState::Dc2Quant(int32_t qindex, int32_t delta)
{
    int32_t retVal;

    qindex = qindex + delta;

    if (qindex > 127)
    {
        qindex = 127;
    }
    else if (qindex < 0)
    {
        qindex = 0;
    }

    retVal = DcQLookup[qindex] * 2;
    return retVal;
}

int32_t Vp8EntropyState::DcUVQuant(int32_t qindex, int32_t delta)
{
    int32_t retVal;

    qindex = qindex + delta;

    if (qindex > 127)
    {
        qindex = 127;
    }
    else if (qindex < 0)
    {
        qindex = 0;
    }

    retVal = DcQLookup[qindex];

    if (retVal > 132)
    {
        retVal = 132;
    }

    return retVal;
}

int32_t Vp8EntropyState::AcYQuant(int32_t qindex)
{
    int32_t retVal;

    if (qindex > 127)
    {
        qindex = 127;
    }
    else if (qindex < 0)
    {
        qindex = 0;
    }

    retVal = AcQLookup[qindex];
    return retVal;
}

int32_t Vp8EntropyState::Ac2Quant(int32_t qindex, int32_t delta)
{
    int32_t retVal;

    qindex = qindex + delta;

    if (qindex > 127)
    {
        qindex = 127;
    }
    else if (qindex < 0)
    {
        qindex = 0;
    }

    retVal = (AcQLookup[qindex] * 101581) >> 16;

    if (retVal < 8)
    {
        retVal = 8;
    }

    return retVal;
}
int32_t Vp8EntropyState::AcUVQuant(int32_t qindex, int32_t delta)
{
    int32_t retVal;

    qindex = qindex + delta;

    if (qindex > 127)
    {
        qindex = 127;
    }
    else if (qindex < 0)
    {
        qindex = 0;
    }

    retVal = AcQLookup[qindex];
    return retVal;
}

void Vp8EntropyState::QuantInit()
{
    for (int32_t i = 0; i < VP8_Q_INDEX_RANGE; i++)
    {
        m_frameHead->Y1DeQuant[i][0] = (int16_t)DcQuant(i, m_frameHead->iY1DcDeltaQ);
        m_frameHead->Y2DeQuant[i][0] = (int16_t)Dc2Quant(i, m_frameHead->iY2DcDeltaQ);
        m_frameHead->UVDeQuant[i][0] = (int16_t)DcUVQuant(i, m_frameHead->iUVDcDeltaQ);

        m_frameHead->Y1DeQuant[i][1] = (int16_t)AcYQuant(i);
        m_frameHead->Y2DeQuant[i][1] = (int16_t)Ac2Quant(i, m_frameHead->iY2AcDeltaQ);
        m_frameHead->UVDeQuant[i][1] = (int16_t)AcUVQuant(i, m_frameHead->iUVAcDeltaQ);
    }
}

void Vp8EntropyState::QuantSetup()
{
    int32_t qupdate = 0;

    m_frameHead->iBaseQIndex = DecodeValue(7);
    m_frameHead->iY1DcDeltaQ = GetDeltaQ(m_frameHead->iY1DcDeltaQ, &qupdate);
    m_frameHead->iY2DcDeltaQ = GetDeltaQ(m_frameHead->iY2DcDeltaQ, &qupdate);
    m_frameHead->iY2AcDeltaQ = GetDeltaQ(m_frameHead->iY2AcDeltaQ, &qupdate);
    m_frameHead->iUVDcDeltaQ = GetDeltaQ(m_frameHead->iUVDcDeltaQ, &qupdate);
    m_frameHead->iUVAcDeltaQ = GetDeltaQ(m_frameHead->iUVAcDeltaQ, &qupdate);

    if (qupdate)
    {
        QuantInit();
    }
}

void Vp8EntropyState::ReadMvContexts(MV_CONTEXT *mvContext)
{
    int32_t i = 0;

    do
    {
        const uint8_t *upProb = MvUpdateProbs[i].MvProb;
        uint8_t *prob = (uint8_t *)(mvContext + i);
        uint8_t *const stopProb = prob + 19;

        do
        {
            if (DecodeBool(*upProb++))
            {
                const uint8_t x = (uint8_t)DecodeValue(7);

                *prob = x ? x << 1 : 1;
            }
        } while (++prob < stopProb);

    } while (++i < 2);
}

MOS_STATUS Vp8EntropyState::ParseFrameHead(PCODEC_VP8_PIC_PARAMS vp8PicParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    ParseFrameHeadInit();

    StartEntropyDecode();

    if (m_frameHead->iFrameType == m_keyFrame)
    {
        DecodeBool(m_probHalf);  // Color Space
        DecodeBool(m_probHalf);  // Clamp Type
    }

    SegmentationEnabled();

    LoopFilterEnabled();

    m_frameHead->MultiTokenPartition = (VP8_TOKEN_PARTITION)DecodeValue(2);

    if (!m_frameHead->bNotFirstCall)
    {
        QuantInit();
    }

    QuantSetup();

    if (m_frameHead->iFrameType != m_keyFrame)
    {
        m_frameHead->iRefreshGoldenFrame = DecodeBool(m_probHalf);

        m_frameHead->iRefreshAltFrame = DecodeBool(m_probHalf);

        m_frameHead->iCopyBufferToGolden = 0;

        if (!m_frameHead->iRefreshGoldenFrame)
        {
            m_frameHead->iCopyBufferToGolden = DecodeValue(2);
        }

        m_frameHead->iCopyBufferToAlt = 0;

        if (!m_frameHead->iRefreshAltFrame)
        {
            m_frameHead->iCopyBufferToAlt = DecodeValue(2);
        }

        m_frameHead->RefFrameSignBias[VP8_GOLDEN_FRAME] = DecodeBool(m_probHalf);
        m_frameHead->RefFrameSignBias[VP8_ALTREF_FRAME] = DecodeBool(m_probHalf);
    }

    if (m_frameHead->bNotFirstCall && m_frameHead->iRefreshEntropyProbs == 0)
    {
        MOS_SecureMemcpy(&m_frameHead->FrameContext, sizeof(m_frameHead->FrameContext), &m_frameHead->LastFrameContext, sizeof(m_frameHead->LastFrameContext));
    }

    m_frameHead->iRefreshEntropyProbs = DecodeBool(m_probHalf);

    if (m_frameHead->iRefreshEntropyProbs == 0)
    {
        MOS_SecureMemcpy(&m_frameHead->LastFrameContext, sizeof(m_frameHead->LastFrameContext), &m_frameHead->FrameContext, sizeof(m_frameHead->FrameContext));
    }

    if (m_frameHead->iFrameType == m_keyFrame || DecodeBool(m_probHalf))
    {
        m_frameHead->iRefreshLastFrame = true;
    }
    else
    {
        m_frameHead->iRefreshLastFrame = false;
    }

    for (int32_t i = 0; i < VP8_BLOCK_TYPES; i++)
        for (int32_t j = 0; j < VP8_COEF_BANDS; j++)
            for (int32_t k = 0; k < VP8_PREV_COEF_CONTEXTS; k++)
                for (int32_t l = 0; l < VP8_ENTROPY_NODES; l++)
                {
                    uint8_t *const p = m_frameHead->FrameContext.CoefProbs[i][j][k] + l;

                    if (DecodeBool(CoefUpdateProbs[i][j][k][l]))
                    {
                        *p = (uint8_t)DecodeValue(8);
                    }
                }

    m_frameHead->iMbNoCoeffSkip = (int32_t)DecodeBool(m_probHalf);
    m_frameHead->iProbSkipFalse = 0;
    if (m_frameHead->iMbNoCoeffSkip)
    {
        m_frameHead->iProbSkipFalse = (uint8_t)DecodeValue(8);
    }

    if (m_frameHead->iFrameType != m_keyFrame)
    {
        m_frameHead->ProbIntra = (uint8_t)DecodeValue(8);
        m_frameHead->ProbLast  = (uint8_t)DecodeValue(8);
        m_frameHead->ProbGf    = (uint8_t)DecodeValue(8);

        if (DecodeBool(m_probHalf))
        {
            int32_t i = 0;

            do
            {
                m_frameHead->YModeProbs[i] = m_frameHead->FrameContext.YModeProb[i] =
                    (uint8_t)DecodeValue(8);
            } while (++i < 4);
        }

        if (DecodeBool(m_probHalf))
        {
            int32_t i = 0;

            do
            {
                m_frameHead->UVModeProbs[i] = m_frameHead->FrameContext.UVModeProb[i] =
                    (uint8_t)DecodeValue(8);
            } while (++i < 3);
        }

        MV_CONTEXT MVContext[2];
        for (int32_t i = 0; i < 2; i++)
        {
            for (int32_t j = 0; j < 19; j++)
            {
                MVContext[i].MvProb[j] = vp8PicParams->ucMvUpdateProb[i][j];
            }
        }

        ReadMvContexts(MVContext);
    }

    vp8PicParams->ucP0EntropyCount = 8 - (m_count & 0x07);
    vp8PicParams->ucP0EntropyValue = (uint8_t)(m_value >> 24);
    vp8PicParams->uiP0EntropyRange = m_range;

    uint32_t firstPartitionAndUncompSize;
    if (m_frameHead->iFrameType == m_keyFrame)
    {
        firstPartitionAndUncompSize = vp8PicParams->uiFirstPartitionSize + 10;
    }
    else
    {
        firstPartitionAndUncompSize = vp8PicParams->uiFirstPartitionSize + 3;
    }

    // Partition Size
    uint32_t partitionNum     = 1 << m_frameHead->MultiTokenPartition;
    uint32_t partitionSizeSum = 0;
    m_dataBuffer              = m_bitstreamBuffer + firstPartitionAndUncompSize;
    if (partitionNum > 1)
    {
        for (int32_t i = 1; i < (int32_t)partitionNum; i++)
        {
            vp8PicParams->uiPartitionSize[i] = m_dataBuffer[0] + (m_dataBuffer[1] << 8) + (m_dataBuffer[2] << 16);
            m_dataBuffer += 3;
            partitionSizeSum += vp8PicParams->uiPartitionSize[i];
        }
    }

    uint32_t offsetCounter                      = ((m_count & 0x18) >> 3) + (((m_count & 0x07) != 0) ? 1 : 0);
    vp8PicParams->uiFirstMbByteOffset           = (uint32_t)(m_buffer - m_bitstreamBuffer) - offsetCounter;
    vp8PicParams->uiPartitionSize[0]            = firstPartitionAndUncompSize - (uint32_t)(m_buffer - m_bitstreamBuffer) + offsetCounter;
    vp8PicParams->uiPartitionSize[partitionNum] = m_bitstreamBufferSize - firstPartitionAndUncompSize - (partitionNum - 1) * 3 - partitionSizeSum;

    return eStatus;
}

void Vp8EntropyState::FrameHeadQuantUpdate(
    PCODEC_VP8_PIC_PARAMS vp8PicParams)
{
    for (int32_t i = 0; i < VP8_Q_INDEX_RANGE; i++)
    {
        m_frameHead->Y1DeQuant[i][0] = (int16_t)DcQuant(i, vp8PicParams->cY1DcDeltaQ);
        m_frameHead->Y2DeQuant[i][0] = (int16_t)Dc2Quant(i, vp8PicParams->cY2DcDeltaQ);
        m_frameHead->UVDeQuant[i][0] = (int16_t)DcUVQuant(i, vp8PicParams->cUVDcDeltaQ);

        m_frameHead->Y1DeQuant[i][1] = (int16_t)AcYQuant(i);
        m_frameHead->Y2DeQuant[i][1] = (int16_t)Ac2Quant(i, vp8PicParams->cY2AcDeltaQ);
        m_frameHead->UVDeQuant[i][1] = (int16_t)AcUVQuant(i, vp8PicParams->cUVAcDeltaQ);
    }
}

void Vp8EntropyState::Initialize(
    PCODECHAL_DECODE_VP8_FRAME_HEAD vp8FrameHeadIn,
    uint8_t*        bitstreamBufferIn,
    uint32_t        bitstreamBufferSizeIn)
{
    m_frameHead           = vp8FrameHeadIn;
    m_dataBuffer          = bitstreamBufferIn;
    m_dataBufferEnd       = bitstreamBufferIn + bitstreamBufferSizeIn;
    m_bitstreamBuffer     = bitstreamBufferIn;
    m_bitstreamBufferSize = bitstreamBufferSizeIn;

    m_frameHead->iFrameType                    = m_dataBuffer[0] & 1;
    m_frameHead->iVersion                      = (m_dataBuffer[0] >> 1) & 7;
    m_frameHead->iShowframe                    = (m_dataBuffer[0] >> 4) & 1;
    m_frameHead->uiFirstPartitionLengthInBytes = (m_dataBuffer[0] | (m_dataBuffer[1] << 8) | (m_dataBuffer[2] << 16)) >> 5;

    m_dataBuffer += 3;

    if (m_frameHead->iFrameType == m_keyFrame)
    {
        m_dataBuffer += 7;
    }
}

MOS_STATUS CodechalDecodeVp8::ParseFrameHead(uint8_t* bitstreamBuffer, uint32_t bitstreamBufferSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(bitstreamBuffer);

    m_vp8EntropyState.Initialize(&m_vp8FrameHead, bitstreamBuffer, bitstreamBufferSize);

    eStatus = m_vp8EntropyState.ParseFrameHead(m_vp8PicParams);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Fail to parse VP8 Frame Head");
        return eStatus;
    }

    //------------------------------------------------------Temporary Separator

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

    // Coef Prob
    if (!Mos_ResourceIsNull(&m_resCoefProbBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resCoefProbBuffer);
    }

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resCoefProbBuffer,
                                                  sizeof(m_vp8FrameHead.FrameContext.CoefProbs),
                                                  "VP8_Coef_Prob"),
        "Failed to allocate VP8 CoefProb Buffer.");

    CodechalResLock ResourceLock(m_osInterface, &m_resCoefProbBuffer);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);

    CODECHAL_DECODE_CHK_NULL_RETURN(data);

    MOS_SecureMemcpy(
        data,
        sizeof(m_vp8FrameHead.FrameContext.CoefProbs),
        (void *)&(m_vp8FrameHead.FrameContext.CoefProbs),
        sizeof(m_vp8FrameHead.FrameContext.CoefProbs));

    m_vp8FrameHead.bNotFirstCall = true;

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8::CopyBitstreamBuffer(
    MOS_RESOURCE    srcBitstreamBuffer,
    PMOS_RESOURCE   dstBitstreamBuffer,
    uint32_t        size)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_videoContextForWa));
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        &cmdBuffer,                     // pCmdBuffer
        &srcBitstreamBuffer,            // presSrc
        dstBitstreamBuffer,             // presDst
        MOS_ALIGN_CEIL(size, 16),       // u32CopyLength
        0,                              // u32CopyInputOffset
        0));                            // u32CopyOutputOffset

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    m_huCCopyInUse = true;

    MOS_SYNC_PARAMS syncParams;

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContextForWa;
    syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        m_videoContextUsesNullHw));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8::AllocateResourcesFixedSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_resSyncObject));

    CodecHalAllocateDataList(
        m_vp8RefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectWaContextInUse));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectVideoContextInUse));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8::AllocateResourcesVariableSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_decodeParams.m_bitstreamLockingInUse && (!m_decodeParams.m_bitstreamLockable))
    {
        if (!Mos_ResourceIsNull(&m_resTmpBitstreamBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resTmpBitstreamBuffer);
        }

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resTmpBitstreamBuffer,
                                                      m_dataSize,
                                                      "VP8_BitStream"),
            "Failed to allocate Bitstream Buffer.");
    }

    uint16_t picWidthInMB    = MOS_MAX(m_picWidthInMbLastMaxAlloced, (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1));
    uint16_t picHeightInMB   = MOS_MAX(m_picHeightInMbLastMaxAlloced, (m_vp8PicParams->wFrameHeightInMbsMinus1 + 1));
    uint32_t maxWidth        = picWidthInMB * CODECHAL_MACROBLOCK_WIDTH;
    uint32_t maxHeight       = picHeightInMB * CODECHAL_MACROBLOCK_HEIGHT;
    uint32_t numMacroblocks  = picWidthInMB * picHeightInMB;

    if (m_mfxInterface->IsDeblockingFilterRowstoreCacheEnabled() == false)
    {
        uint16_t maxMfdDFRowStoreScratchBufferPicWidthInMB;
        maxMfdDFRowStoreScratchBufferPicWidthInMB = MOS_MAX(m_mfdDeblockingFilterRowStoreScratchBufferPicWidthInMb,
            (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1));

        if ((maxMfdDFRowStoreScratchBufferPicWidthInMB > m_mfdDeblockingFilterRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&m_resMfdDeblockingFilterRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&m_resMfdDeblockingFilterRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resMfdDeblockingFilterRowStoreScratchBuffer);
            }
            // Deblocking Filter Row Store Scratch buffer
            //(Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resMfdDeblockingFilterRowStoreScratchBuffer,
                                                          maxMfdDFRowStoreScratchBufferPicWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                                                          "DeblockingScratchBuffer"),
                "Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        m_mfdDeblockingFilterRowStoreScratchBufferPicWidthInMb = maxMfdDFRowStoreScratchBufferPicWidthInMB;
    }

    if (m_mfxInterface->IsIntraRowstoreCacheEnabled() == false)
    {
        uint16_t maxMfdIntraRowStoreScratchBufferPicWidthInMB;
        maxMfdIntraRowStoreScratchBufferPicWidthInMB = MOS_MAX(m_mfdIntraRowStoreScratchBufferPicWidthInMb, (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1));

        if ((maxMfdIntraRowStoreScratchBufferPicWidthInMB > m_mfdIntraRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&m_resMfdIntraRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&m_resMfdIntraRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resMfdIntraRowStoreScratchBuffer);
            }

            // Intra Row Store Scratch buffer
            // (FrameWidth in MB) * (CacheLine size per MB)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resMfdIntraRowStoreScratchBuffer,
                                                          maxMfdIntraRowStoreScratchBufferPicWidthInMB * CODECHAL_CACHELINE_SIZE,
                                                          "IntraScratchBuffer"),
                "Failed to allocate VP8 BSD Intra Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        m_mfdIntraRowStoreScratchBufferPicWidthInMb = maxMfdIntraRowStoreScratchBufferPicWidthInMB;
    }

    if (m_mfxInterface->IsBsdMpcRowstoreCacheEnabled() == false)
    {
        uint16_t maxBsdMpcRowStoreScratchBufferPicWidthInMB;
        maxBsdMpcRowStoreScratchBufferPicWidthInMB = MOS_MAX(m_bsdMpcRowStoreScratchBufferPicWidthInMb, (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1));

        if ((maxBsdMpcRowStoreScratchBufferPicWidthInMB > m_bsdMpcRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&m_resBsdMpcRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&m_resBsdMpcRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resBsdMpcRowStoreScratchBuffer);
            }
            // BSD/MPC Row Store Scratch buffer
            // (FrameWidth in MB) * (2) * (CacheLine size per MB)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                          &m_resBsdMpcRowStoreScratchBuffer,
                                                          maxBsdMpcRowStoreScratchBufferPicWidthInMB * CODECHAL_CACHELINE_SIZE * 2,
                                                          "MpcScratchBuffer"),
                "Failed to allocate BSD/MPC Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        m_bsdMpcRowStoreScratchBufferPicWidthInMb = maxBsdMpcRowStoreScratchBufferPicWidthInMB;
    }

    if ((picWidthInMB > m_picWidthInMbLastMaxAlloced) ||
        Mos_ResourceIsNull(&m_resMprRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resMprRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resMprRowStoreScratchBuffer);
        }
        // MPR Row Store Scratch buffer
        // (FrameWidth in MB) * (2) * (CacheLine size per MB)
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resMprRowStoreScratchBuffer,
                                                      picWidthInMB * CODECHAL_CACHELINE_SIZE * 22,
                                                      "MprScratchBuffer"),
            "Failed to allocate MPR Row Store Scratch Buffer.");
    }

    if ((numMacroblocks > (uint32_t)m_picWidthInMbLastMaxAlloced * m_picHeightInMbLastMaxAlloced) ||
        Mos_ResourceIsNull(&m_resSegmentationIdStreamBuffer))
    {
        if (!Mos_ResourceIsNull(&m_resSegmentationIdStreamBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resSegmentationIdStreamBuffer);
        }
        // Segmentation ID Stream buffer
        //(Num MacroBlocks) * (Cachline size) * (2 bit)
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resSegmentationIdStreamBuffer,
                                                      MOS_MAX(numMacroblocks * CODECHAL_CACHELINE_SIZE * 2 / 8, 64),
                                                      "SegmentationIdStreamBuffer"),
            "Failed to allocate Segmentation ID Stream Buffer.");
    }

    m_picWidthInMbLastMaxAlloced  = picWidthInMB;
    m_picHeightInMbLastMaxAlloced = picHeightInMB;

    return eStatus;
}

CodechalDecodeVp8::~CodechalDecodeVp8()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObject);

    CodecHalFreeDataList(m_vp8RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);

    // indicate resCoefProbBuffer is allocated internal, not from m_decodeParams.m_coefProbBuffer
    if (m_vp8FrameHead.bNotFirstCall == true)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resCoefProbBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resTmpBitstreamBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMfdDeblockingFilterRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMfdIntraRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resBsdMpcRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMprRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resSegmentationIdStreamBuffer);

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &m_resSyncObjectWaContextInUse);

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &m_resSyncObjectVideoContextInUse);

    return;
}

MOS_STATUS CodechalDecodeVp8::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    m_dataSize          = m_decodeParams.m_dataSize;
    m_dataOffset        = m_decodeParams.m_dataOffset;
    m_destSurface       = *(m_decodeParams.m_destSurface);
    m_presLastRefSurface    = m_decodeParams.m_presNoneRegLastRefFrame;
    m_presAltRefSurface     = m_decodeParams.m_presNoneRegAltRefFrame;
    m_presGoldenRefSurface  = m_decodeParams.m_presNoneRegGoldenRefFrame;

    m_resDataBuffer     = *(m_decodeParams.m_dataBuffer);
    m_vp8PicParams      = (PCODEC_VP8_PIC_PARAMS)m_decodeParams.m_picParams;
    m_vp8IqMatrixParams = (PCODEC_VP8_IQ_MATRIX_PARAMS)m_decodeParams.m_iqMatrixBuffer;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_vp8PicParams);

    PCODEC_REF_LIST destEntry = m_vp8RefList[m_vp8PicParams->CurrPic.FrameIdx];
    CODEC_PICTURE   currPic   = m_vp8PicParams->CurrPic;

    MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
    destEntry->RefPic = currPic;
    destEntry->resRefPic = m_destSurface.OsResource;

    m_statusReportFeedbackNumber = m_vp8PicParams->uiStatusReportFeedbackNumber;

    m_deblockingEnabled = !m_vp8PicParams->LoopFilterDisable ? true : false;

    if (m_mfxInterface->IsRowStoreCachingSupported())
    {
        MHW_VDBOX_ROWSTORE_PARAMS rowstoreParams;
        MOS_ZeroMemory(&rowstoreParams, sizeof(rowstoreParams));
        rowstoreParams.dwPicWidth = m_width;
        rowstoreParams.bMbaff = false;
        rowstoreParams.Mode = CODECHAL_DECODE_MODE_VP8VLD;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->SetRowstoreCachingOffsets(&rowstoreParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesVariableSizes());

    if (m_decodeParams.m_bitstreamLockingInUse)
    {
        if (m_decodeParams.m_bitstreamLockable)
        {
            CodechalResLock ResourceLock(m_osInterface, &m_resDataBuffer);
            auto bitstreamBuffer = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);

            CODECHAL_DECODE_CHK_STATUS_RETURN(ParseFrameHead(bitstreamBuffer + m_dataOffset, m_dataSize));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CopyBitstreamBuffer(m_resDataBuffer, &m_resTmpBitstreamBuffer, m_dataSize));

            CodechalResLock ResourceLock(m_osInterface, &m_resTmpBitstreamBuffer);
            auto bitstreamBuffer = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);

            CODECHAL_DECODE_CHK_STATUS_RETURN(ParseFrameHead(bitstreamBuffer, m_dataSize));
        }

        m_decodeParams.m_coefProbSize = sizeof(m_vp8FrameHead.FrameContext.CoefProbs);
    }
    else
    {
        m_resCoefProbBuffer = *(m_decodeParams.m_coefProbBuffer);
    }

    m_width  = (m_vp8PicParams->wFrameWidthInMbsMinus1 + 1) * CODECHAL_MACROBLOCK_WIDTH;
    m_height = (m_vp8PicParams->wFrameHeightInMbsMinus1 + 1) * CODECHAL_MACROBLOCK_HEIGHT;

    // Overwrite the actual surface height with the coded height and width of the frame
    // for VP8 since it's possible for a VP8 frame to change size during playback
    m_destSurface.dwWidth  = m_width;
    m_destSurface.dwHeight = m_height;

    m_perfType = m_vp8PicParams->key_frame ? I_TYPE : P_TYPE;

    m_crrPic = m_vp8PicParams->CurrPic;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_NULL_RETURN(m_debugInterface);
        m_vp8PicParams->CurrPic.PicFlags = PICTURE_FRAME;
        m_debugInterface->m_currPic      = m_crrPic;
        m_debugInterface->m_frameType    = m_perfType;

        if (m_vp8PicParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
                m_vp8PicParams));
        }

        if (m_vp8IqMatrixParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(
                m_vp8IqMatrixParams));
        }

        // Dump Vp8CoefProb
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_resCoefProbBuffer),
            CodechalDbgAttr::attrCoeffProb,
            "_DEC",
            m_decodeParams.m_coefProbSize));)

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_vp8PicParams->key_frame)  // reference surface should be nullptr when key_frame == true
    {
        m_presLastRefSurface   = nullptr;
        m_presGoldenRefSurface = nullptr;
        m_presAltRefSurface    = nullptr;
    }
    else
    {
        if((Mos_ResourceIsNull(&m_vp8RefList[m_vp8PicParams->ucLastRefPicIndex]->resRefPic)) && (m_presLastRefSurface))
        {
           m_vp8RefList[m_vp8PicParams->ucLastRefPicIndex]->resRefPic = *m_presLastRefSurface;
        }
        else
        {
           m_presLastRefSurface = &(m_vp8RefList[m_vp8PicParams->ucLastRefPicIndex]->resRefPic);
        }
        if((Mos_ResourceIsNull(&m_vp8RefList[m_vp8PicParams->ucGoldenRefPicIndex]->resRefPic)) && (m_presGoldenRefSurface))
        {
           m_vp8RefList[m_vp8PicParams->ucGoldenRefPicIndex]->resRefPic = *m_presGoldenRefSurface;
        }
        else
        {
           m_presGoldenRefSurface = &(m_vp8RefList[m_vp8PicParams->ucGoldenRefPicIndex]->resRefPic);
        }
        if((Mos_ResourceIsNull(&m_vp8RefList[m_vp8PicParams->ucAltRefPicIndex]->resRefPic)) && (m_presAltRefSurface))
        {
           m_vp8RefList[m_vp8PicParams->ucAltRefPicIndex]->resRefPic = *m_presAltRefSurface;
        }
        else
        {
           m_presAltRefSurface = &(m_vp8RefList[m_vp8PicParams->ucAltRefPicIndex]->resRefPic);
        }
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode               = m_mode;
    pipeModeSelectParams.bStreamOutEnabled  = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable = m_deblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable  = !m_deblockingEnabled;
    pipeModeSelectParams.bShortFormatInUse     = m_shortFormatInUse;

    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;
    surfaceParams.psSurface = &m_destSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode = m_mode;

    if (m_deblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = &m_destSurface;
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = &m_destSurface;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));
#endif

    // when there is no last, golden and alternate reference,
    // the index is set to the destination frame index
    pipeBufAddrParams.presReferences[CodechalDecodeLastRef]      = m_presLastRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeGoldenRef]    = m_presGoldenRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeAlternateRef] = m_presAltRefSurface;

    pipeBufAddrParams.presMfdIntraRowStoreScratchBuffer            = &m_resMfdIntraRowStoreScratchBuffer;
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resMfdDeblockingFilterRowStoreScratchBuffer;
    if (m_streamOutEnabled)
    {
        pipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

    // set all ref pic addresses to valid addresses for error concealment purpose
    for (uint32_t i = 0; i <= CodechalDecodeAlternateRef; i++)
    {
        if (pipeBufAddrParams.presReferences[i] == nullptr && 
            MEDIA_IS_WA(m_waTable, WaDummyReference) && 
            !Mos_ResourceIsNull(&m_dummyReference.OsResource))
        {
            pipeBufAddrParams.presReferences[i] = &m_dummyReference.OsResource;
        }
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));
#endif

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = m_mode;

    indObjBaseAddrParams.dwDataSize     = m_dataSize;
    indObjBaseAddrParams.dwDataOffset   = m_dataOffset;
    indObjBaseAddrParams.presDataBuffer = &m_resDataBuffer;

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer;
    bspBufBaseAddrParams.presMprRowStoreScratchBuffer    = &m_resMprRowStoreScratchBuffer;

    MHW_VDBOX_VP8_PIC_STATE vp8PicState;
    vp8PicState.pVp8PicParams                  = m_vp8PicParams;
    vp8PicState.pVp8IqMatrixParams             = m_vp8IqMatrixParams;
    vp8PicState.presSegmentationIdStreamBuffer = &m_resSegmentationIdStreamBuffer;
    vp8PicState.dwCoefProbTableOffset = 0;
    vp8PicState.presCoefProbBuffer             = &m_resCoefProbBuffer;

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVp8PicCmd(&cmdBuffer, &vp8PicState));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Fill BSD Object Commands
    MHW_VDBOX_VP8_BSD_PARAMS vp8BsdParams;
    vp8BsdParams.pVp8PicParams = m_vp8PicParams;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVp8BsdObjectCmd(&cmdBuffer, &vp8BsdParams));

    // Check if destination surface needs to be synchronized
    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource         = &m_destSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
    if (m_osInterface->bTagResourceSync)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vp8PicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = m_vp8PicParams->CurrPic;
        decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes  = m_vp8RefList[m_vp8PicParams->CurrPic.FrameIdx]->resRefPic;
        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(m_vp8PicParams->CurrPic);
            decodeStatusReport.m_frameType   = m_perfType;)
        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    if (m_huCCopyInUse)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        m_huCCopyInUse = false;
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }

    // Needs to be re-set for Linux buffer re-use scenarios
        m_vp8RefList[m_vp8PicParams->ucCurrPicIndex]->resRefPic =
            m_destSurface.OsResource;

        // Send the signal to indicate decode completion, in case On-Demand Sync is not present
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

        return eStatus;
}

MOS_STATUS CodechalDecodeVp8::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeVp8, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp8::AllocateStandard(
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width         = settings->width;
    m_height        = settings->height;
    m_shortFormatInUse = settings->shortFormatInUse ? true : false;
    m_huCCopyInUse     = false;

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        m_shortFormatInUse);

    // Primitive Level Commands
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        m_shortFormatInUse);

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesFixedSizes());

    return eStatus;
}

CodechalDecodeVp8::CodechalDecodeVp8(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalDecode(hwInterface, debugInterface, standardInfo),
                                            m_picWidthInMbLastMaxAlloced(0),
                                            m_picHeightInMbLastMaxAlloced(0),
                                            m_shortFormatInUse(false),
                                            m_dataSize(0),
                                            m_dataOffset(0),
                                            m_vp8PicParams(nullptr),
                                            m_vp8IqMatrixParams(nullptr),
                                            m_presLastRefSurface(nullptr),
                                            m_presGoldenRefSurface(nullptr),
                                            m_presAltRefSurface(nullptr),
                                            m_mfdDeblockingFilterRowStoreScratchBufferPicWidthInMb(0),
                                            m_mfdIntraRowStoreScratchBufferPicWidthInMb(0),
                                            m_bsdMpcRowStoreScratchBufferPicWidthInMb(0),
                                            m_privateInputBufferSize(0),
                                            m_coeffProbTableOffset(0),
                                            m_deblockingEnabled(false),
                                            m_huCCopyInUse(false)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));
    MOS_ZeroMemory(&m_resCoefProbBuffer, sizeof(m_resCoefProbBuffer));
    MOS_ZeroMemory(&m_resTmpBitstreamBuffer, sizeof(m_resTmpBitstreamBuffer));
    MOS_ZeroMemory(&m_resMfdIntraRowStoreScratchBuffer, sizeof(m_resMfdIntraRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resMfdDeblockingFilterRowStoreScratchBuffer, sizeof(m_resMfdDeblockingFilterRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resBsdMpcRowStoreScratchBuffer, sizeof(m_resBsdMpcRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resMprRowStoreScratchBuffer, sizeof(m_resMprRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resSegmentationIdStreamBuffer, sizeof(m_resSegmentationIdStreamBuffer));
    MOS_ZeroMemory(&m_resSyncObject, sizeof(m_resSyncObject));
    MOS_ZeroMemory(&m_resPrivateInputBuffer, sizeof(m_resPrivateInputBuffer));
    MOS_ZeroMemory(&m_resSyncObjectWaContextInUse, sizeof(m_resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&m_resSyncObjectVideoContextInUse, sizeof(m_resSyncObjectVideoContextInUse));
    MOS_ZeroMemory(&m_vp8FrameHead, sizeof(m_vp8FrameHead));
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeVp8::DumpPicParams(
    PCODEC_VP8_PIC_PARAMS           picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss<< "CurrPic FrameIdx: "<<std::hex<< +picParams->CurrPic.FrameIdx<<std::endl;
    oss<< "CurrPic PicFlags: "<<std::hex<< +picParams->CurrPic.PicFlags<<std::endl;
    oss<< "wFrameWidthInMbsMinus1: "<<std::hex<< +picParams->wFrameWidthInMbsMinus1<<std::endl;
    oss<< "wFrameHeightInMbsMinus1: "<<std::hex<< +picParams->wFrameHeightInMbsMinus1<<std::endl;
    oss<< "ucCurrPicIndex: "<<std::hex<< +picParams->ucCurrPicIndex<<std::endl;
    oss<< "ucLastRefPicIndex: "<<std::hex<< +picParams->ucLastRefPicIndex<<std::endl;
    oss<< "ucGoldenRefPicIndex: "<<std::hex<< +picParams->ucGoldenRefPicIndex<<std::endl;
    oss<< "ucAltRefPicIndex: "<<std::hex<< +picParams->ucAltRefPicIndex<<std::endl;
    oss<< "ucDeblockedPicIndex: "<<std::hex<< +picParams->ucDeblockedPicIndex<<std::endl;
    oss<< "ucReserved8Bits: "<<std::hex<< +picParams->ucReserved8Bits<<std::endl;
    oss<< "wPicFlags: "<<std::hex<< +picParams->wPicFlags<<std::endl;
    oss<< "key_frame: "<<std::hex<< +picParams->key_frame<<std::endl;
    oss<< "version: "<<std::hex<< +picParams->version<<std::endl;
    oss<< "segmentation_enabled: "<<std::hex<< +picParams->segmentation_enabled<<std::endl;
    oss<< "update_mb_segmentation_map: "<<std::hex<< +picParams->update_mb_segmentation_map<<std::endl;
    oss<< "update_segment_feature_data: "<<std::hex<< +picParams->update_segment_feature_data<<std::endl;
    oss<< "filter_type: "<<std::hex<< +picParams->filter_type<<std::endl;
    oss<< "sign_bias_golden: "<<std::hex<< +picParams->sign_bias_golden<<std::endl;
    oss<< "sign_bias_alternate: "<<std::hex<< +picParams->sign_bias_alternate<<std::endl;
    oss<< "mb_no_coeff_skip: "<<std::hex<< +picParams->mb_no_coeff_skip<<std::endl;
    oss<< "mode_ref_lf_delta_update: "<<std::hex<< +picParams->mode_ref_lf_delta_update<<std::endl;
    oss<< "CodedCoeffTokenPartition: "<<std::hex<< +picParams->CodedCoeffTokenPartition<<std::endl;
    oss<< "LoopFilterDisable: "<<std::hex<< +picParams->LoopFilterDisable<<std::endl;
    oss<< "loop_filter_adj_enable: "<<std::hex<< +picParams->loop_filter_adj_enable<<std::endl;

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "ucLoopFilterLevel[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->ucLoopFilterLevel[i]<<std::endl;
    }

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "cRefLfDelta["<<std::dec<<+i<<"]: "<<std::hex<< +picParams->cRefLfDelta[i]<<std::endl;
    }

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "cModeLfDelta[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->cModeLfDelta[i]<<std::endl;
    }
    oss<< "ucSharpnessLevel: " <<std::dec<<std::hex<< +picParams->ucSharpnessLevel<<std::endl;

    for(uint8_t i=0;i<3;++i)
    {
        oss<< "cMbSegmentTreeProbs[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->cMbSegmentTreeProbs[i]<<std::endl;
    }
    oss<< "ucProbSkipFalse: "<<std::hex<< +picParams->ucProbSkipFalse<<std::endl;
    oss<< "ucProbIntra: "<<std::hex<< +picParams->ucProbIntra<<std::endl;
    oss<< "ucProbLast: "<<std::hex<< +picParams->ucProbLast<<std::endl;
    oss<< "ucProbGolden: "<<std::hex<< +picParams->ucProbGolden<<std::endl;

    for(uint8_t i=0;i<4;++i)
    {
        oss<< "ucYModeProbs[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->ucYModeProbs[i]<<std::endl;
    }

    for(uint8_t i=0;i<3;++i)
    {
        oss<< "ucUvModeProbs[" <<std::dec<<+i<<"]: "<<std::hex<< +picParams->ucUvModeProbs[i]<<std::endl;
    }
    oss<< "ucReserved8Bits1: "<<std::hex<< +picParams->ucReserved8Bits1<<std::endl;
    oss<< "ucP0EntropyCount: "<<std::hex<< +picParams->ucP0EntropyCount<<std::endl;
    oss<< "ucP0EntropyValue: "<<std::hex<< +picParams->ucP0EntropyValue<<std::endl;
    oss<< "uiP0EntropyRange: "<<std::hex<< +picParams->uiP0EntropyRange<<std::endl;
    oss<< "uiFirstMbByteOffset: "<<std::hex<< +picParams->uiFirstMbByteOffset<<std::endl;

    for(uint8_t i=0;i<2;++i)
    {
        for(uint8_t j=0;j<CODEC_VP8_MVP_COUNT;++j)
        {
            oss<< "ucMvUpdateProb["<<std::dec<<+i<<"]["<<std::dec<<+j<<"]: "<<std::hex<< +picParams->ucMvUpdateProb[i][j]<<std::endl;
        }
    }
    for(uint8_t i=0;i<CODEC_VP8_MAX_PARTITION_NUMBER;++i)
    {
        oss<< "uiPartitionSize["<<std::dec<<+i<<"]: "<<std::hex<< +picParams->uiPartitionSize[i]<<std::endl;
    }
    oss<< "uiStatusReportFeedbackNumber: "<<std::hex<< +picParams->uiStatusReportFeedbackNumber<<std::endl;

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVp8::DumpIQParams(
    PCODEC_VP8_IQ_MATRIX_PARAMS          matrixData)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrIqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(matrixData);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    for(uint8_t i=0;i<4;++i)
    {
        for(uint8_t j=0;j<6;++j)
        {
            oss<< "quantization_values["<< std::dec<< +i <<"]["<<+j<<"]: "<<std::hex<< +matrixData->quantization_values[i][j]<<std::endl;
        }
    }

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufIqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

#endif
