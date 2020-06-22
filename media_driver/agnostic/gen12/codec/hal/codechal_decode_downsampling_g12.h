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
//! \file     codechal_decode_downsampling_g12.h
//! \brief    Implements the decode interface extension for downsampling on Gen12.
//!
#ifndef __CODECHAL_DECODER_DOWNSAMPLING_G12_H__
#define __CODECHAL_DECODER_DOWNSAMPLING_G12_H__

#include "codechal_decode_downsampling.h"

class FieldScalingInterfaceG12 : public FieldScalingInterface
{
public:
    //!
    //! \brief    Constructor
    //!
    FieldScalingInterfaceG12(CodechalHwInterface *hwInterface);

    //!
    //! \brief    Destructor
    //!
    ~FieldScalingInterfaceG12() {};

    MOS_STATUS SetupMediaVfe(
        PMOS_COMMAND_BUFFER  cmdBuffer,
        MHW_KERNEL_STATE     *kernelState);

    MOS_STATUS InitMmcState();
};

#endif // __CODECHAL_DECODER_DOWNSAMPLING_G12_H__