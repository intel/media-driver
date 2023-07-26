/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file        mos_bufmgr_util_debug.h 
//! \brief 
//! \brief    Bufmgr Debug and Print utilities
//! \details  Provides assert and print to debug console functionality
//!           All Bufmgr debug and print utilities will only function in debug or
//!           release internal drivers, in a release driver they will be NOPs.
//!           To enable log in mos bufmgr by bellow config:
//!           [config]
//!           Mos Message Tags=4
//!           Mos Sub Components Tags=15728640 //uint32_t(24:21) -> (F00000)
//!           Mos Tags By Sub Component=1
//!
#ifndef __MOS_BUFMGR_UTIL_DEBUG_H__
#define __MOS_BUFMGR_UTIL_DEBUG_H__
#include "mos_util_debug.h"

//
//!
//! \def MOS_CHK_STATUS_MESSAGE_RETURN_WH_OP(_compID, _subCompID, _stmt, _ptr, _freeFunc, _message, ...)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure, and print message, and delete given ptr
//!
#define MOS_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(_compID, _subCompID, _stmt, _ptr, _freeFunc, _retVal, _message, ...)\
{                                                                               \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);                                \
    if (stmtStatus != MOS_STATUS_SUCCESS)                                       \
    {                                                                           \
        if(_ptr)                                                                \
            _freeFunc(_ptr);                                                    \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);        \
        MT_ERR3(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, stmtStatus); \
        return _retVal;                                                         \
    }                                                                           \
}

//!
//! \def MOS_DRM_CHK_NULL_NO_STATUS_RETURN(_ptr)
//!  MOS_ASSERTMESSAGE \a _ptr with MOS utility comp/subcomp info without returning a status
//!
#define MOS_DRM_CHK_NULL_NO_STATUS_RETURN(_ptr)                                                               \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _ptr)

//!
//! \def MOS_DRM_CHK_NULL_RETURN(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_NULL_RETURN(_ptr)                                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _ptr)

//!
//! \def MOS_DRM_CHK_NULL_RETURN_VALUE(_ptr, retVal)
//!  MOS_CHK_COND_RETURN_VALUE \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_NULL_RETURN_VALUE(_ptr, retVal)                                         \
    MOS_CHK_COND_RETURN_VALUE(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, (_ptr == nullptr), retVal, "Invalid (nullptr) Pointer.")

//!
//! \def MOS_DRM_CHK_NULL_MESSAGE_RETURN(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_NULL_MESSAGE_RETURN(_ptr, _message, ...) \
    MOS_CHK_NULL_MESSAGE_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _ptr, _message,  ##__VA_ARGS__)

//!
//! \def MOS_DRM_CHECK_CONDITION(_condition, _str, _ret)
//!  MOS_CHECK_CONDITION \a _condition with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHECK_CONDITION(_condition, _str, _ret)                                      \
   MOS_CHECK_CONDITION(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _condition, _str, _ret)

//!
//! \def MOS_DRM_ASSERT(_expr)
//!  MOS_ASSERT \a _expr with MOS Utility comp/subcomp info
//!
#define MOS_DRM_ASSERT(_expr)                                                                \
    MOS_ASSERT(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _expr)

//!
//! \def MOS_DRM_CHK_STATUS(_stmt)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_STATUS_RETURN(_stmt)                                                            \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _stmt)

//!
//! \def MOS_DRM_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ......)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                                  \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _stmt, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_CHK_STATUS_MESSAGE_RETURN_WH_OP(_stmt, _ptr, _freeFunc, _message, ...)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_STATUS_MESSAGE_RETURN_WH_OP(_stmt, _ptr, _freeFunc, _message, ...)                                  \
    MOS_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _stmt, _ptr, _freeFunc, _stmt, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(_stmt, _ptr, _freeFunc, _retVal, _message, ...)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_DRM_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(_stmt, _ptr, _freeFunc, _retVal, _message, ...)                                  \
    MOS_CHK_STATUS_MESSAGE_RETURN_VALUE_WH_OP(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _stmt, _ptr, _freeFunc, _retVal, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_ASSERTMESSAGE(_message, ...)
//!  MOS_ASSERTMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_DRM_ASSERTMESSAGE(_message, ...)                                                 \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_CRITICALMESSAGE(_message, ...)
//!  MOS_CRITICALMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_DRM_CRITICALMESSAGE(_message, ...)                                                 \
    MOS_CRITICALMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_NORMALMESSAGE(_message, ...)
//!  MOS_UTIL_NORMALMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_DRM_NORMALMESSAGE(_message, ...)                                                 \
    MOS_NORMALMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_VERBOSEMESSAGE(_message, ...)
//!  MOS_VERBOSEMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_DRM_VERBOSEMESSAGE(_message, ...)                                                \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _message, ##__VA_ARGS__)

//!
//! \def MOS_DRM_FUNCTION_ENTER
//!  Output ENTRY message with MOS Utility comp/subcomp info
//!
#define MOS_DRM_FUNCTION_ENTER                                                               \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT)

//!
//! \def MOS_DRM_MEMNINJAMESSAGE(_message, ...)
//!  MOS_MEMNINJAMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_DRM_MEMNINJAMESSAGE(_message, ...)                                               \
    MOS_MEMNINJAMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_EXT, _message, ##__VA_ARGS__)


#endif //__MOS_BUFMGR_UTIL_DEBUG_H__
