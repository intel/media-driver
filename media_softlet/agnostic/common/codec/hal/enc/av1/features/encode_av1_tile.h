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
//! \file     encode_av1_tile.h
//! \brief    Defines the common interface for av1 tile
//!
#ifndef __ENCODE_AV1_TILE_H__
#define __ENCODE_AV1_TILE_H__
#include "encode_tile.h"
#include "codec_def_encode_av1.h"
#include "encode_back_annotation_packet.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_hwcmd.h"

namespace encode
{
struct Av1TileStatusInfo
{
    uint32_t uiAv1PakStatistics = 0;
    uint32_t uiVdencStatistics = 0;
};

struct Av1TileGroupHeaderInfo
{
    uint8_t     *pBuffer    = nullptr;
    uint32_t    bitOffset   = 0;
};
struct Av1ReportTileGroupParams
{
    uint8_t  TileGroupStart;
    uint8_t  TileGroupEnd;
    uint32_t TileGroupOBUSizeInBytes;
    uint32_t TileGroupNum;
    uint32_t FirstTileGroupByteOffset;
};

//!
//! \brief VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT
//! \details
//!
//!
struct VdencStatistics
{
    union
    {
        //!< DWORD 0
        struct
        {
            uint32_t SumSadHaarForBestModeDecision : __CODEGEN_BITFIELD(0, 31);  //!< Sum sad\haar for best mode decision
        };
        uint32_t Value;
    } DW0;

    union
    {
        //!< DWORD 1
        struct
        {
            uint32_t IntraCuCountNormalized : __CODEGEN_BITFIELD(0, 19);   //!< Intra CU count normalized
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
    } DW1;

    union
    {
        //!< DWORD 2
        struct
        {
            uint32_t NonSkipInterCuCountNormalized : __CODEGEN_BITFIELD(0, 19);   //!< Non-skip Inter CU count normalized
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
    } DW2;

    union
    {
        //!< DWORD 3
        struct
        {
            uint32_t SegmentMapCount0 : __CODEGEN_BITFIELD(0, 19);   //!< segment map count 0
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW3;

    union
    {
        //!< DWORD 4
        struct
        {
            uint32_t SegmentMapCount1 : __CODEGEN_BITFIELD(0, 19);   //!< segment map count 1
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW4;

    union
    {
        //!< DWORD 5
        struct
        {
            uint32_t SegmentMapCount2 : __CODEGEN_BITFIELD(0, 19);   //!< segment map count 2
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW5;

    union
    {
        //!< DWORD 6
        struct
        {
            uint32_t SegmentMapCount3 : __CODEGEN_BITFIELD(0, 19);   //!< segment map count 3
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW6;

    union
    {
        //!< DWORD 7
        struct
        {
            uint32_t xGlobalMeSample0 : __CODEGEN_BITFIELD(0, 15);   //!< MV.x Global ME sample 0 (.25x,.25x)
            uint32_t yGlobalMeSample0 : __CODEGEN_BITFIELD(16, 31);  //!< MV.y Global ME sample 0 (.25x,.25x)
        };
        uint32_t Value;
    } DW7;

    union
    {
        //!< DWORD 8
        struct
        {
            uint32_t xyGlobalMeSample1 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 1 (.25x,.25x)
        };
        uint32_t Value;
    } DW8;

    union
    {
        //!< DWORD 9
        struct
        {
            uint32_t xyGlobalMeSample2 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 2 (.25x,.25x)
        };
        uint32_t Value;
    } DW9;

    union
    {
        //!< DWORD 10
        struct
        {
            uint32_t xyGlobalMeSample3 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 3 (.25x,.25x)
        };
        uint32_t Value;
    } DW10;

    union
    {
        //!< DWORD 11
        struct
        {
            uint32_t xyGlobalMeSample4 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 4 (.25x,.25x)
        };
        uint32_t Value;
    } DW11;

    union
    {
        //!< DWORD 12
        struct
        {
            uint32_t xyGlobalMeSample5 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 5 (.25x,.25x)
        };
        uint32_t Value;
    } DW12;

    union
    {
        //!< DWORD 13
        struct
        {
            uint32_t xyGlobalMeSample6 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 6 (.25x,.25x)
        };
        uint32_t Value;
    } DW13;

    union
    {
        //!< DWORD 14
        struct
        {
            uint32_t xyGlobalMeSample7 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 7 (.25x,.25x)
        };
        uint32_t Value;
    } DW14;

    union
    {
        //!< DWORD 15
        struct
        {
            uint32_t xyGlobalMeSample8 : __CODEGEN_BITFIELD(0, 31);  //!< MV.xy Global ME sample 8 (.25x,.25x)
        };
        uint32_t Value;
    } DW15;

    union
    {
        //!< DWORD 16
        struct
        {
            uint32_t RefIdForGlobalMeSample0 : __CODEGEN_BITFIELD(0, 1);    //!< RefID corresponding to of Global ME sample 0
            uint32_t RefIdForGlobalMeSample1_8 : __CODEGEN_BITFIELD(2, 17);   //!< RefID corresponding to of Global ME sample 0
            uint32_t Reserved : __CODEGEN_BITFIELD(18, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW16;

    union
    {
        //!< DWORD 17
        struct
        {
            uint32_t PaletteCuCountNormalized : __CODEGEN_BITFIELD(0, 19);   //!< Palette CU Count Normalized
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW17;

    union
    {
        //!< DWORD 18
        struct
        {
            uint32_t IbcCuCountNormalized : __CODEGEN_BITFIELD(0, 19);   //!< IBC CU Count Normalized
            uint32_t Reserved : __CODEGEN_BITFIELD(20, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW18;

    union
    {
        //!< DWORD 19
        struct
        {
            uint32_t NumberSecondaryColorsChannel1 : __CODEGEN_BITFIELD(0, 15);   //!< Number of secondary colors (Channel1)
            uint32_t NumberSecondaryColorsChannel0 : __CODEGEN_BITFIELD(16, 31);  //!< Number of secondary colors (Channel0)
        };
        uint32_t Value;
    } DW19;

    union
    {
        //!< DWORD 20
        struct
        {
            uint32_t NumberSecondaryColorsChannel2 : __CODEGEN_BITFIELD(0, 15);   //!< Number of secondary colors (Channel2)
            uint32_t Reserved : __CODEGEN_BITFIELD(16, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW20;

    union
    {
        //!< DWORD 21
        struct
        {
            uint32_t Reserved : __CODEGEN_BITFIELD(0, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW21;

    union
    {
        //!< DWORD 22
        struct
        {
            uint32_t PositionTimerExpiration : __CODEGEN_BITFIELD(0, 15);   //!< Position of Timer expiration
            uint32_t TimerExpireStatus : __CODEGEN_BITFIELD(16, 16);  //!< Timer Expire status
            uint32_t Reserved : __CODEGEN_BITFIELD(17, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW22;

    union
    {
        //!< DWORD 23
        struct
        {
            uint32_t LocationPanic : __CODEGEN_BITFIELD(0, 15);   //!< Location of panic
            uint32_t PanicDetected : __CODEGEN_BITFIELD(16, 16);  //!< Panic detected
            uint32_t Reserved : __CODEGEN_BITFIELD(17, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW23;

    union
    {
        //!< DWORD 24 - 28
        uint32_t Value[5];
    } DW24_28;

    union
    {
        //!< DWORD 29
        struct
        {
            uint32_t SumSadHaarForBestModeDecisionBottomHalfPopulation : __CODEGEN_BITFIELD(0, 31);   //!< Sum sad\haar for best mode decision bottom half population
        };
        uint32_t Value;
    } DW29;

    union
    {
        //!< DWORD 30
        struct
        {
            uint32_t SumSadHaarForBestModeDecisionTopHalfPopulation : __CODEGEN_BITFIELD(0, 31);   //!< Sum sad\haar for best mode decision top half population
        };
        uint32_t Value;
    } DW30;

    union
    {
        //!< DWORD 31
        struct
        {
            uint32_t SumTopHalfPopulationOccurrences : __CODEGEN_BITFIELD(0, 15);   //!< Sum top half population occurrences
            uint32_t SumBottomHalfPopulationOccurrences : __CODEGEN_BITFIELD(16, 31);  //!< Sum bottom half population occurrences
        };
        uint32_t Value;
    } DW31;

    union
    {
        //!< DWORD 32
        struct
        {
            uint32_t ReadRequestBank0 : __CODEGEN_BITFIELD(0, 23);   //!< Read Request Bank 0
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW32;

    union
    {
        //!< DWORD 33
        struct
        {
            uint32_t CacheMissCountBank0 : __CODEGEN_BITFIELD(0, 23);   //!< Cache Miss count Bank 0
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW33;

    union
    {
        //!< DWORD 34
        struct
        {
            uint32_t ReadRequestBank1 : __CODEGEN_BITFIELD(0, 23);   //!< Read Request Bank 1
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW34;

    union
    {
        //!< DWORD 35
        struct
        {
            uint32_t CacheMissCountBank1 : __CODEGEN_BITFIELD(0, 23);   //!< Cache Miss count Bank 1
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW35;

    union
    {
        //!< DWORD 36
        struct
        {
            uint32_t ReadRequestBank2 : __CODEGEN_BITFIELD(0, 23);   //!< Read Request Bank 2
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW36;

    union
    {
        //!< DWORD 37
        struct
        {
            uint32_t CacheMissCountBank2 : __CODEGEN_BITFIELD(0, 23);   //!< Cache Miss count Bank 2
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW37;

    union
    {
        //!< DWORD 38
        struct
        {
            uint32_t ReadRequestBank3 : __CODEGEN_BITFIELD(0, 23);   //!< Read Request Bank 3
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW38;

    union
    {
        //!< DWORD 39
        struct
        {
            uint32_t CacheMissCountBank3 : __CODEGEN_BITFIELD(0, 23);   //!< Cache Miss count Bank 3
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW39;

    union
    {
        //!< DWORD 40
        struct
        {
            uint32_t ReadRequestBank4 : __CODEGEN_BITFIELD(0, 23);   //!< Read Request Bank 4
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW40;

    union
    {
        //!< DWORD 41
        struct
        {
            uint32_t CacheMissCountBank4 : __CODEGEN_BITFIELD(0, 23);   //!< Cache Miss count Bank 4
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW41;

    union
    {
        //!< DWORD 42
        struct
        {
            uint32_t HimeRequestCount : __CODEGEN_BITFIELD(0, 23);   //!< HIME request count
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW42;

    union
    {
        //!< DWORD 43
        struct
        {
            uint32_t HimeRequestArbitraionLostCycleCount : __CODEGEN_BITFIELD(0, 23);   //!< HIME request arbitraion lost cycle count
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW43;

    union
    {
        //!< DWORD 44
        struct
        {
            uint32_t HimeRequestStallCount : __CODEGEN_BITFIELD(0, 23);   //!< HIME Request Stall count
            uint32_t Reserved : __CODEGEN_BITFIELD(24, 31);  //!< Reserved
        };
        uint32_t Value;
    } DW44;

    union
    {
        //!< DWORD 45 - 46
        uint32_t Value[2];
    } DW45_46;

    union
    {
        //!< DWORD 47
        struct
        {
            uint32_t TotalReferenceReadCount : __CODEGEN_BITFIELD(0, 31);  //!< Total Reference Read count
        };
        uint32_t Value;
    } DW47;

    union
    {
        //!< DWORD 48 - 303
        uint32_t Value[256];
    } DW48_303;

    //! \brief Explicit member initialization function
    VdencStatistics();

    static const size_t dwSize   = 304;
    static const size_t byteSize = 1216;
};

struct Av1TileInfo
{
    uint16_t tileId      = 0;
    uint16_t tgTileNum   = 0;
    uint16_t tileGroupId = 0;

    uint16_t tileColPositionInSb = 0;
    uint16_t tileRowPositionInSb = 0;

    uint16_t tileWidthInSbMinus1  = 0;
    uint16_t tileHeightInSbMinus1 = 0;

    uint32_t tileStartXInLCU = 0;
    uint32_t tileStartYInLCU = 0;
    uint32_t tileEndXInLCU   = 0;
    uint32_t tileEndYInLCU   = 0;

    bool firstTileInAFrame    = false;
    bool lastTileOfColumn     = false;
    bool lastTileOfRow        = false;
    bool firstTileOfTileGroup = false;
    bool lastTileOfTileGroup  = false;
    bool lastTileOfFrame      = false;
};

class Av1EncodeTile : public EncodeTile, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    Av1EncodeTile(MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings);

    virtual ~Av1EncodeTile();

    //!
    //! \brief  Init encode parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update encode parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MOS_STATUS IsFirstTileInGroup(bool &firstTileInGroup, uint32_t &tileGroupIdx) const;
    MOS_STATUS SetHucCtrlBuffer(VdencAv1HucCtrlBigData& hucCtrlBuffer);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(AVP_TILE_CODING);

    MHW_SETPAR_DECL_HDR(AVP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PAK_INSERT_OBJECT);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MOS_STATUS GetTileStatsOffset(uint32_t &offset);

    MOS_STATUS GetTileStatusInfo(Av1TileStatusInfo &av1TileStatsOffset, Av1TileStatusInfo &av1StatsSize);

    MOS_STATUS GetTileGroupInfo(PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS& tileGroupParams, uint32_t& numTilegroups)
    {
        tileGroupParams = m_av1TileGroupParams;
        numTilegroups = m_numTileGroups;
        return MOS_STATUS_SUCCESS;
    }
    void       WriteObuHeader(Av1TileGroupHeaderInfo &buf, const PCODEC_AV1_ENCODE_PICTURE_PARAMS &av1PicParam);
    void       WriteBit(Av1TileGroupHeaderInfo &buf, uint8_t bit);
    void       WriteLiteral(Av1TileGroupHeaderInfo &buf, uint64_t data, uint64_t bits);
    uint8_t    write_leb128(uint64_t in_value, uint64_t &out_encoded_value, const uint8_t fixed_output_len);
    uint16_t   CeilLog2(uint16_t x);
    uint16_t   TileLog2(uint16_t blksize, uint16_t target);
    uint16_t   PrepareTileGroupHeaderAv1(uint8_t *buffer, uint32_t index, const PCODEC_AV1_ENCODE_PICTURE_PARAMS &av1PicParam);
    MOS_STATUS TileSizeCheck(const PCODEC_AV1_ENCODE_PICTURE_PARAMS &av1PicParam);
    MOS_STATUS MakeTileGroupHeaderAv1(void *params);
    MOS_STATUS GetTileGroupReportParams(uint32_t idx, const Av1ReportTileGroupParams *&reportTileData);
    MOS_STATUS ReadObuSize(const uint8_t *ObuOffset, uint32_t &size);
    MOS_STATUS GetTileInfo(Av1TileInfo *av1TileInfo) const;
    MOS_STATUS GetDummyIdx(uint8_t &idx) { idx = m_firstDummyIdx; return MOS_STATUS_SUCCESS;}

protected:
    //!
    //! \brief  Calculate one tile's width and height
    //!
    //! \param  [in] av1PicParams
    //!         Picture params used for cal
    //! \param  [out] rowBd
    //!         Tile row boundary
    //! \param  [out] colBd
    //!         Tile column boundary
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateTilesBoundary(
        PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams,
        uint32_t *rowBd,
        uint32_t *colBd);

    //!
    //! \brief  Calculate Lcu number by tile layout
    //!
    //! \param  [in] av1PicParams
    //!         Picture params used for cal
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateNumLcuByTiles(PCODEC_AV1_ENCODE_PICTURE_PARAMS av1PicParams);

    //!
    //! \brief  Calculate one tile's width and height
    //!
    //! \param  [in] av1PicParams
    //!         Picture params used for cal
    //! \param  [in] av1SeqParams
    //!         Sequence params used for cal
    //! \param  [in] rowIndex
    //!         Tile row index
    //! \param  [in] colIndex
    //!         Tile column index
    //! \param  [in] rowBd
    //!         Tile row boundary
    //! \param  [in] colBd
    //!         Tile column boundary
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateTileWidthAndHeight(
        PCODEC_AV1_ENCODE_PICTURE_PARAMS  av1PicParams,
        uint32_t  rowIndex,
        uint32_t  colIndex,
        uint32_t *rowBd,
        uint32_t *colBd);

    //!
    //! \brief    Allocate Tile Statistics
    //!
    //! \param  [in] params
    //!         Pointer to parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateTileStatistics(void *params) override;


    virtual MOS_STATUS SetTileData(void *params) override;

    MOS_STATUS SetTileGroupReportParams();

    PCODEC_AV1_ENCODE_TILE_GROUP_PARAMS m_av1TileGroupParams = nullptr;   //!< Pointer to slice parameter

    Av1ReportTileGroupParams *m_reportTileGroupParams[EncodeBasicFeature::m_uncompressedSurfaceNum] = {};

    uint32_t              m_numTileGroups    = 0;        //!< Total number of tile groups in current frame
    uint32_t              m_numSbInPic       = 0;        //!< Total number of Sb in pic cal by each tile

    Av1TileStatusInfo m_av1TileStatsOffset  = {};   //!< Page aligned offsets used to program AVP / VDEnc pipe and HuC PAK Integration kernel input
    Av1TileStatusInfo m_av1FrameStatsOffset = {};   //!< Page aligned offsets used to program HuC PAK Integration kernel output, HuC BRC kernel input
    Av1TileStatusInfo m_av1StatsSize        = {};   //!< HEVC Statistics size

    static const uint32_t m_av1VdencStateSize = 1216;    // VDEnc Statistic: 48DWs (3CLs) of HMDC Frame Stats + 256 DWs (16CLs) of Histogram Stats = 1216 bytes
    std::vector<uint8_t>  m_tgHeaderBuf       = {};

    uint8_t               m_firstDummyIdx     = 0;

MEDIA_CLASS_DEFINE_END(encode__Av1EncodeTile)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_TILE_H__
