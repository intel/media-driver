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
CmLogger* CmLogger::GlobalCmLogger = nullptr;
CMRT_UMD::CSync     GlobalCmLogLock;

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

    if(mVerbosityLevel == CM_LOG_LEVEL_NONE)
    {// if it is not set, no file will be generated.
        return ;
    }

    //Get Log File name
    std::ostringstream OutPutFile;

    SYSTEMTIME Systime;
    GetLocalTime(&Systime);

    OutPutFile << "CM_LOG_" << CmGetCurProcessId() << "_" << Systime.wMonth
                << "_" << Systime.wDay << "_" << Systime.wHour
                << "_" << Systime.wMinute << "_" << Systime.wSecond<<".log";

    mLogFile = OutPutFile.str();

    // Open file
    mStreamOut.open(mLogFile.c_str(), std::ios::app);

}

CmLogger::~CmLogger()
{

    if(mLogFile.empty())
    {  // empty, dump to screen
        mStreamOut.close();
    }

}

/**
 * Method to get a reference to the object (i.e., Singleton)
 * It is a static method.
 * @return Reference to the object.
 */
CmLogger& CmLogger::GetInstance()
{
    CmLogger::Lock();

    if (GlobalCmLogger == nullptr)
    {
        GlobalCmLogger = new CmLogger();
    }

    CmLogger::Unlock();
    return *GlobalCmLogger;
}

void CmLogger::GetVerbosityLevel()
{
    // Read VerbosityLevel from RegisterKey
    MOS_USER_FEATURE_VALUE      UserFeatureValue;
    MOS_USER_FEATURE            UserFeature;
    // User feature key reads
    MOS_ZeroMemory(&UserFeatureValue, sizeof(MOS_USER_FEATURE_VALUE));
    UserFeature.Type            = MOS_USER_FEATURE_TYPE_USER;
    UserFeature.pPath           = __MEDIA_USER_FEATURE_SUBKEY_INTERNAL;
    UserFeature.pValues         = &UserFeatureValue;
    UserFeature.uiNumValues     = 1;

    UserFeatureValue.u32Data = CM_LOG_LEVEL_NONE; // default value

    MOS_UserFeature_ReadValue(
        nullptr,
        &UserFeature,
        __MEDIA_USER_FEATURE_VALUE_MDF_LOG_LEVEL,
        MOS_USER_FEATURE_VALUE_TYPE_UINT32);

    mVerbosityLevel = UserFeatureValue.u32Data;

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
                    const std::string& file,
                    const int line,
                    const std::string& message)
{

    CmLogger::Lock();

    if(verbosityLevel <= mVerbosityLevel)
    {
        switch (verbosityLevel)
        {
        case CM_LOG_LEVEL_DEBUG:
            mStreamOut << "[DEBUG]";
            break;

        case CM_LOG_LEVEL_INFO:
            mStreamOut << "[INFO]";
            break;

        case CM_LOG_LEVEL_WARN:
            mStreamOut << "[WARN]";
            break;

        case CM_LOG_LEVEL_ERROR:
            mStreamOut << "[ERROR]";
            break;
        }

        mStreamOut << "[PID :" << CmGetCurProcessId() << "][TID :" << CmGetCurThreadId() << "]"
                   << "[" << file << ":" << line << "] @ " <<
                   ":" << message << std::endl;

    }

    CmLogger::Unlock();
}

void CmLogger::LogDataArrayHex(std::ostringstream &oss, unsigned char * data, unsigned int size )
{
    std::ios::fmtflags f(oss.flags()); // store the oss flags

    oss << "Data[Hex] : ";

    for (unsigned int i = 0; i < size; ++i)
    {
        oss << std::setfill('0') << std::setw(2) << std::hex << static_cast<short>(data[i]);
    }
    oss << std::endl;

    oss.flags(f);// restore the flags

}

void CmLogger::Lock()
{
    GlobalCmLogLock.Acquire();

}

void CmLogger::Unlock()
{
    GlobalCmLogLock.Release();
}

CmLogTimer::CmLogTimer(std::string str) :
    mString(str),
    mTimer (str)
{
}

CmLogTimer::~CmLogTimer()
{
    mTimer.Stop();
    mString = mTimer.ToString();
    CM_INFO(mString);
}

void CmLogTimer::Stop()
{
    mTimer.Stop();
}

#endif
