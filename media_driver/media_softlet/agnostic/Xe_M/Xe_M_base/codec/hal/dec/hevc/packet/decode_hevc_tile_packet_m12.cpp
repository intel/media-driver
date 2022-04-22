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
//! \file     decode_hevc_tile_packet_m12.cpp
//! \brief    Defines the interface for hevc decode tile coding packet M12
//!
#include "codechal_utilities.h"
#include "decode_hevc_tile_packet_m12.h"

namespace decode
{

HevcDecodeTilePktM12::~HevcDecodeTilePktM12()
{
}

MOS_STATUS HevcDecodeTilePktM12::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hevcPipeline);
    DECODE_CHK_NULL(m_hcpInterface);

    m_hevcBasicFeature = dynamic_cast<HevcBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_hevcBasicFeature);

#if (_DEBUG || _RELEASE_INTERNAL)
    m_dbgOvrdWidthInMinCb = ReadUserFeature(m_pipeline->GetUserSetting(), "Scalability Split Width In MinCb", MediaUserSetting::Group::Sequence).Get<uint32_t>();
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint16_t tileX, uint16_t tileY)
{
    DECODE_CHK_STATUS(AddHcpTileCoding(cmdBuffer, tileX, tileY));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint8_t virtualTileIdx)
{
    DECODE_CHK_STATUS(AddHcpTileCoding(cmdBuffer, virtualTileIdx));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcPicParams);
    m_hevcPicParams = m_hevcBasicFeature->m_hevcPicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::SetHcpTileCodingParams(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 &tileCodingParams,
                                                        uint16_t tileX, uint16_t tileY)
{
    DECODE_FUNC_CALL();

    uint32_t minCbSize = m_hevcBasicFeature->m_minCtbSize;
    uint32_t LCUSize = m_hevcBasicFeature->m_ctbSize;

    uint16_t startCtbX = m_hevcBasicFeature->m_tileCoding.GetTileCtbX(tileX);
    uint16_t startCtbY = m_hevcBasicFeature->m_tileCoding.GetTileCtbY(tileY);

    if (tileX < m_hevcPicParams->num_tile_columns_minus1)
    {
        const uint16_t *tileColWidth = m_hevcBasicFeature->m_tileCoding.GetTileColWidth();
        tileCodingParams.TileWidthInMinCbMinus1 =
            (tileColWidth[tileX] << m_hevcPicParams->log2_diff_max_min_luma_coding_block_size) - 1;
    }
    else
    {
        tileCodingParams.TileWidthInMinCbMinus1 =
            m_hevcPicParams->PicWidthInMinCbsY - (startCtbX * LCUSize / minCbSize) - 1;
    }

    if (tileY < m_hevcPicParams->num_tile_rows_minus1)
    {
        const uint16_t *tileRowHeight = m_hevcBasicFeature->m_tileCoding.GetTileRowHeight();
        tileCodingParams.TileHeightInMinCbMinus1 =
            (tileRowHeight[tileY] << m_hevcPicParams->log2_diff_max_min_luma_coding_block_size) - 1;
    }
    else
    {
        tileCodingParams.TileHeightInMinCbMinus1 =
            m_hevcPicParams->PicHeightInMinCbsY - (startCtbY * LCUSize / minCbSize) - 1;
    }

    tileCodingParams.TileStartLCUX = startCtbX;
    tileCodingParams.TileStartLCUY = startCtbY;
    tileCodingParams.ucNumDecodePipes = m_hevcPipeline->GetPipeNum();
    tileCodingParams.ucPipeIdx = m_hevcPipeline->GetCurrentPipe();
    tileCodingParams.IsLastTileofColumn = (tileY == m_hevcPicParams->num_tile_rows_minus1);
    tileCodingParams.IsLastTileofRow = (tileX == m_hevcPicParams->num_tile_columns_minus1);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::SetHcpTileCodingParams(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 &tileCodingParams,
                                                        uint8_t virtualTileIdx)
{
    DECODE_FUNC_CALL();
    DECODE_ASSERT(virtualTileIdx < m_virtualTileMaxNum);

    uint8_t  pipeNum        = m_hevcPipeline->GetPipeNum();

    uint32_t widthInCtb     = m_hevcBasicFeature->m_widthInCtb;
    uint32_t heightInCtb    = m_hevcBasicFeature->m_heightInCtb;

    uint16_t virtualTileWidthInLCU[m_virtualTileMaxNum];
    uint16_t virtualTileColPos = 0;

    //calc virtual tile width and position
    for (uint32_t i = 0; i <= virtualTileIdx; i++)
    {
        virtualTileWidthInLCU[i] = ((i + 1) * widthInCtb / pipeNum) - (i * widthInCtb / pipeNum);
        if (i >= 1)
        {
            virtualTileColPos += virtualTileWidthInLCU[i - 1];
        }
    }

    if (virtualTileWidthInLCU[virtualTileIdx] < 2)
    {
        DECODE_CHK_STATUS_MESSAGE(MOS_STATUS_INVALID_PARAMETER,
            "HW limitation: Virtual tile for decode should be at least 2 LCU.");
    }

    uint32_t minCtbSize = m_hevcBasicFeature->m_minCtbSize;
    DECODE_ASSERT(minCtbSize != 0);
    uint32_t maxCtbSize = m_hevcBasicFeature->m_ctbSize;
    DECODE_ASSERT(maxCtbSize != 0);

    uint32_t widthInMinCtb  = m_hevcPicParams->PicWidthInMinCbsY;
    uint32_t heightInMinCtb = m_hevcPicParams->PicHeightInMinCbsY;

    //if the last tile, the tilewidth need to align to min CU not LCU.
    if (virtualTileIdx == (pipeNum - 1))
    {
        uint16_t virtualTileColPosInMinCtb = virtualTileColPos * maxCtbSize / minCtbSize;
        tileCodingParams.TileWidthInMinCbMinus1 = widthInMinCtb - virtualTileColPosInMinCtb - 1;
    }
    else
    {
        tileCodingParams.TileWidthInMinCbMinus1 = virtualTileWidthInLCU[virtualTileIdx] * maxCtbSize / minCtbSize - 1;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_dbgOvrdWidthInMinCb > 0 && pipeNum == 2)
    {
        if (virtualTileIdx == 1)
        {
            tileCodingParams.TileWidthInMinCbMinus1 = widthInMinCtb - m_dbgOvrdWidthInMinCb - 1;
            virtualTileColPos = m_dbgOvrdWidthInMinCb * minCtbSize / maxCtbSize;
        }
        else
        {
            tileCodingParams.TileWidthInMinCbMinus1 = m_dbgOvrdWidthInMinCb - 1;
            virtualTileColPos = 0;
        }
    }
#endif

    tileCodingParams.TileHeightInMinCbMinus1        = heightInMinCtb - 1;
    tileCodingParams.TileStartLCUX                  = virtualTileColPos;
    tileCodingParams.ucNumDecodePipes               = pipeNum;
    tileCodingParams.ucPipeIdx                      = virtualTileIdx;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::AddHcpTileCoding(MOS_COMMAND_BUFFER &cmdBuffer, uint16_t tileX, uint16_t tileY)
{
    MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileCodingParams;
    MOS_ZeroMemory(&tileCodingParams, sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12));

    DECODE_CHK_STATUS(SetHcpTileCodingParams(tileCodingParams, tileX, tileY));

    //insert 2 dummy VD_CONTROL_STATE packets with data=0 before every HCP_TILE_CODING
    if (MEDIA_IS_WA(m_hevcPipeline->GetWaTable(), Wa_14010222001))
    {
        MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
        DECODE_CHK_NULL(miInterfaceG12);

        MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        for (int i = 0; i < 2; i++)
        {
            DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));
        }
    }

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpTileCodingCmd(&cmdBuffer, &tileCodingParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::AddHcpTileCoding(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t virtualTileIdx)
{
    MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileCodingParams;
    MOS_ZeroMemory(&tileCodingParams, sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12));

    DECODE_CHK_STATUS(SetHcpTileCodingParams(tileCodingParams, virtualTileIdx));

    //insert 2 dummy VD_CONTROL_STATE packets with data=0 before every HCP_TILE_CODING
    if (MEDIA_IS_WA(m_hevcPipeline->GetWaTable(), Wa_14010222001))
    {
        MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
        DECODE_CHK_NULL(miInterfaceG12);

        MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        for (int i = 0; i < 2; i++)
        {
            DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));
        }
    }

    DECODE_CHK_STATUS(m_hcpInterface->AddHcpTileCodingCmd(&cmdBuffer, &tileCodingParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktM12::CalculateCommandSize(uint32_t &commandBufferSize,
                                                  uint32_t &requestedPatchListSize)
{
    commandBufferSize      = mhw_vdbox_hcp_g12_X::HCP_TILE_CODING_CMD::byteSize;
    requestedPatchListSize = 0;
    return MOS_STATUS_SUCCESS;
}

}
