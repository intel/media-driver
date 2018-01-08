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

#if USE_EXTENSION_CODE
#include "cm_mov_inst_ext.h"
#endif

#define GENERATE_GLOBAL_SURFACE_INDEX

#define READ_FIELD_FROM_BUF( dst, type ) \
    dst = *((type *) &buf[byte_pos]); \
    byte_pos += sizeof(type);

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
    pVmeIndexArray[VmeIndexArrayPosition] = value ;                 \
    VmeIndexArrayPosition ++;

#define   ADD_INTO_VME_CM_INDEX_ARRAY(value)  ; \
    pVmeCmIndexArray[VmeCmIndexArrayPosition] = value ;                 \
    VmeCmIndexArrayPosition ++;

typedef CM_ARG* PCM_ARG;

#define CM_KERNEL_DATA_CLEAN                   0         // kernel data clean
#define CM_KERNEL_DATA_KERNEL_ARG_DIRTY        1         // per kernel arg dirty
#define CM_KERNEL_DATA_THREAD_ARG_DIRTY        (1 << 1)  // per thread arg dirty
#define CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY      (1 << 2)  // indirect payload data dirty
#define CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY (1 << 3)  // indirect payload data size changes
#define CM_KERNEL_DATA_GLOBAL_SURFACE_DIRTY    (1 << 4)  // global surface dirty
#define CM_KERNEL_DATA_THREAD_COUNT_DIRTY      (1 << 5)  // thread count dirty, reset() be called
#define CM_KERNEL_DATA_SAMPLER_BTI_DIRTY       (1 << 6)  // sampler bti dirty

int32_t Partition( PCM_ARG* ppArg, int32_t p, int32_t r )
{
    uint16_t x = ppArg[p]->unitOffsetInPayload;
    int32_t i = p - 1;
    int32_t j = r + 1;
    while( 1 )
    {
        do {
            j --;
        } while( ppArg[j]->unitOffsetInPayload > x );

        do {
            i ++;
        } while( ppArg[i]->unitOffsetInPayload < x );

        if( i < j )
        {
            PCM_ARG tmpP = ppArg[i];
            ppArg[i] = ppArg[j];
            ppArg[j] = tmpP;
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
    bool bMatch = false;
    va_list ap;
    va_start( ap, nType );
    int iType = 0;

    while ( ( iType = va_arg( ap, int ) ) >= 0 )
    {
        if( iType == nType )
        {
            bMatch = true;
            break;
        }
    }
    va_end(ap);

    return bMatch;
}

void QuickSort( PCM_ARG* ppArg, int32_t p, int32_t r )
{
    if( p < r )
    {
        int32_t q = Partition( ppArg, p, r );
        QuickSort( ppArg, p, q );
        QuickSort( ppArg, q + 1, r );
    }
}

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:     Create CM Kernel
//| Arguments :
//|               pCmDev        [in]    Pointer to device
//|               pProgram      [in]    Pointer to cm Program
//|               kernelName    [in]    Name of kernel
//|               kernelId      [in]    Kernel's ID
//|               pKernel       [in/out]    Reference Pointer to CM Kernel
//|               options       [in]    jitter, or non-jitter
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Create(CmDeviceRT *pCmDev,
                           CmProgramRT *pProgram,
                           const char *kernelName,
                           uint32_t KernelIndex,
                           uint32_t KernelSeqNum,
                           CmKernelRT* &pKernel,
                           const char *options)
{
    int32_t result = CM_SUCCESS;
    pKernel = new (std::nothrow) CmKernelRT( pCmDev, pProgram, KernelIndex, KernelSeqNum );
    if( pKernel )
    {
        pKernel->Acquire();
        result = pKernel->Initialize( kernelName, options );
        if( result != CM_SUCCESS )
        {
            CmKernelRT::Destroy( pKernel, pProgram);
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
            pKernel->m_blCreatingGPUCopyKernel = true;
        }
        else
        {
            pKernel->m_blCreatingGPUCopyKernel = false;
        }
    }

#if USE_EXTENSION_CODE
    result = pKernel->InitForGTPin(pCmDev, pProgram, pKernel);
#endif

    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destory Kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::Destroy( CmKernelRT* &pKernel, CmProgramRT *&pProgram )
{
    uint32_t refCount = pKernel->SafeRelease();
    if (refCount == 0)
    {
        pKernel = nullptr;
    }

    refCount = pProgram->SafeRelease();
    if (refCount == 0)
    {
        pProgram = nullptr;
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
        PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)m_pCmDev->GetAccelData();
        PCM_HAL_STATE pState = pCmData->cmHalState;
        if (pState->bDynamicStateHeap)
        {
            pState->pfnDSHUnregisterKernel(pState, m_Id);
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
CmKernelRT::CmKernelRT(CmDeviceRT *pCmDev,
                       CmProgramRT *pProgram,
                       uint32_t KernelIndex,
                       uint32_t KernelSeqNum):
    m_pCmDev( pCmDev ),
    m_pSurfaceMgr( nullptr ),
    m_pProgram( pProgram ),
    m_Options( nullptr ),
    m_pBinary( nullptr ),
    m_uiBinarySize(0),
    m_ThreadCount( 0 ),
    m_LastThreadCount( 0 ),
    m_SizeInCurbe( 0 ),
    m_SizeInPayload( 0 ),
    m_ArgCount( 0 ),
    m_Args( nullptr ),
    m_pKernelInfo(nullptr),
    m_kernelIndexInProgram( CM_INVALID_KERNEL_INDEX ),
    m_CurbeEnable( true ),
    m_NonstallingScoreboardEnable(false),
    m_Dirty( CM_KERNEL_DATA_CLEAN ),
    m_pLastKernelData( nullptr ),
    m_LastKernelDataSize( 0 ),
    m_IndexInTask(0),
    m_AssociatedToTS(false),
    m_blPerThreadArgExists(false),
    m_blPerKernelArgExists( false ),
    m_pThreadSpace( nullptr ),
    m_adjustScoreboardY( 0 ),
    m_LastAdjustScoreboardY( 0 ),
    m_blCreatingGPUCopyKernel( false),
    m_usKernelPayloadDataSize( 0 ),
    m_pKernelPayloadData( nullptr ),
    m_usKernelPayloadSurfaceCount( 0 ),
    m_refcount(0),
    m_pHalMaxValues( nullptr ),
    m_pHalMaxValuesEx( nullptr ),
    m_SurfaceArray(nullptr),
    m_pThreadGroupSpace( nullptr ),
    m_VMESurfaceCount( 0 ),
    m_MaxSurfaceIndexAllocated(0),
    m_BarrierMode(CM_LOCAL_BARRIER),
    m_SamplerBTICount( 0 ),
    m_IsClonedKernel(false),
    m_CloneKernelID(0),
    m_HasClones( false ),
    m_state_buffer_bounded( CM_STATE_BUFFER_NONE ),
    m_pBinaryOrig(nullptr),
    m_uiBinarySizeOrig(0)
{
    pProgram->Acquire();
    m_pProgram = pProgram;

    pCmDev->GetSurfaceManager(m_pSurfaceMgr);

    m_Id = KernelSeqNum; // Unique number for each kernel. This ID is used in Batch buffer.
    m_Id <<= 32;
    m_kernelIndex = KernelIndex;

    for (int i = 0; i < CM_GLOBAL_SURFACE_NUMBER; i++)
    {
        m_GlobalSurfaces[i] = nullptr;
        m_GlobalCmIndex[i] = 0;
    }

    m_blhwDebugEnable = pProgram->IsHwDebugEnabled();

    CmSafeMemSet(m_pKernelPayloadSurfaceArray, 0, sizeof(m_pKernelPayloadSurfaceArray));
    CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, sizeof(m_IndirectSurfaceInfoArray));
    CmSafeMemSet( m_SamplerBTIEntry, 0, sizeof( m_SamplerBTIEntry ) );

    if (m_SamplerBTICount > 0)
    {
        CmSafeMemSet(m_SamplerBTIEntry, 0, sizeof(m_SamplerBTIEntry));
        m_SamplerBTICount = 0;
    }

    ResetKernelSurfaces();
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Class CmKernel
//| Returns:    None.
//*-----------------------------------------------------------------------------
CmKernelRT::~CmKernelRT( void )
{
    MosSafeDeleteArray(m_Options);

    DestroyArgs();

    if(m_pLastKernelData)
    {
        CmKernelData::Destroy( m_pLastKernelData );
    }

    if( m_pCmDev->CheckGTPinEnabled() && !m_blCreatingGPUCopyKernel)
    {
        MosSafeDeleteArray(m_pBinary);
    }

    MosSafeDeleteArray(m_pBinaryOrig);

    if( CM_INVALID_KERNEL_INDEX != m_kernelIndexInProgram )
    {
        m_pProgram->ReleaseKernelInfo(m_kernelIndexInProgram);
    }

    for(int i=0; i< CM_GLOBAL_SURFACE_NUMBER; i++)
    {
        SurfaceIndex *pSurfIndex = m_GlobalSurfaces[i];
        MosSafeDelete(pSurfIndex);
    }

    MosSafeDeleteArray(m_pKernelPayloadData);
    MosSafeDeleteArray(m_SurfaceArray);
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
    m_pProgram->GetKernelCount( kernelCount );

    CM_KERNEL_INFO* kernelInfo = nullptr;
    uint32_t i = 0;
    for( i = 0; i < kernelCount; i ++ )
    {
        m_pProgram->GetKernelInfo( i, kernelInfo );
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

    m_pCmDev->GetHalMaxValues( m_pHalMaxValues, m_pHalMaxValuesEx);

    m_pProgram->AcquireKernelInfo(i);
    m_pKernelInfo = kernelInfo;
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
            m_Options = MOS_NewArray(char, (length+1));
            if( !m_Options )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;

            }
            CmFastMemCopy( m_Options, options, length);
            m_Options[ length ] = '\0';

            char* pTmp = strstr( m_Options, "nocurbe" );
            if( pTmp )
            {
                m_CurbeEnable = false;
            }
        }
    }


    m_NonstallingScoreboardEnable = true;

    void* pCommonISACode = nullptr;
    uint32_t commonISACodeSize = 0;
    m_pProgram->GetCommonISACode(pCommonISACode, commonISACodeSize);
    if ((pCommonISACode == nullptr) || (commonISACodeSize <= 0))
    {
        CM_ASSERTMESSAGE("Error: Invalid VISA.");
        return CM_INVALID_COMMON_ISA;
    }

    bool bUseVisaApi = true;
    vISA::ISAfile *isaFile = nullptr;
    vISA::KernelBody *kernelBody = nullptr;

    auto getVersionAsInt = [](int major, int minor) { return major * 100 + minor; };
    if (getVersionAsInt(m_pProgram->m_CISA_majorVersion, m_pProgram->m_CISA_minorVersion) < getVersionAsInt(3, 2))
    {
        bUseVisaApi = false;
    }
    else
    {
        isaFile = m_pProgram->getISAfile();
        if (!isaFile)
        {
            CM_ASSERTMESSAGE("Error: Invalid VISA.");
            return CM_INVALID_COMMON_ISA;
        }
        kernelBody = isaFile->getKernelsData().at(m_kernelIndexInProgram);
    }

    uint8_t *buf = (uint8_t*)pCommonISACode;
    uint32_t byte_pos = m_pKernelInfo->kernelIsaOffset;

    uint32_t kernelInfoRefCount = 0;
    m_pProgram->GetKernelInfoRefCount(m_kernelIndexInProgram, kernelInfoRefCount);
    if (kernelInfoRefCount <= 2)    //Only read for 1st time Kernel creation, later we reuse them
    {
        if (bUseVisaApi)
        {
            m_pKernelInfo->globalStringCount = kernelBody->getStringCount();
        }
        {
            READ_FIELD_FROM_BUF(m_pKernelInfo->globalStringCount, unsigned short);
        }

        m_pKernelInfo->globalStrings = (const char**) malloc( m_pKernelInfo->globalStringCount * sizeof(char*) );
        if(m_pKernelInfo->globalStrings  == nullptr)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
        CmSafeMemSet(m_pKernelInfo->globalStrings, 0, m_pKernelInfo->globalStringCount * sizeof(char*) );

        if (bUseVisaApi)
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
                CmFastMemCopy(string, globalString->getString(), stringLength);
                string[stringLength] = '\0';
                m_pKernelInfo->globalStrings[i] = string;
                i++;
            }
        }
        else
        {
            for (int i = 0; i < (int)m_pKernelInfo->globalStringCount; i++)
            {
                char* string = (char*)malloc(CM_MAX_KERNEL_STRING_IN_BYTE + 1);
                if (string == nullptr)
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    return CM_OUT_OF_HOST_MEMORY;
                }
                int j = 0;
                while (buf[byte_pos] != '\0' && j < CM_MAX_KERNEL_STRING_IN_BYTE) {
                    string[j++] = buf[byte_pos++];
                }
                string[j] = '\0';
                byte_pos++;
                m_pKernelInfo->globalStrings[i] = string;
            }
        }
    }

    uint32_t count = 0;
    if (bUseVisaApi)
    {
        count = kernelBody->getNumInputs();
    }
    else
    {
        byte_pos = m_pKernelInfo->inputCountOffset;

        uint8_t countTemp = 0;
        READ_FIELD_FROM_BUF(countTemp, uint8_t);
        count = countTemp;
    }

    if( count > m_pHalMaxValues->maxArgsPerKernel )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg count.");
        return CM_EXCEED_KERNEL_ARG_AMOUNT;
    }

    m_Args = MOS_NewArray(CM_ARG, count);
    if( (!m_Args) && (count != 0) )
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        MosSafeDeleteArray(m_Options);
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet(m_Args, 0, sizeof(CM_ARG) * count);
    m_ArgCount  = count;

    uint32_t preDefinedSurfNum;
    if ( (m_pProgram->m_CISA_majorVersion > 3) || ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion >=1)) )  //CISA 3.1 +
    {
        preDefinedSurfNum = COMMON_ISA_NUM_PREDEFINED_SURF_VER_3_1;
    }
    else if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion == 0))
    {
        preDefinedSurfNum = COMMON_ISA_NUM_PREDEFINED_SURF_VER_2_1;
    }
    else //CISA 2.0
    {
        preDefinedSurfNum = COMMON_ISA_NUM_PREDEFINED_SURF_VER_2;
    }

    uint32_t argSize = 0;

    for (uint32_t i = 0; i < m_ArgCount; i++)
    {
        vISA::InputInfo *inputInfo = nullptr;
        uint8_t kind = 0;

        if (bUseVisaApi)
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
            m_Args[i].bIsSet = true;
            m_Args[i].unitCount = 1;
        }
        else if (kind == 0x10) {
            kind = ARG_KIND_IMPLICT_GROUPSIZE;
            m_Args[i].bIsSet = true;
            m_Args[i].unitCount = 1;
        }
        else if (kind == 0x18) {
            kind = ARG_KIND_IMPLICIT_LOCALID;
            m_Args[i].bIsSet = true;
            m_Args[i].unitCount = 1;
            m_blPerKernelArgExists = true;  //only VISA3.3+, can come here, so, no matter it is there any explicit arg, implicit arg exits
        }
        else if (kind == 0x2A) {
            kind = ARG_KIND_SURFACE_2D_SCOREBOARD;
        }
        else if (kind == 0x20) {
            kind = ARG_KIND_GENERAL_DEPVEC;
        }

        m_Args[i].unitKind = kind;
        m_Args[i].unitKindOrig = kind;

        if (kind == ARG_KIND_SURFACE && m_pKernelInfo->surface_count)
        {
            m_Args[i].s_k = DATA_PORT_SURF;
        }

        if (bUseVisaApi)
        {
            m_Args[i].unitOffsetInPayload = inputInfo->getOffset();
            m_Args[i].unitOffsetInPayloadOrig = inputInfo->getOffset();

            m_Args[i].unitSize = inputInfo->getSize();
            m_Args[i].unitSizeOrig = inputInfo->getSize();
        }
        else
        {
            uint32_t varID;
            READ_FIELD_FROM_BUF(varID, uint16_t);

            uint16_t tmpW;
            READ_FIELD_FROM_BUF(tmpW, uint16_t);
            m_Args[i].unitOffsetInPayload = tmpW;
            m_Args[i].unitOffsetInPayloadOrig = tmpW;

            READ_FIELD_FROM_BUF(tmpW, uint16_t);
            m_Args[i].unitSize = tmpW;
            m_Args[i].unitSizeOrig = tmpW;
        }

        argSize += m_Args[i].unitSize;
    }
    //////////////////////////////////////////////////////////////////////////

    if (kernelInfoRefCount <= 2)    //Only read for 1st time Kernel creation, later we reuse them
    {
        uint16_t attribute_count = 0;
        if (bUseVisaApi)
        {
            attribute_count = kernelBody->getAttributeCount();
        }
        else
        {
            /////////////////////////////////////////////////////////////////////////
            // Get pre-defined kernel attributes, Start
            //skipping size and entry
            byte_pos += 8;

            READ_FIELD_FROM_BUF(attribute_count, uint16_t);
        }

        for (int i = 0; i < attribute_count; i++)
        {
            vISA::AttributeInfo *attribute = nullptr;
            uint32_t name_index = 0;
            uint8_t size = 0;

            if (bUseVisaApi)
            {
                attribute = kernelBody->getAttributeInfo()[i];
                name_index = attribute->getName();
                size = attribute->getSize();
            }
            else
            {
                READ_FIELD_FROM_BUF(name_index, uint16_t);
                READ_FIELD_FROM_BUF(size, uint8_t);
            }

            if( strcmp( m_pKernelInfo->globalStrings[name_index], "AsmName" ) == 0 )
            {
                if (bUseVisaApi)
                {
                    CmFastMemCopy(m_pKernelInfo->kernelASMName, attribute->getValue(), size);
                }
                else
                {
                    CmFastMemCopy(m_pKernelInfo->kernelASMName, &buf[byte_pos], size);
                    byte_pos += size;
                }
            }
            else if (strcmp( m_pKernelInfo->globalStrings[name_index], "SLMSize" ) == 0)
            {
                if (bUseVisaApi)
                {
                    m_pKernelInfo->kernelSLMSize = attribute->getValue()[0];
                }
                else
                {
                    READ_FIELD_FROM_BUF(m_pKernelInfo->kernelSLMSize, uint8_t);
                }

                /* Notes by Stony@2014-04-09
                 * <=CISA3.1: the size is number of 4KB
                 * > CISA3.1: the size is number of 1KB
                 * Here convert it to the number of 1KB if <=CISA 3.1
                 */
                if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion <= 1))
                {
                    m_pKernelInfo->kernelSLMSize = m_pKernelInfo->kernelSLMSize * 4;
                }

                // align to power of 2
                uint32_t v = m_pKernelInfo->kernelSLMSize;
                v--;
                v |= v >> 1;
                v |= v >> 2;
                v |= v >> 4;
                v |= v >> 8;
                v |= v >> 16;
                v++;
                m_pKernelInfo->kernelSLMSize = ( uint8_t )v;
            }
            else if (strcmp(m_pKernelInfo->globalStrings[name_index], "NoBarrier") == 0)
            {
                m_pKernelInfo->blNoBarrier = true;
                if (!bUseVisaApi)
                {
                    byte_pos += size;
                }
            }
            else
            {
                if (!bUseVisaApi)
                {
                    byte_pos += size;
                }
            }
        }
    }

    if(argSize > m_pHalMaxValues->maxArgByteSizePerKernel)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
        return CM_EXCEED_KERNEL_ARG_SIZE_IN_BYTE;
    }

    buf = (uint8_t*)pCommonISACode;

    if(m_pProgram->IsJitterEnabled())
    {
        //m_JitterEnable = true;
        char *programOptions;
        m_pProgram->GetKernelOptions(programOptions);
        //if no options or same options, copy load program's binary. else re-jitter
        {
            m_pBinary = (char *)m_pKernelInfo->jitBinaryCode;
            m_uiBinarySize = m_pKernelInfo->jitBinarySize;
            m_pKernelInfo->pOrigBinary = m_pKernelInfo->jitBinaryCode;
            m_pKernelInfo->uiOrigBinarySize = m_pKernelInfo->jitBinarySize;
        }
    }
    else
    {
        char* pBinary = (char*)(buf + m_pKernelInfo->genxBinaryOffset );

        //No copy, point to the binary offset in CISA code.
        m_pBinary = pBinary;
        m_uiBinarySize = m_pKernelInfo->genxBinarySize;

        m_pKernelInfo->pOrigBinary = pBinary;
        m_pKernelInfo->uiOrigBinarySize = m_pKernelInfo->genxBinarySize;
    }

    if (m_pKernelInfo->blNoBarrier)
    {
        m_BarrierMode = CM_NO_BARRIER;
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

    if (m_pThreadSpace == nullptr)
    {
        if (m_ThreadCount)
        {
            // Setting threadCount twice with different values will cause reset of kernels
            if (m_ThreadCount != count)
            {
                Reset();
                m_ThreadCount = count;
                m_Dirty |= CM_KERNEL_DATA_THREAD_COUNT_DIRTY;
            }
        }
        else // first time
        {
            m_ThreadCount = count;
        }
    }
    return CM_SUCCESS;
}

int32_t CmKernelRT::GetThreadCount(uint32_t& count )
{
    count = m_ThreadCount;
    return CM_SUCCESS;
}

int32_t CmKernelRT::GetKernelSurfaces(bool  *&surfArray)
{
    surfArray = m_SurfaceArray;
    return CM_SUCCESS;
}

int32_t CmKernelRT::ResetKernelSurfaces()
{
    uint32_t surfacePoolSize = m_pSurfaceMgr->GetSurfacePoolSize();
    if (!m_SurfaceArray)
    {
        m_SurfaceArray = MOS_NewArray(bool, surfacePoolSize);
        if (!m_SurfaceArray)
        {
            CM_ASSERTMESSAGE("Error: Failed to rest kernel surfaces due to out of system memory.");
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    CmSafeMemSet( m_SurfaceArray, 0, surfacePoolSize * sizeof( bool ) );

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get CmSurface from surface manager.
//|             Use "pValue + IndexSurfaceArray" to locate its SurfaceIndex
//| Returns:    CmSurface. Null if not found
//*-----------------------------------------------------------------------------
CmSurface* CmKernelRT::GetSurfaceFromSurfaceArray( SurfaceIndex* pValue, uint32_t IndexSurfaceArray)
{
    int32_t hr                          = CM_SUCCESS;
    CmSurface *pCmSurface           = nullptr;
    SurfaceIndex* pSurfaceIndex     = nullptr;

    pSurfaceIndex = pValue + IndexSurfaceArray;
    CMCHK_NULL(pSurfaceIndex);

    if (pSurfaceIndex->get_data() == CM_NULL_SURFACE
        || pSurfaceIndex->get_data() == 0)
    {
        pCmSurface = (CmSurface *)CM_NULL_SURFACE;
        goto finish;
    }

    m_pSurfaceMgr->GetSurface(pSurfaceIndex->get_data(), pCmSurface);

finish:
    if(hr != CM_SUCCESS)
    {
        pCmSurface = nullptr;
    }

    return pCmSurface;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set kernel arg for single vme surface or multiple vme surfaces
//|             in surface array. So far, don't support vme surface array in thread arg.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsVme(CM_KERNEL_INTERNAL_ARG_TYPE nArgType, uint32_t ArgIndex, const void *pValue, uint32_t nThreadID)
{
    uint32_t NumofElements = 0;
    CM_ARG& arg        = m_Args[ ArgIndex ];
    uint32_t TotalVmeArgValueSize       = 0;
    uint32_t TotalSurfacesInVme         = 0;
    uint32_t TempVmeArgValueSize        = 0;
    uint32_t VmeArgValueOffset          = 0;
    uint32_t LastVmeSurfCount           = 0;
    CmSurfaceVme* pSurfVme          = nullptr;
    uint8_t *pVmeArgValueArray         = nullptr;
    uint16_t *pVmeCmIndexArray          = nullptr;
    int32_t hr = CM_SUCCESS;

    //Get Number of elements in surface array
    if (arg.unitVmeArraySize == 0)
    {  //First Time
        NumofElements = arg.unitSize / sizeof(uint32_t);
    }
    else
    {
        NumofElements = arg.unitVmeArraySize;
    }

    //Get Size of pVmeIndexArray and pVmeCmIndexArray.
    for(uint32_t i=0; i< NumofElements; i++)
    {
        if (((SurfaceIndex*)(pValue)+i)->get_data() == 0 || ((SurfaceIndex*)(pValue)+i)->get_data() == CM_NULL_SURFACE)
        {
            TempVmeArgValueSize = sizeof(CM_HAL_VME_ARG_VALUE);
            TotalVmeArgValueSize += TempVmeArgValueSize;
            TotalSurfacesInVme++;
        }
        else
        {
            pSurfVme = static_cast<CmSurfaceVme*>(GetSurfaceFromSurfaceArray((SurfaceIndex*)pValue, i));
            CMCHK_NULL(pSurfVme);
            TempVmeArgValueSize = pSurfVme->GetVmeCmArgSize();
            TotalVmeArgValueSize += TempVmeArgValueSize;
            TotalSurfacesInVme += pSurfVme->GetTotalSurfacesCount();
        }
    }

    // Allocate and Zero Memory for arg.pValue and arg.surfIndex
    // arg.pValue    : an array of CM_HAL_VME_ARG_VALUE structure followed by an array of reference surfaces
    // arg.surfIndex : an array listing all the Cm surface indexes, in the order of current, fw surfaces, bw surfaces

    if (arg.unitSize < TotalVmeArgValueSize) // need to re-allocate larger area)
    {
        if (arg.pValue) 
        {
            MosSafeDeleteArray(arg.pValue);
        }
        arg.pValue = MOS_NewArray(uint8_t, TotalVmeArgValueSize);
        
        if (arg.surfIndex) 
        {
            MosSafeDeleteArray(arg.surfIndex);
        }
        arg.surfIndex = MOS_NewArray(uint16_t, TotalSurfacesInVme);
    }
    
    CMCHK_NULL(arg.pValue);
    CmSafeMemSet(arg.pValue, 0, TotalVmeArgValueSize);
    CMCHK_NULL(arg.surfIndex);
    CmSafeMemSet(arg.surfIndex, 0, TotalSurfacesInVme * sizeof(uint16_t));

    //Set each Vme Surface
    for (uint32_t i = 0; i< NumofElements; i++)
    {
        if (((SurfaceIndex*)(pValue)+i)->get_data() == 0 || ((SurfaceIndex*)(pValue)+i)->get_data() == CM_NULL_SURFACE)
        {
            PCM_HAL_VME_ARG_VALUE pVmeArg = (PCM_HAL_VME_ARG_VALUE)(arg.pValue + VmeArgValueOffset);
            pVmeArg->fwRefNum = 0;
            pVmeArg->bwRefNum = 0;
            pVmeArg->curSurface = CM_NULL_SURFACE;
            TempVmeArgValueSize = sizeof(CM_HAL_VME_ARG_VALUE);
            VmeArgValueOffset += TempVmeArgValueSize;
            arg.surfIndex[LastVmeSurfCount] = CM_NULL_SURFACE;
            LastVmeSurfCount++;
        }
        else
        {
            pSurfVme = static_cast<CmSurfaceVme*>(GetSurfaceFromSurfaceArray((SurfaceIndex*)pValue, i));
            CMCHK_NULL(pSurfVme);
            SetArgsSingleVme(pSurfVme, arg.pValue + VmeArgValueOffset, arg.surfIndex + LastVmeSurfCount);
            TempVmeArgValueSize = pSurfVme->GetVmeCmArgSize();
            VmeArgValueOffset += TempVmeArgValueSize;
            LastVmeSurfCount += pSurfVme->GetTotalSurfacesCount();
        }
    }

    if ( nArgType == CM_KERNEL_INTERNEL_ARG_PERKERNEL ) // per kernel arg
    {
        // First time set
        if( !arg.pValue )
        {   // Increment size kernel arguments will take up in CURBE
            m_SizeInCurbe += CM_ARGUMENT_SURFACE_SIZE * NumofElements;
        }

        arg.unitCount = 1;
        arg.bIsDirty  = true;
        arg.bIsSet    = true;
        arg.unitKind  = ARG_KIND_SURFACE_VME;
        arg.unitSize = (uint16_t)TotalVmeArgValueSize; // the unitSize can't represent surfaces count here
        arg.unitVmeArraySize = NumofElements;

        m_Dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
        m_blPerKernelArgExists = true;
    }
    else
    {
        // Thread Arg doesn't support VME surfaces as it is rarely used and it is complex to implement,
        // since each thread may has different surface number in its vme surface argment.
        hr = CM_THREAD_ARG_NOT_ALLOWED;
    }

finish:
    if(hr != CM_SUCCESS)
    {
        MosSafeDeleteArray(arg.pValue);
        MosSafeDeleteArray(arg.surfIndex);
    }
    return hr;

}


//*-----------------------------------------------------------------------------
//| Purpose:    Fill arg for a single vme surface.
//|             pVmeIndexArray points to arg.pValue
//|             pVmeCmIndexArray points to arg.surfIndex
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsSingleVme(CmSurfaceVme* pSurfVme, uint8_t *pVmeArgValueArray, uint16_t *pCmSufacesArray)
{

    int32_t hr = CM_SUCCESS;
    CM_SURFACE_MEM_OBJ_CTRL memCtl;
    uint32_t VmeBackwardSurfaceCount        = 0;
    uint32_t VmeForwardSurfaceCount         = 0;
    uint32_t VmeCurrentSurfaceIndex         = 0;
    uint16_t VmeCurrentCmIndex              = 0;
    int32_t VmeIndexArrayPosition          = 0; // Offset for pVmeIndexArray
    int32_t VmeCmIndexArrayPosition        = 0; // Offset for pVmeCmIndexArray
    uint32_t tempOutput                     = 0;
    uint32_t CmSurfArrayIdx                 = 0;
    uint32_t surfStateWidth                 = 0;
    uint32_t surfStateHeight                = 0;

    uint32_t *f_array       = nullptr;
    uint32_t *b_array       = nullptr;
    uint32_t *f_CmIndex     = nullptr;
    uint32_t *b_CmIndex     = nullptr;

    uint32_t *fwSurfInArg = nullptr;
    uint32_t *bwSurfInArg = nullptr;

    CmSurface *pSurface = nullptr;
    PCM_HAL_VME_ARG_VALUE pVmeArg = (PCM_HAL_VME_ARG_VALUE)pVmeArgValueArray;

    CMCHK_NULL(pSurfVme);
    CMCHK_NULL(pVmeArg);
    CMCHK_NULL(pCmSufacesArray);

    if(pSurfVme == (CmSurfaceVme *)CM_NULL_SURFACE)
    {
        pVmeArg->fwRefNum = 0;
        pVmeArg->bwRefNum = 0;
        pVmeArg->curSurface = CM_NULL_SURFACE;
        pCmSufacesArray[CmSurfArrayIdx] =  CM_NULL_SURFACE;
        return hr;
    }

    // Get Vme Backward Forward Surface Count
    pSurfVme->GetIndexBackwardCount(VmeBackwardSurfaceCount);
    pSurfVme->GetIndexForwardCount(VmeForwardSurfaceCount);

    pVmeArg->fwRefNum = VmeForwardSurfaceCount;
    pVmeArg->bwRefNum = VmeBackwardSurfaceCount; // these two numbers must be set before any other operations

    pSurfVme->GetSurfaceStateResolution(pVmeArg->surfStateParam.iSurfaceStateWidth, pVmeArg->surfStateParam.iSurfaceStateHeight);

    pSurfVme->GetIndexForwardArray(f_array);
    pSurfVme->GetIndexBackwardArray(b_array);
    pSurfVme->GetIndexCurrent(VmeCurrentSurfaceIndex);

    pSurfVme->GetCmIndexCurrent(VmeCurrentCmIndex);
    pSurfVme->GetCmIndexForwardArray(f_CmIndex);
    pSurfVme->GetCmIndexBackwardArray(b_CmIndex);

    pCmSufacesArray[CmSurfArrayIdx++] = VmeCurrentCmIndex;

    // Set Current Vme Surface
    m_pSurfaceMgr->GetSurface(VmeCurrentCmIndex, pSurface);
    CMCHK_NULL(pSurface);

    pVmeArg->curSurface = VmeCurrentSurfaceIndex;

    //Set Forward Vme Surfaces
    fwSurfInArg = findFwRefInVmeArg(pVmeArg);
    for (uint32_t i = 0; i < VmeForwardSurfaceCount; i++)
    {
        GetVmeSurfaceIndex( f_array, f_CmIndex, i, &tempOutput);
        fwSurfInArg[i] = tempOutput;
        pCmSufacesArray[CmSurfArrayIdx++] = (uint16_t)f_CmIndex[i];
    }

    //Set Backward Vme Surfaces
    bwSurfInArg = findBwRefInVmeArg(pVmeArg);
    for (uint32_t i = 0; i < VmeBackwardSurfaceCount; i++)
    {
        GetVmeSurfaceIndex( b_array, b_CmIndex, i, &tempOutput);
        bwSurfInArg[i] = tempOutput;
        pCmSufacesArray[CmSurfArrayIdx++] = (uint16_t)b_CmIndex[i];
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
    uint32_t *pVmeIndexArray,
    uint32_t *pVmeCmIndexArray,
    uint32_t index,
    uint32_t *pOutputValue)
{
    int32_t hr = CM_SUCCESS;
    uint32_t value = pVmeIndexArray[index];

    if (pVmeIndexArray[index] == CM_INVALID_VME_SURFACE)
    {
        value = CM_NULL_SURFACE;
    }

    *pOutputValue = value;

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
//!             7) Value of surface handle combined with memory object control
//!             8) Original surface index for each surface in array
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetArgsInternalSurfArray(
    int32_t offset,uint32_t kernelArgIndex,
    int32_t surfCount, CmSurface* pCurrentSurface,
    uint32_t currentSurfIndex, SurfaceIndex* pValue,
    uint32_t surfValue[], uint16_t origSurfIndex[])
{
    CM_SURFACE_MEM_OBJ_CTRL memCtl;
    uint32_t                surfRegTableIndex = 0;
    uint32_t                handle = 0;
    uint32_t                samplerIndex;
    uint16_t                samplerCmIndex;
    uint32_t                surfaceArraySize = 0;

    m_pSurfaceMgr->GetSurfaceArraySize(surfaceArraySize);
    MosSafeDeleteArray(m_Args[kernelArgIndex].pSurfArrayArg); // delete it if it was allcated
    m_Args[kernelArgIndex].pSurfArrayArg = MOS_NewArray(SURFACE_ARRAY_ARG, surfCount);
    if (!m_Args[kernelArgIndex].pSurfArrayArg)
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        return CM_OUT_OF_HOST_MEMORY;
    }
    CmSafeMemSet((void *)m_Args[kernelArgIndex].pSurfArrayArg, 0,  sizeof(SURFACE_ARRAY_ARG) * surfCount);
    while (offset < surfCount)
    {
        switch (pCurrentSurface->Type())
        {
          case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
          {
             CmSurface2DRT* pSurf2D = static_cast<CmSurface2DRT*>(pCurrentSurface);

             uint32_t numAliases = 0;
             pSurf2D->GetNumAliases(numAliases);
             if (numAliases)
             {
                 m_Args[kernelArgIndex].bAliasCreated = true;
             }
             else
             {
                 m_Args[kernelArgIndex].bAliasCreated = false;
             }

             //set memory object control
             pSurf2D->GetIndexFor2D(surfRegTableIndex);

             surfValue[offset] = surfRegTableIndex;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_2D;
             m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_2D;

             break;
         }
         case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
         {
             CmBuffer_RT* pSurf1D = static_cast<CmBuffer_RT*>(pCurrentSurface);

             uint32_t numAliases = 0;
             pSurf1D->GetNumAliases(numAliases);
             if (numAliases)
             {
                 m_Args[kernelArgIndex].bAliasCreated = true;
             }
             else
             {
                 m_Args[kernelArgIndex].bAliasCreated = false;
             }

             //set memory object control
             pSurf1D->GetHandle(handle);

             surfValue[offset] = handle;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_1D;
             m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_1D;
             break;
         }
         case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
         {
             CmSurface2DUPRT* pSurf2DUP = static_cast<CmSurface2DUPRT*>(pCurrentSurface);

             //set memory object
             pSurf2DUP->GetHandle(handle);

             surfValue[offset] = handle;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_2D_UP;
             m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_2D_UP;
             break;
         }
         case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
         {
             CmSurface3DRT* pSurf3D = static_cast<CmSurface3DRT*>(pCurrentSurface);

             pSurf3D->GetHandle(handle);

             surfValue[offset] = handle;
             origSurfIndex[offset] = (uint16_t)currentSurfIndex;

             m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_3D;
             m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_3D;

             break;
         }

         case CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER:
         {
             CmStateBuffer* pStateBuffer = static_cast< CmStateBuffer* >( pCurrentSurface );
             pStateBuffer->GetHandle( handle );

             surfValue[ offset ] = handle;
             origSurfIndex[ offset ] = ( uint16_t )currentSurfIndex;

             m_Args[ kernelArgIndex ].pSurfArrayArg[ offset ].argKindForArray = ARG_KIND_STATE_BUFFER;
             m_Args[ kernelArgIndex ].unitKind = ARG_KIND_STATE_BUFFER;

             break;
         }

         //sampler surface
         case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
         {
             CmSurfaceSampler* pSurfSampler = static_cast <CmSurfaceSampler *> (pCurrentSurface);
             pSurfSampler->GetHandle(samplerIndex);
             pSurfSampler->GetCmIndexCurrent(samplerCmIndex);

             m_pSurfaceMgr->GetSurface(samplerCmIndex, pCurrentSurface);
             if (!pCurrentSurface)
             {
                 CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
                 return CM_NULL_POINTER;
             }

             surfValue[offset] = samplerIndex;
             origSurfIndex[offset] = (uint16_t)samplerCmIndex;

             SAMPLER_SURFACE_TYPE type;
             pSurfSampler->GetSurfaceType(type);
             if (type == SAMPLER_SURFACE_TYPE_2D)
             {
                 m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_SAMPLER;
                 m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_SAMPLER;
             }
             else if (type == SAMPLER_SURFACE_TYPE_2DUP)
             {
                 m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE2DUP_SAMPLER;
                 m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE2DUP_SAMPLER;
             }
             else if(type == SAMPLER_SURFACE_TYPE_3D)
             {
                 m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_3D;
                 m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_3D;
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
             CmSurfaceSampler8x8* pSurfSampler8x8 = static_cast <CmSurfaceSampler8x8 *> (pCurrentSurface);
             pSurfSampler8x8->GetIndexCurrent(samplerIndex);
             pSurfSampler8x8->GetCmIndex(samplerCmIndex);

             m_pSurfaceMgr->GetSurface(samplerCmIndex, pCurrentSurface);
             if (!pCurrentSurface)
             {
                 CM_ASSERTMESSAGE("Error: Pointer to current surface is null.");
                 return CM_FAILURE;
             }

             surfValue[offset] = samplerIndex;
             origSurfIndex[offset] = (uint16_t)samplerCmIndex;

             CM_SAMPLER8x8_SURFACE type;
             type = pSurfSampler8x8->GetSampler8x8SurfaceType();
             if (type == CM_VA_SURFACE)
             {
                 m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_VA;
                 m_Args[kernelArgIndex].pSurfArrayArg[offset].addressModeForArray = pSurfSampler8x8->GetAddressControlMode();
                 m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_SAMPLER8X8_VA;
             }
             else if(type == CM_AVS_SURFACE)
             {
                 m_Args[kernelArgIndex].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
                 m_Args[kernelArgIndex].pSurfArrayArg[offset].argKindForArray = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
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
           currentSurfIndex = pValue[offset].get_data();

           while ((!currentSurfIndex && (offset < surfCount))||(currentSurfIndex == CM_NULL_SURFACE))
           {
               surfValue[offset] = CM_NULL_SURFACE;
               origSurfIndex[offset] = 0;
               offset++;
               if (offset >= surfCount)
                   break;
               currentSurfIndex = pValue[offset].get_data();
           }

           if (currentSurfIndex > surfaceArraySize)
           {
               currentSurfIndex = currentSurfIndex % surfaceArraySize;
           }
       }
       if (offset < surfCount)
       {
           m_pSurfaceMgr->GetSurface(currentSurfIndex, pCurrentSurface);
           if (nullptr == pCurrentSurface)
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
int32_t CmKernelRT::SetArgsInternal( CM_KERNEL_INTERNAL_ARG_TYPE nArgType, uint32_t index, size_t size, const void *pValue, uint32_t nThreadID )
{
    uint32_t surfRegTableIndex = 0; // for 2D surf
    uint32_t handle = 0; // for 1D surf

    uint32_t samplerIndex;
    uint16_t samplerCmIndex;
    uint32_t samplerIdx = 0;
    uint32_t vmeIdx = 0;
    uint16_t *pSurfIndexValue =  nullptr;
    uint32_t surfaces[CM_MAX_ARGS_PER_KERNEL];
    uint16_t surfIndexArray[CM_MAX_ARGS_PER_KERNEL];
    std::vector< int > sampler_index_array;

    //Clear "set" flag in case user call API to set the same one argument multiple times.
    m_Args[index].bIsSet = false;
    if( m_Args[ index ].unitKind == ARG_KIND_GENERAL || (m_Args[index].unitKind == ARG_KIND_GENERAL_DEPVEC))
    {
        if( size != m_Args[ index ].unitSize )
        {
            CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
            return CM_INVALID_ARG_SIZE;
        }
    }
    //For surface type
    else if (CHECK_SURFACE_TYPE(m_Args[index].unitKind,
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

        // this code is to convert surfaceIndex object to index of type uint32_t,
        // which is expected by commonISA/genBinary
        // index is the index of the surface in surface registration table of CM device
        // in driver

        int signature_size = m_Args[index].unitSize;
        int num_surfaces = signature_size / sizeof(int);
        SurfaceIndex* pSurfIndex = (SurfaceIndex*)pValue;
        if (pSurfIndex == (SurfaceIndex*)CM_NULL_SURFACE)
        {
            m_Args[index].bIsSet = true;
            m_Args[index].unitCount = 1; // per kernel arg
            m_Dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
            m_blPerKernelArgExists = true;
            m_Args[index].bIsDirty = true;
            m_Args[index].bIsNull = true;
            return CM_SUCCESS;
        }
        m_Args[index].bIsNull = false;
        CM_SURFACE_MEM_OBJ_CTRL memCtl;

        if (m_Args[index].unitKind != ARG_KIND_SURFACE_VME)
        {
            if (size != sizeof(SurfaceIndex)* num_surfaces)
            {
                CM_ASSERTMESSAGE("Error: Invalid kernel arg size.");
                return CM_INVALID_ARG_SIZE;
            }
        }

        uint32_t surfIndex = pSurfIndex->get_data();
        int i = 0;
        uint32_t surfaceArraySize = 0;
        m_pSurfaceMgr->GetSurfaceArraySize(surfaceArraySize);

        if (surfIndex > surfaceArraySize)
        {
            if (m_Args[index].aliasIndex != surfIndex)
            {
                m_Args[index].bIsDirty = true;
                m_Dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
                m_Args[index].aliasIndex = surfIndex;
            }

            surfIndex = surfIndex % surfaceArraySize;
        }
        else
        {
            m_Args[index].aliasIndex = 0;
        }

        while (!surfIndex && (i < num_surfaces))
        {
            surfaces[i] = CM_NULL_SURFACE;
            surfIndexArray[i] = 0;
            i++;
            if (i >= num_surfaces)
                break;
            surfIndex = pSurfIndex[i].get_data();
        }

        if (i >= num_surfaces)
        {
            m_Args[index].unitKind = ARG_KIND_SURFACE_2D;
            pValue = surfaces;
            size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
            m_Args[index].unitSize = (uint16_t)size;
            goto finish;
        }
        CmSurface* pSurface = nullptr;
        m_pSurfaceMgr->GetSurface(surfIndex, pSurface);
        if (nullptr == pSurface)
        {
            CM_ASSERTMESSAGE("Error: Invalid surface.");
            return CM_FAILURE;
        }

        if (SurfTypeToArgKind(pSurface->Type()) != m_Args[index].unitKind)
        {   // if surface type changes i.e 2D <-> 2DUP  Need to set bIsDrity as true
            m_Args[index].bIsDirty = true;
            m_Dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
        }

        uint32_t CISA_majorVersion, CISA_minorVersion;
        m_pProgram->GetCISAVersion(CISA_majorVersion, CISA_minorVersion);

        //This path is for surface array, including 1D, 2D, 3D,samplersurface, samplersurface8x8
        if ((num_surfaces > 1) && (pSurface->Type() != CM_ENUM_CLASS_TYPE_CMSURFACEVME))
        {
            int32_t hr = SetArgsInternalSurfArray(i,index, num_surfaces, pSurface, surfIndex, pSurfIndex,surfaces, surfIndexArray);
            if (hr != CM_SUCCESS)
            {
                CM_ASSERTMESSAGE("Error: SetArgsInternal for surface array failed!\n");
                return CM_INVALID_ARG_VALUE;
            }
            pValue = surfaces;
            pSurfIndexValue = surfIndexArray;
            size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
            m_Args[index].unitSize = (uint16_t)size;
        }
        else
        {   //This is for single surface and surface array for VME surface
            switch (pSurface->Type())
            {
                 case CM_ENUM_CLASS_TYPE_CMSURFACE2D:
                 {
                     CmSurface2DRT* pSurf2D = static_cast<CmSurface2DRT*>(pSurface);

                     uint32_t numAliases = 0;
                     pSurf2D->GetNumAliases(numAliases);
                     if (numAliases)
                     {
                         m_Args[index].bAliasCreated = true;
                     }
                     else
                     {
                         m_Args[index].bAliasCreated = false;
                     }

                     //set memory object control
                     pSurf2D->GetIndexFor2D(surfRegTableIndex);

                     surfaces[i] = surfRegTableIndex;
                     surfIndexArray[i] = (uint16_t)surfIndex;

                     pValue = surfaces;
                     pSurfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_Args[index].unitSize = (uint16_t)size;

                     if ((m_Args[index].unitKind == ARG_KIND_SURFACE) || (m_Args[index].unitKind == ARG_KIND_SURFACE_2D_UP)) // first time or last time is set to 2DUP
                     {
                         m_Args[index].unitKind = ARG_KIND_SURFACE_2D;
                         if (m_Args[index].s_k == SAMPLER_SURF)
                             m_Args[index].unitKind = ARG_KIND_SURFACE_SAMPLER;
                     }
                     else if (m_Args[index].unitKind != ARG_KIND_SURFACE_2D &&
                         m_Args[index].unitKind != ARG_KIND_SURFACE_SAMPLER &&
                         m_Args[index].unitKind != ARG_KIND_SURFACE2DUP_SAMPLER &&
                         m_Args[index].unitKind != ARG_KIND_SURFACE_2D_SCOREBOARD)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 2D surface to the arg which is previously assigned 1D surface, 3D surface, or VME surface.");
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMBUFFER_RT:
                 {
                     CmBuffer_RT* pSurf1D = static_cast<CmBuffer_RT*>(pSurface);

                     uint32_t numAliases = 0;
                     pSurf1D->GetNumAliases(numAliases);
                     if (numAliases)
                     {
                         m_Args[index].bAliasCreated = true;
                     }
                     else
                     {
                         m_Args[index].bAliasCreated = false;
                     }

                     //set memory object control
                     pSurf1D->GetHandle(handle);

                     surfaces[i] = handle;
                     surfIndexArray[i] = (uint16_t)surfIndex;

                     pValue = surfaces;
                     pSurfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_Args[index].unitSize = (uint16_t)size;

                     if (m_Args[index].unitKind == ARG_KIND_SURFACE)
                     {
                         m_Args[index].unitKind = ARG_KIND_SURFACE_1D;
                     }
                     else if (m_Args[index].unitKind != ARG_KIND_SURFACE_1D)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 1D surface to the arg which is previously assigned 2D surface, 3D surface, or VME surface.");
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACE2DUP:
                 {
                     CmSurface2DUPRT* pSurf2DUP = static_cast<CmSurface2DUPRT*>(pSurface);

                     //set memory object
                     pSurf2DUP->GetHandle(handle);

                     surfaces[i] = handle;
                     surfIndexArray[i] = (uint16_t)surfIndex;

                     pValue = surfaces;
                     pSurfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_Args[index].unitSize = (uint16_t)size;

                     if ((m_Args[index].unitKind == ARG_KIND_SURFACE) || (m_Args[index].unitKind == ARG_KIND_SURFACE_2D)) // first time or last time is set to 2D
                     {
                         m_Args[index].unitKind = ARG_KIND_SURFACE_2D_UP;
                     }
                     else if (m_Args[index].unitKind != ARG_KIND_SURFACE_2D_UP)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 2D surface UP to the arg which is previously assigned other surfaces.");
                         return CM_INVALID_ARG_VALUE;
                     }

                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACE3D:
                 {
                     CmSurface3DRT* pSurf3D = static_cast<CmSurface3DRT*>(pSurface);

                     pSurf3D->GetHandle(handle);

                     surfaces[i] = handle;
                     surfIndexArray[i] = (uint16_t)surfIndex;

                     pValue = surfaces;
                     pSurfIndexValue = surfIndexArray;

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_Args[index].unitSize = (uint16_t)size;

                     if (m_Args[index].unitKind == ARG_KIND_SURFACE) // first time
                     {
                         m_Args[index].unitKind = ARG_KIND_SURFACE_3D;
                     }
                     else if (m_Args[index].unitKind != ARG_KIND_SURFACE_3D)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a 3D surface to the arg which is previously assigned 1D surface, 2D surface or VME surface");
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }

                 case CM_ENUM_CLASS_TYPE_CM_STATE_BUFFER:
                 {
                     CmStateBuffer* pStateBuffer = static_cast< CmStateBuffer* >( pSurface );
                     pStateBuffer->GetHandle( handle );

                     surfaces[ i ] = handle;
                     surfIndexArray[ i ] = ( uint16_t )surfIndex;

                     pValue = surfaces;
                     pSurfIndexValue = surfIndexArray;


                     size = ( size / sizeof( SurfaceIndex ) ) * sizeof( uint32_t );
                     m_Args[ index ].unitSize = ( uint16_t )size;

                     if ( m_Args[ index ].unitKind == ARG_KIND_SURFACE ) // first time
                     {
                         m_Args[ index ].unitKind = ARG_KIND_STATE_BUFFER;
                     }
                     else if ( m_Args[ index ].unitKind != ARG_KIND_STATE_BUFFER )
                     {
                         CM_ASSERTMESSAGE( "Error: Assign a state buffer to the arg which is previously assigned 1D surface, 2D surface, 3D surface or VME surface" );
                         return CM_INVALID_ARG_VALUE;
                     }
                     break;
                 }

                 case CM_ENUM_CLASS_TYPE_CMSURFACEVME:
                 {
                     return SetArgsVme(nArgType, index, pValue, nThreadID);
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8:
                 {
                     CmSurfaceSampler8x8* pSurfSampler8x8 = static_cast <CmSurfaceSampler8x8 *> (pSurface);
                     pSurfSampler8x8->GetIndexCurrent(samplerIndex);
                     pSurfSampler8x8->GetCmIndex(samplerCmIndex);

                     m_pSurfaceMgr->GetSurface(samplerCmIndex, pSurface);
                     if (!pSurface)
                     {
                         CM_ASSERTMESSAGE("Error: Invalid sampler8x8 surface.");
                         return CM_FAILURE;
                     }

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_Args[index].unitSize = (uint16_t)size;

                     pValue = &samplerIndex;
                     pSurfIndexValue = &samplerCmIndex;

                     if (m_Args[index].unitKind == ARG_KIND_SURFACE)
                     {
                         if (pSurfSampler8x8->GetSampler8x8SurfaceType() == CM_VA_SURFACE)
                         {
                             m_Args[index].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_VA;
                             m_Args[index].nCustomValue = pSurfSampler8x8->GetAddressControlMode();
                         }
                         else
                         {
                             m_Args[index].unitKind = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
                         }
                     }
                     else if (m_Args[index].unitKind != ARG_KIND_SURFACE_SAMPLER8X8_AVS &&
                         m_Args[index].unitKind != ARG_KIND_SURFACE_SAMPLER8X8_VA)
                     {
                         CM_ASSERTMESSAGE("Error: Assign a Sampler8x8 surface to the arg which is previously 2D surface.");
                         return CM_FAILURE;
                     }
                     break;
                 }
                 case CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER:
                 {
                     CmSurfaceSampler* pSurfSampler = static_cast <CmSurfaceSampler *> (pSurface);
                     pSurfSampler->GetHandle(samplerIndex);
                     pSurfSampler->GetCmIndexCurrent(samplerCmIndex);

                     m_pSurfaceMgr->GetSurface(samplerCmIndex, pSurface);
                     if (!pSurface)
                     {
                         CM_ASSERTMESSAGE("Error: Invalid sampler surface.");
                         return CM_FAILURE;
                     }

                     size = (size / sizeof(SurfaceIndex)) * sizeof(uint32_t);
                     m_Args[index].unitSize = (uint16_t)size;

                     pValue = &samplerIndex;
                     pSurfIndexValue = &samplerCmIndex;

                     if (m_Args[index].unitKind == ARG_KIND_SURFACE)
                     {   // first time
                         SAMPLER_SURFACE_TYPE type;
                         pSurfSampler->GetSurfaceType(type);
                         if (type == SAMPLER_SURFACE_TYPE_2D)
                         {
                             m_Args[index].unitKind = ARG_KIND_SURFACE_SAMPLER;
                         }
                         else if (type == SAMPLER_SURFACE_TYPE_2DUP)
                         {
                             m_Args[index].unitKind = ARG_KIND_SURFACE2DUP_SAMPLER;
                         }
                         else
                         {
                             m_Args[index].unitKind = ARG_KIND_SURFACE_3D;
                         }

                     }
                     else if ((m_Args[index].unitKind != ARG_KIND_SURFACE_SAMPLER) &&
                         m_Args[index].unitKind != ARG_KIND_SURFACE2DUP_SAMPLER &&
                         (m_Args[index].unitKind != ARG_KIND_SURFACE_3D))
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
    else if (m_Args[index].unitKind == ARG_KIND_SAMPLER)
    {
        unsigned int num_samplers = m_Args[index].unitSize / sizeof(int);

        if (num_samplers > 1)
        {
            size = num_samplers * sizeof(unsigned int);

            for (unsigned int i = 0; i < num_samplers; i++)
            {
                SamplerIndex* pSamplerIndex = (SamplerIndex*)pValue + i;
                samplerIdx = pSamplerIndex->get_data();
                sampler_index_array.push_back(samplerIdx);
            }
        }
        else
        {
            SamplerIndex* pSamplerIndex = (SamplerIndex*)pValue;
            samplerIdx = pSamplerIndex->get_data();
            size = sizeof(unsigned int);
            m_Args[index].unitSize = (uint16_t)size;
            pValue = &samplerIdx;
        }
    }

finish:
    if ( nArgType == CM_KERNEL_INTERNEL_ARG_PERKERNEL ) // per kernel arg
    {
        CM_ARG& arg = m_Args[ index ];

        // Assume from now on, size is valid, i.e. confirmed with function signature
        if( !arg.pValue )
        {
            //Increment size kernel arguments will take up in CURBE
            uint32_t tempUnitSize = m_Args[ index ].unitSize;
            if( (m_Args[index].unitKind == ARG_KIND_SURFACE_VME ) ||
                (m_Args[index].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                (m_Args[index].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ))
            {
                tempUnitSize = CM_ARGUMENT_SURFACE_SIZE;
            }

            // first setKernelArg or first setKernelArg after each enqueue
            arg.pValue = MOS_NewArray(uint8_t,size);
            if( !arg.pValue )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;
            }

            arg.unitCount = 1;

            CmFastMemCopy((void *)arg.pValue, pValue, size);

            if((( m_Args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && pSurfIndexValue )
            {
                arg.surfIndex = MOS_NewArray(uint16_t, (size / sizeof(int32_t)));
                if (!arg.surfIndex)
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    MosSafeDeleteArray(arg.pValue);
                    return CM_OUT_OF_HOST_MEMORY;
                }
                CmSafeMemSet((void *)arg.surfIndex, 0, size/sizeof(int32_t) * sizeof(uint16_t));
                if( pSurfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmFastMemCopy((void *)arg.surfIndex, pSurfIndexValue, size / sizeof(int32_t) * sizeof(uint16_t));
            }

            if (m_Args[index].unitKind == ARG_KIND_SAMPLER)
            {
                for (unsigned int sampler_index = 0; sampler_index < sampler_index_array.size(); sampler_index++)
                {
                    *( (int *)arg.pValue + sampler_index) = sampler_index_array[sampler_index];
                }
            }

            m_Dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
            arg.bIsDirty = true;
        }
        else
        {
            if( arg.unitCount != 1 )
            {
                CM_ASSERTMESSAGE("Error: Invalid arg count.");
                return CM_FAILURE;
            }
            if( memcmp( (void *)arg.pValue, pValue, size ) != 0 )
            {
                CmFastMemCopy((void *)arg.pValue, pValue, size);
                m_Dirty |= CM_KERNEL_DATA_KERNEL_ARG_DIRTY;
                arg.bIsDirty = true;
            }
            if((( m_Args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
             ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
             ( m_Args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && pSurfIndexValue )
            {
                CmSafeMemSet((void *)arg.surfIndex, 0, size/sizeof(int32_t) * sizeof(uint16_t));
                if( pSurfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmFastMemCopy((void *)arg.surfIndex, pSurfIndexValue, size/sizeof(int32_t) * sizeof(uint16_t));
            }

            if (m_Args[index].unitKind == ARG_KIND_SAMPLER)
            {
                for (unsigned int sampler_index = 0; sampler_index < sampler_index_array.size(); sampler_index++)
                {
                    *((int *)arg.pValue + sampler_index) = sampler_index_array[sampler_index];
                }
            }
        }

        m_blPerKernelArgExists = true;
    }
    else //per thread arg
    {
        CM_ARG& arg = m_Args[ index ];

        // Assume from now on, size is valid, i.e. confirmed with function signature
        if( !arg.pValue )
        {
            //Increment size per-thread arguments will take up in payload of media object or media object walker commands
            m_SizeInPayload += arg.unitSize;
            DW_ALIGNMENT(m_SizeInPayload);

            // first setThreadArg or first setThreadArg after each enqueue
            arg.pValue = MOS_NewArray(uint8_t, (size * m_ThreadCount));
            if( !arg.pValue )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;

            }
            arg.unitCount = m_ThreadCount;

            uint32_t offset = size * nThreadID;
            uint8_t *pThreadValue = ( uint8_t *)arg.pValue;
            pThreadValue += offset;

            CmFastMemCopy(pThreadValue, pValue, size);
            if((( m_Args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && pSurfIndexValue )
            {
                arg.surfIndex = MOS_NewArray(uint16_t, (size / sizeof(uint32_t) * m_ThreadCount));
                if( !arg.surfIndex )
                {
                    CM_ASSERTMESSAGE("Error: Out of system memory.");
                    MosSafeDeleteArray(arg.pValue);
                    return CM_OUT_OF_HOST_MEMORY;
                }
                CmSafeMemSet((void *)arg.surfIndex, 0, size/sizeof(uint32_t) * sizeof(uint16_t) * m_ThreadCount);
                if( pSurfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmFastMemCopy((void *)(arg.surfIndex + size/sizeof(uint32_t)  * nThreadID), pSurfIndexValue, size/sizeof(uint32_t) * sizeof(uint16_t));
            }
            m_blPerThreadArgExists = true;
        }
        else
        {
            if( arg.unitCount != m_ThreadCount )
            {
                CM_ASSERTMESSAGE("Error: Arg count is not matched with thread count.");
                return CM_FAILURE;

            }
            uint32_t offset = size * nThreadID;
            uint8_t *pThreadValue = ( uint8_t *)arg.pValue;
            pThreadValue += offset;

            if( memcmp( pThreadValue, pValue, size ) != 0 )
            {
                CmFastMemCopy(pThreadValue, pValue, size);
                m_Dirty |= CM_KERNEL_DATA_THREAD_ARG_DIRTY;
                arg.bIsDirty = true;
            }
            if((( m_Args[ index ].unitKind == ARG_KIND_SURFACE ) || // first time
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_1D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_3D ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_VME ) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
                 ( m_Args[ index ].unitKind == ARG_KIND_STATE_BUFFER ) ) && pSurfIndexValue )
            {
                if( pSurfIndexValue == nullptr )
                {
                    CM_ASSERTMESSAGE("Error: Pointer to surface index value is null.");
                    return CM_NULL_POINTER;
                }
                CmFastMemCopy((void *)(arg.surfIndex + size/sizeof(uint32_t)  * nThreadID), pSurfIndexValue, size/sizeof(uint32_t) * sizeof(uint16_t));
            }
        }
    }

    m_Args[index].bIsSet = true;

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
//!     CM_INVALID_ARG_VALUE if pValue is NULL.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetKernelArg(uint32_t index, size_t size, const void * pValue )
{
    INSERT_API_CALL_LOG();
    //It should be mutual exclusive with Indirect Data
    if(m_pKernelPayloadData)
    {
        CM_ASSERTMESSAGE("Error: SetKernelArg should be mutual exclusive with indirect data.");
        return CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL;
    }

    if( index >= m_ArgCount )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel arg count.");
        return CM_INVALID_ARG_INDEX;

    }

    if( !pValue)
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
    if ( ( nRetVal = SetArgsInternal( CM_KERNEL_INTERNEL_ARG_PERKERNEL, index, size, pValue ) ) != CM_SUCCESS )
    {
        return nRetVal;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Set Static Buffer
//| Return :   The result of operation
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetStaticBuffer(uint32_t index, const void * pValue )
{
    INSERT_API_CALL_LOG();
    if(index >= CM_GLOBAL_SURFACE_NUMBER)
    {
        CM_ASSERTMESSAGE("Error: Surface Index exceeds max global surface number.");
        return CM_INVALID_GLOBAL_BUFFER_INDEX;
    }

    if(!pValue)
    {
        CM_ASSERTMESSAGE("Error: Invalid StaticBuffer arg value.");
        return CM_INVALID_BUFFER_HANDLER;
    }

    SurfaceIndex* pSurfIndex = (SurfaceIndex* )pValue;
    uint32_t surfIndex = pSurfIndex->get_data();
    if (surfIndex >= m_pSurfaceMgr->GetSurfacePoolSize())
    {
        CM_ASSERTMESSAGE("Error: StaticBuffer doesn't allow alias index.");
        return CM_INVALID_ARG_INDEX;
    }
    uint32_t handle = 0; // for 1D surf

    CmSurface* pSurface  = nullptr;
    m_pSurfaceMgr->GetSurface( surfIndex, pSurface );
    if(pSurface == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid surface.");
        return CM_INVALID_BUFFER_HANDLER;
    }

    CmBuffer_RT* pSurf1D = nullptr;
    if ( pSurface->Type() == CM_ENUM_CLASS_TYPE_CMBUFFER_RT )
    {
        CM_SURFACE_MEM_OBJ_CTRL memCtl;
        pSurf1D = static_cast< CmBuffer_RT* >( pSurface );
        pSurf1D->GetHandle( handle );

        if (m_GlobalSurfaces[index] == nullptr)
        {
            m_GlobalSurfaces[index] = MOS_New(SurfaceIndex,0);
            if( !m_GlobalSurfaces[index] )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;
            }
        }
        *m_GlobalSurfaces[index] = handle;
        m_GlobalCmIndex[index] = surfIndex;
        m_Dirty |= CM_KERNEL_DATA_GLOBAL_SURFACE_DIRTY;
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
//!     CM_INVALID_ARG_VALUE if pValue is nullptr
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::SetThreadArg(uint32_t threadId, uint32_t index, size_t size, const void * pValue )
{
    INSERT_API_CALL_LOG();

    //It should be mutual exclusive with Indirect Data
    if(m_pKernelPayloadData)
    {
        CM_ASSERTMESSAGE("Error: SetThredArg should be mutual exclusive with indirect data.");
        return CM_KERNELPAYLOAD_PERTHREADARG_MUTEX_FAIL;
    }

    if(m_ThreadCount > m_pHalMaxValues->maxUserThreadsPerTask || m_ThreadCount <=0)
    {
        CM_ASSERTMESSAGE("Error: Minimum or Maximum number of threads exceeded.");
        return CM_FAILURE;
    }

    if( index >= m_ArgCount )
    {
        CM_ASSERTMESSAGE("Error: Invalid thread arg count.");
        return CM_INVALID_ARG_INDEX;

    }

    if( threadId >= m_ThreadCount )
    {
        CM_ASSERTMESSAGE("Error: thread id exceeds the threadcount.");
        return CM_INVALID_THREAD_INDEX;

    }

    if( !pValue)
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
    if ( ( nRetVal = SetArgsInternal( CM_KERNEL_INTERNEL_ARG_PERTHREAD, index, size, pValue, threadId ) ) != CM_SUCCESS )
    {
        return nRetVal;
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:  Calculate the total size of kernel data
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CalcKernelDataSize(
                uint32_t MovInsNum,                 // [in] the number of move instructions
                uint32_t NumArgs,                   // [in] number of args , surface array count
                uint32_t ArgSize,                   // [in] Size of arguments
                uint32_t & TotalKernelDataSize)      // [out] total size of kernel data
{
    int32_t hr             = CM_SUCCESS;

    uint32_t headSize = ( KERNEL_INFO_SIZE_IN_DWORD + NumArgs * PER_ARG_SIZE_IN_DWORD ) * sizeof( uint32_t );
    uint32_t totalSize =  headSize + MovInsNum * CM_MOVE_INSTRUCTION_SIZE + m_uiBinarySize + ArgSize;

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

    TotalKernelDataSize = totalSize;


    return hr;
}

#if !USE_EXTENSION_CODE
//*-----------------------------------------------------------------------------
//| Purpose:   Create object for mov instructions
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::ConstructObjMovs(InstructionDistanceConfig *pInstDist, uint32_t dstOffset, uint32_t srcOffset, uint32_t size, CmDynamicArray &movInsts, uint32_t index, bool is_BDW, bool is_hwdebug)
{
    UNUSED(pInstDist);
    return MovInst_RT::CreateMoves(dstOffset, srcOffset, size, movInsts, index, is_BDW, is_hwdebug);
}
#endif

//*-----------------------------------------------------------------------------
//| Purpose:   Create mov instructions
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateMovInstructions( uint32_t &movInstNum, uint8_t *&pCodeDst, CM_ARG* pTempArgs, uint32_t NumArgs)
{
    //Create Mov Instruction
    CmDynamicArray      movInsts( NumArgs );
    InstructionDistanceConfig InstDist(movInsts.GetMaxSize());

    movInstNum = 0;

    //Note: if no thread arg and no per kernel arg, no need move instrcutions at all.
    if( m_CurbeEnable && (m_blPerThreadArgExists || m_blPerKernelArgExists))
    {
        if( ( m_ArgCount > 0 ) && ( m_ThreadCount > 1) )
        {
            PCM_ARG* ppArgSorted = MOS_NewArray(PCM_ARG,NumArgs);
            if( !ppArgSorted )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;
            }
            for( uint32_t j = 0; j < NumArgs; j++ )
            {
                ppArgSorted[ j ] = pTempArgs + j;
            }
            // sort pArg to ppArgSorted accorind to offsetinPayload
            QuickSort( ppArgSorted, 0, NumArgs - 1 );


            // record compiler generated offset, used as move dst later
            uint16_t *unitOffsetInPayloadSorted = MOS_NewArray(uint16_t, NumArgs);
            if( !unitOffsetInPayloadSorted )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                MosSafeDeleteArray(ppArgSorted);
                return CM_OUT_OF_HOST_MEMORY;
            }
            for( uint32_t j = 0; j < NumArgs; j++ )
            {
                unitOffsetInPayloadSorted[j] = ppArgSorted[j]->unitOffsetInPayload;
            }

            uint16_t kernelArgEnd = 32;
            bool beforeFirstThreadArg = true;
            for( uint32_t j = 0; j < NumArgs; j++ )
            {
                if( ppArgSorted[j]->unitCount == 1 )
                    // consider m_ThreadCount = 1 case later, where all args are treated as per thread arg
                {
                    if( beforeFirstThreadArg )
                    {
                        kernelArgEnd = ppArgSorted[j]->unitOffsetInPayload + ppArgSorted[j]->unitSize;
                    }
                    else
                    {
                        DW_ALIGNMENT( kernelArgEnd ); // necessary ?
                        ppArgSorted[j]->unitOffsetInPayload = kernelArgEnd;
                        kernelArgEnd += ppArgSorted[j]->unitSize;
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

            for (uint32_t j = 0; j < NumArgs; j++)
            {
                if (ppArgSorted[j]->unitCount > 1) // per thread
                {
                    ppArgSorted[j]->unitOffsetInPayload = (uint16_t)threadArgStart;
                    threadArgStart += ppArgSorted[j]->unitSize;
                    DW_ALIGNMENT(threadArgStart);
                }
            }

            bool needMovInstructions = false;
            for( uint32_t j = 0; j < NumArgs; j++ )
            {
                if ( unitOffsetInPayloadSorted[j] != ppArgSorted[j]->unitOffsetInPayload )
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
                nextIndex += ConstructObjMovs(&InstDist, R64_OFFSET, 32, size, movInsts, nextIndex, true, m_blhwDebugEnable);

                beforeFirstThreadArg = true;
                for (uint32_t j = 0; j < NumArgs; j++)
                {
                    if (ppArgSorted[j]->unitCount == 1)
                        // consider m_ThreadCount = 1 case later, where all args are treated as per thread arg
                    {
                        if (beforeFirstThreadArg == false)
                        {
                            // add move inst to move from ppArgSorted[j]->unitOffsetInPayload + R64 to unitOffsetInPayloadSorted[j]
                            nextIndex += ConstructObjMovs(&InstDist, unitOffsetInPayloadSorted[j],
                                R64_OFFSET + ppArgSorted[j]->unitOffsetInPayload - 32,
                                ppArgSorted[j]->unitSize, movInsts, nextIndex, true, m_blhwDebugEnable);
                        }
                    }
                    else // per thread
                    {
                        if (beforeFirstThreadArg)
                        {
                            beforeFirstThreadArg = false;
                        }

                        // add move inst to move from ppArgSorted[j]->unitOffsetInPayload + R64 to unitOffsetInPayloadSorted[j]
                        nextIndex += ConstructObjMovs(&InstDist, unitOffsetInPayloadSorted[j],
                            R64_OFFSET + ppArgSorted[j]->unitOffsetInPayload - CM_PAYLOAD_OFFSET,
                            ppArgSorted[j]->unitSize, movInsts, nextIndex, true, m_blhwDebugEnable);
                    }
                }

                movInstNum = nextIndex;
            }

            MosSafeDeleteArray(ppArgSorted);
            MosSafeDeleteArray(unitOffsetInPayloadSorted);
        }
    }// End of if( m_CurbeEnable && m_ThreadArgExists)

    uint32_t addInstDW[4];
    MOS_ZeroMemory(addInstDW, CM_MOVE_INSTRUCTION_SIZE);
    uint32_t addInstNum =0;

    if(m_pThreadSpace && m_adjustScoreboardY)
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
        pCodeDst = MOS_NewArray(uint8_t, ((movInstNum + addInstNum)  * CM_MOVE_INSTRUCTION_SIZE));
        if (!pCodeDst)
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
            MosSafeDeleteArray(pCodeDst);
            return CM_FAILURE;
        }
        if (j != 0)
        {
            movInst->ClearDebug();
        }
        CmFastMemCopy(pCodeDst + j * CM_MOVE_INSTRUCTION_SIZE, movInst->GetBinary(), CM_MOVE_INSTRUCTION_SIZE);
        CmSafeDelete(movInst); // delete each element in movInsts
    }
    movInsts.Delete();

    if(addInstNum != 0)
    {
       CmFastMemCopy(pCodeDst + movInstNum * CM_MOVE_INSTRUCTION_SIZE, addInstDW, CM_MOVE_INSTRUCTION_SIZE);

       movInstNum += addInstNum; // take add Y instruction into consideration
    }


    return CM_SUCCESS;
}

int32_t CmKernelRT::CreateKernelArgDataGroup(
    uint8_t   *&pData,
    uint32_t   Value)
{
    if (pData == nullptr)
    {
        pData = MOS_NewArray(uint8_t, sizeof(uint32_t));
        if(!pData)
        {
            return CM_OUT_OF_HOST_MEMORY;
        }
    }
    *(uint32_t *)pData = Value;
    return CM_SUCCESS;
}

int32_t CmKernelRT::CreateKernelImplicitArgDataGroup(
    uint8_t   *&pData,
    uint32_t   size)
{
    pData = MOS_NewArray(uint8_t, (size * sizeof(uint32_t)));
    if (!pData)
    {
        return CM_OUT_OF_HOST_MEMORY;
    }
    *(uint32_t *)pData = 0;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Create mov instructions
//|            instructions will be copied into DstMem
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateThreadArgData(
    PCM_HAL_KERNEL_ARG_PARAM    pKernelArg,
    uint32_t                    ThreadArgIndex,
    CmThreadSpaceRT*              pThreadSpace,
    CM_ARG*                     pCmArgs )
{
    int32_t         hr              = CM_SUCCESS;
    uint32_t        ThreadArgCount  = pCmArgs[ ThreadArgIndex].unitCount;
    uint32_t        ThreadArgSize   = pCmArgs[ ThreadArgIndex ].unitSize;

    if (CHECK_SURFACE_TYPE(pCmArgs->unitKind,  ARG_KIND_SURFACE_VME))
    {
        // reallocate the memory since the number of surfaces in a vme surface could vary
        MosSafeDeleteArray(pKernelArg->firstValue);
    }

    if( pKernelArg->firstValue  == nullptr)
    {   
        // if firstValue = nullptr, then create a new one, otherwise, update the exisitng one
        pKernelArg->firstValue = MOS_NewArray(uint8_t, (pCmArgs[ThreadArgIndex].unitCount * pCmArgs[ThreadArgIndex].unitSize));
        if( !pKernelArg->firstValue )
        {
            hr = CM_OUT_OF_HOST_MEMORY;
            goto finish;
        }
    }

    if(pKernelArg->unitCount == 1 ) // kernel arg
    {
        if (pCmArgs[ThreadArgIndex].pValue)
        {
            CmFastMemCopy(pKernelArg->firstValue, pCmArgs[ThreadArgIndex].pValue, ThreadArgCount * ThreadArgSize);
        }
        goto finish;
    }

    if( pThreadSpace != nullptr )
    {
        CM_DEPENDENCY_PATTERN DependencyPatternType = CM_NONE_DEPENDENCY;
        pThreadSpace->GetDependencyPatternType(DependencyPatternType);

        if ((m_AssociatedToTS == true) &&  (DependencyPatternType != CM_NONE_DEPENDENCY))
        {
            CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
            pThreadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);

            uint32_t *pBoardOrder = nullptr;
            pThreadSpace->GetBoardOrder(pBoardOrder);

            for (uint32_t index = 0; index < ThreadArgCount; index++)
            {
                uint32_t offset = pThreadSpaceUnit[pBoardOrder[index]].threadId;
                uint8_t *pArgSrc = (uint8_t*)pCmArgs[ThreadArgIndex].pValue + offset * ThreadArgSize;
                uint8_t *pArgDst = pKernelArg->firstValue + index * ThreadArgSize;
                CmFastMemCopy(pArgDst, pArgSrc, ThreadArgSize);
            }
        }
        else
        {
           CmFastMemCopy(pKernelArg->firstValue, pCmArgs[ ThreadArgIndex ].pValue, ThreadArgCount * ThreadArgSize);
        }
    }
    else
    {
        CmFastMemCopy(pKernelArg->firstValue, pCmArgs[ ThreadArgIndex ].pValue, ThreadArgCount * ThreadArgSize);
    }

finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Sort thread space for scorboarding
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SortThreadSpace( CmThreadSpaceRT*  pThreadSpace )
{
    int32_t                   hr = CM_SUCCESS;
    CM_DEPENDENCY_PATTERN DependencyPatternType = CM_NONE_DEPENDENCY;

    CMCHK_NULL(pThreadSpace);

    pThreadSpace->GetDependencyPatternType(DependencyPatternType);

    if(!pThreadSpace->IsThreadAssociated())
    {//Skip Sort if it is media walker
        return CM_SUCCESS;
    }

    if (pThreadSpace->CheckDependencyVectorsSet())
    {
        pThreadSpace->WavefrontDependencyVectors();
    }
    else
    {
        switch (DependencyPatternType)
        {
            case CM_WAVEFRONT:
                pThreadSpace->Wavefront45Sequence();
                break;

            case CM_WAVEFRONT26:
                pThreadSpace->Wavefront26Sequence();
                break;

            case CM_WAVEFRONT26Z:
                pThreadSpace->Wavefront26ZSequence();
                break;

            case CM_WAVEFRONT26ZI:
                CM_26ZI_DISPATCH_PATTERN dispatchPattern;
                pThreadSpace->Get26ZIDispatchPattern(dispatchPattern);
                switch (dispatchPattern)
                {
                case VVERTICAL_HVERTICAL_26:
                    pThreadSpace->Wavefront26ZISeqVVHV26();
                    break;
                case VVERTICAL_HHORIZONTAL_26:
                    pThreadSpace->Wavefront26ZISeqVVHH26();
                    break;
                case VVERTICAL26_HHORIZONTAL26:
                    pThreadSpace->Wavefront26ZISeqVV26HH26();
                    break;
                case VVERTICAL1X26_HHORIZONTAL1X26:
                    pThreadSpace->Wavefront26ZISeqVV1x26HH1x26();
                    break;
                default:
                    pThreadSpace->Wavefront26ZISeqVVHV26();
                    break;
                }
                break;

            case CM_HORIZONTAL_WAVE:
                pThreadSpace->HorizentalSequence();
                break;

            case CM_VERTICAL_WAVE:
                pThreadSpace->VerticalSequence();
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
    uint32_t     NumofArgs,
    CM_ARG*      &pTempArgs)
{
    int32_t     hr              = CM_SUCCESS;
    int32_t     num_surfaces    = 0;
    int32_t     increased_args  = 0;

    if( NumofArgs < m_ArgCount || pTempArgs != nullptr )
    {
        CM_ASSERTMESSAGE("Error: Invalid arg number or arg value.");
        hr = CM_FAILURE;
        goto finish;
    }

    pTempArgs = MOS_NewArray(CM_ARG, NumofArgs);
    CMCHK_NULL_RETURN(pTempArgs, CM_OUT_OF_HOST_MEMORY);
    CmSafeMemSet(pTempArgs, 0, NumofArgs* sizeof(CM_ARG) );

    for( uint32_t j = 0; j < m_ArgCount; j++ )
    {
        if ( CHECK_SURFACE_TYPE( m_Args[ j ].unitKind, // first time
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
            num_surfaces = m_Args[j].unitSize/sizeof(int);

            if (num_surfaces > 1)
            {
                if (m_Args[j].unitCount == 1)
                { //Kernel arg
                    for (int32_t k = 0; k < num_surfaces; k++)
                    {
                        pTempArgs[j + increased_args + k] = m_Args[j];
                        pTempArgs[j + increased_args + k].unitSize = sizeof(int32_t);
                        pTempArgs[j + increased_args + k].unitSizeOrig = sizeof(int32_t);
                        pTempArgs[j + increased_args + k].pValue = (uint8_t *)((uint32_t *)m_Args[j].pValue + k);
                        pTempArgs[j + increased_args + k].unitOffsetInPayload = m_Args[j].unitOffsetInPayload + 4 * k;
                        pTempArgs[j + increased_args + k].unitOffsetInPayloadOrig = pTempArgs[j + increased_args + k].unitOffsetInPayload;
                        //For each surface kind and custom value  in surface array
                        if (!m_Args[j].surfIndex[k])
                        {
                            //if surfIndex is 0, set kind to be CM_ARGUMENT_SURFACE2D
                            //This is for special usage if there is empty element in surface array.
                            pTempArgs[j + increased_args + k].unitKind = CM_ARGUMENT_SURFACE2D;
                            continue;
                        }
                        pTempArgs[j + increased_args + k].unitKind = m_Args[j].pSurfArrayArg[k].argKindForArray;
                        pTempArgs[j + increased_args + k].nCustomValue = m_Args[j].pSurfArrayArg[k].addressModeForArray;
                    }
                }
                else
                {
                    uint32_t *surfaces = (uint32_t *)MOS_NewArray(uint8_t, ((sizeof(int32_t) * m_Args[j].unitCount)));
                    CMCHK_NULL_RETURN(surfaces, CM_OUT_OF_HOST_MEMORY);
                    for (int32_t k = 0; k < num_surfaces; k++)
                    {
                        pTempArgs[j + increased_args + k] = m_Args[j];
                        pTempArgs[j + increased_args + k].unitSize = sizeof(int32_t);
                        pTempArgs[j + increased_args + k].unitSizeOrig = sizeof(int32_t);
                        pTempArgs[j + increased_args + k].pValue = MOS_NewArray(uint8_t, ((sizeof(int32_t) * m_Args[j].unitCount)));
                        if(pTempArgs[j + increased_args + k].pValue == nullptr)
                        {
                            CM_ASSERTMESSAGE("Error: Out of system memory.");
                            hr = CM_OUT_OF_HOST_MEMORY;
                            MosSafeDeleteArray(surfaces);
                            goto finish;
                        }
                        for (uint32_t s = 0; s < m_Args[j].unitCount; s++)
                        {
                            surfaces[s] = *(uint32_t *)((uint32_t *)m_Args[j].pValue + k + num_surfaces * s);
                        }
                        CmFastMemCopy(pTempArgs[j + increased_args + k].pValue, surfaces, sizeof(int32_t) * m_Args[j].unitCount);
                        pTempArgs[j + increased_args + k].unitOffsetInPayload = m_Args[j].unitOffsetInPayload + 4 * k;
                        pTempArgs[j + increased_args + k].unitOffsetInPayloadOrig = (uint16_t)-1;
                    }
                    MosSafeDeleteArray(surfaces);
                }
                increased_args += num_surfaces - 1;
            }
            else
            {
                pTempArgs[j + increased_args] = m_Args[j];
            }
        }
        else if (m_Args[ j ].unitKind == ARG_KIND_SURFACE_VME)
        {
            num_surfaces = m_Args[ j ].unitVmeArraySize;
            if(num_surfaces == 1)
            {  // single vme surface
               pTempArgs[j + increased_args] = m_Args[j];
            }
            else
            {  // multiple vme surfaces in surface array
                if (m_Args[j].unitCount == 1) { //Kernel Arg
                    uint32_t VmeSurfoffset = 0;

                    for (int32_t k = 0; k < num_surfaces; k++)
                    {
                        uint16_t VmeSize = (uint16_t)getVmeArgValueSize((PCM_HAL_VME_ARG_VALUE)(m_Args[j].pValue + VmeSurfoffset));

                        pTempArgs[j + increased_args + k] = m_Args[j];
                        pTempArgs[j + increased_args + k].unitSize = VmeSize;
                        pTempArgs[j + increased_args + k].unitSizeOrig = VmeSize;
                        pTempArgs[j + increased_args + k].pValue = (uint8_t *)(m_Args[j].pValue + VmeSurfoffset);
                        pTempArgs[j + increased_args + k].unitOffsetInPayload = m_Args[j].unitOffsetInPayload + k*4;
                        pTempArgs[j + increased_args + k].unitOffsetInPayloadOrig = pTempArgs[j + increased_args + k].unitOffsetInPayload;

                        VmeSurfoffset += VmeSize;
                    }
                }
             }
            increased_args += num_surfaces - 1;
        }
        else if (m_Args[j].unitKind == ARG_KIND_SAMPLER)
        {
            unsigned int num_samplers = m_Args[j].unitSize / sizeof(int);

            if (num_samplers > 1)
            {
                if (m_Args[j].unitCount == 1)
                {
                    //Kernel arg
                    for (unsigned int k = 0; k < num_samplers; k++)
                    {
                        pTempArgs[j + increased_args + k] = m_Args[j];
                        pTempArgs[j + increased_args + k].unitSize = sizeof(int);
                        pTempArgs[j + increased_args + k].unitSizeOrig = sizeof(int);
                        pTempArgs[j + increased_args + k].pValue = (unsigned char *)((unsigned int *)m_Args[j].pValue + k);
                        pTempArgs[j + increased_args + k].unitOffsetInPayload = m_Args[j].unitOffsetInPayload + 4 * k;
                        pTempArgs[j + increased_args + k].unitOffsetInPayloadOrig = pTempArgs[j + increased_args + k].unitOffsetInPayload;
                        pTempArgs[j + increased_args + k].unitKind = CM_ARGUMENT_SAMPLER;
                    }
                }
                else
                {
                    // Use sampler index array as thread arg.
                    // Not implemented yet.
                    return CM_NOT_IMPLEMENTED;
                }
                increased_args += num_samplers - 1;
            }
            else
            {
                pTempArgs[j + increased_args] = m_Args[j];
            }
        }
        else
        {
            pTempArgs[j + increased_args] = m_Args[j];
        }
    }

finish:
    if(hr == CM_OUT_OF_HOST_MEMORY)
    {
        if(pTempArgs)
        {
            for (uint32_t j = 0; j < NumofArgs; j++)
            {
                MosSafeDeleteArray(pTempArgs[j].pValue);
            }
        }
        MosSafeDeleteArray( pTempArgs );
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the number of args includes the num of surfaces in surface array
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetArgCountPlusSurfArray(uint32_t &ArgSize, uint32_t & ArgCountPlus)
{
    unsigned int extra_args = 0;

    ArgCountPlus = m_ArgCount;
    ArgSize      = 0;

    if(m_usKernelPayloadDataSize)
    { // if payload data exists, the number of args is zero
        ArgCountPlus  = 0;
        ArgSize       = 0;
        return CM_SUCCESS;
    }

    if( m_ArgCount != 0 )   //Need pass the arg either by arguments area, or by indirect payload area
    {
         //Sanity check for argument setting
        if((m_blPerThreadArgExists == false) && (m_blPerKernelArgExists == false) && (m_usKernelPayloadDataSize == 0))
        {
            if ( m_state_buffer_bounded == CM_STATE_BUFFER_NONE )
            {
                CM_ASSERTMESSAGE( "Error: Kernel arguments are not set." );
                return CM_NOT_SET_KERNEL_ARGUMENT;
            }
        }

        if(m_blPerThreadArgExists || m_blPerKernelArgExists)
        {
            for( uint32_t j = 0; j < m_ArgCount; j ++ )
            {
                //Sanity checking for every argument setting
                if ( !m_Args[j].bIsSet )
                {
                    CM_ASSERTMESSAGE("Error: One Kernel argument is not set.");
                    return CM_KERNEL_ARG_SETTING_FAILED;
                }

                ArgSize += m_Args[j].unitSize * m_Args[j].unitCount;

                if ( CHECK_SURFACE_TYPE( m_Args[ j ].unitKind,
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
                     int num_surfaces = m_Args[j].unitSize/sizeof(int);
                     if (num_surfaces > 1) {
                           extra_args += num_surfaces - 1;
                     }
                }
                else if (CHECK_SURFACE_TYPE(m_Args[j].unitKind, ARG_KIND_SURFACE_VME))
                {
                    int num_surfaces = m_Args[j].unitVmeArraySize;
                    if (num_surfaces > 1) {
                        extra_args += num_surfaces - 1;
                    }
                }
                else if (m_Args[j].unitKind == ARG_KIND_SAMPLER)
                {
                    int num_samplers = m_Args[j].unitSize / sizeof(int);
                    if (num_samplers > 1)
                    {
                        extra_args += (num_samplers - 1);
                    }
                }
            }

            ArgCountPlus = m_ArgCount + extra_args;
        }
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Create Thread Space Param
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateThreadSpaceParam(
    PCM_HAL_KERNEL_THREADSPACE_PARAM pCmKernelThreadSpaceParam,
    CmThreadSpaceRT*                   pThreadSpace     )
{
    int32_t                      hr = CM_SUCCESS;
    CM_HAL_DEPENDENCY*           pDependency = nullptr;
    uint32_t                     TsWidth = 0;
    uint32_t                     TsHeight =0;
    CM_THREAD_SPACE_UNIT         *pThreadSpaceUnit = nullptr;
    CM_THREAD_SPACE_DIRTY_STATUS dirtyStatus = CM_THREAD_SPACE_CLEAN;

    if (pCmKernelThreadSpaceParam == nullptr || pThreadSpace == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to CmKernelThreadSpaceParam or thread space is null.");
        hr = CM_NULL_POINTER;
        goto finish;
    }

    pThreadSpace->GetThreadSpaceSize(TsWidth, TsHeight);
    pCmKernelThreadSpaceParam->threadSpaceWidth =  (uint16_t)TsWidth;
    pCmKernelThreadSpaceParam->threadSpaceHeight = (uint16_t)TsHeight;

    pThreadSpace->GetDependencyPatternType(pCmKernelThreadSpaceParam->patternType);
    pThreadSpace->GetWalkingPattern(pCmKernelThreadSpaceParam->walkingPattern);
    pThreadSpace->GetDependency( pDependency);

    if(pDependency != nullptr)
    {
        CmFastMemCopy(&pCmKernelThreadSpaceParam->dependencyInfo, pDependency, sizeof(CM_HAL_DEPENDENCY));
    }

    if( pThreadSpace->CheckWalkingParametersSet( ) )
    {
        pCmKernelThreadSpaceParam->walkingParamsValid = 1;
        CMCHK_HR(pThreadSpace->GetWalkingParameters(pCmKernelThreadSpaceParam->walkingParams));
    }
    else
    {
        pCmKernelThreadSpaceParam->walkingParamsValid = 0;
    }

    if( pThreadSpace->CheckDependencyVectorsSet( ) )
    {
        pCmKernelThreadSpaceParam->dependencyVectorsValid = 1;
        CMCHK_HR(pThreadSpace->GetDependencyVectors(pCmKernelThreadSpaceParam->dependencyVectors));
    }
    else
    {
        pCmKernelThreadSpaceParam->dependencyVectorsValid = 0;
    }

    pThreadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);

    if(pThreadSpaceUnit)
    {
        pCmKernelThreadSpaceParam->threadCoordinates = MOS_NewArray(CM_HAL_SCOREBOARD, (TsWidth * TsHeight));
        CMCHK_NULL_RETURN(pCmKernelThreadSpaceParam->threadCoordinates , CM_OUT_OF_HOST_MEMORY);
        CmSafeMemSet(pCmKernelThreadSpaceParam->threadCoordinates, 0, TsHeight * TsWidth * sizeof(CM_HAL_SCOREBOARD));

        uint32_t *pBoardOrder = nullptr;
        pThreadSpace->GetBoardOrder(pBoardOrder);
        CMCHK_NULL(pBoardOrder);

        pCmKernelThreadSpaceParam->reuseBBUpdateMask  = 0;
        for(uint32_t i=0; i< TsWidth * TsHeight ; i++)
        {
            pCmKernelThreadSpaceParam->threadCoordinates[i].x = pThreadSpaceUnit[pBoardOrder[i]].scoreboardCoordinates.x;
            pCmKernelThreadSpaceParam->threadCoordinates[i].y = pThreadSpaceUnit[pBoardOrder[i]].scoreboardCoordinates.y;
            pCmKernelThreadSpaceParam->threadCoordinates[i].mask = pThreadSpaceUnit[pBoardOrder[i]].dependencyMask;
            pCmKernelThreadSpaceParam->threadCoordinates[i].resetMask= pThreadSpaceUnit[pBoardOrder[i]].reset;
            pCmKernelThreadSpaceParam->threadCoordinates[i].color = pThreadSpaceUnit[pBoardOrder[i]].scoreboardColor;
            pCmKernelThreadSpaceParam->threadCoordinates[i].sliceSelect = pThreadSpaceUnit[pBoardOrder[i]].sliceDestinationSelect;
            pCmKernelThreadSpaceParam->threadCoordinates[i].subSliceSelect = pThreadSpaceUnit[pBoardOrder[i]].subSliceDestinationSelect;
            pCmKernelThreadSpaceParam->reuseBBUpdateMask |= pThreadSpaceUnit[pBoardOrder[i]].reset;
        }

        if( pCmKernelThreadSpaceParam->patternType == CM_WAVEFRONT26Z )
        {
            CM_HAL_WAVEFRONT26Z_DISPATCH_INFO dispatchInfo;
            pThreadSpace->GetWavefront26ZDispatchInfo(dispatchInfo);

            pCmKernelThreadSpaceParam->dispatchInfo.numWaves = dispatchInfo.numWaves;
            pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave = MOS_NewArray(uint32_t, dispatchInfo.numWaves);
            CMCHK_NULL_RETURN(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, CM_OUT_OF_HOST_MEMORY);
            CmFastMemCopy(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave,
                dispatchInfo.numThreadsInWave, dispatchInfo.numWaves*sizeof(uint32_t));

         }
    }

    //Get group select setting information
    pThreadSpace->GetMediaWalkerGroupSelect(pCmKernelThreadSpaceParam->groupSelect);

    //Get color count
    pThreadSpace->GetColorCountMinusOne(pCmKernelThreadSpaceParam->colorCountMinusOne);

    dirtyStatus = pThreadSpace->GetDirtyStatus();
    switch (dirtyStatus)
    {
    case CM_THREAD_SPACE_CLEAN:
        pCmKernelThreadSpaceParam->bbDirtyStatus = CM_HAL_BB_CLEAN;
        break;
    default:
        pCmKernelThreadSpaceParam->bbDirtyStatus = CM_HAL_BB_DIRTY;
        break;
    }

finish:
    if( hr == CM_OUT_OF_HOST_MEMORY)
    {
        if( pCmKernelThreadSpaceParam )
        {
            MosSafeDeleteArray(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave);
            MosSafeDeleteArray(pCmKernelThreadSpaceParam->threadCoordinates);
        }
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Delete the args array
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::DestroyArgs( void )
{
    for( uint32_t i =0 ; i < m_ArgCount; i ++ )
    {
        CM_ARG& arg = m_Args[ i ];
        MosSafeDeleteArray( arg.pValue );
        MosSafeDeleteArray(arg.surfIndex);
        MosSafeDeleteArray(arg.pSurfArrayArg);
        arg.unitCount = 0;
        arg.unitSize = 0;
        arg.unitKind = 0;
        arg.unitOffsetInPayload = 0;
        arg.bIsDirty = true;
        arg.bIsSet = false;
    }

    MosSafeDeleteArray( m_Args );

    m_AssociatedToTS        = false;
    m_pThreadSpace          = nullptr;

    m_blPerThreadArgExists  = false;
    m_blPerKernelArgExists  = false;

    m_SizeInCurbe = 0;
    m_CurbeEnable = true;

    m_SizeInPayload = 0;
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
    for( uint32_t i =0 ; i < m_ArgCount; i ++ )
    {
        CM_ARG& arg = m_Args[ i ];
        MosSafeDeleteArray( arg.pValue );
        MosSafeDeleteArray( arg.surfIndex);
        MosSafeDeleteArray(arg.pSurfArrayArg);
        arg.pValue = nullptr;
        arg.unitCount = 0;

        arg.unitSize = arg.unitSizeOrig;
        arg.unitKind = arg.unitKindOrig;
        arg.unitOffsetInPayload = arg.unitOffsetInPayloadOrig;

        arg.bIsDirty = true;
        arg.bIsSet = false;
        arg.unitVmeArraySize = 0;
    }

    m_ThreadCount = 0;

    m_IndexInTask = 0;

    m_blPerThreadArgExists = false;
    m_blPerKernelArgExists = false;

    m_SizeInCurbe = 0;
    m_CurbeEnable = true;

    m_SizeInPayload = 0;

    m_AssociatedToTS = false;
    m_pThreadSpace = nullptr;
    m_adjustScoreboardY = 0;

    m_pThreadGroupSpace = nullptr;

    MosSafeDeleteArray(m_pKernelPayloadData);
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
int32_t CmKernelRT::GetArgs( CM_ARG* & pArg )
{
    pArg = m_Args;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the arguments' count
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetArgCount( uint32_t & argCount )
{
    argCount = m_ArgCount;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the value of member CurbeEnable
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetCurbeEnable( bool& b )
{
    b = m_CurbeEnable;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set the CurbeEnable member
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetCurbeEnable( bool b )
{
    m_CurbeEnable = b;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the kernel's size in Curbe
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetSizeInCurbe( uint32_t& size )
{
    size = m_SizeInCurbe;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Get the total size in payload of meida object or media walker
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetSizeInPayload( uint32_t& size )
{
    size = m_SizeInPayload;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the pointer to CM device
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::GetCmDevice(CmDeviceRT* &pCmDev)
{
    pCmDev = m_pCmDev;
    return CM_SUCCESS;
}

int32_t CmKernelRT::GetCmProgram( CmProgramRT* & pProgram )
{
    pProgram = m_pProgram;
    return CM_SUCCESS;
}

int32_t CmKernelRT::CollectKernelSurface()
{
    m_VMESurfaceCount = 0;
    m_MaxSurfaceIndexAllocated = 0;

    for( uint32_t j = 0; j < m_ArgCount; j ++ )
    {
        if ((m_Args[ j ].unitKind == ARG_KIND_SURFACE ) || // first time
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_1D ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_2D ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_2D_UP ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_SAMPLER ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE2DUP_SAMPLER ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_3D ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_AVS) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_SAMPLER8X8_VA) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_VME ) ||
             ( m_Args[ j ].unitKind == ARG_KIND_SURFACE_2D_SCOREBOARD) ||
             ( m_Args[ j ].unitKind == ARG_KIND_STATE_BUFFER ) )
        {
            int num_surfaces;
            int num_valid_surfaces = 0;

            if (m_Args[ j ].unitKind == ARG_KIND_SURFACE_VME)
            {
                num_surfaces = getSurfNumFromArgArraySize(m_Args[j].unitSize, m_Args[j].unitVmeArraySize);
            }
            else
            {
                num_surfaces = m_Args[j].unitSize/sizeof(int);
            }

            for (uint32_t k = 0; k < num_surfaces * m_Args[j].unitCount; k ++)
            {
                uint16_t surfIndex = 0;
                if (m_Args[j].surfIndex)
                {
                    surfIndex = m_Args[j].surfIndex[k];
                }
                if (surfIndex != 0 && surfIndex != CM_NULL_SURFACE)
                {
                    m_SurfaceArray[surfIndex] = true;
                    num_valid_surfaces ++;
                    m_MaxSurfaceIndexAllocated = Max(m_MaxSurfaceIndexAllocated, surfIndex);
                }
            }
            if (m_Args[ j ].unitKind == ARG_KIND_SURFACE_VME)
            {
                m_VMESurfaceCount += num_valid_surfaces;
            }
        }
    }

    for( int32_t i=0; i < CM_GLOBAL_SURFACE_NUMBER; ++i )
    {
        if( m_GlobalSurfaces[i] != nullptr )
        {
            uint32_t surfIndex = m_GlobalCmIndex[i];
            m_SurfaceArray[surfIndex] = true;
        }
    }

    for (int32_t i = 0; i < m_usKernelPayloadSurfaceCount; i++)
    {
        if (m_pKernelPayloadSurfaceArray[i] != nullptr)
        {
            uint32_t surfIndex = m_pKernelPayloadSurfaceArray[i]->get_data();
            m_SurfaceArray[surfIndex] = true;
        }
    }

    return CM_SUCCESS;
}

int32_t CmKernelRT::IsKernelDataReusable( CmThreadSpaceRT* pTS)
{
    if(pTS)
    {
        if(pTS->IsThreadAssociated() && (pTS->GetDirtyStatus()!= CM_THREAD_SPACE_CLEAN))
        {
            return false;
        }
    }

    if(m_pThreadSpace)
    {
        if(m_pThreadSpace->GetDirtyStatus()!= CM_THREAD_SPACE_CLEAN)
        {
            return  false;
        }
    }

    if(m_Dirty !=  CM_KERNEL_DATA_CLEAN)
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
    CmKernelData* & pKernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadSpaceRT* pTS )    // in
{
    int32_t              hr              = CM_SUCCESS;
    PCM_HAL_KERNEL_PARAM pHalKernelParam = nullptr;

    if( (pTS != nullptr) && (m_pThreadSpace != nullptr) )
    {
        // per-kernel threadspace and per-task threadspace cannot be set at the same time
        return CM_INVALID_THREAD_SPACE;
    }

    if(m_pLastKernelData == nullptr)
    {
        CMCHK_HR(CreateKernelDataInternal(pKernelData, kernelDataSize, pTS));
        CMCHK_HR(AcquireKernelProgram()); // increase kernel/program's ref count
        CMCHK_HR(UpdateLastKernelData(pKernelData));
    }
    else
    {
        if(IsKernelDataReusable(const_cast<CmThreadSpaceRT *>(pTS)))
        {
            // nothing changed; Reuse m_pLastKernelData
            pKernelData = m_pLastKernelData;
            CMCHK_HR(AcquireKernelData(pKernelData));
            CMCHK_HR(AcquireKernelProgram()); // increase kernel and program's ref count
            kernelDataSize = pKernelData->GetKernelDataSize();

            if (m_pThreadSpace)
            {
                pHalKernelParam = pKernelData->GetHalCmKernelData();
                CMCHK_NULL(pHalKernelParam);
                // need to set to clean here because CmThreadSpaceParam.BBdirtyStatus is only set in CreateKernelDataInternal
                // flag used to re-use batch buffer, don't care if BB is busy if it is "clean"
                pHalKernelParam->kernelThreadSpaceParam.bbDirtyStatus = CM_HAL_BB_CLEAN;
            }
        }
        else
        {
            if(m_pLastKernelData->IsInUse())
            { // Need to Create a new one , if the kernel data is in use
                CMCHK_HR(CreateKernelDataInternal(pKernelData, kernelDataSize, pTS));
                CMCHK_HR(AcquireKernelProgram()); // increase kernel/program's ref count
                CMCHK_HR(UpdateLastKernelData(pKernelData));
            }
            else if(pTS && pTS->IsThreadAssociated() && (pTS->GetDirtyStatus() != CM_THREAD_SPACE_CLEAN))
            { // if thread space is assocaited , don't support reuse
                CMCHK_HR(CreateKernelDataInternal(pKernelData, kernelDataSize, pTS));
                CMCHK_HR(AcquireKernelProgram()); // increase kernel/program's ref count
                CMCHK_HR(UpdateLastKernelData(pKernelData));
            }
            else if(m_Dirty < CM_KERNEL_DATA_THREAD_COUNT_DIRTY || // Kernel arg or thread arg dirty
                (m_pThreadSpace && m_pThreadSpace->GetDirtyStatus() == CM_THREAD_SPACE_DEPENDENCY_MASK_DIRTY))
            {
                CMCHK_HR(UpdateKernelData(m_pLastKernelData,pTS));
                pKernelData = m_pLastKernelData;
                CMCHK_HR(AcquireKernelData(pKernelData));
                CMCHK_HR(AcquireKernelProgram()); // increase kernel and program's ref count
                kernelDataSize = pKernelData->GetKernelDataSize();

            }
            else
            {
               CMCHK_HR(CreateKernelDataInternal(pKernelData, kernelDataSize, pTS));
               CMCHK_HR(AcquireKernelProgram()); // increase kernel/program's ref count
               CMCHK_HR(UpdateLastKernelData(pKernelData));
            }
        }
    }

    CleanArgDirtyFlag();
    if(pTS)
    {
        pTS->SetDirtyStatus(CM_THREAD_SPACE_CLEAN);
    }
    if (m_pThreadSpace)
    {
        m_pThreadSpace->SetDirtyStatus(CM_THREAD_SPACE_CLEAN);
    }

finish:
    return hr;
}

char* CmKernelRT::GetName() { return (char*)m_pKernelInfo->kernelName; }

//*-----------------------------------------------------------------------------
//| Purpose:    Create Kernel Data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelData(
    CmKernelData* & pKernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadGroupSpace* pTGS )    // in
{
    int32_t     hr   = CM_SUCCESS;
    CmThreadGroupSpace* pUsedTGS = nullptr;

    //If kernel has associated TGS, we will use it, instead of per-task TGS
    if (m_pThreadGroupSpace)
    {
        pUsedTGS = m_pThreadGroupSpace;
    }
    else
    {
        pUsedTGS = const_cast<CmThreadGroupSpace*>(pTGS);
    }

    if(m_pLastKernelData == nullptr)
    {
        CMCHK_HR(CreateKernelDataInternal(pKernelData, kernelDataSize, pUsedTGS));
        CMCHK_HR(AcquireKernelProgram()); // increase kernel/program's ref count
        CMCHK_HR(UpdateLastKernelData(pKernelData));
    }
    else
    {
        if(!(m_Dirty & CM_KERNEL_DATA_KERNEL_ARG_DIRTY))
        {
            // nothing changed; Reuse m_pLastKernelData
            pKernelData = m_pLastKernelData;
            CMCHK_HR(AcquireKernelData(pKernelData));
            CMCHK_HR(AcquireKernelProgram()); // increase kernel and program's ref count
            kernelDataSize = pKernelData->GetKernelDataSize();
        }
        else
        {
            if(m_pLastKernelData->IsInUse())
            { // Need to Clone a new one
                CMCHK_HR(CreateKernelDataInternal(pKernelData, kernelDataSize, pUsedTGS));
                CMCHK_HR(AcquireKernelProgram()); // increase kernel/program's ref count
                CMCHK_HR(UpdateLastKernelData(pKernelData));
            }
            else
            {
                // change happend -> Reuse m_pLastKernelData but need to change its content accordingly
                CMCHK_HR(UpdateKernelData(m_pLastKernelData, pUsedTGS));
                pKernelData = m_pLastKernelData;
                CMCHK_HR(AcquireKernelData(pKernelData));
                CMCHK_HR(AcquireKernelProgram()); // increase kernel and program's ref count
                kernelDataSize = pKernelData->GetKernelDataSize();
            }
        }
    }

    CleanArgDirtyFlag();

finish:
    return hr;
}


int32_t CmKernelRT::CleanArgDirtyFlag()
{

    for(uint32_t i =0 ; i< m_ArgCount; i++)
    {
        m_Args[i].bIsDirty = false;
    }

    if(m_pThreadSpace && m_pThreadSpace->GetDirtyStatus())
    {
        m_pThreadSpace->SetDirtyStatus(CM_THREAD_SPACE_CLEAN);
    }

    m_Dirty                 = CM_KERNEL_DATA_CLEAN;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update the global surface and gtpin surface info to kernel data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateKernelDataGlobalSurfaceInfo( PCM_HAL_KERNEL_PARAM pHalKernelParam )
{
    int32_t hr = CM_SUCCESS;

    //global surface
    for ( uint32_t j = 0; j < CM_GLOBAL_SURFACE_NUMBER; j++ )
    {
        if ( m_GlobalSurfaces[ j ] != nullptr )
        {
            pHalKernelParam->globalSurface[ j ] = m_GlobalSurfaces[ j ]->get_data();
            pHalKernelParam->globalSurfaceUsed = true;
        }
        else
        {
            pHalKernelParam->globalSurface[ j ] = CM_NULL_SURFACE;
        }
    }

    for ( uint32_t j = CM_GLOBAL_SURFACE_NUMBER; j < CM_MAX_GLOBAL_SURFACE_NUMBER; j++ )
    {
        pHalKernelParam->globalSurface[ j ] = CM_NULL_SURFACE;
    }
#if USE_EXTENSION_CODE
    UpdateKernelDataGTPinSurfaceInfo(pHalKernelParam);
#endif

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelDataInternal(
    CmKernelData* & pKernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadGroupSpace* pTGS)    // in
{
    PCM_HAL_KERNEL_PARAM  pHalKernelParam = nullptr;
    int32_t               hr = CM_SUCCESS;
    uint32_t              movInstNum = 0;
    uint32_t              KrnCurbeSize = 0;
    uint32_t              NumArgs = 0;
    CM_ARG                *pTempArgs = nullptr;
    uint32_t              ArgSize = 0;
    uint32_t              surfNum = 0; //Pass needed BT entry numbers to HAL CM
    CmKernelRT             *pCmKernel = nullptr;

    CMCHK_HR(CmKernelData::Create(this, pKernelData));
    pHalKernelParam = pKernelData->GetHalCmKernelData();
    CMCHK_NULL(pHalKernelParam);

    //Get Num of args with surface array
    CMCHK_HR(GetArgCountPlusSurfArray(ArgSize, NumArgs));

    //Create Temp args
    CMCHK_HR(CreateTempArgs(NumArgs, pTempArgs));

    //Create move instructions
    CMCHK_HR(CreateMovInstructions(movInstNum, pHalKernelParam->movInsData, pTempArgs, NumArgs));
    CMCHK_HR(CalcKernelDataSize(movInstNum, NumArgs, ArgSize, kernelDataSize));
    CMCHK_HR(pKernelData->SetKernelDataSize(kernelDataSize));

    pHalKernelParam->clonedKernelParam.isClonedKernel = m_IsClonedKernel;
    pHalKernelParam->clonedKernelParam.kernelID       = m_CloneKernelID;
    pHalKernelParam->clonedKernelParam.hasClones      = m_HasClones;

    pHalKernelParam->kernelId = m_Id++;
    if ((m_pProgram->m_CISA_majorVersion >= 3 && m_pProgram->m_CISA_minorVersion >= 3))
        pHalKernelParam->numArgs = NumArgs;
    else
        pHalKernelParam->numArgs = NumArgs + CM_GPUWALKER_IMPLICIT_ARG_NUM;
    pHalKernelParam->numThreads = m_ThreadCount;
    pHalKernelParam->kernelBinarySize = m_uiBinarySize + movInstNum * CM_MOVE_INSTRUCTION_SIZE;
    pHalKernelParam->kernelDataSize = kernelDataSize;
    pHalKernelParam->movInsDataSize = movInstNum * CM_MOVE_INSTRUCTION_SIZE;
    pHalKernelParam->kernelDebugEnabled = m_blhwDebugEnable;

    pHalKernelParam->cmFlags = m_CurbeEnable ? CM_FLAG_CURBE_ENABLED : 0;
    pHalKernelParam->cmFlags |= m_NonstallingScoreboardEnable ? CM_FLAG_NONSTALLING_SCOREBOARD_ENABLED : 0;

    pHalKernelParam->kernelBinary = (uint8_t*)m_pBinary;

    CMCHK_HR(pKernelData->GetCmKernel(pCmKernel));
    if (pCmKernel == nullptr)
    {
        return CM_NULL_POINTER;
    }
    MOS_SecureStrcpy(pHalKernelParam->kernelName, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, pCmKernel->GetName());

    uint32_t thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth;
    pTGS->GetThreadGroupSpaceSize(thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);

    for (uint32_t i = 0; i < NumArgs; i++)
    {
        pHalKernelParam->argParams[i].unitCount = pTempArgs[i].unitCount;
        pHalKernelParam->argParams[i].kind = (CM_HAL_KERNEL_ARG_KIND)(pTempArgs[i].unitKind);
        pHalKernelParam->argParams[i].unitSize = pTempArgs[i].unitSize;
        pHalKernelParam->argParams[i].payloadOffset = pTempArgs[i].unitOffsetInPayload;
        pHalKernelParam->argParams[i].perThread = false;
        pHalKernelParam->argParams[i].nCustomValue = pTempArgs[i].nCustomValue;
        pHalKernelParam->argParams[i].isNull = pTempArgs[ i ].bIsNull;

        if (pTempArgs[i].unitKind == CM_ARGUMENT_IMPLICT_LOCALSIZE) {
            CMCHK_HR(CreateKernelImplicitArgDataGroup(pHalKernelParam->argParams[i].firstValue, 3));
            *(uint32_t *)pHalKernelParam->argParams[i].firstValue = thrdSpaceWidth;
            *(uint32_t *)(pHalKernelParam->argParams[i].firstValue + 4) = thrdSpaceHeight;
            *(uint32_t *)(pHalKernelParam->argParams[i].firstValue + 8) = thrdSpaceDepth;
        }
        else if (pTempArgs[i].unitKind == CM_ARGUMENT_IMPLICT_GROUPSIZE) {
            CMCHK_HR(CreateKernelImplicitArgDataGroup(pHalKernelParam->argParams[i].firstValue, 3));
            *(uint32_t *)pHalKernelParam->argParams[i].firstValue = grpSpaceWidth;
            *(uint32_t *)(pHalKernelParam->argParams[i].firstValue + 4) = grpSpaceHeight;
            *(uint32_t *)(pHalKernelParam->argParams[i].firstValue + 8) = grpSpaceDepth;
        }
        else if (pTempArgs[i].unitKind == ARG_KIND_IMPLICIT_LOCALID) {
            CMCHK_HR(CreateKernelImplicitArgDataGroup(pHalKernelParam->argParams[i].firstValue, 3));
            pHalKernelParam->localIdIndex = i;
        }
        else
            CreateThreadArgData(&pHalKernelParam->argParams[i], i, nullptr, pTempArgs);

        if (pHalKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE)
        {
            if (IsKernelArg(pHalKernelParam->argParams[i]))
            {
                // Kernel Arg : calculate curbe size & adjust payloadoffset
                pHalKernelParam->argParams[i].payloadOffset -= CM_PAYLOAD_OFFSET;
                if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion < 3)) {
                    if ((pHalKernelParam->argParams[i].payloadOffset + pHalKernelParam->argParams[i].unitSize > KrnCurbeSize))
                    {  // The largest one
                        KrnCurbeSize = pHalKernelParam->argParams[i].payloadOffset + pHalKernelParam->argParams[i].unitSize;
                    }
                }
                else
                {
                    if ((pHalKernelParam->argParams[i].payloadOffset + pHalKernelParam->argParams[i].unitSize > KrnCurbeSize) && (pTempArgs[i].unitKind != ARG_KIND_IMPLICIT_LOCALID))
                    {  // The largest one
                        KrnCurbeSize = pHalKernelParam->argParams[i].payloadOffset + pHalKernelParam->argParams[i].unitSize;
                    }
                }
            }
        }
    }

    if ( m_state_buffer_bounded != CM_STATE_BUFFER_NONE )
    {
        PCM_CONTEXT_DATA pCmData = ( PCM_CONTEXT_DATA )m_pCmDev->GetAccelData();
        PCM_HAL_STATE pState = pCmData->cmHalState;
        KrnCurbeSize = pState->pfnGetStateBufferSizeForKernel( pState, this );
        pHalKernelParam->stateBufferType = pState->pfnGetStateBufferTypeForKernel( pState, this );
    }

    if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion < 3))
    {
        // GPGPU walker - implicit args
        for (uint32_t i = NumArgs; i < NumArgs + CM_GPUWALKER_IMPLICIT_ARG_NUM; i++)
        {
            pHalKernelParam->argParams[i].unitCount = 1;
            pHalKernelParam->argParams[i].kind = CM_ARGUMENT_GENERAL;
            pHalKernelParam->argParams[i].unitSize = 4;
            pHalKernelParam->argParams[i].payloadOffset = MOS_ALIGN_CEIL(KrnCurbeSize, 4) + (i - NumArgs) * sizeof(uint32_t);
            pHalKernelParam->argParams[i].perThread = false;
        }

        CMCHK_HR(CreateKernelArgDataGroup(pHalKernelParam->argParams[NumArgs + 0].firstValue, thrdSpaceWidth));
        CMCHK_HR(CreateKernelArgDataGroup(pHalKernelParam->argParams[NumArgs + 1].firstValue, thrdSpaceHeight));
        CMCHK_HR(CreateKernelArgDataGroup(pHalKernelParam->argParams[NumArgs + 2].firstValue, grpSpaceWidth));
        CMCHK_HR(CreateKernelArgDataGroup(pHalKernelParam->argParams[NumArgs + 3].firstValue, grpSpaceHeight));
        CMCHK_HR(CreateKernelArgDataGroup(pHalKernelParam->argParams[NumArgs + 4].firstValue, thrdSpaceWidth));
        CMCHK_HR(CreateKernelArgDataGroup(pHalKernelParam->argParams[NumArgs + 5].firstValue, thrdSpaceHeight));
        pHalKernelParam->localIdIndex = pHalKernelParam->numArgs - 2;
    }
    pHalKernelParam->gpgpuWalkerParams.gpgpuEnabled = true;
    pHalKernelParam->gpgpuWalkerParams.groupWidth = grpSpaceWidth;
    pHalKernelParam->gpgpuWalkerParams.groupHeight = grpSpaceHeight;
    pHalKernelParam->gpgpuWalkerParams.groupDepth = grpSpaceDepth;
    pHalKernelParam->gpgpuWalkerParams.threadHeight = thrdSpaceHeight;
    pHalKernelParam->gpgpuWalkerParams.threadWidth = thrdSpaceWidth;
    pHalKernelParam->gpgpuWalkerParams.threadDepth = thrdSpaceDepth;
    //Get SLM size
    pHalKernelParam->slmSize = GetSLMSize();

    //Get spill area to adjust scratch space
    pHalKernelParam->spillSize = GetSpillMemUsed();

    //Set Barrier mode
    pHalKernelParam->barrierMode = m_BarrierMode;
    pHalKernelParam->numberThreadsInGroup = thrdSpaceWidth * thrdSpaceHeight * thrdSpaceDepth;
    if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion < 3))
        KrnCurbeSize = MOS_ALIGN_CEIL(KrnCurbeSize, 4) + CM_GPUWALKER_IMPLICIT_ARG_NUM * sizeof(uint32_t);
    else
        KrnCurbeSize = MOS_ALIGN_CEIL(KrnCurbeSize, 4);
    if ((KrnCurbeSize % 32) == 4) //The per-thread data occupy 2 GRF.
    {
        pHalKernelParam->curbeSizePerThread = 64;
    }
    else
    {
        pHalKernelParam->curbeSizePerThread = 32;
    }
    if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion < 3)) {
        pHalKernelParam->totalCurbeSize = MOS_ALIGN_CEIL(KrnCurbeSize, 32) - pHalKernelParam->curbeSizePerThread + pHalKernelParam->curbeSizePerThread *
            thrdSpaceWidth * thrdSpaceHeight;
        //Since the CURBE is 32 bytes alignment, for GPGPU walker without the user specified thread argument, implicit per-thread id arguments will occupy at most 32 bytes
        pHalKernelParam->crossThreadConstDataLen = MOS_ALIGN_CEIL(KrnCurbeSize, 32) - pHalKernelParam->curbeSizePerThread;
    }
    else {
        pHalKernelParam->totalCurbeSize = MOS_ALIGN_CEIL(KrnCurbeSize, 32) + pHalKernelParam->curbeSizePerThread *
            thrdSpaceWidth * thrdSpaceHeight * thrdSpaceDepth;
        //Since the CURBE is 32 bytes alignment, for GPGPU walker without the user specified thread argument, implicit per-thread id arguments will occupy at most 32 bytes
        pHalKernelParam->crossThreadConstDataLen = MOS_ALIGN_CEIL(KrnCurbeSize, 32);
    }
    pHalKernelParam->payloadSize = 0; // no thread arg allowed

    m_SizeInCurbe = GetAlignedCurbeSize(pHalKernelParam->totalCurbeSize);

    CMCHK_HR(CreateKernelIndirectData(&pHalKernelParam->indirectDataParam));

    if (m_SamplerBTICount != 0)
    {
        CmFastMemCopy((void*)pHalKernelParam->samplerBTIParam.samplerInfo, (void*)m_SamplerBTIEntry, sizeof(m_SamplerBTIEntry));
        pHalKernelParam->samplerBTIParam.samplerCount = m_SamplerBTICount;

        CmSafeMemSet(m_SamplerBTIEntry, 0, sizeof(m_SamplerBTIEntry));
        m_SamplerBTICount = 0;
    }

    CalculateKernelSurfacesNum(surfNum, pHalKernelParam->numSurfaces);

    UpdateKernelDataGlobalSurfaceInfo(pHalKernelParam);

    //Destroy Temp Args
    for (uint32_t j = 0; j < NumArgs; j++)
    {
        if (pTempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
        {
            MosSafeDeleteArray(pTempArgs[j].pValue);
        }
    }
    MosSafeDeleteArray(pTempArgs);

    CMCHK_HR(UpdateSamplerHeap(pKernelData));
finish:
    if (hr != CM_SUCCESS)
    {
        //Clean allocated memory : need to count the implicit args
        if ((m_pProgram->m_CISA_majorVersion == 3) && (m_pProgram->m_CISA_minorVersion < 3)) {

            for (uint32_t i = 0; i < NumArgs + CM_GPUWALKER_IMPLICIT_ARG_NUM; i++)
            {
                if (pHalKernelParam)
                {
                    if (pHalKernelParam->argParams[i].firstValue)
                    {
                        MosSafeDeleteArray(pHalKernelParam->argParams[i].firstValue);
                    }
                }
            }
        }
        else
        {
            for (uint32_t i = 0; i < NumArgs; i++)
            {
                if (pHalKernelParam)
                {
                    if (pHalKernelParam->argParams[i].firstValue)
                    {
                        MosSafeDeleteArray(pHalKernelParam->argParams[i].firstValue);
                    }
                }
            }
        }
        //Destroy Temp Args in failing case
        if (pTempArgs)
        {
            for (uint32_t j = 0; j < NumArgs; j++)
            {
                if (pTempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
                {
                    MosSafeDeleteArray(pTempArgs[j].pValue);
                }
            }
            MosSafeDeleteArray(pTempArgs);
        }
    }
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelRT::IsBatchBufferReusable( CmThreadSpaceRT * pTaskThreadSpace )
{
    bool Reusable = true;
    //Update m_Id if the batch buffer is not reusable.
    if (m_Dirty & CM_KERNEL_DATA_THREAD_ARG_DIRTY)
    {
        Reusable = false; // if thread arg dirty
    }
    else if ((m_Dirty & CM_KERNEL_DATA_KERNEL_ARG_DIRTY) && (m_CurbeEnable == false))
    {
        Reusable = false; // if kernel arg dirty and curbe disabled
    }
    else if (m_Dirty & CM_KERNEL_DATA_THREAD_COUNT_DIRTY)
    {
        Reusable = false; // if thread count dirty
    }
    else if (m_pThreadSpace)
    {
       if (m_pThreadSpace->GetDirtyStatus() == CM_THREAD_SPACE_DATA_DIRTY)
       {
          Reusable = false; // if per kernel thread space exists and it is completely dirty
       }
    }
    else if (pTaskThreadSpace)
    {
       if (pTaskThreadSpace->GetDirtyStatus() == CM_THREAD_SPACE_DATA_DIRTY)
       {
          Reusable = false; // if per task thread space change and it is completely dirty
       }
    }
    return Reusable;

}

//*-----------------------------------------------------------------------------
//| Purpose:    Checks to see if kernel prologue has changed
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelRT::IsPrologueDirty( void )
{
    bool prologueDirty = false;

    if( m_ThreadCount != m_LastThreadCount )
    {
        if( m_LastThreadCount )
        {
            if( m_ThreadCount == 1 || m_LastThreadCount == 1 )
            {
                prologueDirty = true;
            }
        }
        m_LastThreadCount = m_ThreadCount;
    }

    if( m_adjustScoreboardY != m_LastAdjustScoreboardY )
    {
        if( m_LastAdjustScoreboardY )
        {
            prologueDirty = true;
        }
        m_LastAdjustScoreboardY = m_adjustScoreboardY;
    }

    return prologueDirty;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Prepare Kernel Data including thread args, kernel args
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelDataInternal(
    CmKernelData* & pKernelData,  // out
    uint32_t& kernelDataSize,         // out
    const CmThreadSpaceRT* pTS )    // in
{
    PCM_HAL_KERNEL_PARAM  pHalKernelParam       = nullptr;
    int32_t               hr                    = CM_SUCCESS;
    uint32_t              movInstNum            = 0;
    uint32_t              KrnCurbeSize          = 0;
    uint32_t              NumArgs               = 0;
    uint32_t              dwBottomRange         = 1024;
    uint32_t              dwUpRange             = 0;
    uint32_t              UnitSize              = 0;
    bool                  hasThreadArg          = false;
    CmThreadSpaceRT         *pCmThreadSpace       = nullptr;
    bool                  isKernelThreadSpace   = false;
    CM_ARG                *pTempArgs            = nullptr;
    uint32_t              ArgSize               = 0;
    uint32_t              surfNum               = 0; //Pass needed BT entry numbers to HAL CM
    CmKernelRT             *pCmKernel             = nullptr;

    if( pTS == nullptr && m_pThreadSpace!= nullptr)
    {
        pCmThreadSpace = m_pThreadSpace;
        isKernelThreadSpace = true;
    }
    else
    {
        pCmThreadSpace = const_cast<CmThreadSpaceRT*>(pTS);
    }

    CMCHK_HR(CmKernelData::Create( this, pKernelData ));
    pHalKernelParam = pKernelData->GetHalCmKernelData();
    CMCHK_NULL(pHalKernelParam);

    //Get Num of args with surface array
    CMCHK_HR(GetArgCountPlusSurfArray(ArgSize, NumArgs));

    if( NumArgs > 0)
    {
        //Create Temp args
        CMCHK_HR(CreateTempArgs(NumArgs, pTempArgs));
        //Create move instructions
        CMCHK_HR(CreateMovInstructions(movInstNum,   pHalKernelParam->movInsData, pTempArgs, NumArgs));
    }

    CMCHK_HR(CalcKernelDataSize(movInstNum, NumArgs, ArgSize, kernelDataSize));
    CMCHK_HR(pKernelData->SetKernelDataSize(kernelDataSize));

    if(!IsBatchBufferReusable(const_cast<CmThreadSpaceRT *>(pTS)))
    {
        m_Id ++;
    }

    if( IsPrologueDirty( ) )
    {
        // can't re-use kernel binary in GSH
        // just update upper 16 bits
        uint64_t tempID = m_Id;
        tempID >>= 48;
        tempID++;
        tempID <<= 48;
        // get rid of old values in upper 16 bits
        m_Id <<= 16;
        m_Id >>= 16;
        m_Id |= tempID;
    }

    pHalKernelParam->clonedKernelParam.isClonedKernel = m_IsClonedKernel;
    pHalKernelParam->clonedKernelParam.kernelID       = m_CloneKernelID;
    pHalKernelParam->clonedKernelParam.hasClones      = m_HasClones;
    pHalKernelParam->kernelId           = m_Id; // kernel id , high 32-bit is kernel id, low 32-bit is kernel data id for batch buffer reuse
    pHalKernelParam->numArgs             = NumArgs;
    pHalKernelParam->numThreads          = m_ThreadCount;
    pHalKernelParam->kernelBinarySize    = m_uiBinarySize + movInstNum * CM_MOVE_INSTRUCTION_SIZE;
    pHalKernelParam->kernelDataSize      = kernelDataSize;
    pHalKernelParam->movInsDataSize      = movInstNum * CM_MOVE_INSTRUCTION_SIZE;

    pHalKernelParam->cmFlags             = m_CurbeEnable ? CM_FLAG_CURBE_ENABLED : 0;
    pHalKernelParam->cmFlags            |= m_NonstallingScoreboardEnable ? CM_FLAG_NONSTALLING_SCOREBOARD_ENABLED : 0;
    pHalKernelParam->kernelDebugEnabled  = m_blhwDebugEnable;

    pHalKernelParam->kernelBinary        = (uint8_t*)m_pBinary;

    CMCHK_HR( pKernelData->GetCmKernel( pCmKernel ) );
    if ( pCmKernel == nullptr )
    {
        return CM_NULL_POINTER;
    }
    MOS_SecureStrcpy( pHalKernelParam->kernelName, CM_MAX_KERNEL_NAME_SIZE_IN_BYTE, pCmKernel->GetName() );

    if ( pCmThreadSpace )
    {// either from per kernel thread space or per task thread space
        CMCHK_HR(SortThreadSpace(pCmThreadSpace)); // must be called before CreateThreadArgData
    }

    for(uint32_t i =0 ; i< NumArgs; i++)
    {
        pHalKernelParam->argParams[i].unitCount        = pTempArgs[ i ].unitCount;
        pHalKernelParam->argParams[i].kind              = (CM_HAL_KERNEL_ARG_KIND)(pTempArgs[ i ].unitKind);
        pHalKernelParam->argParams[i].unitSize         = pTempArgs[ i ].unitSize;
        pHalKernelParam->argParams[i].payloadOffset    = pTempArgs[ i ].unitOffsetInPayload;
        pHalKernelParam->argParams[i].perThread        = (pTempArgs[ i ].unitCount > 1) ? true :false;
        pHalKernelParam->argParams[i].nCustomValue      = pTempArgs[ i ].nCustomValue;
        pHalKernelParam->argParams[i].aliasIndex       = pTempArgs[ i ].aliasIndex;
        pHalKernelParam->argParams[i].aliasCreated     = pTempArgs[ i ].bAliasCreated;
        pHalKernelParam->argParams[i].isNull           = pTempArgs[ i ].bIsNull;

        CreateThreadArgData(&pHalKernelParam->argParams[i], i, pCmThreadSpace, pTempArgs);

        if(CHECK_SURFACE_TYPE ( pHalKernelParam->argParams[i].kind,
            ARG_KIND_SURFACE_VME,
            ARG_KIND_SURFACE_SAMPLER,
            ARG_KIND_SURFACE2DUP_SAMPLER))
        {
            UnitSize = CM_ARGUMENT_SURFACE_SIZE;
        }
        else
        {
            UnitSize = pHalKernelParam->argParams[i].unitSize;
        }

        if (pHalKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE)
        {
            if(IsKernelArg(pHalKernelParam->argParams[i]))
            {
                // Kernel Arg : calculate curbe size & adjust payloadoffset
                // Note: Here the payloadOffset may be different from original value
                uint32_t dwOffset = pHalKernelParam->argParams[i].payloadOffset - CM_PAYLOAD_OFFSET;
                if (dwOffset >= KrnCurbeSize)
                {
                    KrnCurbeSize = dwOffset + UnitSize;
                }
                pHalKernelParam->argParams[i].payloadOffset -= CM_PAYLOAD_OFFSET;
            }
        }

        if(!IsKernelArg(pHalKernelParam->argParams[i]))
        {   //Thread Arg : Calculate payload size & adjust payloadoffset
            hasThreadArg  = true;
            pHalKernelParam->argParams[i].payloadOffset -= CM_PAYLOAD_OFFSET;

            if(pHalKernelParam->argParams[i].payloadOffset < dwBottomRange)
            {
               dwBottomRange = pHalKernelParam->argParams[i].payloadOffset;
            }
            if(pHalKernelParam->argParams[i].payloadOffset >=  dwUpRange)
            {
               dwUpRange = pHalKernelParam->argParams[i].payloadOffset + UnitSize;
            }
        }
    }

    if ( m_state_buffer_bounded != CM_STATE_BUFFER_NONE )
    {
        PCM_CONTEXT_DATA pCmData = ( PCM_CONTEXT_DATA )m_pCmDev->GetAccelData();
        PCM_HAL_STATE pState = pCmData->cmHalState;
        KrnCurbeSize = pState->pfnGetStateBufferSizeForKernel( pState, this );
        pHalKernelParam->stateBufferType = pState->pfnGetStateBufferTypeForKernel( pState, this );
    }

    pHalKernelParam->payloadSize         = hasThreadArg ? MOS_ALIGN_CEIL(dwUpRange -  dwBottomRange, 4): 0;
    pHalKernelParam->totalCurbeSize      = MOS_ALIGN_CEIL(KrnCurbeSize, 32);
    pHalKernelParam->curbeSizePerThread  = pHalKernelParam->totalCurbeSize;

    pHalKernelParam->perThreadArgExisted = hasThreadArg;

    m_SizeInCurbe = GetAlignedCurbeSize( KrnCurbeSize );

    if ( pHalKernelParam->cmFlags & CM_KERNEL_FLAGS_CURBE )
    {
        for(uint32_t i=0; i< NumArgs; i++)
        {
            if(!IsKernelArg(pHalKernelParam->argParams[i]))
            {  // thread arg: need to minus curbe size
                pHalKernelParam->argParams[i].payloadOffset -= pHalKernelParam->curbeSizePerThread;
            }
        }
    }

    //Create indirect data
    CMCHK_HR(CreateKernelIndirectData(&pHalKernelParam->indirectDataParam));

    if ( m_SamplerBTICount != 0 )
    {
        CmFastMemCopy( ( void* )pHalKernelParam->samplerBTIParam.samplerInfo, ( void* )m_SamplerBTIEntry, sizeof( m_SamplerBTIEntry ) );
        pHalKernelParam->samplerBTIParam.samplerCount = m_SamplerBTICount;

        CmSafeMemSet(m_SamplerBTIEntry, 0, sizeof(m_SamplerBTIEntry));
        m_SamplerBTICount = 0;
    }

    CalculateKernelSurfacesNum(surfNum, pHalKernelParam->numSurfaces);

    //Create thread space param: only avaliable if per kernel ts exists
    if(m_pThreadSpace)
    {
        CMCHK_HR(CreateThreadSpaceParam(&pHalKernelParam->kernelThreadSpaceParam, m_pThreadSpace));
    }

    //Get SLM size
    pHalKernelParam->slmSize = GetSLMSize();

    //Get Spill mem used
    pHalKernelParam->spillSize = GetSpillMemUsed();

    //Set Barrier mode
    pHalKernelParam->barrierMode = m_BarrierMode;

    CMCHK_HR(UpdateKernelDataGlobalSurfaceInfo( pHalKernelParam ));

    //Destroy Temp Args
    for (uint32_t j = 0; j < NumArgs; j++)
    {
        if (pTempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
        {
            MosSafeDeleteArray(pTempArgs[j].pValue);
        }
    }
    MosSafeDeleteArray( pTempArgs );

    CMCHK_HR(UpdateSamplerHeap(pKernelData));
finish:
    if(hr != CM_SUCCESS)
    {
         if(pHalKernelParam)
         {
             //Clean allocated memory
             for(uint32_t i =0 ; i< NumArgs; i++)
             {
                if( pHalKernelParam->argParams[i].firstValue )
                {
                    MosSafeDeleteArray(pHalKernelParam->argParams[i].firstValue);
                }
             }
         }

         //Destroy Temp Args
         if (pTempArgs)
         {
             for (uint32_t j = 0; j < NumArgs; j++)
             {
                 if (pTempArgs[j].unitOffsetInPayloadOrig == (uint16_t)-1)
                 {
                     MosSafeDeleteArray(pTempArgs[j].pValue);
                 }
             }
             MosSafeDeleteArray(pTempArgs);
         }
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update kernel data's kernel arg, thread arg, thread count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateKernelData(
    CmKernelData*   pKernelData,  // in
    const CmThreadSpaceRT* pTS)
{
    int32_t               hr                      = CM_SUCCESS;
    PCM_HAL_KERNEL_PARAM  pHalKernelParam         = nullptr;
    bool                  bBBresuable             = true;
    CmThreadSpaceRT         *pCmThreadSpace         = nullptr;
    bool                  isKernelThreadSpace     = false;
    uint32_t              ArgIndexStep            = 0;
    uint32_t              ArgIndex                = 0;
    uint32_t              surfNum                 = 0; //Update Number of surface used by kernel

    if( pTS == nullptr && m_pThreadSpace!= nullptr)
    {
        pCmThreadSpace = m_pThreadSpace;
        isKernelThreadSpace = true;
    }
    else
    {
        pCmThreadSpace = const_cast<CmThreadSpaceRT*>(pTS);
    }

    CMCHK_NULL(pKernelData);
    CM_ASSERT(pKernelData->IsInUse() == false);

    pHalKernelParam = pKernelData->GetHalCmKernelData();
    CMCHK_NULL(pHalKernelParam);

    if(!IsBatchBufferReusable(const_cast<CmThreadSpaceRT *>(pTS)))
    {
        m_Id ++;
        pHalKernelParam->kernelId = m_Id;
    }

    //Update arguments
    for(uint32_t OrgArgIndex =0 ; OrgArgIndex< m_ArgCount; OrgArgIndex++)
    {
        ArgIndexStep = 1;

        if ( CHECK_SURFACE_TYPE( m_Args[ OrgArgIndex ].unitKind,
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
            ArgIndexStep = m_Args[OrgArgIndex].unitSize/sizeof(int); // Surface array exists
        }
        else if (CHECK_SURFACE_TYPE(m_Args[OrgArgIndex].unitKind,  ARG_KIND_SURFACE_VME))
        {
            ArgIndexStep = m_Args[OrgArgIndex].unitVmeArraySize;
        }

        if(m_Args[ OrgArgIndex ].bIsDirty)
        {
            if(m_Args[ OrgArgIndex ].unitCount > 1)
            { // thread arg is dirty
                bBBresuable          = false;
            }

            if ( CHECK_SURFACE_TYPE( m_Args[ OrgArgIndex ].unitKind,
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

                uint32_t num_surfaces = m_Args[OrgArgIndex].unitSize/sizeof(int); // Surface array
                if(m_Args[ OrgArgIndex ].unitCount ==  1) // kernel arg
                {
                    if (num_surfaces > 1)
                    {
                        for (uint32_t kk = 0; kk < num_surfaces; kk++)
                        {
                            CM_ASSERT(pHalKernelParam->argParams[ArgIndex + kk].firstValue != nullptr);
                            CmFastMemCopy(pHalKernelParam->argParams[ArgIndex + kk].firstValue,
                                m_Args[OrgArgIndex].pValue + kk*sizeof(uint32_t), sizeof(uint32_t));
                            pHalKernelParam->argParams[ArgIndex + kk].aliasIndex = m_Args[OrgArgIndex].aliasIndex;
                            pHalKernelParam->argParams[ArgIndex + kk].aliasCreated = m_Args[OrgArgIndex].bAliasCreated;
                            pHalKernelParam->argParams[ArgIndex + kk].isNull = m_Args[OrgArgIndex].bIsNull;

                            if (!m_Args[OrgArgIndex].surfIndex[kk])
                            {
                                //if surfIndex is 0, set kind to be CM_ARGUMENT_SURFACE2D
                                //This is for special usage if there is empty element in surface array.
                                pHalKernelParam->argParams[ArgIndex + kk].kind = CM_ARGUMENT_SURFACE2D;
                                continue;
                            }

                            pHalKernelParam->argParams[ArgIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[OrgArgIndex].pSurfArrayArg[kk].argKindForArray;
                            pHalKernelParam->argParams[ArgIndex + kk].nCustomValue = m_Args[OrgArgIndex].pSurfArrayArg[kk].addressModeForArray;
                        }
                    }
                    else
                    {
                        CM_ASSERT(pHalKernelParam->argParams[ArgIndex].firstValue != nullptr);
                        CmFastMemCopy(pHalKernelParam->argParams[ArgIndex].firstValue,
                                m_Args[ OrgArgIndex ].pValue, sizeof(uint32_t));
                        pHalKernelParam->argParams[ArgIndex].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[ OrgArgIndex ].unitKind;
                        pHalKernelParam->argParams[ArgIndex].aliasIndex   = m_Args[OrgArgIndex].aliasIndex;
                        pHalKernelParam->argParams[ArgIndex].aliasCreated = m_Args[OrgArgIndex].bAliasCreated;
                        pHalKernelParam->argParams[ArgIndex].isNull = m_Args[OrgArgIndex].bIsNull;
                    }

                 }
                 else // thread arg
                 {
                    uint32_t num_surfaces = m_Args[OrgArgIndex].unitSize/sizeof(int); // Surface array
                    uint32_t *surfaces = (uint32_t *)MOS_NewArray(uint8_t, (sizeof(uint32_t) * m_Args[OrgArgIndex].unitCount));
                    CMCHK_NULL_RETURN(surfaces, CM_OUT_OF_HOST_MEMORY);
                    for (uint32_t kk=0;  kk< num_surfaces ; kk++)
                    {
                        for (uint32_t s = 0; s < m_Args[OrgArgIndex].unitCount; s++)
                        {
                            surfaces[s] = *(uint32_t *)((uint32_t *)m_Args[OrgArgIndex].pValue + kk + num_surfaces * s);
                        }
                        CmFastMemCopy(pHalKernelParam->argParams[ArgIndex + kk].firstValue,
                            surfaces, sizeof(uint32_t) * m_Args[OrgArgIndex].unitCount);

                        pHalKernelParam->argParams[ArgIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[ OrgArgIndex ].unitKind;

                        pHalKernelParam->argParams[ArgIndex + kk].aliasIndex = m_Args[OrgArgIndex].aliasIndex;
                        pHalKernelParam->argParams[ArgIndex + kk].aliasCreated = m_Args[OrgArgIndex].bAliasCreated;
                        pHalKernelParam->argParams[ArgIndex + kk].isNull = m_Args[OrgArgIndex].bIsNull;

                    }
                    MosSafeDeleteArray(surfaces);
                 }

            }
            else if (CHECK_SURFACE_TYPE(m_Args[OrgArgIndex].unitKind, ARG_KIND_SURFACE_VME))
            {
                uint32_t num_surfaces = m_Args[OrgArgIndex].unitVmeArraySize;
                if (m_Args[OrgArgIndex].unitCount == 1) // kernel arg
                {
                    uint32_t VmeSurfoffset = 0;
                    for (uint32_t kk = 0; kk< num_surfaces; kk++)
                    {
                        uint16_t VmeSize = (uint16_t)getVmeArgValueSize((PCM_HAL_VME_ARG_VALUE)(m_Args[OrgArgIndex].pValue + VmeSurfoffset));

                        // reallocate the firstValue for VME surface every time
                        // since the number of surfaces may vary
                        MosSafeDeleteArray(pHalKernelParam->argParams[ArgIndex + kk].firstValue);
                        pHalKernelParam->argParams[ArgIndex + kk].firstValue = MOS_NewArray(uint8_t, VmeSize);
                        CM_ASSERT(pHalKernelParam->argParams[ArgIndex + kk].firstValue != nullptr);
                        CmFastMemCopy(pHalKernelParam->argParams[ArgIndex + kk].firstValue,
                            m_Args[OrgArgIndex].pValue + VmeSurfoffset, VmeSize);

                        pHalKernelParam->argParams[ArgIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[OrgArgIndex].unitKind;

                        pHalKernelParam->argParams[ArgIndex + kk].aliasIndex = m_Args[OrgArgIndex].aliasIndex;
                        pHalKernelParam->argParams[ArgIndex + kk].aliasCreated = m_Args[OrgArgIndex].bAliasCreated;
                        pHalKernelParam->argParams[ArgIndex + kk].isNull = m_Args[OrgArgIndex].bIsNull;
                        pHalKernelParam->argParams[ArgIndex + kk].unitSize = VmeSize;
                        VmeSurfoffset += VmeSize;
                    }
                }
            }
            else
            {
                CMCHK_HR(CreateThreadArgData(&pHalKernelParam->argParams[ArgIndex ], OrgArgIndex, pCmThreadSpace, m_Args));
            }
        }
        ArgIndex += ArgIndexStep;
    }

    //Update Thread space param
    if(m_pThreadSpace && m_pThreadSpace->GetDirtyStatus())
    {

        CMCHK_HR(SortThreadSpace(m_pThreadSpace));

        uint32_t TsWidth, TsHeight;
        PCM_HAL_KERNEL_THREADSPACE_PARAM  pCmKernelThreadSpaceParam = &pHalKernelParam->kernelThreadSpaceParam;
        m_pThreadSpace->GetThreadSpaceSize(TsWidth, TsHeight);

        pCmKernelThreadSpaceParam->threadSpaceWidth  = (uint16_t)TsWidth;
        pCmKernelThreadSpaceParam->threadSpaceHeight = (uint16_t)TsHeight;
        m_pThreadSpace->GetDependencyPatternType(pCmKernelThreadSpaceParam->patternType);
        m_pThreadSpace->GetWalkingPattern(pCmKernelThreadSpaceParam->walkingPattern);

        CM_HAL_DEPENDENCY*     pDependency;
        m_pThreadSpace->GetDependency( pDependency);

        if(pDependency != nullptr)
        {
            CmFastMemCopy(&pCmKernelThreadSpaceParam->dependencyInfo, pDependency, sizeof(CM_HAL_DEPENDENCY));
        }

        if( m_pThreadSpace->CheckWalkingParametersSet() )
        {
            CMCHK_HR(m_pThreadSpace->GetWalkingParameters(pCmKernelThreadSpaceParam->walkingParams));
        }

        if( m_pThreadSpace->CheckDependencyVectorsSet() )
        {
            CMCHK_HR(m_pThreadSpace->GetDependencyVectors(pCmKernelThreadSpaceParam->dependencyVectors));
        }

        if(m_pThreadSpace->IsThreadAssociated())
        {// media object only
            uint32_t *pBoardOrder = nullptr;
            m_pThreadSpace->GetBoardOrder(pBoardOrder);
            CMCHK_NULL(pBoardOrder);

            CM_THREAD_SPACE_UNIT *pThreadSpaceUnit = nullptr;
            m_pThreadSpace->GetThreadSpaceUnit(pThreadSpaceUnit);
            CMCHK_NULL(pThreadSpaceUnit);

            pCmKernelThreadSpaceParam->reuseBBUpdateMask = 0;
            for(uint32_t i=0; i< TsWidth * TsHeight ; i++)
            {
                pCmKernelThreadSpaceParam->threadCoordinates[i].x = pThreadSpaceUnit[pBoardOrder[i]].scoreboardCoordinates.x;
                pCmKernelThreadSpaceParam->threadCoordinates[i].y = pThreadSpaceUnit[pBoardOrder[i]].scoreboardCoordinates.y;
                pCmKernelThreadSpaceParam->threadCoordinates[i].mask = pThreadSpaceUnit[pBoardOrder[i]].dependencyMask;
                pCmKernelThreadSpaceParam->threadCoordinates[i].resetMask = pThreadSpaceUnit[pBoardOrder[i]].reset;
                pCmKernelThreadSpaceParam->threadCoordinates[i].color = pThreadSpaceUnit[pBoardOrder[i]].scoreboardColor;
                pCmKernelThreadSpaceParam->threadCoordinates[i].sliceSelect = pThreadSpaceUnit[pBoardOrder[i]].sliceDestinationSelect;
                pCmKernelThreadSpaceParam->threadCoordinates[i].subSliceSelect = pThreadSpaceUnit[pBoardOrder[i]].subSliceDestinationSelect;
                pCmKernelThreadSpaceParam->reuseBBUpdateMask |= pThreadSpaceUnit[pBoardOrder[i]].reset;
            }

            if( pCmKernelThreadSpaceParam->patternType == CM_WAVEFRONT26Z )
            {
                CM_HAL_WAVEFRONT26Z_DISPATCH_INFO dispatchInfo;
                m_pThreadSpace->GetWavefront26ZDispatchInfo(dispatchInfo);

                if (pCmKernelThreadSpaceParam->dispatchInfo.numWaves >= dispatchInfo.numWaves)
                {
                    pCmKernelThreadSpaceParam->dispatchInfo.numWaves = dispatchInfo.numWaves;
                    CmFastMemCopy(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, dispatchInfo.numThreadsInWave, dispatchInfo.numWaves*sizeof(uint32_t));
                }
                else
                {
                    pCmKernelThreadSpaceParam->dispatchInfo.numWaves = dispatchInfo.numWaves;
                    MosSafeDeleteArray(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave);
                    pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave = MOS_NewArray(uint32_t, dispatchInfo.numWaves);
                    CMCHK_NULL_RETURN(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, CM_OUT_OF_HOST_MEMORY);
                    CmFastMemCopy(pCmKernelThreadSpaceParam->dispatchInfo.numThreadsInWave, dispatchInfo.numThreadsInWave, dispatchInfo.numWaves*sizeof(uint32_t));
                }
            }
        }
    }


    // Update indirect data
    if( m_Dirty & CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY)
    {
        pHalKernelParam->indirectDataParam.indirectDataSize = m_usKernelPayloadDataSize;
        pHalKernelParam->indirectDataParam.surfaceCount     = m_usKernelPayloadSurfaceCount;

        if(m_usKernelPayloadDataSize != 0)
        {
            if(m_Dirty & CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY)
            { // size change, need to reallocate
                MosSafeDeleteArray(pHalKernelParam->indirectDataParam.indirectData);
                pHalKernelParam->indirectDataParam.indirectData = MOS_NewArray(uint8_t, m_usKernelPayloadDataSize);
                CMCHK_NULL_RETURN(pHalKernelParam->indirectDataParam.indirectData, CM_OUT_OF_HOST_MEMORY);
            }
            CmFastMemCopy(pHalKernelParam->indirectDataParam.indirectData, (void *)m_pKernelPayloadData, m_usKernelPayloadDataSize);
        }

        if(m_usKernelPayloadSurfaceCount != 0)
        {
            if(m_Dirty & CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY)
            { // size change, need to reallocate
                MosSafeDeleteArray(pHalKernelParam->indirectDataParam.surfaceInfo);
                pHalKernelParam->indirectDataParam.surfaceInfo = MOS_NewArray(CM_INDIRECT_SURFACE_INFO, m_usKernelPayloadSurfaceCount);
                CMCHK_NULL_RETURN(pHalKernelParam->indirectDataParam.surfaceInfo, CM_OUT_OF_HOST_MEMORY);

            }
            CmFastMemCopy((void*)pHalKernelParam->indirectDataParam.surfaceInfo, (void*)m_IndirectSurfaceInfoArray,
                             m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
            //clear m_IndirectSurfaceInfoArray every enqueue
            CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
            m_usKernelPayloadSurfaceCount = 0;
        }
    }

    if (m_Dirty & CM_KERNEL_DATA_SAMPLER_BTI_DIRTY)
    {
        if ( m_SamplerBTICount != 0 )
        {
            CmFastMemCopy( ( void* )pHalKernelParam->samplerBTIParam.samplerInfo, ( void* )m_SamplerBTIEntry, sizeof( m_SamplerBTIEntry ) );
            pHalKernelParam->samplerBTIParam.samplerCount = m_SamplerBTICount;

            CmSafeMemSet(m_SamplerBTIEntry, 0, sizeof(m_SamplerBTIEntry));
            m_SamplerBTICount = 0;
        }
    }
    CMCHK_HR(UpdateKernelDataGlobalSurfaceInfo( pHalKernelParam ));

    CMCHK_HR(CalculateKernelSurfacesNum(surfNum, pHalKernelParam->numSurfaces));

    CMCHK_HR(UpdateSamplerHeap(pKernelData));

finish:
    if( hr != CM_SUCCESS)
    {
        if( pHalKernelParam )
        {
            MosSafeDeleteArray(pHalKernelParam->indirectDataParam.indirectData);
            MosSafeDeleteArray(pHalKernelParam->indirectDataParam.surfaceInfo);
        }
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Update kernel data's kernel arg, thread arg, thread count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateKernelData(
    CmKernelData*   pKernelData,  // in
    const CmThreadGroupSpace* pTGS )    // in
{
    int32_t               hr                      = CM_SUCCESS;
    PCM_HAL_KERNEL_PARAM  pHalKernelParam         = nullptr;
    uint32_t              ArgIndexStep            = 0;
    uint32_t              ArgIndex                = 0;
    uint32_t              surfNum                 = 0;
    auto getVersionAsInt = [](int major, int minor) { return major * 100 + minor; };

    CMCHK_NULL(pKernelData);
    CM_ASSERT(pKernelData->IsInUse() == false);

    pHalKernelParam = pKernelData->GetHalCmKernelData();
    CMCHK_NULL(pHalKernelParam);

    CMCHK_NULL(pTGS);

    //Update arguments
    for(uint32_t OrgArgIndex =0 ; OrgArgIndex< m_ArgCount; OrgArgIndex++)
    {
        ArgIndexStep = 1;

        if ( CHECK_SURFACE_TYPE( m_Args[ OrgArgIndex ].unitKind,
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
            ArgIndexStep = m_Args[OrgArgIndex].unitSize/sizeof(int); // Surface array exists
        }
        else if (CHECK_SURFACE_TYPE(m_Args[OrgArgIndex].unitKind, ARG_KIND_SURFACE_VME))
        {
            ArgIndexStep = m_Args[OrgArgIndex].unitVmeArraySize;
        }

        if(m_Args[ OrgArgIndex ].bIsDirty)
        {
            if(m_Args[ OrgArgIndex ].unitCount > 1)
            { // thread arg is dirty
                CM_ASSERTMESSAGE("Error: Thread arg is not allowed in GPGPU walker.");
                hr = CM_FAILURE; // Thread arg is not allowed in GPGPU walker
                goto finish;
            }

            if ( CHECK_SURFACE_TYPE( m_Args[ OrgArgIndex ].unitKind,
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
                uint32_t num_surfaces = m_Args[OrgArgIndex].unitSize/sizeof(int); // Surface array
                if(m_Args[ OrgArgIndex ].unitCount ==  1) // kernel arg
                {
                    if (num_surfaces > 1 )
                    {
                        for(uint32_t kk=0;  kk< num_surfaces ; kk++)
                        {
                            CM_ASSERT(pHalKernelParam->argParams[ArgIndex + kk].firstValue != nullptr);
                            CmFastMemCopy(pHalKernelParam->argParams[ArgIndex + kk].firstValue,
                            m_Args[ OrgArgIndex ].pValue + kk*sizeof(uint32_t), sizeof(uint32_t));

                            if (!m_Args[OrgArgIndex].surfIndex[kk])
                            {
                                //if surfIndex is 0, set kind to be CM_ARGUMENT_SURFACE2D
                                //This is for special usage if there is empty element in surface array.
                                pHalKernelParam->argParams[ArgIndex + kk].kind = CM_ARGUMENT_SURFACE2D;
                                continue;
                            }
                            pHalKernelParam->argParams[ArgIndex + kk].isNull = m_Args[OrgArgIndex].bIsNull;
                            pHalKernelParam->argParams[ArgIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[OrgArgIndex].pSurfArrayArg[kk].argKindForArray;
                            pHalKernelParam->argParams[ArgIndex + kk].nCustomValue = m_Args[OrgArgIndex].pSurfArrayArg[kk].addressModeForArray;

                        }
                    }
                    else
                    {
                        CM_ASSERT(pHalKernelParam->argParams[ArgIndex].firstValue != nullptr);
                        CmFastMemCopy(pHalKernelParam->argParams[ArgIndex].firstValue,
                            m_Args[OrgArgIndex].pValue, sizeof(uint32_t));

                        pHalKernelParam->argParams[ArgIndex].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[OrgArgIndex].unitKind;
                        pHalKernelParam->argParams[ArgIndex].isNull = m_Args[OrgArgIndex].bIsNull;
                    }
                 }
            }
            else if (CHECK_SURFACE_TYPE(m_Args[OrgArgIndex].unitKind, ARG_KIND_SURFACE_VME))
            {
                uint32_t num_surfaces = m_Args[OrgArgIndex].unitVmeArraySize;
                if (m_Args[OrgArgIndex].unitCount == 1) // kernel arg
                {
                    uint32_t VmeSurfoffset = 0;
                    for (uint32_t kk = 0; kk< num_surfaces; kk++)
                    {
                        uint32_t VmeSize = getVmeArgValueSize((PCM_HAL_VME_ARG_VALUE)(m_Args[OrgArgIndex].pValue + VmeSurfoffset));

                        // reallocate the firstValue for VME surface every time
                        // since the number of surfaces may vary
                        MosSafeDeleteArray(pHalKernelParam->argParams[ArgIndex + kk].firstValue);
                        pHalKernelParam->argParams[ArgIndex + kk].firstValue = MOS_NewArray(uint8_t, VmeSize);
                        CM_ASSERT(pHalKernelParam->argParams[ArgIndex + kk].firstValue != nullptr);
                        CmFastMemCopy(pHalKernelParam->argParams[ArgIndex + kk].firstValue,
                            m_Args[OrgArgIndex].pValue + VmeSurfoffset, VmeSize);

                        pHalKernelParam->argParams[ArgIndex + kk].kind = (CM_HAL_KERNEL_ARG_KIND)m_Args[OrgArgIndex].unitKind;

                        pHalKernelParam->argParams[ArgIndex + kk].aliasIndex = m_Args[OrgArgIndex].aliasIndex;
                        pHalKernelParam->argParams[ArgIndex + kk].aliasCreated = m_Args[OrgArgIndex].bAliasCreated;
                        pHalKernelParam->argParams[ArgIndex + kk].isNull = m_Args[OrgArgIndex].bIsNull;
                        pHalKernelParam->argParams[ArgIndex + kk].unitSize = m_Args[OrgArgIndex].unitSize;
                        VmeSurfoffset += VmeSize;
                    }
                }
            }
            else
            {
                CMCHK_HR(CreateThreadArgData(&pHalKernelParam->argParams[ArgIndex ], OrgArgIndex, nullptr, m_Args));
            }
        }
        ArgIndex += ArgIndexStep;
    }

    if (m_Dirty & CM_KERNEL_DATA_SAMPLER_BTI_DIRTY)
    {
        if ( m_SamplerBTICount != 0 )
        {
            CmFastMemCopy( ( void* )pHalKernelParam->samplerBTIParam.samplerInfo, ( void* )m_SamplerBTIEntry, sizeof( m_SamplerBTIEntry ) );
            pHalKernelParam->samplerBTIParam.samplerCount = m_SamplerBTICount;

            CmSafeMemSet(m_SamplerBTIEntry, 0, sizeof(m_SamplerBTIEntry));
            m_SamplerBTICount = 0;
        }
    }

    CMCHK_HR(UpdateKernelDataGlobalSurfaceInfo( pHalKernelParam ));

    CMCHK_HR(CalculateKernelSurfacesNum(surfNum, pHalKernelParam->numSurfaces));

    // GPGPU walker - implicit args
    uint32_t thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth;
    pTGS->GetThreadGroupSpaceSize(thrdSpaceWidth, thrdSpaceHeight, thrdSpaceDepth, grpSpaceWidth, grpSpaceHeight, grpSpaceDepth);

    if (getVersionAsInt(m_pProgram->m_CISA_majorVersion, m_pProgram->m_CISA_minorVersion) < getVersionAsInt(3, 3))
    {
        CMCHK_HR(CreateKernelArgDataGroup (pHalKernelParam->argParams[ArgIndex + 0].firstValue, thrdSpaceWidth));
        CMCHK_HR(CreateKernelArgDataGroup (pHalKernelParam->argParams[ArgIndex + 1].firstValue, thrdSpaceHeight));
        CMCHK_HR(CreateKernelArgDataGroup (pHalKernelParam->argParams[ArgIndex + 2].firstValue, grpSpaceWidth));
        CMCHK_HR(CreateKernelArgDataGroup (pHalKernelParam->argParams[ArgIndex + 3].firstValue, grpSpaceHeight));
        CMCHK_HR(CreateKernelArgDataGroup (pHalKernelParam->argParams[ArgIndex + 4].firstValue, thrdSpaceWidth));
        CMCHK_HR(CreateKernelArgDataGroup (pHalKernelParam->argParams[ArgIndex + 5].firstValue, thrdSpaceHeight));
    }

    CMCHK_HR(UpdateSamplerHeap(pKernelData));
finish:
    return hr;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Create kernel indirect data
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CreateKernelIndirectData(
    PCM_HAL_INDIRECT_DATA_PARAM  pHalIndreictData )    // in/out
{
    int32_t hr = CM_SUCCESS;

    pHalIndreictData->indirectDataSize = m_usKernelPayloadDataSize;
    pHalIndreictData->surfaceCount     = m_usKernelPayloadSurfaceCount;

    if( pHalIndreictData->indirectData == nullptr &&  m_usKernelPayloadDataSize != 0)
    {
        pHalIndreictData->indirectData = MOS_NewArray(uint8_t, pHalIndreictData->indirectDataSize);
        CMCHK_NULL_RETURN(pHalIndreictData->indirectData, CM_OUT_OF_HOST_MEMORY);
    }

    // For future kernel data, pKbyte is starting point
    if( pHalIndreictData->surfaceInfo == nullptr &&  m_usKernelPayloadSurfaceCount != 0)
    {
        pHalIndreictData->surfaceInfo = MOS_NewArray(CM_INDIRECT_SURFACE_INFO, pHalIndreictData->surfaceCount);
        CMCHK_NULL_RETURN(pHalIndreictData->surfaceInfo, CM_OUT_OF_HOST_MEMORY);
    }

    if(m_usKernelPayloadDataSize != 0)
    {
        CmFastMemCopy(pHalIndreictData->indirectData, (void *)m_pKernelPayloadData, m_usKernelPayloadDataSize);
    }

    if(m_usKernelPayloadSurfaceCount != 0)
    {
        CmFastMemCopy((void*)pHalIndreictData->surfaceInfo, (void*)m_IndirectSurfaceInfoArray,
                    m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
        //clear m_IndirectSurfaceInfoArray every enqueue
        CmSafeMemSet(m_IndirectSurfaceInfoArray, 0, m_usKernelPayloadSurfaceCount * sizeof(CM_INDIRECT_SURFACE_INFO));
        m_usKernelPayloadSurfaceCount = 0;
    }
finish:
    if( hr != CM_SUCCESS)
    {
        if(pHalIndreictData->indirectData)                 MosSafeDeleteArray(pHalIndreictData->indirectData);
        if(pHalIndreictData->surfaceInfo)                  MosSafeDeleteArray(pHalIndreictData->surfaceInfo);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    UpdateLastKernelData
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::UpdateLastKernelData(
    CmKernelData* & pKernelData)    // in
{
    int32_t hr = CM_SUCCESS;

    if( pKernelData == nullptr || m_pLastKernelData == pKernelData )
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel data handle.");
        return CM_NULL_POINTER;
    }

    if(m_pLastKernelData)
    {
        CmKernelData::Destroy(m_pLastKernelData); // reduce ref count or delete it
    }
    CSync* pKernelLock = m_pCmDev->GetProgramKernelLock();
    CLock locker(*pKernelLock);
    m_pLastKernelData = pKernelData;
    m_pLastKernelData->Acquire();
    m_LastKernelDataSize = m_pLastKernelData->GetKernelDataSize();

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Wrapper of  CmKernelData::Destroy.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::ReleaseKernelData(
    CmKernelData* & pKernelData)
{
    int32_t hr = CM_SUCCESS;

    if( pKernelData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel data handle.");
        return CM_NULL_POINTER;
    }

    CSync* pKernelLock = m_pCmDev->GetProgramKernelLock();
    CLock locker(*pKernelLock);

    if(m_pLastKernelData == pKernelData)
    {
        // If the kernel data is the last kernel data
        // Need to update m_pLastKernelData.
        hr = CmKernelData::Destroy(m_pLastKernelData);
    }
    else
    {
        hr = CmKernelData::Destroy(pKernelData);
    }

    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Acquire Kernel and Program
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::AcquireKernelProgram()
{
    CSync* pKernelLock = m_pCmDev->GetProgramKernelLock();
    CLock locker(*pKernelLock);

    this->Acquire(); // increase kernel's ref count
    m_pProgram->Acquire(); // increase program's ref count

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Acquire KenrelData, Kernel and Program
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::AcquireKernelData(
    CmKernelData * &pKernelData)
{
    int32_t hr = CM_SUCCESS;

    if (pKernelData == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel data handle.");
        return CM_NULL_POINTER;
    }

    CSync* pKernelLock = m_pCmDev->GetProgramKernelLock();
    CLock locker(*pKernelLock);
    pKernelData->Acquire(); // increase kernel data's ref count

    return hr;
}

void CmKernelRT::SetAsClonedKernel(uint32_t cloneKernelID)
{
    m_IsClonedKernel = true;
    m_CloneKernelID = cloneKernelID;
}

bool CmKernelRT::GetCloneKernelID(uint32_t& cloneKernelID)
{
    if (m_IsClonedKernel)
    {
        cloneKernelID = m_CloneKernelID;
        return true;
    }

    return false;
}

void CmKernelRT::SetHasClones()
{
    m_HasClones = true;
}

//*-----------------------------------------------------------------------------
//| Purpose:   Clone/copy current kernel
//| Returns:   New kernel with content of source kernel
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::CloneKernel(CmKernelRT *& pKernelOut, uint32_t id)
{
    int32_t hr = CM_SUCCESS;

    CSync* pKernelLock = m_pCmDev->GetProgramKernelLock();
    CLock locker(*pKernelLock);

    CmDynamicArray * pKernelArray = m_pCmDev->GetKernelArray();

    uint32_t freeSlotinKernelArray = pKernelArray->GetFirstFreeIndex();

    hr = Create(m_pCmDev, m_pProgram, (char*)GetName(), freeSlotinKernelArray, id, pKernelOut, m_Options);

    if (hr == CM_SUCCESS)
    {
        pKernelOut->SetAsClonedKernel(m_Id >> 32);
        pKernelArray->SetElement(freeSlotinKernelArray, pKernelOut);
        uint32_t *kernelCount = m_pCmDev->GetKernelCount();
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
    m_IndexInTask = index;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel's index in one task
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetIndexInTask(void)
{
    return m_IndexInTask;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Associated Flag
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmKernelRT::SetAssociatedToTSFlag(bool b)
{
    m_AssociatedToTS = b;
    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose: Set threadspace for kernel
//| Returns: Result of the operation.
//| Note: It's exclusive with AssociateThreadGroupSpace()
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::AssociateThreadSpace(CmThreadSpace *&pThreadSpace)
{
    if( pThreadSpace == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Pointer to thread space is null.");
        return CM_INVALID_ARG_VALUE;
    }
    if (m_pThreadGroupSpace != nullptr)
    {
        CM_ASSERTMESSAGE("Error: It's exclusive with AssociateThreadGroupSpace().");
        return CM_INVALID_KERNEL_THREADSPACE;
    }

    bool TSChanged = false;
    if( m_pThreadSpace )
    {
        if( m_pThreadSpace != static_cast<CmThreadSpaceRT *>(pThreadSpace) )
        {
            TSChanged = true;
        }
    }

    m_pThreadSpace = static_cast<CmThreadSpaceRT *>(pThreadSpace);

    uint32_t threadSpaceWidth = 0;
    uint32_t threadSpaceHeight = 0;
    m_pThreadSpace->GetThreadSpaceSize(threadSpaceWidth, threadSpaceHeight);
    uint32_t threadCount = threadSpaceWidth * threadSpaceHeight;
    if (m_ThreadCount)
    {
        // Setting threadCount twice with different values will cause reset of kernels
        if (m_ThreadCount != threadCount)
        {
            m_ThreadCount = threadCount;
            m_Dirty |= CM_KERNEL_DATA_THREAD_COUNT_DIRTY;
        }
    }
    else // first time
    {
        m_ThreadCount = threadCount;
    }

    if( TSChanged )
    {
        m_pThreadSpace->SetDirtyStatus( CM_THREAD_SPACE_DATA_DIRTY);
    }

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Set thread group space for kernel
//| Returns: Result of the operation.
//| Note: It's exclusive with AssociateThreadSpace()
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::AssociateThreadGroupSpace(CmThreadGroupSpace *&pTGS)
{
    if( pTGS == nullptr )
    {
        CM_ASSERTMESSAGE("Error: Invalid null pointer.");
        return CM_INVALID_ARG_VALUE;
    }

    if (m_pThreadSpace != nullptr)
    {
        CM_ASSERTMESSAGE("Error: It's exclusive with AssociateThreadSpace().");
        return CM_INVALID_KERNEL_THREADGROUPSPACE;
    }

    m_pThreadGroupSpace = pTGS;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose: Create a surface in the surface manager array, return the surface index
//| Returns: Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API CM_RETURN_CODE CmKernelRT::GetIndexForCurbeData( uint32_t curbe_data_size, SurfaceIndex *surface_index )
{
    CM_RETURN_CODE hr = CM_SUCCESS;

    PCM_CONTEXT_DATA pCmData = ( PCM_CONTEXT_DATA )m_pCmDev->GetAccelData();
    PCM_HAL_STATE pState = pCmData->cmHalState;
    PRENDERHAL_MEDIA_STATE media_state_ptr = nullptr;
    void  *temp_ptr = nullptr;
    CmStateBuffer *state_buffer = nullptr;

    if ( pState->bDynamicStateHeap == false )
    {
        // Currently only support it when dynamic state heap is enabled
        return CM_FAILED_TO_CREATE_CURBE_SURFACE;
    }

    CMCHK_HR( m_pSurfaceMgr->CreateMediaStateByCurbeSize( temp_ptr, curbe_data_size ) );
    media_state_ptr = static_cast< PRENDERHAL_MEDIA_STATE >( temp_ptr );
    CMCHK_HR( m_pSurfaceMgr->CreateStateBuffer( CM_STATE_BUFFER_CURBE, curbe_data_size, media_state_ptr, this, state_buffer ) );

    if ( ( state_buffer != nullptr ) && ( media_state_ptr != nullptr ) )
    {
        // Get curbe address, ideally the DSH should provide the API to get all of the GFX VA of different part of the heap
        uint64_t curbe_gfx_va = pState->pOsInterface->pfnGetResourceGfxAddress( pState->pOsInterface, &( media_state_ptr->pDynamicState->pMemoryBlock->pStateHeap->resHeap ) ) +
            media_state_ptr->pDynamicState->pMemoryBlock->dwDataOffset + media_state_ptr->pDynamicState->Curbe.dwOffset;

        SurfaceIndex *temp_index = nullptr;
        uint32_t handle = 0;
        state_buffer->GetIndex( temp_index );
        state_buffer->GetHandle( handle );
        if ( temp_index != nullptr )
        {
            *surface_index = *temp_index;
            pState->pfnInsertToStateBufferList( pState, this, handle, CM_STATE_BUFFER_CURBE, curbe_data_size, curbe_gfx_va, media_state_ptr );
        }
        else
        {
            // it means the state_buffer was not created successfully, null pointer failure
            return CM_FAILED_TO_CREATE_CURBE_SURFACE;
        }
    }
    else
    {
        // null pointer failure
        return CM_FAILED_TO_CREATE_CURBE_SURFACE;
    }

    m_state_buffer_bounded = CM_STATE_BUFFER_CURBE;
finish:
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose: Clear threadspace for kernel
//| Returns: Result of the operation.
//*-----------------------------------------------------------------------------
CM_RT_API int32_t CmKernelRT::DeAssociateThreadSpace(CmThreadSpace * &pThreadSpace)
{
    if (pThreadSpace == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to thread space is null.");
        return CM_NULL_POINTER;
    }
    if (m_pThreadSpace != static_cast<CmThreadSpaceRT *>(pThreadSpace))
    {
        CM_ASSERTMESSAGE("Error: Invalid thread space handle.");
        return CM_INVALID_ARG_VALUE;
    }
    m_pThreadSpace = nullptr;

    return CM_SUCCESS;
}
//*--------------------------------------------------------------------------------------------
//| Purpose: query spill memory size, the function can only take effect when jitter is enabled
//| Return: Result of the operation.
//*---------------------------------------------------------------------------------------------

CM_RT_API int32_t CmKernelRT::QuerySpillSize(uint32_t &spillMemorySize)
{
    CM_KERNEL_INFO  *pKernelInfo;

    int32_t hr = m_pProgram->GetKernelInfo(m_kernelIndex, pKernelInfo);
    if (hr != CM_SUCCESS || pKernelInfo == nullptr)
        return hr;

    if (m_pProgram->IsJitterEnabled()) {
        if (pKernelInfo->jitInfo != nullptr) {
            spillMemorySize = (pKernelInfo->jitInfo)->spillMemUsed;
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
int32_t CmKernelRT::DeAssociateThreadGroupSpace(CmThreadGroupSpace * &pThreadGroupSpace)
{
    if (pThreadGroupSpace == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid null pointer.");
        return CM_NULL_POINTER;
    }
    if (m_pThreadGroupSpace != pThreadGroupSpace)
    {
        CM_ASSERTMESSAGE("Error: Invalid thread group space handle.");
        return CM_INVALID_ARG_VALUE;
    }
    m_pThreadGroupSpace = nullptr;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Indicate whether thread arg existed.
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
bool CmKernelRT::IsThreadArgExisted()
{
    return m_blPerThreadArgExists;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the size of Shared Local Memory
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetSLMSize()
{
    return (uint32_t)m_pKernelInfo->kernelSLMSize;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the spill size of the kernel from JIT
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetSpillMemUsed()
{
    uint32_t uiSpillSize;

    if (m_pProgram->IsJitterEnabled() && m_pKernelInfo->jitInfo != nullptr)
    {
        uiSpillSize = (m_pKernelInfo->jitInfo)->spillMemUsed;
    }
    else
    {
        // kernel uses "--nojitter" option, use spill size indicated by client during device creation
        uiSpillSize = m_pHalMaxValues->maxSpillSizePerHwThread;
    }

    return uiSpillSize;
}

int32_t CmKernelRT::SearchAvailableIndirectSurfInfoTableEntry(uint16_t kind, uint32_t surfaceIndex, uint32_t BTIndex)
{
    uint16_t i = 0;
    for ( i = 0; i < CM_MAX_STATIC_SURFACE_STATES_PER_BT; i++ )
    {
        if ( ( ( m_IndirectSurfaceInfoArray[ i ].surfaceIndex == surfaceIndex ) && ( m_IndirectSurfaceInfoArray[ i ].kind == kind ) && ( m_IndirectSurfaceInfoArray[ i ].bindingTableIndex == BTIndex ) ) ||
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
int32_t CmKernelRT::SetSurfBTINumForIndirectData(CM_SURFACE_FORMAT format, CM_ENUM_CLASS_TYPE SurfaceType)
{
    if (SurfaceType == CM_ENUM_CLASS_TYPE_CMBUFFER_RT)
    {
        return 1;
    }
    else
    {
        if ((format == CM_SURFACE_FORMAT_NV12) ||
            (format == CM_SURFACE_FORMAT_P010) ||
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
CM_RT_API int32_t CmKernelRT::SetSurfaceBTI(SurfaceIndex* pSurface, uint32_t BTIndex)
{

    uint32_t                    width, height, bytesPerPixel;
    CM_SURFACE_FORMAT           format = CM_SURFACE_FORMAT_INVALID;
    //Sanity check
    if (pSurface == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Pointer to surface is null.");
        return CM_NULL_POINTER;
    }
    if (!m_pSurfaceMgr->IsValidSurfaceIndex(BTIndex))
    {
        CM_ASSERTMESSAGE("Error: Invalid binding table index.");
        return CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX;
    }

    //Sanity check: if the BTI has been used once enqueue
    uint32_t i = 0;
    for (i = 0; i < m_usKernelPayloadSurfaceCount; i++)
    {
        if (m_IndirectSurfaceInfoArray[i].bindingTableIndex == (uint16_t)BTIndex)
        {
            CM_ASSERTMESSAGE("Error: Binding table index has been used once enqueue.");
            return CM_KERNELPAYLOAD_SURFACE_INVALID_BTINDEX;
        }
    }

    uint32_t index = pSurface->get_data();
    uint32_t handle = 0;

    CmSurface* pSurface_RT = nullptr;
    m_pSurfaceMgr->GetSurface( index, pSurface_RT );
    if(pSurface_RT == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid surface.");
        return CM_NULL_POINTER;
    }

    CmSurface2DRT* pSurf2D = nullptr;
    uint32_t IndirectSurfInfoEntry = 0;
    if ( pSurface_RT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACE2D )
    {
        pSurf2D = static_cast< CmSurface2DRT* >( pSurface_RT );
        pSurf2D->GetHandle( handle );
        IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_2D, handle, BTIndex);
        if (IndirectSurfInfoEntry == CM_FAILURE)
        {
            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
            return CM_FAILURE;
        }
        m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_2D;
        m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
        pSurf2D->GetSurfaceDesc(width, height, format, bytesPerPixel);
    }
    else
    {
        CmBuffer_RT* pCmBuffer = nullptr;
        if ( pSurface_RT->Type() == CM_ENUM_CLASS_TYPE_CMBUFFER_RT )
        {
            pCmBuffer = static_cast< CmBuffer_RT* >( pSurface_RT );
            pCmBuffer->GetHandle( handle );
            IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_1D, handle, BTIndex);
            if (IndirectSurfInfoEntry == CM_FAILURE)
            {
                CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                return CM_FAILURE;
            }
            m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_1D;
            m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
        }
        else
        {
            CmSurface2DUPRT* pSurf2DUP = nullptr;
            if ( pSurface_RT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACE2DUP )
            {
                pSurf2DUP = static_cast< CmSurface2DUPRT* >( pSurface_RT );
                pSurf2DUP->GetHandle( handle );
                IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_2D_UP, handle, BTIndex);
                if (IndirectSurfInfoEntry == CM_FAILURE)
                {
                    CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                    return CM_FAILURE;
                }
                m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_2D_UP;
                m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
                pSurf2DUP->GetSurfaceDesc(width, height, format, bytesPerPixel);
            }
            else
            {
                CmSurfaceSampler* pSurfSampler = nullptr;
                if ( pSurface_RT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER )
                {
                    pSurfSampler = static_cast< CmSurfaceSampler* >(pSurface_RT);

                    //Get  actually SurfaceIndex ID for 2D
                    uint16_t SurfIndexForCurrent = 0;
                    pSurfSampler->GetCmIndexCurrent(SurfIndexForCurrent);
                    CmSurface* pSurfSamp_RT= nullptr;
                    m_pSurfaceMgr->GetSurface(SurfIndexForCurrent, pSurfSamp_RT);
                    if(pSurfSamp_RT == nullptr)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid surface.");
                        return CM_NULL_POINTER;
                    }

                    SAMPLER_SURFACE_TYPE SurfaceType;
                    pSurfSampler->GetSurfaceType(SurfaceType);
                    pSurfSampler->GetHandle( handle );
                    if ( SurfaceType == SAMPLER_SURFACE_TYPE_2D )
                    {
                        CmSurface2DRT* pSurfSamp_2D = nullptr;
                        pSurfSamp_2D = static_cast<CmSurface2DRT*>(pSurfSamp_RT);
                        pSurfSamp_2D->GetSurfaceDesc(width, height, format, bytesPerPixel);

                        IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_SAMPLER, handle, BTIndex);
                        if (IndirectSurfInfoEntry == CM_FAILURE)
                        {
                            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                            return CM_FAILURE;
                        }
                        m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_SAMPLER;
                    }
                    else if ( SurfaceType == SAMPLER_SURFACE_TYPE_2DUP )
                    {
                        CmSurface2DUPRT* pSurfSamp2DUP = nullptr;
                        pSurfSamp2DUP = static_cast<CmSurface2DUPRT*>(pSurfSamp_RT);
                        pSurfSamp2DUP->GetSurfaceDesc(width, height, format, bytesPerPixel);

                        IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE2DUP_SAMPLER, handle, BTIndex);
                        if (IndirectSurfInfoEntry == CM_FAILURE)
                        {
                            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                            return CM_FAILURE;
                        }
                        m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE2DUP_SAMPLER;
                    }
                    else if ( SurfaceType == SAMPLER_SURFACE_TYPE_3D )
                    {
                        IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_3D, handle, BTIndex);
                        if (IndirectSurfInfoEntry == CM_FAILURE)
                        {
                            CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                            return CM_FAILURE;
                        }
                        m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_3D;
                    }
                    m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
                }
                else
                {
                    CmSurfaceSampler8x8* pSurfSampler8x8 = nullptr;
                    if ( pSurface_RT->Type() == CM_ENUM_CLASS_TYPE_CMSURFACESAMPLER8X8 )
                    {
                        pSurfSampler8x8 = static_cast< CmSurfaceSampler8x8* >( pSurface_RT );
                        pSurfSampler8x8->GetIndexCurrent( handle );

                        //Get  actually SurfaceIndex ID for 2D
                        uint16_t SurfIndexForCurrent = 0;
                        pSurfSampler8x8->GetCmIndex(SurfIndexForCurrent);
                        CmSurface* pSurfSamp8x8_RT = nullptr;
                        m_pSurfaceMgr->GetSurface(SurfIndexForCurrent, pSurfSamp8x8_RT);
                        if(pSurfSamp8x8_RT == nullptr)
                        {
                            CM_ASSERTMESSAGE("Error: Invalid surface.");
                            return CM_NULL_POINTER;
                        }

                        CmSurface2DRT* pSurfSamp8x8_2D = nullptr;
                        pSurfSamp8x8_2D = static_cast<CmSurface2DRT*>(pSurfSamp8x8_RT);
                        pSurfSamp8x8_2D->GetSurfaceDesc(width, height, format, bytesPerPixel);

                        if ( pSurfSampler8x8->GetSampler8x8SurfaceType() == CM_AVS_SURFACE )
                        {
                            IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_SAMPLER8X8_AVS, handle, BTIndex);
                            if (IndirectSurfInfoEntry == CM_FAILURE)
                            {
                                CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                                return CM_FAILURE;
                            }
                            m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_SAMPLER8X8_AVS;
                        }
                        else if ( pSurfSampler8x8->GetSampler8x8SurfaceType() == CM_VA_SURFACE )
                        {
                            IndirectSurfInfoEntry = SearchAvailableIndirectSurfInfoTableEntry(ARG_KIND_SURFACE_SAMPLER8X8_VA, handle, BTIndex);
                            if (IndirectSurfInfoEntry == CM_FAILURE)
                            {
                                CM_ASSERTMESSAGE("Error: Can not get available indirect surface info table entry.");
                                return CM_FAILURE;
                            }
                            m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].kind = ARG_KIND_SURFACE_SAMPLER8X8_VA;
                        }
                        m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].surfaceIndex = (uint16_t)handle;
                    }
                    else
                    {
                            return CM_FAILURE;
                    }
                }
            }
        }
    }

    m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].bindingTableIndex = (uint16_t)BTIndex;
    if (SetSurfBTINumForIndirectData(format, pSurface_RT->Type())== 0)
    {
        CM_ASSERTMESSAGE("Error: Set surface binding table index count failure.");
        return CM_FAILURE;
    }
    m_IndirectSurfaceInfoArray[IndirectSurfInfoEntry].numBTIPerSurf = (uint16_t)SetSurfBTINumForIndirectData(format, pSurface_RT->Type());

    //Copy it to surface index array
    if (m_pKernelPayloadSurfaceArray[IndirectSurfInfoEntry] == nullptr)
    {
        m_pKernelPayloadSurfaceArray[IndirectSurfInfoEntry] = pSurface;
    }

    // count is actally one larger than the actual index
    m_usKernelPayloadSurfaceCount = IndirectSurfInfoEntry + 1;
    m_Dirty |= (CM_KERNEL_DATA_PAYLOAD_DATA_DIRTY | CM_KERNEL_DATA_PAYLOAD_DATA_SIZE_DIRTY);
    return CM_SUCCESS;
}

uint32_t CmKernelRT::GetKernelIndex()
{
    return m_kernelIndex;
}
uint32_t CmKernelRT::GetKernelGenxBinarySize(void)
{
    if(m_pKernelInfo == nullptr)
    {
        CM_ASSERTMESSAGE("Error: Invalid kernel genx binary size.");
        return 0;
    }
    else
    {
        return m_pKernelInfo->genxBinarySize;
    }
}

//-----------------------------------------------------------------------------------------------------------------
//! Map Surface type to Kernel Arg Kind.
//! INPUT:  Surface type    :CM_ENUM_CLASS_TYPE
//! OUTPUT: Kernel Arg Kind :CM_ARG_KIND
//-----------------------------------------------------------------------------------------------------------------
CM_ARG_KIND CmKernelRT::SurfTypeToArgKind(CM_ENUM_CLASS_TYPE SurfType)
{
    switch(SurfType)
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
    CmSurface*          pSurf = nullptr;
    CmSurface2DRT*        pSurf2D = nullptr;
    CmSurface2DUPRT*      pSurf2D_UP = nullptr;
    uint32_t              width, height, bytesPerPixel;
    CM_SURFACE_FORMAT     format;
    uint32_t              uiMaxBTIndex = 0;

    kernelSurfaceNum = 0;
    neededBTEntryNum = 0;

    surfaceArraySize = m_pSurfaceMgr->GetSurfacePoolSize();

    //Calculate surface number and needed binding table entries
    for (uint32_t surfIndex = 0; surfIndex <= m_MaxSurfaceIndexAllocated; surfIndex ++)
    {
        if (m_SurfaceArray[surfIndex])
        {
            pSurf = nullptr;
            m_pSurfaceMgr->GetSurface(surfIndex, pSurf);
            if (pSurf)
            {
                switch(pSurf->Type())
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
                        pSurf2D = static_cast<CmSurface2DRT*>(pSurf);
                        format = CM_SURFACE_FORMAT_INVALID;
                        pSurf2D->GetSurfaceDesc(width, height, format, bytesPerPixel);
                        if ((format == CM_SURFACE_FORMAT_NV12) ||
                            (format == CM_SURFACE_FORMAT_P010) ||
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
                        pSurf2D_UP = static_cast<CmSurface2DUPRT*>(pSurf);
                        format = CM_SURFACE_FORMAT_INVALID;
                        pSurf2D_UP->GetSurfaceDesc(width, height, format, bytesPerPixel);
                        if ((format == CM_SURFACE_FORMAT_NV12) ||
                            (format == CM_SURFACE_FORMAT_P010) ||
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

    if ((uiMaxBTIndex + 1) > neededBTEntryNum)
    {
        neededBTEntryNum = uiMaxBTIndex + 1;
    }

    //Wordaround: the calculation maybe not accurate if the VME surfaces are existed
    neededBTEntryNum += m_VMESurfaceCount;

    return CM_SUCCESS;
}


//*-----------------------------------------------------------------------------
//| Purpose:    Get aligned curbe size for different platforms
//| Returns:    Result of operation.
//*-----------------------------------------------------------------------------
uint32_t CmKernelRT::GetAlignedCurbeSize(uint32_t value)
{
    uint32_t CurbeAlignedSize    = 0;

    CurbeAlignedSize = MOS_ALIGN_CEIL(value, RENDERHAL_CURBE_BLOCK_ALIGN);
    return CurbeAlignedSize;
}

#if CM_LOG_ON
std::string CmKernelRT::Log()
{

    std::ostringstream  oss;

    oss << " Kernel Name:"         << m_pKernelInfo->kernelName << std::endl
        << " Kernel Binary Size:"  << m_pKernelInfo->jitBinarySize
        << " Index In Task:"       << m_IndexInTask
        << " Thread Count:"        << m_ThreadCount
        << " Curbe Size:"          << m_SizeInCurbe
        << " Kernel Arg Count:"    << m_ArgCount
        << std::endl;

     // Per Kernel Thread Space Log
    if(m_pThreadSpace)
    {
        oss << m_pThreadSpace->Log();
    }

    // Per Kernel Thread Group Space Log
    if(m_pThreadGroupSpace)
    {
        oss << m_pThreadGroupSpace->Log();
    }

    // Arguments Log
    for (uint32_t ArgIndex= 0; ArgIndex< m_ArgCount; ArgIndex++ )
    {
        if (m_Args[ArgIndex].pValue) // filter out the implicit arguments
        {
            ArgLog(oss, ArgIndex, m_Args[ArgIndex]);
        }
    }

    return oss.str();
}

void CmKernelRT::ArgLog(std::ostringstream &oss, uint32_t index, CM_ARG Arg)
{

    oss << "[" << index << "] th Argument"
        << " Type :" << Arg.unitKind
        << " Count:" << Arg.unitCount
        << " Size:" << Arg.unitSize
        << " Surface Kind:" << (int)Arg.s_k
        << " OffsetInPayload:" << Arg.unitOffsetInPayload
        << " OffsetInPayloadOrig:" << Arg.unitOffsetInPayloadOrig << "";

    CmLogger::LogDataArrayHex( oss, Arg.pValue, Arg.unitSize * Arg.unitCount);

    if (CHECK_SURFACE_TYPE(Arg.unitKind,
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
        uint16_t numSurfaces = Arg.unitSize / sizeof(uint32_t);
        if (Arg.unitKind == ARG_KIND_SURFACE_VME)
        {
            numSurfaces = (Arg.unitSize - sizeof(CM_HAL_VME_ARG_VALUE) * Arg.unitVmeArraySize) / sizeof(uint32_t) + Arg.unitVmeArraySize;
        }
        for (uint16_t i = 0; i < numSurfaces; i++)
        {
            uint32_t SurfaceIndex = *(uint16_t *)(Arg.surfIndex + i);

            if(SurfaceIndex == CM_NULL_SURFACE)
                continue;

            CmSurface *pSurf = nullptr;
            m_pSurfaceMgr->GetSurface(SurfaceIndex, pSurf);
            if (pSurf == nullptr)
            {
                continue;
            }
            pSurf->Log(oss);
        }
    }
}
#endif


void CmKernelRT::SurfaceDump(uint32_t kernelNumber, int32_t taskId)
{
#if MDF_SURFACE_CONTENT_DUMP
    CM_ARG arg;

    for (uint32_t argIndex = 0; argIndex< m_ArgCount; argIndex++)
    {
        arg = m_Args[argIndex];
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
                uint32_t SurfaceIndex = *(uint16_t *)(arg.surfIndex + i);
                CmSurface *pSurf = nullptr;
                m_pSurfaceMgr->GetSurface(SurfaceIndex, pSurf);
                if (pSurf == nullptr)
                {
                    return;
                }
                pSurf->DumpContent(kernelNumber, taskId, argIndex);
            }
        }
    }
#endif
}

CM_RT_API int32_t CmKernelRT::SetSamplerBTI(SamplerIndex* pSampler, uint32_t nIndex)
{
    if (!pSampler)
    {
        return CM_NULL_POINTER;
    }
    if (CM_SAMPLER_MAX_BINDING_INDEX < nIndex)
    {
        return CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX;
    }

    uint32_t        samplerIndex   = pSampler->get_data();
    PCM_HAL_STATE   pCmHalState    = ((PCM_CONTEXT_DATA)m_pCmDev->GetAccelData())->cmHalState;

    uint32_t i = 0;
    for (i = 0; i < m_SamplerBTICount; i++)
    {
        if ((m_SamplerBTIEntry[i].samplerIndex == samplerIndex) && (m_SamplerBTIEntry[i].samplerBTI == nIndex))
        {
            break;
        }
        if (m_Dirty & CM_KERNEL_DATA_SAMPLER_BTI_DIRTY)
        {
            if ((m_SamplerBTIEntry[i].samplerIndex != samplerIndex) && (m_SamplerBTIEntry[i].samplerBTI == nIndex))
            {
                if (pCmHalState->use_new_sampler_heap)
                {
                    SamplerParam sampler1 = {};
                    SamplerParam sampler2 = {};
                    pCmHalState->pCmHalInterface->GetSamplerParamInfoForSamplerType(&pCmHalState->pSamplerTable[m_SamplerBTIEntry[i].samplerIndex], sampler1);
                    pCmHalState->pCmHalInterface->GetSamplerParamInfoForSamplerType(&pCmHalState->pSamplerTable[samplerIndex], sampler2);

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
            CmSampler8x8State_RT *tmp_sampler8x8 = nullptr;
            m_pCmDev->GetSampler8x8(samplerIndex, sampler8x8);
            m_pCmDev->GetSampler8x8(m_SamplerBTIEntry[i].samplerIndex, tmp_sampler8x8);

            if (sampler8x8 && tmp_sampler8x8 && (sampler8x8->GetStateType() == CM_SAMPLER8X8_AVS)
                && (tmp_sampler8x8->GetStateType() == CM_SAMPLER8X8_AVS) &&
                pCmHalState->pCmHalInterface->IsAdjacentSamplerIndexRequiredbyHw())
            {
                if ((m_SamplerBTIEntry[i].samplerIndex != samplerIndex) &&
                    ((m_SamplerBTIEntry[i].samplerBTI == nIndex + 1) || (m_SamplerBTIEntry[i].samplerBTI == nIndex - 1)))
                    return CM_FAILURE;
            }
        }
    }

    if (i >= CM_MAX_SAMPLER_TABLE_SIZE)
    {
        CM_ASSERTMESSAGE("Error: Exceed maximum sampler table size.");
        return CM_FAILURE;
    }

    if (i == m_SamplerBTICount)
    {
        m_SamplerBTIEntry[i].samplerIndex = samplerIndex;
        m_SamplerBTIEntry[i].samplerBTI = nIndex;

        m_SamplerBTICount = i + 1;

        m_Dirty |= CM_KERNEL_DATA_SAMPLER_BTI_DIRTY;
    }
    return CM_SUCCESS;
}

CMRT_UMD_API int32_t CmKernelRT::GetBinary(std::vector<char>& binary)
{
    binary.resize(m_uiBinarySize);

    CmSafeMemCopy((void *)&binary[0], (void *)m_pBinary, m_uiBinarySize);

    return CM_SUCCESS;
}

CMRT_UMD_API int32_t CmKernelRT::ReplaceBinary(std::vector<char>& binary)
{
    uint32_t size = binary.size();

    if (size == 0)
    {
        return CM_INVALID_ARG_VALUE;
    }

	if(m_pBinaryOrig == nullptr)
	{
        //Store the orignal binary once.
        m_pBinaryOrig = m_pBinary;
        m_uiBinarySizeOrig = m_uiBinarySize;
	}

    m_pBinary = MOS_NewArray(char, size);
    CmSafeMemCopy((void *)m_pBinary, (void *)&binary[0], size);

    m_uiBinarySize = size;

    return CM_SUCCESS;
}

CMRT_UMD_API int32_t CmKernelRT::ResetBinary()
{
    if (m_pBinaryOrig == nullptr)
    {
        //ReplaceBinary is never called
        return CM_SUCCESS;
    }
	if(m_pBinary!= m_pBinaryOrig)
	{
        MosSafeDeleteArray(m_pBinary);
	}
    m_pBinary = m_pBinaryOrig;
    m_uiBinarySize = m_uiBinarySizeOrig;

    return CM_SUCCESS;
}

int CmKernelRT::UpdateSamplerHeap(CmKernelData *pCmKernelData)
{
    // Get sampler bti & offset
    PCM_HAL_KERNEL_PARAM pCmKernel = nullptr;
    PCM_CONTEXT_DATA pCmData = (PCM_CONTEXT_DATA)m_pCmDev->GetAccelData();
    PCM_HAL_STATE pState = pCmData->cmHalState;
    std::list<SamplerParam>::iterator iter;
    unsigned int heap_offset = 0;

    if (pState->use_new_sampler_heap == false)
    {
        return CM_SUCCESS;
    }

    heap_offset = 0;
    pCmKernel = pCmKernelData->GetHalCmKernelData();
    std::list<SamplerParam> *sampler_heap = pCmKernel->samplerHeap;

    // First pass, inserts sampler with user-defined BTI to the list. Sorts by element order low to high, then by BTI order low to high.
    for (unsigned int sampler_element_type = MHW_Sampler1Element; sampler_element_type < MHW_SamplerTotalElements; sampler_element_type++)
    {
        for (unsigned int n = 0; n < pCmKernel->samplerBTIParam.samplerCount; ++n)
        {
            SamplerParam sampler = {};
            sampler.samplerTableIndex = pCmKernel->samplerBTIParam.samplerInfo[n].samplerIndex;

            if (pState->pSamplerTable[sampler.samplerTableIndex].ElementType == sampler_element_type)
            {
                sampler.bti = pCmKernel->samplerBTIParam.samplerInfo[n].samplerBTI;
                sampler.userDefinedBti = true;
                pState->pCmHalInterface->GetSamplerParamInfoForSamplerType(&pState->pSamplerTable[sampler.samplerTableIndex], sampler);

                // Guarantees each user-defined BTI has a spacing between each other user-defined BTIs larger than the stepping
                for (iter = sampler_heap->begin(); iter != sampler_heap->end(); iter++)
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
                for (iter = sampler_heap->begin(); iter != sampler_heap->end(); iter++)
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
    for (unsigned int sampler_element_type = MHW_Sampler1Element; sampler_element_type < MHW_SamplerTotalElements; sampler_element_type++)
    {
        for (unsigned int index = 0; index < pCmKernel->numArgs; index++)
        {
            PCM_HAL_KERNEL_ARG_PARAM arg_param = &pCmKernel->argParams[index];
            if (arg_param->isNull)
            {
                continue;
            }

            for (unsigned int thread_index = 0; thread_index < arg_param->unitCount; thread_index++)
            {
                if (arg_param->kind == CM_ARGUMENT_SAMPLER)
                {
                    unsigned char *arg = arg_param->firstValue + (thread_index * arg_param->unitSize);
                    unsigned int sampler_table_index = *((uint32_t *)arg);

                    SamplerParam sampler = {};
                    sampler.samplerTableIndex = sampler_table_index;
                    pState->pCmHalInterface->GetSamplerParamInfoForSamplerType(&pState->pSamplerTable[sampler.samplerTableIndex], sampler);
                    sampler.regularBti = true;

                    if (sampler.elementType != sampler_element_type)
                    {
                        continue;
                    }

                    // if the sampler is already in the heap, skip
                    bool is_duplicate = false;
                    for (iter = sampler_heap->begin(); iter != sampler_heap->end(); iter++)
                    {
                        if (iter->samplerTableIndex == sampler.samplerTableIndex)
                        {
                            is_duplicate = true;
                            iter->regularBti = true;
                            break;
                        }
                    }
                    if (is_duplicate == true)
                    {
                        continue;
                    }

                    // insert the new sampler to the heap
                    heap_offset = 0;
                    for (iter = sampler_heap->begin(); iter != sampler_heap->end(); iter++)
                    {
                        if (iter->elementType == sampler.elementType)
                        {
                            // Needs to keep the inserted sampler's correctness, so do not insert before same element regular sampler
                            // Only insert before user-defined BTI
                            if (iter->userDefinedBti == true)
                            {
                                unsigned int cur_offset = iter->heapOffset;
                                if (heap_offset > cur_offset)
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
                                    if (cur_offset - heap_offset >= sampler.btiStepping * sampler.btiMultiplier)
                                    {
                                        break;
                                    }
                                    else
                                    {
                                        heap_offset = cur_offset + iter->btiStepping * iter->btiMultiplier;
                                    }
                                }
                            }
                            else
                            {
                                heap_offset += iter->btiStepping * iter->btiMultiplier;
                            }
                        }
                        else if (iter->elementType > sampler.elementType)
                        {
                            break;
                        }
                        else
                        {
                            heap_offset = iter->heapOffset + iter->size;
                            std::list<SamplerParam>::iterator iter_next = std::next(iter, 1);
                            if ((iter_next != sampler_heap->end()) && (iter_next->elementType > iter->elementType))
                            {
                                // Aligns heap_offset to next nearest multiple of sampler size if next sampler is a different element type
                                heap_offset = (heap_offset + iter_next->btiStepping * iter_next->btiMultiplier - 1) / (iter_next->btiStepping * iter_next->btiMultiplier) * (iter_next->btiStepping * iter_next->btiMultiplier);
                            }
                        }
                    }

                    if (iter == sampler_heap->end())
                    {
                        // Aligns heap_offset to next nearest multiple of sampler size if next sampler is a different element type
                        heap_offset = (heap_offset + sampler.btiStepping * sampler.btiMultiplier - 1) / (sampler.btiStepping * sampler.btiMultiplier) * (sampler.btiStepping * sampler.btiMultiplier);
                    }
                    sampler.heapOffset = heap_offset;
                    sampler.bti = sampler.heapOffset / sampler.btiMultiplier;
                    sampler_heap->insert(iter, sampler);
                }
            }
        }
    }

    return CM_SUCCESS;
}
}