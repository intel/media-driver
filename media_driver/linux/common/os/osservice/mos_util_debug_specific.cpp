/*
* Copyright (c) 2013-2022, Intel Corporation
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

#if MOS_MESSAGES_ENABLED

#include "mos_utilities_specific.h"
#include "mos_utilities.h"
#include "string.h"
#include <time.h>      //get_clocktime
#include <unistd.h>    //read, lseek
#include <fcntl.h>     //open
#include "mos_util_debug_specific_next.h"

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

    if (MosUtilDebugSpecific::MosShouldPrintMessage(level, compID, subCompID, message) == false)
    {
        return;
    }

    va_start(var_args, message);
    MosUtilities::MosLockMutex(&MosUtilDebugSpecific::m_mosMsgMutex);
    // Proceed to print the message
    if (functionName == nullptr)
    {
        MosUtilities::MosSecureStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - ",
                MosUtilDebug::m_mosComponentName[compID],
                MosUtilDebug::m_mosLogLevelName[level]);
        nLen = strlen(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);
    }
    else
    {
#if USE_PRETTY_FUNCTION
    // call MosGetClassMethod to convert pretty function to class::function
    // return string locate in static memory, mutex should be hold.
        func = MosUtilDebugSpecific::MosGetClassMethod(functionName);
#endif //USE_PRETTY_FUNCTION
        if (lineNum < 0)
        {
            // no line number output
            MosUtilities::MosSecureStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer,
                MOS_MAX_MSG_BUF_SIZE,
                (MOS_MAX_MSG_BUF_SIZE-1),
                "%s%s - %s",
                MosUtilDebug::m_mosComponentName[compID],
                MosUtilDebug::m_mosLogLevelName[level],
                func);
            nLen = strlen(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer);
        }
        else
        {
            MosUtilities::MosSecureStringPrint(MosUtilDebug::m_mosMsgParams.g_MosMsgBuffer,
                    MOS_MAX_MSG_BUF_SIZE,
                    (MOS_MAX_MSG_BUF_SIZE-1),
                    "%s%s - %s:%d: ",
                    MosUtilDebug::m_mosComponentName[compID],
                    MosUtilDebug::m_mosLogLevelName[level],
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
            printf("ERROR: m_mosMsgParams.pLogFile is NULL!\n");
        }
    }
    MosUtilities::MosUnlockMutex(&MosUtilDebugSpecific::m_mosMsgMutex);

    va_end(var_args);
    return;
}

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
    return MosUtilDebug::MosAssert(compID,subCompID);
}

#endif // MOS_ASSERT_ENABLED

#endif // MOS_MESSAGES_ENABLED
