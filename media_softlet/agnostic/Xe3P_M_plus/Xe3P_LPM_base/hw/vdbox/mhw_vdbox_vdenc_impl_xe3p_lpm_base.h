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
//! \file     mhw_vdbox_vdenc_impl_xe3p_lpm_base.h
//! \brief    MHW VDBOX VDENC interface common base for Xe3P_LPM_Base
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_IMPL_XE3P_LPM_BASE_H__
#define __MHW_VDBOX_VDENC_IMPL_XE3P_LPM_BASE_H__

#include "mhw_vdbox_vdenc_impl.h"

#ifdef _MEDIA_RESERVED
#include "mhw_vdbox_vdenc_impl_xe3p_lpm_base_ext.h"
#endif

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
namespace xe3p_lpm_base
{
template <typename cmd_t>
class BaseImpl : public vdenc::Impl<cmd_t>
{
public:
    MOS_STATUS SetRowstoreCachingOffsets(const RowStorePar &par) override
    {
        MHW_FUNCTION_ENTER;
        base_t::SetRowstoreCachingOffsets(par);

        switch (par.mode)
        {
        case RowStorePar::AV1: {
            if (this->m_rowStoreCache.vdenc.supported)
            {
                if (par.frameWidth > 2048)
                {
                    this->m_rowStoreCache.vdenc.enabled = false;
                }
            }
            break;
        }
        default: {
            break;
        }
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = vdenc::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_MODE_SELECT);
#define DO_FIELDS()                                                                  \
    DO_FIELD(DW1, VDENC_PIPE_MODE_SELECT_DW1_BIT29, params.VdencPipeModeSelectPar8); \
    DO_FIELD(DW6, FastPassEn, params.fastPassEn);                                    \
    DO_FIELD(DW6, FastPassScale, params.fastPassScale);                              \
    DO_FIELD(DW6, DownScaleType, params.DownScaleType);                              \
    DO_FIELD(DW6, RgbYuvBt2020, params.bt2020RGB2YUV );                              \
    DO_FIELD(DW6, Rgb10InputRange, params.rgbInputStudioRange);                      \
    DO_FIELD(DW6, YuvOutputRange, params.convertedYUVStudioRange);

#define DO_FIELDS_EXT() \
        __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_PIPE_MODE_SELECT_IMPL_XE3P_LPM_BASE)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(VDENC_PIPE_BUF_ADDR_STATE);

        __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_PIPE_BUF_ADDR_IMPL_XE3P_LPM_BASE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(VDENC_CMD2)
    {
        _MHW_SETCMD_CALLBASE(VDENC_CMD2);

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_VDENC_WRAPPER_EXT(VDENC_CMD2_IMPL_XE3P_LPM_EXT)

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

#define DO_FIELDS_EXT() \
        __MHW_VDBOX_VDENC_WRAPPER_EXT(VD_PIPELINE_FLUSH_IMPL_XE3P_LPM_BASE_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

MEDIA_CLASS_DEFINE_END(mhw__vdbox__vdenc__xe3p_lpm_base__BaseImpl)
};
}  // namespace xe3p_lpm_base
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_VDENC_IMPL_XE3P_LPM_BASE_H__
