/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_decode_hevc_long_g12.cpp
//! \brief    Implements HEVC slice level command processing for HEVC long format.
//! \details  Implement HEVC slice level command processing for all HEVC/SCC configuration and 
//!           generate tile/slice level commands into second level buffer as short format.
//!

#include "codechal_decode_hevc_long_g12.h"

// HEVC Long format 
HevcDecodeSliceLongG12::HevcDecodeSliceLongG12(
    CodechalDecodeHevcG12       *decoder,
    MhwVdboxHcpInterface        *hcpInterface,
    MhwMiInterface              *miInterface)
{
    m_decoder = decoder;
    m_hcpInterface = static_cast<MhwVdboxHcpInterfaceG12*>(hcpInterface);
    m_miInterface = miInterface;

    //copy other params from decoder
    m_numSlices = m_decoder->m_numSlices;
    m_hevcPicParams = m_decoder->m_hevcPicParams;
    m_hevcSliceParams = m_decoder->m_hevcSliceParams;
    m_hevcExtPicParams = m_decoder->m_hevcExtPicParams;
    m_hevcExtSliceParams = m_decoder->m_hevcExtSliceParams;
    m_hevcSccPicParams = m_decoder->m_hevcSccPicParams;
    m_hevcSubsetParams = m_decoder->m_hevcSubsetParams;

    m_widthInCtb = MOS_ROUNDUP_DIVIDE(m_decoder->m_width, m_decoder->m_ctbSize);

    m_secureDecoder = m_decoder->m_secureDecoder;
    m_scalabilityState = m_decoder->m_scalabilityState;

    m_tileColWidth = &decoder->m_tileColWidth[0];
    m_tileRowHeight = &decoder->m_tileRowHeight[0];

    m_copyDataBufferInUse = m_decoder->m_copyDataBufferInUse;
    m_resCopyDataBuffer = &m_decoder->m_resCopyDataBuffer;
    m_resDataBuffer = &m_decoder->m_resDataBuffer;

    m_refIdxMapping = &m_decoder->m_refIdxMapping[0];
    m_hevcRefList = m_decoder->m_hevcRefList;

    m_isRealTile = m_decoder->m_isRealTile;
    m_isSeparateTileDecoding = m_decoder->m_isSeparateTileDecoding;
    m_isSccPaletteMode = CodecHalDecodeIsSCCPLTMode(m_hevcSccPicParams);
    m_tileDecoding = (m_isRealTile || m_isSeparateTileDecoding);
}

MOS_STATUS HevcDecodeSliceLongG12::ProcessSliceLong(uint8_t *cmdResBase, uint32_t cmdBufSize)
{
    auto eStatus = MOS_STATUS_SUCCESS;
    auto slc = m_hevcSliceParams;
    auto slcBase = slc;
    auto slcExt = m_hevcExtSliceParams;

    PMOS_COMMAND_BUFFER cmdBufArray, cmdBuf;

    // Init fake cmd buffers on client buffer
    int cmdBufArraySize = m_isRealTile ? m_hevcPicParams->num_tile_columns_minus1 + 1 : 1;

    cmdBufArray = (MOS_COMMAND_BUFFER*)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER) * cmdBufArraySize);
    CODECHAL_DECODE_CHK_NULL_RETURN(cmdBufArray);

    for (int i = 0; i < cmdBufArraySize; i++)
    {
        cmdBufArray[i].pCmdBase = (uint32_t*)(cmdResBase + cmdBufSize * i);
        cmdBufArray[i].pCmdPtr = cmdBufArray[i].pCmdBase;
        cmdBufArray[i].iRemaining = cmdBufSize;
    }

    bool                    isFirstSliceOfTile = true;
    HEVC_TILE_SLICE_PARAMS  tileSliceParams = {0,};

    for (uint32_t i = 0; i < m_numSlices; i++)
    {
        uint16_t numTiles = 0;
        uint16_t tileX = 0, tileY = 0;
        uint16_t origCtbX = 0, origCtbY = 0;

        PHEVC_SLICE_TILE_PARAMS     sliceTileParams = nullptr;

        bool isIndependentSlice = (i == 0) || !slc->LongSliceFlags.fields.dependent_slice_segment_flag;
        if (isIndependentSlice)
        {
            origCtbX = slc->slice_segment_address % m_widthInCtb;
            origCtbY = slc->slice_segment_address / m_widthInCtb;
        }
        else //dependent
        {
            int index;
            for (index = i - 1; index >= 0; --index) //search backword
            {
                if(!(slcBase + index)->LongSliceFlags.fields.dependent_slice_segment_flag) //independent slice
                {
                    origCtbX = (slcBase + index)->slice_segment_address % m_widthInCtb;
                    origCtbY = (slcBase + index)->slice_segment_address / m_widthInCtb;
                    break;
                }
            }
        }

        if (m_isSeparateTileDecoding || m_isRealTile)
        {
            tileX = GetSliceTileX(slc);
            tileY = GetSliceTileY(slc);

            if (i == m_numSlices || slc->LongSliceFlags.fields.LastSliceOfPic)
            {
                numTiles = (m_hevcPicParams->num_tile_columns_minus1 + 1) * (m_hevcPicParams->num_tile_rows_minus1 + 1 - tileY) - tileX;
            }
            else
            {
                uint32_t nextTileX = GetSliceTileX(slc + 1);
                uint32_t nextTileY = GetSliceTileY(slc + 1);

                numTiles = (m_hevcPicParams->num_tile_columns_minus1 + 1) * (nextTileY - tileY) + nextTileX - tileX;
            }

            tileSliceParams.tileX = tileX;
            tileSliceParams.tileY = tileY;
            tileSliceParams.lastSliceOfTile = (numTiles > 0);

            if (numTiles > 1)
            {
                sliceTileParams = (PHEVC_SLICE_TILE_PARAMS)MOS_AllocAndZeroMemory(sizeof(HEVC_SLICE_TILE_PARAMS) 
                    + (numTiles - 1) * sizeof(HEVC_SLICE_TILE_PARAMS::PER_TILE_INFO));
                if (sliceTileParams == nullptr)
                {
                    MOS_FreeMemory(sliceTileParams);
                    MOS_FreeMemory(cmdBufArray);
                    CODECHAL_DECODE_CHK_NULL_RETURN(nullptr);
                }

                sliceTileParams->numTiles = numTiles;
                sliceTileParams->slc = slc;
                sliceTileParams->tileX = tileX;
                sliceTileParams->tileY = tileY;
                eStatus = (MOS_STATUS)(InitSliceTileParams(sliceTileParams));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(sliceTileParams);
                    MOS_SafeFreeMemory(cmdBufArray);
                    CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
                }
            }
        }

        uint16_t subStreamCount = (numTiles > 0) ? numTiles : 1;
        for (uint16_t j = 0; j < subStreamCount; j++)
        {
            cmdBuf = m_isRealTile ? &cmdBufArray[tileX] : &cmdBufArray[0];

            if (isFirstSliceOfTile && (m_isSeparateTileDecoding || m_isRealTile))
            {
                MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileCodingParams;

                eStatus = (MOS_STATUS)(InitTileCodingParams(tileX, tileY, &tileCodingParams));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(sliceTileParams);
                    CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
                }

                eStatus = (MOS_STATUS)(m_hcpInterface->AddHcpTileCodingCmd(cmdBuf, &tileCodingParams));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(sliceTileParams);
                    CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
                }

            }

            if (m_isSccPaletteMode && (isFirstSliceOfTile || isIndependentSlice))
            {
                eStatus = (MOS_STATUS)(m_hcpInterface->AddHcpPaletteInitializerStateCmd(
                    cmdBuf,
                    m_hevcSccPicParams));
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_SafeFreeMemory(sliceTileParams);
                    MOS_SafeFreeMemory(cmdBufArray);
                    CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
                }
            }

            MHW_VDBOX_HEVC_SLICE_STATE_G12  sliceState;

            sliceState.dwSliceIndex = i;
            sliceState.u16OrigCtbX = origCtbX;
            sliceState.u16OrigCtbY = origCtbY;

            eStatus = (MOS_STATUS)(InitSliceStateParams(&sliceState, slc, slcExt,
                &tileSliceParams, sliceTileParams, j));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(sliceTileParams);
                MOS_SafeFreeMemory(cmdBufArray);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
            }

            eStatus = (MOS_STATUS)(m_hcpInterface->AddHcpSliceStateCmd(
                cmdBuf,
                &sliceState));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(sliceTileParams);
                MOS_SafeFreeMemory(cmdBufArray);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
            }

            eStatus = (MOS_STATUS)(SendRefIdxState(cmdBuf, &sliceState));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(sliceTileParams);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
            }

            eStatus = (MOS_STATUS)(SendWeightOffset(cmdBuf, &sliceState));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(sliceTileParams);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
            }

            eStatus = (MOS_STATUS)(SendSecureDecodeState(cmdBuf, &sliceState));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(sliceTileParams);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
            }

            eStatus = (MOS_STATUS)(SendBsdObj(cmdBuf, &sliceState));
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_SafeFreeMemory(sliceTileParams);
                CODECHAL_DECODE_CHK_STATUS_RETURN(eStatus);
            }

            if (++tileX > m_hevcPicParams->num_tile_columns_minus1)
            {
                tileX = 0;  ++tileY;
            }
        }

        // Update it for next slice
        isFirstSliceOfTile = (numTiles > 0);
        if (sliceTileParams)
        {
            MOS_FreeMemory(sliceTileParams);
            sliceTileParams = nullptr;
        }

        slc++;
        if (slcExt)
            slcExt++;
    }

    for (int i = 0; i < cmdBufArraySize; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBufArray[i],
            nullptr));
    }

    if (cmdBufArray)
    {
        MOS_FreeMemory(cmdBufArray);
        cmdBufArray = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::InitTileCodingParams(
    uint32_t                              col,
    uint32_t                              row,
    MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 *hcpTileCodingParam)
{
    uint16_t    startCtbX = 0, startCtbY = 0;

    CODECHAL_DECODE_CHK_NULL_RETURN(hcpTileCodingParam);

    MOS_ZeroMemory(hcpTileCodingParam, sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12));

    uint32_t minCbSize = 1 << (m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
    uint32_t LCUSize = 1 << (m_hevcPicParams->log2_diff_max_min_luma_coding_block_size + m_hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);

    for (uint8_t i = 0; i < col; i++)
    {
        startCtbX += m_tileColWidth[i];
    }
    for (uint8_t i = 0; i < row; i++)
    {
        startCtbY += m_tileRowHeight[i];
    }

    if (col < m_hevcPicParams->num_tile_columns_minus1)
    {
        hcpTileCodingParam->TileWidthInMinCbMinus1 = (m_tileColWidth[col] << m_hevcPicParams->log2_diff_max_min_luma_coding_block_size) - 1;
    }
    else
    {
        hcpTileCodingParam->TileWidthInMinCbMinus1 = m_hevcPicParams->PicWidthInMinCbsY - ((startCtbX * LCUSize) / minCbSize) - 1;
    }

    if (row < m_hevcPicParams->num_tile_rows_minus1)
    {
        hcpTileCodingParam->TileHeightInMinCbMinus1 = (m_tileRowHeight[row] << m_hevcPicParams->log2_diff_max_min_luma_coding_block_size) - 1;
    }
    else
    {
        hcpTileCodingParam->TileHeightInMinCbMinus1 = m_hevcPicParams->PicHeightInMinCbsY - ((startCtbY * LCUSize) / minCbSize) - 1;
    }

    hcpTileCodingParam->TileStartLCUX = startCtbX;
    hcpTileCodingParam->TileStartLCUY = startCtbY;
    hcpTileCodingParam->ucNumDecodePipes = m_scalabilityState ? m_scalabilityState->ucScalablePipeNum : 1;
    hcpTileCodingParam->ucPipeIdx = m_scalabilityState ? m_scalabilityState->u8RtCurPipe : 0;
    hcpTileCodingParam->IsLastTileofColumn = (row == m_hevcPicParams->num_tile_rows_minus1);
    hcpTileCodingParam->IsLastTileofRow = (col == m_hevcPicParams->num_tile_columns_minus1);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::InitSliceTileParams(
    PHEVC_SLICE_TILE_PARAMS sliceTileParams)
{
    PCODEC_HEVC_SLICE_PARAMS    slc;
    uint16_t                    tileX, tileY;
    uint32_t*                   entryPointOffsets;
    uint32_t                    bsdOffset = 0;

    CODECHAL_DECODE_CHK_NULL_RETURN(sliceTileParams);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hevcSubsetParams);

    slc = sliceTileParams->slc;
    CODECHAL_DECODE_CHK_NULL_RETURN(slc);
    CODECHAL_DECODE_ASSERT(sliceTileParams->numTiles == slc->num_entry_point_offsets + 1);

    tileX = sliceTileParams->tileX;
    tileY = sliceTileParams->tileY;
    entryPointOffsets = &m_hevcSubsetParams->entry_point_offset_minus1[slc->EntryOffsetToSubsetArray];

    for (uint16_t i = 0; i < sliceTileParams->numTiles; i++)
    {
        sliceTileParams->TileArray[i].ctbX = GetTileCtbX(tileX);
        sliceTileParams->TileArray[i].ctbY = GetTileCtbY(tileY);
        sliceTileParams->TileArray[i].bsdOffset = bsdOffset;
        if (i == 0)
        {
            sliceTileParams->TileArray[i].bsdLength = slc->ByteOffsetToSliceData + slc->NumEmuPrevnBytesInSliceHdr;
            sliceTileParams->TileArray[i].bsdLength += entryPointOffsets[i] + 1;
        }
        else if (i == sliceTileParams->numTiles - 1)
        {
            sliceTileParams->TileArray[i].bsdLength = slc->slice_data_size - sliceTileParams->TileArray[i].bsdOffset;
        }
        else
        {
            sliceTileParams->TileArray[i].bsdLength = entryPointOffsets[i] + 1;
        }
        bsdOffset += sliceTileParams->TileArray[i].bsdLength;
        if (++tileX > m_hevcPicParams->num_tile_columns_minus1)
        {
            tileX = 0;  ++tileY;
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::InitSliceStateParams(
    PMHW_VDBOX_HEVC_SLICE_STATE_G12  sliceState,
    PCODEC_HEVC_SLICE_PARAMS         sliceParams,
    PCODEC_HEVC_EXT_SLICE_PARAMS     extSliceParams,
    PHEVC_TILE_SLICE_PARAMS          tileSliceParams,
    PHEVC_SLICE_TILE_PARAMS          sliceTileParams,
    uint16_t                         tileIndex)
{
    CODECHAL_DECODE_CHK_NULL_RETURN(sliceState);
    CODECHAL_DECODE_CHK_NULL_RETURN(sliceParams);

    sliceState->presDataBuffer = m_copyDataBufferInUse ? m_resCopyDataBuffer : m_resDataBuffer;
    sliceState->pHevcPicParams = m_hevcPicParams;
    sliceState->pHevcExtPicParam = m_hevcExtPicParams;
    sliceState->pHevcSccPicParam = m_hevcSccPicParams;
    sliceState->pRefIdxMapping = m_refIdxMapping;
    sliceState->bTileInSlice = (sliceTileParams != nullptr);
    sliceState->pHevcSliceParams = sliceParams;
    sliceState->pHevcExtSliceParams = extSliceParams;
    if (tileSliceParams)
    {
        sliceState->bLastSliceInTile       = tileSliceParams->lastSliceOfTile;
        sliceState->bLastSliceInTileColumn = (sliceState->bLastSliceInTile && (tileSliceParams->tileY == m_hevcPicParams->num_tile_rows_minus1));
    }
    if (sliceTileParams)
    {
        uint16_t tileY = sliceTileParams->tileY + (sliceTileParams->tileX + tileIndex) / (m_hevcPicParams->num_tile_columns_minus1 + 1);
        sliceState->u16SliceHeaderLength = (tileIndex == 0) ? sliceParams->ByteOffsetToSliceData : 0;
        sliceState->u16TileCtbX = sliceTileParams->TileArray[tileIndex].ctbX;
        sliceState->u16TileCtbY = sliceTileParams->TileArray[tileIndex].ctbY;
        sliceState->dwOffset = sliceTileParams->TileArray[tileIndex].bsdOffset;
        sliceState->dwLength = sliceTileParams->TileArray[tileIndex].bsdLength;
        sliceState->bLastSlice = sliceParams->LongSliceFlags.fields.LastSliceOfPic && (tileIndex == sliceTileParams->numTiles - 1);
        sliceState->bIsNotFirstTile = (tileIndex != 0);
        sliceState->bLastSliceInTile = true;
        sliceState->bLastSliceInTileColumn = (tileY == m_hevcPicParams->num_tile_rows_minus1);

        if (sliceState->bLastSlice)
        {
            sliceState->u16NextTileCtbX = 0;
            sliceState->u16NextTileCtbY = 0;
        }
        else if (tileIndex == sliceTileParams->numTiles - 1)
        {
            sliceState->u16NextTileCtbX = (sliceParams + 1)->slice_segment_address % m_widthInCtb;
            sliceState->u16NextTileCtbY = (sliceParams + 1)->slice_segment_address / m_widthInCtb;
        }
        else
        {
            sliceState->u16NextTileCtbX = sliceTileParams->TileArray[tileIndex + 1].ctbX;
            sliceState->u16NextTileCtbY = sliceTileParams->TileArray[tileIndex + 1].ctbY;
        }
    }
    else
    {
        sliceState->bLastSlice = sliceParams->LongSliceFlags.fields.LastSliceOfPic;
        sliceState->dwLength = sliceParams->slice_data_size;
        sliceState->bIsNotFirstTile = false;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::SendRefIdxState(
    PMOS_COMMAND_BUFFER             cmdBuf,
    PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState)
{
    PCODEC_HEVC_SLICE_PARAMS slc = sliceState->pHevcSliceParams;

    if (!m_hcpInterface->IsHevcISlice(slc->LongSliceFlags.fields.slice_type))
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 refIdxParams;
 
        FixSliceRefList(slc);

        refIdxParams.CurrPic = m_hevcPicParams->CurrPic;
        refIdxParams.ucNumRefForList = slc->num_ref_idx_l0_active_minus1 + 1;

        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &refIdxParams.RefPicList,
            sizeof(refIdxParams.RefPicList),
            &slc->RefPicList,
            sizeof(slc->RefPicList)));

        refIdxParams.hevcRefList = (void**)m_hevcRefList;
        refIdxParams.poc_curr_pic = m_hevcPicParams->CurrPicOrderCntVal;
        for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = m_hevcPicParams->PicOrderCntValList[i];
        }

        refIdxParams.pRefIdxMapping = sliceState->pRefIdxMapping;
        refIdxParams.RefFieldPicFlag = m_hevcPicParams->RefFieldPicFlag;
        refIdxParams.RefBottomFieldFlag = m_hevcPicParams->RefBottomFieldFlag;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
            cmdBuf,
            nullptr,
            &refIdxParams));

        if (m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type))
        {
            refIdxParams.ucList = 1;
            refIdxParams.ucNumRefForList = slc->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
                cmdBuf,
                nullptr,
                &refIdxParams));
        }
    }
    else if (MEDIA_IS_WA(m_decoder->GetOsInterface()->pfnGetWaTable(m_decoder->GetOsInterface()), WaDummyReference) && 
        !m_decoder->GetOsInterface()->bSimIsActive)
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 refIdxParams;
        MOS_ZeroMemory(&refIdxParams, sizeof(MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12));
        refIdxParams.bDummyReference = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(
            cmdBuf,
            nullptr,
            &refIdxParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::SendWeightOffset(
    PMOS_COMMAND_BUFFER             cmdBuf,
    PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState)
{
    PCODEC_HEVC_SLICE_PARAMS        slc = sliceState->pHevcSliceParams;
    PCODEC_HEVC_EXT_SLICE_PARAMS    slcExt = sliceState->pHevcExtSliceParams;

    if ((m_hevcPicParams->weighted_pred_flag &&
        m_hcpInterface->IsHevcPSlice(slc->LongSliceFlags.fields.slice_type)) ||
        (m_hevcPicParams->weighted_bipred_flag &&
            m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type)))
    {
        MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS weightOffsetParams;

        weightOffsetParams.ucList = 0;

        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[0],
            sizeof(weightOffsetParams.LumaWeights[0]),
            &slc->delta_luma_weight_l0,
            sizeof(slc->delta_luma_weight_l0)));

        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &weightOffsetParams.LumaWeights[1],
            sizeof(weightOffsetParams.LumaWeights[1]),
            &slc->delta_luma_weight_l1,
            sizeof(slc->delta_luma_weight_l1)));

        if (slcExt) //REXT
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &weightOffsetParams.LumaOffsets[0],
                sizeof(weightOffsetParams.LumaOffsets[0]),
                &slcExt->luma_offset_l0,
                sizeof(slcExt->luma_offset_l0)));

            CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &weightOffsetParams.LumaOffsets[1],
                sizeof(weightOffsetParams.LumaOffsets[1]),
                &slcExt->luma_offset_l1,
                sizeof(slcExt->luma_offset_l1)));

            CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &weightOffsetParams.ChromaOffsets[0],
                sizeof(weightOffsetParams.ChromaOffsets[0]),
                &slcExt->ChromaOffsetL0,
                sizeof(slcExt->ChromaOffsetL0)));

            CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                &weightOffsetParams.ChromaOffsets[1],
                sizeof(weightOffsetParams.ChromaOffsets[1]),
                &slcExt->ChromaOffsetL1,
                sizeof(slcExt->ChromaOffsetL1)));
        }
        else
        {
            for (int32_t i = 0; i < 15; i++)
            {
                weightOffsetParams.LumaOffsets[0][i] = (int16_t)slc->luma_offset_l0[i];
                weightOffsetParams.LumaOffsets[1][i] = (int16_t)slc->luma_offset_l1[i];

                for (int32_t j = 0; j < 2; j++)
                {
                    weightOffsetParams.ChromaOffsets[0][i][j] = (int16_t)slc->ChromaOffsetL0[i][j];
                    weightOffsetParams.ChromaOffsets[1][i][j] = (int16_t)slc->ChromaOffsetL1[i][j];
                }
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[0],
            sizeof(weightOffsetParams.ChromaWeights[0]),
            &slc->delta_chroma_weight_l0,
            sizeof(slc->delta_chroma_weight_l0)));

        CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            &weightOffsetParams.ChromaWeights[1],
            sizeof(weightOffsetParams.ChromaWeights[1]),
            &slc->delta_chroma_weight_l1,
            sizeof(slc->delta_chroma_weight_l1)));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(
            cmdBuf,
            nullptr,
            &weightOffsetParams));

        if (m_hcpInterface->IsHevcBSlice(slc->LongSliceFlags.fields.slice_type))
        {
            weightOffsetParams.ucList = 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(
                cmdBuf,
                nullptr,
                &weightOffsetParams));
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::SendSecureDecodeState(
    PMOS_COMMAND_BUFFER             cmdBuf,
    PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState)
{
    if (m_secureDecoder)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_secureDecoder->AddHcpSecureState(
            cmdBuf,
            sliceState));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeSliceLongG12::SendBsdObj(
    PMOS_COMMAND_BUFFER             cmdBuf,
    PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState)
{
    MHW_VDBOX_HCP_BSD_PARAMS    bsdParams;
    PCODEC_HEVC_SLICE_PARAMS    slc = sliceState->pHevcSliceParams;

    MOS_ZeroMemory(&bsdParams, sizeof(bsdParams));
    bsdParams.dwBsdDataLength = sliceState->dwLength;
    bsdParams.dwBsdDataStartOffset = slc->slice_data_offset + sliceState->dwOffset;

    CODECHAL_DECODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterface*>(m_hcpInterface)->AddHcpBsdObjectCmd(
        cmdBuf,
        &bsdParams));

    return MOS_STATUS_SUCCESS;
}
