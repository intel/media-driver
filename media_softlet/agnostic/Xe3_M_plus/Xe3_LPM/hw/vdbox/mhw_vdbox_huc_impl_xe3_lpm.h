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
//! \file     mhw_vdbox_huc_impl_xe3_lpm.h
//! \brief    MHW VDBOX HUC interface common base for Xe3_LPM
//! \details
//!

#ifndef __MHW_VDBOX_HUC_IMPL_XE3_LPM_H__
#define __MHW_VDBOX_HUC_IMPL_XE3_LPM_H__

#include "mhw_vdbox_huc_impl.h"
#include "mhw_vdbox_huc_hwcmd_xe3_lpm.h"

namespace mhw
{
namespace vdbox
{
namespace huc
{
namespace xe3_lpm_base
{
namespace xe3_lpm
{
class Impl : public huc::Impl<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = huc::Impl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__huc__xe3_lpm_base__xe3_lpm__Impl)
};
}  // namespace xe3_lpm
}  // namespace xe3_lpm_base
}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_IMPL_XE3_LPM_H__
