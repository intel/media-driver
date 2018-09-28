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
//! \file      cm_device_rt.h 
//! \brief     Contains CmDeviceRT declarations. 
//!

#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_

#include "cm_device.h"

#include "cm_array.h"
#include "cm_csync.h"
#include "cm_hal.h"
#include "cm_log.h"
#include "cm_program.h"
#include "cm_notifier.h"

#if USE_EXTENSION_CODE
#include "cm_gtpin.h"
#endif

namespace CMRT_UMD
{
class CmQueueRT;
class CmSurfaceManager;
class CmSampler8x8State_RT;

//! \brief    Class CmDeviceRT definitions for Linux
class CmDeviceRT: public CmDevice
{
public:
    static int32_t
    Create(MOS_CONTEXT *umdContext,
           CmDeviceRT* &device,
           uint32_t options = CM_DEVICE_CREATE_OPTION_DEFAULT);

    static int32_t Destroy(CmDeviceRT* &device);

    CM_RT_API int32_t CreateBuffer(uint32_t size,
                                   CmBuffer* &surface);

    CM_RT_API int32_t CreateBuffer(PMOS_RESOURCE mosResource,
                                   CmBuffer* &surface);

    CM_RT_API int32_t CreateSurface2D(uint32_t width,
                                      uint32_t height,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface2D* &surface);

    CM_RT_API int32_t CreateSurface2D(PMOS_RESOURCE mosResource,
                                      CmSurface2D* &surface);

    CM_RT_API int32_t CreateSurface2D(VASurfaceID vaSurface,
                                      VADriverContext *vaDriverCtx,
                                      CmSurface2D* &surface);

    CM_RT_API int32_t CreateSurface3D(uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface3D* &surface);

    CM_RT_API int32_t DestroySurface(CmBuffer* &surface);

    CM_RT_API int32_t DestroySurface(CmSurface2D* &surface);

    CM_RT_API int32_t DestroySurface(CmSurface3D* &surface);

    CM_RT_API int32_t CreateQueue(CmQueue* &queue);

    CM_RT_API int32_t LoadProgram(void *commonIsaCode,
                                  const uint32_t size,
                                  CmProgram*  &program,
                                  const char *options = nullptr);

    CM_RT_API int32_t CreateKernel(CmProgram *program,
                                   const char *kernelName,
                                   CmKernel* &kernel,
                                   const char *options = nullptr);

    CM_RT_API int32_t
    CreateSampler(const CM_SAMPLER_STATE &sampleState,
                  CmSampler* &sampler);

    CM_RT_API int32_t DestroyKernel(CmKernel* &kernel);

    CM_RT_API int32_t DestroySampler(CmSampler* &sampler);

    CM_RT_API int32_t DestroyProgram(CmProgram* &program);

    CM_RT_API int32_t DestroyThreadSpace(CmThreadSpace* &threadSpace);

    CM_RT_API int32_t CreateTask(CmTask* &task);

    CM_RT_API int32_t DestroyTask(CmTask* &task);

    CM_RT_API int32_t GetCaps(CM_DEVICE_CAP_NAME capName,
                              uint32_t &capValueSize,
                              void *capValue);

    CM_RT_API int32_t CreateThreadSpace(uint32_t width,
                                        uint32_t height,
                                        CmThreadSpace* &threadSpace);

    CM_RT_API int32_t CreateBufferUP(uint32_t size,
                                     void *sysMem,
                                     CmBufferUP* &surface);

    CM_RT_API int32_t DestroyBufferUP(CmBufferUP* &surface);

    CM_RT_API int32_t ForceDestroyBufferUP(CmBufferUP* &surface);

    CM_RT_API int32_t GetSurface2DInfo(uint32_t width,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       uint32_t &pitch,
                                       uint32_t &physicalSize);

    CM_RT_API int32_t CreateSurface2DUP(uint32_t width,
                                        uint32_t height,
                                        CM_SURFACE_FORMAT format,
                                        void *sysMem,
                                        CmSurface2DUP* &surface);

    CM_RT_API int32_t DestroySurface2DUP(CmSurface2DUP* &surface);

    CM_RT_API int32_t CreateVmeSurfaceG7_5(CmSurface2D *currentSurface,
                                           CmSurface2D **forwardSurfaces,
                                           CmSurface2D **backwardSurfaces,
                                           const uint32_t forwardSurfaceCount,
                                           const uint32_t backwardSurfaceCount,
                                           SurfaceIndex* &vmeIndex);

    CM_RT_API int32_t DestroyVmeSurfaceG7_5(SurfaceIndex* &vmeIndex);

    CM_RT_API int32_t CreateSampler8x8(const CM_SAMPLER_8X8_DESCR &sampler8x8Descriptor,
                                       CmSampler8x8 *&sampler8x8);

    CM_RT_API int32_t DestroySampler8x8(CmSampler8x8* &sampler8x8);

    CM_RT_API int32_t
    CreateSampler8x8Surface(CmSurface2D *surface2D,
                            SurfaceIndex* &sampler8x8SurfIndex,
                            CM_SAMPLER8x8_SURFACE surfType,
                            CM_SURFACE_ADDRESS_CONTROL_MODE mode);

    CM_RT_API int32_t DestroySampler8x8Surface(SurfaceIndex* &surfaceIndex);

    CM_RT_API int32_t CreateThreadGroupSpace(uint32_t thrdSpaceWidth,
                                             uint32_t thrdSpaceHeight,
                                             uint32_t grpSpaceWidth,
                                             uint32_t grpSpaceHeight,
                                             CmThreadGroupSpace* &threadGroupSpace);

    CM_RT_API int32_t DestroyThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace);

    CM_RT_API int32_t SetL3Config(const L3ConfigRegisterValues *l3Config);

    CM_RT_API int32_t SetSuggestedL3Config(L3_SUGGEST_CONFIG l3SuggestConfig);

    CM_RT_API int32_t SetCaps(CM_DEVICE_CAP_NAME capName,
                              size_t capValueSize,
                              void *capValue);

    CM_RT_API int32_t
    CreateSamplerSurface2D(CmSurface2D *Surface2d,
                           SurfaceIndex* &samplerSurfaceIndex);

    CM_RT_API int32_t
    CreateSamplerSurface3D(CmSurface3D *p3DSurface,
                           SurfaceIndex* &samplerSurfaceIndex);

    CM_RT_API int32_t
    DestroySamplerSurface(SurfaceIndex* &samplerSurfaceIndex);

    CM_RT_API int32_t
    InitPrintBuffer(size_t size = CM_DEFAULT_PRINT_BUFFER_SIZE);

    CM_RT_API int32_t FlushPrintBuffer();

    CM_RT_API int32_t CreateVebox(CmVebox* &vebox);

    CM_RT_API int32_t DestroyVebox(CmVebox* &vebox);

    CM_RT_API int32_t CreateBufferSVM(uint32_t size,
                                      void* &sysMem,
                                      uint32_t accessFlag,
                                      CmBufferSVM* &bufferSVM);

    CM_RT_API int32_t DestroyBufferSVM(CmBufferSVM* &bufferSVM);

    CM_RT_API int32_t
    CreateSamplerSurface2DUP(CmSurface2DUP *surface2dUP,
                             SurfaceIndex* &samplerSurfaceIndex);

    CM_RT_API int32_t CloneKernel(CmKernel* &kernelDst, CmKernel* kernelSrc);

    CM_RT_API int32_t CreateSurface2DAlias(CmSurface2D *surface2d,
                                           SurfaceIndex* &aliasSurfaceIndex);

    CM_RT_API int32_t
    CreateHevcVmeSurfaceG10(CmSurface2D *currentSurface,
                            CmSurface2D **forwardSurfaces,
                            CmSurface2D **backwardSurfaces,
                            const uint32_t forwardSurfaceCount,
                            const uint32_t backwardSurfaceCount,
                            SurfaceIndex* &vmeIndex);

    CM_RT_API int32_t
    DestroyHevcVmeSurfaceG10(SurfaceIndex* &vmeIndex);

    CM_RT_API int32_t CreateSamplerEx(const CM_SAMPLER_STATE_EX &sampleState,
                                      CmSampler* &sampler);

    CM_RT_API int32_t FlushPrintBufferIntoFile(const char *filename);

    CM_RT_API int32_t CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth,
                                               uint32_t thrdSpaceHeight,
                                               uint32_t thrdSpaceDepth,
                                               uint32_t grpSpaceWidth,
                                               uint32_t grpSpaceHeight,
                                               uint32_t grpSpaceDepth,
                                               CmThreadGroupSpace* &threadGroupSpace);

    CM_RT_API int32_t
    CreateSampler8x8SurfaceEx(CmSurface2D *surface2d,
                              SurfaceIndex* &sampler8x8SurfIndex,
                              CM_SAMPLER8x8_SURFACE surfType = CM_VA_SURFACE,
                              CM_SURFACE_ADDRESS_CONTROL_MODE addressControl
                                  = CM_SURFACE_CLAMP,
                              CM_FLAG *flag = nullptr);

    CM_RT_API int32_t
    CreateSamplerSurface2DEx(CmSurface2D* surface2d,
                             SurfaceIndex* &samplerSurfaceIndex,
                             CM_FLAG *flag = nullptr);

    CM_RT_API int32_t CreateBufferAlias(CmBuffer *buffer,
                                        SurfaceIndex* &aliasIndex);

    CM_RT_API int32_t
    SetVmeSurfaceStateParam(SurfaceIndex *vmeIndex,
                            CM_VME_SURFACE_STATE_PARAM *surfStateParam);

    CM_RT_API int32_t GetVISAVersion(uint32_t &majorVersion,
                                     uint32_t &minorVersion);

    CM_RT_API int32_t
    CreateQueueEx(CmQueue* &queue,
                  CM_QUEUE_CREATE_OPTION QueueCreateOption
                      = CM_DEFAULT_QUEUE_CREATE_OPTION);

    void* GetAccelData() { return m_accelData; }

    MOS_CONTEXT* GetUMDCtx() { return m_mosContext; }

    uint32_t GetAccelsize() { return m_accelSize; }

    int32_t GetHalMaxValues(CM_HAL_MAX_VALUES* &halMaxValues,
                            CM_HAL_MAX_VALUES_EX* &halMaxValuesEx);

    int32_t GetGenPlatform(uint32_t &platform);

    void Sampler8x8CoefficientFormatTransform(
         CM_AVS_INTERNEL_NONPIPLINED_STATE *dstAvsState,
         CM_AVS_NONPIPLINED_STATE *srcAvsState);

    int32_t GetSurfaceManager(CmSurfaceManager* &surfaceMgr);

    std::vector<CmQueueRT*>& GetQueue();

    CSync* GetSurfaceLock();

    CSync* GetSurfaceCreationLock();

    CSync* GetProgramKernelLock();

    CSync* GetQueueLock();

    int32_t GetJITCompileFnt(pJITCompile &jitCompile);

    int32_t GetJITCompileFntV2(pJITCompile_v2 &fJITCompile_v2);

    int32_t GetFreeBlockFnt(pFreeBlock &freeBlock);

    int32_t GetJITVersionFnt(pJITVersion &jitVersion);

    int32_t LoadJITDll();

    int32_t LoadPredefinedCopyKernel(CmProgram* &program);

    int32_t LoadPredefinedInitKernel(CmProgram* &program);

    //GTPIN related interfaces
    bool CheckGTPinEnabled();

#if USE_EXTENSION_CODE
    int32_t EnableGTPin(CmBufferUP *bufferUP0,
                        CmBufferUP *bufferUP1,
                        CmBufferUP *bufferUP2,
                        char *gtpinCom,
                        GTPIN_INVOKE_STRUCT *invokeStruct,
                        bool reserveRegisters);

    CmGTPin *GetGTPin();
#endif

    int32_t GetGenStepInfo(char* &stepinfostr);

    int32_t GetCapsInternal(void *caps, uint32_t *size);

    int32_t RegisterSyncEvent(void *syncEventHandle);

    int32_t GetOSSyncEventHandle(void* &hOSSyncEvent);

    int32_t Acquire();

    int32_t Release();

    int32_t GetPrintBufferIndex(SurfaceIndex* &index) const;

    bool IsPrintEnable() const;

    bool IsVtuneLogOn() const;

    int32_t GetPrintBufferMem(unsigned char* &printBufferMem) const;

    int32_t ClearPrintBuffer();

    int32_t GetSurf2DLookUpEntry(uint32_t index, PCMLOOKUP_ENTRY &lookupEntry);

    int32_t DestroySurfaceInPool(uint32_t &freeSurfNum);

    int32_t CreateSurface2D(PMOS_RESOURCE mosResource,
                            bool isCmCreated,
                            CmSurface2D* &surface);

    int32_t GetSampler8x8(uint32_t index, CmSampler8x8State_RT* &sampler8x8);

    int32_t SetVaCtxID(uint32_t vaCtxID);

    int32_t GetVaCtxID(uint32_t &vaCtxID);

    bool IsScratchSpaceDisabled();

    int32_t SetSurfaceArraySizeForAlias();

    uint32_t GetDriverStoreFlag() { return m_isDriverStoreEnabled; }

    void SetDriverStoreFlag(uint32_t enabled)
    { m_isDriverStoreEnabled = enabled; }

    CmDynamicArray* GetKernelArray();

    uint32_t *GetKernelCount();

#if CM_LOG_ON
    std::string Log();
#endif

    int32_t RegisterCallBack(pCallBackReleaseVaSurface callBack);

    int32_t ReleaseVASurface(void *vaDisplay, void *vaSurfaceID);

    int32_t DestroyVmeSurface(SurfaceIndex* &vmeIndex);

    CmNotifierGroup* GetNotifiers() {return m_notifierGroup;}

protected:
    int32_t Initialize(MOS_CONTEXT *mosContext);

    int32_t InitializeOSSpecific(MOS_CONTEXT *mosContext);

    int32_t CreateAuxDevice(MOS_CONTEXT *mosContext);

    int32_t DestroyAuxDevice();

    int32_t DestroyQueue(CmQueueRT* &queue);

    int32_t RegisterSamplerState(const CM_SAMPLER_STATE &sampleState,
                                 uint32_t &index);

    int32_t RegisterSamplerStateEx(const CM_SAMPLER_STATE_EX &sampleState,
                                   uint32_t &index);

    int32_t UnregisterSamplerState(uint32_t index);

    int32_t
    RegisterSampler8x8State(const CM_SAMPLER_8X8_DESCR &sampler8x8State,
                            uint32_t &index);

    int32_t UnregisterSampler8x8State(uint32_t index);

    int32_t GetMaxValueFromCaps(CM_HAL_MAX_VALUES &maxValues,
                                CM_HAL_MAX_VALUES_EX &maxValuesEx);

    int32_t InitDevCreateOption(CM_HAL_CREATE_PARAM &cmHalCreateParam,
                                uint32_t option);

    int32_t ReadVtuneProfilingFlag();

    CmDeviceRT(uint32_t option);

    ~CmDeviceRT();

    void ConstructOSSpecific(uint32_t devCreateOption);

    void DestructCommon();

    CM_RETURN_CODE GetCapsInternalOSSpecific(PCM_QUERY_CAPS queryCaps);

    int32_t QuerySurface2DFormats(void *capValue, uint32_t &capValueSize);

    inline bool IsMediaResetNeeded(uint32_t options)
    {
        UNUSED(options);
        return false;
    }

    inline HANDLE QueryRegHandleInternal(PCM_HAL_STATE cmHalState)
    {
        UNUSED(cmHalState);
        return (HANDLE)nullptr;
    }

    CM_RETURN_CODE QueryGPUInfoInternal(PCM_QUERY_CAPS queryCaps);

    CM_RETURN_CODE QuerySurface2DFormatsInternal(PCM_QUERY_CAPS queryCaps);

    int32_t FlushPrintBufferInternal(const char *filename);

    MOS_CONTEXT *m_mosContext;

    VAContextID m_vaCtxID;

    void *m_accelData;    // Pointer to the private data used by the acceleration service

    uint32_t m_accelSize;  // Size of the private data

    CM_HAL_MAX_VALUES m_halMaxValues;

    CM_HAL_MAX_VALUES_EX m_halMaxValuesEx;

    CmSurfaceManager *m_surfaceMgr;

    std::vector<CmQueueRT*> m_queue;

    CmDynamicArray m_programArray;

    uint32_t m_programCount;

    CmDynamicArray m_kernelArray;

    uint32_t m_kernelCount;

    CmDynamicArray m_sampler8x8Array;

    CmDynamicArray m_samplerArray;

    CmDynamicArray m_threadSpaceArray;

    uint32_t m_threadSpaceCount;

    CmDynamicArray m_veboxArray;

    uint32_t m_veboxCount;

    HMODULE m_hJITDll;

    pJITCompile m_fJITCompile;

    pJITCompile_v2 m_fJITCompile_v2;

    pFreeBlock m_fFreeBlock;

    pJITVersion m_fJITVersion;

    uint32_t m_ddiVersion;

    uint32_t m_platform;

    uint32_t m_cmDeviceRefCount;

    CmProgram *m_gpuCopyKernelProgram;

    CmProgram *m_surfInitKernelProgram;

    void  *m_osSyncEvent;   //KMD Notification

#if USE_EXTENSION_CODE
    CmGTPin *m_gtpin;
#endif

    // synchronization objects
    CSync m_criticalSectionProgramKernel;

    CSync m_criticalSectionSurface;

    CSync m_criticalSectionReadWriteSurface2D;

    CSync m_criticalSectionSampler;

    CSync m_criticalSectionSampler8x8;

    CSync m_criticalSectionVmeState;

    CSync m_criticalSectionThreadSpace;

    CSync m_criticalSectionDeviceRefCount;

    CSync m_criticalSectionThreadGroupSpace;

    CSync m_criticalSectionTask;

    CSync m_criticalSectionVebox;

    CSync m_criticalSectionQueue;

    pCallBackReleaseVaSurface  m_pfnReleaseVaSurface;

public:
    static CSync m_globalCriticalSectionSurf2DUserDataLock;

protected:
    unsigned char *m_printBufferMem;

    CmBufferUP *m_printBufferUP;

    bool m_isPrintEnabled;

    size_t m_printBufferSize;

    SurfaceIndex *m_printBufferIndex;

    CmDynamicArray m_threadGroupSpaceArray;

    uint32_t m_threadGroupSpaceCount;

    CmDynamicArray m_taskArray;

    uint32_t m_taskCount;

    CM_HAL_CREATE_PARAM m_cmHalCreateOption;

    uint32_t m_nGPUFreqOriginal;

    uint32_t m_nGPUFreqMin;

    uint32_t m_nGPUFreqMax;

    bool m_vtuneOn;

    uint32_t m_isDriverStoreEnabled;

    CmNotifierGroup *m_notifierGroup;

private:
    CmDeviceRT(const CmDeviceRT& other);

    CmDeviceRT& operator= (const CmDeviceRT& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_
