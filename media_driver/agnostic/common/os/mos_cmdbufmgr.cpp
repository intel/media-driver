﻿/*
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
//! \file    mos_cmdbufmgr.cpp
//! \brief   Container class for the basic command buffer manager
//!
#include "mos_cmdbufmgr.h"
#include <algorithm>

CmdBufMgr::CmdBufMgr()
{
    MOS_OS_FUNCTION_ENTER;

    m_availableCmdBufPool.clear();
    m_inUseCmdBufPool.clear();
    m_initialized = false;
}

CmdBufMgr::~CmdBufMgr()
{
    MOS_OS_FUNCTION_ENTER;
}

CmdBufMgr* CmdBufMgr::GetObject()
{
    MOS_OS_FUNCTION_ENTER;
    return MOS_New(CmdBufMgr);
}

MOS_STATUS CmdBufMgr::Initialize(OsContext *osContext, uint32_t cmdBufSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;
    MOS_OS_CHK_NULL_RETURN(osContext);

    if (!m_initialized)
    {
        m_osContext          = osContext;

        m_inUsePoolMutex     = MOS_CreateMutex();
        MOS_OS_CHK_NULL_RETURN(m_inUsePoolMutex);

        m_availablePoolMutex = MOS_CreateMutex();
        MOS_OS_CHK_NULL_RETURN(m_availablePoolMutex);

        for (uint32_t i = 0; i < m_initBufNum; i++)
        {
            auto cmdBuf = CommandBuffer::CreateCmdBuf();
            if (cmdBuf == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("input nullptr returned by CommandBuffer::CreateCmdBuf.");
                return MOS_STATUS_INVALID_HANDLE;
            }

            eStatus = cmdBuf->Allocate(m_osContext, cmdBufSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Allocate CmdBuf#%d failed", i);
                return MOS_STATUS_INVALID_HANDLE;
            }

            MOS_LockMutex(m_availablePoolMutex);
            m_availableCmdBufPool.push_back(cmdBuf);
            MOS_UnlockMutex(m_availablePoolMutex);

            m_cmdBufTotalNum++;
        }

        m_initialized = true;
    }

    return MOS_STATUS_SUCCESS;
}

void CmdBufMgr::CleanUp()
{
    MOS_OS_FUNCTION_ENTER;

    if (m_initialized)
    {
        CommandBuffer *cmdBuf = nullptr;
        MOS_LockMutex(m_availablePoolMutex);
    
        for (auto& cmdBuf : m_availableCmdBufPool)
        {
            if (cmdBuf != nullptr)
            {
                cmdBuf->Free();
                MOS_Delete(cmdBuf);
            }
            else
            {
                MOS_OS_ASSERTMESSAGE("Unexpected, found null command buffer!");
            }
        }
    
        // clear available command buffer pool
        m_availableCmdBufPool.clear();
        MOS_UnlockMutex(m_availablePoolMutex);
        MOS_LockMutex(m_inUsePoolMutex);
    
        if (!m_inUseCmdBufPool.empty())
        {
            MOS_OS_ASSERTMESSAGE("Unexpected, inUseCmdBufPool is not empty!");
            for (auto& cmdBuf : m_inUseCmdBufPool)
            {
                if (cmdBuf != nullptr)
                {
                    cmdBuf->Free();
                    MOS_Delete(cmdBuf);
                }
            }
        }
    
        // clear in-use command buffer pool
        m_inUseCmdBufPool.clear();
        MOS_UnlockMutex(m_inUsePoolMutex);
    
        m_cmdBufTotalNum = 0;
        m_initialized    = false;
        MOS_DestroyMutex(m_inUsePoolMutex);
        m_inUsePoolMutex = nullptr;
        MOS_DestroyMutex(m_availablePoolMutex);
        m_availablePoolMutex = nullptr;
    }
}

CommandBuffer *CmdBufMgr::PickupOneCmdBuf(uint32_t size)
{
    MOS_OS_FUNCTION_ENTER;

    if (!m_initialized)
    {
        MOS_OS_ASSERTMESSAGE("cmd buf pool need be initialized before buffer picking up!");
        return nullptr;
    }

    // lock for both in-use and available command buffer pool before pick up
    MOS_LockMutex(m_inUsePoolMutex);
    MOS_LockMutex(m_availablePoolMutex);

    CommandBuffer* cmdBuf = nullptr;
    CommandBuffer* retbuf  = nullptr;
    MOS_STATUS     eStatus = MOS_STATUS_SUCCESS;

    if (!m_availableCmdBufPool.empty())
    {
        cmdBuf = *(m_availableCmdBufPool.begin());
        if (cmdBuf == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("available command buf pool is null.");
            MOS_UnlockMutex(m_inUsePoolMutex);
            MOS_UnlockMutex(m_availablePoolMutex);
            return nullptr;
        }

        // find available buf
        if (size <= cmdBuf->GetCmdBufSize())
        {
            m_inUseCmdBufPool.push_back(cmdBuf);

            m_availableCmdBufPool.erase(m_availableCmdBufPool.begin());

            MOS_OS_VERBOSEMESSAGE("successfully get available buf from pool");
        }
        // available buf  is not large enough, need reallocate
        else
        {
            MOS_OS_VERBOSEMESSAGE("find available buf, but is not large enough");

            cmdBuf = CommandBuffer::CreateCmdBuf();
            if (cmdBuf == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("input nullptr returned by CommandBuffer::CreateCmdBuf.");
            }
            else
            {
                eStatus = cmdBuf->Allocate(m_osContext, size);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_OS_ASSERTMESSAGE("Allocate CmdBuf failed");
                }

                // directly push into inuse pool
                m_inUseCmdBufPool.push_back(cmdBuf);
                m_cmdBufTotalNum++;
            }
        }

        retbuf = cmdBuf;
    }
    // no available buf in the pool, will allocate in batch
    else
    {
        MOS_OS_VERBOSEMESSAGE("No more cmd buf in the pool");

        if (m_cmdBufTotalNum < m_maxPoolSize)
        {
            MOS_OS_VERBOSEMESSAGE("Increase the cmd buf pool size by %d", m_bufIncStepSize);
            for (uint32_t i = 0; i < m_bufIncStepSize; i++)
            {
                cmdBuf = CommandBuffer::CreateCmdBuf();
                if (cmdBuf == nullptr)
                {
                    MOS_OS_ASSERTMESSAGE("input nullptr returned by CommandBuffer::CreateCmdBuf.");
                    continue;
                }

                eStatus = cmdBuf->Allocate(m_osContext, size);
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    MOS_OS_ASSERTMESSAGE("Allocate CmdBuf#%d failed", i);
                    continue;
                }

                if (i == 0)
                {
                    // directly push into inuse pool
                    m_inUseCmdBufPool.push_back(cmdBuf);
                    retbuf = cmdBuf;
                }
                else
                {
                    m_availableCmdBufPool.insert(m_availableCmdBufPool.begin(), cmdBuf);
                }
                m_cmdBufTotalNum++;
            }
            // sort by decent order
            std::sort(m_availableCmdBufPool.begin(), m_availableCmdBufPool.end(), &CmdBufMgr::GreaterSizeSort);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("No availabe cmd buf in pool and the total buf num hit the ceiling, may need wait for a while.");
            retbuf = nullptr;
        }
    }

    // unlock after got return buffer
    MOS_UnlockMutex(m_inUsePoolMutex);
    MOS_UnlockMutex(m_availablePoolMutex);

    return retbuf;
}

void CmdBufMgr::UpperInsert(CommandBuffer *cmdBuf)
{
    auto it = std::find_if(m_availableCmdBufPool.begin(), m_availableCmdBufPool.end(), [=](CommandBuffer * p1){return p1->GetCmdBufSize() < cmdBuf->GetCmdBufSize();});
    m_availableCmdBufPool.emplace(it, cmdBuf);
}

MOS_STATUS CmdBufMgr::ReleaseCmdBuf(CommandBuffer *cmdBuf)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_STATUS     eStatus = MOS_STATUS_SUCCESS;

    if (!m_initialized)
    {
        MOS_OS_ASSERTMESSAGE("cmd buf pool need be initialized before buffer release!");
        return MOS_STATUS_NULL_POINTER;
    }

    MOS_OS_CHK_NULL_RETURN(cmdBuf);

    // lock for both in-use and available command buffer pool before release
    MOS_LockMutex(m_inUsePoolMutex);
    MOS_LockMutex(m_availablePoolMutex);

    bool           found = false;
    for (auto iter = m_inUseCmdBufPool.begin(); iter != m_inUseCmdBufPool.end(); iter++)
    {
        if (cmdBuf == *iter)
        {
            found = true;
            m_inUseCmdBufPool.erase(iter);
            break;
        }
    }

    if (!found)
    {
        MOS_OS_ASSERTMESSAGE("Cannot find the specified cmdbuf in inusepool, sth must be wrong!");
        eStatus = MOS_STATUS_UNKNOWN;
    }
    else
    {
        UpperInsert(cmdBuf);
    }

    // unlock after release buffer
    MOS_UnlockMutex(m_inUsePoolMutex);
    MOS_UnlockMutex(m_availablePoolMutex);

    return eStatus;
}

MOS_STATUS CmdBufMgr::ResizeOneCmdBuf(CommandBuffer *cmdBufToResize, uint32_t newSize)
{
    MOS_OS_FUNCTION_ENTER;

    MOS_OS_CHK_NULL_RETURN(cmdBufToResize);

    if (!m_initialized)
    {
        MOS_OS_ASSERTMESSAGE("cmd buf pool need be initialized before buffer resize!");
        return MOS_STATUS_UNKNOWN;
    }

    return cmdBufToResize->ReSize(newSize);
}
