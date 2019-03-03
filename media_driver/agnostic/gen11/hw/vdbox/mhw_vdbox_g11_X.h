/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file      mhw_vdbox.h  
//! \brief     This modules implements HW interface layer to be used on all platforms on all operating systems/DDIs, across MHW components.  
//!
//!
//! \file     mhw_vdbox_g11_X.h
//! \brief    Defines structures used for constructing Vdbox commands on Gen11-based platforms
//!

#ifndef _MHW_VDBOX_G11_H_
#define _MHW_VDBOX_G11_H_

#include "mos_os.h"
#include "mhw_vdbox.h"

struct MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11 : public MHW_VDBOX_PIPE_MODE_SELECT_PARAMS
{
    MHW_VDBOX_HCP_PIPE_WORK_MODE      PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE   MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    virtual ~MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11(){}
};
using PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11 = MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11 *;

struct MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11 : public MHW_VDBOX_STATE_CMDSIZE_PARAMS
{
    bool                        bScalableMode = false;
};
using PMHW_VDBOX_STATE_CMDSIZE_PARAMS_G11 = MHW_VDBOX_STATE_CMDSIZE_PARAMS_G11 *;

struct MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11
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

    // Offsets for scalability 
    uint32_t        NumberOfActiveBePipes;
    uint32_t        BitstreamByteOffset;
    uint32_t        PakTileStatisticsOffset;
    uint32_t        CuLevelStreamoutOffset;
    uint32_t        SliceSizeStreamoutOffset;
    uint32_t        CuRecordOffset;
    uint32_t        SseRowstoreOffset;
    uint32_t        SaoRowstoreOffset;
    uint32_t        TileSizeStreamoutOffset;
    uint32_t        Vp9ProbabilityCounterStreamoutOffset;
    uint32_t        TileStreaminOffset;

    PMOS_RESOURCE   presHcpSyncBuffer; // this buffer is not used for either HEVC/VP9 encoder and decoder.

    //Decode specific sparameters
    uint8_t         ucNumDecodePipes;
    uint8_t         ucPipeIdx;
};
using PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 = MHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 *;

#endif
