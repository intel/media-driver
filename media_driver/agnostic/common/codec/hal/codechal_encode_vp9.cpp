/*
* Copyright (c) 2019-2020 Intel Corporation.
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
//! \file     codechal_encode_vp9.cpp
//! \brief    Defines base class for VP9 dual-pipe encoder.
//!
#include "codechal_encode_vp9.h"
#include "codechal_mmc_encode_vp9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug.h"
#endif

static constexpr uint32_t PAK_TX_MODE_IDX = 0;                                   //idx=0
static constexpr uint32_t PAK_TX_MODE_SELECT_IDX = (PAK_TX_MODE_IDX + 2);                 //idx=2
static constexpr uint32_t PAK_TX_8x8_PROB_IDX = (PAK_TX_MODE_SELECT_IDX + 1);          //idx=3
static constexpr uint32_t PAK_TX_16x16_PROB_IDX = (PAK_TX_8x8_PROB_IDX + 4);             //idx=7
static constexpr uint32_t PAK_TX_32x32_PROB_IDX = (PAK_TX_16x16_PROB_IDX + 8);           //idx=15
static constexpr uint32_t PAK_TX_4x4_COEFF_PROB_IDX = (PAK_TX_32x32_PROB_IDX + 12);          //idx=27
static constexpr uint32_t PAK_TX_8x8_COEFF_PROB_IDX = (PAK_TX_4x4_COEFF_PROB_IDX + 793);     //idx=820
static constexpr uint32_t PAK_TX_16x16_COEFF_PROB_IDX = (PAK_TX_8x8_COEFF_PROB_IDX + 793);     //idx=1613
static constexpr uint32_t PAK_TX_32x32_COEFF_PROB_IDX = (PAK_TX_16x16_COEFF_PROB_IDX + 793);   //idx=2406
static constexpr uint32_t PAK_SKIP_CONTEXT_IDX = (PAK_TX_32x32_COEFF_PROB_IDX + 793);   //idx=3199  
static constexpr uint32_t PAK_INTER_MODE_CTX_IDX = (PAK_SKIP_CONTEXT_IDX + 6);            //idx=3205
static constexpr uint32_t PAK_SWITCHABLE_FILTER_CTX_IDX = (PAK_INTER_MODE_CTX_IDX + 42);         //idx=3247 
static constexpr uint32_t PAK_INTRA_INTER_CTX_IDX = (PAK_SWITCHABLE_FILTER_CTX_IDX + 16);  //idx=3263
static constexpr uint32_t PAK_COMPOUND_PRED_MODE_IDX = (PAK_INTRA_INTER_CTX_IDX + 8);         //idx=3271
static constexpr uint32_t PAK_HYBRID_PRED_CTX_IDX = (PAK_COMPOUND_PRED_MODE_IDX + 2);      //idx=3273   
static constexpr uint32_t PAK_SINGLE_REF_PRED_CTX_IDX = (PAK_HYBRID_PRED_CTX_IDX + 10);        //idx=3283   
static constexpr uint32_t PAK_CMPUND_PRED_CTX_IDX = (PAK_SINGLE_REF_PRED_CTX_IDX + 20);    //idx=3303
static constexpr uint32_t PAK_INTRA_MODE_PROB_CTX_IDX = (PAK_CMPUND_PRED_CTX_IDX + 10);        //idx=3313
static constexpr uint32_t PAK_PARTITION_PROB_IDX = (PAK_INTRA_MODE_PROB_CTX_IDX + 72);    //idx=3385
static constexpr uint32_t PAK_MVJOINTS_PROB_IDX = (PAK_PARTITION_PROB_IDX + 96);         //idx=3481
static constexpr uint32_t PAK_MVCOMP0_IDX = (PAK_MVJOINTS_PROB_IDX + 24);          //idx=3505
static constexpr uint32_t PAK_MVCOMP1_IDX = (PAK_MVCOMP0_IDX + 176);               //idx=3681
static constexpr uint32_t PAK_MVFRAC_COMP0_IDX = (PAK_MVCOMP1_IDX + 176);               //idx=3857
static constexpr uint32_t PAK_MVFRAC_COMP1_IDX = (PAK_MVFRAC_COMP0_IDX + 72);           //idx=3929
static constexpr uint32_t PAK_MVHP_COMP0_IDX = (PAK_MVFRAC_COMP1_IDX + 72);           //idx=4001
static constexpr uint32_t PAK_MVHP_COMP1_IDX = (PAK_MVHP_COMP0_IDX + 16);             //idx=4017
static constexpr uint32_t PAK_COMPRESSED_HDR_SYNTAX_ELEMS = (PAK_MVHP_COMP1_IDX + 16);             //=4033

#define CODECHAL_GET_WIDTH_IN_BLOCKS(dwWidth, dwBlockSize)       (((dwWidth) + (dwBlockSize - 1)) / dwBlockSize)
#define CODECHAL_GET_HEIGHT_IN_BLOCKS(dwHeight, dwBlockSize)     (((dwHeight) + (dwBlockSize - 1)) / dwBlockSize)

typedef struct _CODECHAL_ENCODE_VP9_BRC_OUTPUT_STATIC_DATA
{
    // DW0
    union
    {
        struct
        {
            uint32_t   LongTermReferenceFlag : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(8, 31);
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
            uint32_t   NewFrameWidth : MOS_BITFIELD_RANGE(0, 31);
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
            uint32_t   NewFrameHeight : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // DW3 - DW15
    uint32_t Reserved[13];
} CODECHAL_ENCODE_VP9_BRC_OUTPUT_STATIC_DATA, *PCODECHAL_ENCODE_VP9_BRC_OUTPUT_STATIC_DATA;

static const uint8_t CODECHAL_ENCODE_VP9_Keyframe_Default_Probs[2048] = {
    0x64, 0x42, 0x14, 0x98, 0x0f, 0x65, 0x03, 0x88, 0x25, 0x05, 0x34, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc3, 0x1d, 0xb7, 0x54, 0x31, 0x88, 0x08, 0x2a, 0x47, 0x1f, 0x6b, 0xa9, 0x23, 0x63, 0x9f, 0x11,
    0x52, 0x8c, 0x08, 0x42, 0x72, 0x02, 0x2c, 0x4c, 0x01, 0x13, 0x20, 0x28, 0x84, 0xc9, 0x1d, 0x72,
    0xbb, 0x0d, 0x5b, 0x9d, 0x07, 0x4b, 0x7f, 0x03, 0x3a, 0x5f, 0x01, 0x1c, 0x2f, 0x45, 0x8e, 0xdd,
    0x2a, 0x7a, 0xc9, 0x0f, 0x5b, 0x9f, 0x06, 0x43, 0x79, 0x01, 0x2a, 0x4d, 0x01, 0x11, 0x1f, 0x66,
    0x94, 0xe4, 0x43, 0x75, 0xcc, 0x11, 0x52, 0x9a, 0x06, 0x3b, 0x72, 0x02, 0x27, 0x4b, 0x01, 0x0f,
    0x1d, 0x9c, 0x39, 0xe9, 0x77, 0x39, 0xd4, 0x3a, 0x30, 0xa3, 0x1d, 0x28, 0x7c, 0x0c, 0x1e, 0x51,
    0x03, 0x0c, 0x1f, 0xbf, 0x6b, 0xe2, 0x7c, 0x75, 0xcc, 0x19, 0x63, 0x9b, 0x1d, 0x94, 0xd2, 0x25,
    0x7e, 0xc2, 0x08, 0x5d, 0x9d, 0x02, 0x44, 0x76, 0x01, 0x27, 0x45, 0x01, 0x11, 0x21, 0x29, 0x97,
    0xd5, 0x1b, 0x7b, 0xc1, 0x03, 0x52, 0x90, 0x01, 0x3a, 0x69, 0x01, 0x20, 0x3c, 0x01, 0x0d, 0x1a,
    0x3b, 0x9f, 0xdc, 0x17, 0x7e, 0xc6, 0x04, 0x58, 0x97, 0x01, 0x42, 0x72, 0x01, 0x26, 0x47, 0x01,
    0x12, 0x22, 0x72, 0x88, 0xe8, 0x33, 0x72, 0xcf, 0x0b, 0x53, 0x9b, 0x03, 0x38, 0x69, 0x01, 0x21,
    0x41, 0x01, 0x11, 0x22, 0x95, 0x41, 0xea, 0x79, 0x39, 0xd7, 0x3d, 0x31, 0xa6, 0x1c, 0x24, 0x72,
    0x0c, 0x19, 0x4c, 0x03, 0x10, 0x2a, 0xd6, 0x31, 0xdc, 0x84, 0x3f, 0xbc, 0x2a, 0x41, 0x89, 0x55,
    0x89, 0xdd, 0x68, 0x83, 0xd8, 0x31, 0x6f, 0xc0, 0x15, 0x57, 0x9b, 0x02, 0x31, 0x57, 0x01, 0x10,
    0x1c, 0x59, 0xa3, 0xe6, 0x5a, 0x89, 0xdc, 0x1d, 0x64, 0xb7, 0x0a, 0x46, 0x87, 0x02, 0x2a, 0x51,
    0x01, 0x11, 0x21, 0x6c, 0xa7, 0xed, 0x37, 0x85, 0xde, 0x0f, 0x61, 0xb3, 0x04, 0x48, 0x87, 0x01,
    0x2d, 0x55, 0x01, 0x13, 0x26, 0x7c, 0x92, 0xf0, 0x42, 0x7c, 0xe0, 0x11, 0x58, 0xaf, 0x04, 0x3a,
    0x7a, 0x01, 0x24, 0x4b, 0x01, 0x12, 0x25, 0x8d, 0x4f, 0xf1, 0x7e, 0x46, 0xe3, 0x42, 0x3a, 0xb6,
    0x1e, 0x2c, 0x88, 0x0c, 0x22, 0x60, 0x02, 0x14, 0x2f, 0xe5, 0x63, 0xf9, 0x8f, 0x6f, 0xeb, 0x2e,
    0x6d, 0xc0, 0x52, 0x9e, 0xec, 0x5e, 0x92, 0xe0, 0x19, 0x75, 0xbf, 0x09, 0x57, 0x95, 0x03, 0x38,
    0x63, 0x01, 0x21, 0x39, 0x53, 0xa7, 0xed, 0x44, 0x91, 0xde, 0x0a, 0x67, 0xb1, 0x02, 0x48, 0x83,
    0x01, 0x29, 0x4f, 0x01, 0x14, 0x27, 0x63, 0xa7, 0xef, 0x2f, 0x8d, 0xe0, 0x0a, 0x68, 0xb2, 0x02,
    0x49, 0x85, 0x01, 0x2c, 0x55, 0x01, 0x16, 0x2f, 0x7f, 0x91, 0xf3, 0x47, 0x81, 0xe4, 0x11, 0x5d,
    0xb1, 0x03, 0x3d, 0x7c, 0x01, 0x29, 0x54, 0x01, 0x15, 0x34, 0x9d, 0x4e, 0xf4, 0x8c, 0x48, 0xe7,
    0x45, 0x3a, 0xb8, 0x1f, 0x2c, 0x89, 0x0e, 0x26, 0x69, 0x08, 0x17, 0x3d, 0x7d, 0x22, 0xbb, 0x34,
    0x29, 0x85, 0x06, 0x1f, 0x38, 0x25, 0x6d, 0x99, 0x33, 0x66, 0x93, 0x17, 0x57, 0x80, 0x08, 0x43,
    0x65, 0x01, 0x29, 0x3f, 0x01, 0x13, 0x1d, 0x1f, 0x9a, 0xb9, 0x11, 0x7f, 0xaf, 0x06, 0x60, 0x91,
    0x02, 0x49, 0x72, 0x01, 0x33, 0x52, 0x01, 0x1c, 0x2d, 0x17, 0xa3, 0xc8, 0x0a, 0x83, 0xb9, 0x02,
    0x5d, 0x94, 0x01, 0x43, 0x6f, 0x01, 0x29, 0x45, 0x01, 0x0e, 0x18, 0x1d, 0xb0, 0xd9, 0x0c, 0x91,
    0xc9, 0x03, 0x65, 0x9c, 0x01, 0x45, 0x6f, 0x01, 0x27, 0x3f, 0x01, 0x0e, 0x17, 0x39, 0xc0, 0xe9,
    0x19, 0x9a, 0xd7, 0x06, 0x6d, 0xa7, 0x03, 0x4e, 0x76, 0x01, 0x30, 0x45, 0x01, 0x15, 0x1d, 0xca,
    0x69, 0xf5, 0x6c, 0x6a, 0xd8, 0x12, 0x5a, 0x90, 0x21, 0xac, 0xdb, 0x40, 0x95, 0xce, 0x0e, 0x75,
    0xb1, 0x05, 0x5a, 0x8d, 0x02, 0x3d, 0x5f, 0x01, 0x25, 0x39, 0x21, 0xb3, 0xdc, 0x0b, 0x8c, 0xc6,
    0x01, 0x59, 0x94, 0x01, 0x3c, 0x68, 0x01, 0x21, 0x39, 0x01, 0x0c, 0x15, 0x1e, 0xb5, 0xdd, 0x08,
    0x8d, 0xc6, 0x01, 0x57, 0x91, 0x01, 0x3a, 0x64, 0x01, 0x1f, 0x37, 0x01, 0x0c, 0x14, 0x20, 0xba,
    0xe0, 0x07, 0x8e, 0xc6, 0x01, 0x56, 0x8f, 0x01, 0x3a, 0x64, 0x01, 0x1f, 0x37, 0x01, 0x0c, 0x16,
    0x39, 0xc0, 0xe3, 0x14, 0x8f, 0xcc, 0x03, 0x60, 0x9a, 0x01, 0x44, 0x70, 0x01, 0x2a, 0x45, 0x01,
    0x13, 0x20, 0xd4, 0x23, 0xd7, 0x71, 0x2f, 0xa9, 0x1d, 0x30, 0x69, 0x4a, 0x81, 0xcb, 0x6a, 0x78,
    0xcb, 0x31, 0x6b, 0xb2, 0x13, 0x54, 0x90, 0x04, 0x32, 0x54, 0x01, 0x0f, 0x19, 0x47, 0xac, 0xd9,
    0x2c, 0x8d, 0xd1, 0x0f, 0x66, 0xad, 0x06, 0x4c, 0x85, 0x02, 0x33, 0x59, 0x01, 0x18, 0x2a, 0x40,
    0xb9, 0xe7, 0x1f, 0x94, 0xd8, 0x08, 0x67, 0xaf, 0x03, 0x4a, 0x83, 0x01, 0x2e, 0x51, 0x01, 0x12,
    0x1e, 0x41, 0xc4, 0xeb, 0x19, 0x9d, 0xdd, 0x05, 0x69, 0xae, 0x01, 0x43, 0x78, 0x01, 0x26, 0x45,
    0x01, 0x0f, 0x1e, 0x41, 0xcc, 0xee, 0x1e, 0x9c, 0xe0, 0x07, 0x6b, 0xb1, 0x02, 0x46, 0x7c, 0x01,
    0x2a, 0x49, 0x01, 0x12, 0x22, 0xe1, 0x56, 0xfb, 0x90, 0x68, 0xeb, 0x2a, 0x63, 0xb5, 0x55, 0xaf,
    0xef, 0x70, 0xa5, 0xe5, 0x1d, 0x88, 0xc8, 0x0c, 0x67, 0xa2, 0x06, 0x4d, 0x7b, 0x02, 0x35, 0x54,
    0x4b, 0xb7, 0xef, 0x1e, 0x9b, 0xdd, 0x03, 0x6a, 0xab, 0x01, 0x4a, 0x80, 0x01, 0x2c, 0x4c, 0x01,
    0x11, 0x1c, 0x49, 0xb9, 0xf0, 0x1b, 0x9f, 0xde, 0x02, 0x6b, 0xac, 0x01, 0x4b, 0x7f, 0x01, 0x2a,
    0x49, 0x01, 0x11, 0x1d, 0x3e, 0xbe, 0xee, 0x15, 0x9f, 0xde, 0x02, 0x6b, 0xac, 0x01, 0x48, 0x7a,
    0x01, 0x28, 0x47, 0x01, 0x12, 0x20, 0x3d, 0xc7, 0xf0, 0x1b, 0xa1, 0xe2, 0x04, 0x71, 0xb4, 0x01,
    0x4c, 0x81, 0x01, 0x2e, 0x50, 0x01, 0x17, 0x29, 0x07, 0x1b, 0x99, 0x05, 0x1e, 0x5f, 0x01, 0x10,
    0x1e, 0x32, 0x4b, 0x7f, 0x39, 0x4b, 0x7c, 0x1b, 0x43, 0x6c, 0x0a, 0x36, 0x56, 0x01, 0x21, 0x34,
    0x01, 0x0c, 0x12, 0x2b, 0x7d, 0x97, 0x1a, 0x6c, 0x94, 0x07, 0x53, 0x7a, 0x02, 0x3b, 0x59, 0x01,
    0x26, 0x3c, 0x01, 0x11, 0x1b, 0x17, 0x90, 0xa3, 0x0d, 0x70, 0x9a, 0x02, 0x4b, 0x75, 0x01, 0x32,
    0x51, 0x01, 0x1f, 0x33, 0x01, 0x0e, 0x17, 0x12, 0xa2, 0xb9, 0x06, 0x7b, 0xab, 0x01, 0x4e, 0x7d,
    0x01, 0x33, 0x56, 0x01, 0x1f, 0x36, 0x01, 0x0e, 0x17, 0x0f, 0xc7, 0xe3, 0x03, 0x96, 0xcc, 0x01,
    0x5b, 0x92, 0x01, 0x37, 0x5f, 0x01, 0x1e, 0x35, 0x01, 0x0b, 0x14, 0x13, 0x37, 0xf0, 0x13, 0x3b,
    0xc4, 0x03, 0x34, 0x69, 0x29, 0xa6, 0xcf, 0x68, 0x99, 0xc7, 0x1f, 0x7b, 0xb5, 0x0e, 0x65, 0x98,
    0x05, 0x48, 0x6a, 0x01, 0x24, 0x34, 0x23, 0xb0, 0xd3, 0x0c, 0x83, 0xbe, 0x02, 0x58, 0x90, 0x01,
    0x3c, 0x65, 0x01, 0x24, 0x3c, 0x01, 0x10, 0x1c, 0x1c, 0xb7, 0xd5, 0x08, 0x86, 0xbf, 0x01, 0x56,
    0x8e, 0x01, 0x38, 0x60, 0x01, 0x1e, 0x35, 0x01, 0x0c, 0x14, 0x14, 0xbe, 0xd7, 0x04, 0x87, 0xc0,
    0x01, 0x54, 0x8b, 0x01, 0x35, 0x5b, 0x01, 0x1c, 0x31, 0x01, 0x0b, 0x14, 0x0d, 0xc4, 0xd8, 0x02,
    0x89, 0xc0, 0x01, 0x56, 0x8f, 0x01, 0x39, 0x63, 0x01, 0x20, 0x38, 0x01, 0x0d, 0x18, 0xd3, 0x1d,
    0xd9, 0x60, 0x2f, 0x9c, 0x16, 0x2b, 0x57, 0x4e, 0x78, 0xc1, 0x6f, 0x74, 0xba, 0x2e, 0x66, 0xa4,
    0x0f, 0x50, 0x80, 0x02, 0x31, 0x4c, 0x01, 0x12, 0x1c, 0x47, 0xa1, 0xcb, 0x2a, 0x84, 0xc0, 0x0a,
    0x62, 0x96, 0x03, 0x45, 0x6d, 0x01, 0x2c, 0x46, 0x01, 0x12, 0x1d, 0x39, 0xba, 0xd3, 0x1e, 0x8c,
    0xc4, 0x04, 0x5d, 0x92, 0x01, 0x3e, 0x66, 0x01, 0x26, 0x41, 0x01, 0x10, 0x1b, 0x2f, 0xc7, 0xd9,
    0x0e, 0x91, 0xc4, 0x01, 0x58, 0x8e, 0x01, 0x39, 0x62, 0x01, 0x24, 0x3e, 0x01, 0x0f, 0x1a, 0x1a,
    0xdb, 0xe5, 0x05, 0x9b, 0xcf, 0x01, 0x5e, 0x97, 0x01, 0x3c, 0x68, 0x01, 0x24, 0x3e, 0x01, 0x10,
    0x1c, 0xe9, 0x1d, 0xf8, 0x92, 0x2f, 0xdc, 0x2b, 0x34, 0x8c, 0x64, 0xa3, 0xe8, 0xb3, 0xa1, 0xde,
    0x3f, 0x8e, 0xcc, 0x25, 0x71, 0xae, 0x1a, 0x59, 0x89, 0x12, 0x44, 0x61, 0x55, 0xb5, 0xe6, 0x20,
    0x92, 0xd1, 0x07, 0x64, 0xa4, 0x03, 0x47, 0x79, 0x01, 0x2d, 0x4d, 0x01, 0x12, 0x1e, 0x41, 0xbb,
    0xe6, 0x14, 0x94, 0xcf, 0x02, 0x61, 0x9f, 0x01, 0x44, 0x74, 0x01, 0x28, 0x46, 0x01, 0x0e, 0x1d,
    0x28, 0xc2, 0xe3, 0x08, 0x93, 0xcc, 0x01, 0x5e, 0x9b, 0x01, 0x41, 0x70, 0x01, 0x27, 0x42, 0x01,
    0x0e, 0x1a, 0x10, 0xd0, 0xe4, 0x03, 0x97, 0xcf, 0x01, 0x62, 0xa0, 0x01, 0x43, 0x75, 0x01, 0x29,
    0x4a, 0x01, 0x11, 0x1f, 0x11, 0x26, 0x8c, 0x07, 0x22, 0x50, 0x01, 0x11, 0x1d, 0x25, 0x4b, 0x80,
    0x29, 0x4c, 0x80, 0x1a, 0x42, 0x74, 0x0c, 0x34, 0x5e, 0x02, 0x20, 0x37, 0x01, 0x0a, 0x10, 0x32,
    0x7f, 0x9a, 0x25, 0x6d, 0x98, 0x10, 0x52, 0x79, 0x05, 0x3b, 0x55, 0x01, 0x23, 0x36, 0x01, 0x0d,
    0x14, 0x28, 0x8e, 0xa7, 0x11, 0x6e, 0x9d, 0x02, 0x47, 0x70, 0x01, 0x2c, 0x48, 0x01, 0x1b, 0x2d,
    0x01, 0x0b, 0x11, 0x1e, 0xaf, 0xbc, 0x09, 0x7c, 0xa9, 0x01, 0x4a, 0x74, 0x01, 0x30, 0x4e, 0x01,
    0x1e, 0x31, 0x01, 0x0b, 0x12, 0x0a, 0xde, 0xdf, 0x02, 0x96, 0xc2, 0x01, 0x53, 0x80, 0x01, 0x30,
    0x4f, 0x01, 0x1b, 0x2d, 0x01, 0x0b, 0x11, 0x24, 0x29, 0xeb, 0x1d, 0x24, 0xc1, 0x0a, 0x1b, 0x6f,
    0x55, 0xa5, 0xde, 0xb1, 0xa2, 0xd7, 0x6e, 0x87, 0xc3, 0x39, 0x71, 0xa8, 0x17, 0x53, 0x78, 0x0a,
    0x31, 0x3d, 0x55, 0xbe, 0xdf, 0x24, 0x8b, 0xc8, 0x05, 0x5a, 0x92, 0x01, 0x3c, 0x67, 0x01, 0x26,
    0x41, 0x01, 0x12, 0x1e, 0x48, 0xca, 0xdf, 0x17, 0x8d, 0xc7, 0x02, 0x56, 0x8c, 0x01, 0x38, 0x61,
    0x01, 0x24, 0x3d, 0x01, 0x10, 0x1b, 0x37, 0xda, 0xe1, 0x0d, 0x91, 0xc8, 0x01, 0x56, 0x8d, 0x01,
    0x39, 0x63, 0x01, 0x23, 0x3d, 0x01, 0x0d, 0x16, 0x0f, 0xeb, 0xd4, 0x01, 0x84, 0xb8, 0x01, 0x54,
    0x8b, 0x01, 0x39, 0x61, 0x01, 0x22, 0x38, 0x01, 0x0e, 0x17, 0xb5, 0x15, 0xc9, 0x3d, 0x25, 0x7b,
    0x0a, 0x26, 0x47, 0x2f, 0x6a, 0xac, 0x5f, 0x68, 0xad, 0x2a, 0x5d, 0x9f, 0x12, 0x4d, 0x83, 0x04,
    0x32, 0x51, 0x01, 0x11, 0x17, 0x3e, 0x93, 0xc7, 0x2c, 0x82, 0xbd, 0x1c, 0x66, 0x9a, 0x12, 0x4b,
    0x73, 0x02, 0x2c, 0x41, 0x01, 0x0c, 0x13, 0x37, 0x99, 0xd2, 0x18, 0x82, 0xc2, 0x03, 0x5d, 0x92,
    0x01, 0x3d, 0x61, 0x01, 0x1f, 0x32, 0x01, 0x0a, 0x10, 0x31, 0xba, 0xdf, 0x11, 0x94, 0xcc, 0x01,
    0x60, 0x8e, 0x01, 0x35, 0x53, 0x01, 0x1a, 0x2c, 0x01, 0x0b, 0x11, 0x0d, 0xd9, 0xd4, 0x02, 0x88,
    0xb4, 0x01, 0x4e, 0x7c, 0x01, 0x32, 0x53, 0x01, 0x1d, 0x31, 0x01, 0x0e, 0x17, 0xc5, 0x0d, 0xf7,
    0x52, 0x11, 0xde, 0x19, 0x11, 0xa2, 0x7e, 0xba, 0xf7, 0xea, 0xbf, 0xf3, 0xb0, 0xb1, 0xea, 0x68,
    0x9e, 0xdc, 0x42, 0x80, 0xba, 0x37, 0x5a, 0x89, 0x6f, 0xc5, 0xf2, 0x2e, 0x9e, 0xdb, 0x09, 0x68,
    0xab, 0x02, 0x41, 0x7d, 0x01, 0x2c, 0x50, 0x01, 0x11, 0x5b, 0x68, 0xd0, 0xf5, 0x27, 0xa8, 0xe0,
    0x03, 0x6d, 0xa2, 0x01, 0x4f, 0x7c, 0x01, 0x32, 0x66, 0x01, 0x2b, 0x66, 0x54, 0xdc, 0xf6, 0x1f,
    0xb1, 0xe7, 0x02, 0x73, 0xb4, 0x01, 0x4f, 0x86, 0x01, 0x37, 0x4d, 0x01, 0x3c, 0x4f, 0x2b, 0xf3,
    0xf0, 0x08, 0xb4, 0xd9, 0x01, 0x73, 0xa6, 0x01, 0x54, 0x79, 0x01, 0x33, 0x43, 0x01, 0x10, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc0, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9e, 0x61, 0x5e, 0x5d,
    0x18, 0x63, 0x55, 0x77, 0x2c, 0x3e, 0x3b, 0x43, 0x95, 0x35, 0x35, 0x5e, 0x14, 0x30, 0x53, 0x35,
    0x18, 0x34, 0x12, 0x12, 0x96, 0x28, 0x27, 0x4e, 0x0c, 0x1a, 0x43, 0x21, 0x0b, 0x18, 0x07, 0x05,
    0xae, 0x23, 0x31, 0x44, 0x0b, 0x1b, 0x39, 0x0f, 0x09, 0x0c, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x90, 0x0b, 0x36, 0x9d, 0xc3, 0x82, 0x2e, 0x3a, 0x6c, 0x76, 0x0f, 0x7b, 0x94, 0x83, 0x65, 0x2c,
    0x5d, 0x83, 0x71, 0x0c, 0x17, 0xbc, 0xe2, 0x8e, 0x1a, 0x20, 0x7d, 0x78, 0x0b, 0x32, 0x7b, 0xa3,
    0x87, 0x40, 0x4d, 0x67, 0x71, 0x09, 0x24, 0x9b, 0x6f, 0x9d, 0x20, 0x2c, 0xa1, 0x74, 0x09, 0x37,
    0xb0, 0x4c, 0x60, 0x25, 0x3d, 0x95, 0x73, 0x09, 0x1c, 0x8d, 0xa1, 0xa7, 0x15, 0x19, 0xc1, 0x78,
    0x0c, 0x20, 0x91, 0xc3, 0x8e, 0x20, 0x26, 0x56, 0x74, 0x0c, 0x40, 0x78, 0x8c, 0x7d, 0x31, 0x73,
    0x79, 0x66, 0x13, 0x42, 0xa2, 0xb6, 0x7a, 0x23, 0x3b, 0x80, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static const uint8_t CODECHAL_ENCODE_VP9_Inter_Default_Probs[2048] = {
    0x64, 0x42, 0x14, 0x98, 0x0f, 0x65, 0x03, 0x88, 0x25, 0x05, 0x34, 0x0d, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc3, 0x1d, 0xb7, 0x54, 0x31, 0x88, 0x08, 0x2a, 0x47, 0x1f, 0x6b, 0xa9, 0x23, 0x63, 0x9f, 0x11,
    0x52, 0x8c, 0x08, 0x42, 0x72, 0x02, 0x2c, 0x4c, 0x01, 0x13, 0x20, 0x28, 0x84, 0xc9, 0x1d, 0x72,
    0xbb, 0x0d, 0x5b, 0x9d, 0x07, 0x4b, 0x7f, 0x03, 0x3a, 0x5f, 0x01, 0x1c, 0x2f, 0x45, 0x8e, 0xdd,
    0x2a, 0x7a, 0xc9, 0x0f, 0x5b, 0x9f, 0x06, 0x43, 0x79, 0x01, 0x2a, 0x4d, 0x01, 0x11, 0x1f, 0x66,
    0x94, 0xe4, 0x43, 0x75, 0xcc, 0x11, 0x52, 0x9a, 0x06, 0x3b, 0x72, 0x02, 0x27, 0x4b, 0x01, 0x0f,
    0x1d, 0x9c, 0x39, 0xe9, 0x77, 0x39, 0xd4, 0x3a, 0x30, 0xa3, 0x1d, 0x28, 0x7c, 0x0c, 0x1e, 0x51,
    0x03, 0x0c, 0x1f, 0xbf, 0x6b, 0xe2, 0x7c, 0x75, 0xcc, 0x19, 0x63, 0x9b, 0x1d, 0x94, 0xd2, 0x25,
    0x7e, 0xc2, 0x08, 0x5d, 0x9d, 0x02, 0x44, 0x76, 0x01, 0x27, 0x45, 0x01, 0x11, 0x21, 0x29, 0x97,
    0xd5, 0x1b, 0x7b, 0xc1, 0x03, 0x52, 0x90, 0x01, 0x3a, 0x69, 0x01, 0x20, 0x3c, 0x01, 0x0d, 0x1a,
    0x3b, 0x9f, 0xdc, 0x17, 0x7e, 0xc6, 0x04, 0x58, 0x97, 0x01, 0x42, 0x72, 0x01, 0x26, 0x47, 0x01,
    0x12, 0x22, 0x72, 0x88, 0xe8, 0x33, 0x72, 0xcf, 0x0b, 0x53, 0x9b, 0x03, 0x38, 0x69, 0x01, 0x21,
    0x41, 0x01, 0x11, 0x22, 0x95, 0x41, 0xea, 0x79, 0x39, 0xd7, 0x3d, 0x31, 0xa6, 0x1c, 0x24, 0x72,
    0x0c, 0x19, 0x4c, 0x03, 0x10, 0x2a, 0xd6, 0x31, 0xdc, 0x84, 0x3f, 0xbc, 0x2a, 0x41, 0x89, 0x55,
    0x89, 0xdd, 0x68, 0x83, 0xd8, 0x31, 0x6f, 0xc0, 0x15, 0x57, 0x9b, 0x02, 0x31, 0x57, 0x01, 0x10,
    0x1c, 0x59, 0xa3, 0xe6, 0x5a, 0x89, 0xdc, 0x1d, 0x64, 0xb7, 0x0a, 0x46, 0x87, 0x02, 0x2a, 0x51,
    0x01, 0x11, 0x21, 0x6c, 0xa7, 0xed, 0x37, 0x85, 0xde, 0x0f, 0x61, 0xb3, 0x04, 0x48, 0x87, 0x01,
    0x2d, 0x55, 0x01, 0x13, 0x26, 0x7c, 0x92, 0xf0, 0x42, 0x7c, 0xe0, 0x11, 0x58, 0xaf, 0x04, 0x3a,
    0x7a, 0x01, 0x24, 0x4b, 0x01, 0x12, 0x25, 0x8d, 0x4f, 0xf1, 0x7e, 0x46, 0xe3, 0x42, 0x3a, 0xb6,
    0x1e, 0x2c, 0x88, 0x0c, 0x22, 0x60, 0x02, 0x14, 0x2f, 0xe5, 0x63, 0xf9, 0x8f, 0x6f, 0xeb, 0x2e,
    0x6d, 0xc0, 0x52, 0x9e, 0xec, 0x5e, 0x92, 0xe0, 0x19, 0x75, 0xbf, 0x09, 0x57, 0x95, 0x03, 0x38,
    0x63, 0x01, 0x21, 0x39, 0x53, 0xa7, 0xed, 0x44, 0x91, 0xde, 0x0a, 0x67, 0xb1, 0x02, 0x48, 0x83,
    0x01, 0x29, 0x4f, 0x01, 0x14, 0x27, 0x63, 0xa7, 0xef, 0x2f, 0x8d, 0xe0, 0x0a, 0x68, 0xb2, 0x02,
    0x49, 0x85, 0x01, 0x2c, 0x55, 0x01, 0x16, 0x2f, 0x7f, 0x91, 0xf3, 0x47, 0x81, 0xe4, 0x11, 0x5d,
    0xb1, 0x03, 0x3d, 0x7c, 0x01, 0x29, 0x54, 0x01, 0x15, 0x34, 0x9d, 0x4e, 0xf4, 0x8c, 0x48, 0xe7,
    0x45, 0x3a, 0xb8, 0x1f, 0x2c, 0x89, 0x0e, 0x26, 0x69, 0x08, 0x17, 0x3d, 0x7d, 0x22, 0xbb, 0x34,
    0x29, 0x85, 0x06, 0x1f, 0x38, 0x25, 0x6d, 0x99, 0x33, 0x66, 0x93, 0x17, 0x57, 0x80, 0x08, 0x43,
    0x65, 0x01, 0x29, 0x3f, 0x01, 0x13, 0x1d, 0x1f, 0x9a, 0xb9, 0x11, 0x7f, 0xaf, 0x06, 0x60, 0x91,
    0x02, 0x49, 0x72, 0x01, 0x33, 0x52, 0x01, 0x1c, 0x2d, 0x17, 0xa3, 0xc8, 0x0a, 0x83, 0xb9, 0x02,
    0x5d, 0x94, 0x01, 0x43, 0x6f, 0x01, 0x29, 0x45, 0x01, 0x0e, 0x18, 0x1d, 0xb0, 0xd9, 0x0c, 0x91,
    0xc9, 0x03, 0x65, 0x9c, 0x01, 0x45, 0x6f, 0x01, 0x27, 0x3f, 0x01, 0x0e, 0x17, 0x39, 0xc0, 0xe9,
    0x19, 0x9a, 0xd7, 0x06, 0x6d, 0xa7, 0x03, 0x4e, 0x76, 0x01, 0x30, 0x45, 0x01, 0x15, 0x1d, 0xca,
    0x69, 0xf5, 0x6c, 0x6a, 0xd8, 0x12, 0x5a, 0x90, 0x21, 0xac, 0xdb, 0x40, 0x95, 0xce, 0x0e, 0x75,
    0xb1, 0x05, 0x5a, 0x8d, 0x02, 0x3d, 0x5f, 0x01, 0x25, 0x39, 0x21, 0xb3, 0xdc, 0x0b, 0x8c, 0xc6,
    0x01, 0x59, 0x94, 0x01, 0x3c, 0x68, 0x01, 0x21, 0x39, 0x01, 0x0c, 0x15, 0x1e, 0xb5, 0xdd, 0x08,
    0x8d, 0xc6, 0x01, 0x57, 0x91, 0x01, 0x3a, 0x64, 0x01, 0x1f, 0x37, 0x01, 0x0c, 0x14, 0x20, 0xba,
    0xe0, 0x07, 0x8e, 0xc6, 0x01, 0x56, 0x8f, 0x01, 0x3a, 0x64, 0x01, 0x1f, 0x37, 0x01, 0x0c, 0x16,
    0x39, 0xc0, 0xe3, 0x14, 0x8f, 0xcc, 0x03, 0x60, 0x9a, 0x01, 0x44, 0x70, 0x01, 0x2a, 0x45, 0x01,
    0x13, 0x20, 0xd4, 0x23, 0xd7, 0x71, 0x2f, 0xa9, 0x1d, 0x30, 0x69, 0x4a, 0x81, 0xcb, 0x6a, 0x78,
    0xcb, 0x31, 0x6b, 0xb2, 0x13, 0x54, 0x90, 0x04, 0x32, 0x54, 0x01, 0x0f, 0x19, 0x47, 0xac, 0xd9,
    0x2c, 0x8d, 0xd1, 0x0f, 0x66, 0xad, 0x06, 0x4c, 0x85, 0x02, 0x33, 0x59, 0x01, 0x18, 0x2a, 0x40,
    0xb9, 0xe7, 0x1f, 0x94, 0xd8, 0x08, 0x67, 0xaf, 0x03, 0x4a, 0x83, 0x01, 0x2e, 0x51, 0x01, 0x12,
    0x1e, 0x41, 0xc4, 0xeb, 0x19, 0x9d, 0xdd, 0x05, 0x69, 0xae, 0x01, 0x43, 0x78, 0x01, 0x26, 0x45,
    0x01, 0x0f, 0x1e, 0x41, 0xcc, 0xee, 0x1e, 0x9c, 0xe0, 0x07, 0x6b, 0xb1, 0x02, 0x46, 0x7c, 0x01,
    0x2a, 0x49, 0x01, 0x12, 0x22, 0xe1, 0x56, 0xfb, 0x90, 0x68, 0xeb, 0x2a, 0x63, 0xb5, 0x55, 0xaf,
    0xef, 0x70, 0xa5, 0xe5, 0x1d, 0x88, 0xc8, 0x0c, 0x67, 0xa2, 0x06, 0x4d, 0x7b, 0x02, 0x35, 0x54,
    0x4b, 0xb7, 0xef, 0x1e, 0x9b, 0xdd, 0x03, 0x6a, 0xab, 0x01, 0x4a, 0x80, 0x01, 0x2c, 0x4c, 0x01,
    0x11, 0x1c, 0x49, 0xb9, 0xf0, 0x1b, 0x9f, 0xde, 0x02, 0x6b, 0xac, 0x01, 0x4b, 0x7f, 0x01, 0x2a,
    0x49, 0x01, 0x11, 0x1d, 0x3e, 0xbe, 0xee, 0x15, 0x9f, 0xde, 0x02, 0x6b, 0xac, 0x01, 0x48, 0x7a,
    0x01, 0x28, 0x47, 0x01, 0x12, 0x20, 0x3d, 0xc7, 0xf0, 0x1b, 0xa1, 0xe2, 0x04, 0x71, 0xb4, 0x01,
    0x4c, 0x81, 0x01, 0x2e, 0x50, 0x01, 0x17, 0x29, 0x07, 0x1b, 0x99, 0x05, 0x1e, 0x5f, 0x01, 0x10,
    0x1e, 0x32, 0x4b, 0x7f, 0x39, 0x4b, 0x7c, 0x1b, 0x43, 0x6c, 0x0a, 0x36, 0x56, 0x01, 0x21, 0x34,
    0x01, 0x0c, 0x12, 0x2b, 0x7d, 0x97, 0x1a, 0x6c, 0x94, 0x07, 0x53, 0x7a, 0x02, 0x3b, 0x59, 0x01,
    0x26, 0x3c, 0x01, 0x11, 0x1b, 0x17, 0x90, 0xa3, 0x0d, 0x70, 0x9a, 0x02, 0x4b, 0x75, 0x01, 0x32,
    0x51, 0x01, 0x1f, 0x33, 0x01, 0x0e, 0x17, 0x12, 0xa2, 0xb9, 0x06, 0x7b, 0xab, 0x01, 0x4e, 0x7d,
    0x01, 0x33, 0x56, 0x01, 0x1f, 0x36, 0x01, 0x0e, 0x17, 0x0f, 0xc7, 0xe3, 0x03, 0x96, 0xcc, 0x01,
    0x5b, 0x92, 0x01, 0x37, 0x5f, 0x01, 0x1e, 0x35, 0x01, 0x0b, 0x14, 0x13, 0x37, 0xf0, 0x13, 0x3b,
    0xc4, 0x03, 0x34, 0x69, 0x29, 0xa6, 0xcf, 0x68, 0x99, 0xc7, 0x1f, 0x7b, 0xb5, 0x0e, 0x65, 0x98,
    0x05, 0x48, 0x6a, 0x01, 0x24, 0x34, 0x23, 0xb0, 0xd3, 0x0c, 0x83, 0xbe, 0x02, 0x58, 0x90, 0x01,
    0x3c, 0x65, 0x01, 0x24, 0x3c, 0x01, 0x10, 0x1c, 0x1c, 0xb7, 0xd5, 0x08, 0x86, 0xbf, 0x01, 0x56,
    0x8e, 0x01, 0x38, 0x60, 0x01, 0x1e, 0x35, 0x01, 0x0c, 0x14, 0x14, 0xbe, 0xd7, 0x04, 0x87, 0xc0,
    0x01, 0x54, 0x8b, 0x01, 0x35, 0x5b, 0x01, 0x1c, 0x31, 0x01, 0x0b, 0x14, 0x0d, 0xc4, 0xd8, 0x02,
    0x89, 0xc0, 0x01, 0x56, 0x8f, 0x01, 0x39, 0x63, 0x01, 0x20, 0x38, 0x01, 0x0d, 0x18, 0xd3, 0x1d,
    0xd9, 0x60, 0x2f, 0x9c, 0x16, 0x2b, 0x57, 0x4e, 0x78, 0xc1, 0x6f, 0x74, 0xba, 0x2e, 0x66, 0xa4,
    0x0f, 0x50, 0x80, 0x02, 0x31, 0x4c, 0x01, 0x12, 0x1c, 0x47, 0xa1, 0xcb, 0x2a, 0x84, 0xc0, 0x0a,
    0x62, 0x96, 0x03, 0x45, 0x6d, 0x01, 0x2c, 0x46, 0x01, 0x12, 0x1d, 0x39, 0xba, 0xd3, 0x1e, 0x8c,
    0xc4, 0x04, 0x5d, 0x92, 0x01, 0x3e, 0x66, 0x01, 0x26, 0x41, 0x01, 0x10, 0x1b, 0x2f, 0xc7, 0xd9,
    0x0e, 0x91, 0xc4, 0x01, 0x58, 0x8e, 0x01, 0x39, 0x62, 0x01, 0x24, 0x3e, 0x01, 0x0f, 0x1a, 0x1a,
    0xdb, 0xe5, 0x05, 0x9b, 0xcf, 0x01, 0x5e, 0x97, 0x01, 0x3c, 0x68, 0x01, 0x24, 0x3e, 0x01, 0x10,
    0x1c, 0xe9, 0x1d, 0xf8, 0x92, 0x2f, 0xdc, 0x2b, 0x34, 0x8c, 0x64, 0xa3, 0xe8, 0xb3, 0xa1, 0xde,
    0x3f, 0x8e, 0xcc, 0x25, 0x71, 0xae, 0x1a, 0x59, 0x89, 0x12, 0x44, 0x61, 0x55, 0xb5, 0xe6, 0x20,
    0x92, 0xd1, 0x07, 0x64, 0xa4, 0x03, 0x47, 0x79, 0x01, 0x2d, 0x4d, 0x01, 0x12, 0x1e, 0x41, 0xbb,
    0xe6, 0x14, 0x94, 0xcf, 0x02, 0x61, 0x9f, 0x01, 0x44, 0x74, 0x01, 0x28, 0x46, 0x01, 0x0e, 0x1d,
    0x28, 0xc2, 0xe3, 0x08, 0x93, 0xcc, 0x01, 0x5e, 0x9b, 0x01, 0x41, 0x70, 0x01, 0x27, 0x42, 0x01,
    0x0e, 0x1a, 0x10, 0xd0, 0xe4, 0x03, 0x97, 0xcf, 0x01, 0x62, 0xa0, 0x01, 0x43, 0x75, 0x01, 0x29,
    0x4a, 0x01, 0x11, 0x1f, 0x11, 0x26, 0x8c, 0x07, 0x22, 0x50, 0x01, 0x11, 0x1d, 0x25, 0x4b, 0x80,
    0x29, 0x4c, 0x80, 0x1a, 0x42, 0x74, 0x0c, 0x34, 0x5e, 0x02, 0x20, 0x37, 0x01, 0x0a, 0x10, 0x32,
    0x7f, 0x9a, 0x25, 0x6d, 0x98, 0x10, 0x52, 0x79, 0x05, 0x3b, 0x55, 0x01, 0x23, 0x36, 0x01, 0x0d,
    0x14, 0x28, 0x8e, 0xa7, 0x11, 0x6e, 0x9d, 0x02, 0x47, 0x70, 0x01, 0x2c, 0x48, 0x01, 0x1b, 0x2d,
    0x01, 0x0b, 0x11, 0x1e, 0xaf, 0xbc, 0x09, 0x7c, 0xa9, 0x01, 0x4a, 0x74, 0x01, 0x30, 0x4e, 0x01,
    0x1e, 0x31, 0x01, 0x0b, 0x12, 0x0a, 0xde, 0xdf, 0x02, 0x96, 0xc2, 0x01, 0x53, 0x80, 0x01, 0x30,
    0x4f, 0x01, 0x1b, 0x2d, 0x01, 0x0b, 0x11, 0x24, 0x29, 0xeb, 0x1d, 0x24, 0xc1, 0x0a, 0x1b, 0x6f,
    0x55, 0xa5, 0xde, 0xb1, 0xa2, 0xd7, 0x6e, 0x87, 0xc3, 0x39, 0x71, 0xa8, 0x17, 0x53, 0x78, 0x0a,
    0x31, 0x3d, 0x55, 0xbe, 0xdf, 0x24, 0x8b, 0xc8, 0x05, 0x5a, 0x92, 0x01, 0x3c, 0x67, 0x01, 0x26,
    0x41, 0x01, 0x12, 0x1e, 0x48, 0xca, 0xdf, 0x17, 0x8d, 0xc7, 0x02, 0x56, 0x8c, 0x01, 0x38, 0x61,
    0x01, 0x24, 0x3d, 0x01, 0x10, 0x1b, 0x37, 0xda, 0xe1, 0x0d, 0x91, 0xc8, 0x01, 0x56, 0x8d, 0x01,
    0x39, 0x63, 0x01, 0x23, 0x3d, 0x01, 0x0d, 0x16, 0x0f, 0xeb, 0xd4, 0x01, 0x84, 0xb8, 0x01, 0x54,
    0x8b, 0x01, 0x39, 0x61, 0x01, 0x22, 0x38, 0x01, 0x0e, 0x17, 0xb5, 0x15, 0xc9, 0x3d, 0x25, 0x7b,
    0x0a, 0x26, 0x47, 0x2f, 0x6a, 0xac, 0x5f, 0x68, 0xad, 0x2a, 0x5d, 0x9f, 0x12, 0x4d, 0x83, 0x04,
    0x32, 0x51, 0x01, 0x11, 0x17, 0x3e, 0x93, 0xc7, 0x2c, 0x82, 0xbd, 0x1c, 0x66, 0x9a, 0x12, 0x4b,
    0x73, 0x02, 0x2c, 0x41, 0x01, 0x0c, 0x13, 0x37, 0x99, 0xd2, 0x18, 0x82, 0xc2, 0x03, 0x5d, 0x92,
    0x01, 0x3d, 0x61, 0x01, 0x1f, 0x32, 0x01, 0x0a, 0x10, 0x31, 0xba, 0xdf, 0x11, 0x94, 0xcc, 0x01,
    0x60, 0x8e, 0x01, 0x35, 0x53, 0x01, 0x1a, 0x2c, 0x01, 0x0b, 0x11, 0x0d, 0xd9, 0xd4, 0x02, 0x88,
    0xb4, 0x01, 0x4e, 0x7c, 0x01, 0x32, 0x53, 0x01, 0x1d, 0x31, 0x01, 0x0e, 0x17, 0xc5, 0x0d, 0xf7,
    0x52, 0x11, 0xde, 0x19, 0x11, 0xa2, 0x7e, 0xba, 0xf7, 0xea, 0xbf, 0xf3, 0xb0, 0xb1, 0xea, 0x68,
    0x9e, 0xdc, 0x42, 0x80, 0xba, 0x37, 0x5a, 0x89, 0x6f, 0xc5, 0xf2, 0x2e, 0x9e, 0xdb, 0x09, 0x68,
    0xab, 0x02, 0x41, 0x7d, 0x01, 0x2c, 0x50, 0x01, 0x11, 0x5b, 0x68, 0xd0, 0xf5, 0x27, 0xa8, 0xe0,
    0x03, 0x6d, 0xa2, 0x01, 0x4f, 0x7c, 0x01, 0x32, 0x66, 0x01, 0x2b, 0x66, 0x54, 0xdc, 0xf6, 0x1f,
    0xb1, 0xe7, 0x02, 0x73, 0xb4, 0x01, 0x4f, 0x86, 0x01, 0x37, 0x4d, 0x01, 0x3c, 0x4f, 0x2b, 0xf3,
    0xf0, 0x08, 0xb4, 0xd9, 0x01, 0x73, 0xa6, 0x01, 0x54, 0x79, 0x01, 0x33, 0x43, 0x01, 0x10, 0x06,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xc0, 0x80, 0x40, 0x02, 0xad, 0x22, 0x07, 0x91, 0x55, 0x07, 0xa6, 0x3f, 0x07, 0x5e, 0x42, 0x08,
    0x40, 0x2e, 0x11, 0x51, 0x1f, 0x19, 0x1d, 0x1e, 0xeb, 0xa2, 0x24, 0xff, 0x22, 0x03, 0x95, 0x90,
    0x09, 0x66, 0xbb, 0xe1, 0xef, 0xb7, 0x77, 0x60, 0x29, 0x21, 0x10, 0x4d, 0x4a, 0x8e, 0x8e, 0xac,
    0xaa, 0xee, 0xf7, 0x32, 0x7e, 0x7b, 0xdd, 0xe2, 0x41, 0x20, 0x12, 0x90, 0xa2, 0xc2, 0x29, 0x33,
    0x62, 0x84, 0x44, 0x12, 0xa5, 0xd9, 0xc4, 0x2d, 0x28, 0x4e, 0xad, 0x50, 0x13, 0xb0, 0xf0, 0xc1,
    0x40, 0x23, 0x2e, 0xdd, 0x87, 0x26, 0xc2, 0xf8, 0x79, 0x60, 0x55, 0x1d, 0xc7, 0x7a, 0x8d, 0x93,
    0x3f, 0x9f, 0x94, 0x85, 0x76, 0x79, 0x68, 0x72, 0xae, 0x49, 0x57, 0x5c, 0x29, 0x53, 0x52, 0x63,
    0x32, 0x35, 0x27, 0x27, 0xb1, 0x3a, 0x3b, 0x44, 0x1a, 0x3f, 0x34, 0x4f, 0x19, 0x11, 0x0e, 0x0c,
    0xde, 0x22, 0x1e, 0x48, 0x10, 0x2c, 0x3a, 0x20, 0x0c, 0x0a, 0x07, 0x06, 0x20, 0x40, 0x60, 0x80,
    0xe0, 0x90, 0xc0, 0xa8, 0xc0, 0xb0, 0xc0, 0xc6, 0xc6, 0xf5, 0xd8, 0x88, 0x8c, 0x94, 0xa0, 0xb0,
    0xc0, 0xe0, 0xea, 0xea, 0xf0, 0x80, 0xd8, 0x80, 0xb0, 0xa0, 0xb0, 0xb0, 0xc0, 0xc6, 0xc6, 0xd0,
    0xd0, 0x88, 0x8c, 0x94, 0xa0, 0xb0, 0xc0, 0xe0, 0xea, 0xea, 0xf0, 0x80, 0x80, 0x40, 0x60, 0x70,
    0x40, 0x40, 0x60, 0x40, 0x80, 0x80, 0x40, 0x60, 0x70, 0x40, 0x40, 0x60, 0x40, 0xa0, 0x80, 0xa0,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x78, 0x07, 0x4c, 0xb0, 0xd0, 0x7e, 0x1c, 0x36, 0x67, 0x30, 0x0c, 0x9a, 0x9b, 0x8b, 0x5a, 0x22,
    0x75, 0x77, 0x43, 0x06, 0x19, 0xcc, 0xf3, 0x9e, 0x0d, 0x15, 0x60, 0x61, 0x05, 0x2c, 0x83, 0xb0,
    0x8b, 0x30, 0x44, 0x61, 0x53, 0x05, 0x2a, 0x9c, 0x6f, 0x98, 0x1a, 0x31, 0x98, 0x50, 0x05, 0x3a,
    0xb2, 0x4a, 0x53, 0x21, 0x3e, 0x91, 0x56, 0x05, 0x20, 0x9a, 0xc0, 0xa8, 0x0e, 0x16, 0xa3, 0x55,
    0x05, 0x20, 0x9c, 0xd8, 0x94, 0x13, 0x1d, 0x49, 0x4d, 0x07, 0x40, 0x74, 0x84, 0x7a, 0x25, 0x7e,
    0x78, 0x65, 0x15, 0x6b, 0xb5, 0xc0, 0x67, 0x13, 0x43, 0x7d, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// loop filter value based on qp index look up table calculated based on c-model formula
static const uint8_t CODECHAL_ENCODE_VP9_LF_VALUE_QP_LOOKUP[CODEC_VP9_QINDEX_RANGE] = {
    0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0a,
    0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e,
    0x0e, 0x0f, 0x0f, 0x0f, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12, 0x13, 0x13,
    0x13, 0x14, 0x14, 0x14, 0x15, 0x15, 0x15, 0x16, 0x16, 0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x19,
    0x19, 0x19, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c, 0x1d, 0x1d, 0x1d, 0x1e, 0x1e, 0x1e, 0x1f,
    0x1f, 0x20, 0x20, 0x20, 0x21, 0x21, 0x22, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24, 0x25, 0x25, 0x25,
    0x26, 0x26, 0x27, 0x27, 0x27, 0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a, 0x2b, 0x2b, 0x2b, 0x2c,
    0x2c, 0x2d, 0x2d, 0x2d, 0x2e, 0x2e, 0x2f, 0x2f, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x31, 0x32, 0x32,
    0x32, 0x33, 0x33, 0x34, 0x34, 0x34, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36, 0x37, 0x37, 0x37, 0x38,
    0x38, 0x39, 0x39, 0x39, 0x3a, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b, 0x3b, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d,
    0x3d, 0x3e, 0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
    0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f
};

MOS_STATUS CodechalEncodeVp9::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeVp9, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

CodechalEncodeVp9::CodechalEncodeVp9(
    CodechalHwInterface*    hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalEncoderState(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(hwInterface);
    m_hwInterface = hwInterface;
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetOsInterface());
    m_osInterface = m_hwInterface->GetOsInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetMfxInterface());
    m_mfxInterface = m_hwInterface->GetMfxInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetHcpInterface());
    m_hcpInterface = m_hwInterface->GetHcpInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetHucInterface());
    m_hucInterface = m_hwInterface->GetHucInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetVdencInterface());
    m_vdencInterface = m_hwInterface->GetVdencInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetMiInterface());
    m_miInterface = m_hwInterface->GetMiInterface();
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_ASSERT(m_hwInterface->GetRenderInterface()->m_stateHeapInterface);
    m_stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_VP9_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_ENCODE_VP9_INIT_DSH_SIZE;

    //Fixme: NO dynamic scaling support yet
    //m_dysKernelState  = MHW_KERNEL_STATE();
    m_meKernelState  = MHW_KERNEL_STATE();

    for (uint8_t i = 0; i < CODECHAL_ENCODE_VP9_MBENC_IDX_NUM; i++)
    {
        m_mbEncKernelStates[i] = MHW_KERNEL_STATE();
    }
    for (uint8_t i = 0; i < CODECHAL_ENCODE_VP9_BRC_IDX_NUM; i++)
    {
        m_brcKernelStates[i] = MHW_KERNEL_STATE();
    }

    MOS_ZeroMemory(&m_picIdx, sizeof(m_picIdx));
    MOS_ZeroMemory(&m_refList, sizeof(m_refList));
    MOS_ZeroMemory(&m_s4XMemvDataBuffer, sizeof(m_s4XMemvDataBuffer));
    MOS_ZeroMemory(&m_mbEncI32x32BindingTable, sizeof(m_mbEncI32x32BindingTable));
    MOS_ZeroMemory(&m_mbEncI16x16BindingTable, sizeof(m_mbEncI16x16BindingTable));
    MOS_ZeroMemory(&m_mbEncPBindingTable, sizeof(m_mbEncPBindingTable));
    MOS_ZeroMemory(&m_mbEncTxBindingTable, sizeof(m_mbEncTxBindingTable));
    MOS_ZeroMemory(&m_s4XMeDistortionBuffer, sizeof(m_s4XMeDistortionBuffer));
    MOS_ZeroMemory(&m_brcBuffers, sizeof(m_brcBuffers));

    /* VP9 uses a CM based down scale kernel */
    m_useCmScalingKernel = true;
    m_interlacedFieldDisabled = true;
    m_firstField = true;
   /* No field pictures in VP8 */
    m_verticalLineStride = CODECHAL_VLINESTRIDE_FRAME;
    m_verticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;

    m_codecGetStatusReportDefined = true;
}

CodechalEncodeVp9::~CodechalEncodeVp9()
{
    FreeResources();
}

void CodechalEncodeVp9::PutDataForCompressedHdr(CompressedHeader* pCompressedHdr, uint32_t bit, uint32_t prob, uint32_t binIdx)
{
    pCompressedHdr[binIdx].fields.valid = 1;
    pCompressedHdr[binIdx].fields.bin_probdiff = 1;
    pCompressedHdr[binIdx].fields.bin = bit;
    pCompressedHdr[binIdx].fields.prob = (prob == 128) ? 0 : 1;
}

MOS_STATUS CodechalEncodeVp9::ConstructPicStateBatchBuf(
    PMOS_RESOURCE                   pPicStateBuffer)
{
    MhwMiInterface                  *pCommonMiInterface;
    MHW_VDBOX_VP9_ENCODE_PIC_STATE  Vp9PicState;
    MOS_COMMAND_BUFFER              ConstructedCmdBuf;
    MOS_LOCK_PARAMS                 LockFlagsWriteOnly;
    uint8_t*                        pbData = NULL;
    uint32_t                        i;
    MOS_GPU_CONTEXT                 curGpuContext;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());

    CODECHAL_ENCODE_CHK_NULL_RETURN(pPicStateBuffer);

    pCommonMiInterface = m_hwInterface->GetMiInterface();

    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    LockFlagsWriteOnly.WriteOnly = 1;
    pbData = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, pPicStateBuffer, &LockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

    // pfnAddMiBatchBufferEnd also adds MediaStateFlush commands in render context, but picState is used later by PAK. Switching context to video before adding BB end so thse DWs are not added
    curGpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);
    m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VIDEO);

    // HCP_VP9_PIC_STATE
    MOS_ZeroMemory(&Vp9PicState, sizeof(Vp9PicState));

    Vp9PicState.pVp9PicParams = m_vp9PicParams;
    Vp9PicState.pVp9SeqParams = m_vp9SeqParams;
    Vp9PicState.ppVp9RefList = &(m_refList[0]);
    Vp9PicState.PrevFrameParams.fields.KeyFrame = PakMode.Hw.m_prevFrameInfo.KeyFrame;
    Vp9PicState.PrevFrameParams.fields.IntraOnly = PakMode.Hw.m_prevFrameInfo.IntraOnly;
    Vp9PicState.PrevFrameParams.fields.Display = PakMode.Hw.m_prevFrameInfo.ShowFrame;
    Vp9PicState.dwPrevFrmWidth = PakMode.Hw.m_prevFrameInfo.FrameWidth;
    Vp9PicState.dwPrevFrmHeight = PakMode.Hw.m_prevFrameInfo.FrameHeight;
    Vp9PicState.ucTxMode = m_ucTxMode;
    //Vp9PicState.uiMaxBitRate                     = m_vp9SeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    //Vp9PicState.uiMinBitRate                     = m_vp9SeqParams->MinBitRate * CODECHAL_ENCODE_BRC_KBPS;


    for (i = 0; i < CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES; i++)
    {
        Vp9PicState.bNonFirstPassFlag = (i != 0) ? true : false;

        ConstructedCmdBuf.pCmdBase   = (uint32_t*)pbData;
        ConstructedCmdBuf.pCmdPtr    = (uint32_t*)(pbData + i * CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS);
        ConstructedCmdBuf.iOffset    = 0;
        ConstructedCmdBuf.iRemaining = CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS;

	eStatus = m_hcpInterface->AddHcpVp9PicStateEncCmd(&ConstructedCmdBuf, nullptr, &Vp9PicState);

        // After adding pic State cmds in above function, pCmdPtr is not at the end of the picState Buffer, so adjusting it.
	//Fixme: Not in i916 driver, 
        //ConstructedCmdBuf.pCmdPtr    = (uint32_t*)(pbData + (i+1) * CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS) - 1;  //-1 to go back one uint32_t where BB end will be added

        CODECHAL_ENCODE_CHK_STATUS_RETURN(pCommonMiInterface->AddMiBatchBufferEnd(&ConstructedCmdBuf, NULL));
    }

    // Switch back to current context
    m_osInterface->pfnSetGpuContext(m_osInterface, curGpuContext);

    if (pbData)
    {

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            pPicStateBuffer);
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::ConstructPakInsertObjBatchBuf(
    PMOS_RESOURCE                   pakInsertObjBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!pakInsertObjBuffer)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
   
    CODECHAL_ENCODE_ASSERT(m_numNalUnit == 1);

    uint32_t nalUnitSize =  m_nalUnitParams[0]->uiSize;
    uint32_t nalUnitOffset = m_nalUnitParams[0]->uiOffset;
    CODECHAL_ENCODE_ASSERT(nalUnitSize > 0 && nalUnitSize < CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER);  

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;
    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, pakInsertObjBuffer, &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
    MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
    pakInsertObjectParams.bEmulationByteBitsInsert = false;
    pakInsertObjectParams.uiSkipEmulationCheckCount = m_nalUnitParams[0]->uiSkipEmulationCheckCount;
    pakInsertObjectParams.pBsBuffer = &m_bsBuffer;
    pakInsertObjectParams.dwBitSize = nalUnitSize * 8;
    pakInsertObjectParams.dwOffset = nalUnitOffset;
    pakInsertObjectParams.bEndOfSlice = pakInsertObjectParams.bLastHeader = true;
    //Fixme: i965 set it as false?
    //pakInsertObjectParams.bEndOfSlice = false;

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase = (uint32_t*)data;
    constructedCmdBuf.pCmdPtr = (uint32_t*)data;
    constructedCmdBuf.iOffset = 0;
    constructedCmdBuf.iRemaining = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&constructedCmdBuf, &pakInsertObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&constructedCmdBuf, nullptr));

    if (data)
    {
        m_osInterface->pfnUnlockResource(
            m_osInterface,
            pakInsertObjBuffer);
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::CtxBufDiffInit(
    uint8_t *ctxBuffer,
    bool setToKey)
{
    int32_t i, j;
    uint32_t byteCnt = CODEC_VP9_INTER_PROB_OFFSET;
    //inter mode probs. have to be zeros for Key frame
    for (i = 0; i < CODEC_VP9_INTER_MODE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_INTER_MODES - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultInterModeProbs[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //switchable interprediction probs
    for (i = 0; i < CODEC_VP9_SWITCHABLE_FILTERS + 1; i++)
    {
        for (j = 0; j < CODEC_VP9_SWITCHABLE_FILTERS - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSwitchableInterpProb[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //intra inter probs
    for (i = 0; i < CODEC_VP9_INTRA_INTER_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultIntraInterProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //comp inter probs
    for (i = 0; i < CODEC_VP9_COMP_INTER_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompInterProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //single ref probs
    for (i = 0; i < CODEC_VP9_REF_CONTEXTS; i++)
    {
        for (j = 0; j < 2; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultSingleRefProb[i][j];
            }
            else
            {
                //zeros for key frame
                byteCnt++;
            }
        }
    }
    //comp ref probs
    for (i = 0; i < CODEC_VP9_REF_CONTEXTS; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultCompRefProb[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //y mode probs
    for (i = 0; i < CODEC_VP9_BLOCK_SIZE_GROUPS; i++)
    {
        for (j = 0; j < CODEC_VP9_INTRA_MODES - 1; j++)
        {
            if (!setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultIFYProb[i][j];
            }
            else
            {
                //zeros for key frame, since HW will not use this buffer, but default right buffer.
                byteCnt++;
            }
        }
    }
    //partition probs, key & intra-only frames use key type, other inter frames use inter type
    for (i = 0; i < CODECHAL_VP9_PARTITION_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_PARTITION_TYPES - 1; j++)
        {
            if (setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultKFPartitionProb[i][j];
            }
            else
            {
                ctxBuffer[byteCnt++] = DefaultPartitionProb[i][j];
            }
        }
    }
    //nmvc joints
    for (i = 0; i < (CODEC_VP9_MV_JOINTS - 1); i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.joints[i];
        }
        else
        {
            //zeros for key frame
            byteCnt++;
        }
    }
    //nmvc comps
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].sign;
            for (j = 0; j < (CODEC_VP9_MV_CLASSES - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].classes[j];
            }
            for (j = 0; j < (CODECHAL_VP9_CLASS0_SIZE - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0[j];
            }
            for (j = 0; j < CODECHAL_VP9_MV_OFFSET_BITS; j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].bits[j];
            }
        }
        else
        {
            byteCnt += 1;
            byteCnt += (CODEC_VP9_MV_CLASSES - 1);
            byteCnt += (CODECHAL_VP9_CLASS0_SIZE - 1);
            byteCnt += (CODECHAL_VP9_MV_OFFSET_BITS);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            for (j = 0; j < CODECHAL_VP9_CLASS0_SIZE; j++)
            {
                for (int32_t k = 0; k < (CODEC_VP9_MV_FP_SIZE - 1); k++)
                {
                    ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_fp[j][k];
                }
            }
            for (j = 0; j < (CODEC_VP9_MV_FP_SIZE - 1); j++)
            {
                ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].fp[j];
            }
        }
        else
        {
            byteCnt += (CODECHAL_VP9_CLASS0_SIZE * (CODEC_VP9_MV_FP_SIZE - 1));
            byteCnt += (CODEC_VP9_MV_FP_SIZE - 1);
        }
    }
    for (i = 0; i < 2; i++)
    {
        if (!setToKey)
        {
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].class0_hp;
            ctxBuffer[byteCnt++] = DefaultNmvContext.comps[i].hp;
        }
        else
        {
            byteCnt += 2;
        }
    }

    //47 bytes of zeros
    byteCnt += 47;

    //uv mode probs
    for (i = 0; i < CODEC_VP9_INTRA_MODES; i++)
    {
        for (j = 0; j < CODEC_VP9_INTRA_MODES - 1; j++)
        {
            if (setToKey)
            {
                ctxBuffer[byteCnt++] = DefaultKFUVModeProb[i][j];
            }
            else
            {
                ctxBuffer[byteCnt++] = DefaultIFUVProbs[i][j];
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeVp9::ContextBufferInit(
    uint8_t *ctxBuffer,
    bool setToKey)
{

    MOS_ZeroMemory(ctxBuffer, CODEC_VP9_SEG_PROB_OFFSET);

    int32_t i, j;
    uint32_t byteCnt = 0;
    //TX probs
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 3; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p8x8[i][j];
        }
    }
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 2; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p16x16[i][j];
        }
    }
    for (i = 0; i < CODEC_VP9_TX_SIZE_CONTEXTS; i++)
    {
        for (j = 0; j < CODEC_VP9_TX_SIZES - 1; j++)
        {
            ctxBuffer[byteCnt++] = DefaultTxProbs.p32x32[i][j];
        }
    }

    //52 bytes of zeros
    byteCnt += 52;

    uint8_t blocktype = 0;
    uint8_t reftype = 0;
    uint8_t coeffbands = 0;
    uint8_t unConstrainedNodes = 0;
    uint8_t prevCoefCtx = 0;
    //coeff probs
    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs4x4[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefPprobs8x8[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs16x16[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    for (blocktype = 0; blocktype < CODEC_VP9_BLOCK_TYPES; blocktype++)
    {
        for (reftype = 0; reftype < CODEC_VP9_REF_TYPES; reftype++)
        {
            for (coeffbands = 0; coeffbands < CODEC_VP9_COEF_BANDS; coeffbands++)
            {
                uint8_t numPrevCoeffCtxts = (coeffbands == 0) ? 3 : CODEC_VP9_PREV_COEF_CONTEXTS;
                for (prevCoefCtx = 0; prevCoefCtx < numPrevCoeffCtxts; prevCoefCtx++)
                {
                    for (unConstrainedNodes = 0; unConstrainedNodes < CODEC_VP9_UNCONSTRAINED_NODES; unConstrainedNodes++)
                    {
                        ctxBuffer[byteCnt++] = DefaultCoefProbs32x32[blocktype][reftype][coeffbands][prevCoefCtx][unConstrainedNodes];
                    }
                }
            }
        }
    }

    //16 bytes of zeros
    byteCnt += 16;

    // mb skip probs
    for (i = 0; i < CODEC_VP9_MBSKIP_CONTEXTS; i++)
    {
        ctxBuffer[byteCnt++] = DefaultMbskipProbs[i];
    }

    // populate prob values which are different between Key and Non-Key frame
    CtxBufDiffInit(ctxBuffer, setToKey);

    //skip Seg tree/pred probs, updating not done in this function.
    byteCnt = CODEC_VP9_SEG_PROB_OFFSET;
    byteCnt += 7;
    byteCnt += 3;

    //28 bytes of zeros
    for (i = 0; i < 28; i++)
    {
        ctxBuffer[byteCnt++] = 0;
    }

    //Just a check.
    if (byteCnt > CODEC_VP9_PROB_MAX_NUM_ELEM)
    {
        CODECHAL_PUBLIC_ASSERTMESSAGE("Error: FrameContext array out-of-bounds, byteCnt = %d!\n", byteCnt);
        return MOS_STATUS_NO_SPACE;
    }
    else
    {
        return MOS_STATUS_SUCCESS;
    }
}

MOS_STATUS CodechalEncodeVp9::RefreshFrameInternalBuffers()
{
    uint8_t*                                pData;
    CompressedHeader*  pCompressedHdr = NULL;
    MOS_LOCK_PARAMS                         LockFlagsWriteOnly;
    uint32_t                                i;
    uint8_t                                 ucFrameCtxId;
    bool                                    bIsScaling, bKeyFrame, bClearAll, bClearSpecified, bResetSegIdBuf;
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS, eStatus1 = MOS_STATUS_SUCCESS, eStatus2 = MOS_STATUS_SUCCESS;
 
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(m_vp9PicParams->PicFlags.fields.refresh_frame_context == 0);

    bKeyFrame = !m_vp9PicParams->PicFlags.fields.frame_type;
    bIsScaling = (m_oriFrameWidth == PakMode.Hw.m_prevFrameInfo.FrameWidth) &&
        (m_oriFrameHeight == PakMode.Hw.m_prevFrameInfo.FrameHeight) ? false : true;

    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    LockFlagsWriteOnly.WriteOnly = 1;

    bClearAll = (bKeyFrame ||
        m_vp9PicParams->PicFlags.fields.error_resilient_mode ||
        (m_vp9PicParams->PicFlags.fields.reset_frame_context == 3 &&
        m_vp9PicParams->PicFlags.fields.intra_only));

    bClearSpecified = (m_vp9PicParams->PicFlags.fields.reset_frame_context == 2 &&
        m_vp9PicParams->PicFlags.fields.intra_only);

    bResetSegIdBuf = bKeyFrame ||
        bIsScaling ||
        m_vp9PicParams->PicFlags.fields.error_resilient_mode ||
        m_vp9PicParams->PicFlags.fields.intra_only;

    if (bResetSegIdBuf)
    {
        pData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &PakMode.Hw.resSegmentIdBuffer,
            &LockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

        MOS_ZeroMemory(pData, m_dwPicSizeInSB * CODECHAL_CACHELINE_SIZE);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
            m_osInterface,
            &PakMode.Hw.resSegmentIdBuffer));
    }

    //refresh inter probs in needed frame context buffers
    //CODEC_VP9_NUM_CONTEXTS
    for (i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
    {
        if (bClearAll || (bClearSpecified && i == m_vp9PicParams->PicFlags.fields.frame_context_idx))
        {
            pData = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                &PakMode.Hw.resProbBuffer[i],
                &LockFlagsWriteOnly);
            CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

            eStatus1 = ContextBufferInit(pData, bKeyFrame || m_vp9PicParams->PicFlags.fields.intra_only);

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &PakMode.Hw.resProbBuffer[i]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus1);

            PakMode.Hw.bClearAllToKey[i] = bKeyFrame || m_vp9PicParams->PicFlags.fields.intra_only;
            if (i == 0)  //reset this flag when Ctx buffer 0 is cleared.
            {
                PakMode.Hw.bPreCtx0InterProbSaved = false;
            }
        }
        else if (PakMode.Hw.bClearAllToKey[i]) // this buffer is inside inter frame, but its interProb has not been init to default inter type data.
        {
            pData = (uint8_t*)m_osInterface->pfnLockResource(
                m_osInterface,
                &PakMode.Hw.resProbBuffer[i],
                &LockFlagsWriteOnly);
            CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

            if (m_vp9PicParams->PicFlags.fields.intra_only && i == 0) // this buffer is used as intra_only context, do not need to set interprob to be inter type.
            {
                eStatus1 = CtxBufDiffInit(pData, true);
            }
            else // set interprob to be inter type.
            {
                eStatus1 = CtxBufDiffInit(pData, false);
                PakMode.Hw.bClearAllToKey[i] = false;
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &PakMode.Hw.resProbBuffer[i]));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus1);
        }
        else if (i == 0) // this buffer do not need to clear in current frame, also it has not been cleared to key type in previous frame.            
        {                // in this case, only context buffer 0 will be temporally overwritten.
            if (m_vp9PicParams->PicFlags.fields.intra_only)
            {
                pData = (uint8_t*)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &PakMode.Hw.resProbBuffer[i],
                    &LockFlagsWriteOnly);
                CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

                if (!PakMode.Hw.bPreCtx0InterProbSaved) // only when non intra-only -> intra-only need save InterProb, otherwise leave saved InterProb unchanged.
                {
                    //save current interprob
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(PakMode.Hw.ucPreCtx0InterProbSaved, CODECHAL_VP9_INTER_PROB_SIZE, pData + CODEC_VP9_INTER_PROB_OFFSET, CODECHAL_VP9_INTER_PROB_SIZE));
                    PakMode.Hw.bPreCtx0InterProbSaved = true;
                }
                eStatus1 = CtxBufDiffInit(pData, true);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &PakMode.Hw.resProbBuffer[i]));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(eStatus1);
            }
            else if (PakMode.Hw.bPreCtx0InterProbSaved)
            {
                pData = (uint8_t*)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &PakMode.Hw.resProbBuffer[i],
                    &LockFlagsWriteOnly);
                CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
                //reload former interprob                
                CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(pData + CODEC_VP9_INTER_PROB_OFFSET, CODECHAL_VP9_INTER_PROB_SIZE, PakMode.Hw.ucPreCtx0InterProbSaved, CODECHAL_VP9_INTER_PROB_SIZE));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                    m_osInterface,
                    &PakMode.Hw.resProbBuffer[i]));

                PakMode.Hw.bPreCtx0InterProbSaved = false;
            }
        }
    }

    // compressed header
    uint32_t index = 0;
    pCompressedHdr = (CompressedHeader*)MOS_AllocAndZeroMemory(sizeof(CompressedHeader)* (PAK_COMPRESSED_HDR_SYNTAX_ELEMS + 1));
    CODECHAL_ENCODE_CHK_NULL_RETURN(pCompressedHdr);
  
    if (!m_vp9PicParams->PicFlags.fields.LosslessFlag)
    {
        if (m_ucTxMode == CODEC_VP9_TX_SELECTABLE)
        {
            PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_TX_MODE_IDX);
            PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_TX_MODE_IDX + 1);
            PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_TX_MODE_SELECT_IDX);
        }
        else if (m_ucTxMode == CODEC_VP9_TX_32X32)
        {
            PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_TX_MODE_IDX);
            PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_TX_MODE_IDX + 1);
            PutDataForCompressedHdr(pCompressedHdr, 0, 128, PAK_TX_MODE_SELECT_IDX);
        }
        else
        {
            PutDataForCompressedHdr(pCompressedHdr, (m_ucTxMode & 0x02) >> 1, 128, PAK_TX_MODE_IDX);
            PutDataForCompressedHdr(pCompressedHdr, (m_ucTxMode & 0x01), 128, PAK_TX_MODE_IDX + 1);
        }

	if (m_ucTxMode == CODEC_VP9_TX_SELECTABLE)
        {
            index = PAK_TX_8x8_PROB_IDX;
            for (auto i = 0; i < 2; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 2;
            }

            index = PAK_TX_16x16_PROB_IDX;
            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                    index += 2;
                }
            }

            index = PAK_TX_32x32_PROB_IDX;
            for (auto i = 0; i < 2; i++)
            {
                for (auto j = 0; j < 3; j++)
                {
                    PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                    index += 2;
                }
            }
        }
    }

    for (auto coeffSize = 0; coeffSize < 4; coeffSize++)
    {
        if (coeffSize > m_ucTxMode)
        {
            continue;
        }

        switch (coeffSize)
        {
        case 0: index = PAK_TX_4x4_COEFF_PROB_IDX;
            break;
        case 1: index = PAK_TX_8x8_COEFF_PROB_IDX;
            break;
        case 2: index = PAK_TX_16x16_COEFF_PROB_IDX;
            break;
        case 3: index = PAK_TX_32x32_COEFF_PROB_IDX;
            break;
        }

        PutDataForCompressedHdr(pCompressedHdr, 0, 128, index);
    }

    PutDataForCompressedHdr(pCompressedHdr, 0, 252, PAK_SKIP_CONTEXT_IDX);
    PutDataForCompressedHdr(pCompressedHdr, 0, 252, PAK_SKIP_CONTEXT_IDX + 2);
    PutDataForCompressedHdr(pCompressedHdr, 0, 252, PAK_SKIP_CONTEXT_IDX + 4);

    if (m_vp9PicParams->PicFlags.fields.frame_type != 0 && !m_vp9PicParams->PicFlags.fields.intra_only)
    {
        index = PAK_INTER_MODE_CTX_IDX;
        for (auto i = 0; i < 7; i++)
        {
            for (auto j = 0; j < 3; j++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 2;
            }
        }

	if (m_vp9PicParams->PicFlags.fields.mcomp_filter_type == CODEC_VP9_SWITCHABLE_FILTERS)
        {
            index = PAK_SWITCHABLE_FILTER_CTX_IDX;
            for (auto i = 0; i < 4; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                    index += 2;
                }
            }
        }

        index = PAK_INTRA_INTER_CTX_IDX;
        for (auto i = 0; i < 4; i++)
        {
            PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
            index += 2;
        }

        bool allow_comp = !(
            (m_vp9PicParams->RefFlags.fields.LastRefSignBias && m_vp9PicParams->RefFlags.fields.GoldenRefSignBias && m_vp9PicParams->RefFlags.fields.AltRefSignBias) ||
            (!m_vp9PicParams->RefFlags.fields.LastRefSignBias && !m_vp9PicParams->RefFlags.fields.GoldenRefSignBias && !m_vp9PicParams->RefFlags.fields.AltRefSignBias)
            );

        if (allow_comp)
        {
            if (m_vp9PicParams->PicFlags.fields.comp_prediction_mode == VP9_PRED_MODE_HYBRID)
            {                   
                PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_COMPOUND_PRED_MODE_IDX);
                PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_COMPOUND_PRED_MODE_IDX + 1);
            }
            else if (m_vp9PicParams->PicFlags.fields.comp_prediction_mode == VP9_PRED_MODE_COMPOUND) {
                PutDataForCompressedHdr(pCompressedHdr, 1, 128, PAK_COMPOUND_PRED_MODE_IDX);
                PutDataForCompressedHdr(pCompressedHdr, 0, 128, PAK_COMPOUND_PRED_MODE_IDX + 1);
            }
            else {
                PutDataForCompressedHdr(pCompressedHdr, 0, 128, PAK_COMPOUND_PRED_MODE_IDX);
            }
        }
	
	if (m_vp9PicParams->PicFlags.fields.comp_prediction_mode != VP9_PRED_MODE_COMPOUND)
        {
            index = PAK_SINGLE_REF_PRED_CTX_IDX;
            for (auto i = 0; i < 5; i++)
            {
                for (auto j = 0; j < 2; j++)
                {
                    PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                    index += 2;
                }
            }
        }

        if (m_vp9PicParams->PicFlags.fields.comp_prediction_mode != VP9_PRED_MODE_SINGLE)
        {
            index = PAK_CMPUND_PRED_CTX_IDX;
            for (auto i = 0; i < 5; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 2;
            }
        }

        index = PAK_INTRA_MODE_PROB_CTX_IDX;
        for (auto i = 0; i < 4; i++)
        {
            for (auto j = 0; j < 9; j++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 2;
            }
        }

        index = PAK_PARTITION_PROB_IDX;
        for (auto i = 0; i < 16; i++)
        {
            for (auto j = 0; j < 3; j++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 2;
            }
        }
        
        index = PAK_MVJOINTS_PROB_IDX;
        for (auto i = 0; i < 3; i++)
        {
            PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
            index += 8;
        }

        for (auto d = 0; d < 2; d++)
        {
            index = (d == 0) ? PAK_MVCOMP0_IDX : PAK_MVCOMP1_IDX;
            PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
            index += 8;
            for (auto i = 0; i < 10; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 8;
            }
            PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
            index += 8;
            for (auto i = 0; i < 10; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 8;
            }
        }

        for (auto d = 0; d < 2; d++)
        {
            index = (d == 0) ? PAK_MVFRAC_COMP0_IDX : PAK_MVFRAC_COMP1_IDX;
            for (auto i = 0; i < 3; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 8;
            }
            for (auto i = 0; i < 3; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 8;
            }
            for (auto i = 0; i < 3; i++)
            {
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 8;
            }
        }

        if (m_vp9PicParams->PicFlags.fields.allow_high_precision_mv)
        {
            for (auto d = 0; d < 2; d++)
            {
                index = (d == 0) ? PAK_MVHP_COMP0_IDX : PAK_MVHP_COMP1_IDX;
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
                index += 8;
                PutDataForCompressedHdr(pCompressedHdr, 0, 252, index);
            }
        }
    		
    }

    pData = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &PakMode.Hw.resCompressedHeaderBuffer,
        &LockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pData);

    for (i = 0; i < PAK_COMPRESSED_HDR_SYNTAX_ELEMS; i += 2)
    {
        pData[i>>1] = (pCompressedHdr[i + 1].value << 0x04) | (pCompressedHdr[i].value);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &PakMode.Hw.resCompressedHeaderBuffer));

finish:
    MOS_FreeMemory(pCompressedHdr);
    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::AllocateBrcResources()
{
    uint32_t                   i;
    uint32_t                   dwSize;
    MOS_ALLOC_GFXRES_PARAMS    AllocParamsForBufferLinear;
    uint8_t*                   pbData;
    MOS_LOCK_PARAMS            LockFlagsWriteOnly;
    MOS_STATUS                 eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // initiate allocation paramters and lock flags
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format = Format_Buffer;

    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    LockFlagsWriteOnly.WriteOnly = 1;

    // BRC history buffer
    dwSize = m_brcHistoryBufferSize;
    AllocParamsForBufferLinear.dwBytes = dwSize;
    AllocParamsForBufferLinear.pBufName = "BRC History Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resBrcHistoryBuffer));

    // BRC Constant Data Buffer
    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_BRC_CONSTANTSURFACE_SIZE;
    AllocParamsForBufferLinear.pBufName = "BRC Constant Data Buffer";
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resBrcConstantDataBuffer));

    PMHW_STATE_HEAP_INTERFACE stateHeapInterface;
    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    // MbEnc Curbe Write surface
    dwSize =
        stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() +
        MOS_ALIGN_CEIL(m_mbEncKernelStates[0].KernelParams.iCurbeLength,   // All MbEnc kernels have same curbe structure, so doesn't matter which kernelState curbe length is taken
        curbeAlignment);
    AllocParamsForBufferLinear.dwBytes = dwSize;
    AllocParamsForBufferLinear.pBufName = "MbEnc Curbe Write Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resBrcMbEncCurbeWriteBuffer));

    // Use a separate surface for PAVP heavy mode MbEnc DSH data
    dwSize =
        stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData() +
        MOS_ALIGN_CEIL(m_mbEncKernelStates[0].KernelParams.iCurbeLength,
        curbeAlignment);
    AllocParamsForBufferLinear.dwBytes = dwSize;
    AllocParamsForBufferLinear.pBufName = "MbEnc Curbe Encrypted Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resMbEncAdvancedDsh));

    pbData = (uint8_t*)m_osInterface->pfnLockResource(
        m_osInterface,
        &m_brcBuffers.resMbEncAdvancedDsh,
        &LockFlagsWriteOnly);

    if (pbData == NULL)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to Lock MbEnc Encrypted Curbe Buffer.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    MOS_ZeroMemory(pbData, dwSize);
    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_brcBuffers.resMbEncAdvancedDsh);

    // PicState Brc read buffer
    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS * CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES;
    AllocParamsForBufferLinear.pBufName = "BRC Pic State Read Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resPicStateBrcReadBuffer));

    // PicState Brc Write and Huc Read buffer
    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS * CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES;
    AllocParamsForBufferLinear.pBufName = "BRC Pic State Write Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resPicStateBrcWriteHucReadBuffer));

    // SegmentState Brc Read buffer
    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE_PER_PASS;
    AllocParamsForBufferLinear.pBufName = "BRC Segment State Read Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resSegmentStateBrcReadBuffer));

    // SegmentState Brc Write buffer
    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE_PER_PASS;
    AllocParamsForBufferLinear.pBufName = "BRC Segment State Write Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resSegmentStateBrcWriteBuffer));

    // BRC Bitstream Size Data buffer
    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER_SIZE;
    AllocParamsForBufferLinear.pBufName = "BRC Bitstream Size Data buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resBrcBitstreamSizeBuffer));

    // BRC HuC Data Buffer
    AllocParamsForBufferLinear.dwBytes = TEMP_CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE;
    AllocParamsForBufferLinear.pBufName = "BRC HuC Data Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resBrcHucDataBuffer));

    AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_BRC_MSDK_PAK_BUFFER_SIZE;
    AllocParamsForBufferLinear.pBufName = "BRC MSDK Buffer";

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBufferLinear,
        &m_brcBuffers.resBrcMsdkPakBuffer));

finish:
    return eStatus;

}

void CodechalEncodeVp9::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcHistoryBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcHistoryBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcConstantDataBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcConstantDataBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcMbEncCurbeWriteBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcMbEncCurbeWriteBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcMbEncCurbeWriteBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface, &m_brcBuffers.resMbEncAdvancedDsh);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resPicStateBrcReadBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resPicStateBrcReadBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resPicStateBrcWriteHucReadBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resPicStateBrcWriteHucReadBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resSegmentStateBrcReadBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resSegmentStateBrcReadBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resSegmentStateBrcWriteBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resSegmentStateBrcWriteBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcBitstreamSizeBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcBitstreamSizeBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcHucDataBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcHucDataBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcMsdkPakBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcMsdkPakBuffer);
    }

finish:
    return;
}

MOS_STATUS CodechalEncodeVp9::AllocateResources()
{
    uint32_t                   i, dwMaxTileNumber;
    uint32_t                   dwSize;
    uint32_t                   dwCurrNumUnitBlocks, dwLumaNumUnitBlocks, dwChromaNumUnitBlocks;
    uint32_t                   dwMaxPicWidthInSB, dwMaxPicHeightInSB, dwMaxPicSizeInSB;
    uint32_t                   dwMaxNumCuRecords;
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferLinear;
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBuffer2D;
    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBufferNV12;
    MOS_LOCK_PARAMS         LockFlagsWriteOnly;
    uint8_t*                   pbData = NULL;
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::AllocateResources());


    m_dwMaxPicWidth = m_frameWidth;
    m_dwMaxPicHeight = m_frameHeight;
    dwMaxPicWidthInSB = MOS_ROUNDUP_DIVIDE(m_dwMaxPicWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
    dwMaxPicHeightInSB = MOS_ROUNDUP_DIVIDE(m_dwMaxPicHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    dwMaxPicSizeInSB = dwMaxPicWidthInSB * dwMaxPicHeightInSB;
    dwMaxNumCuRecords = dwMaxPicSizeInSB * 64;

    // initiate allocation paramters and lock flags
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format = Format_Buffer;

    MOS_ZeroMemory(&AllocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBuffer2D.Type = MOS_GFXRES_2D;
    AllocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
    AllocParamsForBuffer2D.Format = Format_Buffer_2D;
    
    MOS_ZeroMemory(&AllocParamsForBufferNV12, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBufferNV12.Type     = MOS_GFXRES_2D;
    AllocParamsForBufferNV12.TileType = MOS_TILE_Y;
    AllocParamsForBufferNV12.Format   = Format_NV12;
 
    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    LockFlagsWriteOnly.WriteOnly = 1;

    // Allocate Ref Lists
    CodecHalAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);

    if (m_pakEnabled)
    {
        // Deblocking filter line buffer
        dwSize = dwMaxPicWidthInSB * 18 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "DeblockingFilterLineBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resDeblockingFilterLineBuffer));

        // Deblocking filter tile line buffer
        dwSize = dwMaxPicWidthInSB * 18 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "DeblockingFilterTileLineBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resDeblockingFilterTileLineBuffer));

        // Deblocking filter tile column buffer
        dwSize = dwMaxPicHeightInSB * 17 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "DeblockingFilterTileColumnBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resDeblockingFilterTileColumnBuffer));

        // Metadata Line buffer
        dwSize = dwMaxPicWidthInSB * 5 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "MetadataLineBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resMetadataLineBuffer));

        // Metadata Tile Line buffer
        dwSize = dwMaxPicWidthInSB * 5 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "MetadataTileLineBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resMetadataTileLineBuffer));

        // Metadata Tile Column buffer
        dwSize = dwMaxPicHeightInSB * 5 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "MetadataTileColumnBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resMetadataTileColumnBuffer));

        // Current MV temporal buffer
        dwSize = dwMaxPicSizeInSB * 9 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "CurrentMvTemporalBuffer";

        for (i = 0; i < 2; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBufferLinear,
                &PakMode.Hw.resMvTemporalBuffer[i]));
        }

        // Proabability buffer
        dwSize = 32 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "ProbabilityBuffer";

        for (i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBufferLinear,
                &PakMode.Hw.resProbBuffer[i]));
        }

        // Segment ID buffer
        dwSize = dwMaxPicSizeInSB * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "SegmentIdBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resSegmentIdBuffer));

        pbData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &PakMode.Hw.resSegmentIdBuffer,
            &LockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);
        
        MOS_ZeroMemory(pbData, dwSize);
        m_osInterface->pfnUnlockResource(m_osInterface, &PakMode.Hw.resSegmentIdBuffer);

        // Probability delta buffer
        dwSize = 29 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "ProbabilityDeltaBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resProbabilityDeltaBuffer));

        // Compressed header buffer
        dwSize = 32 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "CompressedHeaderBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resCompressedHeaderBuffer));

        // Probability counter buffer
        dwSize = 193 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "ProbabilityCounterBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resProbabilityCounterBuffer));

        // Tile record stream out buffer 
        dwSize = dwMaxPicSizeInSB * CODECHAL_CACHELINE_SIZE; // worst case: each SB is a tile
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "TileRecordStrmOutBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resTileRecordStrmOutBuffer));

        // CU statistics stream out buffer 
        dwSize = MOS_ALIGN_CEIL(dwMaxPicSizeInSB * 64 * 8, CODECHAL_CACHELINE_SIZE);
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "CuStatsStrmOutBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resCuStatsStrmOutBuffer));

        // HUC Prob DMEM buffer
        AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_HUC_DMEM_SIZE;
        AllocParamsForBufferLinear.pBufName = "HucProbDmemBuffer";
        for (i = 0; i < 2; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBufferLinear,
                &PakMode.Hw.resHucProbDmemBuffer[i]));
        }

        // Huc default prob buffer
        AllocParamsForBufferLinear.dwBytes = sizeof(CODECHAL_ENCODE_VP9_Keyframe_Default_Probs)+sizeof(CODECHAL_ENCODE_VP9_Inter_Default_Probs);
        AllocParamsForBufferLinear.pBufName = "HucDefaultProbBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucDefaultProbBuffer));
       
        pbData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &PakMode.Hw.resHucDefaultProbBuffer,
            &LockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

        MOS_SecureMemcpy(pbData, sizeof(CODECHAL_ENCODE_VP9_Keyframe_Default_Probs), 
                         CODECHAL_ENCODE_VP9_Keyframe_Default_Probs, sizeof(CODECHAL_ENCODE_VP9_Keyframe_Default_Probs));
        MOS_SecureMemcpy(pbData + sizeof(CODECHAL_ENCODE_VP9_Keyframe_Default_Probs), sizeof(CODECHAL_ENCODE_VP9_Inter_Default_Probs), 
                         CODECHAL_ENCODE_VP9_Inter_Default_Probs, sizeof(CODECHAL_ENCODE_VP9_Inter_Default_Probs));

        m_osInterface->pfnUnlockResource(m_osInterface, &PakMode.Hw.resHucDefaultProbBuffer);

        // Huc probability output buffer
        AllocParamsForBufferLinear.dwBytes = 32 * CODECHAL_CACHELINE_SIZE;
        AllocParamsForBufferLinear.pBufName = "HucProbabilityOutputBuffer";
      
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucProbOutputBuffer));
       
        // Huc VP9 pak insert uncompressed header
        AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;
        AllocParamsForBufferLinear.pBufName = "HucPakInsertUncompressedHeaderReadBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucPakInsertUncompressedHeaderReadBuffer));

        AllocParamsForBufferLinear.dwBytes = CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER;
        AllocParamsForBufferLinear.pBufName = "HucPakInsertUncompressedHeaderWriteBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucPakInsertUncompressedHeaderWriteBuffer));

        // Huc VP9 pak mmio buffer
        AllocParamsForBufferLinear.dwBytes = 4 * sizeof(uint32_t);
        AllocParamsForBufferLinear.pBufName = "HucPakMmioBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucPakMmioBuffer));

        // Huc VP9 Super Frame buffer
        AllocParamsForBufferLinear.dwBytes = TEMP_CODECHAL_ENCODE_VP9_BRC_SUPER_FRAME_BUFFER_SIZE;
        AllocParamsForBufferLinear.pBufName = "HucSuperFrameBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucSuperFrameBuffer));

        // Huc debug output buffer
        AllocParamsForBufferLinear.dwBytes = 1024 * sizeof(uint32_t);
        AllocParamsForBufferLinear.pBufName = "HucDebugOutputBuffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBufferLinear,
            &PakMode.Hw.resHucDebugOutputBuffer));
    }

    if (m_encEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBrcResources());

        if (m_hmeSupported)
        {
            MOS_ZeroMemory(&m_s4XMemvDataBuffer, sizeof(MOS_SURFACE));
            m_s4XMemvDataBuffer.TileType = MOS_TILE_LINEAR;
            m_s4XMemvDataBuffer.bArraySpacing = true;
            m_s4XMemvDataBuffer.Format = Format_Buffer_2D;
            m_s4XMemvDataBuffer.dwWidth = m_downscaledWidthInMb4x * 32;
            m_s4XMemvDataBuffer.dwHeight = m_downscaledHeightInMb4x * 4 * 4;
            m_s4XMemvDataBuffer.dwPitch = MOS_ALIGN_CEIL(m_s4XMemvDataBuffer.dwWidth, 64);

            AllocParamsForBuffer2D.dwWidth = m_s4XMemvDataBuffer.dwWidth;
            AllocParamsForBuffer2D.dwHeight = m_s4XMemvDataBuffer.dwHeight;
            AllocParamsForBuffer2D.pBufName = "4xME MV Data Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBuffer2D,
                &m_s4XMemvDataBuffer.OsResource));

            MOS_ZeroMemory(&m_s4XMeDistortionBuffer, sizeof(MOS_SURFACE));
            m_s4XMeDistortionBuffer.TileType = MOS_TILE_LINEAR;
            m_s4XMeDistortionBuffer.bArraySpacing = true;
            m_s4XMeDistortionBuffer.Format = Format_Buffer_2D;
            m_s4XMeDistortionBuffer.dwWidth = m_downscaledWidthInMb4x * 8;
            m_s4XMeDistortionBuffer.dwHeight = m_downscaledHeightInMb4x * 4 * 4;
            m_s4XMeDistortionBuffer.dwPitch = MOS_ALIGN_CEIL(m_s4XMeDistortionBuffer.dwWidth, 64);

            AllocParamsForBuffer2D.dwWidth = m_s4XMeDistortionBuffer.dwWidth;
            AllocParamsForBuffer2D.dwHeight = m_s4XMeDistortionBuffer.dwHeight;
            AllocParamsForBuffer2D.pBufName = "4xME Distortion Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBuffer2D,
                &m_s4XMeDistortionBuffer.OsResource));
        }

        if (m_16xMeSupported)
        {
            MOS_ZeroMemory(&m_s16XMemvDataBuffer, sizeof(MOS_SURFACE));
            m_s16XMemvDataBuffer.TileType = MOS_TILE_LINEAR;
            m_s16XMemvDataBuffer.bArraySpacing = true;
            m_s16XMemvDataBuffer.Format = Format_Buffer_2D;

            m_s16XMemvDataBuffer.dwWidth = MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64);
            m_s16XMemvDataBuffer.dwHeight = m_downscaledHeightInMb16x * 4 * 4;
            m_s16XMemvDataBuffer.dwPitch = m_s16XMemvDataBuffer.dwWidth;

            AllocParamsForBuffer2D.dwWidth = m_s4XMemvDataBuffer.dwWidth;
            AllocParamsForBuffer2D.dwHeight = m_s4XMemvDataBuffer.dwHeight;
            AllocParamsForBuffer2D.pBufName = "16xME MV Data Buffer";
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBuffer2D,
                &m_s16XMemvDataBuffer.OsResource));
        }

        // intermediate surface to be used by the P kernel to help reduce number of SIC calls
        MOS_ZeroMemory(&m_output16x16InterModes, sizeof(MOS_SURFACE));
        m_output16x16InterModes.TileType = MOS_TILE_LINEAR;
        m_output16x16InterModes.bArraySpacing = true;
        m_output16x16InterModes.Format = Format_Buffer_2D;
        m_output16x16InterModes.dwWidth = 16 * m_picWidthInMb;
        m_output16x16InterModes.dwHeight = 8 * m_picHeightInMb;
        m_output16x16InterModes.dwPitch = MOS_ALIGN_CEIL(m_output16x16InterModes.dwWidth, 64);

        AllocParamsForBuffer2D.dwWidth = m_output16x16InterModes.dwWidth;
        AllocParamsForBuffer2D.dwHeight = m_output16x16InterModes.dwHeight;
        AllocParamsForBuffer2D.pBufName = "Intermediate surface";
            
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &AllocParamsForBuffer2D,
            &m_output16x16InterModes.OsResource));

        dwSize = 16 * m_picWidthInMb * m_picHeightInMb * sizeof(uint32_t);
        AllocParamsForBufferLinear.dwBytes = dwSize;
        AllocParamsForBufferLinear.pBufName = "Mode Decision Buffer";

        for (i = 0; i < 2; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
                m_osInterface,
                &AllocParamsForBufferLinear,
                &PakMode.Hw.resModeDecision[i]));
        }
    }

finish:
    return eStatus;
}

void CodechalEncodeVp9::FreeResources()
{
    PCODEC_REF_LIST             *ppVp9RefList;
    uint32_t                       i;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    ppVp9RefList = &m_refList[0];

    // Release Ref Lists
    for (i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9; i++)
    {
#if 0
        if (!Mos_ResourceIsNull(&ppVp9RefList[i]->sDysSurface.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &ppVp9RefList[i]->sDysSurface.OsResource);
        }

        if (!Mos_ResourceIsNull(&ppVp9RefList[i]->sDys4xScaledSurface.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &ppVp9RefList[i]->sDys4xScaledSurface.OsResource);
        }

        if (!Mos_ResourceIsNull(&ppVp9RefList[i]->sDys16xScaledSurface.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &ppVp9RefList[i]->sDys16xScaledSurface.OsResource);
        }
#endif
    }
    CodecHalFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resDeblockingFilterLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resDeblockingFilterTileLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resDeblockingFilterTileColumnBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resMetadataLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resMetadataTileLineBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resMetadataTileColumnBuffer);

    for (i = 0; i < 2; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &PakMode.Hw.resMvTemporalBuffer[i]);
    }

    for (i = 0; i < CODEC_VP9_NUM_CONTEXTS; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &PakMode.Hw.resProbBuffer[i]);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resSegmentIdBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resProbabilityDeltaBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resCompressedHeaderBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resProbabilityCounterBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resTileRecordStrmOutBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resCuStatsStrmOutBuffer);

    for (i = 0; i < 2; i++)
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &PakMode.Hw.resHucProbDmemBuffer[i]);
    }

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resHucDefaultProbBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resHucProbOutputBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resHucPakInsertUncompressedHeaderReadBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resHucPakInsertUncompressedHeaderWriteBuffer);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &PakMode.Hw.resHucSuperFrameBuffer);

        m_osInterface->pfnFreeResource(
            m_osInterface,
            &PakMode.Hw.resHucDebugOutputBuffer);

    m_osInterface->pfnFreeResource(
        m_osInterface,
        &PakMode.Hw.resHucDebugOutputBuffer);

    if (m_encEnabled)
    {
	FreeBrcResources();

        for (i = 0; i < 2; i++)
        {
            if (!Mos_ResourceIsNull(&PakMode.Hw.resModeDecision[i]))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &PakMode.Hw.resModeDecision[i]);
            }
        }

        if (!Mos_ResourceIsNull(&m_output16x16InterModes.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_output16x16InterModes.OsResource);
        }

        if (m_hmeSupported)
        {
            if (!Mos_ResourceIsNull(&m_s4XMemvDataBuffer.OsResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_s4XMemvDataBuffer.OsResource);
            }

            if (!Mos_ResourceIsNull(&m_s4XMeDistortionBuffer.OsResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_s4XMeDistortionBuffer.OsResource);
            }
        }

        if (m_16xMeSupported)
        {
            if (!Mos_ResourceIsNull(&m_s16XMemvDataBuffer.OsResource))
            {
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_s16XMemvDataBuffer.OsResource);
            }
        }
    }

    return;
}

void CodechalEncodeVp9::ResizeBuffer()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // if resolution changed, free existing DS/CSC/MbCode/MvData resources
    m_trackedBuf->Resize();
}

MOS_STATUS CodechalEncodeVp9::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_bitstreamUpperBound = params.dwBitstreamSize;

    m_vp9SeqParams = (PCODEC_VP9_ENCODE_SEQUENCE_PARAMS)(params.pSeqParams);
    m_vp9PicParams = (PCODEC_VP9_ENCODE_PIC_PARAMS)(params.pPicParams);
    m_vp9SegmentParams = (PCODEC_VP9_ENCODE_SEGMENT_PARAMS)(params.pSegmentParams);    
    m_nalUnitParams = params.ppNALUnitParams;
    m_numNalUnit    = params.uiNumNalUnits;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9SeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9PicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9SegmentParams);
  
    m_mbSegmentMapSurface = *(params.psMbSegmentMapSurface); 

    // Need to index properly when more than one temporal layer is present.
    CODECHAL_ENCODE_ASSERT(m_vp9SeqParams->FrameRate[0].uiDenominator > 0);
    m_frameRate = m_vp9SeqParams->FrameRate[0].uiNumerator / m_vp9SeqParams->FrameRate[0].uiDenominator; 

    m_vp9SegmentParams = (PCODEC_VP9_ENCODE_SEGMENT_PARAMS)(params.pSegmentParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9SegmentParams);

    if (m_vp9PicParams->PicFlags.fields.segmentation_enabled) {
      CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
          m_osInterface,
          &m_mbSegmentMapSurface));
    }
 
    if (m_brcEnabled)
    {
        //if BRC is enabled, these values need to be filled by the BRC kernel,so zeroing to initialize 
        MOS_ZeroMemory(m_vp9SegmentParams, sizeof(CODEC_VP9_ENCODE_SEGMENT_PARAMS));
    }
    
    // reset for next frame
    if (m_b16xMeEnabled)
    {
        m_b16xMeDone = false;
    }

    if (m_newSeq)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPictureStructs());

#if 0
    if (CODECHAL_ENCODE_CSC_DISABLED != pEncoder->cscDsCopyEnable)
    {
        if (m_vp9SeqParams->SeqFlags.fields.EnableDynamicScaling)
        {
            CodecHalEncode_CscDsCopy_ReleaseResources(pEncoder);
        }

        // check if we need to do CSC or copy non-aligned surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncode_CscDsCopy_CheckCondition(pEncoder));
    }
#endif

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(m_refList[m_currReconstructedPic.FrameIdx]));

    /* No DebugDump support for now */
#if 0
    CODECHAL_DEBUG_TOOL(
        pEncoder->pDebugInterface->CurrPic = m_vp9PicParams->CurrOriginalPic;
        pEncoder->pDebugInterface->dwBufferDumpFrameNum = pEncoder->dwStoreData;
        pEncoder->pDebugInterface->wFrameType = m_pictureCodingType;

        if (pEncoder->bNewSeq)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeVp9SeqParams(
                pEncoder->pDebugInterface,
                m_vp9SeqParams));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeVp9PicParams(
            pEncoder->pDebugInterface,
            m_vp9PicParams));

        
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgDumpEncodeVp9SegmentParams(
            pEncoder->pDebugInterface,
            m_vp9SegmentParams));
    
    )
#endif
    m_bitstreamUpperBound = m_frameWidth * m_frameHeight * 3 / 2;

    return status;
}

MOS_STATUS CodechalEncodeVp9::SetSequenceStructs()
{

    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS status = MOS_STATUS_SUCCESS;

    auto pSeqParams = m_vp9SeqParams;

        m_numPasses = CODECHAL_ENCODE_VP9_CQP_NUM_OF_PASSES - 1;
        m_brcEnabled = CodecHalIsRateControlBrc(pSeqParams->RateControlMethod, CODECHAL_VP9);
        if(m_brcEnabled)
        {
            m_brcReset    = m_vp9SeqParams->SeqFlags.fields.bResetBRC;
            m_numPasses   = m_multipassBrcSupported ? CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES: CODECHAL_ENCODE_VP9_BRC_DEFAULT_NUM_OF_PASSES - 1;

	    //Workaround to fix the PATCHLOCATIONLIST_SIZE limitation
	    //In a multi-pack scenario, 4th iteration will exceeds the
	    //PATCHLOCATIONLIST_SIZE which is currently set to CODECHAL_MAX_REGS
	    //(==128), and will segfault.
	    m_numPasses = 3;
        }

    return status;
}

MOS_STATUS CodechalEncodeVp9::SetPictureStructs()
{
    uint8_t                                 i, ii = 0, ucIndex = 0, currRefIdx;
    bool                                    bDuplicatedIdx;
    uint16_t        brcPrecision;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9SeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9PicParams);

    auto pSeqParams = m_vp9SeqParams;
    auto pPicParams = m_vp9PicParams;

    // setup internal parameters
    // dwOriFrameWidth and dwOriFrameHeight are encoded resolutions which might be different from source resoultions if dynamic scaling is enabled
    if (!pSeqParams->SeqFlags.fields.EnableDynamicScaling)
    {
        m_oriFrameWidth = pPicParams->SrcFrameWidthMinus1 + 1;
        m_oriFrameHeight = pPicParams->SrcFrameHeightMinus1 + 1;

        pPicParams->DstFrameWidthMinus1 = pPicParams->SrcFrameWidthMinus1;
        pPicParams->DstFrameHeightMinus1 = pPicParams->SrcFrameHeightMinus1;
    }
    else
    {
        m_oriFrameWidth = pPicParams->DstFrameWidthMinus1 + 1;
        m_oriFrameHeight = pPicParams->DstFrameHeightMinus1 + 1;
    }

    CODECHAL_ENCODE_ASSERT(((pPicParams->SrcFrameWidthMinus1 + 1) % CODEC_VP9_MIN_BLOCK_WIDTH) != 0);
    CODECHAL_ENCODE_ASSERT(((pPicParams->SrcFrameHeightMinus1 + 1) % CODEC_VP9_MIN_BLOCK_HEIGHT) != 0);
    CODECHAL_ENCODE_ASSERT(((pPicParams->DstFrameWidthMinus1 + 1) % CODEC_VP9_MIN_BLOCK_WIDTH) != 0);
    CODECHAL_ENCODE_ASSERT(((pPicParams->DstFrameHeightMinus1 + 1) % CODEC_VP9_MIN_BLOCK_HEIGHT) != 0);

    if (m_oriFrameWidth == 0 || m_oriFrameWidth > m_dwMaxPicWidth ||
        m_oriFrameHeight == 0 || m_oriFrameHeight > m_dwMaxPicHeight)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_dwPicWidthInSB = MOS_ROUNDUP_DIVIDE(m_oriFrameWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
    m_dwPicHeightInSB = MOS_ROUNDUP_DIVIDE(m_oriFrameHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    m_dwPicSizeInSB = m_dwPicWidthInSB * m_dwPicHeightInSB;

    m_picWidthInMb  = (uint16_t)CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth);
    m_picHeightInMb = (uint16_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight);
    m_frameWidth  = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    // HME Scaling WxH
    m_downscaledWidthInMb4x  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x = m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x = m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    // SuperHME Scaling WxH
    m_downscaledWidthInMb16x  = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_16x);
    m_downscaledHeightInMb16x = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_16x);
    m_downscaledWidth16x = m_downscaledWidthInMb16x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight16x = m_downscaledHeightInMb16x * CODECHAL_MACROBLOCK_HEIGHT;

    m_frameFieldHeight = m_frameHeight;
    m_frameFieldHeightInMb = m_picHeightInMb;
    m_downscaledFrameFieldHeightInMb4x = m_downscaledHeightInMb4x;
    m_downscaledFrameFieldHeightInMb16x = m_downscaledHeightInMb16x;

    // enable this once the C-model is updated to handle the reference window WA
    MotionEstimationDisableCheck();

    if (Mos_ResourceIsNull(&m_rawSurface.OsResource))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    // set the correct raw surface width/height (in case of padding)
    m_rawSurface.dwWidth = pPicParams->SrcFrameWidthMinus1 + 1;
    m_rawSurface.dwHeight = pPicParams->SrcFrameHeightMinus1 + 1;

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) && 
        (!pSeqParams->SeqFlags.fields.bUseRawReconRef || m_codecFunction != CODECHAL_FUNCTION_ENC))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Sync initialize
    m_bWaitForENC = false;
    if ((m_firstFrame) ||
        (m_codecFunction == CODECHAL_FUNCTION_ENC) ||
        (!m_brcEnabled && pSeqParams->SeqFlags.fields.bUseRawReconRef) ||
        (!m_brcEnabled && (pPicParams->PicFlags.fields.frame_type == 0 || pPicParams->PicFlags.fields.intra_only)))
    {
        m_waitForPak = false;
    }
    else
    {
        m_waitForPak = true;
    }

    if ((m_codecFunction != CODECHAL_FUNCTION_ENC || m_brcEnabled) && !m_vdencEnabled)
    {
        m_signalEnc = true;
    }
    else
    {
        m_signalEnc = false;
    }

    currRefIdx = pPicParams->CurrReconstructedPic.FrameIdx;

    // m_refFrameFlags is to indicate which frames to be used as reference
    // m_refFrameFlags & 0x01 != 0: Last ref frames used as reference
    // m_refFrameFlags & 0x02 != 0: Golden ref frames used as reference
    // m_refFrameFlags & 0x04 != 0: Alternate ref frames used as reference
    m_refFrameFlags = 0;
    m_numRefFrames = 0;
    m_lastRefPic = 0;
    m_goldenRefPic = 0;
    m_altRefPic = 0;
   
    if (pPicParams->PicFlags.fields.frame_type != 0 && !pPicParams->PicFlags.fields.intra_only)
    {
        /* TODO: m_refFrameFlags may also depend on the TU setting */
        m_refFrameFlags = pPicParams->RefFlags.fields.ref_frame_ctrl_l0 | pPicParams->RefFlags.fields.ref_frame_ctrl_l1;

        if (CodecHal_PictureIsInvalid(pPicParams->RefFrameList[pPicParams->RefFlags.fields.LastRefIdx]))
        {
            m_refFrameFlags &= ~0x1;
        }
        if (CodecHal_PictureIsInvalid(pPicParams->RefFrameList[pPicParams->RefFlags.fields.GoldenRefIdx]))
        {
            m_refFrameFlags &= ~0x2;
        }
        if (CodecHal_PictureIsInvalid(pPicParams->RefFrameList[pPicParams->RefFlags.fields.AltRefIdx]))
        {
            m_refFrameFlags &= ~0x4;
        }
       
        //consilidate the reference flag, becasue two reference frame may have the same index
        if ((m_refFrameFlags & 0x01) && 
            (pPicParams->RefFrameList[pPicParams->RefFlags.fields.LastRefIdx].FrameIdx == pPicParams->RefFrameList[pPicParams->RefFlags.fields.GoldenRefIdx].FrameIdx))
        {
            m_refFrameFlags &= ~0x2;  //skip golden frame
        }
        if ((m_refFrameFlags & 0x01) &&
                (pPicParams->RefFrameList[pPicParams->RefFlags.fields.LastRefIdx].FrameIdx == pPicParams->RefFrameList[pPicParams->RefFlags.fields.AltRefIdx].FrameIdx))
        {
            m_refFrameFlags &= ~0x4;  //skip alt frame
        }
        if ((m_refFrameFlags & 0x02) && 
            (pPicParams->RefFrameList[pPicParams->RefFlags.fields.GoldenRefIdx].FrameIdx == pPicParams->RefFrameList[pPicParams->RefFlags.fields.AltRefIdx].FrameIdx))
        {
            m_refFrameFlags &= ~0x4;  //skip alt frame
        }

        if (m_refFrameFlags == 0)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Ref list is empty!.");
            status = MOS_STATUS_INVALID_PARAMETER;
	    return status;
        }

        if (m_refFrameFlags & 0x01)
        {
            ucIndex = pPicParams->RefFrameList[pPicParams->RefFlags.fields.LastRefIdx].FrameIdx;
            m_refList[ucIndex]->sRefBuffer = 
                pSeqParams->SeqFlags.fields.bUseRawReconRef ?
                m_refList[ucIndex]->sRefRawBuffer :
                m_refList[ucIndex]->sRefReconBuffer;

            m_lastRefPic = &m_refList[ucIndex]->sRefBuffer;
            CodecHalGetResourceInfo(m_osInterface, m_lastRefPic);
            m_lastRefPic->dwWidth = m_refList[ucIndex]->dwFrameWidth;
            m_lastRefPic->dwHeight = m_refList[ucIndex]->dwFrameHeight;
			m_numRefFrames++;
        }

        if (m_refFrameFlags & 0x02)
        {
            ucIndex = pPicParams->RefFrameList[pPicParams->RefFlags.fields.GoldenRefIdx].FrameIdx;
            m_refList[ucIndex]->sRefBuffer =
                pSeqParams->SeqFlags.fields.bUseRawReconRef ?
                m_refList[ucIndex]->sRefRawBuffer :
                m_refList[ucIndex]->sRefReconBuffer;

            m_goldenRefPic = &m_refList[ucIndex]->sRefBuffer;
            CodecHalGetResourceInfo(m_osInterface, m_goldenRefPic);
            m_goldenRefPic->dwWidth = m_refList[ucIndex]->dwFrameWidth;
            m_goldenRefPic->dwHeight = m_refList[ucIndex]->dwFrameHeight;
			m_numRefFrames++;
        }

        if (m_refFrameFlags & 0x04)
        {
            ucIndex = pPicParams->RefFrameList[pPicParams->RefFlags.fields.AltRefIdx].FrameIdx;
            m_refList[ucIndex]->sRefBuffer =
                pSeqParams->SeqFlags.fields.bUseRawReconRef ?
                m_refList[ucIndex]->sRefRawBuffer :
                m_refList[ucIndex]->sRefReconBuffer;

            m_altRefPic = &m_refList[ucIndex]->sRefBuffer;
            CodecHalGetResourceInfo(m_osInterface, m_altRefPic);
            m_altRefPic->dwWidth = m_refList[ucIndex]->dwFrameWidth;
            m_altRefPic->dwHeight = m_refList[ucIndex]->dwFrameHeight;
			m_numRefFrames++;
        }
    }
    
    m_refList[currRefIdx]->sRefReconBuffer = m_reconSurface;
    m_refList[currRefIdx]->sRefRawBuffer = m_rawSurface;
    m_refList[currRefIdx]->RefPic = pPicParams->CurrOriginalPic;
    m_refList[currRefIdx]->bUsedAsRef = true;
    m_refList[currRefIdx]->resBitstreamBuffer = m_resBitstreamBuffer;
    m_refList[currRefIdx]->dwFrameWidth = m_oriFrameWidth;
    m_refList[currRefIdx]->dwFrameHeight = m_oriFrameHeight;
 
    m_currOriginalPic = pPicParams->CurrOriginalPic;
    m_currReconstructedPic = pPicParams->CurrReconstructedPic;
    m_statusReportFeedbackNumber = pPicParams->StatusReportFeedbackNumber;
    m_pictureCodingType = pPicParams->PicFlags.fields.frame_type == 0 ? I_TYPE : P_TYPE;

    for (i = 0; i < CODEC_VP9_NUM_REF_FRAMES; i++)
    {
        m_picIdx[i].bValid = false;
        if (pPicParams->RefFrameList[i].PicFlags != PICTURE_INVALID)
        {
            ucIndex = pPicParams->RefFrameList[i].FrameIdx;
            bDuplicatedIdx = false;
            for (ii = 0; ii < i; ii++)
            {
                if (m_picIdx[ii].bValid && ucIndex == pPicParams->RefFrameList[ii].FrameIdx)
                {
                    // we find the same FrameIdx in the ref_frame_list. Multiple reference frames are the same.
                    bDuplicatedIdx = true;
                    break;
                }
            }
            if (bDuplicatedIdx)
            {
                continue;
            }

            // this reference frame in unique. Save it into the full reference list with 127 items
            m_refList[ucIndex]->RefPic.PicFlags =
                CodecHal_CombinePictureFlags(m_refList[ucIndex]->RefPic, pPicParams->RefFrameList[i]);

            m_picIdx[i].bValid = true;
            m_picIdx[i].ucPicIdx = ucIndex;

        }
    }

    // Save the current RefList
    ii = 0;
    for (i = 0; i < CODEC_VP9_NUM_REF_FRAMES; i++)
    {
        if (m_picIdx[i].bValid)
        {
            m_refList[currRefIdx]->RefList[ii] = pPicParams->RefFrameList[i];
            ii++;
        }
    }
    m_refList[currRefIdx]->ucNumRef = ii;
    m_refList[currRefIdx]->bUsedAsRef = true; /* VP8 has no non reference pictures */
    m_currRefList                     = m_refList[currRefIdx];
   
    if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
    {
	// the actual MbCode/MvData surface to be allocated later
        m_trackedBuf->SetAllocationFlag(true);
    } else {
	CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeParams.presMbCodeSurface);
        m_resMbCodeSurface = *m_encodeParams.presMbCodeSurface;
        m_refList[currRefIdx]->resRefMbCodeBuffer = m_resMbCodeSurface;
    }

    m_refList[currRefIdx]->ucQPValue[0] = pPicParams->LumaACQIndex + pPicParams->LumaDCQIndexDelta;

    m_hmeEnabled = m_hmeSupported && m_pictureCodingType != I_TYPE && !pPicParams->PicFlags.fields.intra_only;
    m_b16xMeEnabled = m_16xMeSupported &&  m_hmeEnabled;
    m_mbEncIFrameDistEnabled  =  m_brcDistortionBufferSupported && (m_pictureCodingType == I_TYPE);
    m_firstField = true; // each frame is treated as the first field

    // TODO: set ucTxMode from TU setting
    m_ucTxMode = CODEC_VP9_TX_SELECTABLE;

    brcPrecision = 0;
    m_hwInterface->GetMfxInterface()->SetBrcNumPakPasses(
        (m_brcEnabled && m_multipassBrcSupported) ? GetNumBrcPakPasses(brcPrecision) : 1);

    return status;
}

MOS_STATUS
CodechalEncodeVp9::ExecutePictureLevel()
{
    MhwMiInterface                                  *commonMiInterface;
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS                  pipeBufAddrParams;
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS              indObjBaseAddrParams;
    MHW_BATCH_BUFFER                                SecondLevelBatchBuffer;
    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS      miConditionalBatchBufferEndParams;
    PMOS_SURFACE                                    presLastRefSurface, presGoldenRefSurface, presAltRefSurface;
    uint8_t                                         ucLastRefPicIndex, ucGoldenRefPicIndex, ucAltRefPicIndex;
    uint8_t                                         ucFrameCtxIdx, ucSegmentCount, i;
    MOS_STATUS                                      eStatus = MOS_STATUS_SUCCESS;
    MHW_VDBOX_SURFACE_PARAMS                        DssurfaceParams[2];

    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_singleTaskPhaseSupported)
    {
       CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    commonMiInterface = m_hwInterface->GetMiInterface();

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);

    // We only need to update Huc PAK insert object and picture state for the first pass
    if (m_currPass == 0)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructPakInsertObjBatchBuf(&PakMode.Hw.resHucPakInsertUncompressedHeaderReadBuffer));

        if (!m_brcEnabled)   // Check if driver already programmed pic state as part of BRC update kernel programming.
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructPicStateBatchBuf(
                &m_brcBuffers.resPicStateBrcWriteHucReadBuffer));
        }
    }
   
    if (m_currPass == 0) {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(RefreshFrameInternalBuffers());
    }

    // set HCP_SURFACE_STATE values
    MHW_VDBOX_SURFACE_PARAMS surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID + 1];
    for (uint8_t i = 0; i <= CODECHAL_HCP_ALTREF_SURFACE_ID; i++)
    {
        MOS_ZeroMemory(&surfaceParams[i], sizeof(surfaceParams[i]));
        surfaceParams[i].Mode = m_mode;
        surfaceParams[i].ucSurfaceStateId = i;
        surfaceParams[i].ChromaType = m_outputChromaFormat;
        surfaceParams[i].bSrc8Pak10Mode   = (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth) && (!m_vp9SeqParams->SeqFlags.fields.SourceBitDepth);

        switch (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
        {
            case VP9_ENCODED_BIT_DEPTH_10: //10 bit encoding
            {
                surfaceParams[i].ucBitDepthChromaMinus8 = 2;
                surfaceParams[i].ucBitDepthLumaMinus8 = 2;
                break;
            }
            default:
            {
                surfaceParams[i].ucBitDepthChromaMinus8 = 0;
                surfaceParams[i].ucBitDepthLumaMinus8 = 0;
                break;
            }
        }
    }

    // For PAK engine, we do NOT use scaled reference images even if dynamic scaling is enabled
    presLastRefSurface = NULL;
    presGoldenRefSurface = NULL;
    presAltRefSurface = NULL;
    if (m_pictureCodingType != I_TYPE)
    {
        if (m_refFrameFlags & 0x01)
        {
            ucLastRefPicIndex = m_vp9PicParams->RefFlags.fields.LastRefIdx;

            CODECHAL_ENCODE_ASSERT((ucLastRefPicIndex < CODEC_VP9_NUM_REF_FRAMES) && (!CodecHal_PictureIsInvalid(m_vp9PicParams->RefFrameList[ucLastRefPicIndex])));
            presLastRefSurface = &(m_refList[m_vp9PicParams->RefFrameList[ucLastRefPicIndex].FrameIdx]->sRefBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, presLastRefSurface));
        }

        if (m_refFrameFlags & 0x02)
        {
            ucGoldenRefPicIndex = m_vp9PicParams->RefFlags.fields.GoldenRefIdx;

            CODECHAL_ENCODE_ASSERT((ucGoldenRefPicIndex < CODEC_VP9_NUM_REF_FRAMES) && (!CodecHal_PictureIsInvalid(m_vp9PicParams->RefFrameList[ucGoldenRefPicIndex])));
            presGoldenRefSurface = &(m_refList[m_vp9PicParams->RefFrameList[ucGoldenRefPicIndex].FrameIdx]->sRefBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, presGoldenRefSurface));
        }

        if (m_refFrameFlags & 0x04)
        {
            ucAltRefPicIndex = m_vp9PicParams->RefFlags.fields.AltRefIdx;

            CODECHAL_ENCODE_ASSERT((ucAltRefPicIndex < CODEC_VP9_NUM_REF_FRAMES) && (!CodecHal_PictureIsInvalid(m_vp9PicParams->RefFrameList[ucAltRefPicIndex])))
            presAltRefSurface = &(m_refList[m_vp9PicParams->RefFrameList[ucAltRefPicIndex].FrameIdx]->sRefBuffer);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, presAltRefSurface));
        }

        if (!presLastRefSurface)
        {
            presLastRefSurface = (presGoldenRefSurface) ? presGoldenRefSurface : presAltRefSurface;
        }

        if (!presGoldenRefSurface)
        {
            presGoldenRefSurface = (presLastRefSurface) ? presLastRefSurface : presAltRefSurface;
        }

        if (!presAltRefSurface)
        {
            presAltRefSurface = (presLastRefSurface) ? presLastRefSurface : presGoldenRefSurface;
        }
    }

    // recon
    surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID].psSurface        = &m_reconSurface;
    surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID].ucSurfaceStateId = CODECHAL_HCP_DECODED_SURFACE_ID;
    surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID].dwReconSurfHeight    = m_rawSurfaceToPak->dwHeight;
    // raw
    surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID].psSurface            = &m_rawSurface;
    surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID].ucSurfaceStateId     = CODECHAL_HCP_SRC_SURFACE_ID;
    surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID].bDisplayFormatSwizzle    = m_vp9SeqParams->SeqFlags.fields.DisplayFormatSwizzle;
    surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID].dwActualWidth            = MOS_ALIGN_CEIL(m_oriFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH);
    surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID].dwActualHeight           = MOS_ALIGN_CEIL(m_oriFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }
     // Making sure ImgStatusCtrl is zeroed out before first PAK pass. HW supposedly does this before start of each frame. Remove this after confirming.
    if (m_currPass == 0)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS miLoadRegImmParams;
        MOS_ZeroMemory(&miLoadRegImmParams, sizeof(miLoadRegImmParams));
        miLoadRegImmParams.dwData     = 0;
        miLoadRegImmParams.dwRegister = mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiLoadRegisterImmCmd(&cmdBuffer, &miLoadRegImmParams));

    }


    // Read Image status before running PAK, to get correct cumulative delta applied for final pass.
    if (m_currPass != m_numPasses)        // Don't read it for Repak
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadImageStatus(&cmdBuffer));
    }

    // BRC 1st-3rd Pass (excludes RePAK) Conditional batch buffer end based on ImgStatus Ctrl
    if (m_brcEnabled &&  (m_currPass > 0) && (m_currPass != m_numPasses))
    {
        // Insert conditional batch buffer end
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer   =
            &m_encodeStatusBuf.resStatusBuffer;
        miConditionalBatchBufferEndParams.dwOffset              =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwImageStatusMaskOffset                       +
            (sizeof(uint32_t) * 2);
	//Choosen this based on legacy i965 impl. With out this, the uncompressed
	//header is not getting updated with qp and lf values for BRC.
        miConditionalBatchBufferEndParams.bDisableCompareMask = true;

	CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
    // set HCP_PIPE_MODE_SELECT values
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);
    pipeModeSelectParams->Mode = m_mode;
    pipeModeSelectParams->bStreamOutEnabled = m_vdencBrcEnabled;
    pipeModeSelectParams->bVdencEnabled = false;
    pipeModeSelectParams->bVdencPakObjCmdStreamOutEnable = 0;
    //pipeModeSelectParams->bTlbPrefetchEnable = true;
    // Add 1 to compensate for VdencPipeModeSelect params values
    pipeModeSelectParams->ChromaType = m_vp9SeqParams->SeqFlags.fields.EncodedFormat + 1;
    switch (m_vp9SeqParams->SeqFlags.fields.EncodedBitDepth)
    {
        case VP9_ENCODED_BIT_DEPTH_10:
        {
            pipeModeSelectParams->ucVdencBitDepthMinus8 = 2;
            break;
        }
        default:
        {
            pipeModeSelectParams->ucVdencBitDepthMinus8 = 0;
            break;
        }
    }

    //VP9Enc:Cmd1: HCP_PIPE_MODE_SELECT
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));

     // This wait cmd is needed to make sure copy is done as suggested by HW folk
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(&cmdBuffer, nullptr, false));

    // Decodec picture
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_DECODED_SURFACE_ID]));

    // Source input
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_SRC_SURFACE_ID]));

    // Prev reference picture 
    if (presLastRefSurface)
    {
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].psSurface           = presLastRefSurface;
        surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID].ucSurfaceStateId    = CODECHAL_HCP_LAST_SURFACE_ID;
	CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_LAST_SURFACE_ID]));
    }

    // Golden reference picture
    if (presGoldenRefSurface)
    {
        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].psSurface         = presGoldenRefSurface;
        surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID].ucSurfaceStateId  = CODECHAL_HCP_GOLDEN_SURFACE_ID;
	CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_GOLDEN_SURFACE_ID]));
    }

    // AltRef reference picture
    if (presAltRefSurface)
    {
        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].psSurface         = presAltRefSurface;
        surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID].ucSurfaceStateId  = CODECHAL_HCP_ALTREF_SURFACE_ID;
	CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &surfaceParams[CODECHAL_HCP_ALTREF_SURFACE_ID]));
    }

    // set HCP_PIPE_BUF_ADDR_STATE values
    MOS_ZeroMemory(&pipeBufAddrParams, sizeof(pipeBufAddrParams));
    pipeBufAddrParams.Mode                  = m_mode;
    pipeBufAddrParams.psPreDeblockSurface   = &m_reconSurface;
    pipeBufAddrParams.psPostDeblockSurface  = &m_reconSurface;
    pipeBufAddrParams.psRawSurface          = &m_rawSurface;
    
    pipeBufAddrParams.presStreamOutBuffer = NULL;
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer =
        &PakMode.Hw.resDeblockingFilterLineBuffer;
    
    pipeBufAddrParams.presDeblockingFilterTileRowStoreScratchBuffer =
        &PakMode.Hw.resDeblockingFilterTileLineBuffer;
    
    pipeBufAddrParams.presDeblockingFilterColumnRowStoreScratchBuffer =
        &PakMode.Hw.resDeblockingFilterTileColumnBuffer;

    pipeBufAddrParams.presMetadataLineBuffer        = &PakMode.Hw.resMetadataLineBuffer;
    pipeBufAddrParams.presMetadataTileLineBuffer    = &PakMode.Hw.resMetadataTileLineBuffer;
    pipeBufAddrParams.presMetadataTileColumnBuffer  = &PakMode.Hw.resMetadataTileColumnBuffer;
    pipeBufAddrParams.presCurMvTempBuffer           = &PakMode.Hw.resMvTemporalBuffer[PakMode.Hw.dwCurrMvTemporalBufferIndex];
   
    ucFrameCtxIdx = m_vp9PicParams->PicFlags.fields.frame_context_idx;
    CODECHAL_ENCODE_ASSERT(ucFrameCtxIdx < CODEC_VP9_NUM_CONTEXTS);
    pipeBufAddrParams.presVp9ProbBuffer = &PakMode.Hw.resProbBuffer[ucFrameCtxIdx];
    pipeBufAddrParams.presVp9SegmentIdBuffer = &PakMode.Hw.resSegmentIdBuffer;

    if (m_pictureCodingType != I_TYPE)
    {
        // dhxtodoVP9: none of LGA can be NULL?
        CODECHAL_ENCODE_CHK_NULL_RETURN(presLastRefSurface);
        CODECHAL_ENCODE_CHK_NULL_RETURN(presGoldenRefSurface);
        CODECHAL_ENCODE_CHK_NULL_RETURN(presAltRefSurface);

        // dhxtodoVP9: change to CODECHAL_LAST_REF
        pipeBufAddrParams.presReferences[0] = &presLastRefSurface->OsResource;
        pipeBufAddrParams.presReferences[1] = &presGoldenRefSurface->OsResource;
        pipeBufAddrParams.presReferences[2] = &presAltRefSurface->OsResource;

        pipeBufAddrParams.presColMvTempBuffer[0] = &PakMode.Hw.resMvTemporalBuffer[PakMode.Hw.dwCurrMvTemporalBufferIndex ^ 0x01];
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetPipeBufAddr(&pipeBufAddrParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    // set HCP_IND_OBJ_BASE_ADDR_STATE values
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode                           = m_mode;
    indObjBaseAddrParams.presMvObjectBuffer             = &m_resMbCodeSurface;

    indObjBaseAddrParams.dwMvObjectOffset               = m_mvOffset;
    indObjBaseAddrParams.dwMvObjectSize                 = m_mbCodeSize - m_mvOffset;
    indObjBaseAddrParams.presPakBaseObjectBuffer        = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize            = m_bitstreamUpperBound;
    indObjBaseAddrParams.presProbabilityDeltaBuffer     = &PakMode.Hw.resProbabilityDeltaBuffer;
    indObjBaseAddrParams.dwProbabilityDeltaSize         = 29 * CODECHAL_CACHELINE_SIZE;
    indObjBaseAddrParams.presCompressedHeaderBuffer     = &PakMode.Hw.resCompressedHeaderBuffer;
    indObjBaseAddrParams.dwCompressedHeaderSize         = 32 * CODECHAL_CACHELINE_SIZE;
    indObjBaseAddrParams.presProbabilityCounterBuffer   = &PakMode.Hw.resProbabilityCounterBuffer;
    indObjBaseAddrParams.dwProbabilityCounterSize       = 193 * CODECHAL_CACHELINE_SIZE;
    indObjBaseAddrParams.presTileRecordBuffer           = &PakMode.Hw.resTileRecordStrmOutBuffer;
    indObjBaseAddrParams.dwTileRecordSize               = m_dwPicSizeInSB * CODECHAL_CACHELINE_SIZE;
    indObjBaseAddrParams.presCuStatsBuffer              = &PakMode.Hw.resCuStatsStrmOutBuffer;
    indObjBaseAddrParams.dwCuStatsSize                  = MOS_ALIGN_CEIL(m_dwPicSizeInSB * 64 * 8, CODECHAL_CACHELINE_SIZE);
    
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));
    

    // Using picstate zero with updated QP and LF deltas by HuC for repak, irrespective of how many Pak passes were run in multi-pass mode.
    MOS_ZeroMemory(&SecondLevelBatchBuffer, sizeof(SecondLevelBatchBuffer));
    SecondLevelBatchBuffer.dwOffset = (m_numPasses > 0) ? CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS * (m_currPass % m_numPasses) : 0;
    SecondLevelBatchBuffer.bSecondLevel = true;
    SecondLevelBatchBuffer.OsResource = m_brcBuffers.resPicStateBrcWriteHucReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
        &cmdBuffer,
        &SecondLevelBatchBuffer));
    
    // HCP_VP9_SEGMENT_STATE
    MHW_VDBOX_VP9_SEGMENT_STATE                     Vp9SegmentState;
    MOS_ZeroMemory(&Vp9SegmentState, sizeof(Vp9SegmentState));
    ucSegmentCount = (m_vp9PicParams->PicFlags.fields.segmentation_enabled) ? CODEC_VP9_MAX_SEGMENTS : 1;
    Vp9SegmentState.pbSegStateBufferPtr = nullptr; // Set this to nullptr, for commands to be prepared by driver
    Vp9SegmentState.Mode = m_mode;
    Vp9SegmentState.pVp9EncodeSegmentParams = m_vp9SegmentParams;
#if 0
    Vp9SegmentState.ucQPIndexLumaAC             = m_vp9PicParams->LumaACQIndex;
    // For BRC with segmentation, seg state commands for PAK are copied from BRC seg state buffer
    // For CQP or BRC with no segmentation, PAK still needs seg state commands and driver prepares those commands.
#endif
    //Vp9SegmentState.pcucLfQpLookup          = &CODECHAL_ENCODE_VP9_LF_VALUE_QP_LOOKUP[0];
    for (i = 0; i < ucSegmentCount; i++)
    {
        Vp9SegmentState.ucCurrentSegmentId = i;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpVp9SegmentStateCmd(&cmdBuffer, nullptr, &Vp9SegmentState));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);


finish:
    return eStatus;
}

MOS_STATUS
CodechalEncodeVp9::ExecuteSliceLevel()
{
    MhwMiInterface                              *miInterface;
    MOS_COMMAND_BUFFER                          cmdBuffer;
    MOS_SYNC_PARAMS                             syncParams;
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS              VdPipelineFlushParams;
    MHW_MI_FLUSH_DW_PARAMS                      FlushDwParams;
    EncodeReadBrcPakStatsParams                 ReadBrcPakStatsParams;
    MHW_BATCH_BUFFER                            SecondLevelBatchBuffer;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;
  
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    miInterface  = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MOS_ZeroMemory(&SecondLevelBatchBuffer, sizeof(SecondLevelBatchBuffer));
    SecondLevelBatchBuffer.dwOffset = 0;
    SecondLevelBatchBuffer.bSecondLevel = true;
    SecondLevelBatchBuffer.OsResource = PakMode.Hw.resHucPakInsertUncompressedHeaderReadBuffer;         
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            &SecondLevelBatchBuffer));
    
    MOS_ZeroMemory(&SecondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    SecondLevelBatchBuffer.OsResource = m_resMbCodeSurface;
    SecondLevelBatchBuffer.dwOffset = 0;
    SecondLevelBatchBuffer.bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            &SecondLevelBatchBuffer));

    //Fixme: This is not in i965
    MOS_ZeroMemory(&VdPipelineFlushParams, sizeof(VdPipelineFlushParams));
    // MFXPipeDone should not be set for tail insertion
    VdPipelineFlushParams.Flags.bWaitDoneMFX =
        (m_lastPicInStream || m_lastPicInSeq) ? 0 : 1;
    VdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
    VdPipelineFlushParams.Flags.bFlushHEVC = 1;
    VdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &VdPipelineFlushParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiFlushDwCmd(&cmdBuffer, &FlushDwParams));

    if (m_currPass >= (m_numPasses - 1))    // Last pass and the one before last
    {
       CODECHAL_ENCODE_CHK_STATUS_RETURN(miInterface->AddMiBatchBufferEnd(&cmdBuffer, NULL));
       //Workaround: avoid the extra pass(repack) which is causing quality issue in CBR mode.
       //legacy i965 driver doesn't have this extra pass. So until we figure out the root
       //cause, keep the code matching with i965 driver to get the same output.
       m_currPass = m_numPasses;
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (m_bWaitForENC &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_videoContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_bWaitForENC = false;
    }

    if (m_currPass >= (m_numPasses - 1))    // Last pass and the one before last
    {
        bool    bRenderFlags;

        bRenderFlags = m_videoContextUsesNullHw;
#if MOS_MEDIASOLO_SUPPORTED
        if (m_osInterface->bSoloInUse && m_singleTaskPhaseSupported)
        {
            bRenderFlags |= SOLO_LOAD_HUC_KERNEL;
        }
#endif // MOS_MEDIASOLO_SUPPORTED

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, bRenderFlags));

    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        if( m_signalEnc &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // Check if the signal obj count exceeds max value
            if (m_semaphoreObjCount == GFX_MIN(m_semaphoreMaxCount, MOS_MAX_OBJECT_SIGNALED))
            {
                syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_renderContext;
                syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
                m_semaphoreObjCount--;
            }

            // signal semaphore
            syncParams = g_cInitSyncParams;
            syncParams.GpuContext = m_videoContext;
            syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_semaphoreObjCount++;
        }

        PakMode.Hw.m_prevFrameInfo.KeyFrame = !m_vp9PicParams->PicFlags.fields.frame_type;
        PakMode.Hw.m_prevFrameInfo.IntraOnly = (m_vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME) || m_vp9PicParams->PicFlags.fields.intra_only;
        PakMode.Hw.m_prevFrameInfo.ShowFrame = m_vp9PicParams->PicFlags.fields.show_frame;
        PakMode.Hw.m_prevFrameInfo.FrameWidth = m_oriFrameWidth;
        PakMode.Hw.m_prevFrameInfo.FrameHeight = m_oriFrameHeight;
        PakMode.Hw.dwCurrMvTemporalBufferIndex ^= 0x01;
        PakMode.Hw.ucContextFrameTypes[m_vp9PicParams->PicFlags.fields.frame_context_idx] = m_vp9PicParams->PicFlags.fields.frame_type;

        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
    }

finish:
    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Retrieves the HCP image status information
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncodeVp9::ReadImageStatus(
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MhwMiInterface                      *commonMiInterface;
    EncodeStatusBuffer                  *encodeStatusBuf;
    MHW_MI_STORE_REGISTER_MEM_PARAMS    MiStoreRegMemParams;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    uint32_t                            dwBaseOffset;
    //MmioRegistersMfx                    *mmioRegisters;
    MHW_MI_COPY_MEM_MEM_PARAMS          CopyMemMemParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    commonMiInterface = m_hwInterface->GetMiInterface();
    encodeStatusBuf = &m_encodeStatusBuf;

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
    dwBaseOffset =
        (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        sizeof(uint32_t)* 2;  // pEncodeStatus is offset by 2 DWs in the resource

    MiStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    MiStoreRegMemParams.dwOffset        = dwBaseOffset + encodeStatusBuf->dwImageStatusMaskOffset;
    MiStoreRegMemParams.dwRegister      = mmioRegisters->hcpVp9EncImageStatusMaskRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &MiStoreRegMemParams));

    MiStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    MiStoreRegMemParams.dwOffset        = dwBaseOffset + encodeStatusBuf->dwImageStatusCtrlOffset;
    MiStoreRegMemParams.dwRegister      = mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &MiStoreRegMemParams));

    MOS_ZeroMemory(&CopyMemMemParams , sizeof(CopyMemMemParams));
    
    CopyMemMemParams.presSrc        = &encodeStatusBuf->resStatusBuffer;
    CopyMemMemParams.dwSrcOffset    = dwBaseOffset + encodeStatusBuf->dwImageStatusCtrlOffset;
    CopyMemMemParams.presDst        = &m_brcBuffers.resBrcBitstreamSizeBuffer;
    CopyMemMemParams.dwDstOffset    = CODECHAL_OFFSETOF(CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER, dwHcpImageStatusControl);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
    
    	&CopyMemMemParams));

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiFlushDwCmd(cmdBuffer, &FlushDwParams));

finish:
    return eStatus;
}

//------------------------------------------------------------------------------
//| Purpose:    Retrieves the HCP registers and stores them in the status report
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncodeVp9::ReadHcpStatus(
    PMOS_COMMAND_BUFFER             cmdBuffer)
{
    MhwMiInterface                      *commonMiInterface;
    EncodeStatusBuffer                  *encodeStatusBuf;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    uint32_t                            dwBaseOffset;
    MHW_MI_COPY_MEM_MEM_PARAMS          CopyMemMemParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    commonMiInterface = m_hwInterface->GetMiInterface();

    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);
    encodeStatusBuf   = &m_encodeStatusBuf;

    dwBaseOffset =
        (encodeStatusBuf->wCurrIndex * m_encodeStatusBuf.dwReportSize) +
        sizeof(uint32_t)* 2;  // pEncodeStatus is offset by 2 DWs in the resource

    MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiFlushDwCmd(cmdBuffer, &FlushDwParams));
    MHW_MI_STORE_REGISTER_MEM_PARAMS    MiStoreRegMemParams;
    MOS_ZeroMemory(&MiStoreRegMemParams, sizeof(MiStoreRegMemParams));
    MiStoreRegMemParams.presStoreBuffer = &encodeStatusBuf->resStatusBuffer;
    MiStoreRegMemParams.dwOffset        = dwBaseOffset + encodeStatusBuf->dwBSByteCountOffset;
    MiStoreRegMemParams.dwRegister      = mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &MiStoreRegMemParams));

    MOS_ZeroMemory(&CopyMemMemParams , sizeof(CopyMemMemParams));  
    CopyMemMemParams.presSrc        = &encodeStatusBuf->resStatusBuffer;
    CopyMemMemParams.dwSrcOffset    = dwBaseOffset + encodeStatusBuf->dwBSByteCountOffset;
    CopyMemMemParams.presDst        = &m_brcBuffers.resBrcBitstreamSizeBuffer;
    CopyMemMemParams.dwDstOffset    = CODECHAL_OFFSETOF(CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER, dwHcpBitstreamByteCountFrame);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(commonMiInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
        &CopyMemMemParams));
finish:
    return eStatus;
}
    
MOS_STATUS CodechalEncodeVp9::ExecuteKernelFunctions()
{
    MOS_SYNC_PARAMS                         syncParams;
    CodechalEncodeCscDs::KernelParams       cscScalingKernelParams;
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS       pVp9SeqParams;
    PCODEC_VP9_ENCODE_PIC_PARAMS            pVp9PicParams;
    MOS_STATUS                              status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf"))
    );

    MOS_ZeroMemory(&syncParams, sizeof(MOS_SYNC_PARAMS));
    // Wait on PAK, if its the P frame after I frame only
    if ((m_waitForPak) && !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    m_scalingEnabled = m_hmeSupported;

    // CSC, Scaling, BRC Init/Reset and HME are included in the same task phase
    m_lastEncPhase = false;
    m_firstTaskInPhase = true;

    if(m_brcEnabled)
      m_brcReset = m_vp9SeqParams->SeqFlags.fields.bResetBRC;

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    if (m_brcEnabled && (m_firstFrame || m_brcReset))
    {
        bool cscEnabled = m_cscDsState->RequireCsc() && m_firstField;
        m_lastTaskInPhase = !(cscEnabled || m_scalingEnabled || m_16xMeSupported || m_hmeEnabled);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcInitResetKernel());
    }

    // initialize ModeDecision buffers to 0 for I frame  
    if (m_pictureCodingType == I_TYPE)
    {
        uint32_t            i;
        MOS_LOCK_PARAMS LockFlags;
        uint8_t*           pbData;

        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
        LockFlags.WriteOnly = 1;

        //m_currentModeDecisionIndex = 0;

        for (i = 0; i < 2; i++)
        {
            pbData = (uint8_t*)m_osInterface->pfnLockResource(
			    m_osInterface,
			    &PakMode.Hw.resModeDecision[i],
			    &LockFlags);
            CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

            MOS_ZeroMemory(pbData, 16 * m_picWidthInMb * m_picHeightInMb * sizeof(uint32_t));
            m_osInterface->pfnUnlockResource(m_osInterface,
			    &PakMode.Hw.resModeDecision[i]);
        }


    }

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));

    if (m_hmeEnabled)
    {
        if (m_b16xMeEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(MeKernel());
        }

        m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MeKernel()); 
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

    // Call Idistortion and BRCUpdate kernels
    if (m_brcEnabled)
    {
        if (m_mbEncIFrameDistEnabled)
        {
           CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcIntraDistKernel());
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(BrcUpdateKernel());
        
        // Reset buffer ID used for BRC kernel performance reports
        m_osInterface->pfnResetPerfBufferID(m_osInterface);
    }

    if (m_pictureCodingType == I_TYPE)
    {
        // I_32x32
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32));

        // I_16x16
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel( CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16));
    }
    else
    {
        // P MB ENC
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel( CODECHAL_MEDIA_STATE_VP9_ENC_P));
    }

    // I_TX
    m_lastTaskInPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel( CODECHAL_MEDIA_STATE_VP9_ENC_TX));

    m_currentModeDecisionIndex ^= 1;

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams = g_cInitSyncParams;
        syncParams.GpuContext = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectRenderContextInUse;
 
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));	
        m_bWaitForENC = true;
    }
    return status;
}

MOS_STATUS CodechalEncodeVp9::MbEncKernel(CODECHAL_MEDIA_STATE_TYPE   EncFunctionType)
{
    PMHW_STATE_HEAP_INTERFACE                          stateHeapInterface;  
    MHW_INTERFACE_DESCRIPTOR_PARAMS                    idParams;
    uint32_t                                           dwKrnStateIdx;
    PMHW_KERNEL_STATE                                  kernelState;
    PMHW_KERNEL_STATE                                  kernelStateI32x32, kernelStateI16x16, kernelStateP, kernelStateTx;
    struct CodechalVp9MbencCurbeParams                 MbEncCurbeParams;
    struct CodechalVp9MbencSurfaceParams               MbEncSurfaceParams;
    void*                                              pBindingTable;
    SendKernelCmdsParams            		       sendKernelCmdsParams;
    CODECHAL_WALKER_CODEC_PARAMS                       walkerCodecParams;
    MHW_WALKER_PARAMS                                  walkerParams;    
    MOS_COMMAND_BUFFER                                 cmdBuffer;
    PCODEC_REF_LIST                                    *pRefList;
    PerfTagSetting                    		       PerfTag;
    uint32_t                                           dwResolutionX, dwResolutionY;
    MOS_STATUS                                         eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_refList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vp9PicParams);

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    pRefList = &m_refList[0];

    switch (EncFunctionType)
    {
        case CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32:
        {
            CODECHAL_ENCODE_SET_PERFTAG_INFO(PerfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_I_32x32);
            dwKrnStateIdx = CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_32x32;
            pBindingTable = (void*) &m_mbEncI32x32BindingTable;
            break;
        }
        case CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16:
        {
            CODECHAL_ENCODE_SET_PERFTAG_INFO(PerfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_I_16x16);
            dwKrnStateIdx = CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_16x16;
            pBindingTable = (void*) &m_mbEncI16x16BindingTable;
            break;
        }
        case CODECHAL_MEDIA_STATE_VP9_ENC_P:
        {
            CODECHAL_ENCODE_SET_PERFTAG_INFO(PerfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_P);
            dwKrnStateIdx = CODECHAL_ENCODE_VP9_MBENC_IDX_INTER;
            pBindingTable = (void*) &m_mbEncPBindingTable;
            break;
        }
        case CODECHAL_MEDIA_STATE_VP9_ENC_TX:
        {
            CODECHAL_ENCODE_SET_PERFTAG_INFO(PerfTag, CODECHAL_ENCODE_PERFTAG_CALL_MBENC_TX);
            dwKrnStateIdx = CODECHAL_ENCODE_VP9_MBENC_IDX_TX;
            pBindingTable = (void*) &m_mbEncTxBindingTable;
            break;
        }
        default:
            eStatus = MOS_STATUS_UNKNOWN;
            goto finish;
    }

    kernelState = &m_mbEncKernelStates[dwKrnStateIdx];
    kernelStateTx = &m_mbEncKernelStates[CODECHAL_ENCODE_VP9_MBENC_IDX_TX];

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    
    // Set curbe
    if (!m_mbEncCurbeSetInBrcUpdate)
    {
        if(EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32 || EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_P)
        {
            if (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32)
            {
		CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                    stateHeapInterface,
                    kernelState,
                    false,
                    m_mbEncDshSize,
                    false,
                    m_storeData));

                MOS_ZeroMemory(&idParams, sizeof(idParams));
                idParams.pKernelState = kernelState;
   	        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                    m_stateHeapInterface,
                    1,
                    &idParams));

                // Setup interface descriptor for I16x16
                kernelStateI16x16 = &m_mbEncKernelStates[CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_16x16];
                kernelStateI16x16->m_dshRegion = kernelState->m_dshRegion;
                MOS_ZeroMemory(&idParams, sizeof(idParams));
                idParams.pKernelState = kernelStateI16x16;
   	        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                    m_stateHeapInterface,
                    1,
                    &idParams));
            }
            else
            {
                //P frame
		CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                    stateHeapInterface,
                    kernelState,
                    false,
                    m_mbEncDshSize,
                    false,
                    m_storeData));

                MOS_ZeroMemory(&idParams, sizeof(idParams));
                idParams.pKernelState = kernelState;
   	        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                    m_stateHeapInterface,
                    1,
                    &idParams));
            }

            //Setup interface descriptor for TX
            kernelStateTx->m_dshRegion = kernelState->m_dshRegion;
            MOS_ZeroMemory(&idParams, sizeof(idParams));
            idParams.pKernelState = kernelStateTx;
   	    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                m_stateHeapInterface,
                1,
                &idParams));
            
            MOS_ZeroMemory(&MbEncCurbeParams, sizeof(struct CodechalVp9MbencCurbeParams));
            MbEncCurbeParams.pPicParams             = m_vp9PicParams;
            MbEncCurbeParams.pSeqParams             = m_vp9SeqParams; 
            MbEncCurbeParams.pSegmentParams         = m_vp9SegmentParams;
            MbEncCurbeParams.ppRefList              = &(m_refList[0]);
            MbEncCurbeParams.wPicWidthInMb          = m_picWidthInMb;
            MbEncCurbeParams.psLastRefPic           = m_lastRefPic;
            MbEncCurbeParams.psGoldenRefPic         = m_goldenRefPic;
            MbEncCurbeParams.psAltRefPic            = m_altRefPic;
            MbEncCurbeParams.wFieldFrameHeightInMb  = m_frameFieldHeightInMb;
            MbEncCurbeParams.bHmeEnabled            = m_hmeEnabled;
            MbEncCurbeParams.ucRefFrameFlags        = m_refFrameFlags;
            MbEncCurbeParams.wPictureCodingType     = m_pictureCodingType;
            MbEncCurbeParams.mediaStateType         = EncFunctionType;
            MbEncCurbeParams.pKernelState           = kernelState;
            MbEncCurbeParams.pKernelStateI32x32     = &m_mbEncKernelStates[CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_32x32];
            MbEncCurbeParams.bMbEncCurbeSetInBrcUpdate = m_mbEncCurbeSetInBrcUpdate;
           
	    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncCurbe(&MbEncCurbeParams));
	}

    }
    else
    {
        //I32x32 or Inter is setup by the BRC update kernel, TX curbe is setup by either I16x16 or P kernel

        if (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16)
        {
            kernelStateI32x32 = &m_mbEncKernelStates[CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_32x32];
            kernelState->m_dshRegion = kernelStateI32x32->m_dshRegion;
            MOS_ZeroMemory(&idParams, sizeof(idParams));
            idParams.pKernelState = kernelState;
   	    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                    m_stateHeapInterface,
                    1,
                    &idParams));
            
            //Set up TX CURBE
            kernelStateTx->m_dshRegion = kernelStateI32x32->m_dshRegion;
            MOS_ZeroMemory(&idParams, sizeof(idParams));
            idParams.pKernelState = kernelStateTx;
   	    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                    m_stateHeapInterface,
                    1,
                    &idParams));
        }

        if (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_P)
        {
            //Set up TX CURBE
            kernelStateTx->m_dshRegion = kernelState->m_dshRegion;
            MOS_ZeroMemory(&idParams, sizeof(idParams));
            idParams.pKernelState = kernelStateTx;
   	    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
                    m_stateHeapInterface,
                    1,
                    &idParams));
        }

    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    sendKernelCmdsParams = SendKernelCmdsParams();
    
    if (CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16 == EncFunctionType ||
        CODECHAL_MEDIA_STATE_VP9_ENC_P == EncFunctionType)
    {
        sendKernelCmdsParams.bEnable45ZWalkingPattern = true;
    }
   
    sendKernelCmdsParams.EncFunctionType = EncFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    // Add surface states
    MOS_ZeroMemory(&MbEncSurfaceParams, sizeof(struct CodechalVp9MbencSurfaceParams));
    MbEncSurfaceParams.MediaStateType               = EncFunctionType;
    MbEncSurfaceParams.wPictureCodingType           = m_pictureCodingType;
    MbEncSurfaceParams.dwOriFrameWidth              = m_frameWidth;
    MbEncSurfaceParams.dwOriFrameHeight             = m_frameHeight;
    MbEncSurfaceParams.dwFrameWidthInMb             = m_picWidthInMb;
    MbEncSurfaceParams.dwFrameFieldHeightInMb       = m_frameFieldHeightInMb;
    MbEncSurfaceParams.dwVerticalLineStride         = m_verticalLineStride;
    MbEncSurfaceParams.dwVerticalLineStrideOffset   = m_verticalLineStrideOffset;
    MbEncSurfaceParams.psCurrPicSurface             = &m_rawSurface;
    MbEncSurfaceParams.bHmeEnabled                  = m_hmeEnabled;
    MbEncSurfaceParams.bSegmentationEnabled         = m_vp9PicParams->PicFlags.fields.segmentation_enabled;
    MbEncSurfaceParams.psSegmentationMap            = &m_mbSegmentMapSurface;
    MbEncSurfaceParams.ps4xMeMvDataBuffer           = &m_s4XMemvDataBuffer;
    MbEncSurfaceParams.ps4xMeDistortionBuffer       = &m_s4XMeDistortionBuffer;
    MbEncSurfaceParams.presModeDecision             = &PakMode.Hw.resModeDecision[m_currentModeDecisionIndex];
    MbEncSurfaceParams.presModeDecisionPrevious     = &PakMode.Hw.resModeDecision[!m_currentModeDecisionIndex];
    MbEncSurfaceParams.psOutput16x16InterModes      = &m_output16x16InterModes;
    MbEncSurfaceParams.bUseEncryptedDsh             = m_encryptedDshInUse;
    MbEncSurfaceParams.presMbEncCurbeBuffer         = (m_encryptedDshInUse) ? &m_brcBuffers.resMbEncAdvancedDsh : NULL;
    MbEncSurfaceParams.ppRefList                    = &m_refList[0];
    MbEncSurfaceParams.psLastRefPic                 = m_lastRefPic;
    MbEncSurfaceParams.psGoldenRefPic               = m_goldenRefPic;
    MbEncSurfaceParams.psAltRefPic                  = m_altRefPic;
    MbEncSurfaceParams.presMbCodeSurface            = &m_resMbCodeSurface;
    MbEncSurfaceParams.pvBindingTable               = pBindingTable;
    MbEncSurfaceParams.pKernelState                 = kernelState;
    MbEncSurfaceParams.pKernelStateTx               = kernelStateTx;
    MbEncSurfaceParams.dwMbDataOffset = m_mvOffset;


#if USE_CODECHAL_DEBUG_TOOL
    if (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_TX)
    {
        MOS_LOCK_PARAMS LockFlags;
        uint8_t*           pbData; 

        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
        LockFlags.WriteOnly = 1;

        pbData = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface, &m_resMbCodeSurface, &LockFlags);
        CODECHAL_DEBUG_CHK_NULL(pbData);
       
        memset(pbData, 0, m_mbCodeSize);

        m_osInterface->pfnUnlockResource(m_osInterface, &m_resMbCodeSurface);
    }
#endif

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfaces(&cmdBuffer, &MbEncSurfaceParams, EncFunctionType));

    if (m_hwWalker)
    {
        dwResolutionX = (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32) ? (uint32_t)CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, 32) : (uint32_t)m_picWidthInMb;
        dwResolutionY = (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32) ? (uint32_t)CODECHAL_GET_HEIGHT_IN_BLOCKS(m_frameHeight, 32) : (uint32_t)m_picHeightInMb;

        MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
        walkerCodecParams.WalkerMode = m_walkerMode;
        walkerCodecParams.dwResolutionX = dwResolutionX;
        walkerCodecParams.dwResolutionY = dwResolutionY;
        walkerCodecParams.wPictureCodingType = m_pictureCodingType;
        walkerCodecParams.bUseScoreboard = (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_P || EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16) ? true : false;
        walkerCodecParams.bNoDependency = (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32 || EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_TX) ? true : false;
        walkerCodecParams.WalkerDegree = (EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_P || EncFunctionType == CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16) ? CODECHAL_45Z_DEGREE : CODECHAL_NO_DEGREE;
    
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
            m_hwInterface,
            &walkerParams,
            &walkerCodecParams));


        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));
	
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, EncFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

     CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        EncFunctionType,
        nullptr)));

     CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

finish:
    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::MeKernel()
{
    PMHW_STATE_HEAP_INTERFACE               stateHeapInterface;
    MOS_COMMAND_BUFFER                      cmdBuffer;
    PMHW_KERNEL_STATE                       kernelState;
    MHW_INTERFACE_DESCRIPTOR_PARAMS         idParams;
    struct CodechalVp9MeCurbeParams         MeParams;
    struct CodechalVp9MeSurfaceParams       meSurfaceParams;
    CODECHAL_MEDIA_STATE_TYPE               encFunctionType;
    SendKernelCmdsParams                    sendKernelCmdsParams;
    CODECHAL_WALKER_CODEC_PARAMS            walkerCodecParams;
    MHW_WALKER_PARAMS                       walkerParams;    
    PerfTagSetting         perfTag;
    bool                                    bUse16xMe;
    uint32_t                                dwResolutionX, dwResolutionY;
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    bUse16xMe = m_b16xMeEnabled && !m_b16xMeDone;

    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode >> CODECHAL_ENCODE_MODE_BIT_OFFSET;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    encFunctionType = (bUse16xMe) ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    kernelState = &m_meKernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    MeParams.pPicParams = m_vp9PicParams;
    MeParams.pSeqParams = m_vp9SeqParams;
    MeParams.dwFrameWidth = m_frameWidth;
    MeParams.dwFrameFieldHeight = m_frameFieldHeight;
    MeParams.ucRefFrameFlags = m_refFrameFlags;
    MeParams.b16xME = bUse16xMe;
    MeParams.b16xMeEnabled = m_b16xMeEnabled;
    MeParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeCurbe(&MeParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    //Add surface states
    MOS_ZeroMemory(&meSurfaceParams, sizeof(struct CodechalVp9MeSurfaceParams));
    meSurfaceParams.ppRefList = &m_refList[0];
    meSurfaceParams.pLastRefPic = m_lastRefPic ? &m_vp9PicParams->RefFrameList[m_vp9PicParams->RefFlags.fields.LastRefIdx] : NULL;
    meSurfaceParams.pGoldenRefPic = m_goldenRefPic ? &m_vp9PicParams->RefFrameList[m_vp9PicParams->RefFlags.fields.GoldenRefIdx] : NULL;
    meSurfaceParams.pAlternateRefPic = m_altRefPic ? &m_vp9PicParams->RefFrameList[m_vp9PicParams->RefFlags.fields.AltRefIdx] : NULL;
    meSurfaceParams.pCurrReconstructedPic= &m_currReconstructedPic;
    meSurfaceParams.ps4xMeMvDataBuffer = &m_s4XMemvDataBuffer;
    meSurfaceParams.ps16xMeMvDataBuffer = &m_s16XMemvDataBuffer;
    meSurfaceParams.psMeDistortionBuffer = &m_s4XMeDistortionBuffer;
    meSurfaceParams.psMeBrcDistortionBuffer = &m_s4XMeDistortionBuffer; // change it later for brc support
    meSurfaceParams.dwDownscaledWidthInMb =
        (bUse16xMe) ? m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    meSurfaceParams.dwDownscaledHeightInMb =
        (bUse16xMe) ? m_downscaledHeightInMb16x : m_downscaledHeightInMb4x;
    meSurfaceParams.dwVerticalLineStride = m_verticalLineStride;
    meSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    meSurfaceParams.dwEncodeWidth = m_oriFrameWidth;
    meSurfaceParams.dwEncodeHeight = m_oriFrameHeight;
    meSurfaceParams.b16xMeInUse = bUse16xMe;
    meSurfaceParams.b16xMeEnabled = m_b16xMeEnabled;
    meSurfaceParams.bDysEnabled = m_vp9SeqParams->SeqFlags.fields.EnableDynamicScaling;
    meSurfaceParams.pKernelState = kernelState;
    meSurfaceParams.pMeBindingTable = &m_meBindingTable;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(&cmdBuffer, &meSurfaceParams));

    dwResolutionX = (bUse16xMe) ?
        m_downscaledWidthInMb16x : m_downscaledWidthInMb4x;
    dwResolutionY = (bUse16xMe) ?
        m_downscaledFrameFieldHeightInMb16x : m_downscaledFrameFieldHeightInMb4x;

    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = dwResolutionX;
    walkerCodecParams.dwResolutionY = dwResolutionY;
    walkerCodecParams.bNoDependency = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
        stateHeapInterface));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));


     CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    if (bUse16xMe)
    {
        m_b16xMeDone = true;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::BrcIntraDistKernel() 
{
    PMHW_STATE_HEAP_INTERFACE                           stateHeapInterface;
    PMHW_KERNEL_STATE                                   kernelState;
    uint32_t                                            dwKrnStateIdx;  
    MHW_INTERFACE_DESCRIPTOR_PARAMS                     idParams;
    CodechalVp9BrcCurbeParams                           BrcIntraDistCurbeParams;
    CodechalVp9BrcIntraDistSurfaceParams                BrcIntraDistSurfaceParams;
    CodechalBindingTableVp9BrcIntraDist                 BindingTable;
    SendKernelCmdsParams                                sendKernelCmdsParams;
    MHW_WALKER_PARAMS                                   walkerParams;
    CODECHAL_WALKER_CODEC_PARAMS                        walkerCodecParams;
    uint32_t                                            dwResolutionX, dwResolutionY;
    MOS_COMMAND_BUFFER                                  cmdBuffer;
    PerfTagSetting                                      perfTag;
    uint8_t*                                            pbData;
    uint32_t                                            dwWidth, dwHeight, dwPitch;
    MOS_LOCK_PARAMS                                     LockFlags;
    CODECHAL_MEDIA_STATE_TYPE                           encFunctionType;
    MOS_STATUS                                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST);

    dwKrnStateIdx = CODECHAL_ENCODE_VP9_BRC_IDX_INTRA_DIST;

    kernelState        = &m_brcKernelStates[dwKrnStateIdx];

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);
    
    encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    BrcIntraDistCurbeParams.mediaStateType      = encFunctionType;
    BrcIntraDistCurbeParams.CurrPic             = m_currOriginalPic;
    BrcIntraDistCurbeParams.pPicParams          = m_vp9PicParams;
    BrcIntraDistCurbeParams.pSeqParams          = m_vp9SeqParams;
    BrcIntraDistCurbeParams.pSegmentParams      = m_vp9SegmentParams;
    BrcIntraDistCurbeParams.dwFrameWidth        = m_frameWidth;
    BrcIntraDistCurbeParams.dwFrameHeight       = m_frameHeight;
    BrcIntraDistCurbeParams.wPictureCodingType  = m_pictureCodingType;
    BrcIntraDistCurbeParams.bInitBrc            = m_firstFrame;
    BrcIntraDistCurbeParams.bMbBrcEnabled       = m_mbBrcEnabled;
    BrcIntraDistCurbeParams.ucRefFrameFlags     = m_refFrameFlags;
    BrcIntraDistCurbeParams.pdBrcInitCurrentTargetBufFullInBits    =
        &m_dBrcInitCurrentTargetBufFullInBits;
    BrcIntraDistCurbeParams.uiFramerate         = m_frameRate;
    BrcIntraDistCurbeParams.pKernelState        = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcCurbe(&BrcIntraDistCurbeParams));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));
   
     CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    //Add surface states
    BrcIntraDistSurfaceParams.ppRefList                  = &(m_refList[0]);
    BrcIntraDistSurfaceParams.pCurrOriginalPic           = &m_currOriginalPic;
    BrcIntraDistSurfaceParams.pCurrReconstructedPic      = &m_currReconstructedPic;
    BrcIntraDistSurfaceParams.psBrcDistortionBuffer      = &m_s4XMeDistortionBuffer;
    BrcIntraDistSurfaceParams.pvBindingTable             = (void*)&m_brcIntraDistBindingTable;
    BrcIntraDistSurfaceParams.pKernelState               = kernelState;
    
    //zeroize the distortion surface
    MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
    LockFlags.WriteOnly = 1;
    pbData = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcIntraDistSurfaceParams.psBrcDistortionBuffer->OsResource, &LockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

    dwWidth = BrcIntraDistSurfaceParams.psBrcDistortionBuffer->dwWidth;
    dwHeight = BrcIntraDistSurfaceParams.psBrcDistortionBuffer->dwHeight;
    dwPitch = BrcIntraDistSurfaceParams.psBrcDistortionBuffer->dwPitch;

    MOS_ZeroMemory(pbData,dwPitch*dwHeight);

    m_osInterface->pfnUnlockResource(
        m_osInterface,
        &BrcIntraDistSurfaceParams.psBrcDistortionBuffer->OsResource);
   
   CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcIntraDistSurfaces(
        &cmdBuffer,
        &BrcIntraDistSurfaceParams));
 
    dwResolutionX = (uint32_t)m_downscaledWidthInMb4x;
    dwResolutionY = (uint32_t)m_downscaledHeightInMb4x;

    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.bUseScoreboard = m_useHwScoreboard;
    walkerCodecParams.bNoDependency = true;
    walkerCodecParams.dwResolutionX = dwResolutionX;
    walkerCodecParams.dwResolutionY = dwResolutionY;
    walkerCodecParams.wPictureCodingType = m_pictureCodingType;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

     CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));
    
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
       CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
             stateHeapInterface));

       CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::BrcInitResetKernel()
{
    PMHW_STATE_HEAP_INTERFACE                           stateHeapInterface;
    CODECHAL_MEDIA_STATE_TYPE                           encFunctionType;
    PMHW_KERNEL_STATE                                   kernelState;
    uint32_t                                            dwKrnStateIdx;  
    MHW_INTERFACE_DESCRIPTOR_PARAMS                     idParams;
    struct CodechalVp9BrcCurbeParams                    BrcInitResetCurbeParams;
    struct CodechalVp9BrcInitResetSurfaceParams         BrcInitResetSurfaceParams;
    struct CodechalBindingTableVp9BrcInitReset          BindingTable;
    SendKernelCmdsParams             sendKernelCmdsParams;
    MHW_MEDIA_OBJECT_PARAMS                             MediaObjectParams;
    MOS_COMMAND_BUFFER                                  cmdBuffer;
    MOS_STATUS                                          status = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    encFunctionType = CODECHAL_MEDIA_STATE_BRC_INIT_RESET;

    dwKrnStateIdx = (m_brcReset && !m_firstFrame) ? CODECHAL_ENCODE_VP9_BRC_IDX_RESET : CODECHAL_ENCODE_VP9_BRC_IDX_INIT;

    kernelState  = &m_brcKernelStates[dwKrnStateIdx];

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelState);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        kernelState->KernelParams.iBTCount));
    
    m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(kernelState->KernelParams.iBTCount);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));


    BrcInitResetCurbeParams.mediaStateType      = encFunctionType;
    BrcInitResetCurbeParams.CurrPic             = m_currOriginalPic;
    BrcInitResetCurbeParams.pPicParams          = m_vp9PicParams;
    BrcInitResetCurbeParams.pSeqParams          = m_vp9SeqParams;
    BrcInitResetCurbeParams.pSegmentParams      = m_vp9SegmentParams;
    BrcInitResetCurbeParams.dwFrameWidth        = m_frameWidth;
    BrcInitResetCurbeParams.dwFrameHeight       = m_frameHeight;
    BrcInitResetCurbeParams.pdBrcInitCurrentTargetBufFullInBits    = &m_dBrcInitCurrentTargetBufFullInBits;
    BrcInitResetCurbeParams.pdwBrcInitResetBufSizeInBits           = &m_brcInitResetBufSizeInBits;
    BrcInitResetCurbeParams.pdBrcInitResetInputBitsPerFrame        = &m_dBrcInitResetInputBitsPerFrame;
    BrcInitResetCurbeParams.wPictureCodingType  = m_pictureCodingType;
    BrcInitResetCurbeParams.bInitBrc            = m_firstFrame;
    BrcInitResetCurbeParams.bMbBrcEnabled       = m_mbBrcEnabled;
    BrcInitResetCurbeParams.ucRefFrameFlags     = m_refFrameFlags;
    BrcInitResetCurbeParams.uiFramerate         = m_frameRate;
    BrcInitResetCurbeParams.pKernelState        = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcCurbe(&BrcInitResetCurbeParams));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    //Fixme: Add debug dump for kernel region, curbe etc

    MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams                    = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType    = encFunctionType;
    sendKernelCmdsParams.bBrcResetRequested = m_brcReset;
    sendKernelCmdsParams.pKernelState       = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        kernelState));

    //Add surface states
    BrcInitResetSurfaceParams.presBrcHistoryBuffer                  = &m_brcBuffers.resBrcHistoryBuffer;
    BrcInitResetSurfaceParams.psBrcDistortionBuffer                 = &m_s4XMeDistortionBuffer;
    BrcInitResetSurfaceParams.dwDownscaledWidthInMb4x               = m_downscaledWidthInMb4x;
    BrcInitResetSurfaceParams.dwDownscaledFrameHeightInMb4x         = m_downscaledHeightInMb4x;
    BrcInitResetSurfaceParams.dwBrcHistoryBufferSize                = m_brcHistoryBufferSize;
    BrcInitResetSurfaceParams.pvBindingTable                        = (void*)&m_brcInitResetBindingTable;
    BrcInitResetSurfaceParams.pKernelState                          = kernelState;
   
   CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcInitResetSurfaces(
        &cmdBuffer,
        &BrcInitResetSurfaceParams));
 
    MOS_ZeroMemory(&MediaObjectParams, sizeof(MediaObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &MediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
      //  m_stateHeapInterface,
        //kernelState));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
	m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }
   
 
    return status;
}

MOS_STATUS CodechalEncodeVp9::BrcUpdateKernel()
{
    PMHW_STATE_HEAP_INTERFACE                           stateHeapInterface;
    PMHW_KERNEL_STATE                                   brcKernelState, mbEncKernelState;
    uint32_t                                            dwBrcKrnStateIdx, dwMbEncKrnStateIdx;
    MHW_INTERFACE_DESCRIPTOR_PARAMS                     idParams;
    struct CodechalVp9MbencCurbeParams                  MbEncCurbeParams;
    struct CodechalVp9BrcCurbeParams                    BrcUpdateCurbeParams;
    struct CodechalVp9InitBrcConstantBufferParams       initBrcConstantBufferParams;
    PMOS_RESOURCE                                       presBrcImageStatesReadBuffer;
    struct CodechalVp9BrcUpdateSurfaceParams            BrcUpdateSurfaceParams;
    struct CodechalBindingTableVp9BrcUpdate             BindingTable;
    CODECHAL_MEDIA_STATE_TYPE                           encFunctionType, mbEncFunctionType;
    SendKernelCmdsParams                                sendKernelCmdsParams;
    MHW_MEDIA_OBJECT_PARAMS                             mediaObjectParams;
    MHW_WALKER_PARAMS                                   walkerParams;
    CODECHAL_WALKER_CODEC_PARAMS                        walkerCodecParams;
    MOS_COMMAND_BUFFER                                  cmdBuffer;
    PerfTagSetting                     perfTag;
    MOS_STATUS                                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetMiInterface());
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetRenderInterface());

    stateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);

    MOS_ZeroMemory(&perfTag, sizeof(PerfTagSetting));
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    encFunctionType   = CODECHAL_MEDIA_STATE_BRC_UPDATE;

    // Setup VP9 MbEnc Curbe 
    mbEncFunctionType = m_pictureCodingType == I_TYPE ? CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32 : CODECHAL_MEDIA_STATE_VP9_ENC_P;
    dwMbEncKrnStateIdx = m_pictureCodingType == I_TYPE ? CODECHAL_ENCODE_VP9_MBENC_IDX_INTRA_32x32 : CODECHAL_ENCODE_VP9_MBENC_IDX_INTER;
 
    mbEncKernelState        = &m_mbEncKernelStates[dwMbEncKrnStateIdx];

    CODECHAL_ENCODE_CHK_NULL_RETURN(mbEncKernelState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        mbEncKernelState,
        false,
        m_mbEncDshSize,
        !m_singleTaskPhaseSupported,
        m_storeData));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = mbEncKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
        stateHeapInterface,
        1,
        &idParams));

    MOS_ZeroMemory(&MbEncCurbeParams, sizeof(struct CodechalVp9MbencCurbeParams));
    MbEncCurbeParams.pPicParams             = m_vp9PicParams;
    MbEncCurbeParams.pSeqParams             = m_vp9SeqParams;
    MbEncCurbeParams.pSegmentParams         = m_vp9SegmentParams;
    MbEncCurbeParams.ppRefList              = &(m_refList[0]);
    MbEncCurbeParams.psLastRefPic           = m_lastRefPic;
    MbEncCurbeParams.psGoldenRefPic         = m_goldenRefPic;
    MbEncCurbeParams.psAltRefPic            = m_altRefPic;
    MbEncCurbeParams.wPicWidthInMb          = m_picWidthInMb;
    MbEncCurbeParams.wFieldFrameHeightInMb  = m_frameFieldHeightInMb;
    MbEncCurbeParams.bHmeEnabled            = m_hmeEnabled;
    MbEncCurbeParams.ucRefFrameFlags        = m_refFrameFlags;
    MbEncCurbeParams.bMultiRefQpCheck       = m_multiRefQPCheckEnabled; // Multi-Ref QP check should be on only when HME enabled.
    MbEncCurbeParams.wPictureCodingType     = m_pictureCodingType;
    MbEncCurbeParams.mediaStateType         = mbEncFunctionType;
    MbEncCurbeParams.pKernelState           = mbEncKernelState;
   
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMbEncCurbe(&MbEncCurbeParams)); 
    m_mbEncCurbeSetInBrcUpdate = true;

    dwBrcKrnStateIdx = CODECHAL_ENCODE_VP9_BRC_IDX_UPDATE;
    brcKernelState = &m_brcKernelStates[dwBrcKrnStateIdx];

    CODECHAL_ENCODE_CHK_NULL_RETURN(brcKernelState);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
        stateHeapInterface,
        brcKernelState->KernelParams.iBTCount));
    m_vmeStatesSize =
        m_hwInterface->GetKernelLoadCommandSize(brcKernelState->KernelParams.iBTCount);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());


    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                    stateHeapInterface,
		    brcKernelState,
                    false,
                    0,
                    false,
                    m_storeData));

    // Setup BRC Update Curbe
    BrcUpdateCurbeParams.mediaStateType         = encFunctionType;
    BrcUpdateCurbeParams.CurrPic                = m_currOriginalPic;
    BrcUpdateCurbeParams.pPicParams             = m_vp9PicParams;
    BrcUpdateCurbeParams.pSeqParams             = m_vp9SeqParams;
    BrcUpdateCurbeParams.pSegmentParams         = m_vp9SegmentParams;
    BrcUpdateCurbeParams.wPictureCodingType     = m_pictureCodingType;
    BrcUpdateCurbeParams.dwFrameWidthInMB       = m_picWidthInMb;
    BrcUpdateCurbeParams.dwFrameHeightInMB      = m_picHeightInMb;
    BrcUpdateCurbeParams.bHmeEnabled            = m_hmeEnabled;
    BrcUpdateCurbeParams.bUsedAsRef             = m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef;
    BrcUpdateCurbeParams.ucKernelMode           = m_kernelMode;
    BrcUpdateCurbeParams.dwHeaderBytesInserted  = m_headerBytesInserted;
    BrcUpdateCurbeParams.sFrameNumber           = m_frameNum;
    BrcUpdateCurbeParams.ucRefFrameFlags        = m_refFrameFlags;
    BrcUpdateCurbeParams.bMbBrcEnabled          = m_mbBrcEnabled;
    BrcUpdateCurbeParams.bMultiRefQpCheck       = m_multiRefQPCheckEnabled;
    BrcUpdateCurbeParams.pKernelState           = brcKernelState;
    BrcUpdateCurbeParams.dwBrcNumPakPasses      = m_numPasses;

    BrcUpdateCurbeParams.pdwBrcInitResetBufSizeInBits         = &m_brcInitResetBufSizeInBits;
    BrcUpdateCurbeParams.pdBrcInitResetInputBitsPerFrame      = &m_dBrcInitResetInputBitsPerFrame;
    BrcUpdateCurbeParams.pdBrcInitCurrentTargetBufFullInBits  = &m_dBrcInitCurrentTargetBufFullInBits;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBrcCurbe(
        &BrcUpdateCurbeParams));

    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = brcKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetInterfaceDescriptor(
          stateHeapInterface,
          1,
          &idParams));
   
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    sendKernelCmdsParams                    = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType    = encFunctionType;
    sendKernelCmdsParams.pKernelState       = brcKernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Check if the constant data surface is present
    if (m_brcConstantBufferSupported)
    {
        MOS_ZeroMemory(&initBrcConstantBufferParams, sizeof(initBrcConstantBufferParams));
        initBrcConstantBufferParams.pOsInterface                = m_osInterface;
        initBrcConstantBufferParams.resBrcConstantDataBuffer    = m_brcBuffers.resBrcConstantDataBuffer;
        initBrcConstantBufferParams.wPictureCodingType          = m_pictureCodingType;
	CODECHAL_ENCODE_CHK_STATUS_RETURN(InitBrcConstantBuffer(&initBrcConstantBufferParams));
    }
    
    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSetBindingTable(
        stateHeapInterface,
        brcKernelState));

    if (m_vp9PicParams->PicFlags.fields.segmentation_enabled)
    {
        CodecHalGetResourceInfo(m_osInterface, &m_mbSegmentMapSurface);
    }

    m_vp9PicParams->filter_level = 0; // clear the filter level value in picParams ebfore programming pic state, as this value will be determined and updated by BRC.

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructPicStateBatchBuf(
            &m_brcBuffers.resPicStateBrcReadBuffer));

    //Add surface states
    BrcUpdateSurfaceParams.wPictureCodingType                   = m_pictureCodingType;
    BrcUpdateSurfaceParams.MbEncMediaStateType                  = mbEncFunctionType;
    BrcUpdateSurfaceParams.presBrcHistoryBuffer                 = &m_brcBuffers.resBrcHistoryBuffer;
    BrcUpdateSurfaceParams.dwBrcHistoryBufferSize               = m_brcHistoryBufferSize;
    BrcUpdateSurfaceParams.psBrcDistortionBuffer                = &m_s4XMeDistortionBuffer;
    BrcUpdateSurfaceParams.presBrcConstantDataBuffer            = &m_brcBuffers.resBrcConstantDataBuffer;
    BrcUpdateSurfaceParams.dwDownscaledWidthInMb4x              = m_downscaledWidthInMb4x;
    BrcUpdateSurfaceParams.dwDownscaledFrameFieldHeightInMb4x   = m_downscaledFrameFieldHeightInMb4x;
    BrcUpdateSurfaceParams.presMbCodeBuffer                     = &m_resMbCodeSurface;
    BrcUpdateSurfaceParams.presMbEncCurbeWriteBuffer            = &m_brcBuffers.resBrcMbEncCurbeWriteBuffer;
    BrcUpdateSurfaceParams.presPicStateReadBuffer               = &m_brcBuffers.resPicStateBrcReadBuffer;
    BrcUpdateSurfaceParams.presPicStateWriteBuffer              = &m_brcBuffers.resPicStateBrcWriteHucReadBuffer;
    BrcUpdateSurfaceParams.presSegmentStateReadBuffer           = &m_brcBuffers.resSegmentStateBrcReadBuffer;
    BrcUpdateSurfaceParams.presSegmentStateWriteBuffer          = &m_brcBuffers.resSegmentStateBrcWriteBuffer;
    BrcUpdateSurfaceParams.presBrcMsdkPakBuffer                 = &m_brcBuffers.resBrcMsdkPakBuffer;
    BrcUpdateSurfaceParams.presBrcHucData                       = &m_brcBuffers.resBrcHucDataBuffer;
    BrcUpdateSurfaceParams.presBrcBitstreamSizeData             = &m_brcBuffers.resBrcBitstreamSizeBuffer;
    BrcUpdateSurfaceParams.pKernelStateMbEnc                    = mbEncKernelState;
    BrcUpdateSurfaceParams.pvBindingTable                       = (void*)&m_brcUpdateBindingTable;
    BrcUpdateSurfaceParams.pKernelState                         = brcKernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcUpdateSurfaces(
        &cmdBuffer,
        &BrcUpdateSurfaceParams));

    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnSubmitBlocks(
      //  stateHeapInterface,
        //brcKernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase) {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(stateHeapInterface->pfnUpdateGlobalCmdBufId(
            stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
   }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;

    }
  
    return eStatus;
}

MOS_STATUS CodechalEncodeVp9::Initialize(CodechalSetting * codecHalSettings)
{
    MOS_STATUS                      status = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::Initialize(codecHalSettings));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);
    
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));

    uint32_t                        dwVDEncPictureStatesSize, dwVDEncPicturePatchListSize;

    m_currentModeDecisionIndex = 0;
    
    // Application needs to pass the maxinum frame width/height 
    uint32_t                   dwMaxPicWidthInSB, dwMaxPicHeightInSB, dwMaxPicSizeInSB;
    uint32_t                   dwMaxNumCuRecords;
    m_dwMaxPicWidth = m_frameWidth;
    m_dwMaxPicHeight = m_frameHeight;
    dwMaxPicWidthInSB = MOS_ROUNDUP_DIVIDE(m_dwMaxPicWidth, CODEC_VP9_SUPER_BLOCK_WIDTH);
    dwMaxPicHeightInSB = MOS_ROUNDUP_DIVIDE(m_dwMaxPicHeight, CODEC_VP9_SUPER_BLOCK_HEIGHT);
    dwMaxPicSizeInSB = dwMaxPicWidthInSB * dwMaxPicHeightInSB;
    dwMaxNumCuRecords = dwMaxPicSizeInSB * 64;

    // MVOffset should be 4KB aligned
    m_mvOffset = MOS_ALIGN_CEIL((dwMaxPicSizeInSB * 4 * sizeof(uint32_t)), 0x1000) + 4096; // 3 uint32_t for HCP_PAK_OBJECT and 1 uint32_t for padding zero in kernel
    m_mbCodeSize =
            m_mvOffset + (dwMaxNumCuRecords * 64) + 0x1000;

    // for VP9: the Ds+Copy kernel is by default used to do CSC and copy non-aligned surface
    m_cscDsState->EnableCopy();
    m_cscDsState->EnableColor();

    // HME enabled by default for VP9
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ME_ENABLE_ID,
        &UserFeatureData, m_osInterface->pOsContext);
    m_hmeSupported = (UserFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_16xME_ENABLE_ID,
        &UserFeatureData, m_osInterface->pOsContext);
    m_16xMeSupported = (UserFeatureData.i32Data) ? true : false;

    // Multi-Pass BRC: currently disabled by default, plan to enable by default after c-model matching
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_ENABLE_ID,
        &UserFeatureData, m_osInterface->pOsContext);
    m_multipassBrcSupported = (UserFeatureData.i32Data) ? true : false;

    // Adaptive Repak: currently disabled by default, plan to enable by default after c-model matching
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_ENABLE_ID,
        &UserFeatureData, m_osInterface->pOsContext);
    m_adaptiveRepakSupported = (UserFeatureData.i32Data) ? true : false;

    m_multiRefQPCheckEnabled = false;

    //Fixme:
#if 0
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        NULL,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &UserFeatureData);
    m_SingleTaskPhaseSupported = (UserFeatureData.i32Data) ? true : false;
#endif
    // Disable singletaskphase for now since it failed on simulation
    m_singleTaskPhaseSupported = false;


    // disable superHME when HME is disabled
    if (m_hmeSupported == false)
    {
        m_16xMeSupported = false;
    }

    //To match with legacy i965
    m_minScaledDimension = 32;
    m_minScaledDimensionInMb = 2;

    MotionEstimationDisableCheck();

    if (CodecHalUsesRenderEngine(m_codecFunction, CODECHAL_VP9))
    {
	CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelState());

        if (m_singleTaskPhaseSupported)
        {
            uint32_t i;
            uint32_t dwScalingBtCount, dwMeBtCount, dwMbEncBtCount, dwBrcBtCount, dwEncOneBtCount, dwEncTwoBtCount;
            auto btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

            dwScalingBtCount = MOS_ALIGN_CEIL(
                m_scaling4xKernelStates[0].KernelParams.iBTCount,
                btIdxAlignment);

            dwMeBtCount = MOS_ALIGN_CEIL(
                m_meKernelState.KernelParams.iBTCount,
                btIdxAlignment);

            dwMbEncBtCount = 0;
            for (i = 0; i < CODECHAL_ENCODE_VP9_MBENC_IDX_NUM; i++)
            {
                dwMbEncBtCount = MOS_ALIGN_CEIL(
                    m_mbEncKernelStates[i].KernelParams.iBTCount,
                    btIdxAlignment);
            }
       
            dwBrcBtCount = 0;
            for (i = 0; i < CODECHAL_ENCODE_VP9_BRC_IDX_NUM; i++)
            {
                dwBrcBtCount += MOS_ALIGN_CEIL(
                    m_brcKernelStates[i].KernelParams.iBTCount,
                    btIdxAlignment);
            }

            dwEncOneBtCount = dwScalingBtCount + dwMeBtCount;
            dwEncOneBtCount += (m_16xMeSupported) ? dwEncOneBtCount : 0;
	    //Fixme
            //dwEncOneBtCount += (pEncoder->b32xMeSupported) ? dwEncOneBtCount : 0;
            dwEncTwoBtCount = dwMbEncBtCount + dwBrcBtCount;
            m_maxBtCount = MOS_MAX(dwEncOneBtCount, dwEncTwoBtCount);
        }
    }

    // enable codec specific registry key reporting
    m_userFeatureKeyReport = true;

    //Picture Level Commands
    MHW_VDBOX_STATE_CMDSIZE_PARAMS stateCmdSizeParams;
	  CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_hwInterface->GetHxxStateCommandSize(
            CODECHAL_ENCODE_MODE_VP9,
            &m_pictureStatesSize,
            &m_picturePatchListSize,
            &stateCmdSizeParams));

        
        // Slice Level Commands
	CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_hwInterface->GetHxxPrimitiveCommandSize(
            CODECHAL_ENCODE_MODE_VP9,
            &m_sliceStatesSize,
            &m_slicePatchListSize,
            m_singleTaskPhaseSupported));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMmcState());

    m_brcDistortionBufferSupported = true;
    m_brcConstantBufferSupported   = true;
    return status;
} 

//------------------------------------------------------------------------------
//| Purpose:    Reports registry keys used for VP9 encoding
//| Return:     N/A
//------------------------------------------------------------------------------
MOS_STATUS CodechalEncodeVp9::UserFeatureKeyReport()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_BRC_IN_USE_ID, m_brcEnabled, m_osInterface->pOsContext);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_MULTIPASS_BRC_IN_USE_ID, m_multipassBrcSupported, m_osInterface->pOsContext);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ADAPTIVE_REPAK_IN_USE_ID, m_adaptiveRepakSupported, m_osInterface->pOsContext);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_ME_ENABLE_ID,  m_hmeEnabled, m_osInterface->pOsContext);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VP9_ENCODE_16xME_ENABLE_ID, m_b16xMeEnabled, m_osInterface->pOsContext);

#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_VDENC_IN_USE_ID, m_vdencEnabled, m_osInterface->pOsContext);
#endif

    return eStatus;
}

/*----------------------------------------------------------------------------
| Name      : GetStatusReport
| Purpose   : Enable VP9 status update for longterm reference enabling from the app.
|             The driver reads the longterm reference suggestion from MSDK Pak Surface
|             and put it in the CODECHAL_ENCODE_STATUS_REPORT
\---------------------------------------------------------------------------*/
MOS_STATUS CodechalEncodeVp9::GetStatusReport(
        EncodeStatus       *pEncodeStatus,
        EncodeStatusReport *pEncodeStatusReport)
{
    PCODECHAL_ENCODE_VP9_BRC_OUTPUT_STATIC_DATA pCmd;
    PMOS_RESOURCE                               pPakBuffer;
    uint8_t*                                    pbData;
    MOS_LOCK_PARAMS                             LockFlagsReadOnly;
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pEncodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pEncodeStatusReport);

    if (m_brcEnabled == true)
    {
        pPakBuffer = &m_brcBuffers.resBrcMsdkPakBuffer;

        MOS_ZeroMemory(&LockFlagsReadOnly, sizeof(MOS_LOCK_PARAMS));
        LockFlagsReadOnly.ReadOnly = 1;

        pbData = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, pPakBuffer, &LockFlagsReadOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(pbData);

        pCmd = (PCODECHAL_ENCODE_VP9_BRC_OUTPUT_STATIC_DATA)pbData;

        pEncodeStatusReport->LongTermIndication = (uint8_t)pCmd->DW0.LongTermReferenceFlag;
        pEncodeStatusReport->NextFrameWidthMinus1 = (uint16_t) pCmd->DW1.NewFrameWidth - 1;
        pEncodeStatusReport->NextFrameHeightMinus1 = (uint16_t) pCmd->DW2.NewFrameHeight - 1;

        m_osInterface->pfnUnlockResource(m_osInterface, pPakBuffer);
    }

    pEncodeStatusReport->bitstreamSize =
        pEncodeStatus->dwMFCBitstreamByteCountPerFrame + pEncodeStatus->dwHeaderBytesInserted;

    pEncodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;

    return eStatus;
}
