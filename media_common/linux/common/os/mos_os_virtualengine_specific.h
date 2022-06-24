/*
* Copyright (c) 2016-2022, Intel Corporation
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
//! \file     mos_os_virtualengine_specific.h
//! \brief    Defines MOS Virtual engine Inteface for Linux.
//! \details  Defines all types, macros, and functions required by MOS Virtual Engine Interface for Linux.
//!

#ifndef __MOS_OS_VIRTUALENGINE_SPECIFIC_H__
#define __MOS_OS_VIRTUALENGINE_SPECIFIC_H__
#include <cstdint>

typedef struct _MOS_SPECIFIC_VE_HINT_PARAMS MOS_VIRTUALENGINE_HINT_PARAMS, *PMOS_VIRTUALENGINE_HINT_PARAMS;

struct _MOS_SPECIFIC_VE_HINT_PARAMS
{
    union
    {
        struct
        {
            uint32_t    UsingSFC                   :  1; // Use SFC or not
            uint32_t    UsingFrameSplit            :  1; // Frame split
            uint32_t    NeedSyncWithPrevious       :  1; // Need to wait until previous submission from the same context is done
            uint32_t    NoReRunAllowed             :  1; // Can't tolerate rerun
            uint32_t    SameEngineAsLastSubmission :  1; // Submit on the same engine as previous, hint only, not hard requirement
            uint32_t    HWRestrictedEngine         :  1; // HW restriction to specific engine
            uint32_t    FrontEndBackEndPresent     :  1; // Frame split decode
#if (_DEBUG || _RELEASE_INTERNAL)
            uint32_t    Reserved                   : 24;
            uint32_t    DebugOverride              :  1; // Debug & validation usage
#else
            uint32_t    Reserved                   : 25;
#endif
        };

        uint32_t    Flags;
    };

    // Number of batch buffers in frame split case (flag UsingFrameSplit is set), not used in all other cases
    uint32_t       BatchBufferCount;

    // Batch buffer addresses in frame split case (flag UsingFrameSplit is set), not used in all other cases
    MOS_RESOURCE   resScalableBatchBufs[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];

    // Logical engine instances to submit workload on; valid only if flag DebugOverride is set.
    uint8_t        EngineInstance[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
};

typedef struct _MOS_TEE_INPUT_OUTPUT_BUFFER_PARAMS
{
    uint32_t                            inputlength;   // input buffer length
    uint32_t                            outputlength;  // output buffer length
    uint64_t                            inputaddress;  // input buffer address
    uint64_t                            outputaddress; // output buffer address
    uint8_t                             MEclient;      // fixed ME client
}MOS_TEE_INPUT_OUTPUT_BUFFER_PARAMS;

struct _MOS_CMD_BUF_ATTRI_VE
{
    int32_t                             bUseVirtualEngineHint;
    MOS_VIRTUALENGINE_HINT_PARAMS       VEngineHintParams;
    int32_t                             bTEEparamspresent;
    MOS_TEE_INPUT_OUTPUT_BUFFER_PARAMS  TEEInputOutPutParams;
};

#endif //__MOS_OS_VIRTUALENGINE_SPECIFIC_H__

