/*
* Copyright (c) 2009-2018, Intel Corporation
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
#include "mos_defs.h"
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

#define MOS_MAX_PERF_FILENAME_LEN 260

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
    char sSummaryFileName[MOS_MAX_PERF_FILENAME_LEN + 1] = "";
    char sDetailsFileName[MOS_MAX_PERF_FILENAME_LEN + 1] = "";
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
    std::map<std::string, std::vector<Tick>*> records;
};

#endif // __cplusplus

//!
//! \brief    Get current run time
//! \details  Get current run time in us
//! \return   double
//!           Returns time in us
//!
double MOS_GetTime();

extern int32_t MosMemAllocCounter;
extern int32_t MosMemAllocFakeCounter;
extern int32_t MosMemAllocCounterGfx;
extern uint8_t MosUltFlag;
extern uint32_t g_apoMosEnabled;

class MosUtilities;

#define GetTime (g_apoMosEnabled ? (MosUtilities::MosGetTime()) : (MOS_GetTime()))

//! Helper Macros for MEMNINJA debug messages
#define MOS_MEMNINJA_ALLOC_MESSAGE(ptr, size, functionName, filename, line)                                                             \
    MOS_OS_VERBOSEMESSAGE(                                                                                                              \
        "MemNinjaSysAlloc: Time = %f, MemNinjaCounter = %d, memPtr = %p, size = %d, functionName = \"%s\", "                            \
        "filename = \"%s\", line = %d/", GetTime, MosMemAllocCounter, ptr, size, functionName, filename, line)

#define MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line)                                                                    \
    MOS_OS_VERBOSEMESSAGE(                                                                                                              \
       "MemNinjaSysFree: Time = %f, MemNinjaCounter = %d, memPtr = %p, functionName = \"%s\", "                                         \
       "filename = \"%s\", line = %d/", GetTime, MosMemAllocCounter, ptr, functionName, filename, line)

#define MOS_MEMNINJA_GFX_ALLOC_MESSAGE(ptr, bufName, component, size, arraySize, functionName, filename, line)                          \
    MOS_OS_VERBOSEMESSAGE(                                                                                                              \
        "MemNinjaGfxAlloc: Time = %f, MemNinjaCounterGfx = %d, memPtr = %p, bufName = %s, component = %d, size = %d, "                  \
        "arraySize = %d, functionName = \"%s\", filename = \"%s\", line = %d/", GetTime, MosMemAllocCounterGfx, ptr,\
        bufName, component, size, arraySize, functionName, filename, line)

#define MOS_MEMNINJA_GFX_FREE_MESSAGE(ptr, functionName, filename, line)                                                                \
    MOS_OS_VERBOSEMESSAGE(                                                                                                              \
        "MemNinjaGfxFree: Time = %f, MemNinjaCounterGfx = %d, memPtr = %p, functionName = \"%s\", "                                     \
        "filename = \"%s\", line = %d/", GetTime, MosMemAllocCounterGfx, ptr, functionName, filename, line)

#include "mos_utilities_next.h"

#ifdef __cplusplus

extern "C" int32_t MOS_AtomicIncrement(int32_t *pValue);   // forward declaration
extern "C" int32_t MOS_AtomicDecrement(int32_t *pValue);   // forward declaration

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

//Memory alloc fail simulatiion related defination
#if (_DEBUG || _RELEASE_INTERNAL)

bool MOS_SimulateAllocMemoryFail(
    size_t      size,
    size_t      alignment,
    const char *functionName,
    const char *filename,
    int32_t     line);
#endif  //(_DEBUG || _RELEASE_INTERNAL)

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
#if (_DEBUG || _RELEASE_INTERNAL)
        //Simulate allocate memory fail if flag turned on
        if (MOS_SimulateAllocMemoryFail(sizeof(_Ty), NO_ALLOC_ALIGNMENT, functionName, filename, line))
        {
            return nullptr;
        }
#endif
        _Ty* ptr = new (std::nothrow) _Ty(std::forward<_Types>(_Args)...);
        if (ptr != nullptr)
        {
            MOS_AtomicIncrement(&MosMemAllocCounter);
            MOS_MEMNINJA_ALLOC_MESSAGE(ptr, sizeof(_Ty), functionName, filename, line);
        }
        else
        {
            MOS_OS_ASSERTMESSAGE("Fail to create class.");
        }
        return ptr;
}

#if MOS_MESSAGES_ENABLED
template<class _Ty, class... _Types>
_Ty* MOS_NewArrayUtil(const char *functionName,
    const char *filename,
    int32_t line, int32_t numElements)
#else
template<class _Ty, class... _Types>
_Ty* MOS_NewArrayUtil(int32_t numElements)
#endif
{
#if (_DEBUG || _RELEASE_INTERNAL)
        //Simulate allocate memory fail if flag turned on
        if (MOS_SimulateAllocMemoryFail(sizeof(_Ty) * numElements, NO_ALLOC_ALIGNMENT, functionName, filename, line))
        {
            return nullptr;
        }
#endif
        _Ty* ptr = new (std::nothrow) _Ty[numElements]();
        if (ptr != nullptr)
        {
            MOS_AtomicIncrement(&MosMemAllocCounter);
            MOS_MEMNINJA_ALLOC_MESSAGE(ptr, numElements*sizeof(_Ty), functionName, filename, line);
        }
        return ptr;
}

#if MOS_MESSAGES_ENABLED
#define MOS_NewArray(classType, numElements) MOS_NewArrayUtil<classType>(__FUNCTION__, __FILE__, __LINE__, numElements)
#define MOS_New(classType, ...) MOS_NewUtil<classType>(__FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define MOS_NewArray(classType, numElements) MOS_NewArrayUtil<classType>(numElements)
#define MOS_New(classType, ...) MOS_NewUtil<classType>(__VA_ARGS__)
#endif

#if MOS_MESSAGES_ENABLED
template<class _Ty> inline
void MOS_DeleteUtil(
    const char *functionName,
    const char *filename,
    int32_t     line,
    _Ty&        ptr)
#else
template<class _Ty> inline
void MOS_DeleteUtil(_Ty& ptr)
#endif
{
    if (ptr != nullptr)
    {
        MOS_AtomicDecrement(&MosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);
        delete(ptr);
        ptr = nullptr;
    }
}

#if MOS_MESSAGES_ENABLED
template<class _Ty> inline
void MOS_DeleteArrayUtil(
    const char *functionName,
    const char *filename,
    int32_t     line,
    _Ty&        ptr)
#else
template <class _Ty> inline
void MOS_DeleteArrayUtil(_Ty& ptr)
#endif
{
    if (ptr != nullptr)
    {
        MOS_AtomicDecrement(&MosMemAllocCounter);
        MOS_MEMNINJA_FREE_MESSAGE(ptr, functionName, filename, line);

        delete[](ptr);
        ptr = nullptr;
    }
}

#if MOS_MESSAGES_ENABLED
#define MOS_DeleteArray(ptr) MOS_DeleteArrayUtil(__FUNCTION__, __FILE__, __LINE__, ptr)
#define MOS_Delete(ptr) MOS_DeleteUtil(__FUNCTION__, __FILE__, __LINE__, ptr)
#else
#define MOS_DeleteArray(ptr) MOS_DeleteArrayUtil(ptr)
#define MOS_Delete(ptr) MOS_DeleteUtil(ptr)
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \brief    Init Function for MOS utilities
//! \details  Initial MOS utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_utilities_init();

//!
//! \brief    Close Function for MOS utilities
//! \details  close/remove MOS utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_utilities_close();

//!
//! \brief    Init Function for MOS OS specific utilities
//! \details  Initial MOS OS specific utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_OS_Utilities_Init();

//!
//! \brief    Close Function for MOS OS utilities
//! \details  close/remove MOS OS utilities related structures, and only execute once for multiple entries
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_OS_Utilities_Close();

//------------------------------------------------------------------------------
//  Allocate, free and set a memory region
//------------------------------------------------------------------------------
//!
//! \brief    Allocates aligned memory and performs error checking
//! \details  Wrapper for aligned_malloc(). Performs error checking.
//!           It increases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] size
//!           Size of memorry to be allocated
//! \param    [in] alignment
//!           alignment
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void *MOS_AlignedAllocMemoryUtils(
    size_t     size,
    size_t     alignment,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_AlignedAllocMemory(size, alignment) \
   MOS_AlignedAllocMemoryUtils(size, alignment, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

void  *MOS_AlignedAllocMemory(
    size_t  size,
    size_t  alignment);

#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Wrapper for aligned_free(). Performs error checking.
//! \details  Wrapper for aligned_free() - Free a block of memory that was allocated by MOS_AlignedAllocMemory.
//!             Performs error checking.
//!           It decreases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MOS_AlignedFreeMemoryUtils(
    void        *ptr,
    const char  *functionName,
    const char  *filename,
    int32_t     line);

#define MOS_AlignedFreeMemory(ptr) \
    MOS_AlignedFreeMemoryUtils(ptr, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

void MOS_AlignedFreeMemory(void  *ptr);

#endif // MOS_MESSAGES_ENABLED
//!
//! \brief    Allocates memory and performs error checking
//! \details  Wrapper for malloc(). Performs error checking.
//!           It increases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] size
//!           Size of memorry to be allocated
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void  *MOS_AllocMemoryUtils(
    size_t     size,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_AllocMemory(size) \
    MOS_AllocMemoryUtils(size, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED

void  *MOS_AllocMemory(
    size_t  size);

#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Allocates and fills memory with 0
//! \details  Wrapper for malloc(). Performs error checking,
//!           and fills the allocated memory with 0.
//!           It increases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] size
//!           Size of memorry to be allocated
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void  *MOS_AllocAndZeroMemoryUtils(
    size_t     size,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_AllocAndZeroMemory(size) \
    MOS_AllocAndZeroMemoryUtils(size, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED
void  *MOS_AllocAndZeroMemory(
    size_t                   size);
#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Reallocate memory
//! \details  Wrapper for realloc(). Performs error checking.
//!           It modifies memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] ptr
//!           Pointer to be reallocated
//! \param    [in] new_size
//!           Size of memory to be allocated
//! \return   void *
//!           Pointer to allocated memory
//!
#if MOS_MESSAGES_ENABLED
void *MOS_ReallocMemoryUtils(
    void       *ptr,
    size_t     newSize,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_ReallocMemory(ptr, newSize) \
    MOS_ReallocMemoryUtils(ptr, newSize, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED
void *MOS_ReallocMemory(
    void       *ptr,
    size_t     newSize);
#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Wrapper for free(). Performs error checking.
//! \details  Wrapper for free(). Performs error checking.
//!           It decreases memory allocation counter variable
//!           MosMemAllocCounter for checking memory leaks.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#if MOS_MESSAGES_ENABLED
void MOS_FreeMemoryUtils(
    void       *ptr,
    const char *functionName,
    const char *filename,
    int32_t    line);

#define MOS_FreeMemory(ptr) \
    MOS_FreeMemoryUtils(ptr, __FUNCTION__, __FILE__, __LINE__)

#else // !MOS_MESSAGES_ENABLED
void MOS_FreeMemory(
    void            *ptr);
#endif // MOS_MESSAGES_ENABLED

//!
//! \brief    Wrapper for MOS_FreeMemory().
//! \details  Wrapper for MOS_FreeMemory().  Calls MOS_FreeMemory() and then sets ptr to NULL.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#define MOS_FreeMemAndSetNull(ptr)  \
do{                                 \
    MOS_FreeMemory(ptr);            \
    ptr = nullptr;                     \
} while (0)

//!
//! \brief    Wrapper for MOS_FreeMemory().
//! \details  Wrapper for MOS_FreeMemory().  Calls MOS_FreeMemory() if ptr is not NULL.
//!           This wrapper can be called when ptr can be nullptr and therefore we don't want to hit the assert inside MOS_FreeMemory().
//!           It should not be used if ptr is expected to be non-NULL. In that case, we want to hit the assert.
//! \param    [in] ptr
//!           Pointer to the memory to be freed
//! \return   void
//!
#define MOS_SafeFreeMemory(ptr)               \
    if (ptr) MOS_FreeMemory(ptr);             \

//!
//! \brief    Wrapper to set a block of memory with zeros.
//! \details  Wrapper to set a block of memory with zeros.
//! \param    [in] pDestination
//!           A pointer to the starting address of the memory
//!           block to fill with zeros.
//! \param    [in] stLength
//!           Size of the memory block in bytes to be filled
//! \return   void
//!
void MOS_ZeroMemory(
    void            *pDestination,
    size_t          stLength);

//!
//! \brief    Wrapper to set a block of memory with a specified value.
//! \details  Wrapper to set a block of memory with a specified value.
//! \param    [in] pDestination
//!           A pointer to the starting address of the memory
//!           block to fill with specified value bFill
//! \param    [in] stLength
//!           Size of the memory block in bytes to be filled
//! \param    [in] bFill
//!           The byte value with which to fill the memory block
//! \return   void
//!
void MOS_FillMemory(
    void            *pDestination,
    size_t          stLength,
    uint8_t         bFill);

//------------------------------------------------------------------------------
//  File I/O Functions
//------------------------------------------------------------------------------
//!
//! \brief    Allocate a buffer and read contents from a file into this buffer
//! \details  Allocate a buffer and read contents from a file into this buffer
//! \param    [in] PpFilename
//!           ointer to the filename from which to read
//! \param    [out] lpNumberOfBytesRead,
//!           pointer to return the number of bytes read
//! \param    [out] ppReadBuffer
//!           Pointer to return the buffer pointer where
//!           the contents from the file are read to
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_ReadFileToPtr(
    const char         *pFilename,
    uint32_t           *lpNumberOfBytesRead,
    void               **ppReadBuffer);

//!
//! \brief    Writes contents of buffer into a file
//! \details  Writes contents of buffer into a file
//! \param    [in] pFilename
//!           Pointer to the filename to write the contents to
//! \param    [in] lpBuffer
//!           Pointer to the buffer whose contents will be written to the file
//! \param    [in] writeSize
//!           Number of bytes to write to the file
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_WriteFileFromPtr(
    const char              *pFilename,
    void                    *lpBuffer,
    uint32_t                writeSize);

//!
//! \brief    Retrieves the size of the specified File.
//! \details  Retrieves the size of the specified File.
//! \param    [in] hFile
//!           Handle to the File.
//! \param    [out] lpFileSizeLow
//!           Pointer to a variable where filesize is returned
//! \param    lpFileSizeHigh
//!           Reserved for now. Used to return higher uint32_t for
//!           filesizes more than 32 bit
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GetFileSize(
    HANDLE             hFile,
    uint32_t           *lpFileSizeLow,
    uint32_t           *lpFileSizeHigh);

//!
//! \brief    Creates a directory
//! \details  Creates a directory
//! \param    [in] lpPathName
//!           Pointer to the path name
//! \return   MOS_STATUS
//!           Returns MOS_STATUS_SUCCESS if directory was created or was already exists,
//!           else MOS_STATUS_DIR_CREATE_FAILED
//!
MOS_STATUS MOS_CreateDirectory(
    char * const       lpPathName);

//!
//! \brief    Creates or opens a file/object
//! \details  Creates or opens a file/object
//!           The definitions of the mode flags for iOpenFlag are in OS's fcntl.h
//! \param    [out] pHandle
//!           Pointer to a variable that recieves the handle
//!           of the file or object oepned
//! \param    [in] lpFileName
//!           Pointer to the file name
//! \param    [in] iOpenFlag
//!           Flag specifying mode and other options for Creating
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_CreateFile(
    PHANDLE               pHandle,
    char * const          lpFileName,
    uint32_t              iOpenFlag);

//!
//! \brief    Read data from a file
//! \details  Read data from a file
//! \param    [in] hFile
//!           Handle to the file to be read
//! \param    [out] lpBuffer
//!           Pointer to the buffer where the data read is placed
//! \param    [in] bytesToRead
//!           The maximum number of bytes to be read
//! \param    [out] pbytesRead
//!           Pointer to a variable that receives the number of bytes read
//! \param    [in/out] lpOverlapped
//!           Not used currently, can be nullptr
//!           When the hFile parameter was opened with FILE_FLAG_OVERLAPPED
//!           It should point to a valid OVERLAPPED structure
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_ReadFile(
    HANDLE          hFile,
    void            *lpBuffer,
    uint32_t        bytesToRead,
    uint32_t        *pbytesRead,
    void            *lpOverlapped);

//!
//! \brief    Write data to a file
//! \details  Write data to a file
//! \param    [in] hFile
//!           Handle to the file to which data will be written
//! \param    [in] lpBuffer
//!           Pointer to the buffer from where the data is read
//! \param    [in] bytesToWrite
//!           The maximum number of bytes to be written
//! \param    [out] pbytesWritten
//!           Pointer to a variable that receives the number of bytes written
//! \param    [in/out] lpOverlapped
//!           Not used currently, can be nullptr
//!           When the hFile parameter was opened with FILE_FLAG_OVERLAPPED
//!           It should point to a valid OVERLAPPED structure
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_WriteFile(
    HANDLE           hFile,
    void             *lpBuffer,
    uint32_t         bytesToWrite,
    uint32_t         *pbytesWritten,
    void             *lpOverlapped);

//!
//! \brief    Moves the File pointer to the specified position
//! \details  Moves the File pointer to the specified position
//!           Specify dwMoveMethod as the same as fseek()
//! \param    [in] hFile
//!           Handle to the file
//! \param    [in] lDistanceToMove
//!           Specifies no. of bytes to move the pointer
//! \param    [in] lpDistanceToMoveHigh
//!           Pointer to the high order 32-bits of
//!           the signed 64-bit distance to move.
//! \param    [in] dwMoveMethod
//!           Starting point for the file pointer move
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SetFilePointer(
    HANDLE                hFile,
    int32_t               lDistanceToMove,
    int32_t               *lpDistanceToMoveHigh,
    int32_t               dwMoveMethod);

//!
//! \brief    Closes an open object handle
//! \details  Closes an open object handle.
//! \param    [in] hObject
//!           A valid handle to an open object.
//! \return   int32_t
//!           true if success else false
//!
int32_t MOS_CloseHandle(
    HANDLE           hObject);

//!
//! \brief    Appends at the end of File
//! \details  Appends at the end of File
//! \param    [in] pFilename
//!           Pointer to the filename to append the contents to
//! \param    [in] pData
//!           Pointer to the buffer whose contents will be appeneded to the file
//! \param    [in] dwSize
//!           Number of bytes to append to the file
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_AppendFileFromPtr(
    const char               *pFilename,
    void                     *pData,
    uint32_t                 dwSize);

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

//!
//! \brief    Generate a User Feature Keys XML file according to user feature keys table in MOS
//! \details  Generate a User Feature Keys XML files according to MOSUserFeatureDescFields
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GenerateUserFeatureKeyXML();

//!
//! \brief    Get the User Feature Value from Table
//! \details  Get the related User Feature Value item according to Filter rules , and pass the item
//!            into return callback function
//! \param    [in] descTable
//!           The user feature key description table
//! \param    [in] numOfItems
//!           Number of user feature keys described in the table
//! \param    [in] maxId
//!           Max value ID in the table
//! \param    [out] keyValueMap
//!           Optional pointer to the value map where the table items will be linked to, could be nullptr
//! \param    [in] CallbackFunc
//!           Pointer to the Callback function, and pass the User Feature Value item as its parameter
//! \param    [in] pUserFeatureKeyFilter
//!           use the filter rule to select some User Feature Value item
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_GetItemFromMOSUserFeatureDescField(
    MOS_USER_FEATURE_VALUE      *descTable,
    uint32_t                    numOfItems,
    uint32_t                    maxId,
    MOS_USER_FEATURE_VALUE_MAP  *keyValueMap,
    MOS_STATUS                  (*CallbackFunc)(MOS_USER_FEATURE_VALUE_MAP *, PMOS_USER_FEATURE_VALUE),
    PMOS_USER_FEATURE_VALUE     pUserFeatureKeyFilter);

//!
//! \brief    Set the User Feature Default Value
//! \details  Set the User Feature Default Value in the user feature key map
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in] pWriteValues
//!           Pointer to User Feature Write Data
//! \param    [in] uiNumOfValues
//!           Number of User Feature Write Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_SetDefaultValues(
    PMOS_USER_FEATURE_INTERFACE             pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA      pWriteValues,
    uint32_t                                uiNumOfValues);

//!
//! \brief    Link user feature key description table items to specified UserFeatureKeyTable
//! \details  Link user feature key description table items to specified UserFeatureKeyTable
//!           according to ID sequence and do some post processing such as malloc related memory
//! \param    [in] userValueDescTable
//!           The user feature key description table
//! \param    [in] numOfValues
//!           Number of user feature keys described in the table
//! \param    [in] maxId
//!           Max value ID in the table
//! \param    [out] keyValueMap
//!           optional pointer to the value map where the table items will be linked to, could be nullptr
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DeclareUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *userValueDescTable,
    uint32_t                   numOfValues,
    uint32_t                   maxId,
    MOS_USER_FEATURE_VALUE_MAP *keyValueMap);

//!
//! \brief    Link the MOSUserFeatureDescFields table items to gc_UserFeatureKeysMap
//! \details  Link the MOSUserFeatureDescFields table items to gc_UserFeatureKeysMap
//!           according to ID sequence and do some post processing such as malloc related memory
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DeclareUserFeatureKeysForAllDescFields();

//!
//!
//! \brief    Destroy the User Feature Value pointer according to the DescField Table
//! \details  Destroy the User Feature Value pointer according to the DescField Table
//!           destroy the user feature key value Map according to Declare Count
//! \param    [in] descTable
//!           The user feature key description table
//! \param    [in] numOfItems
//!           Number of user feature keys described in the table
//! \param    [in] maxId
//!           Max value ID in the table
//! \param    [out] keyValueMap
//!           optional pointer to the value map where the table items will be destroyed, could be nullptr
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DestroyUserFeatureKeysFromDescFields(
    MOS_USER_FEATURE_VALUE     *descTable,
    uint32_t                   numOfItems,
    uint32_t                   maxId,
    MOS_USER_FEATURE_VALUE_MAP *keyValueMap);

//!
//!
//! \brief    Destroy the User Feature Value pointer according to the Global DescField Table
//! \details  Destroy the User Feature Value pointer according to the Global DescField Table
//!           destroy the gc_UserFeatureKeysMap according to Declare Count
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DestroyUserFeatureKeysForAllDescFields();

//!
//! \brief    Copy the VALUE_DATA from source to destination pointer
//! \details  Copy the VALUE_DATA from source to destination pointer
//! \param    [in] pSrcData
//!           Pointer to the Source Value Data
//! \param    [in] pDstData
//!           Pointer to the Destination Value Data
//! \param    [in] ValueType
//!           Value Type for the copy data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_CopyUserFeatureValueData(
    PMOS_USER_FEATURE_VALUE_DATA pSrcData,
    PMOS_USER_FEATURE_VALUE_DATA pDstData,
    MOS_USER_FEATURE_VALUE_TYPE ValueType
);

//!
//! \brief    Free the allocated memory for the related Value type
//! \details  Free the allocated memory for the related Value type
//! \param    [in] pData
//!           Pointer to the User Feature Value Data
//! \param    [in] ValueType
//!           related Value Type needed to be deallocated.
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_DestroyUserFeatureData(
    PMOS_USER_FEATURE_VALUE_DATA pData,
    MOS_USER_FEATURE_VALUE_TYPE  ValueType);

#ifdef  __MOS_USER_FEATURE_WA_
MOS_STATUS MOS_UserFeature_ReadValue (
    PMOS_USER_FEATURE_INTERFACE       pOsUserFeatureInterface,
    PMOS_USER_FEATURE                 pUserFeature,
    const char                        *pValueName,
    MOS_USER_FEATURE_VALUE_TYPE       ValueType);
#endif

//!
//! \brief    Read Single Value from User Feature based on value of enum type in MOS_USER_FEATURE_VALUE_TYPE
//! \details  This is a unified funtion to read user feature key for all components.
//!           (Codec/VP/CP/CM)
//!           It is required to prepare all memories for buffers before calling this function.
//!           User can choose to use array variable or allocated memory for the buffer.
//!           If the buffer is allocated dynamically, it must be freed by user to avoid memory leak. 
//!           ------------------------------------------------------------------------------------
//!           Usage example: 
//!           a) Initiation:
//!           MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
//!           b.0) Don't need to input a default value if the default value in user feature key Desc Fields table item is good 
//!                for your case
//!           b.1) For uint32_t type:
//!           UserFeatureData.u32Data = 1;    // overwrite a custom default value 
//!           UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE; 
//!                                           // raise a flag to use this custom default value instead of 
//!                                              default value in user feature key Desc Fields table item
//!           b.2) For String/Binary type:
//!           char cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           UserFeatureData.StringData.pStringData = cStringData; // make sure the pointer is valid
//!           b.3) For MultiString type:
//!           char                          cStringData[MOS_USER_CONTROL_MAX_DATA_SIZE];
//!           MOS_USER_FEATURE_VALUE_STRING Strings[__MAX_MULTI_STRING_COUNT];
//!           UserFeatureData.MultiStringData.pMultStringData = cStringData; // make sure the pointer is valid
//!           for (ui = 0; ui < VPHAL_3P_MAX_LIB_PATH_COUNT; ui++)
//!           {
//!             Strings[ui].pStringData = (char *)MOS_AllocAndZeroMemory(MOS_USER_CONTROL_MAX_DATA_SIZE);
//!           }
//!           UserFeatureData.MultiStringData.pStrings = Strings;
//!           c) Read user feature key:
//!           MOS_UserFeature_ReadValue_ID();
//!           -------------------------------------------------------------------------------------
//!           Important note: The pointer pStringData/pMultStringData may be modified if the 
//!           previous MOS_UserFeature_ReadValue() doesn't read a same user feature key type. So it's 
//!           suggested to set the union members in UserFeatureValue every time before 
//!           MOS_UserFeature_ReadValue() if you are not familiar with the details of this function.
//!           If a new key is added, please make sure to declare a definition in corresponding
//!           user feature key Desc Fields tableby MOS_DECLARE_UF_KEY
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in] ValueID
//!           value of enum type in MOS_USER_FEATURE_VALUE_TYPE. declares the user feature key to be readed
//! \param    [in,out] pValueData
//!           Pointer to User Feature Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!           For pValueData return value:
//!                 MOS_STATUS_SUCCESS: pValueData is from User Feature Key
//!                 MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED: pValueData is from default value
//!                 MOS_STATUS_UNKNOWN: pValueData is from default value
//!                 MOS_STATUS_USER_FEATURE_KEY_READ_FAILED: pValueData is from default value
//!                 MOS_STATUS_NULL_POINTER: NO USER FEATURE KEY DEFINITION in corresponding user feature key Desc Field table, 
//!                                          No default value or User Feature Key value return
//! 
//!
#ifdef  __MOS_USER_FEATURE_WA_
MOS_STATUS MOS_UserFeature_ReadValue_ID(
#else
MOS_STATUS MOS_UserFeature_ReadValue(
#endif
    PMOS_USER_FEATURE_INTERFACE  pOsUserFeatureInterface,
    uint32_t                     ValueID,
    PMOS_USER_FEATURE_VALUE_DATA pValueData);

//!
//! \brief    Write Values to User Feature with specified ID
//! \details  Write Values to User Feature with specified ID
//!           The caller is responsible to allocate values / names
//!           and free them later if necessary
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in] pWriteValues
//!           Pointer to User Feature Data, and related User Feature Key ID (enum type in MOS_USER_FEATURE_VALUE_TYPE)
//! \param    [in] uiNumOfValues
//!           number of user feature keys to be written.
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
#ifdef  __MOS_USER_FEATURE_WA_
MOS_STATUS MOS_UserFeature_WriteValues_ID(
#else
MOS_STATUS MOS_UserFeature_WriteValues(
#endif
    PMOS_USER_FEATURE_INTERFACE              pOsUserFeatureInterface,
    PMOS_USER_FEATURE_VALUE_WRITE_DATA       pWriteValues,
    uint32_t                                 uiNumOfValues);


// User Feature Report Writeout
#define WriteUserFeature64(key, value)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i64Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1);\
}

#define WriteUserFeature(key, value)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i32Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1);\
}

#define WriteUserFeatureString(key, value, len)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.StringData.pStringData = (value);\
    UserFeatureWriteData.Value.StringData.uSize = (len);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1);\
}

//!
//! \brief    Lookup the user feature value name associated with the ID
//! \details  Lookup the user feature value name associated with the ID
//! \param    [in] ValueId
//!           The user feature value ID to be looked up
//! \return   const char*
//!           pointer to the char array holding the user feature value name
//!
const char* MOS_UserFeature_LookupValueName(
    uint32_t ValueID);

//!
//! \brief    Lookup the read path associated with the ID
//! \details  Lookup the read path associated with the ID
//! \param    [in] ValueId
//!           The user feature value ID to be looked up
//! \return   const char*
//!           pointer to the char array holding the read path
//!
const char* MOS_UserFeature_LookupReadPath(
    uint32_t ValueID);

//!
//! \brief    Lookup the write path associated with the ID
//! \details  Lookup the write path associated with the ID
//! \param    [in] ValueId
//!           The user feature value ID to be looked up
//! \return   const char*
//!           pointer to the char array holding the write path
//!
const char* MOS_UserFeature_LookupWritePath(
    uint32_t ValueID);

//!
//! \brief    Enable user feature change notification
//! \details  Enable user feature change notification
//!           Create notification data and register the wait event
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in/out] pNotification
//!           Pointer to User Feature Notification Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_EnableNotification(
    PMOS_USER_FEATURE_INTERFACE               pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA             pNotification);

//!
//! \brief    Disable user feature change notification
//! \details  Disable user feature change notification
//!           Unregister the wait event and frees notification data
//! \param    [in] pOsUserFeatureInterface
//!           Pointer to OS User Interface structure
//! \param    [in/out] pNotification
//!           Pointer to User Feature Notification Data
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_DisableNotification(
    PMOS_USER_FEATURE_INTERFACE                pOsUserFeatureInterface,
    PMOS_USER_FEATURE_NOTIFY_DATA              pNotification);

//!
//! \brief    Parses the user feature path and gets type and sub path
//! \details  Parses the user feature path and gets type and sub path
//!           It verifies if the user feature path is valid,
//!           and check if it belongs to UFEXT or UFINT UFKEY.
//!           The identified type and subpath are set accordingly.
//! \param    [in] pOsUserFeatureInterface,
//!           Pointer to OS User Interface structure
//! \param    [in] pInputPath
//!           The input user feature path
//! \param    [out] pUserFeatureType
//!           Pointer to the variable to receive user feature type
//! \param    [out] ppSubPath
//!           Pointer to a variable that accepts the pointer to the subpath
//! \return   MOS_STATUS
//!           Returns MOS_STATUS_INVALID_PARAMETER if failed, else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_UserFeature_ParsePath(
    PMOS_USER_FEATURE_INTERFACE  pOsUserFeatureInterface,
    char * const                 pInputPath,
    PMOS_USER_FEATURE_TYPE       pUserFeatureType,
    char                         **ppSubPath);

//!
//! \brief    String concatenation with security checks.
//! \details  String concatenation with security checks.
//!           Append strSource to strDestination, with buffer size checking
//! \param    [in/out] strDestination
//!           Pointer to destination string
//! \param    [in] numberOfElements
//!           Size of the destination buffer
//! \param    [in] strSource
//!           Pointer to the source string
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureStrcat(
    char                *strDestination,
    size_t              numberOfElements,
    const char * const  strSource);

//!
//! \brief    Find string token with security checks.
//! \details  Find string token with security checks.
//!           Subsequent calls with nullptr in strToken and same contex to get
//!           remaining tokens
//! \param    [in/out] strToken
//!           String containing token or tokens
//!           Pass nullptr for this parameter in subsequent calls
//!           to MOS_SecureStrtok to find the remaining tokens
//! \param    [in] strDelimit
//!           Set of delimiter characters
//! \param    [in/out] contex
//!           Used to store position information between calls to MOS_SecureStrtok
//! \return   char *
//!           Returns tokens else nullptr
//!
char  *MOS_SecureStrtok(
    char                *strToken,
    const char          *strDelimit,
    char                **contex);

//!
//! \brief    String copy with security checks.
//! \details  String copy with security checks.
//!           Copy strSource to strDestination, with buffer size checking
//! \param    [out] strDestination
//!           Pointer to destination string
//! \param    [in] numberOfElements
//!           Size of the destination buffer
//! \param    [in] strSource
//!           Pointer to the source string
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureStrcpy(
    char                *strDestination,
    size_t              numberOfElements,
    const char * const  strSource);

//!
//! \brief    Memory copy with security checks.
//! \details  Memory copy with security checks.
//!           Copy pSource to pDestination, with buffer size checking
//! \param    [out] pDestination
//!           Pointer to destination buffer
//! \param    [in] dstLength
//!           Size of the destination buffer
//! \param    [in] pSource
//!           Pointer to the source buffer
//! \param    [in] srcLength
//!           Number of bytes to copy from source to destination
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureMemcpy(
    void                *pDestination,
    size_t              dstLength,
    const void          *pSource,
    size_t              srcLength);

//!
//! \brief    Open a file with security checks.
//! \details  Open a file with security checks.
//! \param    [out] ppFile
//!           Pointer to a variable that receives the file pointer.
//! \param    [in] filename
//!           Pointer to the file name string
//! \param    [in] mode
//!           Specifies open mode such as read, write etc
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_SecureFileOpen(
    FILE       **ppFile,
    const char *filename,
    const char *mode);

//!
//! \brief    Write formatted data to a string with security checks.
//! \details  Write formatted data to a string with security checks.
//!           Optional arguments are passed in individually
//!           Buffer must have space for null character after copying length
//! \param    [out] buffer
//!           Pointer to a string to which formatted data is printed
//! \param    [in] bufSize
//!           Size of the buffer where the data is printed
//! \param    [in] length
//!           Number of characters to be printed
//! \param    [in] format
//!           Format string to be printed
//! \return   int32_t
//!           Returns the number of characters printed or -1 if an error occurs
//!
int32_t MOS_SecureStringPrint(
    char                     *buffer,
    size_t                   bufSize,
    size_t                   length,
    const  char * const      format,
                                 ...);

//!
//! \brief    Write formatted data to a string with security checks, va_list version
//! \details  Write formatted data to a string with security checks.
//!           Pointer to an optional arguments list is passed in
//!           Buffer must have space for null character after copying length
//! \param    [out] buffer
//!           Pointer to a string to which formatted data is printed
//! \param    [in] bufSize
//!           Size of the buffer where the data is printed
//! \param    [in] length
//!           Number of characters to be printed
//! \param    [in] format
//!           Format string to be printed
//! \param    [in] var_args
//!           Optional argument list
//! \return   int32_t
//!           Returns the number of characters printed or -1 if an error occurs
//!
MOS_STATUS MOS_SecureVStringPrint(
    char                      *buffer,
    size_t                    bufSize,
    size_t                    length,
    const char * const        format,
    va_list                   var_args);

//------------------------------------------------------------------------------
// Library, process and OS related functions
//------------------------------------------------------------------------------
//!
//! \brief    Maps the specified executable module into the address space of
//!           the calling process.
//! \details  Maps the specified executable module into the address space of
//!           the calling process.
//! \param    [in] lpLibFileName
//!           A valid handle to an open object.
//! \param    [out] phModule
//!           Pointer variable that accepts the module handle
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_LoadLibrary(
    const char * const lpLibFileName,
    PHMODULE           phModule);

//!
//! \brief    Free the loaded dynamic-link library
//! \details  Free the loaded dynamic-link library
//! \param    [in] hLibModule
//!           A handle to the loaded DLL module
//! \return   int32_t
//!           true if success else false
//!
int32_t MOS_FreeLibrary(HMODULE hLibModule);

//!
//! \brief    Retrieves the address of an exported function or variable from
//!           the specified dynamic-link library
//! \details  Retrieves the address of an exported function or variable from
//!           the specified dynamic-link library
//! \param    [in] hLibModule
//!           A handle to the loaded DLL module.
//!           The LoadLibrary function returns this handle.
//! \param    [in] lpProcName
//!           The function or variable name, or the function's ordinal value.
//! \return   void *
//!           If succeeds, the return value is the address of the exported
//!           function or variable. If fails, the return value is NULL.
//!           To get extended error information, call GetLastError.
//!
void  *MOS_GetProcAddress(
    HMODULE     hModule,
    const char  *lpProcName);

//!
//! \brief    Retrieves the current process id
//! \details  Retrieves the current process id
//! \return   int32_t
//!           Return the current process id
//!
int32_t MOS_GetPid();

//!
//! \brief    Retrieves the frequency of the high-resolution performance
//!           counter, if one exists.
//! \details  Retrieves the frequency of the high-resolution performance
//!           counter, if one exists.
//! \param    [out] pFrequency
//!           Pointer to a variable that receives the current
//!           performance-counter frequency, in counts per second.
//! \return   int32_t
//!           If the installed hardware supports a high-resolution performance
//!           counter, the return value is nonzero. If the function fails, the
//!           return value is zero.
//!
int32_t MOS_QueryPerformanceFrequency(
    uint64_t                       *pFrequency);

//!
//! \brief    Retrieves the current value of the high-resolution performance
//!           counter
//! \details  Retrieves the current value of the high-resolution performance
//!           counter
//! \param    [out] pPerformanceCount
//!           Pointer to a variable that receives the current
//!           performance-counter value, in counts.
//! \return   int32_t
//!           If the installed hardware supports a high-resolution performance
//!           counter, the return value is nonzero. If the function fails, the
//!           return value is zero. To get extended error information, call GetLastError.
//!
int32_t MOS_QueryPerformanceCounter(
    uint64_t                     *pPerformanceCount);

//!
//! \brief    Sleep for given duration in ms
//! \details  Sleep for given duration ms
//! \param    [in] mSec
//!           Sleep duration in ms
//! \return   void
//!
void MOS_Sleep(
    uint32_t   mSec);

//------------------------------------------------------------------------------
// Wrappers for OS Specific User Feature Functions Implementations
//------------------------------------------------------------------------------
//!
//! \brief    Opens the specified user feature key
//! \details  Opens the specified user feature key
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//! \param    [in] lpSubKey
//!           The name of the user feature subkey to be opened.
//! \param    [in] lOptions
//!           This parameter is reserved and must be zero.
//! \param    [in] samDesired
//!           Reserved, could be any REGSAM type value
//! \param    [out] phkResult
//!           A pointer to a variable that receives a handle to the opened key.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureOpenKey(
    void              *UFKey,
    const char        *lpSubKey,
    uint32_t          ulOptions,
    uint32_t          samDesired,
    void              **phkResult);

//!
//! \brief    Closes a handle to the specified user feature key
//! \details  Closes a handle to the specified user feature key
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureCloseKey(
    void               *UFKey);

//!
//! \brief    Retrieves the type and data for the specified user feature value
//! \details  Retrieves the type and data for the specified user feature value
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//! \param    [in] lpSubKey
//!           The name of the user feature key. This key must be a
//!           subkey of the key specified by the hkey parameter
//! \param    [in] lpValue
//!           The name of the user feature value
//! \param    [in] dwFlags
//!           The flags that restrict the data type of value to be queried
//! \param    [out] pdwType
//!           A pointer to a variable that receives a code indicating the type
//!           of data stored in the specified value.
//! \param    [out] pvData
//!           A pointer to a buffer that receives the value's data.
//! \param    [in/out] pcbData
//!           A pointer to a variable that specifies the size of the buffer
//!           pointed to by the pvData parameter, in bytes. When the function
//!           returns, this variable contains the size of the data copied to lpData.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureGetValue(
    void               *UFKey,
    const char         *lpSubKey,
    const char         *lpValue,
    uint32_t           dwFlags,
    uint32_t           *pdwType,
    void               *pvData,
    uint32_t           *pcbData);

//!
//! \brief    Retrieves the type and data for the specified value name
//!           associated with an open user feature key.
//! \details  Retrieves the type and data for the specified value name
//!           associated with an open user feature key.
//! \param    [in] UFKey
//!           A handle to an open user feature key
//! \param    [in] lpValueName
//!           The name of the user feature value
//! \param    [in] lpReserved
//!           This parameter is reserved and must be NULL.
//! \param    [out] lpType
//!           A pointer to a variable that receives a code indicating
//!           the type of data stored in the specified value.
//! \param    [out] lpData
//!           A pointer to a buffer that receives the value's data.
//! \param    [in/out] lpcbData
//!           A pointer to a variable that specifies the size
//!           of the buffer pointed to by the pvData parameter,
//!           in bytes. When the function returns, this variable
//!           contains the size of the data copied to lpData.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureQueryValueEx(
    void                    *UFKey,
    char                    *lpValueName,
    uint32_t                *lpReserved,
    uint32_t                *lpType,
    char                    *lpData,
    uint32_t                *lpcbData);

//!
//! \brief    Sets the data and type of a specified value under a user feature key
//! \details  Sets the data and type of a specified value under a user feature key
//! \param    [in] UFKey
//!           A handle to an open user feature key
//! \param    [in] lpValueName
//!           The name of the user feature value
//! \param    [in] Reserved
//!           This parameter is reserved and must be nullptr
//! \param    [in] dwType
//!           The type of data pointed to by the lpData parameter
//! \param    [in] lpData
//!           The data to be stored.
//! \param    [in] cbData
//!           The size of the information pointed to by the lpData parameter, in bytes.
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureSetValueEx(
    void                 *UFKey,
    const char           *lpValueName,
    uint32_t             Reserved,
    uint32_t             dwType,
    uint8_t              *lpData,
    uint32_t             cbData);

//!
//! \brief    Notifies the caller about changes to the attributes or contents
//!           of a specified user feature key
//! \details  Notifies the caller about changes to the attributes or contents
//!           of a specified user feature key
//!           Used internally by MOS_UserFeature_EnableNotification()
//! \param    [in] UFKey
//!           A handle to an open user feature key.
//!           The key must have been opened with the KEY_NOTIFY access right.
//! \param    [in] bWatchSubtree
//!           true including subkey changes; false for the key itself
//! \param    [in] hEvent
//!           A handle to an event to be signaled when key changes if is true
//! \param    [in] fAsynchronous
//!           true: Return immediately and signal the hEvent when key change
//!           false: Does not return until a change has occured
//! \return   MOS_STATUS
//!           If the function succeeds, the return value is MOS_STATUS_SUCCESS.
//!           If the function fails, the return value is a error code defined
//!           in mos_utilities.h.
//!
MOS_STATUS MOS_UserFeatureNotifyChangeKeyValue(
    void                           *UFKey,
    int32_t                        bWatchSubtree,
    HANDLE                         hEvent,
    int32_t                        fAsynchronous);

//!
//! \brief    Creates or opens a event object and returns a handle to the object
//! \details  Creates or opens a event object and returns a handle to the object
//! \param    [in] lpEventAttributes
//!           A pointer to a SECURITY_ATTRIBUTES structure.
//!           If lpEventAttributes is nullptr, the event handle cannot be inherited
//!           by child processes.
//! \param    [in] lpName
//!           The name of the event object.If lpName is nullptr, the event object is
//!           created without a name.
//! \param    [in] dwFlags
//!           Combines the following flags
//!           CREATE_EVENT_INITIAL_SET: Singal initial state or not
//!           CREATE_EVENT_MANUAL_RESET: Must be manually reset or not
//! \return   HANDLE
//!           If the function succeeds, the return value is a handle to the
//!           event object. If failed, returns NULL. To get extended error
//!           information, call GetLastError.
//!
HANDLE MOS_CreateEventEx(
    void                 *lpEventAttributes,
    char                 *lpName,
    uint32_t             dwFlags);

//!
//! \brief    Create a wait thread to wait on the object
//! \details  Create a wait thread to wait on the object
//!           Add this function to capatible WDK-9200 on vs2012.
//! \param    [out] phNewWaitObject
//!           A pointer to a variable that receives a wait handle on return.
//! \param    [in] hObject
//!           A handle to the object
//! \param    [in] Callback
//!           A pointer to the application-defined function of type
//!           WAITORTIMERCALLBACK to be executed when wait ends.
//! \param    [in] Context
//!           A single value that is passed to the callback function
//! \return   int32_t
//!           The return value is int32_t type. If the function succeeds,
//!           the return value is nonzero. If the function fails, the
//!           return value is zero.
//!
int32_t MOS_UserFeatureWaitForSingleObject(
    PTP_WAIT                         *phNewWaitObject,
    HANDLE                           hObject,
    void                             *Callback,
    void                             *Context);

//!
//! \brief    Cancels a registered wait operation issued by the
//!           RegisterWaitForSingleObject function
//! \details  Cancels a registered wait operation issued by the
//!           RegisterWaitForSingleObject function
//! \param    [in] hWaitHandle
//!           The wait handle. This handle is returned by the
//!           RegisterWaitForSingleObject function
//! \return   int32_t
//!           The return value is int32_t type. If the function succeeds,
//!           the return value is nonzero. If the function fails, the
//!           return value is zero.
//!
int32_t MOS_UnregisterWaitEx(
    PTP_WAIT                hWaitHandle);

//!
//! \brief    Get logical core number of current CPU
//! \details  Get logical core number of current CPU
//! \return   uint32_t
//!           If the function succeeds, the return value is the number of
//!           current CPU.
//!
uint32_t MOS_GetLogicalCoreNumber();

//!
//! \brief    Creates or opens a thread object and returns a handle to the object
//! \details  Creates or opens a thread object and returns a handle to the object
//! \param    [in] ThreadFunction
//!           A pointer to a thread function.
//! \param    [in] ThreadData
//!           A pointer to thread data.
//! \return   MOS_THREADHANDLE
//!           If the function succeeds, the return value is a handle to the
//!           thread object. If failed, returns NULL.
//!
MOS_THREADHANDLE MOS_CreateThread(
    void                        *ThreadFunction,
    void                        *ThreadData);

//!
//! \brief    Get thread id
//! \details  Get thread id
//! \param    [in] hThread
//!           A handle of thread object.
//! \return   uint32_t
//!           Return the current thread id
//!
uint32_t MOS_GetThreadId(
    MOS_THREADHANDLE            hThread);

//!
//! \brief    Retrieves the current thread id
//! \details  Retrieves the current thread id
//! \return   uint32_t
//!           Return the current thread id
//!
uint32_t MOS_GetCurrentThreadId();

//!
//! \brief    Wait for thread to terminate
//! \details  Wait for thread to terminate
//! \param    [in] hThread
//!           A handle of thread object.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_WaitThread(
    MOS_THREADHANDLE            hThread);

//!
//! \brief    Create mutex for context protection across threads
//! \details  Create mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    NONE
//! \return   PMOS_MUTEX
//!           Pointer of mutex
//!
PMOS_MUTEX MOS_CreateMutex();

//!
//! \brief    Destroy mutex for context protection across threads
//! \details  Destroy mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    [in] pMutex
//!           Pointer of mutex
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_DestroyMutex(PMOS_MUTEX pMutex);

//!
//! \brief    Lock mutex for context protection across threads
//! \details  Lock mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    [in] pMutex
//!           Pointer of mutex
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_LockMutex(PMOS_MUTEX pMutex);

//!
//! \brief    Unlock mutex for context protection across threads
//! \details  Unlock mutex for context protection across threads
//!           Used for multi-threading of Hybrid Decoder
//! \param    [in] pMutex
//!           Pointer of mutex
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_UnlockMutex(PMOS_MUTEX pMutex);

//!
//! \brief    Creates or opens a semaphore object and returns a handle to the object
//! \details  Creates or opens a semaphore object and returns a handle to the object
//! \param    [in] uiInitialCount
//!           Initial count of semaphore usage.
//! \param    [in] uiMaximumCount
//!           Maximum count of semaphore usage.
//! \return   PMOS_SEMAPHORE
//!           If the function succeeds, the return value is a handle to the
//!           semaphore object. If failed, returns NULL. To get extended error
//!           information, call GetLastError.
//!
PMOS_SEMAPHORE MOS_CreateSemaphore(
    uint32_t                    uiInitialCount,
    uint32_t                    uiMaximumCount);

//!
//! \brief    Destroy a semaphore object
//! \details  Destroy a semaphore object
//! \param    [in] pSemaphore
//!           A handle of semaphore object.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_DestroySemaphore(
    PMOS_SEMAPHORE              pSemaphore);

//!
//! \brief    Wait a semaphore object
//! \details  Wait a semaphore object
//! \param    [in] pSemaphore
//!           A handle of semaphore object.
//! \param    [in] uiMilliseconds
//!           Wait time.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_WaitSemaphore(
    PMOS_SEMAPHORE              pSemaphore,
    uint32_t                    uiMilliseconds);

//!
//! \brief    Post a semaphore object
//! \details  Post a semaphore object
//! \param    [in] pSemaphore
//!           A handle of semaphore object.
//! \param    [in] uiPostCount
//!           semaphore post count.
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_PostSemaphore(
    PMOS_SEMAPHORE              pSemaphore,
    uint32_t                    uiPostCount);

//!
//! \brief    Wait for single object of semaphore/mutex/thread and returns the result
//! \details  Wait for single object of semaphore/mutex/thread and returns the result
//! \param    [in] pObject
//!           Object handle.
//! \param    [in] uiMilliseconds
//!           Wait time.
//! \return   uint32_t
//!           If the function succeeds, the return value is the wait result of the
//!           semaphore/mutex/thread object.
//!
uint32_t MOS_WaitForSingleObject(
    void                        *pObject,
    uint32_t                    uiMilliseconds);

//!
//! \brief    Wait for multiple objects of semaphore/mutex/thread and returns the result
//! \details  Wait for multiple objects of semaphore/mutex/thread and returns the result
//! \param    [in] uiThreadCount
//!           The number of object handles in the array pointed to by ppObjects.
//! \param    [in] ppObjects
//!           An array of object handles.
//! \param    [in] bWaitAll
//!           If true, the function returns when the state of all objects in the ppObjects array is signaled.
//!           If false, the function returns when the state of any one of the objects is set to signaled.
//! \param    [in] uiMilliseconds
//!           The time-out interval, in milliseconds.
//! \return   uint32_t
//!           Return the wait result
//!
uint32_t MOS_WaitForMultipleObjects(
    uint32_t                    uiThreadCount,
    void                        **ppObjects,
    uint32_t                    bWaitAll,
    uint32_t                    uiMilliseconds);

//!
//! \brief    Increments (increases by one) the value of the specified int32_t variable as an atomic operation.
//! \param    [in] pValue
//!           A pointer to the variable to be incremented.
//! \return   int32_t
//!           The function returns the resulting incremented value.
//!
int32_t MOS_AtomicIncrement(
    int32_t *pValue);

//!
//! \brief    Decrements (decreases by one) the value of the specified int32_t variable as an atomic operation.
//! \param    [in] pValue
//!           A pointer to the variable to be decremented.
//! \return   int32_t
//!           The function returns the resulting decremented value.
//!
int32_t MOS_AtomicDecrement(
    int32_t *pValue);

//!
//! \brief      Convert MOS_STATUS to OS dependent RESULT/Status
//! \param      [in] eStatus
//!             MOS_STATUS that will be converted
//! \return     MOS_OSRESULT
//!             Corresponding return code on different OSes
//!
MOS_OSRESULT MOS_StatusToOsResult(
    MOS_STATUS               eStatus);

//!
//! \brief      Convert OS dependent RESULT/Status to MOS_STATUS
//! \param      [in] eResult
//!             OS dependent result that will be converted
//! \return     MOS_STATUS
//!             Corresponding MOS_STATUS
//!
MOS_STATUS OsResultToMOS_Status(
    MOS_OSRESULT            eResult);

//!
//! \brief    sinc
//! \details  Calculate sinc(x)
//! \param    [in] x
//!           float
//! \return   float
//!           sinc(x)
//!
float MOS_Sinc(
    float                   x);

//!
//! \brief    Lanczos
//! \details  Calculate lanczos(x)
//!           Basic formula is:  lanczos(x)= MOS_Sinc(x) * MOS_Sinc(x / fLanczosT)
//! \param    [in] x
//!           float
//! \param    [in] dwNumEntries
//!           dword
//! \param    [in] fLanczosT
//! 
//! \return   float
//!           lanczos(x)
//!
float MOS_Lanczos(
    float                   x,
    uint32_t                dwNumEntries,
    float                   fLanczosT);

//!
//! \brief    General Lanczos
//! \details  Calculate lanczos(x)  with odd entry num support
//!           Basic formula is:  lanczos(x)= MOS_Sinc(x) * MOS_Sinc(x / fLanczosT)
//! \param    [in] x
//!           float
//! \param    [in] dwNumEntries
//!           dword
//! \param    [in]fLanczosT
//! 
//! \return   float
//!           lanczos(x)
//!
float MOS_Lanczos_g(
    float                   x,
    uint32_t                dwNumEntries,
    float                   fLanczosT);

//!
//! \brief    GCD
//! \details  Recursive GCD calculation of two numbers
//! \param    [in] a
//!           uint32_t
//! \param    [in] b
//!           uint32_t
//! \return   uint32_t
//!           MOS_GCD(a, b)
//!
uint32_t MOS_GCD(
    uint32_t               a,
    uint32_t               b);

//!
//! \brief    Get local time
//! \details  Get local time
//! \param    [out] tm
//!           tm struct
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_GetLocalTime(
    struct tm* tm);

//!
//! \brief    Swizzles the given linear offset via the specified tiling params.
//! \details  Swizzles the given linear offset via the specified tiling parameters. 
//!           Used to provide linear access to raw, tiled data.
//! \param    [in] OffsetX
//!           Horizontal byte offset from left edge of tiled surface.
//! \param    [in] OffsetY
//!           Vertical offset from top of tiled surface.
//! \param    [in] Pitch
//!           Row-to-row byte stride.
//! \param    [in] TileFormat
//!           Either 'x' or 'y'--for X-Major or Y-Major tiling, respectively.
//! \param    [in] CsxSwizzle
//!           (Boolean) Additionally perform Channel Select XOR swizzling.
//! \param    [in] flags
//!           More flags to indicate different tileY.
//! \return   int32_t
//!           Return SwizzleOffset
//!    
int32_t __Mos_SwizzleOffset(
    int32_t         OffsetX,
    int32_t         OffsetY,
    int32_t         Pitch,
    MOS_TILE_TYPE   TileFormat,
    int32_t         CsxSwizzle,
    int32_t         flags);

//!
//! \brief    Wrapper function for SwizzleOffset
//! \details  Wrapper function for SwizzleOffset in Mos 
//! \param    [in] pSrc
//!           Pointer to source data.
//! \param    [out] pDst
//!           Pointer to destiny data.
//! \param    [in] SrcTiling
//!           Source Tile Type
//! \param    [in] DstTiling
//!           Destiny Tile Type
//! \param    [in] iHeight
//!           Height
//! \param    [in] iPitch
//!           Pitch
//! \param    [in] extended flags
//!           Pitch
//! \return   void
//!
void Mos_SwizzleData(
    uint8_t         *pSrc,
    uint8_t         *pDst,
    MOS_TILE_TYPE   SrcTiling,
    MOS_TILE_TYPE   DstTiling,
    int32_t         iHeight,
    int32_t         iPitch,
    int32_t         extFlags);

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
    void * const     pArg1,
    uint32_t         dwSize1,
    void * const     pArg2,
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
    char *const pcName,
    uint32_t    flags,
    void *const pBuf,
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

#ifdef __cplusplus
}
#endif

#endif // __MOS_UTILITIES_H__
