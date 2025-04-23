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
//! \file     decode_vp8_mem_compression_xe_lpm_plus_base.h
//! \brief    Defines the common interface for VP8 decode mmc for Xe_LPM_plus+
//! \details  The VP8 decode media mmc is to handle decode mmc operations
//!

#ifndef __VP8_DECODE_MEM_COMPRESSION_XE_LPM_PLUS_BASE_H__
#define __VP8_DECODE_MEM_COMPRESSION_XE_LPM_PLUS_BASE_H__

#include "decode_vp8_mem_compression.h"
#include "decode_mem_compression_xe_lpm_plus_base.h"

#ifdef _MMC_SUPPORTED

namespace decode
{
    class Vp8DecodeMemCompXe_Lpm_Plus_Base : public DecodeMemCompXe_Lpm_Plus_Base, public Vp8DecodeMemComp
    {
        public:
            //!
            //! \brief    Construct
            //!
            Vp8DecodeMemCompXe_Lpm_Plus_Base(CodechalHwInterfaceNext *hwInterface);

            //!
            //! \brief    Copy constructor
            //!
            Vp8DecodeMemCompXe_Lpm_Plus_Base(const DecodeMemComp&) = delete;

            //!
            //! \brief    Copy assignment operator
            //!
            Vp8DecodeMemCompXe_Lpm_Plus_Base& operator=(const Vp8DecodeMemCompXe_Lpm_Plus_Base&) = delete;

            //!
            //! \brief    Destructor
            //!
            virtual ~Vp8DecodeMemCompXe_Lpm_Plus_Base() {};

        protected:
            PMOS_INTERFACE m_osInterface = nullptr;

        MEDIA_CLASS_DEFINE_END(decode__Vp8DecodeMemCompXe_Lpm_Plus_Base)
    };
}
#endif
#endif //__HEVC_DECODE_MEM_COMPRESSION_XE_LPM_PLUS_BASE_H__