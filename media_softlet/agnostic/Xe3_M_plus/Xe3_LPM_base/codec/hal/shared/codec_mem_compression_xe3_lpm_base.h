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
//! \file     codec_mem_compression_xe3_lpm_base.h
//! \brief    Defines the common interface for codec mmc
//! \details  The codec mmc is to handle mmc operations
//!

#ifndef __MEDIA_CODEC_MEM_COMPRESSION_XE3_LPM_BASE_H__
#define __MEDIA_CODEC_MEM_COMPRESSION_XE3_LPM_BASE_H__

# include "media_context.h"

class CodecMmcAuxTableXe3_Lpm_Base
{
public:
    //!
    //! \brief    Construct
    //!
    CodecMmcAuxTableXe3_Lpm_Base() {};

    //!
    //! \brief    Destructor
    //!
    virtual ~CodecMmcAuxTableXe3_Lpm_Base() {};

    //!
    //! \brief    LoadAuxTableMmio
    //!
    MOS_STATUS LoadAuxTableMmio(
        PMOS_INTERFACE      osItf,
        mhw::mi::Itf        &miItf,
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                bRcsIsUsed);

MEDIA_CLASS_DEFINE_END(CodecMmcAuxTableXe3_Lpm_Base)
};

#endif //__MEDIA_CODEC_MEM_COMPRESSION_XE3_LPM_BASE_H__
