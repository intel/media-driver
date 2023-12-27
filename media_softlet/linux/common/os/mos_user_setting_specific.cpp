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
//! \file        mos_user_setting_specific.cpp
//! \brief       This file defines the user settings keys for all components
//! \details  Common OS debug across different platform
//!

#include "mos_user_setting.h"

MOS_STATUS MosUserSetting::InitMosUserSettingSpecific(MediaUserSettingSharedPtr userSettingPtr)
{
    DeclareUserSettingKey(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_SOFTPIN,
        MediaUserSetting::Group::Device,
        1,
        true); //"Switch between softpin and relocation."

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_DISABLE_KMD_WATCHDOG,
        MediaUserSetting::Group::Device,
        0,
        true); //"Disable KMD Watchdog"
#endif

    DeclareUserSettingKey(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VM_BIND,
        MediaUserSetting::Group::Device,
        0,
        true); //"Enable VM Bind."

    DeclareUserSettingKey(
        userSettingPtr,
        "INTEL MEDIA ALLOC MODE",
        MediaUserSetting::Group::Device,
        0,
        false); //

    return MOS_STATUS_SUCCESS;
}
