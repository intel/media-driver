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
//! \file     media_scalability_mdf.cpp
//! \brief    Defines the common interface for media scalability
//! \details  The media scalability interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#include "media_scalability_mdf.h"

MOS_STATUS MediaScalabilityMdf::Initialize(const MediaScalabilityOption &option)
{
    uint32_t devOp = CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE;
    //create CM device
    if (!m_cmDev)
    {
        m_osInterface->pfnNotifyStreamIndexSharing(m_osInterface);
        SCALABILITY_CHK_STATUS_RETURN(CreateCmDevice(
            m_osInterface->pOsContext,
            m_cmDev,
            devOp));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilityMdf::Destroy()
{

    if (m_cmDev)
    {
        DestroyCmDevice(m_cmDev);
        m_cmDev = nullptr;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaScalabilityMdf::GetQueue(bool newQueue, CmQueue* & queue)
{
    //create CM Queue
    if (newQueue || m_cmQueueList.empty())
    {
        CM_QUEUE_CREATE_OPTION queueCreateOption = CM_DEFAULT_QUEUE_CREATE_OPTION;
        if (m_computeContextEnabled)
        {
            queueCreateOption.QueueType = CM_QUEUE_TYPE_COMPUTE;
        }
        SCALABILITY_CHK_STATUS_RETURN(m_cmDev->CreateQueueEx(m_cmQueue, queueCreateOption));
        m_cmQueueList.push_back(m_cmQueue);
    }

    SCALABILITY_CHK_NULL_RETURN(m_cmQueue);
    queue = m_cmQueue;

    return MOS_STATUS_SUCCESS;
}

