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
//! \file      codechal_hw_g8_bdw.h 
//! \brief         This modules implements HW interface layer to be used on bdw on all operating systems/DDIs, across CODECHAL components. 
//!
#ifndef __CODECHAL_HW_G8_BDW_H__
#define __CODECHAL_HW_G8_BDW_H__

#include "codechal_hw_g8_X.h"

//!  Codechal hw interface Gen8 Bdw
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen8 Bdw platforms
*/
class CodechalHwInterfaceG8Bdw : public CodechalHwInterfaceG8
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG8Bdw(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces)
        : CodechalHwInterfaceG8(osInterface, codecFunction, mhwInterfaces)
    {
        CODECHAL_HW_FUNCTION_ENTER;

        m_ssEuTable = m_defaultSsEuLutG8;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG8Bdw() {}
};

#endif // __CODECHAL_HW_G8_BDW_H__
