/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_av1_filmgrain_feature_g12.cpp
//! \brief    Defines the interface for av1 decode film grain feature
//!

#include "decode_av1_filmgrain_feature_g12.h"
#include "decode_av1_feature_manager_g12.h"
#include "codechal_utilities.h"
#include "decode_av1_feature_defs_g12.h"
#include "mhw_render_g12_X.h"
#include "decode_utils.h"

namespace decode
{

// Constant values
// Samples with Gaussian distribution in the range of [-2048, 2047] (12 bits)
// with zero mean and standard deviation of about 512.
// should be divided by 4 for 10-bit range and 16 for 8-bit range.
static const int16_t defaultGaussianSequence[2048] = {
    56,    568,   -180,  172,   124,   -84,   172,   -64,   -900,  24,   820,
    224,   1248,  996,   272,   -8,    -916,  -388,  -732,  -104,  -188, 800,
    112,   -652,  -320,  -376,  140,   -252,  492,   -168,  44,    -788, 588,
    -584,  500,   -228,  12,    680,   272,   -476,  972,   -100,  652,  368,
    432,   -196,  -720,  -192,  1000,  -332,  652,   -136,  -552,  -604, -4,
    192,   -220,  -136,  1000,  -52,   372,   -96,   -624,  124,   -24,  396,
    540,   -12,   -104,  640,   464,   244,   -208,  -84,   368,   -528, -740,
    248,   -968,  -848,  608,   376,   -60,   -292,  -40,   -156,  252,  -292,
    248,   224,   -280,  400,   -244,  244,   -60,   76,    -80,   212,  532,
    340,   128,   -36,   824,   -352,  -60,   -264,  -96,   -612,  416,  -704,
    220,   -204,  640,   -160,  1220,  -408,  900,   336,   20,    -336, -96,
    -792,  304,   48,    -28,   -1232, -1172, -448,  104,   -292,  -520, 244,
    60,    -948,  0,     -708,  268,   108,   356,   -548,  488,   -344, -136,
    488,   -196,  -224,  656,   -236,  -1128, 60,    4,     140,   276,  -676,
    -376,  168,   -108,  464,   8,     564,   64,    240,   308,   -300, -400,
    -456,  -136,  56,    120,   -408,  -116,  436,   504,   -232,  328,  844,
    -164,  -84,   784,   -168,  232,   -224,  348,   -376,  128,   568,  96,
    -1244, -288,  276,   848,   832,   -360,  656,   464,   -384,  -332, -356,
    728,   -388,  160,   -192,  468,   296,   224,   140,   -776,  -100, 280,
    4,     196,   44,    -36,   -648,  932,   16,    1428,  28,    528,  808,
    772,   20,    268,   88,    -332,  -284,  124,   -384,  -448,  208,  -228,
    -1044, -328,  660,   380,   -148,  -300,  588,   240,   540,   28,   136,
    -88,   -436,  256,   296,   -1000, 1400,  0,     -48,   1056,  -136, 264,
    -528,  -1108, 632,   -484,  -592,  -344,  796,   124,   -668,  -768, 388,
    1296,  -232,  -188,  -200,  -288,  -4,    308,   100,   -168,  256,  -500,
    204,   -508,  648,   -136,  372,   -272,  -120,  -1004, -552,  -548, -384,
    548,   -296,  428,   -108,  -8,    -912,  -324,  -224,  -88,   -112, -220,
    -100,  996,   -796,  548,   360,   -216,  180,   428,   -200,  -212, 148,
    96,    148,   284,   216,   -412,  -320,  120,   -300,  -384,  -604, -572,
    -332,  -8,    -180,  -176,  696,   116,   -88,   628,   76,    44,   -516,
    240,   -208,  -40,   100,   -592,  344,   -308,  -452,  -228,  20,   916,
    -1752, -136,  -340,  -804,  140,   40,    512,   340,   248,   184,  -492,
    896,   -156,  932,   -628,  328,   -688,  -448,  -616,  -752,  -100, 560,
    -1020, 180,   -800,  -64,   76,    576,   1068,  396,   660,   552,  -108,
    -28,   320,   -628,  312,   -92,   -92,   -472,  268,   16,    560,  516,
    -672,  -52,   492,   -100,  260,   384,   284,   292,   304,   -148, 88,
    -152,  1012,  1064,  -228,  164,   -376,  -684,  592,   -392,  156,  196,
    -524,  -64,   -884,  160,   -176,  636,   648,   404,   -396,  -436, 864,
    424,   -728,  988,   -604,  904,   -592,  296,   -224,  536,   -176, -920,
    436,   -48,   1176,  -884,  416,   -776,  -824,  -884,  524,   -548, -564,
    -68,   -164,  -96,   692,   364,   -692,  -1012, -68,   260,   -480, 876,
    -1116, 452,   -332,  -352,  892,   -1088, 1220,  -676,  12,    -292, 244,
    496,   372,   -32,   280,   200,   112,   -440,  -96,   24,    -644, -184,
    56,    -432,  224,   -980,  272,   -260,  144,   -436,  420,   356,  364,
    -528,  76,    172,   -744,  -368,  404,   -752,  -416,  684,   -688, 72,
    540,   416,   92,    444,   480,   -72,   -1416, 164,   -1172, -68,  24,
    424,   264,   1040,  128,   -912,  -524,  -356,  64,    876,   -12,  4,
    -88,   532,   272,   -524,  320,   276,   -508,  940,   24,    -400, -120,
    756,   60,    236,   -412,  100,   376,   -484,  400,   -100,  -740, -108,
    -260,  328,   -268,  224,   -200,  -416,  184,   -604,  -564,  -20,  296,
    60,    892,   -888,  60,    164,   68,    -760,  216,   -296,  904,  -336,
    -28,   404,   -356,  -568,  -208,  -1480, -512,  296,   328,   -360, -164,
    -1560, -776,  1156,  -428,  164,   -504,  -112,  120,   -216,  -148, -264,
    308,   32,    64,    -72,   72,    116,   176,   -64,   -272,  460,  -536,
    -784,  -280,  348,   108,   -752,  -132,  524,   -540,  -776,  116,  -296,
    -1196, -288,  -560,  1040,  -472,  116,   -848,  -1116, 116,   636,  696,
    284,   -176,  1016,  204,   -864,  -648,  -248,  356,   972,   -584, -204,
    264,   880,   528,   -24,   -184,  116,   448,   -144,  828,   524,  212,
    -212,  52,    12,    200,   268,   -488,  -404,  -880,  824,   -672, -40,
    908,   -248,  500,   716,   -576,  492,   -576,  16,    720,   -108, 384,
    124,   344,   280,   576,   -500,  252,   104,   -308,  196,   -188, -8,
    1268,  296,   1032,  -1196, 436,   316,   372,   -432,  -200,  -660, 704,
    -224,  596,   -132,  268,   32,    -452,  884,   104,   -1008, 424,  -1348,
    -280,  4,     -1168, 368,   476,   696,   300,   -8,    24,    180,  -592,
    -196,  388,   304,   500,   724,   -160,  244,   -84,   272,   -256, -420,
    320,   208,   -144,  -156,  156,   364,   452,   28,    540,   316,  220,
    -644,  -248,  464,   72,    360,   32,    -388,  496,   -680,  -48,  208,
    -116,  -408,  60,    -604,  -392,  548,   -840,  784,   -460,  656,  -544,
    -388,  -264,  908,   -800,  -628,  -612,  -568,  572,   -220,  164,  288,
    -16,   -308,  308,   -112,  -636,  -760,  280,   -668,  432,   364,  240,
    -196,  604,   340,   384,   196,   592,   -44,   -500,  432,   -580, -132,
    636,   -76,   392,   4,     -412,  540,   508,   328,   -356,  -36,  16,
    -220,  -64,   -248,  -60,   24,    -192,  368,   1040,  92,    -24,  -1044,
    -32,   40,    104,   148,   192,   -136,  -520,  56,    -816,  -224, 732,
    392,   356,   212,   -80,   -424,  -1008, -324,  588,   -1496, 576,  460,
    -816,  -848,  56,    -580,  -92,   -1372, -112,  -496,  200,   364,  52,
    -140,  48,    -48,   -60,   84,    72,    40,    132,   -356,  -268, -104,
    -284,  -404,  732,   -520,  164,   -304,  -540,  120,   328,   -76,  -460,
    756,   388,   588,   236,   -436,  -72,   -176,  -404,  -316,  -148, 716,
    -604,  404,   -72,   -88,   -888,  -68,   944,   88,    -220,  -344, 960,
    472,   460,   -232,  704,   120,   832,   -228,  692,   -508,  132,  -476,
    844,   -748,  -364,  -44,   1116,  -1104, -1056, 76,    428,   552,  -692,
    60,    356,   96,    -384,  -188,  -612,  -576,  736,   508,   892,  352,
    -1132, 504,   -24,   -352,  324,   332,   -600,  -312,  292,   508,  -144,
    -8,    484,   48,    284,   -260,  -240,  256,   -100,  -292,  -204, -44,
    472,   -204,  908,   -188,  -1000, -256,  92,    1164,  -392,  564,  356,
    652,   -28,   -884,  256,   484,   -192,  760,   -176,  376,   -524, -452,
    -436,  860,   -736,  212,   124,   504,   -476,  468,   76,    -472, 552,
    -692,  -944,  -620,  740,   -240,  400,   132,   20,    192,   -196, 264,
    -668,  -1012, -60,   296,   -316,  -828,  76,    -156,  284,   -768, -448,
    -832,  148,   248,   652,   616,   1236,  288,   -328,  -400,  -124, 588,
    220,   520,   -696,  1032,  768,   -740,  -92,   -272,  296,   448,  -464,
    412,   -200,  392,   440,   -200,  264,   -152,  -260,  320,   1032, 216,
    320,   -8,    -64,   156,   -1016, 1084,  1172,  536,   484,   -432, 132,
    372,   -52,   -256,  84,    116,   -352,  48,    116,   304,   -384, 412,
    924,   -300,  528,   628,   180,   648,   44,    -980,  -220,  1320, 48,
    332,   748,   524,   -268,  -720,  540,   -276,  564,   -344,  -208, -196,
    436,   896,   88,    -392,  132,   80,    -964,  -288,  568,   56,   -48,
    -456,  888,   8,     552,   -156,  -292,  948,   288,   128,   -716, -292,
    1192,  -152,  876,   352,   -600,  -260,  -812,  -468,  -28,   -120, -32,
    -44,   1284,  496,   192,   464,   312,   -76,   -516,  -380,  -456, -1012,
    -48,   308,   -156,  36,    492,   -156,  -808,  188,   1652,  68,   -120,
    -116,  316,   160,   -140,  352,   808,   -416,  592,   316,   -480, 56,
    528,   -204,  -568,  372,   -232,  752,   -344,  744,   -4,    324,  -416,
    -600,  768,   268,   -248,  -88,   -132,  -420,  -432,  80,    -288, 404,
    -316,  -1216, -588,  520,   -108,  92,    -320,  368,   -480,  -216, -92,
    1688,  -300,  180,   1020,  -176,  820,   -68,   -228,  -260,  436,  -904,
    20,    40,    -508,  440,   -736,  312,   332,   204,   760,   -372, 728,
    96,    -20,   -632,  -520,  -560,  336,   1076,  -64,   -532,  776,  584,
    192,   396,   -728,  -520,  276,   -188,  80,    -52,   -612,  -252, -48,
    648,   212,   -688,  228,   -52,   -260,  428,   -412,  -272,  -404, 180,
    816,   -796,  48,    152,   484,   -88,   -216,  988,   696,   188,  -528,
    648,   -116,  -180,  316,   476,   12,    -564,  96,    476,   -252, -364,
    -376,  -392,  556,   -256,  -576,  260,   -352,  120,   -16,   -136, -260,
    -492,  72,    556,   660,   580,   616,   772,   436,   424,   -32,  -324,
    -1268, 416,   -324,  -80,   920,   160,   228,   724,   32,    -516, 64,
    384,   68,    -128,  136,   240,   248,   -204,  -68,   252,   -932, -120,
    -480,  -628,  -84,   192,   852,   -404,  -288,  -132,  204,   100,  168,
    -68,   -196,  -868,  460,   1080,  380,   -80,   244,   0,     484,  -888,
    64,    184,   352,   600,   460,   164,   604,   -196,  320,   -64,  588,
    -184,  228,   12,    372,   48,    -848,  -344,  224,   208,   -200, 484,
    128,   -20,   272,   -468,  -840,  384,   256,   -720,  -520,  -464, -580,
    112,   -120,  644,   -356,  -208,  -608,  -528,  704,   560,   -424, 392,
    828,   40,    84,    200,   -152,  0,     -144,  584,   280,   -120, 80,
    -556,  -972,  -196,  -472,  724,   80,    168,   -32,   88,    160,  -688,
    0,     160,   356,   372,   -776,  740,   -128,  676,   -248,  -480, 4,
    -364,  96,    544,   232,   -1032, 956,   236,   356,   20,    -40,  300,
    24,    -676,  -596,  132,   1120,  -104,  532,   -1096, 568,   648,  444,
    508,   380,   188,   -376,  -604,  1488,  424,   24,    756,   -220, -192,
    716,   120,   920,   688,   168,   44,    -460,  568,   284,   1144, 1160,
    600,   424,   888,   656,   -356,  -320,  220,   316,   -176,  -724, -188,
    -816,  -628,  -348,  -228,  -380,  1012,  -452,  -660,  736,   928,  404,
    -696,  -72,   -268,  -892,  128,   184,   -344,  -780,  360,   336,  400,
    344,   428,   548,   -112,  136,   -228,  -216,  -820,  -516,  340,  92,
    -136,  116,   -300,  376,   -244,  100,   -316,  -520,  -284,  -12,  824,
    164,   -548,  -180,  -128,  116,   -924,  -828,  268,   -368,  -580, 620,
    192,   160,   0,     -1676, 1068,  424,   -56,   -360,  468,   -156, 720,
    288,   -528,  556,   -364,  548,   -148,  504,   316,   152,   -648, -620,
    -684,  -24,   -376,  -384,  -108,  -920,  -1032, 768,   180,   -264, -508,
    -1268, -260,  -60,   300,   -240,  988,   724,   -376,  -576,  -212, -736,
    556,   192,   1092,  -620,  -880,  376,   -56,   -4,    -216,  -32,  836,
    268,   396,   1332,  864,   -600,  100,   56,    -412,  -92,   356,  180,
    884,   -468,  -436,  292,   -388,  -804,  -704,  -840,  368,   -348, 140,
    -724,  1536,  940,   372,   112,   -372,  436,   -480,  1136,  296,  -32,
    -228,  132,   -48,   -220,  868,   -1016, -60,   -1044, -464,  328,  916,
    244,   12,    -736,  -296,  360,   468,   -376,  -108,  -92,   788,  368,
    -56,   544,   400,   -672,  -420,  728,   16,    320,   44,    -284, -380,
    -796,  488,   132,   204,   -596,  -372,  88,    -152,  -908,  -636, -572,
    -624,  -116,  -692,  -200,  -56,   276,   -88,   484,   -324,  948,  864,
    1000,  -456,  -184,  -276,  292,   -296,  156,   676,   320,   160,  908,
    -84,   -1236, -288,  -116,  260,   -372,  -644,  732,   -756,  -96,  84,
    344,   -520,  348,   -688,  240,   -84,   216,   -1044, -136,  -676, -396,
    -1500, 960,   -40,   176,   168,   1516,  420,   -504,  -344,  -364, -360,
    1216,  -940,  -380,  -212,  252,   -660,  -708,  484,   -444,  -152, 928,
    -120,  1112,  476,   -260,  560,   -148,  -344,  108,   -196,  228,  -288,
    504,   560,   -328,  -88,   288,   -1008, 460,   -228,  468,   -836, -196,
    76,    388,   232,   412,   -1168, -716,  -644,  756,   -172,  -356, -504,
    116,   432,   528,   48,    476,   -168,  -608,  448,   160,   -532, -272,
    28,    -676,  -12,   828,   980,   456,   520,   104,   -104,  256,  -344,
    -4,    -28,   -368,  -52,   -524,  -572,  -556,  -200,  768,   1124, -208,
    -512,  176,   232,   248,   -148,  -888,  604,   -600,  -304,  804,  -156,
    -212,  488,   -192,  -804,  -256,  368,   -360,  -916,  -328,  228,  -240,
    -448,  -472,  856,   -556,  -364,  572,   -12,   -156,  -368,  -340, 432,
    252,   -752,  -152,  288,   268,   -580,  -848,  -592,  108,   -76,  244,
    312,   -716,  592,   -80,   436,   360,   4,     -248,  160,   516,  584,
    732,   44,    -468,  -280,  -292,  -156,  -588,  28,    308,   912,  24,
    124,   156,   180,   -252,  944,   -924,  -772,  -520,  -428,  -624, 300,
    -212,  -1144, 32,    -724,  800,   -1128, -212,  -1288, -848,  180,  -416,
    440,   192,   -576,  -792,  -76,   -1080, 80,    -532,  -352,  -132, 380,
    -820,  148,   1112,  128,   164,   456,   700,   -924,  144,   -668, -384,
    648,   -832,  508,   552,   -52,   -100,  -656,  208,   -568,  748,  -88,
    680,   232,   300,   192,   -408,  -1012, -152,  -252,  -268,  272,  -876,
    -664,  -648,  -332,  -136,  16,    12,    1152,  -28,   332,   -536, 320,
    -672,  -460,  -316,  532,   -260,  228,   -40,   1052,  -816,  180,  88,
    -496,  -556,  -672,  -368,  428,   92,    356,   404,   -408,  252,  196,
    -176,  -556,  792,   268,   32,    372,   40,    96,    -332,  328,  120,
    372,   -900,  -40,   472,   -264,  -592,  952,   128,   656,   112,  664,
    -232,  420,   4,     -344,  -464,  556,   244,   -416,  -32,   252,  0,
    -412,  188,   -696,  508,   -476,  324,   -1096, 656,   -312,  560,  264,
    -136,  304,   160,   -64,   -580,  248,   336,   -720,  560,   -348, -288,
    -276,  -196,  -500,  852,   -544,  -236,  -1128, -992,  -776,  116,  56,
    52,    860,   884,   212,   -12,   168,   1020,  512,   -552,  924,  -148,
    716,   188,   164,   -340,  -520,  -184,  880,   -152,  -680,  -208, -1156,
    -300,  -528,  -472,  364,   100,   -744,  -1056, -32,   540,   280,  144,
    -676,  -32,   -232,  -280,  -224,  96,    568,   -76,   172,   148,  148,
    104,   32,    -296,  -32,   788,   -80,   32,    -16,   280,   288,  944,
    428,   -484
};

// Binding Table Definitions
//!
//! \enum     FilmGrainGetRandomValuesBindingTableOffset
//! \brief    Binding table offset for GetRandomValues kernel
//!
enum FilmGrainGetRandomValuesBindingTableOffset
{
    grvInputGaussianSeq = 0,
    grvOutputYRandomValue,
    grvOutputURandomValue,
    grvOutputVRandomValue,
    grvOutputCoordinates,
    grvNumSurfaces
};

//!
//! \enum     FilmGrainRegressPhase1BindingTableOffset
//! \brief    Binding table offset for regressPhase1 kernel
//!
enum FilmGrainRegressPhase1BindingTableOffset
{
    rp1InputYRandomValue = 0,
    rp1OutputYDitheringSurface,
    rp1InputYCoeff,
    rp1NumSurfaces
};

//!
//! \enum     FilmGrainRegressPhase2BindingTableOffset
//! \brief    Binding table offset for regressPhase2 kernel
//!
enum FilmGrainRegressPhase2BindingTableOffset
{
    rp2InputYRandomValue = 0,
    rp2InputURandomValue,
    rp2InputVRandomValue,
    rp2InputYDithering,
    rp2OutputYDithering,
    rp2OutputUDithering,
    rp2OutputVDithering,
    rp2InputYCoeff,
    rp2InputUCoeff,
    rp2InputVCoeff,
    rp2NumSurfaces
};

//!
//! \enum     FilmGrainApplyNoiseBindingTableOffset
//! \brief    Binding table offset for ApplyNoise kernel
//!
enum FilmGrainApplyNoiseBindingTableOffset
{
    anInputYuv = 0,
    anOutputY,
    anOutputUv,
    anInputYDithering,
    anInputUDithering,
    anInputVDithering,
    anInputRandomValuesCoordinates,
    anInputYGammaLut,
    anInputUGammaLut,
    anInputVGammaLut,
    anNumSurfaces
};

const int32_t Av1DecodeFilmGrainG12::m_filmGrainBindingTableCount[kernelNum] = {
    grvNumSurfaces,
    rp1NumSurfaces,
    rp2NumSurfaces,
    anNumSurfaces
};

//Curbe definitions
struct FilmGrainGetRandomValuesCurbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   GaussianSeqSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   YRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t  URandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint32_t 3
    union
    {
        struct
        {
            uint32_t  VRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // uint32_t 4
    union
    {
        struct
        {
            uint32_t  CoordinatesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);  //Random values for coordinates surface index
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // uint32_t 5
    union
    {
        struct
        {
            uint32_t  NoiseShiftAmount  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  Reserved          : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // uint32_t 6
    union
    {
        struct
        {
            uint32_t  GrainSeed : MOS_BITFIELD_RANGE(0, 31);    //Random number generation initializer
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // uint32_t 7
    union
    {
        struct
        {
            uint32_t  CoordinatesWidth  : MOS_BITFIELD_RANGE(0, 15);   //RoundUp(ImageHeight/64)
            uint32_t  CoordinatesHeight : MOS_BITFIELD_RANGE(16, 31);  //RoundUp(ImageWidth/64)
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    static const size_t m_size = 8;
    static const size_t m_byteSize = 32;
};
C_ASSERT(sizeof(FilmGrainGetRandomValuesCurbe) == 32);

struct FilmGrainRegressPhase1Curbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   YRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   YDitheringSurface : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t  YCoeffSurface : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    static const size_t m_size = 3;
    static const size_t m_byteSize = 12;
};
C_ASSERT(sizeof(FilmGrainRegressPhase1Curbe) == 12);

struct FilmGrainRegressPhase2Curbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   YRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   URandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t  VRandomValuesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint32_t 3
    union
    {
        struct
        {
            uint32_t  YDitheringInputSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // uint32_t 4
    union
    {
        struct
        {
            uint32_t  YDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // uint32_t 5
    union
    {
        struct
        {
            uint32_t  UDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // uint32_t 6
    union
    {
        struct
        {
            uint32_t  VDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // uint32_t 7
    union
    {
        struct
        {
            uint32_t  YCoeffSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    // uint32_t 8
    union
    {
        struct
        {
            uint32_t  UCoeffSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    // uint32_t 9
    union
    {
        struct
        {
            uint32_t  VCoeffSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    // uint32_t 10
    union
    {
        struct
        {
            uint32_t  RegressionCoefficientShift    : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  Reserved                      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    static const size_t m_size = 11;
    static const size_t m_byteSize = 44;
};
C_ASSERT(sizeof(FilmGrainRegressPhase2Curbe) == 44);

struct FilmGrainApplyNoiseCurbe
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   InputYuvSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   OutputYSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t  OutputUvSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint32_t 3
    union
    {
        struct
        {
            uint32_t  YDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // uint32_t 4
    union
    {
        struct
        {
            uint32_t  UDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // uint32_t 5
    union
    {
        struct
        {
            uint32_t  VDitheringSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // uint32_t 6
    union
    {
        struct
        {
            uint32_t  RandomValuesForCoordinatesSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // uint32_t 7
    union
    {
        struct
        {
            uint32_t  YGammaCorrectionLutSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

    // uint32_t 8
    union
    {
        struct
        {
            uint32_t  UGammaCorrectionLutSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW8;

    // uint32_t 9
    union
    {
        struct
        {
            uint32_t  VGammaCorrectionLutSurfaceIndex : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;
 
    // uint32_t 10
    union
    {
        struct
        {
            uint32_t  EnableYFilmGrain                     : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  EnableUFilmGrain                     : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    // uint32_t 11
    union
    {
        struct
        {
            uint32_t  EnableVFilmGrain                      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  RandomValuesForCoordinatesTableWidth  : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

    // uint32_t 12
    union
    {
        struct
        {
            uint32_t  ImageHeight       : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  ScalingShiftValue : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW12;

    // uint32_t 13
    union
    {
        struct
        {
            uint32_t  MinimumYClippingValue : MOS_BITFIELD_RANGE(0, 15); 
            uint32_t  MaximumYClippingValue : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW13;

    // uint32_t 14
    union
    {
        struct
        {
            uint32_t  MinimumUvClippingValue : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  MaximumUvClippingValue : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW14;

    // uint32_t 15
    union
    {
        struct
        {
            uint32_t  CbLumaMultiplier : MOS_BITFIELD_RANGE(0, 15); 
            uint32_t  CbMultiplier : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    // uint32_t 16
    union
    {
        struct
        {
            uint32_t  CbOffset          : MOS_BITFIELD_RANGE(0, 15); 
            uint32_t  CrLumaMultiplier  : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW16;

    // uint32_t 17
    union
    {
        struct
        {
            uint32_t  CrMultiplier  : MOS_BITFIELD_RANGE(0, 15);
            uint32_t  CrOffset      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW17;

    static const size_t m_size = 18;
    static const size_t m_byteSize = 72;
};
C_ASSERT(sizeof(FilmGrainApplyNoiseCurbe) == 72);

const int32_t Av1DecodeFilmGrainG12::m_filmGrainCurbeSize[kernelNum] = {
    (sizeof(FilmGrainGetRandomValuesCurbe)),
    (sizeof(FilmGrainRegressPhase1Curbe)),
    (sizeof(FilmGrainRegressPhase2Curbe)),
    (sizeof(FilmGrainApplyNoiseCurbe))
};

// Initialize the static const float variables in class Av1DecodeFilmGrainG12.
const float Av1DecodeFilmGrainG12::m_maxScaleRatio = 1.0f;
const float Av1DecodeFilmGrainG12::m_minScaleRatio = 0.125f;

Av1DecodeFilmGrainG12::Av1DecodeFilmGrainG12(
    MediaFeatureManager *featureManager,
    DecodeAllocator     *allocator,
    CodechalHwInterface *hwInterface) : 
    m_allocator(allocator)
{
    m_featureManager = featureManager;

    auto decFeatureManager = dynamic_cast<DecodeAv1FeatureManagerG12 *>(featureManager);
    DECODE_CHK_NULL_NO_STATUS_RETURN(decFeatureManager);

    m_basicFeature = dynamic_cast<Av1BasicFeatureG12 *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    DECODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    m_hwInterface = hwInterface;
}

Av1DecodeFilmGrainG12::~Av1DecodeFilmGrainG12()
{
    m_allocator->Destroy(m_gaussianSequenceSurface);
    m_allocator->Destroy(m_yRandomValuesSurface);
    m_allocator->Destroy(m_uRandomValuesSurface);
    m_allocator->Destroy(m_vRandomValuesSurface);
    m_allocator->Destroy(m_yDitheringTempSurface);

    m_allocator->Destroy(m_coordinatesRandomValuesSurfaceArray);
    m_allocator->Destroy(m_yCoefficientsSurfaceArray);
    m_allocator->Destroy(m_yDitheringSurfaceArray);
    m_allocator->Destroy(m_uDitheringSurfaceArray);
    m_allocator->Destroy(m_vDitheringSurfaceArray);
    m_allocator->Destroy(m_yCoeffSurfaceArray);
    m_allocator->Destroy(m_uCoeffSurfaceArray);
    m_allocator->Destroy(m_vCoeffSurfaceArray);
    m_allocator->Destroy(m_yGammaLUTSurfaceArray);
    m_allocator->Destroy(m_uGammaLUTSurfaceArray);
    m_allocator->Destroy(m_vGammaLUTSurfaceArray);
    m_allocator->Destroy(m_coordinatesRandomValuesSurfaceArray);
}

MOS_STATUS Av1DecodeFilmGrainG12::Init(void *settings)
{
    DECODE_FUNC_CALL();

    memset(&m_kernelSize, 0, sizeof(m_kernelSize));
    memset(&m_dshSize, 0, sizeof(m_dshSize));
    memset(&m_syncObject, 0, sizeof(m_syncObject));

    for (uint8_t i = getRandomValues; i < kernelNum; i++)
    {
        m_kernelBinary[i] = nullptr;
        m_kernelStates[i] = MHW_KERNEL_STATE();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodeFilmGrainG12::Update(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;
    m_picParams = static_cast<CodecAv1PicParams *>(decodeParams->m_picParams);
    DECODE_CHK_NULL(m_picParams);

    m_bitDepthIndicator = m_basicFeature->m_av1DepthIndicator;

    if (!m_resourceAllocated)
    {
        DECODE_CHK_STATUS(InitializeKernelState());
        DECODE_CHK_STATUS(AllocateFixedSizeSurfaces());
        m_resourceAllocated = true;
    }

    /*Note: for scenario that m_applyGrain=true but (applyY | applyCb | applyCr)=false,
     * umd no need to generate noise for perf optimization, but need to apply noise with default value directly*/
    bool applyY  = (m_picParams->m_filmGrainParams.m_numYPoints > 0) ? 1 : 0;
    bool applyCb = (m_picParams->m_filmGrainParams.m_numCbPoints > 0 || m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma) ? 1 : 0;
    bool applyCr = (m_picParams->m_filmGrainParams.m_numCrPoints > 0 || m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma) ? 1 : 0;
    m_filmGrainEnabled = m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain && (applyY | applyCb | applyCr);

    if (m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_applyGrain)
    {
        m_av1TileParams = static_cast<CodecAv1TileParams*>(decodeParams->m_sliceParams);
        DECODE_CHK_NULL(m_av1TileParams);

        m_segmentParams = &m_picParams->m_av1SegData;
        DECODE_CHK_NULL(m_segmentParams);

        DECODE_CHK_STATUS(SetFrameStates(m_picParams));
        DECODE_CHK_STATUS(AllocateVariableSizeSurfaces());
    }

#if (_DEBUG || _RELEASE_INTERNAL)
    m_fgOutputSurfList[m_basicFeature->m_curRenderPic.FrameIdx] = m_basicFeature->m_fgOutputSurf;
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodeFilmGrainG12::InitInterfaceStateHeapSetting()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    DECODE_CHK_STATUS(m_hwInterface->GetFilmGrainKernelInfo(
        m_kernelBaseCommon,
        m_combinedKernelSize));
    DECODE_CHK_NULL(m_kernelBaseCommon);

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::AllocateStateHeap(
    CodechalHwInterface               *hwInterface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    MhwRenderInterface *renderInterface = m_hwInterface->GetRenderInterface();
    DECODE_CHK_NULL(renderInterface);

    MHW_STATE_HEAP_SETTINGS *stateHeapSettings = m_hwInterface->GetStateHeapSettings();
    DECODE_CHK_NULL(stateHeapSettings);

    stateHeapSettings->m_ishBehavior = HeapManager::Behavior::clientControlled;
    stateHeapSettings->m_dshBehavior = HeapManager::Behavior::destructiveExtend;
    // As a performance optimization keep the DSH locked always,
    // the ISH is only accessed at device creation and thus does not need to be locked
    stateHeapSettings->m_keepDshLocked = true;
    stateHeapSettings->dwDshIncrement = 2 * MOS_PAGE_SIZE;

    if (stateHeapSettings->dwIshSize > 0 &&
        stateHeapSettings->dwDshSize > 0 &&
        stateHeapSettings->dwNumSyncTags > 0)
    {
        DECODE_CHK_STATUS(renderInterface->AllocateHeaps(
            *stateHeapSettings));
    }

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::GetCommonKernelHeaderAndSize(
    void                            *binary,
    FilmGrainKernelStateIdx         index,
    uint8_t                         bitDepthIndicator,
    void                            *krnHeader,
    uint32_t                        *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(binary);
    DECODE_CHK_NULL(krnHeader);
    DECODE_CHK_NULL(krnSize);

    FilmGrainCombinedKernelHeader *kernelHeaderTable;
    kernelHeaderTable = (FilmGrainCombinedKernelHeader*)binary;
    CodecKernelHeader *invalidEntry;
    invalidEntry = &(kernelHeaderTable->applyNoise10b) + 1;

    CodecKernelHeader *currKrnHeader;
    switch (index)
    {
    case getRandomValues:
        currKrnHeader = &kernelHeaderTable->getRandomValues8b;
        break;
    case regressPhase1:
        currKrnHeader = &kernelHeaderTable->regressPhase1;
        break;
    case regressPhase2:
        currKrnHeader = &kernelHeaderTable->regressPhase2For8b;
        break;
    case applyNoise:
        currKrnHeader = &kernelHeaderTable->applyNoise8b;
        break;
    default:
        DECODE_VERBOSEMESSAGE("Unsupported film grain stage requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (index != regressPhase1)
    {
        currKrnHeader += bitDepthIndicator;
    }

    *((CodecKernelHeader *)krnHeader) = *currKrnHeader;

    CodecKernelHeader *nextKrnHeader;
    nextKrnHeader = (currKrnHeader + 1);
    uint32_t nextKrnOffset;
    nextKrnOffset = *krnSize;

    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::InitializeKernelState()
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    if (m_bitDepthIndicator > 1)
    {
        DECODE_VERBOSEMESSAGE("Bit depth not supported!\n");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    PMOS_INTERFACE osInterface = m_hwInterface->GetOsInterface();
    m_osInterface           = osInterface;

    // Init State Heap
    DECODE_CHK_STATUS(InitInterfaceStateHeapSetting());

    CODECHAL_KERNEL_HEADER currKrnHeader;
    MHW_KERNEL_STATE *kernelState;
    for (auto krnStateIdx = 0; krnStateIdx < kernelNum; krnStateIdx++)
    {
        uint32_t kernelSize = m_combinedKernelSize;
        kernelState = &m_kernelStates[krnStateIdx];

        DECODE_CHK_STATUS(GetCommonKernelHeaderAndSize(
            m_kernelBaseCommon,
            (FilmGrainKernelStateIdx)krnStateIdx,
            m_bitDepthIndicator,
            &currKrnHeader,
            &kernelSize))

        kernelState->KernelParams.iBTCount          = m_filmGrainBindingTableCount[krnStateIdx];
        kernelState->KernelParams.iCurbeLength      = m_filmGrainCurbeSize[krnStateIdx];
        kernelState->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
        kernelState->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
        kernelState->KernelParams.iIdCount          = 1;
        kernelState->dwKernelBinaryOffset           = 0; 

        kernelState->KernelParams.pBinary   = m_kernelBaseCommon + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelState->KernelParams.iSize     = kernelSize;
    }

    DECODE_CHK_STATUS(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_syncObject));

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::SetupMediaVfe(
    PMOS_COMMAND_BUFFER  cmdBuffer,
    MHW_KERNEL_STATE     *kernelState)
{
    MHW_VFE_PARAMS_G12 vfeParams = {};
    vfeParams.pKernelState = kernelState;

    DECODE_CHK_STATUS(m_renderInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1DecodeFilmGrainG12::AllocateFixedSizeSurfaces()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    //Gaussian sequence surface
    m_gaussianSequenceSurface = m_allocator->AllocateBuffer(
        MOS_ALIGN_CEIL(2048 * sizeof(int16_t), CODECHAL_PAGE_SIZE), "GaussianSequenceSurface",
        resourceInternalReadWriteCache, lockableVideoMem);
    DECODE_CHK_NULL(m_gaussianSequenceSurface);

    auto data = (int16_t *)m_allocator->LockResourceForWrite(&m_gaussianSequenceSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 2048 * sizeof(int16_t), defaultGaussianSequence, 2048 * sizeof(int16_t));

    // Surfaces/buffers for GetRandomValues kernel
    //Y random values surface
    PMOS_SURFACE surface = nullptr;
    m_yRandomValuesSurface = m_allocator->AllocateSurface(
        70 * sizeof(int16_t),
        70,
        "Film Grain GRV [out] YRandomValuesSurface",
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_yRandomValuesSurface);

    //U random values surface
    m_uRandomValuesSurface = m_allocator->AllocateSurface(
        38 * sizeof(int16_t),
        38,
        "Film Grain GRV [out] URandomValuesSurface",
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_uRandomValuesSurface);

    //V random values surface
    m_vRandomValuesSurface = m_allocator->AllocateSurface(
        38 * sizeof(int16_t),
        38,
        "Film Grain GRV [out] VRandomValuesSurface",
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_vRandomValuesSurface);

    //Y Dithering Temp LUT Surface
    m_yDitheringTempSurface = m_allocator->AllocateSurface(
        70 * sizeof(int32_t),
        70,
        "Film Grain RP1 [out] YDitheringTempSurface",
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_yDitheringTempSurface);

    // Surfaces/buffers for RegressPhase1 kernel
    //Y Coefficients Surface
    m_yCoefficientsSurfaceArray = m_allocator->AllocateBufferArray(
        24 * sizeof(int16_t),
        "YCoeffSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_yCoefficientsSurfaceArray);

    //Y dithering Surface
    m_yDitheringSurfaceArray = m_allocator->AllocateSurfaceArray(
        128 * ((m_bitDepthIndicator == 1) ? sizeof(int16_t) : sizeof(int8_t)),
        128,
        "Film Grain RP2 [out] YDitheringSurface",
        m_bufferPoolDepth,
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_yDitheringSurfaceArray);

    //U dithering surface
    m_uDitheringSurfaceArray = m_allocator->AllocateSurfaceArray(
        64 * ((m_bitDepthIndicator == 1) ? sizeof(int16_t) : sizeof(int8_t)),
        64,
        "Film Grain RP2 [out] UDitheringSurface",
        m_bufferPoolDepth,
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_uDitheringSurfaceArray);

    //V Dithering surface
    m_vDitheringSurfaceArray = m_allocator->AllocateSurfaceArray(
        64 * ((m_bitDepthIndicator == 1) ? sizeof(int16_t) : sizeof(int8_t)),
        64,
        "Film Grain RP2 [out] VDitheringSurface",
        m_bufferPoolDepth,
        Format_R8UN,
        false,
        resourceInternalReadWriteCache,
        notLockableVideoMem);
    DECODE_CHK_NULL(m_vDitheringSurfaceArray);

    //Y Coefficients Surface, for input of RegressPhase2
    m_yCoeffSurfaceArray = m_allocator->AllocateBufferArray(
        MOS_ALIGN_CEIL(32 * sizeof(int16_t), CODECHAL_PAGE_SIZE),
        "YCoeffSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_yCoeffSurfaceArray);

    //U Coefficients Surface, for input of RegressPhase2
    m_uCoeffSurfaceArray = m_allocator->AllocateBufferArray(
        MOS_ALIGN_CEIL(32 * sizeof(int16_t), CODECHAL_PAGE_SIZE),
        "UCoeffSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_uCoeffSurfaceArray);

    //V Coefficients Surface, for input of RegressPhase2
    m_vCoeffSurfaceArray = m_allocator->AllocateBufferArray(
        MOS_ALIGN_CEIL(32 * sizeof(int16_t), CODECHAL_PAGE_SIZE),
        "VCoeffSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_vCoeffSurfaceArray);

    //Y Gamma LUT Surface, for input of ApplyNoise
    m_yGammaLUTSurfaceArray = m_allocator->AllocateBufferArray(
        MOS_ALIGN_CEIL(257 * sizeof(int16_t), CODECHAL_PAGE_SIZE),
        "YGammaLUTSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_yGammaLUTSurfaceArray);

    //U Gamma LUT Surface, for input of ApplyNoise
    m_uGammaLUTSurfaceArray = m_allocator->AllocateBufferArray(
        MOS_ALIGN_CEIL(257 * sizeof(int16_t), CODECHAL_PAGE_SIZE),
        "UGammaLUTSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_uGammaLUTSurfaceArray);

    //V Gamma LUT Surface, for input of ApplyNoise
    m_vGammaLUTSurfaceArray = m_allocator->AllocateBufferArray(
        MOS_ALIGN_CEIL(257 * sizeof(int16_t), CODECHAL_PAGE_SIZE),
        "VGammaLUTSurface",
        m_bufferPoolDepth,
        resourceInternalReadWriteCache,
        lockableVideoMem);
    DECODE_CHK_NULL(m_vGammaLUTSurfaceArray);

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::AllocateVariableSizeSurfaces()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    //Random values for coordinates
    uint32_t coordsWidth = MOS_ROUNDUP_SHIFT(m_picParams->m_superResUpscaledWidthMinus1 + 1, 6);
    uint32_t coordsHeight = MOS_ROUNDUP_SHIFT(m_picParams->m_superResUpscaledHeightMinus1 + 1, 6);
    uint32_t allocSize = MOS_ALIGN_CEIL(coordsWidth * coordsHeight * sizeof(int32_t), CODECHAL_PAGE_SIZE);

    if (m_coordinatesRandomValuesSurfaceArray == nullptr)
    {
        m_coordinatesRandomValuesSurfaceArray= m_allocator->AllocateBufferArray(
            allocSize,
            "FilmGrainGRVCoordinateSurface",
            m_bufferPoolDepth,
            resourceInternalReadWriteCache,
            notLockableVideoMem);
        DECODE_CHK_NULL(m_coordinatesRandomValuesSurfaceArray);
        m_coordinatesRandomValuesSurface = m_coordinatesRandomValuesSurfaceArray->Fetch();
        DECODE_CHK_NULL(m_coordinatesRandomValuesSurface);
    }
    else
    {
        auto &buffer = m_coordinatesRandomValuesSurfaceArray->Fetch();
        DECODE_CHK_NULL(buffer);
        DECODE_CHK_STATUS(m_allocator->Resize(
            buffer, allocSize, notLockableVideoMem));
        m_coordinatesRandomValuesSurface = buffer;
    }
    m_coordinateSurfaceSize = allocSize;

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::InitScalingFunction(
    uint8_t *pointValue,    //corresponds to scaling_points[][0]
    uint8_t *pointScaling,  //corresponds to scaling_points[][1]
    uint8_t numPoints,
    int16_t *scalingLUT)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    DECODE_CHK_NULL(pointValue);
    DECODE_CHK_NULL(pointScaling);
    DECODE_CHK_NULL(scalingLUT);

    if (numPoints == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    for (auto i = 0; i < pointValue[0]; i++)
    {
        scalingLUT[i] = pointScaling[0];
    }

    for (auto point = 0; point < numPoints - 1; point++)
    {
        int32_t delta_y = pointScaling[point + 1] - pointScaling[point];
        int32_t delta_x = pointValue[point + 1] - pointValue[point];

        DECODE_CHK_COND(delta_x == 0, " Value of delta x cannot be zero.");
        int64_t delta = (int64_t)delta_y * ((65536 + (delta_x >> 1)) / delta_x);

        for (auto x = 0; x < delta_x; x++)
        {
            scalingLUT[pointValue[point] + x] = pointScaling[point] + (int32_t)((x * delta + 32768) >> 16);
        }
    }

    for (uint32_t i = pointValue[numPoints - 1]; i < 256; i++)
    {
        scalingLUT[i] = pointScaling[numPoints - 1];
    }

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::PreProcScalingPointsAndLUTs()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    memset(m_scalingLutY, 0, sizeof(*m_scalingLutY) * 256);
    memset(m_scalingLutCb, 0, sizeof(*m_scalingLutCb) * 256);
    memset(m_scalingLutCr, 0, sizeof(*m_scalingLutCr) * 256);

    // Check film grain parameter of the luma component
    if (m_picParams->m_filmGrainParams.m_numYPoints > 14)
    {
        DECODE_ASSERTMESSAGE("Invalid film grain num_y_points (should be in [0, 14]) in pic parameter!");
        return MOS_STATUS_INVALID_PARAMETER;
    }
    for (auto i = 1; i < m_picParams->m_filmGrainParams.m_numYPoints; i++)
    {
        if (m_picParams->m_filmGrainParams.m_pointYValue[i] <= m_picParams->m_filmGrainParams.m_pointYValue[i - 1])
        {
            DECODE_ASSERTMESSAGE("Invalid film grain point_y_value (point_y_value[%d] should be greater than point_y_value[%d]) in pic parameter!", i, i - 1);
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    DECODE_CHK_STATUS(InitScalingFunction(
        m_picParams->m_filmGrainParams.m_pointYValue,
        m_picParams->m_filmGrainParams.m_pointYScaling,
        m_picParams->m_filmGrainParams.m_numYPoints,
        m_scalingLutY));

    if (m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_chromaScalingFromLuma)
    {
        MOS_SecureMemcpy(m_scalingLutCb, sizeof(int16_t) * 256, m_scalingLutY, sizeof(int16_t) * 256);
        MOS_SecureMemcpy(m_scalingLutCr, sizeof(int16_t) * 256, m_scalingLutY, sizeof(int16_t) * 256);
    }
    else
    {
        // Check film grain parameter of the cb component
        if (m_picParams->m_filmGrainParams.m_numCbPoints > 10)
        {
            DECODE_ASSERTMESSAGE("Invalid film grain num_cb_points (should be in [0, 10]) in pic parameter!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        for (auto i = 1; i < m_picParams->m_filmGrainParams.m_numCbPoints; i++)
        {
            if (m_picParams->m_filmGrainParams.m_pointCbValue[i] <= m_picParams->m_filmGrainParams.m_pointCbValue[i - 1])
            {
                DECODE_ASSERTMESSAGE("Invalid film grain point_cb_value (point_cb_value[%d] should be greater than point_cb_value[%d]) in pic parameter!", i, i - 1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        DECODE_CHK_STATUS(InitScalingFunction(
            m_picParams->m_filmGrainParams.m_pointCbValue,
            m_picParams->m_filmGrainParams.m_pointCbScaling,
            m_picParams->m_filmGrainParams.m_numCbPoints,
            m_scalingLutCb));

        // Check film grain parameter of the cr component
        if (m_picParams->m_filmGrainParams.m_numCrPoints > 10)
        {
            DECODE_ASSERTMESSAGE("Invalid film grain num_cr_points (should be in [0, 10]) in pic parameter!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        for (auto i = 1; i < m_picParams->m_filmGrainParams.m_numCrPoints; i++)
        {
            if (m_picParams->m_filmGrainParams.m_pointCrValue[i] <= m_picParams->m_filmGrainParams.m_pointCrValue[i - 1])
            {
                DECODE_ASSERTMESSAGE("Invalid film grain point_cr_value (point_cr_value[%d] should be greater than point_cr_value[%d]) in pic parameter!", i, i - 1);
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        DECODE_CHK_STATUS(InitScalingFunction(
            m_picParams->m_filmGrainParams.m_pointCrValue,
            m_picParams->m_filmGrainParams.m_pointCrScaling,
            m_picParams->m_filmGrainParams.m_numCrPoints,
            m_scalingLutCr));
    }

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::PreProcArCoeffs(
    int16_t *yCoeff, 
    int16_t *uCoeff, 
    int16_t *vCoeff)
{

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    DECODE_FUNC_CALL();

    uint32_t arCoeffLag = m_picParams->m_filmGrainParams.m_filmGrainInfoFlags.m_fields.m_arCoeffLag;
    if (arCoeffLag == 3)
    {
        for (auto i = 0; i < 24; i++)
        {
            yCoeff[i] = m_picParams->m_filmGrainParams.m_arCoeffsY[i];
        }
        for (auto i = 0; i < 25; i++)
        {
            uCoeff[i] = m_picParams->m_filmGrainParams.m_arCoeffsCb[i];
        }
        for (auto i = 0; i < 25; i++)
        {
            vCoeff[i] = m_picParams->m_filmGrainParams.m_arCoeffsCr[i];
        }

        return MOS_STATUS_SUCCESS;
    }

    memset(yCoeff, 0, 24 * sizeof(int16_t));
    memset(uCoeff, 0, 25 * sizeof(int16_t));
    memset(vCoeff, 0, 25 * sizeof(int16_t));

    uint32_t mappedIdxLag2[13]  = { 8, 9, 10, 11, 12, 15, 16, 17, 18, 19, 22, 23, 24 };
    uint32_t mappedIdxLag1[5]   = { 16, 17, 18, 23, 24 };
    uint32_t mappedIdxLag0[1]   = { 24 };

    uint32_t numPosLuma = 2 * arCoeffLag * (arCoeffLag + 1);
    uint32_t numPosChroma = numPosLuma;
    if (m_picParams->m_filmGrainParams.m_numYPoints > 0)
    {
        ++numPosChroma;
    }

    uint32_t *mappedIdx = nullptr; mappedIdxLag2;
    if (arCoeffLag == 2)
    {
        mappedIdx = mappedIdxLag2;
    }
    else if (arCoeffLag == 1)
    {
        mappedIdx = mappedIdxLag1;
    }
    else if (arCoeffLag == 0)
    {
        mappedIdx = mappedIdxLag0;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < numPosLuma; i++)
    {
        yCoeff[mappedIdx[i]] = m_picParams->m_filmGrainParams.m_arCoeffsY[i];
    }
    for (uint32_t i = 0; i < numPosChroma; i++)
    {
        uCoeff[mappedIdx[i]] = m_picParams->m_filmGrainParams.m_arCoeffsCb[i];
        vCoeff[mappedIdx[i]] = m_picParams->m_filmGrainParams.m_arCoeffsCr[i];
    }

    return eStatus;
}

MOS_STATUS Av1DecodeFilmGrainG12::SetFrameStates(
    CodecAv1PicParams *picParams)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    DECODE_CHK_NULL(picParams);

    DECODE_FUNC_CALL();

    // Picture parameters from decoder
    m_picParams = picParams;

    //Pre-process Scaling Points related params and calculate scaling LUTs
    DECODE_CHK_STATUS(PreProcScalingPointsAndLUTs());

    // Initialize surfaces
    int16_t coeffY[24], coeffU[25], coeffV[25];
    DECODE_CHK_STATUS(PreProcArCoeffs(coeffY,coeffU, coeffV));

    // Y coefficients surface as input of RegressPhase1
    m_yCoefficientsSurface = m_yCoefficientsSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_yCoefficientsSurface);
    auto data = (int16_t *)m_allocator->LockResourceForWrite(&m_yCoefficientsSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 24 * sizeof(int16_t), coeffY, 24 * sizeof(int16_t));

    //Y/U/V dithering surfaces as out of RegressPhase2
    m_yDitheringSurface                = m_yDitheringSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_yDitheringSurface);
    m_uDitheringSurface                = m_uDitheringSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_uDitheringSurface);
    m_vDitheringSurface                = m_vDitheringSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_vDitheringSurface);

    //Y/U/V coefficients surfaces as input of RegressPhase2
    m_yCoeffSurface = m_yCoeffSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_yCoeffSurface);
    data = (int16_t *)m_allocator->LockResourceForWrite(&m_yCoeffSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 24 * sizeof(int16_t), coeffY, 24 * sizeof(int16_t));

    m_uCoeffSurface = m_uCoeffSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_uCoeffSurface);
    data = (int16_t *)m_allocator->LockResourceForWrite(&m_uCoeffSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 25 * sizeof(int16_t), coeffU, 25 * sizeof(int16_t));

    m_vCoeffSurface = m_vCoeffSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_vCoeffSurface);
    data = (int16_t *)m_allocator->LockResourceForWrite(&m_vCoeffSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 25 * sizeof(int16_t), coeffV, 25 * sizeof(int16_t));

    // Scaling LUTs surfaces
    m_yGammaLUTSurface = m_yGammaLUTSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_yGammaLUTSurface);
    data = (int16_t *)m_allocator->LockResourceForWrite(&m_yGammaLUTSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 256 * sizeof(int16_t), m_scalingLutY, 256 * sizeof(int16_t));
    data[256] = m_scalingLutY[255];

    m_uGammaLUTSurface = m_uGammaLUTSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_uGammaLUTSurface);
    data = (int16_t *)m_allocator->LockResourceForWrite(&m_uGammaLUTSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 256 * sizeof(int16_t), m_scalingLutCb, 256 * sizeof(int16_t));
    data[256] = m_scalingLutCb[255];

    m_vGammaLUTSurface = m_vGammaLUTSurfaceArray->Fetch();
    DECODE_CHK_NULL(m_vGammaLUTSurface);
    data = (int16_t *)m_allocator->LockResourceForWrite(&m_vGammaLUTSurface->OsResource);
    DECODE_CHK_NULL(data);
    MOS_SecureMemcpy(data, 256 * sizeof(int16_t), m_scalingLutCr, 256 * sizeof(int16_t));
    data[256] = m_scalingLutCr[255];

    return eStatus;
}

}  // namespace Decode
