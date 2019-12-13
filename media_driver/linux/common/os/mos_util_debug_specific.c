/*
* Copyright (c) 2013-2017, Intel Corporation
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

#include "mos_util_debug.h"

#if MOS_MESSAGES_ENABLED

#include "mos_utilities_specific.h"
#include "mos_utilities.h"
#include "string.h"
#include <time.h>      //get_clocktime
#include <unistd.h>    //read, lseek
#include <fcntl.h>     //open

#ifdef ANDROID
#include <android/log.h>
#else
#include <signal.h>
#endif // ANDROID

extern int32_t MOS_ShouldPrintMessage(
    MOS_MESSAGE_LEVEL level,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      message);

//!
//! \brief HLT log file prefix
//!
const PCCHAR MosLogPathPrefix    = "/etc/log";
const PCCHAR MosUltLogPathPrefix = "./";

extern MOS_MESSAGE_PARAMS g_MosMsgParams;
extern uint8_t            MosUltFlag;
static MOS_MUTEX gMosMsgMutex = PTHREAD_MUTEX_INITIALIZER;

/*----------------------------------------------------------------------------
| Name      : MOS_HltpCopyFile
| Purpose   : Copy all file content from the source file to the target file.
| Arguments : szFileName - source file name to copy from
|             pFile - target file
| Returns   : Returns one of the MOS_STATUS error codes if failed,
|             else MOS_STATUS_SUCCESS
| Comments  :
\---------------------------------------------------------------------------*/
#define HLT_COPY_BUFFER_LENGTH 200
static MOS_STATUS MOS_HltpCopyFile(PFILE pFile, const PCCHAR szFileName)
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

/*----------------------------------------------------------------------------
| Name      : MOS_HltpPreface
| Purpose   : Add preface information to the HLT log when initialized
| Arguments : pFile - Pointer to the log file
| Returns   : None
| Comments  :
\---------------------------------------------------------------------------*/
void MOS_HltpPreface(PFILE pFile)
{
    time_t      rawtime;
    struct tm*  timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    if (timeinfo == nullptr)
    {
        printf("fail to call localtime in MOS_HltpPreface\n");
        return;
    }

    fprintf(pFile, "//\n"
                   "// HLT log file: version1.0\n"
                   "// Logtime %d-%d-%d %d:%d:%d; Machine=",
                   timeinfo->tm_year,timeinfo->tm_mon, timeinfo->tm_yday,
                   timeinfo->tm_hour,timeinfo->tm_min,timeinfo->tm_sec);

    fwrite("\n", 1, 1, pFile);
    MOS_HltpCopyFile(pFile, "/etc/hosts");

    // This section has been temporarily commented out because it's printing a non standard character to the logfile and corrupting it.
    /*
    fprintf(pFile, "\n"
                   "// PID=%d; Process name=",
                   getpid());

    MOS_HltpCopyFile(pFile, "/proc/self/cmdline");
    */

    fwrite("\n", 1, 1, pFile);

    MOS_HltpCopyFile(pFile, "/proc/version");
    // Linux version 2.6.35-22-generic (buildd@rothera) (gcc version 4.4.5 (Ubuntu/Linaro // 4.4.4-14ubuntu4) ) #33-Ubuntu SMP Sun Sep 19 20:34:50 UTC 2010

    if(g_MosMsgParams.bEnableMaps)
    {
        fwrite("\n// Loaded modules:\n", 20, 1, pFile);

        MOS_HltpCopyFile(pFile, "/proc/self/maps");
        fwrite("\n", 1, 1, pFile);
    }

    fflush(pFile);
}

//!
//! \brief    Form a string that will prefix MOS's log file name
//! \details  Form a string that will prefix MOS's log file name
//!           MosLogPathPrefix defines the system directory that
//!           Mos uses to prefix the log filename by default.
//! \param    char  *fileNamePrefix
//!           [out] Pointer to the string where the prefix is returned
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_LogFileNamePrefix(char  *fileNamePrefix)
{
    int32_t                             iRet = 0;
    MOS_USER_FEATURE_VALUE_DATA         UserFeatureData;
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData;
    MOS_STATUS                          eStatus = MOS_STATUS_UNKNOWN;

    if (MosUltFlag)
    {
        iRet =  MOS_SecureStringPrint(
                     fileNamePrefix,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MosUltLogPathPrefix);

        if (iRet > 0)
        {
            eStatus = MOS_STATUS_SUCCESS;
        }

        return eStatus;
    }

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.StringData.pStringData = fileNamePrefix;
    eStatus = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY_ID,
        &UserFeatureData);

    // If the user feature key was not found, create it with the default value.
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        iRet = MOS_SecureStringPrint(
                     fileNamePrefix,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MOS_MAX_HLT_FILENAME_LEN,
                     MosLogPathPrefix);

        if (iRet > 0)
        {
            MOS_ZeroMemory(&UserFeatureWriteData, sizeof(UserFeatureWriteData));
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

//!
//! \brief    Prints debug messages when enabled
//! \details  Prints debug messages if prints are enabled and the level of the comp and sub-comp is
//!           set to less than the message level.
//! \param    MOS_MESSAGE_LEVEL level
//!           [in] Level of the message
//! \param    const PCCHAR logtag
//!           [in] For Linux only, used for tagging the message.
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \param    const char  *functionName
//!           [in] pointer to the function name
//! \param    int32_t lineNum
//!           [in] Indicates which line the message locate, -1 for no line output
//! \param    const char  *message
//!           [in] pointer to the message format string
//! \param    var_args
//!           [in] variable list of arguments for the message
//! \return   void
//!
void MOS_Message(
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

    if (MOS_ShouldPrintMessage(level, compID, subCompID, message) == false)
    {
        return;
    }

    va_start(var_args, message);
    MOS_LockMutex(&gMosMsgMutex);
    // Proceed to print the message
    if (functionName == nullptr)
    {
        MOS_SecureStringPrint(g_MosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - ",
                MOS_ComponentName[compID],
                MOS_LogLevelName[level]);
        nLen = strlen(g_MosMsgParams.g_MosMsgBuffer);
    }
    else
    {
#if USE_PRETTY_FUNCTION
        PCCHAR MOS_getClassMethod(PCCHAR pcPrettyFunction);
        // call MOS_getClassMethod to convert pretty function to class::function
        // return string locate in static memory, mutex should be hold.
        func = MOS_getClassMethod(functionName);
#endif //USE_PRETTY_FUNCTION
        if (lineNum < 0)
        {
            // no line number output
            MOS_SecureStringPrint(g_MosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - %s",
                MOS_ComponentName[compID],
                MOS_LogLevelName[level],
                func);
            nLen = strlen(g_MosMsgParams.g_MosMsgBuffer);
        }
        else
        {
            MOS_SecureStringPrint(g_MosMsgParams.g_MosMsgBuffer,
                    MOS_MAX_MSG_BUF_SIZE,
                    (MOS_MAX_MSG_BUF_SIZE-1),
                    "%s%s - %s:%d: ",
                    MOS_ComponentName[compID],
                    MOS_LogLevelName[level],
                    func,
                    lineNum);
            nLen = strlen(g_MosMsgParams.g_MosMsgBuffer);
        }
    }
    MOS_SecureVStringPrint(g_MosMsgParams.g_MosMsgBuffer + nLen,
                MOS_MAX_MSG_BUF_SIZE - nLen,
                (MOS_MAX_MSG_BUF_SIZE - 1 - nLen),
                message,
                var_args);

    // Dump message to debugger if print to output window enabled
    if (g_MosMsgParams.bUseOutputDebugString)
    {
#ifdef ANDROID
        int android_level;
        switch (level)
        {
            case MOS_MESSAGE_LVL_CRITICAL:
                android_level = ANDROID_LOG_ERROR;
                break;
            case MOS_MESSAGE_LVL_NORMAL:
                android_level = ANDROID_LOG_DEBUG;
                break;
            case MOS_MESSAGE_LVL_VERBOSE:
                android_level = ANDROID_LOG_VERBOSE;
                break;
            case MOS_MESSAGE_LVL_FUNCTION_ENTRY:
            case MOS_MESSAGE_LVL_FUNCTION_EXIT:
            case MOS_MESSAGE_LVL_FUNCTION_ENTRY_VERBOSE:
            case MOS_MESSAGE_LVL_FUNCTION_EXIT_VERBOSE:
            default:
                android_level = ANDROID_LOG_INFO;
                break;
        }

        __android_log_print(android_level, logtag, "%s\n", g_MosMsgParams.g_MosMsgBuffer);

#else // ANDROID

        printf("%s\n", g_MosMsgParams.g_MosMsgBuffer);

#endif // ANDROID
    }

    // Write to log file if HLT enabled. File already open to add preface information
    if (g_MosMsgParams.bUseHybridLogTrace)
    {
        if (g_MosMsgParams.pLogFile != nullptr)
        {
            nLen = strlen(g_MosMsgParams.g_MosMsgBuffer);
            fwrite(g_MosMsgParams.g_MosMsgBuffer, nLen, 1, g_MosMsgParams.pLogFile);
            fprintf(g_MosMsgParams.pLogFile, "\n");
        }
        else
        {
#ifdef ANDROID
            __android_log_print(ANDROID_LOG_ERROR, logtag, "ERROR: g_MosMsgParams.pLogFile is NULL!");
#else // ANDROID
            printf("ERROR: g_MosMsgParams.pLogFile is NULL!\n");
#endif // ANDROID
        }
    }
    MOS_UnlockMutex(&gMosMsgMutex);
    va_end(var_args);
}

//!
//! When printing from a C++ class, we'd like the class and function to be printed.
//! With our current Linux compiler, __FUNCTION__ does not include the class name.
//! So we use __PRETTY_FUNCTION__ and call MOS_getClassMethod(__PRETTY_FUNCTION__) to remove extra data.
//! This is not needed for prints from C files so they will usually use __FUNCTION__.
//!
#if USE_PRETTY_FUNCTION

//!
//! gFunctionName is used to temporarily store the concatinated __PRETTY_FUNCTION__,
//! when calling MOS_getClassMethod().
//!
static char gFunctionName[256]; // 256 is an arbitrary long enough size.

//!
//! \brief    Converts a __PRETTY_FUNCTION__ into Class::Method
//! \details  Converts a __PRETTY_FUNCTION__ into Class::Method to allow prettier debug output
//! \param    PCCHAR pcPrettyFunction
//!           [in] in the form of "TYPE [CLASS::]FUNCTION(INPUT LIST)"
//! \return   PCCHAR in the form of [CLASS::]FUNCTION
//!
PCCHAR MOS_getClassMethod(PCCHAR pcPrettyFunction)
{
    PCCHAR end  = nullptr;
    uint32_t len  = 0;

    memset(gFunctionName, '\0', sizeof(gFunctionName));

    // Find the first '(', if it exists.
    end = strchr(pcPrettyFunction, '(');

    // We want to copy pcPrettyFunction into gFunction, only up to the first '('.
    len = end ? (uint32_t)(end - pcPrettyFunction) : strlen(pcPrettyFunction);

    MOS_SecureMemcpy(gFunctionName, sizeof(gFunctionName), pcPrettyFunction, len);

    // dismiss anything before the last ' '.
    return strrchr(gFunctionName, ' ') ? strrchr(gFunctionName, ' ') + 1 : gFunctionName;
}

#endif // USE_PRETTY_FUNCTION

#if MOS_ASSERT_ENABLED

extern int32_t MOS_ShouldAssert(MOS_COMPONENT_ID compID, uint8_t subCompID);

//!
//! \brief    MOS assert function for MOS internal use
//! \details  Halts the cpu in debug mode when expression resolves to zero
//!           and only if assert enabled for both comp and sub-comp.
//!           Nop in release version
//!           Called by MOS_ASSERT macro only
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \return   void
//!
void _MOS_Assert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    if (MOS_ShouldAssert(compID, subCompID) == false)
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
