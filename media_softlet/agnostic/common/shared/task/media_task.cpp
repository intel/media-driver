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
//! \file     media_task.cpp
//! \brief    Defines the common interface for media task
//! \details  The media task is further sub-divided into mdf and cmd type
//!           this file is for the base interface which is shared by all task.
//!

#include "media_task.h"

MOS_STATUS MediaTask::AddPacket(PacketProperty *packet)
{
    if (nullptr != packet)
    {
        m_packets.push_back(*packet);
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        return MOS_STATUS_NULL_POINTER;
    }
}

MOS_STATUS MediaTask::Clear()
{
    m_packets.clear();
    return MOS_STATUS_SUCCESS;
}