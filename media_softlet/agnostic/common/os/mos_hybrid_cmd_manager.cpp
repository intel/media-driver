/*
* Copyright (c) 2025, Intel Corporation
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
#include "mos_hybrid_cmd_manager.h"
#include "mos_util_debug.h"
#include <functional>

HybridCmdMgr ::~HybridCmdMgr()
{
    StopThread();
}

MOS_STATUS HybridCmdMgr::StartThread()
{
    std::lock_guard<std::mutex> lock(m_threadMutex);
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopFlag = false;
    }
    if (!m_consumerThread.joinable())
    {
        MOS_OS_NORMALMESSAGE("Hybrid Consumer Thread Started");
        m_consumerThread = std::thread(std::bind(&HybridCmdMgr::Consumer, this));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HybridCmdMgr::StopThread()
{
    std::lock_guard<std::mutex> lock(m_threadMutex);
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_stopFlag = true;
    }
    m_wakeCondition.notify_one();

    if (m_consumerThread.joinable())
    {
        MOS_OS_NORMALMESSAGE("Hybrid Consumer Thread is Stopping");
        m_consumerThread.join();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HybridCmdMgr::SubmitPackage(CmdPackage &cmdPackage)
{
    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        m_queue.push(cmdPackage.Clone());
    }
    m_wakeCondition.notify_one();

    return MOS_STATUS_SUCCESS;
}

bool HybridCmdMgr::Enabled()
{
    return m_consumerThread.joinable();
}

void HybridCmdMgr::Consumer()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_wakeCondition.wait(lock, [&] { return !m_queue.empty() || m_stopFlag; });

        //When asked to stop, need to first finish the queue
        //Or the monitor fence of context will never update and destroy will stuck
        if (m_queue.empty() && m_stopFlag)
        {
            break;
        }

        while (!m_queue.empty())
        {
            std::unique_ptr<CmdPackage> lastCmdPackage = std::move(m_lastCmdPackage);
            m_lastCmdPackage                           = std::move(m_queue.front());
            m_queue.pop();
            lock.unlock();

            if (lastCmdPackage != nullptr)
            {
                if (MOS_STATUS_SUCCESS != lastCmdPackage->Wait())
                {
                    MOS_OS_ASSERTMESSAGE("Wait Last Cmd Package Fail");
                }
            }
            else
            {
                MOS_OS_ASSERTMESSAGE("Last Cmd Package is nullptr, so skip wait");
            }
            if (m_lastCmdPackage != nullptr)
            {
                if (MOS_STATUS_SUCCESS != m_lastCmdPackage->Submit())
                {
                    MOS_OS_ASSERTMESSAGE("Execute Current Package Fail");
                }
            }
            else
            {
                MOS_OS_ASSERTMESSAGE("Current Cmd Package is nullptr, cannot submit");
            }

            //check the cmd packages that need defered released and release those releaseable ones
            auto it = m_deferReleaseList.begin();
            while (it != m_deferReleaseList.end())
            {
                if ((*it) == nullptr || (*it)->Releaseable())
                {
                    it = m_deferReleaseList.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            if (lastCmdPackage != nullptr && !lastCmdPackage->Releaseable())
            {
                //for those asyn cmd package, they cannot be immediately relased after submit
                m_deferReleaseList.push_back(std::move(lastCmdPackage));
            }

            lock.lock();
        }
    }
}