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
//! \file     encode_hevc_vdenc_const_settings_xe2_lpm_base.h
//! \brief    Defines the common interface for Xe2_LPM+ Base henvc vdenc const settings
//!

#ifndef __ENCODE_HEVC_VDENC_CONST_SETTINGS_XE2_LPM_BASE_H__
#define __ENCODE_HEVC_VDENC_CONST_SETTINGS_XE2_LPM_BASE_H__

#include "encode_hevc_vdenc_const_settings.h"
#include "codec_def_common_encode.h"
namespace encode
{

class EncodeHevcVdencConstSettingsXe2_Lpm_Base : public EncodeHevcVdencConstSettings
{
public:

    //!
    //! \brief  EncodeHevcVdencConstSettingsXe2_Lpm_Base deconstructor
    //!
    virtual ~EncodeHevcVdencConstSettingsXe2_Lpm_Base() {}
protected:

    //!
    //! \brief  Prepare TU related settings
    //! \param  [in] targetUsage
    //!         target usage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetTUSettings() override;

    MOS_STATUS SetVdencStreaminStateSettings() override;

    MOS_STATUS SetVdencCmd1Settings() override;

    MOS_STATUS SetVdencCmd2Settings() override;

MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencConstSettingsXe2_Lpm_Base)
};

}
#endif // !__ENCODE_HEVC_VDENC_CONST_SETTINGS_XE2_LPM_BASE_H__
