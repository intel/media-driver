/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_impl_xe3p_lpm.h
//! \brief    MHW VDBOX VDENC interface common base for Xe3P_LPM platforms
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE3P_LPM_H__
#define __MHW_VDBOX_VDENC_IMPL_XE3P_LPM_H__

#include "mhw_vdbox_vdenc_hwcmd_xe3p_lpm.h"
#include "mhw_vdbox_vdenc_impl_xe3p_lpm_base.h"


namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe3p_lpm_base
{
namespace xe3p_lpm
{
class Impl : public BaseImpl<Cmd>
{
protected:
    using cmd_t = Cmd;
    using base_t = BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};

protected:
    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_MODE_SELECT);

#define DO_FIELDS() \
    DO_FIELD(DW1, ChromaPrefetchDisable, params.chromaPrefetchDisable);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD2)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD2);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_AVC_IMG_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_AVC_IMG_STATE);

#define DO_FIELDS_EXT() \
        __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_AVC_IMG_STATE_IMPL_XE_LPM_BASE_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe3p_lpm_base__xe3p_lpm__Impl)
    
};
}  // namespace xe3p_lpm
}  // namespace xe3p_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE3P_LPM_H__
