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
    uint32_t dwTaskID;
    uint32_t dwThreadID;
    uint32_t dwKernelCount;
    uint32_t dwKernelNameLen;
    char     *pKernelNames;
    uint32_t *pLocalWorkWidth;
    uint32_t *pLocalWorkHeight;
    uint32_t *pGlobalWorkWidth;
    uint32_t *pGlobalWorkHeight;

    LARGE_INTEGER EnqueueTime;
    LARGE_INTEGER FlushTime;
    LARGE_INTEGER HwStartTime;
    LARGE_INTEGER HwEndTime;
    LARGE_INTEGER CompleteTime;
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

    static int32_t Create(const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], const CmThreadSpaceRT* pTS, CmDeviceRT* pCmDevice, const uint64_t uiSyncBitmap, CmTaskInternal*& pTask, const uint64_t uiConditionalendBitmap, PCM_HAL_CONDITIONAL_BB_END_INFO pConditionalEndInfo);
    static int32_t Destroy( CmTaskInternal* &pTask );
    static int32_t Create( const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], const CmThreadGroupSpace* pTGS, CmDeviceRT* pCmDevice, const uint64_t uiSyncBitmap, CmTaskInternal*& pTask);
    static int32_t Create( CmDeviceRT* pCmDevice, CmVeboxRT* pVebox, CmTaskInternal*& pTask );
    static int32_t Create( const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], CmTaskInternal*& pTask, uint32_t numTasksGenerated, bool isLastTask, uint32_t hints, CmDeviceRT* pCmDevice);

    int32_t GetKernelCount( uint32_t& count );
    int32_t GetKernel( const uint32_t index, CmKernelRT* & pKernel );
    int32_t GetKernelData( const uint32_t index, CmKernelData* & pKernelData ); 
    int32_t GetKernelDataSize( const uint32_t index, uint32_t & size );
    uint32_t GetKernelCurbeOffset( const uint32_t index );
    int32_t GetTotalThreadCount( uint32_t& totalThreadCount );
    
    int32_t SetTaskEvent( CmEventRT* pEvent );
    int32_t GetTaskEvent( CmEventRT* & pEvent );

    int32_t CreateThreadSpaceData(const CmThreadSpaceRT* pTS);
    int32_t GetKernelCoordinates(const uint32_t index, void  *&pKernelCoordinates);
    int32_t GetKernelDependencyMasks(const uint32_t index, void  *&pKernelDependencyMasks);
    int32_t GetDependencyPattern(CM_DEPENDENCY_PATTERN &pDependencyPattern);
    int32_t GetWalkingPattern(CM_WALKING_PATTERN &pWalkingPattern);
    int32_t GetWalkingParameters(CM_WALKING_PARAMETERS &pWalkingParameters);
    int32_t GetDependencyVectors(CM_HAL_DEPENDENCY &pDependencyVector);
    bool CheckWalkingParametersSet();
    bool CheckDependencyVectorsSet();
#if _DEBUG
    int32_t DisplayThreadSpaceData(uint32_t width, uint32_t height);
#endif
    int32_t GetThreadSpaceSize(uint32_t& width, uint32_t& height );
    int32_t GetThreadGroupSpaceSize(uint32_t& trdSpaceWidth, uint32_t& trdSpaceHeight, uint32_t& trdSpaceDepth, uint32_t& grpSpaceWidth, uint32_t& grpSpaceHeight, uint32_t& grpSpaceDepth);
    int32_t GetSLMSize(uint32_t& iSLMSize);
    int32_t GetSpillMemUsed(uint32_t& iSpillMemUsed);
    int32_t GetColorCountMinusOne(uint32_t& colorCount);
    int32_t GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT& groupSelect);
    int32_t GetHints(uint32_t& hints);
    int32_t GetNumTasksGenerated(uint32_t& numTasksGenerated);
    int32_t GetLastTask(bool& isLastTask);
    
    bool IsThreadGroupSpaceCreated(void);
    bool IsThreadSpaceCreated(void );
    bool IsThreadCoordinatesExisted(void);

    int32_t AllocateKernelSurfInfo();
    int32_t GetKernelSurfInfo(CM_HAL_SURFACE_ENTRY_INFO_ARRAYS & SurfEntryInfoArray);
    int32_t ClearKernelSurfInfo();
    int32_t ResetKernelDataStatus();
    
    int32_t GetTaskType(uint32_t& taskType);

    int32_t GetVeboxState(CM_VEBOX_STATE & pVeboxState);
    int32_t GetVeboxParam(CmBufferUP * &pVeboxParam);
    int32_t GetVeboxSurfaceData(CM_VEBOX_SURFACE_DATA &pVeboxSurfaceData);

    int32_t GetTaskSurfaces( bool  *&surfArray );

    uint64_t GetSyncBitmap();
    uint64_t GetConditionalEndBitmap();
    CM_HAL_CONDITIONAL_BB_END_INFO* GetConditionalEndInfo();

    int32_t SetPowerOption( PCM_POWER_OPTION pPowerOption );
    PCM_POWER_OPTION GetPowerOption();
    int32_t GetTaskStatus(CM_STATUS & TaskStatus);
    int32_t SetProperty(CM_TASK_CONFIG * pTaskConfig);
    PCM_TASK_CONFIG GetTaskConfig();
    void  *GetMediaStatePtr();
#if CM_LOG_ON
    std::string Log();
#endif 

    void SurfaceDump(int32_t taskId);

    int32_t VtuneInitProfilingInfo(const CmThreadGroupSpace *pPerTaskThreadGroupSpace);
    int32_t VtuneInitProfilingInfo(const CmThreadSpaceRT *pPerTaskTs);
    int32_t VtuneSetFlushTime();

protected:

    CmTaskInternal(const uint32_t kernelCount, const uint32_t totalThreadCount, CmKernelRT* pKernelArray[], CmDeviceRT* pCmDevice, const uint64_t uiSyncBitmap, const uint64_t uiConditionalEndBitmap, PCM_HAL_CONDITIONAL_BB_END_INFO pConditionalEndInfo);
    ~CmTaskInternal( void );

    int32_t Initialize(const CmThreadSpaceRT* pTS, bool isWithHints);
    int32_t Initialize(const CmThreadGroupSpace* pTGS);
    
    int32_t Initialize(CmVeboxRT* pVebox);
    int32_t Initialize(uint32_t hints, uint32_t numTasksGenerated, bool isLastTask);

    int32_t VtuneWriteEventInfo();
    int32_t VtuneReleaseProfilingInfo();

    int32_t UpdateSurfaceStateOnTaskCreation();
    int32_t UpdateSurfaceStateOnTaskDestroy();

    CmDynamicArray m_Kernels;
    CmDynamicArray m_KernelData; // one CmKernelData* per kernel
    uint32_t *m_pKernelCurbeOffsetArray; // Array to record each kernel's curbe offset. Even the same kernel could have different curbe offset. 
    uint32_t m_KernelCount;

    uint32_t m_TotalThreadCount;    //total thread count per task

    CmEventRT* m_pTaskEvent;

    //ThreadSpaceInformation from CreateThreadSpace() or CreateThreadGroupSpace()
    bool            m_IsThreadSpaceCreated;
    bool            m_IsThreadCoordinatesExisted;
    uint32_t        m_ThreadSpaceWidth;     
    uint32_t        m_ThreadSpaceHeight;
    uint32_t        m_ThreadSpaceDepth;   

    PCM_HAL_SCOREBOARD *m_pThreadCoordinates;
    CM_DEPENDENCY_PATTERN m_DependencyPattern;
    CM_WALKING_PATTERN m_WalkingPattern;
    uint32_t m_WalkingParameters[CM_NUM_DWORD_FOR_MW_PARAM];
    bool m_MediaWalkerParamsSet;
    CM_HAL_DEPENDENCY m_DependencyVectors;
    bool m_DependencyVectorsSet;
    PCM_HAL_MASK_AND_RESET    *m_pDependencyMasks;
    CM_MW_GROUP_SELECT         m_MediaWalkerGroupSelect;

    bool            m_IsThreadGroupSpaceCreated;
    uint32_t        m_GroupSpaceWidth;
    uint32_t        m_GroupSpaceHeight;
    uint32_t        m_GroupSpaceDepth;
    uint32_t        m_SLMSize;      //SLM size per thread group in 1KB unit
    uint32_t        m_SpillMemUsed;

    uint32_t        m_ColorCountMinusOne;
    uint32_t        m_Hints;
    uint32_t        m_NumTasksGenerated;
    bool            m_IsLastTask;

    uint64_t        m_ui64SyncBitmap;
    uint64_t        m_ui64ConditionalEndBitmap;
    CM_HAL_CONDITIONAL_BB_END_INFO m_ConditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];

    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS m_KernelSurfInfo;
    CmDeviceRT*                      m_pCmDevice;
    bool                             *m_SurfaceArray;  // vector-flag of surfaces R/W by this CM Task (containing multi-kernel) 
    bool                             m_IsSurfaceUpdateDone;

    uint32_t        m_TaskType; //0 - Task with thread space, 1 - Task with thread group space, 2 - Task for VEBOX
    
    CmBufferUP   *  m_pVeboxParam;
    CM_VEBOX_STATE  m_VeboxState;
    CM_VEBOX_SURFACE_DATA m_VeboxSurfaceData;

    CM_POWER_OPTION m_PowerOption;
    CM_PROFILING_INFO   m_TaskProfilingInfo;
    CM_TASK_CONFIG  m_TaskConfig;
    void            *m_media_state_ptr;
private:
    CmTaskInternal (const CmTaskInternal& other);
    CmTaskInternal& operator= (const CmTaskInternal& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASKINTERNAL_H_
