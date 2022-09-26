/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     sw_filter.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __SW_FILTER_H__
#define __SW_FILTER_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_render_common.h"
#include <vector>
#include "media_sfc_interface.h"

namespace vp
{
class VpInterface;
class SwFilterSubPipe;

#define FEATURE_TYPE_ENGINE_BITS_SFC        0x20
#define FEATURE_TYPE_ENGINE_BITS_VEBOX      0x40
#define FEATURE_TYPE_ENGINE_BITS_RENDER     0x80

#define FEATURE_TYPE_ENGINE_BITS_SUB_STEP   0x01

#define IS_FEATURE_TYPE_ON_SFC(type)        ((type)&FEATURE_TYPE_ENGINE_BITS_SFC)
#define IS_FEATURE_TYPE_ON_VEBOX(type)      ((type)&FEATURE_TYPE_ENGINE_BITS_VEBOX)
#define IS_FEATURE_TYPE_ON_VEBOX_SFC(type)  (IS_FEATURE_TYPE_ON_SFC(type) || IS_FEATURE_TYPE_ON_VEBOX(type))
#define IS_FEATURE_TYPE_ON_RENDER(type)     ((type)&FEATURE_TYPE_ENGINE_BITS_RENDER)

#define FEATURE_TYPE_MASK                   0xffffff00
#define FEATURE_TYPE_ENGINE_ASSIGNED(feature) (((feature)&FEATURE_TYPE_MASK) != (feature))

#define SURFACETYPE_SIZE32                  31
#define SURFACETYPE_SIZE16                  15
#define SURFACETYPE_SIZE5                   4
#define SURFACETYPE_SIZE10                  9

#define GFSURFACE_COUNT 8
#define MOTION_CHANNEL 3
#define MAX_MODELSURFACE_COUNT 85

#define VPHAL_MAX_HDR_INPUT_LAYER           8
#define VPHAL_MAX_HDR_OUTPUT_LAYER          1
#define VPHAL_HDR_BTINDEX_LAYER0            16
#define VPHAL_HDR_BTINDEX_PER_LAYER0        5
#define VPHAL_HDR_BTINDEX_RENDERTARGET      56
#define VPHAL_HDR_BTINDEX_PER_TARGET        3
#define VPHAL_HDR_SAMPLER8X8_TABLE_NUM      2
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))

enum FeatureType
{
    FeatureTypeInvalid          = 0,
    FeatureTypeCsc              = 0x100,
    FeatureTypeCscOnSfc         = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeCscOnVebox       = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeCscOnRender      = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeRotMir           = 0x200,
    FeatureTypeRotMirOnSfc      = FeatureTypeRotMir | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeRotMirOnRender   = FeatureTypeRotMir | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeScaling          = 0x300,
    FeatureTypeScalingOnSfc     = FeatureTypeScaling | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeScalingOnRender  = FeatureTypeScaling | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeDn               = 0x400,
    FeatureTypeDnOnVebox        = FeatureTypeDn | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeDnHVSCalOnRender = FeatureTypeDn | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeDi               = 0x500,
    FeatureTypeDiOnVebox        = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeDiOnRender       = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeDiFmdOnRender    = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeSte              = 0x600,
    FeatureTypeSteOnVebox       = FeatureTypeSte | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeAce              = 0x700,
    FeatureTypeAceOnVebox       = FeatureTypeAce | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeVeboxUpdate      = 0x800,
    FeatureTypeVeboxUpdateOnRender = FeatureTypeVeboxUpdate | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeTcc              = 0x900,
    FeatureTypeTccOnVebox       = FeatureTypeTcc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeProcamp          = 0xA00,
    FeatureTypeProcampOnVebox   = FeatureTypeProcamp | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeProcampOnRender  = FeatureTypeProcamp | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeCgc              = 0xB00,
    FeatureTypeCgcOnVebox       = FeatureTypeCgc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeHdr              = 0xC00,
    FeatureTypeHdrOnVebox       = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeHdr3DLutCalOnRender = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeHdrOnRender      = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeFD               = 0xD00,
    FeatureTypeFLD              = 0xE00,
    FeatureTypeFB               = 0xF00,
    FeatureTypeSecureCopy       = 0x1000,
    FeatureTypeSecureCopyOnRender = FeatureTypeSecureCopy | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeSR               = 0x1100,
    FeatureTypeSROnRender       = FeatureTypeSR | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeLace             = 0x1200,
    FeatureTypeLaceOnVebox      = FeatureTypeLace | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeLaceOnRender     = FeatureTypeLace | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeGamutExt         = 0x1300,
    FeatureTypeDV               = 0x1400,
    FeatureTypeDVOnVebox        = FeatureTypeDV | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeFc               = 0x1500,
    FeatureTypeFcOnRender       = FeatureTypeFc | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeLumakey          = 0x1600,
    FeatureTypeLumakeyOnRender  = FeatureTypeLumakey | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeBlending         = 0x1700,
    FeatureTypeBlendingOnRender = FeatureTypeBlending | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeColorFill        = 0x1800,
    FeatureTypeColorFillOnSfc   = FeatureTypeColorFill | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeColorFillOnRender = FeatureTypeColorFill | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeAlpha            = 0x1900,
    FeatureTypeAlphaOnSfc       = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeAlphaOnVebox     = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeAlphaOnRender    = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeCappipe          = 0x2000,
    FeatureTypeCappipeOnVebox   = FeatureTypeCappipe | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeCappipeOnRender  = FeatureTypeCappipe | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeFDFB             = 0x2100,
    FeatureTypeFDFBOnVebox      = FeatureTypeFDFB | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeFDFBOnRender     = FeatureTypeFDFB | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeSegmentation     = 0x2200,
    FeatureTypeSegmentationOnRender = FeatureTypeSegmentation | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeS3D                  = 0x2300,
    // ...
    NumOfFeatureType
};

enum RenderTargetType
{
    RenderTargetTypeInvalid = 0,
    RenderTargetTypeSurface,
    RenderTargetTypeParameter
};

enum SurfaceType
{
    SurfaceTypeInvalid = 0,
    SurfaceTypeVeboxInput,
    SurfaceTypeVeboxPreviousInput,
    SurfaceTypeDNOutput,
    SurfaceTypeVeboxCurrentOutput,
    SurfaceTypeVeboxPreviousOutput,
    SurfaceTypeScalar,
    SurfaceTypeSTMMIn,
    SurfaceTypeSTMMOut,
    SurfaceTypeACEHistory,
    SurfaceTypeFMDHistory,
    SurfaceTypeLaceAceRGBHistogram,
    SurfaceTypeLaceLut,
    SurfaceTypeStatistics,
    SurfaceTypeSkinScore,
    SurfaceType3DLut,
    SurfaceType1k1dLut,
    SurfaceType1dLutHDR,
    SurfaceTypeAlphaOrVignette,
    SurfaceTypeVeboxStateHeap_Drv,
    SurfaceTypeVeboxStateHeap_Knr,
    SurfaceTypeAutoDNNoiseLevel, // with kernel path needed
    SurfaceTypeAutoDNSpatialConfig,
    SurfaceTypeRenderInput,
    SurfaceTypeRenderOutput,
    SurfaceTypeRenderSRInput, //Super Resolution related Surface and Buffer index Reserved
    SurfaceTypeRenderSRBuffer = SurfaceTypeRenderSRInput + 0x100,
    SurfaceTypeRenderSRMax = SurfaceTypeRenderSRBuffer + 0x100,
    SurfaceTypeAggregatedHistogram,
    SurfaceTypeFrameHistogram,
    SurfaceTypeStdStatistics,
    SurfaceTypePwlfIn,
    SurfaceTypePwlfOut,
    SurfaceTypeWeitCoef,
    SurfaceTypGlobalToneMappingCurveLUT,
    // FC
    SurfaceTypeFcInputLayer0,
    SurfaceTypeFcInputLayer1,
    SurfaceTypeFcInputLayer2,
    SurfaceTypeFcInputLayer3,
    SurfaceTypeFcInputLayer4,
    SurfaceTypeFcInputLayer5,
    SurfaceTypeFcInputLayer6,
    SurfaceTypeFcInputLayer7,
    SurfaceTypeFcInputLayerMax = SurfaceTypeFcInputLayer7,
    SurfaceTypeFcInputLayer0Field1Dual,
    SurfaceTypeFcInputLayer1Field1Dual,
    SurfaceTypeFcInputLayer2Field1Dual,
    SurfaceTypeFcInputLayer3Field1Dual,
    SurfaceTypeFcInputLayer4Field1Dual,
    SurfaceTypeFcInputLayer5Field1Dual,
    SurfaceTypeFcInputLayer6Field1Dual,
    SurfaceTypeFcInputLayer7Field1Dual,
    SurfaceTypeFcInputLayerMaxField1Dual = SurfaceTypeFcInputLayer7Field1Dual,
    SurfaceTypeFcTarget0,
    SurfaceTypeFcTarget1,
    SurfaceTypeFcCscCoeff,
    // LGCA related Surfaces
    SurfaceTypeSamplerSurfaceR,
    SurfaceTypeSamplerSurfaceG,
    SurfaceTypeSamplerSurfaceB,
    SurfaceTypeOutputSurfaceR,
    SurfaceTypeOutputSurfaceG,
    SurfaceTypeOutputSurfaceB,
    SurfaceTypeSamplerParamsMinMax,
    // 3DLut Kernel
    SurfaceType3DLut2D,
    SurfaceType3DLutCoef,
    // FDFB
    SurfaceTypeFDFBSrcHandle,
    SurfaceTypeFDFBOutFaceBuffer,
    SurfaceTypeFDFBOutFaceCountBuffer,
    SurfaceTypeFDFBOutFaceViewBuffer,
    SurfaceTypeFDFBFrameCount,
    SurfaceTypeFDFBFaceFlag,
    SurfaceTypeFDFBValidatorFlag,
    SurfaceTypeFDFBSkipFB,
    SurfaceTypeFDFBConfidenceOut,
    SurfaceTypeFDFBExitFlag,
    SurfaceTypeFDFBFDExitFlag,
    SurfaceTypeFDFBStrength2,
    SurfaceTypeFDFBRemappedLM,
    SurfaceTypeFDFBPostProcessingOutCur,
    SurfaceTypeFDFBPreFaceCountBuffer,
    //FD
    SurfaceTypeFDLutListBuffer,
    SurfaceTypeFDLutListBuffer2,
    SurfaceTypeFDLutListBuffer3,
    SurfaceTypeFDLutListBuffer4,
    SurfaceTypeFDLutListBuffer5,
    SurfaceTypeFDLutListBuffer6,
    SurfaceTypeFDLutListBuffer7,
    SurfaceTypeFDLutListBuffer8,
    SurfaceTypeFDLutListBuffer9,
    SurfaceTypeFDLutListBuffer10,
    SurfaceTypeFDLutListBuffer11,
    SurfaceTypeFDLutListBuffer12,
    SurfaceTypeFDLutListBuffer13,
    SurfaceTypeFDLutListBuffer14,
    SurfaceTypeFDLutListBuffer15,
    SurfaceTypeFDLutListBuffer16,
    SurfaceTypeFDLutListBuffer17,
    SurfaceTypeFDLutListBuffer18,
    SurfaceTypeFDLutListBuffer19,
    SurfaceTypeFDLutListBuffer20,
    SurfaceTypeFDLutListBuffer21,
    SurfaceTypeFDLutListBuffer22,
    SurfaceTypeFDLutListBuffer23,
    SurfaceTypeFDLutListBuffer24,
    SurfaceTypeFDLutListBuffer25,
    SurfaceTypeFDLutListBuffer26,
    SurfaceTypeFDLutListBuffer27,
    SurfaceTypeFDLutListBuffer28,
    SurfaceTypeFDLutListBuffer29,
    SurfaceTypeFDLutListBuffer30,
    SurfaceTypeFDLutListBuffer31,
    SurfaceTypeFDLutListBuffer32,
    SurfaceTypeFDLutListBuffer33,
    SurfaceTypeFDLutListBuffer34,
    SurfaceTypeFDLutListBuffer35,
    SurfaceTypeFDLutListBuffer36,
    SurfaceTypeFDLutListBuffer37,
    SurfaceTypeFDLutListBuffer38,
    SurfaceTypeFDLutListBuffer39,
    SurfaceTypeFDLutListBuffer40,
    SurfaceTypeFDLutListBuffer41,
    SurfaceTypeFDLutListBuffer42,
    SurfaceTypeFDLutListBuffer43,
    SurfaceTypeFDLutListBuffer44,
    SurfaceTypeFDLutListBuffer45,
    SurfaceTypeFDLutListBuffer46,
    SurfaceTypeFDLutListBuffer47,
    SurfaceTypeFDLutListBuffer48,
    SurfaceTypeFDLutListBuffer49,
    SurfaceTypeFDLutListBuffer50,
    SurfaceTypeFDLutListBuffer51,
    SurfaceTypeFDLutListBuffer52,
    SurfaceTypeFDLutListBuffer53,
    SurfaceTypeFDLutListBuffer54,
    SurfaceTypeFDLutListBuffer55,
    SurfaceTypeFDLutListBuffer56,
    SurfaceTypeFDLutListBuffer57,
    SurfaceTypeFDLutListBuffer58,
    SurfaceTypeFDLutListBuffer59,
    SurfaceTypeFDLutListBuffer60,
    SurfaceTypeFDLutListBuffer61,
    SurfaceTypeFDLutListBuffer62,
    SurfaceTypeFDLutListBuffer63,
    SurfaceTypeFDLutListBuffer64,
    SurfaceTypeFDLutCounterBuffer,
    SurfaceTypeFDLutCounterBuffer2,
    SurfaceTypeFDLutCounterBuffer3,
    SurfaceTypeFDLutCounterBuffer4,
    SurfaceTypeFDLutCounterBuffer5,
    SurfaceTypeFDLutCounterBuffer6,
    SurfaceTypeFDLutCounterBuffer7,
    SurfaceTypeFDLutCounterBuffer8,
    SurfaceTypeFDLutCounterBuffer9,
    SurfaceTypeFDLutCounterBuffer10,
    SurfaceTypeFDLutCounterBuffer11,
    SurfaceTypeFDLutCounterBuffer12,
    SurfaceTypeFDLutCounterBuffer13,
    SurfaceTypeFDLutCounterBuffer14,
    SurfaceTypeFDLutCounterBuffer15,
    SurfaceTypeFDLutCounterBuffer16,
    SurfaceTypeFDLutCounterBuffer17,
    SurfaceTypeFDLutCounterBuffer18,
    SurfaceTypeFDLutCounterBuffer19,
    SurfaceTypeFDLutCounterBuffer20,
    SurfaceTypeFDLutCounterBuffer21,
    SurfaceTypeFDLutCounterBuffer22,
    SurfaceTypeFDLutCounterBuffer23,
    SurfaceTypeFDLutCounterBuffer24,
    SurfaceTypeFDLutCounterBuffer25,
    SurfaceTypeFDLutCounterBuffer26,
    SurfaceTypeFDLutCounterBuffer27,
    SurfaceTypeFDLutCounterBuffer28,
    SurfaceTypeFDLutCounterBuffer29,
    SurfaceTypeFDLutCounterBuffer30,
    SurfaceTypeFDLutCounterBuffer31,
    SurfaceTypeFDLutCounterBuffer32,
    SurfaceTypeFDLutCounterBuffer33,
    SurfaceTypeFDLutCounterBuffer34,
    SurfaceTypeFDLutCounterBuffer35,
    SurfaceTypeFDLutCounterBuffer36,
    SurfaceTypeFDLutCounterBuffer37,
    SurfaceTypeFDLutCounterBuffer38,
    SurfaceTypeFDLutCounterBuffer39,
    SurfaceTypeFDLutCounterBuffer40,
    SurfaceTypeFDLutCounterBuffer41,
    SurfaceTypeFDLutCounterBuffer42,
    SurfaceTypeFDLutCounterBuffer43,
    SurfaceTypeFDLutCounterBuffer44,
    SurfaceTypeFDLutCounterBuffer45,
    SurfaceTypeFDLutCounterBuffer46,
    SurfaceTypeFDLutCounterBuffer47,
    SurfaceTypeFDLutCounterBuffer48,
    SurfaceTypeFDLutCounterBuffer49,
    SurfaceTypeFDLutCounterBuffer50,
    SurfaceTypeFDLutCounterBuffer51,
    SurfaceTypeFDLutCounterBuffer52,
    SurfaceTypeFDLutCounterBuffer53,
    SurfaceTypeFDLutCounterBuffer54,
    SurfaceTypeFDLutCounterBuffer55,
    SurfaceTypeFDLutCounterBuffer56,
    SurfaceTypeFDLutCounterBuffer57,
    SurfaceTypeFDLutCounterBuffer58,
    SurfaceTypeFDLutCounterBuffer59,
    SurfaceTypeFDLutCounterBuffer60,
    SurfaceTypeFDLutCounterBuffer61,
    SurfaceTypeFDLutCounterBuffer62,
    SurfaceTypeFDLutCounterBuffer63,
    SurfaceTypeFDLutCounterBuffer64,
    SurfaceTypeFDFaceListBuffer,
    SurfaceTypeFDFaceListBuffer2,
    SurfaceTypeFDFaceListBuffer3,
    SurfaceTypeFDFaceListBuffer4,
    SurfaceTypeFDFaceListBuffer5,
    SurfaceTypeFDFaceCounterBuffer,
    SurfaceTypeFDFaceCounterBuffer2,
    SurfaceTypeFDFaceCounterBuffer3,
    SurfaceTypeFDFaceCounterBuffer4,
    SurfaceTypeFDFaceCounterBuffer5,
    SurfaceTypeFDLutIntHandle,
    SurfaceTypeFDLutIntHandle2,
    SurfaceTypeFDLutIntHandle3,
    SurfaceTypeFDLutIntHandle4,
    SurfaceTypeFDLutIntHandle5,
    SurfaceTypeFDCoordnateHandle,
    SurfaceTypeFDCoordnateHandle2,
    SurfaceTypeFDCoordnateHandle3,
    SurfaceTypeFDCoordnateHandle4,
    SurfaceTypeFDCoordnateHandle5,
    SurfaceTypeFDRoundsHandle,
    SurfaceTypeFDRoundsHandle2,
    SurfaceTypeFDRoundsHandle3,
    SurfaceTypeFDRoundsHandle4,
    SurfaceTypeFDRoundsHandle5,
    SurfaceTypeFDThresholdHandle,
    SurfaceTypeFDThresholdHandle2,
    SurfaceTypeFDThresholdHandle3,
    SurfaceTypeFDThresholdHandle4,
    SurfaceTypeFDThresholdHandle5,
    SurfaceTypeFDFaceGroupCountBuffer,
    SurfaceTypeFDFaceGroupCountBuffer2,
    SurfaceTypeFDFaceGroupCountBuffer3,
    SurfaceTypeFDFaceGroupCountBuffer4,
    SurfaceTypeFDFaceGroupCountBuffer5,
    SurfaceTypeFDFaceGroupBuffer,
    SurfaceTypeFDFaceGroupBuffer2,
    SurfaceTypeFDFaceGroupBuffer3,
    SurfaceTypeFDFaceGroupBuffer4,
    SurfaceTypeFDFaceGroupBuffer5,
    SurfaceTypeFDOutFaceBuffer,
    SurfaceTypeFDOutFaceCountBuffer,
    SurfaceTypeFDOutFaceViewBuffer,
    SurfaceTypeFDFrameCount,
    SurfaceTypeFDFaceFlag,
    SurfaceTypeFDValidatorFlag,
    SurfaceTypeFDConfidenceOut,
    SurfaceTypeFDPostProcessingOutCur,
    SurfaceTypeFDPreFaceCountBuffer,
    SurfaceTypeFDScaleHandle,
    SurfaceTypeFDScaleHandle2,
    SurfaceTypeFDScaleHandle3,
    SurfaceTypeFDScaleHandle4,
    SurfaceTypeFDScaleHandle5,
    SurfaceTypeFDScaleHandle6,
    SurfaceTypeFDScaleHandle7,
    SurfaceTypeFDScaleHandle8,
    SurfaceTypeFDScaleHandle9,
    SurfaceTypeFDScaleHandle10,
    SurfaceTypeFDScaleHandle11,
    SurfaceTypeFDScaleHandle12,
    SurfaceTypeFDScaleHandle13,
    SurfaceTypeFDScaleHandle14,
    SurfaceTypeFDScaleHandle15,
    SurfaceTypeFDScaleHandle16,
    SurfaceTypeFDScaleHandle17,
    SurfaceTypeFDScaleHandle18,
    SurfaceTypeFDScaleHandle19,
    SurfaceTypeFDScaleHandle20,
    SurfaceTypeFDScaleHandle21,
    SurfaceTypeFDScaleHandle22,
    SurfaceTypeFDScaleHandle23,
    SurfaceTypeFDScaleHandle24,
    SurfaceTypeFDScaleHandle25,
    SurfaceTypeFDScaleHandle26,
    SurfaceTypeFDScaleHandle27,
    SurfaceTypeFDScaleHandle28,
    SurfaceTypeFDScaleHandle29,
    SurfaceTypeFDScaleHandle30,
    SurfaceTypeFDScaleHandle31,
    SurfaceTypeFDScaleHandle32,
    SurfaceTypeFDScaleHandle33,
    SurfaceTypeFDScaleHandle34,
    SurfaceTypeFDScaleHandle35,
    SurfaceTypeFDScaleHandle36,
    SurfaceTypeFDScaleHandle37,
    SurfaceTypeFDScaleHandle38,
    SurfaceTypeFDScaleHandle39,
    SurfaceTypeFDScaleHandle40,
    SurfaceTypeFDScaleHandle41,
    SurfaceTypeFDScaleHandle42,
    SurfaceTypeFDScaleHandle43,
    SurfaceTypeFDScaleHandle44,
    SurfaceTypeFDScaleHandle45,
    SurfaceTypeFDScaleHandle46,
    SurfaceTypeFDScaleHandle47,
    SurfaceTypeFDScaleHandle48,
    SurfaceTypeFDScaleHandle49,
    SurfaceTypeFDScaleHandle50,
    SurfaceTypeFDScaleHandle51,
    SurfaceTypeFDScaleHandle52,
    SurfaceTypeFDScaleHandle53,
    SurfaceTypeFDScaleHandle54,
    SurfaceTypeFDScaleHandle55,
    SurfaceTypeFDScaleHandle56,
    SurfaceTypeFDScaleHandle57,
    SurfaceTypeFDScaleHandle58,
    SurfaceTypeFDScaleHandle59,
    SurfaceTypeFDScaleHandle60,
    SurfaceTypeFDScaleHandle61,
    SurfaceTypeFDScaleHandle62,
    SurfaceTypeFDScaleHandle63,
    SurfaceTypeFDScaleHandle64,
    SurfaceTypeFDMLbpHandle,
    SurfaceTypeFDMLbpHandle2,
    SurfaceTypeFDMLbpHandle3,
    SurfaceTypeFDMLbpHandle4,
    SurfaceTypeFDMLbpHandle5,
    SurfaceTypeFDMLbpHandle6,
    SurfaceTypeFDMLbpHandle7,
    SurfaceTypeFDMLbpHandle8,
    SurfaceTypeFDMLbpHandle9,
    SurfaceTypeFDMLbpHandle10,
    SurfaceTypeFDMLbpHandle11,
    SurfaceTypeFDMLbpHandle12,
    SurfaceTypeFDMLbpHandle13,
    SurfaceTypeFDMLbpHandle14,
    SurfaceTypeFDMLbpHandle15,
    SurfaceTypeFDMLbpHandle16,
    SurfaceTypeFDMLbpHandle17,
    SurfaceTypeFDMLbpHandle18,
    SurfaceTypeFDMLbpHandle19,
    SurfaceTypeFDMLbpHandle20,
    SurfaceTypeFDMLbpHandle21,
    SurfaceTypeFDMLbpHandle22,
    SurfaceTypeFDMLbpHandle23,
    SurfaceTypeFDMLbpHandle24,
    SurfaceTypeFDMLbpHandle25,
    SurfaceTypeFDMLbpHandle26,
    SurfaceTypeFDMLbpHandle27,
    SurfaceTypeFDMLbpHandle28,
    SurfaceTypeFDMLbpHandle29,
    SurfaceTypeFDMLbpHandle30,
    SurfaceTypeFDMLbpHandle31,
    SurfaceTypeFDMLbpHandle32,
    SurfaceTypeFDMLbpHandle33,
    SurfaceTypeFDMLbpHandle34,
    SurfaceTypeFDMLbpHandle35,
    SurfaceTypeFDMLbpHandle36,
    SurfaceTypeFDMLbpHandle37,
    SurfaceTypeFDMLbpHandle38,
    SurfaceTypeFDMLbpHandle39,
    SurfaceTypeFDMLbpHandle40,
    SurfaceTypeFDMLbpHandle41,
    SurfaceTypeFDMLbpHandle42,
    SurfaceTypeFDMLbpHandle43,
    SurfaceTypeFDMLbpHandle44,
    SurfaceTypeFDMLbpHandle45,
    SurfaceTypeFDMLbpHandle46,
    SurfaceTypeFDMLbpHandle47,
    SurfaceTypeFDMLbpHandle48,
    SurfaceTypeFDMLbpHandle49,
    SurfaceTypeFDMLbpHandle50,
    SurfaceTypeFDMLbpHandle51,
    SurfaceTypeFDMLbpHandle52,
    SurfaceTypeFDMLbpHandle53,
    SurfaceTypeFDMLbpHandle54,
    SurfaceTypeFDMLbpHandle55,
    SurfaceTypeFDMLbpHandle56,
    SurfaceTypeFDMLbpHandle57,
    SurfaceTypeFDMLbpHandle58,
    SurfaceTypeFDMLbpHandle59,
    SurfaceTypeFDMLbpHandle60,
    SurfaceTypeFDMLbpHandle61,
    SurfaceTypeFDMLbpHandle62,
    SurfaceTypeFDMLbpHandle63,
    SurfaceTypeFDMLbpHandle64,
    // FLD
//    SurfaceTypeFLDFrameCount,
    SurfaceTypeFLDValidatorFlag,
    SurfaceTypeFLDGlobalFlag,
    SurfaceTypeFLDSurfaceSrc,
    SurfaceTypeFLDResizeImageOutIn,
    SurfaceTypeFLDGaussianWeightOutIn,
    SurfaceTypeFLDFeaturesOutIn,
    SurfaceTypeFLDLandmarkInOut,
    SurfaceTypeFLDGlobalSumOutIn,
    SurfaceTypeFLDCoefficientIn,
    SurfaceTypeFLDMeanShapeInOut,
    SurfaceTypeFLDEyeMeanShape,
    SurfaceTypeFLDEyeLandmarks,
    SurfaceTypeFLDInvAffineMatrix,
    SurfaceTypeFLDEyeCoefficientIn,
    SurfaceTypeFLDRemappedLM,
    SurfaceTypeFLDEFFilteredLMK,
    SurfaceTypeFLDPostProcessingOutInPre,
    SurfaceTypeFLDPreProcessingIn,
    SurfaceTypeFLDScoreboarGetSum,
    SurfaceTypeFLDScoreboarExtractFeature,
    SurfaceTypeFLDThreadMapGetSum,
    SurfaceTypeFLDThreadMapExtractFeature,
    SurfaceTypeFLDScaleOutIn,
    SurfaceTypeFLDFlagOutIn,
    SurfaceTypeFLDStartOutIn,
    SurfaceTypeFLDNormalizeOut,
    SurfaceTypeFLDNormalizeCoeff,
    SurfaceTypeFLDMlbpOut,
    SurfaceTypeFLDGetsumCoeffX,
    SurfaceTypeFLDGetsumCoeffY,
    SurfaceTypeFLDGetsumCoeffH,
    SurfaceTypeFLDFaceModel,
    // FB
    SurfaceTypeFBSurfaceSrc,
    SurfaceTypeFBSurface1,
    SurfaceTypeFBSurface2,
    SurfaceTypeFBSurface3,
    SurfaceTypeFBSurfaceTempY,
    SurfaceTypeFBSurfaceTempUV,
    SurfaceTypeFBSurfaceTempTposedY,
    SurfaceTypeFBSurfaceTempTposedUV,
    SurfaceTypeFBMask1,
    SurfaceTypeFBMask2,
    SurfaceTypeFBFaceMask,
    SurfaceTypeFBFaceMask2x,
    SurfaceTypeFBFaceMask4x,
    SurfaceTypeFBLipMaskInput,
    SurfaceTypeFBLipMaskOutput,
    SurfaceTypeFBLipPosition,
    SurfaceTypeFBLipPosition2,
    SurfaceTypeFBLipSum,
    SurfaceTypeFBLipMax,
    SurfaceTypeFBLipMin,
    SurfaceTypeFBSamplerOut,
    SurfaceTypeFBSamplerOut1,
    SurfaceTypeFBSTDFrameCount,
    SurfaceTypeFBSTDSamples,
    SurfaceTypeFBSTDHistogram,
    SurfaceTypeFBSTDHistory,
    SurfaceTypeFBEyeModelMeta,
    SurfaceTypeFBEyeModelDatas,
    SurfaceTypeFBEyeModelDatas2,
    SurfaceTypeFBEyeModelDatas3,
    SurfaceTypeFBEyeModelDatas4,
    SurfaceTypeFBEyeModelDatas5,
    SurfaceTypeFBEyeModelDatas6,
    SurfaceTypeFBEyeModelDatas7,
    SurfaceTypeFBEyeModelDatas8,
    SurfaceTypeFBEyeModelDatas9,
    SurfaceTypeFBEyeModelDatas10,
    SurfaceTypeFBEyeAffineMatrix,
    SurfaceTypeFBEyeFeatureFlag,
    SurfaceTypeFBEyeParams,
    SurfaceTypeFBEyeRMap,
    SurfaceTypeFBEyeCoMap,
    SurfaceTypeFBBrightenMask,
    SurfaceTypeFBEyeCurve,
    SurfaceTypeFBShadowMask1,
    SurfaceTypeFBShadowMask2,
    SurfaceTypeFBPupilMask,
    SurfaceTypeFBPupilParamSurf,
    SurfaceTypeFBPupilHistSurf,
    SurfaceTypeFBPupilVerSurf,
    SurfaceTypeFBPupilHorSurf,
    SurfaceTypeFBTeethColorMap,
    SurfaceTypeFBSurfacesForVeBox,
    SurfaceTypeFBSurfacesForVeBox1,
    SurfaceTypeFBSurfacesForVeBox2,
    SurfaceTypeFBVeboxParamBuf,
    SurfaceTypeFBFacialRegions,
    SurfaceTypeFBPostProcessingOutCur,
    SurfaceTypeFBRemappedLM,
    SurfaceTypeFBSamplerIndex,
    SurfaceTypeFBInputSampler,
    // HVS Kernel
    SurfaceTypeHVSTable,
    //Segmentation
    SurfaceTypeSegRenderPreviousInput,
    SurfaceTypeSegRenderTempOutput, //Used for seg out only
    SurfaceTypeSegBackground,
    SurfaceTypeSegGaussianCoeffBuffer,
    SurfaceTypeSegTFMask,
    SurfaceTypeSegTFOutMask,
    SurfaceTypeSegWindowSize,
    SurfaceTypeSegMaskThreshold,
    SurfaceTypeSegMaskScaling,
    SurfaceTypeSegMaskBlur,
    SurfaceTypeSegBlurScaling,
    SurfaceTypeSegGaussianBlur,
    SurfaceTypeSegBlur,
    SurfaceTypeSegBlur2,
    SurfaceTypeSegBlur3,
    SurfaceTypeSegGFLayer,
    SurfaceTypeSegGFLayerEnd = SurfaceTypeSegGFLayer + GFSURFACE_COUNT,
    SurfaceTypeSegGFOut,
    SurfaceTypeSegGFOut2,
    SurfaceTypeSegGFOut3,
    SurfaceTypeSegGFOut4,
    SurfaceTypeSegInputMotion,
    SurfaceTypeSegInputMotionEnd = SurfaceTypeSegInputMotion + MOTION_CHANNEL - 1,
    SurfaceTypeSegPreInputMotion,
    SurfaceTypeSegPreInputMotionEnd = SurfaceTypeSegPreInputMotion + MOTION_CHANNEL - 1,
    SurfaceTypeSegDiffMotion,
    SurfaceTypeSegDiffMotionEnd = SurfaceTypeSegDiffMotion + MOTION_CHANNEL - 1,
    SurfaceTypeSegErode1x2Motion,
    SurfaceTypeSegErode1x2MotionEnd = SurfaceTypeSegErode1x2Motion + MOTION_CHANNEL - 1,
    SurfaceTypeSegErode2x1Motion,
    SurfaceTypeSegErode2x1Motion2,
    SurfaceTypeSegErode2x1Motion3,
    SurfaceTypeSegSumMotion,
    SurfaceTypeSegRemoveBlob,
    // Segmentation layers
    SurfaceTypeSegModelLayer,
    SurfaceTypeSegModelLayerEnd = SurfaceTypeSegModelLayer + MAX_MODELSURFACE_COUNT,
    SurfaceTypeSegWeights,
    SurfaceTypeSegWeightsEnd = SurfaceTypeSegWeights + MAX_MODELSURFACE_COUNT,
    SurfaceTypeSegBias,
    SurfaceTypeSegBiasEnd = SurfaceTypeSegBias + MAX_MODELSURFACE_COUNT,

    // SR
    SurfaceTypeSRLumaInputSurf,
    SurfaceTypeSRTempOutputSurf,
    SurfaceTypeSRTempLumaOutput,
    SurfaceTypeSRTempLumaOutputEnd = SurfaceTypeSRTempLumaOutput + SURFACETYPE_SIZE32,
    SurfaceTypeSRTempLuma2Output,
    SurfaceTypeSRTempLuma2OutputEnd = SurfaceTypeSRTempLuma2Output + SURFACETYPE_SIZE16,
    SurfaceTypeSRTempLuma3Output,
    SurfaceTypeSRTempLuma3OutputEnd = SurfaceTypeSRTempLuma3Output + SURFACETYPE_SIZE5,
    SurfaceTypeSRTempLuma4Output,
    SurfaceTypeSRTempLuma4OutputEnd = SurfaceTypeSRTempLuma4Output + SURFACETYPE_SIZE32,
    SurfaceTypeSRTempLuma5Output,
    SurfaceTypeSRWeightBuffer,
    SurfaceTypeSRWeightBufferEnd = SurfaceTypeSRWeightBuffer + SURFACETYPE_SIZE10,
    SurfaceTypeSRBiasBuffer,
    SurfaceTypeSRBiasBufferEnd = SurfaceTypeSRBiasBuffer + SURFACETYPE_SIZE10,
    SurfaceTypeSRPreluBuffer,
    SurfaceTypeSRPreluBufferEnd = SurfaceTypeSRPreluBuffer + SURFACETYPE_SIZE10,
    SurfaceTypeSRSlopeBuffer,
    SurfaceTypeSRSlopeBufferEnd = SurfaceTypeSRSlopeBuffer + SURFACETYPE_SIZE5,
    SurfaceTypeSROutput,
    SurfaceTypeSRChromaInput,

    // HDR Kernel
    SurfaceTypeHdrInputLayer0,
    SurfaceTypeHdrInputLayer1,
    SurfaceTypeHdrInputLayer2,
    SurfaceTypeHdrInputLayer3,
    SurfaceTypeHdrInputLayer4,
    SurfaceTypeHdrInputLayer5,
    SurfaceTypeHdrInputLayer6,
    SurfaceTypeHdrInputLayer7,
    SurfaceTypeHdrInputLayerMax = SurfaceTypeHdrInputLayer7,
    SurfaceTypeHdrOETF1DLUTSurface0,
    SurfaceTypeHdrOETF1DLUTSurface1,
    SurfaceTypeHdrOETF1DLUTSurface2,
    SurfaceTypeHdrOETF1DLUTSurface3,
    SurfaceTypeHdrOETF1DLUTSurface4,
    SurfaceTypeHdrOETF1DLUTSurface5,
    SurfaceTypeHdrOETF1DLUTSurface6,
    SurfaceTypeHdrOETF1DLUTSurface7,
    SurfaceTypeHdrCRI3DLUTSurface0,
    SurfaceTypeHdrCRI3DLUTSurface1,
    SurfaceTypeHdrCRI3DLUTSurface2,
    SurfaceTypeHdrCRI3DLUTSurface3,
    SurfaceTypeHdrCRI3DLUTSurface4,
    SurfaceTypeHdrCRI3DLUTSurface5,
    SurfaceTypeHdrCRI3DLUTSurface6,
    SurfaceTypeHdrCRI3DLUTSurface7,
    SurfaceTypeHdrTarget0,
    SurfaceTypeHdrCoeff,
    SurfaceTypeHdrAutoModeCoeff,
    SurfaceTypeHdrAutoModeIirTempSurface,
    NumberOfSurfaceType
};

using  VP_SURFACE_GROUP = std::map<SurfaceType, VP_SURFACE*>;

struct REMOVE_BB_SETTING
{
    bool     isRemoveBB    = false;
    bool     isKeepMaxBlob = false;
    uint32_t index                          = 0;
    uint32_t height                         = 0;
    uint32_t width                          = 0;
    uint32_t size                           = 0;
    uint16_t inputActiveRegionWidth         = 0;
    uint16_t inputActiveRegionHeight        = 0;
    uint8_t *removeBlobLinearAddressAligned = 0;
};

struct MOTIONLESS_SETTING
{
    bool     isEnable        = false;
    bool     isSkipDetection = false;
    bool     isInfer         = false;
    bool     isFirstConv     = false;
    bool     isMotion        = false;
    bool     isResUpdate     = false;
    uint32_t width           = 0;
    uint32_t height          = 0;
};

struct VP_POSTPROCESS_SURFACE
{
    REMOVE_BB_SETTING  removeBBSetting;
    MOTIONLESS_SETTING motionlessSetting;
};

struct VP_SURFACE_SETTING
{
    VP_SURFACE_GROUP    surfGroup;
    bool                isPastHistogramValid       = false;
    uint32_t            imageWidthOfPastHistogram  = 0;
    uint32_t            imageHeightOfPastHistogram = 0;
    uint32_t            dwVeboxPerBlockStatisticsHeight = 0;
    uint32_t            dwVeboxPerBlockStatisticsWidth  = 0;
    uint32_t            aggregateBlockSize              = 0;
    bool                laceLutValid                    = false;
    bool                updateGlobalToneMappingCurveLUTSurface = false;
    bool                updateWeitCoefSurface                  = false;
    bool                dumpLaceSurface                        = false;
    bool                dumpPreSurface                         = false;
    bool                dumpPostSurface                        = false;
    VP_POSTPROCESS_SURFACE postProcessSurface                  = {};
    const uint16_t     *pHDRStageConfigTable                   = nullptr;
    bool                coeffAllocated                         = false;
    bool                OETF1DLUTAllocated                     = false;

    void Clean()
    {
        surfGroup.clear();
        isPastHistogramValid        = false;
        imageWidthOfPastHistogram   = 0;
        imageHeightOfPastHistogram  = 0;
        dwVeboxPerBlockStatisticsHeight = 0;
        dwVeboxPerBlockStatisticsWidth  = 0;
        aggregateBlockSize              = 0;
        laceLutValid                    = false;
        updateGlobalToneMappingCurveLUTSurface = true;
        updateWeitCoefSurface                  = true;
        dumpLaceSurface                        = false;
        dumpPreSurface                         = false;
        dumpPostSurface                        = false;
        postProcessSurface.removeBBSetting     = {};
        postProcessSurface.motionlessSetting   = {};
        pHDRStageConfigTable                   = nullptr;
        coeffAllocated                         = false;
        OETF1DLUTAllocated                     = false;
    }
};

#define FEATURE_TYPE_MASK   0xffffff00
#define FEATURE_TYPE_ENGINE_ASSIGNED(feature) (((feature)&FEATURE_TYPE_MASK) != (feature))

inline bool operator==(FeatureType a, FeatureType b)
{
    return (int)a == (int)b || (int)(a & FEATURE_TYPE_MASK) == (int)b || (int)a == (int)(FEATURE_TYPE_MASK & b);
}

inline bool operator!=(FeatureType a, FeatureType b)
{
    return !(a == b);
}

inline bool operator<(FeatureType a, FeatureType b)
{
    return a != b && (int)a < (int)b;
}

#define RECT_ROTATE(rcOut, rcIn)        \
{                                       \
    (rcOut).left    = (rcIn).top;       \
    (rcOut).right   = (rcIn).bottom;    \
    (rcOut).top     = (rcIn).left;      \
    (rcOut).bottom  = (rcIn).right;     \
}

struct FeatureParam
{
    FeatureType type         = FeatureTypeInvalid;
    MOS_FORMAT  formatInput  = Format_None;
    MOS_FORMAT  formatOutput = Format_None;
};

enum FeatureCategory
{
    FeatureCategoryBasic    = 0,
    FeatureCategoryIsolated,
    FeatureCategoryFC,
};

class SwFilterSet;

class SwFilter
{
public:
    SwFilter(VpInterface &vpInterface, FeatureType type);
    virtual ~SwFilter();
    virtual MOS_STATUS Clean()
    {
        MOS_ZeroMemory(&m_EngineCaps, sizeof(m_EngineCaps));
        m_noNeedUpdate = false;
        m_isInExePipe = false;
        return MOS_STATUS_SUCCESS;
    }
    virtual FeatureType GetFeatureType()
    {
        return m_type;
    }
    virtual bool IsEngineAssigned()
    {
        return m_type != (m_type & FEATURE_TYPE_MASK);
    }
    virtual RenderTargetType GetRenderTargetType()
    {
        return m_renderTargetType;
    }
    bool IsFeatureEnabled(VP_EXECUTE_CAPS caps)
    {
        return m_EngineCaps.bEnabled && (m_EngineCaps.SfcNeeded && caps.bSFC ||
            m_EngineCaps.VeboxNeeded && caps.bVebox || m_EngineCaps.RenderNeeded && caps.bRender);
    }
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool bInputSurf, int surfIndex) = 0;
    virtual MOS_STATUS Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual MOS_STATUS Configure(SwFilter& swFilter, VP_EXECUTE_CAPS caps)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual SwFilter *Clone() = 0;
    virtual bool operator == (class SwFilter&) = 0;
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe) = 0;
    virtual MOS_STATUS SetFeatureType(FeatureType type);
    virtual MOS_STATUS ResetFeatureType();
    virtual MOS_STATUS SetRenderTargetType(RenderTargetType type);
    SwFilter* CreateSwFilter(FeatureType type);
    void DestroySwFilter(SwFilter* p);

    void SetLocation(SwFilterSet *swFilterSet)
    {
        m_location = swFilterSet;
    }

    SwFilterSet *GetLocation()
    {
        return m_location;
    }

    VpInterface& GetVpInterface()
    {
        return m_vpInterface;
    }

    VP_EngineEntry& GetFilterEngineCaps()
    {
        return m_EngineCaps;
    }

    // For some feature in output pipe, enable or disable depends on other feature status of input pipe.
    // If singleInputPipeSelected != nullptr, means single input pipe inuse, otherwise, multi-input pipe in use.
    virtual VP_EngineEntry GetCombinedFilterEngineCaps(SwFilterSubPipe *singleInputPipeSelected)
    {
        return m_EngineCaps;
    }

    // The child class need to implement SetResourceAssignmentHint only when any feature
    // parameters will affect the resource assignment.
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        return MOS_STATUS_SUCCESS;
    }

    void SetExePipeFlag(bool isInExePipe)
    {
        m_isInExePipe = isInExePipe;
    }

protected:
    VpInterface &m_vpInterface;
    FeatureType m_type = FeatureTypeInvalid;
    // SwFilterSet current swFilter belongs to.
    SwFilterSet *m_location = nullptr;
    VP_EngineEntry  m_EngineCaps = {};
    bool m_noNeedUpdate = false;
    RenderTargetType m_renderTargetType = RenderTargetTypeSurface;
    bool m_isInExePipe = false;

MEDIA_CLASS_DEFINE_END(vp__SwFilter)
};

struct FeatureParamCsc : public FeatureParam
{
    struct CSC_PARAMS
    {
        VPHAL_CSPACE    colorSpace      = CSpace_None;
        uint32_t        chromaSiting    = 0;
        bool operator == (const struct CSC_PARAMS &b)
        {
            return colorSpace == b.colorSpace && chromaSiting == b.chromaSiting;
        }
    };
    CSC_PARAMS          input           = {};
    CSC_PARAMS          output          = {};
    PVPHAL_IEF_PARAMS   pIEFParams      = nullptr;
    PVPHAL_ALPHA_PARAMS pAlphaParams    = nullptr;
    FeatureParamCsc     *next           = nullptr;                //!< pointe to new/next generated CSC params
};

class SwFilterCsc : public SwFilter
{
public:
    SwFilterCsc(VpInterface &vpInterface);
    virtual ~SwFilterCsc();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual MOS_STATUS Configure(FeatureParamCsc &params);
    virtual FeatureParamCsc &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetFeatureType(FeatureType type);

private:
    FeatureParamCsc m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterCsc)
};

struct FeatureParamScaling : public FeatureParam
{
    struct SCALING_PARAMS
    {
        uint32_t                dwWidth  = 0;
        uint32_t                dwHeight = 0;
        RECT                    rcSrc    = {0, 0, 0, 0};
        RECT                    rcDst    = {0, 0, 0, 0};  //!< Input dst rect without rotate being applied.
        RECT                    rcMaxSrc = {0, 0, 0, 0};
        VPHAL_SAMPLE_TYPE       sampleType = SAMPLE_PROGRESSIVE;
        MOS_TILE_MODE_GMM       tileMode   = MOS_TILE_4_GMM;
        bool operator == (struct SCALING_PARAMS &b)
        {
            return 0 == memcmp(this, &b, sizeof(SCALING_PARAMS));
        }
    };

    // Parameters maintained by scaling feature parameters
    SCALING_PARAMS              input       = {};
    SCALING_PARAMS              output      = {};
    bool                        isPrimary   = false;
    VPHAL_SCALING_MODE          scalingMode = VPHAL_SCALING_NEAREST;
    VPHAL_SCALING_PREFERENCE    scalingPreference  = VPHAL_SCALING_PREFER_SFC;  //!< DDI indicate Scaling preference
    bool                        bDirectionalScalar = false;     //!< Vebox Directional Scalar
    bool                        bTargetRectangle   = false;     // Target rectangle enabled
    PVPHAL_COLORFILL_PARAMS     pColorFillParams = nullptr;     //!< ColorFill - BG only
    PVPHAL_ALPHA_PARAMS         pCompAlpha       = nullptr;     //!< Alpha for composited surfaces
    VPHAL_ISCALING_TYPE         interlacedScalingType = ISCALING_NONE;

    // Parameters maintained by other feature parameters.
    struct {
        VPHAL_CSPACE colorSpaceOutput = CSpace_None;
    } csc;

    struct {
        bool                    rotationNeeded = false;                 //!< Whether rotate SwFilter exists on SwFilterPipe.
    } rotation;

    bool operator == (struct FeatureParamScaling &b)
    {
        return formatInput          == b.formatInput            &&
            formatOutput            == b.formatOutput           &&
            input                   == b.input                  &&
            output                  == b.output                 &&
            isPrimary               == b.isPrimary              &&
            scalingMode             == b.scalingMode            &&
            scalingPreference       == b.scalingPreference      &&
            bDirectionalScalar      == b.bDirectionalScalar     &&
            bTargetRectangle        == b.bTargetRectangle       &&
            interlacedScalingType   == b.interlacedScalingType  &&
            csc.colorSpaceOutput    == b.csc.colorSpaceOutput   &&
            rotation.rotationNeeded == b.rotation.rotationNeeded &&
            (nullptr == pColorFillParams    && nullptr == b.pColorFillParams ||
            nullptr != pColorFillParams     && nullptr != b.pColorFillParams &&
            0 == memcmp(pColorFillParams, b.pColorFillParams, sizeof(VPHAL_COLORFILL_PARAMS))) &&
            (nullptr == pCompAlpha          && nullptr == b.pCompAlpha ||
            nullptr != pCompAlpha           && nullptr != b.pCompAlpha &&
            0 == memcmp(pCompAlpha, b.pCompAlpha, sizeof(VPHAL_ALPHA_PARAMS)));
    }

    FeatureParamScaling        *next = nullptr;                           //!< pointe to new/next generated scaling params
};

class SwFilterScaling : public SwFilter
{
public:
    SwFilterScaling(VpInterface &vpInterface);
    virtual ~SwFilterScaling();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual MOS_STATUS Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps);
    virtual FeatureParamScaling &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.isIScalingTypeNone = ISCALING_NONE == m_Params.interlacedScalingType;
        hint.isFieldWeaving     = ISCALING_FIELD_TO_INTERLEAVED == m_Params.interlacedScalingType;
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamScaling m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterScaling)
};

struct FeatureParamRotMir : public FeatureParam
{
    // Parameters maintained by rotation feature parameters
    VPHAL_ROTATION rotation = VPHAL_ROTATION_IDENTITY;

    // Parameters maintained by other feature parameters.
    struct {
        MOS_TILE_TYPE tileOutput = MOS_TILE_X;
    } surfInfo;

    bool operator == (const struct FeatureParamRotMir &b)
    {
        return rotation == b.rotation &&
            surfInfo.tileOutput == b.surfInfo.tileOutput;
    }
};

class SwFilterRotMir : public SwFilter
{
public:
    SwFilterRotMir(VpInterface &vpInterface);
    virtual ~SwFilterRotMir();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual FeatureParamRotMir &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamRotMir m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterRotMir)
};

enum DN_STAGE
{
    DN_STAGE_DEFAULT = 0,
    DN_STAGE_HVS_KERNEL,
    DN_STAGE_VEBOX_HVS_UPDATE,
    DN_STAGE_VEBOX_HVS_NO_UPDATE,
};

struct FeatureParamDenoise : public FeatureParam
{
    VPHAL_SAMPLE_TYPE    sampleTypeInput      = SAMPLE_PROGRESSIVE;
    VPHAL_DENOISE_PARAMS denoiseParams        = {};
    uint32_t             widthAlignUnitInput  = 0;
    uint32_t             heightAlignUnitInput = 0;
    uint32_t             heightInput          = 0;
    bool                 secureDnNeeded       = false;
    DN_STAGE             stage                = DN_STAGE_DEFAULT;
    bool                 operator==(const struct FeatureParamDenoise &b)
    {
        return sampleTypeInput     == b.sampleTypeInput &&
               denoiseParams       == b.denoiseParams   &&
               widthAlignUnitInput == b.widthAlignUnitInput &&
               heightAlignUnitInput == b.heightAlignUnitInput &&
               heightInput         == b.heightInput &&
               secureDnNeeded      == b.secureDnNeeded &&
               stage               == b.stage;
    }
};

class SwFilterDenoise : public SwFilter
{
public:
    SwFilterDenoise(VpInterface& vpInterface);
    virtual ~SwFilterDenoise();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamDenoise& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS           SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.isHVSTableNeeded = DN_STAGE_HVS_KERNEL == m_Params.stage ||
                                DN_STAGE_VEBOX_HVS_UPDATE == m_Params.stage;
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamDenoise m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterDenoise)
};

struct FeatureParamDeinterlace : public FeatureParam
{
    VPHAL_SAMPLE_TYPE       sampleTypeInput      = SAMPLE_PROGRESSIVE;
    bool                    bHDContent           = false;
    PVPHAL_DI_PARAMS        diParams             = nullptr;
    bool                    bFmdExtraVariance    = false;     //!< Check if extra FMD variances need to be calculated
    bool                    bFmdKernelEnable     = false;     //!< FMD kernel path enabled
    bool                    bQueryVarianceEnable = false;     //!< Query variance enabled
    uint32_t                heightInput          = 0;
    RECT                    rcSrc                = {0, 0, 0, 0};
};

class SwFilterDeinterlace : public SwFilter
{
public:
    SwFilterDeinterlace(VpInterface& vpInterface);
    virtual ~SwFilterDeinterlace();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamDeinterlace& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.bDi        = 1;
        hint.b60fpsDi   = m_Params.diParams && !m_Params.diParams->bSingleField;
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamDeinterlace m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterDeinterlace)
};

struct FeatureParamSte : public FeatureParam
{
    bool       bEnableSTE  = false;
    uint32_t   dwSTEFactor = 0;
};

class SwFilterSte : public SwFilter
{
public:
    SwFilterSte(VpInterface& vpInterface);
    virtual ~SwFilterSte();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamSte& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamSte m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterSte)
};

struct FeatureParamTcc : public FeatureParam
{
    bool                bEnableTCC = false;
    uint8_t             Red        = 0;
    uint8_t             Green      = 0;
    uint8_t             Blue       = 0;
    uint8_t             Cyan       = 0;
    uint8_t             Magenta    = 0;
    uint8_t             Yellow     = 0;
};

class SwFilterTcc : public SwFilter
{
public:
    SwFilterTcc(VpInterface& vpInterface);
    virtual ~SwFilterTcc();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamTcc& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamTcc m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterTcc)
};

struct FeatureParamProcamp : public FeatureParam
{
    PVPHAL_PROCAMP_PARAMS procampParams = nullptr;
};

class SwFilterProcamp : public SwFilter
{
public:
    SwFilterProcamp(VpInterface& vpInterface);
    virtual ~SwFilterProcamp();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamProcamp& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamProcamp m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterProcamp)
};

enum HDR_STAGE
{
    HDR_STAGE_DEFAULT = 0,
    HDR_STAGE_3DLUT_KERNEL,
    HDR_STAGE_VEBOX_3DLUT_UPDATE,
    HDR_STAGE_VEBOX_3DLUT_NO_UPDATE,
};

//!
//! \brief Hdr stages enable flag
//!
typedef union _HDRStageEnables
{
    uint16_t value;
    struct
    {
        uint16_t PriorCSCEnable : 1;
        uint16_t EOTFEnable : 1;
        uint16_t CCMEnable : 1;
        uint16_t PWLFEnable : 1;
        uint16_t CCMExt1Enable : 1;
        uint16_t GamutClamp1Enable : 1;
        uint16_t CCMExt2Enable : 1;
        uint16_t GamutClamp2Enable : 1;
        uint16_t OETFEnable : 1;
        uint16_t PostCSCEnable : 1;
        uint16_t Reserved : 6;
    };
} HDRStageEnables, *PHDRStageEnables;

//!
//! Structure VPHAL_HDR_PARAMS
//! \brief High Dynamic Range parameters
//!
typedef struct _HDR_PARAMS
{
    VPHAL_HDR_EOTF_TYPE EOTF                 = VPHAL_HDR_EOTF_INVALID;    //!< Electronic-Optimal Transfer Function
    uint16_t display_primaries_x[3]          = {0};                       //!< Display Primaries X chromaticity coordinates
    uint16_t display_primaries_y[3]          = {0};                       //!< Display Primaries Y chromaticity coordinates
    uint16_t white_point_x                   = 0;                         //!< X Chromaticity coordinate of White Point
    uint16_t white_point_y                   = 0;                         //!< Y Chromaticity coordinate of White Point
    uint16_t max_display_mastering_luminance = 0;                         //!< The nominal maximum display luminance of the mastering display
    uint16_t min_display_mastering_luminance = 0;                         //!< The nominal minimum display luminance of the mastering display
    uint16_t MaxCLL                          = 0;                         //!< Max Content Light Level
    uint16_t MaxFALL                         = 0;                         //!< Max Frame Average Light Level
    bool     bAutoMode                       = false;                     //!< Hdr auto mode.
    bool     bPathKernel                     = false;                     //!< Hdr path config to use kernel
} HDR_PARAMS, *PHDR_PARAMS;

struct FeatureParamHdr : public FeatureParam
{
    uint32_t           uiMaxDisplayLum                                      = 0;                   //!< Maximum Display Luminance
    uint32_t           uiMaxContentLevelLum                                 = 0;                   //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE     hdrMode                                              = VPHAL_HDR_MODE_NONE;
    VPHAL_CSPACE       srcColorSpace                                        = CSpace_None;
    VPHAL_CSPACE       dstColorSpace                                        = CSpace_None;
    HDR_STAGE          stage                                                = HDR_STAGE_DEFAULT;
    uint32_t           widthInput                                           = 0;
    uint32_t           heightInput                                          = 0;
    VPHAL_HDR_LUT_MODE LUTMode[VPHAL_MAX_HDR_INPUT_LAYER]                   = {};                  //!< LUT Mode
    VPHAL_HDR_LUT_MODE GlobalLutMode                                        = {};                  //!< Global LUT mode control for debugging purpose
    bool               bGpuGenerate3DLUT                                    = false;               //!< Flag for per frame GPU generation of 3DLUT
    uint32_t           uSourceCount                                         = 0;                   //!< Number of sources
    uint32_t           uTargetCount                                         = 0;                   //!< Number of targets

    PVPHAL_COLORFILL_PARAMS pColorFillParams                     = nullptr;               //!< ColorFill - BG only
    bool                    bDisableAutoMode                     = false;                 //!< Force to disable Hdr auto mode tone mapping for debugging purpose
    uint32_t                uiSplitFramePortions                 = 1;                     //!< Split Frame flag
    bool                    bForceSplitFrame                     = false;
    bool                    bNeed3DSampler                       = false;                 //!< indicate whether 3D should neede by force considering AVS removal etc.
    VPHAL_SCALING_MODE      ScalingMode                          = VPHAL_SCALING_NEAREST; //!<  Scaling Mode

    uint16_t         InputSrc[VPHAL_MAX_HDR_INPUT_LAYER]                 = {}; // Input Surface
    uint16_t         Target[VPHAL_MAX_HDR_OUTPUT_LAYER]                  = {}; // Target Surface
    HDR_PARAMS       srcHDRParams[VPHAL_MAX_HDR_INPUT_LAYER]             = {};
    HDR_PARAMS       targetHDRParams[VPHAL_MAX_HDR_OUTPUT_LAYER]         = {};
};

class SwFilterHdr : public SwFilter
{
public:
    SwFilterHdr(VpInterface &vpInterface);
    virtual ~SwFilterHdr();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamHdr &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.is3DLut2DNeeded = HDR_STAGE_3DLUT_KERNEL == m_Params.stage ||
                               HDR_STAGE_VEBOX_3DLUT_UPDATE == m_Params.stage;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HdrIsInputFormatSupported(
        PVPHAL_SURFACE pSrcSurface,
        bool          *pbSupported);

    MOS_STATUS HdrIsOutputFormatSupported(
            PVPHAL_SURFACE pTargetSurface,
            bool          *pbSupported);

private:
    FeatureParamHdr m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterHdr)
};

struct FeatureParamLumakey : public FeatureParam
{
    PVPHAL_LUMAKEY_PARAMS lumaKeyParams = nullptr;
};

class SwFilterLumakey : public SwFilter
{
public:
    SwFilterLumakey(VpInterface &vpInterface);
    virtual ~SwFilterLumakey();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamLumakey &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamLumakey m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterLumakey)
};

struct FeatureParamBlending : public FeatureParam
{
    PVPHAL_BLENDING_PARAMS  blendingParams = nullptr;
};

class SwFilterBlending : public SwFilter
{
public:
    SwFilterBlending(VpInterface &vpInterface);
    virtual ~SwFilterBlending();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamBlending &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamBlending m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterBlending)
};

struct FeatureParamColorFill : public FeatureParam
{
    PVPHAL_COLORFILL_PARAMS colorFillParams = nullptr;     //!< ColorFill - BG only
    bool operator == (const struct FeatureParamColorFill &b)
    {
        return (nullptr == colorFillParams && nullptr == b.colorFillParams ||
            nullptr != colorFillParams && nullptr != b.colorFillParams &&
            0 == memcmp(colorFillParams, b.colorFillParams, sizeof(*b.colorFillParams)));
    }
};

class SwFilterColorFill : public SwFilter
{
public:
    SwFilterColorFill(VpInterface &vpInterface);
    virtual ~SwFilterColorFill();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamColorFill &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual VP_EngineEntry   GetCombinedFilterEngineCaps(SwFilterSubPipe *singleInputPipeSelected);

private:
    FeatureParamColorFill m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterColorFill)
};

struct FeatureParamAlpha : public FeatureParam
{
    PVPHAL_ALPHA_PARAMS     compAlpha         = nullptr;      //!< Alpha for composited surface
    bool                    calculatingAlpha  = false;        //!< Alpha calculation parameters
    bool operator == (const struct FeatureParamAlpha &b)
    {
        return calculatingAlpha == b.calculatingAlpha &&
            (nullptr == compAlpha   && nullptr == b.compAlpha ||
            nullptr != compAlpha    && nullptr != b.compAlpha &&
            0 == memcmp(compAlpha, b.compAlpha, sizeof(*b.compAlpha)));
    }
};

class SwFilterAlpha : public SwFilter
{
public:
    SwFilterAlpha(VpInterface &vpInterface);
    virtual ~SwFilterAlpha();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamAlpha &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamAlpha m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterAlpha)
};

class SwFilterSet
{
public:
    SwFilterSet();
    virtual ~SwFilterSet();

    MOS_STATUS AddSwFilter(SwFilter *swFilter);
    MOS_STATUS RemoveSwFilter(SwFilter *swFilter);
    MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    MOS_STATUS Clean();
    SwFilter *GetSwFilter(FeatureType type);
    bool IsEmpty()
    {
        return m_swFilters.empty();
    }

    std::vector<class SwFilterSet *> *GetLocation();
    void SetLocation(std::vector<class SwFilterSet *> *location);
    RenderTargetType                  GetRenderTargetType();

private:
    std::map<FeatureType, SwFilter *> m_swFilters;
    // nullptr if it is unordered filters, otherwise, it's the pointer to m_OrderedFilters it belongs to.
    std::vector<class SwFilterSet *> *m_location = nullptr;

MEDIA_CLASS_DEFINE_END(vp__SwFilterSet)
};

}
#endif // !__SW_FILTER_H__
