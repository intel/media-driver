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
//! \file      cm_kernel_rt.h
//! \brief     Contains CmKernelRT declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELRT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELRT_H_

#include "cm_kernel.h"
#include "cm_hal.h"
#include "cm_log.h"

enum SURFACE_KIND
{
    DATA_PORT_SURF,
    SAMPLER_SURF,
};

struct SURFACE_ARRAY_ARG
{
    uint16_t argKindForArray;  // record each arg kind in array, used for surface array
    uint32_t addressModeForArray;  // record each arg address control mode for media sampler in surface array
};

struct CM_ARG
{
    uint16_t unitKind; // value is of type CM_ARG_KIND
    uint16_t unitKindOrig; // used to restore unitKind when reset

    uint16_t index;
    SURFACE_KIND surfaceKind;

    uint32_t unitCount; // 1 for for per kernel arg ; thread # for per thread arg

    uint16_t unitSize; // size of arg in byte
    uint16_t unitSizeOrig; // used to restore unitSize when reset

    uint16_t unitOffsetInPayload; // offset relative to R0 in payload
    uint16_t unitOffsetInPayloadOrig; // used to restore unitOffsetInPayload in adding move instruction for CURBE
    bool isDirty;      // used to indicate if its value be changed
    bool isSet;        // used to indicate if this argument is set correctly
    uint32_t nCustomValue;  // CM defined value for special argument kind

    uint32_t aliasIndex;    // CmSurface2D alias index
    bool aliasCreated; // whether or not alias was created for this argument

    bool isNull;       // used to indicate if this is a null surface

    uint32_t unitVmeArraySize; // number of Vme surfaces in surface array

    // pointer to the arg values. the size is unitCount * unitSize
    union
    {
        uint8_t *value;
        int32_t *intValue;
        uint32_t *uintValue;
        float  *floatValue;
    };

    uint16_t *surfIndex;
    SURFACE_ARRAY_ARG *surfArrayArg; // record each arg kind and address control mode for media sampler in surface array
    bool isStatelessBuffer;
    CM_ARG()
    {
        unitKind = 0;
        unitCount = 0;
        unitSize = 0;
        unitOffsetInPayload = 0;
        value = nullptr;
        isDirty = false;
        isNull = false;
        unitVmeArraySize = 0;
        surfIndex = nullptr;
        aliasIndex = 0;
        unitOffsetInPayloadOrig = 0;
        isSet = false;
        index = 0;
        unitKindOrig = 0;
        nCustomValue = 0;
        surfaceKind = DATA_PORT_SURF;
        unitSizeOrig = 0;
        surfArrayArg = nullptr;
        aliasCreated = false;
        isStatelessBuffer = false;
    }
};

enum CM_KERNEL_INTERNAL_ARG_TYPE
{
    CM_KERNEL_INTERNEL_ARG_PERKERNEL = 0,
    CM_KERNEL_INTERNEL_ARG_PERTHREAD = 1
};

struct CM_KERNEL_INFO;
class CmExecutionAdv;

namespace CMRT_UMD
{
class CmDeviceRT;
class CmKernelData;
class CmThreadSpaceRT;
class CmSurfaceVme;
class CmSurface;
class CmSurfaceManager;
class CmProgramRT;
class CmDynamicArray;

class CmMovInstConstructor
{
public:
    CmMovInstConstructor() {}
    virtual ~CmMovInstConstructor() {}

    virtual CM_RETURN_CODE SetInstDistanceConfig(uint32_t size, uint32_t renderGen)
    {
        // not implemented in currect platforms
        // reserved for future platforms
        return CM_NOT_IMPLEMENTED;
    }

    virtual uint32_t ConstructObjMovs(uint32_t dstOffset,
                             uint32_t srcOffset,
                             uint32_t size,
                             CmDynamicArray &movInsts,
                             uint32_t index,
                             bool isBdw,
                             bool isHwDebug);
    
};

//*-----------------------------------------------------------------------------
//! CM Kernel
//*-----------------------------------------------------------------------------
class CmKernelRT: public CmKernel
{
public:
    static int32_t Create(CmDeviceRT *device,
                          CmProgramRT *program,
                          const char *kernelName,
                          uint32_t kernelIndex,
                          uint32_t kernelSeqNum,
                          CmKernelRT *&kernel,
                          const char *options);

    static int32_t Destroy(CmKernelRT *&kernel, CmProgramRT *&program);

    int32_t GetThreadCount(uint32_t &count);

    CM_RT_API int32_t SetThreadCount(uint32_t count);

    CM_RT_API int32_t SetKernelArg(uint32_t index,
                                   size_t size,
                                   const void *value);

    CM_RT_API virtual int32_t SetKernelArgPointer(uint32_t index,
                                                  size_t size,
                                                  const void *value);

    CM_RT_API int32_t SetThreadArg(uint32_t threadId,
                                   uint32_t index,
                                   size_t size,
                                   const void *value);

    CM_RT_API int32_t SetStaticBuffer(uint32_t index, const void *value);

    CM_RT_API int32_t SetSurfaceBTI(SurfaceIndex *surface, uint32_t bti);

    CM_RT_API int32_t AssociateThreadSpace(CmThreadSpace *&threadSpace);

    CM_RT_API int32_t AssociateThreadGroupSpace(CmThreadGroupSpace *&threadGroupSpace);

    CM_RT_API int32_t SetSamplerBTI(SamplerIndex *sampler, uint32_t nIndex);

    CM_RT_API int32_t DeAssociateThreadSpace(CmThreadSpace *&threadSpace);

    CM_RT_API int32_t DeAssociateThreadGroupSpace(CmThreadGroupSpace *&threadGroupSpace);

    CM_RT_API int32_t QuerySpillSize(uint32_t &spillMemorySize);

    CMRT_UMD_API int32_t GetBinary(std::vector<char> &binary);

    CMRT_UMD_API int32_t ReplaceBinary(std::vector<char> &binary);

    CMRT_UMD_API int32_t ResetBinary();

    int32_t GetArgs(CM_ARG *&arg);

    int32_t GetArgCount(uint32_t &argCount);

    int32_t GetCurbeEnable(bool &b);

    int32_t SetCurbeEnable(bool b);

    int32_t GetSizeInCurbe(uint32_t &size);

    uint32_t GetAlignedCurbeSize(uint32_t value);

    int32_t GetCmDevice(CmDeviceRT *&);

    int32_t GetCmProgram(CmProgramRT *&);

    int32_t GetSizeInPayload(uint32_t &size);

    int32_t CreateKernelData(CmKernelData *&kernelData,
                             uint32_t &kernelDataSize,
                             const CmThreadSpaceRT *threadSpace);

    int32_t CreateKernelData(CmKernelData *&kernelData,
                             uint32_t &kernelDataSize,
                             const CmThreadGroupSpace *threadGroupSpace);

    char *GetName();

    int32_t SetIndexInTask(uint32_t index);

    uint32_t GetIndexInTask();

    int32_t SetAssociatedToTSFlag(bool b);

    bool IsThreadArgExisted();

    uint32_t GetKernelIndex();

    int32_t GetThreadSpace(CmThreadSpaceRT *&threadSpace)
    {
        threadSpace = m_threadSpace;
        return CM_SUCCESS;
    }

    int32_t GetThreadGroupSpace(CmThreadGroupSpace *&threadGroupSpace)
    {
        threadGroupSpace = m_threadGroupSpace;
        return CM_SUCCESS;
    }

    int32_t SetAdjustedYCoord(uint32_t value)
    {
        m_adjustScoreboardY = value;
        return CM_SUCCESS;
    }

    int32_t GetAdjustedYCoord() { return m_adjustScoreboardY; }

    uint32_t GetSLMSize();

    uint32_t GetSpillMemUsed();

    int32_t Acquire();

    int32_t SafeRelease();

    int32_t CollectKernelSurface();

    int32_t GetKernelSurfaces(bool *&surfArray);

    int32_t ResetKernelSurfaces();

    int32_t CalculateKernelSurfacesNum(uint32_t &kernelSurfaceNum,
                                       uint32_t &neededBTEntryNum);

    uint32_t GetKernelGenxBinarySize();

    int32_t ReleaseKernelData(CmKernelData *&kernelData);

    int32_t AcquireKernelData(CmKernelData *&kernelData);

    int32_t CloneKernel(CmKernelRT *&kernelOut, uint32_t id);

    void SetAsClonedKernel(uint32_t cloneKernelID);

    bool GetCloneKernelID(uint32_t &cloneKernelID);

    void SetHasClones();

    uint32_t GetMaxSurfaceIndexAllocated()
    { return m_maxSurfaceIndexAllocated; }

    int UpdateSamplerHeap(CmKernelData *kernelData);

#if CM_LOG_ON
    std::string Log();
#endif

    void SurfaceDump(uint32_t kernelNumber, int32_t taskId);

protected:
    friend CmExecutionAdv;
    CmKernelRT(CmDeviceRT *device,
               CmProgramRT *program,
               uint32_t kernelIndex,
               uint32_t kernelSeqNum);

    virtual ~CmKernelRT();

    int32_t SetArgsInternal(CM_KERNEL_INTERNAL_ARG_TYPE nArgType,
                            uint32_t index,
                            size_t size,
                            const void *value,
                            uint32_t nThreadID = 0);

    virtual int32_t Initialize(const char *kernelName, const char *options);

    int32_t DestroyArgs();

    int32_t Reset();

    int32_t IsKernelDataReusable(CmThreadSpaceRT *threadSpace);

    int32_t CreateKernelArgDataGroup(uint8_t *&data, uint32_t value);

    int32_t CreateMovInstructions(uint32_t &movInstNum,
                                  uint8_t *&codeDst,
                                  CM_ARG *tempArgs,
                                  uint32_t numArgs);

    int32_t CalcKernelDataSize(uint32_t movInstNum,
                               uint32_t numArgs,
                               uint32_t argSize,
                               uint32_t &totalKernelDataSize);

    int32_t GetArgCountPlusSurfArray(uint32_t &argSize, uint32_t &argCountPlus);

    int32_t CreateKernelDataInternal(CmKernelData *&kernelData,
                                     uint32_t &kernelDataSize,
                                     const CmThreadSpaceRT *threadSpace);

    int32_t CreateKernelDataInternal(CmKernelData *&kernelData,
                                     uint32_t &kernelDataSize,
                                     const CmThreadGroupSpace *threadGroupSpace);

    int32_t UpdateKernelData(CmKernelData *kernelData,
                             const CmThreadSpaceRT *threadSpace);

    int32_t UpdateKernelData(CmKernelData *kernelData,
                             const CmThreadGroupSpace *threadGroupSpace);

    int32_t CreateThreadArgData(PCM_HAL_KERNEL_ARG_PARAM kernelArg,
                                uint32_t threadArgIndex,
                                CmThreadSpaceRT *threadSpace,
                                CM_ARG *cmArgs);

    int32_t UpdateLastKernelData(CmKernelData *&kernelData);

    int32_t CreateKernelIndirectData(
        PCM_HAL_INDIRECT_DATA_PARAM halIndirectData);

    int32_t CreateThreadSpaceParam(
        PCM_HAL_KERNEL_THREADSPACE_PARAM kernelThreadSpaceParam,
        CmThreadSpaceRT *threadSpace);

    int32_t CreateTempArgs(uint32_t numArgs, CM_ARG *&tempArgs);

    int32_t SortThreadSpace(CmThreadSpaceRT *threadSpace);

    int32_t CleanArgDirtyFlag();

    bool IsBatchBufferReusable(CmThreadSpaceRT *taskThreadSpace);

    bool IsPrologueDirty();

    void DumpKernelData(CmKernelData *kernelData);

    int32_t
    UpdateKernelDataGlobalSurfaceInfo(PCM_HAL_KERNEL_PARAM halKernelParam);

    CM_ARG_KIND SurfTypeToArgKind(CM_ENUM_CLASS_TYPE surfType);

    int32_t AcquireKernelProgram();

    int32_t SetArgsVme(CM_KERNEL_INTERNAL_ARG_TYPE nArgType,
                       uint32_t argIndex,
                       const void *value,
                       uint32_t nThreadID);

    int32_t SetArgsSingleVme(CmSurfaceVme *vmeSurface,
                             uint8_t *vmeArgValueArray,
                             uint16_t *cmSufacesArray);

    int32_t GetVmeSurfaceIndex(uint32_t *vmeIndexArray,
                               uint32_t *vmeCmIndexArray,
                               uint32_t index,
                               uint32_t *outputValue);

    CmSurface *GetSurfaceFromSurfaceArray(SurfaceIndex *value,
                                          uint32_t indexSurfaceArray);

    void ArgLog(std::ostringstream &oss,
                uint32_t index,
                CM_ARG arg);

    int32_t CreateKernelImplicitArgDataGroup(uint8_t *&data, uint32_t size);

    int32_t SearchAvailableIndirectSurfInfoTableEntry(uint16_t kind,
                                                      uint32_t surfaceIndex,
                                                      uint32_t bti);

    int32_t SetSurfBTINumForIndirectData(CM_SURFACE_FORMAT format,
                                         CM_ENUM_CLASS_TYPE surfaceType);

    int32_t SetArgsInternalSurfArray(int32_t offset,
                                     uint32_t kernelArgIndex,
                                     int32_t surfCount,
                                     CmSurface *currentSurface,
                                     uint32_t currentSurfIndex,
                                     SurfaceIndex *value,
                                     uint32_t surfValue[],
                                     uint16_t origSurfIndex[]);

#if USE_EXTENSION_CODE
    int InitForGTPin(CmDeviceRT *device,
                     CmProgramRT *program,
                     CmKernelRT *kernel);

    int32_t
    UpdateKernelDataGTPinSurfaceInfo(PCM_HAL_KERNEL_PARAM halKernelParam);
#endif

    CmDeviceRT *m_device;
    CmSurfaceManager *m_surfaceMgr;
    CmProgramRT *m_program;
    char *m_options;
    char *m_binary;
    char *m_binaryOrig;
    uint32_t m_binarySize;
    uint32_t m_binarySizeOrig;

    uint32_t m_threadCount;
    uint32_t m_lastThreadCount;
    uint32_t m_sizeInCurbe;  //data size in CURBE
    uint32_t m_sizeInPayload;  //data size of inline data in media object or media walker commands
    uint32_t m_argCount;

    CM_ARG *m_args;
    SurfaceIndex *m_globalSurfaces[CM_GLOBAL_SURFACE_NUMBER];
    uint32_t m_globalCmIndex[CM_GLOBAL_SURFACE_NUMBER];
    CM_KERNEL_INFO *m_kernelInfo;
    uint32_t m_kernelIndexInProgram;

    bool m_curbeEnabled;
    bool m_nonstallingScoreboardEnabled;

    uint64_t m_id;  // high 32bit is kernel id (highest 16 bits used for kernel binary re-use
                    // in GSH), low 32bit is kernel data id

    uint32_t m_dirty;
    CmKernelData *m_lastKernelData;
    uint32_t m_lastKernelDataSize;

    uint32_t m_indexInTask;
    bool m_threadSpaceAssociated;  // Indicates if this kernel is associated the task threadspace
                            // (scoreboard)

    bool m_perThreadArgExists;  // Indicates if this kernel has thread arg.
    bool m_perKernelArgExists;  // Indicates if the user call SetKernelArg() to set per-kernel arg

    CmThreadSpaceRT *m_threadSpace;  // Pointer to the kernel threadspace
    uint32_t m_adjustScoreboardY;     // value to adjust Y coordinate read from r0.1 used for
                                      // EnqueueWithHints
    uint32_t m_lastAdjustScoreboardY;

    bool m_blCreatingGPUCopyKernel;  // Indicate if this is a predefined GPUCopy kernel
    bool m_blhwDebugEnable;          // Indicate if the hw debug enabled

    uint16_t m_usKernelPayloadDataSize;  // Size of kernel indirect data (in byte)
    uint8_t *m_kernelPayloadData;       // Pointer to the kernel indirect data memory

    uint16_t m_usKernelPayloadSurfaceCount;  //the surface count in kernel indirect data

    // the surface index list included in kernel indirect data
    SurfaceIndex*
    m_pKernelPayloadSurfaceArray[CM_MAX_STATIC_SURFACE_STATES_PER_BT];

    CM_INDIRECT_SURFACE_INFO
    m_IndirectSurfaceInfoArray[CM_MAX_STATIC_SURFACE_STATES_PER_BT];  // information used by driver

    uint32_t m_samplerBtiCount;
    CM_SAMPLER_BTI_ENTRY m_samplerBtiEntry[CM_MAX_SAMPLER_TABLE_SIZE];

    uint32_t m_refcount;  // reference count for kernel

    CM_HAL_MAX_VALUES *m_halMaxValues;
    CM_HAL_MAX_VALUES_EX *m_halMaxValuesEx;
    bool *m_surfaceArray;

    uint32_t m_kernelIndex;  // Kernel index in kernel array

    CmThreadGroupSpace *m_threadGroupSpace;  //should be exclusive with m_threadSpace

    uint32_t m_vmeSurfaceCount;  // to record how many VME surface are using in this kernel
    uint32_t m_maxSurfaceIndexAllocated;  // to record the largest surface index used in the pool,
                                          // static or reserved surfaces are not included
                                          // the var should be inited in CollectKernelSurface

    uint32_t m_barrierMode;  // to record barrier mode for this kernel

    bool m_isClonedKernel;
    uint32_t m_cloneKernelID;
    bool m_hasClones;
    CM_STATE_BUFFER_TYPE m_stateBufferBounded;

    CmMovInstConstructor *m_movInstConstructor;

    friend class CmThreadSpaceRT;

private:
    CmKernelRT(const CmKernelRT &other);
    CmKernelRT &operator=(const CmKernelRT &other);
};
};  //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELRT_H_
