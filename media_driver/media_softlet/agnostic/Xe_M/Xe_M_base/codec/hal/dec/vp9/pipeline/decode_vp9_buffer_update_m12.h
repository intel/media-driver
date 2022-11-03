/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_vp9_buffer_update_m12.h
//! \brief    Defines the interface for vp9 buffer update
//! \details  Defines the interface to handle the vp9 buffer update for
//!           segment id buffer and probability buffer.
//!

#ifndef __DECODE_VP9_BUFFER_UPDATE_M12_H__
#define __DECODE_VP9_BUFFER_UPDATE_M12_H__

#include "decode_vp9_buffer_update.h" 
#include "codechal_hw.h"

namespace decode {

class DecodeVp9BufferUpdateM12 : public DecodeVp9BufferUpdate
{
public:
    //!
    //! \brief  Decode input bitstream constructor
    //!
    DecodeVp9BufferUpdateM12(Vp9Pipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface);

    //!
    //! \brief  Decode input bitstream destructor
    //!
    virtual ~DecodeVp9BufferUpdateM12(){};

    //!
    //! \brief  Initialize the bitstream context
    //!
    //! \param  [in] settings
    //!         Reference to the Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(CodechalSetting& settings) override;

    CodechalHwInterface *m_hwInterface = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodeVp9BufferUpdateM12)
};

}

#endif
