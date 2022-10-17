/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_mpeg2_mem_compression.h
//! \brief    Defines the common interface for Mpeg2 decode mmc
//! \details  The Mpeg2 decode media mmc is to handle decode mmc operations
//!

#ifndef __MPEG2_DECODE_MEM_COMPRESSION_H__
#define __MPEG2_DECODE_MEM_COMPRESSION_H__

#include "decode_mem_compression.h"
#include "decode_mpeg2_basic_feature.h"

namespace decode
{

class Mpeg2DecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    Mpeg2DecodeMemComp(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Mpeg2DecodeMemComp(const DecodeMemComp&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Mpeg2DecodeMemComp& operator=(const Mpeg2DecodeMemComp&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Mpeg2DecodeMemComp() {};

    virtual MOS_STATUS CheckReferenceList(
        Mpeg2BasicFeature &mpeg2BasicFeature, MOS_MEMCOMP_STATE &preDeblockSurfMmcState, MOS_MEMCOMP_STATE &postDeblockSurfMmcState);

protected:
    PMOS_INTERFACE m_osInterface = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeMemComp)
};

}

#endif //__MPEG2_DECODE_MEM_COMPRESSION_H__
