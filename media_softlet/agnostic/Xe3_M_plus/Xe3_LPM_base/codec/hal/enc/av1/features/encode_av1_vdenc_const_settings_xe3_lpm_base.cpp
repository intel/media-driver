/*
* Copyright (c) 2022-2025, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings_xe3_lpm_base.cpp
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_const_settings_xe3_lpm_base.h"
#include "encode_utils.h"

namespace encode
{
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par4[NUM_TARGET_USAGE_MODES]               = {16, 16, 16, 16, 16, 16, 16, 16};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par38[NUM_TARGET_USAGE_MODES]                    = {4, 4, 4, 4, 4, 4, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par39[NUM_TARGET_USAGE_MODES]                     = {8, 8, 8, 6, 6, 6, 4, 3};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par67[NUM_TARGET_USAGE_MODES]                     = {8, 8, 8, 8, 8, 8, 8, 8};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par83Table2[NUM_TARGET_USAGE_MODES]               = {false, false, false, false, false, false, true, true};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par83Table1[NUM_TARGET_USAGE_MODES]               = {false, false, false, false, false, false, true, true};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par83Table0[NUM_TARGET_USAGE_MODES]                 = {false, false, false, true, true, true, true, true};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par84Table0[NUM_TARGET_USAGE_MODES]             = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par84Table1[NUM_TARGET_USAGE_MODES]           = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par84Table2[NUM_TARGET_USAGE_MODES]           = {0, 0, 0, 0, 0, 0, 0, 0};
const uint32_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par85Table0[NUM_TARGET_USAGE_MODES]            = {0, 0, 0, 3, 3, 3, 3, 3};  // { 0:none, 1:sad, 2:RDE, 3:noneForInternal}
const uint32_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par85Table1[NUM_TARGET_USAGE_MODES]          = {2, 2, 2, 1, 1, 1, 0, 0};  // { 0:none, 1:Sad, 2: RDE, 3:noneForInternal}
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par86[NUM_TARGET_USAGE_MODES]                   = {false, false, false, false, false, false, true, true};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par87Table3[NUM_TARGET_USAGE_MODES]            = {5, 5, 5, 2, 2, 2, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par87Table2[NUM_TARGET_USAGE_MODES]            = {5, 5, 5, 2, 2, 2, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par87Table1[NUM_TARGET_USAGE_MODES]            = {4, 4, 4, 2, 2, 2, 2, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par87Table0[NUM_TARGET_USAGE_MODES]              = {4, 4, 4, 2, 2, 2, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table13[NUM_TARGET_USAGE_MODES]                   = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table12[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table11[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 1, 1, 1, 3, 3};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table10[NUM_TARGET_USAGE_MODES]                     = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table23[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table22[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table21[NUM_TARGET_USAGE_MODES]                   = {3, 3, 3, 3, 3, 3, 3, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table20[NUM_TARGET_USAGE_MODES]                     = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table03[NUM_TARGET_USAGE_MODES]                   = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table02[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table01[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par88Table00[NUM_TARGET_USAGE_MODES]                     = {5, 5, 5, 5, 5, 5, 1, 1};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par89[NUM_TARGET_USAGE_MODES]                   = {false, false, false, false, false, false, true, true};
const uint16_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par92[NUM_TARGET_USAGE_MODES]                    = {0xffff, 0xffff, 0xffff, 0, 0, 0, 0, 0};
const uint16_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par93[NUM_TARGET_USAGE_MODES]            = {0, 0, 0, 0xff00, 0xff00, 0xff00, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par151[NUM_TARGET_USAGE_MODES]                        = {0, 0, 0, 0, 0, 0, 0, 0};  //{no->0, opt1->1, opt2->2, reserve->3}
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par100[NUM_TARGET_USAGE_MODES]              = {0, 0, 0, 0, 0, 0, 3, 3};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par94[NUM_TARGET_USAGE_MODES]              = {false, false, false, true, true, true, true, true};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par95[NUM_TARGET_USAGE_MODES]          = {false, false, false, false, false, false, true, true};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par96[NUM_TARGET_USAGE_MODES]          = {2, 2, 2, 0, 0, 0, 1, 3};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par97[NUM_TARGET_USAGE_MODES]            = {false, false, false, false, false, false, false, false};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par98[NUM_TARGET_USAGE_MODES]             = {0, 0, 0, 1, 1, 1, 1, 1};  // { 0->leftLCU, 1->lcu32x32};
const uint16_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par12[NUM_TARGET_USAGE_MODES]                   = {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200};
const uint16_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par13[NUM_TARGET_USAGE_MODES]                   = {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200};
const uint16_t Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par14[NUM_TARGET_USAGE_MODES]                     = {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::temporalMvp[NUM_TARGET_USAGE_MODES]                          = {false, false, false, false, false, false, false, false};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par18[NUM_TARGET_USAGE_MODES]                 = {false, false, false, false, false, false, false, false};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par15[NUM_TARGET_USAGE_MODES]                       = {0, 0, 0, 0, 0, 0, 0, 0};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par23[NUM_TARGET_USAGE_MODES]                            = {false, false, false, false, false, false, false, false};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par133[NUM_TARGET_USAGE_MODES]                 = {false, false, false, false, false, false, false, false};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par102[NUM_TARGET_USAGE_MODES]                 = {true, true, true, true, true, true, false, false};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par101[NUM_TARGET_USAGE_MODES]                 = {false, false, false, false, false, false, false, false};
const bool     Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par109[NUM_TARGET_USAGE_MODES] = {false, false, false, false, false, false, false, false};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par145[NUM_TARGET_USAGE_MODES]          = {1, 1, 1, 1, 1, 1, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par146[NUM_TARGET_USAGE_MODES]          = {1, 1, 1, 1, 1, 1, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par147[NUM_TARGET_USAGE_MODES]            = {1, 1, 1, 1, 1, 1, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par150[NUM_TARGET_USAGE_MODES]      = {7, 7, 7, 3, 3, 3, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par138[NUM_TARGET_USAGE_MODES]            = {7, 7, 7, 4, 4, 4, 4, 2};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par149[NUM_TARGET_USAGE_MODES]        = {7, 7, 7, 3, 3, 3, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par148[NUM_TARGET_USAGE_MODES]        = {7, 7, 7, 3, 3, 3, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par152[NUM_TARGET_USAGE_MODES]             = {7, 7, 7, 2, 2, 2, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par139[NUM_TARGET_USAGE_MODES]          = {7, 7, 7, 4, 4, 4, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par153[NUM_TARGET_USAGE_MODES]               = {7, 7, 7, 4, 4, 4, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par154[NUM_TARGET_USAGE_MODES]               = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par142[NUM_TARGET_USAGE_MODES]         = {3, 3, 3, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par143[NUM_TARGET_USAGE_MODES]           = {3, 3, 3, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par144[NUM_TARGET_USAGE_MODES]           = {3, 3, 3, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par155[NUM_TARGET_USAGE_MODES]                   = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par156[NUM_TARGET_USAGE_MODES]                    = {2, 2, 2, 2, 2, 2, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par140[NUM_TARGET_USAGE_MODES]                   = {1, 1, 1, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::vdencCmd2Par141[NUM_TARGET_USAGE_MODES]                 = {1, 1, 1, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe3_Lpm_Base::av1EnableIntraEdgeFilter[NUM_TARGET_USAGE_MODES]             = {1, 1, 1, 1, 1, 1, 0, 0};

MOS_STATUS EncodeAv1VdencConstSettingsXe3_Lpm_Base::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings = {
    VDENC_CMD1_LAMBDA()
    {
        bool     isIntra = AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type);
        uint16_t qp      = m_av1PicParams->base_qindex;

        static const std::array<std::array<uint16_t, 256>, 2> par0Array =
            {{
                {1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 8, 8, 9, 9,
            10, 11, 12, 12, 13, 14, 15, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,
            26, 27, 28, 29, 30, 32, 33, 34, 35, 37, 38, 39, 41, 42, 43, 45, 46, 48,
            49, 51, 52, 54, 55, 57, 58, 60, 62, 63, 65, 67, 68, 70, 72, 74, 76, 78,
            79, 81, 83, 85, 87, 89, 91, 93, 95, 97, 99, 102, 104, 106, 108, 110, 113,
            115, 117, 122, 126, 131, 136, 141, 146, 151, 157, 162, 167, 173, 179, 184,
            190, 196, 202, 208, 214, 221, 227, 233, 240, 246, 253, 260, 270, 281, 292,
            303, 314, 325, 337, 348, 360, 373, 385, 398, 410, 423, 437, 450, 464, 482,
            501, 520, 540, 559, 580, 600, 621, 643, 664, 686, 709, 732, 761, 790, 820,
            851, 882, 914, 946, 979, 1013, 1047, 1088, 1131, 1174, 1218, 1263, 1308, 1355,
            1402, 1450, 1507, 1565, 1625, 1685, 1746, 1809, 1873, 1947, 2022, 2100, 2178,
            2258, 2339, 2433, 2528, 2625, 2723, 2824, 2926, 3042, 3160, 3281, 3403, 3528,
            3668, 3811, 3956, 4104, 4255, 4423, 4594, 4768, 4945, 5141, 5341, 5544, 5751,
            5979, 6211, 6447, 6687, 6950, 7218, 7491, 7788, 8090, 8398, 8732, 9072, 9419,
            9793, 10175, 10563, 10982, 11408, 11843, 12309, 12784, 13293, 13811, 14340, 14904,
            15479, 16092, 16717, 17382, 18060, 18779, 19513, 20291, 21084, 21924, 22780, 23686,
            24609, 25583, 26577, 27624, 28692, 29817, 31001, 32207, 33476, 34808, 36167, 37593},
                {1, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 5, 5, 6, 6, 7, 8, 8, 9, 10, 10, 10,
            11, 12, 13, 14, 15, 15, 16, 17, 18, 19, 20, 21, 21, 22, 23, 24, 25, 26, 26, 28,
            29, 30, 32, 33, 33, 34, 36, 37, 39, 40, 40, 42, 43, 45, 47, 47, 48, 50, 52, 53,
            55, 55, 57, 59, 61, 62, 62, 64, 66, 68, 70, 70, 72, 74, 76, 78, 78, 81, 83, 85,
            87, 87, 89, 92, 94, 94, 96, 99, 101, 103, 103, 108, 111, 116, 121, 124, 129, 132,
            138, 140, 146, 149, 155, 158, 164, 167, 173, 176, 183, 186, 193, 196, 199, 206, 210,
            217, 224, 231, 238, 246, 257, 265, 273, 281, 289, 297, 305, 314, 322, 331, 340, 349,
            358, 371, 385, 395, 409, 424, 434, 449, 464, 474, 490, 501, 517, 528, 545, 567, 584,
            602, 620, 638, 656, 674, 693, 712, 732, 758, 778, 805, 825, 846, 874, 895, 917, 946,
            976, 1006, 1036, 1060, 1091, 1123, 1155, 1188, 1221, 1255, 1289, 1324, 1368, 1403, 1439,
            1485, 1522, 1560, 1607, 1656, 1695, 1745, 1795, 1846, 1898, 1950, 2003, 2057, 2112, 2167,
            2235, 2292, 2361, 2419, 2491, 2563, 2636, 2710, 2786, 2862, 2952, 3044, 3124, 3232, 3328,
            3425, 3538, 3653, 3769, 3903, 4023, 4161, 4317, 4476, 4637, 4802, 4986, 5191, 5400, 5613,
            5867, 6126, 6391, 6701, 7018, 7363, 7759, 8164, 8603, 9075, 9607, 10179, 10816, 11499, 12255,
            13063, 13981, 14960, 16033, 17268, 18581, 20079, 21740, 23540, 25565}
            }};

        static const std::array<std::array<uint16_t, 256>, 2> par1Array =
            {{
                {1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4,
            4, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 8, 8,
            8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 11, 11,
            11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13,
            13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16,
            17, 17, 17, 17, 18, 18, 18, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21,
            22, 22, 22, 23, 23, 23, 24, 24, 25, 25, 26, 26, 26, 27, 27, 28, 28,
            29, 29, 30, 30, 30, 31, 32, 32, 33, 33, 34, 35, 35, 36, 36, 37, 38,
            38, 39, 40, 41, 41, 42, 43, 44, 44, 45, 46, 47, 48, 48, 49, 50, 51,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 64, 65, 66, 67, 68, 70,
            71, 72, 74, 75, 77, 78, 80, 81, 83, 84, 86, 87, 89, 91, 92, 94, 96,
            98, 99, 101, 103, 105, 107, 109, 111, 114, 116, 118, 120, 122, 125,
            127, 130, 132, 135, 137, 140, 143, 145, 148, 151, 154, 157, 160, 163,
            166, 169, 173, 176, 179, 183, 186, 190, 194, 198, 201, 205, 209, 213,
            218, 222, 226, 231, 235, 240, 244, 249, 254, 259, 264, 269, 274},
                {1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4,
            5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
            8, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10, 10, 11, 11, 11, 11, 11, 11,
            11, 12, 12, 12, 12, 12, 12, 13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14,
            14, 14, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 17, 17, 17, 18, 18,
            18, 19, 19, 19, 20, 20, 20, 20, 21, 21, 21, 22, 22, 22, 23, 23, 23, 24,
            24, 24, 25, 25, 26, 26, 27, 27, 28, 28, 29, 29, 30, 30, 31, 31, 31, 32,
            33, 33, 34, 35, 35, 36, 36, 37, 38, 38, 39, 40, 40, 41, 42, 43, 43, 44,
            45, 46, 47, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
            62, 63, 64, 66, 67, 68, 69, 71, 72, 73, 75, 76, 78, 79, 81, 82, 84, 85,
            87, 89, 90, 92, 94, 95, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115,
            117, 120, 122, 124, 126, 129, 131, 134, 137, 139, 142, 145, 147, 150, 153,
            156, 159, 162, 165, 168, 172, 175, 178, 182, 185, 189, 193, 196, 200, 204,
            208, 212, 216, 221, 225, 229, 234, 238, 243, 248, 252, 257, 262, 267, 273,
            278, 283}}};

        static const std::array<uint8_t, 8>  par2Array = {0, 2, 3, 5, 6, 8, 9, 11};
        static const std::array<uint8_t, 12> par3Array = {2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68};
        static const std::array<uint8_t, 12> par4Array = {16, 5, 8, 8, 7, 9, 8, 9, 13, 13, 45, 0};
        static const std::array<std::array<uint8_t, 4>, 2> par8and12Array =
            {{
                {0, 0, 0, 1},
                {0, 0, 0, 0}}};
        static const std::array<std::array<uint8_t, 4>, 2> par9and13Array =
            {{
                {0x0A, 0x0A, 0x0A, 0x08},
                {0, 0, 0, 0}}};
        static const std::array<std::array<uint8_t, 4>, 2> par10and14Array =
            {{
                {0x40, 0x40, 0x40, 0x40},
                {0, 0, 0, 0}}};
        static const std::array<std::array<uint8_t, 4>, 2> par11and15Array =
            {{
                {0xFC, 0xFC, 0xFC, 0xFC},
                {0, 0, 0, 0}}};
        static const std::array<std::array<uint8_t, 4>, 2> par96to99Array =
            {{
                {0x40, 0, 1, 1},
                {0, 0, 0, 0}}};
        static const std::array<uint8_t, 4>  par104Array = {18, 18, 22, 22};
        static const std::array<uint8_t, 4>  par105Array = {26, 22, 26, 26};
        static const std::array<uint8_t, 4>  par106Array = {18, 17, 15, 14};

        par.vdencCmd1Par0 = par0Array[isIntra ? 1 : 0][qp];
        par.vdencCmd1Par1 = par1Array[isIntra ? 1 : 0][qp];

        for (auto i = 0; i < 8; i++)
        {
            par.vdencCmd1Par2[i] = par2Array[i];
        }

        for (auto i = 0; i < 12; i++)
        {
            par.vdencCmd1Par3[i] = par3Array[i];
            par.vdencCmd1Par4[i] = par4Array[i];
        }

        for (auto i = 0; i < 4; i++)
        {
            par.vdencCmd1Par8[i]  = par8and12Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par9[i]  = par9and13Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par10[i] = par10and14Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par11[i] = par11and15Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par12[i] = par8and12Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par13[i] = par9and13Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par14[i] = par10and14Array[isIntra ? 1 : 0][i];
            par.vdencCmd1Par15[i] = par11and15Array[isIntra ? 1 : 0][i];
        }

        par.vdencCmd1Par107 = 3;

        par.vdencCmd1Par50  = 0x0C;
        par.vdencCmd1Par49  = 0x0C;
        par.vdencCmd1Par48  = 0x0C;
        par.vdencCmd1Par47  = 0x0C;
        par.vdencCmd1Par54  = 0x0C;
        par.vdencCmd1Par53  = 0x0C;
        par.vdencCmd1Par52  = 0x0C;
        par.vdencCmd1Par51  = 0x0C;

        par.vdencCmd1Par58  = 0x10;
        par.vdencCmd1Par57  = 0x10;
        par.vdencCmd1Par56  = 0x10;
        par.vdencCmd1Par55  = 0x10;
        par.vdencCmd1Par74  = 0x10;
        par.vdencCmd1Par73  = 0x10;
        par.vdencCmd1Par72  = 0x10;
        par.vdencCmd1Par71  = 0x10;

        par.vdencCmd1Par62  = 0x10;
        par.vdencCmd1Par61  = 0x10;
        par.vdencCmd1Par60  = 0x10;
        par.vdencCmd1Par59  = 0x10;
        par.vdencCmd1Par78  = 0x10;
        par.vdencCmd1Par77  = 0x10;
        par.vdencCmd1Par76  = 0x10;
        par.vdencCmd1Par75  = 0x10;

        par.vdencCmd1Par66  = 0x10;
        par.vdencCmd1Par65  = 0x10;
        par.vdencCmd1Par64  = 0x10;
        par.vdencCmd1Par63  = 0x10;
        par.vdencCmd1Par82  = 0x10;
        par.vdencCmd1Par81  = 0x10;
        par.vdencCmd1Par80  = 0x10;
        par.vdencCmd1Par79  = 0x10;

        par.vdencCmd1Par70  = 0x10;
        par.vdencCmd1Par69  = 0x10;
        par.vdencCmd1Par68  = 0x10;
        par.vdencCmd1Par67  = 0x10;
        par.vdencCmd1Par86  = 0x10;
        par.vdencCmd1Par85  = 0x10;
        par.vdencCmd1Par84  = 0x10;
        par.vdencCmd1Par83  = 0x10;

        for (auto i = 0; i < 4; i++)
        {
            par.vdencCmd1Par104[i]      = par104Array[i];
            par.vdencCmd1Par105[i]      = par105Array[i];
            par.vdencCmd1Par106[i]      = par106Array[i];
        }

        par.vdencCmd1Par6       = 0x04;
        par.vdencCmd1Par5       = 0x08;
        par.vdencCmd1Par7       = 0x0C;
        par.vdencCmd1Par24      = 0;
        par.vdencCmd1Par25      = 0;
        par.vdencCmd1Par28      = 0;
        par.vdencCmd1Par29      = 0;
        par.vdencCmd1Par26      = 0;
        par.vdencCmd1Par27      = 0;
        par.vdencCmd1Par30      = 0;
        par.vdencCmd1Par31      = 0;
        par.vdencCmd1Par32      = 0;
        par.vdencCmd1Par33      = 0;
        par.vdencCmd1Par34      = 0x15;
        par.vdencCmd1Par36      = 0x15;

        for (auto i = 0; i < 4; i++)
        {
            par.vdnecCmd1Par96[i] = par96to99Array[isIntra ? 1 : 0][i];
            par.vdnecCmd1Par97[i] = par96to99Array[isIntra ? 1 : 0][i];
            par.vdnecCmd1Par98[i] = par96to99Array[isIntra ? 1 : 0][i];
            par.vdnecCmd1Par99[i] = par96to99Array[isIntra ? 1 : 0][i];
        }

        for (auto i = 0; i < 2; i++)
        {
            par.vdnecCmd1Par100[i]  = 0;
            par.vdnecCmd1Par101[i]  = 0;
            par.vdnecCmd1Par102[i]  = 0;
            par.vdnecCmd1Par103[i]  = 0;
        }

        if (isIntra)
        {
            par.vdencCmd1Par37      = 0x2F;
            par.vdencCmd1Par38      = 0x10;
            par.vdencCmd1Par39      = 0x10;
            par.vdencCmd1Par23      = 0x2A;
            par.vdencCmd1Par42      = 0x3A;
            par.vdencCmd1Par43      = 0x14;
            par.vdencCmd1Par40      = 0x1E;
            par.vdencCmd1Par41      = 0x1E;
            par.vdencCmd1Par44      = 0;
            par.vdencCmd1Par45      = 0x14;
            par.vdencCmd1Par46      = 0;

            par.vdencCmd1Par94      = 0x18;
            par.vdencCmd1Par93      = 0x18;
        }
        else
        {
            if (isLowDelay)
            {
                par.vdencCmd1Par20      = 0x19;
                par.vdencCmd1Par16      = 0xB2;
                par.vdencCmd1Par17      = 0x2C;
                par.vdencCmd1Par18      = 0xCF;
                par.vdencCmd1Par19      = 0x2E;
                par.vdencCmd1Par21      = 0x1A;
                par.vdencCmd1Par22      = 0x07;

                par.vdencCmd1Par44      = 0;
                par.vdencCmd1Par45      = 0x14;
                par.vdencCmd1Par46      = 0;

                par.vdencCmd1Par90      = 0;
                par.vdencCmd1Par91      = 0;
                par.vdencCmd1Par92      = 0;
            }
            else
            {
                par.vdencCmd1Par20      = 0x2C;
                par.vdencCmd1Par16      = 0xCE;
                par.vdencCmd1Par17      = 0x25;
                par.vdencCmd1Par18      = 0xB8;
                par.vdencCmd1Par19      = 0x3A;
                par.vdencCmd1Par21      = 0x34;
                par.vdencCmd1Par22      = 0x0E;

                par.vdencCmd1Par44      = 0x4;
                par.vdencCmd1Par45      = 0x14;
                par.vdencCmd1Par46      = 0x14;
            }

            par.vdencCmd1Par37      = 0x1D;
            par.vdencCmd1Par38      = 0x1D;
            par.vdencCmd1Par39      = 0x1D;
            par.vdencCmd1Par23      = 0x36;
            par.vdencCmd1Par40      = 0x2D;

            par.vdencCmd1Par41      = 0x44;

            par.vdencCmd1Par87      = 0x14;
            par.vdencCmd1Par88      = 0x14;
            par.vdencCmd1Par89      = 0x14;

            par.vdencCmd1Par42      = 0x25;
            par.vdencCmd1Par43      = 0x25;

            par.vdencCmd1Par95      = 0x38;

            par.vdencCmd1Par93      = 0x18;
            par.vdencCmd1Par94      = 0x18;
        }
        return MOS_STATUS_SUCCESS;
    }
};
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettingsXe3_Lpm_Base::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);
#if _MEDIA_RESERVED
#define VDENC_CMD2_SETTINGS_EXT
#include "encode_av1_vdenc_const_settings_xe3_lpm_base_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#else
#define VDENC_CMD2_SETTINGS_OPEN
#include "encode_av1_vdenc_const_settings_xe3_lpm_base_open.h"
#undef VDENC_CMD2_SETTINGS_OPEN
#endif  // _MEDIA_RESERVED
    return MOS_STATUS_SUCCESS;    


}

MOS_STATUS EncodeAv1VdencConstSettingsXe3_Lpm_Base::SetVdencStreaminStateSettings()
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<Av1VdencFeatureSettings *>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencStreaminStateSettings.emplace_back(
        VDENC_STREAMIN_STATE_LAMBDA() {
            static const std::array<
                std::array<
                    uint8_t,
                    NUM_TARGET_USAGE_MODES + 1>,
                4>
                numMergeCandidates = {{
                    {4, 4, 4, 2, 2, 2, 0, 0},
                    {4, 4, 4, 2, 2, 2, 2, 1},
                    {5, 5, 5, 2, 2, 2, 2, 2},
                    {5, 5, 5, 2, 2, 2, 2, 2},
                }};

            static const std::array<
                uint8_t,
                NUM_TARGET_USAGE_MODES + 1>
                numImePredictors =  {8, 8, 8, 6, 6, 6, 4, 3};

            par.maxTuSize                = 3;  //Maximum TU Size allowed, restriction to be set to 3
            par.maxCuSize                = (cu64Align) ? 3 : 2;
            par.numMergeCandidateCu64x64 = numMergeCandidates[3][m_av1SeqParams->TargetUsage];
            par.numMergeCandidateCu32x32 = numMergeCandidates[2][m_av1SeqParams->TargetUsage];
            par.numMergeCandidateCu16x16 = numMergeCandidates[1][m_av1SeqParams->TargetUsage];
            par.numMergeCandidateCu8x8   = numMergeCandidates[0][m_av1SeqParams->TargetUsage];
            par.numImePredictors         = numImePredictors[m_av1SeqParams->TargetUsage];

            return MOS_STATUS_SUCCESS;
        });

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
