/*===================== begin_copyright_notice ==================================

# Copyright (c) 2022, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_hcp_impl_xe_lpm_plus.h
//! \brief    MHW VDBOX HCP interface common base for Xe_LPM_plus
//! \details
//!

#ifndef __MHW_VDBOX_HCP_IMPL_XE_LPM_PLUS_H__
#define __MHW_VDBOX_HCP_IMPL_XE_LPM_PLUS_H__

#include "mhw_vdbox_hcp_impl_xe_lpm_plus_base.h"
#include "mhw_vdbox_hcp_hwcmd_xe_lpm_plus.h"

namespace mhw
{
namespace vdbox
{
namespace hcp
{
namespace xe_lpm_plus_base
{
namespace v0
{
class Impl : public BaseImpl<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__hcp__xe_lpm_plus_base__v0__Impl)
};
}  // namespace v0
}  // namespace xe_lpm_plus_base
}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HCP_IMPL_XE_LPM_PLUS_H__
