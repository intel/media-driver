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
//! \file     encode_hevc_vdenc_const_settings_xe_xpm_base.h
//! \brief    Defines the common interface for M12.5 Plus henvc vdenc const settings
//!

#ifndef __ENCODE_HEVC_VDENC_CONST_SETTINGS_XE_XPM_BASE_H__
#define __ENCODE_HEVC_VDENC_CONST_SETTINGS_XE_XPM_BASE_H__

#include "encode_hevc_vdenc_const_settings.h"
#include "media_class_trace.h"
#include "mos_defs.h"

namespace encode
{

class EncodeHevcVdencConstSettingsXe_Xpm_Base : public EncodeHevcVdencConstSettings
{
public:

    //!
    //! \brief  EncodeHevcVdencConstSettingsXe_Xpm_Base deconstructor
    //!
    virtual ~EncodeHevcVdencConstSettingsXe_Xpm_Base() {}
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
MEDIA_CLASS_DEFINE_END(encode__EncodeHevcVdencConstSettingsXe_Xpm_Base)
};


}
#endif // !__ENCODE_HEVC_VDENC_CONST_SETTINGS_XE_XPM_BASE_H__
