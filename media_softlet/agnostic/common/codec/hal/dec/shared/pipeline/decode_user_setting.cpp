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
//! \file     decode_user_setting.cpp
//! \brief    Initialize user setting of decode
//!

#include "decode_pipeline.h"

namespace decode
{
MOS_STATUS DecodePipeline::InitUserSetting(MediaUserSettingSharedPtr userSettingPtr)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_STATUS(MediaPipeline::InitUserSetting(userSettingPtr));
    DeclareUserSettingKey(
        userSettingPtr,
        "Enable Decode MMC",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1BtdlRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1SmvlRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1IpdlRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1DflyRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1DfluRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1DflvRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "DisableAv1CdefRowstoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode Histogram Debug",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode MMC In Use",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode Extended MMC In Use",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode RT Compressible",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode RT Compress Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Num of Codec Devices on VDBOX1",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Num of Codec Devices on VDBOX2",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Media Reset Count",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "VC1 Decode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "AV1 Decode Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode Single Task Phase Enable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode RT Compressible",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "Decode RT Compress Mode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKey(
        userSettingPtr,
        "VT Decoded Count",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKey(
        userSettingPtr,
        "Av1 Error Status Addr Value",
        MediaUserSetting::Group::Sequence,
        uint32_t(0),
        true);
#if (_DEBUG || _RELEASE_INTERNAL)
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Disable Decode Lock",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);

    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "DisableMprRowStoreCache",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Stream Out",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Decompress Decode Output",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Decompress Decode Sfc Output",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Status Reporting",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Watchdog Timer Threshold",
        MediaUserSetting::Group::Sequence,
        int32_t(120),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Used VDBOX ID",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Delay Miliseconds",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Decode SFC RGB Format Output",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Decode SFC Linear Output Debug",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Force Av1 Tile Based Decode",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "Enable AVP Scalability Decode",
        MediaUserSetting::Group::Sequence,
        int32_t(120),
        false);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "ApogeiosAv1dEnable",
        MediaUserSetting::Group::Sequence,
        int32_t(1),
        true);
    DeclareUserSettingKeyForDebug(
        userSettingPtr,
        "CRCDebugModeEnable",
        MediaUserSetting::Group::Sequence,
        int32_t(0),
        true);
#endif
    return MOS_STATUS_SUCCESS;
}

}  // namespace decode