/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file      cm_hal_g12.h
//! \brief     Common HAL CM Gen12LP (TGLLP) function declarations.
//!

#ifndef _CM_HAL_G12_H_
#define _CM_HAL_G12_H_

#include "cm_hal.h"
#include "mhw_render_hwcmd_g8_X.h"

#define CM_NUM_HW_POLYPHASE_TABLES_G12         17
#define CM_NUM_HW_POLYPHASE_EXTRA_TABLES_G12   15

#define TGL_L3_CONFIG_NUM                      7
#define CM_THREADSPACE_MAX_COLOR_COUNT_GEN12   256

#define CM_NS_PER_TICK_RENDER_G12_DEFAULT      (83.333)

struct CM_TASK_CONFIG_EX;


// TGLLP L3 Allocation Control Register
static const L3ConfigRegisterValues TGL_L3_PLANE[TGL_L3_CONFIG_NUM] = {
                                       //  URB  Rest  DC  RO   Z    Color  UTC  CB  Sum (in KB)
    { 0xD0000020, 0x0,        0, 0 },  //  64   416   0   0    0    0      0    0   480
    { 0x78000040, 0x306044,   0, 0 },  //  128  240   0   0    48   48     0    16  480
    { 0x21E020,   0x408044,   0, 0 },  //  64   0     32  240  64   64     0    16  480
    { 0x48000020, 0x810044,   0, 0 },  //  64   144   0   0    128  128    0    16  480
    { 0x18000020, 0xB0000044, 0, 0 },  //  64   48    0   0    0    0      352  16  480
    { 0x88000020, 0x40000044, 0, 0 },  //  64   272   0   0    0    0      128  16  480
    { 0xC8000020, 0x44,       0, 0 },  //  64   400   0   0    0    0      0    16  480
    };

struct CM_HAL_G12_X:public CM_HAL_GENERIC
{

public:
    CM_HAL_G12_X(CM_HAL_STATE *cmState);

    ~CM_HAL_G12_X() {}

    MOS_STATUS GetCopyKernelIsa(void  *&isa, uint32_t &isaSize);

    MOS_STATUS GetInitKernelIsa(void  *&isa, uint32_t &isaSize);

    MOS_STATUS SetMediaWalkerParams(
                        CM_WALKING_PARAMETERS          engineeringParams,
                        PCM_HAL_WALKER_PARAMS          walkerParams);

    MOS_STATUS HwSetSurfaceMemoryObjectControl(
                        uint16_t                        memObjCtl,
                        PRENDERHAL_SURFACE_STATE_PARAMS surfStateParams);

    MOS_STATUS RegisterSampler8x8AVSTable(
                       PCM_HAL_SAMPLER_8X8_TABLE  sampler8x8AvsTable,
                       PCM_AVS_TABLE_STATE_PARAMS avsTable);

    MOS_STATUS RegisterSampler8x8(
                        PCM_HAL_SAMPLER_8X8_PARAM    param);

    MOS_STATUS SubmitCommands(
                        PMHW_BATCH_BUFFER       batchBuffer,
                        int32_t                 taskId,
                        PCM_HAL_KERNEL_PARAM    *kernelParam,
                        void                    **cmdBuffer);

#if (_RELEASE_INTERNAL || _DEBUG)
#if defined (CM_DIRECT_GUC_SUPPORT)
    MOS_STATUS SubmitDummyCommands(
        PMHW_BATCH_BUFFER       batchBuffer,
        int32_t                 taskId,
        PCM_HAL_KERNEL_PARAM    *kernelParam,
        void                    **cmdBuffer);
#endif
#endif

    uint32_t   GetMediaWalkerMaxThreadWidth();
    uint32_t   GetMediaWalkerMaxThreadHeight();

    MOS_STATUS GetHwSurfaceBTIInfo(
               PCM_SURFACE_BTI_INFO btiInfo);

    MOS_STATUS SetSuggestedL3Conf(
               L3_SUGGEST_CONFIG l3Config);

    MOS_STATUS AllocateSIPCSRResource();

    MOS_STATUS GetGenStepInfo(char*& stepinfostr);

    bool IsScoreboardParamNeeded() { return false; };

    bool IsSupportedVMESurfaceFormat(MOS_FORMAT format) {
        bool isColorFormatSupported = false;
        switch (format)
        {
        case Format_NV12:
        case Format_YUY2:
        case Format_YUYV:
        case Format_A8R8G8B8:
        case Format_P010:
        case Format_AYUV:
        case Format_Y210:
        case Format_Y410:
        case Format_Y216:
        case Format_Y416:
        case Format_P016:
        case Format_P208:
            isColorFormatSupported = true;
            break;
        default:
            CM_ASSERTMESSAGE("Error: color format = %d not supported by VME on Gen12!", format);
            break;
        }
        return isColorFormatSupported;
    }

    int32_t ColorCountSanityCheck(uint32_t colorCount);

    bool MemoryObjectCtrlPolicyCheck(uint32_t memCtrl);

    int32_t GetConvSamplerIndex(
        PMHW_SAMPLER_STATE_PARAM  samplerParam,
        char                     *samplerIndexTable,
        int32_t                   nSamp8X8Num,
        int32_t                   nSampConvNum);

    MOS_STATUS SetL3CacheConfig(
        const L3ConfigRegisterValues *values,
        PCmHalL3Settings cmHalL3Setting );

    MOS_STATUS GetSamplerParamInfoForSamplerType(
        PMHW_SAMPLER_STATE_PARAM mhwSamplerParam,
        SamplerParam  &samplerParam);

    MOS_STATUS GetExpectedGtSystemConfig(
        PCM_EXPECTED_GT_SYSTEM_INFO expectedConfig);

    int32_t GetTimeStampResourceSize()
    {
        return sizeof(uint64_t)*CM_SYNC_QWORD_PER_TASK
                + sizeof(uint64_t)*CM_TRACKER_ID_QWORD_PER_TASK;
    }

    uint64_t ConverTicksToNanoSecondsDefault(uint64_t ticks)
    {
        return static_cast<uint64_t>(
            ticks * CM_NS_PER_TICK_RENDER_G12_DEFAULT);
    }

    bool CheckMediaModeAvailability() { return true; }

    bool IsFastPathByDefault() { return true; }
    
    uint32_t GetSmallestMaxThreadNum() { return 64; }

private:
    MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              euSaturated);

    MOS_STATUS SetupHwDebugControl(
                        PRENDERHAL_INTERFACE   renderHal,
                        PMOS_COMMAND_BUFFER    cmdBuffer);
};

#endif  // #ifndef _CM_HAL_G12_H_
