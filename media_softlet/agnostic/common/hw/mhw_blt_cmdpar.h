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
//! \file     mhw_blt_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_BLT_CMDPAR_H__
#define __MHW_BLT_CMDPAR_H__

#include "mhw_blt.h"

namespace mhw
{
namespace blt
{
struct _MHW_PAR_T(XY_FAST_COPY_BLT)
{
    uint32_t      dwColorDepth;
    uint32_t      dwSrcPitch;
    uint32_t      dwDstPitch;
    uint32_t      dwSrcTop;
    uint32_t      dwSrcLeft;
    uint32_t      dwDstTop;
    uint32_t      dwDstBottom;
    uint32_t      dwDstLeft;
    uint32_t      dwDstRight;
    uint32_t      dwSrcOffset;
    uint32_t      dwDstOffset;
    PMOS_RESOURCE pSrcOsResource;
    PMOS_RESOURCE pDstOsResource;
};

struct _MHW_PAR_T(XY_BLOCK_COPY_BLT)
{
    uint32_t      dwColorDepth;
    uint32_t      dwSrcPitch;
    uint32_t      dwDstPitch;
    uint32_t      dwSrcTop;
    uint32_t      dwSrcLeft;
    uint32_t      dwDstTop;
    uint32_t      dwDstBottom;
    uint32_t      dwDstLeft;
    uint32_t      dwDstRight;
    uint32_t      dwSrcOffset;
    uint32_t      dwDstOffset;
    uint32_t      dwPlaneIndex;
    uint32_t      dwPlaneNum;
    PMOS_RESOURCE pSrcOsResource;
    PMOS_RESOURCE pDstOsResource;
};

}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_CMDPAR_H__
