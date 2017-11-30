/*
* Copyright (c) 2009-2017, Intel Corporation
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
//! \file     vphal_common_composite.h
//! \brief    vphal composite interface clarification
//! \details  vphal composite interface clarification inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_COMMON_COMPOSITE_H__
#define __VPHAL_COMMON_COMPOSITE_H__

// Compositing Block size
#define VPHAL_COMP_BLOCK_WIDTH  16
#define VPHAL_COMP_BLOCK_HEIGHT 16

// NLAS Default Values
#define NLAS_VERTICALCROP_MIN         0.0F
#define NLAS_VERTICALCROP_MAX         1.0F
#define NLAS_VERTICALCROP_DEFAULT     0.0F
#define NLAS_VERTICALCROP_STEP        0.001F
#define NLAS_HLINEARREGION_MIN        0.0F
#define NLAS_HLINEARREGION_MAX        1.0F
#define NLAS_HLINEARREGION_DEFAULT    1.0F
#define NLAS_HLINEARREGION_STEP       0.001F
#define NLAS_NONLINEARCROP_MIN        0.0F
#define NLAS_NONLINEARCROP_MAX        1.0F
#define NLAS_NONLINEARCROP_DEFAULT    0.0F
#define NLAS_NONLINEARCROP_STEP       0.001F

//!
//! Structure VPHAL_IEF_PARAMS
//! \brief IEF parameters - Image Enhancement (Detail) Filter
//!
typedef struct _VPHAL_IEF_PARAMS
{
    bool                bEnabled;
    bool                bSmoothMode;
    bool                bSkintoneTuned;
    bool                bEmphasizeSkinDetail;
    float               fIEFFactor;
    uint16_t            StrongEdgeWeight;
    uint16_t            RegularWeight;
    uint16_t            StrongEdgeThreshold;
    void*               pExtParam;
} VPHAL_IEF_PARAMS, *PVPHAL_IEF_PARAMS;

//!
//! Structure VPHAL_NLAS_PARAMS
//! \brief NLAS parameters - Non-Anamorphic Scaling
//!
typedef struct _VPHAL_NLAS_PARAMS
{
    float               fVerticalCrop;
    float               fHLinearRegion;
    float               fNonLinearCrop;
} VPHAL_NLAS_PARAMS, *PVPHAL_NLAS_PARAMS;

//!
//! Structure VPHAL_COLORFILL_PARAMS
//! \brief ColorFill parameters
//!
typedef struct _VPHAL_COLORFILL_PARAMS
{
    bool                bYCbCr;
    uint32_t            Color;
    VPHAL_CSPACE        CSpace;
} VPHAL_COLORFILL_PARAMS, *PVPHAL_COLORFILL_PARAMS;

//!
//! Structure VPHAL_ALPHA_FILL_MODE
//! \brief Alpha mode
//!
typedef enum _VPHAL_ALPHA_FILL_MODE
{
  VPHAL_ALPHA_FILL_MODE_NONE      = 0,
  VPHAL_ALPHA_FILL_MODE_OPAQUE,
  VPHAL_ALPHA_FILL_MODE_BACKGROUND,
  VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM,
} VPHAL_ALPHA_FILL_MODE;

//!
//! Structure VPHAL_ALPHA_PARAMS
//! \brief Alpha parameters
//!
typedef struct _VPHAL_ALPHA_PARAMS
{
    float                     fAlpha;
    VPHAL_ALPHA_FILL_MODE     AlphaMode;
} VPHAL_ALPHA_PARAMS, *PVPHAL_ALPHA_PARAMS;

//!
//! Structure VPHAL_CONSTRICTION_PARAMS
//! \brief Constriction parameters
//!
typedef struct _VPHAL_CONSTRICTION_PARAMS
{
    RECT                rcConstriction;
} VPHAL_CONSTRICTION_PARAMS, *PVPHAL_CONSTRICTION_PARAMS;

//!
//! Structure VPHAL_BLENDING_PARAMS
//! \brief Layer Blending parameters
//!
typedef struct _VPHAL_BLENDING_PARAMS
{
    VPHAL_BLEND_TYPE        BlendType;
    float                   fAlpha;
} VPHAL_BLENDING_PARAMS, *PVPHAL_BLENDING_PARAMS;

//!
//! Structure VPHAL_LUMAKEY_PARAMS
//! \brief Luma Keying parameters
//!
typedef struct _VPHAL_LUMAKEY_PARAMS
{
    int16_t               LumaLow;
    int16_t               LumaHigh;
} VPHAL_LUMAKEY_PARAMS, *PVPHAL_LUMAKEY_PARAMS;

#endif  // __VPHAL_COMMON_COMPOSITE_H__
