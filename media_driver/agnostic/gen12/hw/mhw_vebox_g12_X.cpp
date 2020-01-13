/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file     mhw_vebox_g12_X.cpp
//! \brief    Constructs vebox commands on Gen12-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_vebox_g12_X.h"
#include "mos_solo_generic.h"
#include "media_user_settings_mgr_g12.h"
#include "mhw_mi_g12_X.h"


// H2S Manual Mode Coef
static const uint16_t g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Input_g12[HDR_OETF_1DLUT_POINT_NUMBER] =
{
       0,     257,     514,     771,    1028,    1285,    1542,    1799,    2056,    2313,    2570,    2827,    3084,    3341,    3598,    3855,
    4112,    4369,    4626,    4883,    5140,    5397,    5654,    5911,    6168,    6425,    6682,    6939,    7196,    7453,    7710,    7967,
    8224,    8481,    8738,    8995,    9252,    9509,    9766,   10023,   10280,   10537,   10794,   11051,   11308,   11565,   11822,   12079,
   12336,   12593,   12850,   13107,   13364,   13621,   13878,   14135,   14392,   14649,   14906,   15163,   15420,   15677,   15934,   16191,
   16448,   16705,   16962,   17219,   17476,   17733,   17990,   18247,   18504,   18761,   19018,   19275,   19532,   19789,   20046,   20303,
   20560,   20817,   21074,   21331,   21588,   21845,   22102,   22359,   22616,   22873,   23130,   23387,   23644,   23901,   24158,   24415,
   24672,   24929,   25186,   25443,   25700,   25957,   26214,   26471,   26728,   26985,   27242,   27499,   27756,   28013,   28270,   28527,
   28784,   29041,   29298,   29555,   29812,   30069,   30326,   30583,   30840,   31097,   31354,   31611,   31868,   32125,   32382,   32639,
   32896,   33153,   33410,   33667,   33924,   34181,   34438,   34695,   34952,   35209,   35466,   35723,   35980,   36237,   36494,   36751,
   37008,   37265,   37522,   37779,   38036,   38293,   38550,   38807,   39064,   39321,   39578,   39835,   40092,   40349,   40606,   40863,
   41120,   41377,   41634,   41891,   42148,   42405,   42662,   42919,   43176,   43433,   43690,   43947,   44204,   44461,   44718,   44975,
   45232,   45489,   45746,   46003,   46260,   46517,   46774,   47031,   47288,   47545,   47802,   48059,   48316,   48573,   48830,   49087,
   49344,   49601,   49858,   50115,   50372,   50629,   50886,   51143,   51400,   51657,   51914,   52171,   52428,   52685,   52942,   53199,
   53456,   53713,   53970,   54227,   54484,   54741,   54998,   55255,   55512,   55769,   56026,   56283,   56540,   56797,   57054,   57311,
   57568,   57825,   58082,   58339,   58596,   58853,   59110,   59367,   59624,   59881,   60138,   60395,   60652,   60909,   61166,   61423,
   61680,   61937,   62194,   62451,   62708,   62965,   63222,   63479,   63736,   63993,   64250,   64507,   64764,   65021,   65278,   65535
};

static const uint16_t g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output_g12[HDR_OETF_1DLUT_POINT_NUMBER] =
{
       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       0,       1,
       1,       1,       1,       1,       1,       1,       1,       2,       2,       2,       2,       2,       3,       3,       3,       4,
       4,       4,       5,       5,       5,       6,       6,       7,       7,       8,       9,       9,      10,      11,      11,      12,
      13,      14,      15,      16,      17,      18,      19,      20,      22,      23,      24,      26,      27,      29,      31,      32,
      34,      36,      38,      40,      43,      45,      47,      50,      52,      55,      58,      61,      64,      67,      71,      74,
      78,      82,      86,      90,      95,      99,     104,     109,     114,     119,     125,     131,     137,     143,     150,     157,
     164,     171,     179,     187,     195,     204,     213,     222,     232,     242,     252,     263,     274,     286,     298,     311,
     324,     338,     352,     367,     382,     398,     414,     431,     449,     467,     486,     506,     527,     548,     570,     593,
     617,     641,     667,     693,     721,     749,     779,     809,     841,     874,     908,     944,     980,    1018,    1058,    1099,
    1141,    1185,    1231,    1278,    1327,    1377,    1430,    1484,    1541,    1599,    1660,    1722,    1787,    1855,    1925,    1997,
    2072,    2150,    2230,    2314,    2400,    2490,    2583,    2679,    2778,    2882,    2989,    3099,    3214,    3333,    3457,    3584,
    3717,    3854,    3996,    4143,    4296,    4454,    4618,    4787,    4963,    5146,    5335,    5530,    5733,    5943,    6161,    6387,
    6621,    6863,    7115,    7375,    7645,    7925,    8215,    8515,    8827,    9150,    9485,    9832,   10192,   10565,   10952,   11353,
   11769,   12200,   12647,   13110,   13591,   14089,   14606,   15142,   15698,   16275,   16873,   17494,   18138,   18805,   19498,   20217,
   20963,   21736,   22539,   23372,   24237,   25134,   26066,   27032,   28036,   29077,   30158,   31281,   32446,   33656,   34912,   36217,
   37572,   38979,   40441,   41959,   43536,   45174,   46876,   48645,   50482,   52392,   54376,   56438,   58582,   60810,   63127,   65535
};

static const uint16_t g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[HDR_OETF_1DLUT_POINT_NUMBER] =
{
       0,     257,     514,     771,    1028,    1285,    1542,    1799,    2056,    2313,    2570,    2827,    3084,    3341,    3598,    3855,
    4112,    4369,    4626,    4883,    5140,    5397,    5654,    5911,    6168,    6425,    6682,    6939,    7196,    7453,    7710,    7967,
    8224,    8481,    8738,    8995,    9252,    9509,    9766,   10023,   10280,   10537,   10794,   11051,   11308,   11565,   11822,   12079,
   12336,   12593,   12850,   13107,   13364,   13621,   13878,   14135,   14392,   14649,   14906,   15163,   15420,   15677,   15934,   16191,
   16448,   16705,   16962,   17219,   17476,   17733,   17990,   18247,   18504,   18761,   19018,   19275,   19532,   19789,   20046,   20303,
   20560,   20817,   21074,   21331,   21588,   21845,   22102,   22359,   22616,   22873,   23130,   23387,   23644,   23901,   24158,   24415,
   24672,   24929,   25186,   25443,   25700,   25957,   26214,   26471,   26728,   26985,   27242,   27499,   27756,   28013,   28270,   28527,
   28784,   29041,   29298,   29555,   29812,   30069,   30326,   30583,   30840,   31097,   31354,   31611,   31868,   32125,   32382,   32639,
   32896,   33153,   33410,   33667,   33924,   34181,   34438,   34695,   34952,   35209,   35466,   35723,   35980,   36237,   36494,   36751,
   37008,   37265,   37522,   37779,   38036,   38293,   38550,   38807,   39064,   39321,   39578,   39835,   40092,   40349,   40606,   40863,
   41120,   41377,   41634,   41891,   42148,   42405,   42662,   42919,   43176,   43433,   43690,   43947,   44204,   44461,   44718,   44975,
   45232,   45489,   45746,   46003,   46260,   46517,   46774,   47031,   47288,   47545,   47802,   48059,   48316,   48573,   48830,   49087,
   49344,   49601,   49858,   50115,   50372,   50629,   50886,   51143,   51400,   51657,   51914,   52171,   52428,   52685,   52942,   53199,
   53456,   53713,   53970,   54227,   54484,   54741,   54998,   55255,   55512,   55769,   56026,   56283,   56540,   56797,   57054,   57311,
   57568,   57825,   58082,   58339,   58596,   58853,   59110,   59367,   59624,   59881,   60138,   60395,   60652,   60909,   61166,   61423,
   61680,   61937,   62194,   62451,   62708,   62965,   63222,   63479,   63736,   63993,   64250,   64507,   64764,   65021,   65278,   65535
};

static const uint16_t g_Hdr_ColorCorrect_OETF_Rec709_Output_g12[HDR_OETF_1DLUT_POINT_NUMBER] =
{
       0,    1157,    2313,    3469,    4626,    5788,    6838,    7795,    8680,    9505,   10282,   11017,   11716,   12383,   13023,   13639,
   14232,   14805,   15359,   15898,   16420,   16929,   17424,   17907,   18379,   18840,   19291,   19733,   20165,   20589,   21006,   21414,
   21816,   22211,   22599,   22981,   23357,   23727,   24092,   24451,   24806,   25155,   25500,   25841,   26177,   26509,   26837,   27161,
   27481,   27798,   28111,   28421,   28727,   29030,   29330,   29627,   29921,   30213,   30501,   30786,   31069,   31350,   31628,   31903,
   32176,   32447,   32715,   32982,   33246,   33507,   33767,   34025,   34281,   34535,   34787,   35037,   35285,   35531,   35776,   36019,
   36260,   36500,   36738,   36974,   37209,   37443,   37674,   37905,   38134,   38361,   38587,   38812,   39035,   39257,   39478,   39697,
   39915,   40132,   40348,   40562,   40776,   40988,   41199,   41409,   41617,   41825,   42031,   42237,   42441,   42645,   42847,   43048,
   43249,   43448,   43646,   43844,   44040,   44236,   44430,   44624,   44817,   45009,   45200,   45390,   45580,   45768,   45956,   46143,
   46329,   46514,   46699,   46882,   47065,   47248,   47429,   47610,   47790,   47969,   48147,   48325,   48502,   48679,   48854,   49029,
   49204,   49378,   49551,   49723,   49895,   50066,   50236,   50406,   50575,   50744,   50912,   51080,   51246,   51413,   51578,   51743,
   51908,   52072,   52235,   52398,   52560,   52722,   52883,   53044,   53204,   53364,   53523,   53682,   53840,   53997,   54154,   54311,
   54467,   54623,   54778,   54932,   55086,   55240,   55393,   55546,   55699,   55850,   56002,   56153,   56303,   56453,   56603,   56752,
   56901,   57049,   57197,   57345,   57492,   57639,   57785,   57931,   58076,   58221,   58366,   58510,   58654,   58798,   58941,   59083,
   59226,   59368,   59509,   59651,   59792,   59932,   60072,   60212,   60351,   60490,   60629,   60768,   60906,   61043,   61181,   61318,
   61454,   61591,   61727,   61862,   61998,   62133,   62267,   62402,   62536,   62669,   62803,   62936,   63069,   63201,   63333,   63465,
   63597,   63728,   63859,   63990,   64120,   64250,   64380,   64509,   64638,   64767,   64896,   65024,   65152,   65280,   65408,   65535
};

const int32_t g_Vebox_BT2020_Inverse_Pixel_Value_g12[256] =
{
    0x0000, 0x14bc, 0x15a8, 0x1694, 0x1780, 0x1870, 0x195c, 0x1a48, 0x1b34, 0x1c24, 0x1d10, 0x1dfc, 0x1eec, 0x1fd8, 0x20c4, 0x21b0,
    0x22a0, 0x238c, 0x2478, 0x2568, 0x2654, 0x2740, 0x282c, 0x291c, 0x2a08, 0x2af4, 0x2be0, 0x2cd0, 0x2dbc, 0x2ea8, 0x2f98, 0x3084,
    0x3170, 0x325c, 0x334c, 0x3438, 0x3524, 0x3614, 0x3700, 0x37ec, 0x38d8, 0x39c8, 0x3ab4, 0x3ba0, 0x3c8c, 0x3d7c, 0x3e68, 0x3f54,
    0x4044, 0x4130, 0x421c, 0x4308, 0x43f8, 0x44e4, 0x45d0, 0x46c0, 0x47ac, 0x4898, 0x4984, 0x4a74, 0x4b60, 0x4c4c, 0x4d38, 0x4e28,
    0x4f14, 0x5000, 0x50f0, 0x51dc, 0x52c8, 0x53b4, 0x54a4, 0x5590, 0x567c, 0x576c, 0x5858, 0x5944, 0x5a30, 0x5b20, 0x5c0c, 0x5cf8,
    0x5de8, 0x5ed4, 0x5fc0, 0x60ac, 0x619c, 0x6288, 0x6374, 0x6460, 0x6550, 0x663c, 0x6728, 0x6818, 0x6904, 0x69f0, 0x6adc, 0x6bcc,
    0x6cb8, 0x6da4, 0x6e94, 0x6f80, 0x706c, 0x7158, 0x7248, 0x7334, 0x7420, 0x750c, 0x75fc, 0x76e8, 0x77d4, 0x78c4, 0x79b0, 0x7a9c,
    0x7b88, 0x7c78, 0x7d64, 0x7e50, 0x7f40, 0x802c, 0x8118, 0x8204, 0x82f4, 0x83e0, 0x84cc, 0x85b8, 0x86a8, 0x8794, 0x8880, 0x8970,
    0x8a5c, 0x8b48, 0x8c34, 0x8d24, 0x8e10, 0x8efc, 0x8fec, 0x90d8, 0x91c4, 0x92b0, 0x93a0, 0x948c, 0x9578, 0x9664, 0x9754, 0x9840,
    0x992c, 0x9a1c, 0x9b08, 0x9bf4, 0x9ce0, 0x9dd0, 0x9ebc, 0x9fa8, 0xa098, 0xa184, 0xa270, 0xa35c, 0xa44c, 0xa538, 0xa624, 0xa714,
    0xa800, 0xa8ec, 0xa9d8, 0xaac8, 0xabb4, 0xaca0, 0xad8c, 0xae7c, 0xaf68, 0xb054, 0xb144, 0xb230, 0xb31c, 0xb408, 0xb4f8, 0xb5e4,
    0xb6d0, 0xb7c0, 0xb8ac, 0xb998, 0xba84, 0xbb74, 0xbc60, 0xbd4c, 0xbe38, 0xbf28, 0xc014, 0xc100, 0xc1f0, 0xc2dc, 0xc3c8, 0xc4b4,
    0xc5a4, 0xc690, 0xc77c, 0xc86c, 0xc958, 0xca44, 0xcb30, 0xcc20, 0xcd0c, 0xcdf8, 0xcee4, 0xcfd4, 0xd0c0, 0xd1ac, 0xd29c, 0xd388,
    0xd474, 0xd560, 0xd650, 0xd73c, 0xd828, 0xd918, 0xda04, 0xdaf0, 0xdbdc, 0xdccc, 0xddb8, 0xdea4, 0xdf94, 0xe080, 0xe16c, 0xe258,
    0xe348, 0xe434, 0xe520, 0xe60c, 0xe6fc, 0xe7e8, 0xe8d4, 0xe9c4, 0xeab0, 0xeb9c, 0xec88, 0xed78, 0xee64, 0xef50, 0xf040, 0xf12c,
    0xf218, 0xf304, 0xf3f4, 0xf4e0, 0xf5cc, 0xf6b8, 0xf7a8, 0xf894, 0xf980, 0xfa70, 0xfb5c, 0xfc48, 0xfd34, 0xfe24, 0xff10, 0xffff
};

const int32_t g_Vebox_BT2020_Forward_Pixel_Value_g12[256] =
{
    0x0000, 0x049c, 0x0598, 0x0694, 0x0794, 0x0890, 0x098c, 0x0a8c, 0x0b88, 0x0c84, 0x0d84, 0x0e80, 0x0f7c, 0x107c, 0x1178, 0x1274,
    0x1374, 0x1470, 0x156c, 0x166c, 0x1768, 0x1864, 0x1964, 0x1a60, 0x1b5c, 0x1c5c, 0x1d58, 0x1e54, 0x1f54, 0x2050, 0x214c, 0x224c,
    0x2348, 0x2444, 0x2544, 0x2640, 0x273c, 0x283c, 0x2938, 0x2a34, 0x2b34, 0x2c30, 0x2d30, 0x2e2c, 0x2f28, 0x3028, 0x3124, 0x3220,
    0x3320, 0x341c, 0x3518, 0x3618, 0x3714, 0x3810, 0x3910, 0x3a0c, 0x3b08, 0x3c08, 0x3d04, 0x3e00, 0x3f00, 0x3ffc, 0x40f8, 0x41f8,
    0x42f4, 0x43f0, 0x44f0, 0x45ec, 0x46e8, 0x47e8, 0x48e4, 0x49e0, 0x4ae0, 0x4bdc, 0x4cd8, 0x4dd8, 0x4ed4, 0x4fd0, 0x50d0, 0x51cc,
    0x52c8, 0x53c8, 0x54c4, 0x55c4, 0x56c0, 0x57bc, 0x58bc, 0x59b8, 0x5ab4, 0x5bb4, 0x5cb0, 0x5dac, 0x5eac, 0x5fa8, 0x60a4, 0x61a4,
    0x62a0, 0x639c, 0x649c, 0x6598, 0x6694, 0x6794, 0x6890, 0x698c, 0x6a8c, 0x6b88, 0x6c84, 0x6d84, 0x6e80, 0x6f7c, 0x707c, 0x7178,
    0x7274, 0x7374, 0x7470, 0x756c, 0x766c, 0x7768, 0x7864, 0x7964, 0x7a60, 0x7b5c, 0x7c5c, 0x7d58, 0x7e58, 0x7f54, 0x8050, 0x8150,
    0x824c, 0x8348, 0x8448, 0x8544, 0x8640, 0x8740, 0x883c, 0x8938, 0x8a38, 0x8b34, 0x8c30, 0x8d30, 0x8e2c, 0x8f28, 0x9028, 0x9124,
    0x9220, 0x9320, 0x941c, 0x9518, 0x9618, 0x9714, 0x9810, 0x9910, 0x9a0c, 0x9b08, 0x9c08, 0x9d04, 0x9e00, 0x9f00, 0x9ffc, 0xa0f8,
    0xa1f8, 0xa2f4, 0xa3f0, 0xa4f0, 0xa5ec, 0xa6ec, 0xa7e8, 0xa8e4, 0xa9e4, 0xaae0, 0xabdc, 0xacdc, 0xadd8, 0xaed4, 0xafd4, 0xb0d0,
    0xb1cc, 0xb2cc, 0xb3c8, 0xb4c4, 0xb5c4, 0xb6c0, 0xb7bc, 0xb8bc, 0xb9b8, 0xbab4, 0xbbb4, 0xbcb0, 0xbdac, 0xbeac, 0xbfa8, 0xc0a4,
    0xc1a4, 0xc2a0, 0xc39c, 0xc49c, 0xc598, 0xc694, 0xc794, 0xc890, 0xc98c, 0xca8c, 0xcb88, 0xcc84, 0xcd84, 0xce80, 0xcf80, 0xd07c,
    0xd178, 0xd278, 0xd374, 0xd470, 0xd570, 0xd66c, 0xd768, 0xd868, 0xd964, 0xda60, 0xdb60, 0xdc5c, 0xdd58, 0xde58, 0xdf54, 0xe050,
    0xe150, 0xe24c, 0xe348, 0xe448, 0xe544, 0xe640, 0xe740, 0xe83c, 0xe938, 0xea38, 0xeb34, 0xec30, 0xed30, 0xee2c, 0xef28, 0xf028,
    0xf124, 0xf220, 0xf320, 0xf41c, 0xf518, 0xf618, 0xf714, 0xf814, 0xf910, 0xfa0c, 0xfb0c, 0xfc08, 0xfd04, 0xfe04, 0xff00, 0xffff
};
const int32_t g_Vebox_BT2020_Inverse_Gamma_LUT_g12[256] =
{
    0x0000, 0x049c, 0x04cc, 0x0503, 0x053a, 0x0574, 0x05ae, 0x05e9, 0x0626, 0x0665, 0x06a5, 0x06e5, 0x0729, 0x076c, 0x07b1, 0x07f7,
    0x083f, 0x0888, 0x08d2, 0x091f, 0x096c, 0x09bb, 0x0a0a, 0x0a5d, 0x0aaf, 0x0b03, 0x0b58, 0x0bb0, 0x0c09, 0x0c62, 0x0cbf, 0x0d1b,
    0x0d79, 0x0dd8, 0x0e3a, 0x0e9c, 0x0f00, 0x0f66, 0x0fcd, 0x1035, 0x109e, 0x110b, 0x1177, 0x11e5, 0x1254, 0x12c6, 0x1339, 0x13ac,
    0x1423, 0x149a, 0x1512, 0x158c, 0x1609, 0x1685, 0x1704, 0x1785, 0x1806, 0x1889, 0x190d, 0x1995, 0x1a1c, 0x1aa5, 0x1b2f, 0x1bbe,
    0x1c4b, 0x1cda, 0x1d6d, 0x1dff, 0x1e92, 0x1f27, 0x1fc0, 0x2059, 0x20f2, 0x2190, 0x222d, 0x22cc, 0x236c, 0x2410, 0x24b3, 0x2558,
    0x2601, 0x26a9, 0x2752, 0x27fe, 0x28ad, 0x295b, 0x2a0b, 0x2abd, 0x2b73, 0x2c28, 0x2cde, 0x2d99, 0x2e53, 0x2f0e, 0x2fcb, 0x308c,
    0x314c, 0x320e, 0x32d5, 0x339a, 0x3460, 0x3528, 0x35f6, 0x36c1, 0x378e, 0x385d, 0x3931, 0x3a03, 0x3ad7, 0x3bb0, 0x3c87, 0x3d60,
    0x3e3a, 0x3f1a, 0x3ff8, 0x40d7, 0x41bc, 0x429f, 0x4383, 0x4469, 0x4555, 0x463e, 0x4729, 0x4816, 0x4909, 0x49f9, 0x4aeb, 0x4be3,
    0x4cd8, 0x4dcf, 0x4ec8, 0x4fc7, 0x50c3, 0x51c1, 0x52c5, 0x53c6, 0x54c9, 0x55ce, 0x56d9, 0x57e1, 0x58eb, 0x59f6, 0x5b08, 0x5c17,
    0x5d28, 0x5e3f, 0x5f54, 0x606a, 0x6181, 0x62a0, 0x63bb, 0x64d8, 0x65fc, 0x671c, 0x683e, 0x6962, 0x6a8d, 0x6bb5, 0x6cde, 0x6e0e,
    0x6f3b, 0x706a, 0x719a, 0x72d1, 0x7405, 0x753b, 0x7672, 0x77b1, 0x78ec, 0x7a29, 0x7b6d, 0x7cad, 0x7def, 0x7f33, 0x807e, 0x81c6,
    0x830f, 0x8460, 0x85ad, 0x86fb, 0x884c, 0x89a4, 0x8af8, 0x8c4e, 0x8da6, 0x8f05, 0x9061, 0x91be, 0x9323, 0x9483, 0x95e6, 0x974a,
    0x98b7, 0x9a1f, 0x9b89, 0x9cfb, 0x9e68, 0x9fd8, 0xa149, 0xa2c2, 0xa437, 0xa5ae, 0xa726, 0xa8a7, 0xaa23, 0xaba2, 0xad28, 0xaeaa,
    0xb02d, 0xb1b3, 0xb341, 0xb4ca, 0xb655, 0xb7e9, 0xb977, 0xbb08, 0xbc9b, 0xbe36, 0xbfcc, 0xc164, 0xc305, 0xc4a1, 0xc63e, 0xc7de,
    0xc986, 0xcb2a, 0xcccf, 0xce76, 0xd026, 0xd1d1, 0xd37e, 0xd533, 0xd6e4, 0xd896, 0xda4a, 0xdc08, 0xddc0, 0xdf7a, 0xe13d, 0xe2fb,
    0xe4bb, 0xe67c, 0xe847, 0xea0c, 0xebd4, 0xed9d, 0xef6f, 0xf13c, 0xf30b, 0xf4e4, 0xf6b6, 0xf88b, 0xfa62, 0xfc42, 0xfe1c, 0xffff,
};

const int32_t g_Vebox_BT2020_Forward_Gamma_LUT_g12[256] =
{
    0x0000, 0x14bc, 0x1901, 0x1cd0, 0x2060, 0x23a3, 0x26b2, 0x29a2, 0x2c60, 0x2eff, 0x318a, 0x33f3, 0x3644, 0x388a, 0x3ab5, 0x3cce,
    0x3ee0, 0x40db, 0x42c9, 0x44b3, 0x4689, 0x4854, 0x4a1c, 0x4bd4, 0x4d82, 0x4f2f, 0x50cd, 0x5264, 0x53f9, 0x5582, 0x5703, 0x5885,
    0x59fb, 0x5b6a, 0x5cdb, 0x5e40, 0x5fa0, 0x6100, 0x6257, 0x63a9, 0x64fc, 0x6646, 0x6790, 0x68d2, 0x6a10, 0x6b4f, 0x6c86, 0x6db9,
    0x6eee, 0x701a, 0x7144, 0x726f, 0x7393, 0x74b3, 0x75d6, 0x76f1, 0x780a, 0x7924, 0x7a37, 0x7b48, 0x7c5b, 0x7d68, 0x7e72, 0x7f7e,
    0x8083, 0x8187, 0x828d, 0x838c, 0x8489, 0x8589, 0x8683, 0x877a, 0x8874, 0x8968, 0x8a5b, 0x8b4f, 0x8c3e, 0x8d2c, 0x8e1b, 0x8f06,
    0x8fee, 0x90d9, 0x91bf, 0x92a6, 0x9389, 0x946a, 0x954e, 0x962c, 0x970a, 0x97e9, 0x98c3, 0x999d, 0x9a78, 0x9b4f, 0x9c24, 0x9cfc,
    0x9dcf, 0x9ea1, 0x9f76, 0xa045, 0xa114, 0xa1e5, 0xa2b1, 0xa37d, 0xa44a, 0xa514, 0xa5dc, 0xa6a6, 0xa76d, 0xa832, 0xa8f9, 0xa9bd,
    0xaa7f, 0xab44, 0xac05, 0xacc4, 0xad86, 0xae44, 0xaf02, 0xafc1, 0xb07c, 0xb137, 0xb1f4, 0xb2ad, 0xb368, 0xb41f, 0xb4d6, 0xb58e,
    0xb643, 0xb6f8, 0xb7ae, 0xb861, 0xb913, 0xb9c7, 0xba78, 0xbb28, 0xbbda, 0xbc89, 0xbd36, 0xbde6, 0xbe93, 0xbf3f, 0xbfed, 0xc097,
    0xc141, 0xc1ed, 0xc296, 0xc33e, 0xc3e8, 0xc48f, 0xc535, 0xc5de, 0xc683, 0xc727, 0xc7ce, 0xc871, 0xc914, 0xc9b9, 0xca5a, 0xcafc,
    0xcb9f, 0xcc3f, 0xccde, 0xcd80, 0xce1e, 0xcebf, 0xcf5c, 0xcff9, 0xd098, 0xd134, 0xd1cf, 0xd26d, 0xd307, 0xd3a1, 0xd43d, 0xd4d6,
    0xd56e, 0xd609, 0xd6a0, 0xd738, 0xd7d1, 0xd867, 0xd8fd, 0xd994, 0xda29, 0xdabe, 0xdb54, 0xdbe8, 0xdc7b, 0xdd10, 0xdda2, 0xde34,
    0xdec8, 0xdf59, 0xdfea, 0xe07d, 0xe10c, 0xe19c, 0xe22d, 0xe2bc, 0xe34a, 0xe3db, 0xe468, 0xe4f5, 0xe584, 0xe611, 0xe69f, 0xe72b,
    0xe7b6, 0xe843, 0xe8ce, 0xe958, 0xe9e4, 0xea6e, 0xeaf7, 0xeb82, 0xec0b, 0xec93, 0xed1d, 0xeda4, 0xee2c, 0xeeb5, 0xef3b, 0xefc1,
    0xf049, 0xf0cf, 0xf154, 0xf1db, 0xf25f, 0xf2e4, 0xf36a, 0xf3ed, 0xf471, 0xf4f6, 0xf579, 0xf5fb, 0xf67f, 0xf701, 0xf783, 0xf806,
    0xf887, 0xf907, 0xf98a, 0xfa0a, 0xfa8a, 0xfb0b, 0xfb8a, 0xfc0b, 0xfc8a, 0xfd08, 0xfd89, 0xfe06, 0xfe84, 0xff03, 0xff80, 0xffff
};

MhwVeboxInterfaceG12::MhwVeboxInterfaceG12(
    PMOS_INTERFACE pInputInterface)
    : MhwVeboxInterfaceGeneric(pInputInterface)
{
    MHW_FUNCTION_ENTER;
    MEDIA_SYSTEM_INFO *pGtSystemInfo;

    m_veboxSettings             = g_Vebox_Settings_g12;
    m_vebox0InUse               = false;
    m_vebox1InUse               = false;
    m_veboxScalabilitySupported = false;
    m_veboxSplitRatio           = 50;
    memset(&m_chromaParams, 0, sizeof(m_chromaParams));
    MOS_SecureMemcpy(m_BT2020InvPixelValue, sizeof(uint32_t)* 256, g_Vebox_BT2020_Inverse_Pixel_Value_g12, sizeof(uint32_t)* 256);
    MOS_SecureMemcpy(m_BT2020FwdPixelValue, sizeof(uint32_t)* 256, g_Vebox_BT2020_Forward_Pixel_Value_g12, sizeof(uint32_t)* 256);
    MOS_SecureMemcpy(m_BT2020InvGammaLUT, sizeof(uint32_t)* 256, g_Vebox_BT2020_Inverse_Gamma_LUT_g12, sizeof(uint32_t)* 256);
    MOS_SecureMemcpy(m_BT2020FwdGammaLUT, sizeof(uint32_t)* 256, g_Vebox_BT2020_Forward_Gamma_LUT_g12, sizeof(uint32_t)* 256);

    MOS_ZeroMemory(&m_laceColorCorrection, sizeof(m_laceColorCorrection));

    MHW_CHK_NULL_NO_STATUS_RETURN(pInputInterface);
    pGtSystemInfo = pInputInterface->pfnGetGtSystemInfo(pInputInterface);
    MHW_CHK_NULL_NO_STATUS_RETURN(pGtSystemInfo);

    if (pGtSystemInfo->VEBoxInfo.IsValid &&
        pGtSystemInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled &&
        pGtSystemInfo->VEBoxInfo.Instances.Bits.VEBox1Enabled)
    {
        m_veboxScalabilitySupported = true;
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    // read the "Vebox Split Ratio" user feature
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VEBOX_SPLIT_RATIO_ID_G12,
        &UserFeatureData);
    m_veboxSplitRatio = UserFeatureData.u32Data;
#endif
}

void MhwVeboxInterfaceG12::SetVeboxIecpStateBecsc(
    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
    PMHW_VEBOX_IECP_PARAMS                 pVeboxIecpParams,
    bool                                   bEnableFECSC)
{
    PMHW_CAPPIPE_PARAMS pCapPipeParams = nullptr;
    MOS_FORMAT          dstFormat;

    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpParams);

    pCapPipeParams = &pVeboxIecpParams->CapPipeParams;
    dstFormat      = pVeboxIecpParams->dstFormat;

#define SET_COEFS(_c0, _c1, _c2, _c3, _c4, _c5, _c6, _c7, _c8) \
    {                                                          \
        pVeboxIecpState->CscState.DW0.C0 = _c0;                \
        pVeboxIecpState->CscState.DW1.C1 = _c1;                \
        pVeboxIecpState->CscState.DW2.C2 = _c2;                \
        pVeboxIecpState->CscState.DW3.C3 = _c3;                \
        pVeboxIecpState->CscState.DW4.C4 = _c4;                \
        pVeboxIecpState->CscState.DW5.C5 = _c5;                \
        pVeboxIecpState->CscState.DW6.C6 = _c6;                \
        pVeboxIecpState->CscState.DW7.C7 = _c7;                \
        pVeboxIecpState->CscState.DW8.C8 = _c8;                \
    }

#define SET_INPUT_OFFSETS(_in1, _in2, _in3)              \
    {                                                    \
        pVeboxIecpState->CscState.DW9.OffsetIn1  = _in1; \
        pVeboxIecpState->CscState.DW10.OffsetIn2 = _in2; \
        pVeboxIecpState->CscState.DW11.OffsetIn3 = _in3; \
    }

#define SET_OUTPUT_OFFSETS(_out1, _out2, _out3)            \
    {                                                      \
        pVeboxIecpState->CscState.DW9.OffsetOut1  = _out1; \
        pVeboxIecpState->CscState.DW10.OffsetOut2 = _out2; \
        pVeboxIecpState->CscState.DW11.OffsetOut3 = _out3; \
    }

    MHW_CHK_NULL_NO_STATUS_RETURN(pCapPipeParams);
    if (pCapPipeParams->bActive)
    {
        // Application controlled CSC operation
        if (pCapPipeParams->BECSCParams.bActive)
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;

            if (IS_RGB_SWAP(dstFormat))
            {
                pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
            }

            // Coeff is S2.16, so multiply the floating value by 65536
            SET_COEFS(
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[0][0] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[0][1] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[0][2] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[1][0] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[1][1] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[1][2] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[2][0] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[2][1] * 65536)),
                ((uint32_t)(pCapPipeParams->BECSCParams.Matrix[2][2] * 65536)));
            SET_INPUT_OFFSETS(
                ((uint32_t)pCapPipeParams->BECSCParams.PreOffset[0]),
                ((uint32_t)pCapPipeParams->BECSCParams.PreOffset[1]),
                ((uint32_t)pCapPipeParams->BECSCParams.PreOffset[2]));
            SET_OUTPUT_OFFSETS(
                ((uint32_t)pCapPipeParams->BECSCParams.PostOffset[0]),
                ((uint32_t)pCapPipeParams->BECSCParams.PostOffset[1]),
                ((uint32_t)pCapPipeParams->BECSCParams.PostOffset[2]));
        }
        // YUV 4:4:4 CSC to xBGR or xRGB
        else if ((bEnableFECSC || (pVeboxIecpParams->srcFormat == Format_AYUV)) &&
                 (IS_RGB_FORMAT(dstFormat)))
        {
            pVeboxIecpState->CscState.DW0.TransformEnable = true;

            if (IS_RGB_SWAP(dstFormat))
            {
                pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
            }

            // CSC matrix to convert YUV 4:4:4 to xBGR. e.g. Format_A8B8G8R8. In the
            // event that dstFormat is xRGB, driver sets R & B channel swapping via
            // CscState.DW0.YuvChannelSwap so a separate matrix is not needed.

            if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT601)
            {
                SET_COEFS(76284, 0, 104595, 76284, MOS_BITFIELD_VALUE((uint32_t)-25689, 19), MOS_BITFIELD_VALUE((uint32_t)-53280, 19), 76284, 132186, 0);

                SET_INPUT_OFFSETS(MOS_BITFIELD_VALUE((uint32_t)-2048, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16));
                SET_OUTPUT_OFFSETS(0, 0, 0);
            }
            else if (pVeboxIecpParams->ColorSpace == MHW_CSpace_BT709)
            {
                SET_COEFS(76284, 0, 117506, 76284, MOS_BITFIELD_VALUE((uint32_t)-13958, 19), MOS_BITFIELD_VALUE((uint32_t)-34930, 19), 76284, 138412, 0);

                SET_INPUT_OFFSETS(MOS_BITFIELD_VALUE((uint32_t)-2048, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16),
                    MOS_BITFIELD_VALUE((uint32_t)-16384, 16));
                SET_OUTPUT_OFFSETS(0, 0, 0);
            }
            else
            {
                MHW_ASSERT(false);
            }
        }
    }
    else if (pVeboxIecpParams->bCSCEnable)
    {
        pVeboxIecpState->CscState.DW0.TransformEnable = true;

        if (IS_RGB_SWAP(dstFormat))
        {
            pVeboxIecpState->CscState.DW0.YuvChannelSwap = true;
        }

        // Coeff is S2.16, so multiply the floating value by 65536
        SET_COEFS(
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[0] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[1] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[2] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[3] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[4] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[5] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[6] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[7] * 65536.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscCoeff[8] * 65536.0F)));

        // Offset is S15, but the SW offsets are calculated as 8bits,
        // so left shift them 7bits to be in the position of MSB
        SET_INPUT_OFFSETS(
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[0] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[1] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscInOffset[2] * 128.0F)));
        SET_OUTPUT_OFFSETS(
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[0] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[1] * 128.0F)),
            ((uint32_t)MOS_F_ROUND(pVeboxIecpParams->pfCscOutOffset[2] * 128.0F)));
    }

    pVeboxIecpState->AlphaAoiState.DW0.AlphaFromStateSelect = pVeboxIecpParams->bAlphaEnable;

    // Alpha is U16, but the SW alpha is calculated as 8bits,
    // so left shift it 8bits to be in the position of MSB
    pVeboxIecpState->AlphaAoiState.DW0.ColorPipeAlpha = pVeboxIecpParams->wAlphaValue * 256;

#undef SET_COEFS
#undef SET_INPUT_OFFSETS
#undef SET_OUTPUT_OFFSETS
}

void MhwVeboxInterfaceG12::SetVeboxSurfaces(
    PMHW_VEBOX_SURFACE_PARAMS                 pSurfaceParam,
    PMHW_VEBOX_SURFACE_PARAMS                 pDerivedSurfaceParam,
    PMHW_VEBOX_SURFACE_PARAMS                 pSkinScoreSurfaceParam,
    mhw_vebox_g12_X::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
    bool                                      bIsOutputSurface,
    bool                                      bDIEnable)
{
    uint32_t dwFormat;
    uint32_t dwSurfaceWidth;
    uint32_t dwSurfaceHeight;
    uint32_t dwSurfacePitch;
    bool     bHalfPitchForChroma;
    bool     bInterleaveChroma;
    uint16_t wUXOffset;
    uint16_t wUYOffset;
    uint16_t wVXOffset;
    uint16_t wVYOffset;
    uint8_t  bBayerOffset;
    uint8_t  bBayerStride;
    uint8_t  bBayerInputAlignment;

    mhw_vebox_g12_X::VEBOX_SURFACE_STATE_CMD VeboxSurfaceState;

    MHW_CHK_NULL_NO_STATUS_RETURN(pSurfaceParam);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxSurfaceState);

    // Initialize
    dwSurfaceWidth       = 0;
    dwSurfaceHeight      = 0;
    dwSurfacePitch       = 0;
    bHalfPitchForChroma  = false;
    bInterleaveChroma    = false;
    wUXOffset            = 0;
    wUYOffset            = 0;
    wVXOffset            = 0;
    wVYOffset            = 0;
    bBayerOffset         = 0;
    bBayerStride         = 0;
    bBayerInputAlignment = 0;
    *pVeboxSurfaceState  = VeboxSurfaceState;

    switch (pSurfaceParam->Format)
    {
        case Format_NV12:
            dwFormat          = VeboxSurfaceState.SURFACE_FORMAT_PLANAR4208;
            bInterleaveChroma = true;
            wUYOffset         = (uint16_t)pSurfaceParam->dwUYoffset;
            break;

        case Format_YUYV:
        case Format_YUY2:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBNORMAL;
            break;

        case Format_UYVY:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPY;
            break;

        case Format_AYUV:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED444A8;
            break;

        case Format_Y416:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44416;
            break;

        case Format_Y410:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44410;
            break;

        case Format_YVYU:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
            break;

        case Format_VYUY:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
            break;

        case Format_A8B8G8R8:
        case Format_X8B8G8R8:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
            break;

        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R16G16B16A16;
            break;

        case Format_L8:
        case Format_P8:
        case Format_Y8:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
            break;

        case Format_IRW0:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW1:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW2:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW3:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
            break;

        case Format_IRW4:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_IRW5:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_IRW6:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_IRW7:
            dwFormat     = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
            bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
            bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
            break;

        case Format_P010:
        case Format_P016:
            dwFormat          = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42016;
            bInterleaveChroma = true;
            wUYOffset         = (uint16_t)pSurfaceParam->dwUYoffset;
            break;

        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
            if (bIsOutputSurface)
            {
                dwFormat = VeboxSurfaceState.SURFACE_FORMAT_B8G8R8A8UNORM;
            }
            else
            {
                dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
            }
            break;

        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB;
            break;

        case Format_Y216:
        case Format_Y210:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED42216;
            break;

        case Format_P216:
        case Format_P210:
            dwFormat  = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42216;
            wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
            break;

        case Format_Y16S:
        case Format_Y16U:
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y16UNORM;
            break;

        default:
            MHW_ASSERTMESSAGE("Unsupported format.");
            goto finish;
            break;
    }

    if (!bIsOutputSurface)
    {
        // camera pipe will use 10/12/14 for LSB, 0 for MSB. For other pipeline,
        // dwBitDepth is inherited from pSrc->dwDepth which may not among (0,10,12,14)
        // For such cases should use MSB as default value.
        switch (pSurfaceParam->dwBitDepth)
        {
            case 10:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_10BITLSBALIGNEDDATA;
                break;

            case 12:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_12BITLSBALIGNEDDATA;
                break;

            case 14:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_14BITLSBALIGNEDDATA;
                break;

            case 0:
            default:
                bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
                break;
        }
    }
    else
    {
        bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
    }

    // adjust boundary for vebox
    VeboxAdjustBoundary(
        pSurfaceParam,
        &dwSurfaceWidth,
        &dwSurfaceHeight,
        bDIEnable);

    dwSurfacePitch = (pSurfaceParam->TileType == MOS_TILE_LINEAR) ? MOS_ALIGN_CEIL(pSurfaceParam->dwPitch, MHW_VEBOX_LINEAR_PITCH) : pSurfaceParam->dwPitch;

    pVeboxSurfaceState->DW1.SurfaceIdentification = bIsOutputSurface;
    pVeboxSurfaceState->DW2.Width                 = dwSurfaceWidth - 1;
    pVeboxSurfaceState->DW2.Height                = dwSurfaceHeight - 1;

    pVeboxSurfaceState->DW3.HalfPitchForChroma  = bHalfPitchForChroma;
    pVeboxSurfaceState->DW3.InterleaveChroma    = bInterleaveChroma;
    pVeboxSurfaceState->DW3.SurfaceFormat       = dwFormat;
    pVeboxSurfaceState->DW3.BayerInputAlignment = bBayerInputAlignment;
    pVeboxSurfaceState->DW3.BayerPatternOffset  = bBayerOffset;
    pVeboxSurfaceState->DW3.BayerPatternFormat  = bBayerStride;
    pVeboxSurfaceState->DW3.SurfacePitch        = dwSurfacePitch - 1;
    pVeboxSurfaceState->DW3.TiledSurface        = (pSurfaceParam->TileType != MOS_TILE_LINEAR) ? true : false;
    pVeboxSurfaceState->DW3.TileWalk            = (pSurfaceParam->TileType == MOS_TILE_Y)
                                                ? VeboxSurfaceState.TILE_WALK_TILEWALKYMAJOR
                                                : VeboxSurfaceState.TILE_WALK_TILEWALKXMAJOR;
    pVeboxSurfaceState->DW4.XOffsetForU         = wUXOffset;
    pVeboxSurfaceState->DW4.YOffsetForU         = wUYOffset;
    pVeboxSurfaceState->DW5.XOffsetForV         = wVXOffset;
    pVeboxSurfaceState->DW5.YOffsetForV         = wVYOffset;

    // May fix this for stereo surfaces
    pVeboxSurfaceState->DW6.YOffsetForFrame = pSurfaceParam->dwYoffset;
    pVeboxSurfaceState->DW6.XOffsetForFrame = 0;

    pVeboxSurfaceState->DW7.DerivedSurfacePitch                    = pDerivedSurfaceParam->dwPitch - 1;
    pVeboxSurfaceState->DW8.SurfacePitchForSkinScoreOutputSurfaces = (bIsOutputSurface && pSkinScoreSurfaceParam->bActive) ? (pSkinScoreSurfaceParam->dwPitch - 1) : 0;

finish:
    return;
}

MOS_STATUS MhwVeboxInterfaceG12::setVeboxPrologCmd(
    PMHW_MI_INTERFACE            mhwMiInterface,
    PMOS_COMMAND_BUFFER          cmdBuffer)
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    uint64_t                              auxTableBaseAddr = 0;

    MHW_RENDERHAL_CHK_NULL(mhwMiInterface);
    MHW_RENDERHAL_CHK_NULL(cmdBuffer);
    MHW_RENDERHAL_CHK_NULL(m_osInterface);

    auxTableBaseAddr = m_osInterface->pfnGetAuxTableBaseAddr(m_osInterface);

    if (auxTableBaseAddr)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams;
        MOS_ZeroMemory(&lriParams, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVe0AuxTableBaseLow;
        lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
        MHW_RENDERHAL_CHK_STATUS(mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioVe0AuxTableBaseHigh;
        lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        MHW_RENDERHAL_CHK_STATUS(mhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &lriParams));
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::VeboxAdjustBoundary(
    PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
    uint32_t                  *pdwSurfaceWidth,
    uint32_t                  *pdwSurfaceHeight,
    bool                      bDIEnable)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pSurfaceParam);
    MHW_CHK_NULL(pdwSurfaceWidth);
    MHW_CHK_NULL(pdwSurfaceHeight);
    MHW_CHK_STATUS(AdjustBoundary(pSurfaceParam, pdwSurfaceWidth, pdwSurfaceHeight, bDIEnable));

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxState(
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams,
    bool                        bCmBuffer)
{
    MOS_STATUS                        eStatus;
    PMOS_INTERFACE                    pOsInterface;
    PMOS_RESOURCE                     pVeboxParamResource = nullptr;
    PMOS_RESOURCE                     pVeboxHeapResource  = nullptr;
    PMHW_VEBOX_HEAP                   pVeboxHeap;
    PMHW_VEBOX_MODE                   pVeboxMode;
    PMHW_VEBOX_CHROMA_SAMPLING        pChromaSampling;
    PMHW_VEBOX_3D_LUT                 pLUT3D;
    uint32_t                          uiInstanceBaseAddr = 0;
    MHW_RESOURCE_PARAMS               ResourceParams;
    MOS_ALLOC_GFXRES_PARAMS           AllocParamsForBufferLinear;
    mhw_vebox_g12_X::VEBOX_STATE_CMD  cmd;

    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxStateCmdParams);

    // Initialize
    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = m_osInterface;
    pVeboxMode      = &pVeboxStateCmdParams->VeboxMode;
    pLUT3D          = &pVeboxStateCmdParams->LUT3D;
    pChromaSampling = &pVeboxStateCmdParams->ChromaSampling;

    if (!pVeboxStateCmdParams->bNoUseVeboxHeap)
    {
        MHW_CHK_NULL(m_veboxHeap);

        pVeboxHeap = m_veboxHeap;
        if (bCmBuffer)
        {
            pVeboxParamResource = pVeboxStateCmdParams->pVeboxParamSurf;
        }
        else
        {
            pVeboxHeapResource = pVeboxStateCmdParams->bUseVeboxHeapKernelResource ? &pVeboxHeap->KernelResource : &pVeboxHeap->DriverResource;

            // Calculate the instance base address
            uiInstanceBaseAddr = pVeboxHeap->uiInstanceSize * pVeboxHeap->uiCurState;
        }
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiDndiStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiDndiStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd          = & (cmd.DW2.Value);
        ResourceParams.dwLocationInCmd = 2;
        ResourceParams.HwCommandType   = MOS_VEBOX_STATE;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiIecpStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiIecpStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW4.Value);
        ResourceParams.dwLocationInCmd    = 4;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        if (pVeboxStateCmdParams->pVebox1DLookUpTables)
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource         = pVeboxStateCmdParams->pVebox1DLookUpTables;
            ResourceParams.dwOffset             = 0;
            ResourceParams.pdwCmd               = &(cmd.DW6.Value);
            ResourceParams.dwLocationInCmd      = 6;
            ResourceParams.HwCommandType        = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset   = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }
        else
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            if (bCmBuffer)
            {
                ResourceParams.presResource     = pVeboxParamResource;
                ResourceParams.dwOffset         = pVeboxHeap->uiGamutStateOffset;
            }
            else
            {
                ResourceParams.presResource     = pVeboxHeapResource;
                ResourceParams.dwOffset         = pVeboxHeap->uiGamutStateOffset + uiInstanceBaseAddr;
            }
            ResourceParams.pdwCmd               = &(cmd.DW6.Value);
            ResourceParams.dwLocationInCmd      = 6;
            ResourceParams.HwCommandType        = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset   = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiVertexTableOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiVertexTableOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW8.Value);
        ResourceParams.dwLocationInCmd    = 8;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiCapturePipeStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiCapturePipeStateOffset + uiInstanceBaseAddr;
        }

        ResourceParams.pdwCmd             = & (cmd.DW10.Value);
        ResourceParams.dwLocationInCmd    = 10;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        if (pVeboxStateCmdParams->pLaceLookUpTables)
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource       = pVeboxStateCmdParams->pLaceLookUpTables;
            ResourceParams.dwOffset           = 0;
            ResourceParams.pdwCmd             = & (cmd.DW12.Value);
            ResourceParams.dwLocationInCmd    = 12;
            ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        if (bCmBuffer)
        {
            ResourceParams.presResource = pVeboxParamResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiGammaCorrectionStateOffset;
        }
        else
        {
            ResourceParams.presResource = pVeboxHeapResource;
            ResourceParams.dwOffset     = pVeboxHeap->uiGammaCorrectionStateOffset + uiInstanceBaseAddr;
        }
        ResourceParams.pdwCmd             = & (cmd.DW14_15.Value[0]);
        ResourceParams.dwLocationInCmd    = 14;
        ResourceParams.HwCommandType      = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));

        if (pVeboxStateCmdParams->pVebox3DLookUpTables)
        {
            MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
            ResourceParams.presResource         = pVeboxStateCmdParams->pVebox3DLookUpTables;
            ResourceParams.dwOffset             = 0;
            ResourceParams.pdwCmd               = &(cmd.DW16.Value);
            ResourceParams.dwLocationInCmd      = 16;
            ResourceParams.HwCommandType        = MOS_VEBOX_STATE;
            ResourceParams.dwSharedMocsOffset   = 1 - ResourceParams.dwLocationInCmd;

            MHW_CHK_STATUS(pfnAddResourceToCmd(
                pOsInterface,
                pCmdBuffer,
                &ResourceParams));
        }        
    }
    else
    {
        // Allocate Resource to avoid Page Fault issue since HW will access it
        if (Mos_ResourceIsNull(&pVeboxStateCmdParams->DummyIecpResource))
        {
            MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));

            AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
            AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
            AllocParamsForBufferLinear.Format = Format_Buffer;
            AllocParamsForBufferLinear.dwBytes = m_veboxSettings.uiIecpStateSize;
            AllocParamsForBufferLinear.pBufName = "DummyIecpResource";

            MHW_CHK_STATUS(pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &pVeboxStateCmdParams->DummyIecpResource));
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource = &pVeboxStateCmdParams->DummyIecpResource;
        ResourceParams.dwOffset = 0;
        ResourceParams.pdwCmd = &(cmd.DW4.Value);
        ResourceParams.dwLocationInCmd = 4;
        ResourceParams.HwCommandType = MOS_VEBOX_STATE;
        ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    MHW_CHK_NULL(pVeboxMode);
    MHW_CHK_NULL(pLUT3D);
    MHW_CHK_NULL(pChromaSampling);

    cmd.DW1.ColorGamutExpansionEnable    = pVeboxMode->ColorGamutExpansionEnable;
    cmd.DW1.ColorGamutCompressionEnable  = pVeboxMode->ColorGamutCompressionEnable;
    cmd.DW1.GlobalIecpEnable             = pVeboxMode->GlobalIECPEnable;
    cmd.DW1.DnEnable                     = pVeboxMode->DNEnable;
    cmd.DW1.DiEnable                     = pVeboxMode->DIEnable;
    cmd.DW1.DnDiFirstFrame               = pVeboxMode->DNDIFirstFrame;
    cmd.DW1.DiOutputFrames               = pVeboxMode->DIOutputFrames;
    cmd.DW1.DemosaicEnable               = pVeboxMode->DemosaicEnable;
    cmd.DW1.VignetteEnable               = pVeboxMode->VignetteEnable;
    cmd.DW1.AlphaPlaneEnable             = pVeboxMode->AlphaPlaneEnable;
    cmd.DW1.HotPixelFilteringEnable      = pVeboxMode->HotPixelFilteringEnable;
    cmd.DW1.LaceCorrectionEnable         = pVeboxMode->LACECorrectionEnable;
    cmd.DW1.DisableEncoderStatistics     = pVeboxMode->DisableEncoderStatistics;
    cmd.DW1.DisableTemporalDenoiseFilter = pVeboxMode->DisableTemporalDenoiseFilter;
    cmd.DW1.SinglePipeEnable             = pVeboxMode->SinglePipeIECPEnable;
    cmd.DW1.ScalarMode                   = pVeboxMode->ScalarMode;
    cmd.DW1.ForwardGammaCorrectionEnable = pVeboxMode->ForwardGammaCorrectionEnable;
    cmd.DW1.HdrEnable                    = pVeboxMode->Hdr1DLutEnable;
    cmd.DW1.Fp16ModeEnable               = pVeboxMode->Fp16ModeEnable;

    cmd.DW17.EncDataControlFor3DLUT       = 0;

    cmd.DW17.ArbitrationPriorityControlForLut3D = pLUT3D->ArbitrationPriorityControl;
    // In GmmCachePolicyExt.h, Gen9/Gen10/Gen11/Gen12 has the same definition for MEMORY_OBJECT_CONTROL_STATE.
    // In MHW_MEMORY_OBJECT_CONTROL_PARAMS, we only defined Gen9 which intended to use for Gen9 later, so reuse Gen9 index.
    cmd.DW17.Lut3DMOCStable                     = pVeboxStateCmdParams->Vebox3DLookUpTablesSurfCtrl.Gen9.Index;
    cmd.DW18.Lut3DEnable                        = pLUT3D->Lut3dEnable;
    cmd.DW18.Lut3DSize                          = pLUT3D->Lut3dSize;

    cmd.DW18.ChromaUpsamplingCoSitedHorizontalOffset   = pChromaSampling->ChromaUpsamplingCoSitedHorizontalOffset;
    cmd.DW18.ChromaUpsamplingCoSitedVerticalOffset     = pChromaSampling->ChromaUpsamplingCoSitedVerticalOffset;
    cmd.DW18.ChromaDownsamplingCoSitedHorizontalOffset = pChromaSampling->ChromaDownsamplingCoSitedHorizontalOffset;
    cmd.DW18.ChromaDownsamplingCoSitedVerticalOffset   = pChromaSampling->ChromaDownsamplingCoSitedVerticalOffset;
    cmd.DW18.BypassChromaUpsampling                    = pChromaSampling->BypassChromaUpsampling;
    cmd.DW18.BypassChromaDownsampling                  = pChromaSampling->BypassChromaDownsampling;

    Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxDiIecp(
    PMOS_COMMAND_BUFFER           pCmdBuffer,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams)
{
    MOS_STATUS          eStatus;
    PMOS_INTERFACE      pOsInterface;
    MHW_RESOURCE_PARAMS ResourceParams;

    mhw_vebox_g12_X::VEB_DI_IECP_CMD cmd;
    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxDiIecpCmdParams);
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwCurrInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwPrevInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB

    // Initialize
    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;

    if (pVeboxDiIecpCmdParams->pOsResCurrInput)
    {
        if (pVeboxDiIecpCmdParams->CurInputSurfMMCState != MOS_MEMCOMP_DISABLED)
        {
            mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *pSurfCtrlBits;
            pSurfCtrlBits = (mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD*)&pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value;
            pSurfCtrlBits->DW0.MemoryCompressionEnable = 1;
            pSurfCtrlBits->DW0.CompressionType = pSurfCtrlBits->MEMORY_COMPRESSION_TYPE_MEDIA_COMPRESSION_ENABLE;
            if (pVeboxDiIecpCmdParams->CurInputSurfMMCState == MOS_MEMCOMP_RC)
            {
                pSurfCtrlBits->DW0.CompressionType = pSurfCtrlBits->MEMORY_COMPRESSION_TYPE_RENDER_COMPRESSION_ENABLE;
            }
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->dwCurrInputSurfOffset + pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW2.Value);
        ResourceParams.dwLocationInCmd = 2;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResPrevInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResPrevInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwPrevInputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW4.Value);
        ResourceParams.dwLocationInCmd = 4;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStmmInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW6.Value);
        ResourceParams.dwLocationInCmd = 6;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStmmOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW8.Value);
        ResourceParams.dwLocationInCmd = 8;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW10.Value);
        ResourceParams.dwLocationInCmd = 10;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResCurrOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW12.Value);
        ResourceParams.dwLocationInCmd = 12;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResPrevOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResPrevOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW14.Value);
        ResourceParams.dwLocationInCmd = 14;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStatisticsOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStatisticsOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW16.Value);
        ResourceParams.dwLocationInCmd = 16;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResAlphaOrVignette)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResAlphaOrVignette;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->AlphaOrVignetteSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW18.Value);
        ResourceParams.dwLocationInCmd = 18;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->LaceOrAceOrRgbHistogramSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW20.Value);
        ResourceParams.dwLocationInCmd = 20;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResSkinScoreSurface)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResSkinScoreSurface;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->SkinScoreSurfaceSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW22.Value);
        ResourceParams.dwLocationInCmd = 22;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (m_vebox0InUse == false && m_vebox1InUse == false)
    {
        cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
        cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;
    }
    else if (m_veboxScalabilitySupported)
    {
        uint32_t iMediumX;
        MHW_ASSERT(pVeboxDiIecpCmdParams->dwEndingX >= 127);

        iMediumX = MOS_ALIGN_FLOOR(((pVeboxDiIecpCmdParams->dwEndingX + 1) * m_veboxSplitRatio / 100), 64);
        iMediumX = MOS_CLAMP_MIN_MAX(iMediumX, 64, (pVeboxDiIecpCmdParams->dwEndingX - 63));

        if (m_vebox0InUse == true &&
            m_vebox1InUse == false)
        {
            cmd.DW1.EndingX   = iMediumX - 1;
            cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;
        }
        else if (m_vebox0InUse == false &&
                 m_vebox1InUse == true)
        {
            cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
            cmd.DW1.StartingX = iMediumX;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
    }

    Mos_AddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxGamutState(
    PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
    PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams)
{
    PMHW_VEBOX_HEAP pVeboxHeap;
    uint32_t        uiOffset;
    uint32_t        i;
    double          dInverseGamma       = 0;
    double          dForwardGamma       = 0;
    MOS_STATUS      eStatus             = MOS_STATUS_SUCCESS;
    uint16_t        usGE_Values[256][8] = {0};
    bool            bEnableCCM = false;

    PMHW_1DLUT_PARAMS                                     p1DLutParams = nullptr;
    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD                 *pIecpState;
    mhw_vebox_g12_X::VEBOX_GAMUT_CONTROL_STATE_CMD        *pGamutState, gamutCmd;
    mhw_vebox_g12_X::Gamut_Expansion_Gamma_Correction_CMD *pVeboxGEGammaCorrection, VeboxGEGammaCorrection;

    MHW_CHK_NULL(pVeboxGamutParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pIecpState =
        (mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                  pVeboxHeap->uiIecpStateOffset +
                                                  uiOffset);
    pVeboxGEGammaCorrection =
        (mhw_vebox_g12_X::Gamut_Expansion_Gamma_Correction_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                  pVeboxHeap->uiGamutStateOffset +
                                                                  uiOffset);

    MHW_CHK_NULL(pIecpState);
    MHW_CHK_NULL(pVeboxGEGammaCorrection);

    // Must initialize VeboxIecpState even if it is not used because GCE
    // requires GlobalIECP enable bit to be turned on
    if (!pVeboxIecpParams)
    {
        IecpStateInitialization(pIecpState);
    }
    pGamutState = &pIecpState->GamutState;
    MHW_CHK_NULL(pGamutState);

    if (pVeboxGamutParams->GCompMode != MHW_GAMUT_MODE_NONE)
    {
        if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_BASIC)
        {
            pGamutState->DW15.Fullrangemappingenable = false;

            if (pVeboxGamutParams->GCompBasicMode == gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR)
            {
                pGamutState->DW17.GccBasicmodeselection = gamutCmd.GCC_BASICMODESELECTION_SCALINGFACTOR;
                pGamutState->DW17.Basicmodescalingfactor =
                    pVeboxGamutParams->iBasicModeScalingFactor;
            }
        }
        else if (pVeboxGamutParams->GCompMode == MHW_GAMUT_MODE_ADVANCED)
        {
            pGamutState->DW15.Fullrangemappingenable = true;
            pGamutState->DW15.D1Out                  = pVeboxGamutParams->iDout;
            pGamutState->DW15.DOutDefault            = pVeboxGamutParams->iDoutDefault;
            pGamutState->DW15.DInDefault             = pVeboxGamutParams->iDinDefault;
            pGamutState->DW16.D1In                   = pVeboxGamutParams->iDin;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid GAMUT MODE");
        }

        // Set Vertex Table if Gamut Compression is enabled
        MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::AddVeboxVertexTable(pVeboxGamutParams->ColorSpace);
    }

    // Initialize the Gamut_Expansion_Gamma_Correction.
    *pVeboxGEGammaCorrection = VeboxGEGammaCorrection;
    if (pVeboxGamutParams->GExpMode != MHW_GAMUT_MODE_NONE)
    {
        // Need to convert YUV input to RGB before GE
        pIecpState->CscState.DW0.TransformEnable = true;
        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 1192;
            pIecpState->CscState.DW1.C1          = MOS_BITFIELD_VALUE((uint32_t)-2, 19);
            pIecpState->CscState.DW2.C2          = 1634;
            pIecpState->CscState.DW3.C3          = 1192;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-401, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-833, 19);
            pIecpState->CscState.DW6.C6          = 1192;
            pIecpState->CscState.DW7.C7          = 2066;
            pIecpState->CscState.DW8.C8          = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 1192;
            pIecpState->CscState.DW1.C1          = MOS_BITFIELD_VALUE((uint32_t)-1, 19);
            pIecpState->CscState.DW2.C2          = 1835;
            pIecpState->CscState.DW3.C3          = 1192;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-218, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-537, 19);
            pIecpState->CscState.DW6.C6          = 1192;
            pIecpState->CscState.DW7.C7          = 2164;
            pIecpState->CscState.DW8.C8          = 1;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-64, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-512, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown primary");
        }

        if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_BASIC)
        {
            pGamutState->DW0.GlobalModeEnable = true;
            pGamutState->DW1.CmW              = 1023;
        }
        else if (pVeboxGamutParams->GExpMode == MHW_GAMUT_MODE_ADVANCED)
        {
            pGamutState->DW0.GlobalModeEnable = false;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid GAMUT MODE");
        }

        pGamutState->DW1.C0 = pVeboxGamutParams->Matrix[0][0];
        pGamutState->DW0.C1 = pVeboxGamutParams->Matrix[0][1];
        pGamutState->DW3.C2 = pVeboxGamutParams->Matrix[0][2];
        pGamutState->DW2.C3 = pVeboxGamutParams->Matrix[1][0];
        pGamutState->DW5.C4 = pVeboxGamutParams->Matrix[1][1];
        pGamutState->DW4.C5 = pVeboxGamutParams->Matrix[1][2];
        pGamutState->DW7.C6 = pVeboxGamutParams->Matrix[2][0];
        pGamutState->DW6.C7 = pVeboxGamutParams->Matrix[2][1];
        pGamutState->DW8.C8 = pVeboxGamutParams->Matrix[2][2];
    }
    else if (pVeboxGamutParams->bGammaCorr)
    {
        // Need to convert YUV input to RGB before Gamma Correction
        pIecpState->CscState.DW0.TransformEnable = true;
        if (IS_RGB_SWAP(pVeboxGamutParams->dstFormat))
        {
            pIecpState->CscState.DW0.YuvChannelSwap = true;
        }
        if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC601 ||
            pVeboxGamutParams->ColorSpace == MHW_CSpace_BT601_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 104597;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-25675, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-53279, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 132201;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_xvYCC709 ||
                 pVeboxGamutParams->ColorSpace == MHW_CSpace_BT709_FullRange)
        {
            pIecpState->CscState.DW0.C0          = 76309;
            pIecpState->CscState.DW1.C1          = 0;
            pIecpState->CscState.DW2.C2          = 117489;
            pIecpState->CscState.DW3.C3          = 76309;
            pIecpState->CscState.DW4.C4          = MOS_BITFIELD_VALUE((uint32_t)-13975, 19);
            pIecpState->CscState.DW5.C5          = MOS_BITFIELD_VALUE((uint32_t)-34925, 19);
            pIecpState->CscState.DW6.C6          = 76309;
            pIecpState->CscState.DW7.C7          = 138438;
            pIecpState->CscState.DW8.C8          = 0;
            pIecpState->CscState.DW9.OffsetIn1   = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW9.OffsetOut1  = 0;
            pIecpState->CscState.DW10.OffsetIn2  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetIn3  = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)
        {
            VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
        }
        else
        {
            MHW_ASSERTMESSAGE("Unknown primary");
        }

        // CCM is needed for CSC(BT2020->BT709/BT601 or vice versa with Different Gamma).
        bEnableCCM = (pVeboxGamutParams->InputGammaValue == pVeboxGamutParams->OutputGammaValue) ? false : true;
        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW1.CmW              = 1023;
        if ((pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020) && bEnableCCM)
        {
            if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT709)
            {
                pGamutState->DW1.C0 = 108190;
                pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38288, 21);
                pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4747, 21);
                pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-7967, 21);
                pGamutState->DW5.C4 = 74174;
                pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-557, 21);
                pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1198, 21);
                pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6587, 21);
                pGamutState->DW8.C8 = 73321;
            }
            else
            {
                pGamutState->DW1.C0 = 116420;
                pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-45094, 21);
                pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-5785, 21);
                pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-10586, 21);
                pGamutState->DW5.C4 = 77814;
                pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-1705, 21);
                pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1036, 21);
                pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6284, 21);
                pGamutState->DW8.C8 = 72864;
            }
        }
        else
        {
            pGamutState->DW1.C0          = 65536;
            pGamutState->DW0.C1          = 0;
            pGamutState->DW3.C2          = 0;
            pGamutState->DW2.C3          = 0;
            pGamutState->DW5.C4          = 65536;
            pGamutState->DW4.C5          = 0;
            pGamutState->DW7.C6          = 0;
            pGamutState->DW6.C7          = 0;
            pGamutState->DW8.C8          = 65536;
            pGamutState->DW9.OffsetInR   = 0;
            pGamutState->DW10.OffsetInG  = 0;
            pGamutState->DW11.OffsetInB  = 0;
            pGamutState->DW12.OffsetOutR = 0;
            pGamutState->DW13.OffsetOutG = 0;
            pGamutState->DW14.OffsetOutB = 0;
        }

        if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_1P0)
        {
            dInverseGamma = 1.0;
        }
        else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P2)
        {
            dInverseGamma = 2.2;
        }
        else if (pVeboxGamutParams->InputGammaValue == MHW_GAMMA_2P6)
        {
            dInverseGamma = 2.6;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid InputGammaValue");
        }

        if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_1P0)
        {
            dForwardGamma = 1.0;
        }
        else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P2)
        {
            dForwardGamma = 2.2;
        }
        else if (pVeboxGamutParams->OutputGammaValue == MHW_GAMMA_2P6)
        {
            dForwardGamma = 2.6;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid OutputGammaValue");
        }

        for (i = 0; i < 255; i++)
        {
            usGE_Values[i][0] = 256 * i;
            usGE_Values[i][1] =
            usGE_Values[i][2] =
            usGE_Values[i][3] = (uint16_t)MOS_F_ROUND(pow((double)((double)i / 256), dInverseGamma) * 65536);

            usGE_Values[i][4] = 256 * i;
            usGE_Values[i][5] =
            usGE_Values[i][6] =
            usGE_Values[i][7] = (uint16_t)MOS_F_ROUND(pow((double)((double)i / 256), 1 / dForwardGamma) * 65536);
        }
        // Copy two uint16_t to one DW (UNT32).
        MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1020, usGE_Values, sizeof(uint16_t) * 8 * 255);
    }
    else if (pVeboxGamutParams->bH2S)
    {
        VeboxInterface_H2SManualMode(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020 ||
             pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)  // BT2020 CSC case
    {
        if (pVeboxIecpParams->s1DLutParams.bActive)
        {
            //CCM setting if 1Dlut VEBOX HDR enabled
            p1DLutParams = &pVeboxIecpParams->s1DLutParams;
            
            pIecpState->CcmState.DW1.C0 = p1DLutParams->pCCM[0];
            pIecpState->CcmState.DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[1], 27);
            pIecpState->CcmState.DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[2], 27);
            pIecpState->CcmState.DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[3], 27);
            pIecpState->CcmState.DW5.C4 = p1DLutParams->pCCM[4];
            pIecpState->CcmState.DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[5], 27);
            pIecpState->CcmState.DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[6], 27);
            pIecpState->CcmState.DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)p1DLutParams->pCCM[7], 27);
            pIecpState->CcmState.DW8.C8 = p1DLutParams->pCCM[8];
            pIecpState->CcmState.DW9.OffsetInR = p1DLutParams->pCCM[9];
            pIecpState->CcmState.DW10.OffsetInG = p1DLutParams->pCCM[10];
            pIecpState->CcmState.DW11.OffsetInB = p1DLutParams->pCCM[11];
            pIecpState->CcmState.DW12.OffsetOutR = p1DLutParams->pCCM[12];
            pIecpState->CcmState.DW13.OffsetOutG = p1DLutParams->pCCM[13];
            pIecpState->CcmState.DW14.OffsetOutB = p1DLutParams->pCCM[14];

            pGamutState->DW0.GlobalModeEnable = false;
            // Still need to set CSC params here
            VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
            goto finish;
        }

        pGamutState->DW0.GlobalModeEnable = true;
        pGamutState->DW1.CmW = 1023;  // Colorimetric accurate image
        if (pVeboxGamutParams->dstColorSpace == MHW_CSpace_BT601)
        {
            pGamutState->DW1.C0 = 116420;
            pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-45094, 21);
            pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-5785, 21);
            pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-10586, 21);
            pGamutState->DW5.C4 = 77814;
            pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-1705, 21);
            pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1036, 21);
            pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6284, 21);
            pGamutState->DW8.C8 = 72864;
        }
        else  //BT709, sRGB has same chromaticity CIE 1931
        {
            pGamutState->DW1.C0 = 108190;
            pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38288, 21);
            pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4747, 21);
            pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-7967, 21);
            pGamutState->DW5.C4 = 74174;
            pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-557, 21);
            pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1198, 21);
            pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6587, 21);
            pGamutState->DW8.C8 = 73321;
        }

        for (i = 0; i < 256; i++)
        {
            usGE_Values[i][0] = (uint16_t)m_BT2020InvPixelValue[i];
            usGE_Values[i][1] =
                usGE_Values[i][2] =
                    usGE_Values[i][3] = (uint16_t)m_BT2020InvGammaLUT[i];

            usGE_Values[i][4] = (uint16_t)m_BT2020FwdPixelValue[i];
            usGE_Values[i][5] =
                usGE_Values[i][6] =
                    usGE_Values[i][7] = (uint16_t)m_BT2020FwdGammaLUT[i];
        }
        // Copy two UNT16 to one DW(UNT32).
        MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
        // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
        VeboxInterface_BT2020YUVToRGB(m_veboxHeap, pVeboxIecpParams, pVeboxGamutParams);
    }
    else if (pVeboxIecpParams && pVeboxIecpParams->s1DLutParams.bActive)
    {
        uint16_t in_val = 0, vchan1_y = 0, vchan2_u = 0, vchan3_v = 0;
        uint32_t nIndex = 0;
        uint16_t* pForwardGamma = (uint16_t*)pVeboxIecpParams->s1DLutParams.p1DLUT;
        MHW_CHK_NULL(pForwardGamma);

        // Gamut Expansion setting
        pGamutState->DW0.GlobalModeEnable     = true;
        pGamutState->DW1.CmW                  = 1023;
        dInverseGamma                         = 1.0;

        for (uint32_t i = 0; i < pVeboxIecpParams->s1DLutParams.LUTSize; i++)
        {
            usGE_Values[i][0] = 257 * i;
            usGE_Values[i][1] =
            usGE_Values[i][2] =
            usGE_Values[i][3] = 257 * i;

            nIndex      = 4 * i;
            in_val      = pForwardGamma[nIndex];
            vchan1_y    = pForwardGamma[nIndex + 1];
            vchan2_u    = pForwardGamma[nIndex + 2];
            vchan3_v    = pForwardGamma[nIndex + 3];

            // ayuv: in_val, vchan1_y, vchan2_u, vchan3_v
            usGE_Values[i][4] = (i == 0) ? 0 : ((i == 255) ? 0xffff : in_val);
            usGE_Values[i][5] = vchan1_y;
            usGE_Values[i][6] = vchan2_u;
            usGE_Values[i][7] = vchan3_v;
        }
        pGamutState->DW1.C0 = 65536;
        pGamutState->DW0.C1 = 0;
        pGamutState->DW3.C2 = 0;
        pGamutState->DW2.C3 = 0;
        pGamutState->DW5.C4 = 65536;
        pGamutState->DW4.C5 = 0;
        pGamutState->DW7.C6 = 0;
        pGamutState->DW6.C7 = 0;
        pGamutState->DW8.C8 = 65536;
        pGamutState->DW9.OffsetInR   = 0;
        pGamutState->DW10.OffsetInG  = 0;
        pGamutState->DW11.OffsetInB  = 0;
        pGamutState->DW12.OffsetOutR = 0;
        pGamutState->DW13.OffsetOutG = 0;
        pGamutState->DW14.OffsetOutB = 0;
        // Copy two uint16_t to one DW (UNT32).
        MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t) * 1024, usGE_Values, sizeof(uint16_t) * 8 * 256);
    }
    else
    {
        MHW_ASSERTMESSAGE("Unknown branch!");
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::FindVeboxGpuNodeToUse(
    PMHW_VEBOX_GPUNODE_LIMIT pGpuNodeLimit)
{
    MOS_GPU_NODE   VeboxGpuNode = MOS_GPU_NODE_VE;
    MOS_STATUS     eStatus      = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pGpuNodeLimit);

    // KMD Virtual Engine, use virtual GPU NODE-- MOS_GPU_NODE_VE
    pGpuNodeLimit->dwGpuNodeToUse = VeboxGpuNode;

#if !EMUL
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_osInterface != nullptr && m_osInterface->bEnableDbgOvrdInVE && !m_veboxScalabilitySupported)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("not support DebugOverride on current OS or Platform.");
        goto finish;
    }

    if ((m_osInterface != nullptr && m_osInterface->bEnableDbgOvrdInVE) ||
        Mos_Solo_IsInUse(m_osInterface))
    {
        MHW_CHK_STATUS(ValidateVeboxScalabilityConfig());
    }
#endif

    Mos_Solo_CheckNodeLimitation(m_osInterface, &pGpuNodeLimit->dwGpuNodeToUse);
#endif

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxDndiState(
    PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams)
{
    PMHW_VEBOX_HEAP pVeboxHeap;
    uint32_t        uiOffset;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g12_X::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, mVeboxDndiState;

    MHW_CHK_NULL(pVeboxDndiParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxDndiState =
        (mhw_vebox_g12_X::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                  pVeboxHeap->uiDndiStateOffset +
                                                  uiOffset);
    MHW_CHK_NULL(pVeboxDndiState);
    *pVeboxDndiState = mVeboxDndiState;

    pVeboxDndiState->DW0.DenoiseMaximumHistory                         = pVeboxDndiParams->dwDenoiseMaximumHistory;
    pVeboxDndiState->DW0.DenoiseStadThreshold                          = pVeboxDndiParams->dwDenoiseSTADThreshold;
    pVeboxDndiState->DW1.DenoiseAsdThreshold                           = pVeboxDndiParams->dwDenoiseASDThreshold;
    pVeboxDndiState->DW1.DenoiseHistoryIncrease                        = pVeboxDndiParams->dwDenoiseHistoryDelta;
    pVeboxDndiState->DW1.DenoiseMovingPixelThreshold                   = pVeboxDndiParams->dwDenoiseMPThreshold;
    pVeboxDndiState->DW2.TemporalDifferenceThreshold                   = pVeboxDndiParams->dwTDThreshold;
    pVeboxDndiState->DW3.LowTemporalDifferenceThreshold                = pVeboxDndiParams->dwLTDThreshold;
    pVeboxDndiState->DW3.ProgressiveDn                                 = pVeboxDndiParams->bProgressiveDN;
    pVeboxDndiState->DW3.HotPixelCountLuma                             = pVeboxDndiParams->dwHotPixelCount;
    pVeboxDndiState->DW4.DenoiseThresholdForSumOfComplexityMeasureLuma = pVeboxDndiParams->dwDenoiseSCMThreshold;
    pVeboxDndiState->DW4.HotPixelThresholdLuma                         = pVeboxDndiParams->dwHotPixelThreshold;
    pVeboxDndiState->DW5.ChromaDenoiseStadThreshold                    = pVeboxDndiParams->dwChromaSTADThreshold;
    pVeboxDndiState->DW5.HotPixelCountChromaU                          = m_chromaParams.dwHotPixelCountChromaU;
    pVeboxDndiState->DW5.HotPixelThresholdChromaU                      = m_chromaParams.dwHotPixelThresholdChromaU;
    pVeboxDndiState->DW6.ChromaDenoiseEnable                           = pVeboxDndiParams->bChromaDNEnable;
    pVeboxDndiState->DW6.ChromaTemporalDifferenceThreshold             = pVeboxDndiParams->dwChromaTDThreshold;
    pVeboxDndiState->DW7.ChromaLowTemporalDifferenceThreshold          = pVeboxDndiParams->dwChromaLTDThreshold;
    pVeboxDndiState->DW7.HotPixelCountChromaV                          = m_chromaParams.dwHotPixelCountChromaV;
    pVeboxDndiState->DW7.HotPixelThresholdChromaV                      = m_chromaParams.dwHotPixelThresholdChromaV;
    pVeboxDndiState->DW8.ChromaDenoiseMovingPixelThreshold             = m_chromaParams.dwHotPixelThresholdChromaV;

    pVeboxDndiState->DW9.DnyWr040 = pVeboxDndiParams->dwPixRangeWeight[0];
    pVeboxDndiState->DW9.DnyWr140 = pVeboxDndiParams->dwPixRangeWeight[1];
    pVeboxDndiState->DW9.DnyWr240 = pVeboxDndiParams->dwPixRangeWeight[2];
    pVeboxDndiState->DW9.DnyWr340 = pVeboxDndiParams->dwPixRangeWeight[3];
    pVeboxDndiState->DW9.DnyWr440 = pVeboxDndiParams->dwPixRangeWeight[4];
    pVeboxDndiState->DW9.DnyWr540 = pVeboxDndiParams->dwPixRangeWeight[5];

    pVeboxDndiState->DW11.DnyPrt5120 = pVeboxDndiParams->dwPixRangeThreshold[5];
    pVeboxDndiState->DW12.DnyPrt4120 = pVeboxDndiParams->dwPixRangeThreshold[4];
    pVeboxDndiState->DW12.DnyPrt3120 = pVeboxDndiParams->dwPixRangeThreshold[3];
    pVeboxDndiState->DW13.DnyPrt2120 = pVeboxDndiParams->dwPixRangeThreshold[2];
    pVeboxDndiState->DW13.DnyPrt1120 = pVeboxDndiParams->dwPixRangeThreshold[1];
    pVeboxDndiState->DW14.DnyPrt0120 = pVeboxDndiParams->dwPixRangeThreshold[0];

    pVeboxDndiState->DW16.DnuWr040 = m_chromaParams.dwPixRangeWeightChromaU[0];
    pVeboxDndiState->DW16.DnuWr140 = m_chromaParams.dwPixRangeWeightChromaU[1];
    pVeboxDndiState->DW16.DnuWr240 = m_chromaParams.dwPixRangeWeightChromaU[2];
    pVeboxDndiState->DW16.DnuWr340 = m_chromaParams.dwPixRangeWeightChromaU[3];
    pVeboxDndiState->DW16.DnuWr440 = m_chromaParams.dwPixRangeWeightChromaU[4];
    pVeboxDndiState->DW16.DnuWr540 = m_chromaParams.dwPixRangeWeightChromaU[5];

    pVeboxDndiState->DW18.DnuPrt5120 = m_chromaParams.dwPixRangeThresholdChromaU[5];
    pVeboxDndiState->DW19.DnuPrt4120 = m_chromaParams.dwPixRangeThresholdChromaU[4];
    pVeboxDndiState->DW19.DnuPrt3120 = m_chromaParams.dwPixRangeThresholdChromaU[3];
    pVeboxDndiState->DW20.DnuPrt2120 = m_chromaParams.dwPixRangeThresholdChromaU[2];
    pVeboxDndiState->DW20.DnuPrt1120 = m_chromaParams.dwPixRangeThresholdChromaU[1];
    pVeboxDndiState->DW21.DnuPrt0120 = m_chromaParams.dwPixRangeThresholdChromaU[0];

    pVeboxDndiState->DW23.DnvWr040  = m_chromaParams.dwPixRangeWeightChromaV[0];
    pVeboxDndiState->DW23.DnvWr5140 = m_chromaParams.dwPixRangeWeightChromaV[1];
    pVeboxDndiState->DW23.DnvWr240  = m_chromaParams.dwPixRangeWeightChromaV[2];
    pVeboxDndiState->DW23.DnvWr340  = m_chromaParams.dwPixRangeWeightChromaV[3];
    pVeboxDndiState->DW23.DnvWr440  = m_chromaParams.dwPixRangeWeightChromaV[4];
    pVeboxDndiState->DW23.DnvWr540  = m_chromaParams.dwPixRangeWeightChromaV[5];

    pVeboxDndiState->DW25.DnvPrt5120 = m_chromaParams.dwPixRangeThresholdChromaV[5];
    pVeboxDndiState->DW26.DnvPrt4120 = m_chromaParams.dwPixRangeThresholdChromaV[4];
    pVeboxDndiState->DW26.DnvPrt3120 = m_chromaParams.dwPixRangeThresholdChromaV[3];
    pVeboxDndiState->DW27.DnvPrt2120 = m_chromaParams.dwPixRangeThresholdChromaV[2];
    pVeboxDndiState->DW27.DnvPrt1120 = m_chromaParams.dwPixRangeThresholdChromaV[1];
    pVeboxDndiState->DW28.DnvPrt0120 = m_chromaParams.dwPixRangeThresholdChromaV[0];

    pVeboxDndiState->DW38.DnDiTopFirst = pVeboxDndiParams->bDNDITopFirst;

    pVeboxDndiState->DW39.ProgressiveCadenceReconstructionFor1StFieldOfCurrentFrame  = pVeboxDndiParams->dwFMDFirstFieldCurrFrame;
    pVeboxDndiState->DW39.ProgressiveCadenceReconstructionFor2NdFieldOfPreviousFrame = pVeboxDndiParams->dwFMDSecondFieldPrevFrame;

    // Improved Deinterlacing
    pVeboxDndiState->DW36.LumatdmWt   = pVeboxDndiParams->dwLumaTDMWeight;
    pVeboxDndiState->DW36.ChromatdmWt = pVeboxDndiParams->dwChromaTDMWeight;

    pVeboxDndiState->DW37.CoringThresholdForSvcm = pVeboxDndiParams->dwSVCMThreshold;
    pVeboxDndiState->DW37.DeltabitValueForSvcm   = pVeboxDndiParams->dwSVCMDelta;
    pVeboxDndiState->DW37.CoringThresholdForShcm = pVeboxDndiParams->dwSHCMThreshold;
    pVeboxDndiState->DW37.DeltabitValueForShcm   = pVeboxDndiParams->dwSHCMDelta;

    pVeboxDndiState->DW39.ChromaSmallerWindowForTdm = pVeboxDndiParams->bTDMChromaSmallerWindow;
    pVeboxDndiState->DW39.LumaSmallerWindowForTdm   = pVeboxDndiParams->bTDMLumaSmallerWindow;
    pVeboxDndiState->DW39.Fastercovergence          = pVeboxDndiParams->bFasterConvergence;

    pVeboxDndiState->DW40.SadWt0 = pVeboxDndiParams->dwSADWT0;
    pVeboxDndiState->DW40.SadWt1 = pVeboxDndiParams->dwSADWT1;
    pVeboxDndiState->DW40.SadWt2 = pVeboxDndiParams->dwSADWT2;
    pVeboxDndiState->DW40.SadWt3 = pVeboxDndiParams->dwSADWT3;
    pVeboxDndiState->DW41.SadWt4 = pVeboxDndiParams->dwSADWT4;
    pVeboxDndiState->DW41.SadWt6 = pVeboxDndiParams->dwSADWT6;

    pVeboxDndiState->DW41.CoringThresholdForLumaSadCalculation   = pVeboxDndiParams->dwLumaTDMCoringThreshold;
    pVeboxDndiState->DW41.CoringThresholdForChromaSadCalculation = pVeboxDndiParams->dwChromaTDMCoringThreshold;

    pVeboxDndiState->DW42.ParDiffcheckslackthreshold   = pVeboxDndiParams->dwDiffCheckSlackThreshold;
    pVeboxDndiState->DW42.ParTearinghighthreshold      = pVeboxDndiParams->dwTearingHighThreshold;
    pVeboxDndiState->DW42.ParTearinglowthreshold       = pVeboxDndiParams->dwTearingLowThreshold;
    pVeboxDndiState->DW42.ParDirectioncheckth          = pVeboxDndiParams->dwDirectionCheckThreshold;
    pVeboxDndiState->DW42.ParSyntheticcontentcheck     = pVeboxDndiParams->bSyntheticContentCheck;
    pVeboxDndiState->DW42.ParLocalcheck                = pVeboxDndiParams->bLocalCheck;
    pVeboxDndiState->DW42.ParUsesyntheticcontentmedian = pVeboxDndiParams->bUseSyntheticContentMedian;
    pVeboxDndiState->DW42.BypassDeflicker              = pVeboxDndiParams->bBypassDeflickerFilter;

    pVeboxDndiState->DW43.Lpfwtlut0 = pVeboxDndiParams->dwLPFWtLUT0;
    pVeboxDndiState->DW43.Lpfwtlut1 = pVeboxDndiParams->dwLPFWtLUT1;
    pVeboxDndiState->DW43.Lpfwtlut2 = pVeboxDndiParams->dwLPFWtLUT2;
    pVeboxDndiState->DW43.Lpfwtlut3 = pVeboxDndiParams->dwLPFWtLUT3;
    pVeboxDndiState->DW44.Lpfwtlut4 = pVeboxDndiParams->dwLPFWtLUT4;
    pVeboxDndiState->DW44.Lpfwtlut5 = pVeboxDndiParams->dwLPFWtLUT5;
    pVeboxDndiState->DW44.Lpfwtlut6 = pVeboxDndiParams->dwLPFWtLUT6;
    pVeboxDndiState->DW44.Lpfwtlut7 = pVeboxDndiParams->dwLPFWtLUT7;

    pVeboxDndiState->DW10.DnyThmin120    = 512;
    pVeboxDndiState->DW10.DnyThmax120    = 2048;
    pVeboxDndiState->DW11.DnyDynThmin120 = 256;

    pVeboxDndiState->DW14.DnyWd2040 = 10;
    pVeboxDndiState->DW14.DnyWd2140 = 10;
    pVeboxDndiState->DW14.DnyWd2240 = 8;
    pVeboxDndiState->DW15.DnyWd0040 = 12;
    pVeboxDndiState->DW15.DnyWd0140 = 12;
    pVeboxDndiState->DW15.DnyWd0240 = 10;
    pVeboxDndiState->DW15.DnyWd1040 = 12;
    pVeboxDndiState->DW15.DnyWd1140 = 11;
    pVeboxDndiState->DW15.DnyWd1240 = 10;

    pVeboxDndiState->DW17.DnuThmin120    = 512;
    pVeboxDndiState->DW17.DnuThmax120    = 2048;
    pVeboxDndiState->DW18.DnuDynThmin120 = 256;

    pVeboxDndiState->DW21.DnuWd2040 = 10;
    pVeboxDndiState->DW21.DnuWd2140 = 10;
    pVeboxDndiState->DW21.DnuWd2240 = 8;
    pVeboxDndiState->DW22.DnuWd0040 = 12;
    pVeboxDndiState->DW22.DnuWd0140 = 12;
    pVeboxDndiState->DW22.DnuWd0240 = 10;
    pVeboxDndiState->DW22.DnuWd1040 = 12;
    pVeboxDndiState->DW22.DnuWd1140 = 11;
    pVeboxDndiState->DW22.DnuWd1240 = 10;

    pVeboxDndiState->DW24.DnvThmin120    = 512;
    pVeboxDndiState->DW24.DnvThmax120    = 2048;
    pVeboxDndiState->DW25.DnvDynThmin120 = 256;

    pVeboxDndiState->DW28.DnvWd2040 = 10;
    pVeboxDndiState->DW28.DnvWd2140 = 10;
    pVeboxDndiState->DW28.DnvWd2240 = 8;
    pVeboxDndiState->DW29.DnvWd0040 = 12;
    pVeboxDndiState->DW29.DnvWd0140 = 12;
    pVeboxDndiState->DW29.DnvWd0240 = 10;
    pVeboxDndiState->DW29.DnvWd1040 = 12;
    pVeboxDndiState->DW29.DnvWd1140 = 11;
    pVeboxDndiState->DW29.DnvWd1240 = 10;

    pVeboxDndiState->DW31.SmallSobelCountThreshold  = 6;
    pVeboxDndiState->DW32.LargeSobelCountThreshold  = 6;
    pVeboxDndiState->DW32.MedianSobelCountThreshold = 40;

    pVeboxDndiState->DW34.StmmC2                                         = 2;
    pVeboxDndiState->DW35.MaximumStmm                                    = 150;
    pVeboxDndiState->DW35.MultiplierForVecm                              = 30;
    pVeboxDndiState->DW35.BlendingConstantAcrossTimeForSmallValuesOfStmm = 125;
    pVeboxDndiState->DW35.BlendingConstantAcrossTimeForLargeValuesOfStmm = 64;

    pVeboxDndiState->DW36.FmdTemporalDifferenceThreshold = 175;
    pVeboxDndiState->DW36.StmmOutputShift                = 5;
    pVeboxDndiState->DW36.StmmShiftUp                    = 1;
    pVeboxDndiState->DW36.MinimumStmm                    = 118;

    pVeboxDndiState->DW38.McdiEnable                      = 1;
    pVeboxDndiState->DW38.FmdTearThreshold                = 2;
    pVeboxDndiState->DW38.Fmd2VerticalDifferenceThreshold = 100;
    pVeboxDndiState->DW38.Fmd1VerticalDifferenceThreshold = 16;

    pVeboxDndiState->DW45.SynthticFrame                   = pVeboxDndiParams->bSyntheticFrame;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxIecpState(
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    bool                   bEnableFECSC = false;
    PMHW_FORWARD_GAMMA_SEG pFwdGammaSeg;
    uint8_t                *p3DLUT;
    PMHW_VEBOX_HEAP        pVeboxHeap;
    uint32_t               uiOffset;
    MOS_STATUS             eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

    MHW_CHK_NULL(pVeboxIecpParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap      = m_veboxHeap;
    uiOffset        = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxIecpState = (mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                pVeboxHeap->uiIecpStateOffset +
                                                                uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);
    IecpStateInitialization(pVeboxIecpState);

    if (pVeboxIecpParams->ColorPipeParams.bActive)
    {
        // Enable STD/E (Skin Tone Detection/Enhancement)
        SetVeboxIecpStateSTE(
            &pVeboxIecpState->StdSteState,
            &pVeboxIecpParams->ColorPipeParams);

        // Enable TCC (Total Color Control)
        if (pVeboxIecpParams->ColorPipeParams.bEnableTCC)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxIecpStateTCC(
                &pVeboxIecpState->TccState,
                &pVeboxIecpParams->ColorPipeParams);
        }
    }

    // Enable ACE (Automatic Contrast Enhancement). Enable LACE if it's enabled.
    if (pVeboxIecpParams->bAce ||
        (pVeboxIecpParams->ColorPipeParams.bActive &&
            pVeboxIecpParams->ColorPipeParams.bEnableACE))
    {
        MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxIecpStateACELACE(
            &pVeboxIecpState->AceState,
            &pVeboxIecpState->AlphaAoiState,
            (pVeboxIecpParams->ColorPipeParams.bEnableLACE == true) ? true : false);
    }

    if (pVeboxIecpParams->CapPipeParams.bActive)
    {
        // IECP needs to operate in YUV space
        if ((pVeboxIecpParams->srcFormat != Format_AYUV) &&
            (pVeboxIecpParams->dstFormat == Format_AYUV ||
                pVeboxIecpParams->dstFormat == Format_Y416 ||
                pVeboxIecpParams->ProcAmpParams.bActive ||
                pVeboxIecpParams->ColorPipeParams.bActive))
        {
            bEnableFECSC = true;
        }
        else if (pVeboxIecpParams->CapPipeParams.FECSCParams.bActive)
        {
            bEnableFECSC = true;
        }
        else
        {
            bEnableFECSC = false;
        }

        // Enable Front End CSC so that input to IECP will be in YUV color space
        if (bEnableFECSC)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxIecpStateFecsc(&pVeboxIecpState->FrontEndCsc, pVeboxIecpParams);
        }

        // Enable Color Correction Matrix
        if (pVeboxIecpParams->CapPipeParams.ColorCorrectionParams.bActive)
        {
            MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxIecpStateCcm(
                pVeboxIecpState,
                &pVeboxIecpParams->CapPipeParams,
                65536);
        }
    }

    // Enable Back End CSC for capture pipeline or Vebox output pipe
    if (pVeboxIecpParams->CapPipeParams.bActive ||
        pVeboxIecpParams->bCSCEnable)
    {
        SetVeboxIecpStateBecsc(
            pVeboxIecpState,
            pVeboxIecpParams,
            bEnableFECSC);
    }

    // Enable ProcAmp
    if (pVeboxIecpParams->ProcAmpParams.bActive &&
        pVeboxIecpParams->ProcAmpParams.bEnabled)
    {
        MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxIecpStateProcAmp(
            &pVeboxIecpState->ProcampState,
            &pVeboxIecpParams->ProcAmpParams);
    }

    if (pVeboxIecpParams && pVeboxIecpParams->CapPipeParams.bActive)
    {
        MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::AddVeboxCapPipeState(
            &pVeboxIecpParams->CapPipeParams);
    }

    if (pVeboxIecpParams &&
        pVeboxIecpParams->CapPipeParams.bActive &&
        pVeboxIecpParams->CapPipeParams.FwdGammaParams.bActive)
    {
        pFwdGammaSeg =
            (PMHW_FORWARD_GAMMA_SEG)(pVeboxHeap->pLockedDriverResourceMem +
                                     pVeboxHeap->uiGammaCorrectionStateOffset +
                                     uiOffset);
        MHW_CHK_NULL(pFwdGammaSeg);
        MOS_SecureMemcpy(
            pFwdGammaSeg,
            sizeof(MHW_FORWARD_GAMMA_SEG) * MHW_FORWARD_GAMMA_SEGMENT_CONTROL_POINT,
            &pVeboxIecpParams->CapPipeParams.FwdGammaParams.Segment[0],
            sizeof(MHW_FORWARD_GAMMA_SEG) * MHW_FORWARD_GAMMA_SEGMENT_CONTROL_POINT);
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxIecpAceState(
    PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams)
{
    PMHW_ACE_PARAMS  pAceParams;
    PMHW_LACE_PARAMS pLaceParams;
    PMHW_VEBOX_HEAP  pVeboxHeap;
    int32_t          uiOffset;
    MOS_STATUS       eStatus                        = MOS_STATUS_SUCCESS;
    const  uint32_t  uiFullRangeYOffsetInU16        = 0;
    const  uint32_t  uiLimitedRangeYOffsetInU16     = 4096;
    const  uint32_t  uiUOffsetInU16                 = 32768;
    const  uint32_t  uiVOffsetInU16                 = 32768;

    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState;

    MHW_CHK_NULL(pVeboxIecpParams);
    MHW_CHK_NULL(m_veboxHeap);

    pVeboxHeap = m_veboxHeap;
    uiOffset   = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;

    pVeboxIecpState = (mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                                pVeboxHeap->uiIecpStateOffset +
                                                                uiOffset);
    MHW_CHK_NULL(pVeboxIecpState);

    MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxAceLaceState(pVeboxIecpParams, pVeboxIecpState);

    if (pVeboxIecpParams->ColorPipeParams.bActive &&
        pVeboxIecpParams->ColorPipeParams.bEnableLACE)
    {
        pLaceParams = &pVeboxIecpParams->ColorPipeParams.LaceParams;

        pVeboxIecpState->AceState.DW0.MinAceLuma  = pLaceParams->wMinAceLuma;
        pVeboxIecpState->AceState.DW12.MaxAceLuma = pLaceParams->wMaxAceLuma;

        pVeboxIecpState->AceState.DW13.LaceColorCorrectionEnable = m_laceColorCorrection.bColorCorrectionEnable;
        if (m_laceColorCorrection.bYUVFullRange)
        {
            pVeboxIecpState->AceState.DW13.LaceYOffset = uiFullRangeYOffsetInU16;
            pVeboxIecpState->AceState.DW14.LaceUOffset = uiUOffsetInU16;
            pVeboxIecpState->AceState.DW14.LaceVOffset = uiVOffsetInU16;
        }
        else
        {
            pVeboxIecpState->AceState.DW13.LaceYOffset = uiLimitedRangeYOffsetInU16;
            pVeboxIecpState->AceState.DW14.LaceUOffset = uiUOffsetInU16;
            pVeboxIecpState->AceState.DW14.LaceVOffset = uiVOffsetInU16;
        }

        pVeboxIecpState->AceState.DW15.LaceGammaCurveBias0  = m_laceColorCorrection.colorWeightLut.iBias[0];
        pVeboxIecpState->AceState.DW15.LaceGammaCurvePoint0 = m_laceColorCorrection.colorWeightLut.iPoint[0];
        pVeboxIecpState->AceState.DW15.LaceGammaCurveSlope0 = m_laceColorCorrection.colorWeightLut.iSlope[0];

        pVeboxIecpState->AceState.DW16.LaceGammaCurveBias1  = m_laceColorCorrection.colorWeightLut.iBias[1];
        pVeboxIecpState->AceState.DW16.LaceGammaCurvePoint1 = m_laceColorCorrection.colorWeightLut.iPoint[1];
        pVeboxIecpState->AceState.DW16.LaceGammaCurveSlope1 = m_laceColorCorrection.colorWeightLut.iSlope[1];

        pVeboxIecpState->AceState.DW17.LaceGammaCurveBias2  = m_laceColorCorrection.colorWeightLut.iBias[2];
        pVeboxIecpState->AceState.DW17.LaceGammaCurvePoint2 = m_laceColorCorrection.colorWeightLut.iPoint[2];
        pVeboxIecpState->AceState.DW17.LaceGammaCurveSlope2 = m_laceColorCorrection.colorWeightLut.iSlope[2];

        pVeboxIecpState->AceState.DW18.LaceGammaCurveBias3  = m_laceColorCorrection.colorWeightLut.iBias[3];
        pVeboxIecpState->AceState.DW18.LaceGammaCurvePoint3 = m_laceColorCorrection.colorWeightLut.iPoint[3];
        pVeboxIecpState->AceState.DW18.LaceGammaCurveSlope3 = m_laceColorCorrection.colorWeightLut.iSlope[3];

        pVeboxIecpState->AceState.DW19.LaceGammaCurveBias4  = m_laceColorCorrection.colorWeightLut.iBias[4];
        pVeboxIecpState->AceState.DW19.LaceGammaCurvePoint4 = m_laceColorCorrection.colorWeightLut.iPoint[4];
        pVeboxIecpState->AceState.DW19.LaceGammaCurveSlope4 = m_laceColorCorrection.colorWeightLut.iSlope[4];

        pVeboxIecpState->AceState.DW20.LaceGammaCurveBias5  = m_laceColorCorrection.colorWeightLut.iBias[5];
        pVeboxIecpState->AceState.DW20.LaceGammaCurvePoint5 = m_laceColorCorrection.colorWeightLut.iPoint[5];
        pVeboxIecpState->AceState.DW20.LaceGammaCurveSlope5 = m_laceColorCorrection.colorWeightLut.iSlope[5];

        pVeboxIecpState->AceState.DW21.LaceGammaCurveBias6  = m_laceColorCorrection.colorWeightLut.iBias[6];
        pVeboxIecpState->AceState.DW21.LaceGammaCurvePoint6 = m_laceColorCorrection.colorWeightLut.iPoint[6];
        pVeboxIecpState->AceState.DW21.LaceGammaCurveSlope6 = m_laceColorCorrection.colorWeightLut.iSlope[6];

        pVeboxIecpState->AceState.DW22.LaceGammaCurveBias7  = m_laceColorCorrection.colorWeightLut.iBias[7];
        pVeboxIecpState->AceState.DW22.LaceGammaCurvePoint7 = m_laceColorCorrection.colorWeightLut.iPoint[7];
        pVeboxIecpState->AceState.DW22.LaceGammaCurveSlope7 = m_laceColorCorrection.colorWeightLut.iSlope[7];

        pVeboxIecpState->AceState.DW23.LaceGammaCurveBias8  = m_laceColorCorrection.colorWeightLut.iBias[8];
        pVeboxIecpState->AceState.DW23.LaceGammaCurvePoint8 = m_laceColorCorrection.colorWeightLut.iPoint[8];
        pVeboxIecpState->AceState.DW23.LaceGammaCurveSlope8 = m_laceColorCorrection.colorWeightLut.iSlope[8];

        pVeboxIecpState->AceState.DW24.LaceGammaCurveBias9  = m_laceColorCorrection.colorWeightLut.iBias[9];
        pVeboxIecpState->AceState.DW24.LaceGammaCurvePoint9 = m_laceColorCorrection.colorWeightLut.iPoint[9];
        pVeboxIecpState->AceState.DW24.LaceGammaCurveSlope9 = m_laceColorCorrection.colorWeightLut.iSlope[9];

        pVeboxIecpState->AceState.DW25.LaceGammaCurveBias10  = m_laceColorCorrection.colorWeightLut.iBias[10];
        pVeboxIecpState->AceState.DW25.LaceGammaCurvePoint10 = m_laceColorCorrection.colorWeightLut.iPoint[10];
        pVeboxIecpState->AceState.DW25.LaceGammaCurveSlope10 = m_laceColorCorrection.colorWeightLut.iSlope[10];

        pVeboxIecpState->AceState.DW26.LaceGammaCurveBias11  = m_laceColorCorrection.colorWeightLut.iBias[11];
        pVeboxIecpState->AceState.DW26.LaceGammaCurvePoint11 = m_laceColorCorrection.colorWeightLut.iPoint[11];
        pVeboxIecpState->AceState.DW26.LaceGammaCurveSlope11 = m_laceColorCorrection.colorWeightLut.iSlope[11];

        pVeboxIecpState->AceState.DW27.LaceGammaCurveBias12  = m_laceColorCorrection.colorWeightLut.iBias[12];
        pVeboxIecpState->AceState.DW27.LaceGammaCurvePoint12 = m_laceColorCorrection.colorWeightLut.iPoint[12];
        pVeboxIecpState->AceState.DW27.LaceGammaCurveSlope12 = m_laceColorCorrection.colorWeightLut.iSlope[12];

        pVeboxIecpState->AceState.DW28.LaceGammaCurveBias13  = m_laceColorCorrection.colorWeightLut.iBias[13];
        pVeboxIecpState->AceState.DW28.LaceGammaCurvePoint13 = m_laceColorCorrection.colorWeightLut.iPoint[13];
        pVeboxIecpState->AceState.DW28.LaceGammaCurveSlope13 = m_laceColorCorrection.colorWeightLut.iSlope[13];

        pVeboxIecpState->AceState.DW29.LaceGammaCurveBias14  = m_laceColorCorrection.colorWeightLut.iBias[14];
        pVeboxIecpState->AceState.DW29.LaceGammaCurvePoint14 = m_laceColorCorrection.colorWeightLut.iPoint[14];
        pVeboxIecpState->AceState.DW29.LaceGammaCurveSlope14 = m_laceColorCorrection.colorWeightLut.iSlope[14];

        pVeboxIecpState->AceState.DW30.LaceGammaCurveBias15  = m_laceColorCorrection.colorWeightLut.iBias[15];
        pVeboxIecpState->AceState.DW30.LaceGammaCurvePoint15 = m_laceColorCorrection.colorWeightLut.iPoint[15];
        pVeboxIecpState->AceState.DW30.LaceGammaCurveSlope15 = m_laceColorCorrection.colorWeightLut.iSlope[15];
    }

finish:
    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS MhwVeboxInterfaceG12::ValidateVeboxScalabilityConfig()
{
    MEDIA_SYSTEM_INFO *pGtSystemInfo;
    MOS_FORCE_VEBOX eForceVebox;
    bool            bScalableVEMode;
    bool            bUseVE1, bUseVE2;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(m_osInterface);

    eForceVebox     = m_osInterface->eForceVebox;
    bScalableVEMode = ((m_osInterface->bVeboxScalabilityMode) ? true : false);
    pGtSystemInfo   = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    MHW_CHK_NULL(pGtSystemInfo);

    if (eForceVebox != MOS_FORCE_VEBOX_NONE &&
        eForceVebox != MOS_FORCE_VEBOX_1 &&
        eForceVebox != MOS_FORCE_VEBOX_2 &&
        eForceVebox != MOS_FORCE_VEBOX_1_2)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("eForceVebox value is invalid.");
        goto finish;
    }

    if (!bScalableVEMode &&
        (eForceVebox == MOS_FORCE_VEBOX_1_2))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("eForceVebox value is not consistent with scalability mode.");
        goto finish;
    }

    if (bScalableVEMode && !m_veboxScalabilitySupported)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("scalability mode is not allowed on current platform!");
        goto finish;
    }

    bUseVE1 = bUseVE2 = false;
    if (eForceVebox == MOS_FORCE_VEBOX_NONE)
    {
        bUseVE1 = true;
    }
    else
    {
        MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_1, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE1);
        MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_2, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE2);
    }

    if (!pGtSystemInfo->VEBoxInfo.IsValid ||
        (bUseVE1 && !pGtSystemInfo->VEBoxInfo.Instances.Bits.VEBox0Enabled) ||
        (bUseVE2 && !pGtSystemInfo->VEBoxInfo.Instances.Bits.VEBox1Enabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("the forced VEBOX is not enabled in current platform.");
        goto finish;
    }

finish:
    return eStatus;
}
#endif

MOS_STATUS MhwVeboxInterfaceG12::SetVeboxInUse( 
    bool inputVebox0,
    bool inputVebox1)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    m_vebox0InUse = inputVebox0;
    m_vebox1InUse = inputVebox1;

    return eStatus;
}

bool MhwVeboxInterfaceG12::IsScalabilitySupported()
{
    return m_veboxScalabilitySupported;
}


MOS_STATUS MhwVeboxInterfaceG12::VeboxInterface_BT2020YUVToRGB(
    PMHW_VEBOX_HEAP             pVeboxHeapInput,
    PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
    PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams)
{
    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD   *pIecpState;
    PMHW_VEBOX_HEAP                         pVeboxHeap;
    uint32_t                                uiOffset;
    MOS_STATUS                              eStatus = MOS_STATUS_NULL_POINTER;

    MHW_CHK_NULL(pVeboxHeapInput);

    MOS_UNUSED(pVeboxIecpParams);
    MOS_UNUSED(pVeboxGamutParams);

    pVeboxHeap = pVeboxHeapInput;
    MHW_CHK_NULL(pVeboxHeap);

    uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pIecpState = (mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiIecpStateOffset + uiOffset);

    MHW_CHK_NULL(pIecpState);

    pIecpState->CscState.DW0.TransformEnable = true;

    if (IS_RGB_SWAP(pVeboxGamutParams->dstFormat))
    {
        pIecpState->CscState.DW0.YuvChannelSwap = true;
    }

    if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020)                   // Limited->Full
    {
        if (pVeboxIecpParams->s1DLutParams.bActive)
        {
            // The updated value for TGL VEBOX HDR and Fp16 path 
            pIecpState->CscState.DW0.C0 = 76533;
            pIecpState->CscState.DW1.C1 = 0;
            pIecpState->CscState.DW2.C2 = 110337;
            pIecpState->CscState.DW3.C3 = 76533;
            pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-12312, 19);
            pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-42751, 19);
            pIecpState->CscState.DW6.C6 = 76533;
            pIecpState->CscState.DW7.C7 = 140776;
            pIecpState->CscState.DW8.C8 = 0;

            pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

            pIecpState->CscState.DW9.OffsetOut1 = 0;
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
        else
        {
            pIecpState->CscState.DW0.C0 = 76607;
            pIecpState->CscState.DW1.C1 = 0;
            pIecpState->CscState.DW2.C2 = 110443;
            pIecpState->CscState.DW3.C3 = 76607;
            pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-12325, 19);
            pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-42793, 19);
            pIecpState->CscState.DW6.C6 = 76607;
            pIecpState->CscState.DW7.C7 = 140911;
            pIecpState->CscState.DW8.C8 = 0;

            pIecpState->CscState.DW9.OffsetIn1 = MOS_BITFIELD_VALUE((uint32_t)-2048, 16);
            pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
            pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

            pIecpState->CscState.DW9.OffsetOut1 = 0;
            pIecpState->CscState.DW10.OffsetOut2 = 0;
            pIecpState->CscState.DW11.OffsetOut3 = 0;
        }
    }
    else if (pVeboxGamutParams->ColorSpace == MHW_CSpace_BT2020_FullRange)    // Full->Full
    {
        pIecpState->CscState.DW0.C0 = 65536;
        pIecpState->CscState.DW1.C1 = 0;
        pIecpState->CscState.DW2.C2 = 96639;
        pIecpState->CscState.DW3.C3 = 65536;
        pIecpState->CscState.DW4.C4 = MOS_BITFIELD_VALUE((uint32_t)-10784, 19);
        pIecpState->CscState.DW5.C5 = MOS_BITFIELD_VALUE((uint32_t)-37444, 19);
        pIecpState->CscState.DW6.C6 = 65536;
        pIecpState->CscState.DW7.C7 = 123299;
        pIecpState->CscState.DW8.C8 = 0;

        pIecpState->CscState.DW9.OffsetIn1 = 0;
        pIecpState->CscState.DW10.OffsetIn2 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);
        pIecpState->CscState.DW11.OffsetIn3 = MOS_BITFIELD_VALUE((uint32_t)-16384, 16);

        pIecpState->CscState.DW9.OffsetOut1 = 0;
        pIecpState->CscState.DW10.OffsetOut2 = 0;
        pIecpState->CscState.DW11.OffsetOut3 = 0;
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported BeCSC input color space");
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::VeboxInterface_H2SManualMode(
    PMHW_VEBOX_HEAP             pVeboxHeapInput,
    PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
    PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams)
{
    PMHW_VEBOX_HEAP                         pVeboxHeap;
    uint32_t                                uiOffset;

    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD                 *pIecpState;
    mhw_vebox_g12_X::VEBOX_GAMUT_CONTROL_STATE_CMD        *pGamutState;
    mhw_vebox_g12_X::Gamut_Expansion_Gamma_Correction_CMD *pVeboxGEGammaCorrection, VeboxGeGammaCorrection;
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    // HDR H2S algorithm related
    int32_t                                 iToneMappingX[5] = { 40, 200, 1000, 2000, 4000 };
    int32_t                                 iToneMappingY[4] = { 2500, 5000, 10000, 10000 };
    float                                   fPivotX[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    float                                   fPivotY[4] = { 0.0, 0.0, 0.0, 0.0 };
    float                                   fSlope[5] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
    float                                   fMaxCLL = 0.0;
    // OETF parameters, corresponding to input
    uint32_t                                uiOETF[HDR_OETF_1DLUT_POINT_NUMBER] = { 0 };
    uint16_t                                usGE_Values[256][8] = { 0 };

    MHW_CHK_NULL(pVeboxGamutParams);
    MHW_CHK_NULL(pVeboxHeapInput);

    pVeboxHeap = pVeboxHeapInput;
    uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pIecpState = (mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD*)(pVeboxHeap->pLockedDriverResourceMem + pVeboxHeap->uiIecpStateOffset + uiOffset);
    pVeboxGEGammaCorrection = (mhw_vebox_g12_X::Gamut_Expansion_Gamma_Correction_CMD *)(pVeboxHeap->pLockedDriverResourceMem
        + pVeboxHeap->uiGamutStateOffset + uiOffset);
    fMaxCLL = (65535 * (float)pVeboxGamutParams->uiMaxCLL) / 10000;

    MHW_CHK_NULL(pIecpState);
    MHW_CHK_NULL(pVeboxGEGammaCorrection);

    // Must initialize VeboxIecpState even if it is not used because GCE
    // requires GlobalIECP enable bit to be turned on
    if (!pVeboxIecpParams)
    {
        IecpStateInitialization(pIecpState);
    }
    pGamutState = &pIecpState->GamutState;

    for (int32_t i = 0; i < 4; i++)
    {
        fPivotX[i] = (iToneMappingY[i] < 10000) ? (65535 * (float)iToneMappingX[i]) / 10000 : MOS_MIN((65535 * (float)iToneMappingX[i]) / 10000, fMaxCLL);
        fPivotY[i] = (65535 * (float)iToneMappingY[i]) / 10000;
    }
    fPivotX[4] = MOS_MIN((65535 * (float)iToneMappingX[4]) / 10000, fMaxCLL);

    // Slope
    fSlope[0] = fPivotX[0] > 0 ? (float)(fPivotY[0] / fPivotX[0]) : 0;
    fPivotY[0] = fSlope[0] * fPivotX[0];
    for (int32_t i = 1; i < 4; i++)
    {
        fSlope[i] = (fPivotX[i] - fPivotX[i - 1]) > 0 ? (float)(fPivotY[i] - fPivotY[i - 1]) / (fPivotX[i] - fPivotX[i - 1]) : 0;
        fPivotY[i] = fSlope[i] * (fPivotX[i] - fPivotX[i - 1]) + fPivotY[i - 1];
    }
    fSlope[4] = (fPivotX[4] - fPivotX[3]) > 0 ? (float)(65535 - fPivotY[3]) / (fPivotX[4] - fPivotX[3]) : 0;

    // Linear Operation
    for (int32_t n = 1; n < HDR_OETF_1DLUT_POINT_NUMBER; n++)
    {
        if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] < fSlope[0] * fPivotX[0])
        {
            uiOETF[n] = (uint32_t)((float)(g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n]) / fSlope[0]);
        }
        else if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] < fSlope[1] * (fPivotX[1] - fPivotX[0]) + fPivotY[0])
        {
            uiOETF[n] = (uint32_t)(((float)(g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n]) - fPivotY[0]) / fSlope[1] + fPivotX[0]);
        }
        else if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] < fSlope[2] * (fPivotX[2] - fPivotX[1]) + fPivotY[1])
        {
            uiOETF[n] = (uint32_t)(((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] - fPivotY[1]) / fSlope[2] + fPivotX[1]);
        }
        else if ((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] < fSlope[3] * (fPivotX[3] - fPivotX[2]) + fPivotY[2])
        {
            uiOETF[n] = (uint32_t)(((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] - fPivotY[2]) / fSlope[3] + fPivotX[2]);
        }
        else
        {
            uiOETF[n] = (uint32_t)(((float)g_Hdr_ColorCorrect_OETF_Rec709_Input_g12[n] - fPivotY[3]) / fSlope[4] + fPivotX[3]);
        }
    }
    uiOETF[0] = 0;
    uiOETF[255] = 65535;

    // Back end CSC setting, need to convert BT2020 YUV input to RGB before GE
    VeboxInterface_BT2020YUVToRGB(pVeboxHeap, pVeboxIecpParams, pVeboxGamutParams);

    // Global setting
    pGamutState->DW0.GlobalModeEnable = true;
    pGamutState->DW1.CmW = 1023; // Colorimetric accurate image

    // CCM
    pGamutState->DW1.C0 = 108822;
    pGamutState->DW0.C1 = MOS_BITFIELD_VALUE((uint32_t)-38511, 21);
    pGamutState->DW3.C2 = MOS_BITFIELD_VALUE((uint32_t)-4774, 21);
    pGamutState->DW2.C3 = MOS_BITFIELD_VALUE((uint32_t)-8163, 21);
    pGamutState->DW5.C4 = 74246;
    pGamutState->DW4.C5 = MOS_BITFIELD_VALUE((uint32_t)-547, 21);
    pGamutState->DW7.C6 = MOS_BITFIELD_VALUE((uint32_t)-1190, 21);
    pGamutState->DW6.C7 = MOS_BITFIELD_VALUE((uint32_t)-6592, 21);
    pGamutState->DW8.C8 = 73317;

    // Gamma Expansion
    *pVeboxGEGammaCorrection = VeboxGeGammaCorrection;
    for (int32_t i = 0; i < 255; i++)
    {
        usGE_Values[i][0] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Input_g12[i];
        usGE_Values[i][1] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output_g12[i];
        usGE_Values[i][2] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output_g12[i];
        usGE_Values[i][3] = g_Hdr_ColorCorrect_EOTF_SMPTE_ST2084_Output_g12[i];

        usGE_Values[i][4] = (uint16_t)uiOETF[i];
        usGE_Values[i][5] = g_Hdr_ColorCorrect_OETF_Rec709_Output_g12[i];
        usGE_Values[i][6] = g_Hdr_ColorCorrect_OETF_Rec709_Output_g12[i];
        usGE_Values[i][7] = g_Hdr_ColorCorrect_OETF_Rec709_Output_g12[i];
    }
    // Keep the last 4 DWs' value as defult 65535.See mhw_vebox_g10_X::Gamut_Expansion_Gamma_Correction_CMD();
    MOS_SecureMemcpy(pVeboxGEGammaCorrection, sizeof(uint32_t)* 1020, usGE_Values, sizeof(uint16_t)* 2040);

finish:
    return eStatus;
}

void MhwVeboxInterfaceG12::IecpStateInitialization(mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD    *pVeboxIecpState)
{
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxIecpState);

    mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD IecpState;
    *pVeboxIecpState = IecpState;

    // Re-set the values
    pVeboxIecpState->StdSteState.DW5.InvMarginVyl = 3300;
    pVeboxIecpState->StdSteState.DW5.InvSkinTypesMargin = 1638;
    pVeboxIecpState->StdSteState.DW12.B3U = 140;
    pVeboxIecpState->StdSteState.DW27.Hues0Dark = 256;
    pVeboxIecpState->StdSteState.DW27.Hues1Dark = 0;

    pVeboxIecpState->AceState.DW0.LaceHistogramSize = 1;

    pVeboxIecpState->TccState.DW0.Satfactor1 = 160;
    pVeboxIecpState->TccState.DW0.Satfactor2 = 160;
    pVeboxIecpState->TccState.DW0.Satfactor3 = 160;
    pVeboxIecpState->TccState.DW1.Satfactor4 = 160;
    pVeboxIecpState->TccState.DW1.Satfactor5 = 160;
    pVeboxIecpState->TccState.DW1.Satfactor6 = 160;

    pVeboxIecpState->GamutState.DW2.CmS = 640;
    pVeboxIecpState->GamutState.DW3.AG = 26;
    pVeboxIecpState->GamutState.DW4.AB = 26;
    pVeboxIecpState->GamutState.DW5.RS = 768;
    pVeboxIecpState->GamutState.DW6.CmI = 192;
    pVeboxIecpState->GamutState.DW7.RI = 128;

    return;
}

MOS_STATUS MhwVeboxInterfaceG12::AdjustBoundary(
    PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
    uint32_t                  *pdwSurfaceWidth,
    uint32_t                  *pdwSurfaceHeight,
    bool                      bDIEnable)
{
    uint16_t                    wWidthAlignUnit;
    uint16_t                    wHeightAlignUnit;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pSurfaceParam);
    MHW_CHK_NULL(pdwSurfaceWidth);
    MHW_CHK_NULL(pdwSurfaceHeight);

    // initialize
    wHeightAlignUnit = 1;
    wWidthAlignUnit = 1;

    switch (pSurfaceParam->Format)
    {
    case Format_NV12:
        wHeightAlignUnit = bDIEnable ? 4 : 2;
        wWidthAlignUnit = 2;
        break;

    case Format_YUYV:
    case Format_YUY2:
    case Format_UYVY:
    case Format_YVYU:
    case Format_VYUY:
    case Format_Y210:
    case Format_Y216:
        wHeightAlignUnit = bDIEnable ? 2 : 1;
        wWidthAlignUnit = 2;
        break;

    case Format_AYUV:
    case Format_Y416:
        wHeightAlignUnit = 1;
        wWidthAlignUnit = 2;
        break;

        // For other formats, we will not do any special alignment
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_L8:
    default:
        break;
    }

    // Align width and height with max src renctange with consideration of
    // these conditions:
    // The minimum of width/height should equal to or larger than
    // MHW_VEBOX_MIN_WIDTH/HEIGHT. The maximum of width/heigh should equal
    // to or smaller than surface width/height
    *pdwSurfaceHeight = MOS_ALIGN_CEIL(
        MOS_MIN(pSurfaceParam->dwHeight, MOS_MAX((uint32_t)pSurfaceParam->rcMaxSrc.bottom, MHW_VEBOX_MIN_HEIGHT)),
        wHeightAlignUnit);
    *pdwSurfaceWidth = MOS_ALIGN_CEIL(
        MOS_MIN(pSurfaceParam->dwWidth, MOS_MAX((uint32_t)pSurfaceParam->rcMaxSrc.right, MHW_VEBOX_MIN_WIDTH)),
        wWidthAlignUnit);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceG12::AddVeboxSurfaceControlBits(
    PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
    uint32_t                       *pSurfCtrlBits)
{
    PLATFORM   Platform = {};
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *pVeboxSurfCtrlBits;

    MHW_CHK_NULL(pVeboxSurfCntlParams);
    MHW_CHK_NULL(pSurfCtrlBits);
    MHW_CHK_NULL(m_osInterface);

    m_osInterface->pfnGetPlatform(m_osInterface, &Platform);

    pVeboxSurfCtrlBits = (mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *)pSurfCtrlBits;

    if (pVeboxSurfCntlParams->CompressionMode != MOS_MMC_DISABLED)
    {
        MHW_CHK_NULL(pVeboxSurfCtrlBits);
        pVeboxSurfCtrlBits->DW0.MemoryCompressionEnable = 1;

        if (pVeboxSurfCntlParams->CompressionMode == MOS_MMC_RC)
        {
            pVeboxSurfCtrlBits->DW0.CompressionType = 1;
        }
    }

finish:
    return eStatus;
}


MOS_STATUS MhwVeboxInterfaceG12::AddVeboxTilingConvert(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VEBOX_SURFACE_PARAMS        inSurParams,
    PMHW_VEBOX_SURFACE_PARAMS        outSurParams)
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    PMOS_RESOURCE                           surface = nullptr;
    PMOS_RESOURCE                           inputSurface;
    PMOS_RESOURCE                           outputSurface;
    mhw_vebox_g12_X::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD   veboxInputSurfCtrlBits, veboxOutputSurfCtrlBits;
    mhw_vebox_g12_X::VEBOX_TILING_CONVERT_CMD                       cmd;
    MHW_RESOURCE_PARAMS ResourceParams = { 0 };

    MHW_CHK_NULL(cmdBuffer);
    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(inSurParams);
    MHW_CHK_NULL(outSurParams);

    inputSurface = inSurParams->pOsResource;
    outputSurface = outSurParams->pOsResource;

    MHW_CHK_NULL(inputSurface);
    MHW_CHK_NULL(outputSurface);

    MOS_ZeroMemory(&ResourceParams, sizeof(MHW_RESOURCE_PARAMS));
    ResourceParams.presResource = inputSurface;
    ResourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;
    ResourceParams.dwLsbNum = 12; // input/output surfaces must be 4Kbyte-aligned for VEBOX_TILING_CONVERT

                                  // set up DW[2:1], input graphics address
    ResourceParams.dwLocationInCmd = 1;
    ResourceParams.pdwCmd = &(cmd.DW1_2.Value[0]);
    ResourceParams.bIsWritable = false;
    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface, 
        cmdBuffer, 
        &ResourceParams));

    MOS_ZeroMemory(&ResourceParams, sizeof(MHW_RESOURCE_PARAMS));
    ResourceParams.presResource = outputSurface;

    ResourceParams.HwCommandType = MOS_VEBOX_TILING_CONVERT;
    ResourceParams.dwLsbNum = 12; // input/output surfaces must be 4Kbyte-aligned for VEBOX_TILING_CONVERT

                                  // set up DW[4:3], output graphics address
    ResourceParams.dwLocationInCmd = 3;
    ResourceParams.pdwCmd = &(cmd.DW3_4.Value[0]);
    ResourceParams.bIsWritable = true;
    MHW_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface, 
        cmdBuffer, 
        &ResourceParams));

    // Set up VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS
    MOS_ZeroMemory(&veboxInputSurfCtrlBits, sizeof(veboxInputSurfCtrlBits));
    MOS_ZeroMemory(&veboxOutputSurfCtrlBits, sizeof(veboxOutputSurfCtrlBits));

    // Set Input surface compression status
    veboxInputSurfCtrlBits.DW0.MemoryCompressionEnable = false;

    switch (inputSurface->TileType)
    {
    case MOS_TILE_YF:
        veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYF;
        break;
    case MOS_TILE_YS:
        veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYS;
        break;
    default:
        veboxInputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_NONE;
        break;
    }

    surface = outputSurface;
    veboxOutputSurfCtrlBits.DW0.MemoryCompressionEnable = false;
    veboxOutputSurfCtrlBits.DW0.CompressionType = 0;
    if (surface)
    {
        switch (surface->TileType)
        {
        case MOS_TILE_YF:
            veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYF;
            break;
        case MOS_TILE_YS:
            veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_TILEYS;
            break;
        default:
            veboxOutputSurfCtrlBits.DW0.TiledResourceModeForOutputFrameSurfaceBaseAddress = TRMODE_NONE;
            break;
        }
    }

    cmd.DW1_2.InputSurfaceControlBits = (uint64_t)veboxInputSurfCtrlBits.DW0.Value;
    cmd.DW3_4.OutputSurfaceControlBits = (uint64_t)veboxOutputSurfCtrlBits.DW0.Value;

    Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}


MOS_STATUS MhwVeboxInterfaceG12::SetVeboxChromaParams(
    MHW_VEBOX_CHROMA_PARAMS *chromaParams)
{
    MHW_CHK_NULL_RETURN(chromaParams);
    MOS_SecureMemcpy(&m_chromaParams, sizeof(MHW_VEBOX_CHROMA_PARAMS), chromaParams, sizeof(MHW_VEBOX_CHROMA_PARAMS));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVeboxInterfaceG12::SetVeboxLaceColorParams(
    MHW_LACE_COLOR_CORRECTION *pLaceColorParams)
{
    MHW_CHK_NULL_RETURN(pLaceColorParams);
    MOS_SecureMemcpy(&m_laceColorCorrection, sizeof(MHW_LACE_COLOR_CORRECTION), pLaceColorParams, sizeof(MHW_LACE_COLOR_CORRECTION));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief    Create Gpu Context for Vebox
//! \details  Create Gpu Context for Vebox
//! \param    [in] pOsInterface
//!           OS interface
//! \param    [in] VeboxGpuContext
//!           Vebox Gpu Context
//! \param    [in] VeboxGpuNode
//!           Vebox Gpu Node
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS MhwVeboxInterfaceG12::CreateGpuContext(
    PMOS_INTERFACE  pOsInterface,
    MOS_GPU_CONTEXT VeboxGpuContext,
    MOS_GPU_NODE    VeboxGpuNode)
{
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pOsInterface);

    Mos_SetVirtualEngineSupported(pOsInterface, true);
    Mos_CheckVirtualEngineSupported(pOsInterface, true, false);

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface))
    {
        MOS_GPUCTX_CREATOPTIONS createOption;
    
        // Create VEBOX/VEBOX2 Context
        MHW_CHK_STATUS(pOsInterface->pfnCreateGpuContext(
            pOsInterface,
            VeboxGpuContext,
            VeboxGpuNode,
            &createOption));
    }
    else
    {
        MOS_GPUCTX_CREATOPTIONS_ENHANCED createOptionenhanced;
    
        createOptionenhanced.LRCACount = 1;
        createOptionenhanced.UsingSFC  = true;
    
        // Create VEBOX/VEBOX2 Context
        MHW_CHK_STATUS(pOsInterface->pfnCreateGpuContext(
            pOsInterface,
            VeboxGpuContext,
            VeboxGpuNode,
            &createOptionenhanced));
    }

finish:
    return eStatus;
}
