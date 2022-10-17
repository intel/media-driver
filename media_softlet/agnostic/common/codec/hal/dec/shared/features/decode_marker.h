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
//! \file     decode_marker.h
//! \brief    Defines the common interface for decode marker
//! \details  The decode marker interface implements the decode marker feature.
//!
#ifndef __DECODE_MARKER_H__
#define __DECODE_MARKER_H__

#include "codec_def_decode.h"
#include "decode_allocator.h"
#include "media_feature.h"
#include "codec_hw_next.h"

namespace decode {

class DecodeMarker: public MediaFeature
{
public:
    //!
    //! \brief  Decode marker constructor 
    //! \param  [in] allocator
    //!         Reference to decode allocator
    //!
    DecodeMarker(DecodeAllocator& allocator);

    //!
    //! \brief  Decode marker destructor
    //!
    virtual ~DecodeMarker();

    //!
    //! \brief  Update decode predication
    //! \param  [in] params
    //!         Reference to CodechalDecodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    bool            m_setMarkerEnabled = false;   //!< Indicates whether or not SetMarker is enabled
    PMOS_BUFFER     m_markerBuffer     = nullptr; //!< Resource for SetMarker
    uint32_t        m_setMarkerNumTs   = 0;       //!< Number Timestamp for SetMarker

protected:
    DecodeAllocator* m_allocator;                 //!< Decode allocator

MEDIA_CLASS_DEFINE_END(decode__DecodeMarker)
};

}//decode

#endif // !__DECODE_MARKER_H__
