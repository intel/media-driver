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
//! \file     encode_avc_user_setting.cpp
//! \brief    Initialize user setting of AVC
//!

#include "encode_avc_vdenc_pipeline.h"

namespace encode
{
MOS_STATUS AvcVdencPipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(EncodePipeline::InitUserSetting(userSettingPtr));

    DeclareUserSettingKey(
        userSettingPtr,
        "Encode Suppress Recon Pic",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);

    DeclareUserSettingKey(
        userSettingPtr,
        "AVC Encode Adaptive Rounding Inter Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);

    DeclareUserSettingKey(
        userSettingPtr,
        "AVC Encode MB BRC",
        MediaUserSetting::Group::Sequence,
        int32_t(255),
        false);

    DeclareUserSettingKey(
        userSettingPtr,
        "VDEnc Single Pass Enable",
        MediaUserSetting::Group::Sequence,
        uint32_t(0),
        false);

    DeclareUserSettingKey(
        userSettingPtr,
        "Codec MMC In Use",
        MediaUserSetting::Group::Frame,
        int32_t(0),
        true);

    DeclareUserSettingKey(
        userSettingPtr,
        "AVC Encode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Encode TU Override",
        MediaUserSetting::Group::Sequence,
        uint32_t(0),
        false);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AVC VDEnc PerMB StreamOut Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AVC VDEnc TCBRC ARB Disable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "VDENC In Use",
        MediaUserSetting::Group::Frame,
        int32_t(0),
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Enable Encode VE CtxBasedScheduling",
        MediaUserSetting::Group::Frame,
        false,
        true);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Lpla TargetBufferFulness Data Address",
        MediaUserSetting::Group::Sequence,
        "",
        false);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "DisableInputSurfaceCopy",
        MediaUserSetting::Group::Sequence,
        false,
        false);
#endif
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
