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
//! \file     decode_mpeg2_mem_compression_m12.h
//! \brief    Defines the common interface for Mpeg2 decode mmc for M12
//! \details  The Mpeg2 decode media mmc is to handle decode mmc operations
//!

#ifndef __MPEG2_DECODE_MEM_COMPRESSION_M12_H__
#define __MPEG2_DECODE_MEM_COMPRESSION_M12_H__

#include "decode_mpeg2_mem_compression.h"
#include "decode_mem_compression_g12.h"

namespace decode
{

class Mpeg2DecodeMemCompM12 : public DecodeMemCompG12, public Mpeg2DecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    Mpeg2DecodeMemCompM12(CodechalHwInterface *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Mpeg2DecodeMemCompM12(const DecodeMemComp&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Mpeg2DecodeMemCompM12& operator=(const Mpeg2DecodeMemCompM12&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Mpeg2DecodeMemCompM12() {};

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeMemCompM12)
};

}
#endif //__MPEG2_DECODE_MEM_COMPRESSION_M12_H__