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
//! \file     decode_hevc_tile_coding.h
//! \brief    Defines tile coding related logic for hevc decode
//!
#ifndef __DECODE_HEVC_TILE_CODING_H__
#define __DECODE_HEVC_TILE_CODING_H__

#include "codec_def_decode_hevc.h"
#include "mhw_vdbox.h"
#include "codechal_setting.h"

namespace decode
{
class HevcBasicFeature;
class HevcPipeline;

class HevcTileCoding
{
public:
    struct SubTileInfo
    {
        uint16_t tileX;     //!< The tile index in horizontal
        uint16_t tileY;     //!< The tile index in vertical
        uint16_t ctbX;      //!< Tile horizontal offset in ctb
        uint16_t ctbY;      //!< Tile vertical offset in ctb
        uint32_t bsdOffset; //!< Tile bitstream offset in the slice segment
        uint32_t bsdLength; //!< Tile bitstream length
    };

    struct SliceTileInfo
    {
        uint16_t sliceTileX;        //!< The tile index in horizontal which contains this slice
        uint16_t sliceTileY;        //!< The tile index in vertical which contains this slice
        bool     firstSliceOfTile;  //!< First slice of the tile
        bool     lastSliceOfTile;   //!< Last slice of the tile

        uint16_t     origCtbX;      //!< Original slice start Ctb X index
        uint16_t     origCtbY;      //!< Original slice start Ctb Y index
        uint16_t     numTiles;      //!< Number of tiles in this slice
        SubTileInfo* tileArrayBuf;  //!< Sub tile buffer in this slice
        uint16_t     tileArraySize; //!< Allocation number for tileArrayBuf
    };

    //!
    //! \brief  HevcTileCoding constructor
    //!
    HevcTileCoding() {};

    //!
    //! \brief  HevcTileCoding deconstructor
    //!
    ~HevcTileCoding();

    //!
    //! \brief  Initialize Hevc tile coding
    //! \param  [in] params
    //!         Pointer to HevcBasicFeature
    //! \param  [in] codecSettings
    //!         Pointer to CodechalSetting
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(HevcBasicFeature *basicFeature, CodechalSetting *codecSettings);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Pointer to picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(const CODEC_HEVC_PIC_PARAMS & picParams);

    //!
    //! \brief  Update reference frames for slice
    //! \param  [in] picParams
    //!         Reference to picture parameters
    //! \param  [in] picParams
    //!         Pointer to slice parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateSlice(const CODEC_HEVC_PIC_PARAMS & picParams,
                           const PCODEC_HEVC_SLICE_PARAMS sliceParams);

    //!
    //! \brief  Update the tile information for slices
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateSliceTileInfo();

    //!
    //! \brief    Utility to get tile column width
    //! \return   uint16_t*
    //!           Tile column width
    //!
    const uint16_t *GetTileColWidth();

    //!
    //! \brief    Utility to get tile row height
    //! \return   uint16_t*
    //!           Tile row height
    //!
    const uint16_t *GetTileRowHeight();

    //!
    //! \brief    Utility to get slice tile info by index
    //! \return   uint16_t*
    //!           Tile row height
    //!
    const SliceTileInfo *GetSliceTileInfo(uint32_t sliceIndex);

    //!
    //! \brief    Utility to get tile index for the 1st tile in the slice
    //! \param  [in] sliceIndex
    //!         Slice index
    //! \return   uint16_t
    //!           tile index in horizontal
    //!
    uint16_t GetSliceTileX(uint32_t sliceIndex);

    //!
    //! \brief    Utility to get tile index for the 1st tile in the slice
    //! \param  [in] sliceIndex
    //!         Slice index
    //! \return   uint16_t
    //!           tile index in vertical
    //!
    uint16_t GetSliceTileY(uint32_t sliceIndex);

    //!
    //! \brief    Utility to get LCU index for specified tile column
    //!
    //! \return   uint16_t
    //!           LCU index in horizontal
    //!
    uint16_t  GetTileCtbX(uint16_t col);

    //!
    //! \brief    Utility to get LCU index for specified tile row
    //!
    //! \return   uint16_t
    //!           LCU index in vertical
    //!
    uint16_t GetTileCtbY(uint16_t row);

protected:
    //!
    //! \brief    Get all tile information
    //! \details  Get all tile information in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetAllTileInfo(const CODEC_HEVC_PIC_PARAMS & picParams, uint32_t widthInCtb, uint32_t heightInCtb);

    //!
    //! \brief    Compute tile index for the 1st tile in the slice
    //! \param  [in] picParams
    //!         Pointer to picture parameters
    //! \param  [in] slc
    //!         Pointer to slice parameters
    //! \return   uint16_t
    //!           tile index in horizontal
    //!
    uint16_t ComputeSliceTileX(const CODEC_HEVC_PIC_PARAMS & picParams, const CODEC_HEVC_SLICE_PARAMS & slc);

    //!
    //! \brief    Compute tile index for the 1st tile in the slice
    //!
    //! \return   uint16_t
    //!           tile index in vertical
    //!
    uint16_t ComputeSliceTileY(const CODEC_HEVC_PIC_PARAMS & picParams, const CODEC_HEVC_SLICE_PARAMS & slc);

    //!
    //! \brief    Allocate tile information context for slice, reuse context if exist
    //! \param  [in] sliceIndex
    //!         Index of slice
    //! \return   SliceTileInfo
    //!           Point to slice tile information context
    //!
    SliceTileInfo* AllocateSliceTileInfo(uint32_t sliceIndex);

    //!
    //! \brief  Compute tile number in this slice
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] sliceIdx
    //!         slice index
    //! \param  [in] sliceTileX
    //!         The tile index in horizontal which contains this slice
    //! \param  [in] sliceTileY
    //!         The tile index in vertical which contains this slice
    //! \param  [in] lastSlice
    //!         Flag to indicate if this slice is last slice of frame
    //! \return   uint16_t
    //!           Number of tile in this slice
    //!
    uint16_t ComputeTileNumForSlice(const CODEC_HEVC_PIC_PARAMS & picParams,
                                    uint32_t sliceIdx,
                                    uint16_t sliceTileX,
                                    uint16_t sliceTileY,
                                    bool lastSlice);

    //!
    //! \brief  Update tiles infomation in slice
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] sliceParams
    //!         slice parameters
    //! \param  [in] sliceTileInfo
    //!         Context of slice tile information
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateSubTileInfo(const CODEC_HEVC_PIC_PARAMS & picParams,
                                 const CODEC_HEVC_SLICE_PARAMS & sliceParams,
                                 SliceTileInfo &sliceTileInfo);

    MOS_STATUS RsToTsAddrConvert(const CODEC_HEVC_PIC_PARAMS &picParams, uint32_t picSizeInCtbsY);

    HevcBasicFeature *  m_basicFeature = nullptr;                   //!<  HEVC paramter
    uint16_t            m_tileColWidth[HEVC_NUM_MAX_TILE_COLUMN];   //!< Table of tile column width
    uint16_t            m_tileRowHeight[HEVC_NUM_MAX_TILE_ROW];     //!< Table of tile row height
    uint32_t            *m_pCtbAddrRsToTs = nullptr;                //!< Entry of raster scan to tile scan map
    uint32_t            m_CurRsToTsTableSize = 0;                   //!< Record of current rs to ts map table size

    std::vector<SliceTileInfo*> m_sliceTileInfoList;                //!< List of slice tile info

MEDIA_CLASS_DEFINE_END(decode__HevcTileCoding)
};

}  // namespace decode

#endif  // !__DECODE_HEVC_TILE_CODING_H__
