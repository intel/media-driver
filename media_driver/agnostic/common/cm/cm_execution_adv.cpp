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
//! \file      cm_execution_adv.cpp 
//! \brief     Contains Class CmExecutionAdv  definitions 
//!
#include "cm_execution_adv.h"
#include "cm_debug.h"
#include "cm_extension_creator.h"
#include "cm_surface_state_manager.h"
#include "cm_kernel_ex.h"
#include "cm_ish.h"
#include "cm_media_state.h"
#include "cm_command_buffer.h"
#include "cm_kernel_ex.h"
#include "cm_ssh.h"
#include "cm_event_ex.h"
#include "cm_tracker.h"
#include "cm_dsh.h"
#include "cm_task_rt.h"
#include "cm_thread_space_rt.h"
#include "cm_surface_manager.h"
#include "cm_queue_rt.h"
#include "cm_scratch_space.h"

#include "cm_hal_g9.h"
#include "cm_hal_g11.h"
#include "cm_hal_g12.h"

static bool gGTPinInitialized = false;

static bool advRegistered = CmExtensionCreator<CmExecutionAdv>::RegisterClass<CmExecutionAdv>();

using namespace CMRT_UMD;

CmExecutionAdv::CmExecutionAdv():
    m_cmhal(nullptr),
    m_tracker (nullptr),
    m_ish (nullptr),
    m_dsh (nullptr)
{
    MOS_ZeroMemory(&m_l3Values, sizeof(m_l3Values));
}

MOS_STATUS CmExecutionAdv::Initialize(CM_HAL_STATE *state)
{
    m_cmhal = state;
    CM_CHK_NULL_RETURN_MOSERROR(m_cmhal);

    m_tracker = MOS_New(CmTracker, m_cmhal->osInterface);
    CM_CHK_NULL_RETURN_MOSERROR(m_tracker);
    CM_CHK_MOSSTATUS_RETURN(m_tracker->Initialize());
    FrameTrackerProducer *trackerProducer = m_tracker->GetTrackerProducer();

    m_ish = MOS_New(CmISH);
    CM_CHK_NULL_RETURN_MOSERROR(m_ish);
    CM_CHK_MOSSTATUS_RETURN(m_ish->Initialize(m_cmhal, trackerProducer));

    m_dsh = MOS_New(CmDSH, m_cmhal);
    CM_CHK_NULL_RETURN_MOSERROR(m_dsh);
    CM_CHK_MOSSTATUS_RETURN(m_dsh->Initialize(trackerProducer));

    MOS_ZeroMemory(&m_l3Values, sizeof(m_l3Values));

    return MOS_STATUS_SUCCESS;
}

CmExecutionAdv::~CmExecutionAdv()
{
    MOS_Delete(m_ish);
    MOS_Delete(m_dsh);
    MOS_Delete(m_tracker);
}

CmSurfaceState2Dor3DMgr* CmExecutionAdv::Create2DStateMgr(MOS_RESOURCE *resource)
{
    return MOS_New(CmSurfaceState2Dor3DMgr, m_cmhal, resource);
}

CmSurfaceState2Dor3DMgr* CmExecutionAdv::Create3DStateMgr(MOS_RESOURCE *resource)
{
    return MOS_New(CmSurfaceState3DMgr, m_cmhal, resource);
}

void CmExecutionAdv::Delete2Dor3DStateMgr(CmSurfaceState2Dor3DMgr *stateMgr)
{
    MOS_Delete(stateMgr);
}

CmSurfaceStateBufferMgr* CmExecutionAdv::CreateBufferStateMgr(MOS_RESOURCE *resource)
{
    return MOS_New(CmSurfaceStateBufferMgr, m_cmhal, resource);
}

void CmExecutionAdv::DeleteBufferStateMgr(CmSurfaceStateBufferMgr *stateMgr)
{
    MOS_Delete(stateMgr);
}

void CmExecutionAdv::DeleteSurfStateVme(CmSurfaceStateVME *state)
{
    MOS_Delete(state);
}

void CmExecutionAdv::SetBufferOrigSize(CmSurfaceStateBufferMgr *stateMgr, uint32_t size)
{
    if (stateMgr)
    {
        stateMgr->SetOrigSize(size);
    }
}

void CmExecutionAdv::SetBufferMemoryObjectControl(CmSurfaceStateBufferMgr *stateMgr, uint16_t mocs)
{
    if (stateMgr)
    {
        stateMgr->SetMemoryObjectControl(mocs);
    }
}

void CmExecutionAdv::Set2Dor3DOrigFormat(CmSurfaceState2Dor3DMgr *stateMgr, MOS_FORMAT format)
{
    if (stateMgr)
    {
        stateMgr->SetOrigFormat(format);
    }
}

void CmExecutionAdv::Set2Dor3DOrigDimension(CmSurfaceState2Dor3DMgr *stateMgr, uint32_t width, uint32_t height, uint32_t depth)
{
    if (stateMgr)
    {
        stateMgr->SetOrigDimension(width, height, depth);
    }
}

void CmExecutionAdv::Set2DRenderTarget(CmSurfaceState2Dor3DMgr *stateMgr, bool renderTarget)
{
    if (stateMgr)
    {
        stateMgr->SetRenderTarget(renderTarget);
    }
}

void CmExecutionAdv::Set2Dor3DMemoryObjectControl(CmSurfaceState2Dor3DMgr *stateMgr, uint16_t mocs)
{
    if (stateMgr)
    {
        stateMgr->SetMemoryObjectControl(mocs);
    }
}

void CmExecutionAdv::Set2DFrameType(CmSurfaceState2Dor3DMgr *stateMgr, CM_FRAME_TYPE frameType)
{
    if (stateMgr)
    {
        stateMgr->SetFrameType(frameType);
    }
}

void CmExecutionAdv::SetRotationFlag(CmSurfaceState2Dor3DMgr *stateMgr, uint32_t rotation)
{
    if (stateMgr)
    {
        stateMgr->SetRotationFlag(rotation);
    }
}

void CmExecutionAdv::SetChromaSitting(CmSurfaceState2Dor3DMgr *stateMgr, uint8_t chromaSitting)
{
    if (stateMgr)
    {
        stateMgr->SetChromaSitting(chromaSitting);
    }
}

FrameTrackerProducer *CmExecutionAdv::GetFastTrackerProducer()
{
    return m_tracker->GetTrackerProducer();
}

CmKernelRT *CmExecutionAdv::CreateKernelRT(CmDeviceRT *device,
               CmProgramRT *program,
               uint32_t kernelIndex,
               uint32_t kernelSeqNum)
{
    return new (std::nothrow) CmKernelEx(device, program, kernelIndex, kernelSeqNum);
}

int CmExecutionAdv::RefreshSurfaces(CmDeviceRT *device)
{
    CM_CHK_NULL_RETURN_CMERROR(device);

    CmSurfaceManager *surfaceMgr  = nullptr;
    CSync *           surfaceLock = nullptr;

    device->GetSurfaceManager(surfaceMgr);
    CM_CHK_NULL_RETURN_CMERROR(surfaceMgr);

    surfaceLock = device->GetSurfaceCreationLock();
    CM_CHK_NULL_RETURN_CMERROR(surfaceLock);

    uint32_t freeSurfNum = 0;
    surfaceLock->Acquire();
    surfaceMgr->RefreshDelayDestroySurfaces(freeSurfNum);
    surfaceLock->Release();

    return CM_SUCCESS;
}

int CmExecutionAdv::SubmitTask(CMRT_UMD::CmQueueRT *queue, 
                CMRT_UMD::CmTask *task, 
                CMRT_UMD::CmEvent *&event,
                const CMRT_UMD::CmThreadSpace *threadSpace,
                MOS_GPU_CONTEXT gpuContext)
{
    CM_NORMALMESSAGE("================ in fast path, media walker===================");

    CM_HAL_STATE * state = m_cmhal;
    CM_CHK_NULL_RETURN_CMERROR(state->advExecutor);
    CmTracker *cmTracker = state->advExecutor->GetTracker();
    CmISH *cmish = state->advExecutor->GetISH();
    CmDSH *cmdsh = state->advExecutor->GetDSH();
    CM_CHK_NULL_RETURN_CMERROR(cmTracker);
    CM_CHK_NULL_RETURN_CMERROR(cmish);
    CM_CHK_NULL_RETURN_CMERROR(cmdsh);

    CLock Locker(m_criticalSection);

    bool isDummyEventCreated = false;
#if MDF_SURFACE_CONTENT_DUMP
    if (state->dumpSurfaceContent && event == CM_NO_EVENT)
    {
        // if surface content dump is needed, the enqueueFast should be a blocking operation
        // we need a dummy event here
        isDummyEventCreated = true;
        event = nullptr;
    }
#endif

    state->osInterface->pfnResetOsStates(state->osInterface);
    state->osInterface->pfnSetIndirectStateSize(state->osInterface, state->renderHal->dwIndirectHeapSize);

    CM_HAL_OSSYNC_PARAM syncParam;
    syncParam.osSyncEvent = nullptr;

    // Call HAL layer to wait for Task finished with event-driven mechanism
    CM_CHK_MOSSTATUS_RETURN(m_cmhal->pfnRegisterUMDNotifyEventHandle(m_cmhal, &syncParam));

    HANDLE osSyncEvent = syncParam.osSyncEvent;

    CmTaskRT *kernelArrayRT = static_cast<CmTaskRT *>(task);
    uint32_t kernelCount = kernelArrayRT->GetKernelCount();
    if (kernelCount == 0 || kernelCount > CM_MAX_KERNELS_PER_TASK)
    {
        return CM_FAILURE;
    }

    // get an array of CmKernelEx
    CmKernelEx *kernels[CM_MAX_KERNELS_PER_TASK];
    MOS_ZeroMemory(kernels, sizeof(kernels));
    for (uint32_t i = 0; i < kernelCount; i++)
    {
        kernels[i] = static_cast<CmKernelEx *>(kernelArrayRT->GetKernelPointer(i));
        CM_CHK_NULL_RETURN_CMERROR(kernels[i]);
        kernels[i]->AllocateCurbe();
    }

    // get CmDeviceRT
    CmDeviceRT *device = nullptr;
    kernels[0]->GetCmDevice(device);
    CM_CHK_NULL_RETURN_CMERROR(device);

    // set printf buffer if needed
    if (device->IsPrintEnable())
    {
        SurfaceIndex *printBufferIndex = nullptr;
        device->CreatePrintBuffer();
        device->GetPrintBufferIndex(printBufferIndex);
        CM_ASSERT(printBufferIndex);
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            kernels[i]->SetStaticBuffer(CM_PRINTF_STATIC_BUFFER_ID, printBufferIndex);
        }
    }

    const CmThreadSpaceRT *threadSpaceRTConst = static_cast<const CmThreadSpaceRT *>(threadSpace);
    CmThreadSpaceRT *threadSpaceRT = const_cast<CmThreadSpaceRT *>(threadSpaceRTConst);
    CmThreadSpaceRT *threadSpaces[CM_MAX_KERNELS_PER_TASK];
    MOS_ZeroMemory(threadSpaces, sizeof(threadSpaces));
    if (threadSpaceRT == nullptr)
    {
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            threadSpaces[i] = kernels[i]->GetThreadSpaceEx();
        }
    }

    // if SWSB is used, update the SWSB arguments in kenrel
    if (!state->cmHalInterface->IsScoreboardParamNeeded())
    {
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            kernels[i]->UpdateSWSBArgs(threadSpaceRT);
        }
    }

    CmCommandBuffer cmdBufData(state);
    CmCommandBuffer *cmdBuf = &cmdBufData;
    CM_CHK_NULL_RETURN_CMERROR(cmdBuf);

    uint32_t tracker;
    uint32_t taskId;
    MOS_STATUS mret = cmTracker->AssignFrameTracker(queue->GetFastTrackerIndex(), &taskId, &tracker, event != CM_NO_EVENT);
    bool taskAssigned = (mret == MOS_STATUS_SUCCESS);

    cmdBuf->Initialize();

    CmSSH *ssh = cmdBuf->GetSSH();
    CM_CHK_NULL_RETURN_CMERROR(ssh);

    // Add kernels to ISH directly
    cmish->LoadKernels(kernels, kernelCount);

    // initialize SSH
    ssh->Initialize(kernels, kernelCount);

    // create new media state
    CmMediaState *cmMediaState = cmdsh->CreateMediaState();
    CM_CHK_NULL_RETURN_CMERROR(cmMediaState);
    cmMediaState->Allocate(kernels, kernelCount, queue->GetFastTrackerIndex(), tracker);

    // generate curbe and load media id
    for (uint32_t i = 0; i < kernelCount; i++)
    {
        ssh->AssignBindingTable();
        kernels[i]->LoadReservedSamplers(cmMediaState, i);
        kernels[i]->LoadReservedSurfaces(ssh);
        kernels[i]->UpdateCurbe(ssh, cmMediaState, i);
        kernels[i]->UpdateFastTracker(queue->GetFastTrackerIndex(), tracker);
        cmMediaState->LoadCurbe(kernels[i], i);
        cmMediaState->LoadMediaID(kernels[i], i, ssh->GetBindingTableOffset());
    }

    // prepare cp resources
    ssh->PrepareResourcesForCp();

    // get the position to write tracker
    MOS_RESOURCE *trackerResource = nullptr;
    uint32_t trackerOffset = 0;
    cmTracker->GetLatestTrackerResource(queue->GetFastTrackerIndex(), &trackerResource, &trackerOffset);

    // call gtpin callback if needed
    CmNotifierGroup *ng = nullptr;
    if (gGTPinInitialized && taskAssigned)
    {
        ng = device->GetNotifiers();
        ng->NotifyTaskFlushed(device, task, ssh, taskId);
    }

    if (m_cmhal->platform.eRenderCoreFamily >= IGFX_GEN12_CORE)
    {
        cmdBuf->AddMMCProlog();
    }
    cmdBuf->AddFlushCacheAndSyncTask(false, false, nullptr);
    cmdBuf->AddFlushCacheAndSyncTask(true, false, nullptr);

    cmdBuf->AddPowerOption(kernelArrayRT->GetPowerOption());

    cmdBuf->AddProtectedProlog();

    cmdBuf->AddReadTimeStamp(cmTracker->GetResource(), cmTracker->GetStartOffset(taskId));
    cmdBuf->AddUmdProfilerStart();

    cmdBuf->AddL3CacheConfig(&m_l3Values);

    cmdBuf->AddPreemptionConfig(false);

    cmdBuf->AddPipelineSelect(false);

    cmdBuf->AddStateBaseAddress(cmish, cmMediaState);

    CM_TASK_CONFIG taskConfig;
    kernelArrayRT->GetProperty(taskConfig);
    if (threadSpaceRT)
    {
        cmdBuf->AddMediaVFE(cmMediaState, taskConfig.fusedEuDispatchFlag == CM_FUSED_EU_ENABLE, &threadSpaceRT);  // global thread space
    }
    else
    {
        cmdBuf->AddMediaVFE(cmMediaState, taskConfig.fusedEuDispatchFlag == CM_FUSED_EU_ENABLE, threadSpaces, kernelCount);
    }

    cmdBuf->AddCurbeLoad(cmMediaState);

    cmdBuf->AddMediaIDLoad(cmMediaState);

    CM_HAL_CONDITIONAL_BB_END_INFO *cbbInfos = kernelArrayRT->GetConditionalEndInfo();
    uint64_t conditionalBitMap = kernelArrayRT->GetConditionalEndBitmap();
    for (uint32_t i = 0; i < kernelCount; i ++)
    {
        CmThreadSpaceRT *ts = (threadSpaceRT != nullptr) ? threadSpaceRT: threadSpaces[i];

        // check whether need to insert a CBB
        bool needCBB = conditionalBitMap & ((uint64_t)1 << i);
        if (needCBB)
        {
            cmdBuf->AddFlushCacheAndSyncTask(false, true, nullptr);

            cmdBuf->AddReadTimeStamp(cmTracker->GetResource(), cmTracker->GetEndOffset(taskId), true);

            cmdBuf->AddConditionalFrameTracker(trackerResource, trackerOffset, tracker, &cbbInfos[i]);

            cmdBuf->AddConditionalBatchBufferEnd(&cbbInfos[i]);
        }

        if (i > 0)
        {
            // check whether the next kernel has a dependency pattern
            uint32_t dcount = 0;
            if (ts != nullptr)
            {
                CM_HAL_DEPENDENCY *dependency;
                ts->GetDependency(dependency);
                dcount = dependency->count;
            }

            bool syncFlag = false;
            uint64_t syncBitMap = kernelArrayRT->GetSyncBitmap();
            syncFlag = syncBitMap & ((uint64_t)1 << (i-1));
            // add sync if necessary
            if ((dcount != 0) || syncFlag)
            {
                cmdBuf->AddSyncBetweenKernels();
            }
        }

        cmdBuf->AddMediaObjectWalker(ts, i);
    }

    cmdBuf->AddFlushCacheAndSyncTask(false, true, nullptr);

    cmdBuf->AddUmdProfilerEnd();
    cmdBuf->AddReadTimeStamp(cmTracker->GetResource(), cmTracker->GetEndOffset(taskId), true);

    cmdBuf->AddFrameTracker(trackerResource, trackerOffset, tracker);

    cmdBuf->AddDummyVFE();

    cmdBuf->AddBatchBufferEnd();

    cmdBuf->ReturnUnusedBuffer();

#if MDF_SURFACE_STATE_DUMP
    if (m_cmhal->dumpSurfaceState)
    {
        ssh->DumpSSH();
    }
#endif

#if MDF_COMMAND_BUFFER_DUMP
    if (m_cmhal->dumpCommandBuffer)
    {
        cmdBuf->Dump();
    }
#endif

#if MDF_CURBE_DATA_DUMP
    if (m_cmhal->dumpCurbeData)
    {
        cmMediaState->Dump();
    }
#endif

    cmdBuf->Submit();

    cmish->Submit(queue->GetFastTrackerIndex(), tracker);

    cmMediaState->Submit();
    cmdsh->DestroyMediaState(cmMediaState);

    if (event != CM_NO_EVENT && taskAssigned)
    {
        CmEventEx *eventEx = MOS_New(CmEventEx, state, taskId, cmTracker);
        CM_CHK_NULL_RETURN_CMERROR(eventEx);
        cmTracker->AssociateEvent(eventEx);
        eventEx->SetTaskOsData(cmdBuf->GetResource(), osSyncEvent);
        event = static_cast<CmEventEx *>(eventEx);

        if (gGTPinInitialized)
        {
            eventEx->SetNotifier(ng);
        }
    }
    else
    {
        event = nullptr;
    }
    cmTracker->Refresh();

    // refresh surfaces in surface manager
    CM_CHK_CMSTATUS_RETURN(RefreshSurfaces(device));

#if MDF_SURFACE_CONTENT_DUMP
    if (state->dumpSurfaceContent && event != nullptr)
    {
        event->WaitForTaskFinished();
        if (isDummyEventCreated)
        {
            DestoryEvent(queue, event);
        }
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            kernels[i]->SurfaceDumpEx(i, taskId);
        }
    }
#endif

    return CM_SUCCESS;
}

int CmExecutionAdv::DestoryEvent(CMRT_UMD::CmQueueRT *queue, CMRT_UMD::CmEvent *&event)
{
    CmEventEx *eventEx = static_cast<CmEventEx *>(event);
    MOS_Delete(eventEx);
    event = nullptr;
    return CM_SUCCESS;
}

int CmExecutionAdv::SubmitComputeTask(CMRT_UMD::CmQueueRT *queue,
                CMRT_UMD::CmTask *task,
                CMRT_UMD::CmEvent* &event,
                const CMRT_UMD::CmThreadGroupSpace* threadGroupSpace,
                MOS_GPU_CONTEXT gpuContext)
{
    CM_ASSERTMESSAGE("Compute Tasks not support on this platform\n");
    return CM_FAILURE;
}

int CmExecutionAdv::WaitForAllTasksFinished()
{
    return m_tracker->WaitForAllTasksFinished();
}

void CmExecutionAdv::SetL3Config(const L3ConfigRegisterValues *l3Config)
{
    m_l3Values.config_register0 = l3Config->config_register0;
    m_l3Values.config_register1 = l3Config->config_register1;
    m_l3Values.config_register2 = l3Config->config_register2;
    m_l3Values.config_register3 = l3Config->config_register3;
}

int CmExecutionAdv::SetSuggestedL3Config(L3_SUGGEST_CONFIG l3SuggestConfig)
{
    const L3ConfigRegisterValues *table = nullptr;
    uint32_t count = 0;

    switch(m_cmhal->platform.eRenderCoreFamily)
    {
        case IGFX_GEN11_CORE:
            count = sizeof(ICL_L3_PLANE)/sizeof(L3ConfigRegisterValues);
            table = (L3ConfigRegisterValues *)ICL_L3_PLANE;
            break;
        case IGFX_GEN12_CORE:
            table = m_cmhal->cmHalInterface->m_l3Plane;
            count = m_cmhal->cmHalInterface->m_l3ConfigCount;
            break;
        default: // gen9
            count = sizeof(SKL_L3_PLANE)/sizeof(L3ConfigRegisterValues);
            table = (L3ConfigRegisterValues *)SKL_L3_PLANE;
            break;
    }
    if (static_cast<size_t>(l3SuggestConfig) >= count)
    {
        return CM_INVALID_ARG_VALUE;
    }
    m_l3Values.config_register0 = table[l3SuggestConfig].config_register0;
    m_l3Values.config_register1 = table[l3SuggestConfig].config_register1;
    m_l3Values.config_register2 = table[l3SuggestConfig].config_register2;
    m_l3Values.config_register3 = table[l3SuggestConfig].config_register3;

    return CM_SUCCESS;
}

int CmExecutionAdv::AssignNewTracker()
{
    FrameTrackerProducer *trackerProducer = m_tracker->GetTrackerProducer();
    return trackerProducer->AssignNewTracker();
}

int CmExecutionAdv::SubmitGpgpuTask(CMRT_UMD::CmQueueRT *queue,
                CMRT_UMD::CmTask *task,
                CMRT_UMD::CmEvent* &event,
                const CMRT_UMD::CmThreadGroupSpace* threadGroupSpace,
                MOS_GPU_CONTEXT gpuContext)
{
    CM_NORMALMESSAGE("================ in fast path, gpgpu walker===================");

    CM_HAL_STATE * state = m_cmhal;
    CM_CHK_NULL_RETURN_CMERROR(state->advExecutor);
    CmTracker *cmTracker = state->advExecutor->GetTracker();
    CmISH *cmish = state->advExecutor->GetISH();
    CmDSH *cmdsh = state->advExecutor->GetDSH();
    CM_CHK_NULL_RETURN_CMERROR(cmTracker);
    CM_CHK_NULL_RETURN_CMERROR(cmish);
    CM_CHK_NULL_RETURN_CMERROR(cmdsh);

    CLock Locker(m_criticalSection);

    bool isDummyEventCreated = false;
#if MDF_SURFACE_CONTENT_DUMP
    if (state->dumpSurfaceContent && event == CM_NO_EVENT)
    {
        // if surface content dump is needed, the enqueueFast should be a blocking operation
        // we need a dummy event here
        isDummyEventCreated = true;
        event = nullptr;
    }
#endif

    state->osInterface->pfnSetGpuContext(state->osInterface, gpuContext);
    state->osInterface->pfnResetOsStates(state->osInterface);
    state->osInterface->pfnSetIndirectStateSize(state->osInterface, state->renderHal->dwIndirectHeapSize);

    CM_HAL_OSSYNC_PARAM syncParam;
    syncParam.osSyncEvent = nullptr;

    // Call HAL layer to wait for Task finished with event-driven mechanism
    CM_CHK_MOSSTATUS_RETURN(m_cmhal->pfnRegisterUMDNotifyEventHandle(m_cmhal, &syncParam));

    HANDLE osSyncEvent = syncParam.osSyncEvent;

    CmTaskRT *kernelArrayRT = static_cast<CmTaskRT *>(task);
    uint32_t kernelCount = kernelArrayRT->GetKernelCount();
    if (kernelCount == 0 || kernelCount > CM_MAX_KERNELS_PER_TASK)
    {
        return CM_FAILURE;
    }

    // get an array of CmKernelEx
    CmKernelEx *kernels[CM_MAX_KERNELS_PER_TASK];
    MOS_ZeroMemory(kernels, sizeof(kernels));
    for (uint32_t i = 0; i < kernelCount; i++)
    {
        kernels[i] = static_cast<CmKernelEx *>(kernelArrayRT->GetKernelPointer(i));
        CM_CHK_NULL_RETURN_CMERROR(kernels[i]);
        kernels[i]->AllocateCurbeAndFillImplicitArgs(const_cast<CmThreadGroupSpace *>(threadGroupSpace));
    }

    // get CmDeviceRT
    CmDeviceRT *device = nullptr;
    kernels[0]->GetCmDevice(device);
    CM_CHK_NULL_RETURN_CMERROR(device);

    // set printf buffer if needed
    if (device->IsPrintEnable())
    {
        SurfaceIndex *printBufferIndex = nullptr;
        device->CreatePrintBuffer();
        device->GetPrintBufferIndex(printBufferIndex);
        CM_ASSERT(printBufferIndex);
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            kernels[i]->SetStaticBuffer(CM_PRINTF_STATIC_BUFFER_ID, printBufferIndex);
        }
    }

    CmThreadGroupSpace *threadGroupSpaces[CM_MAX_KERNELS_PER_TASK];
    MOS_ZeroMemory(threadGroupSpaces, sizeof(threadGroupSpaces));
    if (threadGroupSpace == nullptr)
    {
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            threadGroupSpaces[i] = kernels[i]->GetThreadGroupSpaceEx();
        }
    }

    CmCommandBuffer cmdBufData(state);
    CmCommandBuffer *cmdBuf = &cmdBufData;
    CM_CHK_NULL_RETURN_CMERROR(cmdBuf);

    uint32_t tracker;
    uint32_t taskId;
    MOS_STATUS mret = cmTracker->AssignFrameTracker(queue->GetFastTrackerIndex(), &taskId, &tracker, event != CM_NO_EVENT);
    bool taskAssigned = (mret == MOS_STATUS_SUCCESS);

    cmdBuf->Initialize();

    CmSSH *ssh = cmdBuf->GetSSH();
    CM_CHK_NULL_RETURN_CMERROR(ssh);

    // Add kernels to ISH directly
    cmish->LoadKernels(kernels, kernelCount);

    // initialize SSH
    ssh->Initialize(kernels, kernelCount);

    // create new media state
    CmMediaState *cmMediaState = cmdsh->CreateMediaState();
    CM_CHK_NULL_RETURN_CMERROR(cmMediaState);
    cmMediaState->Allocate(kernels, kernelCount, 0, tracker);

    // generate curbe and load media id
    for (uint32_t i = 0; i < kernelCount; i++)
    {
        ssh->AssignBindingTable();
        kernels[i]->LoadReservedSamplers(cmMediaState, i);
        kernels[i]->LoadReservedSurfaces(ssh);
        kernels[i]->UpdateCurbe(ssh, cmMediaState, i);
        kernels[i]->UpdateFastTracker(queue->GetFastTrackerIndex(), tracker);
        cmMediaState->LoadCurbe(kernels[i], i);
        CmThreadGroupSpace *tgs = (threadGroupSpace != nullptr) ?
                                  const_cast<CmThreadGroupSpace *>(threadGroupSpace)
                                  : threadGroupSpaces[i];
        cmMediaState->LoadMediaID(kernels[i], i, ssh->GetBindingTableOffset(), tgs);
    }

    // prepare cp resources
    ssh->PrepareResourcesForCp();

    // get the position to write tracker
    MOS_RESOURCE *trackerResource = nullptr;
    uint32_t trackerOffset = 0;
    cmTracker->GetLatestTrackerResource(queue->GetFastTrackerIndex(), &trackerResource, &trackerOffset);

    if (m_cmhal->platform.eRenderCoreFamily >= IGFX_GEN12_CORE)
    {
        cmdBuf->AddMMCProlog();
    }
    cmdBuf->AddFlushCacheAndSyncTask(false, false, nullptr);
    cmdBuf->AddFlushCacheAndSyncTask(true, false, nullptr);

    cmdBuf->AddPowerOption(kernelArrayRT->GetPowerOption());

    cmdBuf->AddProtectedProlog();

    cmdBuf->AddReadTimeStamp(cmTracker->GetResource(), cmTracker->GetStartOffset(taskId));
    cmdBuf->AddUmdProfilerStart();

    cmdBuf->AddL3CacheConfig(&m_l3Values);

    cmdBuf->AddPreemptionConfig(true);

    cmdBuf->AddPipelineSelect(true);

    cmdBuf->AddStateBaseAddress(cmish, cmMediaState);

    cmdBuf->AddSipState(cmish->GetSipKernelOffset());

    CM_CHK_MOSSTATUS_RETURN(m_cmhal->osInterface->pfnRegisterResource(
        m_cmhal->osInterface,
        &m_cmhal->csrResource,
        true,
        true));

    cmdBuf->AddCsrBaseAddress(&m_cmhal->csrResource);

    CM_TASK_CONFIG taskConfig;
    kernelArrayRT->GetProperty(taskConfig);
    cmdBuf->AddMediaVFE(cmMediaState, taskConfig.fusedEuDispatchFlag == CM_FUSED_EU_ENABLE);

    cmdBuf->AddCurbeLoad(cmMediaState);

    cmdBuf->AddMediaIDLoad(cmMediaState);

    const CM_EXECUTION_CONFIG *exeConfig = kernelArrayRT->GetKernelExecuteConfig();
    CM_HAL_CONDITIONAL_BB_END_INFO *cbbInfos = kernelArrayRT->GetConditionalEndInfo();
    uint64_t conditionalBitMap = kernelArrayRT->GetConditionalEndBitmap();
    for (uint32_t i = 0; i < kernelCount; i ++)
    {
        CmThreadGroupSpace *tgs = (threadGroupSpace != nullptr) ? const_cast<CmThreadGroupSpace *>(threadGroupSpace) : threadGroupSpaces[i];

        // check whether need to insert a CBB
        bool needCBB = conditionalBitMap & ((uint64_t)1 << i);
        if (needCBB)
        {
            cmdBuf->AddFlushCacheAndSyncTask(false, true, nullptr);

            cmdBuf->AddReadTimeStamp(cmTracker->GetResource(), cmTracker->GetEndOffset(taskId), true);

            cmdBuf->AddConditionalFrameTracker(trackerResource, trackerOffset, tracker, &cbbInfos[i]);

            cmdBuf->AddConditionalBatchBufferEnd(&cbbInfos[i]);
        }

        if (i > 0)
        {
            bool syncFlag = false;
            uint64_t syncBitMap = kernelArrayRT->GetSyncBitmap();
            syncFlag = syncBitMap & ((uint64_t)1 << (i-1));
            // add sync if necessary
            if (syncFlag)
            {
                cmdBuf->AddSyncBetweenKernels();
            }
        }

        cmdBuf->AddGpgpuWalker(tgs, kernels[i], i);
    }

    cmdBuf->AddFlushCacheAndSyncTask(false, true, nullptr);

    cmdBuf->AddUmdProfilerEnd();
    cmdBuf->AddReadTimeStamp(cmTracker->GetResource(), cmTracker->GetEndOffset(taskId), true);

    cmdBuf->AddFrameTracker(trackerResource, trackerOffset, tracker);

    cmdBuf->AddDummyVFE();

    cmdBuf->AddBatchBufferEnd();

    cmdBuf->ReturnUnusedBuffer();

    cmdBuf->Submit();

    cmish->Submit(queue->GetFastTrackerIndex(), tracker);

#if MDF_SURFACE_STATE_DUMP
    if (m_cmhal->dumpSurfaceState)
    {
        ssh->DumpSSH();
    }
#endif

#if MDF_COMMAND_BUFFER_DUMP
    if (m_cmhal->dumpCommandBuffer)
    {
        cmdBuf->Dump();
    }
#endif

#if MDF_CURBE_DATA_DUMP
    if (m_cmhal->dumpCurbeData)
    {
        cmMediaState->Dump();
    }
#endif

    cmMediaState->Submit();
    cmdsh->DestroyMediaState(cmMediaState);

    if (event != CM_NO_EVENT && taskAssigned)
    {
        CmEventEx *eventEx = MOS_New(CmEventEx, state, taskId, cmTracker);
        eventEx->SetTaskOsData(cmdBuf->GetResource(), osSyncEvent);
        event = static_cast<CmEventEx *>(eventEx);
    }
    else
    {
        event = nullptr;
    }
    cmTracker->Refresh();

    // refresh surfaces in surface manager
    CM_CHK_CMSTATUS_RETURN(RefreshSurfaces(device));

#if MDF_SURFACE_CONTENT_DUMP
    if (state->dumpSurfaceContent && event != nullptr)
    {
        event->WaitForTaskFinished();
        if (isDummyEventCreated)
        {
            DestoryEvent(queue, event);
        }
        for (uint32_t i = 0; i < kernelCount; i++)
        {
            kernels[i]->SurfaceDumpEx(i, taskId);
        }
    }
#endif

    return CM_SUCCESS;
}

bool CmExecutionAdv::SwitchToFastPath(CmTask *task)
{
    CmTaskRT *kernelArrayRT = static_cast<CmTaskRT *>(task);
    uint32_t kernelCount = kernelArrayRT->GetKernelCount();
    for (uint32_t i = 0; i < kernelCount; i++)
    {
        CmKernelEx *kernel = static_cast<CmKernelEx *>(kernelArrayRT->GetKernelPointer(i));
        if (kernel == nullptr)
        {
            return false;
        }
        if (kernel->IsFastPathSupported() == false)
        {
            return false;
        }
    }
    return true;
}

