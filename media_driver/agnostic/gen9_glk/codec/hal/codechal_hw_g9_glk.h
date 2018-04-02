/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file      codechal_hw_g9_glk.h 
//! \brief         This modules implements HW interface layer to be used on kbl on all operating systems/DDIs, across CODECHAL components.
//!
#ifndef __CODECHAL_HW_G9_GLK_H__
#define __CODECHAL_HW_G9_GLK_H__

#include "codechal_hw_g9_X.h"

//!  Codechal hw interface Gen9 Glk
/*!
This class defines the interfaces for hardware dependent settings and functions used in Codechal for Gen9 Glk platforms
*/
class CodechalHwInterfaceG9Glk : public CodechalHwInterfaceG9
{
protected:
    static const CODECHAL_SSEU_SETTING m_defaultSsEuLutG9Glk[CODECHAL_NUM_MEDIA_STATES];

public:
    //!
    //! \brief    Constructor
    //!
    CodechalHwInterfaceG9Glk(
        PMOS_INTERFACE    osInterface,
        CODECHAL_FUNCTION codecFunction,
        MhwInterfaces     *mhwInterfaces)
        : CodechalHwInterfaceG9(osInterface, codecFunction, mhwInterfaces)
    {
        CODECHAL_HW_FUNCTION_ENTER;

        m_isVdencSuperSliceEnabled = true;
        m_ssEuTable = m_defaultSsEuLutG9Glk;       // currently initialize to BXT values. Will be tuned later.
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalHwInterfaceG9Glk() {}
};

// GLK default table. Currently initialized with dummy values, just as an example. Will be updated later.
const CODECHAL_SSEU_SETTING CodechalHwInterfaceG9Glk::m_defaultSsEuLutG9Glk[CODECHAL_NUM_MEDIA_STATES] =
{
    // Slice    Sub-Slice   EU      Rsvd(freq)
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_OLP
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_NORMAL
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_PERFORMANCE
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_QUALITY
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_32X_SCALING
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_16X_SCALING
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_4X_SCALING
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_32X_ME
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_16X_ME
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_4X_ME
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_BRC_INIT_RESET
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_BRC_UPDATE
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_BRC_BLOCK_COPY
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HYBRID_PAK_P1
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HYBRID_PAK_P2
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_I_FRAME_CHROMA
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_I_FRAME_LUMA
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_MPU_FHB
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_TPU_FHB
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_PA_COPY
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_PL2_COPY
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_ADV
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_2X_SCALING
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_32x32_PU_MODE_DECISION
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_16x16_PU_SAD
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_16x16_PU_MODE_DECISION
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_8x8_PU
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_8x8_PU_FMODE
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_32x32_B_INTRA_CHECK
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HEVC_B_MBENC
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_RESET_VLINE_STRIDE
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HEVC_B_PAK
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HEVC_BRC_LCU_UPDATE
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_I_32x32
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_I_16x16
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_P
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_ENC_TX
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_RECON
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_RECON
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_DEBLOCK_MASK
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_LUMA_DEBLOCK
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_CHROMA_DEBLOCK
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_MC_PRED
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_LUMA_RECON
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_CHROMA_RECON
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_LUMA_RECON
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_VP9_PAK_P_FRAME_INTRA_CHROMA_RECON
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_PREPROC
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_ENC_WP
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HEVC_I_MBENC
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_CSC_DS_COPY
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_2X_4X_SCALING
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_HEVC_LCU64_B_MBENC
    { 1,        3,        6,         0 },    // CODECHAL_MEDIA_STATE_MB_BRC_UPDATE
    { 1,        3,        6,         0 }     // CODECHAL_MEDIA_STATE_STATIC_FRAME_DETECTION
};

#endif // __CODECHAL_HW_G9_GLK_H__
