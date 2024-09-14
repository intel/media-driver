/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     mhw_vdbox_aqm_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_VDBOX_AQM_CMDPAR_H__
#define __MHW_VDBOX_AQM_CMDPAR_H__

#include "codec_def_common_encode.h"
#include "mhw_vdbox_cmdpar.h"

#if IGFX_AQM_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_aqm_cmdpar_ext.h"
#define __MHW_VDBOX_AQM_WRAPPER(STUFF)
#define __MHW_VDBOX_AQM_WRAPPER_EXT(STUFF) STUFF
#else
#define __MHW_VDBOX_AQM_WRAPPER(STUFF) STUFF
#define __MHW_VDBOX_AQM_WRAPPER_EXT(STUFF)
#endif  // _MEDIA_RESERVED

namespace mhw
{
namespace vdbox
{
namespace aqm
{
enum class AQM_SURFACE_FORMAT
{
    SURFACE_FORMAT_P010VARIANT = 3,
    SURFACE_FORMAT_PLANAR4208 = 4,
    SURFACE_FORMAT_P010 = 13,
};

struct _MHW_PAR_T(AQM_FRAME_START)
{
    uint32_t aqmFrameStart = 0;
};

struct _MHW_PAR_T(AQM_PIC_STATE)
{
    uint16_t frameWidthInPixelMinus1  = 0;
    uint16_t FrameHeightInPixelMinus1 = 0;
    bool     vdaqmEnable              = false;
    bool     tileBasedEngine          = false;
    uint8_t  lcuSize                  = 0;
    uint8_t  pixelbitdepth            = 0;
    uint8_t  chromasubsampling        = 0;
    uint8_t  aqmMode                  = 0;
    uint8_t  codectype                = 0;
    bool     sseEnable                = false;

    __MHW_VDBOX_AQM_WRAPPER(
        std::vector<std::function<MOS_STATUS(uint32_t *)>> extSettings);
    __MHW_VDBOX_AQM_WRAPPER_EXT(AQM_PIC_STATE_CMDPAR_EXT);
};

struct _MHW_PAR_T(AQM_SURFACE_STATE)
{
    uint32_t              pitch             = 0;
    uint32_t              uOffset           = 0;
    uint32_t              vOffset           = 0;
    uint8_t               surfaceStateId    = 0;
    MOS_MEMCOMP_STATE     mmcStateRawSurf   = MOS_MEMCOMP_DISABLED ;
    MOS_MEMCOMP_STATE     mmcStateReconSurf = MOS_MEMCOMP_DISABLED;
    uint32_t              compressionFormat = 0;
    AQM_SURFACE_FORMAT    surfaceFormat     = AQM_SURFACE_FORMAT::SURFACE_FORMAT_PLANAR4208;
};

struct _MHW_PAR_T(AQM_PIPE_BUF_ADDR_STATE)
{
    MOS_MEMCOMP_STATE mmcStateRawSurf            = MOS_MEMCOMP_DISABLED;
    MOS_MEMCOMP_STATE mmcStateReconSurf          = MOS_MEMCOMP_DISABLED;
    uint32_t          compressionFormat          = 0;
    PMOS_RESOURCE     surfaceRawBuffer           = nullptr;
    PMOS_RESOURCE     surfaceReconBuffer         = nullptr;
    PMOS_RESOURCE     AqmPipeBufAddrStatePar0[5] = {};
    uint32_t          AqmPipeBufAddrStatePar1[5] = {};
    PMOS_RESOURCE     AqmPipeBufAddrStatePar2    = nullptr;
    MOS_MEMCOMP_STATE AqmPipeBufAddrStatePar3    = {};
    PMOS_RESOURCE     AqmPipeBufAddrStatePar4[5] = {};
    MOS_MEMCOMP_STATE AqmPipeBufAddrStatePar5[5] = {};
};

struct _MHW_PAR_T(AQM_TILE_CODING)
{
    uint16_t tileId      = 0;
    uint16_t tileGroupId = 0;

    uint16_t tileColPositionInSb = 0;
    uint16_t tileRowPositionInSb = 0;

    uint16_t tileWidthInSbMinus1  = 0;  //!< Tile width minus 1 in SB unit
    uint16_t tileHeightInSbMinus1 = 0;  //!< Tile height minus 1 in SB unit

    uint16_t tileNum = 0;  //!< Tile ID in its Tile group
};

struct _MHW_PAR_T(AQM_VD_CONTROL_STATE)
{
};

struct _MHW_PAR_T(AQM_SLICE_STATE)
{
    bool     firstSuperSlice          = true;
    uint32_t tileSliceStartLcuMbX     = 0;
    uint32_t tileSliceStartLcuMbY     = 0;
    uint32_t nextTileSliceStartLcuMbX = 0;
    uint32_t nextTileSliceStartLcuMbY = 0;
};

__MHW_VDBOX_AQM_WRAPPER_EXT(AQM_CMD_CMDPAR_EXT);

}  // namespace aqm
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AQM_CMDPAR_H__
