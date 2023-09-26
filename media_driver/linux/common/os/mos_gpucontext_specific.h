/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     mos_gpucontext_specific.h
//! \brief    Container class for the linux/Android specfic gpu context
//!

#ifndef __GPU_CONTEXT_SPECIFIC_H__
#define __GPU_CONTEXT_SPECIFIC_H__

#include "mos_gpucontext.h"
#include "mos_graphicsresource_specific.h"
#include "mos_oca_interface_specific.h"

#define ENGINE_INSTANCE_SELECT_ENABLE_MASK                   0xFF
#define ENGINE_INSTANCE_SELECT_COMPUTE_INSTANCE_SHIFT        16
#define ENGINE_INSTANCE_SELECT_VEBOX_INSTANCE_SHIFT          8
#define ENGINE_INSTANCE_SELECT_VDBOX_INSTANCE_SHIFT          0

//!
//! \class  GpuContextSpecific
//! \brief  Linux/Android specific gpu context 
//!
class GpuContextSpecific : public GpuContext
{
public:
    //!
    //! \brief  Constructor
    //!
    GpuContextSpecific(
        const MOS_GPU_NODE gpuNode,
        MOS_GPU_CONTEXT    mosGpuCtx,
        CmdBufMgr         *cmdBufMgr,
        GpuContext        *reusedContext);

    //!
    //! \brief  Destructor
    //!
    ~GpuContextSpecific();

    //!
    //! \brief    Initialize gpu context
    //! \details  Linux specific initialize for gpu context
    //! \param    [out] osContext
    //!           Os context pointer
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] GpuNode
    //!           GPU node
    //! \param    [in] createOption
    //!           Context option
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS Init(OsContext *osContext,
                    PMOS_INTERFACE osInterface,
                    MOS_GPU_NODE GpuNode,
                    PMOS_GPUCTX_CREATOPTIONS createOption);

    void Clear(void);

    //!
    //! \brief    Register graphics resource
    //! \details  Set the Allocation Index in OS resource structure
    //! \param    [out] osContext
    //!           Os context pointer
    //! \param    [in] writeFlag
    //!           Write Flag
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS RegisterResource(
        PMOS_RESOURCE osResource,
        bool          writeFlag);

    MOS_STATUS GetCommandBuffer(
        PMOS_COMMAND_BUFFER comamndBuffer,
        uint32_t            flags);

    MOS_STATUS VerifyCommandBufferSize(const uint32_t requestedSize)
    {
        return (m_commandBufferSize < requestedSize) ? MOS_STATUS_UNKNOWN:MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetIndirectState(
        uint32_t *offset,
        uint32_t *size);

    MOS_STATUS GetIndirectStatePointer(
        uint8_t **indirectState);

    MOS_STATUS SetIndirectStateSize(const uint32_t size);

    //!
    //! \brief    Set patch entry
    //! \details  Sets the patch entry in patch list
    //! \param    [in] osInterface
    //!           Pointer to OS interface structure
    //! \param    [in] params
    //!           patch entry params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetPatchEntry(
        PMOS_INTERFACE          osInterface,
        PMOS_PATCH_ENTRY_PARAMS params);

    void ReturnCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            flags);

    //!
    //! \brief    reset command buffer space
    //! \details  resets the command buffer space
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS ResetCommandBuffer();

    //!
    //! \brief    Verifys the patch list to be used for rendering GPU commands is large enough
    //! \param    [in] requestedSize
    //!           Patch list size to be verified
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS VerifyPatchListSize(const uint32_t requestedSize)
    {
        return (requestedSize > m_maxPatchLocationsize) ? MOS_STATUS_UNKNOWN : MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SubmitCommandBuffer(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                nullRendering);

    MOS_STATUS ResizeCommandBufferAndPatchList(
        uint32_t requestedCommandBufferSize,
        uint32_t requestedPatchListSize,
        uint32_t flags);

    //!
    //! \brief    Resizes the buffer to be used for rendering GPU commands
    //! \param    [in] requestedSize
    //!           Requested size
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS ResizeCommandBuffer(uint32_t requestedSize);

    //!
    //! \brief    Get GPU status tag
    //! \details  Gets the status tag
    //! \return   uint32_t
    //!           Returns the tag
    //!
    uint32_t   GetGpuStatusTag() { return m_GPUStatusTag; }

    //!
    //! \brief    Get i915 ctx
    //! \param    [in] ctxNodeIndex
    //!           ctx index in array
    //! \return   MOS_LINUX_CONTEXT
    //!           mos linux ctx in array of ctxNodeIndex
    //!
    MOS_LINUX_CONTEXT* GetI915Context(uint32_t ctxNodeIndex){return ctxNodeIndex < (MAX_ENGINE_INSTANCE_NUM+1) ?  m_i915Context[ctxNodeIndex] : nullptr;}

    //!
    //! \brief    Increment GPU status tag
    //!
    void       IncrementGpuStatusTag();

    void       ResetGpuContextStatus();

    //!
    //! \brief    Allocate gpu status buffer for gpu sync
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateGPUStatusBuf();

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
    void                PushCmdResPtr(const void *p) { m_cmdResPtrs.push_back(p); }
    void                ClearCmdResPtrs() { m_cmdResPtrs.clear(); }
    const std::vector<const void *> &GetCmdResPtrs() const { return m_cmdResPtrs; }
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
protected:
    //!
    //! \brief    Map resources with aux plane to aux table
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS MapResourcesToAuxTable(mos_linux_bo *cmd_bo);

    //!
    //! \brief    Submit command buffer for single pipe in scalability mode
    //! \return   int32_t
    //!           Return 0 if successful, otherwise error code
    //!
    int32_t SubmitPipeCommands(MOS_COMMAND_BUFFER *cmdBuffer,
                               MOS_LINUX_BO *cmdBo,
                               PMOS_CONTEXT osContext,
                               const std::vector<MOS_LINUX_BO *> &skipSyncBoList,
                               uint32_t execFlag,
                               int32_t dr4);

    //!
    //! \brief    Combin cmd buffer for each pipe and use one submission  in scalability mode
    //! \return   int32_t
    //!           Return 0 if successful, otherwise error code
    //!
    int32_t ParallelSubmitCommands(std::map<uint32_t, PMOS_COMMAND_BUFFER> secondaryCmdBufs,
                               PMOS_CONTEXT osContext,
                               uint32_t execFlag,
                               int32_t dr4);

    //!
    //! \brief    Set the flags of engin quering according to create options
    //! \return   void
    //!
    void SetEngineQueryFlags(
        PMOS_GPUCTX_CREATOPTIONS option,
        __u64 &caps);

#if (_DEBUG || _RELEASE_INTERNAL)
    bool SelectEngineInstanceByUser(struct i915_engine_class_instance *engineMap,
        uint32_t *engineNum, uint32_t userEngineInstance, MOS_GPU_NODE gpuNode);
#endif
    void UnlockPendingOcaBuffers(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_CONTEXT mosContext);

private:
    //! \brief    internal command buffer pool per gpu context
    std::vector<CommandBuffer *> m_cmdBufPool;

    //! \brief    internal command buffer pool per gpu context
    PMOS_MUTEX m_cmdBufPoolMutex = nullptr;

    //! \brief    next fetch index of m_cmdBufPool
    uint32_t m_nextFetchIndex = 0;

    //! \brief    initialized comamnd buffer size
    uint32_t m_commandBufferSize = 0;

    //! \brief    Flag to indicate current command buffer flused or not, if not
    //!           re-use it
    volatile bool m_cmdBufFlushed = false;

    //! \brief    internal back up for in-use command buffer
    PMOS_COMMAND_BUFFER m_commandBuffer = nullptr;

    //! \brief    secondary command buffers for scalability
    std::map<uint32_t, PMOS_COMMAND_BUFFER> m_secondaryCmdBufs;

    //! \brief    Allcoation List related struct
    ALLOCATION_LIST *m_allocationList = nullptr;
    uint32_t         m_numAllocations = 0;  //!< number of registered allocation list
    uint32_t         m_maxNumAllocations = 0;  //!< max number of allocation list

    //! \brief    Pathc List related struct
    PATCHLOCATIONLIST *m_patchLocationList = nullptr;
    uint32_t           m_currentNumPatchLocations = 0; //!< number of registered patch list
    uint32_t           m_maxPatchLocationsize; //!< max number of patch list

   //! \brief    Resource registrations
    uint32_t      m_resCount = 0;  //!< number of resources registered
    PMOS_RESOURCE m_attachedResources = nullptr;  //!< Pointer to resources list
    bool         *m_writeModeList     = nullptr;  //!< Write mode

    //! \brief    GPU Status tag
    uint32_t m_GPUStatusTag = 0;

    //! \brief    Os context
    OsContext *m_osContext = nullptr;

    MOS_GPUCTX_CREATOPTIONS_ENHANCED *m_createOptionEnhanced = nullptr;
    MOS_LINUX_CONTEXT*  m_i915Context[MAX_ENGINE_INSTANCE_NUM+1] = {};
    uint32_t     m_i915ExecFlag = 0;

#if (_DEBUG || _RELEASE_INTERNAL)
    /*!\brief bits(23...16), (15...8), (7...0) are for Compute, VEbox and VDbox ;
    single or multi engine instance can be selected at same time(0x10103 to select Compute1, VE1, VD1&VD2 for example)*/
    uint32_t m_engineInstanceSelect = 0x0;
#endif

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
    std::vector<const void *> m_cmdResPtrs; //!< Command OS resource pointers registered by pfnRegisterResource
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED
};
#endif  // __GPU_CONTEXT_SPECIFIC_H__
