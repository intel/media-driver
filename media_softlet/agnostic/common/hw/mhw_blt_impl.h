/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     mhw_blt_impl.h
//! \brief    MHW BLT interface common base
//! \details
//!

#ifndef __MHW_BLT_IMPL_H__
#define __MHW_BLT_IMPL_H__

#include "mhw_blt_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace blt
{

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _BLT_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
    }

    uint32_t GetFastTilingMode(BLT_TILE_TYPE TileType)
    {
        MHW_FUNCTION_ENTER;

        switch (TileType)
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


_MHW_SETCMD_OVERRIDE_DECL(XY_FAST_COPY_BLT)
{
    MHW_FUNCTION_ENTER;

    _MHW_SETCMD_CALLBASE(XY_FAST_COPY_BLT);

    MHW_RESOURCE_PARAMS                    ResourceParams;
    BLT_TILE_TYPE dstTiledMode = static_cast<BLT_TILE_TYPE>(params.pDstOsResource->pGmmResInfo->GetTileType());
    BLT_TILE_TYPE srcTiledMode = static_cast<BLT_TILE_TYPE>(params.pSrcOsResource->pGmmResInfo->GetTileType());

    MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);
    MHW_CHK_NULL_RETURN(this->m_osItf);

    cmd.DW0.SourceTilingMethod            = GetFastTilingMode(srcTiledMode);
    cmd.DW0.DestinationTilingMethod       = GetFastTilingMode(dstTiledMode);
    cmd.DW1.TileYTypeForSource            = (srcTiledMode == BLT_NOT_TILED) ? 0 : 1;
    cmd.DW1.TileYTypeForDestination       = (dstTiledMode == BLT_NOT_TILED) ? 0 : 1;
    cmd.DW1.ColorDepth                    = params.dwColorDepth;
    cmd.DW1.DestinationPitch              = params.dwDstPitch;
    cmd.DW2.DestinationX1CoordinateLeft   = params.dwDstLeft;
    cmd.DW2.DestinationY1CoordinateTop    = params.dwDstTop;
    cmd.DW3.DestinationX2CoordinateRight  = params.dwDstRight;
    cmd.DW3.DestinationY2CoordinateBottom = params.dwDstBottom;
    cmd.DW6.SourceX1CoordinateLeft        = params.dwSrcLeft;
    cmd.DW6.SourceY1CoordinateTop         = params.dwSrcTop;
    cmd.DW7.SourcePitch                   = params.dwSrcPitch;

    // add source address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.dwOffset        = params.dwSrcOffset;   //srcOffset;
    ResourceParams.presResource    = params.pSrcOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW8_9.Value[0]);
    ResourceParams.dwLocationInCmd = 8;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS_RETURN(AddResourceToCmd(
        this->m_osItf,
        this->m_currentCmdBuf,
        &ResourceParams));

    // add destination address
    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
    ResourceParams.dwLsbNum        = 12;
    ResourceParams.dwOffset        = params.dwDstOffset;  //dstOffset;
    ResourceParams.presResource    = params.pDstOsResource;
    ResourceParams.pdwCmd          = &(cmd.DW4_5.Value[0]);
    ResourceParams.dwLocationInCmd = 4;
    ResourceParams.bIsWritable     = true;

    MHW_CHK_STATUS_RETURN(AddResourceToCmd(
        this->m_osItf,
        this->m_currentCmdBuf,
        &ResourceParams));

    return MOS_STATUS_SUCCESS;
}

_MHW_SETCMD_OVERRIDE_DECL(XY_BLOCK_COPY_BLT)
{
    MHW_FUNCTION_ENTER;

    _MHW_SETCMD_CALLBASE(XY_BLOCK_COPY_BLT);

    return MOS_STATUS_SUCCESS;
}

protected:
    using base_t = Itf;
};
}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_IMPL_H__
