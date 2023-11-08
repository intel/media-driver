/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_user_setting.cpp
//! \brief    Initialize user setting of media
//!
#include "media_pipeline.h"
MOS_STATUS MediaPipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    DeclareUserSettingKey(
        userSettingPtr,
        "Lockable Resource",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableTlbPrefetch",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Enable VECopy For Surface Dump",
        MediaUserSetting::Group::Sequence,
        false,
        false);
#endif  // _DEBUG || _RELEASE_INTERNAL

    return MOS_STATUS_SUCCESS;
}
