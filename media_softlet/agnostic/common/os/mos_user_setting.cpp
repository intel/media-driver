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
//! \file        mos_user_setting.cpp
//! \brief       This file defines the user settings of mos
//! \details  Common OS debug across different platform
//!

#include "mos_user_setting.h"

MOS_STATUS MosUserSetting::InitMosUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    InitMosCommonUserSetting(userSettingPtr);
#if MOS_MESSAGES_ENABLED
    InitMosMessageUserSetting(userSettingPtr);
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    InitUserSettingForDebug(userSettingPtr);
#endif

    InitMosUserSettingSpecific(userSettingPtr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosUserSetting::InitMosCommonUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    DeclareUserSettingKey(
        userSettingPtr,
        "ApoMosEnable",
        MediaUserSetting::Group::Device,
        0,
        true);
    
    DeclareUserSettingKey(
        userSettingPtr,
        "ApoDdiEnable",
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        "Enable Compressible Surface Creation",
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_MEMNINJA_COUNTER,
        MediaUserSetting::Group::Device,
        0,
        true);

    return MOS_STATUS_SUCCESS;
}

#if MOS_MESSAGES_ENABLED
MOS_STATUS MosUserSetting::InitMosMessageUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_DISABLE_ASSERT,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_ENABLED,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_HLT_OUTPUT_DIRECTORY,
        MediaUserSetting::Group::Device,
        "",
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_PRINT_ENABLED,
        MediaUserSetting::Group::Device,
        1,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_OS_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_OS,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_OS_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_MHW_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MHW,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MHW_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_CODEC_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CODEC,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CODEC_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_VP_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_VP,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_VP_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_CP_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CP,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_DDI_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_DDI,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_DDI_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_CM_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_CM,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CM_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_SCALABILITY_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_SCALABILITY,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_SCALABILITY_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_MMC_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MMC,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MMC_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_MESSAGE_MCPY_TAG,
        MediaUserSetting::Group::Device,
        __MOS_USER_FEATURE_KEY_MESSAGE_DEFAULT_VALUE,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_BY_SUB_COMPONENT_MCPY,
        MediaUserSetting::Group::Device,
        0,
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_SUB_COMPONENT_MCPY_TAG,
        MediaUserSetting::Group::Device,
        0,
        true);

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_DDI_DUMP_DIRECTORY,
        MediaUserSetting::Group::Device,
        "",
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        __MOS_USER_FEATURE_KEY_ENCODE_DDI_DUMP_ENABLE,
        MediaUserSetting::Group::Device,
        0,
        true);
#endif
    return MOS_STATUS_SUCCESS;
}
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS MosUserSetting::InitUserSettingForDebug(MediaUserSettingSharedPtr userSettingPtr)
{
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Resource Addr Dump Enable",
        MediaUserSetting::Group::Device,
        0,
        true);

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS MosUserSetting::DestroyMediaUserSetting()
{
    MediaUserSetting::MediaUserSetting::Destroy();
    return MOS_STATUS_SUCCESS;
}
