/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_fei_avc_g9.h
//! \brief    This file defines the C++ class/interface for Gen9 SKL platform's
//!           AVC FEI encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_FEI_AVC_G9_SKL_H__
#define __CODECHAL_FEI_AVC_G9_SKL_H__

#include "codechal_fei_avc_g9.h"

class CodechalEncodeAvcEncFeiG9Skl : public CodechalEncodeAvcEncFeiG9
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncFeiG9Skl(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEncFeiG9(hwInterface, debugInterface, standardInfo){};

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAvcEncFeiG9Skl() {};

    //!
    //! \brief    Update the slice count according to the DymanicSliceShutdown policy
    //!
    void UpdateSSDSliceCount();
};
#endif

