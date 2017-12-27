/*
* Copyright (c) 2017, Intel Corporation
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
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif

void Vp8EntropyState::DecodeFill()
{
    int32_t        shift       = BD_VALUE_SIZE - 8 - (iCount + 8);
    uint32_t       bytesLeft  = (uint32_t)(pBufferEnd - pBuffer);
    uint32_t       bitsLeft   = bytesLeft * CHAR_BIT;
    int32_t        num         = (int32_t)(shift + CHAR_BIT - bitsLeft);
    int32_t        loopEnd     = 0;

    if (num >= 0)
    {
        iCount += LOTS_OF_BITS;
        loopEnd = num;
    }

    if (num < 0 || bitsLeft)
    {
        while (shift >= loopEnd)
        {
            iCount += CHAR_BIT;
            uiValue |= (uint32_t)*pBuffer << shift;
            ++pBuffer;
            shift -= CHAR_BIT;
        }
    }
}

uint32_t Vp8EntropyState::DecodeBool(int32_t probability)
{
    uint32_t split    = 1 + (((uiRange - 1) * probability) >> 8);
    uint32_t bigSplit = (uint32_t)split << (BD_VALUE_SIZE - 8);
    uint32_t origRange = uiRange;
    uiRange = split;

    uint32_t bit = 0;
    if (uiValue >= bigSplit)
    {
        uiRange = origRange - split;
        uiValue = uiValue - bigSplit;
        bit = 1;
    }

    int32_t shift = Norm[uiRange];
    uiRange <<= shift;
    uiValue <<= shift;
    iCount -= shift;

    if (iCount < 0)
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
    if (pFrameHead->iFrameType == KEY_FRAME)
    {
        MOS_SecureMemcpy(pFrameHead->FrameContext.MvContext, sizeof(DefaultMvContext), DefaultMvContext, sizeof(DefaultMvContext));
        MOS_SecureMemcpy(pFrameHead->FrameContext.YModeProb, sizeof(KfYModeProb), KfYModeProb, sizeof(KfYModeProb));
        MOS_SecureMemcpy(pFrameHead->FrameContext.UVModeProb, sizeof(KfUVModeProb), KfUVModeProb, sizeof(KfUVModeProb));
        MOS_SecureMemcpy(pFrameHead->FrameContext.CoefProbs, sizeof(DefaultCoefProbs), DefaultCoefProbs, sizeof(DefaultCoefProbs));

        MOS_SecureMemcpy(pFrameHead->YModeProbs, sizeof(KfYModeProb), KfYModeProb, sizeof(KfYModeProb));
        MOS_SecureMemcpy(pFrameHead->UVModeProbs, sizeof(KfUVModeProb), KfUVModeProb, sizeof(KfUVModeProb));
        MOS_SecureMemcpy(pFrameHead->YModeProbs, sizeof(YModeProb), YModeProb, sizeof(YModeProb));
        MOS_SecureMemcpy(pFrameHead->UVModeProbs, sizeof(UVModeProb), UVModeProb, sizeof(UVModeProb));

        memset(pFrameHead->SegmentFeatureData, 0, sizeof(pFrameHead->SegmentFeatureData));
        pFrameHead->u8MbSegementAbsDelta = 0;

        memset(pFrameHead->RefLFDeltas, 0, sizeof(pFrameHead->RefLFDeltas));
        memset(pFrameHead->ModeLFDeltas, 0, sizeof(pFrameHead->ModeLFDeltas));

        pFrameHead->iRefreshGoldenFrame  = 1;
        pFrameHead->iRefreshAltFrame     = 1;
        pFrameHead->iCopyBufferToGolden  = 0;
        pFrameHead->iCopyBufferToAlt     = 0;

        pFrameHead->iLastFrameBufferCurrIdx   = pFrameHead->iNewFrameBufferIdx;
        pFrameHead->iGoldenFrameBufferCurrIdx = pFrameHead->iNewFrameBufferIdx;
        pFrameHead->iAltFrameBufferCurrIdx    = pFrameHead->iNewFrameBufferIdx;

        pFrameHead->RefFrameSignBias[VP8_GOLDEN_FRAME] = 0;
        pFrameHead->RefFrameSignBias[VP8_ALTREF_FRAME] = 0;
    }
}

int32_t Vp8EntropyState::StartEntropyDecode()
{
    pBufferEnd = pDataBufferEnd;
    pBuffer    = pDataBuffer;
    uiValue    = 0;
    iCount     = -8;
    uiRange    = 255;

    if ((pBufferEnd - pBuffer) > 0 && pBuffer == nullptr)
    {
        return 1;
    }

    DecodeFill();

    return 0;
}

void Vp8EntropyState::SegmentationEnabled()
{
    pFrameHead->u8SegmentationEnabled = (uint8_t)DecodeBool(PROB_HALF);

    if (pFrameHead->u8SegmentationEnabled)
    {
        pFrameHead->u8UpdateMbSegmentationMap = (uint8_t)DecodeBool(PROB_HALF);
        pFrameHead->u8UpdateMbSegmentationData = (uint8_t)DecodeBool(PROB_HALF);

        if (pFrameHead->u8UpdateMbSegmentationData)
        {
            pFrameHead->u8MbSegementAbsDelta = (uint8_t)DecodeBool(PROB_HALF);

            memset(pFrameHead->SegmentFeatureData, 0, sizeof(pFrameHead->SegmentFeatureData));

            for (int32_t i = 0; i < VP8_MB_LVL_MAX; i++)
            {
                for (int32_t j = 0; j < VP8_MAX_MB_SEGMENTS; j++)
                {
                    if (DecodeBool(PROB_HALF))
                    {
                        pFrameHead->SegmentFeatureData[i][j] = (int8_t)DecodeValue(MbFeatureDataBits[i]);

                        if (DecodeBool(PROB_HALF))
                        {
                            pFrameHead->SegmentFeatureData[i][j] = -pFrameHead->SegmentFeatureData[i][j];
                        }
                    }
                    else
                    {
                        pFrameHead->SegmentFeatureData[i][j] = 0;
                    }
                }
            }
        }

        if (pFrameHead->u8UpdateMbSegmentationMap)
        {
            memset(pFrameHead->MbSegmentTreeProbs, 255, sizeof(pFrameHead->MbSegmentTreeProbs));

            for (int32_t i = 0; i < VP8_MB_SEGMENT_TREE_PROBS; i++)
            {
                if (DecodeBool(PROB_HALF))
                {
                    pFrameHead->MbSegmentTreeProbs[i] = (uint8_t)DecodeValue(8);
                }
            }
        }
    }
    else
    {
        pFrameHead->u8UpdateMbSegmentationMap = 0;
        pFrameHead->u8UpdateMbSegmentationData = 0;
    }
}

void Vp8EntropyState::LoopFilterInit(int32_t defaultFilterLvl)
{
    for (int32_t segmentNum = 0; segmentNum < VP8_MAX_MB_SEGMENTS; segmentNum++)
    {
        int32_t segmentLvl = defaultFilterLvl;

        if (pFrameHead->u8SegmentationEnabled)
        {
            if (pFrameHead->u8MbSegementAbsDelta == 1)
            {
                pFrameHead->LoopFilterLevel[segmentNum] = segmentLvl = pFrameHead->SegmentFeatureData[VP8_MB_LVL_ALT_LF][segmentNum];
            }
            else
            {
                segmentLvl += pFrameHead->SegmentFeatureData[VP8_MB_LVL_ALT_LF][segmentNum];
                pFrameHead->LoopFilterLevel[segmentNum] = segmentLvl = (segmentLvl > 0) ? ((segmentLvl > 63) ? 63 : segmentLvl) : 0;
            }
        }
    }
}

void Vp8EntropyState::LoopFilterEnabled()
{
    pFrameHead->FilterType       = (VP8_LF_TYPE)DecodeBool(PROB_HALF);
    pFrameHead->iFilterLevel     = DecodeValue(6);
    pFrameHead->iSharpnessLevel  = DecodeValue(3);

    pFrameHead->u8ModeRefLfDeltaUpdate  = 0;
    pFrameHead->u8ModeRefLfDeltaEnabled = (uint8_t)DecodeBool(PROB_HALF);

    if (pFrameHead->u8ModeRefLfDeltaEnabled)
    {
        pFrameHead->u8ModeRefLfDeltaUpdate = (uint8_t)DecodeBool(PROB_HALF);

        if (pFrameHead->u8ModeRefLfDeltaUpdate)
        {
            for (int32_t i = 0; i < VP8_MAX_REF_LF_DELTAS; i++)
            {
                if (DecodeBool(PROB_HALF))
                {
                    pFrameHead->RefLFDeltas[i] = (int8_t)DecodeValue(6);

                    if (DecodeBool(PROB_HALF))
                    {
                        pFrameHead->RefLFDeltas[i] = pFrameHead->RefLFDeltas[i] * (-1);
                    }
                }
            }

            for (int32_t i = 0; i < VP8_MAX_MODE_LF_DELTAS; i++)
            {
                if (DecodeBool(PROB_HALF))
                {
                    pFrameHead->ModeLFDeltas[i] = (int8_t)DecodeValue(6);

                    if (DecodeBool(PROB_HALF))
                    {
                        pFrameHead->ModeLFDeltas[i] = pFrameHead->ModeLFDeltas[i] * (-1);
                    }
                }
            }
        }
    }

    if (pFrameHead->iFilterLevel)
    {
        LoopFilterInit(pFrameHead->iFilterLevel);
    }
}

int32_t Vp8EntropyState::GetDeltaQ(int32_t prevVal, int32_t *qupdate)
{
    int32_t retVal = 0;

    if (DecodeBool(PROB_HALF))
    {
        retVal = DecodeValue(4);

        if (DecodeBool(PROB_HALF))
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
        pFrameHead->Y1DeQuant[i][0] = (int16_t)DcQuant(i, pFrameHead->iY1DcDeltaQ);
        pFrameHead->Y2DeQuant[i][0] = (int16_t)Dc2Quant(i, pFrameHead->iY2DcDeltaQ);
        pFrameHead->UVDeQuant[i][0] = (int16_t)DcUVQuant(i, pFrameHead->iUVDcDeltaQ);

        pFrameHead->Y1DeQuant[i][1] = (int16_t)AcYQuant(i);
        pFrameHead->Y2DeQuant[i][1] = (int16_t)Ac2Quant(i, pFrameHead->iY2AcDeltaQ);
        pFrameHead->UVDeQuant[i][1] = (int16_t)AcUVQuant(i, pFrameHead->iUVAcDeltaQ);
    }
}

void Vp8EntropyState::QuantSetup()
{
    int32_t qupdate = 0;

    pFrameHead->iBaseQIndex = DecodeValue(7);
    pFrameHead->iY1DcDeltaQ = GetDeltaQ(pFrameHead->iY1DcDeltaQ, &qupdate);
    pFrameHead->iY2DcDeltaQ = GetDeltaQ(pFrameHead->iY2DcDeltaQ, &qupdate);
    pFrameHead->iY2AcDeltaQ = GetDeltaQ(pFrameHead->iY2AcDeltaQ, &qupdate);
    pFrameHead->iUVDcDeltaQ = GetDeltaQ(pFrameHead->iUVDcDeltaQ, &qupdate);
    pFrameHead->iUVAcDeltaQ = GetDeltaQ(pFrameHead->iUVAcDeltaQ, &qupdate);

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

    if (pFrameHead->iFrameType == KEY_FRAME)
    {
        DecodeBool(PROB_HALF);    // Color Space
        DecodeBool(PROB_HALF);    // Clamp Type
    }

    SegmentationEnabled();

    LoopFilterEnabled();

    pFrameHead->MultiTokenPartition = (VP8_TOKEN_PARTITION)DecodeValue(2);

    if (!pFrameHead->bNotFirstCall)
    {
        QuantInit();
    }

    QuantSetup();

    if (pFrameHead->iFrameType != KEY_FRAME)
    {
        pFrameHead->iRefreshGoldenFrame = DecodeBool(PROB_HALF);

        pFrameHead->iRefreshAltFrame = DecodeBool(PROB_HALF);

        pFrameHead->iCopyBufferToGolden = 0;

        if (!pFrameHead->iRefreshGoldenFrame)
        {
            pFrameHead->iCopyBufferToGolden = DecodeValue(2);
        }

        pFrameHead->iCopyBufferToAlt = 0;

        if (!pFrameHead->iRefreshAltFrame)
        {
            pFrameHead->iCopyBufferToAlt = DecodeValue(2);
        }

        pFrameHead->RefFrameSignBias[VP8_GOLDEN_FRAME] = DecodeBool(PROB_HALF);
        pFrameHead->RefFrameSignBias[VP8_ALTREF_FRAME] = DecodeBool(PROB_HALF);
    }

    if (pFrameHead->bNotFirstCall && pFrameHead->iRefreshEntropyProbs == 0)
    {
        MOS_SecureMemcpy(&pFrameHead->FrameContext, sizeof(pFrameHead->FrameContext), &pFrameHead->LastFrameContext, sizeof(pFrameHead->LastFrameContext));
    }

    pFrameHead->iRefreshEntropyProbs = DecodeBool(PROB_HALF);

    if (pFrameHead->iRefreshEntropyProbs == 0)
    {
        MOS_SecureMemcpy(&pFrameHead->LastFrameContext, sizeof(pFrameHead->LastFrameContext), &pFrameHead->FrameContext, sizeof(pFrameHead->FrameContext));
    }

    if (pFrameHead->iFrameType == KEY_FRAME || DecodeBool(PROB_HALF))
    {
        pFrameHead->iRefreshLastFrame = true;
    }
    else
    {
        pFrameHead->iRefreshLastFrame = false;
    }

    for (int32_t i = 0; i < VP8_BLOCK_TYPES; i++)
        for (int32_t j = 0; j < VP8_COEF_BANDS; j++)
            for (int32_t k = 0; k < VP8_PREV_COEF_CONTEXTS; k++)
                for (int32_t l = 0; l < VP8_ENTROPY_NODES; l++)
                {
                    uint8_t *const p = pFrameHead->FrameContext.CoefProbs[i][j][k] + l;

                    if (DecodeBool(CoefUpdateProbs[i][j][k][l]))
                    {
                        *p = (uint8_t)DecodeValue(8);
                    }
                }

    pFrameHead->iMbNoCoeffSkip = (int32_t)DecodeBool(PROB_HALF);
    pFrameHead->iProbSkipFalse = 0;
    if (pFrameHead->iMbNoCoeffSkip)
    {
        pFrameHead->iProbSkipFalse = (uint8_t)DecodeValue(8);
    }

    if (pFrameHead->iFrameType != KEY_FRAME)
    {
        pFrameHead->ProbIntra = (uint8_t)DecodeValue(8);
        pFrameHead->ProbLast = (uint8_t)DecodeValue(8);
        pFrameHead->ProbGf = (uint8_t)DecodeValue(8);

        if (DecodeBool(PROB_HALF))
        {
            int32_t i = 0;

            do
            {
                pFrameHead->YModeProbs[i] = pFrameHead->FrameContext.YModeProb[i] =
                    (uint8_t)DecodeValue(8);
            } while (++i < 4);
        }

        if (DecodeBool(PROB_HALF))
        {
            int32_t i = 0;

            do
            {
                pFrameHead->UVModeProbs[i] = pFrameHead->FrameContext.UVModeProb[i] =
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

    vp8PicParams->ucP0EntropyCount = 8 - (iCount & 0x07);
    vp8PicParams->ucP0EntropyValue = (uint8_t)(uiValue >> 24);
    vp8PicParams->uiP0EntropyRange = uiRange;

    uint32_t firstPartitionAndUncompSize;
    if (pFrameHead->iFrameType == KEY_FRAME)
    {
        firstPartitionAndUncompSize = vp8PicParams->uiFirstPartitionSize + 10;
    }
    else
    {
        firstPartitionAndUncompSize = vp8PicParams->uiFirstPartitionSize + 3;
    }

    // Partition Size
    uint32_t partitionNum = 1 << pFrameHead->MultiTokenPartition;
    uint32_t partitionSizeSum = 0;
    pDataBuffer = pBitstreamBuffer + firstPartitionAndUncompSize;
    if (partitionNum > 1)
    {
        for (int32_t i = 1; i < (int32_t)partitionNum; i++)
        {
            vp8PicParams->uiPartitionSize[i] = pDataBuffer[0] + (pDataBuffer[1] << 8) + (pDataBuffer[2] << 16);
            pDataBuffer += 3;
            partitionSizeSum += vp8PicParams->uiPartitionSize[i];
        }
    }

    uint32_t offsetCounter = ((iCount & 0x18) >> 3) + (((iCount & 0x07) != 0) ? 1 : 0);
    vp8PicParams->uiFirstMbByteOffset = (uint32_t)(pBuffer - pBitstreamBuffer) - offsetCounter;
    vp8PicParams->uiPartitionSize[0] = firstPartitionAndUncompSize - (uint32_t)(pBuffer - pBitstreamBuffer) + offsetCounter;
    vp8PicParams->uiPartitionSize[partitionNum] = u32BitstreamBufferSize - firstPartitionAndUncompSize - (partitionNum - 1) * 3 - partitionSizeSum;

    return eStatus;
}

void Vp8EntropyState::FrameHeadQuantUpdate(
    PCODEC_VP8_PIC_PARAMS vp8PicParams)
{
    for (int32_t i = 0; i < VP8_Q_INDEX_RANGE; i++)
    {
        pFrameHead->Y1DeQuant[i][0] = (int16_t)DcQuant(i, vp8PicParams->cY1DcDeltaQ);
        pFrameHead->Y2DeQuant[i][0] = (int16_t)Dc2Quant(i, vp8PicParams->cY2DcDeltaQ);
        pFrameHead->UVDeQuant[i][0] = (int16_t)DcUVQuant(i, vp8PicParams->cUVDcDeltaQ);

        pFrameHead->Y1DeQuant[i][1] = (int16_t)AcYQuant(i);
        pFrameHead->Y2DeQuant[i][1] = (int16_t)Ac2Quant(i, vp8PicParams->cY2AcDeltaQ);
        pFrameHead->UVDeQuant[i][1] = (int16_t)AcUVQuant(i, vp8PicParams->cUVAcDeltaQ);
    }
}

void Vp8EntropyState::Initialize(
    PCODECHAL_DECODE_VP8_FRAME_HEAD vp8FrameHeadIn,
    uint8_t*        bitstreamBufferIn,
    uint32_t        bitstreamBufferSizeIn)
{
    pFrameHead = vp8FrameHeadIn;
    pDataBuffer = bitstreamBufferIn;
    pDataBufferEnd = bitstreamBufferIn + bitstreamBufferSizeIn;
    pBitstreamBuffer = bitstreamBufferIn;
    u32BitstreamBufferSize = bitstreamBufferSizeIn;

    pFrameHead->iFrameType   = pDataBuffer[0] & 1;
    pFrameHead->iVersion     = (pDataBuffer[0] >> 1) & 7;
    pFrameHead->iShowframe   = (pDataBuffer[0] >> 4) & 1;
    pFrameHead->uiFirstPartitionLengthInBytes = (pDataBuffer[0] | (pDataBuffer[1] << 8) | (pDataBuffer[2] << 16)) >> 5;

    pDataBuffer += 3;

    if (pFrameHead->iFrameType == KEY_FRAME)
    {
        pDataBuffer += 7;
    }
}

MOS_STATUS CodechalDecodeVp8::ParseFrameHead(uint8_t* bitstreamBuffer, uint32_t bitstreamBufferSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(bitstreamBuffer);

    Vp8EntropyState.Initialize(&Vp8FrameHead, bitstreamBuffer, bitstreamBufferSize);

    eStatus = Vp8EntropyState.ParseFrameHead(pVp8PicParams);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Fail to parse VP8 Frame Head");
        return eStatus;
    }

    //------------------------------------------------------Temporary Separator

    // Loop Filter
    for (int32_t i = 0; i < VP8_MAX_MB_SEGMENTS; i++)
    {
        int32_t segmentLvl = pVp8PicParams->ucFilterLevel;

        if (pVp8PicParams->segmentation_enabled)
        {
            if (pVp8PicParams->mb_segement_abs_delta == 1)
            {
                pVp8PicParams->ucLoopFilterLevel[i] = segmentLvl = pVp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_LF][i];
            }
            else
            {
                segmentLvl += pVp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_LF][i];
                pVp8PicParams->ucLoopFilterLevel[i] = segmentLvl = (segmentLvl > 0) ? ((segmentLvl > 63) ? 63 : segmentLvl) : 0;
            }
        }
        else
        {
            pVp8PicParams->ucLoopFilterLevel[i] = pVp8PicParams->ucFilterLevel;
        }
    }

    // Quant Matrix
    int32_t vp8QIndex[VP8_MAX_MB_SEGMENTS];
    if (pVp8PicParams->segmentation_enabled)
    {
        for (int32_t i = 0; i < 4; i++)
        {
            if (pVp8PicParams->mb_segement_abs_delta == 1)
            {
                vp8QIndex[i] = (int32_t)pVp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_Q][i];
            }
            else
            {
                vp8QIndex[i] = (int32_t)pVp8PicParams->ucBaseQIndex + (int32_t)pVp8PicParams->cSegmentFeatureData[VP8_MB_LVL_ALT_Q][i];
                vp8QIndex[i] = (vp8QIndex[i] >= 0) ? ((vp8QIndex[i] <= VP8_MAX_Q) ? vp8QIndex[i] : VP8_MAX_Q) : 0;    // Clamp to valid range
            }
        }
    }
    else
    {
        vp8QIndex[0] = (int32_t)pVp8PicParams->ucBaseQIndex;
        vp8QIndex[1] = 0;
        vp8QIndex[2] = 0;
        vp8QIndex[3] = 0;
    }

    Vp8EntropyState.FrameHeadQuantUpdate(pVp8PicParams);

    pVp8IqMatrixParams->quantization_values[0][0] = Vp8FrameHead.Y1DeQuant[vp8QIndex[0]][0];
    pVp8IqMatrixParams->quantization_values[0][1] = Vp8FrameHead.Y1DeQuant[vp8QIndex[0]][1];
    pVp8IqMatrixParams->quantization_values[0][2] = Vp8FrameHead.UVDeQuant[vp8QIndex[0]][0];
    pVp8IqMatrixParams->quantization_values[0][3] = Vp8FrameHead.UVDeQuant[vp8QIndex[0]][1];
    pVp8IqMatrixParams->quantization_values[0][4] = Vp8FrameHead.Y2DeQuant[vp8QIndex[0]][0];
    pVp8IqMatrixParams->quantization_values[0][5] = Vp8FrameHead.Y2DeQuant[vp8QIndex[0]][1];

    if (Vp8FrameHead.u8SegmentationEnabled)
    {
        for (int32_t i = 1; i < 4; i++)
        {
            pVp8IqMatrixParams->quantization_values[i][0] = Vp8FrameHead.Y1DeQuant[vp8QIndex[i]][0];
            pVp8IqMatrixParams->quantization_values[i][1] = Vp8FrameHead.Y1DeQuant[vp8QIndex[i]][1];
            pVp8IqMatrixParams->quantization_values[i][2] = Vp8FrameHead.UVDeQuant[vp8QIndex[i]][0];
            pVp8IqMatrixParams->quantization_values[i][3] = Vp8FrameHead.UVDeQuant[vp8QIndex[i]][1];
            pVp8IqMatrixParams->quantization_values[i][4] = Vp8FrameHead.Y2DeQuant[vp8QIndex[i]][0];
            pVp8IqMatrixParams->quantization_values[i][5] = Vp8FrameHead.Y2DeQuant[vp8QIndex[i]][1];
        }
    }
    else
    {
        for (int32_t i = 1; i < 4; i++)
        {
            for (int32_t j = 0; j < 6; j++)
            {
                pVp8IqMatrixParams->quantization_values[i][j] = 0;
            }
        }
    }

    // Coef Prob
    if (!Mos_ResourceIsNull(&resCoefProbBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resCoefProbBuffer);
    }

    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
        &resCoefProbBuffer,
        sizeof(Vp8FrameHead.FrameContext.CoefProbs),
        "VP8_Coef_Prob"),
        "Failed to allocate VP8 CoefProb Buffer.");

    CodechalResLock ResourceLock(m_osInterface, &resCoefProbBuffer);
    auto data = (uint8_t*)ResourceLock.Lock(CodechalResLock::writeOnly);

    MOS_SecureMemcpy(
        data,
        sizeof(Vp8FrameHead.FrameContext.CoefProbs),
        (void*)&(Vp8FrameHead.FrameContext.CoefProbs),
        sizeof(Vp8FrameHead.FrameContext.CoefProbs));

    Vp8FrameHead.bNotFirstCall = true;

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

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    bHuCCopyInUse = true;

    MOS_SYNC_PARAMS syncParams;

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &resSyncObjectVideoContextInUse;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContextForWa;
    syncParams.presSyncResource = &resSyncObjectVideoContextInUse;

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

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &resSyncObject));

    CodecHalAllocateDataList(
        pVp8RefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &resSyncObjectWaContextInUse));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &resSyncObjectVideoContextInUse));

    return eStatus;
}

MOS_STATUS CodechalDecodeVp8::AllocateResourcesVariableSizes()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_decodeParams.m_bitstreamLockingInUse && (!m_decodeParams.m_bitstreamLockable))
    {
        if (!Mos_ResourceIsNull(&resTmpBitstreamBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resTmpBitstreamBuffer);
        }

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resTmpBitstreamBuffer,
            u32DataSize,
            "VP8_BitStream"),
            "Failed to allocate Bitstream Buffer.");
    }

    uint16_t picWidthInMB    = MOS_MAX(u16PicWidthInMbLastMaxAlloced, (pVp8PicParams->wFrameWidthInMbsMinus1 + 1));
    uint16_t picHeightInMB   = MOS_MAX(u16PicHeightInMbLastMaxAlloced, (pVp8PicParams->wFrameHeightInMbsMinus1 + 1));
    uint32_t maxWidth        = picWidthInMB * CODECHAL_MACROBLOCK_WIDTH;
    uint32_t maxHeight       = picHeightInMB * CODECHAL_MACROBLOCK_HEIGHT;
    uint32_t numMacroblocks  = picWidthInMB * picHeightInMB;

    if (m_mfxInterface->IsDeblockingFilterRowstoreCacheEnabled() == false)
    {
        uint16_t maxMfdDFRowStoreScratchBufferPicWidthInMB;
        maxMfdDFRowStoreScratchBufferPicWidthInMB = MOS_MAX(u16MfdDeblockingFilterRowStoreScratchBufferPicWidthInMb,
            (pVp8PicParams->wFrameWidthInMbsMinus1 + 1));

        if ((maxMfdDFRowStoreScratchBufferPicWidthInMB > u16MfdDeblockingFilterRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&resMfdDeblockingFilterRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resMfdDeblockingFilterRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMfdDeblockingFilterRowStoreScratchBuffer);
            }
            // Deblocking Filter Row Store Scratch buffer
            //(Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMfdDeblockingFilterRowStoreScratchBuffer,
                maxMfdDFRowStoreScratchBufferPicWidthInMB * 2 * CODECHAL_CACHELINE_SIZE,
                "DeblockingScratchBuffer"),
                "Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u16MfdDeblockingFilterRowStoreScratchBufferPicWidthInMb = maxMfdDFRowStoreScratchBufferPicWidthInMB;
    }

    if (m_mfxInterface->IsIntraRowstoreCacheEnabled() == false)
    {
        uint16_t maxMfdIntraRowStoreScratchBufferPicWidthInMB;
        maxMfdIntraRowStoreScratchBufferPicWidthInMB = MOS_MAX(u16MfdIntraRowStoreScratchBufferPicWidthInMb, (pVp8PicParams->wFrameWidthInMbsMinus1 + 1));

        if ((maxMfdIntraRowStoreScratchBufferPicWidthInMB > u16MfdIntraRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&resMfdIntraRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resMfdIntraRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resMfdIntraRowStoreScratchBuffer);
            }

            // Intra Row Store Scratch buffer
            // (FrameWidth in MB) * (CacheLine size per MB)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resMfdIntraRowStoreScratchBuffer,
                maxMfdIntraRowStoreScratchBufferPicWidthInMB * CODECHAL_CACHELINE_SIZE,
                "IntraScratchBuffer"),
                "Failed to allocate VP8 BSD Intra Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u16MfdIntraRowStoreScratchBufferPicWidthInMb = maxMfdIntraRowStoreScratchBufferPicWidthInMB;
    }

    if (m_mfxInterface->IsBsdMpcRowstoreCacheEnabled() == false)
    {
        uint16_t maxBsdMpcRowStoreScratchBufferPicWidthInMB;
        maxBsdMpcRowStoreScratchBufferPicWidthInMB = MOS_MAX(u16BsdMpcRowStoreScratchBufferPicWidthInMb, (pVp8PicParams->wFrameWidthInMbsMinus1 + 1));

        if ((maxBsdMpcRowStoreScratchBufferPicWidthInMB > u16BsdMpcRowStoreScratchBufferPicWidthInMb) ||
            Mos_ResourceIsNull(&resBsdMpcRowStoreScratchBuffer))
        {
            if (!Mos_ResourceIsNull(&resBsdMpcRowStoreScratchBuffer))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &resBsdMpcRowStoreScratchBuffer);
            }
            // BSD/MPC Row Store Scratch buffer
            // (FrameWidth in MB) * (2) * (CacheLine size per MB)
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                &resBsdMpcRowStoreScratchBuffer,
                maxBsdMpcRowStoreScratchBufferPicWidthInMB * CODECHAL_CACHELINE_SIZE * 2,
                "MpcScratchBuffer"),
                "Failed to allocate BSD/MPC Row Store Scratch Buffer.");
        }

        //record the width and height used for allocation internal resources.
        u16BsdMpcRowStoreScratchBufferPicWidthInMb = maxBsdMpcRowStoreScratchBufferPicWidthInMB;
    }

    if ((picWidthInMB > u16PicWidthInMbLastMaxAlloced) ||
        Mos_ResourceIsNull(&resMprRowStoreScratchBuffer))
    {
        if (!Mos_ResourceIsNull(&resMprRowStoreScratchBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resMprRowStoreScratchBuffer);
        }
        // MPR Row Store Scratch buffer
        // (FrameWidth in MB) * (2) * (CacheLine size per MB)
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resMprRowStoreScratchBuffer,
            picWidthInMB * CODECHAL_CACHELINE_SIZE * 22,
            "MprScratchBuffer"),
            "Failed to allocate MPR Row Store Scratch Buffer.");
    }

    if ((numMacroblocks > (uint32_t)u16PicWidthInMbLastMaxAlloced * u16PicHeightInMbLastMaxAlloced) ||
        Mos_ResourceIsNull(&resSegmentationIdStreamBuffer))
    {
        if (!Mos_ResourceIsNull(&resSegmentationIdStreamBuffer))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &resSegmentationIdStreamBuffer);
        }
        // Segmentation ID Stream buffer
        //(Num MacroBlocks) * (Cachline size) * (2 bit)
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
            &resSegmentationIdStreamBuffer,
            MOS_MAX(numMacroblocks * CODECHAL_CACHELINE_SIZE * 2 / 8, 64),
            "SegmentationIdStreamBuffer"),
            "Failed to allocate Segmentation ID Stream Buffer.");
    }

    u16PicWidthInMbLastMaxAlloced = picWidthInMB;
    u16PicHeightInMbLastMaxAlloced = picHeightInMB;

    return eStatus;
}

CodechalDecodeVp8::~CodechalDecodeVp8()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnDestroySyncResource(m_osInterface, &resSyncObject);

    CodecHalFreeDataList(pVp8RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8);

    // indicate resCoefProbBuffer is allocated internal, not from m_decodeParams.m_coefProbBuffer
    if (Vp8FrameHead.bNotFirstCall == true)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &resCoefProbBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resTmpBitstreamBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMfdDeblockingFilterRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMfdIntraRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resBsdMpcRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resMprRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &resSegmentationIdStreamBuffer);


    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &resSyncObjectWaContextInUse);

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &resSyncObjectVideoContextInUse);

    return;
}

MOS_STATUS CodechalDecodeVp8::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    u32DataSize   = m_decodeParams.m_dataSize;
    u32DataOffset = m_decodeParams.m_dataOffset;
    sDestSurface  = *(m_decodeParams.m_destSurface);
    resDataBuffer = *(m_decodeParams.m_dataBuffer);
    pVp8PicParams = (PCODEC_VP8_PIC_PARAMS)m_decodeParams.m_picParams;
    pVp8IqMatrixParams = (PCODEC_VP8_IQ_MATRIX_PARAMS)m_decodeParams.m_iqMatrixBuffer;

    CODECHAL_DECODE_CHK_NULL_RETURN(pVp8PicParams);

    PCODEC_REF_LIST destEntry = pVp8RefList[pVp8PicParams->CurrPic.FrameIdx];
    CODEC_PICTURE currPic = pVp8PicParams->CurrPic;

    MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
    destEntry->RefPic = currPic;
    destEntry->resRefPic = sDestSurface.OsResource;

    m_statusReportFeedbackNumber = pVp8PicParams->uiStatusReportFeedbackNumber;

    bDeblockingEnabled = !pVp8PicParams->LoopFilterDisable ? true : false;

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
            CodechalResLock ResourceLock(m_osInterface, &resDataBuffer);
            auto bitstreamBuffer = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);

            CODECHAL_DECODE_CHK_STATUS_RETURN(ParseFrameHead(bitstreamBuffer + u32DataOffset, u32DataSize));
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(CopyBitstreamBuffer(resDataBuffer, &resTmpBitstreamBuffer, u32DataSize));

            CodechalResLock ResourceLock(m_osInterface, &resTmpBitstreamBuffer);
            auto bitstreamBuffer = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);

            CODECHAL_DECODE_CHK_STATUS_RETURN(ParseFrameHead(bitstreamBuffer, u32DataSize));
        }

        m_decodeParams.m_coefProbSize = sizeof(Vp8FrameHead.FrameContext.CoefProbs);
    }
    else
    {
        resCoefProbBuffer = *(m_decodeParams.m_coefProbBuffer);
    }

    m_width = (pVp8PicParams->wFrameWidthInMbsMinus1 + 1) * CODECHAL_MACROBLOCK_WIDTH;
    m_height = (pVp8PicParams->wFrameHeightInMbsMinus1 + 1) * CODECHAL_MACROBLOCK_HEIGHT;

    // Overwrite the actual surface height with the coded height and width of the frame
    // for VP8 since it's possible for a VP8 frame to change size during playback
    sDestSurface.dwWidth = m_width;
    sDestSurface.dwHeight = m_height;

    m_perfType = pVp8PicParams->key_frame ? I_TYPE : P_TYPE;

    m_crrPic = pVp8PicParams->CurrPic;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_NULL_RETURN(m_debugInterface);
    pVp8PicParams->CurrPic.PicFlags = PICTURE_FRAME;
    m_debugInterface->CurrPic = m_crrPic;
    m_debugInterface->wFrameType = m_perfType;

    if (pVp8PicParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
            pVp8PicParams));
    }

    if (pVp8IqMatrixParams)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(DumpIQParams(
            pVp8IqMatrixParams));
    }

    // Dump Vp8CoefProb
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &(resCoefProbBuffer),
        CodechalDbgAttr::attrCoeffProb,
        "_DEC",
        m_decodeParams.m_coefProbSize));
    )

        return eStatus;
}

MOS_STATUS CodechalDecodeVp8::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (pVp8PicParams->key_frame)  // reference surface should be nullptr when key_frame == true
    {
        presLastRefSurface = nullptr;
        presGoldenRefSurface = nullptr;
        presAltRefSurface = nullptr;
    }
    else
    {
        presLastRefSurface = &(pVp8RefList[pVp8PicParams->ucLastRefPicIndex]->resRefPic);
        presGoldenRefSurface = &(pVp8RefList[pVp8PicParams->ucGoldenRefPicIndex]->resRefPic);
        presAltRefSurface = &(pVp8RefList[pVp8PicParams->ucAltRefPicIndex]->resRefPic);
    }


    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    MOS_ZeroMemory(&pipeModeSelectParams, sizeof(pipeModeSelectParams));
    pipeModeSelectParams.Mode               = m_mode;
    pipeModeSelectParams.bStreamOutEnabled  = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable  = bDeblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable   = !bDeblockingEnabled;
    pipeModeSelectParams.bShortFormatInUse      = bShortFormatInUse;

    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;
    surfaceParams.psSurface = &sDestSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    MOS_ZeroMemory(&pipeBufAddrParams, sizeof(pipeBufAddrParams));
    pipeBufAddrParams.Mode = m_mode;

    if (bDeblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = &sDestSurface;        
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = &sDestSurface;        
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));

    // when there is no last, golden and alternate reference,
    // the index is set to the destination frame index
    pipeBufAddrParams.presReferences[CodechalDecodeLastRef]         = presLastRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeGoldenRef]       = presGoldenRefSurface;
    pipeBufAddrParams.presReferences[CodechalDecodeAlternateRef]    = presAltRefSurface;

    pipeBufAddrParams.presMfdIntraRowStoreScratchBuffer = &resMfdIntraRowStoreScratchBuffer;
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &resMfdDeblockingFilterRowStoreScratchBuffer;
    if (m_streamOutEnabled)
    {
        pipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = m_mode;

    indObjBaseAddrParams.dwDataSize = u32DataSize;
    indObjBaseAddrParams.dwDataOffset = u32DataOffset;
    indObjBaseAddrParams.presDataBuffer = &resDataBuffer;

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &resBsdMpcRowStoreScratchBuffer;
    bspBufBaseAddrParams.presMprRowStoreScratchBuffer = &resMprRowStoreScratchBuffer;

    MHW_VDBOX_VP8_PIC_STATE vp8PicState;
    vp8PicState.pVp8PicParams = pVp8PicParams;
    vp8PicState.pVp8IqMatrixParams = pVp8IqMatrixParams;
    vp8PicState.presSegmentationIdStreamBuffer = &resSegmentationIdStreamBuffer;
    vp8PicState.dwCoefProbTableOffset = 0;
    vp8PicState.presCoefProbBuffer = &resCoefProbBuffer;

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
    vp8BsdParams.pVp8PicParams = pVp8PicParams;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVp8BsdObjectCmd(&cmdBuffer, &vp8BsdParams));

    // Check if destination surface needs to be synchronized
    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &sDestSurface.OsResource;
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
        decodeStatusReport.m_currDecodedPic     = pVp8PicParams->CurrPic;
        decodeStatusReport.m_currDeblockedPic   = pVp8PicParams->CurrPic;
        decodeStatusReport.m_codecStatus        = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes  = pVp8RefList[pVp8PicParams->CurrPic.FrameIdx]->resRefPic;
        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField = CodecHal_PictureIsBottomField(pVp8PicParams->CurrPic);
            decodeStatusReport.m_frameType = m_perfType;
        )
            CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddWatchdogTimerStopCmd(&cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            nullptr));
    )

    if (bHuCCopyInUse)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        bHuCCopyInUse = false;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&sDestSurface);
    )

        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }

    // Needs to be re-set for Linux buffer re-use scenarios
    pVp8RefList[pVp8PicParams->ucCurrPicIndex]->resRefPic =
        sDestSurface.OsResource;

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
    PCODECHAL_SETTINGS          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    m_width         = settings->dwWidth;
    m_height        = settings->dwHeight;
    bShortFormatInUse = settings->bShortFormatInUse ? true : false;
    bHuCCopyInUse   = false;

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        bShortFormatInUse);

    // Primitive Level Commands
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        bShortFormatInUse);

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResourcesFixedSizes());

    return eStatus;
}

CodechalDecodeVp8::CodechalDecodeVp8(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecode(hwInterface, debugInterface, standardInfo),
    u16PicWidthInMbLastMaxAlloced(0),
    u16PicHeightInMbLastMaxAlloced(0),
    bShortFormatInUse(false),
    u32DataSize(0),
    pVp8PicParams(nullptr),
    pVp8IqMatrixParams(nullptr),
    presLastRefSurface(nullptr),
    presGoldenRefSurface(nullptr),
    presAltRefSurface(nullptr),
    u16MfdDeblockingFilterRowStoreScratchBufferPicWidthInMb(0),
    u16MfdIntraRowStoreScratchBufferPicWidthInMb(0),
    u16BsdMpcRowStoreScratchBufferPicWidthInMb(0),
    u32PrivateInputBufferSize(0),
    u32CoeffProbTableOffset(0),
    bDeblockingEnabled(false),
    bHuCCopyInUse(false)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&sDestSurface,                                   sizeof(sDestSurface));
    MOS_ZeroMemory(&resDataBuffer,                                  sizeof(resDataBuffer));
    MOS_ZeroMemory(&resCoefProbBuffer,                              sizeof(resCoefProbBuffer));
    MOS_ZeroMemory(&resTmpBitstreamBuffer,                          sizeof(resTmpBitstreamBuffer));
    MOS_ZeroMemory(&resMfdIntraRowStoreScratchBuffer,               sizeof(resMfdIntraRowStoreScratchBuffer));
    MOS_ZeroMemory(&resMfdDeblockingFilterRowStoreScratchBuffer,    sizeof(resMfdDeblockingFilterRowStoreScratchBuffer));
    MOS_ZeroMemory(&resBsdMpcRowStoreScratchBuffer,                 sizeof(resBsdMpcRowStoreScratchBuffer));
    MOS_ZeroMemory(&resMprRowStoreScratchBuffer,                    sizeof(resMprRowStoreScratchBuffer));
    MOS_ZeroMemory(&resSegmentationIdStreamBuffer,                  sizeof(resSegmentationIdStreamBuffer));
    MOS_ZeroMemory(&resSyncObject,                                  sizeof(resSyncObject));
    MOS_ZeroMemory(&resPrivateInputBuffer,                          sizeof(resPrivateInputBuffer));
    MOS_ZeroMemory(&resSyncObjectWaContextInUse,                    sizeof(resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&resSyncObjectVideoContextInUse,                 sizeof(resSyncObjectVideoContextInUse));
    MOS_ZeroMemory(&Vp8FrameHead,                                   sizeof(Vp8FrameHead));
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
