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

#ifdef _MEDIA_RESERVED
    #define DO_FIELDS()                                                   \
    DO_FIELD(DW64, Reserved2048, params.avpPicStatePar0);                 \
    DO_FIELD(DW51, RhoDomainStreamoutEnableFlag, params.rhoDomainEnable); \
    DO_FIELD(DW75, RhoDomainQp, params.rhoDomainQP);                      
    #include "mhw_hwcmd_process_cmdfields.h"   
#else
    #define DO_FIELDS()       \

    #include "mhw_hwcmd_process_cmdfields.h"                      
#endif   

    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};
        resourceParams.dwLsbNum            = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType       = MOS_MFX_PIPE_BUF_ADDR;

        if (!Mos_ResourceIsNull(params.rhoDomainThresholdTableBuffer))
        {
            InitMocsParams(resourceParams, &cmd.RhoDomainThresholdsBufferAddressAttributes.DW0.Value, 1, 6);

            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.rhoDomainThresholdTableBuffer, &details));

            cmd.RhoDomainThresholdsBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStatePreDeblock);
            cmd.RhoDomainThresholdsBufferAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStatePreDeblock);
            cmd.RhoDomainThresholdsBufferAddressAttributes.DW0.TileMode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

            resourceParams.presResource    = params.rhoDomainThresholdTableBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.RhoDomainThresholdsBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(RhoDomainThresholdsBufferAddress);
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

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
