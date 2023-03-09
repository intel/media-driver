/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     decode_av1_tile_coding.h
//! \brief    Defines tile coding related logic for av1 decode
//!
#ifndef __DECODE_AV1_TILE_CODING_H__
#define __DECODE_AV1_TILE_CODING_H__

#include "codec_def_decode_av1.h"
#include "mhw_vdbox.h"
#include "codechal_setting.h"

namespace decode
{
    class Av1BasicFeature;
    class Av1Pipeline;

    class Av1DecodeTile
    {
    public:
        //!
        //! \struct TileDesc
        //! Tile Descriptor to store each tile info
        //!
        struct TileDesc
        {
            uint32_t    m_offset;           //!< byte-aligned starting address of a tile in the bitstream
            uint32_t    m_size;             //!< tile size for the current tile
            uint16_t    m_tileGroupId;      //!< tile Group ID that this tile belongs to
            uint16_t    m_tileNum;          //!< tile num in the current tile group
            bool        m_lastInGroup;      //!< last tile in this tile group
            uint16_t    m_tileRow;          //!< tile row index in source position if large scale tile is enabled
            uint16_t    m_tileColumn;       //!< tile column index in source position if large scale tile is enabled
            uint16_t    m_tileIndex;        //!< tile index in tile list, valid when large scale tile is enabled
            uint8_t     m_anchorFrameIdx;   //!< anchor frame index for this tile, valid when large scale tile is enabled
            uint16_t    m_tileIndexCount;   //!< tile index in tile list, valid when large scale tile is enabled
        };

        //multiple tiles enabling
        int16_t         m_curTile                = -1;           //!< tile ID currently decoding
        int16_t         m_lastTileId             = -1;           //!< tile ID of the last tile parsed in the current execute() call
        uint16_t        m_prevFrmTileNum         = 0;            //!< record the tile numbers in previous frame.
        uint16_t        m_firstTileInTg          = 0;            //!< tile ID of the first tile in the current tile group
        uint16_t        m_tileGroupId            = 0;            //!< record the last tile group ID
        bool            m_isTruncatedTile        = false;        //!< flag to indicate if the last tile is truncated tile
        TileDesc        *m_tileDesc              = nullptr;      //!< tile descriptors for each tile of the frame
        bool            m_hasTileMissing         = false;        //! flag to indicate if having tile missing
        bool            m_hasDuplicateTile       = false;        //! flag to indicate if having tile duplicate
        uint32_t        m_tileStartOffset        = 0;            //!< record the tile or tile group header start address offset against the first byte of the bitstream buffer
        bool            m_newFrameStart          = true;         //!< flag to indicate a new frame is coming, which means m_lastTileId should be reset to -1.
        uint32_t        m_numTiles               = 0;            //!< Num of tiles
        uint32_t        m_totalTileNum           = 0;            //!< Total tile number in the frame.
        uint16_t         m_decPassNum            = 1;            //!< Total decode number pass in the frame.
        //Used to calc tile width/height
        uint16_t        m_miCols                 = 0;            //!< frame width in MI units (4x4)
        uint16_t        m_miRows                 = 0;            //!< frame height in MI units (4x4)
        uint16_t        m_tileWidthInMi          = 0;            //!< tile width in MI_units (4x4)
        uint16_t        m_tileHeightInMi         = 0;            //!< tile height in MI units (4x4)
        uint16_t        m_tileColStartSb[64];                    //!< tile column start SB
        uint16_t        m_tileRowStartSb[64];                    //!< tile row start SB

        // Super-res x_step_qn and x0_qn
        int32_t         m_lumaXStepQn            = 0;            //!< x_step_qn for luma
        int32_t         m_lumaX0Qn[64];                          //!< x0_qn for each tile column for luma
        int32_t         m_chromaXStepQn          = 0;            //!< x_step_qn for chroma
        int32_t         m_chromaX0Qn[64];                        //!< x0_qn for each tile column for chroma

        //!
        //! \brief  Av1TileCoding constructor
        //!
        Av1DecodeTile() {};

        //!
        //! \brief  Av1TileCoding deconstructor
        //!
        ~Av1DecodeTile();

        //!
        //! \brief  Initialize Av1 tile coding
        //! \param  [in] params
        //!         Pointer to Av1BasicFeature
        //! \param  [in] codecSettings
        //!         Pointer to CodechalSetting
        //! \return  MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init(Av1BasicFeature *basicFeature, CodechalSetting *codecSettings);

        //!
        //! \brief  Update reference frames for tile
        //! \param  [in] picParams
        //!         Reference to picture parameters
        //! \param  [in] picParams
        //!         Pointer to tile parameters
        //! \return  MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Update(CodecAv1PicParams & picParams,
                               CodecAv1TileParams *tileParams);

        //!
        //! \brief  Detect conformance conflict and do error concealment
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ErrorDetectAndConceal();

        //!
        //! \brief    Calculate decode pass number
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalcNumPass(const CodecAv1PicParams &picParams, CodecAv1TileParams *tileParams);
        //!
        //! \brief    Calculate upscaled Convolve Step and offset
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        void GetUpscaleConvolveStepX0(const CodecAv1PicParams &picParams, bool isChroma);

        //!
        //! \brief  Get Decode pass number
        //! \return uint16_t
        uint16_t GetNumPass() { return m_decPassNum; }

    protected:
        //!
        //! \brief    Calaculate Tile info Max tile
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalcTileInfoMaxTile(CodecAv1PicParams & picParams);

        //!
        //! \brief    Calculate tile column width and tile column number
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculateTileCols(CodecAv1PicParams & picParams);

        //!
        //! \brief    Calculate tile row height and tile row number
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculateTileRows(CodecAv1PicParams & picParams);

        //!
        //! \brief    Parse tile params to get each tile info
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ParseTileInfo(const CodecAv1PicParams & picParams, CodecAv1TileParams *tileParams);

        Av1BasicFeature *m_basicFeature = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__Av1DecodeTile)
    };

}  // namespace decode

#endif  // !__DECODE_AV1_TILE_CODING_H__
