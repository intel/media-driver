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
//! \file     media_cmd_task.cpp
//! \brief    Defines the interface for media cmd task
//! \details  The media cmd task is dedicated for command buffer submission
//!
#include "media_cmd_task.h"
#include "media_packet.h"
#include "media_utils.h"

CmdTask::CmdTask(PMOS_INTERFACE osInterface)
    : m_osInterface(osInterface)
{

}

MOS_STATUS CmdTask::CalculateCmdBufferSizeFromActivePackets()
{
    uint32_t curCommandBufferSize      = 0;
    uint32_t curRequestedPatchListSize = 0;

    m_cmdBufSize    = 0;
    m_patchListSize = 0;
    for (auto prop : m_packets)
    {
        // Calculate total size based on pipe 0
        if (prop.stateProperty.currentPipe == 0)
        {
            auto packetId = prop.packetId;
            auto packet   = prop.packet;

            MEDIA_CHK_NULL_RETURN(packet);

            curCommandBufferSize      = 0;
            curRequestedPatchListSize = 0;

            packet->CalculateCommandSize(curCommandBufferSize, curRequestedPatchListSize);
            m_cmdBufSize += curCommandBufferSize;
            m_patchListSize += curRequestedPatchListSize;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmdTask::Submit(bool immediateSubmit, MediaScalability *scalability, CodechalDebugInterface *debugInterface)
{
    MEDIA_CHK_NULL_RETURN(scalability);

    // Algin this variable in pipeline, packet and scalability.
    bool singleTaskPhaseSupportedInPak = false;
    MEDIA_CHK_STATUS_RETURN(CalculateCmdBufferSizeFromActivePackets());

    // prepare cmd buffer
    MOS_COMMAND_BUFFER cmdBuffer;
    // initialize the command buffer struct
    MOS_ZeroMemory(&cmdBuffer, sizeof(MOS_COMMAND_BUFFER));

    if (m_packets.size() > 0)
    {
        MEDIA_CHK_STATUS_RETURN(scalability->UpdateState(&m_packets[0].stateProperty));

        // VerifyCmdBuffer could be called for duplicated times for singleTaskPhase mult-pass cases
        // Each task submit verify only once
        MEDIA_CHK_STATUS_RETURN(scalability->VerifyCmdBuffer(m_cmdBufSize, m_patchListSize, singleTaskPhaseSupportedInPak));
    }
    else
    {
        SCALABILITY_ASSERTMESSAGE("No packets to execute in the task!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (auto& prop : m_packets)
    {
        MEDIA_CHK_STATUS_RETURN(scalability->UpdateState(&prop.stateProperty));

        auto packet = prop.packet;
        uint8_t packetPhase = MediaPacket::otherPacket;
        MEDIA_CHK_NULL_RETURN(packet);

        MEDIA_CHK_STATUS_RETURN(packet->Prepare());

        MEDIA_CHK_STATUS_RETURN(scalability->GetCmdBuffer(&cmdBuffer));

        if (&prop == &m_packets.front())
        {
            packetPhase = MediaPacket::firstPacket;
        }
        else if (&prop == &m_packets.back())
        {
            packetPhase = MediaPacket::lastPacket;
        }

        MEDIA_CHK_STATUS_RETURN(packet->Submit(&cmdBuffer, packetPhase));

        MEDIA_CHK_STATUS_RETURN(scalability->ReturnCmdBuffer(&cmdBuffer));
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MEDIA_CHK_STATUS_RETURN(DumpCmdBufferAllPipes(&cmdBuffer, debugInterface, scalability));
#endif  // _DEBUG || _RELEASE_INTERNAL

    // submit cmd buffer
    MEDIA_CHK_STATUS_RETURN(scalability->SubmitCmdBuffer(&cmdBuffer));

#if (_DEBUG || _RELEASE_INTERNAL)
    for (auto prop : m_packets)
    {
        MEDIA_CHK_STATUS_RETURN(scalability->UpdateState(&prop.stateProperty));

        auto packet = prop.packet;
        MEDIA_CHK_NULL_RETURN(packet);
        MEDIA_CHK_STATUS_RETURN(packet->DumpOutput());
    }
#endif // _DEBUG || _RELEASE_INTERNAL

    // clear the packet lists since all commands are composed into command buffer
    m_packets.clear();

    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CmdTask::DumpCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, CodechalDebugInterface *debugInterface, uint8_t pipeIdx)
{
    MEDIA_CHK_NULL_RETURN(cmdBuffer);

    if (debugInterface)
    {
        std::string packetName = "";
        for (auto prop : m_packets)
        {
            // Construct cmd buffer dump name only from packets in pipe 0
            if (prop.stateProperty.currentPipe == 0)
            {
                packetName += prop.packet->GetPacketName();
            }
        }

        // Put pipe index to file name
        std::stringstream pipeIdxStrStream;
        pipeIdxStrStream << "_" << pipeIdx;
        packetName += pipeIdxStrStream.str();

        MEDIA_CHK_STATUS_RETURN(debugInterface->DumpCmdBuffer(
            cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            packetName.data()));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CmdTask::DumpCmdBufferAllPipes(PMOS_COMMAND_BUFFER cmdBuffer, CodechalDebugInterface *debugInterface, MediaScalability *scalability)
{
    MEDIA_CHK_NULL_RETURN(cmdBuffer);
    MEDIA_CHK_NULL_RETURN(scalability);

    if (debugInterface)
    {
        auto pipeNum = scalability->GetPipeNumber();
        auto lastPipeIndex = scalability->GetCurrentPipe();
        for (uint8_t pipeIdx = 0; pipeIdx < pipeNum; pipeIdx++)
        {
            scalability->SetCurrentPipeIndex(pipeIdx);
            MEDIA_CHK_STATUS_RETURN(scalability->GetCmdBuffer(cmdBuffer));
            MEDIA_CHK_STATUS_RETURN(DumpCmdBuffer(cmdBuffer, debugInterface));
            MEDIA_CHK_STATUS_RETURN(scalability->ReturnCmdBuffer(cmdBuffer));
        }

        // Recover scalability current pipe index after dump
        scalability->SetCurrentPipeIndex(lastPipeIndex);
    }

    return MOS_STATUS_SUCCESS;
}
#endif // _DEBUG || _RELEASE_INTERNAL