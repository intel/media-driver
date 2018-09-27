/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file      cm_hal_g11.h 
//! \brief     Common HAL CM Gen11 functions 
//!

#ifndef _CM_HAL_G11_H_
#define _CM_HAL_G11_H_

#include "cm_hal.h"
//#include "mhw_render_hwcmd_g8_X.h"

#define CM_NUM_HW_POLYPHASE_TABLES_G11         17
#define CM_NUM_HW_POLYPHASE_EXTRA_TABLES_G11   15

#define ICL_L3_CONFIG_NUM                      9
#define CM_THREADSPACE_MAX_COLOR_COUNT_GEN11   256

#define CM_NS_PER_TICK_RENDER_G11_DEFAULT      (83.333)

// L3 Allocation Control Register
static const L3ConfigRegisterValues ICL_L3_PLANE[ ICL_L3_CONFIG_NUM ] = {
                                     //  URB  Rest  DC  RO   Z    Color  UTC  CB  Sum (in KB)
    {0x80000080, 0x0000000D, 0, 0},  //  128  128   0   0    0    0      0    0   256
    {0x70000080, 0x40804D,   0, 0},  //  128  112   0   0    64   64     0    16  384
    {0x41C060,   0x40804D,   0, 0},  //  96   0     32  112  64   64     0    16  384
    {0x2C040,    0x20C04D,   0, 0},  //  64   0     0   176  32   96     0    16  384
    {0x30000040, 0x81004D,   0, 0},  //  64   48    0   0    128  128    0    16  384
    {0xC040,     0x8000004D, 0, 0},  //  64   0     0   48   0    0      256  16  384
    {0xA0000420, 0x0000000D, 0, 0},  //  64   320   0   0    0    0      0    0   384
    {0xC0000040, 0x4000000D, 0, 0},  //  64   192   0   0    0    0      128  0   384
    {0xB0000040, 0x4000004D, 0, 0},  //  64   176   0   0    0    0      128  16  384
};

struct CM_HAL_G11_X:public CM_HAL_GENERIC
{

public:
    CM_HAL_G11_X(PCM_HAL_STATE cmState):
        CM_HAL_GENERIC(cmState) {}
        
    ~CM_HAL_G11_X() {}

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
#if defined(CM_DIRECT_GUC_SUPPORT)
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

    MOS_STATUS GetGenStepInfo(char*& stepInfoStr);

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
        case Format_P208:
            isColorFormatSupported = true;
            break;
        default:
            CM_ASSERTMESSAGE("Error: color format = %d not supported by VME on Gen11!", format);
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
            PCmHalL3Settings cmHalL3Setting);

    MOS_STATUS GetSamplerParamInfoForSamplerType(
            PMHW_SAMPLER_STATE_PARAM mhwSamplerParam,
            SamplerParam  &samplerParam);

    MOS_STATUS GetExpectedGtSystemConfig(
        PCM_EXPECTED_GT_SYSTEM_INFO expectedConfig);

    uint64_t ConverTicksToNanoSecondsDefault(uint64_t ticks);

private:
    MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              euSaturated);

    MOS_STATUS SetupHwDebugControl(
                        PRENDERHAL_INTERFACE   renderHal,
                        PMOS_COMMAND_BUFFER    cmdBuffer);
};

#endif  // _CM_HAL_G11_H_
