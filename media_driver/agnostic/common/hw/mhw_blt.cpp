/*
* Copyright (c) 2020, Intel Corporation
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

MhwBltInterface::MhwBltInterface(PMOS_INTERFACE pOsInterface)
{
    MHW_FUNCTION_ENTER;

    pfnAddResourceToCmd = nullptr;

    if (pOsInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid OsInterface pointers provided");
        return;
    }

    m_osInterface       = pOsInterface;
    pfnAddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
}

MOS_STATUS MhwBltInterface::AddFastCopyBlt(
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_FAST_COPY_BLT_PARAM pFastCopyBltParam)
{
    MHW_FUNCTION_ENTER;

    MHW_RESOURCE_PARAMS ResourceParams;
    MOS_STATUS          eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    mhw_blt_state::XY_FAST_COPY_BLT_CMD cmd;

    cmd.DW0.SourceTilingMethod            = GetTileType(pFastCopyBltParam->pSrcOsResource);
    cmd.DW0.DestinationTilingMethod       = GetTileType(pFastCopyBltParam->pDstOsResource);
    cmd.DW1.TileYTypeForSource            = pFastCopyBltParam->pSrcOsResource->TileType == MOS_TILE_YF ? 1 : 0;
    cmd.DW1.TileYTypeForDestination       = pFastCopyBltParam->pDstOsResource->TileType == MOS_TILE_YF ? 1 : 0;
    cmd.DW1.ColorDepth                    = pFastCopyBltParam->dwColorDepth;
    cmd.DW1.DestinationPitch              = pFastCopyBltParam->dwDstPitch;
    cmd.DW2.DestinationX1CoordinateLeft   = pFastCopyBltParam->dwDstLeft;
    cmd.DW2.DestinationY1CoordinateTop    = pFastCopyBltParam->dwDstTop;
    cmd.DW3.DestinationX2CoordinateRight  = pFastCopyBltParam->dwDstRight;
    cmd.DW3.DestinationY2CoordinateBottom = pFastCopyBltParam->dwDstBottom;
    cmd.DW6.SourceX1CoordinateLeft        = pFastCopyBltParam->dwSrcLeft;
    cmd.DW6.SourceY1CoordinateTop         = pFastCopyBltParam->dwSrcTop;
    cmd.DW7.SourcePitch                   = pFastCopyBltParam->dwSrcPitch;

    // add source address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.presResource    = pFastCopyBltParam->pSrcOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW8_9.Value[0]);
    ResourceParams.dwLocationInCmd = 8;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    // add destination address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.presResource    = pFastCopyBltParam->pDstOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW4_5.Value[0]);
    ResourceParams.dwLocationInCmd = 4;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

uint32_t MhwBltInterface::GetTileType(
    PMOS_RESOURCE pOsResource)
{
    uint32_t tileType = 0;
    switch (pOsResource->TileType)
    {
        case MOS_TILE_LINEAR:
            tileType = 0;
            break;
        case MOS_TILE_X:
            tileType = 1;
            break;
        case MOS_TILE_Y:
            tileType = 2;
            break;
        case MOS_TILE_YF:
            tileType = 2;
            break;
        case MOS_TILE_YS:
            tileType = 3;
            break;
        default:
            tileType = 0;
            break;
    }

    return tileType;
}