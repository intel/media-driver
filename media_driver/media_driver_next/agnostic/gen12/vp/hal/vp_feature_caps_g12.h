/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     vp_feature_caps_g12.cpp
//! \brief    Defines the common interface for vp features caps on diff engines
//! \details  The vp feature caps is designed based on vp engines
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __VP_FEATURE_CAPS__G12_H__
#define __VP_FEATURE_CAPS__G12_H__
#include "vp_feature_caps.h"

//#define VP_FF_FORMAT(Name, bInput, bOutput, _MaxResolution, _MinResolution, _HorizUnit, _VertUnit, _RotationSupported, _MirrorSupported, _CscSupported, _ScalingSupported, 
//                      _SrcAlphaSupported, _ConstAlphaSupported, _IScalingSupported, _DetheringSupported, _IefSupported)
#define MHW_SFC_MIN_RESOLUTION_G12 128;
#define MHW_SFC_MAX_RESOLUTION_G12 (16 * 1024);

#define MHW_SFC_MIN_SCALING_RATIO (1.0 / 8.0)
#define MHW_SFC_MAX_SCALING_RATIO (8.0)

/****************************************************************************\
  SFC Feature Caps TABLE

#define VP_FF_FORMAT(SurfaceFormat, bInput, bOutput, _MaxResolution, _MinResolution, _HorizUnit, _VertUnit, _RotationSupported, _MirrorSupported, _CscSupported, _ScalingSupported, 
                      _SrcAlphaSupported, _ConstAlphaSupported, _IScalingSupported, _DetheringSupported, _IefSupported)
  SFC IEF Suppored -------------------------------------------------------------------------------------------------------------------------o
      Dethering Suppored ---------------------------------------------------------------------------------------------------------------o   |
      InterlaceScaling Suppored ----------------------------------------------------------------------------------------------------o   |   |
      Const Alpha Suppored -----------------------------------------------------------------------------------------------------o   |   |   |
      Src Alpha Suppored ---------------------------------------------------------------------------------------------------o   |   |   |   |
      Scaling Suppored -------------------------------------------------------------------------------------------------o   |   |   |   |   |
      CSC Suppored -------------------------------------------------------------------------------------------------o   |   |   |   |   |   |
      Mirror Suppored ------------------------------------------------------------------------------------------o   |   |   |   |   |   |   |
      Roattion Suppored ------------------------------------------------------------------------------------o   |   |   |   |   |   |   |   |
      Processing Unit in Vertical ----------------------------------------------------------------------o   |   |   |   |   |   |   |   |   |
      Processing Unit in Horizontal ----------------------------------------------------------------o   |   |   |   |   |   |   |   |   |   |
      MAX Process Resolution(Width/Height) ---------------------------------------o                 |   |   |   |   |   |   |   |   |   |   |
      MAX Process Resolution(Width/Height) ------------o                          |                 |   |   |   |   |   |   |   |   |   |   |
  Output Format Supported --------------o              |                          |                 |   |   |   |   |   |   |   |   |   |   |
  Input Format Supported -----------o   |              |                          |                 |   |   |   |   |   |   |   |   |   |   |
                Format              |   |              |                          |                 |   |   |   |   |   |   |   |   |   |   |
-----------------------------------------------------------------------------------------------------------------------------------------------*/
VP_FF_FORMAT(Format_NV12,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  2,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_P010,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  2,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_P016,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  2,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_YUY2,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_YUYV,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_UYVY,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y216,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y210,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_P216,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_P210,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_AYUV,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y416,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y410,           1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_YVYU,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_VYUY,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  2,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_A8B8G8R8,       1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_X8B8G8R8,       1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_A8R8G8B8,       1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_X8R8G8B8,       1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_A16B16G16R16,   1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_A16R16G16B16,   1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_A16B16G16R16F,  1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_A16R16G16B16F,  1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_L8,             1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_P8,             1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y8,             1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y16S,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_Y16U,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_R10G10B10A2,    1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_B10G10R10A2,    1,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_R5G6B5,         0,  1, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)

VP_FF_FORMAT(Format_IRW0,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW1,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW2,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW3,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW4,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW5,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW6,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)
VP_FF_FORMAT(Format_IRW7,           1,  0, MHW_SFC_MAX_RESOLUTION_G12, MHW_SFC_MIN_RESOLUTION_G12,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1, MHW_SFC_MAX_SCALING_RATIO, MHW_SFC_MIN_SCALING_RATIO)

#endif //__VP_FEATURE_CAPS__G12_H__