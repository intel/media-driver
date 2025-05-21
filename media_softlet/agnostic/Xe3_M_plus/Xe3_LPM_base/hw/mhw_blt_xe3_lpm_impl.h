/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     mhw_blt_xe3_lpm_impl.h
//! \brief    MHW blt interface common base for xe3_lpm
//! \details
//!

#ifndef __MHW_BLT_XE3_LPM_IMPL_H__
#define __MHW_BLT_XE3_LPM_IMPL_H__

#include "mhw_blt_impl.h"
#include "mhw_blt_hwcmd_xe3_lpm.h"
#include "mhw_blt_itf.h"
#include "mhw_impl.h"
#include "mos_solo_generic.h"

namespace mhw
{
namespace blt
{
namespace xe3_lpm
{
class Impl : public blt::Impl<mhw::blt::xe3_lpm::Cmd>
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

protected:
    using base_t = blt::Impl<mhw::blt::xe3_lpm::Cmd>;

MEDIA_CLASS_DEFINE_END(mhw__blt__xe3_lpm__Impl)
};  // Impl
}  // xe3_Lpm
}  // namespace blt
}  // namespace mhw

#endif  // __MHW_BLT_XE3_LPM_IMPL_H__
