/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     mos_commandbuffer_next.h
//! \brief    Container for comamnd buffer
//!

#ifndef __MOS_COMMANDBUFFER_NEXT_H__
#define __MOS_COMMANDBUFFER_NEXT_H__

#include "mos_graphicsresource_next.h"
#include "mos_gpucontext_next.h"

//!
//! \class  CommandBuffer
//!
class CommandBufferNext
{
public:
    //!
    //! \brief  Constructor
    //!
    CommandBufferNext(CmdBufMgrNext *cmdBufMgr)
    {
        m_cmdBufMgr = cmdBufMgr;
    }

    //!
    //! \brief  Destructor
    //!
    virtual ~CommandBufferNext(){}

    //!
    //! \brief    Create Os specific command buffer
    //! \return   CommandBufferNext*
    //!           Os specific command buffer
    //!
    static class CommandBufferNext *CreateCmdBuf(CmdBufMgrNext *cmdBufMgr);

    //!
    //! \brief    Allocate graphics resource for current command buffer
    //! \param    [in] osContext
    //!           Related os context
    //! \param    [in] size
    //!           Required size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise fail reason
    //!
    virtual MOS_STATUS Allocate(OsContextNext* osContext, uint32_t size) = 0;

    //!
    //! \brief  Free allocated graphics resource for current command buffer
    //!
    virtual void Free() = 0;

    //!
    //! \brief    Bind current command buffer to gived gpu context
    //! \param    [in] gpuContext
    //!           Gpu context to be binded
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise fail reason
    //!
    virtual MOS_STATUS BindToGpuContext(GpuContextNext* gpuContext) = 0;

    //!
    //! \brief    Unbind current command buffer from gpu context
    //! \param    [in] isNative
    //!           Indicate it is unbinded from dummy or native gpu context in async mode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise fail reason
    //!
    virtual void UnBindToGpuContext(bool isNative = true) = 0;

    //!
    //! \brief    Resize command buffer
    //! \param    [in] newSize
    //!           Required size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, otherwise fail reason
    //!
    virtual MOS_STATUS ReSize(uint32_t newSize) = 0;

    //!
    //! \brief    Query command buffer if it is in HW execution
    //!           Always return false in normal mode, only return true in aync mode
    //!           when cmd buffer still in use by HW
    //! \return   bool
    //!           True if is used by HW, false if not
    //!
    virtual bool IsUsedByHw() { return false; }

    //!
    //! \brief    Query command buffer if it is in Cmd List
    //!           Always return false in normal mode, only return true in aync mode
    //!           when cmd buffer still in one or multiple cmd Lists
    //! \return   bool
    //!           True if is in cmdlist, false if not
    //!
    virtual bool IsInCmdList() { return false; }

    //!
    //! \brief    Query command buffer ready to use
    //! \return   bool
    //!           True if ready, false if not ready
    //!
    bool IsReadyToUse() { return m_readyToUse; }

    //!
    //! \brief    Get command buffer size
    //! \return   uint32_t
    //!           Command buffer size
    //!
    uint32_t GetCmdBufSize() { return m_size; }

    //!
    //! \brief    Get current command buffer attached resource
    //! \return   GraphicsResource*
    //!           Attached graphics resource
    //!
    GraphicsResourceNext* GetResource() { return m_graphicsResource; }

    //!
    //! \brief    Get current command buffer's lock address for cpu side usage
    //! \return   uint8_t*
    //!           Current command buffer's lock address for cpu side usage
    //!
    uint8_t* GetLockAddr() { return m_lockAddr; }

    //!
    //! \brief    Get last native gpu context
    //! \return   GpuContextNext*
    //!           Pointer to the last native gpu context
    //!
    GpuContextNext *GetLastNativeGpuContext()
    {
        return m_lastNativeGpuContextHandle == MOS_GPU_CONTEXT_INVALID_HANDLE ?
            nullptr : m_lastNativeGpuContext;
    }

    //!
    //! \brief    Get last native gpu context handle
    //! \return   GPU_CONTEXT_HANDLE
    //!           Pointer to the last native gpu context handle
    //!
    GPU_CONTEXT_HANDLE GetLastNativeGpuContextHandle()
    {
        return m_lastNativeGpuContextHandle;
    }

    //!
    //! \brief    Reset last native gpu context
    //! \return   void
    //!
    void ResetLastNativeGpuContext()
    {
        m_lastNativeGpuContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;
        m_lastNativeGpuContext       = nullptr;
        return;
    }

    //!
    //! \brief    Get last gpu context
    //! \return   GpuContextNext*
    //!           Pointer to the last gpu context
    //!
    GpuContextNext *GetGpuContext()
    {
        return m_gpuContext;
    }

    //!
    //! \brief    Get last gpu context handle
    //! \return   GPU_CONTEXT_HANDLE
    //!           Pointer to the last gpu context handle
    //!
    GPU_CONTEXT_HANDLE GetGpuContextHandle()
    {
        return m_gpuContextHandle;
    }

    //!
    //! \brief    Reset gpu context
    //! \return   void
    //!
    void ResetGpuContext()
    {
        m_gpuContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;
        m_gpuContext       = nullptr;
        return;
    }

    //!
    //! \brief    Get cmd buffer manager current cmd buffer belong to
    //! \return   CmdBufMgrNext *
    //!           Pointer to the command buffer manager
    //!
    CmdBufMgrNext *GetCmdBufferMgr()
    {
        return m_cmdBufMgr;
    }

protected:
    //!
    //! \brief    Set ready to use
    //! \params   [in] readyToUse
    //!
    void SetReadyToUse(bool readyToUse) { m_readyToUse = readyToUse; }

    //! \brief    Os context
    OsContextNext*        m_osContext        = nullptr;

    //! \brief    Gpu context binded to
    GpuContextNext*       m_gpuContext       = nullptr;

    //! \brief    Gpu context handle
    GPU_CONTEXT_HANDLE    m_gpuContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;

    //! \brief    Last executed native Gpu context in async mode (Should not be used in normal mode)
    GpuContextNext       *m_lastNativeGpuContext = nullptr;

    //! \brief    the cmd buf manager it belongs to
    CmdBufMgrNext       *m_cmdBufMgr           = nullptr;

    //! \brief    Last native Gpu context handle
    GPU_CONTEXT_HANDLE    m_lastNativeGpuContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;

    //! \brief    Graphics resource for command buffer
    GraphicsResourceNext* m_graphicsResource = nullptr;

    //! \brief    Lock address for cpu side usage
    uint8_t*          m_lockAddr         = nullptr;

    //! \brief    Indicate if ready to use
    bool              m_readyToUse       = false;

    //! \brief    Command buffer size
    uint32_t          m_size             = 0;
MEDIA_CLASS_DEFINE_END(CommandBufferNext)
};
#endif // __MOS_COMMANDBUFFERNext_NEXT_H__
