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

#ifndef __CM_HAL_G8_H__
#define __CM_HAL_G8_H__

#include "mos_os.h"
#include "renderhal_legacy.h"
#include "cm_def.h"
#include "cm_hal.h"
#include "mhw_vebox_hwcmd_g8_X.h"
#include "mhw_render_hwcmd_g8_X.h"
#include "mhw_mmio_g8.h"

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
            uint32_t length                  : 16;   // OP_LENGTH
            uint32_t instructionSubOpcode    : 8;    // 3h
            uint32_t instructionOpcode       : 3;    // 1h
            uint32_t instructionPipeline     : 2;    // 2h
            uint32_t instructionType         : 3;    // 3h
        };
        struct
        {
            uint32_t value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            uint32_t interfaceDescriptorOffset       : 6; // 5 bits PreHSW, 6 bits HSW+
            uint32_t                                : 2;
            uint32_t objectID                        : 24;
        };
        struct
        {
            uint32_t value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            uint32_t indirectDataLength              : 17;   // U17 in bytes
            uint32_t                                : 4;
            uint32_t useScoreboard                   : 1;
            uint32_t                                : 2;
            uint32_t threadSynchronization           : 1;    // bool
            uint32_t                                : 6;
            uint32_t childrenPresent                 : 1;    // bool
        };
        struct
        {
            uint32_t value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            uint32_t indirectDataStartAddress;               // GTT [31:0] [0-512MB] Bits 31:29 MBZ
        };
        struct
        {
            uint32_t value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            uint32_t reserved                        : 32;
        };
        struct
        {
            uint32_t value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            uint32_t scoreboardMask                  : 8;     // Scoreboard Mask
            uint32_t groupIdLoopSelect               : 24;    // Gen8+
        };
        struct
        {
            uint32_t value;
        };
    } DW5;

    // DWORD 6
    union
    {
        struct
        {
            uint32_t                                : 8;
            uint32_t midLoopUnitX                    : 2;
            uint32_t                                : 2;
            uint32_t midLoopUnitY                    : 2;
            uint32_t                                : 2;
            uint32_t midLoopExtraSteps               : 5;     // Middle Loop Extra Steps
            uint32_t                                : 3;
            uint32_t colorCountMinusOne              : 4;     // Color Count Minus One
            uint32_t                                : 1;
            uint32_t quadMode                        : 1;     // Pre-Gen8. For products with 4 half-slices
            uint32_t repel                           : 1;     // Pre-Gen8
            uint32_t dualMode                        : 1;     // Pre-Gen8. For products with 2 half-slices
        };
        struct
        {
            uint32_t value;
        };
    } DW6;

    // DWORD 7
    union
    {
        struct
        {
            uint32_t localLoopExecCount              : 10;
            uint32_t                                : 6;
            uint32_t globalLoopExecCount             : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t value;
        };
    } DW7;

    // DWORD 8
    union
    {
        struct
        {
            uint32_t blockResolutionX                : 9;
            uint32_t                                : 7;
            uint32_t blockResolutionY                : 9;
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t value;
        };
    } DW8;

    // DWORD 9
    union
    {
        struct
        {
            uint32_t localStartX                     : 9;
            uint32_t                                : 7;
            uint32_t localStartY                     : 9;
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t value;
        };
    } DW9;

    // DWORD 10
    union
    {
        struct
        {
            uint32_t localEndX                       : 9;    // Pre-Gen7.5. Reserved MBZ in Gen7.5+
            uint32_t                                : 7;
            uint32_t localEndY                       : 9;    // Pre-Gen7.5. Reserved MBZ in Gen7.5+
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t value;
        };
    } DW10;

    // DWORD 11
    union
    {
        struct
        {
            uint32_t localOuterLoopStrideX           : 10;
            uint32_t                                : 6;
            uint32_t localOuterLoopStrideY           : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t value;
        };
    } DW11;

    // DWORD 12
    union
    {
        struct
        {
            uint32_t localInnerLoopUnitX             : 10;
            uint32_t                                : 6;
            uint32_t localInnerLoopUnitY             : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t value;
        };
    } DW12;

    // DWORD 13
    union
    {
        struct
        {
            uint32_t globalResolutionX               : 9;
            uint32_t                                : 7;
            uint32_t globalResolutionY               : 9;
            uint32_t                                : 7;
        };
        struct
        {
            uint32_t value;
        };
    } DW13;

    // DWORD 14
    union
    {
        struct
        {
            uint32_t globalStartX                    : 10;
            uint32_t                                : 6;
            uint32_t globalStartY                    : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t value;
        };
    } DW14;

    // DWORD 15
    union
    {
        struct
        {
            uint32_t globalOuterLoopStrideX          : 10;
            uint32_t                                : 6;
            uint32_t globalOuterLoopStrideY          : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t value;
        };
    } DW15;

    // DWORD 16
    union
    {
        struct
        {
            uint32_t globalInnerLoopUnitX            : 10;
            uint32_t                                : 6;
            uint32_t globalInnerLoopUnitY            : 10;
            uint32_t                                : 6;
        };
        struct
        {
            uint32_t value;
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
    CM_HAL_G8_X(PCM_HAL_STATE cmState):CM_HAL_GENERIC(cmState){};
    ~CM_HAL_G8_X(){};

    MOS_STATUS GetCopyKernelIsa(void  *&isa, uint32_t &isaSize);

    MOS_STATUS GetInitKernelIsa(void  *&isa, uint32_t &isaSize);

    MOS_STATUS SetMediaWalkerParams(
                        CM_WALKING_PARAMETERS          engineeringParams,
                        PCM_HAL_WALKER_PARAMS          walkerParams);

    MOS_STATUS HwSetSurfaceMemoryObjectControl(
                        uint16_t                        memObjCtl,
                        PRENDERHAL_SURFACE_STATE_PARAMS surfStateParams);

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
    MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              euSaturated);

    MOS_STATUS GetExpectedGtSystemConfig(
                        PCM_EXPECTED_GT_SYSTEM_INFO expectedConfig);

    uint32_t   GetMediaWalkerMaxThreadWidth();
    uint32_t   GetMediaWalkerMaxThreadHeight();

    MOS_STATUS GetHwSurfaceBTIInfo(
                       PCM_SURFACE_BTI_INFO btiInfo);

    MOS_STATUS SetSuggestedL3Conf(
                       L3_SUGGEST_CONFIG l3Config);

    MOS_STATUS AllocateSIPCSRResource();

    MOS_STATUS GetGenStepInfo(char*& stepInfoStr);

    bool IsSurf3DQpitchSupportedbyHw(){ return false;};

    bool IsCompareMaskSupportedbyHw(){ return false;};

    bool IsAdjacentSamplerIndexRequiredbyHw(){ return true;};

    bool IsSurfaceCompressionWARequired(){ return false;};

    int32_t ColorCountSanityCheck(uint32_t colorCount);

    bool MemoryObjectCtrlPolicyCheck(uint32_t memCtrl);

    bool IsGPUCopySurfaceNoCacheWARequired(){ return false;};

    bool IsP010SinglePassSupported() { return false; };

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

    uint64_t ConverTicksToNanoSecondsDefault(uint64_t ticks);


private:

    MOS_STATUS RegisterSampler8x8AVSTable(
                        PCM_HAL_SAMPLER_8X8_TABLE  sampler8x8AvsTable,
                        PCM_AVS_TABLE_STATE_PARAMS avsTable);

    MOS_STATUS SetupHwDebugControl(
                        PRENDERHAL_INTERFACE   renderHal,
                        PMOS_COMMAND_BUFFER    cmdBuffer);

};

#endif  // __CM_HAL_G8_H__
