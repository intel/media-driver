/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file     renderhal.cpp
//! \brief    Render Engine state heap manager for VP and CM
//! \details  Platform/OS Independent Render Engine state heap management interfaces
//!

#include "mos_os.h"
#include "renderhal.h"
#include "hal_kerneldll.h"
#include "renderhal_platform_interface.h"
#include "media_interfaces_renderhal.h"
#include "media_interfaces_mhw.h"
#include "hal_oca_interface.h"

extern const SURFACE_STATE_TOKEN_COMMON g_cInit_SURFACE_STATE_TOKEN_COMMON =
{
    // DWORD 0
    {
        OP_LENGTH(SIZE32(SURFACE_STATE_TOKEN_COMMON)),          // Length
        GFXSUBOP_SURFACE_STATE_TOKEN,                       // InstructionSubOpcode
        GFXOP_PIPELINED,                                    // InstructionOpcode
        PIPE_3D,                                            // InstructionPipeline
        INSTRUCTION_GFX,                                    // InstructionType
        1                                                   // Token
    },

    // DWORD 1
    {
        0,                       // SurfaceStateHeapOffset
        0                        // SurfaceAllocationIndex
    },

    // DWORD 2
    {
        0                        // SurfaceOffset
    },

    // DWORD 3
    {
        0,                       // RenderTargetEnable
        0,                       // YUVPlane
        0                        // SurfaceStateType
    },

    // DWORD 4
    {
        0                        // SurfaceBaseAddress
    },

    // DWORD 5
    {
        0                       // SurfaceBaseAddress64
    },
};

const MHW_PIPE_CONTROL_PARAMS  g_cRenderHal_InitPipeControlParams =
{
    nullptr,                           // presDest
    0,                              // dwResourceOffset
    0,                              // dwDataDW1
    0,                              // dwDataDW2
    MHW_FLUSH_NONE,                 // dwFlushMode
    MHW_FLUSH_NOWRITE,              // dwPostSyncOp
    false,                          // bDisableCSStall
    false,                          // bInvalidateStateCache
    false,                          // bInvalidateConstantCache
    false,                          // bInvalidateVFECache
    false,                          // bInvalidateInstructionCache
    true,                           // bFlushRenderTargetCache
    false,                          // bTlbInvalidate
    false                           // bInvalidateTextureCache
};

extern const RENDERHAL_SURFACE_STATE_ENTRY g_cInitSurfaceStateEntry =
{
    RENDERHAL_SURFACE_TYPE_INVALID, // Type
    nullptr,                           // pSurface
    nullptr,                           // pSurfaceState
    {                               // SurfaceS Token
        {},   //DW0
        {},   //DW1
        {},   //DW2
        {},   //DW3
        {},   //DW4
        {},   //DW5
        nullptr, // pResourceInfo
    },
    -1,                             // iSurfStateID
    0,                              // dwSurfStateOffset
    0,                              // dwFormat
    0,                              // dwWidth
    0,                              // dwHeight
    0,                              // dwPitch
    0,                              // dwQPitch
    0,                              // YUVPlane
    false,                          // bAVS
    false,                          // bRenderTarget
    false,                          // bVertStride
    false,                          // bVertStrideOffs
    false,                          // bWidthInDword
    false,                          // bTiledSurface
    false,                          // bTileWalk
    false,                          // bHalfPitchChroma
    false,                          // bInterleaveChroma
    0,                              // DirectionV
    0,                              // DirectionU
    0,                              // AddressControl
    0,                              // wUXOffset
    0,                              // wUYOffset
    0,                              // wVXOffset
    0                               // wVYOffset
};

const MHW_MEDIA_STATE_FLUSH_PARAM  g_cRenderHal_InitMediaStateFlushParams =
{
    false,                              // bFlushToGo
    0                                   // ui8InterfaceDescriptorOffset
};

const RENDERHAL_KERNEL_PARAM g_cRenderHal_InitKernelParams =
{
    0,                                  // GRF_Count;
    0,                                  // BT_Count;
    0,                                  // Sampler_Count
    0,                                  // Thread_Count
    0,                                  // GRF_Start_Register
    0,                                  // CURBE_Length
    0,                                  // block_width
    0,                                  // block_height
    0,                                  // blocks_x
    0                                   // blocks_y
};

//!
//! \brief      Table only used on HSW (look @ renderhal.c for HSW- table)
//!             Constants used for setting up surface states ui8PlaneID, 
//!             ui8ScaleWidth, ui8ScaleHeight, ui8AlignWidth, ui8AlignHeight, 
//!             ui8PixelsPerDword, bAdvanced, dwFormat
//!
extern const MHW_SURFACE_PLANES g_cRenderHal_SurfacePlanes[RENDERHAL_PLANES_DEFINITION_COUNT] =
{
    // RENDERHAL_PLANES_PL3
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 2, 2, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 2, 2, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_NV12
    {   1,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8 }
        }
    },
    // RENDERHAL_PLANES_YUY2
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_NORMAL }
        }
    },
    // RENDERHAL_PLANES_UYVY
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPY }
        }
    },
    // RENDERHAL_PLANES_YVYU
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUV }
        }
    },
    // RENDERHAL_PLANES_VYUY
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_YCRCB_SWAPUVY }
        }
    },
    // RENDERHAL_PLANES_ARGB
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_B8G8R8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_XRGB
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_B8G8R8X8_UNORM }
        }
    },
    // RENDERHAL_PLANES_ABGR
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_XBGR
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8X8_UNORM }
        }
    },
    // RENDERHAL_PLANES_RGB16
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_B5G6R5_UNORM }
        }
    },
    // RENDERHAL_PLANES_RGB24
    {   1,
        {
           { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8_UNORM }
        }
    },
    // RENDERHAL_PLANES_R16U
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT }
        }
    },
    // RENDERHAL_PLANES_R16S
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_SINT }
        }
    },
    // RENDERHAL_PLANES_R32U
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R32_UINT }
        }
    },
    // RENDERHAL_PLANES_R32S
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R32_SINT }
        }
    },
    // RENDERHAL_PLANES_R32F
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R32_FLOAT }
        }
    },
    // RENDERHAL_PLANES_V8U8
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_SNORM }
        }
    },
    // RENDERHAL_PLANES_R8G8_UNORM
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM }
        }
    },
    // RENDERHAL_PLANES_411P
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 4, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 4, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_411R
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 1, 4, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 4, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_422H
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 2, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 2, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_422V
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 1, 2, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 2, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_444P
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_RGBP
    {   3,
        {
            { MHW_U_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_BGRP
    {   3,
        {
            { MHW_U_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_AI44_PALLETE_0
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_P4A4_UNORM_PALETTE_0 }
        }
    },
    // RENDERHAL_PLANES_IA44_PALLETE_0
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_A4P4_UNORM_PALETTE_0 }
        }
    },
    // RENDERHAL_PLANES_P8_PALLETE_0
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_P8_UNORM_PALETTE_0 }
        }
    },
    // RENDERHAL_PLANES_A8P8_PALLETE_0
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_P8A8_UNORM_PALETTE_0 }
        }
    },
    // RENDERHAL_PLANES_AI44_PALLETE_1
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_P4A4_UNORM_PALETTE_1 }
        }
    },
    // RENDERHAL_PLANES_IA44_PALLETE_1
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_A4P4_UNORM_PALETTE_1 }
        }
    },
    // RENDERHAL_PLANES_P8_PALLETE_1
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_P8_UNORM_PALETTE_1 }
        }
    },
    // RENDERHAL_PLANES_A8P8_PALLETE_1
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_P8A8_UNORM_PALETTE_1 }
        }
    },
    // RENDERHAL_PLANES_AYUV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8B8G8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_STMM
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM }
        }
    },
    // RENDERHAL_PLANES_L8
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM }
        }
    },
    // RENDERHAL_PLANES_PL3_ADV
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_U_PLANE      , 2, 2, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 2, 2, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_NV12_ADV
    {   1,
        {
            { MHW_Y_PLANE      , 1, 1, 2, 2, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_8 }
        }
    },
    // RENDERHAL_PLANES_YUY2_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 2, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL }
        }
    },
    // RENDERHAL_PLANES_UYVY_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 2, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPY }
        }
    },
    // RENDERHAL_PLANES_YVYU_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 2, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUV }
        }
    },
    // RENDERHAL_PLANES_VYUY_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 2, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_SWAPUVY }
        }
    },
    // RENDERHAL_PLANES_ARGB_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_B8G8R8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_ABGR_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8G8B8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_AYUV_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 1, MHW_MEDIASTATE_SURFACEFORMAT_A8Y8U8V8_UNORM }
        }
    },
    // RENDERHAL_PLANES_STMM_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_STMM_DN_STATISTICS }
        }
    },
    // RENDERHAL_PLANES_L8_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_A8_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8 }
        }
    },
    // RENDERHAL_PLANES_A8
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_R8
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_NV12_2PLANES
    {   2,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM   },
            { MHW_U_PLANE      , 2, 2, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM }
        }
    },
    // RENDERHAL_PLANES_NV12_2PLANES_ADV
    {   2,
        {
            { MHW_Y_PLANE      , 1, 1, 2, 2, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM   },
            { MHW_U_PLANE      , 2, 2, 1, 1, 2, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8B8_UNORM }
        }
    },
    // RENDERHAL_PLANES_411P_ADV
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_U_PLANE      , 4, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 4, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_411R_ADV
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_U_PLANE      , 1, 4, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 4, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_422H_ADV
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_U_PLANE      , 2, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 2, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_422V_ADV
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_U_PLANE      , 1, 2, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 2, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_444P_ADV
    {   3,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_U_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_RGBP_ADV
    {   3,
        {
            { MHW_U_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_V_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_BGRP_ADV
    {   3,
        {
            { MHW_U_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM },
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_V_PLANE      , 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_R8_UNORM }
        }
    },
    // RENDERHAL_PLANES_R16_UNORM
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM }
        }
    },
        // RENDERHAL_PLANES_Y8
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_411_8 }
        }
    },    
    // RENDERHAL_PLANES_Y1
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, 16 }
        }
    },
    // RENDERHAL_PLANES_Y16U
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 1, MHW_MEDIASTATE_SURFACEFORMAT_STMM_DN_STATISTICS }
        }
    },
    // RENDERHAL_PLANES_Y16S
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 1, MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_422_8 }
        }
    },
    // RENDERHAL_PLANES_A16B16G16R16
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UNORM } //setting "PixelsPerDword = 0" for sampler_8x8 use
        }
    },
    // RENDERHAL_PLANES_A16B16G16R16_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 1, MHW_MEDIASTATE_SURFACEFORMAT_R16G16B16A16 } //setting "PixelsPerDword = 0" for sampler_8x8 use
        }
    },
    // RENDERHAL_PLANES_R10G10B10A2
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R10G10B10A2_UNORM }
        }
    },
    // RENDERHAL_PLANES_R10G10B10A2_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 1, MHW_MEDIASTATE_SURFACEFORMAT_R10G10B10A2_UNORM }
        }
    },
    // RENDERHAL_PLANES_B10G10R10A2
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_B10G10R10A2_UNORM }
        }
    },
    // RENDERHAL_PLANES_L16
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_L16_UNORM }
        }
    },
    // RENDERHAL_PLANES_NV21
    {   2,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM   },
            { MHW_U_PLANE      , 2, 2, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM }
        }
    },
    // RENDERHAL_PLANES_YV12
    {   1,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8 }
        }
    },
          // RENDERHAL_PLANES_P016
    {   2,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM   },
            { MHW_U_PLANE      , 2, 2, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM }
        }
    },
    // RENDERHAL_PLANES_P016_2PLANES_ADV
    {   2,
        {
            { MHW_Y_PLANE      , 1, 1, 2, 2, 2, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y16_UNORM   },
            { MHW_U_PLANE      , 2, 2, 2, 2, 1, 1, MHW_MEDIASTATE_SURFACEFORMAT_R16B16_UNORM }
        }
    },
    // RENDERHAL_PLANES_P010
    {   2,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM   },
            { MHW_U_PLANE      , 2, 2, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM }
        }
    },
    // RENDERHAL_PLANES_P010_1PLANE
    {   1,
        {
            { MHW_Y_PLANE      , 1, 1, 2, 2, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_16 }
        }
    },
    // RENDERHAL_PLANES_P010_1PLANE_ADV
    {   1,
        {
            { MHW_Y_PLANE      , 1, 1, 2, 2, 2, 1, MHW_MEDIASTATE_SURFACEFORMAT_PLANAR_420_16 }
        }
    },
    // RENDERHAL_PLANES_IRW0
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM }
        }
    },
    // RENDERHAL_PLANES_IRW1
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM }
        }
    },
    // RENDERHAL_PLANES_IRW2
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM }
         }
    },
    // RENDERHAL_PLANES_IRW3
    { 1,
         {
             { MHW_GENERIC_PLANE, 1, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_UNORM }
         }
    },
    // RENDERHAL_PLANES_A16B16G16R16F
    {1,
         {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_FLOAT } //setting "PixelsPerDword = 0" in DP R/W usage, "0" means no need to adjust dwSurfaceWidth. already DWord aligned.
         }
    },
    // RENDERHAL_PLANES_R16G16_UNORM
    {1,
         {
             { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM }
        }
    },
    // RENDERHAL_PLANES_R16F
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16_FLOAT }
         }
    },
    // RENDERHAL_PLANES_A16R16G16B16F
    {1,
         {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_FLOAT } //setting "PixelsPerDword = 0" in DP R/W usage, "0" means no need to adjust dwSurfaceWidth. already DWord aligned.
         }
    },
    // RENDERHAL_PLANES_YUY2_2PLANES
    { 2,
        {
            { MHW_Y_PLANE, 1, 1, 2, 2, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM },
            { MHW_U_PLANE, 2, 1, 2, 2, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_Y210_ADV
    { 2,
        {
            { MHW_Y_PLANE, 1, 1, 1, 1, 1, 1, MHW_MEDIASTATE_SURFACEFORMAT_R16B16_UNORM },
            { MHW_U_PLANE, 2, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UNORM }
        }
    },
    // RENDERHAL_PLANES_Y210_RT
    { 1,
        {
            { MHW_GENERIC_PLANE, 2, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM }
        }
    },
    // RENDERHAL_PLANES_Y210
    { 2,
        {
            { MHW_Y_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_UNORM },
            { MHW_U_PLANE, 2, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_UNORM }
        }
    },
    // RENDERHAL_PLANES_Y210_1PLANE_ADV
    { 1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 1, MHW_MEDIASTATE_SURFACEFORMAT_R16B16_UNORM }
        }
    },
    // RENDERHAL_PLANES_R16G16_SINT
    { 1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R16G16_SINT }
        }
    },
    // RENDERHAL_PLANES_R24_UNORM_X8_TYPELESS
    { 1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R24_UNORM_X8_TYPELESS }
        }
    },
    // RENDERHAL_PLANES_R32_FLOAT_X8X24_TYPELESS
    { 1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R32_FLOAT_X8X24_TYPELESS }
        }
    },
    // RENDERHAL_PLANES_P208
    { 2,
        {
            { MHW_Y_PLANE      , 1, 1, 1, 1, 4, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM },
            { MHW_U_PLANE      , 2, 1, 1, 1, 2, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8_UNORM }
        }
    },
    // RENDERHAL_PLANES_P208_1PLANE_ADV
    { 1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 2, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_YCRCB_NORMAL }
        }
    },
    // RENDERHAL_PLANES_Y416_RT
    { 1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 1, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R8G8B8A8_UNORM }
        }
    },
        // RENDERHAL_PLANES_R32G32B32A32
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 0, 0, MHW_GFX3DSTATE_SURFACEFORMAT_R32G32B32A32_FLOAT }
        }
    },
        //RENDERHAL_PLANES_Y8_ADV
    {   1,
        {
            { MHW_GENERIC_PLANE, 1, 1, 1, 1, 4, 1, MHW_MEDIASTATE_SURFACEFORMAT_Y8_UNORM }
        }
    }
};

//!
//! \brief    Get Y Offset according to the planeOffset struct and surface pitch
//! \details  Get Y Offset according to the planeOffset struct and surface pitch
//! \param    pOsInterface
//!           [in] pointer to OS Interface
//! \param    pOsResource
//!           [in] Pointers to Surface OsResource
//! \return   uint16_t
//!           [out] the plane Y offset
//!
uint16_t RenderHal_CalculateYOffset(PMOS_INTERFACE pOsInterface, PMOS_RESOURCE pOsResource);

MOS_STATUS RenderHal_AllocateDebugSurface(
    PRENDERHAL_INTERFACE     pRenderHal);

MOS_STATUS RenderHal_SetupDebugSurfaceState(
    PRENDERHAL_INTERFACE    pRenderHal);

void RenderHal_FreeDebugSurface(
    PRENDERHAL_INTERFACE     pRenderHal);

int32_t RenderHal_LoadDebugKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMHW_KERNEL_PARAM       pSipKernel);

MOS_STATUS RenderHal_LoadSipKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    void                    *pSipKernel,
    uint32_t                dwSipSize);

MOS_STATUS RenderHal_SendSipStateCmd(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer);

MOS_STATUS RenderHal_AddDebugControl(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer);

//Forward declaration
MOS_STATUS RenderHal_SetSurfaceStateToken(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMHW_SURFACE_TOKEN_PARAMS   pParams,
    void                        *pSurfaceStateToken);

//!
//! \brief    Get Align Unit
//! \details  Set HW alignment Unit
//! \param    uint16_t *pwWidthAlignUnit
//!           [out] Width Align Unit
//! \param    uint16_t *pwHeightAlignUnit
//!           [out] Height Align Unit
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Surface
//! \return   void
//!
void RenderHal_GetAlignUnit(
    uint16_t           *pwWidthAlignUnit,
    uint16_t           *pwHeightAlignUnit,
    PRENDERHAL_SURFACE pRenderHalSurface)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHalSurface);
    
    switch (pRenderHalSurface->OsSurface.Format)
    {
        case Format_YUY2:
        case Format_UYVY:
        case Format_YVYU:
        case Format_VYUY:
        case Format_P208:
            *pwWidthAlignUnit = 1;

            // Height alignment should be 1 but is currently set to 2 because
            // of an issue in AVS scaling.
            *pwHeightAlignUnit = 2;
            break;

        default:
            *pwWidthAlignUnit = 1;
            *pwHeightAlignUnit = 1;
            break;
    }

    // For deinterlace messages, the width must be a multiple of 8.
    if (pRenderHalSurface->bDeinterlaceEnable)
    {
        *pwWidthAlignUnit = 8;
    }
}

//!
//! \brief    Adjust Boundary
//! \details  Adjust the height and width of the surface based on Boundary
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Input Surface
//! \param    RENDERHAL_SS_BOUNDARY Boundary
//!           [in] How the adjustment needs to be done
//! \param    uint32_t *pdwSurfaceWidth
//!           [out] Adjusted surface width
//! \param    uint32_t *pdwSurfaceHeight
//!           [out] Adjusted surface height
//! \return   void
//!
void RenderHal_AdjustBoundary(
    PRENDERHAL_INTERFACE  pRenderHal,
    PRENDERHAL_SURFACE    pRenderHalSurface,
    RENDERHAL_SS_BOUNDARY Boundary,
    uint32_t              *pdwSurfaceWidth,
    uint32_t              *pdwSurfaceHeight)
{
    uint16_t wWidthAlignUnit;
    uint16_t wHeightAlignUnit;

    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pdwSurfaceWidth);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pdwSurfaceHeight);
    
    PMOS_SURFACE pSurface = &(pRenderHalSurface->OsSurface);

    pRenderHal->pfnGetAlignUnit(&wWidthAlignUnit, &wHeightAlignUnit, pRenderHalSurface);

    switch (Boundary)
    {
        case RENDERHAL_SS_BOUNDARY_SRCRECT:
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwHeight, (uint32_t)pRenderHalSurface->rcSrc.bottom), wHeightAlignUnit);
            *pdwSurfaceWidth  = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwWidth, (uint32_t)pRenderHalSurface->rcSrc.right), wWidthAlignUnit);
            break;

        case RENDERHAL_SS_BOUNDARY_DSTRECT:
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwHeight, (uint32_t)pRenderHalSurface->rcDst.bottom), wHeightAlignUnit);
            *pdwSurfaceWidth  = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwWidth, (uint32_t)pRenderHalSurface->rcDst.right), wWidthAlignUnit);
            break;

        // align with max src rect
        case RENDERHAL_SS_BOUNDARY_MAXSRCRECT:
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwHeight, (uint32_t)pRenderHalSurface->rcMaxSrc.bottom), wHeightAlignUnit);
            *pdwSurfaceWidth  = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwWidth, (uint32_t)pRenderHalSurface->rcMaxSrc.right), wWidthAlignUnit);
            break;

        case RENDERHAL_SS_BOUNDARY_ORIGINAL:
        default:
            *pdwSurfaceHeight = MOS_ALIGN_CEIL(pSurface->dwHeight, wHeightAlignUnit);
            *pdwSurfaceWidth  = MOS_ALIGN_CEIL(pSurface->dwWidth,  wWidthAlignUnit);
            break;
    }
}

//!
//! \brief    Get Pixels Per Sample
//! \details  Get Number of Pixels per Dataport Sample
//! \param    MOS_FORMAT format
//!           [in] Surface Format
//! \param    uint32_t *pdwPixelsPerSampleUV
//!           [in] Pointer to dwPixelsPerSampleUV
//! \return   void
//!
void RenderHal_GetPixelsPerSample(
    MOS_FORMAT    format,
    uint32_t      *pdwPixelsPerSampleUV)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pdwPixelsPerSampleUV);
    
    *pdwPixelsPerSampleUV = 0;
    switch (format)
    {
        CASE_PL3_FORMAT:
        CASE_PL3_RGB_FORMAT:
            *pdwPixelsPerSampleUV = 4;   // 4 U/V pixels per dataport sample (uint32_t)
            break;

        // Y and UV surfaces have different formats
        CASE_PL2_FORMAT:
        case Format_400P:
            *pdwPixelsPerSampleUV = 2;   // 2 UV pixels per dataport sample (uint32_t)
            break;

        default:
            *pdwPixelsPerSampleUV = 1;
            MHW_RENDERHAL_ASSERTMESSAGE("Incorrect Filter Format.");
            break;
    }
}

//!
//! \brief    Allocate GSH, SSH, ISH control structures and heaps
//! \details  Allocates State Heap control structure (system memory)
//!           and associated State Buffer in gfx/sys memory (GSH,ISH,SSH)
//!           initializes the State Heap control structure and state buffers
/*-----------------------------------------------------------------------------
|
|      GSH ORGANIZATION (GSH block in GFX memory)
|
|    |===========================================|
|    |              SYNC/PERF TAGS   (SY)        |
|    |-------------------------------------------|
|    |                                           |
|    |             MEDIA STATE [0]   (MS)        |
|    |                                           |
|    |-------------------------------------------|
|    |                                           |
|    |                    ...                    |
|    |                                           |
|    |-------------------------------------------|
|    |                                           |
|    |              MEDIA STATE [Q-1]            |
|    |                                           |
|    |-------------------------------------------|
|    |                                           |
|    |              KERNEL SPILL AREA            |
|    |                                           |
|    |===========================================|
|    where SY = (sSettings.iSyncSize) bytes
|          MS = Media state heap entry shown below.
|          Q  = (sSettings.iMediaStateHeaps) Media States
|
|      ISH ORGANIZATION (ISH block in GFX memory)
|      ~~~~~~~~~~~~~~~~~~~~~~~~
|
|    |===========================================|
|    |   |   |   |   |   |   |   |   |   |   |   |
|    |---+---+---+---+---+---+---+---+---+---+---|
|    |   |   |   |   |   |   |   |   |   |   |   |
|    |---+---+--                       --+---+---|
|    |              KERNEL HEAP  (KH)            |
|    |---+---+--                       --+---+---|
|    |   |   |   |   |   |   |   |   |   |   |   |
|    |---+---+---+---+---+---+---+---+---+---+---|
|    |   |   |   |   |   |   |   |   |   |   |   |
|    |-------------------------------------------|
|    |                                           |
|    |        SYSTEM ROUTINE (SIP Kernel)        |
|    |                                           |
|    |===========================================|
|     where KH = (sSettings.iKernelHeapSize) bytes in allocation
|                 blocks of (sSettings.iKernelBlockSize) bytes
|
|     MEDIA STATE ORGANIZATION (Media States in GFX memory)
|      ~~~~~~~~~~~~~~~~~~~~~~~~
|              (Gen6, Gen7, Gen75 Layout)
|    |================================================|
|    |                                                |
|    |              CURBE DATA        (C)             |
|    |                                                |
|    |------------------------------------------------|
|    | SAMPLER STATES [0] to [S-1] - for ID[0]        |
|    |                    .                           |
|    |                    .                           |
|    | SAMPLER STATES [0] to [S-1] - for ID[M-1]      |
|    |------------------------------------------------|
|    | SAMPLER 8x8 TABLE [0]                          |
|    |                    .                           |
|    | SAMPLER 8x8 TABLE [AS-1]                       |
|    |------------------------------------------------|
|    | MEDIA INTERFACE DESCRIPTOR [0]                 |
|    |                    .                           |
|    |                    .                           |
|    | MEDIA INTERFACE DESCRIPTOR [M-1]               |
|    |------------------------------------------------|
|    | Kernel Execution start time (8 Bytes)          |
|    |------------------------------------------------|
|    | Kernel Execution end time (8 Bytes)            |
|    |------------------------------------------------|
|    | Component ID (4 Bytes)                         |
|    |------------------------------------------------|
|    | Reserved (44 Bytes)                            |
|    |================================================|
|
|                    (Gen8+ Layout)
|    |================================================|
|    |                                                |
|    |              CURBE DATA        (C)             |
|    |                                                |
|    |------------------------------------------------|
|    | SAMPLER STATES         [0] to [S-1]            |
|    | VA SAMPLER 8x8 STATE   [0] to [VA-1]           | ID[0]
|    | AVS SAMPLER 8x8 STATE  [0] to [AS-1]           |
|    |------------------------------------------------|
|    |                    .                           |
|    |                    .                           |
|    |                    .                           |
|    |------------------------------------------------|
|    | SAMPLER STATES         [0] to [S-1]            | ID[M-1]
|    | VA SAMPLER 8x8 STATE   [0] to [VA-1]           |
|    | AVS SAMPLER 8x8 STATE  [0] to [AS-1]           |
|    |------------------------------------------------|
|    | MEDIA INTERFACE DESCRIPTOR [0]                 |
|    |        .                                       |
|    |        .                                       |
|    | MEDIA INTERFACE DESCRIPTOR [M-1]               |
|    |------------------------------------------------|
|    | Kernel Execution start time (8 Bytes)          |
|    |------------------------------------------------|
|    | Kernel Execution end time (8 Bytes)            |
|    |------------------------------------------------|
|    | Component ID (4 Bytes)                         |
|    |------------------------------------------------|
|    | Reserved (44 Bytes)                            |
|    |================================================|
|
|    where C  = (sSettings.iCurbeSize) bytes for static parameters
|          S  = (sSettings.iSamplers) samplers per Media ID
|          AS = (sSettings.iSamplersAVS) advanced sampler 8x8 tables (preGen8)
|          AS = (sSettings.iSamplersAVS) advanced sampler 8x8 states (Gen8+)
|          VA = (sSettings.iSamplersVA)  video analytics sampler
|          M  = (sSettings.iMediaIDs) Media Interface Descriptors (ID)
|
|
|         STATE HEAP CONTROL STRUCTURE ORGANIZATION (in system memory)
|         ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
|            |===========================================|
|            |       RENDERHAL_STATE_HEAP STRUCTURE      |
|            |                                           |
|  +---------| pSshBuffer                                |
|  |         |-------------------------------------------|
|  |         | Kernel Allocation [0]                     |
|  |         |                    .                      |
|  |         |                    .                      |
|  |         | Kernel Allocation [K-1]                   |
|  |         |-------------------------------------------|
|  |         | Media State Control Structure [0]         |--+
|  |         | Media State Control Structure [1]         |--|--+
|  |         |                    .                      |  |  |
|  |         |                    .                      |  |  |
|  |         | Media State Control Structure [Q-1]       |--|--|--+
|  |         |-------------------------------------------|  |  |  |
|  |         | KAID[0] to KAID[M-1] for Media State[0]   |<-+  |  |
|  |         | KAID[0] to KAID[M-1] for Media State[1]   |<----+  |
|  |         |                    .                      |        |
|  |         |                    .                      |        |
|  |         | KAID[0] to KAID[M-1] for Media State[Q-1] |<-------+
|  |         |-------------------------------------------|
|  |  +------| SurfStateEntry[0]                         |
|  |  |      |                    .                      |
|  |  |      |                    .                      |
|  |  |  +---| SurfStateEntry[P-1]                       |
|  |  |  |   |===========================================|
|  |  |  |
|  |  |  |    SSH BUFFER for HW (or SW) - or IndirectBuffer
|  +--|--|-->|==============================| <-- iBindingTableOffset = 0
|     |  |   | BI[0]   ......       BI[T-1] |    <- BT[0] (iBindingTableSize)
|     |  |   | BI[0]   ......       BI[T-1] |    <- BT[1]
|     |  |   |            .                 |
|     |  |   | BI[0]   ......       BI[T-1] |    <- BT[N-1]
|     |  |   |------------------------------| <-- iSurfaceStateOffset
|     +--|-->| SurfState[0]                 |
|        |   |        .                     |
|        |   |        .                     |
|        +-->| SurfState[P-1]               |
|            |==============================|
|
|     where K  = (sSettings.iKernelCount)     Kernel Allocation Entries
|           Q  = (sSettings.iMediaStateHeaps) Media States
|           M  = (sSettings.iMediaIDs)        Media Interface Descriptors (ID)
|           P  = (sSettings.iSurfaceStates)   Surface States
|           T  = (sSettings.iSurfacesPerBT)   Binding Index entries per Binding Table
|           N  = (sSettings.iBindingTables)   Binding Tables
|
|           BT = Binding Table (Each Binding Table contains M Binding Indices)
|           BI = Binding Index
|
|        KAID  => Kernel Allocation ID - each value determines the kernel
|                 allocation entry in StateHeap (0 to sSettings.iKernelCount - 1)
|
|-----------------------------------------------------------------------------*/
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Rendering Interface Structure
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [in] Pointer to state heap settings (GSH/SSH/ISH)
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if state heaps were successfully allocated and initialized
//!           others             if bad parameters or allocation failed
//!
MOS_STATUS RenderHal_AllocateStateHeaps(
    PRENDERHAL_INTERFACE           pRenderHal,
    PRENDERHAL_STATE_HEAP_SETTINGS pSettings)
{
    MHW_STATE_HEAP_SETTINGS      MhwStateHeapSettings;
    PMHW_RENDER_STATE_SIZES      pHwSizes;
    PRENDERHAL_STATE_HEAP        pStateHeap;
    int32_t                      *pAllocations;
    uint32_t                     dwSizeAlloc;
    uint32_t                     dwSizeGSH;
    uint32_t                     dwSizeSSH;
    uint32_t                     dwSizeISH;
    uint32_t                     dwSizeMediaState;
    uint32_t                     dwSizeSamplers;
    PMHW_STATE_HEAP              pDshHeap;
    PMHW_STATE_HEAP              pIshHeap;
    int32_t                      i;
    uint8_t                      *ptr;
    MOS_STATUS                   eStatus;

    // Initialize locals
    eStatus          = MOS_STATUS_UNKNOWN;
    pStateHeap       = nullptr;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pSettings);
    // verify GSH parameters and alignments
    MHW_RENDERHAL_ASSERT((pSettings->iSyncSize        % RENDERHAL_SYNC_BLOCK_ALIGN)   == 0);
    MHW_RENDERHAL_ASSERT((pSettings->iCurbeSize       % RENDERHAL_URB_BLOCK_ALIGN)    == 0);
    MHW_RENDERHAL_ASSERT((pSettings->iKernelHeapSize  % RENDERHAL_KERNEL_BLOCK_ALIGN) == 0);
    MHW_RENDERHAL_ASSERT((pSettings->iKernelBlockSize % RENDERHAL_KERNEL_BLOCK_ALIGN) == 0);
    // verify SSH parameters
    MHW_RENDERHAL_ASSERT(pSettings->iBindingTables >= RENDERHAL_SSH_BINDING_TABLES_MIN);
    MHW_RENDERHAL_ASSERT(pSettings->iBindingTables <= RENDERHAL_SSH_BINDING_TABLES_MAX);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfaceStates >= RENDERHAL_SSH_SURFACE_STATES_MIN);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfaceStates <= RENDERHAL_SSH_SURFACE_STATES_MAX);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfacesPerBT >= RENDERHAL_SSH_SURFACES_PER_BT_MIN);
    MHW_RENDERHAL_ASSERT(pSettings->iSurfacesPerBT <= RENDERHAL_SSH_SURFACES_PER_BT_MAX);
    //---------------------------------------

    pHwSizes   = pRenderHal->pHwSizes;

    //---------------------------------------
    // Setup General State Heap
    //---------------------------------------
    // Calculate size of State Heap control structure
    dwSizeAlloc  = MOS_ALIGN_CEIL(sizeof(RENDERHAL_STATE_HEAP)                                       , 16);
    dwSizeAlloc += MOS_ALIGN_CEIL(pSettings->iKernelCount     * sizeof(RENDERHAL_KRN_ALLOCATION)     , 16);
    dwSizeAlloc += MOS_ALIGN_CEIL(pSettings->iMediaStateHeaps * sizeof(RENDERHAL_MEDIA_STATE)        , 16);
    dwSizeAlloc += MOS_ALIGN_CEIL(pSettings->iMediaStateHeaps * pSettings->iMediaIDs * sizeof(int32_t)   , 16);
    dwSizeAlloc += MOS_ALIGN_CEIL(pSettings->iSurfaceStates   * sizeof(RENDERHAL_SURFACE_STATE_ENTRY), 16);

    // Allocate State Heap control structure (aligned)
    pRenderHal->pStateHeap = pStateHeap = (PRENDERHAL_STATE_HEAP)MOS_AlignedAllocMemory(dwSizeAlloc, 16);
    pRenderHal->dwStateHeapSize = dwSizeAlloc;

    MHW_RENDERHAL_CHK_NULL(pStateHeap);
    MOS_ZeroMemory(pStateHeap, dwSizeAlloc);

    //-------------------------------------------------------------------------
    // Reset resource allocations
    //-------------------------------------------------------------------------
    Mos_ResetResource(&pStateHeap->GshOsResource);
    pStateHeap->bGshLocked = false;
    pStateHeap->pGshBuffer = nullptr;

    Mos_ResetResource(&pStateHeap->SshOsResource);
    pStateHeap->pSshBuffer = nullptr;
    pStateHeap->bSshLocked = false;

    Mos_ResetResource(&pStateHeap->IshOsResource);
    pStateHeap->bIshLocked = false;
    pStateHeap->pIshBuffer = nullptr;

    //-------------------------------------------------------------------------
    // Setup State Heap control structures
    //-------------------------------------------------------------------------
    // Skip RENDERHAL_STATE_HEAP structure
    ptr = (uint8_t*)pStateHeap;
    ptr += MOS_ALIGN_CEIL(sizeof(RENDERHAL_STATE_HEAP), 16);

    // Pointer to Kernel allocations
    pStateHeap->pKernelAllocation = (PRENDERHAL_KRN_ALLOCATION) ptr;
    ptr += MOS_ALIGN_CEIL(pSettings->iKernelCount * sizeof(RENDERHAL_KRN_ALLOCATION), 16);

    // Pointer to Media State allocations
    pStateHeap->pMediaStates = (PRENDERHAL_MEDIA_STATE) ptr;
    ptr += MOS_ALIGN_CEIL(pSettings->iMediaStateHeaps * sizeof(RENDERHAL_MEDIA_STATE), 16);

    // Pointer to Media ID allocations
    pAllocations = (int32_t*) ptr;
    ptr += MOS_ALIGN_CEIL(pSettings->iMediaStateHeaps * pSettings->iMediaIDs * sizeof(int32_t), 16);

    // Pointer to Surface State allocations
    pStateHeap->pSurfaceEntry = (PRENDERHAL_SURFACE_STATE_ENTRY) ptr;

    //-------------------------------------------------------------------------
    // Calculate offsets/sizes in GSH
    //-------------------------------------------------------------------------
    // Synchronization, debugging data
    pStateHeap->dwOffsetSync = 0;
    pStateHeap->dwSizeSync   = pSettings->iSyncSize;
    pStateHeap->dwNextTag    = 0;
    pStateHeap->dwSyncTag    = 0;
    dwSizeGSH                = pStateHeap->dwSizeSync;

    // Align Media State base
    dwSizeGSH = MOS_ALIGN_CEIL(dwSizeGSH, MHW_MEDIA_STATE_ALIGN);

    dwSizeMediaState = 0;

    // Reset current media state
    pStateHeap->iCurMediaState     = 0;
    pStateHeap->iNextMediaState    = 0;

    // Constant URBs
    pStateHeap->dwOffsetCurbe = dwSizeMediaState;
    pStateHeap->dwSizeCurbe   = pSettings->iCurbeSize;
    dwSizeMediaState         += pStateHeap->dwSizeCurbe;

    // Allocate Samplers
    if (pRenderHal->bHasCombinedAVSSamplerState == false)
    {
        // Sampler states
        pStateHeap->dwOffsetSampler     = dwSizeMediaState;
        pStateHeap->dwSizeSampler       = MOS_ALIGN_CEIL(pSettings->iSamplers * pHwSizes->dwSizeSamplerState,  MHW_SAMPLER_STATE_ALIGN);
        dwSizeMediaState               += pSettings->iMediaIDs * pStateHeap->dwSizeSampler;

        // Sampler 8x8 State table is allocated in this area
        pStateHeap->dwOffsetSampler8x8Table = dwSizeMediaState;
        pStateHeap->dwSizeSampler8x8Table   = MOS_ALIGN_CEIL(pHwSizes->dwSizeSamplerStateTable8x8,  MHW_SAMPLER_STATE_ALIGN);
        dwSizeMediaState                   += pSettings->iSamplersAVS * pHwSizes->dwSizeSampler8x8Table;
    }
    else
    {
        // Sampler states - Start of this area to be aligned on MHW_SAMPLER_STATE_ALIGN byte boundary
        uint32_t dwOffsetSamplers          = dwSizeMediaState;
        pStateHeap->dwOffsetSampler     = dwSizeMediaState;
        pStateHeap->dwSizeSampler       = MOS_ALIGN_CEIL(pSettings->iSamplers * pHwSizes->dwSizeSamplerState,  MHW_SAMPLER_STATE_ALIGN);
        dwSizeMediaState               += pStateHeap->dwSizeSampler;

        // Sampler for MinMax, Erode and Dilate - Start of this area to be aligned on MHW_SAMPLER_STATE_VA_ALIGN byte boundary
        pStateHeap->dwOffsetSamplerVA   = dwSizeMediaState;
        pStateHeap->dwSizeSamplerVA     = MOS_ALIGN_CEIL(pSettings->iSamplersVA * pHwSizes->dwSizeSamplerStateVA, MHW_SAMPLER_STATE_VA_ALIGN);
        dwSizeMediaState               += pStateHeap->dwSizeSamplerVA;

        // Sampler for AVS - Start of this area to be aligned on 64 byte boundary
        if(GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform))
        {
            // AVS state should begin on MHW_SAMPLER_STATE_AVS_ALIGN_G9 byte boundary in g9+
            dwSizeMediaState           = MOS_ALIGN_CEIL((dwSizeMediaState - pStateHeap->dwSizeCurbe),  MHW_SAMPLER_STATE_AVS_ALIGN_G9) + pStateHeap->dwSizeCurbe;
        }

        pStateHeap->dwOffsetSamplerAVS = dwSizeMediaState;
        pStateHeap->dwSizeSamplerAVS   = MOS_ALIGN_CEIL(pSettings->iSamplersAVS *
                                                        pHwSizes->dwSizeSamplerStateAvs,  MHW_SAMPLER_STATE_AVS_ALIGN);
        dwSizeMediaState               += pStateHeap->dwSizeSamplerAVS;

        // Sampler Indirect State
        pStateHeap->dwOffsetSamplerIndirect = dwSizeMediaState;
        pStateHeap->dwSizeSamplerIndirect = MOS_ALIGN_CEIL(pSettings->iSamplers * pHwSizes->dwSizeSamplerIndirectState, MHW_SAMPLER_STATE_ALIGN);
        dwSizeMediaState += pStateHeap->dwSizeSamplerIndirect;

        // Get the total size of all the samplers
        dwSizeSamplers                  = MOS_ALIGN_CEIL((dwSizeMediaState - dwOffsetSamplers),  MHW_SAMPLER_STATE_ALIGN);
        dwSizeMediaState               += pSettings->iMediaIDs * dwSizeSamplers;

        // Sampler 8x8 State table is part of Sampler8x8 state
        pStateHeap->dwOffsetSampler8x8Table   = 0;
        pStateHeap->dwSizeSampler8x8Table     = pHwSizes->dwSizeSamplerStateTable8x8;
    }

    // Interface Descriptors
    pStateHeap->dwOffsetMediaID     = dwSizeMediaState;
    pStateHeap->dwSizeMediaID       = pHwSizes->dwSizeInterfaceDescriptor;
    dwSizeMediaState               += pSettings->iMediaIDs * pStateHeap->dwSizeMediaID;

    // 3P Budget
    pStateHeap->dwOffsetStartTime   = dwSizeMediaState;
    pStateHeap->dwStartTimeSize     = sizeof(uint64_t);
    dwSizeMediaState               += pStateHeap->dwStartTimeSize;

    pStateHeap->dwOffsetEndTime     = dwSizeMediaState;
    pStateHeap->dwEndTimeSize       = sizeof(uint64_t);
    dwSizeMediaState               += pStateHeap->dwEndTimeSize;

    pStateHeap->dwOffsetComponentID = dwSizeMediaState;
    pStateHeap->dwComponentIDSize   = sizeof(uint32_t);
    dwSizeMediaState               += pStateHeap->dwComponentIDSize;

    pStateHeap->dwOffsetReserved    = dwSizeMediaState;
    pStateHeap->dwReservedSize      = 44;  // 64 - 2 * sizeof(uint64_t) - sizeof(uint32_t);
    dwSizeMediaState               += pStateHeap->dwReservedSize;

    // Ensure correct alignment of consecutive Media States
    if(GFX_IS_GEN_9_OR_LATER(pRenderHal->Platform))
    {
        dwSizeMediaState = MOS_ALIGN_CEIL(dwSizeMediaState, MHW_SAMPLER_STATE_AVS_ALIGN_G9);
    }
    else
    {
        dwSizeMediaState = MOS_ALIGN_CEIL(dwSizeMediaState, MHW_SAMPLER_STATE_AVS_ALIGN);
    }

    // Create multiple instances of Media state heaps for Dynamic GSH
    for (i = 0; i < pSettings->iMediaStateHeaps; i++)
    {
        pStateHeap->pMediaStates[i].dwOffset     = dwSizeGSH;
        pStateHeap->pMediaStates[i].piAllocation = pAllocations;
        dwSizeGSH    += dwSizeMediaState;
        pAllocations += pSettings->iMediaIDs;
    }

    // Kernel Spill Area
    if (pSettings->iPerThreadScratchSize > 0)
    {
        // Scratch Space Base Pointer. Specifies the MHW_SCRATCH_SPACE_ALIGN
        // aligned address offset to scratch space for use by the kernel.
        // This pointer is relative to the General State Base Address.
        dwSizeGSH = MOS_ALIGN_CEIL(dwSizeGSH, MHW_SCRATCH_SPACE_ALIGN);

        MHW_RENDERHAL_ASSERT(pSettings->iPerThreadScratchSize ==
            MOS_ALIGN_CEIL(pSettings->iPerThreadScratchSize, MHW_SCRATCH_SPACE_ALIGN));

        pStateHeap->dwScratchSpaceSize = pRenderHal->pfnGetScratchSpaceSize(pRenderHal, pSettings->iPerThreadScratchSize);
        pStateHeap->dwScratchSpaceBase = dwSizeGSH;
        dwSizeGSH                     += pStateHeap->dwScratchSpaceSize;
    }

    pStateHeap->dwSizeGSH = dwSizeGSH;

    //----------------------------------
    // Instruction State Heap (Kernel Heap)
    //----------------------------------
    pStateHeap->dwKernelBase = 0;
    dwSizeISH = MOS_ALIGN_CEIL(pSettings->iKernelHeapSize, RENDERHAL_KERNEL_BLOCK_ALIGN);

    // System Routine Area, always loaded for Cm
    // Don't put the allocation under bIsaAsmDebugEnable
    pStateHeap->dwSipBase                 = dwSizeISH;
    pRenderHal->SipStateParams.bSipKernel = false;
    pRenderHal->SipStateParams.dwSipBase  = dwSizeISH;
    dwSizeISH            += MOS_ALIGN_CEIL(pSettings->iSipSize, RENDERHAL_KERNEL_BLOCK_ALIGN);

    pStateHeap->dwSizeISH = dwSizeISH;

    //----------------------------------
    // Surface State Heap
    //----------------------------------
    // Reset initial SSH allocations
    pStateHeap->iCurSshBufferIndex    = 0;
    pStateHeap->iCurrentBindingTable  = 0;
    pStateHeap->iCurrentSurfaceState  = 0;

    // Set BT sizes
    pStateHeap->iBindingTableSize = MOS_ALIGN_CEIL(pSettings->iSurfacesPerBT * pHwSizes->dwSizeBindingTableState,
                                                   pSettings->iBTAlignment);

    // Set offsets to BT and SS entries
    pStateHeap->iBindingTableOffset  = 0;
    pStateHeap->iSurfaceStateOffset  = pSettings->iBindingTables * pStateHeap->iBindingTableSize;

    // Calculate size of a single SSH instance and total SSH buffer size
    dwSizeSSH = pStateHeap->iSurfaceStateOffset +
                pSettings->iSurfaceStates * pRenderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize();
    pStateHeap->dwSshIntanceSize   = dwSizeSSH;
    pRenderHal->dwIndirectHeapSize = MOS_ALIGN_CEIL(dwSizeSSH, MHW_PAGE_SIZE);

    // Allocate SSH buffer in system memory, not Gfx
    pStateHeap->dwSizeSSH  = dwSizeSSH; // Single SSH instance
    pStateHeap->pSshBuffer = (uint8_t*)MOS_AllocAndZeroMemory(dwSizeSSH);
    if (!pStateHeap->pSshBuffer)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Fail to Allocate SSH buffer.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    pStateHeap->bSshLocked = true;

    //----------------------------------
    // Allocate State Heap in MHW
    //----------------------------------
    MOS_ZeroMemory(&MhwStateHeapSettings, sizeof(MhwStateHeapSettings));
    MhwStateHeapSettings.dwDshSize     = pStateHeap->dwSizeGSH;
    MhwStateHeapSettings.dwIshSize     = pStateHeap->dwSizeISH;
    MhwStateHeapSettings.dwNumSyncTags = pStateHeap->dwSizeSync;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AllocateHeaps(MhwStateHeapSettings));
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface->m_stateHeapInterface);

    pDshHeap = pRenderHal->pMhwStateHeap->GetDSHPointer();
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->LockStateHeap(pDshHeap));
    pStateHeap->GshOsResource = pDshHeap->resHeap;
    pStateHeap->pGshBuffer    = (uint8_t*)pDshHeap->pvLockedHeap;
    pStateHeap->bGshLocked    = true;

    pIshHeap = pRenderHal->pMhwStateHeap->GetISHPointer();
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->LockStateHeap(pIshHeap));
    pStateHeap->IshOsResource = pIshHeap->resHeap;
    pStateHeap->pIshBuffer    = (uint8_t*)pIshHeap->pvLockedHeap;
    pStateHeap->bIshLocked    = true;

    //-----------------------------
    // Heap initialization
    //-----------------------------

    // Reset GSH contents until scratch space (excluding)
    MOS_ZeroMemory(pStateHeap->pGshBuffer, pStateHeap->dwScratchSpaceBase);

    // Setup pointer to sync tags
    pStateHeap->pSync = (uint32_t*) (pStateHeap->pGshBuffer + pStateHeap->dwOffsetSync);

    // Reset kernel allocations
    pRenderHal->pfnResetKernels(pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (pStateHeap)
        {
            // Free SSH buffer
            if (pStateHeap->pSshBuffer)
            {
                MOS_FreeMemory(pStateHeap->pSshBuffer);
            }

            // Free State Heap control structure
            MOS_AlignedFreeMemory(pStateHeap);
            pRenderHal->pStateHeap = nullptr;
        }
    }

    return eStatus;
}

//!
//! \brief    Free State Heaps (including MHW interfaces)
//! \details  Free State Heap resources allocated by RenderHal
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Render Hal Interface
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_FreeStateHeaps(PRENDERHAL_INTERFACE pRenderHal)
{
    PMOS_INTERFACE        pOsInterface;
    PRENDERHAL_STATE_HEAP pStateHeap;
    MOS_STATUS            eStatus;

    //------------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //------------------------------------------------

    eStatus      = MOS_STATUS_UNKNOWN;

    pOsInterface = pRenderHal->pOsInterface;
    pStateHeap   = pRenderHal->pStateHeap;

    // Free SSH Resource
    if (pStateHeap->pSshBuffer)
    {
        MOS_FreeMemory(pStateHeap->pSshBuffer);
        pStateHeap->pSshBuffer = nullptr;
    }

    // Free MOS surface in surface state entry
    for (int32_t index = 0; index < pRenderHal->StateHeapSettings.iSurfaceStates; ++index) {
        PRENDERHAL_SURFACE_STATE_ENTRY entry = pStateHeap->pSurfaceEntry + index;
        MOS_SafeFreeMemory(entry->pSurface);
        entry->pSurface = nullptr;
    }

    // Free State Heap Control structure
    MOS_AlignedFreeMemory(pStateHeap);
    pRenderHal->pStateHeap = nullptr;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Allocate BB
//! \details  Allocate Batch Buffer resource (not BB structure),
//!           attaching it to head of linked list in RenderHal.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardwaew interface structure
//! \param    PMHW_BATCH_BUFFER pBatchBuffer
//!           [out] Pointer to batch buffer structure to be initialized
//! \param    int32_t iSize
//!           [in] Size of the batch buffer to be allocated
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS  if the Batch Buffer was successfully allocated
//!           others              if failed
//!
MOS_STATUS RenderHal_AllocateBB(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMHW_BATCH_BUFFER       pBatchBuffer,
    int32_t                 iSize)
{
    PMOS_INTERFACE           pOsInterface;
    MOS_RESOURCE             OsResource;
    MOS_ALLOC_GFXRES_PARAMS  AllocParams;
    MOS_STATUS               eStatus;

    //---------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pBatchBuffer);
    //---------------------------------------------

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = pRenderHal->pOsInterface;

    MOS_ZeroMemory(&OsResource, sizeof(OsResource));

    MOS_ZeroMemory(&AllocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    AllocParams.Type     = MOS_GFXRES_BUFFER;
    AllocParams.TileType = MOS_TILE_LINEAR;
    AllocParams.Format   = Format_Buffer;
    AllocParams.dwBytes  = iSize;
    AllocParams.pBufName = "RenderHalBB";

    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &OsResource));

    // Reset Allocation
    pOsInterface->pfnResetResourceAllocationIndex(
        pOsInterface,
        &OsResource);

    pBatchBuffer->OsResource       = OsResource;
    pBatchBuffer->iRemaining       = iSize;
    pBatchBuffer->iSize            = iSize;
    pBatchBuffer->iCurrent         = 0;
    pBatchBuffer->bLocked          = false;
    pBatchBuffer->pData            = nullptr;

    pBatchBuffer->dwOffset         = 0;

    pBatchBuffer->bBusy            = false;
    pBatchBuffer->dwCmdBufId       = 0;

    // Link BB to beginning of linked list
    pBatchBuffer->pPrev            = nullptr;
    pBatchBuffer->pNext            = pRenderHal->pBatchBufferList;
    pRenderHal->pBatchBufferList   = pBatchBuffer;
    if (pBatchBuffer->pNext)
    {
        pBatchBuffer->pNext->pPrev = pBatchBuffer;
    }

    // Reset Sync/BB Reuse data
    pBatchBuffer->dwSyncTag        = 0;
    pBatchBuffer->bMatch           = false;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Free BB
//! \details  Frees Batch Buffer Resource (not structure),
//!           detaching it from linked list in RenderHal.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMHW_BATCH_BUFFER pBatchBuffer
//!           [in] Pointer to Batch Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_FreeBB(
    PRENDERHAL_INTERFACE pRenderHal,
    PMHW_BATCH_BUFFER pBatchBuffer)
{
    PMOS_INTERFACE      pOsInterface;
    MOS_STATUS          eStatus;

    //----------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pBatchBuffer);
    //----------------------------------------------

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = pRenderHal->pOsInterface;

    if (pBatchBuffer->bLocked)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnUnlockBB(pRenderHal, pBatchBuffer));
    }

    pOsInterface->pfnFreeResource(pOsInterface, &pBatchBuffer->OsResource);

    pBatchBuffer->dwSyncTag        = 0;
    pBatchBuffer->iSize            = 0;
    pBatchBuffer->iCurrent         = 0;
    pBatchBuffer->bMatch           = 0;

    // Unlink BB from synchronization list
    if (pBatchBuffer->pNext)
    {
        pBatchBuffer->pNext->pPrev = pBatchBuffer->pPrev;
    }

    if (pBatchBuffer->pPrev)
    {
        pBatchBuffer->pPrev->pNext = pBatchBuffer->pNext;
    }
    else
    {
        pRenderHal->pBatchBufferList = pBatchBuffer->pNext;
    }

    pBatchBuffer->pPrev = pBatchBuffer->pNext = nullptr;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Lock BB
//! \details  Locks Batch Buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMHW_BATCH_BUFFER pBatchBuffer
//!           [in] Pointer to Batch Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_LockBB(
    PRENDERHAL_INTERFACE pRenderHal,
    PMHW_BATCH_BUFFER pBatchBuffer)
{
    PMOS_INTERFACE      pOsInterface;
    MOS_LOCK_PARAMS     LockFlags;
    MOS_STATUS          eStatus;

    //-----------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pBatchBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //-----------------------------------

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = pRenderHal->pOsInterface;

    if (pBatchBuffer->bLocked)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Batch Buffer is already locked.");
        goto finish;
    }

    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));

    LockFlags.WriteOnly = 1;

    pBatchBuffer->pData = (uint8_t*)pOsInterface->pfnLockResource(
                                    pOsInterface,
                                    &pBatchBuffer->OsResource,
                                    &LockFlags);

    MHW_RENDERHAL_CHK_NULL(pBatchBuffer->pData);

    pBatchBuffer->bLocked   = true;
    eStatus                 = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Unlock BB
//! \details  Unlocks Batch Buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMHW_BATCH_BUFFER pBatchBuffer
//!           [in] Pointer to Batch Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_UnlockBB(
    PRENDERHAL_INTERFACE pRenderHal,
    PMHW_BATCH_BUFFER pBatchBuffer)
{
    PMOS_INTERFACE      pOsInterface;
    MOS_STATUS          eStatus;

    //---------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pBatchBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //---------------------------------

    eStatus         = MOS_STATUS_UNKNOWN;
    pOsInterface    = pRenderHal->pOsInterface;

    if (!pBatchBuffer->bLocked)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Batch buffer is locked.");
        goto finish;
    }

    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnUnlockResource(
                pOsInterface,
                &pBatchBuffer->OsResource));

    pBatchBuffer->bLocked = false;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Refresh Sync
//! \details  Update Sync tags
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_RefreshSync(PRENDERHAL_INTERFACE pRenderHal)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_MEDIA_STATE      pCurMediaState;
    PMHW_BATCH_BUFFER           pBatchBuffer;
    uint32_t                    dwCurrentTag;
    int32_t                     i;
    int32_t                     iStatesInUse;
    int32_t                     iBuffersInUse;
    MOS_STATUS                  eStatus;
    MOS_NULL_RENDERING_FLAGS    NullRenderingFlags;
    uint8_t                     *pCurrentPtr;
    uint64_t                    uiStartTime;
    uint64_t                    uiEndTime;
    uint64_t                    uiDiff;
    uint64_t                    uiNS;
    double                      TimeMS;
    uint32_t                    uiComponent;

    //----------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //----------------------------------

    eStatus      = MOS_STATUS_UNKNOWN;
    pCurrentPtr  = nullptr;
    uiNS         = 0;

    // GSH must be locked
    pStateHeap = pRenderHal->pStateHeap;
    if (!pStateHeap->bGshLocked)
    {
        goto finish;
    }

    // Most recent tag
    dwCurrentTag = pStateHeap->pSync[0];
    pStateHeap->dwSyncTag = dwCurrentTag - 1;

    // Refresh batch buffers
    iBuffersInUse       = 0;
    pBatchBuffer        = pRenderHal->pBatchBufferList;

    NullRenderingFlags  = pRenderHal->pOsInterface->pfnGetNullHWRenderFlags(
                        pRenderHal->pOsInterface);

    for (; pBatchBuffer != nullptr; pBatchBuffer = pBatchBuffer->pNext)
    {
        if (!pBatchBuffer->bBusy) continue;

        // Clear BB busy flag when Sync Tag is reached
        if ((int32_t)(dwCurrentTag - pBatchBuffer->dwSyncTag) > 0 ||
            NullRenderingFlags.VPGobal)
        {
            pBatchBuffer->bBusy = false;
        }
        else
        {
            iBuffersInUse++;
        }
    }

    // Refresh media states
    pCurMediaState = pStateHeap->pMediaStates;
    iStatesInUse   = 0;
    for (i = pRenderHal->StateHeapSettings.iMediaStateHeaps; i > 0; i--, pCurMediaState++)
    {
        if (!pCurMediaState->bBusy) continue;

        // The condition below is valid when sync tag wraps from 2^32-1 to 0
        if ((int32_t)(dwCurrentTag - pCurMediaState->dwSyncTag) > 0)
        {
            pCurMediaState->bBusy = false;
            if (pRenderHal->bKerneltimeDump)
            {
                // Dump Kernel execution time when media state is being freed
                pCurrentPtr = pStateHeap->pGshBuffer +
                              pCurMediaState->dwOffset +
                              pStateHeap->dwOffsetStartTime;
                if (pCurrentPtr)
                {
                    uiStartTime = *((uint64_t *) pCurrentPtr);
                    pCurrentPtr += pStateHeap->dwStartTimeSize;

                    uiEndTime =  *((uint64_t *) pCurrentPtr);
                    pCurrentPtr += pStateHeap->dwEndTimeSize;

                    uiComponent = *((RENDERHAL_COMPONENT *) pCurrentPtr);
                    if (uiComponent < (uint32_t)RENDERHAL_COMPONENT_COUNT)
                    {
                        // Convert ticks to ns
                        uiDiff   = uiEndTime - uiStartTime;
                        uiNS     = 0;
                        pRenderHal->pfnConvertToNanoSeconds(pRenderHal, uiDiff, &uiNS);

                        TimeMS = ((double)uiNS) / (1000 * 1000); // Convert to ms (double)

                        pRenderHal->kernelTime[uiComponent] += TimeMS;
                    }
                }
            }
        }
        else
        {
            iStatesInUse++;
        }
    }

    // Save number of states/buffers in use
    pRenderHal->iBuffersInUse     = iBuffersInUse;
    pRenderHal->iMediaStatesInUse = iStatesInUse;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Load Kernel
//! \details  Load a kernel from cache into GSH; searches for unused space in 
//!           the kernel heap; deallocates kernels identified as no longer in use.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PCRENDERHAL_KERNEL_PARAM pParameters
//!           [in] Pointer to Kernel Parameters
//! \param    PMHW_KERNEL_PARAMS pKernel
//!           [in] Pointer to Kernel entry
//! \param    Kdll_CacheEntry pKernelEntry
//!           [in] The cache entry pointer maintaining the load status.
//!                For cache entries from local variable,
//!                set it to nullptr to avoid memory corruption
//! \return   int32_t
//!           Index to a kernel allocation index
//!           -1 if invalid parameters, no available space and no
//!            deallocation possible
//!
int32_t RenderHal_LoadKernel(
    PRENDERHAL_INTERFACE       pRenderHal,
    PCRENDERHAL_KERNEL_PARAM   pParameters,
    PMHW_KERNEL_PARAM          pKernel,
    Kdll_CacheEntry            *pKernelEntry)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;

    int32_t iKernelAllocationID;    // Kernel allocation ID in GSH
    int32_t iKernelCacheID;         // Kernel cache ID
    int32_t iKernelUniqueID;        // Kernel unique ID
    void    *pKernelPtr;
    int32_t iKernelSize;
    int32_t iSearchIndex;
    int32_t iMaxKernels;            // Max number of kernels allowed in GSH
    uint32_t dwOffset;
    int32_t iSize;
    MOS_STATUS eStatus;

    iKernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
    eStatus             = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pKernelAllocation);
    MHW_RENDERHAL_CHK_NULL(pParameters);
    MHW_RENDERHAL_CHK_NULL(pKernel);

    pStateHeap          = pRenderHal->pStateHeap;

    // Validate parameters
    if (pStateHeap->bGshLocked == false ||
        pKernel->iSize == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_RENDERHAL_NORMALMESSAGE("Failed to load kernel - invalid parameters.");
        goto finish;
    }

    // Kernel parameters
    pKernelPtr      = pKernel->pBinary;
    iKernelSize     = pKernel->iSize;
    iKernelUniqueID = pKernel->iKUID;
    iKernelCacheID  = pKernel->iKCID;

    // Check if kernel is already loaded; Search free allocation index
    iSearchIndex = -1;
    iMaxKernels  = pRenderHal->StateHeapSettings.iKernelCount;
    pKernelAllocation = pStateHeap->pKernelAllocation;
    for (iKernelAllocationID = 0;
         iKernelAllocationID < iMaxKernels;
         iKernelAllocationID++, pKernelAllocation++)
    {
        if (pKernelAllocation->iKUID == iKernelUniqueID &&
            pKernelAllocation->iKCID == iKernelCacheID)
        {
            break;
        }

        if (iSearchIndex < 0 &&
            pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
        {
            iSearchIndex = iKernelAllocationID;
        }
    }

    // The kernel size to be dumped in oca buffer.
    pStateHeap->iKernelUsedForDump = iKernelSize;

    // Kernel already loaded: refresh timer; return allocation index
    if (iKernelAllocationID < iMaxKernels)
    {
        // To reload the kernel forcibly if needed
        if (pKernel->bForceReload)
        {
            dwOffset = pKernelAllocation->dwOffset;
            MOS_SecureMemcpy(pStateHeap->pIshBuffer + dwOffset, iKernelSize, pKernelPtr, iKernelSize);

            pKernel->bForceReload = false;
        }
        goto finish;
    }

    // Simple allocation: allocation index available, space available
    if ((iSearchIndex >= 0) &&
        (pStateHeap->iKernelUsed + iKernelSize <= pStateHeap->iKernelSize))
    {
        // Allocate kernel at the end of the heap
        iKernelAllocationID = iSearchIndex;
        pKernelAllocation   = &(pStateHeap->pKernelAllocation[iSearchIndex]);

        // Allocate block from the end of the heap
        dwOffset = pStateHeap->dwKernelBase + pStateHeap->iKernelUsed;
        iSize    = MOS_ALIGN_CEIL(iKernelSize, pRenderHal->StateHeapSettings.iKernelBlockSize);

        // Update heap
        pStateHeap->iKernelUsed += iSize;

        // Load kernel
        goto loadkernel;
    }

    // Search block from deallocated entry
    if (iSearchIndex >= 0)
    {
        int32_t iMinSize = 0;

        iSearchIndex = -1;
        pKernelAllocation = pStateHeap->pKernelAllocation;
        for (iKernelAllocationID = 0;
             iKernelAllocationID < iMaxKernels;
             iKernelAllocationID++, pKernelAllocation++)
        {
            // Skip allocated/empty entries
            if (pKernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_FREE ||
                pKernelAllocation->iSize   == 0)
            {
                continue;
            }

            // Allocate minimum available block
            if (pKernelAllocation->iSize >= iKernelSize)
            {
                if (iSearchIndex < 0 ||
                    pKernelAllocation->iSize < iMinSize)
                {
                    iSearchIndex = iKernelAllocationID;
                    iMinSize     = pKernelAllocation->iSize;
                }
            }
        }
    }

    // Did not find block, try to deallocate a kernel not recently used
    if (iSearchIndex < 0)
    {
        uint32_t dwOldest = 0;
        uint32_t dwLastUsed;

        // Search and deallocate least used kernel
        pKernelAllocation = pStateHeap->pKernelAllocation;
        for (iKernelAllocationID = 0;
             iKernelAllocationID < iMaxKernels;
             iKernelAllocationID++, pKernelAllocation++)
        {
            // Skip unused entries and entries that would not fit
            // Skip kernels flagged as locked (cannot be automatically deallocated)
            if (pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||
                pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_LOCKED ||
                pKernelAllocation->iSize < iKernelSize)
            {
                continue;
            }

            // Check if kernel may be replaced (not in use by GPU)
            if ((int32_t)(pStateHeap->dwSyncTag - pKernelAllocation->dwSync) < 0)
            {
                continue;
            }

            // Find kernel not used for the greater amount of time (measured in number of operations)
            // Must not unload recently allocated kernels
            dwLastUsed = (uint32_t)(pStateHeap->dwAccessCounter - pKernelAllocation->dwCount);
            if (dwLastUsed > dwOldest)
            {
                iSearchIndex = iKernelAllocationID;
                dwOldest     = dwLastUsed;
            }
        }

        // Did not found any entry for deallocation
        if (iSearchIndex < 0)
        {
            MHW_RENDERHAL_NORMALMESSAGE("Failed to load kernel - no space available in GSH.");
            iKernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
            goto finish;
        }

        // Free kernel entry and states associated with the kernel (if any)
        if (pRenderHal->pfnUnloadKernel(pRenderHal, iSearchIndex) != MOS_STATUS_SUCCESS)
        {
            MHW_RENDERHAL_NORMALMESSAGE("Failed to load kernel - no space available in GSH.");
            iKernelAllocationID = RENDERHAL_KERNEL_LOAD_FAIL;
            goto finish;
        }
    }

    // Allocate the entry
    iKernelAllocationID = iSearchIndex;
    pKernelAllocation   = &(pStateHeap->pKernelAllocation[iSearchIndex]);

    dwOffset = pKernelAllocation->dwOffset;
    iSize    = pKernelAllocation->iSize;

loadkernel:
    // Allocate kernel
    pKernelAllocation->iKID            = -1;
    pKernelAllocation->iKUID           = iKernelUniqueID;
    pKernelAllocation->iKCID           = iKernelCacheID;
    pKernelAllocation->dwSync          = 0;
    FrameTrackerTokenFlat_Clear(&pKernelAllocation->trackerToken);
    pKernelAllocation->dwOffset        = dwOffset;
    pKernelAllocation->iSize           = iSize;
    pKernelAllocation->dwFlags         = RENDERHAL_KERNEL_ALLOCATION_USED;
    pKernelAllocation->dwCount         = 0;  // will be updated by "TouchKernel"
    pKernelAllocation->Params          = *pParameters;
    pKernelAllocation->pKernelEntry    = pKernelEntry;
    pKernelAllocation->iAllocIndex     = iKernelAllocationID;

    // Copy kernel data
    MOS_SecureMemcpy(pStateHeap->pIshBuffer + dwOffset, iKernelSize, pKernelPtr, iKernelSize);
    if (iKernelSize < iSize)
    {
        MOS_ZeroMemory(pStateHeap->pIshBuffer + dwOffset + iKernelSize, iSize - iKernelSize);
    }

finish:
    if (iKernelAllocationID != RENDERHAL_KERNEL_LOAD_FAIL)
    {
        // Update kernel usage
        pRenderHal->pfnTouchKernel(pRenderHal, iKernelAllocationID);

        // Increment reference counter
        if (pKernelEntry)
        {
            pKernelEntry->dwLoaded = 1;
        }
        pRenderHal->iKernelAllocationID = iKernelAllocationID;
    }

    // Return kernel allocation index
    return iKernelAllocationID;
}

//!
//! \brief    Unload Kernel
//! \details  Unload a kernel from GSH, free kernel heap space
//!           Notify that the kernel has been unloaded (for tracking)
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t iKernelAllocationID
//!           [in] Kernel allocation index in GSH
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS    if success
//!           others                if invalid parameters or if kernel cannot be unloaded
//!
MOS_STATUS RenderHal_UnloadKernel(
    PRENDERHAL_INTERFACE pRenderHal,
    int32_t              iKernelAllocationID)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;
    MOS_STATUS                  eStatus;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL((void*)(iKernelAllocationID >= 0));
    //---------------------------------------

    eStatus    = MOS_STATUS_UNKNOWN;
    pStateHeap = pRenderHal->pStateHeap;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pKernelAllocation);
    MHW_RENDERHAL_ASSERT(iKernelAllocationID < pRenderHal->StateHeapSettings.iKernelCount);
    //---------------------------------------

    pKernelAllocation = &(pStateHeap->pKernelAllocation[iKernelAllocationID]);

    if (pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE)
    {
        goto finish;
    }

    // Update Sync tags
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnRefreshSync(pRenderHal));

    // Check if kernel may be unloaded
    if ((int32_t)(pStateHeap->dwSyncTag - pKernelAllocation->dwSync) < 0)
    {
        goto finish;
    }

    // Unload kernel
    if (pKernelAllocation->pKernelEntry)
    {
        pKernelAllocation->pKernelEntry->dwLoaded = 0;
    }

    // Release kernel entry (Offset/size may be used for reallocation)
    pKernelAllocation->iKID             = -1;
    pKernelAllocation->iKUID            = -1;
    pKernelAllocation->iKCID            = -1;
    pKernelAllocation->dwSync           = 0;
    FrameTrackerTokenFlat_Clear(&pKernelAllocation->trackerToken);
    pKernelAllocation->dwFlags          = RENDERHAL_KERNEL_ALLOCATION_FREE;
    pKernelAllocation->dwCount          = 0;
    pKernelAllocation->pKernelEntry     = nullptr;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Touch Kernel
//! \details  Touch Kernel
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t iKernelAllocationID
//!           [in] Kernel Allocation ID
//! \return   void
//!
void RenderHal_TouchKernel(
    PRENDERHAL_INTERFACE pRenderHal,
    int32_t             iKernelAllocationID)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;

    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap == nullptr ||
        pStateHeap->pKernelAllocation == nullptr ||
        iKernelAllocationID < 0 ||
        iKernelAllocationID >= pRenderHal->StateHeapSettings.iKernelCount)
    {
        return;
    }

    // Update usage
    pKernelAllocation = &(pStateHeap->pKernelAllocation[iKernelAllocationID]);
    if (pKernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_FREE &&
        pKernelAllocation->dwFlags != RENDERHAL_KERNEL_ALLOCATION_LOCKED)
    {
        pKernelAllocation->dwCount = pStateHeap->dwAccessCounter++;
    }

    // Set sync tag, for deallocation control
    pKernelAllocation->dwSync = pStateHeap->dwNextTag;
}

//!
//! \brief    Reset Kernels
//! \details  Reset Kernels
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \return   void
//!
void RenderHal_ResetKernels(
    PRENDERHAL_INTERFACE pRenderHal)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;
    int32_t                     i;
    MOS_STATUS                  eStatus = MOS_STATUS_UNKNOWN;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pKernelAllocation);
    //---------------------------------------

    pStateHeap    = pRenderHal->pStateHeap;

    // Unload kernels and notify HAL layer
    pKernelAllocation = pStateHeap->pKernelAllocation;
    for (i = 0; i< pRenderHal->StateHeapSettings.iKernelCount; i++, pKernelAllocation++)
    {
        // Unload kernel
        if (pKernelAllocation->pKernelEntry)
        {
            pKernelAllocation->pKernelEntry->dwLoaded = 0;
        }

        pKernelAllocation->iKID             = -1;
        pKernelAllocation->iKUID            = -1;
        pKernelAllocation->iKCID            = -1;
        pKernelAllocation->dwSync           = 0;
        FrameTrackerTokenFlat_Clear(&pKernelAllocation->trackerToken);
        pKernelAllocation->dwOffset         = 0;
        pKernelAllocation->iSize            = 0;
        pKernelAllocation->dwFlags          = RENDERHAL_KERNEL_ALLOCATION_FREE;
        pKernelAllocation->dwCount          = 0;
        pKernelAllocation->pKernelEntry     = nullptr;
        pKernelAllocation->iAllocIndex      = i;
        pKernelAllocation->Params           = g_cRenderHal_InitKernelParams;
    }

    // Free Kernel Heap
    pStateHeap->dwAccessCounter = 0;
    pStateHeap->iKernelSize = pRenderHal->StateHeapSettings.iKernelHeapSize;
    pStateHeap->iKernelUsed = 0;
    pStateHeap->iKernelUsedForDump = 0;

finish:
    return;
}

//!
//! \brief    Get Kernel Offset
//! \details  Get Kernel Offset
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t iKernelAllocationIndex
//!           [in] Kernel Allocation Index
//! \return   int32_t
//!
int32_t RenderHal_GetKernelOffset(
    PRENDERHAL_INTERFACE pRenderHal,
    int32_t             iKernelAllocationIndex)
{
    PRENDERHAL_STATE_HEAP pStateHeap;
    int32_t               iResult;

    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap &&
        pStateHeap->pKernelAllocation &&
        iKernelAllocationIndex >= 0 &&
        iKernelAllocationIndex < pRenderHal->StateHeapSettings.iKernelCount)
    {
        iResult = pStateHeap->pKernelAllocation[iKernelAllocationIndex].dwOffset;
    }
    else
    {
        iResult = -1;
    }

    return iResult;
}

//!
//! \brief    Allocate Media ID
//! \details  Allocates an setup Interface Descriptor for Media Pipeline
//!           Kernel        must be preloaded using pfnLoadKernel
//!           Curbe         must be allocated using pfnAllocateCurbe
//!           Binding Table must be allocated using pfnAllocateBindingTable
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t iKernelAllocationID
//!           [in] Kernel Allocation ID
//! \param    int32_t iBindingTableID
//!           [in] Binding Table ID
//! \param    int32_t iCurbeOffset
//!           [in] Curbe Offset (from Curbe base)
//! \param    int32_t iCurbeLength
//!           [in] Curbe Length
//! \param    int32_t iCrsThrdConstDataLn
//!           [in] Cross Thread constant data length
//! \param    PMHW_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
//!           [in] Pointer to GpGpu Walker Params
//! \return   int32_t
//!           Media Interface descriptor ID
//!           -1 if invalid parameters, no ID entry available in GSH
//!
int32_t RenderHal_AllocateMediaID(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                    iKernelAllocationID,
    int32_t                    iBindingTableID,
    int32_t                    iCurbeOffset,
    int32_t                    iCurbeLength,
    int32_t                    iCrsThrdConstDataLn,
    PMHW_GPGPU_WALKER_PARAMS   pGpGpuWalkerParams)
{
    PRENDERHAL_STATE_HEAP             pStateHeap;
    PRENDERHAL_KRN_ALLOCATION         pKernelAllocation;
    PRENDERHAL_MEDIA_STATE            pCurMediaState;
    int32_t                           iCurbeSize;
    int32_t                           iInterfaceDescriptor;
    RENDERHAL_INTERFACE_DESCRIPTOR_PARAMS InterfaceDescriptorParams;

    iInterfaceDescriptor = -1;

    // Validate GSH
    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap == nullptr ||
        pStateHeap->pKernelAllocation == nullptr ||
        pStateHeap->bGshLocked == false)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid GSH State.");
        goto finish;
    }

    // Obtain pointer to current media state
    pCurMediaState = pStateHeap->pCurMediaState;
    if (pCurMediaState == nullptr ||
        pCurMediaState->piAllocation == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Media State.");
        goto finish;
    }

    // Validate kernel allocation ID (kernel must be pre-loaded into GSH)
    if (iKernelAllocationID < 0 ||
        iKernelAllocationID >= pRenderHal->StateHeapSettings.iKernelCount)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Kernel Allocation ID.");
        goto finish;
    }

    // Obtain pointer to kernel allocation control and kernel parameters
    pKernelAllocation = &(pStateHeap->pKernelAllocation[iKernelAllocationID]);
    if (pKernelAllocation->dwFlags == RENDERHAL_KERNEL_ALLOCATION_FREE ||
        pKernelAllocation->iSize   == 0)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Kernel Allocation.");
        goto finish;
    }

    // Check Curbe allocation (CURBE_Lenght is in 256-bit count -> convert to bytes)
    iCurbeSize = iCurbeLength;
    if (iCurbeSize <= 0)
    {
        // Curbe is not used by the kernel
        iCurbeSize = iCurbeOffset = 0;
    }
    // Validate Curbe Offset (curbe must be pre-allocated)
    else if ( iCurbeOffset < 0 ||                                       // Not allocated
             (iCurbeOffset & 0x1F) != 0 ||                              // Invalid alignment
             (iCurbeOffset + iCurbeSize) > pCurMediaState->iCurbeOffset)   // Invalid size
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Curbe Allocation.");
        goto finish;
    }

    // Search available ID in current media state heap, try to reuse index (2nd level BB reuse)
    iInterfaceDescriptor = pRenderHal->pfnGetMediaID(pRenderHal, pCurMediaState, pKernelAllocation);
    if (iInterfaceDescriptor < 0)
    {
            MHW_RENDERHAL_ASSERTMESSAGE("No Interface Descriptor available.");
            goto finish;
        }

    MOS_ZeroMemory((void *)&InterfaceDescriptorParams, sizeof(InterfaceDescriptorParams));

    InterfaceDescriptorParams.iMediaID            = iInterfaceDescriptor;
    InterfaceDescriptorParams.iBindingTableID     = iBindingTableID;
    InterfaceDescriptorParams.iCurbeOffset        = iCurbeOffset;
    InterfaceDescriptorParams.iCurbeLength        = iCurbeLength;
    InterfaceDescriptorParams.iCrsThrdConstDataLn = iCrsThrdConstDataLn;

    // barrier and slm
    if (pGpGpuWalkerParams && pGpGpuWalkerParams->GpGpuEnable)
    {
        InterfaceDescriptorParams.blBarrierEnable = true;
        InterfaceDescriptorParams.iNumberThreadsInGroup = pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight;
        InterfaceDescriptorParams.iSLMSize = pGpGpuWalkerParams->SLMSize; // The shift will be handled by pfnEncodeSLMSize()
        InterfaceDescriptorParams.blGlobalBarrierEnable = false;
    }
    else //Reset barrier and slm setting since they may be set before
    {
        InterfaceDescriptorParams.blBarrierEnable = false;
        InterfaceDescriptorParams.iNumberThreadsInGroup = pRenderHal->dwMinNumberThreadsInGroup;
        InterfaceDescriptorParams.iSLMSize = 0;
        InterfaceDescriptorParams.iCrsThrdConstDataLn &= pRenderHal->dwMaskCrsThdConDataRdLn;
        InterfaceDescriptorParams.blGlobalBarrierEnable = false;
    }

    // Setup Media ID entry - this call could be HW dependent
    if (MOS_STATUS_SUCCESS != pRenderHal->pfnSetupInterfaceDescriptor(
                                              pRenderHal,
                                              pCurMediaState,
                                              pKernelAllocation,
                                              &InterfaceDescriptorParams))
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Failed to setup interface descriptor.");
        goto finish;
    }

    // Update kernel usage
    pRenderHal->pfnTouchKernel(pRenderHal, iInterfaceDescriptor);

finish:
    return iInterfaceDescriptor;
}

int32_t RenderHal_GetMediaID(
    PRENDERHAL_INTERFACE        pRenderHal,
    PRENDERHAL_MEDIA_STATE      pMediaState,
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation)
{
    int32_t    iInterfaceDescriptor = -1;
    int32_t    *Allocation;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pMediaState->piAllocation);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);

    iInterfaceDescriptor = pKernelAllocation->iKID;
    Allocation           = pMediaState->piAllocation;

    // Try to reuse interface descriptor (for 2nd level buffer optimizations)
    // Check if ID already in use by another kernel - must use a different ID
    if (iInterfaceDescriptor >= 0 &&
        Allocation[iInterfaceDescriptor] >= 0 &&
        Allocation[iInterfaceDescriptor] != pKernelAllocation->iAllocIndex)
    {
        iInterfaceDescriptor = -1;
    }

    // Search available ID in current media state heap
    if (iInterfaceDescriptor < 0)
    {
        int32_t iMax = pRenderHal->StateHeapSettings.iMediaIDs;
        for (iInterfaceDescriptor = 0;
             iInterfaceDescriptor < iMax;
             iInterfaceDescriptor++)
        {
            if (Allocation[iInterfaceDescriptor] < 0)
            {
                break;
            }
        }

        // All IDs are in use - fail
        if (iInterfaceDescriptor >= iMax)
        {
            MHW_RENDERHAL_ASSERT("No Interface Descriptor available.");
            iInterfaceDescriptor = -1;
            goto finish;
        }
    }

    // Set kernel allocation for the current Media ID
    Allocation[iInterfaceDescriptor] = pKernelAllocation->iAllocIndex;

    // Set preferred Media ID for the current kernel
    // This is necessary for 2nd level BB optimization.
    if (pKernelAllocation->iKID < 0)
    {
        pKernelAllocation->iKID = iInterfaceDescriptor;
    }

finish:
    return iInterfaceDescriptor;
}

uint32_t RenderHal_SetSurfacesPerBT(PRENDERHAL_INTERFACE pRenderHal, uint32_t dwSurfacesPerBT)
{
    if (pRenderHal == nullptr)
    {
        MHW_RENDERHAL_ASSERT("Invalid renderhal interface.");
        return dwSurfacesPerBT;
    }

    // Set number of surfaces per Binding Table
    uint32_t dwSizeBT = (uint32_t)pRenderHal->StateHeapSettings.iBTAlignment/pRenderHal->pRenderHalPltInterface->GetBTStateCmdSize();
    while (dwSizeBT < dwSurfacesPerBT) dwSizeBT <<= 1;
    dwSurfacesPerBT = dwSizeBT;

    // Reconfigure the binding table size
    pRenderHal->pStateHeap->iBindingTableSize = MOS_ALIGN_CEIL(dwSizeBT * pRenderHal->pRenderHalPltInterface->GetBTStateCmdSize(),
                                                                  pRenderHal->StateHeapSettings.iBTAlignment);

    // Reconfigure the binding table number, surface per BT
    pRenderHal->StateHeapSettings.iBindingTables = pRenderHal->StateHeapSettings.iBindingTables *
                                                   pRenderHal->StateHeapSettings.iSurfacesPerBT / dwSurfacesPerBT;
    pRenderHal->StateHeapSettings.iSurfacesPerBT = (int32_t) dwSurfacesPerBT;

    return dwSurfacesPerBT;
}

//!
//! \brief    Assign Surface State
//! \details  Assign a new surface state from already allocated surfState pool
//!           Setup the surface state to default
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    RENDERHAL_SURFACE_STATE_TYPE Type
//!           [in] Surface State Type
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntry
//!           [in] Pointer to Surface State Entry
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_AssignSurfaceState(
    PRENDERHAL_INTERFACE            pRenderHal,
    RENDERHAL_SURFACE_STATE_TYPE    Type,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntry)
{
    PRENDERHAL_STATE_HEAP           pStateHeap;
    int32_t                         iSurfaceEntry;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    uint32_t                        dwOffset;
    MOS_STATUS                      eStatus;

    //----------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(ppSurfaceEntry);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //----------------------------------------------

    eStatus    = MOS_STATUS_UNKNOWN;
    pStateHeap = pRenderHal->pStateHeap;

    if (pStateHeap->iCurrentSurfaceState >= pRenderHal->StateHeapSettings.iSurfaceStates)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Unable to allocate Surface State. Exceeds Maximum.");
        goto finish;
    }

    // Calculate the Offset to the Surface State
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);
    dwOffset = pStateHeap->iSurfaceStateOffset +
               (pStateHeap->iCurrentSurfaceState *
                pRenderHal->pRenderHalPltInterface->GetSurfaceStateCmdSize()); // Moves the pointer to a Currently assigned Surface State

    // Obtain new surface entry and initialize
    iSurfaceEntry                       = pStateHeap->iCurrentSurfaceState;
    pSurfaceEntry                       = &pStateHeap->pSurfaceEntry[iSurfaceEntry];
    if (pSurfaceEntry->pSurface)
    {
        MOS_SafeFreeMemory(pSurfaceEntry->pSurface);
        pSurfaceEntry->pSurface = nullptr;
    }
    *pSurfaceEntry                      = g_cInitSurfaceStateEntry;

    // Setup Surface Entry parameters
    pSurfaceEntry->iSurfStateID         = iSurfaceEntry;
    pSurfaceEntry->Type                 = Type;
    pSurfaceEntry->dwSurfStateOffset    = (uint32_t)-1;                         // Each platform to setup
    pSurfaceEntry->pSurfaceState        = pStateHeap->pSshBuffer + dwOffset;
    pSurfaceEntry->pSurface             = (PMOS_SURFACE)MOS_AllocAndZeroMemory(sizeof(MOS_SURFACE));
    if (pSurfaceEntry->pSurface == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Allocating Surface failed!");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }

    *ppSurfaceEntry                     = pSurfaceEntry;

    // Increment the Current Surface State Entry
    ++pStateHeap->iCurrentSurfaceState;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Get DI UV Offset
//! \details  Get DI UV Offset
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Surface
//! \param    uint16_t *pUXOffset
//!           [out] X Offset for U Plane
//! \param    uint16_t *pUYOffset
//!           [out] Y Offset for U Plane
//! \param    uint16_t *pVXOffset
//!           [out] X Offset for V Plane
//! \param    uint16_t *pVYOffset
//!           [out] Y Offset for V Plane
//! \return   void
//!
void RenderHal_Get_DI_UVOffSet(
    PRENDERHAL_SURFACE pRenderHalSurface,
    uint16_t           *pUXOffset,
    uint16_t           *pUYOffset,
    uint16_t           *pVXOffset,
    uint16_t           *pVYOffset)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pUXOffset);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pUYOffset);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pVXOffset);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pVXOffset);

    PMOS_SURFACE pSurface = &pRenderHalSurface->OsSurface;
    MOS_FORMAT fmt = pSurface->Format;
    uint32_t dwNumRowsFromTopV = 0;
    uint32_t dwNumRowsFromTopU = 0;
    uint32_t dwNumColsFromLeftV = 0;
    uint32_t dwNumColsFromLeftU = 0;
    uint32_t dwHeight = pSurface->dwHeight;
    uint32_t dwPitch  = pSurface->dwPitch;

    switch (fmt)
    {
        //  YY
        //  YY
        //  V-
        //  U-
        case Format_IMC1:
            dwNumRowsFromTopV = dwHeight;
            dwNumRowsFromTopU = dwHeight + (dwHeight >> 1);
            break;

        //  YY
        //  YY
        //  VU
        case Format_IMC2:
            dwNumRowsFromTopV = dwNumRowsFromTopU = dwHeight;
            dwNumColsFromLeftU = dwPitch >> 1;
            break;

        //  YY
        //  YY
        //  U-
        //  V-
        case Format_IMC3:
            dwNumRowsFromTopU = dwHeight;
            dwNumRowsFromTopV = dwHeight + (dwHeight >> 1);
            break;

        //  YY
        //  YY
        //  UV
        case Format_IMC4:
            dwNumRowsFromTopU = dwNumRowsFromTopV = dwHeight;
            dwNumColsFromLeftV = dwPitch >> 1;
            break;

        //  YY
        //  YY
        //  U
        //  V
        case Format_I420:
        case Format_IYUV:
            dwNumRowsFromTopU = dwHeight;
            dwNumRowsFromTopV = dwHeight + (dwHeight >> 1);
            break;

        //  YY
        //  YY
        //  V
        //  U
        case Format_YV12:
            dwNumRowsFromTopU = dwHeight + (dwHeight >> 1);
            dwNumRowsFromTopV = dwHeight;
            break;

        //  YYYY
        //  YYYY
        //  YYYY
        //  YYYY
        //  V
        //  U
        case Format_YVU9:
            dwNumRowsFromTopU = dwHeight;
            dwNumRowsFromTopV = dwHeight + (dwHeight >> 2);
            break;

        //  NV12                P208
        //  ----                ----
        //  YY                  YY
        //  YY                  UV (interleaved)
        //  UV (interleaved)
        case Format_NV12:
        case Format_P208:
        case Format_P016:
        case Format_P010:
            dwNumRowsFromTopU = dwNumRowsFromTopV = dwHeight;
            break;

        //  NV11
        //  ----
        //  YYYY
        //  UV (interleaved)
        case Format_NV11:
            dwNumRowsFromTopU = dwNumRowsFromTopV = dwHeight;
            break;

       default:
           MHW_RENDERHAL_ASSERTMESSAGE("called with Packed or Unknown format.");
           break;
    }

    // the Offsets must be even numbers so we round down
    dwNumRowsFromTopU = MOS_ALIGN_FLOOR(dwNumRowsFromTopU, 2);
    dwNumColsFromLeftU = MOS_ALIGN_FLOOR(dwNumColsFromLeftU, 2);
    dwNumRowsFromTopV = MOS_ALIGN_FLOOR(dwNumRowsFromTopV, 2);
    dwNumColsFromLeftV = MOS_ALIGN_FLOOR(dwNumColsFromLeftV, 2);

    *pVYOffset = (uint16_t)dwNumRowsFromTopV;
    *pUYOffset = (uint16_t)dwNumRowsFromTopU;
    *pVXOffset = (uint16_t)dwNumColsFromLeftV;
    *pUXOffset = (uint16_t)dwNumColsFromLeftU;
}

//!
//! \brief    Set Slice Shutdown Mode
//! \details  Sets Slice Shutdown Mode
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    bool bMode
//!           [in] Slice Shutdown Mode
//! \return   void
//!
void RenderHal_SetSliceShutdownMode(
    PRENDERHAL_INTERFACE     pRenderHal,
    bool                     bMode)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    pRenderHal->bRequestSingleSlice = bMode;
}

//!
//! \brief    Set General Slice Shutdown Mode
//! \details  Sets General Slice Shutdown Mode
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PRENDERHAL_POWEROPTION pMode
//!           [in] Slice Shutdown Mode
//! \return   void
//!
void RenderHal_SetPowerOptionMode(
    PRENDERHAL_INTERFACE     pRenderHal,
    PRENDERHAL_POWEROPTION   pMode )
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pMode);
    pRenderHal->PowerOption = *pMode;
}

//!
//! \brief    Enable Middle Batch Buffer Preemption
//! \details  Enable Middle Batch Buffer Preemption
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structurev
//! \return   void
//!
void RenderHal_EnableGpgpuMiddleBatchBufferPreemption(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    pRenderHal->bEnableGpgpuMidBatchPreEmption = MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrGpGpuMidBatchPreempt) ? true : false;
}

//!
//! \brief    Enable Middle Thread Preemption
//! \details  Enable Middle Thread Preemption
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structurev
//! \return   void
//!
void RenderHal_EnableGpgpuMiddleThreadPreemption(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    pRenderHal->bEnableGpgpuMidThreadPreEmption = MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrGpGpuMidThreadLevelPreempt) ? true : false;
}

//!
//! \brief    Get Surface State Entries
//! \details  Gets the Surface State Entries
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!           [in] Pointer to Surface State Params
//! \param    int32_t *piNumEntries
//!           [out] Number of Surface Entries (one entry per surface plane)
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntries
//!           [out] Array of Surface State Entries
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_GetSurfaceStateEntries(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams,
    int32_t                         *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries)
{
    MOS_STATUS                      eStatus;
    PCMHW_PLANE_SETTING             pPlane;
    PMOS_SURFACE                    pSurface;
    RENDERHAL_PLANE_DEFINITION      PlaneDefinition;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    uint32_t                        dwSurfaceWidth;
    uint32_t                        dwSurfaceHeight;
    uint32_t                        dwUVPitch;
    int32_t                         i;
    bool                            bHalfPitchForChroma;
    bool                            bInterleaveChroma;
    bool                            bWidthInDword;
    uint8_t                         Direction;
    uint16_t                        wUXOffset;
    uint16_t                        wUYOffset;
    uint16_t                        wVXOffset;
    uint16_t                        wVYOffset;
    bool                            bIsChromaSitEnabled;

    //------------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL(pParams);
    MHW_RENDERHAL_CHK_NULL(piNumEntries);
    MHW_RENDERHAL_CHK_NULL(ppSurfaceEntries);
    //------------------------------------------------

    eStatus             = MOS_STATUS_UNKNOWN;
    pSurface            = &pRenderHalSurface->OsSurface;
    dwUVPitch           = pSurface->dwPitch;
    bHalfPitchForChroma = false;
    bInterleaveChroma   = false;
    Direction           = MEDIASTATE_VDIRECTION_FULL_FRAME;
    wUXOffset           = 0;
    wUYOffset           = 0;
    wVXOffset           = 0;
    wVYOffset           = 0;
    *piNumEntries       = -1;
    PlaneDefinition     = RENDERHAL_PLANES_DEFINITION_COUNT;
    bIsChromaSitEnabled = 0;

    pRenderHal->bIsAVS  = pParams->bAVS;

    // Check palette allocations
    if (IS_PAL_FORMAT(pSurface->Format))
    {
        if (pRenderHalSurface->iPaletteID < 0 ||
            pRenderHalSurface->iPaletteID >= pRenderHal->iMaxPalettes)
        {
            MHW_RENDERHAL_ASSERTMESSAGE("Invalid palette.");
            goto finish;
        }
    }

    // adjust the U/V-Pitch for formats that have U/V-Pitch smaller than Y-Pitch
    if (pSurface->Format == Format_I420 ||
        pSurface->Format == Format_IYUV ||
        pSurface->Format == Format_YV12 ||
        pSurface->Format == Format_NV11)
    {
        dwUVPitch >>= 1;
    }
    else if (pSurface->Format == Format_YVU9)
    {
        dwUVPitch >>= 2;
    }

    if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G8      ||
        pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G9      ||
        pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G10)
    {
        // Select the surface/plane description
        switch (pSurface->Format)
        {
            case Format_NV12:
                // On G8, NV12 format needs the width and Height to be a multiple
                // of 4 for both 3D sampler and 8x8 sampler; G75 needs the width
                // of NV12 input surface to be a multiple of 4 for 3D sampler;
                // G9+ does not has such restriction; to simplify the implementation,
                // we enable 2 plane NV12 for all of the platform when the width
                // or Height is not a multiple of 4
                if (pRenderHal->pfnIs2PlaneNV12Needed(pRenderHal,
                                                      pRenderHalSurface,
                                                      pParams->Boundary))
                {
                    PlaneDefinition = RENDERHAL_PLANES_NV12_2PLANES_ADV;
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_NV12_ADV;
                    wUYOffset       = RenderHal_CalculateYOffset(pRenderHal->pOsInterface, &pSurface->OsResource);
                }

                bHalfPitchForChroma = false;
                bInterleaveChroma   = true;

                // Set up chroma direction
                Direction           = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                break;

            case Format_P208:
                PlaneDefinition = RENDERHAL_PLANES_P208_1PLANE_ADV;
                break;

            case Format_IMC1:
            case Format_IMC2:
            case Format_IMC3:
            case Format_IMC4:
            case Format_I420:
            case Format_IYUV:
            case Format_YV12:
            case Format_YVU9:
                PlaneDefinition     = RENDERHAL_PLANES_PL3_ADV;
                bHalfPitchForChroma = false;

                if (pSurface->Format == Format_I420 ||
                    pSurface->Format == Format_IYUV ||
                    pSurface->Format == Format_YV12 ||
                    pSurface->Format == Format_NV11)
                {
                    bHalfPitchForChroma = true;
                }

                // Set up chroma direction
                Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);

                if (!pParams->bAVS)
                {
                    // Get U/V offset for PL3 DNDI
                    RenderHal_Get_DI_UVOffSet(pRenderHalSurface,
                                      &wUXOffset,
                                      &wUYOffset,
                                      &wVXOffset,
                                      &wVYOffset);
                    PlaneDefinition = RENDERHAL_PLANES_NV12_ADV;
                }
                break;

            case Format_400P:
                // Single Y plane here is treated like a NV12 surface.
                // U and V offsets fall inside this Y plane. Eventhough false UV pixels are
                // picked by the kernel, CSC coeffecients are such that the effect of these
                // are nullified.
                PlaneDefinition     = RENDERHAL_PLANES_NV12_ADV;
                break;

            case Format_411P:
                PlaneDefinition     = RENDERHAL_PLANES_411P_ADV;
                break;

            case Format_411R:
                PlaneDefinition     = RENDERHAL_PLANES_411R_ADV;
                break;

            case Format_422H:
                PlaneDefinition     = RENDERHAL_PLANES_422H_ADV;
                break;

            case Format_422V:
                PlaneDefinition     = RENDERHAL_PLANES_422V_ADV;
                break;

            case Format_444P:
                PlaneDefinition     = RENDERHAL_PLANES_444P_ADV;
                break;

            case Format_RGBP:
                PlaneDefinition     = RENDERHAL_PLANES_RGBP_ADV;
                break;

            case Format_BGRP:
                PlaneDefinition     = RENDERHAL_PLANES_BGRP_ADV;
                break;

            case Format_AYUV:
                PlaneDefinition = RENDERHAL_PLANES_AYUV_ADV;
                break;

            case Format_YUYV:
            case Format_YUY2:
                if (pParams->bVmeUse)
                {
                    //Since 422 planar is not supported on application side. 
                    //App is using 422 packed as WA with w=w/2 and h=h*2
                    pSurface->dwWidth = pSurface->dwWidth * 2;
                    pSurface->dwHeight = pSurface->dwHeight / 2;
                    pRenderHalSurface->rcSrc.right = pSurface->dwWidth;
                    pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
                    pRenderHalSurface->rcDst = pRenderHalSurface->rcSrc;
                    PlaneDefinition = RENDERHAL_PLANES_YUY2_ADV;

                    // Set up chroma direction
                    Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_YUY2_ADV;

                    // Set up chroma direction
                    Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                }
                break;

            case Format_UYVY:
                PlaneDefinition     = RENDERHAL_PLANES_UYVY_ADV;

                // Set up chroma direction
                Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                break;

            case Format_YVYU:
                PlaneDefinition     = RENDERHAL_PLANES_YVYU_ADV;

                // Set up chroma direction
                Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                break;

            case Format_VYUY:
                PlaneDefinition     = RENDERHAL_PLANES_VYUY_ADV;

                // Set up chroma direction
                Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                break;

            case Format_A8R8G8B8:
            case Format_X8R8G8B8:
                if (pParams->bVASurface)
                {
                    pSurface->dwWidth *= 32;
                    PlaneDefinition     = RENDERHAL_PLANES_Y1;
                }
                else
                {
                    PlaneDefinition     = RENDERHAL_PLANES_ARGB_ADV;
                }
                break;

            case Format_R8G8SN:
                if ( pParams->bVASurface )
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y16S;
                }
                break;

            case Format_V8U8:
                if ( pParams->bVASurface )
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y16U;
                }
                break;

            case Format_A8B8G8R8:
            case Format_X8B8G8R8:
                PlaneDefinition     = RENDERHAL_PLANES_ABGR_ADV;
                break;

            case Format_STMM:
                PlaneDefinition     = RENDERHAL_PLANES_STMM_ADV;
                break;

            case Format_A8:
            case Format_Buffer_2D:
                if (pParams->bVASurface)
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y8;
                }
                break;
            case Format_Y8:
                PlaneDefinition = RENDERHAL_PLANES_Y8_ADV;
                break;

            case Format_L8:
            case Format_R8UN:
                if (pParams->bForceNV12)
                {
                    PlaneDefinition     = RENDERHAL_PLANES_NV12_ADV;
                    bHalfPitchForChroma = false;
                    bInterleaveChroma   = true;
                    wUYOffset           = (uint16_t) pSurface->dwHeight;
                }
                else
                {
                    PlaneDefinition     = RENDERHAL_PLANES_L8_ADV;
                }
                break;

            case Format_A16B16G16R16:
            case Format_Y416:
                PlaneDefinition        = RENDERHAL_PLANES_A16B16G16R16_ADV;
                break;

            case Format_R10G10B10A2:
            case Format_Y410:
                PlaneDefinition        = RENDERHAL_PLANES_R10G10B10A2_ADV;
                break;

            case Format_L16:
            case Format_R16S:
                if (pParams->bVASurface)
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y16S;
                }
                break;

            case Format_D16:
            case Format_R16U:
                if (pParams->bVASurface)
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y16U;
                }
                break;

            case Format_P010:
            case Format_P016:
                if (pParams->bVmeUse)
                {
                    PlaneDefinition = RENDERHAL_PLANES_P010_1PLANE_ADV;
                }
                else if (pRenderHal->bEnableP010SinglePass &&
                    (pRenderHalSurface->SurfType != RENDERHAL_SURF_OUT_RENDERTARGET))
                {
                    PlaneDefinition     = RENDERHAL_PLANES_P010_1PLANE_ADV;
                    bHalfPitchForChroma = false;
                    bInterleaveChroma   = true;
                    wUYOffset           = RenderHal_CalculateYOffset(pRenderHal->pOsInterface, &pSurface->OsResource);

                    // Set up chroma direction
                    Direction = pRenderHal->pfnSetChromaDirection(pRenderHal, pRenderHalSurface);
                }
                else
                {
                    // Format not supported with AVS - use regular format
                    MHW_RENDERHAL_NORMALMESSAGE("Format not supported with AVS.");

                    pParams->bAVS = false;

                    // Since the format is not supported with AVS, set the scaling mode as bilinear.
                    pRenderHalSurface->ScalingMode = RENDERHAL_SCALING_BILINEAR;

                    if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G8)
                    {
                        pParams->Type = RENDERHAL_SURFACE_TYPE_G8;
                    }
                    else if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G9)
                    {
                        pParams->Type = RENDERHAL_SURFACE_TYPE_G9;
                    }
                    else if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G10)
                    {
                        pParams->Type = RENDERHAL_SURFACE_TYPE_G10;
                    }
                    else
                    {
                        MHW_RENDERHAL_ASSERTMESSAGE("Unsupported surface type");
                    }
                }
                break;
            case Format_Y210:
            case Format_Y216:
                if (pParams->bVmeUse)
                {
                    //Since 422 planar is not supported on application side. 
                    //App is using 422 packed as WA with w=w/2 and h=h*2
                    pSurface->dwWidth = pSurface->dwWidth * 2;
                    pSurface->dwHeight = pSurface->dwHeight / 2;
                    pRenderHalSurface->rcSrc.right = pSurface->dwWidth;
                    pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
                    pRenderHalSurface->rcDst = pRenderHalSurface->rcSrc;
                    PlaneDefinition = RENDERHAL_PLANES_Y210_1PLANE_ADV;
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y210_ADV;
                }
                break;
            default:
                // Format not supported with AVS - use regular format
                MHW_RENDERHAL_NORMALMESSAGE("Format not supported with AVS.");

                pParams->bAVS = false;

                // Since the format is not supported with AVS, set the scaling mode as bilinear.
                pRenderHalSurface->ScalingMode = RENDERHAL_SCALING_BILINEAR;

                if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G8)
                {
                    pParams->Type = RENDERHAL_SURFACE_TYPE_G8;
                }
                else if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G9)
                {
                    pParams->Type = RENDERHAL_SURFACE_TYPE_G9;
                }
                else if (pParams->Type == RENDERHAL_SURFACE_TYPE_ADV_G10)
                {
                    pParams->Type = RENDERHAL_SURFACE_TYPE_G10;
                }
                else
                {
                    MHW_RENDERHAL_ASSERTMESSAGE("Unsupported surface type");
                }

                break;
        }
    }

    // Select the surface/plane description
    if (pParams->Type == RENDERHAL_SURFACE_TYPE_G8  ||
        pParams->Type == RENDERHAL_SURFACE_TYPE_G9  ||
        pParams->Type == RENDERHAL_SURFACE_TYPE_G10)
    {
        bIsChromaSitEnabled = pRenderHal->pRenderHalPltInterface->IsChromasitingEnabled(pRenderHal, pParams);

        switch (pSurface->Format)
        {
            case Format_IMC1:
            case Format_IMC2:
            case Format_IMC3:
            case Format_IMC4:
            case Format_I420:
            case Format_IYUV:
            case Format_YVU9:
                PlaneDefinition = RENDERHAL_PLANES_PL3;
                break;

            case Format_YV12:
                bHalfPitchForChroma = true;

                // Y_Uoffset(Height*2 + Height/2) of RENDERHAL_PLANES_YV12 define Bitfield_Range(0, 13) on gen9+.
                // The max value is 16383. So use PL3 kernel to avoid out of range when Y_Uoffset is larger than 16383.
                // Use PL3 plane to avoid YV12 blending issue with DI enabled and U channel shift issue with not 4-aligned height
                PlaneDefinition = (pRenderHal->bEnableYV12SinglePass                              &&
                                   !pRenderHalSurface->pDeinterlaceParams                         &&
                                   !pRenderHalSurface->bInterlacedScaling                         &&
                                   MOS_IS_ALIGNED(pSurface->dwHeight, 4)                          &&
                                   pRenderHalSurface->SurfType != RENDERHAL_SURF_OUT_RENDERTARGET &&
                                   (pSurface->dwHeight * 2 + pSurface->dwHeight / 2) < RENDERHAL_MAX_YV12_PLANE_Y_U_OFFSET_G9)?
                                   RENDERHAL_PLANES_YV12 : RENDERHAL_PLANES_PL3;
                break;

            case Format_400P:
                // Single Y plane here is treated like a NV12 surface.
                // U and V offsets fall inside this Y plane. Eventhough false UV pixels are
                // picked by the kernel, CSC coeffecients are such that the effect of these
                // are nullified.
                PlaneDefinition = RENDERHAL_PLANES_NV12;
                break;

            case Format_P208:
                PlaneDefinition = RENDERHAL_PLANES_P208;
                break;

            case Format_P010:
            case Format_P016:
                if (pRenderHal->bEnableP010SinglePass &&
                    (pRenderHalSurface->SurfType != RENDERHAL_SURF_OUT_RENDERTARGET))
                {
                    PlaneDefinition = RENDERHAL_PLANES_P010_1PLANE;
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_P010;
                }
                break;

            case Format_411P:
                PlaneDefinition = RENDERHAL_PLANES_411P;
                break;

            case Format_411R:
                PlaneDefinition = RENDERHAL_PLANES_411R;
                break;

            case Format_422H:
                PlaneDefinition = RENDERHAL_PLANES_422H;
                break;

            case Format_422V:
                PlaneDefinition = RENDERHAL_PLANES_422V;
                break;

            case Format_444P:
                PlaneDefinition = RENDERHAL_PLANES_444P;
                break;

            case Format_RGBP:
                PlaneDefinition = RENDERHAL_PLANES_RGBP;
                break;

            case Format_BGRP:
                PlaneDefinition = RENDERHAL_PLANES_BGRP;
                break;

            case Format_NV12:
                // On Gen7.5 (Haswell) NV12 format needs a single plane instead
                // of two (listed in renderhal_g75.c for RENDERHAL_PLANES_NV12),  and
                // is also expected by the Sampler or Media Kernels. Yet, the
                // Data Port works with two planes instead. Besides, the Sampler
                // uses it for input only (as there is no output) while the Data
                // Port uses it for input as well as output or both for the same
                // surface. Hence the check added for bWidthInDword_Y &&
                // bWidthInDword_UV, which are set in vphal_render_3P.c for the
                // above reason. Two plane NV12 can also be explicitly spcified.

                // On G8, NV12 format needs the width and Height to be a multiple
                // of 4 for both 3D sampler and 8x8 sampler; G75 needs the width
                // of NV12 input surface to be a multiple of 4 for 3D sampler;
                // On G9+, width need to be a multiple of 2, while height still need
                // be a multiple of 4; since G9 already post PV, just keep the old logic
                // to enable 2 plane NV12 when the width or Height is not a multiple of 4.
                // For G10+, enable 2 plane NV12 when width is not multiple of 2 or height
                // is not multiple of 4.
                if ( pRenderHalSurface->SurfType == RENDERHAL_SURF_OUT_RENDERTARGET   ||
                     (pParams->bWidthInDword_Y && pParams->bWidthInDword_UV)          ||
                     pParams->b2PlaneNV12NeededByKernel                               ||
                     bIsChromaSitEnabled ||
                     pRenderHal->pfnIs2PlaneNV12Needed(pRenderHal,
                                                       pRenderHalSurface,
                                                       pParams->Boundary))
                {
                    PlaneDefinition = RENDERHAL_PLANES_NV12_2PLANES;
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_NV12;
                }
                break;

            case Format_YUYV    :
            case Format_YUY2    :
                if (bIsChromaSitEnabled)
                {
                    PlaneDefinition = RENDERHAL_PLANES_YUY2_2PLANES;
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_YUY2;
                }
                break;

            case Format_G8R8_G8B8:
            case Format_UYVY     :
                PlaneDefinition = RENDERHAL_PLANES_UYVY;
                break;

            case Format_YVYU:
                PlaneDefinition = RENDERHAL_PLANES_YVYU;
                break;

            case Format_VYUY:
                PlaneDefinition = RENDERHAL_PLANES_VYUY;
                break;

            case Format_A8R8G8B8:
                PlaneDefinition = RENDERHAL_PLANES_ARGB;
                break;

            case Format_R32U:
                PlaneDefinition = RENDERHAL_PLANES_R32U;
                break;

            case Format_R32S:
                PlaneDefinition = RENDERHAL_PLANES_R32S;
                break;

            case Format_R32F:
            case Format_D32F:
            case Format_R32:
                PlaneDefinition = RENDERHAL_PLANES_R32F;
                break;

            case Format_Y8:
                PlaneDefinition = RENDERHAL_PLANES_R8;
                break;

            case Format_Y1:
                PlaneDefinition = RENDERHAL_PLANES_Y1;
                break;

            case Format_Y16U:
                PlaneDefinition = RENDERHAL_PLANES_Y16U;
                break;

            case Format_Y16S:
                PlaneDefinition = RENDERHAL_PLANES_Y16S;
                break;

            case Format_R8G8SN:
            case Format_V8U8:
                PlaneDefinition = RENDERHAL_PLANES_V8U8;
                break;

            case Format_R16U:
                PlaneDefinition = RENDERHAL_PLANES_R16U;
                break;

            case Format_R16S:
                PlaneDefinition = RENDERHAL_PLANES_R16S;
                break;

            case Format_R8G8UN:
                PlaneDefinition = RENDERHAL_PLANES_R8G8_UNORM;
                break;

            case Format_X8R8G8B8:
                // h/w doesn't support XRGB render target
                PlaneDefinition =
                    (pParams->bRenderTarget) ? RENDERHAL_PLANES_ARGB : RENDERHAL_PLANES_XRGB;
                break;

            case Format_A8B8G8R8:
                PlaneDefinition = RENDERHAL_PLANES_ABGR;
                break;

            case Format_X8B8G8R8:
                // h/w doesn't support XBGR render target
                PlaneDefinition =
                    (pParams->bRenderTarget) ? RENDERHAL_PLANES_ABGR : RENDERHAL_PLANES_XBGR;
                break;

            case Format_R5G6B5:
                PlaneDefinition = RENDERHAL_PLANES_RGB16;
                break;

            case Format_R8G8B8:
                PlaneDefinition = RENDERHAL_PLANES_RGB24;
                break;

            case Format_AYUV    :
                PlaneDefinition = RENDERHAL_PLANES_AYUV;
                break;

            case Format_AI44    :
                PlaneDefinition = (pRenderHalSurface->iPaletteID == 0) ?
                                                RENDERHAL_PLANES_AI44_PALLETE_0 :
                                                RENDERHAL_PLANES_AI44_PALLETE_1;
                break;

            case Format_IA44:
                PlaneDefinition = (pRenderHalSurface->iPaletteID == 0) ?
                                                RENDERHAL_PLANES_IA44_PALLETE_0 :
                                                RENDERHAL_PLANES_IA44_PALLETE_1;
                break;

            case Format_P8:
                PlaneDefinition = (pRenderHalSurface->iPaletteID == 0) ?
                                                RENDERHAL_PLANES_P8_PALLETE_0 :
                                                RENDERHAL_PLANES_P8_PALLETE_1;
                break;

            case Format_A8P8:
                PlaneDefinition = (pRenderHalSurface->iPaletteID == 0) ?
                                                RENDERHAL_PLANES_A8P8_PALLETE_0 :
                                                RENDERHAL_PLANES_A8P8_PALLETE_1;
                break;

            case Format_STMM:
                PlaneDefinition = RENDERHAL_PLANES_STMM;
                break;

            case Format_L8:
                PlaneDefinition = RENDERHAL_PLANES_L8;
                break;

            case Format_A8:
            case Format_Buffer_2D:
                PlaneDefinition = RENDERHAL_PLANES_A8;
                break;

            case Format_R8U:
            case Format_R8UN:
                PlaneDefinition = RENDERHAL_PLANES_R8;
                break;

            case Format_R16UN:
            case Format_D16:
            case Format_R16:
                PlaneDefinition = RENDERHAL_PLANES_R16_UNORM;
                break;

            case Format_A16B16G16R16:
                PlaneDefinition = RENDERHAL_PLANES_A16B16G16R16;
                break;
            case Format_Y416:
                if (pRenderHalSurface->SurfType == RENDERHAL_SURF_OUT_RENDERTARGET)
                {
                    PlaneDefinition = RENDERHAL_PLANES_Y416_RT;
                }
                else
                {
                    PlaneDefinition = RENDERHAL_PLANES_A16B16G16R16;
                }
                break;
            case Format_A16B16G16R16F:
                PlaneDefinition = RENDERHAL_PLANES_A16B16G16R16F;
                break;
            case Format_A16R16G16B16F:
                PlaneDefinition = RENDERHAL_PLANES_A16R16G16B16F;
                break;
            case Format_R32G32B32A32F:
                PlaneDefinition = RENDERHAL_PLANES_R32G32B32A32F;
                break;

            case Format_NV21:
                PlaneDefinition = RENDERHAL_PLANES_NV21;
                break;
            case Format_L16:
                PlaneDefinition = RENDERHAL_PLANES_L16;
                break;

            case Format_R10G10B10A2:
            case Format_Y410:
                PlaneDefinition = RENDERHAL_PLANES_R10G10B10A2;
                break;

            case Format_Y210:
            case Format_Y216:
                MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->GetPlaneDefForFormatY216(
                    (pRenderHalSurface->SurfType == RENDERHAL_SURF_OUT_RENDERTARGET),
                    pRenderHal,
                    PlaneDefinition));
                break;

            case Format_B10G10R10A2:
                PlaneDefinition = RENDERHAL_PLANES_B10G10R10A2;
                break;

            case Format_IRW0:
                PlaneDefinition = RENDERHAL_PLANES_IRW0;
                break;

            case Format_IRW1:
                PlaneDefinition = RENDERHAL_PLANES_IRW1;
                break;

            case Format_IRW2:
                PlaneDefinition = RENDERHAL_PLANES_IRW2;
                break;

            case Format_IRW3:
                PlaneDefinition = RENDERHAL_PLANES_IRW3;
                break;

            case Format_R16G16UN:
                PlaneDefinition = RENDERHAL_PLANES_R16G16_UNORM;
                break;

            case Format_R16G16S:
                PlaneDefinition = RENDERHAL_PLANES_R16G16_SINT;
                break;

            case Format_R16F:
                PlaneDefinition = RENDERHAL_PLANES_R16_FLOAT;
                break;

            case Format_R24G8:
            case Format_D24S8UN:
                PlaneDefinition = RENDERHAL_PLANES_R24_UNORM_X8_TYPELESS;
                break;

            case Format_R32G8X24:
            case Format_D32S8X24_FLOAT:
                PlaneDefinition = RENDERHAL_PLANES_R32_FLOAT_X8X24_TYPELESS;
                break;

            default:
                goto finish;
        }
    }

    // Get plane definitions
    MHW_RENDERHAL_ASSERT(PlaneDefinition < RENDERHAL_PLANES_DEFINITION_COUNT);
    *piNumEntries   = pRenderHal->pPlaneDefinitions[PlaneDefinition].dwNumPlanes;
    pPlane          = pRenderHal->pPlaneDefinitions[PlaneDefinition].Plane;
    if (*piNumEntries == 0)
    {
        goto finish;
    }

    // Surface state allocation/setting loop
    for (i = 0; i < *piNumEntries; i++, pPlane++)
    {
        // Assign a New Surface State Entry
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnAssignSurfaceState(pRenderHal,
                                                   pParams->Type,
                                                   &pSurfaceEntry));

        // Save surface entry
        ppSurfaceEntries[i] = pSurfaceEntry;

        // Adjust the surface height and width
        pRenderHal->pfnAdjustBoundary(pRenderHal,
                                        pRenderHalSurface,
                                        pParams->Boundary,
                                        &dwSurfaceWidth,
                                        &dwSurfaceHeight);

        //set dwSurfaceHeight = roundup(dwSurfaceHeight / pPlane->ui8ScaleHeight) to support NV12 format with odd height
        dwSurfaceHeight = (dwSurfaceHeight + pPlane->ui8ScaleHeight - 1) / pPlane->ui8ScaleHeight;
        dwSurfaceWidth  = dwSurfaceWidth  / pPlane->ui8ScaleWidth;

        // U/V/UV plane
        if (pPlane->ui8PlaneID == MHW_U_PLANE ||
            pPlane->ui8PlaneID == MHW_V_PLANE)
        {
            bWidthInDword = pParams->bWidthInDword_UV;
        }
        else
        {
            bWidthInDword = pParams->bWidthInDword_Y;
        }

        // Adjust the width
        if (bWidthInDword)
        {
            if (PlaneDefinition == RENDERHAL_PLANES_R32G32B32A32F)
            {
                dwSurfaceWidth = dwSurfaceWidth << 2;
            }
            else if (PlaneDefinition == RENDERHAL_PLANES_A16B16G16R16     ||
                PlaneDefinition == RENDERHAL_PLANES_A16B16G16R16_ADV ||
                PlaneDefinition == RENDERHAL_PLANES_A16B16G16R16F    ||
                PlaneDefinition == RENDERHAL_PLANES_A16R16G16B16F    ||
                PlaneDefinition == RENDERHAL_PLANES_Y210_RT          ||
                PlaneDefinition == RENDERHAL_PLANES_Y416_RT          ||
                PlaneDefinition == RENDERHAL_PLANES_R32_FLOAT_X8X24_TYPELESS)
            {
                dwSurfaceWidth = dwSurfaceWidth << 1;
            }
            else
            {
                dwSurfaceWidth = (dwSurfaceWidth + pPlane->ui8PixelsPerDword - 1) /
                                                        pPlane->ui8PixelsPerDword;
            }
        }

        if (pParams->bVertStride)
        {
            dwSurfaceHeight /= 2;
            dwSurfaceHeight = MOS_MAX(dwSurfaceHeight, 1);
        }

        dwSurfaceHeight = MOS_ALIGN_FLOOR(dwSurfaceHeight, pPlane->ui8AlignHeight);
        dwSurfaceWidth  = MOS_ALIGN_FLOOR(dwSurfaceWidth , pPlane->ui8AlignWidth);

        // Setup surface state entry
        *(pSurfaceEntry->pSurface)   = *pSurface;
        pSurfaceEntry->dwFormat      = pPlane->dwFormat;
        pSurfaceEntry->dwWidth       = MOS_MAX(1, dwSurfaceWidth);
        pSurfaceEntry->dwHeight      = MOS_MAX(1, dwSurfaceHeight);
        pSurfaceEntry->bWidthInDword = bWidthInDword;

        if (pPlane->ui8PlaneID == MHW_U_PLANE ||
            pPlane->ui8PlaneID == MHW_V_PLANE)
        {
            pSurfaceEntry->dwPitch       = dwUVPitch;
        }
        else
        {
            pSurfaceEntry->dwPitch       = pSurface->dwPitch;
        }

        pSurfaceEntry->dwQPitch          = pSurface->dwQPitch;

        pSurfaceEntry->YUVPlane          = pPlane->ui8PlaneID;
        pSurfaceEntry->bAVS              = pPlane->bAdvanced;
        pSurfaceEntry->bRenderTarget     = pParams->bRenderTarget;
        pSurfaceEntry->bVertStride       = pParams->bVertStride;
        pSurfaceEntry->bVertStrideOffs   = pParams->bVertStrideOffs;
        pSurfaceEntry->bTiledSurface     = (pSurface->TileType != MOS_TILE_LINEAR)
                                                ? true
                                                : false;
        pSurfaceEntry->bTileWalk         = IS_Y_MAJOR_TILE_FORMAT(pSurface->TileType)
                                                ? GFX3DSTATE_TILEWALK_YMAJOR
                                                : GFX3DSTATE_TILEWALK_XMAJOR;
        // AVS/ADI parameters
        pSurfaceEntry->bHalfPitchChroma  = bHalfPitchForChroma;
        pSurfaceEntry->bInterleaveChroma = bInterleaveChroma;
        pSurfaceEntry->DirectionV        = Direction & 0x7;
        pSurfaceEntry->DirectionU        = Direction >> 0x3;
        pSurfaceEntry->wUXOffset         = wUXOffset;
        pSurfaceEntry->wUYOffset         = wUYOffset;
        pSurfaceEntry->wVXOffset         = wVXOffset;
        pSurfaceEntry->wVYOffset         = wVYOffset;
        pSurfaceEntry->AddressControl    = pParams->AddressControl;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Enable Palette
//! \details  Enable HW palette - reuse previous palette data
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint32_t uiPaletteID
//!           [in] Palette ID (0 to available palettes - 1)
//! \param    int32_t iPaletteSize
//!           [in] Palette Size (<=0 - disable palette)
//! \return   MOS_STATUS
//!           Error code if invalid parameters, MOS_STATUS_SUCCESS otherwise
//!
MOS_STATUS RenderHal_EnablePalette(
    PRENDERHAL_INTERFACE    pRenderHal,
    int32_t                 iPaletteID,
    int32_t                 iPaletteSize)
{
    PMHW_PALETTE_PARAMS      pPalette;

    //-------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_ASSERT(iPaletteID >= 0);
    MHW_RENDERHAL_ASSERT(iPaletteID < pRenderHal->iMaxPalettes);
    //-------------------------------------

    // Palette ID provided invalid or allocation failed
    //pHwCommands = pRenderHal->pHwCommands;

    // Set palette size - 0 disables palette send command
    iPaletteSize = MOS_MAX(iPaletteSize, 0);
    iPaletteSize = MOS_MIN(iPaletteSize, pRenderHal->iMaxPaletteEntries);
    pPalette = &(pRenderHal->Palette[iPaletteID]);
    pPalette->iNumEntries = iPaletteSize;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Allocate Palette ID
//! \details  Allocate palette ID for Client
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface structure
//! \param    int32_t *pPaletteID
//!           [out] Pointer to Palette ID
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS, otherwise MOS_STATUS_NO_SPACE or MOS_STATUS_NULL_POINTER
//!
MOS_STATUS RenderHal_AllocatePaletteID(
                            PRENDERHAL_INTERFACE    pRenderHal,
                            int32_t                 *pPaletteID)
{
    int32_t                 i;
    PMHW_PALETTE_PARAMS     pOutPalette;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pPaletteID);

    *pPaletteID = -1;

    pOutPalette = pRenderHal->Palette;
    // search first palette not in use
    for (i = 0;
         i < pRenderHal->iMaxPalettes;
         i++, pOutPalette++)
    {
        if (pOutPalette->iNumEntries == 0)
        {
            pOutPalette->iNumEntries = -1;
            break;
        }
    }

    // Allocation failed
    if (i < 0 || i >= pRenderHal->iMaxPalettes)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("cannot find valid palette ID.");
        eStatus = MOS_STATUS_NO_SPACE;
        goto finish;
    }
    else
    {
        *pPaletteID = i;
    }

finish:
    return eStatus;
}

//!
//! \brief    Get Palette Entry Size and Address
//! \details  Get Palette Entry Size and Address for Client to load palette
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface structure
//! \param    int32_t iPaletteID
//!           [in] Input Palette ID
//! \param    int32_t iInNumEntries
//!           [in] Number of Input Palette entries
//! \param    int32_t *piOutNumEntries
//!           [out] Number of Output Palette entries
//! \param    void  **pPaletteData
//!           [out] Pointer to Output Palette data address
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS, otherwise MOS_STATUS_INVALID_PARAMETER or MOS_STATUS_NULL_POINTER
//!
MOS_STATUS RenderHal_GetPaletteEntry(
    PRENDERHAL_INTERFACE    pRenderHal,
    int32_t                 iPaletteID,
    int32_t                 iInNumEntries,
    int32_t                 *piOutNumEntries,
    void                    **pPaletteData)
{
    PMHW_PALETTE_PARAMS     pOutPalette;
    int32_t                 iSize;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(piOutNumEntries);
    MHW_RENDERHAL_CHK_NULL(pPaletteData);

    *piOutNumEntries    = -1;
    *pPaletteData       = nullptr;
    // Palette Index provided invalid
    if (iPaletteID < 0 || iPaletteID >= pRenderHal->iMaxPalettes)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("invalid palette ID.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Input Palette entry number invalid
    if (iInNumEntries < 1)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("invalid Input Palette entries.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    // Get pointer to output palette
    pOutPalette = &(pRenderHal->Palette[iPaletteID]);

    // Pallete is too large - truncate
    if (iInNumEntries > pRenderHal->iMaxPaletteEntries)
    {
        iSize = pRenderHal->iMaxPaletteEntries;
        MHW_RENDERHAL_ASSERTMESSAGE("Palette truncated from %d to %d.", iInNumEntries, iSize);
    }
    else
    {
        iSize = iInNumEntries;
    }
    // Pallete is being overwritten - just log
    if (pOutPalette->iNumEntries != 0 &&
        pOutPalette->iNumEntries != -1)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("overwriting palette %d.", iPaletteID);
    }

    pOutPalette->iNumEntries    = iSize;
    *piOutNumEntries            = iSize;
    *pPaletteData               = pOutPalette->pPaletteData;

finish:
    return eStatus;
}
//!
//! \brief    Free Palette ID
//! \details  Free palette ID
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface structure
//! \param    int32_t *pPaletteID
//!           [in/out] Pointer to Palette ID
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS, otherwise MOS_STATUS_INVALID_PARAMETER or MOS_STATUS_NULL_POINTER
//!
MOS_STATUS RenderHal_FreePaletteID(
                            PRENDERHAL_INTERFACE    pRenderHal,
                            int32_t                 *pPaletteID)
{
    PMHW_PALETTE_PARAMS     pOutPalette;
    int32_t                 iPaletteID;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pPaletteID);

    iPaletteID      = *pPaletteID;
    if (iPaletteID < 0 || iPaletteID >= pRenderHal->iMaxPalettes)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("invalid palette ID.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    pOutPalette                 = &(pRenderHal->Palette[iPaletteID]);
    pOutPalette->iNumEntries    = 0;
    *pPaletteID                 = -1;

finish:
    return eStatus;
}

//!
//! \brief    Allocate ChromaKey
//! \details  Allocate chroma key for use with sampler
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint32_t dwLow
//!           Min Key Range
//! \param    uint32_t dwHigh
//!           Max Key Range
//! \return   int32_t
//!           Chroma key index
//!           -1 if not available
//!
int32_t RenderHal_AllocateChromaKey(
    PRENDERHAL_INTERFACE     pRenderHal,
    uint32_t                 dwLow,
    uint32_t                 dwHigh)
{
    PMHW_CHROMAKEY_PARAMS    pChromaKey;
    int32_t                  iChromaKeyIndex = -1;

    // Validate parameters
    if (pRenderHal == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid RenderHal interface.");
        goto finish;
    }

    if (pRenderHal->iChromaKeyCount > pRenderHal->iMaxChromaKeys)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Reached max number of chroma keys.");
        goto finish;
    }

    // Get chroma index - setup command
    iChromaKeyIndex = pRenderHal->iChromaKeyCount++;
    pChromaKey = &pRenderHal->ChromaKey[iChromaKeyIndex];
    pChromaKey->dwLow  = dwLow;
    pChromaKey->dwHigh = dwHigh;

finish:
    // Return Chroma Key Index
    return iChromaKeyIndex;
}

//!
//! \brief    Assign Media State
//! \details  Gets a pointer to the next available media state in GSH;
//!           fails if not available
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hadrware Interface Structure
//! \param    RENDERHAL_COMPONENT componentID
//!           [in] Identifier of the requesting component
//! \return   PRENDERHAL_MEDIA_STATE
//!           gets a new Media State, returns pointer to Media State structure
//!           nullptr - invalid, no states available + timeout
//!
PRENDERHAL_MEDIA_STATE RenderHal_AssignMediaState(
    PRENDERHAL_INTERFACE     pRenderHal,
    RENDERHAL_COMPONENT      componentID)
{
    uint32_t                dwWaitMs, dwWaitTag;
    PMOS_INTERFACE          pOsInterface = nullptr;   // OS interface
    PRENDERHAL_STATE_HEAP   pStateHeap   = nullptr;   // State Heap control struct
    PRENDERHAL_MEDIA_STATE  pCurMediaState;        // Media state control in GSH struct
    uint8_t                 *pCurrentPtr;
    int                     i;

    pCurMediaState = nullptr;
    pCurrentPtr    = nullptr;

    if (pRenderHal)
    {
        pOsInterface = pRenderHal->pOsInterface;
        pStateHeap   = pRenderHal->pStateHeap;
    }

    // Validate state
    if (pRenderHal   == nullptr ||             // invalid Hw state
        pOsInterface == nullptr ||             // invalid OS interface
        pStateHeap   == nullptr ||             // invalid State Heap
        pStateHeap->pMediaStates == nullptr || // invalid Media State Array
        pStateHeap->bGshLocked == false ||  // State Heap not locked
        pRenderHal->StateHeapSettings.iMediaStateHeaps == 0)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid state.");
        goto finish;
    }

    // Refresh sync tag for all media states
    pRenderHal->pfnRefreshSync(pRenderHal);

    // Get next media state and tag to check
    pCurMediaState = &pStateHeap->pMediaStates[pStateHeap->iNextMediaState];

    // The code below is unlikely to be executed - unless all media states are in use
    // If this ever happens, please consider increasing the number of media states
    if (pCurMediaState->bBusy)
    {
        dwWaitTag   = pCurMediaState->dwSyncTag;

        // Wait for Batch Buffer complete event OR timeout
        for (dwWaitMs = pRenderHal->dwTimeoutMs; dwWaitMs > 0; dwWaitMs--)
        {
            //MOS_GPU_CONTEXT_RENDER or MOS_GPU_CONTEXT_RENDER3
            pOsInterface->pfnWaitForBBCompleteNotifyEvent(pOsInterface, pOsInterface->CurrentGpuContextOrdinal, RENDERHAL_EVENT_TIMEOUT_MS);

            // Wait for tag (end of command buffer, start of next command buffer)
            if ((int32_t)(pStateHeap->pSync[0] - dwWaitTag) >  0) break;
        }

        // Timeout
        if (dwWaitMs == 0)
        {
            MHW_RENDERHAL_ASSERTMESSAGE("Timeout for waiting free media state.");
            pStateHeap->pCurMediaState = pCurMediaState = nullptr;
            goto finish;
        }
    }

    // Setup the Current Media State
    pStateHeap->pCurMediaState    = pCurMediaState;
    pStateHeap->iCurMediaState    = pStateHeap->iNextMediaState;

    // Point to the next media state
    pStateHeap->iNextMediaState   = (pStateHeap->iNextMediaState + 1) %
                              (pRenderHal->StateHeapSettings.iMediaStateHeaps);

    // Reset media state
    pCurMediaState->dwSyncTag    = pStateHeap->dwNextTag;
    pCurMediaState->dwSyncCount  = 0;
    pCurMediaState->iCurbeOffset = 0;
    MOS_FillMemory( pCurMediaState->piAllocation,
                pRenderHal->StateHeapSettings.iMediaIDs * sizeof(int32_t),
                -1);

    // Reset HW allocations
    pRenderHal->iChromaKeyCount = 0;
    for (i = 0; i < pRenderHal->iMaxPalettes; i++)
    {
        pRenderHal->Palette[i].iNumEntries = 0;
    }

    // Zero Memory start time and end time
    pCurrentPtr = pStateHeap->pGshBuffer +           // GSH base
                  pStateHeap->pCurMediaState->dwOffset +
                  pStateHeap->dwOffsetStartTime;

    if(pCurrentPtr)
    {
        // Start time
        *((uint64_t*)pCurrentPtr) = 0;
    }

    // End time
    pCurrentPtr += pStateHeap->dwStartTimeSize;
    *((uint64_t*)pCurrentPtr) = 0;

    // Component ID
    pCurrentPtr += pStateHeap->dwEndTimeSize;
    *((RENDERHAL_COMPONENT *)pCurrentPtr) = componentID;

finish:
    return pCurMediaState;
}

//!
//! \brief    Destroy
//! \details  Free all resources allocated by RenderHal
//! \param    PRENDERHAL_INTERFACE pRenderHal
//! \return   void
//!
MOS_STATUS RenderHal_Destroy(PRENDERHAL_INTERFACE pRenderHal)
{
    MOS_STATUS            eStatus;

    //------------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //------------------------------------------------
    eStatus      = MOS_STATUS_UNKNOWN;

    // Free State Heaps
    MHW_CHK_STATUS(pRenderHal->pfnFreeStateHeaps(pRenderHal));

    // Destroy MHW Render Interface
    if (pRenderHal->pMhwRenderInterface)
    {
        MOS_Delete(pRenderHal->pMhwRenderInterface);
        pRenderHal->pMhwRenderInterface = nullptr;
    }

    // Destroy MHW MI Interface
    if (pRenderHal->pMhwMiInterface)
    {
        MOS_Delete(pRenderHal->pMhwMiInterface);
        pRenderHal->pMhwMiInterface = nullptr;
    }

    // Release pBatchBufferMemPool
    if (pRenderHal->pBatchBufferMemPool)
    {
        MOS_Delete(pRenderHal->pBatchBufferMemPool);
        pRenderHal->pBatchBufferMemPool = nullptr;
    }

    // Release PredicationBuffer
    if (!Mos_ResourceIsNull(&pRenderHal->PredicationBuffer))
    {
        pRenderHal->pOsInterface->pfnFreeResource(
            pRenderHal->pOsInterface,
            &pRenderHal->PredicationBuffer);
    }

    // Destruct Platform Interface
    if (pRenderHal->pRenderHalPltInterface)
    {
        MOS_Delete(pRenderHal->pRenderHalPltInterface);
        pRenderHal->pRenderHalPltInterface = nullptr;
    }

    if (pRenderHal->pPerfProfiler)
    {
       MediaPerfProfiler::Destroy(pRenderHal->pPerfProfiler, (void*)pRenderHal, pRenderHal->pOsInterface);
       pRenderHal->pPerfProfiler = nullptr;
    }

    // Free multiple trackers
    pRenderHal->trackerProducer.~FrameTrackerProducer();

    // Free Debug Surface
    RenderHal_FreeDebugSurface(pRenderHal);

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Load Curbe Data
//! \details  Allocates and load CURBE data for Media
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in]  Pointer to RenderHal Interface structure
//! \param    PRENDERHAL_MEDIA_STATE pCurMediaState
//!           [out] Pointer to Current Media State structure
//! \param    void  *pData
//!           [in]  Pointer to Data
//! \param    int32_t iSize
//!           [in]  Number of bytes to allocate
//! \return   int32_t
//!           Offset of the CURBE block from CURBE base (in bytes)
//!           -1 if no CURBE space available in GSH
//!
int32_t RenderHal_LoadCurbeData(
    PRENDERHAL_INTERFACE pRenderHal,
    PRENDERHAL_MEDIA_STATE  pCurMediaState,
    void                *pData,
    int32_t             iSize)
{
    int32_t               iOffset;
    int32_t               iCurbeSize;
    uint8_t               *pPtrCurbe;
    PRENDERHAL_STATE_HEAP pStateHeap;

    iOffset    = -1;
    pStateHeap = (pRenderHal) ? pRenderHal->pStateHeap : nullptr;
    if (pStateHeap && pCurMediaState)
    {
        iCurbeSize = MOS_ALIGN_CEIL(iSize, pRenderHal->dwCurbeBlockAlign);
        if (pCurMediaState->iCurbeOffset + iCurbeSize <= (int)pStateHeap->dwSizeCurbe)
        {
            iOffset = pCurMediaState->iCurbeOffset;
            pCurMediaState->iCurbeOffset += iCurbeSize;

            if (pData)
            {
                pPtrCurbe = pStateHeap->pGshBuffer +                // GSH base
                            pStateHeap->pCurMediaState->dwOffset +  // Offset to media state
                            pStateHeap->dwOffsetCurbe +             // Offset to curbe area
                            iOffset;                                // Current curbe offset

                // Copy data to CURBE
                MOS_SecureMemcpy(pPtrCurbe, iSize, pData, iSize);

                // Zero remaining CURBE (for buffer alignment)
                iCurbeSize -= iSize;
                if (iCurbeSize > 0)
                {
                    MOS_ZeroMemory(pPtrCurbe + iSize, iCurbeSize);
                }
            }
        }
    }

    return iOffset;
}

//!
//! \brief    Send Curbe Load
//! \details  Send Curbe Load command
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendCurbeLoad(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MHW_CURBE_LOAD_PARAMS CurbeLoadParams;
    PRENDERHAL_STATE_HEAP pStateHeap;
    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE        pOsInterface = nullptr;
    MOS_CONTEXT           *pOsContext = nullptr;
    MOS_OCA_BUFFER_HANDLE hOcaBuf = 0;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface->pOsContext);
    //-----------------------------------------

    eStatus                 = MOS_STATUS_SUCCESS;
    pStateHeap              = pRenderHal->pStateHeap;
    pOsInterface            = pRenderHal->pOsInterface;
    pOsContext              = pOsInterface->pOsContext;

    // CURBE size is in bytes
    if (pStateHeap->pCurMediaState->iCurbeOffset != 0)
    {
        CurbeLoadParams.pKernelState            = nullptr;
        CurbeLoadParams.bOldInterface           = false;
        CurbeLoadParams.dwCURBETotalDataLength  = pStateHeap->pCurMediaState->iCurbeOffset;
        CurbeLoadParams.dwCURBEDataStartAddress = pStateHeap->pCurMediaState->dwOffset + pStateHeap->dwOffsetCurbe;

        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddMediaCurbeLoadCmd(pCmdBuffer, &CurbeLoadParams));

        HalOcaInterface::OnIndirectState(*pCmdBuffer, *pOsContext,pRenderHal->StateBaseAddressParams.presDynamicState,
            CurbeLoadParams.dwCURBEDataStartAddress, false, CurbeLoadParams.dwCURBETotalDataLength);
    }

finish:
    return eStatus;
}

MOS_STATUS RenderHal_SendMediaIdLoad(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MHW_ID_LOAD_PARAMS    IdLoadParams;
    PRENDERHAL_STATE_HEAP pStateHeap;
    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE        pOsInterface = nullptr;
    MOS_CONTEXT           *pOsContext = nullptr;
    MOS_OCA_BUFFER_HANDLE hOcaBuf = 0;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface->pOsContext);
    //-----------------------------------------

    eStatus                 = MOS_STATUS_SUCCESS;
    pStateHeap              = pRenderHal->pStateHeap;
    pOsInterface            = pRenderHal->pOsInterface;
    pOsContext              = pOsInterface->pOsContext;

    IdLoadParams.pKernelState                     = nullptr;
    IdLoadParams.dwInterfaceDescriptorStartOffset = pStateHeap->pCurMediaState->dwOffset +  pStateHeap->dwOffsetMediaID;
    IdLoadParams.dwInterfaceDescriptorLength      = pRenderHal->StateHeapSettings.iMediaIDs * pStateHeap->dwSizeMediaID;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwRenderInterface->AddMediaIDLoadCmd(pCmdBuffer, &IdLoadParams));

    HalOcaInterface::OnIndirectState(*pCmdBuffer, *pOsContext, pRenderHal->StateBaseAddressParams.presDynamicState,
        IdLoadParams.dwInterfaceDescriptorStartOffset, false, IdLoadParams.dwInterfaceDescriptorLength);

finish:
    return eStatus;
 }

//!
//! \brief    Send Chroma Key
//! \details  Sends Chroma Key
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendChromaKey(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MhwRenderInterface           *pMhwRender;
    PMHW_CHROMAKEY_PARAMS        pChromaKeyParams;
    int32_t                      i;
    MOS_STATUS                   eStatus;

    //----------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    //----------------------------------

    eStatus     = MOS_STATUS_SUCCESS;
    pMhwRender  = pRenderHal->pMhwRenderInterface;

    // Send Chroma Keys in use
    pChromaKeyParams = pRenderHal->ChromaKey;
    for (i = pRenderHal->iChromaKeyCount; i > 0; i--, pChromaKeyParams++)
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddChromaKeyCmd(pCmdBuffer, pChromaKeyParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Send Palette
//! \details  Sends Palette
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendPalette(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    MhwRenderInterface           *pMhwRender;
    PMHW_PALETTE_PARAMS          pPaletteLoadParams;
    int32_t                      i;
    MOS_STATUS                   eStatus;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    //-----------------------------------------

    eStatus     = MOS_STATUS_SUCCESS;
    pMhwRender  = pRenderHal->pMhwRenderInterface;

    // Send Palettes in use
    pPaletteLoadParams = pRenderHal->Palette;
    for (i = pRenderHal->iMaxPalettes; i > 0; i--, pPaletteLoadParams++)
    {
        if (pPaletteLoadParams->iNumEntries > 0)
        {
            MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddPaletteLoadCmd(pCmdBuffer, pPaletteLoadParams));
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Reset RenderHal States
//! \details  Reset RenderHal States in preparation for a new command buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_Reset(
    PRENDERHAL_INTERFACE pRenderHal)
{
    PMOS_INTERFACE          pOsInterface;
    PRENDERHAL_STATE_HEAP   pStateHeap;
    MOS_STATUS              eStatus;

    //----------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //----------------------------------

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = pRenderHal->pOsInterface;
    pStateHeap      = pRenderHal->pStateHeap;

    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface,
                                            &pStateHeap->GshOsResource,
                                            true,
                                            true));

    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface,
                                            &pStateHeap->IshOsResource,
                                            true,
                                            true));

    // Reset Slice Shutdown Mode
    pRenderHal->bRequestSingleSlice   = false;
    pRenderHal->PowerOption.nSlice    = 0;
    pRenderHal->PowerOption.nEU       = 0;
    pRenderHal->PowerOption.nSubSlice = 0;

finish:
    return eStatus;
}

//!
//! \brief    Assign Ssh Instance
//! \details  Get a pointer to the next available SSH Buffer Instance in SSH,
//!           Reset SSH allocations
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if succeded, MOS_STATUS_UNKOWN otherwise
//!
MOS_STATUS RenderHal_AssignSshInstance(
    PRENDERHAL_INTERFACE     pRenderHal)
{
    MOS_STATUS eStatus;
    PRENDERHAL_STATE_HEAP pStateHeap;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //-----------------------------------------

    eStatus    = MOS_STATUS_SUCCESS;
    pStateHeap = pRenderHal->pStateHeap;

    // Init SSH Params
    if (pStateHeap)
    {
        pStateHeap->iCurrentBindingTable = 0;
        pStateHeap->iCurrentSurfaceState = 0;
    }
    else
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

finish:
    return eStatus;
}

//!
//! \brief    Adds predication attributes in command buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pcmdBuffer
//!           [in] Pointer to Command Buffer 
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendPredicationCommand(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface->GetMmioRegisters());
    //-----------------------------------------

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS  condBBEndParams;
    MOS_ZeroMemory(&condBBEndParams, sizeof(condBBEndParams));

    MOS_SYNC_PARAMS syncParams;
    MOS_ZeroMemory(&syncParams, sizeof(syncParams));
    syncParams.uiSemaphoreCount         = 1;
    // Currently only sync between VEBOX and 3D, also need to consider sync between Render Engine and 3D
    // low priority since current VP Predication test case does not cover this scenario.
    syncParams.GpuContext               = MOS_GPU_CONTEXT_VEBOX;
    syncParams.presSyncResource         = pRenderHal->PredicationParams.pPredicationResource;
    syncParams.bReadOnly                = true;
    syncParams.bDisableDecodeSyncLock   = false;
    syncParams.bDisableLockForTranscode = false;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pOsInterface->pfnPerformOverlaySync(pRenderHal->pOsInterface, &syncParams));
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pOsInterface->pfnResourceWait(pRenderHal->pOsInterface, &syncParams));

    // This function is only support VEBox right now, since register returned by pMhwMiInterface->GetMmioRegisters() is for VEBox.
    // Keep implementation same between Render and VEBox engines - for Render it is highly inefficient
    // Skip current frame if presPredication is not equal to zero
    if (pRenderHal->PredicationParams.predicationNotEqualZero)
    {
        auto mmioRegistersRender = pRenderHal->pMhwMiInterface->GetMmioRegisters();
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        // load presPredication to general purpose register0
        MHW_MI_STORE_REGISTER_MEM_PARAMS    loadRegisterMemParams;
        MOS_ZeroMemory(&loadRegisterMemParams, sizeof(loadRegisterMemParams));
        loadRegisterMemParams.presStoreBuffer   = pRenderHal->PredicationParams.pPredicationResource;
        loadRegisterMemParams.dwOffset          = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        loadRegisterMemParams.dwRegister        = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterMemCmd(
            pCmdBuffer,
            &loadRegisterMemParams));

        MHW_MI_LOAD_REGISTER_IMM_PARAMS     loadRegisterImmParams;
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData            = 0;
        loadRegisterImmParams.dwRegister        = mmioRegistersRender->generalPurposeRegister0HiOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        // load 0 to general purpose register4
        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData            = 0;
        loadRegisterImmParams.dwRegister        = mmioRegistersRender->generalPurposeRegister4LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        MOS_ZeroMemory(&loadRegisterImmParams, sizeof(loadRegisterImmParams));
        loadRegisterImmParams.dwData            = 0;
        loadRegisterImmParams.dwRegister        = mmioRegistersRender->generalPurposeRegister4HiOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(
            pCmdBuffer,
            &loadRegisterImmParams));

        //perform the add operation
        MHW_MI_MATH_PARAMS  miMathParams;
        MHW_MI_ALU_PARAMS   miAluParams[4];
        MOS_ZeroMemory(&miMathParams, sizeof(miMathParams));
        MOS_ZeroMemory(&miAluParams, sizeof(miAluParams));
        // load     srcA, reg0
        miAluParams[0].AluOpcode    = MHW_MI_ALU_LOAD;
        miAluParams[0].Operand1     = MHW_MI_ALU_SRCA;
        miAluParams[0].Operand2     = MHW_MI_ALU_GPREG0;
        // load     srcB, reg4
        miAluParams[1].AluOpcode    = MHW_MI_ALU_LOAD;
        miAluParams[1].Operand1     = MHW_MI_ALU_SRCB;
        miAluParams[1].Operand2     = MHW_MI_ALU_GPREG4;
        // add      srcA, srcB
        miAluParams[2].AluOpcode    = MHW_MI_ALU_ADD;
        miAluParams[2].Operand1     = MHW_MI_ALU_SRCB;
        miAluParams[2].Operand2     = MHW_MI_ALU_GPREG4;
        // store      reg0, ZF
        miAluParams[3].AluOpcode    = MHW_MI_ALU_STORE;
        miAluParams[3].Operand1     = MHW_MI_ALU_GPREG0;
        miAluParams[3].Operand2     = MHW_MI_ALU_ZF;
        miMathParams.pAluPayload    = miAluParams;
        miMathParams.dwNumAluParams = 4; // four ALU commands needed for this substract opertaion. see following ALU commands.
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiMathCmd(
            pCmdBuffer,
            &miMathParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer  = &pRenderHal->PredicationBuffer;
        storeRegParams.dwOffset         = 0x10;
        storeRegParams.dwRegister       = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreRegisterMemCmd(
            pCmdBuffer,
            &storeRegParams));

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        MHW_MI_STORE_DATA_PARAMS dataParams;
        MOS_ZeroMemory(&dataParams, sizeof(dataParams));
        dataParams.pOsResource = &pRenderHal->PredicationBuffer;
        dataParams.dwValue = 1;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 2;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 3;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 4;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.postSyncOperation = 1;
        flushDwParams.pOsResource = &pRenderHal->PredicationBuffer;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        condBBEndParams.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        condBBEndParams.dwOffset            = 0x10;
        condBBEndParams.dwValue             = 0;
        condBBEndParams.bDisableCompareMask = true;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiConditionalBatchBufferEndCmd(
            pCmdBuffer,
            &condBBEndParams));

        pRenderHal->PredicationParams.ptempPredicationBuffer = &pRenderHal->PredicationBuffer;
    }
    else
    {
        auto mmioRegistersRender = pRenderHal->pMhwMiInterface->GetMmioRegisters();

        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        // load presPredication to general purpose register0
        MHW_MI_STORE_REGISTER_MEM_PARAMS    loadRegisterMemParams;
        MOS_ZeroMemory(&loadRegisterMemParams, sizeof(loadRegisterMemParams));
        loadRegisterMemParams.presStoreBuffer = pRenderHal->PredicationParams.pPredicationResource;
        loadRegisterMemParams.dwOffset        = (uint32_t)pRenderHal->PredicationParams.predicationResOffset;
        loadRegisterMemParams.dwRegister      = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterMemCmd(
            pCmdBuffer,
            &loadRegisterMemParams));

        // if zero, the zero flag will be 0xFFFFFFFF, else zero flag will be 0x0.
        MHW_MI_STORE_REGISTER_MEM_PARAMS    storeRegParams;
        MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
        storeRegParams.presStoreBuffer = &pRenderHal->PredicationBuffer;
        storeRegParams.dwOffset        = 0x10;
        storeRegParams.dwRegister      = mmioRegistersRender->generalPurposeRegister0LoOffset;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreRegisterMemCmd(
            pCmdBuffer,
            &storeRegParams));

        // Programming of 4 dummy MI_STORE_DATA_IMM commands prior to programming of MiConditionalBatchBufferEnd
        MHW_MI_STORE_DATA_PARAMS dataParams;
        MOS_ZeroMemory(&dataParams, sizeof(dataParams));
        dataParams.pOsResource = &pRenderHal->PredicationBuffer;
        dataParams.dwValue     = 1;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 2;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 3;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        dataParams.dwValue = 4;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiStoreDataImmCmd(
            pCmdBuffer,
            &dataParams));

        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.postSyncOperation = 1;
        flushDwParams.pOsResource       = &pRenderHal->PredicationBuffer;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(pCmdBuffer, &flushDwParams));

        // Skip current frame if presPredication is equal to zero
        condBBEndParams.presSemaphoreBuffer = &pRenderHal->PredicationBuffer;
        condBBEndParams.dwOffset            = 0x10;
        condBBEndParams.bDisableCompareMask = true;
        condBBEndParams.dwValue             = 0;
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiConditionalBatchBufferEndCmd(
            pCmdBuffer,
            &condBBEndParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Adds marker attributes in command buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pcmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    bool isRender
//!           [in] Flag of Render Engine
//! \return   MOS_STATUS 
//!
MOS_STATUS RenderHal_SendMarkerCommand(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     cmdBuffer,
    bool                    isRender)
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    //-----------------------------------------

    if (isRender)
    {
        // Send pipe_control to get the timestamp
        MHW_PIPE_CONTROL_PARAMS             pipeControlParams;
        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.presDest          = pRenderHal->SetMarkerParams.pSetMarkerResource;
        pipeControlParams.dwResourceOffset  = 0;
        pipeControlParams.dwPostSyncOp      = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        pipeControlParams.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;

        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddPipeControl(cmdBuffer, NULL, &pipeControlParams));
    }
    else
    {
        // Send flush_dw to get the timestamp 
        MHW_MI_FLUSH_DW_PARAMS  flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.pOsResource           = pRenderHal->SetMarkerParams.pSetMarkerResource;
        flushDwParams.dwResourceOffset      = 0;
        flushDwParams.postSyncOperation     = MHW_FLUSH_WRITE_TIMESTAMP_REG;
        flushDwParams.bQWordEnable          = 1;

        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Initializes command buffer attributes and inserts prolog
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParam
//!           [in] Pointer to MHW generic prolog parameters
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_InitCommandBuffer(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS  pGenericPrologParams)
{
    PMOS_INTERFACE              pOsInterface;
    MHW_GENERIC_PROLOG_PARAMS   genericPrologParams;
    MOS_STATUS                  eStatus;
    MEDIA_SYSTEM_INFO           *pGtSystemInfo;
    bool                        isRender;

    //---------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);
    //---------------------------------------------

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = pRenderHal->pOsInterface;
    pGtSystemInfo   = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pGtSystemInfo);

    // Send Start Marker command
    isRender = MOS_RCS_ENGINE_USED(pOsInterface->pfnGetGpuContext(pOsInterface));
    if (pRenderHal->SetMarkerParams.setMarkerEnabled)
    {
        MHW_RENDERHAL_CHK_STATUS(RenderHal_SendMarkerCommand(
            pRenderHal, pCmdBuffer, isRender));
    }

    // Init Cmd Buffer
#ifdef _MMC_SUPPORTED
    if (isRender)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->SetCompositePrologCmd(pRenderHal, pCmdBuffer));
    }
#endif // _MMC_SUPPORTED

    // Set indirect heap size - limits the size of the command buffer available for rendering
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnSetIndirectStateSize(pOsInterface, pRenderHal->dwIndirectHeapSize));

    pCmdBuffer->Attributes.bIsMdfLoad = pRenderHal->IsMDFLoad;
    pCmdBuffer->Attributes.bTurboMode = pRenderHal->bTurboMode;

    // Set power option status
    if (pRenderHal->pRenderHalPltInterface)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->SetPowerOptionStatus(pRenderHal, pCmdBuffer));
    }

    // Preemption: Need to set UsesMediaPipeline, UsesGPGPUPipeline, NeedsMidBatchPreEmptionSupport in command buffer header
    // Use IsMDFLoad to distinguish MDF context from other Media Contexts
    pCmdBuffer->Attributes.bMediaPreemptionEnabled =
        (pRenderHal->bEnableGpgpuMidBatchPreEmption ||
        pRenderHal->bEnableGpgpuMidThreadPreEmption ||
        pRenderHal->pMhwRenderInterface->IsPreemptionEnabled());

    if (pGenericPrologParams)
    {
        if (pGenericPrologParams->bEnableMediaFrameTracking)
        {
            MHW_RENDERHAL_CHK_NULL(pGenericPrologParams->presMediaFrameTrackingSurface);
            pCmdBuffer->Attributes.bEnableMediaFrameTracking = pGenericPrologParams->bEnableMediaFrameTracking;
            pCmdBuffer->Attributes.dwMediaFrameTrackingTag = pGenericPrologParams->dwMediaFrameTrackingTag;
            pCmdBuffer->Attributes.dwMediaFrameTrackingAddrOffset = pGenericPrologParams->dwMediaFrameTrackingAddrOffset;
            pCmdBuffer->Attributes.resMediaFrameTrackingSurface = *(pGenericPrologParams->presMediaFrameTrackingSurface);
        }
        else
        {
            pCmdBuffer->Attributes.bEnableMediaFrameTracking = false;
        }
    }

    // Check if Override is needed
    if (pRenderHal->pRenderHalPltInterface)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->IsOvrdNeeded(pRenderHal, pCmdBuffer, pGenericPrologParams));
    }

    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface        = pRenderHal->pOsInterface;
    genericPrologParams.pvMiInterface       = pRenderHal->pMhwMiInterface;
    genericPrologParams.bMmcEnabled         = pGenericPrologParams ? pGenericPrologParams->bMmcEnabled : false;
    MHW_RENDERHAL_CHK_STATUS(Mhw_SendGenericPrologCmd(pCmdBuffer, &genericPrologParams));

    // Send predication command
    if (pRenderHal->PredicationParams.predicationEnabled)
    {
        MHW_RENDERHAL_CHK_STATUS(RenderHal_SendPredicationCommand(pRenderHal, pCmdBuffer));
    }

finish:
    return eStatus;
}

//!
//! \brief    Send Sync Tag
//! \details  Sends Synchronization Tags
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendSyncTag(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer)
{
    PRENDERHAL_STATE_HEAP           pStateHeap;
    MOS_STATUS                      eStatus;
    PMHW_MI_INTERFACE               pMhwMiInterface;
    MHW_PIPE_CONTROL_PARAMS         PipeCtl;

    //-------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //-------------------------------------

    pStateHeap      = pRenderHal->pStateHeap;
    pMhwMiInterface = pRenderHal->pMhwMiInterface;

    // Send PIPE_CONTROL Token
    // CMD_MI_FLUSH is disabled by default on GT, use PIPE_CONTROL
    // Requires a token and the actual pipe control command
    // Flush write caches
    PipeCtl = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest          = &pStateHeap->GshOsResource;
    PipeCtl.dwPostSyncOp      = MHW_FLUSH_NOWRITE;
    PipeCtl.dwFlushMode       = MHW_FLUSH_WRITE_CACHE;
    MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));

    // Invalidate read-only caches and perform a post sync write
    PipeCtl = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest          = &pStateHeap->GshOsResource;
    PipeCtl.dwResourceOffset  = pStateHeap->dwOffsetSync;
    PipeCtl.dwPostSyncOp      = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
    PipeCtl.dwFlushMode       = MHW_FLUSH_READ_CACHE;
    PipeCtl.dwDataDW1         = pStateHeap->dwNextTag;
    MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));

finish:
    return eStatus;
}

MOS_STATUS RenderHal_SendSyncTagIndex(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    int32_t                 iIndex)
{
    PRENDERHAL_STATE_HEAP           pStateHeap;
    MOS_STATUS                      eStatus;
    MHW_PIPE_CONTROL_PARAMS         PipeCtl;
    PMHW_MI_INTERFACE               pMhwMiInterface;

    //-------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    //-------------------------------------

    pStateHeap      = pRenderHal->pStateHeap;
    pMhwMiInterface = pRenderHal->pMhwMiInterface;

    // Invalidate read-only caches and perform a post sync write
    PipeCtl = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest = &pStateHeap->GshOsResource;
    PipeCtl.dwResourceOffset = pStateHeap->dwOffsetSync + iIndex * 8;
    PipeCtl.dwPostSyncOp = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
    PipeCtl.dwFlushMode = MHW_FLUSH_READ_CACHE;
    PipeCtl.dwDataDW1 = pStateHeap->dwNextTag;
    MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));

finish:
    return eStatus;
}

void RenderHal_SetupPrologParams(
    PRENDERHAL_INTERFACE              renderHal,
    RENDERHAL_GENERIC_PROLOG_PARAMS  *prologParams,
    PMOS_RESOURCE                     osResource,
    uint32_t                          offset,
    uint32_t                          tag);

//!
//! \brief    Initialize
//! \details  Initialize HW states
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PCRENDERHAL_SETTINGS pSettings
//!           [in] Pointer to Settings
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS    if succeeded
//!           others                if failed to allocate/initialize HW commands
//!
MOS_STATUS RenderHal_Initialize(
    PRENDERHAL_INTERFACE   pRenderHal,
    PRENDERHAL_SETTINGS    pSettings)
{
    MOS_STATUS          eStatus;
    PMOS_INTERFACE      pOsInterface;
    MHW_STATE_BASE_ADDR_PARAMS *pStateBaseParams;
    MOS_ALLOC_GFXRES_PARAMS     AllocParams;

    //------------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //------------------------------------------------

    pOsInterface = pRenderHal->pOsInterface;

    // Apply settings
    if (pSettings)
    {
        pRenderHal->StateHeapSettings.iMediaStateHeaps = pSettings->iMediaStates;
    }

    // Apply SSH settings for the current platform
    pRenderHal->StateHeapSettings.iSurfaceStateHeaps =
                                pRenderHal->StateHeapSettings.iMediaStateHeaps;

    // Initialize MHW interfaces
    // Allocate and initialize state heaps (GSH, SSH, ISH)
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnAllocateStateHeaps(pRenderHal, &pRenderHal->StateHeapSettings));

    // If ASM debug is enabled, allocate debug resource
    MHW_RENDERHAL_CHK_STATUS(RenderHal_AllocateDebugSurface(pRenderHal));

    // Allocate Predication buffer
    MOS_ZeroMemory(&AllocParams, sizeof(AllocParams));
    AllocParams.Type        = MOS_GFXRES_BUFFER;
    AllocParams.TileType    = MOS_TILE_LINEAR;
    AllocParams.Format      = Format_Buffer;
    AllocParams.dwBytes     = MHW_PAGE_SIZE;
    AllocParams.pBufName    = "PredicationBuffer";

    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnAllocateResource(
        pOsInterface,
        &AllocParams,
        &pRenderHal->PredicationBuffer));

    // Setup State Base Address command
    pStateBaseParams   = &pRenderHal->StateBaseAddressParams;
    pStateBaseParams->presGeneralState              = &pRenderHal->pStateHeap->GshOsResource;
    pStateBaseParams->dwGeneralStateSize            = pRenderHal->pStateHeap->dwSizeGSH;
    pStateBaseParams->presDynamicState              = &pRenderHal->pStateHeap->GshOsResource;
    pStateBaseParams->dwDynamicStateSize            = pRenderHal->pStateHeap->dwSizeGSH;
    pStateBaseParams->bDynamicStateRenderTarget     = false;
    pStateBaseParams->presIndirectObjectBuffer      = &pRenderHal->pStateHeap->GshOsResource;
    pStateBaseParams->dwIndirectObjectBufferSize    = pRenderHal->pStateHeap->dwSizeGSH;
    pStateBaseParams->presInstructionBuffer         = &pRenderHal->pStateHeap->IshOsResource;
    pStateBaseParams->dwInstructionBufferSize       = pRenderHal->pStateHeap->dwSizeISH;

    if (!pRenderHal->pPerfProfiler)
    {
        pRenderHal->pPerfProfiler = MediaPerfProfiler::Instance();
        MHW_RENDERHAL_CHK_NULL(pRenderHal->pPerfProfiler);

        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pPerfProfiler->Initialize((void*)pRenderHal, pOsInterface));
    }

    new(&pRenderHal->trackerProducer) FrameTrackerProducer();

finish:
    return eStatus;
}

//!
//! \brief    Send Rcs Status Tag
//! \details  Adds pipe control command in Command Buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS  if succeeded
//!           MOS_STATUS_UNKNOWN if failed to allocate/initialize HW commands
//!
MOS_STATUS RenderHal_SendRcsStatusTag(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE               pOsInterface;
    PMHW_MI_INTERFACE            pMhwMiInterface;
    MHW_PIPE_CONTROL_PARAMS      PipeCtl;
    MOS_RESOURCE                 OsResource;

    //------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    //------------------------------------

    pOsInterface    = pRenderHal->pOsInterface;
    pMhwMiInterface = pRenderHal->pMhwMiInterface;

    // Get the Os Resource
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, &OsResource));

    // Register the buffer
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface, &OsResource, true, true));

    // Issue pipe control to write GPU Status Tag
    PipeCtl                   = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest          = &OsResource;
    PipeCtl.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal); //MOS_GPU_CONTEXT_RENDER or MOS_GPU_CONTEXT_RENDER3
    PipeCtl.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    PipeCtl.dwPostSyncOp      = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
    PipeCtl.dwFlushMode       = MHW_FLUSH_NONE;
    MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));

    // Increment GPU Status Tag
    pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

finish:
    return eStatus;
}

//!
//! \brief    Send CSC Coefficient surface
//! \details  Adds pipe control command in Command Buffer
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    PMOS_RESOURCE presCscCoeff
//!           [in] Pointer to CSC Coefficient Surface
//! \param    Kdll_CacheEntry *pKernelEntry
//!           [in] Pointer to Kernel Entry
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS  if succeeded
//!           MOS_STATUS_UNKNOWN if failed to allocate/initialize HW commands
//!
MOS_STATUS RenderHal_SendCscCoeffSurface(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    PMOS_RESOURCE                presCscCoeff,
    Kdll_CacheEntry              *pKernelEntry)
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE               pOsInterface;
    PMHW_MI_INTERFACE            pMhwMiInterface;
    MHW_PIPE_CONTROL_PARAMS      PipeCtl;
    MOS_SURFACE                  Surface;
    uint64_t                     *pTempCoeff;
    uint32_t                     dwLow;
    uint32_t                     dwHigh;
    uint32_t                     dwOffset;
    uint32_t                     dwCount;
    uint8_t                      uiPatchMatrixID;

    //------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(presCscCoeff);
    MHW_RENDERHAL_CHK_NULL(pKernelEntry);
    MHW_RENDERHAL_CHK_NULL(pKernelEntry->pCscParams);
    //------------------------------------

    pOsInterface    = pRenderHal->pOsInterface;
    pMhwMiInterface = pRenderHal->pMhwMiInterface;
    dwOffset        = 0;
    dwCount         = sizeof(pKernelEntry->pCscParams->Matrix[0].Coeff) / sizeof(uint64_t);
    MOS_ZeroMemory(&Surface, sizeof(Surface));

    // Register the buffer
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(pOsInterface, presCscCoeff, true, true));
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnGetResourceInfo(pOsInterface, presCscCoeff, &Surface));

    PipeCtl              = g_cRenderHal_InitPipeControlParams;
    PipeCtl.presDest     = presCscCoeff;
    PipeCtl.dwPostSyncOp = MHW_FLUSH_WRITE_IMMEDIATE_DATA;
    PipeCtl.dwFlushMode  = MHW_FLUSH_READ_CACHE;

    for (uint32_t j = 0; j < pKernelEntry->pCscParams->PatchMatrixNum; j++)
    {
        uiPatchMatrixID = pKernelEntry->pCscParams->PatchMatrixID[j];
        pTempCoeff = (uint64_t *)pKernelEntry->pCscParams->Matrix[uiPatchMatrixID].Coeff;

        // Issue pipe control to write CSC Coefficient Surface
        for (uint16_t i = 0; i < dwCount; i++, pTempCoeff++)
        {
            dwLow = (uint32_t)((*pTempCoeff) & 0xFFFFFFFF);
            dwHigh = (uint32_t)(((*pTempCoeff) >> 32) & 0xFFFFFFFF);
            PipeCtl.dwResourceOffset = dwOffset + sizeof(uint64_t) * i;
            PipeCtl.dwDataDW1 = dwLow;
            PipeCtl.dwDataDW2 = dwHigh;

            MHW_RENDERHAL_CHK_STATUS(pMhwMiInterface->AddPipeControl(pCmdBuffer, nullptr, &PipeCtl));
        }

        dwOffset += Surface.dwPitch;
    }

finish:
    return eStatus;
}

//!
//! \brief    Issue command to write timestamp
//! \param    [in] pRenderHal
//! \param    [in] pCmdBuffer
//! \param    [in] bStartTime
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendTimingData(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    bool                         bStartTime);

MOS_STATUS RenderHal_SendStateBaseAddress(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    MhwRenderInterface    *pMhwRender;
    MOS_STATUS            eStatus;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);

    pMhwRender = pRenderHal->pMhwRenderInterface;
    eStatus    = pMhwRender->AddStateBaseAddrCmd(pCmdBuffer,
                                                 &pRenderHal->StateBaseAddressParams);
finish:
    return eStatus;
}

//!
//! \brief    Send Media States
//! \details  Send Media States
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
//!           [in]    Pointer to GPGPU walker parameters
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendMediaStates(
    PRENDERHAL_INTERFACE      pRenderHal,
    PMOS_COMMAND_BUFFER       pCmdBuffer,
    PMHW_WALKER_PARAMS        pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS  pGpGpuWalkerParams)
{
    PMOS_INTERFACE               pOsInterface = nullptr;
    MhwRenderInterface           *pMhwRender = nullptr;
    PMHW_MI_INTERFACE            pMhwMiInterface = nullptr;
    PRENDERHAL_STATE_HEAP        pStateHeap = nullptr;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    MHW_VFE_PARAMS               *pVfeStateParams = nullptr;
    MOS_CONTEXT                  *pOsContext = nullptr;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterImmParams = {};
    PMHW_MI_MMIOREGISTERS        pMmioRegisters = nullptr;
    MOS_OCA_BUFFER_HANDLE        hOcaBuf = 0;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);
    MHW_RENDERHAL_ASSERT(pRenderHal->pStateHeap->bGshLocked);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwRenderInterface->GetMmioRegisters());

    //---------------------------------------
    pOsInterface            = pRenderHal->pOsInterface;
    pMhwRender              = pRenderHal->pMhwRenderInterface;
    pMhwMiInterface         = pRenderHal->pMhwMiInterface;
    pStateHeap              = pRenderHal->pStateHeap;
    pOsContext              = pOsInterface->pOsContext;
    pMmioRegisters          = pMhwRender->GetMmioRegisters();

    // This need not be secure, since PPGTT will be used here. But moving this after
    // L3 cache configuration will delay UMD from fetching another media state.
    // Send Sync Tag
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendSyncTag(pRenderHal, pCmdBuffer));

    // Setup L3$ Config, LRI commands used here & hence must be launched from a secure bb
    pRenderHal->L3CacheSettings.bEnableSLM = (pGpGpuWalkerParams && pGpGpuWalkerParams->SLMSize > 0);
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnEnableL3Caching(pRenderHal, &pRenderHal->L3CacheSettings));

    // Send L3 Cache Configuration
    MHW_RENDERHAL_CHK_STATUS(pMhwRender->SetL3Cache(pCmdBuffer));

    MHW_RENDERHAL_CHK_STATUS(pMhwRender->EnablePreemption(pCmdBuffer));

    // Send Debug Control, LRI commands used here & hence must be launched from a secure bb
    MHW_RENDERHAL_CHK_STATUS(RenderHal_AddDebugControl(pRenderHal, pCmdBuffer));

    // Send Pipeline Select command
    MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddPipelineSelectCmd(pCmdBuffer,
                                                                 (pGpGpuWalkerParams) ? true: false));

    // The binding table for surface states is at end of command buffer. No need to add it to indirect state heap.
    HalOcaInterface::OnIndirectState(*pCmdBuffer, *pOsContext, pRenderHal->StateBaseAddressParams.presInstructionBuffer,
        pStateHeap->CurIDEntryParams.dwKernelOffset, false, pStateHeap->iKernelUsedForDump);

    // Send State Base Address command
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendStateBaseAddress(pRenderHal, pCmdBuffer));

    if (pRenderHal->bComputeContextInUse)
    {
        pRenderHal->pRenderHalPltInterface->SendTo3DStateBindingTablePoolAlloc(pRenderHal, pCmdBuffer);
    }

    // Send Surface States
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendSurfaces(pRenderHal, pCmdBuffer));

    // Send SIP State if ASM debug enabled
    if (pRenderHal->bIsaAsmDebugEnable)
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddSipStateCmd(pCmdBuffer,
                                                                &pRenderHal->SipStateParams));
    }

    pVfeStateParams = pRenderHal->pRenderHalPltInterface->GetVfeStateParameters();
    if (!pRenderHal->bComputeContextInUse)
    {
        // set VFE State
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddMediaVfeCmd(pCmdBuffer, pVfeStateParams));
    }
    else
    {
        // set CFE State
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddCfeStateCmd(pCmdBuffer, pVfeStateParams));
    }

    // Send CURBE Load
    if (!pRenderHal->bComputeContextInUse)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendCurbeLoad(pRenderHal, pCmdBuffer));
    }

    // Send Interface Descriptor Load
    if (!pRenderHal->bComputeContextInUse)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendMediaIdLoad(pRenderHal, pCmdBuffer));
    }

    // Send Chroma Keys
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendChromaKey(pRenderHal, pCmdBuffer));

    // Send Palettes in use
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSendPalette(pRenderHal, pCmdBuffer));

    HalOcaInterface::OnDispatch(*pCmdBuffer, *pOsContext, *pRenderHal->pMhwMiInterface, *pMmioRegisters);

    // Send Media object walker
    if(pWalkerParams)
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddMediaObjectWalkerCmd(
            pCmdBuffer,
            pWalkerParams));
    }
    else if (pGpGpuWalkerParams && (!pRenderHal->bComputeContextInUse))
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->AddGpGpuWalkerStateCmd(
            pCmdBuffer,
            pGpGpuWalkerParams));
    }
    else if (pGpGpuWalkerParams && pRenderHal->bComputeContextInUse)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->SendComputeWalker(
            pRenderHal,
            pCmdBuffer,
            pGpGpuWalkerParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Assign binding Table
//! \details  Assigns binding Table
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    int32_t *piBindingTable
//!           [out] Pointer to Binding Table
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_AssignBindingTable(
    PRENDERHAL_INTERFACE     pRenderHal,
    int32_t                  *piBindingTable)
{
    PRENDERHAL_STATE_HEAP     pStateHeap;
    uint32_t                  dwOffset;
    MOS_STATUS                eStatus;

    //----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(piBindingTable);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    //----------------------------------------

    *piBindingTable = -1;
    pStateHeap      = pRenderHal->pStateHeap;
    eStatus         = MOS_STATUS_UNKNOWN;

    if (pStateHeap->iCurrentBindingTable >= pRenderHal->StateHeapSettings.iBindingTables)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Unable to allocate Binding Table. Exceeds Maximum.");
        goto finish;
    }

    *piBindingTable = pStateHeap->iCurrentBindingTable;

    // Get Offset to Current Binding Table
    dwOffset    = *piBindingTable * pStateHeap->iBindingTableSize;            // Moves the pointer to a Particular Binding Table

    // Reset Binding Table
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pSshBuffer);
    MOS_ZeroMemory(pStateHeap->pSshBuffer + dwOffset, pStateHeap->iBindingTableSize);

    // Setup Debug surface state if needed
    MHW_RENDERHAL_CHK_STATUS(RenderHal_SetupDebugSurfaceState(pRenderHal));

    // Increment the Current Binding Table
    ++pStateHeap->iCurrentBindingTable;

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Setup Buffer Surface State
//! \details  Setup Buffer Surface States
//!           For buffer surfaces, the number of entries in the buffer 
//!           ranges from 1 to 2^27.   After subtracting one from the number 
//!           of entries, software must place the fields of the resulting 
//!           27-bit value into the Height, Width, and Depth fields as 
//!           indicated, right-justified in each field.
//!           Unused upper bits must be set to zero.
//!
//!           Width:  contains bits [6:0] of the number of entries in the 
//!                   buffer 1 [0,127]  --> 7 Bits
//!           Height: contains bits [20:7] of the number of entries in the 
//!                   buffer 1 [0,16383] --> 14 Bits
//!           Depth:  contains bits [26:21] of the number of entries in the 
//!                   buffer 1 [0,63]  --> 6 Bits
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in]  Pointer to RenderHal Interface
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in]  Pointer to MOS Command Buffer
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in]  Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!           [in]  Pointer to Surface state parameters
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntry
//!           [out] Pointer to Surface entry
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SetupBufferSurfaceState(
    PRENDERHAL_INTERFACE             pRenderHal,
    PRENDERHAL_SURFACE               pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS  pParams,
    PRENDERHAL_SURFACE_STATE_ENTRY   *ppSurfaceEntry)
{
    MOS_STATUS                      eStatus;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    MHW_RCS_SURFACE_PARAMS          RcsSurfaceParams;

    //--------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL(pParams);
    MHW_RENDERHAL_CHK_NULL(ppSurfaceEntry);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_ASSERT(pRenderHalSurface->OsSurface.dwWidth > 0);
    //--------------------------------------

    eStatus    = MOS_STATUS_SUCCESS;

    // Force surface type to Default (non-AVS)
    pParams->Type = pRenderHal->SurfaceTypeDefault;

    // Assign Surface State
    // Assign a New Surface State Entry
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnAssignSurfaceState(
        pRenderHal,
        pRenderHal->SurfaceTypeDefault,
        ppSurfaceEntry));

    pSurfaceEntry = *ppSurfaceEntry;
    MHW_RENDERHAL_CHK_NULL(pSurfaceEntry);

    // Update surface state offset in SSH
    *pSurfaceEntry->pSurface = pRenderHalSurface->OsSurface;
    pSurfaceEntry->dwSurfStateOffset =
            pRenderHal->pStateHeap->iSurfaceStateOffset +
            pSurfaceEntry->iSurfStateID * pRenderHal->pHwSizes->dwSizeSurfaceState;

    // Setup MHW parameters
    MOS_ZeroMemory(&RcsSurfaceParams, sizeof(MHW_RCS_SURFACE_PARAMS));
    RcsSurfaceParams.psSurface             = &pRenderHalSurface->OsSurface;
    RcsSurfaceParams.dwOffsetInSSH         = pSurfaceEntry->dwSurfStateOffset;
    RcsSurfaceParams.dwCacheabilityControl = pRenderHal->pfnGetSurfaceMemoryObjectControl(pRenderHal, pParams);
    RcsSurfaceParams.bIsWritable           = pParams->bRenderTarget;
    RcsSurfaceParams.bRenderTarget         = pParams->bRenderTarget;

    // Call MHW to setup the Surface State Heap entry for Buffer
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetSurfaceStateBuffer(pRenderHal, &RcsSurfaceParams, pSurfaceEntry->pSurfaceState));

    // Setup OS Specific States
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetupSurfaceStatesOs(pRenderHal, pParams, pSurfaceEntry));

finish:
    return eStatus;
}

//!
//! \brief    Set surface state buffer
//! \param    [in] pRenderHal
//!           pointer to render hal
//!           [in] pParams
//!           pointer to surface parameters
//! \param    void  *pSurfaceState
//!           [in/out] Surface State Pointer
//!
MOS_STATUS RenderHal_SetSurfaceStateBuffer(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMHW_RCS_SURFACE_PARAMS     pParams,
    void                        *pSurfaceState)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(pParams);
    MHW_MI_CHK_NULL(pParams->psSurface);
    MHW_MI_CHK_NULL(pSurfaceState);

    MHW_SURFACE_STATE_PARAMS Params;
    MOS_ZeroMemory(&Params, sizeof(Params));
    PMOS_SURFACE pSurface = pParams->psSurface;
    uint32_t dwBufferSize = pSurface->dwWidth - 1;
    Params.SurfaceType3D = MOS_GFXRES_SCRATCH == pSurface->Type?
            GFX3DSTATE_SURFACETYPE_SCRATCH
            : GFX3DSTATE_SURFACETYPE_BUFFER;

    if (MOS_GFXRES_SCRATCH == pSurface->Type)
    {
        Params.dwPitch = 1023;
        uint32_t entry_count = pSurface->dwWidth/(Params.dwPitch + 1);
        Params.dwWidth = (entry_count - 1) & MOS_MASKBITS32(0, 6);
        Params.dwHeight = (((entry_count - 1) & MOS_MASKBITS32(7, 20)) >> 7);
        Params.dwDepth = (((entry_count - 1) & 0xFFE00000) >> 21);
        Params.dwFormat = MHW_GFX3DSTATE_SURFACEFORMAT_RAW;
    }
    else
    {
        // Width  contains bits [ 6:0] of the number of entries in the buffer
        Params.dwWidth = (uint8_t)(dwBufferSize & MOS_MASKBITS32(0, 6));
        // Height contains bits [20:7] of the number of entries in the buffer
        Params.dwHeight = (uint16_t)((dwBufferSize & MOS_MASKBITS32(7, 20)) >> 7);
        // For SURFTYPE_BUFFER, pitch is defaulted to 0. Resetting is unnecessary.

        uint32_t depthMaskBuffer = pRenderHal->pRenderHalPltInterface
                ->GetDepthBitMaskForBuffer();
        uint32_t depthMaskRawBuffer = pRenderHal->pRenderHalPltInterface
                ->GetDepthBitMaskForRawBuffer();
        switch (pSurface->Format)
        {
            // We consider MOS's Format_Buffer as MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM
            // format for most of cases
            case Format_Buffer:
                Params.dwFormat = MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM;
                Params.dwDepth
                        = (uint16_t)((dwBufferSize & depthMaskBuffer) >> 21);
                break;

            case Format_RAW:
                Params.dwFormat = MHW_GFX3DSTATE_SURFACEFORMAT_RAW;
                Params.dwDepth
                        = (uint16_t)((dwBufferSize & depthMaskRawBuffer) >> 21);
                break;

            case Format_L8:
                Params.dwFormat = MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM;
                Params.dwDepth
                        = (uint16_t)((dwBufferSize & depthMaskBuffer) >> 21);
                break;

            default:
                MHW_ASSERTMESSAGE("Invalid buffer Resource format");
                break;
        }
    }

    Params.pSurfaceState = (uint8_t*)pSurfaceState;
    Params.dwCacheabilityControl = pParams->dwCacheabilityControl;

    // Setup Surface State Entry via MHW state heap interface
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->SetSurfaceStateEntry(&Params));

finish:
    return eStatus;
}

//!
//! \brief      Setup Interface Descriptor
//! \details    Set interface descriptor
//! \param      PRENDERHAL_INTERFACE                    pRenderHal
//!             [in]    Pointer to HW interface
//! \param      PRENDERHAL_MEDIA_STATE                  pMediaState
//!             [in]    Pointer to media state
//! \param      PRENDERHAL_KRN_ALLOCATION               pKernelAllocation
//!             [in]    Pointer to kernel allocation
//! \param      PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS  pInterfaceDescriptorParams
//!             [in]    Pointer to interface descriptor parameters
//! \param      PMHW_GPGPU_WALKER_PARAMS          pGpGpuWalkerParams
//!             [in]    Pointer to gpgpu walker parameters
//! \return     MOS_STATUS
//!
MOS_STATUS RenderHal_SetupInterfaceDescriptor(
    PRENDERHAL_INTERFACE                   pRenderHal,
    PRENDERHAL_MEDIA_STATE                 pMediaState,
    PRENDERHAL_KRN_ALLOCATION              pKernelAllocation,
    PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS pInterfaceDescriptorParams)
{
    MOS_STATUS               eStatus = MOS_STATUS_SUCCESS;
    PMHW_ID_ENTRY_PARAMS     pParams = nullptr;
    PRENDERHAL_STATE_HEAP    pStateHeap = nullptr;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pMediaState);
    MHW_RENDERHAL_CHK_NULL(pKernelAllocation);
    MHW_RENDERHAL_CHK_NULL(pInterfaceDescriptorParams);
    //-----------------------------------------

    // Get states, params
    pStateHeap      = pRenderHal->pStateHeap;
    pParams         = &pStateHeap->CurIDEntryParams;

    pParams->dwMediaIdOffset      = pMediaState->dwOffset + pStateHeap->dwOffsetMediaID;
    pParams->iMediaId             = pInterfaceDescriptorParams->iMediaID;
    pParams->dwKernelOffset       = pKernelAllocation->dwOffset;
    pParams->dwSamplerOffset      = pMediaState->dwOffset + pStateHeap->dwOffsetSampler +
                                  pInterfaceDescriptorParams->iMediaID * pStateHeap->dwSizeSampler;
    pParams->dwSamplerCount       = pKernelAllocation->Params.Sampler_Count;
    pParams->dwBindingTableOffset = pInterfaceDescriptorParams->iBindingTableID * pStateHeap->iBindingTableSize;
    pParams->iCurbeOffset         = pInterfaceDescriptorParams->iCurbeOffset;
    pParams->iCurbeLength         = pInterfaceDescriptorParams->iCurbeLength;

    pParams->bBarrierEnable                   = pInterfaceDescriptorParams->blBarrierEnable;
    pParams->bGlobalBarrierEnable             = pInterfaceDescriptorParams->blGlobalBarrierEnable;    //It's only applied for BDW+
    pParams->dwNumberofThreadsInGPGPUGroup    = pInterfaceDescriptorParams->iNumberThreadsInGroup;
    pParams->dwSharedLocalMemorySize          = pRenderHal->pfnEncodeSLMSize(pRenderHal, pInterfaceDescriptorParams->iSLMSize);
    pParams->iCrsThdConDataRdLn               = pInterfaceDescriptorParams->iCrsThrdConstDataLn;
    pParams->pGeneralStateHeap                = nullptr;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->SetInterfaceDescriptorEntry(pParams));

finish:
    return eStatus;
}

//!
//! \brief      Get Media Walker Status
//! \details    Returns Media Walker Enabled / Disabled
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to RenderHal Interface
//! \return     bool
//!
bool RenderHal_GetMediaWalkerStatus(
    PRENDERHAL_INTERFACE pRenderHal)
{
    //-----------------------------------------
    if (pRenderHal == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid renderhal.");
        return false;
    }
    //-----------------------------------------
    
    if (pRenderHal->MediaWalkerMode == MHW_WALKER_MODE_DISABLED)
    {
        return false;
    }

    return true;
}

//!
//! \brief      Get surface memory object control \
//! \details    Returns surface memory object control
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in] Pointer to RenderHal Interface
//! \param      PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!             [in] Pointer to surface state params
//! \return     uint32_t 
//!
uint32_t RenderHal_GetSurfaceMemoryObjectControl(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams)
{
    MHW_RENDERHAL_UNUSED(pRenderHal);
    
    //-----------------------------------------
    if (pParams == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("nullptr pointer.");
        return 0;
    }
    //-----------------------------------------

    return pParams->MemObjCtl;
}

//!
//! \brief      Get the size of Scratch Space
//! \details    Get scratch space size based on max gen config and scratch space 
//!             size per thread
//!             FFTID is used by HW to access scratch space; since FFTID generated
//!             is not continuous, need to allocate scratch space based on maximum gen config
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]     Hardware interface
//! \param      uint32_t iPerThreadScratchSpaceSize
//!             [in]     Per thread scratch space size
//! \return     uint32_t
//!
uint32_t RenderHal_GetScratchSpaceSize(
    PRENDERHAL_INTERFACE       pRenderHal,
    uint32_t                   iPerThreadScratchSpaceSize)
{
    MEDIA_SYSTEM_INFO             *pGtSystemInfo;

    //-----------------------------------------
    if (pRenderHal == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid renderhal.");
        return 0;
    }
    //-----------------------------------------

    pGtSystemInfo                       = pRenderHal->pOsInterface->pfnGetGtSystemInfo(pRenderHal->pOsInterface);
    uint32_t numHWThreadsPerEU          = pGtSystemInfo->ThreadCount / pGtSystemInfo->EUCount;

    uint32_t dwNumberOfScratchSpaceEntries = pGtSystemInfo->MaxEuPerSubSlice * numHWThreadsPerEU * pGtSystemInfo->MaxSubSlicesSupported;
    uint32_t dwScratchSpaceSize            = dwNumberOfScratchSpaceEntries * iPerThreadScratchSpaceSize;

    return dwScratchSpaceSize;
}

//!
//! \brief    Setup OS specific surface state parameters
//! \details  Setup Platform and Operating System Specific Surface State
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!           [in] Pointer to Surface Params
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY pSurfaceEntry
//!           [in] Pointer to Surface State Entry
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if successful
//!
MOS_STATUS RenderHal_SetupSurfaceStatesOs(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams,
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry)
{
    PMOS_SURFACE                    pSurface;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MHW_SURFACE_TOKEN_PARAMS        TokenParams;

    uint32_t additional_plane_offset = 0;
    uint32_t vertical_offset_in_surface_state = 0;

    //-----------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pParams);
    MHW_RENDERHAL_CHK_NULL(pSurfaceEntry);
    //-----------------------------------------

    pSurface = pSurfaceEntry->pSurface;

    // Surface, plane, offset
    TokenParams.pOsSurface         = pSurface;
    TokenParams.YUVPlane           = pSurfaceEntry->YUVPlane;

    switch (pSurfaceEntry->YUVPlane)
    {
        case MHW_U_PLANE:
            vertical_offset_in_surface_state = pSurface->UPlaneOffset.iYOffset;
            vertical_offset_in_surface_state &= 0x1C;  // The offset value in surface state commands.
            additional_plane_offset = pSurface->UPlaneOffset.iYOffset
                    - vertical_offset_in_surface_state;
            additional_plane_offset *= pSurface->dwPitch;
            TokenParams.dwSurfaceOffset = pSurface->UPlaneOffset.iSurfaceOffset
                    + additional_plane_offset;
            break;
        case MHW_V_PLANE:
            vertical_offset_in_surface_state = pSurface->VPlaneOffset.iYOffset;
            vertical_offset_in_surface_state &= 0x1C;
            additional_plane_offset = pSurface->VPlaneOffset.iYOffset
                    - vertical_offset_in_surface_state;
            additional_plane_offset *= pSurface->dwPitch;
            TokenParams.dwSurfaceOffset = pSurface->VPlaneOffset.iSurfaceOffset
                    + additional_plane_offset;
            break;
        default:
            vertical_offset_in_surface_state = pSurface->YPlaneOffset.iYOffset;
            vertical_offset_in_surface_state &= 0x1C;
            additional_plane_offset = pSurface->YPlaneOffset.iYOffset
                    - vertical_offset_in_surface_state;
            additional_plane_offset *= pSurface->dwPitch;
            TokenParams.dwSurfaceOffset
                    = pSurface->dwOffset + additional_plane_offset;
            break;
    }

    // Surface type
    TokenParams.bRenderTarget   = pParams->bRenderTarget;
    TokenParams.bSurfaceTypeAvs = pSurfaceEntry->bAVS;

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetSurfaceStateToken(
            pRenderHal,
            &TokenParams,
            &pSurfaceEntry->SurfaceToken));

finish:
    return eStatus;
}

//!
//! \brief    Bind Surface State
//! \details  Binds Surface State
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface
//! \param    int32_t iBindingTableIndex
//!           [in] Binding Table Index
//! \param    int32_t iBindingTableEntry
//!           [out] Binding Table Entry
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY pSurfaceEntry
//!           [in] Pointer to Surface State Entry
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_BindSurfaceState(
    PRENDERHAL_INTERFACE            pRenderHal,
    int32_t                         iBindingTableIndex,
    int32_t                         iBindingTableEntry,
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PMHW_RENDER_STATE_SIZES     pHwSizes;
    MHW_BINDING_TABLE_PARAMS    Params;
    uint32_t                    dwOffset;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    //--------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pSurfaceEntry);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    MHW_RENDERHAL_ASSERT(iBindingTableIndex >= 0);
    MHW_RENDERHAL_ASSERT(iBindingTableEntry >= 0);
    //--------------------------------------------

    pStateHeap = pRenderHal->pStateHeap;
    pHwSizes   = pRenderHal->pHwSizes;

    // Get Offset to Current Binding Table
    dwOffset    = (pStateHeap->iCurSshBufferIndex * pStateHeap->dwSshIntanceSize) + // Points to the Base of Current SSH Buffer Instance
                  (pStateHeap->iBindingTableOffset)                               + // Moves the pointer to Base of Array of Binding Tables
                  (iBindingTableIndex * pStateHeap->iBindingTableSize)            + // Moves the pointer to a Particular Binding Table
                  (iBindingTableEntry * pHwSizes->dwSizeBindingTableState);         // Move the pointer to correct entry

    MHW_RENDERHAL_CHK_NULL(pStateHeap->pSshBuffer);
    Params.pBindingTableEntry   = pStateHeap->pSshBuffer + dwOffset;
    Params.dwSurfaceStateOffset = pSurfaceEntry->dwSurfStateOffset;
    Params.bSurfaceStateAvs     = (pSurfaceEntry->Type == pRenderHal->SurfaceTypeAdvanced ) ? true : false;
    Params.iBindingTableEntry   = iBindingTableEntry;

    // Set binding table entry in MHW
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->SetBindingTableEntry(&Params));

finish:
    return eStatus;
}

//!
//! \brief    Send Surfaces PatchList
//! \details  Send Surface State commands
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendSurfaces_PatchList(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer);

//!
//! \brief    Set Vfe State Params
//! \details  Sets VFE State parameters
//!           this functions must be called to setup
//!           parameters for pMhwRender->AddMediaVfeCmd()
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint32_t dwDebugCounterControl
//!           [in] Debug Counter Control
//! \param    uint32_t dwMaximumNumberofThreads
//!           [in] Maximum Number of Threads
//! \param    uint32_t dwCURBEAllocationSize
//!           [in] CURBE Allocation Size
//! \param    uint32_t dwURBEntryAllocationSize
//!           [in] URB Entry Allocation Size
//! \param    PRENDERHAL_SCOREBOARD_PARAMS pScoreboardParams
//!           [in] Pointer to Scoreboard Params
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SetVfeStateParams(
    PRENDERHAL_INTERFACE    pRenderHal,
    uint32_t                dwDebugCounterControl,
    uint32_t                dwMaximumNumberofThreads,
    uint32_t                dwCURBEAllocationSize,
    uint32_t                dwURBEntryAllocationSize,
    PMHW_VFE_SCOREBOARD     pScoreboardParams)
{
    PMHW_VFE_PARAMS                 pVfeParams;
    PRENDERHAL_STATE_HEAP           pStateHeap;
    PMHW_RENDER_ENGINE_CAPS         pHwCaps;
    PRENDERHAL_STATE_HEAP_SETTINGS  pSettings;
    uint32_t                        dwMaxURBSize;
    uint32_t                        dwMaxCURBEAllocationSize;
    uint32_t                        dwMaxURBEntryAllocationSize;
    uint32_t                        dwNumberofURBEntries;
    uint32_t                        dwMaxURBEntries;
    uint32_t                        dwMaxInterfaceDescriptorEntries;
    MOS_STATUS                      eStatus;
    uint32_t i;

    //---------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pWaTable);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwCaps);
    //---------------------------------------------

    eStatus     = MOS_STATUS_SUCCESS;
    pStateHeap  = pRenderHal->pStateHeap;
    pHwCaps     = pRenderHal->pHwCaps;
    pVfeParams  = pRenderHal->pRenderHalPltInterface->GetVfeStateParameters();
    MHW_RENDERHAL_CHK_NULL(pVfeParams);
    pSettings   = &(pRenderHal->StateHeapSettings);

    pVfeParams->pKernelState             = nullptr;
    pVfeParams->eVfeSliceDisable         = MHW_VFE_SLICE_ALL;

    //-------------------------------------------------------------------------
    // --Gen6 GT1--
    // In this calculation, URBEntryAllocationSize and CURBEAllocationSize are
    // in 256-bit units:
    // (URBEntryAllocationSize * NumberofURBEntries + CURBEAllocationSize +
    //  MaxInterfaceDescriptorEntries) <= 1024
    //
    // --Gen6 GT2 + Gen7--
    // In this calculation, URBEntryAllocationSize and CURBEAllocationSize are
    // in 256-bit units:
    // (URBEntryAllocationSize * NumberofURBEntries + CURBEAllocationSize +
    //  MaxInterfaceDescriptorEntries) <= 2048
    //-------------------------------------------------------------------------

    // get the Max for all the fields
    dwMaxURBSize                    = pHwCaps->dwMaxURBSize;
    dwMaxURBEntries                 = pHwCaps->dwMaxURBEntries;
    dwMaxURBEntryAllocationSize     = pHwCaps->dwMaxURBEntryAllocationSize;
    dwMaxCURBEAllocationSize        = pHwCaps->dwMaxCURBEAllocationSize;
    dwMaxInterfaceDescriptorEntries = pHwCaps->dwMaxInterfaceDescriptorEntries;

    // CURBEAllocationSize must be >= CurbeTotalDataLength in CURBE_LOAD.
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pCurMediaState);
    dwCURBEAllocationSize = MOS_MAX(dwCURBEAllocationSize, (uint32_t)pStateHeap->pCurMediaState->iCurbeOffset);

    // CURBEAllocationSize is in 256-bit (32-byte) units, so we round up to
    // 32. We do not force it to be greater than zero because a Kernel may not
    // be using any CURBE parameters.
    dwCURBEAllocationSize = MOS_ROUNDUP_SHIFT(dwCURBEAllocationSize, 5);

    // URBEntryAllocationSize is in 256-bit (32-byte) units, so we round up to
    // 32. We ensure that it is greater than zero, because we will use this in
    // division when we calculate dwNumberofURBEntries, so we need a value
    // that is greater than zero.
    // URBEntryAllocationSize sets the size of the Inline Data. All the Inline
    // Data will be placed in the URB which is a buffer that is managed by the
    // hardware. When the thread is dispatched, the data in the URB will be
    // copied to the GRF.
    dwURBEntryAllocationSize = MOS_ROUNDUP_SHIFT(dwURBEntryAllocationSize, 5);
    dwURBEntryAllocationSize = MOS_MAX(1, dwURBEntryAllocationSize);

    // NumberofURBEntries defines the depth of the dispatch queue. Up to this
    // many threads can be in the queue. It's related to performance. For hardware,
    // we use the size of the Inline Data to calculate the maximum number of entries
    // we can use. We cap it to 32 URB entries, and then ensure that it is greater
    // than zero.
    dwNumberofURBEntries =
        (dwMaxURBSize - dwCURBEAllocationSize - dwMaxInterfaceDescriptorEntries) /
         dwURBEntryAllocationSize;
    dwNumberofURBEntries = MOS_CLAMP_MIN_MAX(dwNumberofURBEntries, 1, 32);

    pVfeParams->dwDebugCounterControl    = dwDebugCounterControl;
    pVfeParams->dwNumberofURBEntries     = dwNumberofURBEntries;
    pVfeParams->dwMaximumNumberofThreads = (dwMaximumNumberofThreads == RENDERHAL_USE_MEDIA_THREADS_MAX) ?
                                            pHwCaps->dwMaxThreads :
                                            MOS_MIN(dwMaximumNumberofThreads, pHwCaps->dwMaxThreads);
    pVfeParams->dwCURBEAllocationSize    = dwCURBEAllocationSize << 5;
    pVfeParams->dwURBEntryAllocationSize = dwURBEntryAllocationSize;

    MHW_RENDERHAL_ASSERT(dwNumberofURBEntries     <= dwMaxURBEntries);
    MHW_RENDERHAL_ASSERT(dwCURBEAllocationSize    <= dwMaxCURBEAllocationSize);
    MHW_RENDERHAL_ASSERT(dwURBEntryAllocationSize <= dwMaxURBEntryAllocationSize);
    MHW_RENDERHAL_ASSERT(dwNumberofURBEntries * dwURBEntryAllocationSize +
                         dwCURBEAllocationSize + dwMaxInterfaceDescriptorEntries <= dwMaxURBSize);

    // Setup Scoreboard Parameters
    if (pScoreboardParams)
    {
        MHW_RENDERHAL_ASSERT(pScoreboardParams->ScoreboardMask < 8);

        pRenderHal->VfeScoreboard.ScoreboardEnable = true;
        pRenderHal->VfeScoreboard.ScoreboardMask   = (1 << pScoreboardParams->ScoreboardMask) - 1;
        pRenderHal->VfeScoreboard.ScoreboardType   = pScoreboardParams->ScoreboardType;
        for (i = 0; i < pScoreboardParams->ScoreboardMask; i++)
        {
            pRenderHal->VfeScoreboard.ScoreboardDelta[i].x = pScoreboardParams->ScoreboardDelta[i].x;
            pRenderHal->VfeScoreboard.ScoreboardDelta[i].y = pScoreboardParams->ScoreboardDelta[i].y;
        }
    }
    else
    {
        pRenderHal->VfeScoreboard.ScoreboardEnable = true;
        pRenderHal->VfeScoreboard.ScoreboardMask   = 0x0;
    }

    // Setup VFE Scoreboard parameters
    pVfeParams->Scoreboard = pRenderHal->VfeScoreboard;

    // Setup Kernel Scratch Space
    if (pSettings->iPerThreadScratchSize > 0)
    {
        int32_t iSize;
        int32_t iRemain;
        int32_t iPerThreadScratchSize;

        MHW_RENDERHAL_ASSERT(pSettings->iPerThreadScratchSize ==
                             MOS_ALIGN_CEIL(pSettings->iPerThreadScratchSize, 1024));

        iPerThreadScratchSize = pSettings->iPerThreadScratchSize >> 10;

        iRemain               = iPerThreadScratchSize % 2;
        iPerThreadScratchSize = iPerThreadScratchSize / 2;
        iSize = 0;
        while (!iRemain && (iPerThreadScratchSize / 2))
        {
            iSize++;
            iRemain               = iPerThreadScratchSize % 2;
            iPerThreadScratchSize = iPerThreadScratchSize / 2;
        }

        MHW_RENDERHAL_ASSERT(!iRemain && iPerThreadScratchSize);
        MHW_RENDERHAL_ASSERT(iSize < 12);
        // Specifies the amount of scratch space allowed to be used by each
        // thread.  The driver must allocate enough contiguous scratch space,
        // pointed to by the Scratch Space Pointer, to ensure that the Maximum
        // Number of Threads each get Per Thread Scratch Space size without
        // exceeding the driver-allocated scratch space.
        // Range = [0,11] indicating [1k bytes, 12k bytes]
        pVfeParams->dwPerThreadScratchSpace = (uint32_t) iSize;

        // Specifies the 1k-byte aligned address offset to scratch space for
        // use by the kernel.  This pointer is relative to the
        // General State Base Address (1k aligned)
        // Format = GeneralStateOffset[31:10]
        pVfeParams->dwScratchSpaceBasePointer = pStateHeap->dwScratchSpaceBase;
    }
    else
    {
        pVfeParams->dwPerThreadScratchSpace   = 0;
        pVfeParams->dwScratchSpaceBasePointer = 0;
    }

finish:
    return eStatus;
}

//!
//! \brief      Is two Planes NV12 Needed
//! \details    Judge whether 2 plane NV12 is needed based on the width/height
//!             and its rectangle for each boundary type
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in] pointer to RenderHal Interface
//! \param      PRENDERHAL_SURFACE pRenderHalSurface
//!             [in] pointer to input Surface
//! \param      RENDERHAL_SS_BOUNDARY Boundary
//!             [in] How the adjustment needs to be done
//! \return     bool
//!
bool RenderHal_Is2PlaneNV12Needed(
    PRENDERHAL_INTERFACE   pRenderHal,
    PRENDERHAL_SURFACE     pRenderHalSurface,
    RENDERHAL_SS_BOUNDARY  Boundary)
{
    PMOS_SURFACE pSurface;
    uint16_t wWidthAlignUnit;
    uint16_t wHeightAlignUnit;
    uint32_t dwSurfaceHeight;
    uint32_t dwSurfaceWidth;
    bool bRet = false;

    //---------------------------------------------
    if (pRenderHal == nullptr
     || pRenderHalSurface == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("nullptr pointer detected.");
        goto finish;
    }
    //---------------------------------------------

    pRenderHal->pfnGetAlignUnit(&wWidthAlignUnit, &wHeightAlignUnit, pRenderHalSurface);
    pSurface = &pRenderHalSurface->OsSurface;

     switch (Boundary)
    {
        case RENDERHAL_SS_BOUNDARY_SRCRECT:
            dwSurfaceHeight = pSurface->dwHeight;
            dwSurfaceWidth  = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwWidth, (uint32_t)pRenderHalSurface->rcSrc.right), wWidthAlignUnit);
            break;

        // align with max src rect
        case RENDERHAL_SS_BOUNDARY_MAXSRCRECT:
            dwSurfaceHeight = pSurface->dwHeight;
            dwSurfaceWidth  = MOS_ALIGN_CEIL(MOS_MIN(pSurface->dwWidth, (uint32_t)pRenderHalSurface->rcMaxSrc.right), wWidthAlignUnit);
            break;

        default:
            dwSurfaceHeight = MOS_ALIGN_CEIL(pSurface->dwHeight, wHeightAlignUnit);
            dwSurfaceWidth  = MOS_ALIGN_CEIL(pSurface->dwWidth,  wWidthAlignUnit);
            break;
    }

    // On G8, NV12 format needs the width and Height to be a multiple
    // of 4 for both 3D sampler and 8x8 sampler; G75 needs the width
    // of NV12 input surface to be a multiple of 4 for 3D sampler.
    // On G9+, width need to be a multiple of 2, while height still need
    // be a multiple of 4. Since G9 already post PV, just keep the old logic
    // to enable 2 plane NV12 when the width or Height is not a multiple of 4.
    // For G10+, enable 2 plane NV12 when width is not multiple of 2 or height
    // is not multiple of 4.
    if (!GFX_IS_GEN_10_OR_LATER(pRenderHal->Platform))
    {
        bRet = (!MOS_IS_ALIGNED(dwSurfaceHeight, 4) || !MOS_IS_ALIGNED(dwSurfaceWidth, 4));
    }
    else
    {
        // For AVS sampler, no limitation for 4 alignment.
        if (RENDERHAL_SCALING_AVS == pRenderHalSurface->ScalingMode)
        {
            bRet = (!MOS_IS_ALIGNED(dwSurfaceHeight, 2) || !MOS_IS_ALIGNED(dwSurfaceWidth, 2));
        }
        else
        {
            bRet = (!MOS_IS_ALIGNED(dwSurfaceHeight, 4) || !MOS_IS_ALIGNED(dwSurfaceWidth, 2));
        }
    }

    // Note: Always using 2 plane NV12 as WA for the corruption of NV12 input
    // of which the height is greater than 16352
    bRet = bRet || (MEDIA_IS_WA(pRenderHal->pWaTable, Wa16KInputHeightNV12Planar420) && dwSurfaceHeight > 16352);

finish:
    return bRet;
}

//!
//! \brief      Sets Sampler States for Gen8
//! \details    Initialize and set sampler states
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to HW interface
//! \param      int32_t iMediaID
//!             [in]    Media Interface Descriptor ID
//! \param      PRENDERHAL_SAMPLER_STATE_PARAMS pSamplerParams
//!             [in]    Pointer to sampler state parameters
//! \param      int32_t iSamplers
//!             [in]    Number of samplers
//! \return     MOS_STATUS MOS_STATUS_SUCCESS if success, otherwise MOS_STATUS_UNKNOWN
//!
MOS_STATUS RenderHal_SetSamplerStates(
    PRENDERHAL_INTERFACE        pRenderHal,
    int32_t                     iMediaID,
    PMHW_SAMPLER_STATE_PARAM    pSamplerParams,
    int32_t                     iSamplers)
{
    MOS_STATUS                  eStatus;
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PMHW_SAMPLER_STATE_PARAM    pSamplerStateParams;
    PRENDERHAL_MEDIA_STATE      pMediaState;
    int32_t                     iOffsetSampler;
    uint8_t                     *pPtrSampler;
    uint8_t                     *pPtrSamplerAvs;
    int32_t                     i;

    eStatus = MOS_STATUS_UNKNOWN;

    //-----------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pSamplerParams);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pGshBuffer);
    MHW_RENDERHAL_ASSERT( iSamplers <= pRenderHal->StateHeapSettings.iSamplers );
    MHW_RENDERHAL_ASSERT((iMediaID >= 0) && (iMediaID < pRenderHal->StateHeapSettings.iMediaIDs));
    //-----------------------------------------------

    pStateHeap    = pRenderHal->pStateHeap;
    pMediaState   = pRenderHal->pStateHeap->pCurMediaState;

    // Offset/Pointer to Samplers
    iOffsetSampler   = pMediaState->dwOffset +                      // Offset to media state
                       pStateHeap->dwOffsetSampler +                // Offset to sampler area
                       iMediaID * pStateHeap->dwSizeSampler;        // Samplers for media ID
    pPtrSampler      = pStateHeap->pGshBuffer + iOffsetSampler;     // Pointer to Samplers

    iOffsetSampler   = pMediaState->dwOffset +                      // Offset to media state
                       pStateHeap->dwOffsetSamplerAVS +             // Offset to sampler area
                       iMediaID * pStateHeap->dwSizeSamplerAVS;     // Samplers for media ID
    pPtrSamplerAvs   = pStateHeap->pGshBuffer + iOffsetSampler;     // Pointer to AVS Samplers

    // Setup sampler states
    pSamplerStateParams = pSamplerParams; // Pointer to First Sampler State in array
    for (i = 0; i < iSamplers; i++, pSamplerStateParams++,
         pPtrSampler += pRenderHal->pHwSizes->dwSizeSamplerState)
    {
        if (pSamplerStateParams->bInUse)
        {
            MHW_RENDERHAL_CHK_STATUS(pRenderHal->pOsInterface->pfnSetCmdBufferDebugInfo(
                pRenderHal->pOsInterface,
                true,  //bSamplerState
                false, //bSurfaceState
                i,
                pSamplerStateParams->SamplerType));

            switch (pSamplerStateParams->SamplerType)
            {
            case MHW_SAMPLER_TYPE_3D:
                eStatus = pRenderHal->pMhwStateHeap->SetSamplerState(pPtrSampler, pSamplerStateParams);
                break;
            case MHW_SAMPLER_TYPE_AVS:
                eStatus = pRenderHal->pMhwStateHeap->SetSamplerState(pPtrSamplerAvs, pSamplerStateParams);
                pPtrSamplerAvs += pRenderHal->dwSamplerAvsIncrement;
                break;
            default:
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                MHW_RENDERHAL_ASSERTMESSAGE("Unknown Sampler Type.");
                break;
            }

            if (MOS_FAILED(eStatus))
            {
                MHW_RENDERHAL_ASSERTMESSAGE("Failed to setup Sampler");
                goto finish;
            }
        }
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    MHW_RENDERHAL_ASSERT(eStatus == MOS_STATUS_SUCCESS);
    return eStatus;
}

//!
//! \brief    Setup Surface State
//! \details  Setup Surface States
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!           [in] Pointer to Surface State Params
//! \param    int32_t *piNumEntries
//!           [out] Pointer to Number of Surface State Entries (Num Planes)
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntries
//!           [out] Array of Surface State Entries
//! \param    PRENDERHAL_OFFSET_OVERRIDE pOffsetOverride
//!           [in] If not nullptr, provides adjustments to Y, UV plane offsets,
//!           used for kernel in a few cases. nullptr is the most common usage.
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SetupSurfaceState(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams,
    int32_t                         *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
    PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    //-----------------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);
    //-----------------------------------------------

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->SetupSurfaceState(
        pRenderHal, pRenderHalSurface, pParams, piNumEntries, ppSurfaceEntries, pOffsetOverride));
finish:
    return eStatus;
}

//!
//! \brief    Get offset and/or pointer to sampler state
//! \details  Get offset and/or pointer to sampler state in General State Heap
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface
//! \param    int32_t iMediaID
//!           [in] Media ID associated with sampler
//! \param    int32_t iSamplerID
//!           [in] Sampler ID
//! \param    uint32_t *pdwSamplerOffset
//!           [out] optional; offset of sampler state from GSH base
//! \param    void  **ppSampler
//!           [out] optional; pointer to sampler state in GSH
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_GetSamplerOffsetAndPtr(
    PRENDERHAL_INTERFACE     pRenderHal,
    int32_t                  iMediaID,
    int32_t                  iSamplerID,
    PMHW_SAMPLER_STATE_PARAM pSamplerParams,
    uint32_t                 *pdwSamplerOffset,
    void                    **ppSampler)
{
    PRENDERHAL_STATE_HEAP        pStateHeap;
    uint32_t                     dwOffset = 0;
    MHW_SAMPLER_ELEMENT_TYPE     ElementType;
    MHW_SAMPLER_TYPE             SamplerType;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    uint32_t                     ElementSize[MHW_SamplerTotalElements] = {1, 2, 4, 8, 64, 128};

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    pStateHeap = pRenderHal->pStateHeap;

    MHW_RENDERHAL_CHK_NULL(pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pStateHeap->pGshBuffer);
    MHW_ASSERT(iMediaID   < pRenderHal->StateHeapSettings.iMediaIDs);
    MHW_ASSERT(iSamplerID < pRenderHal->StateHeapSettings.iSamplers);
    MHW_RENDERHAL_CHK_NULL(pSamplerParams);

    ElementType = pSamplerParams->ElementType;
    SamplerType = (pSamplerParams) ? pSamplerParams->SamplerType : MHW_SAMPLER_TYPE_3D;

    if (SamplerType == MHW_SAMPLER_TYPE_VME)
    {
        dwOffset = pStateHeap->pCurMediaState->dwOffset +                 // Offset to current media state base
            pStateHeap->dwOffsetSampler +                                 // Go to base of the sampler area
            iMediaID   * pStateHeap->dwSizeSampler +                      // Go to base of media ID's sampler area
            iSamplerID * pRenderHal->pHwSizes->dwSizeSamplerState;        // Goto to "samplerID" sampler state
    }
    else
    {
        switch (ElementType)
        {
        case MHW_Sampler1Element:
        case MHW_Sampler4Elements:
        {
            dwOffset = pStateHeap->pCurMediaState->dwOffset +             // Offset to current media state base
                pStateHeap->dwOffsetSampler +                             // Go to base of the sampler area
                iMediaID   * pStateHeap->dwSizeSampler +                  // Go to base of media ID's sampler area
                iSamplerID * pRenderHal->pHwSizes->dwSizeSamplerState;    // Goto to "samplerID" sampler state
            if (pSamplerParams)
            {
                pSamplerParams->Unorm.IndirectStateOffset =
                    pStateHeap->pCurMediaState->dwOffset +                // Offset to current media state base
                    pStateHeap->dwOffsetSamplerIndirect +                 // Go to base of the sampler indirect area
                    iMediaID   * pStateHeap->dwSizeSampler +              // Go to base of media ID's sampler area
                    iSamplerID * pRenderHal->pHwSizes->dwSizeSamplerIndirectState;   // Goto to "samplerID" sampler indirect state
                pSamplerParams->Unorm.pIndirectState = (void *)(pStateHeap->pGshBuffer + pSamplerParams->Unorm.IndirectStateOffset);
            }
            break;
        }
        case MHW_Sampler2Elements:
        case MHW_Sampler8Elements:
            dwOffset = pStateHeap->pCurMediaState->dwOffset +             // Offset to current media state base
                pStateHeap->dwOffsetSampler +                             // Go to base of the sampler area
                iMediaID   * pStateHeap->dwSizeSampler +                  // Go to base of media ID's sampler area
                iSamplerID * ElementSize[ElementType] * 16;               // Goto to "samplerID" sampler state
            break;

        case MHW_Sampler64Elements:
            dwOffset = pStateHeap->pCurMediaState->dwOffset +             // Offset to current media state base
                pStateHeap->dwOffsetSampler +                             // Go to base of the sampler area
                iMediaID   * pStateHeap->dwSizeSampler +                  // Go to base of media ID's sampler area
                iSamplerID * 32 * 16;                                     // Goto to "samplerID" sampler state
            break;

        case MHW_Sampler128Elements:
            if (pRenderHal->pRenderHalPltInterface->IsSampler128ElementsSupported())
            {
                dwOffset = pStateHeap->pCurMediaState->dwOffset +         // Offset to current media state base
                pStateHeap->dwOffsetSampler +                             // Go to base of the sampler area
                iMediaID   * pStateHeap->dwSizeSampler +                  // Go to base of media ID's sampler area
                iSamplerID * ElementSize[ElementType] * 16;                                        // Goto to "samplerID" sampler state
            } else
            {
                MHW_RENDERHAL_NORMALMESSAGE("Platform doesn't have any 128 element sampler, quit!");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
            }
            break;
        default:
            MHW_RENDERHAL_NORMALMESSAGE("Failed to parse sampler - invalid sampler type.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }
    }
    if (pdwSamplerOffset)
    {
        *pdwSamplerOffset = dwOffset;
    }

    if (ppSampler)
    {
        *ppSampler = (void *)(pStateHeap->pGshBuffer + dwOffset);
    }

finish:
    return eStatus;
}

//!
//! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW
//! \details    For BDW GT1/2/3 A0 steppings, per thread scratch space size in VFE state
//!             is 11 bits indicating [2k bytes, 2 Mbytes]: 0=2k, 1=4k, 2=8k � 10=2M
//!             BDW+ excluding A0 step is 12 bits indicating [1k bytes, 2 Mbytes]: 0=1k, 1=2k, 2=4k, 3=8k � 11=2M
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to RenderHal interface
//! \return     true if BDW A0 stepping, false otherwise
//!
bool RenderHal_PerThreadScratchSpaceStart2K(
    PRENDERHAL_INTERFACE pRenderHal)
{
    if (pRenderHal == nullptr || pRenderHal->pRenderHalPltInterface == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return false;
    }

    return pRenderHal->pRenderHalPltInterface->PerThreadScratchSpaceStart2K(pRenderHal);
}

//!
//! \brief    Encode SLM Size for Interface Descriptor
//! \details  Setup SLM size
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to RenderHal interface
//! \param    uint32_t SLMSize
//!             [in] SLM size in 1K
//! \return   encoded output
//!
uint32_t RenderHal_EncodeSLMSize(
    PRENDERHAL_INTERFACE pRenderHal,
    uint32_t             SLMSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    if (pRenderHal == nullptr || pRenderHal->pRenderHalPltInterface == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return 0;
    }

    return pRenderHal->pRenderHalPltInterface->EncodeSLMSize(SLMSize);
}

//!
//! \brief    Set Chroma Direction
//! \details  Setup Chroma Direction
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in]  Pointer to Hardware Interface
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in]  Pointer to Render Hal Surface
//! \return   uint8_t
//!
uint8_t RenderHal_SetChromaDirection(
    PRENDERHAL_INTERFACE pRenderHal,
    PRENDERHAL_SURFACE   pRenderHalSurface)
{
    if (pRenderHal == nullptr || pRenderHal->pRenderHalPltInterface == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return false;
    }

    return pRenderHal->pRenderHalPltInterface->SetChromaDirection(pRenderHal, pRenderHalSurface);
}

//!
//! \brief    Convert To Nano Seconds
//! \details  Convert to Nano Seconds
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint64_t iTicks
//!           [in] Ticks
//! \param    uint64_t *piNs
//!           [in] Nano Seconds
//! \return   void
//!
void RenderHal_ConvertToNanoSeconds(
    PRENDERHAL_INTERFACE                pRenderHal,
    uint64_t                            iTicks,
    uint64_t                            *piNs)
{
    if (pRenderHal == nullptr || pRenderHal->pRenderHalPltInterface == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return;
    }
    pRenderHal->pRenderHalPltInterface->ConvertToNanoSeconds(pRenderHal, iTicks, piNs);
}

//!
//! \brief    Enables L3 cacheing flag and sets related registers/values
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [in]  Pointer to Hardware Interface
//! \param    pCacheSettings
//!           [in] L3 Cache Configurations
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS RenderHal_EnableL3Caching(
    PRENDERHAL_INTERFACE                pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS        pCacheSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);

    MHW_RENDERHAL_CHK_STATUS( pRenderHal->pRenderHalPltInterface->EnableL3Caching(pRenderHal, pCacheSettings));
finish:
    return eStatus;
}

//!
//! \brief      Set L3 cache override config parameters
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in,out] pCacheSettings
//!             Pointer to pCacheSettings
//! \param      [in] bEnableSLM
//!             Flag to enable SLM
//! \return     MOS_STATUS
//!             MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS RenderHal_SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pRenderHalPltInterface->SetCacheOverrideParams(
        pRenderHal,
        pCacheSettings,
        bEnableSLM)
    );

    pCacheSettings->bOverride =
        pCacheSettings->bCntlRegOverride  ||
        pCacheSettings->bCntlReg2Override ||
        pCacheSettings->bCntlReg3Override ||
        pCacheSettings->bLra1RegOverride  ||
        pCacheSettings->bSqcReg1Override;

finish:
    return eStatus;
}

//!
//! \brief    Send Surface State Entry
//! \param    [in] pRenderHal
//!           pointer to render hal
//! \param    [in] pCmdBuffer
//!           pointer to command buffer
//! \param    [in] pParams
//!           pointer to surface state send parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS RenderHal_SendSurfaceStateEntry(
    PRENDERHAL_INTERFACE            pRenderHal,
    PMOS_COMMAND_BUFFER             pCmdBuffer,
    PMHW_SURFACE_STATE_SEND_PARAMS  pParams)
{
    //-----------------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(pParams);
    //-----------------------------------------------

    PMOS_INTERFACE     pOsInterface = pRenderHal->pOsInterface;
    uint8_t            *pSurfaceState = pParams->pSurfaceStateSource;
    int32_t            iSurfaceStateOffset = pParams->iSurfaceStateOffset;
    int32_t            iIndirectStateBase = pParams->iIndirectStateBase;

    SURFACE_STATE_TOKEN_COMMON *pSurfaceStateToken = (SURFACE_STATE_TOKEN_COMMON*)pParams->pSurfaceToken;

    uint32_t* pdwCmd = (uint32_t*)(pParams->pIndirectStateBase + iSurfaceStateOffset);

    // Copy surface state from system memory to graphics memory/indirect state
    if (pSurfaceStateToken->DW3.SurfaceStateType == MEDIASTATE_BTS_DEFAULT_TYPE)
    {
        MOS_SecureMemcpy(pdwCmd, pRenderHal->pHwSizes->dwSizeSurfaceState,
            pSurfaceState, pRenderHal->pHwSizes->dwSizeSurfaceState);

        // Patch offset is 8 DW from the surface state base
        pdwCmd += 8;
        iSurfaceStateOffset += 8 * sizeof(uint32_t);
    }
    else
    {
        MOS_SecureMemcpy(pdwCmd, pRenderHal->pHwSizes->dwSizeSurfaceStateAvs,
            pSurfaceState, pRenderHal->pHwSizes->dwSizeSurfaceStateAvs);

        // Patch offset is 6 DW from the surface state base
        pdwCmd += 6;
        iSurfaceStateOffset += 6 * sizeof(uint32_t);
    }

    if (pOsInterface->bUsesGfxAddress)
    {
        *pdwCmd = pSurfaceStateToken->DW4.SurfaceBaseAddress;
        *(pdwCmd + 1) = pSurfaceStateToken->DW5.SurfaceBaseAddress64;
    }

    MOS_PATCH_ENTRY_PARAMS PatchEntryParams;

    uint8_t *pbPtrCmdBuf = (uint8_t *)pCmdBuffer->pCmdBase;

    MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
    PatchEntryParams.uiAllocationIndex  = pSurfaceStateToken->DW1.SurfaceAllocationIndex;
    PatchEntryParams.uiResourceOffset = pSurfaceStateToken->DW2.SurfaceOffset;
    PatchEntryParams.uiPatchOffset    = iIndirectStateBase + iSurfaceStateOffset;
    PatchEntryParams.bWrite           = pSurfaceStateToken->DW3.RenderTargetEnable;
    PatchEntryParams.HwCommandType    = (MOS_HW_COMMAND)pSurfaceStateToken->DW0.DriverID;
    PatchEntryParams.forceDwordOffset = 0;
    PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
    PatchEntryParams.presResource     = (PMOS_RESOURCE)pSurfaceStateToken->pResourceInfo;

    // Set patch for surface state address
    pOsInterface->pfnSetPatchEntry(
        pOsInterface,
        &PatchEntryParams);

    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
    PMOS_RESOURCE pMosResource = (PMOS_RESOURCE)pSurfaceStateToken->pResourceInfo;
    if (pOsInterface->pfnGetMemoryCompressionMode)
    {
        pOsInterface->pfnGetMemoryCompressionMode(pOsInterface, pMosResource, &mmcMode);
    }

    if (mmcMode == MOS_MEMCOMP_RC && pSurfaceStateToken->DW3.SurfaceStateType == MEDIASTATE_BTS_DEFAULT_TYPE)
    {
# if !EMUL
        if (pOsInterface->bUsesGfxAddress)
        {
            uint64_t ui64GfxAddress = 0;
            ui64GfxAddress |= (uint64_t)(pSurfaceStateToken->DW5.SurfaceBaseAddress64 & 0x0000FFFF) << 32;
            ui64GfxAddress |= (uint64_t)(pSurfaceStateToken->DW4.SurfaceBaseAddress);
            pdwCmd = (uint32_t*)(pParams->pIndirectStateBase + pParams->iSurfaceStateOffset); //point to the start of current RENDER_SURFACE_STATE_CMD

            if (pMosResource->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS))
            {
                // Set GFX address of AuxiliarySurfaceBaseAddress
                uint64_t auxAddress = ui64GfxAddress + (uint64_t)pMosResource->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);
                *(pdwCmd + 10) = (*(pdwCmd + 10) & 0x00000FFF) | (uint32_t)(auxAddress & 0x00000000FFFFF000);
                *(pdwCmd + 11) = *(pdwCmd + 11) | (uint32_t)((auxAddress & 0x0000FFFF00000000) >> 32);
            }

            if (pMosResource->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC))
            {
                // Set GFX address of ClearAddress
                uint64_t clearAddress = ui64GfxAddress + (uint32_t)pMosResource->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC);
                *(pdwCmd + 12) = (*(pdwCmd + 12) & 0x0000001F) | (uint32_t)(clearAddress & 0x00000000FFFFFFE0);
                *(pdwCmd + 13) = *(pdwCmd + 13) | (uint32_t)((clearAddress & 0x0000FFFF00000000) >> 32);
            }
        }
        else
        {
            // Set patch for AuxiliarySurfaceBaseAddress
            MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
            PatchEntryParams.uiAllocationIndex  = pSurfaceStateToken->DW1.SurfaceAllocationIndex;
            PatchEntryParams.uiResourceOffset = pSurfaceStateToken->DW2.SurfaceOffset
                                                + (uint32_t)pMosResource->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);
            PatchEntryParams.uiPatchOffset    = iIndirectStateBase + pParams->iSurfaceStateOffset + 10 * sizeof(uint32_t);
            PatchEntryParams.bWrite           = pSurfaceStateToken->DW3.RenderTargetEnable;
            PatchEntryParams.HwCommandType    = (MOS_HW_COMMAND)pSurfaceStateToken->DW0.DriverID;
            PatchEntryParams.forceDwordOffset = 0;
            PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
            PatchEntryParams.presResource     = (PMOS_RESOURCE)pSurfaceStateToken->pResourceInfo;
            pOsInterface->pfnSetPatchEntry(pOsInterface, &PatchEntryParams);

            // Set patch for ClearAddress
            MOS_ZeroMemory(&PatchEntryParams, sizeof(PatchEntryParams));
            PatchEntryParams.uiAllocationIndex  = pSurfaceStateToken->DW1.SurfaceAllocationIndex;
            PatchEntryParams.uiResourceOffset = pSurfaceStateToken->DW2.SurfaceOffset
                                                + (uint32_t)pMosResource->pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC);
            PatchEntryParams.uiPatchOffset    = iIndirectStateBase + pParams->iSurfaceStateOffset + 12 * sizeof(uint32_t);
            PatchEntryParams.bWrite           = pSurfaceStateToken->DW3.RenderTargetEnable;
            PatchEntryParams.HwCommandType    = (MOS_HW_COMMAND)pSurfaceStateToken->DW0.DriverID;
            PatchEntryParams.forceDwordOffset = 0;
            PatchEntryParams.cmdBufBase       = pbPtrCmdBuf;
            PatchEntryParams.presResource     = (PMOS_RESOURCE)pSurfaceStateToken->pResourceInfo;
            pOsInterface->pfnSetPatchEntry(pOsInterface, &PatchEntryParams);
        }
#endif
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Init Special Interface
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//!
void RenderHal_InitInterfaceEx(PRENDERHAL_INTERFACE pRenderHal);

//!
//! \brief    Init Interface
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    MhwCpInterface** ppCpInterface
//!           [in/out] Pointer of pointer to MHW CP Interface Structure, which 
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_UNKNOWN : Invalid parameters
//!
MOS_STATUS RenderHal_InitInterface(
    PRENDERHAL_INTERFACE pRenderHal,
    MhwCpInterface       **ppCpInterface,
    PMOS_INTERFACE       pOsInterface)
{
    PMOS_USER_FEATURE_INTERFACE     pUserFeatureInterface = nullptr;
    MOS_USER_FEATURE                UserFeature;
    MOS_USER_FEATURE_VALUE          UserFeatureValue;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MHW_VFE_PARAMS                  *pVfeStateParams = nullptr;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(ppCpInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pOsInterface);
    //---------------------------------------

    // Basic initialization
    pRenderHal->pOsInterface                  = pOsInterface;

    pRenderHal->bKerneltimeDump        = false;
    MOS_ZeroMemory(&pRenderHal->kernelTime, sizeof(pRenderHal->kernelTime));

    pOsInterface->pfnGetPlatform(pOsInterface, &pRenderHal->Platform);

    pRenderHal->pSkuTable                     = pOsInterface->pfnGetSkuTable(pOsInterface);
    pRenderHal->pWaTable                      = pOsInterface->pfnGetWaTable(pOsInterface);

    // create mhw interfaces including mhw_render, cp, and mi
    MhwInterfaces::CreateParams params;
    MOS_ZeroMemory(&params, sizeof(params));
    params.Flags.m_render = true;
    params.m_heapMode = pRenderHal->bDynamicStateHeap;
    MhwInterfaces *mhwInterfaces =  MhwInterfaces::CreateFactory(params, pOsInterface);
    MHW_RENDERHAL_CHK_NULL(mhwInterfaces);
    MHW_RENDERHAL_CHK_NULL(mhwInterfaces->m_cpInterface);
    MHW_RENDERHAL_CHK_NULL(mhwInterfaces->m_miInterface);
    MHW_RENDERHAL_CHK_NULL(mhwInterfaces->m_renderInterface);
    pRenderHal->pCpInterface = mhwInterfaces->m_cpInterface;
    pRenderHal->pMhwMiInterface = mhwInterfaces->m_miInterface;
    pRenderHal->pMhwRenderInterface = mhwInterfaces->m_renderInterface;
    MOS_Delete(mhwInterfaces);

    // Set Cp Interface
    *ppCpInterface = pRenderHal->pCpInterface;

    // Initialize MHW State Heap Interface
    // Note: there are two pStateHeapInterface in below line. First one is pointer to legacy MHW_STATE_HEAP
    // sturcture, while the last one points to the new class.
    if (pRenderHal->pMhwRenderInterface->m_stateHeapInterface != nullptr)
    {
        pRenderHal->pMhwStateHeap = pRenderHal->pMhwRenderInterface->m_stateHeapInterface->pStateHeapInterface;
    }

    pRenderHal->pHwCaps  = pRenderHal->pMhwRenderInterface->GetHwCaps();

    pRenderHal->pHwSizes = pRenderHal->pMhwStateHeap->GetHwSizesPointer();

    pRenderHal->dwTimeoutMs            = RENDERHAL_TIMEOUT_MS_DEFAULT;
    pRenderHal->iMaxPalettes           = RENDERHAL_PALETTE_MAX;
    pRenderHal->iMaxPaletteEntries     = RENDERHAL_PALETTE_ENTRIES_MAX;
    pRenderHal->iMaxChromaKeys         = RENDERHAL_CHROMA_KEY_MAX;

    //set MDF load to default value false
    pRenderHal->IsMDFLoad = false;

    //set AVS to default value false
    pRenderHal->bIsAVS = false;

    pRenderHal->iChromaKeyCount        = 0;
    for (int i = 0; i < pRenderHal->iMaxChromaKeys; i++)
    {
        pRenderHal->ChromaKey[i].dwIndex = i;
    }

    // Read VDI Walker Regkey once during initialization
    MOS_ZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
    pUserFeatureInterface   = &pRenderHal->pOsInterface->UserFeatureInterface;
    UserFeature             = *pUserFeatureInterface->pUserFeatureInit;
    UserFeature.Type        = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath       = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues     = &UserFeatureValue;
    UserFeature.uiNumValues = 1;

    UserFeatureValue.u32Data = true;    // Init as default value
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_VDI_MODE,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);
#endif
    pRenderHal->bVDIWalker = UserFeature.pValues[0].u32Data ? true : false;

    UserFeatureValue.u32Data = MHW_WALKER_MODE_NOT_SET;    // Init as default value
#if (_DEBUG || _RELEASE_INTERNAL)
    // Read Media Walker Mode from RegKey once in initialization
    MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_WALKER_MODE,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);
#endif
    pRenderHal->MediaWalkerMode = (MHW_WALKER_MODE)UserFeature.pValues[0].u32Data;

    pRenderHal->pPlaneDefinitions             = g_cRenderHal_SurfacePlanes;

    // disable RenderHal kernel debugging.
    pRenderHal->bIsaAsmDebugEnable            = false;

    pRenderHal->iMaxPalettes                  = RENDERHAL_PALETTE_COUNT;
    pRenderHal->iMaxPaletteEntries            = RENDERHAL_PALETTE_ENTRIES;
    pRenderHal->iMaxChromaKeys                = RENDERHAL_CHROMA_KEY_COUNT;

    MOS_ZeroMemory(&pRenderHal->PredicationParams, sizeof(pRenderHal->PredicationParams));
    MOS_ZeroMemory(&pRenderHal->SetMarkerParams, sizeof(pRenderHal->SetMarkerParams));

    // CMFC CSC Coefficient Surface update
    pRenderHal->bCmfcCoeffUpdate              = false;
    pRenderHal->iKernelAllocationID           = RENDERHAL_KERNEL_LOAD_FAIL;
    pRenderHal->pCmfcCoeffSurface             = nullptr;

    // Initialization/Cleanup function
    pRenderHal->pfnInitialize                 = RenderHal_Initialize;
    pRenderHal->pfnDestroy                    = RenderHal_Destroy;

    // Allocate/Destroy state heaps
    pRenderHal->pfnAllocateStateHeaps         = RenderHal_AllocateStateHeaps;
    pRenderHal->pfnFreeStateHeaps             = RenderHal_FreeStateHeaps;

    // Slice Shutdown Mode
    pRenderHal->pfnSetSliceShutdownMode       = RenderHal_SetSliceShutdownMode;
    pRenderHal->pfnSetPowerOptionMode         = RenderHal_SetPowerOptionMode;

    // Preemption
    pRenderHal->pfnEnableGpgpuMiddleBatchBufferPreemption  =
         RenderHal_EnableGpgpuMiddleBatchBufferPreemption;
    pRenderHal->pfnEnableGpgpuMiddleThreadPreemption       =
         RenderHal_EnableGpgpuMiddleThreadPreemption;

    // Surface State and Binding Table management functions
    pRenderHal->pfnSetSurfacesPerBT           = RenderHal_SetSurfacesPerBT;
    pRenderHal->pfnGetSurfaceStateEntries     = RenderHal_GetSurfaceStateEntries;
    pRenderHal->pfnAssignSurfaceState         = RenderHal_AssignSurfaceState;
    pRenderHal->pfnGetAlignUnit               = RenderHal_GetAlignUnit;
    pRenderHal->pfnAdjustBoundary             = RenderHal_AdjustBoundary;
    pRenderHal->pfnAssignBindingTable         = RenderHal_AssignBindingTable;
    pRenderHal->pfnSetupBufferSurfaceState    = RenderHal_SetupBufferSurfaceState;
    pRenderHal->pfnSetupSurfaceStatesOs       = RenderHal_SetupSurfaceStatesOs;
    pRenderHal->pfnBindSurfaceState           = RenderHal_BindSurfaceState;
    pRenderHal->pfnSendSurfaces               = RenderHal_SendSurfaces_PatchList;
    pRenderHal->pfnSendSurfaceStateEntry      = RenderHal_SendSurfaceStateEntry;
    pRenderHal->pfnSetSurfaceStateToken       = RenderHal_SetSurfaceStateToken;
    pRenderHal->pfnSetSurfaceStateBuffer      = RenderHal_SetSurfaceStateBuffer;
    pRenderHal->pfnCalculateYOffset           = RenderHal_CalculateYOffset;

    // Media states management functions
    pRenderHal->pfnAllocateBB                 = RenderHal_AllocateBB;
    pRenderHal->pfnFreeBB                     = RenderHal_FreeBB;
    pRenderHal->pfnLockBB                     = RenderHal_LockBB;
    pRenderHal->pfnUnlockBB                   = RenderHal_UnlockBB;
    pRenderHal->pfnEnablePalette              = RenderHal_EnablePalette;
    pRenderHal->pfnGetPaletteEntry            = RenderHal_GetPaletteEntry;
    pRenderHal->pfnAllocatePaletteID          = RenderHal_AllocatePaletteID;
    pRenderHal->pfnFreePaletteID              = RenderHal_FreePaletteID;
    pRenderHal->pfnAllocateChromaKey          = RenderHal_AllocateChromaKey;
    pRenderHal->pfnAssignMediaState           = RenderHal_AssignMediaState;
    pRenderHal->pfnAllocateMediaID            = RenderHal_AllocateMediaID;
    pRenderHal->pfnGetMediaID                 = RenderHal_GetMediaID;

    // Old-style Kernel management functions
    pRenderHal->pfnRefreshSync                = RenderHal_RefreshSync;
    pRenderHal->pfnLoadKernel                 = RenderHal_LoadKernel;
    pRenderHal->pfnUnloadKernel               = RenderHal_UnloadKernel;
    pRenderHal->pfnResetKernels               = RenderHal_ResetKernels;
    pRenderHal->pfnTouchKernel                = RenderHal_TouchKernel;
    pRenderHal->pfnGetKernelOffset            = RenderHal_GetKernelOffset;

    // ISA ASM Debug support functions
    pRenderHal->pfnLoadDebugKernel            = RenderHal_LoadDebugKernel;
    pRenderHal->pfnLoadSipKernel              = RenderHal_LoadSipKernel;
    pRenderHal->pfnSendSipStateCmd            = RenderHal_SendSipStateCmd;

    // Command buffer programming functions
    pRenderHal->pfnLoadCurbeData              = RenderHal_LoadCurbeData;
    pRenderHal->pfnSendCurbeLoad              = RenderHal_SendCurbeLoad;
    pRenderHal->pfnSendMediaIdLoad            = RenderHal_SendMediaIdLoad;
    pRenderHal->pfnSendChromaKey              = RenderHal_SendChromaKey;
    pRenderHal->pfnSendPalette                = RenderHal_SendPalette;
    pRenderHal->pfnSendMediaStates            = RenderHal_SendMediaStates;
    pRenderHal->pfnSendStateBaseAddress       = RenderHal_SendStateBaseAddress;

    // Initialize OS dependent RenderHal Interfaces common to all platforms
    pRenderHal->pfnReset                      = RenderHal_Reset;
    pRenderHal->pfnAssignSshInstance          = RenderHal_AssignSshInstance;
    pRenderHal->pfnInitCommandBuffer          = RenderHal_InitCommandBuffer;
    pRenderHal->pfnSendTimingData             = RenderHal_SendTimingData;
    pRenderHal->pfnSendRcsStatusTag           = RenderHal_SendRcsStatusTag;
    pRenderHal->pfnSendSyncTag                = RenderHal_SendSyncTag;
    pRenderHal->pfnSendCscCoeffSurface        = RenderHal_SendCscCoeffSurface;

    // Tracker tag
    pRenderHal->pfnSetupPrologParams          = RenderHal_SetupPrologParams;

    // InterfaceDescriptor
    pRenderHal->pfnSetupInterfaceDescriptor   = RenderHal_SetupInterfaceDescriptor;

    // Media Walker
    pRenderHal->pfnGetMediaWalkerStatus       = RenderHal_GetMediaWalkerStatus;

    //Surface Memory Object Control
    pRenderHal->pfnGetSurfaceMemoryObjectControl
                                              = RenderHal_GetSurfaceMemoryObjectControl;
    //Scratch Space
    pRenderHal->pfnGetScratchSpaceSize        = RenderHal_GetScratchSpaceSize;

    // Other states
    pRenderHal->pfnSetVfeStateParams          = RenderHal_SetVfeStateParams;
    pRenderHal->pfnSetSamplerStates           = RenderHal_SetSamplerStates;

    pRenderHal->pfnIs2PlaneNV12Needed         = RenderHal_Is2PlaneNV12Needed;

    // Initialize hardware resources for the current Os/Platform
    pRenderHal->pRenderHalPltInterface = RenderHalDevice::CreateFactory(pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);

    // Set the platform-specific fields in renderhal
    // Set State Heap settings
    pRenderHal->pRenderHalPltInterface->InitStateHeapSettings(pRenderHal);

    // Set default / advanced surface types
    pRenderHal->pRenderHalPltInterface->InitSurfaceTypes(pRenderHal);

    // Set MMC Enabled status
    pRenderHal->pRenderHalPltInterface->IsRenderHalMMCEnabled(pRenderHal);

    // Set platform dependent parameters
    pRenderHal->bHasCombinedAVSSamplerState   = true;
    pRenderHal->bEnableYV12SinglePass         = pRenderHal->pRenderHalPltInterface->IsEnableYV12SinglePass(pRenderHal);
    pRenderHal->dwSamplerAvsIncrement         = pRenderHal->pRenderHalPltInterface->GetSizeSamplerStateAvs(pRenderHal);
    pRenderHal->bComputeContextInUse          = pRenderHal->pRenderHalPltInterface->IsComputeContextInUse(pRenderHal);

    pRenderHal->dwMaskCrsThdConDataRdLn       = (uint32_t) -1;
    pRenderHal->dwMinNumberThreadsInGroup     = 1;
    pRenderHal->dwCurbeBlockAlign             = RENDERHAL_CURBE_BLOCK_ALIGN;
    pRenderHal->dwScratchSpaceMaxThreads      = pRenderHal->pHwCaps->dwMaxThreads;

    // Set palette data
    for (int i = 0; i < pRenderHal->iMaxPalettes; i++)
    {
        pRenderHal->Palette[i].iPaletteID   = i;
        pRenderHal->Palette[i].iNumEntries  = 0;
        pRenderHal->Palette[i].pPaletteData =
            pRenderHal->pRenderHalPltInterface->GetPaletteDataAddress(i);
    }

    // Set default VFEState Params
    pVfeStateParams = pRenderHal->pRenderHalPltInterface->GetVfeStateParameters();
    pVfeStateParams->dwDebugCounterControl    = MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING;
    pVfeStateParams->dwMaximumNumberofThreads = pRenderHal->pHwCaps->dwMaxThreads;

    // Hardware dependent calls
    pRenderHal->pfnGetSamplerOffsetAndPtr     = RenderHal_GetSamplerOffsetAndPtr;
    pRenderHal->pfnSetupSurfaceState          = RenderHal_SetupSurfaceState;
    pRenderHal->pfnEncodeSLMSize              = RenderHal_EncodeSLMSize;
    pRenderHal->pfnSetChromaDirection         = RenderHal_SetChromaDirection;
    pRenderHal->pfnEnableL3Caching            = RenderHal_EnableL3Caching;
    pRenderHal->pfnSetCacheOverrideParams     = RenderHal_SetCacheOverrideParams;

    // CM specific function
    pRenderHal->pfnConvertToNanoSeconds       = RenderHal_ConvertToNanoSeconds;

    pRenderHal->pfnPerThreadScratchSpaceStart2K = RenderHal_PerThreadScratchSpaceStart2K;

    // Special functions
    RenderHal_InitInterfaceEx(pRenderHal);

finish:
    return eStatus;
}

//!
//! \brief    Set Surface for HW Access
//! \details  Common Function for setting up surface state
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to RenderHal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams
//!           [in] Pointer to Surface Params
//! \param    int32_t iBindingTable
//!           [in] Binding Table to bind surface
//! \param    int32_t iBTEntry
//!           [in] Binding Table Entry index
//! \param    bool bWrite
//!           [in] Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS RenderHal_SetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
    int32_t                         iBindingTable,
    int32_t                         iBTEntry,
    bool                            bWrite)
{

    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                         iSurfaceEntries;
    int32_t                         i;
    MOS_STATUS                      eStatus;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //---------------------------------------

    // Initialize Variables
    eStatus                 = MOS_STATUS_SUCCESS;
    pOsInterface            = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pRenderHalSurface->OsSurface.OsResource,
        bWrite,
        true));

    // Setup surface states-----------------------------------------------------
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        pRenderHalSurface,
        pSurfaceParams,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr));

    // Bind surface states------------------------------------------------------
    for (i = 0; i < iSurfaceEntries; i++, iBTEntry++)
    {
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            iBTEntry,
            pSurfaceEntries[i]));
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Buffer Surface for HW Access
//! \details  Common Function for setting up buffer surface state
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to RenderHal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams
//!           [in] Pointer to Surface Params
//! \param    int32_t iBindingTable
//!           [in] Binding Table to Bind Surface
//! \param    int32_t iBTEntry
//!           [in] Binding Table Entry index
//! \param    bool bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS RenderHal_SetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
    int32_t                         iBindingTable,
    int32_t                         iBTEntry,
    bool                            bWrite)
{
    PMOS_INTERFACE                  pOsInterface;
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    MOS_STATUS                      eStatus;

    //---------------------------------------
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    //---------------------------------------

    // Initialize Variables
    eStatus                         = MOS_STATUS_SUCCESS;
    pOsInterface                    = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    MHW_RENDERHAL_CHK_STATUS(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pRenderHalSurface->OsSurface.OsResource,
        bWrite,
        true));

    // Setup Buffer surface-----------------------------------------------------
    if (pSurfaceParams == nullptr)
    {
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
        pSurfaceParams = &SurfaceParam;
    }

    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetupBufferSurfaceState(
        pRenderHal,
        pRenderHalSurface,
        pSurfaceParams,
        &pSurfaceEntry));

    // Bind surface state-------------------------------------------------------
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnBindSurfaceState(
        pRenderHal,
        iBindingTable,
        iBTEntry,
        pSurfaceEntry));

finish:
    return eStatus;
}

