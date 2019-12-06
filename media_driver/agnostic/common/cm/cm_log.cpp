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
//! \file      cm_log.h
//! \brief     Contains Class Cm Logger definitions
//!

#include <iostream>
#include <iomanip>
#include "cm_log.h"
#include "cm_csync.h"
#include "mos_utilities.h"

#if CM_LOG_ON

// Definition (and initialization) of static attributes
CMRT_UMD::CSync globalCmLogLock;

/**
 * Logger Constructor.
 * It is a private constructor, called only by getInstance() and only the
 * first time. It is called inside a lock, so lock inside this method
 * is not required.
 * It only initializes the initial time. All configuration is done inside the
 * configure() method.
 */
CmLogger::CmLogger()
{
    //Get Verbosity Level
    GetVerbosityLevel();

    if (m_verbosityLevel == CM_LOG_LEVEL_NONE)
    {  // if it is not set, no file will be generated.
        return;
    }

    //Get Log File name
    std::ostringstream OutPutFile;
    char               fileNamePrefix[MAX_PATH];

    SYSTEMTIME systime;
    GetLocalTime(&systime);

    OutPutFile << "CM_LOG_" << CmGetCurProcessId() << "_" << systime.wMonth
               << "_" << systime.wDay << "_" << systime.wHour
               << "_" << systime.wMinute << "_" << systime.wSecond << ".log";
    m_logFile = OutPutFile.str();
    GetLogFileLocation(OutPutFile.str().c_str(), fileNamePrefix);

    // Open file
    m_streamOut.open(fileNamePrefix, std::ios::app);
    if (!m_streamOut)
    {
        CM_ASSERTMESSAGE("Open file failed!");
    }
}

CmLogger::~CmLogger()
{
    if (m_logFile.empty())
    {  // empty, dump to screen
        m_streamOut.close();
    }
}

void CmLogger::GetVerbosityLevel()
{
    // Read VerbosityLevel from RegisterKey
    MOS_USER_FEATURE_VALUE userFeatureValue;
    MOS_USER_FEATURE       userFeature;
    // User feature key reads
    MOS_ZeroMemory(&userFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    userFeature.Type        = MOS_USER_FEATURE_TYPE_USER;
    userFeature.pPath       = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    userFeature.pValues     = &userFeatureValue;
    userFeature.uiNumValues = 1;

    userFeatureValue.u32Data = CM_LOG_LEVEL_NONE;  // default value

    MOS_UserFeature_ReadValue(
        nullptr,
        &userFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_LOG_LEVEL,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);

    m_verbosityLevel = userFeatureValue.u32Data;
}

/**
 * Method used to print messages.
 * Called by the CM_DEBUG() macro.
 * @param Priority of the message
 * @param Source file where the method has been called (set equal to __FILE__
 *           by the DEBUG macro)
 * @param Source line where the method has been called (set equal to __LINE__
          by the macro)
 * @param Message
 */
void CmLogger::Print(const unsigned int verbosityLevel,
    const std::string &                 file,
    const int                           line,
    const std::string &                 message)
{
    CmLogger::Lock();

    if (verbosityLevel <= m_verbosityLevel)
    {
        switch (verbosityLevel)
        {
        case CM_LOG_LEVEL_DEBUG:
            m_streamOut << "[DEBUG]";
            break;

        case CM_LOG_LEVEL_INFO:
            m_streamOut << "[INFO]";
            break;

        case CM_LOG_LEVEL_WARN:
            m_streamOut << "[WARN]";
            break;

        case CM_LOG_LEVEL_ERROR:
            m_streamOut << "[ERROR]";
            break;
        }

        m_streamOut << "[PID :" << CmGetCurProcessId() << "][TID :" << CmGetCurThreadId() << "]"
                    << "[" << file << ":" << line << "] @ "
                    << ":" << message << std::endl;
    }

    CmLogger::Unlock();
}

void CmLogger::LogDataArrayHex(std::ostringstream &oss, unsigned char *data, unsigned int size)
{
    std::ios::fmtflags f(oss.flags());  // store the oss flags

    oss << "Data[Hex] : ";

    for (unsigned int i = 0; i < size; ++i)
    {
        oss << std::setfill('0') << std::setw(2) << std::hex << static_cast<short>(data[i]);
    }
    oss << std::endl;

    oss.flags(f);  // restore the flags
}

void CmLogger::Lock()
{
    globalCmLogLock.Acquire();
}

void CmLogger::Unlock()
{
    globalCmLogLock.Release();
}

CmLogTimer::CmLogTimer(const std::string str) :
    m_string(str),
    m_timer(str)
{
}

CmLogTimer::~CmLogTimer()
{
    m_timer.Stop();
    m_string = m_timer.ToString();
    CM_INFO(m_string);
}

void CmLogTimer::Stop()
{
    m_timer.Stop();
}

#endif
