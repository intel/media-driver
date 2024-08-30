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
//! \file     mhw_vdbox_avp_impl_xe2_lpm.h
//! \brief    MHW VDBOX AVP interface common base for Xe2_LPM
//! \details
//!

#ifndef __MHW_VDBOX_AVP_IMPL_XE2_LPM_H__
#define __MHW_VDBOX_AVP_IMPL_XE2_LPM_H__

#include "mhw_vdbox_avp_impl_xe2_lpm_base.h"
#include "mhw_vdbox_avp_hwcmd_xe2_lpm.h"

#ifdef IGFX_AVP_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_avp_impl_xe2_lpm_ext.h"
#endif

namespace mhw
{
namespace vdbox
{
namespace avp
{
namespace xe2_lpm_base
{
namespace xe2_lpm
{
class Impl : public BaseImpl<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIC_STATE);

#define DO_FIELDS() \
    DO_FIELD(DW64, VDAQMenable, params.VdaqmEnable);

#ifdef _MEDIA_RESERVED
    #define DO_FIELDS_EXT() \
        __MHW_VDBOX_AVP_WRAPPER_EXT(AVP_PIC_STATE_IMPL_XE2_LPM)              
#endif  
    #include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_BUF_ADDR_STATE);

#ifdef _MEDIA_RESERVED
        __MHW_VDBOX_AVP_WRAPPER_EXT(AVP_PIPE_BUF_ADDR_STATE_IMPL_XE2_LPM)
#endif
        return MOS_STATUS_SUCCESS; 
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__xe2_lpm_base__xe2_lpm__Impl)
};
}  // namespace xe2_lpm
}  // namespace xe2_lpm_base
}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_IMPL_XE2_LPM_H__
