/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings_xe2_hpm.h
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The default setting is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __ENCODE_AV1_VDENC_CONST_SETTINGS_XE2_HPM_H__
#define __ENCODE_AV1_VDENC_CONST_SETTINGS_XE2_HPM_H__

#include "encode_av1_vdenc_const_settings.h"

namespace encode
{

struct Av1VdencTUConstSettingsXe2_Hpm
{
    static const uint8_t  vdencCmd2Par4[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par38[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par39[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par67[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par83Table2[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par83Table1[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par83Table0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par84Table0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par84Table1[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par84Table2[NUM_TARGET_USAGE_MODES];
    static const uint32_t vdencCmd2Par85Table0[NUM_TARGET_USAGE_MODES];
    static const uint32_t vdencCmd2Par85Table1[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par86[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table3[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table2[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table1[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par87Table0[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table13[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table12[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table11[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table10[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table23[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table22[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table21[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table20[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table03[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table02[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table01[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par88Table00[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par89[NUM_TARGET_USAGE_MODES];
    static const uint16_t vdencCmd2Par92[NUM_TARGET_USAGE_MODES];
    static const uint16_t vdencCmd2Par93[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par100[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par94[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par95[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par96[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par97[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par98[NUM_TARGET_USAGE_MODES];
    static const uint16_t vdencCmd2Par14[NUM_TARGET_USAGE_MODES];
    static const uint8_t  temporalMvp[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par18[NUM_TARGET_USAGE_MODES];
    static const uint8_t  vdencCmd2Par15[NUM_TARGET_USAGE_MODES];
    static const uint16_t vdencCmd2Par12[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par23[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par102[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par101[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par133[NUM_TARGET_USAGE_MODES];
    static const bool     vdencCmd2Par109[NUM_TARGET_USAGE_MODES];
};

class EncodeAv1VdencConstSettingsXe2_Hpm : public EncodeAv1VdencConstSettings
{
public:

    //!
    //! \brief  EncodeAv1VdencConstSettingsXe2_Hpm constructor
    //!
    EncodeAv1VdencConstSettingsXe2_Hpm(PMOS_INTERFACE osInterface) :
        EncodeAv1VdencConstSettings(osInterface) {}

    //!
    //! \brief  EncodeAv1VdencConstSettingsXe2_Hpm deconstructor
    //!
    virtual ~EncodeAv1VdencConstSettingsXe2_Hpm() {}

    virtual MOS_STATUS SetVdencCmd1Settings() override;
    //!
    //! \brief  Prepare CMD2 TU related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencCmd2Settings() override;

MEDIA_CLASS_DEFINE_END(encode__EncodeAv1VdencConstSettingsXe2_Hpm)
};


}
#endif // !__ENCODE_AV1_VDENC_CONST_SETTINGS_XE2_HPM_H__
