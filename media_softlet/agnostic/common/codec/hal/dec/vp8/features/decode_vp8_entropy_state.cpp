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
//! \file     decode_vp8_entropy_state.cpp
//! \brief    Defines entropy state list related logic for vp8 decode
//!

#include "decode_utils.h"
#include "decode_vp8_entropy_state.h"
#include "codec_def_decode_vp8.h"

namespace decode
{
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
}  // namespace decode
