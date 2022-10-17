/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_thread_space_rt.cpp
//! \brief     Contains Class CmThreadSpaceRT implementations.
//!

#include "cm_thread_space_rt.h"

#include "cm_kernel_rt.h"
#include "cm_task_rt.h"
#include "cm_mem.h"
#include "cm_device_rt.h"
#include "cm_surface_2d.h"
#include "cm_extension_creator.h"

enum CM_TS_FLAG
{
    WHITE = 0,
    GRAY  = 1,
    BLACK = 2
};

static CM_DEPENDENCY waveFrontPattern =
{
    3,
    {-1, -1, 0},
    {0, -1, -1}
};

static CM_DEPENDENCY waveFront26Pattern =
{
    4,
    {-1, -1, 0, 1},
    {0, -1, -1, -1}
};

static CM_DEPENDENCY waveFront26ZPattern =
{
    5,
    {-1, -1, -1, 0, 1},
    { 1, 0, -1, -1, -1}
};

static CM_DEPENDENCY waveFront26ZIPattern =
{
    7,
    {-1, -2, -1, -1, 0, 1, 1},
    {1, 0, 0, -1, -1, -1, 0}
};

static CM_DEPENDENCY horizontalPattern =
{
    1,
    {0},
    {-1}
};

static CM_DEPENDENCY verticalPattern =
{
    1,
    {-1},
    {0}
};

static CM_DEPENDENCY waveFront26XPattern =
{
    7,
    { -1, -1, -1, 0, 0, 0, 1 },
    { 3, 1, -1, -1, -2, -3, -3 }
};

static CM_DEPENDENCY waveFront26ZIGPattern =
{
    5,
    { -1, -1, -1, 0, 1 },
    { 1, 0, -1, -1, -1 }
};

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Reset task and clear all the kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Create( CmDeviceRT* device, uint32_t indexTsArray, uint32_t width, uint32_t height, CmThreadSpaceRT* & threadSpace )
{
    if( (0 == width) || (0 == height) )
    {
        CM_ASSERTMESSAGE("Error: Invalid thread space width or height.");
        return CM_INVALID_THREAD_SPACE;
    }

    int32_t result = CM_SUCCESS;
    threadSpace = new (std::nothrow) CmThreadSpaceRT( device, indexTsArray, width, height );
    if( threadSpace )
    {
        device->m_memObjectCount.threadSpaceCount++;

        result = threadSpace->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmThreadSpaceRT::Destroy( threadSpace);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmThreadSpace due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy CM thread space
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Destroy( CmThreadSpaceRT* &threadSpace )
{
    if( threadSpace )
    {
        threadSpace->m_device->m_memObjectCount.threadSpaceCount--;
        delete threadSpace;
        threadSpace = nullptr;
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmThreadSpace
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmThreadSpaceRT::CmThreadSpaceRT( CmDeviceRT* device , uint32_t indexTsArray, uint32_t width, uint32_t height ):
    m_device( device ),
    m_width( width ),
    m_height( height ),
    m_colorCountMinusOne( 0 ),
    m_26ZIBlockWidth( CM_26ZI_BLOCK_WIDTH ),
    m_26ZIBlockHeight( CM_26ZI_BLOCK_HEIGHT ),
    m_threadSpaceUnit(nullptr),
    m_threadAssociated(false),
    m_needSetKernelPointer(false),
    m_kernel(nullptr),
    m_dependencyPatternType(CM_NONE_DEPENDENCY),
    m_currentDependencyPattern(CM_NONE_DEPENDENCY),
    m_26ZIDispatchPattern(VVERTICAL_HVERTICAL_26),
    m_current26ZIDispatchPattern(VVERTICAL_HVERTICAL_26),
    m_boardFlag(nullptr),
    m_boardOrderList(nullptr),
    m_indexInList(0),
    m_indexInThreadSpaceArray(indexTsArray),
    m_walkingPattern(CM_WALK_DEFAULT),
    m_mediaWalkerParamsSet(false),
    m_dependencyVectorsSet(false),
    m_threadSpaceOrderSet(false),
    m_swBoardSurf(nullptr),
    m_swBoard(nullptr),
    m_swScoreBoardEnabled(false),
    m_threadGroupSpace(nullptr),
    m_dirtyStatus(nullptr),
    m_groupSelect(CM_MW_GROUP_NONE)
{
    CmSafeMemSet( &m_dependency, 0, sizeof(CM_HAL_DEPENDENCY) );
    CmSafeMemSet( &m_wavefront26ZDispatchInfo, 0, sizeof(CM_HAL_WAVEFRONT26Z_DISPATCH_INFO) );
    CmSafeMemSet( &m_walkingParameters, 0, sizeof(m_walkingParameters) );
    CmSafeMemSet( &m_dependencyVectors, 0, sizeof(m_dependencyVectors) );
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmThreadSpaceRT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmThreadSpaceRT::~CmThreadSpaceRT( void )
{
    MosSafeDeleteArray(m_threadSpaceUnit);
    MosSafeDeleteArray(m_boardFlag);
    MosSafeDeleteArray(m_boardOrderList);
    CmSafeDelete( m_dirtyStatus );
    CmSafeDelete(m_kernel);

    if (m_wavefront26ZDispatchInfo.numThreadsInWave)
    {
        MOS_FreeMemory(m_wavefront26ZDispatchInfo.numThreadsInWave);
    }
    
    if (m_swScoreBoardEnabled)
    {
        MosSafeDeleteArray(m_swBoard);
        if (m_swBoardSurf != nullptr)
        {
            m_device->DestroySurface(m_swBoardSurf);
        }
    }

    if (m_threadGroupSpace != nullptr) 
    {
        m_device->DestroyThreadGroupSpace(m_threadGroupSpace);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmThreadSpaceRT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Initialize( void )
{
    m_dirtyStatus = new (std::nothrow) CM_THREAD_SPACE_DIRTY_STATUS;
    if(m_dirtyStatus == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmThreadSpace due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    *m_dirtyStatus = CM_THREAD_SPACE_CLEAN;

    m_kernel = new (std::nothrow) CmKernelRT*;
    if (m_kernel == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmThreadSpace due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    *m_kernel = nullptr;

    PCM_HAL_STATE cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    m_swScoreBoardEnabled = !(cmHalState->cmHalInterface->IsScoreboardParamNeeded());

    if (cmHalState->cmHalInterface->CheckMediaModeAvailability() == false)
    {
        CM_CHK_CMSTATUS_RETURN(m_device->CreateThreadGroupSpaceEx(1, 1, 1, m_width, m_height, 1, m_threadGroupSpace));
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Associate a thread to one uint in the 2-dimensional dependency board with default mask
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::AssociateThread( uint32_t x, uint32_t y, CmKernel* kernel , uint32_t threadId )
{
    return AssociateThreadWithMask(x, y, kernel, threadId, CM_DEFAULT_THREAD_DEPENDENCY_MASK);
}

//*-----------------------------------------------------------------------------
//! Associate a thread to one uint in the 2-dimensional dependency board.
//! If call this function twice with same x/y pair and different thread, the 2nd one will fail
//! Enqueue will make sure each x/y pair in the CmThreadSpaceRT object is associated with
//! a unique thread in the task to enqueue.Otherwise enqueue will fail.
//! Input :
//!     1) X/Y coordinats of the uint in dependency board
//!     2) pointer to CmKernel
//!     3) thread index. It is the same as the read index in
//!     CmKernel::SetThreadArg(uint32_t threadId, uint32_t index, size_t size, const void * pValue )
//! OUTPUT :
//!     CM_SUCCESS if the association is successful
//!     CM_INVALID_ARG_VALUE if the input parameters are invalid
//!     CM_OUT_OF_HOST_MEMORY if the necessary memory allocation is failed.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::AssociateThreadWithMask( uint32_t x, uint32_t y, CmKernel* kernel , uint32_t threadId, uint8_t dependencyMask )
{
    INSERT_API_CALL_LOG(GetHalState());

    if((x >= m_width) || (y >= m_height) || (kernel == nullptr))
    {
        CM_ASSERTMESSAGE("Error: Invalid input arguments.");
        return CM_INVALID_ARG_VALUE;
    }

    //Check if the m_threadSpaceUnit is allocated, we only need allocate it once at the first time.
    if( m_threadSpaceUnit == nullptr )
    {
         m_threadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_height * m_width));
        if (m_threadSpaceUnit)
        {
            CmSafeMemSet(m_threadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT) * m_height * m_width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    uint32_t linearOffset = y*m_width + x;
    if( (m_threadSpaceUnit[linearOffset].kernel == kernel) &&
        (m_threadSpaceUnit[linearOffset].threadId == threadId) &&
        (m_threadSpaceUnit[linearOffset].scoreboardCoordinates.x == x) &&
        (m_threadSpaceUnit[linearOffset].scoreboardCoordinates.y == y) )
    {
        if( m_threadSpaceUnit[linearOffset].dependencyMask == dependencyMask )
        {
            m_threadSpaceUnit[linearOffset].reset = CM_REUSE_DEPENDENCY_MASK;
        }
        else
        {
            m_threadSpaceUnit[linearOffset].dependencyMask = dependencyMask;
            m_threadSpaceUnit[linearOffset].reset = CM_RESET_DEPENDENCY_MASK;
        }
        *m_dirtyStatus = CM_THREAD_SPACE_DEPENDENCY_MASK_DIRTY;
    }
    else
    {
        m_threadSpaceUnit[linearOffset].kernel = kernel;
        m_threadSpaceUnit[linearOffset].threadId = threadId;
        m_threadSpaceUnit[linearOffset].scoreboardCoordinates.x = x;
        m_threadSpaceUnit[linearOffset].scoreboardCoordinates.y = y;
        m_threadSpaceUnit[linearOffset].dependencyMask = dependencyMask;
        m_threadSpaceUnit[linearOffset].reset = CM_NO_BATCH_BUFFER_REUSE;
        *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    if (!m_threadAssociated)
    {
        m_threadAssociated = true;
    }

    CmKernelRT *kernelRT = static_cast<CmKernelRT *>(kernel);
    kernelRT->SetAssociatedToTSFlag(true);

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Set the dependency pattern. There can be at most 8 dependent unit in the pattern.
//! Each dependent unit is indicated as the delta in X coordinat and the delta in Y coordinat
//! The call will fail if there is a pair of deltaX/Y with value ( 0, 0 )
//! By default, there is no dependent unit, i.e. count is 0.
//! Input :
//!     1) Total number of dependent units. It is <= 8.
//!     2) Array of deltaX. Array size is the first argument.
//!        Each deltaX is in the range of [-8, 7]
//!     3) Array of deltaY. Array size is the first argument.
//!        Each deltaY is in the range of [-8, 7]
//! OUTPUT :
//!     CM_SUCCESS if the pattern is set
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SetThreadDependencyPattern( uint32_t count, int32_t *deltaX, int32_t *deltaY )
{
    INSERT_API_CALL_LOG(GetHalState());

    if( count > CM_MAX_DEPENDENCY_COUNT )
    {
        CM_ASSERTMESSAGE("Error: Exceed dependency count limitation, which is 8.");
        return CM_FAILURE;
    }

    m_dependency.count = count;

    CmSafeMemCopy( m_dependency.deltaX, deltaX, sizeof( int32_t ) * count );
    CmSafeMemCopy( m_dependency.deltaY, deltaY, sizeof( int32_t ) * count );

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Select from X predefined dependency patterns.
//! Input :
//!     1) pattern index
//! OUTPUT :
//!     CM_SUCCESS if the pattern is selected
//!     CM_OUT_OF_HOST_MEMORY if the necessary memory allocation is failed.
//!     CM_FAILURE if the input dependency pattern is not supported.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SelectThreadDependencyPattern (CM_DEPENDENCY_PATTERN pattern )
{
    INSERT_API_CALL_LOG(GetHalState());

    int32_t hr = CM_SUCCESS;

     //Check if the m_boardFlag and m_boardOrderList are NULL. We only need allocate it once at the first time
    if ( m_boardFlag == nullptr )
    {
        m_boardFlag = MOS_NewArray(uint32_t, (m_height * m_width));
        if ( m_boardFlag )
        {
            CmSafeMemSet(m_boardFlag, 0, sizeof(uint32_t) * m_height * m_width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    if ( m_boardOrderList == nullptr )
    {
        m_boardOrderList = MOS_NewArray(uint32_t, (m_height * m_width));
        if (m_boardOrderList )
        {
            CmSafeMemSet(m_boardOrderList, 0, sizeof(uint32_t) * m_height * m_width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(m_boardFlag);
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    if( (pattern != CM_NONE_DEPENDENCY) && (m_walkingPattern != CM_WALK_DEFAULT ) )
    {
        CM_ASSERTMESSAGE("Error: Only valid when no walking pattern has been selected.");
        return CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN;
    }

    switch (pattern)
    {
        case CM_VERTICAL_WAVE:
            m_dependencyPatternType = CM_VERTICAL_WAVE;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(verticalPattern.count, verticalPattern.deltaX, verticalPattern.deltaY));
            break;

        case CM_HORIZONTAL_WAVE:
            m_dependencyPatternType = CM_HORIZONTAL_WAVE;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(horizontalPattern.count, horizontalPattern.deltaX, horizontalPattern.deltaY));
            break;

        case CM_WAVEFRONT:
            m_dependencyPatternType = CM_WAVEFRONT;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(waveFrontPattern.count, waveFrontPattern.deltaX, waveFrontPattern.deltaY));
            break;

        case CM_WAVEFRONT26:
            m_dependencyPatternType = CM_WAVEFRONT26;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(waveFront26Pattern.count, waveFront26Pattern.deltaX, waveFront26Pattern.deltaY));
            break;

        case CM_WAVEFRONT26Z:
            m_dependencyPatternType = CM_WAVEFRONT26Z;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(waveFront26ZPattern.count, waveFront26ZPattern.deltaX, waveFront26ZPattern.deltaY));
            m_wavefront26ZDispatchInfo.numThreadsInWave = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t) * m_width * m_height);
            if (m_threadSpaceUnit == nullptr && !CheckThreadSpaceOrderSet())
            {
                m_threadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_height * m_width));
                if (m_threadSpaceUnit)
                {
                    CmSafeMemSet(m_threadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT)* m_height * m_width);
                }
                else
                {
                    return CM_OUT_OF_HOST_MEMORY;
                }
                uint32_t threadId = 0;
                uint32_t linearOffset = 0;
                for (uint32_t y = 0; y < m_height; ++y)
                {
                    for (uint32_t x = 0; x < m_width; ++x)
                    {
                        linearOffset = y*m_width + x;
                        m_threadSpaceUnit[linearOffset].threadId = threadId++;
                        m_threadSpaceUnit[linearOffset].scoreboardCoordinates.x = x;
                        m_threadSpaceUnit[linearOffset].scoreboardCoordinates.y = y;
                        m_threadSpaceUnit[linearOffset].dependencyMask = (1 << waveFront26ZPattern.count) - 1;
                        m_threadSpaceUnit[linearOffset].reset = CM_NO_BATCH_BUFFER_REUSE;
                    }
                }

                *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
                m_threadAssociated = true;
                m_needSetKernelPointer = true;
            }
            break;

        case CM_WAVEFRONT26ZI:
            m_dependencyPatternType = CM_WAVEFRONT26ZI;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(waveFront26ZIPattern.count, waveFront26ZIPattern.deltaX, waveFront26ZIPattern.deltaY));
            if (m_threadSpaceUnit == nullptr&& !CheckThreadSpaceOrderSet())
            {
                m_threadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_height * m_width));
                if (m_threadSpaceUnit)
                {
                    CmSafeMemSet(m_threadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT)* m_height * m_width);
                }
                else
                {
                    return CM_OUT_OF_HOST_MEMORY;
                }
                uint32_t threadId = 0;
                uint32_t linearOffset = 0;
                for (uint32_t y = 0; y < m_height; ++y)
                {
                    for (uint32_t x = 0; x < m_width; ++x)
                    {
                        linearOffset = y*m_width + x;
                        m_threadSpaceUnit[linearOffset].threadId = threadId++;
                        m_threadSpaceUnit[linearOffset].scoreboardCoordinates.x = x;
                        m_threadSpaceUnit[linearOffset].scoreboardCoordinates.y = y;
                        m_threadSpaceUnit[linearOffset].dependencyMask = (1 << waveFront26ZIPattern.count) - 1;
                        m_threadSpaceUnit[linearOffset].reset = CM_NO_BATCH_BUFFER_REUSE;
                    }
                }

                *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
                m_threadAssociated = true;
                m_needSetKernelPointer = true;
            }
            break;

        case CM_WAVEFRONT26X:
            m_dependencyPatternType = CM_WAVEFRONT26X;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(waveFront26XPattern.count, waveFront26XPattern.deltaX, waveFront26XPattern.deltaY));
            break;

        case CM_WAVEFRONT26ZIG:
            m_dependencyPatternType = CM_WAVEFRONT26ZIG;
            CM_CHK_CMSTATUS_GOTOFINISH(SetThreadDependencyPattern(waveFront26ZIGPattern.count, waveFront26ZIGPattern.deltaX, waveFront26ZIGPattern.deltaY));
            break;

        case CM_NONE_DEPENDENCY:
            m_dependencyPatternType = CM_NONE_DEPENDENCY;
            hr = CM_SUCCESS;
            break;

        default:
            hr = CM_FAILURE;
            break;
    }

    UpdateDependency();

    if( m_dependencyPatternType != m_currentDependencyPattern )
    {
        *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

finish:
    return hr;
}

CM_RT_API int32_t CmThreadSpaceRT::SelectMediaWalkingPattern( CM_WALKING_PATTERN pattern )
{
    INSERT_API_CALL_LOG(GetHalState());

    int result = CM_SUCCESS;

    if( m_dependencyPatternType != CM_NONE_DEPENDENCY )
    {
        CM_ASSERTMESSAGE("Error: Only valid when no thread dependency has been selected.");
        return CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN;
    }

    switch( pattern )
    {
        case CM_WALK_DEFAULT:
        case CM_WALK_HORIZONTAL:
        case CM_WALK_VERTICAL:
        case CM_WALK_WAVEFRONT:
        case CM_WALK_WAVEFRONT26:
        case CM_WALK_WAVEFRONT26ZIG:
        case CM_WALK_WAVEFRONT26X:
        case CM_WALK_WAVEFRONT26XALT:
        case CM_WALK_WAVEFRONT45D:
        case CM_WALK_WAVEFRONT45XD_2:
        case CM_WALK_WAVEFRONT26D:
        case CM_WALK_WAVEFRONT26XD:
            m_walkingPattern = pattern;
            break;
        default:
            CM_ASSERTMESSAGE("Error: Invalid media walking pattern.");
            result = CM_INVALID_MEDIA_WALKING_PATTERN;
            break;
    }

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the media walker parameters for the CmThreadSpaceRT
//|             Used for engineering build, no error checking
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SelectMediaWalkingParameters(CM_WALKING_PARAMETERS parameters)
{
    INSERT_API_CALL_LOG(GetHalState());

    // [0..11] of parameters maps to DWORD5 through DWORD16
    // No error checking here

    if( CmSafeMemCompare(&m_walkingParameters, &parameters, sizeof(m_walkingParameters)) != 0 )
    {
        CmSafeMemCopy(&m_walkingParameters, &parameters, sizeof(m_walkingParameters));
        *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    m_mediaWalkerParamsSet = true;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the thread space order for the CmThreadSpaceRT
//|             Used for engineering build
//| Returns:
//|     CM_SUCCESS if thread space order is successfully set
//|     CM_OUT_OF_HOST_MEMORY if the necessary memory allocation is failed.
//|     CM_INVALID_ARG_VALUE if the input arg is not correct.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SetThreadSpaceOrder(uint32_t threadCount, const CM_THREAD_PARAM* threadSpaceOrder)
{
    INSERT_API_CALL_LOG(GetHalState());

    if (threadCount != m_width*m_height || threadSpaceOrder == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Thread count does not match the thread space size.");
        return CM_INVALID_ARG_VALUE;
    }
    //Check if the m_threadSpaceUnit is allocated, we only need allocate it once at the first time.
    if (m_threadSpaceUnit == nullptr)
    {
        m_threadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_height * m_width));
        if (m_threadSpaceUnit)
        {
            CmSafeMemSet(m_threadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT)* m_height * m_width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    uint32_t threadId = 0;

    for (uint32_t i = 0; i < m_width*m_height; i++)
    {
        m_threadSpaceUnit[i].threadId = threadId++;
        m_threadSpaceUnit[i].scoreboardCoordinates = threadSpaceOrder[i].scoreboardCoordinates;
        m_threadSpaceUnit[i].scoreboardColor = threadSpaceOrder[i].scoreboardColor;
        m_threadSpaceUnit[i].sliceDestinationSelect = threadSpaceOrder[i].sliceDestinationSelect;
        m_threadSpaceUnit[i].subSliceDestinationSelect = threadSpaceOrder[i].subSliceDestinationSelect;
        m_threadSpaceUnit[i].dependencyMask = CM_DEFAULT_THREAD_DEPENDENCY_MASK;
        m_threadSpaceUnit[i].reset = CM_NO_BATCH_BUFFER_REUSE;
    }
    m_threadAssociated = true;
    m_needSetKernelPointer = true;
    m_threadSpaceOrderSet = true;
    *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    return CM_SUCCESS;
}
//*-----------------------------------------------------------------------------
//| Purpose:    Sets the dependency vectors for the CmThreadSpaceRT
//|             Used for engineering build, no error checking
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SelectThreadDependencyVectors(CM_DEPENDENCY dependencyVectors)
{
    INSERT_API_CALL_LOG(GetHalState());

    if( CmSafeMemCompare(&m_dependencyVectors, &dependencyVectors, sizeof(m_dependencyVectors)) != 0 )
    {
        CmSafeMemCopy(&m_dependencyVectors, &dependencyVectors, sizeof(m_dependencyVectors));
        *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    m_dependencyVectorsSet = true;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the color count minus one of the CmThreadSpaceRT
//| Returns:    CM_INVALID_ARG_VALUE if colorCount is 0 or greater than 16
//|             CM_SUCCESS otherwise
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SetThreadSpaceColorCount(uint32_t colorCount)
{
    INSERT_API_CALL_LOG(GetHalState());

    int32_t result = CM_SUCCESS;

    PCM_HAL_STATE cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;

    result = cmHalState->cmHalInterface->ColorCountSanityCheck(colorCount);
    if(result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Color count sanity check failure.");
        return result;
    }

    m_colorCountMinusOne = colorCount - 1;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the dispatch pattern for 26ZI
//| Returns:    CM_SUCCESS if valid dispath pattern, CM_FAILURE otherwise
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::Set26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN pattern )
{
    INSERT_API_CALL_LOG(GetHalState());

     int result = CM_SUCCESS;

     switch( pattern )
     {
     case VVERTICAL_HVERTICAL_26:
         m_26ZIDispatchPattern = VVERTICAL_HVERTICAL_26;
         break;
     case VVERTICAL_HHORIZONTAL_26:
         m_26ZIDispatchPattern = VVERTICAL_HHORIZONTAL_26;
         break;
     case VVERTICAL26_HHORIZONTAL26:
         m_26ZIDispatchPattern = VVERTICAL26_HHORIZONTAL26;
         break;
     case VVERTICAL1X26_HHORIZONTAL1X26:
         m_26ZIDispatchPattern = VVERTICAL1X26_HHORIZONTAL1X26;
         break;
      default:
        result = CM_FAILURE;
        break;
     }

    if( m_26ZIDispatchPattern != m_current26ZIDispatchPattern)
    {
        *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

     return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the macro block size to be used to calculate 26ZI dispatch
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::Set26ZIMacroBlockSize( uint32_t width, uint32_t height )
{
    INSERT_API_CALL_LOG(GetHalState());
    int32_t hr = CM_SUCCESS;
    m_26ZIBlockWidth = width;
    m_26ZIBlockHeight = height;
    hr = UpdateDependency();
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the color count of the CmThreadSpaceRT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetColorCountMinusOne(uint32_t & colorCount)
{
    colorCount = m_colorCountMinusOne;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the width and height of CmThreadSpaceRT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetThreadSpaceSize(uint32_t & width, uint32_t & height)
{
    width = m_width;
    height = m_height;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get thread space's unit
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetThreadSpaceUnit(CM_THREAD_SPACE_UNIT* &threadSpaceUnit)
{
    threadSpaceUnit = m_threadSpaceUnit;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the dependency
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetDependency(CM_HAL_DEPENDENCY* &dependency)
{
    dependency = &m_dependency;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get its dependency type
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetDependencyPatternType(CM_DEPENDENCY_PATTERN &dependencyPatternType)
{
    dependencyPatternType = m_dependencyPatternType;

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::Get26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN &pattern)
{
    pattern = m_26ZIDispatchPattern;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get walking pattern
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetWalkingPattern(CM_WALKING_PATTERN &walkingPattern)
{
    walkingPattern = m_walkingPattern;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get media walking parameters
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetWalkingParameters(CM_WALKING_PARAMETERS &walkingParameters)
{
    CmSafeMemCopy(&walkingParameters, &m_walkingParameters, sizeof(m_walkingParameters));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return true if media walker parameters are set, false otherwise
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::CheckWalkingParametersSet( )
{
    return m_mediaWalkerParamsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get dependency vectors
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetDependencyVectors(CM_HAL_DEPENDENCY &dependencyVectors)
{
    CmSafeMemCopy(&dependencyVectors, &m_dependencyVectors, sizeof(m_dependencyVectors));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return true if dependency vectors are set, false otherwise
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::CheckDependencyVectorsSet( )
{
    return m_dependencyVectorsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return true if thread space order is set, false otherwise
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::CheckThreadSpaceOrderSet()
{
    return m_threadSpaceOrderSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Wavefront26ZDispatchInfo
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetWavefront26ZDispatchInfo(CM_HAL_WAVEFRONT26Z_DISPATCH_INFO &dispatchInfo)
{
    dispatchInfo = m_wavefront26ZDispatchInfo;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check the integrity of thread space' association
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::IntegrityCheck(CmTaskRT* task)
{
    CmKernelRT *kernelRT = nullptr;
    uint32_t i;
    uint32_t kernelCount = 0;
    uint32_t threadNumber = 0;
    uint32_t kernelIndex = 0;
    uint32_t unassociated = 0;
    int32_t hr = CM_SUCCESS;

    uint8_t **threadSpaceMapping = nullptr;
    uint8_t *kernelInScoreboard = nullptr;

    kernelCount = task->GetKernelCount();
    //Check if it is mult-kernel task, since no threadspace is allowed for multi-kernel tasks
    if (kernelCount > 1)
    {
        CM_ASSERTMESSAGE("Error: threadSpace->IntegrityCheck Failed: ThreadSpace is not allowed in multi-kernel task.");
        return false;
    }

    kernelRT = task->GetKernelPointer(0);
    CM_CHK_NULL_GOTOFINISH_CMERROR(kernelRT);

    //To check if the thread space size is matched with thread count
    kernelRT->GetThreadCount(threadNumber);

    //Till now, all disallowed settings are abort, now we need check if the thread space association is correct.
    if (this->IsThreadAssociated())
    {
        //For future extending to multiple kernels cases, we're using a general mechanism to check the integrity

        threadSpaceMapping = MOS_NewArray(uint8_t*, kernelCount);
        kernelInScoreboard = MOS_NewArray(uint8_t, kernelCount);

        CM_CHK_NULL_GOTOFINISH_CMERROR(threadSpaceMapping);
        CM_CHK_NULL_GOTOFINISH_CMERROR(kernelInScoreboard);

        CmSafeMemSet(threadSpaceMapping, 0, kernelCount*sizeof(uint8_t *));
        CmSafeMemSet(kernelInScoreboard, 0, kernelCount*sizeof(uint8_t));

        for (i = 0; i < kernelCount; i++)
        {
            kernelRT = task->GetKernelPointer(i);
            CM_CHK_NULL_GOTOFINISH_CMERROR(kernelRT);
            kernelRT->GetThreadCount(threadNumber);
            if (threadNumber == 0)
            {
                threadNumber = m_width * m_height;
            }
            threadSpaceMapping[i] = MOS_NewArray(uint8_t, threadNumber);
            CM_CHK_NULL_GOTOFINISH_CMERROR(threadSpaceMapping[i]);
            CmSafeMemSet(threadSpaceMapping[i], 0, threadNumber * sizeof(uint8_t));
            kernelInScoreboard[i] = 0;
        }

        for (i = 0; i < m_width * m_height; i ++ )
        {
            kernelRT = static_cast<CmKernelRT *> (m_threadSpaceUnit[i].kernel);
            if (kernelRT == nullptr)
            {
                if (m_needSetKernelPointer)
                {
                    kernelRT = *m_kernel;
                }
            }
            CM_CHK_NULL_GOTOFINISH_CMERROR(kernelRT);

            kernelIndex = kernelRT->GetIndexInTask();
            threadSpaceMapping[kernelIndex][m_threadSpaceUnit[i].threadId] = 1;
            kernelInScoreboard[kernelIndex] = 1;
        }

        for (i = 0; i < kernelCount; i ++)
        {
            if(kernelInScoreboard[i])
            {
                kernelRT = task->GetKernelPointer(i);
                CM_CHK_NULL_GOTOFINISH_CMERROR(kernelRT);

                kernelRT->GetThreadCount(threadNumber);
                if (threadNumber == 0)
                {
                    threadNumber = m_width * m_height;
                }
                kernelRT->SetAssociatedToTSFlag(true);
                for (uint32_t j = 0; j < threadNumber; j++)
                {
                    if (threadSpaceMapping[i][j] == 0)
                    {
                        unassociated ++;
                        break;
                    }
                }
            }
            MosSafeDeleteArray(threadSpaceMapping[i]);
        }

        if (unassociated != 0)
        {
            CM_ASSERTMESSAGE("Error: The thread space association is not correct.");
            hr = CM_FAILURE;
        }
    }

finish:

    MosSafeDeleteArray(threadSpaceMapping);
    MosSafeDeleteArray(kernelInScoreboard);

    return (hr == CM_SUCCESS)? true: false;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wave45 Sequence
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront45Sequence()
{
    if ( m_currentDependencyPattern == CM_WAVEFRONT )
    {
        return CM_SUCCESS;
    }
    m_currentDependencyPattern = CM_WAVEFRONT;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    for (uint32_t y = 0; y < m_height; y ++)
    {
        for (uint32_t x = 0; x < m_width; x ++)
        {
            CM_COORDINATE tempCoordinate;
            int32_t linearOffset = y * m_width + x;
            if (m_boardFlag[linearOffset] == WHITE)
            {
                m_boardOrderList[m_indexInList ++] = linearOffset;
                m_boardFlag[linearOffset] = BLACK;
                tempCoordinate.x = x - 1;
                tempCoordinate.y = y + 1;
                while ((tempCoordinate.x >= 0) && (tempCoordinate.y >= 0) &&
                    (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height))
                {
                    if (m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinate.y * m_width + tempCoordinate.x;
                        m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                    }
                    tempCoordinate.x = tempCoordinate.x - 1;
                    tempCoordinate.y = tempCoordinate.y + 1;
                }
            }
        }
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wave26 Sequence
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront26Sequence()
{
    if ( m_currentDependencyPattern == CM_WAVEFRONT26 )
    {
        return CM_SUCCESS;
    }
    m_currentDependencyPattern = CM_WAVEFRONT26;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    for (uint32_t y = 0; y < m_height; y ++)
    {
        for (uint32_t x = 0; x < m_width; x ++)
        {
            CM_COORDINATE tempCoordinate;
            int32_t linearOffset = y * m_width + x;
            if (m_boardFlag[linearOffset] == WHITE)
            {
                m_boardOrderList[m_indexInList ++] = linearOffset;
                m_boardFlag[linearOffset] = BLACK;
                tempCoordinate.x = x - 2;
                tempCoordinate.y = y + 1;
                while ((tempCoordinate.x >= 0) && (tempCoordinate.y >= 0) &&
                    (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height))
                {
                    if (m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinate.y * m_width + tempCoordinate.x;
                        m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                    }
                    tempCoordinate.x = tempCoordinate.x - 2;
                    tempCoordinate.y = tempCoordinate.y + 1;
                }
            }
        }
    }

   return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wave26Z Sequence
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront26ZSequence()
{
    if ( m_currentDependencyPattern == CM_WAVEFRONT26Z )
    {
        return CM_SUCCESS;
    }
    m_currentDependencyPattern = CM_WAVEFRONT26Z;

    uint32_t threadsInWave = 0;
    uint32_t numWaves = 0;

    if ( ( m_height % 2 != 0 ) || ( m_width % 2 != 0 ) )
    {
        return CM_INVALID_ARG_SIZE;
    }
    CmSafeMemSet( m_boardFlag, WHITE, m_width * m_height * sizeof( uint32_t ) );
    m_indexInList = 0;

    uint32_t iX, iY, nOffset;
    iX = iY = nOffset = 0;

    uint32_t *waveFrontPosition = MOS_NewArray(uint32_t, m_width);
    uint32_t *waveFrontOffset = MOS_NewArray(uint32_t, m_width);
    if ( ( waveFrontPosition == nullptr ) || ( waveFrontOffset == nullptr ) )
    {
        MosSafeDeleteArray( waveFrontPosition );
        MosSafeDeleteArray( waveFrontOffset );
        return CM_FAILURE;
    }
    CmSafeMemSet( waveFrontPosition, 0, m_width * sizeof( int ) );

    // set initial value
    m_boardFlag[ 0 ] = BLACK;
    m_boardOrderList[ 0 ] = 0;
    waveFrontPosition[ 0 ] = 1;
    m_indexInList = 0;

    CM_COORDINATE mask[ 8 ];
    uint32_t nMaskNumber = 0;

    m_wavefront26ZDispatchInfo.numThreadsInWave[numWaves] = 1;
    numWaves++;

    while ( m_indexInList < m_width * m_height - 1 )
    {

        CmSafeMemSet( waveFrontOffset, 0, m_width * sizeof( int ) );
        for ( uint32_t iX = 0; iX < m_width; ++iX )
        {
            uint32_t iY = waveFrontPosition[ iX ];
            nOffset = iY * m_width + iX;
            CmSafeMemSet( mask, 0, sizeof( mask ) );

            if ( m_boardFlag[ nOffset ] == WHITE )
            {
                if ( ( iX % 2 == 0 ) && ( iY % 2 == 0 ) )
                {
                    if ( iX == 0 )
                    {
                        mask[ 0 ].x = 0;
                        mask[ 0 ].y = -1;
                        mask[ 1 ].x = 1;
                        mask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else if ( iY == 0 )
                    {
                        mask[ 0 ].x = -1;
                        mask[ 0 ].y = 1;
                        mask[ 1 ].x = -1;
                        mask[ 1 ].y = 0;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        mask[ 0 ].x = -1;
                        mask[ 0 ].y = 1;
                        mask[ 1 ].x = -1;
                        mask[ 1 ].y = 0;
                        mask[ 2 ].x = 0;
                        mask[ 2 ].y = -1;
                        mask[ 3 ].x = 1;
                        mask[ 3 ].y = -1;
                        nMaskNumber = 4;
                    }
                }
                else if ( ( iX % 2 == 0 ) && ( iY % 2 == 1 ) )
                {
                    if ( iX == 0 )
                    {
                        mask[ 0 ].x = 0;
                        mask[ 0 ].y = -1;
                        mask[ 1 ].x = 1;
                        mask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        mask[ 0 ].x = -1;
                        mask[ 0 ].y = 0;
                        mask[ 1 ].x = 0;
                        mask[ 1 ].y = -1;
                        mask[ 2 ].x = 1;
                        mask[ 2 ].y = -1;
                        nMaskNumber = 3;
                    }
                }
                else if ( ( iX % 2 == 1 ) && ( iY % 2 == 0 ) )
                {
                    if ( iY == 0 )
                    {
                        mask[ 0 ].x = -1;
                        mask[ 0 ].y = 0;
                        nMaskNumber = 1;
                    }
                    else if ( iX == m_width - 1 )
                    {
                        mask[ 0 ].x = -1;
                        mask[ 0 ].y = 0;
                        mask[ 1 ].x = 0;
                        mask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        mask[ 0 ].x = -1;
                        mask[ 0 ].y = 0;
                        mask[ 1 ].x = 0;
                        mask[ 1 ].y = -1;
                        mask[ 2 ].x = 1;
                        mask[ 2 ].y = -1;
                        nMaskNumber = 3;
                    }
                }
                else
                {
                    mask[ 0 ].x = -1;
                    mask[ 0 ].y = 0;
                    mask[ 1 ].x = 0;
                    mask[ 1 ].y = -1;
                    nMaskNumber = 2;
                }

                // check if all of the dependencies are in the dispatch queue
                bool allInQueue = true;
                for ( uint32_t i = 0; i < nMaskNumber; ++i )
                {
                    if ( m_boardFlag[ nOffset + mask[ i ].x + mask[ i ].y * m_width ] == WHITE )
                    {
                        allInQueue = false;
                        break;
                    }
                }
                if ( allInQueue )
                {
                    waveFrontOffset[ iX ] = nOffset;
                    if( waveFrontPosition[ iX ] < m_height - 1 )
                    {
                        waveFrontPosition[ iX ]++;
                    }
                }
            }
        }

        for ( uint32_t iX = 0; iX < m_width; ++iX )
        {
            if ( ( m_boardFlag[ waveFrontOffset[ iX ] ] == WHITE ) && ( waveFrontOffset[ iX ] != 0 ) )
            {
                m_indexInList++;
                m_boardOrderList[ m_indexInList ] = waveFrontOffset[ iX ];
                m_boardFlag[ waveFrontOffset[ iX ] ] = BLACK;
                threadsInWave++;
            }
        }

        m_wavefront26ZDispatchInfo.numThreadsInWave[numWaves] = threadsInWave;
        threadsInWave = 0;
        numWaves++;
    }

    MosSafeDeleteArray( waveFrontPosition );
    MosSafeDeleteArray( waveFrontOffset );

    m_wavefront26ZDispatchInfo.numWaves = numWaves;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wavefront26ZI Sequence
//|             Dispatch order:
//|                Vertical threads vertically in macro block
//|                Horizontal threads vertically in macro block
//|                Overall 26 pattern
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront26ZISeqVVHV26()
{
    if ( m_currentDependencyPattern == CM_WAVEFRONT26ZI  &&
        ( m_current26ZIDispatchPattern == VVERTICAL_HVERTICAL_26 ) )
    {
        return CM_SUCCESS;
    }

    m_currentDependencyPattern = CM_WAVEFRONT26ZI;
    m_current26ZIDispatchPattern = VVERTICAL_HVERTICAL_26;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    for( uint32_t y = 0; y < m_height; y = y + m_26ZIBlockHeight )
    {
        for( uint32_t x = 0; x < m_width; x = x + m_26ZIBlockWidth )
        {
            CM_COORDINATE tempCoordinateFor26;
            tempCoordinateFor26.x = x;
            tempCoordinateFor26.y = y;

            do
            {
                if( m_boardFlag[tempCoordinateFor26.y * m_width + tempCoordinateFor26.x] == WHITE )
                {
                    m_boardOrderList[m_indexInList ++] = tempCoordinateFor26.y * m_width + tempCoordinateFor26.x;
                    m_boardFlag[tempCoordinateFor26.y * m_width + tempCoordinateFor26.x] = BLACK;

                    // do vertical edges
                    for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE tempCoordinate;
                        uint32_t localHeightCounter = 0;

                        tempCoordinate.x = tempCoordinateFor26.x + widthCount;
                        tempCoordinate.y = tempCoordinateFor26.y;
                        while( (tempCoordinate.x >= 0) && (tempCoordinate.y >=0) &&
                            (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                            {
                                m_boardOrderList[m_indexInList ++ ] = tempCoordinate.y * m_width + tempCoordinate.x;
                                m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                            }
                            tempCoordinate.y = tempCoordinate.y + 1;
                            localHeightCounter++;
                        }
                    } // vertical edges

                     // do horizontal edges
                    for( uint32_t widthCount = 1; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE tempCoordinate;
                        uint32_t localHeightCounter = 0;

                        tempCoordinate.x = tempCoordinateFor26.x + widthCount;
                        tempCoordinate.y = tempCoordinateFor26.y;
                        while( (tempCoordinate.x >= 0) && (tempCoordinate.y >=0) &&
                            (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                            {
                                m_boardOrderList[m_indexInList ++ ] = tempCoordinate.y * m_width + tempCoordinate.x;
                                m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                            }
                            tempCoordinate.y = tempCoordinate.y + 1;
                            localHeightCounter++;
                        }
                    } // horizontal edges
                }

                tempCoordinateFor26.x = tempCoordinateFor26.x - (2 * m_26ZIBlockWidth);
                tempCoordinateFor26.y = tempCoordinateFor26.y + (1 * m_26ZIBlockHeight);

            } while( ( tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0)
                && (tempCoordinateFor26.x < (int32_t)m_width) && ( tempCoordinateFor26.y < (int32_t)m_height));
        }
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wavefront26ZI Sequence
//|             Dispatch order:
//|                Vertical threads vertically in macro block
//|                Horizontal threads horizontally in macro block
//|                Overall 26 pattern
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront26ZISeqVVHH26()
{
    if ( m_currentDependencyPattern == CM_WAVEFRONT26ZI &&
        ( m_current26ZIDispatchPattern == VVERTICAL_HHORIZONTAL_26))
    {
        return CM_SUCCESS;
    }

    m_currentDependencyPattern = CM_WAVEFRONT26ZI;
    m_current26ZIDispatchPattern = VVERTICAL_HHORIZONTAL_26;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    for( uint32_t y = 0; y < m_height; y = y + m_26ZIBlockHeight )
    {
        for( uint32_t x = 0; x < m_width; x = x + m_26ZIBlockWidth )
        {
            CM_COORDINATE tempCoordinateFor26;
            tempCoordinateFor26.x = x;
            tempCoordinateFor26.y = y;

            do
            {
                if( m_boardFlag[tempCoordinateFor26.y * m_width + tempCoordinateFor26.x] == WHITE )
                {
                    m_boardOrderList[m_indexInList ++] = tempCoordinateFor26.y * m_width + tempCoordinateFor26.x;
                    m_boardFlag[tempCoordinateFor26.y * m_width + tempCoordinateFor26.x] = BLACK;

                    // do vertical edges
                    for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE tempCoordinate;
                        uint32_t localHeightCounter = 0;

                        tempCoordinate.x = tempCoordinateFor26.x + widthCount;
                        tempCoordinate.y = tempCoordinateFor26.y;
                        while( (tempCoordinate.x >= 0) && (tempCoordinate.y >=0) &&
                            (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                            {
                                m_boardOrderList[m_indexInList ++ ] = tempCoordinate.y * m_width + tempCoordinate.x;
                                m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                            }
                            tempCoordinate.y = tempCoordinate.y + 1;
                            localHeightCounter++;
                        }
                    } // vertical edges

                    // horizontal edges
                    for( uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
                    {
                        CM_COORDINATE tempCoordinate;
                        uint32_t localWidthCounter = 0;

                        tempCoordinate.x = tempCoordinateFor26.x + 1;
                        tempCoordinate.y = tempCoordinateFor26.y + heightCount;
                        while ( (tempCoordinate.x >= 0) && (tempCoordinate.y >= 0) &&
                            (tempCoordinate.x< (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height) &&
                            (localWidthCounter < (m_26ZIBlockWidth / 2) ) )
                        {
                            if( m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                            {
                                m_boardOrderList[m_indexInList ++ ] = tempCoordinate.y * m_width + tempCoordinate.x;
                                m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                            }

                            tempCoordinate.x = tempCoordinate.x + 2;
                            localWidthCounter++;
                        }
                    }
                    // horizontal edges
                }

                tempCoordinateFor26.x = tempCoordinateFor26.x - (2 * m_26ZIBlockWidth);
                tempCoordinateFor26.y = tempCoordinateFor26.y + (1 * m_26ZIBlockHeight);

            } while( ( tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0)
                && (tempCoordinateFor26.x < (int32_t)m_width) && ( tempCoordinateFor26.y < (int32_t)m_height));
        }
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wavefront26ZI Sequence
//|             Dispatch order:
//|                Vertical threads vertically in macro block and then along 26 wave
//|                Horizontal threads horizontally in macro block and then along 26 wave
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront26ZISeqVV26HH26()
{
    if( (m_currentDependencyPattern == CM_WAVEFRONT26ZI) &&
        (m_current26ZIDispatchPattern == VVERTICAL26_HHORIZONTAL26) )
    {
        return CM_SUCCESS;
    }

    m_currentDependencyPattern = CM_WAVEFRONT26ZI;
    m_current26ZIDispatchPattern = VVERTICAL26_HHORIZONTAL26;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    uint32_t waveFrontNum = 0;
    uint32_t waveFrontStartX = 0;
    uint32_t waveFrontStartY = 0;

    uint32_t adjustHeight = 0;

    CM_COORDINATE tempCoordinateFor26;
    tempCoordinateFor26.x = 0;
    tempCoordinateFor26.y = 0;

    while( (tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0) &&
        (tempCoordinateFor26.x < (int32_t)m_width) && (tempCoordinateFor26.y < (int32_t)m_height) )
    {
        // use horizontal coordinates to save starting (x,y) for overall 26
        CM_COORDINATE tempCoordinateForHorz;
        tempCoordinateForHorz.x = tempCoordinateFor26.x;
        tempCoordinateForHorz.y = tempCoordinateFor26.y;

       do
        {
            CM_COORDINATE tempCoordinateForVer;

            for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount += 2 )
            {
                uint32_t localHeightCounter = 0;
                tempCoordinateForVer.x = tempCoordinateFor26.x + widthCount;
                tempCoordinateForVer.y = tempCoordinateFor26.y;

                while( (tempCoordinateForVer.x < (int32_t)m_width) && (tempCoordinateForVer.y < (int32_t)m_height) &&
                        (tempCoordinateForVer.x >= 0) && (tempCoordinateForVer.y >= 0) && (localHeightCounter < m_26ZIBlockHeight) )
                {
                    if(m_boardFlag[tempCoordinateForVer.y * m_width + tempCoordinateForVer.x] == WHITE )
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinateForVer.y * m_width + tempCoordinateForVer.x;
                        m_boardFlag[tempCoordinateForVer.y * m_width + tempCoordinateForVer.x] = BLACK;
                    }
                    tempCoordinateForVer.y += 1;
                    localHeightCounter++;
                }
            }

            tempCoordinateFor26.x = tempCoordinateFor26.x + (2 * m_26ZIBlockWidth);
            tempCoordinateFor26.y = tempCoordinateFor26.y - (1 * m_26ZIBlockHeight);

        } while( (tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0) &&
            (tempCoordinateFor26.x < (int32_t)m_width) && (tempCoordinateFor26.y < (int32_t)m_height) );

        tempCoordinateFor26.x = tempCoordinateForHorz.x;
        tempCoordinateFor26.y = tempCoordinateForHorz.y;

        do
        {
            // do horizontal edges
            for ( uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
            {
                uint32_t localWidthCounter = 0;
                tempCoordinateForHorz.x = tempCoordinateFor26.x + 1;
                tempCoordinateForHorz.y = tempCoordinateFor26.y + heightCount;
                while( (tempCoordinateForHorz.x >= 0) && (tempCoordinateForHorz.y >= 0) &&
                    (tempCoordinateForHorz.x < (int32_t)m_width) && (tempCoordinateForHorz.y < (int32_t)m_height) &&
                    (localWidthCounter < (m_26ZIBlockWidth / 2)) )
                {
                    if( m_boardFlag[tempCoordinateForHorz.y * m_width + tempCoordinateForHorz.x] == WHITE )
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinateForHorz.y * m_width + tempCoordinateForHorz.x;
                        m_boardFlag[tempCoordinateForHorz.y * m_width + tempCoordinateForHorz.x] = BLACK;
                    }

                    tempCoordinateForHorz.x += 2;
                    localWidthCounter++;
                }
            }

            tempCoordinateFor26.x = tempCoordinateFor26.x + (2 * m_26ZIBlockWidth);
            tempCoordinateFor26.y = tempCoordinateFor26.y - (1 * m_26ZIBlockHeight);

        } while( (tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0) &&
            (tempCoordinateFor26.x < (int32_t)m_width) && (tempCoordinateFor26.y < (int32_t)m_height) );

        if (m_width <= m_26ZIBlockWidth)
        {
            tempCoordinateFor26.x = 0;
            tempCoordinateFor26.y = tempCoordinateForHorz.y + m_26ZIBlockHeight;
        }
        else
        {
            // update wavefront number
            waveFrontNum++;
            adjustHeight = (uint32_t)ceil((double)m_height / m_26ZIBlockHeight);

            if (waveFrontNum < (2 * adjustHeight))
            {
                waveFrontStartX = waveFrontNum & 1;
                waveFrontStartY = (uint32_t)floor((double)waveFrontNum / 2);
            }
            else
            {
                waveFrontStartX = (waveFrontNum - 2 * adjustHeight) + 2;
                waveFrontStartY = (adjustHeight)-1;
            }

            tempCoordinateFor26.x = waveFrontStartX * m_26ZIBlockWidth;
            tempCoordinateFor26.y = waveFrontStartY * m_26ZIBlockHeight;
        }
     }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wavefront26ZI Sequence
//|             Dispatch order:
//|                Vertical threads vertically along 26 wave then in macro block
//|                Horizontal threads horizontally along 26 wave then in macro block
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront26ZISeqVV1x26HH1x26()
{
    if ( (m_currentDependencyPattern == CM_WAVEFRONT26ZI) &&
        (m_current26ZIDispatchPattern == VVERTICAL1X26_HHORIZONTAL1X26))
    {
        return CM_SUCCESS;
    }

    m_currentDependencyPattern = CM_WAVEFRONT26ZI;
    m_current26ZIDispatchPattern = VVERTICAL1X26_HHORIZONTAL1X26;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    uint32_t waveFrontNum = 0;
    uint32_t waveFrontStartX = 0;
    uint32_t waveFrontStartY = 0;

    uint32_t adjustHeight = 0;

    CM_COORDINATE tempCoordinateFor26;
    tempCoordinateFor26.x = 0;
    tempCoordinateFor26.y = 0;

    CM_COORDINATE saveTempCoordinateFor26;
    saveTempCoordinateFor26.x = 0;
    saveTempCoordinateFor26.y = 0;

    CM_COORDINATE tempCoordinateForVer;
    CM_COORDINATE tempCoordinateForHorz;

    while( (tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0) &&
        (tempCoordinateFor26.x < (int32_t)m_width) && (tempCoordinateFor26.y < (int32_t)m_height) )
    {
        saveTempCoordinateFor26.x = tempCoordinateFor26.x;
        saveTempCoordinateFor26.y = tempCoordinateFor26.y;

        // do vertical edges
        for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount += 2 )
        {
            // restore original starting point
            tempCoordinateFor26.x = saveTempCoordinateFor26.x;
            tempCoordinateFor26.y = saveTempCoordinateFor26.y;

            do
            {
                uint32_t localHeightCounter = 0;
                tempCoordinateForVer.x = tempCoordinateFor26.x + widthCount;
                tempCoordinateForVer.y = tempCoordinateFor26.y;
                while( (tempCoordinateForVer.x < (int32_t)m_width) && (tempCoordinateForVer.y < (int32_t)m_height) &&
                        (tempCoordinateForVer.x >= 0) && (tempCoordinateForVer.y >= 0) && (localHeightCounter < m_26ZIBlockHeight) )
                {
                    if(m_boardFlag[tempCoordinateForVer.y * m_width + tempCoordinateForVer.x] == WHITE )
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinateForVer.y * m_width + tempCoordinateForVer.x;
                        m_boardFlag[tempCoordinateForVer.y * m_width + tempCoordinateForVer.x] = BLACK;
                    }
                    tempCoordinateForVer.y += 1;
                    localHeightCounter++;
                }

                tempCoordinateFor26.x = tempCoordinateFor26.x + (2 * m_26ZIBlockWidth);
                tempCoordinateFor26.y = tempCoordinateFor26.y - ( 1 * m_26ZIBlockHeight);

            } while( (tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0) &&
            (tempCoordinateFor26.x < (int32_t)m_width) && (tempCoordinateFor26.y < (int32_t)m_height) );
        }

        // do horizontal edges
        // restore original starting position
        tempCoordinateFor26.x = saveTempCoordinateFor26.x;
        tempCoordinateFor26.y = saveTempCoordinateFor26.y;

        for(uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
        {
            // restore original starting point
            tempCoordinateFor26.x = saveTempCoordinateFor26.x;
            tempCoordinateFor26.y = saveTempCoordinateFor26.y;

            do
            {
                uint32_t localWidthCounter = 0;
                tempCoordinateForHorz.x = tempCoordinateFor26.x + 1;
                tempCoordinateForHorz.y = tempCoordinateFor26.y + heightCount;
                while( (tempCoordinateForHorz.x >= 0) && (tempCoordinateForHorz.y >= 0) &&
                    (tempCoordinateForHorz.x < (int32_t)m_width) && (tempCoordinateForHorz.y < (int32_t)m_height) &&
                    (localWidthCounter < (m_26ZIBlockWidth / 2)) )
                {
                    if( m_boardFlag[tempCoordinateForHorz.y * m_width + tempCoordinateForHorz.x] == WHITE )
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinateForHorz.y * m_width + tempCoordinateForHorz.x;
                        m_boardFlag[tempCoordinateForHorz.y * m_width + tempCoordinateForHorz.x] = BLACK;
                    }

                    tempCoordinateForHorz.x += 2;
                    localWidthCounter++;
                }

                tempCoordinateFor26.x = tempCoordinateFor26.x + (2 * m_26ZIBlockWidth);
                tempCoordinateFor26.y = tempCoordinateFor26.y - ( 1 * m_26ZIBlockHeight);

            } while( (tempCoordinateFor26.x >= 0) && (tempCoordinateFor26.y >= 0) &&
            (tempCoordinateFor26.x < (int32_t)m_width) && (tempCoordinateFor26.y < (int32_t)m_height) );

        }

        if (m_width <= m_26ZIBlockWidth)
        {
            tempCoordinateFor26.x = 0;
            tempCoordinateFor26.y = saveTempCoordinateFor26.y + m_26ZIBlockHeight;
        }
        else
        {
            // update wavefront number
            waveFrontNum++;
            adjustHeight = (uint32_t)ceil((double)m_height / m_26ZIBlockHeight);

            if (waveFrontNum < (2 * adjustHeight))
            {
                waveFrontStartX = waveFrontNum & 1;
                waveFrontStartY = (uint32_t)floor((double)waveFrontNum / 2);
            }
            else
            {
                waveFrontStartX = (waveFrontNum - 2 * adjustHeight) + 2;
                waveFrontStartY = (adjustHeight)-1;
            }

            tempCoordinateFor26.x = waveFrontStartX * m_26ZIBlockWidth;
            tempCoordinateFor26.y = waveFrontStartY * m_26ZIBlockHeight;
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::VerticalSequence()
{
    if ( m_currentDependencyPattern == CM_VERTICAL_WAVE)
    {
        return CM_SUCCESS;
    }
    m_currentDependencyPattern = CM_VERTICAL_WAVE;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    for (uint32_t x = 0; x < m_width; x ++)
    {
        for (uint32_t y = 0; y < m_height; y ++)
        {
            CM_COORDINATE tempCoordinate;
            int32_t linearOffset = y * m_width + x;
            if (m_boardFlag[linearOffset] == WHITE)
            {
                m_boardOrderList[m_indexInList ++] = linearOffset;
                m_boardFlag[linearOffset] = BLACK;
                tempCoordinate.x = x;
                tempCoordinate.y = y + 1;
                while ((tempCoordinate.x >= 0) && (tempCoordinate.y >= 0) &&
                    (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height))
                {
                    if (m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinate.y * m_width + tempCoordinate.x;
                        m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                    }
                    tempCoordinate.y = tempCoordinate.y + 1;
                }
            }
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::HorizentalSequence()
{
    if ( m_currentDependencyPattern == CM_HORIZONTAL_WAVE)
    {
        return CM_SUCCESS;
    }
    m_currentDependencyPattern = CM_HORIZONTAL_WAVE;

    CmSafeMemSet(m_boardFlag, WHITE, m_width*m_height*sizeof(uint32_t));
    m_indexInList = 0;

    for (uint32_t y = 0; y < m_height; y ++)
    {
        for (uint32_t x = 0; x < m_width; x ++)
        {
            CM_COORDINATE tempCoordinate;
            int32_t linearOffset = y * m_width + x;
            if (m_boardFlag[linearOffset] == WHITE)
            {
                m_boardOrderList[m_indexInList ++] = linearOffset;
                m_boardFlag[linearOffset] = BLACK;
                tempCoordinate.x = x + 1;
                tempCoordinate.y = y;
                while ((tempCoordinate.x >= 0) && (tempCoordinate.y >= 0) &&
                    (tempCoordinate.x < (int32_t)m_width) && (tempCoordinate.y < (int32_t)m_height))
                {
                    if (m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] == WHITE)
                    {
                        m_boardOrderList[m_indexInList ++] = tempCoordinate.y * m_width + tempCoordinate.x;
                        m_boardFlag[tempCoordinate.y * m_width + tempCoordinate.x] = BLACK;
                    }
                    tempCoordinate.x = tempCoordinate.x + 1;
                }
            }
        }
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wave Sequence for depenedncy vectors
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::WavefrontDependencyVectors()
{
    if (m_boardFlag == nullptr)
    {
        m_boardFlag = MOS_NewArray(uint32_t, (m_height * m_width));
        if (m_boardFlag)
        {
            CmSafeMemSet(m_boardFlag, WHITE, (sizeof(uint32_t)* m_height * m_width));
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    if (m_boardOrderList == nullptr)
    {
        m_boardOrderList = MOS_NewArray(uint32_t, (m_height * m_width));
        if (m_boardOrderList)
        {
            CmSafeMemSet(m_boardOrderList, 0, sizeof(uint32_t)* m_height * m_width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(m_boardFlag);
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    uint32_t iX, iY, nOffset;
    iX = iY = nOffset = 0;

    uint32_t *waveFrontPosition = MOS_NewArray(uint32_t, m_width);
    uint32_t *waveFrontOffset = MOS_NewArray(uint32_t, m_width);
    if ((waveFrontPosition == nullptr) || (waveFrontOffset == nullptr))
    {
        MosSafeDeleteArray(waveFrontPosition);
        MosSafeDeleteArray(waveFrontOffset);
        return CM_FAILURE;
    }
    CmSafeMemSet(waveFrontPosition, 0, m_width * sizeof(int));

    // set initial value
    m_boardFlag[0] = BLACK;
    m_boardOrderList[0] = 0;
    waveFrontPosition[0] = 1;
    m_indexInList = 0;

    while (m_indexInList < m_width * m_height - 1)
    {
        CmSafeMemSet(waveFrontOffset, 0, m_width * sizeof(int));
        for (uint32_t iX = 0; iX < m_width; ++iX)
        {
            uint32_t iY = waveFrontPosition[iX];
            nOffset = iY * m_width + iX;
            if (m_boardFlag[nOffset] == WHITE)
            {
                // check if all of the dependencies are in the dispatch queue
                bool allInQueue = true;
                for (uint32_t i = 0; i < m_dependencyVectors.count; ++i)
                {
                    uint32_t tempOffset = nOffset + m_dependencyVectors.deltaX[i] + m_dependencyVectors.deltaY[i] * m_width;
                    if (tempOffset <= m_width * m_height - 1)
                    {
                        if (m_boardFlag[nOffset + m_dependencyVectors.deltaX[i] + m_dependencyVectors.deltaY[i] * m_width] == WHITE)
                        {
                            allInQueue = false;
                            break;
                        }
                    }
                }
                if (allInQueue)
                {
                    waveFrontOffset[iX] = nOffset;
                    if (waveFrontPosition[iX] < m_height - 1)
                    {
                        waveFrontPosition[iX]++;
                    }
                }
            }
        }

        for (uint32_t iX = 0; iX < m_width; ++iX)
        {
            if ((m_boardFlag[waveFrontOffset[iX]] == WHITE) && (waveFrontOffset[iX] != 0))
            {
                m_indexInList++;
                m_boardOrderList[m_indexInList] = waveFrontOffset[iX];
                m_boardFlag[waveFrontOffset[iX]] = BLACK;
            }
        }
    }

    MosSafeDeleteArray(waveFrontPosition);
    MosSafeDeleteArray(waveFrontOffset);
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Board Order list
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetBoardOrder(uint32_t *&boardOrder)
{
    boardOrder = m_boardOrderList;
    return CM_SUCCESS;
}

#ifdef _DEBUG
int32_t CmThreadSpaceRT::PrintBoardOrder()
{
    CM_NORMALMESSAGE("According to dependency, the score board order is:");
    for (uint32_t i = 0; i < m_height * m_width; i ++)
    {
        CM_NORMALMESSAGE("%d->", m_boardOrderList[i]);
    }
    CM_NORMALMESSAGE("NIL.");
    return 0;
}
#endif

bool CmThreadSpaceRT::IsThreadAssociated() const
{
    return m_threadAssociated;
}

bool CmThreadSpaceRT::IsDependencySet()
{
    return ((m_dependencyPatternType != CM_NONE_DEPENDENCY) ? true : false);
}

bool CmThreadSpaceRT::GetNeedSetKernelPointer() const
{
    return m_needSetKernelPointer;
}

int32_t CmThreadSpaceRT::SetKernelPointer(CmKernelRT* kernel) const
{
    *m_kernel = kernel;
    return CM_SUCCESS;
}

bool CmThreadSpaceRT::KernelPointerIsNULL() const
{
    if (*m_kernel == nullptr)
    {
        return true;
    }
    else
    {
        return false;
    }
}

CmKernelRT* CmThreadSpaceRT::GetKernelPointer() const
{
    return *m_kernel;
}

uint32_t CmThreadSpaceRT::GetIndexInTsArray()
{
    return m_indexInThreadSpaceArray;
}

CM_THREAD_SPACE_DIRTY_STATUS CmThreadSpaceRT::GetDirtyStatus() const
{
    return *m_dirtyStatus;
}

uint32_t CmThreadSpaceRT::SetDirtyStatus(CM_THREAD_SPACE_DIRTY_STATUS dirtyStatus) const
{
    *m_dirtyStatus = dirtyStatus;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmThreadSpaceRT::SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect)
{
    if (groupSelect != m_groupSelect)
    {
        m_groupSelect = groupSelect;
        *m_dirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT &groupSelect)
{
    groupSelect = m_groupSelect;
    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::UpdateDependency()
{
    //Init SW scoreboard
    if (!m_swScoreBoardEnabled)
    {
        return CM_SUCCESS;
    }
    if (m_swBoard == nullptr)
    {
        m_swBoard = MOS_NewArray(uint32_t, (m_height * m_width));
        if (m_swBoard)
        {
            CmSafeMemSet(m_swBoard, 0, sizeof(uint32_t)* m_height * m_width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(m_swBoard);
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    if (m_swBoardSurf == nullptr)
    {
        //for 2D atomic
        CM_CHK_CMSTATUS_RETURN(m_device->CreateSurface2D(m_width,
                m_height, 
                Format_R32S,
                m_swBoardSurf));
    }
    CM_CHK_CMSTATUS_RETURN(InitSwScoreBoard());
    CM_CHK_CMSTATUS_RETURN(m_swBoardSurf->WriteSurface((uint8_t *)m_swBoard, nullptr));
    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::SetDependencyArgToKernel(CmKernelRT *pKernel) const
{
    if (!m_swScoreBoardEnabled)
    {
        return CM_SUCCESS;
    }
    int32_t hr = CM_SUCCESS;

    for (uint32_t k = 0; k < pKernel->m_argCount; k++)
    {
        if (pKernel->m_args[k].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD)
        {
            SurfaceIndex* ScoreboardIndex = nullptr;
            CM_CHK_CMSTATUS_RETURN(m_swBoardSurf->GetIndex(ScoreboardIndex));
            CM_CHK_CMSTATUS_RETURN(pKernel->SetKernelArg(k, sizeof(SurfaceIndex), ScoreboardIndex));
        }
        else if (pKernel->m_args[k].unitKind == ARG_KIND_GENERAL_DEPVEC)
        {
            char vectors[CM_MAX_DEPENDENCY_COUNT * 2];
            for (int ii = 0; ii < CM_MAX_DEPENDENCY_COUNT; ii++) 
            {
                vectors[ii] = (char)m_dependency.deltaX[ii];
                vectors[ii + CM_MAX_DEPENDENCY_COUNT] = (char)m_dependency.deltaY[ii];
            }
            CM_CHK_CMSTATUS_RETURN(pKernel->SetKernelArg(k, (sizeof(char)*CM_MAX_DEPENDENCY_COUNT * 2), vectors));
        }
        else if (pKernel->m_args[k].unitKind == ARG_KIND_GENERAL_DEPCNT)
        {
            CM_CHK_CMSTATUS_RETURN(pKernel->SetKernelArg(k, sizeof(uint32_t), &(m_dependency.count)));
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::InitSwScoreBoard()
{
    int SB_BufLen = m_height * m_width;
    int bufIdx = 0;
    int temp_x = 0, temp_y = 0;
    for (int i = 0; i < SB_BufLen; i++)
    {
        int x = i % m_width;
        int y = i / m_width;
        uint32_t entry_value = 0;   //only support for 8 dependencies, but in uint32_t type
        for (uint32_t j = 0; j < m_dependency.count; j++)
        {
            if (((x + m_dependency.deltaX[j]) >= 0) &&
                ((x + m_dependency.deltaX[j]) < (int)m_width)
                && ((y + m_dependency.deltaY[j]) >= 0)
                && ((y + m_dependency.deltaY[j]) < (int)m_height))
            {
                entry_value |= (1 << j);
            }
        }
        switch (m_dependencyPatternType)
        {
            case CM_WAVEFRONT26Z: 
            case CM_WAVEFRONT26ZIG:
                if ((x % 2) == 1 && (y % 2) == 1) {
                    entry_value &= 0xE; // force 0 bit and 4th bit to be zero
                }
                else if ((x % 2) != 0 || (y % 2) != 0) {
                    entry_value &= 0x1E; // force 0 bit to be zero
                }
                break;
            case CM_WAVEFRONT26X:
                if ((y % 4) == 3) {
                    entry_value &= 0x3C; // force 0, 1 and 6th bit of dependency value to be zero. 7th is by default 0
                }
                else if ((y % 4) != 0) {
                    entry_value &= 0x7E; // force 0th bit of dependency value to be zero.
                }
                break;
            case CM_WAVEFRONT26ZI:
                temp_x = x % m_26ZIBlockWidth;
                temp_y = y % m_26ZIBlockHeight;
                if (temp_x == 0) {
                    if (temp_y == m_26ZIBlockHeight - 1)
                        entry_value &= 0x1E;
                    else if (temp_y == 0)
                        entry_value &= 0x3F;
                    else
                        entry_value &= 0x1F;
                }
                else if (temp_x == m_26ZIBlockWidth - 1) {
                    if (m_26ZIBlockWidth % 2 == 0) {
                        if (temp_y == m_26ZIBlockHeight - 1)
                            entry_value &= 0x1E;
                        else if (temp_y == 0)
                            entry_value &= 0x3F;
                        else
                            entry_value &= 0x1F;
                    }
                    else {
                        if (temp_y == 0)
                            entry_value &= 0x1A;
                        else
                            entry_value &= 0x12;
                    }

                }
                else if ((temp_x % 2) != 0) {
                    if (temp_y == m_26ZIBlockHeight - 1)
                        entry_value &= 0x7E;
                }
                else{ // ((temp_x % 2) == 0)
                    if (temp_y == 0)
                        entry_value &= 0x3A;
                    else
                        entry_value &= 0x12;
                }
                break;
            case CM_NONE_DEPENDENCY:
            case CM_WAVEFRONT:
            case CM_WAVEFRONT26:
            case CM_VERTICAL_WAVE:
            case CM_HORIZONTAL_WAVE:
            default:
                break;
        }

        *(m_swBoard + i) = entry_value;
    }
    return CM_SUCCESS;
}

#if CM_LOG_ON
std::string CmThreadSpaceRT::Log()
{
    std::ostringstream oss;

    oss << "Thread Space Parameters"
        << " Width :"<< m_width
        << " Height :" << m_height
        << " DependencyPatten :" << (int)m_dependencyPatternType
        << " IsAssociated :" <<m_threadAssociated
        << std::endl;

    return oss.str();
}

CM_HAL_STATE* CmThreadSpaceRT::GetHalState() { return m_device->GetHalState(); }

#endif  // #if CM_LOG_ON

CmThreadGroupSpace *CmThreadSpaceRT::GetThreadGroupSpace() const
{
    return m_threadGroupSpace;
}
}  // namespace
