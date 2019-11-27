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
//! \file      cm_program.cpp 
//! \brief     Contains Class CmProgram definitions 
//!

#include "cm_program.h"

#include "cm_device_rt.h"
#include "cm_mem.h"
#include "cm_hal.h"

#if USE_EXTENSION_CODE
#include "cm_hw_debugger.h"
#endif

#include <string>
#include <functional>

#define READ_FIELD_FROM_BUF( dst, type ) \
    dst = *((type *) &buf[bytePos]); \
    bytePos += sizeof(type);

#define CM_RT_JITTER_DEBUG_FLAG "-debug"
#define CM_RT_JITTER_NCSTATELESS_FLAG "-ncstateless"

#define CM_RT_JITTER_MAX_NUM_FLAGS      30
#define CM_RT_JITTER_NUM_RESERVED_FLAGS 3  // one for gtpin;  two for hw stepping info.
#define CM_RT_JITTER_MAX_NUM_USER_FLAGS (CM_RT_JITTER_MAX_NUM_FLAGS - CM_RT_JITTER_NUM_RESERVED_FLAGS)

namespace CMRT_UMD
{
//*-----------------------------------------------------------------------------
//| Purpose:    Create Cm Program
//| Arguments :
//|               device            [in]     Pointer to Cm Device
//|               commonISACode    [in]     Pointer to memory where common isa locates
//|               size              [in]     Size of memory
//|               pProgram          [in]     Reference to pointer to CmProgram
//|               options           [in]     jitter or non-jitter
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::Create( CmDeviceRT* device, void* cisaCode, const uint32_t cisaCodeSize, CmProgramRT*& pProgram,  const char* options, const uint32_t programId )
{
    int32_t result = CM_SUCCESS;
    pProgram = new (std::nothrow) CmProgramRT( device, programId );
    if( pProgram )
    {
        pProgram->Acquire();
        result = pProgram->Initialize( cisaCode, cisaCodeSize, options );
        if( result != CM_SUCCESS )
        {
            CmProgramRT::Destroy( pProgram);
        }
    }
    else
    {
        CM_ASSERTMESSAGE("Error: Failed to create CmProgram due to out of system memory.");
        result = CM_OUT_OF_HOST_MEMORY;
    }
    return result;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destroy Cm Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::Destroy( CmProgramRT* &program )
{
    long refCount = program->SafeRelease(  );
    if( refCount == 0 )
    {
        program = nullptr;
    }
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Acquire: Increase refcount
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::Acquire( void  )
{
    m_refCount++;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    SafeRelease:
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::SafeRelease( void )
{
    --m_refCount;
    if( m_refCount == 0 )
    {
        delete this;
        return 0;
    }
    return m_refCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Constructor of Cm Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmProgramRT::CmProgramRT( CmDeviceRT* device, uint32_t programId ):
    m_device( device ),
    m_programCodeSize( 0 ),
    m_programCode(nullptr),
    m_isaFile(nullptr),
    m_options( nullptr ),
    m_surfaceCount( 0 ),
    m_kernelCount( 0 ),
    m_kernelInfo( CM_INIT_KERNEL_PER_PROGRAM ),
    m_isJitterEnabled(false),
    m_isHwDebugEnabled(false),
    m_refCount(0),
    m_programIndex(programId),
    m_fJITCompile(nullptr),
    m_fFreeBlock(nullptr),
    m_fJITVersion(nullptr),
    m_fJITCompile_v2(nullptr),
    m_cisaMagicNumber(0),
    m_cisaMajorVersion(0),
    m_cisaMinorVersion(0)
{
    CmSafeMemSet(m_isaFileName,0,sizeof(char)*CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Cm Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmProgramRT::~CmProgramRT( void )
{
    MosSafeDeleteArray( m_options );
    MosSafeDeleteArray( m_programCode );
    for( uint32_t i = 0; i < m_kernelCount; i ++ )
    {
        uint32_t refCount = this->ReleaseKernelInfo(i);
        CM_ASSERT(refCount == 0);

    }
    m_kernelInfo.Delete();
    CmSafeDelete(m_isaFile);
}

#if (_RELEASE_INTERNAL)
int32_t CmProgramRT::ReadUserFeatureValue(const char *pcMessageKey, uint32_t &value)
{
    MOS_USER_FEATURE        userFeature;
    MOS_USER_FEATURE_VALUE  userFeatureValue = __NULL_USER_FEATURE_VALUE__;
    CM_RETURN_CODE          hr = CM_SUCCESS;

    // Set the component's message level and asserts:
    userFeatureValue.u32Data    = __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE;
    userFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath           = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues         = &userFeatureValue;
    userFeature.uiNumValues     = 1;

    CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(MOS_UserFeature_ReadValue(
                                  nullptr,
                                  &userFeature,
                                  pcMessageKey,
                                  MOS_USER_FEATURE_VALUE_TYPE_UINT32));

    value = userFeature.pValues->u32Data;

finish:
    return hr;
}
#endif

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Cm Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::Initialize( void* cisaCode, const uint32_t cisaCodeSize, const char* options )
{
    bool loadingGPUCopyKernel = false;
    int32_t result = CM_SUCCESS;
    int32_t hr     = CM_FAILURE;

    m_isJitterEnabled = true; //by default jitter is ON

    int numJitFlags = 0;
    const char *jitFlags[CM_RT_JITTER_MAX_NUM_FLAGS];
    CmSafeMemSet(jitFlags, 0, sizeof(char *) * CM_RT_JITTER_MAX_NUM_FLAGS);

    char* flagStepInfo = nullptr;

    if( options )
    {
        size_t length = strnlen( options, CM_MAX_OPTION_SIZE_IN_BYTE );
        if(length >= CM_MAX_OPTION_SIZE_IN_BYTE)
        {
            CM_ASSERTMESSAGE("Error: option size is too long.");
            return CM_INVALID_ARG_VALUE;
        }
        else
        {
            m_options = MOS_NewArray(char, (length + 1));
            if( !m_options )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;

            }
            CmSafeMemCopy( m_options, options, length);
            m_options[ length ] = '\0';

            if(strstr(options, "nojitter"))
                m_isJitterEnabled = false;

            if(!strcmp(m_options, "PredefinedGPUKernel"))
                loadingGPUCopyKernel = true;
            if( (m_isJitterEnabled == true) && (loadingGPUCopyKernel == false))
            { // option: "nonjitter" and "PredefinedGPUCopyKernel" should not be passed to jitter
                char *token = nullptr;
                char *nextToken = nullptr;
                char *ptr =  m_options;

                while( nullptr != (token = strtok_s(ptr," ",&nextToken)))
                {
                    if(numJitFlags >= CM_RT_JITTER_MAX_NUM_USER_FLAGS)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid jitter user flags number.");
                        MosSafeDeleteArray(m_options);
                        return CM_FAILURE;
                    }
                    if(!strcmp(token, CM_RT_JITTER_DEBUG_FLAG))
                    {
                        m_isHwDebugEnabled = true;
                    }
                    jitFlags[numJitFlags] = token;
                    numJitFlags++;
                    ptr = nextToken;
                }
            }
        }
    }

    uint8_t *buf = (uint8_t*)cisaCode;
    uint32_t bytePos = 0;

    READ_FIELD_FROM_BUF(m_cisaMagicNumber, uint32_t);
    READ_FIELD_FROM_BUF(m_cisaMajorVersion, uint8_t);
    READ_FIELD_FROM_BUF(m_cisaMinorVersion, uint8_t);

    bool useVisaApi = true;
    vISA::Header *header = nullptr;

    auto getVersionAsInt = [](int major, int minor) {return major * 100 + minor;};
    if (getVersionAsInt(m_cisaMajorVersion, m_cisaMinorVersion) < getVersionAsInt(3, 2))
    {
        useVisaApi = false;
    }
    else
    {
        m_isaFile = new vISA::ISAfile((uint8_t*)cisaCode, cisaCodeSize);
        if (!m_isaFile->readFile())
        {
            CM_ASSERTMESSAGE("Error: invalid VISA.");
            MosSafeDeleteArray(m_options);
            return CM_INVALID_COMMON_ISA;
        }
        header = m_isaFile->getHeader();
    }

    if (m_cisaMagicNumber != CISA_MAGIC_NUMBER)
    {
        CM_ASSERTMESSAGE("Error: Invalid CISA magic number.");
        MosSafeDeleteArray(m_options);
        return CM_INVALID_COMMON_ISA;
    }

    if(loadingGPUCopyKernel)//for predefined kernel(GPUCopy), forcely disable jitting
    {
        m_isJitterEnabled = false;
    }

    const char *platform = nullptr;

    PCM_HAL_STATE  cmHalState = \
        ((PCM_CONTEXT_DATA)m_device->GetAccelData())->cmHalState;
    CM_CHK_NULL_GOTOFINISH_CMERROR(cmHalState);

    cmHalState->cmHalInterface->GetGenPlatformInfo(nullptr, nullptr, &platform);

    if( m_isJitterEnabled )
    {
    //reg control for svm IA/GT cache coherence
#if (_RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_MDF_FORCE_COHERENT_STATELESSBTI_ID,
            &userFeatureData);
        if (userFeatureData.i32Data == 1)
        {
            jitFlags[numJitFlags] = CM_RT_JITTER_NCSTATELESS_FLAG;
            numJitFlags++;
        }

#endif // (_RELEASE_INTERNAL)

        //Load jitter library and add function pointers to program
        // Get hmodule from CmDevice_RT or CmDevice_Sim, which is casted from CmDevice
        result = m_device->LoadJITDll();
        if(result != CM_SUCCESS)
        {
            CM_ASSERTMESSAGE("Error: Load jitter library failure.");
            MosSafeDeleteArray(m_options);
            return result;
        }

        m_device->GetJITCompileFnt(m_fJITCompile);
        m_device->GetJITCompileFntV2(m_fJITCompile_v2);
        m_device->GetFreeBlockFnt(m_fFreeBlock);
        m_device->GetJITVersionFnt(m_fJITVersion);

        uint32_t jitMajor = 0;
        uint32_t jitMinor = 0;
        m_fJITVersion(jitMajor, jitMinor);
        if((jitMajor < m_cisaMajorVersion) || (jitMajor == m_cisaMajorVersion && jitMinor < m_cisaMinorVersion))
            return CM_JITDLL_OLDER_THAN_ISA;
#if USE_EXTENSION_CODE
        if( m_device->CheckGTPinEnabled() && !loadingGPUCopyKernel)
        {
            hr = InitForGTPin(jitFlags, numJitFlags);
            if (hr != CM_SUCCESS)
            {
                goto finish;
            }
        }
#endif
        //Pass stepping info to JITTER..."
        char *stepstr = nullptr;
        m_device->GetGenStepInfo(stepstr);
        if(stepstr != nullptr)
        {
            flagStepInfo = MOS_NewArray(char, CM_JIT_FLAG_SIZE);
            if (flagStepInfo)
            {
                jitFlags[numJitFlags] = "-stepping";
                numJitFlags++;

                CmSafeMemSet(flagStepInfo, 0, CM_JIT_FLAG_SIZE);
                MOS_SecureStringPrint(flagStepInfo, CM_JIT_FLAG_SIZE, CM_JIT_FLAG_SIZE, "%s", stepstr);
                if (numJitFlags >= CM_RT_JITTER_MAX_NUM_FLAGS)
                {
                    CM_ASSERTMESSAGE("Error: Invalid jitter user flags number.");
                    hr = CM_FAILURE;
                    goto finish;
                }
                jitFlags[numJitFlags] = flagStepInfo;
                numJitFlags++;
            }
            else
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                MosSafeDeleteArray(m_options);
                return CM_OUT_OF_HOST_MEMORY;
            }
        }
    }

    if (useVisaApi)
    {
        m_kernelCount = header->getNumKernels();
    }
    else
    {
        bytePos = 4 + 1 + 1; // m_cisaMagicNumber:sizeof (uint32_t) + m_cisaMajorVersion:sizeof(uint8_t) + m_cisaMinorVersion:sizeof(uint8_t)
        unsigned short numKernels;
        READ_FIELD_FROM_BUF(numKernels, unsigned short);
        m_kernelCount = numKernels;
    }

#ifdef _DEBUG
    if(m_isJitterEnabled)
        CM_NORMALMESSAGE("Jitter Compiling...");
#endif

    for (uint32_t i = 0; i < m_kernelCount; i++)
    {
        CM_KERNEL_INFO* kernInfo = new (std::nothrow) CM_KERNEL_INFO;
        if (!kernInfo)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(flagStepInfo);
            MosSafeDeleteArray(m_options);
            hr = CM_OUT_OF_HOST_MEMORY;
            goto finish;
        }
        CmSafeMemSet(kernInfo, 0, sizeof(CM_KERNEL_INFO));

        vISA::Kernel *kernel = nullptr;
        uint8_t nameLen = 0;
        if (useVisaApi)
        {
            kernel = header->getKernelInfo()[i];
            nameLen = kernel->getNameLen();
            CmSafeMemCopy(kernInfo->kernelName, kernel->getName(), nameLen);
        }
        else
        {
            READ_FIELD_FROM_BUF(nameLen, uint8_t);
            CmSafeMemCopy(kernInfo->kernelName, buf + bytePos, nameLen);
            // move bytePos to the right index
            bytePos += nameLen;
        }

        if(m_isJitterEnabled)
        {
            if (useVisaApi)
            {
                kernInfo->kernelIsaOffset = kernel->getOffset();
                kernInfo->kernelIsaSize = kernel->getSize();
                kernInfo->inputCountOffset = kernel->getInputOffset();
            }
            else
            {
                uint32_t kernelIsaOffset;
                uint32_t kernelIsaSize;
                uint32_t inputCountOffset;

                READ_FIELD_FROM_BUF(kernelIsaOffset, uint32_t); //read kernel isa offset
                READ_FIELD_FROM_BUF(kernelIsaSize, uint32_t); //read kernel isa size
                READ_FIELD_FROM_BUF(inputCountOffset, uint32_t);

                kernInfo->kernelIsaOffset = kernelIsaOffset;
                kernInfo->kernelIsaSize = kernelIsaSize;
                kernInfo->inputCountOffset = inputCountOffset;

                // read relocation symbols
                unsigned short numVarSyms = 0, num_func_syms = 0;
                unsigned char numGenBinaries = 0;

                READ_FIELD_FROM_BUF(numVarSyms, uint16_t);
                // CISA layout of var_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                bytePos += sizeof(uint16_t) * numVarSyms;
                bytePos += sizeof(uint16_t) * numVarSyms;

                READ_FIELD_FROM_BUF(num_func_syms, uint16_t);
                // CISA layout of func_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                bytePos += sizeof(uint16_t) * num_func_syms;
                bytePos += sizeof(uint16_t) * num_func_syms;

                // numGenBinaries
                READ_FIELD_FROM_BUF(numGenBinaries, uint8_t);
                // CISA layout of gen_binaries is genPlatform[0], binary offset[0],
                // binary size[0]...binary size[n-1]. Skip all.
                // skip all genPlatform
                bytePos += sizeof(uint8_t) * numGenBinaries;
                // skip all binary offset
                bytePos += sizeof(uint32_t) * numGenBinaries;
                // skip all binary size
                bytePos += sizeof(uint32_t) * numGenBinaries;

                // bytePos should point to nameLen of next kernel
            }
        }
        else // non jitting
        {
            uint8_t numGenBinaries = 0;
            if (useVisaApi)
            {
                kernInfo->kernelIsaOffset = kernel->getOffset();
                kernInfo->inputCountOffset = kernel->getInputOffset();
                numGenBinaries = kernel->getNumGenBinaries();
            }
            else
            {
                uint32_t kernelIsaOffset;
                READ_FIELD_FROM_BUF(kernelIsaOffset, uint32_t); //read kernel isa offset
                kernInfo->kernelIsaOffset = kernelIsaOffset;

                // skipping kernelIsaSize
                bytePos += 4;

                uint32_t inputCountOffset;
                READ_FIELD_FROM_BUF(inputCountOffset, uint32_t);
                kernInfo->inputCountOffset = inputCountOffset;

                // read relocation symbols
                unsigned short numVarSyms = 0, num_func_syms = 0;

                READ_FIELD_FROM_BUF(numVarSyms, uint16_t);
                // CISA layout of var_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                bytePos += sizeof(uint16_t) * numVarSyms;
                bytePos += sizeof(uint16_t) * numVarSyms;

                READ_FIELD_FROM_BUF(num_func_syms, uint16_t);
                // CISA layout of func_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                bytePos += sizeof(uint16_t) * num_func_syms;
                bytePos += sizeof(uint16_t) * num_func_syms;

                // numGenBinaries
                READ_FIELD_FROM_BUF(numGenBinaries, uint8_t);
            }

            uint32_t genxBinaryOffset = 0;
            uint32_t genxBinarySize = 0;
            for (int j = 0; j < numGenBinaries; j++)
            {
                vISA::GenBinary *genBinary = nullptr;
                uint8_t genPlatform = 0;
                uint32_t offset = 0;
                uint32_t size = 0;

                if (useVisaApi)
                {
                    genBinary = kernel->getGenBinaryInfo()[j];
                    genPlatform = genBinary->getGenPlatform();
                    offset = genBinary->getBinaryOffset();
                    size = genBinary->getBinarySize();
                }
                else
                {
                    // genPlatform
                    READ_FIELD_FROM_BUF(genPlatform, uint8_t);
                    // binary offset
                    READ_FIELD_FROM_BUF(offset, uint32_t);
                    // binary size
                    READ_FIELD_FROM_BUF(size, uint32_t);
                }

                if (cmHalState->cmHalInterface->IsCisaIDSupported((uint32_t)genPlatform))
                {
                    // assign correct offset/size based on platform
                    genxBinaryOffset = offset;
                    genxBinarySize = size;
                }
                else
                {
                    MosSafeDeleteArray(flagStepInfo);
                    MosSafeDeleteArray(m_options);
                    CmSafeDelete(kernInfo);
                    return CM_INVALID_GENX_BINARY;
                }
            }

            kernInfo->genxBinaryOffset = genxBinaryOffset;
            kernInfo->genxBinarySize = genxBinarySize;

            if ( kernInfo->genxBinarySize == 0 || kernInfo->genxBinaryOffset == 0 )
            {
                CM_ASSERTMESSAGE("Error: Invalid genx binary.");
                MosSafeDeleteArray(flagStepInfo);
                MosSafeDeleteArray(m_options);
                CmSafeDelete(kernInfo);
                return CM_INVALID_GENX_BINARY;
            }
        }

        if(m_isJitterEnabled)
        {
            kernInfo->jitBinaryCode = 0;
            void* jitBinary = 0;// = (void**)malloc(m_kernelCount*sizeof(void*));
            uint32_t jitBinarySize = 0;// = (uint32_t*)malloc(m_kernelCount*sizeof(uint32_t*));
            char* errorMsg = (char*)malloc(CM_JIT_ERROR_MESSAGE_SIZE);
            if (errorMsg == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                CmSafeDelete(kernInfo);
                hr = CM_OUT_OF_HOST_MEMORY;
                goto finish;
            }
            CmSafeMemSet( errorMsg, 0, CM_JIT_ERROR_MESSAGE_SIZE );

            FINALIZER_INFO *jitProfInfo = (FINALIZER_INFO *)malloc(CM_JIT_PROF_INFO_SIZE);
            if(jitProfInfo == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                free(errorMsg);
                CmSafeDelete(kernInfo);
                hr = CM_OUT_OF_HOST_MEMORY;
                goto finish;
            }
            CmSafeMemSet( jitProfInfo, 0, CM_JIT_PROF_INFO_SIZE );

            void *extra_info = nullptr;
            CmNotifierGroup *notifiers = m_device->GetNotifiers();
            if (notifiers)
            {
                notifiers->NotifyCallingJitter(&extra_info);
            }

            if (m_fJITCompile_v2)
            {
                result = m_fJITCompile_v2( kernInfo->kernelName, (uint8_t*)cisaCode, cisaCodeSize,
                                    jitBinary, jitBinarySize, platform, m_cisaMajorVersion, m_cisaMinorVersion, numJitFlags, jitFlags, errorMsg, jitProfInfo, extra_info );
            }
            else
            {
                result = m_fJITCompile( kernInfo->kernelName, (uint8_t*)cisaCode, cisaCodeSize,
                                    jitBinary, jitBinarySize, platform, m_cisaMajorVersion, m_cisaMinorVersion, numJitFlags, jitFlags, errorMsg, jitProfInfo );
            }

            //if error code returned or error message not nullptr
            if(result != CM_SUCCESS)// || errorMsg[0])
            {
                CM_NORMALMESSAGE("%s.", errorMsg);
                free(errorMsg);
                CmSafeDelete(kernInfo);
                hr = CM_JIT_COMPILE_FAILURE;
                goto finish;
            }

            // if spill code exists and scrach space disabled, return error to user
            if( jitProfInfo->isSpill &&  m_device->IsScratchSpaceDisabled())
            {
                CmSafeDelete(kernInfo);
                free(errorMsg);
                return CM_INVALID_KERNEL_SPILL_CODE;
            }

            free(errorMsg);

            kernInfo->jitBinaryCode = jitBinary;
            kernInfo->jitBinarySize = jitBinarySize;
            kernInfo->jitInfo = jitProfInfo;

#if USE_EXTENSION_CODE
            if ( m_isHwDebugEnabled )
            {
                NotifyKernelBinary(this->m_device,
                                   this,
                                   kernInfo->kernelName,
                                   jitBinary,
                                   jitBinarySize,
                                   jitProfInfo->genDebugInfo,
                                   jitProfInfo->genDebugInfoSize,
                                   nullptr,
                                   m_device->GetDriverStoreFlag());
            }
#endif
        }

        m_kernelInfo.SetElement( i, kernInfo );
        this->AcquireKernelInfo(i);
    }

#ifdef _DEBUG
    if(m_isJitterEnabled)
        CM_NORMALMESSAGE("Jitter Done.");
#endif

    // now bytePos index to the start of common isa body;
    // compute the code size for common isa
    m_programCodeSize = cisaCodeSize;
    m_programCode = MOS_NewArray(uint8_t, m_programCodeSize);
    if( !m_programCode )
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        hr = CM_OUT_OF_HOST_MEMORY;
        goto finish;
    }

    //Copy CISA content
    CmFastMemCopy((void *)m_programCode, cisaCode, cisaCodeSize);

    //Caculate hash value for each kernel
    for (uint32_t i = 0; i < m_kernelCount; i++)
    {
        CM_KERNEL_INFO *kernelInfo = (CM_KERNEL_INFO *)m_kernelInfo.GetElement(i);
        CM_CHK_NULL_GOTOFINISH_CMERROR(kernelInfo);
        // higher 32bit is the order of kernel in LoadProgram in the device
        // lower 32bit is the hash value of kernel info
        kernelInfo->hashValue = GetKernelInfoHash(kernelInfo) | ((uint64_t)m_device->KernelsLoaded() << 32);
        ++ m_device->KernelsLoaded();
    }

    hr = CM_SUCCESS;

finish:
    MosSafeDeleteArray(flagStepInfo);
    if(hr != CM_SUCCESS )
    {
        MosSafeDeleteArray(m_options);
        MosSafeDeleteArray(m_programCode);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get size and address of Common Isa
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetCommonISACode( void* & commonISACode, uint32_t & size )
{
    commonISACode = (void *)m_programCode;
    size = m_programCodeSize;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the count of kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetKernelCount( uint32_t& kernelCount )
{
    kernelCount = m_kernelCount;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Kernel's Infomation
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetKernelInfo( uint32_t index, CM_KERNEL_INFO*& kernelInfo )
{
    if( index < m_kernelCount )
    {
        kernelInfo = (CM_KERNEL_INFO*)m_kernelInfo.GetElement( index ) ;
        return CM_SUCCESS;
    }
    else
    {
        kernelInfo = nullptr;
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the name of ISA file
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetIsaFileName( char* & isaFileName )
{
    isaFileName = m_isaFileName;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel's options
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetKernelOptions( char* & kernelOptions )
{
    kernelOptions = m_options;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the number of Surfaces
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmProgramRT::GetSurfaceCount(void)
{
    return m_surfaceCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Program's surface count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::SetSurfaceCount(uint32_t count)
{
    m_surfaceCount = count;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Acquire Kernel Info
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmProgramRT::AcquireKernelInfo(uint32_t index)
{
    CM_KERNEL_INFO* kernelInfo = nullptr;

    if( index < m_kernelCount )
    {
        kernelInfo = (CM_KERNEL_INFO *)m_kernelInfo.GetElement( index ) ;
        if (kernelInfo)
        {
            CM_ASSERT( (int32_t)kernelInfo->kernelInfoRefCount >= 0 );
            CM_ASSERT( kernelInfo->kernelInfoRefCount < UINT_MAX );

            ++ kernelInfo->kernelInfoRefCount;
            return kernelInfo->kernelInfoRefCount;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Release Kernel Info
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmProgramRT::ReleaseKernelInfo(uint32_t index)
{
    CM_KERNEL_INFO* kernelInfo = nullptr;

    if( index < m_kernelCount )
    {
        kernelInfo = (CM_KERNEL_INFO *)m_kernelInfo.GetElement( index ) ;
        if (kernelInfo)
        {
            CM_ASSERT( kernelInfo->kernelInfoRefCount > 0 );

            -- kernelInfo->kernelInfoRefCount;

            if (kernelInfo->kernelInfoRefCount == 1)
            {
                /////////////////////////////////////////////////////////////
                //Free global string memory space, Start
                for (int i = 0; i < (int) kernelInfo->globalStringCount; i ++)
                {
                    if (kernelInfo->globalStrings[i])
                    {
                        free((void *)kernelInfo->globalStrings[i]);
                    }
                }
                if (kernelInfo->globalStrings)
                {
                    free((void *)kernelInfo->globalStrings);
                    kernelInfo->globalStrings = nullptr;
                    kernelInfo->globalStringCount = 0;
                }
                //Free global string memory space, End
                /////////////////////////////////////////////////////////////

                for (uint32_t i = 0; i < kernelInfo->surfaceCount; i++) {
                    if (kernelInfo->surface[i].attributeCount && kernelInfo->surface[i].attributes) {
                        free(kernelInfo->surface[i].attributes);
                    }
                }
                if (kernelInfo->surface) {
                    free(kernelInfo->surface);
                    kernelInfo->surface = nullptr;
                    kernelInfo->surfaceCount = 0;
                }

                return 1;
            }

            else if (kernelInfo->kernelInfoRefCount == 0)
            {
                if(m_isJitterEnabled)
                {
                    if(kernelInfo->jitBinaryCode)
                        m_fFreeBlock(kernelInfo->jitBinaryCode);
                    if(kernelInfo->jitInfo)
                    {
                        if (kernelInfo->jitInfo->freeGRFInfo)
                        {
                            m_fFreeBlock(kernelInfo->jitInfo->freeGRFInfo);
                        }
                        free(kernelInfo->jitInfo);
                    }
                }

                /////////////////////////////////////////////////////////////
                //Free global string memory space, Start
                for (int i = 0; i < (int) kernelInfo->globalStringCount; i ++)
                {
                    if (kernelInfo->globalStrings[i])
                    {
                        free((void *)kernelInfo->globalStrings[i]);
                    }
                }
                if (kernelInfo->globalStrings)
                {
                    free((void *)kernelInfo->globalStrings);
                }
                //Free global string memory space, End
                /////////////////////////////////////////////////////////////

                for (uint32_t i = 0; i < kernelInfo->surfaceCount; i++) {
                    if (kernelInfo->surface[i].attributeCount && kernelInfo->surface[i].attributes) {
                        free(kernelInfo->surface[i].attributes);
                    }
                }
                if (kernelInfo->surface) {
                    free(kernelInfo->surface);
                }

                CmSafeDelete( kernelInfo );

                m_kernelInfo.SetElement(index, nullptr);

                return 0;
            }
            else
            {
                return kernelInfo->kernelInfoRefCount;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
}

int32_t CmProgramRT::GetKernelInfoRefCount(uint32_t index, uint32_t& refCount)
{
    CM_KERNEL_INFO* kernelInfo = nullptr;

    refCount = 0;

    if( index < m_kernelCount )
    {
        kernelInfo =(CM_KERNEL_INFO *) m_kernelInfo.GetElement( index ) ;
        if (kernelInfo)
        {
            refCount = kernelInfo->kernelInfoRefCount;
            return CM_SUCCESS;
        }
        else
        {
            return CM_FAILURE;
        }
    }
    else
    {
        return CM_FAILURE;
    }
}

int32_t CmProgramRT::GetCISAVersion(uint32_t& majorVersion, uint32_t& minorVersion)
{
    majorVersion = m_cisaMajorVersion;
    minorVersion = m_cisaMinorVersion;

    return CM_SUCCESS;
}

uint32_t CmProgramRT::GetProgramIndex()
{
    return m_programIndex;
}

vISA::ISAfile *CmProgramRT::getISAfile()
{
    return m_isaFile;
}

template <typename T>
inline void hashCombine(uint32_t &res, const T &field)
{
    std::hash<T> hasher;
    res ^= hasher(field) + 0x9e3779b9 + (res << 6) + (res >> 2);
}

inline void hashCombineString(uint32_t &res, char *str)
{
    uint32_t strHash = std::hash<std::string>{}(std::string(str));
    hashCombine(res, strHash);
}

uint32_t CmProgramRT::GetKernelInfoHash(CM_KERNEL_INFO *kernelInfo)
{
    uint32_t value = 0;
    hashCombineString(value, kernelInfo->kernelName);
    hashCombine(value, kernelInfo->inputCountOffset);
    hashCombine(value, kernelInfo->kernelIsaOffset);
    hashCombine(value, kernelInfo->kernelIsaSize);
    uint8_t *kernelBin = nullptr;
    uint32_t kernelSize = 0;
    if (m_isJitterEnabled)
    {
        kernelBin = (uint8_t *)kernelInfo->jitBinaryCode;
        kernelSize = kernelInfo->jitBinarySize;
    }
    else
    {
        kernelBin = m_programCode + kernelInfo->genxBinaryOffset;
        kernelSize = kernelInfo->genxBinarySize;
    }
    uint32_t *kernelBinDW = (uint32_t *)kernelBin;
    uint32_t kernelSizeDW = kernelSize / 4;
    double step = (double)kernelSizeDW/64.0;

    for (int i = 0; i < 256; i ++)
    {
        int index = (int)(kernelSizeDW - 1 - i*step);
        if (index < 0)
        {
            index = 0;
        }
        hashCombine(value, kernelBinDW[index]);
    }
    hashCombine(value, kernelSize);

    return value;
}

}
