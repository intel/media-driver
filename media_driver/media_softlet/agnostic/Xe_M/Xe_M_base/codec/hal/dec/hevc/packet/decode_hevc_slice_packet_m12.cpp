/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_hevc_slice_packet_m12.cpp
//! \brief    Defines the interface for hevc decode slice packet for M12
//!
#include "codechal_utilities.h"
#include "decode_hevc_slice_packet_m12.h"
#include "mhw_vdbox_hcp_g12_X.h"

namespace decode
{

HevcDecodeSlcPktM12::~HevcDecodeSlcPktM12()
{
}

MOS_STATUS HevcDecodeSlcPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
{
    DECODE_CHK_STATUS(AddHcpPaletteInitializerState(cmdBuffer, sliceIdx));
    DECODE_CHK_STATUS(AddHcpSliceState(cmdBuffer, sliceIdx, subTileIdx));
    DECODE_CHK_STATUS(AddRefIdxState(cmdBuffer, sliceIdx));
    DECODE_CHK_STATUS(AddWeightOffset(cmdBuffer, sliceIdx));
    DECODE_CHK_STATUS(AddHcpCpState(cmdBuffer, sliceIdx, subTileIdx));
    DECODE_CHK_STATUS(AddBsdObj(cmdBuffer, sliceIdx, subTileIdx));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktM12::AddHcpPaletteInitializerState(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            sliceIdx)
{
    const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
    DECODE_CHK_NULL(sliceTileInfo);

    bool sccPaletteMode = m_hevcBasicFeature->m_isSCCPLTMode;
    bool firstSliceOfTile = sliceTileInfo->firstSliceOfTile;
    bool independentSlice = m_hevcBasicFeature->IsIndependentSlice(sliceIdx);

    if (sccPaletteMode && (firstSliceOfTile || independentSlice))
    {
        MhwVdboxHcpInterfaceG12* hcpInterfaceG12 = dynamic_cast<MhwVdboxHcpInterfaceG12*>(m_hcpInterface);
        DECODE_CHK_NULL(hcpInterfaceG12);
        DECODE_CHK_STATUS(hcpInterfaceG12->AddHcpPaletteInitializerStateCmd(&cmdBuffer, m_hevcSccPicParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktM12::SetHcpSliceStateParams(
    MHW_VDBOX_HEVC_SLICE_STATE &sliceStateParamsBase,
    uint32_t                    sliceIdx,
    uint32_t                    subTileIdx)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HEVC_SLICE_STATE_G12 &sliceStateParams =
        static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12&>(sliceStateParamsBase);

    const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
    DECODE_CHK_NULL(sliceTileInfo);
    DECODE_CHK_STATUS(ValidateSubTileIdx(*sliceTileInfo, subTileIdx));

    DECODE_CHK_STATUS(HevcDecodeSlcPktXe_M_Base::SetHcpSliceStateParams(sliceStateParamsBase, sliceIdx, subTileIdx));

    sliceStateParams.pHevcExtPicParam    = m_hevcRextPicParams;
    sliceStateParams.pHevcSccPicParam    = m_hevcSccPicParams;
    sliceStateParams.pHevcExtSliceParams = m_hevcRextSliceParams + sliceIdx;

    sliceStateParams.u16OrigCtbX         = sliceTileInfo->origCtbX;
    sliceStateParams.u16OrigCtbY         = sliceTileInfo->origCtbY;

    sliceStateParams.bTileInSlice        = sliceTileInfo->numTiles > 1;
    if (sliceStateParams.bTileInSlice)
    {
        CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcBasicFeature->m_hevcSliceParams + sliceIdx;

        sliceStateParams.u16SliceHeaderLength = (subTileIdx == 0) ? sliceParams->ByteOffsetToSliceData : 0;
        sliceStateParams.u16TileCtbX          = sliceTileInfo->tileArrayBuf[subTileIdx].ctbX;
        sliceStateParams.u16TileCtbY          = sliceTileInfo->tileArrayBuf[subTileIdx].ctbY;
        sliceStateParams.dwOffset             = sliceTileInfo->tileArrayBuf[subTileIdx].bsdOffset;
        sliceStateParams.dwLength             = sliceTileInfo->tileArrayBuf[subTileIdx].bsdLength;
        sliceStateParams.bLastSlice           = m_hevcBasicFeature->IsLastSlice(sliceIdx) &&
                                                (subTileIdx == sliceTileInfo->numTiles - 1);
        sliceStateParams.bIsNotFirstTile      = (subTileIdx != 0);
        sliceStateParams.bLastSliceInTile     = true;

        uint16_t tileY = (sliceTileInfo->sliceTileX + subTileIdx) / (m_hevcPicParams->num_tile_columns_minus1 + 1) +
                         sliceTileInfo->sliceTileY;
        sliceStateParams.bLastSliceInTileColumn = (tileY == m_hevcPicParams->num_tile_rows_minus1);

        if (sliceStateParams.bLastSlice)
        {
            sliceStateParams.u16NextTileCtbX = 0;
            sliceStateParams.u16NextTileCtbY = 0;
        }
        else if (subTileIdx == sliceTileInfo->numTiles - 1)
        {
            sliceStateParams.u16NextTileCtbX = (sliceParams + 1)->slice_segment_address %
                                               m_hevcBasicFeature->m_widthInCtb;
            sliceStateParams.u16NextTileCtbY = (sliceParams + 1)->slice_segment_address /
                                               m_hevcBasicFeature->m_widthInCtb;
        }
        else
        {
            sliceStateParams.u16NextTileCtbX = sliceTileInfo->tileArrayBuf[subTileIdx + 1].ctbX;
            sliceStateParams.u16NextTileCtbY = sliceTileInfo->tileArrayBuf[subTileIdx + 1].ctbY;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktM12::AddHcpSliceState(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t           sliceIdx,
    uint32_t           subTileIdx)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_HEVC_SLICE_STATE_G12 sliceStateParams;
    DECODE_CHK_STATUS(SetHcpSliceStateParams(sliceStateParams, sliceIdx, subTileIdx));
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpSliceStateCmd(&cmdBuffer, &sliceStateParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSlcPktM12::AddRefIdxState(
    MOS_COMMAND_BUFFER &cmdBuffer,
    uint32_t            sliceIdx)
{
    CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcBasicFeature->m_hevcSliceParams + sliceIdx;

    // Below cases don't need refIdxState command.
    // 1. I slice in simulation.
    // 2. I slice without dummy reference workaround.
    if (m_hcpInterface->IsHevcISlice(sliceParams->LongSliceFlags.fields.slice_type) &&
        (!m_hevcBasicFeature->m_useDummyReference || m_osInterface->bSimIsActive))
    {
        return MOS_STATUS_SUCCESS;
    }

    MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 refIdxParams;

    DECODE_CHK_STATUS(SetRefIdxParams(refIdxParams, sliceIdx));
    DECODE_CHK_STATUS(m_hcpInterface->AddHcpRefIdxStateCmd(&cmdBuffer, nullptr, &refIdxParams));

    if (m_hcpInterface->IsHevcBSlice(sliceParams->LongSliceFlags.fields.slice_type))
    {
        refIdxParams.ucList = 1;
        refIdxParams.ucNumRefForList = sliceParams->num_ref_idx_l1_active_minus1 + 1;
        DECODE_CHK_STATUS(m_hcpInterface->AddHcpRefIdxStateCmd(&cmdBuffer, nullptr, &refIdxParams));
    }

    return MOS_STATUS_SUCCESS;
}

}
