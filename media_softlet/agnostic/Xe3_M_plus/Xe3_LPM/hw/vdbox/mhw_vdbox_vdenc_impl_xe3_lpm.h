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
//! \file     mhw_vdbox_vdenc_impl_xe3_lpm.h
//! \brief    MHW VDBOX VDENC interface common base for Xe3_LPM platforms
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE3_LPM_H__
#define __MHW_VDBOX_VDENC_IMPL_XE3_LPM_H__

#include "mhw_vdbox_vdenc_hwcmd_xe3_lpm.h"
#include "mhw_vdbox_vdenc_impl_xe3_lpm_base.h"

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe3_lpm_base
{
namespace xe3_lpm
{
class Impl : public BaseImpl<Cmd>
{
protected:
    using cmd_t = Cmd;
    using base_t = BaseImpl<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};

    
    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD1)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD1);

#define DO_FIELDS()                                                    \
    DO_FIELD(DW9, VDENC_CMD1_DW9_BIT24, params.vdencCmd1Par95);        \
    DO_FIELD(DW32, VDENC_CMD1_DW32_BIT24, params.vdencCmd1Par93);      \
    DO_FIELD(DW31, VDENC_CMD1_DW31_BIT24, params.vdencCmd1Par94);      \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT0, params.vdencCmd1Par11[1]);    \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT8, params.vdencCmd1Par15[3]);    \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT16, params.vdencCmd1Par15[2]);   \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT24, params.vdencCmd1Par15[1]);   \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT0, params.vdencCmd1Par14[2]);    \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT8, params.vdencCmd1Par14[1]);    \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT16, params.vdencCmd1Par11[3]);   \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT24, params.vdencCmd1Par11[2]);   \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT0, params.vdencCmd1Par10[3]);    \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT8, params.vdencCmd1Par10[2]);    \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT16, params.vdencCmd1Par10[1]);   \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT24, params.vdencCmd1Par14[3]);   \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT0, params.vdencCmd1Par9[1]);     \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT8, params.vdencCmd1Par13[3]);    \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT16, params.vdencCmd1Par13[2]);   \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT24, params.vdencCmd1Par13[1]);   \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT0, params.vdencCmd1Par12[2]);    \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT8, params.vdencCmd1Par12[1]);    \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT16, params.vdencCmd1Par9[3]);    \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT24, params.vdencCmd1Par9[2]);    \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT0, params.vdencCmd1Par8[3]);     \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT8, params.vdencCmd1Par8[2]);     \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT16, params.vdencCmd1Par8[1]);    \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT24, params.vdencCmd1Par12[3]);   \
    DO_FIELD(DW39, VDENC_CMD1_DW39_BIT0, params.vdnecCmd1Par96[0]);    \
    DO_FIELD(DW39, VDENC_CMD1_DW39_BIT8, params.vdnecCmd1Par97[0]);    \
    DO_FIELD(DW39, VDENC_CMD1_DW39_BIT16, params.vdnecCmd1Par98[0]);   \
    DO_FIELD(DW39, VDENC_CMD1_DW39_BIT24, params.vdnecCmd1Par99[0]);   \
    DO_FIELD(DW40, VDENC_CMD1_DW40_BIT0, params.vdnecCmd1Par96[1]);    \
    DO_FIELD(DW40, VDENC_CMD1_DW40_BIT8, params.vdnecCmd1Par97[1]);    \
    DO_FIELD(DW40, VDENC_CMD1_DW40_BIT16, params.vdnecCmd1Par98[1]);   \
    DO_FIELD(DW40, VDENC_CMD1_DW40_BIT24, params.vdnecCmd1Par99[1]);   \
    DO_FIELD(DW41, VDENC_CMD1_DW41_BIT0, params.vdnecCmd1Par96[2]);    \
    DO_FIELD(DW41, VDENC_CMD1_DW41_BIT8, params.vdnecCmd1Par97[2]);    \
    DO_FIELD(DW41, VDENC_CMD1_DW41_BIT16, params.vdnecCmd1Par98[2]);   \
    DO_FIELD(DW41, VDENC_CMD1_DW41_BIT24, params.vdnecCmd1Par99[2]);   \
    DO_FIELD(DW42, VDENC_CMD1_DW42_BIT0, params.vdnecCmd1Par96[3]);    \
    DO_FIELD(DW42, VDENC_CMD1_DW42_BIT8, params.vdnecCmd1Par97[3]);    \
    DO_FIELD(DW42, VDENC_CMD1_DW42_BIT16, params.vdnecCmd1Par98[3]);   \
    DO_FIELD(DW42, VDENC_CMD1_DW42_BIT24, params.vdnecCmd1Par99[3]);   \
    DO_FIELD(DW43, VDENC_CMD1_DW43_BIT0, params.vdnecCmd1Par100[0]);   \
    DO_FIELD(DW43, VDENC_CMD1_DW43_BIT8, params.vdnecCmd1Par101[0]);   \
    DO_FIELD(DW43, VDENC_CMD1_DW43_BIT16, params.vdnecCmd1Par102[0]);  \
    DO_FIELD(DW43, VDENC_CMD1_DW43_BIT24, params.vdnecCmd1Par103[0]);  \
    DO_FIELD(DW44, VDENC_CMD1_DW44_BIT0, params.vdnecCmd1Par100[1]);   \
    DO_FIELD(DW44, VDENC_CMD1_DW44_BIT8, params.vdnecCmd1Par101[1]);   \
    DO_FIELD(DW44, VDENC_CMD1_DW44_BIT16, params.vdnecCmd1Par102[1]);  \
    DO_FIELD(DW44, VDENC_CMD1_DW44_BIT24, params.vdnecCmd1Par103[1]);  \
    DO_FIELD(DW45, VDENC_CMD1_DW45_BIT16, params.vdencCmd1Par104[0]);  \
    DO_FIELD(DW45, VDENC_CMD1_DW45_BIT8, params.vdencCmd1Par105[0]);   \
    DO_FIELD(DW45, VDENC_CMD1_DW45_BIT0, params.vdencCmd1Par106[0]);   \
    DO_FIELD(DW45, VDENC_CMD1_DW45_BIT21, params.vdencCmd1Par107);     \
    DO_FIELD(DW46, VDENC_CMD1_DW46_BIT16, params.vdencCmd1Par104[1]);  \
    DO_FIELD(DW46, VDENC_CMD1_DW46_BIT8, params.vdencCmd1Par105[1]);   \
    DO_FIELD(DW46, VDENC_CMD1_DW46_BIT0, params.vdencCmd1Par106[1]);   \
    DO_FIELD(DW47, VDENC_CMD1_DW47_BIT16, params.vdencCmd1Par104[2]);  \
    DO_FIELD(DW47, VDENC_CMD1_DW47_BIT8, params.vdencCmd1Par105[2]);   \
    DO_FIELD(DW47, VDENC_CMD1_DW47_BIT0, params.vdencCmd1Par106[2]);   \
    DO_FIELD(DW48, VDENC_CMD1_DW48_BIT16, params.vdencCmd1Par104[3]);  \
    DO_FIELD(DW48, VDENC_CMD1_DW48_BIT8, params.vdencCmd1Par105[3]);   \
    DO_FIELD(DW48, VDENC_CMD1_DW48_BIT0, params.vdencCmd1Par106[3])

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD2)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD2);

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD2_IMPL_XE3_LPM_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_HEVC_VP9_TILE_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_HEVC_VP9_TILE_SLICE_STATE);

#define DO_FIELDS()                                                                                 \
    DO_FIELD(DW11, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW11_BIT24, params.VdencHEVCVP9TileSlicePar22);  \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT0, params.VdencHEVCVP9TileSlicePar5);    \
    DO_FIELD(DW12, PaletteModeEnable, params.paletteModeEnable);                                    \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT8, params.VdencHEVCVP9TileSlicePar8);    \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT0, params.VdencHEVCVP9TileSlicePar9);    \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT16, params.VdencHEVCVP9TileSlicePar10)

#include "mhw_hwcmd_process_cmdfields.h"
    }
    
    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_MODE_SELECT);
#define DO_FIELDS()                                                                  \
    DO_FIELD(DW1, VDENC_PIPE_MODE_SELECT_DW1_BIT29, params.VdencPipeModeSelectPar8); \
    DO_FIELD(DW1, ChromaPrefetchDisable, params.chromaPrefetchDisable);              \
    DO_FIELD(DW6, FastPassEn, params.fastPassEn);                                    \
    DO_FIELD(DW6, FastPassScale, params.fastPassScale);                              \
    DO_FIELD(DW6, DownScaleType, params.DownScaleType)

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_PIPE_MODE_SELECT_IMPL_XE3_LPM_BASE)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_BUF_ADDR_STATE);

        __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_PIPE_BUF_ADDR_IMPL_XE3_LPM_BASE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_AVC_IMG_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_AVC_IMG_STATE);

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_AVC_IMG_STATE_IMPL_XE2_HPM_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VD_PIPELINE_FLUSH)
    {
        _MHW_SETCMD_CALLBASE(VD_PIPELINE_FLUSH);

#define DO_FIELDS()                                                           \
    DO_FIELD(DW1, VdaqmPipelineDone, params.waitDoneVDAQM);                   \
    DO_FIELD(DW1, VdaqmPipelineCommandFlush, params.flushVDAQM);              \
    DO_FIELD(DW1, VdCommandMessageParserDone, params.waitDoneVDCmdMsgParser); \
    DO_FIELD(DW1, VvcpPipelineDone, params.vvcpPipelineDone);                 \
    DO_FIELD(DW1, VvcpPipelineCommandFlush, params.vvcpPipelineCommandFlush);

#include "mhw_hwcmd_process_cmdfields.h"
    }

protected:

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe3_lpm_base__xe3_lpm__Impl)
    
};
}  // namespace xe3_lpm
}  // namespace xe3_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE3_LPM_H__
