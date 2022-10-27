/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     mhw_blt.cpp
//! \brief    MHW interface for constructing commands for the BLT
//!
#include "mhw_blt.h"

mhw_blt_state::XY_BLOCK_COPY_BLT_CMD::XY_BLOCK_COPY_BLT_CMD()
{
    DW0.Value                                        = 0x5040000a;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.ColorDepth                                   = COLOR_DEPTH_8BITCOLOR;
    //DW0.InstructionTargetOpcode                      = INSTRUCTION_TARGETOPCODE_INSTRUCTIONTARGETXYBLOCKCOPYBLT;
    //DW0.Client                                       = CLIENT_2DPROCESSOR;

    DW1.Value                                        = 0x00000000;
    //DW1.DestinationTiling                            = DESTINATION_TILING_LINEAR;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8.Value                                        = 0x00000000;
    //DW8.SourceTiling                                 = SOURCE_TILING_LINEAR;

    DW9_10.Value[0] = DW9_10.Value[1]                = 0x00000000;

    DW11.Value                                       = 0x00000000;
}

mhw_blt_state::XY_FAST_COPY_BLT_CMD::XY_FAST_COPY_BLT_CMD()
{
    DW0.Value                                        = 0x50800008;
    //DW0.DwordLength                                  = GetOpLength(dwSize);
    //DW0.DestinationTilingMethod                      = DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
    //DW0.SourceTilingMethod                           = SOURCE_TILING_METHOD_LINEAR_TILINGDISABLED;
    //DW0.InstructionTargetOpcode                      = INSTRUCTION_TARGETOPCODE_UNNAMED66;
    //DW0.Client                                       = CLIENT_2DPROCESSOR;

    DW1.Value                                        = 0x00000000;
    //DW1.ColorDepth                                   = COLOR_DEPTH_8BITCOLOR;
    //DW1.TileYTypeForDestination                      = 0;
    //DW1.TileYTypeForSource                           = 0;

    DW2.Value                                        = 0x00000000;

    DW3.Value                                        = 0x00000000;

    DW4_5.Value[0] = DW4_5.Value[1]                  = 0x00000000;

    DW6.Value                                        = 0x00000000;

    DW7.Value                                        = 0x00000000;

    DW8_9.Value[0] = DW8_9.Value[1]                  = 0x00000000;

}


mhw_blt_state::BCS_SWCTRL_CMD::BCS_SWCTRL_CMD()
{
    DW0.Value                                        = 0;
    DW0.TileYSource                                  = 0x0;
    DW0.TileYDestination                             = 0x0;
    DW0.NotInvalidateBlitterCacheonBCSFlush          = 0x0;
    DW0.ShrinkBlitterCache                           = 0x0;
    DW0.TileYSourceMask                              = 0x1;
    DW0.TileYDestinationMask                         = 0x1;
    DW0.Mask                                         = 0x0;
}

mhw_blt_state::BCS_SWCTRL_XE::BCS_SWCTRL_XE()
{
    DW0.Value                                        = 0;
    DW0.Tile4Source                                  = TILE_4_SOURCE_XMAJOR;
    DW0.Tile4Destination                             = TILE_4_DESTINATION_XMAJOR;
    DW0.SystemMemoryThrottleThreshold                = 0x40;
    DW0.Mask = 0x0;
}