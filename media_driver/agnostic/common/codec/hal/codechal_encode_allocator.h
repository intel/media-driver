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
//! \file     codechal_encode_allocator.h
//! \brief    Resource allocator for all buffer/surface used by encoder
//!

#ifndef __CODECHAL_ENCODE_ALLOCATOR_H__
#define __CODECHAL_ENCODE_ALLOCATOR_H__

#include "codechal_allocator.h"

//!
//! \enum     ResourceName
//! \brief    Resource name
//!
enum ResourceName
{
    // common VDEnc buffer
    vdencStats = 0,
    vdencIntraRowStoreScratch = vdencStats + 1,
    vdencEnd = vdencIntraRowStoreScratch,

    // common PAK buffer
    pakStats = vdencEnd + 1,
    pakInfo = pakStats + 1,
    pakEnd = pakInfo,

    // HEVC buffer
    hevcMbCodeBuffer = pakEnd + 1,
    brcInputForEncKernel = hevcMbCodeBuffer + 1,

    // tracked buffer
    trackedBuffer = 512,
    cscSurface = trackedBuffer,
    mbCodeBuffer = cscSurface + 32,
    mvDataBuffer = mbCodeBuffer + 32,
    mvTemporalBuffer = mvDataBuffer + 32, 
    ds4xSurface = mvTemporalBuffer + 32,
    ds2xSurface = ds4xSurface + 32,
    ds16xSurface = ds2xSurface + 32,
    ds32xSurface = ds16xSurface + 32,
    ds4xRecon = ds32xSurface + 32,
    ds8xRecon = ds4xRecon + 32,
    trackedBufferEnd = ds8xRecon,
    
    // recycled buffer
    recycledBuffer = trackedBufferEnd + 32,
    brcReadSurface = recycledBuffer,
    recycledBufferEnd = brcReadSurface
};

//!
//! \class    CodechalEncodeAllocator
//! \brief    This class provides resource management for all codecs to
//!           obtain buffer/surface/resource used by EU kernel and HW.
//!
class CodechalEncodeAllocator : public CodechalAllocator
{
public:
    //!
    //! \brief    Allocate resource
    //!
    //! \return   pointer to resource, NULL if error
    //!
    void* AllocateResource(
        uint32_t codec, uint32_t width, uint32_t height, ResourceName name, uint8_t index = 0,
        bool zeroOnAllocation = false, MOS_FORMAT format = Format_Buffer, MOS_TILE_TYPE tile = MOS_TILE_LINEAR);

    //!
    //! \brief    Return address/pointer of a resource already allocated
    //!
    //! \return   pointer to resource, NULL if the resource not allcoated yet
    //!
    void* GetResource(uint32_t codec, ResourceName name, uint8_t index = 0);

    //!
    //! \brief    Return size of allocated resource
    //!
    //! \return   size of allocated resource
    //!
    uint32_t GetResourceSize(uint32_t codec, ResourceName name, uint8_t index = 0);

    //!
    //! \brief    Release a resource
    //!
    void ReleaseResource(uint32_t codec, ResourceName name, uint8_t index = 0);

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAllocator(CodechalEncoderState* encoder);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAllocator();

private:
    CodechalEncodeAllocator(const CodechalEncodeAllocator&) = delete;
    CodechalEncodeAllocator& operator=(const CodechalEncodeAllocator&) = delete;

    void MosToAllocatorCodec(uint32_t);
    MOS_FORMAT AllocatorToMosFormat();
    void MosToAllocatorFormat(MOS_FORMAT format); 
    MOS_TILE_TYPE AllocatorToMosTile();
    void MosToAllocatorTile(MOS_TILE_TYPE type);
    
    inline bool IsTrackedBuffer(ResourceName name)
    {
        return (trackedBuffer <= name && name <= trackedBufferEnd);
    }

    inline bool IsRecycleBuffer(ResourceName name)
    {
        return (recycledBuffer <= name && name <= recycledBufferEnd);
    }
    
    uint16_t SetResourceID(uint32_t codec, ResourceName name, uint8_t index);
    virtual uint16_t GetResourceID(uint64_t resourceTag, Match level) override;

    CodechalEncoderState*           m_encoder = nullptr;                //!< Pointer to ENCODER base class

    static const uint16_t           m_bufIndexMask = (1 << 5) - 1;
    static const uint16_t           m_bufNameMask = (1 << 14) - 1;

    union
    {
        struct
        {
            union
            {
                struct
                {
                    uint16_t        m_trackedRecycleBufferIndex : MOS_BITFIELD_RANGE(0, 4);
                    uint16_t        m_trackedRecycleBufferName : MOS_BITFIELD_RANGE(5, 10);
                    uint16_t        m_codec : MOS_BITFIELD_RANGE(11, 13); 
                    uint16_t        m_type : MOS_BITFIELD_RANGE(14, 15);
                };
                uint16_t            m_typeID;
            };
            uint16_t                m_format : MOS_BITFIELD_RANGE(0, 3);
            uint16_t                m_tile : MOS_BITFIELD_RANGE(4, 6);
            uint16_t                m_zeroOnAllocation : MOS_BITFIELD_BIT(7);
            uint16_t                m_reserved : MOS_BITFIELD_RANGE(8, 15);
            union
            {
                struct
                {
                    uint16_t        m_width;
                    uint16_t        m_height;
                };
                uint32_t            m_size;
            };
        };
        uint64_t                    m_tag;
    };
};
#endif  // __CODECHAL_ENCODE_ALLOCATOR_H__
