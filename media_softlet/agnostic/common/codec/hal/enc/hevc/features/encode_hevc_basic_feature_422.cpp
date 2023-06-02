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
//! \file     encode_hevc_basic_feature_rsvd.cpp
//! \brief    Defines the common interface for encode hevc basic feature rsvd
//!
#include "encode_hevc_basic_feature_422.h"

namespace encode
{
    MOS_STATUS HevcBasicFeature422::Init(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams, PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        ENCODE_CHK_NULL_RETURN(hevcPicParams);
        if (hevcSeqParams->chroma_format_idc == HCP_CHROMA_FORMAT_YUV422)
        {
            m_is422 = true;
        }

        if (m_is422 && hevcPicParams->tiles_enabled_flag )
        {
            ENCODE_ASSERTMESSAGE(" 422 with multi tiles is not supported.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcBasicFeature422::RegisterMbCodeBuffer(TrackedBuffer *trackedBuf, bool &isRegistered, uint32_t mbCodeSize)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(trackedBuf);

        MOS_ALLOC_GFXRES_PARAMS allocParamsForLinear;
        MOS_ZeroMemory(&allocParamsForLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForLinear.Format   = Format_Buffer;
        allocParamsForLinear.dwMemType        = MOS_MEMPOOL_SYSTEMMEMORY;
        allocParamsForLinear.Flags.bCacheable = true;
        // set the ResUsageType to enable coherency in gmm
        allocParamsForLinear.ResUsageType     = MOS_HW_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM;

        if (mbCodeSize > 0)
        {
            allocParamsForLinear.pBufName = "mbCodeBuffer";
            allocParamsForLinear.dwBytes = mbCodeSize + 8 * CODECHAL_CACHELINE_SIZE;
            ENCODE_CHK_STATUS_RETURN(trackedBuf->RegisterParam(encode::BufferType::mbCodedBuffer, allocParamsForLinear));
            isRegistered = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcBasicFeature422::Update422Format(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams, uint8_t &outputChromaFormat, MOS_FORMAT &reconFormat, bool is10Bit)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        hevcSeqParams->chroma_format_idc =
            outputChromaFormat = HCP_CHROMA_FORMAT_YUV420;
        reconFormat            = is10Bit ? Format_P010 : Format_NV12;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcBasicFeature422::Revert422Format(PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams, uint8_t &outputChromaFormat, MOS_FORMAT &reconFormat, bool is10Bit)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        hevcSeqParams->chroma_format_idc =
            outputChromaFormat = HCP_CHROMA_FORMAT_YUV422;
        reconFormat            = is10Bit ? Format_Y216 : Format_YUY2;

        return MOS_STATUS_SUCCESS;
    }

}
