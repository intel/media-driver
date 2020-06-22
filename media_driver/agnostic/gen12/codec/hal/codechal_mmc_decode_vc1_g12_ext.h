/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     codechal_mmc_decode_vc1_g12_ext.h
//! \brief    Defines the public interface for VC1 Gen12 CodecHal Media Memory Compression
//!
#ifndef __CODECHAL_MMC_DECODE_VC1_G12_EXT_H__
#define __CODECHAL_MMC_DECODE_VC1_G12_EXT_H__

#include "codechal_mmc_g12.h"
#include "codechal_decode_vc1.h"

//! \class CodechalMmcDecodeVc1G12Ext
//! \brief Gen12 Media memory compression decode VC1 state. This class defines the member fields
//!        functions etc used by decode VC1 memory compression. 
//!
class CodechalMmcDecodeVc1G12Ext
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalMmcDecodeVc1G12Ext(
        CodechalHwInterface *hwInterface,
        CodecHalMmcState *mmcState);

    //!
    //! \brief    Destructor
    //!
    ~CodechalMmcDecodeVc1G12Ext() {}

    MOS_STATUS CopyAuxSurfForSkip(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       srcResource,
        PMOS_RESOURCE       destResource);

private:
    CodechalHwInterface    *m_hwInterface = nullptr;
    CodecHalMmcState       *m_mmcState = nullptr;
};

#endif  // __CODECHAL_MMC_DECODE_VC1_G12_EXT_H__