/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_mem_compression_g12.h
//! \brief    Defines the common interface for decode mmc
//! \details  The decode mmc is to handle mmc operations
//!

#ifndef __MEDIA_DECODE_MEM_COMPRESSION_G12_H__
#define __MEDIA_DECODE_MEM_COMPRESSION_G12_H__

#include "decode_mem_compression.h"
#include "codec_mem_compression_g12.h"
#include "codechal_hw.h"

class DecodeMemCompG12 : public DecodeMemComp, public CodecMmcAuxTableG12
{
    public:
    //!
    //! \brief    Construct
    //!
    DecodeMemCompG12(CodechalHwInterface *hwInterface);

    //!
    //! \brief    Destructor
    //!
    virtual ~DecodeMemCompG12() {};

    //!
    //! \brief    SendPrologCmd
    //!
    virtual MOS_STATUS SendPrologCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool bRcsIsUsed);

    MhwMiInterface *m_mhwMiInterface = nullptr;  //!< Point to MI interface

MEDIA_CLASS_DEFINE_END(DecodeMemCompG12)
};

#endif //__MEDIA_DECODE_MEM_COMPRESSION_G12_H__
