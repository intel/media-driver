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

#define READ_FIELD_FROM_BUF( dst, type ) \
    dst = *((type *) &buf[byte_pos]); \
    byte_pos += sizeof(type);

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
//|               pCmDev            [in]     Pointer to Cm Device
//|               pCommonISACode    [in]     Pointer to memory where common isa locates
//|               size              [in]     Size of memory 
//|               pProgram          [in]     Reference to pointer to CmProgram
//|               options           [in]     jitter or non-jitter
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::Create( CmDeviceRT* pCmDev, void* pCISACode, const uint32_t uiCISACodeSize, void* pGenCode, const uint32_t uiGenCodeSize, CmProgramRT*& pProgram,  const char* options, const uint32_t programId )
{
    int32_t result = CM_SUCCESS;
    pProgram = new (std::nothrow) CmProgramRT( pCmDev, programId );
    if( pProgram )
    {
        pProgram->Acquire();
        result = pProgram->Initialize( pCISACode, uiCISACodeSize, pGenCode, uiGenCodeSize, options );
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
int32_t CmProgramRT::Destroy( CmProgramRT* &pProgram )
{
    long refCount = pProgram->SafeRelease(  );
    if( refCount == 0 )
    {
        pProgram = nullptr;
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
CmProgramRT::CmProgramRT( CmDeviceRT* pCmDev, uint32_t programId ): 
    m_pCmDev( pCmDev ),
    m_ProgramCodeSize( 0 ), 
    m_pProgramCode(nullptr), 
    m_ISAfile(nullptr),
    m_Options( nullptr ),
    m_SurfaceCount( 0 ),
    m_KernelCount( 0 ), 
    m_pKernelInfo( CM_INIT_KERNEL_PER_PROGRAM ),
    m_IsJitterEnabled(false),
    m_IsHwDebugEnabled(false),
    m_refCount(0),
    m_programIndex(programId),
    m_fJITCompile(nullptr),
    m_fFreeBlock(nullptr),
    m_fJITVersion(nullptr),
    m_CISA_magicNumber(0),
    m_CISA_majorVersion(0),
    m_CISA_minorVersion(0)
{
    CmSafeMemSet(m_IsaFileName,0,sizeof(char)*CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE);
}

//*-----------------------------------------------------------------------------
//| Purpose:    Destructor of Cm Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
CmProgramRT::~CmProgramRT( void )
{
    MosSafeDeleteArray( m_Options );
    MosSafeDeleteArray( m_pProgramCode );
    for( uint32_t i = 0; i < m_KernelCount; i ++ )
    {
        uint32_t refCount = this->ReleaseKernelInfo(i);
        CM_ASSERT(refCount == 0);

    }
    m_pKernelInfo.Delete();
    CmSafeDelete(m_ISAfile);
}

#if (_RELEASE_INTERNAL)
int32_t CmProgramRT::ReadUserFeatureValue(const char *pcMessageKey, uint32_t &value)
{
    MOS_USER_FEATURE        UserFeature;
    MOS_USER_FEATURE_VALUE  UserFeatureValue = __NULL_USER_FEATURE_VALUE__;
    CM_RETURN_CODE          hr = CM_SUCCESS;

    // Set the component's message level and asserts:
    UserFeatureValue.u32Data    = __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE;
    UserFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath           = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues         = &UserFeatureValue;
    UserFeature.uiNumValues     = 1;

    CHK_MOSSTATUS_RETURN_CMERROR(MOS_UserFeature_ReadValue(
                                nullptr,
                                &UserFeature,
                                pcMessageKey,
                                MOS_USER_FEATURE_VALUE_TYPE_UINT32));

    value = UserFeature.pValues->u32Data;

finish:
    return hr;
}
#endif

//*-----------------------------------------------------------------------------
//| Purpose:    Initialize Cm Program
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::Initialize( void* pCISACode, const uint32_t uiCISACodeSize, void* pGenCode, const uint32_t uiGenCodeSize, const char* options )
{
    bool bLoadingGPUCopyKernel = false;
    int32_t result = CM_SUCCESS;
    int32_t hr     = CM_FAILURE;
    
    m_IsJitterEnabled = true; //by default jitter is ON

    int numJitFlags = 0;
    const char *jitFlags[CM_RT_JITTER_MAX_NUM_FLAGS];
    CmSafeMemSet(jitFlags, 0, sizeof(char *) * CM_RT_JITTER_MAX_NUM_FLAGS);

    char* pFlagStepInfo = nullptr;

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
            m_Options = MOS_NewArray(char, (length + 1));
            if( !m_Options )
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                return CM_OUT_OF_HOST_MEMORY;

            }
            CmFastMemCopy( m_Options, options, length);
            m_Options[ length ] = '\0';

            if(strstr(options, "nojitter"))
                m_IsJitterEnabled = false;

            if(!strcmp(m_Options, "PredefinedGPUKernel"))
                bLoadingGPUCopyKernel = true;
            if( (m_IsJitterEnabled == true) && (bLoadingGPUCopyKernel == false))
            { // option: "nonjitter" and "PredefinedGPUCopyKernel" should not be passed to jitter
                char *token = nullptr;
                char *next_token = nullptr;
                char *ptr =  m_Options;

                while( nullptr != (token = strtok_s(ptr," ",&next_token)))
                {
                    if(numJitFlags >= CM_RT_JITTER_MAX_NUM_USER_FLAGS)
                    {
                        CM_ASSERTMESSAGE("Error: Invalid jitter user flags number.");
                        MosSafeDeleteArray(m_Options);
                        return CM_FAILURE;
                    }
                    if(!strcmp(token, CM_RT_JITTER_DEBUG_FLAG))
                    {
                        m_IsHwDebugEnabled = true;
                    }
                    jitFlags[numJitFlags] = token;
                    numJitFlags++;
                    ptr = next_token;
                }
            }
        }
    }

    uint8_t *buf = (uint8_t*)pCISACode;
    uint32_t byte_pos = 0;

    READ_FIELD_FROM_BUF(m_CISA_magicNumber, uint32_t);
    READ_FIELD_FROM_BUF(m_CISA_majorVersion, uint8_t);
    READ_FIELD_FROM_BUF(m_CISA_minorVersion, uint8_t);

    bool bUseVisaApi = true;
    vISA::Header *header = nullptr;

    auto getVersionAsInt = [](int major, int minor) {return major * 100 + minor;};
    if (getVersionAsInt(m_CISA_majorVersion, m_CISA_minorVersion) < getVersionAsInt(3, 2))
    {
        bUseVisaApi = false;
    }
    else
    {
        m_ISAfile = new vISA::ISAfile((uint8_t*)pCISACode, uiCISACodeSize);
        if (!m_ISAfile->readFile())
        {
            CM_ASSERTMESSAGE("Error: invalid VISA.");
            MosSafeDeleteArray(m_Options);
            return CM_INVALID_COMMON_ISA;
        }
        header = m_ISAfile->getHeader();
    }

    if (m_CISA_magicNumber != CISA_MAGIC_NUMBER)
    {
        CM_ASSERTMESSAGE("Error: Invalid CISA magic number.");
        MosSafeDeleteArray(m_Options);
        return CM_INVALID_COMMON_ISA;
    }

    if(bLoadingGPUCopyKernel)//for predefined kernel(GPUCopy), forcely disable jitting
    {
        m_IsJitterEnabled = false;
    }

    const char *platform = nullptr;
    
    PCM_HAL_STATE  pCmHalState = \
        ((PCM_CONTEXT_DATA)m_pCmDev->GetAccelData())->pCmHalState;
    CMCHK_NULL(pCmHalState);

    pCmHalState->pCmHalInterface->GetGenPlatformInfo(nullptr, nullptr, &platform);
    
    if( m_IsJitterEnabled )
    {
    //reg control for svm IA/GT cache coherence
#if (_RELEASE_INTERNAL)
        uint32_t value = 0;
        if (ReadUserFeatureValue(CM_RT_USER_FEATURE_FORCE_COHERENT_STATELESSBTI, value) == CM_SUCCESS && value == 1)
        {
            jitFlags[numJitFlags] = CM_RT_JITTER_NCSTATELESS_FLAG;
            numJitFlags++;
        }

#endif // (_RELEASE_INTERNAL)

        //Load jitter library and add function pointers to program
        // Get hmodule from CmDevice_RT or CmDevice_Sim, which is casted from CmDevice
        result = m_pCmDev->LoadJITDll();
        if(result != CM_SUCCESS)
        {
            CM_ASSERTMESSAGE("Error: Load jitter library failure.");
            MosSafeDeleteArray(m_Options);
            return result;
        }

        m_pCmDev->GetJITCompileFnt(m_fJITCompile);
        m_pCmDev->GetFreeBlockFnt(m_fFreeBlock);
        m_pCmDev->GetJITVersionFnt(m_fJITVersion);

        uint32_t jitMajor = 0;
        uint32_t jitMinor = 0;
        m_fJITVersion(jitMajor, jitMinor);
        if((jitMajor < m_CISA_majorVersion) || (jitMajor == m_CISA_majorVersion && jitMinor < m_CISA_minorVersion))
            return CM_JITDLL_OLDER_THAN_ISA;
#if USE_EXTENSION_CODE
        if( m_pCmDev->CheckGTPinEnabled() && !bLoadingGPUCopyKernel)
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
        m_pCmDev->GetGenStepInfo(stepstr);
        if(stepstr != nullptr)
        {
            pFlagStepInfo = MOS_NewArray(char, CM_JIT_FLAG_SIZE);
            if (pFlagStepInfo)
            {
                jitFlags[numJitFlags] = "-stepping";
                numJitFlags++;

                CmSafeMemSet(pFlagStepInfo, 0, CM_JIT_FLAG_SIZE);
                MOS_SecureStringPrint(pFlagStepInfo, CM_JIT_FLAG_SIZE, CM_JIT_FLAG_SIZE, "%s", stepstr);
                if (numJitFlags >= CM_RT_JITTER_MAX_NUM_FLAGS)
                {
                    CM_ASSERTMESSAGE("Error: Invalid jitter user flags number.");
                    hr = CM_FAILURE;
                    goto finish;
                }
                jitFlags[numJitFlags] = pFlagStepInfo;
                numJitFlags++;
            }
            else
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                MosSafeDeleteArray(m_Options);
                return CM_OUT_OF_HOST_MEMORY;
            }
        }
    }

    if (bUseVisaApi)
    {
        m_KernelCount = header->getNumKernels();
    }
    else
    {
        byte_pos = 4 + 1 + 1; // m_CISA_magicNumber:sizeof (uint32_t) + m_CISA_majorVersion:sizeof(uint8_t) + m_CISA_minorVersion:sizeof(uint8_t)
        unsigned short num_kernels;
        READ_FIELD_FROM_BUF(num_kernels, unsigned short);
        m_KernelCount = num_kernels;
    }

#ifdef _DEBUG
    if(m_IsJitterEnabled)
        CM_NORMALMESSAGE("Jitter Compiling...");
#endif

    for (uint32_t i = 0; i < m_KernelCount; i++)
    {
        CM_KERNEL_INFO* pKernInfo = new (std::nothrow) CM_KERNEL_INFO;
        if (!pKernInfo)
        {
            CM_ASSERTMESSAGE("Error: Out of system memory.");
            MosSafeDeleteArray(pFlagStepInfo);
            MosSafeDeleteArray(m_Options);
            hr = CM_OUT_OF_HOST_MEMORY;
            goto finish;
        }
        CmSafeMemSet(pKernInfo, 0, sizeof(CM_KERNEL_INFO));

        vISA::Kernel *kernel = nullptr;
        uint8_t name_len = 0;
        if (bUseVisaApi)
        {
            kernel = header->getKernelInfo()[i];
            name_len = kernel->getNameLen();
            CmFastMemCopy(pKernInfo->kernelName, kernel->getName(), name_len);
        }
        else
        {
            READ_FIELD_FROM_BUF(name_len, uint8_t);
            CmFastMemCopy(pKernInfo->kernelName, buf + byte_pos, name_len);
            // move byte_pos to the right index
            byte_pos += name_len;
        }
       
        if(m_IsJitterEnabled)
        {
            if (bUseVisaApi)
            {
                pKernInfo->kernelIsaOffset = kernel->getOffset();
                pKernInfo->kernelIsaSize = kernel->getSize();
                pKernInfo->inputCountOffset = kernel->getInputOffset();
            }
            else
            {
                uint32_t kernelIsaOffset;
                uint32_t kernelIsaSize;
                uint32_t inputCountOffset;

                READ_FIELD_FROM_BUF(kernelIsaOffset, uint32_t); //read kernel isa offset
                READ_FIELD_FROM_BUF(kernelIsaSize, uint32_t); //read kernel isa size
                READ_FIELD_FROM_BUF(inputCountOffset, uint32_t);

                pKernInfo->kernelIsaOffset = kernelIsaOffset;
                pKernInfo->kernelIsaSize = kernelIsaSize;
                pKernInfo->inputCountOffset = inputCountOffset;

                // read relocation symbols
                unsigned short num_var_syms = 0, num_func_syms = 0;
                unsigned char num_gen_binaries = 0;

                READ_FIELD_FROM_BUF(num_var_syms, uint16_t);
                // CISA layout of var_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                byte_pos += sizeof(uint16_t) * num_var_syms;
                byte_pos += sizeof(uint16_t) * num_var_syms;

                READ_FIELD_FROM_BUF(num_func_syms, uint16_t);
                // CISA layout of func_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                byte_pos += sizeof(uint16_t) * num_func_syms;
                byte_pos += sizeof(uint16_t) * num_func_syms;

                // num_gen_binaries
                READ_FIELD_FROM_BUF(num_gen_binaries, uint8_t);
                // CISA layout of gen_binaries is gen_platform[0], binary offset[0],
                // binary size[0]...binary size[n-1]. Skip all.
                // skip all gen_platform
                byte_pos += sizeof(uint8_t) * num_gen_binaries;
                // skip all binary offset
                byte_pos += sizeof(uint32_t) * num_gen_binaries;
                // skip all binary size
                byte_pos += sizeof(uint32_t) * num_gen_binaries;

                // byte_pos should point to name_len of next kernel
            }
        }
        else // non jitting
        {
            uint8_t num_gen_binaries = 0;
            if (bUseVisaApi)
            {
                pKernInfo->kernelIsaOffset = kernel->getOffset();
                pKernInfo->inputCountOffset = kernel->getInputOffset();
                num_gen_binaries = kernel->getNumGenBinaries();
            }
            else
            {
                uint32_t kernelIsaOffset;
                READ_FIELD_FROM_BUF(kernelIsaOffset, uint32_t); //read kernel isa offset
                pKernInfo->kernelIsaOffset = kernelIsaOffset;

                // skipping kernelIsaSize
                byte_pos += 4;

                uint32_t inputCountOffset;
                READ_FIELD_FROM_BUF(inputCountOffset, uint32_t);
                pKernInfo->inputCountOffset = inputCountOffset;

                // read relocation symbols
                unsigned short num_var_syms = 0, num_func_syms = 0;

                READ_FIELD_FROM_BUF(num_var_syms, uint16_t);
                // CISA layout of var_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                byte_pos += sizeof(uint16_t) * num_var_syms;
                byte_pos += sizeof(uint16_t) * num_var_syms;

                READ_FIELD_FROM_BUF(num_func_syms, uint16_t);
                // CISA layout of func_syms is symbolic_index[0], resolved_index[0],
                // ...resolved_index[n-1]. Skip all.
                byte_pos += sizeof(uint16_t) * num_func_syms;
                byte_pos += sizeof(uint16_t) * num_func_syms;

                // num_gen_binaries
                READ_FIELD_FROM_BUF(num_gen_binaries, uint8_t);
            }
            
            uint32_t genxBinaryOffset = 0;
            uint32_t genxBinarySize = 0;
            for (int j = 0; j < num_gen_binaries; j++)
            {
                vISA::GenBinary *genBinary = nullptr;
                uint8_t gen_platform = 0;
                uint32_t offset = 0;
                uint32_t size = 0;

                if (bUseVisaApi)
                {
                    genBinary = kernel->getGenBinaryInfo()[j];
                    gen_platform = genBinary->getGenPlatform();
                    offset = genBinary->getBinaryOffset();
                    size = genBinary->getBinarySize();
                }
                else
                {
                    // gen_platform
                    READ_FIELD_FROM_BUF(gen_platform, uint8_t);
                    // binary offset
                    READ_FIELD_FROM_BUF(offset, uint32_t);
                    // binary size
                    READ_FIELD_FROM_BUF(size, uint32_t);
                }
           
                if (pCmHalState->pCmHalInterface->IsCisaIDSupported((uint32_t)gen_platform))
                {
                    // assign correct offset/size based on platform
                    genxBinaryOffset = offset;
                    genxBinarySize = size;
                }
                else
                {
                    MosSafeDeleteArray(pFlagStepInfo);
                    MosSafeDeleteArray(m_Options);
                    CmSafeDelete(pKernInfo);
                    return CM_INVALID_GENX_BINARY;
                }
            }

            if (pGenCode == nullptr)
            {
                pKernInfo->genxBinaryOffset = genxBinaryOffset; 
                pKernInfo->genxBinarySize = genxBinarySize;
            }
            else 
            {
                // If user provided Gen binary passed, will use it instead of binary from FAT CISA
                pKernInfo->genxBinaryOffset = uiCISACodeSize; 
                pKernInfo->genxBinarySize = uiGenCodeSize;
            }

            if ( pKernInfo->genxBinarySize == 0 || pKernInfo->genxBinaryOffset == 0 )
            {
                CM_ASSERTMESSAGE("Error: Invalid genx binary.");
                MosSafeDeleteArray(pFlagStepInfo);
                MosSafeDeleteArray(m_Options);
                CmSafeDelete(pKernInfo);
                return CM_INVALID_GENX_BINARY;
            }
        }

        if(m_IsJitterEnabled)
        {
            pKernInfo->jitBinaryCode = 0;
            void* jitBinary = 0;// = (void**)malloc(m_KernelCount*sizeof(void*));
            uint32_t jitBinarySize = 0;// = (uint32_t*)malloc(m_KernelCount*sizeof(uint32_t*));
            char* errorMsg = (char*)malloc(CM_JIT_ERROR_MESSAGE_SIZE);
            if (errorMsg == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                CmSafeDelete(pKernInfo);
                hr = CM_OUT_OF_HOST_MEMORY;
                goto finish;
            }
            CmSafeMemSet( errorMsg, 0, CM_JIT_ERROR_MESSAGE_SIZE );

            FINALIZER_INFO *jitProfInfo = (FINALIZER_INFO *)malloc(CM_JIT_PROF_INFO_SIZE);
            if(jitProfInfo == nullptr)
            {
                CM_ASSERTMESSAGE("Error: Out of system memory.");
                free(errorMsg);
                CmSafeDelete(pKernInfo);
                hr = CM_OUT_OF_HOST_MEMORY;
                goto finish;
            }
            CmSafeMemSet( jitProfInfo, 0, CM_JIT_PROF_INFO_SIZE );


            result = m_fJITCompile( pKernInfo->kernelName, (uint8_t*)pCISACode, uiCISACodeSize,
                                    jitBinary, jitBinarySize, platform, m_CISA_majorVersion, m_CISA_minorVersion, numJitFlags, jitFlags, errorMsg, jitProfInfo );
           

            //if error code returned or error message not nullptr
            if(result != CM_SUCCESS)// || errorMsg[0])
            {
                CM_NORMALMESSAGE("%s.", errorMsg);
                free(errorMsg);
                CmSafeDelete(pKernInfo);
                hr = CM_JIT_COMPILE_FAILURE;
                goto finish;
            }

            // if spill code exists and scrach space disabled, return error to user
            if( jitProfInfo->isSpill &&  m_pCmDev->IsScratchSpaceDisabled())
            {
                CmSafeDelete(pKernInfo);
                free(errorMsg);
                return CM_INVALID_KERNEL_SPILL_CODE;
            }
            
            free(errorMsg);

            pKernInfo->jitBinaryCode = jitBinary;
            pKernInfo->jitBinarySize = jitBinarySize;
            pKernInfo->jitInfo = jitProfInfo;

#if USE_EXTENSION_CODE
            if ( m_IsHwDebugEnabled )
            {
                NotifyKernelBinary(this->m_pCmDev,
                                   this,
                                   pKernInfo->kernelName,
                                   jitBinary,
                                   jitBinarySize,
                                   jitProfInfo->genDebugInfo,
                                   jitProfInfo->genDebugInfoSize,
                                   nullptr,
                                   m_pCmDev->GetDriverStoreFlag());
            }
#endif
        }

        m_pKernelInfo.SetElement( i, pKernInfo );
        this->AcquireKernelInfo(i);
    }

#ifdef _DEBUG
    if(m_IsJitterEnabled)
        CM_NORMALMESSAGE("Jitter Done.");
#endif

    // now byte_pos index to the start of common isa body;
    // compute the code size for common isa
    m_ProgramCodeSize = uiCISACodeSize + uiGenCodeSize;
    m_pProgramCode = MOS_NewArray(uint8_t, m_ProgramCodeSize);
    if( !m_pProgramCode )
    {
        CM_ASSERTMESSAGE("Error: Out of system memory.");
        hr = CM_OUT_OF_HOST_MEMORY;
        goto finish;
    }

    //Copy CISA content
    CmFastMemCopy((void *)m_pProgramCode, pCISACode, uiCISACodeSize);

    //Copy User provided Gen binary content if it has
    if(pGenCode)
    {
        CmFastMemCopy(m_pProgramCode + uiCISACodeSize, pGenCode, uiGenCodeSize);
    }

    hr = CM_SUCCESS;

finish:
    MosSafeDeleteArray(pFlagStepInfo);
    if(hr != CM_SUCCESS )
    {
        MosSafeDeleteArray(m_Options);
        MosSafeDeleteArray(m_pProgramCode);
    }
    return hr;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get size and address of Common Isa
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetCommonISACode( void* & pCommonISACode, uint32_t & size ) 
{
    pCommonISACode = (void *)m_pProgramCode;
    size = m_ProgramCodeSize;

    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the count of kernel
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetKernelCount( uint32_t& kernelCount )
{
    kernelCount = m_KernelCount;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the Kernel's Infomation
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetKernelInfo( uint32_t index, CM_KERNEL_INFO*& pKernelInfo )
{
    if( index < m_KernelCount )
    {
        pKernelInfo = (CM_KERNEL_INFO*)m_pKernelInfo.GetElement( index ) ;
        return CM_SUCCESS;
    }
    else
    {
        pKernelInfo = nullptr;
        return CM_FAILURE;
    }
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the name of ISA file
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetIsaFileName( char* & isaFileName )
{
    isaFileName = m_IsaFileName;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get Kernel's options
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::GetKernelOptions( char* & kernelOptions )
{
    kernelOptions = m_Options;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Get the number of Surfaces
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmProgramRT::GetSurfaceCount(void)
{
    return m_SurfaceCount;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Set Program's surface count
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
int32_t CmProgramRT::SetSurfaceCount(uint32_t count)
{
    m_SurfaceCount = count;
    return CM_SUCCESS;
}

//*-----------------------------------------------------------------------------
//| Purpose:    Acquire Kernel Info
//| Returns:    Result of the operation.
//*-----------------------------------------------------------------------------
uint32_t CmProgramRT::AcquireKernelInfo(uint32_t index)
{
    CM_KERNEL_INFO* pKernelInfo = nullptr;

    if( index < m_KernelCount )
    {
        pKernelInfo = (CM_KERNEL_INFO *)m_pKernelInfo.GetElement( index ) ;
        if (pKernelInfo)
        {
            CM_ASSERT( (int32_t)pKernelInfo->kernelInfoRefCount >= 0 );
            CM_ASSERT( pKernelInfo->kernelInfoRefCount < UINT_MAX );

            ++ pKernelInfo->kernelInfoRefCount;
            return pKernelInfo->kernelInfoRefCount;
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
    CM_KERNEL_INFO* pKernelInfo = nullptr;

    if( index < m_KernelCount )
    {
        pKernelInfo = (CM_KERNEL_INFO *)m_pKernelInfo.GetElement( index ) ;
        if (pKernelInfo)
        {
            CM_ASSERT( pKernelInfo->kernelInfoRefCount > 0 );

            -- pKernelInfo->kernelInfoRefCount;

            if (pKernelInfo->kernelInfoRefCount == 1)
            {
                /////////////////////////////////////////////////////////////
                //Free global string memory space, Start
                for (int i = 0; i < (int) pKernelInfo->globalStringCount; i ++)
                {
                    if (pKernelInfo->globalStrings[i])
                    {
                        free((void *)pKernelInfo->globalStrings[i]);
                    }
                }
                if (pKernelInfo->globalStrings)
                {
                    free((void *)pKernelInfo->globalStrings);
                    pKernelInfo->globalStrings = nullptr;
                    pKernelInfo->globalStringCount = 0;
                }
                //Free global string memory space, End
                /////////////////////////////////////////////////////////////

                for (uint32_t i = 0; i < pKernelInfo->surface_count; i++) {
                    if (pKernelInfo->surface[i].attribute_count && pKernelInfo->surface[i].attributes) {
                        free(pKernelInfo->surface[i].attributes);
                    }
                }
                if (pKernelInfo->surface) {
                    free(pKernelInfo->surface);
                    pKernelInfo->surface = nullptr;
                    pKernelInfo->surface_count = 0;
                }

                return 1;
            }

            else if (pKernelInfo->kernelInfoRefCount == 0)
            {
                if(m_IsJitterEnabled)
                {
                    if(pKernelInfo && pKernelInfo->jitBinaryCode)
                        m_fFreeBlock(pKernelInfo->jitBinaryCode);
                    if(pKernelInfo && pKernelInfo->jitInfo)
                        free(pKernelInfo->jitInfo);
                }

                /////////////////////////////////////////////////////////////
                //Free global string memory space, Start
                for (int i = 0; i < (int) pKernelInfo->globalStringCount; i ++)
                {
                    if (pKernelInfo->globalStrings[i])
                    {
                        free((void *)pKernelInfo->globalStrings[i]);
                    }
                }
                if (pKernelInfo->globalStrings)
                {
                    free((void *)pKernelInfo->globalStrings);
                }
                //Free global string memory space, End
                /////////////////////////////////////////////////////////////

                for (uint32_t i = 0; i < pKernelInfo->surface_count; i++) {
                    if (pKernelInfo->surface[i].attribute_count && pKernelInfo->surface[i].attributes) {
                        free(pKernelInfo->surface[i].attributes);
                    }
                }
                if (pKernelInfo->surface) {
                    free(pKernelInfo->surface);
                }

                CmSafeDelete( pKernelInfo );

                m_pKernelInfo.SetElement(index, nullptr);

                return 0;
            }
            else
            {
                return pKernelInfo->kernelInfoRefCount;
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
    CM_KERNEL_INFO* pKernelInfo = nullptr;

    refCount = 0;

    if( index < m_KernelCount )
    {
        pKernelInfo =(CM_KERNEL_INFO *) m_pKernelInfo.GetElement( index ) ;
        if (pKernelInfo)
        {
            refCount = pKernelInfo->kernelInfoRefCount;
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
    majorVersion = m_CISA_majorVersion;
    minorVersion = m_CISA_minorVersion;

    return CM_SUCCESS;
}

uint32_t CmProgramRT::GetProgramIndex()
{
    return m_programIndex;
}

vISA::ISAfile *CmProgramRT::getISAfile()
{
    return m_ISAfile;
}
}