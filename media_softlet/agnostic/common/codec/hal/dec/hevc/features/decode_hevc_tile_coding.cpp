/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_hevc_tile_coding.cpp
//! \brief    Defines the common interface for decode hevc tile coding
//!

#include "decode_hevc_tile_coding.h"
#include "decode_hevc_basic_feature.h"
#include "codec_def_common.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "decode_pipeline.h"

namespace decode
{
HevcTileCoding::~HevcTileCoding()
{
    for (auto sliceTileInfo : m_sliceTileInfoList)
    {
        MOS_DeleteArray(sliceTileInfo->tileArrayBuf);
        MOS_Delete(sliceTileInfo);
    }
    m_sliceTileInfoList.clear();
}

MOS_STATUS HevcTileCoding::Init(HevcBasicFeature *basicFeature, CodechalSetting *codecSettings)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(basicFeature);
    DECODE_CHK_NULL(codecSettings);

    m_basicFeature = basicFeature;
    m_shortFormatInUse  = codecSettings->shortFormatInUse;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcTileCoding::UpdatePicture(const CODEC_HEVC_PIC_PARAMS & picParams)
{
    DECODE_FUNC_CALL();
    if (picParams.tiles_enabled_flag == 1)
    {
        DECODE_CHK_STATUS(GetAllTileInfo(picParams, m_basicFeature->m_widthInCtb,
                                         m_basicFeature->m_heightInCtb));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcTileCoding::UpdateSlice(const CODEC_HEVC_PIC_PARAMS & picParams,
                                       const PCODEC_HEVC_SLICE_PARAMS sliceParams)
{
    DECODE_FUNC_CALL();

    if (m_shortFormatInUse)
    {
        return MOS_STATUS_SUCCESS;
    }

    for (uint32_t slcIdx = 0; slcIdx < m_basicFeature->m_numSlices; slcIdx++)
    {
        SliceTileInfo* sliceTileInfo = AllocateSliceTileInfo(slcIdx);
        DECODE_CHK_NULL(sliceTileInfo);

        if(sliceParams[slcIdx].slice_data_offset + sliceParams[slcIdx].slice_data_size > m_basicFeature->m_dataSize)
        {
            DECODE_ASSERTMESSAGE("invalid slice %d, data size overflow\n", slcIdx);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (m_basicFeature->IsIndependentSlice(slcIdx))
        {
            sliceTileInfo->origCtbX = sliceParams[slcIdx].slice_segment_address % m_basicFeature->m_widthInCtb;
            sliceTileInfo->origCtbY = sliceParams[slcIdx].slice_segment_address / m_basicFeature->m_widthInCtb;

            if(slcIdx == 0 && sliceParams[slcIdx].slice_segment_address != 0)
            {
                DECODE_ASSERTMESSAGE("invalid independent slice %d, slice_segment_address=%d\n",
                    slcIdx, sliceParams[slcIdx].slice_segment_address);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            // Dependent slice share same orig ctbx and ctby with previous slice in real tile and separate tile mode.
            for (int32_t index = slcIdx - 1; index >= 0; --index) //search backword
            {
                if(!sliceParams[index].LongSliceFlags.fields.dependent_slice_segment_flag) //independent slice
                {
                    sliceTileInfo->origCtbX = sliceParams[index].slice_segment_address % m_basicFeature->m_widthInCtb;
                    sliceTileInfo->origCtbY = sliceParams[index].slice_segment_address / m_basicFeature->m_widthInCtb;

                    if(index == 0 && sliceParams[index].slice_segment_address != 0)
                    {
                        DECODE_ASSERTMESSAGE("invalid dependent slice %d, index %d, slice_segment_address=%d\n",
                            index, sliceParams[index].slice_segment_address);
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    break;
                }
            }
        }
    }

    if (picParams.tiles_enabled_flag == 1)
    {
        for (uint32_t slcIdx = 0; slcIdx < m_basicFeature->m_numSlices; slcIdx++)
        {
            SliceTileInfo* sliceTileInfo = m_sliceTileInfoList[slcIdx];
            DECODE_CHK_NULL(sliceTileInfo);
            sliceTileInfo->sliceTileX = ComputeSliceTileX(picParams, sliceParams[slcIdx]);
            sliceTileInfo->sliceTileY = ComputeSliceTileY(picParams, sliceParams[slcIdx]);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcTileCoding::UpdateSliceTileInfo()
{
    DECODE_FUNC_CALL();

    if (m_shortFormatInUse)
    {
        return MOS_STATUS_SUCCESS;
    }

    const CODEC_HEVC_PIC_PARAMS&   picParams   = *(m_basicFeature->m_hevcPicParams);
    const PCODEC_HEVC_SLICE_PARAMS sliceParams = m_basicFeature->m_hevcSliceParams;

    DECODE_CHK_COND(m_basicFeature->m_numSlices > m_sliceTileInfoList.size(),
                    "Number of slices is exceeds the size of tile info list!");

    for (uint32_t slcIdx = 0; slcIdx < m_basicFeature->m_numSlices; slcIdx++)
    {
        SliceTileInfo* sliceTileInfo = m_sliceTileInfoList[slcIdx];
        DECODE_CHK_NULL(sliceTileInfo);

        if (slcIdx == 0)
        {
            sliceTileInfo->firstSliceOfTile = true;
        }
        else
        {
            auto tileInfo = GetSliceTileInfo(slcIdx-1);
            DECODE_CHK_NULL(tileInfo);
            sliceTileInfo->firstSliceOfTile = tileInfo->numTiles > 0;
        }

        bool lastSlice = m_basicFeature->IsLastSlice(slcIdx);
        sliceTileInfo->numTiles = ComputeTileNumForSlice(picParams, slcIdx,
                                    sliceTileInfo->sliceTileX, sliceTileInfo->sliceTileY, lastSlice);

        // Case 1: multiple tiles in single slice, this is the last slice in tile.
        // Case 2: multiple slices in single tile, these slices share same tile position which means numTilesInSlice is 0.
        sliceTileInfo->lastSliceOfTile = (sliceTileInfo->numTiles > 0);

        if (picParams.tiles_enabled_flag == 1 && sliceTileInfo->numTiles > 1)
        {
            if (sliceTileInfo->numTiles > sliceTileInfo->tileArraySize)
            {
                MOS_DeleteArray(sliceTileInfo->tileArrayBuf);
                sliceTileInfo->tileArrayBuf = MOS_NewArray(SubTileInfo, sliceTileInfo->numTiles);
                DECODE_CHK_NULL(sliceTileInfo->tileArrayBuf);
                sliceTileInfo->tileArraySize = sliceTileInfo->numTiles;
            }
            DECODE_CHK_STATUS(UpdateSubTileInfo(picParams, sliceParams[slcIdx], *sliceTileInfo));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcTileCoding::UpdateSubTileInfo(const CODEC_HEVC_PIC_PARAMS & picParams,
                                             const CODEC_HEVC_SLICE_PARAMS & sliceParams,
                                             SliceTileInfo &sliceTileInfo)
{
    DECODE_CHK_COND(sliceTileInfo.numTiles != (sliceParams.num_entry_point_offsets + 1),
        "tiles number does not equal to current num_entry_point_offsets.");

    uint32_t* entryPointOffsets = nullptr;
    if (m_basicFeature->m_hevcSubsetParams != nullptr)
    {
        auto hevcSubsetParams = m_basicFeature->m_hevcSubsetParams;
        entryPointOffsets = &hevcSubsetParams->entry_point_offset_minus1[sliceParams.EntryOffsetToSubsetArray];
    }

    uint16_t  tileX = sliceTileInfo.sliceTileX;
    uint16_t  tileY = sliceTileInfo.sliceTileY;
    uint32_t  bsdOffset = 0;

    for (uint16_t i = 0; i < sliceTileInfo.numTiles; i++)
    {
        sliceTileInfo.tileArrayBuf[i].tileX = tileX;
        sliceTileInfo.tileArrayBuf[i].tileY = tileY;
        sliceTileInfo.tileArrayBuf[i].ctbX = GetTileCtbX(tileX);
        sliceTileInfo.tileArrayBuf[i].ctbY = GetTileCtbY(tileY);
        sliceTileInfo.tileArrayBuf[i].bsdOffset = bsdOffset;

        if (i == 0)
        {
            sliceTileInfo.tileArrayBuf[i].bsdLength = sliceParams.ByteOffsetToSliceData +
                                                      sliceParams.NumEmuPrevnBytesInSliceHdr;
            sliceTileInfo.tileArrayBuf[i].bsdLength += (entryPointOffsets != nullptr) ? entryPointOffsets[i] + 1 : 1;
        }
        else if (i == sliceTileInfo.numTiles - 1)
        {
            sliceTileInfo.tileArrayBuf[i].bsdLength = sliceParams.slice_data_size -
                                                      sliceTileInfo.tileArrayBuf[i].bsdOffset;
        }
        else
        {
            sliceTileInfo.tileArrayBuf[i].bsdLength = (entryPointOffsets != nullptr) ? entryPointOffsets[i] + 1 : 1;
        }

        bsdOffset += sliceTileInfo.tileArrayBuf[i].bsdLength;

        if (++tileX > picParams.num_tile_columns_minus1)
        {
            tileX = 0;
            ++tileY;
        }
    }

    return MOS_STATUS_SUCCESS;
}

uint16_t HevcTileCoding::GetSliceTileX(uint32_t sliceIndex)
{
    if (sliceIndex >= m_sliceTileInfoList.size())
    {
        return 0;
    }
    return m_sliceTileInfoList[sliceIndex]->sliceTileX;
}

uint16_t HevcTileCoding::GetSliceTileY(uint32_t sliceIndex)
{
    if (sliceIndex >= m_sliceTileInfoList.size())
    {
        return 0;
    }
    return m_sliceTileInfoList[sliceIndex]->sliceTileY;
}

HevcTileCoding::SliceTileInfo* HevcTileCoding::AllocateSliceTileInfo(uint32_t sliceIndex)
{
    SliceTileInfo* sliceTileInfo;
    if (sliceIndex < m_sliceTileInfoList.size())
    {
        sliceTileInfo = m_sliceTileInfoList[sliceIndex];
        DECODE_ASSERT(sliceTileInfo != nullptr);

        sliceTileInfo->sliceTileX       = 0;
        sliceTileInfo->sliceTileY       = 0;
        sliceTileInfo->firstSliceOfTile = false;
        sliceTileInfo->lastSliceOfTile  = false;
        sliceTileInfo->origCtbX         = 0;
        sliceTileInfo->origCtbY         = 0;
        sliceTileInfo->numTiles         = 0;
    }
    else
    {
        sliceTileInfo = MOS_New(SliceTileInfo);
        if (sliceTileInfo != nullptr)
        {
            MOS_ZeroMemory(sliceTileInfo, sizeof(SliceTileInfo));
            m_sliceTileInfoList.push_back(sliceTileInfo);
        }
    }
    return sliceTileInfo;
}

MOS_STATUS HevcTileCoding::GetAllTileInfo(const CODEC_HEVC_PIC_PARAMS & picParams,
                                          uint32_t widthInCtb, uint32_t heightInCtb)
{
    DECODE_FUNC_CALL();

    if (picParams.uniform_spacing_flag == 1)
    {
        for (auto i = 0; i <= picParams.num_tile_columns_minus1; i++)
        {
            m_tileColWidth[i] = ((i + 1) * widthInCtb) / (picParams.num_tile_columns_minus1 + 1) -
                                (i * widthInCtb) / (picParams.num_tile_columns_minus1 + 1);
        }

        for (auto i = 0; i <= picParams.num_tile_rows_minus1; i++)
        {
            m_tileRowHeight[i] = ((i + 1) * heightInCtb) / (picParams.num_tile_rows_minus1 + 1) -
                                 (i * heightInCtb) / (picParams.num_tile_rows_minus1 + 1);
        }
    }
    else
    {
        m_tileColWidth[picParams.num_tile_columns_minus1] = widthInCtb & 0xffff;
        for (auto i = 0; i < picParams.num_tile_columns_minus1; i++)
        {
            m_tileColWidth[i] = picParams.column_width_minus1[i] + 1;
            m_tileColWidth[picParams.num_tile_columns_minus1] -= m_tileColWidth[i];
        }

        m_tileRowHeight[picParams.num_tile_rows_minus1] = heightInCtb & 0xffff;
        for (auto i = 0; i < picParams.num_tile_rows_minus1; i++)
        {
            m_tileRowHeight[i] = picParams.row_height_minus1[i] + 1;
            m_tileRowHeight[picParams.num_tile_rows_minus1] -= m_tileRowHeight[i];
        }
    }

    return MOS_STATUS_SUCCESS;
}

const uint16_t *HevcTileCoding::GetTileColWidth()
{
    return m_tileColWidth;
}

const uint16_t *HevcTileCoding::GetTileRowHeight()
{
    return m_tileRowHeight;
}

const HevcTileCoding::SliceTileInfo *HevcTileCoding::GetSliceTileInfo(uint32_t sliceIndex)
{
    if (sliceIndex >= m_sliceTileInfoList.size())
    {
        return nullptr;
    }
    return m_sliceTileInfoList[sliceIndex];
}

uint16_t HevcTileCoding::ComputeSliceTileX(const CODEC_HEVC_PIC_PARAMS & picParams,
                                           const CODEC_HEVC_SLICE_PARAMS & slc)
{
    DECODE_FUNC_CALL();

    uint16_t ctbX, ctbStart = 0;

    ctbX = slc.slice_segment_address % m_basicFeature->m_widthInCtb;
    for (uint16_t i = 0; i <= picParams.num_tile_columns_minus1; i++)
    {
        if (ctbX >= ctbStart && ctbX < ctbStart + m_tileColWidth[i])
        {
            return i;
        }
        ctbStart += m_tileColWidth[i];
    }
    return 0;
}

uint16_t HevcTileCoding::ComputeSliceTileY(const CODEC_HEVC_PIC_PARAMS & picParams,
                                           const CODEC_HEVC_SLICE_PARAMS & slc)
{
    DECODE_FUNC_CALL();

    uint32_t ctbY, ctbStart = 0;

    ctbY = slc.slice_segment_address / m_basicFeature->m_widthInCtb;
    for (uint16_t i = 0; i <= picParams.num_tile_rows_minus1; i++)
    {
        if (ctbY >= ctbStart && ctbY < ctbStart + m_tileRowHeight[i])
        {
            return i;
        }
        ctbStart += m_tileRowHeight[i];
    }
    return 0;
}

uint16_t HevcTileCoding::ComputeTileNumForSlice(const CODEC_HEVC_PIC_PARAMS & picParams,
                                                uint32_t sliceIdx,
                                                uint16_t sliceTileX,
                                                uint16_t sliceTileY,
                                                bool lastSlice)
{
    uint16_t numTiles;
    if (lastSlice)
    {
        numTiles = (picParams.num_tile_columns_minus1 + 1) *
                   (picParams.num_tile_rows_minus1 + 1 - sliceTileY) - sliceTileX;
    }
    else
    {
        uint32_t nextTileX = GetSliceTileX(sliceIdx + 1);
        uint32_t nextTileY = GetSliceTileY(sliceIdx + 1);
        numTiles = (picParams.num_tile_columns_minus1 + 1) * (nextTileY - sliceTileY) +
                   nextTileX - sliceTileX;
    }
    return numTiles;
}

uint16_t HevcTileCoding::GetTileCtbX(uint16_t col)
{
    DECODE_FUNC_CALL();

    uint16_t ctbX = 0;
    for (uint16_t i = 0; i < col; i++)
    {
        ctbX += m_tileColWidth[i];
    }
    return ctbX;
}

uint16_t HevcTileCoding::GetTileCtbY(uint16_t row)
{
    DECODE_FUNC_CALL();

    uint16_t ctbY = 0;
    for (uint16_t i = 0; i < row; i++)
    {
        ctbY += m_tileRowHeight[i];
    }
    return ctbY;
}

}
