/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_hevc_slice_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for hevc decode slice packet for Xe_LPM_plus+
//!
#include "decode_hevc_slice_packet_xe_lpm_plus_base.h"

namespace decode
{

    HevcDecodeSlcPktXe_Lpm_Plus_Base::~HevcDecodeSlcPktXe_Lpm_Plus_Base()
    {
    }

    MOS_STATUS HevcDecodeSlcPktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
    {
        DECODE_CHK_STATUS(AddCmd_HCP_PALETTE_INITIALIZER_STATE(cmdBuffer, sliceIdx));
        DECODE_CHK_STATUS(AddCmd_HCP_SLICE_STATE(cmdBuffer, sliceIdx, subTileIdx));
        DECODE_CHK_STATUS(AddCmd_HCP_REF_IDX_STATE(cmdBuffer, sliceIdx));
        DECODE_CHK_STATUS(AddCmd_HCP_WEIGHTOFFSET_STATE(cmdBuffer, sliceIdx));
        DECODE_CHK_STATUS(AddHcpCpState(cmdBuffer, sliceIdx, subTileIdx));
        DECODE_CHK_STATUS(AddCmd_HCP_BSD_OBJECT(cmdBuffer, sliceIdx, subTileIdx));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPktXe_Lpm_Plus_Base::Set_HCP_SLICE_STATE(uint32_t sliceIdx, uint32_t subTileIdx)
    {
        DECODE_FUNC_CALL();

        const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(sliceIdx);
        DECODE_CHK_NULL(sliceTileInfo);
        DECODE_CHK_STATUS(ValidateSubTileIdx(*sliceTileInfo, subTileIdx));

        DECODE_CHK_STATUS(HevcDecodeSlcPkt::SET_HCP_SLICE_STATE(sliceIdx, subTileIdx));

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_SLICE_STATE)();

        params.originalSliceStartCtbX = sliceTileInfo->origCtbX;
        params.originalSliceStartCtbY = sliceTileInfo->origCtbY;

        bool bTileInSlice = sliceTileInfo->numTiles > 1;
        if (bTileInSlice)
        {
            CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcBasicFeature->m_hevcSliceParams + sliceIdx;

            params.sliceheaderlength                     = (subTileIdx == 0) ? sliceParams->ByteOffsetToSliceData : 0;
            params.slicestartctbxOrSliceStartLcuXEncoder = sliceTileInfo->tileArrayBuf[subTileIdx].ctbX;
            params.slicestartctbyOrSliceStartLcuYEncoder = sliceTileInfo->tileArrayBuf[subTileIdx].ctbY;
            params.lastsliceofpic                        = m_hevcBasicFeature->IsLastSlice(sliceIdx) && (subTileIdx == sliceTileInfo->numTiles - 1);
            params.bIsNotFirstTile                       = (subTileIdx != 0);
            params.lastSliceInTile                       = true;

            uint16_t tileY = (sliceTileInfo->sliceTileX + subTileIdx) / (m_hevcPicParams->num_tile_columns_minus1 + 1) + sliceTileInfo->sliceTileY;
            params.lastSliceInTileColumn = (tileY == m_hevcPicParams->num_tile_rows_minus1);

            if (params.lastsliceofpic)
            {
                params.nextslicestartctbxOrNextSliceStartLcuXEncoder = 0;
                params.nextslicestartctbyOrNextSliceStartLcuYEncoder = 0;
            }
            else if (subTileIdx == sliceTileInfo->numTiles - 1)
            {
                params.nextslicestartctbxOrNextSliceStartLcuXEncoder = (sliceParams + 1)->slice_segment_address % m_hevcBasicFeature->m_widthInCtb;
                params.nextslicestartctbyOrNextSliceStartLcuYEncoder = (sliceParams + 1)->slice_segment_address / m_hevcBasicFeature->m_widthInCtb;
            }
            else
            {
                params.nextslicestartctbxOrNextSliceStartLcuXEncoder = sliceTileInfo->tileArrayBuf[subTileIdx + 1].ctbX;
                params.nextslicestartctbyOrNextSliceStartLcuYEncoder = sliceTileInfo->tileArrayBuf[subTileIdx + 1].ctbY;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPktXe_Lpm_Plus_Base::AddCmd_HCP_SLICE_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx, uint32_t subTileIdx)
    {
        DECODE_FUNC_CALL();

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_SLICE_STATE)();
        params       = {};

        DECODE_CHK_STATUS(Set_HCP_SLICE_STATE(sliceIdx, subTileIdx));
        DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_SLICE_STATE)(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcDecodeSlcPktXe_Lpm_Plus_Base::AddCmd_HCP_REF_IDX_STATE(MOS_COMMAND_BUFFER &cmdBuffer, uint32_t sliceIdx)
    {
        DECODE_FUNC_CALL();

        CODEC_HEVC_SLICE_PARAMS *sliceParams = m_hevcBasicFeature->m_hevcSliceParams + sliceIdx;

        // I slice don't need refIdxState command.
        if (m_hcpItf->IsHevcISlice(sliceParams->LongSliceFlags.fields.slice_type))
        {
            return MOS_STATUS_SUCCESS;
        }

        auto &params = m_hcpItf->MHW_GETPAR_F(HCP_REF_IDX_STATE)();
        params       = {};

        DECODE_CHK_STATUS(SET_HCP_REF_IDX_STATE(sliceIdx));
        DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_REF_IDX_STATE)(&cmdBuffer));

        if (m_hcpItf->IsHevcBSlice(sliceParams->LongSliceFlags.fields.slice_type))
        {
            params.ucList          = 1;
            params.ucNumRefForList = sliceParams->num_ref_idx_l1_active_minus1 + 1;
            DECODE_CHK_STATUS(SET_HCP_REF_IDX_STATE(sliceIdx));
            DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_REF_IDX_STATE)(&cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

}
