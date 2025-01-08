/*===================== begin_copyright_notice ==================================

Copyright (c) 2022, Intel Corporation

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
//! \file     mhw_vdbox_xe3_lpm_base.h
//! \brief    Defines structures used for constructing VDBox commands on Xe3_LPM-based platforms
//!

#ifndef _MHW_VDBOX_XE3_LPM_BASE_H_
#define _MHW_VDBOX_XE3_LPM_BASE_H_

#include "mos_os.h"
#include "mhw_vdbox.h"

#define MmcEnable(x) ((x) == MOS_MEMCOMP_RC || (x) == MOS_MEMCOMP_MC)
#define MmcIsRc(x) ((x) == MOS_MEMCOMP_RC)

namespace mhw
{
namespace vdbox
{
namespace xe3_lpm_base
{
// Meta/MV, DeBlock, SAO, VDEnc, HSAO
const bool rowStoreCacheEnableHEVC[16][5] =
{
    { 1, 1, 1, 0, 1 }, { 1, 1, 1, 1, 1 }, { 1, 1, 0, 0, 0 }, { 1, 1, 0, 1, 0 },
    { 1, 1, 1, 1, 1 }, { 1, 1, 0, 0, 1 }, { 1, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1 },
    { 1, 1, 1, 0, 0 }, { 1, 0, 1, 0, 1 }, { 1, 1, 1, 1, 1 }, { 1, 1, 0, 1, 1 },
    { 1, 1, 1, 1, 1 }, { 1, 0, 1, 1, 1 }, { 1, 1, 1, 1, 1 }, { 1, 0, 1, 1, 1 }
};

const uint32_t rowStoreCacheAddrHEVC[16][5] =
{
    { 0, 256, 1280,    0, 2048 }, { 0, 256, 1280, 1824, 1792 }, { 0, 512,    0,    0,    0 }, { 0, 256,   0, 2304,    0 },
    { 0, 256, 1024,    0, 1792 }, { 0, 512,    0,    0, 2048 }, { 0, 256, 1792,    0,    0 }, { 0,   0, 512,    0, 2048 },
    { 0, 256, 1792,    0,    0 }, { 0,   0,  256,    0, 1792 }, { 0, 256, 1024, 1568, 1536 }, { 0, 512,   0, 2112, 2048 },
    { 0, 256, 1792, 2336, 2304 }, { 0,   0,  512, 1600, 1536 }, { 0, 128, 1664, 2336, 2304 }, { 0,   0, 256, 1600, 1536 }
};

// HVD, Meta/MV, DeBlock, VDEnc
const bool rowStoreCacheEnableVP9[13][4] =
{
    { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 1, 0, 1, 1 }, { 1, 1, 0, 1 },
    { 1, 1, 1, 1 }, { 0, 0, 1, 1 }, { 0, 0, 1, 0 }, { 1, 1, 0, 1 },
    { 1, 1, 1, 1 }, { 1, 1, 0, 1 }, { 1, 1, 1, 1 }, { 1, 1, 0, 1 },
    { 1, 1, 0, 1 }
};

const uint32_t rowStoreCacheAddrVP9[13][4] =
{
    { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,  64, 2368, }, { 0, 128,   0,  768, },
    { 0,  64, 384, 1536, }, { 0,   0,   0, 2304, }, { 0,   0,   0,    0, }, { 0, 128,   0,  768, },
    { 0,  64, 384, 2112, }, { 0, 128,   0,  768, }, { 0,  32, 192, 1920, }, { 0, 128,   0,  768, },
    { 0, 128,   0,  768, }
};

struct MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_XE3_LPM_BASE : public MHW_VDBOX_PIPE_MODE_SELECT_PARAMS
{
    // GEN11+ specific
    MHW_VDBOX_HCP_PIPE_WORK_MODE      pipeWorkMode    = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE   multiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;

    // GEN12 specific
    bool                        bTileBasedReplayMode            = false;
    bool                        bWirelessEncodeEnabled          = false;
    uint8_t                     ucWirelessSessionId             = 0;
    bool                        bRGBEncodingMode                = false;
    uint8_t                     ucPhaseIndicator                = 0;
    bool                        bBRCEnabled                     = false;
    bool                        bHEVCSeparateTileProgramming    = false;
    bool                        bStreamingBufferEnabled         = false;
    bool                        bIsRandomAccess                 = false;
    bool                        bLookaheadPass                  = false;
    uint8_t                     tuMinus1                        = 3;
    uint8_t                     ucQuantizationPrecision         = 1;
    uint8_t                     tuSettingsRevision              = 0;
};
using PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_XE3_LPM_BASE = MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_XE3_LPM_BASE *;

struct MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE : public MHW_VDBOX_STATE_CMDSIZE_PARAMS
{
    bool                        bScalableMode = 0;
};
using PMHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE = MHW_VDBOX_STATE_CMDSIZE_PARAMS_XE3_LPM_BASE *;

struct MHW_VDBOX_HCP_TILE_CODING_PARAMS_XE3_LPM_BASE
{
    uint32_t        numOfTilesInFrame;
    uint32_t        numOfTileColumnsInFrame;
    uint32_t        tileStartLCUX;
    uint32_t        tileStartLCUY;
    uint16_t        tileHeightInMinCbMinus1;
    uint16_t        tileWidthInMinCbMinus1;
    bool            isLastTileofColumn;
    bool            isLastTileofRow;
    uint32_t        tileRowStoreSelect;
    uint32_t        tileColumnStoreSelect;
    uint32_t        mode;
    bool            isFirstPass;
    bool            isLastPass;
    bool            bTileReplayEnable;

    // Offsets for scalability 
    uint32_t            numberOfActiveBePipes;
    uint32_t            bitstreamByteOffset;
    uint32_t            pakTileStatisticsOffset;
    uint32_t            cuLevelStreamoutOffset;
    uint32_t            sliceSizeStreamoutOffset;
    uint32_t            cuRecordOffset;
    uint32_t            sseRowstoreOffset;
    uint32_t            saoRowstoreOffset;
    uint32_t            tileSizeStreamoutOffset;
    uint32_t            vp9ProbabilityCounterStreamoutOffset;
    uint32_t            tileStreaminOffset;
    uint32_t            cumulativeCUTileOffset;
    uint32_t            tileLCUStreamOutOffset;

    PMOS_RESOURCE       presHcpSyncBuffer; // this buffer is not used for either HEVC/VP9 encoder and decoder.

    //Decode specific sparameters
    uint8_t             ucNumDecodePipes;
    uint8_t             ucPipeIdx;
};
using PMHW_VDBOX_HCP_TILE_CODING_PARAMS_XE3_LPM_BASE = MHW_VDBOX_HCP_TILE_CODING_PARAMS_XE3_LPM_BASE *;

struct MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_XE3_LPM_BASE
{
    union
    {
        struct
        {
            uint16_t       bWaitDoneHEVC            : 1;
            uint16_t       bWaitDoneVDENC           : 1;
            uint16_t       bWaitDoneMFL             : 1;
            uint16_t       bWaitDoneMFX             : 1;
            uint16_t       bWaitDoneVDCmdMsgParser  : 1;
            uint16_t       bFlushHEVC               : 1;
            uint16_t       bFlushVDENC              : 1;
            uint16_t       bFlushMFL                : 1;
            uint16_t       bFlushMFX                : 1;
            uint16_t       bWaitDoneAV1             : 1;
            uint16_t       bFlushAV1                : 1;
            uint16_t       bWaitDoneVVC             : 1;
            uint16_t       bFlushVVC                : 1;
            uint16_t                                : 3;
        };
        struct
        {
            uint16_t       value;
        };
    }Flags;
};
using PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS_XE3_LPM_BASE = MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_XE3_LPM_BASE *;
} // namespace xe3_lpm_base
} // namespace vdbox
} // namespace mhw
#endif
