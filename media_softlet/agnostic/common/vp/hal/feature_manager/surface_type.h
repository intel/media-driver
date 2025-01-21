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
    SurfaceTypeSubPlane                 = 0x100,
    SurfaceTypeGeneralEnd               = 0x200, // for general usage

    SurfaceTypeVeboxInput               = SurfaceTypeGeneralEnd + 0x100,
    SurfaceTypeVeboxPreviousInput       = SurfaceTypeGeneralEnd + 0x200,
    SurfaceTypeDNOutput                 = SurfaceTypeGeneralEnd + 0x300,
    SurfaceTypeVeboxCurrentOutput       = SurfaceTypeGeneralEnd + 0x400,
    SurfaceTypeVeboxPreviousOutput      = SurfaceTypeGeneralEnd + 0x500,
    SurfaceTypeScalar                   = SurfaceTypeGeneralEnd + 0x600,
    SurfaceTypeSTMMIn                   = SurfaceTypeGeneralEnd + 0x700,
    SurfaceTypeSTMMOut                  = SurfaceTypeGeneralEnd + 0x800,
    SurfaceTypeACEHistory               = SurfaceTypeGeneralEnd + 0x900,
    SurfaceTypeFMDHistory               = SurfaceTypeGeneralEnd + 0xa00,
    SurfaceTypeLaceAceRGBHistogram      = SurfaceTypeGeneralEnd + 0xb00,
    SurfaceTypeLaceLut                  = SurfaceTypeGeneralEnd + 0xc00,
    SurfaceTypeStatistics               = SurfaceTypeGeneralEnd + 0xd00,
    SurfaceTypeSkinScore                = SurfaceTypeGeneralEnd + 0xe00,
    SurfaceType3DLut                    = SurfaceTypeGeneralEnd + 0xf00,
    SurfaceType1k1dLut                  = SurfaceTypeGeneralEnd + 0x1000,
    SurfaceType1dLutHDR                 = SurfaceTypeGeneralEnd + 0x1100,
    SurfaceTypeAlphaOrVignette          = SurfaceTypeGeneralEnd + 0x1200,
    SurfaceTypeVeboxStateHeap_Drv       = SurfaceTypeGeneralEnd + 0x1300,
    SurfaceTypeVeboxStateHeap_Knr       = SurfaceTypeGeneralEnd + 0x1400,
    SurfaceTypeAutoDNNoiseLevel         = SurfaceTypeGeneralEnd + 0x1500,  // with kernel path needed
    SurfaceTypeAutoDNSpatialConfig      = SurfaceTypeGeneralEnd + 0x1600,
    SurfaceTypeRenderInput              = SurfaceTypeGeneralEnd + 0x1700,
    SurfaceTypeRenderOutput             = SurfaceTypeGeneralEnd + 0x1800,
    SurfaceTypeAggregatedHistogram      = SurfaceTypeGeneralEnd + 0x1900,
    SurfaceTypeFrameHistogram           = SurfaceTypeGeneralEnd + 0x1a00,
    SurfaceTypeStdStatistics            = SurfaceTypeGeneralEnd + 0x1b00,
    SurfaceTypePwlfIn                   = SurfaceTypeGeneralEnd + 0x1c00,
    SurfaceTypePwlfOut                  = SurfaceTypeGeneralEnd + 0x1d00,
    SurfaceTypeWeitCoef                 = SurfaceTypeGeneralEnd + 0x1e00,
    SurfaceTypGlobalToneMappingCurveLUT = SurfaceTypeGeneralEnd + 0x1f00,
    SurfaceTypeFixFuncEnd               = SurfaceTypeGeneralEnd + 0x2000, //for fix function usage

    // FC
    SurfaceTypeFcInputLayer0                     = SurfaceTypeFixFuncEnd + 0x100,
    SurfaceTypeFcInputLayerMax                   = SurfaceTypeFcInputLayer0 + SURFACETYPE_SIZE8,
    SurfaceTypeFcInputLayer0Field1Dual           = SurfaceTypeFixFuncEnd + 0x200,
    SurfaceTypeFcInputLayerMaxField1Dual         = SurfaceTypeFcInputLayer0Field1Dual + SURFACETYPE_SIZE8,
    SurfaceTypeFcTarget0                         = SurfaceTypeFixFuncEnd + 0x300,
    SurfaceTypeFcTarget1                         = SurfaceTypeFixFuncEnd + 0x400,
    SurfaceTypeFcCscCoeff                        = SurfaceTypeFixFuncEnd + 0x500,
    SurfaceTypeDecompressionSync                 = SurfaceTypeFixFuncEnd + 0x600,
    SurfaceTypeFcIntermediaInput                 = SurfaceTypeFixFuncEnd + 0x700,
    SurfaceTypeFcSeparateIntermediaInputSecPlane = SurfaceTypeFixFuncEnd + 0x800,
    SurfaceTypeFcIntermediaOutput                = SurfaceTypeFixFuncEnd + 0x900,
    SurfaceTypeFcEnd                             = SurfaceTypeFixFuncEnd + 0xa00,

    // 3DLut Kernel
    SurfaceType3DLut2D                   = SurfaceTypeFcEnd + 0x100,
    SurfaceType3DLutCoef                 = SurfaceTypeFcEnd + 0x200,
    SurfaceType3DLutEnd                  = SurfaceTypeFcEnd + 0x300,

    // HVS Kernel
    SurfaceTypeHVSTable                  = SurfaceType3DLutEnd + 0x100,
    SurfaceTypeHVSEnd                    = SurfaceType3DLutEnd + 0x200,

    // HDR Kernel
    SurfaceTypeHdrInputLayer0            = SurfaceTypeHVSEnd + 0x100,
    SurfaceTypeHdrInputLayerMax          = SurfaceTypeHdrInputLayer0 + SURFACETYPE_SIZE8,
    SurfaceTypeHdrOETF1DLUTSurface0      = SurfaceTypeHVSEnd + 0x200,
    SurfaceTypeHdrOETF1DLUTSurfaceMax    = SurfaceTypeHdrOETF1DLUTSurface0 + SURFACETYPE_SIZE8,
    SurfaceTypeHdrCRI3DLUTSurface0       = SurfaceTypeHVSEnd + 0x300,
    SurfaceTypeHdrCRI3DLUTSurfaceMax     = SurfaceTypeHdrCRI3DLUTSurface0 + SURFACETYPE_SIZE8,
    SurfaceTypeHdrTarget0                = SurfaceTypeHVSEnd + 0x400,
    SurfaceTypeHdrCoeff                  = SurfaceTypeHVSEnd + 0x500,
    SurfaceTypeHdrAutoModeCoeff          = SurfaceTypeHVSEnd + 0x600,
    SurfaceTypeHdrAutoModeIirTempSurface = SurfaceTypeHVSEnd + 0x700,
    SurfaceTypeInnerTileConvertInput     = SurfaceTypeHVSEnd + 0x800,  // Intermedia surface between VESFC states and tile convert states inside single batchbuffer.
    SurfaceTpyeHdrEnd                    = SurfaceTypeHVSEnd + 0x900,

    // AI Kernel Common
    SurfaceTypeAiInput0                  = SurfaceTpyeHdrEnd + 0x100,
    SurfaceTypeAiTarget0                 = SurfaceTpyeHdrEnd + 0x200,
    SurfaceTypeAiEnd                     = SurfaceTpyeHdrEnd + 0x200,

    NumberOfSurfaceTypeBase              = SurfaceTypeAiEnd + 0x900,

#ifdef _MEDIA_RESERVED
    #include "surface_type_ext.h"
#endif
    NumberOfSurfaceType
};
}  // namespace vp
#endif  // !__SURFACE_TYPE_H__
