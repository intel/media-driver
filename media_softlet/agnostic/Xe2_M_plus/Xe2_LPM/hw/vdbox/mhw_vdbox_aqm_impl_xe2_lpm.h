/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     mhw_vdbox_aqm_impl_xe2_lpm.h
//! \brief    MHW VDBOX AQM interface common base for Xe2_Lpm
//! \details
//!

#ifndef __MHW_VDBOX_AQM_IMPL_XE2_LPM_H__
#define __MHW_VDBOX_AQM_IMPL_XE2_LPM_H__

#include "mhw_vdbox_aqm_impl.h"
#include "mhw_vdbox_aqm_hwcmd_xe2_lpm.h"

namespace mhw
{
namespace vdbox
{
namespace aqm
{
namespace xe2_lpm
{

static constexpr uint32_t MEMORY_ADDRESS_ATTRIBUTES_MOCS_CLEAN_MASK = 0xFFFFFF81;

class Impl : public aqm::Impl<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = aqm::Impl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};

#ifdef _MEDIA_RESERVED
#include "mhw_vdbox_aqm_impl_xe2_lpm_ext.h"
#endif

MEDIA_CLASS_DEFINE_END(mhw__vdbox__aqm__xe2_lpm__Impl)
};
}  // namespace xe2_lpm
}  // namespace aqm
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AQM_IMPL_XE2_LPM_H__
