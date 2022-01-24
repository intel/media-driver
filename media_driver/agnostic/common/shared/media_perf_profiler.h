/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     media_perf_profiler.h
//! \brief    Defines data structures and interfaces for media performance profiler.

#ifndef __MEDIA_PERF_PROFILER_H__
#define __MEDIA_PERF_PROFILER_H__

#include "media_perf_profiler_next.h"

class MediaPerfProfiler : public MediaPerfProfilerNext
{
public:

    //!
    //! \brief    Get the instance of the profiler
    //!
    //! \return   pointer of profiler
    //!
    static MediaPerfProfiler *Instance();

    //!
    //! \brief    Destroy the resurces of profiler
    //!
    //! \param    [in] profiler
    //!           Pointer of profiler
    //! \param    [in] context
    //!           Pointer of Codechal/VPHal
    //! \param    [in] osInterface
    //!           Pointer of OS interface
    //!
    //! \return   void
    //!
    static void Destroy(MediaPerfProfiler* profiler, void* context, MOS_INTERFACE *osInterface);

    //!
    //! \brief    Constructor
    //!
     MediaPerfProfiler() {}

    //!
    //! \brief    Deconstructor
    //!
    ~MediaPerfProfiler(){}

    //!
    //! \brief    Save data to the buffer which store the performance data 
    //!
    //! \param    [in] miInterface
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //! \param    [in] value       
    //!           Value of data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StoreData(MhwMiInterface *miInterface,
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset,
                         uint32_t value);

    //!
    //! \brief    Save data to the buffer which store the performance data 
    //!
    //! \param    [in] miInterface
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //! \param    [in] value       
    //!           Value of data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreDataNext(MhwMiInterface *miInterface, 
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset,
                         uint32_t value);

    //!
    //! \brief    Save register value to the buffer which store the performance data 
    //! \param    [in] osInterface
    //!           Pointer of MOS_INTERFACE
    //!
    //! \param    [in] miInterface
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //! \param    [in] reg
    //!           Address of register
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StoreRegister(
                        MOS_INTERFACE *osInterface,
                        MhwMiInterface *miInterface,
                        PMOS_COMMAND_BUFFER cmdBuffer,
                        uint32_t offset,
                        uint32_t reg);

    //!
    //! \brief    Save register value to the buffer which store the performance data 
    //! \param    [in] osInterface
    //!           Pointer of MOS_INTERFACE
    //!
    //! \param    [in] miInterface
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //! \param    [in] reg
    //!           Address of register
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreRegisterNext(
                        MOS_INTERFACE *osInterface,
                        MhwMiInterface *miInterface,
                        PMOS_COMMAND_BUFFER cmdBuffer,
                        uint32_t offset,
                        uint32_t reg);

    //!
    //! \brief    Save timestamp to the buffer by Pipe control command 
    //!
    //! \param    [in] miInterface 
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StoreTSByPipeCtrl(MhwMiInterface *miInterface,
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset);

    //!
    //! \brief    Save timestamp to the buffer by Pipe control command 
    //!
    //! \param    [in] miInterface 
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreTSByPipeCtrlNext(MhwMiInterface *miInterface,
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset);

    //!
    //! \brief    Save timestamp to the buffer by MI command 
    //!
    //! \param    [in] miInterface 
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StoreTSByMiFlush(MhwMiInterface *miInterface,
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset);

    //!
    //! \brief    Save timestamp to the buffer by MI command 
    //!
    //! \param    [in] miInterface 
    //!           Pointer of MI interface
    //! \param    [in] cmdBuffer
    //!           Pointer of OS command buffer
    //! \param    [in] offset
    //!           Offset in the buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
        MOS_STATUS StoreTSByMiFlushNext(MhwMiInterface* miInterface,
            PMOS_COMMAND_BUFFER cmdBuffer,
            uint32_t offset);

     //!
     //! \brief    Insert start command of storing performance data
     //!
     //! \param    [in] context
     //!           Pointer of Codechal/VPHal
     //! \param    [in] osInterface
     //!           Pointer of OS interface
     //! \param    [in] miInterface
     //!           pointer of MI interface
     //! \param    [in] cmdBuffer
     //!           Pointer of OS command buffer
     //!
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
     virtual MOS_STATUS AddPerfCollectStartCmd(void* context,
         MOS_INTERFACE *osInterface,
         MhwMiInterface *miInterface,
         MOS_COMMAND_BUFFER *cmdBuffer);

     //!
     //! \brief    Insert end command of storing performance data
     //!
    //! \param    [in] context
     //!           Pointer of Codechal/VPHal
     //! \param    [in] osInterface
     //!           Pointer of OS interface
     //! \param    [in] miInterface
     //!           pointer of MI interface
     //! \param    [in] cmdBuffer
     //!           Pointer of OS command buffer
     //!
     //! \return   MOS_STATUS
     //!           MOS_STATUS_SUCCESS if success, else fail reason
     //!
     virtual MOS_STATUS AddPerfCollectEndCmd(void* context,
         MOS_INTERFACE *osInterface,
         MhwMiInterface *miInterface,
         MOS_COMMAND_BUFFER *cmdBuffer);

    //!
    //! \brief    Save performance data in to a file
    //!
    //! \param    [in] osInterface
    //!           Pointer of OS interface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SavePerfData(MOS_INTERFACE *osInterface) override;
};

#endif // __MEDIA_PERF_PROFILER_H__
