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

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPROGRAM_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPROGRAM_H_

#include "cm_def.h"
#include "cm_array.h"
#include "cm_jitter_info.h"
#include "cm_visa.h"

struct attribute_info_t
{
    unsigned short nameIndex;
    unsigned char size;
    unsigned char* values;
    char *name;
};

struct gen_var_info_t
{
    unsigned short nameIndex;
    unsigned char bitProperties;
    unsigned short numElements;
    unsigned short aliasIndex;
    unsigned short aliasOffset;
    unsigned char attributeCount;
    attribute_info_t* attributes;
} ;

struct spec_var_info_t
{
    unsigned short nameIndex;
    unsigned short numElements;
    unsigned char attributeCount;
    attribute_info_t* attributes;
};

struct label_info_t
{
    unsigned short nameIndex;
    unsigned char kind;
    unsigned char attributeCount;
    attribute_info_t* attributes;
};

struct CM_KERNEL_INFO
{
    char kernelName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE ];
    uint32_t inputCountOffset;

    //Used to store the input for the jitter from CISA
    uint32_t kernelIsaOffset;
    uint32_t kernelIsaSize;

    //Binary Size
    union
    {
        uint32_t jitBinarySize;
        uint32_t genxBinarySize;
    };

    union
    {
        void* jitBinaryCode;   //pointer to code created by jitter
        uint32_t genxBinaryOffset; //pointer to binary offset in CISA (use when jit is not enabled)
    };

    //Just a copy for original binary pointer and size (GTPin using only)
    void* origBinary;
    uint32_t origBinarySize;

    uint32_t globalStringCount;
    const char** globalStrings;
    char kernelASMName[CM_MAX_KERNEL_NAME_SIZE_IN_BYTE + 1];        //The name of the Gen assembly file for this kernel (no extension)
    uint8_t kernelSLMSize;     //Size of the SLM used by each thread group
    bool blNoBarrier;       //Indicate if the barrier is used in kernel: true means no barrier used, false means barrier is used.

    FINALIZER_INFO *jitInfo;

    uint32_t variableCount;
    gen_var_info_t *variables;
    uint32_t addressCount;
    spec_var_info_t *address;
    uint32_t predicateCount;
    spec_var_info_t *predicates;
    uint32_t labelCount;
    label_info_t *label;
    uint32_t samplerCount;
    spec_var_info_t *sampler;
    uint32_t surfaceCount;
    spec_var_info_t *surface;
    uint32_t vmeCount;
    spec_var_info_t *vme;

    uint32_t kernelInfoRefCount;    //reference counter for kernel info to reuse kernel info and jitbinary
    uint64_t hashValue;
};

//Function pointer definition for jitter compilation functions.
typedef int (__cdecl *pJITCompile)(const char *kernelName,
                                   const void *kernelIsa,
                                   uint32_t kernelIsaSize,
                                   void* &genBinary,
                                   uint32_t &genBinarySize,
                                   const char *platform,
                                   int majorVersion,
                                   int minorVersion,
                                   int numArgs,
                                   const char *args[],
                                   char *errorMsg,
                                   FINALIZER_INFO *jitInfo);

typedef int (__cdecl *pJITCompile_v2)(const char *kernelName,
                                   const void *kernelIsa,
                                   uint32_t kernelIsaSize,
                                   void* &genBinary,
                                   uint32_t &genBinarySize,
                                   const char *platform,
                                   int majorVersion,
                                   int minorVersion,
                                   int numArgs,
                                   const char *args[],
                                   char *errorMsg,
                                   FINALIZER_INFO *jitInfo,
                                   void *extra_info);

typedef void (__cdecl *pFreeBlock)(void*);

typedef void (__cdecl *pJITVersion)(unsigned int &majorV,
                                    unsigned int &minorV);

#define CM_JIT_FLAG_SIZE           256
#define CM_JIT_ERROR_MESSAGE_SIZE  512
#define CM_JIT_PROF_INFO_SIZE      4096
#define CM_RT_JITTER_MAX_NUM_FLAGS 30

#define JITCOMPILE_FUNCTION_STR "JITCompile"
#define JITCOMPILEV2_FUNCTION_STR "JITCompile_v2"
#define FREEBLOCK_FUNCTION_STR  "freeBlock"
#define JITVERSION_FUNCTION_STR "getJITVersion"

namespace CMRT_UMD
{
class CmDeviceRT;

class CmProgram
{
public:
    virtual int32_t GetCommonISACode(void* & commonISACode, uint32_t & size) = 0;
};

//*-----------------------------------------------------------------------------
//! CM Program
//*-----------------------------------------------------------------------------
class CmProgramRT : public CmProgram
{
public:
    static int32_t Create( CmDeviceRT* device, void* cisaCode, const uint32_t cisaCodeSize, CmProgramRT*& program,  const char* options, const uint32_t programId );
    static int32_t Destroy( CmProgramRT* &program );

    int32_t GetCommonISACode( void* & commonISACode, uint32_t & size ) ;
    int32_t GetKernelCount( uint32_t& kernelCount );
    int32_t GetKernelInfo( uint32_t index, CM_KERNEL_INFO*& kernelInfo );
    int32_t GetIsaFileName( char* & kernelName );
    int32_t GetKernelOptions( char* & kernelOptions );

    uint32_t GetSurfaceCount(void);
    int32_t SetSurfaceCount(uint32_t count);

    bool IsJitterEnabled( void ){ return m_isJitterEnabled; }
    bool IsHwDebugEnabled (void ){ return m_isHwDebugEnabled;}

    uint32_t AcquireKernelInfo(uint32_t index);
    uint32_t ReleaseKernelInfo(uint32_t index);
    int32_t GetKernelInfoRefCount(uint32_t index, uint32_t& refCount);

    int32_t GetCISAVersion(uint32_t& majorVersion, uint32_t& minorVersion);

    int32_t Acquire( void);
    int32_t SafeRelease( void);

    uint32_t GetProgramIndex();

    //! \brief    get m_isaFile object
    //! \detail   m_isaFile object provides methods to read, parse and write ISA files.
    //! \return   Pointer to m_isaFile object
    vISA::ISAfile *getISAfile();

protected:
    CmProgramRT( CmDeviceRT* device, uint32_t programId );
    ~CmProgramRT( void );

    int32_t Initialize( void* cisaCode, const uint32_t cisaCodeSize, const char* options );
#if USE_EXTENSION_CODE
    int InitForGTPin(const char *jitFlags[CM_RT_JITTER_MAX_NUM_FLAGS], int &numJitFlags);
#endif
    uint32_t GetKernelInfoHash(CM_KERNEL_INFO *kernelInfo);

    CmDeviceRT* m_device;

    uint32_t m_programCodeSize;
    uint8_t *m_programCode;
    vISA::ISAfile* m_isaFile;
    char* m_options;
    char m_isaFileName[ CM_MAX_ISA_FILE_NAME_SIZE_IN_BYTE ];
    uint32_t m_surfaceCount;

    uint32_t m_kernelCount;
    CmDynamicArray m_kernelInfo;

    bool m_isJitterEnabled;
    bool m_isHwDebugEnabled;

    uint32_t m_refCount;

    uint32_t m_programIndex;

    //Function point to JIT compiling functions, get from CmDevice_RT or CmDeviceSim
    pJITCompile     m_fJITCompile;
    pFreeBlock      m_fFreeBlock;
    pJITVersion     m_fJITVersion;
    pJITCompile_v2  m_fJITCompile_v2;

public:
    uint32_t m_cisaMagicNumber;
    uint8_t m_cisaMajorVersion;
    uint8_t m_cisaMinorVersion;

private:
    CmProgramRT (const CmProgramRT& other);
    CmProgramRT& operator= (const CmProgramRT& other);
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPROGRAM_H_
