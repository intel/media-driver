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
#ifndef CMRTLIB_AGNOSTIC_SHARE_CMDEBUG_H_
#define CMRTLIB_AGNOSTIC_SHARE_CMDEBUG_H_

#include <cstdio>
#include "cm_def_os.h"

inline void CmPrintMessage( char *str, ... )
{
#ifdef _DEBUG
    //iSTD::PrintMessage( str );
#endif
}

inline void CmAssertMessage(const char *message) {
  printf("%s\n", message);
  CmAssert(0);
  return;
}

#ifdef _DEBUG
#define CmDebugMessage(arg) CmPrintMessage arg
#else
#define CmDebugMessage(arg)
#endif // _DEBUG

#define CmReleaseMessage(arg) CmPrintMessage arg

//*-----------------------------------------------------------------------------
//| Macro checks the COM Results
//*-----------------------------------------------------------------------------
#ifndef CHK_RET
#define CHK_RET(stmt)                                                           \
{                                                                               \
    result = (stmt);                                                            \
    if (result != CM_SUCCESS)                                                   \
    {                                                                           \
        CmPrintMessage("%s: hr check failed\n", __FUNCTION__);                  \
        CmAssert(0);                                                            \
        goto finish;                                                            \
    }                                                                           \
}
#endif // CHK_HR

#ifndef CHK_NULL
#define CHK_NULL(p)                                                           \
{                                                                               \
    if ( (p) == nullptr)                                                   \
    {                                                                           \
        CmPrintMessage("%s: nullptr check failed\n", __FUNCTION__);                  \
        CmAssert(0);                                                            \
        result = CM_NULL_POINTER;                                               \
        goto finish;                                                            \
    }                                                                           \
}
#endif

#ifndef CHK_NULL_RETURN
#define CHK_NULL_RETURN(p)                                                           \
{                                                                               \
    if ( (p) == nullptr)                                                   \
    {                                                                           \
        CmPrintMessage("%s: nullptr check failed\n", __FUNCTION__);                  \
        CmAssert(0);                                                            \
        return CM_NULL_POINTER;                                               \
    }                                                                           \
}
#endif

#ifndef CHK_FAILURE_RETURN
#define CHK_FAILURE_RETURN(ret)                                                           \
{                                                                               \
    if ( (ret) != CM_SUCCESS)                                                   \
    {                                                                           \
        CmPrintMessage("%s:%d: return check failed\n", __FUNCTION__, __LINE__);                  \
        return ret;                                                            \
    }                                                                           \
}
#endif

typedef void* Handle;
typedef Handle CmDeviceHandle;
typedef Handle CmUmdDeviceHandle;

int NotifyNewDevice(CmDeviceHandle deviceHandle, CmUmdDeviceHandle umdHandle, uint32_t driverStoreEnabled);
int NotifyDeviceDestruction(CmDeviceHandle deviceHandle, uint32_t driverStoreEnabled);

#endif  // #ifndef CMRTLIB_AGNOSTIC_SHARE_CMDEBUG_H_
