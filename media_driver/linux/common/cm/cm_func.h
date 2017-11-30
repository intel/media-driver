/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_func.h  
//! \brief     CM RT DDI function ID  
//!
#pragma once
#include "media_libva_common.h" //for VADriverContextP

//*-----------------------------------------------------------------------------
//| CM extension Function Codes
//*-----------------------------------------------------------------------------
typedef enum _CM_FUNCTION_ID
{
    CM_FN_RT_ULT                       = 0x900,
    CM_FN_RT_ULT_INFO                  = 0x902,

    CM_FN_CREATECMDEVICE                   = 0x1000,
    CM_FN_DESTROYCMDEVICE                  = 0x1001,

    CM_FN_CMDEVICE_CREATEBUFFER            = 0x1100,
    CM_FN_CMDEVICE_DESTROYBUFFER           = 0x1101,
    CM_FN_CMDEVICE_CREATEBUFFERUP          = 0x1102,
    CM_FN_CMDEVICE_DESTROYBUFFERUP         = 0x1103,
    CM_FN_CMDEVICE_CREATESURFACE2D         = 0x1104,
    CM_FN_CMDEVICE_DESTROYSURFACE2D        = 0x1105,
    CM_FN_CMDEVICE_CREATESURFACE2DUP       = 0x1106,
    CM_FN_CMDEVICE_DESTROYSURFACE2DUP      = 0x1107,
    CM_FN_CMDEVICE_GETSURFACE2DINFO        = 0x1108,
    CM_FN_CMDEVICE_CREATESURFACE3D         = 0x1109,
    CM_FN_CMDEVICE_DESTROYSURFACE3D        = 0x110A,
    CM_FN_CMDEVICE_CREATEQUEUE             = 0x110B,
    CM_FN_CMDEVICE_LOADPROGRAM             = 0x110C,
    CM_FN_CMDEVICE_DESTROYPROGRAM          = 0x110D,
    CM_FN_CMDEVICE_CREATEKERNEL            = 0x110E,
    CM_FN_CMDEVICE_DESTROYKERNEL           = 0x110F,
    CM_FN_CMDEVICE_CREATETASK              = 0x1110,
    CM_FN_CMDEVICE_DESTROYTASK             = 0x1111,
    CM_FN_CMDEVICE_GETCAPS                 = 0x1112,
    CM_FN_CMDEVICE_SETCAPS                 = 0x1113,
    CM_FN_CMDEVICE_CREATETHREADSPACE       = 0x1114,
    CM_FN_CMDEVICE_DESTROYTHREADSPACE      = 0x1115,
    CM_FN_CMDEVICE_CREATETHREADGROUPSPACE  = 0x1116,
    CM_FN_CMDEVICE_DESTROYTHREADGROUPSPACE = 0x1117,
    CM_FN_CMDEVICE_SETL3CONFIG             = 0x1118,
    CM_FN_CMDEVICE_SETSUGGESTEDL3CONFIG    = 0x1119,
    CM_FN_CMDEVICE_CREATESAMPLER           = 0x111A,
    CM_FN_CMDEVICE_DESTROYSAMPLER          = 0x111B,
    CM_FN_CMDEVICE_CREATESAMPLER8X8        = 0x111C,
    CM_FN_CMDEVICE_DESTROYSAMPLER8X8       = 0x111D,
    CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE = 0x111E,
    CM_FN_CMDEVICE_DESTROYSAMPLER8X8SURFACE= 0x111F,
    CM_FN_CMDEVICE_DESTROYVMESURFACE       = 0x1123,
    CM_FN_CMDEVICE_CREATEVMESURFACEG7_5    = 0x1124,
    CM_FN_CMDEVICE_DESTROYVMESURFACEG7_5   = 0x1125,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D  = 0x1126,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE3D  = 0x1127,
    CM_FN_CMDEVICE_DESTROYSAMPLERSURFACE   = 0x1128,
    CM_FN_CMDEVICE_ENABLE_GTPIN            = 0X112A,
    CM_FN_CMDEVICE_INIT_PRINT_BUFFER       = 0x112C,
    CM_FN_CMDEVICE_CREATEVEBOX             = 0x112D,
    CM_FN_CMDEVICE_DESTROYVEBOX            = 0x112E,
    CM_FN_CMDEVICE_CREATEBUFFERSVM         = 0x1131,
    CM_FN_CMDEVICE_DESTROYBUFFERSVM        = 0x1132,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2DUP= 0x1133,
    CM_FN_CMDEVICE_CHECK_EXISTING_2DWRAPPER= 0x1135,
    CM_FN_CMDEVICE_REGISTER_GTPIN_MARKERS  = 0x1136,
    CM_FN_CMDEVICE_CLONEKERNEL             = 0x1137,
    CM_FN_CMDEVICE_CREATESURFACE2D_ALIAS   = 0x1138,
    CM_FN_CMDEVICE_CREATESAMPLER_EX        = 0x1139,
    CM_FN_CMDEVICE_CREATESAMPLER8X8SURFACE_EX = 0x113A,
    CM_FN_CMDEVICE_CREATESAMPLERSURFACE2D_EX = 0x113B,
    CM_FN_CMDEVICE_CREATEBUFFER_ALIAS      = 0x113D,
    CM_FN_CMDEVICE_CONFIGVMESURFACEDIMENSION = 0x113E,
    CM_FN_CMDEVICE_CREATEHEVCVMESURFACEG10 = 0x113F,
    CM_FN_CMDEVICE_GETVISAVERSION          = 0x1140,


    CM_FN_CMQUEUE_ENQUEUE                  = 0x1500,
    CM_FN_CMQUEUE_DESTROYEVENT             = 0x1501,
    CM_FN_CMQUEUE_ENQUEUECOPY              = 0x1502,
    CM_FN_CMQUEUE_ENQUEUEWITHGROUP         = 0x1504,
    CM_FN_CMQUEUE_ENQUEUESURF2DINIT        = 0x1505,
    CM_FN_CMQUEUE_ENQUEUECOPY_V2V          = 0x1506,
    CM_FN_CMQUEUE_ENQUEUECOPY_L2L          = 0x1507,
    CM_FN_CMQUEUE_ENQUEUEVEBOX             = 0x1508,
    CM_FN_CMQUEUE_ENQUEUEWITHHINTS         = 0x1509,

}CM_FUNCTION_ID;

#if defined(__cplusplus)
extern "C" {
#endif
int32_t CmThinExecute(
    VADriverContextP    pVaDrvCtx,
    void                *pCmDeviceHandle,
    uint32_t            inputFunctionId,  
    void                *inputData, 
    uint32_t            inputDataLen 
    );
#if defined(__cplusplus)
};
#endif

