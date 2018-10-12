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
//! \file     codechal_encode_sw_scoreboard_g11.h
//! \brief    Gen11 class for SW socreboard init kernel.
//!

#ifndef __CODECHAL_ENCODE_SW_SCOREBOARD_G11_H__
#define __CODECHAL_ENCODE_SW_SCOREBOARD_G11_H__

#include "codechal_encode_sw_scoreboard.h"

class CodechalEncodeSwScoreboardG11 : public CodechalEncodeSwScoreboard
{
public:

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeSwScoreboardG11(CodechalEncoderState* encoder) :
        CodechalEncodeSwScoreboard(encoder) { }
    
    //!
    //! \brief    Initialize SW scoreboard init kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelState() override;
};

#endif  // __CODECHAL_ENCODE_SW_SCOREBOARD_G11_H__
