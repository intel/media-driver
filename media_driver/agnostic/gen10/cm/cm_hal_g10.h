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

#ifndef __HAL_CM_G10__
#define __HAL_CM_G10__

#include "cm_hal.h"
#include "mhw_render_hwcmd_g10_X.h"

#define CM_NUM_HW_POLYPHASE_TABLES_G10         17
#define CM_NUM_HW_POLYPHASE_EXTRA_TABLES_G10   15

#define CM_CNL_L3_CONFIG_NUM                      9

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

// Gen10 L3 Control Register Definition
typedef struct _CM_L3_CONTROL_REGISTER_G10
{
    // uint32_t 0
    union _DW0
    {
        struct _BitField
        {
            uint32_t SlmModeEnable                 : BITFIELD_BIT(      0  );   //
            uint32_t UrbAllocation                 : BITFIELD_RANGE(  1,7  );   //
            uint32_t GpgpuL3CreditModeEnable       : BITFIELD_BIT(      8  );   //
            uint32_t ErrorDetectionBehaviorControl : BITFIELD_BIT(      9  );   //
            uint32_t Reserved0                     : BITFIELD_BIT(     10  );   //
            uint32_t ReadOnlyClientPool            : BITFIELD_RANGE( 11,17 );   //
            uint32_t DcWayAssignment               : BITFIELD_RANGE( 18,24 );   //
            uint32_t AllL3ClientPool               : BITFIELD_RANGE( 25,31 );   //
        } BitField;

        uint32_t Value;
    } DW0;

} CM_L3_CONTROL_REGISTER_G10, *PCM_L3_CONTROL_REGISTER_G10;

#define CM_L3_CACHE_CONFIG_CNTLREG_VALUE_G10        0x0x80000121

struct CM_HAL_G10_X:public CM_HAL_GENERIC
{

public:
    CM_HAL_G10_X(PCM_HAL_STATE pCmState):CM_HAL_GENERIC(pCmState){};
    ~CM_HAL_G10_X(){};

    MOS_STATUS GetCopyKernelIsa(void  *&pIsa, uint32_t &IsaSize);

    MOS_STATUS GetInitKernelIsa(void  *&pIsa, uint32_t &IsaSize);

    MOS_STATUS SetMediaWalkerParams(
                        CM_WALKING_PARAMETERS          engineeringParams,
                        PCM_HAL_WALKER_PARAMS          pWalkerParams);

    MOS_STATUS HwSetSurfaceMemoryObjectControl(
                        uint16_t                        wMemObjCtl,
                        PRENDERHAL_SURFACE_STATE_PARAMS pParams);

    MOS_STATUS RegisterSampler8x8AVSTable(
                       PCM_HAL_SAMPLER_8X8_TABLE  pSampler8x8AVSTable,
                       PCM_AVS_TABLE_STATE_PARAMS pAVSTable);

    MOS_STATUS RegisterSampler8x8(
                        PCM_HAL_SAMPLER_8X8_PARAM    pParam);

    MOS_STATUS SubmitCommands(
                        PMHW_BATCH_BUFFER       pBatchBuffer,
                        int32_t                 iTaskId,
                        PCM_HAL_KERNEL_PARAM    *pKernels,
                        void                    **ppCmdBuffer);

    uint32_t   GetMediaWalkerMaxThreadWidth();
    uint32_t   GetMediaWalkerMaxThreadHeight();

    MOS_STATUS GetHwSurfaceBTIInfo(
               PCM_SURFACE_BTI_INFO pBTIinfo);

    MOS_STATUS SetSuggestedL3Conf(
               L3_SUGGEST_CONFIG L3Conf);

    MOS_STATUS AllocateSIPCSRResource();

    MOS_STATUS GetGenStepInfo(char*& stepinfostr);

    int32_t ColorCountSanityCheck(uint32_t colorCount);

    bool MemoryObjectCtrlPolicyCheck(uint32_t memCtrl);

    int32_t GetConvSamplerIndex(
            PMHW_SAMPLER_STATE_PARAM  pSamplerParam,
            char                     *pSamplerIndexTable,
            int32_t                   nSamp8X8Num,
            int32_t                   nSampConvNum);

    MOS_STATUS SetL3CacheConfig(
            const L3ConfigRegisterValues *values_ptr,
            PCmHalL3Settings cmhal_l3_cache_ptr );

    MOS_STATUS GetSamplerParamInfoForSamplerType(
            PMHW_SAMPLER_STATE_PARAM sampler_param_ptr,
            SamplerParam  &sampler_param);

    MOS_STATUS GetExpectedGtSystemConfig(
        PCM_EXPECTED_GT_SYSTEM_INFO pExpectedConfig);

private:
    MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              bEUSaturation);

    MOS_STATUS SetupHwDebugControl(
                        PRENDERHAL_INTERFACE   pRenderHal,
                        PMOS_COMMAND_BUFFER    pCmdBuffer);

};

#endif  // __VPHAL_CM_G10__
