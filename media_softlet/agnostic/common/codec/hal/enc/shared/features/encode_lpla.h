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
//! \file     encode_lpla.h
//! \brief    Defines the common interface for LowPower Lookahead encode
//!

#ifndef __ENCODE_LPLA_H__
#define __ENCODE_LPLA_H__

#include "media_feature.h"
#include "encode_pipeline.h"

namespace encode
{
    class EncodeLPLA
    {
    public:
        //!
        //! \brief  Calculate target buffer fullness
        //! \param  [in] targetBufferFulness
        //!         target buffer fullness
        //! \param  [in] prevTargetFrameSize
        //!         Previous target frame size
        //! \param  [in] averageFrameSize
        //!         Average frame size
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculateTargetBufferFullness(
            uint32_t &targetBufferFulness,
            uint32_t &prevTargetFrameSize,
            uint32_t &averageFrameSize);
        
        //!
        //! \brief  Check if frame rate is valid
        //! \param  [in] Numerator
        //!         Numerator in ddi params
        //! \param  [in] Denominator
        //!         Denominator in ddi params
        //! \param  [in] TargetBitRate
        //!         Target bit rate
        //! \param  [in] averageFrameSize
        //!         Average frame size
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CheckFrameRate(
            uint32_t &Numerator,
            uint32_t &Denominator,
            uint32_t &TargetBitRate,
            uint32_t &averageFrameSize);

        //!
        //! \brief  Check if VBV Buffer is valid
        //! \param  [in] VBVBufferSizeInBit
        //!         VBV buffer size in bit
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CheckVBVBuffer(
            uint32_t &VBVBufferSizeInBit,
            uint32_t &InitVBVBufferFullnessInBit);

        //!
        //! \brief  Calculate delta QP
        //! \param  [in] QpModulationStrength
        //!         QpModulation strength
        //! \param  [in] initDeltaQP
        //!         flag to indicate initial delta QP
        //! \param  [in] isLastPass
        //!         flag to indicate the last pass
        //! \param  [in] DeltaQP
        //!         delta qp
        //! \param  [in] prevQpModulationStrength
        //!         previous qp modeulation strength
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculateDeltaQP(
            uint8_t  &QpModulationStrength,
            bool     &initDeltaQP,
            bool     isLastPass,
            uint8_t  &DeltaQP,
            uint32_t &prevQpModulationStrength);

    MEDIA_CLASS_DEFINE_END(encode__EncodeLPLA)
    };
} // encode

#endif
