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
//! \file     codechal_allocator.h
//! \brief    A generic resource allocator
//!

#ifndef __CODECHAL_ALLOCATOR_H__
#define __CODECHAL_ALLOCATOR_H__

#include "codechal.h"

//!
//! This class provides a generic resource allocation service.
//! So far support 3 types: 1D buffer, 2D surface, and batch buffer
//!
class CodechalAllocator
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalAllocator(MOS_INTERFACE*);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalAllocator();

protected:
    //!
    //! \brief    Resource's unique ID contains different level of info, search/match can be performed
    //!           based on different level
    //!
    //! \return   resource's 64-bit tag
    //!
    enum Match
    {
        matchLevel0 = 0,
        matchLevel1 = 1,
        matchAll = 2
    };

    //!
    //! \brief    Get resource's 64-bit tag according to resource's ID and match level
    //!
    //! \return   resource's 64-bit tag
    //!
    uint64_t GetResourceTag(uint16_t resourceID, Match level);

    //!
    //! \brief    Get resource's address/pointer according to resource's ID and match level
    //!
    //! \return   resource's address/pointer, NULL if resource does not exist in the list
    //!
    void* GetResourcePointer(uint16_t resourceID, Match level);

    //!
    //! \brief    Allocate 1D buffer
    //!
    //! \return   pointer to 1D buffer
    //!
    void* Allocate1DBuffer(uint64_t resourceTag, uint32_t size,
        bool zeroOnAllocation = false, const char *bufName = nullptr);

    //!
    //! \brief    Allocate 2D buffer
    //!
    //! \return   pointer to 2D buffer
    //!
    void* Allocate2DBuffer(
        uint64_t resourceTag, uint32_t width, uint32_t height, MOS_FORMAT format,
        MOS_TILE_TYPE tile, bool zeroOnAllocation = false, const char *bufName = nullptr);

    //!
    //! \brief    Allocate batch buffer
    //!
    //! \return   pointer to batch buffer
    //!
    void* AllocateBatchBuffer(uint64_t resourceTag, uint32_t size, bool zeroOnAllocation = false);

    //!
    //! \brief    Release a resource
    //!
    void ReleaseResource(uint16_t resourceID, Match level);

private:
    CodechalAllocator(const CodechalAllocator&) = delete;
    CodechalAllocator& operator=(const CodechalAllocator&) = delete;

    bool Is1DBuffer(uint64_t resourceTag);
    bool Is2DBuffer(uint64_t resourceTag);
    bool IsBatchBuffer(uint64_t resourceTag);
    void ClearResource(MOS_RESOURCE*, size_t);
    void Deallocate(uint64_t tag, void* pointer);

    //!
    //! \brief    Get resource's ID according to different level info for purpose of match at different level
    //!           Derivde class to implement this
    //!
    //! \return   resource's 16-bit ID containing corresponding level-identifiable info
    //!
    virtual uint16_t GetResourceID(uint64_t resourceTag, Match level) = 0;

    MOS_INTERFACE*                      m_osInterface = nullptr;            //!< OS interface
    std::map<uint64_t, void*>           m_resourceList{};                   //!< list of <tag, pointer>
};

#endif  // __CODECHAL_ALLOCATOR_H__