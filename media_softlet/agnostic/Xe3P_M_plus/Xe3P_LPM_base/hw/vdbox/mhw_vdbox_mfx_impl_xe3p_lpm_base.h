/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     mhw_vdbox_mfx_impl_xe3p_lpm_base.h
//! \brief    MHW VDBOX MFX interface common base for XE3P_LPM_Base
//! \details
//!

#ifndef __MHW_VDBOX_MFX_IMPL_XE3P_LPM_BASE_H__
#define __MHW_VDBOX_MFX_IMPL_XE3P_LPM_BASE_H__

#include "mhw_vdbox_mfx_impl.h"

namespace mhw
{
namespace vdbox
{
namespace mfx
{
namespace xe3p_lpm_base
{
#define mpeg2WeightScaleSize 16

template <typename cmd_t>
class BaseImpl : public mfx::Impl<cmd_t>
{
protected:
    bool GetVlfRowstoreCacheMbaffSupport() const override
    {
        return true; // Xe3P_LPM platforms support VLF rowstore cache with MBAFF
    }
    using base_t = mfx::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : base_t(osItf, cpItf){};
MEDIA_CLASS_DEFINE_END(mhw__vdbox__mfx__xe3p_lpm_base__BaseImpl)
};
}  // namespace xe3p_lpm_base
}  // namespace mfx
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_MFX_IMPL_XE3P_LPM_BASE_H__
