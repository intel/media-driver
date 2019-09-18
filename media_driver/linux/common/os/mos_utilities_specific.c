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
//! \file        mos_utilities_specific.c
//! \brief        This module implements the MOS wrapper functions for Linux/Android
//!

#include "mos_utilities_specific.h"
#include "mos_utilities.h"
#include "mos_util_debug.h"
#include <fcntl.h>     // open
#include <stdlib.h>    // atoi
#include <string.h>    // strlen, strcat, etc.
#include <errno.h>     // strerror(errno)
#include <time.h>      // get_clocktime
#include <sys/stat.h>  // fstat
#include <dlfcn.h>     // dlopen, dlsym, dlclose
#include <sys/types.h>
#include <unistd.h>
#if _MEDIA_RESERVED
#include "codechal_util_user_interface_ext.h"
#endif // _MEDIA_RESERVED
#ifndef ANDROID
#include <sys/ipc.h>   // System V IPC
#include <sys/types.h>
#include <sys/sem.h>
#include <signal.h>
#include <unistd.h>    // fork
#else
#include <cutils/properties.h>
#endif // ANDROID

static const char* szUserFeatureFile = USER_FEATURE_FILE;

#ifdef __cplusplus

void PerfUtility::startTick(std::string tag)
{
    Tick newTick = {};
    struct timespec ts = {};

    // get start tick count
    clock_gettime(CLOCK_REALTIME, &ts);
    newTick.start = int(ts.tv_sec * 1000000) + int(ts.tv_nsec / 1000); // us

    std::vector<Tick> *perf = nullptr;
    std::map<std::string, std::vector<Tick>*>::iterator it;
    it = records.find(tag);
    if (it == records.end())
    {
        perf = new std::vector<Tick>;
        perf->push_back(newTick);
        records[tag] = perf;
    }
    else
    {
        it->second->push_back(newTick);
    }
}

void PerfUtility::stopTick(std::string tag)
{
    struct timespec ts = {};
    std::map<std::string, std::vector<Tick>*>::iterator it;
    it = records.find(tag);
    if (it == records.end())
    {
        // should not happen
        return;
    }

    // get stop tick count
    clock_gettime(CLOCK_REALTIME, &ts);
    it->second->back().stop = int(ts.tv_sec * 1000000) + int(ts.tv_nsec / 1000); // us

    // calculate time interval
    it->second->back().time = double(it->second->back().stop - it->second->back().start) / 1000.0; // ms
}

#endif // __cplusplus

//!
//! \brief    Get current run time
//! \details  Get current run time in us
//! \return   double
//!           Returns time in us
//!
double MOS_GetTime()
{
    struct timespec ts = {};
    clock_gettime(CLOCK_REALTIME, &ts);
    return double(ts.tv_sec) * 1000000.0 + double(ts.tv_nsec) / 1000.0;
}

//!
//! \brief Linux specific user feature define, used in MOS_UserFeature_ParsePath
//!        They can be unified with the win definitions, since they are identical.
//!
#define MOS_UF_SEPARATOR  "\\"
#define MOS_UFKEY_EXT     "UFKEY_EXTERNAL"
#define MOS_UFKEY_INT     "UFKEY_INTERNAL"
PUFKEYOPS      pUFKeyOps = nullptr;

extern int32_t MosMemAllocCounterNoUserFeature;
extern int32_t MosMemAllocCounterNoUserFeatureGfx;

//!
//! \brief Linux specific trace entry path and file description.
//!
const char * const MosTracePath = "/sys/kernel/debug/tracing/trace_marker";
static int32_t MosTraceFd = -1;

//!
//! \brief for int64_t/uint64_t format print warning
//!
#if __WORDSIZE == 64
#define __MOS64_PREFIX    "l"
#else
#define __MOS64_PREFIX    "ll"
#endif

#define MOSd64     __MOS64_PREFIX "d"
#define MOSu64     __MOS64_PREFIX "u"

#if _MEDIA_RESERVED
MosUtilUserInterface  *utilUserInterface = nullptr;
#endif // _MEDIA_RESERVED

//!
//! \brief mutex for mos utilities multi-threading protection
//!
MOS_MUTEX gMosUtilMutex = PTHREAD_MUTEX_INITIALIZER;

static uint32_t uiMOSUtilInitCount = 0; // number count of mos utilities init

MOS_STATUS MOS_SecureStrcat(char  *strDestination, size_t numberOfElements, const char * const strSource)
{
    if ( (strDestination == nullptr) || (strSource == nullptr) )
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if(strnlen(strDestination, numberOfElements) == numberOfElements) // Not null terminated
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if((strlen(strDestination) + strlen(strSource)) >= numberOfElements) // checks space for null termination.
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    strcat(strDestination, strSource);
    return MOS_STATUS_SUCCESS;
}

char  *MOS_SecureStrtok(
    char                *strToken,
    const char          *strDelimit,
    char                **contex)
{
    return strtok_r(strToken, strDelimit, contex);
}

MOS_STATUS MOS_SecureStrcpy(char  *strDestination, size_t numberOfElements, const char * const strSource)
{
    if ( (strDestination == nullptr) || (strSource == nullptr) )
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ( numberOfElements <= strlen(strSource) ) // checks if there is space for null termination after copy.
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    strcpy(strDestination, strSource);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_SecureMemcpy(void  *pDestination, size_t dstLength, PCVOID pSource, size_t srcLength)
{
    if ( (pDestination == nullptr) || (pSource == nullptr) )
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ( dstLength < srcLength )
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if(pDestination != pSource)
    {
        memcpy(pDestination, pSource, srcLength);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_SecureFileOpen(
    FILE       **ppFile,
    const char *filename,
    const char *mode)
{
    PFILE fp;

    if ((ppFile == nullptr) || (filename == nullptr) || (mode == nullptr))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    fp = fopen(filename, mode);

    if (fp == nullptr)
    {
        *ppFile = nullptr;
        return MOS_STATUS_FILE_OPEN_FAILED;
    }
    else
    {
        *ppFile = fp;
        return MOS_STATUS_SUCCESS;
    }
}

int32_t MOS_SecureStringPrint(char  *buffer, size_t bufSize, size_t length, const char * const format, ...)
{
    int32_t iRet = -1;
    va_list var_args;

    if((buffer == nullptr) || (format == nullptr) || (bufSize < length))
    {
        return iRet;
    }

    va_start(var_args, format);

    iRet = vsnprintf(buffer, length, format, var_args);

    va_end(var_args);

    return iRet;
}

MOS_STATUS MOS_SecureVStringPrint(char  *buffer, size_t bufSize, size_t length, const char * const format, va_list var_args)
{
    if((buffer == nullptr) || (format == nullptr) || (bufSize < length))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    vsnprintf(buffer, length, format, var_args);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_GetFileSize(
    HANDLE              hFile,
    uint32_t            *lpFileSizeLow,
    uint32_t            *lpFileSizeHigh)
{
    struct stat     Buf;
    MOS_UNUSED(lpFileSizeHigh);

    if((hFile == nullptr) || (lpFileSizeLow == nullptr))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ( (fstat((intptr_t)hFile, &Buf)) < 0 )
    {
        *lpFileSizeLow = 0;
        return MOS_STATUS_INVALID_FILE_SIZE;
    }
    *lpFileSizeLow  = (uint32_t)Buf.st_size;

    //to-do, lpFileSizeHigh store high 32-bit of File size
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_CreateDirectory(
    char * const       lpPathName)
{
    uint32_t   mode;
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    MOS_OS_CHK_NULL(lpPathName);

    // Set read/write access right for usr/group.
    mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP;
    if (mkdir(lpPathName, mode) < 0 &&
        errno != EEXIST) // Directory already exists, don't return failure in this case.
    {
        MOS_OS_ASSERTMESSAGE("Failed to create the directory '%s'. Error = %s", lpPathName, strerror(errno));
        eStatus = MOS_STATUS_DIR_CREATE_FAILED;
        goto finish;
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS MOS_CreateFile(
    PHANDLE             pHandle,
    char * const        lpFileName,
    uint32_t            iOpenFlag)
{
    int32_t             iFileDescriptor;
    uint32_t            mode;

    if((lpFileName == nullptr) || (pHandle == nullptr))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    //set read/write access right for usr/group, mMode only takes effect when
    //O_CREAT is set
    mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;
    if ( (iFileDescriptor = open(lpFileName, iOpenFlag, mode)) < 0 )
    {
        *pHandle = (HANDLE)((intptr_t) iFileDescriptor);
        return MOS_STATUS_INVALID_HANDLE;
    }

    *pHandle = (HANDLE)((intptr_t) iFileDescriptor);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_ReadFile(
    HANDLE  hFile,
    void    *lpBuffer,
    uint32_t bytesToRead,
    uint32_t *pBytesRead,
    void    *lpOverlapped)
{
    size_t  nNumBytesToRead;
    ssize_t nNumBytesRead;
    MOS_UNUSED(lpOverlapped);

    if((hFile == nullptr) || (lpBuffer == nullptr) || (pBytesRead == nullptr))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    nNumBytesToRead   = (size_t)bytesToRead;
    nNumBytesRead     = 0;

    //To-do: process lpOverlapped

    if ((nNumBytesRead = read((intptr_t)hFile, lpBuffer, nNumBytesToRead)) < 0)
    {
        *pBytesRead = 0;
        return MOS_STATUS_FILE_READ_FAILED;
    }

    *pBytesRead = (uint32_t)nNumBytesRead;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_WriteFile(
    HANDLE    hFile,
    void      *lpBuffer,
    uint32_t  bytesToWrite,
    uint32_t  *pbytesWritten,
    void      *lpOverlapped)
{
    size_t    nNumBytesToWrite;
    ssize_t   nNumBytesWritten;
    MOS_UNUSED(lpOverlapped);

    if((hFile == nullptr) || (lpBuffer == nullptr) || (pbytesWritten == nullptr))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    nNumBytesToWrite = (size_t)bytesToWrite;
    nNumBytesWritten = 0;

    //To-do, process lpOverlapped

    if ((nNumBytesWritten = write((intptr_t)hFile, lpBuffer, nNumBytesToWrite)) < 0)
    {
        *pbytesWritten = 0;
        return MOS_STATUS_FILE_WRITE_FAILED;
    }

    *pbytesWritten = (uint32_t)nNumBytesWritten;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_SetFilePointer(
    HANDLE        hFile,
    int32_t       lDistanceToMove,
    int32_t       *lpDistanceToMoveHigh,
    int32_t       dwMoveMethod)
{
    int32_t     iOffSet;
    int32_t     iCurPos;

    if(hFile == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (lpDistanceToMoveHigh == nullptr)
    {
        iOffSet = lDistanceToMove;
    }
    else
    {
        //to-do, let lpDistanceToMoveHigh and lDistanceToMove form a 64-bit iOffSet
        iOffSet = (int32_t)lDistanceToMove;
    }

    if ((iCurPos = lseek((intptr_t)hFile, iOffSet, dwMoveMethod)) < 0)
    {
        return MOS_STATUS_SET_FILE_POINTER_FAILED;
    }

    return MOS_STATUS_SUCCESS;
}

int32_t MOS_CloseHandle(HANDLE hObject)
{
    int32_t iRet = false;

    if(hObject != nullptr)
    {
        close((intptr_t)hObject);
        iRet = true;
    }

    return iRet;
}

//library
MOS_STATUS MOS_LoadLibrary(const char * const lpLibFileName, PHMODULE phModule)
{
    if (lpLibFileName == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    *phModule = dlopen((const char *)lpLibFileName, RTLD_LAZY);

    return ((*phModule != nullptr) ? MOS_STATUS_SUCCESS : MOS_STATUS_LOAD_LIBRARY_FAILED);
}

int32_t MOS_FreeLibrary (HMODULE hLibModule)
{
    uint32_t iRet = 10;   // Initialize to some non-zero value

    if(hLibModule != nullptr)
    {
        iRet = dlclose(hLibModule);
    }
    return (iRet == 0) ? true : false;
}

void  *MOS_GetProcAddress(HMODULE hModule, const char *lpProcName)
{
    void  *pSym = nullptr;

    if (hModule    == nullptr ||
        lpProcName == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Invalid parameter.");
    }
    else
    {
        pSym = dlsym(hModule, lpProcName);
    }

    return pSym;
}

int32_t MOS_GetPid()
{
    return(getpid());
}

//Performace
int32_t MOS_QueryPerformanceFrequency(uint64_t *pFrequency)
{
    struct timespec  Res;
    int32_t          iRet;

    if(pFrequency == nullptr)
    {
        return false;
    }

    if ( (iRet = clock_getres(CLOCK_MONOTONIC, &Res)) != 0 )
    {
        return false;
    }

    // resolution (precision) can't be in seconds for current machine and OS
    if (Res.tv_sec != 0)
    {
        return false;
    }
    *pFrequency = (uint64_t)((1000 * 1000 * 1000) / Res.tv_nsec);

    return true;
}

int32_t MOS_QueryPerformanceCounter(uint64_t *pPerformanceCount)
{
    struct timespec     Res;
    struct timespec     t;
    int32_t             iRet;

    if(pPerformanceCount == nullptr)
    {
        return false;
    }
    if ( (iRet = clock_getres (CLOCK_MONOTONIC, &Res)) != 0 )
    {
        return false;
    }
    if (Res.tv_sec != 0)
    { // resolution (precision) can't be in seconds for current machine and OS
        return false;
    }
    if( (iRet = clock_gettime(CLOCK_MONOTONIC, &t)) != 0)
    {
        return false;
    }
    *pPerformanceCount = (uint64_t)((1000 * 1000 * 1000 * t.tv_sec + t.tv_nsec) / Res.tv_nsec);

    return true;
}

void MOS_Sleep(uint32_t mSec)
{
    usleep(1000 * mSec);
}

//User Feature
/*----------------------------------------------------------------------------
| Name      : _UserFeature_FindKey
| Purpose   : This function finds a key in keys linked list according to key
|             name.
| Arguments : pKeyList   [in] Key Linked list.
|             pcKeyName  [in] Name to the key to find.
| Returns   : Matched uf_key data. otherwise return NULL.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_UF_KEY* _UserFeature_FindKey(MOS_PUF_KEYLIST pKeyList, char * const pcKeyName)
{
    int32_t           iResult;
    MOS_PUF_KEYLIST   pTempNode;

    iResult = -1;

    for(pTempNode = pKeyList; pTempNode; pTempNode = pTempNode->pNext)
    {
        iResult = strcmp(pTempNode->pElem->pcKeyName, pcKeyName);
        if ( iResult == 0 )
        {
            return pTempNode->pElem;
        }
    }
    return nullptr; //not found
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_FindValue
| Purpose   : Find a value in values array of a key. Return position in values
|             array
| Arguments : UFKey        [in] Searched Key node.
|             pcValueName  [in] Value name.
| Returns   : Matched value No. if it can be found, otherwise, return
|             NOT_FOUND(-1);
| Comments  :
\---------------------------------------------------------------------------*/
static int32_t _UserFeature_FindValue(MOS_UF_KEY UFKey, char * const pcValueName)
{
    int32_t iResult;
    int32_t i;

    iResult = -1;

    for ( i = 0; i < (int32_t)UFKey.ulValueNum; i++ )
    {
        iResult = strcmp(UFKey.pValueArray[i].pcValueName, pcValueName);
        if ( iResult == 0 )
        {
            return i;
        }
    }
    return NOT_FOUND;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_Add
| Purpose   : Add new key to keys' linked list.
| Arguments : pKeyList       [in] Key linked list.
|             NewKey         [in] Added new key.
| Returns   : MOS_STATUS_SUCCESS            success
|             MOS_STATUS_INVALID_PARAMETER  invalid NewKey
|             MOS_STATUS_NO_SPACE           no space left for allocate
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_Add(MOS_PUF_KEYLIST *pKeyList, MOS_UF_KEY *NewKey)
{
    MOS_UF_KEYNODE  *pNewNode;
    MOS_UF_KEYNODE  *pTempNode;
    MOS_UF_KEYNODE  *pStartNode;

    pNewNode   =  nullptr;
    pTempNode  =  nullptr;
    pStartNode =  *pKeyList;

    if ( NewKey == nullptr )
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    pNewNode = (MOS_UF_KEYNODE*)MOS_AllocMemory(sizeof(MOS_UF_KEYNODE));
    if (pNewNode == nullptr)
    {
        return MOS_STATUS_NO_SPACE;
    }
    pNewNode->pElem = NewKey;

    if (*pKeyList == nullptr ) // the key list is empty
    {
        pNewNode->pNext = nullptr;
        (*pKeyList) = pNewNode;
    }
    else // the key list is not empty, append to the front
    {
        pTempNode = pStartNode->pNext;
        pStartNode->pNext = pNewNode;
        pNewNode->pNext = pTempNode;
    }
    return MOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_Set
| Purpose   : This function set a key to the key list.
| Arguments : pKeyList          [in] Key linked list.
|             NewKey            [in] Set key content.
| Returns   : MOS_STATUS_SUCCESS      Operation success.
|             MOS_STATUS_UNKNOWN      Can't find key in User Feature File.
|             MOS_STATUS_NO_SPACE     no space left for allocate
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_Set(MOS_PUF_KEYLIST *pKeyList, MOS_UF_KEY NewKey)
{
    int32_t       iPos;
    MOS_UF_VALUE  *pValueArray;
    MOS_UF_KEY    *Key;
    void          *ulValueBuf;

    iPos         = -1;
    pValueArray  = nullptr;

    if ( (Key = _UserFeature_FindKey(*pKeyList, NewKey.pcKeyName)) == nullptr )
    {
        // can't find key in File
        return MOS_STATUS_UNKNOWN;
    }

    // Prepare the ValueBuff of the NewKey
    if ((ulValueBuf = MOS_AllocMemory(NewKey.pValueArray[0].ulValueLen)) == nullptr)
    {
         return MOS_STATUS_NO_SPACE;
    }

    if ( (iPos = _UserFeature_FindValue(*Key, NewKey.pValueArray[0].pcValueName)) == NOT_FOUND)
    {
        //not found, add a new value to key struct.
        //reallocate memory for appending this value.
        pValueArray = (MOS_UF_VALUE*)MOS_AllocMemory(sizeof(MOS_UF_VALUE)*(Key->ulValueNum+1));
        if (pValueArray == nullptr)
        {
            MOS_FreeMemory(ulValueBuf);
            return MOS_STATUS_NO_SPACE;
        }

        MOS_SecureMemcpy(pValueArray,
                        sizeof(MOS_UF_VALUE)*(Key->ulValueNum),
                        Key->pValueArray,
                        sizeof(MOS_UF_VALUE)*(Key->ulValueNum));

        MOS_FreeMemory(Key->pValueArray);

        Key->pValueArray = pValueArray;

        iPos = Key->ulValueNum;
        MOS_SecureStrcpy(Key->pValueArray[Key->ulValueNum].pcValueName,
            MAX_USERFEATURE_LINE_LENGTH,
            NewKey.pValueArray[0].pcValueName);
        Key->ulValueNum ++;
    }
    else
    {
        //if found, the previous value buffer needs to be freed before reallocating
        MOS_FreeMemory(Key->pValueArray[iPos].ulValueBuf);
    }

    Key->pValueArray[iPos].ulValueLen  = NewKey.pValueArray[0].ulValueLen;
    Key->pValueArray[iPos].ulValueType = NewKey.pValueArray[0].ulValueType;
    Key->pValueArray[iPos].ulValueBuf  = ulValueBuf;

    MOS_ZeroMemory(Key->pValueArray[iPos].ulValueBuf, NewKey.pValueArray[0].ulValueLen);

    MOS_SecureMemcpy(Key->pValueArray[iPos].ulValueBuf,
                     NewKey.pValueArray[0].ulValueLen,
                     NewKey.pValueArray[0].ulValueBuf,
                     NewKey.pValueArray[0].ulValueLen);

    return MOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_Query
| Purpose   : This function query a key's value and return matched key node
|             content just with matched value content.
| Arguments : pKeyList      [in] Key linked list.
|             NewKey        [in] New key content with matched value.
| Returns   : MOS_STATUS_SUCCESS         Operation success.
|             MOS_STATUS_UNKNOWN         Can't find key or value in User Feature File.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_Query(MOS_PUF_KEYLIST pKeyList, MOS_UF_KEY *NewKey)
{
    int32_t       iPos;
    MOS_UF_VALUE  *pValueArray;
    MOS_UF_KEY    *Key;

    iPos         = -1;
    pValueArray  = nullptr;

    // can't find key in user feature
    if ( (Key = _UserFeature_FindKey(pKeyList, NewKey->pcKeyName)) == nullptr )
    {
        return MOS_STATUS_UNKNOWN;
    }

    // can't find Value in the key
    if ( (iPos = _UserFeature_FindValue(*Key, NewKey->pValueArray[0].pcValueName)) == NOT_FOUND)
    {
        return MOS_STATUS_UNKNOWN;
    }

    //get key content from user feature
    MOS_SecureMemcpy(NewKey->pValueArray[0].ulValueBuf,
                     Key->pValueArray[iPos].ulValueLen,
                     Key->pValueArray[iPos].ulValueBuf,
                     Key->pValueArray[iPos].ulValueLen);

    NewKey->pValueArray[0].ulValueLen    =  Key->pValueArray[iPos].ulValueLen;
    NewKey->pValueArray[0].ulValueType   =  Key->pValueArray[iPos].ulValueType;

    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS _UserFeature_ReadNextTokenFromFile(FILE *pFile, const char *szFormat, char  *szToken)
{
    size_t nTokenSize = 0;

    // Reads the next token from the given pFile.
    if (fscanf(pFile, szFormat, szToken) <= 0)
    {
        MOS_OS_VERBOSEMESSAGE("Failed reading the next token from the user feature file. This is probably because the token does not exist in the user feature file.");
        return MOS_STATUS_FILE_READ_FAILED;
    }

    // Converts to Unix-style line endings to prevent compatibility problems.
    nTokenSize = strnlen(szToken, MAX_USERFEATURE_LINE_LENGTH);
    if (szToken[nTokenSize-1] == '\r')
    {
        szToken[nTokenSize-1] = '\0';
    }

    return MOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_DumpFile
| Purpose   : This function read the whole User Feature File and dump User Feature File
|             data to key linked list.
| Arguments : szFileName         [in]  User Feature File name.
|             pKeyList           [out] Key Linked list.
| Returns   : MOS_STATUS_SUCCESS           Operation success.
|             MOS_STATUS_USER_FEATURE_KEY_READ_FAILED  User Feature File can't be open as read.
|             MOS_STATUS_NO_SPACE          no space left for allocate
|             MOS_STATUS_UNKNOWN           unknown user feature type found in User Feature File
|             MOS_STATUS_INVALID_PARAMETER unknown items found in User Feature File
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_DumpFile(const char * const szFileName, MOS_PUF_KEYLIST* pKeyList)
{
    MOS_UF_KEY      *CurKey;
    MOS_UF_VALUE    *CurValue;
    char            szTmp[MAX_USERFEATURE_LINE_LENGTH];
    int32_t         iResult;
    size_t          nSize;
    int32_t         bFirst;
    int32_t         iCount;
    PFILE           File;
    int32_t         bEmpty;
    int32_t         iCurId;
    MOS_STATUS      eStatus;
    char            *tmpChar; // Used in the 64-bit case to read uint64_t

    CurValue  =  nullptr;
    nSize     =  0;
    bFirst    =  1;    // 1 stand for "is the first key".
    iCount    =  0;
    File      =  nullptr;
    bEmpty    =  0;
    iCurId    =  0;
    eStatus  =  MOS_STATUS_SUCCESS;

    CurKey = (MOS_UF_KEY*)MOS_AllocMemory(sizeof(MOS_UF_KEY));
    if (CurKey == nullptr)
    {
        return MOS_STATUS_NO_SPACE;
    }
    CurKey->ulValueNum       = 0;
    CurKey->pcKeyName[0]    = '\0';
    CurKey->pValueArray       = nullptr;

    if ( (File = fopen(szFileName, "r")) == nullptr)
    {
        MOS_FreeMemory(CurKey);
        return MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    }
    while (feof(File) != EOF)
    {
        MOS_ZeroMemory(szTmp, MAX_USERFEATURE_LINE_LENGTH*sizeof(char ));
        if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
        {
            break;
        }

        // set szDumpData with extracted File content.
        iResult = strcmp(szTmp, UF_KEY_ID);
        if ( iResult == 0 )
        {
            // It is a new key starting!
            if (! bFirst )
            {
                // Add last key struct to contents when the key is not first.
                // otherwise, continue to load key struct data.
                CurKey->pValueArray   = CurValue;
                CurKey->ulValueNum   = iCount;
                if(_UserFeature_Add(pKeyList, CurKey) != MOS_STATUS_SUCCESS)
                {
                    // if the CurKey didn't be added in pKeyList, free it.
                    MOS_FreeMemory(CurKey);
                }
                CurKey = (MOS_UF_KEY*)MOS_AllocMemory(sizeof(MOS_UF_KEY));
                if (CurKey == nullptr)
                {
                    eStatus = MOS_STATUS_NO_SPACE;
                    break;
                }
            } // if (! bFirst )

            if (fscanf(File, "%x\n", (uint32_t*)&iCurId) <= 0)
            {
                break;
            }

            CurKey->UFKey = (void *)(intptr_t)iCurId;

            MOS_ZeroMemory(szTmp, MAX_USERFEATURE_LINE_LENGTH * sizeof(char));
            if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
            {
                break;
            }

            MOS_SecureStrcpy(CurKey->pcKeyName, MAX_USERFEATURE_LINE_LENGTH, szTmp);
            CurKey->ulValueNum = 0;

            // allocate capability length for valuearray.
            CurValue = (MOS_UF_VALUE*)MOS_AllocMemory(sizeof(MOS_UF_VALUE)*UF_CAPABILITY);
            if (CurValue == nullptr)
            {
                eStatus = MOS_STATUS_NO_SPACE;
                break;
            }
            bFirst = 0;
            iCount = 0;  // next key's array number.
            bEmpty = 1;
        } // if ( iResult == 0 )
        else // not a key
        {
            // Is it a value starting?
            iResult = strcmp(szTmp, UF_VALUE_ID);
            if ( iResult == 0 )
            {
                if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
                {
                    break;
                }

                if (CurValue == nullptr)
                {
                    break;
                }

                // Out of bounds technically based on how much memory we allocated
                if (iCount < 0 || iCount >= UF_CAPABILITY)
                {
                    eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
                    break;
                }

                // Load value name;
                MOS_SecureStrcpy(CurValue[iCount].pcValueName, MAX_USERFEATURE_LINE_LENGTH, szTmp);

                // Load value type
                if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
                {
                    break;
                }

                CurValue[iCount].ulValueType = atoi(szTmp);

                // Load value buffer.
                switch ( CurValue[iCount].ulValueType )
                {
                case UF_DWORD: // 32-bit
                    if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
                    {
                        break;
                    }

                    CurValue[iCount].ulValueLen = sizeof(uint32_t);
                    CurValue[iCount].ulValueBuf = MOS_AllocMemory(sizeof(uint32_t));
                    if(CurValue[iCount].ulValueBuf == nullptr)
                    {
                        eStatus = MOS_STATUS_NO_SPACE;
                        break;
                    }
                    *(uint32_t*)(CurValue[iCount].ulValueBuf) = atoi(szTmp);
                    break;
                case UF_QWORD: // 64-bit
                    if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
                    {
                        break;
                    }

                    CurValue[iCount].ulValueLen = sizeof(uint64_t);
                    CurValue[iCount].ulValueBuf = MOS_AllocMemory(sizeof(uint64_t));
                    if(CurValue[iCount].ulValueBuf == nullptr)
                    {
                        eStatus = MOS_STATUS_NO_SPACE;
                        break;
                    }
                    tmpChar = &szTmp[0];
                    *(uint64_t*)(CurValue[iCount].ulValueBuf) = strtoll(tmpChar,&tmpChar,0);
                    break;
                case UF_SZ:
                case UF_MULTI_SZ:
                    if (MOS_FAILED(_UserFeature_ReadNextTokenFromFile(File, MAX_UF_LINE_STRING_FORMAT, szTmp)))
                    {
                        break;
                    }

                    nSize = strlen(szTmp);
                    CurValue[iCount].ulValueLen = (nSize+1)*sizeof(char );
                    CurValue[iCount].ulValueBuf = MOS_AllocMemory(nSize+1);
                    if(CurValue[iCount].ulValueBuf == nullptr)
                    {
                        eStatus = MOS_STATUS_NO_SPACE;
                        break;
                    }
                    MOS_ZeroMemory(CurValue[iCount].ulValueBuf, nSize+1);
                    MOS_SecureMemcpy(CurValue[iCount].ulValueBuf, nSize, szTmp, nSize);
                    break;
                default:
                    eStatus = MOS_STATUS_UNKNOWN;
                }
                if (eStatus != MOS_STATUS_SUCCESS)
                {
                    break;
                }

                iCount ++; // do the error checking near the top

            } // if ( iResult == 0 )
            else   // It is not a value starting, it's bad User Feature File.
            {
                int32_t iResult = strcmp(szTmp, "");
                if ( !iResult )
                {
                    continue;
                }
                else
                {
                    eStatus =  MOS_STATUS_INVALID_PARAMETER;
                    break;
                }
            } // else ( iResult == 0 )
        }
    } // while (feof(File) != EOF)

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        if ( bEmpty && (strlen(CurKey->pcKeyName) > 0) &&
            (CurKey->ulValueNum == 0) )
        {
            CurKey->pValueArray   = CurValue;
            CurKey->ulValueNum   = iCount;
            if(_UserFeature_Add(pKeyList, CurKey) != MOS_STATUS_SUCCESS)
            {
                // if the CurKey didn't be added in pKeyList, free it.
                for (uint32_t i = 0; i < iCount; i++)
                {
                    if (CurValue)
                    {
                        MOS_FreeMemory(CurValue[i].ulValueBuf);
                    }
                }
                MOS_FreeMemory(CurKey);
            }
        }
        else
        {
            for (uint32_t i = 0; i < iCount; i++)
            {
                if (CurValue)
                {
                    MOS_FreeMemory(CurValue[i].ulValueBuf);
                }
            }
            MOS_FreeMemory(CurKey);
        }
    }
    else
    {
        for (uint32_t i = 0; i < iCount; i++)
        {
            if (CurValue)
            {
                MOS_FreeMemory(CurValue[i].ulValueBuf);
            }
        }
        MOS_FreeMemory(CurKey);
    }
    fclose(File);
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_DumpDataToFile
| Purpose   : This function dump key linked list data to File.
| Arguments : szFileName             [in] A handle to the File.
|             pKeyList               [in] Reserved, any LPDWORD type value.
| Returns   : MOS_STATUS_SUCCESS                        Operation success.
|             MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED  File can't be written.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_DumpDataToFile(const char *szFileName, MOS_PUF_KEYLIST pKeyList)
{
    int32_t           iResult;
    PFILE             File;
    MOS_PUF_KEYLIST   pKeyTmp;
    int32_t           j;

    File = fopen(szFileName, "w+");
    if ( !File )
    {
        return MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
    }

    for (pKeyTmp = pKeyList; pKeyTmp; pKeyTmp = pKeyTmp->pNext)
    {
        fprintf(File, "%s\n", UF_KEY_ID);
        fprintf(File,  "\t0x%.8x\n", (uint32_t)(uintptr_t)pKeyTmp->pElem->UFKey);
        fprintf(File,  "\t%s\n", pKeyTmp->pElem->pcKeyName);
        for ( j = 0; j < (int32_t)pKeyTmp->pElem->ulValueNum; j ++ )
        {
            fprintf(File, "\t\t%s\n", UF_VALUE_ID);
            if ( strlen(pKeyTmp->pElem->pValueArray[j].pcValueName) > 0 )
            {
                fprintf(File, "\t\t\t%s\n",
                    pKeyTmp->pElem->pValueArray[j].pcValueName);
            }
            fprintf(File, "\t\t\t%d\n", pKeyTmp->pElem->pValueArray[j].ulValueType);
            if (pKeyTmp->pElem->pValueArray[j].ulValueBuf != nullptr)
            {
                switch (pKeyTmp->pElem->pValueArray[j].ulValueType)
                {
                case UF_SZ:
                    fprintf(File,  "\t\t\t%s\n",
                        (char *)(pKeyTmp->pElem->pValueArray[j].ulValueBuf));
                    break;
                case UF_DWORD:
                case UF_QWORD:
                    fprintf(File, "\t\t\t%d\n",
                        *(uint32_t*)(pKeyTmp->pElem->pValueArray[j].ulValueBuf));
                    break;
                default:
                    fprintf(File, "\t\t\t%s\n",
                        (char *)(pKeyTmp->pElem->pValueArray[j].ulValueBuf));
                    break;
                } //switch (pKeyTmp->pElem->pValueArray[j].ulValueType)
            }
        } // for ( j = 0; j < (int32_t)pKeyTmp->pElem->ulValueNum; j ++ )
    } //for (pKeyTmp = pKeyList; pKeyTmp; pKeyTmp = pKeyTmp->pNext)
    fclose(File);
    MOS_UserFeatureNotifyChangeKeyValue(nullptr, false, nullptr, true);

    return MOS_STATUS_SUCCESS;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_FreeKeyList
| Purpose   : Free key list
| Arguments : pKeyList           [in] key list to be free.
| Returns   : None
| Comments  :
\---------------------------------------------------------------------------*/
static void _UserFeature_FreeKeyList(MOS_PUF_KEYLIST pKeyList)
{
    MOS_PUF_KEYLIST     pKeyTmp;
    MOS_PUF_KEYLIST     pKeyTmpNext;
    uint32_t            i;

    pKeyTmp = pKeyList;
    while(pKeyTmp)
    {
        pKeyTmpNext = pKeyTmp->pNext;
        for(i=0;i<pKeyTmp->pElem->ulValueNum;i++)
        {
            MOS_FreeMemory(pKeyTmp->pElem->pValueArray[i].ulValueBuf);
        }
        MOS_FreeMemory(pKeyTmp->pElem->pValueArray);
        MOS_FreeMemory(pKeyTmp->pElem);
        MOS_FreeMemory(pKeyTmp);
        pKeyTmp = pKeyTmpNext;
    }
    return;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_SetValue
| Purpose   : Modify or add a value of the specified user feature key.
| Arguments : strKey         [in] Pointer to user feature key name.
|             pcValueName    [in] Pointer to a string containing the name of
|                                 the value to set. If a value with this name
|                                 is not already present in the key, the
|                                 function adds it to the key.
|             uiValueType    [in] Type of information to be stored.
|             szValueData    [in] Pointer to a null-terminated string
|                                 containing the data to set for the default
|                                 value of the specified key
|             uiValueDataLen [in] Size of the string pointed to by the
|                                 szValueData parameter, not including the
|                                 terminating null character, in bytes
| Returns   : MOS_STATUS_SUCCESS           function success
|             MOS_STATUS_INVALID_PARAMETER invalid paramater
|             MOS_STATUS_USER_FEATURE_KEY_READ_FAILED  User Feature File can't be open as read.
|             MOS_STATUS_NO_SPACE          no space left for allocate
|             MOS_STATUS_UNKNOWN           unknown user feature type found in User Feature File
|             MOS_STATUS_INVALID_PARAMETER unknown items found in User Feature File
|             MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED  User Feature File can't be written.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_SetValue(
    char * const        strKey,
    const char * const  pcValueName,
    uint32_t            uiValueType,
    void                *pData,
    int32_t             nDataSize)
{
    MOS_UF_KEY          NewKey;
    MOS_UF_VALUE        NewValue;
    MOS_STATUS          eStatus;
    MOS_PUF_KEYLIST     pKeyList;

    eStatus   = MOS_STATUS_UNKNOWN;
    pKeyList   = nullptr;

    if ( (strKey== nullptr) || (pcValueName == nullptr) )
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_ZeroMemory(NewValue.pcValueName, MAX_USERFEATURE_LINE_LENGTH);
    MOS_SecureStrcpy(NewValue.pcValueName, MAX_USERFEATURE_LINE_LENGTH, pcValueName);
    NewValue.ulValueType    = uiValueType;
    if( NewValue.ulValueType == UF_DWORD)
    {
        NewValue.ulValueLen = sizeof(uint32_t);
    }
    else
    {
        NewValue.ulValueLen = nDataSize;
    }
    NewValue.ulValueBuf     = pData;

    MOS_ZeroMemory(NewKey.pcKeyName, MAX_USERFEATURE_LINE_LENGTH);
    MOS_SecureStrcpy(NewKey.pcKeyName, MAX_USERFEATURE_LINE_LENGTH, strKey);
    NewKey.pValueArray = &NewValue;
    NewKey.ulValueNum = 1;

    if ( (eStatus = _UserFeature_DumpFile(szUserFeatureFile, &pKeyList)) != MOS_STATUS_SUCCESS )
    {
        MOS_FreeMemory(pKeyList);
        return eStatus;
    }

    if ( ( eStatus = _UserFeature_Set(&pKeyList, NewKey)) == MOS_STATUS_SUCCESS )
    {
        eStatus = _UserFeature_DumpDataToFile(szUserFeatureFile, pKeyList);
    }

    _UserFeature_FreeKeyList(pKeyList);
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_QueryValue
| Purpose   : The QueryValue function retrieves the type and data for a
|             specified value name associated with a special user feature key.
| Arguments : strKey         [in]  Pointer to user feature key name.
|             pcValueName    [in]  Pointer to a string containing the name
|                                  of the value to query.
|             uiValueType    [out] Output Value's type
|             pData          [out] Output value's content
|             nDataSize      [out] Output the size of value's content.
| Returns   : MOS_STATUS_SUCCESS           function success
|             MOS_STATUS_INVALID_PARAMETER invalid paramater
|             MOS_STATUS_USER_FEATURE_KEY_READ_FAILED  User Feature File can't be open as read.
|             MOS_STATUS_NO_SPACE          no space left for allocate
|             MOS_STATUS_UNKNOWN           Can't find key or value in User Feature File.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_QueryValue(
    char * const        strKey,
    const char * const  pcValueName,
    uint32_t            *uiValueType,
    void                *pData,
    int32_t             *nDataSize)
{
    MOS_UF_KEY          NewKey;
    MOS_UF_VALUE        NewValue;
    size_t              nKeyLen, nValueLen;
    MOS_STATUS          eStatus;
    MOS_PUF_KEYLIST     pKeyList;
    char                strTempKey[MAX_USERFEATURE_LINE_LENGTH];
    char                strTempValueName[MAX_USERFEATURE_LINE_LENGTH];

    eStatus   = MOS_STATUS_UNKNOWN;
    pKeyList   = nullptr;

    if ( (strKey == nullptr) || (pcValueName == nullptr))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    MOS_ZeroMemory(NewValue.pcValueName, MAX_USERFEATURE_LINE_LENGTH);
    MOS_SecureStrcpy(NewValue.pcValueName, MAX_USERFEATURE_LINE_LENGTH, pcValueName);
    NewValue.ulValueBuf     = pData;

    MOS_ZeroMemory(NewKey.pcKeyName, MAX_USERFEATURE_LINE_LENGTH);
    MOS_SecureStrcpy(NewKey.pcKeyName, MAX_USERFEATURE_LINE_LENGTH, strKey);
    NewKey.pValueArray = &NewValue;
    NewKey.ulValueNum = 1;

    if ( (eStatus = _UserFeature_DumpFile(szUserFeatureFile, &pKeyList)) == MOS_STATUS_SUCCESS)
    {
        if ( (eStatus = _UserFeature_Query(pKeyList, &NewKey)) == MOS_STATUS_SUCCESS )
        {
            if(uiValueType != nullptr)
            {
                *uiValueType = NewKey.pValueArray[0].ulValueType;
            }
            if (nDataSize != nullptr)
            {
                *nDataSize   = NewKey.pValueArray[0].ulValueLen;
            }
        }
    }
    _UserFeature_FreeKeyList(pKeyList);

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_GetKeyIdbyName
| Purpose   : Get ID of the user feature key bu its name
| Arguments : pcKeyName      [in]  Pointer to user feature key name.
|             pUFKey         [out] A UFKEY pointer to store returned UFKey
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_GetKeyIdbyName(const char  *pcKeyName, void **pUFKey)
{
    MOS_PUF_KEYLIST     pKeyList;
    int32_t             iResult;
    MOS_STATUS          eStatus;
    MOS_PUF_KEYLIST     pTempNode;

    pKeyList   = nullptr;
    iResult    = -1;

    if ( (eStatus = _UserFeature_DumpFile(szUserFeatureFile, &pKeyList)) !=
        MOS_STATUS_SUCCESS )
    {
        MOS_FreeMemory(pKeyList);
        return eStatus;
    }

    eStatus   = MOS_STATUS_INVALID_PARAMETER;

    for(pTempNode=pKeyList; pTempNode; pTempNode=pTempNode->pNext)
    {
        iResult = strcmp(pTempNode->pElem->pcKeyName, pcKeyName);
        if ( iResult == 0 )
        {
            *pUFKey = pTempNode->pElem->UFKey;
            eStatus = MOS_STATUS_SUCCESS;
            break;
        }
    }
    _UserFeature_FreeKeyList(pKeyList);

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : _UserFeature_GetKeyNamebyId
| Purpose   : Get name of the user feature key bu its ID
| Arguments : UFKey      [in]  ID of the user feature key
|             pcKeyName  [out] To store user feature key name.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
static MOS_STATUS _UserFeature_GetKeyNamebyId(void  *UFKey, char  *pcKeyName)
{
    MOS_PUF_KEYLIST     pKeyList;
    MOS_PUF_KEYLIST     pTempNode;
    MOS_STATUS          eStatus;

    pKeyList   = nullptr;

    switch((uintptr_t)UFKey)
    {
    case UFKEY_INTERNAL:
        MOS_SecureStrcpy(pcKeyName, MAX_USERFEATURE_LINE_LENGTH, USER_FEATURE_KEY_INTERNAL);
        eStatus = MOS_STATUS_SUCCESS;
        break;
    case UFKEY_EXTERNAL:
        MOS_SecureStrcpy(pcKeyName, MAX_USERFEATURE_LINE_LENGTH, USER_FEATURE_KEY_EXTERNAL);
        eStatus = MOS_STATUS_SUCCESS;
        break;
    default:
        if ( (eStatus = _UserFeature_DumpFile(szUserFeatureFile, &pKeyList)) !=
            MOS_STATUS_SUCCESS )
        {
            MOS_FreeMemory(pKeyList);
            return eStatus;
        }

        eStatus   = MOS_STATUS_UNKNOWN;

        for(pTempNode=pKeyList;pTempNode;pTempNode=pTempNode->pNext)
        {
            if(pTempNode->pElem->UFKey == UFKey)
            {
                MOS_SecureStrcpy(pcKeyName, MAX_USERFEATURE_LINE_LENGTH, pTempNode->pElem->pcKeyName);
                eStatus = MOS_STATUS_SUCCESS;
                break;
            }
        }
        _UserFeature_FreeKeyList(pKeyList);
        break;
    }

    return eStatus;

}

/*----------------------------------------------------------------------------
| Name      : MOS_CheckMountStatus
| Purpose   : check mount status
| Arguments : pKeyWord   [in]  Keyword for the Mountpoint
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_CheckMountStatus(char  *pKeyWord)
{
    char        sPartitionPath[MAX_UF_PATH] = {'\0'};
    char        sMountPoint[MAX_UF_PATH] = {'\0'};
    char        sSystemType[MAX_UF_PATH] = {'\0'};
    char        sTemp0[MAX_UF_PATH] = {'\0'};
    char        sTemp1[MAX_UF_PATH] = {'\0'};
    char        sTemp2[MAX_UF_PATH] = {'\0'};
    FILE*       file;
    MOS_STATUS  eStatus = MOS_STATUS_UNKNOWN;

    file = fopen("/proc/mounts", "r");
    MOS_OS_CHK_NULL(file);
    MOS_OS_CHK_NULL(pKeyWord);

    while( fscanf( file, "%255s %255s %255s %255s %255s %255s\n", sPartitionPath, sMountPoint, sSystemType, sTemp0, sTemp1, sTemp2 ) > 0 )
    {
        if( strcmp(sSystemType, pKeyWord) == 0 )
        {
            eStatus = MOS_STATUS_SUCCESS;
            break;
        }
        if( strcmp(sMountPoint, pKeyWord) == 0 )
        {
            eStatus = MOS_STATUS_SUCCESS;
            break;
        }
    }
finish:
    if (file != nullptr)
    {
        fclose(file);
    }
    return eStatus;
}
#ifdef ANDROID
/*----------------------------------------------------------------------------
| Name      : MOS_Strip_Chars
| Purpose   : Strip some characters from a string
| Arguments : pstorestr  [out] To store a striped string.
|             pstring    [in]  string needed to be striped.
|             pchars     [in]  stripped keyword.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is an error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_StripChars(char  *pstorestr, const char *pstring, const char *pchars)
{
    int32_t       counter = 0;
    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pstring);
    MOS_OS_CHK_NULL(pchars);

    for ( ; *pstring; pstring++)
    {
        if (!strchr(pchars, *pstring))
        {
            pstorestr[ counter ] = *pstring;
            ++ counter;
        }
    }
    pstorestr[counter] = 0;
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_InitAndroidPropInfo
| Purpose   : Init Android Property Info
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_InitAndroidPropInfo()
{
    char                            prop[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_STATUS                      eStatus = MOS_STATUS_UNKNOWN;
    int32_t                         ret;
    int32_t                         iUFKeyEnable;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;

    eStatus = MOS_CheckMountStatus((char *)"/system");
    if (eStatus == MOS_STATUS_SUCCESS)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        if (property_get("debug.LibVa.RegKeyEnable", prop, nullptr) > 0)
        {
            if (sscanf(prop, "%d\n", &iUFKeyEnable) > 0)
            {
                if (iUFKeyEnable == 1)
                {
                    eStatus = MOS_STATUS_SUCCESS;
                }
            }
        }
    }
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_GetAndroidPropPath
| Purpose   : get AndroidProp path
| Arguments : pPath         [out] store the path
|             pUserFeature  [in]  User Feature data for input User Feature Key
|             FuncType      [in]  Read/Write Type
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_GetAndroidPropPath(
    char                        *pPath,
    PMOS_USER_FEATURE_VALUE     pUserFeature,
    LINUX_UF_FUNC_TYPE          FuncType)
{

    char        sPath[MAX_UF_PATH];
    char        sKey[MAX_UF_PATH];
    char        sFuncType[MAX_UF_PATH];
    char        pPrefixPath[MAX_UF_PATH];
    char        sUFName[MAX_UF_PATH];
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    MOS_OS_CHK_NULL(pUserFeature);
    MOS_OS_CHK_NULL(pPath);

    MOS_ZeroMemory(sPath, MAX_UF_PATH);
    MOS_ZeroMemory(sKey,  MAX_UF_PATH);
    switch(pUserFeature->Type)
    {
    case MOS_USER_FEATURE_TYPE_USER:
        MOS_SecureStrcpy(sKey, MAX_UF_PATH, UFINT_PATH_LINUX);
        break;
    case MOS_USER_FEATURE_TYPE_SYSTEM:
        MOS_SecureStrcpy(sKey, MAX_UF_PATH, UFEXT_PATH_LINUX);
        break;
    default:
        MOS_SecureStrcpy(sKey, MAX_UF_PATH, UFINT_PATH_LINUX);
        break;
    }// switch

    MOS_ZeroMemory(sFuncType, MAX_UF_PATH);
    switch(FuncType)
    {
    case LINUX_UF_FUNCTYPE_READ:
        MOS_SecureStrcpy(sFuncType, MAX_UF_PATH, pUserFeature->pcPath);
        break;
    case LINUX_UF_FUNCTYPE_WRITE:
        MOS_SecureStrcpy(sFuncType, MAX_UF_PATH, pUserFeature->pcWritePath);
        break;
    default:
        MOS_SecureStrcpy(sFuncType, MAX_UF_PATH, pUserFeature->pcPath);
        break;
    }// switch

    MOS_OS_CHK_STATUS(MOS_StripChars(sUFName,pUserFeature->pValueName," "));
    snprintf(
        sPath,
        sizeof(sPath),
        "%s.%s.%s",
        sKey,
        sFuncType,
        sUFName);

    MOS_SecureMemcpy(pPath, strlen(sPath)+1, sPath, strlen(sPath)+1);
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_AndroidPropOpenKey
| Purpose   : open a file for AndroidProp
| Arguments : pPath       [in]
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_AndroidPropOpenKey(
    char                *pPath)
{
    MOS_STATUS      eStatus = MOS_STATUS_FILE_OPEN_FAILED;
    char            prop[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_OS_CHK_NULL(pPath);

    if (property_get(pPath, prop, nullptr) > 0)
    {
        eStatus = MOS_STATUS_SUCCESS;
    }
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_AndroidPropWriteValue
| Purpose   : write a AndroidProp User Feature Data Value
| Arguments : pPath       [in]
|             pUserData   [in]  user feature key write value
|             ValueType   [in]  Type of Data
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_AndroidPropWriteValue(
    char                                *pPath,
    PMOS_USER_FEATURE_VALUE_DATA        pUserData,
    MOS_USER_FEATURE_VALUE_TYPE         ValueType)
{
    MOS_STATUS  eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
    char        prop[MOS_USER_CONTROL_MAX_DATA_SIZE];
    int32_t     ret = 0;
    MOS_OS_CHK_NULL(pPath);
    MOS_OS_CHK_NULL(pUserData);

    switch(ValueType)
    {
     case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
        ret = snprintf(prop, sizeof(prop), "%d\n", pUserData->bData);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT32:
        ret = snprintf(prop, sizeof(prop), "%d\n", pUserData->i32Data);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
        ret = snprintf(prop, sizeof(prop), "%u\n", pUserData->u32Data);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_INT64:
        ret = snprintf(prop, sizeof(prop), "%" MOSd64 "\n", pUserData->i64Data);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
        ret = snprintf(prop, sizeof(prop), "%" MOSu64 "\n", pUserData->u64Data);
        break;
    case MOS_USER_FEATURE_VALUE_TYPE_STRING:
        if ((pUserData->StringData.pStringData != nullptr) && (strlen(pUserData->StringData.pStringData) != 0))
        {
            ret = snprintf(prop, sizeof(prop), "%s\n", pUserData->StringData.pStringData);
        }
        break;
    default:
        break;
    }
    if (ret > 0)
    {
        ret = property_set(pPath, prop);
        if (ret == 0)
        {
            eStatus = MOS_STATUS_SUCCESS;
        }
    }
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_AndroidPropCreateKey
| Purpose   : set default value to AndroidProp User Feature Key according to userfeature
| Arguments : pUserFeature   [in]  value for ceated key
|             FuncType       [in]  Type of function
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_AndroidPropCreateKey(
    PMOS_USER_FEATURE_VALUE     pUserFeature,
    LINUX_UF_FUNC_TYPE          FuncType)
{
    char                        sPath[MAX_UF_PATH];
    MOS_STATUS                  eStatus = MOS_STATUS_DIR_CREATE_FAILED;
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_USER_FEATURE_VALUE_TYPE ValueType = MOS_USER_FEATURE_VALUE_TYPE_INVALID;
    char                        prop[MOS_USER_CONTROL_MAX_DATA_SIZE];
    int32_t                     ret;

    MOS_OS_CHK_NULL(pUserFeature);
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));

    // create key
    MOS_ZeroMemory(sPath, MAX_UF_PATH);
    MOS_OS_CHK_STATUS(MOS_GetAndroidPropPath(sPath, pUserFeature, FuncType));
    // set the default Data and Type value according to default value and type in pUserFeature.
    MOS_OS_CHK_STATUS(MOS_AndroidPropWriteValue(sPath, &pUserFeature->Value, pUserFeature->ValueType));
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_AndroidPropReadValue
| Purpose   : read a AndroidProp User Feature Data Value
| Arguments : pPath       [in]
|             pUserData   [out] To store user feature key value.
|             ValueType   [in]  Type of Data
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_AndroidPropReadValue(
    char                            *pPath,
    PMOS_USER_FEATURE_VALUE_DATA    pUserData,
    MOS_USER_FEATURE_VALUE_TYPE     ValueType)
{
    MOS_USER_FEATURE_VALUE_DATA UserData;
    char                        pcTmpStr[MOS_USER_CONTROL_MAX_DATA_SIZE];
    MOS_STATUS                  eStatus = MOS_STATUS_USER_FEATURE_KEY_READ_FAILED;
    char                        prop[MOS_USER_CONTROL_MAX_DATA_SIZE];
    int32_t                     ret = 0;

    MOS_OS_CHK_NULL(pPath);
    MOS_OS_CHK_NULL(pUserData);

    MOS_ZeroMemory(&UserData, sizeof(UserData));

    if (property_get(pPath, prop, nullptr) > 0)
    {
        switch(ValueType)
        {
         case MOS_USER_FEATURE_VALUE_TYPE_BOOL:
            ret = sscanf(prop, "%d\n", &pUserData->bData);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_INT32:
            ret = sscanf(prop, "%d\n", &pUserData->i32Data);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
            ret = sscanf(prop, "%u\n", &pUserData->u32Data);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_INT64:
            ret = sscanf(prop, "%" MOSd64 "\n", &pUserData->i64Data);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
            ret = sscanf(prop, "%" MOSu64 "\n", &pUserData->u64Data);
        break;
        case MOS_USER_FEATURE_VALUE_TYPE_STRING:
            if( sscanf( prop, "%s\n", pcTmpStr) > 0 )
            {
                if (strlen(pcTmpStr) > 0)
                {
                    MOS_SafeFreeMemory(pUserData->StringData.pStringData);
                    pUserData->StringData.pStringData = (char *)MOS_AllocAndZeroMemory(strlen(pcTmpStr)+1);
                    MOS_SecureMemcpy(pUserData->StringData.pStringData, strlen(pcTmpStr), pcTmpStr, MOS_MIN(strlen(pcTmpStr), MOS_USER_CONTROL_MAX_DATA_SIZE));
                    pUserData->StringData.uSize = strlen(pcTmpStr);
                    ret = pUserData->StringData.uSize;
                }
            }
            break;
        default:
            break;
        }
    }
    if (ret > 0)
    {
        eStatus = MOS_STATUS_SUCCESS;
    }
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_UserFeatureOpenKey_AndroidProp
| Purpose   : Opens the specified user feature key.
| Arguments : UFKey        [in]  A handle to an open user feature key.
|             lpSubKey     [in]  The name of the user feature subkey to be opened.
|             ulOptions    [in]  This parameter is reserved and must be zero.
|             samDesired   [in]  Reserved, could be any REGSAM type value
|             phkResult    [out] A pointer to a variable that receives a handle
|                                to the opened key.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_UserFeatureOpenKey_AndroidProp(
    void       *UFKey,
    const char *lpSubKey,
    uint32_t   ulOptions,
    uint32_t   samDesired,
    void       **phkResult)
{
    char                            pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    PMOS_USER_FEATURE_VALUE         pUserFeature = nullptr;
    char                            sPath[MAX_UF_PATH];
    LINUX_UF_FUNC_TYPE              FuncType = LINUX_UF_FUNCTYPE_INVALID;
    MOS_UNUSED(UFKey);
    MOS_UNUSED(ulOptions);

    MOS_OS_CHK_NULL(lpSubKey);
    pUserFeature = (PMOS_USER_FEATURE_VALUE)*phkResult;
    MOS_OS_CHK_NULL(pUserFeature);
    if( strcmp(lpSubKey, pUserFeature->pcWritePath) == 0 )
    {
        FuncType = LINUX_UF_FUNCTYPE_WRITE;
    }
    else if( strcmp(lpSubKey, pUserFeature->pcPath) == 0 )
    {
        FuncType = LINUX_UF_FUNCTYPE_READ;
    }
    MOS_ZeroMemory(sPath, MAX_UF_PATH);
    MOS_OS_CHK_STATUS(MOS_GetAndroidPropPath(sPath, pUserFeature, FuncType));
    if (MOS_AndroidPropOpenKey(sPath) != MOS_STATUS_SUCCESS)
    {
        // No Sub Key return directly
        eStatus = MOS_STATUS_FILE_OPEN_FAILED;
        // KEY_WRITE
        if (samDesired == KEY_WRITE)
        {
            if (MOS_AndroidPropCreateKey(pUserFeature, FuncType) != MOS_STATUS_SUCCESS)
            {
                eStatus = MOS_STATUS_DIR_CREATE_FAILED;
            }
        }
        else
        {
            eStatus = MOS_STATUS_FILE_OPEN_FAILED;
        }
    }
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_UserFeatureGetValue_AndroidProp
| Purpose   : Retrieves the type and data for the specified user feature value.
| Arguments : UFKey     [in]  A handle to an open user feature key.
|             lpSubKey  [in]  The name of the user feature key. This key must be a
|                             subkey of the key specified by the UFKey parameter
|             lpValue   [in]  The name of the user feature value.
|             dwFlags   [in]  Reserved, could be any uint32_t type value
|             pdwType   [out] A pointer to a variable that receives a code
|                             indicating the type of data stored in the
|                             specified value.
|             pvData    [out] A pointer to a buffer that receives the value's
|                             data.
|             pcbData   [out] A pointer to a variable that specifies the size
|                             of the buffer pointed to by the pvData parameter,
|                             in bytes.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_UserFeatureGetValue_AndroidProp(
    void       *UFKey,
    const char *lpSubKey,
    const char *lpValue,
    uint32_t   dwFlags,
    uint32_t   *pdwType,
    void       *pvData,
    uint32_t   *pcbData)
{
    char                        pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    int32_t                     dData;
    int32_t                     index = 0;
    PMOS_USER_FEATURE_VALUE     pSettingsValue;
    PMOS_USER_FEATURE_VALUE     pUserFeature = nullptr;
    char                        sPath[MAX_UF_PATH];
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(lpSubKey);
    MOS_UNUSED(lpValue);
    MOS_UNUSED(dwFlags);
    MOS_UNUSED(pdwType);

    if(UFKey == nullptr)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        goto finish;
    }
    pUserFeature = (PMOS_USER_FEATURE_VALUE)UFKey;

    MOS_ZeroMemory(sPath, MAX_UF_PATH);
    MOS_OS_CHK_STATUS(MOS_GetAndroidPropPath(sPath, pUserFeature, LINUX_UF_FUNCTYPE_READ));

    // Read Type
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    if (MOS_AndroidPropReadValue(sPath, &UserFeatureData, pUserFeature->ValueType) == MOS_STATUS_SUCCESS)
    {
        // get key content from user feature
        switch(pUserFeature->ValueType)
        {
        case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
            *(int32_t*)pvData = UserFeatureData.bData;
            *(uint32_t*)pcbData = sizeof(int32_t);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_INT32:
            *(int32_t*)pvData = UserFeatureData.i32Data;
            *(uint32_t*)pcbData = sizeof(int32_t);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_INT64:
            *(int64_t*)pvData = UserFeatureData.i64Data;
            *(uint32_t*)pcbData = sizeof(int64_t);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
            *(uint32_t*)pvData = UserFeatureData.u32Data;
            *(uint32_t*)pcbData = sizeof(uint32_t);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
            *(uint64_t*)pvData = UserFeatureData.u64Data;
            *(uint32_t*)pcbData = sizeof(uint64_t);
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_STRING:
            MOS_SecureMemcpy(pvData, UserFeatureData.StringData.uSize, UserFeatureData.StringData.pStringData, UserFeatureData.StringData.uSize);
            MOS_SafeFreeMemory(UserFeatureData.StringData.pStringData);
            *(uint32_t*)pcbData = UserFeatureData.StringData.uSize;
            break;
        default:
            break;
        }// switch
    }
    else
    {
       eStatus = MOS_STATUS_FILE_NOT_FOUND;
    }
finish:
    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_UserFeatureSetValueEx_AndroidProp
| Purpose   : Sets the data and type of a specified value under a user feature key.
| Arguments : UFKey        [in] A handle to an open user feature key.
|             lpValueName  [in] The name of the user feature value.
|             Reserved     [in] This parameter is reserved and must be NULL.
|             dwType       [in] The type of data pointed to by the lpData
|                               parameter.
|             lpData       [in] The data to be stored.
|             cbData       [in] The size of the information pointed to by the
|                               lpData parameter, in bytes.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_UserFeatureSetValueEx_AndroidProp(
    void            *UFKey,
    const char      *lpValueName,
    uint32_t        Reserved,
    uint32_t        dwType,
    uint8_t         *lpData,
    uint32_t        cbData)
{
    char                        pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    int32_t                     dData;
    int32_t                     index = 0;
    uint32_t                    ui;
    PMOS_USER_FEATURE_VALUE     pUserFeature = nullptr;
    char                        sPath[MAX_UF_PATH];
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_USER_FEATURE_VALUE_TYPE ValueType = MOS_USER_FEATURE_VALUE_TYPE_INVALID;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_UNUSED(lpValueName);
    MOS_UNUSED(Reserved);
    MOS_UNUSED(dwType);

    if(UFKey == nullptr)
    {
       eStatus = MOS_STATUS_INVALID_PARAMETER;
       goto finish;
    }
    pUserFeature = (PMOS_USER_FEATURE_VALUE)UFKey;
    for (ui = 0; ui < pUserFeature->uiNumOfValues; ui++)
    {
        // Check the Key exist or not
        MOS_ZeroMemory(sPath, MAX_UF_PATH);
        MOS_OS_CHK_STATUS(MOS_GetAndroidPropPath(sPath, pUserFeature, LINUX_UF_FUNCTYPE_WRITE));

        if (MOS_AndroidPropOpenKey(sPath) != MOS_STATUS_SUCCESS){
            // create the Key according to pFeatureValue->ValueType
            if( MOS_AndroidPropCreateKey(pUserFeature, LINUX_UF_FUNCTYPE_WRITE) != MOS_STATUS_SUCCESS )
            {
                eStatus = MOS_STATUS_DIR_CREATE_FAILED;
                goto finish;
            }
        }
        MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
        ValueType = (MOS_USER_FEATURE_VALUE_TYPE)pUserFeature->ValueType;
        switch(ValueType)
        {
        case MOS_USER_FEATURE_VALUE_TYPE_BINARY:
            UserFeatureData.bData =*(int32_t*)lpData ;
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_INT32:
            UserFeatureData.i32Data =*(int32_t*)lpData ;
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_INT64:
            UserFeatureData.i64Data =*(int64_t*)lpData;
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_UINT32:
            UserFeatureData.u32Data = *(uint32_t*)lpData;
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_UINT64:
            UserFeatureData.u64Data =*(uint64_t*)lpData;
            break;
        case MOS_USER_FEATURE_VALUE_TYPE_STRING:
            UserFeatureData.StringData.pStringData =(char *)lpData;
            UserFeatureData.StringData.uSize = cbData;
            break;
        default:
            break;
        }//switch
        if (MOS_AndroidPropWriteValue(sPath, &UserFeatureData, ValueType) != MOS_STATUS_SUCCESS)
        {
            eStatus = MOS_STATUS_USER_FEATURE_KEY_WRITE_FAILED;
            goto finish;
        }
    }
finish:
    return eStatus;
}
#endif

/*----------------------------------------------------------------------------
| Name      : MOS_UserFeatureOpenKey_File
| Purpose   : Opens the specified user feature key.
| Arguments : UFKey        [in]  A handle to an open user feature key.
|             lpSubKey     [in]  The name of the user feature subkey to be opened.
|             ulOptions    [in]  This parameter is reserved and must be zero.
|             samDesired   [in]  Reserved, could be any REGSAM type value
|             phkResult    [out] A pointer to a variable that receives a handle
|                                to the opened key.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_UserFeatureOpenKey_File(
    void       *UFKey,
    const char *lpSubKey,
    uint32_t   ulOptions,  // reserved
    uint32_t   samDesired,
    void       **phkResult)
{
    char           pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS     iRet;
    uintptr_t      h_key = (uintptr_t)UFKey;
    MOS_UNUSED(ulOptions);
    MOS_UNUSED(samDesired);

    if((h_key == 0) /*|| (lpSubKey == nullptr)*/ || (phkResult == nullptr))    //[SH]: subkey can be NULL???
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_ZeroMemory(pcKeyName, MAX_USERFEATURE_LINE_LENGTH*sizeof(char));
    switch(h_key)
    {
    case UFKEY_INTERNAL:
        MOS_SecureStrcpy(pcKeyName, MAX_USERFEATURE_LINE_LENGTH, USER_FEATURE_KEY_INTERNAL);
        break;
    case UFKEY_EXTERNAL:
        MOS_SecureStrcpy(pcKeyName, MAX_USERFEATURE_LINE_LENGTH, USER_FEATURE_KEY_EXTERNAL);
        break;
    default:
        break;
    }

    MOS_SecureStrcat(pcKeyName, sizeof(pcKeyName), lpSubKey);
    iRet =  _UserFeature_GetKeyIdbyName(pcKeyName, phkResult);

    return iRet;
}

/*----------------------------------------------------------------------------
| Name      : MOS_UserFeatureGetValue_File
| Purpose   : Retrieves the type and data for the specified user feature value.
| Arguments : UFKey     [in]  A handle to an open user feature key.
|             lpSubKey  [in]  The name of the user feature key. This key must be a
|                             subkey of the key specified by the UFKey parameter
|             lpValue   [in]  The name of the user feature value.
|             dwFlags   [in]  Reserved, could be any uint32_t type value
|             pdwType   [out] A pointer to a variable that receives a code
|                             indicating the type of data stored in the
|                             specified value.
|             pvData    [out] A pointer to a buffer that receives the value's
|                             data.
|             pcbData   [out] A pointer to a variable that specifies the size
|                             of the buffer pointed to by the pvData parameter,
|                             in bytes.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_UserFeatureGetValue_File(
    void       *UFKey,
    const char *lpSubKey,
    const char *lpValue,
    uint32_t   dwFlags,
    uint32_t   *pdwType,
    void       *pvData,
    uint32_t   *pcbData)
{
    char          pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS    eStatus;
    MOS_UNUSED(dwFlags);

    if(UFKey == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    eStatus = MOS_STATUS_UNKNOWN;
    MOS_ZeroMemory(pcKeyName, MAX_USERFEATURE_LINE_LENGTH * sizeof(char));
    if ( (eStatus = _UserFeature_GetKeyNamebyId(UFKey,pcKeyName)) != MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }

    if(lpSubKey != nullptr)
    {
        MOS_SecureStrcat(pcKeyName, sizeof(pcKeyName), lpSubKey);
    }
    eStatus = _UserFeature_QueryValue(pcKeyName,
                                  lpValue,
                                  (uint32_t*)pdwType,
                                  pvData,
                                  (int32_t*)pcbData);

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : MOS_UserFeatureSetValueEx_File
| Purpose   : Sets the data and type of a specified value under a user feature key.
| Arguments : UFKey        [in] A handle to an open user feature key.
|             lpValueName  [in] The name of the user feature value.
|             Reserved      in] This parameter is reserved and must be NULL.
|             dwType       [in] The type of data pointed to by the lpData
|                               parameter.
|             lpData       [in] The data to be stored.
|             cbData       [in] The size of the information pointed to by the
|                               lpData parameter, in bytes.
| Returns   : If the function succeeds, the return value is MOS_STATUS_SUCCESS.
|             If the function fails, the return value is a error code defined
|             in mos_utilities.h.
| Comments  :
\---------------------------------------------------------------------------*/
MOS_STATUS MOS_UserFeatureSetValueEx_File(
    void            *UFKey,
    const char      *lpValueName,
    uint32_t        Reserved,
    uint32_t        dwType,
    uint8_t         *lpData,
    uint32_t        cbData)
{
    char    pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS  eStatus;
    MOS_UNUSED(Reserved);

    if (UFKey == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    MOS_ZeroMemory(pcKeyName, MAX_USERFEATURE_LINE_LENGTH*sizeof(char));
    if ((eStatus = _UserFeature_GetKeyNamebyId(UFKey,pcKeyName)) != MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }

    eStatus = _UserFeature_SetValue(pcKeyName,lpValueName,dwType,lpData,cbData);

    return eStatus;
}

MOS_STATUS MOS_OS_Utilities_Init()
{
    MOS_STATUS     eStatus = MOS_STATUS_SUCCESS;

    // lock mutex to avoid multi init in multi-threading env
    MOS_LockMutex(&gMosUtilMutex);

#if (_DEBUG || _RELEASE_INTERNAL)
    // Get use user feature file from env, instead of default.
    FILE* fp = nullptr;
    static char* tmpFile = getenv("GFX_FEATURE_FILE");

    if (tmpFile != nullptr)
    {
      if ((fp = fopen(tmpFile, "r")) != nullptr)
      {
        szUserFeatureFile = tmpFile;
        fclose(fp);
        MOS_OS_NORMALMESSAGE("using %s for USER_FEATURE_FILE", szUserFeatureFile);
      }
      else
      {
        MOS_OS_ASSERTMESSAGE("Can't open %s for USER_FEATURE_FILE!!!", tmpFile);
        eStatus =  MOS_STATUS_FILE_NOT_FOUND;
        goto finish;
      }
    }
#endif

    if (uiMOSUtilInitCount == 0)
    {
        pUFKeyOps = (PUFKEYOPS)MOS_AllocAndZeroMemory(sizeof(UFKEYOPS));
        MOS_OS_CHK_NULL(pUFKeyOps);
#ifdef ANDROID
        if (MOS_InitAndroidPropInfo() == MOS_STATUS_SUCCESS)
        {
            pUFKeyOps->pfnUserFeatureOpenKey = MOS_UserFeatureOpenKey_AndroidProp;
            pUFKeyOps->pfnUserFeatureGetValue = MOS_UserFeatureGetValue_AndroidProp;
            pUFKeyOps->pfnUserFeatureSetValueEx = MOS_UserFeatureSetValueEx_AndroidProp;
        }
        else
#endif
        {
            pUFKeyOps->pfnUserFeatureOpenKey = MOS_UserFeatureOpenKey_File;
            pUFKeyOps->pfnUserFeatureGetValue = MOS_UserFeatureGetValue_File;
            pUFKeyOps->pfnUserFeatureSetValueEx = MOS_UserFeatureSetValueEx_File;
        }
        //Init MOS User Feature Key from mos desc table
        eStatus = MOS_DeclareUserFeatureKeysForAllDescFields();
#if _MEDIA_RESERVED
        utilUserInterface = new CodechalUtilUserInterface();
#endif // _MEDIA_RESERVED
        eStatus = MOS_GenerateUserFeatureKeyXML();
#if MOS_MESSAGES_ENABLED
        // Initialize MOS message params structure and HLT
        MOS_MessageInit();
#endif // MOS_MESSAGES_ENABLED
        MosMemAllocCounter     = 0;
        MosMemAllocFakeCounter = 0;
        MosMemAllocCounterGfx  = 0;
        MOS_TraceEventInit();
    }
    uiMOSUtilInitCount++;

finish:
    MOS_UnlockMutex(&gMosUtilMutex);
    return eStatus;
}

MOS_STATUS MOS_OS_Utilities_Close()
{
    int32_t                             MemoryCounter = 0;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    // lock mutex to avoid multi close in multi-threading env
    MOS_LockMutex(&gMosUtilMutex);
    uiMOSUtilInitCount--;
    if (uiMOSUtilInitCount == 0 )
    {
        MOS_TraceEventClose();
        MosMemAllocCounter -= MosMemAllocFakeCounter;
        MemoryCounter = MosMemAllocCounter + MosMemAllocCounterGfx;
        MosMemAllocCounterNoUserFeature = MosMemAllocCounter;
        MosMemAllocCounterNoUserFeatureGfx = MosMemAllocCounterGfx;
        MOS_OS_VERBOSEMESSAGE("MemNinja leak detection end");

        UserFeatureWriteData.Value.i32Data    =   MemoryCounter;
        UserFeatureWriteData.ValueID          = __MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER_ID;
        MOS_UserFeature_WriteValues_ID(NULL, &UserFeatureWriteData, 1);

        eStatus = MOS_DestroyUserFeatureKeysForAllDescFields();
#if _MEDIA_RESERVED
        if (utilUserInterface) delete utilUserInterface;
#endif // _MEDIA_RESERVED
#if (_DEBUG || _RELEASE_INTERNAL)
        // MOS maintains a reference counter,
        // so if there still is another active lib instance, logs would still be printed.
        MOS_MessageClose();
#endif
        MOS_FreeMemory(pUFKeyOps);
        pUFKeyOps = nullptr;
    }
    MOS_UnlockMutex(&gMosUtilMutex);
    return eStatus;
}

MOS_STATUS MOS_UserFeatureOpenKey(
    void       *UFKey,
    const char *lpSubKey,
    uint32_t   ulOptions,
    uint32_t   samDesired,
    void       **phkResult)
{
    char           pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS     iRet;
    intptr_t       h_key = (intptr_t)UFKey;

    if((h_key == 0) /*|| (lpSubKey == nullptr)*/ || (phkResult == nullptr))    //[SH]: subkey can be NULL???
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (( pUFKeyOps != nullptr) && (pUFKeyOps->pfnUserFeatureOpenKey != nullptr))
    {
        return pUFKeyOps->pfnUserFeatureOpenKey(UFKey, lpSubKey, ulOptions, samDesired, phkResult);
    }
    else
    {
        return MOS_UserFeatureOpenKey(UFKey, lpSubKey, ulOptions, samDesired, phkResult);
    }
}

MOS_STATUS MOS_UserFeatureCloseKey(void  *UFKey)
{
    MOS_UNUSED(UFKey);
    //always return success, because we actually dong't have a key opened.
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_UserFeatureGetValue(
    void       *UFKey,
    const char *lpSubKey,
    const char *lpValue,
    uint32_t   dwFlags,
    uint32_t   *pdwType,
    void       *pvData,
    uint32_t   *pcbData)
{
    char          pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS    eStatus;

    if(UFKey == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    eStatus = MOS_STATUS_UNKNOWN;
    if (( pUFKeyOps != nullptr) && (pUFKeyOps->pfnUserFeatureGetValue != nullptr))
    {
        return pUFKeyOps->pfnUserFeatureGetValue(UFKey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }
    else
    {
        return MOS_UserFeatureGetValue_File(UFKey, lpSubKey, lpValue, dwFlags, pdwType, pvData, pcbData);
    }

}

MOS_STATUS MOS_UserFeatureQueryValueEx(
    void            *UFKey,
    char            *lpValueName,
    uint32_t        *lpReserved,
    uint32_t        *lpType,
    char            *lpData,
    uint32_t        *lpcbData)
{
    MOS_UNUSED(lpReserved);
    return MOS_UserFeatureGetValue(UFKey, "", lpValueName, 0, lpType, lpData, lpcbData);
}

MOS_STATUS MOS_UserFeatureSetValueEx(
    void            *UFKey,
    const char      *lpValueName,
    uint32_t        Reserved,
    uint32_t        dwType,
    uint8_t         *lpData,
    uint32_t        cbData)
{
    char        pcKeyName[MAX_USERFEATURE_LINE_LENGTH];
    MOS_STATUS  eStatus;

    if (UFKey == nullptr)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (( pUFKeyOps != nullptr) && (pUFKeyOps->pfnUserFeatureSetValueEx!= nullptr))
    {
        return pUFKeyOps->pfnUserFeatureSetValueEx(UFKey, lpValueName, Reserved, dwType, lpData, cbData);
    }
    else
    {
        return MOS_UserFeatureSetValueEx_File(UFKey, lpValueName, Reserved, dwType, lpData, cbData);
    }
}

// Event Related Functions: Android does not support these
#ifndef ANDROID
MOS_STATUS MOS_UserFeatureNotifyChangeKeyValue(
    void                *UFKey,
    int32_t             bWatchSubtree,
    HANDLE              hEvent,
    int32_t             fAsynchronous)
{
    key_t          key;
    int32_t        semid;
    struct sembuf  operation[1] ;

    key = ftok(szUserFeatureFile,1);
    semid = semget(key,1,0);
    //change semaphore
    operation[0].sem_op  = 1;
    operation[0].sem_num = 0;
    operation[0].sem_flg = SEM_UNDO;
    semop(semid, operation, 1);

    return MOS_STATUS_SUCCESS;
}

HANDLE MOS_CreateEventEx(
    void                *lpEventAttributes,
    char                *lpName,
    uint32_t            dwFlags)
{
    int32_t     semid;
    key_t       key;
    union semun
    {
        int32_t val;
        struct semid_ds *Buf;
        unsigned short *array;
    } semctl_arg;

    semid = 0;

    //Generate a unique key, U can also supply a value instead
    key = ftok(szUserFeatureFile, 1);
    semid = semget(key,  1, 0666 | IPC_CREAT );
    semctl_arg.val = 0; //Setting semval to 0
    semctl(semid, 0, SETVAL, semctl_arg);

    HANDLE ret = reinterpret_cast<HANDLE>(semid);

    return ret;
}

int32_t MOS_UserFeatureWaitForSingleObject(
    PTP_WAIT*           phNewWaitObject,
    HANDLE              hObject,
    void                *Callback,
    void                *Context)
{
    int32_t                  iRet;
    int32_t                  semid;
    struct sembuf            operation[1];
    pid_t                    pid;
    MOS_UserFeatureCallback  pCallback;
    LARGE_INTEGER            largeInteger;

    pCallback = (MOS_UserFeatureCallback)Callback;

    iRet  = 0;

    largeInteger.QuadPart = (int64_t)hObject;

    semid = largeInteger.u.LowPart;

    if ((pid=fork()) == -1)
    {
        printf("error\n");
    }
    else if(pid == 0)
    {
        while(1)
        {
            operation[0].sem_op = -1;
            operation[0].sem_num = 0;
            //now waiting
            semop(semid, operation, 1);
            pCallback(Context, 0);
        }
        exit(0);
    }
    else
    {
        iRet = pid;
    }

    *phNewWaitObject = reinterpret_cast<PTP_WAIT>(iRet);

    return (iRet != 0);
}

int32_t MOS_UnregisterWaitEx(PTP_WAIT hWaitHandle)
{
    int32_t iPid;
    LARGE_INTEGER largeInteger;

    largeInteger.QuadPart = (int64_t)hWaitHandle;

    iPid = largeInteger.u.LowPart;
    kill(iPid,SIGKILL);
    return true;
}

/*----------------------------------------------------------------------------
| Name      : GMMDebugBreak
| Purpose   : Fix compiling issue for Gmmlib on debug mode
| Arguments : N/A
| Returns   : void
| Calls     : N/A
| Callers   : Several
\---------------------------------------------------------------------------*/
void GMMDebugBreak(const char  *file, const char  *function,const int32_t line)
{
    // Not required for media driver
    return;
}

/*----------------------------------------------------------------------------
| Name      : GMMPrintMessage
| Purpose   : Fix compiling issue for Gmmlib on debug mode
| Arguments : N/A
| Returns   : void
| Calls     : N/A
| Callers   : Several
\---------------------------------------------------------------------------*/
void GMMPrintMessage(int32_t debuglevel, const char  *function, ...)
{
    // Not Required for media driver
    return;
}

#else // ANDROID
MOS_STATUS MOS_UserFeatureNotifyChangeKeyValue(
    void                *UFKey,
    int32_t             bWatchSubtree,
    HANDLE              hEvent,
    int32_t             fAsynchronous)
{
    MOS_UNUSED(UFKey);
    MOS_UNUSED(bWatchSubtree);
    MOS_UNUSED(hEvent);
    MOS_UNUSED(fAsynchronous);
    return MOS_STATUS_SUCCESS;
}

HANDLE MOS_CreateEventEx(
    void                *lpEventAttributes,
    char                *lpName,
    uint32_t            dwFlags)
{
    MOS_UNUSED(lpEventAttributes);
    MOS_UNUSED(lpName);
    MOS_UNUSED(dwFlags);
    return (HANDLE)1;
}

int32_t MOS_UserFeatureWaitForSingleObject(
    PTP_WAIT*           phNewWaitObject,
    HANDLE              hObject,
    void                *Callback,
    void                *Context)
{
    MOS_UNUSED(phNewWaitObject);
    MOS_UNUSED(hObject);
    MOS_UNUSED(Callback);
    MOS_UNUSED(Context);
    return true;
}

int32_t MOS_UnregisterWaitEx(PTP_WAIT hWaitHandle)
{
    MOS_UNUSED(hWaitHandle);
    return true;
}
#endif // !ANDROID

MOS_STATUS MOS_UserFeature_ParsePath(
    PMOS_USER_FEATURE_INTERFACE     pOsUserFeatureInterface,
    char * const                    pInputPath,
    PMOS_USER_FEATURE_TYPE          pUserFeatureType,
    char                            **ppSubPath)
{
    char                            *pValue;
    MOS_USER_FEATURE_TYPE           UserFeatureType;
    size_t                          uUFKeyLen;
    size_t                          uHKeyLen;
    size_t                          uValLen;
    size_t                          uSepLen;
    MOS_UNUSED(pOsUserFeatureInterface);

    //-------------------------------------------
    // the UserFeature interface is not currently an actual interface, just a collection
    // of functions, so pOsUserFeatureInterface will always be nullptr until this changes
    //MOS_OS_ASSERT(pOsUserFeatureInterface);
    MOS_OS_ASSERT(pInputPath);
    MOS_OS_ASSERT(strlen(pInputPath) > 0);
    MOS_OS_ASSERT(pUserFeatureType);
    MOS_OS_ASSERT(ppSubPath);
    //-------------------------------------------

    pValue = nullptr;

    pValue = strstr(pInputPath, MOS_UF_SEPARATOR);

    if (!pValue)
    {
        MOS_OS_ASSERTMESSAGE("Invalid user feature key %s.", pInputPath);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uUFKeyLen   = strlen(pInputPath);
    uValLen     = strlen(pValue);
    uSepLen     = strlen(MOS_UF_SEPARATOR);
    uHKeyLen    = uUFKeyLen - uValLen;

    if (uHKeyLen == 0)
    {
        MOS_OS_ASSERTMESSAGE("Invalid user feature key %s. Path separator in the begining.", pInputPath);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (uValLen <= uSepLen)
    {
        MOS_OS_ASSERTMESSAGE("Invalid user feature key %s. No value after path separator.", pInputPath);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if ((uHKeyLen == strlen(MOS_UFKEY_EXT)) &&
        (strncmp(pInputPath, MOS_UFKEY_EXT, uHKeyLen) == 0))
    {
        UserFeatureType = MOS_USER_FEATURE_TYPE_SYSTEM;
    }
    else if ((uHKeyLen == strlen(MOS_UFKEY_INT)) &&
        (strncmp(pInputPath, MOS_UFKEY_INT, uHKeyLen) == 0))
    {
        UserFeatureType = MOS_USER_FEATURE_TYPE_USER;
    }
    else
    {
        MOS_OS_ASSERTMESSAGE("Invalid user feature key %s. Expected %s or %s.", pInputPath, MOS_UFKEY_EXT, MOS_UFKEY_INT);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    pValue             = pValue + uSepLen;

    *pUserFeatureType  = UserFeatureType;
    *ppSubPath         = pValue;

    return MOS_STATUS_SUCCESS;
}

uint32_t MOS_GetLogicalCoreNumber()
{
    return sysconf(_SC_NPROCESSORS_CONF);
}

MOS_THREADHANDLE MOS_CreateThread(
    void                        *ThreadFunction,
    void                        *ThreadData)
{
    MOS_THREADHANDLE Thread;

    if (0 != pthread_create(&Thread, nullptr, (void *(*)(void *))ThreadFunction, ThreadData))
    {
        Thread = 0;
        MOS_OS_ASSERTMESSAGE("Create thread failed.");
    }

    return Thread;
}

uint32_t MOS_GetThreadId(
    MOS_THREADHANDLE            hThread)
{
    MOS_UNUSED(hThread);
    return 0;
}

uint32_t MOS_GetCurrentThreadId()
{
    return (uint32_t)pthread_self();
}

MOS_STATUS MOS_WaitThread(
    MOS_THREADHANDLE            hThread)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    if (hThread == 0)
    {
        MOS_OS_ASSERTMESSAGE("MOS wait thread failed, invalid thread handle.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }
    else if (0 != pthread_join(hThread, nullptr))
    {
        MOS_OS_ASSERTMESSAGE("Failed to join thread.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

PMOS_MUTEX MOS_CreateMutex()
{
    PMOS_MUTEX pMutex;

    pMutex = (PMOS_MUTEX)MOS_AllocMemory(sizeof(*pMutex));
    if (pMutex != nullptr)
    {
        if (pthread_mutex_init(pMutex, nullptr))
        {
            MOS_FreeMemory(pMutex);
            pMutex = nullptr;
        }
    }

    return pMutex;
}

MOS_STATUS MOS_DestroyMutex(PMOS_MUTEX pMutex)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (pMutex)
    {
        if (pthread_mutex_destroy(pMutex))
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
        MOS_FreeMemory(pMutex);
    }

    return eStatus;
}

MOS_STATUS MOS_LockMutex(PMOS_MUTEX pMutex)
{
    MOS_OS_CHK_NULL_RETURN(pMutex);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (pthread_mutex_lock(pMutex))
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

MOS_STATUS MOS_UnlockMutex(PMOS_MUTEX pMutex)
{
    MOS_OS_CHK_NULL_RETURN(pMutex);

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (pthread_mutex_unlock(pMutex))
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

PMOS_SEMAPHORE MOS_CreateSemaphore(
    uint32_t            uiInitialCount,
    uint32_t            uiMaximumCount)
{
    PMOS_SEMAPHORE pSemaphore = nullptr;
    MOS_UNUSED(uiMaximumCount);

    pSemaphore = (PMOS_SEMAPHORE)MOS_AllocMemory(sizeof(*pSemaphore));
    if (!pSemaphore)
        return nullptr;
    if (sem_init(pSemaphore, 0, uiInitialCount))
    {
        MOS_SafeFreeMemory(pSemaphore);
        pSemaphore = nullptr;
    }

    return pSemaphore;
}

MOS_STATUS MOS_DestroySemaphore(
    PMOS_SEMAPHORE              pSemaphore)
{
    MOS_SafeFreeMemory(pSemaphore);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MOS_WaitSemaphore(
    PMOS_SEMAPHORE              pSemaphore,
    uint32_t                    uiMilliseconds)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    if (uiMilliseconds == INFINITE)
    {
        if (sem_wait(pSemaphore))
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
    }
    else
    {
        struct timespec time = {
            (int32_t)uiMilliseconds / 1000000,
            ((int32_t)uiMilliseconds % 1000000) * 1000};

        if (sem_timedwait(pSemaphore, &time))
        {
            eStatus = MOS_STATUS_UNKNOWN;
        }
    }

    return eStatus;
}

MOS_STATUS MOS_PostSemaphore(
    PMOS_SEMAPHORE              pSemaphore,
    uint32_t                    uiPostCount)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    if (uiPostCount > 0)
    {
        while (uiPostCount--)
        {
            if (sem_post(pSemaphore))
            {
                eStatus = MOS_STATUS_UNKNOWN;
                break;
            }
        }
    }
    else
    {
        eStatus = MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

uint32_t MOS_WaitForSingleObject(
    void                        *pObject,
    uint32_t                    uiMilliseconds)
{
    uint32_t WaitSignal = 0;
    MOS_UNUSED(pObject);
    MOS_UNUSED(uiMilliseconds);

    return WaitSignal;
}

uint32_t MOS_WaitForMultipleObjects(
    uint32_t                    uiThreadCount,
    void                        **ppObjects,
    uint32_t                    bWaitAll,
    uint32_t                    uiMilliseconds)
{
    MOS_UNUSED(uiThreadCount);
    MOS_UNUSED(ppObjects);
    MOS_UNUSED(bWaitAll);
    MOS_UNUSED(uiMilliseconds);
    return 0;
}

int32_t MOS_AtomicIncrement(
    int32_t *pValue)
{
    return __sync_fetch_and_add(pValue, 1);
}

int32_t MOS_AtomicDecrement(
    int32_t *pValue)
{
    return __sync_fetch_and_sub(pValue, 1);
}

VAStatus MOS_StatusToOsResult(
    MOS_STATUS               eStatus)
{
    switch (eStatus)
    {
        case MOS_STATUS_SUCCESS:                        return VA_STATUS_SUCCESS;
        case MOS_STATUS_NO_SPACE:                       return VA_STATUS_ERROR_ALLOCATION_FAILED;
        case MOS_STATUS_INVALID_PARAMETER:              return VA_STATUS_ERROR_INVALID_PARAMETER;
        case MOS_STATUS_INVALID_HANDLE:                 return VA_STATUS_ERROR_INVALID_BUFFER;
        case MOS_STATUS_NULL_POINTER:                   return VA_STATUS_ERROR_INVALID_CONTEXT;
        default:                                        return VA_STATUS_ERROR_OPERATION_FAILED;
    }

    return VA_STATUS_ERROR_OPERATION_FAILED;
}

MOS_STATUS OsResultToMOS_Status(
    VAStatus                 eResult)
{
    switch (eResult)
    {
        case VA_STATUS_SUCCESS:                     return MOS_STATUS_SUCCESS;
        case VA_STATUS_ERROR_ALLOCATION_FAILED:     return MOS_STATUS_NO_SPACE;
        case VA_STATUS_ERROR_INVALID_PARAMETER:     return MOS_STATUS_INVALID_PARAMETER;
        case VA_STATUS_ERROR_INVALID_BUFFER:        return MOS_STATUS_INVALID_HANDLE;
        case VA_STATUS_ERROR_INVALID_CONTEXT:       return MOS_STATUS_NULL_POINTER;
        default:                                    return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_UNKNOWN;
}

MOS_STATUS MOS_GetLocalTime(
    struct tm* Tm)
{
    MOS_STATUS     eStatus = MOS_STATUS_SUCCESS;
    struct tm      *pTm;
    time_t         lTime = time(nullptr);
    pTm = localtime(&lTime);
    if(pTm == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("Failed to get localtime.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    eStatus = MOS_SecureMemcpy(Tm, sizeof(struct tm), pTm, sizeof(struct tm));
    return eStatus;
}

void MOS_TraceEventInit()
{
    // close first, if already opened.
    if (MosTraceFd >= 0)
    {
        close(MosTraceFd);
        MosTraceFd = -1;
    }
    MosTraceFd = open(MosTracePath, O_WRONLY);
    return;
}

void MOS_TraceEventClose()
{
    if (MosTraceFd >= 0)
    {
        close(MosTraceFd);
        MosTraceFd = -1;
    }
    return;
}

#define TRACE_EVENT_MAX_SIZE    4096
void MOS_TraceEvent(
    uint16_t         usId,
    uint8_t          ucType,
    void * const     pArg1,
    uint32_t         dwSize1,
    void * const     pArg2,
    uint32_t         dwSize2)
{
    if (MosTraceFd >= 0)
    {
        char  *pTraceBuf = (char *)MOS_AllocAndZeroMemory(TRACE_EVENT_MAX_SIZE);
        uint32_t   nLen = 0;

        if (pTraceBuf)
        {
            MOS_SecureStringPrint(pTraceBuf,
                        TRACE_EVENT_MAX_SIZE,
                        (TRACE_EVENT_MAX_SIZE-1),
                        "IMTE|%d|%d", // magic number IMTE (IntelMediaTraceEvent)
                        usId,
                        ucType);
            nLen = strlen(pTraceBuf);
            if (pArg1)
            {
                // convert raw event data to string. native raw data will be supported
                // from linux kernel 4.10, hopefully we can skip this convert in the future.
                const static char n2c[] = "0123456789ABCDEF";
                unsigned char *pData = (unsigned char *)pArg1;

                pTraceBuf[nLen++] = '|'; // prefix splite marker.
                while(dwSize1-- > 0 && nLen < TRACE_EVENT_MAX_SIZE-2)
                {
                    pTraceBuf[nLen++] = n2c[(*pData) >> 4];
                    pTraceBuf[nLen++] = n2c[(*pData++) & 0xf];
                }
                if (pArg2)
                {
                    pData = (unsigned char *)pArg2;
                    while(dwSize2-- > 0 && nLen < TRACE_EVENT_MAX_SIZE-2)
                    {
                        pTraceBuf[nLen++] = n2c[(*pData) >> 4];
                        pTraceBuf[nLen++] = n2c[(*pData++) & 0xf];
                    }
                }
            }
            size_t writeSize = write(MosTraceFd, pTraceBuf, nLen);
            MOS_FreeMemory(pTraceBuf);
        }
    }
    return;
}

MOS_STATUS MOS_GfxInfoInit()
{
    // not implemented
    return MOS_STATUS_SUCCESS;
}

void MOS_GfxInfoClose()
{
    // not implemented
}

void MOS_GfxInfo_RTErr(uint8_t ver,
    uint16_t    compId,
    uint16_t    FtrId,
    uint32_t    ErrorCode,
    uint8_t     num_of_triples,
    ...)
{
    // not implemented
}

void MOS_GfxInfo(
    uint8_t         ver,
    uint16_t        compId,
    uint32_t        tmtryID,
    uint8_t         num_of_triples,
    ...)
{
    // not implemented
}
