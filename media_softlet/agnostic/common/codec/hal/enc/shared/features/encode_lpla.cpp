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
//! \file     encode_lpla.cpp
//! \brief    Defines the common interface for LowPower Lookahead encode
//!

#include "encode_lpla.h"

namespace encode
{
    MOS_STATUS EncodeLPLA::CalculateTargetBufferFullness(
        uint32_t &targetBufferFulness,
        uint32_t &prevTargetFrameSize,
        uint32_t &averageFrameSize)
    {
        ENCODE_FUNC_CALL();
        if (prevTargetFrameSize > 0)
        {
            int64_t bufferFulness = (int64_t)targetBufferFulness;
            bufferFulness += (int64_t)(prevTargetFrameSize << 3) - (int64_t)averageFrameSize;
            targetBufferFulness = bufferFulness < 0 ? 0 : (bufferFulness > 0xFFFFFFFF ? 0xFFFFFFFF : (uint32_t)bufferFulness);
        }

        return MOS_STATUS_SUCCESS;
    }

     MOS_STATUS EncodeLPLA::CheckFrameRate(
        uint32_t &Numerator,
        uint32_t &Denominator,
        uint32_t &TargetBitRate,
        uint32_t &averageFrameSize)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        uint64_t targetBitRate = (uint64_t)TargetBitRate * 1000;
        double   frameRate     = (Denominator ? (double) Numerator / Denominator : 30);
        if ((frameRate < 1) || (targetBitRate < frameRate) || (targetBitRate > 0xFFFFFFFF))
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            ENCODE_ASSERTMESSAGE("Invalid FrameRate or TargetBitRate in LPLA!\n");
        }
        averageFrameSize = (uint32_t)(targetBitRate / frameRate);

        return eStatus;
    }

    MOS_STATUS EncodeLPLA::CheckVBVBuffer(
        uint32_t &VBVBufferSizeInBit,
        uint32_t &InitVBVBufferFullnessInBit)
    {
        ENCODE_FUNC_CALL();
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (VBVBufferSizeInBit < InitVBVBufferFullnessInBit)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            ENCODE_ASSERTMESSAGE("VBVBufferSizeInBit is less than InitVBVBufferFullnessInBit\n");
            return eStatus;
        }

        return eStatus;
    }

    MOS_STATUS EncodeLPLA::CalculateDeltaQP(
        uint8_t  &QpModulationStrength,
        bool     &initDeltaQP,
        bool     isLastPass,
        uint8_t  &DeltaQP,
        uint32_t &prevQpModulationStrength)
    {
        ENCODE_FUNC_CALL();
        uint8_t QpStrength = (uint8_t)(QpModulationStrength + (QpModulationStrength >> 1));
        if (!initDeltaQP)
        {
            DeltaQP = QpModulationStrength ? (prevQpModulationStrength + QpStrength + 1) >> 1 : 0;
        }
        else
        {
            DeltaQP = QpStrength;
            if (isLastPass)
            {
                initDeltaQP = false;
            }
        }
        prevQpModulationStrength = DeltaQP;

        return MOS_STATUS_SUCCESS;
    }

} // encode
