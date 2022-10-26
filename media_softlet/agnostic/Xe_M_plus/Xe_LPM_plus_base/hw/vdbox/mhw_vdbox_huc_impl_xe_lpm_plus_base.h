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
//! \file     mhw_vdbox_huc_impl_xe_lpm_plus_base.h
//! \brief    MHW VDBOX HUC interface common base for Xe_LPM_plus+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_HUC_IMPL_XE_LPM_PLUS_BASE_H__
#define __MHW_VDBOX_HUC_IMPL_XE_LPM_PLUS_BASE_H__

#include "mhw_vdbox_huc_impl.h"

namespace mhw
{
namespace vdbox
{
namespace huc
{
namespace xe_lpm_plus_base
{
template <typename cmd_t>
class BaseImpl: public huc::Impl<cmd_t>
{
protected:
    using base_t = huc::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};

MEDIA_CLASS_DEFINE_END(mhw__vdbox__huc__xe_lpm_plus_base__BaseImpl)
};
}  // namespace xe_lpm_plus_base
}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_IMPL_XE_LPM_PLUS_BASE_H__
