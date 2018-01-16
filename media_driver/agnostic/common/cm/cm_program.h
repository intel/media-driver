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
    unsigned short name_index;
    unsigned char size;
    unsigned char* values;
    char *name;
};

struct gen_var_info_t
{
    unsigned short name_index;
    unsigned char bit_properties;
    unsigned short num_elements;
    unsigned short alias_index;
    unsigned short alias_offset;
    unsigned char attribute_count;
    attribute_info_t* attributes;
} ;

struct spec_var_info_t
{
    unsigned short name_index;
    unsigned short num_elements;
    unsigned char attribute_count;
    attribute_info_t* attributes;
};

struct label_info_t
{
    unsigned short name_index;
    unsigned char kind;
    unsigned char attribute_count;
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
    void* pOrigBinary;
    uint32_t uiOrigBinarySize;

    uint32_t globalStringCount;
    const char** globalStrings;
    char kernelASMName[CM_MAX_KERNEL_NAME_SIZE_IN_BYTE + 1];        //The name of the Gen assembly file for this kernel (no extension)
    uint8_t kernelSLMSize;     //Size of the SLM used by each thread group
    bool blNoBarrier;       //Indicate if the barrier is used in kernel: true means no barrier used, false means barrier is used.

    FINALIZER_INFO *jitInfo;

    uint32_t variable_count;
    gen_var_info_t *variables;
    uint32_t address_count;
    spec_var_info_t *address;
    uint32_t predicte_count;
    spec_var_info_t *predictes;
    uint32_t label_count;
    label_info_t *label;
    uint32_t sampler_count;
    spec_var_info_t *sampler;
    uint32_t surface_count;
    spec_var_info_t *surface;
    uint32_t vme_count;
    spec_var_info_t *vme;

    uint32_t kernelInfoRefCount;    //reference counter for kernel info to reuse kernel info and jitbinary
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

typedef void (__cdecl *pFreeBlock)(void*);

typedef void (__cdecl *pJITVersion)(unsigned int &majorV,
                                    unsigned int &minorV);

#define CM_JIT_FLAG_SIZE           256
#define CM_JIT_ERROR_MESSAGE_SIZE  512
#define CM_JIT_PROF_INFO_SIZE      4096
#define CM_RT_JITTER_MAX_NUM_FLAGS 30

#define JITCOMPILE_FUNCTION_STR "JITCompile"
#define FREEBLOCK_FUNCTION_STR  "freeBlock"
#define JITVERSION_FUNCTION_STR "getJITVersion"

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

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMPROGRAM_H_
