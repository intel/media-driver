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
//! \file      cm_hal_g9.h
//! \brief     Common HAL CM Gen9 functions  
//!

#ifndef __VPHAL_CM_G9__
#define __VPHAL_CM_G9__

#include "cm_hal.h"
#include "mos_os.h"
#include "renderhal.h"
#include "cm_def.h"
#include "mhw_vebox_hwcmd_g9_X.h"
#include "mhw_render_hwcmd_g9_X.h"
#include "mhw_state_heap_hwcmd_g9_X.h"

#define CM_NUM_HW_POLYPHASE_TABLES_G9            17
#define CM_NUM_HW_POLYPHASE_EXTRA_TABLES_G9      15

#define VEBOX_SURFACE_NUMBER_G9                 (16)        //SKL

#define CM_MAX_THREADSPACE_WIDTH_SKLUP_FOR_MW   2047
#define CM_MAX_THREADSPACE_HEIGHT_SKLUP_FOR_MW  2047

// The defines for the CM VEBOX usage   

typedef struct __CM_VEBOX_PARAM_G9
{
    unsigned char                padding1[4024];
    unsigned char                padding2[3732];
    unsigned char                padding3[3936];
    unsigned char                padding4[2048];

    mhw_vebox_g9_X::VEBOX_GAMUT_STATE_CMD        *pGamutState;
    mhw_vebox_g9_X::VEBOX_VERTEX_TABLE_CMD       *pVertexTable;
    mhw_vebox_g9_X::VEBOX_CAPTURE_PIPE_STATE_CMD *pCapturePipe;
    mhw_vebox_g9_X::VEBOX_DNDI_STATE_CMD         *pDndiState;
    mhw_vebox_g9_X::VEBOX_IECP_STATE_CMD         *pIecpState;
}CM_VEBOX_PARAM_G9, PCM_VEBOX_PARAM_G9;

#define CM_SKL_L3_CONFIG_NUM 8
static const L3_CONFIG SKL_L3[CM_SKL_L3_CONFIG_NUM]  = 
{  //8k unit
    {256,    128,    384,    0,    0,    0,    0,    0,    0}
};

static const L3ConfigRegisterValues SKL_L3_PLANE[CM_SKL_L3_CONFIG_NUM] =
{                                                         // SLM    URB   Rest   DC    RO    I/S    C     T     Sum
    { 0x00000000, 0x00000000, 0x00000000, 0x60000060 },   //{0,     48,    48,    0,    0,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x00808060 },   //{0,     48,    0,    16,   32,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x00818040 },   //{0,     32,    0,    16,   48,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x00030040 },   //{0,     32,    0,     0,   64,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x80000040 },   //{0,     32,    64,    0,    0,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x60000121 },   //{32,    16,    48,    0,    0,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x00410121 },   //{32,    16,    0,    16,   32,    0,    0,    0,    96},
    { 0x00000000, 0x00000000, 0x00000000, 0x00808121 }    //{32,    16,    0,    32,   16,    0,    0,    0,    96}
};

struct CM_HAL_G9_X:public CM_HAL_GENERIC
{
    
public:
    CM_HAL_G9_X(PCM_HAL_STATE pCmState):
        CM_HAL_GENERIC(pCmState) {};
        
    ~CM_HAL_G9_X(){};

    MOS_STATUS GetCopyKernelIsa(void  *&pIsa, uint32_t &IsaSize);

    MOS_STATUS GetInitKernelIsa(void  *&pIsa, uint32_t &IsaSize);

    MOS_STATUS SetMediaWalkerParams(
                        CM_WALKING_PARAMETERS          engineeringParams,
                        PCM_HAL_WALKER_PARAMS          pWalkerParams);

    MOS_STATUS HwSetSurfaceMemoryObjectControl(
                        uint16_t                        wMemObjCtl,
                        PRENDERHAL_SURFACE_STATE_PARAMS pParams);

    MOS_STATUS RegisterSampler8x8(    
                        PCM_HAL_SAMPLER_8X8_PARAM    pParam); 

    MOS_STATUS SubmitCommands(
                        PMHW_BATCH_BUFFER       pBatchBuffer,       
                        int32_t                 iTaskId,           
                        PCM_HAL_KERNEL_PARAM    *pKernels,          
                        void                    **ppCmdBuffer); 

    MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              bEUSaturation);
    
    uint32_t   GetMediaWalkerMaxThreadWidth();
    
    uint32_t   GetMediaWalkerMaxThreadHeight();

    MOS_STATUS GetHwSurfaceBTIInfo(
                   PCM_SURFACE_BTI_INFO pBTIinfo);

    MOS_STATUS SetSuggestedL3Conf(
                  L3_SUGGEST_CONFIG L3Conf);

    MOS_STATUS AllocateSIPCSRResource();

    MOS_STATUS GetGenStepInfo(char*& stepinfostr);

    int32_t ColorCountSanityCheck(uint32_t colorCount);

    bool IsP010SinglePassSupported() { return false; };

    bool MemoryObjectCtrlPolicyCheck(uint32_t memCtrl);

    int32_t GetConvSamplerIndex(
            PMHW_SAMPLER_STATE_PARAM  pSamplerParam,
            char                     *pSamplerIndexTable,
            int32_t                   nSamp8X8Num,
            int32_t                   nSampConvNum);

    MOS_STATUS SetL3CacheConfig(
            const L3ConfigRegisterValues *values_ptr, 
            PCmHalL3Settings cmhal_l3_cache_ptr);

    MOS_STATUS GetSamplerParamInfoForSamplerType(
            PMHW_SAMPLER_STATE_PARAM sampler_param_ptr,
            SamplerParam  &sampler_param);

    void OverwriteSteppingTable(const char **newTable, int len)
    {
        m_steppingTable.clear();
        for (int i = 0; i < len; i ++)
        {
            m_steppingTable.push_back(newTable[i]);
        }
    }


    MOS_STATUS GetExpectedGtSystemConfig(
        PCM_EXPECTED_GT_SYSTEM_INFO pExpectedConfig);

private:
    MOS_STATUS RegisterSampler8x8AVSTable(
                       PCM_HAL_SAMPLER_8X8_TABLE  pSampler8x8AVSTable,
                       PCM_AVS_TABLE_STATE_PARAMS pAVSTable);
        
    MOS_STATUS SetupHwDebugControl(    
                        PRENDERHAL_INTERFACE   pRenderHal,
                        PMOS_COMMAND_BUFFER    pCmdBuffer);

    std::vector<const char *> m_steppingTable;
    
};

#define REG_GPR_BASE_G9 0x2600
#define REG_TIMESTAMP_BASE_G9  0x2358


#endif  // __VPHAL_CM_G9__
