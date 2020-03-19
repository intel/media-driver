/*
* Copyright (c) 2020 Intel Corporation
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


#define SUPPORTED 1
#define UNSUPPORTED 0

#define VP_SFC_RESOLUTION_FEASIBLE(_InputWidth, _InputHeight, _OutputWidth, _OutputHeight, _90D_Rotation)    \


#define VP_FF_SFC_FORMAT(SurfaceFormat, bInput, bOutput, _MaxResolution, _MinResolution, _HorizUnit, _VertUnit, _RotationSupported, _MirrorSupported, _CscSupported, _ScalingSupported, \
                      _SrcAlphaSupported, _ConstAlphaSupported, _IScalingSupported, _DetheringSupported, _IefSupported, _Max_ScalingRatio, _Min_ScalingRatio)     \
        {                                                                                                                   \
            sfcHwEntry[SurfaceFormat].inputSupported                            = bInput;                                   \
            sfcHwEntry[SurfaceFormat].outputSupported                           = bOutput;                                  \
            sfcHwEntry[SurfaceFormat].maxResolution                             = _MaxResolution;                           \
            sfcHwEntry[SurfaceFormat].minResolution                             = _MinResolution;                           \
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

typedef struct VP_SFC_ENTRY_REC
{
    bool                          inputSupported;
    bool                          outputSupported;
    uint32_t                      maxResolution;
    uint32_t                      minResolution;
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

#define VP_FF_VEBOX_FORMAT(SurfaceFormat, bInput, bOutput, _MaxResolution, _MinResolution, _HorizUnit, _VertUnit, _HdrSupported, _CapturePipeSupported, \
                           _DNSupported, _DISupported, _LACESupported, _FrontCscSupported, _BackEndCscSupported, _3DLutSupported, _IecpSupported, _bHsbMode)     \
        {                                                                                                                   \
            veboxHwEntry[SurfaceFormat].inputSupported                          = bInput;                                   \
            veboxHwEntry[SurfaceFormat].outputSupported                         = bOutput;                                  \
            veboxHwEntry[SurfaceFormat].maxResolution                           = _MaxResolution;                           \
            veboxHwEntry[SurfaceFormat].minResolution                           = _MinResolution;                           \
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
        }                                                                                                                   \

typedef struct VP_VEBOX_ENTRY_REC
{
    bool                          inputSupported;
    bool                          outputSupported;
    uint32_t                      maxResolution;
    uint32_t                      minResolution;
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
}VP_VEBOX_ENTRY_REC;

#endif// __VP_FEATURE_CAPS_H__