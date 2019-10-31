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
//! \file     codechal_decode_hevc_long_g12.h
//! \brief    Defines HEVC slice level command processing for HEVC long format.
//! \details  Defines HEVC slice level command processing for all HEVC/SCC configuration and 
//!           generate tile/slice level commands into second level buffer as short format.
//!

#ifndef __CODECHAL_DECODER_HEVC_LONG_G12_H__
#define __CODECHAL_DECODER_HEVC_LONG_G12_H__

#include "codechal_decode_hevc_g12.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "codechal_secure_decode_interface.h"

//!
//! \class HevcDecodeSliceLongG12
//! \brief This class defines all HEVC long slice processing functions and provided a interface to
//!        HEVC decoder to generate tile/slice level commands into 2nd level batch buffer.
//!
class HevcDecodeSliceLongG12
{
    //!
    //! \struct HEVC_SLICE_TILE_PARAMS
    //! \brief  Describe tile informations in a slice
    //!
    typedef struct _HEVC_SLICE_TILE_PARAMS
    {
        PCODEC_HEVC_SLICE_PARAMS                    slc;                 //!< Pointer to slice parameters
        uint16_t                                    numTiles;            //!< Number of tiles in this slice
        uint16_t                                    tileX;               //!< First tile index in horizontal
        uint16_t                                    tileY;               //!< First tile index in vertical
        uint16_t                                    origCtbX;            //!< Original slice start Ctb X index
        uint16_t                                    origCtbY;            //!< Original slice start Ctb Y index
        struct PER_TILE_INFO
        {
            uint16_t                                    ctbX;            //!< Tile horizontal offset in ctb
            uint16_t                                    ctbY;            //!< Tile vertical offset in ctb
            uint32_t                                    bsdOffset;       //!< Tile bitstream offset in the slice segment
            uint32_t                                    bsdLength;       //!< Tile bitstream length
        } TileArray[1];
    } HEVC_SLICE_TILE_PARAMS, *PHEVC_SLICE_TILE_PARAMS;

    //!
    //! \struct HEVC_TILE_SLICE_PARAMS
    //! \brief  Describe slice informations in a tile
    //!
    typedef struct _HEVC_TILE_SLICE_PARAMS
    {
        uint16_t                                    tileX;               //!< The tile index in horizontal
        uint16_t                                    tileY;               //!< The tile index in vertical
        bool                                        lastSliceOfTile;     //!< Last slice of current tile
    } HEVC_TILE_SLICE_PARAMS, *PHEVC_TILE_SLICE_PARAMS;

public:
    //!
    //! \brief  Constructor
    //! \param    [in] decoder
    //!           Pointer to the HEVC decoder
    //! \param    [in] hcpInterface
    //!           Pointer to MHW HCP interface
    //! \param    [in] miInterface
    //!           Pointer to MHW MI interface
    //!
    HevcDecodeSliceLongG12(
        CodechalDecodeHevcG12   *decoder, 
        MhwVdboxHcpInterface    *hcpInterface,
        MhwMiInterface          *miInterface);
    //!
    //! \brief  Destructor
    //!
    ~HevcDecodeSliceLongG12() {};

    //!
    //! \brief    Interface for decoder to process long slice
    //! \details  Provide a interface to  HEVC decoder to process long slices and 
    //!           generate 2nd level command in specific buffer.
    //! \param    [in] cmdResBase
    //!           Base address to 2nd level buffer resource
    //! \param    [in] cmdBufSize
    //!           Command buffer size for each 2nd batch buffer in the resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ProcessSliceLong(uint8_t *cmdResBase, uint32_t cmdBufSize);

protected:
    //!
    //! \brief    Utility to get tile index for the 1st tile in the slice
    //!
    //! \return   uint16_t
    //!           tile index in horizontal
    //!
    uint16_t GetSliceTileX(PCODEC_HEVC_SLICE_PARAMS slc) const
    {
        uint16_t  ctbX, ctbStart = 0;

        ctbX = slc->slice_segment_address % m_widthInCtb;
        for (uint16_t i = 0; i <= m_hevcPicParams->num_tile_columns_minus1; i++)
        {
            if (ctbX >= ctbStart && ctbX < ctbStart + m_tileColWidth[i])
            {
                return i;
            }
            ctbStart += m_tileColWidth[i];
        }
        return 0;
    }

    //!
    //! \brief    Utility to get tile index for the 1st tile in the slice
    //!
    //! \return   uint16_t
    //!           tile index in vertical
    //!
    uint16_t GetSliceTileY(PCODEC_HEVC_SLICE_PARAMS slc) const
    {
        uint32_t    ctbY, ctbStart = 0;

        ctbY = slc->slice_segment_address / m_widthInCtb;
        for (uint16_t i = 0; i <= m_hevcPicParams->num_tile_rows_minus1; i++)
        {
            if (ctbY >= ctbStart && ctbY < ctbStart + m_tileRowHeight[i])
            {
                return i;
            }
            ctbStart += m_tileRowHeight[i];
        }
        return 0;
    }

    //!
    //! \brief    Utility to get LCU index for specified tile column
    //!
    //! \return   uint16_t
    //!           LCU index in horizontal
    //!
    uint16_t  GetTileCtbX(uint16_t col) const
    {
        uint16_t ctbX = 0;
        for (uint16_t i = 0; i < col; i++)
        {
            ctbX += m_tileColWidth[i];
        }
        return ctbX;
    }

    //!
    //! \brief    Utility to get LCU index for specified tile row
    //!
    //! \return   uint16_t
    //!           LCU index in vertical
    //!
    uint16_t GetTileCtbY(uint16_t row) const
    {
        uint16_t ctbY = 0;
        for (uint16_t i = 0; i < row; i++)
        {
            ctbY += m_tileRowHeight[i];
        }
        return ctbY;
    }

    //!
    //! \brief    Utility to fix referen list of HEVC slice
    //!
    void FixSliceRefList(PCODEC_HEVC_SLICE_PARAMS slc)
    {
        uint32_t m = 0, n = 0, k = 0, j = 0;
        for (m = 0; m < CODEC_MAX_NUM_REF_FRAME_HEVC; m++)
        {
            int32_t poc = m_hevcPicParams->PicOrderCntValList[m];
            for (n = m + 1; n < CODEC_MAX_NUM_REF_FRAME_HEVC; n++)
            {
                if (poc == m_hevcPicParams->PicOrderCntValList[n])
                {
                    for (k = 0; k < 2; k++)
                    {
                        for (j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
                        {
                            if (slc->RefPicList[k][j].FrameIdx == m_hevcPicParams->RefFrameList[n].FrameIdx)
                            {
                                slc->RefPicList[k][j].FrameIdx = m_hevcPicParams->RefFrameList[m].FrameIdx;
                                slc->RefPicList[k][j].PicEntry = m_hevcPicParams->RefFrameList[m].PicEntry;
                                slc->RefPicList[k][j].PicFlags = m_hevcPicParams->RefFrameList[m].PicFlags;
                            }
                        }
                    }
                }
            }
        }
    }

    //!
    //! \brief    Helper function to initialize tile coding parameters
    //!
    //! \param    [in] col
    //!           Tile index in horizontal
    //! \param    [in] row
    //!           Tile index in vertical
    //! \param    [in] hcpTileCodingParam
    //!           Pointer to tile coding parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitTileCodingParams(
        uint32_t                              col,
        uint32_t                              row,
        MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 *hcpTileCodingParam);

    //!
    //! \brief    Helper function to fill tile information of slice
    //!
    //! \param    [in] sliceTileParams
    //!           Pointer to slice tile parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitSliceTileParams(PHEVC_SLICE_TILE_PARAMS sliceTileParams);

    //!
    //! \brief    Helper function to fill MHW HEVC slice state parameters
    //!
    //! \param    [in] sliceState
    //!           Pointer to MHW HEVC slice state
    //! \param    [in] sliceParams
    //!           Pointer to HEVC slice parameters
    //! \param    [in] extSliceParams
    //!           Pointer to HEVC extended slice parameters
    //! \param    [in] tileSliceParams
    //!           Pointer to tile slice infor
    //! \param    [in] sliceTileParams
    //!           Pointer to slice tile infor
    //! \param    [in] tileIndex
    //!           Tile index in the slice
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitSliceStateParams(
        PMHW_VDBOX_HEVC_SLICE_STATE_G12  sliceState,
        PCODEC_HEVC_SLICE_PARAMS         sliceParams,
        PCODEC_HEVC_EXT_SLICE_PARAMS     extSliceParams,
        PHEVC_TILE_SLICE_PARAMS          tileSliceParams,
        PHEVC_SLICE_TILE_PARAMS          sliceTileParams,
        uint16_t                         tileIndex);

    //!
    //! \brief    Helper function to program HcpRefIdx cmds
    //!
    //! \param    [in] cmdBuf
    //!           Pointer to cmd buffer
    //! \param    [in] sliceState
    //!           Pointer to MHW HEVC slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendRefIdxState(
        PMOS_COMMAND_BUFFER             cmdBuf,
        PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState);

    //!
    //! \brief    Helper function to program HcpWeightOffset cmds
    //!
    //! \param    [in] cmdBuf
    //!           Pointer to cmd buffer
    //! \param    [in] sliceState
    //!           Pointer to MHW HEVC slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendWeightOffset(
        PMOS_COMMAND_BUFFER             cmdBuf,
        PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState);

    //!
    //! \brief    Helper function to program secure decode cmds
    //!
    //! \param    [in] cmdBuf
    //!           Pointer to cmd buffer
    //! \param    [in] sliceState
    //!           Pointer to MHW HEVC slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendSecureDecodeState(
        PMOS_COMMAND_BUFFER             cmdBuf,
        PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState);

    //!
    //! \brief    Helper function to program HcpBsdObj cmds
    //!
    //! \param    [in] cmdBuf
    //!           Pointer to cmd buffer
    //! \param    [in] sliceState
    //!           Pointer to MHW HEVC slice state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendBsdObj(
        PMOS_COMMAND_BUFFER             cmdBuf,
        PMHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState);

private:
    CodechalDecodeHevcG12       *m_decoder = nullptr;            //!< Pointer to the HEVC decoder
    MhwVdboxHcpInterfaceG12     *m_hcpInterface = nullptr;       //!< Pointer to MHW HCP interface
    MhwMiInterface              *m_miInterface = nullptr;        //!< Pointer to MHW MI interface
    CodechalSecureDecodeInterface   *m_secureDecoder = nullptr;      //!< Secure decoder pointer

    uint32_t                     m_numSlices = 0;                 //!< Number of slices in the frame
    PCODEC_HEVC_PIC_PARAMS       m_hevcPicParams = nullptr;       //!< Pointer to HEVC picture parameter
    PCODEC_HEVC_SLICE_PARAMS     m_hevcSliceParams = nullptr;     //!< Pointer to HEVC slice parameter
    PCODEC_HEVC_EXT_PIC_PARAMS   m_hevcExtPicParams = nullptr;    //!< Extended pic params for Rext
    PCODEC_HEVC_EXT_SLICE_PARAMS m_hevcExtSliceParams = nullptr;  //!< Extended slice params for Rext
    PCODEC_HEVC_SCC_PIC_PARAMS   m_hevcSccPicParams = nullptr;    //!< Pic params for SCC
    PCODEC_HEVC_SUBSET_PARAMS    m_hevcSubsetParams = nullptr;    //!< Hevc subset params for tile entrypoint offset
   
    PCODECHAL_DECODE_SCALABILITY_STATE_G12  m_scalabilityState = nullptr;   //!< Scalability state pointer

    uint16_t       *m_tileColWidth = nullptr;           //!< Pointer to table of tile column width
    uint16_t       *m_tileRowHeight = nullptr;          //!< Pointer to table of tile row height

    uint16_t        m_widthInCtb = 0;                   //!< Frame width in LCU

    bool            m_copyDataBufferInUse = false;      //!< Indicate if BSD is from copied
    MOS_RESOURCE   *m_resCopyDataBuffer = nullptr;      //!< Poiner to the copied data buffer
    MOS_RESOURCE   *m_resDataBuffer = nullptr;          //!< Pointer to the original BSD buffer

    int8_t             *m_refIdxMapping = nullptr;      //!< Pointer to RefIdx mapping table
    PCODEC_REF_LIST    *m_hevcRefList = nullptr;        //!< Pointer to RefList

    bool            m_isRealTile = false;               //!< Flag to indicate if real tile decoding mode in use
    bool            m_isSeparateTileDecoding = false;   //!< Flag to indicate if SCC seperate tile decoding in use
    bool            m_isSccPaletteMode = false;         //!< Flag to indicate if SCC palette mode decoding in use
    bool            m_tileDecoding = false;             //!< Flag to indicate if tile decoding mode in use
};
#endif  // __CODECHAL_DECODER_HEVC_LONG_G12_H__
