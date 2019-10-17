/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     codechal_decode_vc1.cpp
//! \brief    Implements the decode interface extension for VC1.
//! \details  Implements all functions required by CodecHal for VC1 decoding.
//!

#include "codechal_decoder.h"
#include "codechal_decode_vc1.h"
#include "codechal_secure_decode_interface.h"
#include "codechal_mmc_decode_vc1.h"
#include "hal_oca_interface.h"
#if USE_CODECHAL_DEBUG_TOOL
#include <sstream>
#include <fstream>
#include "codechal_debug.h"
#endif
#define CODECHAL_DECODE_VC1_EOS                    ((uint32_t)(-1))

// picture layer bits
#define CODECHAL_DECODE_VC1_BITS_INTERPFRM         1
#define CODECHAL_DECODE_VC1_BITS_FRMCNT            2
#define CODECHAL_DECODE_VC1_BITS_RANGEREDFRM       1
#define CODECHAL_DECODE_VC1_BITS_FPTYPE            3
#define CODECHAL_DECODE_VC1_BITS_BF                7
#define CODECHAL_DECODE_VC1_BITS_PQINDEX           5
#define CODECHAL_DECODE_VC1_BITS_HALFQP            1
#define CODECHAL_DECODE_VC1_BITS_PQUANTIZER        1
#define CODECHAL_DECODE_VC1_BITS_RESPIC            2
#define CODECHAL_DECODE_VC1_BITS_INTERLCF          1

#define CODECHAL_DECODE_VC1_BITS_TRANSACFRM_1      1
#define CODECHAL_DECODE_VC1_BITS_TRANSACFRM_2      1
#define CODECHAL_DECODE_VC1_BITS_TRANSACFRM2_1     1
#define CODECHAL_DECODE_VC1_BITS_TRANSACFRM2_2     1
#define CODECHAL_DECODE_VC1_BITS_TRANSDCTAB        1

#define CODECHAL_DECODE_VC1_BITS_FRSKIP            1
#define CODECHAL_DECODE_VC1_BITS_TFCNTR            8
#define CODECHAL_DECODE_VC1_BITS_FCM_1             1
#define CODECHAL_DECODE_VC1_BITS_FCM_2             1
#define CODECHAL_DECODE_VC1_BITS_TFF               1
#define CODECHAL_DECODE_VC1_BITS_RFF               1
#define CODECHAL_DECODE_VC1_BITS_REPSEQHDR         1
#define CODECHAL_DECODE_VC1_BITS_UVSAMP            1
#define CODECHAL_DECODE_VC1_BITS_POSTPROC          2
#define CODECHAL_DECODE_VC1_BITS_DQUANTFRM         1
#define CODECHAL_DECODE_VC1_BITS_DQPROFILE         2
#define CODECHAL_DECODE_VC1_BITS_DQSBEDGE          2
#define CODECHAL_DECODE_VC1_BITS_DQDBEDGE          2
#define CODECHAL_DECODE_VC1_BITS_DQBILEVEL         1
#define CODECHAL_DECODE_VC1_BITS_PQDIFF            3
#define CODECHAL_DECODE_VC1_BITS_ABSPQ             5

#define CODECHAL_DECODE_VC1_BITS_MVMODEBIT         1
#define CODECHAL_DECODE_VC1_BITS_LUMSCALE          6
#define CODECHAL_DECODE_VC1_BITS_LUMSHIFT          6
#define CODECHAL_DECODE_VC1_BITS_MVTAB             2
#define CODECHAL_DECODE_VC1_BITS_CBPTAB            2
#define CODECHAL_DECODE_VC1_BITS_TTMBF             1
#define CODECHAL_DECODE_VC1_BITS_TTFRM             2
#define CODECHAL_DECODE_VC1_BITS_ABSMQ             5

#define CODECHAL_DECODE_VC1_BITS_RPTFRM            2
#define CODECHAL_DECODE_VC1_BITS_RNDCTRL           1

#define CODECHAL_DECODE_VC1_BITS_PS_PRESENT        1
#define CODECHAL_DECODE_VC1_BITS_PS_HOFFSET        18
#define CODECHAL_DECODE_VC1_BITS_PS_VOFFSET        18
#define CODECHAL_DECODE_VC1_BITS_PS_WIDTH          14
#define CODECHAL_DECODE_VC1_BITS_PS_HEIGHT         14

#define CODECHAL_DECODE_VC1_BITS_REFDIST           2
#define CODECHAL_DECODE_VC1_BITS_REFFIELD          1

#define CODECHAL_DECODE_VC1_BITS_NUMREF            1
#define CODECHAL_DECODE_VC1_BITS_MBMODETAB         3
#define CODECHAL_DECODE_VC1_BITS_CBPTAB_INTERLACE  3
#define CODECHAL_DECODE_VC1_BITS_4MVBPTAB          2
#define CODECHAL_DECODE_VC1_BITS_2MVBPTAB          2

#define CODECHAL_DECODE_VC1_BITS_4MVSWITCH         1
#define CODECHAL_DECODE_VC1_BITS_INTCOMP           1

// bitplane
#define CODECHAL_DECODE_VC1_BITS_BITPLANE_INVERT   1
#define CODECHAL_DECODE_VC1_BITS_BITPLANE_ROWSKIP  1
#define CODECHAL_DECODE_VC1_BITS_BITPLANE_COLSKIP  1

// slice layer bits
#define CODECHAL_DECODE_VC1_BITS_SC_SUFFIX         8
#define CODECHAL_DECODE_VC1_BITS_SLICE_ADDR        9
#define CODECHAL_DECODE_VC1_BITS_PIC_HEADER_FLAG   1
#define CODECHAL_DECODE_VC1_BITS_SLICE_HEADER      ((CODECHAL_DECODE_VC1_BITS_SC_SUFFIX) + (CODECHAL_DECODE_VC1_BITS_SLICE_ADDR) + (CODECHAL_DECODE_VC1_BITS_PIC_HEADER_FLAG))

#define CODECHAL_DECODE_VC1_MV_OFFEST_SIZE         3

MOS_STATUS CodechalDecodeVc1::GetBits(uint32_t bitsRead, uint32_t &value)
{
    value = GetBits(bitsRead);

    if (CODECHAL_DECODE_VC1_EOS == value)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1::GetVLC(const uint32_t* table, uint32_t &value)
{
    value = GetVLC(table);
    if (CODECHAL_DECODE_VC1_EOS == value)
    {
        return MOS_STATUS_UNKNOWN;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1::SkipWords(uint32_t dwordNumber, uint32_t &value)
{
    for (uint32_t i = 0; i < dwordNumber; i++)
    {
        value = SkipBits(16);
        if (CODECHAL_DECODE_VC1_EOS == value)
        {
            return MOS_STATUS_UNKNOWN;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1::SkipBits(uint32_t bits, uint32_t &value)
{
    if ((bits) > 0)
    {
        value = SkipBits(bits);
        if (CODECHAL_DECODE_VC1_EOS == value)
        {
            return MOS_STATUS_UNKNOWN;
        }
    }
    return MOS_STATUS_SUCCESS;
}

typedef union _CODECHAL_DECODE_VC1_BITSTREAM_BUFFER_VALUE
{
    uint32_t u32Value;
    uint8_t  u8Value[sizeof(uint32_t)];
} CODECHAL_DECODE_VC1_BITSTREAM_VALUE, *PCODECHAL_DECODE_VC1_BITSTREAM_VALUE;

typedef enum _CODECHAL_DECODE_VC1_MVMODE
{
    CODECHAL_VC1_MVMODE_1MV_HALFPEL_BILINEAR,
    CODECHAL_VC1_MVMODE_1MV_HALFPEL,
    CODECHAL_VC1_MVMODE_1MV,
    CODECHAL_VC1_MVMODE_MIXEDMV,
    CODECHAL_VC1_MVMODE_IC          // Intensity Compensation
} CODECHAL_DECODE_VC1_MVMODE;

typedef enum _CODECHAL_DECODE_VC1_BITPLANE_CODING_MODE
{
    CODECHAL_VC1_BITPLANE_RAW,
    CODECHAL_VC1_BITPLANE_NORMAL2,
    CODECHAL_VC1_BITPLANE_DIFF2,
    CODECHAL_VC1_BITPLANE_NORMAL6,
    CODECHAL_VC1_BITPLANE_DIFF6,
    CODECHAL_VC1_BITPLANE_ROWSKIP,
    CODECHAL_VC1_BITPLANE_COLSKIP
} CODECHAL_DECODE_VC1_BITPLANE_CODING_MODE;

static const uint32_t CODECHAL_DECODE_VC1_VldBitplaneModeTable[] =
{
    4, /* max bits */
    0, /* 1-bit codes */
    2, /* 2-bit codes */
    2, CODECHAL_VC1_BITPLANE_NORMAL2,
    3, CODECHAL_VC1_BITPLANE_NORMAL6,
    3, /* 3-bit codes */
    1, CODECHAL_VC1_BITPLANE_DIFF2,
    2, CODECHAL_VC1_BITPLANE_ROWSKIP,
    3, CODECHAL_VC1_BITPLANE_COLSKIP,
    2, /* 4-bit codes */
    0, CODECHAL_VC1_BITPLANE_RAW,
    1, CODECHAL_VC1_BITPLANE_DIFF6,
    (uint32_t)-1
};

static const uint32_t CODECHAL_DECODE_VC1_VldCode3x2Or2x3TilesTable[] =
{
    13, /* max bits */
    1,  /* 1-bit codes */
    1, 0,
    0,  /* 2-bit codes */
    0,  /* 3-bit codes */
    6,  /* 4-bit codes */
    2, 1,
    3, 2,
    4, 4,
    5, 8,

    6, 16,
    7, 32,
    0,  /* 5-bit codes */
    1,  /* 6-bit codes */
    (3 << 1) | 1, 63,
    0,  /* 7-bit codes */
    15, /* 8-bit codes */
    0, 3,
    1, 5,
    2, 6,
    3, 9,

    4, 10,
    5, 12,
    6, 17,
    7, 18,

    8, 20,
    9, 24,
    10, 33,
    11, 34,

    12, 36,
    13, 40,
    14, 48,
    6, /* 9-bit codes */
    (3 << 4) | 7, 31,
    (3 << 4) | 6, 47,
    (3 << 4) | 5, 55,
    (3 << 4) | 4, 59,

    (3 << 4) | 3, 61,
    (3 << 4) | 2, 62,
    20, /* 10-bit codes */
    (1 << 6) | 11, 11,
    (1 << 6) | 7, 7,
    (1 << 6) | 13, 13,
    (1 << 6) | 14, 14,

    (1 << 6) | 19, 19,
    (1 << 6) | 21, 21,
    (1 << 6) | 22, 22,
    (1 << 6) | 25, 25,

    (1 << 6) | 26, 26,
    (1 << 6) | 28, 28,
    (1 << 6) | 3, 35,
    (1 << 6) | 5, 37,

    (1 << 6) | 6, 38,
    (1 << 6) | 9, 41,
    (1 << 6) | 10, 42,
    (1 << 6) | 12, 44,

    (1 << 6) | 17, 49,
    (1 << 6) | 18, 50,
    (1 << 6) | 20, 52,
    (1 << 6) | 24, 56,
    0,  /* 11-bit codes */
    0,  /* 12-bit codes */
    15, /* 13-bit codes */
    (3 << 8) | 14, 15,
    (3 << 8) | 13, 23,
    (3 << 8) | 12, 27,
    (3 << 8) | 11, 29,

    (3 << 8) | 10, 30,
    (3 << 8) | 9, 39,
    (3 << 8) | 8, 43,
    (3 << 8) | 7, 45,

    (3 << 8) | 6, 46,
    (3 << 8) | 5, 51,
    (3 << 8) | 4, 53,
    (3 << 8) | 3, 54,

    (3 << 8) | 2, 57,
    (3 << 8) | 1, 58,
    (3 << 8) | 0, 60,
    (uint32_t)-1
};

static const uint32_t CODECHAL_DECODE_VC1_VldPictureTypeTable[] =
{
    4,  /* max bits */
    1, /* 1-bit codes */
    0, vc1PFrame,
    1, /* 2-bit codes */
    2, vc1BFrame,
    1, /* 3-bit codes */
    6, vc1IFrame,
    2, /* 4-bit codes */
    14, vc1BIFrame,
    15, vc1SkippedFrame,
    (uint32_t)-1
};

static const uint32_t CODECHAL_DECODE_VC1_VldBFractionTable[] =
{
    7,  /* max bits */
    0,  /* 1-bit codes */
    0,  /* 2-bit codes */
    7,  /* 3-bit codes */
    0x00, 0,
    0x01, 1,
    0x02, 2,
    0x03, 3,

    0x04, 4,
    0x05, 5,
    0x06, 6,
    0,  /* 4-bit codes */
    0,  /* 5-bit codes */
    0,  /* 6-bit codes */
    14, /* 7-bit codes */
    0x70, 7,
    0x71, 8,
    0x72, 9,
    0x73, 10,

    0x74, 11,
    0x75, 12,
    0x76, 13,
    0x77, 14,

    0x78, 15,
    0x79, 16,
    0x7A, 17,
    0x7B, 18,

    0x7C, 19,
    0x7D, 20,
    (uint32_t)-1
};

static const uint32_t CODECHAL_DECODE_VC1_VldRefDistTable[] =
{
    14, /* max bits */
    1,  /* 1-bit codes */
    0, 3,
    1,  /* 2-bit codes */
    2, 4,
    1,  /* 3-bit codes */
    6, 5,
    1,  /* 4-bit codes */
    14, 6,
    1,  /* 5-bit codes */
    30, 7,
    1,  /* 6-bit codes */
    62, 8,
    1,  /* 7-bit codes */
    126, 9,
    1,  /* 8-bit codes */
    254, 10,
    1,  /* 9-bit codes */
    510, 11,
    1,  /* 10-bit codes */
    1022, 12,
    1,  /* 11-bit codes */
    2046, 13,
    1,  /* 12-bit codes */
    4094, 14,
    1,  /* 13-bit codes */
    8190, 15,
    1,  /* 14-bit codes */
    16382, 16,
    (uint32_t)-1
};

// lookup tables for MVMODE
static const uint32_t CODECHAL_DECODE_VC1_LowRateMvModeTable[] =
{
    CODECHAL_VC1_MVMODE_1MV_HALFPEL_BILINEAR,
    CODECHAL_VC1_MVMODE_1MV,
    CODECHAL_VC1_MVMODE_1MV_HALFPEL,
    CODECHAL_VC1_MVMODE_MIXEDMV,
    CODECHAL_VC1_MVMODE_IC
};

static const uint32_t CODECHAL_DECODE_VC1_HighRateMvModeTable[] =
{
    CODECHAL_VC1_MVMODE_1MV,
    CODECHAL_VC1_MVMODE_MIXEDMV,
    CODECHAL_VC1_MVMODE_1MV_HALFPEL,
    CODECHAL_VC1_MVMODE_1MV_HALFPEL_BILINEAR,
    CODECHAL_VC1_MVMODE_IC
};

// lock up table for luma polarity (Interlaced picture)
static const CODECHAL_DECODE_VC1_I_LUMA_BLOCKS CODECHAL_DECODE_VC1_LumaBlocks_I[16] =
{
    { 4,{ 0 }, 0, 0, 0 },{ 3,{ 0 }, 2, 4, 6 },{ 3,{ 0 }, 0, 4, 6 },{ 2,{ 4 }, 6, 0, 2 },{ 3,{ 0 }, 0, 2, 6 },{ 2,{ 2 }, 6, 0, 4 },{ 2,{ 0 }, 6, 2, 4 },{ 3,{ 1 }, 0, 2, 4 },
    { 3,{ 0 }, 0, 2, 4 },{ 2,{ 2 }, 4, 0, 6 },{ 2,{ 0 }, 4, 2, 6 },{ 3,{ 1 }, 0, 2, 6 },{ 2,{ 0 }, 2, 4, 6 },{ 3,{ 1 }, 0, 4, 6 },{ 3,{ 1 }, 2, 4, 6 },{ 4,{ 1 }, 0, 0, 0 }
};

// lock up table for luma inter-coded blocks (Progressive picture)
static const CODECHAL_DECODE_VC1_P_LUMA_BLOCKS CODECHAL_DECODE_VC1_LumaBlocks_P[16] =
{
    { 4, 0, 0, 0 },{ 3, 0, 2, 4 },{ 3, 0, 2, 6 },{ 2, 0, 2, 0 },{ 3, 0, 4, 6 },{ 2, 0, 4, 0 },{ 2, 0, 6, 0 },{ 0, 0, 0, 0 },
    { 3, 2, 4, 6 },{ 2, 2, 4, 0 },{ 2, 2, 6, 0 },{ 0, 0, 0, 0 },{ 2, 4, 6, 0 },{ 0, 0, 0, 0 },{ 0, 0, 0, 0 },{ 0, 0, 0, 0 }
};

static const int16_t CODECHAL_DECODE_VC1_MV_OFFEST[CODECHAL_DECODE_VC1_MV_OFFEST_SIZE][2] = { { 0, 2 },{ -2, 0 },{ 0, 0 } };
static const uint8_t CODECHAL_DECODE_VC1_RndTb[4] = { 0, 0, 0, 1 };

const CODECHAL_DECODE_VC1_OLP_STATIC_DATA g_cInit_CODECHAL_DECODE_VC1_OLP_STATIC_DATA =
{
    // uint32_t 0
    {
        { 0 }                                                // Reserved
    },

    // uint32_t 1
    {
        {
            16,                                              // BlockWidth in Byte
            16                                               // BlockHeight in Byte
        }
    },

    // uint32_t 2
    {
        {
            0,                                               // Profile
            0,                                               // RangeExpansionFlag
            0,                                               // UpsamplingFlag
            0,                                               // InterlaceFieldFlag
            0,                                               // RangeMapUV
            0,                                               // RangeMapUVFlag
            0,                                               // RangeMapY
            0,                                               // RangeMapYFlag
            0                                                // ComponentFlag
        }
    },

    // uint32_t 3
    {
        { 0 }                                                // ComponentFlag
    },

    // uint32_t 4
    {
        { 0 }                                                // SourceDataBindingIndex (default: CODECHAL_DECODE_VC1_OLP_SRC_Y)
    },

    // uint32_t 5
    {
        { 3 }                                                // DestDataBindingIndex (default: CODECHAL_DECODE_VC1_OLP_DST_Y)
    },

    // uint32_t 6
    {
        { 0 }                                                // Reserved
    },

    // uint32_t 7
    {
        { 0 }                                                // Reserved
    }
};

//==<Functions>=======================================================

int16_t CodechalDecodeVc1::PackMotionVectorsMedian3(int16_t mv1, int16_t mv2, int16_t mv3)
{
    if (mv1 > mv2)
    {
        if (mv2 > mv3)
            return mv2;
        if (mv1 > mv3)
            return mv3;
        return mv1;
    }
    if (mv1 > mv3)
        return mv1;
    if (mv2 > mv3)
        return mv3;
    return mv2;
}

int16_t CodechalDecodeVc1::PackMotionVectorsMedian4(int16_t mv1, int16_t mv2, int16_t mv3, int16_t mv4)
{
    int16_t max = mv1, min = mv1;

    if (mv2 > max)
    {
        max = mv2;
    }
    else if (mv2 < min)
    {
        min = mv2;
    }

    if (mv3 > max)
    {
        max = mv3;
    }
    else if (mv3 < min)
    {
        min = mv3;
    }

    if (mv4 > max)
    {
        max = mv4;
    }
    else if (mv4 < min)
    {
        min = mv4;
    }

    return (mv1 + mv2 + mv3 + mv4 - max - min) / 2;
}

void CodechalDecodeVc1::PackMotionVectorsChroma4MvP(uint16_t intraFlags, int16_t *lmv, int16_t *cmv)
{
    int16_t mvX = 0, mvY = 0;

    if (CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8NumIntercodedBlocks == 4)
    {
        mvX = PackMotionVectorsMedian4(lmv[0], lmv[2], lmv[4], lmv[6]);
        mvY = PackMotionVectorsMedian4(lmv[1], lmv[3], lmv[5], lmv[7]);
    }
    else if (CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8NumIntercodedBlocks == 3)
    {
        mvX = PackMotionVectorsMedian3(lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex1],
            lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex2],
            lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex3]);
        mvY = PackMotionVectorsMedian3(lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex1 + 1],
            lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex2 + 1],
            lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex3 + 1]);
    }
    else if (CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8NumIntercodedBlocks == 2)
    {
        mvX = (lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex1] + lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex2]) / 2;
        mvY = (lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex1 + 1] + lmv[CODECHAL_DECODE_VC1_LumaBlocks_P[intraFlags].u8MvIndex2 + 1]) / 2;
    }

    cmv[0] = CODECHAL_DECODE_VC1_CHROMA_MV(mvX);
    cmv[1] = CODECHAL_DECODE_VC1_CHROMA_MV(mvY);
}

uint8_t CodechalDecodeVc1::PackMotionVectorsChroma4MvI(
    uint16_t    fieldSelect,
    uint16_t    currentField,
    bool        fastUVMotionCompensation,
    int16_t      *lmv,
    int16_t      *cmv)
{
    int16_t mvX = 0, mvY = 0;
    uint8_t polarity;
    uint16_t offset, offsetIndex;
    uint8_t index1, index2, index3, index4;

    if ((currentField == PICTURE_FRAME) || (currentField == PICTURE_INTERLACED_FRAME))
    {
        offsetIndex = 2;
    }
    else
    {
        offsetIndex = currentField - 1;
    }

    if (offsetIndex >= CODECHAL_DECODE_VC1_MV_OFFEST_SIZE)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("ERROR: offsetIndex out of bounds (%d, max %d)", offsetIndex, CODECHAL_DECODE_VC1_MV_OFFEST_SIZE);
        return 0;
    }

    if (CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8NumSamePolarity == 4)
    {
        polarity = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8Polarity ? 1 : 0;
        offset = CODECHAL_DECODE_VC1_MV_OFFEST[offsetIndex][polarity];

        // Unadjust Luma
        lmv[1] += offset;
        lmv[3] += offset;
        lmv[5] += offset;
        lmv[7] += offset;

        mvX = PackMotionVectorsMedian4(lmv[0], lmv[2], lmv[4], lmv[6]);
        mvY = PackMotionVectorsMedian4(lmv[1], lmv[3], lmv[5], lmv[7]);
    }
    else if (CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8NumSamePolarity == 3)
    {
        polarity = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8Polarity ? 1 : 0;
        index2 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex1;
        index3 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex2;
        index4 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex3;
        index1 = 12 - (index2 + index3 + index4); // Sum of indices is 12

                                                          // Unadjust Luma of current polarity
        offset = CODECHAL_DECODE_VC1_MV_OFFEST[offsetIndex][polarity];
        lmv[index2 + 1] += offset;
        lmv[index3 + 1] += offset;
        lmv[index4 + 1] += offset;

        if (PICTURE_TOP_FIELD != currentField && PICTURE_BOTTOM_FIELD != currentField)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("Invalid Parameters.");
        }
        else
        {
            // Unadjust Luma of opposite polarity
            offset = CODECHAL_DECODE_VC1_MV_OFFEST[currentField - 1][1 - polarity];
        }

        lmv[index1 + 1] += offset;

        mvX = PackMotionVectorsMedian3(lmv[index2],
            lmv[index3],
            lmv[index4]);
        mvY = PackMotionVectorsMedian3(lmv[index2 + 1],
            lmv[index3 + 1],
            lmv[index4 + 1]);
    }
    else
    {
        if (currentField == PICTURE_TOP_FIELD)
        {
            index1 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex0;
            index2 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex1;
            index3 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex2;
            index4 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex3;
            polarity = 0;
        }
        else
        {
            index1 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex2;
            index2 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex3;
            index3 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex0;
            index4 = CODECHAL_DECODE_VC1_LumaBlocks_I[fieldSelect].u8MvIndex1;
            polarity = 1;
        }

        // Unadjust Luma of current polarity
        offset = CODECHAL_DECODE_VC1_MV_OFFEST[offsetIndex][polarity];
        lmv[index1 + 1] += offset;
        lmv[index2 + 1] += offset;

        // Unadjust Luma of opposite polarity
        offset = CODECHAL_DECODE_VC1_MV_OFFEST[offsetIndex][1 - polarity];
        lmv[index3 + 1] += offset;
        lmv[index4 + 1] += offset;

        mvX = (lmv[index1] + lmv[index2]) / 2;
        mvY = (lmv[index1 + 1] + lmv[index2 + 1]) / 2;
        offset = 0;
    }

    cmv[0] = CODECHAL_DECODE_VC1_CHROMA_MV(mvX);
    cmv[1] = CODECHAL_DECODE_VC1_CHROMA_MV(mvY);

    if (fastUVMotionCompensation)
    {
        cmv[0] = CODECHAL_DECODE_VC1_FAST_CHROMA_MV(cmv[0]);
        cmv[1] = CODECHAL_DECODE_VC1_FAST_CHROMA_MV(cmv[1]);
    }

    return polarity;
}

void CodechalDecodeVc1::PackMotionVectors(
    PMHW_VDBOX_VC1_MB_STATE         vc1MbState,
    int16_t                           *mv,
    int16_t                           *packedLumaMvs,
    int16_t                           *packedChromaMv)
{
    uint16_t selectFlags;

    PCODEC_VC1_MB_PARAMS mb = vc1MbState->pMb;
    uint8_t b4Mv = mb->mb_type.motion_4mv;
    uint8_t mfwd = mb->mb_type.motion_forward;
    uint8_t mbwd = mb->mb_type.motion_backward;
    uint8_t mtype = mb->mb_type.motion_type;
    vc1MbState->bMotionSwitch = 0;
    PCODEC_VC1_PIC_PARAMS vc1PicParams = vc1MbState->pVc1PicParams;

    bool isPPicture = m_mfxInterface->IsVc1PPicture(
        vc1PicParams->CurrPic,
        vc1PicParams->picture_fields.is_first_field,
        vc1PicParams->picture_fields.picture_type) ? true : false;

    packedLumaMvs[0] = packedLumaMvs[2] = packedLumaMvs[4] = packedLumaMvs[6] = 0;
    packedLumaMvs[1] = packedLumaMvs[3] = packedLumaMvs[5] = packedLumaMvs[7] = 0;

    packedChromaMv[0] = CODECHAL_DECODE_VC1_CHROMA_MV(packedLumaMvs[0]);
    packedChromaMv[1] = CODECHAL_DECODE_VC1_CHROMA_MV(packedLumaMvs[1]);

    if (b4Mv)
    {
        packedLumaMvs[0] = (int16_t)mv[CodechalDecodeRstFirstForwHorz];
        packedLumaMvs[1] = (int16_t)mv[CodechalDecodeRstFirstForwVert];
        packedLumaMvs[2] = (int16_t)mv[CodechalDecodeRstFirstBackHorz];
        packedLumaMvs[3] = (int16_t)mv[CodechalDecodeRstFirstBackVert];
        packedLumaMvs[4] = (int16_t)mv[CodechalDecodeRstSecndForwHorz];
        packedLumaMvs[5] = (int16_t)mv[CodechalDecodeRstSecndForwVert];
        packedLumaMvs[6] = (int16_t)mv[CodechalDecodeRstSecndBackHorz];
        packedLumaMvs[7] = (int16_t)mv[CodechalDecodeRstSecndBackVert];
        if (vc1MbState->PicFlags == PICTURE_FRAME)
        {
            selectFlags = mb->pattern_code.block_luma_intra;
            PackMotionVectorsChroma4MvP(selectFlags, packedLumaMvs, packedChromaMv);
        }
        else if (vc1MbState->PicFlags != PICTURE_INTERLACED_FRAME)
        {
            selectFlags = (mb->mb_type.value & 0xF000) >> 12;
            vc1MbState->bFieldPolarity = PackMotionVectorsChroma4MvI(
                selectFlags,
                vc1MbState->PicFlags,
                vc1PicParams->fast_uvmc_flag ? true : false,
                packedLumaMvs,
                packedChromaMv);
        }
    }
    else
    {
        // default settings for frame pictures, relevant MVs will be reset if needed
        packedLumaMvs[0] = packedLumaMvs[2] = packedLumaMvs[4] = packedLumaMvs[6] = (int16_t)mv[CodechalDecodeRstFirstForwHorz];
        packedLumaMvs[1] = packedLumaMvs[3] = packedLumaMvs[5] = packedLumaMvs[7] = (int16_t)mv[CodechalDecodeRstFirstForwVert];

        packedChromaMv[0] = CODECHAL_DECODE_VC1_CHROMA_MV(packedLumaMvs[0]);
        packedChromaMv[1] = CODECHAL_DECODE_VC1_CHROMA_MV(packedLumaMvs[1]);

        if (vc1MbState->PicFlags == PICTURE_FRAME)
        {
            if (mbwd && mfwd)
            {
                // Progressive, direct
                packedLumaMvs[2] = packedLumaMvs[6] = (int16_t)mv[CodechalDecodeRstSecndForwHorz];
                packedLumaMvs[3] = packedLumaMvs[7] = (int16_t)mv[CodechalDecodeRstSecndForwVert];
            }
        }
        else
        {
            if (vc1MbState->PicFlags == PICTURE_INTERLACED_FRAME)
            {
                packedLumaMvs[2] = packedLumaMvs[6] = (int16_t)mv[CodechalDecodeRstFirstBackHorz];
                packedLumaMvs[3] = packedLumaMvs[7] = (int16_t)mv[CodechalDecodeRstFirstBackVert];

                if (mtype == CodechalDecodeMcFrame)
                {
                    if (isPPicture)
                    {
                        packedLumaMvs[2] = packedLumaMvs[6] = packedLumaMvs[0];
                        packedLumaMvs[3] = packedLumaMvs[7] = packedLumaMvs[1];
                    }
                }
                else if (mtype == CodechalDecodeMcField)
                {
                    packedLumaMvs[4] = (int16_t)mv[CodechalDecodeRstSecndForwHorz];
                    packedLumaMvs[5] = (int16_t)mv[CodechalDecodeRstSecndForwVert];
                    packedLumaMvs[6] = (int16_t)mv[CodechalDecodeRstSecndBackHorz];
                    packedLumaMvs[7] = (int16_t)mv[CodechalDecodeRstSecndBackVert];
                }
            }
            else // Interlaced field
            {
                uint8_t fieldPolarity2, offsetIndex;
                uint32_t i;
                int16_t offset;

                i = 0;
                fieldPolarity2 = 0;
                offsetIndex = vc1MbState->PicFlags - 1;

                if (offsetIndex >= CODECHAL_DECODE_VC1_MV_OFFEST_SIZE)
                {
                    CODECHAL_DECODE_ASSERTMESSAGE("ERROR: offsetIndex out of bounds (%d, max %d)", offsetIndex, CODECHAL_DECODE_VC1_MV_OFFEST_SIZE);
                    return;
                }

                if (mfwd)
                {
                    vc1MbState->bFieldPolarity = mb->mb_type.mvert_field_sel_0;
                    fieldPolarity2 = mb->mb_type.mvert_field_sel_1;
                    i = 1;
                }

                if (mbwd)
                {
                    vc1MbState->bFieldPolarity = mb->mb_type.mvert_field_sel_1;
                    fieldPolarity2 = mb->mb_type.mvert_field_sel_0;
                    i = 3;

                    packedLumaMvs[2] = packedLumaMvs[6] = (int16_t)mv[CodechalDecodeRstFirstBackHorz];
                    packedLumaMvs[3] = packedLumaMvs[7] = (int16_t)mv[CodechalDecodeRstFirstBackVert];
                }

                // Unadjust luma
                offset = CODECHAL_DECODE_VC1_MV_OFFEST[offsetIndex][vc1MbState->bFieldPolarity];
                packedLumaMvs[i] += offset;

                // Unadjust Luma of opposite polarity
                offset = CODECHAL_DECODE_VC1_MV_OFFEST[offsetIndex][fieldPolarity2];
                packedLumaMvs[4 - i] += offset;

                if (isPPicture)
                {
                    packedLumaMvs[3] = packedLumaMvs[5] = packedLumaMvs[7] = packedLumaMvs[1];

                    if (mb->mb_type.mvert_field_sel_0)
                    {
                        mb->mb_type.mvert_field_sel_0 = 1;
                        mb->mb_type.mvert_field_sel_1 = 1;
                        mb->mb_type.mvert_field_sel_2 = 1;
                        mb->mb_type.mvert_field_sel_3 = 1;
                    }
                }
                else
                {
                    packedLumaMvs[5] = packedLumaMvs[1];
                    packedLumaMvs[7] = packedLumaMvs[3];
                }

                // Derive unadjusted chroma
                packedChromaMv[0] = CODECHAL_DECODE_VC1_CHROMA_MV(packedLumaMvs[i - 1]);
                packedChromaMv[1] = CODECHAL_DECODE_VC1_CHROMA_MV(packedLumaMvs[i]);
            }
        }
    }

    if ((vc1MbState->PicFlags == PICTURE_INTERLACED_FRAME) && (mtype == CodechalDecodeMcField))
    {
        uint16_t mvFieldSel = (mb->mb_type.value & 0xF000) >> 12;
        uint16_t concealForSel2 = 0;
        uint16_t concealForSel3 = 0;

        if (!mb->mb_type.mvert_field_sel_2)
        {
            if (!(packedLumaMvs[4] || packedLumaMvs[5]))
            {
                concealForSel2 = 1;
            }
            packedLumaMvs[5] += 4;
        }
        if (!mb->mb_type.mvert_field_sel_3)
        {
            if (!(packedLumaMvs[6] || packedLumaMvs[7]))
            {
                concealForSel3 = 1;
            }
            packedLumaMvs[7] += 4;
        }

        if (!(mfwd && mbwd) && !b4Mv)
        {
            uint16_t topPredBit, botPredBit;
            uint16_t topPred, botPred;

            if (mfwd && !mbwd)
            {
                vc1MbState->bMotionSwitch = mb->mb_type.mvert_field_sel_1;

                topPredBit = 0;
                botPredBit = (vc1MbState->bMotionSwitch) ? 3 : 2;
            }
            else
            {
                vc1MbState->bMotionSwitch = mb->mb_type.mvert_field_sel_0;

                topPredBit = 1;
                botPredBit = (vc1MbState->bMotionSwitch) ? 2 : 3;
            }

            topPred = mvFieldSel & (1 << topPredBit);
            botPred = mvFieldSel & (1 << botPredBit);

            if (isPPicture)
            {
                packedLumaMvs[0] = packedLumaMvs[2] = packedLumaMvs[topPredBit * 2];
                packedLumaMvs[1] = packedLumaMvs[3] = packedLumaMvs[(topPredBit * 2) + 1];

                packedLumaMvs[4] = packedLumaMvs[6] = packedLumaMvs[botPredBit * 2];
                packedLumaMvs[5] = packedLumaMvs[7] = packedLumaMvs[(botPredBit * 2) + 1];

                mb->mb_type.value &= 0x0FFF;

                if (topPred)
                {
                    mb->mb_type.mvert_field_sel_0 = 1;
                    mb->mb_type.mvert_field_sel_1 = 1;
                }

                if (botPred)
                {
                    mb->mb_type.mvert_field_sel_2 = 1;
                    mb->mb_type.mvert_field_sel_3 = 1;
                }
            }
            else if (vc1MbState->bMotionSwitch) // && !P_TYPE
            {
                if (concealForSel2)
                {
                    packedLumaMvs[4] = packedLumaMvs[6];
                    packedLumaMvs[5] = packedLumaMvs[7];
                }
                if (concealForSel3)
                {
                    packedLumaMvs[6] = packedLumaMvs[4];
                    packedLumaMvs[7] = packedLumaMvs[5];
                }

                mb->mb_type.value &= 0x0FFF;

                if (topPred)
                {
                    if (topPredBit == 1)
                    {
                        mb->mb_type.mvert_field_sel_1 = 1;
                    }
                    else // topPredBit == 0
                    {
                        mb->mb_type.mvert_field_sel_0 = 1;
                    }
                }

                if (botPred)
                {
                    if (botPredBit == 3)
                    {
                        mb->mb_type.mvert_field_sel_2 = 1;
                    }
                    else // botPredBit == 2
                    {
                        mb->mb_type.mvert_field_sel_3 = 1;
                    }
                }
            }
        }
    }

    if (vc1PicParams->fast_uvmc_flag)
    {
        packedChromaMv[0] = CODECHAL_DECODE_VC1_FAST_CHROMA_MV(packedChromaMv[0]);
        packedChromaMv[1] = CODECHAL_DECODE_VC1_FAST_CHROMA_MV(packedChromaMv[1]);
    }
}

MOS_STATUS CodechalDecodeVc1::InitializeUnequalFieldSurface(
    uint8_t                     refListIdx,
    bool                        nullHwInUse)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODEC_PICTURE picture = m_vc1RefList[refListIdx]->RefPic;

    bool isBPicture = m_mfxInterface->IsVc1BPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;

    if (m_vc1RefList[refListIdx]->bUnequalFieldSurfaceValid)
    {
        // Reset the Surface Valid flag for second field of B picture
        if (m_vc1PicParams->CurrPic.FrameIdx == refListIdx && isBPicture)
        {
            m_vc1RefList[refListIdx]->bUnequalFieldSurfaceValid = false;
        }

        // Unequal field surface may be re-used
        return eStatus;
    }

    uint32_t currUnequalFieldIdx, prevRefListIdx;
    if (m_vc1PicParams->CurrPic.FrameIdx == refListIdx && isBPicture)
    {
        currUnequalFieldIdx = m_unequalFieldSurfaceForBType;
    }
    else
    {
        currUnequalFieldIdx = m_currUnequalFieldSurface;

        m_currUnequalFieldSurface =
            (m_currUnequalFieldSurface + 1) % (CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES - 1);

        prevRefListIdx = m_unequalFieldRefListIdx[currUnequalFieldIdx];

        if (prevRefListIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1 &&
            m_vc1PicParams->CurrPic.FrameIdx != prevRefListIdx)
        {
            // Invalidate unequal field index for the old reference
            m_vc1RefList[prevRefListIdx]->bUnequalFieldSurfaceValid = false;
        }
    }
    // Set up new unequal field values
    m_vc1RefList[refListIdx]->bUnequalFieldSurfaceValid = true;
    m_vc1RefList[refListIdx]->dwUnequalFieldSurfaceIdx  = currUnequalFieldIdx;
    m_unequalFieldRefListIdx[currUnequalFieldIdx]       = refListIdx;

    if (m_vc1PicParams->CurrPic.FrameIdx != refListIdx)
    {
        MOS_SURFACE srcSurface;
        MOS_ZeroMemory(&srcSurface, sizeof(MOS_SURFACE));
        srcSurface.Format = Format_NV12;
        srcSurface.OsResource = m_vc1RefList[refListIdx]->resRefPic;
        CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &srcSurface));

        // Format the unequal field reference
        CODECHAL_DECODE_CHK_STATUS_RETURN(FormatUnequalFieldPicture(
            srcSurface,
            m_unequalFieldSurface[currUnequalFieldIdx],
            false,
            nullHwInUse));
    }

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : CodechalDecodeVc1::FormatUnequalFieldPicture
| Purpose   : Formats the destination surface, in the pack case the UV surface
|               is moved to be adjacent to the UV surface such that NV12
|               formatting is maintained when the surface is returned to SW,
|               in the unpack case the UV surface is moved to be 32-pixel rows
|               away from the Y surface so that during decoding HW will not
|               overwrite the UV surface
| Arguments : dwSrcSurfaceHandle - surface handle for the source surface
|             dwDstSurfaceHandle - surface handle for the destination surface
|             bPack - boolean value determining whether or not the source
|               surface should be packed or unpacked
| Returns   : MOS_STATUS - for success or failure
\---------------------------------------------------------------------------*/
MOS_STATUS CodechalDecodeVc1::FormatUnequalFieldPicture(
    MOS_SURFACE                 srcSurface,
    MOS_SURFACE                 dstSurface,
    bool                        pack,
    bool                        nullHwInUse)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    uint32_t uvblockWidth = 16;
    uint32_t uvblockHeight = 16;
    uint32_t uvblockSize = uvblockWidth * uvblockHeight;
    uint32_t frameHeight = MOS_ALIGN_CEIL(m_height, 16);
    uint32_t ysize = srcSurface.dwPitch * MOS_ALIGN_CEIL(frameHeight, MOS_YTILE_H_ALIGNMENT);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));

    uint32_t frameSize = MOS_ALIGN_CEIL((srcSurface.dwPitch * (frameHeight + frameHeight / 2)), MOS_YTILE_H_ALIGNMENT);

    // Copy Y data first
    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &srcSurface.OsResource;
        dataCopyParams.srcSize = ysize;
        dataCopyParams.srcOffset = 0;
        dataCopyParams.dstResource = &dstSurface.OsResource;
        dataCopyParams.dstSize = frameSize;
        dataCopyParams.dstOffset = 0;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
    }
    else
    {
        // Stream object params
        CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
            &cmdBuffer,                 // pCmdBuffer
            &srcSurface.OsResource,    // presSrc
            &dstSurface.OsResource,    // presDst
            ysize,                   // u32CopyLength
            0,                          // u32CopyInputOffset
            0));                        // u32CopyOutputOffset
    }

    // Copy 1 MB row of UV
    uint32_t srcOffset, dstOffset;
    for (uint32_t x = 0; x < srcSurface.dwPitch; x += uvblockWidth)
    {
        if (pack)
        {
            srcOffset = LinearToYTiledAddress(
                x,
                frameHeight + MOS_YTILE_H_ALIGNMENT,
                srcSurface.dwPitch);
            dstOffset = LinearToYTiledAddress(
                x,
                frameHeight,
                dstSurface.dwPitch);
        }
        else
        {
            srcOffset = LinearToYTiledAddress(
                x,
                frameHeight,
                srcSurface.dwPitch);
            dstOffset = LinearToYTiledAddress(
                x,
                frameHeight + MOS_YTILE_H_ALIGNMENT,
                dstSurface.dwPitch);
        }

        if (m_hwInterface->m_noHuC)
        {
            CodechalDataCopyParams dataCopyParams;
            MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
            dataCopyParams.srcResource = &srcSurface.OsResource;
            dataCopyParams.srcSize = uvblockSize;
            dataCopyParams.srcOffset = srcOffset;
            dataCopyParams.dstResource = &dstSurface.OsResource;
            dataCopyParams.dstSize = frameSize;
            dataCopyParams.dstOffset = dstOffset;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
        }
        else
        {
            // Stream object params
            CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
                &cmdBuffer,                 // pCmdBuffer
                &srcSurface.OsResource,    // presSrc
                &dstSurface.OsResource,    // presDst
                uvblockSize,             // u32CopyLength
                srcOffset,               // u32CopyInputOffset
                dstOffset));             // u32CopyOutputOffset
        }
    }

    uint32_t uvsize = srcSurface.dwPitch * MOS_ALIGN_CEIL(((frameHeight / 2) - uvblockHeight), MOS_YTILE_H_ALIGNMENT);

    if (pack)
    {
        srcOffset = srcSurface.dwPitch *
            (frameHeight + MOS_YTILE_H_ALIGNMENT + uvblockHeight);
        dstOffset = dstSurface.dwPitch * (frameHeight + uvblockHeight);
    }
    else
    {
        srcOffset = srcSurface.dwPitch * (frameHeight + uvblockHeight);
        dstOffset = dstSurface.dwPitch *
            (frameHeight + MOS_YTILE_H_ALIGNMENT + uvblockHeight);
    }

    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &srcSurface.OsResource;
        dataCopyParams.srcSize = uvsize;
        dataCopyParams.srcOffset = srcOffset;
        dataCopyParams.dstResource = &dstSurface.OsResource;
        dataCopyParams.dstSize = frameSize;
        dataCopyParams.dstOffset = dstOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
    }
    else
    {
        // Stream object params
        CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
            &cmdBuffer,                 // pCmdBuffer
            &srcSurface.OsResource,    // presSrc
            &dstSurface.OsResource,    // presDst
            uvsize,                  // u32CopyLength
            srcOffset,               // u32CopyInputOffset
            dstOffset));             // u32CopyOutputOffset
    }

    if (pack)
    {
        MOS_SYNC_PARAMS syncParams;

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly = false;
        syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

        // Update the resource tag (s/w tag) for On-Demand Sync
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // Update the tag in GPU Sync status buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
        }

    }
    else
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, nullHwInUse));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ConstructBistreamBuffer()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &m_resDataBuffer;
        dataCopyParams.srcSize     = m_dataSize;
        dataCopyParams.srcOffset = 0;
        dataCopyParams.dstResource = &m_resPrivateBistreamBuffer;
        dataCopyParams.dstSize     = m_privateBistreamBufferSize;
        dataCopyParams.dstOffset = CODECHAL_DECODE_VC1_STUFFING_BYTES;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));

        return MOS_STATUS_SUCCESS;
    }

    m_huCCopyInUse = true;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContextForWa));
    m_osInterface->pfnResetOsStates(m_osInterface);
    m_osInterface->pfnSetPerfTag(m_osInterface, (uint16_t)(((m_mode << 4) & 0xF0) | COPY_TYPE));

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Send command buffer header at the beginning (OS dependent)
    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
        &cmdBuffer,                            // pCmdBuffer
        &m_resDataBuffer,                      // presSrc
        &m_resPrivateBistreamBuffer,           // presDst
        MOS_ALIGN_CEIL(m_dataSize, 16),        // u32CopyLength
        0,                                     // u32CopyInputOffset
        CODECHAL_DECODE_VC1_STUFFING_BYTES));  // u32CopyOutputOffset

    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
        &cmdBuffer,
        &flushDwParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
        &cmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    MOS_SYNC_PARAMS syncParams;

    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = m_videoContext;
    syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams                  = g_cInitSyncParams;
    syncParams.GpuContext       = m_videoContextForWa;
    syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext));

    return (MOS_STATUS)eStatus;
}

MOS_STATUS CodechalDecodeVc1::HandleSkipFrame()
{
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_GENERIC_PROLOG_PARAMS           genericPrologParams;
    MOS_SURFACE                         srcSurface;
    uint8_t                             fwdRefIdx;
    uint32_t                            surfaceSize;
    MOS_SYNC_PARAMS                     syncParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    fwdRefIdx = (uint8_t)m_vc1PicParams->ForwardRefIdx;

    MOS_ZeroMemory(&srcSurface, sizeof(MOS_SURFACE));
    srcSurface.Format      = Format_NV12;
    srcSurface.OsResource  = m_vc1RefList[fwdRefIdx]->resRefPic;
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &srcSurface));

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetSurfaceMmcMode(&m_destSurface, &srcSurface));
#endif

        surfaceSize = ((srcSurface.OsResource.pGmmResInfo->GetArraySize()) > 1) ?
            ((uint32_t)(srcSurface.OsResource.pGmmResInfo->GetQPitchPlanar(GMM_PLANE_Y) *
                        srcSurface.OsResource.pGmmResInfo->GetRenderPitch())) :
            (uint32_t)(srcSurface.OsResource.pGmmResInfo->GetSizeMainSurface());

    // HuC is present
    if (m_hwInterface->m_noHuC)
    {
        CodechalDataCopyParams dataCopyParams;
        MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
        dataCopyParams.srcResource = &srcSurface.OsResource;
        dataCopyParams.srcSize     = surfaceSize;
        dataCopyParams.srcOffset = srcSurface.dwOffset;
        dataCopyParams.dstResource = &m_destSurface.OsResource;
        dataCopyParams.dstSize     = surfaceSize;
        dataCopyParams.dstOffset   = m_destSurface.dwOffset;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));
    }
    else
    {
        m_huCCopyInUse = true;

        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams                  = g_cInitSyncParams;
        syncParams.GpuContext       = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContextForWa));
        m_osInterface->pfnResetOsStates(m_osInterface);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

        // Send command buffer header at the beginning (OS dependent)
        CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, false));

        CODECHAL_DECODE_CHK_STATUS_RETURN(HucCopy(
            &cmdBuffer,                             // pCmdBuffer
            &srcSurface.OsResource,                 // presSrc
            &m_destSurface.OsResource,              // presDst
            surfaceSize,                            // u32CopyLength
            srcSurface.dwOffset,                    // u32CopyInputOffset
            m_destSurface.dwOffset));               // u32CopyOutputOffset

        syncParams                          = g_cInitSyncParams;
        syncParams.GpuContext               = m_videoContextForWa;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly                = false;
        syncParams.bDisableDecodeSyncLock   = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

        // Update the resource tag (s/w tag) for On-Demand Sync
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(
                &cmdBuffer,
                &syncParams));
        }

        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
                &cmdBuffer,
                nullptr));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext));
    }

    return (MOS_STATUS)eStatus;
}

uint32_t CodechalDecodeVc1::PeekBits(uint32_t bitsRead)
{
    uint32_t value = 0;

    CODECHAL_DECODE_ASSERT((bitsRead) > 0 && (bitsRead) <= 32);

    uint32_t *cache       = m_bitstream.pu32Cache;
    int32_t   shiftOffset = m_bitstream.iBitOffset - (bitsRead);

    if (shiftOffset >= 0)
    {
        value = (*cache) >> (shiftOffset);
    }
    else
    {
        shiftOffset += 32;
        value = (cache[0] << (32 - shiftOffset)) + (cache[1] >> shiftOffset);
    }

    return (value & ((1 << bitsRead) - 1));
}

uint32_t CodechalDecodeVc1::UpdateBitstreamBuffer()
{
    uint32_t *cache             = (uint32_t *)m_bitstream.CacheBuffer;
    uint32_t *cacheEnd          = m_bitstream.pu32CacheEnd;
    uint32_t *cacheDataEnd      = m_bitstream.pu32CacheDataEnd;
    uint32_t  zeroNum           = m_bitstream.u32ZeroNum;
    uint8_t * originalBitBuffer = m_bitstream.pOriginalBitBuffer;
    uint8_t * originalBufferEnd = m_bitstream.pOriginalBufferEnd;

    if (cacheDataEnd == cacheEnd)
    {
        *cache++ = *cacheEnd;
    }

    while (cache <= cacheEnd)
    {
        uint32_t leftByte;
        CODECHAL_DECODE_VC1_BITSTREAM_VALUE value;
        if (m_bitstream.bIsEBDU)
        {
            // for EBDU, set dwLeftByte to 4 to remove emulation prevention bytes in the later while loop
            leftByte = 4;
            value.u32Value = 0;
        }
        else
        {
            leftByte = 0;
            value.u8Value[3] = *originalBitBuffer++;
            value.u8Value[2] = *originalBitBuffer++;
            value.u8Value[1] = *originalBitBuffer++;
            value.u8Value[0] = *originalBitBuffer++;
        }

        while (leftByte)
        {
            if (originalBitBuffer >= originalBufferEnd) // End of the bitstream;
            {
                *cache = value.u32Value;
                m_bitstream.pu32Cache          = (uint32_t *)m_bitstream.CacheBuffer;
                m_bitstream.u32ZeroNum         = zeroNum;
                m_bitstream.pOriginalBitBuffer = originalBitBuffer;
                m_bitstream.pu32CacheDataEnd   = cache;
                m_bitstream.iBitOffsetEnd      = leftByte * 8;
                return 0;
            }

            uint8_t data = *originalBitBuffer++;

            if (zeroNum < 2)
            {
                zeroNum = data ? 0 : zeroNum + 1;
            }
            else if (zeroNum == 2)
            {
                if (data == 0x03)
                {
                    if (originalBitBuffer < originalBufferEnd)
                    {
                        data = *originalBitBuffer++;
                        zeroNum = (data == 0);
                    }
                    else
                    {
                        CODECHAL_DECODE_ASSERTMESSAGE("VC1 Bitstream Parsing Error: Incomplete bitstream.");
                        return(CODECHAL_DECODE_VC1_EOS);
                    }

                    if (data > 0x03)
                    {
                        CODECHAL_DECODE_ASSERTMESSAGE("VC1 Bitstream Parsing Error: Not a valid code 0x000003 %x.", data);
                        return(CODECHAL_DECODE_VC1_EOS);
                    }
                }
                else if (data == 0x02)
                {
                    CODECHAL_DECODE_ASSERTMESSAGE("VC1 Bitstream Parsing Error: Not a valid code 0x000002.");
                    return(CODECHAL_DECODE_VC1_EOS);
                }
                else
                {
                    zeroNum = data ? 0 : (zeroNum + 1);
                }
            }
            else // zeroNum > 3
            {
                if (data == 0x00)
                {
                    zeroNum++;
                }
                else if (data == 0x01)
                {
                    zeroNum = 0;
                }
                else
                {
                    CODECHAL_DECODE_ASSERTMESSAGE("VC1 Bitstream Parsing Error: Not a start code 0x000001.");
                    return(CODECHAL_DECODE_VC1_EOS);
                }
            }

            leftByte--;
            value.u8Value[leftByte] = data;
        }

        *cache = value.u32Value;
        cache++;
    }

    m_bitstream.pu32Cache          = (uint32_t *)m_bitstream.CacheBuffer;
    m_bitstream.u32ZeroNum         = zeroNum;
    m_bitstream.pOriginalBitBuffer = originalBitBuffer;
    m_bitstream.iBitOffsetEnd      = 0;
    m_bitstream.pu32CacheDataEnd   = m_bitstream.pu32CacheEnd;

    return 0;
}

uint32_t CodechalDecodeVc1::GetBits(uint32_t bitsRead)
{
    uint32_t        value = 0;

    CODECHAL_DECODE_ASSERT((bitsRead > 0) && (bitsRead <= 32));

    uint32_t *cache       = m_bitstream.pu32Cache;
    int32_t   shiftOffset = m_bitstream.iBitOffset - (bitsRead);

    if (shiftOffset >= 0)
    {
        value = (*cache) >> (shiftOffset);
    }
    else
    {
        shiftOffset += 32;
        value = (cache[0] << (32 - shiftOffset)) + (cache[1] >> shiftOffset);
        m_bitstream.pu32Cache++;
    }

    value &= ((0x1 << bitsRead) - 1);
    m_bitstream.iBitOffset = shiftOffset;
    m_bitstream.u32ProcessedBitNum += bitsRead;

    if ((cache == m_bitstream.pu32CacheDataEnd) &&
        (m_bitstream.iBitOffset < m_bitstream.iBitOffsetEnd))
    {
        return CODECHAL_DECODE_VC1_EOS;
    }

    if (cache == m_bitstream.pu32CacheEnd)
    {
        if (UpdateBitstreamBuffer() == CODECHAL_DECODE_VC1_EOS)
        {
            return CODECHAL_DECODE_VC1_EOS;
        }
    }

    return value;
}

uint32_t CodechalDecodeVc1::SkipBits(uint32_t bitsRead)
{
    CODECHAL_DECODE_ASSERT((bitsRead > 0) && (bitsRead <= 32));

    uint32_t *cache       = m_bitstream.pu32Cache;
    int32_t   shiftOffset = m_bitstream.iBitOffset - (bitsRead);

    if (shiftOffset < 0)
    {
        shiftOffset += 32;
        m_bitstream.pu32Cache++;
    }

    m_bitstream.iBitOffset = shiftOffset;
    m_bitstream.u32ProcessedBitNum += bitsRead;

    if ((cache == m_bitstream.pu32CacheDataEnd) &&
        (m_bitstream.iBitOffset < m_bitstream.iBitOffsetEnd))
    {
        return CODECHAL_DECODE_VC1_EOS;
    }

    if (cache == m_bitstream.pu32CacheEnd)
    {
        if (UpdateBitstreamBuffer() == CODECHAL_DECODE_VC1_EOS)
        {
            return CODECHAL_DECODE_VC1_EOS;
        }
    }

    return 0;
}

uint32_t CodechalDecodeVc1::GetVLC(const uint32_t *table)
{
    if (table == nullptr)
        return CODECHAL_DECODE_VC1_EOS;

    CODECHAL_DECODE_ASSERT(table[0] > 0);    // max bits

    uint32_t maxCodeLength = table[0];
    uint32_t tableSize = table[0];
    uint32_t index = 1;
    uint32_t codeLength = 1;

    uint32_t value = PeekBits(maxCodeLength);
    if (CODECHAL_DECODE_VC1_EOS == value)
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Bitstream exhausted.");
        return(value);
    }

    for (uint32_t entryIndex = 0; entryIndex < tableSize; entryIndex++)
    {
        uint32_t subtableSize = table[index++];

        if (subtableSize > 0)
        {
            while (subtableSize--)
            {
                if (table[index++] == (value >> (maxCodeLength - codeLength)))
                {
                    value = GetBits((uint8_t)codeLength);

                    return(table[index]);
                }

                index++;
            }
        }

        codeLength++;
    }

    CODECHAL_DECODE_ASSERTMESSAGE("Code is not in VLC table.");

    return(CODECHAL_DECODE_VC1_EOS);
}

MOS_STATUS CodechalDecodeVc1::InitialiseBitstream(
    uint8_t*                           buffer,
    uint32_t                           length,
    bool                               isEBDU)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_ZeroMemory(&m_bitstream, sizeof(m_bitstream));
    CODECHAL_DECODE_CHK_NULL_RETURN(&m_bitstream);
    CODECHAL_DECODE_CHK_NULL_RETURN(buffer);

    m_bitstream.pOriginalBitBuffer = buffer;
    m_bitstream.pOriginalBufferEnd = buffer + length;
    m_bitstream.u32ZeroNum         = 0;
    m_bitstream.u32ProcessedBitNum = 0;
    m_bitstream.pu32Cache          = (uint32_t *)m_bitstream.CacheBuffer;
    m_bitstream.pu32CacheEnd       = (uint32_t *)(m_bitstream.CacheBuffer + CODECHAL_DECODE_VC1_BITSTRM_BUF_LEN);
    m_bitstream.pu32CacheDataEnd   = (uint32_t *)m_bitstream.CacheBuffer;
    m_bitstream.iBitOffset         = 32;
    m_bitstream.iBitOffsetEnd      = 32;
    m_bitstream.bIsEBDU            = isEBDU;

    if (UpdateBitstreamBuffer() == CODECHAL_DECODE_VC1_EOS)
    {
        return MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::BitplaneNorm2Mode()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);
    uint16_t frameFieldWidthInMb = m_picWidthInMb;

    uint32_t count = frameFieldWidthInMb * frameFieldHeightInMb;

    uint32_t value;
    if ((frameFieldWidthInMb * frameFieldHeightInMb) & 1)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));

        count--;
    }

    for (uint32_t i = 0; i < count / 2; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            if (value == 0)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::BitplaneNorm6Mode()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);
    uint16_t frameFieldWidthInMb = m_picWidthInMb;

    bool is2x3Tiled = (0 != frameFieldWidthInMb % 3) && (0 == frameFieldHeightInMb % 3);

    uint32_t heightInTiles, widthInTiles;
    uint32_t residualX, residualY;
    uint32_t value;
    if (is2x3Tiled)
    {
        widthInTiles = frameFieldWidthInMb / 2;
        heightInTiles = frameFieldHeightInMb / 3;

        for (uint32_t j = 0; j < heightInTiles; j++)
        {
            for (uint32_t i = 0; i < widthInTiles; i++)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldCode3x2Or2x3TilesTable, value));
            }
        }

        residualX = frameFieldWidthInMb & 1;
        residualY = 0;
    }
    else // 3x2 tiles
    {
        widthInTiles = frameFieldWidthInMb / 3;
        heightInTiles = frameFieldHeightInMb / 2;

        for (uint32_t j = 0; j < heightInTiles; j++)
        {
            for (uint32_t i = 0; i < widthInTiles; i++)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldCode3x2Or2x3TilesTable, value));
            }
        }

        residualX = frameFieldWidthInMb % 3;
        residualY = frameFieldHeightInMb & 1;
    }

    // ResidualY 0 or 1 or 2
    for (uint32_t i = 0; i < residualX; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_BITPLANE_COLSKIP, value));

        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipWords(frameFieldHeightInMb >> 4, value));
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(frameFieldHeightInMb & 0xF, value));
        }
    }

    // ResidualY 0 or 1
    for (uint32_t j = 0; j < residualY; j++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_BITPLANE_ROWSKIP, value));

        if (value)
        {
            uint32_t skipBits = frameFieldWidthInMb - residualX;
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipWords(skipBits >> 4, value));
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(skipBits & 0xF, value));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::BitplaneRowskipMode()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);
    uint16_t frameFieldWidthInMb = m_picWidthInMb;

    uint32_t value;
    for (uint32_t j = 0; j < frameFieldHeightInMb; j++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_BITPLANE_ROWSKIP, value));

        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipWords(frameFieldWidthInMb >> 4, value));
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(frameFieldWidthInMb & 0xF, value));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::BitplaneColskipMode()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint16_t meFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        meFieldHeightInMb);
    uint16_t frameFieldWidthInMb = m_picWidthInMb;

    uint32_t value;
    uint32_t colSkip;
    for (uint32_t i = 0; i < frameFieldWidthInMb; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_BITPLANE_COLSKIP, value));
        colSkip = value;

        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipWords(meFieldHeightInMb >> 4, value));
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(meFieldHeightInMb & 0xF, value));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseVopDquant()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t value;
    uint32_t dquantFRM = 0, dqprofile = 0, dqbilevel = 0;
    if ((1 == m_vc1PicParams->pic_quantizer_fields.dquant) ||
        (3 == m_vc1PicParams->pic_quantizer_fields.dquant))
    {
        // The DQUANTFRM field is a 1 bit value that is present only
        // when DQUANT = 1.  If DQUANTFRM = 0 then the current picture
        // is only quantized with PQUANT.
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_DQUANTFRM, value));
        dquantFRM = value;

        if (dquantFRM)
        {
            // The DQPROFILE field is a 2 bits value that is present
            // only when DQUANT = 1 and DQUANTFRM = 1.  It indicates
            // where we are allowed to change quantization step sizes
            // within the current picture.
            // Table 15:  Macroblock Quantization Profile (DQPROFILE) Code Table
            // FLC    Location
            // 00    All four Edges
            // 01    Double Edges
            // 10    Single Edges
            // 11    All Macroblocks
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_DQPROFILE, value));
            dqprofile = value;

            switch (dqprofile)
            {
            case 0: // all 4 edges
            {
                break;
            }
            case 1: // double edges
            {
                // The DQSBEDGE field is a 2 bits value that is present
                // when DQPROFILE = Single Edge.  It indicates which edge
                // will be quantized with ALTPQUANT.
                // Table 16:  Single Boundary Edge Selection (DQSBEDGE) Code Table
                // FLC    Boundary Edge
                // 00    Left
                // 01    Top
                // 10    Right
                // 11    Bottom
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_DQDBEDGE, value));
                break;
            }
            case 2: // single edge
            {
                // The DQSBEDGE field is a 2 bits value that is present
                // when DQPROFILE = Single Edge.  It indicates which edge
                // will be quantized with ALTPQUANT.
                // Table 16:  Single Boundary Edge Selection (DQSBEDGE) Code Table
                // FLC    Boundary Edge
                // 00    Left
                // 01    Top
                // 10    Right
                // 11    Bottom
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_DQDBEDGE, value));
                break;
            }
            case 3: // all MBs
            {
                // The DQBILEVEL field is a 1 bit value that is present
                // when DQPROFILE = All Macroblock.  If DQBILEVEL = 1,
                // then each macroblock in the picture can take one of
                // two possible values (PQUANT or ALTPQUANT).  If
                // DQBILEVEL = 0, then each macroblock in the picture
                // can take on any quantization step size.
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_DQBILEVEL, value));
                dqbilevel = value;
                break;
            }
            }
        }
    }
    else if (2 == m_vc1PicParams->pic_quantizer_fields.dquant)
    {
        dquantFRM = 1;
    }

    // PQDIFF is a 3 bit field that encodes either the PQUANT
    // differential or encodes an escape code.
    // If PQDIFF does not equal 7 then PQDIFF encodes the
    // differential and the ABSPQ field does not follow in
    // the bitstream. In this case:
    //       ALTPQUANT = PQUANT + PQDIFF + 1
    // If PQDIFF equals 7 then the ABSPQ field follows in
    // the bitstream and ALTPQUANT is decoded as:
    //       ALTPQUANT = ABSPQ
    if (dquantFRM)
    {
        if ((m_vc1PicParams->pic_quantizer_fields.dquant == 2) ||
            !(dqprofile == 3 && dqbilevel == 0))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_PQDIFF, value));

            if (7 == value)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_ABSPQ, value));
            }
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseMvRange()
{
    uint32_t    value;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    // MVRANGE
    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));

    if (value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));

        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseProgressiveMvMode(
    const uint32_t                     mvModeTable[],
    uint32_t*                          mvMode)
{
    uint32_t    value;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    uint32_t bitCount = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
    while ((value == 0) && (bitCount < 4))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        bitCount++;
    }

    uint32_t index = (bitCount < 4) ? bitCount - 1 : bitCount + value - 1;
    uint32_t mvModeType = mvModeTable[index];

    if (CODECHAL_VC1_MVMODE_IC == mvModeType)
    {
        bitCount = 1;
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        while ((value == 0) && (bitCount < 3))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            bitCount++;
        }

        index = (bitCount < 3) ? bitCount - 1 : bitCount + !value - 1;
        mvModeType = mvModeTable[index];

        CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(CODECHAL_DECODE_VC1_BITS_LUMSCALE + CODECHAL_DECODE_VC1_BITS_LUMSHIFT, value));
    }

    *mvMode = mvModeType;

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseInterlaceMVMode(
    bool                               isPPicture,
    uint32_t*                          mvmode)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_CHK_NULL_RETURN(mvmode);

    const uint32_t *mvModeTable;
    if (12 < m_vc1PicParams->pic_quantizer_fields.pic_quantizer_scale)
    {
        mvModeTable = CODECHAL_DECODE_VC1_LowRateMvModeTable;
    }
    else
    {
        mvModeTable = CODECHAL_DECODE_VC1_HighRateMvModeTable;
    }

    uint32_t bitCount = 1;
    uint32_t value;
    uint32_t index, mvMode;
    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));

    if (isPPicture)
    {
        while ((value == 0) && (bitCount < 4))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            bitCount++;
        }

        index = (bitCount < 4) ? bitCount - 1 : bitCount + value - 1;
        mvMode = mvModeTable[index];
    }
    else // B picture
    {
        while ((value == 0) && (bitCount < 3))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            bitCount++;
        }

        index = (bitCount < 3) ? bitCount - 1 : bitCount + !value - 1;
    }

    mvMode = mvModeTable[index];

    if (CODECHAL_VC1_MVMODE_IC == value)
    {
        bitCount = 1;
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        while ((value == 0) && (bitCount < 3))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            bitCount++;
        }

        index = (bitCount < 3) ? bitCount - 1 : bitCount + !value - 1;
        mvMode = mvModeTable[index];

        // Intensity compensation flags
        uint32_t skipBits = 0;
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        if (value == 0)
        {
            skipBits += 1 + CODECHAL_DECODE_VC1_BITS_LUMSCALE + CODECHAL_DECODE_VC1_BITS_LUMSHIFT;
        }

        skipBits += CODECHAL_DECODE_VC1_BITS_LUMSCALE + CODECHAL_DECODE_VC1_BITS_LUMSHIFT;

        CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(skipBits, value));
    }

    *mvmode = mvMode;

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseBitplane()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t value;
    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_BITPLANE_INVERT, value));

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldBitplaneModeTable, value));

    switch (value) // Bitplane mode
    {
    case CODECHAL_VC1_BITPLANE_NORMAL2:
        eStatus = BitplaneNorm2Mode();
        break;
    case CODECHAL_VC1_BITPLANE_NORMAL6:
        eStatus = BitplaneNorm6Mode();
        break;
    case CODECHAL_VC1_BITPLANE_DIFF2:
        eStatus = BitplaneNorm2Mode();  // Diff2 is the same as Norm2 mode
        break;
    case CODECHAL_VC1_BITPLANE_DIFF6:
        eStatus = BitplaneNorm6Mode();  // Diff6 is the same as Norm6 mode
        break;
    case CODECHAL_VC1_BITPLANE_ROWSKIP:
        eStatus = BitplaneRowskipMode();
        break;
    case CODECHAL_VC1_BITPLANE_COLSKIP:
        eStatus = BitplaneColskipMode();
        break;
    case CODECHAL_VC1_BITPLANE_RAW:
        // nothing to do
        break;
    default:
        CODECHAL_DECODE_ASSERTMESSAGE("Invalid bitplane mode %d.", value);
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParsePictureLayerIAdvanced()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t value;
    if (CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());

    if (m_vc1PicParams->sequence_fields.overlap &&
        (m_vc1PicParams->pic_quantizer_fields.pic_quantizer_scale <= 8))
    {
        //conditional overlap flag
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));

        if (1 == value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            if (1 == value)
            {
                // CONDOVER == 2
                CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());
            }
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_1, value));
    if (0 != value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_2, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM2_1, value));
    if (0 != value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM2_2, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSDCTAB, value));

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseVopDquant());

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParsePictureLayerPAdvanced()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_vc1PicParams->mv_fields.extended_mv_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseMvRange());
    }

    const uint32_t *mvModeTable = nullptr;
    mvModeTable                 = (12 < m_vc1PicParams->pic_quantizer_fields.pic_quantizer_scale) ? CODECHAL_DECODE_VC1_LowRateMvModeTable : CODECHAL_DECODE_VC1_HighRateMvModeTable;

    uint32_t mvMode;
    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseProgressiveMvMode(mvModeTable, &mvMode));

    if (CODECHAL_VC1_MVMODE_MIXEDMV == mvMode)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());

    uint32_t value;
    CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(CODECHAL_DECODE_VC1_BITS_MVTAB + CODECHAL_DECODE_VC1_BITS_CBPTAB, value));

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseVopDquant());

    if (m_vc1PicParams->transform_fields.variable_sized_transform_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTMBF, value));

        if (1 == value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTFRM, value));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_1, value));

    if (0 != value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_2, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSDCTAB, value));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParsePictureLayerBAdvanced()
{
    uint32_t    value;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    if (m_vc1PicParams->mv_fields.extended_mv_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseMvRange());
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));

    // B frame direct mode macroblock bit syntax element
    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());

    // skipped macroblock bit syntax element
    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());

    CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(CODECHAL_DECODE_VC1_BITS_MVTAB + CODECHAL_DECODE_VC1_BITS_CBPTAB, value));

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseVopDquant());

    if (m_vc1PicParams->transform_fields.variable_sized_transform_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTMBF, value));

        if (1 == value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTFRM, value));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_1, value));

    if (0 != value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_2, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSDCTAB, value));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseFieldPictureLayerPAdvanced()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t value;
    uint32_t numRef;
    if (CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_NUMREF, value));
        numRef = value;

        if (0 == value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_REFFIELD, value));
        }
    }
    else
    {
        numRef = 0;
    }

    if (m_vc1PicParams->mv_fields.extended_mv_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseMvRange());
    }

    if (m_vc1PicParams->mv_fields.extended_dmv_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            if (value)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            }
        }
    }

    uint32_t mvMode;
    if (CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseInterlaceMVMode(true, &mvMode));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_4MVSWITCH, value));
        if (value)
        {
            mvMode = CODECHAL_VC1_MVMODE_MIXEDMV;
        }
        else
        {
            mvMode = CODECHAL_VC1_MVMODE_1MV;
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_INTCOMP, value));

        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(CODECHAL_DECODE_VC1_BITS_LUMSCALE + CODECHAL_DECODE_VC1_BITS_LUMSHIFT, value));
        }

        // skipped macroblock bitplane
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());
    }

    uint32_t skipBits = 0;

    if (CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        skipBits += 2;    // 2-bit MBMODETAB
    }
    else
    {
        skipBits += 3;    // 3 bit MBMODETAB
    }

    if (0 == numRef)
    {
        skipBits += 2;    // 2-bit MVTAB
    }
    else
    {
        skipBits += 3;    // 3-bit MVTAB
    }

    skipBits += CODECHAL_DECODE_VC1_BITS_CBPTAB_INTERLACE;

    if (CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        skipBits += CODECHAL_DECODE_VC1_BITS_2MVBPTAB;
    }

    if (CODECHAL_VC1_MVMODE_MIXEDMV == mvMode)
    {
        skipBits += CODECHAL_DECODE_VC1_BITS_4MVBPTAB;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(skipBits, value));

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseVopDquant());

    if (m_vc1PicParams->transform_fields.variable_sized_transform_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTMBF, value));

        if (1 == value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTFRM, value));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_1, value));

    if (0 != value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_2, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSDCTAB, value));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParseFieldPictureLayerBAdvanced()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t value;
    if (CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldBFractionTable, value));
        m_vc1PicParams->b_picture_fraction = (uint8_t)value;
    }

    if (m_vc1PicParams->mv_fields.extended_mv_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseMvRange());
    }

    if (m_vc1PicParams->mv_fields.extended_dmv_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        if (value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            if (value)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
            }
        }
    }

    uint32_t mvMode;
    if (CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseInterlaceMVMode(false, &mvMode));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_INTCOMP, value));

        if (value)
        {
            CODECHAL_DECODE_VERBOSEMESSAGE("INTCOMP is not false.");
        }
        mvMode = CODECHAL_VC1_MVMODE_1MV;

        // direct macroblock bitplane
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());

        // skipped macroblock bitplane
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());
    }

    if (CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        // forward macroblock
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParseBitplane());
    }

    uint32_t skipBits = 0;

    if (CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        skipBits += 2;    // 2-bit MBMODETAB
    }
    else
    {
        skipBits += 3;    // 3 bit MBMODETAB
    }

    if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        skipBits += 2;    // 2-bit MVTAB
    }
    else
    {
        skipBits += 3;    // 3-bit MVTAB
    }

    skipBits += CODECHAL_DECODE_VC1_BITS_CBPTAB_INTERLACE;

    if (CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        skipBits += CODECHAL_DECODE_VC1_BITS_2MVBPTAB;
    }

    if ((CODECHAL_VC1_MVMODE_MIXEDMV == mvMode) ||
        CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        skipBits += CODECHAL_DECODE_VC1_BITS_4MVBPTAB;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(skipBits, value));

    CODECHAL_DECODE_CHK_STATUS_RETURN(ParseVopDquant());

    if (m_vc1PicParams->transform_fields.variable_sized_transform_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTMBF, value));

        if (1 == value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TTFRM, value));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_1, value));

    if (0 != value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSACFRM_2, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TRANSDCTAB, value));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParsePictureHeaderAdvanced()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    bool isIPicture = m_mfxInterface->IsVc1IPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isPPicture = m_mfxInterface->IsVc1PPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isBPicture = m_mfxInterface->IsVc1BPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isBIPicture = m_mfxInterface->IsVc1BIPicture(
                           m_vc1PicParams->CurrPic,
                           m_vc1PicParams->picture_fields.is_first_field,
                           m_vc1PicParams->picture_fields.picture_type)
                           ? true
                           : false;

    uint32_t value;
    if (m_vc1PicParams->sequence_fields.interlace)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_FCM_1, value));
        if (0 != value)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_FCM_2, value));
        }
    }

    if (CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_FPTYPE, value));
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldPictureTypeTable, value));
    }

    if (m_vc1PicParams->sequence_fields.tfcntrflag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TFCNTR, value));
    }

    uint32_t repeatFrameCount = 0, numPanScanWindows, skipBits;
    uint32_t repeatFirstField = 0;
    if (m_vc1PicParams->sequence_fields.pulldown)
    {
        if (!m_vc1PicParams->sequence_fields.interlace)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_RPTFRM, value));

            repeatFrameCount = value;
        }
        else
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_TFF, value));
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_RFF, value));

            repeatFirstField = value;
        }
    }

    if (m_vc1PicParams->entrypoint_fields.panscan_flag)
    {
        // parse PANSCAN
        if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
        {
            if (!m_vc1PicParams->sequence_fields.pulldown)
            {
                numPanScanWindows = 1;
            }
            else
            {
                numPanScanWindows = 1 + repeatFrameCount;
            }
        }
        else
        {
            if (!m_vc1PicParams->sequence_fields.pulldown)
            {
                numPanScanWindows = 2;
            }
            else
            {
                numPanScanWindows = 2 + repeatFirstField;
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_PS_PRESENT, value));

        if (value)
        {
            skipBits = CODECHAL_DECODE_VC1_BITS_PS_HOFFSET + CODECHAL_DECODE_VC1_BITS_PS_VOFFSET;
            skipBits += CODECHAL_DECODE_VC1_BITS_PS_WIDTH + CODECHAL_DECODE_VC1_BITS_PS_HEIGHT;
            skipBits = skipBits * numPanScanWindows;

            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipWords(skipBits >> 4, value));
            CODECHAL_DECODE_CHK_STATUS_RETURN(SkipBits(skipBits & 0xF, value));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_RNDCTRL, value));

    if (isIPicture || isBIPicture)
    {
        if (value != 0)
        {
            CODECHAL_DECODE_ASSERTMESSAGE("RNDCTRL is not 0 for I, BI pictures.");
            return MOS_STATUS_UNKNOWN;
        }
    }

    if (m_vc1PicParams->sequence_fields.interlace)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_UVSAMP, value));
    }

    if (m_vc1PicParams->sequence_fields.finterpflag && CodecHal_PictureIsFrame(m_vc1PicParams->CurrPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_INTERPFRM, value));
    }

    if (!CodecHal_PictureIsInterlacedFrame(m_vc1PicParams->CurrPic))
    {
        if (isBPicture ||
            (CodecHal_PictureIsField(m_vc1PicParams->CurrPic) && isBIPicture))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldBFractionTable, value));
            m_vc1PicParams->b_picture_fraction = (uint8_t)value;
        }
    }

    // REFDIST
    if (CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        m_vc1PicParams->reference_fields.reference_distance_flag &&
        m_vc1PicParams->reference_fields.reference_picture_flag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(2, value));

        if (value == 3)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldRefDistTable, value));
        }

        m_vc1PicParams->reference_fields.reference_distance = value;
    }

    // Quantization Params
    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_PQINDEX, value));

    if (8 >= value)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_HALFQP, value));
    }

    if (1 == m_vc1PicParams->pic_quantizer_fields.quantizer)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_PQUANTIZER, value));
    }

    // POSTPROC
    if (m_vc1PicParams->post_processing)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_POSTPROC, value));
    }

    if (!CodecHal_PictureIsFrame(m_vc1PicParams->CurrPic))
    {
        if (isIPicture || isBIPicture)
        {
            eStatus = ParsePictureLayerIAdvanced();
        }
        else if (isPPicture)
        {
            eStatus = ParseFieldPictureLayerPAdvanced();
        }
        else if (isBPicture)
        {
            eStatus = ParseFieldPictureLayerBAdvanced();
        }
    }
    else
    {
        if (isIPicture || isBIPicture)
        {
            eStatus = ParsePictureLayerIAdvanced();
        }
        else if (isPPicture)
        {
            eStatus = ParsePictureLayerPAdvanced();
        }
        else if (isBPicture)
        {
            eStatus = ParsePictureLayerBAdvanced();
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParsePictureHeaderMainSimple()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t value;
    if (m_vc1PicParams->sequence_fields.finterpflag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_INTERPFRM, value));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_FRMCNT, value));

    if (m_vc1PicParams->sequence_fields.rangered)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_RANGEREDFRM, value));
    }

    // picture type
    CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
    if ((0 == value) && (m_vc1PicParams->sequence_fields.max_b_frames > 0))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(1, value));
        if (0 == value)
        {
            // it's B or BI picture, get B fraction
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetVLC(CODECHAL_DECODE_VC1_VldBFractionTable, value));
            m_vc1PicParams->b_picture_fraction = (uint8_t)value;
        }
    }

    // we don't need to parse more since we only want B fraction value

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::GetSliceMbDataOffset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_numSlices == 1)
    {
        return eStatus;
    }

    CodechalResLock ResourceLock(m_osInterface, &m_resDataBuffer);
    auto bitstream = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(bitstream);

    // start from the second slice since HW only need MB data offset for subsequent slices
    uint32_t macroblockOffset = 0;
    for (uint32_t slcCount = 1; slcCount < m_numSlices; slcCount++)
    {
        uint8_t *slice = bitstream + m_vc1SliceParams[slcCount].slice_data_offset;

        // skip start code prefix
        slice += m_vldSliceRecord[slcCount].dwOffset;
        uint32_t length = m_vldSliceRecord[slcCount].dwLength;

        CODECHAL_DECODE_CHK_STATUS_RETURN(InitialiseBitstream(slice, length, true));

        // parse slice header to get PIC_HEADER_FLAG
        uint32_t value;
        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_SC_SUFFIX, value));

        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_SLICE_ADDR, value));

        CODECHAL_DECODE_CHK_STATUS_RETURN(GetBits(CODECHAL_DECODE_VC1_BITS_PIC_HEADER_FLAG, value));

        // parse picture header to get MB data offset if PIC_HEADER_FLAG == true
        if (value)
        {
            if (macroblockOffset == 0)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeaderAdvanced());

                macroblockOffset = m_bitstream.u32ProcessedBitNum +
                                   (CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH << 3);
            }

            m_vc1SliceParams[slcCount].macroblock_offset = macroblockOffset;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::ParsePictureHeader()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    bool isEBDU = m_vc1PicParams->sequence_fields.AdvancedProfileFlag;

    CodechalResLock ResourceLock(m_osInterface, &m_resDataBuffer);
    auto bitstream = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);
    CODECHAL_DECODE_CHK_NULL_RETURN(bitstream);

    uint32_t skippedBytes = 0;
    if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        // skip start code (4-byte)
        skippedBytes = CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH + (CODECHAL_DECODE_VC1_BITS_SC_SUFFIX >> 3);
    }

    bitstream += skippedBytes;
    uint32_t length = m_dataSize - skippedBytes;
    CODECHAL_DECODE_CHK_STATUS_RETURN(InitialiseBitstream(bitstream, length, isEBDU));

    if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeaderAdvanced());
    }
    else
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeaderMainSimple());
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    m_numMacroblocks   = m_picWidthInMb * m_picHeightInMb;
    m_numMacroblocksUv = m_picWidthInMb * (MOS_ALIGN_CEIL(m_picHeightInMb, 2) / 2);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(m_osInterface, &m_resSyncObject));

    CodecHalAllocateDataList(
        m_vc1RefList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1);

    m_vldSliceRecord =
        (PCODECHAL_VC1_VLD_SLICE_RECORD)MOS_AllocAndZeroMemory(m_picHeightInMb * sizeof(CODECHAL_VC1_VLD_SLICE_RECORD));

    // Second level batch buffer for IT mode
    if (m_mode == CODECHAL_DECODE_MODE_VC1IT)
    {
        MOS_ZeroMemory(&m_itObjectBatchBuffer, sizeof(m_itObjectBatchBuffer));

        // Must reserve at least 8 cachelines after MI_BATCH_BUFFER_END_CMD since HW prefetch max 8 cachelines from BB everytime
        uint32_t size = m_standardDecodeSizeNeeded * m_numMacroblocks + m_hwInterface->m_sizeOfCmdBatchBufferEnd + 8 * CODECHAL_CACHELINE_SIZE;

        CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_itObjectBatchBuffer,
            nullptr,
            size));
        m_itObjectBatchBuffer.bSecondLevel = true;
    }

    // Deblocking Filter Row Store Scratch buffer
    //(Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resMfdDeblockingFilterRowStoreScratchBuffer,
                                                  m_picWidthInMb * 7 * CODECHAL_CACHELINE_SIZE,
                                                  "DeblockingScratchBuffer"),
        "Failed to allocate Deblocking Filter Row Store Scratch Buffer.");

    // BSD/MPC Row Store Scratch buffer
    // (FrameWidth in MB) * (2) * (CacheLine size per MB)
    CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                  &m_resBsdMpcRowStoreScratchBuffer,
                                                  m_picWidthInMb * CODECHAL_CACHELINE_SIZE * 2,
                                                  "MpcScratchBuffer"),
        "Failed to allocate BSD/MPC Row Store Scratch Buffer.");

    // VC1 MV buffer, 1 cacheline for every MB
    for (uint32_t i = 0; i < CODECHAL_DECODE_VC1_DMV_MAX; i++)
    {
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resVc1BsdMvData[i],
                                                      CODECHAL_CACHELINE_SIZE * m_numMacroblocks,
                                                      "MvBuffer"),
            "Failed to allocate VC1 BSD MV Buffer.");
    }

    // Bitplane buffer
    // (Bitplane buffer pitch) * (Height in Macroblock)
    uint32_t size;
    if (m_shortFormatInUse)
    {
        if (m_width <= 2048)
        {
            size = MHW_VDBOX_VC1_BITPLANE_BUFFER_PITCH_SMALL * m_picHeightInMb;
        }
        else
        {
            size = MHW_VDBOX_VC1_BITPLANE_BUFFER_PITCH_LARGE * m_picHeightInMb;
        }

        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resBitplaneBuffer,
                                                      size,
                                                      "BitplaneBuffer"),
            "Failed to allocate Bitplane Buffer.");
    }

    // For SP/MP short format
    // Private bitstream buffer
    // FrameWidth * FrameHeight * 1.5 + CODECHAL_DECODE_VC1_STUFFING_BYTES
    if (m_shortFormatInUse)
    {
        size = m_width * m_height * 3 / 2 + CODECHAL_DECODE_VC1_STUFFING_BYTES;
        m_privateBistreamBufferSize = size;
        CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateBuffer(
                                                      &m_resPrivateBistreamBuffer,
                                                      size,
                                                      "PrivateBistreamBuffer"),
            "Failed to allocate BSD/MPC Row Store Scratch Buffer.");
    }

    m_unequalFieldWaInUse = (MEDIA_IS_WA(m_waTable, WaVC1UnequalFieldHeights) && (m_picHeightInMb % 2));

    if (m_unequalFieldWaInUse)
    {
        // Decoded frame surface
        for (uint32_t i = 0; i < CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES; i++)
        {
            // Error Frame is 1MB x 2MB
            CODECHAL_DECODE_CHK_STATUS_MESSAGE_RETURN(AllocateSurface(
                                                          &m_unequalFieldSurface[i],
                                                          m_width,
                                                          m_height + MOS_YTILE_H_ALIGNMENT,
                                                          "Vc1UnequalFieldSurface"),
                "Failed to allocate VC1 Unequal Fields WA decoding ouput surface data buffer.");

            // ensure that no entries are valid
            m_unequalFieldRefListIdx[i] = CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1;
        }

        m_unequalFieldSurfaceForBType = CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES - 1;
        m_currUnequalFieldSurface     = 0;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectWaContextInUse));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateSyncResource(
        m_osInterface,
        &m_resSyncObjectVideoContextInUse));

    return (MOS_STATUS)eStatus;
}

CodechalDecodeVc1::~CodechalDecodeVc1()
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    m_osInterface->pfnDestroySyncResource(m_osInterface, &m_resSyncObject);

    CodecHalFreeDataList(m_vc1RefList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VC1);

    MOS_FreeMemory(m_vldSliceRecord);

    Mhw_FreeBb(m_osInterface, &m_itObjectBatchBuffer, nullptr);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resMfdDeblockingFilterRowStoreScratchBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resBsdMpcRowStoreScratchBuffer);

    for (uint32_t i = 0; i < CODECHAL_DECODE_VC1_DMV_MAX; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resVc1BsdMvData[i]);
    }

    if (m_shortFormatInUse)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resBitplaneBuffer);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &m_resPrivateBistreamBuffer);

    if (m_unequalFieldWaInUse)
    {
        for (uint32_t i = 0; i < CODECHAL_DECODE_VC1_UNEQUAL_FIELD_WA_SURFACES; i++)
        {
            // Error Frame is 1MB x 2MB
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_unequalFieldSurface[i].OsResource);
            ;
        }
    }

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &m_resSyncObjectWaContextInUse);

    m_osInterface->pfnDestroySyncResource(
        m_osInterface,
        &m_resSyncObjectVideoContextInUse);
}

MOS_STATUS CodechalDecodeVc1::SetFrameStates()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_destSurface);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_decodeParams.m_dataBuffer);

    m_dataSize          = m_decodeParams.m_dataSize;
    m_dataOffset        = m_decodeParams.m_dataOffset;
    m_numSlices         = m_decodeParams.m_numSlices;
    m_numMacroblocks    = m_decodeParams.m_numMacroblocks;
    m_vc1PicParams      = (PCODEC_VC1_PIC_PARAMS)(m_decodeParams.m_picParams);
    m_vc1SliceParams    = (PCODEC_VC1_SLICE_PARAMS)(m_decodeParams.m_sliceParams);
    m_vc1MbParams       = (PCODEC_VC1_MB_PARAMS)(m_decodeParams.m_macroblockParams);
    m_destSurface       = *(m_decodeParams.m_destSurface);
    m_resDataBuffer     = *(m_decodeParams.m_dataBuffer);
    m_deblockDataBuffer = m_decodeParams.m_deblockData;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1PicParams);

    if (m_vc1PicParams->coded_width > m_destSurface.dwPitch ||
        m_vc1PicParams->coded_height > m_destSurface.dwHeight)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (CodecHalIsDecodeModeVLD(m_mode))
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1SliceParams);
    }
    else if (CodecHalIsDecodeModeIT(m_mode))
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1MbParams);

        // Catch the case the codec does not send a deblocking surface, but requests ILDB
        if (m_deblockDataBuffer == nullptr)
        {
            m_vc1PicParams->entrypoint_fields.loopfilter = 0;
        }
    }

    // For short format, check if it is skipped frame.
    if (m_shortFormatInUse)
    {
        m_numMacroblocks = m_picWidthInMb * m_picHeightInMb;

        if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
        {
            if ((m_vc1SliceParams->macroblock_offset == 0xFFFF) &&
                (m_vc1SliceParams->number_macroblocks == m_numMacroblocks))
            {
                m_vc1PicParams->picture_fields.picture_type = vc1SkippedFrame;
            }
        }
        else // Simple or Main Profiles
        {
            if (((m_vc1SliceParams->slice_data_size == 0) ||
                    (m_vc1SliceParams->slice_data_size == 8)) &&
                (m_vc1SliceParams->number_macroblocks == m_numMacroblocks))
            {
                m_vc1PicParams->picture_fields.picture_type = vc1SkippedFrame;
            }
        }
    }

    PCODEC_REF_LIST destEntry = m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx];
    uint16_t        picType   = (uint16_t)m_vc1PicParams->picture_fields.picture_type;
    CODEC_PICTURE   currPic   = m_vc1PicParams->CurrPic;

    if (!CodecHal_PictureIsField(currPic) ||
        (CodecHal_PictureIsField(currPic) && m_vc1PicParams->picture_fields.is_first_field))
    {
        MOS_ZeroMemory(destEntry, sizeof(CODEC_REF_LIST));
        destEntry->RefPic = currPic;
        destEntry->resRefPic = m_destSurface.OsResource;
    }
    if (!m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        if (m_vc1PicParams->range_mapping_fields.range_mapping_enabled)
        {
            MOS_BIT_ON(destEntry->dwRefSurfaceFlags, CODECHAL_WMV9_RANGE_ADJUSTMENT);
        }
        else
        {
            MOS_BIT_OFF(destEntry->dwRefSurfaceFlags, CODECHAL_WMV9_RANGE_ADJUSTMENT);
        }
    }

    if (CodecHal_PictureIsFrame(currPic))
    {
        MOS_BIT_ON(destEntry->dwRefSurfaceFlags, CODECHAL_VC1_PROGRESSIVE);
    }

    m_statusReportFeedbackNumber = m_vc1PicParams->StatusReportFeedbackNumber;

    m_deblockingEnabled = m_vc1PicParams->entrypoint_fields.loopfilter;
    m_width             = m_vc1PicParams->coded_width;
    m_height            = m_vc1PicParams->coded_height;
    m_picWidthInMb =
        ((uint16_t)m_width + CODECHAL_MACROBLOCK_WIDTH - 1) / CODECHAL_MACROBLOCK_WIDTH;
    m_picHeightInMb =
        ((uint16_t)m_height + CODECHAL_MACROBLOCK_HEIGHT - 1) / CODECHAL_MACROBLOCK_HEIGHT;

    if (CodecHal_PictureIsField(currPic) && (m_picHeightInMb % 2))
    {
        m_vc1OddFrameHeight = true;
    }
    else
    {
        m_vc1OddFrameHeight = false;
    }

    // Overwrite the actual surface height with the coded height and width of the frame
    // for VC1 since it's possible for a VC1 frame to change size during playback
    m_destSurface.dwWidth  = m_width;
    m_destSurface.dwHeight = m_height;

    bool bOLPParamsAvailable =
        m_vc1PicParams->range_mapping_fields.range_mapping_enabled || m_vc1PicParams->UpsamplingFlag;

    if (m_decodeParams.m_deblockSurface &&
        ((m_vc1PicParams->DeblockedPicIdx != m_vc1PicParams->CurrPic.FrameIdx) || bOLPParamsAvailable) &&
        !(CodecHal_PictureIsField(currPic) && m_vc1PicParams->picture_fields.is_first_field))
    {
        m_olpNeeded      = true;
        m_deblockSurface = *(m_decodeParams.m_deblockSurface);
    }

    if (m_decodeParams.m_vc1BitplaneSize == 0)
    {
        m_vc1PicParams->raw_coding.bitplane_present = 0;
    }

    if (m_vc1PicParams->raw_coding.bitplane_present)
    {
        m_resBitplaneBuffer = *(m_decodeParams.m_bitplaneBuffer);
    }

    bool pictureIsI = m_mfxInterface->IsVc1IPicture(currPic, m_vc1PicParams->picture_fields.is_first_field, picType) ? true : false;
    bool pictureIsP = m_mfxInterface->IsVc1PPicture(currPic, m_vc1PicParams->picture_fields.is_first_field, picType) ? true : false;
    bool pictureIsB =
        (m_mfxInterface->IsVc1BPicture(currPic, m_vc1PicParams->picture_fields.is_first_field, picType) |
            m_mfxInterface->IsVc1BIPicture(currPic, m_vc1PicParams->picture_fields.is_first_field, picType))
            ? true
            : false;

    // Save anchor picture type and field structure (TFF/BFF)
    if (!pictureIsB)
    {
        m_prevAnchorPictureTff = (uint16_t)m_vc1PicParams->picture_fields.top_field_first;
        if (CodecHal_PictureIsBottomField(currPic))
        {
            m_prevOddAnchorPictureIsP = pictureIsP;
        }
        else
        {
            m_prevEvenAnchorPictureIsP = pictureIsP;
        }
    }

    if (m_unequalFieldWaInUse && CodecHal_PictureIsField(currPic))
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeUnequalFieldSurface(
            (uint8_t)m_vc1PicParams->CurrPic.FrameIdx,
            m_renderContextUsesNullHw));
        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeUnequalFieldSurface(
            (uint8_t)m_vc1PicParams->ForwardRefIdx,
            m_renderContextUsesNullHw));
        CODECHAL_DECODE_CHK_STATUS_RETURN(InitializeUnequalFieldSurface(
            (uint8_t)m_vc1PicParams->BackwardRefIdx,
            m_renderContextUsesNullHw));
    }

    m_perfType = pictureIsI ? I_TYPE : (pictureIsP ? P_TYPE : B_TYPE);

    m_crrPic = currPic;
    m_secondField = (m_vc1PicParams->picture_fields.is_first_field == 1) ? false : true;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_NULL_RETURN(m_debugInterface);
        m_debugInterface->m_currPic     = m_crrPic;
        m_debugInterface->m_secondField = m_secondField;
        m_debugInterface->m_frameType   = m_perfType;

        if (m_vc1PicParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpPicParams(
                m_vc1PicParams));
        }

        if (m_vc1SliceParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpSliceParams(
                m_vc1SliceParams));
        }

        if (m_vc1MbParams) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(DumpMbParams(
                m_vc1MbParams));
        }

        if (m_deblockDataBuffer) {
            //Dump decode deblocking
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpData(
                m_deblockDataBuffer,
                m_decodeParams.m_deblockDataSize,
                CodechalDbgAttr::attrDeblocking,
                "_DEC"));
        }

        if (m_decodeParams.m_vc1BitplaneSize != 0) {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resBitplaneBuffer,
                CodechalDbgAttr::attrVc1Bitplane,
                "_DEC",
                m_decodeParams.m_vc1BitplaneSize));

        })

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::DecodeStateLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PCODEC_REF_LIST     *vc1RefList;
    vc1RefList = &(m_vc1RefList[0]);

    uint8_t destIdx   = m_vc1PicParams->CurrPic.FrameIdx;
    uint8_t fwdRefIdx = (uint8_t)m_vc1PicParams->ForwardRefIdx;
    uint8_t bwdRefIdx = (uint8_t)m_vc1PicParams->BackwardRefIdx;

    bool isIPicture = m_mfxInterface->IsVc1IPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isPPicture = m_mfxInterface->IsVc1PPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;
    bool isBPicture = m_mfxInterface->IsVc1BPicture(
                          m_vc1PicParams->CurrPic,
                          m_vc1PicParams->picture_fields.is_first_field,
                          m_vc1PicParams->picture_fields.picture_type)
                          ? true
                          : false;

    PMOS_SURFACE    destSurface;
    PMOS_RESOURCE   fwdRefSurface, bwdRefSurface;
    if (m_unequalFieldWaInUse && CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        destSurface =
            &(m_unequalFieldSurface[vc1RefList[destIdx]->dwUnequalFieldSurfaceIdx]);
        fwdRefSurface =
            &(m_unequalFieldSurface[vc1RefList[fwdRefIdx]->dwUnequalFieldSurfaceIdx].OsResource);
        bwdRefSurface =
            &(m_unequalFieldSurface[vc1RefList[bwdRefIdx]->dwUnequalFieldSurfaceIdx].OsResource);

        // Overwrite the actual surface height with the coded height and width of the frame
        // for VC1 since it's possible for a VC1 frame to change size during playback
        destSurface->dwWidth = m_width;
        destSurface->dwHeight = m_height;
    }
    else
    {
        destSurface   = &m_destSurface;
        fwdRefSurface = &(vc1RefList[fwdRefIdx]->resRefPic);
        bwdRefSurface = &(vc1RefList[bwdRefIdx]->resRefPic);
    }

    // For SP/MP short format
    if (m_shortFormatInUse &&
        !m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ConstructBistreamBuffer());
    }

    MOS_COMMAND_BUFFER  cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(m_vdboxIndex);
    HalOcaInterface::On1stLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bStreamOutEnabled = m_streamOutEnabled;
    pipeModeSelectParams.bPostDeblockOutEnable = m_deblockingEnabled;
    pipeModeSelectParams.bPreDeblockOutEnable  = !m_deblockingEnabled;
    pipeModeSelectParams.bShortFormatInUse     = m_shortFormatInUse;
    pipeModeSelectParams.bVC1OddFrameHeight    = m_vc1OddFrameHeight;

    MHW_VDBOX_SURFACE_PARAMS    surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;
    surfaceParams.psSurface = destSurface;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS  pipeBufAddrParams;
    pipeBufAddrParams.Mode = m_mode;
    if (m_deblockingEnabled)
    {
        pipeBufAddrParams.psPostDeblockSurface = destSurface;
    }
    else
    {
        pipeBufAddrParams.psPreDeblockSurface = destSurface;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetPipeBufAddr(&pipeBufAddrParams));
#endif

    // when there is not a forward or backward reference,
    // the index is set to the destination frame index
    m_presReferences[CodechalDecodeFwdRefTop] =
        m_presReferences[CodechalDecodeFwdRefBottom] =
            fwdRefSurface;
    m_presReferences[CodechalDecodeBwdRefTop] =
        m_presReferences[CodechalDecodeBwdRefBottom] =
            bwdRefSurface;
    // special case for second fields
    if (!m_vc1PicParams->picture_fields.is_first_field &&
        !m_mfxInterface->IsVc1IPicture(
            m_vc1PicParams->CurrPic,
            m_vc1PicParams->picture_fields.is_first_field,
            m_vc1PicParams->picture_fields.picture_type))
    {
        if (m_vc1PicParams->picture_fields.top_field_first)
        {
            m_presReferences[CodechalDecodeFwdRefTop] =
                &destSurface->OsResource;
        }
        else
        {
            m_presReferences[CodechalDecodeFwdRefBottom] =
                &destSurface->OsResource;
        }
    }

    // set all ref pic addresses to valid addresses for error concealment purpose
    for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
    {
        if (m_presReferences[i] == nullptr && 
            MEDIA_IS_WA(m_waTable, WaDummyReference) && 
            !Mos_ResourceIsNull(&m_dummyReference.OsResource))
        {
            m_presReferences[i] = &m_dummyReference.OsResource;
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(pipeBufAddrParams.presReferences, sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC, m_presReferences, sizeof(PMOS_RESOURCE) * CODEC_MAX_NUM_REF_FRAME_NON_AVC));

    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &m_resMfdDeblockingFilterRowStoreScratchBuffer;

    if (m_streamOutEnabled)
    {
        pipeBufAddrParams.presStreamOutBuffer =
            &(m_streamOutBuffer[m_streamOutCurrBufIdx]);
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->CheckReferenceList(&pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->SetRefrenceSync(m_disableDecodeSyncLock, m_disableLockForTranscode));
#endif

    CODECHAL_DEBUG_TOOL(
        for (int i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
        {
            if (pipeBufAddrParams.presReferences[i])
            {
                MOS_SURFACE dstSurface;

                MOS_ZeroMemory(&dstSurface, sizeof(MOS_SURFACE));
                dstSurface.Format = Format_NV12;
                dstSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
                CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &dstSurface));

                m_debugInterface->m_refIndex = (uint16_t)i;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &dstSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data()));
            }
        }
    )

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS      indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = m_mode;
    if (m_shortFormatInUse &&
        !m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        indObjBaseAddrParams.dwDataSize     = m_dataSize + CODECHAL_DECODE_VC1_STUFFING_BYTES;
        indObjBaseAddrParams.presDataBuffer = &m_resPrivateBistreamBuffer;
    }
    else
    {
        indObjBaseAddrParams.dwDataSize     = m_dataSize;
        indObjBaseAddrParams.presDataBuffer = &m_resDataBuffer;
    }

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS  bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer;

    if (m_vc1PicParams->raw_coding.bitplane_present || m_shortFormatInUse)
    {
        bspBufBaseAddrParams.presBitplaneBuffer = &m_resBitplaneBuffer;
    }

    MHW_VDBOX_VC1_PRED_PIPE_PARAMS  vc1PredPipeParams;
    vc1PredPipeParams.pVc1PicParams = m_vc1PicParams;
    vc1PredPipeParams.ppVc1RefList = vc1RefList;

    MHW_VDBOX_VC1_PIC_STATE vc1PicState;
    vc1PicState.pVc1PicParams             = m_vc1PicParams;
    vc1PicState.Mode = m_mode;
    vc1PicState.ppVc1RefList = vc1RefList;
    vc1PicState.wPrevAnchorPictureTFF     = m_prevAnchorPictureTff;
    vc1PicState.bPrevEvenAnchorPictureIsP = m_prevEvenAnchorPictureIsP;
    vc1PicState.bPrevOddAnchorPictureIsP  = m_prevOddAnchorPictureIsP;

    if (m_shortFormatInUse)
    {
        // APP does not provide REFDIST for I/P pictures correctly
        if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag &&
            CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
            (isIPicture || isPPicture) &&
            m_vc1PicParams->reference_fields.reference_distance_flag)
        {
            if (m_vc1PicParams->picture_fields.is_first_field)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeader());
                m_referenceDistance = m_vc1PicParams->reference_fields.reference_distance;
            }
            else
            {
                m_vc1PicParams->reference_fields.reference_distance = m_referenceDistance;
            }
        }

        // APP does not provide BFRACTION correctly. So parse picture header to get BFRACTION
        if (isBPicture)
        {
            if (m_vc1PicParams->picture_fields.is_first_field)
            {
                CODECHAL_DECODE_CHK_STATUS_RETURN(ParsePictureHeader());
            }
        }
    }

    MHW_VDBOX_VC1_DIRECTMODE_PARAMS vc1DirectmodeParams;
    if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        uint8_t dmvBufferIdx                   = (m_vc1PicParams->CurrPic.PicFlags == PICTURE_BOTTOM_FIELD) ? CODECHAL_DECODE_VC1_DMV_ODD : CODECHAL_DECODE_VC1_DMV_EVEN;
        vc1DirectmodeParams.presDmvReadBuffer  = &m_resVc1BsdMvData[dmvBufferIdx];
        vc1DirectmodeParams.presDmvWriteBuffer = &m_resVc1BsdMvData[dmvBufferIdx];
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, !m_olpNeeded));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer));
    }

    if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        m_vc1PicParams->picture_fields.picture_type == vc1SkippedFrame)
    {
        // no further picture level commands needed for skipped frames
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        return eStatus;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVc1PredPipeCmd(&cmdBuffer, &vc1PredPipeParams));

    if (m_intelEntrypointInUse || m_mode == CODECHAL_DECODE_MODE_VC1IT)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVc1LongPicCmd(&cmdBuffer, &vc1PicState));
    }
    else if (m_shortFormatInUse)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ShortPicCmd(&cmdBuffer, &vc1PicState));
    }
    else
    {
        CODECHAL_DECODE_ASSERTMESSAGE("Unsupported decode mode.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxVc1DirectmodeCmd(&cmdBuffer, &vc1DirectmodeParams));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::DecodePrimitiveLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    if (m_mode == CODECHAL_DECODE_MODE_VC1IT)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(DecodePrimitiveLevelIT());
    }
    else if (m_mode == CODECHAL_DECODE_MODE_VC1VLD)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(DecodePrimitiveLevelVLD())
    }
    else
    {
        return MOS_STATUS_UNKNOWN;
    }

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::DecodePrimitiveLevelVLD()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_SYNC_PARAMS syncParams;

    // static VC1 slice parameters
    MHW_VDBOX_VC1_SLICE_STATE vc1SliceState;
    vc1SliceState.presDataBuffer = &m_resDataBuffer;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        m_vc1PicParams->picture_fields.picture_type == vc1SkippedFrame)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(HandleSkipFrame());
        goto submit;
    }
    else
    {
        PCODEC_VC1_SLICE_PARAMS slc             = m_vc1SliceParams;
        bool firstValidSlice = true;
        int prevValidSlc = 0;
        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            m_vldSliceRecord[slcCount].dwSliceYOffset     = slc->slice_vertical_position;
            m_vldSliceRecord[slcCount].dwNextSliceYOffset = frameFieldHeightInMb;  // init to last slice

            int32_t length = slc->slice_data_size >> 3;
            int32_t offset = slc->macroblock_offset >> 3;

            CodechalResLock ResourceLock(m_osInterface, &m_resDataBuffer);
            auto buf = (uint8_t*)ResourceLock.Lock(CodechalResLock::readOnly);
            buf += slc->slice_data_offset;
            if (offset > 3 && buf != nullptr &&
                m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
            {
                int i = 0;
                int j = 0;
                for (i = 0, j = 0; i < offset - 1; i++, j++)
                {
                    if (!buf[j] && !buf[j + 1] && buf[j + 2] == 3 && buf[j + 3] < 4)
                    {
                        i++, j += 2;
                    }
                }
                if (i == offset - 1)
                {
                    if (!buf[j] && !buf[j + 1] && buf[j + 2] == 3 && buf[j + 3] < 4)
                    {
                        buf[j + 2] = 0;
                        j++;
                    }
                    j++;
                }
                offset = (8 * j + slc->macroblock_offset % 8)>>3;
            }

            // Check that the slice data does not overrun the bitstream buffer size
            if (((uintptr_t)(slc->slice_data_offset) + length) > m_dataSize)
            {
                length = m_dataSize - (uintptr_t)(slc->slice_data_offset);

                if (length < 0)
                {
                    length = 0;
                }
            }

            // Error handling for garbage data
            if (((uintptr_t)(slc->slice_data_offset)) > m_dataSize)
            {
                slc++;
                m_vldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            // Check offset not larger than slice length, can have slice length of 0
            if (offset > length)
            {
                slc++;
                m_vldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            // Check that the slices do not overlap, else do not send the lower slice
            if (!firstValidSlice &&
                (m_vldSliceRecord[slcCount].dwSliceYOffset <= m_vldSliceRecord[prevValidSlc].dwSliceYOffset))
            {
                slc++;
                m_vldSliceRecord[slcCount].dwSkip = true;
                continue;
            }

            if (firstValidSlice)
            {
                // Ensure that the first slice starts from 0
                m_vldSliceRecord[slcCount].dwSliceYOffset = 0;
                slc->slice_vertical_position = 0;
            }
            else
            {
                // Set next slice start Y offset of previous slice
                m_vldSliceRecord[prevValidSlc].dwNextSliceYOffset =
                    m_vldSliceRecord[slcCount].dwSliceYOffset;
            }

            if (m_shortFormatInUse)
            {
                if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
                {
                    if ((slc->macroblock_offset >> 3) < CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH)
                    {
                        slc++;
                        m_vldSliceRecord[slcCount].dwSkip = true;
                        continue;
                    }

                    // set macroblock_offset of the first slice to 0 match HW expectations.
                    if (slcCount == 0)
                    {
                        slc->macroblock_offset = CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH << 3;
                    }

                    offset = CODECHAL_DECODE_VC1_SC_PREFIX_LENGTH;
                }
                else // Simple Profile or Main Profile
                {
                    {
                        offset = CODECHAL_DECODE_VC1_STUFFING_BYTES - 1;
                        length += CODECHAL_DECODE_VC1_STUFFING_BYTES;
                        slc->macroblock_offset += CODECHAL_DECODE_VC1_STUFFING_BYTES << 3;
                        slc->macroblock_offset &= (~0x7); // Clear bit offset of first MB for short format
                    }
                }
            }

            m_vldSliceRecord[slcCount].dwOffset = offset;
            m_vldSliceRecord[slcCount].dwLength = length - offset;
            firstValidSlice = false;
            prevValidSlc = slcCount;
            slc++;
        }

        if (m_shortFormatInUse &&
            m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(GetSliceMbDataOffset());
        }

        // Reset slc pointer
        slc -= m_numSlices;

        //------------------------------------
        // Fill BSD Object Commands
        //------------------------------------
        for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
        {
            if (m_vldSliceRecord[slcCount].dwSkip)
            {
                slc++;
                continue;
            }

            vc1SliceState.pSlc = slc;
            vc1SliceState.dwOffset               = m_vldSliceRecord[slcCount].dwOffset;
            vc1SliceState.dwLength               = m_vldSliceRecord[slcCount].dwLength;
            vc1SliceState.dwNextVerticalPosition = m_vldSliceRecord[slcCount].dwNextSliceYOffset;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1BsdObjectCmd(&cmdBuffer, &vc1SliceState));

            slc++;
        }

        // Free VLD slice record
        MOS_ZeroMemory(m_vldSliceRecord, (m_numSlices * sizeof(CODECHAL_VC1_VLD_SLICE_RECORD)));
    }

    // Check if destination surface needs to be synchronized
    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        !m_vc1PicParams->picture_fields.is_first_field)
    {
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    }
    else
    {
        // Check if destination surface needs to be synchronized
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly = false;
        syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) ||
            m_vc1PicParams->picture_fields.is_first_field)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            !(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) && m_vc1PicParams->picture_fields.is_first_field))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
        }
    }

submit:
    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vc1PicParams->CurrPic;
        if (m_olpNeeded)
        {
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_currDeblockedPic.FrameIdx = (uint8_t)m_vc1PicParams->DeblockedPicIdx;
                decodeStatusReport.m_currDeblockedPic.PicFlags = PICTURE_FRAME;)
            decodeStatusReport.m_deblockedPicResOlp = m_deblockSurface.OsResource;
        }
        else
        {
            decodeStatusReport.m_currDeblockedPic = m_vc1PicParams->CurrPic;
        }
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes = m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic;

        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField =
                (m_vc1PicParams->picture_fields.is_first_field == 1) ? false : true;
            decodeStatusReport.m_olpNeeded = m_olpNeeded;
            decodeStatusReport.m_frameType = m_perfType;)

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));

    //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
    //    m_debugInterface,
    //    &cmdBuffer));
    )

    if (m_huCCopyInUse)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        m_huCCopyInUse = false;
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        !m_vc1PicParams->picture_fields.is_first_field)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1RefList);

        uint32_t destFrameIdx = m_vc1PicParams->CurrPic.FrameIdx;

        CODECHAL_DECODE_CHK_STATUS_RETURN(FormatUnequalFieldPicture(
            m_unequalFieldSurface[m_vc1RefList[destFrameIdx]->dwUnequalFieldSurfaceIdx],
            m_destSurface,
            true,
            m_videoContextUsesNullHw));
    }

    if (m_olpNeeded)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(PerformVc1Olp());
    }
    else
    {
        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic = m_destSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
            m_vc1PicParams->picture_fields.is_first_field))
    {
        MOS_SYNC_PARAMS syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

        if (m_olpNeeded)
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_deblockSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }

    m_olpNeeded = false;

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::DecodePrimitiveLevelIT()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_SYNC_PARAMS syncParams;

    PCODEC_VC1_MB_PARAMS mb = m_vc1MbParams;

    MHW_VDBOX_VC1_MB_STATE vc1MbState;
    MOS_ZeroMemory(&vc1MbState, sizeof(vc1MbState));

    // static VC1 MB parameters
    vc1MbState.presDataBuffer     = &m_resDataBuffer;
    vc1MbState.pVc1PicParams      = m_vc1PicParams;
    vc1MbState.pWaTable = m_waTable;
    vc1MbState.pDeblockDataBuffer = m_deblockDataBuffer;
    vc1MbState.dwDataSize         = m_dataSize;
    vc1MbState.wPicWidthInMb      = m_picWidthInMb;
    vc1MbState.wPicHeightInMb     = m_picHeightInMb;
    vc1MbState.PicFlags           = m_vc1PicParams->CurrPic.PicFlags;
    vc1MbState.bFieldPolarity     = m_fieldPolarity;

    uint16_t frameFieldHeightInMb;
    CodecHal_GetFrameFieldHeightInMb(
        m_vc1PicParams->CurrPic,
        m_picHeightInMb,
        frameFieldHeightInMb);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_itObjectBatchBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_LockBb(m_osInterface, &m_itObjectBatchBuffer));

    PMHW_BATCH_BUFFER batchBuffer = &m_itObjectBatchBuffer;

    uint32_t mbAddress = 0;
    uint32_t mbCount;
    for (mbCount = 0; mbCount < m_numMacroblocks; mbCount++)
    {
        vc1MbState.pMb = mb + mbCount;

        // Skipped MBs before current MB
        uint16_t skippedMBs = (mbCount) ?
            (mb[mbCount].mb_address - mb[mbCount - 1].mb_address - 1) :
            (mb[mbCount].mb_address);

        while (skippedMBs--)
        {
            vc1MbState.bMbHorizOrigin = (uint8_t)(mbAddress % m_picWidthInMb);
            vc1MbState.bMbVertOrigin  = (uint8_t)(mbAddress / m_picWidthInMb);
            vc1MbState.bSkipped = true;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ItObjectCmd(batchBuffer, &vc1MbState));

            mbAddress++;
        }

        // Current MB
        if (mbCount + 1 == m_numMacroblocks)
        {
            vc1MbState.dwLength = m_dataSize - mb[mbCount].data_offset;
        }
        else
        {
            vc1MbState.dwLength = mb[mbCount + 1].data_offset - mb[mbCount].data_offset;
        }

        vc1MbState.bMbHorizOrigin = mb[mbCount].mb_address % m_picWidthInMb;
        vc1MbState.bMbVertOrigin  = mb[mbCount].mb_address / m_picWidthInMb;
        vc1MbState.dwOffset = (vc1MbState.dwLength) ? mb[mbCount].data_offset : 0;
        vc1MbState.bSkipped = false;

        if (m_vc1PicParams->entrypoint_fields.loopfilter)
        {
            eStatus = MOS_SecureMemcpy(vc1MbState.DeblockData,
                CODEC_NUM_BLOCK_PER_MB,
                m_deblockDataBuffer + CODEC_NUM_BLOCK_PER_MB * mb[mbCount].mb_address,
                CODEC_NUM_BLOCK_PER_MB);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_DECODE_ASSERTMESSAGE("Failed to copy memory.");
                m_olpNeeded = false;
                return eStatus;
            }
        }

        if (!mb[mbCount].mb_type.intra_mb)
        {
            if (mb[mbCount].mb_type.motion_forward || mb[mbCount].mb_type.motion_backward)
            {
                PackMotionVectors(
                    &vc1MbState,
                    (int16_t *)mb[mbCount].motion_vector,
                    (int16_t *)vc1MbState.PackedLumaMvs,
                    (int16_t *)&vc1MbState.PackedChromaMv);
            }
            else
            {
                mb[mbCount].mb_type.motion_forward = 1;
                MOS_ZeroMemory(vc1MbState.PackedLumaMvs, sizeof(vc1MbState.PackedLumaMvs)); // MV's of zero
                vc1MbState.bMotionSwitch = 0;
            }
        }

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ItObjectCmd(batchBuffer, &vc1MbState));

        mbAddress = mb[mbCount].mb_address;
    }

    m_fieldPolarity = vc1MbState.bFieldPolarity;

    // skipped MBs at the end
    uint16_t skippedMBs = m_picWidthInMb * frameFieldHeightInMb - mb[mbCount - 1].mb_address - 1;

    while (skippedMBs--)
    {
        vc1MbState.bSkipped = true;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfdVc1ItObjectCmd(batchBuffer, &vc1MbState));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &m_itObjectBatchBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
            batchBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, &m_itObjectBatchBuffer, true));

    // Check if destination surface needs to be synchronized
    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic))
    {
        if (!m_vc1PicParams->picture_fields.is_first_field)
        {
            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
        }
    }
    else
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource         = &m_destSurface.OsResource;
        syncParams.bReadOnly = false;
        syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
        syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

        if (!CodecHal_PictureIsField(m_vc1PicParams->CurrPic) ||
            m_vc1PicParams->picture_fields.is_first_field)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

            // Update the resource tag (s/w tag) for On-Demand Sync
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }

        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        // Update the tag in GPU Sync eStatus buffer (H/W Tag) to match the current S/W tag
        if (m_osInterface->bTagResourceSync &&
            !(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) && m_vc1PicParams->picture_fields.is_first_field))
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
        }
    }

    if (m_statusQueryReportingEnabled)
    {
        CodechalDecodeStatusReport decodeStatusReport;

        decodeStatusReport.m_statusReportNumber = m_statusReportFeedbackNumber;
        decodeStatusReport.m_currDecodedPic     = m_vc1PicParams->CurrPic;
        if (m_olpNeeded)
        {
            CODECHAL_DEBUG_TOOL(
                decodeStatusReport.m_currDeblockedPic.FrameIdx = (uint8_t)m_vc1PicParams->DeblockedPicIdx;
                decodeStatusReport.m_currDeblockedPic.PicFlags = PICTURE_FRAME;)
            decodeStatusReport.m_deblockedPicResOlp = m_deblockSurface.OsResource;
        }
        else
        {
            decodeStatusReport.m_currDeblockedPic = m_vc1PicParams->CurrPic;
        }
        decodeStatusReport.m_codecStatus = CODECHAL_STATUS_UNAVAILABLE;
        decodeStatusReport.m_currDecodedPicRes = m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic;

        CODECHAL_DEBUG_TOOL(
            decodeStatusReport.m_secondField =
                (m_vc1PicParams->picture_fields.is_first_field == 1) ? false : true;
            decodeStatusReport.m_olpNeeded = m_olpNeeded;
            decodeStatusReport.m_frameType = m_perfType;)

        CODECHAL_DECODE_CHK_STATUS_RETURN(EndStatusReport(decodeStatusReport, &cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            "_DEC"));

        //CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
        //    m_debugInterface,
        //    &cmdBuffer));
    )

    if (m_huCCopyInUse)
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContextForWa;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectWaContextInUse;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

        m_huCCopyInUse = false;
    }

    HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_videoContextUsesNullHw));

    CODECHAL_DEBUG_TOOL(
        m_mmc->UpdateUserFeatureKey(&m_destSurface);)

    if (m_unequalFieldWaInUse &&
        CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
        !m_vc1PicParams->picture_fields.is_first_field)
    {
        CODECHAL_DECODE_CHK_NULL_RETURN(m_vc1RefList);

        uint32_t destFrameIdx = m_vc1PicParams->CurrPic.FrameIdx;

        CODECHAL_DECODE_CHK_STATUS_RETURN(FormatUnequalFieldPicture(
            m_unequalFieldSurface[m_vc1RefList[destFrameIdx]->dwUnequalFieldSurfaceIdx],
            m_destSurface,
            true,
            m_videoContextUsesNullHw));
    }

    if (m_olpNeeded)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(PerformVc1Olp());
    }
    else
    {
        if (m_statusQueryReportingEnabled)
        {
            CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_videoContextUsesNullHw));
        }
    }

    // Needs to be re-set for Linux buffer re-use scenarios
    m_vc1RefList[m_vc1PicParams->CurrPic.FrameIdx]->resRefPic = m_destSurface.OsResource;

    // Send the signal to indicate decode completion, in case On-Demand Sync is not present
    if (!(CodecHal_PictureIsField(m_vc1PicParams->CurrPic) &&
            m_vc1PicParams->picture_fields.is_first_field))
    {
        MOS_SYNC_PARAMS syncParams;
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_destSurface.OsResource;

        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));

        if (m_olpNeeded)
        {
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_renderContext;
            syncParams.presSyncResource = &m_deblockSurface.OsResource;

            CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceSignal(m_osInterface, &syncParams));
        }
    }

    m_olpNeeded = false;

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::AddVc1OlpCmd(
    PCODECHAL_DECODE_VC1_OLP_PARAMS vc1OlpParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MhwRenderInterface *renderEngineInterface = m_hwInterface->GetRenderInterface();
    PMHW_KERNEL_STATE   kernelState           = &m_olpKernelState;

    // Launch media walker to handle Y component
    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = MHW_WALKER_MODE_SINGLE;
    walkerCodecParams.dwResolutionX = m_picWidthInMb;
    walkerCodecParams.dwResolutionY = m_picHeightInMb;
    walkerCodecParams.bNoDependency = true;     // force raster scan mode

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaObjectWalkerCmd(
        vc1OlpParams->pCmdBuffer,
        &walkerParams));

    vc1OlpParams->pPipeControlParams->dwFlushMode = MHW_FLUSH_READ_CACHE;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(
        vc1OlpParams->pCmdBuffer,
        nullptr,
        vc1OlpParams->pPipeControlParams));
    vc1OlpParams->pPipeControlParams->dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(
        vc1OlpParams->pCmdBuffer,
        nullptr,
        vc1OlpParams->pPipeControlParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddStateBaseAddrCmd(
        vc1OlpParams->pCmdBuffer,
        vc1OlpParams->pStateBaseAddrParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaVfeCmd(
        vc1OlpParams->pCmdBuffer,
        vc1OlpParams->pVfeParams));

    kernelState->dwCurbeOffset += kernelState->KernelParams.iCurbeLength;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaCurbeLoadCmd(
        vc1OlpParams->pCmdBuffer,
        vc1OlpParams->pCurbeLoadParams));
    kernelState->dwCurbeOffset -= kernelState->KernelParams.iCurbeLength;

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaIDLoadCmd(
        vc1OlpParams->pCmdBuffer,
        vc1OlpParams->pIdLoadParams));

    // For UV component, block size changed in CURBE static data and keep FrameWidth/HeightInMb unchanged here
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaObjectWalkerCmd(
        vc1OlpParams->pCmdBuffer,
        &walkerParams));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::PerformVc1Olp()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MhwRenderInterface *renderEngineInterface = m_hwInterface->GetRenderInterface();
    PMHW_KERNEL_STATE         kernelState           = &m_olpKernelState;
    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = renderEngineInterface->m_stateHeapInterface;

    CODECHAL_DECODE_CHK_NULL_RETURN(stateHeapInterface);

    MOS_SYNC_PARAMS syncParams;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    // Initialize DSH kernel region
    m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
    m_osInterface->pfnResetOsStates(m_osInterface);

    m_osInterface->pfnSetPerfTag(
        m_osInterface,
        (uint16_t)(((m_mode << 4) & 0xF0) | OLP_TYPE));
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    CodecHalGetResourceInfo(m_osInterface, &m_deblockSurface);  // DstSurface

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->DisableSurfaceMmcState(&m_deblockSurface));
#endif

    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        m_olpDshSize,
        false,
        m_decodeStatusBuf.m_swStoreData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(SetCurbeOlp());

    // Send HW commands (including SSH)
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_PIPE_CONTROL_PARAMS pipeControlParams;
    MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->GetDefaultSSEuSetting(CODECHAL_MEDIA_STATE_OLP, false, false, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
        &cmdBuffer, true));

    if (renderEngineInterface->GetL3CacheConfig()->bL3CachingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->SetL3Cache(&cmdBuffer));
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->EnablePreemption(&cmdBuffer));

    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddPipelineSelectCmd(&cmdBuffer, false));

    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    // common function for codec needed when we make change for AVC
    MHW_RCS_SURFACE_PARAMS surfaceParamsSrc;
    MOS_ZeroMemory(&surfaceParamsSrc, sizeof(surfaceParamsSrc));
    surfaceParamsSrc.dwNumPlanes = 2;    // Y, UV
    surfaceParamsSrc.psSurface          = &m_destSurface;
    surfaceParamsSrc.psSurface->dwDepth = 1;    // depth needs to be 0 for codec 2D surface
                                                // Y Plane
    surfaceParamsSrc.dwBindingTableOffset[MHW_Y_PLANE] = CODECHAL_DECODE_VC1_OLP_SRC_Y;
    surfaceParamsSrc.ForceSurfaceFormat[MHW_Y_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM;
    // UV Plane
    surfaceParamsSrc.dwBindingTableOffset[MHW_U_PLANE] = CODECHAL_DECODE_VC1_OLP_SRC_UV;
    surfaceParamsSrc.ForceSurfaceFormat[MHW_U_PLANE] = MHW_GFX3DSTATE_SURFACEFORMAT_R16_UINT;
    surfaceParamsSrc.dwBaseAddrOffset[MHW_U_PLANE] =
        m_destSurface.dwPitch *
        MOS_ALIGN_FLOOR(m_destSurface.UPlaneOffset.iYOffset, MOS_YTILE_H_ALIGNMENT);
    surfaceParamsSrc.dwHeightToUse[MHW_U_PLANE] = surfaceParamsSrc.psSurface->dwHeight / 2;
    surfaceParamsSrc.dwYOffset[MHW_U_PLANE] =
        (m_destSurface.UPlaneOffset.iYOffset % MOS_YTILE_H_ALIGNMENT);

#ifdef _MMC_SUPPORTED
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_mmc->GetSurfaceMmcState(surfaceParamsSrc.psSurface));
#endif

    MHW_RCS_SURFACE_PARAMS surfaceParamsDst;
    MOS_ZeroMemory(&surfaceParamsDst, sizeof(surfaceParamsDst));
    surfaceParamsDst = surfaceParamsSrc;
    surfaceParamsDst.bIsWritable = true;
    surfaceParamsDst.psSurface                         = &m_deblockSurface;
    surfaceParamsDst.psSurface->dwDepth = 1;    // depth needs to be 0 for codec 2D surface
    surfaceParamsDst.dwBindingTableOffset[MHW_Y_PLANE] = CODECHAL_DECODE_VC1_OLP_DST_Y;
    surfaceParamsDst.dwBindingTableOffset[MHW_U_PLANE] = CODECHAL_DECODE_VC1_OLP_DST_UV;

    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetSurfaceState(
        stateHeapInterface,
        kernelState,
        &cmdBuffer,
        1,
        &surfaceParamsSrc));
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetSurfaceState(
        stateHeapInterface,
        kernelState,
        &cmdBuffer,
        1,
        &surfaceParamsDst));

    MHW_STATE_BASE_ADDR_PARAMS stateBaseAddrParams;
    MOS_ZeroMemory(&stateBaseAddrParams, sizeof(stateBaseAddrParams));
    MOS_RESOURCE *dsh = nullptr, *ish = nullptr;
    CODECHAL_DECODE_CHK_NULL_RETURN(dsh = kernelState->m_dshRegion.GetResource());
    CODECHAL_DECODE_CHK_NULL_RETURN(ish = kernelState->m_ishRegion.GetResource());
    stateBaseAddrParams.presDynamicState = dsh;
    stateBaseAddrParams.dwDynamicStateSize = kernelState->m_dshRegion.GetHeapSize();
    stateBaseAddrParams.presInstructionBuffer = ish;
    stateBaseAddrParams.dwInstructionBufferSize = kernelState->m_ishRegion.GetHeapSize();
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddStateBaseAddrCmd(&cmdBuffer, &stateBaseAddrParams));

    MHW_VFE_PARAMS vfeParams = {};
    vfeParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaVfeCmd(&cmdBuffer, &vfeParams));

    MHW_CURBE_LOAD_PARAMS curbeLoadParams;
    MOS_ZeroMemory(&curbeLoadParams, sizeof(curbeLoadParams));
    curbeLoadParams.pKernelState = kernelState;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaCurbeLoadCmd(&cmdBuffer, &curbeLoadParams));

    MHW_ID_LOAD_PARAMS idLoadParams;
    MOS_ZeroMemory(&idLoadParams, sizeof(idLoadParams));
    idLoadParams.pKernelState = kernelState;
    idLoadParams.dwNumKernelsLoaded = 1;
    CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaIDLoadCmd(&cmdBuffer, &idLoadParams));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            CODECHAL_MEDIA_STATE_OLP,
            MHW_DSH_TYPE,
            kernelState));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            CODECHAL_MEDIA_STATE_OLP,
            MHW_SSH_TYPE,
            kernelState));
    )

        CODECHAL_DECODE_VC1_OLP_PARAMS vc1OlpParams;
    vc1OlpParams.pCmdBuffer = &cmdBuffer;
    vc1OlpParams.pPipeControlParams = &pipeControlParams;
    vc1OlpParams.pStateBaseAddrParams = &stateBaseAddrParams;
    vc1OlpParams.pVfeParams = &vfeParams;
    vc1OlpParams.pCurbeLoadParams = &curbeLoadParams;
    vc1OlpParams.pIdLoadParams = &idLoadParams;
    CODECHAL_DECODE_CHK_STATUS_RETURN(AddVc1OlpCmd(&vc1OlpParams));

    // Check if destination surface needs to be synchronized, before command buffer submission
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource         = &m_deblockSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // Update the resource tag (s/w tag) for On-Demand Sync
    m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

    // Update GPU Sync tag for on demand synchronization
    if (m_osInterface->bTagResourceSync)
    {

        pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(&cmdBuffer, nullptr, &pipeControlParams));
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->WriteSyncTagToResource(&cmdBuffer, &syncParams));
    }
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
        stateHeapInterface,
        kernelState));
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    // Add PipeControl to invalidate ISP and MediaState to avoid PageFault issue
    // This code is temporal and it will be moved to batch buffer end in short
    if (GFX_IS_GEN_9_OR_LATER(m_hwInterface->GetPlatform()))
    {
        MHW_PIPE_CONTROL_PARAMS pipeControlParams;

        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
        pipeControlParams.bGenericMediaStateClear = true;
        pipeControlParams.bIndirectStatePointersDisable = true;
        pipeControlParams.bDisableCSStall = false;
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddPipeControl(&cmdBuffer, nullptr, &pipeControlParams));

        if (MEDIA_IS_WA(m_hwInterface->GetWaTable(), WaSendDummyVFEafterPipelineSelect))
        {
            MHW_VFE_PARAMS vfeStateParams = {};
            vfeStateParams.dwNumberofURBEntries = 1;
            CODECHAL_DECODE_CHK_STATUS_RETURN(renderEngineInterface->AddMediaVfeCmd(&cmdBuffer, &vfeStateParams));
        }
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    // To clear the SSEU values in the hw interface struct, so next kernel can be programmed correctly
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, false, true));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_MEDIA_STATE_OLP,
            "_DEC"));
    )

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    if (m_statusQueryReportingEnabled)
    {
        CODECHAL_DECODE_CHK_STATUS_RETURN(ResetStatusReport(m_renderContextUsesNullHw));
    }

    m_osInterface->pfnSetGpuContext(m_osInterface, m_videoContext);

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::UpdateVc1KernelState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    PCODECHAL_DECODE_VC1_KERNEL_HEADER_CM  decodeKernel;
    PMHW_KERNEL_STATE                      kernelState = &m_olpKernelState;

    decodeKernel = (PCODECHAL_DECODE_VC1_KERNEL_HEADER_CM)kernelState->KernelParams.pBinary;
    kernelState->dwKernelBinaryOffset =
        decodeKernel->OLP.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    m_olpDshSize =
        stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() +
        (MOS_ALIGN_CEIL(m_olpCurbeStaticDataLength,
             stateHeapInterface->pStateHeapInterface->GetCurbeAlignment()) *
            2);

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::InitKernelStateVc1Olp()
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    MhwRenderInterface *renderEngineInterface = m_hwInterface->GetRenderInterface();
    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = renderEngineInterface->m_stateHeapInterface;
    CODECHAL_DECODE_CHK_NULL_RETURN(stateHeapInterface);
    PMHW_KERNEL_STATE kernelState = &m_olpKernelState;

    kernelState->KernelParams.pBinary      = m_olpKernelBase;
    kernelState->KernelParams.iSize        = m_olpKernelSize;
    kernelState->KernelParams.iBTCount = CODECHAL_DECODE_VC1_OLP_NUM_SURFACES;
    kernelState->KernelParams.iThreadCount = renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelState->KernelParams.iCurbeLength = m_olpCurbeStaticDataLength;
    kernelState->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelState->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelState->KernelParams.iIdCount = 1;

    kernelState->dwCurbeOffset = stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    CODECHAL_DECODE_CHK_STATUS_RETURN(stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount,
        &kernelState->dwSshSize,
        &kernelState->dwBindingTableSize));

    CODECHAL_DECODE_CHK_STATUS_RETURN(UpdateVc1KernelState());

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(
        stateHeapInterface,
        &m_olpKernelState));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::SetCurbeOlp()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_DECODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface()->m_stateHeapInterface);

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    // Configure Curbe data for Y component
    CODECHAL_DECODE_VC1_OLP_STATIC_DATA cmd = g_cInit_CODECHAL_DECODE_VC1_OLP_STATIC_DATA;

    cmd.DW2.InterlaceFieldFlag      = CodecHal_PictureIsField(m_vc1PicParams->CurrPic);
    cmd.DW2.PictureUpsamplingFlag   = m_vc1PicParams->UpsamplingFlag;
    cmd.DW2.RangeExpansionFlag      = (m_vc1PicParams->range_mapping_fields.range_mapping_enabled != 0);
    cmd.DW2.Profile                 = m_vc1PicParams->sequence_fields.AdvancedProfileFlag;
    cmd.DW2.ComponentFlag           = 0;

    if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        cmd.DW2.RangeMapUV     = m_vc1PicParams->range_mapping_fields.chroma;
        cmd.DW2.RangeMapUVFlag = m_vc1PicParams->range_mapping_fields.chroma_flag;
        cmd.DW2.RangeMapY      = m_vc1PicParams->range_mapping_fields.luma;
        cmd.DW2.RangeMapYFlag  = m_vc1PicParams->range_mapping_fields.luma_flag;
    }

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_olpKernelState.m_dshRegion.AddData(
        &cmd,
        m_olpKernelState.dwCurbeOffset,
        sizeof(cmd)));

    // Configure Curbe data for UV component
    cmd = g_cInit_CODECHAL_DECODE_VC1_OLP_STATIC_DATA;

    cmd.DW2.InterlaceFieldFlag      = CodecHal_PictureIsField(m_vc1PicParams->CurrPic);
    cmd.DW2.PictureUpsamplingFlag   = m_vc1PicParams->UpsamplingFlag;
    cmd.DW2.RangeExpansionFlag      = (m_vc1PicParams->range_mapping_fields.range_mapping_enabled != 0);
    cmd.DW2.Profile                 = m_vc1PicParams->sequence_fields.AdvancedProfileFlag;
    cmd.DW2.ComponentFlag           = 1;

    if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        cmd.DW2.RangeMapUV     = m_vc1PicParams->range_mapping_fields.chroma;
        cmd.DW2.RangeMapUVFlag = m_vc1PicParams->range_mapping_fields.chroma_flag;
        cmd.DW2.RangeMapY      = m_vc1PicParams->range_mapping_fields.luma;
        cmd.DW2.RangeMapYFlag  = m_vc1PicParams->range_mapping_fields.luma_flag;
    }

    cmd.DW4.SourceDataBindingIndex  = CODECHAL_DECODE_VC1_OLP_SRC_UV;
    cmd.DW5.DestDataBindingIndex    = CODECHAL_DECODE_VC1_OLP_DST_UV;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_olpKernelState.m_dshRegion.AddData(
        &cmd,
        m_olpKernelState.dwCurbeOffset +
            MOS_ALIGN_CEIL(m_olpCurbeStaticDataLength, stateHeapInterface->pStateHeapInterface->GetCurbeAlignment()),
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1::InitMmcState()
{
#ifdef _MMC_SUPPORTED
    m_mmc = MOS_New(CodechalMmcDecodeVc1, m_hwInterface, this);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_mmc);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1::AllocateStandard(
    CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL_RETURN(settings);

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitMmcState());

    bool isComputeContextEnabled = false;
    MOS_GPUCTX_CREATOPTIONS createOption;

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_DECODE_ENABLE_COMPUTE_CONTEXT_ID,
        &userFeatureData);
    isComputeContextEnabled = (userFeatureData.u32Data) ? true : false;
#endif

    if (!MEDIA_IS_SKU(m_skuTable, FtrCCSNode))
    {
        isComputeContextEnabled = false;
    }

    if (isComputeContextEnabled)
    {
        // Create Render Context for field scaling
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_COMPUTE,
            MOS_GPU_NODE_COMPUTE,
            &createOption));
        m_renderContext = MOS_GPU_CONTEXT_COMPUTE;
    }
    else
    {
        // Create Render Context for field scaling
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_RENDER,
            MOS_GPU_NODE_3D,
            &createOption));
        m_renderContext = MOS_GPU_CONTEXT_RENDER;
    }

    m_intelEntrypointInUse = settings->intelEntrypointInUse;
    m_width = settings->width;
    m_height = settings->height;
    m_picWidthInMb         = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_width);
    m_picHeightInMb        = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_height);
    m_shortFormatInUse     = settings->shortFormatInUse;
    m_huCCopyInUse         = false;

    CODECHAL_DECODE_CHK_STATUS_RETURN(InitKernelStateVc1Olp());

    CODECHAL_DEBUG_TOOL(
        CODECHAL_DECODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            CODECHAL_MEDIA_STATE_OLP,
            MHW_ISH_TYPE,
            &m_olpKernelState));)

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        m_mode,
        &m_commandBufferSizeNeeded,
        &m_commandPatchListSizeNeeded,
        m_shortFormatInUse);

    // Primitive Level Commands
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        m_mode,
        &m_standardDecodeSizeNeeded,
        &m_standardDecodePatchListSizeNeeded,
        m_shortFormatInUse);

    CODECHAL_DECODE_CHK_STATUS_RETURN(AllocateResources());

    return eStatus;
}

CodechalDecodeVc1::CodechalDecodeVc1(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecode(hwInterface, debugInterface, standardInfo),
    m_huCCopyInUse(0)
{
    CODECHAL_DECODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_resMfdDeblockingFilterRowStoreScratchBuffer, sizeof(m_resMfdDeblockingFilterRowStoreScratchBuffer));
    MOS_ZeroMemory(&m_resBsdMpcRowStoreScratchBuffer, sizeof(m_resBsdMpcRowStoreScratchBuffer));
    MOS_ZeroMemory(m_resVc1BsdMvData, sizeof(m_resVc1BsdMvData));
    MOS_ZeroMemory(&m_resSyncObject, sizeof(m_resSyncObject));
    MOS_ZeroMemory(&m_resPrivateBistreamBuffer, sizeof(m_resPrivateBistreamBuffer));
    MOS_ZeroMemory(&m_bitstream, sizeof(m_bitstream));
    MOS_ZeroMemory(&m_itObjectBatchBuffer, sizeof(m_itObjectBatchBuffer));
    MOS_ZeroMemory(m_unequalFieldSurface, sizeof(m_unequalFieldSurface));
    MOS_ZeroMemory(m_unequalFieldRefListIdx, sizeof(m_unequalFieldRefListIdx));
    MOS_ZeroMemory(&m_destSurface, sizeof(m_destSurface));
    MOS_ZeroMemory(&m_deblockSurface, sizeof(m_deblockSurface));
    MOS_ZeroMemory(&m_resDataBuffer, sizeof(m_resDataBuffer));
    MOS_ZeroMemory(&m_resBitplaneBuffer, sizeof(m_resBitplaneBuffer));
    MOS_ZeroMemory(&m_resSyncObjectWaContextInUse, sizeof(m_resSyncObjectWaContextInUse));
    MOS_ZeroMemory(&m_resSyncObjectVideoContextInUse, sizeof(m_resSyncObjectVideoContextInUse));

}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalDecodeVc1::DumpPicParams(
    PCODEC_VC1_PIC_PARAMS           vc1PicParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(vc1PicParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss<< "CurrPic FrameIdx: "<< +vc1PicParams->CurrPic.FrameIdx<<std::endl;
    oss<< "CurrPic PicFlags: "<< +vc1PicParams->CurrPic.PicFlags<<std::endl;
    oss<< "DeblockedPicIdx: "<< +vc1PicParams->DeblockedPicIdx<<std::endl;
    oss<< "ForwardRefIdx: "<< +vc1PicParams->ForwardRefIdx<<std::endl;
    oss<< "BackwardRefIdx: "<< +vc1PicParams->BackwardRefIdx<<std::endl;

    //Dump sequence_fields
    oss<< "sequence_fields value: "<< +vc1PicParams->sequence_fields.value<<std::endl;
    oss<< "pulldown: "<< +vc1PicParams->sequence_fields.pulldown<<std::endl;
    oss<< "interlace: "<< +vc1PicParams->sequence_fields.interlace<<std::endl;
    oss<< "tfcntrflag: "<< +vc1PicParams->sequence_fields.tfcntrflag<<std::endl;
    oss<< "finterpflag: "<< +vc1PicParams->sequence_fields.finterpflag<<std::endl;
    oss<< "psf: "<< +vc1PicParams->sequence_fields.psf<<std::endl;
    oss<< "multires: "<< +vc1PicParams->sequence_fields.multires<<std::endl;
    oss<< "overlap: "<< +vc1PicParams->sequence_fields.overlap<<std::endl;
    oss<< "syncmarker: "<< +vc1PicParams->sequence_fields.syncmarker<<std::endl;
    oss<< "rangered: "<< +vc1PicParams->sequence_fields.rangered<<std::endl;
    oss<< "max_b_frames: "<< +vc1PicParams->sequence_fields.max_b_frames<<std::endl;
    oss<< "AdvancedProfileFlag: "<< +vc1PicParams->sequence_fields.AdvancedProfileFlag<<std::endl;
    oss<< "coded_width: "<< +vc1PicParams->coded_width<<std::endl;
    oss<< "coded_height: "<< +vc1PicParams->coded_height<<std::endl;

    //Dump entrypoint_fields
    oss<< "broken_link: "<< +vc1PicParams->entrypoint_fields.broken_link<<std::endl;
    oss<< "closed_entry: "<< +vc1PicParams->entrypoint_fields.closed_entry<<std::endl;
    oss<< "panscan_flag: "<< +vc1PicParams->entrypoint_fields.panscan_flag<<std::endl;
    oss<< "loopfilter: "<< +vc1PicParams->entrypoint_fields.loopfilter<<std::endl;
    oss<< "conditional_overlap_flag: "<< +vc1PicParams->conditional_overlap_flag<<std::endl;
    oss<< "fast_uvmc_flag: "<< +vc1PicParams->fast_uvmc_flag<<std::endl;

    //Dump range_mapping_fields
    oss<< "range_mapping_fields range_mapping_enabled: "<< +vc1PicParams->range_mapping_fields.range_mapping_enabled<<std::endl;
    oss<< "luma_flag: "<< +vc1PicParams->range_mapping_fields.luma_flag<<std::endl;
    oss<< "luma: "<< +vc1PicParams->range_mapping_fields.luma<<std::endl;
    oss<< "chroma_flag: "<< +vc1PicParams->range_mapping_fields.chroma_flag<<std::endl;
    oss<< "chroma: "<< +vc1PicParams->range_mapping_fields.chroma<<std::endl;
    oss<< "UpsamplingFlag: "<< +vc1PicParams->UpsamplingFlag<<std::endl;
    oss<< "ScaleFactor: "<< +vc1PicParams->ScaleFactor<<std::endl;
    oss<< "b_picture_fraction: "<< +vc1PicParams->b_picture_fraction<<std::endl;
    oss<< "cbp_table: "<< +vc1PicParams->cbp_table<<std::endl;
    oss<< "mb_mode_table: "<< +vc1PicParams->mb_mode_table<<std::endl;
    oss<< "range_reduction_frame: "<< +vc1PicParams->range_reduction_frame<<std::endl;
    oss<< "rounding_control: "<< +vc1PicParams->rounding_control<<std::endl;
    oss<< "post_processing: "<< +vc1PicParams->post_processing<<std::endl;
    oss<< "picture_resolution_index: "<< +vc1PicParams->picture_resolution_index<<std::endl;
    oss<< "luma_scale: "<< +vc1PicParams->luma_scale<<std::endl;
    oss<< "luma_shift: "<< +vc1PicParams->luma_shift<<std::endl;

    //Dump picture_fields
    oss<< "picture_fields value: "<< +vc1PicParams->picture_fields.value<<std::endl;
    oss<< "picture_type: "<< +vc1PicParams->picture_fields.picture_type<<std::endl;
    oss<< "frame_coding_mode: "<< +vc1PicParams->picture_fields.frame_coding_mode<<std::endl;
    oss<< "top_field_first: "<< +vc1PicParams->picture_fields.top_field_first<<std::endl;
    oss<< "is_first_field: "<< +vc1PicParams->picture_fields.is_first_field<<std::endl;
    oss<< "intensity_compensation: "<< +vc1PicParams->picture_fields.intensity_compensation<<std::endl;

    //Dump raw_coding
    oss<< "raw_coding value: "<< +vc1PicParams->raw_coding.value<<std::endl;
    oss<< "bitplane_present: "<< +vc1PicParams->raw_coding.bitplane_present<<std::endl;
    oss<< "mv_type_mb: "<< +vc1PicParams->raw_coding.mv_type_mb<<std::endl;
    oss<< "direct_mb: "<< +vc1PicParams->raw_coding.direct_mb<<std::endl;
    oss<< "skip_mb: "<< +vc1PicParams->raw_coding.skip_mb<<std::endl;
    oss<< "field_tx: "<< +vc1PicParams->raw_coding.field_tx<<std::endl;
    oss<< "forward_mb: "<< +vc1PicParams->raw_coding.forward_mb<<std::endl;
    oss<< "ac_pred: "<< +vc1PicParams->raw_coding.ac_pred<<std::endl;
    oss<< "overflags: "<< +vc1PicParams->raw_coding.overflags<<std::endl;

    //Dump reference_fields
    oss<< "reference_fields value: "<< +vc1PicParams->reference_fields.value<<std::endl;
    oss<< "reference_distance_flag: "<< +vc1PicParams->reference_fields.reference_distance_flag<<std::endl;
    oss<< "reference_distance: "<< +vc1PicParams->reference_fields.reference_distance<<std::endl;
    oss<< "BwdReferenceDistance: "<< +vc1PicParams->reference_fields.BwdReferenceDistance<<std::endl;
    oss<< "num_reference_pictures: "<< +vc1PicParams->reference_fields.num_reference_pictures<<std::endl;
    oss<< "reference_field_pic_indicator: "<< +vc1PicParams->reference_fields.reference_field_pic_indicator<<std::endl;

    //Dump mv_fields
    oss<< "mv_fields value: "<< +vc1PicParams->mv_fields.value<<std::endl;
    oss<< "MvMode: "<< +vc1PicParams->mv_fields.MvMode<<std::endl;
    oss<< "UnifiedMvMode: "<< +vc1PicParams->mv_fields.UnifiedMvMode<<std::endl;
    oss<< "mv_table: "<< +vc1PicParams->mv_fields.mv_table<<std::endl;
    oss<< "two_mv_block_pattern_table: "<< +vc1PicParams->mv_fields.two_mv_block_pattern_table<<std::endl;
    oss<< "four_mv_switch: "<< +vc1PicParams->mv_fields.four_mv_switch<<std::endl;
    oss<< "four_mv_block_pattern_table: "<< +vc1PicParams->mv_fields.four_mv_block_pattern_table<<std::endl;
    oss<< "extended_mv_flag: "<< +vc1PicParams->mv_fields.extended_mv_flag<<std::endl;
    oss<< "extended_mv_range: "<< +vc1PicParams->mv_fields.extended_mv_range<<std::endl;
    oss<< "extended_dmv_flag: "<< +vc1PicParams->mv_fields.extended_dmv_flag<<std::endl;
    oss<< "extended_dmv_range: "<< +vc1PicParams->mv_fields.extended_dmv_range<<std::endl;

    //Dump pic_quantizer_fields
    oss<< "pic_quantizer_fields value: "<< +vc1PicParams->pic_quantizer_fields.value<<std::endl;
    oss<< "dquant: "<< +vc1PicParams->pic_quantizer_fields.dquant<<std::endl;
    oss<< "quantizer: "<< +vc1PicParams->pic_quantizer_fields.quantizer<<std::endl;
    oss<< "half_qp: "<< +vc1PicParams->pic_quantizer_fields.half_qp<<std::endl;
    oss<< "AltPQuantEdgeMask: "<< +vc1PicParams->pic_quantizer_fields.AltPQuantEdgeMask<<std::endl;
    oss<< "AltPQuantConfig: "<< +vc1PicParams->pic_quantizer_fields.AltPQuantConfig<<std::endl;
    oss<< "pic_quantizer_scale: "<< +vc1PicParams->pic_quantizer_fields.pic_quantizer_scale<<std::endl;
    oss<< "pic_quantizer_type: "<< +vc1PicParams->pic_quantizer_fields.pic_quantizer_type<<std::endl;
    oss<< "alt_pic_quantizer: "<< +vc1PicParams->pic_quantizer_fields.alt_pic_quantizer<<std::endl;

    //Dump transform_fields
    oss<< "transform_fields value: "<< +vc1PicParams->transform_fields.value<<std::endl;
    oss<< "variable_sized_transform_flag: "<< +vc1PicParams->transform_fields.variable_sized_transform_flag<<std::endl;
    oss<< "mb_level_transform_type_flag: "<< +vc1PicParams->transform_fields.mb_level_transform_type_flag<<std::endl;
    oss<< "frame_level_transform_type: "<< +vc1PicParams->transform_fields.frame_level_transform_type<<std::endl;
    oss<< "transform_ac_codingset_idx1: "<< +vc1PicParams->transform_fields.transform_ac_codingset_idx1<<std::endl;
    oss<< "transform_ac_codingset_idx2: "<< +vc1PicParams->transform_fields.transform_ac_codingset_idx2<<std::endl;
    oss<< "intra_transform_dc_table: "<< +vc1PicParams->transform_fields.intra_transform_dc_table<<std::endl;
    oss<< "StatusReportFeedbackNumber : "<< +vc1PicParams->StatusReportFeedbackNumber<<std::endl;

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1::DumpSliceParams(
    PCODEC_VC1_SLICE_PARAMS         sliceControl)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceControl);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss<< "slice_data_size: "<< +sliceControl->slice_data_size<<std::endl;
    oss<< "slice_data_offset: "<< +sliceControl->slice_data_offset<<std::endl;
    oss<< "macroblock_offset: "<< +sliceControl->macroblock_offset<<std::endl;
    oss<< "slice_vertical_position: "<< +sliceControl->slice_vertical_position<<std::endl;

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1::DumpMbParams(
    PCODEC_VC1_MB_PARAMS            mbParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrMbParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(mbParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);
    oss.setf(std::ios::hex, std::ios::basefield);

    oss<< "mb_address: "<< +mbParams->mb_address<<std::endl;
    oss<< "mb_skips_following: "<< +mbParams->mb_skips_following<<std::endl;
    oss<< "data_offset: "<< +mbParams->data_offset<<std::endl;
    oss<< "data_length: "<< +mbParams->data_length<<std::endl;

    //Dump num_coef[CODEC_NUM_BLOCK_PER_MB]
    for(uint16_t i=0;i<CODEC_NUM_BLOCK_PER_MB;++i)
    {
        oss<< "num_coef["<<+i<<"]: "<< +mbParams->num_coef[i]<<std::endl;
    }
    //Dump union mb_type
    oss<< "mb_type.value: "<< +mbParams->mb_type.value<<std::endl;
    oss<< "mb_type.intra_mb: "<< +mbParams->mb_type.intra_mb<<std::endl;
    oss<< "mb_type.motion_forward: "<< +mbParams->mb_type.motion_forward<<std::endl;
    oss<< "mb_type.motion_backward: "<< +mbParams->mb_type.motion_backward<<std::endl;
    oss<< "mb_type.motion_4mv: "<< +mbParams->mb_type.motion_4mv<<std::endl;
    oss<< "mb_type.h261_loopfilter: "<<+mbParams->mb_type.h261_loopfilter<<std::endl;
    oss<< "mb_type.field_residual: "<< +mbParams->mb_type.field_residual<<std::endl;
    oss<< "mb_type.mb_scan_method: "<< +mbParams->mb_type.mb_scan_method<<std::endl;
    oss<< "mb_type.motion_type: "<< +mbParams->mb_type.motion_type<<std::endl;
    oss<< "mb_type.host_resid_diff: "<< +mbParams->mb_type.host_resid_diff<<std::endl;
    oss<< "mb_type.reserved: "<< +mbParams->mb_type.reserved<<std::endl;
    oss<< "mb_type.mvert_field_sel_0: "<< +mbParams->mb_type.mvert_field_sel_0<<std::endl;
    oss<< "mb_type.mvert_field_sel_1: "<< +mbParams->mb_type.mvert_field_sel_1<<std::endl;
    oss<< "mb_type.mvert_field_sel_2: "<< +mbParams->mb_type.mvert_field_sel_2<<std::endl;
    oss<< "mb_type.mvert_field_sel_3: "<< +mbParams->mb_type.mvert_field_sel_3<<std::endl;

    //Dump union pattern_code
    oss<< "pattern_code.value: "<< +mbParams->pattern_code.value<<std::endl;
    oss<< "pattern_code.block_coded_pattern: "<< +mbParams->pattern_code.block_coded_pattern<<std::endl;
    oss<< "pattern_code.block_luma_intra: "<< +mbParams->pattern_code.block_luma_intra<<std::endl;
    oss<< "pattern_code.block_chroma_intra: "<< +mbParams->pattern_code.block_chroma_intra<<std::endl;

    //Dump union motion_vector[4]
    for(uint8_t i=0;i<4;++i)
    {
        oss<< "motion_vector["<<+i<<"].value: "<< +mbParams->motion_vector[i].value<<std::endl;
        oss<< "motion_vector["<<+i<<"].mv_x: "<< +mbParams->motion_vector[i].mv_x<<std::endl;
        oss<< "motion_vector["<<+i<<"].mv_y: "<< +mbParams->motion_vector[i].mv_y<<std::endl;
    }

    const char* fileName = m_debugInterface->CreateFileName(
        "_DEC",
        CodechalDbgBufferType::bufMbParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

#endif
