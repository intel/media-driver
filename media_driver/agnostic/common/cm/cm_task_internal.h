/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_task_internal.h 
//! \brief     Contains Class CmTaskInternal  definitions 
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKINTERNAL_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKINTERNAL_H_

#include "cm_def.h"
#include "cm_array.h"
#include "cm_event.h"
#include "cm_hal.h"
#include "cm_hal_vebox.h"
#include "cm_log.h"

enum CM_INTERNAL_TASK_TYPE
{
    CM_INTERNAL_TASK_WITH_THREADSPACE,
    CM_INTERNAL_TASK_WITH_THREADGROUPSPACE,
    CM_INTERNAL_TASK_VEBOX,
    CM_INTERNAL_TASK_ENQUEUEWITHHINTS
};

#define CM_TASK_TYPE_DEFAULT CM_INTERNAL_TASK_WITH_THREADSPACE

//*-----------------------------------------------------------------------------
//| CM Task Profiling Information
//*-----------------------------------------------------------------------------
struct CM_PROFILING_INFO
{
    uint32_t taskID;
    uint32_t threadID;
    uint32_t kernelCount;
    uint32_t kernelNameLen;
    char     *kernelNames;
    uint32_t *localWorkWidth;
    uint32_t *localWorkHeight;
    uint32_t *globalWorkWidth;
    uint32_t *globalWorkHeight;

    LARGE_INTEGER enqueueTime;
    LARGE_INTEGER flushTime;
    LARGE_INTEGER hwStartTime;
    LARGE_INTEGER hwEndTime;
    LARGE_INTEGER completeTime;
};

namespace CMRT_UMD
{
class CmKernelRT;
class CmEventRT;
class CmKernelData;
class CmThreadSpaceRT;
class CmThreadGroupSpace;
class CmDeviceRT;
class CmVeboxRT;
class CmVeboxData;
class CmBufferUP;

class CmTaskInternal: public CmDynamicArray
{
public:

    static int32_t Create( const uint32_t kernelCount, const uint32_t totalThreadCount,
                           CmKernelRT* kernelArray[], const CmThreadSpaceRT* threadSpace,
                           CmDeviceRT* device, const uint64_t syncBitmap, CmTaskInternal*& task,
                           const uint64_t conditionalEndBitmap,
                           PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo );
    static int32_t Destroy( CmTaskInternal* &task );
    static int32_t Create( const uint32_t kernelCount, const uint32_t totalThreadCount,
                           CmKernelRT* kernelArray[], const CmThreadGroupSpace* threadGroupSpace,
                           CmDeviceRT* device, const uint64_t syncBitmap, CmTaskInternal*& task,
                           const uint64_t conditionalEndBitmap,
                           PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo,
                           const CM_EXECUTION_CONFIG* krnExecCfg);
    static int32_t Create( CmDeviceRT* device, CmVeboxRT* vebox, CmTaskInternal*& task );
    static int32_t Create( const uint32_t kernelCount, const uint32_t totalThreadCount,
                           CmKernelRT* kernelArray[], CmTaskInternal*& task,
                           uint32_t numGeneratedTasks, bool isLastTask, uint32_t hints,
                           CmDeviceRT* device);

    virtual int32_t GetKernelCount( uint32_t& count );
    virtual int32_t GetKernel( const uint32_t index, CmKernelRT* & kernel );
    virtual int32_t GetKernelData( const uint32_t index, CmKernelData* & kernelData );
    int32_t GetKernelDataSize( const uint32_t index, uint32_t & size );
    uint32_t GetKernelCurbeOffset( const uint32_t index );
    int32_t GetTotalThreadCount( uint32_t& totalThreadCount );

    int32_t SetTaskEvent( CmEventRT* event );
    int32_t GetTaskEvent( CmEventRT* & event );

    int32_t CreateThreadSpaceData(const CmThreadSpaceRT* threadSpace);
    int32_t GetKernelCoordinates(const uint32_t index, void  *&kernelCoordinates);
    int32_t GetKernelDependencyMasks(const uint32_t index, void  *&kernelDependencyMasks);
    int32_t GetDependencyPattern(CM_DEPENDENCY_PATTERN &dependencyPattern);
    int32_t GetWalkingPattern(CM_WALKING_PATTERN &walkingPattern);
    int32_t GetWalkingParameters(CM_WALKING_PARAMETERS &walkingParameters);
    int32_t GetDependencyVectors(CM_HAL_DEPENDENCY &dependencyVector);
    bool CheckWalkingParametersSet();
    bool CheckDependencyVectorsSet();
#if _DEBUG
    int32_t DisplayThreadSpaceData(uint32_t width, uint32_t height);
#endif
    int32_t GetThreadSpaceSize(uint32_t& width, uint32_t& height );
    int32_t GetThreadGroupSpaceSize(uint32_t& threadSpaceWidth, uint32_t& threadSpaceHeight,
                                    uint32_t& threadSpaceDepth, uint32_t& groupSpaceWidth,
                                    uint32_t& groupSpaceHeight, uint32_t& groupSpaceDepth);
    int32_t GetSLMSize(uint32_t& slmSize);
    int32_t GetSpillMemUsed(uint32_t& spillMemUsed);
    int32_t GetColorCountMinusOne(uint32_t& colorCount);
    int32_t GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT& groupSelect);
    int32_t GetHints(uint32_t& hints);
    int32_t GetNumTasksGenerated(uint32_t& numTasksGenerated);
    int32_t GetLastTask(bool& isLastTask);

    bool IsThreadGroupSpaceCreated(void);
    bool IsThreadSpaceCreated(void );
    bool IsThreadCoordinatesExisted(void);

    int32_t AllocateKernelSurfInfo();
    int32_t GetKernelSurfInfo(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS & surfEntryInfoArray);
    int32_t ClearKernelSurfInfo();
    int32_t ResetKernelDataStatus();

    int32_t GetTaskType(uint32_t& taskType);

    int32_t GetVeboxState(CM_VEBOX_STATE & veboxState);
    int32_t GetVeboxParam(CmBufferUP * &veboxParam);
    int32_t GetVeboxSurfaceData(CM_VEBOX_SURFACE_DATA &veboxSurfaceData);

    int32_t GetTaskSurfaces( bool  *&surfArray );

    uint64_t GetSyncBitmap();
    uint64_t GetConditionalEndBitmap();
    CM_HAL_CONDITIONAL_BB_END_INFO* GetConditionalEndInfo();

    int32_t SetPowerOption( PCM_POWER_OPTION powerOption );
    PCM_POWER_OPTION GetPowerOption();
    int32_t GetTaskStatus(CM_STATUS & taskStatus);
    int32_t SetProperty(CM_TASK_CONFIG * taskConfig);
    int32_t GetProperty(CM_TASK_CONFIG &taskConfig);
    const CM_EXECUTION_CONFIG* GetKernelExecuteConfig() { return m_krnExecCfg; };
    void  *GetMediaStatePtr();
#if CM_LOG_ON
    std::string Log();

    CM_HAL_STATE *GetHalState();
#endif

    void SurfaceDump(int32_t taskId);

    int32_t VtuneInitProfilingInfo(const CmThreadGroupSpace *perTaskThreadGroupSpace);
    int32_t VtuneInitProfilingInfo(const CmThreadSpaceRT *perTaskThreadSpace);
    int32_t VtuneSetFlushTime();

protected:

    CmTaskInternal(const uint32_t kernelCount, const uint32_t totalThreadCount,
                   CmKernelRT* kernelArray[], CmDeviceRT* device, const uint64_t syncBitmap,
                   const uint64_t conditionalEndBitmap,
                   PCM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo,
                   const CM_EXECUTION_CONFIG* krnExecCfg);
    ~CmTaskInternal( void );

    int32_t Initialize(const CmThreadSpaceRT* threadSpace, bool isWithHints);
    int32_t Initialize(const CmThreadGroupSpace* threadGroupSpace);

    int32_t Initialize(CmVeboxRT* vebox);
    int32_t Initialize(uint32_t hints, uint32_t numTasksGenerated, bool isLastTask);

    int32_t VtuneWriteEventInfo();
    int32_t VtuneReleaseProfilingInfo();

    int32_t UpdateSurfaceStateOnTaskCreation();

    CmDynamicArray m_kernels;
    CmDynamicArray m_kernelData; // one CmKernelData* per kernel
    uint32_t *m_kernelCurbeOffsetArray; // Array to record each kernel's curbe offset. Even the same kernel could have different curbe offset.
    uint32_t m_kernelCount;

    uint32_t m_totalThreadCount;    //total thread count per task

    CmEventRT* m_taskEvent;

    //ThreadSpaceInformation from CreateThreadSpace() or CreateThreadGroupSpace()
    bool            m_isThreadSpaceCreated;
    bool            m_isThreadCoordinatesExisted;
    uint32_t        m_threadSpaceWidth;
    uint32_t        m_threadSpaceHeight;
    uint32_t        m_threadSpaceDepth;

    PCM_HAL_SCOREBOARD *m_threadCoordinates;
    CM_DEPENDENCY_PATTERN m_dependencyPattern;
    CM_WALKING_PATTERN m_walkingPattern;
    uint32_t m_walkingParameters[CM_NUM_DWORD_FOR_MW_PARAM];
    bool m_mediaWalkerParamsSet;
    CM_HAL_DEPENDENCY m_dependencyVectors;
    bool m_dependencyVectorsSet;
    PCM_HAL_MASK_AND_RESET    *m_dependencyMasks;
    CM_MW_GROUP_SELECT         m_mediaWalkerGroupSelect;

    bool            m_isThreadGroupSpaceCreated;
    uint32_t        m_groupSpaceWidth;
    uint32_t        m_groupSpaceHeight;
    uint32_t        m_groupSpaceDepth;
    uint32_t        m_slmSize;      //SLM size per thread group in 1KB unit
    uint32_t        m_spillMemUsed;

    uint32_t        m_colorCountMinusOne;
    uint32_t        m_hints;
    uint32_t        m_numTasksGenerated;
    bool            m_isLastTask;

    uint64_t        m_ui64SyncBitmap;
    uint64_t        m_ui64ConditionalEndBitmap;
    CM_HAL_CONDITIONAL_BB_END_INFO m_conditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];

    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS m_kernelSurfInfo;
    CmDeviceRT*                      m_cmDevice;
    bool                             *m_surfaceArray;  // vector-flag of surfaces R/W by this CM Task (containing multi-kernel)
    bool                             m_isSurfaceUpdateDone;

    uint32_t        m_taskType; //0 - Task with thread space, 1 - Task with thread group space, 2 - Task for VEBOX

    CmBufferUP   *  m_veboxParam;
    CM_VEBOX_STATE  m_veboxState;
    CM_VEBOX_SURFACE_DATA m_veboxSurfaceData;

    CM_POWER_OPTION m_powerOption;
    CM_PROFILING_INFO   m_taskProfilingInfo;
    CM_TASK_CONFIG  m_taskConfig;
    CM_EXECUTION_CONFIG m_krnExecCfg[CM_MAX_KERNELS_PER_TASK];
    void            *m_mediaStatePtr;
private:
    CmTaskInternal (const CmTaskInternal& other);
    CmTaskInternal& operator= (const CmTaskInternal& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKINTERNAL_H_
