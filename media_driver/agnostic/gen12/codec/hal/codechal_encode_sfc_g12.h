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
//! \file     codechal_encode_sfc_g12.h
//! \brief    Defines the encode interface for CSC via SFC.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#ifndef __CODECHAL_ENCODE_SFC_G12_H__
#define __CODECHAL_ENCODE_SFC_G12_H__

#include "codechal_encode_sfc.h"

class CodecHalEncodeSfcG12 : public CodecHalEncodeSfc
{
public:
    CodecHalEncodeSfcG12() {};
    ~CodecHalEncodeSfcG12() {};

    MOS_STATUS AddSfcCommands(
        PMHW_SFC_INTERFACE              sfcInterface,
        PMOS_COMMAND_BUFFER             cmdBuffer);
};
#endif  // __CODECHAL_ENCODE_SFC_G12_H__
