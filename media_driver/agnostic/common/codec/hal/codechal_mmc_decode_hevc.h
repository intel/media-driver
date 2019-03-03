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
//! \file     codechal_mmc_decode_hevc.h
//! \brief    Defines the public interface for CodecHal Media Memory Compression
//!
#ifndef __CODECHAL_MMC_DECODE_HEVC
#define __CODECHAL_MMC_DECODE_HEVC

#include "codechal_mmc.h"
#include "codechal_decode_hevc.h"

//! \class CodechalMmcDecodeHevc
//! \brief Media memory compression decode HEVC state. This class defines the member fields
//!        functions etc used by decode HEVC memory compression. 
//!
class CodechalMmcDecodeHevc:public CodecHalMmcState
{
public:

    //!
    //! \brief    Constructor
    //!
    CodechalMmcDecodeHevc(
        CodechalHwInterface    *hwInterface,
        void *standardState);

    //!
    //! \brief    Destructor
    //!
    ~CodechalMmcDecodeHevc() {};

    MOS_STATUS SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        PMOS_COMMAND_BUFFER cmdBuffer = nullptr) override;

    MOS_STATUS SetRefrenceSync(
        bool disableDecodeSyncLock,
        bool disableLockForTranscode) override;

    MOS_STATUS CheckReferenceList(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams) override;

    CodechalDecodeHevc  *m_hevcState = nullptr;     //!< Pinter to HEVC decode state
};

#endif  // __CODECHAL_MMC_DECODE_HEVC
