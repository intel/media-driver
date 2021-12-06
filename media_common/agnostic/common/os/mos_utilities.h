/*
* Copyright (c) 2009-2021, Intel Corporation
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
//! \file     mos_utilities.h
//! \brief    Common OS service across different platform
//! \details  Common OS service across different platform
//!
#ifndef __MOS_UTILITIES_H__
#define __MOS_UTILITIES_H__
#include "mos_utilities_common.h"
#include "mos_util_user_feature_keys.h"
#include "mos_resource_defs.h"
#include "mos_util_debug.h"
#include "mos_os_trace_event.h"

#ifdef __cplusplus
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <stdint.h>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <mutex>

#define MOS_MAX_PERF_FILENAME_LEN 260
#define EVENT_DECODE_BUFFER_KEYWORD 0x100000
#define EVENT_DECODE_DSTYUV_KEYWORD 0x200000
#define EVENT_DECODE_REFYUV_KEYWORD 0x400000

class PerfUtility
{
public:
    struct Tick
    {
        double freq;
        int64_t start;
        int64_t stop;
        double time;
    };
    struct PerfInfo
    {
        uint32_t count;
        double avg;
        double max;
        double min;
    };

public:
    static PerfUtility *getInstance();
    ~PerfUtility();
    PerfUtility();
    void startTick(std::string tag);
    void stopTick(std::string tag);
    void savePerfData();
    void setupFilePath(char *perfFilePath);
    void setupFilePath();
    bool bPerfUtilityKey;
    char sSummaryFileName[MOS_MAX_PERF_FILENAME_LEN + 1] = {'\0'};
    char sDetailsFileName[MOS_MAX_PERF_FILENAME_LEN + 1] = {'\0'};
    int32_t dwPerfUtilityIsEnabled;

private:
    void printPerfSummary();
    void printPerfDetails();
    void printHeader(std::ofstream& fout);
    void printBody(std::ofstream& fout);
    void printFooter(std::ofstream& fout);
    std::string formatPerfData(std::string tag, std::vector<Tick>& record);
    void getPerfInfo(std::vector<Tick>& record, PerfInfo* info);
    std::string getDashString(uint32_t num);

private:
    static std::shared_ptr<PerfUtility> instance;
    static std::mutex perfMutex;
    std::map<std::string, std::vector<Tick>*> records;
};

//!
//! \brief    Get Current time
//! \details  Get Current time in us
//! \return   uint64_t
//!           Returns time in us
//!
uint64_t MOS_GetCurTime();

#endif // __cplusplus


class MosUtilities;

//! Helper Macros for MEMNINJA debug messages
#define MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line)                                                                                 \
    MOS_OS_MEMNINJAMESSAGE(                                                                                                                                 \
        "MemNinjaSysAlloc: Time = %f, MemNinjaCounter = %d, memPtr = %p, size = %d, functionName = \"%s\", "                                                \
        "filename = \"%s\", line = %d/", MosUtilities::MosGetTime(), MosUtilities::m_mosMemAllocCounter, ptr, size, functionName, filename, line);          \


#define MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line)                                                                                        \
    MOS_OS_MEMNINJAMESSAGE(                                                                                                                                 \
        "MemNinjaSysFree: Time = %f, MemNinjaCounter = %d, memPtr = %p, functionName = \"%s\", "                                                            \
        "filename = \"%s\", line = %d/", MosUtilities::MosGetTime(), MosUtilities::m_mosMemAllocCounter, ptr, functionName, filename, line);                \


#define MOS_MEMNINJA_GFX_ALLOC_MESSAGE(ptr, bufName, component, size, arraySize, functionName, filename, line)                                              \
    MOS_OS_MEMNINJAMESSAGE(                                                                                                                                 \
        "MemNinjaGfxAlloc: Time = %f, MemNinjaCounterGfx = %d, memPtr = %p, bufName = %s, component = %d, size = 0x%llx, "                                  \
        "arraySize = %d, functionName = \"%s\", filename = \"%s\", line = %d/", MosUtilities::MosGetTime(), MosUtilities::m_mosMemAllocCounterGfx, ptr,     \
        bufName, component, size, arraySize, functionName, filename, line);                                                                                 \

#define MOS_MEMNINJA_GFX_FREE_MESSAGE(ptr, functionName, filename, line)                                                                                    \
    MOS_OS_MEMNINJAMESSAGE(                                                                                                                                 \
        "MemNinjaGfxFree: Time = %f, MemNinjaCounterGfx = %d, memPtr = %p, functionName = \"%s\", "                                                         \
        "filename = \"%s\", line = %d/", MosUtilities::MosGetTime(), MosUtilities::m_mosMemAllocCounterGfx, ptr, functionName, filename, line);             \


#include "mos_utilities_next.h"

#ifdef __cplusplus

//template<class _Ty, class... _Types> inline
//std::shared_ptr<_Ty> MOS_MakeShared(_Types&&... _Args)
//{
//    try
//    {
//        return std::make_shared<_Ty>(std::forward<_Types>(_Args)...);
//    }
//    catch (const std::bad_alloc&)
//    {
//        return nullptr;
//    }
//}

#if MOS_MESSAGES_ENABLED
template<class _Ty, class... _Types>
_Ty* MOS_NewUtil(const char *functionName,
    const char *filename,
    int32_t line, _Types&&... _Args)
#else
template<class _Ty, class... _Types>
_Ty* MOS_NewUtil(_Types&&... _Args)
#endif
{
#if MOS_MESSAGES_ENABLED
    return MosUtilities::MosNewUtil<_Ty, _Types...>(functionName, filename, line, std::forward<_Types>(_Args)...);
#else
    return MosUtilities::MosNewUtil<_Ty, _Types...>(std::forward<_Types>(_Args)...);
#endif
}

#if MOS_MESSAGES_ENABLED
template<class _Ty, class... _Types>
_Ty *MOS_NewArrayUtil(const char *functionName,
    const char *filename,
    int32_t line, int32_t numElements)
#else
template<class _Ty, class... _Types>
_Ty* MOS_NewArrayUtil(int32_t numElements)
#endif
{
#if MOS_MESSAGES_ENABLED
    return MosUtilities::MosNewArrayUtil<_Ty>(functionName, filename, line, numElements);
#else
    return MosUtilities::MosNewArrayUtil<_Ty>(numElements);
#endif
}

#if MOS_MESSAGES_ENABLED
#define MOS_NewArray(classType, numElements) MOS_NewArrayUtil<classType>(__FUNCTION__, __FILE__, __LINE__, numElements)
#define MOS_New(classType, ...) MOS_NewUtil<classType>(__FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define MOS_NewArray(classType, numElements) MOS_NewArrayUtil<classType>(numElements)
#define MOS_New(classType, ...) MOS_NewUtil<classType>(__VA_ARGS__)
#endif

#if MOS_MESSAGES_ENABLED
    #define MOS_DeleteUtil(functionName, filename, line, ptr) \
        if (ptr != nullptr) \
            { \
                MosUtilities::MosAtomicDecrement(&MosUtilities::m_mosMemAllocCounter); \
                MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line); \
                delete(ptr); \
                ptr = nullptr; \
            }
#else
    #define MOS_DeleteUtil(ptr) \
        if (ptr != nullptr) \
            { \
                MosUtilities::MosAtomicDecrement(&MosUtilities::m_mosMemAllocCounter); \
                MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line); \
                delete(ptr); \
                ptr = nullptr; \
            }
#endif

#if MOS_MESSAGES_ENABLED
    #define MOS_DeleteArrayUtil(functionName, filename, line, ptr) \
        if (ptr != nullptr) \
        { \
            MosUtilities::MosAtomicDecrement(&MosUtilities::m_mosMemAllocCounter); \
            MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line); \
            delete[](ptr); \
            ptr = nullptr; \
        }
#else
    #define MOS_DeleteArrayUtil(ptr) \
        if (ptr != nullptr) \
        { \
            MosUtilities::MosAtomicDecrement(&MosUtilities::m_mosMemAllocCounter); \
            MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line); \
            delete[](ptr); \
            ptr = nullptr; \
        }
#endif

#if MOS_MESSAGES_ENABLED
#define MOS_DeleteArray(ptr) MOS_DeleteArrayUtil(__FUNCTION__, __FILE__, __LINE__, ptr)
#define MOS_Delete(ptr) MOS_DeleteUtil(__FUNCTION__, __FILE__, __LINE__, ptr)
#else
#define MOS_DeleteArray(ptr) MOS_DeleteArrayUtil(ptr)
#define MOS_Delete(ptr) MOS_DeleteUtil(ptr)
#endif

#endif

//------------------------------------------------------------------------------
//  Allocate, free and set a memory region
//------------------------------------------------------------------------------

#if MOS_MESSAGES_ENABLED

#define MOS_AlignedAllocMemory(size, alignment) \
   MosUtilities::MosAlignedAllocMemoryUtils(size, alignment, __FUNCTION__, __FILE__, __LINE__)
#define MOS_AlignedFreeMemory(ptr) \
    MosUtilities::MosAlignedFreeMemoryUtils(ptr, __FUNCTION__, __FILE__, __LINE__)

#define MOS_AllocMemory(size) \
    MosUtilities::MosAllocMemoryUtils(size, __FUNCTION__, __FILE__, __LINE__)
#define MOS_FreeMemory(ptr) \
    MosUtilities::MosFreeMemoryUtils(ptr, __FUNCTION__, __FILE__, __LINE__)

#define MOS_AllocAndZeroMemory(size) \
    MosUtilities::MosAllocAndZeroMemoryUtils(size, __FUNCTION__, __FILE__, __LINE__)

#define MOS_ReallocMemory(ptr, newSize) \
    MosUtilities::MosReallocMemoryUtils(ptr, newSize, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

#define MOS_AlignedAllocMemory(size, alignment) \
   MosUtilities::MosAlignedAllocMemory(size, alignment)
#define MOS_AlignedFreeMemory(ptr) \
    MosUtilities::MosAlignedFreeMemory(ptr)

#define MOS_AllocMemory(size) \
    MosUtilities::MosAllocMemory(size)
#define MOS_FreeMemory(ptr) \
    MosUtilities::MosFreeMemory(ptr)

#define MOS_AllocAndZeroMemory(size) \
    MosUtilities::MosAllocAndZeroMemory(size)

#define MOS_ReallocMemory(ptr, newSize) \
    MosUtilities::MosReallocMemory(ptr, newSize)

#endif // MOS_MESSAGES_ENABLED

#define MOS_FreeMemAndSetNull(ptr)                      \
do{                                                     \
    MOS_FreeMemory(ptr);                                \
    ptr = nullptr;                                      \
} while (0)

#define MOS_SafeFreeMemory(ptr)                         \
    if (ptr) MOS_FreeMemory(ptr);                       \

#define MOS_ZeroMemory(pDestination, stLength)          \
    MosUtilities::MosZeroMemory(pDestination, stLength)

#define MOS_FillMemory(pDestination, stLength, bFill)   \
    MosUtilities::MosFillMemory(pDestination, stLength, bFill)

//------------------------------------------------------------------------------
//  User Settings
//------------------------------------------------------------------------------
//ptr could be moscontext or user feature key info
#define MOS_UserFeature_ReadValue_ID(pOsUserFeatureInterface, valueID, pValueData, ptr)                 \
    MosUtilities::MosUserFeatureReadValueID(pOsUserFeatureInterface, valueID, pValueData, ptr)

//ptr could be moscontext or user feature key info
#define MOS_UserFeature_WriteValues_ID(pOsUserFeatureInterface, pWriteValues, uiNumOfValues, ptr)        \
    MosUtilities::MosUserFeatureWriteValuesID(pOsUserFeatureInterface, pWriteValues, uiNumOfValues, ptr)

// User Feature Report Writeout
#define WriteUserFeature64(key, value, mosCtx)                                                       \
{                                                                                                    \
    MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__; \
    UserFeatureWriteData.Value.i64Data                     = (value);                                \
    UserFeatureWriteData.ValueID                           = (key);                                  \
    MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData, 1, mosCtx);            \
}

#define WriteUserFeature(key, value, mosCtx)                                                         \
{                                                                                                    \
    MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__; \
    UserFeatureWriteData.Value.i32Data                     = (value);                                \
    UserFeatureWriteData.ValueID                           = (key);                                  \
    MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData, 1, mosCtx);            \
}

#define WriteUserFeatureString(key, value, len, mosCtx)                                              \
{                                                                                                    \
    MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__; \
    UserFeatureWriteData.Value.StringData.pStringData      = (value);                                \
    UserFeatureWriteData.Value.StringData.uSize            = (len + 1);                              \
    UserFeatureWriteData.ValueID                           = (key);                                  \
    MosUtilities::MosUserFeatureWriteValuesID(nullptr, &UserFeatureWriteData, 1, mosCtx);            \
}

//------------------------------------------------------------------------------
//  string
//------------------------------------------------------------------------------
#define MOS_SecureStrcat(strDestination, numberOfElements, strSource)                               \
    MosUtilities::MosSecureStrcat(strDestination, numberOfElements, strSource)

#define MOS_SecureStrcpy(strDestination, numberOfElements, strSource)                               \
    MosUtilities::MosSecureStrcpy(strDestination, numberOfElements, strSource)

#define MOS_SecureMemcpy(pDestination, dstLength, pSource, srcLength)                               \
    MosUtilities::MosSecureMemcpy(pDestination, dstLength, pSource, srcLength)

#define MOS_SecureStringPrint(buffer, bufSize, length, format, ...)                                 \
    MosUtilities::MosSecureStringPrint(buffer, bufSize, length, format, ##__VA_ARGS__)


#ifdef __cplusplus
extern "C" {
#endif


//------------------------------------------------------------------------------
//  File I/O Functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// User Feature Functions
//------------------------------------------------------------------------------
//!
//! \brief    Read Single Value from User Feature
//! \details  This is a unified funtion to read user feature key for all components.
//!           (Codec/VP/CP/CM)
//!           It is required to prepare all memories for buffers before calling this function.
//!           User can choose to use array variable or allocated memory for the buffer.
//!           If the buffer is allocated dynamically, it must be freed by user to avoid memory leak.
//!           ------------------------------------------------------------------------------------
//!           Usage example:
//!           a) Initiation:
//!           MOS_ZeroMemory(&UserFeatureValue, sizeof(UserFeatureValue));
//!           UserFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
//!           UserFeature.pPath           = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
//!           UserFeature.pValues         = &UserFeatureValue;
//!           UserFeature.uiNumValues     = 1;
//!           b.1) For uint32_t type:
//!           UserFeatureValue.u32Data = 1;    //set the default value, must be initiated with one valid value.
//!           b.2) For String/Binary type:
//!           char cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           UserFeatureValue.StringData.pStringData = cStringData; // make sure the pointer is valid
//!           UserFeatureValue.StringData.uMaxSize    = MOS_USER_CONTROL_MAX_DATA_SIZE;
//!           UserFeatureValue.StringData.uSize       = 0;  //set the default value. 0 is empty buffer.
//!           b.3) For MultiString type:
//!           char cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           MOS_USER_FEATURE_VALUE_STRING strings[MAX_STRING_COUNT];
//!           UserFeatureValue.MultiStringData.pMultStringData = cStringData; // make sure the pointer is valid
//!           UserFeatureValue.MultiStringData.uMaxSize        = MOS_USER_CONTROL_MAX_DATA_SIZE;
//!           UserFeatureValue.MultiStringData.uSize           = 0;  //set the default value. 0 is empty buffer.
//!           UserFeatureValue.MultiStringData.pStrings        = strings; // make sure the pointer is valid
//!           UserFeatureValue.MultiStringData.uCount          = MAX_STRING_COUNT;
//!           c) Read user feature key:
//!           MOS_UserFeature_ReadValue();
//!           -------------------------------------------------------------------------------------
//!           Important note: The pointer pStringData/pMultStringData may be modified if the
//!           previous MOS_UserFeature_ReadValue() doesn't read a same user feature key type. So it's
//!           suggested to set the union members in UserFeatureValue every time before
//!           MOS_UserFeature_ReadValue() if you are not familiar with the details of this function.
//! \param    PMOS_USER_FEATURE_INTERFACE pOsUserFeatureInterface
//!           [in] Pointer to OS User Interface structure
//! \param    PMOS_USER_FEATURE pUserFeature
//!           [in/out] Pointer to User Feature Interface
//! \param    char  *pValueName
//!           [in] Pointer to the name of the user feature key value
//! \param    MOS_USER_FEATURE_VALUE_TYPE ValueType
//!           [in] User Feature Value type
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief    Generate a User Feature Keys XML file according to user feature keys table in MOS
//! \details  Generate a User Feature Keys XML files according to MOSUserFeatureDescFields
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_FUNC_EXPORT MOS_STATUS MOS_EXPORT_DECL DumpUserFeatureKeyDefinitionsMedia();

#endif

#define  Mos_SwizzleData(pSrc, pDst, SrcTiling, DstTiling, iHeight, iPitch, extFlags)   \
    MosUtilities::MosSwizzleData(pSrc, pDst, SrcTiling, DstTiling, iHeight, iPitch, extFlags)

//!
//! \brief    MOS trace event initialize
//! \details  register provide Global ID to the system. 
//! \param    void
//! \return   void
//!
void MOS_TraceEventInit();

//!
//! \brief    MOS trace event close
//! \details  un-register provider Global ID. 
//! \param    void
//! \return   void
//!
void MOS_TraceEventClose();

//!
//! \brief    setup static platform info for trace events
//! \details  send static platform info to trace struct, which itself determine when to send them.
//!           static platform info should only send 1 time per trace capture, no more no less.
//! \param    [in] driver version
//! \param    [in] platform family
//! \param    [in] render family
//! \param    [in] device id
//! \return   void
//!
void MOS_TraceSetupInfo(
    uint32_t DrvVer,
    uint32_t PlatFamily,
    uint32_t RenderFamily,
    uint32_t DeviceID);

uint64_t MOS_GetTraceEventKeyword();

//!
//! \brief    MOS log trace event
//! \details  log trace event by id and event type, arg1 and arg2 are optional arguments
//!           arguments are in raw data format, need match data structure in manifest.
//! \param    [in] usId
//!           Indicates event id
//! \param    [in] ucType
//!           Indicates event type
//! \param    [in] pArg1
//!           event data address
//! \param    [in] dwSize1
//!           event data size
//! \param    [in] pArg2
//!           event data address
//! \param    [in] dwSize2
//!           event data size
//! \return   void
//!
void MOS_TraceEvent(
    uint16_t         usId,
    uint8_t          ucType,
    const void       *pArg1,
    uint32_t         dwSize1,
    const void       *pArg2,
    uint32_t         dwSize2);

//!
//! \brief    MOS log trace dump
//! \details  dump driver data in raw format, data could be driver internal state or surface.
//! \param    [in] name
//!           Indicates dump name
//! \param    [in] flags
//!           additional buffer info flags
//! \param    [in] pBuf
//!           Indicates buffer address
//! \param    [in] size
//!           Indicates buffer size
//! \return   void
//!
void MOS_TraceDataDump(
    const char *pcName,
    uint32_t    flags,
    const void *pBuf,
    uint32_t    dwSize);

//!
//! \brief    MOS log data dictionary item
//! \details  Dictionary is name:value pair
//! \param    [in] name
//!           Indicates item name
//! \param    [in] pBuf
//!           Indicates value address
//! \param    [in] size
//!           Indicates value size
//! \return   void
//!
void MOS_TraceDataDictionary(
    const char* pcName,
    const void* pBuf,
    uint32_t    dwSize);

//!
//! \brief    MOS log trace event Msg
//! \details  log trace event msg w/ level/compID/functionname/lineNum arguments
//!           arguments are in raw data format, need match data structure in manifest.
//! \param    [in] level
//!           Indicates msg level
//! \param    [in] compID
//!           Indicates compID
//! \param    [in] message
//!           event msg
//! \param    [in] functionName
//!           func name
//! \param    [in] lineNum
//!           event line number
//! \return   void
//!
void MOS_TraceEventMsg(
    uint8_t          level,
    uint8_t          compID,
    void*            message,
    void*            functionName,
    uint32_t         lineNum);

//!
//! \brief    MOS gfx info initialize
//! \details  Load igdinfoXX.dll library and get gfx info function pointer
//! \param    void
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GfxInfoInit();

//!
//! \brief    MOS gfx info close
//! \details  Release igdinfoXX.dll library
//! \param    void
//! \return   void
//!
void MOS_GfxInfoClose();

//!
//! \brief    MOS_GfxInfo_RTErr
//! \details  Custom gfx info trace to report runtime errors detected by each component.
//! \param    [in] ver
//!           Version
//! \param    [in] compId
//!           Component ID defined in GFXINFO_COMP_ID
//! \param    [in] FtrId
//!           Feature ID, an unique identifier for each component.
//! \param    [in] ErrorCode
//!           Error code that will be recorded.
//! \param    [in] num_of_triples
//!           Number of triples (name, type, value) to be compose as an <I N='name'>value</I> XML element
//! \param    [in] ...
//!           Triples (name, type, value), for example
//!             int8_t i = 3;
//!             "Name1", GFXINFO_PTYPE_UINT8, &i
//!             "Name2", GFXINFO_PTYPE_ANSISTRING, "string value"
//! \return   void
//!
void MOS_GfxInfo_RTErr(uint8_t ver,
    uint16_t    compId,
    uint16_t    FtrId,
    uint32_t    ErrorCode,
    uint8_t     num_of_triples,
    ...);

//!
//! \brief    MOS_GfxInfo
//! \details  A helper function to help to compose gfx info xml string
//! \param    [in] ver
//!           Version
//! \param    [in] compId
//!           Component ID defined in GFXINFO_COMP_ID
//! \param    [in] tmtryID
//!           Gfx info ID, an unique identifier for each component.
//! \param    [in] num_of_triples
//!           Number of triples (name, type, value) to be compose as an <I N='name'>value</I> XML element
//! \param    [in] ...
//!           Triples (name, type, value), for example
//!             int8_t i = 3;
//!             "Name1", GFXINFO_PTYPE_UINT8, &i
//!             "Name2", GFXINFO_PTYPE_ANSISTRING, "string value"
//! \return   void
//!
void MOS_GfxInfo(
    uint8_t         ver,
    uint16_t        compId,
    uint32_t        tmtryID,
    uint8_t         num_of_triples,
    ...);

class MosMutex
{
public:
    MosMutex(void)
    {
        m_lock = MosUtilities::MosCreateMutex();
    }

    ~MosMutex()
    {
        MosUtilities::MosDestroyMutex(m_lock);
    }

    void Lock()
    {
        MosUtilities::MosLockMutex(m_lock);
    }

    void Unlock()
    {
        MosUtilities::MosUnlockMutex(m_lock);
    }

private:
    PMOS_MUTEX m_lock = nullptr;
};

#ifdef __cplusplus
}
#endif

#endif // __MOS_UTILITIES_H__
