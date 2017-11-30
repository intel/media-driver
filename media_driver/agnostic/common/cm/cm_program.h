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
//! \file      cm_program.h  
//! \brief     Contains Class CmProgram definitions  
//!
#pragma once

#include "cm_def.h"
#include "cm_array.h"
#include "cm_visa.h"

namespace CMRT_UMD
{

class CmDeviceRT;

class CmProgram
{
public:
    virtual int32_t GetCommonISACode(void* & pCommonISACode, uint32_t & size) = 0;
};


//*-----------------------------------------------------------------------------
//! CM Program
//*-----------------------------------------------------------------------------
class CmProgramRT : public CmProgram
{
public:
    static int32_t Create( CmDeviceRT* pCmDev, void* pCISACode, const uint32_t uiCISACodeSize, void* pGenCode, const uint32_t uiGenCodeSize, CmProgramRT*& pProgram,  const char* options, const uint32_t programId );
    static int32_t Destroy( CmProgramRT* &pProgram );

    int32_t GetCommonISACode( void* & pCommonISACode, uint32_t & size ) ;
    int32_t GetKernelCount( uint32_t& kernelCount );
    int32_t GetKernelInfo( uint32_t index, CM_KERNEL_INFO*& pKernelInfo );
    int32_t GetIsaFileName( char* & kernelName );
    int32_t GetKernelOptions( char* & kernelOptions );

    uint32_t GetSurfaceCount(void);
    int32_t SetSurfaceCount(uint32_t count);

    bool IsJitterEnabled( void ){ return m_IsJitterEnabled; }
    bool IsHwDebugEnabled (void ){ return m_IsHwDebugEnabled;}

    uint32_t AcquireKernelInfo(uint32_t index);
    uint32_t ReleaseKernelInfo(uint32_t index);
    int32_t GetKernelInfoRefCount(uint32_t index, uint32_t& refCount);

    int32_t GetCISAVersion(uint32_t& majorVersion, uint32_t& minorVersion);

    int32_t Acquire( void);
    int32_t SafeRelease( void);
    
    uint32_t GetProgramIndex();

#if (_RELEASE_INTERNAL)
    int32_t ReadUserFeatureValue(const char *pcMessageKey, uint32_t &value);
#endif

    //! \brief    get ISAfile object
    //! \detail   ISAfile object provides methods to read, parse and write ISA files.
    //! \return   Pointer to ISAfile object
    vISA::ISAfile *getISAfile();

protected:
    CmProgramRT( CmDeviceRT* pCmDev, uint32_t programId );
    ~CmProgramRT( void );

    int32_t Initialize( void* pCISACode, const uint32_t uiCISACodeSize, void* pGenCode, const uint32_t uiGenCodeSize, const char* options );
#if USE_EXTENSION_CODE
    int InitForGTPin(const char *jitFlags[CM_RT_JITTER_MAX_NUM_FLAGS], int &numJitFlags);
#endif
    CmDeviceRT* m_pCmDev;

    uint32_t m_ProgramCodeSize;
    uint8_t *m_pProgramCode;
    vISA::ISAfile* m_ISAfile;
    char* m_Options;
    char m_IsaFileName[ CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE ];
    uint32_t m_SurfaceCount;

    uint32_t m_KernelCount;
    CmDynamicArray m_pKernelInfo;

    bool m_IsJitterEnabled;
    bool m_IsHwDebugEnabled;

    uint32_t m_refCount;

    uint32_t m_programIndex;

    //Function point to JIT compiling functions, get from CmDevice_RT or CmDeviceSim
    pJITCompile     m_fJITCompile;
    pFreeBlock      m_fFreeBlock;
    pJITVersion     m_fJITVersion;

public:
    uint32_t m_CISA_magicNumber;
    uint8_t m_CISA_majorVersion;
    uint8_t m_CISA_minorVersion;   

private:
    CmProgramRT (const CmProgramRT& other);
    CmProgramRT& operator= (const CmProgramRT& other);
};
}; //namespace
