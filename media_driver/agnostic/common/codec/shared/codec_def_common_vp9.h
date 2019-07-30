/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codec_def_common_vp9.h
//! \brief    Defines decode VP9 types and macros shared by CodecHal, MHW, and DDI layer
//! \details  Applies to VP9 codec  only. Should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_VP9_H__
#define __CODEC_DEF_COMMON_VP9_H__

#include "mos_os.h"

#define CODEC_VP9_SUPER_BLOCK_WIDTH          64
#define CODEC_VP9_SUPER_BLOCK_HEIGHT         64
#define CODEC_VP9_MIN_BLOCK_WIDTH            8
#define CODEC_VP9_MIN_BLOCK_HEIGHT           8

#define CODEC_VP9_BLOCK_TYPES                 2 //Outside dimension.  0 = Y with DC, 1 = UV
#define CODEC_VP9_REF_TYPES                   2 // intra=0, inter=1
#define CODEC_VP9_COEF_BANDS                  6 //Middle dimension reflects the coefficient position within the transform.
#define CODEC_VP9_UNCONSTRAINED_NODES         3
/* Inside dimension is measure of nearby complexity, that reflects the energy
of nearby coefficients are nonzero.  For the first coefficient (DC, unless
block type is 0), we look at the (already encoded) blocks above and to the
left of the current block.  The context index is then the number (0,1,or 2)
of these blocks having nonzero coefficients.
After decoding a coefficient, the measure is determined by the size of the
most recently decoded coefficient.
Note that the intuitive meaning of this measure changes as coefficients
are decoded, e.g., prior to the first token, a zero means that my neighbors
are empty while, after the first token, because of the use of end-of-block,
a zero means we just decoded a zero and hence guarantees that a non-zero
coefficient will appear later in this block.  However, this shift
in meaning is perfectly OK because our context depends also on the
coefficient band (and since zigzag positions 0, 1, and 2 are in
distinct bands). */
#define CODEC_VP9_PREV_COEF_CONTEXTS          6
#define CODEC_VP9_MBSKIP_CONTEXTS             3
#define CODEC_VP9_INTER_MODE_CONTEXTS         7
#define CODEC_VP9_INTER_MODES                 4 //NEAREST_MV, NEAR_MV, ZERO_MV, NEW_MV
#define CODEC_VP9_SWITCHABLE_FILTERS          3   // number of switchable filters
#define CODEC_VP9_INTRA_INTER_CONTEXTS        4
#define CODEC_VP9_COMP_INTER_CONTEXTS         5
#define CODEC_VP9_REF_CONTEXTS                5
#define CODEC_VP9_BLOCK_SIZE_GROUPS           4
#define CODEC_VP9_INTRA_MODES                 10 //DC_PRED,V_PRED,H_PRED,D45_PRED,D135_PRED,D117_PRED,D153_PRED,D207_PRED,D63_PRED,TM_PRED
#define CODEC_VP9_PARTITION_PLOFFSET          4  // number of probability models per block size
#define CODECHAL_VP9_PARTITION_CONTEXTS          (4 * CODEC_VP9_PARTITION_PLOFFSET)
#define CODEC_VP9_MV_JOINTS                   4
#define CODEC_VP9_MV_CLASSES                  11
#define CODEC_VP9_CLASS0_BITS                 1  /* bits at integer precision for class 0 */
#define CODECHAL_VP9_CLASS0_SIZE                 (1 << CODEC_VP9_CLASS0_BITS)
#define CODECHAL_VP9_MV_OFFSET_BITS              (CODEC_VP9_MV_CLASSES + CODEC_VP9_CLASS0_BITS - 2)
#define CODEC_VP9_MV_FP_SIZE                  4
#define CODEC_VP9_NUM_REF_FRAMES_LOG2         3
#define CODEC_VP9_NUM_REF_FRAMES              (1 << CODEC_VP9_NUM_REF_FRAMES_LOG2)
#define CODECHAL_VP9_NUM_DPB_BUFFERS             (CODEC_VP9_NUM_REF_FRAMES + 4)
#define CODEC_VP9_NUM_CONTEXTS                4
#define CODEC_VP9_MAX_REF_LF_DELTAS           4
#define CODEC_VP9_MAX_MODE_LF_DELTAS          2
#define CODECHAL_VP9_SEG_TREE_PROBS              (CODEC_VP9_MAX_SEGMENTS - 1)
#define CODEC_VP9_PREDICTION_PROBS            3
#define CODEC_VP9_MAX_LOOP_FILTER             63

#define CODEC_VP9_MAX_QP                      255
#define CODEC_VP9_QINDEX_RANGE                (CODEC_VP9_MAX_QP + 1)
#define CODEC_VP9_MAX_SEGMENTS                8

//VP9 Profile
typedef enum {
    CODEC_PROFILE_VP9_PROFILE0 = 0,
    CODEC_PROFILE_VP9_PROFILE1 = 1,
    CODEC_PROFILE_VP9_PROFILE2 = 2,
    CODEC_PROFILE_VP9_PROFILE3 = 3
}CODEC_VP9_PROFILE_IDC;

typedef enum {
    CODEC_VP9_KEY_FRAME = 0,
    CODEC_VP9_INTER_FRAME = 1,
    CODEC_VP9_FRAME_TYPES,
} CODEC_VP9_FRAME_TYPE;

// block transform size
typedef enum {
    CODEC_VP9_TX_4X4 = 0,                      // 4x4 transform
    CODEC_VP9_TX_8X8 = 1,                      // 8x8 transform
    CODEC_VP9_TX_16X16 = 2,                    // 16x16 transform
    CODEC_VP9_TX_32X32 = 3,                    // 32x32 transform
    CODEC_VP9_TX_SELECTABLE = 4,               // selectable transform
    CODEC_VP9_TX_SIZES = 4,
} CODEC_VP9_TX_SIZE;

typedef enum CODEC_VP9_PARTITION_TYPE {
    CODEC_VP9_PARTITION_NONE,
    CODEC_VP9_PARTITION_HORZ,
    CODEC_VP9_PARTITION_VERT,
    CODEC_VP9_PARTITION_SPLIT,
    CODEC_VP9_PARTITION_TYPES,
    CODECHAL_VP9_PARTITION_INVALID = CODEC_VP9_PARTITION_TYPES
} CODEC_VP9_PARTITION_TYPE;

typedef uint8_t CODEC_VP9_COEFF_PROBS_MODEL[CODEC_VP9_REF_TYPES][CODEC_VP9_COEF_BANDS]
    [CODEC_VP9_PREV_COEF_CONTEXTS]
    [CODEC_VP9_UNCONSTRAINED_NODES];

static const uint16_t CODECHAL_VP9_QUANT_DC[CODEC_VP9_QINDEX_RANGE] = {
      4,       8,    8,    9,   10,   11,   12,   12,
      13,     14,   15,   16,   17,   18,   19,   19,
      20,     21,   22,   23,   24,   25,   26,   26,
      27,     28,   29,   30,   31,   32,   32,   33,
      34,     35,   36,   37,   38,   38,   39,   40,
      41,     42,   43,   43,   44,   45,   46,   47,
      48,     48,   49,   50,   51,   52,   53,   53,
      54,     55,   56,   57,   57,   58,   59,   60,
      61,     62,   62,   63,   64,   65,   66,   66,
      67,     68,   69,   70,   70,   71,   72,   73,
      74,     74,   75,   76,   77,   78,   78,   79,
      80,     81,   81,   82,   83,   84,   85,   85,
      87,     88,   90,   92,   93,   95,   96,   98,
      99,    101,  102,  104,  105,  107,  108,  110,
      111,   113,  114,  116,  117,  118,  120,  121,
      123,   125,  127,  129,  131,  134,  136,  138,
      140,   142,  144,  146,  148,  150,  152,  154,
      156,   158,  161,  164,  166,  169,  172,  174,
      177,   180,  182,  185,  187,  190,  192,  195,
      199,   202,  205,  208,  211,  214,  217,  220,
      223,   226,  230,  233,  237,  240,  243,  247,
      250,   253,  257,  261,  265,  269,  272,  276,
      280,   284,  288,  292,  296,  300,  304,  309,
      313,   317,  322,  326,  330,  335,  340,  344,
      349,   354,  359,  364,  369,  374,  379,  384,
      389,   395,  400,  406,  411,  417,  423,  429,
      435,   441,  447,  454,  461,  467,  475,  482,
      489,   497,  505,  513,  522,  530,  539,  549,
      559,   569,  579,  590,  602,  614,  626,  640,
      654,   668,  684,  700,  717,  736,  755,  775,
      796,   819,  843,  869,  896,  925,  955,  988,
      1022, 1058, 1098, 1139, 1184, 1232, 1282, 1336,
};

static const uint16_t CODECHAL_VP9_QUANT_AC[CODEC_VP9_QINDEX_RANGE] = {
      4,       8,    9,   10,   11,   12,   13,   14,
      15,     16,   17,   18,   19,   20,   21,   22,
      23,     24,   25,   26,   27,   28,   29,   30,
      31,     32,   33,   34,   35,   36,   37,   38,
      39,     40,   41,   42,   43,   44,   45,   46,
      47,     48,   49,   50,   51,   52,   53,   54,
      55,     56,   57,   58,   59,   60,   61,   62,
      63,     64,   65,   66,   67,   68,   69,   70,
      71,     72,   73,   74,   75,   76,   77,   78,
      79,     80,   81,   82,   83,   84,   85,   86,
      87,     88,   89,   90,   91,   92,   93,   94,
      95,     96,   97,   98,   99,  100,  101,  102,
      104,   106,  108,  110,  112,  114,  116,  118,
      120,   122,  124,  126,  128,  130,  132,  134,
      136,   138,  140,  142,  144,  146,  148,  150,
      152,   155,  158,  161,  164,  167,  170,  173,
      176,   179,  182,  185,  188,  191,  194,  197,
      200,   203,  207,  211,  215,  219,  223,  227,
      231,   235,  239,  243,  247,  251,  255,  260,
      265,   270,  275,  280,  285,  290,  295,  300,
      305,   311,  317,  323,  329,  335,  341,  347,
      353,   359,  366,  373,  380,  387,  394,  401,
      408,   416,  424,  432,  440,  448,  456,  465,
      474,   483,  492,  501,  510,  520,  530,  540,
      550,   560,  571,  582,  593,  604,  615,  627,
      639,   651,  663,  676,  689,  702,  715,  729,
      743,   757,  771,  786,  801,  816,  832,  848,
      864,   881,  898,  915,  933,  951,  969,  988,
      1007, 1026, 1046, 1066, 1087, 1108, 1129, 1151,
      1173, 1196, 1219, 1243, 1267, 1292, 1317, 1343,
      1369, 1396, 1423, 1451, 1479, 1508, 1537, 1567,
      1597, 1628, 1660, 1692, 1725, 1759, 1793, 1828,
};

static const uint16_t CODECHAL_VP9_QUANT_DC_10[CODEC_VP9_QINDEX_RANGE] = {
      4,    9,    10,   13,   15,   17,   20,   22,
      25,   28,   31,   34,   37,   40,   43,   47,
      50,   53,   57,   60,   64,   68,   71,   75,
      78,   82,   86,   90,   93,   97,   101,  105,
      109,  113,  116,  120,  124,  128,  132,  136,
      140,  143,  147,  151,  155,  159,  163,  166,
      170,  174,  178,  182,  185,  189,  193,  197,
      200,  204,  208,  212,  215,  219,  223,  226,
      230,  233,  237,  241,  244,  248,  251,  255,
      259,  262,  266,  269,  273,  276,  280,  283,
      287,  290,  293,  297,  300,  304,  307,  310,
      314,  317,  321,  324,  327,  331,  334,  337,
      343,  350,  356,  362,  369,  375,  381,  387,
      394,  400,  406,  412,  418,  424,  430,  436,
      442,  448,  454,  460,  466,  472,  478,  484,
      490,  499,  507,  516,  525,  533,  542,  550,
      559,  567,  576,  584,  592,  601,  609,  617,
      625,  634,  644,  655,  666,  676,  687,  698,
      708,  718,  729,  739,  749,  759,  770,  782,
      795,  807,  819,  831,  844,  856,  868,  880,
      891,  906,  920,  933,  947,  961,  975,  988,
      1001, 1015, 1030, 1045, 1061, 1076, 1090, 1105,
      1120, 1137, 1153, 1170, 1186, 1202, 1218, 1236,
      1253, 1271, 1288, 1306, 1323, 1342, 1361, 1379,
      1398, 1416, 1436, 1456, 1476, 1496, 1516, 1537,
      1559, 1580, 1601, 1624, 1647, 1670, 1692, 1717,
      1741, 1766, 1791, 1817, 1844, 1871, 1900, 1929,
      1958, 1990, 2021, 2054, 2088, 2123, 2159, 2197,
      2236, 2276, 2319, 2363, 2410, 2458, 2508, 2561,
      2616, 2675, 2737, 2802, 2871, 2944, 3020, 3102,
      3188, 3280, 3375, 3478, 3586, 3702, 3823, 3953,
      4089, 4236, 4394, 4559, 4737, 4929, 5130, 5347,
};

static const uint16_t CODECHAL_VP9_QUANT_AC_10[CODEC_VP9_QINDEX_RANGE] = {
      4,    9,    11,   13,   16,   18,   21,   24,
      27,   30,   33,   37,   40,   44,   48,   51,
      55,   59,   63,   67,   71,   75,   79,   83,
      88,   92,   96,   100,  105,  109,  114,  118,
      122,  127,  131,  136,  140,  145,  149,  154,
      158,  163,  168,  172,  177,  181,  186,  190,
      195,  199,  204,  208,  213,  217,  222,  226,
      231,  235,  240,  244,  249,  253,  258,  262,
      267,  271,  275,  280,  284,  289,  293,  297,
      302,  306,  311,  315,  319,  324,  328,  332,
      337,  341,  345,  349,  354,  358,  362,  367,
      371,  375,  379,  384,  388,  392,  396,  401,
      409,  417,  425,  433,  441,  449,  458,  466,
      474,  482,  490,  498,  506,  514,  523,  531,
      539,  547,  555,  563,  571,  579,  588,  596,
      604,  616,  628,  640,  652,  664,  676,  688,
      700,  713,  725,  737,  749,  761,  773,  785,
      797,  809,  825,  841,  857,  873,  889,  905,
      922,  938,  954,  970,  986,  1002, 1018, 1038,
      1058, 1078, 1098, 1118, 1138, 1158, 1178, 1198,
      1218, 1242, 1266, 1290, 1314, 1338, 1362, 1386,
      1411, 1435, 1463, 1491, 1519, 1547, 1575, 1603,
      1631, 1663, 1695, 1727, 1759, 1791, 1823, 1859,
      1895, 1931, 1967, 2003, 2039, 2079, 2119, 2159,
      2199, 2239, 2283, 2327, 2371, 2415, 2459, 2507,
      2555, 2603, 2651, 2703, 2755, 2807, 2859, 2915,
      2971, 3027, 3083, 3143, 3203, 3263, 3327, 3391,
      3455, 3523, 3591, 3659, 3731, 3803, 3876, 3952,
      4028, 4104, 4184, 4264, 4348, 4432, 4516, 4604,
      4692, 4784, 4876, 4972, 5068, 5168, 5268, 5372,
      5476, 5584, 5692, 5804, 5916, 6032, 6148, 6268,
      6388, 6512, 6640, 6768, 6900, 7036, 7172, 7312,
};

typedef struct _CODEC_VP9_SEG_PARAMS
{
    union
    {
        struct
        {
            uint16_t      SegmentReferenceEnabled         : 1;        // [0..1]
            uint16_t      SegmentReference                : 2;        // [0..3]
            uint16_t      SegmentReferenceSkipped         : 1;        // [0..1]
            uint16_t      ReservedField3                  : 12;       // [0]
        } fields;
        uint32_t            value;
    } SegmentFlags;

    uint8_t               FilterLevel[4][2];                          // [0..63]
    uint16_t              LumaACQuantScale;                           //
    uint16_t              LumaDCQuantScale;                           //
    uint16_t              ChromaACQuantScale;                         //
    uint16_t              ChromaDCQuantScale;                         //
} CODEC_VP9_SEG_PARAMS, *PCODEC_VP9_SEG_PARAMS;

#endif

