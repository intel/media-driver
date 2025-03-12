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
//! \file     mhw_vdbox_hcp_impl_xe3_lpm_base.h
//! \brief    MHW VDBOX HCP interface common base for all XE3_LPM platforms
//! \details
//!

#ifndef __MHW_VDBOX_HCP_IMPL_XE3_LPM_BASE_H__
#define __MHW_VDBOX_HCP_IMPL_XE3_LPM_BASE_H__

#include "mhw_vdbox_hcp_impl.h"

namespace mhw
{
namespace vdbox
{
namespace hcp
{
namespace xe3_lpm_base
{
template <typename cmd_t>
class BaseImpl : public hcp::Impl<cmd_t>
{
public:
    uint32_t GetHcpVp9PicStateCommandSize()
    {
        return cmd_t::HCP_VP9_PIC_STATE_CMD::byteSize;
    }

    uint32_t GetHcpVp9SegmentStateCommandSize()
    {
        return cmd_t::HCP_VP9_SEGMENT_STATE_CMD::byteSize;
    }

protected:
    using base_t = hcp::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(HCP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(HCP_PIC_STATE);

#define DO_FIELDS() \
    DO_FIELD(DW36, VdaqmEnable, params.vdaqmEnable)

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__hcp__xe3_lpm_base__BaseImpl)
};
}  // namespace xe3_lpm_base
}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HCP_IMPL_XE3_LPM_BASE_H__
