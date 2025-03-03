﻿/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     mos_gpucontextmgr_next.h
//! \brief    Container class for the gpu context manager
//!

#ifndef __MOS_GPU_CONTEXT_MGR_NEXT_H__
#define __MOS_GPU_CONTEXT_MGR_NEXT_H__

#include "mos_defs.h"
#include "mos_gpucontext_next.h"

class OsContextNext;
//!
//! \class  GpuContextMgr
//!
class GpuContextMgrNext
{
public:
    //!
    //! \brief  Constructor
    //!
    GpuContextMgrNext(OsContextNext *osContext);

    //!
    //! \brief    Copy constructor
    //!
    GpuContextMgrNext(const GpuContextMgrNext&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    GpuContextMgrNext& operator=(const GpuContextMgrNext&) = delete;

    //!
    //! \brief  Destructor
    //!
    virtual ~GpuContextMgrNext();

    //!
    //! \brief    Static entrypoint, get gpu context manager object
    //! \return   GpuContextMgrNext*
    //!           gpu context manager specific object if success, else nullptr
    //!
    static GpuContextMgrNext* GetObject(
        OsContextNext      *osContext);

    //!
    //! \brief    Initialized the gpu context manager
    //!
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    Clean up the gpu context manager
    //! \details  This function mainly celar all allocated gpu context in 
    //!           apu context array
    //!
    void CleanUp();

    //!
    //! \brief    Create GPU context
    //! \param    [in] gpuNode
    //!           Reqired gpu node
    //! \param    [in] cmdBufMgr
    //!           Command buffer manager 
    //! \return   GpuContext*
    //!           Gpu context pointer if success, otherwise nullptr
    //!
    virtual GpuContextNext* CreateGpuContext(
        const MOS_GPU_NODE gpuNode,
        CmdBufMgrNext     *cmdBufMgr);

    //!
    //! \brief    Get GPU context base on gpu context handle
    //! \detail   Gpu context manager maintain an array, gpu context handle
    //!           as its index.
    //! \param    [in] gpuContextHandle
    //!           Reqired gpu context's handle
    //! \return   GpuContext*
    //!           Gpu context pointer if success, otherwise nullptr
    //!
    virtual GpuContextNext* GetGpuContext(GPU_CONTEXT_HANDLE gpuContextHandle);

    //!
    //! \brief    Destroy specified gpu context
    //! \param    [in] gpuContext
    //!           Gpu context need to be destroyed
    //!
    virtual void DestroyGpuContext(GpuContextNext *gpuContext);

    //!
    //! \brief    Destroy all gpu context instance
    //! \detail   Destroy all gpu context maintained in m_gpuContextArray
    //!
    void DestroyAllGpuContexts();

    //!
    //! \brief    Determine whether gpu context reuse is needed
    //! \detail   Implementation to be added after reuse scheme is nailed down
    //! \return   bool
    //!           True if needed, otherwise false
    //!
    bool ContextReuseNeeded();

    //!
    //! \brief    Select one gpu context to be reused
    //! \detail   Implementation to be added after reuse scheme is nailed down
    //! \return   GpuContext*
    //!           Gpu context pointer if success, otherwise nullptr
    //!
    GpuContextNext* SelectContextToReuse();

    //!
    //! \brief    Get os context used in manager
    //! \return   OsContext*
    //!           Os context pointer if success, otherwise nullptr
    //!
    OsContextNext* GetOsContext(){ return m_osContext; }

    std::map<GPU_CONTEXT_HANDLE, GpuContextNext *> &GetGpuContextMap()
    {
        return m_gpuContextMap;
    }
        //!
    //! \brief    Get the validity flag
    //! \return   bool
    //!           Get the validity flag of GpuContextMgrNext
    //!
    bool IsInitialized()
    {
        return m_initialized;
    }

    PMOS_MUTEX GetGpuContextArrayMutex()
    {
        return m_gpuContextArrayMutex;
    }

    //! \brief   Indicate whether new gpu context is inserted into the first slot w/ null ctx handle 
    //!          or always at the end of the gpucontext array
    bool m_noCycledGpuCxtMgmt = false;

protected:
    GpuContextMgrNext() = default;

    //! \brief    Os Context
    OsContextNext *m_osContext = nullptr;

    //! \brief    Gpu context array mutex
    PMOS_MUTEX m_gpuContextArrayMutex = nullptr;

    //! \brief    Gpu context array mutex for delete
    PMOS_MUTEX m_gpuContextDeleteArrayMutex = nullptr;

    //! \brief    Gpu context count
    uint32_t m_gpuContextCount = 0;
    uint32_t m_gpuContextHanleForNonCycledCase = 0;
    //! \brief    Maintained gpu context array
    std::map<GPU_CONTEXT_HANDLE, GpuContextNext *> m_gpuContextMap;

    //! \brief   Flag to indicate gpu context mgr initialized or not
    bool m_initialized = false;

MEDIA_CLASS_DEFINE_END(GpuContextMgrNext)
};

#endif  // #ifndef __MOS_GPU_CONTEXT_MGR_NEXT_H__
