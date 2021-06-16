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
//! \file     media_mdf_task.cpp
//! \brief    Defines the interface for media mdf task
//! \details  The media mdf task is dedicated for mdf kernel submission
//!
#include "media_mdf_task.h"
#include "media_packet.h"
#include "media_utils.h"


#if USE_PROPRIETARY_CODE
#include "cm_device_rt.h"
#endif

MdfTask::MdfTask(CmDevice *device)
    : m_CmDevice(device)
{
    //create CM task
    if (m_CmDevice != nullptr)
    {
       m_CmDevice->CreateTask(m_cmTask);
    }
}

MOS_STATUS MdfTask::Submit(bool immediateSubmit, MediaScalability *scalability, CodechalDebugInterface *debugInterface)
{
    MEDIA_CHK_NULL_RETURN(scalability);
    MEDIA_CHK_NULL_RETURN(m_cmTask);

    CmQueue *queue = nullptr;
    CmKernel *kernel = nullptr;

    for (auto prop : m_packets)
    {

        MEDIA_CHK_STATUS_RETURN(scalability->GetQueue(false, queue));

        uint8_t phase = MediaPacket::PacketPhaseFlag::firstPacket | MediaPacket::PacketPhaseFlag::lastPacket;
        //MEDIA_CHK_STATUS_RETURN(prop.packet->Submit(m_cmDev, kernel, phase));
        MEDIA_CHK_STATUS_RETURN(m_cmTask->AddKernel(kernel));
        MEDIA_CHK_STATUS_RETURN(m_cmTask->AddSync());
    }

    if (immediateSubmit)
    {
        CmEvent *event = CM_NO_EVENT;
        MEDIA_CHK_NULL_RETURN(queue);
        MEDIA_CHK_STATUS_RETURN(queue->EnqueueWithGroup(m_cmTask, event/*, m_threadGroupSpace*/));
        MEDIA_CHK_STATUS_RETURN(m_cmTask->Reset());

#if (_DEBUG || _RELEASE_INTERNAL)
        for (auto prop : m_packets)
        {
            MEDIA_CHK_STATUS_RETURN(prop.packet->DumpOutput());
        }
#endif // _DEBUG || _RELEASE_INTERNAL

    }

    // clear the packet lists since all commands are composed into command buffer
    m_packets.clear();

    return MOS_STATUS_SUCCESS;
}

