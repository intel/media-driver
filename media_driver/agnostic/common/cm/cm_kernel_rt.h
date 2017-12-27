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

#if USE_EXTENSION_CODE
struct InstructionDistanceConfig;
#else
typedef int InstructionDistanceConfig;
#endif

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
    SURFACE_KIND s_k;
    
    uint32_t unitCount; // 1 for for per kernel arg ; thread # for per thread arg

    uint16_t unitSize; // size of arg in byte
    uint16_t unitSizeOrig; // used to restore unitSize when reset

    uint16_t unitOffsetInPayload; // offset relative to R0 in payload
    uint16_t unitOffsetInPayloadOrig; // used to restore unitOffsetInPayload in adding move instruction for CURBE
    bool bIsDirty;      // used to indicate if its value be changed
    bool bIsSet;        // used to indicate if this argument is set correctly
    uint32_t nCustomValue;  // CM defined value for special argument kind

    uint32_t aliasIndex;    // CmSurface2D alias index
    bool bAliasCreated; // whether or not alias was created for this argument

    bool bIsNull;       // used to indicate if this is a null surface

    uint32_t unitVmeArraySize; // number of Vme surfaces in surface array 

    // pointer to the arg values. the size is unitCount * unitSize
    union
    {
        uint8_t *pValue; 
        int32_t *pIValue;
        uint32_t *pUIValue;
        float  *pFValue; 
    };

    uint16_t *surfIndex;
    SURFACE_ARRAY_ARG *pSurfArrayArg; // record each arg kind and address control mode for media sampler in surface array
    CM_ARG()
    {
        unitKind = 0;
        unitCount = 0;
        unitSize = 0;
        unitOffsetInPayload = 0;
        pValue = nullptr;
        bIsDirty = false;
        bIsNull = false;
        unitVmeArraySize = 0;
    }
};

enum CM_KERNEL_INTERNAL_ARG_TYPE
{
    CM_KERNEL_INTERNEL_ARG_PERKERNEL = 0,
    CM_KERNEL_INTERNEL_ARG_PERTHREAD = 1
};

struct CM_KERNEL_INFO;

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

//*-----------------------------------------------------------------------------
//! CM Kernel
//*-----------------------------------------------------------------------------
class CmKernelRT: public CmKernel
{
public:
    static int32_t Create(CmDeviceRT *pCmDev,
                          CmProgramRT *pProgram,
                          const char *kernelName,
                          uint32_t KernelIndex,
                          uint32_t KernelSeqNum,
                          CmKernelRT *&pKernel,
                          const char *options);

    static int32_t Destroy(CmKernelRT *&pKernel, CmProgramRT *&pProgram);

    int32_t GetThreadCount(uint32_t &count);

    CM_RT_API int32_t SetThreadCount(uint32_t count);

    CM_RT_API int32_t SetKernelArg(uint32_t index,
                                   size_t size,
                                   const void *pValue);

    CM_RT_API int32_t SetThreadArg(uint32_t threadId,
                                   uint32_t index,
                                   size_t size,
                                   const void *pValue);

    CM_RT_API int32_t SetStaticBuffer(uint32_t index, const void *pValue);

    CM_RT_API int32_t SetSurfaceBTI(SurfaceIndex *pSurface, uint32_t BTIndex);

    CM_RT_API int32_t AssociateThreadSpace(CmThreadSpace *&pThreadSpace);

    CM_RT_API int32_t AssociateThreadGroupSpace(CmThreadGroupSpace *&pTGS);

    CM_RT_API int32_t SetSamplerBTI(SamplerIndex *pSampler, uint32_t nIndex);

    CM_RT_API int32_t DeAssociateThreadSpace(CmThreadSpace *&pTS);

    CM_RT_API int32_t DeAssociateThreadGroupSpace(CmThreadGroupSpace *&pTGS);

    CM_RT_API int32_t QuerySpillSize(uint32_t &spillMemorySize);

    CM_RT_API CM_RETURN_CODE
    GetIndexForCurbeData(uint32_t curbe_data_size, SurfaceIndex *surface_index);

    CMRT_UMD_API int32_t GetBinary(std::vector<char> &binary);

    CMRT_UMD_API int32_t ReplaceBinary(std::vector<char> &binary);

    CMRT_UMD_API int32_t ResetBinary();

    int32_t GetArgs(CM_ARG *&pArg);

    int32_t GetArgCount(uint32_t &argCount);

    int32_t GetCurbeEnable(bool &b);

    int32_t SetCurbeEnable(bool b);

    int32_t GetSizeInCurbe(uint32_t &size);

    uint32_t GetAlignedCurbeSize(uint32_t value);

    int32_t GetCmDevice(CmDeviceRT *&);

    int32_t GetCmProgram(CmProgramRT *&);

    int32_t GetSizeInPayload(uint32_t &size);

    int32_t CreateKernelData(CmKernelData *&pKernelData,
                             uint32_t &kernelDataSize,
                             const CmThreadSpaceRT *pTS);

    int32_t CreateKernelData(CmKernelData *&pKernelData,
                             uint32_t &kernelDataSize,
                             const CmThreadGroupSpace *pTGS);

    char *GetName();

    int32_t SetIndexInTask(uint32_t index);

    uint32_t GetIndexInTask();

    int32_t SetAssociatedToTSFlag(bool b);

    bool IsThreadArgExisted();

    uint32_t GetKernelIndex();

    int32_t GetThreadSpace(CmThreadSpaceRT *&pThreadSpace)
    {
        pThreadSpace = m_pThreadSpace;
        return CM_SUCCESS;
    }

    int32_t GetThreadGroupSpace(CmThreadGroupSpace *&pThreadGroupSpace)
    {
        pThreadGroupSpace = m_pThreadGroupSpace;
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

    int32_t ReleaseKernelData(CmKernelData *&pKernelData);

    int32_t AcquireKernelData(CmKernelData *&pKernelData);

    int32_t CloneKernel(CmKernelRT *&pKernelOut, uint32_t id);

    void SetAsClonedKernel(uint32_t cloneKernelID);

    bool GetCloneKernelID(uint32_t &cloneKernelID);

    void SetHasClones();

    uint32_t GetMaxSurfaceIndexAllocated()
    { return m_MaxSurfaceIndexAllocated; }

    int UpdateSamplerHeap(CmKernelData *pCmKernelData);

#if CM_LOG_ON
    std::string Log();
#endif

    void SurfaceDump(uint32_t kernelNumber, int32_t taskId);

protected:
    CmKernelRT(CmDeviceRT *pCmDev,
               CmProgramRT *pProgram,
               uint32_t KernelIndex,
               uint32_t KernelSeqNum);

    ~CmKernelRT();

    int32_t SetArgsInternal(CM_KERNEL_INTERNAL_ARG_TYPE nArgType,
                            uint32_t index,
                            size_t size,
                            const void *pValue,
                            uint32_t nThreadID = 0);

    int32_t Initialize(const char *kernelName, const char *options);

    int32_t DestroyArgs();

    int32_t Reset();

    int32_t IsKernelDataReusable(CmThreadSpaceRT *pTS);

    int32_t CreateKernelArgDataGroup(uint8_t *&pData, uint32_t Value);

    int32_t ConstructObjMovs(InstructionDistanceConfig *pInstDist,
                             uint32_t dstOffset,
                             uint32_t srcOffset,
                             uint32_t size,
                             CmDynamicArray &movInsts,
                             uint32_t index,
                             bool is_BDW,
                             bool is_hwdebug);

    int32_t CreateMovInstructions(uint32_t &movInstNum,
                                  uint8_t *&pCodeDst,
                                  CM_ARG *pTempArgs,
                                  uint32_t NumArgs);

    int32_t CalcKernelDataSize(uint32_t MovInsNum,
                               uint32_t NumArgs,
                               uint32_t ArgSize,
                               uint32_t &TotalKernelDataSize);

    int32_t GetArgCountPlusSurfArray(uint32_t &ArgSize, uint32_t &ArgCountPlus);

    int32_t CreateKernelDataInternal(CmKernelData *&pKernelData,
                                     uint32_t &kernelDataSize,
                                     const CmThreadSpaceRT *pTS);

    int32_t CreateKernelDataInternal(CmKernelData *&pKernelData,
                                     uint32_t &kernelDataSize,
                                     const CmThreadGroupSpace *pTGS);

    int32_t UpdateKernelData(CmKernelData *pKernelData,
                             const CmThreadSpaceRT *pTS);

    int32_t UpdateKernelData(CmKernelData *pKernelData,
                             const CmThreadGroupSpace *pTGS);

    int32_t CreateThreadArgData(PCM_HAL_KERNEL_ARG_PARAM pKernelArg,
                                uint32_t ThreadArgIndex,
                                CmThreadSpaceRT *pThreadSpace,
                                CM_ARG *pCmArgs);

    int32_t UpdateLastKernelData(CmKernelData *&pKernelData);

    int32_t CreateKernelIndirectData(
        PCM_HAL_INDIRECT_DATA_PARAM pHalIndreictData);

    int32_t CreateThreadSpaceParam(
        PCM_HAL_KERNEL_THREADSPACE_PARAM pCmKernelThreadSpaceParam,
        CmThreadSpaceRT *pThreadSpace);

    int32_t CreateTempArgs(uint32_t NumofArgs, CM_ARG *&pTempArgs);

    int32_t SortThreadSpace(CmThreadSpaceRT *pThreadSpace);

    int32_t CleanArgDirtyFlag();

    bool IsBatchBufferReusable(CmThreadSpaceRT *pTaskThreadSpace);

    bool IsPrologueDirty();

    void DumpKernelData(CmKernelData *pKernelData);

    int32_t
    UpdateKernelDataGlobalSurfaceInfo(PCM_HAL_KERNEL_PARAM pHalKernelParam);

    CM_ARG_KIND SurfTypeToArgKind(CM_ENUM_CLASS_TYPE SurfType);

    int32_t AcquireKernelProgram();

    int32_t SetArgsVme(CM_KERNEL_INTERNAL_ARG_TYPE nArgType,
                       uint32_t ArgIndex,
                       const void *pValue,
                       uint32_t nThreadID);

    int32_t SetArgsSingleVme(CmSurfaceVme *pSurfVme,
                             uint8_t *pVmeArgValueArray,
                             uint16_t *pCmSufacesArray);

    int32_t GetVmeSurfaceIndex(uint32_t *pVmeIndexArray,
                               uint32_t *pVmeCmIndexArray,
                               uint32_t index,
                               uint32_t *pOutputValue);

    CmSurface *GetSurfaceFromSurfaceArray(SurfaceIndex *pValue,
                                          uint32_t IndexSurfaceArray);

    void ArgLog(std::ostringstream &oss,
                uint32_t index,
                CM_ARG Arg);

    int32_t CreateKernelImplicitArgDataGroup(uint8_t *&pData, uint32_t size);

    int32_t SearchAvailableIndirectSurfInfoTableEntry(uint16_t kind,
                                                      uint32_t surfaceIndex,
                                                      uint32_t BTIndex);

    int32_t SetSurfBTINumForIndirectData(CM_SURFACE_FORMAT format,
                                         CM_ENUM_CLASS_TYPE SurfaceType);

    int32_t SetArgsInternalSurfArray(int32_t offset,
                                     uint32_t kernelArgIndex,
                                     int32_t surfCount,
                                     CmSurface *pCurrentSurface,
                                     uint32_t currentSurfIndex,
                                     SurfaceIndex *pValue,
                                     uint32_t surfValue[],
                                     uint16_t origSurfIndex[]);

#if USE_EXTENSION_CODE
    int InitForGTPin(CmDeviceRT *pCmDev,
                     CmProgramRT *pProgram,
                     CmKernelRT *pKernel);

    int32_t
    UpdateKernelDataGTPinSurfaceInfo(PCM_HAL_KERNEL_PARAM pHalKernelParam);
#endif

    CmDeviceRT *m_pCmDev;
    CmSurfaceManager *m_pSurfaceMgr;
    CmProgramRT *m_pProgram;
    char *m_Options;
    char *m_pBinary;
    char *m_pBinaryOrig;
    uint32_t m_uiBinarySize;
    uint32_t m_uiBinarySizeOrig;

    uint32_t m_ThreadCount;
    uint32_t m_LastThreadCount;
    uint32_t m_SizeInCurbe;  //data size in CURBE
    uint32_t m_SizeInPayload;  //data size of inline data in media object or media walker commands
    uint32_t m_ArgCount;

    CM_ARG *m_Args;
    SurfaceIndex *m_GlobalSurfaces[CM_GLOBAL_SURFACE_NUMBER];
    uint32_t m_GlobalCmIndex[CM_GLOBAL_SURFACE_NUMBER];
    CM_KERNEL_INFO *m_pKernelInfo;
    uint32_t m_kernelIndexInProgram;

    bool m_CurbeEnable;
    bool m_NonstallingScoreboardEnable;

    uint64_t m_Id;  // high 32bit is kernel id (highest 16 bits used for kernel binary re-use
                    // in GSH), low 32bit is kernel data id

    uint32_t m_Dirty;
    CmKernelData *m_pLastKernelData;
    uint32_t m_LastKernelDataSize;

    uint32_t m_IndexInTask;
    bool m_AssociatedToTS;  // Indicates if this kernel is associated the task threadspace
                            // (scoreboard)

    bool m_blPerThreadArgExists;  // Indicates if this kernel has thread arg.
    bool m_blPerKernelArgExists;  // Indicates if the user call SetKernelArg() to set per-kernel arg

    CmThreadSpaceRT *m_pThreadSpace;  // Pointer to the kernel threadspace
    uint32_t m_adjustScoreboardY;     // value to adjust Y coordinate read from r0.1 used for
                                      // EnqueueWithHints
    uint32_t m_LastAdjustScoreboardY;

    bool m_blCreatingGPUCopyKernel;  // Indicate if this is a predefined GPUCopy kernel
    bool m_blhwDebugEnable;          // Indicate if the hw debug enabled

    uint16_t m_usKernelPayloadDataSize;  // Size of kernel indirect data (in byte)
    uint8_t *m_pKernelPayloadData;       // Pointer to the kernel indirect data memory

    uint16_t m_usKernelPayloadSurfaceCount;  //the surface count in kernel indirect data

    // the surface index list included in kernel indirect data
    SurfaceIndex*
    m_pKernelPayloadSurfaceArray[CM_MAX_STATIC_SURFACE_STATES_PER_BT];

    CM_INDIRECT_SURFACE_INFO
    m_IndirectSurfaceInfoArray[CM_MAX_STATIC_SURFACE_STATES_PER_BT];  // information used by driver

    uint32_t m_SamplerBTICount;
    CM_SAMPLER_BTI_ENTRY m_SamplerBTIEntry[CM_MAX_SAMPLER_TABLE_SIZE];

    uint32_t m_refcount;  // reference count for kernel

    CM_HAL_MAX_VALUES *m_pHalMaxValues;
    CM_HAL_MAX_VALUES_EX *m_pHalMaxValuesEx;
    bool *m_SurfaceArray;

    uint32_t m_kernelIndex;  // Kernel index in kernel array

    CmThreadGroupSpace *m_pThreadGroupSpace;  //should be exclusive with m_pThreadSpace

    uint32_t m_VMESurfaceCount;  // to record how many VME surface are using in this kernel
    uint32_t m_MaxSurfaceIndexAllocated;  // to record the largest surface index used in the pool,
                                          // static or reserved surfaces are not included
                                          // the var should be inited in CollectKernelSurface

    uint32_t m_BarrierMode;  // to record barrier mode for this kernel

    bool m_IsClonedKernel;
    uint32_t m_CloneKernelID;
    bool m_HasClones;
    CM_STATE_BUFFER_TYPE m_state_buffer_bounded;

#if USE_EXTENSION_CODE
    friend class CmThreadSpaceExt;
#endif

private:
    CmKernelRT(const CmKernelRT &other);
    CmKernelRT &operator=(const CmKernelRT &other);
};
};  //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNELRT_H_
