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
//! \file     mhw_vdbox_avp_impl_xe_lpm_plus.h
//! \brief    MHW VDBOX AVP interface common base for Xe_XPM_plus
//! \details
//!

#ifndef __MHW_VDBOX_AVP_IMPL_XE_LPM_PLUS_H__
#define __MHW_VDBOX_AVP_IMPL_XE_LPM_PLUS_H__

#include "mhw_vdbox_avp_impl_xe_lpm_plus_base.h"
#include "mhw_vdbox_avp_hwcmd_xe_lpm_plus.h"

namespace mhw
{
namespace vdbox
{
namespace avp
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

    _MHW_SETCMD_OVERRIDE_DECL(AVP_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_SURFACE_STATE);

        //Add compression setting for each ref
#define DO_FIELDS()                                                             \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1IntraFrame, MmcEnabled(params.mmcState[intraFrame]) ? 1 : 0);   \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1LastFrame, MmcEnabled(params.mmcState[lastFrame]) ? 1 : 0);    \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1Last2Frame, MmcEnabled(params.mmcState[last2Frame]) ? 1 : 0);   \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1Last3Frame, MmcEnabled(params.mmcState[last3Frame]) ? 1 : 0);   \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1GoldenFrame, MmcEnabled(params.mmcState[goldenFrame]) ? 1 : 0);  \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1BwdrefFrame, MmcEnabled(params.mmcState[bwdRefFrame]) ? 1 : 0);  \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1Altref2Frame, MmcEnabled(params.mmcState[altRef2Frame]) ? 1 : 0); \
    DO_FIELD(DW4, MemoryCompressionEnableForAv1AltrefFrame, MmcEnabled(params.mmcState[altRefFrame]) ? 1 : 0);  \
    DO_FIELD(DW4, CompressionTypeForIntraFrame, MmcRcEnabled(params.mmcState[intraFrame]) ? 1 : 0);            \
    DO_FIELD(DW4, CompressionTypeForLastFrame, MmcRcEnabled(params.mmcState[lastFrame]) ? 1 : 0);             \
    DO_FIELD(DW4, CompressionTypeForLast2Frame, MmcRcEnabled(params.mmcState[last2Frame]) ? 1 : 0);            \
    DO_FIELD(DW4, CompressionTypeForLast3Frame, MmcRcEnabled(params.mmcState[last3Frame]) ? 1 : 0);            \
    DO_FIELD(DW4, CompressionTypeForGoldenFrame, MmcRcEnabled(params.mmcState[goldenFrame]) ? 1 : 0);           \
    DO_FIELD(DW4, CompressionTypeForBwdrefFrame, MmcRcEnabled(params.mmcState[bwdRefFrame]) ? 1 : 0);           \
    DO_FIELD(DW4, CompressionTypeForAltref2Frame, MmcRcEnabled(params.mmcState[altRef2Frame]) ? 1 : 0);          \
    DO_FIELD(DW4, CompressionTypeForAltrefFrame, MmcRcEnabled(params.mmcState[altRefFrame]) ? 1 : 0)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIC_STATE);

#define DO_FIELDS()                                                                             \
    DO_FIELD(DW75, SbMaxBitsizeallowed, params.sbMaxBitSizeAllowed);                            \
    DO_FIELD(DW75, SbmaxbitstatusenSbmaxsizereportmask, params.sbMaxSizeReportMask ? 1 : 0);

#include "mhw_hwcmd_process_cmdfields.h"
    }

MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__xe_lpm_plus_base__v0__Impl)
};
}  // namespace v0
}  // namespace xe_lpm_plus_base
}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_IMPL_XE_LPM_PLUS_H__
