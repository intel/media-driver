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
//! \file        mos_util_debug_specific.h 
//! \brief 
//!
//!

#ifndef __MOS_UTIL_DEBUG_SPECIFIC_H__
#define __MOS_UTIL_DEBUG_SPECIFIC_H__

#include "mos_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MOS_MESSAGES_ENABLED
//!
//! \brief Define messaging levels here in the order of importance
//!
typedef enum
{
    MOS_MESSAGE_LVL_DISABLED                  = 0,
    MOS_MESSAGE_LVL_CRITICAL                  = 1,
    MOS_MESSAGE_LVL_NORMAL                    = 2,
    MOS_MESSAGE_LVL_VERBOSE                   = 3,
    MOS_MESSAGE_LVL_FUNCTION_ENTRY            = 4,
    MOS_MESSAGE_LVL_FUNCTION_EXIT             = 5,
    MOS_MESSAGE_LVL_FUNCTION_ENTRY_VERBOSE    = 6,
    MOS_MESSAGE_LVL_MEMNINJA                  = 7,
    MOS_MESSAGE_LVL_COUNT
} MOS_MESSAGE_LEVEL;

//!
//! \brief Define Component IDs
//! When adding a component, need to update
//!   MOS_COMPONENT_ID,
//!   MOS_ComponentName,
//!   pcComponentUserFeatureKeys,
//!   subComponentCount
//!   and MOS_MESSAGE_COMPONENT_TAG.
//!
typedef enum
{
    MOS_COMPONENT_OS,
    MOS_COMPONENT_HW,
    MOS_COMPONENT_CODEC,
    MOS_COMPONENT_VP,
    MOS_COMPONENT_CP,
    MOS_COMPONENT_DDI,
    MOS_COMPONENT_CM,
    MOS_COMPONENT_CPLIB,
    MOS_COMPONENT_SCALABILITY,
    MOS_COMPONENT_MMC,
    MOS_COMPONENT_MCPY,
    MOS_COMPONENT_COUNT
} MOS_COMPONENT_ID;

//!
//! \brief    Prints debug messages in debug mode when enabled
//! \details  Prints debug messages if the level of the comp and sub-comp is
//!           set to less than the message level. Nop in release version.
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
                      ...);

#ifndef LOG_TAG
#define LOG_TAG "MOS"
#endif

//!
//! When printing from a C++ class, we'd like the class and function to be printed.
//! With our current Linux compiler, __FUNCTION__ does not include the class name.
//! So we use __PRETTY_FUNCTION__ and MOS_Message will remove extra data.
//! This is not needed for prints from C files so they will usually use __FUNCTION__.
//!

#if USE_PRETTY_FUNCTION
#define MOS_FUNCTION __PRETTY_FUNCTION__
#else
#define MOS_FUNCTION __FUNCTION__
#endif // USE_PRETTY_FUNCTION

//!
//! \def MOS_DEBUGMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_DEBUGMESSAGE(_level, _compID, _subCompID, _message, ...)                        \
    MOS_Message(_level, LOG_TAG, _compID, _subCompID, MOS_FUNCTION, __LINE__, _message, ##__VA_ARGS__)

//!
//! \def MOS_DEBUGMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_DEBUGMESSAGE_NOLINE(_level, _compID, _subCompID, _message, ...)                 \
    MOS_Message(_level, LOG_TAG, _compID, _subCompID, MOS_FUNCTION, -1, _message, ##__VA_ARGS__)

#endif //MOS_MESSAGES_ENABLED

#ifdef __cplusplus
}
#endif

//!
//! \brief HLT file name template
//!
#define MOS_LOG_PATH_TEMPLATE "%s/igd_Pid%u_Tid%u.%s"
#define MOS_COMPONENT_NAME_DDI_STRING "[LIBVA]:"

//!
//! \def MOS_CHK_NULL_WITH_HR(_compID, _subCompID, _ptr)
//!  Check if \a _ptr == nullptr, if so assert and return an HRESULT error
//!
#define MOS_CHK_NULL_WITH_HR(_compID, _subCompID, _ptr)                                     \
{                                                                                           \
    if ((_ptr) == nullptr)                                                                     \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");                  \
        hr = MOS_STATUS_NULL_POINTER;                                                                        \
        goto finish;                                                                        \
    }                                                                                       \
}

#define MOS_DIRECTORY_DELIMITER            '/'

#ifndef ANDROID
#define MOS_DEBUG_DEFAULT_OUTPUT_LOCATION  "/tmp/codechal_dump/"
#else
#define MOS_DEBUG_DEFAULT_OUTPUT_LOCATION  "/data/codechal_dump/"
#endif

#endif //__MOS_UTIL_DEBUG_SPECIFIC_H__

