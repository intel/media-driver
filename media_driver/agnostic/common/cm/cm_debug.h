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
//! \file      cm_debug.h 
//! \brief     Contains CM debug definitions 
//!

#ifndef __CM_DEBUG_H__
#define __CM_DEBUG_H__

#include "cm_common.h"
#include "mos_utilities.h"
#include "mos_util_debug.h"

#define CM_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _expr)

#define CM_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _ptr)

#define CM_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _stmt)

#define CM_ASSERTMESSAGE(_message, ...)                                         \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define CM_NORMALMESSAGE(_message, ...)                                         \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define CM_ERROR_ASSERT(message, ...)                                           \
    CM_ASSERTMESSAGE(message, ##__VA_ARGS__);                                   \
    hr = MOS_STATUS_UNKNOWN;

#define CM_ERROR_ASSERT_RETURN(_ret, message, ...)                              \
    CM_ASSERTMESSAGE(message, ##__VA_ARGS__);                                   \
    hr = _ret;

#define CM_DDI_ASSERT(_expr)                                                    \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI, _expr)

#define CM_DDI_ASSERTMESSAGE(_message, ...)                                     \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define CM_DDI_NORMALMESSAGE(_message, ...)                                     \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI, _message, ##__VA_ARGS__)

#define CM_DDI_FUNCTION_ENTER                                                   \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CM, MOS_DDI_SUBCOMP_SELF)

#define CM_MOS_CHK_NULL_RETURN( _ptr )                                          \
    MOS_CHK_NULL_RETURN( MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _ptr )

#define CM_MOS_CHK_STATUS_RETURN( _stmt )                                       \
    MOS_CHK_STATUS_RETURN( MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, _stmt )

// Check the condition, if true, print the error message
// and return the specified value, do nothing otherwise.
#define CM_DDI_CHK_CONDITION(condition, _str, _ret)                             \
    if (condition) {                                                            \
        CM_DDI_ASSERTMESSAGE(_str);                                             \
        return _ret;                                                            \
    }

// If pointer is nullptr, print the error message and return the specified value.
#define CM_DDI_CHK_NULL(_ptr, _str, _ret)                                       \
    CM_DDI_CHK_CONDITION((nullptr == (_ptr)), _str, _ret)

#define CM_CHK_LESS(p, upper, str, ret)                                         \
    CM_DDI_CHK_CONDITION((p >= upper),str,ret)

#define CM_CHK_LESS_THAN(a, b, ret)                                             \
    CM_DDI_CHK_CONDITION(((a)<(b)),"Less Than",ret)

#define CM_FAILED(_cmstatus)                ((CM_RETURN_CODE)(_cmstatus) != CM_SUCCESS)
#define MOSSTATUS2CM(_mosstatus, _cmstatus)                                     \
{                                                                               \
    switch((MOS_STATUS)_mosstatus) {                                            \
        case MOS_STATUS_SUCCESS:                                                \
            _cmstatus = CM_SUCCESS;                                             \
            break;                                                              \
        case MOS_STATUS_NULL_POINTER:                                           \
            _cmstatus = CM_NULL_POINTER;                                        \
            break;                                                              \
        case MOS_STATUS_EXCEED_MAX_BB_SIZE:                                     \
            _cmstatus = CM_TOO_MUCH_THREADS;                                    \
            break;                                                              \
        default:                                                                \
            _cmstatus = (CM_RETURN_CODE)(0 - _mosstatus + CM_MOS_STATUS_CONVERTED_CODE_OFFSET);   \
            break;                                                              \
    }                                                                           \
}

#define MOSSTATUS2CM_AND_CHECK(_mosstatus, _cmstatus)                           \
{                                                                               \
    MOSSTATUS2CM(_mosstatus, _cmstatus);                                        \
    if (_cmstatus != CM_SUCCESS)                                                \
    {                                                                           \
        CM_ASSERT(0);                                                           \
        goto finish;                                                            \
    }                                                                           \
}

#ifndef CHK_MOSSTATUS_RETURN_CMERROR
#define CHK_MOSSTATUS_RETURN_CMERROR(_stmt)                                     \
{                                                                               \
    MOS_STATUS hr_mos = (MOS_STATUS)(_stmt);                                    \
    if (hr_mos != MOS_STATUS_SUCCESS)                                           \
    {                                                                           \
        CM_ASSERT(0);                                                           \
        MOSSTATUS2CM(hr_mos, hr);                                               \
        goto finish;                                                            \
    }                                                                           \
}
#endif

//*-----------------------------------------------------------------------------
//| Macro checks the CM Results
//*-----------------------------------------------------------------------------
#define CMCHK_HR(stmt)                                                          \
{                                                                               \
    hr = (CM_RETURN_CODE)(stmt);                                                \
    if (hr != CM_SUCCESS)                                                       \
    {                                                                           \
        CM_ASSERTMESSAGE("hr check failed.");                                   \
        goto finish;                                                            \
    }                                                                           \
}

//*-----------------------------------------------------------------------------
//| Macro checks the CM Results
//*-----------------------------------------------------------------------------
#define CMCHK_HR_MESSAGE(stmt, _str)                                            \
{                                                                               \
    hr = (stmt);                                                                \
    if (hr != CM_SUCCESS)                                                       \
    {                                                                           \
        CM_DDI_ASSERTMESSAGE("%s [%d].", _str, hr);                             \
        goto finish;                                                            \
    }                                                                           \
}

//*-----------------------------------------------------------------------------
//| Macro checks for nullptr and sets the long32
//*-----------------------------------------------------------------------------
#define CMCHK_NULL(ptr)                                                         \
{                                                                               \
    if ((ptr) == nullptr)                                                          \
    {                                                                           \
        CM_ASSERTMESSAGE("Invalid (nullptr) Pointer.");                            \
        hr = CM_NULL_POINTER;                                                   \
        goto finish;                                                            \
    }                                                                           \
}

//*-----------------------------------------------------------------------------
//| Macro checks for nullptr and returns
//*-----------------------------------------------------------------------------
#define CMCHK_NULL_AND_RETURN(ptr)                                              \
{                                                                               \
    if ((ptr) == nullptr)                                                       \
    {                                                                           \
        CM_ASSERTMESSAGE("Invalid (nullptr) Pointer.");                         \
        return CM_NULL_POINTER;                                                 \
    }                                                                           \
}

//*-----------------------------------------------------------------------------
//| Macro checks status and returns
//*-----------------------------------------------------------------------------
#define CMCHK_STATUS_AND_RETURN(stmt)                                          \
{                                                                               \
    CM_RETURN_CODE _tmp = (CM_RETURN_CODE)(stmt);                               \
    if (_tmp != CM_SUCCESS)                                                     \
    {                                                                           \
      CM_ASSERT(0);                                                             \
      return _tmp;                                                              \
    }                                                                           \
}


//*-----------------------------------------------------------------------------
//| Macro checks for nullptr and return a specific value
//*-----------------------------------------------------------------------------
#define CMCHK_NULL_RETURN(ptr, returnValue)                                     \
{                                                                               \
    if ((ptr) == nullptr)                                                          \
    {                                                                           \
        CM_ASSERTMESSAGE("Invalid (nullptr) Pointer.");                            \
        hr = returnValue;                                                       \
        goto finish;                                                            \
    }                                                                           \
}

//*-----------------------------------------------------------------------------
//| Notes:
//| Below two are old MACRO definitions, please don't use them!!
//| Somehow they are used by other components, can't removed
//*-----------------------------------------------------------------------------
#ifndef CHK_HR
#define CHK_HR(_stmt)       CMCHK_HR(_stmt)
#endif

#ifndef CHK_NULL
#define CHK_NULL(_ptr)      CMCHK_NULL(_ptr)
#endif

/*===================== EU Debugger related stuff ===========================*/

bool RequestSipBinary(PLATFORM platform,
                      uint32_t bti,
                      const uint8_t *& sip,
                      uint32_t& sipSize,
                      uint32_t& resSize);

/*===================== end EU Debugger related stuff =======================*/
uint32_t GetLogFileLocation(std::string &logFile,std::ostringstream &outputFileName, char fileNamePrefix[]);
#endif // __CM_DEBUG_H__
