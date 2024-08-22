/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_impl_xe2_hpm.h
//! \brief    MHW VDBOX VDENC interface common base for Xe2_HPM
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE2_HPM_H__
#define __MHW_VDBOX_VDENC_IMPL_XE2_HPM_H__

#include "mhw_vdbox_vdenc_impl_xe_lpm_plus_base.h"
#include "mhw_vdbox_vdenc_hwcmd_xe2_hpm.h"

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe_lpm_plus_base
{
namespace v1
{
class Impl : public BaseImplGeneric<Cmd>
{
protected:
    using cmd_t  = Cmd;
    using base_t = BaseImplGeneric<cmd_t>;

public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf){};

protected:
    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD1)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD1);

#define DO_FIELDS()                                                   \
    DO_FIELD(DW9,  VDENC_CMD1_DW9_BIT24, params.vdencCmd1Par95);      \
    DO_FIELD(DW32, VDENC_CMD1_DW32_BIT24, params.vdencCmd1Par93);     \
    DO_FIELD(DW31, VDENC_CMD1_DW31_BIT24, params.vdencCmd1Par94);     \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT0, params.vdencCmd1Par11[1]);   \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT8, params.vdencCmd1Par15[3]);   \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT16, params.vdencCmd1Par15[2]);  \
    DO_FIELD(DW33, VDENC_CMD1_DW33_BIT24, params.vdencCmd1Par15[1]);  \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT0, params.vdencCmd1Par14[2]);   \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT8, params.vdencCmd1Par14[1]);   \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT16, params.vdencCmd1Par11[3]);  \
    DO_FIELD(DW34, VDENC_CMD1_DW34_BIT24, params.vdencCmd1Par11[2]);  \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT0, params.vdencCmd1Par10[3]);   \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT8, params.vdencCmd1Par10[2]);   \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT16, params.vdencCmd1Par10[1]);  \
    DO_FIELD(DW35, VDENC_CMD1_DW35_BIT24, params.vdencCmd1Par14[3]);  \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT0, params.vdencCmd1Par9[1]);    \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT8, params.vdencCmd1Par13[3]);   \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT16, params.vdencCmd1Par13[2]);  \
    DO_FIELD(DW36, VDENC_CMD1_DW36_BIT24, params.vdencCmd1Par13[1]);  \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT0, params.vdencCmd1Par12[2]);   \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT8, params.vdencCmd1Par12[1]);   \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT16, params.vdencCmd1Par9[3]);   \
    DO_FIELD(DW37, VDENC_CMD1_DW37_BIT24, params.vdencCmd1Par9[2]);   \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT0, params.vdencCmd1Par8[3]);    \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT8, params.vdencCmd1Par8[2]);    \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT16, params.vdencCmd1Par8[1]);   \
    DO_FIELD(DW38, VDENC_CMD1_DW38_BIT24, params.vdencCmd1Par12[3])

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD2)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD2);

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD2_IMPL_XE2_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_HEVC_VP9_TILE_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_HEVC_VP9_TILE_SLICE_STATE);

#define DO_FIELDS() \
    DO_FIELD(DW11, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW11_BIT24, params.VdencHEVCVP9TileSlicePar22); \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT0, params.VdencHEVCVP9TileSlicePar5);   \
    DO_FIELD(DW12, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW12_BIT3, params.VdencHEVCVP9TileSlicePar24);  \
    DO_FIELD(DW12, PaletteModeEnable, params.paletteModeEnable);                                   \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT8, params.VdencHEVCVP9TileSlicePar8);   \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT0, params.VdencHEVCVP9TileSlicePar9);   \
    DO_FIELD(DW14, VDENC_HEVC_VP9_TILE_SLICE_STATE_DW14_BIT16, params.VdencHEVCVP9TileSlicePar10)

#include "mhw_hwcmd_process_cmdfields.h"
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

#define DO_FIELDS()                                                            \
    DO_FIELD(DW1, VdaqmPipelineDone, params.waitDoneVDAQM);                    \
    DO_FIELD(DW1, VdaqmPipelineCommandFlush, params.flushVDAQM)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_MODE_SELECT);
#define DO_FIELDS()                                     \
    DO_FIELD(DW6, FastPassEn, params.fastPassEn);       \
    DO_FIELD(DW6, FastPassScale, params.fastPassScale); \
    DO_FIELD(DW6, DownScaleType, params.DownScaleType)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe_lpm_plus_base__v1__Impl)
};
}  // namespace v1
}  // namespace xe_lpm_plus_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE2_HPM_H__
