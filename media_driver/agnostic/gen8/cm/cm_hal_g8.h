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
//! \file      cm_hal_g8.h  
//! \brief     Common HAL CM Gen8 functions  
//!

#ifndef __VPHAL_CM_G8__
#define __VPHAL_CM_G8__

#include "mos_os.h"
#include "renderhal.h"
#include "cm_def.h"
#include "cm_hal.h"
#include "mhw_vebox_hwcmd_g8_X.h"
#include "mhw_render_hwcmd_g8_X.h"

#define CM_NUM_HW_POLYPHASE_TABLES_G8          17

#define CM_MAX_THREADSPACE_WIDTH_FOR_MW        511
#define CM_MAX_THREADSPACE_HEIGHT_FOR_MW       511

#define CM_BDW_L3_CONFIG_NUM 8
#define CM_NUM_CONVOLVE_ROWS_BDW 16

static const L3_CONFIG BDW_L3[CM_BDW_L3_CONFIG_NUM]  = 
{  //8k unit
    {0,    48,    48,    0,    0,    0,    0,    0,    96},
    {0,    48,    0,    16,    32,    0,    0,    0,    96},
    {0,    32,    0,    16,    48,    0,    0,    0,    96},
    {0,    32,    0,    0,    64,    0,    0,    0,    96},
    {0,    32,    64,    0,    0,    0,    0,    0,    96},
    {32,    16,    48,    0,    0,    0,    0,    0,    96},
    {32,    16,    0,    16,    32,    0,    0,    0,    96},
    {32,    16,    0,    32,    16,    0,    0,    0,    96}
};
static const L3ConfigRegisterValues BDW_L3_PLANE[CM_BDW_L3_CONFIG_NUM] =
{                                                 // SLM    URB   Rest     DC     RO     I/S    C    T      Sum ( BDW GT2; for GT1, half of the values; for GT3, double the values )
    { 0, 0, 0, 0x60000060 },                      //{  0,   384,    384,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x00410060 },                      //{  0,   384,      0,   128,   256,    0,    0,    0,    768},
    { 0, 0, 0, 0x00418040 },                      //{  0,   256,      0,   128,   384,    0,    0,    0,    768},
    { 0, 0, 0, 0x00020040 },                      //{  0,   256,      0,     0,   512,    0,    0,    0,    768},
    { 0, 0, 0, 0x80000040 },                      //{  0,   256,    512,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x60000021 },                      //{192,   128,    384,     0,     0,    0,    0,    0,    768},
    { 0, 0, 0, 0x00410021 },                      //{192,   128,      0,   128,   256,    0,    0,    0,    768},
    { 0, 0, 0, 0x00808021 }                       //{192,   128,      0,   256,   128,    0,    0,    0,    768}
};

// BDW GT2
// SLM     URB    Rest   DC       RO     I/S     C     T      Sum
// {192,   128,    0,    256,     128,   0,      0,    0       }
#define CM_L3_CACHE_CONFIG_SQCREG1_VALUE_G8         0x00610000
#define CM_L3_CACHE_CONFIG_CNTLREG_VALUE_G8         0x00808021

//! \brief      for BDW GT2 WA
//!              SLM     URB     Rest     DC     RO    I/S     C       T
//!             { 256,   128,    384,      0,     0,    0,     0,      0,     }
#define CM_L3_CACHE_CONFIG_CNTLREG_VALUE_ALLOCATE_SLM_E0F0_WA_G8 (0x60000021)

#define CM_RENDER_ENGINE_REG_L3_CACHE_SQCREG1_G8   0xB100
#define CM_RENDER_ENGINE_REG_L3_CACHE_CNTLREG_G8   0x7034

#define CM_PREMP_DBG_ADDRESS_OFFSET             (0x2248)
#define CM_PREMP_DEFAULT_VALUE                  (0x00000000)
#define CM_PREMP_ON_MI_ARB_CHECK_ONLY           (0x00000100) // Bit "8" of the Register needs to be set

typedef struct _MEDIA_OBJECT_WALKER_CMD_G6
{
    // DWORD 0
    union
    {
        struct
        {
            uint32_t Length                  : 16;   // OP_LENGTH
            uint32_t InstructionSubOpcode    : 8;    // 3h
            uint32_t InstructionOpcode       : 3;    // 1h
            uint32_t InstructionPipeline     : 2;    // 2h
            uint32_t InstructionType         : 3;    // 3h
        };
        struct
        {
            uint32_t Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            uint32_t InterfaceDescriptorOffset       : 6; // 5 bits PreHSW, 6 bits HSW+
            uint32_t                                : 2;
            uint32_t ObjectID                        : 24;
        };
        struct
        {
            uint32_t Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            uint32_t IndirectDataLength              : 17;   // U17 in bytes
            uint32_t                                : 4;
            uint32_t UseScoreboard                   : 1;
            uint32_t                                : 2;
            uint32_t ThreadSynchronization           : 1;    // bool
            uint32_t                                : 6;
            uint32_t ChildrenPresent                 : 1;    // bool
        };
        struct
        {
            uint32_t Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            uint32_t IndirectDataStartAddress;               // GTT [31:0] [0-512MB] Bits 31:29 MBZ
        };
        struct
        {
            uint32_t Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            uint32_t Reserved                        : 32;
        };
        struct
        {
            uint32_t Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            uint32_t ScoreboardMask                  : 8;     // Scoreboard Mask
            uint32_t GroupIdLoopSelect               : 24;    // Gen8+
        };
        struct
        {
            uint32_t Value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            uint32_t                                : 8;
            uint32_t MidLoopUnitX                    : 2;
            uint32_t                                : 2;
            uint32_t MidLoopUnitY                    : 2;
            uint32_t                                : 2;
            uint32_t MidLoopExtraSteps               : 5;     // Middle Loop Extra Steps
            uint32_t                                : 3;
            uint32_t ColorCountMinusOne              : 4;     // Color Count Minus One
            uint32_t                                : 1;
            uint32_t QuadMode                        : 1;     // Pre-Gen8. For products with 4 half-slices
            uint32_t Repel                           : 1;     // Pre-Gen8
            uint32_t DualMode                        : 1;     // Pre-Gen8. For products with 2 half-slices
        };
        struct
        {
            uint32_t Value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            uint32_t LocalLoopExecCount              : 10;
            uint32_t                                : 6;
            uint32_t GlobalLoopExecCount             : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t Value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            uint32_t BlockResolutionX                : 9;
            uint32_t                                : 7;
            uint32_t BlockResolutionY                : 9;
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t Value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            uint32_t LocalStartX                     : 9;
            uint32_t                                : 7;
            uint32_t LocalStartY                     : 9;
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t Value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            uint32_t LocalEndX                       : 9;    // Pre-Gen7.5. Reserved MBZ in Gen7.5+
            uint32_t                                : 7;
            uint32_t LocalEndY                       : 9;    // Pre-Gen7.5. Reserved MBZ in Gen7.5+
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t Value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            uint32_t LocalOuterLoopStrideX           : 10;
            uint32_t                                : 6;
            uint32_t LocalOuterLoopStrideY           : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t Value;
        };
    } DW11;

    // DWORD 12
    union
    {
        struct
        {
            uint32_t LocalInnerLoopUnitX             : 10;
            uint32_t                                : 6;
            uint32_t LocalInnerLoopUnitY             : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t Value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            uint32_t GlobalResolutionX               : 9;
            uint32_t                                : 7;
            uint32_t GlobalResolutionY               : 9;
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t Value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            uint32_t GlobalStartX                    : 10;
            uint32_t                                : 6;
            uint32_t GlobalStartY                    : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t Value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            uint32_t GlobalOuterLoopStrideX          : 10;
            uint32_t                                : 6;
            uint32_t GlobalOuterLoopStrideY          : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t Value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            uint32_t GlobalInnerLoopUnitX            : 10;
            uint32_t                                : 6;
            uint32_t GlobalInnerLoopUnitY            : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t Value;
        };
    } DW16;
} MEDIA_OBJECT_WALKER_CMD_G6, *PMEDIA_OBJECT_WALKER_CMD_G6;

typedef struct _CM_VEBOX_STATE_G8
{
    union
    {
        struct
        {
            uint32_t    ColorGamutExpansionEnable : 1;
            uint32_t    ColorGamutCompressionEnable : 1;
            uint32_t    GlobalIECPEnable : 1;
            uint32_t    DNEnable : 1;
            uint32_t    DIEnable : 1;
            uint32_t    DNDIFirstFrame : 1;
            uint32_t    DownsampleMethod422to420 : 1;
            uint32_t    DownsampleMethod444to422 : 1;
            uint32_t    DIOutputFrames : 2;
            uint32_t    DemosaicEnable : 1;
            uint32_t    VignetteEnable : 1;
            uint32_t    AlphaPlaneEnable : 1;
            uint32_t    HotPixelFilteringEnable : 1;
            uint32_t    SingleSliceVeboxEnable : 1;
            uint32_t: 11; // Reserved
            uint32_t    StateSurfaceControlBits : 6;
        };
        struct
        {
            uint32_t    Value;
        };
    } DW0;

    mhw_vebox_g8_X::VEBOX_GAMUT_STATE_CMD  *pGamutState;
    mhw_vebox_g8_X::VEBOX_VERTEX_TABLE_CMD *pVertexTable;
}  CM_VEBOX_STATE_G8, *PCM_VEBOX_STATE_G8;

struct CM_HAL_G8_X:public CM_HAL_GENERIC
{
    
public:
    CM_HAL_G8_X(PCM_HAL_STATE pCmState):CM_HAL_GENERIC(pCmState){};
    ~CM_HAL_G8_X(){};

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

    MOS_STATUS GetExpectedGtSystemConfig(
                        PCM_EXPECTED_GT_SYSTEM_INFO pExpectedConfig);

    uint32_t   GetMediaWalkerMaxThreadWidth();
    uint32_t   GetMediaWalkerMaxThreadHeight();

    MOS_STATUS GetHwSurfaceBTIInfo(
                       PCM_SURFACE_BTI_INFO pBTIinfo);

    MOS_STATUS SetSuggestedL3Conf(
                       L3_SUGGEST_CONFIG L3Conf);

    MOS_STATUS AllocateSIPCSRResource();

    MOS_STATUS GetGenStepInfo(char*& stepinfostr);
    
    bool IsSurf3DQpitchSupportedbyHw(){ return false;};

    bool IsCompareMaskSupportedbyHw(){ return false;};

    bool IsAdjacentSamplerIndexRequiredbyHw(){ return true;};

    bool IsSurfaceCompressionWARequired(){ return false;};

    int32_t ColorCountSanityCheck(uint32_t colorCount);

    bool MemoryObjectCtrlPolicyCheck(uint32_t memCtrl);

    bool IsGPUCopySurfaceNoCacheWARequired(){ return false;};

    bool IsP010SinglePassSupported() { return false; };

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

private:

    MOS_STATUS RegisterSampler8x8AVSTable(
                        PCM_HAL_SAMPLER_8X8_TABLE  pSampler8x8AVSTable,
                        PCM_AVS_TABLE_STATE_PARAMS pAVSTable);
        
    MOS_STATUS SetupHwDebugControl(    
                        PRENDERHAL_INTERFACE   pRenderHal,
                        PMOS_COMMAND_BUFFER    pCmdBuffer);
};

#endif  // __VPHAL_CM_G8__
