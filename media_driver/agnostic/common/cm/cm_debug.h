/*
* Copyright (c) 2018, Intel Corporation
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

//*-----------------------------------------------------------------------------
//| Assert Definitions
//*-----------------------------------------------------------------------------
#define CM_ASSERT(expr) \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, expr)
#define CM_ASSERT_DDI(expr) \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI, expr)
#define CM_ASSERT_PUBLIC(expr) \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_PUBLIC, expr)
#define CM_ASSERT_RENDERHAL(expr) \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, expr)

//*-----------------------------------------------------------------------------
//| Message Print Definitions
//*-----------------------------------------------------------------------------
#define CM_ASSERTMESSAGE(msg, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, msg, ##__VA_ARGS__)
#define CM_ASSERTMESSAGE_DDI(msg, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI, msg, ##__VA_ARGS__)
#define CM_ASSERTMESSAGE_PUBLIC(msg, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_PUBLIC, msg, ##__VA_ARGS__)
#define CM_ASSERTMESSAGE_RENDERHAL(msg, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, msg, ##__VA_ARGS__)
    
#define CM_NORMALMESSAGE(msg, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF, msg, ##__VA_ARGS__)   
#define CM_NORMALMESSAGE_DDI(msg, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI, msg, ##__VA_ARGS__)
#define CM_NORMALMESSAGE_PUBLIC(msg, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_PUBLIC, msg, ##__VA_ARGS__)   
#define CM_NORMALMESSAGE_RENDERHAL(msg, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, msg, ##__VA_ARGS__)   
   
#define CM_FUNCTION_ENTER \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_SELF)
#define CM_FUNCTION_ENTER_DDI \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_DDI)
#define CM_FUNCTION_ENTER_PUBLIC \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_PUBLIC)
#define CM_FUNCTION_ENTER_RENDERHAL \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL)


//*-----------------------------------------------------------------------------
//| Private Definitions (not suggest to use in other file)
//*-----------------------------------------------------------------------------
#define _CHECK_AND_GOTO_FINISH(cond, ret, retval, msg, ...) \
{ \
    if (cond) \
    { \
        ret = retval; \
        CM_ASSERTMESSAGE(msg, ##__VA_ARGS__); \
        goto finish;\
    } \
}

#define _CHECK_AND_RETURN(cond, retval, msg, ...) \
{ \
    if (cond) \
    {\
        CM_ASSERTMESSAGE(msg, ##__VA_ARGS__); \
        return retval;\
    }\
}

#define _CHECK_AND_RETURN_VOID(cond, msg, ...) \
{ \
    if(cond) \
    { \
        CM_ASSERTMESSAGE(msg, ##__VA_ARGS__); \
        return; \
    } \
}

#define _MOSSTATUS2CM(mosstatus, cmstatus) \
{ \
    switch((MOS_STATUS)mosstatus) { \
        case MOS_STATUS_SUCCESS: \
            cmstatus = CM_SUCCESS; \
            break; \
        case MOS_STATUS_NULL_POINTER: \
            cmstatus = CM_NULL_POINTER; \
            break;  \
        case MOS_STATUS_EXCEED_MAX_BB_SIZE: \
            cmstatus = CM_TOO_MUCH_THREADS; \
            break; \
        default: \
            cmstatus = (CM_RETURN_CODE)(0 - mosstatus + CM_MOS_STATUS_CONVERTED_CODE_OFFSET); \
            break; \
    } \
}

//*-----------------------------------------------------------------------------
//| Public Check Definitions
//*-----------------------------------------------------------------------------
// Check general condition.
#define CM_CHK_COND_GOTOFINISH(cond, retval, msg, ...) \
    _CHECK_AND_GOTO_FINISH(cond, eStatus, retval, msg, ##__VA_ARGS__)
#define CM_CHK_COND_RETURN(cond, retval, msg, ...) \
    _CHECK_AND_RETURN(cond, retval, msg, ##__VA_ARGS__)

// Check nullptr then goto finish.
#define CM_CHK_NULL_GOTOFINISH_MOSERROR(ptr) \
    _CHECK_AND_GOTO_FINISH((ptr == nullptr), eStatus, MOS_STATUS_NULL_POINTER, "Null pointer found!")
#define CM_CHK_NULL_GOTOFINISH_CMERROR(ptr) \
    _CHECK_AND_GOTO_FINISH((ptr == nullptr), hr, CM_NULL_POINTER, "Null pointer found!")
#define CM_CHK_NULL_GOTOFINISH(ptr, retval) \
    _CHECK_AND_GOTO_FINISH((ptr == nullptr), hr, retval, "Null pointer found!")
#define CM_CHK_NULL_GOTOFINISH_WITH_MSG(ptr, retval, msg, ...) \
    _CHECK_AND_GOTO_FINISH((ptr == nullptr), hr, retval, msg, ##__VA_ARGS__);

// Check nullptr then return.
#define CM_CHK_NULL_RETURN_MOSERROR(ptr) \
    _CHECK_AND_RETURN((ptr == nullptr), MOS_STATUS_NULL_POINTER, "Null pointer found!");
#define CM_CHK_NULL_RETURN_CMERROR(ptr) \
    _CHECK_AND_RETURN((ptr == nullptr), CM_NULL_POINTER, "Null pointer found!");
#define CM_CHK_NULL_RETURN(ptr, retval) \
    _CHECK_AND_RETURN((ptr == nullptr), retval, "Null pointer found!");
#define CM_CHK_NULL_RETURN_WITH_MSG(ptr, retval, msg, ...) \
    _CHECK_AND_RETURN((ptr == nullptr), retval, msg, ##__VA_ARGS__);
#define CM_CHK_NULL_RETURN_VOID(ptr) \
    _CHECK_AND_RETURN_VOID((ptr == nullptr), "Null pointer found!");

// Check return status.
#define CM_CHK_MOSSTATUS_GOTOFINISH(stmt) \
{ \
    eStatus = (MOS_STATUS)(stmt); \
    _CHECK_AND_GOTO_FINISH((eStatus != MOS_STATUS_SUCCESS), eStatus, eStatus , "MOS return error [%d]", eStatus); \
}
#define CM_CHK_MOSSTATUS_RETURN(stmt) \
{ \
    MOS_STATUS _tmp = (MOS_STATUS)(stmt); \
    _CHECK_AND_RETURN((_tmp != MOS_STATUS_SUCCESS), _tmp, "MOS return error [%d]", _tmp) \
}
#define CM_CHK_CMSTATUS_GOTOFINISH(stmt) \
{ \
    hr = (CM_RETURN_CODE)(stmt); \
    _CHECK_AND_GOTO_FINISH((hr != CM_SUCCESS), hr, hr, "CM return error [%d]", hr); \
}
#define CM_CHK_CMSTATUS_RETURN(stmt) \
{ \
    CM_RETURN_CODE _tmp = (CM_RETURN_CODE)(stmt); \
    _CHECK_AND_RETURN((_tmp != CM_SUCCESS), _tmp, "CM return error [%d]", _tmp) \
} 
#define CM_CHK_CMSTATUS_GOTOFINISH_WITH_MSG(stmt, msg, ...) \
{ \
    hr = (CM_RETURN_CODE)(stmt); \
    _CHECK_AND_GOTO_FINISH((hr != CM_SUCCESS), hr, hr, msg,  ##__VA_ARGS__); \
}
#define CM_CHK_CMSTATUS_RETURN_WITH_MSG(stmt, msg, ...) \
{ \
    CM_RETURN_CODE _tmp = (CM_RETURN_CODE)(stmt); \
    _CHECK_AND_RETURN((_tmp != CM_SUCCESS), _tmp, msg,  ##__VA_ARGS__) \
}
#define CM_CHK_HRESULT_GOTOFINISH_MOSERROR(stmt) \
{ \
    eStatus = (MOS_STATUS)OsResultToMOS_Status(stmt); \
    _CHECK_AND_GOTO_FINISH((eStatus != MOS_STATUS_SUCCESS), eStatus, eStatus, "hr check failed [%d]", eStatus); \
}
#define CM_CHK_MOSSTATUS_GOTOFINISH_CMERROR(stmt) \
{ \
    MOS_STATUS hr_mos = (MOS_STATUS)(stmt); \
    _MOSSTATUS2CM(hr_mos, hr); \
    _CHECK_AND_GOTO_FINISH((hr_mos != MOS_STATUS_SUCCESS), hr, hr, "MOS return error [%d]", hr_mos); \
}

/*===================== EU Debugger related stuff ===========================*/

bool RequestSipBinary(PLATFORM platform,
                      uint32_t bti,
                      const uint8_t *& sip,
                      uint32_t& sipSize,
                      uint32_t& resSize);

/*===================== end EU Debugger related stuff =======================*/
uint32_t GetLogFileLocation(const char *filename, char fileNamePrefix[]);
int32_t  GetDumpCounter(uint32_t valueID);
int32_t RecordDumpCounter(int32_t count, uint32_t ValueID);

int32_t GetCommandBufferDumpCounter(uint32_t valueID);
int32_t RecordCommandBufferDumpCounter(int32_t count, uint32_t ValueID);

int32_t GetSurfaceStateDumpCounter(uint32_t valueID);
int32_t RecordSurfaceStateDumpCounter(int32_t count, uint32_t ValueID);

int32_t GetInterfaceDescriptorDataDumpCounter(uint32_t valueID);
int32_t RecordInterfaceDescriptorDataDumpCounter(int32_t count, uint32_t ValueID);

uint32_t GetCommandBufferHeaderDWords(PMOS_INTERFACE osInterface);

#endif // __CM_DEBUG_H__
