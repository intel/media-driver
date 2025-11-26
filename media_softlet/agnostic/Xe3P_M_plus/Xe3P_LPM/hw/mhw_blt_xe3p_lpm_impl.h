/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     mhw_blt_xe3p_lpm_impl.h
//! \brief    MHW blt interface for Xe3P_LPM
//! \details
//!

#ifndef __MHW_BLT_XE3P_LPM_IMPL_H__
#define __MHW_BLT_XE3P_LPM_IMPL_H__

#include "mhw_blt_xe3p_lpm_base_impl.h"
#include "mhw_blt_hwcmd_xe3p_lpm.h"

namespace mhw
{
namespace blt
{
namespace xe3p_lpm
{
class Impl : public xe3p_lpm_base::BaseImpl<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = xe3p_lpm_base::BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};
MEDIA_CLASS_DEFINE_END(mhw__blt__xe3p_lpm__Impl)
};
}  // namespace xe3p_lpm
}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_XE3P_LPM_IMPL_H__
