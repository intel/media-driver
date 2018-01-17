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
//! \file     codechal_encode_allocator.cpp
//! \brief    Resource allocator for all buffer/surface used by encoder
//!

#include "codechal_encode_allocator.h"
#include "codechal_encoder_base.h"

//!
//! \enum     AllocatorCodec
//! \brief    Allocator codec
//!
enum AllocatorCodec
{
    allocatorAVC = 0,
    allocatorHEVC = 1,
    allocatorJPEG = 2,
    allocatorMPEG2 = 3,
    allocatorVP9 = 4,
    allocatorVP8 = 5
};

//!
//! \enum     AllocatorType
//! \brief    Allocator type
//!
enum AllocatorType
{
    allocator1D = 0,
    allocator2D = 1,
    allocatorBatch = 2
};

//!
//! \enum     AllocatorFormat
//! \brief    Allocator format
//!
enum AllocatorFormat
{
    allocatorLinearBuffer = 0,
    allocatorBatchBuffer = 1,
    allocatorSurface = 2
};

//!
//! \enum     AllocatorTile
//! \brief    Allocator tile
//!
enum AllocatorTile
{
    allocatorTileLinear = 0,
    allocatorTileY = 1,
    allocatorTileYf = 2,
    allocatorTileYs = 3
};

void CodechalEncodeAllocator::MosToAllocatorCodec(uint32_t codec)
{
    if (CODECHAL_AVC == codec)
    {
        m_codec = allocatorAVC;
    }
    else if (CODECHAL_HEVC == codec)
    {
        m_codec = allocatorHEVC;
    }
    else if (CODECHAL_JPEG == codec)
    {
        m_codec = allocatorJPEG;
    }
    else if (CODECHAL_MPEG2 == codec)
    {
        m_codec = allocatorMPEG2;
    }
    else if (CODECHAL_VP9 == codec)
    {
        m_codec = allocatorVP9;
    }
    else if (CODECHAL_VP8 == codec)
    {
        m_codec = allocatorVP8;
    }
}

void CodechalEncodeAllocator::MosToAllocatorFormat(MOS_FORMAT format)
{
    switch (format)
    {
    case Format_Buffer :
        m_format = allocatorLinearBuffer;
        break;
    case Format_Any :
        m_format = allocatorBatchBuffer;
        break;
    case Format_NV12 :
    case Format_YUY2 :
    case Format_P208 :
        m_format = allocatorSurface;
        break;
    default :
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid format type = %d", format);
        break;
    }
}

void CodechalEncodeAllocator::MosToAllocatorTile(MOS_TILE_TYPE type)
{
    if (MOS_TILE_LINEAR == type)
    {
        m_tile = allocatorTileLinear;
    }
    else if (MOS_TILE_Y == type)
    {
        m_tile = allocatorTileY;
    }
    else if (MOS_TILE_YF == type)
    {
        m_tile = allocatorTileYf;
    }
    else
    {
        m_tile = allocatorTileYs;
    }
}

void* CodechalEncodeAllocator::AllocateResource(
    uint32_t codec, uint32_t width, uint32_t height, ResourceName name,
    uint8_t index, bool zeroOnAllocation, MOS_FORMAT format, MOS_TILE_TYPE tile)
{
    // set buffer name and index
    SetResourceID(codec, name, index);

    MosToAllocatorFormat(format);
    MosToAllocatorTile(tile);
    m_zeroOnAllocation = zeroOnAllocation;

    // set type and size info
    if (allocatorLinearBuffer == m_format)
    {
        m_size = width;
        m_type = allocator1D;
    }
    else if (allocatorBatchBuffer == m_format)
    {
        m_size = width;
        m_type = allocatorBatch;
    }
    else if (allocatorSurface == m_format)
    {
        m_width = (uint16_t)width;
        m_height = (uint16_t)height;
        m_type = allocator2D;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid format type = %d", format);
        return nullptr;
    }

    CODECHAL_ENCODE_NORMALMESSAGE("Alloc ID = 0x%x, type = %d, codec = %d, format = %d, tile = %d, zero = %d, width = %d, height = %d, size = %d", 
        m_typeID, m_type, m_codec, m_format, m_tile, m_zeroOnAllocation, m_width, m_height, m_size);

    void* buffer = nullptr;
    if (allocator1D == m_type)
    {
        buffer = Allocate1DBuffer(m_tag, width, zeroOnAllocation);
    }
    else if (allocator2D == m_type)
    {
        buffer = Allocate2DBuffer(m_tag, width, height, format, tile, zeroOnAllocation);
    }
    else if (allocatorBatch == m_type)
    {
        buffer = AllocateBatchBuffer(m_tag, width, zeroOnAllocation);
    }

    return buffer;
}

void* CodechalEncodeAllocator::GetResource(uint32_t codec, ResourceName name, uint8_t index)
{
    // find the resource from list
    return GetResourcePointer(SetResourceID(codec, name, index), matchLevel1);
}

uint32_t CodechalEncodeAllocator::GetResourceSize(uint32_t codec, ResourceName name, uint8_t index)
{
    if (m_tag = GetResourceTag(SetResourceID(codec, name, index), matchLevel1))
    {
        return m_size;
    }
    else
    {
        return 0;
    }
}

void CodechalEncodeAllocator::ReleaseResource(uint32_t codec, ResourceName name, uint8_t index)
{
    CodechalAllocator::ReleaseResource(SetResourceID(codec, name, index), matchLevel1);
}

uint16_t CodechalEncodeAllocator::SetResourceID(uint32_t codec, ResourceName name, uint8_t index)
{
    m_typeID = name;
    MosToAllocatorCodec(codec);
    if (IsTrackedBuffer(name) || IsRecycleBuffer(name))
    {
        m_trackedRecycleBufferIndex = index;
    }
    return m_typeID;
}

uint16_t CodechalEncodeAllocator::GetResourceID(uint64_t resourceTag, Match level)
{
    m_tag = resourceTag;
    if (matchLevel0 == level)
    {
        // extract codec/name/indx
        m_typeID &= m_bufNameMask;

        // only match name, clear index for tracked/recycled buffer
        m_typeID |= m_bufIndexMask;
    }
    else if (matchLevel1 == level)
    {
        // extract codec/name/indx
        m_typeID &= m_bufNameMask;
    }
    return m_typeID;
}

CodechalEncodeAllocator::CodechalEncodeAllocator(CodechalEncoderState* encoder)
    : CodechalAllocator(encoder->GetOsInterface())
{
    // Initilize interface pointers
    m_encoder = encoder;
    m_tag = 0;
}

CodechalEncodeAllocator::~CodechalEncodeAllocator()
{
}
