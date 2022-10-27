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
//! \file     encode_preenc_const_settings.cpp
//! \brief    Defines the common interface for preenc const settings
//! \details  The encode feature manager is further sub-divided by platform type
//!           this file is for the base interface which is shared by all components.
//!

#include "encode_preenc_const_settings.h"
#include "mos_utilities.h"
#include "encode_utils.h"
#include "codec_def_encode.h"

namespace encode
{

#define CLIP3(MIN_, MAX_, X) (((X) < (MIN_)) ? (MIN_) : (((X) > (MAX_)) ? (MAX_) : (X)))

EncodePreEncConstSettings::EncodePreEncConstSettings()
{
    m_featureSetting = MOS_New(PreEncFeatureSettings);
}

EncodePreEncConstSettings::~EncodePreEncConstSettings()
{
    MOS_Delete(m_featureSetting);
    m_featureSetting = nullptr;
}

MOS_STATUS EncodePreEncConstSettings::PrepareConstSettings()
{
    ENCODE_FUNC_CALL();

    ENCODE_CHK_STATUS_RETURN(SetCommonSettings());
    ENCODE_CHK_STATUS_RETURN(SetVdencCmd1Settings());
    ENCODE_CHK_STATUS_RETURN(SetVdencCmd2Settings());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePreEncConstSettings::SetVdencCmd1Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    m_featureSetting->vdencCmd1Settings = {
        PREENC_VDENC_CMD1_LAMBDA()
        {
            static constexpr std::array<std::array<double, 4>, 2> ConstTable1 =
            {{
                {0.68445, 1.03428, 1.17, 1.17},
                {0.7605, 0.9464, 0.9464, 1.04}
            }};

            static constexpr std::array<double, 13> LowDelayTable =
            {
                0.7048, 0.763533, 0.822267, 0.881, 0.939733, 0.998467,
                1.0572, 1.11593, 1.17467, 1.2334, 1.29213, 1.35087, 1.4096
            };

            static constexpr std::array<double, 52> ConstTable2 =
            {
                1.000000,  1.000000,  1.000000,  1.000000,  1.000000, 
                1.000000,  1.000000,  1.000000,  1.000000,  1.000000,
                1.000000,  1.000000,  1.259921,  1.587401,  2.000000, 
                2.519842,  3.174802,  4.000000,  5.039684,  6.349604, 
                8.000000,  10.079368, 12.699208, 16.000000, 20.158737,
                25.398417, 32.000000, 40.317474, 50.796834, 64.000000, 
                80.634947, 101.593667, 128.0000, 161.269894, 203.187335,
                256.00000, 322.539789, 406.374669, 512.0000, 645.079578,
                812.749339, 1024.0000, 1290.159155, 1625.498677, 2048.0, 
                2580.31831, 3250.997354, 4096.0000, 5160.636620, 6501.994709, 8192
            };

            double doubleNum0;

            uint32_t bGopSize = preEncConfig.GopRefDist;
            int32_t  depth    = preEncConfig.HierarchLevelPlus1 ? preEncConfig.HierarchLevelPlus1 - 1 : 0;
            uint8_t  qp       = 22; // preenc use const qp 22

            if (preEncConfig.LowDelayMode)
            {
                if (preEncConfig.CodingType == I_TYPE)
                {
                    doubleNum0 = 0.4845;
                }
                else
                {
                    if (depth == 0)
                    {
                        doubleNum0 = 0.578;
                    }
                    else
                    {
                        int tmp = CLIP3(24, 36, qp);
                        doubleNum0 = LowDelayTable[tmp - 24];
                    }
                }
            }
            else
            {
                if (preEncConfig.CodingType == I_TYPE)
                {
                    doubleNum0 = 0.60;
                }
                else if (preEncConfig.CodingType == B_TYPE && bGopSize == 4)
                {
                    doubleNum0 = ConstTable1[0][depth];
                }
                else if (preEncConfig.CodingType == B_TYPE && bGopSize == 8)
                {
                    doubleNum0 = ConstTable1[1][depth];
                }
                else
                {
                    doubleNum0 = 0.65;
                }
            }

            double doubleNum1 = doubleNum0 * ConstTable2[qp - 1];
            par.vdencCmd1Par0 = (uint16_t)(MOS_MIN(65535, doubleNum1 * 4 + 0.5));

            doubleNum1 = sqrt(doubleNum1);
            par.vdencCmd1Par1 = (uint16_t)(MOS_MIN(65535, doubleNum1 * 4 + 0.5));

            par.vdencCmd1Par2[0] = 0;
            par.vdencCmd1Par2[1] = 2;
            par.vdencCmd1Par2[2] = 3;
            par.vdencCmd1Par2[3] = 5;
            par.vdencCmd1Par2[4] = 6;
            par.vdencCmd1Par2[5] = 8;
            par.vdencCmd1Par2[6] = 9;
            par.vdencCmd1Par2[7] = 11;

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            static const std::array<uint8_t, 12> data = {
                4, 12, 20, 28, 36, 44, 52, 60, 68, 76, 84, 92
            };

            for (size_t i = 0; i < data.size(); i++)
            {
                par.vdencCmd1Par3[i] = data[i];
            }

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            static const std::array<uint8_t, 12> data = {
                3, 10, 16, 22, 29, 35, 42, 48, 54, 61, 67, 74};

            for (size_t i = 0; i < data.size(); i++)
            {
                par.vdencCmd1Par4[i] = data[i];
            }

            return MOS_STATUS_SUCCESS;
        },

        PREENC_VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par22 = 4;

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par24 = 0;
            par.vdencCmd1Par25 = 0;
            par.vdencCmd1Par26 = 0;
            par.vdencCmd1Par27 = 0;
            par.vdencCmd1Par28 = 0;
            par.vdencCmd1Par29 = 0;
            par.vdencCmd1Par30 = 0;
            par.vdencCmd1Par31 = 0;

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par32 = 0;
            par.vdencCmd1Par33 = 0;

            if (preEncConfig.CodingType == I_TYPE)
            {
                par.vdencCmd1Par34 = 21;
                par.vdencCmd1Par35 = 0;
            }
            else
            {
                par.vdencCmd1Par34 = 7;
                par.vdencCmd1Par35 = 4;
            }

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par44 = 0;
            par.vdencCmd1Par45 = 20;
            par.vdencCmd1Par46 = 0;

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par47 = 12;
            par.vdencCmd1Par48 = 12;
            par.vdencCmd1Par49 = 12;
            par.vdencCmd1Par50 = 12;
            par.vdencCmd1Par51 = 12;
            par.vdencCmd1Par52 = 12;
            par.vdencCmd1Par53 = 12;
            par.vdencCmd1Par54 = 12;

            return MOS_STATUS_SUCCESS;
        },
        PREENC_VDENC_CMD1_LAMBDA()
        {
            par.vdencCmd1Par55 = 0x0E;
            par.vdencCmd1Par56 = 0x0E;
            par.vdencCmd1Par57 = 0x0C;
            par.vdencCmd1Par58 = 0x0B;
            par.vdencCmd1Par59 = 0x10;
            par.vdencCmd1Par60 = 0x10;
            par.vdencCmd1Par61 = 0x0F;
            par.vdencCmd1Par62 = 0x0F;
            par.vdencCmd1Par63 = 0x10;
            par.vdencCmd1Par64 = 0x10;
            par.vdencCmd1Par65 = 0x10;
            par.vdencCmd1Par66 = 0x10;
            par.vdencCmd1Par67 = 0x14;
            par.vdencCmd1Par68 = 0x10;
            par.vdencCmd1Par69 = 0x10;
            par.vdencCmd1Par70 = 0x10;
            par.vdencCmd1Par71 = 0x0C;
            par.vdencCmd1Par72 = 0x0C;
            par.vdencCmd1Par73 = 0x0A;
            par.vdencCmd1Par74 = 0x0A;
            par.vdencCmd1Par75 = 0x10;
            par.vdencCmd1Par76 = 0x10;
            par.vdencCmd1Par77 = 0x10;
            par.vdencCmd1Par78 = 0x10;
            par.vdencCmd1Par79 = 0x10;
            par.vdencCmd1Par80 = 0x10;
            par.vdencCmd1Par81 = 0x10;
            par.vdencCmd1Par82 = 0x10;
            par.vdencCmd1Par83 = 0x10;
            par.vdencCmd1Par84 = 0x10;
            par.vdencCmd1Par85 = 0x0E;
            par.vdencCmd1Par86 = 0x0F;

            return MOS_STATUS_SUCCESS;
        }
    };

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            static const std::array<uint8_t, 16> data = {
                11, 0, 0, 0, 14, 0, 0, 0,
                11, 0, 0, 0, 0, 0, 0, 0
            };

            if (preEncConfig.CodingType == I_TYPE)
            {
                return MOS_STATUS_SUCCESS;
            }

            for (size_t i = 0; i < 4; i++)
            {
                par.vdencCmd1Par8[i]  = data[i];
                par.vdencCmd1Par9[i]  = data[i + 4];
                par.vdencCmd1Par10[i] = data[i + 8];
                par.vdencCmd1Par11[i] = data[i + 12];
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            static const std::array<uint8_t, 16> data = {
                23, 0, 0, 0, 26, 0, 0, 0,
                21, 0, 0, 0, 0, 0, 0, 0
            };
            if (preEncConfig.CodingType == I_TYPE)
            {
                return MOS_STATUS_SUCCESS;
            }

            for (size_t i = 0; i < 4; i++)
            {
                par.vdencCmd1Par12[i] = data[i];
                par.vdencCmd1Par13[i] = data[i + 4];
                par.vdencCmd1Par14[i] = data[i + 8];
                par.vdencCmd1Par15[i] = data[i + 12];
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            if (preEncConfig.CodingType == P_TYPE)
            {
                par.vdencCmd1Par16 = 82;
                par.vdencCmd1Par17 = 20;
                par.vdencCmd1Par18 = 83;
                par.vdencCmd1Par19 = 17;
                par.vdencCmd1Par20 = 15;
                par.vdencCmd1Par21 = 0;
            }
            else if (preEncConfig.CodingType == B_TYPE)
            {
                par.vdencCmd1Par16 = 99;
                par.vdencCmd1Par17 = 23;
                par.vdencCmd1Par18 = 99;
                par.vdencCmd1Par19 = 19;
                par.vdencCmd1Par20 = 17;
                par.vdencCmd1Par21 = 0;
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            if (preEncConfig.CodingType == I_TYPE)
            {
                par.vdencCmd1Par23 = 63;
            }
            else
            {
                par.vdencCmd1Par23 = 54;
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            if (preEncConfig.CodingType == I_TYPE)
            {
                 par.vdencCmd1Par30 = 12;
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            if (preEncConfig.CodingType == I_TYPE)
            {
                par.vdencCmd1Par36 = 17;
                par.vdencCmd1Par37 = 47;
                par.vdencCmd1Par38 = 20;
                par.vdencCmd1Par39 = 9;
                par.vdencCmd1Par40 = 17;
                par.vdencCmd1Par41 = 30;
            }
            else
            {
                par.vdencCmd1Par36 = 7;
                par.vdencCmd1Par37 = 18;
                par.vdencCmd1Par38 = 18;
                par.vdencCmd1Par39 = 18;
                par.vdencCmd1Par40 = 27;
                par.vdencCmd1Par41 = 68;
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            if (preEncConfig.CodingType == P_TYPE)
            {
                par.vdencCmd1Par48 = 0;
                par.vdencCmd1Par49 = 32;
                par.vdencCmd1Par50 = 68;
            }

            return MOS_STATUS_SUCCESS;
        });

    m_featureSetting->vdencCmd1Settings.emplace_back(
        PREENC_VDENC_CMD1_LAMBDA() {
            static constexpr std::array<
                std::array<uint8_t,
                    3>,
                3>
                data = {{
                    {20, 35, 35},
                    {20, 35, 35},
                    {47, 16, 16}
                    }};

            if (preEncConfig.CodingType == I_TYPE)
            {
                par.vdencCmd1Par87 = data[2][2];
                par.vdencCmd1Par88 = data[2][1];
                par.vdencCmd1Par89 = data[2][0];
            }
            else if (preEncConfig.CodingType == P_TYPE)
            {
                par.vdencCmd1Par87 = data[1][2];
                par.vdencCmd1Par88 = data[1][1];
                par.vdencCmd1Par89 = data[1][0];
            }
            else if (preEncConfig.CodingType == B_TYPE)
            {
                par.vdencCmd1Par87 = data[0][2];
                par.vdencCmd1Par88 = data[0][1];
                par.vdencCmd1Par89 = data[0][0];
            }

            return MOS_STATUS_SUCCESS;
        });


    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePreEncConstSettings::SetCommonSettings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

    m_featureSetting->transformSkipCoeffsTable = {{
        {{
            {{
                {{
                    {42, 37}, {32, 40}
                }},
                {{
                    {40, 40}, {32, 45}
                }}
            }},
            {{
                {{
                    {29, 48}, {26, 53}
                }},
                {{
                    {26, 56}, {24, 62}
                }}
            }}
        }},
        {{
            {{
                {{
                    {42, 40}, {32, 45}
                }},
                {{
                    {40, 46}, {32, 48}
                }}
            }},
            {{
                {{
                    {26, 53}, {24, 58}
                }},
                {{
                    {32, 53}, {26, 64}
                }}
            }}
        }},
        {{
            {{
                {{
                    {38, 42}, {32, 51}
                }},
                {{
                    {43, 43}, {35, 46}
                }}
            }},
            {{
                {{
                    {26, 56}, {24, 64}
                }},
                {{
                    {35, 50}, {32, 57}
                }}
            }}
        }},
        {{
            {{
                {{
                    {35, 46}, {32, 52}
                }},
                {{
                    {51, 42}, {38, 53}
                }}
            }},
            {{
                {{
                    {29, 56}, {29, 70}
                }},
                {{
                    {38, 47}, {37, 64}
                }}
            }}
        }},
    }};

    m_featureSetting->transformSkipLambdaTable = {
    149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149, 149,
    149, 149, 149, 149, 149, 149, 149, 149, 149, 162, 174, 186, 199, 211, 224, 236,
    249, 261, 273, 286, 298, 298, 298, 298, 298, 298, 298, 298, 298, 298, 298, 298,
    298, 298, 298, 298
    };

    m_featureSetting->rdoqLamdas8bits = {{
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
        }},
        {{
            {   //Inter Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }},
    {{
        {{
            {   //Intra Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
        {{
            {   //Inter Luma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }}
}};

    m_featureSetting->rdoqLamdas10bits = {{
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
                0x0003, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000a, 0x000d,
                0x0011, 0x0015, 0x001a, 0x0021, 0x002a, 0x0034, 0x0042, 0x0053,
                0x0069, 0x0084, 0x00a6, 0x00d2, 0x0108, 0x014d, 0x01a3, 0x0210,
                0x029a, 0x0347, 0x0421, 0x0533, 0x068d, 0x0841, 0x0a66, 0x0d1a,
                0x1082, 0x14cd, 0x1a35, 0x2105, 0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002,
                0x0003, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000a, 0x000d,
                0x0011, 0x0015, 0x001a, 0x0021, 0x002a, 0x0034, 0x0042, 0x0053,
                0x0069, 0x0084, 0x00a6, 0x00d2, 0x0108, 0x014d, 0x01a3, 0x0210,
                0x029a, 0x0347, 0x0421, 0x0533, 0x068d, 0x0841, 0x0a66, 0x0d1a,
                0x1082, 0x14cd, 0x1a35, 0x2105, 0x299a, 0x346a, 0x4209, 0x5333
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }},
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0006, 0x0007,
                0x0009, 0x000b, 0x000e, 0x0012, 0x0016, 0x001c, 0x0023, 0x002c,
                0x0038, 0x0046, 0x0059, 0x0075, 0x009b, 0x00cc, 0x010c, 0x0160,
                0x01cd, 0x025b, 0x0314, 0x0405, 0x053d, 0x06d2, 0x08df, 0x0b2d,
                0x0e14, 0x11bd, 0x165a, 0x1c29, 0x237b, 0x2cb4, 0x3852, 0x46f5,
                0x5967, 0x70a4, 0x8deb, 0xb2ce, 0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005, 0x0007,
                0x0008, 0x000b, 0x000d, 0x0011, 0x0015, 0x001b, 0x0021, 0x002a,
                0x0035, 0x0043, 0x0054, 0x006c, 0x008c, 0x00b4, 0x00e7, 0x0129,
                0x017d, 0x01ea, 0x0275, 0x0327, 0x040c, 0x0530, 0x06a7, 0x0862,
                0x0a8f, 0x0d4e, 0x10c3, 0x151f, 0x1a9c, 0x2187, 0x2a3d, 0x3538,
                0x430d, 0x547b, 0x6a70, 0x861b, 0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }}
}};

    m_featureSetting->rdoqLamdas12bits = {{
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001,
                0x0001, 0x0001, 0x0002, 0x0002, 0x0003, 0x0003, 0x0004, 0x0005,
                0x0007, 0x0008, 0x000a, 0x000d, 0x0011, 0x0015, 0x001a, 0x0021,
                0x002a, 0x0034, 0x0042, 0x0053, 0x0069, 0x0084, 0x00a6, 0x00d2,
                0x0108, 0x014d, 0x01a3, 0x0210, 0x029a, 0x0347, 0x0421, 0x0533,
                0x068d, 0x0841, 0x0a66, 0x0d1a, 0x1082, 0x14cd, 0x1a35, 0x2105,
                0x299a, 0x346a, 0x4209, 0x5333
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }},
    {{
        {{
            {   //Intra Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Intra Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
        {{
            {   //Inter Luma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0006, 0x0007, 0x0009, 0x000b, 0x000e, 0x0012,
                0x0016, 0x001c, 0x0023, 0x002c, 0x0038, 0x0046, 0x0059, 0x0075,
                0x009b, 0x00cc, 0x010c, 0x0160, 0x01cd, 0x025b, 0x0314, 0x0405,
                0x053d, 0x06d2, 0x08df, 0x0b2d, 0x0e14, 0x11bd, 0x165a, 0x1c29,
                0x237b, 0x2cb4, 0x3852, 0x46f5, 0x5967, 0x70a4, 0x8deb, 0xb2ce,
                0xe148, 0xffff, 0xffff, 0xffff
            },
            {   //Inter Chroma
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
                0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0002, 0x0002, 0x0003,
                0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000b, 0x000d, 0x0011,
                0x0015, 0x001b, 0x0021, 0x002a, 0x0035, 0x0043, 0x0054, 0x006c,
                0x008c, 0x00b4, 0x00e7, 0x0129, 0x017d, 0x01ea, 0x0275, 0x0327,
                0x040c, 0x0530, 0x06a7, 0x0862, 0x0a8f, 0x0d4e, 0x10c3, 0x151f,
                0x1a9c, 0x2187, 0x2a3d, 0x3538, 0x430d, 0x547b, 0x6a70, 0x861b,
                0xa8f6, 0xd4e0, 0xffff, 0xffff
            },
        }},
    }}
}};

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS EncodePreEncConstSettings::SetVdencCmd2Settings()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(m_featureSetting);

#if !(_MEDIA_RESERVED)
    m_featureSetting->vdencCmd2Settings.emplace_back(
        PREENC_VDENC_CMD2_LAMBDA()
        {
            par.extSettings.emplace_back(
                [this, isLowDelay, preEncConfig, &par](uint32_t *data) {
                    uint32_t CodingTypeMinus1 = preEncConfig.CodingType - 1;
                    uint32_t lowDelay         = isLowDelay;

                    static const uint32_t dw2Lut = 0x3;
                    data[2] |= dw2Lut;

                    static const uint32_t dw5Lut[3] = { 0xc0ac00, 0xc1ac00, 0xc0ac00,};
                    data[5] |= dw5Lut[CodingTypeMinus1];

                    static const uint32_t dw7Lut[3][2] = { { 0x64003, 0x64003,}, { 0x64003, 0xe4003,}, { 0x64003, 0xe4003,},};
                    data[7] |= dw7Lut[CodingTypeMinus1][lowDelay];

                    static const uint32_t dw8Lut[3][2] = { { 0x54555555, 0,}, { 0xfffdccaa, 0xfffdccaa,}, { 0xfffdfcaa, 0xfffdccaa,},};
                    data[8] |= dw8Lut[CodingTypeMinus1][lowDelay];

                    static const uint32_t dw9Lut[3][2] = { { 0x635555, 0x630000,}, { 0x63ffff, 0x63ffff,}, { 0x63fcff, 0x63ffff,},};
                    data[9] |= dw9Lut[CodingTypeMinus1][lowDelay];

                    static const uint32_t dw11Lut = 0x80000000;
                    data[11] |= dw11Lut;

                    static const uint32_t dw12Lut = 0xce4014a0;
                    data[12] |= dw12Lut;

                    static const uint32_t dw16Lut = 0xf000000;
                    data[16] |= dw16Lut;

                    static const uint32_t dw19Lut = 0x98000000;
                    data[19] |= dw19Lut;

                    static const uint32_t dw23Lut = 0xcccc0000;
                    data[23] |= dw23Lut;

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

                    static const uint32_t dw35Lut = 0xecc;
                    data[35] |= dw35Lut;

                    static const uint32_t dw51Lut = 0x22223552;
                    data[51] |= dw51Lut;

                    static const uint32_t dw52Lut = 0x1255959;
                    data[52] |= dw52Lut;

                    static const uint32_t dw53Lut = 0xff000000;
                    data[53] |= dw53Lut;

                    static const uint32_t dw54Lut = 0x44000000;
                    data[54] |= dw54Lut;
                
                    return MOS_STATUS_SUCCESS;
                });

            return MOS_STATUS_SUCCESS;
        });
    m_featureSetting->vdencCmd2Settings.emplace_back(
        PREENC_VDENC_CMD2_LAMBDA()
        {
            par.extSettings.emplace_back(
                [this, preEncConfig](uint32_t *data) {

                uint8_t tmp0 = 0;
                uint8_t tmp1 = 0;

                if (preEncConfig.CodingType == I_TYPE)
                {
                    tmp0 = 10;
                }
                else if (preEncConfig.HierarchicalFlag && preEncConfig.HierarchLevelPlus1 > 0)
                {
                    //Hierachical GOP
                    if (preEncConfig.HierarchLevelPlus1 == 1)
                    {
                        tmp0 = 10;
                    }
                    else if (preEncConfig.HierarchLevelPlus1 == 2)
                    {
                        tmp0 = 9;
                    }
                    else
                    {
                        tmp0 = 8;
                    }
                }
                else
                {
                    tmp0 = 10;
                }

                if (preEncConfig.CodingType == I_TYPE)
                {
                    tmp1 = 4;
                }
                else if (preEncConfig.HierarchicalFlag && preEncConfig.HierarchLevelPlus1 > 0)
                {
                    //Hierachical GOP
                    if (preEncConfig.HierarchLevelPlus1 == 1)
                    {
                        tmp1 = 4;
                    }
                    else if (preEncConfig.HierarchLevelPlus1 == 2)
                    {
                        tmp1 = 3;
                    }
                    else
                    {
                        tmp1 = 2;
                    }
                }
                else
                {
                    tmp1 = 4;
                }

                tmp0 &= 0xf;
                tmp1 &= 0xf;

                data[32] |= (tmp1 << 16);
                data[32] |= (tmp1 << 20);
                data[32] |= (tmp0 << 24);
                data[32] |= (tmp0 << 28);

                data[33] |= tmp1;
                data[33] |= (tmp1 << 4);
                data[33] |= (tmp1 << 8);
                data[33] |= (tmp1 << 12);
                data[33] |= (tmp0 << 16);
                data[33] |= (tmp0 << 20);
                data[33] |= (tmp1 << 24);
                data[33] |= (tmp1 << 28);

                data[34] |= tmp1;
                data[34] |= (tmp1 << 4);
                data[34] |= (tmp0 << 8);
                data[34] |= (tmp0 << 12);
                data[34] |= (tmp1 << 16);
                data[34] |= (tmp1 << 20);

                return MOS_STATUS_SUCCESS;
            });

            return MOS_STATUS_SUCCESS;
        });
#else
#define VDENC_CMD2_SETTINGS_EXT
    #include "encode_preenc_const_settings_ext.h"
#undef VDENC_CMD2_SETTINGS_EXT
#endif  // !(_MEDIA_RESERVED)

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
