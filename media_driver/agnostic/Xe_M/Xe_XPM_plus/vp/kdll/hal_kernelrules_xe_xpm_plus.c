/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file      hal_kernelrules_xe_xpm_plus.c 
//! \brief         Fast Compositing Kernel DLL rules for Xe_XPM_plus FC 
//!
#include "hal_kerneldll.h"  // Rule definitions
#include "vpkrnheader.h"    // Kernel IDs

extern const Kdll_RuleEntry g_KdllRuleTable_xe_xpm_plus[] =
{
    // Kernel Setup

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_Begin                       , Kdll_None },
    { RID_SetParserState   , Parser_SetRenderMethod             , Kdll_None },

    //set upscaling walker
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetRenderMethod             , Kdll_None },
    { RID_IsRenderMethod   , RenderMethod_MediaObjectWalker     , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_DP_FC_Setup_Walker_16x16    , Kdll_None },
    { RID_SetParserState   , Parser_SetupLayer0                 , Kdll_None },

    //set scalingRatio (1/2, 1+1/6]'s walker
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetRenderMethod             , Kdll_None },
    { RID_IsRenderMethod   , RenderMethod_MediaObjectWalker     , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_DP_FC_Setup_Walker_8x8      , Kdll_None },
    { RID_SetParserState   , Parser_SetupLayer0                 , Kdll_None },

    //set scalingRatio <= 1/2 's walker
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetRenderMethod             , Kdll_None },
    { RID_IsRenderMethod   , RenderMethod_MediaObjectWalker     , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_DP_FC_Setup_Walker_4x4      , Kdll_None },
    { RID_SetParserState   , Parser_SetupLayer0                 , Kdll_None },

    // 1st layer
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetupLayer0                 , Kdll_None },
    { RID_IsLayerNumber    , 0                                  , Kdll_None },
    //{ RID_SetKernel        , IDR_VP_Set_Layer_0                 , Kdll_None },
    { RID_SetParserState   , Parser_SetParamsLayer0             , Kdll_None },

    // Set Layer 1

    // Please don't change the order of 2 rulesets below.
    // Single layer
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetupLayer1                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Any                         , Kdll_None },  // Src0 must be valid for single layer case.
    { RID_SetNextLayer     , -1                                 , Kdll_None },  // backoff one layer
    { RID_SetParserState   , Parser_SampleLayer0                , Kdll_None },

    // Last layer
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetupLayer1                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_SetParserState   , Parser_SetParamsTarget             , Kdll_None },  // Setup CSC for render target

    // Setup Parameters for Layer 0

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetParamsLayer0             , Kdll_None },
    { RID_SetSrc0Format    , Format_Source                      , Kdll_None },
    { RID_SetSrc0Coeff     , CoeffID_Source                     , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_Source                      , Kdll_None },
    { RID_SetSrc0Rotation  , Rotate_Source                      , Kdll_None },
    { RID_SetSrc0ColorFill , ColorFill_Source                   , Kdll_None },
    { RID_SetSrc0LumaKey   , LumaKey_Source                     , Kdll_None },
    { RID_SetSrc0Procamp   , Procamp_Source                     , Kdll_None },
    { RID_SetSrc0Processing, Process_Source                     , Kdll_None },
    { RID_SetNextLayer     , 0                                  , Kdll_None },
    { RID_SetParserState   , Parser_SetupLayer1                 , Kdll_None },

    // Setup Parameters for Render Target

    //If only RT is present, it's colorfill only case. Go to write directly.
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetParamsTarget             , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerNumber    , 0                                  , Kdll_None },
    { RID_SetSrc0ColorFill , ColorFill_Source                   , Kdll_None },
    { RID_SetSrc0Format    , Format_Source                      , Kdll_None },
    { RID_SetParserState   , Parser_WriteOutput                 , Kdll_None },


    // If CSC0 is done before Mix, then jump to SetupCSC1
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetParamsTarget             , Kdll_None },
    { RID_IsCSCBeforeMix   , true                               , Kdll_None },
    { RID_SetTargetCspace  , CSpace_Source                      , Kdll_None },
    { RID_SetParserState   , Parser_SetupCSC1                   , Kdll_None },

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetParamsTarget             , Kdll_None },
    { RID_SetSrc0Format    , Format_Source                      , Kdll_None },
    { RID_SetSrc0Coeff     , CoeffID_Source                     , Kdll_None },
    { RID_SetTargetCspace  , CSpace_Source                      , Kdll_None },
    { RID_SetParserState   , Parser_SetupCSC0                   , Kdll_None },

    // ARGB/AGBP-->RGBP
    // ------------ARGB/ABGR Rotation with the scalingratio >1+1/6------------------
    // 90 Rotation, ARGB/ABGR upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, ARGB/ABGR upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, ARGB/ABGR upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR Rotation with the scalingratio (1/2, 1+1/6]------------------
    // 90 Rotation, ARGB/ABGR scalingRatio(1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, ARGB/ABGR  scalingRatio(1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, ARGB/ABGR scalingRatio(1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR Rotation with the scalingratio (1/4, 1/2]-------------------
    // 90 Rotation, ARGB/ABGR scalingRatio(1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, ARGB/ABGR scalingRatio(1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, ARGB/ABGR scalingRatio(1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR Rotation with the scalingratio (1/8, 1/4]-------------------
    // 90 Rotation, ARGB/ABGR scalingRatio(1/8, 1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
   // { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_rot_90, Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, ARGB/ABGR scalingRatio(1/8, 1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, ARGB/ABGR scalingRatio(1/8, 1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
   // { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_rot_270, Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR Rotation with the scalingratio <=1/8-------------------
    // 90 Rotation, ARGB/ABGR scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, ARGB/ABGR scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, ARGB/ABGR scalingRatio(1/8, 1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR mirror with the scalingratio > 1+1/6-------------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //  90 MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR mirror with the scalingratio (1/2, 1+1/6]-------------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //  90 MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR mirror with the scalingratio (1/4, 1/2]-------------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //  90 MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR mirror with the scalingratio (1/8, 1/4]-------------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

      // ------------ARGB/ABGR mirror with the scalingratio <=1/8-------------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------ARGB/ABGR no Rotation ------------------
    // No Rotation, ARGB/ABGR upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_UpScaling_16x16 , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio (1/2, 1]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_8x8 , Kdll_None  },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio (1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None},
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_R8G8B8A8_DP_DownScaling_4x4 , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio (1/8, 1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None},
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel, IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_1_8th, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                         , Kdll_None},
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel, IDR_VP_R8G8B8A8_DP_DownScaling_4x4_scale_to_any_ratio, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // NV12-->RGBP
    //------------------NV12 Ratation with scalingratio > 1+1/6----------------
    // 90 Rotation, NV12 upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, NV12 upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, NV12 upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 Ratation with scalingratio (1/2, 1+1/6]----------------
    // 90 Rotation, NV12 scalingRatio(1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, NV12 scalingRatio(1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, NV12 scalingRatio(1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 Ratation with scalingratio (1/4,1/2]----------------
    // 90 Rotation, NV12 scalingRatio(1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, NV12 scalingRatio(1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, NV12 scalingRatio(1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 Ratation with scalingratio (1/8,1/4]----------------
    // 90 Rotation, NV12 scalingRatio (1/8,1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, NV12 scalingRatio (1/8,1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, NV12 scalingRatio (1/8,1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 Ratation with scalingratio <=1/8 ----------------
    // 90 Rotation, NV12 scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_90                  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_rot_90, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 180 Rotation, NV12 scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_180                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_rot_180, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 270 Rotation, NV12 scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATION_270                 , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_rot_270, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 mirror > 1+1/6----------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTA
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 mirror (1/2, 1+1/6]----------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTA
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 mirror (1/4, 1/2]----------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTA
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 mirror (1/8, 1/4]----------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTA
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    //------------------NV12 mirror <=1/8----------------
    // MIRROR_HORIZONTAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_HORIZONTAL            , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_MIRROR_VERTICAL              , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_HORIZONTA
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_HORIZONTAL  , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_rot_90_mirror_h, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // 90 MIRROR_VERTICAL
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsSrc0Rotation   , VPHAL_ROTATE_90_MIRROR_VERTICAL    , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio_rot_90_mirror_v, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // ------------------NV12 without Rotation ----------------
    // No Rotation, NV12 upscaling
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_UpScaling_16x16     , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio (1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_8x8     , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio (1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None},
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_None },
    { RID_SetKernel        , IDR_VP_NV12_DP_DownScaling_4x4     , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio (1/8, 1/4]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None},
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_None },
    { RID_SetKernel, IDR_VP_NV12_DP_DownScaling_4x4_scale_1_8th , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // No Rotation down scalingRatio <=1/8
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0                , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                        , Kdll_None},
    { RID_IsSrc0Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel, IDR_VP_NV12_DP_DownScaling_4x4_scale_to_any_ratio, Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer0Done            , Kdll_None },

    // Src0 Sampling is complete -> if ARGB/ABGR->RGBP/ARGB, no CSC
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                              , Kdll_None },
    { RID_IsTargetFormat   , Format_RGBP                             , Kdll_Or   },
    { RID_IsTargetFormat   , Format_A8R8G8B8                         , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    // Src0 Sampling is complete -> if NV12->NV12, no CSC
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                             , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    // Src0 Sampling is complete -> nv12->RGBP/ARGB do CSC
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                             , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_Not  },
    { RID_IsScalingRatio   , Scalingratio_over1                      , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetKernel        , IDR_VP_CSC_16x16                        , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                             , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_Not  },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1                    , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetKernel        , IDR_VP_CSC_8x8                          , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_NV12                             , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_Not  },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2                  , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4                  , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_Any                        , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetKernel        , IDR_VP_CSC_4x4                          , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

     // Src0 Sampling is complete -> RGBP/ARGB->NV12 do CSC
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                              , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_None},
    { RID_IsScalingRatio   , Scalingratio_over1                      , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetKernel        , IDR_VP_CSC_16x16                        , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                              , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1                    , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetKernel        , IDR_VP_CSC_8x8                          , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                        , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer0Done                 , Kdll_None },
    { RID_IsSrc0Format     , Format_RGB                              , Kdll_None },
    { RID_IsTargetFormat   , Format_NV12                             , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2                  , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4                  , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_Any                        , Kdll_None },
    { RID_SetSrc0Sampling  , Sample_None                             , Kdll_None },
    { RID_SetSrc0Format    , Format_None                             , Kdll_None },
    { RID_SetKernel        , IDR_VP_CSC_4x4                          , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                     , Kdll_None },

    // Sampling is complete for both layers, perform CSC
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer1                , Kdll_None },
    { RID_IsSrc0Sampling   , Sample_None                        , Kdll_None },
    { RID_IsSrc1Sampling   , Sample_None                        , Kdll_None },
    { RID_SetParserState   , Parser_SetupCSC1                   , Kdll_None },

    // No Rotation
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_SampleLayer1                , Kdll_None },
    { RID_IsSrc1Format     , Format_PL3                         , Kdll_Or   },
    { RID_IsSrc1Format     , Format_PL3_RGB                     , Kdll_None },
    { RID_IsSrc1Sampling   , Sample_Scaling_034x                , Kdll_None },
    { RID_SetParserState   , Parser_SetupCSC1                   , Kdll_None },

    // Setup CSC coefficients for layer 0
    // CSC not present for Render Target: if RGB->RGBP no CSC
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetupCSC0                   , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsSrc0Coeff      , CoeffID_None                       , Kdll_Or   },
    { RID_IsSrc0Coeff      , CoeffID_0                          , Kdll_None },
    { RID_SetParserState   , Parser_WriteOutput                 , Kdll_None },
 
    // Setup CSC coefficients for layer 1

    // No color space conversion for any layer
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_SetupCSC1                   , Kdll_None },
    { RID_IsSrc0Coeff      , CoeffID_None                       , Kdll_None },
    { RID_IsSrc1Coeff      , CoeffID_None                       , Kdll_None },
    { RID_SetParserState   , Parser_Lumakey                     , Kdll_None },

    // Lumakey
    // lumakey and CSC not needed for current layer.
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_Lumakey                     , Kdll_None },
    { RID_IsSrc1Coeff      , CoeffID_None                       , Kdll_None },
    { RID_IsSrc1LumaKey    , LumaKey_False                      , Kdll_None },
    { RID_SetParserState   , Parser_ProcessLayer                , Kdll_None },
     //--------

    // CSC0 is complete
    // Process layer

    // Single layer -> skip processing - prepare CSC for Render Target
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_ProcessLayer                , Kdll_None },
    { RID_IsSrc1Processing , Process_None                       , Kdll_None },
    { RID_SetParserState   , Parser_ProcessLayerDone            , Kdll_None },

    // Layer processing is complete

    // Quadrants 0,1 are complete - sample/process quadrants 2,3
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None },
    { RID_IsParserState    , Parser_ProcessLayerDone            , Kdll_None },
    { RID_IsQuadrant       , 0                                  , Kdll_None },
    { RID_SetQuadrant      , 2                                  , Kdll_None },
    { RID_SetParserState   , Parser_SampleLayer1                , Kdll_None },

    // All quadrants are processed - start next layer
    { RID_Op_NewEntry      , RULE_NO_OVERRIDE                   , Kdll_None }, 
    { RID_IsParserState    , Parser_ProcessLayerDone            , Kdll_None },
    { RID_IsQuadrant       , 2                                  , Kdll_None },
    { RID_SetSrc1Sampling  , Sample_None                        , Kdll_None },
    { RID_SetSrc1Format    , Format_None                        , Kdll_None },
    { RID_SetSrc1Processing, Process_None                       , Kdll_None },
    { RID_SetNextLayer     , 0                                  , Kdll_None },
    { RID_SetQuadrant      , 0                                  , Kdll_None },
    { RID_SetParserState   , Parser_SetupLayer1                 , Kdll_None },

    // Write
    // Write RGBP - Normal Save, upscaling ratio >1
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_RGBP                        , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_RGBP_16x16             , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

     // Write RGBP - Normal Save, down scalingRatio (1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_RGBP                        , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_RGBP_8x8               , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

    // Write RGBP - Normal Save, down scalingRatio (1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_RGBP                        , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_RGBP_4x4               , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

     // Write
    // Write ARGB - Normal Save, upscaling ratio > 1 + 1/6
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_RGB                         , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_ARGB_16x16             , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

     // Write ARGB - Normal Save, down scalingRatio (1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_RGB                         , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_ARGB_8x8               , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

    // Write ARGB - Normal Save, down scalingRatio (1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_RGB                         , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_ARGB_4x4               , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

    // Write NV12 - Normal Save, upscaling ratio > 1 + 1/6
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_NV12                        , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_over1                 , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_NV12_16x16             , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

     // Write NV12 - Normal Save, down scalingRatio (1/2, 1+1/6]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_NV12                        , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p2to1               , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_NV12_8x8               , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

    // Write NV12 - Normal Save, down scalingRatio (1/4, 1/2]
    { RID_Op_NewEntry      , RULE_DEFAULT                       , Kdll_None },
    { RID_IsParserState    , Parser_WriteOutput                 , Kdll_None },
    { RID_IsLayerID        , Layer_RenderTarget                 , Kdll_None },
    { RID_IsLayerFormat    , Format_NV12                        , Kdll_None },
    { RID_IsScalingRatio   , Scalingratio_b1p4to1p2             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_b1p8to1p4             , Kdll_Or   },
    { RID_IsScalingRatio   , Scalingratio_Any                   , Kdll_None },
    { RID_SetKernel        , IDR_VP_Save_NV12_4x4               , Kdll_None },
    { RID_SetKernel        , IDR_VP_EOT                         , Kdll_None },
    { RID_SetParserState   , Parser_End                         , Kdll_None },

    //last entry
    { RID_Op_EOF           , 0                                  , Kdll_None }
};
