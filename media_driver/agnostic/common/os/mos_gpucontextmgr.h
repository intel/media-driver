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
//! \file     mos_gpucontextmgr.h
//! \brief    Container class for the gpu context manager
//!

#ifndef __MOS_GPU_CONTEXT_MGR_H__
#define __MOS_GPU_CONTEXT_MGR_H__

#include "mos_os.h"
#include "mos_gpucontext.h"

//!
//! \class  GpuContextMgr
//!
class GpuContextMgr
{
public:
    //!
    //! \brief  Constructor
    //!
    GpuContextMgr(GT_SYSTEM_INFO *gtSystemInfo, OsContext *osContext);

    //!
    //! \brief    Copy constructor
    //!
    GpuContextMgr(const GpuContextMgr&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    GpuContextMgr& operator=(const GpuContextMgr&) = delete;

    //!
    //! \brief  Destructor
    //!
    virtual ~GpuContextMgr();

    //!
    //! \brief    Static entrypoint, get gpu context manager object
    //! \return   GpuContextMgr*
    //!           gpu context manager specific object if success, else nullptr
    //!
    static GpuContextMgr* GetObject(
        GT_SYSTEM_INFO *gtSystemInfo,
        OsContext      *osContext);

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
    //! \param    [in] mosGpuCtx
    //!           Required gpu context type
    //! \return   GpuContext*
    //!           Gpu context pointer if success, otherwise nullptr
    //!
    GpuContext* CreateGpuContext(
        const MOS_GPU_NODE gpuNode,
        CmdBufMgr         *cmdBufMgr,
        MOS_GPU_CONTEXT    mosGpuCtx);

    //!
    //! \brief    Get GPU context base on gpu context handle
    //! \detail   Gpu context manager maintain an array, gpu context handle
    //!           as its index.
    //! \param    [in] gpuContextHandle
    //!           Reqired gpu context's handle
    //! \return   GpuContext*
    //!           Gpu context pointer if success, otherwise nullptr
    //!
    GpuContext* GetGpuContext(GPU_CONTEXT_HANDLE gpuContextHandle);

    //!
    //! \brief    Destroy specified gpu context
    //! \param    [in] gpuContext
    //!           Gpu context need to be destroyed
    //!
    void DestroyGpuContext(GpuContext *gpuContext);

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
    GpuContext* SelectContextToReuse();

    //!
    //! \brief    Get os context used in manager
    //! \return   OsContext*
    //!           Os context pointer if success, otherwise nullptr
    //!
    OsContext* GetOsContext() { return m_osContext; }

    //!
    //! \brief    Get Gpu context number
    //! \return   uint32_t
    //!           Number of all Gpu contexts, include the node which was already destroyed
    //!
    uint32_t GetGpuContextNumber()
    {
        return m_gpuContextArray.size();
    }

protected:
    //! \brief    Gt system info 
    //! \detail   reserve to reuse gpu context
    GT_SYSTEM_INFO m_gtSystemInfo = {};

    //! \brief    Os Context
    OsContext *m_osContext = nullptr;

    //! \brief    Gpu context array mutex
    PMOS_MUTEX m_gpuContextArrayMutex = nullptr;
    
    //! \brief    Gpu context count
    uint32_t m_gpuContextCount = 0;

    //! \brief    Maintained gpu context array
    std::vector<GpuContext *> m_gpuContextArray;
};

#endif  // #ifndef __MOS_GPU_CONTEXT_MGR_H__
