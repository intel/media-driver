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
//! \file      cm_kernel_rt.cpp
//! \brief     Contains CmKernelRT definitions.
//!

#include "cm_kernel_rt.h"

#include "cm_program.h"
#include "cm_device_rt.h"
#include "cm_surface_manager.h"
#include "cm_surface_2d_up_rt.h"
#include "cm_surface_3d_rt.h"
#include "cm_buffer_rt.h"
#include "cm_mov_inst.h"
#include "cm_kernel_data.h"
#include "cm_thread_space_rt.h"
#include "cm_state_buffer.h"
#include "cm_surface_vme.h"
#include "cm_debug.h"
#include "cm_surface_sampler8x8.h"
#include "cm_surface_sampler.h"
#include "cm_group_space.h"
#include "cm_surface_2d_rt.h"
#include "cm_sampler8x8_state_rt.h"
#include "cm_visa.h"
#include "cm_extension_creator.h"
#include "cm_execution_adv.h"

#define GENERATE_GLOBAL_SURFACE_INDEX

#define READ_FIELD_FROM_BUF( dst, type ) \
    dst = *((type *) &buf[bytePosition]); \
    bytePosition += sizeof(type);

#define PER_ARG_SIZE_IN_DWORD 3
#define KERNEL_INFO_SIZE_IN_DWORD 4

#define DW_ALIGNMENT( byte_address ) \
    if( byte_address % 4 ) \
    byte_address = ( byte_address / 4 + 1 ) * 4;

#define GRF_ALIGNMENT( byte_address ) \
    if( byte_address % 32 ) \
    byte_address = ( byte_address / 32 + 1 ) * 32;

// To check if surface type nType is equal to the surface type list in argument ...
#define CHECK_SURFACE_TYPE( nType, ... )  ( _CheckSurfaceType( nType, __VA_ARGS__, -1 ) )

#define IsKernelArg(arg)    ((arg).unitCount == 1)

// Warning : x must be uint32_t
#define SET_MEMORY_OBJECT_CONTROL(x, memCtl) \
           x = ((uint16_t)(memCtl.mem_ctrl<< 8 | memCtl.mem_type << 4 | memCtl.age)) << 16 | (x);

#define   ADD_INTO_VME_INDEX_ARRAY(value)     \
    vmeIndexArray[vmeIndexArrayPosition] = value ;                 \
    vmeIndexArrayPosition ++;

#define   ADD_INTO_VME_CM_INDEX_ARRAY(value)  ; \
    vmeCmIndexArray[vmeCmIndexArrayPosition] = value ;                 \
    vmeCmIndexArrayPosition ++;

typedef CM_ARG* PCM_ARG;

#define CM_KERNEL_DATA_CLEAN                   0         // kernel data clean
#define CM_KERNEL_DATA_KERNEL_ARG_DIRTY        1         // per kernel arg dirty
#define CM_KERNEL_DATA_THREAD_ARG_DIRTY        (1 << 1)  // per thread arg dirty
#define CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY      (1 << 2)  // indirect payload data dirty
#define CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY (1 << 3)  // indirect payload data size changes
#define CM_KERNEL_DATA_GLOBAL_SURFACE_DIRTY    (1 << 4)  // global surface dirty
#define CM_KERNEL_DATA_THREAD_COUNT_DIRTY      (1 << 5)  // thread count dirty, reset() be called
#define cMKERNELDATASAMPLERBTIDIRTY            (1 << 6)  // sampler bti dirty
#define CM_KERNEL_DATA_THREAD_GROUP_SPACE_DIRTY      (1 << 7)       // threadgroupspace dirty

int32_t Partition( PCM_ARG* args, int32_t p, int32_t r )
{
    uint16_t x = args[p]->unitOffsetInPayload;
    int32_t i = p - 1;
    int32_t j = r + 1;
    while( 1 )
    {
        do {
            j --;
        } while( args[j]->unitOffsetInPayload > x );

        do {
            i ++;
        } while( args[i]->unitOffsetInPayload < x );

        if( i < j )
        {
            PCM_ARG tmpP = args[i];
            args[i] = args[j];
            args[j] = tmpP;
        }
        else
        {
            return j;
        }
    }
}

// Cannot be called directly! use macro CHECK_SURFACE_TYPE!
bool _CheckSurfaceType( int nType, ... )
{
    bool match = false;
    va_list ap;
    va_start( ap, nType );
    int type = 0;

    while ( ( type = va_arg( ap, int ) ) >= 0 )
    {
        if( type == nType )
        {
            match = true;
            break;
        }
    }
    va_end(ap);

    return match;
}

void QuickSort( PCM_ARG* args, int32_t p, int32_t r )
{
    if( p < r )
    {
        int32_t q = Partition( args, p, r );
        QuickSort( args, p, q );
        QuickSort( args, q + 1, r );
    }
}

namespace CMRT_UMD
{
static bool bCmMovInstRegistered = CmExtensionCreator<CmMovInstConstructor>::RegisterClass<CmMovInstConstructor>();
//*-----------------------------------------------------------------------------
//| Purpose:   Create object for mov instructions
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
uint32_t CmMovInstConstructor::ConstructObjMovs(uint32_t dstOffset, uint32_t srcOffset, uint32_t size, CmDynamicArray &movInsts, uint32_t index, bool isBdw, bool isHwDebug)
{
    return MovInst_RT::CreateMoves(dstOffset, srcOffset, size, movInsts, index, isBdw, isHwDebug);
}

//*-----------------------------------------------------------------------------
//| Purpose:     Create CM Kernel
//| Arguments :
//|               device        [in]    Pointer to device
//|               program      [in]    Pointer to cm Program
//|               kernelName    [in]    Name of kernel
//|               kernelId      [in]    Kernel's ID
//|               kernel       [in/out]    Reference Pointer to CM Kernel
//|               options       [in]    jitter, or non-jitter
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Create(CmDeviceRT *device,
                           CmProgramRT *program,
                           const char *kernelName,
                           uint32_t kernelIndex,
                           uint32_t kernelSeqNum,
                           CmKernelRT* &kernel,
                           const char *options)
{
    int32_t result = CM_SUCCESS;
    CM_HAL_STATE * state = ((PCM_CONTEXT_DATA)device->GetAccelData())->cmHalState;

    if (state && state->advExecutor)
    {
        kernel = state->advExecutor->CreateKernelRT(device, program, kernelIndex, kernelSeqNum);
    }
    else
    {
        kernel = new (std::nothrow) CmKernelRT(device, program, kernelIndex, kernelSeqNum);
    }
    
    if( kernel )
    {
        kernel->Acquire();
        result = kernel->Initialize( kernelName, options );
        if( result != CM_SUCCESS )
        {
            CmKernelRT::Destroy( kernel, program);
            return result;
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmKernel due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    if (options)
    {
        if (strcmp(options, "PredefinedGPUCopyKernel") == 0)
        {
            kernel->m_blCreatingGPUCopyKernel = true;
        }
        else
        {
            kernel->m_blCreatingGPUCopyKernel = false;
        }
    }

#if USE_EXTENSION_CODE
    result = kernel->InitForGTPin(device, program, kernel);
#endif

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destory Kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Destroy( CmKernelRT* &kernel, CmProgramRT *&program )
{
    uint32_t refCount = kernel->SafeRelease();
    if (refCount == 0)
    {
        kernel = nullptr;
    }

    refCount = program->SafeRelease();
    if (refCount == 0)
    {
        program = nullptr;
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Acuqire Kernel: increment refcount
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Acquire( void)
{
    m_refcount ++;
    return m_refcount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SafeRelease Kernel: Delete the instance
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SafeRelease( void)
{
    --m_refcount;
    if (m_refcount == 0)
    {
        PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
        PCM_HAL_STATE state = cmData->cmHalState;
        if (state->dshEnabled)
        {
            state->pfnDSHUnregisterKernel(state, m_id);
        }
        delete this;
        return 0;
    }
    return m_refcount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Kernel constructor
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmKernelRT::CmKernelRT(CmDeviceRT *device,
                       CmProgramRT *program,
                       uint32_t kernelIndex,
                       uint32_t kernelSeqNum):
    m_device( device ),
    m_surfaceMgr( nullptr ),
    m_program( program ),
    m_options( nullptr ),
    m_binary( nullptr ),
    m_binaryOrig(nullptr),
    m_binarySize(0),
    m_binarySizeOrig(0),
    m_threadCount( 0 ),
    m_lastThreadCount( 0 ),
    m_sizeInCurbe( 0 ),
    m_sizeInPayload( 0 ),
    m_argCount( 0 ),
    m_args( nullptr ),
    m_kernelInfo(nullptr),
    m_kernelIndexInProgram( CM_INVALID_KERNEL_INDEX ),
    m_curbeEnabled( true ),
    m_nonstallingScoreboardEnabled(false),
    m_dirty( CM_KERNEL_DATA_CLEAN ),
    m_lastKernelData( nullptr ),
    m_lastKernelDataSize( 0 ),
    m_indexInTask(0),
    m_threadSpaceAssociated(false),
    m_perThreadArgExists(false),
    m_perKernelArgExists( false ),
    m_threadSpace( nullptr ),
    m_adjustScoreboardY( 0 ),
    m_lastAdjustScoreboardY( 0 ),
    m_blCreatingGPUCopyKernel( false),
    m_usKernelPayloadDataSize( 0 ),
    m_kernelPayloadData( nullptr ),
    m_usKernelPayloadSurfaceCount( 0 ),
    m_samplerBtiCount( 0 ),
    m_refcount(0),
    m_halMaxValues( nullptr ),
    m_halMaxValuesEx( nullptr ),
    m_surfaceArray(nullptr),
    m_threadGroupSpace( nullptr ),
    m_vmeSurfaceCount( 0 ),
    m_maxSurfaceIndexAllocated(0),
    m_barrierMode(CM_LOCAL_BARRIER),
    m_isClonedKernel(false),
    m_cloneKernelID(0),
    m_hasClones( false ),
    m_stateBufferBounded( CM_STATE_BUFFER_NONE ),
    m_movInstConstructor(nullptr)
{
    program->Acquire();
    m_program = program;

    device->GetSurfaceManager(m_surfaceMgr);

    m_id = kernelSeqNum; // Unique number for each kernel. This ID is used in Batch buffer.
    m_id <<= 32;
    m_kernelIndex = kernelIndex;

    for (int i = 0; i < CM_GLOBAL_SURFACE_NUMBER; i++)
    {
        m_globalSurfaces[i] = nullptr;
        m_globalCmIndex[i] = 0;
    }

    m_blhwDebugEnable = program->IsHwDebugEnabled();

    CmSafeMemSet(m_pKernelPayloadSurfaceArray, 0, sizeof(m_pKernelPayloadSurfaceArray));
    CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, sizeof(m_IndirectSurfaceInfoArray));
    CmSafeMemSet( m_samplerBtiEntry, 0, sizeof( m_samplerBtiEntry ) );

    if (m_samplerBtiCount > 0)
    {
        CmSafeMemSet(m_samplerBtiEntry, 0, sizeof(m_samplerBtiEntry));
        m_samplerBtiCount = 0;
    }

    ResetKernelSurfaces();
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Class CmKernel
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmKernelRT::~CmKernelRT( void )
{
    MosSafeDeleteArray(m_options);

    DestroyArgs();

    if(m_lastKernelData)
    {
        CmKernelData::Destroy( m_lastKernelData );
    }

    if( m_device->CheckGTPinEnabled() && !m_blCreatingGPUCopyKernel)
    {
        MosSafeDeleteArray(m_binary);
    }

    if( CM_INVALID_KERNEL_INDEX != m_kernelIndexInProgram )
    {
        m_program->ReleaseKernelInfo(m_kernelIndexInProgram);
    }

    for(int i=0; i< CM_GLOBAL_SURFACE_NUMBER; i++)
    {
        SurfaceIndex *surfIndex = m_globalSurfaces[i];
        MosSafeDelete(surfIndex);
    }

    MosSafeDeleteArray(m_kernelPayloadData);
    MosSafeDeleteArray(m_surfaceArray);
    MosSafeDelete(m_movInstConstructor);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize CM kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Initialize( const char* kernelName, const char* options )
{
    if( kernelName == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Kernel name is null.");
        return CM_NULL_POINTER;
    }

    size_t length = strnlen( kernelName, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE );
    if( length >= CM_MAX_KERNEL_NAME_SIZE_IN_BYTE  )
    {
        CM_ASSERTMESSAGE("Error: Kernel name size is too long.");
        return CM_FAILURE;
    }

    uint32_t kernelCount = 0;
    m_program->GetKernelCount( kernelCount );

    CM_KERNEL_INFO* kernelInfo = nullptr;
    uint32_t i = 0;
    for( i = 0; i < kernelCount; i ++ )
    {
        m_program->GetKernelInfo( i, kernelInfo );
        if( !kernelInfo )
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel info.");
            return CM_NULL_POINTER;
        }
        if( strcmp( kernelInfo->kernelName, kernelName ) == 0 )
        {
            break;
        }
    }

    if( i == kernelCount )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel count.");
        return CM_FAILURE;
    }

    m_device->GetHalMaxValues( m_halMaxValues, m_halMaxValuesEx);

    m_program->AcquireKernelInfo(i);
    m_kernelInfo = kernelInfo;
    m_kernelIndexInProgram = i;

    if( options )
    {
        size_t length = strnlen( options, CM_MAX_OPTION_SIZE_IN_BYTE );
        if(length >= CM_MAX_OPTION_SIZE_IN_BYTE)
        {
            CM_ASSERTMESSAGE("Error: Option string is too long.");
            return CM_INVALID_ARG_VALUE;
        }
        else
        {
            m_options = MOS_NewArray(char, (length+1));
            if( !m_options )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;

            }
            CmSafeMemCopy( m_options, options, length);
            m_options[ length ] = '\0';

            char* tmp = strstr( m_options, "nocurbe" );
            if( tmp )
            {
                m_curbeEnabled = false;
            }
        }
    }

    m_nonstallingScoreboardEnabled = true;

    void* commonISACode = nullptr;
    uint32_t commonISACodeSize = 0;
    m_program->GetCommonISACode(commonISACode, commonISACodeSize);
    if ((commonISACode == nullptr) || (commonISACodeSize <= 0))
    {
        CM_ASSERTMESSAGE("Error: Invalid VISA.");
        return CM_INVALID_COMMON_ISA;
    }

    bool useVisaApi = true;
    vISA::ISAfile *isaFile = nullptr;
    vISA::KernelBody *kernelBody = nullptr;

    auto getVersionAsInt = [](int major, int minor) { return major * 100 + minor; };
    if (getVersionAsInt(m_program->m_cisaMajorVersion, m_program->m_cisaMinorVersion) < getVersionAsInt(3, 2))
    {
        useVisaApi = false;
    }
    else
    {
        isaFile = m_program->getISAfile();
        if (!isaFile)
        {
            CM_ASSERTMESSAGE("Error: Invalid VISA.");
            return CM_INVALID_COMMON_ISA;
        }
        kernelBody = isaFile->getKernelsData().at(m_kernelIndexInProgram);
    }

    uint8_t *buf = (uint8_t*)commonISACode;
    uint32_t bytePosition = m_kernelInfo->kernelIsaOffset;

    uint32_t kernelInfoRefCount = 0;
    m_program->GetKernelInfoRefCount(m_kernelIndexInProgram, kernelInfoRefCount);
    if (kernelInfoRefCount <= 2)    //Only read for 1st time Kernel creation, later we reuse them
    {
        if (useVisaApi)
        {
            m_kernelInfo->globalStringCount = kernelBody->getStringCount();
        }
        {
            READ_FIELD_FROM_BUF(m_kernelInfo->globalStringCount, unsigned short);
        }

        m_kernelInfo->globalStrings = (const char**) malloc( m_kernelInfo->globalStringCount * sizeof(char*) );
        if(m_kernelInfo->globalStrings  == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
        CmSafeMemSet(m_kernelInfo->globalStrings, 0, m_kernelInfo->globalStringCount * sizeof(char*) );

        if (useVisaApi)
        {
            int i = 0;
            for (vISA::StringPool *globalString : kernelBody->getStringPool())
            {
                size_t stringLength = std::strlen(globalString->getString());
                char *string = (char*)malloc(stringLength + 1);
                if (string == nullptr)
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    return CM_OUT_OF_HOST_MEMORY;
                }
                CmSafeMemCopy(string, globalString->getString(), stringLength);
                string[stringLength] = '\0';
                m_kernelInfo->globalStrings[i] = string;
                i++;
            }
        }
        else
        {
            for (int i = 0; i < (int)m_kernelInfo->globalStringCount; i++)
            {
                char* string = (char*)malloc(CM_MAX_KERNEL_STRING_IN_BYTE + 1);
                if (string == nullptr)
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    return CM_OUT_OF_HOST_MEMORY;
                }
                int j = 0;
                while (buf[bytePosition] != '\0' && j < CM_MAX_KERNEL_STRING_IN_BYTE) {
                    string[j++] = buf[bytePosition++];
                }
                string[j] = '\0';
                bytePosition++;
                m_kernelInfo->globalStrings[i] = string;
            }
        }
    }

    uint32_t count = 0;
    if (useVisaApi)
    {
        count = kernelBody->getNumInputs();
    }
    else
    {
        bytePosition = m_kernelInfo->inputCountOffset;

        uint8_t countTemp = 0;
        READ_FIELD_FROM_BUF(countTemp, uint8_t);
        count = countTemp;
    }

    if( count > m_halMaxValues->maxArgsPerKernel )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg count.");
        return CM_EXCEED_KERNEL_ARG_AMOUNT;
    }

    m_args = MOS_NewArray(CM_ARG, count);
    if( (!m_args) && (count != 0) )
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        MosSafeDeleteArray(m_options);
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet(m_args, 0, sizeof(CM_ARG) * count);
    m_argCount  = count;

    uint32_t preDefinedSurfNum;
    if ( (m_program->m_cisaMajorVersion > 3) || ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion >=1)) )  //CISA 3.1 +
    {
        preDefinedSurfNum = COMMON_ISA_NUM_PREDEFINED_SURF_VER_3_1;
    }
    else if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion == 0))
    {
        preDefinedSurfNum = COMMON_ISA_NUM_PREDEFINED_SURF_VER_2_1;
    }
    else //CISA 2.0
    {
        preDefinedSurfNum = COMMON_ISA_NUM_PREDEFINED_SURF_VER_2;
    }

    uint32_t argSize = 0;

    for (uint32_t i = 0; i < m_argCount; i++)
    {
        vISA::InputInfo *inputInfo = nullptr;
        uint8_t kind = 0;

        if (useVisaApi)
        {
            inputInfo = kernelBody->getInputInfo()[i];
            kind = inputInfo->getKind();
        }
        else
        {
            READ_FIELD_FROM_BUF(kind, uint8_t);
        }

        if (kind == 0x2) // compiler value for surface
        {
            kind = ARG_KIND_SURFACE;
                // runtime value for surface. surface will be further classified to 1D/2D/3D
        }
        else if (kind == 0x3) // compiler value for vme index
        {
            kind = ARG_KIND_VME_INDEX;
        }
        else if (kind == 0x8)
        {
            kind = ARG_KIND_IMPLICT_LOCALSIZE;
            m_args[i].isSet = true;
            m_args[i].unitCount = 1;
        }
        else if (kind == 0x10) {
            kind = ARG_KIND_IMPLICT_GROUPSIZE;
            m_args[i].isSet = true;
            m_args[i].unitCount = 1;
        }
        else if (kind == 0x18) {
            kind = ARG_KIND_IMPLICIT_LOCALID;
            m_args[i].isSet = true;
            m_args[i].unitCount = 1;
            m_perKernelArgExists = true;  //only VISA3.3+, can come here, so, no matter it is there any explicit arg, implicit arg exits
        }
        else if (kind == 0x2A) {
            kind = ARG_KIND_SURFACE_2D_SCOREBOARD;
        }
        else if (kind == 0x20) {
            kind = ARG_KIND_GENERAL_DEPVEC;
        }
        else if (kind == 0x30) {
            kind = ARG_KIND_GENERAL_DEPCNT;
        }
        else if (kind == 0x80) {
            // IMP_PSEUDO_INPUT = 0x80 is pseudo input. All inputs after this
            // will be ignored by CMRT without checking and payload copied.
            // This resizes the argument count to achieve this.
            m_argCount = i;
            break;
        }

        m_args[i].unitKind = kind;
        m_args[i].unitKindOrig = kind;

        if (kind == ARG_KIND_SURFACE && m_kernelInfo->surfaceCount)
        {
            m_args[i].surfaceKind = DATA_PORT_SURF;
        }

        if (useVisaApi)
        {
            m_args[i].unitOffsetInPayload = inputInfo->getOffset();
            m_args[i].unitOffsetInPayloadOrig = inputInfo->getOffset();

            m_args[i].unitSize = inputInfo->getSize();
            m_args[i].unitSizeOrig = inputInfo->getSize();
        }
        else
        {
            uint32_t varID;
            READ_FIELD_FROM_BUF(varID, uint16_t);

            uint16_t tmpW;
            READ_FIELD_FROM_BUF(tmpW, uint16_t);
            m_args[i].unitOffsetInPayload = tmpW;
            m_args[i].unitOffsetInPayloadOrig = tmpW;

            READ_FIELD_FROM_BUF(tmpW, uint16_t);
            m_args[i].unitSize = tmpW;
            m_args[i].unitSizeOrig = tmpW;
        }

        argSize += m_args[i].unitSize;
    }
    //////////////////////////////////////////////////////////////////////////

    if (kernelInfoRefCount <= 2)    //Only read for 1st time Kernel creation, later we reuse them
    {
        uint16_t attributeCount = 0;
        if (useVisaApi)
        {
            attributeCount = kernelBody->getAttributeCount();
        }
        else
        {
            /////////////////////////////////////////////////////////////////////////
            // Get pre-defined kernel attributes, Start
            //skipping size and entry
            bytePosition += 8;

            READ_FIELD_FROM_BUF(attributeCount, uint16_t);
        }

        for (int i = 0; i < attributeCount; i++)
        {
            vISA::AttributeInfo *attribute = nullptr;
            uint32_t nameIndex = 0;
            uint8_t size = 0;

            if (useVisaApi)
            {
                attribute = kernelBody->getAttributeInfo()[i];
                nameIndex = attribute->getName();
                size = attribute->getSize();
            }
            else
            {
                READ_FIELD_FROM_BUF(nameIndex, uint16_t);
                READ_FIELD_FROM_BUF(size, uint8_t);
            }

            if( strcmp( m_kernelInfo->globalStrings[nameIndex], "AsmName" ) == 0 )
            {
                if (useVisaApi)
                {
                    CmSafeMemCopy(m_kernelInfo->kernelASMName, attribute->getValue(), size);
                }
                else
                {
                    CmSafeMemCopy(m_kernelInfo->kernelASMName, &buf[bytePosition], size);
                    bytePosition += size;
                }
            }
            else if (strcmp( m_kernelInfo->globalStrings[nameIndex], "SLMSize" ) == 0)
            {
                if (useVisaApi)
                {
                    m_kernelInfo->kernelSLMSize = attribute->getValue()[0];
                }
                else
                {
                    READ_FIELD_FROM_BUF(m_kernelInfo->kernelSLMSize, uint8_t);
                }

                /* Notes by Stony@2014-04-09
                 * <=CISA3.1: the size is number of 4KB
                 * > CISA3.1: the size is number of 1KB
                 * Here convert it to the number of 1KB if <=CISA 3.1
                 */
                if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion <= 1))
                {
                    m_kernelInfo->kernelSLMSize = m_kernelInfo->kernelSLMSize * 4;
                }

                // align to power of 2
                uint32_t v = m_kernelInfo->kernelSLMSize;
                v--;
                v |= v >> 1;
                v |= v >> 2;
                v |= v >> 4;
                v |= v >> 8;
                v |= v >> 16;
                v++;
                m_kernelInfo->kernelSLMSize = ( uint8_t )v;
            }
            else if (strcmp(m_kernelInfo->globalStrings[nameIndex], "NoBarrier") == 0)
            {
                m_kernelInfo->blNoBarrier = true;
                if (!useVisaApi)
                {
                    bytePosition += size;
                }
            }
            else
            {
                if (!useVisaApi)
                {
                    bytePosition += size;
                }
            }
        }
    }

    if(argSize > m_halMaxValues->maxArgByteSizePerKernel)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE;
    }

    buf = (uint8_t*)commonISACode;

    if(m_program->IsJitterEnabled())
    {
        //m_JitterEnable = true;
        char *programOptions;
        m_program->GetKernelOptions(programOptions);
        //if no options or same options, copy load program's binary. else re-jitter
        {
            m_binary = (char *)m_kernelInfo->jitBinaryCode;
            m_binarySize = m_kernelInfo->jitBinarySize;
            m_kernelInfo->origBinary = m_kernelInfo->jitBinaryCode;
            m_kernelInfo->origBinarySize = m_kernelInfo->jitBinarySize;
        }
    }
    else
    {
        char* binary = (char*)(buf + m_kernelInfo->genxBinaryOffset );

        //No copy, point to the binary offset in CISA code.
        m_binary = binary;
        m_binarySize = m_kernelInfo->genxBinarySize;

        m_kernelInfo->origBinary = binary;
        m_kernelInfo->origBinarySize = m_kernelInfo->genxBinarySize;
    }

    if (m_kernelInfo->blNoBarrier)
    {
        m_barrierMode = CM_NO_BARRIER;
    }

    m_movInstConstructor = CmExtensionCreator<CmMovInstConstructor>::CreateClass();
    if (m_movInstConstructor == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Failed to allocate movInstConstructor due to out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }

    CmNotifierGroup *notifiers = m_device->GetNotifiers();
    if (notifiers != nullptr)
    {
        notifiers->NotifyKernelCreated(this);
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! A CmKernel can run in multiple threads concurrently. This
//! fucntion is to set the number of threads.
//! INPUT:
//!     number of threads
//! OUTPUT:
//!     CM_SUCCESS or
//!     CM_INVALID_ARG_VALUE if the number is larger than CmKernel's capacity
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetThreadCount(uint32_t count )
{
    INSERT_API_CALL_LOG();
    // Check per kernel, per task check will be at enqueue time
    if ((int)count <= 0)
        return CM_INVALID_ARG_VALUE;

    if (m_threadSpace == nullptr)
    {
        if (m_threadCount)
        {
            // Setting threadCount twice with different values will cause reset of kernels
            if (m_threadCount != count)
            {
                Reset();
                m_threadCount = count;
                m_dirty |= CM_KERNEL_DATA_THREAD_COUNT_DIRTY;
            }
        }
        else // first time
        {
            m_threadCount = count;
        }
    }
    return CM_SUCCESS;
}

int32_t CmKernelRT::GetThreadCount(uint32_t& count )
{
    count = m_threadCount;
    return CM_SUCCESS;
}

int32_t CmKernelRT::GetKernelSurfaces(bool  *&surfArray)
{
    surfArray = m_surfaceArray;
    return CM_SUCCESS;
}

int32_t CmKernelRT::ResetKernelSurfaces()
{
    uint32_t surfacePoolSize = m_surfaceMgr->GetSurfacePoolSize();
    if (!m_surfaceArray)
    {
        m_surfaceArray = MOS_NewArray(bool, surfacePoolSize);
        if (!m_surfaceArray)
        {
            CM_ASSERTMESSAGE("Error: Failed to rest kernel surfaces due to out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    CmSafeMemSet( m_surfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get CmSurface from surface manager.
//|             Use "value + indexSurfaceArray" to locate its surfaceIndex
//| Returns:    CmSurface. Null if not found
//*-----------------------------------------------------------------------------
CmSurface* CmKernelRT::GetSurfaceFromSurfaceArray( SurfaceIndex* value, uint32_t indexSurfaceArray)
{
    int32_t hr                          = CM_SUCCESS;
    CmSurface *surface           = nullptr;
    SurfaceIndex* surfaceIndex     = nullptr;

    surfaceIndex = value + indexSurfaceArray;
    CM_CHK_NULL_GOTOFINISH_CMERROR(surfaceIndex);

    if (surfaceIndex->get_data() == CM_NULL_SURFACE
        || surfaceIndex->get_data() == 0)
    {
        surface = (CmSurface *)CM_NULL_SURFACE;
        goto finish;
    }

    m_surfaceMgr->GetSurface(surfaceIndex->get_data(), surface);

finish:
    if(hr != CM_SUCCESS)
    {
        surface = nullptr;
    }

    return surface;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set kernel arg for single vme surface or multiple vme surfaces
//|             in surface array. So far, don't support vme surface array in thread arg.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsVme(CM_KERNEL_INTERNAL_ARG_TYPE nArgType, uint32_t argIndex, const void *value, uint32_t nThreadID)
{
    uint32_t elementNum = 0;
    CM_ARG& arg        = m_args[ argIndex ];
    uint32_t totalVmeArgValueSize       = 0;
    uint32_t totalSurfacesInVme         = 0;
    uint32_t tempVmeArgValueSize        = 0;
    uint32_t vmeArgValueOffset          = 0;
    uint32_t lastVmeSurfCount           = 0;
    CmSurfaceVme* surfVme          = nullptr;
    uint8_t *vmeArgValueArray         = nullptr;
    uint16_t *vmeCmIndexArray          = nullptr;
    int32_t hr = CM_SUCCESS;

    //Get Number of elements in surface array
    if (arg.unitVmeArraySize == 0)
    {  //First Time
        elementNum = arg.unitSize / sizeof(uint32_t);
    }
    else
    {
        elementNum = arg.unitVmeArraySize;
    }

    //Get Size of vmeIndexArray and vmeCmIndexArray.
    for(uint32_t i=0; i< elementNum; i++)
    {
        if (((SurfaceIndex*)(value)+i)->get_data() == 0 || ((SurfaceIndex*)(value)+i)->get_data() == CM_NULL_SURFACE)
        {
            tempVmeArgValueSize = sizeof(CM_HAL_VME_ARG_VALUE);
            totalVmeArgValueSize += tempVmeArgValueSize;
            totalSurfacesInVme++;
        }
        else
        {
            surfVme = static_cast<CmSurfaceVme*>(GetSurfaceFromSurfaceArray((SurfaceIndex*)value, i));
            CM_CHK_NULL_GOTOFINISH_CMERROR(surfVme);
            tempVmeArgValueSize = surfVme->GetVmeCmArgSize();
            totalVmeArgValueSize += tempVmeArgValueSize;
            totalSurfacesInVme += surfVme->GetTotalSurfacesCount();
        }
    }

    // Allocate and Zero Memory for arg.pValue and arg.surfIndex
    // arg.pValue    : an array of CM_HAL_VME_ARG_VALUE structure followed by an array of reference surfaces
    // arg.surfIndex : an array listing all the Cm surface indexes, in the order of current, fw surfaces, bw surfaces

    if (arg.unitSize < totalVmeArgValueSize) // need to re-allocate larger area)
    {
        if (arg.value)
        {
            MosSafeDeleteArray(arg.value);
        }
        arg.value = MOS_NewArray(uint8_t, totalVmeArgValueSize);

        if (arg.surfIndex)
        {
            MosSafeDeleteArray(arg.surfIndex);
        }
        arg.surfIndex = MOS_NewArray(uint16_t, totalSurfacesInVme);
    }

    CM_CHK_NULL_GOTOFINISH_CMERROR(arg.value);
    CmSafeMemSet(arg.value, 0, totalVmeArgValueSize);
    CM_CHK_NULL_GOTOFINISH_CMERROR(arg.surfIndex);
    CmSafeMemSet(arg.surfIndex, 0, totalSurfacesInVme * sizeof(uint16_t));

    //Set each Vme Surface
    for (uint32_t i = 0; i< elementNum; i++)
    {
        if (((SurfaceIndex*)(value)+i)->get_data() == 0 || ((SurfaceIndex*)(value)+i)->get_data() == CM_NULL_SURFACE)
        {
            PCM_HAL_VME_ARG_VALUE vmeArg = (PCM_HAL_VME_ARG_VALUE)(arg.value + vmeArgValueOffset);
            vmeArg->fwRefNum = 0;
            vmeArg->bwRefNum = 0;
            vmeArg->curSurface = CM_NULL_SURFACE;
            tempVmeArgValueSize = sizeof(CM_HAL_VME_ARG_VALUE);
            vmeArgValueOffset += tempVmeArgValueSize;
            arg.surfIndex[lastVmeSurfCount] = CM_NULL_SURFACE;
            lastVmeSurfCount++;
        }
        else
        {
            surfVme = static_cast<CmSurfaceVme*>(GetSurfaceFromSurfaceArray((SurfaceIndex*)value, i));
            CM_CHK_NULL_GOTOFINISH_CMERROR(surfVme);
            SetArgsSingleVme(surfVme, arg.value + vmeArgValueOffset, arg.surfIndex + lastVmeSurfCount);
            tempVmeArgValueSize = surfVme->GetVmeCmArgSize();
            vmeArgValueOffset += tempVmeArgValueSize;
            lastVmeSurfCount += surfVme->GetTotalSurfacesCount();
        }
    }

    if ( nArgType == CM_KERNEL_INTERNEL_ARG_PERKERNEL ) // per kernel arg
    {
        // First time set
        if( !arg.value )
        {   // Increment size kernel arguments will take up in CURBE
            m_sizeInCurbe += CM_ARGUMENT_SURFACE_SIZE * elementNum;
        }

        arg.unitCount = 1;
        arg.isDirty  = true;
        arg.isSet    = true;
        arg.unitKind  = ARG_KIND_SURFACE_VME;
        arg.unitSize = (uint16_t)totalVmeArgValueSize; // the unitSize can't represent surfaces count here
        arg.unitVmeArraySize = elementNum;

        m_dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
        m_perKernelArgExists = true;
    }
    else
    {
        // Thread arg doesn't support VME surfaces as it is rarely used and it is complex to implement,
        // since each thread may has different surface number in its vme surface argment.
        hr = CM_THREAD_ARG_NOT_ALLOWED;
    }

finish:
    if(hr != CM_SUCCESS)
    {
        MosSafeDeleteArray(arg.value);
        MosSafeDeleteArray(arg.surfIndex);
    }
    return hr;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Fill arg for a single vme surface.
//|             vmeIndexArray points to arg.pValue
//|             vmeCmIndexArray points to arg.surfIndex
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsSingleVme(CmSurfaceVme* vmeSurface, uint8_t *vmeArgValueArray, uint16_t *cmSufacesArray)
{

    int32_t hr = CM_SUCCESS;
    CM_SURFACE_MEM_OBJ_CTRL memCtl;
    uint32_t vmeBackwardSurfaceCount        = 0;
    uint32_t vmeForwardSurfaceCount         = 0;
    uint32_t vmeCurrentSurfaceIndex         = 0;
    uint16_t vmeCurrentCmIndex              = 0;
    int32_t vmeIndexArrayPosition          = 0; // Offset for vmeIndexArray
    int32_t vmeCmIndexArrayPosition        = 0; // Offset for vmeCmIndexArray
    uint32_t tempOutput                     = 0;
    uint32_t cmSurfArrayIdx                 = 0;
    uint32_t surfStateWidth                 = 0;
    uint32_t surfStateHeight                = 0;

    uint32_t *fArray       = nullptr;
    uint32_t *bArray       = nullptr;
    uint32_t *fCmIndex     = nullptr;
    uint32_t *bCmIndex     = nullptr;

    uint32_t *fwSurfInArg = nullptr;
    uint32_t *bwSurfInArg = nullptr;

    CmSurface *surface = nullptr;
    PCM_HAL_VME_ARG_VALUE vmeArg = (PCM_HAL_VME_ARG_VALUE)vmeArgValueArray;

    CM_CHK_NULL_GOTOFINISH_CMERROR(vmeSurface);
    CM_CHK_NULL_GOTOFINISH_CMERROR(vmeArg);
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmSufacesArray);

    if(vmeSurface == (CmSurfaceVme *)CM_NULL_SURFACE)
    {
        vmeArg->fwRefNum = 0;
        vmeArg->bwRefNum = 0;
        vmeArg->curSurface = CM_NULL_SURFACE;
        cmSufacesArray[cmSurfArrayIdx] =  CM_NULL_SURFACE;
        return hr;
    }

    // Get Vme Backward Forward Surface Count
    vmeSurface->GetIndexBackwardCount(vmeBackwardSurfaceCount);
    vmeSurface->GetIndexForwardCount(vmeForwardSurfaceCount);

    vmeArg->fwRefNum = vmeForwardSurfaceCount;
    vmeArg->bwRefNum = vmeBackwardSurfaceCount; // these two numbers must be set before any other operations

    vmeSurface->GetSurfaceStateResolution(vmeArg->surfStateParam.surfaceStateWidth, vmeArg->surfStateParam.surfaceStateHeight);

    vmeSurface->GetIndexForwardArray(fArray);
    vmeSurface->GetIndexBackwardArray(bArray);
    vmeSurface->GetIndexCurrent(vmeCurrentSurfaceIndex);

    vmeSurface->GetCmIndexCurrent(vmeCurrentCmIndex);
    vmeSurface->GetCmIndexForwardArray(fCmIndex);
    vmeSurface->GetCmIndexBackwardArray(bCmIndex);

    cmSufacesArray[cmSurfArrayIdx++] = vmeCurrentCmIndex;

    // Set Current Vme Surface
    m_surfaceMgr->GetSurface(vmeCurrentCmIndex, surface);
    CM_CHK_NULL_GOTOFINISH_CMERROR(surface);

    vmeArg->curSurface = vmeCurrentSurfaceIndex;

    //Set Forward Vme Surfaces
    fwSurfInArg = findFwRefInVmeArg(vmeArg);
    for (uint32_t i = 0; i < vmeForwardSurfaceCount; i++)
    {
        GetVmeSurfaceIndex( fArray, fCmIndex, i, &tempOutput);
        fwSurfInArg[i] = tempOutput;
        cmSufacesArray[cmSurfArrayIdx++] = (uint16_t)fCmIndex[i];
    }

    //Set Backward Vme Surfaces
    bwSurfInArg = findBwRefInVmeArg(vmeArg);
    for (uint32_t i = 0; i < vmeBackwardSurfaceCount; i++)
    {
        GetVmeSurfaceIndex( bArray, bCmIndex, i, &tempOutput);
        bwSurfInArg[i] = tempOutput;
        cmSufacesArray[cmSurfArrayIdx++] = (uint16_t)bCmIndex[i];
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Vme Surface Index with memory object setting .
//|             Output value will be filled into arg.pValue
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetVmeSurfaceIndex(
    uint32_t *vmeIndexArray,
    uint32_t *vmeCmIndexArray,
    uint32_t index,
    uint32_t *outputValue)
{
    int32_t hr = CM_SUCCESS;
    uint32_t value = vmeIndexArray[index];

    if (vmeIndexArray[index] == CM_INVALID_VME_SURFACE)
    {
        value = CM_NULL_SURFACE;
    }

    *outputValue = value;

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set arguments for function SetKernelArg().
//|             Kernel argument is surface array.
//! INPUT:
//!             1) Current index in surface array
//!             2) Index of kernel argument
//!             3) Surface count in surface array
//!             4) Pointer to current surface in surface array.
//!             5) Current surface  index
//!             6) Pointer to argument value
//!             7) value of surface handle combined with memory object control
//!             8) Original surface index for each surface in array
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsInternalSurfArray(
    int32_t offset,uint32_t kernelArgIndex,
    int32_t surfCount, CmSurface* currentSurface,
    uint32_t currentSurfIndex, SurfaceIndex* value,
    uint32_t surfValue[], uint16_t origSurfIndex[])
{
    CM_SURFACE_MEM_OBJ_CTRL memCtl;
    uint32_t                surfRegTableIndex = 0;
    uint32_t                handle = 0;
    uint32_t                samplerIndex;
    uint16_t                samplerCmIndex;
    uint32_t                surfaceArraySize = 0;

    m_surfaceMgr->GetSurfaceArraySize(surfaceArraySize);
    MosSafeDeleteArray(m_args[kernelArgIndex].surfArrayArg); // delete it if it was allcated
    m_args[kernelArgIndex].surfArrayArg = MOS_NewArray(SURFACE_ARRAY_ARG, surfCount);
    if (!m_args[kernelArgIndex].surfArrayArg)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet((void *)m_args[kernelArgIndex].surfArrayArg, 0,  sizeof(SURFACE_ARRAY_ARG) * surfCount);
    while (offset < surfCount)
    {
        switch (currentSurface->Type())
        {
          case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
          {
             CmSurface2DRT* surf2D = static_cast<CmSurface2DRT*>(currentSurface);

             uint32_t numAliases = 0;
             surf2D->GetNumAliases(numAliases);
             if (numAliases)
             {
                 m_args[kernelArgIndex].aliasCreated = true;
             }
             else
             {
                 m_args[kernelArgIndex].aliasCreated = false;
             }

             //set memory object control
             surf2D->GetIndexFor2D(surfRegTableIndex);

             surfValue[offset] = surfRegTableIndex;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_2D;
             m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_2D;

             break;
         }
         case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
         {
             CmBuffer_RT* surf1D = static_cast<CmBuffer_RT*>(currentSurface);

             uint32_t numAliases = 0;
             surf1D->GetNumAliases(numAliases);
             if (numAliases)
             {
                 m_args[kernelArgIndex].aliasCreated = true;
             }
             else
             {
                 m_args[kernelArgIndex].aliasCreated = false;
             }

             //set memory object control
             surf1D->GetHandle(handle);

             surfValue[offset] = handle;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_1D;
             m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_1D;
             break;
         }
         case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
         {
             CmSurface2DUPRT* surf2DUP = static_cast<CmSurface2DUPRT*>(currentSurface);

             //set memory object
             surf2DUP->GetHandle(handle);

             surfValue[offset] = handle;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_2D_UP;
             m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_2D_UP;
             break;
         }
         case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
         {
             CmSurface3DRT* surf3D = static_cast<CmSurface3DRT*>(currentSurface);

             surf3D->GetHandle(handle);

             surfValue[offset] = handle;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_3D;
             m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_3D;

             break;
         }

         case CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER:
         {
             CmStateBuffer* stateBuffer = static_cast< CmStateBuffer* >( currentSurface );
             stateBuffer->GetHandle( handle );

             surfValue[ offset ] = handle;
             origSurfIndex[ offset ] = ( uint16_t )currentSurfIndex;

             m_args[ kernelArgIndex ].surfArrayArg[ offset ].argKindForArray = ARG_KIND_STATE_BUFFER;
             m_args[ kernelArgIndex ].unitKind = ARG_KIND_STATE_BUFFER;

             break;
         }

         //sampler surface
         case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
         {
             CmSurfaceSampler* surfSampler = static_cast <CmSurfaceSampler *> (currentSurface);
             surfSampler->GetHandle(samplerIndex);
             surfSampler->GetCmIndexCurrent(samplerCmIndex);

             m_surfaceMgr->GetSurface(samplerCmIndex, currentSurface);
             if (!currentSurface)
             {
                 CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
                 return CM_NULL_POINTER;
             }

             surfValue[offset] = samplerIndex;
             origSurfIndex[offset] = (uint16_t)samplerCmIndex;

             SAMPLER_SURFACE_TYPE type;
             surfSampler->GetSurfaceType(type);
             if (type == SAMPLER_SURFACE_TYPE_2D)
             {
                 m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_SAMPLER;
                 m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_SAMPLER;
             }
             else if (type == SAMPLER_SURFACE_TYPE_2DUP)
             {
                 m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE2DUP_SAMPLER;
                 m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE2DUP_SAMPLER;
             }
             else if(type == SAMPLER_SURFACE_TYPE_3D)
             {
                 m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_3D;
                 m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_3D;
             }
             else
             {
                 CM_ASSERTMESSAGE("Error: Assign a Sampler surface to the arg which is previously 2D/3D surface.");
                 return CM_FAILURE;
             }
             break;
         }
         //sampler8x8surface
         case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
         {
             CmSurfaceSampler8x8* surfSampler8x8 = static_cast <CmSurfaceSampler8x8 *> (currentSurface);
             surfSampler8x8->GetIndexCurrent(samplerIndex);
             surfSampler8x8->GetCmIndex(samplerCmIndex);

             m_surfaceMgr->GetSurface(samplerCmIndex, currentSurface);
             if (!currentSurface)
             {
                 CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
                 return CM_FAILURE;
             }

             surfValue[offset] = samplerIndex;
             origSurfIndex[offset] = (uint16_t)samplerCmIndex;

             CM_SAMPLER8x8_SURFACE type;
             type = surfSampler8x8->GetSampler8x8SurfaceType();
             if (type == CM_VA_SURFACE)
             {
                 m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_VA;
                 m_args[kernelArgIndex].surfArrayArg[offset].addressModeForArray = surfSampler8x8->GetAddressControlMode();
                 m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_SAMPLER8X8_VA;
             }
             else if(type == CM_AVS_SURFACE)
             {
                 m_args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
                 m_args[kernelArgIndex].surfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
             }
             else
             {
                 CM_ASSERTMESSAGE("Error: Assign a Sampler8x8 surface to the arg which is previously 2D surface.");
                 return CM_FAILURE;
             }
             break;
         }
         default:
         {
             CM_ASSERTMESSAGE("Error: No matched surface for surface array");
             return CM_INVALID_ARG_VALUE;
         }
       }
       offset++;
       if (offset < surfCount)
       {
           currentSurfIndex = value[offset].get_data();

           while ((!currentSurfIndex && (offset < surfCount))||(currentSurfIndex == CM_NULL_SURFACE))
           {
               surfValue[offset] = CM_NULL_SURFACE;
               origSurfIndex[offset] = 0;
               offset++;
               if (offset >= surfCount)
                   break;
               currentSurfIndex = value[offset].get_data();
           }

           if(surfaceArraySize == 0)
           {
               CM_ASSERTMESSAGE("Error: No surface in surface array");
               return CM_NO_AVAILABLE_SURFACE;
           }
           if (currentSurfIndex > surfaceArraySize)
           {
               currentSurfIndex = currentSurfIndex % surfaceArraySize;
           }
       }
       if (offset < surfCount)
       {
           m_surfaceMgr->GetSurface(currentSurfIndex, currentSurface);
           if (nullptr == currentSurface)
           {
               CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
               return CM_FAILURE;
           }
       }
    }
    return CM_SUCCESS;
}
//*-----------------------------------------------------------------------------
// Set arguments for function SetKernelArg() and SetThreadArg()
// Set parameter nArgType to CM_KERNEL_INTERNEL_ARG_KERNEL to set a kernel
// argument; set parameter nArgType to CM_KERNEL_INTERNEL_ARG_THREAD to set
// a thread argument
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsInternal( CM_KERNEL_INTERNAL_ARG_TYPE nArgType, uint32_t index, size_t size, const void *value, uint32_t nThreadID )
{
    uint32_t surfRegTableIndex = 0; // for 2D surf
    uint32_t handle = 0; // for 1D surf

    uint32_t samplerIndex;
    uint16_t samplerCmIndex;
    uint32_t samplerIdx = 0;
    uint32_t vmeIdx = 0;
    uint16_t *surfIndexValue =  nullptr;
    uint32_t surfaces[CM_MAX_ARGS_PER_KERNEL];
    uint16_t surfIndexArray[CM_MAX_ARGS_PER_KERNEL];
    std::vector< int > sampler_index_array;

    //Clear "set" flag in case user call API to set the same one argument multiple times.
    m_args[index].isSet = false;
    if( m_args[ index ].unitKind == ARG_KIND_GENERAL || (m_args[index].unitKind == ARG_KIND_GENERAL_DEPVEC) || (m_args[index].unitKind == ARG_KIND_GENERAL_DEPCNT))
    {
        if( size != m_args[ index ].unitSize )
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_INVALID_ARG_SIZE;
        }
    }
    //For surface type
    else if (CHECK_SURFACE_TYPE(m_args[index].unitKind,
        ARG_KIND_SURFACE,
        ARG_KIND_SURFACE_1D,
        ARG_KIND_SURFACE_2D,
        ARG_KIND_SURFACE_2D_UP,
        ARG_KIND_SURFACE_3D,
        ARG_KIND_SURFACE_SAMPLER,
        ARG_KIND_SURFACE2DUP_SAMPLER,
        ARG_KIND_SURFACE_VME,
        ARG_KIND_SURFACE_SAMPLER8X8_AVS,
        ARG_KIND_SURFACE_SAMPLER8X8_VA,
        ARG_KIND_SURFACE_2D_SCOREBOARD,
        ARG_KIND_STATE_BUFFER
        ))
    {

        // this code is to convert SurfaceIndex object to index of type uint32_t,
        // which is expected by commonISA/genBinary
        // index is the index of the surface in surface registration table of CM device
        // in driver

        int signatureSize = m_args[index].unitSize;
        int numSurfaces = signatureSize / sizeof(int);
        SurfaceIndex* surfIndex = (SurfaceIndex*)value;
        if (surfIndex == (SurfaceIndex*)CM_NULL_SURFACE)
        {
            m_args[index].isSet = true;
            m_args[index].unitCount = 1; // per kernel arg
            m_dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
            m_perKernelArgExists = true;
            m_args[index].isDirty = true;
            m_args[index].isNull = true;
            return CM_SUCCESS;
        }
        else
        {
            // In case that CM_NULL_SURFACE was set at last time and will 
            // set a read surface index this time. So need set isDirty as
            // well to indicate update kernel data.
            if (m_args[index].isNull == true)
            {
                m_args[index].isDirty = true;
                m_args[index].isNull = false;
            }
        }
        
        m_args[index].isNull = false;
        CM_SURFACE_MEM_OBJ_CTRL memCtl;

        if (m_args[index].unitKind != ARG_KIND_SURFACE_VME)
        {
            if (size != sizeof(SurfaceIndex)* numSurfaces)
            {
                CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
                return CM_INVALID_ARG_SIZE;
            }
        }

        uint32_t surfIndexData = surfIndex->get_data();
        int i = 0;
        uint32_t surfaceArraySize = 0;
        m_surfaceMgr->GetSurfaceArraySize(surfaceArraySize);

        if (surfIndexData > surfaceArraySize)
        {
            if (m_args[index].aliasIndex != surfIndexData)
            {
                m_args[index].isDirty = true;
                m_dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
                m_args[index].aliasIndex = surfIndexData;
            }

            surfIndexData = surfIndexData % surfaceArraySize;
        }
        else
        {
            m_args[index].aliasIndex = 0;
        }

        while (!surfIndexData && (i < numSurfaces))
        {
            surfaces[i] = CM_NULL_SURFACE;
            surfIndexArray[i] = 0;
            i++;
            if (i >= numSurfaces)
                break;
            surfIndexData = surfIndex[i].get_data();
        }

        if (i >= numSurfaces)
        {
            m_args[index].unitKind = ARG_KIND_SURFACE_2D;
            value = surfaces;
            size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
            m_args[index].unitSize = (uint16_t)size;
            goto finish;
        }
        CmSurface* surface = nullptr;
        m_surfaceMgr->GetSurface(surfIndexData, surface);
        if (nullptr == surface)
        {
            CM_ASSERTMESSAGE("Error: Invalid surface.");
            return CM_FAILURE;
        }

        if (SurfTypeToArgKind(surface->Type()) != m_args[index].unitKind)
        {   // if surface type changes i.e 2D <-> 2DUP  Need to set bIsDrity as true
            m_args[index].isDirty = true;
            m_dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
        }

        uint32_t cisaMajorVersion, cisaMinorVersion;
        m_program->GetCISAVersion(cisaMajorVersion, cisaMinorVersion);

        //This path is for surface array, including 1D, 2D, 3D,samplersurface, samplersurface8x8
        if ((numSurfaces > 1) && (surface->Type() != CM_ENUM_CLASS_TYPE_CMSURFACEVME))
        {
            int32_t hr = SetArgsInternalSurfArray(i,index, numSurfaces, surface, surfIndexData, surfIndex,surfaces, surfIndexArray);
            if (hr != CM_SUCCESS)
            {
                CM_ASSERTMESSAGE("Error: SetArgsInternal for surface array failed!\n");
                return CM_INVALID_ARG_VALUE;
            }
            value = surfaces;
            surfIndexValue = surfIndexArray;
            size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
            m_args[index].unitSize = (uint16_t)size;
        }
        else
        {   //This is for single surface and surface array for VME surface
            switch (surface->Type())
            {
                 case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
                 {
                     CmSurface2DRT* surf2D = static_cast<CmSurface2DRT*>(surface);

                     uint32_t numAliases = 0;
                     surf2D->GetNumAliases(numAliases);
                     if (numAliases)
                     {
                         m_args[index].aliasCreated = true;
                     }
                     else
                     {
                         m_args[index].aliasCreated = false;
                     }

                     //set memory object control
                     surf2D->GetIndexFor2D(surfRegTableIndex);

                     surfaces[i] = surfRegTableIndex;
                     surfIndexArray[i] = (uint16_t)surfIndexData;

                     value = surfaces;
                     surfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_args[index].unitSize = (uint16_t)size;

                     if ((m_args[index].unitKind == ARG_KIND_SURFACE) || (m_args[index].unitKind == ARG_KIND_SURFACE_2D_UP)) // first time or last time is set to 2DUP
                     {
                         m_args[index].unitKind = ARG_KIND_SURFACE_2D;
                         if (m_args[index].surfaceKind == SAMPLER_SURF)
                             m_args[index].unitKind = ARG_KIND_SURFACE_SAMPLER;
                     }
                     else if (m_args[index].unitKind != ARG_KIND_SURFACE_2D &&
                         m_args[index].unitKind != ARG_KIND_SURFACE_SAMPLER &&
                         m_args[index].unitKind != ARG_KIND_SURFACE2DUP_SAMPLER &&
                         m_args[index].unitKind != ARG_KIND_SURFACE_2D_SCOREBOARD)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 2D surface to the arg which is previously assigned 1D surface, 3D surface, or VME surface.");
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
                 {
                     CmBuffer_RT* surf1D = static_cast<CmBuffer_RT*>(surface);

                     uint32_t numAliases = 0;
                     surf1D->GetNumAliases(numAliases);
                     if (numAliases)
                     {
                         m_args[index].aliasCreated = true;
                     }
                     else
                     {
                         m_args[index].aliasCreated = false;
                     }

                     //set memory object control
                     surf1D->GetHandle(handle);

                     surfaces[i] = handle;
                     surfIndexArray[i] = (uint16_t)surfIndexData;

                     value = surfaces;
                     surfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_args[index].unitSize = (uint16_t)size;

                     if (m_args[index].unitKind == ARG_KIND_SURFACE)
                     {
                         m_args[index].unitKind = ARG_KIND_SURFACE_1D;
                     }
                     else if (m_args[index].unitKind != ARG_KIND_SURFACE_1D)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 1D surface to the arg which is previously assigned 2D surface, 3D surface, or VME surface.");
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
                 {
                     CmSurface2DUPRT* surf2DUP = static_cast<CmSurface2DUPRT*>(surface);

                     //set memory object
                     surf2DUP->GetHandle(handle);

                     surfaces[i] = handle;
                     surfIndexArray[i] = (uint16_t)surfIndexData;

                     value = surfaces;
                     surfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_args[index].unitSize = (uint16_t)size;

                     if ((m_args[index].unitKind == ARG_KIND_SURFACE) || (m_args[index].unitKind == ARG_KIND_SURFACE_2D)) // first time or last time is set to 2D
                     {
                         m_args[index].unitKind = ARG_KIND_SURFACE_2D_UP;
                     }
                     else if (m_args[index].unitKind != ARG_KIND_SURFACE_2D_UP)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 2D surface UP to the arg which is previously assigned other surfaces.");
                         return CM_INVALID_ARG_VALUE;
                     }

                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
                 {
                     CmSurface3DRT* surf3D = static_cast<CmSurface3DRT*>(surface);

                     surf3D->GetHandle(handle);

                     surfaces[i] = handle;
                     surfIndexArray[i] = (uint16_t)surfIndexData;

                     value = surfaces;
                     surfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_args[index].unitSize = (uint16_t)size;

                     if (m_args[index].unitKind == ARG_KIND_SURFACE) // first time
                     {
                         m_args[index].unitKind = ARG_KIND_SURFACE_3D;
                     }
                     else if (m_args[index].unitKind != ARG_KIND_SURFACE_3D)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 3D surface to the arg which is previously assigned 1D surface, 2D surface or VME surface");
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }

                 case CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER:
                 {
                     CmStateBuffer* stateBuffer = static_cast< CmStateBuffer* >( surface );
                     stateBuffer->GetHandle( handle );

                     surfaces[ i ] = handle;
                     surfIndexArray[ i ] = ( uint16_t )surfIndexData;

                     value = surfaces;
                     surfIndexValue = surfIndexArray;

                     size = ( size / sizeof( SurfaceIndex ) ) * sizeof( uint32_t );
                     m_args[ index ].unitSize = ( uint16_t )size;

                     if ( m_args[ index ].unitKind == ARG_KIND_SURFACE ) // first time
                     {
                         m_args[ index ].unitKind = ARG_KIND_STATE_BUFFER;
                     }
                     else if ( m_args[ index ].unitKind != ARG_KIND_STATE_BUFFER )
                     {
                         CM_ASSERTMESSAGE( "Error: Assign a state buffer to the arg which is previously assigned 1D surface, 2D surface, 3D surface or VME surface" );
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }

                 case CM_ENUM_CLASS_TYPE_CMSURFACEVME:
                 {
                     return SetArgsVme(nArgType, index, value, nThreadID);
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
                 {
                     CmSurfaceSampler8x8* surfSampler8x8 = static_cast <CmSurfaceSampler8x8 *> (surface);
                     surfSampler8x8->GetIndexCurrent(samplerIndex);
                     surfSampler8x8->GetCmIndex(samplerCmIndex);
                     if (samplerCmIndex > surfaceArraySize)
                     {
                         m_args[index].aliasIndex = samplerCmIndex;
                         m_args[index].aliasCreated = true;
                         samplerCmIndex %= surfaceArraySize;
                     }

                     m_surfaceMgr->GetSurface(samplerCmIndex, surface);
                     if (!surface)
                     {
                         CM_ASSERTMESSAGE("Error: Invalid sampler8x8 surface.");
                         return CM_FAILURE;
                     }

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_args[index].unitSize = (uint16_t)size;

                     value = &samplerIndex;
                     surfIndexValue = &samplerCmIndex;

                     if (m_args[index].unitKind == ARG_KIND_SURFACE)
                     {
                         if (surfSampler8x8->GetSampler8x8SurfaceType() == CM_VA_SURFACE)
                         {
                             m_args[index].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_VA;
                             m_args[index].nCustomValue = surfSampler8x8->GetAddressControlMode();
                         }
                         else
                         {
                             m_args[index].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
                         }
                     }
                     else if (m_args[index].unitKind != ARG_KIND_SURFACE_SAMPLER8X8_AVS &&
                         m_args[index].unitKind != ARG_KIND_SURFACE_SAMPLER8X8_VA)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a Sampler8x8 surface to the arg which is previously 2D surface.");
                         return CM_FAILURE;
                     }
                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
                 {
                     CmSurfaceSampler* surfSampler = static_cast <CmSurfaceSampler *> (surface);
                     surfSampler->GetHandle(samplerIndex);
                     surfSampler->GetCmIndexCurrent(samplerCmIndex);

                     m_surfaceMgr->GetSurface(samplerCmIndex, surface);
                     if (!surface)
                     {
                         CM_ASSERTMESSAGE("Error: Invalid sampler surface.");
                         return CM_FAILURE;
                     }

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_args[index].unitSize = (uint16_t)size;

                     value = &samplerIndex;
                     surfIndexValue = &samplerCmIndex;

                     if (m_args[index].unitKind == ARG_KIND_SURFACE)
                     {   // first time
                         SAMPLER_SURFACE_TYPE type;
                         surfSampler->GetSurfaceType(type);
                         if (type == SAMPLER_SURFACE_TYPE_2D)
                         {
                             m_args[index].unitKind = ARG_KIND_SURFACE_SAMPLER;
                         }
                         else if (type == SAMPLER_SURFACE_TYPE_2DUP)
                         {
                             m_args[index].unitKind = ARG_KIND_SURFACE2DUP_SAMPLER;
                         }
                         else
                         {
                             m_args[index].unitKind = ARG_KIND_SURFACE_3D;
                         }

                     }
                     else if ((m_args[index].unitKind != ARG_KIND_SURFACE_SAMPLER) &&
                         m_args[index].unitKind != ARG_KIND_SURFACE2DUP_SAMPLER &&
                         (m_args[index].unitKind != ARG_KIND_SURFACE_3D))
                     {
                         CM_ASSERTMESSAGE("Error: Assign a Sampler surface to the arg which is previously 2D/3D surface.");
                         return CM_FAILURE;
                     }
                     break;
                 }
                 default:
                 {
                     CM_ASSERTMESSAGE("Error: Invalid surface type.");
                     return CM_INVALID_ARG_VALUE;
                 }
            }
        }
    }
    else if (m_args[index].unitKind == ARG_KIND_SAMPLER)
    {
        unsigned int numSamplers = m_args[index].unitSize / sizeof(int);

        if (numSamplers > 1)
        {
            size = numSamplers * sizeof(unsigned int);

            for (unsigned int i = 0; i < numSamplers; i++)
            {
                SamplerIndex* samplerIndex = (SamplerIndex*)value + i;
                samplerIdx = samplerIndex->get_data();
                sampler_index_array.push_back(samplerIdx);
            }
        }
        else
        {
            SamplerIndex* samplerIndex = (SamplerIndex*)value;
            samplerIdx = ((SamplerIndex*)value)->get_data();
            size = sizeof(unsigned int);
            m_args[index].unitSize = (uint16_t)size;
            value = &samplerIdx;
        }
    }

finish:
    if ( nArgType == CM_KERNEL_INTERNEL_ARG_PERKERNEL ) // per kernel arg
    {
        CM_ARG& arg = m_args[ index ];

        // Assume from now on, size is valid, i.e. confirmed with function signature
        if( !arg.value )
        {
            //Increment size kernel arguments will take up in CURBE
            uint32_t tempUnitSize = m_args[ index ].unitSize;
            if( (m_args[index].unitKind == ARG_KIND_SURFACE_VME ) ||
                (m_args[index].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                (m_args[index].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ))
            {
                tempUnitSize = CM_ARGUMENT_SURFACE_SIZE;
            }

            // first setKernelArg or first setKernelArg after each enqueue
            arg.value = MOS_NewArray(uint8_t,size);
            if( !arg.value )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;
            }

            arg.unitCount = 1;

            CmSafeMemCopy((void *)arg.value, value, size);

            if((( m_args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
                 ( m_args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && surfIndexValue )
            {
                arg.surfIndex = MOS_NewArray(uint16_t, (size / sizeof(int32_t)));
                if (!arg.surfIndex)
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    MosSafeDeleteArray(arg.value);
                    return CM_OUT_OF_HOST_MEMORY;
                }
                CmSafeMemSet((void *)arg.surfIndex, 0, size/sizeof(int32_t) * sizeof(uint16_t));
                if( surfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmSafeMemCopy((void *)arg.surfIndex, surfIndexValue, size / sizeof(int32_t) * sizeof(uint16_t));
            }

            if (m_args[index].unitKind == ARG_KIND_SAMPLER)
            {
                for (unsigned int samplerIndex = 0; samplerIndex < sampler_index_array.size(); samplerIndex++)
                {
                    *( (int *)arg.value + samplerIndex) = sampler_index_array[samplerIndex];
                }
            }

            m_dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
            arg.isDirty = true;
        }
        else
        {
            if( arg.unitCount != 1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid arg count.");
                return CM_FAILURE;
            }
            if( memcmp( (void *)arg.value, value, size ) != 0 )
            {
                CmSafeMemCopy((void *)arg.value, value, size);
                m_dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
                arg.isDirty = true;
            }
            if((( m_args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
             ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
             ( m_args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && surfIndexValue )
            {
                CmSafeMemSet((void *)arg.surfIndex, 0, size/sizeof(int32_t) * sizeof(uint16_t));
                if( surfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmSafeMemCopy((void *)arg.surfIndex, surfIndexValue, size/sizeof(int32_t) * sizeof(uint16_t));
            }

            if (m_args[index].unitKind == ARG_KIND_SAMPLER)
            {
                for (unsigned int samplerIndex = 0; samplerIndex < sampler_index_array.size(); samplerIndex++)
                {
                    *((int *)arg.value + samplerIndex) = sampler_index_array[samplerIndex];
                }
            }
        }

        m_perKernelArgExists = true;
    }
    else //per thread arg
    {
        CM_ARG& arg = m_args[ index ];

        // Assume from now on, size is valid, i.e. confirmed with function signature
        if( !arg.value )
        {
            //Increment size per-thread arguments will take up in payload of media object or media object walker commands
            m_sizeInPayload += arg.unitSize;
            DW_ALIGNMENT(m_sizeInPayload);

            // first setThreadArg or first setThreadArg after each enqueue
            arg.value = MOS_NewArray(uint8_t, (size * m_threadCount));
            if( !arg.value )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;

            }
            arg.unitCount = m_threadCount;

            uint32_t offset = size * nThreadID;
            uint8_t *threadValue = ( uint8_t *)arg.value;
            threadValue += offset;

            CmSafeMemCopy(threadValue, value, size);
            if((( m_args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
                 ( m_args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && surfIndexValue )
            {
                arg.surfIndex = MOS_NewArray(uint16_t, (size / sizeof(uint32_t) * m_threadCount));
                if( !arg.surfIndex )
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    MosSafeDeleteArray(arg.value);
                    return CM_OUT_OF_HOST_MEMORY;
                }
                CmSafeMemSet((void *)arg.surfIndex, 0, size/sizeof(uint32_t) * sizeof(uint16_t) * m_threadCount);
                if( surfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmSafeMemCopy((void *)(arg.surfIndex + size/sizeof(uint32_t)  * nThreadID), surfIndexValue, size/sizeof(uint32_t) * sizeof(uint16_t));
            }
            m_perThreadArgExists = true;
        }
        else
        {
            if( arg.unitCount != m_threadCount )
            {
                CM_ASSERTMESSAGE("Error: arg count is not matched with thread count.");
                return CM_FAILURE;

            }
            uint32_t offset = size * nThreadID;
            uint8_t *threadValue = ( uint8_t *)arg.value;
            threadValue += offset;

            if( memcmp( threadValue, value, size ) != 0 )
            {
                CmSafeMemCopy(threadValue, value, size);
                m_dirty |= CM_KERNEL_DATA_THREAD_ARG_DIRTY;
                arg.isDirty = true;
            }
            if((( m_args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
                 ( m_args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
                 ( m_args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && surfIndexValue )
            {
                if( surfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmSafeMemCopy((void *)(arg.surfIndex + size/sizeof(uint32_t)  * nThreadID), surfIndexValue, size/sizeof(uint32_t) * sizeof(uint16_t));
            }
        }
    }

    m_args[index].isSet = true;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Set per kernel arguments. The total size of all per kernel arguments and per thread
//! arguments should be less than or equal to 256 Bytes (CM_MAX_ARG_SIZE_IN_BYTE).
//! The life time of all per kernel arguments and per thread lasts untill the next enqueue
//! i.e. after enqueue, ALL arguments need to be reset.
//! INPUT:
//!     1) Index of argument in CM kernel function (genx_main). The index is
//!        global for per kernel arguments and per thread arguments.
//!     2) Size of the argument.
//!     3) Pointer to argument value.
//! OUTPUT:
//!     CM_SUCCESS or
//!     CM_INVALID_ARG_INDEX if index is invalid;
//!     CM_INVALID_ARG_SIZE if size is invalid;
//!     CM_INVALID_ARG_VALUE if value is NULL.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetKernelArg(uint32_t index, size_t size, const void * value )
{
    INSERT_API_CALL_LOG();
    //It should be mutual exclusive with Indirect Data
    if(m_kernelPayloadData)
    {
        CM_ASSERTMESSAGE("Error: SetKernelArg should be mutual exclusive with indirect data.");
        return CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL;
    }

    if( index >= m_argCount )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg count.");
        return CM_INVALID_ARG_INDEX;

    }

    if( !value)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg value.");
        return CM_INVALID_ARG_VALUE;
    }

    if( size == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_INVALID_ARG_SIZE;
    }

    int32_t nRetVal = 0;
    if ( ( nRetVal = SetArgsInternal( CM_KERNEL_INTERNEL_ARG_PERKERNEL, index, size, value ) ) != CM_SUCCESS )
    {
        return nRetVal;
    }

    return CM_SUCCESS;
}

CM_RT_API int32_t CmKernelRT::SetKernelArgPointer(uint32_t index, size_t size, const void *value)
{
    INSERT_API_CALL_LOG();

    //It should be mutual exclusive with Indirect Data
    if (m_kernelPayloadData)
    {
        CM_ASSERTMESSAGE("Error: SetKernelArg should be mutual exclusive with indirect data.");
        return CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL;
    }

    if (index >= m_argCount)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg count.");
        return CM_INVALID_ARG_INDEX;
    }

    if (!value)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg value.");
        return CM_INVALID_ARG_VALUE;
    }

    uint64_t *argValue = MOS_NewArray(uint64_t, 1);
    if (!argValue)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet(argValue, 0, sizeof(uint64_t));
    CmSafeMemCopy(argValue, value, size);

    // Get the gfx start address of SVM/stateless buffer.
    uint64_t gfxAddress = *(argValue);
    MosSafeDeleteArray(argValue);

    // Check the gfx start address is valid or not
    std::set<CmSurface *> statelessSurfArray = m_surfaceMgr->GetStatelessSurfaceArray();
    bool valid = false;
    for(auto surface : statelessSurfArray)
    {
        CmBuffer_RT *buffer = static_cast<CmBuffer_RT *>(surface);
        uint64_t startAddress = 0;
        buffer->GetGfxAddress(startAddress);
        size_t size = buffer->GetSize();

        if (gfxAddress >= startAddress
            && gfxAddress < (startAddress + size))
        {
            SurfaceIndex *surfIndex = nullptr;
            buffer->GetIndex(surfIndex);
            uint32_t surfIndexData = surfIndex->get_data();
            m_surfaceArray[surfIndexData] = true;

            m_args[index].isStatelessBuffer = true;
            m_args[index].index = (uint16_t)surfIndexData;

            valid = true;
            break;
        }
    }
    if (!valid)
    {
        CM_ASSERTMESSAGE("Error: the kernel arg pointer is not valid.");
        return CM_INVALID_KERNEL_ARG_POINTER;
    }

    int32_t nRetVal = SetArgsInternal(CM_KERNEL_INTERNEL_ARG_PERKERNEL,
                                      index,
                                      size,
                                      value);
    if (nRetVal != CM_SUCCESS)
    {
        return nRetVal;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Set Static Buffer
//| Return :   The result of operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetStaticBuffer(uint32_t index, const void * value )
{
    INSERT_API_CALL_LOG();
    if(index >= CM_GLOBAL_SURFACE_NUMBER)
    {
        CM_ASSERTMESSAGE("Error: Surface Index exceeds max global surface number.");
        return CM_INVALID_GLOBAL_BUFFER_INDEX;
    }

    if(!value)
    {
        CM_ASSERTMESSAGE("Error: Invalid StaticBuffer arg value.");
        return CM_INVALID_BUFFER_HANDLER;
    }

    SurfaceIndex* surfIndex = (SurfaceIndex* )value;
    uint32_t surfIndexData = surfIndex->get_data();
    if (surfIndexData >= m_surfaceMgr->GetSurfacePoolSize())
    {
        CM_ASSERTMESSAGE("Error: StaticBuffer doesn't allow alias index.");
        return CM_INVALID_ARG_INDEX;
    }

    CmSurface* surface  = nullptr;
    m_surfaceMgr->GetSurface( surfIndexData, surface );
    if(surface == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid surface.");
        return CM_INVALID_BUFFER_HANDLER;
    }

    CmBuffer_RT* surf1D = nullptr;
    if ( surface->Type() == CM_ENUM_CLASS_TYPE_CMBUFFER_RT )
    {
        uint32_t handle = 0; // for 1D surf

        surf1D = static_cast< CmBuffer_RT* >( surface );
        surf1D->GetHandle( handle );

        if (m_globalSurfaces[index] == nullptr)
        {
            m_globalSurfaces[index] = MOS_New(SurfaceIndex,0);
            if( !m_globalSurfaces[index] )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;
            }
        }
        *m_globalSurfaces[index] = handle;
        m_globalCmIndex[index] = surfIndexData;
        m_dirty |= CM_KERNEL_DATA_GLOBAL_SURFACE_DIRTY;
    }
    else
    {
        CM_ASSERTMESSAGE("Error: StaticBuffer only supports CmBuffer type.");
         return CM_INVALID_BUFFER_HANDLER;
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//! Set per thread arguments. The total size of all per kernel arguments and per thread
//! arguments should be less than or equal to 256 Bytes
//! The life time of all per kernel arguments and per thread lasts untill the next enqueue
//! i.e. after enqueue, ALL arguments need to be reset.
//! INPUT:
//!     1) Thread index.
//!     2) Index of argument in CM kernel function (genx_main). The index is
//!        global for per kernel arguments and per thread arguments.
//!     3) Size of the argument.
//!     4) Pointer to argument .
//! OUTPUT:
//!     CM_SUCCESS or
//!     CM_INVALID_ARG_INDEX if index is invalid
//!     CM_INVALID_ARG_SIZE if size is invalid
//!     CM_INVALID_ARG_VALUE if value is nullptr
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetThreadArg(uint32_t threadId, uint32_t index, size_t size, const void * value )
{
    INSERT_API_CALL_LOG();

    //It should be mutual exclusive with Indirect Data
    if(m_kernelPayloadData)
    {
        CM_ASSERTMESSAGE("Error: SetThredArg should be mutual exclusive with indirect data.");
        return CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL;
    }

    if(m_threadCount > m_halMaxValues->maxUserThreadsPerTask || m_threadCount <=0)
    {
        CM_ASSERTMESSAGE("Error: Minimum or Maximum number of threads exceeded.");
        return CM_FAILURE;
    }

    if( index >= m_argCount )
    {
        CM_ASSERTMESSAGE("Error: Invalid thread arg count.");
        return CM_INVALID_ARG_INDEX;

    }

    if( threadId >= m_threadCount )
    {
        CM_ASSERTMESSAGE("Error: thread id exceeds the threadcount.");
        return CM_INVALID_THREAD_INDEX;

    }

    if( !value)
    {
        CM_ASSERTMESSAGE("Error: Invalid thread arg value.");
        return CM_INVALID_ARG_VALUE;
    }

    if( size == 0)
    {
        CM_ASSERTMESSAGE("Error: Invalid thread arg size.");
        return CM_INVALID_ARG_SIZE;
    }

    int32_t nRetVal = 0;
    if ( ( nRetVal = SetArgsInternal( CM_KERNEL_INTERNEL_ARG_PERTHREAD, index, size, value, threadId ) ) != CM_SUCCESS )
    {
        return nRetVal;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Calculate the total size of kernel data
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CalcKernelDataSize(
                uint32_t movInstNum,                 // [in] the number of move instructions
                uint32_t numArgs,                   // [in] number of args , surface array count
                uint32_t argSize,                   // [in] Size of arguments
                uint32_t & totalKernelDataSize)      // [out] total size of kernel data
{
    int32_t hr             = CM_SUCCESS;

    uint32_t headSize = ( KERNEL_INFO_SIZE_IN_DWORD + numArgs * PER_ARG_SIZE_IN_DWORD ) * sizeof( uint32_t );
    uint32_t totalSize =  headSize + movInstNum * CM_MOVE_INSTRUCTION_SIZE + m_binarySize + argSize;

    totalSize += 4; // one dword for flag. the first bit is curbe on/off
    totalSize += 8; //sizeof( uint64_t ) for id

    totalSize += 16; // static buffer indices
    totalSize += 12; // GT Pin buffer indices

    ////////////////////////////////////////////////////////////////////////////
    // Calculate indirect data size (start)
    ////////////////////////////////////////////////////////////////////////////
    // Memory layout for indirect data:
    // Indirect Data Size -------------------- 2 bytes (must present)
    // Below area is present only if above value is not ZERO
    // Indirect Data Buffer ------------------ Size indicated above
    totalSize += sizeof(uint16_t);  //field for indirect data size
    if(m_usKernelPayloadDataSize)
    {
        totalSize += m_usKernelPayloadDataSize;
    }
    // Memory layout for indirect surface:
    // Indirect Surface Count ----------------- 2 bytes (must present)
    // Below are present only if the above value is not ZERO
    // Kind of Indirect Surface 0 ------------- 2 Bytes
    // Handle of Indirect Surface 0 ----------- 2 Bytes
    // Surface Index of Indirect Surface 0 ---- 2 Bytes
    // ..........
    // Kind of Indirect Surface n-1 ----------- 2 Bytes
    // Handle of Indirect Surface n-1---------- 2 Bytes
    // Surface Index of Indirect Surface n-1 -- 2 Bytes
    totalSize +=  sizeof(uint16_t); //field for indirect surface count
    if(m_usKernelPayloadSurfaceCount)
    {
        totalSize +=  m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO);
    }

    totalKernelDataSize = totalSize;

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Create mov instructions
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateMovInstructions( uint32_t &movInstNum, uint8_t *&codeDst, CM_ARG* tempArgs, uint32_t numArgs)
{
    //Create Mov Instruction
    CmDynamicArray      movInsts( numArgs );
    uint32_t renderGen = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState->platform.eRenderCoreFamily;
    CM_RETURN_CODE ret = m_movInstConstructor->SetInstDistanceConfig(movInsts.GetMaxSize(), renderGen);
    if (ret != CM_SUCCESS && ret != CM_NOT_IMPLEMENTED)
    {
        return ret;
    }

    movInstNum = 0;

    //Note: if no thread arg and no per kernel arg, no need move instrcutions at all.
    if( m_curbeEnabled && (m_perThreadArgExists || m_perKernelArgExists))
    {
        if( ( m_argCount > 0 ) && ( m_threadCount > 1) )
        {
            PCM_ARG* sortedArgs = MOS_NewArray(PCM_ARG,numArgs);
            if( !sortedArgs )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;
            }
            for( uint32_t j = 0; j < numArgs; j++ )
            {
                sortedArgs[ j ] = tempArgs + j;
            }
            // sort arg to sortedArgs accorind to offsetinPayload
            QuickSort( sortedArgs, 0, numArgs - 1 );

            // record compiler generated offset, used as move dst later
            uint16_t *unitOffsetInPayloadSorted = MOS_NewArray(uint16_t, numArgs);
            if( !unitOffsetInPayloadSorted )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                MosSafeDeleteArray(sortedArgs);
                return CM_OUT_OF_HOST_MEMORY;
            }
            for( uint32_t j = 0; j < numArgs; j++ )
            {
                unitOffsetInPayloadSorted[j] = sortedArgs[j]->unitOffsetInPayload;
            }

            uint16_t kernelArgEnd = 32;
            bool beforeFirstThreadArg = true;
            for( uint32_t j = 0; j < numArgs; j++ )
            {
                if( sortedArgs[j]->unitCount == 1 )
                    // consider m_threadCount = 1 case later, where all args are treated as per thread arg
                {
                    if( beforeFirstThreadArg )
                    {
                        kernelArgEnd = sortedArgs[j]->unitOffsetInPayload + sortedArgs[j]->unitSize;
                    }
                    else
                    {
                        DW_ALIGNMENT( kernelArgEnd ); // necessary ?
                        sortedArgs[j]->unitOffsetInPayload = kernelArgEnd;
                        kernelArgEnd += sortedArgs[j]->unitSize;
                    }
                }
                else // per thread
                {
                    if( beforeFirstThreadArg )
                    {
                        beforeFirstThreadArg = false;
                    }
                }
            }

            GRF_ALIGNMENT(kernelArgEnd); // offset of thread arg start related to R0
            uint32_t threadArgStart = kernelArgEnd;

            for (uint32_t j = 0; j < numArgs; j++)
            {
                if (sortedArgs[j]->unitCount > 1) // per thread
                {
                    sortedArgs[j]->unitOffsetInPayload = (uint16_t)threadArgStart;
                    threadArgStart += sortedArgs[j]->unitSize;
                    DW_ALIGNMENT(threadArgStart);
                }
            }

            bool needMovInstructions = false;
            for( uint32_t j = 0; j < numArgs; j++ )
            {
                if ( unitOffsetInPayloadSorted[j] != sortedArgs[j]->unitOffsetInPayload )
                {
                    needMovInstructions = true;
                    break;
                }
            }

            if (needMovInstructions)
            {
                // Add move
                GRF_ALIGNMENT(threadArgStart);
                uint32_t threadArgEnd = threadArgStart;
                uint32_t size = threadArgEnd - 32;
                CM_ASSERT((size % 32) == 0);

                // move all arguments starting from R1 (32 ) through threadArgEnd to R64 (R0 reserved for media dispatch)
                uint32_t nextIndex = 0;
                nextIndex += m_movInstConstructor->ConstructObjMovs(R64_OFFSET, 32, size, movInsts, nextIndex, true, m_blhwDebugEnable);

                beforeFirstThreadArg = true;
                for (uint32_t j = 0; j < numArgs; j++)
                {
                    if (sortedArgs[j]->unitCount == 1)
                        // consider m_threadCount = 1 case later, where all args are treated as per thread arg
                    {
                        if (beforeFirstThreadArg == false)
                        {
                            // add move inst to move from sortedArgs[j]->unitOffsetInPayload + R64 to unitOffsetInPayloadSorted[j]
                            nextIndex += m_movInstConstructor->ConstructObjMovs(unitOffsetInPayloadSorted[j],
                                R64_OFFSET + sortedArgs[j]->unitOffsetInPayload - 32,
                                sortedArgs[j]->unitSize, movInsts, nextIndex, true, m_blhwDebugEnable);
                        }
                    }
                    else // per thread
                    {
                        if (beforeFirstThreadArg)
                        {
                            beforeFirstThreadArg = false;
                        }

                        // add move inst to move from sortedArgs[j]->unitOffsetInPayload + R64 to unitOffsetInPayloadSorted[j]
                        nextIndex += m_movInstConstructor->ConstructObjMovs(unitOffsetInPayloadSorted[j],
                            R64_OFFSET + sortedArgs[j]->unitOffsetInPayload - CM_PAYLOAD_OFFSET,
                            sortedArgs[j]->unitSize, movInsts, nextIndex, true, m_blhwDebugEnable);
                    }
                }

                movInstNum = nextIndex;
            }

            MosSafeDeleteArray(sortedArgs);
            MosSafeDeleteArray(unitOffsetInPayloadSorted);
        }
    }// End of if( m_curbeEnabled && m_ThreadArgExists)

    uint32_t addInstDW[4];
    MOS_ZeroMemory(addInstDW, CM_MOVE_INSTRUCTION_SIZE);
    uint32_t addInstNum =0;

    if(m_threadSpace && m_adjustScoreboardY)
    {
        addInstNum = 1;

        addInstDW[0] = CM_BDW_ADJUST_Y_SCOREBOARD_DW0;
        addInstDW[1] = CM_BDW_ADJUST_Y_SCOREBOARD_DW1;
        addInstDW[2] = CM_BDW_ADJUST_Y_SCOREBOARD_DW2;

        // constant word needs high 16 bits to be same as low 16 bits
        uint16_t tmp = - (int32_t)(m_adjustScoreboardY);
        addInstDW[3] = (tmp << 16) + tmp;

    }

    if (movInstNum || addInstNum)
    {
        codeDst = MOS_NewArray(uint8_t, ((movInstNum + addInstNum)  * CM_MOVE_INSTRUCTION_SIZE));
        if (!codeDst)
        {
            return CM_OUT_OF_HOST_MEMORY;
        }
    }

    for( uint32_t j = 0; j < movInstNum; j ++ )
    {
        MovInst_RT* movInst = (MovInst_RT*)movInsts.GetElement( j );
        if (!movInst)
        {
            CM_ASSERTMESSAGE("Error: Invalid move instructions.");
            MosSafeDeleteArray(codeDst);
            return CM_FAILURE;
        }
        if (j != 0)
        {
            movInst->ClearDebug();
        }
        CmSafeMemCopy(codeDst + j * CM_MOVE_INSTRUCTION_SIZE, movInst->GetBinary(), CM_MOVE_INSTRUCTION_SIZE);
        CmSafeDelete(movInst); // delete each element in movInsts
    }
    movInsts.Delete();

    if(addInstNum != 0)
    {
       CmSafeMemCopy(codeDst + movInstNum * CM_MOVE_INSTRUCTION_SIZE, addInstDW, CM_MOVE_INSTRUCTION_SIZE);

       movInstNum += addInstNum; // take add Y instruction into consideration
    }

    return CM_SUCCESS;
}

int32_t CmKernelRT::CreateKernelArgDataGroup(
    uint8_t   *&data,
    uint32_t   value)
{
    if (data == nullptr)
    {
        data = MOS_NewArray(uint8_t, sizeof(uint32_t));
        if(!data)
        {
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    *(uint32_t *)data = value;
    return CM_SUCCESS;
}

int32_t CmKernelRT::CreateKernelImplicitArgDataGroup(
    uint8_t   *&data,
    uint32_t   size)
{
    data = MOS_NewArray(uint8_t, (size * sizeof(uint32_t)));
    if (!data)
    {
        return CM_OUT_OF_HOST_MEMORY;
    }
    *(uint32_t *)data = 0;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Create mov instructions
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateThreadArgData(
    PCM_HAL_KERNEL_ARG_PARAM    kernelArg,
    uint32_t                    threadArgIndex,
    CmThreadSpaceRT*              threadSpace,
    CM_ARG*                     cmArgs )
{
    int32_t         hr              = CM_SUCCESS;
    uint32_t        threadArgCount  = cmArgs[ threadArgIndex].unitCount;
    uint32_t        threadArgSize   = cmArgs[ threadArgIndex ].unitSize;

    if (CHECK_SURFACE_TYPE(cmArgs->unitKind,  ARG_KIND_SURFACE_VME))
    {
        // reallocate the memory since the number of surfaces in a vme surface could vary
        MosSafeDeleteArray(kernelArg->firstValue);
    }

    if( kernelArg->firstValue  == nullptr)
    {
        // if firstValue = nullptr, then create a new one, otherwise, update the exisitng one
        kernelArg->firstValue = MOS_NewArray(uint8_t, (cmArgs[threadArgIndex].unitCount * cmArgs[threadArgIndex].unitSize));
        if( !kernelArg->firstValue )
        {
            hr = CM_OUT_OF_HOST_MEMORY;
            goto finish;
        }
    }

    if(kernelArg->unitCount == 1 ) // kernel arg
    {
        if (cmArgs[threadArgIndex].value)
        {
            CmSafeMemCopy(kernelArg->firstValue, cmArgs[threadArgIndex].value, threadArgCount * threadArgSize);
        }
        goto finish;
    }

    if( threadSpace != nullptr )
    {
        CM_DEPENDENCY_PATTERN dependencyPatternType = CM_NONE_DEPENDENCY;
        threadSpace->GetDependencyPatternType(dependencyPatternType);

        if ((m_threadSpaceAssociated == true) &&  (dependencyPatternType != CM_NONE_DEPENDENCY))
        {
            CM_THREAD_SPACE_UNIT *threadSpaceUnit = nullptr;
            threadSpace->GetThreadSpaceUnit(threadSpaceUnit);

            uint32_t *boardOrder = nullptr;
            threadSpace->GetBoardOrder(boardOrder);

            for (uint32_t index = 0; index < threadArgCount; index++)
            {
                uint32_t offset = threadSpaceUnit[boardOrder[index]].threadId;
                uint8_t *argSrc = (uint8_t*)cmArgs[threadArgIndex].value + offset * threadArgSize;
                uint8_t *argDst = kernelArg->firstValue + index * threadArgSize;
                CmSafeMemCopy(argDst, argSrc, threadArgSize);
            }
        }
        else
        {
           CmSafeMemCopy(kernelArg->firstValue, cmArgs[ threadArgIndex ].value, threadArgCount * threadArgSize);
        }
    }
    else
    {
        CmSafeMemCopy(kernelArg->firstValue, cmArgs[ threadArgIndex ].value, threadArgCount * threadArgSize);
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Sort thread space for scorboarding
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SortThreadSpace( CmThreadSpaceRT*  threadSpace )
{
    int32_t                   hr = CM_SUCCESS;
    CM_DEPENDENCY_PATTERN dependencyPatternType = CM_NONE_DEPENDENCY;

    CM_CHK_NULL_GOTOFINISH_CMERROR(threadSpace);

    threadSpace->GetDependencyPatternType(dependencyPatternType);

    if(!threadSpace->IsThreadAssociated())
    {//Skip Sort if it is media walker
        return CM_SUCCESS;
    }

    if (threadSpace->CheckDependencyVectorsSet())
    {
        threadSpace->WavefrontDependencyVectors();
    }
    else
    {
        switch (dependencyPatternType)
        {
            case CM_WAVEFRONT:
                threadSpace->Wavefront45Sequence();
                break;

            case CM_WAVEFRONT26:
                threadSpace->Wavefront26Sequence();
                break;

            case CM_WAVEFRONT26Z:
                threadSpace->Wavefront26ZSequence();
                break;

            case CM_WAVEFRONT26ZI:
                CM_26ZI_DISPATCH_PATTERN dispatchPattern;
                threadSpace->Get26ZIDispatchPattern(dispatchPattern);
                switch (dispatchPattern)
                {
                case VVERTICAL_HVERTICAL_26:
                    threadSpace->Wavefront26ZISeqVVHV26();
                    break;
                case VVERTICAL_HHORIZONTAL_26:
                    threadSpace->Wavefront26ZISeqVVHH26();
                    break;
                case VVERTICAL26_HHORIZONTAL26:
                    threadSpace->Wavefront26ZISeqVV26HH26();
                    break;
                case VVERTICAL1X26_HHORIZONTAL1X26:
                    threadSpace->Wavefront26ZISeqVV1x26HH1x26();
                    break;
                default:
                    threadSpace->Wavefront26ZISeqVVHV26();
                    break;
                }
                break;

            case CM_HORIZONTAL_WAVE:
                threadSpace->HorizentalSequence();
                break;

            case CM_VERTICAL_WAVE:
                threadSpace->VerticalSequence();
                break;

            case CM_NONE_DEPENDENCY:
            case CM_WAVEFRONT26X:
            case CM_WAVEFRONT26ZIG:
                break;

            default:
                CM_ASSERTMESSAGE("Error: Invalid thread dependency type.");
                hr = CM_FAILURE;
                break;
        }
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Create temp args array with surface array broken down
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateTempArgs(
    uint32_t     numArgs,
    CM_ARG*      &tempArgs)
{
    int32_t     hr              = CM_SUCCESS;
    int32_t     numSurfaces    = 0;
    int32_t     increasedArgs  = 0;

    if( numArgs < m_argCount || tempArgs != nullptr )
    {
        CM_ASSERTMESSAGE("Error: Invalid arg number or arg value.");
        hr = CM_FAILURE;
        goto finish;
    }

    tempArgs = MOS_NewArray(CM_ARG, numArgs);
    CM_CHK_NULL_GOTOFINISH(tempArgs, CM_OUT_OF_HOST_MEMORY);
    CmSafeMemSet(tempArgs, 0, numArgs* sizeof(CM_ARG) );

    for( uint32_t j = 0; j < m_argCount; j++ )
    {
        if ( CHECK_SURFACE_TYPE( m_args[ j ].unitKind, // first time
                                ARG_KIND_SURFACE,
                                ARG_KIND_SURFACE_1D,
                                ARG_KIND_SURFACE_2D,
                                ARG_KIND_SURFACE_2D_UP,
                                ARG_KIND_SURFACE_SAMPLER,
                                ARG_KIND_SURFACE2DUP_SAMPLER,
                                ARG_KIND_SURFACE_3D,
                                ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                                ARG_KIND_SURFACE_SAMPLER8X8_VA,
                                ARG_KIND_SURFACE_2D_SCOREBOARD,
                                ARG_KIND_STATE_BUFFER ) )
        {
            numSurfaces = m_args[j].unitSize/sizeof(int);

            if (numSurfaces > 1)
            {
                if (m_args[j].unitCount == 1)
                { //Kernel arg
                    for (int32_t k = 0; k < numSurfaces; k++)
                    {
                        tempArgs[j + increasedArgs + k] = m_args[j];
                        tempArgs[j + increasedArgs + k].unitSize = sizeof(int32_t);
                        tempArgs[j + increasedArgs + k].unitSizeOrig = sizeof(int32_t);
                        tempArgs[j + increasedArgs + k].value = (uint8_t *)((uint32_t *)m_args[j].value + k);
                        tempArgs[j + increasedArgs + k].unitOffsetInPayload = m_args[j].unitOffsetInPayload + 4 * k;
                        tempArgs[j + increasedArgs + k].unitOffsetInPayloadOrig = tempArgs[j + increasedArgs + k].unitOffsetInPayload;
                        //For each surface kind and custom value  in surface array
                        if (!m_args[j].surfIndex[k])
                        {
                            //if surfIndex is 0, set kind to be CM_ARGUMENT_SURFACE2D
                            //This is for special usage if there is empty element in surface array.
                            tempArgs[j + increasedArgs + k].unitKind = CM_ARGUMENT_SURFACE2D;
                            continue;
                        }
                        tempArgs[j + increasedArgs + k].unitKind = m_args[j].surfArrayArg[k].argKindForArray;
                        tempArgs[j + increasedArgs + k].nCustomValue = m_args[j].surfArrayArg[k].addressModeForArray;
                    }
                }
                else
                {
                    uint32_t *surfaces = (uint32_t *)MOS_NewArray(uint8_t, ((sizeof(int32_t) * m_args[j].unitCount)));
                    CM_CHK_NULL_GOTOFINISH(surfaces, CM_OUT_OF_HOST_MEMORY);
                    for (int32_t k = 0; k < numSurfaces; k++)
                    {
                        tempArgs[j + increasedArgs + k] = m_args[j];
                        tempArgs[j + increasedArgs + k].unitSize = sizeof(int32_t);
                        tempArgs[j + increasedArgs + k].unitSizeOrig = sizeof(int32_t);
                        tempArgs[j + increasedArgs + k].value = MOS_NewArray(uint8_t, ((sizeof(int32_t) * m_args[j].unitCount)));
                        if(tempArgs[j + increasedArgs + k].value == nullptr)
                        {
                            CM_ASSERTMESSAGE("Error: Out of system memory.");
                            hr = CM_OUT_OF_HOST_MEMORY;
                            MosSafeDeleteArray(surfaces);
                            goto finish;
                        }
                        for (uint32_t s = 0; s < m_args[j].unitCount; s++)
                        {
                            surfaces[s] = *(uint32_t *)((uint32_t *)m_args[j].value + k + numSurfaces * s);
                        }
                        CmSafeMemCopy(tempArgs[j + increasedArgs + k].value, surfaces, sizeof(int32_t) * m_args[j].unitCount);
                        tempArgs[j + increasedArgs + k].unitOffsetInPayload = m_args[j].unitOffsetInPayload + 4 * k;
                        tempArgs[j + increasedArgs + k].unitOffsetInPayloadOrig = (uint16_t)-1;
                    }
                    MosSafeDeleteArray(surfaces);
                }
                increasedArgs += numSurfaces - 1;
            }
            else
            {
                tempArgs[j + increasedArgs] = m_args[j];
            }
        }
        else if (m_args[ j ].unitKind == ARG_KIND_SURFACE_VME)
        {
            numSurfaces = m_args[ j ].unitVmeArraySize;
            if(numSurfaces == 1)
            {  // single vme surface
               tempArgs[j + increasedArgs] = m_args[j];
            }
            else
            {  // multiple vme surfaces in surface array
                if (m_args[j].unitCount == 1) { //Kernel arg
                    uint32_t vmeSurfOffset = 0;

                    for (int32_t k = 0; k < numSurfaces; k++)
                    {
                        uint16_t vmeSize = (uint16_t)getVmeArgValueSize((PCM_HAL_VME_ARG_VALUE)(m_args[j].value + vmeSurfOffset));

                        tempArgs[j + increasedArgs + k] = m_args[j];
                        tempArgs[j + increasedArgs + k].unitSize = vmeSize;
                        tempArgs[j + increasedArgs + k].unitSizeOrig = vmeSize;
                        tempArgs[j + increasedArgs + k].value = (uint8_t *)(m_args[j].value + vmeSurfOffset);
                        tempArgs[j + increasedArgs + k].unitOffsetInPayload = m_args[j].unitOffsetInPayload + k*4;
                        tempArgs[j + increasedArgs + k].unitOffsetInPayloadOrig = tempArgs[j + increasedArgs + k].unitOffsetInPayload;

                        vmeSurfOffset += vmeSize;
                    }
                }
             }
            increasedArgs += numSurfaces - 1;
        }
        else if (m_args[j].unitKind == ARG_KIND_SAMPLER)
        {
            unsigned int numSamplers = m_args[j].unitSize / sizeof(int);

            if (numSamplers > 1)
            {
                if (m_args[j].unitCount == 1)
                {
                    //Kernel arg
                    for (unsigned int k = 0; k < numSamplers; k++)
                    {
                        tempArgs[j + increasedArgs + k] = m_args[j];
                        tempArgs[j + increasedArgs + k].unitSize = sizeof(int);
                        tempArgs[j + increasedArgs + k].unitSizeOrig = sizeof(int);
                        tempArgs[j + increasedArgs + k].value = (unsigned char *)((unsigned int *)m_args[j].value + k);
                        tempArgs[j + increasedArgs + k].unitOffsetInPayload = m_args[j].unitOffsetInPayload + 4 * k;
                        tempArgs[j + increasedArgs + k].unitOffsetInPayloadOrig = tempArgs[j + increasedArgs + k].unitOffsetInPayload;
                        tempArgs[j + increasedArgs + k].unitKind = CM_ARGUMENT_SAMPLER;
                    }
                }
                else
                {
                    // Use sampler index array as thread arg.
                    // Not implemented yet.
                    return CM_NOT_IMPLEMENTED;
                }
                increasedArgs += numSamplers - 1;
            }
            else
            {
                tempArgs[j + increasedArgs] = m_args[j];
            }
        }
        else
        {
            tempArgs[j + increasedArgs] = m_args[j];
        }
    }

finish:
    if(hr == CM_OUT_OF_HOST_MEMORY)
    {
        if(tempArgs)
        {
            for (uint32_t j = 0; j < numArgs; j++)
            {
                MosSafeDeleteArray(tempArgs[j].value);
            }
        }
        MosSafeDeleteArray( tempArgs );
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the number of args includes the num of surfaces in surface array
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetArgCountPlusSurfArray(uint32_t &argSize, uint32_t & argCountPlus)
{
    argCountPlus = m_argCount;
    argSize      = 0;

    if(m_usKernelPayloadDataSize)
    { // if payload data exists, the number of args is zero
        argCountPlus  = 0;
        argSize       = 0;
        return CM_SUCCESS;
    }

    if( m_argCount != 0 )   //Need pass the arg either by arguments area, or by indirect payload area
    {
         //Sanity check for argument setting
        if((m_perThreadArgExists == false) && (m_perKernelArgExists == false) && (m_usKernelPayloadDataSize == 0))
        {
            if ( m_stateBufferBounded == CM_STATE_BUFFER_NONE )
            {
                CM_ASSERTMESSAGE( "Error: Kernel arguments are not set." );
                return CM_NOT_SET_KERNEL_ARGUMENT;
            }
        }

        if(m_perThreadArgExists || m_perKernelArgExists)
        {
            unsigned int extraArgs = 0;
            
            for( uint32_t j = 0; j < m_argCount; j ++ )
            {
                //Sanity checking for every argument setting
                if ( !m_args[j].isSet )
                {
                    CM_ASSERTMESSAGE("Error: One Kernel argument is not set.");
                    return CM_KERNEL_ARG_SETTING_FAILED;
                }

                argSize += m_args[j].unitSize * m_args[j].unitCount;

                if ( CHECK_SURFACE_TYPE( m_args[ j ].unitKind,
                                        ARG_KIND_SURFACE,
                                        ARG_KIND_SURFACE_1D,
                                        ARG_KIND_SURFACE_2D,
                                        ARG_KIND_SURFACE_2D_UP,
                                        ARG_KIND_SURFACE_SAMPLER,
                                        ARG_KIND_SURFACE2DUP_SAMPLER,
                                        ARG_KIND_SURFACE_3D,
                                        ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                                        ARG_KIND_SURFACE_SAMPLER8X8_VA,
                                        ARG_KIND_SURFACE_2D_SCOREBOARD,
                                        ARG_KIND_STATE_BUFFER ) )
                {
                     int numSurfaces = m_args[j].unitSize/sizeof(int);
                     if (numSurfaces > 1) {
                           extraArgs += numSurfaces - 1;
                     }
                }
                else if (CHECK_SURFACE_TYPE(m_args[j].unitKind, ARG_KIND_SURFACE_VME))
                {
                    int numSurfaces = m_args[j].unitVmeArraySize;
                    if (numSurfaces > 1) {
                        extraArgs += numSurfaces - 1;
                    }
                }
                else if (m_args[j].unitKind == ARG_KIND_SAMPLER)
                {
                    int numSamplers = m_args[j].unitSize / sizeof(int);
                    if (numSamplers > 1)
                    {
                        extraArgs += (numSamplers - 1);
                    }
                }
            }

            argCountPlus = m_argCount + extraArgs;
        }
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Create Thread Space Param
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateThreadSpaceParam(
    PCM_HAL_KERNEL_THREADSPACE_PARAM kernelThreadSpaceParam,
    CmThreadSpaceRT*                   threadSpace     )
{
    int32_t                      hr = CM_SUCCESS;
    CM_HAL_DEPENDENCY*           dependency = nullptr;
    uint32_t                     threadSpaceWidth = 0;
    uint32_t                     threadSpaceHeight =0;
    CM_THREAD_SPACE_UNIT         *threadSpaceUnit = nullptr;
    CM_THREAD_SPACE_DIRTY_STATUS dirtyStatus = CM_THREAD_SPACE_CLEAN;

    if (kernelThreadSpaceParam == nullptr || threadSpace == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CmKernelThreadSpaceParam or thread space is null.");
        hr = CM_NULL_POINTER;
        goto finish;
    }

    threadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);
    kernelThreadSpaceParam->threadSpaceWidth =  (uint16_t)threadSpaceWidth;
    kernelThreadSpaceParam->threadSpaceHeight = (uint16_t)threadSpaceHeight;

    threadSpace->GetDependencyPatternType(kernelThreadSpaceParam->patternType);
    threadSpace->GetWalkingPattern(kernelThreadSpaceParam->walkingPattern);
    threadSpace->GetDependency( dependency);

    if(dependency != nullptr)
    {
        CmSafeMemCopy(&kernelThreadSpaceParam->dependencyInfo, dependency, sizeof(CM_HAL_DEPENDENCY));
    }

    if( threadSpace->CheckWalkingParametersSet( ) )
    {
        kernelThreadSpaceParam->walkingParamsValid = 1;
        CM_CHK_CMSTATUS_GOTOFINISH(threadSpace->GetWalkingParameters(kernelThreadSpaceParam->walkingParams));
    }
    else
    {
        kernelThreadSpaceParam->walkingParamsValid = 0;
    }

    if( threadSpace->CheckDependencyVectorsSet( ) )
    {
        kernelThreadSpaceParam->dependencyVectorsValid = 1;
        CM_CHK_CMSTATUS_GOTOFINISH(threadSpace->GetDependencyVectors(kernelThreadSpaceParam->dependencyVectors));
    }
    else
    {
        kernelThreadSpaceParam->dependencyVectorsValid = 0;
    }

    threadSpace->GetThreadSpaceUnit(threadSpaceUnit);

    if(threadSpaceUnit)
    {
        kernelThreadSpaceParam->threadCoordinates = MOS_NewArray(CM_HAL_SCOREBOARD, (threadSpaceWidth * threadSpaceHeight));
        CM_CHK_NULL_GOTOFINISH(kernelThreadSpaceParam->threadCoordinates , CM_OUT_OF_HOST_MEMORY);
        CmSafeMemSet(kernelThreadSpaceParam->threadCoordinates, 0, threadSpaceHeight * threadSpaceWidth * sizeof(CM_HAL_SCOREBOARD));

        uint32_t *boardOrder = nullptr;
        threadSpace->GetBoardOrder(boardOrder);
        CM_CHK_NULL_GOTOFINISH_CMERROR(boardOrder);

        kernelThreadSpaceParam->reuseBBUpdateMask  = 0;
        for(uint32_t i=0; i< threadSpaceWidth * threadSpaceHeight ; i++)
        {
            kernelThreadSpaceParam->threadCoordinates[i].x = threadSpaceUnit[boardOrder[i]].scoreboardCoordinates.x;
            kernelThreadSpaceParam->threadCoordinates[i].y = threadSpaceUnit[boardOrder[i]].scoreboardCoordinates.y;
            kernelThreadSpaceParam->threadCoordinates[i].mask = threadSpaceUnit[boardOrder[i]].dependencyMask;
            kernelThreadSpaceParam->threadCoordinates[i].resetMask= threadSpaceUnit[boardOrder[i]].reset;
            kernelThreadSpaceParam->threadCoordinates[i].color = threadSpaceUnit[boardOrder[i]].scoreboardColor;
            kernelThreadSpaceParam->threadCoordinates[i].sliceSelect = threadSpaceUnit[boardOrder[i]].sliceDestinationSelect;
            kernelThreadSpaceParam->threadCoordinates[i].subSliceSelect = threadSpaceUnit[boardOrder[i]].subSliceDestinationSelect;
            kernelThreadSpaceParam->reuseBBUpdateMask |= threadSpaceUnit[boardOrder[i]].reset;
        }

        if( kernelThreadSpaceParam->patternType == CM_WAVEFRONT26Z )
        {
            CM_HAL_WAVEFRONT26Z_DISPATCH_INFO dispatchInfo;
            threadSpace->GetWavefront26ZDispatchInfo(dispatchInfo);

            kernelThreadSpaceParam->dispatchInfo.numWaves = dispatchInfo.numWaves;
            kernelThreadSpaceParam->dispatchInfo.numThreadsInWave = MOS_NewArray(uint32_t, dispatchInfo.numWaves);
            CM_CHK_NULL_GOTOFINISH(kernelThreadSpaceParam->dispatchInfo.numThreadsInWave, CM_OUT_OF_HOST_MEMORY);
            CmSafeMemCopy(kernelThreadSpaceParam->dispatchInfo.numThreadsInWave,
                dispatchInfo.numThreadsInWave, dispatchInfo.numWaves*sizeof(uint32_t));

         }
    }

    //Get group select setting information
    threadSpace->GetMediaWalkerGroupSelect(kernelThreadSpaceParam->groupSelect);

    //Get color count
    threadSpace->GetColorCountMinusOne(kernelThreadSpaceParam->colorCountMinusOne);

    dirtyStatus = threadSpace->GetDirtyStatus();
    switch (dirtyStatus)
    {
    case CM_THREAD_SPACE_CLEAN:
        kernelThreadSpaceParam->bbDirtyStatus = CM_HAL_BB_CLEAN;
        break;
    default:
        kernelThreadSpaceParam->bbDirtyStatus = CM_HAL_BB_DIRTY;
        break;
    }

finish:
    if( hr == CM_OUT_OF_HOST_MEMORY)
    {
        if( kernelThreadSpaceParam )
        {
            MosSafeDeleteArray(kernelThreadSpaceParam->dispatchInfo.numThreadsInWave);
            MosSafeDeleteArray(kernelThreadSpaceParam->threadCoordinates);
        }
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Delete the args array
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::DestroyArgs( void )
{
    for( uint32_t i =0 ; i < m_argCount; i ++ )
    {
        CM_ARG& arg = m_args[ i ];
        MosSafeDeleteArray( arg.value );
        MosSafeDeleteArray(arg.surfIndex);
        MosSafeDeleteArray(arg.surfArrayArg);
        arg.unitCount = 0;
        arg.unitSize = 0;
        arg.unitKind = 0;
        arg.unitOffsetInPayload = 0;
        arg.isDirty = true;
        arg.isSet = false;
    }

    MosSafeDeleteArray( m_args );

    m_threadSpaceAssociated        = false;
    m_threadSpace          = nullptr;

    m_perThreadArgExists  = false;
    m_perKernelArgExists  = false;

    m_sizeInCurbe = 0;
    m_curbeEnabled = true;

    m_sizeInPayload = 0;
    m_adjustScoreboardY = 0;

    ResetKernelSurfaces();

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
// Calling reset makes it possible to change the per kernel or per thread
// property of the argurments b/c it reset releases the memory for arguments
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Reset( void )
{
    for( uint32_t i =0 ; i < m_argCount; i ++ )
    {
        CM_ARG& arg = m_args[ i ];
        MosSafeDeleteArray( arg.value );
        MosSafeDeleteArray( arg.surfIndex);
        MosSafeDeleteArray(arg.surfArrayArg);
        arg.value = nullptr;
        arg.unitCount = 0;

        arg.unitSize = arg.unitSizeOrig;
        arg.unitKind = arg.unitKindOrig;
        arg.unitOffsetInPayload = arg.unitOffsetInPayloadOrig;

        arg.isDirty = true;
        arg.isSet = false;
        arg.unitVmeArraySize = 0;

        arg.isStatelessBuffer = false;
        arg.index = 0;
    }

    m_threadCount = 0;

    m_indexInTask = 0;

    m_perThreadArgExists = false;
    m_perKernelArgExists = false;

    m_sizeInCurbe = 0;
    m_curbeEnabled = true;

    m_sizeInPayload = 0;

    m_threadSpaceAssociated = false;
    m_threadSpace = nullptr;
    m_adjustScoreboardY = 0;

    m_threadGroupSpace = nullptr;

    MosSafeDeleteArray(m_kernelPayloadData);
    m_usKernelPayloadDataSize = 0;

    if (m_usKernelPayloadSurfaceCount)
    {
        CmSafeMemSet(m_pKernelPayloadSurfaceArray, 0, m_usKernelPayloadSurfaceCount * sizeof(SurfaceIndex *));
        CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
        m_usKernelPayloadSurfaceCount = 0;
    }

    ResetKernelSurfaces();

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the pointer to arguments array
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetArgs( CM_ARG* & arg )
{
    arg = m_args;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the arguments' count
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetArgCount( uint32_t & argCount )
{
    argCount = m_argCount;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the value of member CurbeEnable
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetCurbeEnable( bool& b )
{
    b = m_curbeEnabled;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set the CurbeEnable member
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetCurbeEnable( bool b )
{
    m_curbeEnabled = b;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the kernel's size in Curbe
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetSizeInCurbe( uint32_t& size )
{
    size = m_sizeInCurbe;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the total size in payload of meida object or media walker
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetSizeInPayload( uint32_t& size )
{
    size = m_sizeInPayload;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the pointer to CM device
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetCmDevice(CmDeviceRT* &device)
{
    device = m_device;
    return CM_SUCCESS;
}

int32_t CmKernelRT::GetCmProgram( CmProgramRT* & program )
{
    program = m_program;
    return CM_SUCCESS;
}

int32_t CmKernelRT::CollectKernelSurface()
{
    m_vmeSurfaceCount = 0;
    m_maxSurfaceIndexAllocated = 0;

    for( uint32_t j = 0; j < m_argCount; j ++ )
    {
        if ((m_args[ j ].unitKind == ARG_KIND_SURFACE ) || // first time
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_1D ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_2D ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_3D ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_VME ) ||
             ( m_args[ j ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
             ( m_args[ j ].unitKind == ARG_KIND_STATE_BUFFER ) )
        {
            int numSurfaces;
            int numValidSurfaces = 0;

            if (m_args[ j ].unitKind == ARG_KIND_SURFACE_VME)
            {
                numSurfaces = getSurfNumFromArgArraySize(m_args[j].unitSize, m_args[j].unitVmeArraySize);
            }
            else
            {
                numSurfaces = m_args[j].unitSize/sizeof(int);
            }

            for (uint32_t k = 0; k < numSurfaces * m_args[j].unitCount; k ++)
            {
                uint16_t surfIndex = 0;
                if (m_args[j].surfIndex)
                {
                    surfIndex = m_args[j].surfIndex[k];
                }
                if (surfIndex != 0 && surfIndex != CM_NULL_SURFACE)
                {
                    m_surfaceArray[surfIndex] = true;
                    numValidSurfaces ++;
                    m_maxSurfaceIndexAllocated = Max(m_maxSurfaceIndexAllocated, surfIndex);
                }
            }
            if (m_args[ j ].unitKind == ARG_KIND_SURFACE_VME)
            {
                m_vmeSurfaceCount += numValidSurfaces;
            }
        }

        if (m_args[ j ].isStatelessBuffer)
        {
            uint32_t surfIndex = m_args[j].index;
            m_surfaceArray[surfIndex] = true;
        }
    }

    for( int32_t i=0; i < CM_GLOBAL_SURFACE_NUMBER; ++i )
    {
        if( m_globalSurfaces[i] != nullptr )
        {
            uint32_t surfIndex = m_globalCmIndex[i];
            m_surfaceArray[surfIndex] = true;
        }
    }

    for (int32_t i = 0; i < m_usKernelPayloadSurfaceCount; i++)
    {
        if (m_pKernelPayloadSurfaceArray[i] != nullptr)
        {
            uint32_t surfIndex = m_pKernelPayloadSurfaceArray[i]->get_data();
            m_surfaceArray[surfIndex] = true;
        }
    }

    return CM_SUCCESS;
}

int32_t CmKernelRT::IsKernelDataReusable( CmThreadSpaceRT* threadSpace)
{
    if(threadSpace)
    {
        if(threadSpace->IsThreadAssociated() && (threadSpace->GetDirtyStatus()!= CM_THREAD_SPACE_CLEAN))
        {
            return false;
        }
    }

    if(m_threadSpace)
    {
        if(m_threadSpace->GetDirtyStatus()!= CM_THREAD_SPACE_CLEAN)
        {
            return  false;
        }
    }

    if(m_dirty !=  CM_KERNEL_DATA_CLEAN)
    {
        return false;
    }

    return true;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelData(
    CmKernelData* & kernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadSpaceRT* threadSpace )    // in
{
    int32_t              hr              = CM_SUCCESS;
    PCM_HAL_KERNEL_PARAM halKernelParam = nullptr;

    if( (threadSpace != nullptr) && (m_threadSpace != nullptr) )
    {
        // per-kernel threadspace and per-task threadspace cannot be set at the same time
        return CM_INVALID_THREAD_SPACE;
    }

    if(m_lastKernelData == nullptr)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelDataInternal(kernelData, kernelDataSize, threadSpace));
        CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel/program's ref count
        CM_CHK_CMSTATUS_GOTOFINISH(UpdateLastKernelData(kernelData));
    }
    else
    {
        if(IsKernelDataReusable(const_cast<CmThreadSpaceRT *>(threadSpace)))
        {
            // nothing changed; Reuse m_lastKernelData
            kernelData = m_lastKernelData;
            CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelData(kernelData));
            CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel and program's ref count
            kernelDataSize = kernelData->GetKernelDataSize();

            if (m_threadSpace)
            {
                halKernelParam = kernelData->GetHalCmKernelData();
                CM_CHK_NULL_GOTOFINISH_CMERROR(halKernelParam);
                // need to set to clean here because CmThreadSpaceParam.BBdirtyStatus is only set in CreateKernelDataInternal
                // flag used to re-use batch buffer, don't care if BB is busy if it is "clean"
                halKernelParam->kernelThreadSpaceParam.bbDirtyStatus = CM_HAL_BB_CLEAN;
            }
        }
        else
        {
            if(m_lastKernelData->IsInUse())
            { // Need to Create a new one , if the kernel data is in use
                CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelDataInternal(kernelData, kernelDataSize, threadSpace));
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel/program's ref count
                CM_CHK_CMSTATUS_GOTOFINISH(UpdateLastKernelData(kernelData));
            }
            else if(threadSpace && threadSpace->IsThreadAssociated() && (threadSpace->GetDirtyStatus() != CM_THREAD_SPACE_CLEAN))
            { // if thread space is assocaited , don't support reuse
                CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelDataInternal(kernelData, kernelDataSize, threadSpace));
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel/program's ref count
                CM_CHK_CMSTATUS_GOTOFINISH(UpdateLastKernelData(kernelData));
            }
            else if(m_dirty < CM_KERNEL_DATA_THREAD_COUNT_DIRTY || // Kernel arg or thread arg dirty
                (m_threadSpace && m_threadSpace->GetDirtyStatus() == CM_THREAD_SPACE_DEPENDENCY_MASK_DIRTY))
            {
                CM_CHK_CMSTATUS_GOTOFINISH(UpdateKernelData(m_lastKernelData,threadSpace));
                kernelData = m_lastKernelData;
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelData(kernelData));
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel and program's ref count
                kernelDataSize = kernelData->GetKernelDataSize();

            }
            else
            {
               CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelDataInternal(kernelData, kernelDataSize, threadSpace));
               CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel/program's ref count
               CM_CHK_CMSTATUS_GOTOFINISH(UpdateLastKernelData(kernelData));
            }
        }
    }

    CleanArgDirtyFlag();
    if(threadSpace)
    {
        threadSpace->SetDirtyStatus(CM_THREAD_SPACE_CLEAN);
    }
    if (m_threadSpace)
    {
        m_threadSpace->SetDirtyStatus(CM_THREAD_SPACE_CLEAN);
    }

finish:
    return hr;
}

char* CmKernelRT::GetName() { return (char*)m_kernelInfo->kernelName; }

//*-----------------------------------------------------------------------------
//| Purpose:    Create Kernel Data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelData(
    CmKernelData* & kernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadGroupSpace* threadGroupSpace )    // in
{
    int32_t     hr   = CM_SUCCESS;
    CmThreadGroupSpace* usedThreadGroupSpace = nullptr;

    //If kernel has associated TGS, we will use it, instead of per-task TGS
    if (m_threadGroupSpace)
    {
        usedThreadGroupSpace = m_threadGroupSpace;
    }
    else
    {
        usedThreadGroupSpace = const_cast<CmThreadGroupSpace*>(threadGroupSpace);
    }

    if(m_lastKernelData == nullptr)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelDataInternal(kernelData, kernelDataSize, usedThreadGroupSpace));
        CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel/program's ref count
        CM_CHK_CMSTATUS_GOTOFINISH(UpdateLastKernelData(kernelData));
    }
    else
    {
        if (!((m_dirty & CM_KERNEL_DATA_KERNEL_ARG_DIRTY) || (m_dirty & CM_KERNEL_DATA_THREAD_GROUP_SPACE_DIRTY)))
        {
            // nothing changed; Reuse m_lastKernelData
            kernelData = m_lastKernelData;
            CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelData(kernelData));
            CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel and program's ref count
            kernelDataSize = kernelData->GetKernelDataSize();
        }
        else
        {
            if(m_lastKernelData->IsInUse())
            { // Need to Clone a new one
                CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelDataInternal(kernelData, kernelDataSize, usedThreadGroupSpace));
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel/program's ref count
                CM_CHK_CMSTATUS_GOTOFINISH(UpdateLastKernelData(kernelData));
            }
            else
            {
                // change happend -> Reuse m_lastKernelData but need to change its content accordingly
                CM_CHK_CMSTATUS_GOTOFINISH(UpdateKernelData(m_lastKernelData, usedThreadGroupSpace));
                kernelData = m_lastKernelData;
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelData(kernelData));
                CM_CHK_CMSTATUS_GOTOFINISH(AcquireKernelProgram()); // increase kernel and program's ref count
                kernelDataSize = kernelData->GetKernelDataSize();
            }
        }
    }

    CleanArgDirtyFlag();

finish:
    return hr;
}

int32_t CmKernelRT::CleanArgDirtyFlag()
{

    for(uint32_t i =0 ; i< m_argCount; i++)
    {
        m_args[i].isDirty = false;
    }

    if(m_threadSpace && m_threadSpace->GetDirtyStatus())
    {
        m_threadSpace->SetDirtyStatus(CM_THREAD_SPACE_CLEAN);
    }

    m_dirty                 = CM_KERNEL_DATA_CLEAN;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update the global surface and gtpin surface info to kernel data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateKernelDataGlobalSurfaceInfo( PCM_HAL_KERNEL_PARAM halKernelParam )
{
    int32_t hr = CM_SUCCESS;

    //global surface
    for ( uint32_t j = 0; j < CM_GLOBAL_SURFACE_NUMBER; j++ )
    {
        if ( m_globalSurfaces[ j ] != nullptr )
        {
            halKernelParam->globalSurface[ j ] = m_globalSurfaces[ j ]->get_data();
            halKernelParam->globalSurfaceUsed = true;
        }
        else
        {
            halKernelParam->globalSurface[ j ] = CM_NULL_SURFACE;
        }
    }

    for ( uint32_t j = CM_GLOBAL_SURFACE_NUMBER; j < CM_MAX_GLOBAL_SURFACE_NUMBER; j++ )
    {
        halKernelParam->globalSurface[ j ] = CM_NULL_SURFACE;
    }
#if USE_EXTENSION_CODE
    UpdateKernelDataGTPinSurfaceInfo(halKernelParam);
#endif

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelDataInternal(
    CmKernelData* & kernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadGroupSpace* threadGroupSpace)    // in
{
    PCM_HAL_KERNEL_PARAM  halKernelParam = nullptr;
    int32_t               hr = CM_SUCCESS;
    uint32_t              movInstNum = 0;
    uint32_t              kernelCurbeSize = 0;
    uint32_t              numArgs = 0;
    CM_ARG                *tempArgs = nullptr;
    uint32_t              argSize = 0;
    uint32_t              surfNum = 0; //Pass needed BT entry numbers to HAL CM
    CmKernelRT            *cmKernel = nullptr;
    uint32_t              minKernelPlayloadOffset = 0;
    bool                  adjustLocalIdPayloadOffset = false;

    CM_CHK_CMSTATUS_GOTOFINISH(CmKernelData::Create(this, kernelData));
    halKernelParam = kernelData->GetHalCmKernelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(halKernelParam);

    //Get Num of args with surface array
    CM_CHK_CMSTATUS_GOTOFINISH(GetArgCountPlusSurfArray(argSize, numArgs));

    //Create Temp args
    CM_CHK_CMSTATUS_GOTOFINISH(CreateTempArgs(numArgs, tempArgs));

    //Create move instructions
    CM_CHK_CMSTATUS_GOTOFINISH(CreateMovInstructions(movInstNum, halKernelParam->movInsData, tempArgs, numArgs));
    CM_CHK_CMSTATUS_GOTOFINISH(CalcKernelDataSize(movInstNum, numArgs, argSize, kernelDataSize));
    CM_CHK_CMSTATUS_GOTOFINISH(kernelData->SetKernelDataSize(kernelDataSize));

    halKernelParam->clonedKernelParam.isClonedKernel = m_isClonedKernel;
    halKernelParam->clonedKernelParam.kernelID       = m_cloneKernelID;
    halKernelParam->clonedKernelParam.hasClones      = m_hasClones;

    halKernelParam->kernelId = m_id++;
    if ((m_program->m_cisaMajorVersion >= 3 && m_program->m_cisaMinorVersion >= 3))
        halKernelParam->numArgs = numArgs;
    else
        halKernelParam->numArgs = numArgs + CM_GPUWALKER_IMPLICIT_ARG_NUM;
    halKernelParam->numThreads = m_threadCount;
    halKernelParam->kernelBinarySize = m_binarySize + movInstNum * CM_MOVE_INSTRUCTION_SIZE;
    halKernelParam->kernelDataSize = kernelDataSize;
    halKernelParam->movInsDataSize = movInstNum * CM_MOVE_INSTRUCTION_SIZE;
    halKernelParam->kernelDebugEnabled = m_blhwDebugEnable;

    halKernelParam->cmFlags = m_curbeEnabled ? CM_FLAG_CURBE_ENABLED : 0;
    halKernelParam->cmFlags |= m_nonstallingScoreboardEnabled ? CM_FLAG_NONSTALLING_SCOREBOARD_ENABLED : 0;

    halKernelParam->kernelBinary = (uint8_t*)m_binary;

    CM_CHK_CMSTATUS_GOTOFINISH(kernelData->GetCmKernel(cmKernel));
    if (cmKernel == nullptr)
    {
        return CM_NULL_POINTER;
    }
    MOS_SecureStrcpy(halKernelParam->kernelName, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, cmKernel->GetName());

    uint32_t thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth;
    threadGroupSpace->GetThreadGroupSpaceSize(thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);

    for (uint32_t i = 0; i < numArgs; i++)
    {
        // get the min kernel payload offset
        if ((halKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE) && IsKernelArg(tempArgs[i]))
        {
            if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion < 3)) 
            {
                if (minKernelPlayloadOffset == 0 || minKernelPlayloadOffset > tempArgs[i].unitOffsetInPayload)
                {
                    minKernelPlayloadOffset = tempArgs[i].unitOffsetInPayload;
                }
            }
            else
            {
                if ((minKernelPlayloadOffset == 0 || minKernelPlayloadOffset > tempArgs[i].unitOffsetInPayload) && (tempArgs[i].unitKind != ARG_KIND_IMPLICIT_LOCALID))
                {
                    minKernelPlayloadOffset = tempArgs[i].unitOffsetInPayload;
                }
            }
        }
    }

    for (uint32_t i = 0; i < numArgs; i++)
    {
        halKernelParam->argParams[i].unitCount = tempArgs[i].unitCount;
        halKernelParam->argParams[i].kind = (CM_HAL_KERNEL_ARG_KIND)(tempArgs[i].unitKind);
        halKernelParam->argParams[i].unitSize = tempArgs[i].unitSize;
        halKernelParam->argParams[i].payloadOffset = tempArgs[i].unitOffsetInPayload;
        halKernelParam->argParams[i].perThread = false;
        halKernelParam->argParams[i].nCustomValue = tempArgs[i].nCustomValue;
        halKernelParam->argParams[i].aliasIndex = tempArgs[i].aliasIndex;
        halKernelParam->argParams[i].aliasCreated = tempArgs[i].aliasCreated;
        halKernelParam->argParams[i].isNull = tempArgs[i].isNull;

        if (tempArgs[i].unitKind == CM_ARGUMENT_IMPLICT_LOCALSIZE) {
            CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelImplicitArgDataGroup(halKernelParam->argParams[i].firstValue, 3));
            *(uint32_t *)halKernelParam->argParams[i].firstValue = thrdSpaceWidth;
            *(uint32_t *)(halKernelParam->argParams[i].firstValue + 4) = thrdSpaceHeight;
            *(uint32_t *)(halKernelParam->argParams[i].firstValue + 8) = thrdSpaceDepth;
        }
        else if (tempArgs[i].unitKind == CM_ARGUMENT_IMPLICT_GROUPSIZE) {
            CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelImplicitArgDataGroup(halKernelParam->argParams[i].firstValue, 3));
            *(uint32_t *)halKernelParam->argParams[i].firstValue = grpSpaceWidth;
            *(uint32_t *)(halKernelParam->argParams[i].firstValue + 4) = grpSpaceHeight;
            *(uint32_t *)(halKernelParam->argParams[i].firstValue + 8) = grpSpaceDepth;
        }
        else if (tempArgs[i].unitKind == ARG_KIND_IMPLICIT_LOCALID) {
            CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelImplicitArgDataGroup(halKernelParam->argParams[i].firstValue, 3));
            halKernelParam->localIdIndex = i;
        }
        else
            CreateThreadArgData(&halKernelParam->argParams[i], i, nullptr, tempArgs);

        if (halKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE)
        {
            if (IsKernelArg(halKernelParam->argParams[i]))
            {
                // Kernel arg : calculate curbe size & adjust payloadoffset
                if (tempArgs[i].unitKind != ARG_KIND_IMPLICIT_LOCALID)
                {
                    halKernelParam->argParams[i].payloadOffset -= minKernelPlayloadOffset;
                }
                else
                {
                    // ARG_KIND_IMPLICIT_LOCALID is only for visa3.3+, need to adjust payloadOffset of local id for visa3.3+ later.
                    adjustLocalIdPayloadOffset = true;
                }

                if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion < 3)) {
                    if ((halKernelParam->argParams[i].payloadOffset + halKernelParam->argParams[i].unitSize > kernelCurbeSize))
                    {  // The largest one
                        kernelCurbeSize = halKernelParam->argParams[i].payloadOffset + halKernelParam->argParams[i].unitSize;
                    }
                }
                else
                {
                    if ((halKernelParam->argParams[i].payloadOffset + halKernelParam->argParams[i].unitSize > kernelCurbeSize) && (tempArgs[i].unitKind != ARG_KIND_IMPLICIT_LOCALID))
                    {  // The largest one
                        kernelCurbeSize = halKernelParam->argParams[i].payloadOffset + halKernelParam->argParams[i].unitSize;
                    }
                }
            }
        }
    }

    if ( m_stateBufferBounded != CM_STATE_BUFFER_NONE )
    {
        PCM_CONTEXT_DATA cmData = ( PCM_CONTEXT_DATA )m_device->GetAccelData();
        PCM_HAL_STATE state = cmData->cmHalState;
        kernelCurbeSize = state->pfnGetStateBufferSizeForKernel( state, this );
        halKernelParam->stateBufferType = state->pfnGetStateBufferTypeForKernel( state, this );
    }

    if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion < 3))
    {
        // GPGPU walker - implicit args
        for (uint32_t i = numArgs; i < numArgs + CM_GPUWALKER_IMPLICIT_ARG_NUM; i++)
        {
            halKernelParam->argParams[i].unitCount = 1;
            halKernelParam->argParams[i].kind = CM_ARGUMENT_GENERAL;
            halKernelParam->argParams[i].unitSize = 4;
            halKernelParam->argParams[i].payloadOffset = MOS_ALIGN_CEIL(kernelCurbeSize, 4) + (i - numArgs) * sizeof(uint32_t);
            halKernelParam->argParams[i].perThread = false;
        }

        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup(halKernelParam->argParams[numArgs + 0].firstValue, thrdSpaceWidth));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup(halKernelParam->argParams[numArgs + 1].firstValue, thrdSpaceHeight));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup(halKernelParam->argParams[numArgs + 2].firstValue, grpSpaceWidth));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup(halKernelParam->argParams[numArgs + 3].firstValue, grpSpaceHeight));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup(halKernelParam->argParams[numArgs + 4].firstValue, thrdSpaceWidth));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup(halKernelParam->argParams[numArgs + 5].firstValue, thrdSpaceHeight));
        halKernelParam->localIdIndex = halKernelParam->numArgs - 2;
    }
    halKernelParam->gpgpuWalkerParams.gpgpuEnabled = true;
    halKernelParam->gpgpuWalkerParams.groupWidth = grpSpaceWidth;
    halKernelParam->gpgpuWalkerParams.groupHeight = grpSpaceHeight;
    halKernelParam->gpgpuWalkerParams.groupDepth = grpSpaceDepth;
    halKernelParam->gpgpuWalkerParams.threadHeight = thrdSpaceHeight;
    halKernelParam->gpgpuWalkerParams.threadWidth = thrdSpaceWidth;
    halKernelParam->gpgpuWalkerParams.threadDepth = thrdSpaceDepth;
    //Get SLM size
    halKernelParam->slmSize = GetSLMSize();

    //Get spill area to adjust scratch space
    halKernelParam->spillSize = GetSpillMemUsed();

    //Set Barrier mode
    halKernelParam->barrierMode = m_barrierMode;
    halKernelParam->numberThreadsInGroup = thrdSpaceWidth * thrdSpaceHeight * thrdSpaceDepth;
    if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion < 3))
        kernelCurbeSize = MOS_ALIGN_CEIL(kernelCurbeSize, 4) + CM_GPUWALKER_IMPLICIT_ARG_NUM * sizeof(uint32_t);
    else
        kernelCurbeSize = MOS_ALIGN_CEIL(kernelCurbeSize, 4);
    if ((kernelCurbeSize % 32) == 4) //The per-thread data occupy 2 GRF.
    {
        halKernelParam->curbeSizePerThread = 64;
    }
    else
    {
        halKernelParam->curbeSizePerThread = 32;
    }
    if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion < 3)) {
        halKernelParam->totalCurbeSize = MOS_ALIGN_CEIL(kernelCurbeSize, 32) - halKernelParam->curbeSizePerThread + halKernelParam->curbeSizePerThread *
            thrdSpaceWidth * thrdSpaceHeight;
        //Since the CURBE is 32 bytes alignment, for GPGPU walker without the user specified thread argument, implicit per-thread id arguments will occupy at most 32 bytes
        halKernelParam->crossThreadConstDataLen = MOS_ALIGN_CEIL(kernelCurbeSize, 32) - halKernelParam->curbeSizePerThread;
    }
    else {
        halKernelParam->totalCurbeSize = MOS_ALIGN_CEIL(kernelCurbeSize, 32) + halKernelParam->curbeSizePerThread *
            thrdSpaceWidth * thrdSpaceHeight * thrdSpaceDepth;
        //Since the CURBE is 32 bytes alignment, for GPGPU walker without the user specified thread argument, implicit per-thread id arguments will occupy at most 32 bytes
        halKernelParam->crossThreadConstDataLen = MOS_ALIGN_CEIL(kernelCurbeSize, 32);
    }
    halKernelParam->payloadSize = 0; // no thread arg allowed

    // adjust payloadOffset of local id for visa3.3+
    if (adjustLocalIdPayloadOffset)
    {
        halKernelParam->argParams[halKernelParam->localIdIndex].payloadOffset = halKernelParam->crossThreadConstDataLen;
    }

    m_sizeInCurbe = GetAlignedCurbeSize(halKernelParam->totalCurbeSize);

    CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelIndirectData(&halKernelParam->indirectDataParam));

    if (m_samplerBtiCount != 0)
    {
        CmSafeMemCopy((void*)halKernelParam->samplerBTIParam.samplerInfo, (void*)m_samplerBtiEntry, sizeof(m_samplerBtiEntry));
        halKernelParam->samplerBTIParam.samplerCount = m_samplerBtiCount;

        CmSafeMemSet(m_samplerBtiEntry, 0, sizeof(m_samplerBtiEntry));
        m_samplerBtiCount = 0;
    }

    CalculateKernelSurfacesNum(surfNum, halKernelParam->numSurfaces);

    UpdateKernelDataGlobalSurfaceInfo(halKernelParam);

    //Destroy Temp Args
    for (uint32_t j = 0; j < numArgs; j++)
    {
        if (tempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
        {
            MosSafeDeleteArray(tempArgs[j].value);
        }
    }
    MosSafeDeleteArray(tempArgs);

    CM_CHK_CMSTATUS_GOTOFINISH(UpdateSamplerHeap(kernelData));
finish:
    if (hr != CM_SUCCESS)
    {
        //Clean allocated memory : need to count the implicit args
        if ((m_program->m_cisaMajorVersion == 3) && (m_program->m_cisaMinorVersion < 3)) {

            for (uint32_t i = 0; i < numArgs + CM_GPUWALKER_IMPLICIT_ARG_NUM; i++)
            {
                if (halKernelParam)
                {
                    if (halKernelParam->argParams[i].firstValue)
                    {
                        MosSafeDeleteArray(halKernelParam->argParams[i].firstValue);
                    }
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < numArgs; i++)
            {
                if (halKernelParam)
                {
                    if (halKernelParam->argParams[i].firstValue)
                    {
                        MosSafeDeleteArray(halKernelParam->argParams[i].firstValue);
                    }
                }
            }
        }
        //Destroy Temp Args in failing case
        if (tempArgs)
        {
            for (uint32_t j = 0; j < numArgs; j++)
            {
                if (tempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
                {
                    MosSafeDeleteArray(tempArgs[j].value);
                }
            }
            MosSafeDeleteArray(tempArgs);
        }
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelRT::IsBatchBufferReusable( CmThreadSpaceRT * taskThreadSpace )
{
    bool reusable = true;
    //Update m_id if the batch buffer is not reusable.
    if (m_dirty & CM_KERNEL_DATA_THREAD_ARG_DIRTY)
    {
        reusable = false; // if thread arg dirty
    }
    else if ((m_dirty & CM_KERNEL_DATA_KERNEL_ARG_DIRTY) && (m_curbeEnabled == false))
    {
        reusable = false; // if kernel arg dirty and curbe disabled
    }
    else if (m_dirty & CM_KERNEL_DATA_THREAD_COUNT_DIRTY)
    {
        reusable = false; // if thread count dirty
    }
    else if (m_threadSpace)
    {
       if (m_threadSpace->GetDirtyStatus() == CM_THREAD_SPACE_DATA_DIRTY)
       {
          reusable = false; // if per kernel thread space exists and it is completely dirty
       }
    }
    else if (taskThreadSpace)
    {
       if (taskThreadSpace->GetDirtyStatus() == CM_THREAD_SPACE_DATA_DIRTY)
       {
          reusable = false; // if per task thread space change and it is completely dirty
       }
    }
    return reusable;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Checks to see if kernel prologue has changed
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelRT::IsPrologueDirty( void )
{
    bool prologueDirty = false;

    if( m_threadCount != m_lastThreadCount )
    {
        if( m_lastThreadCount )
        {
            if( m_threadCount == 1 || m_lastThreadCount == 1 )
            {
                prologueDirty = true;
            }
        }
        m_lastThreadCount = m_threadCount;
    }

    if( m_adjustScoreboardY != m_lastAdjustScoreboardY )
    {
        if( m_lastAdjustScoreboardY )
        {
            prologueDirty = true;
        }
        m_lastAdjustScoreboardY = m_adjustScoreboardY;
    }

    return prologueDirty;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelDataInternal(
    CmKernelData* & kernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadSpaceRT* threadSpace )    // in
{
    PCM_HAL_KERNEL_PARAM  halKernelParam       = nullptr;
    int32_t               hr                    = CM_SUCCESS;
    uint32_t              movInstNum            = 0;
    uint32_t              kernelCurbeSize          = 0;
    uint32_t              numArgs               = 0;
    uint32_t              bottomRange         = 1024;
    uint32_t              upRange             = 0;
    uint32_t              unitSize              = 0;
    bool                  hasThreadArg          = false;
    CmThreadSpaceRT         *cmThreadSpace       = nullptr;
    bool                  isKernelThreadSpace   = false;
    CM_ARG                *tempArgs            = nullptr;
    uint32_t              argSize               = 0;
    uint32_t              surfNum               = 0; //Pass needed BT entry numbers to HAL CM
    CmKernelRT             *cmKernel             = nullptr;

    if( threadSpace == nullptr && m_threadSpace!= nullptr)
    {
        cmThreadSpace = m_threadSpace;
        isKernelThreadSpace = true;
    }
    else
    {
        cmThreadSpace = const_cast<CmThreadSpaceRT*>(threadSpace);
    }

    CM_CHK_CMSTATUS_GOTOFINISH(CmKernelData::Create( this, kernelData ));
    halKernelParam = kernelData->GetHalCmKernelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(halKernelParam);

    //Get Num of args with surface array
    CM_CHK_CMSTATUS_GOTOFINISH(GetArgCountPlusSurfArray(argSize, numArgs));

    if( numArgs > 0)
    {
        //Create Temp args
        CM_CHK_CMSTATUS_GOTOFINISH(CreateTempArgs(numArgs, tempArgs));
        //Create move instructions
        CM_CHK_CMSTATUS_GOTOFINISH(CreateMovInstructions(movInstNum,   halKernelParam->movInsData, tempArgs, numArgs));
    }

    CM_CHK_CMSTATUS_GOTOFINISH(CalcKernelDataSize(movInstNum, numArgs, argSize, kernelDataSize));
    CM_CHK_CMSTATUS_GOTOFINISH(kernelData->SetKernelDataSize(kernelDataSize));

    if(!IsBatchBufferReusable(const_cast<CmThreadSpaceRT *>(threadSpace)))
    {
        m_id ++;
    }

    if( IsPrologueDirty( ) )
    {
        // can't re-use kernel binary in GSH
        // just update upper 16 bits
        uint64_t tempID = m_id;
        tempID >>= 48;
        tempID++;
        tempID <<= 48;
        // get rid of old values in upper 16 bits
        m_id <<= 16;
        m_id >>= 16;
        m_id |= tempID;
    }

    halKernelParam->clonedKernelParam.isClonedKernel = m_isClonedKernel;
    halKernelParam->clonedKernelParam.kernelID       = m_cloneKernelID;
    halKernelParam->clonedKernelParam.hasClones      = m_hasClones;
    halKernelParam->kernelId           = m_id; // kernel id , high 32-bit is kernel id, low 32-bit is kernel data id for batch buffer reuse
    halKernelParam->numArgs             = numArgs;
    halKernelParam->numThreads          = m_threadCount;
    halKernelParam->kernelBinarySize    = m_binarySize + movInstNum * CM_MOVE_INSTRUCTION_SIZE;
    halKernelParam->kernelDataSize      = kernelDataSize;
    halKernelParam->movInsDataSize      = movInstNum * CM_MOVE_INSTRUCTION_SIZE;

    halKernelParam->cmFlags             = m_curbeEnabled ? CM_FLAG_CURBE_ENABLED : 0;
    halKernelParam->cmFlags            |= m_nonstallingScoreboardEnabled ? CM_FLAG_NONSTALLING_SCOREBOARD_ENABLED : 0;
    halKernelParam->kernelDebugEnabled  = m_blhwDebugEnable;

    halKernelParam->kernelBinary        = (uint8_t*)m_binary;

    CM_CHK_CMSTATUS_GOTOFINISH( kernelData->GetCmKernel( cmKernel ) );
    if ( cmKernel == nullptr )
    {
        return CM_NULL_POINTER;
    }
    MOS_SecureStrcpy( halKernelParam->kernelName, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, cmKernel->GetName() );

    if ( cmThreadSpace )
    {// either from per kernel thread space or per task thread space
        CM_CHK_CMSTATUS_GOTOFINISH(SortThreadSpace(cmThreadSpace)); // must be called before CreateThreadArgData
    }

    for(uint32_t i =0 ; i< numArgs; i++)
    {
        halKernelParam->argParams[i].unitCount        = tempArgs[ i ].unitCount;
        halKernelParam->argParams[i].kind              = (CM_HAL_KERNEL_ARG_KIND)(tempArgs[ i ].unitKind);
        halKernelParam->argParams[i].unitSize         = tempArgs[ i ].unitSize;
        halKernelParam->argParams[i].payloadOffset    = tempArgs[ i ].unitOffsetInPayload;
        halKernelParam->argParams[i].perThread        = (tempArgs[ i ].unitCount > 1) ? true :false;
        halKernelParam->argParams[i].nCustomValue      = tempArgs[ i ].nCustomValue;
        halKernelParam->argParams[i].aliasIndex       = tempArgs[ i ].aliasIndex;
        halKernelParam->argParams[i].aliasCreated     = tempArgs[ i ].aliasCreated;
        halKernelParam->argParams[i].isNull           = tempArgs[ i ].isNull;

        CreateThreadArgData(&halKernelParam->argParams[i], i, cmThreadSpace, tempArgs);

        if(CHECK_SURFACE_TYPE ( halKernelParam->argParams[i].kind,
            ARG_KIND_SURFACE_VME,
            ARG_KIND_SURFACE_SAMPLER,
            ARG_KIND_SURFACE2DUP_SAMPLER))
        {
            unitSize = CM_ARGUMENT_SURFACE_SIZE;
        }
        else
        {
            unitSize = halKernelParam->argParams[i].unitSize;
        }

        if (halKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE)
        {
            if(IsKernelArg(halKernelParam->argParams[i]))
            {
                // Kernel arg : calculate curbe size & adjust payloadoffset
                // Note: Here the payloadOffset may be different from original value
                uint32_t offset = halKernelParam->argParams[i].payloadOffset - CM_PAYLOAD_OFFSET;
                if (offset >= kernelCurbeSize)
                {
                    kernelCurbeSize = offset + unitSize;
                }
                halKernelParam->argParams[i].payloadOffset -= CM_PAYLOAD_OFFSET;
            }
        }

        if(!IsKernelArg(halKernelParam->argParams[i]))
        {   //Thread arg : Calculate payload size & adjust payloadoffset
            hasThreadArg  = true;
            halKernelParam->argParams[i].payloadOffset -= CM_PAYLOAD_OFFSET;

            if(halKernelParam->argParams[i].payloadOffset < bottomRange)
            {
               bottomRange = halKernelParam->argParams[i].payloadOffset;
            }
            if(halKernelParam->argParams[i].payloadOffset >=  upRange)
            {
               upRange = halKernelParam->argParams[i].payloadOffset + unitSize;
            }
        }
    }

    if ( m_stateBufferBounded != CM_STATE_BUFFER_NONE )
    {
        PCM_CONTEXT_DATA cmData = ( PCM_CONTEXT_DATA )m_device->GetAccelData();
        PCM_HAL_STATE state = cmData->cmHalState;
        kernelCurbeSize = state->pfnGetStateBufferSizeForKernel( state, this );
        halKernelParam->stateBufferType = state->pfnGetStateBufferTypeForKernel( state, this );
    }

    halKernelParam->payloadSize         = hasThreadArg ? MOS_ALIGN_CEIL(upRange -  bottomRange, 4): 0;
    halKernelParam->totalCurbeSize      = MOS_ALIGN_CEIL(kernelCurbeSize, 32);
    halKernelParam->curbeSizePerThread  = halKernelParam->totalCurbeSize;

    halKernelParam->perThreadArgExisted = hasThreadArg;

    m_sizeInCurbe = GetAlignedCurbeSize( kernelCurbeSize );

    if ( halKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE )
    {
        for(uint32_t i=0; i< numArgs; i++)
        {
            if(!IsKernelArg(halKernelParam->argParams[i]))
            {  // thread arg: need to minus curbe size
                halKernelParam->argParams[i].payloadOffset -= halKernelParam->curbeSizePerThread;
            }
        }
    }

    //Create indirect data
    CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelIndirectData(&halKernelParam->indirectDataParam));

    if ( m_samplerBtiCount != 0 )
    {
        CmSafeMemCopy( ( void* )halKernelParam->samplerBTIParam.samplerInfo, ( void* )m_samplerBtiEntry, sizeof( m_samplerBtiEntry ) );
        halKernelParam->samplerBTIParam.samplerCount = m_samplerBtiCount;

        CmSafeMemSet(m_samplerBtiEntry, 0, sizeof(m_samplerBtiEntry));
        m_samplerBtiCount = 0;
    }

    CalculateKernelSurfacesNum(surfNum, halKernelParam->numSurfaces);

    //Create thread space param: only avaliable if per kernel ts exists
    if(m_threadSpace)
    {
        CM_CHK_CMSTATUS_GOTOFINISH(CreateThreadSpaceParam(&halKernelParam->kernelThreadSpaceParam, m_threadSpace));
    }

    //Get SLM size
    halKernelParam->slmSize = GetSLMSize();

    //Get Spill mem used
    halKernelParam->spillSize = GetSpillMemUsed();

    //Set Barrier mode
    halKernelParam->barrierMode = m_barrierMode;

    CM_CHK_CMSTATUS_GOTOFINISH(UpdateKernelDataGlobalSurfaceInfo( halKernelParam ));

    //Destroy Temp Args
    for (uint32_t j = 0; j < numArgs; j++)
    {
        if (tempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
        {
            MosSafeDeleteArray(tempArgs[j].value);
        }
    }
    MosSafeDeleteArray( tempArgs );

    CM_CHK_CMSTATUS_GOTOFINISH(UpdateSamplerHeap(kernelData));
finish:
    if(hr != CM_SUCCESS)
    {
         if(halKernelParam)
         {
             //Clean allocated memory
             for(uint32_t i =0 ; i< numArgs; i++)
             {
                if( halKernelParam->argParams[i].firstValue )
                {
                    MosSafeDeleteArray(halKernelParam->argParams[i].firstValue);
                }
             }
         }

         //Destroy Temp Args
         if (tempArgs)
         {
             for (uint32_t j = 0; j < numArgs; j++)
             {
                 if (tempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
                 {
                     MosSafeDeleteArray(tempArgs[j].value);
                 }
             }
             MosSafeDeleteArray(tempArgs);
         }
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update kernel data's kernel arg, thread arg, thread count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateKernelData(
    CmKernelData*   kernelData,  // in
    const CmThreadSpaceRT* threadSpace)
{
    int32_t               hr                      = CM_SUCCESS;
    PCM_HAL_KERNEL_PARAM  halKernelParam         = nullptr;
    bool                  bbResuable             = true;
    CmThreadSpaceRT         *cmThreadSpace         = nullptr;
    bool                  isKernelThreadSpace     = false;
    uint32_t              argIndexStep            = 0;
    uint32_t              argIndex                = 0;
    uint32_t              surfNum                 = 0; //Update Number of surface used by kernel

    if( threadSpace == nullptr && m_threadSpace!= nullptr)
    {
        cmThreadSpace = m_threadSpace;
        isKernelThreadSpace = true;
    }
    else
    {
        cmThreadSpace = const_cast<CmThreadSpaceRT*>(threadSpace);
    }

    CM_CHK_NULL_GOTOFINISH_CMERROR(kernelData);
    CM_ASSERT(kernelData->IsInUse() == false);

    halKernelParam = kernelData->GetHalCmKernelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(halKernelParam);

    if(!IsBatchBufferReusable(const_cast<CmThreadSpaceRT *>(threadSpace)))
    {
        m_id ++;
        halKernelParam->kernelId = m_id;
    }

    //Update arguments
    for(uint32_t orgArgIndex =0 ; orgArgIndex< m_argCount; orgArgIndex++)
    {
        argIndexStep = 1;

        if ( CHECK_SURFACE_TYPE( m_args[ orgArgIndex ].unitKind,
                        ARG_KIND_SURFACE,
                        ARG_KIND_SURFACE_1D,
                        ARG_KIND_SURFACE_2D,
                        ARG_KIND_SURFACE_2D_UP,
                        ARG_KIND_SURFACE_SAMPLER,
                        ARG_KIND_SURFACE2DUP_SAMPLER,
                        ARG_KIND_SURFACE_3D,
                        ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                        ARG_KIND_SURFACE_SAMPLER8X8_VA,
                        ARG_KIND_SURFACE_2D_SCOREBOARD,
                        ARG_KIND_STATE_BUFFER ) )
        {
            argIndexStep = m_args[orgArgIndex].unitSize/sizeof(int); // Surface array exists
        }
        else if (CHECK_SURFACE_TYPE(m_args[orgArgIndex].unitKind,  ARG_KIND_SURFACE_VME))
        {
            argIndexStep = m_args[orgArgIndex].unitVmeArraySize;
        }

        if(m_args[ orgArgIndex ].isDirty)
        {
            if(m_args[ orgArgIndex ].unitCount > 1)
            { // thread arg is dirty
                bbResuable          = false;
            }

            if ( CHECK_SURFACE_TYPE( m_args[ orgArgIndex ].unitKind,
                        ARG_KIND_SURFACE,
                        ARG_KIND_SURFACE_1D,
                        ARG_KIND_SURFACE_2D,
                        ARG_KIND_SURFACE_2D_UP,
                        ARG_KIND_SURFACE_SAMPLER,
                        ARG_KIND_SURFACE2DUP_SAMPLER,
                        ARG_KIND_SURFACE_3D,
                        ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                        ARG_KIND_SURFACE_SAMPLER8X8_VA,
                        ARG_KIND_SURFACE_2D_SCOREBOARD,
                        ARG_KIND_STATE_BUFFER ) )
            {  // for surface args

                uint32_t numSurfaces = m_args[orgArgIndex].unitSize/sizeof(int); // Surface array
                if(m_args[ orgArgIndex ].unitCount ==  1) // kernel arg
                {
                    if (numSurfaces > 1)
                    {
                        for (uint32_t kk = 0; kk < numSurfaces; kk++)
                        {
                            CM_ASSERT(halKernelParam->argParams[argIndex + kk].firstValue != nullptr);
                            CmSafeMemCopy(halKernelParam->argParams[argIndex + kk].firstValue,
                                m_args[orgArgIndex].value + kk*sizeof(uint32_t), sizeof(uint32_t));
                            halKernelParam->argParams[argIndex + kk].aliasIndex = m_args[orgArgIndex].aliasIndex;
                            halKernelParam->argParams[argIndex + kk].aliasCreated = m_args[orgArgIndex].aliasCreated;
                            halKernelParam->argParams[argIndex + kk].isNull = m_args[orgArgIndex].isNull;

                            if (!m_args[orgArgIndex].surfIndex[kk])
                            {
                                //if surfIndex is 0, set kind to be CM_ARGUMENT_SURFACE2D
                                //This is for special usage if there is empty element in surface array.
                                halKernelParam->argParams[argIndex + kk].kind = CM_ARGUMENT_SURFACE2D;
                                continue;
                            }

                            halKernelParam->argParams[argIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_args[orgArgIndex].surfArrayArg[kk].argKindForArray;
                            halKernelParam->argParams[argIndex + kk].nCustomValue = m_args[orgArgIndex].surfArrayArg[kk].addressModeForArray;
                        }
                    }
                    else
                    {
                        CM_ASSERT(halKernelParam->argParams[argIndex].firstValue != nullptr);
                        CmSafeMemCopy(halKernelParam->argParams[argIndex].firstValue,
                                m_args[ orgArgIndex ].value, sizeof(uint32_t));
                        halKernelParam->argParams[argIndex].kind = (CM_HAL_KERNEL_ARG_KIND)m_args[ orgArgIndex ].unitKind;
                        halKernelParam->argParams[argIndex].aliasIndex   = m_args[orgArgIndex].aliasIndex;
                        halKernelParam->argParams[argIndex].aliasCreated = m_args[orgArgIndex].aliasCreated;
                        halKernelParam->argParams[argIndex].isNull = m_args[orgArgIndex].isNull;
                    }

                 }
                 else // thread arg
                 {
                    uint32_t numSurfaces = m_args[orgArgIndex].unitSize/sizeof(int); // Surface array
                    uint32_t *surfaces = (uint32_t *)MOS_NewArray(uint8_t, (sizeof(uint32_t) * m_args[orgArgIndex].unitCount));
                    CM_CHK_NULL_GOTOFINISH(surfaces, CM_OUT_OF_HOST_MEMORY);
                    for (uint32_t kk=0;  kk< numSurfaces ; kk++)
                    {
                        for (uint32_t s = 0; s < m_args[orgArgIndex].unitCount; s++)
                        {
                            surfaces[s] = *(uint32_t *)((uint32_t *)m_args[orgArgIndex].value + kk + numSurfaces * s);
                        }
                        CmSafeMemCopy(halKernelParam->argParams[argIndex + kk].firstValue,
                            surfaces, sizeof(uint32_t) * m_args[orgArgIndex].unitCount);

                        halKernelParam->argParams[argIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_args[ orgArgIndex ].unitKind;

                        halKernelParam->argParams[argIndex + kk].aliasIndex = m_args[orgArgIndex].aliasIndex;
                        halKernelParam->argParams[argIndex + kk].aliasCreated = m_args[orgArgIndex].aliasCreated;
                        halKernelParam->argParams[argIndex + kk].isNull = m_args[orgArgIndex].isNull;

                    }
                    MosSafeDeleteArray(surfaces);
                 }

            }
            else if (CHECK_SURFACE_TYPE(m_args[orgArgIndex].unitKind, ARG_KIND_SURFACE_VME))
            {
                uint32_t numSurfaces = m_args[orgArgIndex].unitVmeArraySize;
                if (m_args[orgArgIndex].unitCount == 1) // kernel arg
                {
                    uint32_t vmeSurfOffset = 0;
                    for (uint32_t kk = 0; kk< numSurfaces; kk++)
                    {
                        uint16_t vmeSize = (uint16_t)getVmeArgValueSize((PCM_HAL_VME_ARG_VALUE)(m_args[orgArgIndex].value + vmeSurfOffset));

                        // reallocate the firstValue for VME surface every time
                        // since the number of surfaces may vary
                        MosSafeDeleteArray(halKernelParam->argParams[argIndex + kk].firstValue);
                        halKernelParam->argParams[argIndex + kk].firstValue = MOS_NewArray(uint8_t, vmeSize);
                        CM_ASSERT(halKernelParam->argParams[argIndex + kk].firstValue != nullptr);
                        CmSafeMemCopy(halKernelParam->argParams[argIndex + kk].firstValue,
                            m_args[orgArgIndex].value + vmeSurfOffset, vmeSize);

                        halKernelParam->argParams[argIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_args[orgArgIndex].unitKind;

                        halKernelParam->argParams[argIndex + kk].aliasIndex = m_args[orgArgIndex].aliasIndex;
                        halKernelParam->argParams[argIndex + kk].aliasCreated = m_args[orgArgIndex].aliasCreated;
                        halKernelParam->argParams[argIndex + kk].isNull = m_args[orgArgIndex].isNull;
                        halKernelParam->argParams[argIndex + kk].unitSize = vmeSize;
                        vmeSurfOffset += vmeSize;
                    }
                }
            }
            else
            {
                CM_CHK_CMSTATUS_GOTOFINISH(CreateThreadArgData(&halKernelParam->argParams[argIndex ], orgArgIndex, cmThreadSpace, m_args));
            }
        }
        argIndex += argIndexStep;
    }

    //Update Thread space param
    if(m_threadSpace && m_threadSpace->GetDirtyStatus())
    {

        CM_CHK_CMSTATUS_GOTOFINISH(SortThreadSpace(m_threadSpace));

        uint32_t threadSpaceWidth = 0, threadSpaceHeight = 0;
        PCM_HAL_KERNEL_THREADSPACE_PARAM  cmKernelThreadSpaceParam = &halKernelParam->kernelThreadSpaceParam;
        m_threadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);

        cmKernelThreadSpaceParam->threadSpaceWidth  = (uint16_t)threadSpaceWidth;
        cmKernelThreadSpaceParam->threadSpaceHeight = (uint16_t)threadSpaceHeight;
        m_threadSpace->GetDependencyPatternType(cmKernelThreadSpaceParam->patternType);
        m_threadSpace->GetWalkingPattern(cmKernelThreadSpaceParam->walkingPattern);
        m_threadSpace->GetColorCountMinusOne(cmKernelThreadSpaceParam->colorCountMinusOne);

        CM_HAL_DEPENDENCY*     dependency = nullptr;
        m_threadSpace->GetDependency( dependency);

        if(dependency != nullptr)
        {
            CmSafeMemCopy(&cmKernelThreadSpaceParam->dependencyInfo, dependency, sizeof(CM_HAL_DEPENDENCY));
        }

        if( m_threadSpace->CheckWalkingParametersSet() )
        {
            CM_CHK_CMSTATUS_GOTOFINISH(m_threadSpace->GetWalkingParameters(cmKernelThreadSpaceParam->walkingParams));
        }

        if( m_threadSpace->CheckDependencyVectorsSet() )
        {
            CM_CHK_CMSTATUS_GOTOFINISH(m_threadSpace->GetDependencyVectors(cmKernelThreadSpaceParam->dependencyVectors));
        }

        if(m_threadSpace->IsThreadAssociated())
        {// media object only
            uint32_t *boardOrder = nullptr;
            m_threadSpace->GetBoardOrder(boardOrder);
            CM_CHK_NULL_GOTOFINISH_CMERROR(boardOrder);

            CM_THREAD_SPACE_UNIT *threadSpaceUnit = nullptr;
            m_threadSpace->GetThreadSpaceUnit(threadSpaceUnit);
            CM_CHK_NULL_GOTOFINISH_CMERROR(threadSpaceUnit);

            cmKernelThreadSpaceParam->reuseBBUpdateMask = 0;
            for(uint32_t i=0; i< threadSpaceWidth * threadSpaceHeight ; i++)
            {
                cmKernelThreadSpaceParam->threadCoordinates[i].x = threadSpaceUnit[boardOrder[i]].scoreboardCoordinates.x;
                cmKernelThreadSpaceParam->threadCoordinates[i].y = threadSpaceUnit[boardOrder[i]].scoreboardCoordinates.y;
                cmKernelThreadSpaceParam->threadCoordinates[i].mask = threadSpaceUnit[boardOrder[i]].dependencyMask;
                cmKernelThreadSpaceParam->threadCoordinates[i].resetMask = threadSpaceUnit[boardOrder[i]].reset;
                cmKernelThreadSpaceParam->threadCoordinates[i].color = threadSpaceUnit[boardOrder[i]].scoreboardColor;
                cmKernelThreadSpaceParam->threadCoordinates[i].sliceSelect = threadSpaceUnit[boardOrder[i]].sliceDestinationSelect;
                cmKernelThreadSpaceParam->threadCoordinates[i].subSliceSelect = threadSpaceUnit[boardOrder[i]].subSliceDestinationSelect;
                cmKernelThreadSpaceParam->reuseBBUpdateMask |= threadSpaceUnit[boardOrder[i]].reset;
            }

            if( cmKernelThreadSpaceParam->patternType == CM_WAVEFRONT26Z )
            {
                CM_HAL_WAVEFRONT26Z_DISPATCH_INFO dispatchInfo;
                m_threadSpace->GetWavefront26ZDispatchInfo(dispatchInfo);

                if (cmKernelThreadSpaceParam->dispatchInfo.numWaves >= dispatchInfo.numWaves)
                {
                    cmKernelThreadSpaceParam->dispatchInfo.numWaves = dispatchInfo.numWaves;
                    CmSafeMemCopy(cmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, dispatchInfo.numThreadsInWave, dispatchInfo.numWaves*sizeof(uint32_t));
                }
                else
                {
                    cmKernelThreadSpaceParam->dispatchInfo.numWaves = dispatchInfo.numWaves;
                    MosSafeDeleteArray(cmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave);
                    cmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave = MOS_NewArray(uint32_t, dispatchInfo.numWaves);
                    CM_CHK_NULL_GOTOFINISH(cmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, CM_OUT_OF_HOST_MEMORY);
                    CmSafeMemCopy(cmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, dispatchInfo.numThreadsInWave, dispatchInfo.numWaves*sizeof(uint32_t));
                }
            }
        }
    }

    // Update indirect data
    if( m_dirty & CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY)
    {
        halKernelParam->indirectDataParam.indirectDataSize = m_usKernelPayloadDataSize;
        halKernelParam->indirectDataParam.surfaceCount     = m_usKernelPayloadSurfaceCount;

        if(m_usKernelPayloadDataSize != 0)
        {
            if(m_dirty & CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY)
            { // size change, need to reallocate
                MosSafeDeleteArray(halKernelParam->indirectDataParam.indirectData);
                halKernelParam->indirectDataParam.indirectData = MOS_NewArray(uint8_t, m_usKernelPayloadDataSize);
                CM_CHK_NULL_GOTOFINISH(halKernelParam->indirectDataParam.indirectData, CM_OUT_OF_HOST_MEMORY);
            }
            CmSafeMemCopy(halKernelParam->indirectDataParam.indirectData, (void *)m_kernelPayloadData, m_usKernelPayloadDataSize);
        }

        if(m_usKernelPayloadSurfaceCount != 0)
        {
            if(m_dirty & CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY)
            { // size change, need to reallocate
                MosSafeDeleteArray(halKernelParam->indirectDataParam.surfaceInfo);
                halKernelParam->indirectDataParam.surfaceInfo = MOS_NewArray(CM_INDIRECT_SURFACE_INFO, m_usKernelPayloadSurfaceCount);
                CM_CHK_NULL_GOTOFINISH(halKernelParam->indirectDataParam.surfaceInfo, CM_OUT_OF_HOST_MEMORY);

            }
            CmSafeMemCopy((void*)halKernelParam->indirectDataParam.surfaceInfo, (void*)m_IndirectSurfaceInfoArray,
                             m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
            //clear m_IndirectSurfaceInfoArray every enqueue
            CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
            m_usKernelPayloadSurfaceCount = 0;
        }
    }

    if (m_dirty & cMKERNELDATASAMPLERBTIDIRTY)
    {
        if ( m_samplerBtiCount != 0 )
        {
            CmSafeMemCopy( ( void* )halKernelParam->samplerBTIParam.samplerInfo, ( void* )m_samplerBtiEntry, sizeof( m_samplerBtiEntry ) );
            halKernelParam->samplerBTIParam.samplerCount = m_samplerBtiCount;

            CmSafeMemSet(m_samplerBtiEntry, 0, sizeof(m_samplerBtiEntry));
            m_samplerBtiCount = 0;
        }
    }
    CM_CHK_CMSTATUS_GOTOFINISH(UpdateKernelDataGlobalSurfaceInfo( halKernelParam ));

    CM_CHK_CMSTATUS_GOTOFINISH(CalculateKernelSurfacesNum(surfNum, halKernelParam->numSurfaces));

    CM_CHK_CMSTATUS_GOTOFINISH(UpdateSamplerHeap(kernelData));

finish:
    if( hr != CM_SUCCESS)
    {
        if( halKernelParam )
        {
            MosSafeDeleteArray(halKernelParam->indirectDataParam.indirectData);
            MosSafeDeleteArray(halKernelParam->indirectDataParam.surfaceInfo);
        }
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update kernel data's kernel arg, thread arg, thread count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateKernelData(
    CmKernelData*   kernelData,  // in
    const CmThreadGroupSpace* threadGroupSpace )    // in
{
    int32_t               hr                      = CM_SUCCESS;
    PCM_HAL_KERNEL_PARAM  halKernelParam         = nullptr;
    uint32_t              argIndexStep            = 0;
    uint32_t              argIndex                = 0;
    uint32_t              surfNum                 = 0;
    auto getVersionAsInt = [](int major, int minor) { return major * 100 + minor; };

    CM_CHK_NULL_GOTOFINISH_CMERROR(kernelData);
    CM_ASSERT(kernelData->IsInUse() == false);

    halKernelParam = kernelData->GetHalCmKernelData();
    CM_CHK_NULL_GOTOFINISH_CMERROR(halKernelParam);

    CM_CHK_NULL_GOTOFINISH_CMERROR(threadGroupSpace);

    //Update arguments
    for(uint32_t orgArgIndex =0 ; orgArgIndex< m_argCount; orgArgIndex++)
    {
        argIndexStep = 1;

        if ( CHECK_SURFACE_TYPE( m_args[ orgArgIndex ].unitKind,
                        ARG_KIND_SURFACE,
                        ARG_KIND_SURFACE_1D,
                        ARG_KIND_SURFACE_2D,
                        ARG_KIND_SURFACE_2D_UP,
                        ARG_KIND_SURFACE_SAMPLER,
                        ARG_KIND_SURFACE2DUP_SAMPLER,
                        ARG_KIND_SURFACE_3D,
                        ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                        ARG_KIND_SURFACE_SAMPLER8X8_VA,
                        ARG_KIND_SURFACE_2D_SCOREBOARD,
                        ARG_KIND_STATE_BUFFER ) )
        {
            argIndexStep = m_args[orgArgIndex].unitSize/sizeof(int); // Surface array exists
        }
        else if (CHECK_SURFACE_TYPE(m_args[orgArgIndex].unitKind, ARG_KIND_SURFACE_VME))
        {
            argIndexStep = m_args[orgArgIndex].unitVmeArraySize;
        }

        if(m_args[ orgArgIndex ].isDirty)
        {
            if(m_args[ orgArgIndex ].unitCount > 1)
            { // thread arg is dirty
                CM_ASSERTMESSAGE("Error: Thread arg is not allowed in GPGPU walker.");
                hr = CM_FAILURE; // Thread arg is not allowed in GPGPU walker
                goto finish;
            }

            if ( CHECK_SURFACE_TYPE( m_args[ orgArgIndex ].unitKind,
                        ARG_KIND_SURFACE,
                        ARG_KIND_SURFACE_1D,
                        ARG_KIND_SURFACE_2D,
                        ARG_KIND_SURFACE_2D_UP,
                        ARG_KIND_SURFACE_SAMPLER,
                        ARG_KIND_SURFACE2DUP_SAMPLER,
                        ARG_KIND_SURFACE_3D,
                        ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                        ARG_KIND_SURFACE_SAMPLER8X8_VA,
                        ARG_KIND_SURFACE_2D_SCOREBOARD,
                        ARG_KIND_STATE_BUFFER ) )
            {  // for surface args
                uint32_t numSurfaces = m_args[orgArgIndex].unitSize/sizeof(int); // Surface array
                if(m_args[ orgArgIndex ].unitCount ==  1) // kernel arg
                {
                    if (numSurfaces > 1 )
                    {
                        for(uint32_t kk=0;  kk< numSurfaces ; kk++)
                        {
                            CM_ASSERT(halKernelParam->argParams[argIndex + kk].firstValue
                                      != nullptr);
                            CmSafeMemCopy(halKernelParam->argParams[argIndex + kk].firstValue,
                                          m_args[ orgArgIndex ].value + kk*sizeof(uint32_t),
                                          sizeof(uint32_t));
                            halKernelParam->argParams[argIndex + kk].aliasIndex
                                    = m_args[orgArgIndex].aliasIndex;
                            halKernelParam->argParams[argIndex + kk].aliasCreated
                                    = m_args[orgArgIndex].aliasCreated;
                            halKernelParam->argParams[argIndex + kk].isNull
                                    = m_args[orgArgIndex].isNull;

                            if (!m_args[orgArgIndex].surfIndex[kk])
                            {
                                //if surfIndex is 0, set kind to be CM_ARGUMENT_SURFACE2D
                                //This is for special usage if there is empty element in surface array.
                                halKernelParam->argParams[argIndex + kk].kind = CM_ARGUMENT_SURFACE2D;
                                continue;
                            }
                            halKernelParam->argParams[argIndex + kk].isNull = m_args[orgArgIndex].isNull;
                            halKernelParam->argParams[argIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_args[orgArgIndex].surfArrayArg[kk].argKindForArray;
                            halKernelParam->argParams[argIndex + kk].nCustomValue = m_args[orgArgIndex].surfArrayArg[kk].addressModeForArray;

                        }
                    }
                    else
                    {
                        CM_ASSERT(halKernelParam->argParams[argIndex].firstValue != nullptr);
                        halKernelParam->argParams[argIndex].kind
                                = (CM_HAL_KERNEL_ARG_KIND)m_args[orgArgIndex].unitKind;
                        halKernelParam->argParams[argIndex].aliasIndex
                                = m_args[orgArgIndex].aliasIndex;
                        halKernelParam->argParams[argIndex].aliasCreated
                                = m_args[orgArgIndex].aliasCreated;
                        halKernelParam->argParams[argIndex].isNull
                                = m_args[orgArgIndex].isNull;
                        if (halKernelParam->argParams[argIndex].isNull)
                        {
                            *(halKernelParam->argParams[argIndex].firstValue)
                                    = 0;
                        }
                        else
                        {
                            CmSafeMemCopy(
                                halKernelParam->argParams[argIndex].firstValue,
                                m_args[orgArgIndex].value, sizeof(uint32_t));
                        }
                    }
                }
            }
            else if (CHECK_SURFACE_TYPE(m_args[orgArgIndex].unitKind, ARG_KIND_SURFACE_VME))
            {
                uint32_t numSurfaces = m_args[orgArgIndex].unitVmeArraySize;
                if (m_args[orgArgIndex].unitCount == 1) // kernel arg
                {
                    uint32_t vmeSurfOffset = 0;
                    for (uint32_t kk = 0; kk< numSurfaces; kk++)
                    {
                        uint32_t vmeSize = getVmeArgValueSize((PCM_HAL_VME_ARG_VALUE)(m_args[orgArgIndex].value + vmeSurfOffset));

                        // reallocate the firstValue for VME surface every time
                        // since the number of surfaces may vary
                        MosSafeDeleteArray(halKernelParam->argParams[argIndex + kk].firstValue);
                        halKernelParam->argParams[argIndex + kk].firstValue = MOS_NewArray(uint8_t, vmeSize);
                        CM_ASSERT(halKernelParam->argParams[argIndex + kk].firstValue != nullptr);
                        CmSafeMemCopy(halKernelParam->argParams[argIndex + kk].firstValue,
                            m_args[orgArgIndex].value + vmeSurfOffset, vmeSize);

                        halKernelParam->argParams[argIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_args[orgArgIndex].unitKind;

                        halKernelParam->argParams[argIndex + kk].aliasIndex = m_args[orgArgIndex].aliasIndex;
                        halKernelParam->argParams[argIndex + kk].aliasCreated = m_args[orgArgIndex].aliasCreated;
                        halKernelParam->argParams[argIndex + kk].isNull = m_args[orgArgIndex].isNull;
                        halKernelParam->argParams[argIndex + kk].unitSize = m_args[orgArgIndex].unitSize;
                        vmeSurfOffset += vmeSize;
                    }
                }
            }
            else
            {
                CM_CHK_CMSTATUS_GOTOFINISH(CreateThreadArgData(&halKernelParam->argParams[argIndex ], orgArgIndex, nullptr, m_args));
            }
        }
        argIndex += argIndexStep;
    }

    if (m_dirty & cMKERNELDATASAMPLERBTIDIRTY)
    {
        if ( m_samplerBtiCount != 0 )
        {
            CmSafeMemCopy( ( void* )halKernelParam->samplerBTIParam.samplerInfo, ( void* )m_samplerBtiEntry, sizeof( m_samplerBtiEntry ) );
            halKernelParam->samplerBTIParam.samplerCount = m_samplerBtiCount;

            CmSafeMemSet(m_samplerBtiEntry, 0, sizeof(m_samplerBtiEntry));
            m_samplerBtiCount = 0;
        }
    }

    CM_CHK_CMSTATUS_GOTOFINISH(UpdateKernelDataGlobalSurfaceInfo( halKernelParam ));

    CM_CHK_CMSTATUS_GOTOFINISH(CalculateKernelSurfacesNum(surfNum, halKernelParam->numSurfaces));

    // GPGPU walker - implicit args
    uint32_t thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth;
    threadGroupSpace->GetThreadGroupSpaceSize(thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);

    halKernelParam->gpgpuWalkerParams.groupDepth = grpSpaceDepth;
    halKernelParam->gpgpuWalkerParams.groupHeight = grpSpaceHeight;
    halKernelParam->gpgpuWalkerParams.groupWidth  = grpSpaceWidth;
    halKernelParam->gpgpuWalkerParams.threadDepth = thrdSpaceDepth;
    halKernelParam->gpgpuWalkerParams.threadWidth  = thrdSpaceWidth;
    halKernelParam->gpgpuWalkerParams.threadHeight = thrdSpaceHeight;

    if (getVersionAsInt(m_program->m_cisaMajorVersion, m_program->m_cisaMinorVersion) < getVersionAsInt(3, 3))
    {
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup (halKernelParam->argParams[argIndex + 0].firstValue, thrdSpaceWidth));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup (halKernelParam->argParams[argIndex + 1].firstValue, thrdSpaceHeight));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup (halKernelParam->argParams[argIndex + 2].firstValue, grpSpaceWidth));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup (halKernelParam->argParams[argIndex + 3].firstValue, grpSpaceHeight));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup (halKernelParam->argParams[argIndex + 4].firstValue, thrdSpaceWidth));
        CM_CHK_CMSTATUS_GOTOFINISH(CreateKernelArgDataGroup (halKernelParam->argParams[argIndex + 5].firstValue, thrdSpaceHeight));
    }

    CM_CHK_CMSTATUS_GOTOFINISH(UpdateSamplerHeap(kernelData));
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Create kernel indirect data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelIndirectData(
    PCM_HAL_INDIRECT_DATA_PARAM  halIndirectData )    // in/out
{
    int32_t hr = CM_SUCCESS;

    halIndirectData->indirectDataSize = m_usKernelPayloadDataSize;
    halIndirectData->surfaceCount     = m_usKernelPayloadSurfaceCount;

    if( halIndirectData->indirectData == nullptr &&  m_usKernelPayloadDataSize != 0)
    {
        halIndirectData->indirectData = MOS_NewArray(uint8_t, halIndirectData->indirectDataSize);
        CM_CHK_NULL_GOTOFINISH(halIndirectData->indirectData, CM_OUT_OF_HOST_MEMORY);
    }

    // For future kernel data, pKbyte is starting point
    if( halIndirectData->surfaceInfo == nullptr &&  m_usKernelPayloadSurfaceCount != 0)
    {
        halIndirectData->surfaceInfo = MOS_NewArray(CM_INDIRECT_SURFACE_INFO, halIndirectData->surfaceCount);
        CM_CHK_NULL_GOTOFINISH(halIndirectData->surfaceInfo, CM_OUT_OF_HOST_MEMORY);
    }

    if(m_usKernelPayloadDataSize != 0)
    {
        CmSafeMemCopy(halIndirectData->indirectData, (void *)m_kernelPayloadData, m_usKernelPayloadDataSize);
    }

    if(m_usKernelPayloadSurfaceCount != 0)
    {
        CmSafeMemCopy((void*)halIndirectData->surfaceInfo, (void*)m_IndirectSurfaceInfoArray,
                    m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
        //clear m_IndirectSurfaceInfoArray every enqueue
        CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
        m_usKernelPayloadSurfaceCount = 0;
    }
finish:
    if( hr != CM_SUCCESS)
    {
        if(halIndirectData->indirectData)                 MosSafeDeleteArray(halIndirectData->indirectData);
        if(halIndirectData->surfaceInfo)                  MosSafeDeleteArray(halIndirectData->surfaceInfo);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    UpdateLastKernelData
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateLastKernelData(
    CmKernelData* & kernelData)    // in
{
    int32_t hr = CM_SUCCESS;

    if( kernelData == nullptr || m_lastKernelData == kernelData )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel data handle.");
        return CM_NULL_POINTER;
    }

    if(m_lastKernelData)
    {
        CmKernelData::Destroy(m_lastKernelData); // reduce ref count or delete it
    }
    CSync* kernelLock = m_device->GetProgramKernelLock();
    CLock locker(*kernelLock);
    m_lastKernelData = kernelData;
    m_lastKernelData->Acquire();
    m_lastKernelDataSize = m_lastKernelData->GetKernelDataSize();

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Wrapper of  CmKernelData::Destroy.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::ReleaseKernelData(
    CmKernelData* & kernelData)
{
    int32_t hr = CM_SUCCESS;

    if( kernelData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel data handle.");
        return CM_NULL_POINTER;
    }

    CSync* kernelLock = m_device->GetProgramKernelLock();
    CLock locker(*kernelLock);

    if(m_lastKernelData == kernelData)
    {
        // If the kernel data is the last kernel data
        // Need to update m_lastKernelData.
        hr = CmKernelData::Destroy(m_lastKernelData);
    }
    else
    {
        hr = CmKernelData::Destroy(kernelData);
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Acquire Kernel and Program
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::AcquireKernelProgram()
{
    CSync* kernelLock = m_device->GetProgramKernelLock();
    CLock locker(*kernelLock);

    this->Acquire(); // increase kernel's ref count
    m_program->Acquire(); // increase program's ref count

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Acquire KenrelData, Kernel and Program
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::AcquireKernelData(
    CmKernelData * &kernelData)
{
    int32_t hr = CM_SUCCESS;

    if (kernelData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel data handle.");
        return CM_NULL_POINTER;
    }

    CSync* kernelLock = m_device->GetProgramKernelLock();
    CLock locker(*kernelLock);
    kernelData->Acquire(); // increase kernel data's ref count

    return hr;
}

void CmKernelRT::SetAsClonedKernel(uint32_t cloneKernelID)
{
    m_isClonedKernel = true;
    m_cloneKernelID = cloneKernelID;
}

bool CmKernelRT::GetCloneKernelID(uint32_t& cloneKernelID)
{
    if (m_isClonedKernel)
    {
        cloneKernelID = m_cloneKernelID;
        return true;
    }

    return false;
}

void CmKernelRT::SetHasClones()
{
    m_hasClones = true;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Clone/copy current kernel
//| Returns:   New kernel with content of source kernel
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CloneKernel(CmKernelRT *& kernelOut, uint32_t id)
{
    int32_t hr = CM_SUCCESS;

    CSync* kernelLock = m_device->GetProgramKernelLock();
    CLock locker(*kernelLock);

    CmDynamicArray * kernelArray = m_device->GetKernelArray();

    uint32_t freeSlotinKernelArray = kernelArray->GetFirstFreeIndex();

    hr = Create(m_device, m_program, (char*)GetName(), freeSlotinKernelArray, id, kernelOut, m_options);

    if (hr == CM_SUCCESS)
    {
        kernelOut->SetAsClonedKernel(m_id >> 32);
        kernelArray->SetElement(freeSlotinKernelArray, kernelOut);
        uint32_t *kernelCount = m_device->GetKernelCount();
        *kernelCount = *kernelCount + 1;

        SetHasClones();
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Kernel's index in one task
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetIndexInTask(uint32_t index)
{
    m_indexInTask = index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel's index in one task
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetIndexInTask(void)
{
    return m_indexInTask;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Associated Flag
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetAssociatedToTSFlag(bool b)
{
    m_threadSpaceAssociated = b;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Set threadspace for kernel
//| Returns: Result of the operation.
//| Note: It's exclusive with AssociateThreadGroupSpace()
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::AssociateThreadSpace(CmThreadSpace *&threadSpace)
{
    if( threadSpace == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to thread space is null.");
        return CM_INVALID_ARG_VALUE;
    }

    PCM_HAL_STATE cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    if (cmHalState->cmHalInterface->CheckMediaModeAvailability() == false)
    {
        CmThreadSpaceRT *threadSpaceRTConst = static_cast<CmThreadSpaceRT *>(threadSpace);
        if (threadSpaceRTConst == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Pointer to thread space is null.");
            return CM_INVALID_ARG_VALUE;
        }
        CmThreadGroupSpace *threadGroupSpace = threadSpaceRTConst->GetThreadGroupSpace();
        return AssociateThreadGroupSpace(threadGroupSpace);
    }
    else 
    {
        if (m_threadGroupSpace != nullptr)
        {
            CM_ASSERTMESSAGE("Error: It's exclusive with AssociateThreadGroupSpace().");
            return CM_INVALID_KERNEL_THREADSPACE;
        }
    }

    bool threadSpaceChanged = false;
    if( m_threadSpace )
    {
        if( m_threadSpace != static_cast<CmThreadSpaceRT *>(threadSpace) )
        {
            threadSpaceChanged = true;
        }
    }

    m_threadSpace = static_cast<CmThreadSpaceRT *>(threadSpace);

    uint32_t threadSpaceWidth = 0;
    uint32_t threadSpaceHeight = 0;
    m_threadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);
    uint32_t threadCount = threadSpaceWidth * threadSpaceHeight;
    if (m_threadCount)
    {
        // Setting threadCount twice with different values will cause reset of kernels
        if (m_threadCount != threadCount)
        {
            m_threadCount = threadCount;
            m_dirty |= CM_KERNEL_DATA_THREAD_COUNT_DIRTY;
        }
    }
    else // first time
    {
        m_threadCount = threadCount;
    }

    if( threadSpaceChanged )
    {
        m_threadSpace->SetDirtyStatus( CM_THREAD_SPACE_DATA_DIRTY);
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Set thread group space for kernel
//| Returns: Result of the operation.
//| Note: It's exclusive with AssociateThreadSpace()
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::AssociateThreadGroupSpace(CmThreadGroupSpace *&threadGroupSpace)
{
    if( threadGroupSpace == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Invalid null pointer.");
        return CM_INVALID_ARG_VALUE;
    }

    if (m_threadSpace != nullptr)
    {
        CM_ASSERTMESSAGE("Error: It's exclusive with AssociateThreadSpace().");
        return CM_INVALID_KERNEL_THREADGROUPSPACE;
    }

    m_threadGroupSpace = threadGroupSpace;   

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Clear threadspace for kernel
//| Returns: Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::DeAssociateThreadSpace(CmThreadSpace * &threadSpace)
{
    if (threadSpace == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to thread space is null.");
        return CM_NULL_POINTER;
    }

    PCM_HAL_STATE cmHalState = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    if (cmHalState->cmHalInterface->CheckMediaModeAvailability() == false)
    {
        CmThreadSpaceRT *threadSpaceRTConst = static_cast<CmThreadSpaceRT *>(threadSpace);
        if (threadSpaceRTConst == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Pointer to thread space is null.");
            return CM_INVALID_ARG_VALUE;
        }

        CmThreadGroupSpace *threadGroupSpace = threadSpaceRTConst->GetThreadGroupSpace();
        if (m_threadGroupSpace != threadGroupSpace)
        {
            CM_ASSERTMESSAGE("Error: Invalid thread group space handle.");
            return CM_INVALID_ARG_VALUE;
        }
        m_threadGroupSpace = nullptr;
    }
    else
    {
        if (m_threadSpace != static_cast<CmThreadSpaceRT *>(threadSpace))
        {
            CM_ASSERTMESSAGE("Error: Invalid thread space handle.");
            return CM_INVALID_ARG_VALUE;
        }
        m_threadSpace = nullptr;
    }

    return CM_SUCCESS;
}
//*--------------------------------------------------------------------------------------------
//| Purpose: query spill memory size, the function can only take effect when jitter is enabled
//| Return: Result of the operation.
//*---------------------------------------------------------------------------------------------

CM_RT_API int32_t CmKernelRT::QuerySpillSize(uint32_t &spillMemorySize)
{
    CM_KERNEL_INFO  *kernelInfo = nullptr;

    int32_t hr = m_program->GetKernelInfo(m_kernelIndex, kernelInfo);
    if (hr != CM_SUCCESS || kernelInfo == nullptr)
        return hr;

    if (m_program->IsJitterEnabled()) {
        if (kernelInfo->jitInfo != nullptr) {
            spillMemorySize = (kernelInfo->jitInfo)->spillMemUsed;
            return hr;
        }
        else
            return CM_FAILURE;
    }

    return CM_FAILURE;
}

//*-----------------------------------------------------------------------------
//| Purpose: Clear threadgroupspace for kernel
//| Returns: Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::DeAssociateThreadGroupSpace(CmThreadGroupSpace * &threadGroupSpace)
{
    if (threadGroupSpace == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid null pointer.");
        return CM_NULL_POINTER;
    }
    if (m_threadGroupSpace != threadGroupSpace)
    {
        CM_ASSERTMESSAGE("Error: Invalid thread group space handle.");
        return CM_INVALID_ARG_VALUE;
    }
    m_threadGroupSpace = nullptr;
    m_dirty            = CM_KERNEL_DATA_THREAD_GROUP_SPACE_DIRTY;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Indicate whether thread arg existed.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelRT::IsThreadArgExisted()
{
    return m_perThreadArgExists;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the size of SharedLocalMemory
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetSLMSize()
{
    return (uint32_t)m_kernelInfo->kernelSLMSize;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the spill size of the kernel from JIT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetSpillMemUsed()
{
    uint32_t spillSize;

    if (m_program->IsJitterEnabled() && m_kernelInfo->jitInfo != nullptr)
    {
        spillSize = (m_kernelInfo->jitInfo)->spillMemUsed;
    }
    else
    {
        // kernel uses "--nojitter" option, don't allocate scratch space
        spillSize = 0;
    }

    return spillSize;
}

int32_t CmKernelRT::SearchAvailableIndirectSurfInfoTableEntry(uint16_t kind, uint32_t surfaceIndex, uint32_t bti)
{
    uint16_t i = 0;
    for ( i = 0; i < CM_MAX_STATIC_SURFACE_STATES_PER_BT; i++ )
    {
        if ( ( ( m_IndirectSurfaceInfoArray[ i ].surfaceIndex == surfaceIndex ) && ( m_IndirectSurfaceInfoArray[ i ].kind == kind ) && ( m_IndirectSurfaceInfoArray[ i ].bindingTableIndex == bti ) ) ||
            ( ( m_IndirectSurfaceInfoArray[ i ].surfaceIndex == 0 ) && ( m_IndirectSurfaceInfoArray[ i ].kind == 0 ) ) )
        {
            return i;
        }
    }
    // should never reach this
    CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
    return CM_FAILURE;
}

//-----------------------------------------------------------------------------------------------------------------
//! Set surface binding table index count for each indirect surface
//! INPUT:
//!     1) Surface format
//!     2) Surface type.
//! OUTPUT:
//!     binding table index count
//-----------------------------------------------------------------------------------------------------------------
int32_t CmKernelRT::SetSurfBTINumForIndirectData(CM_SURFACE_FORMAT format, CM_ENUM_CLASS_TYPE surfaceType)
{
    if (surfaceType == CM_ENUM_CLASS_TYPE_CMBUFFER_RT)
    {
        return 1;
    }
    else
    {
        if ((format == CM_SURFACE_FORMAT_NV12) ||
            (format == CM_SURFACE_FORMAT_P010) ||
            (format == CM_SURFACE_FORMAT_P208) ||
            (format == CM_SURFACE_FORMAT_P016))
        {
            return 2;
        }
        else if (format == CM_SURFACE_FORMAT_422H ||
            format == CM_SURFACE_FORMAT_411P ||
            format == CM_SURFACE_FORMAT_IMC3 ||
            format == CM_SURFACE_FORMAT_422V ||
            format == CM_SURFACE_FORMAT_444P)
        {   // 3 planes surface
            return 3;
        }
        else
        {
            return 1;
        }
    }
    // should never reach this
    CM_ASSERTMESSAGE("Error: Set surface binding table index count failure.");
    return 0;
}

//-----------------------------------------------------------------------------------------------------------------
//! Set surface binding table index by user.
//! If application hope to assign a specific binding table index for a surface, it should call this function.
//! The assigned binding table index should be an valid value for general surface ( say >=1 and <=242),
//! otherwise, this call will return failure.
//! INPUT:
//!     1) Surface whose binding table index need be set.
//!     2) Assiend binding table index.
//! OUTPUT:
//!     CM_SUCCESS
//!     CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX if the surface index is not a valid binding table index (valid: 1~242)
//!     CM_FAILURE otherwise
//-----------------------------------------------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetSurfaceBTI(SurfaceIndex* surface, uint32_t btIndex)
{

    uint32_t                    width, height, bytesPerPixel;
    CM_SURFACE_FORMAT           format = CM_SURFACE_FORMAT_INVALID;
    //Sanity check
    if (surface == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface is null.");
        return CM_NULL_POINTER;
    }
    if (!m_surfaceMgr->IsValidSurfaceIndex(btIndex))
    {
        CM_ASSERTMESSAGE("Error: Invalid binding table index.");
        return CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX;
    }

    //Sanity check: if the BTI has been used once enqueue
    uint32_t i = 0;
    for (i = 0; i < m_usKernelPayloadSurfaceCount; i++)
    {
        if (m_IndirectSurfaceInfoArray[i].bindingTableIndex == (uint16_t)btIndex)
        {
            CM_ASSERTMESSAGE("Error: Binding table index has been used once enqueue.");
            return CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX;
        }
    }

    uint32_t index = surface->get_data();
    uint32_t handle = 0;

    CmSurface* surfaceRT = nullptr;
    m_surfaceMgr->GetSurface( index, surfaceRT );
    if(surfaceRT == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid surface.");
        return CM_NULL_POINTER;
    }

    CmSurface2DRT* surf2D = nullptr;
    uint32_t indirectSurfInfoEntry = 0;
    if ( surfaceRT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACE2D )
    {
        surf2D = static_cast< CmSurface2DRT* >( surfaceRT );
        surf2D->GetHandle( handle );
        indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_2D, handle, btIndex);
        if (indirectSurfInfoEntry == CM_FAILURE)
        {
            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
            return CM_FAILURE;
        }
        m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_2D;
        m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
        surf2D->GetSurfaceDesc(width, height, format, bytesPerPixel);
    }
    else
    {
        CmBuffer_RT* cmBuffer = nullptr;
        if ( surfaceRT->Type() == CM_ENUM_CLASS_TYPE_CMBUFFER_RT )
        {
            cmBuffer = static_cast< CmBuffer_RT* >( surfaceRT );
            cmBuffer->GetHandle( handle );
            indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_1D, handle, btIndex);
            if (indirectSurfInfoEntry == CM_FAILURE)
            {
                CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                return CM_FAILURE;
            }
            m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_1D;
            m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
        }
        else
        {
            CmSurface2DUPRT* surf2DUP = nullptr;
            if ( surfaceRT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACE2DUP )
            {
                surf2DUP = static_cast< CmSurface2DUPRT* >( surfaceRT );
                surf2DUP->GetHandle( handle );
                indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_2D_UP, handle, btIndex);
                if (indirectSurfInfoEntry == CM_FAILURE)
                {
                    CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                    return CM_FAILURE;
                }
                m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_2D_UP;
                m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
                surf2DUP->GetSurfaceDesc(width, height, format, bytesPerPixel);
            }
            else
            {
                CmSurfaceSampler* surfSampler = nullptr;
                if ( surfaceRT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER )
                {
                    surfSampler = static_cast< CmSurfaceSampler* >(surfaceRT);

                    //Get  actually SurfaceIndex ID for 2D
                    uint16_t surfIndexForCurrent = 0;
                    surfSampler->GetCmIndexCurrent(surfIndexForCurrent);
                    CmSurface* surfSampRT= nullptr;
                    m_surfaceMgr->GetSurface(surfIndexForCurrent, surfSampRT);
                    if(surfSampRT == nullptr)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid surface.");
                        return CM_NULL_POINTER;
                    }

                    SAMPLER_SURFACE_TYPE surfaceType;
                    surfSampler->GetSurfaceType(surfaceType);
                    surfSampler->GetHandle( handle );
                    if ( surfaceType == SAMPLER_SURFACE_TYPE_2D )
                    {
                        CmSurface2DRT* surfSamp2D = nullptr;
                        surfSamp2D = static_cast<CmSurface2DRT*>(surfSampRT);
                        surfSamp2D->GetSurfaceDesc(width, height, format, bytesPerPixel);

                        indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_SAMPLER, handle, btIndex);
                        if (indirectSurfInfoEntry == CM_FAILURE)
                        {
                            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                            return CM_FAILURE;
                        }
                        m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_SAMPLER;
                    }
                    else if ( surfaceType == SAMPLER_SURFACE_TYPE_2DUP )
                    {
                        CmSurface2DUPRT* surfSamp2DUP = nullptr;
                        surfSamp2DUP = static_cast<CmSurface2DUPRT*>(surfSampRT);
                        surfSamp2DUP->GetSurfaceDesc(width, height, format, bytesPerPixel);

                        indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE2DUP_SAMPLER, handle, btIndex);
                        if (indirectSurfInfoEntry == CM_FAILURE)
                        {
                            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                            return CM_FAILURE;
                        }
                        m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE2DUP_SAMPLER;
                    }
                    else if ( surfaceType == SAMPLER_SURFACE_TYPE_3D )
                    {
                        indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_3D, handle, btIndex);
                        if (indirectSurfInfoEntry == CM_FAILURE)
                        {
                            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                            return CM_FAILURE;
                        }
                        m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_3D;
                    }
                    m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
                }
                else
                {
                    CmSurfaceSampler8x8* surfSampler8x8 = nullptr;
                    if ( surfaceRT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8 )
                    {
                        surfSampler8x8 = static_cast< CmSurfaceSampler8x8* >( surfaceRT );
                        surfSampler8x8->GetIndexCurrent( handle );

                        //Get  actually SurfaceIndex ID for 2D
                        uint16_t surfIndexForCurrent = 0;
                        surfSampler8x8->GetCmIndex(surfIndexForCurrent);
                        CmSurface* surfSamp8x8RT = nullptr;
                        m_surfaceMgr->GetSurface(surfIndexForCurrent, surfSamp8x8RT);
                        if(surfSamp8x8RT == nullptr)
                        {
                            CM_ASSERTMESSAGE("Error: Invalid surface.");
                            return CM_NULL_POINTER;
                        }

                        CmSurface2DRT* surfSamp8x82D = nullptr;
                        surfSamp8x82D = static_cast<CmSurface2DRT*>(surfSamp8x8RT);
                        surfSamp8x82D->GetSurfaceDesc(width, height, format, bytesPerPixel);

                        if ( surfSampler8x8->GetSampler8x8SurfaceType() == CM_AVS_SURFACE )
                        {
                            indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_SAMPLER8X8_AVS, handle, btIndex);
                            if (indirectSurfInfoEntry == CM_FAILURE)
                            {
                                CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                                return CM_FAILURE;
                            }
                            m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
                        }
                        else if ( surfSampler8x8->GetSampler8x8SurfaceType() == CM_VA_SURFACE )
                        {
                            indirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_SAMPLER8X8_VA, handle, btIndex);
                            if (indirectSurfInfoEntry == CM_FAILURE)
                            {
                                CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                                return CM_FAILURE;
                            }
                            m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].kind = ARG_KIND_SURFACE_SAMPLER8X8_VA;
                        }
                        m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
                    }
                    else
                    {
                            return CM_FAILURE;
                    }
                }
            }
        }
    }

    m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].bindingTableIndex = (uint16_t)btIndex;
    if (SetSurfBTINumForIndirectData(format, surfaceRT->Type())== 0)
    {
        CM_ASSERTMESSAGE("Error: Set surface binding table index count failure.");
        return CM_FAILURE;
    }
    m_IndirectSurfaceInfoArray[indirectSurfInfoEntry].numBTIPerSurf = (uint16_t)SetSurfBTINumForIndirectData(format, surfaceRT->Type());

    //Copy it to surface index array

    m_pKernelPayloadSurfaceArray[indirectSurfInfoEntry] = surface;


    // count is actally one larger than the actual index
    m_usKernelPayloadSurfaceCount = indirectSurfInfoEntry + 1;
    m_dirty |= (CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY | CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY);
    return CM_SUCCESS;
}

uint32_t CmKernelRT::GetKernelIndex()
{
    return m_kernelIndex;
}
uint32_t CmKernelRT::GetKernelGenxBinarySize(void)
{
    if(m_kernelInfo == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel genx binary size.");
        return 0;
    }
    else
    {
        return m_kernelInfo->genxBinarySize;
    }
}

//-----------------------------------------------------------------------------------------------------------------
//! Map Surface type to Kernel arg Kind.
//! INPUT:  Surface type    :CM_ENUM_CLASS_TYPE
//! OUTPUT: Kernel arg Kind :CM_ARG_KIND
//-----------------------------------------------------------------------------------------------------------------
CM_ARG_KIND CmKernelRT::SurfTypeToArgKind(CM_ENUM_CLASS_TYPE surfType)
{
    switch(surfType)
    {
        case CM_ENUM_CLASS_TYPE_CMBUFFER_RT          :return ARG_KIND_SURFACE_1D;
        case CM_ENUM_CLASS_TYPE_CMSURFACE2D          :return ARG_KIND_SURFACE_2D;
        case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP        :return ARG_KIND_SURFACE_2D_UP;
        case CM_ENUM_CLASS_TYPE_CMSURFACE3D          :return ARG_KIND_SURFACE_3D;
        case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER     :return ARG_KIND_SURFACE_SAMPLER;
        case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8  :return ARG_KIND_SURFACE_SAMPLER8X8_AVS;
        case CM_ENUM_CLASS_TYPE_CMSURFACEVME         :return ARG_KIND_SURFACE_VME;
        case CM_ENUM_CLASS_TYPE_CMSAMPLER_RT         :return ARG_KIND_SAMPLER;
        case CM_ENUM_CLASS_TYPE_CMSAMPLER8X8STATE_RT :return ARG_KIND_SAMPLER;
        case CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER      :return ARG_KIND_STATE_BUFFER;

        default:
            CM_ASSERTMESSAGE("Error: Invalid surface type.");
            break;
   }
   return ARG_KIND_GENERAL;
}

int32_t CmKernelRT::CalculateKernelSurfacesNum(uint32_t& kernelSurfaceNum, uint32_t& neededBTEntryNum)
{
    uint32_t            surfaceArraySize = 0;
    CmSurface*          surf = nullptr;
    CmSurface2DRT*        surf2D = nullptr;
    CmSurface2DUPRT*      surf2DUP = nullptr;
    uint32_t              width, height, bytesPerPixel;
    CM_SURFACE_FORMAT     format;
    uint32_t              maxBTIndex = 0;

    kernelSurfaceNum = 0;
    neededBTEntryNum = 0;

    surfaceArraySize = m_surfaceMgr->GetSurfacePoolSize();

    //Calculate surface number and needed binding table entries
    for (uint32_t surfIndex = 0; surfIndex <= m_maxSurfaceIndexAllocated; surfIndex ++)
    {
        if (m_surfaceArray[surfIndex%surfaceArraySize])
        {
            surf = nullptr;
            m_surfaceMgr->GetSurface(surfIndex, surf);
            if (surf)
            {
                switch(surf->Type())
                {
                    case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
                    case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
                        kernelSurfaceNum ++;
                        neededBTEntryNum ++;
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACEVME:
                    case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
                    case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
                        //virtual surface, no need increase count
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
                        kernelSurfaceNum++;
                        surf2D = static_cast<CmSurface2DRT*>(surf);
                        format = CM_SURFACE_FORMAT_INVALID;
                        surf2D->GetSurfaceDesc(width, height, format, bytesPerPixel);
                        if ((format == CM_SURFACE_FORMAT_NV12) ||
                            (format == CM_SURFACE_FORMAT_P010) ||
                            (format == CM_SURFACE_FORMAT_P208) ||
                            (format == CM_SURFACE_FORMAT_P016))
                        {
                            neededBTEntryNum += 2;
                        }
                        else if (format == CM_SURFACE_FORMAT_422H ||
                            format == CM_SURFACE_FORMAT_411P ||
                            format == CM_SURFACE_FORMAT_IMC3 ||
                            format == CM_SURFACE_FORMAT_422V ||
                            format == CM_SURFACE_FORMAT_444P)
                        {   // 3 planes surface
                            neededBTEntryNum += 3;
                        }
                        else
                        {
                            neededBTEntryNum += 1;
                        }
                        break;

                    case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
                        kernelSurfaceNum++;
                        surf2DUP = static_cast<CmSurface2DUPRT*>(surf);
                        format = CM_SURFACE_FORMAT_INVALID;
                        surf2DUP->GetSurfaceDesc(width, height, format, bytesPerPixel);
                        if ((format == CM_SURFACE_FORMAT_NV12) ||
                            (format == CM_SURFACE_FORMAT_P010) ||
                            (format == CM_SURFACE_FORMAT_P208) ||
                            (format == CM_SURFACE_FORMAT_P016))
                        {
                            neededBTEntryNum += 2;
                        }
                        else if (format == CM_SURFACE_FORMAT_422H ||
                            format == CM_SURFACE_FORMAT_411P ||
                            format == CM_SURFACE_FORMAT_IMC3 ||
                            format == CM_SURFACE_FORMAT_422V ||
                            format == CM_SURFACE_FORMAT_444P)
                        {   // 3 planes surface
                            neededBTEntryNum += 3;
                        }
                        else
                        {
                            neededBTEntryNum += 1;
                        }
                        break;

                    default:
                        break;
                }
            }
        }
    }

    if ((maxBTIndex + 1) > neededBTEntryNum)
    {
        neededBTEntryNum = maxBTIndex + 1;
    }

    //Wordaround: the calculation maybe not accurate if the VME surfaces are existed
    neededBTEntryNum += m_vmeSurfaceCount;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get aligned curbe size for different platforms
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetAlignedCurbeSize(uint32_t value)
{
    uint32_t curbeAlignedSize    = 0;

    curbeAlignedSize = MOS_ALIGN_CEIL(value, RENDERHAL_CURBE_BLOCK_ALIGN);
    return curbeAlignedSize;
}

#if CM_LOG_ON
std::string CmKernelRT::Log()
{

    std::ostringstream  oss;

    oss << " Kernel Name:"         << m_kernelInfo->kernelName << std::endl
        << " Kernel Binary Size:"  << m_kernelInfo->jitBinarySize
        << " Index In Task:"       << m_indexInTask
        << " Thread Count:"        << m_threadCount
        << " Curbe Size:"          << m_sizeInCurbe
        << " Kernel arg Count:"    << m_argCount
        << std::endl;

     // Per Kernel Thread Space Log
    if(m_threadSpace)
    {
        oss << m_threadSpace->Log();
    }

    // Per Kernel Thread Group Space Log
    if(m_threadGroupSpace)
    {
        oss << m_threadGroupSpace->Log();
    }

    // Arguments Log
    for (uint32_t argIndex= 0; argIndex< m_argCount; argIndex++ )
    {
        if (m_args[argIndex].value) // filter out the implicit arguments
        {
            ArgLog(oss, argIndex, m_args[argIndex]);
        }
    }

    return oss.str();
}

void CmKernelRT::ArgLog(std::ostringstream &oss, uint32_t index, CM_ARG arg)
{

    oss << "[" << index << "] th Argument"
        << " Type :" << arg.unitKind
        << " Count:" << arg.unitCount
        << " Size:" << arg.unitSize
        << " Surface Kind:" << (int)arg.surfaceKind
        << " OffsetInPayload:" << arg.unitOffsetInPayload
        << " OffsetInPayloadOrig:" << arg.unitOffsetInPayloadOrig << "";

    CmLogger::LogDataArrayHex( oss, arg.value, arg.unitSize * arg.unitCount);

    if (CHECK_SURFACE_TYPE(arg.unitKind,
                           ARG_KIND_SURFACE_1D,
                           ARG_KIND_SURFACE_2D,
                           ARG_KIND_SURFACE_2D_UP,
                           ARG_KIND_SURFACE_VME,
                           ARG_KIND_SURFACE_SAMPLER,
                           ARG_KIND_SURFACE_3D,
                           ARG_KIND_SURFACE_SAMPLER8X8_AVS,
                           ARG_KIND_SURFACE_SAMPLER8X8_VA,
                           ARG_KIND_SURFACE2DUP_SAMPLER))
    {
        uint16_t numSurfaces = arg.unitSize / sizeof(uint32_t);
        if (arg.unitKind == ARG_KIND_SURFACE_VME)
        {
            numSurfaces = (arg.unitSize - sizeof(CM_HAL_VME_ARG_VALUE) * arg.unitVmeArraySize) / sizeof(uint32_t) + arg.unitVmeArraySize;
        }
        for (uint16_t i = 0; i < numSurfaces; i++)
        {
            uint32_t surfaceIndex = *(uint16_t *)(arg.surfIndex + i);

            if(surfaceIndex == CM_NULL_SURFACE)
                continue;

            CmSurface *surf = nullptr;
            m_surfaceMgr->GetSurface(surfaceIndex, surf);
            if (surf == nullptr)
            {
                continue;
            }
            surf->Log(oss);
        }
    }
}
#endif

void CmKernelRT::SurfaceDump(uint32_t kernelNumber, int32_t taskId)
{
#if MDF_SURFACE_CONTENT_DUMP
    CM_ARG arg;

    for (uint32_t argIndex = 0; argIndex< m_argCount; argIndex++)
    {
        arg = m_args[argIndex];
        if (CHECK_SURFACE_TYPE(arg.unitKind,
            ARG_KIND_SURFACE_1D,
            ARG_KIND_SURFACE_2D,
            ARG_KIND_SURFACE_2D_UP,
            ARG_KIND_SURFACE_VME,
            ARG_KIND_SURFACE_SAMPLER,
            ARG_KIND_SURFACE_3D,
            ARG_KIND_SURFACE_SAMPLER8X8_AVS,
            ARG_KIND_SURFACE_SAMPLER8X8_VA,
            ARG_KIND_SURFACE2DUP_SAMPLER))
        {
            uint16_t numSurfaces = arg.unitSize / sizeof(uint32_t);
            if (arg.unitKind == ARG_KIND_SURFACE_VME)
            {
                numSurfaces = (arg.unitSize - sizeof(CM_HAL_VME_ARG_VALUE) * arg.unitVmeArraySize) / sizeof(uint32_t) + arg.unitVmeArraySize;
            }

            for (uint16_t i = 0; i < numSurfaces; i++)
            {
                uint32_t surfaceIndex = *(uint16_t *)(arg.surfIndex + i);
                CmSurface *surf = nullptr;
                m_surfaceMgr->GetSurface(surfaceIndex, surf);
                if (surf == nullptr)
                {
                    return;
                }
                surf->DumpContent(kernelNumber, m_kernelInfo->kernelName, taskId, argIndex, i);
            }
        }
    }
#endif
}

CM_RT_API int32_t CmKernelRT::SetSamplerBTI(SamplerIndex* sampler, uint32_t nIndex)
{
    if (!sampler)
    {
        return CM_NULL_POINTER;
    }
    if (CM_SAMPLER_MAX_BINDING_INDEX < nIndex)
    {
        return CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX;
    }

    uint32_t        samplerIndex   = sampler->get_data();
    PCM_HAL_STATE   cmHalState    = ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;

    uint32_t i = 0;
    for (i = 0; i < m_samplerBtiCount; i++)
    {
        if ((m_samplerBtiEntry[i].samplerIndex == samplerIndex) && (m_samplerBtiEntry[i].samplerBTI == nIndex))
        {
            break;
        }
        if (m_dirty & cMKERNELDATASAMPLERBTIDIRTY)
        {
            if ((m_samplerBtiEntry[i].samplerIndex != samplerIndex) && (m_samplerBtiEntry[i].samplerBTI == nIndex))
            {
                if (cmHalState->useNewSamplerHeap)
                {
                    SamplerParam sampler1 = {};
                    SamplerParam sampler2 = {};
                    cmHalState->cmHalInterface->GetSamplerParamInfoForSamplerType(&cmHalState->samplerTable[m_samplerBtiEntry[i].samplerIndex], sampler1);
                    cmHalState->cmHalInterface->GetSamplerParamInfoForSamplerType(&cmHalState->samplerTable[samplerIndex], sampler2);

                    if (sampler1.elementType== sampler2.elementType)
                    {
                        // return failure only if the two samplers have the same type, because different type samplers are able to set to the same BTI
                        return CM_FAILURE;
                    }
                }
                else
                {
                    return CM_FAILURE;
                }
            }

            CmSampler8x8State_RT *sampler8x8 = nullptr;
            CmSampler8x8State_RT *tmpSampler8x8 = nullptr;
            m_device->GetSampler8x8(samplerIndex, sampler8x8);
            m_device->GetSampler8x8(m_samplerBtiEntry[i].samplerIndex, tmpSampler8x8);

            if (sampler8x8 && tmpSampler8x8 && (sampler8x8->GetStateType() == CM_SAMPLER8X8_AVS)
                && (tmpSampler8x8->GetStateType() == CM_SAMPLER8X8_AVS) &&
                cmHalState->cmHalInterface->IsAdjacentSamplerIndexRequiredbyHw())
            {
                if ((m_samplerBtiEntry[i].samplerIndex != samplerIndex) &&
                    ((m_samplerBtiEntry[i].samplerBTI == nIndex + 1) || (m_samplerBtiEntry[i].samplerBTI == nIndex - 1)))
                    return CM_FAILURE;
            }
        }
    }

    if (i >= CM_MAX_SAMPLER_TABLE_SIZE)
    {
        CM_ASSERTMESSAGE("Error: Exceed maximum sampler table size.");
        return CM_FAILURE;
    }

    if (i == m_samplerBtiCount)
    {
        m_samplerBtiEntry[i].samplerIndex = samplerIndex;
        m_samplerBtiEntry[i].samplerBTI = nIndex;

        m_samplerBtiCount = i + 1;

        m_dirty |= cMKERNELDATASAMPLERBTIDIRTY;
    }
    return CM_SUCCESS;
}

CMRT_UMD_API int32_t CmKernelRT::GetBinary(std::vector<char>& binary)
{
    binary.resize(m_binarySize);

    CmSafeMemCopy((void *)&binary[0], (void *)m_binary, m_binarySize);

    return CM_SUCCESS;
}

CMRT_UMD_API int32_t CmKernelRT::ReplaceBinary(std::vector<char>& binary)
{
    uint32_t size = binary.size();

    if (size == 0)
    {
        return CM_INVALID_ARG_VALUE;
    }

    if(m_binaryOrig == nullptr)
    {
        //Store the orignal binary once.
        m_binaryOrig = m_binary;
        m_binarySizeOrig = m_binarySize;
    }

    m_binary = MOS_NewArray(char, size);
    CmSafeMemCopy((void *)m_binary, (void *)&binary[0], size);

    m_binarySize = size;

    return CM_SUCCESS;
}

CMRT_UMD_API int32_t CmKernelRT::ResetBinary()
{
    if (m_binaryOrig == nullptr)
    {
        //ReplaceBinary is never called
        return CM_SUCCESS;
    }
    if(m_binary!= m_binaryOrig)
    {
        MosSafeDeleteArray(m_binary);
    }
    m_binary = m_binaryOrig;
    m_binarySize = m_binarySizeOrig;

    return CM_SUCCESS;
}

int CmKernelRT::UpdateSamplerHeap(CmKernelData *kernelData)
{
    // Get sampler bti & offset
    PCM_HAL_KERNEL_PARAM cmKernel = nullptr;
    PCM_CONTEXT_DATA cmData = (PCM_CONTEXT_DATA)m_device->GetAccelData();
    PCM_HAL_STATE state = cmData->cmHalState;
    std::list<SamplerParam>::iterator iter;
    unsigned int heapOffset = 0;

    if (state->useNewSamplerHeap == false)
    {
        return CM_SUCCESS;
    }

    heapOffset = 0;
    cmKernel = kernelData->GetHalCmKernelData();
    std::list<SamplerParam> *sampler_heap = cmKernel->samplerHeap;

    // First pass, inserts sampler with user-defined BTI to the list. Sorts by element order low to high, then by BTI order low to high.
    for (unsigned int samplerElementType = MHW_Sampler1Element; samplerElementType < MHW_SamplerTotalElements; samplerElementType++)
    {
        for (unsigned int n = 0; n < cmKernel->samplerBTIParam.samplerCount; ++n)
        {
            SamplerParam sampler = {};
            sampler.samplerTableIndex = cmKernel->samplerBTIParam.samplerInfo[n].samplerIndex;

            if (state->samplerTable[sampler.samplerTableIndex].ElementType == samplerElementType)
            {
                sampler.bti = cmKernel->samplerBTIParam.samplerInfo[n].samplerBTI;
                sampler.userDefinedBti = true;
                state->cmHalInterface->GetSamplerParamInfoForSamplerType(&state->samplerTable[sampler.samplerTableIndex], sampler);

                // Guarantees each user-defined BTI has a spacing between each other user-defined BTIs larger than the stepping
                for (iter = sampler_heap->begin(); iter != sampler_heap->end(); ++iter)
                {
                    if (iter->elementType == sampler.elementType)
                    {
                        unsigned int diff = (iter->bti > sampler.bti) ? (iter->bti - sampler.bti) : (sampler.bti - iter->bti);
                        if (diff < sampler.btiStepping)
                        {
                            CM_ASSERTMESSAGE("Sampler BTI setting error. Confliction with other Sampler BTI.\n");
                            return MOS_STATUS_INVALID_PARAMETER;
                        }
                    }
                }

                // Inserts by the order
                for (iter = sampler_heap->begin(); iter != sampler_heap->end(); ++iter)
                {
                    if (iter->elementType > sampler.elementType)
                    {
                        break;
                    }
                    else if ((iter->elementType == sampler.elementType) && (iter->bti > sampler.bti))
                    {
                        break;
                    }
                }
                sampler.heapOffset = sampler.bti * sampler.btiMultiplier;
                sampler_heap->insert(iter, sampler);
            }
        }
    }

    // Second pass, loops over all kernel/thread args, find regular sampler and insert to sampler heap.
    // Follows the existing sorted order.
    for (unsigned int samplerElementType = MHW_Sampler1Element; samplerElementType < MHW_SamplerTotalElements; samplerElementType++)
    {
        for (unsigned int index = 0; index < cmKernel->numArgs; index++)
        {
            PCM_HAL_KERNEL_ARG_PARAM argParam = &cmKernel->argParams[index];
            if (argParam->isNull)
            {
                continue;
            }

            for (unsigned int threadIndex = 0; threadIndex < argParam->unitCount; threadIndex++)
            {
                if (argParam->kind == CM_ARGUMENT_SAMPLER)
                {
                    unsigned char *arg = argParam->firstValue + (threadIndex * argParam->unitSize);
                    unsigned int samplerTableIndex = *((uint32_t *)arg);

                    SamplerParam sampler = {};
                    sampler.samplerTableIndex = samplerTableIndex;
                    state->cmHalInterface->GetSamplerParamInfoForSamplerType(&state->samplerTable[sampler.samplerTableIndex], sampler);
                    sampler.regularBti = true;

                    if (sampler.elementType != samplerElementType)
                    {
                        continue;
                    }

                    // if the sampler is already in the heap, skip
                    bool isDuplicate = false;
                    for (iter = sampler_heap->begin(); iter != sampler_heap->end(); ++iter)
                    {
                        if (iter->samplerTableIndex == sampler.samplerTableIndex)
                        {
                            isDuplicate = true;
                            iter->regularBti = true;
                            break;
                        }
                    }
                    if (isDuplicate == true)
                    {
                        continue;
                    }

                    // insert the new sampler to the heap
                    heapOffset = 0;
                    for (iter = sampler_heap->begin(); iter != sampler_heap->end(); ++iter)
                    {
                        if (iter->elementType == sampler.elementType)
                        {
                            // Needs to keep the inserted sampler's correctness, so do not insert before same element regular sampler
                            // Only insert before user-defined BTI
                            if (iter->userDefinedBti == true)
                            {
                                unsigned int curOffset = iter->heapOffset;
                                if (heapOffset > curOffset)
                                {
                                    // Confliction, which means that sampler heap in smaller
                                    // element type has excced the position which is supposed
                                    // to put this user-defined BTI sampler.
                                    // User needs to set the BTI to a larger value.
                                    CM_ASSERTMESSAGE("Sampler BTI setting error. Confliction with other Sampler BTI.\n");
                                    return MOS_STATUS_INVALID_PARAMETER;
                                }
                                else
                                {
                                    if (curOffset - heapOffset >= sampler.btiStepping * sampler.btiMultiplier)
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        heapOffset = curOffset + iter->btiStepping * iter->btiMultiplier;
                                    }
                                }
                            }
                            else
                            {
                                heapOffset += iter->btiStepping * iter->btiMultiplier;
                            }
                        }
                        else if (iter->elementType > sampler.elementType)
                        {
                            break;
                        }
                        else
                        {
                            heapOffset = iter->heapOffset + iter->size;
                            std::list<SamplerParam>::iterator iter_next = std::next(iter, 1);
                            if ((iter_next != sampler_heap->end()) && (iter_next->elementType > iter->elementType))
                            {
                                // Aligns heapOffset to next nearest multiple of sampler size if next sampler is a different element type
                                heapOffset = (heapOffset + iter_next->btiStepping * iter_next->btiMultiplier - 1) / (iter_next->btiStepping * iter_next->btiMultiplier) * (iter_next->btiStepping * iter_next->btiMultiplier);
                            }
                        }
                    }

                    if (iter == sampler_heap->end())
                    {
                        // Aligns heapOffset to next nearest multiple of sampler size if next sampler is a different element type
                        heapOffset = (heapOffset + sampler.btiStepping * sampler.btiMultiplier - 1) / (sampler.btiStepping * sampler.btiMultiplier) * (sampler.btiStepping * sampler.btiMultiplier);
                    }
                    sampler.heapOffset = heapOffset;

                    if (sampler.btiMultiplier != 0) 
                    {
                        sampler.bti = sampler.heapOffset / sampler.btiMultiplier;
                    }
                    else
                    {
                        CM_ASSERTMESSAGE("Sampler BTI setting error. Multiplier cannot be zero!\n");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                    sampler_heap->insert(iter, sampler);
                }
            }
        }
    }

    return CM_SUCCESS;
}
}
