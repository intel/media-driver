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
//! \file      cm_hal_generic.h
//! \brief     Main Entry point for CM HAL Generic component
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALGENERIC_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALGENERIC_H_

#include "cm_def.h"

typedef struct CM_HAL_GENERIC *           PCM_HAL_GENERIC;
typedef struct _CM_HAL_SAMPLER_8X8_PARAM *PCM_HAL_SAMPLER_8X8_PARAM;
typedef struct CmHalL3Settings *          PCmHalL3Settings;

//-------------------------------
//| CM HW platform info
//-------------------------------
struct CM_PLATFORM_INFO
{
    uint32_t numSlices;
    uint32_t numSubSlices;
    uint32_t numEUsPerSubSlice;
    uint32_t numHWThreadsPerEU;
    uint32_t numMaxEUsPerPool;
};
typedef CM_PLATFORM_INFO *PCM_PLATFORM_INFO;

struct CM_HAL_WALKER_XY
{
    union
    {
        struct
        {
            uint32_t x : 16;
            uint32_t y : 16;
        };
        uint32_t value;
    };
};
typedef CM_HAL_WALKER_XY *PCM_HAL_WALKER_XY;

// The following enum type must match
// MHW_WALKER_MODE defined in mhw_render.h
enum CM_HAL_WALKER_MODE
{
    CM_HAL_WALKER_MODE_NOT_SET  = -1,
    CM_HAL_WALKER_MODE_DISABLED = 0,
    CM_HAL_WALKER_MODE_SINGLE   = 1,  // dual = 0, repel = 1
    CM_HAL_WALKER_MODE_DUAL     = 2,  // dual = 1, repel = 0)
    CM_HAL_WALKER_MODE_TRI      = 3,  // applies in BDW GT2 which has 1 slice and 3 sampler/VME per slice
    CM_HAL_WALKER_MODE_QUAD     = 4,  // applies in HSW GT3 which has 2 slices and 2 sampler/VME per slice
    CM_HAL_WALKER_MODE_HEX      = 6,  // applies in BDW GT2 which has 2 slices and 3 sampler/VME per slice
    CM_HAL_WALKER_MODE_OCT      = 8   // may apply in future Gen media architectures
};

// The following structure must match the structure
// MHW_WALKER_PARAMS defined in mhw_render.h
struct CM_HAL_WALKER_PARAMS
{
    uint32_t interfaceDescriptorOffset : 5;
    uint32_t cmWalkerEnable            : 1;
    uint32_t colorCountMinusOne        : 8;
    uint32_t useScoreboard             : 1;
    uint32_t scoreboardMask            : 8;
    uint32_t midLoopUnitX              : 2;
    uint32_t midLoopUnitY              : 2;
    uint32_t middleLoopExtraSteps      : 5;
    uint32_t groupIdLoopSelect         : 24;
    uint32_t                           : 8;

    uint32_t inlineDataLength;
    uint8_t *inlineData;
    uint32_t localLoopExecCount;
    uint32_t globalLoopExecCount;

    CM_HAL_WALKER_MODE walkerMode;
    CM_HAL_WALKER_XY   blockResolution;
    CM_HAL_WALKER_XY   localStart;
    CM_HAL_WALKER_XY   localEnd;
    CM_HAL_WALKER_XY   localOutLoopStride;
    CM_HAL_WALKER_XY   localInnerLoopUnit;
    CM_HAL_WALKER_XY   globalResolution;
    CM_HAL_WALKER_XY   globalStart;
    CM_HAL_WALKER_XY   globalOutlerLoopStride;
    CM_HAL_WALKER_XY   globalInnerLoopUnit;

    bool addMediaFlush;
    bool requestSingleSlice;
};
typedef CM_HAL_WALKER_PARAMS *PCM_HAL_WALKER_PARAMS;

struct SamplerParam
{
    unsigned int samplerTableIndex;
    unsigned int heapOffset;
    unsigned int bti;
    unsigned int btiStepping;
    unsigned int btiMultiplier;
    bool         userDefinedBti;
    bool         regularBti;
    unsigned int elementType;
    unsigned int size;
};

struct CM_SURFACE_BTI_INFO
{
    uint32_t normalSurfaceStart;    // start index of normal surface
    uint32_t normalSurfaceEnd;      // end index of normal surface
    uint32_t reservedSurfaceStart;  // start index of reserved surface
    uint32_t reservedSurfaceEnd;    // end index of reserved surface
};
typedef CM_SURFACE_BTI_INFO *PCM_SURFACE_BTI_INFO;

//------------------------------------------------------------------------------
//| CM HW Expected GT system info
//------------------------------------------------------------------------------
struct CM_EXPECTED_GT_SYSTEM_INFO
{
    uint32_t numSlices;
    uint32_t numSubSlices;
};
typedef CM_EXPECTED_GT_SYSTEM_INFO *PCM_EXPECTED_GT_SYSTEM_INFO;

struct CM_HAL_GENERIC
{
#define ASSIGN_IF_VALID(ptr, value) \
    if (ptr)                        \
    {                               \
        *ptr = value;               \
    }

public:
    PCM_HAL_STATE m_cmState;
    const L3ConfigRegisterValues *m_l3Plane = nullptr;
    size_t m_l3ConfigCount = 0;

    CM_HAL_GENERIC(PCM_HAL_STATE cmState) : m_cmState(cmState),
                                            m_platformID(PLATFORM_INTEL_UNKNOWN),
                                            m_genGT(PLATFORM_INTEL_GT_UNKNOWN),
                                            m_platformStr(nullptr),
                                            m_requestShutdownSubslicesForVmeUsage(false),
                                            m_overridePowerOptionPerGpuContext(false),
                                            m_redirectRcsToCcs(false),
                                            m_decompress(false),
                                            m_fastpathDefault(false),
                                            m_defaultMocs(MOS_CM_RESOURCE_USAGE_SurfaceState){};

    virtual ~CM_HAL_GENERIC(){};

    //!
    //! \brief    Get GPUCopy Kernel's ISA and Size
    //! \details  Get GPUCopy Kernel's ISA and Size
    //! \param    [out] isa
    //!           pointer to memory of gpucopy isa
    //! \param    [out] isaSize
    //!           size of gpucopy isa
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCopyKernelIsa(void *&isa, uint32_t &isaSize) = 0;

    //!
    //! \brief    Get GPU Surface Initialization Kernel's ISA and Size
    //! \details  Get GPU Surface Initialization Kernel's ISA and Size
    //! \param    [out] isa
    //!           pointer to memory of gpu initialization isa
    //! \param    [out] isaSize
    //!           size of gpu initialization isa
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetInitKernelIsa(void *&isa, uint32_t &isaSize) = 0;

    //!
    //! \brief    Set media walker parameters
    //! \details  Set media walker parameters
    //! \param    [in]  engineeringParams
    //!           engineering params passed by caller
    //! \param    [in] walkerParams
    //!           pointer to walker paramaeters to set
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMediaWalkerParams(
        CM_WALKING_PARAMETERS engineeringParams,
        PCM_HAL_WALKER_PARAMS walkerParams) = 0;

    //!
    //! \brief    Set Surface Memory Object Control
    //! \details  Convert Memory Object Control bits to RenderHal Surface State
    //! \param    [in]  memObjCtl
    //!           memObjCtl passed by caller
    //! \param    [in] surfStateParams
    //!           pointer to surface state param
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HwSetSurfaceMemoryObjectControl(
        uint16_t                        memObjCtl,
        PRENDERHAL_SURFACE_STATE_PARAMS surfStateParams) = 0;

    //!
    //! \brief    Register Sampler8x8
    //! \details  Register Sampler8x8
    //! \param    [in]  param
    //!           pointer to cmhal sampler8x8 param
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RegisterSampler8x8(
        PCM_HAL_SAMPLER_8X8_PARAM param) = 0;

    //!
    //! \brief    Submit commmand to kernel mode driver
    //! \details  Submit commmand to kernel mode driver
    //! \param    [in]  batchBuffer
    //!           pointer to mhw batch buffer to submit
    //! \param    [in]  taskId
    //!           id of task
    //! \param    [in]  kernelParam
    //!           pointer to array of kernel param
    //! \param    [out]  cmdBuffer
    //!           pointer cmd buffer returned to cm event
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCommands(
        PMHW_BATCH_BUFFER     batchBuffer,
        int32_t               taskId,
        PCM_HAL_KERNEL_PARAM *kernelParam,
        void **               cmdBuffer) = 0;
#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
    //!
    //! \brief    Submit dummy commmand to kernel mode driver to set up page table for Direct submission
    //! \details  Submit commmand to kernel mode driver
    //! \param    [in]  pBatchBuffer
    //!           pointer to mhw batch buffer to submit
    //! \param    [in]  iTaskId
    //!           id of task
    //! \param    [out]  ppCmdBuffer
    //!           pointer cmd buffer returned to cm event
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitDummyCommands(
        PMHW_BATCH_BUFFER     batchBuffer,
        int32_t               taskId,
        PCM_HAL_KERNEL_PARAM *kernelParam,
        void **               cmdBuffer) = 0;
#endif
#endif

    //!
    //! \brief    Submit a commmand to get the time stamp base
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitTimeStampBaseCommands()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Update platform information from power option
    //! \details  Power option can be used to do slice shutdown. This function is
    //!           to adjust platform info (EU numbers/Slice number) accordingly.
    //! \param    [in]  platformInfo
    //!           pointer to platform info
    //! \param    [in]  euSaturated
    //!           if EU Saturation required.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdatePlatformInfoFromPower(
        PCM_PLATFORM_INFO platformInfo,
        bool              euSaturated) = 0;

    //!
    //! \brief    Get media walker's max width
    //! \details  Get media walker's max width
    //! \return   media walker's max width
    //!
    virtual uint32_t GetMediaWalkerMaxThreadWidth() = 0;

    //!
    //! \brief    Get media walker's max height
    //! \details  Get media walker's max height
    //! \return   media walker's max height
    //!
    virtual uint32_t GetMediaWalkerMaxThreadHeight() = 0;

    //!
    //! \brief    Get Surface binding table index info
    //! \details  Get Surface binding table index info, including the start/end index of
    //!           reserved surfaces and normal surfaces
    //! \param    [in]  btiInfo
    //!           pointer to binding table information
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetHwSurfaceBTIInfo(
        PCM_SURFACE_BTI_INFO btiInfo) = 0;

    //!
    //! \brief    Set Suggested L3 Configuration to RenderHal
    //! \details  Set Suggested L3 Configuration to RenderHal
    //! \param    [in]  l3Config
    //!           index of selected configuration
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSuggestedL3Conf(
        L3_SUGGEST_CONFIG l3Config) = 0;

    //!
    //! \brief    Allocate SIP/CSR Resource for Preemption and Debug
    //! \details  Allocate SIP/CSR Resource for Preemption and Debug
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS AllocateSIPCSRResource() = 0;

    //!
    //! \brief    Get the stepping string of Gen platform
    //! \details  Get the stepping string of Gen platform
    //! \param    [in,out]  stepInfoStr
    //!           reference to stepping information string
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetGenStepInfo(char *&stepInfoStr) = 0;

    //!
    //! \brief    Get the platform code and GT type of Gen platform
    //! \param    [out]  platformID
    //!           pointer to the platform code defined in GPU_PLATFORM
    //! \param    [out]  gengt
    //!           pointer to the GT type defined in GPU_GT_PLATFORM
    //! \param    [out]  platformStr
    //!           pointer to platform string
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetGenPlatformInfo(uint32_t *platformID,
        uint32_t *                                  genGT,
        const char **                               platformStr)
    {
        ASSIGN_IF_VALID(platformID, m_platformID);
        ASSIGN_IF_VALID(genGT, m_genGT);
        ASSIGN_IF_VALID(platformStr, m_platformStr);
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    set the platform code and GT type of Gen platform
    //! \param    [in]  platformID
    //!           the platform code defined in GPU_PLATFORM
    //! \param    [in]  gengt
    //!           the GT type defined in GPU_GT_PLATFORM
    //! \param    [in]  platformStr
    //!           platform string
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetGenPlatformInfo(uint32_t platformID,
        uint32_t                                   genGT,
        const char *                               platformStr)
    {
        m_platformID  = platformID;
        m_genGT       = genGT;
        m_platformStr = platformStr;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    enable or disable the slice downdown feature
    //! \param    [in]  enabled
    //!           true: enable slice shutdown; false: disable slice shutdown
    virtual void SetRequestShutdownSubslicesForVmeUsage(bool enabled)
    {
        m_requestShutdownSubslicesForVmeUsage = enabled;
    }

    //!
    //! \brief    return whether the slice downdown feature is enabled
    //! \return   bool
    //!           true: slice shutdown enabled; false: slice shutdown disabled
    virtual bool IsRequestShutdownSubslicesForVmeUsage()
    {
        return m_requestShutdownSubslicesForVmeUsage;
    }

    //!
    //! \brief    check if the CISA Gen ID is supported on the platform
    //! \param    [in]  cisaGenID
    //!           the CISA ID that will be checked
    //! \return   bool
    //!           true if it is supported; false if not
    virtual bool IsCisaIDSupported(uint32_t cisaGenID)
    {
        for (uint32_t id : m_cisaGenIDs)
        {
            if (id == cisaGenID)
            {
                return true;
            }
        }
        return false;
    }

    //!
    //! \brief    add the supported CISA Gen IDs of this product
    //! \param    [in]  cisaGenIDs
    //!           pointer to the vector of supported cisaGenIDs
    //! \param    [in]  len
    //!           length of the vector of supported cisaGenIDs
    //! \return   MOS_STATUS
    //!           always return MOS_STATUS_SUCCESS
    virtual MOS_STATUS AddSupportedCisaIDs(uint32_t *cisaGenIDs, int len = 1)
    {
        for (int i = 0; i < len; i++)
        {
            m_cisaGenIDs.push_back(cisaGenIDs[i]);
        }
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Check if Qpitch is supported by hw in surface 3d lock
    //! \details  Check if Qpitch is supported by hw in surface 3d lock
    //!           Qpitch is supported from SKL
    //! \return   True for SKL+
    virtual bool IsSurf3DQpitchSupportedbyHw() { return true; };

    //!
    //! \brief    Check if compare mask supported by hw in conditional buffer
    //! \details  Check if compare mask supported by hw in conditional buffer
    //!           compare mask is supported from SKL
    //! \return   True for SKL+
    virtual bool IsCompareMaskSupportedbyHw() { return true; };

    //!
    //! \brief    Check if two adjacent sampler index requried by hardware
    //! \details  Check if two adjacent sampler index requried by hardware
    //!           compare mask is supported from SKL
    //! \return   True for BDW, and False for SKL+
    virtual bool IsAdjacentSamplerIndexRequiredbyHw() { return false; };

    //!
    //! \brief    Check if the WA to disable surface compression required
    //! \details  Check if the WA to disable surface compression required
    //! \return   False for BDW, and True for SKL+
    virtual bool IsSurfaceCompressionWARequired() { return true; };

    //!
    //! \brief    Check if scoreboading parameters are supported.
    //! \details  Check if scoreboading parameters are supported.
    //! \return   False for Not needed, and True for Needed
    virtual bool IsScoreboardParamNeeded() { return true; };

    //!
    //! \brief    Check if surface color format is supported by VME
    //! \return   true if format is supported by VME surface
    virtual bool IsSupportedVMESurfaceFormat(MOS_FORMAT format)
    {
        if (format != Format_NV12)
            return false;
        else
            return true;
    }

    //!
    //! \brief    Sanity check for ColorCount.
    //! \details  Sanity check for ColorCount.
    //!           ColorCountMinusOne varies from 4 bit to 8 bit on different platforms
    //! \return   Result of the operation.
    virtual int32_t ColorCountSanityCheck(uint32_t colorCount) = 0;

    //!
    //! \brief    Sanity check for memory object control policy.
    //! \details  Sanity check for memory object control policy.
    //!           Each platform supports different control policy.
    //! \param    [in]  memCtrl
    //!           input of memory object control to check
    //! \return   Result of the operation.
    virtual bool MemoryObjectCtrlPolicyCheck(uint32_t memCtrl) = 0;

    //!
    //! \brief    Check if the WA to use No cache setting for GPUCopy surface required
    //! \details  Check if the WA to use No cache setting for GPUCopy surface required
    //!           Configure memory object control for the two BufferUP to solve the same
    //!           cache-line coherency issue.
    //! \return   False for BDW, and True for SKL+
    virtual bool IsGPUCopySurfaceNoCacheWARequired() { return true; };

    //!
    //! \brief    Check if one plane P010 surface is supported.
    //! \details  Check if one plane P010 surface is supported.
    //!           one plane P010 surface is supported since CNL.
    //! \return   False for pre-CNL, and True for CNL+
    virtual bool IsP010SinglePassSupported() { return true; };

    //!
    //! \brief    Get Convolution Sampler Index.
    //! \details  Get Convolution Sampler Index.
    //! \param    [in]  samplerParam
    //!           pointer to sampler param
    //! \param    [in]  samplerIndexTable
    //!           pointer to sampler index table
    //! \param    [in]  nSamp8X8Num
    //!           number of sampler8x8
    //! \param    [in]  nSampConvNum
    //!           number of conv sampler
    //! \return   Sampler index.
    virtual int32_t GetConvSamplerIndex(
        PMHW_SAMPLER_STATE_PARAM samplerParam,
        char *                   samplerIndexTable,
        int32_t                  nSamp8X8Num,
        int32_t                  nSampConvNum) = 0;

    //!
    //! \brief    Set L3 values in CM hal layer.
    //! \details  Use the L3 struct to set L3 to different platforms.
    //! \param    [in]  values
    //!           pointer to input L3 config values
    //! \param    [in]  cmHalL3Setting
    //!           pointer to hal layer L3 config values
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetL3CacheConfig(
        const L3ConfigRegisterValues *values,
        PCmHalL3Settings              cmHalL3Setting) = 0;

    //!
    //! \brief    Get sampler element count for a given sampler type.
    //! \details  Convert the sampler type to how many element for this sampler
    //!           type for current platform.
    //! \param    [in]  mhwSamplerParam
    //!           pointer to the sampler param defined by MHW
    //! \param    [in,out]  samplerParam
    //!           Will get sampler size, sampler element type, sampler bti
    //!           stepping and sampler multiplier for this type of sampler
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetSamplerParamInfoForSamplerType(
        PMHW_SAMPLER_STATE_PARAM mhwSamplerParam,
        SamplerParam &           samplerParam) = 0;

    //!
    //! \brief    Get the expected configuration for specific GT
    //! \details  Get the expected configuration for specific GT
    //! \param    [in]  expectedConfig
    //!           pointer to expected config
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetExpectedGtSystemConfig(
        PCM_EXPECTED_GT_SYSTEM_INFO expectedConfig) = 0;

    //!
    //! \brief    Get the size of the timestamp resource for each task
    //! \details  Get the size of the timestamp resource for each task
    //! \return   int32_t
    //!           Size of the timestamp resource for each task
    //!
    virtual int32_t GetTimeStampResourceSize()
    {
        // Default: 2 QWORDs for each kernel in the task + 1 QWORD for frame tracking
        return (sizeof(uint64_t) * CM_SYNC_QWORD_PER_TASK) + (sizeof(uint64_t) * CM_TRACKER_ID_QWORD_PER_TASK);
    }

    //!
    //! \brief    Covnert the ticks to nano seconds with default config if KMD querying failed
    //! \param    [in]  ticks
    //!           input ticks
    //! \return   uint64_t
    //!           Nano seconds converted from the input ticks
    //!
    virtual uint64_t ConverTicksToNanoSecondsDefault(uint64_t ticks) = 0;

    //!
    //! \brief    Check if the platform has media mode or not
    //! \details  Check if the platform has media mode or not
    //! \return   bool
    //!           true: the platform has media mode; false the platform
    //!           does not have media mode.
    //!
    virtual bool CheckMediaModeAvailability() { return true; }

    //!
    //! \brief    enable or disable the power option per GPU context
    //! \param    [in]  enabled
    //!           true: enable per GPU context; false: disable per Batch command
    virtual void SetOverridePowerOptionPerGpuContext(bool enabled)
    {
        m_overridePowerOptionPerGpuContext = enabled;
    }

    //!
    //! \brief    return whether the power option per GPU context is enabled
    //! \return   bool
    //!           true: enable per GPU context; false: disable per Batch command
    virtual bool IsOverridePowerOptionPerGpuContext()
    {
        return m_overridePowerOptionPerGpuContext;
    }

    //!
    //! \brief    enable or disable redirect of RCS to CCS
    //! \param    [in] enabled
    //!           true: enable redirect of RCS to CCS; false: disable redirect of RCS to CCS
    virtual void SetRedirectRcsToCcs(bool enabled)
    {
        m_redirectRcsToCcs = enabled;
    }

    //!
    //! \brief    return whether need redirect RCS to CCS
    //! \return   bool
    //!           true: redirect is need; false: don't need redirect
    virtual bool IsRedirectRcsToCcs()
    {
        return m_redirectRcsToCcs;
    }

    //!
    //! \brief    enable or disable decompression flag
    //! \param    [in]  enabled
    //!           true: enable decompression for surface;
    //!           false: disable decompression
    virtual void SetDecompressFlag(bool enabled)
    {
        m_decompress = enabled;
    }

    //!
    //! \brief    return whether the decompreesion option
    //! \return   bool
    //!           true: need to decompress a compressed surface;
    //!           false: no need to decompress the compressed surface
    virtual bool GetDecompressFlag()
    {
        return m_decompress;
    }

    virtual MOS_STATUS RegisterResourceBeforeLock(PMOS_INTERFACE pOsInterface, PMOS_RESOURCE pResource)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Init the default value of task related property
    //! \param    [in,out]  taskConfig
    //!           Task related property
    //! \return   MOS_STATUS_SUCCESS
    //!
    virtual MOS_STATUS InitTaskProperty(CM_TASK_CONFIG &taskConfig)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Whether switch to fast path by default
    //! \return   true or false
    //!
    virtual void SetFastPathByDefault(bool flag)
    {
        m_fastpathDefault = flag;
    }

    //!
    //! \brief    Whether switch to fast path by default
    //! \return   true or false
    //!
    virtual bool IsFastPathByDefault()
    {
        return m_fastpathDefault;
    }
    
    //! \brief    Get the smallest max thread number that can be set in VFE
    //! \return   the smallest max thread number
    //!
    virtual uint32_t GetSmallestMaxThreadNum()
    {
        return 1;
    }

    //!
    //! \brief    Set the default MOCS for this platform
    //! \return   void
    //!
    inline void SetDefaultMOCS(MOS_HW_RESOURCE_DEF mocs)
    {
        m_defaultMocs = mocs;
    }

    //!
    //! \brief    Get the default MOCS for this platform
    //! \return   the default MOCS
    //!
    inline MOS_HW_RESOURCE_DEF GetDefaultMOCS()
    {
        return m_defaultMocs;
    }

    //! \brief    Check whether compressed output is needed
    //! \return   true if compression format is needed
    //!
    virtual bool SupportCompressedOutput()
    {
        return false;
    }

    //! \brief    Check whether separate scratch space is needed
    //! \return   true if separate scratch is needed
    //!
    virtual bool IsSeparateScratch()
    {
        return false;
    }

protected:
    uint32_t              m_platformID;
    uint32_t              m_genGT;
    const char *          m_platformStr;
    std::vector<uint32_t> m_cisaGenIDs;
    bool                  m_requestShutdownSubslicesForVmeUsage;
    bool                  m_overridePowerOptionPerGpuContext;
    bool                  m_redirectRcsToCcs;
    bool                  m_decompress;
    bool                  m_fastpathDefault;
    MOS_HW_RESOURCE_DEF   m_defaultMocs;
};

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALGENERIC_H_
