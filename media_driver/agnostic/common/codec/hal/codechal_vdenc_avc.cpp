/*
* Copyright (c) 2011-2019, Intel Corporation
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
//! \file     codechal_vdenc_avc.cpp
//! \brief    This file implements the base C++ class/interface for AVC VDENC encoding
//!           to be used across CODECHAL components.
//!

#include "codechal_vdenc_avc.h"
#include "hal_oca_interface.h"
#include "mhw_cmd_reader.h"

#define CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE 128
#define CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE 52
#define CODECHAL_ENCODE_AVC_SEI_BUFFER_SIZE 10240  // 10K is just estimation
#define CODECHAL_ENCODE_AVC_DEFAULT_TRELLIS_QUANT_ROUNDING 3
#define CODECHAL_ENCODE_AVC_SKIP_BIAS_ADJUSTMENT_QP_THRESHOLD 22
#define CODECHAL_ENCODE_AVC_HME_FIRST_STEP 0
#define CODECHAL_ENCODE_AVC_HME_FOLLOWING_STEP 1
#define CODECHAL_ENCODE_AVC_MV_SHIFT_FACTOR_32x 1
#define CODECHAL_ENCODE_AVC_MV_SHIFT_FACTOR_16x 2
#define CODECHAL_ENCODE_AVC_MV_SHIFT_FACTOR_4x 2
#define CODECHAL_ENCODE_AVC_PREV_MV_READ_POSITION_16x 1
#define CODECHAL_ENCODE_AVC_PREV_MV_READ_POSITION_4x 0

#define CODECHAL_ENCODE_VDENC_IMG_STATE_CMD_SIZE 140
#define CODECHAL_ENCODE_MI_BATCH_BUFFER_END_CMD_SIZE 4

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_ME
{
    CODECHAL_ENCODE_AVC_ME_MV_DATA_SURFACE    = 0,
    CODECHAL_ENCODE_AVC_16xME_MV_DATA_SURFACE = 1,
    CODECHAL_ENCODE_AVC_32xME_MV_DATA_SURFACE = 1,
    CODECHAL_ENCODE_AVC_ME_DISTORTION_SURFACE = 2,
    CODECHAL_ENCODE_AVC_ME_BRC_DISTORTION     = 3,
    CODECHAL_ENCODE_AVC_ME_RESERVED0          = 4,
    CODECHAL_ENCODE_AVC_ME_CURR_FOR_FWD_REF   = 5,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX0       = 6,
    CODECHAL_ENCODE_AVC_ME_RESERVED1          = 7,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX1       = 8,
    CODECHAL_ENCODE_AVC_ME_RESERVED2          = 9,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX2       = 10,
    CODECHAL_ENCODE_AVC_ME_RESERVED3          = 11,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX3       = 12,
    CODECHAL_ENCODE_AVC_ME_RESERVED4          = 13,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX4       = 14,
    CODECHAL_ENCODE_AVC_ME_RESERVED5          = 15,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX5       = 16,
    CODECHAL_ENCODE_AVC_ME_RESERVED6          = 17,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX6       = 18,
    CODECHAL_ENCODE_AVC_ME_RESERVED7          = 19,
    CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX7       = 20,
    CODECHAL_ENCODE_AVC_ME_RESERVED8          = 21,
    CODECHAL_ENCODE_AVC_ME_CURR_FOR_BWD_REF   = 22,
    CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX0       = 23,
    CODECHAL_ENCODE_AVC_ME_RESERVED9          = 24,
    CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX1       = 25,
    CODECHAL_ENCODE_AVC_ME_VDENC_STREAMIN     = 26,
    CODECHAL_ENCODE_AVC_ME_NUM_SURFACES       = 27
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_ME;

// binding table for State Content Detection
typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_SFD_COMMON
{
    CODECHAL_ENCODE_AVC_SFD_VDENC_INPUT_IMAGE_STATE_COMMON  = 0,
    CODECHAL_ENCODE_AVC_SFD_MV_DATA_SURFACE_COMMON          = 1,
    CODECHAL_ENCODE_AVC_SFD_INTER_DISTORTION_SURFACE_COMMON = 2,
    CODECHAL_ENCODE_AVC_SFD_OUTPUT_DATA_SURFACE_COMMON      = 3,
    CODECHAL_ENCODE_AVC_SFD_VDENC_OUTPUT_IMAGE_STATE_COMMON = 4,
    CODECHAL_ENCODE_AVC_SFD_NUM_SURFACES                    = 5
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_SFD_COMMON;

const uint32_t CodechalVdencAvcState::AVC_Mode_Cost[2][12][CODEC_AVC_NUM_QP] =
    {
        //INTRASLICE
        {
            //LUTMODE_INTRA_NONPRED
            {
                14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,  //QP=[0 ~12]
                16,
                18,
                22,
                24,
                13,
                15,
                16,
                18,
                13,
                15,
                15,
                12,
                14,  //QP=[13~25]
                12,
                12,
                10,
                10,
                11,
                10,
                10,
                10,
                9,
                9,
                8,
                8,
                8,  //QP=[26~38]
                8,
                8,
                8,
                8,
                8,
                8,
                8,
                8,
                7,
                7,
                7,
                7,
                7,  //QP=[39~51]
            },
            //LUTMODE_INTRA_16x16, LUTMODE_INTRA
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //QP=[0 ~12]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[13~25]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[26~38]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[39~51]
            },
            //LUTMODE_INTRA_8x8
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //QP=[0 ~12]
                0,
                0,
                0,
                0,
                0,
                0,
                1,
                1,
                1,
                1,
                1,
                1,
                1,  //QP=[13~25]
                1,
                1,
                1,
                1,
                1,
                4,
                4,
                4,
                4,
                6,
                6,
                6,
                6,  //QP=[26~38]
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                7,
                7,
                7,
                7,
                7,  //QP=[39~51]
            },
            //LUTMODE_INTRA_4x4
            {
                56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,  //QP=[0 ~12]
                64,
                72,
                80,
                88,
                48,
                56,
                64,
                72,
                53,
                59,
                64,
                56,
                64,  //QP=[13~25]
                57,
                64,
                58,
                55,
                64,
                64,
                64,
                64,
                59,
                59,
                60,
                57,
                50,  //QP=[26~38]
                46,
                42,
                38,
                34,
                31,
                27,
                23,
                22,
                19,
                18,
                16,
                14,
                13,  //QP=[39~51]
            },

            //LUTMODE_INTER_16x8, LUTMODE_INTER_8x16
            {
                0,
            },
            //LUTMODE_INTER_8x8q
            {
                0,
            },
            //LUTMODE_INTER_8x4q, LUTMODE_INTER_4x8q, LUTMODE_INTER_16x8_FIELD
            {
                0,
            },
            //LUTMODE_INTER_4x4q, LUTMODE_INTER_8x8_FIELD
            {
                0,
            },
            //LUTMODE_INTER_16x16, LUTMODE_INTER
            {
                0,
            },
            //LUTMODE_INTER_BWD
            {
                0,
            },
            //LUTMODE_REF_ID
            {
                0,
            },
            //LUTMODE_INTRA_CHROMA
            {
                0,
            },
        },
        //PREDSLICE
        {
            //LUTMODE_INTRA_NONPRED
            {
                6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,  //QP=[0 ~12]
                7,
                8,
                9,
                10,
                5,
                6,
                7,
                8,
                6,
                7,
                7,
                7,
                7,  //QP=[13~25]
                6,
                7,
                7,
                6,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,  //QP=[26~38]
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,  //QP=[39~51]
            },
            {
                21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,  //QP=[0 ~12]
                24,
                28,
                31,
                35,
                19,
                21,
                24,
                28,
                20,
                24,
                25,
                21,
                24,  //QP=[13~25]
                24,
                24,
                24,
                21,
                24,
                24,
                26,
                24,
                24,
                24,
                24,
                24,
                24,  //QP=[26~38]
                24,
                24,
                24,
                24,
                24,
                24,
                24,
                24,
                24,
                24,
                24,
                24,
                24,  //QP=[39~51]
            },

            //LUTMODE_INTRA_8x8
            {
                26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,  //QP=[0 ~12]
                28,
                32,
                36,
                40,
                22,
                26,
                28,
                32,
                24,
                26,
                30,
                26,
                28,  //QP=[13~25]
                26,
                28,
                26,
                26,
                30,
                28,
                28,
                28,
                26,
                28,
                28,
                26,
                28,  //QP=[26~38]
                28,
                28,
                28,
                28,
                28,
                28,
                28,
                28,
                28,
                28,
                28,
                28,
                28,  //QP=[39~51]
            },
            //LUTMODE_INTRA_4x4
            {
                64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,  //QP=[0 ~12]
                72,
                80,
                88,
                104,
                56,
                64,
                72,
                80,
                58,
                68,
                76,
                64,
                68,  //QP=[13~25]
                64,
                68,
                68,
                64,
                70,
                70,
                70,
                70,
                68,
                68,
                68,
                68,
                68,  //QP=[26~38]
                68,
                68,
                68,
                68,
                68,
                68,
                68,
                68,
                68,
                68,
                68,
                68,
                68,  //QP=[39~51]
            },
            //LUTMODE_INTER_16x8, LUTMODE_INTER_8x16
            {
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  //QP=[0 ~12]
                8,
                9,
                11,
                12,
                6,
                7,
                9,
                10,
                7,
                8,
                9,
                8,
                9,  //QP=[13~25]
                8,
                9,
                8,
                8,
                9,
                9,
                9,
                9,
                8,
                8,
                8,
                8,
                8,  //QP=[26~38]
                8,
                8,
                8,
                9,
                9,
                9,
                9,
                9,
                9,
                9,
                9,
                9,
                9,  //QP=[39~51]
            },
            //LUTMODE_INTER_8x8q
            {
                2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  //QP=[0 ~12]
                2,
                3,
                3,
                3,
                2,
                2,
                2,
                3,
                2,
                2,
                2,
                2,
                3,  //QP=[13~25]
                2,
                2,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,  //QP=[26~38]
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,
                3,  //QP=[39~51]
            },
            //LUTMODE_INTER_8x4q, LUTMODE_INTER_4x8q, LUTMODE_INTER_16x8_FIELD
            {
                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  //QP=[0 ~12]
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,  //QP=[13~25]
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,  //QP=[26~38]
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,
                5,  //QP=[39~51]
            },
            //LUTMODE_INTER_4x4q, LUTMODE_INTER_8x8_FIELD
            {
                7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,  //QP=[0 ~12]
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,  //QP=[13~25]
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,  //QP=[26~38]
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,
                7,  //QP=[39~51]
            },
            //LUTMODE_INTER_16x16, LUTMODE_INTER
            {
                5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,  //QP=[0 ~12]
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,  //QP=[13~25]
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,  //QP=[26~38]
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,
                6,  //QP=[39~51]
            },
            //LUTMODE_INTER_BWD
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //QP=[0 ~12]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[13~25]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[26~38]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[39~51]
            },
            //LUTMODE_REF_ID
            {
                4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0 ~12]
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,  //QP=[13~25]
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,  //QP=[26~38]
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,
                4,  //QP=[39~51]
            },
            //LUTMODE_INTRA_CHROMA
            {
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //QP=[0 ~12]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[13~25]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[26~38]
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,  //QP=[39~51]
            },
        },
};

const int8_t CodechalVdencAvcState::BRC_UPD_GlobalRateQPAdjTabI_U8[64] =
    {
        48,
        40,
        32,
        24,
        16,
        8,
        0,
        -8,
        40,
        32,
        24,
        16,
        8,
        0,
        -8,
        -16,
        32,
        24,
        16,
        8,
        0,
        -8,
        -16,
        -24,
        24,
        16,
        8,
        0,
        -8,
        -16,
        -24,
        -32,
        16,
        8,
        0,
        -8,
        -16,
        -24,
        -32,
        -40,
        8,
        0,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        0,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        -56,
        48,
        40,
        32,
        24,
        16,
        8,
        0,
        -8,
};

const int8_t CodechalVdencAvcState::BRC_UPD_GlobalRateQPAdjTabP_U8[64] =
    {
        48, 40, 32, 24, 16, 8, 0, -8, 40, 32, 24, 16, 8, 0, -8, -16, 16, 8, 8, 4, -8, -16, -16, -24, 8, 0, 0, -8, -16, -16, -16, -24, 8, 0, 0, -24, -32, -32, -32, -48, 0, -16, -16, -24, -32, -48, -56, -64, -8, -16, -32, -32, -48, -48, -56, -64, -16, -32, -48, -48, -48, -56, -64, -80};

// P picture global rate QP Adjustment table for sliding window BRC
const int8_t CodechalVdencAvcState::BRC_UPD_SlWinGlobalRateQPAdjTabP_U8[64] =
    {
        48,
        40,
        32,
        24,
        16,
        8,
        0,
        -8,
        40,
        32,
        24,
        16,
        8,
        0,
        -8,
        -16,
        16,
        8,
        8,
        4,
        -8,
        -16,
        -16,
        -24,
        8,
        0,
        0,
        -8,
        -16,
        -16,
        -16,
        -24,
        8,
        0,
        0,
        -24,
        -32,
        -32,
        -32,
        -48,
        0,
        -16,
        -24,
        -32,
        -40,
        -56,
        -64,
        -72,
        -8,
        -16,
        -32,
        -40,
        -48,
        -56,
        -64,
        -64,
        -16,
        -32,
        -48,
        -48,
        -48,
        -56,
        -64,
        -80,
};

const int8_t CodechalVdencAvcState::BRC_UPD_GlobalRateQPAdjTabB_U8[64] =
    {
        48, 40, 32, 24, 16, 8, 0, -8, 40, 32, 24, 16, 8, 0, -8, -16, 32, 24, 16, 8, 0, -8, -16, -24, 24, 16, 8, 0, -8, -8, -16, -24, 16, 8, 0, 0, -8, -16, -24, -32, 16, 8, 0, 0, -8, -16, -24, -32, 0, -8, -8, -16, -32, -48, -56, -64, 0, -8, -8, -16, -32, -48, -56, -64};

const uint8_t CodechalVdencAvcState::BRC_UPD_DistThreshldI_U8[10] =
    {
        2, 4, 8, 12, 19, 32, 64, 128, 0, 0};

const uint8_t CodechalVdencAvcState::BRC_UPD_DistThreshldP_U8[10] =
    {
        2, 4, 8, 12, 19, 32, 64, 128, 0, 0};

const int8_t CodechalVdencAvcState::CBR_UPD_DistQPAdjTabI_U8[81] =
    {
        0,
        0,
        0,
        0,
        0,
        3,
        4,
        6,
        8,
        0,
        0,
        0,
        0,
        0,
        2,
        3,
        5,
        7,
        -1,
        0,
        0,
        0,
        0,
        2,
        2,
        4,
        5,
        -1,
        -1,
        0,
        0,
        0,
        1,
        2,
        2,
        4,
        -2,
        -2,
        -1,
        0,
        0,
        0,
        1,
        2,
        4,
        -2,
        -2,
        -1,
        0,
        0,
        0,
        1,
        2,
        4,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        5,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        2,
        4,
        7,
        -4,
        -3,
        -2,
        -1,
        0,
        1,
        3,
        5,
        8,
};

const int8_t CodechalVdencAvcState::CBR_UPD_DistQPAdjTabP_U8[81] =
    {
        -1,
        0,
        0,
        0,
        0,
        1,
        1,
        2,
        3,
        -1,
        -1,
        0,
        0,
        0,
        1,
        1,
        2,
        3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        1,
        2,
        3,
        -3,
        -2,
        -2,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
};

const int8_t CodechalVdencAvcState::CBR_UPD_DistQPAdjTabB_U8[81] =
    {
        0,
        0,
        0,
        0,
        0,
        2,
        3,
        3,
        4,
        0,
        0,
        0,
        0,
        0,
        2,
        3,
        3,
        4,
        -1,
        0,
        0,
        0,
        0,
        2,
        2,
        3,
        3,
        -1,
        -1,
        0,
        0,
        0,
        1,
        2,
        2,
        2,
        -1,
        -1,
        -1,
        0,
        0,
        0,
        1,
        2,
        2,
        -2,
        -1,
        -1,
        0,
        0,
        0,
        0,
        1,
        2,
        -2,
        -1,
        -1,
        -1,
        0,
        0,
        0,
        1,
        3,
        -2,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        1,
        3,
        -2,
        -2,
        -1,
        -1,
        0,
        1,
        1,
        2,
        4,
};

const int8_t CodechalVdencAvcState::VBR_UPD_DistQPAdjTabI_U8[81] =
    {
        0,
        0,
        0,
        0,
        0,
        3,
        4,
        6,
        8,
        0,
        0,
        0,
        0,
        0,
        2,
        3,
        5,
        7,
        -1,
        0,
        0,
        0,
        0,
        2,
        2,
        4,
        5,
        -1,
        -1,
        0,
        0,
        0,
        1,
        2,
        2,
        4,
        -2,
        -2,
        -1,
        0,
        0,
        0,
        1,
        2,
        4,
        -2,
        -2,
        -1,
        0,
        0,
        0,
        1,
        2,
        4,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        5,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        2,
        4,
        7,
        -4,
        -3,
        -2,
        -1,
        0,
        1,
        3,
        5,
        8,
};

const int8_t CodechalVdencAvcState::VBR_UPD_DistQPAdjTabP_U8[81] =
    {
        -1,
        0,
        0,
        0,
        0,
        1,
        1,
        2,
        3,
        -1,
        -1,
        0,
        0,
        0,
        1,
        1,
        2,
        3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        1,
        2,
        3,
        -3,
        -2,
        -2,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
        -3,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        2,
        3,
};

const int8_t CodechalVdencAvcState::VBR_UPD_DistQPAdjTabB_U8[81] =
    {
        0,
        0,
        0,
        0,
        0,
        2,
        3,
        3,
        4,
        0,
        0,
        0,
        0,
        0,
        2,
        3,
        3,
        4,
        -1,
        0,
        0,
        0,
        0,
        2,
        2,
        3,
        3,
        -1,
        -1,
        0,
        0,
        0,
        1,
        2,
        2,
        2,
        -1,
        -1,
        -1,
        0,
        0,
        0,
        1,
        2,
        2,
        -2,
        -1,
        -1,
        0,
        0,
        0,
        0,
        1,
        2,
        -2,
        -1,
        -1,
        -1,
        0,
        0,
        0,
        1,
        3,
        -2,
        -2,
        -1,
        -1,
        0,
        0,
        1,
        1,
        3,
        -2,
        -2,
        -1,
        -1,
        0,
        1,
        1,
        2,
        4,
};

const int8_t CodechalVdencAvcState::CBR_UPD_FrmSzAdjTabI_S8[72] =
    {
        -4,
        -20,
        -28,
        -36,
        -40,
        -44,
        -48,
        -80,
        0,
        -8,
        -12,
        -20,
        -24,
        -28,
        -32,
        -36,
        0,
        0,
        -8,
        -16,
        -20,
        -24,
        -28,
        -32,
        8,
        4,
        0,
        0,
        -8,
        -16,
        -24,
        -28,
        32,
        24,
        16,
        2,
        -4,
        -8,
        -16,
        -20,
        36,
        32,
        28,
        16,
        8,
        0,
        -4,
        -8,
        40,
        36,
        24,
        20,
        16,
        8,
        0,
        -8,
        48,
        40,
        28,
        24,
        20,
        12,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::CBR_UPD_FrmSzAdjTabP_S8[72] =
    {
        -8,
        -24,
        -32,
        -44,
        -48,
        -56,
        -64,
        -80,
        -8,
        -16,
        -32,
        -40,
        -44,
        -52,
        -56,
        -64,
        0,
        0,
        -16,
        -28,
        -36,
        -40,
        -44,
        -48,
        8,
        4,
        0,
        0,
        -8,
        -16,
        -24,
        -36,
        20,
        12,
        4,
        0,
        -8,
        -8,
        -8,
        -16,
        24,
        16,
        8,
        8,
        8,
        0,
        -4,
        -8,
        40,
        36,
        24,
        20,
        16,
        8,
        0,
        -8,
        48,
        40,
        28,
        24,
        20,
        12,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::CBR_UPD_FrmSzAdjTabB_S8[72] =
    {
        0,
        -4,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        1,
        0,
        -4,
        -8,
        -16,
        -24,
        -32,
        -40,
        4,
        2,
        0,
        -1,
        -3,
        -8,
        -16,
        -24,
        8,
        4,
        2,
        0,
        -1,
        -4,
        -8,
        -16,
        20,
        16,
        4,
        0,
        -1,
        -4,
        -8,
        -16,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        -8,
        28,
        24,
        20,
        16,
        8,
        4,
        0,
        -8,
        32,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::VBR_UPD_FrmSzAdjTabI_S8[72] =
    {
        -4,
        -20,
        -28,
        -36,
        -40,
        -44,
        -48,
        -80,
        0,
        -8,
        -12,
        -20,
        -24,
        -28,
        -32,
        -36,
        0,
        0,
        -8,
        -16,
        -20,
        -24,
        -28,
        -32,
        8,
        4,
        0,
        0,
        -8,
        -16,
        -24,
        -28,
        32,
        24,
        16,
        2,
        -4,
        -8,
        -16,
        -20,
        36,
        32,
        28,
        16,
        8,
        0,
        -4,
        -8,
        40,
        36,
        24,
        20,
        16,
        8,
        0,
        -8,
        48,
        40,
        28,
        24,
        20,
        12,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::VBR_UPD_FrmSzAdjTabP_S8[72] =
    {
        -8,
        -24,
        -32,
        -44,
        -48,
        -56,
        -64,
        -80,
        -8,
        -16,
        -32,
        -40,
        -44,
        -52,
        -56,
        -64,
        0,
        0,
        -16,
        -28,
        -36,
        -40,
        -44,
        -48,
        8,
        4,
        0,
        0,
        -8,
        -16,
        -24,
        -36,
        20,
        12,
        4,
        0,
        -8,
        -8,
        -8,
        -16,
        24,
        16,
        8,
        8,
        8,
        0,
        -4,
        -8,
        40,
        36,
        24,
        20,
        16,
        8,
        0,
        -8,
        48,
        40,
        28,
        24,
        20,
        12,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::VBR_UPD_FrmSzAdjTabB_S8[72] =
    {
        0,
        -4,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        1,
        0,
        -4,
        -8,
        -16,
        -24,
        -32,
        -40,
        4,
        2,
        0,
        -1,
        -3,
        -8,
        -16,
        -24,
        8,
        4,
        2,
        0,
        -1,
        -4,
        -8,
        -16,
        20,
        16,
        4,
        0,
        -1,
        -4,
        -8,
        -16,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        -8,
        28,
        24,
        20,
        16,
        8,
        4,
        0,
        -8,
        32,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::QVBR_UPD_FrmSzAdjTabP_S8[72] =
    {
        -8,
        -24,
        -32,
        -44,
        -48,
        -56,
        -64,
        -80,
        -8,
        -16,
        -32,
        -40,
        -44,
        -52,
        -56,
        -64,
        0,
        0,
        -16,
        -28,
        -36,
        -40,
        -44,
        -48,
        16,
        16,
        8,
        0,
        -8,
        -16,
        -24,
        -36,
        20,
        16,
        8,
        0,
        -8,
        -8,
        -8,
        -16,
        24,
        16,
        8,
        8,
        8,
        0,
        -4,
        -8,
        40,
        36,
        24,
        20,
        16,
        8,
        0,
        -8,
        48,
        40,
        28,
        24,
        20,
        12,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::LOW_DELAY_UPD_FrmSzAdjTabI_S8[72] =
    {
        0,
        0,
        -8,
        -12,
        -16,
        -20,
        -28,
        -36,
        0,
        0,
        -4,
        -8,
        -12,
        -16,
        -24,
        -32,
        4,
        2,
        0,
        -1,
        -3,
        -8,
        -16,
        -24,
        8,
        4,
        2,
        0,
        -1,
        -4,
        -8,
        -16,
        20,
        16,
        4,
        0,
        -1,
        -4,
        -8,
        -16,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        -8,
        28,
        24,
        20,
        16,
        8,
        4,
        0,
        -8,
        32,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::LOW_DELAY_UPD_FrmSzAdjTabP_S8[72] =
    {
        -8,
        -24,
        -32,
        -40,
        -44,
        -48,
        -52,
        -80,
        -8,
        -16,
        -32,
        -40,
        -40,
        -44,
        -44,
        -56,
        0,
        0,
        -12,
        -20,
        -24,
        -28,
        -32,
        -36,
        8,
        4,
        0,
        0,
        -8,
        -16,
        -24,
        -32,
        32,
        16,
        8,
        4,
        -4,
        -8,
        -16,
        -20,
        36,
        24,
        16,
        8,
        4,
        -2,
        -4,
        -8,
        40,
        36,
        24,
        20,
        16,
        8,
        0,
        -8,
        48,
        40,
        28,
        24,
        20,
        12,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const int8_t CodechalVdencAvcState::LOW_DELAY_UPD_FrmSzAdjTabB_S8[72] =
    {
        0,
        -4,
        -8,
        -16,
        -24,
        -32,
        -40,
        -48,
        1,
        0,
        -4,
        -8,
        -16,
        -24,
        -32,
        -40,
        4,
        2,
        0,
        -1,
        -3,
        -8,
        -16,
        -24,
        8,
        4,
        2,
        0,
        -1,
        -4,
        -8,
        -16,
        20,
        16,
        4,
        0,
        -1,
        -4,
        -8,
        -16,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        -8,
        28,
        24,
        20,
        16,
        8,
        4,
        0,
        -8,
        32,
        24,
        20,
        16,
        8,
        4,
        0,
        -4,
        64,
        48,
        28,
        20,
        16,
        12,
        8,
        4,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_FrmSzMinTabP_U8[9] =
    {
        1, 2, 4, 6, 8, 10, 16, 16, 16};

const uint8_t CodechalVdencAvcState::BRC_UPD_FrmSzMinTabI_U8[9] =
    {
        1, 2, 4, 8, 16, 20, 24, 32, 36};

const uint8_t CodechalVdencAvcState::BRC_UPD_FrmSzMaxTabP_U8[9] =
    {
        48, 64, 80, 96, 112, 128, 144, 160, 160};

const uint8_t CodechalVdencAvcState::BRC_UPD_FrmSzMaxTabI_U8[9] =
    {
        48, 64, 80, 96, 112, 128, 144, 160, 160};

const uint8_t CodechalVdencAvcState::BRC_UPD_FrmSzSCGTabP_U8[9] =
    {
        4, 8, 12, 16, 20, 24, 24, 0, 0};

const uint8_t CodechalVdencAvcState::BRC_UPD_FrmSzSCGTabI_U8[9] =
    {
        4, 8, 12, 16, 20, 24, 24, 0, 0};

// Cost Table 14*42 = 588 bytes
const uint8_t CodechalVdencAvcState::BRC_UPD_I_IntraNonPred[42] =
    {
        0x0e, 0x0e, 0x0e, 0x18, 0x19, 0x1b, 0x1c, 0x0d, 0x0f, 0x18, 0x19, 0x0d, 0x0f, 0x0f, 0x0c, 0x0e, 0x0c, 0x0c, 0x0a, 0x0a, 0x0b, 0x0a, 0x0a, 0x0a, 0x09, 0x09, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x07, 0x07, 0x07, 0x07, 0x07};

const uint8_t CodechalVdencAvcState::BRC_UPD_I_Intra8x8[42] =
    {
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x00,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x01,
        0x04,
        0x04,
        0x04,
        0x04,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x07,
        0x07,
        0x07,
        0x07,
        0x07,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_I_Intra4x4[42] =
    {
        0x2e, 0x2e, 0x2e, 0x38, 0x39, 0x3a, 0x3b, 0x2c, 0x2e, 0x38, 0x39, 0x2d, 0x2f, 0x38, 0x2e, 0x38, 0x2e, 0x38, 0x2f, 0x2e, 0x38, 0x38, 0x38, 0x38, 0x2f, 0x2f, 0x2f, 0x2e, 0x2d, 0x2c, 0x2b, 0x2a, 0x29, 0x28, 0x1e, 0x1c, 0x1b, 0x1a, 0x19, 0x18, 0x0e, 0x0d};

const uint8_t CodechalVdencAvcState::BRC_UPD_I_IntraChroma[42] =
    {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_IntraNonPred[42] =
    {
        0x06, 0x06, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x05, 0x06, 0x07, 0x08, 0x06, 0x07, 0x07, 0x07, 0x07, 0x06, 0x07, 0x07, 0x06, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_Intra16x16[42] =
    {
        0x1b, 0x1b, 0x1b, 0x1c, 0x1e, 0x28, 0x29, 0x1a, 0x1b, 0x1c, 0x1e, 0x1a, 0x1c, 0x1d, 0x1b, 0x1c, 0x1c, 0x1c, 0x1c, 0x1b, 0x1c, 0x1c, 0x1d, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_Intra8x8[42] =
    {
        0x1d,
        0x1d,
        0x1d,
        0x1e,
        0x28,
        0x29,
        0x2a,
        0x1b,
        0x1d,
        0x1e,
        0x28,
        0x1c,
        0x1d,
        0x1f,
        0x1d,
        0x1e,
        0x1d,
        0x1e,
        0x1d,
        0x1d,
        0x1f,
        0x1e,
        0x1e,
        0x1e,
        0x1d,
        0x1e,
        0x1e,
        0x1d,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
        0x1e,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_Intra4x4[42] =
    {
        0x38,
        0x38,
        0x38,
        0x39,
        0x3a,
        0x3b,
        0x3d,
        0x2e,
        0x38,
        0x39,
        0x3a,
        0x2f,
        0x39,
        0x3a,
        0x38,
        0x39,
        0x38,
        0x39,
        0x39,
        0x38,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
        0x39,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_Inter16x8[42] =
    {
        0x07,
        0x07,
        0x07,
        0x08,
        0x09,
        0x0b,
        0x0c,
        0x06,
        0x07,
        0x09,
        0x0a,
        0x07,
        0x08,
        0x09,
        0x08,
        0x09,
        0x08,
        0x09,
        0x08,
        0x08,
        0x09,
        0x09,
        0x09,
        0x09,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x08,
        0x09,
        0x09,
        0x09,
        0x09,
        0x09,
        0x09,
        0x09,
        0x09,
        0x09,
        0x09,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_Inter8x8[42] =
    {
        0x02,
        0x02,
        0x02,
        0x02,
        0x03,
        0x03,
        0x03,
        0x02,
        0x02,
        0x02,
        0x03,
        0x02,
        0x02,
        0x02,
        0x02,
        0x03,
        0x02,
        0x02,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
        0x03,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_Inter16x16[42] =
    {
        0x05,
        0x05,
        0x05,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
        0x06,
};

const uint8_t CodechalVdencAvcState::BRC_UPD_P_RefId[42] =
    {
        0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04};

const bool CodechalVdencAvcState::SHMEEnabled[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 1, 1, 0, 0, 0, 0, 0};

const bool CodechalVdencAvcState::UHMEEnabled[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 1, 1, 0, 0, 0, 0, 0};

const uint8_t CodechalVdencAvcState::AdaptiveInterRoundingPWithoutB[CODEC_AVC_NUM_QP] =
    {
        //QP =  0   1   2   3   4   5   6   7   8   9   10  11  12
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,  //QP=[0~12]
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        1,
        0,
        0,
        0,
        0,  //QP=[13~25]
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,  //QP=[26~38]
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0  //QP=[39~51]
};

const uint8_t CodechalVdencAvcState::AdaptiveInterRoundingP[CODEC_AVC_NUM_QP] =
    {
        //QP =  0   1   2   3   4   5   6   7   8   9   10  11  12
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,
        4,  //QP=[0~12]
        4,
        4,
        4,
        4,
        4,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,  //QP=[13~25]
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,  //QP=[26~38]
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3,
        3  //QP=[39~51]
};

const uint32_t CodechalVdencAvcState::InterRoundingP[NUM_TARGET_USAGE_MODES] =
    {
        0, 3, 3, 3, 3, 3, 3, 3};

const uint32_t CodechalVdencAvcState::InterRoundingB[NUM_TARGET_USAGE_MODES] =
    {
        0, 0, 0, 0, 0, 0, 0, 0};

const uint32_t CodechalVdencAvcState::InterRoundingBRef[NUM_TARGET_USAGE_MODES] =
    {
        0, 2, 2, 2, 2, 2, 2, 2};

const uint8_t CodechalVdencAvcState::AdaptiveInterRoundingB[CODEC_AVC_NUM_QP] =
    {
        4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,  //QP=[0~12]
        4,
        3,
        3,
        3,
        3,
        3,
        3,
        0,
        0,
        0,
        0,
        0,
        0,  //QP=[13~25]
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,  //QP=[26~38]
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0  //QP=[39~51]
};

/* real thresholds are computed as multiplication on -50 and casting to int */
const double CodechalVdencAvcState::BRC_DevThreshI0_FP_NEG[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
    {
        0.80, 0.60, 0.34, 0.2};

/* real thresholds are computed as multiplication on 50 and casting to int */
const double CodechalVdencAvcState::BRC_DevThreshI0_FP_POS[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
    {
        0.2, 0.4, 0.66, 0.9};

/* real thresholds are computed as multiplication on 50 and casting to int */
const double CodechalVdencAvcState::BRC_DevThreshPB0_FP_NEG[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
    {
        0.90, 0.66, 0.46, 0.3};

/* real thresholds are computed as multiplication on 50 and casting to int */
const double CodechalVdencAvcState::BRC_DevThreshPB0_FP_POS[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
    {
        0.3, 0.46, 0.70, 0.90};

/* real negative thresholds are computed as multiplication on -50 and casting to int */
const double CodechalVdencAvcState::BRC_DevThreshVBR0_NEG[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
    {
        0.90, 0.70, 0.50, 0.3};

/* positive thresholds are computed as multiplication on 100 and casting to int */
const double CodechalVdencAvcState::BRC_DevThreshVBR0_POS[CODECHAL_VDENC_AVC_N_DEV_THRESHLDS / 2] =
    {
        0.4, 0.5, 0.75, 0.90};

const int8_t CodechalVdencAvcState::BRC_LowDelay_DevThreshPB0_S8[8] =
    {
        -45, -33, -23, -15, -8, 0, 15, 25};

const int8_t CodechalVdencAvcState::BRC_LowDelay_DevThreshI0_S8[8] =
    {
        -40, -30, -17, -10, -5, 0, 10, 20};

const int8_t CodechalVdencAvcState::BRC_LowDelay_DevThreshVBR0_S8[8] =
    {
        -45, -35, -25, -15, -8, 0, 20, 40};

const int8_t CodechalVdencAvcState::BRC_INIT_DistQPDelta_I8[4] =
    {
        -5, -2, 2, 5};

const uint8_t CodechalVdencAvcState::BRC_EstRateThreshP0_U8[7] =
    {
        4, 8, 12, 16, 20, 24, 28};

const uint8_t CodechalVdencAvcState::BRC_EstRateThreshI0_U8[7] =
    {
        4, 8, 12, 16, 20, 24, 28};

const uint16_t CodechalVdencAvcState::BRC_UPD_start_global_adjust_frame[4] =
    {
        10, 50, 100, 150};

const uint8_t CodechalVdencAvcState::BRC_UPD_global_rate_ratio_threshold[7] =
    {
        80, 90, 95, 101, 105, 115, 130};

// global rate ratio threshold table for sliding window BRC
const uint8_t CodechalVdencAvcState::BRC_UPD_slwin_global_rate_ratio_threshold[7] =
    {
        80, 90, 95, 101, 105, 110, 120};

const uint8_t CodechalVdencAvcState::BRC_UPD_start_global_adjust_mult[5] =
    {
        1, 1, 3, 2, 1};

const uint8_t CodechalVdencAvcState::BRC_UPD_start_global_adjust_div[5] =
    {
        40, 5, 5, 3, 1};

const uint16_t CodechalVdencAvcState::BRC_UPD_SLCSZ_UPD_THRDELTAP_100Percent_U16[42] =  // slice size threshold delta for P frame targeted for 99% compliance
    {
        1400, 1400, 1400, 1400, 1400, 1400, 1400, 1250, 1100, 950,  //[10-19]
        850,
        750,
        650,
        600,
        550,
        525,
        500,
        450,
        400,
        390,  //[20-29]
        380,
        300,
        300,
        300,
        250,
        200,
        175,
        150,
        150,
        150,  //[30-39]
        150,
        100,
        100,
        100,
        100,
        100,
        100,
        100,
        100,
        100,  //[40-49]
        100,
        100  //[50-51]
};

const uint16_t CodechalVdencAvcState::BRC_UPD_SLCSZ_UPD_THRDELTAI_100Percent_U16[42] =  // slice size threshold delta for I frame targeted for 99% compliance
    {
        1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1350,  //[10-19]
        1300,
        1300,
        1200,
        1155,
        1120,
        1075,
        1000,
        975,
        950,
        800,  //[20-29]
        750,
        650,
        600,
        550,
        500,
        450,
        400,
        350,
        300,
        300,  //[30-39]
        300,
        250,
        200,
        200,
        200,
        200,
        200,
        200,
        200,
        200,  //[40-49]
        200,
        200  //[50-51]
};

const int8_t CodechalVdencAvcState::BRC_UPD_global_rate_ratio_threshold_qp[8] =
    {
        -3, -2, -1, 0, 1, 1, 2, 3};

const uint8_t CodechalVdencAvcState::MaxRefIdx0[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 2, 2, 1, 1, 1, 0, 0};

const uint32_t CodechalVdencAvcState::TrellisQuantizationRounding[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 7, 7, 7, 7, 7, 7, 0};

const bool CodechalVdencAvcState::TrellisQuantizationEnable[NUM_VDENC_TARGET_USAGE_MODES] =
    {
        0, 0, 0, 0, 0, 0, 0, 0};

MOS_STATUS CodechalVdencAvcState::ComputeBRCInitQP(
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS seqParams,
    int32_t *                         initQP)
{
    const float x0 = 0, y0 = 1.19f, x1 = 1.75f, y1 = 1.75f;
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;
    uint32_t    frameSize;
    int32_t     QP, deltaQ;

    CODECHAL_ENCODE_CHK_NULL_RETURN(seqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(initQP);

    // InitQPIP calculation
    frameSize = ((m_frameWidth * m_frameHeight * 3) >> 1);
    QP        = (int32_t)(1. / 1.2 * pow(10.0, (log10(frameSize * 2. / 3. * ((float)seqParams->FramesPer100Sec) / ((float)(seqParams->TargetBitRate) * 100)) - x0) * (y1 - y0) / (x1 - x0) + y0) + 0.5);
    QP += 2;
    //add additional change based on buffer size. It is especially useful for low delay
    deltaQ = (int32_t)(9 - (seqParams->VBVBufferSizeInBit * ((float)seqParams->FramesPer100Sec) / ((float)(seqParams->TargetBitRate) * 100)));
    QP += deltaQ < 0 ? 0 : deltaQ;
    CodecHal_Clip3(CODECHAL_ENCODE_AVC_BRC_MIN_QP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP, QP);
    QP--;
    if (QP < 0)
        QP = 1;

    *initQP = QP;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::AvcVdencStoreHuCStatus2Register(
    CodechalHwInterface *hwInterface,
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MhwMiInterface *                 miInterface;
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MHW_MI_STORE_DATA_PARAMS         storeDataParams;
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hwInterface->GetMiInterface());
    miInterface = hwInterface->GetMiInterface();

    // Write HUC_STATUS2 mask - bit 6 - valid IMEM loaded
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource      = &m_resHucStatus2Buffer;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue          = hwInterface->GetHucInterface()->GetHucStatus2ImemLoadedMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    // Store HUC_STATUS2 register
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resHucStatus2Buffer;
    storeRegParams.dwOffset        = sizeof(uint32_t);

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    storeRegParams.dwRegister = hwInterface->GetHucInterface()->GetMmioRegisters(m_vdboxIndex)->hucStatus2RegOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegParams));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetTLBAllocation(
    PMOS_COMMAND_BUFFER  cmdBuffer,
    PTLBAllocationParams params)
{
    MhwMiInterface *                 miInterface;
    MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS  miLoadRegImmParams;
    MmioRegistersMfx *               mmioRegisters;
    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presTlbMmioBuffer);

    miInterface = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

    // Save MFX_LRA_0/1/2 registers to a temp buffer so we can restore registers after frame encoding
    MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));

    miStoreRegMemParams.presStoreBuffer = params->presTlbMmioBuffer;
    miStoreRegMemParams.dwOffset        = 0;
    miStoreRegMemParams.dwRegister      = mmioRegisters->mfxLra0RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.dwOffset   = sizeof(uint32_t);
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxLra1RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    miStoreRegMemParams.dwOffset   = 2 * sizeof(uint32_t);
    miStoreRegMemParams.dwRegister = mmioRegisters->mfxLra2RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &miStoreRegMemParams));

    // Update MFX_LRA_0/1/2 registers to set TBL for VMC=240
    // =======================================================
    // Clients      LRA     LRA range Min   LRA range Max
    // =======================================================
    // VMXRA + VMC  LRA0    0               239
    // VMX          LRA1    240             245
    // BSP          LRA2    246             250
    // VCR + VCS    LRA3    251             255
    // =======================================================
    MOS_ZeroMemory(&miLoadRegImmParams, sizeof(miLoadRegImmParams));
    miLoadRegImmParams.dwRegister = mmioRegisters->mfxLra0RegOffset;
    miLoadRegImmParams.dwData     = (params->dwMmioMfxLra0Override > 0) ? params->dwMmioMfxLra0Override : CODECHAL_VDENC_AVC_MMIO_MFX_LRA_0_VMC240;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    miLoadRegImmParams.dwRegister = mmioRegisters->mfxLra1RegOffset;
    miLoadRegImmParams.dwData     = (params->dwMmioMfxLra1Override > 0) ? params->dwMmioMfxLra1Override : CODECHAL_VDENC_AVC_MMIO_MFX_LRA_1_VMC240;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    miLoadRegImmParams.dwRegister = mmioRegisters->mfxLra2RegOffset;
    miLoadRegImmParams.dwData     = (params->dwMmioMfxLra2Override > 0) ? params->dwMmioMfxLra2Override : CODECHAL_VDENC_AVC_MMIO_MFX_LRA_2_VMC240;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &miLoadRegImmParams));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::RestoreTLBAllocation(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMOS_RESOURCE       tlbMmioBuffer)
{
    MhwMiInterface *                miInterface;
    MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
    MmioRegistersMfx *              mmioRegisters;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(tlbMmioBuffer);

    miInterface = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    mmioRegisters = m_hwInterface->SelectVdboxAndGetMmioRegister(m_vdboxIndex, cmdBuffer);

    // Restore MFX_LRA_0/1/2 registers
    miLoadRegMemParams.presStoreBuffer = tlbMmioBuffer;
    miLoadRegMemParams.dwOffset        = 0;
    miLoadRegMemParams.dwRegister      = mmioRegisters->mfxLra0RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

    miLoadRegMemParams.dwOffset   = sizeof(uint32_t);
    miLoadRegMemParams.dwRegister = mmioRegisters->mfxLra1RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

    miLoadRegMemParams.dwOffset   = 2 * sizeof(uint32_t);
    miLoadRegMemParams.dwRegister = mmioRegisters->mfxLra2RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiLoadRegisterMemCmd(cmdBuffer, &miLoadRegMemParams));

    return eStatus;
}

CodechalVdencAvcState::CodechalVdencAvcState(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcBase(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    InitializeDataMember();

    // Setup initial data
    m_brcInit = true;
    // enable codec specific user feature key reporting for AVC
    m_userFeatureKeyReport = true;

    m_swBrcMode = nullptr;

    m_cmKernelEnable  = true;
    m_brcRoiSupported = true;

    if (m_cmKernelEnable)
    {
        m_useCmScalingKernel = 1;
    }

    MOS_ZeroMemory(&m_vdencIntraRowStoreScratchBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_vdencColocatedMVBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_pakStatsBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_vdencStatsBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_vdencTlbMmioBuffer, sizeof(MOS_RESOURCE));
}

CodechalVdencAvcState::~CodechalVdencAvcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencIntraRowStoreScratchBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencColocatedMVBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencStatsBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_pakStatsBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencTlbMmioBuffer);
    if (m_vdencBrcImgStatAllocated)
    {
        for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            Mhw_FreeBb(m_osInterface, &m_batchBufferForVdencImgStat[i], nullptr);
        }
    }
    else
    {
        Mhw_FreeBb(m_osInterface, &m_batchBufferForVdencImgStat[0], nullptr);
    }

    if (m_seiData.pSEIBuffer)
    {
        MOS_FreeMemory(m_seiData.pSEIBuffer);
        m_seiData.pSEIBuffer = nullptr;
    }

    MOS_Delete(m_sfdKernelState);
    m_sfdKernelState = nullptr;

    if (m_pakEnabled)
    {
        // release skip frame copy buffer
        m_osInterface->pfnFreeResource(m_osInterface, &m_resSkipFrameBuffer);
    }

    // SFD surfaces
    {
        // SFD output buffer
        for (uint32_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_resSfdOutputBuffer[i]);
        }

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resSfdCostTablePFrameBuffer);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_resSfdCostTableBFrameBuffer);
    }

    if (m_swBrcMode != nullptr)
    {
        m_osInterface->pfnFreeLibrary(m_swBrcMode);
        m_swBrcMode = nullptr;
    }

    for (uint32_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        for (uint32_t j = 0; j < CODECHAL_VDENC_BRC_NUM_OF_PASSES; j++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencBrcUpdateDmemBuffer[i][j]);
        }
        m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencBrcInitDmemBuffer[i]);
        m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencBrcImageStatesReadBuffer[i]);
    }

    m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencBrcConstDataBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencBrcHistoryBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencSfdImageStateReadBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resVdencBrcDbgBuffer);
}

MOS_STATUS CodechalVdencAvcState::Initialize(CodechalSetting *settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(settings);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcBase::Initialize(settings));

    if (m_cscDsState)
    {
        // for AVC: the Ds+Copy kernel is by default used to do CSC and copy
        // non-aligned surface
        m_cscDsState->EnableCopy();
        m_cscDsState->EnableColor();
        m_cscDsState->EnableSfc();
    }

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
#if (_DEBUG || _RELEASE_INTERNAL)
    /*MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AVC_BRC_SOFTWARE_ID,
        &userFeatureData);

    if (userFeatureData.i32Data)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnLoadLibrary(m_osInterface, CODECHAL_DBG_STRING_SWAVCBRCLIBRARY, &m_swBrcMode));
    }*/

    // SW BRC DLL Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_AVC_BRC_SOFTWARE_IN_USE_ID, (m_swBrcMode == nullptr) ? false : true);
#endif  // (_DEBUG || _RELEASE_INTERNAL)

    if (m_codecFunction != CODECHAL_FUNCTION_PAK)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_ME_ENABLE_ID,
            &userFeatureData);
        m_hmeSupported = (userFeatureData.u32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_16xME_ENABLE_ID,
            &userFeatureData);

        if (userFeatureData.i32Data == 0 || userFeatureData.i32Data == 1)
        {
            m_16xMeUserfeatureControl = true;
            m_16xMeSupported          = (userFeatureData.i32Data) ? true : false;
        }
        else
        {
            m_16xMeUserfeatureControl = false;
            m_16xMeSupported          = true;
        }

#ifndef _FULL_OPEN_SOURCE
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_STATIC_FRAME_DETECTION_ENABLE_ID,
            &userFeatureData);
        m_staticFrameDetectionEnable = (userFeatureData.i32Data) ? true : false;
#endif

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AVC_FORCE_TO_SKIP_ENABLE_ID,
            &userFeatureData);
        m_forceToSkipEnable = (userFeatureData.u32Data) ? true : false;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AVC_SLIDING_WINDOW_SIZE_ID,
            &userFeatureData);
        m_slidingWindowSize = userFeatureData.u32Data;

        m_groupIdSelectSupported = 0;  // Disabled by default for AVC for now
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_GROUP_ID_SELECT_ENABLE_ID,
            &userFeatureData);
        m_groupIdSelectSupported = (userFeatureData.i32Data) ? true : false;
#endif

        m_groupId = 0;  // default value for group ID  = 0
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_GROUP_ID_ID,
            &userFeatureData);
        m_groupId = (uint8_t)userFeatureData.i32Data;
#endif

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VDENC_CRE_PREFETCH_ENABLE_ID,
            &userFeatureData);
        m_crePrefetchEnable = userFeatureData.bData == 1;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VDENC_SINGLE_PASS_ENABLE_ID,
            &userFeatureData);
        m_vdencSinglePassEnable = userFeatureData.bData == 1;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VDENC_TLB_PREFETCH_ENABLE_ID,
            &userFeatureData);
        m_tlbPrefetchEnable = userFeatureData.bData == 1;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VDENC_TLB_ALLOCATION_WA_ENABLE_ID,
            &userFeatureData);
        if (userFeatureData.u32Data == 0)  // MFX_LRA_0/1/2 offsets might not be available
        {
            MEDIA_WR_WA(m_waTable, WaTlbAllocationForAvcVdenc, false);
        }

        if (MEDIA_IS_WA(m_waTable, WaTlbAllocationForAvcVdenc))
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_0_OVERRIDE_ID,
                &userFeatureData);
            m_mmioMfxLra0Override = userFeatureData.u32Data;

            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_1_OVERRIDE_ID,
                &userFeatureData);
            m_mmioMfxLra1Override = userFeatureData.u32Data;

            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_MMIO_MFX_LRA_2_OVERRIDE_ID,
                &userFeatureData);
            m_mmioMfxLra2Override = userFeatureData.u32Data;
        }
    }

    // Initialize hardware resources for the current Os/Platform
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitializeState());

    MotionEstimationDisableCheck();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Initialize());

    // common function for all codecs needed
    if (m_cscDsState && CodecHalUsesRenderEngine(m_codecFunction, m_standard))
    {
        if (m_hmeSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
        }

        if (m_staticFrameDetectionEnable)
        {
            // init Static frame detection kernel
            CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateSFD());
        }

        if (m_singleTaskPhaseSupported)
        {
            uint32_t i;
            uint32_t scalingBtCount, meBtCount, mbEncBtCount, brcBtCount, encOneBtCount, encTwoBtCount;

            scalingBtCount = MOS_ALIGN_CEIL(
                m_scaling4xKernelStates[0].KernelParams.iBTCount,
                m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
            meBtCount = MOS_ALIGN_CEIL(
                m_hmeKernel ? m_hmeKernel->GetBTCount() : m_meKernelStates[0].KernelParams.iBTCount,
                m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
            mbEncBtCount = 0;
            brcBtCount   = 0;

            encOneBtCount = scalingBtCount + meBtCount;
            encOneBtCount += (m_16xMeSupported) ? encOneBtCount : 0;
            encOneBtCount += (m_32xMeSupported) ? encOneBtCount : 0;
            encTwoBtCount = mbEncBtCount + brcBtCount;
            m_maxBtCount  = MOS_MAX(encOneBtCount, encTwoBtCount);
        }
    }

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_AVC,
        &m_pictureStatesSize,
        &m_picturePatchListSize,
        0);

    // Slice Level Commands
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        CODECHAL_ENCODE_MODE_AVC,
        &m_sliceStatesSize,
        &m_slicePatchListSize,
        m_singleTaskPhaseSupported);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CalculateVdencPictureStateCommandSize());

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::CalculateVdencPictureStateCommandSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    PMHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams = CreateMhwVdboxStateCmdsizeParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateCmdSizeParams);
    uint32_t vdencPictureStatesSize, vdencPicturePatchListSize;

    m_hwInterface->GetHxxStateCommandSize(
        CODECHAL_ENCODE_MODE_AVC,
        &vdencPictureStatesSize,
        &vdencPicturePatchListSize,
        stateCmdSizeParams);
    MOS_Delete(stateCmdSizeParams);

    m_pictureStatesSize += vdencPictureStatesSize;
    m_picturePatchListSize += vdencPicturePatchListSize;

    m_hwInterface->GetVdencStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_AVC,
        &vdencPictureStatesSize,
        &vdencPicturePatchListSize);

    m_pictureStatesSize += vdencPictureStatesSize;
    m_picturePatchListSize += vdencPicturePatchListSize;

    return eStatus;
}

void CodechalVdencAvcState::InitializeDataMember()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // SEI
    MOS_ZeroMemory(&m_seiData, sizeof(m_seiData));
    m_seiDataOffset  = false;
    m_seiParamBuffer = nullptr;

    m_brcInit                      = false;
    m_brcReset                     = false;
    m_mbBrcEnabled                 = false;
    m_mbBrcUserFeatureKeyControl   = false;
    m_dBrcTargetSize               = 0.0f;
    m_trellis                      = false;
    m_acceleratorHeaderPackingCaps = false;  //flag set by driver from driver caps

    m_dBrcInitCurrentTargetBufFullInBits = 0;
    m_dBrcInitResetInputBitsPerFrame     = 0;
    m_brcInitResetBufSizeInBits          = false;
    m_brcInitPreviousTargetBufFullInBits = false;
    // Below values will be set if qp control params are sent by app
    m_minMaxQpControlEnabled = false;  // Flag to indicate if min/max QP feature is enabled or not.
    m_iMinQp                 = 0;
    m_iMaxQp                 = 0;
    m_pMinQp                 = 0;
    m_pMaxQp                 = 0;
    m_pFrameMinMaxQpControl  = false;  // Indicates min/max QP values for P-frames are set separately or not.

    m_skipFrameBufferSize = false;                                // size of skip frame packed data
    MOS_ZeroMemory(&m_resSkipFrameBuffer, sizeof(MOS_RESOURCE));  // copy skip frame packed data from DDI

    // VDENC BRC Buffers
    MOS_ZeroMemory(&m_resVdencBrcUpdateDmemBuffer, sizeof(MOS_RESOURCE) * CODECHAL_ENCODE_RECYCLED_BUFFER_NUM * CODECHAL_VDENC_BRC_NUM_OF_PASSES);

    MOS_ZeroMemory(&m_resVdencBrcInitDmemBuffer, sizeof(MOS_RESOURCE) * CODECHAL_ENCODE_RECYCLED_BUFFER_NUM);

    MOS_ZeroMemory(&m_resVdencBrcImageStatesReadBuffer, sizeof(MOS_RESOURCE) * CODECHAL_ENCODE_RECYCLED_BUFFER_NUM);

    MOS_ZeroMemory(&m_resVdencBrcConstDataBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_resVdencBrcHistoryBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_resVdencBrcDbgBuffer, sizeof(MOS_RESOURCE));

    // Static frame detection
    m_staticFrameDetectionEnable = false;  // Static frame detection enable
    MOS_ZeroMemory(&m_resSfdOutputBuffer, sizeof(MOS_RESOURCE) * CODECHAL_ENCODE_RECYCLED_BUFFER_NUM);

    MOS_ZeroMemory(&m_resSfdCostTablePFrameBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_resSfdCostTableBFrameBuffer, sizeof(MOS_RESOURCE));
    MOS_ZeroMemory(&m_resVdencSfdImageStateReadBuffer, sizeof(MOS_RESOURCE));
    m_sfdKernelState = nullptr;

    // Generation Specific Support Flags & User Feature Key Reads
    m_mbBrcSupportCaps            = 0;
    m_ftqEnable                   = false;  // FTQEnable
    m_skipBiasAdjustmentSupported = false;  // SkipBiasAdjustment support for P frame
    m_sliceLevelReportSupported   = false;  // Slice Level Report support
    m_brcRoiSupported             = false;
    m_brcMotionAdaptiveEnable     = false;

    m_roundingInterEnable         = false;
    m_adaptiveRoundingInterEnable = false;
    m_roundingInterP              = false;

    MOS_ZeroMemory(m_vdEncModeCost, 12 * sizeof(uint8_t));

    MOS_ZeroMemory(m_vdEncMvCost, 8 * sizeof(uint8_t));

    MOS_ZeroMemory(m_vdEncHmeMvCost, 8 * sizeof(uint8_t));

    m_slidingWindowSize            = false;
    m_forceToSkipEnable            = false;
    m_vdencBrcInitDmemBufferSize   = false;
    m_vdencBrcUpdateDmemBufferSize = false;
    m_vdencStaticFrame             = false;
    m_vdencStaticRegionPct         = false;
}

MOS_STATUS CodechalVdencAvcState::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;

    // Set interleaved scaling output to support PAFF.
    m_fieldScalingOutputInterleaved = true;

    if (m_encEnabled)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AVC_FTQ_ENABLE_ID,
            &userFeatureData);
        m_ftqEnable = (userFeatureData.i32Data) ? true : false;

        m_mbBrcSupportCaps = 1;
        if (m_mbBrcSupportCaps)
        {
            // If the MBBRC user feature key does not exist, MBBRC will be set in SPS parsing
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_AVC_MB_BRC_ENABLE_ID,
                &userFeatureData);
            if (userFeatureData.i32Data == 0 || userFeatureData.i32Data == 1)
            {
                m_mbBrcUserFeatureKeyControl = true;
                m_mbBrcEnabled               = userFeatureData.i32Data ? true : false;
            }
        }

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AVC_ENCODE_32xME_ENABLE_ID,
            &userFeatureData);

        if (userFeatureData.i32Data == 0 || userFeatureData.i32Data == 1)
        {
            m_32xMeUserfeatureControl = true;
            m_32xMeSupported          = (userFeatureData.i32Data) ? true : false;
        }
        else
        {
            m_32xMeUserfeatureControl = false;
            m_32xMeSupported          = true;
        }
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_ENABLE_ID,
        &userFeatureData);
    m_roundingInterEnable = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = CODECHAL_ENCODE_AVC_INVALID_ROUNDING;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AVC_ROUNDING_INTER_P_ID,
        &userFeatureData);
    m_roundingInterP = userFeatureData.i32Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = 1;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AVC_ADAPTIVE_ROUNDING_INTER_ENABLE_ID,
        &userFeatureData);
    m_adaptiveRoundingInterEnable = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_AVC_SKIP_BIAS_ADJUSTMENT_ENABLE_ID,
        &userFeatureData);
    m_skipBiasAdjustmentSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = CODECHAL_VDENC_AVC_MB_SLICE_TRHESHOLD;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_MB_SLICE_THRESHOLD_ID,
        &userFeatureData);
    m_mbSlcThresholdValue = (userFeatureData.i32Data);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = USE_SLICE_THRESHOLD_TABLE_100_PERCENT;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_SLICE_THRESHOLD_TABLE_ID,
        &userFeatureData);
    m_sliceThresholdTable = (userFeatureData.i32Data);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = MHW_VDBOX_VDENC_DYNAMIC_SLICE_WA_COUNT;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_TAIL_INSERTION_DELAY_COUNT_ID,
        &userFeatureData);
    m_vdencFlushDelayCount = (userFeatureData.i32Data);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data     = AVC_I_SLICE_SIZE_MINUS;
    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_THRESHOLD_I_SLICE_SIZE_MINUS_ID,
        &userFeatureData);
    m_vdencSliceMinusI = (userFeatureData.i32Data);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    userFeatureData.i32Data = AVC_P_SLICE_SIZE_MINUS;
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_THRESHOLD_P_SLICE_SIZE_MINUS_ID,
        &userFeatureData);
    m_vdencSliceMinusP = (userFeatureData.i32Data);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENC_BRC_MOTION_ADAPTIVE_ENABLE_ID,
        &userFeatureData);
    m_brcMotionAdaptiveEnable = (userFeatureData.i32Data) ? true : false;

    m_vdencBrcStatsBufferSize    = AVC_BRC_STATS_BUF_SIZE;
    m_vdencBrcPakStatsBufferSize = AVC_BRC_PAK_STATS_BUF_SIZE;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pAvcSliceParams);

    uint8_t numRefIdx0MinusOne = params->pAvcSliceParams->num_ref_idx_l0_active_minus1;
    uint8_t numRefIdx1MinusOne = params->pAvcSliceParams->num_ref_idx_l1_active_minus1;

    // Nothing to do here if numRefIdx = 0 and frame encoded
    if (numRefIdx0MinusOne == 0 && !CodecHal_PictureIsField(params->pPicParams->CurrOriginalPic))
    {
        if (params->wPictureCodingType == P_TYPE ||
            (params->wPictureCodingType == B_TYPE && numRefIdx1MinusOne == 0))
        {
            return eStatus;
        }
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        uint8_t maxPNumRefIdx0MinusOne = MaxRefIdx0[params->pSeqParams->TargetUsage];
        if (numRefIdx0MinusOne > maxPNumRefIdx0MinusOne)
        {
            CODECHAL_ENCODE_NORMALMESSAGE("Invalid active reference list size.");
            numRefIdx0MinusOne = maxPNumRefIdx0MinusOne;
        }

        numRefIdx1MinusOne = 0;
    }

    // Override number of references used by VME. PAK uses value from DDI (num_ref_idx_l*_active_minus1_from_DDI)
    params->pAvcSliceParams->num_ref_idx_l0_active_minus1 = numRefIdx0MinusOne;
    params->pAvcSliceParams->num_ref_idx_l1_active_minus1 = numRefIdx1MinusOne;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::GetInterRounding(PMHW_VDBOX_AVC_SLICE_STATE sliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState->pEncodeAvcSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState->pEncodeAvcPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState->pEncodeAvcSliceParams);

    auto    avcSeqParams   = sliceState->pEncodeAvcSeqParams;
    auto    avcPicParams   = sliceState->pEncodeAvcPicParams;
    auto    avcSliceParams = sliceState->pEncodeAvcSliceParams;
    uint8_t sliceQP        = avcPicParams->pic_init_qp_minus26 + 26 + avcSliceParams->slice_qp_delta;

    switch (Slice_Type[avcSliceParams->slice_type])
    {
    case SLICE_P:
        if (m_roundingInterP == CODECHAL_ENCODE_AVC_INVALID_ROUNDING)
        {
            // Adaptive Rounding is only used in CQP case
            if (m_adaptiveRoundingInterEnable && !m_vdencBrcEnabled)
            {
                // If IPPP scenario
                if (avcSeqParams->GopRefDist == 1)
                {
                    sliceState->dwRoundingValue = CodechalVdencAvcState::AdaptiveInterRoundingPWithoutB[sliceQP];
                }
                else
                {
                    sliceState->dwRoundingValue = CodechalVdencAvcState::AdaptiveInterRoundingP[sliceQP];
                }
            }
            else
            {
                sliceState->dwRoundingValue = CodechalVdencAvcState::InterRoundingP[avcSeqParams->TargetUsage];
            }
        }
        else
        {
            sliceState->dwRoundingValue = m_roundingInterP;
        }
        break;
    case SLICE_B:
        if (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef)
        {
            sliceState->dwRoundingValue = InterRoundingBRef[avcSeqParams->TargetUsage];
        }
        else
        {
            if (m_adaptiveRoundingInterEnable && !m_vdencBrcEnabled)
            {
                sliceState->dwRoundingValue = AdaptiveInterRoundingB[sliceQP];
            }
            else
            {
                sliceState->dwRoundingValue = InterRoundingB[avcSeqParams->TargetUsage];
            }
        }
        break;
    default:
        // do nothing
        break;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::GetSkipBiasAdjustment(uint8_t sliceQP, uint16_t gopRefDist, bool *skipBiasAdjustmentEnable)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(skipBiasAdjustmentEnable);

    // Determine if SkipBiasAdjustment should be enabled for P picture
    // 1. No B frame 2. Qp >= 22 3. CQP mode
    *skipBiasAdjustmentEnable = m_skipBiasAdjustmentSupported && (m_pictureCodingType == P_TYPE) && (gopRefDist == 1) && (sliceQP >= CODECHAL_ENCODE_AVC_SKIP_BIAS_ADJUSTMENT_QP_THRESHOLD);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::GetHmeSupportedBasedOnTU(HmeLevel hmeLevel, bool *supported)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(supported);

    switch (hmeLevel)
    {
    case HME_LEVEL_4x:
        //HME always supported
        *supported = true;
        break;
    case HME_LEVEL_16x:
        *supported = SHMEEnabled[m_targetUsage & 0x7] ? true : false;
        break;
    case HME_LEVEL_32x:
        *supported = UHMEEnabled[m_targetUsage & 0x7] ? true : false;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid hme Level");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        break;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::InitKernelStateSFD()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_sfdKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_sfdKernelState);

    uint8_t *kernelBinary;
    uint32_t kernelSize;

    uint32_t   kuid   = m_useCommonKernel ? m_kuidCommon : m_kuid;
    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        kernelBinary,
        ENC_SFD,
        0,
        &currKrnHeader,
        &kernelSize));

    auto kernelStatePtr                            = m_sfdKernelState;
    kernelStatePtr->KernelParams.iBTCount          = CODECHAL_ENCODE_AVC_SFD_NUM_SURFACES;
    kernelStatePtr->KernelParams.iThreadCount      = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength      = sizeof(CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON);
    kernelStatePtr->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount          = 1;
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

MOS_STATUS CodechalVdencAvcState::SetCurbeSFD(PCODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CODECHAL_ENCODE_AVC_SFD_CURBE_COMMON curbe;
    MOS_ZeroMemory(&curbe, sizeof(curbe));
    curbe.DW0.EnableIntraCostScalingForStaticFrame = 1;
    curbe.DW0.EnableAdaptiveMvStreamIn             = 0;  // VDEnc
    curbe.DW0.StreamInType                         = 7;  // VDEnc
    curbe.DW0.SliceType                            = (m_pictureCodingType + 1) % 3;
    curbe.DW0.BRCModeEnable                        = (m_vdencBrcEnabled != 0);
    curbe.DW0.VDEncModeDisable                     = false;

    curbe.DW1.HMEStreamInRefCost = 5;
    curbe.DW1.NumOfRefs          = m_avcSliceParams->num_ref_idx_l0_active_minus1;  // VDEnc
    curbe.DW1.QPValue            = m_avcPicParam->QpY + m_avcSliceParams->slice_qp_delta;

    // SFD kernel requires to round-down to 4-MB aligned
    curbe.DW2.FrameHeightInMBs      = (m_oriFrameHeight / CODECHAL_MACROBLOCK_HEIGHT >> 2) << 2;
    curbe.DW2.FrameWidthInMBs       = (m_oriFrameWidth / CODECHAL_MACROBLOCK_WIDTH >> 2) << 2;
    curbe.DW3.LargeMvThresh         = 128;
    uint32_t totalMb                = curbe.DW2.FrameWidthInMBs * curbe.DW2.FrameHeightInMBs;
    curbe.DW4.TotalLargeMvThreshold = totalMb / 100;
    curbe.DW5.ZMVThreshold          = 4;
    curbe.DW6.TotalZMVThreshold     = totalMb * m_avcPicParam->dwZMvThreshold / 100;
    curbe.DW7.MinDistThreshold      = 10;

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(curbe.CostTable, CODEC_AVC_NUM_QP * sizeof(uint8_t), m_codechalEncodeAvcSfdCostTableVdEnc, CODEC_AVC_NUM_QP * sizeof(uint8_t)),
        "Failed to copy VDEnc SFD cost table");

    curbe.DW21.ActualHeightInMB            = curbe.DW2.FrameHeightInMBs;
    curbe.DW21.ActualWidthInMB             = curbe.DW2.FrameWidthInMBs;
    curbe.DW24.VDEncInputImagStateIndex    = CODECHAL_ENCODE_AVC_SFD_VDENC_INPUT_IMAGE_STATE_COMMON;
    curbe.DW26.MVDataSurfaceIndex          = CODECHAL_ENCODE_AVC_SFD_MV_DATA_SURFACE_COMMON;
    curbe.DW27.InterDistortionSurfaceIndex = CODECHAL_ENCODE_AVC_SFD_INTER_DISTORTION_SURFACE_COMMON;
    curbe.DW28.OutputDataSurfaceIndex      = CODECHAL_ENCODE_AVC_SFD_OUTPUT_DATA_SURFACE_COMMON;
    curbe.DW29.VDEncOutputImagStateIndex   = CODECHAL_ENCODE_AVC_SFD_VDENC_OUTPUT_IMAGE_STATE_COMMON;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateSfdParam(
            &curbe));)

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto seqParams = m_avcSeqParam;

    if (m_targetUsageOverride)
    {
        seqParams->TargetUsage = m_targetUsageOverride;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcBase::SetSequenceStructs());

    // App does tail insertion in VDEnc dynamic slice non-CP case
    m_vdencNoTailInsertion =
        seqParams->EnableSliceLevelRateCtrl &&
        (!m_osInterface->osCpInterface->IsCpEnabled());

    // If 16xMe is supported then check if it is supported in the TU settings
    if (!m_16xMeUserfeatureControl && m_16xMeSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetHmeSupportedBasedOnTU(HME_LEVEL_16x, &m_16xMeSupported));
    }

    // If 32xMe is supported then check if it is supported in the TU settings
    if (!m_32xMeUserfeatureControl && m_32xMeSupported)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetHmeSupportedBasedOnTU(HME_LEVEL_32x, &m_32xMeSupported));
    }

    if (m_firstFrame)
    {
        m_oriFrameHeight = seqParams->FrameHeight;
        m_oriFrameWidth  = seqParams->FrameWidth;
    }

    // check if there is a dynamic resolution change
    if ((m_oriFrameHeight && (m_oriFrameHeight != seqParams->FrameHeight)) ||
        (m_oriFrameWidth && (m_oriFrameWidth != seqParams->FrameWidth)))
    {
        m_resolutionChanged = true;
        m_oriFrameHeight    = seqParams->FrameHeight;
        m_oriFrameWidth     = seqParams->FrameWidth;
        // Need to call BRCInit instead of BRCReset for dynamic resolution change
        m_brcInit = true;
    }
    else
    {
        m_resolutionChanged = false;
    }

    // HuC based BRC should be used when 1) BRC is requested by App and 2) HuC FW is loaded and HuC is enabled for use.
    if (CodecHalIsRateControlBrc(seqParams->RateControlMethod, CODECHAL_AVC))
    {
        if (!MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEnableMediaKernels))
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_STATUS_UNKNOWN, "Failed to load HuC firmware!");
        }

        m_vdencBrcEnabled = MEDIA_IS_SKU(m_hwInterface->GetSkuTable(), FtrEnableMediaKernels);
    }

    if (m_mbBrcSupportCaps && (m_vdencBrcEnabled))
    {
        // control MBBRC if the user feature key does not exist
        if (!m_mbBrcUserFeatureKeyControl)
        {
            if (seqParams->RateControlMethod == RATECONTROL_ICQ || seqParams->RateControlMethod == RATECONTROL_QVBR)
            {
                // If the rate control method is ICQ or QVBR then enable MBBRC by default for all TUs and ignore the app input
                m_mbBrcEnabled = true;
                CODECHAL_ENCODE_NORMALMESSAGE("MBBRC enabled with rate control = %d", seqParams->RateControlMethod);
            }
            else if (seqParams->RateControlMethod == RATECONTROL_VCM)
            {
                // If the rate control method is VCM then disable MBBRC by default for all TUs and ignore the app input
                m_mbBrcEnabled = false;
            }
            else
            {
                switch (seqParams->MBBRC)
                {
                case mbBrcInternal:
                    m_mbBrcEnabled = true;
                    break;
                case mbBrcDisabled:
                    m_mbBrcEnabled = false;
                    break;
                case mbBrcEnabled:
                    m_mbBrcEnabled = true;
                    break;
                }
            }
        }
    }

    m_trellis = seqParams->Trellis;

    // Simple check for BRC parameters; if error, disable BRC and continue encoding
    if ((m_vdencBrcEnabled) &&
        ((((!seqParams->InitVBVBufferFullnessInBit ||
               !seqParams->VBVBufferSizeInBit ||
               !seqParams->MaxBitRate) &&
              (seqParams->RateControlMethod != RATECONTROL_AVBR)) ||
             !seqParams->TargetBitRate ||
             !seqParams->FramesPer100Sec) &&
            seqParams->RateControlMethod != RATECONTROL_ICQ))
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Fatal error in AVC Encoding BRC parameters.");
        CODECHAL_ENCODE_ASSERTMESSAGE("RateControlMethod = %d, InitVBVBufferFullnessInBit = %d, VBVBufferSizeInBit = %d, MaxBitRate = %d, TargetBitRate = %d, FramesPer100Sec = %d",
            seqParams->RateControlMethod,
            seqParams->InitVBVBufferFullnessInBit,
            seqParams->VBVBufferSizeInBit,
            seqParams->MaxBitRate,
            seqParams->TargetBitRate,
            seqParams->FramesPer100Sec);
        m_vdencBrcEnabled = false;
    }

    // BRC Init or Reset
    if (seqParams->bInitBRC)
    {
        m_brcInit = seqParams->bInitBRC;
    }
    else
    {
        m_brcReset = seqParams->bResetBRC;
    }

    if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        if (seqParams->ICQQualityFactor < CODECHAL_ENCODE_AVC_MIN_ICQ_QUALITYFACTOR ||
            seqParams->ICQQualityFactor > CODECHAL_ENCODE_AVC_MAX_ICQ_QUALITYFACTOR)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid ICQ Quality Factor input\n");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }
    }

    if (seqParams->EnableSliceLevelRateCtrl)
    {
        m_waReadVDEncOverflowStatus = MEDIA_IS_WA(m_hwInterface->GetWaTable(), WaReadVDEncOverflowStatus);
    }

    // if GOP structure is I-frame only, we use 3 non-ref slots for tracked buffer
    m_gopIsIdrFrameOnly = (seqParams->GopPicSize == 1 && seqParams->GopRefDist == 0);

    // Set sliding window size to one second by default, up to 60
    if (m_slidingWindowSize == 0)
    {
        m_slidingWindowSize = MOS_MIN((uint32_t)(seqParams->FramesPer100Sec / 100), 60);
    }

    m_maxNumSlicesAllowed = CodecHalAvcEncode_GetMaxNumSlicesAllowed(
        (CODEC_AVC_PROFILE_IDC)(seqParams->Profile),
        (CODEC_AVC_LEVEL_IDC)(seqParams->Level),
        seqParams->FramesPer100Sec);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto picParams  = m_avcPicParam;
    auto seqParams  = m_avcSeqParam;
    auto avcRefList = &m_refList[0];
    auto avcPicIdx  = &m_picIdx[0];

    uint8_t prevRefIdx = m_currReconstructedPic.FrameIdx;
    uint8_t currRefIdx = picParams->CurrReconstructedPic.FrameIdx;

    int16_t prevFrameNum = m_frameNum;
    int16_t currFrameNum = picParams->frame_num;

    if (m_firstFrame)
    {
        m_oriFieldCodingFlag = picParams->FieldCodingFlag;
    }

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) &&
        (!picParams->UserFlags.bUseRawPicForRef || m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Sync initialize
    if ((m_firstFrame) ||
        (picParams->UserFlags.bUseRawPicForRef) ||  // No need to wait for previous PAK if reconstructed pic is not used as reference (but RAW pic is used for ref)
        ((picParams->CodingType == I_TYPE)) ||      // No need to wait for I-Frames
        ((currFrameNum == prevFrameNum) &&
            CodecHal_PictureIsFrame(picParams->CurrOriginalPic)) ||  // No need to wait if current and previous pics have same frame numbers (Same reference list is used for two pictures with same frame numbers)
        (!avcRefList[prevRefIdx]->bUsedAsRef &&
            CodecHal_PictureIsField(picParams->CurrOriginalPic)))
    {
        m_waitForPak = false;
    }
    else
    {
        m_waitForPak = true;
    }

    m_signalEnc = false;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcBase::SetPictureStructs());

    m_hwInterface->GetMfxInterface()->SetBrcNumPakPasses(GetNumBrcPakPasses(picParams->BRCPrecision));
    if (m_vdencBrcEnabled)
    {
        m_numPasses = CODECHAL_VDENC_BRC_NUM_OF_PASSES - 1;  // 2-pass VDEnc BRC

        if (picParams->BRCPrecision == 1)
        {
            m_vdencSinglePassEnable = true;
            m_numPasses             = 0;
        }
    }
    else
    {
        // legacy AVC : 1 original plus extra to handle IPCM MBs
        // VDENC : single PAK pass
        m_numPasses = (CODECHAL_VDENC_AVC_CQP_NUM_OF_PASSES - 1);
    }

    if (seqParams->RateControlMethod == RATECONTROL_VCM && m_pictureCodingType == B_TYPE)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("VCM BRC mode does not support B-frames\n");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (seqParams->RateControlMethod == RATECONTROL_VCM && (picParams->FieldCodingFlag || picParams->FieldFrameCodingFlag))
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("VCM BRC mode does not support interlaced\n");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (picParams->FieldCodingFlag || picParams->FieldFrameCodingFlag)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("VDEnc does not support interlaced picture\n");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    avcRefList[currRefIdx]->pRefPicSelectListEntry = nullptr;

    if (m_avcPicParam->NumDirtyROI)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupDirtyROI(
            &(m_resVdencStreamInBuffer[m_currRecycledBufIdx])));
    }

    if (m_avcPicParam->NumROI)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetupROIStreamIn(
            m_avcPicParam,
            &(m_resVdencStreamInBuffer[m_currRecycledBufIdx])));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetSliceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto slcParams = m_avcSliceParams;
    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;

    if (m_pictureCodingType != I_TYPE)
    {
        CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS validateNumRefsParams;
        validateNumRefsParams.pSeqParams            = seqParams;
        validateNumRefsParams.pPicParams            = picParams;
        validateNumRefsParams.pAvcSliceParams       = slcParams;
        validateNumRefsParams.wPictureCodingType    = m_pictureCodingType;
        validateNumRefsParams.wPicHeightInMB        = m_picHeightInMb;
        validateNumRefsParams.wFrameFieldHeightInMB = m_frameFieldHeightInMb;
        validateNumRefsParams.bFirstFieldIPic       = m_firstFieldIdrPic;
        validateNumRefsParams.bVDEncEnabled         = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ValidateNumReferences(&validateNumRefsParams));
    }
    else
    {
        slcParams->num_ref_idx_l0_active_minus1 = 0;
        slcParams->num_ref_idx_l1_active_minus1 = 0;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcBase::SetSliceStructs());

    if (eStatus == MOS_STATUS_INVALID_PARAMETER)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid slice parameters.");
    }
    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SendSFDSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    // VDEnc input/output image state (CQP mode)
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    auto                          kernelState = params->pKernelState;
    if (params->bVdencActive && !params->bVdencBrcEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->presVDEncImageStateInputBuffer);
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer            = params->presVDEncImageStateInputBuffer;
        surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(m_hwInterface->m_vdencBrcImgStateBufferSize);
        surfaceCodecParams.dwOffset              = 0;
        surfaceCodecParams.bRenderTarget         = false;
        surfaceCodecParams.bIsWritable           = false;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_VDENC_IMAGESTATE_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = CODECHAL_ENCODE_AVC_SFD_VDENC_INPUT_IMAGE_STATE_COMMON;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->presVDEncImageStateOutputBuffer);
        surfaceCodecParams.presBuffer            = params->presVDEncImageStateOutputBuffer;
        surfaceCodecParams.bRenderTarget         = true;
        surfaceCodecParams.bIsWritable           = true;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_VDENC_IMAGESTATE_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = CODECHAL_ENCODE_AVC_SFD_VDENC_OUTPUT_IMAGE_STATE_COMMON;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // HME MV Data surface
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeMvDataSurface);
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface          = true;
    surfaceCodecParams.bMediaBlockRW         = true;
    surfaceCodecParams.psSurface             = params->psMeMvDataSurface;
    surfaceCodecParams.dwOffset              = params->dwMeMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = CODECHAL_ENCODE_AVC_SFD_MV_DATA_SURFACE_COMMON;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // HME distortion surface
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeDistortionSurface);
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface          = true;
    surfaceCodecParams.bMediaBlockRW         = true;
    surfaceCodecParams.psSurface             = params->psMeDistortionSurface;
    surfaceCodecParams.dwOffset              = params->dwMeDistortionBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = CODECHAL_ENCODE_AVC_SFD_INTER_DISTORTION_SURFACE_COMMON;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // output buffer
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presOutputBuffer);
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = params->presOutputBuffer;
    surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE);
    surfaceCodecParams.dwOffset              = 0;
    surfaceCodecParams.bRenderTarget         = true;
    surfaceCodecParams.bIsWritable           = true;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = CODECHAL_ENCODE_AVC_SFD_OUTPUT_DATA_SURFACE_COMMON;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SFDKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL : CODECHAL_ENCODE_PERFTAG_CALL_SFD_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    // set function type and kernel state pointer
    auto encFunctionType = CODECHAL_MEDIA_STATE_STATIC_FRAME_DETECTION;
    auto kernelState     = m_sfdKernelState;

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        uint32_t dwMaxBtCount = m_singleTaskPhaseSupported ? m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            dwMaxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(dwMaxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &idParams));

    // set-up SFD Curbe
    CODECHAL_ENCODE_AVC_SFD_CURBE_PARAMS sfdcurbeParams;
    MOS_ZeroMemory(&sfdcurbeParams, sizeof(sfdcurbeParams));
    sfdcurbeParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeSFD(&sfdcurbeParams));

    // dump DSH/Curbe/ISH
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            kernelState));)

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType      = encFunctionType;
    sendKernelCmdsParams.pKernelState         = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    if (!m_vdencBrcEnabled)
    {
        //Set VDENC image state command in VDENC buffer for CQP mode
        PMHW_VDBOX_AVC_IMG_PARAMS imageStateParams = CreateMhwVdboxAvcImgParams();
        CODECHAL_ENCODE_CHK_NULL_RETURN(imageStateParams);

        imageStateParams->pEncodeAvcPicParams   = m_avcPicParam;
        imageStateParams->pEncodeAvcSeqParams   = m_avcSeqParam;
        imageStateParams->pEncodeAvcSliceParams = m_avcSliceParams;
        imageStateParams->wPicWidthInMb         = m_picWidthInMb;
        imageStateParams->wPicHeightInMb        = m_picHeightInMb;
        imageStateParams->wSlcHeightInMb        = m_sliceHeight;
        imageStateParams->dwMaxVmvR             = CodecHalAvcEncode_GetMaxVmvR(m_avcSeqParam->Level);
        imageStateParams->bVdencBRCEnabled      = m_vdencBrcEnabled;
        imageStateParams->bVdencStreamInEnabled = m_vdencStreamInEnabled;
        imageStateParams->bCrePrefetchEnable    = m_crePrefetchEnable;

        if (m_avcSeqParam->EnableSliceLevelRateCtrl)
        {
            imageStateParams->dwMbSlcThresholdValue = m_mbSlcThresholdValue;
        }

        imageStateParams->pVDEncModeCost  = m_vdencModeCostTbl;
        imageStateParams->pVDEncHmeMvCost = m_vdencHmeMvCostTbl;
        imageStateParams->pVDEncMvCost    = m_vdencMvCostTbl;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AddVdencSfdImgBuffer(
            &m_resVdencSfdImageStateReadBuffer, imageStateParams));
        MOS_Delete(imageStateParams);

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
                0,
                nullptr));)
    }

    // Add surface states
    CODECHAL_ENCODE_AVC_SFD_SURFACE_PARAMS sfdsurfaceParams;
    MOS_ZeroMemory(&sfdsurfaceParams, sizeof(sfdsurfaceParams));
    sfdsurfaceParams.dwDownscaledWidthInMb4x         = m_downscaledWidthInMb4x;
    sfdsurfaceParams.dwDownscaledHeightInMb4x        = m_downscaledFrameFieldHeightInMb4x;
    sfdsurfaceParams.psMeMvDataSurface               = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer) : &m_4xMeMvDataBuffer;
    sfdsurfaceParams.dwMeMvBottomFieldOffset         = m_hmeKernel ? m_hmeKernel->Get4xMeMvBottomFieldOffset() : (uint32_t)m_meMvBottomFieldOffset;
    sfdsurfaceParams.psMeDistortionSurface           = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer) : &m_4xMeDistortionBuffer;
    sfdsurfaceParams.dwMeDistortionBottomFieldOffset = m_hmeKernel ? m_hmeKernel->GetDistortionBottomFieldOffset() : (uint32_t)m_meDistortionBottomFieldOffset;
    sfdsurfaceParams.bVdencActive                    = true;
    sfdsurfaceParams.bVdencBrcEnabled                = m_vdencBrcEnabled;
    sfdsurfaceParams.presOutputBuffer                = &m_resSfdOutputBuffer[m_currRecycledBufIdx];
    sfdsurfaceParams.pKernelState                    = kernelState;
    if (!m_vdencBrcEnabled)
    {
        sfdsurfaceParams.presVDEncImageStateInputBuffer  = &m_resVdencSfdImageStateReadBuffer;
        sfdsurfaceParams.presVDEncImageStateOutputBuffer = &m_batchBufferForVdencImgStat[m_currRecycledBufIdx].OsResource;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSFDSurfaces(
        &cmdBuffer,
        &sfdsurfaceParams));

    // dump SSH
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));)

    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MediaObjectInlineData   mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData      = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetupROIStreamIn(PCODEC_AVC_ENCODE_PIC_PARAMS picParams, PMOS_RESOURCE vdencStreamIn)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(picParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencStreamIn);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;

    CODECHAL_VDENC_STREAMIN_STATE *pData = (CODECHAL_VDENC_STREAMIN_STATE *)m_osInterface->pfnLockResource(
        m_osInterface,
        vdencStreamIn,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

    MOS_ZeroMemory(pData, m_picHeightInMb * m_picWidthInMb * CODECHAL_VDENC_STREAMIN_STATE::byteSize);
    // ROI 0 reserved for non-ROI zone, VDEnc support max 3 ROIs
    CODECHAL_ENCODE_ASSERT(picParams->NumROI < 4);

    m_vdencStreamInEnabled = true;

    // legacy AVC ROI[n]->VDEnc ROI[n+1], ROI 1 has higher priority than 2 and so on
    for (int32_t i = picParams->NumROI - 1; i >= 0; i--)
    {
        uint32_t curX, curY;
        for (curY = picParams->ROI[i].Top; curY < picParams->ROI[i].Bottom; curY++)
        {
            for (curX = picParams->ROI[i].Left; curX < picParams->ROI[i].Right; curX++)
            {
                (pData + (m_picWidthInMb * curY + curX))->DW0.RegionOfInterestRoiSelection = i + 1;  //Shift ROI by 1
            }
        }
    }

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        vdencStreamIn);

    return eStatus;
}

bool CodechalVdencAvcState::ProcessRoiDeltaQp()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Intialize ROIDistinctDeltaQp to be min expected delta qp, setting to -128
    // Check if forceQp is needed or not
    // forceQp is enabled if there are greater than 3 distinct delta qps or if the deltaqp is beyond range (-8, 7)
    for (auto k = 0; k < m_maxNumRoi; k++)
    {
        m_avcPicParam->ROIDistinctDeltaQp[k] = -128;
    }

    int32_t numQp = 0;
    for (int32_t i = 0; i < m_avcPicParam->NumROI; i++)
    {
        bool dqpNew = true;

        //Get distinct delta Qps among all ROI regions, index 0 having the lowest delta qp
        int32_t k = numQp - 1;
        for (; k >= 0; k--)
        {
            if (m_avcPicParam->ROI[i].PriorityLevelOrDQp == m_avcPicParam->ROIDistinctDeltaQp[k] ||
                m_avcPicParam->ROI[i].PriorityLevelOrDQp == 0)
            {
                dqpNew = false;
                break;
            }
            else if (m_avcPicParam->ROI[i].PriorityLevelOrDQp < m_avcPicParam->ROIDistinctDeltaQp[k])
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (dqpNew)
        {
            for (int32_t j = numQp - 1; (j >= k + 1 && j >= 0); j--)
            {
                m_avcPicParam->ROIDistinctDeltaQp[j + 1] = m_avcPicParam->ROIDistinctDeltaQp[j];
            }
            m_avcPicParam->ROIDistinctDeltaQp[k + 1] = m_avcPicParam->ROI[i].PriorityLevelOrDQp;
            numQp++;
        }
    }

    //Set the ROI DeltaQp to zero for remaining array elements
    for (auto k = numQp; k < m_maxNumRoi; k++)
    {
        m_avcPicParam->ROIDistinctDeltaQp[k] = 0;
    }

    // return whether is native ROI or not
    return (!(numQp > m_maxNumNativeRoi || m_avcPicParam->ROIDistinctDeltaQp[0] < -8 || m_avcPicParam->ROIDistinctDeltaQp[numQp - 1] > 7));
}


MOS_STATUS CodechalVdencAvcState::SetupDirtyROI(PMOS_RESOURCE vdencStreamIn)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencStreamIn);

    m_vdencStaticFrame     = false;
    m_vdencStaticRegionPct = 0;

    //Dirty ROI feature just for p frame
    if (m_pictureCodingType != P_TYPE)
    {
        return eStatus;
    }

    //Dirty ROI feature just for the previous frame reference
    auto ppsIdx          = m_avcSliceParams->pic_parameter_set_id;
    auto refPicListIdx   = m_avcSliceParams[ppsIdx].RefPicList[0][0].FrameIdx;
    auto refFrameListIdx = m_avcPicParam[ppsIdx].RefFrameList[refPicListIdx].FrameIdx;
    if (m_prevReconFrameIdx != refFrameListIdx)
    {
        return eStatus;
    }

    auto picParams = m_avcPicParam;
    CODECHAL_ENCODE_CHK_NULL_RETURN(picParams);

    // calculate the non-dirty percentage
    uint16_t staticArea = m_picHeightInMb * m_picWidthInMb;

    for (int32_t i = picParams->NumDirtyROI - 1; i >= 0; i--)
    {
        staticArea -= (picParams->DirtyROI[i].Right - picParams->DirtyROI[i].Left) *
                      (picParams->DirtyROI[i].Bottom - picParams->DirtyROI[i].Top);
    }

    uint16_t staticRegionPct = (MOS_MAX(0, staticArea * 256)) / (m_picHeightInMb * m_picWidthInMb);
    m_vdencStaticFrame       = staticRegionPct > (uint16_t)(CODECHAL_VDENC_AVC_STATIC_FRAME_ZMV_PERCENT * 256 / 100.0);
    m_vdencStaticRegionPct   = staticRegionPct;

    //BRC + MBQP => streamIn
    if (m_vdencBrcEnabled && m_mbBrcEnabled)
    {
        m_vdencStreamInEnabled = true;

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        auto pData = (CODECHAL_VDENC_STREAMIN_STATE *)m_osInterface->pfnLockResource(
            m_osInterface,
            vdencStreamIn,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_ZeroMemory(pData, m_picHeightInMb * m_picWidthInMb * CODECHAL_VDENC_STREAMIN_STATE::byteSize);

        for (int32_t i = picParams->NumDirtyROI - 1; i >= 0; i--)
        {
            for (uint32_t curY = picParams->DirtyROI[i].Top; curY < picParams->DirtyROI[i].Bottom; curY++)
            {
                for (uint32_t curX = picParams->DirtyROI[i].Left; curX < picParams->DirtyROI[i].Right; curX++)
                {
                    (pData + (m_picWidthInMb * curY + curX))->DW0.RegionOfInterestRoiSelection = 1;
                }
            }
        }

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            vdencStreamIn);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::HuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    auto avcSeqParams = m_avcSeqParam;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        MHW_MI_MMIOREGISTERS mmioRegister;
        bool validMmio = m_mfxInterface->ConvertToMiRegister(m_vdboxIndex, mmioRegister);
        // Send command buffer header at the beginning (OS dependent)
        bool bRequestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(
            &cmdBuffer, bRequestFrameTracking, validMmio ? &mmioRegister: nullptr));
    }

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucVdencBrcInitKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = CreateMhwVdboxPipeModeSelectParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(pipeModeSelectParams);

    pipeModeSelectParams->Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));
    MOS_Delete(pipeModeSelectParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcInitReset());

    // set HuC DMEM param
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx];
    dmemParams.dwDataLength      = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_resVdencBrcHistoryBuffer;
    virtualAddrParams.regionParams[0].isWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcDummyStreamObject(&cmdBuffer));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AvcVdencStoreHuCStatus2Register(m_hwInterface, &cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bFlushHEVC    = 1;
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdpipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (!m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, renderingFlags));
    }

    CODECHAL_DEBUG_TOOL(DumpHucBrcInit());
    m_firstTaskInPhase = false;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::HuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);
    auto avcSeqParams  = m_avcSeqParam;
    auto avcPicParams  = m_avcPicParam;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (!m_singleTaskPhaseSupported || (m_firstTaskInPhase && !m_brcInit))
    {
        MHW_MI_MMIOREGISTERS mmioRegister;
        bool validMmio = m_mfxInterface->ConvertToMiRegister(m_vdboxIndex, mmioRegister);

        // Send command buffer header at the beginning (OS dependent)
        bool bRequestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            SendPrologWithFrameTracking(&cmdBuffer, bRequestFrameTracking, validMmio ? &mmioRegister : nullptr));
    }

    if (m_brcInit || m_brcReset)
    {
        // Insert conditional batch buffer end for HuC valid IMEM loaded check
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;

        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_resHucStatus2Buffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    //Set MFX/VDENC image state command in VDENC BRC buffer
    PMHW_VDBOX_AVC_IMG_PARAMS imageStateParams = CreateMhwVdboxAvcImgParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(imageStateParams);
    SetMfxAvcImgStateParams(*imageStateParams);
    imageStateParams->bVdencBRCEnabled           = 1;
    imageStateParams->bSliceSizeStreamOutEnabled = m_sliceSizeStreamoutSupported;

    if (avcSeqParams->EnableSliceLevelRateCtrl)
    {
        imageStateParams->dwMbSlcThresholdValue  = m_mbSlcThresholdValue;
        imageStateParams->dwSliceThresholdTable  = m_sliceThresholdTable;
        imageStateParams->dwVdencSliceMinusBytes = (m_pictureCodingType == I_TYPE) ? m_vdencSliceMinusI : m_vdencSliceMinusP;
    }

    if (m_minMaxQpControlEnabled)
    {
        // Convert range [1,51] to [10,51] for VDEnc due to HW limitation
        if (m_pictureCodingType == I_TYPE)
        {
            imageStateParams->pEncodeAvcPicParams->ucMaximumQP = MOS_MAX(m_iMaxQp, 10);
            imageStateParams->pEncodeAvcPicParams->ucMinimumQP = MOS_MAX(m_iMinQp, 10);
        }
        else if (m_pictureCodingType == P_TYPE)
        {
            imageStateParams->pEncodeAvcPicParams->ucMaximumQP = MOS_MAX(m_pMaxQp, 10);
            imageStateParams->pEncodeAvcPicParams->ucMinimumQP = MOS_MAX(m_pMinQp, 10);
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AddVdencBrcImgBuffer(
        &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx],
        imageStateParams));
    MOS_Delete(imageStateParams);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
            nullptr,
            nullptr));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
            0,
            nullptr));)

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucVdencBrcUpdateKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = CreateMhwVdboxPipeModeSelectParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(pipeModeSelectParams);

    pipeModeSelectParams->Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));
    MOS_Delete(pipeModeSelectParams);

    // DMEM set
    SetDmemHuCBrcUpdate();
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));

    dmemParams.presHucDataSource = &(m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass]);
    dmemParams.dwDataLength      = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    // Set Const Data buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetConstDataHuCBrcUpdate());

    // Add Virtual addr
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    // Input regions
    virtualAddrParams.regionParams[1].presRegion = &m_vdencStatsBuffer;
    virtualAddrParams.regionParams[2].presRegion = &m_pakStatsBuffer;
    virtualAddrParams.regionParams[3].presRegion = &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx];
    if (m_staticFrameDetectionInUse)
    {
        virtualAddrParams.regionParams[4].presRegion = &m_resSfdOutputBuffer[m_currRecycledBufIdx];
    }
    virtualAddrParams.regionParams[5].presRegion = &m_resVdencBrcConstDataBuffer;
    // Output regions
    virtualAddrParams.regionParams[0].presRegion = &m_resVdencBrcHistoryBuffer;
    virtualAddrParams.regionParams[0].isWritable = true;
    virtualAddrParams.regionParams[6].presRegion = &m_batchBufferForVdencImgStat[0].OsResource;
    virtualAddrParams.regionParams[6].isWritable = true;
    // region 15 always in clear
    virtualAddrParams.regionParams[15].presRegion = &m_resVdencBrcDbgBuffer;

    if (m_sliceSizeStreamoutSupported)
    {
        virtualAddrParams.regionParams[7].presRegion = &m_pakSliceSizeStreamoutBuffer;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcDummyStreamObject(&cmdBuffer));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AvcVdencStoreHuCStatus2Register(m_hwInterface, &cmdBuffer));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bFlushHEVC    = 1;
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdpipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Write HUC_STATUS mask
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource      = &m_resPakMmioBuffer;
    storeDataParams.dwResourceOffset = sizeof(uint32_t);
    storeDataParams.dwValue          = CODECHAL_VDENC_AVC_BRC_HUC_STATUS_REENCODE_MASK;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    // store HUC_STATUS register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resPakMmioBuffer;
    storeRegParams.dwOffset        = 0;
    storeRegParams.dwRegister      = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    if ((!m_singleTaskPhaseSupported) && (m_osInterface->bNoParsingAssistanceInKmd))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(true));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, renderingFlags));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::LoadCosts(uint16_t pictureCodingType, uint8_t qp)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_ASSERT(qp < CODEC_AVC_NUM_QP);

    MOS_ZeroMemory(m_vdEncModeCost, 12);
    MOS_ZeroMemory(m_vdEncMvCost, 8);
    MOS_ZeroMemory(m_vdEncHmeMvCost, 8);

    m_vdEncModeCost[LutMode_INTRA_NONPRED] = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTRA_NONPRED][qp]), 0x6f);
    m_vdEncModeCost[LutMode_INTRA_16x16]   = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTRA_16x16][qp]), 0x8f);
    m_vdEncModeCost[LutMode_INTRA_8x8]     = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTRA_8x8][qp]), 0x8f);
    m_vdEncModeCost[LutMode_INTRA_4x4]     = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTRA_4x4][qp]), 0x8f);

    if (pictureCodingType == P_TYPE)
    {
        // adjustment due to dirty ROI
        if (m_vdencStaticFrame)
        {
            uint32_t temp                        = AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTRA_16x16][qp];
            temp                                 = (uint32_t)(temp * CODECHAL_VDENC_AVC_STATIC_FRAME_INTRACOSTSCLRatioP / 100.0 + 0.5);
            m_vdEncModeCost[LutMode_INTRA_16x16] = Map44LutValue(temp, 0x8f);
        }
        m_vdEncModeCost[LutMode_INTER_16x16] = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTER_16x16][qp]), 0x8f);
        m_vdEncModeCost[LutMode_INTER_16x8]  = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTER_16x8][qp]), 0x8f);
        m_vdEncModeCost[LutMode_INTER_8x8q]  = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTER_8x8q][qp]), 0x6f);
        m_vdEncModeCost[LutMode_INTER_8x4q]  = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTER_8x4q][qp]), 0x6f);
        m_vdEncModeCost[LutMode_INTER_4x4q]  = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_INTER_4x4q][qp]), 0x6f);
        m_vdEncModeCost[LutMode_REF_ID]      = Map44LutValue((uint32_t)(AVC_Mode_Cost[pictureCodingType - 1][LutMode_REF_ID][qp]), 0x6f);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadMvCost(qp));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadHmeMvCost(qp));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::HuCBrcDummyStreamObject(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // pass dummy buffer by Ind Obj Addr command
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjParams;
    MOS_ZeroMemory(&indObjParams, sizeof(indObjParams));
    indObjParams.presDataBuffer = &m_resVdencBrcDbgBuffer;
    indObjParams.dwDataSize     = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucIndObjBaseAddrStateCmd(cmdBuffer, &indObjParams));

    MHW_VDBOX_HUC_STREAM_OBJ_PARAMS streamObjParams;
    MOS_ZeroMemory(&streamObjParams, sizeof(streamObjParams));
    streamObjParams.dwIndStreamInLength = 1;
    streamObjParams.bHucProcessing      = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStreamObjectCmd(cmdBuffer, &streamObjParams));

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetConstDataHuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Set VDENC BRC constant buffer, data remains the same till BRC Init is called
    if (m_brcInit)
    {
        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        auto            hucConstData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencBrcConstDataBuffer, &lockFlagsWriteOnly);
        FillHucConstData(hucConstData);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcConstDataBuffer);
    }

    if (m_vdencStaticFrame)
    {
        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        auto            hucConstData = (PAVCVdencBRCCostantData)m_osInterface->pfnLockResource(m_osInterface, &m_resVdencBrcConstDataBuffer, &lockFlagsWriteOnly);

        // adjustment due to dirty ROI
        for (int j = 0; j < 42; j++)
        {
            uint32_t temp                     = AVC_Mode_Cost[1][LutMode_INTRA_16x16][10 + j];
            temp                              = (uint32_t)(temp * CODECHAL_VDENC_AVC_STATIC_FRAME_INTRACOSTSCLRatioP / 100.0 + 0.5);
            hucConstData->UPD_P_Intra16x16[j] = Map44LutValue(temp, 0x8f);
        }

        m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcConstDataBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::InitializePicture(const EncoderParams &params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto ppsidx = ((PCODEC_AVC_ENCODE_SLICE_PARAMS)(params.pSliceParams))->pic_parameter_set_id;
    auto spsidx = ((PCODEC_AVC_ENCODE_PIC_PARAMS)(params.pPicParams))->seq_parameter_set_id;

    if (ppsidx >= CODEC_AVC_MAX_PPS_NUM || spsidx >= CODEC_AVC_MAX_SPS_NUM)
    {
        //should never happen
        return MOS_STATUS_UNKNOWN;
    }

    m_madEnabled = params.bMADEnabled;

    m_avcSeqParams[spsidx] = (PCODEC_AVC_ENCODE_SEQUENCE_PARAMS)(params.pSeqParams);
    m_avcPicParams[ppsidx] = (PCODEC_AVC_ENCODE_PIC_PARAMS)(params.pPicParams);
    m_avcQCParams          = (PCODECHAL_ENCODE_AVC_QUALITY_CTRL_PARAMS)params.pAVCQCParams;
    m_avcRoundingParams    = (PCODECHAL_ENCODE_AVC_ROUNDING_PARAMS)params.pAVCRoundingParams;

    m_avcSeqParam           = m_avcSeqParams[spsidx];
    m_avcPicParam           = m_avcPicParams[ppsidx];
    m_avcVuiParams          = (PCODECHAL_ENCODE_AVC_VUI_PARAMS)params.pVuiParams;
    m_avcSliceParams        = (PCODEC_AVC_ENCODE_SLICE_PARAMS)params.pSliceParams;
    m_avcFeiPicParams       = (CodecEncodeAvcFeiPicParams *)params.pFeiPicParams;
    m_avcIQMatrixParams     = (PCODEC_AVC_IQ_MATRIX_PARAMS)params.pIQMatrixBuffer;
    m_avcIQWeightScaleLists = (PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS)params.pIQWeightScaleLists;
    m_nalUnitParams         = params.ppNALUnitParams;
    m_sliceStructCaps       = params.uiSlcStructCaps;

    m_skipFrameFlag = m_avcPicParam->SkipFrameFlag;

    // Picture and slice header packing flag from DDI caps
    m_acceleratorHeaderPackingCaps = params.bAcceleratorHeaderPackingCaps;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcIQMatrixParams);

    // so far this only applies to AVC
    // can move to MotionEstimationDisableCheck() if the logic applies to other codecs later
    if (m_avcQCParams)
    {
        // disable 4x/16x/32 HME if DDI wants to do so, enabling logic is not affected
        if (m_avcQCParams->HMEDisable)
        {
            m_hmeSupported   = false;
            m_16xMeSupported = false;
            m_32xMeSupported = false;
        }
        else if (m_avcQCParams->SuperHMEDisable)
        {
            m_16xMeSupported = false;
            m_32xMeSupported = false;
        }
        else if (m_avcQCParams->UltraHMEDisable)
        {
            m_32xMeSupported = false;
        }
    }

    // Sei for AVC
    if (params.pSeiData != nullptr)
    {
        if (params.pSeiData->dwSEIBufSize > 0)  // sei is present
        {
            if (params.pSeiData->dwSEIBufSize > m_seiData.dwSEIBufSize)
            {
                // Destroy and re-allocate
                if (m_seiData.pSEIBuffer)
                {
                    MOS_FreeMemory(m_seiData.pSEIBuffer);
                    m_seiData.pSEIBuffer = nullptr;
                }
                m_seiData.dwSEIBufSize = params.pSeiData->dwSEIBufSize;
                m_seiData.pSEIBuffer   = (uint8_t *)MOS_AllocAndZeroMemory(m_seiData.dwSEIBufSize);
                CODECHAL_ENCODE_CHK_NULL_RETURN(m_seiData.pSEIBuffer);
            }

            m_seiParamBuffer        = params.pSeiParamBuffer;
            m_seiDataOffset         = params.dwSEIDataOffset;
            m_seiData.newSEIData    = params.pSeiData->newSEIData;
            m_seiData.dwSEIDataSize = params.pSeiData->dwSEIDataSize;

            eStatus = MOS_SecureMemcpy(m_seiData.pSEIBuffer,
                m_seiData.dwSEIDataSize,
                (m_seiParamBuffer + m_seiDataOffset),
                m_seiData.dwSEIDataSize);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
                return eStatus;
            }
        }

        m_extraPictureStatesSize = m_seiData.dwSEIDataSize;
    }

    m_deblockingEnabled = 0;
    for (uint32_t i = 0; i < m_numSlices; i++)
    {
        if (m_avcSliceParams[i].disable_deblocking_filter_idc != 1)
        {
            m_deblockingEnabled = 1;
            break;
        }
    }

    if (m_newSeq)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPictureStructs());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSliceStructs());

    m_scalingEnabled = m_16xMeSupported;
    m_useRawForRef   = m_userFlags.bUseRawPicForRef;

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = m_avcPicParam->CurrOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType          = m_pictureCodingType;

        if (m_newSeq) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
                m_avcSeqParam,
                m_avcIQMatrixParams));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateConstParam());
        }

        if (m_newVuiData) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpVuiParams(
                m_avcVuiParams));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            m_avcPicParam,
            m_avcIQMatrixParams));

        for (uint32_t i = 0; i < m_numSlices; i++) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSliceParams(
                &m_avcSliceParams[i],
                m_avcPicParam));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateDdiParam(
                m_avcSeqParam,
                m_avcPicParam,
                &m_avcSliceParams[i]));
        })

    // Set min/max QP values in AVC State based on frame type if atleast one of them is non-zero
    if (m_avcPicParam->ucMinimumQP || m_avcPicParam->ucMaximumQP)
    {
        m_minMaxQpControlEnabled = true;
        if (m_avcPicParam->CodingType == I_TYPE)
        {
            m_iMaxQp = MOS_MIN(MOS_MAX(m_avcPicParam->ucMaximumQP, 1), 51);        // Clamp to the max QP to [1, 51] . Zero is not used by our Kernel.
            m_iMinQp = MOS_MIN(MOS_MAX(m_avcPicParam->ucMinimumQP, 1), m_iMaxQp);  // Clamp the min QP to [1, maxQP] to make sure minQP <= maxQP
            if (!m_pFrameMinMaxQpControl)
            {
                m_pMinQp = m_iMinQp;
                m_pMaxQp = m_iMaxQp;
            }
        }
        else if (m_avcPicParam->CodingType == P_TYPE)
        {
            m_pFrameMinMaxQpControl = true;
            m_pMaxQp                = MOS_MIN(MOS_MAX(m_avcPicParam->ucMaximumQP, 1), 51);        // Clamp to the max QP to [1, 51]. Zero is not used by our Kernel.
            m_pMinQp                = MOS_MIN(MOS_MAX(m_avcPicParam->ucMinimumQP, 1), m_pMaxQp);  // Clamp the min QP to [1, maxQP] to make sure minQP <= maxQP
        }

        // Zero out the QP values, so we don't update the AVCState settings until new values are sent in MiscParamsRC
        m_avcPicParam->ucMinimumQP = 0;
        m_avcPicParam->ucMaximumQP = 0;
    }

    if (m_codecFunction != CODECHAL_FUNCTION_ENC && !m_userFlags.bDisableAcceleratorHeaderPacking && m_acceleratorHeaderPackingCaps)
    {
        CODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS packPicHeaderParams;
        packPicHeaderParams.pBsBuffer          = &m_bsBuffer;
        packPicHeaderParams.pPicParams         = m_avcPicParam;
        packPicHeaderParams.pSeqParams         = m_avcSeqParam;
        packPicHeaderParams.pAvcVuiParams      = m_avcVuiParams;
        packPicHeaderParams.pAvcIQMatrixParams = m_avcIQMatrixParams;
        packPicHeaderParams.ppNALUnitParams    = m_nalUnitParams;
        packPicHeaderParams.pSeiData           = &m_seiData;
        packPicHeaderParams.dwFrameHeight      = m_frameHeight;
        packPicHeaderParams.dwOriFrameHeight   = m_oriFrameHeight;
        packPicHeaderParams.wPictureCodingType = m_avcPicParam->CodingType;
        packPicHeaderParams.bNewSeq            = m_newSeq;
        packPicHeaderParams.pbNewPPSHeader     = &m_newPpsHeader;
        packPicHeaderParams.pbNewSeqHeader     = &m_newSeqHeader;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_PackPictureHeader(&packPicHeaderParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(m_refList[m_currReconstructedPic.FrameIdx]));

    m_bitstreamUpperBound = m_encodeParams.dwBitstreamSize;
    uint8_t sliceQP       = m_avcPicParam->pic_init_qp_minus26 + 26 + m_avcSliceParams->slice_qp_delta;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetSkipBiasAdjustment(sliceQP, m_avcSeqParam->GopRefDist, &m_skipBiasAdjustmentEnable));

    // Determine if Trellis Quantization should be enabled
    MOS_ZeroMemory(&m_trellisQuantParams, sizeof(m_trellisQuantParams));

    if (!(m_trellis & trellisDisabled))
    {
        if (m_trellis == trellisInternal)
        {
            CODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS tqInputParams;
            tqInputParams.ucQP               = sliceQP;
            tqInputParams.ucTargetUsage      = m_avcSeqParam->TargetUsage;
            tqInputParams.wPictureCodingType = m_pictureCodingType;
            tqInputParams.bBrcEnabled        = false;
            tqInputParams.bVdEncEnabled      = true;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(GetTrellisQuantization(
                &tqInputParams,
                &m_trellisQuantParams));
        }
        else if ((m_pictureCodingType == I_TYPE && (m_trellis & trellisEnabledI)) ||
                 (m_pictureCodingType == P_TYPE && (m_trellis & trellisEnabledP)) ||
                 (m_pictureCodingType == B_TYPE && (m_trellis & trellisEnabledB)))
        {
            m_trellisQuantParams.dwTqEnabled  = true;
            m_trellisQuantParams.dwTqRounding = CODECHAL_ENCODE_AVC_DEFAULT_TRELLIS_QUANT_ROUNDING;
        }
    }

    m_resVdencStatsBuffer = &(m_vdencStatsBuffer);
    m_resPakStatsBuffer   = &(m_pakStatsBuffer);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (!m_cscDsState)
    {
        return eStatus;
    }

    if (m_avcPicParam->bRepeatFrame)
    {
        m_cscDsState->ResetCscFlag();
        m_rawSurfaceToEnc = &m_prevRawSurface;
        m_rawSurfaceToPak = &m_prevRawSurface;
    }

    // SHME and CSC require calling EU kernels
    if (!(m_16xMeSupported || m_cscDsState->RequireCsc()))
    {
        return eStatus;
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf")))

    m_firstTaskInPhase = true;

    if (!m_avcPicParam->bRepeatFrame &&
        ((m_rawSurfaceToEnc->Format == Format_A8R8G8B8) || m_rawSurfaceToEnc->Format == Format_A8B8G8R8))
    {
        m_pollingSyncEnabled = m_avcPicParam->bEnableSync;
        m_syncMarkerOffset   = m_rawSurfaceToEnc->dwPitch * m_avcPicParam->SyncMarkerY + m_avcPicParam->SyncMarkerX * 4;
        if ((m_avcPicParam->SyncMarkerSize >= 4) && (m_avcPicParam->pSyncMarkerValue != nullptr))
        {
            // driver only uses the lower 4 bytes as marker for now, as MI_SEMAPHORE_WAIT only supports 32-bit semaphore data.
            m_syncMarkerValue = *((uint32_t *)m_avcPicParam->pSyncMarkerValue);
        }
        else  // application is not sending valid marker, use default value.
        {
            m_syncMarkerValue = 0x01234501;
        }
    }
    else
    {
        m_pollingSyncEnabled = false;
    }

    if (m_cscDsState->UseSfc() && m_cscDsState->RequireCsc())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->CscUsingSfc(m_avcSeqParam->InputColorSpace));

        return eStatus;
    }

    UpdateSSDSliceCount();

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
    cscScalingKernelParams.bLastTaskInPhaseCSC   = !m_scalingEnabled;
    cscScalingKernelParams.bLastTaskInPhase4xDS  = false;
    cscScalingKernelParams.bLastTaskInPhase16xDS = !(m_32xMeSupported || m_pictureCodingType != I_TYPE);
    cscScalingKernelParams.bLastTaskInPhase32xDS = m_pictureCodingType == I_TYPE;
    cscScalingKernelParams.inputColorSpace       = m_avcSeqParam->InputColorSpace;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));

    if (!m_16xMeSupported)
    {
        // for 4xME VDEnc used its internal 4x scaled surface, no more operation needed
        return eStatus;
    }

    // Static frame detection
    // no need to call for I-frame
    m_staticFrameDetectionInUse = m_staticFrameDetectionEnable && m_hmeEnabled;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ExecuteMeKernel());

    // call SFD kernel after HME in same command buffer
    if (m_staticFrameDetectionInUse)
    {
        // Load VDEnc Costs
        CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadCosts(m_avcPicParam->CodingType,
            m_avcPicParam->QpY + m_avcSliceParams->slice_qp_delta));

        m_vdencHmeMvCostTbl = m_vdEncHmeMvCost;
        m_vdencModeCostTbl  = m_vdEncModeCost;
        m_vdencMvCostTbl    = m_vdEncMvCost;

        m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SFDKernel());
    }

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ME_OUTPUT_PARAMS meOutputParams;

        MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
        meOutputParams.psMeMvBuffer = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me16xMvDataBuffer) : &m_16xMeMvDataBuffer;
        meOutputParams.b16xMeInUse  = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &meOutputParams.psMeMvBuffer->OsResource,
            CodechalDbgAttr::attrOutput,
            "MvData",
            meOutputParams.psMeMvBuffer->dwHeight * meOutputParams.psMeMvBuffer->dwPitch,
            CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
            CODECHAL_MEDIA_STATE_16X_ME));

        meOutputParams.pResVdenStreamInBuffer = &(m_resVdencStreamInBuffer[m_currRecycledBufIdx]);
        meOutputParams.psMeMvBuffer           = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer) : &m_4xMeMvDataBuffer;
        meOutputParams.psMeDistortionBuffer   = m_hmeKernel ? m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer) : &m_4xMeDistortionBuffer;
        meOutputParams.b16xMeInUse            = false;
        meOutputParams.bVdencStreamInInUse    = true;
        if (m_vdencStreamInEnabled) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "MvData",
                m_picWidthInMb * m_picHeightInMb * CODECHAL_CACHELINE_SIZE,
                0,
                CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN));
        }

        if (m_staticFrameDetectionInUse) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resSfdOutputBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "Out",
                SFD_OUTPUT_BUFFER_SIZE,
                0,
                CODECHAL_MEDIA_STATE_STATIC_FRAME_DETECTION));
        }

    );

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER   cmdBuffer,
    bool                  frameTracking,
    MHW_MI_MMIOREGISTERS  *mmioRegister)
{
    // Set flag bIsMdfLoad in remote gaming scenario to boost GPU frequency for low latency
    cmdBuffer->Attributes.bFrequencyBoost = (m_avcSeqParam->ScenarioInfo == ESCENARIO_REMOTEGAMING);
    return CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

MOS_STATUS CodechalVdencAvcState::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_BATCH_BUFFER batchBuffer;
    MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
    batchBuffer.dwOffset     = m_currPass * BRC_IMG_STATE_SIZE_PER_PASS;
    batchBuffer.bSecondLevel = true;

    CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS encodePictureLevelParams;
    MOS_ZeroMemory(&encodePictureLevelParams, sizeof(encodePictureLevelParams));
    encodePictureLevelParams.psPreDeblockSurface  = &m_reconSurface;
    encodePictureLevelParams.psPostDeblockSurface = &m_reconSurface;
    encodePictureLevelParams.bBrcEnabled          = false;
    encodePictureLevelParams.pImgStateBatchBuffer = &batchBuffer;

    bool suppressReconPic =
        ((!m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) && m_suppressReconPicSupported);
    encodePictureLevelParams.bDeblockerStreamOutEnable = 0;
    encodePictureLevelParams.bPreDeblockOutEnable      = !m_deblockingEnabled && !suppressReconPic;
    encodePictureLevelParams.bPostDeblockOutEnable     = m_deblockingEnabled && !suppressReconPic;

    if (!m_staticFrameDetectionInUse)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadCosts(m_avcPicParam->CodingType,
            m_avcPicParam->QpY + m_avcSliceParams->slice_qp_delta));

        m_vdencHmeMvCostTbl = m_vdEncHmeMvCost;
        m_vdencModeCostTbl  = m_vdEncModeCost;
        m_vdencMvCostTbl    = m_vdEncMvCost;
    }

    // VDEnc HuC BRC
    if (m_vdencBrcEnabled)
    {
        PerfTagSetting perfTag;
        perfTag.Value = 0;
        perfTag.Mode  = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
        // STF: HuC+VDEnc+PAK single BB, non-STF: HuC Init/HuC Update/(VDEnc+PAK) in separate BBs
        perfTag.CallType          = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
        perfTag.PictureCodingType = m_pictureCodingType;
        m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

        // Set HuC DMEM buffers which need to be updated.
        // They are first pass of next frame and next pass of current frame, as the 2nd VDEnc+PAK pass may not be triggered.
        uint32_t nextRecycledBufIdx         = (m_currRecycledBufIdx + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        uint32_t nextPass                   = (m_currPass + 1) % CODECHAL_VDENC_BRC_NUM_OF_PASSES;
        m_resVdencBrcUpdateDmemBufferPtr[0] = &m_resVdencBrcUpdateDmemBuffer[nextRecycledBufIdx][0];
        if (m_lastTaskInPhase)
        {
            // last pass of current frame, no next pass
            m_resVdencBrcUpdateDmemBufferPtr[1] = nullptr;
        }
        else
        {
            m_resVdencBrcUpdateDmemBufferPtr[1] =
                &m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][nextPass];
        }

        // Invoke BRC init/reset FW
        if (m_brcInit || m_brcReset)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcInitReset());
        }

        if (!m_singleTaskPhaseSupported)
        {
            perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE;
            m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);
        }

        // Invoke BRC update FW
        CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcUpdate());
        m_brcInit = m_brcReset = false;
    }

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // PAK cmd buffer header insertion for 1) non STF 2) STF (except VDEnc BRC case inserted in HuC cmd buffer)
    if (!m_singleTaskPhaseSupported || (m_firstTaskInPhase && (!m_vdencBrcEnabled)))
    {
        bool requestFrameTracking = false;

        m_hwInterface->m_numRequestedEuSlices = ((m_frameHeight * m_frameWidth) >= m_ssdResolutionThreshold &&
                                                    m_targetUsage <= m_ssdTargetUsageThreshold)
                                                    ? m_sliceShutdownRequestState
                                                    : m_sliceShutdownDefaultState;

        MHW_MI_MMIOREGISTERS mmioRegister;
        bool validMmio = m_mfxInterface->ConvertToMiRegister(m_vdboxIndex, mmioRegister);

        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking, validMmio ? &mmioRegister : nullptr));

        m_hwInterface->m_numRequestedEuSlices = CODECHAL_SLICE_SHUTDOWN_DEFAULT;
    }

    // Set TBL distribution to VMC = 240 for VDEnc performance
    if (MEDIA_IS_WA(m_waTable, WaTlbAllocationForAvcVdenc) &&
        (!m_singleTaskPhaseSupported || !m_currPass))
    {
        TLBAllocationParams tlbAllocationParams;
        tlbAllocationParams.presTlbMmioBuffer     = &m_vdencTlbMmioBuffer;
        tlbAllocationParams.dwMmioMfxLra0Override = m_mmioMfxLra0Override;
        tlbAllocationParams.dwMmioMfxLra1Override = m_mmioMfxLra1Override;
        tlbAllocationParams.dwMmioMfxLra2Override = m_mmioMfxLra2Override;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTLBAllocation(&cmdBuffer, &tlbAllocationParams));
    }

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
    if (m_vdencBrcEnabled)
    {
        // Insert conditional batch buffer end for HuC valid IMEM loaded check
        MOS_ZeroMemory(&miConditionalBatchBufferEndParams, sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));
        miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_resHucStatus2Buffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_miInterface->AddMiConditionalBatchBufferEndCmd(
                &cmdBuffer,
                &miConditionalBatchBufferEndParams));
    }

    if (m_currPass)
    {
        if (m_inlineEncodeStatusUpdate && m_vdencBrcEnabled)
        {
            // inc dwStoreData conditionaly
            UpdateEncodeStatus(&cmdBuffer, false);
        }

        // Insert conditional batch buffer end
        MOS_ZeroMemory(&miConditionalBatchBufferEndParams, sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        if (!m_vdencBrcEnabled)
        {
            miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
            miConditionalBatchBufferEndParams.dwOffset =
                (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
                m_encodeStatusBuf.dwImageStatusMaskOffset + (sizeof(uint32_t) * 2);
        }
        else
        {
            // VDENC uses HuC BRC FW generated semaphore for conditional 2nd pass
            miConditionalBatchBufferEndParams.presSemaphoreBuffer = &m_resPakMmioBuffer;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    if (!m_currPass && m_osInterface->bTagResourceSync)
    {
        // This is a short term solution to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.
        MOS_RESOURCE globalGpuContextSyncTagBuffer;
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        uint32_t                 value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource      = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue          = (value > 0) ? (value - 1) : 0;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencControlStateCmd(&cmdBuffer));

    // set MFX_SURFACE_STATE values
    // Ref surface
    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    MOS_ZeroMemory(&reconSurfaceParams, sizeof(reconSurfaceParams));
    reconSurfaceParams.Mode             = m_mode;
    reconSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_REF_SURFACE_ID;
    reconSurfaceParams.psSurface        = &m_reconSurface;

    // Src surface
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode                  = m_mode;
    surfaceParams.ucSurfaceStateId      = CODECHAL_MFX_SRC_SURFACE_ID;
    surfaceParams.psSurface             = m_rawSurfaceToPak;
    surfaceParams.dwActualHeight        = surfaceParams.psSurface->dwHeight;
    surfaceParams.dwActualWidth         = surfaceParams.psSurface->dwWidth;
    surfaceParams.bDisplayFormatSwizzle = m_avcPicParam->bDisplayFormatSwizzle;
    surfaceParams.bColorSpaceSelection  = (m_avcSeqParam->InputColorSpace == ECOLORSPACE_P709) ? 1 : 0;

    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.pRawSurfParam      = &surfaceParams;
    pipeBufAddrParams.pDecodedReconParam = &reconSurfaceParams;
    SetMfxPipeBufAddrStateParams(encodePictureLevelParams, pipeBufAddrParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    m_mmcState->SetPipeBufAddr(&pipeBufAddrParams, &cmdBuffer);

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = CreateMhwVdboxPipeModeSelectParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(pipeModeSelectParams);

    SetMfxPipeModeSelectParams(encodePictureLevelParams, *pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &reconSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    // 4xDS surface
    MHW_VDBOX_SURFACE_PARAMS dsSurfaceParams;
    MOS_ZeroMemory(&dsSurfaceParams, sizeof(dsSurfaceParams));
    dsSurfaceParams.Mode             = m_mode;
    dsSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_DSRECON_SURFACE_ID;
    dsSurfaceParams.psSurface        = m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &dsSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetMfxIndObjBaseAddrStateParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    SetMfxBspBufBaseAddrStateParams(bspBufBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));
    MOS_Delete(pipeModeSelectParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSrcSurfaceStateCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &reconSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencDsRefSurfaceStateCmd(&cmdBuffer, &dsSurfaceParams, 1));

    // PerfMode is enabled only on BXT, KBL+, replace all 4x Ds refs with the 1st L0 ref
    if (m_vdencInterface->IsPerfModeSupported() && m_perfModeEnabled[m_avcSeqParam->TargetUsage] &&
        pipeBufAddrParams.dwNumRefIdxL0ActiveMinus1 == 0)
    {
        pipeBufAddrParams.dwNumRefIdxL0ActiveMinus1 = 1;
        pipeBufAddrParams.presVdencReferences[1]    = nullptr;
        pipeBufAddrParams.presVdenc4xDsSurface[1]   = pipeBufAddrParams.presVdenc4xDsSurface[0];
    }

    if (m_pictureCodingType == P_TYPE)
    {
        pipeBufAddrParams.presVdencColocatedMVWriteBuffer = &m_vdencColocatedMVBuffer;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        pipeBufAddrParams.presVdencColocatedMVReadBuffer = &m_vdencColocatedMVBuffer;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    MHW_VDBOX_VDENC_CQPT_STATE_PARAMS vdencCQPTStateParams;
    SetVdencCqptStateParams(vdencCQPTStateParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencConstQPStateCmd(&cmdBuffer, &vdencCQPTStateParams));

    if (encodePictureLevelParams.bBrcEnabled && m_avcSeqParam->RateControlMethod != RATECONTROL_ICQ)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            encodePictureLevelParams.pImgStateBatchBuffer));
    }
    else
    {
        //Set MFX_AVC_IMG_STATE command
        PMHW_VDBOX_AVC_IMG_PARAMS imageStateParams = CreateMhwVdboxAvcImgParams();
        CODECHAL_ENCODE_CHK_NULL_RETURN(imageStateParams);
        SetMfxAvcImgStateParams(*imageStateParams);

        PMHW_BATCH_BUFFER secondLevelBatchBufferUsed = nullptr;

        // VDENC CQP case
        if (!m_vdencBrcEnabled)
        {
            // VDENC case uses multiple buffers for concurrency between SFD and VDENC
            secondLevelBatchBufferUsed = &(m_batchBufferForVdencImgStat[m_currRecycledBufIdx]);

            if (!m_staticFrameDetectionInUse)
            {
                // CQP case, driver programs the 2nd Level BB
                CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(m_osInterface, secondLevelBatchBufferUsed));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencAvcCostStateCmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencImgStateCmd(nullptr, secondLevelBatchBufferUsed, imageStateParams));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, secondLevelBatchBufferUsed));

                OVERRIDE_CMD_DATA((uint32_t*)secondLevelBatchBufferUsed->pData, secondLevelBatchBufferUsed->iCurrent / sizeof(uint32_t));

                CODECHAL_DEBUG_TOOL(
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
                        nullptr,
                        secondLevelBatchBufferUsed));

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateEncParam(
                        0,
                        nullptr));

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpEncodeImgStats(nullptr));)

                CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(m_osInterface, secondLevelBatchBufferUsed, true));
            }
            else
            {
                // SFD enabled, SFD kernel updates VDENC IMG STATE
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(&cmdBuffer, nullptr, imageStateParams));
#if (_DEBUG || _RELEASE_INTERNAL)
                secondLevelBatchBufferUsed->iLastCurrent = CODECHAL_ENCODE_VDENC_IMG_STATE_CMD_SIZE + CODECHAL_ENCODE_MI_BATCH_BUFFER_END_CMD_SIZE;
#endif
                CODECHAL_DEBUG_TOOL(
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
                        &cmdBuffer,
                        nullptr));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpEncodeImgStats(&cmdBuffer));)
            }
        }
        else
        {
            secondLevelBatchBufferUsed = &(m_batchBufferForVdencImgStat[0]);
        }
        MOS_Delete(imageStateParams);

        HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext,&secondLevelBatchBufferUsed->OsResource, 0, true, 0);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, secondLevelBatchBufferUsed));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                secondLevelBatchBufferUsed,
                CODECHAL_MEDIA_STATE_ENC_NORMAL,
                nullptr));)
    }

    MHW_VDBOX_QM_PARAMS qmParams;
    MHW_VDBOX_QM_PARAMS fqmParams;
    SetMfxQmStateParams(qmParams, fqmParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(&cmdBuffer, &qmParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxFqmCmd(&cmdBuffer, &fqmParams));

    if (m_pictureCodingType == B_TYPE)
    {
        // Add AVC Direct Mode command
        MHW_VDBOX_AVC_DIRECTMODE_PARAMS directmodeParams;
        MOS_ZeroMemory(&directmodeParams, sizeof(directmodeParams));
        directmodeParams.CurrPic = m_avcPicParam->CurrReconstructedPic;
        directmodeParams.isEncode = true;
        directmodeParams.uiUsedForReferenceFlags = 0xFFFFFFFF;
        directmodeParams.pAvcPicIdx = &(m_picIdx[0]);
        directmodeParams.avcRefList = (void**)m_refList;
        directmodeParams.bPicIdRemappingInUse = false;
        directmodeParams.bDisableDmvBuffers = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcDirectmodeCmd(&cmdBuffer, &directmodeParams));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto cpInterface  = m_hwInterface->GetCpInterface();
    auto avcSlcParams = m_avcSliceParams;
    auto avcPicParams = m_avcPicParams[avcSlcParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];
    auto slcData      = m_slcData;

    // For use with the single task phase implementation
    if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
    {
        uint32_t numSlc = (m_frameFieldHeightInMb + m_sliceHeight - 1) / m_sliceHeight;

        if (numSlc != m_numSlices)
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    bool useBatchBufferForPakSlices = false;
    if (m_singleTaskPhaseSupported && m_singleTaskPhaseSupportedInPak)
    {
        if (m_currPass == 0)
        {
            // The same buffer is used for all slices for all passes.
            uint32_t batchBufferForPakSlicesSize =
                (m_numPasses + 1) * m_numSlices * m_pakSliceSize;
            if (batchBufferForPakSlicesSize >
                (uint32_t)m_batchBufferForPakSlices[m_currRecycledBufIdx].iSize)
            {
                if (m_batchBufferForPakSlices[m_currRecycledBufIdx].iSize)
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReleaseBatchBufferForPakSlices(m_currRecycledBufIdx));
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBatchBufferForPakSlices(
                    m_numSlices,
                    m_numPasses,
                    m_currRecycledBufIdx));
            }
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_LockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currRecycledBufIdx]));
        useBatchBufferForPakSlices = true;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_osInterface->osCpInterface->IsCpEnabled())
    {
        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.bLastPass = (m_currPass == m_numPasses) ? true : false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->SetMfxProtectionState(false, &cmdBuffer, nullptr, &sliceInfoParam));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->UpdateParams(false));
    }

    avcSlcParams = m_avcSliceParams;

    CODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS packSlcHeaderParams;
    packSlcHeaderParams.pBsBuffer          = &m_bsBuffer;
    packSlcHeaderParams.pPicParams         = avcPicParams;
    packSlcHeaderParams.pSeqParams         = m_avcSeqParam;
    packSlcHeaderParams.ppRefList          = &(m_refList[0]);
    packSlcHeaderParams.CurrPic            = m_currOriginalPic;
    packSlcHeaderParams.CurrReconPic       = m_currReconstructedPic;
    packSlcHeaderParams.UserFlags          = m_userFlags;
    packSlcHeaderParams.NalUnitType        = m_nalUnitType;
    packSlcHeaderParams.wPictureCodingType = m_pictureCodingType;
    packSlcHeaderParams.bVdencEnabled      = true;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.presDataBuffer      = &m_resMbCodeSurface;
    sliceState.pAvcPicIdx          = &(m_picIdx[0]);
    sliceState.pEncodeAvcSeqParams = m_avcSeqParam;
    sliceState.pEncodeAvcPicParams = avcPicParams;
    sliceState.pBsBuffer           = &m_bsBuffer;
    sliceState.ppNalUnitParams     = m_nalUnitParams;
    sliceState.bBrcEnabled         = false;
    // Disable Panic mode when min/max QP control is on. kernel may disable it, but disable in driver also.
    sliceState.bRCPanicEnable                = m_panicEnable && (!m_minMaxQpControlEnabled);
    sliceState.bAcceleratorHeaderPackingCaps = m_encodeParams.bAcceleratorHeaderPackingCaps;
    sliceState.wFrameFieldHeightInMB         = m_frameFieldHeightInMb;

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        if (m_currPass == 0)
        {
            packSlcHeaderParams.pAvcSliceParams = &avcSlcParams[slcCount];
            if (m_acceleratorHeaderPackingCaps)
            {
                slcData[slcCount].SliceOffset = m_bsBuffer.SliceOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_PackSliceHeader(&packSlcHeaderParams));
                slcData[slcCount].BitSize = m_bsBuffer.BitSize;
            }
            if (m_sliceStructCaps != CODECHAL_SLICE_STRUCT_ARBITRARYMBSLICE)
            {
                slcData[slcCount].CmdOffset = slcCount * m_sliceHeight * m_picWidthInMb * 16 * 4;
            }
            else
            {
                slcData[slcCount].CmdOffset = packSlcHeaderParams.pAvcSliceParams->first_mb_in_slice * 16 * 4;
            }
        }

        sliceState.pEncodeAvcSliceParams = &avcSlcParams[slcCount];
        sliceState.dwDataBufferOffset =
            m_slcData[slcCount].CmdOffset + m_mbcodeBottomFieldOffset;
        sliceState.dwOffset                  = slcData[slcCount].SliceOffset;
        sliceState.dwLength                  = slcData[slcCount].BitSize;
        sliceState.uiSkipEmulationCheckCount = slcData[slcCount].SkipEmulationByteCount;
        sliceState.dwSliceIndex              = (uint32_t)slcCount;
        sliceState.bFirstPass                = (m_currPass == 0);
        sliceState.bLastPass                 = (m_currPass == m_numPasses);
        sliceState.bInsertBeforeSliceHeaders = (slcCount == 0);
        sliceState.bVdencInUse               = true;
        // App handles tail insertion for VDEnc dynamic slice in non-cp case
        sliceState.bVdencNoTailInsertion = m_vdencNoTailInsertion;

        uint32_t batchBufferForPakSlicesStartOffset =
            (uint32_t)m_batchBufferForPakSlices[m_currRecycledBufIdx].iCurrent;

        if (useBatchBufferForPakSlices)
        {
            sliceState.pBatchBufferForPakSlices =
                &m_batchBufferForPakSlices[m_currRecycledBufIdx];
            sliceState.bSingleTaskPhaseSupported            = true;
            sliceState.dwBatchBufferForPakSlicesStartOffset = batchBufferForPakSlicesStartOffset;
        }

        if (m_avcRoundingParams != nullptr && m_avcRoundingParams->bEnableCustomRoudingIntra)
        {
            sliceState.dwRoundingIntraValue = m_avcRoundingParams->dwRoundingIntra;
        }
        else
        {
            sliceState.dwRoundingIntraValue = 5;
        }
        if (m_avcRoundingParams != nullptr && m_avcRoundingParams->bEnableCustomRoudingInter)
        {
            sliceState.bRoundingInterEnable = true;
            sliceState.dwRoundingValue      = m_avcRoundingParams->dwRoundingInter;
        }
        else
        {
            sliceState.bRoundingInterEnable = m_roundingInterEnable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));
        }

        sliceState.oneOnOneMapping = m_oneOnOneMapping;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSlice(&cmdBuffer, &sliceState));

        // Add dumps for 2nd level batch buffer
        if (sliceState.bSingleTaskPhaseSupported && !sliceState.bVdencInUse)
        {
            CODECHAL_ENCODE_CHK_NULL_RETURN(sliceState.pBatchBufferForPakSlices);

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->Dump2ndLvlBatch(
                    sliceState.pBatchBufferForPakSlices,
                    CODECHAL_MEDIA_STATE_ENC_NORMAL,
                    nullptr));)
        }

        // For SKL, only the 1st slice state should be programmed for VDENC
        if (!m_hwInterface->m_isVdencSuperSliceEnabled)
        {
            break;
        }
        else  // For CNL slice state is programmed per Super slice
        {
            MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
            // MfxPipeDone should be set for all super slices except the last super slice and should not be set for tail insertion.
            vdPipelineFlushParams.Flags.bWaitDoneMFX =
                (slcCount == (m_numSlices)-1) ? ((m_lastPicInStream || m_lastPicInSeq) ? 0 : 1) : 1;
            vdPipelineFlushParams.Flags.bWaitDoneVDENC          = 1;
            vdPipelineFlushParams.Flags.bFlushVDENC             = 1;
            vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

            //Do not send MI_FLUSH for last Super slice now
            if (slcCount != ((m_numSlices)-1))
            {
                // Send MI_FLUSH for every Super slice
                MHW_MI_FLUSH_DW_PARAMS flushDwParams;
                MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
                    &cmdBuffer,
                    &flushDwParams));
            }
        }
    }

    if (useBatchBufferForPakSlices)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_UnlockBb(
            m_osInterface,
            &m_batchBufferForPakSlices[m_currRecycledBufIdx],
            m_lastTaskInPhase));
    }

    //Send VDENC WALKER cmd per every frame for SKL
    if (!m_hwInterface->m_isVdencSuperSliceEnabled)
    {
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams = CreateMhwVdboxVdencWalkerStateParams();
        CODECHAL_ENCODE_CHK_NULL_RETURN(vdencWalkerStateParams);
        vdencWalkerStateParams->Mode          = CODECHAL_ENCODE_MODE_AVC;
        vdencWalkerStateParams->pAvcSeqParams = avcSeqParams;
        vdencWalkerStateParams->pAvcSlcParams = avcSlcParams;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(&cmdBuffer, vdencWalkerStateParams));
        MOS_Delete(vdencWalkerStateParams);

        MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
        // MFXPipeDone should not be set for tail insertion
        vdPipelineFlushParams.Flags.bWaitDoneMFX =
            (m_lastPicInStream || m_lastPicInSeq) ? 0 : 1;
        vdPipelineFlushParams.Flags.bWaitDoneVDENC          = 1;
        vdPipelineFlushParams.Flags.bFlushVDENC             = 1;
        vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));
    }

    // Insert end of sequence/stream if set
    if (m_lastPicInStream || m_lastPicInSeq)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq    = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        pakInsertObjectParams.dwBitSize        = 32;  // use dwBitSize for SrcDataEndingBitInclusion
        if (m_lastPicInSeq)
        {
            pakInsertObjectParams.dwLastPicInSeqData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSEQ << 24);
        }
        if (m_lastPicInStream)
        {
            pakInsertObjectParams.dwLastPicInStreamData = (uint32_t)((1 << 16) | CODECHAL_ENCODE_AVC_NAL_UT_EOSTREAM << 24);
        }
        pakInsertObjectParams.bHeaderLengthExcludeFrmSize = true;
        if (pakInsertObjectParams.bEmulationByteBitsInsert)
        {
            //Does not matter here, but keeping for consistency
            CODECHAL_ENCODE_ASSERTMESSAGE("The emulation prevention bytes are not inserted by the app and are requested to be inserted by HW.");
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr, &pakInsertObjectParams));
    }

    if (m_hwInterface->m_isVdencSuperSliceEnabled)
    {
        // Send MI_FLUSH with bVideoPipelineCacheInvalidate set to true for last Super slice
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(
            &cmdBuffer,
            &flushDwParams));
    }

    // On-demand sync for VDEnc StreamIn surface and CSC surface
    if (m_cscDsState && m_currPass == 0)
    {
        if (m_cscDsState->RequireCsc())
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->WaitCscSurface(m_videoContext, true));
        }

        if (m_16xMeSupported)
        {
            auto syncParams             = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContext;
            syncParams.bReadOnly        = true;
            syncParams.presSyncResource = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
            m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(&cmdBuffer));

    if (m_vdencBrcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreNumPasses(
            &(m_encodeStatusBuf),
            m_miInterface,
            &cmdBuffer,
            m_currPass));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pak_pass = "PAK_PASS" + std::to_string(static_cast<uint32_t>(m_currPass));
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pak_pass.data()));

        //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
        //    m_debugInterface,
        //    &cmdBuffer));
    )

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    bool renderingFlags = m_videoContextUsesNullHw;

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        // Restore TLB allocation
        if (MEDIA_IS_WA(m_waTable, WaTlbAllocationForAvcVdenc))
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(RestoreTLBAllocation(&cmdBuffer, &m_vdencTlbMmioBuffer));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER),
                CodechalDbgAttr::attrReconstructedSurface,
                "4XScaling")) 
                for (int i = 0; i < (avcPicParams->num_ref_idx_l0_active_minus1 + 1); i++)
                {
                    std::string refSurfName = "4XScaling_RefL0[" + std::to_string(static_cast<uint32_t>(i)) + "]";
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        (m_trackedBuf->Get4xDsReconSurface(m_refList[m_avcSliceParams->RefPicList[0][i].FrameIdx]->ucScalingIdx)),
                        CodechalDbgAttr::attrReconstructedSurface,
                        refSurfName.c_str()))
                }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        &m_reconSurface,
                        CodechalDbgAttr::attrReconstructedSurface,
                        "ReconSurf")))

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            })

        if (m_sliceSizeStreamoutSupported)
        {
            CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_pakSliceSizeStreamoutBuffer,
                CodechalDbgAttr::attrOutput,
                "SliceSizeStreamout",
                CODECHAL_ENCODE_SLICESIZE_BUF_SIZE,
                0,
                CODECHAL_NUM_MEDIA_STATES)));
        }

        if ((m_currPass == m_numPasses) &&
            m_signalEnc &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // Check if the signal obj count exceeds max value
            if (m_semaphoreObjCount == MOS_MIN(m_semaphoreMaxCount, MOS_MAX_OBJECT_SIGNALED))
            {
                auto syncParams             = g_cInitSyncParams;
                syncParams.GpuContext       = m_renderContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
                m_semaphoreObjCount--;
            }

            // signal semaphore
            auto syncParams             = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_semaphoreObjCount++;
        }
    }

    if (m_vdencBrcEnabled)
    {
        CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(false));
        CODECHAL_DEBUG_TOOL(DumpEncodeImgStats(nullptr));
    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
    }
  
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateSliceStateParam(
            m_adaptiveRoundingInterEnable,
            &sliceState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpFrameParFile());)

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams = CreateMhwVdboxVdencWalkerStateParams();
    CODECHAL_ENCODE_CHK_NULL_RETURN(vdencWalkerStateParams);
    auto avcSlcParams = m_avcSliceParams;
    auto avcPicParams = m_avcPicParams[avcSlcParams->pic_parameter_set_id];
    auto avcSeqParams = m_avcSeqParams[avcPicParams->seq_parameter_set_id];

    vdencWalkerStateParams->Mode          = CODECHAL_ENCODE_MODE_AVC;
    vdencWalkerStateParams->pAvcSeqParams = avcSeqParams;
    vdencWalkerStateParams->pAvcSlcParams = m_avcSliceParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, vdencWalkerStateParams));

    MOS_Delete(vdencWalkerStateParams);
    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcBase::UserFeatureKeyReport());

    // AVC HME Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_ME_IN_USE_ID, m_hmeSupported);

    // AVC SuperHME Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_16xME_IN_USE_ID, m_16xMeSupported);

    // AVC UltraHME Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_32xME_IN_USE_ID, m_32xMeSupported);

    // AVC RateControl Method Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_RATECONTROL_METHOD_ID, m_avcSeqParam->RateControlMethod);

    // Static frame detection Enable Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_STATIC_FRAME_DETECTION_ENABLE_ID, m_staticFrameDetectionEnable);

    // AVC FTQ Reporting
#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_AVC_FTQ_IN_USE_ID, m_ftqEnable);

    // VDENC Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VDENC_IN_USE_ID, true);
#endif  // _DEBUG || _RELEASE_INTERNAL

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeAvcBase::AllocateResources();

    // Allocate SEI buffer
    m_seiData.pSEIBuffer = (uint8_t *)MOS_AllocAndZeroMemory(CODECHAL_ENCODE_AVC_SEI_BUFFER_SIZE);
    if (m_seiData.pSEIBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SEI Buffer.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }
    m_seiData.dwSEIBufSize = CODECHAL_ENCODE_AVC_SEI_BUFFER_SIZE;

    // initiate allocation parameters and lock flags
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    allocParamsForBuffer2D.Format   = Format_Buffer_2D;

    // initiate allocation parameters
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferNV12;
    MOS_ZeroMemory(&allocParamsForBufferNV12, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferNV12.Type     = MOS_GFXRES_2D;
    allocParamsForBufferNV12.TileType = MOS_TILE_Y;
    allocParamsForBufferNV12.Format   = Format_NV12;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    if (m_pakEnabled)
    {
        // Allocate skip frame copy buffer
        allocParamsForBufferLinear.dwBytes = m_skipFrameBufferSize = CODECHAL_PAGE_SIZE;
        allocParamsForBufferLinear.pBufName                        = "Skip Frame Copy Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &m_resSkipFrameBuffer),
            "Failed to allocate Skip Frame Copy Buffer\n");
    }

    if (m_staticFrameDetectionEnable)
    {
        // SFD output buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "Static frame detection output buffer";

        // VDENC case needs a set of buffers for concurrency between SFD and HuC BRC FW
        for (uint32_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                          m_osInterface,
                                                          &allocParamsForBufferLinear,
                                                          &m_resSfdOutputBuffer[i]),
                "Failed to allocate static frame detection output buffer\n");
        }

        // SFD P-frame cost table buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "SFD P-frame cost table buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &m_resSfdCostTablePFrameBuffer),
            "Failed to allocate SFD P-frame cost table buffer\n");

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &m_resSfdCostTableBFrameBuffer),
            "Failed to allocate SFD B-frame cost table buffer\n");

        // copy SFD P-frame cost table
        uint8_t *data;
        if (nullptr == (data = (uint8_t *)m_osInterface->pfnLockResource(
                            m_osInterface,
                            &m_resSfdCostTablePFrameBuffer,
                            &lockFlagsWriteOnly)))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock SFD P-frame cost table Buffer.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(data,
                                                      CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE * sizeof(uint8_t),
                                                      m_codechalEncodeAvcSfdCostTablePFrame,
                                                      CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE * sizeof(uint8_t)),
            "Failed to copy SFD P-frame cost table");
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resSfdCostTablePFrameBuffer);

        // copy SFD B-frame cost table
        if (nullptr == (data = (uint8_t *)m_osInterface->pfnLockResource(
                            m_osInterface,
                            &m_resSfdCostTableBFrameBuffer,
                            &lockFlagsWriteOnly)))
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock SFD B-frame cost table Buffer.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(data,
                                                      CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE * sizeof(uint8_t),
                                                      m_codechalEncodeAvcSfdCostTableBFrame,
                                                      CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE * sizeof(uint8_t)),
            "Failed to copy SFD B-frame cost table");
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resSfdCostTableBFrameBuffer);
    }

    // VDENC BRC buffer allocation
    for (uint32_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        // BRC update DMEM
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
        for (uint32_t j = 0; j < CODECHAL_VDENC_BRC_NUM_OF_PASSES; j++)
        {
            eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resVdencBrcUpdateDmemBuffer[i][j]);
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC Update DMEM Buffer\n", __FUNCTION__);
                return eStatus;
            }

            uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
                m_osInterface,
                &(m_resVdencBrcUpdateDmemBuffer[i][j]),
                &lockFlagsWriteOnly);

            if (data == nullptr)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock VDEnc BRC Update DMEM Buffer.");
                eStatus = MOS_STATUS_UNKNOWN;
                return eStatus;
            }

            MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcUpdateDmemBuffer[i][j]);
        }

        // BRC init/reset DMEM
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BrcInit DmemBuffer";
        eStatus                             = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resVdencBrcInitDmemBuffer[i]);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC Init DMEM Buffer\n", __FUNCTION__);
            return eStatus;
        }

        // VDENC IMG STATE read buffer
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC BRC IMG State Read Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resVdencBrcImageStatesReadBuffer[i]);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC IMG State Read Buffer\n", __FUNCTION__);
            return eStatus;
        }
    }

    // Const Data buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(GetBRCCostantDataSize(), CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resVdencBrcConstDataBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC Const Data Buffer\n", __FUNCTION__);
        return eStatus;
    }

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
        m_osInterface,
        &(m_resVdencBrcConstDataBuffer),
        &lockFlagsWriteOnly);

    if (data == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock VDEnc VDENC BRC Const Data Buffer.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
    m_osInterface->pfnUnlockResource(m_osInterface, &m_resVdencBrcConstDataBuffer);

    // BRC history buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_VDENC_AVC_BRC_HISTORY_BUF_SIZE, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC History Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resVdencBrcHistoryBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC History Buffer\n", __FUNCTION__);
        return eStatus;
    }

    if (!m_vdencBrcEnabled && m_staticFrameDetectionEnable)
    {
        // SFD VDENC IMG STATE input buffer (CQP mode)
        allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_hwInterface->m_vdencBrcImgStateBufferSize, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "VDENC IMG SFD input Buffer";

        eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resVdencSfdImageStateReadBuffer);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC IMG SFD input Buffer\n", __FUNCTION__);
            return eStatus;
        }
    }

    // Debug buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(CODECHAL_VDENC_AVC_BRC_DEBUG_BUF_SIZE, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Debug Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resVdencBrcDbgBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC Debug Buffer\n", __FUNCTION__);
        return eStatus;
    }

    // VDENC Intra Row Store Scratch buffer
    // 1 cacheline per MB
    allocParamsForBufferLinear.dwBytes  = m_picWidthInMb * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "VDENC Intra Row Store Scratch Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencIntraRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate VDENC Intra Row Store Scratch Buffer.");
        return eStatus;
    }

    //VDEnc colocated MV buffer
    m_vdencColocatedMVBufferSize = (m_picWidthInMb * m_picHeightInMb) * (CODECHAL_CACHELINE_SIZE / 2);
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencColocatedMVBufferSize, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC Colocated MV buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencColocatedMVBuffer);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate VDENC Colocated MV buffer.");
        return eStatus;
    }

    // VDENC Statistics buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcStatsBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC Statistics Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencStatsBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate BRC VDENC Statistics Buffer\n", __FUNCTION__);
        return eStatus;
    }

    // PAK Statistics buffer
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(m_vdencBrcPakStatsBufferSize, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC BRC PAK Statistics Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_pakStatsBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC BRC PAK Statistics Buffer\n", __FUNCTION__);
        return eStatus;
    }

    // VDENC uses second level batch buffer for image state cmds
    if (!m_vdencBrcEnabled)
    {
        // CQP mode needs a set of buffers for concurrency between SFD and VDEnc
        for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            MOS_ZeroMemory(
                &m_batchBufferForVdencImgStat[i],
                sizeof(m_batchBufferForVdencImgStat[i]));
            m_batchBufferForVdencImgStat[i].bSecondLevel = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_osInterface,
                &m_batchBufferForVdencImgStat[i],
                nullptr,
                m_hwInterface->m_vdencBrcImgStateBufferSize));
        }
        m_vdencBrcImgStatAllocated = true;
    }
    else
    {
        MOS_ZeroMemory(
            &m_batchBufferForVdencImgStat[0],
            sizeof(m_batchBufferForVdencImgStat[0]));
        m_batchBufferForVdencImgStat[0].bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_batchBufferForVdencImgStat[0],
            nullptr,
            m_hwInterface->m_vdencBrcImgStateBufferSize));
    }

    // Buffer to store VDEnc TLB MMIO values (registers MFX_LRA_0/1/2)
    allocParamsForBufferLinear.dwBytes  = MOS_ALIGN_CEIL(3 * sizeof(uint32_t), CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDENC TLB MMIO Buffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencTlbMmioBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("%s: Failed to allocate VDENC TLB MMIO Buffer\n", __FUNCTION__);
        return eStatus;
    }
    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::Initialize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcBase::Initialize());

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::InitKernelStateMe()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t *kernelBinary;
    uint32_t kernelSize;

    uint32_t   kuid   = m_useCommonKernel ? m_kuidCommon : m_kuid;
    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    for (auto krnStateIdx = 0; krnStateIdx < 2; krnStateIdx++)
    {
        CODECHAL_KERNEL_HEADER currKrnHeader;
        auto                   kernelStatePtr = &m_meKernelStates[krnStateIdx];

        auto EncOperation = (krnStateIdx > 0) ? VDENC_ME : ENC_ME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            kernelBinary,
            EncOperation,
            (EncOperation == VDENC_ME) ? 0 : krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount     = CODECHAL_ENCODE_AVC_ME_NUM_SURFACES;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_ME_CURBE);
        kernelStatePtr->KernelParams.iBlockWidth  = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount     = 1;

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

        if (m_noMeKernelForPFrame)
        {
            m_meKernelStates[1] = m_meKernelStates[0];
            break;
        }
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable                    = &m_meBindingTable;
    bindingTable->dwMEMVDataSurface      = CODECHAL_ENCODE_AVC_ME_MV_DATA_SURFACE;
    bindingTable->dw16xMEMVDataSurface   = CODECHAL_ENCODE_AVC_16xME_MV_DATA_SURFACE;
    bindingTable->dw32xMEMVDataSurface   = CODECHAL_ENCODE_AVC_32xME_MV_DATA_SURFACE;
    bindingTable->dwMEDist               = CODECHAL_ENCODE_AVC_ME_DISTORTION_SURFACE;
    bindingTable->dwMEBRCDist            = CODECHAL_ENCODE_AVC_ME_BRC_DISTORTION;
    bindingTable->dwMECurrForFwdRef      = CODECHAL_ENCODE_AVC_ME_CURR_FOR_FWD_REF;
    bindingTable->dwMEFwdRefPicIdx[0]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX0;
    bindingTable->dwMEFwdRefPicIdx[1]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX1;
    bindingTable->dwMEFwdRefPicIdx[2]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX2;
    bindingTable->dwMEFwdRefPicIdx[3]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX3;
    bindingTable->dwMEFwdRefPicIdx[4]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX4;
    bindingTable->dwMEFwdRefPicIdx[5]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX5;
    bindingTable->dwMEFwdRefPicIdx[6]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX6;
    bindingTable->dwMEFwdRefPicIdx[7]    = CODECHAL_ENCODE_AVC_ME_FWD_REF_IDX7;
    bindingTable->dwMECurrForBwdRef      = CODECHAL_ENCODE_AVC_ME_CURR_FOR_BWD_REF;
    bindingTable->dwMEBwdRefPicIdx[0]    = CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX0;
    bindingTable->dwMEBwdRefPicIdx[1]    = CODECHAL_ENCODE_AVC_ME_BWD_REF_IDX1;
    bindingTable->dwVdencStreamInSurface = CODECHAL_ENCODE_AVC_ME_VDENC_STREAMIN;

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetCurbeMe(MeCurbeParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_ASSERT(m_avcSeqParam->TargetUsage <= NUM_TARGET_USAGE_MODES);

    auto slcParams    = m_avcSliceParams;
    auto framePicture = CodecHal_PictureIsFrame(m_avcPicParam->CurrOriginalPic);
    auto qpPrimeY =
        (m_avcPicParam->pic_init_qp_minus26 + 26) +
        m_avcSliceParams->slice_qp_delta;

    auto     mvShiftFactor       = 0;
    auto     prevMvReadPosFactor = 0;
    bool     useMvFromPrevStep, writeDistortions;
    uint32_t scaleFactor;

    switch (params->hmeLvl)
    {
    case HME_LEVEL_32x:
        useMvFromPrevStep = CODECHAL_ENCODE_AVC_HME_FIRST_STEP;
        writeDistortions  = false;
        scaleFactor       = SCALE_FACTOR_32x;
        mvShiftFactor     = CODECHAL_ENCODE_AVC_MV_SHIFT_FACTOR_32x;
        break;
    case HME_LEVEL_16x:
        useMvFromPrevStep   = (m_32xMeEnabled) ? CODECHAL_ENCODE_AVC_HME_FOLLOWING_STEP : CODECHAL_ENCODE_AVC_HME_FIRST_STEP;
        writeDistortions    = false;
        scaleFactor         = SCALE_FACTOR_16x;
        mvShiftFactor       = CODECHAL_ENCODE_AVC_MV_SHIFT_FACTOR_16x;
        prevMvReadPosFactor = CODECHAL_ENCODE_AVC_PREV_MV_READ_POSITION_16x;
        break;
    case HME_LEVEL_4x:
        useMvFromPrevStep   = (m_16xMeEnabled) ? CODECHAL_ENCODE_AVC_HME_FOLLOWING_STEP : CODECHAL_ENCODE_AVC_HME_FIRST_STEP;
        writeDistortions    = true;
        scaleFactor         = SCALE_FACTOR_4x;
        mvShiftFactor       = CODECHAL_ENCODE_AVC_MV_SHIFT_FACTOR_4x;
        prevMvReadPosFactor = CODECHAL_ENCODE_AVC_PREV_MV_READ_POSITION_4x;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
        break;
    }

    CODECHAL_ENCODE_AVC_ME_CURBE cmd;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &cmd,
        sizeof(CODECHAL_ENCODE_AVC_ME_CURBE),
        g_cInit_CODECHAL_ENCODE_AVC_ME_CURBE,
        sizeof(CODECHAL_ENCODE_AVC_ME_CURBE)));

    cmd.DW3.SubPelMode = 3;
    if (m_fieldScalingOutputInterleaved)
    {
        cmd.DW3.SrcAccess =
            cmd.DW3.RefAccess    = CodecHal_PictureIsField(m_avcPicParam->CurrOriginalPic) ? 1 : 0;
        cmd.DW7.SrcFieldPolarity = CodecHal_PictureIsBottomField(m_avcPicParam->CurrOriginalPic) ? 1 : 0;
    }

    cmd.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    cmd.DW4.PictureWidth        = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    cmd.DW5.QpPrimeY            = qpPrimeY;
    cmd.DW6.WriteDistortions    = writeDistortions;
    cmd.DW6.UseMvFromPrevStep   = useMvFromPrevStep;

    cmd.DW6.SuperCombineDist = m_superCombineDistGeneric[m_avcSeqParam->TargetUsage];
    cmd.DW6.MaxVmvR          = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level) >> 1) * 4;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        // set it to 32
        cmd.DW1.BiWeight = 32;
        cmd.DW13.NumRefIdxL1MinusOne =
            m_avcSliceParams->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE ||
        m_pictureCodingType == B_TYPE)
    {
        if (m_16xMeSupported)
        {
            cmd.DW30.ActualMBHeight = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);
            cmd.DW30.ActualMBWidth  = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth);
        }
        cmd.DW13.NumRefIdxL0MinusOne =
            m_avcSliceParams->num_ref_idx_l0_active_minus1;
    }

    cmd.DW13.RefStreaminCost = 5;
    // This flag is to indicate the ROI source type instead of indicating ROI is enabled or not
    cmd.DW13.ROIEnable = 0;

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.DW14.List0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW14.List0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.DW14.List0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.DW14.List0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.DW14.List0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.DW14.List0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.DW14.List0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.DW14.List0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.DW14.List1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.DW14.List1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    cmd.DW15.MvShiftFactor       = mvShiftFactor;
    cmd.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    // r3 & r4
    uint8_t tu         = m_avcSeqParam->TargetUsage;
    uint8_t ucMeMethod = m_meMethodTable ?  // use the ME table dependent on prototype or codec standard
                             m_meMethodTable[tu]
                                         : m_meMethodGeneric[tu];
    uint8_t tableIdx = 0;
    eStatus          = MOS_SecureMemcpy(&(cmd.SPDelta), 14 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][ucMeMethod], 14 * sizeof(uint32_t));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // r5
    cmd.DW32._4xMeMvOutputDataSurfIndex      = CODECHAL_ENCODE_AVC_ME_MV_DATA_SURFACE;
    cmd.DW33._16xOr32xMeMvInputDataSurfIndex = (params->hmeLvl == HME_LEVEL_32x) ? CODECHAL_ENCODE_AVC_32xME_MV_DATA_SURFACE : CODECHAL_ENCODE_AVC_16xME_MV_DATA_SURFACE;
    cmd.DW34._4xMeOutputDistSurfIndex        = CODECHAL_ENCODE_AVC_ME_DISTORTION_SURFACE;
    cmd.DW35._4xMeOutputBrcDistSurfIndex     = CODECHAL_ENCODE_AVC_ME_BRC_DISTORTION;
    cmd.DW36.VMEFwdInterPredictionSurfIndex  = CODECHAL_ENCODE_AVC_ME_CURR_FOR_FWD_REF;
    cmd.DW37.VMEBwdInterPredictionSurfIndex  = CODECHAL_ENCODE_AVC_ME_CURR_FOR_BWD_REF;
    cmd.DW38.VDEncStreamInSurfIndex          = CODECHAL_ENCODE_AVC_ME_VDENC_STREAMIN;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateHmeParam(m_16xMeEnabled, m_32xMeEnabled, ucMeMethod, &cmd));)

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SendMeSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, MeSurfaceParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeMvDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeDistortionBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psMeVdencStreamInBuffer);

    CODECHAL_MEDIA_STATE_TYPE encMediaStateType;
    encMediaStateType = (params->b32xMeInUse) ? CODECHAL_MEDIA_STATE_32X_ME : params->b16xMeInUse ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    if (params->bVdencStreamInEnabled && encMediaStateType == CODECHAL_MEDIA_STATE_4X_ME)
    {
        encMediaStateType = CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN;
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pMeBindingTable);
    auto meBindingTable = params->pMeBindingTable;

    auto    currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    auto    currBottomField  = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    uint8_t ucCurrVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);

    PMOS_SURFACE currScaledSurface = nullptr, meMvDataBuffer = nullptr;
    uint32_t     meMvBottomFieldOffset = 0, currScaledBottomFieldOffset = 0, refScaledBottomFieldOffset = 0;
    if (params->b32xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps32xMeMvDataBuffer);
        currScaledSurface           = m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer              = params->ps32xMeMvDataBuffer;
        meMvBottomFieldOffset       = params->dw32xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw32xScaledBottomFieldOffset;
    }
    else if (params->b16xMeInUse)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps16xMeMvDataBuffer);
        currScaledSurface           = m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer              = params->ps16xMeMvDataBuffer;
        meMvBottomFieldOffset       = params->dw16xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw16xScaledBottomFieldOffset;
    }
    else
    {
        currScaledSurface           = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
        meMvDataBuffer              = params->ps4xMeMvDataBuffer;
        meMvBottomFieldOffset       = params->dw4xMeMvBottomFieldOffset;
        currScaledBottomFieldOffset = params->dw4xScaledBottomFieldOffset;
    }

    // Reference height and width information should be taken from the current scaled surface rather
    // than from the reference scaled surface in the case of PAFF.
    auto refScaledSurface = *currScaledSurface;

    auto width  = MOS_ALIGN_CEIL(params->dwDownscaledWidthInMb * 32, 64);
    auto height = params->dwDownscaledHeightInMb * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER;

    // Force the values
    meMvDataBuffer->dwWidth  = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch  = width;

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface          = true;
    surfaceParams.bMediaBlockRW         = true;
    surfaceParams.psSurface             = meMvDataBuffer;
    surfaceParams.dwOffset              = meMvBottomFieldOffset;
    surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEMVDataSurface;
    surfaceParams.bIsWritable           = true;
    surfaceParams.bRenderTarget         = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    if (params->b16xMeInUse && params->b32xMeEnabled)
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface  = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface     = params->ps32xMeMvDataBuffer;
        surfaceParams.dwOffset =
            currBottomField ? params->dw32xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dw32xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }
    else if (!params->b32xMeInUse && params->b16xMeEnabled)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface  = true;
        surfaceParams.bMediaBlockRW = true;
        surfaceParams.psSurface     = params->ps16xMeMvDataBuffer;
        surfaceParams.dwOffset =
            currBottomField ? params->dw16xMeMvBottomFieldOffset : 0;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dw16xMEMVDataSurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    // Insert Distortion buffers only for 4xMe case
    if (!params->b32xMeInUse && !params->b16xMeInUse)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.bIs2DSurface          = true;
        surfaceParams.bMediaBlockRW         = true;
        surfaceParams.psSurface             = params->psMeDistortionBuffer;
        surfaceParams.dwOffset              = params->dwMeDistortionBottomFieldOffset;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEDist;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable           = true;
        surfaceParams.bRenderTarget         = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    // Setup references 1...n
    // LIST 0 references
    uint8_t       refPicIdx;
    CODEC_PICTURE refPic;
    bool          refFieldPicture, refBottomField;
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
    {
        refPic = params->pL0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState          = true;
                surfaceParams.psSurface             = currScaledSurface;
                surfaceParams.dwOffset              = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset  = meBindingTable->dwMECurrForFwdRef;
                surfaceParams.ucVDirection          = ucCurrVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }

            refFieldPicture   = CodecHal_PictureIsField(refPic) ? 1 : 0;
            refBottomField    = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            refPicIdx         = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;
            if (params->b32xMeInUse)
            {
                MOS_SURFACE *p32xSurface = m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
            }
            else if (params->b16xMeInUse)
            {
                MOS_SURFACE *p16xSurface = m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
            }
            else
            {
                MOS_SURFACE *p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
            }
            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState          = true;
            surfaceParams.psSurface             = &refScaledSurface;
            surfaceParams.dwOffset              = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
            surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEFwdRefPicIdx[refIdx];
            surfaceParams.ucVDirection          = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME : ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (uint8_t refIdx = 0; refIdx <= params->dwNumRefIdxL1ActiveMinus1; refIdx++)
    {
        refPic = params->pL1RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && params->pPicIdx[refPic.FrameIdx].bValid)
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
                surfaceParams.bUseAdvState          = true;
                surfaceParams.psSurface             = currScaledSurface;
                surfaceParams.dwOffset              = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
                surfaceParams.dwBindingTableOffset  = meBindingTable->dwMECurrForBwdRef;
                surfaceParams.ucVDirection          = ucCurrVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceParams,
                    params->pKernelState));
            }

            refFieldPicture   = CodecHal_PictureIsField(refPic) ? 1 : 0;
            refBottomField    = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            refPicIdx         = params->pPicIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx = params->ppRefList[refPicIdx]->ucScalingIdx;
            if (params->b32xMeInUse)
            {
                MOS_SURFACE *p32xSurface = m_trackedBuf->Get32xDsSurface(scaledIdx);
                if (p32xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p32xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
            }
            else if (params->b16xMeInUse)
            {
                MOS_SURFACE *p16xSurface = m_trackedBuf->Get16xDsSurface(scaledIdx);
                if (p16xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p16xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
            }
            else
            {
                MOS_SURFACE *p4xSurface = m_trackedBuf->Get4xDsSurface(scaledIdx);
                if (p4xSurface != nullptr)
                {
                    refScaledSurface.OsResource = p4xSurface->OsResource;
                }
                else
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("NULL pointer of DsSurface");
                }
                refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;
            }
            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
            surfaceParams.bUseAdvState          = true;
            surfaceParams.psSurface             = &refScaledSurface;
            surfaceParams.dwOffset              = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value;
            surfaceParams.dwBindingTableOffset  = meBindingTable->dwMEBwdRefPicIdx[refIdx];
            surfaceParams.ucVDirection          = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME : ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceParams,
                params->pKernelState));
        }
    }

    if (encMediaStateType == CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
        surfaceParams.dwSize                = params->dwVDEncStreamInSurfaceSize;
        surfaceParams.bIs2DSurface          = false;
        surfaceParams.presBuffer            = params->psMeVdencStreamInBuffer;
        surfaceParams.dwBindingTableOffset  = meBindingTable->dwVdencStreamInSurface;
        surfaceParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
        surfaceParams.bIsWritable           = true;
        surfaceParams.bRenderTarget         = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceParams,
            params->pKernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencAvcState::SetMfxPipeBufAddrStateParams(
    CODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS genericParam,
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &                 param)
{
    MOS_STATUS eStatus = CodechalEncodeAvcBase::SetMfxPipeBufAddrStateParams(genericParam, param);

    param.ps4xDsSurface                       = m_trackedBuf->Get4xDsReconSurface(CODEC_CURR_TRACKED_BUFFER);
    param.presVdencIntraRowStoreScratchBuffer = &m_vdencIntraRowStoreScratchBuffer;
    param.presVdencStreamOutBuffer            = &m_vdencStatsBuffer;
    param.presStreamOutBuffer                 = &m_pakStatsBuffer;
    param.dwNumRefIdxL0ActiveMinus1           = m_avcSliceParams->num_ref_idx_l0_active_minus1;
    param.dwNumRefIdxL1ActiveMinus1           = m_avcSliceParams->num_ref_idx_l1_active_minus1;
    param.oneOnOneMapping                     = m_oneOnOneMapping;

    if (m_pictureCodingType != I_TYPE)
    {
        // populate the RefPic and DS surface so pfnAddVdencPipeBufAddrCmd() can directly use them
        auto l0RefFrameList = m_avcSliceParams->RefPicList[LIST_0];
        for (uint8_t refIdx = 0; refIdx <= m_avcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            auto refPic = l0RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L0 references
                auto refPicIdx                    = m_picIdx[refPic.FrameIdx].ucPicIdx;
                param.presVdencReferences[refIdx] = &m_refList[refPicIdx]->sRefReconBuffer.OsResource;
                param.presVdenc4xDsSurface[refIdx] =
                    &(m_trackedBuf->Get4xDsReconSurface(m_refList[refPicIdx]->ucScalingIdx))->OsResource;
            }
        }
    }

    if (m_vdencStreamInEnabled)
        param.presVdencStreamInBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];

    return eStatus;
}

void CodechalVdencAvcState::SetVdencCqptStateParams(MHW_VDBOX_VDENC_CQPT_STATE_PARAMS &param)
{
    MOS_ZeroMemory(&param, sizeof(param));
    param.wPictureCodingType = m_pictureCodingType;
    param.bFTQEnabled        = m_vdencInterface->VdencFTQEnabled(m_avcSeqParam->TargetUsage);
    param.bTransform8x8Flag  = m_avcPicParam->transform_8x8_mode_flag;
}

void CodechalVdencAvcState::SetMfxAvcImgStateParams(MHW_VDBOX_AVC_IMG_PARAMS &param)
{
    CodechalEncodeAvcBase::SetMfxAvcImgStateParams(param);
    if (m_avcSeqParam->EnableSliceLevelRateCtrl)
    {
        param.dwMbSlcThresholdValue  = m_mbSlcThresholdValue;
        param.dwSliceThresholdTable  = m_sliceThresholdTable;
        param.dwVdencSliceMinusBytes = (m_pictureCodingType == I_TYPE) ? m_vdencSliceMinusI : m_vdencSliceMinusP;
    }

    param.bVdencEnabled   = true;
    param.pVDEncModeCost  = m_vdencModeCostTbl;
    param.pVDEncHmeMvCost = m_vdencHmeMvCostTbl;
    param.pVDEncMvCost    = m_vdencMvCostTbl;
    param.bVDEncPerfModeEnabled =
        m_vdencInterface->IsPerfModeSupported() && m_perfModeEnabled[m_avcSeqParam->TargetUsage];
}

PMHW_VDBOX_STATE_CMDSIZE_PARAMS CodechalVdencAvcState::CreateMhwVdboxStateCmdsizeParams()
{
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams = MOS_New(MHW_VDBOX_STATE_CMDSIZE_PARAMS);
    return stateCmdSizeParams;
}

PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CodechalVdencAvcState::CreateMhwVdboxPipeModeSelectParams()
{
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);

    return pipeModeSelectParams;
}

PMHW_VDBOX_AVC_IMG_PARAMS CodechalVdencAvcState::CreateMhwVdboxAvcImgParams()
{
    PMHW_VDBOX_AVC_IMG_PARAMS avcImgParams = MOS_New(MHW_VDBOX_AVC_IMG_PARAMS);

    return avcImgParams;
}

PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS CodechalVdencAvcState::CreateMhwVdboxVdencWalkerStateParams()
{
    PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS vdencWalkerStateParams = MOS_New(MHW_VDBOX_VDENC_WALKER_STATE_PARAMS);

    return vdencWalkerStateParams;
}

MOS_STATUS CodechalVdencAvcState::FillHucConstData(uint8_t *data)
{
    auto hucConstData = (PAVCVdencBRCCostantData)data;
    auto avcSeqParams = m_avcSeqParam;

    MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabI_U8, 64 * sizeof(uint8_t), (void *)BRC_UPD_GlobalRateQPAdjTabI_U8, 64 * sizeof(uint8_t));
    if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW)  // Sliding Window BRC
    {
        MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t), (void *)BRC_UPD_SlWinGlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t), (void *)BRC_UPD_GlobalRateQPAdjTabP_U8, 64 * sizeof(uint8_t));
    }
    MOS_SecureMemcpy(hucConstData->UPD_GlobalRateQPAdjTabB_U8, 64 * sizeof(uint8_t), (void *)BRC_UPD_GlobalRateQPAdjTabB_U8, 64 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_DistThreshldI_U8, 10 * sizeof(uint8_t), (void *)BRC_UPD_DistThreshldI_U8, 10 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_DistThreshldP_U8, 10 * sizeof(uint8_t), (void *)BRC_UPD_DistThreshldP_U8, 10 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_DistThreshldB_U8, 10 * sizeof(uint8_t), (void *)BRC_UPD_DistThreshldP_U8, 10 * sizeof(uint8_t));

    if (avcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabI_U8, 81 * sizeof(uint8_t), (void *)CBR_UPD_DistQPAdjTabI_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabP_U8, 81 * sizeof(uint8_t), (void *)CBR_UPD_DistQPAdjTabP_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabB_U8, 81 * sizeof(uint8_t), (void *)CBR_UPD_DistQPAdjTabB_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabI_S8, 72 * sizeof(uint8_t), (void *)CBR_UPD_FrmSzAdjTabI_S8, 72 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t), (void *)CBR_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabB_S8, 72 * sizeof(uint8_t), (void *)CBR_UPD_FrmSzAdjTabB_S8, 72 * sizeof(int8_t));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabI_U8, 81 * sizeof(uint8_t), (void *)VBR_UPD_DistQPAdjTabI_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabP_U8, 81 * sizeof(uint8_t), (void *)VBR_UPD_DistQPAdjTabP_U8, 81 * sizeof(int8_t));
        MOS_SecureMemcpy(hucConstData->UPD_DistQPAdjTabB_U8, 81 * sizeof(uint8_t), (void *)VBR_UPD_DistQPAdjTabB_U8, 81 * sizeof(int8_t));

        if (avcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)  // Low Delay Mode
        {
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabI_S8, 72 * sizeof(uint8_t), (void *)LOW_DELAY_UPD_FrmSzAdjTabI_S8, 72 * sizeof(int8_t));
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t), (void *)LOW_DELAY_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabB_S8, 72 * sizeof(uint8_t), (void *)LOW_DELAY_UPD_FrmSzAdjTabB_S8, 72 * sizeof(int8_t));
        }
        else
        {
            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabI_S8, 72 * sizeof(uint8_t), (void *)VBR_UPD_FrmSzAdjTabI_S8, 72 * sizeof(int8_t));

            if (avcSeqParams->RateControlMethod == RATECONTROL_QVBR)
            {
                MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t), (void *)QVBR_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
            }
            else
            {
                MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabP_S8, 72 * sizeof(uint8_t), (void *)VBR_UPD_FrmSzAdjTabP_S8, 72 * sizeof(int8_t));
            }

            MOS_SecureMemcpy(hucConstData->UPD_BufRateAdjTabB_S8, 72 * sizeof(uint8_t), (void *)VBR_UPD_FrmSzAdjTabB_S8, 72 * sizeof(int8_t));
        }
    }

    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMinTabP_U8, 9 * sizeof(uint8_t), (void *)BRC_UPD_FrmSzMinTabP_U8, 9 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMinTabI_U8, 9 * sizeof(uint8_t), (void *)BRC_UPD_FrmSzMinTabI_U8, 9 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMaxTabP_U8, 9 * sizeof(uint8_t), (void *)BRC_UPD_FrmSzMaxTabP_U8, 9 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_FrmSzMaxTabI_U8, 9 * sizeof(uint8_t), (void *)BRC_UPD_FrmSzMaxTabI_U8, 9 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_FrmSzSCGTabP_U8, 9 * sizeof(uint8_t), (void *)BRC_UPD_FrmSzSCGTabP_U8, 9 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_FrmSzSCGTabI_U8, 9 * sizeof(uint8_t), (void *)BRC_UPD_FrmSzSCGTabI_U8, 9 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_I_IntraNonPred, 42 * sizeof(uint8_t), (void *)BRC_UPD_I_IntraNonPred, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_I_Intra8x8, 42 * sizeof(uint8_t), (void *)BRC_UPD_I_Intra8x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_I_Intra4x4, 42 * sizeof(uint8_t), (void *)BRC_UPD_I_Intra4x4, 42 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_P_IntraNonPred, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_IntraNonPred, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Intra16x16, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_Intra16x16, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Intra8x8, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_Intra8x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Intra4x4, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_Intra4x4, 42 * sizeof(uint8_t));

    MOS_SecureMemcpy(hucConstData->UPD_P_Inter16x8, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_Inter16x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Inter8x8, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_Inter8x8, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_Inter16x16, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_Inter16x16, 42 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucConstData->UPD_P_RefId, 42 * sizeof(uint8_t), (void *)BRC_UPD_P_RefId, 42 * sizeof(uint8_t));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(LoadHmeMvCostTable(avcSeqParams, hucConstData->UPD_HMEMVCost));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcState::ExecuteMeKernel()
{
    if (m_hmeEnabled)
    {
        if (m_16xMeEnabled)
        {
            m_lastTaskInPhase = false;
            if (m_32xMeEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(nullptr, HME_LEVEL_32x));
            }
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(nullptr, HME_LEVEL_16x));
        }

        // On-demand sync for VDEnc SHME StreamIn surface
        auto syncParams             = g_cInitSyncParams;
        syncParams.GpuContext       = m_renderContext;
        syncParams.presSyncResource = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));
        m_osInterface->pfnSetResourceSyncTag(m_osInterface, &syncParams);

        // HME StreamIn
        m_lastTaskInPhase = !m_staticFrameDetectionInUse;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(nullptr, HME_LEVEL_4x));
        m_vdencStreamInEnabled = true;
    }
    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencAvcState::DumpHucBrcInit()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
        &m_resVdencBrcInitDmemBuffer[m_currRecycledBufIdx],
        m_vdencBrcInitDmemBufferSize,
        m_currPass,
        hucRegionDumpInit));

    // History Buffer dump
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
        &m_resVdencBrcHistoryBuffer,
        0,
        CODECHAL_VDENC_AVC_BRC_HISTORY_BUF_SIZE,
        0,
        "_History",
        0,
        m_currPass,
        hucRegionDumpInit));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcState::DumpHucBrcUpdate(bool isInput)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (isInput)
    {
        //HUC DMEM dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
            &m_resVdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_currPass],
            m_vdencBrcUpdateDmemBufferSize,
            m_currPass,
            hucRegionDumpUpdate));

        // Constant Data Buffer dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_resVdencBrcConstDataBuffer,
            0,
            GetBRCCostantDataSize(),
            5,
            "_ConstData",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));

        // VDENC Statistics Buffer dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_vdencStatsBuffer,
            0,
            m_vdencBrcStatsBufferSize,
            1,
            "_VdencStats",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));

        // PAK Statistics Buffer dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_pakStatsBuffer,
            0,
            m_vdencBrcPakStatsBufferSize,
            2,
            "_PakStats",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));

        // VDENC Img State Read Buffer dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_resVdencBrcImageStatesReadBuffer[m_currRecycledBufIdx],
            0,
            m_hwInterface->m_vdencBrcImgStateBufferSize,
            3,
            "_ImageStateRead",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));

        // SFD output buffer dump
        if (m_staticFrameDetectionInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_resSfdOutputBuffer[m_currRecycledBufIdx],
                0,
                sizeof(CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE_COMMON),
                4,
                "_SfdOutput",
                isInput,
                m_currPass,
                hucRegionDumpUpdate));
        }

        //  Slice size Buffer dump
        if (m_sliceSizeStreamoutSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
                &m_pakSliceSizeStreamoutBuffer,
                0,
                CODECHAL_ENCODE_SLICESIZE_BUF_SIZE,
                7,
                "_SliceSizeStreamOut",
                isInput,
                m_currPass,
                hucRegionDumpUpdate));
        }
    }
    else
    {
        // History Buffer dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_resVdencBrcHistoryBuffer,
            0,
            CODECHAL_VDENC_AVC_BRC_HISTORY_BUF_SIZE,
            0,
            "_History",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));

        // VDENC Img State Write Buffer dump
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_batchBufferForVdencImgStat[0].OsResource,
            0,
            m_hwInterface->m_vdencBrcImgStateBufferSize,
            6,
            "_ImageStateWrite",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
            &m_resVdencBrcDbgBuffer,
            0,
            CODECHAL_VDENC_AVC_BRC_DEBUG_BUF_SIZE,
            15,
            "_Debug",
            isInput,
            m_currPass,
            hucRegionDumpUpdate));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcState::DumpEncodeImgStats(
    PMOS_COMMAND_BUFFER cmdbuffer)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrImageState))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t size = m_mfxInterface->GetAvcImgStateSize() + m_vdencInterface->GetVdencAvcImgStateSize();

    std::string SurfName = "Pak_VDEnc_Pass[" + std::to_string(static_cast<uint32_t>(m_currPass)) + "]";

    // MFX_AVC_IMG_STATE
    if (m_vdencBrcEnabled)
    {
        // BRC case: both MFX_AVC_IMG_STATE and VDENC_IMG_STATE are updated by HuC FW
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_batchBufferForVdencImgStat[0].OsResource,
            CodechalDbgAttr::attrImageState,
            SurfName.c_str(),
            size,
            0,
            CODECHAL_NUM_MEDIA_STATES));
    }
    else
    {
        // CQP case: updated by driver or SFD kernel
        if (!m_staticFrameDetectionInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_batchBufferForVdencImgStat[m_currRecycledBufIdx].OsResource,
                CodechalDbgAttr::attrImageState,
                SurfName.c_str(),
                size,
                0,
                CODECHAL_NUM_MEDIA_STATES));
        }
        else
        {
            if (!cmdbuffer->pCmdPtr)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }

            uint8_t *data = (uint8_t *)MOS_AllocAndZeroMemory(size);
            CODECHAL_DEBUG_CHK_NULL(data);

            // MFX AVC IMG STATE is updated by driver
            uint8_t *mfxData = (uint8_t *)(cmdbuffer->pCmdPtr - (m_mfxInterface->GetAvcImgStateSize() / sizeof(uint32_t)));
            CODECHAL_DEBUG_CHK_NULL(mfxData);
            MOS_SecureMemcpy(data, m_mfxInterface->GetAvcImgStateSize(), mfxData, m_mfxInterface->GetAvcImgStateSize());

            // VDENC IMG STATE is updated by SFD kernel
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.ReadOnly = 1;
            uint8_t *vdencData = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &m_batchBufferForVdencImgStat[m_currRecycledBufIdx].OsResource, &lockFlags);
            CODECHAL_DEBUG_CHK_NULL(vdencData);
            MOS_SecureMemcpy((data + m_mfxInterface->GetAvcImgStateSize()), m_vdencInterface->GetVdencAvcImgStateSize(), vdencData, m_vdencInterface->GetVdencAvcImgStateSize());
            m_osInterface->pfnUnlockResource(m_osInterface, &m_batchBufferForVdencImgStat[m_currRecycledBufIdx].OsResource);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpData(
                data,
                size,
                CodechalDbgAttr::attrImageState,
                SurfName.c_str()));

            MOS_FreeMemory(data);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcState::PopulateHmeParam(
    bool    is16xMeEnabled,
    bool    is32xMeEnabled,
    uint8_t meMethod,
    void *  cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_ME_CURBE *curbe = (CODECHAL_ENCODE_AVC_ME_CURBE *)cmd;

    if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->SuperHME             = is16xMeEnabled;
        m_avcPar->UltraHME             = is32xMeEnabled;
        m_avcPar->SuperCombineDist     = curbe->DW6.SuperCombineDist;
        m_avcPar->StreamInEnable       = is16xMeEnabled;
        m_avcPar->StreamInL0FromNewRef = is16xMeEnabled ? 7 : 0;
        m_avcPar->StreamInL1FromNewRef = 0;
        m_avcPar->MEMethod             = meMethod;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcState::DumpFrameParFile()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    if (m_pictureCodingType == I_TYPE)
    {
        // I Slice Parameters
        // DDI Params
        oss << "ProfileIDC = " << std::dec << +m_avcPar->ProfileIDC << std::endl;
        oss << "LevelIDC = " << std::dec << +m_avcPar->LevelIDC << std::endl;
        oss << "DisableVUIHeader = " << std::dec << +m_avcPar->DisableVUIHeader << std::endl;
        oss << "ChromaFormatIDC = " << std::dec << +m_avcPar->ChromaFormatIDC << std::endl;
        oss << "ChromaQpOffset = " << std::dec << +m_avcPar->ChromaQpOffset << std::endl;
        oss << "SecondChromaQpOffset = " << std::dec << +m_avcPar->SecondChromaQpOffset << std::endl;
        oss << "PictureCodingType = " << std::dec << +m_avcPar->PictureCodingType << std::endl;
        oss << "NumP = " << std::dec << +m_avcPar->NumP << std::endl;
        oss << "NumB = " << std::dec << +m_avcPar->NumB << std::endl;
        oss << "NumSlices = " << std::dec << +m_avcPar->NumSlices << std::endl;
        oss << "ISliceQP = " << std::dec << +m_avcPar->ISliceQP << std::endl;
        oss << "FrameRateM = " << std::dec << +m_avcPar->FrameRateM << std::endl;
        oss << "FrameRateD = " << std::dec << +m_avcPar->FrameRateD << std::endl;
        oss << "BRCMethod = " << std::dec << +m_avcPar->BRCMethod << std::endl;
        oss << "BRCType = " << std::dec << +m_avcPar->BRCType << std::endl;
        oss << "DeblockingIDC = " << std::dec << +m_avcPar->DeblockingIDC << std::endl;
        oss << "DeblockingFilterAlpha = " << std::dec << +m_avcPar->DeblockingFilterAlpha << std::endl;
        oss << "DeblockingFilterBeta = " << std::dec << +m_avcPar->DeblockingFilterBeta << std::endl;
        oss << "EntropyCodingMode = " << std::dec << +m_avcPar->EntropyCodingMode << std::endl;
        oss << "DirectInference = " << std::dec << +m_avcPar->DirectInference << std::endl;
        oss << "Transform8x8Mode = " << std::dec << +m_avcPar->Transform8x8Mode << std::endl;
        oss << "CRFQualityFactor = " << std::dec << +m_avcPar->CRFQualityFactor << std::endl;
        oss << "ConstrainedIntraPred = " << std::dec << +m_avcPar->ConstrainedIntraPred << std::endl;
        if (m_avcPar->NumP == 0)  // There's no P frame
        {
            oss << "MaxRefIdxL0 = " << std::dec << +m_avcPar->MaxRefIdxL0 << std::endl;
            oss << "MaxRefIdxL1 = " << std::dec << +m_avcPar->MaxRefIdxL1 << std::endl;
        }
        oss << "SliceMode = " << std::dec << +m_avcPar->SliceMode << std::endl;
        if (m_avcPar->SliceMode == 2)
        {
            oss << "MaxNumSlicesCheckEnable = 1" << std::endl;
        }

        // DS Params
        oss << "MBFlatnessThreshold = " << std::dec << +m_encodeParState->m_commonPar->mbFlatnessThreshold << std::endl;

        // BRC init Params
        oss << "MBBRCEnable = " << std::dec << +m_avcPar->MBBRCEnable << std::endl;
        oss << "MBRC = " << std::dec << +m_avcPar->MBRC << std::endl;
        oss << "BitRate = " << std::dec << +m_avcPar->BitRate << std::endl;
        oss << "InitVbvFullnessInBit = " << std::dec << +m_avcPar->InitVbvFullnessInBit << std::endl;
        oss << "MaxBitRate = " << std::dec << +m_avcPar->MaxBitRate << std::endl;
        oss << "VbvSzInBit = " << std::dec << +m_avcPar->VbvSzInBit << std::endl;
        oss << "UserMaxFrame = " << std::dec << +m_avcPar->UserMaxFrame << std::endl;
        oss << "SlidingWindowRCEnable = " << std::dec << +m_avcPar->SlidingWindowEnable << std::endl;
        oss << "SlidingWindowSize = " << std::dec << +m_avcPar->SlidingWindowSize << std::endl;
        oss << "SlidingWindowMaxRateRatio = " << std::dec << +m_avcPar->SlidingWindowMaxRateRatio << std::endl;
        oss << "LowDelayGoldenFrameBoost = " << std::dec << +m_avcPar->LowDelayGoldenFrameBoost << std::endl;
        oss << "TopQPDeltaThrforAdaptive2Pass = " << std::dec << +m_avcPar->TopQPDeltaThrforAdaptive2Pass << std::endl;
        oss << "BotQPDeltaThrforAdaptive2Pass = " << std::dec << +m_avcPar->BotQPDeltaThrforAdaptive2Pass << std::endl;
        oss << "TopFrmSzPctThrforAdaptive2Pass = " << std::dec << +m_avcPar->TopFrmSzPctThrforAdaptive2Pass << std::endl;
        oss << "BotFrmSzPctThrforAdaptive2Pass = " << std::dec << +m_avcPar->BotFrmSzPctThrforAdaptive2Pass << std::endl;
        oss << "MBHeaderCompensation = " << std::dec << +m_avcPar->MBHeaderCompensation << std::endl;
        oss << "QPSelectMethodforFirstPass = " << std::dec << +m_avcPar->QPSelectMethodforFirstPass << std::endl;
        oss << "MBQpCtrl = " << std::dec << +m_avcPar->MBQpCtrl << std::endl;
        oss << "QPMax = " << std::dec << +m_avcPar->QPMax << std::endl;
        oss << "QPMin = " << std::dec << +m_avcPar->QPMin << std::endl;
        oss << "HrdConformanceCheckDisable = " << std::dec << +m_avcPar->HrdConformanceCheckDisable << std::endl;
        oss << "ICQReEncode = " << std::dec << +m_avcPar->ICQReEncode << std::endl;
        oss << "AdaptiveCostAdjustEnable = " << std::dec << +m_avcPar->AdaptiveCostAdjustEnable << std::endl;
        oss << "AdaptiveHMEExtension = " << std::dec << +m_avcPar->AdaptiveHMEExtension << std::endl;
        oss << "StreamInStaticRegion = " << std::dec << +m_avcPar->StreamInStaticRegion << std::endl;
        oss << "ScenarioInfo = " << std::dec << +m_avcPar->ScenarioInfo << std::endl;
        if (m_avcPar->SliceMode == 2)
        {
            oss << "SliceSizeWA = " << std::dec << +m_avcPar->SliceSizeWA << std::endl;
            oss << "INumMbsLag = " << std::dec << +m_mbSlcThresholdValue << std::endl;
            oss << "PNumMbsLag = " << std::dec << +m_mbSlcThresholdValue << std::endl;
        }

        // BRC frame update Params
        oss << "EnableMultipass = " << std::dec << +m_avcPar->EnableMultipass << std::endl;
        oss << "MaxNumPakPasses = " << std::dec << +m_avcPar->MaxNumPakPasses << std::endl;
        oss << "SceneChgDetectEn = " << std::dec << +m_avcPar->SceneChgDetectEn << std::endl;
        oss << "SceneChgPrevIntraPctThresh = " << std::dec << +m_avcPar->SceneChgPrevIntraPctThresh << std::endl;
        oss << "SceneChgCurIntraPctThresh = " << std::dec << +m_avcPar->SceneChgCurIntraPctThresh << std::endl;
        oss << "SceneChgWidth0 = " << std::dec << +m_avcPar->SceneChgWidth0 << std::endl;
        oss << "SceneChgWidth1 = " << std::dec << +m_avcPar->SceneChgWidth1 << std::endl;
        if (m_avcPar->SliceMode == 2)
        {
            oss << "SliceSizeThr = " << std::dec << +m_avcPar->SliceSizeThr << std::endl;
            oss << "SliceMaxSize = " << std::dec << +m_avcPar->SliceMaxSize << std::endl;
        }

        // Enc Params
        oss << "BlockBasedSkip = " << std::dec << +m_avcPar->BlockBasedSkip << std::endl;
        oss << "VDEncPerfMode = " << std::dec << +m_avcPar->VDEncPerfMode << std::endl;

        // PAK Params
        oss << "TrellisQuantizationEnable = " << std::dec << +m_avcPar->TrellisQuantizationEnable << std::endl;
        oss << "RoundingIntraEnabled = " << std::dec << +m_avcPar->RoundingIntraEnabled << std::endl;
        oss << "RoundingIntra = " << std::dec << +m_avcPar->RoundingIntra << std::endl;
        oss << "EnableAdaptiveTrellisQuantization = " << std::dec << +m_avcPar->EnableAdaptiveTrellisQuantization << std::endl;
        oss << "TrellisQuantizationRounding = " << std::dec << +m_avcPar->TrellisQuantizationRounding << std::endl;
        oss << "TrellisQuantizationChromaDisable = " << std::dec << +m_avcPar->TrellisQuantizationChromaDisable << std::endl;
        oss << "ExtendedRhoDomainEn = " << std::dec << +m_avcPar->ExtendedRhoDomainEn << std::endl;
        oss << "EnableSEI = " << std::dec << +m_avcPar->EnableSEI << std::endl;
        if (m_avcPar->NumP == 0)  // There's no P frame
        {
            oss << "FrmHdrEncodingFrequency = " << std::dec << +m_avcPar->FrmHdrEncodingFrequency << std::endl;
        }
        oss << "VDEncMode = 1" << std::endl;
        oss << "EnableExternalCost = 0" << std::endl;
        oss << "EnableNewCost = 1" << std::endl;
        oss << "BestDistQPDelta0 = 0" << std::endl;
        oss << "BestDistQPDelta1 = 0" << std::endl;
        oss << "BestDistQPDelta2 = 0" << std::endl;
        oss << "BestDistQPDelta3 = 0" << std::endl;
        oss << "BestIntra4x4QPDelta = 0" << std::endl;
        oss << "BestIntra8x8QPDelta = 0" << std::endl;
        oss << "BestIntra16x16QPDelta = 0" << std::endl;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        // P Slice Parameters
        // DDI Params
        oss << "PSliceQP = " << std::dec << +m_avcPar->PSliceQP << std::endl;
        oss << "CabacInitIDC = " << std::dec << +m_avcPar->CabacInitIDC << std::endl;
        oss << "MaxRefIdxL0 = " << std::dec << +m_avcPar->MaxRefIdxL0 << std::endl;
        oss << "MaxRefIdxL1 = " << std::dec << +m_avcPar->MaxRefIdxL1 << std::endl;
        if (m_avcPar->NumB == 0)  // There's no B frame
        {
            oss << "EnableWeightPredictionDetection = " << std::dec << +m_avcPar->EnableWeightPredictionDetection << std::endl;
        }
        oss << "WeightedPred = " << std::dec << +m_avcPar->WeightedPred << std::endl;
        if (m_avcPar->WeightedPred)
        {
            oss << "EnableWeightPredictionDetection = 1" << std::endl;
            oss << "FadeDetectionMethod = 1" << std::endl;
        }
        oss << "UseOrigAsRef = " << std::dec << +m_avcPar->UseOrigAsRef << std::endl;
        oss << "BiSubMbPartMask = " << std::dec << +m_avcPar->BiSubMbPartMask << std::endl;
        oss << "StaticFrameZMVPercent = " << std::dec << +m_avcPar->StaticFrameZMVPercent << std::endl;
        oss << "HME0XOffset = " << std::dec << +m_avcPar->hme0XOffset << std::endl;
        oss << "HME0YOffset = " << std::dec << +m_avcPar->hme0YOffset << std::endl;
        oss << "HME1XOffset = " << std::dec << +m_avcPar->hme1XOffset << std::endl;
        oss << "HME1YOffset = " << std::dec << +m_avcPar->hme1YOffset << std::endl;

        // HME Params
        oss << "SuperHME = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->superHME : m_avcPar->SuperHME) << std::endl;
        oss << "UltraHME = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->ultraHME : m_avcPar->UltraHME) << std::endl;
        oss << "SuperCombineDist = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->superCombineDist : m_avcPar->SuperCombineDist) << std::endl;
        oss << "StreamInEnable = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->streamInEnable : m_avcPar->StreamInEnable) << std::endl;
        oss << "StreamInL0FromNewRef = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->streamInL0FromNewRef : m_avcPar->StreamInL0FromNewRef) << std::endl;
        oss << "StreamInL1FromNewRef = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->streamInL1FromNewRef : m_avcPar->StreamInL1FromNewRef) << std::endl;
        oss << "MEMethod = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->meMethod : m_avcPar->MEMethod) << std::endl;

        // Enc Params
        oss << "SubPelMode = " << std::dec << +m_avcPar->SubPelMode << std::endl;
        oss << "FTQBasedSkip = " << std::dec << +m_avcPar->FTQBasedSkip << std::endl;
        oss << "BiMixDisable = " << std::dec << +m_avcPar->BiMixDisable << std::endl;
        oss << "SurvivedSkipCost = " << std::dec << +m_avcPar->SurvivedSkipCost << std::endl;
        oss << "UniMixDisable = " << std::dec << +m_avcPar->UniMixDisable << std::endl;
        oss << "EnableIntraCostScalingForStaticFrame = " << std::dec << +m_avcPar->EnableIntraCostScalingForStaticFrame << std::endl;
        if (m_avcPar->EnableIntraCostScalingForStaticFrame)
        {
            oss << "IntraCostUpdateMethod = 3" << std::endl;
        }
        oss << "StaticFrameIntraCostScalingRatioP = " << std::dec << +m_avcPar->StaticFrameIntraCostScalingRatioP << std::endl;
        oss << "VdencExtPakObjDisable = " << std::dec << +m_avcPar->VdencExtPakObjDisable << std::endl;
        oss << "PPMVDisable = " << std::dec << +m_avcPar->PPMVDisable << std::endl;
        oss << "AdaptiveMvStreamIn = " << std::dec << +m_avcPar->AdaptiveMvStreamIn << std::endl;
        oss << "LargeMvThresh = " << std::dec << +m_avcPar->LargeMvThresh << std::endl;
        oss << "LargeMvPctThreshold = " << std::dec << +m_avcPar->LargeMvPctThreshold << std::endl;

        // BRC Frame Update
        oss << "Transform8x8PDisable = " << std::dec << +m_avcPar->Transform8x8PDisable << std::endl;

        // PAK Params
        oss << "RoundingInterEnabled = " << std::dec << +m_avcPar->RoundingInterEnabled << std::endl;
        oss << "RoundingInter = " << std::dec << +m_avcPar->RoundingInter << std::endl;
        oss << "FrmHdrEncodingFrequency = " << std::dec << +m_avcPar->FrmHdrEncodingFrequency << std::endl;
        oss << "AdaptiveRoundingEnabled = " << std::dec << +m_avcPar->EnableAdaptiveRounding << std::endl;
    }
    else
    {
        oss << "BSliceQP = " << std::dec << +m_avcPar->BSliceQP << std::endl;
    }

    // Dump per frame par file
    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeFrame",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencAvcState::DumpSeqParFile()
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    // I Slice Parameters
    // DDI Params
    oss << "ProfileIDC = " << std::dec << +m_avcPar->ProfileIDC << std::endl;
    oss << "LevelIDC = " << std::dec << +m_avcPar->LevelIDC << std::endl;
    oss << "DisableVUIHeader = " << std::dec << +m_avcPar->DisableVUIHeader << std::endl;
    oss << "ChromaFormatIDC = " << std::dec << +m_avcPar->ChromaFormatIDC << std::endl;
    oss << "ChromaQpOffset = " << std::dec << +m_avcPar->ChromaQpOffset << std::endl;
    oss << "SecondChromaQpOffset = " << std::dec << +m_avcPar->SecondChromaQpOffset << std::endl;
    oss << "PictureCodingType = " << std::dec << +m_avcPar->PictureCodingType << std::endl;
    oss << "NumP = " << std::dec << +m_avcPar->NumP << std::endl;
    oss << "NumB = " << std::dec << +m_avcPar->NumB << std::endl;
    oss << "NumSlices = " << std::dec << +m_avcPar->NumSlices << std::endl;
    oss << "ISliceQP = " << std::dec << +m_avcPar->ISliceQP << std::endl;
    oss << "FrameRateM = " << std::dec << +m_avcPar->FrameRateM << std::endl;
    oss << "FrameRateD = " << std::dec << +m_avcPar->FrameRateD << std::endl;
    oss << "BRCMethod = " << std::dec << +m_avcPar->BRCMethod << std::endl;
    oss << "BRCType = " << std::dec << +m_avcPar->BRCType << std::endl;
    oss << "DeblockingIDC = " << std::dec << +m_avcPar->DeblockingIDC << std::endl;
    oss << "DeblockingFilterAlpha = " << std::dec << +m_avcPar->DeblockingFilterAlpha << std::endl;
    oss << "DeblockingFilterBeta = " << std::dec << +m_avcPar->DeblockingFilterBeta << std::endl;
    oss << "EntropyCodingMode = " << std::dec << +m_avcPar->EntropyCodingMode << std::endl;
    oss << "DirectInference = " << std::dec << +m_avcPar->DirectInference << std::endl;
    oss << "Transform8x8Mode = " << std::dec << +m_avcPar->Transform8x8Mode << std::endl;
    oss << "CRFQualityFactor = " << std::dec << +m_avcPar->CRFQualityFactor << std::endl;
    oss << "ConstrainedIntraPred = " << std::dec << +m_avcPar->ConstrainedIntraPred << std::endl;
    if (m_avcPar->NumP == 0)  // There's no P frame
    {
        oss << "MaxRefIdxL0 = " << std::dec << +m_avcPar->MaxRefIdxL0 << std::endl;
        oss << "MaxRefIdxL1 = " << std::dec << +m_avcPar->MaxRefIdxL1 << std::endl;
    }
    oss << "SliceMode = " << std::dec << +m_avcPar->SliceMode << std::endl;
    if (m_avcPar->SliceMode == 2)
    {
        oss << "MaxNumSlicesCheckEnable = 1" << std::endl;
    }

    // DS Params
    oss << "MBFlatnessThreshold = " << std::dec << +m_encodeParState->m_commonPar->mbFlatnessThreshold << std::endl;

    // BRC init Params
    oss << "MBBRCEnable = " << std::dec << +m_avcPar->MBBRCEnable << std::endl;
    oss << "MBRC = " << std::dec << +m_avcPar->MBRC << std::endl;
    oss << "BitRate = " << std::dec << +m_avcPar->BitRate << std::endl;
    oss << "InitVbvFullnessInBit = " << std::dec << +m_avcPar->InitVbvFullnessInBit << std::endl;
    oss << "MaxBitRate = " << std::dec << +m_avcPar->MaxBitRate << std::endl;
    oss << "VbvSzInBit = " << std::dec << +m_avcPar->VbvSzInBit << std::endl;
    oss << "UserMaxFrame = " << std::dec << +m_avcPar->UserMaxFrame << std::endl;
    oss << "SlidingWindowRCEnable = " << std::dec << +m_avcPar->SlidingWindowEnable << std::endl;
    oss << "SlidingWindowSize = " << std::dec << +m_avcPar->SlidingWindowSize << std::endl;
    oss << "SlidingWindowMaxRateRatio = " << std::dec << +m_avcPar->SlidingWindowMaxRateRatio << std::endl;
    oss << "LowDelayGoldenFrameBoost = " << std::dec << +m_avcPar->LowDelayGoldenFrameBoost << std::endl;
    oss << "TopQPDeltaThrforAdaptive2Pass = " << std::dec << +m_avcPar->TopQPDeltaThrforAdaptive2Pass << std::endl;
    oss << "BotQPDeltaThrforAdaptive2Pass = " << std::dec << +m_avcPar->BotQPDeltaThrforAdaptive2Pass << std::endl;
    oss << "TopFrmSzPctThrforAdaptive2Pass = " << std::dec << +m_avcPar->TopFrmSzPctThrforAdaptive2Pass << std::endl;
    oss << "BotFrmSzPctThrforAdaptive2Pass = " << std::dec << +m_avcPar->BotFrmSzPctThrforAdaptive2Pass << std::endl;
    oss << "MBHeaderCompensation = " << std::dec << +m_avcPar->MBHeaderCompensation << std::endl;
    oss << "QPSelectMethodforFirstPass = " << std::dec << +m_avcPar->QPSelectMethodforFirstPass << std::endl;
    oss << "MBQpCtrl = " << std::dec << +m_avcPar->MBQpCtrl << std::endl;
    oss << "QPMax = " << std::dec << +m_avcPar->QPMax << std::endl;
    oss << "QPMin = " << std::dec << +m_avcPar->QPMin << std::endl;
    oss << "HrdConformanceCheckDisable = " << std::dec << +m_avcPar->HrdConformanceCheckDisable << std::endl;
    oss << "ICQReEncode = " << std::dec << +m_avcPar->ICQReEncode << std::endl;
    oss << "AdaptiveCostAdjustEnable = " << std::dec << +m_avcPar->AdaptiveCostAdjustEnable << std::endl;
    oss << "AdaptiveHMEExtension = " << std::dec << +m_avcPar->AdaptiveHMEExtension << std::endl;
    oss << "StreamInStaticRegion = " << std::dec << +m_avcPar->StreamInStaticRegion << std::endl;
    oss << "ScenarioInfo = " << std::dec << +m_avcPar->ScenarioInfo << std::endl;
    if (m_avcPar->SliceMode == 2)
    {
        oss << "SliceSizeWA = " << std::dec << +m_avcPar->SliceSizeWA << std::endl;
        oss << "INumMbsLag = " << std::dec << +m_mbSlcThresholdValue << std::endl;
        oss << "PNumMbsLag = " << std::dec << +m_mbSlcThresholdValue << std::endl;
    }

    // BRC frame update Params
    oss << "EnableMultipass = " << std::dec << +m_avcPar->EnableMultipass << std::endl;
    oss << "MaxNumPakPasses = " << std::dec << +m_avcPar->MaxNumPakPasses << std::endl;
    oss << "SceneChgDetectEn = " << std::dec << +m_avcPar->SceneChgDetectEn << std::endl;
    oss << "SceneChgPrevIntraPctThresh = " << std::dec << +m_avcPar->SceneChgPrevIntraPctThresh << std::endl;
    oss << "SceneChgCurIntraPctThresh = " << std::dec << +m_avcPar->SceneChgCurIntraPctThresh << std::endl;
    oss << "SceneChgWidth0 = " << std::dec << +m_avcPar->SceneChgWidth0 << std::endl;
    oss << "SceneChgWidth1 = " << std::dec << +m_avcPar->SceneChgWidth1 << std::endl;
    if (m_avcPar->SliceMode == 2)
    {
        oss << "SliceSizeThr = " << std::dec << +m_avcPar->SliceSizeThr << std::endl;
        oss << "SliceMaxSize = " << std::dec << +m_avcPar->SliceMaxSize << std::endl;
    }

    // Enc Params
    oss << "BlockBasedSkip = " << std::dec << +m_avcPar->BlockBasedSkip << std::endl;
    oss << "VDEncPerfMode = " << std::dec << +m_avcPar->VDEncPerfMode << std::endl;
    oss << "SubPelMode = " << std::dec << +m_avcPar->SubPelMode << std::endl;
    oss << "LeftNbrPelMode = " << std::dec << +m_avcPar->LeftNbrPelMode << std::endl;
    oss << "ImePredOverlapThr = " << std::dec << +m_avcPar->ImePredOverlapThr << std::endl;
    oss << "MBSizeEstScalingRatioINTRA = " << std::dec << +m_avcPar->MBSizeEstScalingRatioINTRA << std::endl;
    oss << "IntraMBHdrScaleFactor = " << std::dec << +m_avcPar->IntraMBHdrScaleFactor << std::endl;
    oss << "MBSizeEstScalingRatioINTER = " << std::dec << +m_avcPar->MBSizeEstScalingRatioINTER << std::endl;
    oss << "InterMBHdrScaleFactor = " << std::dec << +m_avcPar->InterMBHdrScaleFactor << std::endl;
    oss << "HMERefWindowSize = " << std::dec << +m_avcPar->HMERefWindowSize << std::endl;
    oss << "IMELeftPredDep = " << std::dec << +m_avcPar->IMELeftPredDep << std::endl;
    oss << "NumFMECandCheck = " << std::dec << +m_avcPar->NumFMECandCheck << std::endl;
    oss << "RdoChromaEnable = " << std::dec << +m_avcPar->RdoChromaEnable << std::endl;
    oss << "Intra4x4ModeMask = " << std::dec << +m_avcPar->Intra4x4ModeMask << std::endl;
    oss << "Intra8x8ModeMask = " << std::dec << +m_avcPar->Intra8x8ModeMask << std::endl;
    oss << "RdoIntraChromaSearch = " << std::dec << +m_avcPar->RdoIntraChromaSearch << std::endl;
    oss << "Intra16x16ModeMask = " << std::dec << +m_avcPar->Intra16x16ModeMask << std::endl;
    oss << "InitMBBudgetTr4x4 = " << std::dec << +m_avcPar->InitMBBudgetTr4x4 << std::endl;
    oss << "ROIEnable = " << std::dec << +m_avcPar->ROIEnable << std::endl;
    oss << "ForceIPCMMinQP = " << std::dec << +m_avcPar->ForceIPCMMinQP << std::endl;
    oss << "IntraTr4x4Percent = " << std::dec << +m_avcPar->IntraTr4x4Percent << std::endl;

    // PAK Params
    oss << "TrellisQuantizationEnable = " << std::dec << +m_avcPar->TrellisQuantizationEnable << std::endl;
    oss << "RoundingIntraEnabled = " << std::dec << +m_avcPar->RoundingIntraEnabled << std::endl;
    oss << "RoundingIntra = " << std::dec << +m_avcPar->RoundingIntra << std::endl;
    oss << "EnableAdaptiveTrellisQuantization = " << std::dec << +m_avcPar->EnableAdaptiveTrellisQuantization << std::endl;
    oss << "TrellisQuantizationRounding = " << std::dec << +m_avcPar->TrellisQuantizationRounding << std::endl;
    oss << "TrellisQuantizationChromaDisable = " << std::dec << +m_avcPar->TrellisQuantizationChromaDisable << std::endl;
    oss << "ExtendedRhoDomainEn = " << std::dec << +m_avcPar->ExtendedRhoDomainEn << std::endl;
    oss << "EnableSEI = " << std::dec << +m_avcPar->EnableSEI << std::endl;
    if (m_avcPar->NumP == 0)  // There's no P frame
    {
        oss << "FrmHdrEncodingFrequency = " << std::dec << +m_avcPar->FrmHdrEncodingFrequency << std::endl;
    }
    oss << "VDEncMode = 1" << std::endl;
    oss << "EnableExternalCost = 0" << std::endl;
    oss << "EnableNewCost = 1" << std::endl;
    oss << "BestDistQPDelta0 = 0" << std::endl;
    oss << "BestDistQPDelta1 = 0" << std::endl;
    oss << "BestDistQPDelta2 = 0" << std::endl;
    oss << "BestDistQPDelta3 = 0" << std::endl;
    oss << "BestIntra4x4QPDelta = 0" << std::endl;
    oss << "BestIntra8x8QPDelta = 0" << std::endl;
    oss << "BestIntra16x16QPDelta = 0" << std::endl;

    if (m_avcPar->NumP > 0)
    {
        // P Slice Parameters
        // DDI Params
        oss << "PSliceQP = " << std::dec << +m_avcPar->PSliceQP << std::endl;
        oss << "CabacInitIDC = " << std::dec << +m_avcPar->CabacInitIDC << std::endl;
        oss << "MaxRefIdxL0 = " << std::dec << +m_avcPar->MaxRefIdxL0 << std::endl;
        oss << "MaxRefIdxL1 = " << std::dec << +m_avcPar->MaxRefIdxL1 << std::endl;
        if (m_avcPar->NumB == 0)  // There's no B frame
        {
            oss << "EnableWeightPredictionDetection = " << std::dec << +m_avcPar->EnableWeightPredictionDetection << std::endl;
        }
        oss << "WeightedPred = " << std::dec << +m_avcPar->WeightedPred << std::endl;
        oss << "WeightedBiPred = " << std::dec << +m_avcPar->WeightedBiPred << std::endl;
        if (m_avcPar->WeightedPred)
        {
            oss << "EnableWeightPredictionDetection = 1" << std::endl;
            oss << "FadeDetectionMethod = 1" << std::endl;
        }
        oss << "UseOrigAsRef = " << std::dec << +m_avcPar->UseOrigAsRef << std::endl;
        oss << "BiSubMbPartMask = " << std::dec << +m_avcPar->BiSubMbPartMask << std::endl;
        oss << "StaticFrameZMVPercent = " << std::dec << +m_avcPar->StaticFrameZMVPercent << std::endl;
        oss << "HME0XOffset = " << std::dec << +m_avcPar->hme0XOffset << std::endl;
        oss << "HME0YOffset = " << std::dec << +m_avcPar->hme0YOffset << std::endl;
        oss << "HME1XOffset = " << std::dec << +m_avcPar->hme1XOffset << std::endl;
        oss << "HME1YOffset = " << std::dec << +m_avcPar->hme1YOffset << std::endl;

        // HME Params
        oss << "SuperHME = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->superHME : m_avcPar->SuperHME) << std::endl;
        oss << "UltraHME = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->ultraHME : m_avcPar->UltraHME) << std::endl;
        oss << "SuperCombineDist = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->superCombineDist : m_avcPar->SuperCombineDist) << std::endl;
        oss << "StreamInEnable = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->streamInEnable : m_avcPar->StreamInEnable) << std::endl;
        oss << "StreamInL0FromNewRef = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->streamInL0FromNewRef : m_avcPar->StreamInL0FromNewRef) << std::endl;
        oss << "StreamInL1FromNewRef = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->streamInL1FromNewRef : m_avcPar->StreamInL1FromNewRef) << std::endl;
        oss << "MEMethod = " << std::dec << +(m_useCommonKernel ? m_encodeParState->m_commonPar->meMethod : m_avcPar->MEMethod) << std::endl;

        // Enc Params
        oss << "FTQBasedSkip = " << std::dec << +m_avcPar->FTQBasedSkip << std::endl;
        oss << "BiMixDisable = " << std::dec << +m_avcPar->BiMixDisable << std::endl;
        oss << "SurvivedSkipCost = " << std::dec << +m_avcPar->SurvivedSkipCost << std::endl;
        oss << "UniMixDisable = " << std::dec << +m_avcPar->UniMixDisable << std::endl;
        oss << "EnableIntraCostScalingForStaticFrame = " << std::dec << +m_avcPar->EnableIntraCostScalingForStaticFrame << std::endl;
        if (m_avcPar->EnableIntraCostScalingForStaticFrame)
        {
            oss << "IntraCostUpdateMethod = 3" << std::endl;
        }
        oss << "StaticFrameIntraCostScalingRatioP = " << std::dec << +m_avcPar->StaticFrameIntraCostScalingRatioP << std::endl;
        oss << "VdencExtPakObjDisable = " << std::dec << +m_avcPar->VdencExtPakObjDisable << std::endl;
        oss << "PPMVDisable = " << std::dec << +m_avcPar->PPMVDisable << std::endl;
        oss << "AdaptiveMvStreamIn = " << std::dec << +m_avcPar->AdaptiveMvStreamIn << std::endl;
        oss << "LargeMvThresh = " << std::dec << +m_avcPar->LargeMvThresh << std::endl;
        oss << "LargeMvPctThreshold = " << std::dec << +m_avcPar->LargeMvPctThreshold << std::endl;
        oss << "DisPSubPartMask = " << std::dec << +m_avcPar->DisPSubPartMask << std::endl;
        oss << "DisPSubMbMask = " << std::dec << +m_avcPar->DisPSubMbMask << std::endl;
        oss << "PFrameMaxNumImePred = " << std::dec << +m_avcPar->PFrameMaxNumImePred << std::endl;
        oss << "PFrameImePredLargeSW = " << std::dec << +m_avcPar->PFrameImePredLargeSW << std::endl;
        oss << "PFrameZeroCbfEn = " << std::dec << +m_avcPar->PFrameZeroCbfEn << std::endl;
        oss << "DirectMode = " << std::dec << +m_avcPar->DirectMode << std::endl;
        oss << "MultiPassHmeEnable = " << std::dec << +m_avcPar->MultiPassHmeEnable << std::endl;

        // BRC Frame Update
        oss << "Transform8x8PDisable = " << std::dec << +m_avcPar->Transform8x8PDisable << std::endl;

        // PAK Params
        oss << "RoundingInterEnabled = " << std::dec << +m_avcPar->RoundingInterEnabled << std::endl;
        oss << "RoundingInter = " << std::dec << +m_avcPar->RoundingInter << std::endl;
        oss << "FrmHdrEncodingFrequency = " << std::dec << +m_avcPar->FrmHdrEncodingFrequency << std::endl;
        oss << "AdaptiveRoundingEnabled = " << std::dec << +m_avcPar->EnableAdaptiveRounding << std::endl;
    }

    if (m_avcPar->NumB > 0)
    {
        oss << "BSliceQP = " << std::dec << +m_avcPar->BSliceQP << std::endl;
        oss << "DisBSubPartMask = " << std::dec << +m_avcPar->DisBSubPartMask << std::endl;
        oss << "DisBSubMbMask = " << std::dec << +m_avcPar->DisBSubMbMask << std::endl;
        oss << "BFrameMaxNumImePred = " << std::dec << +m_avcPar->BFrameMaxNumImePred << std::endl;
        oss << "BFrameImePredLargeSW = " << std::dec << +m_avcPar->BFrameImePredLargeSW << std::endl;
        oss << "BFrameZeroCbfEn = " << std::dec << +m_avcPar->BFrameZeroCbfEn << std::endl;
    }

    const char *fileName = m_debugInterface->CreateFileName(
        "EncodeSequence",
        "EncodePar",
        CodechalDbgExtType::par);

    std::ofstream ofs(fileName, std::ios::app);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

#endif
