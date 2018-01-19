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
//! \file     CMRTKernel_I_16x16Sad.h
//! \brief    HEVC FEI I 16x16 SAD class for GEN9 SKL.
//!

#ifndef _CMRTKERNEL_I_16x16SAD_
#define _CMRTKERNEL_I_16x16SAD_

#include "CMRTKernelBase.h"

class CMRTKernelI16x16Sad:public CMRTKernelBase
{
public:
    CMRTKernelI16x16Sad();
    ~CMRTKernelI16x16Sad();
    CM_RETURN_CODE SetupCurbe(void *curbe);
    CM_RETURN_CODE CreateAndDispatchKernel(CmEvent *&cmEvent, bool destroyEvent, bool isEnqueue);//(EventList[i]);
};

class CMRTKernelI16x16SadUMD:public CMRTKernelI16x16Sad
{

public:
    CMRTKernelI16x16SadUMD() : CMRTKernelI16x16Sad(){};

    CM_RETURN_CODE AllocateSurfaces(void *params);
};

#endif

