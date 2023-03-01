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
//! \file      cm_thread_space_rt.h
//! \brief     Contains CmThreadSpaceRT declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTHREADSPACERT_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTHREADSPACERT_H_

#include "cm_thread_space.h"
#include "cm_hal.h"
#include "cm_log.h"

struct CM_THREAD_SPACE_UNIT
{
    void *kernel;
    uint32_t threadId;
    int32_t numEdges; //For Emulation mode
    CM_COORDINATE scoreboardCoordinates;
    uint8_t dependencyMask;
    uint8_t reset;
    uint8_t scoreboardColor;
    uint8_t sliceDestinationSelect;
    uint8_t subSliceDestinationSelect;
};

enum CM_THREAD_SPACE_DIRTY_STATUS
{
    CM_THREAD_SPACE_CLEAN                 = 0,
    CM_THREAD_SPACE_DEPENDENCY_MASK_DIRTY = 1,
    CM_THREAD_SPACE_DATA_DIRTY            = 2
};

namespace CMRT_UMD
{
class CmDeviceRT;
class CmKernel;
class CmKernelRT;
class CmTaskRT;
class CmSurface2D;
class CmThreadSpaceRT;
class CmThreadSpaceEx;
class CmThreadGroupSpace;

class CmThreadSpaceRT: public CmThreadSpace
{
public:
    static int32_t Create(CmDeviceRT *device,
                          uint32_t indexTsArray,
                          uint32_t width,
                          uint32_t height,
                          CmThreadSpaceRT* &threadSpace);

    static int32_t Destroy(CmThreadSpaceRT* &threadSpace);

    CM_RT_API int32_t AssociateThread(uint32_t x,
                                      uint32_t y,
                                      CmKernel *kernel,
                                      uint32_t threadId);

    CM_RT_API int32_t AssociateThreadWithMask(uint32_t x,
                                              uint32_t y,
                                              CmKernel *kernel,
                                              uint32_t threadId,
                                              uint8_t dependencyMask);

    CM_RT_API int32_t SetThreadDependencyPattern(uint32_t count,
                                                 int32_t *deltaX,
                                                 int32_t *deltaY);

    CM_RT_API int32_t
    SelectThreadDependencyPattern(CM_DEPENDENCY_PATTERN pattern);

    CM_RT_API int32_t SetThreadSpaceColorCount(uint32_t colorCount);

    CM_RT_API int32_t SelectMediaWalkingPattern(CM_WALKING_PATTERN pattern);

    CM_RT_API int32_t Set26ZIDispatchPattern(CM_26ZI_DISPATCH_PATTERN pattern);

    CM_RT_API int32_t Set26ZIMacroBlockSize(uint32_t width, uint32_t height);

    CM_RT_API int32_t SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect);

    CM_RT_API int32_t
    SelectMediaWalkingParameters(CM_WALKING_PARAMETERS parameters);

    CM_RT_API int32_t
    SelectThreadDependencyVectors(CM_DEPENDENCY dependencyVectors);

    CM_RT_API int32_t
    SetThreadSpaceOrder(uint32_t threadCount,
                        const CM_THREAD_PARAM *threadSpaceOrder);

    int32_t GetThreadSpaceSize(uint32_t &width, uint32_t &height);

    int32_t GetThreadSpaceUnit(CM_THREAD_SPACE_UNIT *&threadSpaceUnit);

    int32_t GetDependency(CM_HAL_DEPENDENCY *&dependency);

    int32_t
    GetDependencyPatternType(CM_DEPENDENCY_PATTERN &dependencyPatternType);

    int32_t GetWalkingPattern(CM_WALKING_PATTERN &walkingPattern);

    int32_t Get26ZIDispatchPattern(CM_26ZI_DISPATCH_PATTERN &pattern);

    int32_t GetWalkingParameters(CM_WALKING_PARAMETERS &walkingParameters);

    int32_t GetDependencyVectors(CM_HAL_DEPENDENCY &dependencyVectors);

    bool CheckWalkingParametersSet();

    bool CheckDependencyVectorsSet();

    bool CheckThreadSpaceOrderSet();

    int32_t GetColorCountMinusOne(uint32_t &colorCount);

    int32_t GetWavefront26ZDispatchInfo(
        CM_HAL_WAVEFRONT26Z_DISPATCH_INFO &dispatchInfo);

    bool IntegrityCheck(CmTaskRT *task);

    int32_t GetBoardOrder(uint32_t *&boardOrder);

    int32_t Wavefront45Sequence();

    int32_t Wavefront26Sequence();

    int32_t Wavefront26ZSequence();

    int32_t Wavefront26ZISeqVVHV26();

    int32_t Wavefront26ZISeqVVHH26();

    int32_t Wavefront26ZISeqVV26HH26();

    int32_t Wavefront26ZISeqVV1x26HH1x26();

    int32_t HorizentalSequence();

    int32_t VerticalSequence();

    int32_t WavefrontDependencyVectors();

    bool IsThreadAssociated() const;

    bool IsDependencySet();

    virtual uint32_t GetIndexInTsArray();

    CM_THREAD_SPACE_DIRTY_STATUS GetDirtyStatus() const;

    uint32_t SetDirtyStatus(CM_THREAD_SPACE_DIRTY_STATUS dirtyStatus) const;

    bool GetNeedSetKernelPointer() const;

    int32_t SetKernelPointer(CmKernelRT *kernel) const;

    bool KernelPointerIsNULL() const;

    CmKernelRT *GetKernelPointer() const;

    int32_t GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT &groupSelect);

    int32_t UpdateDependency();
    int32_t SetDependencyArgToKernel(CmKernelRT *pKernel) const;

#if CM_LOG_ON
    std::string Log();

    CM_HAL_STATE* GetHalState();
#endif

    CmThreadGroupSpace *GetThreadGroupSpace() const;

protected:
    CmThreadSpaceRT(CmDeviceRT *device,
                    uint32_t indexTsArray,
                    uint32_t width,
                    uint32_t height);

    ~CmThreadSpaceRT();

    int32_t Initialize();

    int32_t InitSwScoreBoard();

#ifdef _DEBUG
    int32_t PrintBoardOrder();
#endif

    CmDeviceRT *m_device;

    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_colorCountMinusOne;

    uint32_t m_26ZIBlockWidth;
    uint32_t m_26ZIBlockHeight;

    CM_THREAD_SPACE_UNIT *m_threadSpaceUnit;
    bool m_threadAssociated;

    bool m_needSetKernelPointer;
    CmKernelRT **m_kernel;

    CM_DEPENDENCY_PATTERN m_dependencyPatternType;
    CM_DEPENDENCY_PATTERN m_currentDependencyPattern;
    CM_HAL_DEPENDENCY m_dependency;
    CM_26ZI_DISPATCH_PATTERN m_26ZIDispatchPattern;
    CM_26ZI_DISPATCH_PATTERN m_current26ZIDispatchPattern;

    uint32_t *m_boardFlag;
    uint32_t *m_boardOrderList;
    uint32_t m_indexInList;
    uint32_t m_indexInThreadSpaceArray;  // index in device's ThreadSpaceArray

    CM_WALKING_PATTERN m_walkingPattern;
    uint32_t m_walkingParameters[CM_NUM_DWORD_FOR_MW_PARAM];
    bool m_mediaWalkerParamsSet;
    CM_HAL_DEPENDENCY m_dependencyVectors;
    bool m_dependencyVectorsSet;
    bool m_threadSpaceOrderSet;

    CmSurface2D *m_swBoardSurf; // SWSB 2D atomic
    uint32_t *m_swBoard; // SWSB system memory store
    bool m_swScoreBoardEnabled;

    // used to emulate thread space when media walker is not available
    CmThreadGroupSpace *m_threadGroupSpace; 

private:
    CmThreadSpaceRT(const CmThreadSpaceRT &other);

    CmThreadSpaceRT &operator=(const CmThreadSpaceRT &other);

    CM_THREAD_SPACE_DIRTY_STATUS *m_dirtyStatus;

    CM_HAL_WAVEFRONT26Z_DISPATCH_INFO m_wavefront26ZDispatchInfo;

    //Group select in media pipe, by default no group setting
    CM_MW_GROUP_SELECT m_groupSelect;
};
};  //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTHREADSPACERT_H_
