/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     decode_features.cpp
//! \brief    Defines the common interface for decode decode_features
//! \details  The decode pipeline interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#include "decode_basic_feature.h"
#include "decode_utils.h"

namespace decode {

DecodeBasicFeature::DecodeBasicFeature(
    DecodeAllocator *allocator,
    CodechalHwInterface *hwInterface):
    m_hwInterface(hwInterface), m_allocator(allocator)
{
    if(hwInterface != nullptr)
    {
        PMOS_INTERFACE osInterface  = hwInterface->GetOsInterface();
        if (osInterface != nullptr)
        {
            MEDIA_WA_TABLE* waTable = osInterface->pfnGetWaTable(osInterface);
            m_useDummyReference = (waTable != nullptr) ? MEDIA_IS_WA(waTable, WaDummyReference) : false;
        }
    }

    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));
    MOS_ZeroMemory(&m_dummyReference, sizeof(m_dummyReference));
}

DecodeBasicFeature::~DecodeBasicFeature()
{
    if (m_dummyReferenceStatus == CODECHAL_DUMMY_REFERENCE_ALLOCATED)
    {
        m_allocator->Destroy(m_dummyReference);
    }
}

MOS_STATUS DecodeBasicFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(setting);
    DECODE_CHK_NULL(m_allocator);

    CodechalSetting *codecSettings = (CodechalSetting*)setting;
    m_standard      = static_cast<CODECHAL_STANDARD>(codecSettings->standard);
    m_mode          = static_cast<CODECHAL_MODE>(codecSettings->mode);
    m_codecFunction = codecSettings->codecFunction;

    m_is10Bit       = (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? true : false;
    m_chromaFormat  = static_cast<HCP_CHROMA_FORMAT_IDC>(codecSettings->chromaFormat);
    m_bitDepth      = (codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_8_BITS) ?
                        8 : ((codecSettings->lumaChromaDepth & CODECHAL_LUMA_CHROMA_DEPTH_10_BITS) ? 10 : 12);

    m_width         = codecSettings->width;
    m_height        = codecSettings->height;
    m_picWidthInMb  = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width);
    m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height);

    m_disableDecodeSyncLock = codecSettings->disableDecodeSyncLock ? true : false;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeBasicFeature::Update(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    CodechalDecodeParams* decodeParams = (CodechalDecodeParams*)params;

    m_dataSize              = decodeParams->m_dataSize; // The bitstream size will be properly set by SetRequiredBitstreamSize later.
    m_dataOffset            = decodeParams->m_dataOffset;
    m_numSlices             = decodeParams->m_numSlices;

    DECODE_CHK_NULL(decodeParams->m_destSurface);
    m_destSurface           = *(decodeParams->m_destSurface);
    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&m_destSurface));

    DECODE_CHK_NULL(decodeParams->m_dataBuffer);
    m_resDataBuffer.OsResource = *(decodeParams->m_dataBuffer);
    m_allocator->UpdateResoreceUsageType(&m_resDataBuffer.OsResource, resourceInputBitstream);

    if (m_useDummyReference)
    {
        m_dummyReference.OsResource = decodeParams->m_destSurface->OsResource;
        m_dummyReferenceStatus = CODECHAL_DUMMY_REFERENCE_DEST_SURFACE;
    }

    return MOS_STATUS_SUCCESS;
}

}
