/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_preenc_defs.h
//! \brief    Defines the common interface for encode preenc basic feature
//!
#pragma once

#include "codec_def_common.h"
#include "codec_def_common_encode.h"
#if _MEDIA_RESERVED
#include "encode_preenc_defs_ext.h"
#endif // _MEDIA_RESERVED

#define CODEC_MAX_NUM_REF_FRAME_PREENC 15

namespace encode
{
enum MediaEncodeMode
{
    SINGLE_NORMAL_ENC = 0,  //000
    MANUAL_RES_PRE_ENC,     //001
    MANUAL_FULL_ENC,        //010
    SINGLE_PRE_FULL_ENC,    //011
    AUTO_RES_PRE_ENC = 5,   //101
};

enum EncodePreencDef0
{
    SINGLE_PASS_ENC = 0,
    PRE_ENC_PASS,
    FULL_ENC_PASS
};

typedef struct mfxI16PAIR
{
    short x, y;  // integer pair
} mfxI16PAIR;

typedef mfxI16PAIR I16PAIR;

struct EncodePreencDef1
{
    I16PAIR  mv;
    uint16_t intraSAD;
    uint16_t interSAD;
};

#undef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

struct PreEncInfo
{
    uint8_t  EncodePreEncInfo0 = 0;
    uint32_t EncodePreEncInfo1 = 0;
    uint8_t  EncodePreEncInfo2 = 0;
    uint8_t  EncodePreEncInfo3 = 0;
    uint32_t preEncSrcWidth    = 0;
    uint32_t preEncSrcHeight   = 0;
};

typedef struct _CODEC_PRE_ENC_PARAMS
{
    bool                    LowDelayMode;
    uint8_t                 BitDepthLumaMinus8;
    uint8_t                 BitDepthChromaMinus8;
    uint8_t                 CodingType;
    CODEC_PICTURE           CurrReconstructedPic;
    CODEC_PICTURE           RefFrameList[CODEC_MAX_NUM_REF_FRAME_PREENC];
    int32_t                 CurrPicOrderCnt;
    int32_t                 RefFramePOCList[CODEC_MAX_NUM_REF_FRAME_PREENC];
    CODEC_PICTURE           RefPicList[2][CODEC_MAX_NUM_REF_FRAME_PREENC];
    bool                    HierarchicalFlag;
    uint8_t                 HierarchLevelPlus1;
    uint8_t                 GopRefDist;
    uint8_t                 SliceType;
    CODEC_PICTURE           CurrOriginalPic;
    bool                    UsedAsRef;
    ENCODE_INPUT_COLORSPACE InputColorSpace;
    CODEC_PIC_ID            PicIdx[CODEC_MAX_NUM_REF_FRAME_PREENC];
    PCODEC_REF_LIST        *RefList;
    bool                    isPToB;

} CODEC_PRE_ENC_PARAMS, *PCODEC_PRE_ENC_PARAMS;

}  // namespace encode
