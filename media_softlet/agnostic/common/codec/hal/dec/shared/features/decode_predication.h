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
//! \file     decode_predication.h
//! \brief    Defines the common interface for decode predication
//! \details  The decode predication interface implements the decode predication feature.
//!
#ifndef __DECODE_PREDICATION_H__
#define __DECODE_PREDICATION_H__

#include "codec_def_decode.h"
#include "decode_allocator.h"
#include "media_feature.h"
#include "codec_hw_next.h"

namespace decode {

class DecodePredication: public MediaFeature
{
public:
    //!
    //! \brief  Decode predication constructor 
    //! \param  [in] allocator
    //!         Reference to decode allocator
    //!
    DecodePredication(DecodeAllocator& allocator);
    //!
    //! \brief  Decode predication destructor
    //!
    virtual ~DecodePredication();
    //!
    //! \brief  Update decode predication
    //! \param  [in] params
    //!         Reference to CodechalDecodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    PMOS_BUFFER      m_predicationBuffer = nullptr;      //!< Internal resource for Predication
    PMOS_BUFFER      m_resPredication = nullptr;         //!< External resource for predication
    uint64_t         m_predicationResOffset = 0;         //!< Offset for Predication resource
    bool             m_predicationEnabled = false;       //!< Indicates whether or not Predication is enabled
    bool             m_predicationNotEqualZero = false;  //!< Predication mode
protected:
    DecodeAllocator* m_allocator;                        //!< Decode allocator

MEDIA_CLASS_DEFINE_END(decode__DecodePredication)
};

}//decode

#endif // !__DECODE_PREDICATION_H__