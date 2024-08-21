/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     decode_vp8_mem_compression_xe2_hpm.h
//! \brief    Defines the common interface for VP8 decode mmc for Xe2_HPM
//! \details  The VP8 decode media mmc is to handle decode mmc operations
//!

#ifndef __VP8_DECODE_MEM_COMPRESSION_XE2_HPM_H__
#define __VP8_DECODE_MEM_COMPRESSION_XE2_HPM_H__

#include "decode_vp8_mem_compression.h"
#include "decode_mem_compression_xe2_hpm.h"

#ifdef _MMC_SUPPORTED

namespace decode
{

class Vp8DecodeMemCompXe2_Hpm : public DecodeMemCompXe2_Hpm, public Vp8DecodeMemComp
{
public:
    //!
    //! \brief    Constructor
    //!
    Vp8DecodeMemCompXe2_Hpm(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Vp8DecodeMemCompXe2_Hpm(const DecodeMemComp &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Vp8DecodeMemCompXe2_Hpm &operator=(const Vp8DecodeMemCompXe2_Hpm &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Vp8DecodeMemCompXe2_Hpm(){};

    //!
    //! \brief    CheckReferenceList
    //!
    virtual MOS_STATUS CheckReferenceList(
        Vp8BasicFeature    &vp8BasicFeature,
        MOS_MEMCOMP_STATE  &PostDeblockSurfMmcState,
        MOS_MEMCOMP_STATE  &PreDeblockSurfMmcState);

    //!
    //! \brief    SetPipeBufAddr
    //!
    virtual MOS_STATUS SetPipeBufAddr(
        Vp8BasicFeature   &vp8BasicFeature,
        MOS_MEMCOMP_STATE &PostDeblockSurfMmcState,
        MOS_MEMCOMP_STATE &PreDeblockSurfMmcState);

MEDIA_CLASS_DEFINE_END(decode__Vp8DecodeMemCompXe2_Hpm)
};

}  // namespace decode
#endif
#endif  //__VP8_DECODE_MEM_COMPRESSION_XE2_HPM_H__
