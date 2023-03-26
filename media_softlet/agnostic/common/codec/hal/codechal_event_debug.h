/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     codechal_event_debug.h
//! \brief    Defines the event dump interface shared by codec only.
//! \details  The debug interface dumps output from Media based on in input config file.
//!
#ifndef __CODEC_EVENT_DEBUG_H__
#define __CODEC_EVENT_DEBUG_H__

#include "media_debug_interface.h"

#define BITSTREAM_INFO_SIZE 16

typedef struct _DECODE_EVENTDATA_BITSTREAM
{
    uint32_t BitstreamSize;
    uint8_t  Data[BITSTREAM_INFO_SIZE];
} DECODE_EVENTDATA_BITSTREAM;

typedef struct _DECODE_EVENTDATA_FRAME
{
    uint32_t FrameIdx;
    uint32_t PicFlags;
    uint32_t PicEntry;
} DECODE_EVENTDATA_FRAME;

#endif /* __CODEC_EVENT_DEBUG_H__ */
