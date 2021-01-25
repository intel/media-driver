/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     mhw_vdbox_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_CMDPAR_H__
#define __MHW_VDBOX_CMDPAR_H__

#include "mhw_cmdpar.h"

namespace mhw
{
namespace vdbox
{
static constexpr uint32_t RAW_UV_PLANE_ALIGNMENT = 4;

struct RowStoreCache
{
    bool     supported = false;
    bool     enabled   = false;
    uint32_t dwAddress = 0;
};
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_CMDPAR_H__
