/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_avc_user_setting.cpp
//! \brief    Defines the interface for avc decode pipeline
//!
#include "decode_avc_pipeline.h"

namespace decode
{
MOS_STATUS AvcPipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::InitUserSetting(userSettingPtr));
    DeclareUserSettingKey(
        userSettingPtr,
        "AVC Decode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    return MOS_STATUS_SUCCESS;
}
}  // namespace decode