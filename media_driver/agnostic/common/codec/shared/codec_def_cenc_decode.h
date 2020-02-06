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
//! \file     codec_def_cenc_decode.h
//! \brief    Defines common types and macros shared by CENC decode
//!
#ifndef __CODEC_DEF_CENC_DECODE_H__
#define __CODEC_DEF_CENC_DECODE_H__

//! \brief  CencDecode share buffer
struct CencDecodeShareBuf
{
    uint32_t                       bufIdx = 0;
    uint32_t                       trackerId = 0;
    bool                           checkStatusRequired = 0;
    MOS_RESOURCE                   *resTracker = nullptr;
    MemoryBlock                    *secondLvlBbBlock = nullptr;
    MOS_RESOURCE                   *resStatus = nullptr;
};

#endif  // __CODEC_DEF_CENC_DECODE_H__
