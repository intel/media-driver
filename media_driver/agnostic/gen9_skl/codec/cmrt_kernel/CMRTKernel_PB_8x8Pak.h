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
//! \file     CMRTKernel_PB_8x8Pak.h
//! \brief    HEVC FEI PB 8x8 PAK class for GEN9 SKL.
//!

#ifndef _CMRTKERNEL_PB_8x8PAK_
#define _CMRTKERNEL_PB_8x8PAK_

#include "CMRTKernelBase.h"

class CMRTKernelPB8x8Pak:public CMRTKernelBase
{
public:
    CMRTKernelPB8x8Pak();
    ~CMRTKernelPB8x8Pak();
    CM_RETURN_CODE SetupCurbe(void *curbe);
    CM_RETURN_CODE CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue);//(EventList[i]);
};

class CMRTKernelPB8x8PakUMD:public CMRTKernelPB8x8Pak
{
public:
    CMRTKernelPB8x8PakUMD(): CMRTKernelPB8x8Pak(){};
    CM_RETURN_CODE AllocateSurfaces(void *params);
};

#endif

