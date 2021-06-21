/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_utilities_next.cpp
//! \brief    Common OS service across different platform
//! \details  Common OS service across different platform
//!

#include "mos_utilities.h"
#include "mos_util_debug_next.h"
#include "media_user_settings_mgr.h"
#include <sstream>
#include <fcntl.h>     //open
#include <string.h>    // memset
#include <stdlib.h>    // atoi atol
#include <math.h>
#include "mos_os.h"

#if MOS_MESSAGES_ENABLED
#include <time.h>     //for simulate random memory allcation failure
#endif

int32_t MosUtilities::m_mosMemAllocCounterNoUserFeature            = 0;
int32_t MosUtilities::m_mosMemAllocCounterNoUserFeatureGfx         = 0;
uint8_t MosUtilities::m_mosUltFlag                                 = 0;

int32_t MosUtilities::m_mosMemAllocCounter                         = 0;
int32_t MosUtilities::m_mosMemAllocFakeCounter                     = 0;
int32_t MosUtilities::m_mosMemAllocCounterGfx                      = 0;

bool MosUtilities::m_enableAddressDump = false;

MOS_FUNC_EXPORT void MosUtilities::MosSetUltFlag(uint8_t ultFlag)
{
    MosUtilities::m_mosUltFlag = ultFlag;
}

MOS_FUNC_EXPORT int32_t MosUtilities::MosGetMemNinjaCounter()
{
    return m_mosMemAllocCounterNoUserFeature;
}

MOS_FUNC_EXPORT int32_t MosUtilities::MosGetMemNinjaCounterGfx()
{
    return m_mosMemAllocCounterNoUserFeatureGfx;
}

#define __MAX_MULTI_STRING_COUNT         128

char MosUtilities::m_xmlFilePath[MOS_USER_CONTROL_MAX_DATA_SIZE] = {};

#if (_DEBUG || _RELEASE_INTERNAL)

uint32_t MosUtilities::m_mosAllocMemoryFailSimulateMode = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateFreq = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateHint = 0;
uint32_t MosUtilities::m_mosAllocMemoryFailSimulateAllocCounter = 0;

#define MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT (0)
#define MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM (1)
#define MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE (2)

#define MIN_MEMORY_ALLOC_FAIL_FREQ (1)      //max memory allcation fail rate 100%
#define MAX_MEMORY_ALLOC_FAIL_FREQ (10000)  //min memory allcation fail rate 1/10000

#define MosAllocMemoryFailSimulationEnabled                                         \
    (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM ||  \
     m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE)

void MosUtilities::MosInitAllocMemoryFailSimulateFlag(MOS_CONTEXT_HANDLE mosCtx)
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureValueData;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    //default off for simulate random fail
    m_mosAllocMemoryFailSimulateMode            = MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT;
    m_mosAllocMemoryFailSimulateFreq            = 0;
    m_mosAllocMemoryFailSimulateHint            = 0;
    m_mosAllocMemoryFailSimulateAllocCounter    = 0;

    // Read Config : memory allocation failure simulate mode
    MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_MODE_ID,
        &userFeatureValueData,
        mosCtx);

    if ((userFeatureValueData.u32Data == MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT) ||
        (userFeatureValueData.u32Data == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM) ||
        (userFeatureValueData.u32Data == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE))
    {
        m_mosAllocMemoryFailSimulateMode = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init MosSimulateAllocMemoryFailSimulateMode as %d \n ", m_mosAllocMemoryFailSimulateMode);
    }
    else
    {
        m_mosAllocMemoryFailSimulateMode = MEMORY_ALLOC_FAIL_SIMULATE_MODE_DEFAULT;
        MOS_OS_NORMALMESSAGE("Invalid Alloc Memory Fail Simulate Mode from config: %d \n ", userFeatureValueData.u32Data);
    }

    // Read Config : memory allocation failure simulate frequence
    MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_FREQ_ID,
        &userFeatureValueData,
        mosCtx);

    if ((userFeatureValueData.u32Data >= MIN_MEMORY_ALLOC_FAIL_FREQ) &&
        (userFeatureValueData.u32Data <= MAX_MEMORY_ALLOC_FAIL_FREQ))
    {
        m_mosAllocMemoryFailSimulateFreq = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init m_MosSimulateRandomAllocMemoryFailFreq as %d \n ", m_mosAllocMemoryFailSimulateFreq);

        if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM)
        {
            srand((unsigned int)time(nullptr));
        }
    }
    else
    {
        m_mosAllocMemoryFailSimulateFreq = 0;
        MOS_OS_NORMALMESSAGE("Invalid Alloc Memory Fail Simulate Freq from config: %d \n ", userFeatureValueData.u32Data);
    }

    // Read Config : memory allocation failure simulate counter
    MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ALLOC_MEMORY_FAIL_SIMULATE_HINT_ID,
        &userFeatureValueData,
        mosCtx);

    if (userFeatureValueData.u32Data <= m_mosAllocMemoryFailSimulateFreq)
    {
        m_mosAllocMemoryFailSimulateHint = userFeatureValueData.u32Data;
        MOS_OS_NORMALMESSAGE("Init m_MosAllocMemoryFailSimulateHint as %d \n ", m_mosAllocMemoryFailSimulateHint);
    }
    else
    {
        m_mosAllocMemoryFailSimulateHint = m_mosAllocMemoryFailSimulateFreq;
        MOS_OS_NORMALMESSAGE("Set m_mosAllocMemoryFailSimulateHint as %d since INVALID CONFIG %d \n ", m_mosAllocMemoryFailSimulateHint, userFeatureValueData.u32Data);
    }
}

bool MosUtilities::MosSimulateAllocMemoryFail(
    size_t      size,
    size_t      alignment,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
{
    bool  bSimulateAllocFail = false;

    if (!MosAllocMemoryFailSimulationEnabled)
    {
        return false;
    }

    if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_RANDOM)
    {
        int32_t Rn = rand();
        m_mosAllocMemoryFailSimulateAllocCounter++;
        if (Rn % m_mosAllocMemoryFailSimulateFreq == 1)
        {
            bSimulateAllocFail = true;
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, \
                "Simulated Allocate Memory Fail (Rn=%d, SimulateAllocCounter=%d) for: functionName: %s, filename: %s, line: %d, size: %d, alignment: %d \n", \
                Rn, m_mosAllocMemoryFailSimulateAllocCounter, functionName, filename, line, size, alignment);
        }
        else
        {
            bSimulateAllocFail = false;
        }
    }
    else if (m_mosAllocMemoryFailSimulateMode == MEMORY_ALLOC_FAIL_SIMULATE_MODE_TRAVERSE)
    {
        if (m_mosAllocMemoryFailSimulateAllocCounter++ == m_mosAllocMemoryFailSimulateHint)
        {
            MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, \
                "Simulated Allocate Memory Fail (hint=%d) for: functionName: %s, filename: %s, line: %d, size: %d \n", \
                 m_mosAllocMemoryFailSimulateHint, functionName, filename, line, size, alignment);
            bSimulateAllocFail = true;
        }
        else
        {
            bSimulateAllocFail = false;
        }
    }
    else
    {
        MOS_OS_NORMALMESSAGE("Invalid m_MosAllocMemoryFailSimulateMode: %d \n ", m_mosAllocMemoryFailSimulateMode);
        bSimulateAllocFail = false;
    }

    return bSimulateAllocFail;
}
#endif  // #if (_DEBUG || _RELEASE_INTERNAL)

MOS_STATUS MosUtilities::MosUtilitiesInit(MOS_CONTEXT_HANDLE mosCtx)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    eStatus = MosOsUtilitiesInit(mosCtx);

#if (_DEBUG || _RELEASE_INTERNAL)
    //Initialize MOS simulate random alloc memorflag
    MosInitAllocMemoryFailSimulateFlag(mosCtx);

    MOS_USER_FEATURE_VALUE_DATA userFeatureValueData;

    MosZeroMemory(&userFeatureValueData, sizeof(userFeatureValueData));
    MosUserFeatureReadValueID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_RESOURCE_ADDR_DUMP_ENABLE_ID,
        &userFeatureValueData,
        mosCtx);
    MosUtilities::m_enableAddressDump = userFeatureValueData.i32Data ? true : false;
#endif

    return eStatus;
}

MOS_STATUS MosUtilities::MosUtilitiesClose(MOS_CONTEXT_HANDLE mosCtx)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_FUNCTION_ENTER;

    MediaUserSettingsMgr::MediaUserSettingClose();
    MediaUserSetting::MediaUserSetting::Destroy();

    // MOS_OS_Utilitlies_Close must be called right before end of function
    // Because Memninja will calc mem leak here.
    // Any memory allocation release after MosOsUtilitiesClose() will be treated as mem leak.
    eStatus = MosOsUtilitiesClose(mosCtx);

#if (_DEBUG || _RELEASE_INTERNAL)
    //Reset Simulate Alloc Memory Fail flags
    MosInitAllocMemoryFailSimulateFlag(mosCtx);
#endif

    return eStatus;
}

void MosUtilities::MosFreeUserFeatureValueString(PMOS_USER_FEATURE_VALUE_STRING pUserString)
{
    if (pUserString != nullptr)
    {
        if (pUserString->uSize > 0)
        {
            if (pUserString->pStringData)
            {
                MOS_FreeMemAndSetNull(pUserString->pStringData);
                m_mosMemAllocFakeCounter--;
            }
            pUserString->uSize = 0;
        }
    }
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosAlignedAllocMemoryUtils(
    size_t      size,
    size_t      alignment,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void  *MosUtilities::MosAlignedAllocMemory(
    size_t  size,
    size_t  alignment)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, alignment, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = _aligned_malloc(size, alignment);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicIncrement(&m_mosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void MosUtilities::MosAlignedFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void MosUtilities::MosAlignedFreeMemory(void *ptr)
#endif // MOS_MESSAGES_ENABLED
{
    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicDecrement(&m_mosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);

        _aligned_free(ptr);
    }
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosAllocMemoryUtils(
    size_t      size,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void *MosUtilities::MosAllocMemory(size_t size)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = malloc(size);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosAtomicIncrement(&m_mosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosAllocAndZeroMemoryUtils(
    size_t      size,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void *MosUtilities::MosAllocAndZeroMemory(size_t size)
#endif // MOS_MESSAGES_ENABLED
{
    void  *ptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(size, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    ptr = malloc(size);

    MOS_OS_ASSERT(ptr != nullptr);

    if(ptr != nullptr)
    {
        MosZeroMemory(ptr, size);

        MosAtomicIncrement(&m_mosMemAllocCounter);
        MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line);
    }

    return ptr;
}

#if MOS_MESSAGES_ENABLED
void *MosUtilities::MosReallocMemoryUtils(
    void       *ptr,
    size_t     newSize,
    const char *functionName,
    const char *filename,
    int32_t    line)
#else
void *MosUtilities::MosReallocMemory(
    void       *ptr,
    size_t     newSize)
#endif // MOS_MESSAGES_ENABLED
{
    void *oldPtr = nullptr;
    void *newPtr = nullptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MosSimulateAllocMemoryFail(newSize, NO_ALLOC_ALIGNMENT, functionName, filename, line))
    {
        return nullptr;
    }
#endif

    oldPtr = ptr;
    newPtr = realloc(ptr, newSize);

    MOS_OS_ASSERT(newPtr != nullptr);

    if (newPtr != oldPtr)
    {
        if (oldPtr != nullptr)
        {
            MosAtomicDecrement(&m_mosMemAllocCounter);
            MOS_MEMNINJA_FREE_MESSAGE(oldPtr, functionName, filename, line);
        }

        if (newPtr != nullptr)
        {
            MosAtomicIncrement(&m_mosMemAllocCounter);
            MOS_MEMNINJA_ALLOC_MESSAGE(newPtr, newSize, functionName, filename, line);
        }
    }

    return newPtr;
}

//!
//! \brief    Wrapper for free(). Performs error checking.
//! \details  Wrapper for free(). Performs error checking.
//!           It decreases memory allocation counter variable
//!           m_mosMemAllocCounter for checking memory leaks.
//! \param    void  *ptr
//!           [in] Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MosUtilities::MosFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line)
#else
void MosUtilities::MosFreeMemory(void  *ptr)
#endif // MOS_MESSAGES_ENABLED
{
    if(ptr != nullptr)
    {
        MosAtomicDecrement(&m_mosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);

        free(ptr);
    }
}

void MosUtilities::MosZeroMemory(void  *pDestination, size_t stLength)
{
    MOS_OS_ASSERT(pDestination != nullptr);

    if(pDestination != nullptr)
    {
        memset(pDestination, 0, stLength);
    }
}

void MosUtilities::MosFillMemory(void  *pDestination, size_t stLength, uint8_t bFill)
{
    MOS_OS_ASSERT(pDestination != nullptr);

    if(pDestination != nullptr)
    {
        memset(pDestination, bFill, stLength);
    }
}

MOS_STATUS  MosUtilities::MosReadFileToPtr(
    const char      *pFilename,
    uint32_t        *lpNumberOfBytesRead,
    void            **ppReadBuffer)
{
    HANDLE          hFile;
    void            *lpBuffer;
    uint32_t        fileSize;
    uint32_t        bytesRead;
    MOS_STATUS      eStatus;

    *ppReadBuffer = nullptr;
    *lpNumberOfBytesRead = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_RDONLY);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    eStatus = MosGetFileSize(hFile, &fileSize, nullptr);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get size of file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    lpBuffer = MOS_AllocAndZeroMemory(fileSize);
    if (lpBuffer == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
        MosCloseHandle(hFile);
        return MOS_STATUS_NO_SPACE;
    }

    if((eStatus = MosReadFile(hFile, lpBuffer, fileSize, &bytesRead, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to read from file '%s'.", pFilename);
        MosCloseHandle(hFile);
        MOS_FreeMemory(lpBuffer);
        lpBuffer = nullptr;
        return eStatus;
    }

    MosCloseHandle(hFile);
    *lpNumberOfBytesRead = bytesRead;
    *ppReadBuffer = lpBuffer;
    return eStatus;
}

MOS_STATUS MosUtilities::MosWriteFileFromPtr(
    const char      *pFilename,
    void            *lpBuffer,
    uint32_t        writeSize)
{
    HANDLE          hFile;
    uint32_t        bytesWritten;
    MOS_STATUS      eStatus;

    MOS_OS_CHK_NULL(pFilename);
    MOS_OS_CHK_NULL(lpBuffer);

    if (writeSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("Attempting to write 0 bytes to a file");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }

    bytesWritten    = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_WRONLY|O_CREAT);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        goto finish;
    }

    if((eStatus = MosWriteFile(hFile, lpBuffer, writeSize, &bytesWritten, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", pFilename);
        MosCloseHandle(hFile);
        goto finish;
    }

    MosCloseHandle(hFile);

finish:
    return eStatus;
}

MOS_STATUS MosUtilities::MosAppendFileFromPtr(
    const char      *pFilename,
    void            *pData,
    uint32_t        dwSize)
{
    MOS_STATUS  eStatus;
    HANDLE      hFile;
    uint32_t    dwWritten;

    //------------------------------
    MOS_OS_ASSERT(pFilename);
    MOS_OS_ASSERT(pData);
    //------------------------------
    dwWritten   = 0;

    eStatus = MosCreateFile(&hFile, (char *)pFilename, O_WRONLY | O_CREAT | O_APPEND);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to open file '%s'.", pFilename);
        return eStatus;
    }

    eStatus = MosSetFilePointer(hFile, 0, nullptr, SEEK_END);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set file pointer'%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    // Write the file
    if((eStatus = MosWriteFile(hFile, pData, dwSize, &dwWritten, nullptr)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write to file '%s'.", pFilename);
        MosCloseHandle(hFile);
        return eStatus;
    }

    MosCloseHandle(hFile);
    return eStatus;
}

/*****************************************************************************
|
|                           USER FEATURE Functions
|
*****************************************************************************/

MOS_STATUS MosUtilities::MosWriteOneUserFeatureKeyToXML(PMOS_USER_FEATURE_VALUE pUserFeature)
{
    char                            sOutBuf[MOS_USER_CONTROL_MAX_DATA_SIZE];
    char                            ValueType[MAX_USER_FEATURE_FIELD_LENGTH];
    char                            KeyPath[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL_RETURN(pUserFeature);

    switch (pUserFeature->Type)
    {
    case MOS_USER_FEATURE_TYPE_USER:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "UFINT\\%s",
            pUserFeature->pcPath);
        break;
    case MOS_USER_FEATURE_TYPE_SYSTEM:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "UFEXT\\%s",
            pUserFeature->pcPath);
        break;
    default:
        MosSecureStringPrint(
            KeyPath,
            sizeof(KeyPath),
            sizeof(KeyPath),
            "%s",pUserFeature->pcPath);
        break;
     }

    switch (pUserFeature->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "bool");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "dword");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "qword");
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "string");
        break;
    default:
        MosSecureStringPrint(
            ValueType,
            sizeof(ValueType),
            sizeof(ValueType),
            "unknown");
        break;
     }

    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "    <Key name=\"%s\" type=\"%s\" location=\"%s\" defaultval=\"%s\" description=\"%s\" />\n",
        pUserFeature->pValueName,
        ValueType,
        KeyPath,
        pUserFeature->DefaultValue,
        pUserFeature->pcDescription);
    MosAppendFileFromPtr(
        m_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return eStatus;
}

MOS_STATUS MosUtilities::MosWriteOneUserFeatureGroupToXML(MOS_USER_FEATURE_VALUE   UserFeatureFilter)
{
    char                                sOutBuf[MAX_USER_FEATURE_FIELD_LENGTH];
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    // Group Header Start
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "  <Group name=\"%s\">\n",
        UserFeatureFilter.pcGroup);
    eStatus = MosAppendFileFromPtr(
        m_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));

    // Group User Feature Keys
    eStatus = MosGetItemFromMosUserFeatureDescField(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        &MosWriteOneUserFeatureKeyToXML,
        &UserFeatureFilter);

    // Group Header End
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "  </Group>\n",
        UserFeatureFilter.pcGroup);
    eStatus = MosAppendFileFromPtr(
        m_xmlFilePath,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return eStatus;
}

MOS_STATUS MosUtilities::MosGenerateUserFeatureKeyXML(MOS_CONTEXT_HANDLE mosCtx)
{
    char                                sOutBuf[MAX_USER_FEATURE_FIELD_LENGTH];
    uint32_t                            uiIndex=0;
    MOS_USER_FEATURE_VALUE              UserFeatureFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    const char * const                  FilterGroups[] = { "Codec", "Decode", "Encode", "CP", "General", "MOS",
                                                           "Report", "VP", "Media", "Secure HEVC Encode", "MDF"};
    uint32_t                            FilterGroupsCount = sizeof(FilterGroups) / sizeof(FilterGroups[0]);
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    // Check if XML dump is enabled by User Feature Key
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    eStatus = MosUserFeatureReadValueID(
                    nullptr,
                    __MOS_USER_FEATURE_KEY_XML_AUTOGEN_ID,
                    &UserFeatureData,
                    mosCtx);
    if (UserFeatureData.u32Data == 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = m_xmlFilePath;
    eStatus = MosUserFeatureReadValueID(
                    nullptr,
                    __MOS_USER_FEATURE_KEY_XML_FILEPATH_ID,
                    &UserFeatureData,
                    mosCtx);
    // User Feature Key Header Start
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "<UserFeatureKeys>\n");
    eStatus = MosWriteFileFromPtr(
        UserFeatureData.StringData.pStringData,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    // User Feature Key Groups
    for (uiIndex = 0; uiIndex < FilterGroupsCount; uiIndex++)
    {
        UserFeatureFilter.pcGroup = FilterGroups[uiIndex];
        eStatus = MosWriteOneUserFeatureGroupToXML(UserFeatureFilter);
    }

    // User Feature Key Header End
    MosZeroMemory(sOutBuf, sizeof(sOutBuf));
    MosSecureStringPrint(
        sOutBuf,
        sizeof(sOutBuf),
        sizeof(sOutBuf),
        "</UserFeatureKeys>\n");
    eStatus = MosAppendFileFromPtr(
        UserFeatureData.StringData.pStringData,
        sOutBuf,
        (uint32_t)strlen(sOutBuf));
    return    eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureSetMultiStringValue(
    PMOS_USER_FEATURE_VALUE_DATA     pFeatureData,
    uint32_t                         dwSize)
{
    PMOS_USER_FEATURE_VALUE_STRING  pStrings;
    uint32_t                        uiNumStrings;
    uint32_t                        ui;
    char                            *pData;
    char                            *pCurData;
    uint32_t                        dwLen;
    uint32_t                        dwPos;

    MOS_OS_CHK_NULL_RETURN(pFeatureData);
    MOS_OS_ASSERT(dwSize);

    pStrings = pFeatureData->MultiStringData.pStrings;
    pData = pFeatureData->MultiStringData.pMultStringData;
    dwPos = 0;
    uiNumStrings = 0;

    MOS_OS_ASSERT(pStrings);
    MOS_OS_ASSERT(pData);

    // Find number of strings in the multi string array
    do
    {
        pCurData = pData + dwPos;
        dwLen = (uint32_t)strlen(pCurData);
        if (dwLen == 0)
        {
            MOS_OS_NORMALMESSAGE("Invalid user feature key entry.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        uiNumStrings++;
        dwPos += dwLen + 1;

        if (dwPos >= (dwSize - 1))
        {
            // last entry
            break;
        }
    } while (true);

    // Check the size of MultiStringData
    if (pFeatureData->MultiStringData.uCount < uiNumStrings)
    {
        MOS_OS_NORMALMESSAGE("pFeatureValue->MultiStringData.uCount is smaller than the actual necessary number.");
        return MOS_STATUS_UNKNOWN;
    }

    // Populate Array
    dwPos = 0;
    for (ui = 0; ui < uiNumStrings; ui++)
    {
        pCurData = pData + dwPos;
        dwLen = (uint32_t)strlen(pCurData);
        MOS_OS_ASSERT(dwLen > 0);
        pStrings[ui].pStringData = pCurData;
        pStrings[ui].uSize = dwLen;

        dwPos += dwLen + 1;
    }

    pFeatureData->MultiStringData.uCount = uiNumStrings;
    pFeatureData->MultiStringData.uSize = dwPos;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosCopyUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA pSrcData,
    PMOS_USER_FEATURE_VALUE_DATA pDstData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                            ui;
    PMOS_USER_FEATURE_VALUE_STRING      pSrcString = nullptr;
    PMOS_USER_FEATURE_VALUE_STRING      pDstString = nullptr;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_ASSERT(pSrcData);
    MOS_OS_ASSERT(pDstData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------

    switch(ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        pDstData->bData = pSrcData->bData;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        pDstData->i32Data = pSrcData->i32Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        pDstData->i64Data = pSrcData->i64Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        pDstData->u32Data = pSrcData->u32Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        pDstData->u64Data = pSrcData->u64Data;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        pDstData->fData = pSrcData->fData;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        if ((pSrcData->StringData.pStringData != nullptr) && (strlen(pSrcData->StringData.pStringData) != 0))
        {
            pDstData->StringData.uMaxSize = pSrcData->StringData.uMaxSize;
            pDstData->StringData.uSize = pSrcData->StringData.uSize;
            if (pDstData->StringData.pStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureMemcpy(
                pDstData->StringData.pStringData,
                pDstData->StringData.uSize,
                pSrcData->StringData.pStringData,
                pSrcData->StringData.uSize);
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        if ((pSrcData->MultiStringData.pMultStringData != nullptr) && (strlen(pSrcData->MultiStringData.pMultStringData) != 0))
        {
            pDstData->MultiStringData.uCount = pSrcData->MultiStringData.uCount;
            pDstData->MultiStringData.uMaxSize = pSrcData->MultiStringData.uMaxSize;
            pDstData->MultiStringData.uSize = pSrcData->MultiStringData.uSize;
            if (pDstData->MultiStringData.pMultStringData != nullptr)
            {
                eStatus = MosSecureMemcpy(
                    pDstData->MultiStringData.pMultStringData,
                    pDstData->MultiStringData.uSize,
                    pSrcData->MultiStringData.pMultStringData,
                    pSrcData->MultiStringData.uSize);

                for (ui = 0; ui < pSrcData->MultiStringData.uCount; ui++)
                {
                    pSrcString = &pSrcData->MultiStringData.pStrings[ui];
                    pDstString = &pDstData->MultiStringData.pStrings[ui];

                    MOS_OS_CHK_NULL(pSrcString);
                    MOS_OS_CHK_NULL(pDstString);
                    pDstString->uMaxSize = pSrcString->uMaxSize;
                    pDstString->uSize = pSrcString->uSize;
                    if (pDstString->pStringData != nullptr)
                    {
                        eStatus = MosSecureMemcpy(
                            pDstString->pStringData,
                            pDstString->uSize+1,
                            pSrcString->pStringData,
                            pSrcString->uSize+1);
                    }// if
                }// for
            }
        }// if
        break;
    default:
        break;
    }
finish:
    return eStatus;
}

MOS_STATUS MosUtilities::MosAssignUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA    pDstData,
    const char                      *pData,
    MOS_USER_FEATURE_VALUE_TYPE     ValueType
)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    uint32_t                        dwUFSize = 0;

    //------------------------------
    MOS_OS_ASSERT(pData);
    MOS_OS_ASSERT(pDstData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------

    switch(ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        pDstData->bData = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        pDstData->i32Data = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        pDstData->i64Data = atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        pDstData->u32Data = atoi(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        pDstData->u64Data = atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        pDstData->fData = (float)atol(pData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        pDstData->StringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
        if ((pData != nullptr) && (strlen(pData) != 0))
        {
            pDstData->StringData.uSize = (uint32_t)strlen(pData) + 1;
            if (pDstData->StringData.uSize > pDstData->StringData.uMaxSize)
            {
                pDstData->StringData.uSize = pDstData->StringData.uMaxSize;
            }
            pDstData->StringData.pStringData = (char *)MOS_AllocAndZeroMemory(strlen(pData) + 1);
            if (pDstData->StringData.pStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            m_mosMemAllocFakeCounter++;
            eStatus = MosSecureStrcpy(
                pDstData->StringData.pStringData,
                pDstData->StringData.uSize,
                (char *)pData);
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:

        pDstData->MultiStringData.uCount = MOS_USER_MAX_STRING_COUNT;
        pDstData->MultiStringData.uMaxSize = MOS_USER_CONTROL_MAX_DATA_SIZE;
        pDstData->MultiStringData.pStrings = (PMOS_USER_FEATURE_VALUE_STRING)MOS_AllocAndZeroMemory(sizeof(MOS_USER_FEATURE_VALUE_STRING) * __MAX_MULTI_STRING_COUNT);
        if (pDstData->MultiStringData.pStrings == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            pDstData->MultiStringData.pMultStringData = nullptr;
            pDstData->MultiStringData.uSize           = 0;
            pDstData->MultiStringData.uCount          = 0;
            return MOS_STATUS_NULL_POINTER;
        }
        if ((pData != nullptr) && (strlen(pData) != 0))
        {
            MOS_SafeFreeMemory(pDstData->MultiStringData.pMultStringData);
            pDstData->MultiStringData.pMultStringData = (char *)MOS_AllocAndZeroMemory(strlen(pData) + 1);
            if (pDstData->MultiStringData.pMultStringData == nullptr)
            {
                MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
                return MOS_STATUS_NULL_POINTER;
            }
            eStatus = MosSecureMemcpy(
                pDstData->MultiStringData.pMultStringData,
                strlen(pData),
                (char *)pData,
                strlen(pData));
            if ((eStatus = MosUserFeatureSetMultiStringValue(
                pDstData,
                dwUFSize)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to set multi string value.");
                return eStatus;
            }
        }
        break;
    default:
        break;
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureSetDefaultValues(
    PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
    uint32_t                           uiNumOfValues)
{
    uint32_t                ui;
    PMOS_USER_FEATURE_VALUE pUserFeature = nullptr;
    uint32_t                ValueID      = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_STATUS              eStatus      = MOS_STATUS_UNKNOWN;

    //--------------------------------------------------
    MOS_OS_CHK_NULL_RETURN(pWriteValues);
    //--------------------------------------------------
    for (ui = 0; ui < uiNumOfValues; ui++)
    {
        ValueID = pWriteValues[ui].ValueID;
        pUserFeature = MosUtilUserInterface::GetValue(ValueID);
        MOS_OS_CHK_NULL_RETURN(pUserFeature);
        // Copy the write data into corresponding user feature value
        MosCopyUserFeatureValueData(
            &pWriteValues[ui].Value,
            &pUserFeature->Value,
            pUserFeature->ValueType);
    }
    eStatus = MOS_STATUS_SUCCESS;
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKey(PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey);
    //------------------------------

    eStatus = MosAssignUserFeatureValueData(
        &pUserFeatureKey->Value,
        pUserFeatureKey->DefaultValue,
        pUserFeatureKey->ValueType);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        MosUtilUserInterface::AddEntry(pUserFeatureKey->ValueID, pUserFeatureKey);
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureData(PMOS_USER_FEATURE_VALUE_DATA pData,MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                    ui;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    if (pData == nullptr)
    {
        return eStatus;
    }
    //------------------------------

    switch (ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        MosFreeUserFeatureValueString(&pData->StringData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        for (ui = 0; ui < pData->MultiStringData.uCount; ui++)
        {
            MosFreeUserFeatureValueString(&pData->MultiStringData.pStrings[ui]);
        }
        MOS_SafeFreeMemory(pData->MultiStringData.pStrings);
        pData->MultiStringData.pStrings = nullptr;
        pData->MultiStringData.pMultStringData = nullptr;
        pData->MultiStringData.uSize = 0;
        pData->MultiStringData.uCount = 0;
        break;
    default:
        break;
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKey(PMOS_USER_FEATURE_VALUE pUserFeatureKey)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    //------------------------------
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey);
    //------------------------------

    MosUtilUserInterface::DelEntry(pUserFeatureKey->ValueID);
    eStatus = MosDestroyUserFeatureData(
        &pUserFeatureKey->Value,
        pUserFeatureKey->ValueType);

    return eStatus;
}

MOS_STATUS MosUtilities::MosIsCorrectDefaultValueType(
    const char                  *pData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType)
{
    uint32_t                    dwLen;
    uint32_t                    ui;
    int32_t                     IntVal;
    MOS_STATUS                  eStatus = MOS_STATUS_INVALID_PARAMETER;

    dwLen = (uint32_t)strlen(pData);
    //------------------------------
    MOS_OS_ASSERT(pData);
    MOS_OS_ASSERT(ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    //------------------------------
    switch (ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        if ((!strcmp(pData, "0")) || (!strcmp(pData, "1")))
        {
            eStatus = MOS_STATUS_SUCCESS;
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        eStatus = MOS_STATUS_SUCCESS;
        for (ui = 0; ui<dwLen; ui++)
        {
            IntVal = pData[ui] - '0';
            if ((0 > IntVal) || (9 < IntVal))
            {
                if ((((ui == 0)&&(pData[ui] - '-') != 0)) && ((pData[ui] - '.') != 0))
                {
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        eStatus = MOS_STATUS_SUCCESS;
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
        eStatus = MOS_STATUS_SUCCESS;
        break;
    default:
        break;
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosIsCorrectUserFeatureDescField(PMOS_USER_FEATURE_VALUE pUserFeatureKey, uint32_t maxKeyID)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey);
    if ((pUserFeatureKey->ValueID <= __MOS_USER_FEATURE_KEY_INVALID_ID) ||
        (pUserFeatureKey->ValueID >= maxKeyID))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pValueName);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pcPath);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pcWritePath);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKey->pcGroup);

    if ((pUserFeatureKey->pcDescription != nullptr) &&
        (strlen(pUserFeatureKey->pcDescription) > MAX_USER_FEATURE_FIELD_LENGTH))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    eStatus = MosIsCorrectDefaultValueType(
        pUserFeatureKey->DefaultValue,
        pUserFeatureKey->ValueType);
    return eStatus;
}

MOS_STATUS MosUtilities::MosGetItemFromMosUserFeatureDescField(
    MOS_USER_FEATURE_VALUE      *descTable,
    uint32_t                    numOfItems,
    uint32_t                    maxId,
    MOS_STATUS                  (*CallbackFunc)(PMOS_USER_FEATURE_VALUE),
    PMOS_USER_FEATURE_VALUE     pUserFeatureKeyFilter)
{
    uint32_t    uiIndex = 0;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    //------------------------------
    MOS_OS_CHK_NULL_RETURN(CallbackFunc);
    MOS_OS_CHK_NULL_RETURN(pUserFeatureKeyFilter);
    MOS_OS_CHK_NULL_RETURN(descTable);
    //------------------------------

    for (uiIndex = __MOS_USER_FEATURE_KEY_INVALID_ID; uiIndex < numOfItems; uiIndex++)
    {
        if (MosIsCorrectUserFeatureDescField(&descTable[uiIndex], maxId) != MOS_STATUS_SUCCESS)
        {
            continue;
        }

        if ((pUserFeatureKeyFilter->ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID) && (pUserFeatureKeyFilter->ValueID != descTable[uiIndex].ValueID))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pValueName != nullptr) && (strcmp(pUserFeatureKeyFilter->pValueName, descTable[uiIndex].pValueName) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcPath != nullptr) && (strcmp(pUserFeatureKeyFilter->pcPath, descTable[uiIndex].pcPath) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcWritePath != nullptr) && (strcmp(pUserFeatureKeyFilter->pcWritePath, descTable[uiIndex].pcWritePath) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->pcGroup != nullptr) && (strcmp(pUserFeatureKeyFilter->pcGroup, descTable[uiIndex].pcGroup) != 0))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->Type != MOS_USER_FEATURE_TYPE_INVALID) && (pUserFeatureKeyFilter->Type != descTable[uiIndex].Type))
        {
            continue;
        }
        if ((pUserFeatureKeyFilter->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID) && (pUserFeatureKeyFilter->ValueType != descTable[uiIndex].ValueType))
        {
            continue;
        }
        eStatus = (*CallbackFunc)(&descTable[uiIndex]);

    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId)
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    eStatus = MosGetItemFromMosUserFeatureDescField(
        descTable,
        numOfItems,
        maxId,
        &MosDeclareUserFeatureKey,
        &UserFeatureKeyFilter);
    return eStatus;
}

MOS_STATUS MosUtilities::MosDeclareUserFeatureKeysForAllDescFields()
{
    MOS_OS_CHK_STATUS_RETURN(MosDeclareUserFeatureKeysFromDescFields(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId)
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    eStatus = MosGetItemFromMosUserFeatureDescField(
        descTable,
        numOfItems,
        maxId,
        &MosDestroyUserFeatureKey,
        &UserFeatureKeyFilter);
    return eStatus;
}

MOS_STATUS MosUtilities::MosDestroyUserFeatureKeysForAllDescFields()
{
    MOS_USER_FEATURE_VALUE      UserFeatureKeyFilter = __NULL_USER_FEATURE_VALUE__;

    MOS_OS_CHK_STATUS_RETURN(MosDestroyUserFeatureKeysFromDescFields(
        m_mosUserFeatureDescFields,
        __MOS_USER_FEATURE_KEY_MAX_ID,
        __MOS_USER_FEATURE_KEY_MAX_ID));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueInit(uint32_t uiNumValues)
{
    // Check if memory is allocated
    if (uiNumValues == 0)
    {
        MOS_OS_ASSERTMESSAGE("pUserFeature->uiNumValues is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

void MosUtilities::MosUserFeatureCallback(
        PTP_CALLBACK_INSTANCE Instance,
        void                  *pvParameter,
        PTP_WAIT              Wait,
        TP_WAIT_RESULT        WaitResult)
{
    PMOS_USER_FEATURE_NOTIFY_DATA  pNotifyData;
    MOS_UNUSED(Instance);
    MOS_UNUSED(Wait);
    MOS_UNUSED(WaitResult);

    MOS_OS_ASSERT(pvParameter);

    pNotifyData = (PMOS_USER_FEATURE_NOTIFY_DATA)pvParameter;
    pNotifyData->bTriggered = true;
}

MOS_STATUS MosUtilities::MosUserFeatureOpen(
    MOS_USER_FEATURE_TYPE KeyType,
    const char            *pSubKey,
    uint32_t              dwAccess,
    void                  **pUFKey,
    MOS_USER_FEATURE_KEY_PATH_INFO  *ufInfo)
{
    MOS_STATUS  eStatus;
    void        *RootKey = 0;

    MOS_OS_ASSERT(pSubKey);
    MOS_OS_ASSERT(pUFKey);

    if (KeyType == MOS_USER_FEATURE_TYPE_USER)
    {
        RootKey = (void *)UFKEY_INTERNAL;
    }
    else if (KeyType == MOS_USER_FEATURE_TYPE_SYSTEM)
    {
        RootKey = (void *)UFKEY_EXTERNAL;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Invalid Key Type %d.", KeyType);
        return MOS_STATUS_UNKNOWN;
    }

    if((eStatus = MosUserFeatureOpenKey(
                             RootKey,
                             pSubKey,
                             0,
                             dwAccess,
                             pUFKey,
                             ufInfo)) !=  MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Unable to open user feature key %s.", pSubKey);
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueBinary(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    void        *pvData;
    uint32_t    dwUFSize;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_BINARY);

    pvData = pFeatureValue->Value.BinaryData.pBinaryData;
    if (!pvData)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->BinaryData.pBinaryData is NULL.");
        return MOS_STATUS_NULL_POINTER;
    }

    dwUFSize = pFeatureValue->Value.BinaryData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->BinaryData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_BINARY,
                  nullptr,
                  pvData,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.BinaryData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.BinaryData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read binary user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }

    pFeatureValue->Value.BinaryData.uSize = dwUFSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFSize;
    char        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];

    //--------------------------------------------------
    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_STRING);
    //--------------------------------------------------

    MosZeroMemory(pcTmpStr, MOS_USER_CONTROL_MAX_DATA_SIZE);
    dwUFSize = pFeatureValue->Value.StringData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->StringData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_SZ,
                  nullptr,
                  pcTmpStr,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.StringData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.StringData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read single string user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }
    if (strlen(pcTmpStr) > 0)
    {
        if (!pFeatureValue->Value.StringData.pStringData)
        {
            m_mosMemAllocFakeCounter++;
        }

        if (pFeatureValue->Value.StringData.uSize < strlen(pcTmpStr) + 1)
        {
            pFeatureValue->Value.StringData.pStringData =
                (char *)MOS_ReallocMemory(pFeatureValue->Value.StringData.pStringData, strlen(pcTmpStr) + 1);
            pFeatureValue->Value.StringData.uSize = strlen(pcTmpStr) + 1;
        }

        MOS_OS_CHK_NULL_RETURN(pFeatureValue->Value.StringData.pStringData);

        MosZeroMemory(pFeatureValue->Value.StringData.pStringData, pFeatureValue->Value.StringData.uSize);

        MosSecureMemcpy(pFeatureValue->Value.StringData.pStringData, pFeatureValue->Value.StringData.uSize, pcTmpStr, strlen(pcTmpStr));
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueMultiString(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFSize;
    char        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING);

    if (!pFeatureValue->Value.MultiStringData.pStrings)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->MultiStringData.pStrings is NULL.");
        return MOS_STATUS_NULL_POINTER;
    }
    MosZeroMemory(pcTmpStr, MOS_USER_CONTROL_MAX_DATA_SIZE);
    dwUFSize = pFeatureValue->Value.MultiStringData.uMaxSize;
    if (dwUFSize == 0)
    {
        MOS_OS_ASSERTMESSAGE("pFeatureValue->MultiStringData.uMaxSize is 0.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  RRF_RT_UF_MULTI_SZ,
                  nullptr,
                  pcTmpStr,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (dwUFSize > pFeatureValue->Value.MultiStringData.uMaxSize) // Buffer size is not enough
        {
            MOS_OS_NORMALMESSAGE("Size %d exceeds max %d.", dwUFSize, pFeatureValue->Value.MultiStringData.uMaxSize);
            return MOS_STATUS_UNKNOWN;
        }
        else // This error case can be hit if the user feature key does not exist.
        {
            MOS_OS_NORMALMESSAGE("Failed to read single string user feature value '%s'.", pFeatureValue->pValueName);
            return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        }
    }

    if (strlen(pcTmpStr) > 0)
    {
        MOS_SafeFreeMemory(pFeatureValue->Value.MultiStringData.pMultStringData);
        pFeatureValue->Value.MultiStringData.pMultStringData = (char *)MOS_AllocAndZeroMemory(strlen(pcTmpStr) + 1);
        m_mosMemAllocFakeCounter++;
        if (pFeatureValue->Value.MultiStringData.pMultStringData == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NULL_POINTER;
        }
        MosSecureMemcpy(
            pFeatureValue->Value.MultiStringData.pMultStringData,
            strlen(pcTmpStr),
            pcTmpStr,
            strlen(pcTmpStr));

    if((eStatus = MosUserFeatureSetMultiStringValue(
            &pFeatureValue->Value,
        dwUFSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to set multi string value.");
        return eStatus;
    }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValuePrimitive(
    void                       *UFKey,
    PMOS_USER_FEATURE_VALUE    pFeatureValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFType = 0;
    uint32_t    dwUFSize;
    void        *pvData = nullptr;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);

    switch(pFeatureValue->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        dwUFType    = RRF_RT_UF_DWORD;
        dwUFSize    = sizeof(uint32_t);
        pvData      = &pFeatureValue->Value.fData;
        break;

    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        dwUFType    = RRF_RT_UF_QWORD;
        dwUFSize    = sizeof(uint64_t);
        pvData      = &pFeatureValue->Value.u64Data;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Invalid primitive value type.");
        return MOS_STATUS_UNKNOWN;
    }

    eStatus = MosUserFeatureGetValue(
                  UFKey,
                  nullptr,
                  pFeatureValue->pValueName,
                  dwUFType,
                  nullptr,
                  pvData,
                  &dwUFSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        // This error case can be hit if the user feature key does not exist.
        MOS_OS_NORMALMESSAGE("Failed to read primitive user feature value \"%s\".", pFeatureValue->pValueName);
        return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS          eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_STRING);
    MOS_OS_ASSERT(pDataValue);

    if((eStatus = MosUserFeatureSetValueEx(
                      UFKey,
                      pFeatureValue->pValueName,
                      0,
                      UF_SZ,
                      (uint8_t*)pDataValue->StringData.pStringData,
                      pDataValue->StringData.uSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write string user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueMultiString(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    PMOS_USER_FEATURE_VALUE_STRING  pStringData;
    uint8_t                         *pData;
    uint8_t                         *pCurData;
    uint32_t                        dwDataSize;
    uint32_t                        dwAvailableSize;
    uint32_t                        ui;
    MOS_STATUS                      eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING);
    MOS_OS_ASSERT(pDataValue);
    MOS_OS_ASSERT(pDataValue->MultiStringData.uCount > 0);

    pData       = nullptr;
    dwDataSize  = 0;

    for (ui = 0; ui < pDataValue->MultiStringData.uCount; ui++)
    {
        pStringData = &pDataValue->MultiStringData.pStrings[ui];
        dwDataSize += pStringData->uSize;
        dwDataSize += 1;                                                        // for \0
    }
    dwDataSize += 1;                                                            // for \0 at the very end (see MULTI_SZ spec)

    // Allocate memory to store data
    pData = (uint8_t*)MOS_AllocAndZeroMemory(dwDataSize);
    if(pData == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
        return MOS_STATUS_NO_SPACE;
    }

    // Copy data from original string array
    pCurData        = pData;
    dwAvailableSize = dwDataSize;
    for (ui = 0; ui < pDataValue->MultiStringData.uCount; ui++)
    {
        pStringData = &pDataValue->MultiStringData.pStrings[ui];
        eStatus = MosSecureMemcpy(pCurData, dwAvailableSize, pStringData->pStringData, pStringData->uSize);
        if(eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to copy memory.");
            goto finish;
        }
        pCurData += pStringData->uSize;
        pCurData++;                                                             // \0 is already added since we zeroed the memory
                                                                                // Very last \0 is already added since we zeroed the memory
        dwAvailableSize -= pStringData->uSize + 1;
    }
    // Write the user feature MULTI_SZ entry
    if((eStatus = MosUserFeatureSetValueEx(
                       UFKey,
                       pFeatureValue->pValueName,
                       0,
                       UF_MULTI_SZ,
                       pData,
                       dwDataSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write multi string user feature value.");
    }

finish:
    MOS_FreeMemory(pData);
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValueBinary(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS      eStatus;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType == MOS_USER_FEATURE_VALUE_TYPE_BINARY);
    MOS_OS_ASSERT(pDataValue);

    if((eStatus = MosUserFeatureSetValueEx(
                       UFKey,
                       pFeatureValue->pValueName,
                       0,
                       UF_BINARY,
                       (uint8_t*)pDataValue->BinaryData.pBinaryData,
                       pDataValue->BinaryData.uSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write binary user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuePrimitive(
    void                            *UFKey,
    PMOS_USER_FEATURE_VALUE         pFeatureValue,
    PMOS_USER_FEATURE_VALUE_DATA    pDataValue)
{
    MOS_STATUS  eStatus;
    uint32_t    dwUFType = UF_NONE;
    uint32_t    dwUFSize = 0;
    void        *pvData = nullptr;

    MOS_OS_ASSERT(UFKey);
    MOS_OS_ASSERT(pFeatureValue);
    MOS_OS_ASSERT(pFeatureValue->pValueName);
    MOS_OS_ASSERT(pFeatureValue->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
    MOS_OS_ASSERT(pDataValue);

    switch(pFeatureValue->ValueType)
    {
    case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
    case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
        dwUFType    = UF_DWORD;
        dwUFSize    = sizeof(uint32_t);
        pvData      = &pDataValue->fData;
        break;

    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        dwUFType    = UF_QWORD;
        dwUFSize    = sizeof(uint64_t);
        pvData      = &pDataValue->u64Data;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Invalid primitive value type.");
        return MOS_STATUS_UNKNOWN;
    }

    if((eStatus = MosUserFeatureSetValueEx(
                        UFKey,
                        pFeatureValue->pValueName,
                        0,
                        dwUFType,
                        (uint8_t*)pvData,
                        dwUFSize)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to write primitive user feature value.");
    }

    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueFromMapID(
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_USER_FEATURE_KEY_PATH_INFO  *ufInfo)
{
    void                        *ufKey           = nullptr;
    PMOS_USER_FEATURE_VALUE     pUserFeature    = nullptr;
    int32_t                     iDataFlag       = MOS_USER_FEATURE_VALUE_DATA_FLAG_NONE_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_STATUS                  eStatus         = MOS_STATUS_SUCCESS;

    //--------------------------------------------------
    MOS_OS_ASSERT(pValueData);
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);
    //--------------------------------------------------
    iDataFlag = pValueData->i32DataFlag;

    pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (nullptr == pUserFeature)
    {
        MOS_OS_NORMALMESSAGE("Cannot found the user feature key.");
        return MOS_STATUS_NULL_POINTER;
    }

    // Open the user feature
    // Assigned the pUserFeature to ufKey for future reading
    ufKey = pUserFeature;
    if((eStatus = MosUserFeatureOpen(
                       pUserFeature->Type,
                       pUserFeature->pcPath,
                       KEY_READ,
                       &ufKey,
                       ufInfo)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Failed to open user feature for reading eStatus:%d.", eStatus);
        eStatus = MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
        goto finish;
    }

    // Initialize Read Value
    if((eStatus = MosUserFeatureReadValueInit(pUserFeature->uiNumOfValues)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Failed to initialize user feature read value eStatus:%d.",eStatus);
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

#if !(_DEBUG || _RELEASE_INTERNAL)
    // For release build, don't read debug only keys, but return default directly
    if (pUserFeature->EffctiveRange == MOS_USER_FEATURE_EFFECT_DEBUGONLY)
    {
        eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
        goto finish;
    }
#endif

    // Read the Values from user feature
    switch(pUserFeature->ValueType)
    {
       case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
           eStatus = MosUserFeatureReadValueBinary(ufKey, pUserFeature);
           break;
       case MOS_USER_FEATURE_VALUE_TYPE_STRING:
           eStatus = MosUserFeatureReadValueString(ufKey, pUserFeature);
           break;
       case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
           eStatus = MosUserFeatureReadValueMultiString(ufKey, pUserFeature);
           break;
       default:
           eStatus = MosUserFeatureReadValuePrimitive(ufKey, pUserFeature);
           break;
    }

    if(eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_NORMALMESSAGE("Failed to read value from user feature eStatus:%d.", eStatus);
        eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }
finish:
    if ((pUserFeature != nullptr) &&
        ((eStatus == MOS_STATUS_SUCCESS) ||
        (iDataFlag != MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE)))
    {
        // Use the User Feature Value or default value in corresponding user feature key Desc Fields
        // when User Feature Key read successfully or no input custom default value
        MosCopyUserFeatureValueData(
            &pUserFeature->Value,
            pValueData,
            pUserFeature->ValueType);
    }
    MosUserFeatureCloseKey(ufKey);  // Closes the key if not nullptr
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_CONTEXT_HANDLE              mosCtx)
{
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo = Mos_GetDeviceUfPathInfo((PMOS_CONTEXT)mosCtx);
    return MosUserFeatureReadValueFromMapID(
        ValueID,
        pValueData,
        ufInfo);
}

MOS_STATUS MosUtilities::MosUserFeatureReadValueID(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    uint32_t                        ValueID,
    PMOS_USER_FEATURE_VALUE_DATA    pValueData,
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo)
{
    return MosUserFeatureReadValueFromMapID(
        ValueID,
        pValueData,
        ufInfo);
}

const char* MosUtilities::MosUserFeatureLookupValueName(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pValueName;
    }
    else
    {
        return nullptr;
    }
}

const char *MosUtilities::MosUserFeatureLookupReadPath(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pcPath;
    }
    else
    {
        return nullptr;
    }
}

const char *MosUtilities::MosUserFeatureLookupWritePath(uint32_t ValueID)
{
    MOS_OS_ASSERT(ValueID != __MOS_USER_FEATURE_KEY_INVALID_ID);

    PMOS_USER_FEATURE_VALUE pUserFeature = MosUtilUserInterface::GetValue(ValueID);
    if (pUserFeature)
    {
        return pUserFeature->pcWritePath;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesTblID(
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues,
    MOS_USER_FEATURE_KEY_PATH_INFO          *ufInfo)
{
    uint32_t                            ui;
    PMOS_USER_FEATURE_VALUE             pFeatureValue      = nullptr;
    void                                *UFKey              = nullptr;
    PMOS_USER_FEATURE_VALUE_WRITE_DATA  pUserWriteData     = nullptr;
    PMOS_USER_FEATURE_VALUE             pUserFeature       = nullptr;
    uint32_t                            ValueID            = __MOS_USER_FEATURE_KEY_INVALID_ID;
    MOS_STATUS                          eStatus            = MOS_STATUS_SUCCESS;
    char                                WritePathWithPID[MAX_PATH];
    int32_t                             pid;
    uint64_t                            ulTraceData         = 0;
    bool                                isValid             = false;

    //--------------------------------------------------
    MOS_OS_CHK_NULL_RETURN(pWriteValues);
    //--------------------------------------------------

    MosZeroMemory(WritePathWithPID, MAX_PATH);

    pid = MosGetPid();

    for (ui = 0; ui < uiNumOfValues; ui++)
    {
        ValueID = pWriteValues[ui].ValueID;

        pUserFeature = MosUtilUserInterface::GetValue(ValueID);

        MOS_OS_CHK_NULL_RETURN(pUserFeature);
        // Open the user feature
        // Assigned the pUserFeature to UFKey for future reading
        UFKey = pUserFeature;

        //append write path with pid
        sprintf_s(WritePathWithPID, MAX_PATH, "%s\\%d", pUserFeature->pcWritePath, pid);

        // Trace data in case opening user feature for write fails
        switch (pUserFeature->ValueType)
        {
            case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
            case MOS_USER_FEATURE_VALUE_TYPE_INT32:
            case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
            case MOS_USER_FEATURE_VALUE_TYPE_FLOAT:
                ulTraceData = pWriteValues[ui].Value.u32Data;
                isValid = true;
                break;
            case MOS_USER_FEATURE_VALUE_TYPE_INT64:
            case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
                ulTraceData = pWriteValues[ui].Value.u64Data;
                isValid = true;
                break;
            default:
                MOS_OS_NORMALMESSAGE("Unknown value type %d", pUserFeature->ValueType);
        }

        if (isValid)
        {
            MosTraceDataDictionary(pUserFeature->pValueName, &ulTraceData, sizeof(ulTraceData));
        }

        //try to open Write path with pid first
        if ((eStatus = MosUserFeatureOpen(
                 pUserFeature->Type,
                 WritePathWithPID,
                 KEY_WRITE,
                 &UFKey,
                 ufInfo)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to open user feature for concurrency.");
            if ((eStatus = MosUserFeatureOpen(
                     pUserFeature->Type,
                     pUserFeature->pcWritePath,
                     KEY_WRITE,
                     &UFKey,
                     ufInfo)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_NORMALMESSAGE("Failed to open user feature for writing.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
                goto finish;
            }
        }

        //------------------------------------
        MOS_OS_ASSERT(pUserFeature->ValueType != MOS_USER_FEATURE_VALUE_TYPE_INVALID);
        //------------------------------------
        switch(pUserFeature->ValueType)
        {
        case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
            if ((eStatus = MosUserFeatureWriteValueBinary(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write binary value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_STRING:
            if ((eStatus = MosUserFeatureWriteValueString(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write string value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_MULTI_STRING:
            if ((eStatus = MosUserFeatureWriteValueMultiString(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write multi string value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
            break;
        default:
            if ((eStatus = MosUserFeatureWriteValuePrimitive(UFKey, pUserFeature, &(pWriteValues[ui].Value))) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("Failed to write primitive data value to user feature.");
                eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
                goto finish;
            }
        }
        MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    }

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MosUserFeatureCloseKey(UFKey);      // Closes the key if not nullptr
    }
    return eStatus;
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues,
    MOS_CONTEXT_HANDLE                      mosCtx)
{
    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo = Mos_GetDeviceUfPathInfo((PMOS_CONTEXT)mosCtx);

    return MosUserFeatureWriteValuesTblID(
        pWriteValues,
        uiNumOfValues,
        ufInfo);
}

MOS_STATUS MosUtilities::MosUserFeatureWriteValuesID(
    PMOS_USER_FEATURE_INTERFACE        pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA pWriteValues,
    uint32_t                           uiNumOfValues,
    MOS_USER_FEATURE_KEY_PATH_INFO *   ufInfo)
{
    return MosUserFeatureWriteValuesTblID(
        pWriteValues,
        uiNumOfValues,
        ufInfo);
}

MOS_STATUS MosUtilities::MosUserFeatureEnableNotification(
    PMOS_USER_FEATURE_INTERFACE            pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA          pNotification,
    MOS_CONTEXT_HANDLE                     mosCtx)
{
    PMOS_USER_FEATURE_NOTIFY_DATA_COMMON    pNotifyCommon;
    int32_t                                 bResult;
    MOS_STATUS                              eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);

    //---------------------------------------
    MOS_OS_ASSERT(pNotification);
    MOS_OS_ASSERT(pNotification->NotifyType != MOS_USER_FEATURE_NOTIFY_TYPE_INVALID);
    MOS_OS_ASSERT(pNotification->pPath);
    //---------------------------------------

    MOS_USER_FEATURE_KEY_PATH_INFO *ufInfo = Mos_GetDeviceUfPathInfo((PMOS_CONTEXT)mosCtx);

    // Reset the triggered flag
    pNotification->bTriggered = false;

    if (pNotification->pHandle == nullptr)
    {
        // Allocate private data as well
        pNotification->pHandle = MOS_AllocAndZeroMemory(sizeof(MOS_USER_FEATURE_NOTIFY_DATA));
        if(pNotification->pHandle == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NO_SPACE;
        }
    }
    pNotifyCommon = (PMOS_USER_FEATURE_NOTIFY_DATA_COMMON)pNotification->pHandle;

    // Open User Feature for Reading
    if (pNotifyCommon->UFKey == 0)
    {
        if((eStatus = MosUserFeatureOpen(
                          pNotification->Type,
                          pNotification->pPath,
                          KEY_READ,
                          &pNotifyCommon->UFKey,
                          ufInfo)) != MOS_STATUS_SUCCESS)
        {
            MOS_OS_ASSERTMESSAGE("Failed to open user feature for reading.");
            return MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED;
        }
    }

    // Create Event for notification
    if (pNotifyCommon->hEvent == nullptr)
    {
        pNotifyCommon->hEvent = MosCreateEventEx(
                                    nullptr,
                                    nullptr,
                                    0);
        if(pNotifyCommon->hEvent == nullptr)
        {
            MOS_OS_ASSERTMESSAGE("Failed to allocate memory.");
            return MOS_STATUS_NO_SPACE;
        }
    }

    // Unregister wait event if already registered
    if (pNotifyCommon->hWaitEvent)
    {
        if ((bResult = MosUnregisterWaitEx(pNotifyCommon->hWaitEvent)) == false)
        {
            MOS_OS_ASSERTMESSAGE("Unable to unregiser wait event.");
            return MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED;
        }
        pNotifyCommon->hWaitEvent = nullptr;
    }

    // Register a Callback
    if((eStatus = MosUserFeatureNotifyChangeKeyValue(
                      pNotifyCommon->UFKey,
                      false,
                      pNotifyCommon->hEvent,
                      true)) != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Unable to setup user feature key notification.");
        return MOS_STATUS_UNKNOWN;
    }

    // Create a wait object
    if ((bResult = MosUserFeatureWaitForSingleObject(
                                              &pNotifyCommon->hWaitEvent,
                                              pNotifyCommon->hEvent,
                                              (void *)MosUserFeatureCallback,
                                              pNotification)) == false)
    {
        MOS_OS_ASSERTMESSAGE("Failed to create a wait object.");
        return MOS_STATUS_EVENT_WAIT_REGISTER_FAILED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUtilities::MosUserFeatureDisableNotification(
    PMOS_USER_FEATURE_INTERFACE            pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA          pNotification)
{
    PMOS_USER_FEATURE_NOTIFY_DATA_COMMON    pNotifyDataCommon;
    int32_t                                 bResult;
    MOS_STATUS                              eStatus;
    MOS_UNUSED(pOsUserFeatureInterface);

    //---------------------------------------
    MOS_OS_ASSERT(pNotification);
    //---------------------------------------

    if (pNotification->pHandle)
    {
        pNotifyDataCommon = (PMOS_USER_FEATURE_NOTIFY_DATA_COMMON)
            pNotification->pHandle;

        if (pNotifyDataCommon->hWaitEvent)
        {
            if ((bResult = MosUnregisterWaitEx(pNotifyDataCommon->hWaitEvent)) == false)
            {
                MOS_OS_ASSERTMESSAGE("Unable to unregiser wait event.");
                    return MOS_STATUS_EVENT_WAIT_UNREGISTER_FAILED;
            }
        }
        if (pNotifyDataCommon->UFKey)
        {
            if ((eStatus = MosUserFeatureCloseKey(pNotifyDataCommon->UFKey)) != MOS_STATUS_SUCCESS)
            {
                MOS_OS_ASSERTMESSAGE("User feature key close failed.");
                return eStatus;
            }
        }
        if (pNotifyDataCommon->hEvent)
        {
            MosCloseHandle(pNotifyDataCommon->hEvent);
        }

        // Free Notify Data Memory
        MOS_FreeMemory(pNotifyDataCommon);
        pNotification->pHandle = nullptr;
    }
    return MOS_STATUS_SUCCESS;
}

float MosUtilities::MosSinc(float x)
{
    return (MOS_ABS(x) < 1e-9f) ? 1.0F : (float)(sin(x) / x);
}

float MosUtilities::MosLanczos(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = dwNumEntries >> 1;
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (MOS_ABS(x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= MOS_PI;

    return MosSinc(x) * MosSinc(x / fLanczosT);
}

float MosUtilities::MosLanczosG(float x, uint32_t dwNumEntries, float fLanczosT)
{
    uint32_t dwNumHalfEntries;

    dwNumHalfEntries = (dwNumEntries >> 1) + (dwNumEntries & 1);
    if (fLanczosT < dwNumHalfEntries)
    {
        fLanczosT = (float)dwNumHalfEntries;
    }

    if (x > (dwNumEntries >> 1) || (- x) >= dwNumHalfEntries)
    {
        return 0.0;
    }

    x *= MOS_PI;

    return MosSinc(x) * MosSinc(x / fLanczosT);
}

uint32_t MosUtilities::MosGCD(uint32_t a, uint32_t b)
{
    if (b == 0)
    {
        return a;
    }
    else
    {
        return MosGCD(b, a % b);
    }
}

#ifdef _MOS_UTILITY_EXT
#include "mos_utilities_ext_next.h"
#else
#define Mos_SwizzleOffset MosUtilities::MosSwizzleOffset
#endif

__inline int32_t MosUtilities::MosSwizzleOffset(
    int32_t         OffsetX,
    int32_t         OffsetY,
    int32_t         Pitch,
    MOS_TILE_TYPE   TileFormat,
    int32_t         CsxSwizzle,
    int32_t         ExtFlags)
{
    // When dealing with a tiled surface, logical linear accesses to the
    // surface (y * pitch + x) must be translated into appropriate tile-
    // formated accesses--This is done by swizzling (rearranging/translating)
    // the given access address--though it is important to note that the
    // swizzling is actually done on the accessing OFFSET into a TILED
    // REGION--not on the absolute address itself.

    // (!) Y-MAJOR TILING, REINTERPRETATION: For our purposes here, Y-Major
    // tiling will be thought of in a different way, we will deal with
    // the 16-byte-wide columns individually--i.e., we will treat a single
    // Y-Major tile as 8 separate, thinner tiles--Doing so allows us to
    // deal with both X- and Y-Major tile formats in the same "X-Major"
    // way--just with different dimensions: either 512B x 8 rows, or
    // 16B x 32 rows, respectively.

    // A linear offset into a surface is of the form
    //     y * pitch + x   =   y:x (Shorthand, meaning: y * (x's per y) + x)
    //
    // To treat a surface as being composed of tiles (though still being
    // linear), just as a linear offset has a y:x composition--its y and x
    // components can be thought of as having Row:Line and Column:X
    // compositions, respectively, where Row specifies a row of tiles, Line
    // specifies a row of pixels within a tile, Column specifies a column
    // of tiles, and X in this context refers to a byte within a Line--i.e.,
    //     offset = y:x
    //     y = Row:Line
    //     x = Col:X
    //     offset = y:x = Row:Line:Col:X

    // Given the Row:Line:Col:X composition of a linear offset, all that
    // tile swizzling does is swap the Line and Col components--i.e.,
    //     Linear Offset:   Row:Line:Col:X
    //     Swizzled Offset: Row:Col:Line:X
    // And with our reinterpretation of the Y-Major tiling format, we can now
    // describe both the X- and Y-Major tiling formats in two simple terms:
    // (1) The bit-depth of their Lines component--LBits, and (2) the
    // swizzled bit-position of the Lines component (after it swaps with the
    // Col component)--LPos.

    int32_t Row, Line, Col, x; // Linear Offset Components
    int32_t LBits, LPos; // Size and swizzled position of the Line component.
    int32_t SwizzledOffset;
    if (TileFormat == MOS_TILE_LINEAR)
    {
        return(OffsetY * Pitch + OffsetX);
    }

    if (TileFormat == MOS_TILE_Y)
    {
        LBits = 5; // Log2(TileY.Height = 32)
        LPos = 4;  // Log2(TileY.PseudoWidth = 16)
    }
    else //if (TileFormat == MOS_TILE_X)
    {
        LBits = 3; // Log2(TileX.Height = 8)
        LPos = 9;  // Log2(TileX.Width = 512)
    }

    Row = OffsetY >> LBits;               // OffsetY / LinesPerTile
    Line = OffsetY & ((1 << LBits) - 1);   // OffsetY % LinesPerTile
    Col = OffsetX >> LPos;                // OffsetX / BytesPerLine
    x = OffsetX & ((1 << LPos) - 1);    // OffsetX % BytesPerLine

    SwizzledOffset =
        (((((Row * (Pitch >> LPos)) + Col) << LBits) + Line) << LPos) + x;
    //                V                V                 V
    //                / BytesPerLine   * LinesPerTile    * BytesPerLine

    /// Channel Select XOR Swizzling ///////////////////////////////////////////
    if (CsxSwizzle)
    {
        if (TileFormat == MOS_TILE_Y) // A6 = A6 ^ A9
        {
            SwizzledOffset ^= ((SwizzledOffset >> (9 - 6)) & 0x40);
        }
        else //if (TileFormat == VPHAL_TILE_X) // A6 = A6 ^ A9 ^ A10
        {
            SwizzledOffset ^= (((SwizzledOffset >> (9 - 6)) ^ (SwizzledOffset >> (10 - 6))) & 0x40);
        }
    }

    return(SwizzledOffset);
}

void MosUtilities::MosSwizzleData(
    uint8_t         *pSrc,
    uint8_t         *pDst,
    MOS_TILE_TYPE   SrcTiling,
    MOS_TILE_TYPE   DstTiling,
    int32_t         iHeight,
    int32_t         iPitch,
    int32_t         extFlags)
{

#define IS_TILED(_a)                ((_a) != MOS_TILE_LINEAR)
#define IS_TILED_TO_LINEAR(_a, _b)  (IS_TILED(_a) && !IS_TILED(_b))
#define IS_LINEAR_TO_TILED(_a, _b)  (!IS_TILED(_a) && IS_TILED(_b))

    int32_t LinearOffset;
    int32_t TileOffset;
    int32_t x;
    int32_t y;

    // Translate from one format to another
    for (y = 0, LinearOffset = 0, TileOffset = 0; y < iHeight; y++)
    {
        for (x = 0; x < iPitch; x++, LinearOffset++)
        {
            // x or y --> linear
            if (IS_TILED_TO_LINEAR(SrcTiling, DstTiling))
            {
                TileOffset = Mos_SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    SrcTiling,
                    false,
                    extFlags);

                *(pDst + LinearOffset) = *(pSrc + TileOffset);
            }
            // linear --> x or y
            else if (IS_LINEAR_TO_TILED(SrcTiling, DstTiling))
            {
                TileOffset = Mos_SwizzleOffset(
                    x,
                    y,
                    iPitch,
                    DstTiling,
                    false,
                    extFlags);

                *(pDst + TileOffset) = *(pSrc + LinearOffset);
            }
            else
            {
                MOS_OS_ASSERT(0);
            }
        }
    }
}
