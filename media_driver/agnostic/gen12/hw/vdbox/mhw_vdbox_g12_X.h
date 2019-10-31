/*===================== begin_copyright_notice ==================================

Copyright (c) 2017-2019, Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_g12_X.h
//! \brief    Defines structures used for constructing VDBox commands on Gen12-based platforms
//!

#ifndef _MHW_VDBOX_G12_H_
#define _MHW_VDBOX_G12_H_

#include "mos_os.h"
#include "mhw_vdbox.h"

// Meta/MV, DeBlock, SAO, VDEnc, HSAO
const bool RowStoreCacheEnableHEVC[16][5] =
{
    { 1, 1, 1, 0, 1 }, { 1, 1, 1, 1, 1 }, { 1, 1, 0, 0, 0 }, { 1, 1, 0, 1, 0 },
    { 1, 1, 1, 1, 1 }, { 1, 1, 0, 0, 1 }, { 1, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1 },
    { 1, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1 }, { 1, 1, 1, 1, 1 }, { 1, 1, 0, 1, 1 },
    { 1, 1, 1, 1, 1 }, { 1, 0, 1, 1, 1 }, { 1, 1, 1, 1, 1 }, { 1, 0, 1, 1, 1 }
};

const uint32_t RowStoreCacheAddrHEVC[16][5] =
{
    { 0, 256, 1280,    0, 2048 }, { 0, 256, 1280, 1824, 1792 }, { 0, 512,    0,    0,    0 }, { 0, 256,   0, 2304,    0 },
    { 0, 256, 1024,    0, 1792 }, { 0, 512,    0,    0, 2048 }, { 0, 256, 1792,    0,    0 }, { 0,   0, 512,    0, 2048 },
    { 0, 256, 1792,    0,    0 }, { 0,   0,  256,    0, 1792 }, { 0, 256, 1024, 1568, 1536 }, { 0, 512,   0, 2112, 2048 },
    { 0, 256, 1792, 2336, 2304 }, { 0,   0,  512, 1600, 1536 }, { 0, 128, 1664, 2336, 2304 }, { 0,   0, 256, 1600, 1536 }
};

// HVD, Meta/MV, DeBlock, VDEnc
const bool RowStoreCacheEnableVP9[13][4] =
{
    { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 1, 1, 0, 1 },
    { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 0 }, { 1, 1, 0, 1 },
    { 1, 1, 1, 1 }, { 1, 1, 0, 1 }, { 1, 1, 1, 1 }, { 1, 1, 0, 1 },
    { 1, 1, 0, 1 }
};

const uint32_t RowStoreCacheAddrVP9[13][4] =
{
    { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,  64, 2368, }, { 0, 128,   0,  768, },
    { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,   0,    0, }, { 0, 128,   0,  768, },
    { 0,  64, 384, 2112, }, { 0, 128,   0,  768, }, { 0,  32, 192, 1920, }, { 0, 128,   0,  768, },
    { 0, 128,   0,  768, }
};

struct MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 : public MHW_VDBOX_PIPE_MODE_SELECT_PARAMS
{
    // GEN11+ specific
    MHW_VDBOX_HCP_PIPE_WORK_MODE      PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE   MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;

    // GEN12 specific
    bool                        bTileBasedReplayMode = false;
    bool                        bWirelessEncodeEnabled = false;
    uint8_t                     ucWirelessSessionId = 0;
    bool                        bRGBEncodingMode = false;
    uint8_t                     ucPhaseIndicator = 0;
    bool                        bBRCEnabled = false;
    bool                        bHEVCSeparateTileProgramming = false;
    bool                        bStreamingBufferEnabled = false;
    bool                        bIsRandomAccess = false;
};
using PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 = MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 *;

struct MHW_VDBOX_AVC_IMG_PARAMS_G12 : public MHW_VDBOX_AVC_IMG_PARAMS
{
    bool                        bVDEncUltraModeEnabled = false;
    bool                        oneOnOneMapping        = false;
};
using PMHW_VDBOX_AVC_IMG_PARAMS_G12 = MHW_VDBOX_AVC_IMG_PARAMS_G12 *;

struct MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 : public MHW_VDBOX_STATE_CMDSIZE_PARAMS
{
    bool                        bScalableMode = 0;
};
using PMHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 = MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 *;

struct MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12
{
    uint32_t        NumOfTilesInFrame;
    uint32_t        NumOfTileColumnsInFrame;
    uint32_t        TileStartLCUX;
    uint32_t        TileStartLCUY;
    uint16_t        TileHeightInMinCbMinus1;
    uint16_t        TileWidthInMinCbMinus1;
    bool            IsLastTileofColumn;
    bool            IsLastTileofRow;
    uint32_t        TileRowStoreSelect;
    uint32_t        TileColumnStoreSelect;
    uint32_t        Mode;
    bool            IsFirstPass;
    bool            IsLastPass;
    bool            bTileReplayEnable;

    // Offsets for scalability 
    uint32_t            NumberOfActiveBePipes;
    uint32_t            BitstreamByteOffset;
    uint32_t            PakTileStatisticsOffset;
    uint32_t            CuLevelStreamoutOffset;
    uint32_t            SliceSizeStreamoutOffset;
    uint32_t            CuRecordOffset;
    uint32_t            SseRowstoreOffset;
    uint32_t            SaoRowstoreOffset;
    uint32_t            TileSizeStreamoutOffset;
    uint32_t            Vp9ProbabilityCounterStreamoutOffset;
    uint32_t            TileStreaminOffset;
    uint32_t            CumulativeCUTileOffset;
    uint32_t            TileLCUStreamOutOffset;

    PMOS_RESOURCE   presHcpSyncBuffer; // this buffer is not used for either HEVC/VP9 encoder and decoder.

    //Decode specific sparameters
    uint8_t                           ucNumDecodePipes;
    uint8_t                           ucPipeIdx;
};
using PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 = MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 *;

#endif
