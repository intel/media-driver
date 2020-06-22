/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_wp_g12.h
//! \brief    Gen12 class for weighted prediction.
//!

#ifndef __CODECHAL_ENCODE_WP_G12_H__
#define __CODECHAL_ENCODE_WP_G12_H__

#include "codechal_encode_wp.h"

class CodechalEncodeWPG12 : public CodechalEncodeWP
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeWPG12(CodechalEncoderState* encoder);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeWPG12() {};

    //!
    //! \brief    Initialize weighted prediction kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelState();
};

#endif  // __CODECHAL_ENCODE_WP_G12_H__
