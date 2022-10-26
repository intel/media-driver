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
//! \file     mhw_blt_xe_lpm_plus_base_next_impl.h
//! \brief    MHW blt interface common base for Xe_LPM_PLUS
//! \details
//!

#ifndef __MHW_BLT_XE_LPM_PLUS_BASE_NEXT_IMPL_H__
#define __MHW_BLT_XE_LPM_PLUS_BASE_NEXT_IMPL_H__

#include "mhw_blt_impl.h"
#include "mhw_blt_hwcmd_xe_lpm_plus_next.h"
#include "mhw_blt_itf.h"
#include "mhw_impl.h"
#include "mos_solo_generic.h"

namespace mhw
{
namespace blt
{
namespace xe_lpm_plus_next
{
class Impl : public blt::Impl<mhw::blt::xe_lpm_plus_next::Cmd>
{
public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;
    }

    uint32_t GetFastTilingMode(BLT_TILE_TYPE TileType) override
    {
        MHW_FUNCTION_ENTER;

        switch (TileType)
        {
        case BLT_NOT_TILED:
            return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
        case BLT_TILED_Y:
        case BLT_TILED_4:
            return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_TILE_Y;
        case BLT_TILED_64:
            return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_64KBTILING;
        default:
            MHW_ASSERTMESSAGE("BLT: Can't support GMM TileType %d.", TileType);
        }
        return Cmd::XY_FAST_COPY_BLT_CMD::DESTINATION_TILING_METHOD_LINEAR_TILINGDISABLED;
    }

    _MHW_SETCMD_OVERRIDE_DECL(XY_BLOCK_COPY_BLT)
    {
        MHW_FUNCTION_ENTER;

        _MHW_SETCMD_CALLBASE(XY_BLOCK_COPY_BLT);

        MHW_RESOURCE_PARAMS                    ResourceParams;

        MHW_CHK_NULL_RETURN(this->m_currentCmdBuf);
        MHW_CHK_NULL_RETURN(this->m_osItf);

        cmd.DW0.ColorDepth       = params.dwColorDepth;
        cmd.DW1.DestinationPitch = params.dwDstPitch - 1;
        cmd.DW1.DestinationMocsValue =
            this->m_osItf->pfnGetGmmClientContext(this->m_osItf)->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_BLT_DESTINATION).DwordValue;

        cmd.DW1.DestinationTiling = (params.pDstOsResource->TileType == MOS_TILE_LINEAR) ? 0 : 1;
        cmd.DW8.SourceTiling      = (params.pSrcOsResource->TileType == MOS_TILE_LINEAR) ? 0 : 1;
        cmd.DW8.SourceMocs =
            this->m_osItf->pfnGetGmmClientContext(this->m_osItf)->CachePolicyGetMemoryObject(nullptr, GMM_RESOURCE_USAGE_BLT_SOURCE).DwordValue;

        cmd.DW2.DestinationX1CoordinateLeft   = 0;
        cmd.DW2.DestinationY1CoordinateTop    = 0;
        cmd.DW3.DestinationX2CoordinateRight  = params.dwDstRight;
        cmd.DW3.DestinationY2CoordinateBottom = params.dwDstBottom;
        cmd.DW7.SourceX1CoordinateLeft        = params.dwSrcLeft;
        cmd.DW7.SourceY1CoordinateTop         = params.dwSrcTop;
        cmd.DW8.SourcePitch                   = params.dwSrcPitch - 1;

        // add source address
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.dwLsbNum        = 12;
        ResourceParams.dwOffset        = params.dwSrcOffset;
        ResourceParams.presResource    = params.pSrcOsResource;
        ResourceParams.pdwCmd          = &(cmd.DW9_10.Value[0]);
        ResourceParams.dwLocationInCmd = 9;
        ResourceParams.bIsWritable     = true;

        MHW_CHK_STATUS_RETURN(AddResourceToCmd(
            this->m_osItf,
            this->m_currentCmdBuf,
            &ResourceParams));

        // add destination address
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.dwLsbNum        = 12;
        ResourceParams.dwOffset        = params.dwDstOffset;
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

protected:
    using base_t = blt::Impl<mhw::blt::xe_lpm_plus_next::Cmd>;
MEDIA_CLASS_DEFINE_END(mhw__blt__xe_lpm_plus_next__Impl)
};  // Impl
}  // xe_lpm_plus_next
}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_IMPL_H__
