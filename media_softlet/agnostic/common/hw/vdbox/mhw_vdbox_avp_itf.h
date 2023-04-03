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
//! \file     mhw_vdbox_avp_itf.h
//! \brief    MHW VDBOX AVP interface common base
//! \details
//!

#ifndef __MHW_VDBOX_AVP_ITF_H__
#define __MHW_VDBOX_AVP_ITF_H__

#include "mhw_itf.h"
#include "mhw_vdbox_avp_cmdpar.h"

#define _AVP_CMD_DEF(DEF)             \
    DEF(AVP_PIPE_MODE_SELECT);        \
    DEF(AVP_PIC_STATE);               \
    DEF(AVP_INLOOP_FILTER_STATE);     \
    DEF(AVP_TILE_CODING);             \
    DEF(AVP_SEGMENT_STATE);           \
    DEF(AVP_PIPE_BUF_ADDR_STATE);     \
    DEF(AVP_INTER_PRED_STATE);        \
    DEF(AVP_IND_OBJ_BASE_ADDR_STATE); \
    DEF(AVP_SURFACE_STATE);           \
    DEF(AVP_BSD_OBJECT);              \
    DEF(AVP_PAK_INSERT_OBJECT);       \
    DEF(AVP_FILM_GRAIN_STATE)

namespace mhw
{
namespace vdbox
{
namespace avp
{
//! \struct   MmioRegistersAvp
//! \brief    MMIO registers AVP
//!
struct AvpMmioRegisters
{
    uint32_t                   avpAv1BitstreamByteCountTileRegOffset         = 0;
    uint32_t                   avpAv1BitstreamByteCountTileNoHeaderRegOffset = 0;
    uint32_t                   avpAv1CabacBinCountTileRegOffset              = 0;
    uint32_t                   avpAv1CabacInsertionCountRegOffset            = 0;
    uint32_t                   avpAv1MinSizePaddingCountRegOffset            = 0;
    uint32_t                   avpAv1ImageStatusMaskRegOffset                = 0;
    uint32_t                   avpAv1ImageStatusControlRegOffset             = 0;
    uint32_t                   avpAv1QpStatusCountRegOffset                  = 0;
    uint32_t                   avpAv1DecErrorStatusAddrRegOffset             = 0;
};

class Itf
{
public:

    enum CommandsNumberOfAddress
    {
        AVP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES        = 0,
        AVP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,
        AVP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     = 65,
        AVP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 3,
        AVP_PIC_STATE_CMD_NUMBER_OF_ADDRESSES               = 0,
        AVP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,
        AVP_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES           = 0,
        AVP_TILE_CODING_CMD_LST_NUMBER_OF_ADDRESSES         =  0,
        AVP_TILE_CODING_CMD_NUMBER_OF_ADDRESSES             = 0,
        AVP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              = 0,
        AVP_INLOOP_FILTER_STATE_CMD_NUMBER_OF_ADDRESSES     = 0,
        AVP_INTER_PRED_STATE_CMD_NUMBER_OF_ADDRESSES        = 0,
        AVP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES       = 0,
        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES           = 0,
        AVP_FILM_GRAIN_STATE_CMD_NUMBER_OF_ADDRESSES        = 0
    };

    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;

        _AVP_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) = 0;
    virtual MOS_STATUS GetAvpBufSize(AvpBufferType bufferType, AvpBufferSizePar *avpBufSizeParam) = 0;
    virtual bool IsRowStoreCachingSupported() = 0;    // Judge if row store caching is supported, overall flag for AVP
    virtual bool IsBufferRowstoreCacheEnabled(AvpBufferType bufferType) = 0; // If row store cache of a particular buffer is enabled
    virtual MOS_STATUS GetAvpPrimitiveCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;
    virtual MOS_STATUS GetAvpStateCmdSize(uint32_t *commandsSize, uint32_t *patchListSize, PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) = 0;
    virtual AvpMmioRegisters* GetMmioRegisters(MHW_VDBOX_NODE_IND index) = 0;
    virtual MOS_STATUS GetRowstoreCachingAddrs(mhw::vdbox::avp::AvpVdboxRowStorePar rowstoreParams) = 0;

    _AVP_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__Itf)
};
}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_ITF_H__
