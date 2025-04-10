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
#ifndef __NPU_CMD_PACKET_H__
#define __NPU_CMD_PACKET_H__
#include "media_cmd_packet.h"
#include "levelzero_npu_interface.h"

class NpuWLContext
{
public:
    NpuWLContext(uint32_t timeoutMs) : m_timeoutMs(timeoutMs) {};
    std::vector<ze_command_list_handle_t>& CmdList()
    {
        return m_cmdLists;
    }
    ze_fence_handle_t& Fence()
    {
        return m_fence;
    }
    std::condition_variable& Condition()
    {
        return m_cv;
    }
    std::mutex& Mutex()
    {
        return m_mtx;
    }
    std::thread& Thread()
    {
        return m_initThread;
    }
    bool& Initialized()
    {
        return m_initialized;
    }
    bool& Failed()
    {
        return m_failed;
    }
    uint32_t GetTimeoutMs()
    {
        return m_timeoutMs;
    }

private:
    //NpuWlContext cannot be copied for it contains condition_variable and mutex
    std::vector<ze_command_list_handle_t> m_cmdLists    = {};
    ze_fence_handle_t                     m_fence     = nullptr;
    bool                                  m_initialized = false;
    bool                                  m_failed      = false;
    std::condition_variable               m_cv          = {};
    std::mutex                            m_mtx         = {};
    uint32_t                              m_timeoutMs   = 0;
    std::thread                           m_initThread;
};

struct NPU_PACKET_PARAM
{
    NpuWLContext *context = nullptr;
};

class NpuCmdPackageSpecific : public CmdPackage
{
public:
    NpuCmdPackageSpecific(L0NpuInterface *npuInterface, NpuWLContext *npuWLContext) : m_npuInterface(npuInterface), m_npuWLContext(npuWLContext) {};
    virtual MOS_STATUS                  Wait() override;
    virtual MOS_STATUS                  Submit() override;
    virtual std::unique_ptr<CmdPackage> Clone() const override;
    virtual bool                        IsAsyncExecute() override { return true; };

private:
    L0NpuInterface *m_npuInterface = nullptr;
    NpuWLContext   *m_npuWLContext = nullptr;
};

class NpuCmdPacket : virtual public CmdPacket
{
public:
    NpuCmdPacket(MediaTask *task, PMOS_INTERFACE osInterface);

    virtual ~NpuCmdPacket();

    MOS_STATUS Init() override;

    MOS_STATUS Destroy() override;

    MOS_STATUS Prepare() override;

    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override; // commandBuffer is no need for npu packet

    MOS_STATUS SetGraphParam(NPU_PACKET_PARAM &npuParam);

protected:
    NPU_PACKET_PARAM m_npuParam     = {};
    L0NpuInterface  *m_npuInterface = nullptr;

MEDIA_CLASS_DEFINE_END(NpuCmdPacket)
};
#endif
