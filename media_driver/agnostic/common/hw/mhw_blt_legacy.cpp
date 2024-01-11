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
//! \file     mhw_blt_legacy.cpp
//! \brief    MHW interface for constructing commands for the BLT
//!
#include "mhw_blt_legacy.h"

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
    if (m_osInterface->bUsesGfxAddress)
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else  //PatchList
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
}

MOS_STATUS MhwBltInterface::AddFastCopyBlt(
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_FAST_COPY_BLT_PARAM pFastCopyBltParam,
    uint32_t                    srcOffset,
    uint32_t                    dstOffset)
{
    MHW_FUNCTION_ENTER;

    MHW_RESOURCE_PARAMS ResourceParams;
    MOS_STATUS          eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL_RETURN(m_osInterface);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pDstOsResource);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pSrcOsResource);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pDstOsResource->pGmmResInfo);
    MHW_CHK_NULL_RETURN(pFastCopyBltParam->pSrcOsResource->pGmmResInfo);

    mhw_blt_state::XY_FAST_COPY_BLT_CMD cmd;

    BLT_TILE_TYPE dstTiledMode = static_cast<BLT_TILE_TYPE>(pFastCopyBltParam->pDstOsResource->pGmmResInfo->GetTileType());
    BLT_TILE_TYPE srcTiledMode = static_cast<BLT_TILE_TYPE>(pFastCopyBltParam->pSrcOsResource->pGmmResInfo->GetTileType());

    cmd.DW0.SourceTilingMethod            = GetFastTilingMode(srcTiledMode);
    cmd.DW0.DestinationTilingMethod       = GetFastTilingMode(dstTiledMode);
    cmd.DW1.TileYTypeForSource            = (srcTiledMode == BLT_NOT_TILED) ? 0 : 1;
    cmd.DW1.TileYTypeForDestination       = (dstTiledMode == BLT_NOT_TILED) ? 0 : 1;
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
    ResourceParams.dwLsbNum        = MHW_BLT_ENGINE_STATE_BASE_ADDRESS_SHIFT;
    ResourceParams.dwOffset        = srcOffset;
    ResourceParams.presResource    = pFastCopyBltParam->pSrcOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW8_9.Value[0]);
    ResourceParams.dwLocationInCmd = 8;
    ResourceParams.bIsWritable     = false;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    // add destination address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = MHW_BLT_ENGINE_STATE_BASE_ADDRESS_SHIFT;
    ResourceParams.dwOffset        = dstOffset;
    ResourceParams.presResource    = pFastCopyBltParam->pDstOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW4_5.Value[0]);
    ResourceParams.dwLocationInCmd = 4;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwBltInterface::AddBlockCopyBlt(
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_FAST_COPY_BLT_PARAM pFastCopyBltParam, 
    uint32_t                 srcOffset,
    uint32_t                 dstOffset)
{
    MHW_FUNCTION_ENTER;

    MHW_RESOURCE_PARAMS ResourceParams;
    MOS_STATUS          eStatus;

    eStatus = MOS_STATUS_SUCCESS;

    mhw_blt_state::XY_BLOCK_COPY_BLT_CMD cmd;
    MHW_CHK_NULL(m_osInterface);
    cmd.DW0.ColorDepth = pFastCopyBltParam->dwColorDepth;
    cmd.DW1.DestinationPitch = pFastCopyBltParam->dwDstPitch -1;
    cmd.DW1.DestinationMocsValue =
        m_osInterface->pfnGetGmmClientContext(m_osInterface)->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_BLT_DESTINATION).DwordValue;

    cmd.DW1.DestinationTiling = (pFastCopyBltParam->pDstOsResource->TileType == MOS_TILE_LINEAR) ? 0:1; 
    cmd.DW8.SourceTiling = (pFastCopyBltParam->pSrcOsResource->TileType == MOS_TILE_LINEAR) ? 0:1;
    cmd.DW8.SourceMocs =
        m_osInterface->pfnGetGmmClientContext(m_osInterface)->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_BLT_SOURCE).DwordValue;

    cmd.DW2.DestinationX1CoordinateLeft = 0;
    cmd.DW2.DestinationY1CoordinateTop = 0;
    cmd.DW3.DestinationX2CoordinateRight = pFastCopyBltParam->dwDstRight;
    cmd.DW3.DestinationY2CoordinateBottom = pFastCopyBltParam->dwDstBottom;
    cmd.DW7.SourceX1CoordinateLeft = pFastCopyBltParam->dwSrcLeft;
    cmd.DW7.SourceY1CoordinateTop = pFastCopyBltParam->dwSrcTop;
    cmd.DW8.SourcePitch = pFastCopyBltParam->dwSrcPitch -1;

    // add source address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.dwOffset        = srcOffset;
    ResourceParams.presResource    = pFastCopyBltParam->pSrcOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW9_10.Value[0]);
    ResourceParams.dwLocationInCmd = 9;
    ResourceParams.bIsWritable     = false;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    // add destination address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.dwOffset        = dstOffset;
    ResourceParams.presResource    = pFastCopyBltParam->pDstOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW4_5.Value[0]);
    ResourceParams.dwLocationInCmd = 4;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        pCmdBuffer,
        &ResourceParams));

    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

uint32_t MhwBltInterface::GetFastTilingMode(BLT_TILE_TYPE TileType)
{
   switch(TileType)
   {
      case BLT_NOT_TILED:
          return mhw_blt_state::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
      case BLT_TILED_Y:
      case BLT_TILED_4:
          return mhw_blt_state::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_TILE_Y;
      case BLT_TILED_64:
          return mhw_blt_state::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_64KBTILING;
      default:
         MHW_ASSERTMESSAGE("BLT: Can't support GMM TileType %d.", TileType);
   }
   return mhw_blt_state::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
}