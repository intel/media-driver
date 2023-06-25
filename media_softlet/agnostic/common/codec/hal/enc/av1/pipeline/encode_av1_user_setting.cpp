/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     encode_av1_user_setting.cpp
//! \brief    Initialize user setting of AV1
//!

#include "encode_av1_pipeline.h"

namespace encode
{
MOS_STATUS Av1Pipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::InitUserSetting(userSettingPtr));

#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Enable SW Back Annotation",
        MediaUserSetting::Group::Sequence,
        (int32_t)1,
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Enable SW Stitching",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Use Ext Costs",
        MediaUserSetting::Group::Sequence,
        uint32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Ext Costs File",
        MediaUserSetting::Group::Sequence,
        "",
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Encode Enable NonDefault Mapping",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Encode RDO Enable",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Encode Adaptive Rounding Enable",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Rho Domain Enable",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "AV1 Dual Encoder Force Off",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        false);
#endif
    DeclareUserSettingKey(
        userSettingPtr,
        "AV1 Encode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "AV1 Post CDEF Recon Compressible",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "AV1 Post CDEF Recon Compress Mode",
        MediaUserSetting::Group::Sequence,
        (int32_t)0,
        true);

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode