/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vp9_mem_compression.h
//! \brief    Defines the common interface for VP9 decode mmc
//! \details  The VP9 decode media mmc is to handle decode mmc operations
//!

#ifndef __VP9_DECODE_MEM_COMPRESSION_H__
#define __VP9_DECODE_MEM_COMPRESSION_H__

#include "decode_mem_compression.h"
#include "decode_vp9_basic_feature.h"

namespace decode
{
class Vp9DecodeMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    Vp9DecodeMemComp(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    Vp9DecodeMemComp(const DecodeMemComp &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Vp9DecodeMemComp &operator=(const Vp9DecodeMemComp &) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Vp9DecodeMemComp(){};

    virtual MOS_STATUS CheckReferenceList(Vp9BasicFeature &vp9BasicFeature,
        MOS_MEMCOMP_STATE &postDeblockSurfMmcState,
        MOS_MEMCOMP_STATE &preDeblockSurfMmcState,
        PMOS_RESOURCE     *presReferences);

    MOS_STATUS SetRefSurfaceMask(
        Vp9BasicFeature         &vp9BasicFeature,
        MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[]);

    MOS_STATUS SetRefSurfaceCompressionFormat(
        Vp9BasicFeature         &vp9BasicFeature,
        MHW_VDBOX_SURFACE_PARAMS refSurfaceParams[]);

    virtual MOS_STATUS SetRefSurfaceMask(
        Vp9BasicFeature                       &vp9BasicFeature,
        mhw::vdbox::hcp::HCP_SURFACE_STATE_PAR refSurfaceParams[]);

protected:
    PMOS_INTERFACE m_osInterface = nullptr;
    uint8_t        m_skipMask    = 0;

MEDIA_CLASS_DEFINE_END(decode__Vp9DecodeMemComp)
};
}  // namespace decode

#endif  //__VP9_DECODE_MEM_COMPRESSION_H__
