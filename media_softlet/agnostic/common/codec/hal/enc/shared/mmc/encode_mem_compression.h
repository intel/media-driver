/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_mem_compression.h
//! \brief    Defines the common interface for encode mmc
//! \details  The encode media mmc is to handle encode mmc operations
//!

#ifndef __MEDIA_ENCODE_MEM_COMPRESSION_H__
#define __MEDIA_ENCODE_MEM_COMPRESSION_H__

#include "media_mem_compression.h"
#include "codec_hw_next.h"
#include "mhw_mi_itf.h"

class EncodeMemComp : public MediaMemComp
{
public:
    //!
    //! \brief    Construct
    //!
    EncodeMemComp(CodechalHwInterfaceNext *hwInterface);

    //!
    //! \brief    Copy constructor
    //!
    EncodeMemComp(const EncodeMemComp&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    EncodeMemComp& operator=(const EncodeMemComp&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~EncodeMemComp() {};

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface);
#endif

protected:
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                    m_userFeatureUpdated = false; //!< Inidate if mmc user feature key for recon is updated
#endif

    void InitEncodeMmc(CodechalHwInterfaceNext *hwInterface);

    bool m_mmcEnabledForEncode = false;                  //!< Indicate if mmc is enabled for encode
    
    std::shared_ptr<mhw::mi::Itf> m_miItf;               //!< Point to MI interface

MEDIA_CLASS_DEFINE_END(EncodeMemComp)
};

#endif //__MEDIA_ENCODE_MEM_COMPRESSION_H__
