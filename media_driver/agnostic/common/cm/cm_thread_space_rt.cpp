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

#if USE_EXTENSION_CODE
#include "cm_thread_space_ext.h"
#endif

enum CM_TS_FLAG
{
    WHITE = 0, 
    GRAY  = 1,
    BLACK = 2
};

static CM_DEPENDENCY WavefrontPattern =
{
    3,
    {-1, -1, 0},
    {0, -1, -1}
};

static CM_DEPENDENCY Wavefront26Pattern =
{
    4,
    {-1, -1, 0, 1},
    {0, -1, -1, -1}
};

static CM_DEPENDENCY Wavefront26ZPattern =
{
    5,
    {-1, -1, -1, 0, 1},
    { 1, 0, -1, -1, -1}
};

static CM_DEPENDENCY Wavefront26ZIPattern =
{
    7,
    {-1, -2, -1, -1, 0, 1, 1},
    {1, 0, 0, -1, -1, -1, 0}
};

static CM_DEPENDENCY HorizontalPattern =
{
    1,
    {0},
    {-1}
};

static CM_DEPENDENCY VerticalPattern =
{
    1,
    {-1},
    {0}
};

static CM_DEPENDENCY Wavefront26XPattern =
{
    7,
    { -1, -1, -1, 0, 0, 0, 1 },
    { 3, 1, -1, -1, -2, -3, -3 }
};

static CM_DEPENDENCY Wavefront26ZIGPattern =
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
int32_t CmThreadSpaceRT::Create( CmDeviceRT* pDevice, uint32_t indexTsArray, uint32_t width, uint32_t height, CmThreadSpaceRT* & pTS )
{
    if( (0 == width) || (0 == height) )
    {
        CM_ASSERTMESSAGE("Error: Invalid thread space width or height.");
        return CM_INVALID_THREAD_SPACE;
    }

    int32_t result = CM_SUCCESS;
    pTS = new (std::nothrow) CmThreadSpaceRT( pDevice, indexTsArray, width, height );
    if( pTS )
    {
        result = pTS->Initialize( );
        if( result != CM_SUCCESS )
        {
            CmThreadSpaceRT::Destroy( pTS);
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
int32_t CmThreadSpaceRT::Destroy( CmThreadSpaceRT* &pTS )
{
    if( pTS )
    {
        delete pTS;
        pTS = nullptr;
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of CmThreadSpace
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmThreadSpaceRT::CmThreadSpaceRT( CmDeviceRT* pDevice , uint32_t indexTsArray, uint32_t width, uint32_t height ):
    m_pDevice( pDevice ),
    m_Width( width ),
    m_Height( height ),
    m_ColorCountMinusOne( 0 ),
    m_26ZIBlockWidth( CM_26ZI_BLOCK_WIDTH ),
    m_26ZIBlockHeight( CM_26ZI_BLOCK_HEIGHT ),
    m_pThreadSpaceUnit(nullptr),
    m_ThreadAssociated(false),
    m_NeedSetKernelPointer(false),
    m_ppKernel(nullptr),
    m_DependencyPatternType(CM_NONE_DEPENDENCY),
    m_CurrentDependencyPattern(CM_NONE_DEPENDENCY),
    m_26ZIDispatchPattern(VVERTICAL_HVERTICAL_26),
    m_Current26ZIDispatchPattern(VVERTICAL_HVERTICAL_26),
    m_pBoardFlag(nullptr),
    m_pBoardOrderList(nullptr),
    m_IndexInList(0),
    m_IndexInTsArray(indexTsArray),
    m_WalkingPattern(CM_WALK_DEFAULT),
    m_MediaWalkerParamsSet(false),
    m_DependencyVectorsSet(false),
    m_pDirtyStatus(nullptr),
    m_groupSelect(CM_MW_GROUP_NONE),
    m_ThreadSpaceOrderSet(false)
{
    CmSafeMemSet( &m_Dependency, 0, sizeof(CM_HAL_DEPENDENCY) );
    CmSafeMemSet( &m_Wavefront26ZDispatchInfo, 0, sizeof(CM_HAL_WAVEFRONT26Z_DISPATCH_INFO) );
    CmSafeMemSet( &m_WalkingParameters, 0, sizeof(m_WalkingParameters) );
    CmSafeMemSet( &m_DependencyVectors, 0, sizeof(m_DependencyVectors) );
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of CmThreadSpaceRT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmThreadSpaceRT::~CmThreadSpaceRT( void )
{
    MosSafeDeleteArray(m_pThreadSpaceUnit);
    MosSafeDeleteArray(m_pBoardFlag);
    MosSafeDeleteArray(m_pBoardOrderList);
    CmSafeDelete( m_pDirtyStatus );
    CmSafeDelete(m_ppKernel);
#if USE_EXTENSION_CODE
    MosSafeDelete(threadSpaceExt);
#endif

    if (m_Wavefront26ZDispatchInfo.pNumThreadsInWave)
    {
        MOS_FreeMemory(m_Wavefront26ZDispatchInfo.pNumThreadsInWave);
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CmThreadSpaceRT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Initialize( void )
{
    m_pDirtyStatus = new (std::nothrow) CM_THREAD_SPACE_DIRTY_STATUS;
    if(m_pDirtyStatus == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmThreadSpace due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    *m_pDirtyStatus = CM_THREAD_SPACE_CLEAN;

    m_ppKernel = new (std::nothrow) CmKernelRT*;
    if (m_ppKernel == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmThreadSpace due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    *m_ppKernel = nullptr;

#if USE_EXTENSION_CODE
    threadSpaceExt = MOS_New(CmThreadSpaceExt, this);
    if (threadSpaceExt == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Failed to initialize CmThreadSpace due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
#endif

    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//! Associate a thread to one uint in the 2-dimensional dependency board with default mask
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::AssociateThread( uint32_t x, uint32_t y, CmKernel* pKernel , uint32_t threadId )
{
    return AssociateThreadWithMask(x, y, pKernel, threadId, CM_DEFAULT_THREAD_DEPENDENCY_MASK);
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
CM_RT_API int32_t CmThreadSpaceRT::AssociateThreadWithMask( uint32_t x, uint32_t y, CmKernel* pKernel , uint32_t threadId, uint8_t dependencyMask )
{
    INSERT_API_CALL_LOG();

    if((x >= m_Width) || (y >= m_Height) || (pKernel == nullptr))
    {
        CM_ASSERTMESSAGE("Error: Invalid input arguments.");
        return CM_INVALID_ARG_VALUE;
    }

    //Check if the m_pThreadSpaceUnit is allocated, we only need allocate it once at the first time.
    if( m_pThreadSpaceUnit == nullptr )
    {
         m_pThreadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_Height * m_Width));
        if (m_pThreadSpaceUnit)
        {
            CmSafeMemSet(m_pThreadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT) * m_Height * m_Width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    uint32_t linear_offset = y*m_Width + x;
    if( (m_pThreadSpaceUnit[linear_offset].pKernel == pKernel) &&
        (m_pThreadSpaceUnit[linear_offset].threadId == threadId) &&
        (m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.x == x) &&
        (m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.y == y) )
    {
        if( m_pThreadSpaceUnit[linear_offset].dependencyMask == dependencyMask )
        {
            m_pThreadSpaceUnit[linear_offset].reset = CM_REUSE_DEPENDENCY_MASK;
        }
        else
        {
            m_pThreadSpaceUnit[linear_offset].dependencyMask = dependencyMask;
            m_pThreadSpaceUnit[linear_offset].reset = CM_RESET_DEPENDENCY_MASK;
        }
        *m_pDirtyStatus = CM_THREAD_SPACE_DEPENDENCY_MASK_DIRTY;
    }
    else
    {
        m_pThreadSpaceUnit[linear_offset].pKernel = pKernel;
        m_pThreadSpaceUnit[linear_offset].threadId = threadId;
        m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.x = x;
        m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.y = y;
        m_pThreadSpaceUnit[linear_offset].dependencyMask = dependencyMask;
        m_pThreadSpaceUnit[linear_offset].reset = CM_NO_BATCH_BUFFER_REUSE;
        *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    if (!m_ThreadAssociated)
    {
        m_ThreadAssociated = true;
    }

    CmKernelRT *pKernelRT = static_cast<CmKernelRT *>(pKernel);
    pKernelRT->SetAssociatedToTSFlag(true);

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
    INSERT_API_CALL_LOG();

    if( count > CM_MAX_DEPENDENCY_COUNT )
    {
        CM_ASSERTMESSAGE("Error: Exceed dependency count limitation, which is 8.");
        return CM_FAILURE;
    }

    m_Dependency.count = count;

    CmSafeMemCopy( m_Dependency.deltaX, deltaX, sizeof( int32_t ) * count );
    CmSafeMemCopy( m_Dependency.deltaY, deltaY, sizeof( int32_t ) * count );

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
    INSERT_API_CALL_LOG();

    int32_t hr = CM_SUCCESS;

     //Check if the m_pBoardFlag and m_pBoardOrderList are NULL. We only need allocate it once at the first time
    if ( m_pBoardFlag == nullptr )
    {
        m_pBoardFlag = MOS_NewArray(uint32_t, (m_Height * m_Width));
        if ( m_pBoardFlag )
        {
            CmSafeMemSet(m_pBoardFlag, 0, sizeof(uint32_t) * m_Height * m_Width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    if ( m_pBoardOrderList == nullptr )
    {
        m_pBoardOrderList = MOS_NewArray(uint32_t, (m_Height * m_Width));
        if (m_pBoardOrderList )
        {
            CmSafeMemSet(m_pBoardOrderList, 0, sizeof(uint32_t) * m_Height * m_Width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(m_pBoardFlag);
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    if( (pattern != CM_NONE_DEPENDENCY) && (m_WalkingPattern != CM_WALK_DEFAULT ) )
    {
        CM_ASSERTMESSAGE("Error: Only valid when no walking pattern has been selected.");
        return CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN;
    }

    switch (pattern)
    {
        case CM_VERTICAL_WAVE:
            m_DependencyPatternType = CM_VERTICAL_WAVE;
            CMCHK_HR(SetThreadDependencyPattern(VerticalPattern.count, VerticalPattern.deltaX, VerticalPattern.deltaY));
            break;

        case CM_HORIZONTAL_WAVE:
            m_DependencyPatternType = CM_HORIZONTAL_WAVE;
            CMCHK_HR(SetThreadDependencyPattern(HorizontalPattern.count, HorizontalPattern.deltaX, HorizontalPattern.deltaY));
            break;

        case CM_WAVEFRONT:
            m_DependencyPatternType = CM_WAVEFRONT;
            CMCHK_HR(SetThreadDependencyPattern(WavefrontPattern.count, WavefrontPattern.deltaX, WavefrontPattern.deltaY));
            break;

        case CM_WAVEFRONT26:
            m_DependencyPatternType = CM_WAVEFRONT26;
            CMCHK_HR(SetThreadDependencyPattern(Wavefront26Pattern.count, Wavefront26Pattern.deltaX, Wavefront26Pattern.deltaY));
            break;

        case CM_WAVEFRONT26Z:
            m_DependencyPatternType = CM_WAVEFRONT26Z;
            CMCHK_HR(SetThreadDependencyPattern(Wavefront26ZPattern.count, Wavefront26ZPattern.deltaX, Wavefront26ZPattern.deltaY));
            m_Wavefront26ZDispatchInfo.pNumThreadsInWave = (uint32_t*)MOS_AllocAndZeroMemory(sizeof(uint32_t) * m_Width * m_Height);
            if (m_pThreadSpaceUnit == nullptr && !CheckThreadSpaceOrderSet())
            {
                m_pThreadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_Height * m_Width));
                if (m_pThreadSpaceUnit)
                {
                    CmSafeMemSet(m_pThreadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT)* m_Height * m_Width);
                }
                else
                {
                    return CM_OUT_OF_HOST_MEMORY;
                }
                uint32_t threadId = 0;
                uint32_t linear_offset = 0;
                for (uint32_t y = 0; y < m_Height; ++y)
                {
                    for (uint32_t x = 0; x < m_Width; ++x)
                    {
                        linear_offset = y*m_Width + x;
                        m_pThreadSpaceUnit[linear_offset].threadId = threadId++;
                        m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.x = x;
                        m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.y = y;
                        m_pThreadSpaceUnit[linear_offset].dependencyMask = (1 << Wavefront26ZPattern.count) - 1;
                        m_pThreadSpaceUnit[linear_offset].reset = CM_NO_BATCH_BUFFER_REUSE;
                    }
                }

                *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
                m_ThreadAssociated = true;
                m_NeedSetKernelPointer = true;
            }
            break;

        case CM_WAVEFRONT26ZI:
            m_DependencyPatternType = CM_WAVEFRONT26ZI;
            CMCHK_HR(SetThreadDependencyPattern(Wavefront26ZIPattern.count, Wavefront26ZIPattern.deltaX, Wavefront26ZIPattern.deltaY));
            if (m_pThreadSpaceUnit == nullptr&& !CheckThreadSpaceOrderSet())
            {
                m_pThreadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_Height * m_Width));
                if (m_pThreadSpaceUnit)
                {
                    CmSafeMemSet(m_pThreadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT)* m_Height * m_Width);
                }
                else
                {
                    return CM_OUT_OF_HOST_MEMORY;
                }
                uint32_t threadId = 0;
                uint32_t linear_offset = 0;
                for (uint32_t y = 0; y < m_Height; ++y)
                {
                    for (uint32_t x = 0; x < m_Width; ++x)
                    {
                        linear_offset = y*m_Width + x;
                        m_pThreadSpaceUnit[linear_offset].threadId = threadId++;
                        m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.x = x;
                        m_pThreadSpaceUnit[linear_offset].scoreboardCoordinates.y = y;
                        m_pThreadSpaceUnit[linear_offset].dependencyMask = (1 << Wavefront26ZIPattern.count) - 1;
                        m_pThreadSpaceUnit[linear_offset].reset = CM_NO_BATCH_BUFFER_REUSE;
                    }
                }

                *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
                m_ThreadAssociated = true;
                m_NeedSetKernelPointer = true;
            }
            break;

        case CM_WAVEFRONT26X:
            m_DependencyPatternType = CM_WAVEFRONT26X;
            CMCHK_HR(SetThreadDependencyPattern(Wavefront26XPattern.count, Wavefront26XPattern.deltaX, Wavefront26XPattern.deltaY));
            break;

        case CM_WAVEFRONT26ZIG:
            m_DependencyPatternType = CM_WAVEFRONT26ZIG;
            CMCHK_HR(SetThreadDependencyPattern(Wavefront26ZIGPattern.count, Wavefront26ZIGPattern.deltaX, Wavefront26ZIGPattern.deltaY));
            break;

        case CM_NONE_DEPENDENCY:
            m_DependencyPatternType = CM_NONE_DEPENDENCY;
            hr = CM_SUCCESS;
            break;

        default:
            hr = CM_FAILURE;
            break;
    }

#if USE_EXTENSION_CODE
    threadSpaceExt->UpdateDependency();
#endif

    if( m_DependencyPatternType != m_CurrentDependencyPattern )
    {
        *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

finish:
    return hr;
}

CM_RT_API int32_t CmThreadSpaceRT::SelectMediaWalkingPattern( CM_WALKING_PATTERN pattern )
{
    INSERT_API_CALL_LOG();

    int result = CM_SUCCESS;

    if( m_DependencyPatternType != CM_NONE_DEPENDENCY )
    {
        CM_ASSERTMESSAGE("Error: Only valid when no thread dependency has been selected.");
        return CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN;
    }

    switch( pattern )
    {
        case CM_WALK_DEFAULT:
            m_WalkingPattern = CM_WALK_DEFAULT;
            break;
        case CM_WALK_HORIZONTAL:
            m_WalkingPattern = CM_WALK_HORIZONTAL;
            break;
        case CM_WALK_VERTICAL:
            m_WalkingPattern = CM_WALK_VERTICAL;
            break;
        case CM_WALK_WAVEFRONT:
            m_WalkingPattern = CM_WALK_WAVEFRONT;
            break;
        case CM_WALK_WAVEFRONT26:
            m_WalkingPattern = CM_WALK_WAVEFRONT26;
            break;
        case CM_WALK_WAVEFRONT26ZIG:
            m_WalkingPattern = CM_WALK_WAVEFRONT26ZIG;
            break;
        case CM_WALK_WAVEFRONT26X:
            m_WalkingPattern = CM_WALK_WAVEFRONT26X;
            break;
        case CM_WALK_WAVEFRONT45D:
            m_WalkingPattern = CM_WALK_WAVEFRONT45D;
            break;
        case CM_WALK_WAVEFRONT45XD_2:
            m_WalkingPattern = CM_WALK_WAVEFRONT45XD_2;
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
    INSERT_API_CALL_LOG();

    // [0..11] of parameters maps to DWORD5 through DWORD16
    // No error checking here

    if( CmSafeMemCompare(&m_WalkingParameters, &parameters, sizeof(m_WalkingParameters)) != 0 )
    {
        CmSafeMemCopy(&m_WalkingParameters, &parameters, sizeof(m_WalkingParameters));
        *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    m_MediaWalkerParamsSet = true;

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
CM_RT_API int32_t CmThreadSpaceRT::SetThreadSpaceOrder(uint32_t threadCount, const CM_THREAD_PARAM* pThreadSpaceOrder)
{
    INSERT_API_CALL_LOG();

    if (threadCount != m_Width*m_Height || pThreadSpaceOrder == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Thread count does not match the thread space size.");
        return CM_INVALID_ARG_VALUE;
    }
    //Check if the m_pThreadSpaceUnit is allocated, we only need allocate it once at the first time.
    if (m_pThreadSpaceUnit == nullptr)
    {
        m_pThreadSpaceUnit = MOS_NewArray(CM_THREAD_SPACE_UNIT, (m_Height * m_Width));
        if (m_pThreadSpaceUnit)
        {
            CmSafeMemSet(m_pThreadSpaceUnit, 0, sizeof(CM_THREAD_SPACE_UNIT)* m_Height * m_Width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    uint32_t threadId = 0;

    for (uint32_t i = 0; i < m_Width*m_Height; i++)
    {
        m_pThreadSpaceUnit[i].threadId = threadId++;
        m_pThreadSpaceUnit[i].scoreboardCoordinates = pThreadSpaceOrder[i].scoreboardCoordinates;
        m_pThreadSpaceUnit[i].scoreboardColor = pThreadSpaceOrder[i].scoreboardColor;
        m_pThreadSpaceUnit[i].sliceDestinationSelect = pThreadSpaceOrder[i].sliceDestinationSelect;
        m_pThreadSpaceUnit[i].subSliceDestinationSelect = pThreadSpaceOrder[i].subSliceDestinationSelect;
        m_pThreadSpaceUnit[i].dependencyMask = CM_DEFAULT_THREAD_DEPENDENCY_MASK;
        m_pThreadSpaceUnit[i].reset = CM_NO_BATCH_BUFFER_REUSE;
    }
    m_ThreadAssociated = true;
    m_NeedSetKernelPointer = true;
    m_ThreadSpaceOrderSet = true;
    *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    return CM_SUCCESS;
}
//*-----------------------------------------------------------------------------
//| Purpose:    Sets the dependency vectors for the CmThreadSpaceRT
//|             Used for engineering build, no error checking
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SelectThreadDependencyVectors(CM_DEPENDENCY dependencyVectors)
{
    INSERT_API_CALL_LOG();

    if( CmSafeMemCompare(&m_DependencyVectors, &dependencyVectors, sizeof(m_DependencyVectors)) != 0 )
    {
        CmSafeMemCopy(&m_DependencyVectors, &dependencyVectors, sizeof(m_DependencyVectors));
        *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    m_DependencyVectorsSet = true;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the color count minus one of the CmThreadSpaceRT
//| Returns:    CM_INVALID_ARG_VALUE if colorCount is 0 or greater than 16
//|             CM_SUCCESS otherwise
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::SetThreadSpaceColorCount(uint32_t colorCount)
{
    INSERT_API_CALL_LOG();

    int32_t result = CM_SUCCESS;

    PCM_HAL_STATE pCmHalState = ((PCM_CONTEXT_DATA)m_pDevice->GetAccelData())->pCmHalState;

    result = pCmHalState->pCmHalInterface->ColorCountSanityCheck(colorCount);
    if(result != CM_SUCCESS)
    {
        CM_ASSERTMESSAGE("Error: Color count sanity check failure.");
        return result;
    }

    m_ColorCountMinusOne = colorCount - 1;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the dispatch pattern for 26ZI
//| Returns:    CM_SUCCESS if valid dispath pattern, CM_FAILURE otherwise
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::Set26ZIDispatchPattern( CM_26ZI_DISPATCH_PATTERN pattern )
{
    INSERT_API_CALL_LOG();

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

    if( m_26ZIDispatchPattern != m_Current26ZIDispatchPattern)
    {
        *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

     return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Sets the macro block size to be used to calculate 26ZI dispatch
//| Returns:    CM_SUCCESS
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmThreadSpaceRT::Set26ZIMacroBlockSize( uint32_t width, uint32_t height )
{
    INSERT_API_CALL_LOG();
    int32_t hr = CM_SUCCESS;
    m_26ZIBlockWidth = width;
    m_26ZIBlockHeight = height;
#if USE_EXTENSION_CODE
    hr = threadSpaceExt->UpdateDependency();
#endif
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the color count of the CmThreadSpaceRT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetColorCountMinusOne(uint32_t & colorCount)
{
    colorCount = m_ColorCountMinusOne;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the width and height of CmThreadSpaceRT
//| Returns:    CM_SUCCESS.
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetThreadSpaceSize(uint32_t & width, uint32_t & height)
{
    width = m_Width;
    height = m_Height;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get thread space's unit
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetThreadSpaceUnit(CM_THREAD_SPACE_UNIT* &pThreadSpaceUnit)
{
    pThreadSpaceUnit = m_pThreadSpaceUnit;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the dependency
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetDependency(CM_HAL_DEPENDENCY* &pDependency)
{
    pDependency = &m_Dependency;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get its dependency type
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetDependencyPatternType(CM_DEPENDENCY_PATTERN &DependencyPatternType)
{
    DependencyPatternType = m_DependencyPatternType;

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
int32_t CmThreadSpaceRT::GetWalkingPattern(CM_WALKING_PATTERN &pWalkingPattern)
{
    pWalkingPattern = m_WalkingPattern;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get media walking parameters
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetWalkingParameters(CM_WALKING_PARAMETERS &pWalkingParameters)
{
    CmSafeMemCopy(&pWalkingParameters, &m_WalkingParameters, sizeof(m_WalkingParameters));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return true if media walker parameters are set, false otherwise
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::CheckWalkingParametersSet( )
{
    return m_MediaWalkerParamsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get dependency vectors
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetDependencyVectors(CM_HAL_DEPENDENCY &pDependencyVectors)
{
    CmSafeMemCopy(&pDependencyVectors, &m_DependencyVectors, sizeof(m_DependencyVectors));
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return true if dependency vectors are set, false otherwise
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::CheckDependencyVectorsSet( )
{
    return m_DependencyVectorsSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Return true if thread space order is set, false otherwise
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::CheckThreadSpaceOrderSet()
{
    return m_ThreadSpaceOrderSet;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Wavefront26ZDispatchInfo
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetWavefront26ZDispatchInfo(CM_HAL_WAVEFRONT26Z_DISPATCH_INFO &dispatchInfo)
{
    dispatchInfo = m_Wavefront26ZDispatchInfo;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Check the integrity of thread space' association
//*-----------------------------------------------------------------------------
bool CmThreadSpaceRT::IntegrityCheck(CmTaskRT* pTask)
{
    CmKernelRT *pKernel_RT = nullptr;
    uint32_t i;
    uint32_t KernelCount = 0;
    uint32_t ThreadNumber = 0;
    uint32_t KernelIndex = 0;
    uint32_t unassociated = 0;
    int32_t hr = CM_SUCCESS;

    byte **pTSMapping = nullptr;
    byte *pKernelInScoreboard = nullptr;

    KernelCount = pTask->GetKernelCount();
    //Check if it is mult-kernel task, since no threadspace is allowed for multi-kernel tasks
    if (KernelCount > 1)
    {
        CM_ASSERTMESSAGE("Error: pTS->IntegrityCheck Failed: ThreadSpace is not allowed in multi-kernel task.");
        return false;
    }

    pKernel_RT = pTask->GetKernelPointer(0);
    CMCHK_NULL(pKernel_RT);

    //To check if the thread space size is matched with thread count
    pKernel_RT->GetThreadCount(ThreadNumber);

    //Till now, all disallowed settings are abort, now we need check if the thread space association is correct.
    if (this->IsThreadAssociated())
    {
        //For future extending to multiple kernels cases, we're using a general mechanism to check the integrity

        pTSMapping = MOS_NewArray(byte*, KernelCount);
        pKernelInScoreboard = MOS_NewArray(byte, KernelCount);

        CMCHK_NULL(pTSMapping);
        CMCHK_NULL(pKernelInScoreboard);

        CmSafeMemSet(pTSMapping, 0, KernelCount*sizeof(byte *));
        CmSafeMemSet(pKernelInScoreboard, 0, KernelCount*sizeof(byte));

        for (i = 0; i < KernelCount; i++)
        {
            pKernel_RT = pTask->GetKernelPointer(i);
            CMCHK_NULL(pKernel_RT);
            pKernel_RT->GetThreadCount(ThreadNumber);
            if (ThreadNumber == 0)
            {
                ThreadNumber = m_Width * m_Height;
            }
            pTSMapping[i] = MOS_NewArray(byte, ThreadNumber);
            CMCHK_NULL(pTSMapping[i]);
            CmSafeMemSet(pTSMapping[i], 0, ThreadNumber * sizeof(byte));
            pKernelInScoreboard[i] = 0;
        }

        for (i = 0; i < m_Width * m_Height; i ++ )
        {
            pKernel_RT = static_cast<CmKernelRT *> (m_pThreadSpaceUnit[i].pKernel);
            if (pKernel_RT == nullptr)
            {
                if (m_NeedSetKernelPointer)
                {
                    pKernel_RT = *m_ppKernel;
                }
            }
            CMCHK_NULL(pKernel_RT);

            KernelIndex = pKernel_RT->GetIndexInTask();
            pTSMapping[KernelIndex][m_pThreadSpaceUnit[i].threadId] = 1;
            pKernelInScoreboard[KernelIndex] = 1;
        }

        for (i = 0; i < KernelCount; i ++)
        {
            if(pKernelInScoreboard[i])
            {
                pKernel_RT = pTask->GetKernelPointer(i);
                CMCHK_NULL(pKernel_RT);

                pKernel_RT->GetThreadCount(ThreadNumber);
                if (ThreadNumber == 0)
                {
                    ThreadNumber = m_Width * m_Height;
                }
                pKernel_RT->SetAssociatedToTSFlag(true);
                for (uint32_t j = 0; j < ThreadNumber; j++)
                {
                    if (pTSMapping[i][j] == 0)
                    {
                        unassociated ++;
                        break;
                    }
                }
            }
            MosSafeDeleteArray(pTSMapping[i]);
        }

        if (unassociated != 0)
        {
            CM_ASSERTMESSAGE("Error: The thread space association is not correct.");
            hr = CM_FAILURE;
        }
    }

finish:

    MosSafeDeleteArray(pTSMapping);
    MosSafeDeleteArray(pKernelInScoreboard);

    return (hr == CM_SUCCESS)? true: false;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Generate Wave45 Sequence
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::Wavefront45Sequence()
{
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT )
    {
        return CM_SUCCESS;
    }
    m_CurrentDependencyPattern = CM_WAVEFRONT;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for (uint32_t y = 0; y < m_Height; y ++)
    {
        for (uint32_t x = 0; x < m_Width; x ++)
        {
            CM_COORDINATE temp_xy;
            int32_t linear_offset = y * m_Width + x;
            if (m_pBoardFlag[linear_offset] == WHITE)
            {
                m_pBoardOrderList[m_IndexInList ++] = linear_offset;
                m_pBoardFlag[linear_offset] = BLACK;
                temp_xy.x = x - 1;
                temp_xy.y = y + 1;
                while ((temp_xy.x >= 0) && (temp_xy.y >= 0) &&
                    (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height))
                {
                    if (m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xy.y * m_Width + temp_xy.x;
                        m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                    }
                    temp_xy.x = temp_xy.x - 1;
                    temp_xy.y = temp_xy.y + 1;
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
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26 )
    {
        return CM_SUCCESS;
    }
    m_CurrentDependencyPattern = CM_WAVEFRONT26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for (uint32_t y = 0; y < m_Height; y ++)
    {
        for (uint32_t x = 0; x < m_Width; x ++)
        {
            CM_COORDINATE temp_xy;
            int32_t linear_offset = y * m_Width + x;
            if (m_pBoardFlag[linear_offset] == WHITE)
            {
                m_pBoardOrderList[m_IndexInList ++] = linear_offset;
                m_pBoardFlag[linear_offset] = BLACK;
                temp_xy.x = x - 2;
                temp_xy.y = y + 1;
                while ((temp_xy.x >= 0) && (temp_xy.y >= 0) &&
                    (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height))
                {
                    if (m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xy.y * m_Width + temp_xy.x;
                        m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                    }
                    temp_xy.x = temp_xy.x - 2;
                    temp_xy.y = temp_xy.y + 1;
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
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26Z )
    {
        return CM_SUCCESS;
    }
    m_CurrentDependencyPattern = CM_WAVEFRONT26Z;

    uint32_t threadsInWave = 0;
    uint32_t numWaves = 0;

    if ( ( m_Height % 2 != 0 ) || ( m_Width % 2 != 0 ) )
    {
        return CM_INVALID_ARG_SIZE;
    }
    CmSafeMemSet( m_pBoardFlag, WHITE, m_Width * m_Height * sizeof( uint32_t ) );
    m_IndexInList = 0;

    uint32_t iX, iY, nOffset;
    iX = iY = nOffset = 0;

    uint32_t *pWaveFrontPos = MOS_NewArray(uint32_t, m_Width);
    uint32_t *pWaveFrontOffset = MOS_NewArray(uint32_t, m_Width);
    if ( ( pWaveFrontPos == nullptr ) || ( pWaveFrontOffset == nullptr ) )
    {
        MosSafeDeleteArray( pWaveFrontPos );
        MosSafeDeleteArray( pWaveFrontOffset );
        return CM_FAILURE;
    }
    CmSafeMemSet( pWaveFrontPos, 0, m_Width * sizeof( int ) );

    // set initial value
    m_pBoardFlag[ 0 ] = BLACK;
    m_pBoardOrderList[ 0 ] = 0;
    pWaveFrontPos[ 0 ] = 1;
    m_IndexInList = 0;

    CM_COORDINATE pMask[ 8 ];
    uint32_t nMaskNumber = 0;

    m_Wavefront26ZDispatchInfo.pNumThreadsInWave[numWaves] = 1;
    numWaves++;

    while ( m_IndexInList < m_Width * m_Height - 1 )
    {

        CmSafeMemSet( pWaveFrontOffset, 0, m_Width * sizeof( int ) );
        for ( uint32_t iX = 0; iX < m_Width; ++iX )
        {
            uint32_t iY = pWaveFrontPos[ iX ];
            nOffset = iY * m_Width + iX;
            CmSafeMemSet( pMask, 0, sizeof( pMask ) );

            if ( m_pBoardFlag[ nOffset ] == WHITE )
            {
                if ( ( iX % 2 == 0 ) && ( iY % 2 == 0 ) )
                {
                    if ( iX == 0 )
                    {
                        pMask[ 0 ].x = 0;
                        pMask[ 0 ].y = -1;
                        pMask[ 1 ].x = 1;
                        pMask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else if ( iY == 0 )
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 1;
                        pMask[ 1 ].x = -1;
                        pMask[ 1 ].y = 0;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 1;
                        pMask[ 1 ].x = -1;
                        pMask[ 1 ].y = 0;
                        pMask[ 2 ].x = 0;
                        pMask[ 2 ].y = -1;
                        pMask[ 3 ].x = 1;
                        pMask[ 3 ].y = -1;
                        nMaskNumber = 4;
                    }
                }
                else if ( ( iX % 2 == 0 ) && ( iY % 2 == 1 ) )
                {
                    if ( iX == 0 )
                    {
                        pMask[ 0 ].x = 0;
                        pMask[ 0 ].y = -1;
                        pMask[ 1 ].x = 1;
                        pMask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        pMask[ 1 ].x = 0;
                        pMask[ 1 ].y = -1;
                        pMask[ 2 ].x = 1;
                        pMask[ 2 ].y = -1;
                        nMaskNumber = 3;
                    }
                }
                else if ( ( iX % 2 == 1 ) && ( iY % 2 == 0 ) )
                {
                    if ( iY == 0 )
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        nMaskNumber = 1;
                    }
                    else if ( iX == m_Width - 1 )
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        pMask[ 1 ].x = 0;
                        pMask[ 1 ].y = -1;
                        nMaskNumber = 2;
                    }
                    else
                    {
                        pMask[ 0 ].x = -1;
                        pMask[ 0 ].y = 0;
                        pMask[ 1 ].x = 0;
                        pMask[ 1 ].y = -1;
                        pMask[ 2 ].x = 1;
                        pMask[ 2 ].y = -1;
                        nMaskNumber = 3;
                    }
                }
                else
                {
                    pMask[ 0 ].x = -1;
                    pMask[ 0 ].y = 0;
                    pMask[ 1 ].x = 0;
                    pMask[ 1 ].y = -1;
                    nMaskNumber = 2;
                }

                // check if all of the dependencies are in the dispatch queue
                bool bAllInQueue = true;
                for ( uint32_t i = 0; i < nMaskNumber; ++i )
                {
                    if ( m_pBoardFlag[ nOffset + pMask[ i ].x + pMask[ i ].y * m_Width ] == WHITE )
                    {
                        bAllInQueue = false;
                        break;
                    }
                }
                if ( bAllInQueue )
                {
                    pWaveFrontOffset[ iX ] = nOffset;
                    if( pWaveFrontPos[ iX ] < m_Height - 1 )
                    {
                        pWaveFrontPos[ iX ]++;
                    }
                }
            }
        }


        for ( uint32_t iX = 0; iX < m_Width; ++iX )
        {
            if ( ( m_pBoardFlag[ pWaveFrontOffset[ iX ] ] == WHITE ) && ( pWaveFrontOffset[ iX ] != 0 ) )
            {
                m_IndexInList++;
                m_pBoardOrderList[ m_IndexInList ] = pWaveFrontOffset[ iX ];
                m_pBoardFlag[ pWaveFrontOffset[ iX ] ] = BLACK;
                threadsInWave++;
            }
        }

        m_Wavefront26ZDispatchInfo.pNumThreadsInWave[numWaves] = threadsInWave;
        threadsInWave = 0;
        numWaves++;
    }

    MosSafeDeleteArray( pWaveFrontPos );
    MosSafeDeleteArray( pWaveFrontOffset );

    m_Wavefront26ZDispatchInfo.numWaves = numWaves;

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
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26ZI  &&
        ( m_Current26ZIDispatchPattern == VVERTICAL_HVERTICAL_26 ) )
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL_HVERTICAL_26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for( uint32_t y = 0; y < m_Height; y = y + m_26ZIBlockHeight )
    {
        for( uint32_t x = 0; x < m_Width; x = x + m_26ZIBlockWidth )
        {
            CM_COORDINATE temp_xyFor26;
            temp_xyFor26.x = x;
            temp_xyFor26.y = y;

            do
            {
                if( m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] == WHITE )
                {
                    m_pBoardOrderList[m_IndexInList ++] = temp_xyFor26.y * m_Width + temp_xyFor26.x;
                    m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] = BLACK;

                    // do vertical edges
                    for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localHeightCounter = 0;

                        temp_xy.x = temp_xyFor26.x + widthCount;
                        temp_xy.y = temp_xyFor26.y;
                        while( (temp_xy.x >= 0) && (temp_xy.y >=0) &&
                            (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.y = temp_xy.y + 1;
                            localHeightCounter++;
                        }
                    } // vertical edges

                     // do horizontal edges
                    for( uint32_t widthCount = 1; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localHeightCounter = 0;

                        temp_xy.x = temp_xyFor26.x + widthCount;
                        temp_xy.y = temp_xyFor26.y;
                        while( (temp_xy.x >= 0) && (temp_xy.y >=0) &&
                            (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.y = temp_xy.y + 1;
                            localHeightCounter++;
                        }
                    } // horizontal edges
                }

                temp_xyFor26.x = temp_xyFor26.x - (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y + (1 * m_26ZIBlockHeight);

            } while( ( temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0)
                && (temp_xyFor26.x < (int32_t)m_Width) && ( temp_xyFor26.y < (int32_t)m_Height));
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
    if ( m_CurrentDependencyPattern == CM_WAVEFRONT26ZI &&
        ( m_Current26ZIDispatchPattern == VVERTICAL_HHORIZONTAL_26))
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL_HHORIZONTAL_26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for( uint32_t y = 0; y < m_Height; y = y + m_26ZIBlockHeight )
    {
        for( uint32_t x = 0; x < m_Width; x = x + m_26ZIBlockWidth )
        {
            CM_COORDINATE temp_xyFor26;
            temp_xyFor26.x = x;
            temp_xyFor26.y = y;

            do
            {
                if( m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] == WHITE )
                {
                    m_pBoardOrderList[m_IndexInList ++] = temp_xyFor26.y * m_Width + temp_xyFor26.x;
                    m_pBoardFlag[temp_xyFor26.y * m_Width + temp_xyFor26.x] = BLACK;

                    // do vertical edges
                    for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount = widthCount + 2 )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localHeightCounter = 0;

                        temp_xy.x = temp_xyFor26.x + widthCount;
                        temp_xy.y = temp_xyFor26.y;
                        while( (temp_xy.x >= 0) && (temp_xy.y >=0) &&
                            (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localHeightCounter < m_26ZIBlockHeight))
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }
                            temp_xy.y = temp_xy.y + 1;
                            localHeightCounter++;
                        }
                    } // vertical edges

                    // horizontal edges
                    for( uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
                    {
                        CM_COORDINATE temp_xy;
                        uint32_t localWidthCounter = 0;

                        temp_xy.x = temp_xyFor26.x + 1;
                        temp_xy.y = temp_xyFor26.y + heightCount;
                        while ( (temp_xy.x >= 0) && (temp_xy.y >= 0) &&
                            (temp_xy.x< (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height) &&
                            (localWidthCounter < (m_26ZIBlockWidth / 2) ) )
                        {
                            if( m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                            {
                                m_pBoardOrderList[m_IndexInList ++ ] = temp_xy.y * m_Width + temp_xy.x;
                                m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                            }

                            temp_xy.x = temp_xy.x + 2;
                            localWidthCounter++;
                        }
                    }
                    // horizontal edges
                }

                temp_xyFor26.x = temp_xyFor26.x - (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y + (1 * m_26ZIBlockHeight);

            } while( ( temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0)
                && (temp_xyFor26.x < (int32_t)m_Width) && ( temp_xyFor26.y < (int32_t)m_Height));
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
    if( (m_CurrentDependencyPattern == CM_WAVEFRONT26ZI) &&
        (m_Current26ZIDispatchPattern == VVERTICAL26_HHORIZONTAL26) )
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL26_HHORIZONTAL26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    uint32_t waveFrontNum = 0;
    uint32_t waveFrontStartX = 0;
    uint32_t waveFrontStartY = 0;

    uint32_t adjustHeight = 0;

    CM_COORDINATE temp_xyFor26;
    temp_xyFor26.x = 0;
    temp_xyFor26.y = 0;

    while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
        (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) )
    {
        // use horizontal coordinates to save starting (x,y) for overall 26
        CM_COORDINATE temp_xyForHorz;
        temp_xyForHorz.x = temp_xyFor26.x;
        temp_xyForHorz.y = temp_xyFor26.y;

       do
        {
            CM_COORDINATE temp_xyForVer;

            for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount += 2 )
            {
                uint32_t localHeightCounter = 0;
                temp_xyForVer.x = temp_xyFor26.x + widthCount;
                temp_xyForVer.y = temp_xyFor26.y;

                while( (temp_xyForVer.x < (int32_t)m_Width) && (temp_xyForVer.y < (int32_t)m_Height) &&
                        (temp_xyForVer.x >= 0) && (temp_xyForVer.y >= 0) && (localHeightCounter < m_26ZIBlockHeight) )
                {
                    if(m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForVer.y * m_Width + temp_xyForVer.x;
                        m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] = BLACK;
                    }
                    temp_xyForVer.y += 1;
                    localHeightCounter++;
                }
            }

            temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
            temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

        } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );

        temp_xyFor26.x = temp_xyForHorz.x;
        temp_xyFor26.y = temp_xyForHorz.y;

        do
        {
            // do horizontal edges
            for ( uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
            {
                uint32_t localWidthCounter = 0;
                temp_xyForHorz.x = temp_xyFor26.x + 1;
                temp_xyForHorz.y = temp_xyFor26.y + heightCount;
                while( (temp_xyForHorz.x >= 0) && (temp_xyForHorz.y >= 0) &&
                    (temp_xyForHorz.x < (int32_t)m_Width) && (temp_xyForHorz.y < (int32_t)m_Height) &&
                    (localWidthCounter < (m_26ZIBlockWidth / 2)) )
                {
                    if( m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForHorz.y * m_Width + temp_xyForHorz.x;
                        m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] = BLACK;
                    }

                    temp_xyForHorz.x += 2;
                    localWidthCounter++;
                }
            }


            temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
            temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

        } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );

        if (m_Width <= m_26ZIBlockWidth)
        {
            temp_xyFor26.x = 0;
            temp_xyFor26.y = temp_xyForHorz.y + m_26ZIBlockHeight;
        }
        else
        {
            // update wavefront number
            waveFrontNum++;
            adjustHeight = (uint32_t)ceil((double)m_Height / m_26ZIBlockHeight);

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

            temp_xyFor26.x = waveFrontStartX * m_26ZIBlockWidth;
            temp_xyFor26.y = waveFrontStartY * m_26ZIBlockHeight;
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
    if ( (m_CurrentDependencyPattern == CM_WAVEFRONT26ZI) &&
        (m_Current26ZIDispatchPattern == VVERTICAL1X26_HHORIZONTAL1X26))
    {
        return CM_SUCCESS;
    }

    m_CurrentDependencyPattern = CM_WAVEFRONT26ZI;
    m_Current26ZIDispatchPattern = VVERTICAL1X26_HHORIZONTAL1X26;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    uint32_t waveFrontNum = 0;
    uint32_t waveFrontStartX = 0;
    uint32_t waveFrontStartY = 0;

    uint32_t adjustHeight = 0;

    CM_COORDINATE temp_xyFor26;
    temp_xyFor26.x = 0;
    temp_xyFor26.y = 0;

    CM_COORDINATE saveTemp_xyFor26;
    saveTemp_xyFor26.x = 0;
    saveTemp_xyFor26.y = 0;

    CM_COORDINATE temp_xyForVer;
    CM_COORDINATE temp_xyForHorz;

    while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
        (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) )
    {
        saveTemp_xyFor26.x = temp_xyFor26.x;
        saveTemp_xyFor26.y = temp_xyFor26.y;

        // do vertical edges
        for( uint32_t widthCount = 0; widthCount < m_26ZIBlockWidth; widthCount += 2 )
        {
            // restore original starting point
            temp_xyFor26.x = saveTemp_xyFor26.x;
            temp_xyFor26.y = saveTemp_xyFor26.y;

            do
            {
                uint32_t localHeightCounter = 0;
                temp_xyForVer.x = temp_xyFor26.x + widthCount;
                temp_xyForVer.y = temp_xyFor26.y;
                while( (temp_xyForVer.x < (int32_t)m_Width) && (temp_xyForVer.y < (int32_t)m_Height) &&
                        (temp_xyForVer.x >= 0) && (temp_xyForVer.y >= 0) && (localHeightCounter < m_26ZIBlockHeight) )
                {
                    if(m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForVer.y * m_Width + temp_xyForVer.x;
                        m_pBoardFlag[temp_xyForVer.y * m_Width + temp_xyForVer.x] = BLACK;
                    }
                    temp_xyForVer.y += 1;
                    localHeightCounter++;
                }

                temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

            } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );
        }

        // do horizontal edges
        // restore original starting position
        temp_xyFor26.x = saveTemp_xyFor26.x;
        temp_xyFor26.y = saveTemp_xyFor26.y;

        for(uint32_t heightCount = 0; heightCount < m_26ZIBlockHeight; ++heightCount )
        {
            // restore original starting point
            temp_xyFor26.x = saveTemp_xyFor26.x;
            temp_xyFor26.y = saveTemp_xyFor26.y;

            do
            {
                uint32_t localWidthCounter = 0;
                temp_xyForHorz.x = temp_xyFor26.x + 1;
                temp_xyForHorz.y = temp_xyFor26.y + heightCount;
                while( (temp_xyForHorz.x >= 0) && (temp_xyForHorz.y >= 0) &&
                    (temp_xyForHorz.x < (int32_t)m_Width) && (temp_xyForHorz.y < (int32_t)m_Height) &&
                    (localWidthCounter < (m_26ZIBlockWidth / 2)) )
                {
                    if( m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] == WHITE )
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xyForHorz.y * m_Width + temp_xyForHorz.x;
                        m_pBoardFlag[temp_xyForHorz.y * m_Width + temp_xyForHorz.x] = BLACK;
                    }

                    temp_xyForHorz.x += 2;
                    localWidthCounter++;
                }

                temp_xyFor26.x = temp_xyFor26.x + (2 * m_26ZIBlockWidth);
                temp_xyFor26.y = temp_xyFor26.y - ( 1 * m_26ZIBlockHeight);

            } while( (temp_xyFor26.x >= 0) && (temp_xyFor26.y >= 0) &&
            (temp_xyFor26.x < (int32_t)m_Width) && (temp_xyFor26.y < (int32_t)m_Height) );

        }

        if (m_Width <= m_26ZIBlockWidth)
        {
            temp_xyFor26.x = 0;
            temp_xyFor26.y = saveTemp_xyFor26.y + m_26ZIBlockHeight;
        }
        else
        {
            // update wavefront number
            waveFrontNum++;
            adjustHeight = (uint32_t)ceil((double)m_Height / m_26ZIBlockHeight);

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

            temp_xyFor26.x = waveFrontStartX * m_26ZIBlockWidth;
            temp_xyFor26.y = waveFrontStartY * m_26ZIBlockHeight;
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::VerticalSequence()
{
    if ( m_CurrentDependencyPattern == CM_VERTICAL_WAVE)
    {
        return CM_SUCCESS;
    }
    m_CurrentDependencyPattern = CM_VERTICAL_WAVE;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for (uint32_t x = 0; x < m_Width; x ++)
    {
        for (uint32_t y = 0; y < m_Height; y ++)
        {
            CM_COORDINATE temp_xy;
            int32_t linear_offset = y * m_Width + x;
            if (m_pBoardFlag[linear_offset] == WHITE)
            {
                m_pBoardOrderList[m_IndexInList ++] = linear_offset;
                m_pBoardFlag[linear_offset] = BLACK;
                temp_xy.x = x;
                temp_xy.y = y + 1;
                while ((temp_xy.x >= 0) && (temp_xy.y >= 0) &&
                    (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height))
                {
                    if (m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xy.y * m_Width + temp_xy.x;
                        m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                    }
                    temp_xy.y = temp_xy.y + 1;
                }
            }
        }
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::HorizentalSequence()
{
    if ( m_CurrentDependencyPattern == CM_HORIZONTAL_WAVE)
    {
        return CM_SUCCESS;
    }
    m_CurrentDependencyPattern = CM_HORIZONTAL_WAVE;

    CmSafeMemSet(m_pBoardFlag, WHITE, m_Width*m_Height*sizeof(uint32_t));
    m_IndexInList = 0;

    for (uint32_t y = 0; y < m_Height; y ++)
    {
        for (uint32_t x = 0; x < m_Width; x ++)
        {
            CM_COORDINATE temp_xy;
            int32_t linear_offset = y * m_Width + x;
            if (m_pBoardFlag[linear_offset] == WHITE)
            {
                m_pBoardOrderList[m_IndexInList ++] = linear_offset;
                m_pBoardFlag[linear_offset] = BLACK;
                temp_xy.x = x + 1;
                temp_xy.y = y;
                while ((temp_xy.x >= 0) && (temp_xy.y >= 0) &&
                    (temp_xy.x < (int32_t)m_Width) && (temp_xy.y < (int32_t)m_Height))
                {
                    if (m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] == WHITE)
                    {
                        m_pBoardOrderList[m_IndexInList ++] = temp_xy.y * m_Width + temp_xy.x;
                        m_pBoardFlag[temp_xy.y * m_Width + temp_xy.x] = BLACK;
                    }
                    temp_xy.x = temp_xy.x + 1;
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
    if (m_pBoardFlag == nullptr)
    {
        m_pBoardFlag = MOS_NewArray(uint32_t, (m_Height * m_Width));
        if (m_pBoardFlag)
        {
            CmSafeMemSet(m_pBoardFlag, WHITE, (sizeof(uint32_t)* m_Height * m_Width));
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    if (m_pBoardOrderList == nullptr)
    {
        m_pBoardOrderList = MOS_NewArray(uint32_t, (m_Height * m_Width));
        if (m_pBoardOrderList)
        {
            CmSafeMemSet(m_pBoardOrderList, 0, sizeof(uint32_t)* m_Height * m_Width);
        }
        else
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(m_pBoardFlag);
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    uint32_t iX, iY, nOffset;
    iX = iY = nOffset = 0;

    uint32_t *pWaveFrontPos = MOS_NewArray(uint32_t, m_Width);
    uint32_t *pWaveFrontOffset = MOS_NewArray(uint32_t, m_Width);
    if ((pWaveFrontPos == nullptr) || (pWaveFrontOffset == nullptr))
    {
        MosSafeDeleteArray(pWaveFrontPos);
        MosSafeDeleteArray(pWaveFrontOffset);
        return CM_FAILURE;
    }
    CmSafeMemSet(pWaveFrontPos, 0, m_Width * sizeof(int));

    // set initial value
    m_pBoardFlag[0] = BLACK;
    m_pBoardOrderList[0] = 0;
    pWaveFrontPos[0] = 1;
    m_IndexInList = 0;

    while (m_IndexInList < m_Width * m_Height - 1)
    {
        CmSafeMemSet(pWaveFrontOffset, 0, m_Width * sizeof(int));
        for (uint32_t iX = 0; iX < m_Width; ++iX)
        {
            uint32_t iY = pWaveFrontPos[iX];
            nOffset = iY * m_Width + iX;
            if (m_pBoardFlag[nOffset] == WHITE)
            {
                // check if all of the dependencies are in the dispatch queue
                bool bAllInQueue = true;
                for (uint32_t i = 0; i < m_DependencyVectors.count; ++i)
                {
                    uint32_t tempOffset = nOffset + m_DependencyVectors.deltaX[i] + m_DependencyVectors.deltaY[i] * m_Width;
                    if (tempOffset <= m_Width * m_Height - 1)
                    {
                        if (m_pBoardFlag[nOffset + m_DependencyVectors.deltaX[i] + m_DependencyVectors.deltaY[i] * m_Width] == WHITE)
                        {
                            bAllInQueue = false;
                            break;
                        }
                    }
                }
                if (bAllInQueue)
                {
                    pWaveFrontOffset[iX] = nOffset;
                    if (pWaveFrontPos[iX] < m_Height - 1)
                    {
                        pWaveFrontPos[iX]++;
                    }
                }
            }
        }

        for (uint32_t iX = 0; iX < m_Width; ++iX)
        {
            if ((m_pBoardFlag[pWaveFrontOffset[iX]] == WHITE) && (pWaveFrontOffset[iX] != 0))
            {
                m_IndexInList++;
                m_pBoardOrderList[m_IndexInList] = pWaveFrontOffset[iX];
                m_pBoardFlag[pWaveFrontOffset[iX]] = BLACK;
            }
        }
    }

    MosSafeDeleteArray(pWaveFrontPos);
    MosSafeDeleteArray(pWaveFrontOffset);
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Board Order list
//*-----------------------------------------------------------------------------
int32_t CmThreadSpaceRT::GetBoardOrder(uint32_t *&pBoardOrder)
{
    pBoardOrder = m_pBoardOrderList;
    return CM_SUCCESS;
}

#ifdef _DEBUG
int32_t CmThreadSpaceRT::PrintBoardOrder()
{
    CM_NORMALMESSAGE("According to dependency, the score board order is:");
    for (uint32_t i = 0; i < m_Height * m_Width; i ++)
    {
        CM_NORMALMESSAGE("%d->", m_pBoardOrderList[i]);
    }
    CM_NORMALMESSAGE("NIL.");
    return 0;
}
#endif

bool CmThreadSpaceRT::IsThreadAssociated() const
{
    return m_ThreadAssociated;
}

bool CmThreadSpaceRT::IsDependencySet()
{
    return ((m_DependencyPatternType != CM_NONE_DEPENDENCY) ? true : false);
}

bool CmThreadSpaceRT::GetNeedSetKernelPointer() const
{
    return m_NeedSetKernelPointer;
}

int32_t CmThreadSpaceRT::SetKernelPointer(CmKernelRT* pKernel) const
{
    *m_ppKernel = pKernel;
    return CM_SUCCESS;
}

bool CmThreadSpaceRT::KernelPointerIsNULL() const
{
    if (*m_ppKernel == nullptr)
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
    return *m_ppKernel;
}

uint32_t CmThreadSpaceRT::GetIndexInTsArray()
{
    return m_IndexInTsArray;
}

CM_THREAD_SPACE_DIRTY_STATUS CmThreadSpaceRT::GetDirtyStatus() const
{
    return *m_pDirtyStatus;
}

uint32_t CmThreadSpaceRT::SetDirtyStatus(CM_THREAD_SPACE_DIRTY_STATUS DirtyStatus) const
{
    *m_pDirtyStatus = DirtyStatus;
    return CM_SUCCESS;
}

CM_RT_API int32_t CmThreadSpaceRT::SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect)
{
    if (groupSelect != m_groupSelect)
    {
        m_groupSelect = groupSelect;
        *m_pDirtyStatus = CM_THREAD_SPACE_DATA_DIRTY;
    }

    return CM_SUCCESS;
}

int32_t CmThreadSpaceRT::GetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT &groupSelect)
{
    groupSelect = m_groupSelect;
    return CM_SUCCESS;
}

#if CM_LOG_ON
std::string CmThreadSpaceRT::Log()
{
    std::ostringstream oss;

    oss << "Thread Space Parameters"
        << " Width :"<< m_Width
        << " Height :" << m_Height
        << " DependencyPatten :" << (int)m_DependencyPatternType
        << " IsAssociated :" <<m_ThreadAssociated
        << std::endl;

    return oss.str();
}
#endif
}