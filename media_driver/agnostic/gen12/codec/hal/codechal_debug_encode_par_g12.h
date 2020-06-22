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
//! \file     codechal_debug_encode_par_g12.h
//! \brief    Defines the debug interface shared by all of CodecHal for encode
//!           PAR file dump.
//!

#ifndef __CODECHAL_DEBUG_ENCODE_PAR_G12_H__
#define __CODECHAL_DEBUG_ENCODE_PAR_G12_H__

#include "codechal.h"
#include "codechal_debug_encode_par.h"

#if USE_CODECHAL_DEBUG_TOOL

class CodechalDebugEncodeParG12 : public CodechalDebugEncodePar
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalDebugEncodeParG12(CodechalEncoderState *encoder) :
        CodechalDebugEncodePar(encoder) {};

    //!
    //! \brief    Destructor
    //!
    ~CodechalDebugEncodeParG12() {};

    virtual MOS_STATUS PopulateDsParam(
        void *cmd);

    virtual MOS_STATUS PopulateHmeParam(
        bool    is16xMeEnabled,
        bool    is32xMeEnabled,
        uint8_t meMethod,
        void    *cmd);

};

#endif // USE_CODECHAL_DEBUG_TOOL

#endif // __CODECHAL_DEBUG_ENCODE_PAR_G12_H__
