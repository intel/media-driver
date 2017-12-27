/*
* Copyright (c) 2011, Intel Corporation
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
//! \file      BRCIF.h  
//! \brief     This modules implements the interface for BRC dll  
//!
#ifndef _BRCIF_H__
#define _BRCIF_H__

//!
//! \enum     ErrorCode
//! \brief    Error code
//!
enum ErrorCode {
    SUCCEED = 0,
    NO_CURBE,
    NO_HISTORY,
    NO_CONSTANT,
    NO_DISTORTION,
    NO_PAK,
    NO_ENC,
    NO_IMGSTATE,
    NO_MBQP,
    UNKNOWN_PROCESSTYPE,
    UNKNOWN_BUFFERTYPE,
    UNKNOWN_ERROR
};

//!
//! \enum     ProcessType
//! \brief    Process type
//!
enum ProcessType {
    UnknownProc = 0,
    BRCInit,
    BRCReset,
    BRCUpdate
};

//!
//! \enum     BufferType
//! \brief    Buffer type
//!
enum BufferType {
    UnknownBuff = 0,
    CURBE,
    HISTORY,
    CONSTANT,
    DISTORTION,
    PAK,
    ENC,
    IMGSTATE,
    MBQP,
    STATISTICS
};

//!
//! \enum     HEVCBufferType
//! \brief    HEVC buffer type
//!
enum HEVCBufferType {
    HEVC_UnknownBuff = 0,
    HEVC_CURBE,
    HEVC_HISTORY,
    HEVC_CONSTANT,
    HEVC_DISTORTION,
    HEVC_PAK,
    HEVC_ENC,
    HEVC_IMGSTATEI,
    HEVC_IMGSTATEO,
    HEVC_STATISTICS,
    HEVC_HMEINFO,
    HEVC_MBQP,
    HEVC_INITCURBE,
    HEVC_INTRADIST,
    HEVC_EXTD
};

extern "C"    void * MC_CreateInstance(ProcessType processor);

extern "C"    void MC_ReleaseInstance(void *obj);

extern "C"    int MC_Process(void *obj);

extern "C"    int MC_SetBuff(unsigned char *buf, BufferType type, void *obj);

#endif
