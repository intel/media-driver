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
//! \file     decode_mpeg2_mem_compression_xe2_hpm.h
//! \brief    Defines the common interface for Mpeg2 decode mmc for Xe2_HPM
//! \details  The Mpeg2 decode media mmc is to handle decode mmc operations
//!

#ifndef __MPEG2_DECODE_MEM_COMPRESSION_XE2_HPM_H__
#define __MPEG2_DECODE_MEM_COMPRESSION_XE2_HPM_H__

#include "decode_mpeg2_mem_compression.h"
#include "decode_mem_compression_xe2_hpm.h"

namespace decode
{

class Mpeg2DecodeMemCompXe2_Hpm : public DecodeMemCompXe2_Hpm, public Mpeg2DecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    Mpeg2DecodeMemCompXe2_Hpm(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Mpeg2DecodeMemCompXe2_Hpm(const DecodeMemComp &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Mpeg2DecodeMemCompXe2_Hpm &operator=(const Mpeg2DecodeMemCompXe2_Hpm &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Mpeg2DecodeMemCompXe2_Hpm(){};

    virtual MOS_STATUS CheckReferenceList(
        Mpeg2BasicFeature &mpeg2BasicFeature,
        MOS_MEMCOMP_STATE &preDeblockSurfMmcState,
        MOS_MEMCOMP_STATE &postDeblockSurfMmcState) override;

MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodeMemCompXe2_Hpm)
};

}
#endif //__MPEG2_DECODE_MEM_COMPRESSION_XE2_HPM_H__
