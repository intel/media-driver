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
//! \file     codechal_decode_sfc_jpeg_g12.h
//! \brief    Defines the SFC interface extension for JPEG decode in G12+ platform.
//! \details  Defines all types, macros, and functions required by CodecHal SFC for JPEG decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODE_SFC_JPEG_G12_H__
#define __CODECHAL_DECODE_SFC_JPEG_G12_H__

#include "mhw_sfc_g12_X.h"
#include "codechal_decode_sfc_jpeg.h"

class CodechalJpegSfcStateG12 : public CodechalJpegSfcState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalJpegSfcStateG12() { CODECHAL_HW_FUNCTION_ENTER; };
    //!
    //! \brief    Destructor
    //!
    ~CodechalJpegSfcStateG12() { CODECHAL_HW_FUNCTION_ENTER; };

    MOS_STATUS AddSfcCommands(
        PMOS_COMMAND_BUFFER                 cmdBuffer);

    MOS_STATUS SetSfcStateParams(
        PMHW_SFC_STATE_PARAMS               sfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS         outSurfaceParams);
};

#endif
