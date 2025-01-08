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
//! \file      codec_hw_xe3_lpm_base_next.h
//! \brief     This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across CODECHAL components. 
//!

#ifndef __CODEC_HW__XE3_LPM_BASE_NEXT_H__
#define __CODEC_HW__XE3_LPM_BASE_NEXT_H__

#include "codec_hw_next.h"
#include "mhw_vdbox_vvcp_itf.h"

//!  Codec hw xe3_lpm_base next interface
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for next M15 platforms
*/
class CodechalHwInterfaceXe3_Lpm_Base_Next :public CodechalHwInterfaceNext
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceXe3_Lpm_Base_Next(
        PMOS_INTERFACE     osInterface,
        CODECHAL_FUNCTION  codecFunction,
        MhwInterfacesNext  *mhwInterfacesNext,
        bool               disableScalability = false);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceXe3_Lpm_Base_Next() {}


MEDIA_CLASS_DEFINE_END(CodechalHwInterfaceXe3_Lpm_Base_Next)
};

#endif  // __CODEC_HW__XE3_LPM_BASE_NEXT_H__
