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
//! \file     decode_hevc_tile_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for hevc decode tile coding packet Xe3_LPM+
//!
#include "decode_hevc_tile_packet_xe3_lpm_base.h"

namespace decode
{

HevcDecodeTilePktXe3_Lpm_Base::~HevcDecodeTilePktXe3_Lpm_Base()
{
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::Init()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_hevcPipeline);
    DECODE_CHK_NULL(m_hcpItf);

    m_hevcBasicFeature = dynamic_cast<HevcBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_hevcBasicFeature);

#if (_DEBUG || _RELEASE_INTERNAL)
    m_dbgOvrdWidthInMinCb = ReadUserFeature(m_pipeline->GetUserSetting(), "Scalability Split Width In MinCb", MediaUserSetting::Group::Sequence).Get<uint32_t>();
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint16_t tileX, uint16_t tileY)
{
    DECODE_CHK_STATUS(AddCmd_HCP_Tile_Coding(cmdBuffer, tileX, tileY));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint8_t virtualTileIdx)
{
    DECODE_CHK_STATUS(AddCmd_HCP_Tile_Coding(cmdBuffer, virtualTileIdx));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_hevcBasicFeature->m_hevcPicParams);
    m_hevcPicParams = m_hevcBasicFeature->m_hevcPicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::SET_HCP_TILE_CODING(uint16_t tileX, uint16_t tileY)
{
    DECODE_FUNC_CALL();
    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_TILE_CODING)();

    uint32_t minCbSize = m_hevcBasicFeature->m_minCtbSize;
    uint32_t LCUSize   = m_hevcBasicFeature->m_ctbSize;

    uint16_t startCtbX = m_hevcBasicFeature->m_tileCoding.GetTileCtbX(tileX);
    uint16_t startCtbY = m_hevcBasicFeature->m_tileCoding.GetTileCtbY(tileY);

    if (tileX < m_hevcPicParams->num_tile_columns_minus1)
    {
        const uint16_t *tileColWidth  = m_hevcBasicFeature->m_tileCoding.GetTileColWidth();
        params.tileWidthInMinCbMinus1 = (tileColWidth[tileX] << m_hevcPicParams->log2_diff_max_min_luma_coding_block_size) - 1;
    }
    else
    {
        params.tileWidthInMinCbMinus1 = m_hevcPicParams->PicWidthInMinCbsY - (startCtbX * LCUSize / minCbSize) - 1;
    }

    if (tileY < m_hevcPicParams->num_tile_rows_minus1)
    {
        const uint16_t *tileRowHeight  = m_hevcBasicFeature->m_tileCoding.GetTileRowHeight();
        params.tileHeightInMinCbMinus1 = (tileRowHeight[tileY] << m_hevcPicParams->log2_diff_max_min_luma_coding_block_size) - 1;
    }
    else
    {
        params.tileHeightInMinCbMinus1 = m_hevcPicParams->PicHeightInMinCbsY - (startCtbY * LCUSize / minCbSize) - 1;
    }

    params.tileStartLCUX      = startCtbX;
    params.tileStartLCUY      = startCtbY;
    params.isLastTileofColumn = (tileY == m_hevcPicParams->num_tile_rows_minus1);
    params.isLastTileofRow    = (tileX == m_hevcPicParams->num_tile_columns_minus1);

    MEDIA_SYSTEM_INFO *gtSystemInfo     = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    uint8_t            numVdbox         = (uint8_t)gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled;
    uint8_t            ucNumDecodePipes = m_hevcPipeline->GetPipeNum();
    uint8_t            ucPipeIdx        = m_hevcPipeline->GetCurrentPipe();

    MHW_ASSERT(ucNumDecodePipes <= numVdbox);
    MHW_ASSERT(ucPipeIdx < ucNumDecodePipes);

    params.numberOfActiveBePipes   = ucNumDecodePipes;
    params.numOfTileColumnsInFrame = ucNumDecodePipes;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::SET_HCP_TILE_CODING(uint8_t virtualTileIdx)
{
    DECODE_FUNC_CALL();

    DECODE_ASSERT(virtualTileIdx < m_virtualTileMaxNum);
    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_TILE_CODING)();

    uint8_t pipeNum = m_hevcPipeline->GetPipeNum();

    uint32_t widthInCtb  = m_hevcBasicFeature->m_widthInCtb;
    uint32_t heightInCtb = m_hevcBasicFeature->m_heightInCtb;

    uint16_t virtualTileWidthInLCU[m_virtualTileMaxNum];
    uint16_t virtualTileColPos = 0;

    // calc virtual tile width and position
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
        DECODE_CHK_STATUS_MESSAGE(MOS_STATUS_INVALID_PARAMETER, "HW limitation: Virtual tile for decode should be at least 2 LCU.");
    }

    uint32_t minCtbSize = m_hevcBasicFeature->m_minCtbSize;
    DECODE_ASSERT(minCtbSize != 0);
    uint32_t maxCtbSize = m_hevcBasicFeature->m_ctbSize;
    DECODE_ASSERT(maxCtbSize != 0);

    uint32_t widthInMinCtb  = m_hevcPicParams->PicWidthInMinCbsY;
    uint32_t heightInMinCtb = m_hevcPicParams->PicHeightInMinCbsY;

    // If the last tile, the tilewidth need to align to min CU not LCU.
    if (virtualTileIdx == (pipeNum - 1))
    {
        uint16_t virtualTileColPosInMinCtb = virtualTileColPos * maxCtbSize / minCtbSize;
        params.tileWidthInMinCbMinus1      = widthInMinCtb - virtualTileColPosInMinCtb - 1;
    }
    else
    {
        params.tileWidthInMinCbMinus1 = virtualTileWidthInLCU[virtualTileIdx] * maxCtbSize / minCtbSize - 1;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_dbgOvrdWidthInMinCb > 0 && pipeNum == 2)
    {
        if (virtualTileIdx == 1)
        {
            params.tileWidthInMinCbMinus1 = widthInMinCtb - m_dbgOvrdWidthInMinCb - 1;
            virtualTileColPos             = m_dbgOvrdWidthInMinCb * minCtbSize / maxCtbSize;
        }
        else
        {
            params.tileWidthInMinCbMinus1 = m_dbgOvrdWidthInMinCb - 1;
            virtualTileColPos             = 0;
        }
    }
#endif

    params.tileHeightInMinCbMinus1 = heightInMinCtb - 1;
    params.tileStartLCUX           = virtualTileColPos;

    uint8_t            ucNumDecodePipes = pipeNum;
    uint8_t            ucPipeIdx        = virtualTileIdx;
    MEDIA_SYSTEM_INFO *gtSystemInfo     = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    uint8_t            numVdbox         = (uint8_t)gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled;

    MHW_ASSERT(ucNumDecodePipes <= numVdbox);
    MHW_ASSERT(ucPipeIdx < ucNumDecodePipes);

    params.numberOfActiveBePipes   = ucNumDecodePipes;
    params.numOfTileColumnsInFrame = ucNumDecodePipes;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::AddCmd_HCP_Tile_Coding(MOS_COMMAND_BUFFER &cmdBuffer, uint16_t tileX, uint16_t tileY)
{
    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_TILE_CODING)();
    params       = {};

    DECODE_CHK_STATUS(SET_HCP_TILE_CODING(tileX, tileY));

    DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_TILE_CODING)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::AddCmd_HCP_Tile_Coding(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t virtualTileIdx)
{
    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_TILE_CODING)();
    params       = {};

    DECODE_CHK_STATUS(SET_HCP_TILE_CODING(virtualTileIdx));

    DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_TILE_CODING)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeTilePktXe3_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    auto hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
    DECODE_CHK_NULL(hcpItf);
    commandBufferSize      = hcpItf->MHW_GETSIZE_F(HCP_TILE_CODING)();
    requestedPatchListSize = 0;
    return MOS_STATUS_SUCCESS;
}

}
