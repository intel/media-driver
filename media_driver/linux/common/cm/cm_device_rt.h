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
    Create(MOS_CONTEXT *pUmdContext,
           CmDeviceRT* &pDevice,
           uint32_t DevCreateOption = CM_DEVICE_CREATE_OPTION_DEFAULT);

    static int32_t Destroy(CmDeviceRT* &pDevice);

    CM_RT_API int32_t CreateBuffer(uint32_t size,
                                   CmBuffer* &pSurface);

    CM_RT_API int32_t CreateBuffer(PMOS_RESOURCE pMosResource,
                                   CmBuffer* &pSurface);

    CM_RT_API int32_t CreateSurface2D(uint32_t width,
                                      uint32_t height,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface2D* &pSurface);

    CM_RT_API int32_t CreateSurface2D(PMOS_RESOURCE pMosResource,
                                      CmSurface2D* &pSurface);

    CM_RT_API int32_t CreateSurface2D(VASurfaceID iVASurface,
                                      VADriverContext *pUMDCtx,
                                      CmSurface2D* &pSurface);

    CM_RT_API int32_t CreateSurface3D(uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface3D* &pSurface);

    CM_RT_API int32_t DestroySurface(CmBuffer* &pSurface);

    CM_RT_API int32_t DestroySurface(CmSurface2D* &pSurface);

    CM_RT_API int32_t DestroySurface(CmSurface3D* &pSurface);

    CM_RT_API int32_t CreateQueue(CmQueue* &pQueue);

    CM_RT_API int32_t LoadProgram(void *pCommonISACode,
                                  const uint32_t size,
                                  CmProgram*  &pProgram,
                                  const char *options = nullptr);

    CM_RT_API int32_t CreateKernel(CmProgram *pProgram,
                                   const char *kernelName,
                                   CmKernel* &pKernel,
                                   const char *options = nullptr);

    CM_RT_API int32_t
    CreateSampler(const CM_SAMPLER_STATE &sampleState,
                  CmSampler* &pSampler);

    CM_RT_API int32_t DestroyKernel(CmKernel* &pKernel);

    CM_RT_API int32_t DestroySampler(CmSampler* &pSampler);

    CM_RT_API int32_t DestroyProgram(CmProgram* &pProgram);

    CM_RT_API int32_t DestroyThreadSpace(CmThreadSpace* &pTS);

    CM_RT_API int32_t CreateTask(CmTask* &pTask);

    CM_RT_API int32_t DestroyTask(CmTask* &pTask);

    CM_RT_API int32_t GetCaps(CM_DEVICE_CAP_NAME capName,
                              uint32_t &capValueSize,
                              void *pCapValue);

    CM_RT_API int32_t CreateThreadSpace(uint32_t width,
                                        uint32_t height,
                                        CmThreadSpace* &pTS);

    CM_RT_API int32_t CreateBufferUP(uint32_t size,
                                     void *pSystMem,
                                     CmBufferUP* &pSurface);

    CM_RT_API int32_t DestroyBufferUP(CmBufferUP* &pSurface);

    CM_RT_API int32_t ForceDestroyBufferUP(CmBufferUP* &pSurface);

    CM_RT_API int32_t GetSurface2DInfo(uint32_t width,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       uint32_t &pitch,
                                       uint32_t &physicalSize);

    CM_RT_API int32_t CreateSurface2DUP(uint32_t width,
                                        uint32_t height,
                                        CM_SURFACE_FORMAT format,
                                        void *pSysMem,
                                        CmSurface2DUP* &pSurface);

    CM_RT_API int32_t DestroySurface2DUP(CmSurface2DUP* &pSurface);

    CM_RT_API int32_t CreateVmeSurfaceG7_5(CmSurface2D *pCurSurface,
                                           CmSurface2D **pForwardSurface,
                                           CmSurface2D **pBackwardSurface,
                                           const uint32_t surfaceCountForward,
                                           const uint32_t surfaceCountBackward,
                                           SurfaceIndex* &pVmeIndex);

    CM_RT_API int32_t DestroyVmeSurfaceG7_5(SurfaceIndex* &pVmeIndex);

    CM_RT_API int32_t CreateSampler8x8(const CM_SAMPLER_8X8_DESCR &smplDescr,
                                       CmSampler8x8 *&psmplrState);

    CM_RT_API int32_t DestroySampler8x8(CmSampler8x8* &ps8x8State);

    CM_RT_API int32_t 
    CreateSampler8x8Surface(CmSurface2D *p2DSurface,
                            SurfaceIndex* &pDIIndex,
                            CM_SAMPLER8x8_SURFACE surf_type,
                            CM_SURFACE_ADDRESS_CONTROL_MODE mode);

    CM_RT_API int32_t DestroySampler8x8Surface(SurfaceIndex* &pDIIndex);

    CM_RT_API int32_t CreateThreadGroupSpace(uint32_t thrdSpaceWidth,
                                             uint32_t thrdSpaceHeight,
                                             uint32_t grpSpaceWidth,
                                             uint32_t grpSpaceHeight,
                                             CmThreadGroupSpace* &pTGS);

    CM_RT_API int32_t DestroyThreadGroupSpace(CmThreadGroupSpace* &pTGS);

    CM_RT_API int32_t SetL3Config(const L3ConfigRegisterValues *l3_c);

    CM_RT_API int32_t SetSuggestedL3Config(L3_SUGGEST_CONFIG l3_s_c);

    CM_RT_API int32_t SetCaps(CM_DEVICE_CAP_NAME capName,
                              size_t capValueSize,
                              void *pCapValue);

    CM_RT_API int32_t
    CreateSamplerSurface2D(CmSurface2D *p2DSurface,
                           SurfaceIndex* &pSamplerSurfaceIndex);

    CM_RT_API int32_t
    CreateSamplerSurface3D(CmSurface3D *p3DSurface,
                           SurfaceIndex* &pSamplerSurfaceIndex);

    CM_RT_API int32_t
    DestroySamplerSurface(SurfaceIndex* &pSamplerSurfaceIndex);

    CM_RT_API int32_t 
    InitPrintBuffer(size_t size = CM_DEFAULT_PRINT_BUFFER_SIZE);

    CM_RT_API int32_t FlushPrintBuffer();

    CM_RT_API int32_t CreateVebox(CmVebox* &pVebox);

    CM_RT_API int32_t DestroyVebox(CmVebox* &pVebox);

    CM_RT_API int32_t CreateBufferSVM(uint32_t size,
                                      void* &pSystMem,
                                      uint32_t access_flag,
                                      CmBufferSVM* &pBufferSVM);

    CM_RT_API int32_t DestroyBufferSVM(CmBufferSVM* &pBufferSVM);

    CM_RT_API int32_t
    CreateSamplerSurface2DUP(CmSurface2DUP *p2DUPSurface,
                             SurfaceIndex* &pSamplerSurfaceIndex);

    CM_RT_API int32_t CloneKernel(CmKernel* &pKernelDest, CmKernel* pKernelSrc);

    CM_RT_API int32_t CreateSurface2DAlias(CmSurface2D *p2DSurface,
                                           SurfaceIndex* &aliasSurfaceIndex);

    CM_RT_API int32_t
    CreateHevcVmeSurfaceG10(CmSurface2D *pCurSurface,
                            CmSurface2D **pForwardSurface,
                            CmSurface2D **pBackwardSurface,
                            const uint32_t surfaceCountForward,
                            const uint32_t surfaceCountBackward,
                            SurfaceIndex* &pVmeIndex);

    CM_RT_API int32_t
    DestroyHevcVmeSurfaceG10(SurfaceIndex* &pVmeIndex);

    CM_RT_API int32_t CreateSamplerEx(const CM_SAMPLER_STATE_EX &sampleState,
                                      CmSampler* &pSampler);

    CM_RT_API int32_t FlushPrintBufferIntoFile(const char *filename);

    CM_RT_API int32_t CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth,
                                               uint32_t thrdSpaceHeight,
                                               uint32_t thrdSpaceDepth,
                                               uint32_t grpSpaceWidth,
                                               uint32_t grpSpaceHeight,
                                               uint32_t grpSpaceDepth,
                                               CmThreadGroupSpace* &pTGS);

    CM_RT_API int32_t
    CreateSampler8x8SurfaceEx(CmSurface2D *p2DSurface,
                              SurfaceIndex* &pDIIndex,
                              CM_SAMPLER8x8_SURFACE surf_type = CM_VA_SURFACE,
                              CM_SURFACE_ADDRESS_CONTROL_MODE address_control
                                  = CM_SURFACE_CLAMP,
                              CM_FLAG *pFlag = nullptr);

    CM_RT_API int32_t
    CreateSamplerSurface2DEx(CmSurface2D* p2DSurface,
                             SurfaceIndex* &pSamplerSurfaceIndex,
                             CM_FLAG *pFlag = nullptr);

    CM_RT_API int32_t CreateBufferAlias(CmBuffer *pBuffer,
                                        SurfaceIndex* &pAliasIndex);

    CM_RT_API int32_t
    SetVmeSurfaceStateParam(SurfaceIndex *pVmeIndex,
                            CM_VME_SURFACE_STATE_PARAM *pSSParam);

    CM_RT_API int32_t GetVISAVersion(uint32_t &majorVersion,
                                     uint32_t &minorVersion);

    CM_RT_API int32_t 
    CreateQueueEx(CmQueue* &pQueue,
                  CM_QUEUE_CREATE_OPTION QueueCreateOption
                      = CM_DEFAULT_QUEUE_CREATE_OPTION);

    void* GetAccelData() { return m_pAccelData; }

    MOS_CONTEXT* GetUMDCtx() { return m_pUmdContext; }

    uint32_t GetAccelsize() { return m_AccelSize; }

    int32_t GetHalMaxValues(CM_HAL_MAX_VALUES* &pHalMaxValues,
                            CM_HAL_MAX_VALUES_EX* &pHalMaxValuesEx);

    int32_t GetGenPlatform(uint32_t &platform);

    void Sampler8x8CoefficientFormatTransform(
         CM_AVS_INTERNEL_NONPIPLINED_STATE *dst_avs_state,
         CM_AVS_NONPIPLINED_STATE *src_avs_state);

    int32_t GetSurfaceManager(CmSurfaceManager* &pSurfaceMgr);

    std::vector<CmQueueRT*>& GetQueue();

    CSync* GetSurfaceLock();

    CSync* GetSurfaceCreationLock();

    CSync* GetProgramKernelLock();

    CSync* GetQueueLock();

    int32_t GetJITCompileFnt(pJITCompile &fJITCompile);

    int32_t GetFreeBlockFnt(pFreeBlock &fFreeBlock);

    int32_t GetJITVersionFnt(pJITVersion &fJITVersion);

    int32_t LoadJITDll();

    int32_t LoadPredefinedCopyKernel(CmProgram* &pProgram);

    int32_t LoadPredefinedInitKernel(CmProgram* &pProgram);

    //GTPIN related interfaces
    bool CheckGTPinEnabled();

#if USE_EXTENSION_CODE
    int32_t EnableGTPin(CmBufferUP *pBufferUP0,
                        CmBufferUP *pBufferUP1,
                        CmBufferUP *pBufferUP2,
                        char *pGTPinCom,
                        GTPIN_INVOKE_STRUCT *pInvokeStruct,
                        bool reserveRegisters);

    CmGTPin *GetGTPin();
#endif

    int32_t GetGenStepInfo(char* &stepinfostr);

    int32_t GetCapsInternal(void *pCaps, uint32_t *puSize);

    int32_t RegisterSyncEvent(void *SyncEventHandle);

    int32_t GetOSSyncEventHandle(void* &hOSSyncEvent);

    int32_t Acquire();

    int32_t Release();

    int32_t GetPrintBufferIndex(SurfaceIndex* &pIndex) const;

    bool IsPrintEnable() const;

    bool IsVtuneLogOn() const;

    int32_t GetPrintBufferMem(unsigned char* &pPrintBufferMem) const;

    int32_t ClearPrintBuffer();

    int32_t GetSurf2DLookUpEntry(uint32_t index, PCMLOOKUP_ENTRY &pLookupEntry);

    //User may provide specific Gen code during LoadProgram stage, this function is used for thin layer
    int32_t LoadProgramWithGenCode(void *pCISACode,
                                   const uint32_t uiCISACodeSize,
                                   void *pGenCode,
                                   const uint32_t uiGenCodeSize,
                                   CmProgram* &pProgram, 
                                   const char *options = nullptr);

    int32_t DestroySurfaceInPool(uint32_t &freeSurfNum);

    int32_t CreateSurface2D(PMOS_RESOURCE pMosResource,
                            bool bIsCmCreated,
                            CmSurface2D* &pSurface);

    int32_t GetSampler8x8(uint32_t index, CmSampler8x8State_RT* &pSampler8x8);

    int32_t SetVaCtxID(uint32_t vaCtxID);

    int32_t GetVaCtxID(uint32_t &vaCtxID);

    bool IsScratchSpaceDisabled();

    int32_t SetSurfaceArraySizeForAlias();

    uint32_t GetDriverStoreFlag() { return m_IsDriverStoreEnabled; }

    void SetDriverStoreFlag(uint32_t dsEnabled) 
    { m_IsDriverStoreEnabled = dsEnabled; }

    CmDynamicArray* GetKernelArray();

    uint32_t *GetKernelCount();

#if CM_LOG_ON
    std::string Log();
#endif

    int32_t RegisterCallBack(pCallBackReleaseVaSurface pCallBack);

    int32_t ReleaseVASurface(void *pVaDpy, void *pVaSurID);

    int32_t DestroyVmeSurface(SurfaceIndex* &pVmeIndex);

protected:
    int32_t Initialize(MOS_CONTEXT *pUmdContext);

    int32_t InitializeOSSpecific(MOS_CONTEXT *pUmdContext);

    int32_t CreateAuxDevice(MOS_CONTEXT *pUmdContext);

    int32_t DestroyAuxDevice();

    int32_t DestroyQueue(CmQueueRT* &pQueue);

    int32_t RegisterSamplerState(const CM_SAMPLER_STATE &sampleState,
                                 uint32_t &index);

    int32_t RegisterSamplerStateEx(const CM_SAMPLER_STATE_EX &sampleState,
                                   uint32_t &index);

    int32_t UnregisterSamplerState(uint32_t index);

    int32_t 
    RegisterSampler8x8State(const CM_SAMPLER_8X8_DESCR &sampler8x8State,
                            uint32_t &index);

    int32_t UnregisterSampler8x8State(uint32_t index);

    int32_t GetMaxValueFromCaps(CM_HAL_MAX_VALUES &MaxValues,
                                CM_HAL_MAX_VALUES_EX &MaxValuesEx);

    int32_t InitDevCreateOption(CM_HAL_CREATE_PARAM &DevCreateParam,
                                uint32_t DevCreateOption);

    int32_t ReadVtuneProfilingFlag();


    CmDeviceRT(uint32_t DevCreateOption);

    ~CmDeviceRT();

    void ConstructOSSpecific(uint32_t devCreateOption);

    void DestructCommon();

    CM_RETURN_CODE GetCapsInternalOSSpecific(PCM_QUERY_CAPS pQueryCaps);

    int32_t QuerySurface2DFormats(void *pCapValue, uint32_t &capValueSize);

    inline bool IsMediaResetNeeded(uint32_t options) 
    {
        UNUSED(options);
        return false; 
    }

    inline HANDLE QueryRegHandleInternal(PCM_HAL_STATE pCmHalState)
    { 
        UNUSED(pCmHalState);
        return (HANDLE)nullptr;
    }

    CM_RETURN_CODE QueryGPUInfoInternal(PCM_QUERY_CAPS pQueryCaps);

    CM_RETURN_CODE QuerySurface2DFormatsInternal(PCM_QUERY_CAPS pQueryCaps);

    int32_t FlushPrintBufferInternal(const char *filename);

    MOS_CONTEXT *m_pUmdContext;

    VAContextID m_VaCtxID;

    void *m_pAccelData;    // Pointer to the private data used by the acceleration service

    uint32_t m_AccelSize;  // Size of the private data

    CM_HAL_MAX_VALUES m_HalMaxValues;

    CM_HAL_MAX_VALUES_EX m_HalMaxValuesEx;

    CmSurfaceManager *m_pSurfaceMgr;

    std::vector<CmQueueRT*> m_pQueue;

    CmDynamicArray m_ProgramArray;

    uint32_t m_ProgramCount;

    CmDynamicArray m_KernelArray;

    uint32_t m_KernelCount;

    CmDynamicArray m_Sampler8x8Array;

    CmDynamicArray m_SamplerArray;

    CmDynamicArray m_ThreadSpaceArray;

    uint32_t m_ThreadSpaceCount;

    CmDynamicArray m_VeboxArray;

    uint32_t m_VeboxCount;

    HMODULE m_hJITDll;

    pJITCompile m_fJITCompile;

    pFreeBlock m_fFreeBlock;

    pJITVersion m_fJITVersion;

    uint32_t m_DDIVersion;

    uint32_t m_Platform;

    uint32_t m_CmDeviceRefCount;

    CmProgram *m_pGPUCopyKernelProgram;

    CmProgram *m_pSurfInitKernelProgram;

    void  *m_OSSyncEvent;   //KMD Notification

#if USE_EXTENSION_CODE
    CmGTPin *m_pGTPin;
#endif

    // synchronization objects
    CSync m_CriticalSection_Program_Kernel;

    CSync m_CriticalSection_Surface;

    CSync m_CriticalSection_ReadWriteSurface2D;

    CSync m_CriticalSection_Sampler;

    CSync m_CriticalSection_Sampler8x8;

    CSync m_CriticalSection_VmeState;

    CSync m_CriticalSection_ThreadSpace;

    CSync m_CriticalSection_DeviceRefCount;

    CSync m_CriticalSection_ThreadGroupSpace;

    CSync m_CriticalSection_Task;

    CSync m_CriticalSection_Vebox;

    CSync m_CriticalSection_Queue;

    pCallBackReleaseVaSurface  m_pfnReleaseVaSurface;

public:
    static CSync GlobalCriticalSection_Surf2DUserDataLock;

protected:
    unsigned char *m_pPrintBufferMem;

    CmBufferUP *m_pPrintBufferUP;

    bool m_IsPrintEnable;

    size_t m_PrintBufferSize;

    SurfaceIndex *m_PrintBufferIndex;

    CmDynamicArray m_ThreadGroupSpaceArray;

    uint32_t m_ThreadGroupSpaceCount;

    CmDynamicArray m_TaskArray;

    uint32_t m_TaskCount;

    CM_HAL_CREATE_PARAM m_DevCreateOption;

    uint32_t m_nGPUFreqOriginal;

    uint32_t m_nGPUFreqMin;

    uint32_t m_nGPUFreqMax;

    bool m_bVtuneOn;

    uint32_t m_IsDriverStoreEnabled;

private:
    CmDeviceRT(const CmDeviceRT& other);

    CmDeviceRT& operator= (const CmDeviceRT& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMDEVICERT_H_
