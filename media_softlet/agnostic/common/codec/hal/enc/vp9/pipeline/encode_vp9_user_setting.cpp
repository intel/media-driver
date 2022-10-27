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
//! \file     encode_vp9_user_setting.cpp
//! \brief    Initialize user setting of VP9
//!
#include "encode_vp9_pipeline.h"
#include "encode_utils.h"

namespace encode
{
MOS_STATUS Vp9Pipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::InitUserSetting(userSettingPtr));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode HUC Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode Single Pass Dys Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode HME",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode SuperHME",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode Multipass BRC Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode Adaptive RePAK In Use",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true));

    ENCODE_CHK_STATUS_RETURN(DeclareUserSettingKeyValue(
        "VP9 Encode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true));

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
