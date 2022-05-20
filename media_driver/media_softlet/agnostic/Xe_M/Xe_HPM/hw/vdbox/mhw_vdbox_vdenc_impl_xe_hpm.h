/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_impl_xe_hpm.h
//! \brief    MHW VDBOX VDENC interface common base for xe_hpm platforms
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE_HPM_H__
#define __MHW_VDBOX_VDENC_IMPL_XE_HPM_H__

#include "mhw_vdbox_vdenc_impl.h"
#include "mhw_vdbox_vdenc_hwcmd_xe_hpm.h"

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe_hpm
{
class Impl : public vdenc::Impl<Cmd>
{
protected:
    using base_t = vdenc::Impl<Cmd>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe_hpm__Impl)
};
}  // namespace xe_hpm
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE_HPM_H__
