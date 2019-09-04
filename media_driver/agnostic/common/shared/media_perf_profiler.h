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
//! \file     media_perf_profiler.h
//! \brief    Defines data structures and interfaces for media performance profiler.

#ifndef __MEDIA_PERF_PROFILER_H__
#define __MEDIA_PERF_PROFILER_H__

#include <map>
#include "mos_os.h"
#include "mhw_mi.h"

using Map = std::map<void*, uint32_t>;

/*! \brief In order to align GPU node value for all of OS,
*   we redifine the GPU node value here.
*/
typedef enum _PerfGPUNode
{
    PERF_GPU_NODE_3D     = 0,
    PERF_GPU_NODE_VIDEO  = 1,
    PERF_GPU_NODE_BLT    = 2,
    PERF_GPU_NODE_VE     = 3,
    PERF_GPU_NODE_VIDEO2 = 4,
    PERF_GPU_NODE_UNKNOW = 0xFF
}PerfGPUNode;

class MediaPerfProfiler
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
    //! \brief    Initialize profiler and allcoate resources
    //!
    //! \param    [in] context
    //!           Pointer of Codechal/VPHal
    //! \param    [in] osInterface
    //!           Pointer of OS interface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(void* context, MOS_INTERFACE *osInterface);

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
    MOS_STATUS AddPerfCollectStartCmd(void* context,
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
    MOS_STATUS AddPerfCollectEndCmd(void* context,
        MOS_INTERFACE *osInterface,
        MhwMiInterface *miInterface,
        MOS_COMMAND_BUFFER *cmdBuffer);

private:
    //!
    //! \brief    Constructor
    //!
    MediaPerfProfiler();

    //!
    //! \brief    Deconstructor
    //!
    virtual ~MediaPerfProfiler();

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
    MOS_STATUS StoreData(MhwMiInterface *miInterface, 
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset,
                         uint32_t value);

    //!
    //! \brief    Save register value to the buffer which store the performance data 
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
    MOS_STATUS StoreRegister(MhwMiInterface *miInterface, 
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
    MOS_STATUS StoreTSByPipeCtrl(MhwMiInterface *miInterface,
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
    MOS_STATUS StoreTSByMiFlush(MhwMiInterface *miInterface,
                         PMOS_COMMAND_BUFFER cmdBuffer,
                         uint32_t offset);

    //!
    //! \brief    Save performance data in to a file 
    //!
    //! \param    [in] osInterface
    //!           Pointer of OS interface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SavePerfData(MOS_INTERFACE *osInterface);

    //!
    //! \brief    Convert GPU context to GPU node 
    //!
    //! \param    [in] context
    //!           GPU context
    //!
    //! \return   PerfGPUNode
    //!
    PerfGPUNode GpuContextToGpuNode(MOS_GPU_CONTEXT context);

    //!
    //! \brief    Check the performance mode 
    //!
    //! \param    [in] regs
    //!           The registers' array
    //!
    //! \return   bool
    //!           true if include the memory information
    //!
    bool IsPerfModeWidthMemInfo(uint32_t *regs);

protected:
    MOS_RESOURCE               m_perfStoreBuffer;       //!< Buffer for perf data collection
    Map                        m_contextIndexMap;       //!< Map between CodecHal/VPHal and PerfDataContext
    PMOS_MUTEX                 m_mutex = nullptr;       //!< Mutex for protecting data of profiler when refereced multi times

    int32_t                    m_profilerEnabled = 0;   //!< UMD Perf Profiler enable or not
    uint32_t                   m_perfDataIndex = 0;     //!< The index of performance data node in buffer
    uint32_t                   m_ref = 0;               //!< The number of refereces
    uint32_t                   m_bufferSize = 10000000; //!< The size of perf data buffer
    uint32_t                   m_timerBase  = 0;        //!< time frequency
    int32_t                    m_multiprocess = 0;      //!< multi process support
    uint32_t                   m_registers[8] = { 0 };  //!< registers of Memory information

    bool                       m_initialized = false;   //!< Indicate whether profiler was initialized
    char                       m_outputFileName[MOS_MAX_PATH_LENGTH + 1];  //!< Name of output file
};

#endif // __MEDIA_PERF_PROFILER_H__
