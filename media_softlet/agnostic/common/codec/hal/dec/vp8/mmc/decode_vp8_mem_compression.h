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
//! \file     decode_vp8_mem_compression.h
//! \brief    Defines the common interface for VP8 decode mmc
//! \details  The VP8 decode media mmc is to handle decode mmc operations
//!

#ifndef __VP8_DECODE_MEM_COMPRESSION_H__
#define __VP8_DECODE_MEM_COMPRESSION_H__

#include "decode_mem_compression.h"
#include "decode_vp8_basic_feature.h"

#ifdef _MMC_SUPPORTED

namespace decode
{
class Vp8DecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    Vp8DecodeMemComp(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Vp8DecodeMemComp(const DecodeMemComp &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Vp8DecodeMemComp &operator=(const Vp8DecodeMemComp &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Vp8DecodeMemComp(){};

    //!
    //! \brief    CheckReferenceList
    //!
    virtual MOS_STATUS CheckReferenceList(Vp8BasicFeature &vp8BasicFeature,
                                          MOS_MEMCOMP_STATE & PostDeblockSurfMmcState,
                                          MOS_MEMCOMP_STATE & PreDeblockSurfMmcState);

    //!
    //! \brief    SetPipeBufAddr
    //!
    virtual MOS_STATUS  SetPipeBufAddr(Vp8BasicFeature &vp8BasicFeature,
                                       MOS_MEMCOMP_STATE &PostDeblockSurfMmcState,
                                       MOS_MEMCOMP_STATE &PreDeblockSurfMmcState);

    bool           m_mmcEnabled  = false;

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
    uint8_t        m_skipMask    = 0;

MEDIA_CLASS_DEFINE_END(decode__Vp8DecodeMemComp)
};
}  // namespace decode

#endif  //_MMC_SUPPORTED

#endif  //__VP8_DECODE_MEM_COMPRESSION_H__
