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
//! \file     media_cmd_task.h
//! \brief    Defines the interface for media cmd task
//! \details  The media cmd task is dedicated for command buffer submission
//!
#ifndef __MEDIA_CMD_TASK_H__
#define __MEDIA_CMD_TASK_H__
#include "media_task.h"
#include "mos_os.h"
#include "codechal_debug.h"

class CmdTask : public MediaTask
{
public:
    //!
    //! \brief  CmdTask constructor
    //! \param  [in] osInterface
    //!         Pointer to MOS_INTERFACE
    //!
    CmdTask(PMOS_INTERFACE osInterface);

    virtual ~CmdTask() { }

    virtual MOS_STATUS Submit(bool immediateSubmit, MediaScalability *scalability, CodechalDebugInterface *debugInterface) override;

protected:
#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS DumpCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, CodechalDebugInterface *debugInterface, uint8_t pipeIdx = 0);
    virtual MOS_STATUS DumpCmdBufferAllPipes(PMOS_COMMAND_BUFFER cmdBuffer, CodechalDebugInterface *debugInterface, MediaScalability *scalability);
#endif // _DEBUG || _RELEASE_INTERNAL

    //! \brief  Calculate Command Size for all packets in packets list
    //!
    //! \return uint32_t
    //!         Command size calculated
    //!
    MOS_STATUS CalculateCmdBufferSizeFromActivePackets();

    PMOS_INTERFACE m_osInterface = nullptr;        //!< PMOS_INTERFACE

};



#endif // !__MEDIA_CMD_TASK_H__
