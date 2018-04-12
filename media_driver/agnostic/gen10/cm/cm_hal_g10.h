/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_hal_g10.h
//! \brief     Common HAL CM Gen10 functions 
//!

#ifndef _CM_HAL_G10_H_
#define _CM_HAL_G10_H_

#include "cm_hal.h"
#include "mhw_render_hwcmd_g10_X.h"

#define CM_NUM_HW_POLYPHASE_TABLES_G10         17
#define CM_NUM_HW_POLYPHASE_EXTRA_TABLES_G10   15

#define CM_CNL_L3_CONFIG_NUM                   9
#define CM_NS_PER_TICK_RENDER_G10_DEFAULT      (83.333)

static const L3_CONFIG CNL_L3[CM_CNL_L3_CONFIG_NUM] =
{  //8k unit
    { 256, 128, 384, 0, 0, 0, 0, 0, 0 }
};

static const L3ConfigRegisterValues CNL_L3_PLANE[CM_CNL_L3_CONFIG_NUM] =
{                                                                    // SLM    URB   Rest   DC     RO      Sum
    { 0x00000000, 0x00000000, 0x00000000, 0x80000080 },              // 0       128    128   0     0       256
    { 0x00000000, 0x00000000, 0x00000000, 0x00418080 },              // 0       128     0    32    96      256
    { 0x00000000, 0x00000000, 0x00000000, 0x00420060 },              // 0       96      0    32    128     256
    { 0x00000000, 0x00000000, 0x00000000, 0x00030040 },              // 0       64      0    0     192     256
    { 0x00000000, 0x00000000, 0x00000000, 0xC0000040 },              // 0       64      0    192    0      256
    { 0x00000000, 0x00000000, 0x00000000, 0x00428040 },              // 0       64      0    32    160     256
    { 0x00000000, 0x00000000, 0x00000000, 0xA0000021 },              // 64      32     160   0     0       256
    { 0x00000000, 0x00000000, 0x00000000, 0x01008021 },              // 64      32       0   128   32      256
    { 0x00000000, 0x00000000, 0x00000000, 0xC0000001 }               // 64      0      192   0     0       256

};

#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW   2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW  2047

#define CM_L3_CACHE_CONFIG_CNTLREG_VALUE_G10        0x0x80000121

struct CM_HAL_G10_X:public CM_HAL_GENERIC
{

public:
    CM_HAL_G10_X(PCM_HAL_STATE cmState):
        CM_HAL_GENERIC(cmState),
        m_timestampBaseStored(false),
        m_nsPerTick(CM_NS_PER_TICK_RENDER_G10_DEFAULT)
    {
        MOS_ZeroMemory(&m_resTimestampBase, sizeof(m_resTimestampBase));
    }
        
    ~CM_HAL_G10_X();

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
    MOS_STATUS SubmitTimeStampBaseCommands();
    uint32_t   GetMediaWalkerMaxThreadWidth();
    uint32_t   GetMediaWalkerMaxThreadHeight();

    MOS_STATUS GetHwSurfaceBTIInfo(
               PCM_SURFACE_BTI_INFO btiInfo);

    MOS_STATUS SetSuggestedL3Conf(
               L3_SUGGEST_CONFIG l3Config);

    MOS_STATUS AllocateSIPCSRResource();

    MOS_STATUS GetGenStepInfo(char*& stepInfoStr);

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

    uint64_t ConvertTicksToNanoSeconds(uint64_t ticks);

private:
    MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              euSaturated);

    MOS_STATUS SetupHwDebugControl(
                        PRENDERHAL_INTERFACE   renderHal,
                        PMOS_COMMAND_BUFFER    cmdBuffer);

    CM_HAL_TS_RESOURCE m_resTimestampBase;
    bool m_timestampBaseStored;
    double m_nsPerTick;

};

#endif  // _CM_HAL_G10_H_
