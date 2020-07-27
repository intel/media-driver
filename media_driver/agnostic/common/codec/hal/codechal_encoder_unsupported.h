/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     codechal_encoder_unsupported.h
//! \brief    Defines the encode interface for Codechal unsupported platform.
//! \details  The encode interface is further sub-divided by standard, this file is for the base interface which is shared by all encode standards.
//!

#ifndef __CODECHAL_ENCODER_UNSUPPORTED_H__
#define __CODECHAL_ENCODER_UNSUPPORTED_H__

#include "codechal_encoder_base.h"


//!
//! \class CodechalEncoderStateUnsupported
//! \brief This class implement all functions to return MOS_STATUS_PLATFORM_NOT_SUPPORTED error code.
//!
class CodechalEncoderStateUnsupported : public CodechalEncoderState
{
public:

    CodechalEncoderStateUnsupported(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo)
        : CodechalEncoderState(hwInterface, debugInterface, standardInfo)
    {
    }

    virtual ~CodechalEncoderStateUnsupported()
    {
    }

    MOS_STATUS Allocate(CodechalSetting* codecHalSettings) override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS Execute(void* params) override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    virtual MOS_STATUS InitializePicture(const EncoderParams& params) override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    };

    virtual MOS_STATUS ExecuteKernelFunctions() override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    };

    virtual MOS_STATUS ExecutePictureLevel() override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    };

    virtual MOS_STATUS ExecuteSliceLevel() override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    };

    MOS_STATUS GetStatusReport(void* status, uint16_t numStatus) override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    virtual MOS_STATUS GetStatusReport(EncodeStatus* encodeStatus, EncodeStatusReport* encodeStatusReport) override
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    };
};

#endif  // __CODECHAL_ENCODER_UNSUPPORTED_H__
