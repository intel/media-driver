/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     codec_def_common_vvc.h
//! \brief    Defines VVC types and macros shared by CodecHal Decoder/Encoder, MHW, and DDI layer
//! \details  This is the base header for all codec_def VVC files. All codec_def vvc files should include this file which should not contain any DDI specific code.
//!
#ifndef __CODEC_DEF_COMMON_VVC_H__
#define __CODEC_DEF_COMMON_VVC_H__

#include "codec_def_common.h"

static const uint32_t vvcMinBlockWidth          = 8;              // Min block width
static const uint32_t vvcMinBlockHeight         = 8;              // Min block height
static const uint32_t vvcCtu32                  = 32;             // CTU 32x32
static const uint32_t vvcCtu64                  = 64;             // CTU 64x64
static const uint32_t vvcCtu128                 = 128;            // CTU 128x128
static const uint32_t vvcMaxTileWCtb32          = 263;            // Max Tile Width for CTU 32x32
static const uint32_t vvcMaxTileWCtb64          = 131;            // Max Tile Width for CTU 64x64
static const uint32_t vvcMaxTileWCtb128         = 65;             // Max Tile Width for CTU 128x128

//!
//! \enum     vvcSliceType
//! \brief    VVC slice type
//!
enum VvcSliceType
{
    vvcSliceB  = 0,
    vvcSliceP  = 1,
    vvcSliceI  = 2
};

//!
//! \enum VvcSurfaceId
//! VVC surface ID
//!
enum VvcSurfaceId
{
    vvcReconPic         = 0,    //!< reconstructed picture
    vvcSrcInputPic      = 1,    //!< input source picture of downscaled resolution (encoder only)
    vvcOrigUpscaledSrc  = 3,    //!< input source picture of original resolution (encoder only)
    vvcRefPic0          = 17,    //!< reference frame 0
    vvcRefPic1          = 18,    //!< reference frame 1
    vvcRefPic2          = 19,    //!< reference frame 2
    vvcRefPic3          = 20,    //!< reference frame 3
    vvcRefPic4          = 21,    //!< reference frame 4
    vvcRefPic5          = 22,    //!< reference frame 5
    vvcRefPic6          = 23,    //!< reference frame 6
    vvcRefPic7          = 24,    //!< reference frame 7
    vvcRefPic8          = 25,    //!< reference frame 8
    vvcRefPic9          = 26,    //!< reference frame 9
    vvcRefPic10         = 27,    //!< reference frame 10
    vvcRefPic11         = 28,    //!< reference frame 11
    vvcRefPic12         = 29,    //!< reference frame 12
    vvcRefPic13         = 30,    //!< reference frame 13
    vvcRefPic14         = 31,    //!< reference frame 14
};

// Forward Declarations
typedef struct _CODEC_REF_LIST_VVC CODEC_REF_LIST_VVC, *PCODEC_REF_LIST_VVC;

//!
//! \struct   _CODEC_REF_LIST_VVC
//! \brief    Codec reference list with VVC supported
//!

struct _CODEC_REF_LIST_VVC : public _CODEC_REF_LIST
{
    // Codec specific parameters
    // VVC
    struct
    {
        uint16_t        m_ppsPicWidthInLumaSamples;             // [8..16888]
        uint16_t        m_ppsPicHeightInLumaSamples;            // [8..16888]
        int32_t         m_ppsScalingWinLeftOffset;
        int32_t         m_ppsScalingWinRightOffset;
        int32_t         m_ppsScalingWinTopOffset;
        int32_t         m_ppsScalingWinBottomOffset;
        uint16_t        m_spsNumSubpicsMinus1;

        uint32_t        m_currPicScalWinWidthL;
        uint32_t        m_currPicScalWinHeightL;

        bool            m_spsHorCollocatedChromaFlag;
        bool            m_spsVerCollocatedChromaFlag;
    };
};

struct VvcRefFrameAttributes
{
    int32_t                         m_refscalingwinleftoffset;
    int32_t                         m_refscalingwinrightoffset;
    int32_t                         m_refscalingwintopoffset;
    int32_t                         m_refscalingwinbottomoffset;
    uint16_t                        m_refpicwidth;
    uint16_t                        m_refpicheight;
    uint32_t                        m_currPicScalWinWidthL;
    uint32_t                        m_currPicScalWinHeightL;
};


#endif  // __CODEC_DEF_COMMON_VVC_H__
