/*===================== begin_copyright_notice ==================================
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
//! \file      cm_device_rt_base.h
//! \brief     Contains CmDeviceRTBase declarations.
//!

#ifndef MEDIADRIVER_COMMON_CM_CMDEVICERT_BASE_H_
#define MEDIADRIVER_COMMON_CM_CMDEVICERT_BASE_H_

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
class CmSurfaceManager;
class CmQueue;
class CmQueueRT;
class CmKernel;
class CmThreadSpace;
class CmThreadGroupSpace;
class CmVebox;
class CmSampler;
class CmEvent;
class CmTask;
class CmQueueBase;
class CmProgram;
class CmBuffer;
class CmBufferUP;
class CmBufferSVM;
class CmBufferStateless;
class CmSurface2D;
class CmSurface2DUP;
class CmSurface2DUPRT;
class CmSurface3D;
class CmSampler8x8;
class CmSampler8x8State_RT;

//! \brief    Class CmDeviceRTBase definitions
class CmDeviceRTBase: public CmDevice
{
public:

    CM_RT_API int32_t CreateBuffer(uint32_t size,
                                   CmBuffer* & pSurface);

    CM_RT_API int32_t CreateBuffer(PMOS_RESOURCE pMosResource,
                                   CmBuffer* & pSurface);

    CM_RT_API virtual  int32_t CreateSurface2D(uint32_t width,
                                      uint32_t height,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface2D* & pSurface) = 0;

    CM_RT_API virtual  int32_t CreateSurface2D(PMOS_RESOURCE pMosResource,
                                      CmSurface2D* & pSurface) = 0;

    CM_RT_API int32_t CreateSurface3D(uint32_t width,
                                      uint32_t height,
                                      uint32_t depth,
                                      CM_SURFACE_FORMAT format,
                                      CmSurface3D* & pSurface);

    CM_RT_API int32_t DestroySurface(CmBuffer* & pSurface);

    CM_RT_API int32_t DestroySurface(CmSurface2D* & pSurface);

    CM_RT_API int32_t DestroySurface(CmSurface3D* & pSurface);

    CM_RT_API int32_t CreateQueue(CmQueue* & pQueue);

    CM_RT_API int32_t LoadProgram(void* pCommonISACode,
                                  const uint32_t size,
                                  CmProgram*& pProgram,
                                  const char* options = nullptr);

    CM_RT_API int32_t CreateKernel(CmProgram* pProgram,
                                   const char* kernelName,
                                   CmKernel* & pKernel,
                                   const char* options = nullptr);

    CM_RT_API virtual int32_t CreateKernel(CmProgram* pProgram,
                                           const char* kernelName,
                                           const void * fncPnt,
                                           CmKernel* & pKernel,
                                           const char* options = nullptr)
    { return CM_NOT_IMPLEMENTED; }

    CM_RT_API int32_t
    CreateSampler(const CM_SAMPLER_STATE & sampleState,
                  CmSampler* & pSampler);

    CM_RT_API int32_t DestroyKernel(CmKernel*& pKernel);

    CM_RT_API int32_t DestroySampler(CmSampler*& pSampler);

    CM_RT_API int32_t DestroyProgram(CmProgram* & pProgram);

    CM_RT_API int32_t DestroyThreadSpace(CmThreadSpace* & pTS);

    CM_RT_API int32_t CreateTask(CmTask *& pTask);

    CM_RT_API int32_t DestroyTask(CmTask*& pTask);

    CM_RT_API int32_t GetCaps(CM_DEVICE_CAP_NAME capName,
                              uint32_t & capValueSize,
                              void* pCapValue);

    CM_RT_API int32_t CreateThreadSpace(uint32_t width,
                                        uint32_t height,
                                        CmThreadSpace* & pTS);

    CM_RT_API int32_t CreateBufferUP(uint32_t size,
                                     void* pSystMem,
                                     CmBufferUP* & pSurface);

    CM_RT_API int32_t DestroyBufferUP(CmBufferUP* & pSurface);

    CM_RT_API int32_t ForceDestroyBufferUP(CmBufferUP* & pSurface);

    CM_RT_API int32_t GetSurface2DInfo(uint32_t width,
                                       uint32_t height,
                                       CM_SURFACE_FORMAT format,
                                       uint32_t & pitch,
                                       uint32_t & physicalSize);

    CM_RT_API int32_t CreateSurface2DUP(uint32_t width,
                                        uint32_t height,
                                        CM_SURFACE_FORMAT format,
                                        void* pSysMem,
                                        CmSurface2DUP* & pSurface);

    CM_RT_API int32_t DestroySurface2DUP(CmSurface2DUP* & pSurface);

    CM_RT_API int32_t CreateVmeSurfaceG7_5(CmSurface2D* pCurSurface,
                                           CmSurface2D** pForwardSurface,
                                           CmSurface2D** pBackwardSurface,
                                           const uint32_t surfaceCountForward,
                                           const uint32_t surfaceCountBackward,
                                           SurfaceIndex* & pVmeIndex);

    CM_RT_API int32_t DestroyVmeSurfaceG7_5(SurfaceIndex* & pVmeIndex);

    CM_RT_API int32_t CreateSampler8x8(const CM_SAMPLER_8X8_DESCR  & smplDescr,
                                       CmSampler8x8 *& psmplrState);

    CM_RT_API int32_t DestroySampler8x8(CmSampler8x8*& ps8x8State);

    CM_RT_API int32_t
    CreateSampler8x8Surface(CmSurface2D* p2DSurface,
                            SurfaceIndex* & pDIIndex,
                            CM_SAMPLER8x8_SURFACE surf_type,
                            CM_SURFACE_ADDRESS_CONTROL_MODE mode);

    CM_RT_API int32_t DestroySampler8x8Surface(SurfaceIndex* & pDIIndex);

    CM_RT_API int32_t CreateThreadGroupSpace(uint32_t thrdSpaceWidth,
                                             uint32_t thrdSpaceHeight,
                                             uint32_t grpSpaceWidth,
                                             uint32_t grpSpaceHeight,
                                             CmThreadGroupSpace*& pTGS);

    CM_RT_API int32_t DestroyThreadGroupSpace(CmThreadGroupSpace*& pTGS);

    CM_RT_API int32_t SetL3Config(const L3ConfigRegisterValues *l3_c);

    CM_RT_API int32_t SetSuggestedL3Config(L3_SUGGEST_CONFIG l3_s_c);

    CM_RT_API int32_t SetCaps(CM_DEVICE_CAP_NAME capName,
                              size_t capValueSize,
                              void* pCapValue);

    CM_RT_API int32_t
    CreateSamplerSurface2D(CmSurface2D* p2DSurface,
                           SurfaceIndex* & pSamplerSurfaceIndex);

    CM_RT_API int32_t
    CreateSamplerSurface3D(CmSurface3D* p3DSurface,
                           SurfaceIndex* & pSamplerSurfaceIndex);

    CM_RT_API int32_t
    DestroySamplerSurface(SurfaceIndex* & pSamplerSurfaceIndex);

    CM_RT_API int32_t
    InitPrintBuffer(size_t size = CM_DEFAULT_PRINT_BUFFER_SIZE);

    CM_RT_API int32_t FlushPrintBuffer();

    CM_RT_API int32_t CreateVebox(CmVebox* & pVebox);

    CM_RT_API int32_t DestroyVebox(CmVebox* & pVebox);

    CM_RT_API int32_t CreateBufferSVM(uint32_t size,
                                      void* & pSystMem,
                                      uint32_t access_flag,
                                      CmBufferSVM* & pBufferSVM);

    CM_RT_API int32_t DestroyBufferSVM(CmBufferSVM* & pBufferSVM);

    CM_RT_API int32_t
    CreateSamplerSurface2DUP(CmSurface2DUP* p2DUPSurface,
                             SurfaceIndex* & pSamplerSurfaceIndex);

    CM_RT_API int32_t CloneKernel(CmKernel* &pKernelDest, CmKernel*pKernelSrc);

    CM_RT_API int32_t CreateSurface2DAlias(CmSurface2D* p2DSurface,
                                           SurfaceIndex* &aliasSurfaceIndex);

    CM_RT_API int32_t
    CreateHevcVmeSurfaceG10(CmSurface2D* pCurSurface,
                            CmSurface2D** pForwardSurface,
                            CmSurface2D** pBackwardSurface,
                            const uint32_t surfaceCountForward,
                            const uint32_t surfaceCountBackward,
                            SurfaceIndex* & pVmeIndex);

    CM_RT_API int32_t
    DestroyHevcVmeSurfaceG10(SurfaceIndex* & pVmeIndex);

    CM_RT_API int32_t CreateSamplerEx(const CM_SAMPLER_STATE_EX & sampleState,
                                      CmSampler* & pSampler);

    CM_RT_API int32_t FlushPrintBufferIntoFile(const char *filename);

    CM_RT_API int32_t CreateThreadGroupSpaceEx(uint32_t thrdSpaceWidth,
                                               uint32_t thrdSpaceHeight,
                                               uint32_t thrdSpaceDepth,
                                               uint32_t grpSpaceWidth,
                                               uint32_t grpSpaceHeight,
                                               uint32_t grpSpaceDepth,
                                               CmThreadGroupSpace*& pTGS);

    CM_RT_API int32_t
    CreateSampler8x8SurfaceEx(
    CmSurface2D* p2DSurface,
    SurfaceIndex* & pDIIndex,
    CM_SAMPLER8x8_SURFACE surf_type = CM_VA_SURFACE,
    CM_SURFACE_ADDRESS_CONTROL_MODE address_control = CM_SURFACE_CLAMP,
    CM_FLAG* pFlag = nullptr);

    CM_RT_API int32_t
    CreateSamplerSurface2DEx(CmSurface2D* p2DSurface,
                             SurfaceIndex* & pSamplerSurfaceIndex,
                             CM_FLAG* pFlag = nullptr);

    CM_RT_API int32_t CreateBufferAlias(CmBuffer *pBuffer,
                                        SurfaceIndex* &pAliasIndex);

    CM_RT_API int32_t
    SetVmeSurfaceStateParam(SurfaceIndex* pVmeIndex,
                            CM_VME_SURFACE_STATE_PARAM *pSSParam);

    CM_RT_API int32_t GetVISAVersion(uint32_t& majorVersion,
                                     uint32_t& minorVersion);

    CM_RT_API int32_t
    CreateQueueEx(CmQueue *&pQueue,
                  CM_QUEUE_CREATE_OPTION QueueCreateOption
                  = CM_DEFAULT_QUEUE_CREATE_OPTION);

    CM_RT_API int32_t UpdateBuffer(PMOS_RESOURCE mosResource, CmBuffer* &surface,
                                   MOS_HW_RESOURCE_DEF mosUsage = MOS_CM_RESOURCE_USAGE_SurfaceState);

    CM_RT_API int32_t UpdateSurface2D(PMOS_RESOURCE mosResource, CmSurface2D* &surface,
                                      MOS_HW_RESOURCE_DEF mosUsage = MOS_CM_RESOURCE_USAGE_SurfaceState);

    CM_RT_API int32_t
    CreateSampler8x8SurfaceFromAlias(
        CmSurface2D *originalSurface,
        SurfaceIndex *aliasIndex,
        CM_SURFACE_ADDRESS_CONTROL_MODE addressControl,
        SurfaceIndex* &sampler8x8SurfaceIndex);

    CM_RT_API int32_t CreateBufferStateless(size_t size,
                                            uint32_t option,
                                            void *sysMem,
                                            CmBufferStateless *&bufferStateless);

    CM_RT_API int32_t DestroyBufferStateless(CmBufferStateless* & bufferStateless);

    void* GetAccelData(){ return m_accelData; }

    MOS_CONTEXT* GetUMDCtx(){ return m_mosContext; }

    uint32_t GetAccelsize(){ return m_accelSize; }

    int32_t GetHalMaxValues(CM_HAL_MAX_VALUES* & pHalMaxValues,
                            CM_HAL_MAX_VALUES_EX* & pHalMaxValuesEx);

    int32_t GetGenPlatform(uint32_t &platform);

    void Sampler8x8CoefficientFormatTransform(
         CM_AVS_INTERNEL_NONPIPLINED_STATE* dst_avs_state,
         CM_AVS_NONPIPLINED_STATE* src_avs_state);

    int32_t GetSurfaceManager(CmSurfaceManager* &pSurfaceMgr);

    std::vector<CmQueueRT *> &GetQueue();

    CSync* GetSurfaceLock();

    CSync* GetSurfaceCreationLock();

    CSync* GetProgramKernelLock();

    CSync* GetQueueLock();

    int32_t LoadPredefinedCopyKernel(CmProgram*& pProgram);

    int32_t LoadPredefinedInitKernel(CmProgram*& pProgram);

    bool CheckGTPinEnabled();

#if USE_EXTENSION_CODE
    int32_t EnableGTPin(CmBufferUP* pBufferUP0,
                        CmBufferUP* pBufferUP1,
                        CmBufferUP* pBufferUP2,
                        char * pGTPinCom,
                        GTPIN_INVOKE_STRUCT* pInvokeStruct,
                        bool reserveRegisters);
    CmGTPin *GetGTPin();
#endif

    int32_t GetGenStepInfo(char*& stepinfostr);

    int32_t GetCapsInternal(void *pCaps, uint32_t *puSize);

    int32_t Acquire();

    int32_t Release();

    int32_t GetPrintBufferIndex(SurfaceIndex *& pIndex) const;

    bool IsPrintEnable() const;

    bool IsVtuneLogOn() const;

    int32_t GetPrintBufferMem(unsigned char *& pPrintBufferMem) const;

    int32_t GetSurf2DLookUpEntry(uint32_t index,
                                 PCMLOOKUP_ENTRY &pLookupEntry);

    int32_t DestroySurfaceInPool(uint32_t &freeSurfNum);

    virtual int32_t CreateSurface2D(PMOS_RESOURCE pMosResource,
                            bool bIsCmCreated,
                            CmSurface2D* & pSurface) = 0;

    int32_t GetSampler8x8(uint32_t index, CmSampler8x8State_RT *&pSampler8x8);

    bool IsScratchSpaceDisabled();

    int32_t SetSurfaceArraySizeForAlias();

    uint32_t GetDriverStoreFlag() { return m_isDriverStoreEnabled; }

    void SetDriverStoreFlag(uint32_t dsEnabled)
    {
        m_isDriverStoreEnabled = dsEnabled;
    }

    CmDynamicArray* GetKernelArray();

    uint32_t *GetKernelCount();

#if CM_LOG_ON
    std::string Log();
#endif

    int32_t DestroyVmeSurface(SurfaceIndex *& pVmeIndex);

    int32_t CreatePrintBuffer();

    CmNotifierGroup* GetNotifiers() {return m_notifierGroup;}

    CM_HAL_CREATE_PARAM &GetCmHalCreateOption() {return m_cmHalCreateOption;}

    inline bool HasGpuCopyKernel() {return m_hasGpuCopyKernel; }

    inline bool HasGpuInitKernel() {return m_hasGpuInitKernel; }

    // Num of kernels included in CmProgram Loaded by this device
    inline uint32_t& KernelsLoaded() {return m_kernelsLoaded; }

    virtual int32_t GetJITCompileFnt(pJITCompile &fJITCompile) = 0;

    virtual int32_t GetJITCompileFntV2(pJITCompile_v2 &fJITCompile_v2) = 0;

    virtual int32_t GetFreeBlockFnt(pFreeBlock &fFreeBlock) = 0;

    virtual int32_t GetJITVersionFnt(pJITVersion &fJITVersion) = 0;

    virtual int32_t LoadJITDll() = 0;

    virtual HANDLE QueryRegHandleInternal(PCM_HAL_STATE pCmHalState) = 0;

    static CSync m_globalCriticalSectionSurf2DUserDataLock;

protected:
    int32_t Initialize(MOS_CONTEXT *pUmdContext);
    int32_t DestroyQueue(CmQueueRT* & pQueue);

    int32_t RegisterSamplerState(const CM_SAMPLER_STATE& sampleState,
                                 uint32_t& index);

    int32_t RegisterSamplerStateEx(const CM_SAMPLER_STATE_EX& sampleState,
                                   uint32_t& index);

    int32_t UnregisterSamplerState(uint32_t index);

    int32_t RegisterSampler8x8State(const CM_SAMPLER_8X8_DESCR & sampler8x8State,
                                    uint32_t& index);

    int32_t UnregisterSampler8x8State(uint32_t index);

    int32_t GetMaxValueFromCaps(CM_HAL_MAX_VALUES &MaxValues,
        CM_HAL_MAX_VALUES_EX &MaxValuesEx);

    int32_t InitDevCreateOption(CM_HAL_CREATE_PARAM & DevCreateParam,
                                uint32_t DevCreateOption);

    CmDeviceRTBase(uint32_t DevCreateOption);

    ~CmDeviceRTBase() {};

    void DestructCommon();

    inline bool IsMediaResetNeeded(uint32_t options)
    {
        return (options & CM_DEVICE_CONFIG_MEDIA_RESET_ENABLE) ? true : false;
    }

    virtual int32_t ReadVtuneProfilingFlag() = 0;

    virtual void ConstructOSSpecific(uint32_t devCreateOption) = 0;

    virtual int32_t QuerySurface2DFormats(void *pCapValue,
        uint32_t & capValueSize) = 0;

    virtual int32_t InitializeOSSpecific(MOS_CONTEXT *pUmdContext) = 0;

    virtual CM_RETURN_CODE QueryGPUInfoInternal(PCM_QUERY_CAPS pQueryCaps) = 0;

    virtual CM_RETURN_CODE QuerySurface2DFormatsInternal(PCM_QUERY_CAPS pQueryCaps) = 0;

    int32_t FlushPrintBufferInternal(const char *filename);

    MOS_CONTEXT    *m_mosContext;

    void* m_accelData;          // Pointer to the private data used by the acceleration service

    uint32_t m_accelSize;           // Size of the private data

    CM_HAL_MAX_VALUES m_halMaxValues;

    CM_HAL_MAX_VALUES_EX m_halMaxValuesEx;

    CmSurfaceManager* m_surfaceMgr;

    std::vector<CmQueueRT *> m_queue;

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

    CmProgram* m_gpuCopyKernelProgram;

    CmProgram* m_surfInitKernelProgram;

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

    std::list<uint8_t *> m_printBufferMems;

    std::list<CmBufferUP *> m_printBufferUPs;

    bool           m_isPrintEnabled;

    size_t         m_printBufferSize;

    CmDynamicArray m_threadGroupSpaceArray;

    uint32_t       m_threadGroupSpaceCount;

    CmDynamicArray m_taskArray;

    uint32_t       m_taskCount;

    CM_HAL_CREATE_PARAM   m_cmHalCreateOption;

    uint32_t       m_nGPUFreqOriginal;

    uint32_t       m_nGPUFreqMin;

    uint32_t       m_nGPUFreqMax;

    bool           m_vtuneOn;

    uint32_t       m_isDriverStoreEnabled;

    CmNotifierGroup *m_notifierGroup;

    bool           m_hasGpuCopyKernel;

    bool           m_hasGpuInitKernel;

    uint32_t       m_kernelsLoaded;

    bool           m_preloadKernelEnabled;

    static const uint32_t m_maxPrintBuffer;
private:
    CmDeviceRTBase(const CmDeviceRTBase& other);

    CmDeviceRTBase& operator= (const CmDeviceRTBase& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_COMMON_CM_CMDEVICERT_BASE_H_
