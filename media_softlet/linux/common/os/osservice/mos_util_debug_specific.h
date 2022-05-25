/*
* Copyright (c) 2019-2022, Intel Corporation
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

#if MOS_MESSAGES_ENABLED

//!
//! \brief HLT file name template
//!
#define MOS_LOG_PATH_TEMPLATE "%s/igd_Pid%u_Tid%u.%s"
#define MOS_COMPONENT_NAME_DDI_STRING "[LIBVA]:"
#endif //MOS_MESSAGES_ENABLED

#define MOS_DIRECTORY_DELIMITER            '/'
#define MOS_DEBUG_DEFAULT_OUTPUT_LOCATION  "/data/codechal_dump/"

//!
//! \def MOS_CHK_NULL_WITH_HR(_compID, _subCompID, _ptr)
//!  Check if \a _ptr == nullptr, if so assert and return an HRESULT error
//!
#define MOS_CHK_NULL_WITH_HR(_compID, _subCompID, _ptr)                                     \
{                                                                                           \
    if ((_ptr) == nullptr)                                                                  \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");               \
        hr = MOS_STATUS_NULL_POINTER;                                                       \
        goto finish;                                                                        \
    }                                                                                       \
}

#define MOS_CHK_NULL_WITH_HR_RETURN(_compID, _subCompID, _ptr)                              \
{                                                                                           \
    if ((_ptr) == NULL)                                                                     \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (NULL) Pointer.");                  \
        MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID);    \
        return -1;                                                                          \
    }                                                                                       \
}

#define MOS_CHK_HR_RETURN(_compID, _subCompID, _stmt)                                       \
    {                                                                                       \
        hr = (_stmt);                                                                       \
        if (hr != 0)                                                                        \
        {                                                                                   \
            return hr;                                                                      \
        }                                                                                   \
    }

//!
//! \def MOS_CHK_STATUS_RETURN_HR(_compID, _subCompID, _stmt)
//!  Check _stmt, assert and return an error for failure
//!
#define MOS_CHK_STATUS_RETURN_HR(_compID, _subCompID, _stmt)                                        \
{                                                                                                   \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);                                                    \
    if (stmtStatus != MOS_STATUS_SUCCESS)                                                           \
    {                                                                                               \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "MOS returned error, eStatus = 0x%x", stmtStatus);   \
        MT_ERR3(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, stmtStatus); \
        return -1;                                                                                  \
    }                                                                                               \
}

#endif //__MOS_UTIL_DEBUG_SPECIFIC_H__

