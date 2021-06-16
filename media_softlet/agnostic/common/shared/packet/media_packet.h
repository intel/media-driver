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
//! \file     media_packet.h
//! \brief    Defines the common interface for media packet
//! \details  The media packet is further sub-divided into mdf and cmd type
//!           this file is for the base interface which is shared by all packet.
//!

#ifndef __MEDIA_PACKET_H__
#define __MEDIA_PACKET_H__
#include "mos_os.h"
#include "mhw_mi.h"
#include "media_status_report.h"

namespace CMRT_UMD
{
    class CmTask;
}
class MediaTask;
class MediaPacket
{
public:
    enum PacketPhaseFlag
    {
        firstPacket = 0x01,
        lastPacket  = 0x02,
        otherPacket = 0x04
    };

    virtual ~MediaPacket() { }
    //!
    //! \brief  MediaPacket constructor
    //! \param  [in] task
    //!         Pointer to MediaTask, it's assigned when create the packet,
    //!         the task is associated with current packet for future submit
    //!
    MediaPacket(MediaTask* task) : m_task(task) { }

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() = 0;

    //!
    //! \brief  Destroy the media packet and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() = 0;

    //!
    //! \brief  Prepare the parameters for command submission
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare() = 0;

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Add CmKernel into CmTask and return to the caller task
    //! \param  [in] cmTask
    //!         Pointer to the CmTask which is created by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(CMRT_UMD::CmTask* cmTask)
    {
        return MOS_STATUS_SUCCESS;
    }

    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return uint32_t
    //!         Command size calculated
    //!
    virtual MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Get current associated media task
    //! \return MediaTask*
    //!         return the media task pointer
    //!
    MediaTask* GetActiveTask()
    {
        return m_task;
    }


    //!
    //! \brief  Dump output resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpOutput()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName()
    {
        return "";
    }

protected:

    //!
    //! \brief  Start Status Report
    //! \param  [in] srType
    //!         status report type for send cmds
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StartStatusReport(
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer);

    virtual MOS_STATUS UpdateStatusReport(
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer);

    //!
    //! \brief  End Status Report
    //! \param  [in] srType
    //!         status report type for send cmds
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndStatusReport(
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer);

    //!
    //! \brief  Set the start tag in the command buffer
    //! \param  [in] osResource
    //!         reource used in the cmd
    //! \param  [in] offset
    //!         reource offset used the cmd
    //! \param  [in] srType
    //!         status report type
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetStartTag(
        MOS_RESOURCE *osResource,
        uint32_t offset,
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer);

    //!
    //! \brief  Set the end tag in the command buffer
    //! \param  [in] osResource
    //!         reource used in the cmd
    //! \param  [in] offset
    //!         reource offset used the cmd
    //! \param  [in] srType
    //!         status report type
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetEndTag(
        MOS_RESOURCE *osResource,
        uint32_t offset,
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer);

protected:
    MediaTask         *m_task         = nullptr;        //!< MediaTask associated with current packet
    MhwMiInterface    *m_miInterface  = nullptr;
    MediaStatusReport *m_statusReport = nullptr;
};

#endif // !__MEDIA_PACKET_H__
