/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_commandbuffer_specific_next.h
//! \brief    Container class for the linux/Android specfic comamnd buffer
//!

#ifndef __COMMAND_BUFFER_SPECIFIC_NEXT_H__
#define __COMMAND_BUFFER_SPECIFIC_NEXT_H__

#include "mos_commandbuffer_next.h"

//!
//! \class  CommandBufferSpecific
//! \brief  Linux/Android specific command buffer 
//!
class CommandBufferSpecificNext : public CommandBufferNext
{

public:
    //!
    //! \brief  Constructor
    //!
    CommandBufferSpecificNext() {}

    //!
    //! \brief  Destructor
    //!
    ~CommandBufferSpecificNext() {}

    MOS_STATUS  Allocate(OsContextNext *osContext, uint32_t size);

    void  Free();

    MOS_STATUS BindToGpuContext(GpuContextNext *gpuContext);

    void UnBindToGpuContext();

    MOS_STATUS ReSize(uint32_t newSize);

    //!
    //! \brief    query command buffer status
    //! \detail   This function will call mos_bo_busy()
    //! \return   int
    //!           >0 if it's busy, otherwise not busy
    //!
    int isBusy();

    //!
    //! \brief    wait command buffer to be rendered by gpu
    //! \detail   This function will call mos_bo_wait_rendering()
    //!
    void waitReady();
};
#endif // __COMMAND_BUFFER_SPECIFIC_NEXT_H__
