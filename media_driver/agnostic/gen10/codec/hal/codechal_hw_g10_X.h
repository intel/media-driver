/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file      codechal_hw_g10_X.h 
//! \brief         This modules implements HW interface layer to be used on Gen10 platforms on all operating systems/DDIs, across CODECHAL components. 
//!
#ifndef __CODECHAL_HW_G10_X_H__
#define __CODECHAL_HW_G10_X_H__

#include "codechal_hw.h"

//!  Codechal hw interface Gen10
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen10 platforms
*/
class CodechalHwInterfaceG10 : public CodechalHwInterface
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG10[CODECHAL_NUM_MEDIA_STATES];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG10(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces *mhwInterfaces);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG10() {}
};

#endif // __CODECHAL_HW_G10_X_H__
