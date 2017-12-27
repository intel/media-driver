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

typedef struct CM_HAL_GENERIC *PCM_HAL_GENERIC;
typedef struct _CM_HAL_SAMPLER_8X8_PARAM *PCM_HAL_SAMPLER_8X8_PARAM;
typedef struct CmHalL3Settings *PCmHalL3Settings;

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
    CM_HAL_WALKER_MODE_SINGLE   = 1,    // dual = 0, repel = 1
    CM_HAL_WALKER_MODE_DUAL     = 2,    // dual = 1, repel = 0)
    CM_HAL_WALKER_MODE_TRI      = 3,    // applies in BDW GT2 which has 1 slice and 3 sampler/VME per slice
    CM_HAL_WALKER_MODE_QUAD     = 4,    // applies in HSW GT3 which has 2 slices and 2 sampler/VME per slice
    CM_HAL_WALKER_MODE_HEX      = 6,    // applies in BDW GT2 which has 2 slices and 3 sampler/VME per slice
    CM_HAL_WALKER_MODE_OCT      = 8     // may apply in future Gen media architectures
};

// The following structure must match the structure
// MHW_WALKER_PARAMS defined in mhw_render.h
struct CM_HAL_WALKER_PARAMS
{
    uint32_t InterfaceDescriptorOffset : 5;
    uint32_t CmWalkerEnable            : 1;
    uint32_t ColorCountMinusOne        : 8;
    uint32_t UseScoreboard             : 1;
    uint32_t ScoreboardMask            : 8;
    uint32_t MidLoopUnitX              : 2;
    uint32_t MidLoopUnitY              : 2;
    uint32_t MiddleLoopExtraSteps      : 5;
    uint32_t GroupIdLoopSelect         : 24;
    uint32_t                           : 8;

    uint32_t InlineDataLength;
    uint8_t *pInlineData;
    uint32_t dwLocalLoopExecCount;
    uint32_t dwGlobalLoopExecCount;

    CM_HAL_WALKER_MODE WalkerMode;
    CM_HAL_WALKER_XY BlockResolution;
    CM_HAL_WALKER_XY LocalStart;
    CM_HAL_WALKER_XY LocalEnd;
    CM_HAL_WALKER_XY LocalOutLoopStride;
    CM_HAL_WALKER_XY LocalInnerLoopUnit;
    CM_HAL_WALKER_XY GlobalResolution;
    CM_HAL_WALKER_XY GlobalStart;
    CM_HAL_WALKER_XY GlobalOutlerLoopStride;
    CM_HAL_WALKER_XY GlobalInnerLoopUnit;

    bool bAddMediaFlush;
    bool bRequestSingleSlice;
};
typedef CM_HAL_WALKER_PARAMS *PCM_HAL_WALKER_PARAMS;


struct SamplerParam
{
    unsigned int sampler_table_index;
    unsigned int heap_offset;
    unsigned int bti;
    unsigned int bti_stepping;
    unsigned int bti_multiplier;
    bool user_defined_bti;
    bool regular_bti;
    unsigned int element_type;
    unsigned int size;
};


struct CM_SURFACE_BTI_INFO
{
    uint32_t dwNormalSurfaceStart;   // start index of normal surface
    uint32_t dwNormalSurfaceEnd;     // end index of normal surface
    uint32_t dwReservedSurfaceStart; // start index of reserved surface
    uint32_t dwReservedSurfaceEnd;   // end index of reserved surface
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
    PCM_HAL_STATE m_pCmState;

    CM_HAL_GENERIC(PCM_HAL_STATE pCmState):
        m_pCmState(pCmState),
        m_sliceShutdownEnabled(false) {};

    virtual ~CM_HAL_GENERIC(){};

    //!
    //! \brief    Get GPUCopy Kernel's ISA and Size
    //! \details  Get GPUCopy Kernel's ISA and Size
    //! \param    [out] pIsa
    //!           pointer to memory of gpucopy isa
    //! \param    [out] IsaSize
    //!           size of gpucopy isa
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetCopyKernelIsa
                        (void  *&pIsa, uint32_t &IsaSize) = 0;

    //!
    //! \brief    Get GPU Surface Initialization Kernel's ISA and Size
    //! \details  Get GPU Surface Initialization Kernel's ISA and Size
    //! \param    [out] pIsa
    //!           pointer to memory of gpu initialization isa
    //! \param    [out] IsaSize
    //!           size of gpu initialization isa
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetInitKernelIsa
                        (void  *&pIsa, uint32_t &IsaSize) = 0;

    //!
    //! \brief    Set media walker parameters
    //! \details  Set media walker parameters
    //! \param    [in]  engineeringParams
    //!           engineering params passed by caller
    //! \param    [in] pWalkerParams
    //!           pointer to walker paramaeters to set
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMediaWalkerParams(
                        CM_WALKING_PARAMETERS          engineeringParams,
                        PCM_HAL_WALKER_PARAMS          pWalkerParams) = 0;

    //!
    //! \brief    Set Surface Memory Object Control
    //! \details  Convert Memory Object Control bits to RenderHal Surface State
    //! \param    [in]  wMemObjCtl
    //!           wMemObjCtl passed by caller
    //! \param    [in] pParams
    //!           pointer to surface state param
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HwSetSurfaceMemoryObjectControl(
                        uint16_t                        wMemObjCtl,
                        PRENDERHAL_SURFACE_STATE_PARAMS pParams) = 0;

    //!
    //! \brief    Register Sampler8x8
    //! \details  Register Sampler8x8
    //! \param    [in]  pParam
    //!           pointer to cmhal sampler8x8 param
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RegisterSampler8x8(
                        PCM_HAL_SAMPLER_8X8_PARAM    pParam) = 0;

    //!
    //! \brief    Submit commmand to kernel mode driver
    //! \details  Submit commmand to kernel mode driver
    //! \param    [in]  pBatchBuffer
    //!           pointer to mhw batch buffer to submit
    //! \param    [in]  iTaskId
    //!           id of task
    //! \param    [in]  pKernels
    //!           pointer to array of kernel param
    //! \param    [out]  ppCmdBuffer
    //!           pointer cmd buffer returned to cm event
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCommands(
                        PMHW_BATCH_BUFFER       pBatchBuffer,
                        int32_t                 iTaskId,
                        PCM_HAL_KERNEL_PARAM    *pKernels,
                        void                    **ppCmdBuffer) = 0;

    //!
    //! \brief    Update platform information from power option
    //! \details  Power option can be used to do slice shutdown. This function is
    //!           to adjust platform info (EU numbers/Slice number) accordingly.
    //! \param    [in]  platformInfo
    //!           pointer to platform info
    //! \param    [in]  bEUSaturation
    //!           if EU Saturation required.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdatePlatformInfoFromPower(
                        PCM_PLATFORM_INFO platformInfo,
                        bool              bEUSaturation) = 0;

    //!
    //! \brief    Get media walker's max width
    //! \details  Get media walker's max width
    //! \return   media walker's max width
    //!
    virtual uint32_t   GetMediaWalkerMaxThreadWidth() = 0;

    //!
    //! \brief    Get media walker's max height
    //! \details  Get media walker's max height
    //! \return   media walker's max height
    //!
    virtual uint32_t   GetMediaWalkerMaxThreadHeight() = 0;

    //!
    //! \brief    Get Surface binding table index info
    //! \details  Get Surface binding table index info, including the start/end index of
    //!           reserved surfaces and normal surfaces
    //! \param    [in]  pBTIinfo
    //!           pointer to binding table information
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetHwSurfaceBTIInfo(
                       PCM_SURFACE_BTI_INFO pBTIinfo) = 0;

    //!
    //! \brief    Set Suggested L3 Configuration to RenderHal
    //! \details  Set Suggested L3 Configuration to RenderHal
    //! \param    [in]  L3Conf
    //!           index of selected configuration
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSuggestedL3Conf(
                       L3_SUGGEST_CONFIG L3Conf) = 0;

    //!
    //! \brief    Allocate SIP/CSR Resource for Preemption and Debug
    //! \details  Allocate SIP/CSR Resource for Preemption and Debug
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS AllocateSIPCSRResource()  = 0;

    //!
    //! \brief    Get the stepping string of Gen platform
    //! \details  Get the stepping string of Gen platform
    //! \param    [in,out]  stepinfostr
    //!           reference to stepping information string
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetGenStepInfo(char*& stepinfostr) = 0;

    //!
    //! \brief    Get the platform code and GT type of Gen platform
    //! \param    [out]  pPlatformID
    //!           pointer to the platform code defined in GPU_PLATFORM
    //! \param    [out]  pGengt
    //!           pointer to the GT type defined in GPU_GT_PLATFORM
    //! \param    [out]  ppPlatformStr
    //!           pointer to platform string
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetGenPlatformInfo(uint32_t *pPlatformID,
                                                 uint32_t *pGengt,
                                                 const char **ppPlatformStr)
    {
        ASSIGN_IF_VALID(pPlatformID, m_platformID);
        ASSIGN_IF_VALID(pGengt, m_gengt);
        ASSIGN_IF_VALID(ppPlatformStr, m_pPlatformStr);
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    set the platform code and GT type of Gen platform
    //! \param    [in]  platformID
    //!           the platform code defined in GPU_PLATFORM
    //! \param    [in]  gengt
    //!           the GT type defined in GPU_GT_PLATFORM
    //! \param    [in]  pPlatformStr
    //!           platform string
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetGenPlatformInfo(uint32_t platformID,
                                                 uint32_t gengt,
                                                 const char *pPlatformStr)
    {
        m_platformID = platformID;
        m_gengt = gengt;
        m_pPlatformStr = pPlatformStr;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    enable or disable the slice downdown feature
    //! \param    [in]  enabled
    //!           true: enable slice shutdown; false: disable slice shutdown
    virtual void EnableSliceShutdown(bool enabled)
    {
        m_sliceShutdownEnabled = enabled;
    }

    //!
    //! \brief    return whether the slice downdown feature is enabled
    //! \return   bool
    //!           true: slice shutdown enabled; false: slice shutdown disabled
    virtual bool IsSliceShutdownEnabled()
    {
        return m_sliceShutdownEnabled;
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
    virtual bool IsSurf3DQpitchSupportedbyHw(){ return true;};

    //!
    //! \brief    Check if compare mask supported by hw in conditional buffer
    //! \details  Check if compare mask supported by hw in conditional buffer
    //!           compare mask is supported from SKL
    //! \return   True for SKL+
    virtual bool IsCompareMaskSupportedbyHw(){ return true;};

    //!
    //! \brief    Check if two adjacent sampler index requried by hardware
    //! \details  Check if two adjacent sampler index requried by hardware
    //!           compare mask is supported from SKL
    //! \return   True for BDW, and False for SKL+
    virtual bool IsAdjacentSamplerIndexRequiredbyHw(){ return false;};

    //!
    //! \brief    Check if the WA to disable surface compression required
    //! \details  Check if the WA to disable surface compression required
    //! \return   False for BDW, and True for SKL+
    virtual bool IsSurfaceCompressionWARequired(){ return true;};

    //!
    //! \brief    Check if scoreboading parameters are supported.
    //! \details  Check if scoreboading parameters are supported.
    //! \return   False for Not needed, and True for Needed
    virtual bool IsScoreboardParamNeeded() { return true; };

    //!
    //! \brief    Check if surface color format is supported by VME
    //! \return   true if format is supported by VME surface
    virtual bool IsSupportedVMESurfaceFormat(MOS_FORMAT format) {
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
    virtual bool IsGPUCopySurfaceNoCacheWARequired(){ return true;};

    //!
    //! \brief    Check if one plane P010 surface is supported.
    //! \details  Check if one plane P010 surface is supported.
    //!           one plane P010 surface is supported since CNL.
    //! \return   False for pre-CNL, and True for CNL+
    virtual bool IsP010SinglePassSupported() { return true; };

    //!
    //! \brief    Get Convolution Sampler Index.
    //! \details  Get Convolution Sampler Index.
    //! \param    [in]  pSamplerParam
    //!           pointer to sampler param
    //! \param    [in]  pSamplerIndexTable
    //!           pointer to sampler index table
    //! \param    [in]  nSamp8X8Num
    //!           number of sampler8x8
    //! \param    [in]  nSampConvNum
    //!           number of conv sampler
    //! \return   Sampler index.
    virtual int32_t GetConvSamplerIndex(
            PMHW_SAMPLER_STATE_PARAM  pSamplerParam,
            char                     *pSamplerIndexTable,
            int32_t                   nSamp8X8Num,
            int32_t                   nSampConvNum) = 0;

    //!
    //! \brief    Set L3 values in CM hal layer.
    //! \details  Use the L3 struct to set L3 to different platforms.
    //! \param    [in]  values_ptr
    //!           pointer to input L3 config values
    //! \param    [in]  cmhal_l3_cache_ptr
    //!           pointer to hal layer L3 config values
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetL3CacheConfig(
            const L3ConfigRegisterValues *values_ptr,
            PCmHalL3Settings cmhal_l3_cache_ptr) = 0;

    //!
    //! \brief    Get sampler element count for a given sampler type.
    //! \details  Convert the sampler type to how many element for this sampler
    //!           type for current platform.
    //! \param    [in]  sampler_param_ptr
    //!           pointer to the sampler param defined by MHW
    //! \param    [in,out]  sampler_param
    //!           Will get sampler size, sampler element type, sampler bti
    //!           stepping and sampler multiplier for this type of sampler
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS GetSamplerParamInfoForSamplerType(
            PMHW_SAMPLER_STATE_PARAM sampler_param_ptr,
            SamplerParam  &sampler_param) = 0;


    //!
    //! \brief    Get the expected configuration for specific GT
    //! \details  Get the expected configuration for specific GT
    //! \param    [in]  pExpectedConfig
    //!           pointer to expected config
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetExpectedGtSystemConfig(
        PCM_EXPECTED_GT_SYSTEM_INFO pExpectedConfig) = 0;

protected:
    uint32_t m_platformID;
    uint32_t m_gengt;
    const char *m_pPlatformStr;
    std::vector<uint32_t> m_cisaGenIDs;
    bool m_sliceShutdownEnabled;
};

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMHALGENERIC_H_
