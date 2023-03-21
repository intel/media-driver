/*
* Copyright (c) 2019-2022 Intel Corporation
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
//! \file     mos_util_debug_specific.cpp
//! \brief    This module implments Linux MOS debug functionalities
//!

#include "mos_util_debug.h"
#include "mos_util_debug_specific.h"
#include "mos_oca_rtlog_mgr_defs.h"
#include "mos_interface.h"
#include <stdlib.h>

#if MOS_MESSAGES_ENABLED

#include "mos_utilities_specific.h"
#include "media_user_setting.h"
#include "string.h"
#include <time.h>      //get_clocktime
#include <unistd.h>    //read, lseek
#include <fcntl.h>     //open

#ifdef ANDROID
#include <android/log.h>
#else
#include <signal.h>
#endif // ANDROID

#define MOS_ULT_LOG_PATH_PREFIX     "./"

//!
//! \brief HLT log file prefix
//!
#define MOS_LOG_PATH_PREFIX     "/etc/log"

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

MOS_STATUS MosUtilDebug::MosLogFileNamePrefix(char *fileNamePrefix, MediaUserSettingSharedPtr userSettingPtr)
{
    int32_t                             iRet = 0;
    MOS_STATUS                          eStatus = MOS_STATUS_UNKNOWN;
    MediaUserSetting::Value             outValue;

    if (MosUtilities::m_mosUltFlag && (*MosUtilities::m_mosUltFlag))
    {
        iRet =  MosUtilities::MosSecureStringPrint(
                     fileNamePrefix,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_ULT_LOG_PATH_PREFIX);

        if (iRet > 0)
        {
            eStatus = MOS_STATUS_SUCCESS;
        }

        return eStatus;
    }

    eStatus = ReadUserSetting(
        userSettingPtr,
        outValue,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY,
        MediaUserSetting::Group::Device);

    if(outValue.ConstString().size() > 0 && outValue.ConstString().size() < MOS_MAX_HLT_FILENAME_LEN)
    {
        MosUtilities::MosSecureStrcpy(fileNamePrefix, MOS_MAX_HLT_FILENAME_LEN, outValue.ConstString().c_str());
    }
    else     // If the user feature key was not found or not valid, create it with the default value.
    {
        MOS_OS_NORMALMESSAGE("eStatus = %x, outValue.size = %x", eStatus, outValue.ConstString().size());

        iRet = MosUtilities::MosSecureStringPrint(
                     fileNamePrefix,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_LOG_PATH_PREFIX);

        if (iRet > 0)
        {
            eStatus = ReportUserSetting(
                userSettingPtr,
                __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY,
                fileNamePrefix,
                MediaUserSetting::Group::Device);
        }
        else
        {
            return MOS_STATUS_UNKNOWN;
        }
    }

    return eStatus;
}

void MosUtilDebug::MosMessageInternal(
    MOS_MESSAGE_LEVEL level,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      functionName,
    int32_t           lineNum,
    const PCCHAR      message,
    va_list           var_args)
{
    uint32_t nLen = 0;
    PCCHAR func = functionName;
    static MOS_MUTEX mosMsgMutex = PTHREAD_MUTEX_INITIALIZER;

    if (MosShouldPrintMessage(level, compID, subCompID, message) == false)
    {
        return;
    }
    if (level == MOS_MESSAGE_LVL_CRITICAL)
    {
        OcaOnMosCriticalMessage(functionName, lineNum);
    }

    MosUtilities::MosLockMutex(&mosMsgMutex);
    // Proceed to print the message
    if (functionName == nullptr)
    {
        MosUtilities::MosSecureStringPrint(m_mosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - ",
                m_mosComponentName[compID],
                m_mosLogLevelName[level]);
        nLen = strlen(m_mosMsgParams.g_MosMsgBuffer);
    }
    else
    {
#if USE_PRETTY_FUNCTION
        // call MosGetClassMethod to convert pretty function to class::function
        // return string locate in static memory, mutex should be hold.
        func = MosGetClassMethod(functionName);
#endif //USE_PRETTY_FUNCTION
        if (lineNum < 0)
        {
            // no line number output
            MosUtilities::MosSecureStringPrint(m_mosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - %s",
                m_mosComponentName[compID],
                m_mosLogLevelName[level],
                func);
            nLen = strlen(m_mosMsgParams.g_MosMsgBuffer);
        }
        else
        {
            MosUtilities::MosSecureStringPrint(m_mosMsgParams.g_MosMsgBuffer,
                    MOS_MAX_MSG_BUF_SIZE,
                    (MOS_MAX_MSG_BUF_SIZE-1),
                    "%s%s - %s:%d: ",
                    m_mosComponentName[compID],
                    m_mosLogLevelName[level],
                    func,
                    lineNum);
            nLen = strlen(m_mosMsgParams.g_MosMsgBuffer);
        }
    }
    MosUtilities::MosSecureVStringPrint(m_mosMsgParams.g_MosMsgBuffer + nLen,
                MOS_MAX_MSG_BUF_SIZE - nLen,
                (MOS_MAX_MSG_BUF_SIZE - 1 - nLen),
                message,
                var_args);

    // Dump message to debugger if print to output window enabled
    if (m_mosMsgParams.bUseOutputDebugString)
    {
        printf("%s\n", m_mosMsgParams.g_MosMsgBuffer);
    }

    // Write to log file if HLT enabled. File already open to add preface information
    if (m_mosMsgParams.bUseHybridLogTrace)
    {
        if (m_mosMsgParams.pLogFile != nullptr)
        {
            nLen = strlen(m_mosMsgParams.g_MosMsgBuffer);
            fwrite(m_mosMsgParams.g_MosMsgBuffer, nLen, 1, m_mosMsgParams.pLogFile);
            fprintf(m_mosMsgParams.pLogFile, "\n");
        }
        else
        {
            printf("ERROR: m_mosMsgParams.pLogFile is NULL!\n");
        }
    }
    MosUtilities::MosUnlockMutex(&mosMsgMutex);

    return;
}

//!
//! When printing from a C++ class, we'd like the class and function to be printed.
//! With our current Linux compiler, __FUNCTION__ does not include the class name.
//! So we use __PRETTY_FUNCTION__ and call MosGetClassMethod(__PRETTY_FUNCTION__) to remove extra data.
//! This is not needed for prints from C files so they will usually use __FUNCTION__.
//!
#if USE_PRETTY_FUNCTION

//!
//! functionName is used to temporarily store the concatinated __PRETTY_FUNCTION__,
//! when calling MosGetClassMethod().
//!

PCCHAR MosUtilDebug::MosGetClassMethod(PCCHAR pcPrettyFunction)
{
    PCCHAR end  = nullptr;
    uint32_t len  = 0;
    static char functionName[256] = {}; // 256 is an arbitrary long enough size.

    memset(functionName, '\0', sizeof(functionName));

    // Find the first '(', if it exists.
    end = strchr(pcPrettyFunction, '(');

    // We want to copy pcPrettyFunction into gFunction, only up to the first '('.
    len = end ? (uint32_t)(end - pcPrettyFunction) : strlen(pcPrettyFunction);

    MosUtilities::MosSecureMemcpy(functionName, sizeof(functionName), pcPrettyFunction, len);

    // dismiss anything before the last ' '.
    return strrchr(functionName, ' ') ? strrchr(functionName, ' ') + 1 : functionName;
}

#endif // USE_PRETTY_FUNCTION

#if MOS_ASSERT_ENABLED

void MosUtilDebug::MosAssert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    if (MosShouldAssert(compID, subCompID) == false)
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
