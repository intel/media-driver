/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     mhw_vdbox_vdenc_itf.h
//! \brief    MHW VDBOX VDENC interface common base
//! \details
//!

#ifndef __MHW_VDBOX_VDENC_ITF_H__
#define __MHW_VDBOX_VDENC_ITF_H__

#include "mhw_def.h"
#include "mhw_vdbox_vdenc_cmdpar.h"

namespace mhw
{
namespace vdbox
{
namespace vdenc
{
class Itf
{
public:
    virtual ~Itf() = default;

    virtual MOS_STATUS SetRowstoreCachingAddrs(const vdbox::RowStoreCacheParams &params) = 0;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_CONTROL_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_PIPE_MODE_SELECT);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_SRC_SURFACE_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_REF_SURFACE_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_DS_REF_SURFACE_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_PIPE_BUF_ADDR_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_WEIGHTSOFFSETS_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VDENC_WALKER_STATE);

    _MHW_CMD_ALL_DEF_FOR_ITF(VD_PIPELINE_FLUSH);
};
}  // namespace vdenc
}  // namespace vdbox
}  // namespace mhw

#ifndef MHW_VDBOX_VDENC_ITF_T
#define MHW_VDBOX_VDENC_ITF_T mhw::vdbox::vdenc::Itf
#endif

#define MHW_VDBOX_VDENC_GET_CMD_PAR(cmd, reset) \
    mhw::DynamicPointerCast<MHW_VDBOX_VDENC_CMD_PAR_T(cmd)>(m_vdencItf->__MHW_CMD_PAR_GET_F(cmd)(reset))

#define MHW_VDBOX_VDENC_GET_CMD_SIZE(cmd) m_vdencItf->__MHW_CMD_BYTE_SIZE_GET_F(cmd)()

#define MHW_VDBOX_VDENC_ADD_CMD(cmd, ...) MHW_CHK_STATUS_RETURN(m_vdencItf->__MHW_CMD_ADD_F(cmd)(__VA_ARGS__))

#define MHW_VDBOX_VDENC_SETPARAMS_AND_ADDCMD(cmd, ...) \
    _MHW_SETPARAMS_AND_ADDCMD(cmd, MHW_VDBOX_VDENC_CMDPAR_T, MHW_VDBOX_VDENC_GET_CMD_PAR, MHW_VDBOX_VDENC_ADD_CMD, __VA_ARGS__)

#endif  // __MHW_VDBOX_VDENC_ITF_H__
