/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     decode_mem_compression.h
//! \brief    Defines the common interface for decode mmc
//! \details  The decode media mmc is to handle decode mmc operations
//!

#ifndef __MEDIA_DECODE_MEM_COMPRESSION_H__
#define __MEDIA_DECODE_MEM_COMPRESSION_H__

#include "media_mem_compression.h"
#include "codec_hw_next.h"

class MhwMiInterface;
class DecodeMemComp : public MediaMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    DecodeMemComp(CodechalHwInterfaceNext *hwInterface, PMOS_INTERFACE osInterface = nullptr);

    //!
    //! \brief    Copy constructor
    //!
    DecodeMemComp(const DecodeMemComp&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    DecodeMemComp& operator=(const DecodeMemComp&) = delete;

    //!
    //! \brief    IsMmcEnabled
    //!
    bool IsMmcEnabled();

    //!
    //! \brief    Destructor
    //!
    virtual ~DecodeMemComp() {};

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface);

    MOS_STATUS ReportSurfaceMmcMode(PMOS_SURFACE surface);
#endif

protected:
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                    m_userFeatureUpdated = false;              //!< Inidate if mmc user feature key for decode is updated

    uint32_t                m_compressibleId  = 0;
    uint32_t                m_compressModeId  = 0;
#endif

    void InitDecodeMmc(CodechalHwInterfaceNext *hwInterface);

    bool m_mmcEnabledForDecode = false;  //!< Indicate if mmc is enabled for decode
    std::shared_ptr<mhw::mi::Itf> m_miItf; //!< Point to MI interface

MEDIA_CLASS_DEFINE_END(DecodeMemComp)
};

#endif //__MEDIA_DECODE_MEM_COMPRESSION_H__
