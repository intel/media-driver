/*===================== begin_copyright_notice ==================================

# Copyright (c) 2023-2026, Intel Corporation

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
//! \file     mhw_vdbox_huc_ppgtt_impl.h
//! \brief    MHW VDBOX HUC PPGTT interface common base for Xe3P_plus+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_HUC_PPGTT_IMPL_H__
#define __MHW_VDBOX_HUC_PPGTT_IMPL_H__

#include "mhw_vdbox_huc_impl.h"
#include "mhw_vdbox_huc_ppgtt_itf.h"

namespace mhw
{
namespace vdbox
{
namespace huc
{
template <typename cmd_t>
class ImplPPGTT: public ItfPPGTT, public huc::Impl<cmd_t>
{
    _HUC_PPGTT_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

protected:
    using base_t = ItfPPGTT;

    ImplPPGTT(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : huc::Impl<cmd_t>(osItf, cpItf) {};

    _MHW_SETCMD_OVERRIDE_DECL(HUC_IMEM_ADDR)
    {
        MHW_FUNCTION_ENTER;

        _MHW_SETCMD_CALLBASE(HUC_IMEM_ADDR);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum      = MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_HUC_DMEM;

        if (!Mos_ResourceIsNull(params.kernelBinBuffer))
        {
            resourceParams.presResource    = params.kernelBinBuffer;
            resourceParams.dwOffset        = params.kernelbinOffset;
            resourceParams.pdwCmd          = cmd.HucPpgttAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 1;
            resourceParams.bIsWritable     = false;
            resourceParams.dwSize          = params.kernelBinSize;

            InitMocsParams(resourceParams, &cmd.MemoryAddressAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

#define DO_FIELDS() \
        DO_FIELD(DW4, IntegrityEnabledBit, params.integrityEnable ? 1 : 0);

#include "mhw_hwcmd_process_cmdfields.h"
    }

MEDIA_CLASS_DEFINE_END(mhw__vdbox__huc__ImplPPGTT)
};
}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_PPGTT_IMPL_H__
