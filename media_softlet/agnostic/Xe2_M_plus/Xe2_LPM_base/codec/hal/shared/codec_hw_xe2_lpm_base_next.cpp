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
//! \file     codec_hw_xe2_lpm_plus_next.cpp
//! \brief    This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across CODECHAL components.
//!
#include "codec_hw_xe2_lpm_base_next.h"

CodechalHwInterfaceXe2_Lpm_Base_Next::CodechalHwInterfaceXe2_Lpm_Base_Next(
    PMOS_INTERFACE     osInterface,
    CODECHAL_FUNCTION  codecFunction,
    MhwInterfacesNext  *mhwInterfacesNext,
    bool               disableScalability) :    
    CodechalHwInterfaceNext(osInterface, codecFunction, mhwInterfacesNext, disableScalability)
{
}
