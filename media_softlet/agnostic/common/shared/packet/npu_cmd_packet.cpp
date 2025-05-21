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
//!
//! \file     npu_cmd_packet.cpp
//! \brief    render packet which used in by mediapipline.
//! \details  render packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "npu_cmd_packet.h"
#include "levelzero_npu_interface.h"
#include "media_utils.h"

#define NPU_CHK_NULL_RETURN(_ptr) \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_HW, MOS_CODEC_SUBCOMP_PUBLIC, _ptr)

#define NPU_CHK_STATUS_RETURN(_stmt) \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_HW, MOS_CODEC_SUBCOMP_PUBLIC, _stmt)

NpuCmdPacket::NpuCmdPacket(MediaTask *task, PMOS_INTERFACE osInterface) : CmdPacket(task)
{
    m_osInterface = osInterface;
    if (osInterface)
    {
        m_npuInterface = osInterface->npuInterface;
    }
}

NpuCmdPacket::~NpuCmdPacket()
{

}

MOS_STATUS NpuCmdPacket::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    //commandBuffer is no need for npu packet, so it could be nullptr
    NPU_CHK_NULL_RETURN(m_npuInterface);
    NPU_CHK_NULL_RETURN(m_npuParam.context);
    NPU_CHK_NULL_RETURN(m_osInterface);
    NPU_CHK_NULL_RETURN(m_osInterface->pfnSubmitPackage);

    NpuCmdPackageSpecific cmdPackage(m_npuInterface, m_npuParam.context);

    NPU_CHK_STATUS_RETURN(m_osInterface->pfnSubmitPackage(m_osInterface, cmdPackage));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS NpuCmdPacket::Init()
{
    NPU_CHK_NULL_RETURN(m_npuInterface);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS NpuCmdPacket::Prepare()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS NpuCmdPacket::SetGraphParam(NPU_PACKET_PARAM &npuParam)
{
    m_npuParam = npuParam;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS NpuCmdPacket::Destroy()
{
    NPU_CHK_STATUS_RETURN(CmdPacket::Destroy());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS NpuCmdPackageSpecific::Wait()
{
    NPU_CHK_NULL_RETURN(m_npuWLContext);
    NPU_CHK_NULL_RETURN(m_npuWLContext->Fence());
    NPU_CHK_NULL_RETURN(m_npuInterface);
    NPU_CHK_NULL_RETURN(m_npuInterface->pfnWaitFence)

    NPU_CHK_STATUS_RETURN(m_npuInterface->pfnWaitFence(m_npuInterface, m_npuWLContext->Fence()));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS NpuCmdPackageSpecific::Submit()
{
    NPU_CHK_NULL_RETURN(m_npuInterface);
    NPU_CHK_NULL_RETURN(m_npuWLContext);
    NPU_CHK_NULL_RETURN(m_npuInterface->pfnExecute);

    std::unique_lock<std::mutex> lock(m_npuWLContext->Mutex());
    if (m_npuWLContext->Condition().wait_for(lock, std::chrono::milliseconds(m_npuWLContext->GetTimeoutMs()), [=] { return m_npuWLContext->Initialized() || m_npuWLContext->Failed(); }))
    {
        if (m_npuWLContext->Failed())
        {
            MEDIA_ASSERTMESSAGE("Graph Package Initialize Failed, Cannot Continue to Submit Graph List to Queue");
            return MOS_STATUS_UNINITIALIZED;
        }
    }
    else
    {
        MEDIA_ASSERTMESSAGE("Timeout %d ms reached, Graph Package still not finish Initialization", m_npuWLContext->GetTimeoutMs());
        return MOS_STATUS_UNINITIALIZED;
    }
    NPU_CHK_STATUS_RETURN(m_npuInterface->pfnExecute(m_npuInterface, m_npuWLContext->CmdList(), m_npuWLContext->Fence()));

    return MOS_STATUS_SUCCESS;
}

std::unique_ptr<CmdPackage> NpuCmdPackageSpecific::Clone() const
{
    return std::make_unique<NpuCmdPackageSpecific>(*this);
}