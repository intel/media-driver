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
//! \file     mhw_blt_itf.h
//! \brief    MHW BLT interface common base
//! \details
//!

#ifndef __MHW_BLT_ITF_H__
#define __MHW_BLT_ITF_H__

#include "mhw_itf.h"
#include "mhw_blt_cmdpar.h"
#include "mhw_mi.h"

#define _BLT_CMD_DEF(DEF) \
    DEF(XY_FAST_COPY_BLT); \
    DEF(XY_BLOCK_COPY_BLT)

#define MAX_BLT_BLOCK_COPY_WIDTH 0x4000u

namespace mhw
{
namespace blt
{
class Itf
{
public:
    class ParSetting
    {
    public:
        virtual ~ParSetting() = default;
        _BLT_CMD_DEF(_MHW_SETPAR_DEF);
    };

    virtual ~Itf() = default;

    virtual uint32_t GetFastTilingMode(
            BLT_TILE_TYPE TileType) = 0;

    virtual MOS_STATUS AddBlockCopyBlt(
            PMOS_COMMAND_BUFFER      pCmdBuffer,
            PMHW_FAST_COPY_BLT_PARAM pBlockCopyBltParam, 
            uint32_t                 srcOffset,
            uint32_t                 dstOffset) = 0;
    
    _BLT_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_ITF);
MEDIA_CLASS_DEFINE_END(mhw__blt__Itf)
};
}  // namespace blt
}  // namespace mhw
#endif  // __MHW_BLT_ITF_H__
