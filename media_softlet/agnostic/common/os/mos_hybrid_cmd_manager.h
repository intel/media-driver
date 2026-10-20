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
#ifndef __MOS_HYBRID_CMD_MANAGER_H__
#define __MOS_HYBRID_CMD_MANAGER_H__
#include "mos_defs.h"
#include "media_class_trace.h"
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>

class CmdPackage
{
public:
    virtual ~CmdPackage()                             = default;
    virtual MOS_STATUS                  Submit()      = 0;
    virtual MOS_STATUS                  Wait()        = 0;
    virtual std::unique_ptr<CmdPackage> Clone() const = 0;
    virtual bool                        Releaseable() { return true; };
    virtual bool                        IsAsyncExecute() { return false; };
};

enum class HYBRID_MGR_SUBMIT_MODE
{
    ASYNC = 0,
    SYNC  = 1
};

class HybridCmdMgr
{
public:
    HybridCmdMgr() {};
    virtual ~HybridCmdMgr();

    MOS_STATUS SubmitPackage(CmdPackage &cmdPackage);
    MOS_STATUS StartThread();
    MOS_STATUS StopThread();
    bool       Enabled();
    MOS_STATUS SetSubmitMode(HYBRID_MGR_SUBMIT_MODE submitMode);

private:
    void Consumer();
    MOS_STATUS SubmitPackageInAsync(std::unique_ptr<CmdPackage> cmdPackage);
    MOS_STATUS SubmitPackageInSync(std::unique_ptr<CmdPackage> cmdPackage);

private:
    HYBRID_MGR_SUBMIT_MODE m_submitMode = HYBRID_MGR_SUBMIT_MODE::ASYNC;

    std::queue<std::unique_ptr<CmdPackage>>  m_queue;
    std::unique_ptr<CmdPackage>              m_lastCmdPackage   = nullptr;
    std::mutex                               m_queueMutex       = {};
    std::vector<std::unique_ptr<CmdPackage>> m_deferReleaseList = {};
    std::condition_variable                  m_wakeCondition    = {};
    bool                                     m_stopFlag         = false;
    std::thread                              m_consumerThread;
    std::mutex                               m_threadMutex = {};

MEDIA_CLASS_DEFINE_END(HybridCmdMgr)
};

#endif