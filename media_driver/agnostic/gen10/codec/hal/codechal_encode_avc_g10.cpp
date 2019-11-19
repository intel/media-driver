/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_avc_g10.cpp
//! \brief    This file implements the C++ class/interface for Gen10 platform's AVC 
//!           DualPipe encoding to be used across CODECHAL components.
//!
#include "codechal_encode_avc_g10.h"
#ifndef _FULL_OPEN_SOURCE
#include "igcodeckrn_g10.h"
#endif
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g10.h"
#include "mhw_vdbox_mfx_hwcmd_g10_X.h"
#endif

enum MbEncIdOffset
{
    mbEncOffsetI     = 0,
    mbEncOffsetP     = 1,
    mbEncOffsetB     = 2,
    mbEncTargetUsage = 3
};

enum FrameBrcUpdateBindingTableOffset
{
    frameBrcUpdateHistory             = 0,
    frameBrcUpdatePakStatisticsOutput = 1,
    frameBrcUpdateImageStateRead      = 2,
    frameBrcUpdateImageStateWrite     = 3,
    frameBrcUpdateMbEncCurbeWrite     = 4,
    frameBrcUpdateDistortion          = 5,
    frameBrcUpdateConstantData        = 6,
    frameBrcUpdateMbStat              = 7,
    frameBrcUpdateMvStat              = 8,
    frameBrcUpdateNumSurfaces         = 9
};

enum MbBrcUpdateBindingTableOffset
{
    mbBrcUpdateHistory     = 0,
    mbBrcUpdateMbQP        = 1,
    mbBrcUpdateROI         = 2,
    mbBrcUpdateMbStat      = 3,
    mbBrcUpdateNumSurfaces = 4
};

enum WeightPreditionBindingTableOffset
{
    wpInputRefSurface     = 0,
    wpOutputScaledSurface = 1,
    wpNumSurfaces         = 2
};

enum MbEncBindingTableOffset
{
    mbEncMfcAvcPakObj                     = 0,
    mbEncIndirectMvData                   = 1,
    mbEncBrcDistortion                    = 2,
    mbEncCurrentY                         = 3,
    mbEncCurrentUV                        = 4,
    mbEncMbSpecificData                   = 5,
    mbEncAuxVmeOutput                     = 6,
    mbEncRefPicSeletcL0                   = 7,
    mbEncMvDataFromMe                     = 8,
    mbEnc4xMeDistortion                   = 9,
    mbEncSliceMapData                     = 10,
    mbEncFwdMbData                        = 11,
    mbEncFwdMvData                        = 12,
    mbEncMbQP                             = 13,
    mbEncMbBrcConstData                   = 14,
    mbEncVmeInterPredCurrPic              = 15,
    mbEncVmeInterPredFwdRefPicIdx0L0      = 16,
    mbEncVmeInterPredBwdRefPicIdx0L1      = 17,
    mbEncVmeInterPredFwdRefPicIdx1L0      = 18,
    mbEncVmeInterPredBwdRefPicIdx1L1      = 19,
    mbEncVmeInterPredFwdRefPicIdx2L0      = 20,
    mbEncReserved0                        = 21,
    mbEncVmeInterPredFwdRefPicIdx3L0      = 22,
    mbEncReserved1                        = 23,
    mbEncVmeInterPredFwdRefPicIdx4L0      = 24,
    mbEncReserved2                        = 25,
    mbEncVmeInterPredFwdRefPicIdx5L0      = 26,
    mbEncReserved3                        = 27,
    mbEncVmeInterPredFwdRefPicIdx6L0      = 28,
    mbEncReserved4                        = 29,
    mbEncVmeInterPredFwdRefPicIdx7L0      = 30,
    mbEncReserved5                        = 31,
    mbEncVmeInterPredMultiRefCurrPic      = 32,
    mbEncVmeInterPredMultiRefBwdPicIdx0L1 = 33,
    mbEncReserved6                        = 34,
    mbEncVmeInterPredMultiRefBwdPicIdx1L1 = 35,
    mbEncReserved7                        = 36,
    mbEncMbStats                          = 37,
    mbEncMadData                          = 38,
    mbEncBrcCurbeData                     = 39,
    mbEncForceNonSkipMbMap                = 40,
    mbEncAdv                              = 41,
    mbEncSfdCostTable                     = 42,
    mbEncNumSurfaces                      = 43
};

const int32_t CodechalEncodeAvcEncG10::m_brcBindingTableCount[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    frameBrcUpdateNumSurfaces,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    mbEncNumSurfaces,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES,
    mbBrcUpdateNumSurfaces
};

// QP is from 0 - 51, pad it to 64 since BRC needs array size to be 64 bytes
const uint8_t CodechalEncodeAvcEncG10::m_brcConstantDataTables[576] =
{
    0x01, 0x02, 0x03, 0x05, 0x06, 0x01, 0x01, 0x02, 0x03, 0x05, 0x00, 0x00, 0x01, 0x02, 0x03, 0xff,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xfe, 0xfe, 0xff, 0x00, 0x01, 0xfd, 0xfd,
    0xff, 0xff, 0x00, 0xfb, 0xfd, 0xfe, 0xff, 0xff, 0xfa, 0xfb, 0xfd, 0xfe, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x03, 0x05, 0x06, 0x01, 0x01, 0x02, 0x03, 0x05, 0x00, 0x01, 0x01, 0x02, 0x03, 0xff,
    0x00, 0x00, 0x01, 0x02, 0xff, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0xff,
    0xff, 0xff, 0x00, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x04, 0x1e,
    0x3c, 0x50, 0x78, 0x8c, 0xc8, 0xff, 0x04, 0x05, 0x06, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x01, 0x01, 0x02, 0x04, 0x05, 0x01, 0x01, 0x01, 0x02, 0x04, 0x00, 0x00, 0x01, 0x01, 0x02, 0xff,
    0x00, 0x00, 0x01, 0x01, 0xff, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00, 0x01, 0xfe, 0xff,
    0xff, 0xff, 0x00, 0xfd, 0xfe, 0xff, 0xff, 0x00, 0xfb, 0xfc, 0xfe, 0xff, 0xff, 0x00, 0x02, 0x14,
    0x28, 0x46, 0x82, 0xa0, 0xc8, 0xff, 0x04, 0x04, 0x05, 0x05, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03,
    0x03, 0x04, 0xff, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x03, 0x03, 0xff, 0xff, 0x00, 0x00, 0x00,
    0x01, 0x02, 0x02, 0x02, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x02, 0x02, 0xfe, 0xff, 0xff,
    0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0x01, 0x02, 0xfe,
    0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01, 0x01, 0xfe, 0xfe, 0xff, 0xff, 0x00, 0x00, 0x00, 0x01,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const uint8_t CodechalEncodeAvcEncG10::m_ftq25[64] = //27 value 4 dummy
{
    0,                                      //qp=0
    0, 0, 0, 0, 0, 0,                       //qp=1,2;3,4;5,6;
    1, 1, 3, 3, 6, 6, 8, 8, 11, 11,         //qp=7,8;9,10;11,12;13,14;15;16
    13, 13, 16, 16, 19, 19, 22, 22, 26, 26, //qp=17,18;19,20;21,22;23,24;25,26
    30, 30, 34, 34, 39, 39, 44, 44, 50, 50, //qp=27,28;29,30;31,32;33,34;35,36
    56, 56, 62, 62, 69, 69, 77, 77, 85, 85, //qp=37,38;39,40;41,42;43,44;45,46
    94, 94, 104, 104, 115, 115,             //qp=47,48;49,50;51
    0, 0, 0, 0, 0, 0, 0, 0                  //dummy
};

const uint16_t CodechalEncodeAvcEncG10::m_lambdData[256] = {
    9,     7,     9,     6,     12,    8,     12,    8,     15,    10,    15,    9,     19,    13,    19,    12,    24,
    17,    24,    15,    30,    21,    30,    19,    38,    27,    38,    24,    48,    34,    48,    31,    60,    43,
    60,    39,    76,    54,    76,    49,    96,    68,    96,    62,    121,   85,    121,   78,    153,   108,   153,
    99,    193,   135,   193,   125,   243,   171,   243,   157,   306,   215,   307,   199,   385,   271,   387,   251,
    485,   342,   488,   317,   612,   431,   616,   400,   771,   543,   777,   505,   971,   684,   981,   638,   1224,
    862,   1237,  806,   1542,  1086,  1562,  1018,  1991,  1402,  1971,  1287,  2534,  1785,  2488,  1626,  3077,  2167,
    3141,  2054,  3982,  2805,  3966,  2596,  4887,  3442,  5007,  3281,  6154,  4335,  6322,  4148,  7783,  5482,  7984,
    5243,  9774,  6885,  10082, 6629,  12489, 8797,  12733, 8382,  15566, 10965, 16082, 10599, 19729, 13897, 20313, 13404,
    24797, 17467, 25660, 16954, 31313, 22057, 32415, 21445, 39458, 27795, 40953, 27129, 49594, 34935, 51742, 34323, 61440,
    43987, 61440, 43428, 61440, 55462, 61440, 54954, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
    61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
    61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
    61440, 61440, 61440, 61440, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,
};

// AVC MBEnc RefCost tables, index [CodingType][QP]
// QP is from 0 - 51, pad it to 64 since BRC needs each subarray size to be 128bytes
const uint16_t CodechalEncodeAvcEncG10::m_refCostMultiRefQp[NUM_PIC_TYPES][64] =
{
    // I-frame
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    },
    // P-slice
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    },
    //B-slice
    {
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000, 0x0000, 0x0000, 0x0000
    }
};

const uint32_t CODECHAL_ENCODE_AVC_TrellisQuantizationRounding[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 0, 0, 0, 0, 0, 0
};

//mark 2
const uint32_t CodechalEncodeAvcEncG10::m_multiPred[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 3, 0, 0, 0, 0, 0
};

const uint32_t CodechalEncodeAvcEncG10::m_multiRefDisableQPCheck[NUM_TARGET_USAGE_MODES] =
{
    0, 1, 0, 0, 0, 0, 0, 0
};

const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD CodechalEncodeAvcEncG10::m_IPCMThresholdTable[5] =
{
    { 2, 3000 },
    { 4, 3600 },
    { 6, 5000 },
    { 10, 7500 },
    { 18, 9000 },
};

const uint32_t CodechalEncodeAvcEncG10::m_intraModeCostForHighTextureMB[CODEC_AVC_NUM_QP]
{
    0x00000303, 0x00000304, 0x00000404, 0x00000405, 0x00000505, 0x00000506, 0x00000607, 0x00000708,
    0x00000809, 0x0000090a, 0x00000a0b, 0x00000b0c, 0x00000c0e, 0x00000e18, 0x00001819, 0x00001918,
    0x00001a19, 0x00001b19, 0x00001d19, 0x00001e18, 0x00002818, 0x00002918, 0x00002a18, 0x00002b19,
    0x00002d18, 0x00002e18, 0x00003818, 0x00003918, 0x00003a18, 0x00003b0f, 0x00003d0e, 0x00003e0e,
    0x0000480e, 0x0000490e, 0x00004a0e, 0x00004b0d, 0x00004d0d, 0x00004e0d, 0x0000580e, 0x0000590e,
    0x00005a0e, 0x00005b0d, 0x00005d0c, 0x00005e0b, 0x0000680a, 0x00006908, 0x00006a09, 0x00006b0a,
    0x00006d0b, 0x00006e0d, 0x0000780e, 0x00007918
};

class CodechalEncodeAvcEncG10::BrcInitResetCurbe
{
public:
    BrcInitResetCurbe()
    {
        m_dw0.Value                      = 0;
        m_dw1.Value                      = 0;
        m_dw2.Value                      = 0;
        m_dw3.Value                      = 0;
        m_dw4.Value                      = 0;
        m_dw5.Value                      = 0;
        m_dw6.Value                      = 0;
        m_dw7.Value                      = 0;
        m_dw8.Value                      = 0;
        m_dw9.Value                      = 0;
        m_dw10.Value                     = 0;
        m_dw11.AVBRConvergence           = 0;
        m_dw11.MinQP                     = 1;
        m_dw12.MaxQP                     = 51;
        m_dw12.NoSlices                  = 0;
        m_dw13.InstantRateThreshold0ForP = 40;
        m_dw13.InstantRateThreshold1ForP = 60;
        m_dw13.InstantRateThreshold2ForP = 80;
        m_dw13.InstantRateThreshold3ForP = 120;
        m_dw14.InstantRateThreshold0ForB = 35;
        m_dw14.InstantRateThreshold1ForB = 60;
        m_dw14.InstantRateThreshold2ForB = 80;
        m_dw14.InstantRateThreshold3ForB = 120;
        m_dw15.InstantRateThreshold0ForI = 40;
        m_dw15.InstantRateThreshold1ForI = 60;
        m_dw15.InstantRateThreshold2ForI = 90;
        m_dw15.InstantRateThreshold3ForI = 115;
        m_dw16.Value                     = 0;
        m_dw17.Value                     = 0;
        m_dw18.Value                     = 0;
        m_dw19.Value                     = 0;
        m_dw20.Value                     = 0;
        m_dw21.Value                     = 0;
        m_dw22.Value                     = 0;
        m_dw23.Value                     = 0;
        m_dw24.Value                     = 0;
        m_dw25.Value                     = 0;
        m_dw26.Value                     = 0;
        m_dw27.Value                     = 0;
        m_dw28.Value                     = 0;
        m_dw29.Value                     = 0;
        m_dw30.Value                     = 0;
        m_dw31.Value                     = 0;
        m_dw32.Value                     = 0;
        m_dw33.Value                     = 0;
    }

    union
    {
        struct
        {
            uint32_t   ProfileLevelMaxFrame;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   InitBufFullInBits;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   BufSizeInBits;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   AverageBitRate;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   MaxBitRate;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   MinBitRate;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   FrameRateM;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t   FrameRateD;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    union
    {
        struct
        {
            uint32_t   BRCFlag : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   GopP : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw8;

    union
    {
        struct
        {
            uint32_t   GopB : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   FrameWidthInBytes : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw9;

    union
    {
        struct
        {
            uint32_t   FrameHeightInBytes : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   AVBRAccuracy : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw10;

    union
    {
        struct
        {
            uint32_t   AVBRConvergence : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   MinQP : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw11;

    union
    {
        struct
        {
            uint32_t   MaxQP : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   NoSlices : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw12;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForP : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForP : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForP : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForP : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw13;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForB : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForB : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForB : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForB : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw14;

    union
    {
        struct
        {
            uint32_t   InstantRateThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   InstantRateThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   InstantRateThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   InstantRateThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw15;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForPandB : MOS_BITFIELD_RANGE(0, 7);       // Signed byte
            uint32_t   DeviationThreshold1ForPandB : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold2ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw16;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForPandB : MOS_BITFIELD_RANGE(0, 7);       // Signed byte
            uint32_t   DeviationThreshold5ForPandB : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold6ForPandB : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForPandB : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw17;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForVBR : MOS_BITFIELD_RANGE(0, 7);       // Signed byte
            uint32_t   DeviationThreshold1ForVBR : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold2ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold3ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw18;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForVBR : MOS_BITFIELD_RANGE(0, 7);       // Signed byte
            uint32_t   DeviationThreshold5ForVBR : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   DeviationThreshold6ForVBR : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   DeviationThreshold7ForVBR : MOS_BITFIELD_RANGE(24, 31);     // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw19;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);        // Signed byte
            uint32_t   DeviationThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);       // Signed byte
            uint32_t   DeviationThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw20;

    union
    {
        struct
        {
            uint32_t   DeviationThreshold4ForI : MOS_BITFIELD_RANGE(0, 7);        // Signed byte
            uint32_t   DeviationThreshold5ForI : MOS_BITFIELD_RANGE(8, 15);       // Signed byte
            uint32_t   DeviationThreshold6ForI : MOS_BITFIELD_RANGE(16, 23);      // Signed byte
            uint32_t   DeviationThreshold7ForI : MOS_BITFIELD_RANGE(24, 31);      // Signed byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t   InitialQPForI     : MOS_BITFIELD_RANGE(0, 7);       // Signed byte
            uint32_t   InitialQPForP     : MOS_BITFIELD_RANGE(8, 15);      // Signed byte
            uint32_t   InitialQPForB     : MOS_BITFIELD_RANGE(16, 23);     // Signed byte
            uint32_t   SlidingWindowSize : MOS_BITFIELD_RANGE(24, 31);     // unsigned byte
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t   ACQP;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t   LongTermInterval : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   Reserved         : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw28;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw29;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw30;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw31;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexhistorybuffer;
        };
        struct
        {
            uint32_t   Value;
        };

    } m_dw32;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexdistortionbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw33;
};

class CodechalEncodeAvcEncG10::BrcFrameUpdateCurbe
{
public:
    BrcFrameUpdateCurbe()
    {
        m_dw0.Value                   = 0;
        m_dw1.Value                   = 0;
        m_dw2.Value                   = 0;
        m_dw3.startGAdjFrame0         = 10;
        m_dw3.startGAdjFrame1         = 50;
        m_dw4.startGAdjFrame2         = 100;
        m_dw4.startGAdjFrame3         = 150;
        m_dw5.Value                   = 0;
        m_dw6.Value                   = 0;
        m_dw7.Value                   = 0;
        m_dw8.StartGlobalAdjustMult0  = 1;
        m_dw8.StartGlobalAdjustMult1  = 1;
        m_dw8.StartGlobalAdjustMult2  = 3;
        m_dw8.StartGlobalAdjustMult3  = 2;
        m_dw9.StartGlobalAdjustMult4  = 1;
        m_dw9.StartGlobalAdjustDiv0   = 40;
        m_dw9.StartGlobalAdjustDiv1   = 5;
        m_dw9.StartGlobalAdjustDiv2   = 5;
        m_dw10.StartGlobalAdjustDiv3  = 3;
        m_dw10.StartGlobalAdjustDiv4  = 1;
        m_dw10.QPThreshold0           = 7;
        m_dw10.QPThreshold1           = 18;
        m_dw11.QPThreshold2           = 25;
        m_dw11.QPThreshold3           = 37;
        m_dw11.gRateRatioThreshold0   = 40;
        m_dw11.gRateRatioThreshold1   = 75;
        m_dw12.gRateRatioThreshold2   = 97;
        m_dw12.gRateRatioThreshold3   = 103;
        m_dw12.gRateRatioThreshold4   = 125;
        m_dw12.gRateRatioThreshold5   = 160;
        m_dw13.gRateRatioThresholdQP0 = -3;
        m_dw13.gRateRatioThresholdQP1 = -2;
        m_dw13.gRateRatioThresholdQP2 = -1;
        m_dw13.gRateRatioThresholdQP3 = 0;
        m_dw14.gRateRatioThresholdQP4 = 1;
        m_dw14.gRateRatioThresholdQP5 = 2;
        m_dw14.gRateRatioThresholdQP6 = 3;
        m_dw14.QPIndexOfCurPic        = 0xFF;
        m_dw15.Value                  = 0;
        m_dw16.Value                  = 0;
        m_dw17.Value                  = 0;
        m_dw18.Value                  = 0;
        m_dw19.Value                  = 0;
        m_dw20.Value                  = 0;
        m_dw21.Value                  = 0;
        m_dw22.Value                  = 0;
        m_dw23.Value                  = 0;
        m_dw24.Value                  = 0;
        m_dw25.Value                  = 0;
        m_dw26.Value                  = 0;
        m_dw27.Value                  = 0;
        m_dw28.Value                  = 0;
        m_dw29.Value                  = 0;
        m_dw30.Value                  = 0;
        m_dw31.Value                  = 0;
        m_dw32.Value                  = 0;
    }

    union
    {
        struct
        {
            uint32_t   TargetSize;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   FrameNumber;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   SizeofPicHeaders;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame0 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   startGAdjFrame1 : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   startGAdjFrame2 : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   startGAdjFrame3 : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   TargetSizeFlag : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   BRCFlag        : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxNumPAKs     : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   CurrFrameType  : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   NumSkipFrames        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MinimumQP            : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaximumQP            : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   EnableForceToSkip    : MOS_BITFIELD_BIT(24);
            uint32_t   EnableSlidingWindow  : MOS_BITFIELD_BIT(25);
            uint32_t   EnableExtremLowDelay : MOS_BITFIELD_BIT(26);
            uint32_t   DisableVarCompute    : MOS_BITFIELD_BIT(27);
            uint32_t   Reserved             : MOS_BITFIELD_RANGE(28, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t    SizeSkipFrames;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw7;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustMult1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustMult2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustMult3 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw8;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult4 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDiv0  : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustDiv1  : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustDiv2  : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw9;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustDiv3 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDiv4 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   QPThreshold0          : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   QPThreshold1          : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw10;

    union
    {
        struct
        {
            uint32_t   QPThreshold2         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QPThreshold3         : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold0 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold1 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw11;

    union
    {
        struct
        {
            uint32_t   gRateRatioThreshold2 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThreshold3 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold4 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold5 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw12;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP0 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThresholdQP1 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThresholdQP2 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThresholdQP3 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw13;

    union
    {
        struct
        {
            uint32_t   gRateRatioThresholdQP4 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   gRateRatioThresholdQP5 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThresholdQP6 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   QPIndexOfCurPic        : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw14;

    union
    {
        struct
        {
            uint32_t   Reserved        : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   EnableROI       : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   RoundingIntra   : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   RoundingInter   : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw15;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw16;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw17;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw18;

    union
    {
        struct
        {
            uint32_t   UserMaxFrame;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw19;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw20;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw21;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw22;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw23;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexBRChistorybuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw24;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexPreciousPAKstatisticsoutputbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw25;

    union
    {
        struct
        {
            uint32_t   SurfaceIndexAVCIMGstateinputbuffer;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw26;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexAVCIMGstateoutputbuffer;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw27;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexAVC_Encbuffer;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw28;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexAVCDISTORTIONbuffer;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw29;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexBRCconstdatabuffer;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw30;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexMBStatsBuffer;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw31;

    union
    {
        struct
        {
            uint32_t  SurfaceIndexMotionvectorbuffer;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw32;
};

class CodechalEncodeAvcEncG10::MbBrcUpdateCurbe
{
public:
    MbBrcUpdateCurbe()
    {
        m_dw0.Value = 0;
        m_dw1.Value = 0;
        m_dw2.Value = 0;
        m_dw3.Value = 0;
        m_dw4.Value = 0;
        m_dw5.Value = 0;
        m_dw6.Value = 0;
        m_dw7.Value = 0;
        m_dw8.Value = 0;
        m_dw9.Value = 0;
        m_dw10.Value = 0;
        m_dw11.Value = 0;
    }

    union
    {
        struct
        {
            uint32_t   CurrFrameType : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   EnableROI     : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   ROIRatio      : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved      : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw0;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw1;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw2;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw3;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw4;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw5;

    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } m_dw6;

    union
    {
        struct
        {
            uint32_t Reserved;
        };
        struct
        {
            uint32_t  Value;
        };
    } m_dw7;

    union
    {
        struct
        {
            uint32_t HistorybufferIndex;
        };
        struct
        {
            uint32_t Value;
        };
    } m_dw8;

    union
    {
        struct
        {
            uint32_t MBQPbufferIndex;
        };
        struct
        {
            uint32_t Value;
        };
    } m_dw9;

    union
    {
        struct
        {
            uint32_t ROIbufferIndex;
        };
        struct
        {
            uint32_t Value;
        };
    } m_dw10;

    union
    {
        struct
        {
            uint32_t MBstatisticalbufferIndex;
        };
        struct
        {
            uint32_t Value;
        };
    } m_dw11;
} ;

class CodechalEncodeAvcEncG10::MbEncCurbe
{
public:
    enum MbEncCurbeType
    {
        typeIDist,
        typeIFrame,
        typeIField,
        typePFrame,
        typePField,
        typeBFrame,
        typeBField
    };
    static const uint32_t m_mbEncCurbeNormalIFrame[88];
    static const uint32_t m_mbEncCurbeNormalIField[88];
    static const uint32_t m_mbEncCurbeNormalPFrame[88];
    static const uint32_t m_mbEncCurbeNormalPField[88];
    static const uint32_t m_mbEncCurbeNormalBFrame[88];
    static const uint32_t m_mbEncCurbeNormalBField[88];
    static const uint32_t m_mbEncCurbeIFrameDist[88];

    MbEncCurbe(MbEncCurbeType curbeType)
    {
        switch (curbeType)
        {
            case typeIDist:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeIFrameDist),
                    m_mbEncCurbeIFrameDist,
                    sizeof(m_mbEncCurbeIFrameDist));
                break;

            case typeIFrame:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeNormalIFrame),
                    m_mbEncCurbeNormalIFrame,
                    sizeof(m_mbEncCurbeNormalIFrame));
                break;

            case typeIField:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeNormalIField),
                    m_mbEncCurbeNormalIField,
                    sizeof(m_mbEncCurbeNormalIField));
                break;

            case typePFrame:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeNormalPFrame),
                    m_mbEncCurbeNormalPFrame,
                    sizeof(m_mbEncCurbeNormalPFrame));
                break;

            case typePField:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeNormalPField),
                    m_mbEncCurbeNormalPField,
                    sizeof(m_mbEncCurbeNormalPField));
                break;

            case typeBFrame:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeNormalBFrame),
                    m_mbEncCurbeNormalBFrame,
                    sizeof(m_mbEncCurbeNormalBFrame));
                break;

            case typeBField:
                MOS_SecureMemcpy(
                    (void *)&m_encCurbe,
                    sizeof(m_mbEncCurbeNormalBField),
                    m_mbEncCurbeNormalBField,
                    sizeof(m_mbEncCurbeNormalBField));
                break;

            default:
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid curbe type.");
                break;
        }
    }

    struct
    {
        // DW0
        union
        {
            struct
            {
                uint32_t SkipModeEn        : MOS_BITFIELD_BIT(0);
                uint32_t AdaptiveEn        : MOS_BITFIELD_BIT(1);
                uint32_t BiMixDis          : MOS_BITFIELD_BIT(2);
                uint32_t                   : MOS_BITFIELD_RANGE(3, 4);
                uint32_t EarlyImeSuccessEn : MOS_BITFIELD_BIT(5);
                uint32_t                   : MOS_BITFIELD_BIT(6);
                uint32_t T8x8FlagForInterEn: MOS_BITFIELD_BIT(7);
                uint32_t                   : MOS_BITFIELD_RANGE(8, 23);
                uint32_t EarlyImeStop      : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t   MaxNumMVs           : MOS_BITFIELD_RANGE(0, 5);
                uint32_t   ExtendedMvCostRange : MOS_BITFIELD_BIT(6);
                uint32_t                       : MOS_BITFIELD_RANGE(7, 15);
                uint32_t   BiWeight            : MOS_BITFIELD_RANGE(16, 21);
                uint32_t                       : MOS_BITFIELD_RANGE(22, 27);
                uint32_t   UniMixDisable       : MOS_BITFIELD_BIT(28);
                uint32_t                       : MOS_BITFIELD_RANGE(29, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t   LenSP    : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MaxNumSU : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   PicWidth : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t   SrcSize                : MOS_BITFIELD_RANGE(0, 1);
                uint32_t                          : MOS_BITFIELD_RANGE(2, 3);
                uint32_t   MbTypeRemap            : MOS_BITFIELD_RANGE(4, 5);
                uint32_t   SrcAccess              : MOS_BITFIELD_BIT(6);
                uint32_t   RefAccess              : MOS_BITFIELD_BIT(7);
                uint32_t   SearchCtrl             : MOS_BITFIELD_RANGE(8, 10);
                uint32_t   DualSearchPathOption   : MOS_BITFIELD_BIT(11);
                uint32_t   SubPelMode             : MOS_BITFIELD_RANGE(12, 13);
                uint32_t   SkipType               : MOS_BITFIELD_BIT(14);
                uint32_t   DisableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
                uint32_t   InterChromaMode        : MOS_BITFIELD_BIT(16);
                uint32_t   FTEnable               : MOS_BITFIELD_BIT(17);
                uint32_t   BMEDisableFBR          : MOS_BITFIELD_BIT(18);
                uint32_t   BlockBasedSkipEnable   : MOS_BITFIELD_BIT(19);
                uint32_t   InterSAD               : MOS_BITFIELD_RANGE(20, 21);
                uint32_t   IntraSAD               : MOS_BITFIELD_RANGE(22, 23);
                uint32_t   SubMbPartMask          : MOS_BITFIELD_RANGE(24, 30);
                uint32_t                          : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t   PicHeightMinus1                      : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MvRestrictionInSliceEnable           : MOS_BITFIELD_BIT(16);
                uint32_t   DeltaMvEnable                        : MOS_BITFIELD_BIT(17);
                uint32_t   TrueDistortionEnable                 : MOS_BITFIELD_BIT(18);
                uint32_t   EnableWavefrontOptimization          : MOS_BITFIELD_BIT(19);
                uint32_t   EnableFBRBypass                      : MOS_BITFIELD_BIT(20);
                uint32_t   EnableIntraCostScalingForStaticFrame : MOS_BITFIELD_BIT(21);
                uint32_t   EnableIntraRefresh                   : MOS_BITFIELD_BIT(22);
                uint32_t   Reserved                             : MOS_BITFIELD_BIT(23);
                uint32_t   EnableDirtyRect                  : MOS_BITFIELD_BIT(24);
                uint32_t   bCurFldIDR                           : MOS_BITFIELD_BIT(25);
                uint32_t   ConstrainedIntraPredFlag             : MOS_BITFIELD_BIT(26);
                uint32_t   FieldParityFlag                      : MOS_BITFIELD_BIT(27);
                uint32_t   HMEEnable                            : MOS_BITFIELD_BIT(28);
                uint32_t   PictureType                          : MOS_BITFIELD_RANGE(29, 30);
                uint32_t   UseActualRefQPValue                  : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t   SliceMbHeight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   RefWidth      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   RefHeight     : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t   BatchBufferEnd : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t   IntraPartMask          : MOS_BITFIELD_RANGE(0, 4);
                uint32_t   NonSkipZMvAdded        : MOS_BITFIELD_BIT(5);
                uint32_t   NonSkipModeAdded       : MOS_BITFIELD_BIT(6);
                uint32_t   LumaIntraSrcCornerSwap : MOS_BITFIELD_BIT(7);
                uint32_t                          : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MVCostScaleFactor      : MOS_BITFIELD_RANGE(16, 17);
                uint32_t   BilinearEnable         : MOS_BITFIELD_BIT(18);
                uint32_t   SrcFieldPolarity       : MOS_BITFIELD_BIT(19);
                uint32_t   WeightedSADHAAR        : MOS_BITFIELD_BIT(20);
                uint32_t   AConlyHAAR             : MOS_BITFIELD_BIT(21);
                uint32_t   RefIDCostMode          : MOS_BITFIELD_BIT(22);
                uint32_t                          : MOS_BITFIELD_BIT(23);
                uint32_t   SkipCenterMask         : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW7;

        struct
        {
            // DW8
            union
            {
                struct
                {
                    uint32_t   Mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   Mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   Mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   Mode3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW8;

            // DW9
            union
            {
                struct
                {
                    uint32_t   Mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   Mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   Mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   Mode7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW9;

            // DW10
            union
            {
                struct
                {
                    uint32_t   Mode8Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   Mode9Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   RefIDCost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   ChromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW10;

            // DW11
            union
            {
                struct
                {
                    uint32_t   MV0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   MV1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   MV2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   MV3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW11;

            // DW12
            union
            {
                struct
                {
                    uint32_t   MV4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   MV5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   MV6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   MV7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW12;

            // DW13
            union
            {
                struct
                {
                    uint32_t   QpPrimeY : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   QpPrimeCb : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   QpPrimeCr : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   TargetSizeInWord : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW13;

            // DW14
            union
            {
                struct
                {
                    uint32_t   SICFwdTransCoeffThreshold_0 : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t   SICFwdTransCoeffThreshold_1 : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   SICFwdTransCoeffThreshold_2 : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW14;

            // DW15
            union
            {
                struct
                {
                    uint32_t   SICFwdTransCoeffThreshold_3 : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t   SICFwdTransCoeffThreshold_4 : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t   SICFwdTransCoeffThreshold_5 : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t   SICFwdTransCoeffThreshold_6 : MOS_BITFIELD_RANGE(24, 31);    // Highest Freq
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW15;
        } ModeMvCost;

        struct
        {
            // DW16
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_0;
                    SearchPathDelta   SPDelta_1;
                    SearchPathDelta   SPDelta_2;
                    SearchPathDelta   SPDelta_3;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW16;

            // DW17
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_4;
                    SearchPathDelta   SPDelta_5;
                    SearchPathDelta   SPDelta_6;
                    SearchPathDelta   SPDelta_7;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW17;

            // DW18
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_8;
                    SearchPathDelta   SPDelta_9;
                    SearchPathDelta   SPDelta_10;
                    SearchPathDelta   SPDelta_11;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW18;

            // DW19
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_12;
                    SearchPathDelta   SPDelta_13;
                    SearchPathDelta   SPDelta_14;
                    SearchPathDelta   SPDelta_15;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW19;

            // DW20
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_16;
                    SearchPathDelta   SPDelta_17;
                    SearchPathDelta   SPDelta_18;
                    SearchPathDelta   SPDelta_19;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW20;

            // DW21
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_20;
                    SearchPathDelta   SPDelta_21;
                    SearchPathDelta   SPDelta_22;
                    SearchPathDelta   SPDelta_23;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW21;

            // DW22
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_24;
                    SearchPathDelta   SPDelta_25;
                    SearchPathDelta   SPDelta_26;
                    SearchPathDelta   SPDelta_27;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW22;

            // DW23
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_28;
                    SearchPathDelta   SPDelta_29;
                    SearchPathDelta   SPDelta_30;
                    SearchPathDelta   SPDelta_31;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW23;

            // DW24
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_32;
                    SearchPathDelta   SPDelta_33;
                    SearchPathDelta   SPDelta_34;
                    SearchPathDelta   SPDelta_35;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW24;

            // DW25
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_36;
                    SearchPathDelta   SPDelta_37;
                    SearchPathDelta   SPDelta_38;
                    SearchPathDelta   SPDelta_39;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW25;

            // DW26
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_40;
                    SearchPathDelta   SPDelta_41;
                    SearchPathDelta   SPDelta_42;
                    SearchPathDelta   SPDelta_43;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW26;

            // DW27
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_44;
                    SearchPathDelta   SPDelta_45;
                    SearchPathDelta   SPDelta_46;
                    SearchPathDelta   SPDelta_47;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW27;

            // DW28
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_48;
                    SearchPathDelta   SPDelta_49;
                    SearchPathDelta   SPDelta_50;
                    SearchPathDelta   SPDelta_51;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW28;

            // DW29
            union
            {
                struct
                {
                    SearchPathDelta   SPDelta_52;
                    SearchPathDelta   SPDelta_53;
                    SearchPathDelta   SPDelta_54;
                    SearchPathDelta   SPDelta_55;
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW29;

            // DW30
            union
            {
                struct
                {
                    uint32_t   Intra4x4ModeMask : MOS_BITFIELD_RANGE(0, 8);
                uint32_t: MOS_BITFIELD_RANGE(9, 15);
                    uint32_t   Intra8x8ModeMask : MOS_BITFIELD_RANGE(16, 24);
                uint32_t: MOS_BITFIELD_RANGE(25, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW30;

            // DW31
            union
            {
                struct
                {
                    uint32_t   Intra16x16ModeMask : MOS_BITFIELD_RANGE(0, 3);
                    uint32_t   IntraChromaModeMask : MOS_BITFIELD_RANGE(4, 7);
                    uint32_t   IntraComputeType : MOS_BITFIELD_RANGE(8, 9);
                uint32_t: MOS_BITFIELD_RANGE(10, 31);
                };
                struct
                {
                    uint32_t   Value;
                };
            } DW31;
        } SPDelta;

        // DW32
        union
        {
            struct
            {
                uint32_t   SkipVal            : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   MultiPredL0Disable : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   MultiPredL1Disable : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t   Intra16x16NonDCPredPenalty : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Intra8x8NonDCPredPenalty   : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   Intra4x4NonDCPredPenalty   : MOS_BITFIELD_RANGE(16, 23);
                uint32_t                              : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t   List0RefID0FieldParity           : MOS_BITFIELD_BIT(0);
                uint32_t   List0RefID1FieldParity           : MOS_BITFIELD_BIT(1);
                uint32_t   List0RefID2FieldParity           : MOS_BITFIELD_BIT(2);
                uint32_t   List0RefID3FieldParity           : MOS_BITFIELD_BIT(3);
                uint32_t   List0RefID4FieldParity           : MOS_BITFIELD_BIT(4);
                uint32_t   List0RefID5FieldParity           : MOS_BITFIELD_BIT(5);
                uint32_t   List0RefID6FieldParity           : MOS_BITFIELD_BIT(6);
                uint32_t   List0RefID7FieldParity           : MOS_BITFIELD_BIT(7);
                uint32_t   List1RefID0FrameFieldFlag        : MOS_BITFIELD_BIT(8);
                uint32_t   List1RefID1FrameFieldFlag        : MOS_BITFIELD_BIT(9);
                uint32_t   IntraRefreshEn               : MOS_BITFIELD_RANGE(10, 11);
                uint32_t   ArbitraryNumMbsPerSlice          : MOS_BITFIELD_BIT(12);
                uint32_t   TQEnable                         : MOS_BITFIELD_BIT(13);
                uint32_t   ForceNonSkipMbEnable             : MOS_BITFIELD_BIT(14);
                uint32_t   DisableEncSkipCheck              : MOS_BITFIELD_BIT(15);
                uint32_t   EnableDirectBiasAdjustment       : MOS_BITFIELD_BIT(16);
                uint32_t   bForceToSkip                     : MOS_BITFIELD_BIT(17);
                uint32_t   EnableGlobalMotionBiasAdjustment : MOS_BITFIELD_BIT(18);
                uint32_t   EnableAdaptiveTxDecision         : MOS_BITFIELD_BIT(19);
                uint32_t   EnablePerMBStaticCheck           : MOS_BITFIELD_BIT(20);
                uint32_t   EnableAdaptiveSearchWindowSize   : MOS_BITFIELD_BIT(21);
                uint32_t   RemoveIntraRefreshOverlap        : MOS_BITFIELD_BIT(22);
                uint32_t   CQPFlag                          : MOS_BITFIELD_BIT(23);
                uint32_t   List1RefID0FieldParity           : MOS_BITFIELD_BIT(24);
                uint32_t   List1RefID1FieldParity           : MOS_BITFIELD_BIT(25);
                uint32_t   MADEnableFlag                    : MOS_BITFIELD_BIT(26);
                uint32_t   ROIEnableFlag                    : MOS_BITFIELD_BIT(27);
                uint32_t   EnableMBFlatnessChkOptimization  : MOS_BITFIELD_BIT(28);
                uint32_t   bDirectMode                      : MOS_BITFIELD_BIT(29);
                uint32_t   MBBrcEnable                      : MOS_BITFIELD_BIT(30);
                uint32_t   bOriginalBff                     : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t   PanicModeMBThreshold : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   SmallMbSizeInWord    : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   LargeMbSizeInWord    : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t   NumRefIdxL0MinusOne      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   HMECombinedExtraSUs      : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   NumRefIdxL1MinusOne      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t                            : MOS_BITFIELD_RANGE(24, 26);
                uint32_t   MBInputEnable            : MOS_BITFIELD_BIT(27);
                uint32_t   IsFwdFrameShortTermRef   : MOS_BITFIELD_BIT(28);
                uint32_t   CheckAllFractionalEnable : MOS_BITFIELD_BIT(29);
                uint32_t   HMECombineOverlap        : MOS_BITFIELD_RANGE(30, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t   SkipModeEn           : MOS_BITFIELD_BIT(0);
                uint32_t   AdaptiveEn           : MOS_BITFIELD_BIT(1);
                uint32_t   BiMixDis             : MOS_BITFIELD_BIT(2);
                uint32_t                        : MOS_BITFIELD_RANGE(3, 4);
                uint32_t   EarlyImeSuccessEn    : MOS_BITFIELD_BIT(5);
                uint32_t                        : MOS_BITFIELD_BIT(6);
                uint32_t   T8x8FlagForInterEn   : MOS_BITFIELD_BIT(7);
                uint32_t                        : MOS_BITFIELD_RANGE(8, 23);
                uint32_t   EarlyImeStop         : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t   LenSP        : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   MaxNumSU     : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefThreshold : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t                                : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   HMERefWindowsCombThreshold   : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefWidth                     : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   RefHeight                    : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t   DistScaleFactorRefID0List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DistScaleFactorRefID1List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t   DistScaleFactorRefID2List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DistScaleFactorRefID3List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t   DistScaleFactorRefID4List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DistScaleFactorRefID5List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t   DistScaleFactorRefID6List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DistScaleFactorRefID7List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t   ActualQPValueForRefID0List0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ActualQPValueForRefID1List0 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   ActualQPValueForRefID2List0 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ActualQPValueForRefID3List0 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t   ActualQPValueForRefID4List0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ActualQPValueForRefID5List0 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   ActualQPValueForRefID6List0 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ActualQPValueForRefID7List0 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t   ActualQPValueForRefID0List1 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ActualQPValueForRefID1List1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   RefCost                     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t   MbQpReadFactor   : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   IntraCostSF      : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   MaxVmvR          : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW47;

        //DW48
        union
        {
            struct
            {
                uint32_t   IntraRefreshMBx            : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   IntraRefreshUnitInMBMinus1 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   IntraRefreshQPDelta        : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW48;

        // DW49
        union
        {
            struct
            {
                uint32_t   ROI1_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI1_Y_top  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW49;

        // DW50
        union
        {
            struct
            {
                uint32_t   ROI1_X_right  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI1_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW50;

        // DW51
        union
        {
            struct
            {
                uint32_t   ROI2_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI2_Y_top  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW51;

        // DW52
        union
        {
            struct
            {
                uint32_t   ROI2_X_right  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI2_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW52;

        // DW53
        union
        {
            struct
            {
                uint32_t   ROI3_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI3_Y_top  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW53;

        // DW54
        union
        {
            struct
            {
                uint32_t   ROI3_X_right  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI3_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW54;

        // DW55
        union
        {
            struct
            {
                uint32_t   ROI4_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI4_Y_top  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW55;

        // DW56
        union
        {
            struct
            {
                uint32_t   ROI4_X_right  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI4_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW56;

        // DW57
        union
        {
            struct
            {
                uint32_t   ROI1_dQpPrimeY : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   ROI2_dQpPrimeY : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   ROI3_dQpPrimeY : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   ROI4_dQpPrimeY : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW57;

        // DW58
        union
        {
            struct
            {
                uint32_t   Lambda_8x8Inter : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   Lambda_8x8Intra : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW58;

        // DW59
        union
        {
            struct
            {
                uint32_t   Lambda_Inter : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   Lambda_Intra : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW59;

        // DW60
        union
        {
            struct
            {
                uint32_t   MBTextureThreshold : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   TxDecisonThreshold : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW60;

        // DW61
        union
        {
            struct
            {
                uint32_t   HMEMVCostScalingFactor : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Reserved               : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   IntraRefreshMBy    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW61;

        // DW62
        union
        {
            struct
            {
                uint32_t   IPCM_QP0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   IPCM_QP1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   IPCM_QP2 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t   IPCM_QP3 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW62;

        // DW63
        union
        {
            struct
            {
                uint32_t   IPCM_QP4     : MOS_BITFIELD_RANGE(0, 7);
                uint32_t   Reserved     : MOS_BITFIELD_RANGE(8, 15);
                uint32_t   IPCM_Thresh0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW63;

        // DW64
        union
        {
            struct
            {
                uint32_t   IPCM_Thresh1 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   IPCM_Thresh2 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW64;

        // DW65
        union
        {
            struct
            {
                uint32_t   IPCM_Thresh3 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   IPCM_Thresh4 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW65;

        // DW66
        union
        {
            struct
            {
                uint32_t   MBDataSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW66;

        // DW67
        union
        {
            struct
            {
                uint32_t   MVDataSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW67;

        // DW68
        union
        {
            struct
            {
                uint32_t   IDistSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW68;

        // DW69
        union
        {
            struct
            {
                uint32_t   SrcYSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW69;

        // DW70
        union
        {
            struct
            {
                uint32_t   MBSpecificDataSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW70;

        // DW71
        union
        {
            struct
            {
                uint32_t   AuxVmeOutSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW71;

        // DW72
        union
        {
            struct
            {
                uint32_t   CurrRefPicSelSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW72;

        // DW73
        union
        {
            struct
            {
                uint32_t   HMEMVPredFwdBwdSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW73;

        // DW74
        union
        {
            struct
            {
                uint32_t   HMEDistSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW74;

        // DW75
        union
        {
            struct
            {
                uint32_t   SliceMapSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW75;

        // DW76
        union
        {
            struct
            {
                uint32_t   FwdFrmMBDataSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW76;

        // DW77
        union
        {
            struct
            {
                uint32_t   FwdFrmMVSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW77;

        // DW78
        union
        {
            struct
            {
                uint32_t   MBQPBuffer;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW78;

        // DW79
        union
        {
            struct
            {
                uint32_t   MBBRCLut;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW79;

        // DW80
        union
        {
            struct
            {
                uint32_t   VMEInterPredictionSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW80;

        // DW81
        union
        {
            struct
            {
                uint32_t   VMEInterPredictionMRSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW81;

        // DW82
        union
        {
            struct
            {
                uint32_t   MBStatsSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW82;

        // DW83
        union
        {
            struct
            {
                uint32_t   MADSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW83;

        // DW84
        union
        {
            struct
            {
                uint32_t   BRCCurbeSurfIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW84;

        // DW85
        union
        {
            struct
            {
                uint32_t   ForceNonSkipMBmapSurface;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW85;

        // DW86
        union
        {
            struct
            {
                uint32_t   ReservedIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW86;

        // DW87
        union
        {
            struct
            {
                uint32_t   StaticDetectionCostTableIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW87;
    } m_encCurbe;
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeNormalIFrame[88] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a83000, 0x00000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeNormalIField[88] =
{
    0x00000082, 0x00000000, 0x00003910, 0x00a830c0, 0x02000000, 0x28300000, 0x05000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80800000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeNormalPFrame[88] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae3000, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeNormalPField[88] =
{
    0x000000a3, 0x00000008, 0x00003910, 0x00ae30c0, 0x30000000, 0x28300000, 0x05000000, 0x01400060,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x80010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeNormalBFrame[88] =
{
    0x000000a3, 0x00200008, 0x00003910, 0x00aa7700, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x60000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x08000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeNormalBField[88] =
{
    0x000000a3, 0x00200008, 0x00003919, 0x00aa77c0, 0x50020000, 0x20200000, 0x05000000, 0xff400000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x01010000, 0x00040c24, 0x00000000, 0xffff00ff, 0x40000000, 0x000000a1, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x04000002,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeAvcEncG10::MbEncCurbe::m_mbEncCurbeIFrameDist[88] =
{
    0x00000082, 0x00200008, 0x001e3910, 0x00a83000, 0x90000000, 0x28300000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xff000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000100,
    0x80800000, 0x00000000, 0x00000800, 0xffff00ff, 0x40000000, 0x00000080, 0x00003900, 0x28300000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const int32_t CodechalEncodeAvcEncG10::m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    (sizeof(BrcInitResetCurbe)),
    (sizeof(BrcFrameUpdateCurbe)),
    (sizeof(BrcInitResetCurbe)),
    (sizeof(MbEncCurbe)),
    0,
    (sizeof(MbBrcUpdateCurbe))
};

class CodechalEncodeAvcEncG10::WPCurbe
{
public:
    WPCurbe()
    {
        memset((void *)&m_wpCurbeCmd, 0, sizeof(WPCurbe));
    }
    struct
    {
        // DW0
        union
        {
            struct
            {
                uint32_t   DefaultWeight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DefaultOffset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t   ROI0_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI0_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t   ROI0_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI0_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t   ROI0Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI0Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t   ROI1_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI1_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t   ROI1_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI1_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t   ROI1Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI1Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t   ROI2_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI2_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW7;

        // DW8
        union
        {
            struct
            {
                uint32_t   ROI2_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI2_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t   ROI2Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI2Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t   ROI3_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI3_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t   ROI3_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI3_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t   ROI3Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI3Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t   ROI4_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI4_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t   ROI4_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI4_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t   ROI4Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI4Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW15;

        // DW16
        union
        {
            struct
            {
                uint32_t   ROI5_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI5_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                uint32_t   ROI5_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI5_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                uint32_t   ROI5Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI5Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                uint32_t   ROI6_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI6_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                uint32_t   ROI6_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI6_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                uint32_t   ROI6Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI6Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                uint32_t   ROI7_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI7_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                uint32_t   ROI7_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI7_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                uint32_t   ROI7Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI7Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                uint32_t   ROI8_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI8_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                uint32_t   ROI8_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI8_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                uint32_t   ROI8Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI8Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                uint32_t   ROI9_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI9_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                uint32_t   ROI9_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI9_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW29;

        // DW30
        union
        {
            struct
            {
                uint32_t   ROI9Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI9Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t   ROI10_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI10_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW31;

        // DW32
        union
        {
            struct
            {
                uint32_t   ROI10_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI10_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t   ROI10Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI10Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t   ROI11_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI11_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t   ROI11_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI11_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t   ROI11Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI11Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t   ROI12_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI12_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t   ROI12_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI12_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t   ROI12Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI12Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t   ROI13_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI13_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t   ROI13_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI13_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t   ROI13Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI13Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t   ROI14_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI14_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t   ROI14_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI14_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t   ROI14Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI14Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t   ROI15_X_left : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI15_Y_top : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t   ROI15_X_right : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI15_Y_bottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW47;

        // DW48
        union
        {
            struct
            {
                uint32_t   ROI15Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   ROI15Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   Value;
            };
        } DW48;

        // DW49
        union
        {
            struct
            {
                uint32_t   InputSurface;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW49;

        // DW50
        union
        {
            struct
            {
                uint32_t   OutputSurface;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW50;
    } m_wpCurbeCmd;
};

class CodechalEncodeAvcEncG10::BrcBlockCopyCurbe
{
public:
    BrcBlockCopyCurbe()
    {
        memset((void *)&m_brcBlockCopyCurbeCmd, 0, sizeof(BrcBlockCopyCurbe));
    }

    struct
    {
        // uint32_t 0
        union
        {
            struct
            {
                uint32_t   BlockHeight : 16;
                uint32_t   BufferOffset : 16;
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
                uint32_t   SrcSurfaceIndex;
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
                uint32_t  DstSurfaceIndex;
            };
            struct
            {
                uint32_t   Value;
            };
        } DW2;

        // QWORD PADDING
        struct
        {
            uint32_t  Reserved;
        } PADDING;
    } m_brcBlockCopyCurbeCmd;
};

struct  _CodechalEncodeAvcKernelHeader
{
    int m_kernelCount;
    CODECHAL_KERNEL_HEADER m_mbEncQltyI;
    CODECHAL_KERNEL_HEADER m_mbEncQltyP;
    CODECHAL_KERNEL_HEADER m_mbEncQltyB;
    CODECHAL_KERNEL_HEADER m_mbEncNormI;
    CODECHAL_KERNEL_HEADER m_mbEncNormP;
    CODECHAL_KERNEL_HEADER m_mbEncNormB;
    CODECHAL_KERNEL_HEADER m_mbEncPerfI;
    CODECHAL_KERNEL_HEADER m_mbEncPerfP;
    CODECHAL_KERNEL_HEADER m_mbEncPerfB;
    CODECHAL_KERNEL_HEADER m_mbEncAdvI;
    CODECHAL_KERNEL_HEADER m_mbEncAdvP;
    CODECHAL_KERNEL_HEADER m_mbEncAdvB;
    CODECHAL_KERNEL_HEADER m_meP;
    CODECHAL_KERNEL_HEADER m_meB;
    CODECHAL_KERNEL_HEADER m_plyDScalePly;
    CODECHAL_KERNEL_HEADER m_plyDScale2fPly2f;
    CODECHAL_KERNEL_HEADER m_initFrameBrc;
    CODECHAL_KERNEL_HEADER m_frameEncUpdate;
    CODECHAL_KERNEL_HEADER m_brcResetFrame;
    CODECHAL_KERNEL_HEADER m_brcIFrameDist;
    CODECHAL_KERNEL_HEADER m_brcBlockCopy;
    CODECHAL_KERNEL_HEADER m_mbBrcUpdate;
    CODECHAL_KERNEL_HEADER m_ply2xDScalePly;
    CODECHAL_KERNEL_HEADER m_ply2xDScale2fPly2f;
    CODECHAL_KERNEL_HEADER m_meVdenc;
    CODECHAL_KERNEL_HEADER m_weightedPrediction;
    CODECHAL_KERNEL_HEADER m_staticFrameDetection;
};
using CodechalEncodeAvcKernelHeader = struct _CodechalEncodeAvcKernelHeader;

MOS_STATUS CodechalEncodeAvcEncG10::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto krnHeaderTable = (CodechalEncodeAvcKernelHeader*)binary;
    auto invalidEntry = &(krnHeaderTable->m_staticFrameDetection) + 1;

    uint32_t nextKrnOffset = *krnSize;
    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_SCALING4X)
    {
        currKrnHeader = &krnHeaderTable->m_plyDScalePly;
    }
    else if (operation == ENC_SCALING2X)
    {
        currKrnHeader = &krnHeaderTable->m_ply2xDScalePly;
    }
    else if (operation == ENC_ME)
    {
        currKrnHeader = &krnHeaderTable->m_meP;
    }
    else if (operation == VDENC_ME)
    {
        currKrnHeader = &krnHeaderTable->m_meVdenc;
    }
    else if (operation == ENC_BRC)
    {
        currKrnHeader = &krnHeaderTable->m_initFrameBrc;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &krnHeaderTable->m_mbEncQltyI;
    }
    else if (operation == ENC_MBENC_ADV)
    {
        currKrnHeader = &krnHeaderTable->m_mbEncAdvI;
    }
    else if (operation == ENC_WP)
    {
        currKrnHeader = &krnHeaderTable->m_weightedPrediction;
    }
    else if (operation == ENC_SFD)
    {
        currKrnHeader = &krnHeaderTable->m_staticFrameDetection;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    auto pNextKrnHeader = (currKrnHeader + 1);
    if (pNextKrnHeader < invalidEntry)
    {
        nextKrnOffset = pNextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return MOS_STATUS_SUCCESS;
}

CodechalEncodeAvcEncG10::CodechalEncodeAvcEncG10(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEnc(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bKernelTrellis = true;
    bExtendedMvCostRange = true;
    bBrcSplitEnable = true;
    bBrcRoiSupported = true;
    bDecoupleMbEncCurbeFromBRC = true;
    bHighTextureModeCostEnable = true;
    bMvDataNeededByBRC = false;
    m_needCheckCpEnabled = true;

    m_cmKernelEnable = true;
    m_mbStatsSupported = true;
    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;
#ifndef _FULL_OPEN_SOURCE
    m_kernelBase = (uint8_t *)IGCODECKRN_G10;
#else
    m_kernelBase = nullptr;
#endif
    AddIshSize(m_kuid, m_kernelBase);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG10, this));
        CreateAvcPar();
    )
}

CodechalEncodeAvcEncG10::~CodechalEncodeAvcEncG10()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

MOS_STATUS CodechalEncodeAvcEncG10::InitializeState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitializeState());

    m_brcHistoryBufferSize = m_initBrcHistoryBufferSize;
    m_mbencBrcBufferSize = m_mbEncrcHistoryBufferSize;
    m_forceBrcMbStatsEnabled   = true;
    dwBrcConstantSurfaceWidth = m_brcConstSurfaceWidth;
    dwBrcConstantSurfaceHeight = m_brcConstSurfaceHeight;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::InitBrcConstantBuffer(
    PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);

    uint8_t tableIdx = params->wPictureCodingType - 1;

    if (tableIdx >= 3)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_LOCK_PARAMS lockFlags;
    memset(&lockFlags, 0, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto dataPtr = (uint8_t*)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dataPtr);

    memset(dataPtr, 0, params->sBrcConstantDataBuffer.dwWidth * params->sBrcConstantDataBuffer.dwHeight);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table
    eStatus = MOS_SecureMemcpy(
        dataPtr,
        sizeof(m_brcConstantDataTables),
        (void*)m_brcConstantDataTables,
        sizeof(m_brcConstantDataTables));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    dataPtr += sizeof(m_brcConstantDataTables);

    bool blockBasedSkipEn = params->dwMbEncBlockBasedSkipEn ? true : false;
    bool transform_8x8_mode_flag = params->pPicParams->transform_8x8_mode_flag ? true : false;

    // Fill surface with Skip Threshold Table
    switch (params->wPictureCodingType)
    {
    case P_TYPE:
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstSurfaceEarlySkipTableSize,
            (void*)&SkipVal_P_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            m_brcConstSurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    case B_TYPE:
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstSurfaceEarlySkipTableSize,
            (void*)&SkipVal_B_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            m_brcConstSurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    default:
        // do nothing for I TYPE
        break;
    }

    if ((params->wPictureCodingType != I_TYPE) && (params->pAvcQCParams != nullptr) && (params->pAvcQCParams->NonFTQSkipThresholdLUTInput))
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            *(dataPtr + 1 + (qp * 2)) = (uint8_t)CalcSkipVal((params->dwMbEncBlockBasedSkipEn ? true : false), (params->pPicParams->transform_8x8_mode_flag ? true : false), params->pAvcQCParams->NonFTQSkipThresholdLUT[qp]);
        }
    }

    dataPtr += m_brcConstSurfaceEarlySkipTableSize;

    // Initialize to -1 (0xff)
    memset(dataPtr, 0xFF, m_brcConstSurfaceQpList0);
    memset(dataPtr + m_brcConstSurfaceQpList0 + m_brcConstSurfaceQpList0Reserved,
           0xFF, m_brcConstSurfaceQpList1);

    switch (params->wPictureCodingType)
    {
    case B_TYPE:
        dataPtr += (m_brcConstSurfaceQpList0 + m_brcConstSurfaceQpList0Reserved);

        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(dataPtr + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        dataPtr -= (m_brcConstSurfaceQpList0 + m_brcConstSurfaceQpList0Reserved);
        // break statement omitted intentionally
    case P_TYPE:
        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(dataPtr + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        break;
    default:
        // do nothing for I type
        break;
    }

    dataPtr += (m_brcConstSurfaceQpList0 + m_brcConstSurfaceQpList0Reserved
        + m_brcConstSurfaceQpList1 + m_brcConstSurfaceQpList1Reserved);

    // Fill surface with Mode cost and MV cost
    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstSurfaceModeMvCostSize,
        (void*)ModeMvCost_Cm[tableIdx],
        m_brcConstSurfaceModeMvCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // If old mode cost is used the update the table
    if (params->wPictureCodingType == I_TYPE && params->bOldModeCostEnable)
    {
        auto pdwDataTemp = (uint32_t *)dataPtr;
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW0 in each sub-array of 16 DWs
            *pdwDataTemp = (uint32_t)OldIntraModeCost_Cm_Common[qp];
            pdwDataTemp += 16;
        }
    }

    if (params->pAvcQCParams)
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            if (params->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                *(dataPtr + (qp * 32) + 24) =
                    *(dataPtr + (qp * 32) + 25) =
                    *(dataPtr + (qp * 32) + 27) =
                    *(dataPtr + (qp * 32) + 28) =
                    *(dataPtr + (qp * 32) + 29) =
                    *(dataPtr + (qp * 32) + 30) =
                    *(dataPtr + (qp * 32) + 31) = params->pAvcQCParams->FTQSkipThresholdLUT[qp];
            }
        }
    }

    dataPtr += m_brcConstSurfaceModeMvCostSize;

    // Fill surface with Refcost
    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstSurfaceRefCostSize,
        (void*)&m_refCostMultiRefQp[tableIdx][0],
        m_brcConstSurfaceRefCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    dataPtr += m_brcConstSurfaceRefCostSize;

    //Fill surface with Intra cost scaling Factor
    if (params->bAdaptiveIntraScalingEnable)
    {
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstSurfaceIntraCostScalingFactor,
            (void*)&AdaptiveIntraScalingFactor_Cm_Common[0],
            m_brcConstSurfaceIntraCostScalingFactor);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }
    else
    {
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstSurfaceIntraCostScalingFactor,
            (void*)&IntraScalingFactor_Cm_Common[0],
            m_brcConstSurfaceIntraCostScalingFactor);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    dataPtr += m_brcConstSurfaceIntraCostScalingFactor;

    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstSurfaceLambdaSize,
        (void*)&m_lambdData[0],
        m_brcConstSurfaceLambdaSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    dataPtr += m_brcConstSurfaceLambdaSize;

    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstSurfaceFtq25Size,
        (void*)&m_ftq25[0],
        m_brcConstSurfaceFtq25Size);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    params->pOsInterface->pfnUnlockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates = mbEncTargetUsage * m_mbEncTargetUsageNum;
    dwNumMbEncEncKrnStates += mbEncTargetUsage;
    pMbEncKernelStates =
        MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    PMHW_KERNEL_STATE kernelStatePtr = pMbEncKernelStates;
    CODECHAL_KERNEL_HEADER currKrnHeader;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (uint32_t krnStateIdx = 0; krnStateIdx < dwNumMbEncEncKrnStates; krnStateIdx++)
    {
        bool kernelState = (krnStateIdx >= mbEncTargetUsage * m_mbEncTargetUsageNum);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            (kernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (kernelState ? krnStateIdx - mbEncTargetUsage * m_mbEncTargetUsageNum : krnStateIdx),
            (void*)&currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount = mbEncNumSurfaces;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(MbEncCurbe);
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;

        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary =
            kernelBinary +
            (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

        kernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &MbEncBindingTable;

    bindingTable->dwAvcMBEncMfcAvcPakObj = mbEncMfcAvcPakObj;
    bindingTable->dwAvcMBEncIndMVData = mbEncIndirectMvData;
    bindingTable->dwAvcMBEncBRCDist = mbEncBrcDistortion;
    bindingTable->dwAvcMBEncCurrY = mbEncCurrentY;
    bindingTable->dwAvcMBEncCurrUV = mbEncCurrentUV;
    bindingTable->dwAvcMBEncMbSpecificData = mbEncMbSpecificData;

    bindingTable->dwAvcMBEncRefPicSelectL0 = mbEncRefPicSeletcL0;
    bindingTable->dwAvcMBEncMVDataFromME = mbEncMvDataFromMe;
    bindingTable->dwAvcMBEncMEDist = mbEnc4xMeDistortion;
    bindingTable->dwAvcMBEncSliceMapData = mbEncSliceMapData;
    bindingTable->dwAvcMBEncBwdRefMBData = mbEncFwdMbData;
    bindingTable->dwAvcMBEncBwdRefMVData = mbEncFwdMvData;
    bindingTable->dwAvcMBEncMbBrcConstData = mbEncMbBrcConstData;
    bindingTable->dwAvcMBEncMBStats = mbEncMbStats;
    bindingTable->dwAvcMBEncMADData = mbEncMadData;
    bindingTable->dwAvcMBEncMbNonSkipMap = mbEncForceNonSkipMbMap;
    bindingTable->dwAvcMBEncAdv = mbEncAdv;
    bindingTable->dwAvcMbEncBRCCurbeData = mbEncBrcCurbeData;
    bindingTable->dwAvcMBEncStaticDetectionCostTable = mbEncSfdCostTable;

    // Frame
    bindingTable->dwAvcMBEncMbQpFrame = mbEncMbQP;
    bindingTable->dwAvcMBEncCurrPicFrame[0] = mbEncVmeInterPredCurrPic;
    bindingTable->dwAvcMBEncFwdPicFrame[0] = mbEncVmeInterPredFwdRefPicIdx0L0;
    bindingTable->dwAvcMBEncBwdPicFrame[0] = mbEncVmeInterPredBwdRefPicIdx0L1;
    bindingTable->dwAvcMBEncFwdPicFrame[1] = mbEncVmeInterPredFwdRefPicIdx1L0;
    bindingTable->dwAvcMBEncBwdPicFrame[1] = mbEncVmeInterPredBwdRefPicIdx1L1;
    bindingTable->dwAvcMBEncFwdPicFrame[2] = mbEncVmeInterPredFwdRefPicIdx2L0;
    bindingTable->dwAvcMBEncFwdPicFrame[3] = mbEncVmeInterPredFwdRefPicIdx3L0;
    bindingTable->dwAvcMBEncFwdPicFrame[4] = mbEncVmeInterPredFwdRefPicIdx4L0;
    bindingTable->dwAvcMBEncFwdPicFrame[5] = mbEncVmeInterPredFwdRefPicIdx5L0;
    bindingTable->dwAvcMBEncFwdPicFrame[6] = mbEncVmeInterPredFwdRefPicIdx6L0;
    bindingTable->dwAvcMBEncFwdPicFrame[7] = mbEncVmeInterPredFwdRefPicIdx7L0;
    bindingTable->dwAvcMBEncCurrPicFrame[1] = mbEncVmeInterPredMultiRefCurrPic;
    bindingTable->dwAvcMBEncBwdPicFrame[2] = mbEncVmeInterPredMultiRefBwdPicIdx0L1;
    bindingTable->dwAvcMBEncBwdPicFrame[3] = mbEncVmeInterPredMultiRefBwdPicIdx1L1;

    // Field
    bindingTable->dwAvcMBEncMbQpField = mbEncMbQP;
    bindingTable->dwAvcMBEncFieldCurrPic[0] = mbEncVmeInterPredCurrPic;
    bindingTable->dwAvcMBEncFwdPicTopField[0] = mbEncVmeInterPredFwdRefPicIdx0L0;
    bindingTable->dwAvcMBEncBwdPicTopField[0] = mbEncVmeInterPredBwdRefPicIdx0L1;
    bindingTable->dwAvcMBEncFwdPicBotField[0] = mbEncVmeInterPredFwdRefPicIdx0L0;
    bindingTable->dwAvcMBEncBwdPicBotField[0] = mbEncVmeInterPredBwdRefPicIdx0L1;
    bindingTable->dwAvcMBEncFwdPicTopField[1] = mbEncVmeInterPredFwdRefPicIdx1L0;
    bindingTable->dwAvcMBEncBwdPicTopField[1] = mbEncVmeInterPredBwdRefPicIdx1L1;
    bindingTable->dwAvcMBEncFwdPicBotField[1] = mbEncVmeInterPredFwdRefPicIdx1L0;
    bindingTable->dwAvcMBEncBwdPicBotField[1] = mbEncVmeInterPredBwdRefPicIdx1L1;
    bindingTable->dwAvcMBEncFwdPicTopField[2] = mbEncVmeInterPredFwdRefPicIdx2L0;
    bindingTable->dwAvcMBEncFwdPicBotField[2] = mbEncVmeInterPredFwdRefPicIdx2L0;
    bindingTable->dwAvcMBEncFwdPicTopField[3] = mbEncVmeInterPredFwdRefPicIdx3L0;
    bindingTable->dwAvcMBEncFwdPicBotField[3] = mbEncVmeInterPredFwdRefPicIdx3L0;
    bindingTable->dwAvcMBEncFwdPicTopField[4] = mbEncVmeInterPredFwdRefPicIdx4L0;
    bindingTable->dwAvcMBEncFwdPicBotField[4] = mbEncVmeInterPredFwdRefPicIdx4L0;
    bindingTable->dwAvcMBEncFwdPicTopField[5] = mbEncVmeInterPredFwdRefPicIdx5L0;
    bindingTable->dwAvcMBEncFwdPicBotField[5] = mbEncVmeInterPredFwdRefPicIdx5L0;
    bindingTable->dwAvcMBEncFwdPicTopField[6] = mbEncVmeInterPredFwdRefPicIdx6L0;
    bindingTable->dwAvcMBEncFwdPicBotField[6] = mbEncVmeInterPredFwdRefPicIdx6L0;
    bindingTable->dwAvcMBEncFwdPicTopField[7] = mbEncVmeInterPredFwdRefPicIdx7L0;
    bindingTable->dwAvcMBEncFwdPicBotField[7] = mbEncVmeInterPredFwdRefPicIdx7L0;
    bindingTable->dwAvcMBEncFieldCurrPic[1] = mbEncVmeInterPredMultiRefCurrPic;
    bindingTable->dwAvcMBEncBwdPicTopField[2] = mbEncVmeInterPredMultiRefBwdPicIdx0L1;
    bindingTable->dwAvcMBEncBwdPicBotField[2] = mbEncVmeInterPredMultiRefBwdPicIdx0L1;
    bindingTable->dwAvcMBEncBwdPicTopField[3] = mbEncVmeInterPredMultiRefBwdPicIdx1L1;
    bindingTable->dwAvcMBEncBwdPicBotField[3] = mbEncVmeInterPredMultiRefBwdPicIdx1L1;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    for (uint32_t krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM; krnStateIdx++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            ENC_BRC,
            krnStateIdx,
            (void*)&currKrnHeader,
            &kernelSize));

        auto kernelStatePtr = &BrcKernelStates[krnStateIdx];
        kernelStatePtr->KernelParams.iBTCount = m_brcBindingTableCount[krnStateIdx];
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = m_brcCurbeSize[krnStateIdx];
        kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount = 1;
        kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary =
            kernelBinary +
            (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &BrcUpdateBindingTable;
    bindingTable->dwFrameBrcHistoryBuffer = frameBrcUpdateHistory;
    bindingTable->dwFrameBrcPakStatisticsOutputBuffer = frameBrcUpdatePakStatisticsOutput;
    bindingTable->dwFrameBrcImageStateReadBuffer = frameBrcUpdateImageStateRead;
    bindingTable->dwFrameBrcImageStateWriteBuffer = frameBrcUpdateImageStateWrite;

    bindingTable->dwFrameBrcMbEncCurbeWriteData = frameBrcUpdateMbEncCurbeWrite;
    bindingTable->dwFrameBrcDistortionBuffer = frameBrcUpdateDistortion;
    bindingTable->dwFrameBrcConstantData = frameBrcUpdateConstantData;
    bindingTable->dwFrameBrcMbStatBuffer = frameBrcUpdateMbStat;
    bindingTable->dwFrameBrcMvDataBuffer = frameBrcUpdateMvStat;
    bindingTable->dwMbBrcHistoryBuffer = mbBrcUpdateHistory;
    bindingTable->dwMbBrcMbQpBuffer = mbBrcUpdateMbQP;
    bindingTable->dwMbBrcROISurface = mbBrcUpdateROI;
    bindingTable->dwMbBrcMbStatBuffer = mbBrcUpdateMbStat;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::InitKernelStateWP()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    EncOperation encOperation = ENC_WP;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        kernelBinary,
        encOperation,
        0,
        &currKrnHeader,
        &kernelSize));

    pWPKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);
    auto kernelStatePtr = pWPKernelState;
    kernelStatePtr->KernelParams.iBTCount = wpNumSurfaces;
    kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = sizeof(WPCurbe);
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;
    kernelStatePtr->KernelParams.iInlineDataLength = 0;

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary =
        kernelBinary +
        (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presBrcConstantDataBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitMbBrcConstantDataBuffer(params));

    if (params->wPictureCodingType == I_TYPE)
    {
        MOS_LOCK_PARAMS lockFlags;
        memset(&lockFlags, 0, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint32_t* dataPtr = (uint32_t*)params->pOsInterface->pfnLockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(dataPtr);

        // Update MbBrcConstantDataBuffer with high texture cost
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW13 in each sub-array of 16 DWs
            *(dataPtr + 13) = (uint32_t)m_intraModeCostForHighTextureMB[qp];
            // 16 DWs per QP value
            dataPtr += 16;
        }

        params->pOsInterface->pfnUnlockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::GetTrellisQuantization(
    PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS    params,
    PCODECHAL_ENCODE_AVC_TQ_PARAMS          trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding =
        trellisQuantParams->dwTqEnabled ? CODECHAL_ENCODE_AVC_TrellisQuantizationRounding[params->ucTargetUsage] : 0;

    // If AdaptiveTrellisQuantization is enabled then disable trellis quantization for
    // B-frames with QP > 26 only in CQP mode
    if (trellisQuantParams->dwTqEnabled
        && EnableAdaptiveTrellisQuantization[params->ucTargetUsage]
        && params->wPictureCodingType == B_TYPE
        && !params->bBrcEnabled && params->ucQP > 26)
    {
        trellisQuantParams->dwTqEnabled = 0;
        trellisQuantParams->dwTqRounding = 0;
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::GetMbEncKernelStateIdx(CodechalEncodeIdOffsetParams* params, uint32_t *kernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelOffset);

    *kernelOffset = mbEncOffsetI;

    if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *kernelOffset +=
            mbEncTargetUsage * m_mbEncTargetUsageNum;
    }
    else
    {
        if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_NORMAL)
        {
            *kernelOffset += mbEncTargetUsage;
        }
        else if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_PERFORMANCE)
        {
            *kernelOffset += mbEncTargetUsage * 2;
        }
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        *kernelOffset += mbEncOffsetP;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        *kernelOffset += mbEncOffsetB;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetCurbeAvcMbEnc(
    PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
{

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcParams);

    auto picParams = params->pPicParams;
    auto seqParams = params->pSeqParams;
    auto slcParams = params->pSlcParams;

    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    uint8_t meMethod =
        (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[seqParams->TargetUsage] : m_meMethodGeneric[seqParams->TargetUsage];
    // set sliceQP to MAX_SLICE_QP for MbEnc Adv kernel, we can use it to verify whether QP is changed or not
    uint8_t sliceQP = (params->bUseMbEncAdvKernel && params->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    bool framePicture = CodecHal_PictureIsFrame(picParams->CurrOriginalPic);
    bool topField = CodecHal_PictureIsTopField(picParams->CurrOriginalPic);
    bool bottomField = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);

    MbEncCurbe::MbEncCurbeType curbeType;
    if (params->bMbEncIFrameDistEnabled)
    {
        curbeType = MbEncCurbe::typeIDist;
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
            if (framePicture)
            {
                curbeType = MbEncCurbe::typeIFrame;
            }
            else
            {
                curbeType = MbEncCurbe::typeIField;
            }
            break;

        case P_TYPE:
            if (framePicture)
            {
                curbeType = MbEncCurbe::typePFrame;
            }
            else
            {
                curbeType = MbEncCurbe::typePField;
            }
            break;

        case B_TYPE:
            if (framePicture)
            {
                curbeType = MbEncCurbe::typeBFrame;
            }
            else
            {
                curbeType = MbEncCurbe::typeBField;
            }
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            return MOS_STATUS_UNKNOWN;
        }
    }

    MbEncCurbe cmd(curbeType);
    // r1
    cmd.m_encCurbe.DW0.AdaptiveEn =
        cmd.m_encCurbe.DW37.AdaptiveEn = EnableAdaptiveSearch[seqParams->TargetUsage];
    cmd.m_encCurbe.DW0.T8x8FlagForInterEn =
        cmd.m_encCurbe.DW37.T8x8FlagForInterEn = picParams->transform_8x8_mode_flag;
    cmd.m_encCurbe.DW2.LenSP                   = MaxLenSP[seqParams->TargetUsage];
    cmd.m_encCurbe.DW1.ExtendedMvCostRange     = bExtendedMvCostRange;
    cmd.m_encCurbe.DW36.MBInputEnable          = bMbSpecificDataEnabled;
    cmd.m_encCurbe.DW38.LenSP                  = 0;  // MBZ
    cmd.m_encCurbe.DW3.SrcAccess =
        cmd.m_encCurbe.DW3.RefAccess = framePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE &&  bFTQEnable)
    {
        if (m_pictureCodingType == P_TYPE)
        {
            cmd.m_encCurbe.DW3.FTEnable = FTQBasedSkip[seqParams->TargetUsage] & 0x01;
        }
        else // B_TYPE
        {
            cmd.m_encCurbe.DW3.FTEnable = (FTQBasedSkip[seqParams->TargetUsage] >> 1) & 0x01;
        }
    }
    else
    {
        cmd.m_encCurbe.DW3.FTEnable = 0;
    }
    if (picParams->UserFlags.bDisableSubMBPartition)
    {
        cmd.m_encCurbe.DW3.SubMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }
    cmd.m_encCurbe.DW2.PicWidth        = params->wPicWidthInMb;
    cmd.m_encCurbe.DW4.PicHeightMinus1 = params->wFieldFrameHeightInMb - 1;
    cmd.m_encCurbe.DW4.FieldParityFlag = cmd.m_encCurbe.DW7.SrcFieldPolarity = bottomField ? 1 : 0;
    cmd.m_encCurbe.DW4.EnableFBRBypass                                       = bFBRBypassEnable;
    cmd.m_encCurbe.DW4.EnableIntraCostScalingForStaticFrame                  = params->bStaticFrameDetectionEnabled;
    cmd.m_encCurbe.DW4.bCurFldIDR                                            = framePicture ? 0 : (picParams->bIdrPic || m_firstFieldIdrPic);
    cmd.m_encCurbe.DW4.ConstrainedIntraPredFlag                              = picParams->constrained_intra_pred_flag;
    cmd.m_encCurbe.DW4.HMEEnable                                             = m_hmeEnabled;
    cmd.m_encCurbe.DW4.PictureType                                           = m_pictureCodingType - 1;
    cmd.m_encCurbe.DW4.UseActualRefQPValue                                   = m_hmeEnabled ? (m_multiRefDisableQPCheck[seqParams->TargetUsage] == 0) : false;
    cmd.m_encCurbe.DW5.SliceMbHeight                                         = params->usSliceHeight;
    cmd.m_encCurbe.DW7.IntraPartMask                                         = picParams->transform_8x8_mode_flag ? 0 : 0x2;  // Disable 8x8 if flag is not set
    // r2
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.m_encCurbe.DW6.BatchBufferEnd = 0;
    }
    else
    {
        uint8_t tableIdx = m_pictureCodingType - 1;
        eStatus          = MOS_SecureMemcpy(&(cmd.m_encCurbe.ModeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm[tableIdx][sliceQP], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        if (m_pictureCodingType == I_TYPE &&  bOldModeCostEnable)
        {
            // Old intra mode cost needs to be used if bOldModeCostEnable is 1
            cmd.m_encCurbe.ModeMvCost.DW8.Value = OldIntraModeCost_Cm_Common[sliceQP];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
            // Load different MvCost for P picture when SkipBiasAdjustment is enabled
            // No need to check for P picture as the flag is only enabled for P picture
            cmd.m_encCurbe.ModeMvCost.DW11.Value = MvCost_PSkipAdjustment_Cm_Common[sliceQP];
        }
    }

    // r3 & r4
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.m_encCurbe.SPDelta.DW31.IntraComputeType = 1;
    }
    else
    {
        uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        eStatus          = MOS_SecureMemcpy(&(cmd.m_encCurbe.SPDelta), 16 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    // r5
    if (m_pictureCodingType == P_TYPE)
    {
        cmd.m_encCurbe.DW32.SkipVal = SkipVal_P_Common[cmd.m_encCurbe.DW3.BlockBasedSkipEnable][picParams->transform_8x8_mode_flag][sliceQP];
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        cmd.m_encCurbe.DW32.SkipVal = SkipVal_B_Common[cmd.m_encCurbe.DW3.BlockBasedSkipEnable][picParams->transform_8x8_mode_flag][sliceQP];
    }

    cmd.m_encCurbe.ModeMvCost.DW13.QpPrimeY = sliceQP;
    // QpPrimeCb and QpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    cmd.m_encCurbe.ModeMvCost.DW13.QpPrimeCb        = sliceQP;
    cmd.m_encCurbe.ModeMvCost.DW13.QpPrimeCr        = sliceQP;
    cmd.m_encCurbe.ModeMvCost.DW13.TargetSizeInWord = 0xff;  // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        switch (m_multiPred[seqParams->TargetUsage])
        {
        case 0: // Disable multipred for both P & B picture types
            cmd.m_encCurbe.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_encCurbe.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 1: // Enable multipred for P pictures only
            cmd.m_encCurbe.DW32.MultiPredL0Disable = (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_encCurbe.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 2: // Enable multipred for B pictures only
            cmd.m_encCurbe.DW32.MultiPredL0Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_encCurbe.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 3: // Enable multipred for both P & B picture types
            cmd.m_encCurbe.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE;
            cmd.m_encCurbe.DW32.MultiPredL1Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;
        }
    }
    else
    {
        cmd.m_encCurbe.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.m_encCurbe.DW32.MultiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.m_encCurbe.DW34.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.m_encCurbe.DW34.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.m_encCurbe.DW34.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.m_encCurbe.DW34.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.m_encCurbe.DW34.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.m_encCurbe.DW34.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.m_encCurbe.DW34.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.m_encCurbe.DW34.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.m_encCurbe.DW34.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.m_encCurbe.DW34.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    if (m_adaptiveTransformDecisionEnabled)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.m_encCurbe.DW34.EnableAdaptiveTxDecision = true;
        }
        cmd.m_encCurbe.DW60.TxDecisonThreshold = m_adaptiveTxDecisionThreshold;
    }
    if (m_adaptiveTransformDecisionEnabled || m_flatnessCheckEnabled)
    {
        cmd.m_encCurbe.DW60.MBTextureThreshold = m_mbTextureThreshold;
    }
    if (m_pictureCodingType == B_TYPE)
    {
        cmd.m_encCurbe.DW34.List1RefID0FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_encCurbe.DW34.List1RefID1FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        cmd.m_encCurbe.DW34.bDirectMode               = slcParams->direct_spatial_mv_pred_flag;
    }

    cmd.m_encCurbe.DW34.EnablePerMBStaticCheck          = params->bStaticFrameDetectionEnabled;
    cmd.m_encCurbe.DW34.EnableAdaptiveSearchWindowSize  = params->bApdatvieSearchWindowSizeEnabled;
    cmd.m_encCurbe.DW34.RemoveIntraRefreshOverlap       = picParams->bDisableRollingIntraRefreshOverlap;
    cmd.m_encCurbe.DW34.bOriginalBff                    = framePicture ? 0 : ((m_firstField && (bottomField)) || (!m_firstField && (!bottomField)));
    cmd.m_encCurbe.DW34.EnableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    cmd.m_encCurbe.DW34.ROIEnableFlag                   = params->bRoiEnabled;
    cmd.m_encCurbe.DW34.MADEnableFlag                   = m_madEnabled;
    cmd.m_encCurbe.DW34.MBBrcEnable                     = bMbBrcEnabled || bMbQpDataEnabled;
    cmd.m_encCurbe.DW34.ArbitraryNumMbsPerSlice         = m_arbitraryNumMbsInSlice;
    cmd.m_encCurbe.DW34.ForceNonSkipMbEnable            = params->bMbDisableSkipMapEnabled;
    if (params->pAvcQCParams && !cmd.m_encCurbe.DW34.ForceNonSkipMbEnable)  // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        cmd.m_encCurbe.DW34.DisableEncSkipCheck = params->pAvcQCParams->skipCheckDisable;
        }
        cmd.m_encCurbe.DW34.TQEnable                   = m_trellisQuantParams.dwTqEnabled;
        cmd.m_encCurbe.DW34.CQPFlag                    = !bBrcEnabled;  // 1 - Rate Control is CQP, 0 - Rate Control is BRC
        cmd.m_encCurbe.DW36.CheckAllFractionalEnable   = bCAFEnable;
        cmd.m_encCurbe.DW38.RefThreshold               = m_refTreshold;
        cmd.m_encCurbe.DW39.HMERefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ? HMEBCombineLen[seqParams->TargetUsage] : HMECombineLen[seqParams->TargetUsage];

        // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
        // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
        // BRC use split kernel, MB QP surface is same size as input picture
        cmd.m_encCurbe.DW47.MbQpReadFactor = (bMbBrcEnabled || bMbQpDataEnabled) ? 0 : 2;

        // Those fields are not really used for I_dist kernel,
        // but set them to 0 to get bit-exact match with kernel prototype
        if (params->bMbEncIFrameDistEnabled)
        {
            cmd.m_encCurbe.ModeMvCost.DW13.QpPrimeY        = 0;
            cmd.m_encCurbe.ModeMvCost.DW13.QpPrimeCb       = 0;
            cmd.m_encCurbe.ModeMvCost.DW13.QpPrimeCr       = 0;
            cmd.m_encCurbe.DW33.Intra16x16NonDCPredPenalty = 0;
            cmd.m_encCurbe.DW33.Intra4x4NonDCPredPenalty   = 0;
            cmd.m_encCurbe.DW33.Intra8x8NonDCPredPenalty   = 0;
    }

    //r6
    if (cmd.m_encCurbe.DW4.UseActualRefQPValue)
    {
        cmd.m_encCurbe.DW44.ActualQPValueForRefID0List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_encCurbe.DW44.ActualQPValueForRefID1List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        cmd.m_encCurbe.DW44.ActualQPValueForRefID2List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        cmd.m_encCurbe.DW44.ActualQPValueForRefID3List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        cmd.m_encCurbe.DW45.ActualQPValueForRefID4List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        cmd.m_encCurbe.DW45.ActualQPValueForRefID5List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        cmd.m_encCurbe.DW45.ActualQPValueForRefID6List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        cmd.m_encCurbe.DW45.ActualQPValueForRefID7List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        cmd.m_encCurbe.DW46.ActualQPValueForRefID0List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_encCurbe.DW46.ActualQPValueForRefID1List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    uint8_t tableIdx = m_pictureCodingType - 1;
    cmd.m_encCurbe.DW46.RefCost = m_refCostMultiRefQp[tableIdx][sliceQP];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.m_encCurbe.DW0.SkipModeEn                  = 0;
        cmd.m_encCurbe.DW37.SkipModeEn                 = 0;
        cmd.m_encCurbe.DW36.HMECombineOverlap          = 0;
        cmd.m_encCurbe.DW47.IntraCostSF                = 16;  // This is not used but recommended to set this to 16 by Kernel team
        cmd.m_encCurbe.DW34.EnableDirectBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.m_encCurbe.DW1.MaxNumMVs        = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.m_encCurbe.DW3.BMEDisableFBR    = 1;
        cmd.m_encCurbe.DW5.RefWidth         = SearchX[seqParams->TargetUsage];
        cmd.m_encCurbe.DW5.RefHeight        = SearchY[seqParams->TargetUsage];
        cmd.m_encCurbe.DW7.NonSkipZMvAdded  = 1;
        cmd.m_encCurbe.DW7.NonSkipModeAdded = 1;
        cmd.m_encCurbe.DW7.SkipCenterMask   = 1;
        cmd.m_encCurbe.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ? AdaptiveIntraScalingFactor_Cm_Common[sliceQP] : IntraScalingFactor_Cm_Common[sliceQP];
        cmd.m_encCurbe.DW47.MaxVmvR                    = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.m_encCurbe.DW36.HMECombineOverlap          = 1;
        cmd.m_encCurbe.DW36.NumRefIdxL0MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.m_encCurbe.DW39.RefWidth                   = SearchX[seqParams->TargetUsage];
        cmd.m_encCurbe.DW39.RefHeight                  = SearchY[seqParams->TargetUsage];
        cmd.m_encCurbe.DW34.EnableDirectBiasAdjustment = 0;
        if (params->pAvcQCParams)
        {
            cmd.m_encCurbe.DW34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }
    else
    {
        // B_TYPE
        cmd.m_encCurbe.DW1.MaxNumMVs      = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.m_encCurbe.DW1.BiWeight       = m_biWeight;
        cmd.m_encCurbe.DW3.SearchCtrl     = 7;
        cmd.m_encCurbe.DW3.SkipType       = 1;
        cmd.m_encCurbe.DW5.RefWidth       = BSearchX[seqParams->TargetUsage];
        cmd.m_encCurbe.DW5.RefHeight      = BSearchY[seqParams->TargetUsage];
        cmd.m_encCurbe.DW7.SkipCenterMask = 0xFF;
        cmd.m_encCurbe.DW47.IntraCostSF =
            bAdaptiveIntraScalingEnable ? AdaptiveIntraScalingFactor_Cm_Common[sliceQP] : IntraScalingFactor_Cm_Common[sliceQP];
        cmd.m_encCurbe.DW47.MaxVmvR           = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.m_encCurbe.DW36.HMECombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            CODEC_PICTURE codecHalPic = params->pSlcParams->RefPicList[LIST_1][0];
            if (codecHalPic.PicFlags != PICTURE_INVALID &&
                codecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
                params->pPicIdx[codecHalPic.FrameIdx].bValid)
            {
                // Although its name is FWD, it actually means the future frame or the backward reference frame
                cmd.m_encCurbe.DW36.IsFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(params->pPicParams->RefFrameList[codecHalPic.FrameIdx]);
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        cmd.m_encCurbe.DW36.NumRefIdxL0MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.m_encCurbe.DW36.NumRefIdxL1MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l1_active_minus1 : 0;
        cmd.m_encCurbe.DW39.RefWidth                   = BSearchX[seqParams->TargetUsage];
        cmd.m_encCurbe.DW39.RefHeight                  = BSearchY[seqParams->TargetUsage];
        cmd.m_encCurbe.DW40.DistScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        cmd.m_encCurbe.DW40.DistScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        cmd.m_encCurbe.DW41.DistScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        cmd.m_encCurbe.DW41.DistScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        cmd.m_encCurbe.DW42.DistScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        cmd.m_encCurbe.DW42.DistScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        cmd.m_encCurbe.DW43.DistScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        cmd.m_encCurbe.DW43.DistScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        if (params->pAvcQCParams)
        {
            cmd.m_encCurbe.DW34.EnableDirectBiasAdjustment = params->pAvcQCParams->directBiasAdjustmentEnable;
            if (cmd.m_encCurbe.DW34.EnableDirectBiasAdjustment)
            {
                cmd.m_encCurbe.DW7.NonSkipModeAdded = 1;
                cmd.m_encCurbe.DW7.NonSkipZMvAdded  = 1;
            }

            cmd.m_encCurbe.DW34.EnableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }

    *params->pdwBlockBasedSkipEn = cmd.m_encCurbe.DW3.BlockBasedSkipEnable;

    if (picParams->EnableRollingIntraRefresh)
    {
        cmd.m_encCurbe.DW34.IntraRefreshEn = picParams->EnableRollingIntraRefresh;

        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        cmd.m_encCurbe.DW32.MultiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && RATECONTROL_CQP != seqParams->RateControlMethod)
        {
            /*BRC update kernel updates these CURBE to MBEnc*/
            cmd.m_encCurbe.DW4.EnableIntraRefresh = false;
            cmd.m_encCurbe.DW34.IntraRefreshEn    = ROLLING_I_DISABLED;
            cmd.m_encCurbe.DW48.IntraRefreshMBx   = 0; /* MB column number */
            cmd.m_encCurbe.DW61.IntraRefreshMBy   = 0; /* MB row number */
        }
        else
        {
            cmd.m_encCurbe.DW4.EnableIntraRefresh = true;
            cmd.m_encCurbe.DW34.IntraRefreshEn    = picParams->EnableRollingIntraRefresh;
            cmd.m_encCurbe.DW48.IntraRefreshMBx   = picParams->IntraRefreshMBx; /* MB column number */
            cmd.m_encCurbe.DW61.IntraRefreshMBy   = picParams->IntraRefreshMBy; /* MB row number */
        }
        cmd.m_encCurbe.DW48.IntraRefreshUnitInMBMinus1 = picParams->IntraRefreshUnitinMB;
        cmd.m_encCurbe.DW48.IntraRefreshQPDelta        = picParams->IntraRefreshQPDelta;
    }
    else
    {
        cmd.m_encCurbe.DW34.IntraRefreshEn = 0;
    }

    if (params->bRoiEnabled)
    {
        cmd.m_encCurbe.DW49.ROI1_X_left   = picParams->ROI[0].Left;
        cmd.m_encCurbe.DW49.ROI1_Y_top    = picParams->ROI[0].Top;
        cmd.m_encCurbe.DW50.ROI1_X_right  = picParams->ROI[0].Right;
        cmd.m_encCurbe.DW50.ROI1_Y_bottom = picParams->ROI[0].Bottom;

        cmd.m_encCurbe.DW51.ROI2_X_left   = picParams->ROI[1].Left;
        cmd.m_encCurbe.DW51.ROI2_Y_top    = picParams->ROI[1].Top;
        cmd.m_encCurbe.DW52.ROI2_X_right  = picParams->ROI[1].Right;
        cmd.m_encCurbe.DW52.ROI2_Y_bottom = picParams->ROI[1].Bottom;

        cmd.m_encCurbe.DW53.ROI3_X_left   = picParams->ROI[2].Left;
        cmd.m_encCurbe.DW53.ROI3_Y_top    = picParams->ROI[2].Top;
        cmd.m_encCurbe.DW54.ROI3_X_right  = picParams->ROI[2].Right;
        cmd.m_encCurbe.DW54.ROI3_Y_bottom = picParams->ROI[2].Bottom;

        cmd.m_encCurbe.DW55.ROI4_X_left   = picParams->ROI[3].Left;
        cmd.m_encCurbe.DW55.ROI4_Y_top    = picParams->ROI[3].Top;
        cmd.m_encCurbe.DW56.ROI4_X_right  = picParams->ROI[3].Right;
        cmd.m_encCurbe.DW56.ROI4_Y_bottom = picParams->ROI[3].Bottom;

        if (bBrcEnabled == false)
        {
            uint8_t numROI = picParams->NumROI;
            int8_t priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = { 0 };

            // cqp case
            for (unsigned int i = 0; i < numROI; i += 1)
            {
                int8_t qpRoi = picParams->ROI[i].PriorityLevelOrDQp;

                // clip qp roi in order to have (qp + qpY) in range [0, 51]
                priorityLevelOrDQp[i] = (int8_t)CodecHal_Clip3(-sliceQP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - sliceQP, qpRoi);
            }

            cmd.m_encCurbe.DW57.ROI1_dQpPrimeY = priorityLevelOrDQp[0];
            cmd.m_encCurbe.DW57.ROI2_dQpPrimeY = priorityLevelOrDQp[1];
            cmd.m_encCurbe.DW57.ROI3_dQpPrimeY = priorityLevelOrDQp[2];
            cmd.m_encCurbe.DW57.ROI4_dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
            // kernel does not support BRC case
            cmd.m_encCurbe.DW34.ROIEnableFlag = 0;
        }
    }
    else if (params->bDirtyRoiEnabled)
    {
        // enable Dirty Rect flag
        cmd.m_encCurbe.DW4.EnableDirtyRect = true;

        cmd.m_encCurbe.DW49.ROI1_X_left   = params->pPicParams->DirtyROI[0].Left;
        cmd.m_encCurbe.DW49.ROI1_Y_top    = params->pPicParams->DirtyROI[0].Top;
        cmd.m_encCurbe.DW50.ROI1_X_right  = params->pPicParams->DirtyROI[0].Right;
        cmd.m_encCurbe.DW50.ROI1_Y_bottom = params->pPicParams->DirtyROI[0].Bottom;

        cmd.m_encCurbe.DW51.ROI2_X_left   = params->pPicParams->DirtyROI[1].Left;
        cmd.m_encCurbe.DW51.ROI2_Y_top    = params->pPicParams->DirtyROI[1].Top;
        cmd.m_encCurbe.DW52.ROI2_X_right  = params->pPicParams->DirtyROI[1].Right;
        cmd.m_encCurbe.DW52.ROI2_Y_bottom = params->pPicParams->DirtyROI[1].Bottom;

        cmd.m_encCurbe.DW53.ROI3_X_left   = params->pPicParams->DirtyROI[2].Left;
        cmd.m_encCurbe.DW53.ROI3_Y_top    = params->pPicParams->DirtyROI[2].Top;
        cmd.m_encCurbe.DW54.ROI3_X_right  = params->pPicParams->DirtyROI[2].Right;
        cmd.m_encCurbe.DW54.ROI3_Y_bottom = params->pPicParams->DirtyROI[2].Bottom;

        cmd.m_encCurbe.DW55.ROI4_X_left   = params->pPicParams->DirtyROI[3].Left;
        cmd.m_encCurbe.DW55.ROI4_Y_top    = params->pPicParams->DirtyROI[3].Top;
        cmd.m_encCurbe.DW56.ROI4_X_right  = params->pPicParams->DirtyROI[3].Right;
        cmd.m_encCurbe.DW56.ROI4_Y_bottom = params->pPicParams->DirtyROI[3].Bottom;
    }

    if (m_trellisQuantParams.dwTqEnabled)
    {
        // Lambda values for TQ
        if (m_pictureCodingType == I_TYPE)
        {
            cmd.m_encCurbe.DW58.Value = TQ_LAMBDA_I_FRAME[sliceQP][0];
            cmd.m_encCurbe.DW59.Value = TQ_LAMBDA_I_FRAME[sliceQP][1];
        }
        else if (m_pictureCodingType == P_TYPE)
        {
            cmd.m_encCurbe.DW58.Value = TQ_LAMBDA_P_FRAME[sliceQP][0];
            cmd.m_encCurbe.DW59.Value = TQ_LAMBDA_P_FRAME[sliceQP][1];
        }
        else
        {
            cmd.m_encCurbe.DW58.Value = TQ_LAMBDA_B_FRAME[sliceQP][0];
            cmd.m_encCurbe.DW59.Value = TQ_LAMBDA_B_FRAME[sliceQP][1];
        }

        MHW_VDBOX_AVC_SLICE_STATE sliceState;
        MOS_ZeroMemory(&sliceState, sizeof(sliceState));
        sliceState.pEncodeAvcSeqParams = seqParams;
        sliceState.pEncodeAvcPicParams = picParams;
        sliceState.pEncodeAvcSliceParams = slcParams;

        // check if Lambda is greater than max value
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

        if (cmd.m_encCurbe.DW58.Lambda_8x8Inter > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.m_encCurbe.DW58.Lambda_8x8Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.m_encCurbe.DW58.Lambda_8x8Intra > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.m_encCurbe.DW58.Lambda_8x8Intra = 0xf000 + m_defaultTrellisQuantIntraRounding;
        }

        // check if Lambda is greater than max value
        if (cmd.m_encCurbe.DW59.Lambda_Inter > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.m_encCurbe.DW59.Lambda_Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.m_encCurbe.DW59.Lambda_Intra > CODECHAL_ENCODE_AVC_MAX_LAMBDA)
        {
            cmd.m_encCurbe.DW59.Lambda_Intra = 0xf000 + m_defaultTrellisQuantIntraRounding;
        }
    }

    //IPCM QP and threshold
    cmd.m_encCurbe.DW62.IPCM_QP0 = m_IPCMThresholdTable[0].QP;
    cmd.m_encCurbe.DW62.IPCM_QP1 = m_IPCMThresholdTable[1].QP;
    cmd.m_encCurbe.DW62.IPCM_QP2 = m_IPCMThresholdTable[2].QP;
    cmd.m_encCurbe.DW62.IPCM_QP3 = m_IPCMThresholdTable[3].QP;
    cmd.m_encCurbe.DW63.IPCM_QP4 = m_IPCMThresholdTable[4].QP;

    cmd.m_encCurbe.DW63.IPCM_Thresh0 = m_IPCMThresholdTable[0].Threshold;
    cmd.m_encCurbe.DW64.IPCM_Thresh1 = m_IPCMThresholdTable[1].Threshold;
    cmd.m_encCurbe.DW64.IPCM_Thresh2 = m_IPCMThresholdTable[2].Threshold;
    cmd.m_encCurbe.DW65.IPCM_Thresh3 = m_IPCMThresholdTable[3].Threshold;
    cmd.m_encCurbe.DW65.IPCM_Thresh4 = m_IPCMThresholdTable[4].Threshold;

    cmd.m_encCurbe.DW66.MBDataSurfIndex               = mbEncMfcAvcPakObj;
    cmd.m_encCurbe.DW67.MVDataSurfIndex               = mbEncIndirectMvData;
    cmd.m_encCurbe.DW68.IDistSurfIndex                = mbEncBrcDistortion;
    cmd.m_encCurbe.DW69.SrcYSurfIndex                 = mbEncCurrentY;
    cmd.m_encCurbe.DW70.MBSpecificDataSurfIndex       = mbEncMbSpecificData;
    cmd.m_encCurbe.DW71.AuxVmeOutSurfIndex            = mbEncAuxVmeOutput;
    cmd.m_encCurbe.DW72.CurrRefPicSelSurfIndex        = mbEncRefPicSeletcL0;
    cmd.m_encCurbe.DW73.HMEMVPredFwdBwdSurfIndex      = mbEncMvDataFromMe;
    cmd.m_encCurbe.DW74.HMEDistSurfIndex              = mbEnc4xMeDistortion;
    cmd.m_encCurbe.DW75.SliceMapSurfIndex             = mbEncSliceMapData;
    cmd.m_encCurbe.DW76.FwdFrmMBDataSurfIndex         = mbEncFwdMbData;
    cmd.m_encCurbe.DW77.FwdFrmMVSurfIndex             = mbEncFwdMvData;
    cmd.m_encCurbe.DW78.MBQPBuffer                    = mbEncMbQP;
    cmd.m_encCurbe.DW79.MBBRCLut                      = mbEncMbBrcConstData;
    cmd.m_encCurbe.DW80.VMEInterPredictionSurfIndex   = mbEncVmeInterPredCurrPic;
    cmd.m_encCurbe.DW81.VMEInterPredictionMRSurfIndex = mbEncVmeInterPredMultiRefCurrPic;
    cmd.m_encCurbe.DW82.MBStatsSurfIndex              = mbEncMbStats;
    cmd.m_encCurbe.DW83.MADSurfIndex                  = mbEncMadData;
    cmd.m_encCurbe.DW84.BRCCurbeSurfIndex             = mbEncBrcCurbeData;
    cmd.m_encCurbe.DW85.ForceNonSkipMBmapSurface      = mbEncForceNonSkipMbMap;
    cmd.m_encCurbe.DW86.ReservedIndex                 = mbEncAdv;
    cmd.m_encCurbe.DW87.StaticDetectionCostTableIndex = mbEncSfdCostTable;
    auto pStateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
            meMethod,
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto picParams = m_avcPicParam;
    auto seqParams = m_avcSeqParam;
    auto vuiParams = m_avcVuiParams;

    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    BrcInitResetCurbe cmd;
    cmd.m_dw0.ProfileLevelMaxFrame = profileLevelMaxFrame;
    cmd.m_dw1.InitBufFullInBits    = seqParams->InitVBVBufferFullnessInBit;
    cmd.m_dw2.BufSizeInBits        = seqParams->VBVBufferSizeInBit;
    cmd.m_dw3.AverageBitRate       = seqParams->TargetBitRate;
    cmd.m_dw4.MaxBitRate           = seqParams->MaxBitRate;
    cmd.m_dw8.GopP                 = (seqParams->GopRefDist) ? ((seqParams->GopPicSize - 1) / seqParams->GopRefDist) : 0;
    cmd.m_dw9.GopB                 = seqParams->GopPicSize - 1 - cmd.m_dw8.GopP;
    cmd.m_dw9.FrameWidthInBytes    = m_frameWidth;
    cmd.m_dw10.FrameHeightInBytes  = m_frameHeight;
    cmd.m_dw12.NoSlices            = m_numSlices;

    cmd.m_dw32.SurfaceIndexhistorybuffer    = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_HISTORY;
    cmd.m_dw33.SurfaceIndexdistortionbuffer = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_DISTORTION;

    // if VUI present, VUI data has high priority
    if (seqParams->vui_parameters_present_flag && seqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        cmd.m_dw4.MaxBitRate = ((vuiParams->bit_rate_value_minus1[0] + 1) << (6 + vuiParams->bit_rate_scale));

        if (seqParams->RateControlMethod == RATECONTROL_CBR)
        {
            cmd.m_dw3.AverageBitRate = cmd.m_dw4.MaxBitRate;
        }
    }

    cmd.m_dw6.FrameRateM = seqParams->FramesPer100Sec;
    cmd.m_dw7.FrameRateD = 100;
    cmd.m_dw8.BRCFlag    = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    cmd.m_dw8.BRCFlag |= (bMbBrcEnabled && !bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        cmd.m_dw4.MaxBitRate = cmd.m_dw3.AverageBitRate;
        cmd.m_dw8.BRCFlag    = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (cmd.m_dw4.MaxBitRate < cmd.m_dw3.AverageBitRate)
        {
            cmd.m_dw3.AverageBitRate = cmd.m_dw4.MaxBitRate;  // Use max bit rate for HRD compliance
        }
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        cmd.m_dw4.MaxBitRate = cmd.m_dw3.AverageBitRate;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        cmd.m_dw23.ACQP   = seqParams->ICQQualityFactor;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (cmd.m_dw4.MaxBitRate < cmd.m_dw3.AverageBitRate)
        {
            cmd.m_dw3.AverageBitRate = cmd.m_dw4.MaxBitRate;  // Use max bit rate for HRD compliance
        }
        cmd.m_dw8.BRCFlag = cmd.m_dw8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        cmd.m_dw23.ACQP = seqParams->ICQQualityFactor;
    }

    cmd.m_dw10.AVBRAccuracy    = usAVBRAccuracy;
    cmd.m_dw11.AVBRConvergence = usAVBRConvergence;

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(cmd.m_dw4.MaxBitRate) * (double)(cmd.m_dw7.FrameRateD) /
            (double)(cmd.m_dw6.FrameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        inputBitsPerFrame *= 0.5;
    }

    if (cmd.m_dw2.BufSizeInBits == 0)
    {
        cmd.m_dw2.BufSizeInBits = (uint32_t)inputBitsPerFrame * 4;
    }

    if (cmd.m_dw1.InitBufFullInBits == 0)
    {
        cmd.m_dw1.InitBufFullInBits = 7 * cmd.m_dw2.BufSizeInBits / 8;
    }
    if (cmd.m_dw1.InitBufFullInBits < (uint32_t)(inputBitsPerFrame * 2))
    {
        cmd.m_dw1.InitBufFullInBits = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (cmd.m_dw1.InitBufFullInBits > cmd.m_dw2.BufSizeInBits)
    {
        cmd.m_dw1.InitBufFullInBits = cmd.m_dw2.BufSizeInBits;
    }

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        cmd.m_dw2.BufSizeInBits     = 2 * seqParams->TargetBitRate;
        cmd.m_dw1.InitBufFullInBits = (uint32_t)(0.75 * cmd.m_dw2.BufSizeInBits);
    }

    double bpsRatio = inputBitsPerFrame / ((double)(cmd.m_dw2.BufSizeInBits) / 30);
    bpsRatio = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    cmd.m_dw16.DeviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, bpsRatio));
    cmd.m_dw16.DeviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, bpsRatio));
    cmd.m_dw16.DeviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, bpsRatio));
    cmd.m_dw16.DeviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.m_dw17.DeviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, bpsRatio));
    cmd.m_dw17.DeviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, bpsRatio));
    cmd.m_dw17.DeviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, bpsRatio));
    cmd.m_dw17.DeviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, bpsRatio));
    cmd.m_dw18.DeviationThreshold0ForVBR   = (uint32_t)(-50 * pow(0.9, bpsRatio));
    cmd.m_dw18.DeviationThreshold1ForVBR   = (uint32_t)(-50 * pow(0.7, bpsRatio));
    cmd.m_dw18.DeviationThreshold2ForVBR   = (uint32_t)(-50 * pow(0.5, bpsRatio));
    cmd.m_dw18.DeviationThreshold3ForVBR   = (uint32_t)(-50 * pow(0.3, bpsRatio));
    cmd.m_dw19.DeviationThreshold4ForVBR   = (uint32_t)(100 * pow(0.4, bpsRatio));
    cmd.m_dw19.DeviationThreshold5ForVBR   = (uint32_t)(100 * pow(0.5, bpsRatio));
    cmd.m_dw19.DeviationThreshold6ForVBR   = (uint32_t)(100 * pow(0.75, bpsRatio));
    cmd.m_dw19.DeviationThreshold7ForVBR   = (uint32_t)(100 * pow(0.9, bpsRatio));
    cmd.m_dw20.DeviationThreshold0ForI     = (uint32_t)(-50 * pow(0.8, bpsRatio));
    cmd.m_dw20.DeviationThreshold1ForI     = (uint32_t)(-50 * pow(0.6, bpsRatio));
    cmd.m_dw20.DeviationThreshold2ForI     = (uint32_t)(-50 * pow(0.34, bpsRatio));
    cmd.m_dw20.DeviationThreshold3ForI     = (uint32_t)(-50 * pow(0.2, bpsRatio));
    cmd.m_dw21.DeviationThreshold4ForI     = (uint32_t)(50 * pow(0.2, bpsRatio));
    cmd.m_dw21.DeviationThreshold5ForI     = (uint32_t)(50 * pow(0.4, bpsRatio));
    cmd.m_dw21.DeviationThreshold6ForI     = (uint32_t)(50 * pow(0.66, bpsRatio));
    cmd.m_dw21.DeviationThreshold7ForI     = (uint32_t)(50 * pow(0.9, bpsRatio));

    cmd.m_dw22.SlidingWindowSize = dwSlidingWindowSize;

    if (bBrcInit)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = cmd.m_dw1.InitBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits    = cmd.m_dw2.BufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetCurbeAvcBrcBlockCopy(PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    BrcBlockCopyCurbe cmd;
    cmd.m_brcBlockCopyCurbeCmd.DW0.BufferOffset    = params->dwBufferOffset;
    cmd.m_brcBlockCopyCurbeCmd.DW0.BlockHeight     = params->dwBlockHeight;
    cmd.m_brcBlockCopyCurbeCmd.DW1.SrcSurfaceIndex = 0x00;
    cmd.m_brcBlockCopyCurbeCmd.DW2.DstSurfaceIndex = 0x01;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;
    auto slcParams = m_avcSliceParams;

    BrcFrameUpdateCurbe cmd;

    cmd.m_dw5.TargetSizeFlag = 0;
    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        cmd.m_dw5.TargetSizeFlag = 1;
    }

    // skipped frame handling
    if (params->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        cmd.m_dw6.NumSkipFrames  = params->dwNumSkipFrames;
        cmd.m_dw7.SizeSkipFrames = params->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * params->dwNumSkipFrames;
    }

    cmd.m_dw0.TargetSize       = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);
    cmd.m_dw1.FrameNumber      = m_storeData - 1;
    cmd.m_dw2.SizeofPicHeaders = m_headerBytesInserted << 3;  // kernel uses how many bits instead of bytes
    cmd.m_dw5.CurrFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    cmd.m_dw5.BRCFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? brcUpdateIsField : ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (brcUpdateIsField | brcUpdateIsBottomField) : 0);
    cmd.m_dw5.BRCFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ? brcUpdateIsReference : 0;

    if (bMultiRefQpEnabled)
    {
        cmd.m_dw5.BRCFlag |= brcUpdateIsActualQp;
        cmd.m_dw14.QPIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    cmd.m_dw5.BRCFlag |= seqParams->bAutoMaxPBFrameSizeForSceneChange ? brcUpdateAutoPbFrameSize : 0;

    cmd.m_dw5.MaxNumPAKs = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    cmd.m_dw6.MinimumQP            = params->ucMinQP;
    cmd.m_dw6.MaximumQP            = params->ucMaxQP;
    cmd.m_dw6.EnableForceToSkip    = (bForceToSkipEnable && !m_avcPicParam->bDisableFrameSkip);
    cmd.m_dw6.EnableSlidingWindow  = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    cmd.m_dw6.EnableExtremLowDelay = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);
    cmd.m_dw6.DisableVarCompute    = bBRCVarCompuBypass;

    *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_dw3.startGAdjFrame0 = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        cmd.m_dw3.startGAdjFrame1 = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        cmd.m_dw4.startGAdjFrame2 = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        cmd.m_dw4.startGAdjFrame3 = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        cmd.m_dw11.gRateRatioThreshold0 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 40)));
        cmd.m_dw11.gRateRatioThreshold1 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 75)));
        cmd.m_dw12.gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 97)));
        cmd.m_dw12.gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (103 - 100)));
        cmd.m_dw12.gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (125 - 100)));
        cmd.m_dw12.gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (160 - 100)));
    }

    cmd.m_dw15.EnableROI = params->ucEnableROI;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams = seqParams;
    sliceState.pEncodeAvcPicParams = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

    cmd.m_dw15.RoundingIntra = 5;
    cmd.m_dw15.RoundingInter = sliceState.dwRoundingValue;

    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    cmd.m_dw19.UserMaxFrame                                  = profileLevelMaxFrame;
    cmd.m_dw24.SurfaceIndexBRChistorybuffer                  = frameBrcUpdateHistory;
    cmd.m_dw25.SurfaceIndexPreciousPAKstatisticsoutputbuffer = frameBrcUpdatePakStatisticsOutput;
    cmd.m_dw26.SurfaceIndexAVCIMGstateinputbuffer            = frameBrcUpdateImageStateRead;
    cmd.m_dw27.SurfaceIndexAVCIMGstateoutputbuffer           = frameBrcUpdateImageStateWrite;
    cmd.m_dw28.SurfaceIndexAVC_Encbuffer                     = frameBrcUpdateMbEncCurbeWrite;
    cmd.m_dw29.SurfaceIndexAVCDISTORTIONbuffer               = frameBrcUpdateDistortion;
    cmd.m_dw30.SurfaceIndexBRCconstdatabuffer                = frameBrcUpdateConstantData;
    cmd.m_dw31.SurfaceIndexMBStatsBuffer                     = frameBrcUpdateMbStat;
    cmd.m_dw32.SurfaceIndexMotionvectorbuffer                = frameBrcUpdateMvStat;
    auto pStateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            &cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    int16_t    weight;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto slcParams = m_avcSliceParams;
    auto seqParams = m_avcSeqParam;
    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    WPCurbe cmd;
    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    weight = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][0];
        cmd.m_wpCurbeCmd.DW0.DefaultWeight = (weight << 6) >> (slcParams->luma_log2_weight_denom);
        cmd.m_wpCurbeCmd.DW0.DefaultOffset = slcParams->Weights[params->RefPicListIdx][0][0][1];

        cmd.m_wpCurbeCmd.DW49.InputSurface  = wpInputRefSurface;
        cmd.m_wpCurbeCmd.DW50.OutputSurface = wpOutputScaledSurface;

        auto kernelState = pWPKernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
            &cmd,
            kernelState->dwCurbeOffset,
            sizeof(cmd)));

        return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    MbBrcUpdateCurbe curbe;

    // BRC curbe requires: 2 for I-frame, 0 for P-frame, 1 for B-frame
    curbe.m_dw0.CurrFrameType = (m_pictureCodingType + 1) % 3;
    if (params->ucEnableROI)
    {
        if (bROIValueInDeltaQP)
        {
            curbe.m_dw0.EnableROI = 2;  // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled
            curbe.m_dw0.ROIRatio  = 0;
        }
        else
        {
            curbe.m_dw0.EnableROI = 1;  // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled

            uint32_t roiSize = 0;
            uint32_t roiRatio = 0;

            for (uint32_t i = 0; i < m_avcPicParam->NumROI; ++i)
            {
                CODECHAL_ENCODE_VERBOSEMESSAGE("ROI[%d] = {%d, %d, %d, %d} {%d}, size = %d", i,
                    m_avcPicParam->ROI[i].Left, m_avcPicParam->ROI[i].Top,
                    m_avcPicParam->ROI[i].Bottom, m_avcPicParam->ROI[i].Right,
                    m_avcPicParam->ROI[i].PriorityLevelOrDQp,
                    (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left)));
                roiSize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left));
            }

            if (roiSize)
            {
                uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
                roiRatio = 2 * (numMBs * 256 / roiSize - 1);
                roiRatio = MOS_MIN(51, roiRatio); // clip QP from 0-51
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("ROIRatio = %d", roiRatio);
            curbe.m_dw0.ROIRatio = roiRatio;
        }
    }
    else
    {
        curbe.m_dw0.ROIRatio = 0;
    }

    curbe.m_dw8.HistorybufferIndex        = mbBrcUpdateHistory;
    curbe.m_dw9.MBQPbufferIndex           = mbBrcUpdateMbQP;
    curbe.m_dw10.ROIbufferIndex           = mbBrcUpdateROI;
    curbe.m_dw11.MBstatisticalbufferIndex = mbBrcUpdateMbStat;
    auto pStateHeapInterface            = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ppRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psCurrPicSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMbEncBindingTable);

    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    auto currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto mbCodeBuffer = &currPicRefListEntry->resRefMbCodeBuffer;
    auto mvDataBuffer = &currPicRefListEntry->resRefMvDataBuffer;
    uint32_t refMbCodeBottomFieldOffset = params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * 64;
    uint32_t refMvBottomFieldOffset = MOS_ALIGN_CEIL(params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * (32 * 4), 0x1000);

    uint8_t vDirection, refVDirection;
    if (params->bMbEncIFrameDistInUse)
    {
        vDirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        vDirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
            (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    auto kernelState = params->pKernelState;
    auto bindingTable = params->pMbEncBindingTable;
    // PAK Obj command buffer
    uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mbCodeBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMbCodeBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMfcAvcPakObj;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mvDataBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncIndMVData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = bindingTable->dwAvcMBEncCurrUV;
    surfaceCodecParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME MV data buffer
    if (params->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMEDist;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbConstDataBufferInUse)
    {
        // 16 DWs per QP value
        size = 16 * 52 * sizeof(uint32_t);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbBrcConstDataBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbQpBufferInUse)
    {
        // AVC MB BRC QP buffer
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMbQpBuffer;
        surfaceCodecParams.dwOffset = params->dwMbQpBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? bindingTable->dwAvcMBEncMbQpField :
            bindingTable->dwAvcMBEncMbQpFrame;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbSpecificDataEnabled)
    {
        size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * sizeof(CODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS);
        CODECHAL_ENCODE_VERBOSEMESSAGE("Send MB specific surface, size = %d", size);
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMbSpecificDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y - VME
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        bindingTable->dwAvcMBEncFieldCurrPic[0] : bindingTable->dwAvcMBEncCurrPicFrame[0];
    surfaceCodecParams.ucVDirection = vDirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        bindingTable->dwAvcMBEncFieldCurrPic[1] : bindingTable->dwAvcMBEncCurrPicFrame[1];
    surfaceCodecParams.ucVDirection = vDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Setup references 1...n
    // LIST 0 references
    uint8_t refIdx;
    for (refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        auto refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool  refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            uint32_t refBindingTableOffset;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture) // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refBindingTableOffset = bindingTable->dwAvcMBEncFwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refBindingTableOffset = bindingTable->dwAvcMBEncFwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refBindingTableOffset = bindingTable->dwAvcMBEncFwdPicFrame[refIdx];
            }

            // Picture Y VME
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if ((params->bUseWeightedSurfaceForL0) &&
                (params->pAvcSlcParams->luma_weight_flag[LIST_0] & (1 << refIdx)) &&
                (refIdx < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L0_START + refIdx].sBuffer;
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        if (!currFieldPicture && refIdx > 0)
        {
            // Only 1 LIST 1 reference required here since only single ref is supported in frame case
            break;
        }

        auto refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        uint32_t refBindingTableOffset;
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture) // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refMbCodeBottomFieldOffsetUsed = refMbCodeBottomFieldOffset;
                    refMvBottomFieldOffsetUsed = refMvBottomFieldOffset;
                    refBindingTableOffset = bindingTable->dwAvcMBEncBwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed = 0;
                    refBindingTableOffset = bindingTable->dwAvcMBEncBwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refMbCodeBottomFieldOffsetUsed = 0;
                refMvBottomFieldOffsetUsed = 0;
                refBindingTableOffset = bindingTable->dwAvcMBEncBwdPicFrame[refIdx];
            }

            // Picture Y VME
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if ((params->bUseWeightedSurfaceForL1) &&
                (params->pAvcSlcParams->luma_weight_flag[LIST_1] & (1 << refIdx)) &&
                (refIdx < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
            {
                surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L1_START + refIdx].sBuffer;
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }

            surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
            surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
            surfaceCodecParams.ucVDirection = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            if (refIdx == 0)
            {
                if (currFieldPicture && (params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_FRAME || params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_INVALID))
                {
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed = 0;
                }
                // MB data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMbCodeBuffer;
                surfaceCodecParams.dwOffset = refMbCodeBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                // MV data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMvDataBuffer;
                surfaceCodecParams.dwOffset = refMvBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncBwdRefMVData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            if (refIdx < CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
            {
                if (currFieldPicture)
                {
                    // The binding table contains multiple entries for IDX0 backwards references
                    if (refBottomField)
                    {
                        refBindingTableOffset = bindingTable->dwAvcMBEncBwdPicBotField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        refBindingTableOffset = bindingTable->dwAvcMBEncBwdPicTopField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    refBindingTableOffset = bindingTable->dwAvcMBEncBwdPicFrame[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.bUseAdvState = true;
                surfaceCodecParams.dwWidthInUse = params->dwFrameWidthInMb * 16;
                surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
                surfaceCodecParams.dwBindingTableOffset = refBindingTableOffset;
                surfaceCodecParams.ucVDirection = refVDirection;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }

    // BRC distortion data buffer for I frame
    if (params->bMbEncIFrameDistInUse)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncBRCDist;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // RefPicSelect of Current Picture
    if (params->bUsedAsRef)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &currPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        surfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(surfaceCodecParams.psSurface->dwHeight, 8);
        surfaceCodecParams.dwOffset = params->dwRefPicSelectBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncRefPicSelectL0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMBVProcStatsEnabled)
    {
        size = (currFieldPicture ? 1 : 2) * params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * sizeof(uint32_t);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMBVProcStatsBuffer;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwMBVProcStatsBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMBStats;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else if (params->bFlatnessCheckEnabled)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psFlatnessCheckSurface;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwFlatnessCheckBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncFlatnessChk;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMADEnabled)
    {
        size = CODECHAL_MAD_BUFFER_SIZE;

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bRawSurface = true;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMADDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMADData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->dwMbEncBRCBufferSize > 0)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbEncBRCBuffer;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMbEncBRCCurbeData;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        uint32_t curbeSize;
        if (params->bUseMbEncAdvKernel)
        {
            // For BRC the new BRC surface is used
            if (params->bUseAdvancedDsh)
            {
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
                curbeSize = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMbEncBRCCurbeData;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
            else // For CQP the DSH CURBE is used
            {
                MOS_RESOURCE *dsh = nullptr;
                CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = params->pKernelState->m_dshRegion.GetResource());
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.presBuffer = dsh;
                surfaceCodecParams.dwOffset =
                    params->pKernelState->m_dshRegion.GetOffset() +
                    params->pKernelState->dwCurbeOffset;
                curbeSize = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMbEncBRCCurbeData;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }

    if (params->bArbitraryNumMbsInSlice)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psSliceMapSurface;
        surfaceCodecParams.bRenderTarget = false;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwSliceMapBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncSliceMapData;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (!params->bMbEncIFrameDistInUse)
    {
        if (params->bMbDisableSkipMapEnabled)
        {
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.bMediaBlockRW = true;
            surfaceCodecParams.psSurface = params->psMbDisableSkipMapSurface;
            surfaceCodecParams.dwOffset = 0;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncMbNonSkipMap;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        if (params->bStaticFrameDetectionEnabled)
        {
            // static frame cost table surface
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.presBuffer = params->presSFDCostTableBuffer;
            surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_sfdCostTableBufferSize);
            surfaceCodecParams.dwOffset = 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwAvcMBEncStaticDetectionCostTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SendAvcBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    auto kernelState = params->pKernelState;
    auto brcUpdateBindingTable = params->pBrcUpdateBindingTable;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcHistoryBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK Statistics buffer
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcPakStatisticBuffer[0];
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcPakStatisticsSize);
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t size = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses());
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer =
        &params->pBrcBuffers->resBrcImageStatesReadBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcImageStateReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - write only
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcImageStatesWriteBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcImageStateWriteBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->dwMbEncBRCBufferSize > 0)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = &params->pBrcBuffers->resMbEncBrcBuffer;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        PMHW_KERNEL_STATE mbEncKernelState;
        CODECHAL_ENCODE_CHK_NULL_RETURN(mbEncKernelState = params->pBrcBuffers->pMbEncKernelStateInUse);

        MOS_RESOURCE *dsh = nullptr;
        CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = mbEncKernelState->m_dshRegion.GetResource());
        // BRC ENC CURBE Buffer - read only
        size = MOS_ALIGN_CEIL(
            mbEncKernelState->KernelParams.iCurbeLength,
            m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = dsh;
        surfaceCodecParams.dwOffset =
            mbEncKernelState->m_dshRegion.GetOffset() +
            mbEncKernelState->dwCurbeOffset;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // BRC ENC CURBE Buffer - write only
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        if (params->bUseAdvancedDsh)
        {
            surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
        }
        else
        {
            surfaceCodecParams.presBuffer = dsh;
            surfaceCodecParams.dwOffset =
                mbEncKernelState->m_dshRegion.GetOffset() +
                mbEncKernelState->dwCurbeOffset;
        }
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // AVC_ME BRC Distortion data buffer - input/output
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = &params->pBrcBuffers->sMeBrcDistortionBuffer;
    surfaceCodecParams.dwOffset = params->pBrcBuffers->dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcDistortionBuffer;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Constant Data Surface
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface =
        &params->pBrcBuffers->sBrcConstantDataBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MBStat buffer - input
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presMbStatBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->psMvDataBuffer)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcMvDataBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SendAvcBrcMbUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    auto kernelState = params->pKernelState;
    auto bindingTable = params->pBrcUpdateBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwMbBrcHistoryBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC MB QP data buffer
    if (params->bMbBrcEnabled)
    {
        params->pBrcBuffers->sBrcMbQpBuffer.dwHeight = MOS_ALIGN_CEIL((params->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.psSurface = &params->pBrcBuffers->sBrcMbQpBuffer;
        surfaceCodecParams.dwOffset = params->pBrcBuffers->dwBrcMbQpBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwMbBrcMbQpBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // BRC ROI feature
    if (params->bBrcRoiEnabled)
    {
        params->psRoiSurface->dwHeight = MOS_ALIGN_CEIL((params->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.psSurface = params->psRoiSurface;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwMbBrcROISurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // MBStat buffer
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presMbStatBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwMbBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SendAvcWPSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psInputRefBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psOutputScaledBuffer);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    memset((void *)&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psInputRefBuffer;// Input surface
    surfaceParams.bIsWritable = false;
    surfaceParams.bRenderTarget = false;
    surfaceParams.dwBindingTableOffset = wpInputRefSurface;
    surfaceParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection = params->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    memset((void *)&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psOutputScaledBuffer;// output surface
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    surfaceParams.dwBindingTableOffset = wpOutputScaledSurface;
    surfaceParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection = params->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS readOnly;
    memset(&readOnly, 0, sizeof(readOnly));
    readOnly.ReadOnly = 1;
    uint32_t* dataPtr = (uint32_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource, &readOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dataPtr);

    uint32_t bufferWidthInByte = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 4), 64);//(m_picWidthInMb * 4 + 63) & ~63;
    uint32_t bufferHeightInByte = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);//(m_picHeightInMb + 7) & ~7;
    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t mb = 0; mb <= numMBs; mb++)
    {
        int32_t curMbY = mb / m_picWidthInMb;
        int32_t curMbX = mb - curMbY * m_picWidthInMb;

        uint32_t outData = 0;
        for (int32_t roi = (m_avcPicParam->NumROI - 1); roi >= 0; roi--)
        {
            int32_t qpLevel;
            if (bROIValueInDeltaQP)
            {
                qpLevel = -m_avcPicParam->ROI[roi].PriorityLevelOrDQp;
            }
            else
            {
                // QP Level sent to ROI surface is (priority * 6)
                qpLevel = m_avcPicParam->ROI[roi].PriorityLevelOrDQp * 6;
            }

            if (qpLevel == 0)
            {
                continue;
            }

            if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right) &&
                (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom))
            {
                outData = 15 | ((qpLevel & 0xFF) << 8);
            }
            else if (bROISmoothEnabled)
            {
                if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left - 1) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right + 1) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top - 1) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom + 1))
                {
                    outData = 14 | ((qpLevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left - 2) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right + 2) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top - 2) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom + 2))
                {
                    outData = 13 | ((qpLevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[roi].Left - 3) && (curMbX < (int32_t)m_avcPicParam->ROI[roi].Right + 3) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roi].Top - 3) && (curMbY < (int32_t)m_avcPicParam->ROI[roi].Bottom + 3))
                {
                    outData = 12 | ((qpLevel & 0xFF) << 8);
                }
            }
        }
        dataPtr[(curMbY * (bufferWidthInByte >> 2)) + curMbX] = outData;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource);

    uint32_t bufferSize = bufferWidthInByte * bufferHeightInByte;
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &BrcBuffers.sBrcRoiSurface.OsResource,
        CodechalDbgAttr::attrInput,
        "ROI",
        bufferSize,
        0,
        CODECHAL_MEDIA_STATE_MB_BRC_UPDATE)));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf"));
    );

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

    // Scaling, BRC Init/Reset and HME are included in the same task phase
    m_lastEncPhase = false;
    m_firstTaskInPhase = true;

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    if (bBrcEnabled && (bBrcInit || bBrcReset))
    {
        bool cscEnabled = m_cscDsState->RequireCsc() && m_firstField;
        m_lastTaskInPhase = !(cscEnabled || m_scalingEnabled || m_16xMeSupported || m_hmeEnabled);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcInitResetKernel());
    }

    UpdateSSDSliceCount();

    if (m_firstField)
    {
        // Csc, Downscaling, and/or 10-bit to 8-bit conversion
        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        memset((void *)&cscScalingKernelParams, 0, sizeof(cscScalingKernelParams));
        cscScalingKernelParams.bLastTaskInPhaseCSC =
            cscScalingKernelParams.bLastTaskInPhase4xDS = !(m_16xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase16xDS = !(m_32xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase32xDS = !m_hmeEnabled;
        cscScalingKernelParams.inputColorSpace = m_avcSeqParam->InputColorSpace;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }

    // SFD should be called only when HME enabled
    auto staticFrameDetectionInUse = bStaticFrameDetectionEnable && m_hmeEnabled;

    staticFrameDetectionInUse = !bPerMbSFD && staticFrameDetectionInUse;

    if (m_hmeEnabled)
    {
        if (m_16xMeEnabled)
        {
            m_lastTaskInPhase = false;
            if (m_32xMeEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(GenericEncodeMeKernel(&BrcBuffers, HME_LEVEL_32x));
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(GenericEncodeMeKernel(&BrcBuffers, HME_LEVEL_16x));
        }
        m_lastTaskInPhase = !staticFrameDetectionInUse;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GenericEncodeMeKernel(&BrcBuffers, HME_LEVEL_4x));
    }

    // Scaling and HME are not dependent on the output from PAK
    if (m_waitForPak && m_semaphoreObjCount && !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        // Wait on PAK
        auto syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        syncParams.uiSemaphoreCount = m_semaphoreObjCount;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_semaphoreObjCount = 0; //reset
    }

    // BRC and MbEnc are included in the same task phase
    m_lastEncPhase = true;
    m_firstTaskInPhase = true;

    if (bBrcEnabled)
    {
        if (bMbEncIFrameDistEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(true));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcFrameUpdateKernel());
        if (bBrcSplitEnable && bMbBrcEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcMbUpdateKernel());
        }

        // Reset buffer ID used for BRC kernel performance reports
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }

    bUseWeightedSurfaceForL0 = false;
    bUseWeightedSurfaceForL1 = false;
    auto slcParams = m_avcSliceParams;
    auto sliceType = Slice_Type[slcParams->slice_type];

    if (bWeightedPredictionSupported &&
        ((((sliceType == SLICE_P) || (sliceType == SLICE_SP)) && (m_avcPicParam->weighted_pred_flag)) ||
        (((sliceType == SLICE_B)) && (m_avcPicParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE)))
        )
    {
        uint8_t i;
        // Weighted Prediction to be applied for L0
        for (i = 0; i < (m_avcPicParam->num_ref_idx_l0_active_minus1 + 1); i++)
        {
            if ((slcParams->luma_weight_flag[LIST_0] & (1 << i)) && (i < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                //Weighted Prediction for ith forward reference frame
                CODECHAL_ENCODE_CHK_STATUS_RETURN(WPKernel(false, i));
            }
        }

        if (((sliceType == SLICE_B)) &&
            (m_avcPicParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))
        {
            for (i = 0; i < (m_avcPicParam->num_ref_idx_l1_active_minus1 + 1); i++)
            {
                // Weighted Pred to be applied for L1
                if ((slcParams->luma_weight_flag[LIST_1] & 1 << i) && (i < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
                {
                    //Weighted Prediction for ith backward reference frame
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(WPKernel(false, i));
                }
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_USER_FEATURE_VALUE_WRITE_DATA   userFeatureWriteData;

    // Weighted prediction for L0 Reporting
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = bUseWeightedSurfaceForL0;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L0_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

    // Weighted prediction for L1 Reporting
    userFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = bUseWeightedSurfaceForL1;
    userFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L1_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

#endif // _DEBUG || _RELEASE_INTERNAL

    m_lastTaskInPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(false));

    // Reset buffer ID used for MbEnc kernel performance reports
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        auto syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

    if (m_madEnabled)
    {
        m_currMadBufferIdx = (m_currMadBufferIdx + 1) % CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS;
    }

    // Reset after BRC Init has been processed
    bBrcInit = false;

    m_setRequestedEUSlices = false;

    //CODECHAL_DEBUG_TOOL(
    //    KernelDebugDumps();
    //);

    if (bBrcEnabled)
    {
        bMbEncCurbeSetInBrcUpdate = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::SceneChangeReport(PMOS_COMMAND_BUFFER    cmdBuffer, PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS params)
{

    MHW_MI_COPY_MEM_MEM_PARAMS                      copyMemMemParams;
    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize)
        + (sizeof(uint32_t) * 2) + m_encodeStatusBuf.dwSceneChangedOffset;

    MOS_ZeroMemory(&copyMemMemParams, sizeof(copyMemMemParams));
    copyMemMemParams.presSrc = params->presBrcHistoryBuffer;
    copyMemMemParams.dwSrcOffset = m_brcHistoryBufferOffsetSceneChanged;
    copyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
    copyMemMemParams.dwDstOffset = offset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
        &copyMemMemParams));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeAvcEncG10::KernelDebugDumps()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DEBUG_TOOL(
      if (m_hmeEnabled)
      {
        CODECHAL_ME_OUTPUT_PARAMS meOutputParams;
        memset((void *)&meOutputParams, 0, sizeof(meOutputParams));
        meOutputParams.psMeMvBuffer = &m_4xMeMvDataBuffer;
        meOutputParams.psMeBrcDistortionBuffer =
            bBrcDistortionBufferSupported ? &BrcBuffers.sMeBrcDistortionBuffer : nullptr;
        meOutputParams.psMeDistortionBuffer =
            m_4xMeDistortionBufferSupported ? &m_4xMeDistortionBuffer : nullptr;
        meOutputParams.b16xMeInUse = false;
        meOutputParams.b32xMeInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &meOutputParams.psMeMvBuffer->OsResource,
            CodechalDbgAttr::attrOutput,
            "MvData",
            meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
            CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
            CODECHAL_MEDIA_STATE_4X_ME));
        if (meOutputParams.psMeBrcDistortionBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &meOutputParams.psMeBrcDistortionBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "BrcDist",
                meOutputParams.psMeBrcDistortionBuffer->dwHeight *meOutputParams.psMeBrcDistortionBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));
            if (meOutputParams.psMeDistortionBuffer)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MeDist",
                    meOutputParams.psMeDistortionBuffer->dwHeight *meOutputParams.psMeDistortionBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4 * 10), 8) : 0,
                    CODECHAL_MEDIA_STATE_4X_ME));
            }
        }

        if (m_16xMeEnabled)
        {
            meOutputParams.psMeMvBuffer = &m_16xMeMvDataBuffer;
            meOutputParams.psMeBrcDistortionBuffer = nullptr;
            meOutputParams.psMeDistortionBuffer = nullptr;
            meOutputParams.b16xMeInUse = true;
            meOutputParams.b32xMeInUse = false;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeMvBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MvData",
                    meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
                    CODECHAL_MEDIA_STATE_16X_ME));

            if (m_32xMeEnabled)
            {
                meOutputParams.psMeMvBuffer = &m_32xMeMvDataBuffer;
                meOutputParams.psMeBrcDistortionBuffer = nullptr;
                meOutputParams.psMeDistortionBuffer = nullptr;
                meOutputParams.b16xMeInUse = false;
                meOutputParams.b32xMeInUse = true;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    m_debugInterface->DumpBuffer(
                        &meOutputParams.psMeMvBuffer->OsResource,
                        CodechalDbgAttr::attrOutput,
                        "MvData",
                        meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                        CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb32x * 32), 64) * (m_downscaledFrameFieldHeightInMb32x * 4) : 0,
                        CODECHAL_MEDIA_STATE_32X_ME));
            }
        }
    }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &BrcBuffers.resBrcImageStatesWriteBuffer,
            CodechalDbgAttr::attrOutput,
            "ImgStateWrite",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &BrcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrOutput,
            "HistoryWrite",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        if (!Mos_ResourceIsNull(&BrcBuffers.sBrcMbQpBuffer.OsResource))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &BrcBuffers.sBrcMbQpBuffer.OsResource,
                CodechalDbgAttr::attrOutput,
                "MbQp",
                BrcBuffers.sBrcMbQpBuffer.dwPitch*BrcBuffers.sBrcMbQpBuffer.dwHeight,
                BrcBuffers.dwBrcMbQpBottomFieldOffset,
                CODECHAL_MEDIA_STATE_MB_BRC_UPDATE));
        }
        if (BrcBuffers.pMbEncKernelStateInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                CODECHAL_MEDIA_STATE_BRC_UPDATE,
                BrcBuffers.pMbEncKernelStateInUse));
        }
        if (m_mbencBrcBufferSize>0)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &BrcBuffers.resMbEncBrcBuffer,
                CodechalDbgAttr::attrOutput,
                "MbEncBRCWrite",
                m_mbencBrcBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
        }

        if (m_mbStatsSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resMbStatsBuffer,
                CodechalDbgAttr::attrOutput,
                "MBStatsSurf",
                m_picWidthInMb * (((CodecHal_PictureIsField(m_currOriginalPic)) ? 2 : 4) * m_downscaledFrameFieldHeightInMb4x) * 16 * sizeof(uint32_t),
                CodecHal_PictureIsBottomField(m_currOriginalPic)?(m_picWidthInMb * 16 * sizeof(uint32_t) * (2 * m_downscaledFrameFieldHeightInMb4x)):0,
                CODECHAL_MEDIA_STATE_4X_SCALING));
        }
        else if (m_flatnessCheckEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_flatnessCheckSurface.OsResource,
                CodechalDbgAttr::attrOutput,
                "FlatnessChkSurf",
                ((CodecHal_PictureIsField(m_currOriginalPic)) ? m_flatnessCheckSurface.dwHeight / 2 : m_flatnessCheckSurface.dwHeight) * m_flatnessCheckSurface.dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? (m_flatnessCheckSurface.dwPitch * m_flatnessCheckSurface.dwHeight >> 1) : 0,
                CODECHAL_MEDIA_STATE_4X_SCALING));
        }

        if (bMbQpDataEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resMbSpecificDataBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrInput,
                "MbSpecificData",
                m_picWidthInMb*m_frameFieldHeightInMb * 16,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        if (bMbSpecificDataEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resMbSpecificDataBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrInput,
                "MbSpecificData",
                m_picWidthInMb*m_frameFieldHeightInMb*16,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        uint8_t index;
        CODEC_PICTURE refPic;
        if (bUseWeightedSurfaceForL0)
        {
            refPic = m_avcSliceParams->RefPicList[LIST_0][0];
            index = m_picIdx[refPic.FrameIdx].ucPicIdx;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_In_L0"));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &WeightedPredOutputPicSelectList[LIST_0].sBuffer,
                CodechalDbgAttr::attrReferenceSurfaces,
                "WP_Out_L0"));
       }
       if (bUseWeightedSurfaceForL1)
       {

        refPic = m_avcSliceParams->RefPicList[LIST_1][0];
        index = m_picIdx[refPic.FrameIdx].ucPicIdx;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &m_refList[index]->sRefBuffer,
            CodechalDbgAttr::attrReferenceSurfaces,
            "WP_In_L1"));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            &WeightedPredOutputPicSelectList[LIST_1].sBuffer,
            CodechalDbgAttr::attrReferenceSurfaces,
            "WP_Out_L1"));

       }
       if (m_avcFeiPicParams->MVPredictorEnable){
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                 &m_avcFeiPicParams->resMVPredictor,
                 CodechalDbgAttr::attrInput,
                 "MvPredictor",
                 m_picWidthInMb * m_frameFieldHeightInMb *40,
                 0,
                 CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

         if (m_arbitraryNumMbsInSlice)
         {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_sliceMapSurface[m_currRecycledBufIdx].OsResource,
                CodechalDbgAttr::attrInput,
                "SliceMapSurf",
                m_sliceMapSurface[m_currRecycledBufIdx].dwPitch * m_frameFieldHeightInMb,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
         }
    )
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG10::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    BrcInitResetCurbe *curbe = (BrcInitResetCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MBBRCEnable          = bMbBrcEnabled;
        m_avcPar->MBRC                 = bMbBrcEnabled;
        m_avcPar->BitRate              = curbe->m_dw3.AverageBitRate;
        m_avcPar->InitVbvFullnessInBit = curbe->m_dw1.InitBufFullInBits;
        m_avcPar->MaxBitRate           = curbe->m_dw4.MaxBitRate;
        m_avcPar->VbvSzInBit           = curbe->m_dw2.BufSizeInBits;
        m_avcPar->AvbrAccuracy         = curbe->m_dw10.AVBRAccuracy;
        m_avcPar->AvbrConvergence      = curbe->m_dw11.AVBRConvergence;
        m_avcPar->SlidingWindowSize    = curbe->m_dw22.SlidingWindowSize;
        m_avcPar->LongTermInterval     = curbe->m_dw24.LongTermInterval;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG10::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    BrcFrameUpdateCurbe *curbe = (BrcFrameUpdateCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->EnableMultipass     = (curbe->m_dw5.MaxNumPAKs > 0) ? 1 : 0;
        m_avcPar->MaxNumPakPasses     = curbe->m_dw5.MaxNumPAKs;
        m_avcPar->SlidingWindowEnable = curbe->m_dw6.EnableSlidingWindow;
        m_avcPar->FrameSkipEnable     = curbe->m_dw6.EnableForceToSkip;
        m_avcPar->UserMaxFrame        = curbe->m_dw19.UserMaxFrame;
    }
    else
    {
        m_avcPar->UserMaxFrameP = curbe->m_dw19.UserMaxFrame;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG10::PopulateEncParam(
    uint8_t meMethod,
    void    *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    MbEncCurbe *curbe = (MbEncCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MRDisableQPCheck = MRDisableQPCheck[m_targetUsage];
        m_avcPar->AllFractional =
            CODECHAL_ENCODE_AVC_AllFractional_Common[m_targetUsage & 0x7];
        m_avcPar->DisableAllFractionalCheckForHighRes =
            CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[m_targetUsage & 0x7];
        m_avcPar->EnableAdaptiveSearch              = curbe->m_encCurbe.DW37.AdaptiveEn;
        m_avcPar->EnableFBRBypass                   = curbe->m_encCurbe.DW4.EnableFBRBypass;
        m_avcPar->BlockBasedSkip                    = curbe->m_encCurbe.DW3.BlockBasedSkipEnable;
        m_avcPar->MADEnableFlag                     = curbe->m_encCurbe.DW34.MADEnableFlag;
        m_avcPar->MBTextureThreshold                = curbe->m_encCurbe.DW60.MBTextureThreshold;
        m_avcPar->EnableMBFlatnessCheckOptimization = curbe->m_encCurbe.DW34.EnableMBFlatnessChkOptimization;
        m_avcPar->EnableArbitrarySliceSize          = curbe->m_encCurbe.DW34.ArbitraryNumMbsPerSlice;
        m_avcPar->RefThresh                         = curbe->m_encCurbe.DW38.RefThreshold;
        m_avcPar->EnableWavefrontOptimization       = curbe->m_encCurbe.DW4.EnableWavefrontOptimization;
        m_avcPar->MaxLenSP                          = curbe->m_encCurbe.DW2.LenSP;
        m_avcPar->DisableExtendedMvCostRange        = !curbe->m_encCurbe.DW1.ExtendedMvCostRange;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->MEMethod                             = meMethod;
        m_avcPar->HMECombineLen                        = HMECombineLen[m_targetUsage];
        m_avcPar->FTQBasedSkip                         = FTQBasedSkip[m_targetUsage];
        m_avcPar->MultiplePred                         = MultiPred[m_targetUsage];
        m_avcPar->EnableAdaptiveIntraScaling           = bAdaptiveIntraScalingEnable;
        m_avcPar->StaticFrameIntraCostScalingRatioP    = 240;
        m_avcPar->SubPelMode                           = curbe->m_encCurbe.DW3.SubPelMode;
        m_avcPar->HMECombineOverlap                    = curbe->m_encCurbe.DW36.HMECombineOverlap;
        m_avcPar->SearchX                              = curbe->m_encCurbe.DW5.RefWidth;
        m_avcPar->SearchY                              = curbe->m_encCurbe.DW5.RefHeight;
        m_avcPar->SearchControl                        = curbe->m_encCurbe.DW3.SearchCtrl;
        m_avcPar->EnableAdaptiveTxDecision             = curbe->m_encCurbe.DW34.EnableAdaptiveTxDecision;
        m_avcPar->TxDecisionThr                        = curbe->m_encCurbe.DW60.TxDecisonThreshold;
        m_avcPar->EnablePerMBStaticCheck               = curbe->m_encCurbe.DW34.EnablePerMBStaticCheck;
        m_avcPar->EnableAdaptiveSearchWindowSize       = curbe->m_encCurbe.DW34.EnableAdaptiveSearchWindowSize;
        m_avcPar->EnableIntraCostScalingForStaticFrame = curbe->m_encCurbe.DW4.EnableIntraCostScalingForStaticFrame;
        m_avcPar->BiMixDisable                         = curbe->m_encCurbe.DW0.BiMixDis;
        m_avcPar->SurvivedSkipCost                     = (curbe->m_encCurbe.DW7.NonSkipZMvAdded << 1) + curbe->m_encCurbe.DW7.NonSkipModeAdded;
        m_avcPar->UniMixDisable                        = curbe->m_encCurbe.DW1.UniMixDisable;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        m_avcPar->BMEMethod                         = meMethod;
        m_avcPar->HMEBCombineLen                    = HMEBCombineLen[m_targetUsage];
        m_avcPar->StaticFrameIntraCostScalingRatioB = 200;
        m_avcPar->BSearchX                          = curbe->m_encCurbe.DW5.RefWidth;
        m_avcPar->BSearchY                          = curbe->m_encCurbe.DW5.RefHeight;
        m_avcPar->BSearchControl                    = curbe->m_encCurbe.DW3.SearchCtrl;
        m_avcPar->BSkipType                         = curbe->m_encCurbe.DW3.SkipType;
        m_avcPar->DirectMode                        = curbe->m_encCurbe.DW34.bDirectMode;
        m_avcPar->BiWeight                          = curbe->m_encCurbe.DW1.BiWeight;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG10::PopulatePakParam(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER   secondLevelBatchBuffer)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint8_t          *data = nullptr;
    MOS_LOCK_PARAMS  lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (cmdBuffer != nullptr)
    {
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g10_X::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
    }
    else if (secondLevelBatchBuffer != nullptr)
    {
        data = secondLevelBatchBuffer->pData;
    }
    else
    {
        data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx], &lockFlags);
    }

    CODECHAL_DEBUG_CHK_NULL(data);

    mhw_vdbox_mfx_g10_X::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g10_X::MFX_AVC_IMG_STATE_CMD *)(data);

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->TrellisQuantizationEnable         = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->EnableAdaptiveTrellisQuantization = mfxCmd.DW5.TrellisQuantizationEnabledTqenb;
        m_avcPar->TrellisQuantizationRounding       = mfxCmd.DW5.TrellisQuantizationRoundingTqr;
        m_avcPar->TrellisQuantizationChromaDisable  = mfxCmd.DW5.TrellisQuantizationChromaDisableTqchromadisable;
        m_avcPar->ExtendedRhoDomainEn               = mfxCmd.DW17.ExtendedRhodomainStatisticsEnable;
    }

    if (data && (cmdBuffer == nullptr) && (secondLevelBatchBuffer == nullptr))
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            &BrcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx]);
    }

    return MOS_STATUS_SUCCESS;
}
#endif
