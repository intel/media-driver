/*
* Copyright (c) 2022, Intel Corporation
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
#include "vp_user_setting.h"
#include "vp_utils.h"

MOS_STATUS VpUserSetting::InitVpUserSetting(MediaUserSettingSharedPtr userSettingPtr, bool clearViewMode)
{
    //skip for clearview perf purpose, debug dump common key needed.
    if (!clearViewMode)
    {
        DeclareUserSettingKey(  //For debugging purpose. true for disabling SFC
            userSettingPtr,
            __VPHAL_VEBOX_DISABLE_SFC,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(  //Disabling SFC DTR output. 1: Disable, 0: Enable
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_DTR_DISABLE,
            MediaUserSetting::Group::Sequence,
            1,
            true);

        DeclareUserSettingKey(  // For Notify which datapath Vebox used
            userSettingPtr,
            __VPHAL_VEBOX_OUTPUTPIPE_MODE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(  //For Notify which feature Vebox used
            userSettingPtr,
            __VPHAL_VEBOX_FEATURE_INUSE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(  //Disabling SFC Centering output. 1 -- Disable, 0 -- Enable.
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_SFC_OUTPUT_CENTERING_DISABLE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(  // VP Bypass Composition Mode
            userSettingPtr,
            __VPHAL_BYPASS_COMPOSITION,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_VEBOX_TGNE_ENABLE_VP,
            MediaUserSetting::Group::Sequence,
            1,
            true);  // Enable Vebox GNE. 1: Enable, 0: Disable

        DeclareUserSettingKey(  //Slice Shutdown Control
            userSettingPtr,
            __VPHAL_RNDR_SSD_CONTROL,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(  // FALSE if CSC coefficient setting mode is Patch mode, otherwise Curbe mode
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_DISABLE_DN,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_DISABLE_PACKET_REUSE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_PACKET_REUSE_TEAMS_ALWAYS,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_FORCE_ENABLE_VEBOX_OUTPUT_SURF,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __VPHAL_HDR_LUT_MODE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __VPHAL_HDR_GPU_GENERTATE_3DLUT,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __VPHAL_HDR_DISABLE_AUTO_MODE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(
            userSettingPtr,
            __VPHAL_HDR_SPLIT_FRAME_PORTIONS,
            MediaUserSetting::Group::Sequence,
            1,
            true);

        DeclareUserSettingKey(  // Eanble Apogeios path in VP PipeLine. 1: enabled, 0: disabled.
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE,
            MediaUserSetting::Group::Sequence,
            uint32_t(0),
            true);

        DeclareUserSettingKey(  // VP Render Target Compression Mode
            userSettingPtr,
            __VPHAL_RT_MMC_COMPRESSMODE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

#if (_DEBUG || _RELEASE_INTERNAL)
        DeclareUserSettingKey(  // VP Render Target Old Cache Usage
            userSettingPtr,
            __VPHAL_RT_Old_Cache_Setting,
            MediaUserSetting::Group::Sequence,
            0,
            true);
#endif

        DeclareUserSettingKey(  // VP Render Target Cache Usage
            userSettingPtr,
            __VPHAL_RT_Cache_Setting,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKey(  // VP Primary Input Compression Mode
            userSettingPtr,
            __VPHAL_PRIMARY_MMC_COMPRESSMODE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

#if (_DEBUG || _RELEASE_INTERNAL)
        DeclareUserSettingKeyForDebug(  // FORCE VP DECOMPRESSED OUTPUT
            userSettingPtr,
            __VPHAL_RNDR_FORCE_VP_DECOMPRESSED_OUTPUT,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  //Software Scoreboard enable Control
            userSettingPtr,
            __VPHAL_RNDR_SCOREBOARD_CONTROL,
            MediaUserSetting::Group::Sequence,
            1,
            true);

        DeclareUserSettingKeyForDebug(  // CM based FC enable Control
            userSettingPtr,
            __VPHAL_RNDR_CMFC_CONTROL,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            __VPHAL_FORCE_3DLUT_INTERPOLATION,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            __VPHAL_FORCE_VP_3DLUT_KERNEL_ONLY,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  //Enable 1K 1DLUT
            userSettingPtr,
            __VPHAL_ENABLE_1K_1DLUT,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(
            userSettingPtr,
            __VPHAL_VEBOX_HDR_MODE,
            MediaUserSetting::Group::Sequence,
            0,
            true);  //"HDR Mode. 0x1: H2S kernel, 0x3: H2H kernel, 0x21 65size H2S, 0x23 65size H2H, 0x31 33size H2S, 0x33 33size H2H."

        DeclareUserSettingKeyForDebug(  // For quality tuning purpose
            userSettingPtr,
            __VPHAL_HDR_ENABLE_QUALITY_TUNING,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  //For bit match purpose
            userSettingPtr,
            __VPHAL_HDR_ENABLE_KERNEL_DUMP,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // For HDR H2S RGB-based tone mapping
            userSettingPtr,
            __VPHAL_HDR_H2S_RGB_TM,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // VP Compression Enable
            userSettingPtr,
            __VPHAL_MMC_ENABLE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // VP Render Target Compressible
            userSettingPtr,
            __VPHAL_RT_MMC_COMPRESSIBLE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // VP Primary Input Compressible
            userSettingPtr,
            __VPHAL_PRIMARY_MMC_COMPRESSIBLE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // VP Enable Compute Context
            userSettingPtr,
            __VPHAL_ENABLE_COMPUTE_CONTEXT,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // Force VP Memorycopy Outputcompressed
            userSettingPtr,
            __VPHAL_VEBOX_FORCE_VP_MEMCOPY_OUTPUTCOMPRESSED,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // VP Composition 8Tap Adaptive Enable
            userSettingPtr,
            __VPHAL_COMP_8TAP_ADAPTIVE_ENABLE,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // Set SFC NV12/P010 Linear Output
            userSettingPtr,
            __VPHAL_ENABLE_SFC_NV12_P010_LINEAR_OUTPUT,
            MediaUserSetting::Group::Sequence,
            0,
            true);

        DeclareUserSettingKeyForDebug(  // Set SFC RGBP Linear/Tile RGB24 Linear Output
            userSettingPtr,
            __VPHAL_ENABLE_SFC_RGBP_RGB24_OUTPUT,
            MediaUserSetting::Group::Sequence,
            0,
            true);

       DeclareUserSettingKey(  // Enable HDR 3DLut table caculate by CPU. 1: enabled, 0: disabled.
            userSettingPtr,
            __VPHAL_HDR_3DLUT_CPU_PATH,
            MediaUserSetting::Group::Sequence,
            0,
            true);
#endif
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(  // VP Parameters Dump Outfile
        userSettingPtr,
        __VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME,
        MediaUserSetting::Group::Sequence,
        "",
        true);

    DeclareUserSettingKeyForDebug(  // VP Parameters Dump Start Frame
        userSettingPtr,
        __VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence,
        1,
        true);

    DeclareUserSettingKeyForDebug(  // VP Parameters Dump End Frame
        userSettingPtr,
        __VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(  // Vphal Debug Dump Output Directory
        userSettingPtr,
        __VPHAL_DBG_DUMP_OUTPUT_DIRECTORY,
        MediaUserSetting::Group::Sequence,
        "",
        true);

    DeclareUserSettingKeyForDebug(  // VP parameter dump sku and wa info enable
        userSettingPtr,
        __VPHAL_DBG_PARA_DUMP_ENABLE_SKUWA_DUMP,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_INTER_FRAME_MEMORY_NINJA_START_COUNTER,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_INTER_FRAME_MEMORY_NINJA_END_COUNTER,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_IFNCC,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VP_OCL_3DLUT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_VP_OCL_3DLUT_ENABLED,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VP_OCL_FC,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_VP_LEGACY_FC_IN_USE,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_VP_OCL_FC_FP,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_SUPPORTED,
        MediaUserSetting::Group::Sequence,
        0,
        true);

     DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_FEATURE_REPORT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_REPORT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VESFC_LINEAR_OUTPUT_BY_TILECONVERT,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_ID_REPORT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_USED_VEBOX_ID,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_FALLBACK_SCALING_TO_RENDER_8K,
        MediaUserSetting::Group::Sequence,
        0,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_FALLBACK_SCALING_TO_RENDER_8K_REPORT,
        MediaUserSetting::Group::Sequence,
        0,
        true);

#endif

    return MOS_STATUS_SUCCESS;
}