/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file      codechal_hw_g9_kbl.h 
//! \brief         This modules implements HW interface layer to be used on kbl on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_G9_KBL_H__
#define __CODECHAL_HW_G9_KBL_H__

#include "codechal_hw_g9_X.h"

//!  Codechal hw interface Gen9 Kbl
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen9 Kbl platforms
*/
class CodechalHwInterfaceG9Kbl : public CodechalHwInterfaceG9
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG9Kbl[CODECHAL_NUM_MEDIA_STATES];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG9Kbl(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces *mhwInterfaces)
        : CodechalHwInterfaceG9(osInterface, codecFunction, mhwInterfaces)
    {
        CODECHAL_HW_FUNCTION_ENTER;

        m_isVdencSuperSliceEnabled = true;
        m_ssEuTable = m_defaultSsEuLutG9Kbl;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG9Kbl() {}
};

#endif // __CODECHAL_HW_G9_KBL_H__
