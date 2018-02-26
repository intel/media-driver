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
//! \file     mos_utilities.h
//! \brief    Common OS service across different platform
//! \details  Common OS service across different platform
//!
#ifndef __MOS_UTILITIES_H__
#define __MOS_UTILITIES_H__

#include "mos_defs.h"
#include "mos_util_user_feature_keys.h"
#include "mos_resource_defs.h"
#include "mos_util_debug.h"
#include "mos_os_trace_event.h"

#ifdef __cplusplus
#include <memory>
#endif

#ifndef __MOS_USER_FEATURE_WA_
#define  __MOS_USER_FEATURE_WA_
#endif
//------------------------------------------------------------------------------
// SECTION: Media User Feature Control
//
// ABSTRACT: Is an abstraction to read and write system level settings relating
//      to GEN media driver.
//------------------------------------------------------------------------------

//!
//! \brief ASSERT when failing to read user feature key or default user feature key value,
//!        according to MOS_UserFeature_ReadValue_ID.
//!
#define MOS_USER_FEATURE_INVALID_KEY_ASSERT(_expr)               \
    if((_expr) == MOS_STATUS_NULL_POINTER)                       \
    {                                                            \
        MOS_OS_ASSERT(false);                                    \
    }                                                            \

//!
//! \brief User Feature Type maximum and minimum data size
//!
#define MOS_USER_CONTROL_MIN_DATA_SIZE         128
#define MOS_USER_CONTROL_MAX_DATA_SIZE         2048
#define MOS_USER_MAX_STRING_COUNT              128

#define MOS_USER_FEATURE_MAX_UINT32_STR_VALUE  "4294967295"

//! MOS User Feature
#define  __NULL_USER_FEATURE_VALUE_WRITE_DATA__     {__MOS_USER_FEATURE_KEY_INVALID_ID,{{0},0}}
#ifdef  __MOS_USER_FEATURE_WA_
#define  __NULL_USER_FEATURE_VALUE__ { __MOS_USER_FEATURE_KEY_INVALID_ID, nullptr, nullptr, nullptr, nullptr, MOS_USER_FEATURE_TYPE_INVALID, MOS_USER_FEATURE_VALUE_TYPE_INVALID, nullptr, nullptr, false, 0, nullptr, nullptr, MOS_USER_FEATURE_EFFECT_ALWAYS, {0} , {{0},0}}
#define  MOS_DECLARE_UF_KEY(Id,ValueName,Readpath,Writepath,Group,Type,ValueType,DefaultValue,Description)  { Id, ValueName, Group, Readpath, Writepath, Type, ValueType , DefaultValue, Description, false, 1, nullptr, nullptr, MOS_USER_FEATURE_EFFECT_ALWAYS, {0}, {{0},0}}
// The MOS_DECLARE_UF_KEY_DBGONLY macro will make the user feature key read only return default value in release build without accessing user setting
// it is an alternative way for removing the key defintion entirely in release driver, and still provide an unified place for default values of the
// user feature key read request that is needed for release driver
#define  MOS_DECLARE_UF_KEY_DBGONLY(Id,ValueName,Readpath,Writepath,Group,Type,ValueType,DefaultValue,Description)  { Id, ValueName, Group, Readpath, Writepath, Type, ValueType , DefaultValue, Description, false, 1, nullptr, nullptr, MOS_USER_FEATURE_EFFECT_DEBUGONLY, {0}, {{0},0}}
#define  MOS_DECLARE_UF_KEY_SETFN(Id,ValueName,Readpath,Writepath,Group,Type,ValueType,pfn,Description)  { Id, ValueName, Group, Readpath, Writepath, Type, ValueType, "0", Description, false, 1, nullptr, pfn, MOS_USER_FEATURE_EFFECT_ALWAYS, {0}, {{0}}}
#else
#define  __NULL_USER_FEATURE_VALUE__ { __MOS_USER_FEATURE_KEY_INVALID_ID, nullptr, nullptr, nullptr, nullptr, MOS_USER_FEATURE_TYPE_INVALID, MOS_USER_FEATURE_VALUE_TYPE_INVALID, nullptr, nullptr, false, 0, nullptr nullptr, MOS_USER_FEATURE_EFFECT_ALWAYS, {{0},0}}
#define  MOS_DECLARE_UF_KEY(Id,ValueName,Readpath,Writepath,Group,Type,ValueType,DefaultValue,Description)  { Id, ValueName, Group, Readpath, Writepath, Type, ValueType , DefaultValue, Description, false, 1, nullptr ,nullptr, MOS_USER_FEATURE_EFFECT_ALWAYS, {{0},0}}
#define  MOS_DECLARE_UF_KEY_DBGONLY(Id,ValueName,Readpath,Writepath,Group,Type,ValueType,DefaultValue,Description)  { Id, ValueName, Group, Readpath, Writepath, Type, ValueType , DefaultValue, Description, false, 1, nullptr ,nullptr, MOS_USER_FEATURE_EFFECT_DEBUGONLY, {{0},0}}
#endif
#ifndef MAX_USER_FEATURE_FIELD_LENGTH
#define MAX_USER_FEATURE_FIELD_LENGTH            256
#endif

extern int32_t MosMemAllocCounter;
extern int32_t MosMemAllocCounterGfx;

//! Helper Macros for MEMNINJA debug messages
#define MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line)                             \
    MOS_OS_VERBOSEMESSAGE(                                                                              \
        "MemNinjaSysAlloc: MemNinjaCounter = %d, memPtr = 0x%x, size = %d, functionName = \"%s\", "     \
        "filename = \"%s\", line = %d/", MosMemAllocCounter, ptr, size, functionName, filename, line)

#define MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line)                                    \
    MOS_OS_VERBOSEMESSAGE(                                                                              \
       "MemNinjaSysFree: MemNinjaCounter = %d, memPtr = 0x%x, functionName = \"%s\", "                  \
       "filename = \"%s\", line = %d/", MosMemAllocCounter, ptr, functionName, filename, line)

#define MOS_MEMNINJA_GFX_ALLOC_MESSAGE(ptr, size, functionName, filename, line)                         \
    MOS_OS_VERBOSEMESSAGE(                                                                              \
        "MemNinjaGfxAlloc: MemNinjaCounterGfx = %d, memPtr = 0x%x, size = %d, functionName = \"%s\", "  \
        "filename = \"%s\", line = %d/", MosMemAllocCounterGfx, ptr, size, functionName, filename, line)

#define MOS_MEMNINJA_GFX_FREE_MESSAGE(ptr, functionName, filename, line)                                \
    MOS_OS_VERBOSEMESSAGE(                                                                              \
        "MemNinjaGfxFree: MemNinjaCounterGfx = %d, memPtr = 0x%x, functionName = \"%s\", "              \
        "filename = \"%s\", line = %d/", MosMemAllocCounterGfx, ptr, functionName, filename, line)

//!
//! \brief User Feature Value IDs
//!
typedef enum _MOS_USER_FEATURE_VALUE_ID
{
    __MOS_USER_FEATURE_KEY_INVALID_ID = 0,
    __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_TH_ID,
    __MEDIA_USER_FEATURE_VALUE_SOFT_RESET_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX_ID,
    __MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_OUTPUT_FILE,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_BUFFER_SIZE,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_TIMER_REG,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_1,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_2,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_3,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_4,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_5,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_6,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_7,
    __MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_8,
    __MEDIA_USER_FEATURE_VALUE_DISABLE_KMD_WATCHDOG_ID,
    __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MFE_MBENC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_RC_PANIC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_FORCE_YFYS_ID,
    __MEDIA_USER_FEATURE_VALUE_DECODE_LOCK_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_SUPPRESS_RECON_PIC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_ME_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_16xME_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_32xME_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_RATECONTROL_METHOD_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_TARGET_USAGE_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_FRAME_TRACKING_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_USED_VDBOX_NUM_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_COMPUTE_CONTEXT_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_ME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_16xME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_32xME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_MULTIPRED_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_INTRA_REFRESH_QP_THRESHOLD_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_FTQ_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_CAF_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_CAF_DISABLE_HD_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_MB_BRC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_P_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_B_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_BREF_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_ROUNDING_INTER_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_SKIP_BIAS_ADJUSTMENT_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_INTRA_SCALING_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_OLD_MODE_COST_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_FORCE_TO_SKIP_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_SLIDING_WINDOW_SIZE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_MB_SLICE_THRESHOLD_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_SLICE_THRESHOLD_TABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_TAIL_INSERTION_DELAY_COUNT_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_THRESHOLD_I_SLICE_SIZE_MINUS_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_THRESHOLD_P_SLICE_SIZE_MINUS_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_CRE_PREFETCH_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_TLB_PREFETCH_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_TLB_ALLOCATION_WA_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENC_SINGLE_PASS_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_0_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_1_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_2_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_FLATNESS_CHECK_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_SEARCH_WINDOW_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_ADAPTIVE_TRANSFORM_DECISION_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L0_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L1_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_FBR_BYPASS_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_STATIC_FRAME_DETECTION_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_COLOR_BIT_SUPPORT_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_GROUP_ID_SELECT_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_BRC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_MULTIREF_QP_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_BRC_SOFTWARE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_BRC_VAR_COMPU_BYPASS_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_BRC_SOFTWARE_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENABLE_CNL_AVC_ENCODE_ARB_WA_ID,
    __MEDIA_USER_FEATURE_VALUE_HUC_DEMO_KERNEL_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_16xME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_HUC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_CMD_INIT_HUC_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_SECURE_INPUT_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MODE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_32xME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_B_KERNEL_SPLIT,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_POWER_SAVING,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_8x8_INTRA_KERNEL_SPLIT,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_WP_SUPPORT_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_MEDIARESET_TEST_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MULTIPASS_BRC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MULTIPASS_BRC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_PATH_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ACQP_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_FORCE_PAK_PASS_NUM_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ROUNDING_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_PAKOBJCMD_STREAMOUT_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_LBCONLY_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_PARTIAL_FRAME_UPDATE_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_NUM_THREADS_PER_LCU_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MDF_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_DECODE_MMC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_DECODE_EXTENDED_MMC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_EXTENDED_MMC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSIBLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSMODE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID,
    __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_DEFAULT_STATE_ID,
    __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_REQUEST_STATE_ID,
    __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_RESOLUTION_THRESHOLD_ID,
    __MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_TARGET_USAGE_THRESHOLD_ID,
    __MEDIA_USER_FEATURE_VALUE_DYNAMIC_SLICE_SHUTDOWN_ID,
    __MEDIA_USER_FEATURE_VALUE_MPEG2_SLICE_STATE_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MPEG2_ENCODE_BRC_DISTORTION_BUFFER_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_NUMBER_OF_CODEC_DEVICES_ON_VDBOX1_ID,
    __MEDIA_USER_FEATURE_VALUE_NUMBER_OF_CODEC_DEVICES_ON_VDBOX2_ID,
    __MEDIA_USER_FEATURE_VALUE_VDI_MODE_ID,
    __MEDIA_USER_FEATURE_VALUE_MEDIA_WALKER_MODE_ID,
    __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP8_HW_SCOREBOARD_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_ME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_16xME_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_REPAK_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_MULTIPASS_BRC_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE_ID,
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#if (_DEBUG || _RELEASE_INTERNAL)
    __MEDIA_USER_FEATURE_VALUE_GROUP_ID_ID,
    __MEDIA_USER_FEATURE_VALUE_MEDIA_PREEMPTION_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_VFE_MAX_THREADS_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_VFE_MAX_THREADS_SCALING_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_FTQ_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_AVC_CAF_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_MODE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG2_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG3_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG1_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG4_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_L3_LRA_1_REG1_OVERRIDE_ID,
    __MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDBOX_ID_USED,
    __MEDIA_USER_FEATURE_VALUE_VDENC_IN_USE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_CSC_METHOD_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_RAW_TILE_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_RAW_FORMAT_ID,
    __MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_SURF_BTI_ID,
    __MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_INTRAROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_DEBLOCKINGFILTERROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_BSDMPCROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MPRROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VDENCROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_CODEC_SIM_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_BREAK_IN_CODECHAL_CREATE_ID,
    __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_STREAM_OUT_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY_ID,
    __MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_CFG_GENERATION_ID,

#endif // (_DEBUG || _RELEASE_INTERNAL)
    __MEDIA_USER_FEATURE_VALUE_STATUS_REPORTING_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_POSITION_ID,
    __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_PARAMETERS_ID,
#if MOS_MESSAGES_ENABLED
    __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_HW_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_HW_ID,
//!
//! \brief 63____________________________________________________________________________3__________0
//!         |                                                                            |   All    |
//!         |                    Reserved                                                |Asrt|level|
//!         |____________________________________________________________________________|__________|
//!
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_HW_TAG_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG_ID,
    __MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG_ID,
    __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM_ID,
    __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG_ID,
#endif // MOS_MESSAGES_ENABLED
    __MEDIA_USER_FEATURE_VALUE_HEVC_SF_2_DMA_SUBMITS_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVCDATROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVCDFROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_HEVCSAOROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_HVDROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_DATROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_VP9_DFROWSTORECACHE_DISABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_DDI_DUMP_DIRECTORY_ID,
    __MEDIA_USER_FEATURE_VALUE_ENCODE_DDI_DUMP_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MDF_ETW_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MDF_LOG_LEVEL_ID,
    __MEDIA_USER_FEATURE_VALUE_MDF_UMD_ULT_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MDF_CURBE_DUMP_ENABLE_ID,
    __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_DUMP_ENABLE_ID,
    __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC_ID,
    __VPHAL_RNDR_SSD_CONTROL_ID,
    __VPHAL_RNDR_SCOREBOARD_CONTROL_ID,
    __VPHAL_RNDR_CMFC_CONTROL_ID,
#if (_DEBUG || _RELEASE_INTERNAL)
    __VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME_ID,
    __VPHAL_DBG_SURF_DUMP_LOCATION_KEY_NAME_ID,
    __VPHAL_DBG_SURF_DUMP_START_FRAME_KEY_NAME_ID,
    __VPHAL_DBG_SURF_DUMP_END_FRAME_KEY_NAME_ID,
    __VPHAL_DBG_SURF_DUMPER_RESOURCE_LOCK_ID,
    __VPHAL_DBG_STATE_DUMP_OUTFILE_KEY_NAME_ID,
    __VPHAL_DBG_STATE_DUMP_LOCATION_KEY_NAME_ID,
    __VPHAL_DBG_STATE_DUMP_START_FRAME_KEY_NAME_ID,
    __VPHAL_DBG_STATE_DUMP_END_FRAME_KEY_NAME_ID,
    __VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME_ID,
    __VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME_ID,
    __VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME_ID,
#endif
    __VPHAL_SET_SINGLE_SLICE_VEBOX_ID,
    __VPHAL_BYPASS_COMPOSITION_ID,
    __VPHAL_VEBOX_DISABLE_SFC_ID,
    __VPHAL_ENABLE_MMC_ID,
    __VPHAL_ENABLE_VEBOX_MMC_DECOMPRESS_ID,
    __VPHAL_VEBOX_DISABLE_TEMPORAL_DENOISE_FILTER_ID,
#if (_DEBUG || _RELEASE_INTERNAL)
    __VPHAL_COMP_8TAP_ADAPTIVE_ENABLE_ID,
#endif
#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL)
    __VPHAL_RNDR_VEBOX_MODE_0_ID,
    __VPHAL_RNDR_VEBOX_MODE_0_TO_2_ID,
    __VPHAL_RNDR_VEBOX_MODE_2_ID,
    __VPHAL_RNDR_VEBOX_MODE_2_TO_0_ID,
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
        __VPHAL_ENABLE_COMPUTE_CONTEXT_ID,
#endif
    __MOS_USER_FEATURE_KEY_VP_CAPS_FF_OVERRIDE_ID,
    __MOS_USER_FEATURE_KEY_XML_AUTOGEN_ID,
    __MOS_USER_FEATURE_KEY_XML_FILEPATH_ID,
    __MOS_USER_FEATURE_KEY_XML_DUMP_GROUPS_ID,
    __MOS_USER_FEATURE_KEY_MAX_ID,
} MOS_USER_FEATURE_VALUE_ID;

//!
//! \brief User Feature Type
//!
typedef enum
{
    MOS_USER_FEATURE_TYPE_INVALID,
    MOS_USER_FEATURE_TYPE_USER,
    MOS_USER_FEATURE_TYPE_SYSTEM,
} MOS_USER_FEATURE_TYPE, *PMOS_USER_FEATURE_TYPE;

//!
//! \brief User Feature Value type
//!
typedef enum
{
    MOS_USER_FEATURE_VALUE_TYPE_INVALID,
    MOS_USER_FEATURE_VALUE_TYPE_BINARY,
    MOS_USER_FEATURE_VALUE_TYPE_BOOL,
    MOS_USER_FEATURE_VALUE_TYPE_INT32,
    MOS_USER_FEATURE_VALUE_TYPE_INT64,
    MOS_USER_FEATURE_VALUE_TYPE_UINT32,
    MOS_USER_FEATURE_VALUE_TYPE_UINT64,
    MOS_USER_FEATURE_VALUE_TYPE_FLOAT,
    MOS_USER_FEATURE_VALUE_TYPE_STRING,
    MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING,
} MOS_USER_FEATURE_VALUE_TYPE, *PMOS_USER_FEATURE_VALUE_TYPE;

//!
//! \brief User Feature Notification type
//!
typedef enum
{
    MOS_USER_FEATURE_NOTIFY_TYPE_INVALID,
    MOS_USER_FEATURE_NOTIFY_TYPE_VALUE_CHANGE,
} MOS_USER_FEATURE_NOTIFY_TYPE, *PMOS_USER_FEATURE_NOTIFY_TYPE;

//!
//! \brief User Feature Data Operation type
//!         NONE_CUSTOM_DEFAULT_VALUE :     None Custom Default Value for Input Data
//!         CUSTOM_DEFAULT_VALUE_TYPE :     With Custom Default Value for Input Data
//!
typedef enum
{
    MOS_USER_FEATURE_VALUE_DATA_FLAG_NONE_CUSTOM_DEFAULT_VALUE_TYPE = 0,
    MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE,
} MOS_USER_FEATURE_VALUE_DATA_FLAG_TYPE, *PMOS_USER_FEATURE_VALUE_DATA_FLAG_TYPE;

//!
//! \brief User Feature Key Effective Range type
//!         EFFECT_ALWALYS   :   Effective on all driver builds
//!         EFFECT_DEBUGONLY :   Effective on release-internal and debug driver only
//!
typedef enum
{
    MOS_USER_FEATURE_EFFECT_ALWAYS = 0,
    MOS_USER_FEATURE_EFFECT_DEBUGONLY,
} MOS_USER_FEATURE_EFFECTIVE_TYPE, *PMOS_USER_FEATURE_EFFECTIVE_TYPE;

//!
//! \brief User Feature String Data
//!
typedef struct
{
    char    *pStringData;
    uint32_t uMaxSize;
    uint32_t uSize;
} MOS_USER_FEATURE_VALUE_STRING, *PMOS_USER_FEATURE_VALUE_STRING;

//!
//! \brief User Feature Multi String Data
//!
typedef struct
{
    char                              *pMultStringData;
    uint32_t                          uMaxSize;
    uint32_t                          uSize;
    PMOS_USER_FEATURE_VALUE_STRING    pStrings;
    uint32_t                          uCount;
} MOS_USER_FEATURE_VALUE_MULTI_STRING, *PMOS_USER_FEATURE_VALUE_MULTI_STRING;

//!
//! \brief User Feature Binary Data
//!
typedef struct
{
    uint8_t *pBinaryData;
    uint32_t uMaxSize;
    uint32_t uSize;
} MOS_USER_FEATURE_VALUE_BINARY, *PMOS_USER_FEATURE_VALUE_BINARY;

//!
//! \brief      User Feature Value Data
//! \details    union :         to store the user feature value
//!             i32DataFlag :   the input data valye type
//!                             refer to MOS_USER_FEATURE_VALUE_DATA_FLAG_TYPE
//!
//!
typedef struct _MOS_USER_FEATURE_VALUE_DATA
{
    union
    {
        int32_t                                 bData;
        uint32_t                                u32Data;
        uint64_t                                u64Data;
        int32_t                                 i32Data;
        int64_t                                 i64Data;
        float                                   fData;
        MOS_USER_FEATURE_VALUE_STRING           StringData;
        MOS_USER_FEATURE_VALUE_MULTI_STRING     MultiStringData;
        MOS_USER_FEATURE_VALUE_BINARY           BinaryData;
    };
    int32_t                                     i32DataFlag;
} MOS_USER_FEATURE_VALUE_DATA, *PMOS_USER_FEATURE_VALUE_DATA;

//!
//! \brief User Feature Value Information
//!
typedef struct _MOS_USER_FEATURE_VALUE_WRITE_DATA
{
    uint32_t                            ValueID;
    MOS_USER_FEATURE_VALUE_DATA         Value;
} MOS_USER_FEATURE_VALUE_WRITE_DATA, *PMOS_USER_FEATURE_VALUE_WRITE_DATA;

//!
//! \brief User Feature Value Information
//!
typedef struct _MOS_USER_FEATURE_VALUE_INFO
{
    char    *pcName;  //store name for the bitmask/enum values
    uint32_t Value;
} MOS_USER_FEATURE_VALUE_INFO, *PMOS_USER_FEATURE_VALUE_INFO;

//!
//! \brief User Feature Data
//!
typedef struct
{
    uint32_t                                            ValueID;
    const char                                          *pValueName;
    const char                                          *pcGroup;             //!< User feature key group - eg: MediaSolo, MOS, Codec
    const char                                          *pcPath;              //!< User feature Key Read Path
    const char                                          *pcWritePath;         //!< User feature Key Write Path
    MOS_USER_FEATURE_TYPE                               Type;                //!< User feature Key User Feature type - eg: System, User
    MOS_USER_FEATURE_VALUE_TYPE                         ValueType;           //!< User feature key type - eg: bool,dword
    const char                                          *DefaultValue;        //!< User feature key value
    const char                                          *pcDescription;       //!< User feature key description
    int32_t                                             bExists;             //<! Set if the user feature key is defined in the user feature key manager
    uint32_t                                            uiNumOfValues;       //<! Number of valid user feature key values. Useful for user feature keys of type bitmask and enum
    PMOS_USER_FEATURE_VALUE_INFO                        pValueInfo;          //<! Store information of all valid enum/bit mask values and names
    MOS_STATUS (*pfnSetDefaultValueData)(PMOS_USER_FEATURE_VALUE_DATA pValueData);
    MOS_USER_FEATURE_EFFECTIVE_TYPE                     EffctiveRange;       //<! User feature key effect range, eg: Always effective / debug driver only
    // Temp WA for old user feature read/write
#ifdef  __MOS_USER_FEATURE_WA_
    union
    {
        int32_t                                 bData;
        uint32_t                                u32Data;
        uint64_t                                u64Data;
        int32_t                                 i32Data;
        int64_t                                 i64Data;
        float                                   fData;
        MOS_USER_FEATURE_VALUE_STRING           StringData;
        MOS_USER_FEATURE_VALUE_MULTI_STRING     MultiStringData;
        MOS_USER_FEATURE_VALUE_BINARY           BinaryData;
    };
#endif
    MOS_USER_FEATURE_VALUE_DATA                         Value;               //!< User feature key value
} MOS_USER_FEATURE_VALUE, *PMOS_USER_FEATURE_VALUE;

//!
//! \brief User Feature Value Information
//!
typedef struct
{
    PMOS_USER_FEATURE_VALUE pUserFeatureValue;
} MOS_USER_FEATURE_VALUE_MAP, *PMOS_USER_FEATURE_VALUE_MAP;

//!
//! \brief User Feature Notification Data
//!
typedef struct
{
    MOS_USER_FEATURE_TYPE             Type;                                 //!< User Feature Type
    char                              *pPath;                                //!< User Feature Path
    MOS_USER_FEATURE_NOTIFY_TYPE      NotifyType;                           //!< Notification Type
    int32_t                           bTriggered;                           //!< Notification is triggered or not
    void                              *pHandle;                              //!< OS Specific Handle
} MOS_USER_FEATURE_NOTIFY_DATA, *PMOS_USER_FEATURE_NOTIFY_DATA;

//!
//! \brief User Feature Interface
//!
typedef struct
{
    MOS_USER_FEATURE_TYPE             Type;                                 //!< User Feature Type
    const char                        *pPath;                                //!< User Feature Path
    PMOS_USER_FEATURE_VALUE           pValues;                              //!< Array of User Feature Values
    uint32_t                          uiNumValues;                          //!< Number of User Feature Values
} MOS_USER_FEATURE, *PMOS_USER_FEATURE;

//!
//! \brief OS User Feature Interface
//!
typedef struct _MOS_USER_FEATURE_INTERFACE *PMOS_USER_FEATURE_INTERFACE;
typedef struct _MOS_USER_FEATURE_INTERFACE
{
    void                                        *pOsInterface;                       //!< Pointer to OS Interface
    int32_t                                     bIsNotificationSupported;           //!< Whether Notification feature is supported
    const MOS_USER_FEATURE                      *pUserFeatureInit;                  //!< Initializer for Os User Feature structure

    MOS_STATUS (* pfnReadValue) (
        PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
        PMOS_USER_FEATURE                       pUserFeature,
        const char                              *pValueName,
        MOS_USER_FEATURE_VALUE_TYPE             ValueType);

    MOS_STATUS (* pfnEnableNotification) (
        PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
        PMOS_USER_FEATURE_NOTIFY_DATA           pNotification);

    MOS_STATUS (* pfnDisableNotification) (
        PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
        PMOS_USER_FEATURE_NOTIFY_DATA           pNotification);

    MOS_STATUS (* pfnParsePath) (
        PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
        char * const                            pInputPath,
        PMOS_USER_FEATURE_TYPE                  pUserFeatureType,
        char                                    **ppSubPath);

} MOS_USER_FEATURE_INTERFACE;

//!
//! \brief User Feature Notification Data Common
//!
typedef struct
{
    void        *UFKey;                                 //!< Handle to User Feature Key
    HANDLE      hEvent;                                //!< Handle to User Feature Key Event
    PTP_WAIT    hWaitEvent;                            //!< Handle to User Feature Key Wait Event
} MOS_USER_FEATURE_NOTIFY_DATA_COMMON, *PMOS_USER_FEATURE_NOTIFY_DATA_COMMON;

#ifdef __cplusplus
//template<class _Ty, class... _Types> inline
//std::shared_ptr<_Ty> MOS_MakeShared(_Types&&... _Args)
//{
//    try
//    {
//        return std::make_shared<_Ty>(std::forward<_Types>(_Args)...);
//    }
//    catch (const std::bad_alloc&)
//    {
//        return nullptr;
//    }
//}

#if MOS_MESSAGES_ENABLED
template<class _Ty, class... _Types>
_Ty* MOS_NewUtil(const char *functionName,
    const char *filename,
    int32_t line, _Types&&... _Args)
#else
template<class _Ty, class... _Types>
_Ty* MOS_NewUtil(_Types&&... _Args)
#endif
{
        _Ty* ptr = new (std::nothrow) _Ty(std::forward<_Types>(_Args)...);
        if (ptr != nullptr)
        {
            MosMemAllocCounter++;
            MOS_MEMNINJA_ALLOC_MESSAGE(ptr, sizeof(_Ty), functionName, filename, line);
        }
        return ptr;
}

#if MOS_MESSAGES_ENABLED
template<class _Ty, class... _Types>
_Ty* MOS_NewArrayUtil(const char *functionName,
    const char *filename,
    int32_t line, int32_t numElements)
#else
template<class _Ty, class... _Types>
_Ty* MOS_NewArrayUtil(int32_t numElements)
#endif
{
        _Ty* ptr = new (std::nothrow) _Ty[numElements]();
        if (ptr != nullptr)
        {
            MosMemAllocCounter++;
            MOS_MEMNINJA_ALLOC_MESSAGE(ptr, numElements*sizeof(_Ty), functionName, filename, line);
        }
        return ptr;
}

#if MOS_MESSAGES_ENABLED
#define MOS_NewArray(classType, numElements) MOS_NewArrayUtil<classType>(__FUNCTION__, __FILE__, __LINE__, numElements)
#define MOS_New(classType, ...) MOS_NewUtil<classType>(__FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define MOS_NewArray(classType, numElements) MOS_NewArrayUtil<classType>(numElements)
#define MOS_New(classType, ...) MOS_NewUtil<classType>(__VA_ARGS__)
#endif

#if MOS_MESSAGES_ENABLED
template<class _Ty> inline
void MOS_DeleteUtil(
    const char *functionName,
    const char *filename,
    int32_t     line,
    _Ty&        ptr)
#else
template<class _Ty> inline
void MOS_DeleteUtil(_Ty& ptr)
#endif
{
    if (ptr != nullptr)
    {
        MosMemAllocCounter--;
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);
        delete(ptr);
        ptr = nullptr;
    }
}

#if MOS_MESSAGES_ENABLED
template<class _Ty> inline
void MOS_DeleteArrayUtil(
    const char *functionName,
    const char *filename,
    int32_t     line,
    _Ty&        ptr)
#else
template <class _Ty> inline
void MOS_DeleteArrayUtil(_Ty& ptr)
#endif
{
    if (ptr != nullptr)
    {
        MosMemAllocCounter--;

        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);

        delete[](ptr);
        ptr = nullptr;
    }
}

#if MOS_MESSAGES_ENABLED
#define MOS_DeleteArray(ptr) MOS_DeleteArrayUtil(__FUNCTION__, __FILE__, __LINE__, ptr)
#define MOS_Delete(ptr) MOS_DeleteUtil(__FUNCTION__, __FILE__, __LINE__, ptr)
#else
#define MOS_DeleteArray(ptr) MOS_DeleteArrayUtil(ptr)
#define MOS_Delete(ptr) MOS_DeleteUtil(ptr)
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \brief    Init Function for MOS utilities
//! \details  Initial MOS utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_utilities_init();

//!
//! \brief    Close Function for MOS utilities
//! \details  close/remove MOS utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_utilities_close();

//!
//! \brief    Init Function for MOS OS specific utilities
//! \details  Initial MOS OS specific utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_OS_Utilities_Init();

//!
//! \brief    Close Function for MOS OS utilities
//! \details  close/remove MOS OS utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_OS_Utilities_Close();

//------------------------------------------------------------------------------
//  Allocate, free and set a memory region
//------------------------------------------------------------------------------
//!
//! \brief    Allocates aligned memory and performs error checking
//! \details  Wrapper for aligned_malloc(). Performs error checking.
//!           It increases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] size
//!           Size of memorry to be allocated
//! \param    [in] alignment
//!           alignment
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void *MOS_AlignedAllocMemoryUtils(
    size_t     size,
    size_t     alignment,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_AlignedAllocMemory(size, alignment) \
   MOS_AlignedAllocMemoryUtils(size, alignment, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

void  *MOS_AlignedAllocMemory(
    size_t  size,
    size_t  alignment);

#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Wrapper for aligned_free(). Performs error checking.
//! \details  Wrapper for aligned_free() - Free a block of memory that was allocated by MOS_AlignedAllocMemory.
//!             Performs error checking.
//!           It decreases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MOS_AlignedFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line);

#define MOS_AlignedFreeMemory(ptr) \
    MOS_AlignedFreeMemoryUtils(ptr, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

void MOS_AlignedFreeMemory(void  *ptr);

#endif // MOS_MESSAGES_ENABLED
//!
//! \brief    Allocates memory and performs error checking
//! \details  Wrapper for malloc(). Performs error checking.
//!           It increases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] size
//!           Size of memorry to be allocated
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void  *MOS_AllocMemoryUtils(
    size_t     size,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_AllocMemory(size) \
    MOS_AllocMemoryUtils(size, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

void  *MOS_AllocMemory(
    size_t  size);

#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Allocates and fills memory with 0
//! \details  Wrapper for malloc(). Performs error checking,
//!           and fills the allocated memory with 0.
//!           It increases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] size
//!           Size of memorry to be allocated
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void  *MOS_AllocAndZeroMemoryUtils(
    size_t     size,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_AllocAndZeroMemory(size) \
    MOS_AllocAndZeroMemoryUtils(size, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED
void  *MOS_AllocAndZeroMemory(
    size_t                   size);
#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Reallocate memory
//! \details  Wrapper for realloc(). Performs error checking.
//!           It modifies memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] ptr
//!           Pointer to be reallocated
//! \param    [in] new_size
//!           Size of memory to be allocated
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void *MOS_ReallocMemoryUtils(
    void       *ptr,
    size_t     newSize,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_ReallocMemory(ptr, newSize) \
    MOS_ReallocMemoryUtils(ptr, newSize, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED
void *MOS_ReallocMemory(
    void       *ptr,
    size_t     newSize);
#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Wrapper for free(). Performs error checking.
//! \details  Wrapper for free(). Performs error checking.
//!           It decreases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MOS_FreeMemoryUtils(
    void       *ptr,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_FreeMemory(ptr) \
    MOS_FreeMemoryUtils(ptr, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED
void MOS_FreeMemory(
    void            *ptr);
#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Wrapper for MOS_FreeMemory().
//! \details  Wrapper for MOS_FreeMemory().  Calls MOS_FreeMemory() and then sets ptr to NULL.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#define MOS_FreeMemAndSetNull(ptr)  \
do{                                 \
    MOS_FreeMemory(ptr);            \
    ptr = nullptr;                     \
} while (0)

//!
//! \brief    Wrapper for MOS_FreeMemory().
//! \details  Wrapper for MOS_FreeMemory().  Calls MOS_FreeMemory() if ptr is not NULL.
//!           This wrapper can be called when ptr can be nullptr and therefore we don't want to hit the assert inside MOS_FreeMemory().
//!           It should not be used if ptr is expected to be non-NULL. In that case, we want to hit the assert.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#define MOS_SafeFreeMemory(ptr)               \
    if (ptr) MOS_FreeMemory(ptr);             \

//!
//! \brief    Wrapper to set a block of memory with zeros.
//! \details  Wrapper to set a block of memory with zeros.
//! \param    [in] pDestination
//!           A pointer to the starting address of the memory
//!           block to fill with zeros.
//! \param    [in] stLength
//!           Size of the memory block in bytes to be filled
//! \return   void
//!
void MOS_ZeroMemory(
    void            *pDestination,
    size_t          stLength);

//!
//! \brief    Wrapper to set a block of memory with a specified value.
//! \details  Wrapper to set a block of memory with a specified value.
//! \param    [in] pDestination
//!           A pointer to the starting address of the memory
//!           block to fill with specified value bFill
//! \param    [in] stLength
//!           Size of the memory block in bytes to be filled
//! \param    [in] bFill
//!           The byte value with which to fill the memory block
//! \return   void
//!
void MOS_FillMemory(
    void            *pDestination,
    size_t          stLength,
    uint8_t         bFill);

//------------------------------------------------------------------------------
//  File I/O Functions
//------------------------------------------------------------------------------
//!
//! \brief    Allocate a buffer and read contents from a file into this buffer
//! \details  Allocate a buffer and read contents from a file into this buffer
//! \param    [in] PpFilename
//!           ointer to the filename from which to read
//! \param    [out] lpNumberOfBytesRead,
//!           pointer to return the number of bytes read
//! \param    [out] ppReadBuffer
//!           Pointer to return the buffer pointer where
//!           the contents from the file are read to
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_ReadFileToPtr(
    const char         *pFilename,
    uint32_t           *lpNumberOfBytesRead,
    void               **ppReadBuffer);

//!
//! \brief    Writes contents of buffer into a file
//! \details  Writes contents of buffer into a file
//! \param    [in] pFilename
//!           Pointer to the filename to write the contents to
//! \param    [in] lpBuffer
//!           Pointer to the buffer whose contents will be written to the file
//! \param    [in] writeSize
//!           Number of bytes to write to the file
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_WriteFileFromPtr(
    const char              *pFilename,
    void                    *lpBuffer,
    uint32_t                writeSize);

//!
//! \brief    Retrieves the size of the specified File.
//! \details  Retrieves the size of the specified File.
//! \param    [in] hFile
//!           Handle to the File.
//! \param    [out] lpFileSizeLow
//!           Pointer to a variable where filesize is returned
//! \param    lpFileSizeHigh
//!           Reserved for now. Used to return higher uint32_t for
//!           filesizes more than 32 bit
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GetFileSize(
    HANDLE             hFile,
    uint32_t           *lpFileSizeLow,
    uint32_t           *lpFileSizeHigh);

//!
//! \brief    Creates a directory
//! \details  Creates a directory
//! \param    [in] lpPathName
//!           Pointer to the path name
//! \return   MOS_STATUS
//!           Returns MOS_STATUS_SUCCESS if directory was created or was already exists,
//!           else MOS_STATUS_DIR_CREATE_FAILED
//!
MOS_STATUS MOS_CreateDirectory(
    char * const       lpPathName);

//!
//! \brief    Creates or opens a file/object
//! \details  Creates or opens a file/object
//!           The definitions of the mode flags for iOpenFlag are in OS's fcntl.h
//! \param    [out] pHandle
//!           Pointer to a variable that recieves the handle
//!           of the file or object oepned
//! \param    [in] lpFileName
//!           Pointer to the file name
//! \param    [in] iOpenFlag
//!           Flag specifying mode and other options for Creating
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_CreateFile(
    PHANDLE               pHandle,
    char * const          lpFileName,
    uint32_t              iOpenFlag);

//!
//! \brief    Read data from a file
//! \details  Read data from a file
//! \param    [in] hFile
//!           Handle to the file to be read
//! \param    [out] lpBuffer
//!           Pointer to the buffer where the data read is placed
//! \param    [in] bytesToRead
//!           The maximum number of bytes to be read
//! \param    [out] pbytesRead
//!           Pointer to a variable that receives the number of bytes read
//! \param    [in/out] lpOverlapped
//!           Not used currently, can be nullptr
//!           When the hFile parameter was opened with FILE_FLAG_OVERLAPPED
//!           It should point to a valid OVERLAPPED structure
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_ReadFile(
    HANDLE          hFile,
    void            *lpBuffer,
    uint32_t        bytesToRead,
    uint32_t        *pbytesRead,
    void            *lpOverlapped);

//!
//! \brief    Write data to a file
//! \details  Write data to a file
//! \param    [in] hFile
//!           Handle to the file to which data will be written
//! \param    [in] lpBuffer
//!           Pointer to the buffer from where the data is read
//! \param    [in] bytesToWrite
//!           The maximum number of bytes to be written
//! \param    [out] pbytesWritten
//!           Pointer to a variable that receives the number of bytes written
//! \param    [in/out] lpOverlapped
//!           Not used currently, can be nullptr
//!           When the hFile parameter was opened with FILE_FLAG_OVERLAPPED
//!           It should point to a valid OVERLAPPED structure
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_WriteFile(
    HANDLE           hFile,
    void             *lpBuffer,
    uint32_t         bytesToWrite,
    uint32_t         *pbytesWritten,
    void             *lpOverlapped);

//!
//! \brief    Moves the File pointer to the specified position
//! \details  Moves the File pointer to the specified position
//!           Specify dwMoveMethod as the same as fseek()
//! \param    [in] hFile
//!           Handle to the file
//! \param    [in] lDistanceToMove
//!           Specifies no. of bytes to move the pointer
//! \param    [in] lpDistanceToMoveHigh
//!           Pointer to the high order 32-bits of
//!           the signed 64-bit distance to move.
//! \param    [in] dwMoveMethod
//!           Starting point for the file pointer move
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SetFilePointer(
    HANDLE                hFile,
    int32_t               lDistanceToMove,
    int32_t               *lpDistanceToMoveHigh,
    int32_t               dwMoveMethod);

//!
//! \brief    Closes an open object handle
//! \details  Closes an open object handle.
//! \param    [in] hObject
//!           A valid handle to an open object.
//! \return   int32_t
//!           true if success else false
//!
int32_t MOS_CloseHandle(
    HANDLE           hObject);

//!
//! \brief    Appends at the end of File
//! \details  Appends at the end of File
//! \param    [in] pFilename
//!           Pointer to the filename to append the contents to
//! \param    [in] pData
//!           Pointer to the buffer whose contents will be appeneded to the file
//! \param    [in] dwSize
//!           Number of bytes to append to the file
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_AppendFileFromPtr(
    const char               *pFilename,
    void                     *pData,
    uint32_t                 dwSize);

//------------------------------------------------------------------------------
// User Feature Functions
//------------------------------------------------------------------------------
//!
//! \brief    Read Single Value from User Feature
//! \details  This is a unified funtion to read user feature key for all components.
//!           (Codec/VP/CP/CM)
//!           It is required to prepare all memories for buffers before calling this function.
//!           User can choose to use array variable or allocated memory for the buffer.
//!           If the buffer is allocated dynamically, it must be freed by user to avoid memory leak.
//!           ------------------------------------------------------------------------------------
//!           Usage example:
//!           a) Initiation:
//!           MOS_ZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
//!           UserFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
//!           UserFeature.pPath           = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
//!           UserFeature.pValues         = &UserFeatureValue;
//!           UserFeature.uiNumValues     = 1;
//!           b.1) For uint32_t type:
//!           UserFeatureValue.u32Data = 1;    //set the default value, must be initiated with one valid value.
//!           b.2) For String/Binary type:
//!           char cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           UserFeatureValue.StringData.pStringData = cStringData; // make sure the pointer is valid
//!           UserFeatureValue.StringData.uMaxSize    = MOS_USER_CONTROL_MAX_DATA_SIZE;
//!           UserFeatureValue.StringData.uSize       = 0;  //set the default value. 0 is empty buffer.
//!           b.3) For MultiString type:
//!           char cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           MOS_USER_FEATURE_VALUE_STRING strings[MAX_STRING_COUNT];
//!           UserFeatureValue.MultiStringData.pMultStringData = cStringData; // make sure the pointer is valid
//!           UserFeatureValue.MultiStringData.uMaxSize        = MOS_USER_CONTROL_MAX_DATA_SIZE;
//!           UserFeatureValue.MultiStringData.uSize           = 0;  //set the default value. 0 is empty buffer.
//!           UserFeatureValue.MultiStringData.pStrings        = strings; // make sure the pointer is valid
//!           UserFeatureValue.MultiStringData.uCount          = MAX_STRING_COUNT;
//!           c) Read user feature key:
//!           MOS_UserFeature_ReadValue();
//!           -------------------------------------------------------------------------------------
//!           Important note: The pointer pStringData/pMultStringData may be modified if the
//!           previous MOS_UserFeature_ReadValue() doesn't read a same user feature key type. So it's
//!           suggested to set the union members in UserFeatureValue every time before
//!           MOS_UserFeature_ReadValue() if you are not familiar with the details of this function.
//! \param    PMOS_USER_FEATURE_INTERFACE pOsUserFeatureInterface
//!           [in] Pointer to OS User Interface structure
//! \param    PMOS_USER_FEATURE pUserFeature
//!           [in/out] Pointer to User Feature Interface
//! \param    char  *pValueName
//!           [in] Pointer to the name of the user feature key value
//! \param    MOS_USER_FEATURE_VALUE_TYPE ValueType
//!           [in] User Feature Value type
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief    Generate a User Feature Keys XML file according to user feature keys table in MOS
//! \details  Generate a User Feature Keys XML files according to MOSUserFeatureDescFields
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_FUNC_EXPORT MOS_STATUS MOS_EXPORT_DECL DumpUserFeatureKeyDefinitionsMedia();

#endif

//!
//! \brief    Generate a User Feature Keys XML file according to user feature keys table in MOS
//! \details  Generate a User Feature Keys XML files according to MOSUserFeatureDescFields
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GenerateUserFeatureKeyXML();

//!
//! \brief    Get the User Feature Value from Table
//! \details  Get the related User Feature Value item according to Filter rules , and pass the item
//!            into return callback function
//! \param    [in] descTable
//!           The user feature key description table
//! \param    [in] numOfItems
//!           Number of user feature keys described in the table
//! \param    [in] maxId
//!           Max value ID in the table
//! \param    [out] keyValueMap
//!           Optional pointer to the value map where the table items will be linked to, could be nullptr
//! \param    [in] CallbackFunc
//!           Pointer to the Callback function, and pass the User Feature Value item as its parameter
//! \param    [in] pUserFeatureKeyFilter
//!           use the filter rule to select some User Feature Value item
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GetItemFromMOSUserFeatureDescField(
    MOS_USER_FEATURE_VALUE      *descTable,
    uint32_t                    numOfItems,
    uint32_t                    maxId,
    MOS_USER_FEATURE_VALUE_MAP  *keyValueMap,
    MOS_STATUS                  (*CallbackFunc)(MOS_USER_FEATURE_VALUE_MAP *, PMOS_USER_FEATURE_VALUE),
    PMOS_USER_FEATURE_VALUE     pUserFeatureKeyFilter);

//!
//! \brief    Set the User Feature Default Value
//! \details  Set the User Feature Default Value in the user feature key map
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in] pWriteValues
//!           Pointer to User Feature Write Data
//! \param    [in] uiNumOfValues
//!           Number of User Feature Write Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_SetDefaultValues(
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues);

//!
//! \brief    Link user feature key description table items to specified UserFeatureKeyTable
//! \details  Link user feature key description table items to specified UserFeatureKeyTable
//!           according to ID sequence and do some post processing such as malloc related memory
//! \param    [in] userValueDescTable
//!           The user feature key description table
//! \param    [in] numOfValues
//!           Number of user feature keys described in the table
//! \param    [in] maxId
//!           Max value ID in the table
//! \param    [out] keyValueMap
//!           optional pointer to the value map where the table items will be linked to, could be nullptr
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DeclareUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *userValueDescTable,
    uint32_t                   numOfValues,
    uint32_t                   maxId,
    MOS_USER_FEATURE_VALUE_MAP *keyValueMap);

//!
//! \brief    Link the MOSUserFeatureDescFields table items to gc_UserFeatureKeysMap
//! \details  Link the MOSUserFeatureDescFields table items to gc_UserFeatureKeysMap
//!           according to ID sequence and do some post processing such as malloc related memory
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DeclareUserFeatureKeysForAllDescFields();

//!
//!
//! \brief    Destroy the User Feature Value pointer according to the DescField Table
//! \details  Destroy the User Feature Value pointer according to the DescField Table
//!           destroy the user feature key value Map according to Declare Count
//! \param    [in] descTable
//!           The user feature key description table
//! \param    [in] numOfItems
//!           Number of user feature keys described in the table
//! \param    [in] maxId
//!           Max value ID in the table
//! \param    [out] keyValueMap
//!           optional pointer to the value map where the table items will be destroyed, could be nullptr
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DestroyUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId,
    MOS_USER_FEATURE_VALUE_MAP *keyValueMap);

//!
//!
//! \brief    Destroy the User Feature Value pointer according to the Global DescField Table
//! \details  Destroy the User Feature Value pointer according to the Global DescField Table
//!           destroy the gc_UserFeatureKeysMap according to Declare Count
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DestroyUserFeatureKeysForAllDescFields();

//!
//! \brief    Copy the VALUE_DATA from source to destination pointer
//! \details  Copy the VALUE_DATA from source to destination pointer
//! \param    [in] pSrcData
//!           Pointer to the Source Value Data
//! \param    [in] pDstData
//!           Pointer to the Destination Value Data
//! \param    [in] ValueType
//!           Value Type for the copy data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_CopyUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA pSrcData,
    PMOS_USER_FEATURE_VALUE_DATA pDstData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType
);

//!
//! \brief    Free the allocated memory for the related Value type
//! \details  Free the allocated memory for the related Value type
//! \param    [in] pData
//!           Pointer to the User Feature Value Data
//! \param    [in] ValueType
//!           related Value Type needed to be deallocated.
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DestroyUserFeatureData(
    PMOS_USER_FEATURE_VALUE_DATA pData,
    MOS_USER_FEATURE_VALUE_TYPE  ValueType);

#ifdef  __MOS_USER_FEATURE_WA_
MOS_STATUS MOS_UserFeature_ReadValue (
    PMOS_USER_FEATURE_INTERFACE       pOsUserFeatureInterface,
    PMOS_USER_FEATURE                 pUserFeature,
    const char                        *pValueName,
    MOS_USER_FEATURE_VALUE_TYPE       ValueType);
#endif

//!
//! \brief    Read Single Value from User Feature based on value of enum type in MOS_USER_FEATURE_VALUE_TYPE
//! \details  This is a unified funtion to read user feature key for all components.
//!           (Codec/VP/CP/CM)
//!           It is required to prepare all memories for buffers before calling this function.
//!           User can choose to use array variable or allocated memory for the buffer.
//!           If the buffer is allocated dynamically, it must be freed by user to avoid memory leak. 
//!           ------------------------------------------------------------------------------------
//!           Usage example: 
//!           a) Initiation:
//!           MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
//!           b.0) Don't need to input a default value if the default value in user feature key Desc Fields table item is good 
//!                for your case
//!           b.1) For uint32_t type:
//!           UserFeatureData.u32Data = 1;    // overwrite a custom default value 
//!           UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE; 
//!                                           // raise a flag to use this custom default value instead of 
//!                                              default value in user feature key Desc Fields table item
//!           b.2) For String/Binary type:
//!           char cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           UserFeatureData.StringData.pStringData = cStringData; // make sure the pointer is valid
//!           b.3) For MultiString type:
//!           char                          cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           MOS_USER_FEATURE_VALUE_STRING Strings[__MAX_MULTI_STRING_COUNT];
//!           UserFeatureData.MultiStringData.pMultStringData = cStringData; // make sure the pointer is valid
//!           for (ui = 0; ui < VPHAL_3P_MAX_LIB_PATH_COUNT; ui++)
//!           {
//!             Strings[ui].pStringData = (char *)MOS_AllocAndZeroMemory(MOS_USER_CONTROL_MAX_DATA_SIZE);
//!           }
//!           UserFeatureData.MultiStringData.pStrings = Strings;
//!           c) Read user feature key:
//!           MOS_UserFeature_ReadValue_ID();
//!           -------------------------------------------------------------------------------------
//!           Important note: The pointer pStringData/pMultStringData may be modified if the 
//!           previous MOS_UserFeature_ReadValue() doesn't read a same user feature key type. So it's 
//!           suggested to set the union members in UserFeatureValue every time before 
//!           MOS_UserFeature_ReadValue() if you are not familiar with the details of this function.
//!           If a new key is added, please make sure to declare a definition in corresponding
//!           user feature key Desc Fields tableby MOS_DECLARE_UF_KEY
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in] ValueID
//!           value of enum type in MOS_USER_FEATURE_VALUE_TYPE. declares the user feature key to be readed
//! \param    [in,out] pValueData
//!           Pointer to User Feature Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!           For pValueData return value:
//!                 MOS_STATUS_SUCCESS: pValueData is from User Feature Key
//!                 MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED: pValueData is from default value
//!                 MOS_STATUS_UNKNOWN: pValueData is from default value
//!                 MOS_STATUS_USER_FEATURE_KEY_READ_FAILED: pValueData is from default value
//!                 MOS_STATUS_NULL_POINTER: NO USER FEATURE KEY DEFINITION in corresponding user feature key Desc Field table, 
//!                                          No default value or User Feature Key value return
//! 
//!
#ifdef  __MOS_USER_FEATURE_WA_
MOS_STATUS MOS_UserFeature_ReadValue_ID(
#else
MOS_STATUS MOS_UserFeature_ReadValue(
#endif
    PMOS_USER_FEATURE_INTERFACE  pOsUserFeatureInterface,
    uint32_t                     ValueID,
    PMOS_USER_FEATURE_VALUE_DATA pValueData);

//!
//! \brief    Write Values to User Feature with specified ID
//! \details  Write Values to User Feature with specified ID
//!           The caller is responsible to allocate values / names
//!           and free them later if necessary
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in] pWriteValues
//!           Pointer to User Feature Data, and related User Feature Key ID (enum type in MOS_USER_FEATURE_VALUE_TYPE)
//! \param    [in] uiNumOfValues
//!           number of user feature keys to be written.
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
#ifdef  __MOS_USER_FEATURE_WA_
MOS_STATUS MOS_UserFeature_WriteValues_ID(
#else
MOS_STATUS MOS_UserFeature_WriteValues(
#endif
    PMOS_USER_FEATURE_INTERFACE              pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA       pWriteValues,
    uint32_t                                 uiNumOfValues);

//!
//! \brief    Lookup the user feature value name associated with the ID
//! \details  Lookup the user feature value name associated with the ID
//! \param    [in] ValueId
//!           The user feature value ID to be looked up
//! \return   const char*
//!           pointer to the char array holding the user feature value name
//!
const char* MOS_UserFeature_LookupValueName(
    uint32_t ValueID);

//!
//! \brief    Lookup the read path associated with the ID
//! \details  Lookup the read path associated with the ID
//! \param    [in] ValueId
//!           The user feature value ID to be looked up
//! \return   const char*
//!           pointer to the char array holding the read path
//!
const char* MOS_UserFeature_LookupReadPath(
    uint32_t ValueID);

//!
//! \brief    Lookup the write path associated with the ID
//! \details  Lookup the write path associated with the ID
//! \param    [in] ValueId
//!           The user feature value ID to be looked up
//! \return   const char*
//!           pointer to the char array holding the write path
//!
const char* MOS_UserFeature_LookupWritePath(
    uint32_t ValueID);

//!
//! \brief    Enable user feature change notification
//! \details  Enable user feature change notification
//!           Create notification data and register the wait event
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in/out] pNotification
//!           Pointer to User Feature Notification Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_EnableNotification(
    PMOS_USER_FEATURE_INTERFACE               pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA             pNotification);

//!
//! \brief    Disable user feature change notification
//! \details  Disable user feature change notification
//!           Unregister the wait event and frees notification data
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in/out] pNotification
//!           Pointer to User Feature Notification Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_DisableNotification(
    PMOS_USER_FEATURE_INTERFACE                pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA              pNotification);

//!
//! \brief    Parses the user feature path and gets type and sub path
//! \details  Parses the user feature path and gets type and sub path
//!           It verifies if the user feature path is valid,
//!           and check if it belongs to UFEXT or UFINT UFKEY.
//!           The identified type and subpath are set accordingly.
//! \param    [in] pOsUserFeatureInterface,
//!           Pointer to OS User Interface structure
//! \param    [in] pInputPath
//!           The input user feature path
//! \param    [out] pUserFeatureType
//!           Pointer to the variable to receive user feature type
//! \param    [out] ppSubPath
//!           Pointer to a variable that accepts the pointer to the subpath
//! \return   MOS_STATUS
//!           Returns MOS_STATUS_INVALID_PARAMETER if failed, else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_ParsePath(
    PMOS_USER_FEATURE_INTERFACE  pOsUserFeatureInterface,
    char * const                 pInputPath,
    PMOS_USER_FEATURE_TYPE       pUserFeatureType,
    char                         **ppSubPath);

//!
//! \brief    String concatenation with security checks.
//! \details  String concatenation with security checks.
//!           Append strSource to strDestination, with buffer size checking
//! \param    [in/out] strDestination
//!           Pointer to destination string
//! \param    [in] numberOfElements
//!           Size of the destination buffer
//! \param    [in] strSource
//!           Pointer to the source string
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureStrcat(
    char                *strDestination,
    size_t              numberOfElements,
    const char * const  strSource);

//!
//! \brief    Find string token with security checks.
//! \details  Find string token with security checks.
//!           Subsequent calls with nullptr in strToken and same contex to get
//!           remaining tokens
//! \param    [in/out] strToken
//!           String containing token or tokens
//!           Pass nullptr for this parameter in subsequent calls
//!           to MOS_SecureStrtok to find the remaining tokens
//! \param    [in] strDelimit
//!           Set of delimiter characters
//! \param    [in/out] contex
//!           Used to store position information between calls to MOS_SecureStrtok
//! \return   char *
//!           Returns tokens else nullptr
//!
char  *MOS_SecureStrtok(
    char                *strToken,
    const char          *strDelimit,
    char                **contex);

//!
//! \brief    String copy with security checks.
//! \details  String copy with security checks.
//!           Copy strSource to strDestination, with buffer size checking
//! \param    [out] strDestination
//!           Pointer to destination string
//! \param    [in] numberOfElements
//!           Size of the destination buffer
//! \param    [in] strSource
//!           Pointer to the source string
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureStrcpy(
    char                *strDestination,
    size_t              numberOfElements,
    const char * const  strSource);

//!
//! \brief    Memory copy with security checks.
//! \details  Memory copy with security checks.
//!           Copy pSource to pDestination, with buffer size checking
//! \param    [out] pDestination
//!           Pointer to destination buffer
//! \param    [in] dstLength
//!           Size of the destination buffer
//! \param    [in] pSource
//!           Pointer to the source buffer
//! \param    [in] srcLength
//!           Number of bytes to copy from source to destination
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureMemcpy(
    void                *pDestination,
    size_t              dstLength,
    const void          *pSource,
    size_t              srcLength);

//!
//! \brief    Open a file with security checks.
//! \details  Open a file with security checks.
//! \param    [out] ppFile
//!           Pointer to a variable that receives the file pointer.
//! \param    [in] filename
//!           Pointer to the file name string
//! \param    [in] mode
//!           Specifies open mode such as read, write etc
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureFileOpen(
    FILE       **ppFile,
    const char *filename,
    const char *mode);

//!
//! \brief    Write formatted data to a string with security checks.
//! \details  Write formatted data to a string with security checks.
//!           Optional arguments are passed in individually
//!           Buffer must have space for null character after copying length
//! \param    [out] buffer
//!           Pointer to a string to which formatted data is printed
//! \param    [in] bufSize
//!           Size of the buffer where the data is printed
//! \param    [in] length
//!           Number of characters to be printed
//! \param    [in] format
//!           Format string to be printed
//! \return   int32_t
//!           Returns the number of characters printed or -1 if an error occurs
//!
int32_t MOS_SecureStringPrint(
    char                     *buffer,
    size_t                   bufSize,
    size_t                   length,
    const  char * const      format,
                                 ...);

//!
//! \brief    Write formatted data to a string with security checks, va_list version
//! \details  Write formatted data to a string with security checks.
//!           Pointer to an optional arguments list is passed in
//!           Buffer must have space for null character after copying length
//! \param    [out] buffer
//!           Pointer to a string to which formatted data is printed
//! \param    [in] bufSize
//!           Size of the buffer where the data is printed
//! \param    [in] length
//!           Number of characters to be printed
//! \param    [in] format
//!           Format string to be printed
//! \param    [in] var_args
//!           Optional argument list
//! \return   int32_t
//!           Returns the number of characters printed or -1 if an error occurs
//!
MOS_STATUS MOS_SecureVStringPrint(
    char                      *buffer,
    size_t                    bufSize,
    size_t                    length,
    const char * const        format,
    va_list                   var_args);

//------------------------------------------------------------------------------
// Library, process and OS related functions
//------------------------------------------------------------------------------
//!
//! \brief    Maps the specified executable module into the address space of
//!           the calling process.
//! \details  Maps the specified executable module into the address space of
//!           the calling process.
//! \param    [in] lpLibFileName
//!           A valid handle to an open object.
//! \param    [out] phModule
//!           Pointer variable that accepts the module handle
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_LoadLibrary(
    const char * const lpLibFileName,
    PHMODULE           phModule);

//!
//! \brief    Free the loaded dynamic-link library
//! \details  Free the loaded dynamic-link library
//! \param    [in] hLibModule
//!           A handle to the loaded DLL module
//! \return   int32_t
//!           true if success else false
//!
int32_t MOS_FreeLibrary(HMODULE hLibModule);

//!
//! \brief    Retrieves the address of an exported function or variable from
//!           the specified dynamic-link library
//! \details  Retrieves the address of an exported function or variable from
//!           the specified dynamic-link library
//! \param    [in] hLibModule
//!           A handle to the loaded DLL module.
//!           The LoadLibrary function returns this handle.
//! \param    [in] lpProcName
//!           The function or variable name, or the function's ordinal value.
//! \return   void *
//!           If succeeds, the return value is the address of the exported
//!           function or variable. If fails, the return value is NULL.
//!           To get extended error information, call GetLastError.
//!
void  *MOS_GetProcAddress(
    HMODULE     hModule,
    const char  *lpProcName);

//!
//! \brief    Retrieves the current process id
//! \details  Retrieves the current process id
//! \return   int32_t
//!           Return the current process id
//!
int32_t MOS_GetPid();

//!
//! \brief    Retrieves the frequency of the high-resolution performance
//!           counter, if one exists.
//! \details  Retrieves the frequency of the high-resolution performance
//!           counter, if one exists.
//! \param    [out] pFrequency
//!           Pointer to a variable that receives the current
//!           performance-counter frequency, in counts per second.
//! \return   int32_t
//!           If the installed hardware supports a high-resolution performance
//!           counter, the return value is nonzero. If the function fails, the
//!           return value is zero.
//!
int32_t MOS_QueryPerformanceFrequency(
    uint64_t                       *pFrequency);

//!
//! \brief    Retrieves the current value of the high-resolution performance
//!           counter
//! \details  Retrieves the current value of the high-resolution performance
//!           counter
//! \param    [out] pPerformanceCount
//!           Pointer to a variable that receives the current
//!           performance-counter value, in counts.
//! \return   int32_t
//!           If the installed hardware supports a high-resolution performance
//!           counter, the return value is nonzero. If the function fails, the
//!           return value is zero. To get extended error information, call GetLastError.
//!
int32_t MOS_QueryPerformanceCounter(
    uint64_t                     *pPerformanceCount);

//!
//! \brief    Sleep for given duration in ms
//! \details  Sleep for given duration ms
//! \param    [in] mSec
//!           Sleep duration in ms
//! \return   void
//!
void MOS_Sleep(
    uint32_t   mSec);

//------------------------------------------------------------------------------
// Wrappers for OS Specific User Feature Functions Implementations
//------------------------------------------------------------------------------
//!
//! \brief    Opens the specified user feature key
//! \details  Opens the specified user feature key
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//! \param    [in] lpSubKey
//!           The name of the user feature subkey to be opened.
//! \param    [in] lOptions
//!           This parameter is reserved and must be zero.
//! \param    [in] samDesired
//!           Reserved, could be any REGSAM type value
//! \param    [out] phkResult
//!           A pointer to a variable that receives a handle to the opened key.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureOpenKey(
    void              *UFKey,
    const char        *lpSubKey,
    uint32_t          ulOptions,
    uint32_t          samDesired,
    void              **phkResult);

//!
//! \brief    Closes a handle to the specified user feature key
//! \details  Closes a handle to the specified user feature key
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureCloseKey(
    void               *UFKey);

//!
//! \brief    Retrieves the type and data for the specified user feature value
//! \details  Retrieves the type and data for the specified user feature value
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//! \param    [in] lpSubKey
//!           The name of the user feature key. This key must be a
//!           subkey of the key specified by the hkey parameter
//! \param    [in] lpValue
//!           The name of the user feature value
//! \param    [in] dwFlags
//!           The flags that restrict the data type of value to be queried
//! \param    [out] pdwType
//!           A pointer to a variable that receives a code indicating the type
//!           of data stored in the specified value.
//! \param    [out] pvData
//!           A pointer to a buffer that receives the value's data.
//! \param    [in/out] pcbData
//!           A pointer to a variable that specifies the size of the buffer
//!           pointed to by the pvData parameter, in bytes. When the function
//!           returns, this variable contains the size of the data copied to lpData.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureGetValue(
    void               *UFKey,
    const char         *lpSubKey,
    const char         *lpValue,
    uint32_t           dwFlags,
    uint32_t           *pdwType,
    void               *pvData,
    uint32_t           *pcbData);

//!
//! \brief    Retrieves the type and data for the specified value name
//!           associated with an open user feature key.
//! \details  Retrieves the type and data for the specified value name
//!           associated with an open user feature key.
//! \param    [in] UFKey
//!           A handle to an open user feature key
//! \param    [in] lpValueName
//!           The name of the user feature value
//! \param    [in] lpReserved
//!           This parameter is reserved and must be NULL.
//! \param    [out] lpType
//!           A pointer to a variable that receives a code indicating
//!           the type of data stored in the specified value.
//! \param    [out] lpData
//!           A pointer to a buffer that receives the value's data.
//! \param    [in/out] lpcbData
//!           A pointer to a variable that specifies the size
//!           of the buffer pointed to by the pvData parameter,
//!           in bytes. When the function returns, this variable
//!           contains the size of the data copied to lpData.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureQueryValueEx(
    void                    *UFKey,
    char                    *lpValueName,
    uint32_t                *lpReserved,
    uint32_t                *lpType,
    char                    *lpData,
    uint32_t                *lpcbData);

//!
//! \brief    Sets the data and type of a specified value under a user feature key
//! \details  Sets the data and type of a specified value under a user feature key
//! \param    [in] UFKey
//!           A handle to an open user feature key
//! \param    [in] lpValueName
//!           The name of the user feature value
//! \param    [in] Reserved
//!           This parameter is reserved and must be nullptr
//! \param    [in] dwType
//!           The type of data pointed to by the lpData parameter
//! \param    [in] lpData
//!           The data to be stored.
//! \param    [in] cbData
//!           The size of the information pointed to by the lpData parameter, in bytes.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureSetValueEx(
    void                 *UFKey,
    const char           *lpValueName,
    uint32_t             Reserved,
    uint32_t             dwType,
    uint8_t              *lpData,
    uint32_t             cbData);

//!
//! \brief    Notifies the caller about changes to the attributes or contents
//!           of a specified user feature key
//! \details  Notifies the caller about changes to the attributes or contents
//!           of a specified user feature key
//!           Used internally by MOS_UserFeature_EnableNotification()
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//!           The key must have been opened with the KEY_NOTIFY access right.
//! \param    [in] bWatchSubtree
//!           true including subkey changes; false for the key itself
//! \param    [in] hEvent
//!           A handle to an event to be signaled when key changes if is true
//! \param    [in] fAsynchronous
//!           true: Return immediately and signal the hEvent when key change
//!           false: Does not return until a change has occured
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureNotifyChangeKeyValue(
    void                           *UFKey,
    int32_t                        bWatchSubtree,
    HANDLE                         hEvent,
    int32_t                        fAsynchronous);

//!
//! \brief    Creates or opens a event object and returns a handle to the object
//! \details  Creates or opens a event object and returns a handle to the object
//! \param    [in] lpEventAttributes
//!           A pointer to a SECURITY_ATTRIBUTES structure.
//!           If lpEventAttributes is nullptr, the event handle cannot be inherited
//!           by child processes.
//! \param    [in] lpName
//!           The name of the event object.If lpName is nullptr, the event object is
//!           created without a name.
//! \param    [in] dwFlags
//!           Combines the following flags
//!           CREATE_EVENT_INITIAL_SET: Singal initial state or not
//!           CREATE_EVENT_MANUAL_RESET: Must be manually reset or not
//! \return   HANDLE
//!           If the function succeeds, the return value is a handle to the
//!           event object. If failed, returns NULL. To get extended error
//!           information, call GetLastError.
//!
HANDLE MOS_CreateEventEx(
    void                 *lpEventAttributes,
    char                 *lpName,
    uint32_t             dwFlags);

//!
//! \brief    Create a wait thread to wait on the object
//! \details  Create a wait thread to wait on the object
//!           Add this function to capatible WDK-9200 on vs2012.
//! \param    [out] phNewWaitObject
//!           A pointer to a variable that receives a wait handle on return.
//! \param    [in] hObject
//!           A handle to the object
//! \param    [in] Callback
//!           A pointer to the application-defined function of type
//!           WAITORTIMERCALLBACK to be executed when wait ends.
//! \param    [in] Context
//!           A single value that is passed to the callback function
//! \return   int32_t
//!           The return value is int32_t type. If the function succeeds,
//!           the return value is nonzero. If the function fails, the
//!           return value is zero.
//!
int32_t MOS_UserFeatureWaitForSingleObject(
    PTP_WAIT                         *phNewWaitObject,
    HANDLE                           hObject,
    void                             *Callback,
    void                             *Context);

//!
//! \brief    Cancels a registered wait operation issued by the
//!           RegisterWaitForSingleObject function
//! \details  Cancels a registered wait operation issued by the
//!           RegisterWaitForSingleObject function
//! \param    [in] hWaitHandle
//!           The wait handle. This handle is returned by the
//!           RegisterWaitForSingleObject function
//! \return   int32_t
//!           The return value is int32_t type. If the function succeeds,
//!           the return value is nonzero. If the function fails, the
//!           return value is zero.
//!
int32_t MOS_UnregisterWaitEx(
    PTP_WAIT                hWaitHandle);

//!
//! \brief    Get logical core number of current CPU
//! \details  Get logical core number of current CPU
//! \return   uint32_t
//!           If the function succeeds, the return value is the number of
//!           current CPU.
//!
uint32_t MOS_GetLogicalCoreNumber();

//!
//! \brief    Creates or opens a thread object and returns a handle to the object
//! \details  Creates or opens a thread object and returns a handle to the object
//! \param    [in] ThreadFunction
//!           A pointer to a thread function.
//! \param    [in] ThreadData
//!           A pointer to thread data.
//! \return   MOS_THREADHANDLE
//!           If the function succeeds, the return value is a handle to the
//!           thread object. If failed, returns NULL.
//!
MOS_THREADHANDLE MOS_CreateThread(
    void                        *ThreadFunction,
    void                        *ThreadData);

//!
//! \brief    Get thread id
//! \details  Get thread id
//! \param    [in] hThread
//!           A handle of thread object.
//! \return   uint32_t
//!           Return the current thread id
//!
uint32_t MOS_GetThreadId(
    MOS_THREADHANDLE            hThread);

//!
//! \brief    Retrieves the current thread id
//! \details  Retrieves the current thread id
//! \return   uint32_t
//!           Return the current thread id
//!
uint32_t MOS_GetCurrentThreadId();

//!
//! \brief    Wait for thread to terminate
//! \details  Wait for thread to terminate
//! \param    [in] hThread
//!           A handle of thread object.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_WaitThread(
    MOS_THREADHANDLE            hThread);

//!
//! \brief    Create mutex for context protection across threads
//! \details  Create mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    NONE
//! \return   PMOS_MUTEX
//!           Pointer of mutex
//!
PMOS_MUTEX MOS_CreateMutex();

//!
//! \brief    Destroy mutex for context protection across threads
//! \details  Destroy mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    [in] pMutex
//!           Pointer of mutex
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_DestroyMutex(PMOS_MUTEX pMutex);

//!
//! \brief    Lock mutex for context protection across threads
//! \details  Lock mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    [in] pMutex
//!           Pointer of mutex
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_LockMutex(PMOS_MUTEX pMutex);

//!
//! \brief    Unlock mutex for context protection across threads
//! \details  Unlock mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    [in] pMutex
//!           Pointer of mutex
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_UnlockMutex(PMOS_MUTEX pMutex);

//!
//! \brief    Creates or opens a semaphore object and returns a handle to the object
//! \details  Creates or opens a semaphore object and returns a handle to the object
//! \param    [in] uiInitialCount
//!           Initial count of semaphore usage.
//! \param    [in] uiMaximumCount
//!           Maximum count of semaphore usage.
//! \return   PMOS_SEMAPHORE
//!           If the function succeeds, the return value is a handle to the
//!           semaphore object. If failed, returns NULL. To get extended error
//!           information, call GetLastError.
//!
PMOS_SEMAPHORE MOS_CreateSemaphore(
    uint32_t                    uiInitialCount,
    uint32_t                    uiMaximumCount);

//!
//! \brief    Destroy a semaphore object
//! \details  Destroy a semaphore object
//! \param    [in] pSemaphore
//!           A handle of semaphore object.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_DestroySemaphore(
    PMOS_SEMAPHORE              pSemaphore);

//!
//! \brief    Wait a semaphore object
//! \details  Wait a semaphore object
//! \param    [in] pSemaphore
//!           A handle of semaphore object.
//! \param    [in] uiMilliseconds
//!           Wait time.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_WaitSemaphore(
    PMOS_SEMAPHORE              pSemaphore,
    uint32_t                    uiMilliseconds);

//!
//! \brief    Post a semaphore object
//! \details  Post a semaphore object
//! \param    [in] pSemaphore
//!           A handle of semaphore object.
//! \param    [in] uiPostCount
//!           semaphore post count.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_PostSemaphore(
    PMOS_SEMAPHORE              pSemaphore,
    uint32_t                    uiPostCount);

//!
//! \brief    Wait for single object of semaphore/mutex/thread and returns the result
//! \details  Wait for single object of semaphore/mutex/thread and returns the result
//! \param    [in] pObject
//!           Object handle.
//! \param    [in] uiMilliseconds
//!           Wait time.
//! \return   uint32_t
//!           If the function succeeds, the return value is the wait result of the
//!           semaphore/mutex/thread object.
//!
uint32_t MOS_WaitForSingleObject(
    void                        *pObject,
    uint32_t                    uiMilliseconds);

//!
//! \brief    Wait for multiple objects of semaphore/mutex/thread and returns the result
//! \details  Wait for multiple objects of semaphore/mutex/thread and returns the result
//! \param    [in] uiThreadCount
//!           The number of object handles in the array pointed to by ppObjects.
//! \param    [in] ppObjects
//!           An array of object handles.
//! \param    [in] bWaitAll
//!           If true, the function returns when the state of all objects in the ppObjects array is signaled.
//!           If false, the function returns when the state of any one of the objects is set to signaled.
//! \param    [in] uiMilliseconds
//!           The time-out interval, in milliseconds.
//! \return   uint32_t
//!           Return the wait result
//!
uint32_t MOS_WaitForMultipleObjects(
    uint32_t                    uiThreadCount,
    void                        **ppObjects,
    uint32_t                    bWaitAll,
    uint32_t                    uiMilliseconds);

//!
//! \brief      Convert MOS_STATUS to OS dependent RESULT/Status
//! \param      [in] eStatus
//!             MOS_STATUS that will be converted
//! \return     MOS_OSRESULT
//!             Corresponding return code on different OSes
//!
MOS_OSRESULT MOS_StatusToOsResult(
    MOS_STATUS               eStatus);

//!
//! \brief      Convert OS dependent RESULT/Status to MOS_STATUS
//! \param      [in] eResult
//!             OS dependent result that will be converted
//! \return     MOS_STATUS
//!             Corresponding MOS_STATUS
//!
MOS_STATUS OsResultToMOS_Status(
    MOS_OSRESULT            eResult);

//!
//! \brief    sinc
//! \details  Calculate sinc(x)
//! \param    [in] x
//!           float
//! \return   float
//!           sinc(x)
//!
float MOS_Sinc(
    float                   x);

//!
//! \brief    Lanczos
//! \details  Calculate lanczos(x)
//!           Basic formula is:  lanczos(x)= MOS_Sinc(x) * MOS_Sinc(x / fLanczosT)
//! \param    [in] x
//!           float
//! \param    [in] dwNumEntries
//!           dword
//! \param    [in] fLanczosT
//! 
//! \return   float
//!           lanczos(x)
//!
float MOS_Lanczos(
    float                   x,
    uint32_t                dwNumEntries,
    float                   fLanczosT);

//!
//! \brief    General Lanczos
//! \details  Calculate lanczos(x)  with odd entry num support
//!           Basic formula is:  lanczos(x)= MOS_Sinc(x) * MOS_Sinc(x / fLanczosT)
//! \param    [in] x
//!           float
//! \param    [in] dwNumEntries
//!           dword
//! \param    [in]fLanczosT
//! 
//! \return   float
//!           lanczos(x)
//!
float MOS_Lanczos_g(
    float                   x,
    uint32_t                dwNumEntries,
    float                   fLanczosT);

//!
//! \brief    GCD
//! \details  Recursive GCD calculation of two numbers
//! \param    [in] a
//!           uint32_t
//! \param    [in] b
//!           uint32_t
//! \return   uint32_t
//!           MOS_GCD(a, b)
//!
uint32_t MOS_GCD(
    uint32_t               a,
    uint32_t               b);

//!
//! \brief    Get local time
//! \details  Get local time
//! \param    [out] tm
//!           tm struct
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_GetLocalTime(
    struct tm* tm);

//!
//! \brief    Wrapper function for SwizzleOffset
//! \details  Wrapper function for SwizzleOffset in Mos 
//! \param    [in] pSrc
//!           Pointer to source data.
//! \param    [out] pDst
//!           Pointer to destiny data.
//! \param    [in] SrcTiling
//!           Source Tile Type
//! \param    [in] DstTiling
//!           Destiny Tile Type
//! \param    [in] iHeight
//!           Height
//! \param    [in] iPitch
//!           Pitch
//! \return   void
//!
void Mos_SwizzleData(
    uint8_t         *pSrc,
    uint8_t         *pDst,
    MOS_TILE_TYPE   SrcTiling,
    MOS_TILE_TYPE   DstTiling,
    int32_t         iHeight,
    int32_t         iPitch);

//!
//! \brief    MOS trace event initialize
//! \details  register provide Global ID to the system. 
//! \param    void
//! \return   void
//!
void MOS_TraceEventInit();

//!
//! \brief    MOS trace event close
//! \details  un-register provider Global ID. 
//! \param    void
//! \return   void
//!
void MOS_TraceEventClose();

//!
//! \brief    MOS log trace event
//! \details  log trace event by id and event type, arg1 and arg2 are optional arguments
//!           arguments are in raw data format, need match data structure in manifest.
//! \param    [in] usId
//!           Indicates event id
//! \param    [in] ucType
//!           Indicates event type
//! \param    [in] pArg1
//!           event data address
//! \param    [in] dwSize1
//!           event data size
//! \param    [in] pArg2
//!           event data address
//! \param    [in] dwSize2
//!           event data size
//! \return   void
//!
void MOS_TraceEvent(
    uint16_t         usId,
    uint8_t          ucType,
    void * const     pArg1,
    uint32_t         dwSize1,
    void * const     pArg2,
    uint32_t         dwSize2);

#ifdef __cplusplus
}
#endif

#endif // __MOS_UTILITIES_H__
