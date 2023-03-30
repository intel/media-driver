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
//! \brief     Contains Class CmLogger declarations. 
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMLOG_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMLOG_H_

#include <fstream>
#include <ostream>
#include <string>
#include <sstream>

#include "cm_hal.h"

#if (_DEBUG || _RELEASE_INTERNAL)
#define CM_LOG_ON                   1
#endif

#if !(CM_LOG_ON)
#define INSERT_API_CALL_LOG(halState)
#define TASK_LOG(_pTask)
#define DEVICE_LOG(_pDev)
#define EVENT_LOG(_pEvt)

#else  // #if !(CM_LOG_ON)

#include "cm_perf.h"  // definition of CmTimer

typedef enum _CM_LOG_LEVEL{
    CM_LOG_LEVEL_NONE     = 0, // This emu must be zero.
    CM_LOG_LEVEL_ERROR   = 1,
    CM_LOG_LEVEL_WARN    = 2,
    CM_LOG_LEVEL_DEBUG   = 3,
    CM_LOG_LEVEL_INFO    = 4
}CM_LOG_LEVEL;

#define _CM_LOG(priority, msg, halState)         \
    CmLogPrint(priority, msg, halState)

#define CM_DEBUG(msg, halState)  _CM_LOG(CM_LOG_LEVEL_DEBUG, msg, halState)
#define INSERT_API_CALL_LOG(halState) CmLogTimer _LogTimer(__FUNCTION__, halState)
#define TASK_LOG(_pTask)    CM_DEBUG(_pTask->Log(), _pTask->GetHalState());
#define DEVICE_LOG(_pDev)   CM_DEBUG(_pDev->Log(), _pDev->GetHalState());
#define EVENT_LOG(_pEvt)    CM_DEBUG(_pEvt->Log(__FUNCTION__), _pEvt->GetHalState());

class CmLogger
{
public:
    static CmLogger* GetInstance(CM_HAL_STATE *halState);

    static void LogDataArrayHex(std::ostringstream &oss,
                                unsigned char *data,
                                unsigned int size);

    virtual void Print(const unsigned int verbosityLevel,
               const std::string &sourceFile,
               const int codeLine,
               const std::string &message);

private:
    /**
     * \brief Initial part of the name of the file used for Logging.
     * Date and time are automatically appended.
     */
    std::string m_logFile;

    /**
     * \brief Stream used when logging on a file or screen
     */
    std::ofstream m_streamOut;

    /**
     * \brief Verbosity threshold
     */
    unsigned int  m_verbosityLevel;

    CmLogger(CM_HAL_STATE *halState);

    ~CmLogger();

    /**
     * \brief Method to lock in case of multithreading
     */
    inline static void Lock();

    /**
     * \brief Method to unlock in case of multithreading
     */
    inline static void Unlock();

    /**
     * \brief Method to get verbosity level from register key
     */
    void GetVerbosityLevel(CM_HAL_STATE *halState);
};

inline void CmLogPrint(CM_LOG_LEVEL priority, std::string msg, CM_HAL_STATE *halState)
{
    std::ostringstream __debug_stream__;
    __debug_stream__ << msg;
    CmLogger* logTmp = CmLogger::GetInstance(halState);
    if (logTmp != nullptr)
    {
        logTmp->Print(priority, __FILE__, __LINE__,
                                        __debug_stream__.str());
    }
}

class CmLogTimer
{

public:
    CmLogTimer(const std::string &str, CM_HAL_STATE *halState)
            : m_string(str),
              m_timer(str),
              m_halState(halState) {}

    ~CmLogTimer()
    {
        m_timer.Stop();
        m_string = m_timer.ToString();
        _CM_LOG(CM_LOG_LEVEL_INFO, m_string, m_halState);
        return;
    }

    void Stop()
    {
        m_timer.Stop();
    }

private:
    std::string m_string;

    CmTimer m_timer;

    CM_HAL_STATE *m_halState;
};

#endif  // #if !(CM_LOG_ON)

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMLOG_H_
