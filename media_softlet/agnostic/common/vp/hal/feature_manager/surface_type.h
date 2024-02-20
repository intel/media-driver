/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     surface_type.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __SURFACE_TYPE_H__
#define __SURFACE_TYPE_H__

namespace vp
{
// Need to make sure maximum size <256
// If exceed 256, need to reserve extra bit
#define SURFACETYPE_SIZE32 31
#define SURFACETYPE_SIZE16 15
#define SURFACETYPE_SIZE5 4
#define SURFACETYPE_SIZE10 9
#define SURFACETYPE_SIZE8 7
#define SURFACETYPE_SIZE64 63
#define SURFACETYPE_SIZE4 3
#define SURFACETYPE_SIZE3 2
#define GFSURFACE_COUNT 8
#define MOTION_CHANNEL 3
#define MAX_MODELSURFACE_COUNT 85

#define SUFACE_MASK 0xffffff00
#define SUFACE_TYPE_ASSIGNED(type) (((type)&SUFACE_MASK) >> 8)

enum SurfaceType
{
    SurfaceTypeInvalid                  = 0x0,
    SurfaceTypeVeboxInput               = 0x100,
    SurfaceTypeVeboxPreviousInput       = 0x200,
    SurfaceTypeDNOutput                 = 0x300,
    SurfaceTypeVeboxCurrentOutput       = 0x400,
    SurfaceTypeVeboxPreviousOutput      = 0x500,
    SurfaceTypeScalar                   = 0x600,
    SurfaceTypeSTMMIn                   = 0x700,
    SurfaceTypeSTMMOut                  = 0x800,
    SurfaceTypeACEHistory               = 0x900,
    SurfaceTypeFMDHistory               = 0xa00,
    SurfaceTypeLaceAceRGBHistogram      = 0xb00,
    SurfaceTypeLaceLut                  = 0xc00,
    SurfaceTypeStatistics               = 0xd00,
    SurfaceTypeSkinScore                = 0xe00,
    SurfaceType3DLut                    = 0xf00,
    SurfaceType1k1dLut                  = 0x1000,
    SurfaceType1dLutHDR                 = 0x1100,
    SurfaceTypeAlphaOrVignette          = 0x1200,
    SurfaceTypeVeboxStateHeap_Drv       = 0x1300,
    SurfaceTypeVeboxStateHeap_Knr       = 0x1400,
    SurfaceTypeAutoDNNoiseLevel         = 0x1500,  // with kernel path needed
    SurfaceTypeAutoDNSpatialConfig      = 0x1600,
    SurfaceTypeRenderInput              = 0x1700,
    SurfaceTypeRenderOutput             = 0x1800,
    SurfaceTypeAggregatedHistogram      = 0x1900,
    SurfaceTypeFrameHistogram           = 0x1a00,
    SurfaceTypeStdStatistics            = 0x1b00,
    SurfaceTypePwlfIn                   = 0x1c00,
    SurfaceTypePwlfOut                  = 0x1d00,
    SurfaceTypeWeitCoef                 = 0x1e00,
    SurfaceTypGlobalToneMappingCurveLUT = 0x1f00,

    // FC
    SurfaceTypeFcInputLayer0             = 0x2000,
    SurfaceTypeFcInputLayerMax           = SurfaceTypeFcInputLayer0 + SURFACETYPE_SIZE8,
    SurfaceTypeFcInputLayer0Field1Dual   = 0x2100,
    SurfaceTypeFcInputLayerMaxField1Dual = SurfaceTypeFcInputLayer0Field1Dual + SURFACETYPE_SIZE8,
    SurfaceTypeFcTarget0                 = 0x2200,
    SurfaceTypeFcTarget1                 = 0x2300,
    SurfaceTypeFcCscCoeff                = 0x2400,
    SurfaceTypeDecompressionSync         = 0x2500,

    // 3DLut Kernel
    SurfaceType3DLut2D   = 0x2600,
    SurfaceType3DLutCoef = 0x2700,

    // HVS Kernel
    SurfaceTypeHVSTable = 0x2800,

    // HDR Kernel
    SurfaceTypeHdrInputLayer0            = 0x2900,
    SurfaceTypeHdrInputLayerMax          = SurfaceTypeHdrInputLayer0 + SURFACETYPE_SIZE8,
    SurfaceTypeHdrOETF1DLUTSurface0      = 0x2a00,
    SurfaceTypeHdrOETF1DLUTSurfaceMax    = SurfaceTypeHdrOETF1DLUTSurface0 + SURFACETYPE_SIZE8,
    SurfaceTypeHdrCRI3DLUTSurface0       = 0x2b00,
    SurfaceTypeHdrCRI3DLUTSurfaceMax     = SurfaceTypeHdrCRI3DLUTSurface0 + SURFACETYPE_SIZE8,
    SurfaceTypeHdrTarget0                = 0x2c00,
    SurfaceTypeHdrCoeff                  = 0x2d00,
    SurfaceTypeHdrAutoModeCoeff          = 0x2e00,
    SurfaceTypeHdrAutoModeIirTempSurface = 0x2f00,
    NumberOfSurfaceTypeBase              = 0x3000,
#ifdef _MEDIA_RESERVED
    #include "surface_type_ext.h"
#endif
    NumberOfSurfaceType
};
}  // namespace vp
#endif  // !__SURFACE_TYPE_H__
