/*
# Copyright (c) 2024, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file     mhw_vdbox_vvcp_impl_xe2_lpm_base.h
//! \brief    MHW VDBOX VVCP interface common base for Xe2_LPM+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_VVCP_IMPL_XE2_LPM_BASE_H__
#define __MHW_VDBOX_VVCP_IMPL_XE2_LPM_BASE_H__

#include "mhw_vdbox_vvcp_impl.h"

namespace mhw
{
namespace vdbox
{
namespace vvcp
{
namespace xe2_lpm_base
{

template <typename cmd_t>
class BaseImpl : public vvcp::Impl<cmd_t>
{
protected:
    using base_t = vvcp::Impl<cmd_t>;
    BaseImpl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};

public:
    uint32_t GetMocsValue(MOS_HW_RESOURCE_DEF hwResType) override
    {
        return this->m_cacheabilitySettings[hwResType].Gen12_7.Index;
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vvcp__xe2_lpm_base__BaseImpl)
};

}  // namespace xe2_lpm_base
}  // namespace vvcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VVCP_IMPL_XE2_LPM_BASE_H__
