/*
* Copyright (c) 2023-2026, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings_xe3p_lpm_base.h
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_AV1_VDENC_CONST_SETTINGS_XE3P_LPM_BASE_H__
#define __ENCODE_AV1_VDENC_CONST_SETTINGS_XE3P_LPM_BASE_H__

#include "encode_av1_vdenc_const_settings.h"

namespace encode
{

class EncodeAv1VdencConstSettingsXe3P_Lpm_Base : public EncodeAv1VdencConstSettings
{
public:

    //!
    //! \brief  EncodeAv1VdencConstSettingsXe3P_Lpm_Base constructor
    //!
    EncodeAv1VdencConstSettingsXe3P_Lpm_Base(PMOS_INTERFACE osInterface) :
        EncodeAv1VdencConstSettings (osInterface) {}

    //!
    //! \brief  EncodeAv1VdencConstSettingsXe3P_Lpm_Base deconstructor
    //!
    virtual ~EncodeAv1VdencConstSettingsXe3P_Lpm_Base() {}

    //!
    //! \brief  Prepare CMD1 TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencCmd1Settings() override;

    //!
    //! \brief  Prepare CMD2 TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencCmd2Settings() override;

    //!
    //! \brief  Prepare StreamIn TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencStreaminStateSettings() override;

MEDIA_CLASS_DEFINE_END(encode__EncodeAv1VdencConstSettingsXe3P_Lpm_Base)
};


}
#endif // !__ENCODE_AV1_VDENC_CONST_SETTINGS_XE3P_LPM_BASE_H__
