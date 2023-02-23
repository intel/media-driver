/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_vp9_vdenc_const_settings_xe_lpm_plus.h
//! \brief    Defines the common interface for vp9 vdenc const settings
//!

#ifndef __ENCODE_VP9_VDENC_CONST_SETTINGS_XE_LPM_PLUS_H__
#define __ENCODE_VP9_VDENC_CONST_SETTINGS_XE_LPM_PLUS_H__

#include "codec_def_common_encode.h"
#include "encode_vp9_vdenc_const_settings.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include <stdint.h>

namespace encode
{

class EncodeVp9VdencConstSettingsXe_Lpm_Plus : public EncodeVp9VdencConstSettings
{
public:
    //!
    //! \brief  EncodeVp9VdencConstSettingsXe_Lpm_Plus deconstructor
    //!
    virtual ~EncodeVp9VdencConstSettingsXe_Lpm_Plus() {}

protected:
    //!
    //! \brief  Prepare TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTUSettings() override;

    virtual MOS_STATUS SetVdencCmd1Settings() override;

    virtual MOS_STATUS SetVdencCmd2Settings() override;

    static const uint32_t m_numMergeCandidateCu64x64Xe_Lpm_Plus[NUM_TARGET_USAGE_MODES + 1];
    static const uint32_t m_numMergeCandidateCu32x32Xe_Lpm_Plus[NUM_TARGET_USAGE_MODES + 1];
    static const uint32_t m_numMergeCandidateCu16x16Xe_Lpm_Plus[NUM_TARGET_USAGE_MODES + 1];
    static const uint32_t m_numMergeCandidateCu8x8Xe_Lpm_Plus[NUM_TARGET_USAGE_MODES + 1];
    static const uint8_t  m_numImePredictorsXe_Lpm_Plus[NUM_TARGET_USAGE_MODES + 1];

MEDIA_CLASS_DEFINE_END(encode__EncodeVp9VdencConstSettingsXe_Lpm_Plus)
};

}  // namespace encode
#endif  // !__ENCODE_VP9_VDENC_CONST_SETTINGS_XE_LPM_PLUS_H__
