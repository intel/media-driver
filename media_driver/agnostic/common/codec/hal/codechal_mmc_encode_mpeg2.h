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
//! \file     codechal_mmc_encode_mpeg2.h
//! \brief    Defines the public interface for CodecHal Media Memory Compression
//!
#ifndef __CODECHAL_MMC_ENCODE_MPEG2_H__
#define __CODECHAL_MMC_ENCODE_MPEG2_H__

#include "codechal_mmc.h"
#include "codechal_encode_mpeg2.h"

//! \class CodechalMmcEncodeMpeg2
//! \brief Media memory compression encode MPEG2 state. This class defines the member fields
//!        functions etc used by encode MPEG2 memory compression. 
//!
class CodechalMmcEncodeMpeg2:public CodecHalMmcState
{
public:

    //!
    //! \brief    Constructor
    //!
    CodechalMmcEncodeMpeg2(
        CodechalHwInterface    *hwInterface,
        void *standardState);

    //!
    //! \brief    Destructor
    //!
    ~CodechalMmcEncodeMpeg2() {};

    MOS_STATUS SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        PMOS_COMMAND_BUFFER cmdBuffer = nullptr) override;

    CodechalEncodeMpeg2           *m_mpeg2State = nullptr; //!< Pinter to MPEG2 encode state

};

#endif  // __CODECHAL_MMC_ENCODE_AVC_H__
