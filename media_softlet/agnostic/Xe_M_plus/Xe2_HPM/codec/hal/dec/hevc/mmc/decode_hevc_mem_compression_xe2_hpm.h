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
//! \file     decode_hevc_mem_compression_xe2_2pm.h
//! \brief    Defines the common interface for Hevc decode mmc for Xe2_HPM
//! \details  The Hevc decode media mmc is to handle decode mmc operations
//!

#ifndef __HEVC_DECODE_MEM_COMPRESSION_XE2_HPM_H__
#define __HEVC_DECODE_MEM_COMPRESSION_XE2_HPM_H__

#include "decode_hevc_mem_compression.h"
#include "decode_mem_compression_xe2_hpm.h"

namespace decode
{

class HevcDecodeMemCompXe2_Hpm : public DecodeMemCompXe2_Hpm, public HevcDecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    HevcDecodeMemCompXe2_Hpm(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    HevcDecodeMemCompXe2_Hpm(const DecodeMemComp &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    HevcDecodeMemCompXe2_Hpm &operator=(const HevcDecodeMemCompXe2_Hpm &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~HevcDecodeMemCompXe2_Hpm(){};

    virtual MOS_STATUS CheckReferenceList(
        HevcBasicFeature  &hevcBasicFeature,
        MOS_MEMCOMP_STATE &postDeblockSurfMmcState,
        MOS_MEMCOMP_STATE &preDeblockSurfMmcState,
        PMOS_RESOURCE     *presReferences) override;

MEDIA_CLASS_DEFINE_END(decode__HevcDecodeMemCompXe2_Hpm)
};

}  // namespace decode
#endif  //__HEVC_DECODE_MEM_COMPRESSION_XE2_HPM_H__
