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
//! \file     decode_hevc_user_setting.cpp
//! \brief    Defines the interface for hevc decode pipeline
//!
#include "decode_hevc_pipeline.h"

namespace decode
{
MOS_STATUS HevcPipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(DecodePipeline::InitUserSetting(userSettingPtr));
    DeclareUserSettingKey(
        userSettingPtr,
        "HEVC Decode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "FE Separate Submission In Use",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "RT Decoded Count",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "SP Decoded Count",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Huc Load Fail",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "HuC Report Critical Error",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Disable HEVC Real Tile Decode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Enable HEVC Real Tile Multi Phase Decode",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Enable HEVC SF 2 DMA Submits",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "DisableHevcDatRowStoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "DisableHevcDfRowStoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "DisableHevcSaoRowStoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "HCP Decode Always Frame Split",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "HCP Decode Mode Switch TH1",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "HCP Decode Mode Switch TH2",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Scalability Split Width In MinCb",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "HCP Decode User Pipe Num",
        MediaUserSetting::Group::Sequence,
        int32_t(2),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "ApogeiosHevcdEnable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
#endif
    return MOS_STATUS_SUCCESS;
}
}  // namespace decode