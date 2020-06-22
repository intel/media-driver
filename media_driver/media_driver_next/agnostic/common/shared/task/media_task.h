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
//! \file     media_task.h
//! \brief    Defines the common interface for media task
//! \details  The media task is further sub-divided into mdf and cmd type
//!           this file is for the base interface which is shared by all task.
//!

#ifndef __MEDIA_TASK_H__
#define __MEDIA_TASK_H__
#include <vector>
#include "mos_defs.h"
#include "media_scalability.h"
#include "codechal_debug.h"

class CodechalDebugInterface;
class MediaPacket;
struct PacketProperty
{
    MediaPacket       *packet = nullptr;
    uint32_t           packetId = 0;
    bool               immediateSubmit = false;

    StateParams        stateProperty;
};

class MediaTask
{
public:
    virtual ~MediaTask() {}
    //!
    //! \brief  Add media packets into current task for further execution
    //! \param  [in] packet
    //!         Pointer to the media packet and its property to add
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPacket(PacketProperty *packet);

    //!
    //! \brief  Submit all the packets in the pool for execution
    //! \param  [in] immediateSubmit
    //!         Submit the command buffer is true, otherwise only build the command sequence and 
    //!         reserve for future sumission
    //! \param  [in] scalability
    //!         Media scalability state instance for task submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(bool immediateSubmit, MediaScalability *scalability, CodechalDebugInterface *debugInterface) = 0;

    //!
    //! \brief  Clear the packets in the pool, this method must be invoked
    //!         after Submit() or when user want to explicitly clears the pool
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Clear();

    virtual void SetupCmdBufSize(uint32_t cmdBufSize, uint32_t patchListSize)
    { 
        m_cmdBufSize = cmdBufSize;
        m_patchListSize = patchListSize;
    }

    enum class TaskType
    {
        cmdTask = 1,
        mdfTask = 2,
    };

protected:
    std::vector<PacketProperty> m_packets;            //!< media packets pool for execution
    uint32_t                          m_cmdBufSize = 0;     //!< Cmd buffer size for execution
    uint32_t                          m_patchListSize = 0;  //!< Patch list size for execution
};

#endif // !__MEDIA_TASK_H__
