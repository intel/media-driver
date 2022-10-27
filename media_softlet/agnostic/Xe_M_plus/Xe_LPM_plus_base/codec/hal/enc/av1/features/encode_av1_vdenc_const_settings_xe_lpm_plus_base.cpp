/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encode_av1_vdenc_const_settings_xe_lpm_base.cpp
//! \brief    Defines the common interface for av1 vdenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_av1_vdenc_const_settings_xe_lpm_plus_base.h"
#include "encode_utils.h"

namespace encode
{
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par4[NUM_TARGET_USAGE_MODES]               = {16, 16, 16, 16, 16, 16, 16, 16};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par38[NUM_TARGET_USAGE_MODES]              = {4, 4, 4, 3, 3, 3, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par39[NUM_TARGET_USAGE_MODES]              = {8, 8, 8, 6, 6, 6, 4, 4};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par67[NUM_TARGET_USAGE_MODES]              = {8, 8, 8, 8, 8, 8, 8, 8};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par83Table2[NUM_TARGET_USAGE_MODES]        = {true, true, true, true, true, true, true, true};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par83Table1[NUM_TARGET_USAGE_MODES]        = {false, false, false, false, false, false, true, true};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par83Table0[NUM_TARGET_USAGE_MODES]        = {false, false, false, false, false, false, false, false};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par84Table0[NUM_TARGET_USAGE_MODES]        = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par84Table1[NUM_TARGET_USAGE_MODES]        = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par84Table2[NUM_TARGET_USAGE_MODES]        = {1, 1, 1, 1, 1, 1, 0, 0};
const uint32_t Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par85Table0[NUM_TARGET_USAGE_MODES]        = {1, 1, 1, 3, 3, 3, 3, 3};
const uint32_t Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par85Table1[NUM_TARGET_USAGE_MODES]        = {1, 1, 1, 1, 1, 1, 0, 0};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par86[NUM_TARGET_USAGE_MODES]              = {false, false, false, false, false, false, true, true};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par87Table3[NUM_TARGET_USAGE_MODES]        = {3, 3, 3, 2, 2, 2, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par87Table2[NUM_TARGET_USAGE_MODES]        = {3, 3, 3, 2, 2, 2, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par87Table1[NUM_TARGET_USAGE_MODES]        = {3, 3, 3, 2, 2, 2, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par87Table0[NUM_TARGET_USAGE_MODES]        = {3, 3, 3, 2, 2, 2, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table13[NUM_TARGET_USAGE_MODES]       = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table12[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 3, 3, 3, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table11[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 3, 3, 3, 3, 3};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table10[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table23[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table22[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 1, 1, 1, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table21[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 3, 3, 3, 3, 3};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table20[NUM_TARGET_USAGE_MODES]       = {1, 1, 1, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table03[NUM_TARGET_USAGE_MODES]       = {0, 0, 0, 0, 0, 0, 0, 0};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table02[NUM_TARGET_USAGE_MODES]       = {7, 7, 7, 3, 3, 3, 2, 2};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table01[NUM_TARGET_USAGE_MODES]       = {3, 3, 3, 1, 1, 1, 1, 1};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par88Table00[NUM_TARGET_USAGE_MODES]       = {7, 7, 7, 7, 7, 7, 1, 1};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par89[NUM_TARGET_USAGE_MODES]              = {false, false, false, false, false, false, true, true};
const uint16_t Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par92[NUM_TARGET_USAGE_MODES]              = {0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000};
const uint16_t Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par93[NUM_TARGET_USAGE_MODES]              = {0xffff, 0xffff, 0xffff, 0xff00, 0xff00, 0xff00, 0xffff, 0xffff};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par100[NUM_TARGET_USAGE_MODES]             = {0, 0, 0, 0, 0, 0, 3, 3};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par94[NUM_TARGET_USAGE_MODES]              = {false, false, false, true, true, true, true, true};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par95[NUM_TARGET_USAGE_MODES]              = {false, false, false, false, false, false, true, true};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par96[NUM_TARGET_USAGE_MODES]              = {0, 0, 0, 0, 0, 0, 3, 3};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par97[NUM_TARGET_USAGE_MODES]              = {false, false, false, true, true, true, false, false};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par98[NUM_TARGET_USAGE_MODES]              = {0, 0, 0, 0, 0, 0, 1, 1};
const uint16_t Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par14[NUM_TARGET_USAGE_MODES]              = {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::temporalMvp[NUM_TARGET_USAGE_MODES]                 = {false, false, false, false, false, false, false, false};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par18[NUM_TARGET_USAGE_MODES]              = {false, false, false, false, false, false, false, false};
const uint8_t  Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par15[NUM_TARGET_USAGE_MODES]              = {0, 0, 0, 0, 0, 0, 0, 0};
const uint16_t Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par12[NUM_TARGET_USAGE_MODES]              = {0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200, 0x200};
const bool     Av1VdencTUConstSettingsXe_Lpm_Base::vdencCmd2Par23[NUM_TARGET_USAGE_MODES]              = {false, false, false, false, false, false, false, false};

MOS_STATUS EncodeAv1VdencConstSettingsXe_Lpm_Plus_Base::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    setting->vdencCmd1Settings = {
        VDENC_CMD1_LAMBDA()
    {
        bool isIntra = AV1_KEY_OR_INRA_FRAME(m_av1PicParams->PicFlags.fields.frame_type);
        uint16_t qp  = m_av1PicParams->base_qindex;

        static const std::array<std::array<uint16_t, 256>, 2> par0Array =
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

        static const std::array<std::array<uint16_t, 256>, 2> par1Array =
            {{{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5,
            5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 9, 10, 10, 11, 11, 12, 12, 13, 14, 14, 15, 
            15, 16, 16, 17, 18, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 
            28, 28, 29, 30, 31, 32, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 
            44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 59, 61, 63, 66, 
            68, 71, 73, 76, 78, 81, 84, 86, 89, 92, 95, 98, 101, 104, 107, 110, 113, 
            117, 120, 123, 127, 130, 135, 140, 146, 151, 157, 163, 168, 174, 180, 186, 
            193, 199, 205, 212, 218, 225, 232, 241, 250, 260, 270, 280, 290, 300, 311, 
            321, 332, 343, 354, 366, 380, 395, 410, 425, 441, 457, 473, 490, 506, 523, 
            544, 565, 587, 609, 631, 654, 677, 701, 725, 754, 783, 812, 842, 873, 905, 
            936, 973, 1011, 1050, 1089, 1129, 1170, 1216, 1264, 1312, 1362, 1412, 1463, 
            1521, 1580, 1640, 1702, 1764, 1834, 1905, 1978, 2052, 2128, 2211, 2297, 2384, 
            2473, 2570, 2670, 2772, 2876, 2989, 3105, 3223, 3344, 3475, 3609, 3745, 3894, 
            4045, 4199, 4366, 4536, 4709, 4897, 5087, 5282, 5491, 5704, 5921, 6154, 6392, 
            6646, 6906, 7170, 7452, 7740, 8046, 8359, 8691, 9030, 9390, 9757, 10146, 10542, 
            10962, 11390, 11843, 12304, 12792, 13288, 13812, 14346, 14908, 15500, 16104, 
            16738, 17404, 18084, 18796},
                {0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 
            5, 6, 6, 7, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15,
            16, 16, 17, 18, 18, 19, 20, 20, 21, 22, 22, 23, 24, 25, 25, 26, 27, 28, 29,
            29, 30, 31, 32, 33, 34, 35, 36, 37, 37, 38, 39, 40, 41, 42, 43, 44, 45, 47, 
            48, 49, 50, 51, 52, 53, 54, 55, 57, 58, 59, 60, 61, 62, 65, 67, 70, 73, 75, 
            78, 81, 84, 86, 89, 92, 95, 98, 102, 105, 108, 111, 114, 118, 121, 125, 128, 
            132, 135, 139, 144, 150, 156, 162, 168, 174, 180, 186, 192, 199, 206, 212, 
            219, 226, 233, 240, 248, 257, 267, 278, 288, 299, 309, 320, 332, 343, 355, 
            366, 378, 391, 406, 422, 438, 454, 471, 488, 505, 523, 541, 559, 581, 604, 
            627, 650, 674, 698, 723, 748, 774, 805, 836, 867, 900, 932, 966, 1000, 1039, 
            1080, 1121, 1163, 1205, 1249, 1299, 1349, 1401, 1454, 1508, 1562, 1624, 1687, 
            1751, 1817, 1884, 1958, 2034, 2112, 2191, 2272, 2361, 2452, 2545, 2640, 2745, 
            2851, 2960, 3071, 3192, 3316, 3442, 3570, 3711, 3854, 3999, 4158, 4319, 4484, 
            4662, 4843, 5029, 5228, 5432, 5640, 5863, 6091, 6323, 6572, 6825, 7097, 7374, 
            7656, 7957, 8264, 8591, 8925, 9280, 9642, 10026, 10418, 10833, 11257, 11705, 
            12162, 12646, 13138, 13659, 14189, 14748, 15318, 15919, 16551, 17195, 17872, 
            18584, 19309, 20070}}};

        static const std::array<std::array<uint8_t, 8>, 2> par2Array =
            {{
                {0, 2, 3, 5, 6, 8, 9, 11},
                {0, 2, 3, 5, 6, 8, 9, 11},
            }};

        static const std::array<uint8_t, 12> par3Array = {4, 14, 24, 34, 44, 54, 64, 74, 84, 94, 104, 114};
        static const std::array<uint8_t, 12> par4Array = {3, 9, 14, 19, 24, 29, 34, 39, 44, 49, 54, 60};

        par.vdencCmd1Par1 = par0Array[isIntra ? 1 : 0][qp];
        par.vdencCmd1Par0 = par1Array[isIntra ? 1 : 0][qp];

        for (auto i = 0; i < 8; i++)
        {
            par.vdencCmd1Par2[i] = par2Array[isIntra ? 1 : 0][i];
        }

        for (auto i = 0; i < 12; i++)
        {
            par.vdencCmd1Par3[i] = par3Array[i];
            par.vdencCmd1Par4[i] = par4Array[i];
        }

        par.vdencCmd1Par50 = 0x05;
        par.vdencCmd1Par49 = 0x05;
        par.vdencCmd1Par48 = 0x05;
        par.vdencCmd1Par47 = 0x05;
        par.vdencCmd1Par54 = 0x0c;
        par.vdencCmd1Par53 = 0x0c;
        par.vdencCmd1Par52 = 0x0c;
        par.vdencCmd1Par51 = 0x0c;

        par.vdencCmd1Par58 = 0x12;
        par.vdencCmd1Par57 = 0x12;
        par.vdencCmd1Par56 = 0x12;
        par.vdencCmd1Par55 = 0x12;
        par.vdencCmd1Par74 = 0x10;
        par.vdencCmd1Par73 = 0x10;
        par.vdencCmd1Par72 = 0x10;
        par.vdencCmd1Par71 = 0x10;

        par.vdencCmd1Par62 = 0x10;
        par.vdencCmd1Par61 = 0x10;
        par.vdencCmd1Par60 = 0x10;
        par.vdencCmd1Par59 = 0x10;
        par.vdencCmd1Par78 = 0x10;
        par.vdencCmd1Par77 = 0x10;
        par.vdencCmd1Par76 = 0x10;
        par.vdencCmd1Par75 = 0x10;

        par.vdencCmd1Par66 = 0x10;
        par.vdencCmd1Par65 = 0x10;
        par.vdencCmd1Par64 = 0x10;
        par.vdencCmd1Par63 = 0x10;
        par.vdencCmd1Par82 = 0x10;
        par.vdencCmd1Par81 = 0x10;
        par.vdencCmd1Par80 = 0x10;
        par.vdencCmd1Par79 = 0x10;

        par.vdencCmd1Par70 = 0x16;
        par.vdencCmd1Par69 = 0x16;
        par.vdencCmd1Par68 = 0x16;
        par.vdencCmd1Par67 = 0x16;
        par.vdencCmd1Par86 = 0x1a;
        par.vdencCmd1Par85 = 0x1a;
        par.vdencCmd1Par84 = 0x1a;
        par.vdencCmd1Par83 = 0x1a;

        if (isIntra)
        {
            par.vdencCmd1Par23 = 0x2a;
            par.vdencCmd1Par24 = 0x00;
            par.vdencCmd1Par25 = 0x00;
            par.vdencCmd1Par26 = 0x00;
            par.vdencCmd1Par27 = 0x00;
            par.vdencCmd1Par28 = 0x00;
            par.vdencCmd1Par29 = 0x00;
            par.vdencCmd1Par30 = 0x00;
            par.vdencCmd1Par31 = 0x00;
            par.vdencCmd1Par32 = 0x00;
            par.vdencCmd1Par33 = 0x00;
            par.vdencCmd1Par34 = 0x15;
            par.vdencCmd1Par36 = 0x15;
            par.vdencCmd1Par37 = 0x2f;
            par.vdencCmd1Par38 = 0x10;
            par.vdencCmd1Par39 = 0x10;
            par.vdencCmd1Par42 = 0x3a;
            par.vdencCmd1Par43 = 0x14;
            par.vdencCmd1Par40 = 0x1e;
            par.vdencCmd1Par41 = 0x1e;
            par.vdencCmd1Par44 = 0x00;
            par.vdencCmd1Par45 = 0x14;
            par.vdencCmd1Par46 = 0x00;
        }
        else if (isLowDelay)
        {
            par.vdencCmd1Par6     = 0x03;
            par.vdencCmd1Par5     = 0x06;
            par.vdencCmd1Par7     = 0x0a;
            par.vdencCmd1Par8[0]  = 0x05;
            par.vdencCmd1Par9[0]  = 0x06;
            par.vdencCmd1Par12[0] = 0x17;
            par.vdencCmd1Par13[0] = 0x1a;
            par.vdencCmd1Par10[0] = 0x05;
            par.vdencCmd1Par11[0] = 0x00;
            par.vdencCmd1Par14[0] = 0x15;
            par.vdencCmd1Par15[0] = 0x00;
            par.vdencCmd1Par16    = 0x5c;
            par.vdencCmd1Par17    = 0x13;
            par.vdencCmd1Par18    = 0x5c;
            par.vdencCmd1Par19    = 0x12;
            par.vdencCmd1Par23    = 0x36;
            par.vdencCmd1Par20    = 0x0f;
            par.vdencCmd1Par21    = 0x04;
            par.vdencCmd1Par22    = 0x04;
            par.vdencCmd1Par24    = 0x00;
            par.vdencCmd1Par25    = 0x00;
            par.vdencCmd1Par26    = 0x00;
            par.vdencCmd1Par27    = 0x00;
            par.vdencCmd1Par28    = 0x00;
            par.vdencCmd1Par29    = 0x00;
            par.vdencCmd1Par30    = 0x00;
            par.vdencCmd1Par31    = 0x00;
            par.vdencCmd1Par32    = 0x00;
            par.vdencCmd1Par33    = 0x00;
            par.vdencCmd1Par34    = 0x15;
            par.vdencCmd1Par36    = 0x15;
            par.vdencCmd1Par37    = 0x17;
            par.vdencCmd1Par38    = 0x18;
            par.vdencCmd1Par39    = 0x1b;
            par.vdencCmd1Par40    = 0x29;
            par.vdencCmd1Par41    = 0x44;
            par.vdencCmd1Par42    = 0x25;
            par.vdencCmd1Par43    = 0x25;
            par.vdencCmd1Par44    = 0x00;
            par.vdencCmd1Par45    = 0x0c;
            par.vdencCmd1Par46    = 0x00;
            par.vdencCmd1Par87    = 0x14;
            par.vdencCmd1Par88    = 0x14;
            par.vdencCmd1Par89    = 0x14;
        }
        else
        {
            par.vdencCmd1Par6     = 0x03;
            par.vdencCmd1Par5     = 0x06;
            par.vdencCmd1Par7     = 0x0a;
            par.vdencCmd1Par8[0]  = 0x05;
            par.vdencCmd1Par9[0]  = 0x06;
            par.vdencCmd1Par12[0] = 0x17;
            par.vdencCmd1Par13[0] = 0x1a;
            par.vdencCmd1Par10[0] = 0x05;
            par.vdencCmd1Par11[0] = 0x00;
            par.vdencCmd1Par14[0] = 0x15;
            par.vdencCmd1Par15[0] = 0x00;
            par.vdencCmd1Par16    = 0x5c;
            par.vdencCmd1Par17    = 0x13;
            par.vdencCmd1Par18    = 0x5c;
            par.vdencCmd1Par19    = 0x12;
            par.vdencCmd1Par21    = 0x04;
            par.vdencCmd1Par22    = 0x04;
            par.vdencCmd1Par23    = 0x36;
            par.vdencCmd1Par20    = 0x0f;
            par.vdencCmd1Par24    = 0x00;
            par.vdencCmd1Par25    = 0x00;
            par.vdencCmd1Par26    = 0x00;
            par.vdencCmd1Par27    = 0x00;
            par.vdencCmd1Par28    = 0x00;
            par.vdencCmd1Par29    = 0x00;
            par.vdencCmd1Par30    = 0x00;
            par.vdencCmd1Par31    = 0x00;
            par.vdencCmd1Par32    = 0x00;
            par.vdencCmd1Par33    = 0x00;
            par.vdencCmd1Par34    = 0x15;
            par.vdencCmd1Par36    = 0x15;
            par.vdencCmd1Par37    = 0x17;
            par.vdencCmd1Par38    = 0x18;
            par.vdencCmd1Par39    = 0x1b;
            par.vdencCmd1Par40    = 0x29;
            par.vdencCmd1Par41    = 0x44;
            par.vdencCmd1Par42    = 0x25;
            par.vdencCmd1Par43    = 0x25;
            par.vdencCmd1Par44    = 0x03;
            par.vdencCmd1Par45    = 0x0c;
            par.vdencCmd1Par46    = 0x0c;
            par.vdencCmd1Par87    = 0x14;
            par.vdencCmd1Par88    = 0x14;
            par.vdencCmd1Par89    = 0x14;
        }

        return MOS_STATUS_SUCCESS;
    }
    };
        return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodeAv1VdencConstSettingsXe_Lpm_Plus_Base::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    auto setting = static_cast<Av1VdencFeatureSettings*>(m_featureSetting);
    ENCODE_CHK_NULL_RETURN(setting);

    

#if _MEDIA_RESERVED
#define VDENC_CMD2_SETTINGS_EXT
#include "encode_av1_vdenc_const_settings_xe_lpm_plus_base_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#else
        setting->vdencCmd2Settings.emplace_back(
        VDENC_CMD2_LAMBDA() {

            par.extSettings.emplace_back(
                [this, isLowDelay, &par](uint32_t *data) {
                    uint32_t TargetUsage        = m_av1SeqParams->TargetUsage;
                    uint32_t lowDelay           = isLowDelay;
                    uint32_t frameType          = m_av1PicParams->PicFlags.fields.frame_type;
                    uint32_t l1RefFrameCtrlNot0 = (m_av1PicParams->ref_frame_ctrl_l1.RefFrameCtrl.value != 0);
                    uint32_t l0RefNum           = par.numRefL0;
                    uint32_t l1RefNum           = par.numRefL1;

                    static const uint32_t dw2Lut = 0x3;
                    data[2] |= dw2Lut;

                    static const uint32_t dw5Lut = 0xc0ac80;
                    data[5] |= dw5Lut;

                    static const uint32_t dw6Lut = 0xe0080200;
                    data[6] |= dw6Lut;

                    static const uint32_t dw7Lut[2][4] = { { 0x64003, 0x64003, 0x64003, 0x64003,}, { 0xe4003, 0xe4003, 0x64003, 0x64003,},};
                    data[7] |= dw7Lut[lowDelay][l0RefNum];

                    static const uint32_t dw8Lut[8][2][4][2] = { { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdfcaa,}, { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdfcaa,},}, { { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,}, { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdfcaa,}, { 0x54555555, 0x54555555,}, { 0xfffdccaa, 0xfffdfcaa,},}, { { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,}, { 0, 0,}, { 0xfffdccaa, 0xfffdccaa,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,}, { 0x54555555, 0x54555555,},}, { { 0, 0,}, { 0, 0,}, { 0, 0,}, { 0, 0,},},}, { { { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,}, { 0x54555555, 0x54555555,}, { 0x55550000, 0x55550000,},}, { { 0, 0,}, { 0x55550000, 0x55550000,}, { 0, 0,}, { 0x55550000, 0x55550000,},},},};
                    data[8] |= dw8Lut[TargetUsage][lowDelay][frameType][l1RefFrameCtrlNot0];

                    static const uint32_t dw9Lut[8][2][4][2] = { { { { 0x845555, 0x845555,}, { 0x845555, 0x845555,}, { 0x845555, 0x845555,}, { 0x845555, 0x845555,},}, { { 0x840000, 0x840000,}, { 0x840000, 0x840000,}, { 0x840000, 0x840000,}, { 0x840000, 0x840000,},},}, { { { 0x845555, 0x845555,}, { 0x845555, 0x845555,}, { 0x845555, 0x845555,}, { 0x845555, 0x845555,},}, { { 0x840000, 0x840000,}, { 0x840000, 0x840000,}, { 0x840000, 0x840000,}, { 0x840000, 0x840000,},},}, { { { 0x845555, 0x845555,}, { 0x84ffff, 0x84fcff,}, { 0x845555, 0x845555,}, { 0x84ffff, 0x84fcff,},}, { { 0x840000, 0x840000,}, { 0x84ffff, 0x84ffff,}, { 0x840000, 0x840000,}, { 0x84ffff, 0x84ffff,},},}, { { { 0x635555, 0x635555,}, { 0x635555, 0x635555,}, { 0x635555, 0x635555,}, { 0x635555, 0x635555,},}, { { 0x630000, 0x630000,}, { 0x630000, 0x630000,}, { 0x630000, 0x630000,}, { 0x630000, 0x630000,},},}, { { { 0x635555, 0x635555,}, { 0x63ffff, 0x63fcff,}, { 0x635555, 0x635555,}, { 0x63ffff, 0x63fcff,},}, { { 0x630000, 0x630000,}, { 0x63ffff, 0x63ffff,}, { 0x630000, 0x630000,}, { 0x63ffff, 0x63ffff,},},}, { { { 0x635555, 0x635555,}, { 0x635555, 0x635555,}, { 0x635555, 0x635555,}, { 0x635555, 0x635555,},}, { { 0x630000, 0x630000,}, { 0x630000, 0x630000,}, { 0x630000, 0x630000,}, { 0x630000, 0x630000,},},}, { { { 0x425555, 0x425555,}, { 0x425555, 0x425555,}, { 0x425555, 0x425555,}, { 0x425555, 0x425555,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,}, { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},}, { { { 0x425555, 0x425555,}, { 0x420000, 0x420000,}, { 0x425555, 0x425555,}, { 0x420000, 0x420000,},}, { { 0x420000, 0x420000,}, { 0x420000, 0x420000,}, { 0x420000, 0x420000,}, { 0x420000, 0x420000,},},},};
                    data[9] |= dw9Lut[TargetUsage][lowDelay][frameType][l1RefFrameCtrlNot0];

                    static const uint32_t dw11Lut[2][4][4] = { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x80000000, 0, 0,}, { 0x80000000, 0, 0, 0,},}, { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x80000000, 0, 0,}, { 0x80000000, 0, 0, 0x80000000,},},};
                    data[11] |= dw11Lut[lowDelay][l0RefNum][l1RefNum];

                    static const uint32_t dw12Lut = 0xffffffff;
                    data[12] |= dw12Lut;

                    static const uint32_t dw14Lut = 0x1f40000;
                    data[14] |= dw14Lut;

                    static const uint32_t dw15Lut = 0x13881388;
                    data[15] |= dw15Lut;

                    static const uint32_t dw16Lut = 0xf000000;
                    data[16] |= dw16Lut;

                    static const uint32_t dw17Lut = 0x3e8;
                    data[17] |= dw17Lut;

                    static const uint32_t dw18Lut = 0x80000;
                    data[18] |= dw18Lut;

                    static const uint32_t dw19Lut = 0x98000040;
                    data[19] |= dw19Lut;

                    static const uint32_t dw20Lut = 0xffff;
                    data[20] |= dw20Lut;

                    static const uint32_t dw27Lut = 0xffff0000;
                    data[27] |= dw27Lut;

                    static const uint32_t dw28Lut = 0x7d00fa0;
                    data[28] |= dw28Lut;

                    static const uint32_t dw29Lut = 0x2bc0bb8;
                    data[29] |= dw29Lut;

                    static const uint32_t dw30Lut = 0x32003e8;
                    data[30] |= dw30Lut;

                    static const uint32_t dw31Lut = 0x1f4012c;
                    data[31] |= dw31Lut;

                    static const uint32_t dw32Lut = 0x190;
                    data[32] |= dw32Lut;

                    static const uint32_t dw35Lut = 0x800;
                    data[35] |= dw35Lut;

                    static const uint32_t dw37Lut = 0x40;
                    data[37] |= dw37Lut;

                    static const uint32_t dw39Lut = 0x8000fc;
                    data[39] |= dw39Lut;

                    static const uint32_t dw40Lut = 0xb10080;
                    data[40] |= dw40Lut;

                    static const uint32_t dw41Lut = 0x300aa;
                    data[41] |= dw41Lut;

                    static const uint32_t dw42Lut = 0xd30069;
                    data[42] |= dw42Lut;

                    static const uint32_t dw43Lut = 0xe000e9;
                    data[43] |= dw43Lut;

                    static const uint32_t dw44Lut = 0x940003;
                    data[44] |= dw44Lut;

                    static const uint32_t dw45Lut = 0x56004d;
                    data[45] |= dw45Lut;

                    static const uint32_t dw46Lut = 0x9500fd;
                    data[46] |= dw46Lut;

                    static const uint32_t dw47Lut = 0x17002d;
                    data[47] |= dw47Lut;

                    static const uint32_t dw48Lut = 0xfd001f;
                    data[48] |= dw48Lut;

                    static const uint32_t dw49Lut = 0x2006c;
                    data[49] |= dw49Lut;

                    static const uint32_t dw50Lut = 0x800080;
                    data[50] |= dw50Lut;

                    static const uint32_t dw51Lut[8] = { 0x33331552, 0x33331552, 0x33331552, 0x22223552, 0x22223552, 0x22223552, 0x12227146, 0x12227146,};
                    data[51] |= dw51Lut[TargetUsage];

                    static const uint32_t dw52Lut[8] = { 0x77f5bdb, 0x77f5bdb, 0x77f5bdb, 0x72d595b, 0x72d595b, 0x72d595b, 0x9295a5a, 0x9295a5a,};
                    data[52] |= dw52Lut[TargetUsage];

                    static const uint32_t dw53Lut[8] = { 0xffffffff, 0xffffffff, 0xffffffff, 0xff000000, 0xff000000, 0xff000000, 0xffff0000, 0xffff0000,};
                    data[53] |= dw53Lut[TargetUsage];

                    static const uint32_t dw54Lut[8] = { 0, 0, 0, 0x44000000, 0x44000000, 0x44000000, 0xbc00000c, 0xbc00000c,};
                    data[54] |= dw54Lut[TargetUsage];

                    static const uint32_t dw56Lut[2][4][4] = { { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0x300, 0, 0, 0,},}, { { 0, 0, 0, 0,}, { 0, 0, 0, 0,}, { 0, 0x300, 0, 0,}, { 0x300, 0, 0, 0x300,},},};
                    data[56] |= dw56Lut[lowDelay][l0RefNum][l1RefNum];



                    return MOS_STATUS_SUCCESS;
                });

            return MOS_STATUS_SUCCESS;
        });
#endif  // _MEDIA_RESERVED

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
