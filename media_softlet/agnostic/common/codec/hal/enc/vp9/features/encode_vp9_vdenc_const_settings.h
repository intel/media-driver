/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     encode_vp9_vdenc_const_settings.h
//! \brief    Defines the common interface for vp9 vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_VP9_VDENC_CONST_SETTINGS_H__
#define __ENCODE_VP9_VDENC_CONST_SETTINGS_H__
#include <stdint.h>
#include "codec_def_common_encode.h"
#include "media_class_trace.h"
#include "encode_const_settings.h"
#include "mos_defs.h"
#include "codec_def_encode_vp9.h"
#include "codec_def_encode.h"
namespace encode
{
struct Vp9VdencFeatureSettings : VdencFeatureSettings
{
    uint32_t *NumMergeCandidateCu64x64 = nullptr;
    uint32_t *NumMergeCandidateCu32x32 = nullptr;
    uint32_t *NumMergeCandidateCu16x16 = nullptr;
    uint32_t *NumMergeCandidateCu8x8   = nullptr;
    uint8_t * NumImePredictors         = nullptr;
};

class EncodeVp9VdencConstSettings : public VdencConstSettings
{
public:
    //!
    //! \brief  EncodeVp9VdencConstSettings constructor
    //!
    EncodeVp9VdencConstSettings();

    //!
    //! \brief  EncodeVp9VdencConstSettings deconstructor
    //!
    virtual ~EncodeVp9VdencConstSettings() {}

    //!
    //! \brief  Prepare const settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareConstSettings() override;
    virtual MOS_STATUS Update(void *params) override;

protected:
    //!
    //! \brief  Prepare TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetTUSettings() override;

    virtual MOS_STATUS SetVdencCmd1Settings() override;

    virtual MOS_STATUS SetVdencCmd2Settings() override;

    virtual MOS_STATUS SetBrcSettings() override;


    static const uint32_t m_numMergeCandidateCu64x64[NUM_TARGET_USAGE_MODES + 1];
    static const uint32_t m_numMergeCandidateCu32x32[NUM_TARGET_USAGE_MODES + 1];
    static const uint32_t m_numMergeCandidateCu16x16[NUM_TARGET_USAGE_MODES + 1];
    static const uint32_t m_numMergeCandidateCu8x8[NUM_TARGET_USAGE_MODES + 1];
    static const uint8_t  m_numImePredictors[NUM_TARGET_USAGE_MODES + 1];

    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS  m_vp9SeqParams = nullptr;
    PCODEC_VP9_ENCODE_PIC_PARAMS      m_vp9PicParams = nullptr;
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS   m_vp9SegmentParams = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeVp9VdencConstSettings)
};

}  // namespace encode
#endif  // !__ENCODE_VP9_VDENC_CONST_SETTINGS_H__
