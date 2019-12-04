/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_utilities_next.cpp
//! \brief    Common OS service across different platform
//! \details  Common OS service across different platform
//!

#include "mos_utilities.h"
#include "mos_util_debug_next.h"
#include "mos_utilities_specific_next.h"
#include "mos_util_user_interface_next.h"
#include <sstream>
#include <fcntl.h>     //open
#include <malloc.h>    // For memalign
#include <string.h>    // memset
#include <stdlib.h>    // atoi atol
#include <math.h>

#if MOS_MESSAGES_ENABLED
#include <time.h>     //for simulate random memory allcation failure
#endif


//int32_t MosMemAllocCounter;      //!< Counter to check memory leaks
//int32_t MosMemAllocFakeCounter;
//int32_t MosMemAllocCounterGfx;
int32_t MosUtilities::m_mosMemAllocCounterNoUserFeature = 0;
int32_t MosUtilities::m_mosMemAllocCounterNoUserFeatureGfx = 0;
uint8_t MosUtilities::m_mosUltFlag = 0;
int32_t MosUtilities::m_mosSimulateRandomAllocMemoryFailFreq = 0;


MOS_FUNC_EXPORT void MosUtilities::MosSetUltFlag(uint8_t ultFlag)
{
    MosUtilities::m_mosUltFlag = ultFlag;
}

MOS_FUNC_EXPORT int32_t MosUtilities::MosGetMemNinjaCounter()
{
    return m_mosMemAllocCounterNoUserFeature;
}

MOS_FUNC_EXPORT int32_t MosUtilities::MosGetMemNinjaCounterGfx()
{
    return m_mosMemAllocCounterNoUserFeatureGfx;
}

#define __MOS_USER_FEATURE_VALUE_SINGLE_SLICE_VEBOX_DEFAULT_VALUE "1"
#define __MAX_MULTI_STRING_COUNT         128

char MosUtilities::m_xmlFilePath[MOS_USER_CONTROL_MAX_DATA_SIZE] = {};

MOS_USER_FEATURE_VALUE_MAP MosUtilities::m_userFeatureKeysMap[__MOS_USER_FEATURE_KEY_MAX_ID] = {};

MOS_USER_FEATURE_VALUE MosUtilities::m_mosUserFeatureDescFields[__MOS_USER_FEATURE_KEY_MAX_ID] =
{
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_ENABLE_ID,
        "Media Reset",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "If enabled, media reset will be enabled. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_TH_ID,
        "Media Reset TH",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, media reset will be enabled. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SOFT_RESET_ENABLE_ID,
        "Soft Reset",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, soft reset will be enabled. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_SIM_IN_USE_ID,
        "Simulation In Use",
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports whether the media driver is used in simulation/emulation mode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_LINUX_PERFORMANCETAG_ENABLE_ID,
        "Linux PerformanceTag Enable",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Linux Performance Tag"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_ID,
        "Perf Profiler Enable",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Perf Profiler Enable Control Flag"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_FE_BE_TIMING,
        "Perf Profiler FE BE timing measurement",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Perf Profiler FE&BE Timing Measurement Flag"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_OUTPUT_FILE,
        "Perf Profiler Output File Name",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "Perf_DATA_00_00.bin",
        "Performance Profiler Output File Name"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_BUFFER_SIZE,
        "Perf Profiler Buffer Size",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "10000000",
        "Performance Profiler Buffer Size"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_TIMER_REG,
        "Perf Profiler Timer Reg",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Timer Register"),
        MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_ENABLE_MULTI_PROCESS,
        "Perf Profiler Multi Process Support",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Multi Process Support"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_1,
        "Perf Profiler Register 1",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_2,
        "Perf Profiler Register 2",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_3,
        "Perf Profiler Register 3",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_4,
        "Perf Profiler Register 4",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_5,
        "Perf Profiler Register 5",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_6,
        "Perf Profiler Register 6",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_7,
        "Perf Profiler Register 7",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_PERF_PROFILER_REGISTER_8,
        "Perf Profiler Register 8",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Performance Profiler Memory Information Register"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DISABLE_KMD_WATCHDOG_ID,
        "Disable KMD Watchdog",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable KMD Watchdog"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        "Single Task Phase Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables single task phase mode. This feature is only enabled for AVC and HEVC encode."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AUX_TABLE_16K_GRANULAR_ID,
        "Aux Table 16K Granular",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "Switches between 1-16K and 0-64K Granularity for Aux Table."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MFE_MBENC_ENABLE_ID,
        "MFE MBEnc Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables MFE MBEnc Mode. This feature is only enabled for AVC encode."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MFE_FIRST_BUFFER_SUBMIT_ID,
        "MFE First Buffer Submit",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to indicate MFE work on UMD level"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_RC_PANIC_ENABLE_ID,
        "RC Panic Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "Enables/Disables PAK panic mode feature."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_ENABLE_ID,
        "Slice Shutdown Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "Used to Enable/Disable Slice shutdown. Only has impact on HSW."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_FORCE_YFYS_ID,
        "Force to allocate YfYs",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Force to allocate internal surface as Yf or Ys"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DECODE_LOCK_DISABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_DECODE_LOCK_DISABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If decode output surface can be locked for sync. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_ID,
        "Encode HW Walker",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Used to Enable/Disable HW walker."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_SUPPRESS_RECON_PIC_ENABLE_ID,
        "Encode Suppress Recon Pic",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Used to suppress recon pic generation for non-ref surfaces."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_ME_IN_USE_ID,
        "Encode HME In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if HME is in use."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_16xME_IN_USE_ID,
        "Encode SuperHME In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if SuperHme is in use."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_32xME_IN_USE_ID,
        "Encode UltraHME In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if UltraHme is in use."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_RATECONTROL_METHOD_ID,
        "Encode RateControl Method",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report the RateControl Method."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_TARGET_USAGE_OVERRIDE_ID,
        "Encode TU Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Used to override TU value "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_ME_ENABLE_ID,
        "AVC Encode HME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables HME for AVC."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_16xME_ENABLE_ID,
        "AVC Encode SuperHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "255",
        "Enables/Disables SHME for AVC."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_32xME_ENABLE_ID,
        "AVC Encode UltraHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "255",
        "Enables/Disables UHME for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_MULTIPRED_ENABLE_ID,
        "AVC Encode MultiPred",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables MultiPred feature for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_INTRA_REFRESH_QP_THRESHOLD_ID,
        "AVC Encode Intra Refresh Qp Threshold",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Gives Intra Refresh Qp Threshold value."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_FTQ_ENABLE_ID,
        "AVC FTQ Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable/Disable FTQ for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_CAF_ENABLE_ID,
        "AVC CAF Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable/Disable CAF for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_CAF_DISABLE_HD_ID,
        "AVC CAF Disable HD",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable/Disable CAF for HD resolutions for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_MB_BRC_ENABLE_ID,
        "AVC Encode MB BRC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "255",
        "Enables/Disables MBBRC for AVC "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_FORCE_TO_SKIP_ENABLE_ID,
        "AVC Force to Skip Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Enables/Disables Force to Skip for AVC Encode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_SLIDING_WINDOW_SIZE_ID,
        "AVC Sliding Window Size",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Sliding Window Size for AVC Encode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_ENABLE_ID,
        "AVC Encode Rounding Inter Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Rounding Inter feature for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_P_ID,
        "AVC Encode Rounding Inter P",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "255",
        "Sets the PAK Inter Rounding value for P frame for AVC "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_B_ID,
        "AVC Encode Rounding Inter B",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "255",
        "Sets the PAK Inter Rounding value for B frame for AVC "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_BREF_ID,
        "AVC Encode Rounding Inter BRef",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "255",
        "Sets the PAK Inter Rounding value for B ref frame for AVC "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_ROUNDING_INTER_ENABLE_ID,
        "AVC Encode Adaptive Rounding Inter Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Adaptive Inter Rounding for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_SKIP_BIAS_ADJUSTMENT_ENABLE_ID,
        "AVC Encode Skip Bias Adjustment Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Skip Bias Adjustment feature for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_INTRA_SCALING_ENABLE_ID,
        "AVC Encode Adaptive Intra Scaling Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Adaptive Intra Scaling feature for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_OLD_MODE_COST_ENABLE_ID,
        "AVC Encode Old Mode Cost Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables/Disables Old Mode Cost tables for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_MB_SLICE_THRESHOLD_ID,
        "VDENC Encode MB Slice Threshold",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "12",
        "Sets the VDENC MB slice threshold."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_SLICE_THRESHOLD_TABLE_ID,
        "Use VDENC Slice Threshold Table",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Sets the VDENC slice threshold table."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_TAIL_INSERTION_DELAY_COUNT_ID,
        "VDENC Encode Tail Insertion Delay Count",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1500",
        "Sets the VDENC Delay count."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_THRESHOLD_I_SLICE_SIZE_MINUS_ID,
        "VDENC Encode Threshold I Slice Size Minus",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "500",
        "Sets the VDENC I SLICE threshold."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_THRESHOLD_P_SLICE_SIZE_MINUS_ID,
        "VDENC Encode Threshold P Slice Size Minus",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "500",
        "Sets the VDENC P SLICE threshold."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_CRE_PREFETCH_ENABLE_ID,
        "AVC VDEnc CRE Prefetch Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables CRE Prefetch for AVC VDEnc. Enabled by default for perf improvement."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_TLB_PREFETCH_ENABLE_ID,
        "AVC VDEnc TLB Prefetch Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables TLB Prefetch for AVC VDEnc. Enabled by default for perf improvement."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_TLB_ALLOCATION_WA_ENABLE_ID,
        "AVC VDEnc TLB WA Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables TLB Allocation WA for AVC VDEnc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_0_OVERRIDE_ID,
        "MFX_LRA_0 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Override Register MFX_LRA_0. Valid Only When AVC VDEnc TLB Allocation WA is Enabled."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_1_OVERRIDE_ID,
        "MFX_LRA_1 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Override Register MFX_LRA_1. Valid Only When AVC VDEnc TLB Allocation WA is Enabled."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_2_OVERRIDE_ID,
        "MFX_LRA_2 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Override Register MFX_LRA_2. Valid Only When AVC VDEnc TLB Allocation WA is Enabled."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_FLATNESS_CHECK_ENABLE_ID,
        "Flatness Check Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Flatness Check feature. This feature is only supported for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_SEARCH_WINDOW_ENABLE_ID,
        "Adaptive Search Window Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Adaptive Search Window feature. This feature is only supported for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ADAPTIVE_TRANSFORM_DECISION_ENABLE_ID,
        "Adaptive transform decision Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_VALUE_ADAPTIVE_TRANSFORM_DECISION_ENABLE_DEFAULT_VALUE,
        "Enables/Disables Adaptive transform decision feature. This feature is only supported for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L0_IN_USE_ID,
        "Weighted prediction used for L0 reference",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Weighted prediction used for L0 reference."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L1_IN_USE_ID,
        "Weighted prediction used for L1 reference",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Weighted prediction used for L1 reference."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_FBR_BYPASS_ENABLE_ID,
        "FBR Bypass Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Enables/Disables FBR Bypass feature. Starting SKL for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_STATIC_FRAME_DETECTION_ENABLE_ID,
        "Static Frame Detection Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Enables/Disables Static Frame Detection feature. Starting BDW for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_SINGLE_PASS_ENABLE_ID,
        "VDEnc Single Pass Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enables/Disables VDEnc single pass feature. Starting from KBL for AVC VDEnc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENC_BRC_MOTION_ADAPTIVE_ENABLE_ID,
        "VDEnc BRC Motion Adaptive Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enables/Disables VDEnc motion adaptive BRC for AVC VDEnc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_FRAME_TRACKING_ID,
        "Enable Frame Tracking",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Frame Tracking."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_COLOR_BIT_SUPPORT_ENABLE_ID,
        "Colorbit Support Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables Colorbit Support. This feature is only supported for AVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_GROUP_ID_SELECT_ENABLE_ID,
        "Group ID Select Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables/Disables Group Id Select."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_BRC_ENABLE_ID,
        "AVC Encode BRC Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_MULTIREF_QP_ID,
        "AVC Encode Multiref Qp",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Used to enable or disable multiref QP feature for BRC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_BRC_VAR_COMPU_BYPASS_ID,
        "BRC Variance Computation Bypass",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Enables/Disables BRC Variance Computation Bypass feature. for GLK perf debug."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_AVC_BRC_SOFTWARE_ID,
        "AVC BRC SW Simulation",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to enable BRC SW simulation."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_BRC_SOFTWARE_IN_USE_ID,
        "AVC BRC SW Simulation In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if AVC BRC SW Simulation is in use."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_CNL_AVC_ENCODE_ARB_WA_ID,
        "Enable CNL AVC Encode ARB WA",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable/Disable CNL AVC Encode ARB WA for hang reproducing."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ME_ENABLE_ID,
        "VP9 Encode HME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP9 ME Encode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_16xME_ENABLE_ID,
        "VP9 Encode SuperHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP9 16xME Encode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_HUC_ENABLE_ID,
        "VP9 Encode HUC Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables VP9 Huc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_IN_USE_ID,
        "Encode BRC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if BRC is turned on"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        "VP9 Encode Multipass BRC Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP9 Encode Multipass BRC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_IN_USE_ID,
        "VP9 Encode Multipass BRC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if Multipass BRC is turned on."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
        "VP9 Encode Adaptive RePAK Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables VP9 Encode Adaptive RePAK."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_IN_USE_ID,
        "VP9 Encode Adaptive RePAK In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if Adaptive RePAK is turned on."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_SINGLE_PASS_DYS_ENABLE_ID,
        "VP9 Encode Single Pass Dys Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Report key to indicate if Single Pass Dys is turned on."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER_ID,
        __MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the internal allocation counter value. If this value is not 0, the test has a memory leak."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_CMD_INIT_HUC_ID,
        "VDEnc CmdInitializer Huc Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable CmdInitializer HuC FW for HEVC/VP9 VDEnc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_ID,
        "HEVC Encode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables HEVC Encode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_SECURE_INPUT_ID,
        "Secure HEVC Encode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Secure HEVC Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Secure HEVC Encode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_MEDIARESET_TEST_ID,
        "Enable MediaReset Test",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable HEVC Encode Media Reset Test, by default:0(disabled)."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_WP_SUPPORT_ID,
        "Enable WP Support",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable Weighted Prediction support in HEVC Encoder."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MODE_ID,
        "HEVC Encode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the internal HEVC encode mode."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ME_ENABLE_ID,
        "HEVC Encode HME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables HME for HEVC."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_16xME_ENABLE_ID,
        "HEVC Encode SuperHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables SuperHme for HEVC."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_32xME_ENABLE_ID,
        "HEVC Encode UltraHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables/Disables UHME for HEVC."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_16xME_ENABLE_ID,
        "Enable HEVC VDEnc SuperHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable/Disable SuperHme for HEVC VDEnc."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_32xME_ENABLE_ID,
        "Enable HEVC VDEnc UltraHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables UHME for HEVC VDEnc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_26Z_ENABLE_ID,
        "HEVC Encode 26Z Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables/Disables 26Z for HEVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_REGION_NUMBER_ID,
        "HEVC Encode WP Number",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "4",
        "Enables/Disables WP for HEVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_B_KERNEL_SPLIT,
        "HEVC Encode B Kernel Split Num",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Number of B kernel splits for HEVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_POWER_SAVING,
        "HEVC Encode Power Save Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable power saving mode in HEVC Enc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_NUM_8x8_INTRA_KERNEL_SPLIT,
        "HEVC Encode 8x8 Intra Kernel Split Num",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Number of 8x8 intra kernel splits for HEVC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_ENABLE_ID,
        "HEVC RDOQ Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable RDOQ for HEVC"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_IFRAME_RDOQ_ENABLE_ID,
        "HEVC I Frame RDOQ Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable I Frame RDOQ for HEVC"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        "HEVC Encode Multipass BRC Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables HEVC Encode Multipass BRC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MULTIPASS_BRC_IN_USE_ID,
        "HEVC Encode Multipass BRC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if Multipass BRC is turned on."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_ID,
        "BRC SW Simulation",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to enable BRC SW simulation Mode"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_PATH_ID,
        "BRC SW Simulation Modules Path",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "0",
        "Used to enable ENCODE BRC SW simulation Custom Path"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_SOFTWARE_IN_USE_ID,
        "BRC SW Simulation In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if ENCODE BRC SW Simulation is in use."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ACQP_ENABLE_ID,
        "HEVC VDEnc ACQP Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable ACQP for HEVC VDEnc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_VQI_ENABLE_ID,
        "HEVC VDEnc VQI Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable VQI for HEVC VDEnc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_FORCE_PAK_PASS_NUM_ID,
        "Force PAK Pass Num",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Force dual pipe PAK pass number.by default = 0: not forcing"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ROUNDING_ENABLE_ID,
        "HEVC VDEnc Rounding Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable Rounding for HEVC VDEnc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_PAKOBJCMD_STREAMOUT_ENABLE_ID,
        "HEVC VDEnc PakObjCmd StreamOut Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable PakObjCmd StreamOut for HEVC VDEnc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_LBCONLY_ENABLE_ID,
        "HEVC VDEnc LBC Only Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable LBC Only for IBC for HEVC VDEnc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_PARTIAL_FRAME_UPDATE_ENABLE_ID,
        "HEVC VDEnc Partial Frame Update Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable Partial Frame Update for HEVC VDEnc"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_NUM_THREADS_PER_LCU_ID,
        "HEVC Num Threads Per LCU",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "8",
        "Sets the number of threads per LCU. Currently used only for CNL."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_MDF_DISABLE_ID,
        "HEVC Encode MDF Disable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables/Disables MDF for HEVC Encoder."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_CODEC_MMC_ENABLE_ID,
        "Enable Codec MMC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable Codec MMCD. (0: Disable codec MMCD; other values: enable codec MMCD)."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DECODE_MMC_ENABLE_ID,
        "Enable Decode MMC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable Decode MMCD. (0: Disable decode MMCD; other values: enable decode MMCD)."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_ENABLE_ID,
        "Enable Encode MMC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable Encode MMCD. (0: Disable encode MMCD; other values: enable encode MMCD)."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_CODEC_MMC_IN_USE_ID,
        "Codec MMC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if codec MMC is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_MMC_IN_USE_ID,
        "Decode MMC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if decode MMC is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_MPEG2_MODE_ID,
        "MPEG2 Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if MPEG2 decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_VC1_MODE_ID,
        "VC1 Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if VC1 decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_AVC_MODE_ID,
        "AVC Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if AVC decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_JPEG_MODE_ID,
        "JPEG Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if JPEG decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_VP8_MODE_ID,
        "VP8 Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if VP8 decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_HEVC_MODE_ID,
        "HEVC Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if HEVC decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_VP9_MODE_ID,
        "VP9 Decode Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if VP9 decode mode is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_HISTOGRAM_FROM_VEBOX_ID,
        "Decode Histogram from VEBox",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if decode histogram is from VEBox "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DECODE_EXTENDED_MMC_IN_USE_ID,
        "Decode Extended MMC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if decode extended MMC is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_MMC_IN_USE_ID,
        "Encode MMC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if encode MMC is turned on "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_EXTENDED_MMC_IN_USE_ID,
        "Encode Extended MMC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report key to indicate if encode extended MMC is turned on "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_USED_VDBOX_NUM_ID,
        "Media Encode Used VDBOX Number",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Media Encode Used VDBOX Number."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_ENABLE_COMPUTE_CONTEXT_ID,
        "Enable Compute Context",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable Compute Context. default:0 disabled."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DECODE_ENABLE_COMPUTE_CONTEXT_ID,
        "Enable Compute Context",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable Compute Context. default:0 disabled."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSIBLE_ID,
        "Decode RT Compressible",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report Key to indicate if the surface is MMCD capable (0: no; 1: yes)."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MMC_DEC_RT_COMPRESSMODE_ID,
        "Decode RT Compress Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report Key to indicate the MMCD compression mode of a surface "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSIBLE_ID,
        "Encode Recon Compressible",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report Key to indicate if the surface is MMCD capable (0: no; 1: yes)."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MMC_ENC_RECON_COMPRESSMODE_ID,
        "Encode Recon Compress Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report Key to indicate the MMCD compression mode of a surface "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE_ID,
        __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "-559038242", //0xDEADC0DE
        "Override Slice/Sub-Slice/EU request"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_DEFAULT_STATE_ID,
        "Slice Shutdown Default State",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Slice Shutdown default state."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_REQUEST_STATE_ID,
        "Slice Shutdown Request State",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Slice Shutdown requested state ."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_RESOLUTION_THRESHOLD_ID,
        "Slice Shutdown Resolution Threshold",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Slice Shutdown Resolution Threshold "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SLICE_SHUTDOWN_TARGET_USAGE_THRESHOLD_ID,
        "Slice Shutdown Target Usage Threshold",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Slice Shutdown Target Usage Threshold "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_SLICE_COUNT_SET_SUPPORT_ID,
        "Slice Count Set Support",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Support Slice Count Set "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DYNAMIC_SLICE_SHUTDOWN_ID,
        "Dynamic Slice Shutdown",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables/Disables Dynamic Slice Shutdown "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MPEG2_SLICE_STATE_ENABLE_ID,
        "Mpeg2 Encode Slice State Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Slice Shutdown related param for Mpeg2."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MPEG2_ENCODE_BRC_DISTORTION_BUFFER_ENABLE_ID,
        "Mpeg2 Encode BRC Distorion Buffer enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enables/Disables BRC distorion buffer dump for MPEG2 Encoder"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENABLE_VDBOX_BALANCING_ID,
        "Enable VDBox load balancing",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Enable balancing of VDBox load by KMD hint. (Default FALSE: disabled"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_NUMBER_OF_CODEC_DEVICES_ON_VDBOX1_ID,
        "Num of Codec Devices on VDBOX1",
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,//read path and write path are the same
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report number of Codec devices created on VDBox #1."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_NUMBER_OF_CODEC_DEVICES_ON_VDBOX2_ID,
        "Num of Codec Devices on VDBOX2",
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,//read path and write path are the same
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Report number of Codec devices created on VDBox #2"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_VDI_MODE_ID,
        __MEDIA_USER_FEATURE_VALUE_VDI_MODE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "Always true for Gen7.5+"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MEDIA_WALKER_MODE_ID,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_WALKER_MODE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "-1",
        "Media Walker Mode: Disabled(0), Repel(1), Dual(2), Quad(3), default(-1):Not Set"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_CSC_COEFF_PATCH_MODE_DISABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "FALSE if CSC coefficient setting mode is Patch mode, otherwise Curbe mode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP8_HW_SCOREBOARD_ENABLE_ID,
        "VP8 HW Scoreboard",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP8 Encode HW Scoreboard Feature."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_ME_ENABLE_ID,
        "VP8 Encode HME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP8 Encode HME Feature."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_16xME_ENABLE_ID,
        "VP8 Encode SuperHME",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP8 Encode SuperHME Feature"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_REPAK_ENABLE_ID,
        "VP8 Encode Repak",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP8 Encode Repak Feature."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        "VP8 Encode Multipass BRC Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP8 Encode Multipass BRC."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP8_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
        "VP8 Encode Adpative Repak Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enables VP8 Encode Adaptive Repak Feature."),
#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE_ID,
        "Dump Command Buffer Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, all of the command buffers submitted through MOS will be dumped (0: disabled, 1: to a file, 2: as a normal message)."),
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_ENABLE_ID,
        "Dump Command Info Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, gpu command info will be dumped (0: disabled, 1: to a file)."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_PATH_ID,
        "Dump Command Info Path",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Path where command info will be dumped, for example: ./"),
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MHW_BASE_VDENC_INTERFACE_ID,
        "Use Mhw Base Vdenc Interface",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Mhw Base Vdenc Interface Active Flag"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MEDIA_PREEMPTION_ENABLE_ID,
        "Media Preemption Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1", // Enable media UMD preemption by default under release internal version and debug version for CNL+ even if there is no user feature setting.
        "Enable/Disable Pre-emption for media"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MDF_OVERRIDE_L3ALLOC_REG,
        "MDF L3ALLOC register override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0xff00ff00",
        "Override L3 ALLOC register value in MDF"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MDF_OVERRIDE_L3TCCNTRL_REG,
        "MDF L3TCCNTRL register override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0xff00ff00",
        "Override L3 TCCNTRL register value in MDF"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MDF_OVERRIDE_MOCS_INDEX,
        "MDF MOCS index override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "5",
        "Override MOCS index value in MDF"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MDF_FORCE_RAMODE,
        "MDF Force RAMode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Force GPU context be created in RAMode"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_GROUP_ID_ID,
        "Group ID",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Sets the value of Group ID"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_VFE_MAX_THREADS_ID,
        "Encode VFE Max Threads",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to set the max number of threads for VFE."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_VFE_MAX_THREADS_SCALING_ID,
        "Encode VFE Max Threads For Scaling",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to set the max number of threads for VFE."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_FTQ_IN_USE_ID,
        "AVC FTQ Enable In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if FTQ is enabled."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_AVC_CAF_IN_USE_ID,
        "AVC CAF Enable In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to report if CAF is enabled."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_HW_WALKER_MODE_ID,
        "Encode HW Walker Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to set walker mode - useful in HSW. Not used for BDW+. ."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG_OVERRIDE_ID,
        "Encode L3CNTLREG Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to override the L3CNTLREG value."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG2_OVERRIDE_ID,
        "Encode L3CNTLREG2 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to override the L3CNTLREG2 value for HSW. Not  yet used for BDW+."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_CNTLREG3_OVERRIDE_ID,
        "Encode L3CNTLREG3 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to override the L3CNTLREG3 value for HSW. Not  yet used for BDW+."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG1_OVERRIDE_ID,
        "Encode L3SQCREG1 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to override the L3SQCREG1 value for HSW. Not  yet used for BDW+."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_L3_CACHE_SQCREG4_OVERRIDE_ID,
        "Encode L3SQCREG4 Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to override the L3SQCREG2 value for HSW. Not  yet used for BDW+."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_L3_LRA_1_REG1_OVERRIDE_ID,
        "L3LRA1RegOverride",
        __MEDIA_USER_FEATURE_SUBKEY_PERFORMANCE,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Used to override the L3LRA1Reg value for HSW. Not  yet used for BDW+."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_NULL_HW_ACCELERATION_ENABLE_ID,
        "NullHWAccelerationEnable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "General",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, go through the nullptr HW driver. (0: Disable, 1: Null HW enabled)."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_FORCE_VDBOX_ID,
        "Force VDBOX",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Force the VDBox to be used. (Default 0: FORCE_VDBOX_NONE "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDBOX_ID_USED,
        "Used VDBOX ID",
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,//read path and write path are the same
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Which VDBox ID is used. (Default 0: Not used, 1: VDBox used. Each Hex symbol represents one VDBOX, e.g. bits[3:0] means VDBOX0, bits[7:4] means VDBOX1)."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_VDENC_IN_USE_ID,
        "VDENC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out if VDEnc is used."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_CSC_METHOD_ID,
        "Encode CSC Method",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out which CSC method is in use."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_RAW_TILE_ID,
        "Encode Raw Surface Tile",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out raw surface tile."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENCODE_RAW_FORMAT_ID,
        "Encode Raw Surface Format",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out raw surface format."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "TRUE for enabling GD2 kernel debug on VPHAL, otherwise disabling"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_SURF_BTI_ID,
        __MEDIA_USER_FEATURE_VALUE_ISA_ASM_DEBUG_SURF_BTI,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "39",
        "BTI for GD2 kernel debug surface on VPHAL."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
        "Disable RowStore Cache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable cache for RowStore buffer. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_INTRAROWSTORECACHE_DISABLE_ID,
        "DisableIntraRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable Intra prediction RowStore buffer cache. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DEBLOCKINGFILTERROWSTORECACHE_DISABLE_ID,
        "DisableDeblockingFilterRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable Intra prediction RowStore buffer cache. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_BSDMPCROWSTORECACHE_DISABLE_ID,
        "DisableBsdMpcRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable decoder BSD/encoder MPC RowStore buffer cache. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MPRROWSTORECACHE_DISABLE_ID,
        "DisableMprRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable MPR RowStore buffer cache. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VDENCROWSTORECACHE_DISABLE_ID,
        "DisableVDEncRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, disable rowstore cache for VDEnc."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODEC_SIM_ENABLE_ID,
        "Codec Simulation Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, specify this is in pre-si simulation/emulation mode."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_BREAK_IN_CODECHAL_CREATE_ID,
        "Break In CodecHal_Create",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, asserts in CodecHal_Create."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MEDIASOLO_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "If enabled, triggers the MediaSolo code path in MOS for pre-si testing."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_STREAM_OUT_ENABLE_ID,
        "Stream Out",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable decode stream out "),
     MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DECOMPRESS_DECODE_OUTPUT_ID,
        "Decompress Decode Output",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Call Vebox decompress for decode output at decode endframe"),
     MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DECOMPRESS_DECODE_SFC_OUTPUT_ID,
        "Decompress Decode Sfc Output",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Call Vebox decompress for sfc output at decode endframe"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_OUTPUT_DIRECTORY_ID,
        "CodecHal Debug Output Directory",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Directory where all CodecHal debug interface can locate cfg file and dump."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_DUMP_OUTPUT_DIRECTORY_ID,
        "CodecHal Dump Output Directory",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "CodecHal Dump Output Directory."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_DEBUG_CFG_GENERATION_ID,
        "CodecHal Debug Cfg Generation",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Enable the Generation of CodecHal Debug Cfg file."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_RDOQ_INTRA_TU_OVERRIDE_ID,
        "RDOQ Intra TU Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Override Intra RDOQ TU setting."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_RDOQ_INTRA_TU_DISABLE_ID,
        "RDOQ Intra TU Disable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Disable RDOQ for Intra TU."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_RDOQ_INTRA_TU_THRESHOLD_ID,
        "RDOQ Intra TU Threshold",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "RDOQ Intra TU Threshold"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_ENABLE_FAKE_HEADER_SIZE_ID,
        "Fake Header Size Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable Fake Header Size"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_FAKE_IFRAME_HEADER_SIZE_ID,
        "Fake IFrame Header Size",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "128",
        "Fake I Frame Header Size"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CODECHAL_FAKE_PBFRAME_HEADER_SIZE_ID,
        "Fake PBFrame Header Size",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "16",
        "Fake P/B Frame Header Size"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_COMMAND_OVERRIDE_INPUT_FILE_PATH_ID,
        "Command Override Input File Path",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Path of command override input file"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HUC_DEMO_KERNEL_ID, // Used to indicate which huc kernel to load for the Huc Demo feature
        "Media Huc Demo kernel Id",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Media",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "3",   // Default is 3 which is huc copy kernel
        "Id of demo huc kernel to load"),
#endif // (_DEBUG || _RELEASE_INTERNAL
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_STATUS_REPORTING_ENABLE_ID,
        "Status Reporting",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "Enable decode status reporting"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_POSITION_ID,
        __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_POSITION,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Demo position: Disable(0), Left(1), Right(2), Top(3), Bottom(4)"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_PARAMETERS_ID,
        __MEDIA_USER_FEATURE_VALUE_SPLIT_SCREEN_DEMO_PARAMETERS,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Specify which VP features on/off for Demo mode"),
#if MOS_MESSAGES_ENABLED
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enables the creation of a log file where all of the enabled messages will be written."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Specifies the location of the log file where all of the enabled messages will be written."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Prints out all of the enabled messages either to a debugger or to the Android log."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of MOS. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different MOS subcomponents to have different debug levels."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_HW_TAG_ID,
        "Mhw Message Tags",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of MHW. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_HW_ID,
        "Mhw Tags By Sub Component",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_HW_TAG_ID,
        "Mhw Sub Components Tags",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different MHW subcomponents to have different debug levels."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of Codec. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different Codec subcomponents to have different debug levels. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of VP"),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different VP subcomponents to have different debug levels."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of CP"),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different CP subcomponents to have different debug levels. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of DDI"),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different MOS subcomponents to have different debug levels. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of CM "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different CM subcomponents to have different debug levels. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_SCALABILITY_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_SCALABILITY_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of SCALABILITY "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_SCALABILITY_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_SCALABILITY,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_SCALABILITY_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_SCALABILITY_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different SCALABILITY subcomponents to have different debug levels. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_MMC_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_MMC_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of MMC "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MMC_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MMC,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_MMC_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MMC_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different MMC subcomponents to have different debug levels. "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_MESSAGE_BLT_TAG_ID,
        __MOS_USER_FEATURE_KEY_MESSAGE_BLT_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE_STR,
        "Enables messages and/or asserts for all of BLT "),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_BLT_ID,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_BLT,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "If enabled, will allow the subcomponent tags to take effect."),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_SUB_COMPONENT_BLT_TAG_ID,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_BLT_TAG,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT64,
        "0",
        "Allows different BLT subcomponents to have different debug levels. "),
#endif // MOS_MESSAGES_ENABLED
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_SF_2_DMA_SUBMITS_ENABLE_ID,
        "Enable HEVC SF 2 DMA Submits",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Specify if send HuC and HCP commands in one DMA buffer or two DMA buffer. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVCDATROWSTORECACHE_DISABLE_ID,
        "DisableHevcDatRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Decide if put the DatRowStore buffer to cache or driver allocated buffer. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVCDFROWSTORECACHE_DISABLE_ID,
        "DisableHevcDfRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Decide if put the DfRowStore buffer to cache or driver allocated buffer. "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVCSAOROWSTORECACHE_DISABLE_ID,
        "DisableHevcSaoRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Decide if put the SAORowStore buffer to cache or driver allocated buffer."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_HVDROWSTORECACHE_DISABLE_ID,
        "DisableVp9HvdRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "VP9"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_DATROWSTORECACHE_DISABLE_ID,
        "DisableVp9DatRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "VP9"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VP9_DFROWSTORECACHE_DISABLE_ID,
        "DisableVp9DfRowStoreCache",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "VP9"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_DDI_DUMP_DIRECTORY_ID,
        "DDI Dump Directory",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "DDI DUMP DIR"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_DDI_DUMP_ENABLE_ID,
        "Encode DDI Dump Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "DDI DUMP ENCODE Enable"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_ETW_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_ETW_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF ETW Log"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_LOG_LEVEL_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_LOG_LEVEL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF Log Level"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_UMD_ULT_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_UMD_ULT_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF UMD ULT"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF Command buffer Dump"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_CURBE_DUMP_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_CURBE_DUMP_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF Curbe Dump"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_DUMP_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_DUMP_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF Surface Dump"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF Surface State Dump"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_COUNTER_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_CMD_DUMP_COUNTER,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Record MDF Command Buffer Dump counter for multiple device create/destroy"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_SURFACE_STATE_DUMP_COUNTER,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Record MDF Surface state Dump counter for multiple device create/destroy"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_DUMP_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_DUMP,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable MDF interface descriptor data dump"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_COUNTER_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_INTERFACE_DESCRIPTOR_DATA_COUNTER,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Record MDF Interface descriptor data Dump counter for multiple device create/destroy"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_DUMPPATH_USER_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_DUMPPATH_USER,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "MDF dump path specified by user"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_FORCE_EXECUTION_PATH_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_EXECUTION_PATH,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MDF execution path specified by user"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_MAX_THREAD_NUM_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_MAX_THREAD_NUM,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MDF maximun thread number specified by user"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_FORCE_COHERENT_STATELESSBTI_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_COHERENT_STATELESSBTI,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MDF coherent stateless BTI specified by user"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_EMU_MODE_ENABLE_ID,
        __MEDIA_USER_FEATURE_VALUE_MDF_EMU_MODE_ENABLE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MDF EMU Enable"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_DEFAULT_CM_QUEUE_TYPE_ID,
        "MDF Default Queue Type",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Program default CM_QUEUE_TYPE for debug."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_CCS_USE_VE_INTERFACE,
        "MDF CCS Use VE Interface",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Switch to use mos virtual engine interface for compute CS."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_MDF_CCS_USE_VE_DEBUG_OVERRIDE,
        "MDF CCS Use VE Debug Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MDF",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Set debug override for mos virtual engine interface for compute CS."),
    MOS_DECLARE_UF_KEY(__VPHAL_VEBOX_OUTPUTPIPE_MODE_ID,
        "VPOutputPipe Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "For Notify which datapath Vebox used"),
    MOS_DECLARE_UF_KEY(__VPHAL_VEBOX_FEATURE_INUSE_ID,
        "VeBox Feature In use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "For Notify which feature Vebox used"),
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_RNDR_SSD_CONTROL_ID,
        "SSD Control",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Slice Shutdown Control"),
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_RNDR_SCOREBOARD_CONTROL_ID,
        "SCOREBOARD Control",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "Software Scoreboard enable Control"),
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_RNDR_CMFC_CONTROL_ID,
        "CMFC Control",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "CM based FC enable Control"),
#if (_DEBUG || _RELEASE_INTERNAL)
     MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_RNDR_FORCE_VP_DECOMPRESSED_OUTPUT_ID,
        "FORCE VP DECOMPRESSED OUTPUT",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "FORCE VP DECOMPRESSED OUTPUT"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMP_OUTFILE_KEY_NAME_ID,
        "outfileLocation",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Surface Dump Outfile"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMP_LOCATION_KEY_NAME_ID,
        "dumpLocations",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "VP Surface Dump Location"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMP_MANUAL_TRIGGER_KEY_NAME_ID,
        "VphalSurfaceDumpManualTrigger",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "-1",
        "Manual trigger to start VP Surface Dump"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMP_START_FRAME_KEY_NAME_ID,
        "startFrame",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Surface Dump Start Frame"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMP_END_FRAME_KEY_NAME_ID,
        "endFrame",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        MOS_USER_FEATURE_MAX_UINT32_STR_VALUE,
        "VP Surface Dump End Frame"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMPER_ENABLE_PLANE_DUMP,
        "enablePlaneDump",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Surface dump each plance seprately"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMP_ENABLE_AUX_DUMP_ID,
        "enableAuxDump",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Surface dump aux data enable"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_SURF_DUMPER_RESOURCE_LOCK_ID,
        "SurfaceDumperResourceLockError",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Surface Dump: Locking Resource"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_STATE_DUMP_OUTFILE_KEY_NAME_ID,
        "outfileLocation",
        __MEDIA_USER_FEATURE_VALUE_VP_DBG_STATE_DUMP_LOCATION,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "VP State Dump Output File"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_STATE_DUMP_LOCATION_KEY_NAME_ID,
        "dumpLocations",
        __MEDIA_USER_FEATURE_VALUE_VP_DBG_STATE_DUMP_LOCATION,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "VP State Dump Location"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_STATE_DUMP_START_FRAME_KEY_NAME_ID,
        "startFrame",
        __MEDIA_USER_FEATURE_VALUE_VP_DBG_STATE_DUMP_LOCATION,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP State Dump Start Frame"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_STATE_DUMP_END_FRAME_KEY_NAME_ID,
        "endFrame",
        __MEDIA_USER_FEATURE_VALUE_VP_DBG_STATE_DUMP_LOCATION,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        MOS_USER_FEATURE_MAX_UINT32_STR_VALUE,
        "VP State Dump End Frame"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_PARAM_DUMP_OUTFILE_KEY_NAME_ID,
        "outxmlLocation",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "VP Parameters Dump Outfile"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_PARAM_DUMP_START_FRAME_KEY_NAME_ID,
        "startxmlFrame",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "1",
        "VP Parameters Dump Start Frame"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_PARAM_DUMP_END_FRAME_KEY_NAME_ID,
        "endxmlFrame",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Parameters Dump End Frame"),
    MOS_DECLARE_UF_KEY(__VPHAL_DBG_DUMP_OUTPUT_DIRECTORY_ID,
        "Vphal Debug Dump Output Directory",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Vphal Debug Dump Output Directory"),
#endif
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_SET_SINGLE_SLICE_VEBOX_ID,
        "SetSingleSliceVeboxEnable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        __MOS_USER_FEATURE_VALUE_SINGLE_SLICE_VEBOX_DEFAULT_VALUE,
        "VP VEBOX: true for enabling single slice"),
    MOS_DECLARE_UF_KEY(__VPHAL_BYPASS_COMPOSITION_ID,
        "Bypass Composition",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Bypass Composition Mode"),
    MOS_DECLARE_UF_KEY(__VPHAL_VEBOX_DISABLE_SFC_ID,
        "Disable SFC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "For debugging purpose. true for disabling SFC"),
    MOS_DECLARE_UF_KEY(__VPHAL_ENABLE_SUPER_RESOLUTION_ID,
        "Enable VP Super Resolution",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "For debugging purpose. true for enabling VPP super resolution scaling"),
    MOS_DECLARE_UF_KEY(__VPHAL_SUPER_RESOLUTION_MODE_ID,
        "Super Resolution Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "For debugging purpose. 0 is to use default setting, 1 means FP32 mode, 2 means Hybrid mode, 3 means FP16 mode"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SUPER_RESOLUTION_ENABLE_ID,
        "SuperResolutionEnable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Eanble Super Resolution. 1: enable, 0: disable."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SUPER_RESOLUTION_MODE_ID,
        "SuperResolutionMode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Super Resolution Mode. 1: FP32, 2: Hibrid, 3: FP16."),
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_ENABLE_VEBOX_MMC_DECOMPRESS_ID,
        "Enable Vebox Decompress",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "For debugging purpose. Enable Vebox In-Place decompression"),
    MOS_DECLARE_UF_KEY(__VPHAL_ENABLE_MMC_ID,
        "Enable VP MMC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Enable memory compression"),
    MOS_DECLARE_UF_KEY(__VPHAL_ENABLE_MMC_IN_USE_ID,
        "VP MMC In Use",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "VP use memory compression"),
    MOS_DECLARE_UF_KEY(__VPHAL_PRIMARY_SURFACE_COMPRESS_MODE_ID,
        "VP Primary Surface Compress Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "VP primary surface compress mode"),
    MOS_DECLARE_UF_KEY(__VPHAL_PRIMARY_SURFACE_COMPRESSIBLE_ID,
        "VP Primary Surface Compressible",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "VP primary surface compressible"),
    MOS_DECLARE_UF_KEY(__VPHAL_RT_COMPRESS_MODE_ID,
        "VP RT Compress Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "VP render target compress mode"),
    MOS_DECLARE_UF_KEY(__VPHAL_RT_COMPRESSIBLE_ID,
        "VP RT Compressible",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "VP render target compressible"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC_ID,
        "Enable Media RenderEngine MMC",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Enable media render engine memory compression in media workload"),
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_VEBOX_DISABLE_TEMPORAL_DENOISE_FILTER_ID,
        "Disable Temporal Denoise Filter",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Temporal denoise filter disable flag"),
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_DECLARE_UF_KEY_DBGONLY(__VPHAL_COMP_8TAP_ADAPTIVE_ENABLE_ID,
        "8-TAP Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "VP Composition 8Tap Adaptive Enable"),
#endif
#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL)
    MOS_DECLARE_UF_KEY(__VPHAL_RNDR_VEBOX_MODE_0_ID,
        "VEBOX_MODE_0",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Render Vebox Mode 0"),
    MOS_DECLARE_UF_KEY(__VPHAL_RNDR_VEBOX_MODE_0_TO_2_ID,
        "VEBOX_MODE_0_TO_2",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Render Vebox Mode 0 to 2"),
    MOS_DECLARE_UF_KEY(__VPHAL_RNDR_VEBOX_MODE_2_ID,
        "VEBOX_MODE_2",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Render Vebox Mode 2"),
    MOS_DECLARE_UF_KEY(__VPHAL_RNDR_VEBOX_MODE_2_TO_0_ID,
        "VEBOX_MODE_2_TO_0",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Render Vebox Mode 2 to 0"),
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_DECLARE_UF_KEY(__VPHAL_ENABLE_COMPUTE_CONTEXT_ID,
        "VP Enable Compute Context",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP Enable Compute Context"),
#endif
    MOS_DECLARE_UF_KEY_DBGONLY(__MOS_USER_FEATURE_KEY_VP_CAPS_FF_OVERRIDE_ID,
        "VP_CAPS_FF_OVERRIDE",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "VP_CAPS_FF_OVERRIDE"),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_XML_AUTOGEN_ID,
        __MOS_USER_FEATURE_KEY_XML_AUTOGEN,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Enable"),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_XML_FILEPATH_ID,
        __MOS_USER_FEATURE_KEY_XML_FILEPATH,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        __MOS_USER_FEATURE_KEY_XML_FILEPATH_LOCATION,
        "Enable"),
    MOS_DECLARE_UF_KEY(__MOS_USER_FEATURE_KEY_XML_DUMP_GROUPS_ID,
        __MOS_USER_FEATURE_KEY_XML_DUMP_GROUPS,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "MOS",
        "Enable"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_FORCE_VEBOX_ID,
        "Force VEBOX",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Force the VEBox to be used. (Default 0: FORCE_VEBOX_NONE "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE_ID,
        "Enable Vebox Scalability",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "TRUE for Enabling Vebox Scalability. (Default FALSE: disabled"),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_VEBOX_SPLIT_RATIO_ID,
        "Vebox Split Ratio",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "50",
        "Vebox Scalability Split Ratio. (Default 50: 50 percent"),
    /* codec gen11 based */
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HCP_DECODE_MODE_SWITCH_THRESHOLD1_ID,
        "HCP Decode Mode Switch TH1",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Hcp Decode mode switch single pipe <-> 2 pipe"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HCP_DECODE_MODE_SWITCH_THRESHOLD2_ID,
        "HCP Decode Mode Switch TH2",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Hcp Decode mode switch single pipe <-> 2/3 pipe"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VE_DEBUG_OVERRIDE,
        "Enable VE Debug Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT64,
        "0",
        "Enable VE Debug Override."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_HW_SEMAPHORE,
        "Enable HW Semaphore",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable HW Semaphore."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VDBOX_HW_SEMAPHORE,
        "Enable HEVC Per VDBOX HW Semaphore in GEN11",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "Enable HEVC Per VDBOX HW Semaphore in GEN11."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_HW_STITCH,
        "HEVC Encode Enable HW Stitch",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "HEVC Encode Enable HW Stitch."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_SUBTHREAD_NUM_ID,
        "HEVC Encode SubThread Number",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "2",
        "Used to enable HEVC ENCODE SubThread Number in the ENC kernel."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_PAK_ONLY_ID,
        "HEVC PAK Only Mode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Set the PAK command/CU record folder name for HEVC encoder"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VME_ENCODE_SSE_ENABLE_ID,
        "HEVC Encode SSE Enable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Used to enable HEVC VME ENCODE SSE.(default 0:disabled)"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENCODE_DISABLE_SCALABILITY,
        "Disable Media Encode Scalability",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Disable Media Encode Scalability."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_PERF_DISABLE_ID,
        "Disable HEVC RDOQ Perf",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Encode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
        "HEVC"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_WATCHDOG_TIMER_THRESHOLD,
        "Watchdog Timer Threshold",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Decode",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "120",
        "Used to override default watchdog timer threshold"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_DECODE_VIRTUAL_ENGINE_ID,
        "Enable Decode VE",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "TRUE for Enabling Decode Virtual Engine. (Default TRUE: enabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_DECODE_VE_CTXSCHEDULING_ID,
        "Enable Decode VE CtxBasedScheduling",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "TRUE for Enabling Decode Virtual Engine context based scheduling. (Default false: disabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_LINUX_FRAME_SPLIT_ID,
        "Enable Linux Frame Split",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "TRUE for Enabling Frame Split. (Default false: disabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VIRTUAL_ENGINE_ID,
        "Enable Encode VE",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "TRUE for Enabling Encode Virtual Engine. (Default TRUE: enabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID,
        "Enable Encode VE CtxBasedScheduling",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "TRUE for Enabling Encode Virtual Engine context based scheduling. (Default false: disabled"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_VE_DEBUG_OVERRIDE_ID,
        "Enable VE Debug Override",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "TRUE for Enabling KMD Virtual Engine Debug Override. (Default FALSE: not override"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_HCP_SCALABILITY_DECODE_ID,
        "Enable HCP Scalability Decode",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "1",
        "Enable HCP Scalability decode mode. (Default 1: Scalable Decode Mode "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HCP_DECODE_ALWAYS_FRAME_SPLIT_ID,
        "HCP Decode Always Frame Split",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "HCP Decode always does frame split instead of make decision based on LZ table. (Default 0: using LZ table "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SCALABILITY_OVERRIDE_SPLIT_WIDTH_IN_MINCB,
        "Scalability Split Width In MinCb",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Override virtual tile scalability width. (Default 0: not overroded "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SCALABILITY_FE_SEPARATE_SUBMISSION_ENABLED_ID,
        "FE Separate Submission Enabled",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0", //Disable FE separate submission by default. Will change it to "Enable" after proving it has performance enhancement.
        "Enable FE separate submission in Scalability decode. (Default 0: Disable FE separate submission "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_SCALABILITY_FE_SEPARATE_SUBMISSION_IN_USE_ID,
        "FE Separate Submission In Use",
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Report FE separate submission is in use in Scalability decode. (Default 0: Disable FE separate submission "),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VME_BRC_LTR_DISABLE_ID,
        "HEVC VME BRC LTR Disable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Disable long term reference in hevc vme brc. (Default 0: LTR Enable"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VME_BRC_LTR_INTERVAL_ID,
        "HEVC VME BRC LTR Interval",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "HEVC Vme encode BRC Long term reference interval. (Default 0: interval=0"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VME_FORCE_SCALABILITY_ID,
        "HEVC VME Force Scalability For Low Size",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "HEVC Vme encode force scalability for low (< 4K) resolution. (Default 0"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_SEMA_RESET_DELAY_ID,
        "HEVC VDEnc Semaphore Reset Delay",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "15",
        "Control the num of placeholder cmds which are used for the delay of VDEnc sync semaphore"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_SET_CMD_DEFAULT_PARS_FROM_FILES_ID,
        "Set CMD Default Parameters From File",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_BOOL,
        "0",
        "Enable to set cmd default parameters from file (Default 0)"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_CMD_PARS_FILES_DIRECORY_ID,
        "CMD Parameters Input File Directory",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        "Set CMD Parameters Input File Directory"),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_ENABLE_ID,
        "ApogeiosEnable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Eanble Apogeios path. 1: enable, 0: disable."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE_ID,
        "VP Apogeios Enabled",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "VP",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Eanble Apogeios path in VP PipeLine. 1: enabled, 0: disabled."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ENABLE_UMD_OCA_ID,
       __MEDIA_USER_FEATURE_VALUE_ENABLE_UMD_OCA,
       __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
       __MEDIA_USER_FEATURE_SUBKEY_REPORT,
       "MOS",
       MOS_USER_FEATURE_TYPE_USER,
       MOS_USER_FEATURE_VALUE_TYPE_UINT32,
       "1",
       "Enable UMD_OCA in media driver. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_COUNT_FOR_OCA_BUFFER_LEAKED_ID,
        __MEDIA_USER_FEATURE_VALUE_COUNT_FOR_OCA_BUFFER_LEAKED,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the count for OCA buffer leaked. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_COUNT_FOR_OCA_1ST_LEVEL_BB_END_MISSED_ID,
        __MEDIA_USER_FEATURE_VALUE_COUNT_FOR_OCA_1ST_LEVEL_BB_END_MISSED,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the count for OCA buffer which missed to call On1stLevelBBEnd. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_COUNT_FOR_ADDITIONAL_OCA_BUFFER_ALLOCATED_ID,
        __MEDIA_USER_FEATURE_VALUE_COUNT_FOR_ADDITIONAL_OCA_BUFFER_ALLOCATED,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the count for additional OCA buffer allocated. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_OCA_STATUS_ID,
        __MEDIA_USER_FEATURE_VALUE_OCA_STATUS,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the first OCA error. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_OCA_ERROR_HINT_ID,
        __MEDIA_USER_FEATURE_VALUE_OCA_ERROR_HINT,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out the line number of first OCA error. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_IS_INDIRECT_STATE_HEAP_INVALID_ID,
        __MEDIA_USER_FEATURE_VALUE_IS_INDIRECT_STATE_HEAP_INVALID,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Report",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Reports out whether indirect state heap invalid. This key is not valid on Linux."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_ENABLE_SW_BACK_ANNOTATION_ID,
       "Encode Enable SW Back Annotation",
       __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
       __MEDIA_USER_FEATURE_SUBKEY_REPORT,
       "Encode",
       MOS_USER_FEATURE_TYPE_USER,
       MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "1",
       "Encode Enable SW Back Annotation."),
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_MODE_ID,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_MODE,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MOS memory alloc fail simulate mode， 0-Disable, 1-Random, 2-Traverse."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_FREQ_ID,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_FREQ,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MOS memory alloc fail simulate frequence."),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_HINT_ID,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_HINT,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "MOS memory alloc fail simulate counter."),
#endif //(_DEBUG || _RELEASE_INTERNAL)
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE_ID,
       __MEDIA_USER_FEATURE_VALUE_PERF_UTILITY_TOOL_ENABLE,
       __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
       __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
       "MOS",
       MOS_USER_FEATURE_TYPE_USER,
       MOS_USER_FEATURE_VALUE_TYPE_UINT32,
       "0",
       "Enable Perf Utility Tool. "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY_ID,
        __MEDIA_USER_FEATURE_VALUE_PERF_OUTPUT_DIRECTORY,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_STRING,
        "",
        " Perf Utility Tool Customize Output Directory. "),
    MOS_DECLARE_UF_KEY(__MEDIA_USER_FEATURE_VALUE_APO_MOS_PATH_ENABLE_ID,
        "ApoMosEnable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "MOS",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32,
        "0",
        "Eanble mos Apogeios path. 1: enable, 0: disable."),
    MOS_DECLARE_UF_KEY_DBGONLY(__MEDIA_USER_FEATURE_VALUE_APOGEIOS_HEVCD_ENABLE_ID,
        "ApogeiosHevcdEnable",
        __MEDIA_USER_FEATURE_SUBKEY_INTERNAL,
        __MEDIA_USER_FEATURE_SUBKEY_REPORT,
        "Codec",
        MOS_USER_FEATURE_TYPE_USER,
        MOS_USER_FEATURE_VALUE_TYPE_INT32,
        "0",
        "Eanble Apogeios hevc decode path. 1: enable, 0: disable."),
};

#define MOS_NUM_USER_FEATURE_VALUES     (sizeof(m_mosUserFeatureDescFields) / sizeof(m_mosUserFeatureDescFields[0]))

#if (_DEBUG || _RELEASE_INTERNAL)

uint32_t MosUtilities::m_mosAllocMemoryFailSimulateMode = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateFreq = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateHint = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateAllocCounter = 0;

#define MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT (0)
#define MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM (1)
#define MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE (2)

#define MIN_MEMORY_ALLOC_FAIL_FREQ (1)      //max memory allcation fail rate 100%
#define MAX_MEMORY_ALLOC_FAIL_FREQ (10000)  //min memory allcation fail rate 1/10000

#define MosAllocMemoryFailSimulationEnabled                                      \
    (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM || \
     m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE)

void MosUtilities::MosInitAllocMemoryFailSimulateFlag()
{
    MOS_USER_FEATURE       UserFeature;
    MOS_USER_FEATURE_VALUE UserFeatureValue;
    MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

    //default off for simulate random fail
    m_mosAllocMemoryFailSimulateMode  = MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT;
    m_mosAllocMemoryFailSimulateFreq = 0;
    m_mosAllocMemoryFailSimulateHint = 0;
    m_mosAllocMemoryFailSimulateAllocCounter = 0;

    // Read Config : memory allocation failure simulate mode
    MosZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
    UserFeatureValue.u32Data = 0;  // Init as default value
    UserFeature.Type        = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath       = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues     = &UserFeatureValue;
    UserFeature.uiNumValues = 1;

    MosUserFeatureReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_MODE,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);

    if ((UserFeature.pValues[0].u32Data == MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT) ||
        (UserFeature.pValues[0].u32Data == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM) ||
        (UserFeature.pValues[0].u32Data == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE))
    {
        m_mosAllocMemoryFailSimulateMode = UserFeature.pValues[0].u32Data;
        MOS_OS_NORMALMESSAGE("Init MosSimulateAllocMemoryFailSimulateMode as %d \n ", m_mosAllocMemoryFailSimulateMode);
    }
    else
    {
        m_mosAllocMemoryFailSimulateMode = MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT;
        MOS_OS_NORMALMESSAGE("Invalid Alloc Memory Fail Simulate Mode from config: %d \n ", UserFeature.pValues[0].u32Data);
    }

    // Read Config : memory allocation failure simulate frequence
    MosZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
    UserFeatureValue.u32Data = 0;  // Init as default value
    UserFeature.Type         = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath        = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues      = &UserFeatureValue;
    UserFeature.uiNumValues  = 1;

    MosUserFeatureReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_FREQ,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);

    if ((UserFeature.pValues[0].u32Data >= MIN_MEMORY_ALLOC_FAIL_FREQ) &&
        (UserFeature.pValues[0].u32Data <= MAX_MEMORY_ALLOC_FAIL_FREQ))
    {
        m_mosAllocMemoryFailSimulateFreq = UserFeature.pValues[0].u32Data;
        MOS_OS_NORMALMESSAGE("Init MosSimulateRandomAllocMemoryFailFreq as %d \n ", m_mosAllocMemoryFailSimulateFreq);

        if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM)
        {
            srand((unsigned int)time(nullptr));
        }
    }
    else
    {
        m_mosAllocMemoryFailSimulateFreq = 0;
        MOS_OS_NORMALMESSAGE("Invalid Alloc Memory Fail Simulate Freq from config: %d \n ", UserFeature.pValues[0].u32Data);
    }

    // Read Config : memory allocation failure simulate counter
    MosZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
    UserFeatureValue.u32Data = 0;  // Init as default value
    UserFeature.Type         = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath        = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues      = &UserFeatureValue;
    UserFeature.uiNumValues  = 1;

    MosUserFeatureReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_HINT,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);

    if (UserFeature.pValues[0].u32Data <= m_mosAllocMemoryFailSimulateFreq)
    {
        m_mosAllocMemoryFailSimulateHint = UserFeature.pValues[0].u32Data;
        MOS_OS_NORMALMESSAGE("Init MosAllocMemoryFailSimulateHint as %d \n ", m_mosAllocMemoryFailSimulateHint);
    }
    else
    {
        m_mosAllocMemoryFailSimulateHint = m_mosAllocMemoryFailSimulateFreq;
        MOS_OS_NORMALMESSAGE("Set m_mosAllocMemoryFailSimulateHint as %d since INVALID CONFIG %d \n ", m_mosAllocMemoryFailSimulateHint, UserFeature.pValues[0].u32Data);
    }
}

bool MosUtilities::MosSimulateAllocMemoryFail(
    size_t      size,
    size_t      alignment,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
{
    bool  bSimulateAllocFail = false;

    if (!MosAllocMemoryFailSimulationEnabled)
    {
        return false;
    }

    if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM)
    {
        int32_t Rn = rand();
        m_mosAllocMemoryFailSimulateAllocCounter++;
        if (Rn % m_mosAllocMemoryFailSimulateFreq == 1)
        {
            bSimulateAllocFail = true;
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, \
                "Simulated Allocate Memory Fail (Rn=%d, SimulateAllocCounter=%d) for: functionName: %s, filename: %s, line: %d, size: %d, alignment: %d \n", \
                Rn, m_mosAllocMemoryFailSimulateAllocCounter, functionName, filename, line, size, alignment);
        }
        else
        {
            bSimulateAllocFail = false;
        }
    }
    else if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE)
    {
        if (m_mosAllocMemoryFailSimulateAllocCounter++ == m_mosAllocMemoryFailSimulateHint)
        {
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, \
                "Simulated Allocate Memory Fail (hint=%d) for: functionName: %s, filename: %s, line: %d, size: %d \n", \
                 m_mosAllocMemoryFailSimulateAllocCounter, functionName, filename, line, size, alignment);
            bSimulateAllocFail = true;
        }
        else
        {
            bSimulateAllocFail = false;
        }
    }
    else
    {
        MOS_OS_NORMALMESSAGE("Invalid MosAllocMemoryFailSimulateMode: %d \n ", m_mosAllocMemoryFailSimulateMode);
        bSimulateAllocFail = false;
    }

    return bSimulateAllocFail;
}
#endif  // #if (_DEBUG || _RELEASE_INTERNAL)


MOS_STATUS MosUtilities::MOS_utilities_init()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MOS_OS_Utilities_Init();

#if (_DEBUG || _RELEASE_INTERNAL)
    //Initialize MOS simulate random alloc memorflag
    MosInitAllocMemoryFailSimulateFlag();
#endif

    return eStatus;
}

MOS_STATUS MosUtilities::MOS_utilities_close()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MosUtilUserInterfaceNext::MosUtilUserInterfaceClose();

    // MOS_OS_Utilitlies_Close must be called right before end of function
    // Because Memninja will calc mem leak here.
    // Any memory allocation release after MOS_OS_UtilitiesNext_Close() will be treated as mem leak.
    eStatus = MOS_OS_Utilities_Close();

#if (_DEBUG || _RELEASE_INTERNAL)
    //Reset Simulate Alloc Memory Fail flags
    MosInitAllocMemoryFailSimulateFlag();
#endif

    return eStatus;
}

void MosUtilities::MosFreeUserFeatureValueString(PMOS_USER_FEATURE_VALUE_STRING pUserString)
{
    if (pUserString != nullptr)
    {
        if (pUserString->uSize > 0)
        {
            MOS_FreeMemAndSetNull(pUserString->pStringData);
            pUserString->uSize = 0;
        }
    }
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MOS_AlignedAllocMemoryUtils(
    size_t      size,
    size_t      alignment,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void  *MosUtilities::MOS_AlignedAllocMemory(
    size_t  size,
    size_t  alignment)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, alignment, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = _aligned_malloc(size, alignment);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicIncrement(&MosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void MosUtilities::MOS_AlignedFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void MosUtilities::MOS_AlignedFreeMemory(void *ptr)
#endif // MOS_MESSAGES_ENABLED
{
    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicDecrement(&MosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);

        _aligned_free(ptr);
    }
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MOS_AllocMemoryUtils(
    size_t      size,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void *MosUtilities::MOS_AllocMemory(size_t size)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = malloc(size);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicIncrement(&MosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MOS_AllocAndZeroMemoryUtils(
    size_t      size,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void *MosUtilities::MOS_AllocAndZeroMemory(size_t size)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = malloc(size);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosZeroMemory(ptr, size);

        MosAtomicIncrement(&MosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MOS_ReallocMemoryUtils(
    void       *ptr,
    size_t     newSize,
    const char *functionName,
    const char *filename,
    int32_t    line)
#else
void *MosUtilities::MOS_ReallocMemory(
    void       *ptr,
    size_t     newSize)
#endif // MOS_MESSAGES_ENABLED
{
    void *oldPtr = nullptr;
    void *newPtr = nullptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(newSize, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    oldPtr = ptr;
    newPtr = realloc(ptr, newSize);

    MOS_OS_ASSERT(newPtr != nullptr);

    if (newPtr != oldPtr)
    {
        if (oldPtr != nullptr)
        {
            MosAtomicDecrement(&MosMemAllocCounter);
            MOS_MEMNINJA_FREE_MESSAGE(oldPtr, functionName, filename, line);
        }

        if (newPtr != nullptr)
        {
            MosAtomicIncrement(&MosMemAllocCounter);
            MOS_MEMNINJA_ALLOC_MESSAGE(newPtr, newSize, functionName, filename, line);
        }
    }

    return newPtr;
}

//!
//! \brief    Wrapper for free(). Performs error checking.
//! \details  Wrapper for free(). Performs error checking.
//!           It decreases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    void  *ptr
//!           [in] Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MosUtilities::MOS_FreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void MosUtilities::MOS_FreeMemory(void  *ptr)
#endif // MOS_MESSAGES_ENABLED
{
    if(ptr != nullptr)
    {
        MosAtomicDecrement(&MosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);

        free(ptr);
    }
}

void MosUtilities::MosZeroMemory(void  *pDestination, size_t stLength)
{
    MOS_OS_ASSERT(pDestination != nullptr);

    if(pDestination != nullptr)
    {
        memset(pDestination, 0, stLength);
    }
}

void MosUtilities::MosFillMemory(void  *pDestination, size_t stLength, uint8_t bFill)
{
    MOS_OS_ASSERT(pDestination != nullptr);

    if(pDestination != nullptr)
    {
        memset(pDestination, bFill, stLength);
    }
}

MOS_STATUS  MosUtilities::MosReadFileToPtr(
    const char      *pFilename,
    uint32_t        *lpNumberOfBytesRead,
    void            **ppReadBuffer)
{
    HANDLE          hFile;
    void            *lpBuffer;
    uint32_t        fileSize;
    uint32_t        bytesRead;
    MOS_STATUS      eStatus;

    *ppReadBuffer = nullptr;
    *lpNumberOfBytesRead = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_RDONLY);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    eStatus = MosGetFileSize(hFile, &fileSize, nullptr);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get size of file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    lpBuffer = MOS_AllocAndZeroMemory(fileSize);
    if (lpBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
        MosCloseHandle(hFile);
        return MOS_STATUS_NO_SPACE;
    }

    if((eStatus = MosReadFile(hFile, lpBuffer, fileSize, &bytesRead, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to read from file '%s'.", pFilename);
        MosCloseHandle(hFile);
        MOS_FreeMemory(lpBuffer);
        lpBuffer = nullptr;
        return eStatus;
    }

    MosCloseHandle(hFile);
    *lpNumberOfBytesRead = bytesRead;
    *ppReadBuffer = lpBuffer;
    return eStatus;
}

MOS_STATUS MosUtilities::MosWriteFileFromPtr(
    const char      *pFilename,
    void            *lpBuffer,
    uint32_t        writeSize)
{
    HANDLE          hFile;
    uint32_t        bytesWritten;
    MOS_STATUS      eStatus;

    MOS_OS_CHK_NULL(pFilename);
    MOS_OS_CHK_NULL(lpBuffer);

    if (writeSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("Attempting to write 0 bytes to a file");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    bytesWritten    = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_WRONLY|O_CREAT);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        goto finish;
    }

    if((eStatus = MosWriteFile(hFile, lpBuffer, writeSize, &bytesWritten, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", pFilename);
        MosCloseHandle(hFile);
        goto finish;
    }

    MosCloseHandle(hFile);

finish:
    return eStatus;
}

MOS_STATUS MosUtilities::MosAppendFileFromPtr(
    const char      *pFilename,
    void            *pData,
    uint32_t        dwSize)
{
    MOS_STATUS  eStatus;
    HANDLE      hFile;
    uint32_t    dwWritten;

    //------------------------------
    MOS_OS_ASSERT(pFilename);
    MOS_OS_ASSERT(pData);
    //------------------------------
    dwWritten   = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_WRONLY | O_CREAT | O_APPEND);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    eStatus = MosSetFilePointer(hFile, 0, nullptr, SEEK_END);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set file pointer'%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    // Write the file
    if((eStatus = MosWriteFile(hFile, pData, dwSize, &dwWritten, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    MosCloseHandle(hFile);
    return eStatus;
}

/*****************************************************************************
|
|                           USER FEATURE Functions
|
*****************************************************************************/

#if (_DEBUG || _RELEASE_INTERNAL)

MOS_FUNC_EXPORT MOS_STATUS MOS_EXPORT_DECL MosUtilities::DumpUserFeatureKeyDefinitionsMedia()
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    // Init MOS User Feature Key from mos desc table
    MOS_OS_CHK_STATUS( MosDeclareUserFeatureKeysForAllDescFields() );
    MOS_OS_CHK_STATUS( MosGenerateUserFeatureKeyXML() );
finish:
    return    eStatus;
}

#endif

MOS_STATUS MosUtilities::MosWriteOneUserFeatureKeyToXML(MOS_USER_FEATURE_VALUE_MAP *keyValueMap, PMOS_USER_FEATURE_VALUE pUserFeature)
{

    char                            sOutBuf[MOS_USER_CONTROL_MAX_DATA_SIZE];
    char                            ValueType[MAX_USER_FEATURE_FIELD_LENGTH];
    char                            KeyPath[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_UNUSED(keyValueMap);
    MOS_OS_ASSERT(pUserFeature);
    //------------------------------

    switch (pUserFeature->Type)
    {
    case MOS_USER_FEATURE_TYPE_USER:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "UFINT\\%s",
            pUserFeature->pcPath);
        break;
    case MOS_USER_FEATURE_TYPE_SYSTEM:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "UFEXT\\%s",
            pUserFeature->pcPath);
        break;
    default:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "%s",pUserFeature->pcPath);
        break;
     }

    switch (pUserFeature->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "bool");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "dword");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "qword");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "string");
        break;
    default:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "unknown");
        break;
     }

    memset(
        sOutBuf,
        0,
        sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "    <Key name=\"%s\" type=\"%s\" location=\"%s\" defaultval=\"%s\" description=\"%s\" />\n",
        pUserFeature->pValueName,
        ValueType,
        KeyPath,
        pUserFeature->DefaultValue,
        pUserFeature->pcDescription);
    MosAppendFileFromPtr(
        m_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return eStatus;
}

MOS_STATUS MosUtilities::MosWriteOneUserFeatureGroupToXML(MOS_USER_FEATURE_VALUE   UserFeatureFilter)
{
    char                                sOutBuf[MAX_USER_FEATURE_FIELD_LENGTH];
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    // Group Header Start
    memset(
        sOutBuf,
        0,
        sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "  <Group name=\"%s\">\n",
        UserFeatureFilter.pcGroup);
    eStatus = MosAppendFileFromPtr(
        m_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));

    // Group User Feature Keys
    eStatus = MosGetItemFromMOSUserFeatureDescField(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        m_userFeatureKeysMap,
        &MosWriteOneUserFeatureKeyToXML,
        &UserFeatureFilter);

    // Group Header End
    memset(
        sOutBuf,
        0,
        sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "  </Group>\n",
        UserFeatureFilter.pcGroup);
    eStatus = MosAppendFileFromPtr(
        m_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return eStatus;
}

MOS_STATUS MosUtilities::MosGenerateUserFeatureKeyXML()
{
    char                                sOutBuf[MAX_USER_FEATURE_FIELD_LENGTH];
    uint32_t                            uiIndex=0;
    MOS_USER_FEATURE_VALUE              UserFeatureFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    const char * const                  FilterGroups[] = { "Codec", "Decode", "Encode", "CP",
                                                           "General", "MOS", "Report", "VP"};
    uint32_t                            FilterGroupsCount = sizeof(FilterGroups) / sizeof(FilterGroups[0]);
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    // Check if XML dump is enabled by User Feature Key
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUserFeatureReadValueID(
                    nullptr,
                    __MOS_USER_FEATURE_KEY_XML_AUTOGEN_ID,
                    &UserFeatureData);
    if (UserFeatureData.u32Data == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = m_xmlFilePath;
    eStatus = MosUserFeatureReadValueID(
                    nullptr,
                    __MOS_USER_FEATURE_KEY_XML_FILEPATH_ID,
                    &UserFeatureData);
    // User Feature Key Header Start
    memset(sOutBuf, 0, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "<UserFeatureKeys>\n");
    eStatus = MosWriteFileFromPtr(
        UserFeatureData.StringData.pStringData,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    // User Feature Key Groups
    for (uiIndex = 0; uiIndex < FilterGroupsCount; uiIndex++)
    {
        UserFeatureFilter.pcGroup = FilterGroups[uiIndex];
        eStatus = MosWriteOneUserFeatureGroupToXML(UserFeatureFilter);
    }

    // User Feature Key Header End
    memset(sOutBuf, 0, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "</UserFeatureKeys>\n");
    eStatus = MosAppendFileFromPtr(
        UserFeatureData.StringData.pStringData,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return    eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureSetMultiStringValue(
    PMOS_USER_FEATURE_VALUE_DATA     pFeatureData,
    uint32_t                         dwSize)
{
    PMOS_USER_FEATURE_VALUE_STRING  pStrings;
    uint32_t                        uiNumStrings;
    uint32_t                        ui;
    char                            *pData;
    char                            *pCurData;
    uint32_t                        dwLen;
    uint32_t                        dwPos;

    MOS_OS_ASSERT(pFeatureData);
    MOS_OS_ASSERT(dwSize);

    pStrings = pFeatureData->MultiStringData.pStrings;
    pData = pFeatureData->MultiStringData.pMultStringData;
    dwPos = 0;
    uiNumStrings = 0;

    MOS_OS_ASSERT(pStrings);
    MOS_OS_ASSERT(pData);

    // Find number of strings in the multi string array
    do
    {
        pCurData = pData + dwPos;
        dwLen = (uint32_t)strlen(pCurData);
        if (dwLen == 0)
        {
            MOS_OS_NORMALMESSAGE("Invalid user feature key entry.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        uiNumStrings++;
        dwPos += dwLen + 1;

        if (dwPos >= (dwSize - 1))
        {
            // last entry
            break;
        }
    } while (true);

    // Check the size of MultiStringData
    if (pFeatureData->MultiStringData.uCount < uiNumStrings)
    {
        MOS_OS_NORMALMESSAGE("pFeatureValue->MultiStringData.uCount is smaller than the actual necessary number.");
        return MOS_STATUS_UNKNOWN;
    }

    // Populate Array
    dwPos = 0;
    for (ui = 0; ui < uiNumStrings; ui++)
    {
        pCurData = pData + dwPos;
        dwLen = (uint32_t)strlen(pCurData);
        MOS_OS_ASSERT(dwLen > 0);
        pStrings[ui].pStringData = pCurData;
        pStrings[ui].uSize = dwLen;

        dwPos += dwLen + 1;
    }

    pFeatureData->MultiStringData.uCount = uiNumStrings;
    pFeatureData->MultiStringData.uSize = dwPos;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosCopyUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA pSrcData,
    PMOS_USER_FEATURE_VALUE_DATA pDstData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                            ui;
    PMOS_USER_FEATURE_VALUE_STRING      pSrcString = nullptr;
    PMOS_USER_FEATURE_VALUE_STRING      pDstString = nullptr;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_ASSERT(pSrcData);
    MOS_OS_ASSERT(pDstData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------

    switch(ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        pDstData->bData = pSrcData->bData;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        pDstData->i32Data = pSrcData->i32Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        pDstData->i64Data = pSrcData->i64Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        pDstData->u32Data = pSrcData->u32Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        pDstData->u64Data = pSrcData->u64Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        pDstData->fData = pSrcData->fData;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        if ((pSrcData->StringData.pStringData != nullptr) && (strlen(pSrcData->StringData.pStringData) != 0))
        {
            pDstData->StringData.uMaxSize = pSrcData->StringData.uMaxSize;
            pDstData->StringData.uSize = pSrcData->StringData.uSize;
            if (pDstData->StringData.pStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureMemcpy(
                pDstData->StringData.pStringData,
                pDstData->StringData.uSize,
                pSrcData->StringData.pStringData,
                pSrcData->StringData.uSize);

            MOS_SafeFreeMemory(pSrcData->StringData.pStringData);
            pSrcData->StringData.pStringData = nullptr;
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        if ((pSrcData->MultiStringData.pMultStringData != nullptr) && (strlen(pSrcData->MultiStringData.pMultStringData) != 0))
        {
            pDstData->MultiStringData.uCount = pSrcData->MultiStringData.uCount;
            pDstData->MultiStringData.uMaxSize = pSrcData->MultiStringData.uMaxSize;
            pDstData->MultiStringData.uSize = pSrcData->MultiStringData.uSize;
            if (pDstData->MultiStringData.pMultStringData != nullptr)
            {
                eStatus = MosSecureMemcpy(
                    pDstData->MultiStringData.pMultStringData,
                    pDstData->MultiStringData.uSize,
                    pSrcData->MultiStringData.pMultStringData,
                    pSrcData->MultiStringData.uSize);

                for (ui = 0; ui < pSrcData->MultiStringData.uCount; ui++)
                {
                    pSrcString = &pSrcData->MultiStringData.pStrings[ui];
                    pDstString = &pDstData->MultiStringData.pStrings[ui];

                    MOS_OS_CHK_NULL(pSrcString);
                    MOS_OS_CHK_NULL(pDstString);
                    pDstString->uMaxSize = pSrcString->uMaxSize;
                    pDstString->uSize = pSrcString->uSize;
                    if (pDstString->pStringData != nullptr)
                    {
                        eStatus = MosSecureMemcpy(
                            pDstString->pStringData,
                            pDstString->uSize+1,
                            pSrcString->pStringData,
                            pSrcString->uSize+1);
                    }// if
                }// for
            }
        }// if
        break;
    default:
        break;
    }
finish:
    return eStatus;
}

MOS_STATUS MosUtilities::MosAssignUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA    pDstData,
    const char                      *pData,
    MOS_USER_FEATURE_VALUE_TYPE     ValueType
)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    uint32_t                        dwUFSize = 0;

    //------------------------------
    MOS_OS_ASSERT(pData);
    MOS_OS_ASSERT(pDstData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------

    switch(ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        pDstData->bData = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        pDstData->i32Data = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        pDstData->i64Data = atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        pDstData->u32Data = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        pDstData->u64Data = atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        pDstData->fData = (float)atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        pDstData->StringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
        if ((pData != nullptr) && (strlen(pData) != 0))
        {
            pDstData->StringData.uSize = (uint32_t)strlen(pData);
            if (pDstData->StringData.uSize > pDstData->StringData.uMaxSize)
            {
                pDstData->StringData.uSize = pDstData->StringData.uMaxSize;
            }
            pDstData->StringData.pStringData = (char *)MOS_AllocAndZeroMemory(strlen(pData) + 1);
            if (pDstData->StringData.pStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureStrcpy(
                pDstData->StringData.pStringData,
                pDstData->StringData.uSize + 1,
                (char *)pData);
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:

        pDstData->MultiStringData.uCount = MOS_USER_MAX_STRING_COUNT;
        pDstData->MultiStringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
        pDstData->MultiStringData.pStrings = (PMOS_USER_FEATURE_VALUE_STRING)MOS_AllocAndZeroMemory(sizeof(MOS_USER_FEATURE_VALUE_STRING) * __MAX_MULTI_STRING_COUNT);
        if (pDstData->MultiStringData.pStrings == nullptr)
              {
                    MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                    return MOS_STATUS_NULL_POINTER;
              }
        if ((pData != nullptr) && (strlen(pData) != 0))
        {
            MOS_SafeFreeMemory(pDstData->MultiStringData.pMultStringData);
            pDstData->MultiStringData.pMultStringData = (char *)MOS_AllocAndZeroMemory(strlen(pData) + 1);
            if (pDstData->MultiStringData.pMultStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                pDstData->MultiStringData.pMultStringData = nullptr;
                pDstData->MultiStringData.uSize = 0;
                pDstData->MultiStringData.uCount = 0;
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureMemcpy(
                pDstData->MultiStringData.pMultStringData,
                strlen(pData),
                (char *)pData,
                strlen(pData));
            if ((eStatus = MosUserFeatureSetMultiStringValue(
                pDstData,
                dwUFSize)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to set multi string value.");
                return eStatus;
            }
        }
        break;
    default:
        break;
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKey(MOS_USER_FEATURE_VALUE_MAP *keyValueMap, PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_ASSERT(pUserFeatureKey);
    //------------------------------

    if (pUserFeatureKey->pfnSetDefaultValueData!=NULL)
    {
        eStatus = pUserFeatureKey->pfnSetDefaultValueData(&pUserFeatureKey->Value);
    }
    else
    {
        eStatus = MosAssignUserFeatureValueData(
            &pUserFeatureKey->Value,
            pUserFeatureKey->DefaultValue,
            pUserFeatureKey->ValueType);
    }
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        if (keyValueMap)
        {
            keyValueMap[pUserFeatureKey->ValueID].pUserFeatureValue = pUserFeatureKey; // legacy path, keep for compatibilty temporally
        }
#ifdef __cplusplus
        MosUtilUserInterfaceNext::AddEntry(pUserFeatureKey->ValueID, pUserFeatureKey);
#endif
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureData(PMOS_USER_FEATURE_VALUE_DATA pData,MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                    ui;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    if (pData == nullptr)
    {
        return eStatus;
    }
    //------------------------------

    switch (ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        MosFreeUserFeatureValueString(&pData->StringData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        for (ui = 0; ui < pData->MultiStringData.uCount; ui++)
        {
            MosFreeUserFeatureValueString(&pData->MultiStringData.pStrings[ui]);
        }
        MOS_SafeFreeMemory(pData->MultiStringData.pStrings);
        pData->MultiStringData.pStrings = nullptr;
        pData->MultiStringData.pMultStringData = nullptr;
        pData->MultiStringData.uSize = 0;
        pData->MultiStringData.uCount = 0;
        break;
    default:
        break;
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKey(MOS_USER_FEATURE_VALUE_MAP *keyValueMap, PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_ASSERT(pUserFeatureKey);
    //------------------------------

#ifdef __cplusplus
    MosUtilUserInterfaceNext::DelEntry(pUserFeatureKey->ValueID);
#endif
    if (keyValueMap)
    {
        keyValueMap[pUserFeatureKey->ValueID].pUserFeatureValue = nullptr; // keep legacy path temporally for compatibility
    }

    eStatus = MosDestroyUserFeatureData(
        &pUserFeatureKey->Value,
        pUserFeatureKey->ValueType);

    return eStatus;
}

MOS_STATUS MosUtilities::MosIsCorrectDefaultValueType(
    const char                  *pData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                    dwLen;
    uint32_t                    ui;
    int32_t                     IntVal;
    MOS_STATUS                  eStatus = MOS_STATUS_INVALID_PARAMETER;

    dwLen = (uint32_t)strlen(pData);
    //------------------------------
    MOS_OS_ASSERT(pData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------
    switch (ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        if ((!strcmp(pData, "0")) || (!strcmp(pData, "1")))
        {
            eStatus = MOS_STATUS_SUCCESS;
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        eStatus = MOS_STATUS_SUCCESS;
        for (ui = 0; ui<dwLen; ui++)
        {
            IntVal = pData[ui] - '0';
            if ((0 > IntVal) || (9 < IntVal))
            {
                if ((((ui == 0)&&(pData[ui] - '-') != 0)) && ((pData[ui] - '.') != 0))
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        eStatus = MOS_STATUS_SUCCESS;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        eStatus = MOS_STATUS_SUCCESS;
        break;
    default:
        break;
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosIsCorrectUserFeatureDescField(PMOS_USER_FEATURE_VALUE pUserFeatureKey, uint32_t maxKeyID)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_ASSERT(pUserFeatureKey);
    //------------------------------
    if ((pUserFeatureKey->ValueID <= __MOS_USER_FEATURE_KEY_INVALID_ID) ||
        (pUserFeatureKey->ValueID >= maxKeyID))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    if (pUserFeatureKey->pValueName == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    if (pUserFeatureKey->pcPath == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    if (pUserFeatureKey->pcWritePath == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    if (pUserFeatureKey->pcGroup == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    if ((pUserFeatureKey->pcDescription != nullptr) &&
        (strlen(pUserFeatureKey->pcDescription) > MAX_USER_FEATURE_FIELD_LENGTH))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
     eStatus = MosIsCorrectDefaultValueType(
         pUserFeatureKey->DefaultValue,
         pUserFeatureKey->ValueType);
    return eStatus;
}

MOS_STATUS MosUtilities::MosGetItemFromMOSUserFeatureDescField(
    MOS_USER_FEATURE_VALUE      *descTable,
    uint32_t                    numOfItems,
    uint32_t                    maxId,
    MOS_USER_FEATURE_VALUE_MAP  *keyValueMap,
    MOS_STATUS                  (*CallbackFunc)(MOS_USER_FEATURE_VALUE_MAP *, PMOS_USER_FEATURE_VALUE),
    PMOS_USER_FEATURE_VALUE     pUserFeatureKeyFilter)
{
    uint32_t  uiIndex = 0;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_ASSERT(CallbackFunc);
    MOS_OS_ASSERT(pUserFeatureKeyFilter);
    //MOS_OS_CHK_NULL_RETURN(descTable);
    //------------------------------

    for (uiIndex = __MOS_USER_FEATURE_KEY_INVALID_ID; uiIndex < numOfItems; uiIndex++)
    {
        if (MosIsCorrectUserFeatureDescField(&descTable[uiIndex], maxId) != MOS_STATUS_SUCCESS)
        {
            continue;
        }

        if ((pUserFeatureKeyFilter->ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID) && (pUserFeatureKeyFilter->ValueID != descTable[uiIndex].ValueID))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pValueName != nullptr) && (strcmp(pUserFeatureKeyFilter->pValueName, descTable[uiIndex].pValueName) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcPath != nullptr) && (strcmp(pUserFeatureKeyFilter->pcPath, descTable[uiIndex].pcPath) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcWritePath != nullptr) && (strcmp(pUserFeatureKeyFilter->pcWritePath, descTable[uiIndex].pcWritePath) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcGroup != nullptr) && (strcmp(pUserFeatureKeyFilter->pcGroup, descTable[uiIndex].pcGroup) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->Type != MOS_USER_FEATURE_TYPE_INVALID) && (pUserFeatureKeyFilter->Type != descTable[uiIndex].Type))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID) && (pUserFeatureKeyFilter->ValueType != descTable[uiIndex].ValueType))
        {
            continue;
        }
        eStatus = (*CallbackFunc)(keyValueMap, &descTable[uiIndex]);

    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId,
    MOS_USER_FEATURE_VALUE_MAP *keyValueMap)
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    eStatus = MosGetItemFromMOSUserFeatureDescField(
        descTable,
        numOfItems,
        maxId,
        keyValueMap,
        &MosDeclareUserFeatureKey,
        &UserFeatureKeyFilter);
    return eStatus;
}

//!
//! \brief    Link the m_mosUserFeatureDescFields table items to m_userFeatureKeysMap
//! \details  Link the m_mosUserFeatureDescFields table items to m_userFeatureKeysMap
//!           according to ID sequence and do some post processing such as malloc related memory
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysForAllDescFields()
{
    MOS_OS_CHK_STATUS_RETURN(MosDeclareUserFeatureKeysFromDescFields(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        m_userFeatureKeysMap));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId,
    MOS_USER_FEATURE_VALUE_MAP *keyValueMap)
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    eStatus = MosGetItemFromMOSUserFeatureDescField(
        descTable,
        numOfItems,
        maxId,
        keyValueMap,
        &MosDestroyUserFeatureKey,
        &UserFeatureKeyFilter);
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysForAllDescFields()
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_STATUS_RETURN(MosDestroyUserFeatureKeysFromDescFields(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        m_userFeatureKeysMap));

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueInit(
    PMOS_USER_FEATURE_INTERFACE   pOsUserFeatureInterface,
    uint32_t                      uiNumValues,
    MOS_USER_FEATURE_VALUE_DATA   Value,
    const char                    *pValueName,
    MOS_USER_FEATURE_VALUE_TYPE   ValueType)
{
    MOS_STATUS                  eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);
    MOS_UNUSED(Value);
    MOS_UNUSED(pValueName);
    MOS_UNUSED(ValueType);

    eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_ASSERT(pValueName);
    //------------------------------
    // Check if memory is allocated
    if (uiNumValues == 0)
    {
        MOS_OS_ASSERTMESSAGE("pUserFeature->uiNumValues is 0.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

finish:
    return eStatus;
}

void MosUtilities::MosUserFeatureCallback(
        PTP_CALLBACK_INSTANCE Instance,
        void                  *pvParameter,
        PTP_WAIT              Wait,
        TP_WAIT_RESULT        WaitResult)
{
    PMOS_USER_FEATURE_NOTIFY_DATA  pNotifyData;
    MOS_UNUSED(Instance);
    MOS_UNUSED(Wait);
    MOS_UNUSED(WaitResult);

    MOS_OS_ASSERT(pvParameter);

    pNotifyData = (PMOS_USER_FEATURE_NOTIFY_DATA)pvParameter;
    pNotifyData->bTriggered = true;
}

MOS_STATUS MosUtilities::MosUserFeatureOpen(
    MOS_USER_FEATURE_TYPE KeyType,
    const char            *pSubKey,
    uint32_t              dwAccess,
    void                  **pUFKey)
{
    MOS_STATUS  eStatus;
    void        *RootKey = 0;

    MOS_OS_ASSERT(pSubKey);
    MOS_OS_ASSERT(pUFKey);

    if (KeyType == MOS_USER_FEATURE_TYPE_USER)
    {
        RootKey = (void *)UFKEY_INTERNAL;
    }
    else if (KeyType == MOS_USER_FEATURE_TYPE_SYSTEM)
    {
        RootKey = (void *)UFKEY_EXTERNAL;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Invalid Key Type %d.", KeyType);
        return MOS_STATUS_UNKNOWN;
    }

    if((eStatus = MosUserFeatureOpenKey(
                             RootKey,
                             pSubKey,
                             0,
                             dwAccess,
                             pUFKey)) !=  MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to open user feature key %s.", pSubKey);
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueBinary(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    void        *pvData;
    uint32_t    dwUFSize;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_BINARY);

    pvData = pFeatureValue->Value.BinaryData.pBinaryData;
    if (!pvData)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->BinaryData.pBinaryData is NULL.");
        return MOS_STATUS_NULL_POINTER;
    }

    dwUFSize = pFeatureValue->Value.BinaryData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->BinaryData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_BINARY,
                  nullptr,
                  pvData,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.BinaryData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.BinaryData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read binary user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }

    pFeatureValue->Value.BinaryData.uSize = dwUFSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFSize;
    char        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];

    //--------------------------------------------------
    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_STRING);
    //--------------------------------------------------

    MosZeroMemory(pcTmpStr, MOS_USER_CONTROL_MAX_DATA_SIZE);
    dwUFSize = pFeatureValue->Value.StringData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->StringData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_SZ,
                  nullptr,
                  pcTmpStr,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.StringData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.StringData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read single string user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }
    if (strlen(pcTmpStr) > 0)
    {
        pFeatureValue->Value.StringData.pStringData = (char *)MOS_AllocAndZeroMemory(strlen(pcTmpStr) + 1);

        MosSecureMemcpy(pFeatureValue->Value.StringData.pStringData, strlen(pcTmpStr), pcTmpStr, strlen(pcTmpStr));
        pFeatureValue->Value.StringData.uSize = dwUFSize;
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueMultiString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFSize;
    char        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING);

    if (!pFeatureValue->Value.MultiStringData.pStrings)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->MultiStringData.pStrings is NULL.");
        return MOS_STATUS_NULL_POINTER;
    }
    MosZeroMemory(pcTmpStr, MOS_USER_CONTROL_MAX_DATA_SIZE);
    dwUFSize = pFeatureValue->Value.MultiStringData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->MultiStringData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_MULTI_SZ,
                  nullptr,
                  pcTmpStr,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.MultiStringData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.MultiStringData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read single string user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }

    if (strlen(pcTmpStr) > 0)
    {
        MOS_SafeFreeMemory(pFeatureValue->Value.MultiStringData.pMultStringData);
        pFeatureValue->Value.MultiStringData.pMultStringData = (char *)MOS_AllocAndZeroMemory(strlen(pcTmpStr) + 1);
        MosMemAllocFakeCounter++;
        if (pFeatureValue->Value.MultiStringData.pMultStringData == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NULL_POINTER;
        }
        MosSecureMemcpy(
            pFeatureValue->Value.MultiStringData.pMultStringData,
            strlen(pcTmpStr),
            pcTmpStr,
            strlen(pcTmpStr));

    if((eStatus = MosUserFeatureSetMultiStringValue(
            &pFeatureValue->Value,
        dwUFSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set multi string value.");
        return eStatus;
    }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValuePrimitive(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFType = 0;
    uint32_t    dwUFSize;
    void        *pvData = nullptr;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);

    switch(pFeatureValue->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        dwUFType    = RRF_RT_UF_DWORD;
        dwUFSize    = sizeof(uint32_t);
        pvData      = &pFeatureValue->Value.fData;
        break;

    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        dwUFType    = RRF_RT_UF_QWORD;
        dwUFSize    = sizeof(uint64_t);
        pvData      = &pFeatureValue->Value.u64Data;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Invalid primitive value type.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  dwUFType,
                  nullptr,
                  pvData,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // This error case can be hit if the user feature key does not exist.
        MOS_OS_NORMALMESSAGE("Failed to read primitive user feature value \"%s\".", pFeatureValue->pValueName);
        return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS          eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_STRING);
    MOS_OS_ASSERT(pDataValue);

    if((eStatus = MosUserFeatureSetValueEx(
                      UFKey,
                      pFeatureValue->pValueName,
                      0,
                      UF_SZ,
                      (uint8_t*)pDataValue->StringData.pStringData,
                      pDataValue->StringData.uSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write string user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueMultiString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    PMOS_USER_FEATURE_VALUE_STRING  pStringData;
    uint8_t                         *pData;
    uint8_t                         *pCurData;
    uint32_t                        dwDataSize;
    uint32_t                        dwAvailableSize;
    uint32_t                        ui;
    MOS_STATUS                      eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING);
    MOS_OS_ASSERT(pDataValue);
    MOS_OS_ASSERT(pDataValue->MultiStringData.uCount > 0);

    pData       = nullptr;
    dwDataSize  = 0;

    for (ui = 0; ui < pDataValue->MultiStringData.uCount; ui++)
    {
        pStringData = &pDataValue->MultiStringData.pStrings[ui];
        dwDataSize += pStringData->uSize;
        dwDataSize += 1;                                                        // for \0
    }
    dwDataSize += 1;                                                            // for \0 at the very end (see MULTI_SZ spec)

    // Allocate memory to store data
    pData = (uint8_t*)MOS_AllocAndZeroMemory(dwDataSize);
    if(pData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
        return MOS_STATUS_NO_SPACE;
    }

    // Copy data from original string array
    pCurData        = pData;
    dwAvailableSize = dwDataSize;
    for (ui = 0; ui < pDataValue->MultiStringData.uCount; ui++)
    {
        pStringData = &pDataValue->MultiStringData.pStrings[ui];
        eStatus = MosSecureMemcpy(pCurData, dwAvailableSize, pStringData->pStringData, pStringData->uSize);
        if(eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to copy memory.");
            goto finish;
        }
        pCurData += pStringData->uSize;
        pCurData++;                                                             // \0 is already added since we zeroed the memory
                                                                                // Very last \0 is already added since we zeroed the memory
        dwAvailableSize -= pStringData->uSize + 1;
    }
    // Write the user feature MULTI_SZ entry
    if((eStatus = MosUserFeatureSetValueEx(
                       UFKey,
                       pFeatureValue->pValueName,
                       0,
                       UF_MULTI_SZ,
                       pData,
                       dwDataSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write multi string user feature value.");
    }

finish:
    MOS_FreeMemory(pData);
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueBinary(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS      eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_BINARY);
    MOS_OS_ASSERT(pDataValue);

    if((eStatus = MosUserFeatureSetValueEx(
                       UFKey,
                       pFeatureValue->pValueName,
                       0,
                       UF_BINARY,
                       (uint8_t*)pDataValue->BinaryData.pBinaryData,
                       pDataValue->BinaryData.uSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write binary user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuePrimitive(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFType = UF_NONE;
    uint32_t    dwUFSize = 0;
    void        *pvData = nullptr;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    MOS_OS_ASSERT(pDataValue);

    switch(pFeatureValue->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        dwUFType    = UF_DWORD;
        dwUFSize    = sizeof(uint32_t);
        pvData      = &pDataValue->fData;
        break;

    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        dwUFType    = UF_QWORD;
        dwUFSize    = sizeof(uint64_t);
        pvData      = &pDataValue->u64Data;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Invalid primitive value type.");
        return MOS_STATUS_UNKNOWN;
    }

    if((eStatus = MosUserFeatureSetValueEx(
                        UFKey,
                        pFeatureValue->pValueName,
                        0,
                        dwUFType,
                        (uint8_t*)pvData,
                        dwUFSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write primitive user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValue (
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    PMOS_USER_FEATURE               pUserFeature,
    const char                      *pValueName,
    MOS_USER_FEATURE_VALUE_TYPE     ValueType)
{
    uint32_t                                ui;
    void                                    *UFKey = nullptr;
    PMOS_USER_FEATURE_VALUE                 pSettingsValue = nullptr;
    MOS_USER_FEATURE_VALUE_DATA             UserFeatureData;
    MOS_USER_FEATURE_VALUE_ID               ValueID = __MOS_USER_FEATURE_KEY_INVALID_ID;
    PMOS_USER_FEATURE_VALUE                 pUserFeatureValue=NULL;
    PMOS_USER_FEATURE_VALUE_STRING          pSrcString = nullptr;
    PMOS_USER_FEATURE_VALUE_STRING          pDstString = nullptr;
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(pOsUserFeatureInterface);

    //--------------------------------------------------
    // NOTE: Please refer to the function details for the impact of pOsUserFeatureInterface here.
    MOS_OS_ASSERT(pUserFeature);
    MOS_OS_ASSERT(pValueName);
    MOS_OS_ASSERT(pUserFeature->pPath);
    MOS_OS_ASSERT(pUserFeature->pValues);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //--------------------------------------------------

    //--------------------------------------------------
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));

    for (ui = (__MOS_USER_FEATURE_KEY_INVALID_ID+1); ui < __MOS_USER_FEATURE_KEY_MAX_ID; ui++)
    {
        if (m_userFeatureKeysMap[ui].pUserFeatureValue == nullptr)
        {
            continue;
        }
        pUserFeatureValue = m_userFeatureKeysMap[ui].pUserFeatureValue;
        if (pUserFeatureValue->pValueName!=NULL)
        {
            if(!strcmp(pValueName,pUserFeatureValue->pValueName))
            {
                ValueID = (MOS_USER_FEATURE_VALUE_ID)ui;
                break;
            }// if
        }// if
    }// for

    if (ValueID == __MOS_USER_FEATURE_KEY_INVALID_ID)
    {
        return  MOS_STATUS_UNKNOWN;
    }

    pUserFeatureValue->uiNumOfValues = pUserFeature->uiNumValues;

    //--------------------------------------------------
    // pStringData pre-allocation
    UserFeatureData.StringData.pStringData = pUserFeature->pValues->StringData.pStringData;

    eStatus = MosUserFeatureReadValueID(
        nullptr,
        ValueID,
        &UserFeatureData);

    //--------------------------------------------------
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        // Copy pUserFeatureValue into pUserFeature->pValue
        switch(ValueType)
        {
            case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
                pUserFeature->pValues->bData = UserFeatureData.bData;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_INT32:
                pUserFeature->pValues->i32Data = UserFeatureData.i32Data;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_INT64:
                pUserFeature->pValues->i64Data = UserFeatureData.i64Data;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
                pUserFeature->pValues->u32Data = UserFeatureData.u32Data;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
                pUserFeature->pValues->u64Data = UserFeatureData.u64Data;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
                pUserFeature->pValues->fData = UserFeatureData.fData;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_STRING:
                pUserFeature->pValues->StringData.uMaxSize = UserFeatureData.StringData.uMaxSize;
                pUserFeature->pValues->StringData.uSize = UserFeatureData.StringData.uSize;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
                eStatus = MOS_STATUS_UNIMPLEMENTED;
                break;
            default:
                break;
        }
    }
    //--------------------------------------------------
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueFromMapID(
    MOS_USER_FEATURE_VALUE_MAP      *keyValueMap,
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData)
{
    void                        *UFKey           = nullptr;
    PMOS_USER_FEATURE_VALUE     pUserFeature    = nullptr;
    int32_t                     iDataFlag       = MOS_USER_FEATURE_VALUE_DATA_FLAG_NONE_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_STATUS                  eStatus         = MOS_STATUS_SUCCESS;

    //--------------------------------------------------
    MOS_OS_ASSERT(pValueData);
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);
    //--------------------------------------------------
    iDataFlag = pValueData->i32DataFlag;
#ifdef __cplusplus
    pUserFeature = MosUtilUserInterfaceNext::GetValue(ValueID);
#else
    if (keyValueMap)
    {
        pUserFeature = keyValueMap[ValueID].pUserFeatureValue;
    }
    else
    {
        MOS_ASSERTMESSAGE("Not supported null key value map in C code");
    }
#endif
    if (nullptr == pUserFeature)
    {
        MOS_OS_NORMALMESSAGE("Cannot found the user feature key.");
        eStatus = MOS_STATUS_NULL_POINTER;
        return eStatus;
    }

    // Open the user feature
    // Assigned the pUserFeature to UFKey for future reading
    UFKey = pUserFeature;
    if((eStatus = MosUserFeatureOpen(
                       pUserFeature->Type,
                       pUserFeature->pcPath,
                       KEY_READ,
                       &UFKey)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Failed to open user feature for reading eStatus:%d.", eStatus);
        eStatus = MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
        goto finish;
    }

    // Initialize Read Value
    if((eStatus = MosUserFeatureReadValueInit(
                        pOsUserFeatureInterface,
                        pUserFeature->uiNumOfValues,
                        pUserFeature->Value,
                        pUserFeature->pValueName,
                        pUserFeature->ValueType)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to initialize user feature read value eStatus:%d.",eStatus);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

#if !(_DEBUG || _RELEASE_INTERNAL)
    // For release build, don't read debug only keys, but return default directly
    if (pUserFeature->EffctiveRange == MOS_USER_FEATURE_EFFECT_DEBUGONLY)
    {
        eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        goto finish;
    }
#endif

    // Read the Values from user feature
    switch(pUserFeature->ValueType)
    {
       case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
           eStatus = MosUserFeatureReadValueBinary(UFKey, pUserFeature);
           break;
       case MOS_USER_FEATURE_VALUE_TYPE_STRING:
           eStatus = MosUserFeatureReadValueString(UFKey, pUserFeature);
           break;
       case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
           eStatus = MosUserFeatureReadValueMultiString(UFKey, pUserFeature);
           break;
       default:
           eStatus = MosUserFeatureReadValuePrimitive(UFKey, pUserFeature);
           break;
    }

    if(eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Failed to read value from user feature eStatus:%d.", eStatus);
        eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }
finish:
    if ((pUserFeature != nullptr) &&
        ((eStatus == MOS_STATUS_SUCCESS) ||
        (iDataFlag != MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE)))
    {
        // Use the User Feature Value or default value in corresponding user feature key Desc Fields
        // when User Feature Key read successfully or no input custom default value
        MosCopyUserFeatureValueData(
            &pUserFeature->Value,
            pValueData,
            pUserFeature->ValueType);
    }
    MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData)
{
    return MosUserFeatureReadValueFromMapID(
        m_userFeatureKeysMap,
        pOsUserFeatureInterface,
        ValueID,
        pValueData);
}

const char* MosUtilities::MosUserFeatureLookupValueName(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

#ifdef __cplusplus
    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterfaceNext::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pValueName;
    }
    else
    {
        return nullptr;
    }
#else
    MOS_OS_ASSERT(m_userFeatureKeysMap[ValueID].pUserFeatureValue != nullptr);
    return m_userFeatureKeysMap[ValueID].pUserFeatureValue->pValueName;
#endif
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesTblID(
    MOS_USER_FEATURE_VALUE_MAP              *keyValueMap,
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues)
{
    uint32_t                            ui;
    PMOS_USER_FEATURE_VALUE             pFeatureValue      = nullptr;
    void                                *UFKey              = nullptr;
    PMOS_USER_FEATURE_VALUE_WRITE_DATA  pUserWriteData     = nullptr;
    PMOS_USER_FEATURE_VALUE             pUserFeature       = nullptr;
    uint32_t                            ValueID            = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_STATUS                          eStatus            = MOS_STATUS_SUCCESS;
    char                                WritePathWithPID[MAX_PATH];
    int32_t                             pid;

    MOS_UNUSED(pOsUserFeatureInterface);
    //--------------------------------------------------
    MOS_OS_ASSERT(pWriteValues);
    //--------------------------------------------------

    MosZeroMemory(WritePathWithPID, MAX_PATH);

    pid = MosGetPid();

    for (ui = 0; ui < uiNumOfValues; ui++)
    {
        ValueID = pWriteValues[ui].ValueID;
#ifdef __cplusplus
        pUserFeature = MosUtilUserInterfaceNext::GetValue(ValueID);
#else
        if (keyValueMap)
        {
            pUserFeature = keyValueMap[ValueID].pUserFeatureValue;
        }
        else
        {
            pUserFeature = nullptr;
        }
#endif
        MOS_OS_CHK_NULL(pUserFeature);
        // Open the user feature
        // Assigned the pUserFeature to UFKey for future reading
        UFKey = pUserFeature;

        //append write path with pid
        sprintf_s(WritePathWithPID, MAX_PATH, "%s\\%d", pUserFeature->pcWritePath, pid);

        //try to open Write path with pid first
        if ((eStatus = MosUserFeatureOpen(
                 pUserFeature->Type,
                 WritePathWithPID,
                 KEY_WRITE,
                 &UFKey)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to open user feature for concurrency.");
            if ((eStatus = MosUserFeatureOpen(
                     pUserFeature->Type,
                     pUserFeature->pcWritePath,
                     KEY_WRITE,
                     &UFKey)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_NORMALMESSAGE("Failed to open user feature for reading.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
                goto finish;
            }
        }

        //------------------------------------
        MOS_OS_ASSERT(pUserFeature->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
        //------------------------------------
        switch(pUserFeature->ValueType)
        {
        case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
            if ((eStatus = MosUserFeatureWriteValueBinary(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write binary value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_STRING:
            if ((eStatus = MosUserFeatureWriteValueString(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write string value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
            if ((eStatus = MosUserFeatureWriteValueMultiString(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write multi string value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        default:
            if ((eStatus = MosUserFeatureWriteValuePrimitive(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write primitive data value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
        }
        MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    }

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues)
{
    return MosUserFeatureWriteValuesTblID(
        m_userFeatureKeysMap,
        pOsUserFeatureInterface,
        pWriteValues,
        uiNumOfValues);
}

MOS_STATUS MosUtilities::MosUserFeatureEnableNotification(
    PMOS_USER_FEATURE_INTERFACE            pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA          pNotification)
{
    PMOS_USER_FEATURE_NOTIFY_DATA_COMMON    pNotifyCommon;
    int32_t                                 bResult;
    MOS_STATUS                              eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);

    //---------------------------------------
    MOS_OS_ASSERT(pNotification);
    MOS_OS_ASSERT(pNotification->NotifyType != MOS_USER_FEATURE_NOTIFY_TYPE_INVALID);
    MOS_OS_ASSERT(pNotification->pPath);
    //---------------------------------------

    // Reset the triggered flag
    pNotification->bTriggered = false;

    if (pNotification->pHandle == nullptr)
    {
        // Allocate private data as well
        pNotification->pHandle = MOS_AllocAndZeroMemory(sizeof(MOS_USER_FEATURE_NOTIFY_DATA));
        if(pNotification->pHandle == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NO_SPACE;
        }
    }
    pNotifyCommon = (PMOS_USER_FEATURE_NOTIFY_DATA_COMMON)pNotification->pHandle;

    // Open User Feature for Reading
    if (pNotifyCommon->UFKey == 0)
    {
        if((eStatus = MosUserFeatureOpen(
                          pNotification->Type,
                          pNotification->pPath,
                          KEY_READ,
                          &pNotifyCommon->UFKey)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to open user feature for reading.");
            return MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
        }
    }

    // Create Event for notification
    if (pNotifyCommon->hEvent == nullptr)
    {
        pNotifyCommon->hEvent = MosCreateEventEx(
                                    nullptr,
                                    nullptr,
                                    0);
        if(pNotifyCommon->hEvent == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NO_SPACE;
        }
    }

    // Unregister wait event if already registered
    if (pNotifyCommon->hWaitEvent)
    {
        if ((bResult = MosUnregisterWaitEx(pNotifyCommon->hWaitEvent)) == false)
        {
            MOS_OS_ASSERTMESSAGE("Unable to unregiser wait event.");
            return MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED;
        }
        pNotifyCommon->hWaitEvent = nullptr;
    }

    // Register a Callback
    if((eStatus = MosUserFeatureNotifyChangeKeyValue(
                      pNotifyCommon->UFKey,
                      false,
                      pNotifyCommon->hEvent,
                      true)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Unable to setup user feature key notification.");
        return MOS_STATUS_UNKNOWN;
    }

    // Create a wait object
    if ((bResult = MosUserFeatureWaitForSingleObject(
                                              &pNotifyCommon->hWaitEvent,
                                              pNotifyCommon->hEvent,
                                              (void *)MosUserFeatureCallback,
                                              pNotification)) == false)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create a wait object.");
        return MOS_STATUS_EVENT_WAIT_REGISTER_FAILED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureDisableNotification(
    PMOS_USER_FEATURE_INTERFACE            pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA          pNotification)
{
    PMOS_USER_FEATURE_NOTIFY_DATA_COMMON    pNotifyDataCommon;
    int32_t                                 bResult;
    MOS_STATUS                              eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);

    //---------------------------------------
    MOS_OS_ASSERT(pNotification);
    //---------------------------------------

    if (pNotification->pHandle)
    {
        pNotifyDataCommon = (PMOS_USER_FEATURE_NOTIFY_DATA_COMMON)
            pNotification->pHandle;

        if (pNotifyDataCommon->hWaitEvent)
        {
            if ((bResult = MosUnregisterWaitEx(pNotifyDataCommon->hWaitEvent)) == false)
            {
                MOS_OS_ASSERTMESSAGE("Unable to unregiser wait event.");
                    return MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED;
            }
        }
        if (pNotifyDataCommon->UFKey)
        {
            if ((eStatus = MosUserFeatureCloseKey(pNotifyDataCommon->UFKey)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("User feature key close failed.");
                return eStatus;
            }
        }
        if (pNotifyDataCommon->hEvent)
        {
            MosCloseHandle(pNotifyDataCommon->hEvent);
        }

        // Free Notify Data Memory
        MOS_FreeMemory(pNotifyDataCommon);
        pNotification->pHandle = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

float MosUtilities::MosSinc(float x)
{
    return (MOS_ABS(x) < 1e-9f) ? 1.0F : (float)(sin(x) / x);
}

float MosUtilities::MosLanczos(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = dwNumEntries >> 1;
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (MOS_ABS(x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= MOS_PI;

    return MosSinc(x) * MosSinc(x / fLanczosT);
}

float MosUtilities::MosLanczosG(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = (dwNumEntries >> 1) + (dwNumEntries & 1);
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (x > (dwNumEntries >> 1) || (- x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= MOS_PI;

    return MosSinc(x) * MosSinc(x / fLanczosT);
}

uint32_t MosUtilities::MosGCD(uint32_t a, uint32_t b)
{
    if (b == 0)
    {
        return a;
    }
    else
    {
        return MosGCD(b, a % b);
    }
}

#ifdef _MOS_UTILITY_EXT
#include "mos_utilities_ext_next.h"
#else
#define Mos_SwizzleOffset MosUtilities::MosSwizzleOffset
#endif

__inline int32_t MosUtilities::MosSwizzleOffset(
    int32_t         OffsetX,
    int32_t         OffsetY,
    int32_t         Pitch,
    MOS_TILE_TYPE   TileFormat,
    int32_t         CsxSwizzle,
    int32_t         ExtFlags)
{
    // When dealing with a tiled surface, logical linear accesses to the
    // surface (y * pitch + x) must be translated into appropriate tile-
    // formated accesses--This is done by swizzling (rearranging/translating)
    // the given access address--though it is important to note that the
    // swizzling is actually done on the accessing OFFSET into a TILED
    // REGION--not on the absolute address itself.

    // (!) Y-MAJOR TILING, REINTERPRETATION: For our purposes here, Y-Major
    // tiling will be thought of in a different way, we will deal with
    // the 16-byte-wide columns individually--i.e., we will treat a single
    // Y-Major tile as 8 separate, thinner tiles--Doing so allows us to
    // deal with both X- and Y-Major tile formats in the same "X-Major"
    // way--just with different dimensions: either 512B x 8 rows, or
    // 16B x 32 rows, respectively.

    // A linear offset into a surface is of the form
    //     y * pitch + x   =   y:x (Shorthand, meaning: y * (x's per y) + x)
    //
    // To treat a surface as being composed of tiles (though still being
    // linear), just as a linear offset has a y:x composition--its y and x
    // components can be thought of as having Row:Line and Column:X
    // compositions, respectively, where Row specifies a row of tiles, Line
    // specifies a row of pixels within a tile, Column specifies a column
    // of tiles, and X in this context refers to a byte within a Line--i.e.,
    //     offset = y:x
    //     y = Row:Line
    //     x = Col:X
    //     offset = y:x = Row:Line:Col:X

    // Given the Row:Line:Col:X composition of a linear offset, all that
    // tile swizzling does is swap the Line and Col components--i.e.,
    //     Linear Offset:   Row:Line:Col:X
    //     Swizzled Offset: Row:Col:Line:X
    // And with our reinterpretation of the Y-Major tiling format, we can now
    // describe both the X- and Y-Major tiling formats in two simple terms:
    // (1) The bit-depth of their Lines component--LBits, and (2) the
    // swizzled bit-position of the Lines component (after it swaps with the
    // Col component)--LPos.

    int32_t Row, Line, Col, x; // Linear Offset Components
    int32_t LBits, LPos; // Size and swizzled position of the Line component.
    int32_t SwizzledOffset;
    if (TileFormat == MOS_TILE_LINEAR)
    {
        return(OffsetY * Pitch + OffsetX);
    }

    if (TileFormat == MOS_TILE_Y)
    {
        LBits = 5; // Log2(TileY.Height = 32)
        LPos = 4;  // Log2(TileY.PseudoWidth = 16)
    }
    else //if (TileFormat == MOS_TILE_X)
    {
        LBits = 3; // Log2(TileX.Height = 8)
        LPos = 9;  // Log2(TileX.Width = 512)
    }

    Row = OffsetY >> LBits;               // OffsetY / LinesPerTile
    Line = OffsetY & ((1 << LBits) - 1);   // OffsetY % LinesPerTile
    Col = OffsetX >> LPos;                // OffsetX / BytesPerLine
    x = OffsetX & ((1 << LPos) - 1);    // OffsetX % BytesPerLine

    SwizzledOffset =
        (((((Row * (Pitch >> LPos)) + Col) << LBits) + Line) << LPos) + x;
    //                V                V                 V
    //                / BytesPerLine   * LinesPerTile    * BytesPerLine

    /// Channel Select XOR Swizzling ///////////////////////////////////////////
    if (CsxSwizzle)
    {
        if (TileFormat == MOS_TILE_Y) // A6 = A6 ^ A9
        {
            SwizzledOffset ^= ((SwizzledOffset >> (9 - 6)) & 0x40);
        }
        else //if (TileFormat == VPHAL_TILE_X) // A6 = A6 ^ A9 ^ A10
        {
            SwizzledOffset ^= (((SwizzledOffset >> (9 - 6)) ^ (SwizzledOffset >> (10 - 6))) & 0x40);
        }
    }

    return(SwizzledOffset);
}

void MosUtilities::MosSwizzleData(
    uint8_t         *pSrc,
    uint8_t         *pDst,
    MOS_TILE_TYPE   SrcTiling,
    MOS_TILE_TYPE   DstTiling,
    int32_t         iHeight,
    int32_t         iPitch,
    int32_t         extFlags)
{

#define IS_TILED(_a)                ((_a) != MOS_TILE_LINEAR)
#define IS_TILED_TO_LINEAR(_a, _b)  (IS_TILED(_a) && !IS_TILED(_b))
#define IS_LINEAR_TO_TILED(_a, _b)  (!IS_TILED(_a) && IS_TILED(_b))

    int32_t LinearOffset;
    int32_t TileOffset;
    int32_t x;
    int32_t y;

    // Translate from one format to another
    for (y = 0, LinearOffset = 0, TileOffset = 0; y < iHeight; y++)
    {
        for (x = 0; x < iPitch; x++, LinearOffset++)
        {
            // x or y --> linear
            if (IS_TILED_TO_LINEAR(SrcTiling, DstTiling))
            {
                TileOffset = Mos_SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    SrcTiling,
                    false,
                    extFlags);

                *(pDst + LinearOffset) = *(pSrc + TileOffset);
            }
            // linear --> x or y
            else if (IS_LINEAR_TO_TILED(SrcTiling, DstTiling))
            {
                TileOffset = Mos_SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    DstTiling,
                    false,
                    extFlags);

                *(pDst + TileOffset) = *(pSrc + LinearOffset);
            }
            else
            {
                MOS_OS_ASSERT(0);
            }
        }
    }
}
