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

uint16_t CodechalEncodeAllocator::MosToAllocatorCodec(uint32_t codec)
{
    uint16_t allocCodec = 0;

    if (CODECHAL_AVC == codec)
    {
        allocCodec = allocatorAVC;
    }
    else if (CODECHAL_HEVC == codec)
    {
        allocCodec = allocatorHEVC;
    }
    else if (CODECHAL_JPEG == codec)
    {
        allocCodec = allocatorJPEG;
    }
    else if (CODECHAL_MPEG2 == codec)
    {
        allocCodec = allocatorMPEG2;
    }
    else if (CODECHAL_VP9 == codec)
    {
        allocCodec = allocatorVP9;
    }
    else if (CODECHAL_VP8 == codec)
    {
        allocCodec = allocatorVP8;
    }

    return allocCodec;
}

uint16_t CodechalEncodeAllocator::MosToAllocatorFormat(MOS_FORMAT format)
{
    uint16_t allocFormat = 0;

    switch (format)
    {
    case Format_Buffer :
        allocFormat = allocatorLinearBuffer;
        break;
    case Format_Any :
        allocFormat = allocatorBatchBuffer;
        break;
    case Format_NV12 :
    case Format_YUY2 :
    case Format_P208 :
        allocFormat = allocatorSurface;
        break;
    default :
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid format type = %d", format);
        break;
    }

    return allocFormat;
}

uint16_t CodechalEncodeAllocator::MosToAllocatorTile(MOS_TILE_TYPE type)
{
    uint16_t allocTile = 0;

    if (MOS_TILE_LINEAR == type)
    {
        allocTile = allocatorTileLinear;
    }
    else if (MOS_TILE_Y == type)
    {
        allocTile = allocatorTileY;
    }
    else if (MOS_TILE_YF == type)
    {
        allocTile = allocatorTileYf;
    }
    else
    {
        allocTile = allocatorTileYs;
    }

    return allocTile;
}

void* CodechalEncodeAllocator::AllocateResource(
    uint32_t codec, uint32_t width, uint32_t height, ResourceName name, const char *bufName,
    uint8_t index, bool zeroOnAllocation, MOS_FORMAT format, MOS_TILE_TYPE tile)
{
    RESOURCE_TAG resTag;
    MOS_ZeroMemory(&resTag, sizeof(resTag));

    // set buffer name and index
    resTag.typeID = SetResourceID(codec, name, index);

    resTag.format = MosToAllocatorFormat(format);
    resTag.tile   = MosToAllocatorTile(tile);
    resTag.zeroOnAllocation = zeroOnAllocation;

    // set type and size info, and allocate buffer
    void* buffer = nullptr;
    if (allocatorLinearBuffer == resTag.format)
    {
        resTag.size = width;
        resTag.type = allocator1D;
        buffer = Allocate1DBuffer(resTag.tag, width, zeroOnAllocation);
    }
    else if (allocatorBatchBuffer == resTag.format)
    {
        resTag.size = width;
        resTag.type = allocatorBatch;
        buffer = AllocateBatchBuffer(resTag.tag, width, zeroOnAllocation);
    }
    else if (allocatorSurface == resTag.format)
    {
        resTag.width = (uint16_t)width;
        resTag.height = (uint16_t)height;
        resTag.type = allocator2D;
        buffer = Allocate2DBuffer(resTag.tag, width, height, format, tile, zeroOnAllocation);
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid format type = %d", format);
    }

    CODECHAL_ENCODE_NORMALMESSAGE("Alloc ID = 0x%x, type = %d, codec = %d, format = %d, tile = %d, zero = %d, width = %d, height = %d, size = %d",
        resTag.typeID, resTag.type, resTag.codec, resTag.format, resTag.tile, resTag.zeroOnAllocation, resTag.width, resTag.height, resTag.size);

    return buffer;
}

void* CodechalEncodeAllocator::GetResource(uint32_t codec, ResourceName name, uint8_t index)
{
    // find the resource from list
    return GetResourcePointer(SetResourceID(codec, name, index), matchLevel1);
}

uint32_t CodechalEncodeAllocator::GetResourceSize(uint32_t codec, ResourceName name, uint8_t index)
{
    RESOURCE_TAG resTag;
    MOS_ZeroMemory(&resTag, sizeof(resTag));

    if ((resTag.tag = GetResourceTag(SetResourceID(codec, name, index), matchLevel1)))
    {
        return resTag.size;
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
    RESOURCE_TAG resTag;
    MOS_ZeroMemory(&resTag, sizeof(resTag));

    resTag.typeID = name;
    resTag.codec  = MosToAllocatorCodec(codec);
    if (IsTrackedBuffer(name) || IsRecycleBuffer(name))
    {
        resTag.trackedRecycleBufferIndex = index;
    }
    return resTag.typeID;
}

uint16_t CodechalEncodeAllocator::GetResourceID(uint64_t resourceTag, Match level)
{
    RESOURCE_TAG resTag;
    MOS_ZeroMemory(&resTag, sizeof(resTag));

    resTag.tag = resourceTag;
    if (matchLevel0 == level)
    {
        // extract codec/name/indx
        resTag.typeID &= m_bufNameMask;

        // only match name, clear index for tracked/recycled buffer
        resTag.typeID |= m_bufIndexMask;
    }
    else if (matchLevel1 == level)
    {
        // extract codec/name/indx
        resTag.typeID &= m_bufNameMask;
    }
    return resTag.typeID;
}

CodechalEncodeAllocator::CodechalEncodeAllocator(CodechalEncoderState* encoder)
    : CodechalAllocator(encoder->GetOsInterface())
{
    // Initilize interface pointers
    m_encoder = encoder;
}

CodechalEncodeAllocator::~CodechalEncodeAllocator()
{
}
