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
//! \file     mos_util_debug_specific.c
//! \brief    This module implments Linux MOS debug functionalities
//!

#include "mos_util_debug_next.h"
#include "mos_util_debug_specific_next.h"

#if MOS_MESSAGES_ENABLED

#include "mos_utilities_specific_next.h"
#include "mos_utilities_next.h"
#include "string.h"
#include <time.h>      //get_clocktime
#include <unistd.h>    //read, lseek
#include <fcntl.h>     //open

#ifdef ANDROID
#include <android/log.h>
#else
#include <signal.h>
#endif // ANDROID

//!
//! \brief HLT log file prefix
//!
const PCCHAR MosUtilDebug::m_mosLogPathPrefix = "/etc/log";
const PCCHAR MosUtilDebug::m_mosUltLogPathPrefix = "./";

MOS_MUTEX MosUtilDebugSpecific::m_mosMsgMutex = PTHREAD_MUTEX_INITIALIZER;

#define HLT_COPY_BUFFER_LENGTH 200
MOS_STATUS MosUtilDebug::MosHltpCopyFile(PFILE pFile, const PCCHAR szFileName)
{
    PFILE   pFileSrc;
    char    szBuffer[HLT_COPY_BUFFER_LENGTH];
    int32_t nRead;

    nRead = 0;
    if( (pFileSrc = fopen(szFileName , "r" )) == nullptr)
    {
        printf("open file %s failed", szFileName);
        return MOS_STATUS_FILE_OPEN_FAILED;
    }
    do
    {
        nRead = fread(szBuffer, 1, HLT_COPY_BUFFER_LENGTH, pFileSrc);
        if( ( nRead > 0 ) && (nRead <= HLT_COPY_BUFFER_LENGTH) )
        {
            fwrite(szBuffer, 1, nRead, pFile);
        }
    } while( nRead > 0 );

    fclose(pFileSrc);

    return MOS_STATUS_SUCCESS;
}

void MosUtilDebug::MosHltpPreface(PFILE pFile)
{
    time_t      rawtime;
    struct tm*  timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo == nullptr)
    {
        printf("fail to call localtime in MosHltpPreface\n");
        return;
    }

    fprintf(pFile, "//\n"
                   "// HLT log file: version1.0\n"
                   "// Logtime %d-%d-%d %d:%d:%d; Machine=",
                   timeinfo->tm_year,timeinfo->tm_mon, timeinfo->tm_yday,
                   timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

    fwrite("\n", 1, 1, pFile);
    MosHltpCopyFile(pFile, "/etc/hosts");

    // This section has been temporarily commented out because it's printing a non standard character to the logfile and corrupting it.
    /*
    fprintf(pFile, "\n"
                   "// PID=%d; Process name=",
                   getpid());

    MosHltpCopyFile(pFile, "/proc/self/cmdline");
    */

    fwrite("\n", 1, 1, pFile);

    MosHltpCopyFile(pFile, "/proc/version");
    // Linux version 2.6.35-22-generic (buildd@rothera) (gcc version 4.4.5 (Ubuntu/Linaro // 4.4.4-14ubuntu4) ) #33-Ubuntu SMP Sun Sep 19 20:34:50 UTC 2010

    if (m_mosMsgParams.bEnableMaps)
    {
        fwrite("\n// Loaded modules:\n", 20, 1, pFile);

        MosHltpCopyFile(pFile, "/proc/self/maps");
        fwrite("\n", 1, 1, pFile);
    }

    fflush(pFile);
}

MOS_STATUS MosUtilDebug::MosLogFileNamePrefix(char  *fileNamePrefix)
{
    int32_t                             iRet = 0;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData;
    MOS_STATUS                          eStatus = MOS_STATUS_UNKNOWN;

    if (MosUltFlag)
    {
        iRet =  MosUtilities::MosSecureStringPrint(
                     fileNamePrefix,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_MAX_HLT_FILENAME_LEN,
                     m_mosUltLogPathPrefix);

        if (iRet == 0)
        {
            eStatus = MOS_STATUS_SUCCESS;
        }

        return eStatus;
    }

    MosUtilities::MosZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = fileNamePrefix;
    eStatus = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY_ID,
        &UserFeatureData);

    // If the user feature key was not found, create it with the default value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        iRet = MosUtilities::MosSecureStringPrint(
                     fileNamePrefix,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_MAX_HLT_FILENAME_LEN,
                     m_mosLogPathPrefix);

        if (iRet == 0)
        {
            MosUtilities::MosZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
            UserFeatureWriteData.Value.StringData.pStringData = fileNamePrefix;
            UserFeatureWriteData.Value.StringData.uSize = strlen(fileNamePrefix) + 1;
            UserFeatureWriteData.ValueID = __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY_ID;

            eStatus = MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1);
        }
        else
        {
            return MOS_STATUS_UNKNOWN;
        }
    }

    return eStatus;
}

void MosUtilDebugSpecific::MosMessage(
    MOS_MESSAGE_LEVEL level,
    const PCCHAR      logtag,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      functionName,
    int32_t           lineNum,
    const PCCHAR      message,
    ...)
{
    va_list var_args;
    uint32_t nLen = 0;
    PCCHAR func = functionName;

    if (MosUtilDebug::MosShouldPrintMessage(level, compID, subCompID, message) == false)
    {
        return;
    }

    va_start(var_args, message);
    MosUtilities::MosLockMutex(&m_mosMsgMutex);
    // Proceed to print the message
    if (functionName == nullptr)
    {
        MosUtilities::MosSecureStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - ",
                MOS_ComponentName[compID],
                MOS_LogLevelName[level]);
        nLen = strlen(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);
    }
    else
    {
#if USE_PRETTY_FUNCTION
        // call MOS_getClassMethod to convert pretty function to class::function
        // return string locate in static memory, mutex should be hold.
        func = MOS_getClassMethod(functionName);
#endif //USE_PRETTY_FUNCTION
        if (lineNum < 0)
        {
            // no line number output
            MosUtilities::MosSecureStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - %s",
                MOS_ComponentName[compID],
                MOS_LogLevelName[level],
                func);
            nLen = strlen(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);
        }
        else
        {
            MosUtilities::MosSecureStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer,
                    MOS_MAX_MSG_BUF_SIZE,
                    (MOS_MAX_MSG_BUF_SIZE-1),
                    "%s%s - %s:%d: ",
                    MOS_ComponentName[compID],
                    MOS_LogLevelName[level],
                    func,
                    lineNum);
            nLen = strlen(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);
        }
    }
    MosUtilities::MosSecureVStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer + nLen,
                MOS_MAX_MSG_BUF_SIZE - nLen,
                (MOS_MAX_MSG_BUF_SIZE - 1 - nLen),
                message,
                var_args);

    // Dump message to debugger if print to output window enabled
    if (MosUtilDebug::m_mosMsgParams.bUseOutputDebugString)
    {


        printf("%s\n", MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);

    }

    // Write to log file if HLT enabled. File already open to add preface information
    if (MosUtilDebug::m_mosMsgParams.bUseHybridLogTrace)
    {
        if (MosUtilDebug::m_mosMsgParams.pLogFile != nullptr)
        {
            nLen = strlen(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);
            fwrite(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer, nLen, 1, MosUtilDebug::m_mosMsgParams.pLogFile);
            fprintf(MosUtilDebug::m_mosMsgParams.pLogFile, "\n");
        }
        else
        {
            printf("ERROR: MosUtilDebug::m_mosMsgParams.pLogFile is NULL!\n");
        }
    }
    MosUtilities::MosUnlockMutex(&m_mosMsgMutex);
    va_end(var_args);
}

//!
//! When printing from a C++ class, we'd like the class and function to be printed.
//! With our current Linux compiler, __FUNCTION__ does not include the class name.
//! So we use __PRETTY_FUNCTION__ and call MOS_getClassMethod_Next(__PRETTY_FUNCTION__) to remove extra data.
//! This is not needed for prints from C files so they will usually use __FUNCTION__.
//!
#if USE_PRETTY_FUNCTION

//!
//! m_functionName is used to temporarily store the concatinated __PRETTY_FUNCTION__,
//! when calling MOS_getClassMethod_Next().
//!
char MosUtilDebugSpecific::m_functionName[256] = {}; // 256 is an arbitrary long enough size.

PCCHAR MosUtilDebugSpecific::MOS_getClassMethod(PCCHAR pcPrettyFunction)
{
    PCCHAR end  = nullptr;
    uint32_t len  = 0;

    memset(m_functionName, '\0', sizeof(m_functionName));

    // Find the first '(', if it exists.
    end = strchr(pcPrettyFunction, '(');

    // We want to copy pcPrettyFunction into gFunction, only up to the first '('.
    len = end ? (uint32_t)(end - pcPrettyFunction) : strlen(pcPrettyFunction);

    MosUtilities::MosSecureMemcpy(m_functionName, sizeof(m_functionName), pcPrettyFunction, len);

    // dismiss anything before the last ' '.
    return strrchr(m_functionName, ' ') ? strrchr(m_functionName, ' ') + 1 : m_functionName;
}

#endif // USE_PRETTY_FUNCTION

#if MOS_ASSERT_ENABLED

void MosUtilDebug::MosAssert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    if (MosUtilDebug::MosShouldAssert(compID, subCompID) == false)
    {
        return;
    }

    //! NOTE: 
    //! If you hit asserts here and do not want to, you can cancel them by updating user feature keys under __MEDIA_USER_FEATURE_SUBKEY_INTERNAL.
    //! These keys can be found in USER_FEATURE_FILE (currently "/etc/igfx_user_feature.txt").
    //! First figure out what component is asserting (check element number compID in MOS_COMPONENT_ID).
    //! Then in the user feature key "<component> Message Tags", set the forth bit to zero.
    raise(SIGTRAP);
}

#endif // MOS_ASSERT_ENABLED

#endif // MOS_MESSAGES_ENABLED
