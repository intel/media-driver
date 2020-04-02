/*
* Copyright (c) 2019, Intel Corporation
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
//! \file      mos_os_specific.h
//! \brief     Common interface and structure used in MOS LINUX OS
//!

#ifndef __MOS_OS_SPECIFIC_NEXT_H__
#define __MOS_OS_SPECIFIC_NEXT_H__
#include "mos_os.h"
#include "media_skuwa_specific.h"
#include "GmmLib.h"
#include "mos_resource_defs.h"
#include "mos_defs.h"
#include "i915_drm.h"
#include "mos_bufmgr.h"
#include "xf86drm.h"

#include <vector>

class GpuContextSpecific;

class MosOsSpecificNext
{
public:

    static GpuContextSpecific * Linux_GetGpuContext(PMOS_INTERFACE pOsInterface, uint32_t gpuContextHandle);
    static GMM_RESOURCE_FORMAT Mos_Specific_ConvertMosFmtToGmmFmt(MOS_FORMAT format);

};

#endif // __MOS_OS_SPECIFIC_NEXT_H__
