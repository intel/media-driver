/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_vdbox_hcp_itf.h
//! \brief    MHW VDBOX HCP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_HCP_ITF_H__
#define __MHW_VDBOX_HCP_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox_hcp_cmdpar.h"

#define _HCP_CMD_DEF(DEF)             \
    DEF(HCP_SURFACE_STATE);           \
    DEF(HCP_PIC_STATE);               \
    DEF(HCP_SLICE_STATE);             \
    DEF(HCP_IND_OBJ_BASE_ADDR_STATE); \
    DEF(HCP_QM_STATE);                \
    DEF(HCP_BSD_OBJECT);              \
    DEF(HCP_TILE_STATE);              \
    DEF(HCP_REF_IDX_STATE);           \
    DEF(HCP_WEIGHTOFFSET_STATE);      \
    DEF(HCP_PIPE_MODE_SELECT);        \
    DEF(HCP_PIPE_BUF_ADDR_STATE);     \
    DEF(HCP_FQM_STATE);               \
    DEF(HCP_PAK_INSERT_OBJECT);       \
    DEF(HCP_VP9_PIC_STATE);           \
    DEF(HCP_VP9_SEGMENT_STATE);       \
    DEF(HEVC_VP9_RDOQ_STATE);         \
    DEF(HCP_TILE_CODING);             \
    DEF(HCP_PALETTE_INITIALIZER_STATE)

namespace mhw
{
namespace vdbox
{
namespace hcp
{
class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _HCP_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;

    virtual MOS_STATUS GetHcpBufSize(const HcpBufferSizePar &par, uint32_t &size) = 0;

    virtual const HcpMmioRegisters *GetMmioRegisters(const MHW_VDBOX_NODE_IND index) const = 0;

    virtual uint32_t GetEncCuRecordSize() = 0;

    virtual uint32_t GetHcpPakObjSize() = 0;

    virtual bool IsRowStoreCachingSupported() = 0;

    virtual uint32_t GetPakHWTileSizeRecordSize() = 0;

    virtual MOS_STATUS SetRowstoreCachingOffsets(const HcpVdboxRowStorePar &rowstoreParams) = 0;

    _HCP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
};
}  // namespace hcp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_ITF_H__
