/*
* Copyright (c) 2020-2021 Intel Corporation
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
//! \file     vp_feature_caps.h
//! \brief    Defines the common interface for vp features caps on diff engines
//! \details  The vp feature caps is designed based on vp engines
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __VP_FEATURE_CAPS_H__
#define __VP_FEATURE_CAPS_H__
#include "vp_utils.h"
#include "mos_resource_defs.h"

#define SUPPORTED 1
#define UNSUPPORTED 0

#define VP_SFC_RESOLUTION_FEASIBLE(_InputWidth, _InputHeight, _OutputWidth, _OutputHeight, _90D_Rotation)    \


#define VP_FF_SFC_FORMAT(SurfaceFormat, bInput, bOutput, _MaxResolution, _InputMinResolution, _OutputMinResolution, _HorizUnit, _VertUnit, _RotationSupported, _MirrorSupported, _CscSupported, _ScalingSupported, \
                      _SrcAlphaSupported, _ConstAlphaSupported, _IScalingSupported, _DetheringSupported, _IefSupported, _Max_ScalingRatio, _Min_ScalingRatio)     \
        {                                                                                                                   \
            sfcHwEntry[SurfaceFormat].inputSupported                            = bInput;                                   \
            sfcHwEntry[SurfaceFormat].outputSupported                           = bOutput;                                  \
            sfcHwEntry[SurfaceFormat].maxResolution                             = _MaxResolution;                           \
            sfcHwEntry[SurfaceFormat].inputMinResolution                        = _InputMinResolution;                      \
            sfcHwEntry[SurfaceFormat].outputMinResolution                       = _OutputMinResolution;                     \
            sfcHwEntry[SurfaceFormat].horizontalAlignUnit                       = _HorizUnit;                               \
            sfcHwEntry[SurfaceFormat].verticalAlignUnit                         = _VertUnit;                                \
            sfcHwEntry[SurfaceFormat].rotationSupported                         = _RotationSupported;                       \
            sfcHwEntry[SurfaceFormat].mirrorSupported                           = _MirrorSupported;                         \
            sfcHwEntry[SurfaceFormat].cscSupported                              = _CscSupported;                            \
            sfcHwEntry[SurfaceFormat].scalingSupported                          = _ScalingSupported;                        \
            sfcHwEntry[SurfaceFormat].sourceAlphaSupported                      = _SrcAlphaSupported;                       \
            sfcHwEntry[SurfaceFormat].constAlphaSupported                       = _ConstAlphaSupported;                     \
            sfcHwEntry[SurfaceFormat].iScalingSupported                         = _IScalingSupported;                       \
            sfcHwEntry[SurfaceFormat].detheringSupported                        = _DetheringSupported;                      \
            sfcHwEntry[SurfaceFormat].minScalingRatio                           = _Min_ScalingRatio;                        \
            sfcHwEntry[SurfaceFormat].maxScalingRatio                           = _Max_ScalingRatio;                        \
            sfcHwEntry[SurfaceFormat].iefSupported                              = _IefSupported;                            \
        }                                                                                                                   \

enum VP_TILE_MODE_MASK
{
    VP_TILE_MODE_MASK_UNUSED = 0,  // 000
    VP_TILE_MODE_MASK_TILE4  = 1,  // 001 1st bit: tile4
    VP_TILE_MODE_MASK_TILE64 = 2,  // 010 2nd bit: tile64
    VP_TILE_MODE_MASK_LINEAR = 4   // 100 3rd bit linear
};

//!
//! \ brief VP SFC Tile Mode Format support
//!
enum VP_SFC_OUTUT_SUPPORT
{
    VP_SFC_OUTPUT_SUPPORT_NONE      = 0,
    VP_SFC_OUTPUT_SUPPORT_TILE_ONLY = (VP_TILE_MODE_MASK_TILE4 | VP_TILE_MODE_MASK_TILE64),
    VP_SFC_OUTPUT_SUPPORT_ALL       = (VP_TILE_MODE_MASK_TILE4 | VP_TILE_MODE_MASK_TILE64 | VP_TILE_MODE_MASK_LINEAR)
};

inline uint32_t VpGetFormatTileSupport(MOS_TILE_MODE_GMM tileMode)
{
    switch (tileMode)
    {
    case MOS_TILE_64_GMM:      //tile64
        return VP_TILE_MODE_MASK_TILE64;
    case MOS_TILE_LINEAR_GMM:  //tilelinear
        return VP_TILE_MODE_MASK_LINEAR;
    case MOS_TILE_4_GMM:       // tile4
    default:
        return VP_TILE_MODE_MASK_TILE4;
    }
}

typedef struct VP_SFC_ENTRY_REC
{
    bool                          inputSupported;
    VP_SFC_OUTUT_SUPPORT          outputSupported;  // 1st bit: tile4, 2nd bit: tile64, 3rd bit linear
    uint32_t                      maxResolution;
    uint32_t                      inputMinResolution;
    uint32_t                      outputMinResolution;
    float                         maxScalingRatio;
    float                         minScalingRatio;
    uint32_t                      horizontalAlignUnit;
    uint32_t                      verticalAlignUnit;
    bool                          rotationSupported;
    bool                          mirrorSupported;
    bool                          cscSupported;
    bool                          scalingSupported;
    bool                          sourceAlphaSupported;
    bool                          constAlphaSupported;
    bool                          iScalingSupported;
    bool                          detheringSupported;
    bool                          iefSupported;
}VP_SFC_ENTRY_REC;

#define VP_FF_VEBOX_FORMAT(SurfaceFormat, bInput, bOutput, _MaxWidth, _MaxHeight, _MinWidth, _MinHeight, _HorizUnit, _VertUnit, _HdrSupported, _CapturePipeSupported, \
                           _DNSupported, _DISupported, _LACESupported, _FrontCscSupported, _BackEndCscSupported, _3DLutSupported, _IecpSupported, _bHsbMode, \
                           _TCCSupported, _ACESupported, _STESupported, _CGCSupported)     \
        {                                                                                                                   \
            veboxHwEntry[SurfaceFormat].inputSupported                          = bInput;                                   \
            veboxHwEntry[SurfaceFormat].outputSupported                         = bOutput;                                  \
            veboxHwEntry[SurfaceFormat].maxWidth                                = _MaxWidth;                           \
            veboxHwEntry[SurfaceFormat].maxHeight                               = _MaxHeight;                           \
            veboxHwEntry[SurfaceFormat].minWidth                                = _MinWidth;                           \
            veboxHwEntry[SurfaceFormat].minHeight                               = _MinHeight;                           \
            veboxHwEntry[SurfaceFormat].horizontalAlignUnit                     = _HorizUnit;                               \
            veboxHwEntry[SurfaceFormat].verticalAlignUnit                       = _VertUnit;                                \
            veboxHwEntry[SurfaceFormat].hdrSupported                            = _HdrSupported;                            \
            veboxHwEntry[SurfaceFormat].capturePipeSupported                    = _CapturePipeSupported;                    \
            veboxHwEntry[SurfaceFormat].denoiseSupported                        = _DNSupported;                             \
            veboxHwEntry[SurfaceFormat].deinterlaceSupported                    = _DISupported;                             \
            veboxHwEntry[SurfaceFormat].laceSupported                           = _LACESupported;                           \
            veboxHwEntry[SurfaceFormat].frontCscSupported                       = _FrontCscSupported;                       \
            veboxHwEntry[SurfaceFormat].backEndCscSupported                     = _BackEndCscSupported;                     \
            veboxHwEntry[SurfaceFormat].b3dLutSupported                         = _3DLutSupported;                          \
            veboxHwEntry[SurfaceFormat].iecp                                    = _IecpSupported;                           \
            veboxHwEntry[SurfaceFormat].hsb                                     = _bHsbMode;                                \
            veboxHwEntry[SurfaceFormat].tccSupported                            = _TCCSupported;                            \
            veboxHwEntry[SurfaceFormat].aceSupported                            = _ACESupported;                            \
            veboxHwEntry[SurfaceFormat].tccSupported                            = _TCCSupported;                            \
            veboxHwEntry[SurfaceFormat].steSupported                            = _STESupported;                            \
            veboxHwEntry[SurfaceFormat].cgcSupported                            = _CGCSupported;                            \
        }                                                                                                                   \

typedef struct VP_VEBOX_ENTRY_REC
{
    bool                          inputSupported;
    bool                          outputSupported;
    uint32_t                      maxWidth;
    uint32_t                      maxHeight;
    uint32_t                      minWidth;
    uint32_t                      minHeight;
    float                         maxScalingRatio;
    float                         minScalingRatio;
    uint32_t                      horizontalAlignUnit;
    uint32_t                      verticalAlignUnit;
    bool                          hdrSupported; // Gamut Expansion, HDR and Forward Gamma Correction are mutually exclusive.
    bool                          capturePipeSupported;
    bool                          denoiseSupported;
    bool                          deinterlaceSupported;
    bool                          laceSupported;
    bool                          frontCscSupported;
    bool                          backEndCscSupported;
    bool                          b3dLutSupported;
    bool                          iecp;// all IECP features like procamp/STD/Gamut etc
    bool                          hsb;// high speed bypass mode
    bool                          tccSupported;
    bool                          aceSupported;
    bool                          steSupported;
    bool                          cgcSupported;
}VP_VEBOX_ENTRY_REC;

struct VP_POLICY_RULES
{
    struct
    {
        struct
        {
            bool enable = false;                            // true if enable 2 pass scaling.
            struct
            {
                float ratioFor1stPass  = 0;                 // scaling ratio for 1st pass when 2 pass downscaling needed. valid value in [minSfcScalingRatio, 1).
                float minRatioEnlarged = 0;                 // min ratio enlarged according to minSfcScalingRatio. valid value is [ratioFor1stPass, 1).
                                                            // minRatio for 2 pass upscaling is minSfcScalingRatio * minRatioEnlarged.
                bool scalingIn1stPassIf1PassEnough = false; // For 1 pass enough case, if true, do scaling in 1st pass, otherwise, do scaling in 2nd pass.
                                                            // e.g. ratioX being 1/2 and ratioY being 1/16, if true, width will do scaling in 1st pass,
                                                            // otherwise, width will do scaling in 2nd pass
            } downScaling;
            struct
            {
                float ratioFor1stPass  = 0;                 // scaling ratio for 1st pass when 2 pass upscaling needed. valid value in (1, maxSfcScalingRatio].
                float maxRatioEnlarged = 0;                 // Max ratio enlarged according to maxSfcScalingRatio. valid value is (1, ratioFor1stPass].
                                                            // maxRatio for 2 pass upscaling is maxSfcScalingRatio * maxRatioEnlarged.
                bool scalingIn1stPassIf1PassEnough = false; // For 1 pass enough case, if true, do scaling in 1st pass, otherwise, do scaling in 2nd pass.
                                                            // e.g. ratioX being 2 and ratioY being 16, if true, width will do scaling in 1st pass,
                                                            // otherwise, width will do scaling in 2nd pass
            } upScaling;
        } scaling;
        struct
        {
            bool enable = false;
        } csc;
    } sfcMultiPassSupport;

    bool isAvsSamplerSupported;
    bool isHDR3DLutKernelEnabled;
    bool is1K1DLutSurfaceInUse;
    bool isHDR33LutSizeEnabled = false;
};

struct VP_HW_CAPS
{
    VP_SFC_ENTRY_REC    m_sfcHwEntry[Format_Count] = {};
    VP_VEBOX_ENTRY_REC  m_veboxHwEntry[Format_Count] = {};
    VP_POLICY_RULES     m_rules = {};
};

#endif// __VP_FEATURE_CAPS_H__