/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_cmdbufmgr_next.h
//! \brief    Container class for the comamnd buffer manager
//!

#ifndef __COMMAND_BUFFER_MANAGER_NEXT_H__
#define __COMMAND_BUFFER_MANAGER_NEXT_H__

#include "mos_commandbuffer_next.h"
#include "mos_gpucontextmgr_next.h"

//!
//! \class  CmdBufMgr
//!
class CmdBufMgrNext
{
public:
    //!
    //! \brief  Constructor
    //!
    CmdBufMgrNext();

    //!
    //! \brief  Destructor
    //!
    ~CmdBufMgrNext();

    //!
    //! \brief    Static entrypoint, get command buffer manager object
    //! \return   CmdBufMgrNext*
    //!           comamnd buffer manager object if success, else nullptr
    //!
    static CmdBufMgrNext* GetObject();

    //!
    //! \brief    Initialize comamnd buffer manager object
    //! \details  This function mainly create initial cmd buffer as input size
    //! \param    [in] osContext
    //!           Pointer to the osContext handle
    //! \param    [in] cmdBufSize
    //!           initialized command buffer size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(OsContextNext *osContext, uint32_t cmdBufSize);

    //!
    //! \brief    Clean up the command buffer manager
    //! \details  This function mainly celar all allocated comamnd buffer in 
    //!           available pool and in use pool
    //!
    void CleanUp();

    //!
    //! \brief    Clean up the command buffer manager
    //! \details  This function will pick up one proper command buffer from 
    //!           available pool, internal logic in below 3 conditions:
    //!           1: if available pool has command buffer and its size bigger than
    //!              required, directly put it into in use pool and return;
    //!           2: if available pool has command buffer but size smaller than
    //!              required, only create one command buffer as reqired and put
    //!              it to in  use pool directly;
    //!           3: if available pool is empty, will re-allocate bunch of command
    //!              buffers, buffer number base on m_initBufNum, buffer size
    //!              base on input required size. After re-allocate, put first buf
    //!              into inuse pool, remains push to available pool.
    //! \param    [in] size
    //!           Required command buffer size
    //! \return   CommandBuffer*
    //!           Proper comamnd bufffer pointer if success, other wise nullptr
    //!
    CommandBufferNext *PickupOneCmdBuf(uint32_t size);

    //!
    //! \brief    insert the command buffer into available pool in proper location.
    //! \details  This function will find a the first cmd buffer which equal or  
    //!           small cmd buf size in available pool, and then insert it.
    //! \param    [in] cmdBuf
    //!           command buffer to be released
    //!
    void UpperInsert(CommandBufferNext *cmdBuf);

    //!
    //! \brief    Release command buffer from in-use status to standby status
    //! \details  This function designed for situations which need retire or 
    //!           discard in use command buffer, it directly erase command buf
    //!           from in use pool and push it to available pool. If the command
    //!           buffer cannot be found inside in-use pool, some thing must be 
    //!           wrong.
    //! \param    [in] cmdBuf
    //!           Command buffer need to be released
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, other wise fail reason
    //!
    MOS_STATUS ReleaseCmdBuf(CommandBufferNext *cmdBuf);

    //!
    //! \brief    Resize command buffer with required size
    //! \param    [in] cmdBufToResize
    //!           Command buffer need to be resized
    //! \param    [in] newSize
    //!           required new size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, other wise fail reason
    //!
    MOS_STATUS ResizeOneCmdBuf(CommandBufferNext *cmdBufToResize, uint32_t newSize);

    //!
    //! \brief    Get the validity flag
    //! \return   bool
    //!           Get the validity flag of cmdBufMgrNext
    //!
    bool IsInitialized()
    {
        return m_initialized;
    }

private:
    //!
    //! \brief    Self define compare method as std:sort input 
    //! \detail   Command buffer size will be compared 
    //! \param    [in] a
    //!           Input command buffer to be compared
    //! \param    [in] b
    //!           Input command buffer to be compared
    //! \return   bool
    //!           true if a size bigger than b, otherwise false
    //!
    static bool GreaterSizeSort(CommandBufferNext *a, CommandBufferNext *b);

    //! \brief   Max comamnd buffer number for per manager, including all
    //!          command buffer in availble pool and in-use pool
    constexpr static uint32_t m_maxPoolSize = 1098304;

    //! \brief   Current command buffer number in available and in-use pool
    uint32_t m_cmdBufTotalNum = 0;

    //! \brief   Command buffer number when bunch of re-allocate
    constexpr static uint32_t m_bufIncStepSize = 8;

    //! \brief   Initial command buffer number
    constexpr static uint32_t m_initBufNum = 32;

    //! \brief   Sorted List of available command buffer pool
    std::vector<CommandBufferNext *> m_availableCmdBufPool;

    //! \brief   Mutex for available command buffer pool
    PMOS_MUTEX m_availablePoolMutex = nullptr;

    //! \brief   List of in used command buffer pool
    std::vector<CommandBufferNext *> m_inUseCmdBufPool;

    //! \brief   Mutex for in-use command buffer pool
    PMOS_MUTEX m_inUsePoolMutex = nullptr;

    //! \brief   Flag to indicate cmd buf mgr initialized or not
    bool m_initialized = false;

    //! \brief   Corresponding os context
    OsContextNext *m_osContext = nullptr;
};
#endif // __COMMAND_BUFFER_MANAGER_NEXT_H__
