/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     decode_hevc_mem_compression.h
//! \brief    Defines the common interface for Hevc decode mmc
//! \details  The Hevc decode media mmc is to handle decode mmc operations
//!

#ifndef __HEVC_DECODE_MEM_COMPRESSION_H__
#define __HEVC_DECODE_MEM_COMPRESSION_H__

#include "decode_mem_compression.h"
#include "decode_hevc_basic_feature.h"

namespace decode
{

class HevcDecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    HevcDecodeMemComp(CodechalHwInterface *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    HevcDecodeMemComp(const DecodeMemComp&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    HevcDecodeMemComp& operator=(const HevcDecodeMemComp&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~HevcDecodeMemComp() {};

    MOS_STATUS CheckReferenceList(
        HevcBasicFeature &hevcBasicFeature, MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

    MOS_STATUS CheckReferenceList(
        HevcBasicFeature  &hevcBasicFeature,
        MOS_MEMCOMP_STATE &postDeblockSurfMmcState,
        MOS_MEMCOMP_STATE &preDeblockSurfMmcState,
        PMOS_RESOURCE     *presReferences);

    MOS_STATUS SetRefSurfaceMask(
        HevcBasicFeature                     &hevcBasicFeature,
        const MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams,
        MHW_VDBOX_SURFACE_PARAMS             &refSurfaceParams);

    MOS_STATUS SetRefSurfaceMask(
        HevcBasicFeature    &hevcBasicFeature,
        const PMOS_RESOURCE *presReferences,
        uint8_t             &mmcSkipMask);

protected:
    PMOS_INTERFACE m_osInterface = nullptr;

MEDIA_CLASS_DEFINE_END(HevcDecodeMemComp)
};

}

#endif //__HEVC_DECODE_MEM_COMPRESSION_H__
