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
//! \file      ddi_decode_trace_specific.h
//! \brief     Declaration for decode event trace dump
//!

#include "codec_def_decode_avc.h"
#include "codec_def_decode_hevc.h"
#include "codec_def_decode_av1.h"

namespace decode
{

#if MOS_EVENT_TRACE_DUMP_SUPPORTED

#define MACROBLOCK_HEIGHT   16
#define MACROBLOCK_WIDTH    16

typedef struct _DECODE_EVENTDATA_INFO_PICTUREVA
{
    uint32_t CodecFormat;
    uint32_t FrameType;
    uint32_t PicStruct;
    uint32_t Width;
    uint32_t Height;
    uint32_t Bitdepth;
    uint32_t ChromaFormat;
    bool     EnabledSCC;
    bool     EnabledSegment;
    bool     EnabledFilmGrain;
} DECODE_EVENTDATA_INFO_PICTUREVA;

typedef struct _DECODE_EVENTDATA_VA_DISPLAYINFO
{
    uint32_t uiDisplayWidth;
    uint32_t uiDisplayHeight;
} DECODE_EVENTDATA_VA_DISPLAYINFO;

typedef struct _DECODE_EVENTDATA_VA_CREATEBUFFER
{
    VABufferType type;
    uint32_t size;
    uint32_t numElements;
    VABufferID *bufId;
} DECODE_EVENTDATA_VA_CREATEBUFFER;

typedef struct _DECODE_EVENTDATA_VA_BEGINPICTURE_START
{
    uint32_t FrameIndex;
} DECODE_EVENTDATA_VA_BEGINPICTURE_START;

typedef struct _DECODE_EVENTDATA_VA_BEGINPICTURE
{
    uint32_t FrameIndex;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_BEGINPICTURE;

typedef struct _DECODE_EVENTDATA_VA_ENDPICTURE_START
{
    uint32_t FrameIndex;
} DECODE_EVENTDATA_VA_ENDPICTURE_START;

typedef struct _DECODE_VA_EVENTDATA_ENDPICTURE
{
    uint32_t FrameIndex;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_ENDPICTURE;


typedef struct _DECODE_EVENTDATA_VA_RENDERPICTURE_START
{
    VABufferID *buffers;
} DECODE_EVENTDATA_VA_RENDERPICTURE_START;

typedef struct _DECODE_EVENTDATA_VA_RENDERPICTURE
{
    VABufferID *buffers;
    uint32_t hRes;
    uint32_t numBuffers;
} DECODE_EVENTDATA_VA_RENDERPICTURE;

typedef struct _DECODE_EVENTDATA_VA_CREATECONTEXT_START
{
    VABufferID configId;
} DECODE_EVENTDATA_VA_CREATECONTEXT_START;

typedef struct _DECODE_EVENTDATA_VA_CREATECONTEXT
{
    VABufferID configId;
    uint32_t hRes;
} DECODE_EVENTDATA_VA_CREATECONTEXT;

typedef struct _DECODE_EVENTDATA_VA_DESTROYCONTEXT_START
{
    VABufferID context;
} DECODE_EVENTDATA_VA_DESTROYCONTEXT_START;

typedef struct _DECODE_EVENTDATA_VA_DESTROYCONTEXT
{
    VABufferID context;
} DECODE_EVENTDATA_VA_DESTROYCONTEXT;

typedef struct _DECODE_EVENTDATA_VA_GETDECCTX
{
    uint32_t bufferID;
} DECODE_EVENTDATA_VA_GETDECCTX;

typedef struct _DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS
{
    uint32_t bufNums;
} DECODE_EVENTDATA_VA_FREEBUFFERHEAPELEMENTS;

typedef struct _DECODE_EVENTDATA_VA_FEATURE_REPORTMODE
{
    uint32_t wMode;
    uint32_t ValueID;
} DECODE_EVENTDATA_VA_FEATURE_REPORTMODE;

#endif
}
