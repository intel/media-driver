/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     media_render_cmd_packet.h
//! \brief    Defines the interface for media render workload cmd packet
//! \details  The media cmd packet is dedicated for command buffer sequenece submit
//!
#ifndef __MEDIA_RENDER_CMD_PACKET_H__
#define __MEDIA_RENDER_CMD_PACKET_H__

#include <stdint.h>
#include <set>
#include "hal_kerneldll_next.h"
#include "media_cmd_packet.h"
#include "renderhal.h"

class MediaFeatureManager;
class MediaTask;
class MhwCpInterface;

#define RENDER_PACKET_CHK_NULL_RETURN(_ptr) \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_HW, 0, _ptr)

#define RENDER_PACKET_CHK_STATUS_RETURN(_stmt) \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, 0, _stmt)

#define RENDER_PACKET_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...) \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_HW, 0, _stmt, _message, ##__VA_ARGS__)

#define RENDER_PACKET_ASSERTMESSAGE(_message, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_HW, 0, _message, ##__VA_ARGS__)

#define RENDER_PACKET_NORMALMESSAGE(_message, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_HW, 0, _message, ##__VA_ARGS__)

#define RENDER_PACKET_VERBOSEMESSAGE(_message, ...) \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_HW, 0, _message, ##__VA_ARGS__)

#define RENDER_PACKET_ASSERT(_expr) \
    MOS_ASSERT(MOS_COMPONENT_HW, 0, _expr)

#define RENDER_PACKET_CHK_NULL_WITH_DESTROY_RETURN_VALUE(_ptr, destroyFunction) \
    MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(MOS_COMPONENT_HW, 0, (nullptr == _ptr), destroyFunction, MOS_STATUS_NULL_POINTER, "error nullptr!")

#define RENDER_PACKET_CHK_STATUS_WITH_DESTROY_RETURN_VALUE(_stmt, destroyFunction)                                                  \
{                                                                                                                                   \
    MOS_STATUS sts = (MOS_STATUS)(_stmt);                                                                                           \
    MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(MOS_COMPONENT_HW, 0, (MOS_STATUS_SUCCESS != sts), destroyFunction, sts, "error status!") \
}

//!
//! \brief Initialize MHW Kernel Param struct for loading Kernel
//!
#define INIT_MHW_KERNEL_PARAM(MhwKernelParam, _pKernelEntry)                        \
    do                                                                              \
    {                                                                               \
        MOS_ZeroMemory(&(MhwKernelParam), sizeof(MhwKernelParam));                  \
        (MhwKernelParam).pBinary  = (_pKernelEntry)->pBinary;                       \
        (MhwKernelParam).iSize    = (_pKernelEntry)->iSize;                         \
        (MhwKernelParam).iKUID    = (_pKernelEntry)->iKUID;                         \
        (MhwKernelParam).iKCID    = (_pKernelEntry)->iKCID;                         \
        (MhwKernelParam).iPaddingSize = (_pKernelEntry)->iPaddingSize;              \
    } while(0)

typedef struct _PIPECONTRL_PARAMS
{
    bool bUpdateNeeded;
    bool bEnableDataPortFlush;
    bool bUnTypedDataPortCacheFlush;
    bool bFlushRenderTargetCache;
    bool bInvalidateTextureCache;
} PIPECONTRL_PARAMS, *PPIPECONTRL_PARAMS;

typedef struct _KERNEL_WALKER_PARAMS
{
    int32_t                             iBindingTable;
    int32_t                             iMediaID;
    int32_t                             iCurbeOffset;
    int32_t                             iCurbeLength;

    int32_t                             iBlocksX;
    int32_t                             iBlocksY;
    RECT                                alignedRect;
    PIPECONTRL_PARAMS                   pipeControlParams;
    bool                                isVerticalPattern;
    bool                                bSyncFlag;
    bool                                bFlushL1;
    bool                                isGroupStartInvolvedInGroupSize;    // true if group start need be involved in the group size.
    bool                                calculateBlockXYByAlignedRect;      // true if iBlocksX/iBlocksY is calculated by alignedRect in RenderCmdPacket instead of kernel object.
    bool                                forcePreferredSLMZero;              // true if preferredSLM need force to 0.

    bool                                isEmitInlineParameter;
    uint32_t                            inlineDataLength;
    uint8_t*                            inlineData;

    uint32_t                            threadWidth;
    uint32_t                            threadHeight;
    uint32_t                            threadDepth;

    bool                                isGenerateLocalID;
    MHW_EMIT_LOCAL_MODE                 emitLocal;

    bool                                hasBarrier;
    uint32_t                            slmSize;
    PMHW_INLINE_DATA_PARAMS             inlineDataParamBase;
    uint32_t                            inlineDataParamSize;
}KERNEL_WALKER_PARAMS, * PKERNEL_WALKER_PARAMS;

typedef struct _KERNEL_PACKET_RENDER_DATA
{
    // Kernel Information
    RENDERHAL_KERNEL_PARAM              KernelParam;
    Kdll_CacheEntry                     KernelEntry;
    int32_t                             iCurbeLength;
    int32_t                             iInlineLength;
    int32_t                             iCurbeOffset;

    MHW_SAMPLER_STATE_PARAM             SamplerStateParams;           //!< Sampler State
    PMHW_AVS_PARAMS                     pAVSParameters;               //!< AVS parameters
    MHW_SAMPLER_AVS_TABLE_PARAM         mhwSamplerAvsTableParam;      //!< params for AVS scaling 8x8 table

    // Media render state
    PRENDERHAL_MEDIA_STATE              mediaState;

    int32_t                             bindingTable;
    uint32_t                            bindingTableEntry;
    int32_t                             mediaID;

    KERNEL_WALKER_PARAMS                walkerParam;
    PMHW_VFE_SCOREBOARD                 scoreboardParams;

    int32_t                             kernelAllocationID;

    // Debug parameters
    // Kernel Used for current rendering
    char* pKernelName;
} KERNEL_PACKET_RENDER_DATA, * PKERNEL_PACKET_RENDER_DATA;

typedef enum _WALKER_TYPE
{
    WALKER_TYPE_DISABLED = 0,
    WALKER_TYPE_MEDIA,
    WALKER_TYPE_COMPUTE
}WALKER_TYPE;

//!
//! \brief VPHAL SS/EU setting
//!
struct SseuSetting
{
    uint8_t   numSlices;
    uint8_t   numSubSlices;
    uint8_t   numEUs;
    uint8_t   reserved;       // Place holder for frequency setting
};

typedef struct _RENDERHAL_SURFACE_NEXT : public _RENDERHAL_SURFACE
{
    uint32_t Index;
}RENDERHAL_SURFACE_NEXT, * PRENDERHAL_SURFACE_NEXT;

class RenderCmdPacket : virtual public CmdPacket, public mhw::mi::Itf::ParSetting
{
public:
    RenderCmdPacket(MediaTask* task, PMOS_INTERFACE pOsInterface, RENDERHAL_INTERFACE* m_renderHal);

    virtual ~RenderCmdPacket();
    virtual MOS_STATUS Init();
    virtual MOS_STATUS Destroy();
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket);

    // Currently only support HDC read/write, for sampler enabling will be in next step
    // Step1 : render engine set up
    MOS_STATUS RenderEngineSetup();

    // Step2: Kernel State Set up
    virtual MOS_STATUS KernelStateSetup(
        void *kernel)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual uint32_t SetSurfaceForHwAccess(
        PMOS_SURFACE                    surface,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        bool                            bWrite,
        std::set<uint32_t>             &stateOffsets);

    // Step3: RSS Setup, return index insert in binding table
    virtual uint32_t SetSurfaceForHwAccess(
        PMOS_SURFACE                    surface,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        bool                            bWrite);

    // Step3: RSS Setup with fixed binding index, return index insert in binding table
    virtual MOS_STATUS SetSurfaceForHwAccess(
        PMOS_SURFACE                    surface,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        uint32_t                        &bindingIndex,
        bool                            bWrite,
        PRENDERHAL_SURFACE_STATE_ENTRY *surfaceEntries      = nullptr,
        uint32_t *                      numOfSurfaceEntries = nullptr);

    virtual MOS_STATUS SetSurfaceForHwAccess(
        PMOS_SURFACE                    surface,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        std::set<uint32_t>             &bindingIndexes,
        bool                            bWrite,
        std::set<uint32_t>             &stateOffsets,
        uint32_t                        capcityOfSurfaceEntries = 0,
        PRENDERHAL_SURFACE_STATE_ENTRY *surfaceEntries      = nullptr,
        uint32_t                       *numOfSurfaceEntries = nullptr);

    virtual uint32_t SetBufferForHwAccess(
        PMOS_SURFACE                    buffer,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        bool                            bWrite,
        std::set<uint32_t>             &stateOffsets);

    virtual uint32_t SetBufferForHwAccess(
        PMOS_SURFACE                    buffer,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        uint32_t                        bindingIndex,
        bool                            bWrite);
    
    virtual MOS_STATUS SetBufferForHwAccess(
        PMOS_SURFACE                    buffer,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        std::set<uint32_t>             &bindingIndexes,
        bool                            bWrite,
        std::set<uint32_t>             &stateOffsets);

    virtual uint32_t SetBufferForHwAccess(
        MOS_BUFFER                      buffer,
        PRENDERHAL_SURFACE_NEXT         pRenderSurface,
        PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
        bool                            bWrite);

    // Step4: Packet to prepare the curbe data Setup, then call packet to set it up
    // PData point to the Curbe prepared by packet
    MOS_STATUS SetupCurbe(
        void *   pData,
        uint32_t curbeLength,
        uint32_t maximumNumberofThreads = 0);

    // Step6: different kernel have different media walker settings
    virtual MOS_STATUS SetupMediaWalker()
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS PrepareMediaWalkerParams(KERNEL_WALKER_PARAMS params, MHW_WALKER_PARAMS &mediaWalker);

    MOS_STATUS PrepareComputeWalkerParams(KERNEL_WALKER_PARAMS params, MHW_GPGPU_WALKER_PARAMS &gpgpuWalker);

    bool m_isMultiBindingTables = false;

    bool m_isLargeSurfaceStateNeeded = false;

    bool m_isMultiKernelOneMediaState = false;

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS StallBatchBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }
#endif

protected:
    // Step5: Load Kernel
    virtual MOS_STATUS LoadKernel();

    // for VPP usage, there are more data need to updated, create as virtual for future inplemention in VPP
    virtual MOS_STATUS InitRenderHalSurface(
        MOS_SURFACE        surface,
        PRENDERHAL_SURFACE pRenderSurface);

    virtual MOS_STATUS InitRenderHalBuffer(
        MOS_BUFFER         surface,
        PRENDERHAL_SURFACE pRenderSurface);

    virtual void OcaDumpDbgInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext)
    {
    }

    virtual MOS_STATUS SetMediaFrameTracking(RENDERHAL_GENERIC_PROLOG_PARAMS &genericPrologParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS InitKernelEntry();

    MOS_STATUS SetPowerMode(
        uint32_t KernelID);

    bool IsMiBBEndNeeded(
        PMOS_INTERFACE pOsInterface)
    {
        // need to be remodify
        bool needed = true;

        return needed;
    }

    void ResetBindingTableEntry()
    {
        m_renderData.bindingTableEntry = 0;
    }

    virtual void UpdateKernelConfigParam(RENDERHAL_KERNEL_PARAM &kernelParam);

protected:
    PRENDERHAL_INTERFACE        m_renderHal = nullptr;
    MhwCpInterface*             m_cpInterface = nullptr;
    MediaFeatureManager*        m_featureManager = nullptr;

    // Perf
    VPHAL_PERFTAG               PerfTag = VPHAL_NONE; // need to check the perf setting in codec

    // Kernel Render Data
    uint32_t                     m_kernelCount = 0;

    // Kernel Render Data
    KERNEL_PACKET_RENDER_DATA   m_renderData = {};

    // object walker: media walker/compute walker
    WALKER_TYPE                 m_walkerType = WALKER_TYPE_DISABLED;

    MHW_WALKER_PARAMS       m_mediaWalkerParams = {};

    MHW_GPGPU_WALKER_PARAMS m_gpgpuWalkerParams = {};

    PMHW_BATCH_BUFFER       pBatchBuffer = nullptr;

    MHW_PIPE_CONTROL_PARAMS m_pipeCtlParams = {};

    MHW_MEDIA_STATE_FLUSH_PARAM m_flushParam = {};
    uint32_t                m_flushMode = 0;
    PMOS_RESOURCE           m_presDest = nullptr;
    uint32_t                m_postSyncOp = 0;
    uint32_t                m_resourceOffset = 0;
    uint32_t                m_dataDW1 = 0;
    uint32_t                m_dataDW2 = 0;
    uint32_t                m_genericMediaStateClear : 1;
    uint32_t                m_IndirectStatePointersDisable : 1;
    uint32_t                m_disableCSStall : 1;
    uint32_t                m_bInvalidateTextureCache : 1;
    uint32_t                m_bFlushRenderTargetCache : 1;
    uint32_t                m_bInvalidateStateCache : 1;
    uint32_t                m_bInvalidateConstantCache : 1;
    uint32_t                m_bInvalidateVFECache : 1;
    uint32_t                m_bInvalidateInstructionCache : 1;
    uint32_t                m_bTlbInvalidate : 1;
    uint32_t                m_bHdcPipelineFlush : 1;
    uint32_t                m_bKernelFenceEnabled : 1;
    bool                    m_bFlushToGo = true;
    uint8_t                 m_ui8InterfaceDescriptorOffset = 0;
MEDIA_CLASS_DEFINE_END(RenderCmdPacket)
};
#endif // __MEDIA_RENDER_CMD_PACKET_H__
