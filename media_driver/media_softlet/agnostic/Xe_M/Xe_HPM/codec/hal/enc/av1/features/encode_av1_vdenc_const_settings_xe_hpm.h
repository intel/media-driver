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
//! \file     encode_av1_vdenc_const_settings_xe_hpm.h
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_AV1_VDENC_CONST_SETTINGS_XE_HPM_H__
#define __ENCODE_AV1_VDENC_CONST_SETTINGS_XE_HPM_H__

#include "encode_av1_vdenc_const_settings.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_os.h"
#include "mos_os_specific.h"

namespace encode
{

class EncodeAv1VdencConstSettingsXe_Hpm: public EncodeAv1VdencConstSettings
{
public:

    //!
    //! \brief  EncodeAv1VdencConstSettingsXe_Hpm constructor
    //!
    EncodeAv1VdencConstSettingsXe_Hpm(PMOS_INTERFACE osInterface) :
        EncodeAv1VdencConstSettings (osInterface) {}

    //!
    //! \brief  EncodeAv1VdencConstSettingsXe_Hpm deconstructor
    //!
    virtual ~EncodeAv1VdencConstSettingsXe_Hpm() {}

    //!
    //! \brief  Prepare CMD2 TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencCmd2Settings() override;
MEDIA_CLASS_DEFINE_END(encode__EncodeAv1VdencConstSettingsXe_Hpm)
};

}

#endif // !__ENCODE_AV1_VDENC_CONST_SETTINGS_XE_HPM_H__
