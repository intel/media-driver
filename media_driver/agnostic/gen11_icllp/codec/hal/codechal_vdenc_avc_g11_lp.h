/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_vdenc_avc_g11_lp.h
//! \brief    This file defines the base C++ class/interface for G11LP AVC VDENC 
//!           encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_VDENC_AVC_G11_LP_H__
#define __CODECHAL_VDENC_AVC_G11_LP_H__

#include "codechal_vdenc_avc_g11.h"
#include "codeckrnheader.h"

class CodechalVdencAvcStateG11LP : public CodechalVdencAvcStateG11
{
public:
    //!
    //! \brief    Destructor
    //!
    ~CodechalVdencAvcStateG11LP() {};

    //!
    //! \brief    Constructor
    //!
    CodechalVdencAvcStateG11LP(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalVdencAvcStateG11(hwInterface, debugInterface, standardInfo)
    {
        CODECHAL_ENCODE_FUNCTION_ENTER;
        m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
        AddIshSize(m_kuidCommon, m_kernelBase);
    }

};

#endif  // __CODECHAL_VDENC_AVC_G11_LP_H__
