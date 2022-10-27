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
//! \file     decode_vp9_tile_packet_xe_lpm_plus_base.cpp
//! \brief    Defines the interface for vp9 decode tile coding packet Xe_LPM_plus+
//!
#include "decode_vp9_tile_packet_xe_lpm_plus_base.h"

namespace decode
{

Vp9DecodeTilePktXe_Lpm_Plus_Base::~Vp9DecodeTilePktXe_Lpm_Plus_Base()
{
}

MOS_STATUS Vp9DecodeTilePktXe_Lpm_Plus_Base::Init()
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_vp9Pipeline);
    DECODE_CHK_NULL(m_hcpItf);

    m_vp9BasicFeature = dynamic_cast<Vp9BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_vp9BasicFeature);

#if (_DEBUG || _RELEASE_INTERNAL)
    m_dbgOvrdWidthInMinCb = ReadUserFeature(m_pipeline->GetUserSetting(), "Scalability Split Width In MinCb", MediaUserSetting::Group::Sequence).Get<uint32_t>();
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeTilePktXe_Lpm_Plus_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, uint8_t virtualTileIdx)
{
    DECODE_CHK_STATUS(AddCmd_HCP_TILE_CODING(cmdBuffer, virtualTileIdx));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeTilePktXe_Lpm_Plus_Base::Prepare()
{
    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(m_vp9BasicFeature->m_vp9PicParams);
    m_vp9PicParams = m_vp9BasicFeature->m_vp9PicParams;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeTilePktXe_Lpm_Plus_Base::SET_HCP_TILE_CODING(uint8_t virtualTileIdx)
{
    DECODE_FUNC_CALL();

    DECODE_ASSERT(virtualTileIdx < m_virtualTileMaxNum);

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_TILE_CODING)();

    uint8_t  pipeNum    = m_vp9Pipeline->GetPipeNum();
    uint32_t widthInCtb = MOS_ROUNDUP_DIVIDE(m_vp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_SUPER_BLOCK_WIDTH);
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
        DECODE_CHK_STATUS_MESSAGE(MOS_STATUS_INVALID_PARAMETER,
            "HW limitation: Virtual tile for decode should be at least 2 LCU.");
    }

    uint32_t minCtbSize = CODEC_VP9_MIN_BLOCK_WIDTH;
    uint32_t maxCtbSize = CODEC_VP9_SUPER_BLOCK_WIDTH;

    uint32_t widthInMinCtb  = MOS_ROUNDUP_DIVIDE(m_vp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
    uint32_t heightInMinCtb = MOS_ROUNDUP_DIVIDE(m_vp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);

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

    MEDIA_SYSTEM_INFO *gtSystemInfo     = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    uint8_t            numVdbox         = (uint8_t)gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled;
    uint8_t            ucNumDecodePipes = pipeNum;
    uint8_t            ucPipeIdx        = virtualTileIdx;

    MHW_ASSERT(ucNumDecodePipes <= numVdbox);
    MHW_ASSERT(ucPipeIdx < ucNumDecodePipes);

    params.numberOfActiveBePipes   = ucNumDecodePipes;
    params.numOfTileColumnsInFrame = ucNumDecodePipes;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeTilePktXe_Lpm_Plus_Base::AddCmd_HCP_TILE_CODING(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t virtualTileIdx)
{
    DECODE_FUNC_CALL();

    auto &params = m_hcpItf->MHW_GETPAR_F(HCP_TILE_CODING)();
    params       = {};

    DECODE_CHK_STATUS(SET_HCP_TILE_CODING(virtualTileIdx));

    DECODE_CHK_STATUS(m_hcpItf->MHW_ADDCMD_F(HCP_TILE_CODING)(&cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9DecodeTilePktXe_Lpm_Plus_Base::CalculateCommandSize(uint32_t &commandBufferSize,
                                                  uint32_t &requestedPatchListSize)
{
    DECODE_FUNC_CALL();

    auto hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
    DECODE_CHK_NULL(hcpItf);
    commandBufferSize = hcpItf->MHW_GETSIZE_F(HCP_TILE_CODING)();

    requestedPatchListSize = 0;

    return MOS_STATUS_SUCCESS;
}

}
