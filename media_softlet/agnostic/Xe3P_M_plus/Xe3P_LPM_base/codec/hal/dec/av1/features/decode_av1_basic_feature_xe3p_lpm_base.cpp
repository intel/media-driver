/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     decode_av1_basic_feature_xe3p_lpm_base.cpp
//! \brief    Defines the common interface for decode av1 parameter
//!

#include "decode_av1_basic_feature_xe3p_lpm_base.h"

namespace decode
{
Av1BasicFeatureXe3P_Lpm_Base::Av1BasicFeatureXe3P_Lpm_Base(DecodeAllocator *allocator,
        void* hwInterface, PMOS_INTERFACE osInterface) :
        Av1BasicFeature(allocator, hwInterface, osInterface)
    {
        DECODE_FUNC_CALL()
        if (hwInterface != nullptr)
        {
            m_avpItf      = static_cast<CodechalHwInterfaceXe3P_Lpm_Base *>(hwInterface)->GetAvpInterfaceNext();
            m_osInterface = osInterface;
        }
    }

    MOS_STATUS Av1BasicFeatureXe3P_Lpm_Base::CheckProfileAndSubsampling()
    {
        DECODE_FUNC_CALL();

        // Profile and subsampling
        bool is420 = (1 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX && 1 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY);
        bool is444 = (0 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX && 0 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY);
        bool is422 = (1 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX && 0 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY);

        if (m_av1PicParams->m_seqInfoFlags.m_fields.m_monoChrome ||
            m_av1PicParams->m_profile > 2 ||
            !(is420 || is444 || is422) ||
            !(m_av1PicParams->m_bitDepthIdx == 0 || m_av1PicParams->m_bitDepthIdx == 1))
        {
            DECODE_ASSERTMESSAGE("Only 4:2:0, 4:4:4 and 4:2:2 8bit and 10bit are supported!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_av1PicParams->m_profile == 0 &&
            !(1 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX && 1 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY))
        {
            DECODE_ASSERTMESSAGE("Profile0 only support 420!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_av1PicParams->m_profile == 1 &&
            !(0 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingX && 0 == m_av1PicParams->m_seqInfoFlags.m_fields.m_subsamplingY))
        {
            DECODE_ASSERTMESSAGE("Profile1 only support 444!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_av1PicParams->m_profile == 2 && !is422)
        {
            DECODE_ASSERTMESSAGE("Profile2 only support 422!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BasicFeatureXe3P_Lpm_Base::ErrorDetectAndConceal()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_STATUS(Av1BasicFeature::ErrorDetectAndConceal());

        if (1 == m_av1PicParams->m_profile || 2 == m_av1PicParams->m_profile)  // 444/422 format
        {
            if (m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
            {
                DECODE_VERBOSEMESSAGE("AV1 444/422 don't support film grain!");
                m_av1PicParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain = 0;
            }
            if (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)
            {
                DECODE_ASSERTMESSAGE("AV1 444/422 don't support LST!");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

}  // namespace decode
