/*
* Copyright (c) 2018, Intel Corporation
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
//! \file    mos_gpucontext.h
//! \brief   Container class for the basic gpu context
//!

#ifndef __MOS_GPU_CONTEXT_H__
#define __MOS_GPU_CONTEXT_H__

#include "mos_os.h"

class CmdBufMgr;
class CommandBuffer;
class CommandBufferSpecific;
class GraphicsResource;

//!
//! \class  GpuContext
//!
class GpuContext
{
public:
    //!
    //! \brief  Constructor
    //!
    GpuContext()
    {
        m_nodeOrdinal = MOS_GPU_NODE_MAX;
        m_gpuContextHandle = 0;
        m_gpuContext = MOS_GPU_CONTEXT_RENDER;
    }

    //!
    //! \brief  Destructor
    //!
    virtual ~GpuContext(){}

    //!
    //! \brief    Static entrypoint, get the gpu context object
    //! \param    [in] gpuNode
    //!           Gpu node
    //! \param    [in] mosGpuCtx
    //!           Mos gpu context
    //! \param    [in] cmdBufMgr
    //!           Command buffer manager
    //! \param    [in] reusedContext
    //!           Reused gpu context
    //! \return   GpuContext*
    //!           the os specific object for gpu context
    //!
    static class GpuContext* Create(
        const MOS_GPU_NODE gpuNode,
        MOS_GPU_CONTEXT    mosGpuCtx,
        CmdBufMgr         *cmdBufMgr,
        GpuContext        *reusedContext);

    //!
    //! \brief    Clear gpu context
    //!
    virtual void Clear() = 0;

    //!
    //! \brief    Verify command buffer size
    //! \details  Verifys the buffer to be used for rendering GPU commands is large enough
    //! \param    [in] requestedSize
    //!           Buffer size requested
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful (command buffer will be large enough)
    //!           otherwise failed reason
    //!
    virtual MOS_STATUS VerifyCommandBufferSize(
        const uint32_t requestedSize) = 0;

    //!
    //! \brief    Get indirect state size
    //! \details  Retrieves indirect state to be used for rendering purposes
    //! \param    [out] offset
    //!           Pointer to indirect buffer offset
    //! \param    [out] size
    //!           Pointer to indirect buffer size
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetIndirectState(
        uint32_t *offset,
        uint32_t *size) = 0;

    //!
    //! \brief    Get Indirect State Pointer
    //! \param    [out] indirectState
    //!           Pointer to Indirect State Buffer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetIndirectStatePointer(
        uint8_t **indirectState) = 0;

    //!
    //! \brief    Get command buffer
    //! \details  Retrieves buffer to be used for rendering GPU commands
    //! \param    [in/out] cmdBuffer
    //!           Pointer to Command Buffer control structure
    //! \param    [in] flags
    //!           Flags to indicate command buffer property
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            flags) = 0;

    //!
    //! \brief    Return command buffer space
    //! \details  Return unused command buffer space
    //! \param    [in/out] cmdBuffer
    //!           Pointer to Command Buffer control structure
    //! \param    [in] flags
    //!           Flags to indicate command buffer property
    //!
    virtual void ReturnCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            flags) = 0;

    //!
    //! \brief    Submit command buffer
    //! \details  Submit the command buffer
    //! \param    [in] osInterface
    //!           Pointer to OS interface structure
    //! \param    [in] cmdBuffer
    //!           Pointer to Command Buffer control structure
    //! \param    [in] nullRendering
    //!           boolean null rendering
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCommandBuffer(
        PMOS_INTERFACE      osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                nullRendering) = 0;

    //!
    //! \brief    Resizes the buffer to be used for rendering GPU commands
    //! \param    [in] requestedCommandBufferSize
    //!           requested command buffer size
    //! \param    [in] requestedPatchListSize
    //!           requested patch list size
    //! \param    [in] flags
    //!           Flags to indicate command buffer property
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS ResizeCommandBufferAndPatchList(
        uint32_t requestedCommandBufferSize,
        uint32_t requestedPatchListSize,
        uint32_t flags) = 0;

    //!
    //! \brief    Resets Gpu context States
    //! \details  Resets all status in current gpu status
    //!
    virtual void ResetGpuContextStatus() = 0;

    //!
    //! \brief    Get Gpu context handle
    //! \details  return the index in gpucontext mgr pool for current gpu context 
    //!
    GPU_CONTEXT_HANDLE GetGpuContextHandle() { return m_gpuContextHandle; }

    //!
    //! \brief    Set Gpu context handle
    //! \details  Register the index in gpucontext mgr pool for current gpu context 
    //!
    void SetGpuContextHandle(GPU_CONTEXT_HANDLE gpuContextHandle)
    {
        m_gpuContextHandle = gpuContextHandle;
    }

    //!
    //! \brief    Get Gpu context Node
    //! \details  Return the hardware node for current gpu context 
    //!
    MOS_GPU_NODE GetContextNode() { return m_nodeOrdinal; }

    //!
    //! \brief    Set Gpu context Node
    //! \details  Set the hardware node for current gpu context 
    //!
    void SetContextNode(MOS_GPU_NODE nodeOrdinal) { m_nodeOrdinal = nodeOrdinal; }

    //!
    //! \brief    Get Gpu context ID
    //! \details  Return the umd side gpu context id for current gpu context 
    //!
    MOS_GPU_CONTEXT GetCpuContextID() { return m_gpuContext; }

    //!
    //! \brief    Get indirect state size
    //!
    uint32_t GetIndirectStateSize() { return m_IndirectHeapSize; }

    //!
    //! \brief    Set indirect state size
    //!
    virtual MOS_STATUS SetIndirectStateSize(const uint32_t size) = 0;

    //!
    //! \brief    Get status buffer resource for current gpu context
    //! \return   GraphicsResource*
    //!
    GraphicsResource* GetStatusBufferResource() { return m_statusBufferResource; }

    //!
    //! \brief    Get status buffer mos resource for current gpu context
    //! \return   GraphicsResource*
    //!
    MOS_RESOURCE_HANDLE GetStatusBufferMosResource() { return m_statusBufferMosResource; }

protected:
    //! \brief    Hardware node for current gpu context
    MOS_GPU_NODE m_nodeOrdinal;

    //! \brief    Indirect heap size (SSH area in DMA buffer)
    uint32_t m_IndirectHeapSize = 0;

    //! \brief    Related command buffer manager
    CmdBufMgr *m_cmdBufMgr = nullptr;

    //! \brief    Index in gpucontext mgr pool for current gpu context
    GPU_CONTEXT_HANDLE m_gpuContextHandle;

    //! \brief    Gpu status report buffer
    GraphicsResource *m_statusBufferResource = nullptr;
    //! \brief    GPU status mos resource
    MOS_RESOURCE_HANDLE m_statusBufferMosResource = nullptr;

    //! \brief    Track the GPU Context Client Info
    MOS_GPU_CONTEXT m_gpuContext;
};
#endif  // #ifndef __MOS_GPU_CONTEXT_H__
