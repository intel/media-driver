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
//! \file     encode_av1_vdenc_const_settings_xe_hpm.cpp
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_const_settings_xe_hpm.h"
#include "encode_utils.h"
#include "mos_solo_generic.h"

namespace encode
{

MOS_STATUS EncodeAv1VdencConstSettingsXe_Hpm::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

#if _MEDIA_RESERVED
#define VDENC_CMD2_SETTINGS_EXT
    #include "encode_av1_vdenc_const_settings_xe_hpm_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#else
    setting->vdencCmd2Settings.emplace_back(
        VDENC_CMD2_LAMBDA() {
            par.extSettings.emplace_back(
                [this, isLowDelay, &par](uint32_t *data) {
                    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
                    ENCODE_CHK_NULL_RETURN(waTable);
                    uint32_t TargetUsage        = m_av1SeqParams->TargetUsage;
                    uint32_t lowDelay           = isLowDelay;
                    uint32_t frameType          = m_av1PicParams->PicFlags.fields.frame_type;
                    uint32_t l1RefFrameCtrlNot0 = (m_av1PicParams->ref_frame_ctrl_l1.RefFrameCtrl.value != 0);
                    uint32_t Wa_2209975292      = MEDIA_IS_WA(waTable, Wa_2209975292);
                    uint32_t Wa_22011549751     = MEDIA_IS_WA(waTable, Wa_22011549751);
                    uint32_t l0RefNum           = par.numRefL0;
                    uint32_t Wa_14010476401     = MEDIA_IS_WA(waTable, Wa_14010476401);
                    uint32_t Wa_22011531258     = MEDIA_IS_WA(waTable, Wa_22011531258);
                    uint32_t l1RefNum           = par.numRefL1;

                    static const uint32_t dw6Lut[2] = { 0xe0080200, 0xe88a2288,};
                    data[6] |= dw6Lut[Wa_2209975292];

                    static const uint32_t dw7Lut[2][4][2][4] = { { { { 0x64003, 0x64003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},}, { { 0x64003, 0x64003, 0x64003, 0x64003,}, { 0x64003, 0x64003, 0x64003, 0x64003,},}, { { 0x64003, 0x64003, 0x64003, 0x64003,}, { 0x64003, 0x64003, 0x64003, 0x64003,},}, { { 0x64003, 0x64003, 0x64003, 0x64003,}, { 0x64003, 0x64003, 0x64003, 0x64003,},},}, { { { 0xe4003, 0xe4003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},}, { { 0xe4003, 0xe4003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},}, { { 0xe4003, 0xe4003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},}, { { 0xe4003, 0xe4003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},},},};
                    data[7] |= dw7Lut[lowDelay][frameType][Wa_22011549751][l0RefNum];

                    static const uint32_t dw8Lut[8][2][4][2] = { { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdfcaa,}, { 0xfffdccaa, 0xfffdfcaa,}, { 0xfffdccaa, 0xfffdfcaa,},}, { { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdccaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdfcaa,}, { 0xfffdccaa, 0xfffdfcaa,}, { 0xfffdccaa, 0xfffdfcaa,},}, { { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdccaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,}, { 0x55550000, 0x55550000,},},},};
                    data[8] |= dw8Lut[TargetUsage][lowDelay][frameType][l1RefFrameCtrlNot0];

                    static const uint32_t dw9Lut[8][2][4][2][2] = { { { { { 0xc45555, 0x45555,}, { 0xc45555, 0x45555,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},}, { { { 0xc40000, 0x40000,}, { 0xc40000, 0x40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},},}, { { { { 0xc45555, 0x45555,}, { 0xc45555, 0x45555,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},}, { { { 0xc40000, 0x40000,}, { 0xc40000, 0x40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},}, { { 0xc40000, 0xc40000,}, { 0xc40000, 0xc40000,},},},}, { { { { 0xc45555, 0x45555,}, { 0xc45555, 0x45555,},}, { { 0x84ffff, 0x84ffff,}, { 0x84fcff, 0x84fcff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84fcff, 0x84fcff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84fcff, 0x84fcff,},},}, { { { 0xc40000, 0x40000,}, { 0xc40000, 0x40000,},}, { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},}, { { 0x84ffff, 0x84ffff,}, { 0x84ffff, 0x84ffff,},},},}, { { { { 0x835555, 0x35555,}, { 0x835555, 0x35555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x830000, 0x30000,}, { 0x830000, 0x30000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},},}, { { { { 0x835555, 0x35555,}, { 0x835555, 0x35555,},}, { { 0x63ffff, 0x63ffff,}, { 0x63fcff, 0x63fcff,},}, { { 0x63ffff, 0x63ffff,}, { 0x63fcff, 0x63fcff,},}, { { 0x63ffff, 0x63ffff,}, { 0x63fcff, 0x63fcff,},},}, { { { 0x830000, 0x30000,}, { 0x830000, 0x30000,},}, { { 0x63ffff, 0x63ffff,}, { 0x63ffff, 0x63ffff,},}, { { 0x63ffff, 0x63ffff,}, { 0x63ffff, 0x63ffff,},}, { { 0x63ffff, 0x63ffff,}, { 0x63ffff, 0x63ffff,},},},}, { { { { 0x835555, 0x35555,}, { 0x835555, 0x35555,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},}, { { { 0x830000, 0x30000,}, { 0x830000, 0x30000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},}, { { 0x830000, 0x830000,}, { 0x830000, 0x830000,},},},}, { { { { 0x425555, 0x25555,}, { 0x425555, 0x25555,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x420000, 0x20000,}, { 0x420000, 0x20000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},},}, { { { { 0x425555, 0x25555,}, { 0x425555, 0x25555,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x420000, 0x20000,}, { 0x420000, 0x20000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},},},};
                    data[9] |= dw9Lut[TargetUsage][lowDelay][frameType][l1RefFrameCtrlNot0][Wa_22011549751];

                    static const uint32_t dw11Lut[2][4][4] = { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x80000000, 0, 0,}, { 0x80000000, 0, 0, 0,},}, { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x80000000, 0, 0,}, { 0x80000000, 0, 0, 0x80000000,},},};
                    data[11] |= dw11Lut[lowDelay][l0RefNum][l1RefNum];

                    static const uint32_t dw51Lut[8][4][2] = { { { 0x33331552, 0x20001552,}, { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x20001552,}, { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x33331552, 0x20001552,}, { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,}, { 0x33331552, 0x33331552,},}, { { 0x22223552, 0x20003552,}, { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x20003552,}, { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22223552, 0x20003552,}, { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,}, { 0x22223552, 0x22223552,},}, { { 0x22227152, 0x20007152,}, { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},}, { { 0x22227152, 0x20007152,}, { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,}, { 0x22227152, 0x22227152,},},};
                    data[51] |= dw51Lut[TargetUsage][frameType][Wa_22011549751];

                    static const uint32_t dw52Lut[8] = { 0x77f5bdb, 0x77f5bdb, 0x77f5bdb, 0x72d595b, 0x72d595b, 0x72d595b, 0x929595a, 0x929595a,};
                    data[52] |= dw52Lut[TargetUsage];

                    static const uint32_t dw53Lut[8][4][2][2] = { { { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},},}, { { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},}, { { 0xffff0000, 0xfffffff0,}, { 0xffff0000, 0xfffffff0,},},}, { { { 0xffffffff, 0xffffffff,}, { 0x80000000, 0x80000000,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},}, { { 0xffffffff, 0xfffffff0,}, { 0xffffffff, 0xfffffff0,},},}, { { { 0xff00ff00, 0xff00ff00,}, { 0x80000000, 0x80000000,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},}, { { { 0xff00ff00, 0xff00ff00,}, { 0x80000000, 0x80000000,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},}, { { { 0xff00ff00, 0xff00ff00,}, { 0x80000000, 0x80000000,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},}, { { 0xff000000, 0xff00fff0,}, { 0xff000000, 0xff00fff0,},},}, { { { 0x8000fffc, 0x8000fffc,}, { 0x80000000, 0x80000000,},}, { { 0x80000000, 0x8000fff0,}, { 0x80000000, 0x8000fff0,},}, { { 0x80000000, 0x8000fff0,}, { 0x80000000, 0x8000fff0,},}, { { 0x80000000, 0x8000fff0,}, { 0x80000000, 0x8000fff0,},},}, { { { 0x8000fffc, 0x8000fffc,}, { 0x80000000, 0x80000000,},}, { { 0x80000000, 0x8000fff0,}, { 0x80000000, 0x8000fff0,},}, { { 0x80000000, 0x8000fff0,}, { 0x80000000, 0x8000fff0,},}, { { 0x80000000, 0x8000fff0,}, { 0x80000000, 0x8000fff0,},},},};
                    data[53] |= dw53Lut[TargetUsage][frameType][Wa_22011549751][Wa_14010476401];

                    static const uint32_t dw54Lut[8][2] = { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0x44000000, 0x44000000,}, { 0x44000000, 0x44000000,}, { 0x4000000, 0x4000000,}, { 0xbc000004, 0x34000004,}, { 0xbc000004, 0x34000004,},};
                    data[54] |= dw54Lut[TargetUsage][Wa_22011531258];

                    static const uint32_t dw56Lut[2][4][4] = { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0x300, 0, 0, 0,},}, { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0x300, 0, 0, 0x300,},},};
                    data[56] |= dw56Lut[lowDelay][l0RefNum][l1RefNum];

                    static const uint32_t dwsLut[] = {0x3, 0xc0ac80, 0xffffffff, 0x1f40000, 0x13881388, 0xf000000, 0x3e8, 0x80000, 0x98000040, 0xffff, 0xffff0000, 0x7d00fa0, 0x2bc0bb8, 0x32003e8, 0x1f4012c, 0x190, 0x800, 0x40, 0x8000fc, 0xb10080, 0x300aa, 0xd30069, 0xe000e9, 0x940003, 0x56004d, 0x9500fd, 0x17002d, 0xfd001f, 0x2006c, 0x800080,};
                    data[2] |= dwsLut[0];
                    data[5] |= dwsLut[1];
                    data[12] |= dwsLut[2];
                    data[14] |= dwsLut[3];
                    data[15] |= dwsLut[4];
                    data[16] |= dwsLut[5];
                    data[17] |= dwsLut[6];
                    data[18] |= dwsLut[7];
                    data[19] |= dwsLut[8];
                    data[20] |= dwsLut[9];
                    data[27] |= dwsLut[10];
                    data[28] |= dwsLut[11];
                    data[29] |= dwsLut[12];
                    data[30] |= dwsLut[13];
                    data[31] |= dwsLut[14];
                    data[32] |= dwsLut[15];
                    data[35] |= dwsLut[16];
                    data[37] |= dwsLut[17];
                    data[39] |= dwsLut[18];
                    data[40] |= dwsLut[19];
                    data[41] |= dwsLut[20];
                    data[42] |= dwsLut[21];
                    data[43] |= dwsLut[22];
                    data[44] |= dwsLut[23];
                    data[45] |= dwsLut[24];
                    data[46] |= dwsLut[25];
                    data[47] |= dwsLut[26];
                    data[48] |= dwsLut[27];
                    data[49] |= dwsLut[28];
                    data[50] |= dwsLut[29];

                    return MOS_STATUS_SUCCESS;
                });

            return MOS_STATUS_SUCCESS;
        });
#endif  // _MEDIA_RESERVED

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
