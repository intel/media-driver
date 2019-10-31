/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_mmc_decode_vc1_g12.h
//! \brief    Defines the public interface for Gen12 CodecHal Media Memory Compression
//!
#ifndef __CODECHAL_MMC_DECODE_VC1_G12_H__
#define __CODECHAL_MMC_DECODE_VC1_G12_H__

#include "codechal_mmc_g12.h"
#include "codechal_decode_vc1.h"
#include "codechal_mmc_decode_vc1_g12_ext.h"

//! \class CodechalMmcDecodeVc1G12
//! \brief Gen12 Media memory compression decode VC1 state. This class defines the member fields
//!        functions etc used by decode VC1 memory compression. 
//!
class CodechalMmcDecodeVc1G12 : public CodecHalMmcStateG12
{
public:

    //!
    //! \brief    Constructor
    //!
    CodechalMmcDecodeVc1G12(
        CodechalHwInterface    *hwInterface,
        void *standardState);

    //!
    //! \brief    Destructor
    //!
    ~CodechalMmcDecodeVc1G12();

    MOS_STATUS SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        PMOS_COMMAND_BUFFER cmdBuffer = nullptr) override;

    MOS_STATUS CopyAuxSurfForSkip(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       srcResource,
        PMOS_RESOURCE       destResource);

    CodechalDecodeVc1           *m_vc1State = nullptr;      //!< Pinter to VC1 decode state
    CodechalHwInterface         *m_hwInterface = nullptr;   //!< Pointer to Hw Interface
    CodechalMmcDecodeVc1G12Ext  *m_vc1MmcExt = nullptr;     //!< Pointer to VC1 MMC ext instance
};

#endif  // __CODECHAL_MMC_DECODE_VC1_G12_H__
