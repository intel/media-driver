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
//! \file     decode_vp9_mem_compression_m12.h
//! \brief    Defines the common interface for Vp9 decode mmc for M12
//! \details  The Vp9 decode media mmc is to handle decode mmc operations
//!

#ifndef __VP9_DECODE_MEM_COMPRESSION_M12_H__
#define __VP9_DECODE_MEM_COMPRESSION_M12_H__

#include "decode_vp9_mem_compression.h"
#include "decode_mem_compression_g12.h"

namespace decode
{
class Vp9DecodeMemCompM12 : public DecodeMemCompG12, public Vp9DecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    Vp9DecodeMemCompM12(CodechalHwInterface *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Vp9DecodeMemCompM12(const DecodeMemComp &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Vp9DecodeMemCompM12 &operator=(const Vp9DecodeMemCompM12 &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Vp9DecodeMemCompM12(){};

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeMemCompM12)
};

}  // namespace decode
#endif  //__VP9_DECODE_MEM_COMPRESSION_M12_H__
