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
//! \file      cm_hal.h  
//! \brief     Main Entry point for CM HAL component  
//!
#ifndef __CM_HAL_H__
#define __CM_HAL_H__

#include "renderhal.h"
#include "cm_common.h"
#include "cm_debug.h"
#include "mhw_vebox.h"
#include "cm_hal_generic.h"
#include <string>
#include <map>


#define GSH_DYNAMIC

#define DiscardLow8Bits(x)  (uint16_t)(0xff00 & x)
#define FloatToS3_12(x)  (uint16_t)((short)(x * 4096))
#define FloatToS3_4(x)   (DiscardLow8Bits(FloatToS3_12(x)))

#ifdef GSH_DYNAMIC
#define CM_32K                      (32*1024)
#endif

#define HAL_CM_KERNEL_CACHE_MISS_THRESHOLD    4
#define HAL_CM_KERNEL_CACHE_HIT_TO_MISS_RATIO 100

#ifdef _WIN64
#define IGC_DLL_NAME   "igc64.dll"
#else
#define IGC_DLL_NAME   "igc32.dll"
#endif  // _WIN64


//*-----------------------------------------------------------------------------
//| Macro unsets bitPos bit in value
//*-----------------------------------------------------------------------------
#ifndef CM_HAL_UNSETBIT
#define CM_HAL_UNSETBIT(value, bitPos)                                         \
{                                                                              \
    value = (value & ~(1 << bitPos));                                          \
}
#endif

#ifndef CM_HAL_UNSETBIT64
#define CM_HAL_UNSETBIT64(value, bitPos)                                       \
{                                                                              \
    value = (value & ~(1i64 << bitPos));                                       \
}
#endif

//*-----------------------------------------------------------------------------
//| Macro sets bitPos bit in value
//*-----------------------------------------------------------------------------
#ifndef CM_HAL_SETBIT
#define CM_HAL_SETBIT(value, bitPos)                                           \
{                                                                              \
    value = (value | (1 << bitPos));                                           \
}
#endif

#ifndef CM_HAL_SETBIT64
#define CM_HAL_SETBIT64(value, bitPos)                                         \
{                                                                              \
    value = (value | (1i64 << bitPos));                                        \
}
#endif

//*-----------------------------------------------------------------------------
//| Macro checks if bitPos bit in value is set
//*-----------------------------------------------------------------------------
#ifndef CM_HAL_CHECKBIT_IS_SET
#define CM_HAL_CHECKBIT_IS_SET(bitIsSet, value, bitPos)                        \
{                                                                              \
    bitIsSet = ((value) & (1 << bitPos))? true: false;                                      \
}
#endif

#ifndef CM_HAL_CHECKBIT_IS_SET64
#define CM_HAL_CHECKBIT_IS_SET64(bitIsSet, value, bitPos)                      \
{                                                                              \
    bitIsSet = ((value) & (1i64 << bitPos));                                   \
}
#endif

#define MAX_CUSTOMIZED_PERFTAG_INDEX    249
#define GPUINIT_PERFTAG_INDEX           250
#define GPUCOPY_READ_PERFTAG_INDEX      251
#define GPUCOPY_WRITE_PERFTAG_INDEX     252
#define GPUCOPY_G2G_PERFTAG_INDEX       253
#define GPUCOPY_C2C_PERFTAG_INDEX       254
#define VEBOX_TASK_PERFTAG_INDEX        255

#define MAX_COMBINE_NUM_IN_PERFTAG      16

#define MDF_CS_CHICKEN1_PREEMPTION_CONTROL_OFFSET       0x2580
#define MDF_CS_CHICKEN1_MID_THREAD_PREEMPT_VALUE        0x00060000
#define MDF_CS_CHICKEN1_THREAD_GROUP_PREEMPT_VALUE      0x00060002
#define MDF_CS_CHICKEN1_MID_BATCH_PREEMPT_VALUE         0x00060004

//------------------------------------------------------------------------------
//| CM Buffer Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BUFFER_PARAM
{
    uint32_t                    iSize;                                          // [in]         Buffer Size
    CM_BUFFER_TYPE              type;                                           // [in]         Buffer type: Buffer, UP, SVM
    void                        *pData;                                          // [in/out]     Pointer to data
    uint32_t                    dwHandle;                                       // [in/out]     Handle
    uint32_t                    iLockFlag;                                      // [in]         Lock flag
    PMOS_RESOURCE               pMosResource;                                   // [in]         Mos resource
    bool                        isAllocatedbyCmrtUmd;                           // [in]         Flag for Cmrt@umd Created Buffer
} CM_HAL_BUFFER_PARAM, *PCM_HAL_BUFFER_PARAM;

//------------------------------------------------------------------------------
//| CM Buffer Set Surface State Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BUFFER_SURFACE_STATE_PARAM
{
    uint32_t                    iSize;                                          // [in]         Surface State Size
    uint32_t                    iOffset;                                        // [in]         Surface State Base Address Offset
    uint16_t                    wMOCS;                                          // [in]         Surface State mocs settings
    uint32_t                    iAliasIndex;                                    // [in]         Surface Alias Index
    uint32_t                    dwHandle;                                       // [in]         Handle
} CM_HAL_BUFFER_SURFACE_STATE_PARAM, *PCM_HAL_BUFFER_SURFACE_STATE_PARAM;

//------------------------------------------------------------------------------
//| CM BB Args
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BB_ARGS
{
    uint64_t  uiKernelIds[CM_MAX_KERNELS_PER_TASK];  
    uint64_t  uiRefCount;
    bool      bLatest;
} CM_HAL_BB_ARGS, *PCM_HAL_BB_ARGS;

//------------------------------------------------------------------------------
//| CM 2DUP Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_UP_PARAM
{
    uint32_t                    iWidth;                                         // [in]         Surface Width
    uint32_t                    iHeight;                                        // [in]         Surface Height
    MOS_FORMAT                  format;                                         // [in]         Surface Format
    void                        *pData;                                          // [in/out]     Pointer to data
    uint32_t                    iPitch;                                         // [out]        Pitch
    uint32_t                    iPhysicalSize;                                  // [out]        Physical size
    uint32_t                    dwHandle;                                       // [in/out]     Handle
} CM_HAL_SURFACE2D_UP_PARAM, *PCM_HAL_SURFACE2D_UP_PARAM;


//------------------------------------------------------------------------------
//| CM 2D Get Surface Information Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_INFO_PARAM
{
    uint32_t                    iWidth;                                         // [out]         Surface Width
    uint32_t                    iHeight;                                        // [out]         Surface Height
    MOS_FORMAT                  format;                                         // [out]         Surface Format
    uint32_t                    iPitch;                                         // [out]         Pitch
    UMD_RESOURCE                SurfaceHandle ;                                 // [in]          Driver Handler
    uint32_t                    SurfaceAllocationIndex;                         // [in]          KMD Driver Handler
} CM_HAL_SURFACE2D_INFO_PARAM, *PCM_HAL_SURFACE2D_INFO_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Set Surface State Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_SURFACE_STATE_PARAM
{
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t pitch;
    uint16_t memory_object_control;
    uint32_t surface_x_offset;
    uint32_t surface_y_offset;
} CM_HAL_SURFACE2D_SURFACE_STATE_PARAM, *PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Lock/Unlock Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM
{
    uint32_t                    iWidth;                                         // [in]         Surface Width
    uint32_t                    iHeight;                                        // [in]         Surface Height
    MOS_FORMAT                  format;                                         // [in]         Surface Format
    void                        *pData;                                          // [in/out]     Pointer to data
    uint32_t                    iPitch;                                         // [out]        Pitch
    uint32_t                    iLockFlag;                                      // [out]        lock flag
    uint32_t                    dwHandle;                                       // [in/out]     Handle
} CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM, *PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM;

//------------------------------------------------------------------------------
//|  CM 2D Surface Compression Parameter
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_COMPRESSION_PARAM
{
    uint32_t                    dwHandle;
    MEMCOMP_STATE               MmcMode;
}CM_HAL_SURFACE2D_COMPRESSIOM_PARAM, *PCM_HAL_SURFACE2D_COMPRESSION_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_PARAM
{
    uint32_t                    isAllocatedbyCmrtUmd;                           // [in]         Flag for Cmrt@umd Created Surface
    PMOS_RESOURCE               pMosResource;                                   // [in]         Mos resource
    uint32_t                    iWidth;                                         // [in]         Surface Width
    uint32_t                    iHeight;                                        // [in]         Surface Height
    MOS_FORMAT                  format;                                         // [in]         Surface Format
    void                        *pData;                                          // [in]         PData
    uint32_t                    iPitch;                                         // [out]        Pitch
    uint32_t                    dwHandle;                                       // [in/out]     Handle
} CM_HAL_SURFACE2D_PARAM, *PCM_HAL_SURFACE2D_PARAM;

//------------------------------------------------------------------------------
//| CM 3D Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_3DRESOURCE_PARAM
{
    uint32_t                    iHeight;                                        // [in]       Surface Height
    uint32_t                    iWidth;                                         // [in]       Surface Width
    uint32_t                    iDepth;                                         // [in]       Surface Depth
    MOS_FORMAT                  format;
    void                        *pData;                                          // [in/out]   Pointer to data
    uint32_t                    dwHandle;                                       // [in/out]   Handle
    uint32_t                    iLockFlag;                                      // [in]       Lock flag
    uint32_t                    pPitch;                                         // [out]      Pitch of Resource
    uint32_t                    dwQPitch;                                       // [out]      QPitch of the Resource
    bool                        bQPitchEnable;                                  // [out]      if QPitch is supported by hw
} CM_HAL_3DRESOURCE_PARAM, *PCM_HAL_3DRESOURCE_PARAM;

//------------------------------------------------------------------------------
//| HalCm Kernel Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_SETUP
{
    RENDERHAL_KERNEL_PARAM  RenderParam;
    MHW_KERNEL_PARAM        CacheEntry;
} CM_HAL_KERNEL_SETUP, *PCM_HAL_KERNEL_SETUP;


//------------------------------------------------------------------------------
//| CM vebox settings
//------------------------------------------------------------------------------
typedef struct _CM_VEBOX_SETTINGS
{
    bool    bCmDIEnable;
    bool    bCmDnDiFirstFrame;
    bool    bCmIECPEnable;
    bool    bCmDNEnable;
    uint32_t DIOutputFrames;
    bool    bDemosaicEnable;
    bool    bVignetteEnable;
    bool    bHotPixelFilterEnable;
}CM_VEBOX_SETTINGS;

#define VEBOX_SURFACE_NUMBER                 (16)     //MAX

#define CM_VEBOX_PARAM_PAGE_SIZE   0x1000
#define CM_VEBOX_PARAM_PAGE_NUM    5 

typedef struct _CM_AVS_TABLE_STATE_PARAMS {
    bool               BypassXAF;
    bool               BypassYAF;
    uint8_t            DefaultSharpLvl;
    uint8_t            maxDerivative4Pixels;
    uint8_t            maxDerivative8Pixels;
    uint8_t            transitionArea4Pixels;
    uint8_t            transitionArea8Pixels;
    CM_AVS_COEFF_TABLE Tbl0X[ NUM_POLYPHASE_TABLES ];
    CM_AVS_COEFF_TABLE Tbl0Y[ NUM_POLYPHASE_TABLES ];
    CM_AVS_COEFF_TABLE Tbl1X[ NUM_POLYPHASE_TABLES ];
    CM_AVS_COEFF_TABLE Tbl1Y[ NUM_POLYPHASE_TABLES ];
    bool               bEnableRGBAdaptive;
    bool               bAdaptiveFilterAllChannels;
} CM_AVS_TABLE_STATE_PARAMS, *PCM_AVS_TABLE_STATE_PARAMS;

typedef  struct _CM_HAL_AVS_PARAM {
    MHW_SAMPLER_STATE_AVS_PARAM avs_state;             // [in] avs state table.
    CM_AVS_TABLE_STATE_PARAMS    avs_table;             // [in] avs table.
} CM_HAL_AVS_PARAM, *PCM_HAL_AVS_PARAM;

/*
 *  CONVOLVE STATE DATA STRUCTURES
 */

typedef struct _CM_HAL_CONVOLVE_COEFF_TABLE{
    float   FilterCoeff_0_0;
    float   FilterCoeff_0_1;
    float   FilterCoeff_0_2;
    float   FilterCoeff_0_3;
    float   FilterCoeff_0_4;
    float   FilterCoeff_0_5;
    float   FilterCoeff_0_6;
    float   FilterCoeff_0_7;
    float   FilterCoeff_0_8;
    float   FilterCoeff_0_9;
    float   FilterCoeff_0_10;
    float   FilterCoeff_0_11;
    float   FilterCoeff_0_12;
    float   FilterCoeff_0_13;
    float   FilterCoeff_0_14;
    float   FilterCoeff_0_15;
    float   FilterCoeff_0_16;
    float   FilterCoeff_0_17;
    float   FilterCoeff_0_18;
    float   FilterCoeff_0_19;
    float   FilterCoeff_0_20;
    float   FilterCoeff_0_21;
    float   FilterCoeff_0_22;
    float   FilterCoeff_0_23;
    float   FilterCoeff_0_24;
    float   FilterCoeff_0_25;
    float   FilterCoeff_0_26;
    float   FilterCoeff_0_27;
    float   FilterCoeff_0_28;
    float   FilterCoeff_0_29;
    float   FilterCoeff_0_30;
    float   FilterCoeff_0_31;
}CM_HAL_CONVOLVE_COEFF_TABLE;

#define CM_NUM_CONVOLVE_ROWS_SKL 31
typedef struct _CM_HAL_CONVOLVE_STATE_MSG{
        bool CoeffSize; //true 16-bit, false 8-bit
        uint8_t SclDwnValue; //Scale down value
        uint8_t Width; //Kernel Width
        uint8_t Height; //Kernel Height   
        //SKL mode
        bool isVertical32Mode;
        bool isHorizontal32Mode;
        bool skl_mode;  // new added
        CM_CONVOLVE_SKL_TYPE nConvolveType;
        CM_HAL_CONVOLVE_COEFF_TABLE Table[ CM_NUM_CONVOLVE_ROWS_SKL ];
} CM_HAL_CONVOLVE_STATE_MSG;

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_HAL_MISC_STATE {
    //uint32_t 0
    union{
        struct{
            uint32_t Height    : 4;
            uint32_t Width     : 4;
            uint32_t Reserved  : 8;
            uint32_t Row0      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW0;

    //uint32_t 1
    union{
        struct{
            uint32_t Row1      : 16;
            uint32_t Row2      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW1;

    //uint32_t 2
    union{
        struct{
            uint32_t Row3      : 16;
            uint32_t Row4      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW2;

    //uint32_t 3
    union{
        struct{
            uint32_t Row5      : 16;
            uint32_t Row6      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW3;

    //uint32_t 4
    union{
        struct{
            uint32_t Row7      : 16;
            uint32_t Row8      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW4;

    //uint32_t 5
    union{
        struct{
            uint32_t Row9      : 16;
            uint32_t Row10      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW5;

    //uint32_t 6
    union{
        struct{
            uint32_t Row11      : 16;
            uint32_t Row12      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW6;

    //uint32_t 7
    union{
        struct{
            uint32_t Row13      : 16;
            uint32_t Row14      : 16;
        };
        struct{
            uint32_t value;
        };
    }DW7;
} CM_HAL_MISC_STATE;

typedef struct _CM_HAL_SAMPLER_8X8_STATE
{
    CM_HAL_SAMPLER_8X8_TYPE         stateType;             // [in] types of Sampler8x8
    union {
        CM_HAL_AVS_PARAM            avs_param;             // [in] avs parameters.
        CM_HAL_CONVOLVE_STATE_MSG   convolve_state;
        CM_HAL_MISC_STATE           misc_state;
        //Will add other sampler8x8 types later.
    };
} CM_HAL_SAMPLER_8X8_STATE;

typedef struct _CM_HAL_SAMPLER_8X8_TABLE
{
    CM_HAL_SAMPLER_8X8_TYPE         stateType;               // [in] types of Sampler8x8
    MHW_SAMPLER_AVS_TABLE_PARAM     mhwSamplerAvsTableParam; // [in] Sampler8x8 avs table
} CM_HAL_SAMPLER_8X8_TABLE, *PCM_HAL_SAMPLER_8X8_TABLE;

typedef struct _CM_HAL_SAMPLER_8X8_PARAM
{
    CM_HAL_SAMPLER_8X8_STATE          sampler8x8State;      // [in]  Sampler8x8 states
    uint32_t                          dwHandle;             // [out] Handle 
} CM_HAL_SAMPLER_8X8_PARAM, *PCM_HAL_SAMPLER_8X8_PARAM;

typedef struct _CM_HAL_SAMPLER_8X8_ENTRY{
    CM_HAL_SAMPLER_8X8_TABLE       sampler8x8State;       // [in]  Sampler8x8 states
    bool                           bInUse;                // [out] If the entry has been occupied.
} CM_HAL_SAMPLER_8X8_ENTRY, *PCM_HAL_SAMPLER_8X8_ENTRY;

typedef struct _CM_HAL_BUFFER_SURFACE_STATE_ENTRY
{
    uint32_t iSurfaceStateSize;
    uint32_t iSurfaceStateOffset;
    uint16_t wSurfaceStateMOCS;
} CM_HAL_BUFFER_SURFACE_STATE_ENTRY, *PCM_HAL_BUFFER_SURFACE_STATE_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM Buffer Table
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BUFFER_ENTRY
{
    MOS_RESOURCE                        OsResource;                                         // [in] Pointer to OS Resource
    uint32_t                            iSize;                                              // [in] Size of Buffer
    void                                *pAddress;                                           // [in] SVM address
    void                                *pGmmResourceInfo;                                   // [out] GMM resource info
    bool                                isAllocatedbyCmrtUmd;                               // [in] Whether Surface allocated by CMRT
    uint16_t                            memObjCtl;                                          // [in] MOCS value set from CMRT
    CM_HAL_BUFFER_SURFACE_STATE_ENTRY   surfaceStateEntry[CM_HAL_MAX_NUM_BUFFER_ALIASES];   // [in] width/height of surface to be used in surface state
} CM_HAL_BUFFER_ENTRY, *PCM_HAL_BUFFER_ENTRY; 

//------------------------------------------------------------------------------
//| HAL CM 2D UP Table
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_UP_ENTRY
{
    MOS_RESOURCE                OsResource;                                     // [in] Pointer to OS Resource
    uint32_t                    iWidth;                                         // [in] Width of Surface
    uint32_t                    iHeight;                                        // [in] Height of Surface
    MOS_FORMAT                  format;                                         // [in] Format of Surface
    void                        *pGmmResourceInfo;                               // [out] GMM resource info
    uint16_t                    memObjCtl;                                      // [in] MOCS value set from CMRT
} CM_HAL_SURFACE2D_UP_ENTRY, *PCM_HAL_SURFACE2D_UP_ENTRY; 

typedef struct _CM_HAL_SURFACE_STATE_ENTRY
{
    uint32_t iSurfaceStateWidth;
    uint32_t iSurfaceStateHeight;
} CM_HAL_SURFACE_STATE_ENTRY, *PCM_HAL_SURFACE_STATE_ENTRY;

typedef struct _CM_HAL_VME_ARG_VALUE
{
    uint32_t                    fwRefNum;
    uint32_t                    bwRefNum;
    CM_HAL_SURFACE_STATE_ENTRY  surfStateParam;
    uint32_t                    curSurface;
    // IMPORTANT: in realization, this struct is always followed by ref surfaces array, fw followed by bw
    // Please use CmSurfaceVme::GetVmeCmArgSize() to allocate the memory for this structure
}CM_HAL_VME_ARG_VALUE, *PCM_HAL_VME_ARG_VALUE;

inline uint32_t *findRefInVmeArg(PCM_HAL_VME_ARG_VALUE pValue)
{
    return (uint32_t *)(pValue+1);
}

inline uint32_t *findFwRefInVmeArg(PCM_HAL_VME_ARG_VALUE pValue)
{
    return (uint32_t *)(pValue+1);
}

inline uint32_t *findBwRefInVmeArg(PCM_HAL_VME_ARG_VALUE pValue)
{
    return &((uint32_t *)(pValue+1))[pValue->fwRefNum];
}

inline uint32_t getVmeArgValueSize(PCM_HAL_VME_ARG_VALUE pValue)
{
    return sizeof(CM_HAL_VME_ARG_VALUE) + (pValue->fwRefNum + pValue->bwRefNum) * sizeof(uint32_t);
}

inline uint32_t getSurfNumFromArgArraySize(uint32_t argArraySize, uint32_t argNum)
{
    return (argArraySize - argNum*sizeof(CM_HAL_VME_ARG_VALUE))/sizeof(uint32_t) + argNum; // substract the overhead of the structure to get number of references, then add the currenct surface
}


//------------------------------------------------------------------------------
//| HAL CM 2D Table
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_ENTRY
{
    MOS_RESOURCE                OsResource;                                     // [in] Pointer to OS Resource
    uint32_t                    iWidth;                                         // [in] Width of Surface
    uint32_t                    iHeight;                                        // [in] Height of Surface
    MOS_FORMAT                  format;                                         // [in] Format of Surface
    void                        *pGmmResourceInfo;                               // [out] GMM resource info
    uint32_t                    isAllocatedbyCmrtUmd;                           // [in] Whether Surface allocated by CMRT
    uint32_t                    iSurfaceStateWidth;                             // [in] Width of Surface to be set in surface state
    uint32_t                    iSurfaceStateHeight;                            // [in] Height of Surface to be set in surface state
    bool                        bReadSync[CM_HAL_GPU_CONTEXT_COUNT];              // [in] Used in on demand sync for each gpu context
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM  surfaceStateParam[CM_HAL_MAX_NUM_2D_ALIASES];   // [in] width/height of surface to be used in surface state
    MHW_ROTATION                rotationFlag;
    int32_t                     chromaSiting;
    CM_FRAME_TYPE               frameType;
    uint16_t                    memObjCtl;                                      // [in] MOCS value set from CMRT
} CM_HAL_SURFACE2D_ENTRY, *PCM_HAL_SURFACE2D_ENTRY; 

//------------------------------------------------------------------------------
//| HAL CM Buffer Table
//------------------------------------------------------------------------------
typedef struct _CM_HAL_3DRESOURCE_ENTRY
{
    MOS_RESOURCE           OsResource;                                     // [in] Pointer to OS Resource
    uint32_t               iWidth;                                         // [in] Width of Surface
    uint32_t               iHeight;                                        // [in] Height of Surface
    uint32_t               iDepth;                                         // [in] Depth of Surface
    MOS_FORMAT             format;                                         // [in] Format of Surface
    uint16_t               memObjCtl;                                      // [in] MOCS value set from CMRT                                        
} CM_HAL_3DRESOURCE_ENTRY, *PCM_HAL_3DRESOURCE_ENTRY; 


//*-----------------------------------------------------------------------------
//| TimeStamp Resource. Used for storing task begin and end timestamps
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_TS_RESOURCE
{
    MOS_RESOURCE                OsResource;                                     // [in] OS Resource
    bool                        bLocked;                                        // [in] Locked Flag
    uint8_t                     *pData;                                          // [in] Linear Data
} CM_HAL_TS_RESOURCE, *PCM_HAL_TS_RESOURCE;

//------------------------------------------------------------------------------
//| HAL CM Struct for a multiple usage mapping from OSResource to BTI states
//------------------------------------------------------------------------------
typedef struct _CM_HAL_MULTI_USE_BTI_ENTRY
{
    union
    {
    struct
    {
        uint32_t RegularSurfIndex : 8;
        uint32_t SamplerSurfIndex : 8;
        uint32_t VmeSurfIndex : 8;
        uint32_t Sampler8x8SurfIndex : 8;
    };
    struct
    {
        uint32_t Value;
    };
    } BTI;

    struct 
    {
        void  *RegularBTIEntryPos;
        void  *SamplerBTIEntryPos;
        void  *VmeBTIEntryPos;
        void  *Sampler8x8BTIEntryPos;
    } BTITableEntry;
    uint32_t nPlaneNumber;
} CM_HAL_MULTI_USE_BTI_ENTRY, *PCM_HAL_MULTI_USE_BTI_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM Struct for a table entry for state buffer
//------------------------------------------------------------------------------
typedef struct _CM_HAL_STATE_BUFFER_ENTRY
{
    void                    *kernel_ptr;
    uint32_t                state_buffer_index;
    CM_STATE_BUFFER_TYPE    state_buffer_type;
    uint32_t                state_buffer_size;
    uint64_t                state_buffer_va_ptr;
    PRENDERHAL_MEDIA_STATE  media_state_ptr;
} CM_HAL_STATE_BUFFER_ENTRY;

typedef struct _CM_HAL_STATE *PCM_HAL_STATE;

//------------------------------------------------------------------------------
//| HAL CM Struct for a L3 settings
//------------------------------------------------------------------------------
typedef struct CmHalL3Settings
{
    bool    enable_slm;     // Enable SLM cache configuration
    bool    override_settings;      // Override cache settings

                                   // Override values
    bool    l3_caching_enabled;

    bool    cntl_reg_override;
    bool    cntl_reg2_override;
    bool    cntl_reg3_override;
    bool    sqc_reg1_override;
    bool    sqc_reg4_override;
    bool    lra1_reg_override;
    bool    tc_cntl_reg_override;
    bool    alloc_reg_override;

    unsigned long   cntl_reg;
    unsigned long   cntl_reg2;
    unsigned long   cntl_reg3;
    unsigned long   sqc_reg1;
    unsigned long   sqc_reg4;
    unsigned long   lra1_reg;
    unsigned long   tc_cntl_reg;
    unsigned long   alloc_reg;
} *PCmHalL3Settings;

//------------------------------------------------------------------------------
//| HAL CM State
//------------------------------------------------------------------------------
typedef struct _CM_HAL_STATE
{
    // Internal/private structures
    PLATFORM                    Platform;
    MEDIA_FEATURE_TABLE         *pSkuTable;
    MEDIA_WA_TABLE              *pWaTable;
    PMOS_INTERFACE              pOsInterface;                                   // OS Interface                                 [*]
    PRENDERHAL_INTERFACE        pRenderHal;                                     // Render Engine Interface                      [*]
    MhwVeboxInterface           *pVeboxInterface;                               // Vebox Interface
    MhwCpInterface*             pCpInterface;                                  // Cp  Interface
    PMHW_BATCH_BUFFER           pBatchBuffers;                                  // Array of Batch Buffers                       [*]
    PCM_HAL_TASK_PARAM          pTaskParam;                                     // Pointer to Task Param                        [*]
    PCM_HAL_TASK_TIMESTAMP      pTaskTimeStamp;                                 // Pointer to Task Param
    CM_HAL_TS_RESOURCE          TsResource;                                     // Resource to store timestamps                 [*]
    CM_HAL_TS_RESOURCE          SipResource;                                    // Resource to store debug info                 [*]
    MOS_RESOURCE                CSRResource;                                    // Resource to store CSR info               
    void                        *pTableMem;                                      // Single Memory for all lookup and temp tables [*]
    CM_HAL_HINT_TASK_INDEXES    HintIndexes;                                    // Indexes for EU Saturation API
    bool                        bRequestSingleSlice;                            // Requests single slice for life of CM device
    bool                        bDisabledMidThreadPreemption;                  // set the flag to indicate to disable the midthread preemption
    bool                        bEnabledKernelDebug;                            // set the flag to indicate to enable SIP debugging
    PCMLOOKUP_ENTRY             pSurf2DTable;                                   // Surface registration lookup entries
    PCM_HAL_SURFACE2D_ENTRY     pUmdSurf2DTable;                                // Surface2D entries used by CMRT@Driver
    PCM_HAL_BUFFER_ENTRY        pBufferTable;                                   // Buffer registration table
    PCM_HAL_SURFACE2D_UP_ENTRY  pSurf2DUPTable;                                 // Buffer registration table
    PCM_HAL_3DRESOURCE_ENTRY    pSurf3DTable;                                   // 3D surface registration table
    PMHW_SAMPLER_STATE_PARAM    pSamplerTable;                              // Sampler table
    CM_SAMPLER_STATISTICS       SamplerStatistics;
    PCM_HAL_SAMPLER_8X8_ENTRY   pSampler8x8Table;                               // Sampler 8x8 table
    char                        *pTaskStatusTable;                               // Table for task status
    int32_t                     iCurrentTaskEntry;                              // Current task status entry id
    PCM_HAL_MULTI_USE_BTI_ENTRY pBT2DIndexTable;                                // Table to store Used 2D binding indexes (temporary)
    PCM_HAL_MULTI_USE_BTI_ENTRY pBT2DUPIndexTable;                              // Table to store Used 2D binding indexes (temporary)
    PCM_HAL_MULTI_USE_BTI_ENTRY pBT3DIndexTable;                                // Table to store Used 3D binding indexes (temporary)
    PCM_HAL_MULTI_USE_BTI_ENTRY pBTBufferIndexTable;                            // Table to store Buffer Binding indexes (temporary)
    char                        *pSamplerIndexTable;                             // Table to store Used Sampler indexes (temporary)
    char                        *pVmeIndexTable;                                 // Table to store Used VME indexes (temporary)
    char                        *pSampler8x8IndexTable;                          // Table to store Used Sampler8x8 indexes (temporary)

    CMSURFACE_REG_TABLE         SurfaceRegTable;                                // Surface registration table
    CM_HAL_DEVICE_PARAM         CmDeviceParam;                                  // Device Param
    RENDERHAL_KRN_ALLOCATION     KernelParams_RenderHal;                        // RenderHal Kernel Setup
    MHW_KERNEL_PARAM            KernelParams_Mhw;                               // MHW Kernel setup
    int32_t                     iNumBatchBuffers;                               // Number of batch buffers
    uint32_t                    dwDummyArg;                                     // Dummy Argument for no argument kernel
    CM_HAL_MAX_HW_THREAD_VALUES MaxHWThreadValues;                              // Maximum number of hardware threads values
    MHW_VFE_SCOREBOARD          ScoreboardParams;                               // Scoreboard Parameters
    MHW_WALKER_PARAMS           WalkerParams;                                   // Walker Parameters
    void                        *pResourceList;                                  // List of resource handles (temporary) NOTE: Only use this for enqueue

    bool                        bNullHwRenderCm;                                // Null rendering flag for Cm function 
    HMODULE                     hLibModule;                                     // module handle pointing to the dynamically opened library

    bool                        bDynamicStateHeap;                              // Enable Dynamic State Heap
    uint32_t                    dwDSHKernelCacheHit;                            // Kernel Cache hit count
    uint32_t                    dwDSHKernelCacheMiss;                              // Kernel Cache miss count

    RENDERHAL_SURFACE           cmVeboxSurfaces[CM_HAL_MAX_VEBOX_SURF_NUM];     // cm vebox surfaces
    CM_VEBOX_SETTINGS           cmVeboxSettings;                                // cm vebox settings
    RENDERHAL_SURFACE           cmVebeboxParamSurf;
    uint32_t                    cmDebugBTIndex;                                 // cm Debug BT index
    
    void                        *pDrmVMap;                                       //for libdrm's patched function "drm_intel_bo_from_vmapping"

    CM_POWER_OPTION             PowerOption;                                    // Power option
    bool                        bEUSaturationEnabled;                           // EU saturation enabled
#ifdef GSH_DYNAMIC
    int32_t                     nNumKernelsInGSH;                               // current kernel number in GSH
    int32_t                     *pTotalKernelSize;                               // Total size table of every kernel in GSH kernel entries 
#endif

    MOS_GPU_CONTEXT             GpuContext;                                     // GPU Context 
    uint32_t                    nSurfaceArraySize;                              // size of surface array used for 2D surface alias

    bool                        bVtuneProfilerOn;                               // Vtune profiling on or not
    bool                        bCBBEnabled;                                    // if conditional batch buffer enabled

    uint32_t                    currentPerfTagIndex[MAX_COMBINE_NUM_IN_PERFTAG];
    std::map<std::string, int>  *pPerfTagIndexMap[MAX_COMBINE_NUM_IN_PERFTAG];  // mapping from kernel name to perf tag

    PCM_HAL_GENERIC             pCmHalInterface;                                // pointer to genX interfaces          

    std::map< void *, CM_HAL_STATE_BUFFER_ENTRY > *state_buffer_list_ptr;        // table of bounded state buffer and kernel ptr

    CmHalL3Settings             l3_settings;

    bool                        use_new_sampler_heap;
//------------------------------------------------------------------------------
// Macros to replace HR macros in oscl.h
//------------------------------------------------------------------------------
#ifndef CM_CHK_MOSSTATUS
#define CM_CHK_MOSSTATUS(_stmt)                                                 \
{                                                                               \
    hr = (MOS_STATUS)(_stmt);                                                   \
    if (hr != MOS_STATUS_SUCCESS)                                               \
    {                                                                           \
        CM_NORMALMESSAGE("hr check failed.");                                   \
        goto finish;                                                            \
    }                                                                           \
}
#endif

#ifndef CM_CHK_NULL_RETURN_MOSSTATUS
#define CM_CHK_NULL_RETURN_MOSSTATUS(_ptr)                                      \
{                                                                               \
    if ((_ptr) == nullptr)                                                         \
    {                                                                           \
        CM_ASSERTMESSAGE("Invalid (nullptr) Pointer");                             \
        hr = MOS_STATUS_NULL_POINTER;                                           \
        goto finish;                                                            \
    }                                                                           \
}
#endif // CM_CHK_NULL_RETURN_MOSSTATUS

#ifndef CM_HRESULT2MOSSTATUS_AND_CHECK
#define CM_HRESULT2MOSSTATUS_AND_CHECK(_stmt)                                   \
{                                                                               \
    hr = (MOS_STATUS)OsResultToMOS_Status(_stmt);                               \
    if (hr != MOS_STATUS_SUCCESS)                                               \
    {                                                                           \
        CM_NORMALMESSAGE("hr check failed.");                                   \
        goto finish;                                                            \
    }                                                                           \
}
#endif

#ifndef CM_CHK_MOSSTATUS_RETURN
#define CM_CHK_MOSSTATUS_RETURN(_stmt)                                          \
{                                                                               \
    hr = (MOS_STATUS)(_stmt);                                                   \
    if (hr != MOS_STATUS_SUCCESS)                                               \
    {                                                                           \
        return hr;                                                              \
    }                                                                           \
}
#endif

#ifndef CM_CHK_NULL_RETURN
#define CM_CHK_NULL_RETURN(_ptr)                                                \
{                                                                               \
    if ((_ptr) == nullptr)                                                         \
    {                                                                           \
        return MOS_STATUS_NULL_POINTER;                                         \
    }                                                                           \
}
#endif

#define CM_PUBLIC_ASSERT(_expr)                                                 \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_PUBLIC, _expr)

#define CM_PUBLIC_ASSERTMESSAGE(_message, ...)                                  \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

    //********************************************************************************
    // Export Interface methods called by CMRT@UMD <START>
    //********************************************************************************
    MOS_STATUS (* pfnCmAllocate)
    (   PCM_HAL_STATE               pState);

    MOS_STATUS (* pfnGetMaxValues)
        (   PCM_HAL_STATE           pState,
            PCM_HAL_MAX_VALUES      pMaxValues);

    MOS_STATUS (* pfnGetMaxValuesEx)
        (   PCM_HAL_STATE           pState,
            PCM_HAL_MAX_VALUES_EX   pMaxValuesEx);

    MOS_STATUS (* pfnExecuteTask) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_EXEC_TASK_PARAM     pParam);

    MOS_STATUS (* pfnExecuteGroupTask) 
    (   PCM_HAL_STATE                   pState,
        PCM_HAL_EXEC_GROUP_TASK_PARAM   pParam);

    MOS_STATUS (* pfnExecuteVeboxTask) 
    (   PCM_HAL_STATE                   pState,
        PCM_HAL_EXEC_VEBOX_TASK_PARAM   pParam);

    MOS_STATUS (* pfnExecuteHintsTask)
    (   PCM_HAL_STATE                   pState,
        PCM_HAL_EXEC_HINTS_TASK_PARAM   pParam);

    MOS_STATUS (* pfnQueryTask) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_QUERY_TASK_PARAM    pParam);

    MOS_STATUS (* pfnRegisterKMDNotifyEventHandle) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_OSSYNC_PARAM        pParam);

    MOS_STATUS (* pfnRegisterSampler) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_SAMPLER_PARAM       pParam);

    MOS_STATUS (* pfnUnRegisterSampler) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle);

    MOS_STATUS (* pfnRegisterSampler8x8) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_SAMPLER_8X8_PARAM   pParam);

    MOS_STATUS (* pfnUnRegisterSampler8x8) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle);

    MOS_STATUS (*pfnAllocateBuffer) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_BUFFER_PARAM        pParam);

    MOS_STATUS (*pfnFreeBuffer) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle);

    MOS_STATUS (*pfnUpdateBuffer) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle,
        uint32_t                    dwSize);

    MOS_STATUS (*pfnUpdateSurface2D) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle,
        uint32_t                    dwWidth,
        uint32_t                    dwHeight);

    MOS_STATUS (*pfnUpdateSurface3D) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle,
        uint32_t                    dwWidth,
        uint32_t                    dwHeight,
        uint32_t                    dwDepth);

    MOS_STATUS (*pfnLockBuffer) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_BUFFER_PARAM        pParam);

    MOS_STATUS (*pfnUnlockBuffer) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_BUFFER_PARAM        pParam);

    MOS_STATUS (*pfnAllocateSurface2DUP) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_SURFACE2D_UP_PARAM  pParam);

    MOS_STATUS (*pfnFreeSurface2DUP) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle);

    MOS_STATUS (*pfnGetSurface2DPitchAndSize) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_SURFACE2D_UP_PARAM  pParam);

    MOS_STATUS (*pfnAllocate3DResource) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_3DRESOURCE_PARAM    pParam);

    MOS_STATUS (*pfnFree3DResource) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle);

    MOS_STATUS (*pfnLock3DResource) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_3DRESOURCE_PARAM    pParam);

    MOS_STATUS (*pfnUnlock3DResource) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_3DRESOURCE_PARAM    pParam);

    MOS_STATUS (*pfnAllocateSurface2D) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_SURFACE2D_PARAM     pParam);

    MOS_STATUS (*pfnFreeSurface2D) 
    (   PCM_HAL_STATE               pState,
        uint32_t                    dwHandle);

    MOS_STATUS (*pfnLock2DResource) 
    (   PCM_HAL_STATE                          pState,
        PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM    pParam);

    MOS_STATUS (*pfnUnlock2DResource) 
    (   PCM_HAL_STATE                          pState,
        PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM    pParam);

    MOS_STATUS (*pfnGetSurface2DTileYPitch) 
    (   PCM_HAL_STATE               pState,
        PCM_HAL_SURFACE2D_PARAM     pParam);

    MOS_STATUS (*pfnSetCaps)
    (   PCM_HAL_STATE               pState,
        PCM_HAL_MAX_SET_CAPS_PARAM  pParam);

    MOS_STATUS (*pfnGetGPUCurrentFrequency)
    (    PCM_HAL_STATE              pState,
        uint32_t                    *pGPUCurrentFreq);

    MOS_STATUS (*pfnSet2DSurfaceStateParam) 
    (   PCM_HAL_STATE                          pState,
        PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM  pParam,
        uint32_t                               iAliasIndex,
        uint32_t                               dwHandle);

    MOS_STATUS (*pfnSetBufferSurfaceStatePara) (
     PCM_HAL_STATE                            pState,
     PCM_HAL_BUFFER_SURFACE_STATE_PARAM       pParam);

    MOS_STATUS (*pfnSetSurfaceMOCS) (
     PCM_HAL_STATE                  pState,
     uint32_t                       dwHanlde,
     uint16_t                       mocs,
     uint32_t                       argKind);

    MOS_STATUS (*pfnSetPowerOption)
    (   PCM_HAL_STATE               pState,
        PCM_POWER_OPTION            pPowerOption);
   
    //********************************************************************************
    // Export Interface methods called by CMRT@UMD <END>
    //********************************************************************************

    //********************************************************************************
    // Internal interface methods called by CM HAL only <START>
    //********************************************************************************
    int32_t (*pfnGetTaskSyncLocation)
    (   int32_t                     iTaskId);

    MOS_STATUS (*pfnGetGpuTime)
    (   PCM_HAL_STATE               pState,
        uint64_t                    *piGpuTime);

    MOS_STATUS (*pfnConvertToQPCTime)
    (   uint64_t                    nanoseconds,
        LARGE_INTEGER               *QPCTime );

    MOS_STATUS (*pfnGetGlobalTime)
    (   LARGE_INTEGER               *pGlobalTime);

    MOS_STATUS (*pfnSendMediaWalkerState)
    (   PCM_HAL_STATE               pState,
        PCM_HAL_KERNEL_PARAM        pKernelParam,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (*pfnSendGpGpuWalkerState)
    (   PCM_HAL_STATE               pState,
        PCM_HAL_KERNEL_PARAM        pKernelParam,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (*pfnUpdatePowerOption)
    (   PCM_HAL_STATE               pState,
        PCM_POWER_OPTION            pPowerOption);

	MOS_STATUS (*pfnGetSipBinary)
    (   PCM_HAL_STATE               pState);

    MOS_STATUS(*pfnGetPlatformInfo)
    (   PCM_HAL_STATE               pState,
        PCM_PLATFORM_INFO           platformInfo,
        bool                        bEUSaturation);

    MOS_STATUS (*pfnSetSurfaceReadFlag) 
    ( PCM_HAL_STATE           pState,
      uint32_t                dwHandle,
      bool                    bReadSync);

    MOS_STATUS (*pfnSetVtuneProfilingFlag)
    (   PCM_HAL_STATE               pState,
        bool                        bVtuneOn);

    MOS_STATUS(*pfnReferenceCommandBuffer)
    (   PMOS_RESOURCE               pOsResource,
        void                        **ppCmdBuffer);

    MOS_STATUS(*pfnSetCommandBufferResource)
    (   PMOS_RESOURCE               pOsResource,
        void                        **ppCmdBuffer);

    MOS_STATUS(*pfnWriteGPUStatusTagToCMTSResource)
    (   PCM_HAL_STATE               pState,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        int32_t                     iTaskID,
        bool                        isVebox);

    MOS_STATUS(*pfnEnableTurboBoost)
    (   PCM_HAL_STATE               pState);

    MOS_STATUS(*pfnSetCompressionMode)
        (
        PCM_HAL_STATE               pState,
        CM_HAL_SURFACE2D_COMPRESSIOM_PARAM  MmcParam
        );

    bool (*pfnIsWASLMinL3Cache)(  );

    MOS_STATUS( *pfnInsertToStateBufferList )
        (
        PCM_HAL_STATE               pState,
        void                        *kernel_ptr,
        uint32_t                    state_buffer_index,
        CM_STATE_BUFFER_TYPE        state_buffer_type,
        uint32_t                    state_buffer_size,
        uint64_t                    state_buffer_va_ptr,
        PRENDERHAL_MEDIA_STATE      media_state_ptr );

    MOS_STATUS( *pfnDeleteFromStateBufferList )
        (
        PCM_HAL_STATE               pState,
        void                        *kernel_ptr );

    PRENDERHAL_MEDIA_STATE( *pfnGetMediaStatePtrForKernel )
        (
        PCM_HAL_STATE               pState,
        void                        *kernel_ptr );

    uint64_t( *pfnGetStateBufferVAPtrForSurfaceIndex )
        (
        PCM_HAL_STATE               pState,
        uint32_t                    surf_index );

    PRENDERHAL_MEDIA_STATE( *pfnGetMediaStatePtrForSurfaceIndex )
        (
        PCM_HAL_STATE               pState,
        uint32_t                    surf_index );

    uint64_t( *pfnGetStateBufferVAPtrForMediaStatePtr )
        (
        PCM_HAL_STATE               pState,
        PRENDERHAL_MEDIA_STATE      media_state_ptr );

    uint32_t( *pfnGetStateBufferSizeForKernel )
        (
        PCM_HAL_STATE               pState,
        void                        *kernel_ptr );

    CM_STATE_BUFFER_TYPE( *pfnGetStateBufferTypeForKernel )
        (
        PCM_HAL_STATE               pState,
        void                        *kernel_ptr );

    MOS_STATUS(*pfnCreateGPUContext)
        (
        PCM_HAL_STATE               pState,
        MOS_GPU_CONTEXT             gpu_context,
        MOS_GPU_NODE                gpu_node );

    //********************************************************************************
    // Internal interface methods called by CM HAL only <END>
    //********************************************************************************

#if (_DEBUG || _RELEASE_INTERNAL)
    bool                        bDumpCommandBuffer;                            //flag to enable command buffer dump
    bool                        bDumpCurbeData;                                //flag to enable curbe data dump
    bool                        bDumpSurfaceContent;                           //flag to enable surface content dump
    int32_t(*pfnInitDumpCommandBuffer)
        (
        PCM_HAL_STATE            pState);
    int32_t(*pfnDumpCommadBuffer)
        (
        PCM_HAL_STATE            pState,
        PMOS_COMMAND_BUFFER      pCmdBuffer,
        int                      offsetSurfaceState,
        size_t                   sizeOfSurfaceState);
#endif //(_DEBUG || _RELEASE_INTERNAL)

} CM_HAL_STATE, *PCM_HAL_STATE;


typedef struct _CM_HAL_MI_REG_OFFSETS 
{
    uint32_t TimeStampOffset;
    uint32_t GPROffset;
} CM_HAL_MI_REG_OFFSETS, *PCM_HAL_MI_REG_OFFSETS;

//------------------------------------------------------------------------------
//| Functions
//------------------------------------------------------------------------------

// inline functions
//*-----------------------------------------------------------------------------
//| Purpose: Get a New Task ID for the new task. If not found, return error
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetNewTaskId(
    PCM_HAL_STATE       pState,                                                 // [in]  Pointer to HAL CM State
    int32_t             *piIndex)                                                // [out] Pointer to Task Index
{
    uint32_t i, j;
    uint32_t maxTasks;

    i = pState->iCurrentTaskEntry;
    maxTasks = pState->CmDeviceParam.iMaxTasks;

    for (j = maxTasks; j > 0; j--, i = (i + 1) % maxTasks)
    {
        if (pState->pTaskStatusTable[i] == CM_INVALID_INDEX)
        {
            *piIndex = i;
            pState->iCurrentTaskEntry = (i + 1) % maxTasks;
            return MOS_STATUS_SUCCESS;
        }
    }

    // Not found
    CM_ASSERTMESSAGE("Unable to find a free slot for Task.");
    return MOS_STATUS_UNKNOWN;
}

MOS_STATUS HalCm_Create(
    PMOS_CONTEXT            pOsDriverContext,
    PCM_HAL_CREATE_PARAM    pCmCreateParam,
    PCM_HAL_STATE           *pCmState);

void HalCm_Destroy(
    PCM_HAL_STATE           pState);

void HalCm_GetUserFeatureSettings(
    PCM_HAL_STATE           pState);

MOS_STATUS HalCm_GetSurfaceDetails(
    PCM_HAL_STATE                   pState,
    PCM_HAL_INDEX_PARAM             pIndexParam,
    uint32_t                        iBTIndex,
    MOS_SURFACE&                    MosSurface,
    int16_t                         globalSurface,
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry,
    uint32_t                        dwTempPlaneIndex,
    RENDERHAL_SURFACE_STATE_PARAMS  SurfaceParam,
    CM_HAL_KERNEL_ARG_KIND          argKind);

MOS_STATUS HalCm_AllocateTsResource(
    PCM_HAL_STATE           pState);

MOS_STATUS HalCm_AllocateTables(
    PCM_HAL_STATE           pState);

MOS_STATUS HalCm_Allocate(
    PCM_HAL_STATE           pState);

MOS_STATUS HalCm_SetupSipSurfaceState(
    PCM_HAL_STATE           pState,
    PCM_HAL_INDEX_PARAM     pIndexParam,
    int32_t                 iBindingTable);

//===============<Below are Os-dependent Private/Non-DDI Functions>============================================

void HalCm_OsInitInterface(
    PCM_HAL_STATE           pCmState);

MOS_STATUS HalCm_GetSurfaceAndRegister(
    PCM_HAL_STATE           pState, 
    PRENDERHAL_SURFACE      pRenderHalSurface,
    CM_HAL_KERNEL_ARG_KIND  surfKind,
    uint32_t                iIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_SendMediaWalkerState(
    PCM_HAL_STATE           pState,
    PCM_HAL_KERNEL_PARAM    pKernelParam,
    PMOS_COMMAND_BUFFER     pCmdBuffer);

MOS_STATUS HalCm_SendGpGpuWalkerState(
    PCM_HAL_STATE           pState,
    PCM_HAL_KERNEL_PARAM    pKernelParam,
    PMOS_COMMAND_BUFFER     pCmdBuffer);

//===============<Below are Os-non-dependent Private/Non-DDI Functions>=========================================

uint32_t HalCm_GetFreeBindingIndex(
    PCM_HAL_STATE           pState,
    PCM_HAL_INDEX_PARAM     pIndexParam,
    uint32_t                count);

void HalCm_PreSetBindingIndex(
    PCM_HAL_INDEX_PARAM     pIndexParam,
    uint32_t                start,
    uint32_t                end);

MOS_STATUS HalCm_Setup2DSurfaceStateWithBTIndex(
    PCM_HAL_STATE           pState,
    int32_t                 iBindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_SetupBufferSurfaceStateWithBTIndex(
    PCM_HAL_STATE           pState,
    int32_t                 iBindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_Setup2DSurfaceUPStateWithBTIndex(
    PCM_HAL_STATE           pState,
    int32_t                 iBindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_SetupSampler8x8SurfaceStateWithBTIndex(
    PCM_HAL_STATE           pState,
    int32_t                 iBindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch,
    CM_HAL_KERNEL_ARG_KIND  iKind,
    uint32_t                AddressControl );

MOS_STATUS HalCm_Setup3DSurfaceStateWithBTIndex(
    PCM_HAL_STATE           pState,
    int32_t                 iBindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex);

MOS_STATUS HalCm_SyncOnResource(
    PCM_HAL_STATE           pState,
    PMOS_SURFACE            pSurface,
    bool                    isWrite);

void HalCm_OsResource_Unreference(
    PMOS_RESOURCE          pOsResource);

void HalCm_OsResource_Reference(
    PMOS_RESOURCE            pOsResource);

MOS_STATUS HalCm_SetSurfaceReadFlag(
    PCM_HAL_STATE           pState,
    uint32_t                dwHandle);

MOS_STATUS HalCm_SetVtuneProfilingFlag(
    PCM_HAL_STATE           pState,
    bool                    bVtuneOn);

#if (_DEBUG || _RELEASE_INTERNAL)
int32_t HalCm_InitDumpCommandBuffer(
    PCM_HAL_STATE            pState);

int32_t HalCm_DumpCommadBuffer(
    PCM_HAL_STATE            pState,
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    int                      offsetSurfaceState,
    size_t                   sizeOfSurfaceState);
#endif //(_DEBUG || _RELEASE_INTERNAL)

MOS_STATUS HalCm_SendVecsStatusTag(
    PCM_HAL_STATE           pState,
    PMOS_COMMAND_BUFFER         pCmdBuffer);

MOS_STATUS HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(
    PRENDERHAL_SURFACE           pRenderHalSurface,
    PMHW_VEBOX_SURFACE_PARAMS    pMhwVeboxSurface);

bool HalCm_IsCbbEnabled(
    PCM_HAL_STATE                           pState);

int32_t HalCm_SyncKernel(
    PCM_HAL_STATE                           pState,
    uint32_t                                dwSync);

MOS_STATUS HalCm_GetGfxTextAddress(
    uint32_t                     addressMode,
    MHW_GFX3DSTATE_TEXCOORDMODE  *pGfxAddress);

MOS_STATUS HalCm_GetGfxMapFilter(
    uint32_t                     filterMode,
    MHW_GFX3DSTATE_MAPFILTER     *pGfxFilter);

MOS_STATUS HalCm_Unlock2DResource(
    PCM_HAL_STATE                           pState,  
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     pParam);

MOS_STATUS HalCm_Lock2DResource(
    PCM_HAL_STATE                           pState,              
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     pParam);

int32_t HalCm_GetTaskSyncLocation(
    int32_t             iTaskId);

MOS_STATUS HalCm_SetL3Cache(
    const L3ConfigRegisterValues            *pL3Values,
    PCmHalL3Settings                      pCmHalL3Cache );

MOS_STATUS HalCm_AllocateSipResource(PCM_HAL_STATE pState);

MOS_STATUS HalCm_AllocateCSRResource(PCM_HAL_STATE pState);

MOS_STATUS HalCm_OsAddArtifactConditionalPipeControl(
    PCM_HAL_MI_REG_OFFSETS pOffsets,
    PCM_HAL_STATE pState,
    PMOS_COMMAND_BUFFER pCmdBuffer,
    int32_t iSyncOffset,
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS pConditionalParams);

//!
//! \brief    Get the number of command buffers according to the max task number
//! \details  Get the number of command buffers according to the max task number
//!           the returned number will be passed to pfnCreateGpuContext()
//! \param    [in] pOsInterface
//!           pointer to OS interface
//! \param    [in] maxTaskNumber
//!           max number of task to support
//! \return   uint32_t
//!
uint32_t HalCm_GetNumCmdBuffers(PMOS_INTERFACE pOsInterface, uint32_t maxTaskNumber);

void HalCm_GetLegacyRenderHalL3Setting( CmHalL3Settings *l3_settings_ptr, RENDERHAL_L3_CACHE_SETTINGS *l3_settings_legacy_ptr );

MOS_STATUS HalCm_GetNumKernelsPerGroup(
    uint8_t     hintsBits,
    uint32_t    numKernels,
    uint32_t    *pNumKernelsPerGroup,
    uint32_t    *pNumKernelGroups,
    uint32_t    *pRemapKrnToGrp,
    uint32_t    *pRemapGrpToKrn
    );

MOS_STATUS HalCm_GetParallelGraphInfo(
    uint32_t                       maximum,
    uint32_t                       numThreads,
    uint32_t                       width,
    uint32_t                       height,
    PCM_HAL_PARALLELISM_GRAPH_INFO graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    bool                           bNoDependencyCase);

MOS_STATUS HalCm_SetDispatchPattern(
    CM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    uint32_t                       *pDispatchFreq
    );

MOS_STATUS HalCm_SetKernelGrpFreqDispatch(
    PCM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    PCM_HAL_KERNEL_GROUP_INFO       groupInfo,
    uint32_t                        numKernelGroups,
    uint32_t                        *pMinSteps);

MOS_STATUS HalCm_SetNoDependKernelDispatchPattern(
    uint32_t                        numThreads,
    uint32_t                        minSteps,
    uint32_t                        *pDispatchFreq);

MOS_STATUS HalCm_SetupSamplerState(
    PCM_HAL_STATE                   pState,
    PCM_HAL_KERNEL_PARAM            pKernelParam,
    PCM_HAL_KERNEL_ARG_PARAM        pArgParam,
    PCM_HAL_INDEX_PARAM             pIndexParam,
    int32_t                         iMediaID,
    uint32_t                        iThreadIndex,
    uint8_t                         *pBuffer);

MOS_STATUS HalCm_SetupBufferSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    int16_t                     globalSurface,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer);

MOS_STATUS HalCm_Setup2DSurfaceUPState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer);

MOS_STATUS HalCm_Setup2DSurfaceUPSamplerState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer);

MOS_STATUS HalCm_Setup2DSurfaceSamplerState(
    PCM_HAL_STATE              pState,
    PCM_HAL_KERNEL_ARG_PARAM   pArgParam,
    PCM_HAL_INDEX_PARAM        pIndexParam,
    int32_t                    iBindingTable,
    uint32_t                   iThreadIndex,
    uint8_t                    *pBuffer);

MOS_STATUS HalCm_Setup2DSurfaceState(
    PCM_HAL_STATE              pState,
    PCM_HAL_KERNEL_ARG_PARAM   pArgParam,
    PCM_HAL_INDEX_PARAM        pIndexParam,
    int32_t                    iBindingTable,
    uint32_t                   iThreadIndex,
    uint8_t                    *pBuffer);

MOS_STATUS HalCm_Setup3DSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer);

MOS_STATUS HalCm_SetupVmeSurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer);

MOS_STATUS HalCm_SetupSampler8x8SurfaceState(
    PCM_HAL_STATE               pState,
    PCM_HAL_KERNEL_ARG_PARAM    pArgParam,
    PCM_HAL_INDEX_PARAM         pIndexParam,
    int32_t                     iBindingTable,
    uint32_t                    iThreadIndex,
    uint8_t                     *pBuffer);

//*-----------------------------------------------------------------------------
//| Helper functions for EnqueueWithHints
//*-----------------------------------------------------------------------------

#endif  // __CM_HAL_H__
