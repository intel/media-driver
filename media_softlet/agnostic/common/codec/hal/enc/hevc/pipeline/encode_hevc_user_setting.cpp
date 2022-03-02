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
//! \file     encode_hevc_user_setting.cpp
//! \brief    Initialize user setting of HEVC 
//!
#include "encode_hevc_pipeline.h"

namespace encode
{
MOS_STATUS HevcPipeline::InitUserSetting()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(EncodePipeline::InitUserSetting());
#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKey(
        "HEVC Encode RDO Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKeyForDebug(
        "Disable HEVC RDOQ Perf",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);
    DeclareUserSettingKey(
        "HEVC VDEnc TileReplay Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        "HEVC VDEnc Rounding Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true);
    DeclareUserSettingKey(
        "HEVC VDEnc ACQP Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        "HEVC RDOQ Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true);
    DeclareUserSettingKey(
        "HEVC VDEnc LBC Only Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        "HEVC VDEnc Force Delta QP Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true);
    DeclareUserSettingKey(
        "HEVC Encode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKeyForDebug(
        "Disable TCBRC ARB for HEVC VDEnc",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        "lpla ds data address",
        MediaUserSetting::Group::Sequence,
        "",
        false);
#endif
    return MOS_STATUS_SUCCESS;
}

}  // namespace encode