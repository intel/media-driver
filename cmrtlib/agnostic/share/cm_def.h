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
#ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEF_H_
#define CMRTLIB_AGNOSTIC_SHARE_CM_DEF_H_

#include "cm_include.h"
#include "cm_def_os.h"

//CM runtime version returning to user
#define CM_1_0 100
#define CM_2_0 200
#define CM_2_1 201
#define CM_2_2 202
#define CM_2_3 203
#define CM_2_4 204
#define CM_3_0 300
#define CM_4_0 400
#define CM_5_0 500
#define CM_6_0 600
#define CM_7_0 700
#define CM_7_2 702 //for MDFRT API refreshment.
#define CURRENT_CM_VERSION  (CM_7_2)

//CM DDI version in UMD layer
#define CM_DDI_1_0 100
#define CM_DDI_1_1 101
#define CM_DDI_1_2 102
#define CM_DDI_1_3 103
#define CM_DDI_1_4 104
#define CM_DDI_2_0 200
#define CM_DDI_2_1 201
#define CM_DDI_2_2 202
#define CM_DDI_2_3 203
#define CM_DDI_2_4 204
#define CM_DDI_3_0 300
#define CM_DDI_4_0 400
#define CM_DDI_5_0 500
#define CM_DDI_6_0 600
#define CM_DDI_7_0 700
#define CM_DDI_7_2 702 //for MDFRT API refreshment.

//Error code definition
typedef enum _CM_RETURN_CODE
{
    CM_SUCCESS                                  = 0,
    /*
     * RANGE -1 ~ -9999 FOR EXTERNAL ERROR CODE
     */
    CM_FAILURE                                  = -1,
    CM_NOT_IMPLEMENTED                          = -2,
    CM_SURFACE_ALLOCATION_FAILURE               = -3,
    CM_OUT_OF_HOST_MEMORY                       = -4,
    CM_SURFACE_FORMAT_NOT_SUPPORTED             = -5,
    CM_EXCEED_SURFACE_AMOUNT                    = -6,
    CM_EXCEED_KERNEL_ARG_AMOUNT                 = -7,
    CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE           = -8,
    CM_INVALID_ARG_INDEX                        = -9,
    CM_INVALID_ARG_VALUE                        = -10,
    CM_INVALID_ARG_SIZE                         = -11,
    CM_INVALID_THREAD_INDEX                     = -12,
    CM_INVALID_WIDTH                            = -13,
    CM_INVALID_HEIGHT                           = -14,
    CM_INVALID_DEPTH                            = -15,
    CM_INVALID_COMMON_ISA                       = -16,
    CM_OPEN_VIDEO_DEVICE_FAILURE                = -17,
    CM_VIDEO_DEVICE_LOCKED                      = -18,  // Video device is already locked.
    CM_LOCK_VIDEO_DEVICE_FAILURE                = -19,  // Video device is not locked but can't be locked.
    CM_EXCEED_SAMPLER_AMOUNT                    = -20,
    CM_EXCEED_MAX_KERNEL_PER_ENQUEUE            = -21,
    CM_EXCEED_MAX_KERNEL_SIZE_IN_BYTE           = -22,
    CM_EXCEED_MAX_THREAD_AMOUNT_PER_ENQUEUE     = -23,
    CM_EXCEED_VME_STATE_G6_AMOUNT               = -24,
    CM_INVALID_THREAD_SPACE                     = -25,
    CM_EXCEED_MAX_TIMEOUT                       = -26,
    CM_JITDLL_LOAD_FAILURE                      = -27,
    CM_JIT_COMPILE_FAILURE                      = -28,
    CM_JIT_COMPILESIM_FAILURE                   = -29,
    CM_INVALID_THREAD_GROUP_SPACE               = -30,
    CM_THREAD_ARG_NOT_ALLOWED                   = -31,
    CM_INVALID_GLOBAL_BUFFER_INDEX              = -32,
    CM_INVALID_BUFFER_HANDLER                   = -33,
    CM_EXCEED_MAX_SLM_SIZE                      = -34,
    CM_JITDLL_OLDER_THAN_ISA                    = -35,
    CM_INVALID_HARDWARE_THREAD_NUMBER           = -36,
    CM_GTPIN_INVOKE_FAILURE                     = -37,
    CM_INVALIDE_L3_CONFIGURATION                = -38,
    CM_INVALID_TEXTURE2D_USAGE                  = -39,
    CM_INTEL_GFX_NOTFOUND                       = -40,
    CM_GPUCOPY_INVALID_SYSMEM                   = -41,
    CM_GPUCOPY_INVALID_WIDTH                    = -42,
    CM_GPUCOPY_INVALID_STRIDE                   = -43,
    CM_EVENT_DRIVEN_FAILURE                     = -44,
    CM_LOCK_SURFACE_FAIL                        = -45, // Lock surface failed
    CM_INVALID_GENX_BINARY                      = -46,
    CM_FEATURE_NOT_SUPPORTED_IN_DRIVER          = -47, // driver out-of-sync
    CM_QUERY_DLL_VERSION_FAILURE                = -48, //Fail in getting DLL file version
    CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL    = -49,
    CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL    = -50,
    CM_KERNELPAYLOAD_SETTING_FAILURE            = -51,
    CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX    = -52,
    CM_NOT_SET_KERNEL_ARGUMENT                  = -53,
    CM_GPUCOPY_INVALID_SURFACES                 = -54,
    CM_GPUCOPY_INVALID_SIZE                     = -55,
    CM_GPUCOPY_OUT_OF_RESOURCE                  = -56,
    CM_INVALID_VIDEO_DEVICE                     = -57,
    CM_SURFACE_DELAY_DESTROY                    = -58,
    CM_INVALID_VEBOX_STATE                      = -59,
    CM_INVALID_VEBOX_SURFACE                    = -60,
    CM_FEATURE_NOT_SUPPORTED_BY_HARDWARE        = -61,
    CM_RESOURCE_USAGE_NOT_SUPPORT_READWRITE     = -62,
    CM_MULTIPLE_MIPLEVELS_NOT_SUPPORTED         = -63,
    CM_INVALID_UMD_CONTEXT                      = -64,
    CM_INVALID_LIBVA_SURFACE                    = -65,
    CM_INVALID_LIBVA_INITIALIZE                 = -66,
    CM_KERNEL_THREADSPACE_NOT_SET               = -67,
    CM_INVALID_KERNEL_THREADSPACE               = -68,
    CM_KERNEL_THREADSPACE_THREADS_NOT_ASSOCIATED= -69,
    CM_KERNEL_THREADSPACE_INTEGRITY_FAILED      = -70,
    CM_INVALID_USERPROVIDED_GENBINARY           = -71,
    CM_INVALID_PRIVATE_DATA                     = -72,
    CM_INVALID_MOS_RESOURCE_HANDLE              = -73,
    CM_SURFACE_CACHED                           = -74,
    CM_SURFACE_IN_USE                           = -75,
    CM_INVALID_GPUCOPY_KERNEL                   = -76,
    CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN  = -77,
    CM_INVALID_MEDIA_WALKING_PATTERN            = -78,
    CM_FAILED_TO_ALLOCATE_SVM_BUFFER            = -79,
    CM_EXCEED_MAX_POWER_OPTION_FOR_PLATFORM     = -80,
    CM_INVALID_KERNEL_THREADGROUPSPACE          = -81,
    CM_INVALID_KERNEL_SPILL_CODE                = -82,
    CM_UMD_DRIVER_NOT_SUPPORTED                 = -83,
    CM_INVALID_GPU_FREQUENCY_VALUE              = -84,
    CM_SYSTEM_MEMORY_NOT_4KPAGE_ALIGNED         = -85,
    CM_KERNEL_ARG_SETTING_FAILED                = -86,
    CM_NO_AVAILABLE_SURFACE                     = -87,
    CM_VA_SURFACE_NOT_SUPPORTED                 = -88,
    CM_TOO_MUCH_THREADS                         = -89,
    CM_NULL_POINTER                             = -90,
    CM_EXCEED_MAX_NUM_2D_ALIASES                = -91,
    CM_INVALID_PARAM_SIZE                       = -92,
    CM_GT_UNSUPPORTED                           = -93,
    CM_GTPIN_FLAG_NO_LONGER_SUPPORTED           = -94,
    CM_PLATFORM_UNSUPPORTED_FOR_API             = -95,
    CM_TASK_MEDIA_RESET                         = -96,
    CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX    = -97,
    CM_EXCEED_MAX_NUM_BUFFER_ALIASES            = -98,
    CM_SYSTEM_MEMORY_NOT_4PIXELS_ALIGNED        = -99,
    CM_FAILED_TO_CREATE_CURBE_SURFACE           = -100,
    CM_INVALID_CAP_NAME                         = -101,
    CM_INVALID_PARAM_FOR_CREATE_QUEUE_EX        = -102,
    CM_INVALID_CREATE_OPTION_FOR_BUFFER_STATELESS = -103,
    CM_INVALID_KERNEL_ARG_POINTER                 = -104,
    CM_LOAD_LIBRARY_FAILED                        = -105,
    CM_NO_SUPPORTED_ADAPTER                       = -106,

    /*
     * RANGE -10000 ~ -19999 FOR INTERNAL ERROR CODE
     */
    CM_INTERNAL_ERROR_CODE_OFFSET               = -10000,

    /*
     * RANGE <=-20000 AREAD FOR MOST STATUS CONVERSION
     */
    CM_MOS_STATUS_CONVERTED_CODE_OFFSET         = -20000
} CM_RETURN_CODE;

#define __CODEGEN_UNIQUE(name)                  _NAME_LABEL_(name, __LINE__)
#define BITFIELD_RANGE( startbit, endbit )     ((endbit)-(startbit)+1)
#define BITFIELD_BIT(bit)                        1

#define CM_MIN_SURF_WIDTH       1
#define CM_MIN_SURF_HEIGHT      1
#define CM_MIN_SURF_DEPTH       2
#define CM_MAX_1D_SURF_WIDTH    0x80000000 // 2^31 2 GB

#define CM_MAX_3D_SURF_WIDTH 2048
#define CM_MAX_3D_SURF_HEIGHT 2048
#define CM_MAX_3D_SURF_DEPTH 2048

// hard ceiling
#define CM_MAX_OPTION_SIZE_IN_BYTE 512
#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE 256
#define CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE 256

#define CM_BUFFER_STATELESS_CREATE_OPTION_GFX_MEM 0
#define CM_BUFFER_STATELESS_CREATE_OPTION_SYS_MEM 1
#define CM_BUFFER_STATELESS_CREATE_OPTION_DEGAULT CM_BUFFER_STATELESS_CREATE_OPTION_GFX_MEM

struct CM_HAL_MAX_VALUES
{
    uint32_t maxTasks;                          // [in] Max Tasks
    uint32_t maxKernelsPerTask;                 // [in] Max kernels per task
    uint32_t maxKernelBinarySize;               // [in] Max kernel binary size
    uint32_t maxSpillSizePerHwThread;           // [in] Max spill size per thread
    uint32_t maxSamplerTableSize;               // [in] Max sampler table size
    uint32_t maxBufferTableSize;                // [in] Buffer table Size
    uint32_t max2DSurfaceTableSize;             // [in] Buffer table Size
    uint32_t max3DSurfaceTableSize;             // [in] Buffer table Size
    uint32_t maxArgsPerKernel;                  // [in] Max arguments per kernel
    uint32_t maxArgByteSizePerKernel;           // [in] Max argument size in byte per kernel
    uint32_t maxSurfacesPerKernel;              // [in] Max Surfaces Per Kernel
    uint32_t maxSamplersPerKernel;              // [in] Max Samplers per kernel
    uint32_t maxHwThreads;                      // [in] Max HW threads
    uint32_t maxUserThreadsPerTask;             // [in] Max user threads per task
    uint32_t maxUserThreadsPerTaskNoThreadArg;  // [in] Max user threads per task without a thread arg
};
typedef CM_HAL_MAX_VALUES *PCM_HAL_MAX_VALUES;

//---------------------------------------------------------------------------
//! HAL CM Max Values extention which has more entries which are not included
//! in CM_HAL_MAX_VALUES
//---------------------------------------------------------------------------
struct CM_HAL_MAX_VALUES_EX
{
    uint32_t max2DUPSurfaceTableSize;       // [in] Max 2D UP surface table Size
    uint32_t maxSampler8x8TableSize;        // [in] Max sampler 8x8 table size
    uint32_t maxCURBESizePerKernel;         // [in] Max CURBE size per kernel
    uint32_t maxCURBESizePerTask;           // [in] Max CURBE size per task
    uint32_t maxIndirectDataSizePerKernel;  // [in] Max indirect data size per kernel
    uint32_t maxUserThreadsPerMediaWalker;  // [in] Max user threads per media walker
    uint32_t maxUserThreadsPerThreadGroup;  // [in] Max user threads per thread group
};
typedef CM_HAL_MAX_VALUES_EX *PCM_HAL_MAX_VALUES_EX;

class CLock
{
public:
    CLock(CSync &refSync) : m_refSync(refSync) { Lock(); }
    ~CLock() { Unlock(); }

private:
    CSync &m_refSync;                     // Synchronization object

    CLock(const CLock &refcSource);
    CLock &operator=(const CLock &refcSource);
    void Lock() { m_refSync.Acquire(); }
    void Unlock() { m_refSync.Release(); }
};

typedef struct _CM_SAMPLER_STATE
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;
    CM_TEXTURE_ADDRESS_TYPE addressU;
    CM_TEXTURE_ADDRESS_TYPE addressV;
    CM_TEXTURE_ADDRESS_TYPE addressW;
} CM_SAMPLER_STATE;

typedef enum _CM_PIXEL_TYPE
{
    CM_PIXEL_UINT,
    CM_PIXEL_SINT,
    CM_PIXEL_OTHER
} CM_PIXEL_TYPE;

typedef struct _CM_SAMPLER_STATE_EX
{
    CM_TEXTURE_FILTER_TYPE minFilterType;
    CM_TEXTURE_FILTER_TYPE magFilterType;
    CM_TEXTURE_ADDRESS_TYPE addressU;
    CM_TEXTURE_ADDRESS_TYPE addressV;
    CM_TEXTURE_ADDRESS_TYPE addressW;

    CM_PIXEL_TYPE surfaceFormat;
    union
    {
        uint32_t borderColorRedU;
        int32_t borderColorRedS;
        float borderColorRedF;
    };
    union
    {
        uint32_t borderColorGreenU;
        int32_t borderColorGreenS;
        float borderColorGreenF;
    };
    union
    {
        uint32_t borderColorBlueU;
        int32_t borderColorBlueS;
        float borderColorBlueF;
    };
    union
    {
        uint32_t borderColorAlphaU;
        int32_t borderColorAlphaS;
        float borderColorAlphaF;
    };
} CM_SAMPLER_STATE_EX;

//struct used by both CmDevice and CmSampler8x8
typedef struct _CM_AVS_COEFF_TABLE
{
    float   filterCoeff_0_0;
    float   filterCoeff_0_1;
    float   filterCoeff_0_2;
    float   filterCoeff_0_3;
    float   filterCoeff_0_4;
    float   filterCoeff_0_5;
    float   filterCoeff_0_6;
    float   filterCoeff_0_7;
} CM_AVS_COEFF_TABLE;

#define CM_NUM_COEFF_ROWS 17
#define CM_NUM_COEFF_ROWS_SKL 32
typedef struct _CM_AVS_NONPIPLINED_STATE
{
    bool bypassXAF;
    bool bypassYAF;
    uint8_t defaultSharpLvl;
    uint8_t maxDerivative4Pixels;
    uint8_t maxDerivative8Pixels;
    uint8_t transitionArea4Pixels;
    uint8_t transitionArea8Pixels;
    CM_AVS_COEFF_TABLE Tbl0X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl0Y[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl1X[ CM_NUM_COEFF_ROWS_SKL ];
    CM_AVS_COEFF_TABLE Tbl1Y[ CM_NUM_COEFF_ROWS_SKL ];
    bool enableRgbAdaptive;
    bool adaptiveFilterAllChannels;
} CM_AVS_NONPIPLINED_STATE;

enum CM_SAMPLER_STATE_TYPE {
    CM_SAMPLER8X8_AVS = 0,
    CM_SAMPLER8X8_CONV = 1,
    CM_SAMPLER8X8_MISC = 3,
    CM_SAMPLER8X8_CONV1DH = 4,
    CM_SAMPLER8X8_CONV1DV = 5,
    CM_SAMPLER8X8_AVS_EX = 6,
    CM_SAMPLER8X8_NONE
};

typedef struct _CM_AVS_STATE_MSG
{
    bool avsType; //true nearest, false adaptive
    bool eightTapAFEnable; //HSW+
    bool bypassIEF; //ignored for BWL, moved to sampler8x8 payload.
    bool shuffleOutputWriteback; //SKL mode only to be set when AVS msg sequence is 4x4 or 8x4
    bool hdcDirectWriteEnable;
    unsigned short gainFactor;
    unsigned char globalNoiseEstm;
    unsigned char strongEdgeThr;
    unsigned char weakEdgeThr;
    unsigned char strongEdgeWght;
    unsigned char regularWght;
    unsigned char nonEdgeWght;
    unsigned short r3xCoefficient;
    unsigned short r3cCoefficient;
    unsigned short r5xCoefficient;
    unsigned short r5cxCoefficient;
    unsigned short r5cCoefficient;
    //For Non-piplined states
    unsigned short stateID;
    CM_AVS_NONPIPLINED_STATE * avsState;
} CM_AVS_STATE_MSG;

//*-----------------------------------------------------------------------------
//| CM Convolve type for SKL+
//*-----------------------------------------------------------------------------
typedef enum _CM_CONVOLVE_SKL_TYPE
{
    CM_CONVOLVE_SKL_TYPE_2D = 0,
    CM_CONVOLVE_SKL_TYPE_1D = 1,
    CM_CONVOLVE_SKL_TYPE_1P = 2
} CM_CONVOLVE_SKL_TYPE;

typedef struct _CM_CONVOLVE_COEFF_TABLE
{
    float   filterCoeff_0_0;
    float   filterCoeff_0_1;
    float   filterCoeff_0_2;
    float   filterCoeff_0_3;
    float   filterCoeff_0_4;
    float   filterCoeff_0_5;
    float   filterCoeff_0_6;
    float   filterCoeff_0_7;
    float   filterCoeff_0_8;
    float   filterCoeff_0_9;
    float   filterCoeff_0_10;
    float   filterCoeff_0_11;
    float   filterCoeff_0_12;
    float   filterCoeff_0_13;
    float   filterCoeff_0_14;
    float   filterCoeff_0_15;
    float   filterCoeff_0_16;
    float   filterCoeff_0_17;
    float   filterCoeff_0_18;
    float   filterCoeff_0_19;
    float   filterCoeff_0_20;
    float   filterCoeff_0_21;
    float   filterCoeff_0_22;
    float   filterCoeff_0_23;
    float   filterCoeff_0_24;
    float   filterCoeff_0_25;
    float   filterCoeff_0_26;
    float   filterCoeff_0_27;
    float   filterCoeff_0_28;
    float   filterCoeff_0_29;
    float   filterCoeff_0_30;
    float   filterCoeff_0_31;
} CM_CONVOLVE_COEFF_TABLE;

#define CM_NUM_CONVOLVE_ROWS 16
#define CM_NUM_CONVOLVE_ROWS_SKL 32
typedef struct _CM_CONVOLVE_STATE_MSG
{
    bool coeffSize; //true 16-bit, false 8-bit
    int8_t sclDwnValue; //Scale down value
    int8_t width; //Kernel Width
    int8_t height; //Kernel Height
    //SKL mode
    bool isVertical32Mode;
    bool isHorizontal32Mode;
    CM_CONVOLVE_SKL_TYPE nConvolveType;
    CM_CONVOLVE_COEFF_TABLE table[CM_NUM_CONVOLVE_ROWS_SKL];
} CM_CONVOLVE_STATE_MSG;

struct CM_AVS_STATE_MSG_EX
{
    CM_RT_API CM_AVS_STATE_MSG_EX();

    bool enableAllChannelAdaptiveFilter;  // adaptive filter for all channels. validValues => [true..false]
    bool enableRgbAdaptiveFilter;          // adaptive filter for all channels. validValues => [true..false]
    bool enable8TapAdaptiveFilter;        // enable 8-tap filter. validValues => [true..false]
    bool enableUV8TapFilter;              // enable 8-tap filter on UV/RB channels. validValues => [true..false]
    bool writebackFormat;                    // true sampleunorm, false standard. validValues => [true..false]
    bool writebackMode;                      // true avs, false ief. validValues => [true..false]
    uint8_t stateSelection;                     // 0=>first,1=>second scaler8x8 state. validValues => [0..1]

    // Image enhancement filter settings.
    bool enableIef;        // image enhancement filter enable. validValues => [true..false]
    bool iefType;          // true "basic" or false "advanced". validValues => [true..false]
    bool enableIefSmooth; // true based on 3x3, false based on 5x5 validValues => [true..false]
    float r3cCoefficient;  // smoothing coeffient. Valid values => [0.0..0.96875]
    float r3xCoefficient;  // smoothing coeffient. Valid values => [0.0..0.96875]
    float r5cCoefficient;  // smoothing coeffient. validValues => [0.0..0.96875]
    float r5cxCoefficient; // smoothing coeffient. validValues => [0.0..0.96875]
    float r5xCoefficient;  // smoothing coeffient. validValues => [0.0..0.96875]

    // Edge processing settings.
    uint8_t strongEdgeThreshold; // validValues => [0..64]
    uint8_t strongEdgeWeight;    // Sharpening strength when a strong edge. validValues => [0..7]
    uint8_t weakEdgeThreshold;   // validValues => [0..64]
    uint8_t regularEdgeWeight;   // Sharpening strength when a weak edge. validValues => [0..7]
    uint8_t nonEdgeWeight;       // Sharpening strength when no edge. validValues => [0..7]

    // Chroma key.
    bool enableChromaKey; // Chroma keying be performed. validValues => [true..false]
    uint8_t chromaKeyIndex;  // ChromaKey Table entry. validValues => [0..3]

    // Skin tone settings.
    bool enableSkinTone;              // SkinToneTunedIEF_Enable. validValues => [true..false]
    bool enableVySkinToneDetection; // Enables STD in the VY subspace. validValues => [true..false]
    bool skinDetailFactor;            // validValues => [true..false]
    uint8_t skinTypesMargin;             // validValues => [0..255]
    uint8_t skinTypesThreshold;          // validValues => [0..255]

    // Miscellaneous settings.
    uint8_t gainFactor;             // validValues => [0..63]
    uint8_t globalNoiseEstimation; // validValues => [0..255]
    bool mrBoost;                // validValues => [true..false]
    uint8_t mrSmoothThreshold;     // validValues => [0..3]
    uint8_t mrThreshold;
    bool steepnessBoost;         // validValues => [true..false]
    uint8_t steepnessThreshold;     // validValues => [0..15]
    bool textureCoordinateMode; // true: clamp, false: mirror. validValues => [true..false]
    uint8_t maxHue;                 // Rectangle half width. validValued => [0..63]
    uint8_t maxSaturation;          // Rectangle half length. validValued => [0..63]
    int angles;                   // validValued => [0..360]
    uint8_t diamondMargin ;         // validValues => [0..7]
    char diamondDu;              // Rhombus center shift in the sat-direction. validValues => [-64..63]
    char diamondDv;              // Rhombus center shift in the hue-direction. validValues => [-64..63]
    float diamondAlpha;          // validValues => [0.0..4.0]
    uint8_t diamondThreshold;       // validValues => [0..63]
    uint8_t rectangleMargin;        // validValues => [0..7]
    uint8_t rectangleMidpoint[2];   // validValues => [[0..255, 0..255]]
    float vyInverseMargin[2];   // validValues => [[0.0..1.0, 0.0..1.0]]

    // Piecewise linear function settings.
    uint8_t piecewiseLinearYPoints[4];      // validValues => [[0..255, 0..255, 0..255, 0..255]]
    float piecewiseLinearYSlopes[2];     // validValues => [[-4.0...4.0, -4.0...4.0]]
    uint8_t piecewiseLinearPointsLower[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]
    uint8_t piecewiseLinearPointsUpper[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]
    float piecewiseLinearSlopesLower[4]; // validValues => [[-4.0...4.0, -4.0...4.0, -4.0...4.0, -4.0...4.0]]
    float piecewiseLinearSlopesUpper[4]; // validValues => [[-4.0...4.0, -4.0...4.0, -4.0...4.0, -4.0...4.0]]
    uint8_t piecewiseLinearBiasesLower[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]
    uint8_t piecewiseLinearBiasesUpper[4];  // validValues => [[0..255, 0..255, 0..255, 0..255]]

    // AVS non-pipelined states.
    uint8_t defaultSharpnessLevel;   // default coefficient between smooth and sharp filtering. validValues => [0..255]
    bool enableXAdaptiveFilter;  // validValues => [true, false]
    bool enableYAdaptiveFilter;  // validValues => [true, false]
    uint8_t maxDerivative4Pixels;   // lower boundary of the smooth 4 pixel area. validValues => [0..255]
    uint8_t maxDerivative8Pixels;   // lower boundary of the smooth 8 pixel area. validValues => [0..255]
    uint8_t transitionArea4Pixels;  // used in adaptive filtering to specify the width of the transition area for the 4 pixel calculation. validValues => [0..8]
    uint8_t transitionArea8Pixels;  // Used in adaptive filtering to specify the width of the transition area for the 8 pixel calculation. validValues => [0..8]
    CM_AVS_COEFF_TABLE table0X[CM_NUM_COEFF_ROWS_SKL];
    CM_AVS_COEFF_TABLE table0Y[CM_NUM_COEFF_ROWS_SKL];
    CM_AVS_COEFF_TABLE table1X[CM_NUM_COEFF_ROWS_SKL];
    CM_AVS_COEFF_TABLE table1Y[CM_NUM_COEFF_ROWS_SKL];
};

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_MISC_STATE
{
    //uint32_t 1
    union
    {
        struct
        {
            uint32_t Row0      : 16;
            uint32_t Reserved  : 8;
            uint32_t Width     : 4;
            uint32_t Height    : 4;
        };
        struct
        {
            uint32_t value;
        };
    } DW0;

    //uint32_t 1
    union
    {
        struct
        {
            uint32_t Row1      : 16;
            uint32_t Row2      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW1;

    //uint32_t 2
    union
    {
        struct
        {
            uint32_t Row3      : 16;
            uint32_t Row4      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW2;

    //uint32_t 3
    union
    {
        struct
        {
            uint32_t Row5      : 16;
            uint32_t Row6      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW3;

    //uint32_t 4
    union
    {
        struct
        {
            uint32_t Row7      : 16;
            uint32_t Row8      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW4;

    //uint32_t 5
    union
    {
        struct
        {
            uint32_t Row9       : 16;
            uint32_t Row10      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW5;

    //uint32_t 6
    union
    {
        struct
        {
            uint32_t Row11      : 16;
            uint32_t Row12      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW6;

    //uint32_t 7
    union
    {
        struct
        {
            uint32_t Row13      : 16;
            uint32_t Row14      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW7;
} CM_MISC_STATE;

/*
 *  CONVOLVE STATE DATA STRUCTURES
 */
typedef struct _CM_MISC_STATE_MSG
{
    //uint32_t 0
    union
    {
        struct
        {
            uint32_t Row0      : 16;
            uint32_t Reserved  : 8;
            uint32_t Width     : 4;
            uint32_t Height    : 4;
        };
        struct
        {
            uint32_t value;
        };
    }DW0;

    //uint32_t 1
    union
    {
        struct
        {
            uint32_t Row1      : 16;
            uint32_t Row2      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW1;

    //uint32_t 2
    union
    {
        struct
        {
            uint32_t Row3      : 16;
            uint32_t Row4      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW2;

    //uint32_t 3
    union
    {
        struct
        {
            uint32_t Row5      : 16;
            uint32_t Row6      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW3;

    //uint32_t 4
    union
    {
        struct
        {
            uint32_t Row7      : 16;
            uint32_t Row8      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW4;

    //uint32_t 5
    union
    {
        struct
        {
            uint32_t Row9      : 16;
            uint32_t Row10      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW5;

    //uint32_t 6
    union
    {
        struct
        {
            uint32_t Row11      : 16;
            uint32_t Row12      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW6;

    //uint32_t 7
    union
    {
        struct
        {
            uint32_t Row13      : 16;
            uint32_t Row14      : 16;
        };
        struct
        {
            uint32_t value;
        };
    } DW7;
} CM_MISC_STATE_MSG;

struct CM_SAMPLER_8X8_DESCR
{
    CM_SAMPLER_STATE_TYPE stateType;
    union
    {
        CM_AVS_STATE_MSG *avs;
        CM_AVS_STATE_MSG_EX *avsEx;
        CM_CONVOLVE_STATE_MSG *conv;
        CM_MISC_STATE_MSG *misc; //ERODE/DILATE/MINMAX
    };
};
#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CM_DEF_H_
