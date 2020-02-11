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
//! \file     codechal_vdenc_hevc_g12.cpp
//! \brief    HEVC VDEnc encoder for GEN12.
//!

#include "codechal_vdenc_hevc_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codeckrnheader.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igcodeckrn_g12.h"
#endif
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_hcp_g12_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "mhw_mi_g12_X.h"
#include "mhw_render_g12_X.h"
#include "codechal_mmc_encode_hevc_g12.h"
#include "media_user_settings_mgr_g12.h"
#include "mhw_mmio_g12.h"
#include "hal_oca_interface.h"

const uint32_t CodechalVdencHevcStateG12::m_VdboxVDENCRegBase[4] = M_VDBOX_VDENC_REG_BASE;

const double CodechalVdencHevcStateG12::m_devThreshIFPNEG[] = {
    0.80, 0.60, 0.34, 0.2,
};

const double CodechalVdencHevcStateG12::m_devThreshIFPPOS[] = {
    0.2, 0.4 , 0.66, 0.9,
};

const double CodechalVdencHevcStateG12::m_devThreshPBFPNEG[] = {
    0.90, 0.66, 0.46, 0.3,
};

const double CodechalVdencHevcStateG12::m_devThreshPBFPPOS[] = {
    0.3, 0.46, 0.70, 0.90,
};

const double CodechalVdencHevcStateG12::m_devThreshVBRNEG[] = {
    0.90, 0.70, 0.50, 0.3,
};

const double CodechalVdencHevcStateG12::m_devThreshVBRPOS[] = {
    0.4, 0.5, 0.75, 0.90,
};

const int8_t CodechalVdencHevcStateG12::m_lowdelayDevThreshPB[] = {
    -45, -33, -23, -15, -8, 0, 15, 25,
};
const int8_t CodechalVdencHevcStateG12::m_lowdelayDevThreshVBR[] = {
    -45, -35, -25, -15, -8, 0, 20, 40,
};
const int8_t CodechalVdencHevcStateG12::m_lowdelayDevThreshI[] = {
    -40, -30, -17, -10, -5, 0, 10, 20,
};

const int8_t CodechalVdencHevcStateG12::m_lowdelayDeltaFrmszI[][8] = {
    { 0,  0, -8, -12, -16, -20, -28, -36 },
    { 0,  0, -4, -8, -12,  -16, -24, -32 },
    { 4,  2,  0, -1, -3,  -8, -16, -24 },
    { 8,  4,  2,  0, -1,  -4,  -8, -16 },
    { 20, 16,  4,  0, -1,  -4,  -8, -16 },
    { 24, 20, 16,  8,  4,   0,  -4, -8 },
    { 28, 24, 20, 16,  8,   4,  0, -8 },
    { 32, 24, 20, 16, 8,   4,   0, -4 },
    { 64, 48, 28, 20, 16,  12,  8,  4 },
};

const int8_t CodechalVdencHevcStateG12::m_lowdelayDeltaFrmszP[][8] = {
    { -8,  -24, -32, -40, -44, -48, -52, -80 },
    { -8,  -16, -32, -40, -40,  -44, -44, -56 },
    { 0,    0,  -12, -20, -24,  -28, -32, -36 },
    { 8,   4,  0,   0,    -8,   -16,  -24, -32 },
    { 32,  16,  8, 4,    -4,   -8,  -16,  -20 },
    { 36,  24,  16, 8,    4,    -2,  -4, -8 },
    { 40, 36, 24,   20, 16,  8,  0, -8 },
    { 48, 40, 28,  24, 20,  12,  0, -4 },
    { 64, 48, 28, 20, 16,  12,  8,  4 },
};

const int8_t CodechalVdencHevcStateG12::m_lowdelayDeltaFrmszB[][8] = {
    { 0, -4, -8, -16, -24, -32, -40, -48 },
    { 1,  0, -4, -8, -16,  -24, -32, -40 },
    { 4,  2,  0, -1, -3,  -8, -16, -24 },
    { 8,  4,  2,  0, -1,  -4,  -8, -16 },
    { 20, 16,  4,  0, -1,  -4,  -8, -16 },
    { 24, 20, 16,  8,  4,   0,  -4, -8 },
    { 28, 24, 20, 16,  8,   4,  0, -8 },
    { 32, 24, 20, 16, 8,   4,   0, -4 },
    { 64, 48, 28, 20, 16,  12,  8,  4 },
};

const uint32_t CodechalVdencHevcStateG12::m_hucConstantData[]  = {
    0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x012c012c, 0x012c012c, 0x012c012c,
    0x012c012c, 0x012c012c, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00640064,
    0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064,
    0x00640064, 0x00640064, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x01900190, 0x012c012c,
    0x012c012c, 0x012c012c, 0x012c012c, 0x012c012c, 0x00c800c8, 0x00c800c8, 0x00c800c8, 0x00c800c8,
    0x00c800c8, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x00640064,
    0x00640064, 0x00640064, 0x00640064, 0x00640064, 0x503c1e04, 0xffc88c78, 0x3c1e0400, 0xc88c7850,
    0x140200ff, 0xa0824628, 0x0000ffc8, 0x00000000, 0x04030302, 0x00000000, 0x03030200, 0x0000ff04,
    0x02020000, 0xffff0303, 0x01000000, 0xff020202, 0x0000ffff, 0x02020100, 0x00fffffe, 0x01010000,
    0xfffffe02, 0x010000ff, 0xfefe0201, 0x0000ffff, 0xfe010100, 0x00fffffe, 0x01010000, 0x00000000,
    0x03030200, 0x00000004, 0x03020000, 0x00ff0403, 0x02000000, 0xff030302, 0x000000ff, 0x02020201,
    0x00ffffff, 0x02010000, 0xfffffe02, 0x01000000, 0xfffe0201, 0x0000ffff, 0xfe020101, 0x00fffffe,
    0x01010000, 0xfffffefe, 0x01000000, 0x00000001, 0x03020000, 0x00000403, 0x02000000, 0xff040303,
    0x00000000, 0x03030202, 0x0000ffff, 0x02020100, 0xffffff02, 0x01000000, 0xfffe0202, 0x000000ff,
    0xfe020101, 0x00ffffff, 0x02010100, 0xfffffefe, 0x01000000, 0xfffefe01, 0x000000ff, 0xe0e00101,
    0xc0d0d0d0, 0xe0e0b0c0, 0xd0d0d0e0, 0xf0f0c0d0, 0xd0e0e0e0, 0x0408d0d0, 0xe8f0f800, 0x1820dce0,
    0xf8fc0210, 0x2024ecf0, 0x0008101c, 0x2428f8fc, 0x08101418, 0x2830f800, 0x0c14181c, 0x3040fc00,
    0x0c10141c, 0xe8f80408, 0xc8d0d4e0, 0xf0f8b0c0, 0xccd4d8e0, 0x0000c0c8, 0xd8dce4f0, 0x0408d0d4,
    0xf0f80000, 0x0808dce8, 0xf0f80004, 0x0810dce8, 0x00080808, 0x0810f8fc, 0x08080808, 0x1010f800,
    0x08080808, 0x1020fc00, 0x08080810, 0xfc000408, 0xe0e8f0f8, 0x0001d0d8, 0xe8f0f8fc, 0x0204d8e0,
    0xf8fdff00, 0x0408e8f0, 0xfcff0002, 0x1014f0f8, 0xfcff0004, 0x1418f0f8, 0x00040810, 0x181cf8fc,
    0x04081014, 0x1820f800, 0x04081014, 0x3040fc00, 0x0c10141c, 0x40300408, 0x80706050, 0x30a0a090,
    0x70605040, 0xa0a09080, 0x60504030, 0xa0908070, 0x040201a0, 0x18141008, 0x02012420, 0x0a080604,
    0x01101010, 0x0c080402, 0x10101010, 0x05030201, 0x02010106, 0x00000503, 0xff030201, 0x02010000,
    0x000000ff, 0xfffefe01, 0xfdfd0100, 0xfb00ffff, 0xfffffefd, 0xfefdfbfa, 0x030201ff, 0x01010605,
    0x00050302, 0x03020101, 0x010000ff, 0x0000ff02, 0xffff0100, 0xfe0100ff, 0x00ffffff, 0xfffffefc,
    0xfefcfb00, 0x0101ffff, 0x01050402, 0x04020101, 0x01010000, 0x0000ff02, 0x00ff0101, 0xff000000,
    0x0100ffff, 0xfffffffe, 0xfffefd00, 0xfcfb00ff, 0x1efffffe, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000,
    0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10,
    0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x1e000000, 0x070d0e10, 0x00003207, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000,
    0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff,
    0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff,
    0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000,
    0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff,
    0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff, 0x0000ffff,
    0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff, 0xffffffff,
    0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff, 0xffffffff,
    0xffffffff, 0x0000ffff, 0xffffffff, 0xffff0000, 0xffffffff, 0xffffffff, 0xffff0000, 0x0000ffff,
    0xffffffff, 0xffffffff, 0x0000ffff, 0xffffffff
};

uint32_t CodechalVdencHevcStateG12::GetMaxBtCount()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t maxBtCount = 0;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    auto btIdxAlignment = m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment();

    // DsConversion kernel
    maxBtCount = 2 * (MOS_ALIGN_CEIL(m_cscDsState->GetBTCount(), btIdxAlignment));
#endif

    // add ME and stream-in later
    return maxBtCount;
}

MOS_STATUS CodechalVdencHevcStateG12::InitKernelStateMe()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface->pStateHeapInterface);

    uint32_t kernelSize = m_combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        m_kernelBinary,
        VDENC_ME_P,
        0,
        &currKrnHeader,
        &kernelSize));

    auto kernelStatePtr = &m_vdencMeKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_ME_P,
        &kernelStatePtr->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_ME_P,
        &m_vdencMeKernelBindingTable));

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary =
        m_kernelBinary +
        (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        m_kernelBinary,
        VDENC_ME_B,
        0,
        &currKrnHeader,
        &kernelSize));

    kernelStatePtr = &m_vdencMeKernelStateRAB;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_ME_B,
        &kernelStatePtr->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_ME_B,
        &m_vdencStreaminKernelBindingTable));

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary =
        m_kernelBinary +
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

MOS_STATUS CodechalVdencHevcStateG12::InitKernelStateStreamIn()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface->pStateHeapInterface);

    uint32_t kernelSize = m_combinedKernelSize;
    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        m_kernelBinary,
        VDENC_STREAMIN_HEVC,
        0,
        &currKrnHeader,
        &kernelSize));

    auto kernelStatePtr = &m_vdencStreaminKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_STREAMIN_HEVC,
        &kernelStatePtr->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_STREAMIN_HEVC,
        &m_vdencStreaminKernelBindingTable));

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary =
        m_kernelBinary +
        (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        m_kernelBinary,
        VDENC_STREAMIN_HEVC_RAB,
        0,
        &currKrnHeader,
        &kernelSize));

    kernelStatePtr = &m_vdencStreaminKernelStateRAB;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetKernelParams(
        VDENC_STREAMIN_HEVC_RAB,
        &kernelStatePtr->KernelParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetBindingTable(
        VDENC_STREAMIN_HEVC_RAB,
        &m_vdencStreaminKernelBindingTable));

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary =
        m_kernelBinary +
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

MOS_STATUS CodechalVdencHevcStateG12::InitKernelState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateMe());
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelStateStreamIn());
#endif

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::DecideEncodingPipeNumber()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    m_numPipePre = m_numPipe;
    m_numPipe = m_numVdbox;

    uint8_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;

    CODECHAL_ENCODE_VERBOSEMESSAGE("Tile Columns = %d.", numTileColumns);

    if (numTileColumns > m_numPipe)
    {
        // Streaming buffer does does work if numTileColumns > m_numPipe
        if (m_hevcSeqParams->EnableStreamingBufferLLC || m_hevcSeqParams->EnableStreamingBufferDDR)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Streaming buffer does does work if numTileColumns > m_numPipe!");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        m_numPipe = 1;
    }

    if (numTileColumns < m_numPipe)
    {
        if (numTileColumns >= 1 && numTileColumns <= 4)
        {
            m_numPipe = numTileColumns;
        }
        else
        {
            m_numPipe = 1;  // invalid tile column test cases and switch back to the single VDBOX mode
        }
    }

    // Tile replay needs scalability enabled, Remove Resolution check for scalability

    m_useVirtualEngine = true;  // always use virtual engine interface for single pipe and scalability mode

    m_numUsedVdbox       = m_numPipe;
    m_numberTilesInFrame = (m_hevcPicParams->num_tile_rows_minus1 + 1) * (m_hevcPicParams->num_tile_columns_minus1 + 1);

    if (m_scalabilityState)
    {
        // Create/ re-use a GPU context with 2 pipes
        m_scalabilityState->ucScalablePipeNum = m_numPipe;
    }

    CODECHAL_ENCODE_VERBOSEMESSAGE("System VDBOX number = %d, decided pipe num = %d.", m_numVdbox, m_numPipe);

    return eStatus;
}

bool CodechalVdencHevcStateG12::CheckSupportedFormat(PMOS_SURFACE surface)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    bool isColorFormatSupported = false;

    if (nullptr == surface)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid (nullptr) Pointer.");
        return isColorFormatSupported;
    }

    switch (surface->Format)
    {
    case Format_NV12:
    case Format_NV21:
    case Format_P010:       // Planar 4:2:0
    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
    case Format_A8R8G8B8:
    case Format_A8B8G8R8:
    case Format_R10G10B10A2:// Packed RGB 4:4:4
    case Format_B10G10R10A2:// Packed RGB 4:4:4
    case Format_AYUV:
    case Format_Y410:       // Packed 4:4:4
        isColorFormatSupported = true;
        break;
    case Format_Y210:       // Packed 4:2:2
        if (MEDIA_IS_WA(m_waTable, WaHEVCVDEncY210LinearInputNotSupported))
        {
            isColorFormatSupported = surface->TileType == MOS_TILE_Y;
        }
        else
        {
            isColorFormatSupported = true;
        }
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Input surface color format = %d not supported!", surface->Format);
        break;
    }

    return isColorFormatSupported;
}

MOS_STATUS CodechalVdencHevcStateG12::PlatformCapabilityCheck()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(DecideEncodingPipeNumber());

    if (MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeScalability_ChkGpuCtxReCreation(this, m_scalabilityState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    if (m_frameWidth * m_frameHeight > ENCODE_HEVC_MAX_16K_PIC_WIDTH * ENCODE_HEVC_MAX_16K_PIC_HEIGHT)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Frame resolution greater than 16k not supported");
    }

    //GopRefDist -- 0: All-Intra, 1: LowDelayMode, > 1: Random Access
    if (m_hevcSeqParams->GopRefDist > 1 && m_hevcSeqParams->TargetUsage == 7)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Random Access B in TU7 not supported");
    }

    // TU configuration for RDOQ
    if (m_hevcRdoqEnabled)
    {
        m_hevcRdoqEnabled = (m_hevcSeqParams->TargetUsage < 7);
    }

    // set RDOQ Intra blocks Threshold for Gen11+
    m_rdoqIntraTuThreshold = 0;
    if (m_hevcRdoqEnabled)
    {
        if (1 == m_hevcSeqParams->TargetUsage)
        {
            m_rdoqIntraTuThreshold = 0xffff;
        }
        else if (4 == m_hevcSeqParams->TargetUsage)
        {
            m_rdoqIntraTuThreshold = m_picWidthInMb * m_picHeightInMb;
            m_rdoqIntraTuThreshold = MOS_MIN(m_rdoqIntraTuThreshold / 10, 0xffff);
        }
    }

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_rsvdState->PlatformCapabilityCheck());
    }
#endif

    return eStatus;
}

CodechalVdencHevcStateG12::~CodechalVdencHevcStateG12()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_scalabilityState)
    {
        MOS_FreeMemAndSetNull(m_scalabilityState);
    }
    //Note: virtual engine interface destroy is done in MOS layer

    CODECHAL_DEBUG_TOOL(
        DestroyHevcPar();
        MOS_Delete(m_encodeParState);
    )
#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        MOS_Delete(m_rsvdState);
        m_rsvdState = nullptr;
    }
#endif
    if(m_gpuCtxCreatOpt)
    {
        MOS_Delete(m_gpuCtxCreatOpt);
        m_gpuCtxCreatOpt = nullptr;
    }
    return;
}

MOS_STATUS CodechalVdencHevcStateG12::AllocatePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint32_t mvt_size = MOS_ALIGN_CEIL(((m_frameWidth + 63) >> 6)*((m_frameHeight + 15) >> 4), 2) * CODECHAL_CACHELINE_SIZE;
    uint32_t mvtb_size = MOS_ALIGN_CEIL(((m_frameWidth + 31) >> 5)*((m_frameHeight + 31) >> 5), 2) * CODECHAL_CACHELINE_SIZE;
    m_sizeOfMvTemporalBuffer = MOS_MAX(mvt_size, mvtb_size);

    const uint32_t picWidthInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_LCU_SIZE);        //assume smallest LCU to get max width
    const uint32_t picHeightInMinLCU = MOS_ROUNDUP_DIVIDE(m_frameHeight, CODECHAL_HEVC_MIN_LCU_SIZE);      //assume smallest LCU to get max height

    MHW_VDBOX_HCP_BUFFER_SIZE_PARAMS hcpBufSizeParam;
    MOS_ZeroMemory(&hcpBufSizeParam, sizeof(hcpBufSizeParam));
    hcpBufSizeParam.ucMaxBitDepth = m_bitDepth;
    hcpBufSizeParam.ucChromaFormat = m_chromaFormat;
    // We should move the buffer allocation to picture level if the size is dependent on LCU size
    hcpBufSizeParam.dwCtbLog2SizeY = 6; //assume Max LCU size
    hcpBufSizeParam.dwPicWidth = MOS_ALIGN_CEIL(m_frameWidth, MAX_LCU_SIZE);
    hcpBufSizeParam.dwPicHeight = MOS_ALIGN_CEIL(m_frameHeight, MAX_LCU_SIZE);

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // Deblocking Filter Row Store Scratch data surface
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Row Store Scratch Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "DeblockingScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Row Store Scratch Buffer.");
        return eStatus;
    }

    // Deblocking Filter Tile Row Store Scratch data surface
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Tile Row Store Scratch Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "DeblockingTileRowScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterTileRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Tile Row Store Scratch Buffer.");
        return eStatus;
    }

    // Deblocking Filter Column Row Store Scratch data surface
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Deblocking Filter Tile Column Store Scratch Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "DeblockingColumnScratchBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resDeblockingFilterColumnRowStoreScratchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Deblocking Filter Tile Column Row Store Scratch Buffer.");
        return eStatus;
    }

    // Metadata Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Metadata Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "MetadataLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Line Buffer.");
        return eStatus;
    }

    // Metadata Tile Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Metadata Tile Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "MetadataTileLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataTileLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Tile Line Buffer.");
        return eStatus;
    }

    // Metadata Tile Column buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for Metadata Tile Column Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "MetadataTileColumnBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resMetadataTileColumnBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate Metadata Tile Column Buffer.");
        return eStatus;
    }

    // SAO Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Line Buffer.");
        return eStatus;
    }

    // SAO Tile Line buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Tile Line Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoTileLineBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoTileLineBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Tile Line Buffer.");
        return eStatus;
    }

    // SAO Tile Column buffer
    eStatus = (MOS_STATUS)m_hcpInterface->GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL,
        &hcpBufSizeParam);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to get the size for SAO Tile Column Buffer.");
        return eStatus;
    }

    allocParamsForBufferLinear.dwBytes = hcpBufSizeParam.dwBufferSize;
    allocParamsForBufferLinear.pBufName = "SaoTileColumnBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoTileColumnBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO Tile Column Buffer.");
        return eStatus;
    }

    // Lcu ILDB StreamOut buffer
    allocParamsForBufferLinear.dwBytes = CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "LcuILDBStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resLcuIldbStreamOutBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate LCU ILDB StreamOut Buffer.");
        return eStatus;
    }

    // Lcu Base Address buffer
    // HEVC Encoder Mode: Slice size is written to this buffer when slice size conformance is enabled.
    // 1 CL (= 16 DWs = 64 bytes) per slice * Maximum number of slices in a frame.
    // Align to page for HUC requirement
    uint32_t maxLcu = picWidthInMinLCU * picHeightInMinLCU;
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(maxLcu * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    allocParamsForBufferLinear.pBufName = "LcuBaseAddressBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resLcuBaseAddressBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate LCU Base Address Buffer.");
        return eStatus;
    }

    // SAO Row Store buffer
    // Aligned to 4 for each tile column
    uint32_t maxTileColumn = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE);
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(picWidthInMinLCU + 3 * maxTileColumn, 4) * 16;
    allocParamsForBufferLinear.pBufName = "SaoRowStoreBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencSAORowStoreBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO row store Buffer.");
        return eStatus;
    }

    // SAO StreamOut buffer
    uint32_t size = MOS_ALIGN_CEIL(picWidthInMinLCU, 4) * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU;
    //extra added size to cover tile enabled case, per tile width aligned to 4.  20: max tile column No.   
    size += 3 * 20 * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU;
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "SaoStreamOutBuffer";

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resSaoStreamOutBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate SAO StreamOut Buffer.");
        return eStatus;
    }

    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format = Format_Buffer;

    // Allocate Frame Statistics Streamout Data Destination Buffer. DW98-100 in HCP PipeBufAddr command
    size = MOS_ALIGN_CEIL(m_sizeOfHcpPakFrameStats * m_maxTileNumber, CODECHAL_PAGE_SIZE);  //Each tile has 9 cache size bytes of data, Align to page is HuC requirement
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "FrameStatStreamOutBuffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resFrameStatStreamOutBuffer),
        "Failed to create VDENC FrameStatStreamOutBuffer Buffer");

    // PAK Statistics buffer
    size = MOS_ALIGN_CEIL(m_vdencBrcPakStatsBufferSize, CODECHAL_PAGE_SIZE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, pakStats, "pakStats"));

    // Slice Count buffer 1 DW = 4 Bytes
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "Slice Count Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_sliceCountBuffer),
        "Failed to create VDENC Slice Count Buffer");

    // VDEncMode Timer buffer 1 DW = 4 Bytes
    allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(4, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.pBufName = "VDEncMode Timer Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_vdencModeTimerBuffer),
        "Failed to create VDEncMode Timer Buffer");

    if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_enableTileReplay)
    {
        uint32_t maxTileRow = MOS_ROUNDUP_DIVIDE(m_frameHeight, CODECHAL_HEVC_MIN_TILE_SIZE);
        uint32_t maxTileColumn = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE);

        allocParamsForBufferLinear.dwBytes = maxTileRow*maxTileColumn*(sizeof(HwCounter));
        allocParamsForBufferLinear.pBufName = "HWCounter";
        allocParamsForBufferLinear.bIsPersistent = true;
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resHwCountTileReplay),
            "Failed to create tile base HW counter buffer");
        allocParamsForBufferLinear.bIsPersistent = false;
    }

    uint32_t frameWidthInCus = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, CODECHAL_HEVC_MIN_CU_SIZE);
    uint32_t frameHeightInCus = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameHeight, CODECHAL_HEVC_MIN_CU_SIZE);
    uint32_t frameWidthInLcus = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, CODECHAL_HEVC_MAX_LCU_SIZE_G10);
    uint32_t frameHeightInLcus = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameHeight, CODECHAL_HEVC_MAX_LCU_SIZE_G10);
    uint32_t maxTileColumns    = MOS_ROUNDUP_DIVIDE(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE);

    // PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
    // One CU has 16-byte. But, each tile needs to be aliged to the cache line
    size = MOS_ALIGN_CEIL(frameWidthInCus * frameHeightInCus * 16, CODECHAL_CACHELINE_SIZE);
    allocParamsForBufferLinear.dwBytes = size;
    allocParamsForBufferLinear.pBufName = "PAK CU Level Streamout Data";

    CODECHAL_ENCODE_CHK_STATUS_RETURN((MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resPakcuLevelStreamoutData.sResource));
    m_resPakcuLevelStreamoutData.dwSize = size;
    CODECHAL_ENCODE_VERBOSEMESSAGE("first allocate cu steam out buffer, size=0x%x.\n", size);

    // these 2 buffers are not used so far, but put the correct size calculation here
    // PAK CU Level Streamout Data:   DW57-59 in HCP pipe buffer address command
    // One CU has 16-byte. But, each tile needs to be aliged to the cache line
    size = MOS_ALIGN_CEIL(frameWidthInCus * frameHeightInCus * 16, CODECHAL_CACHELINE_SIZE);

    // PAK Slice Level Streamut Data. DW60-DW62 in HCP pipe buffer address command
    // one LCU has one cache line. Use CU as LCU during creation
    size = frameWidthInLcus * frameHeightInLcus * CODECHAL_CACHELINE_SIZE;

    // Allocate SSE Source Pixel Row Store Buffer
    m_sizeOfSseSrcPixelRowStoreBufferPerLcu = CODECHAL_CACHELINE_SIZE * (4 + 4) << 1;
    allocParamsForBufferLinear.dwBytes      = 2 * m_sizeOfSseSrcPixelRowStoreBufferPerLcu * (m_widthAlignedMaxLcu + 3 * maxTileColumns);
    allocParamsForBufferLinear.pBufName = "SseSrcPixelRowStoreBuffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resSseSrcPixelRowStoreBuffer),
        "Failed to create SseSrcPixelRowStoreBuffer");

    //HCP scalability Sync buffer
    allocParamsForBufferLinear.dwBytes = CODECHAL_HEVC_MAX_NUM_HCP_PIPE * CODECHAL_CACHELINE_SIZE;
    allocParamsForBufferLinear.pBufName = "GEN11 HCP scalability Sync buffer ";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resHcpScalabilitySyncBuffer.sResource),
        "Failed to create GEN11 HCP scalability Sync Buffer");

    // create the tile coding state parameters
    for (auto i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; i++)
    {
        m_tileParams[i] = (PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12)MOS_AllocAndZeroMemory(
            sizeof(MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12)* m_maxTileNumber);
    }

    if (m_enableHWSemaphore)
    {
        // Create the HW sync objects which will be used by each reference frame and BRC in GEN11
        allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
        allocParamsForBufferLinear.pBufName = "SemaphoreMemory";

        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;

        uint32_t* data = nullptr;

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_refSync); i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                          m_osInterface,
                                                          &allocParamsForBufferLinear,
                                                          &m_refSync[i].resSemaphoreMem.sResource),
                "Failed to create HW Semaphore Memory.");
            m_refSync[i].resSemaphoreMem.dwSize = allocParamsForBufferLinear.dwBytes;

            CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint32_t *)m_osInterface->pfnLockResource(
                                                m_osInterface,
                                                &m_refSync[i].resSemaphoreMem.sResource,
                                                &lockFlagsWriteOnly));

            *data = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_refSync[i].resSemaphoreMem.sResource));
        }
    }

    // create the HW semaphore buffer to sync up between VDBOXes. This is used to WA HW internal lock issue
    if (m_enableVdBoxHWSemaphore)
    {
        allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
        allocParamsForBufferLinear.pBufName = "VDBOX SemaphoreMemory";

        MOS_LOCK_PARAMS lockFlagsWriteOnly;
        MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
        lockFlagsWriteOnly.WriteOnly = 1;

        uint32_t* data = nullptr;

        for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resVdBoxSemaphoreMem); i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                          m_osInterface,
                                                          &allocParamsForBufferLinear,
                                                          &m_resVdBoxSemaphoreMem[i].sResource),
                "Failed to create VDBOX HW Semaphore Memory.");

            CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint32_t *)m_osInterface->pfnLockResource(
                                                m_osInterface,
                                                &m_resVdBoxSemaphoreMem[i].sResource,
                                                &lockFlagsWriteOnly));

            *data = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_resVdBoxSemaphoreMem[i].sResource));
        }
    }

    uint32_t* data = nullptr;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
    allocParamsForBufferLinear.pBufName = "BrcPakSemaphoreMemory";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resBrcPakSemaphoreMem.sResource),
        "Failed to create BRC PAK Semaphore Memory.");

    CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint32_t *)m_osInterface->pfnLockResource(
                                        m_osInterface,
                                        &m_resBrcPakSemaphoreMem.sResource,
                                        &lockFlagsWriteOnly));

    *data = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resBrcPakSemaphoreMem.sResource));

    // 3rd level batch buffer
    // To be moved to a more proper place later
    MOS_ZeroMemory(&m_thirdLevelBatchBuffer, sizeof(m_thirdLevelBatchBuffer));
    m_thirdLevelBatchBuffer.bSecondLevel = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
        m_osInterface,
        &m_thirdLevelBatchBuffer,
        nullptr,
        m_thirdLBSize));

    if (m_enableTileStitchByHW)
    {
        if (Mos_ResourceIsNull(&m_resHucStatus2Buffer))
        {
            // HUC STATUS 2 Buffer for HuC status check in COND_BB_END
            allocParamsForBufferLinear.dwBytes = sizeof(uint64_t);
            allocParamsForBufferLinear.pBufName    = "HUC STATUS 2 Buffer";
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resHucStatus2Buffer),
                "%s: Failed to allocate HUC STATUS 2 Buffer\n",
                __FUNCTION__);
        }
        uint8_t *data;
        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            for (auto j = 0; j < CODECHAL_HEVC_MAX_NUM_BRC_PASSES; j++)
            {
                // HuC stitching Data buffer
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucCommandDataVdencG12), CODECHAL_PAGE_SIZE);
                allocParamsForBufferLinear.pBufName = "HEVC HuC Stitch Data Buffer";
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_resHucStitchDataBuffer[i][j]));
                MOS_LOCK_PARAMS lockFlagsWriteOnly;
                MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
                lockFlagsWriteOnly.WriteOnly = 1;
                uint8_t *pData = (uint8_t *)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_resHucStitchDataBuffer[i][j],
                    &lockFlagsWriteOnly);
                CODECHAL_ENCODE_CHK_NULL_RETURN(pData);
                MOS_ZeroMemory(pData, allocParamsForBufferLinear.dwBytes);
                m_osInterface->pfnUnlockResource(m_osInterface, &m_resHucStitchDataBuffer[i][j]);
            }
        }
        //Second level BB for huc stitching cmd
        MOS_ZeroMemory(&m_HucStitchCmdBatchBuffer, sizeof(m_HucStitchCmdBatchBuffer));
        m_HucStitchCmdBatchBuffer.bSecondLevel = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
            m_osInterface,
            &m_HucStitchCmdBatchBuffer,
            nullptr,
            m_hwInterface->m_HucStitchCmdBatchBufferSize));
    }

    if (m_numDelay)
    {
        allocParamsForBufferLinear.dwBytes = sizeof(uint32_t);
        allocParamsForBufferLinear.pBufName = "DelayMinusMemory";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resDelayMinus), "Failed to allocate delay minus memory.");

        uint8_t* data;
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resDelayMinus,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, sizeof(uint32_t));

        m_osInterface->pfnUnlockResource(m_osInterface, &m_resDelayMinus);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::FreePakResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_resSseSrcPixelRowStoreBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resHcpScalabilitySyncBuffer.sResource);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencSAORowStoreBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_resPakcuLevelStreamoutData.sResource);
    if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_enableTileReplay)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resHwCountTileReplay);
    }

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resTileBasedStatisticsBuffer); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resTileBasedStatisticsBuffer[i].sResource);
    }
    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_tileRecordBuffer); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_tileRecordBuffer[i].sResource);
    }
    m_osInterface->pfnFreeResource(m_osInterface, &m_resHuCPakAggregatedFrameStatsBuffer.sResource);

    m_osInterface->pfnFreeResource(m_osInterface, &m_resBrcDataBuffer);

    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resHucPakStitchDmemBuffer[k][i]);
        }
    }

    if (m_numDelay)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resDelayMinus);
    }

    for (auto i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; i++)
    {
        MOS_FreeMemory(m_tileParams[i]);
    }

    // command buffer for VE, allocated in MOS_STATUS CodechalEncodeHevcBase::VerifyCommandBufferSize()
    for (auto i = 0; i < CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC; i++)
    {
        for (auto j = 0; j < CODECHAL_HEVC_MAX_NUM_HCP_PIPE; j++)
        {
            for (auto k = 0; k < CODECHAL_HEVC_MAX_NUM_BRC_PASSES; k++)
            {
                PMOS_COMMAND_BUFFER cmdBuffer = &m_veBatchBuffer[i][j][k];

                if (!Mos_ResourceIsNull(&cmdBuffer->OsResource))
                {
                    if (cmdBuffer->pCmdBase)
                    {
                        m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
                    }
                    m_osInterface->pfnFreeResource(m_osInterface, &cmdBuffer->OsResource);
                }
            }
        }
    }

    for (auto i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_refSync); i++)
    {
        auto sync = &m_refSync[i];

        if (!Mos_ResourceIsNull(&sync->resSyncObject))
        {
            // if this object has been signaled before, we need to wait to ensure singal-wait is in pair.
            if (sync->uiSemaphoreObjCount || sync->bInUsed)
            {
                MOS_SYNC_PARAMS syncParams = g_cInitSyncParams;
                syncParams.GpuContext = m_renderContext;
                syncParams.presSyncResource = &sync->resSyncObject;
                syncParams.uiSemaphoreCount = sync->uiSemaphoreObjCount;
                m_osInterface->pfnEngineWait(m_osInterface, &syncParams);
            }
        }
        m_osInterface->pfnFreeResource(m_osInterface, &sync->resSemaphoreMem.sResource);
    }

    for (uint32_t i = 0; i < CODECHAL_GET_ARRAY_LENGTH(m_resVdBoxSemaphoreMem); i++)
    {
        m_osInterface->pfnFreeResource(m_osInterface, &m_resVdBoxSemaphoreMem[i].sResource);
    }

   if (m_enableTileStitchByHW)
    {
        for (auto i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
        {
            for (auto j = 0; j < CODECHAL_HEVC_MAX_NUM_BRC_PASSES; j++)
            {
                // HuC stitching Data buffer
                m_osInterface->pfnFreeResource(
                    m_osInterface,
                    &m_resHucStitchDataBuffer[i][j]);
            }
        }
        //Second level BB for huc stitching cmd
        Mhw_FreeBb(m_osInterface, &m_HucStitchCmdBatchBuffer, nullptr);
    }

    Mhw_FreeBb(m_osInterface, &m_thirdLevelBatchBuffer, nullptr);
    FreeTileLevelBatch();
    FreeTileRowLevelBRCBatch();

    m_osInterface->pfnFreeResource(m_osInterface, &m_resBrcPakSemaphoreMem.sResource);

    return CodechalVdencHevcState::FreePakResources();
}

MOS_STATUS CodechalVdencHevcStateG12::AllocateEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::AllocateEncResources());

    if (m_hmeSupported)
    {
        HmeParams hmeParams;

        MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
        hmeParams.b4xMeDistortionBufferSupported = true;
        hmeParams.ps16xMeMvDataBuffer = &m_s16XMeMvDataBuffer;
        hmeParams.ps32xMeMvDataBuffer = &m_s32XMeMvDataBuffer;
        hmeParams.ps4xMeDistortionBuffer = &m_s4XMeDistortionBuffer;
        hmeParams.ps4xMeMvDataBuffer = &m_s4XMeMvDataBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources4xME(&hmeParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources16xME(&hmeParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources32xME(&hmeParams));
    }

    // VDENC tile row store buffer
    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.dwBytes  = MOS_ROUNDUP_DIVIDE(m_frameWidth, 32) * CODECHAL_CACHELINE_SIZE * 2;
    allocParamsForBufferLinear.pBufName = "VDENC Tile Row Store Buffer";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_vdencTileRowStoreBuffer),
            "Failed to allocate VDENC Tile Row Store Buffer");

    MOS_ALLOC_GFXRES_PARAMS allocParamsForSurface;
    MOS_ZeroMemory(&allocParamsForSurface, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForSurface.Type = MOS_GFXRES_BUFFER;
    allocParamsForSurface.TileType = MOS_TILE_LINEAR;
    allocParamsForSurface.Format = Format_Buffer;
    allocParamsForSurface.dwBytes = m_numLcu * 4;
    allocParamsForSurface.pBufName = "VDEnc Cumulative CU Count Streamout Surface";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForSurface,
            &m_vdencCumulativeCuCountStreamoutSurface),
            "Failed to allocate VDEnc Cumulative CU Count Streamout Surface");

    // Move from CodechalVdencHevcState::AllocateEncResources()

    // PAK stream-out buffer
    allocParamsForBufferLinear.dwBytes = CODECHAL_HEVC_PAK_STREAMOUT_SIZE;
    allocParamsForBufferLinear.pBufName = "Pak StreamOut Buffer";
    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParamsForBufferLinear,
        &m_resStreamOutBuffer[0]),
        "Failed to allocate Pak Stream Out Buffer.");

    // VDENC Intra Row Store Scratch buffer
    // 1 cacheline per MB
    //  Double the size for Tile Replay
    uint32_t size = MOS_ROUNDUP_DIVIDE(m_frameWidth, MAX_LCU_SIZE) * CODECHAL_CACHELINE_SIZE * 2 * 2;
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, vdencIntraRowStoreScratch, "vdencIntraRowStoreScratch"));

    // VDENC Statistics buffer
    // Enabled for BRC only
    size = MOS_ALIGN_CEIL(m_vdencBrcStatsBufferSize * m_maxTileNumber, CODECHAL_PAGE_SIZE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_allocator->AllocateResource(
        m_standard, size, 1, vdencStats, "vdencStats"));

    // end of CodechalVdencHevcState::AllocateEncResources()

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_rsvdState->AllocateEncResources());
    }
#endif

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::FreeEncResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencTileRowStoreBuffer);
    m_osInterface->pfnFreeResource(m_osInterface, &m_vdencCumulativeCuCountStreamoutSurface);

    // Free ME resources
    HmeParams hmeParams;

    MOS_ZeroMemory(&hmeParams, sizeof(hmeParams));
    hmeParams.ps16xMeMvDataBuffer = &m_s16XMeMvDataBuffer;
    hmeParams.ps32xMeMvDataBuffer = &m_s32XMeMvDataBuffer;
    hmeParams.ps4xMeDistortionBuffer = &m_s4XMeDistortionBuffer;
    hmeParams.ps4xMeMvDataBuffer = &m_s4XMeMvDataBuffer;
    DestroyMEResources(&hmeParams);

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_rsvdState->FreeEncResources());
    }
#endif

    return CodechalVdencHevcState::FreeEncResources();
}

MOS_STATUS CodechalVdencHevcStateG12::AllocateBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::AllocateBrcResources());

    uint32_t* data = nullptr;
    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
    MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    allocParamsForBufferLinear.Format   = Format_Buffer;
    allocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
    allocParamsForBufferLinear.pBufName = "TileRowBRCSyncSemaphore";

    CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                  m_osInterface,
                                                  &allocParamsForBufferLinear,
                                                  &m_resTileRowBRCsyncSemaphore),
        "Failed to create Tile Row BRC sync Semaphore Memory.");

    CODECHAL_ENCODE_CHK_NULL_RETURN(data = (uint32_t *)m_osInterface->pfnLockResource(
                                        m_osInterface,
                                        &m_resTileRowBRCsyncSemaphore,
                                        &lockFlagsWriteOnly));

    *data = 0;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnUnlockResource(
        m_osInterface,
        &m_resTileRowBRCsyncSemaphore));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::FreeBrcResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_osInterface->pfnFreeResource(m_osInterface, &m_resTileRowBRCsyncSemaphore);
    return CodechalVdencHevcState::FreeBrcResources();
}

MOS_STATUS CodechalVdencHevcStateG12::AllocateTileLevelBatch()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Only allocate when the number of tile changed
    if (m_numTileBatchAllocated >= m_numTiles)
    {
        return eStatus;
    }

    // Make it simple, free first if need reallocate
    if (m_numTileBatchAllocated > 0)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(FreeTileLevelBatch());
    }

    // First allocate the batch buffer array
    for (int32_t idx = 0; idx < CODECHAL_VDENC_BRC_NUM_OF_PASSES; idx++)
    {
        if (m_tileLevelBatchBuffer[idx] == nullptr)
        {
            m_tileLevelBatchBuffer[idx] = (PMHW_BATCH_BUFFER)MOS_AllocAndZeroMemory(sizeof(MHW_BATCH_BUFFER) * m_numTiles);

            if (nullptr == m_tileLevelBatchBuffer[idx])
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Allocate memory for tile batch buffer failed");
                return MOS_STATUS_NO_SPACE;
            }  
        }

        // Allocate the batch buffer for each tile
        uint32_t  i = 0;
        for (i = 0; i < m_numTiles; i++)
        {
            MOS_ZeroMemory(&m_tileLevelBatchBuffer[idx][i], sizeof(MHW_BATCH_BUFFER));
            m_tileLevelBatchBuffer[idx][i].bSecondLevel = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_osInterface,
                &m_tileLevelBatchBuffer[idx][i],
                nullptr,
                m_tileLevelBatchSize));
        }
    }

    // Record the number of allocated batch buffer for tiles
    m_numTileBatchAllocated = m_numTiles;
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::FreeTileLevelBatch()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Free the batch buffer for each tile
    uint32_t  i = 0;
    uint32_t  j = 0;
    for (i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
    {
        for (j = 0; j < m_numTileBatchAllocated; j++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_FreeBb(m_osInterface, &m_tileLevelBatchBuffer[i][j], nullptr));
        }

        MOS_FreeMemory(m_tileLevelBatchBuffer[i]);
        m_tileLevelBatchBuffer[i] = nullptr;
    }

    // Reset the number of tile batch allocated
    m_numTileBatchAllocated = 0;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::AllocateTileRowLevelBRCBatch()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Only allocate when the number of tile row changed
    if (m_numTileRowBRCBatchAllocated >= m_numTileRows)
    {
        return eStatus;
    }

    // Make it simple, free first if need reallocate
    if (m_numTileRowBRCBatchAllocated > 0)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(FreeTileRowLevelBRCBatch());
    }

    // First allocate the batch buffer array
    for (int32_t idx = 0; idx < CODECHAL_VDENC_BRC_NUM_OF_PASSES; idx++)
    {
        if (m_TileRowBRCBatchBuffer[idx] == nullptr)
        {
            m_TileRowBRCBatchBuffer[idx] = (PMHW_BATCH_BUFFER)MOS_AllocAndZeroMemory(sizeof(MHW_BATCH_BUFFER) * m_numTileRows);

            if (nullptr == m_TileRowBRCBatchBuffer[idx])
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Allocate memory for tile row level BRC batch buffer failed");
                return MOS_STATUS_NO_SPACE;
            }
        }

        // Allocate the batch buffer for each tile row
        uint32_t  i = 0;
        for (i = 0; i < m_numTileRows; i++)
        {
            MOS_ZeroMemory(&m_TileRowBRCBatchBuffer[idx][i], sizeof(MHW_BATCH_BUFFER));
            m_TileRowBRCBatchBuffer[idx][i].bSecondLevel = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_AllocateBb(
                m_osInterface,
                &m_TileRowBRCBatchBuffer[idx][i],
                nullptr,
                m_hwInterface->m_hucCommandBufferSize));
        }
    }

    // Record the number of allocated batch buffer for tiles
    m_numTileRowBRCBatchAllocated = m_numTileRows;
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::FreeTileRowLevelBRCBatch()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Free the batch buffer for each tile row
    uint32_t  i = 0;
    uint32_t  j = 0;
    for (i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
    {
        for (j = 0; j < m_numTileRowBRCBatchAllocated; j++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_FreeBb(m_osInterface, &m_TileRowBRCBatchBuffer[i][j], nullptr));
        }

        MOS_FreeMemory(m_TileRowBRCBatchBuffer[i]);
        m_TileRowBRCBatchBuffer[i] = nullptr;
    }

    // Reset the number of tile row BRC batch allocated
    m_numTileRowBRCBatchAllocated = 0;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::InitializePicture(const EncoderParams& params)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // common initilization
    return CodechalVdencHevcState::InitializePicture(params);
}

MOS_STATUS CodechalVdencHevcStateG12::SetPictureStructs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::SetPictureStructs());

    if ((uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_chromaFormat &&
        (uint8_t)HCP_CHROMA_FORMAT_YUV422 == m_outputChromaFormat)
    {
        if (Format_YUY2 != m_reconSurface.Format)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Recon surface format is not correct!");
        }
        else if (m_reconSurface.dwHeight < m_oriFrameHeight * 2 ||
            m_reconSurface.dwWidth < m_oriFrameWidth / 2)
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(eStatus, "Recon surface allocation size is not correct!");
        }
        else
        {
            // update Recon surface to Variant format
            CodechalEncodeHevcBase::UpdateYUY2SurfaceInfo(&m_reconSurface, m_is10BitHevc);
        }
    }

    // Frame level BRC pass set to one pass when tile replay is enabled
    if (m_enableTileReplay)
    {
        m_numPasses = 0;
    }

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetPictureStructs();
    }
#endif

    // EOS is not working on GEN12, disable it by setting below to false (WA)
    m_lastPicInSeq = false;
    m_lastPicInStream = false;
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::GetStatusReport(
    EncodeStatus *encodeStatus,
    EncodeStatusReport *encodeStatusReport)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatus);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport);

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpVdencOutputs()));

    // When tile replay is enabled with tile replay, need to report out the tile size and the bit stream is not continous
    if ((encodeStatusReport->UsedVdBoxNumber == 1) && (!m_enableTileReplay || (m_enableTileReplay && encodeStatusReport->NumberTilesInFrame == 1)))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::GetStatusReport(encodeStatus, encodeStatusReport));
        return eStatus;
    }

    // Allocate the tile size report memory
    encodeStatusReport->SizeOfTileInfoBuffer = encodeStatusReport->NumberTilesInFrame * sizeof(CodechalTileInfo);
    if (encodeStatusReport->pHEVCTileinfo)
    {
        MOS_FreeMemory(encodeStatusReport->pHEVCTileinfo);
        encodeStatusReport->pHEVCTileinfo = nullptr;
    }
    encodeStatusReport->pHEVCTileinfo = (CodechalTileInfo *)MOS_AllocAndZeroMemory(encodeStatusReport->SizeOfTileInfoBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(encodeStatusReport->pHEVCTileinfo);

    // In case of CQP, PAK integration kernel is not called, so used tile size record from HW
    // PAK integration kernel does not handle stitching for single pipe mode
    PCODECHAL_ENCODE_BUFFER tileSizeStatusReport = &m_tileRecordBuffer[encodeStatusReport->CurrOriginalPic.FrameIdx];
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[encodeStatusReport->CurrOriginalPic.FrameIdx];

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;
    HCPPakHWTileSizeRecord_G12* tileStatusReport = (HCPPakHWTileSizeRecord_G12*)m_osInterface->pfnLockResource(
        m_osInterface,
        &tileSizeStatusReport->sResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileStatusReport);

    encodeStatusReport->CodecStatus = CODECHAL_STATUS_SUCCESSFUL;
    encodeStatusReport->PanicMode = false;
    encodeStatusReport->AverageQp = 0;
    encodeStatusReport->QpY = 0;
    encodeStatusReport->SuggestedQpYDelta = 0;
    encodeStatusReport->NumberPasses = 1;
    encodeStatusReport->bitstreamSize = 0;
    encodeStatus->ImageStatusCtrlOfLastBRCPass.hcpCumulativeFrameDeltaQp = 0;
    encodeStatusReport->NumberSlices = 0;

    uint32_t* sliceSize = nullptr;

    // pSliceSize is set/ allocated only when dynamic slice is enabled. Cannot use SSC flag here, as it is an asynchronous call
    if (encodeStatus->sliceReport.pSliceSize)
    {
        sliceSize = (uint32_t*)m_osInterface->pfnLockResource(m_osInterface, encodeStatus->sliceReport.pSliceSize, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(sliceSize);
    }

    uint32_t totalCU = 0;
    uint32_t sliceCount = 0;
    double sumQp = 0.0;
    for (uint32_t i = 0; i < encodeStatusReport->NumberTilesInFrame; i++)
    {
        if (tileStatusReport[i].Length == 0)
        {
            encodeStatusReport->CodecStatus = CODECHAL_STATUS_INCOMPLETE;
            return eStatus;
        }
        //update tile info with HW counter
        if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_enableTileReplay)
        {
            MOS_LOCK_PARAMS LockFlagsNoOverWrite;
            MOS_ZeroMemory(&LockFlagsNoOverWrite, sizeof(MOS_LOCK_PARAMS));
            LockFlagsNoOverWrite.WriteOnly = 1;
            LockFlagsNoOverWrite.NoOverWrite = 1;

            uint8_t* dataHWCountTileReplay = (uint8_t*)m_osInterface->pfnLockResource(
                                                       m_osInterface,
                                                       &m_resHwCountTileReplay,
                                                       &LockFlagsNoOverWrite);

            CODECHAL_ENCODE_CHK_NULL_RETURN(dataHWCountTileReplay);
            uint64_t *pAddress2Counter = (uint64_t *)(dataHWCountTileReplay + i * sizeof(HwCounter));
            encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.Count = *pAddress2Counter;
            encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.Count = SwapEndianness(encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.Count); //Report back in Big endian
            encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.IV = *(++pAddress2Counter);
            encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.IV = SwapEndianness(encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.IV); //Report back in Big endian
            CODECHAL_ENCODE_NORMALMESSAGE("tile = %d, hwCounterValue.Count = 0x%llx, hwCounterValue.IV = 0x%llx", i, encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.Count, encodeStatusReport->pHEVCTileinfo[i].HWCounterValue.IV);
            if (dataHWCountTileReplay)
            {
                m_osInterface->pfnUnlockResource(m_osInterface, &m_resHwCountTileReplay);
            }
        }
        encodeStatusReport->pHEVCTileinfo[i].TileSizeInBytes     = tileStatusReport[i].Length;
        // The offset only valid if there is no stream stitching
        encodeStatusReport->pHEVCTileinfo[i].TileBitStreamOffset = tileParams[i].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
        encodeStatusReport->pHEVCTileinfo[i].TileRowNum = i / tileParams[i].NumOfTileColumnsInFrame;
        encodeStatusReport->pHEVCTileinfo[i].TileColNum = i % tileParams[i].NumOfTileColumnsInFrame;
        encodeStatusReport->NumTileReported =  i + 1;
        encodeStatusReport->bitstreamSize += tileStatusReport[i].Length;
        totalCU += (tileParams[i].TileHeightInMinCbMinus1 + 1) * (tileParams[i].TileWidthInMinCbMinus1 + 1);
        sumQp += tileStatusReport[i].Hcp_Qp_Status_Count;

        if (sliceSize)
        {
            encodeStatusReport->pSliceSizes = (uint16_t*)sliceSize;
            encodeStatusReport->NumberSlices += (uint8_t)tileStatusReport[i].Hcp_Slice_Count_Tile;
            uint16_t prevCumulativeSliceSize = 0;
            // HW writes out a DW for each slice size. Copy in place the DW into 16bit fields expected by App
            for (uint32_t idx = 0; idx < tileStatusReport[i].Hcp_Slice_Count_Tile; idx++)
            {
                // PAK output the sliceSize at 16DW intervals. 
                CODECHAL_ENCODE_CHK_NULL_RETURN(&sliceSize[sliceCount * 16]);

                //convert cummulative slice size to individual, first slice may have PPS/SPS,
                uint32_t CurrAccumulatedSliceSize = sliceSize[sliceCount * 16];
                encodeStatusReport->pSliceSizes[sliceCount] = CurrAccumulatedSliceSize - prevCumulativeSliceSize;
                prevCumulativeSliceSize += encodeStatusReport->pSliceSizes[sliceCount];
                sliceCount++;
            }
        }
    }

    if (sliceSize)
    {
        encodeStatusReport->SizeOfSliceSizesBuffer = sizeof(uint16_t) * encodeStatusReport->NumberSlices;
        encodeStatusReport->SliceSizeOverflow = (encodeStatus->sliceReport.SliceSizeOverflow >> 16) & 1;
        m_osInterface->pfnUnlockResource(m_osInterface, encodeStatus->sliceReport.pSliceSize);
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CalculatePSNR(encodeStatus, encodeStatusReport));

    if (encodeStatusReport->bitstreamSize == 0 ||
        encodeStatusReport->bitstreamSize >m_bitstreamUpperBound)
    {
        encodeStatusReport->CodecStatus = CODECHAL_STATUS_ERROR;
        encodeStatusReport->bitstreamSize = 0;
        return MOS_STATUS_INVALID_FILE_SIZE;
    }

    if (totalCU != 0)
    {
        encodeStatusReport->QpY = encodeStatusReport->AverageQp =
            (uint8_t)((sumQp / (double)totalCU) / 4.0); // due to TU is 4x4 and there are 4 TUs in one CU
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (m_enableTileStitchByHW)
    {
        if (tileStatusReport)
        {
            // clean-up the tile status report buffer
            MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * encodeStatusReport->NumberTilesInFrame);
            m_osInterface->pfnUnlockResource(m_osInterface, &tileSizeStatusReport->sResource);
        }
        return eStatus;
    }

    //Driver stitching is not allowed for secure encode case
    if (!m_osInterface->osCpInterface->IsCpEnabled())
    {
        uint8_t *tempBsBuffer = nullptr, *bufPtr = nullptr;
        tempBsBuffer = bufPtr = (uint8_t*)MOS_AllocAndZeroMemory(encodeStatusReport->bitstreamSize);
        CODECHAL_ENCODE_CHK_NULL_RETURN(tempBsBuffer);

        PCODEC_REF_LIST currRefList = encodeStatus->encodeStatusReport.pCurrRefList;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.ReadOnly = 1;
        uint8_t* bitstream = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &currRefList->resBitstreamBuffer,
            &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(bitstream);

        for (uint32_t i = 0; i < encodeStatusReport->NumberTilesInFrame; i++)
        {
            uint32_t offset = tileParams[i].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE;
            uint32_t len = tileStatusReport[i].Length;

            MOS_SecureMemcpy(bufPtr, len, &bitstream[offset], len);
            bufPtr += len;
        }

        MOS_SecureMemcpy(bitstream, encodeStatusReport->bitstreamSize, tempBsBuffer, encodeStatusReport->bitstreamSize);
        MOS_ZeroMemory(&bitstream[encodeStatusReport->bitstreamSize], m_bitstreamUpperBound - encodeStatusReport->bitstreamSize);

        if (bitstream)
        {
            m_osInterface->pfnUnlockResource(m_osInterface, &currRefList->resBitstreamBuffer);
        }

        MOS_FreeMemory(tempBsBuffer);
    }

    if (tileStatusReport)
    {
        // clean-up the tile status report buffer
        MOS_ZeroMemory(tileStatusReport, sizeof(tileStatusReport[0]) * encodeStatusReport->NumberTilesInFrame);

        m_osInterface->pfnUnlockResource(m_osInterface, &tileSizeStatusReport->sResource);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ValidateRefFrameData(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    bool isRandomAccess = false;

    CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);

    if (slcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
    {
        if (slcParams->num_ref_idx_l0_active_minus1 != slcParams->num_ref_idx_l1_active_minus1)
        {
            isRandomAccess = true;
        }

        for (auto j = 0; j < CODEC_MAX_NUM_REF_FRAME_HEVC; j++)
        {
            if (slcParams->RefPicList[0][j].PicEntry != slcParams->RefPicList[1][j].PicEntry)
            {
                isRandomAccess = true;
            }
        }
    }

    if (isRandomAccess)
    {
        if (m_hevcPicParams->bEnableRollingIntraRefresh)
        {
            CODECHAL_ENCODE_ASSERT(false);
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
    }

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_rsvdState->ValidateRefFrameData(isRandomAccess));
    }
#endif

    uint8_t maxNumRef0 = isRandomAccess ? 2 : m_numMaxVdencL0Ref;
    uint8_t maxNumRef1 = isRandomAccess ? 1 : m_numMaxVdencL1Ref;

    if (slcParams->num_ref_idx_l0_active_minus1 > maxNumRef0 - 1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l0_active_minus1 = maxNumRef0 - 1;
    }

    if (slcParams->num_ref_idx_l1_active_minus1 > maxNumRef1 - 1)
    {
        CODECHAL_ENCODE_ASSERT(false);
        slcParams->num_ref_idx_l1_active_minus1 = maxNumRef1 - 1;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)
    CodecHalEncode_WriteKey64(__MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VE_DEBUG_OVERRIDE_G12, m_kmdVeOveride.Value);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENCODE_USED_VDBOX_NUM_ID, m_numPipe);
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID_G12, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));
#endif
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::EncodeKernelFunctions()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf")));

    CODECHAL_DEBUG_TOOL(
        PCODEC_PICTURE l0RefFrameList = m_hevcSliceParams->RefPicList[LIST_0];
        for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = l0RefFrameList[refIdx];

            if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
            {
                // L0 references
                uint8_t refPicIdx            = m_picIdx[refPic.FrameIdx].ucPicIdx;
                m_debugInterface->m_refIndex = (uint16_t)m_refList[refPicIdx]->iFieldOrderCnt[0];
                std::string refSurfName      = "RefSurf_List0_POC" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &m_refList[refPicIdx]->sRefBuffer,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data()))
            }
        }

        if (!m_lowDelay)
        {
            PCODEC_PICTURE l1RefFrameList = m_hevcSliceParams->RefPicList[LIST_1];
            for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
            {
                CODEC_PICTURE refPic = l1RefFrameList[refIdx];

                if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
                {
                    // L1 references
                    uint8_t refPicIdx            = m_picIdx[refPic.FrameIdx].ucPicIdx;
                    m_debugInterface->m_refIndex = (uint16_t)m_refList[refPicIdx]->iFieldOrderCnt[0];
                    std::string refSurfName      = "RefSurf_List1_POC" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                        &m_refList[refPicIdx]->sRefBuffer,
                        CodechalDbgAttr::attrReferenceSurfaces,
                        refSurfName.data()))
                }
            }
        });

    auto singleTaskPhaseSupported = m_singleTaskPhaseSupported;    // local variable to save current setting before overwriting

    if (m_16xMeSupported)
    {
        // disable SingleTaskPhase for now with SHME
        m_singleTaskPhaseSupported = false;

        CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
        MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));

        cscScalingKernelParams.bLastTaskInPhaseCSC  =
        cscScalingKernelParams.bLastTaskInPhase4xDS = false;
        cscScalingKernelParams.bLastTaskInPhase16xDS    = !(m_32xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase32xDS    = !m_hmeEnabled;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }

    if (m_b16XMeEnabled)
    {
        if (m_b32XMeEnabled)
        {
            //HME_P kernel for 32xME
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_32x));
        }
 
        //HME_P kernel for 16xME
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_16x));
 
        //StreamIn kernel, 4xME
        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel(HME_LEVEL_4x));
    }

    // retrieve SingleTaskPhase setting (SAO will need STP enabled setting)
    m_singleTaskPhaseSupported = singleTaskPhaseSupported;

    CODECHAL_DEBUG_TOOL(
        if (m_hmeEnabled) {
            CODECHAL_ME_OUTPUT_PARAMS meOutputParams;

            MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
            meOutputParams.psMeMvBuffer            = &m_s4XMeMvDataBuffer;
            meOutputParams.psMeBrcDistortionBuffer = nullptr;
            meOutputParams.psMeDistortionBuffer    = &m_s4XMeDistortionBuffer;
            meOutputParams.b16xMeInUse = false;
            meOutputParams.b32xMeInUse = false;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &meOutputParams.psMeMvBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));

            //CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            //    &meOutputParams.psMeBrcDistortionBuffer->OsResource,
            //    CodechalDbgAttr::attrOutput,
            //    "BrcDist",
            //    meOutputParams.psMeBrcDistortionBuffer->dwHeight *meOutputParams.psMeBrcDistortionBuffer->dwPitch,
            //    CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
            //    CODECHAL_MEDIA_STATE_4X_ME));
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
            if (m_b16XMeEnabled)
            {
                MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
                meOutputParams.psMeMvBuffer            = &m_s16XMeMvDataBuffer;
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
            }
            if (m_b32XMeEnabled)
            {
                MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
                meOutputParams.psMeMvBuffer = &m_s32XMeMvDataBuffer;
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

            MOS_ZeroMemory(&meOutputParams, sizeof(meOutputParams));
            meOutputParams.pResVdenStreamInBuffer = &(m_resVdencStreamInBuffer[m_currRecycledBufIdx]);
            meOutputParams.psMeMvBuffer = &m_s4XMeMvDataBuffer;
            meOutputParams.psMeDistortionBuffer = &m_s4XMeDistortionBuffer;
            meOutputParams.b16xMeInUse = false;
            meOutputParams.bVdencStreamInInUse = true;
            if (m_vdencStreamInEnabled) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                    CodechalDbgAttr::attrOutput,
                    "StreaminData",
                    m_picWidthInMb * m_picHeightInMb * CODECHAL_CACHELINE_SIZE,
                    0,
                    CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN));
            }
        })
#endif

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ReadSliceSize(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // Use FrameStats buffer if in single pipe mode.
    if (m_numPipe == 1)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::ReadSliceSize(cmdBuffer));
        return eStatus;
    }

    // In multi-tile multi-pipe mode, use PAK integration kernel output
    // PAK integration kernel accumulates frame statistics across tiles, which should be used to setup slice size report
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize + sizeof(uint32_t) * 2);  // encodeStatus is offset by 2 DWs in the resource

                                                                                                                   // Report slice size to app only when dynamic scaling is enabled
    if (!m_hevcSeqParams->SliceSizeControl)
    {
        // Clear slice size report structure in EncodeStatus
        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_encodeStatusBuf.resStatusBuffer, &lockFlags);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        EncodeStatus* dataStatus = (EncodeStatus*)(data + baseOffset);
        MOS_ZeroMemory(&(dataStatus->sliceReport), sizeof(EncodeStatusSliceReport));
        m_osInterface->pfnUnlockResource(m_osInterface, &m_encodeStatusBuf.resStatusBuffer);

        return eStatus;
    }

    uint32_t sizeOfSliceSizesBuffer = MOS_ALIGN_CEIL(m_numLcu * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);

    if (IsFirstPipe())
    {
        if (IsFirstPass())
        {
            // Create/ Initialize slice report buffer once per frame, to be used across passes
            if (Mos_ResourceIsNull(&m_resSliceReport[m_encodeStatusBuf.wCurrIndex]))
            {
                MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
                MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
                allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
                allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
                allocParamsForBufferLinear.Format = Format_Buffer;
                allocParamsForBufferLinear.dwBytes = sizeOfSliceSizesBuffer;

                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBufferLinear,
                    &m_resSliceReport[m_encodeStatusBuf.wCurrIndex]),
                    "Failed to create HEVC VDEnc Slice Report Buffer ");
            }

            // Clear slice size structure to be sent in EncodeStatusReport buffer
            uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, &m_resSliceReport[m_encodeStatusBuf.wCurrIndex], &lockFlags);
            CODECHAL_ENCODE_CHK_NULL_RETURN(data);
            MOS_ZeroMemory(data, sizeOfSliceSizesBuffer);
            m_osInterface->pfnUnlockResource(m_osInterface, &m_resSliceReport[m_encodeStatusBuf.wCurrIndex]);

            // Set slice size pointer in slice size structure
            data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, (&m_encodeStatusBuf.resStatusBuffer), &lockFlags);
            CODECHAL_ENCODE_CHK_NULL_RETURN(data);
            EncodeStatus* dataStatus = (EncodeStatus*)(data + baseOffset);
            (dataStatus)->sliceReport.pSliceSize = &m_resSliceReport[m_encodeStatusBuf.wCurrIndex];
            m_osInterface->pfnUnlockResource(m_osInterface, &m_encodeStatusBuf.resStatusBuffer);
        }

        // Copy Slize size data buffer from PAK to be sent back to App
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CopyDataBlock(cmdBuffer,
            &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource,
            m_hevcTileStatsOffset.uiHevcSliceStreamout,
            &m_resSliceReport[m_encodeStatusBuf.wCurrIndex],
            0,
            sizeOfSliceSizesBuffer));

        MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
        MOS_ZeroMemory(&miCpyMemMemParams, sizeof(MHW_MI_COPY_MEM_MEM_PARAMS));
        miCpyMemMemParams.presSrc = &m_resHuCPakAggregatedFrameStatsBuffer.sResource; // Slice size overflow is in m_resFrameStatStreamOutBuffer DW0[16]
        miCpyMemMemParams.dwSrcOffset = m_hevcFrameStatsOffset.uiHevcPakStatistics;
        miCpyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
        miCpyMemMemParams.dwDstOffset = baseOffset + m_encodeStatusBuf.dwSliceReportOffset;     // Slice size overflow is at DW0 EncodeStatusSliceReport
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ExecutePictureLevel()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = GetCurrentPass();
    int32_t currentPipe = GetCurrentPipe();

    if (IsFirstPipe() && IsFirstPass())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTileData(m_tileParams[m_virtualEngineBbIndex]));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateTileStatistics());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRoundingValues());
    }

    if (m_hevcPicParams->bUsedAsRef || (m_brcEnabled && !m_hevcSeqParams->ParallelBRC))
    {
        if (m_currRefSync == nullptr)
        {
            m_currRefSync = &m_refSync[m_currMbCodeIdx];
        }
    }
    else
    {
        m_currRefSync = nullptr;
    }

    m_firstTaskInPhase = m_singleTaskPhaseSupported ? IsFirstPass() : false;
    m_lastTaskInPhase = m_singleTaskPhaseSupported ? IsLastPass() : true;

    // Per frame maximum HuC kernels is 5 - BRC Init, BRC Update, PAK Int, BRC Update, PAK Int
    m_hucCommandsSize = m_hwInterface->m_hucCommandBufferSize * 5;

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE);

    if (m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex())                                                                         \
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("ERROR - vdbox index exceed the maximum");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifyCommandBufferSize());

    if (!m_singleTaskPhaseSupportedInPak)
    {
        // Command buffer or patch list size are too small and so we cannot submit multiple pass of PAKs together
        m_firstTaskInPhase = true;
        m_lastTaskInPhase = true;
    }

    // To be double checked later!!!
    // PAK pass type for each pass: VDEnc+PAK vs. PAK-only
    SetPakPassType();

    bool pakOnlyMultipassEnable;

    // "PAK-Only Multi-Pass Enable" will be decided by HUC kernel for BRC
    if (m_numPipe >= 2)
    {
        pakOnlyMultipassEnable = false;
    }
    else
    {
        pakOnlyMultipassEnable = false;
    }

    bool panicEnabled = (m_brcEnabled) && (m_panicEnable) && (IsLastPass()) && !m_pakOnlyPass;

    uint32_t rollingILimit = (m_hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ? MOS_ROUNDUP_DIVIDE(m_frameHeight, 32) : (m_hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_COLUMN) ? MOS_ROUNDUP_DIVIDE(m_frameWidth, 32) : 0;

    m_refList[m_currReconstructedPic.FrameIdx]->rollingIntraRefreshedPosition =
        CodecHal_Clip3(0, rollingILimit, m_hevcPicParams->IntraInsertionLocation + m_hevcPicParams->IntraInsertionSize);

    // For ACQP / BRC, update pic params rolling intra reference location here before cmd buffer is prepared.
    PCODEC_PICTURE l0RefFrameList = m_hevcSliceParams->RefPicList[LIST_0];
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = l0RefFrameList[refIdx];

        if (!CodecHal_PictureIsInvalid(refPic) && m_picIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = m_picIdx[refPic.FrameIdx].ucPicIdx;
            m_hevcPicParams->RollingIntraReferenceLocation[refIdx] = m_refList[refPicIdx]->rollingIntraRefreshedPosition;
        }
    }

    if (IsFirstPass())
    {
        MOS_COMMAND_BUFFER cmdBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));
        MHW_MI_MMIOREGISTERS mmioRegister;
        bool validMmio = m_hwInterface->GetMfxInterface()->ConvertToMiRegister(m_vdboxIndex, mmioRegister);
        if (validMmio)
        {
            HalOcaInterface::On1stLevelBBStart(
                cmdBuffer,
                *m_hwInterface->GetOsInterface()->pOsContext,
                m_hwInterface->GetOsInterface()->CurrentGpuContextHandle,
                *m_hwInterface->GetMiInterface(),
                mmioRegister);
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));
    }

    if (m_numPipe >= 2)
    {
        // Send Cmd Buffer Header for VE in last pipe only
        MOS_COMMAND_BUFFER cmdBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));
        bool requestFrameTracking = m_singleTaskPhaseSupported ? IsFirstPass() : IsLastPass();
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

        MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
        MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
        forceWakeupParams.bMFXPowerWellControl = true;
        forceWakeupParams.bMFXPowerWellControlMask = true;
        forceWakeupParams.bHEVCPowerWellControl = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
            &cmdBuffer,
            &forceWakeupParams));

        // clean-up per VDBOX semaphore memory, only in the first BRC pass. Same semaphore is re-used across BRC passes for stitch command
        if (IsFirstPass())
        {
            if (!Mos_ResourceIsNull(&m_resVdBoxSemaphoreMem[currentPipe].sResource))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    SetSemaphoreMem(
                        &m_resVdBoxSemaphoreMem[currentPipe].sResource,
                        &cmdBuffer,
                        0));
            }

            // Do not clear BRC PAK semaphore because of timing issue with =0 on 1st pipe and +1 on 2nd pipe
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));
    }
    else if (IsFirstPass())
    {
        // Send force wake command for VDBOX
        MOS_COMMAND_BUFFER cmdBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

        MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
        MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
        forceWakeupParams.bMFXPowerWellControl = true;
        forceWakeupParams.bMFXPowerWellControlMask = true;
        forceWakeupParams.bHEVCPowerWellControl = true;
        forceWakeupParams.bHEVCPowerWellControlMask = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
            &cmdBuffer,
            &forceWakeupParams));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));
    }

    if (m_vdencHucUsed && IsFirstPipe())
    {
        // STF: HuC+VDEnc+PAK single BB, non-STF: HuC Init/HuC Update/(VDEnc+PAK) in separate BBs
        perfTag.CallType = m_singleTaskPhaseSupported ? CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE :
            CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
        CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, perfTag.CallType);

        m_resVdencBrcUpdateDmemBufferPtr[0] = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, pakInfo);

        // Invoke BRC init/reset FW
        if (m_brcInit || m_brcReset)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcInitReset());
        }

        if (!m_singleTaskPhaseSupported)
        {
            CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE);
        }

        // Invoke BRC update FW
        // When tile replay is enabled, BRC update is also called at tile row level
        if (m_enableTileReplay)
        {
            m_FrameLevelBRCForTileRow = true;
            m_TileRowLevelBRC         = false;
        }
        else
        {
            m_FrameLevelBRCForTileRow = false;
            m_TileRowLevelBRC         = false;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcUpdate());

        m_brcInit = m_brcReset = false;
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported || (m_firstTaskInPhase && !m_hevcVdencAcqpEnabled)) && (m_numPipe == 1))
    {
        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase :
            m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    // clean-up per VDBOX semaphore memory
    if (currentPipe < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // Ensure the previous BRC Update is done, before executing PAK
    if (m_vdencHucUsed && (m_numPipe >= 2))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resBrcPakSemaphoreMem.sResource, 1, MHW_MI_ATOMIC_INC, &cmdBuffer));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(
            &m_resBrcPakSemaphoreMem.sResource,
            &cmdBuffer,
            m_numPipe));

        // Program some placeholder cmds to resolve the hazard between pipe sync
        MHW_MI_STORE_DATA_PARAMS dataParams;
        dataParams.pOsResource = &m_resDelayMinus;
        dataParams.dwResourceOffset = 0;
        dataParams.dwValue = 0xDE1A;
        for (uint32_t i = 0; i < m_numDelay; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                &cmdBuffer,
                &dataParams));
        }

        //clean HW semaphore memory
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMIAtomicCmd(&m_resBrcPakSemaphoreMem.sResource, 1, MHW_MI_ATOMIC_DEC, &cmdBuffer));
    }

    if ((!IsFirstPass()) && m_vdencHuCConditional2ndPass)
    {
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;

        // Insert conditional batch buffer end
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        // VDENC uses HuC FW generated semaphore for conditional 2nd pass
        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_resPakMmioBuffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));

        auto mmioRegisters = m_hcpInterface->GetMmioRegisters(m_vdboxIndex);

        uint32_t baseOffset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // encodeStatus is offset by 2 DWs in the resource

        // Write back the HCP image control register for RC6 may clean it out
        MHW_MI_LOAD_REGISTER_MEM_PARAMS miLoadRegMemParams;
        MOS_ZeroMemory(&miLoadRegMemParams, sizeof(miLoadRegMemParams));
        miLoadRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
        miLoadRegMemParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOffset;
        miLoadRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterMemCmd(&cmdBuffer, &miLoadRegMemParams));

        MHW_MI_STORE_REGISTER_MEM_PARAMS miStoreRegMemParams;
        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = &m_vdencBrcBuffers.resBrcPakStatisticBuffer[m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite];
        miStoreRegMemParams.dwOffset = CODECHAL_OFFSETOF(CODECHAL_ENCODE_HEVC_PAK_STATS_BUFFER, HCP_IMAGE_STATUS_CONTROL_FOR_LAST_PASS);
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));

        MOS_ZeroMemory(&miStoreRegMemParams, sizeof(miStoreRegMemParams));
        miStoreRegMemParams.presStoreBuffer = &m_encodeStatusBuf.resStatusBuffer;
        miStoreRegMemParams.dwOffset = baseOffset + m_encodeStatusBuf.dwImageStatusCtrlOfLastBRCPassOffset;
        miStoreRegMemParams.dwRegister = mmioRegisters->hcpEncImageStatusCtrlRegOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &miStoreRegMemParams));
    }

    if (IsFirstPass() && m_osInterface->bTagResourceSync)
    {
        // This is a short term WA to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.

        MOS_RESOURCE globalGpuContextSyncTagBuffer;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        uint32_t value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue = (value > 0) ? (value - 1) : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    if (IsFirstPipe())
    {
        if (IsFirstPass())
        {
            // Check other dependent VDBOXs if they are ready
            // The inter frame sync method was changed, remove this first, to be tuned
            // CODECHAL_ENCODE_CHK_STATUS_RETURN(WaitForVDBOX(&cmdBuffer));

            // clean-up HW semaphore memory
            if (m_currRefSync && !Mos_ResourceIsNull(&m_currRefSync->resSemaphoreMem.sResource))
            {
                // Ensure this semaphore is not used before. If yes, wait until it is done.
                // The inter frame sync method was changed, remove this first, to be tuned
                // CODECHAL_ENCODE_CHK_STATUS_RETURN(
                //    SendHWWaitCommand(&pCurrRefSync->resSemaphoreMem.sResource, &cmdBuffer, 1));

                MHW_MI_STORE_DATA_PARAMS storeDataParams;
                MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
                storeDataParams.pOsResource      = &m_currRefSync->resSemaphoreMem.sResource;
                storeDataParams.dwResourceOffset = 0;
                storeDataParams.dwValue = 0;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
                    &cmdBuffer,
                    &storeDataParams));
            }
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(m_vdencInterface->CreateMhwVdboxPipeModeSelectParams());

    SetHcpPipeModeSelectParams(*pipeModeSelectParams);

    // HCP_PIPE_SELECT can not be generated by FW in BRC mode for GEN11+
    {
        MHW_VDBOX_VDENC_CONTROL_STATE_PARAMS  vdencControlStateParams;
        MHW_MI_VD_CONTROL_STATE_PARAMS        vdControlStateParams;

        //set up VDENC_CONTROL_STATE command
        {
            MOS_ZeroMemory(&vdencControlStateParams, sizeof(MHW_VDBOX_VDENC_CONTROL_STATE_PARAMS));
            vdencControlStateParams.bVdencInitialization  = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                static_cast<MhwVdboxVdencInterfaceG12X*>(m_vdencInterface)->AddVdencControlStateCmd(&cmdBuffer, &vdencControlStateParams));
        }

        //set up VD_CONTROL_STATE command
        {
            MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
            vdControlStateParams.initialization = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(
                static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(&cmdBuffer, &vdControlStateParams));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));
    }

    MHW_VDBOX_SURFACE_PARAMS srcSurfaceParams;
    SetHcpSrcSurfaceParams(srcSurfaceParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &srcSurfaceParams));

    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    SetHcpReconSurfaceParams(reconSurfaceParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &reconSurfaceParams));

    // Add the surface state for reference picture, GEN12 HW change
    reconSurfaceParams.ucSurfaceStateId = CODECHAL_HCP_REF_SURFACE_ID;
    *m_pipeBufAddrParams = {};
    SetHcpPipeBufAddrParams(*m_pipeBufAddrParams);

#ifdef _MMC_SUPPORTED
    #ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetHcpReconSurfaceParams(reconSurfaceParams, m_slotForRecNotFiltered);
    }
    #endif
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSurfaceCmd(&cmdBuffer, &reconSurfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AddHcpPipeBufAddrCmd(&cmdBuffer));

    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    SetHcpIndObjBaseAddrParams(indObjBaseAddrParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    MHW_VDBOX_QM_PARAMS fqmParams, qmParams;
    SetHcpQmStateParams(fqmParams, qmParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpFqmStateCmd(&cmdBuffer, &fqmParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpQmStateCmd(&cmdBuffer, &qmParams));

    SetVdencPipeModeSelectParams(*pipeModeSelectParams);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(&cmdBuffer, pipeModeSelectParams));

    MHW_VDBOX_SURFACE_PARAMS dsSurfaceParams[2];
    SetVdencSurfaceStateParams(srcSurfaceParams, reconSurfaceParams, dsSurfaceParams[0], dsSurfaceParams[1]);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencSrcSurfaceStateCmd(&cmdBuffer, &srcSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencRefSurfaceStateCmd(&cmdBuffer, &reconSurfaceParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencDsRefSurfaceStateCmd(&cmdBuffer, &dsSurfaceParams[0], 2));

    SetVdencPipeBufAddrParams(*m_pipeBufAddrParams);
    m_pipeBufAddrParams->pRawSurfParam = &srcSurfaceParams;
    m_pipeBufAddrParams->pDecodedReconParam = &reconSurfaceParams;
#ifdef _MMC_SUPPORTED
    m_mmcState->SetPipeBufAddr(m_pipeBufAddrParams);
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeBufAddrCmd(&cmdBuffer, m_pipeBufAddrParams));

    MHW_VDBOX_HEVC_PIC_STATE_G12 picStateParams;
    SetHcpPicStateParams(picStateParams);

    if (m_vdencHucUsed && (!m_hevcPicParams->tiles_enabled_flag))
    {
        // 2nd level batch buffer
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
        HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource, 0, true, 0);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx]));

        // save offset for next 2nd level batch buffer usage
        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset += m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
    }
    // When tile is enabled, below commands are needed for each tile instead of each picture
    else if (!m_hevcPicParams->tiles_enabled_flag)
    {
        SetAddCommands(CODECHAL_CMD1, &cmdBuffer, true, m_roundInterValue, m_roundIntraValue, m_lowDelay);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(&cmdBuffer, &picStateParams));

        SetAddCommands(CODECHAL_CMD2, &cmdBuffer, true, m_roundInterValue, m_roundIntraValue, m_lowDelay, m_refIdxMapping, m_slotForRecNotFiltered);
    }

    // Send HEVC_VP9_RDOQ_STATE command
    if (m_hevcRdoqEnabled && !m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(&cmdBuffer, &picStateParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    m_vdencInterface->ReleaseMhwVdboxPipeModeSelectParams(pipeModeSelectParams);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ExecuteSliceLevel()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::ExecuteSliceLevel());
    }
    else
    {
        if (m_vdencHucUsed && m_enableTileReplay)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncWithTileRowLevelBRC());
        }
        else
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncTileLevel());
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::EncTileLevel()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPipe = GetCurrentPipe();
    int32_t currentPass = GetCurrentPass();

    if (currentPipe < 0 || currentPass < 0)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid pipe number or pass number");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Currently this implementation is only for CQP, single pass
    // Allocate more tile batch when try multiple passes
    if (IsFirstPass() && IsFirstPipe() && (!m_osInterface->bUsesPatchList))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateTileLevelBatch());
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(m_vdencInterface->CreateMhwVdboxPipeModeSelectParams());

    SetHcpPipeModeSelectParams(*pipeModeSelectParams);
    SetVdencPipeModeSelectParams(*pipeModeSelectParams);

    MHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState;
    SetHcpSliceStateCommonParams(sliceState);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    MHW_MI_VD_CONTROL_STATE_PARAMS     vdControlStateParams;
    uint32_t                           numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    uint32_t                           numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;

    // Construct The third level batch buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructTLB(&m_thirdLevelBatchBuffer));

    for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
    {
        for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
        {
            PCODEC_ENCODER_SLCDATA  slcData = m_slcData;
            uint32_t                slcCount, idx, sliceNumInTile = 0;

            idx = tileRow * numTileColumns + tileCol;

            if ((m_numPipe > 1) && (tileCol != currentPipe))
            {
                continue;
            }

            MOS_COMMAND_BUFFER  tileBatchBuf;
            PMOS_COMMAND_BUFFER tempCmdBuf = &cmdBuffer;
            uint8_t             *data      = nullptr;

            // Move tile level commands to first level command buffer when use patch list.
            if (!m_osInterface->bUsesPatchList)
            {
                MOS_LOCK_PARAMS lockFlags;
                MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
                lockFlags.WriteOnly = true;

                uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(m_tileLevelBatchBuffer[currentPass][idx].OsResource), &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);

                MOS_ZeroMemory(&tileBatchBuf, sizeof(tileBatchBuf));
                tileBatchBuf.pCmdBase = tileBatchBuf.pCmdPtr = (uint32_t *)data;
                tileBatchBuf.iRemaining = m_tileLevelBatchSize;

                HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_tileLevelBatchBuffer[m_tileRowPass][idx].OsResource, 0, true, 0);
                // Add batch buffer start for tile
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_tileLevelBatchBuffer[currentPass][idx]));

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->AddProlog(m_osInterface, &cmdBuffer));

                if (m_osInterface->osCpInterface->IsCpEnabled() && m_hwInterface->GetCpInterface()->IsHWCounterAutoIncrementEnforced(m_osInterface) && m_enableTileReplay)
                {
                    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface->GetCpInterface());

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->ReadEncodeCounterFromHW(
                        m_osInterface,
                        &tileBatchBuf,
                        &m_resHwCountTileReplay,
                        (uint16_t)idx));
                }

                tempCmdBuf = &tileBatchBuf;
            }

            // Construct the tile batch
            // To be moved to one sub function later
            // HCP Lock for multiple pipe mode
            if (m_numPipe > 1)
            {
                MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
                vdControlStateParams.scalableModePipeLock = true;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(tempCmdBuf, &vdControlStateParams));
            }
            // VDENC_PIPE_MODE_SELECT
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(tempCmdBuf, pipeModeSelectParams));
            // HCP_PIPE_MODE_SELECT
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(tempCmdBuf, pipeModeSelectParams));

            // 3nd level batch buffer
            if (m_hevcVdencAcqpEnabled || m_brcEnabled)
            {
                HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource, 0, true, 0);
                m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(tempCmdBuf, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx]));
                if (m_hevcRdoqEnabled)
                {
                    MHW_VDBOX_HEVC_PIC_STATE_G12 picStateParams;
                    SetHcpPicStateParams(picStateParams);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(tempCmdBuf, &picStateParams));
                }
            }
            else
            {
                HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_thirdLevelBatchBuffer.OsResource, 0, true, 0);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(tempCmdBuf, &m_thirdLevelBatchBuffer));
            }

            // HCP_TILE_CODING commmand
            // Set Tile replay related parameters
            tileParams[idx].IsFirstPass        = IsFirstPass();
            tileParams[idx].IsLastPass         = IsLastPass();
            tileParams[idx].bTileReplayEnable  = m_enableTileReplay;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG12*>(m_hcpInterface)->AddHcpTileCodingCmd(tempCmdBuf, &tileParams[idx]));

            for (slcCount = 0; slcCount < m_numSlices; slcCount++)
            {
                bool lastSliceInTile = false, sliceInTile = false;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                    &tileParams[idx],
                    &sliceInTile,
                    &lastSliceInTile));

                if (!sliceInTile)
                {
                    continue;
                }

                if (m_hevcVdencAcqpEnabled || m_brcEnabled)
                {
                    // save offset for next 2nd level batch buffer usage
                    // This is because we don't know how many times HCP_WEIGHTOFFSET_STATE & HCP_PAK_INSERT_OBJECT will be inserted for each slice
                    // dwVdencBatchBufferPerSliceConstSize: constant size for each slice
                    // m_vdencBatchBufferPerSliceVarSize:   variable size for each slice

                    // starting location for executing slice level cmds
                    // To do: Improvize to only add current slice wSlcCount
                    m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;

                    for (uint32_t j = 0; j < slcCount; j++)
                    {
                        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset
                            += (m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_vdencBatchBufferPerSliceVarSize[j]);
                    }

                }

                SetHcpSliceStateParams(sliceState, slcData, (uint16_t)slcCount, tileParams, lastSliceInTile, idx);

                CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(tempCmdBuf, &sliceState));

                // Send VD_PIPELINE_FLUSH command  for each slice
                MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
                vdPipelineFlushParams.Flags.bWaitDoneMFX = 1;
                vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
                vdPipelineFlushParams.Flags.bFlushVDENC = 1;
                vdPipelineFlushParams.Flags.bFlushHEVC  = 1;
                vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(tempCmdBuf, &vdPipelineFlushParams));

                sliceNumInTile++;
            } // end of slice

            if (0 == sliceNumInTile)
            {
                // One tile must have at least one slice
                CODECHAL_ENCODE_ASSERT(false);
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                break;
            }

            if (sliceNumInTile > 1 && (numTileColumns > 1 || numTileRows > 1))
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Multi-slices in a tile is not supported!");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            //HCP unLock for multiple pipe mode
            if (m_numPipe > 1)
            {
                MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
                vdControlStateParams.scalableModePipeUnlock = true;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(tempCmdBuf, &vdControlStateParams));
            }

            // Send VD_PIPELINE_FLUSH command
            MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
            vdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
            vdPipelineFlushParams.Flags.bFlushHEVC = 1;
            vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(tempCmdBuf, &vdPipelineFlushParams));

            // Send MI_FLUSH command
            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
            flushDwParams.bVideoPipelineCacheInvalidate = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(tempCmdBuf, &flushDwParams));

            // Update head pointer for capture mode
            if (m_CaptureModeEnable && IsLastPipe())
            {
                MHW_MI_LOAD_REGISTER_IMM_PARAMS     registerImmParams;
                MOS_ZeroMemory(&registerImmParams, sizeof(registerImmParams));
                registerImmParams.dwData      = 1;
                registerImmParams.dwRegister  = m_VdboxVDENCRegBase[currentPipe] + 0x90;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(tempCmdBuf, &registerImmParams));
            }

            if (!m_osInterface->bUsesPatchList)
            {
                // Add batch buffer end at the end of each tile batch, 2nd level batch buffer
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(tempCmdBuf, nullptr));

                std::string pakPassName = "PAK_PASS[" + std::to_string(GetCurrentPass()) + "]_PIPE[" + std::to_string(GetCurrentPipe()) + "]_TILELEVEL";
                CODECHAL_DEBUG_TOOL(
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
                        tempCmdBuf,
                        CODECHAL_NUM_MEDIA_STATES,
                        pakPassName.data()));)

                if (data)
                {
                    m_osInterface->pfnUnlockResource(m_osInterface, &(m_tileLevelBatchBuffer[currentPass][idx].OsResource));
                }
            }

        } // end of row tile
    } // end of column tile

    m_vdencInterface->ReleaseMhwVdboxPipeModeSelectParams(pipeModeSelectParams);

    // Insert end of sequence/stream if set
    // To be moved to slice level?
    if ((m_lastPicInStream || m_lastPicInSeq) && IsLastPipe())
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&cmdBuffer, &pakInsertObjectParams));
    }

    // Send VD_CONTROL_STATE (Memory Implict Flush)
    MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdControlStateParams.memoryImplicitFlush = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(&cmdBuffer, &vdControlStateParams));


    // Send VD_PIPELINE_FLUSH command 
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    vdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
    vdPipelineFlushParams.Flags.bFlushHEVC = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Set the HW semaphore to indicate current pipe done
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    if (!Mos_ResourceIsNull(&m_resVdBoxSemaphoreMem[currentPipe].sResource))
    {
        flushDwParams.pOsResource = &m_resVdBoxSemaphoreMem[currentPipe].sResource;
        flushDwParams.dwDataDW1 = currentPass + 1;
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (IsFirstPipe())
    {
        // first pipe needs to ensure all other pipes are ready
        for (uint32_t i = 0; i < m_numPipe; i++)
        {
            if (!Mos_ResourceIsNull(&m_resVdBoxSemaphoreMem[i].sResource))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    SendHWWaitCommand(&m_resVdBoxSemaphoreMem[i].sResource,
                        &cmdBuffer,
                        currentPass + 1));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    SetSemaphoreMem(&m_resVdBoxSemaphoreMem[i].sResource,
                        &cmdBuffer,
                        0x0));
            }
        }

        // Whenever ACQP/ BRC is enabled with tiling, PAK Integration kernel is needed.
        // ACQP/ BRC need PAK integration kernel to aggregate statistics
        if (m_vdencHucUsed)  
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HucPakIntegrate(&cmdBuffer));
        }

        // Use HW stitch commands only in the scalable mode
        // For single pipe with tile replay, stitch also needed
        if (m_enableTileStitchByHW)
        {
            if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CQP && !m_hevcVdencAcqpEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(HucPakIntegrateStitch(&cmdBuffer));
            }
            // 2nd level BB buffer for stitching cmd
            // current location to add cmds in 2nd level batch buffer
            m_HucStitchCmdBatchBuffer.iCurrent = 0;
            // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
            m_HucStitchCmdBatchBuffer.dwOffset = 0;
            HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_HucStitchCmdBatchBuffer.OsResource, 0, true, 0);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_HucStitchCmdBatchBuffer));
            // This wait cmd is needed to make sure copy command is done as suggested by HW folk in encode cases
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(&cmdBuffer, nullptr, m_osInterface->osCpInterface->IsCpEnabled() ? true : false));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSseStatistics(&cmdBuffer));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSliceSize(&cmdBuffer));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

        if (m_numPipe <= 1)  // single pipe mode can read the info from MMIO register. Otherwise, we have to use the tile size statistic buffer
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));

            // BRC PAK statistics different for each pass
            if (m_brcEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStats(&cmdBuffer));
            }
        }

        MHW_MI_STORE_DATA_PARAMS    storeDataParams;
        // Signal HW semaphore for the reference frame dependency (i.e., current coding frame waits for the reference frame being ready)
        if (m_currRefSync && !Mos_ResourceIsNull(&m_currRefSync->resSemaphoreMem.sResource))
        {
            // the reference frame semaphore must be set in each pass because of the conditional BRC batch buffer. Some BRC passes could be skipped.
            MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
            storeDataParams.pOsResource      = &m_currRefSync->resSemaphoreMem.sResource;
            storeDataParams.dwResourceOffset = 0;
            storeDataParams.dwValue = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiStoreDataImmCmd(
                &cmdBuffer,
                &storeDataParams));
        }
    }

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    std::string pakPassName = "PAK_PASS[" + std::to_string(GetCurrentPass()) + "]_PIPE[" + std::to_string(GetCurrentPipe()) + "]";
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN( m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data()));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        bool nullRendering = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, nullRendering));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpHucDebugOutputBuffers());
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

        if (IsFirstPipe() &&
            IsLastPass() &&
            m_signalEnc &&
            m_currRefSync &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // signal semaphore
            MOS_SYNC_PARAMS syncParams;
            syncParams                  = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount++;
            m_currRefSync->bInUsed = true;
            }
    }

    // Reset parameters for next PAK execution
    if (IsLastPipe() &&
        IsLastPass())
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_currPakSliceIdx = (m_currPakSliceIdx + 1) % CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS;

        if (m_hevcSeqParams->ParallelBRC)
        {
            m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite =
                (m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::EncWithTileRowLevelBRC()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPipe = GetCurrentPipe();
    int32_t currentPass = GetCurrentPass();

    if (currentPipe < 0 || currentPass < 0)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid pipe number or pass number");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Revisit the buffer reuse for multiple frames later
    if (IsFirstPass() && IsFirstPipe())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateTileLevelBatch());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateTileRowLevelBRCBatch());
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(m_vdencInterface->CreateMhwVdboxPipeModeSelectParams());

    SetHcpPipeModeSelectParams(*pipeModeSelectParams);
    SetVdencPipeModeSelectParams(*pipeModeSelectParams);

    MHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState;
    SetHcpSliceStateCommonParams(sliceState);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    MHW_MI_VD_CONTROL_STATE_PARAMS     vdControlStateParams;
    uint32_t                           numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    uint32_t                           numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipelineFlushParams;
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];

    m_FrameLevelBRCForTileRow = false;
    m_TileRowLevelBRC = true;

    for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
    {
        for (m_tileRowPass = 0; m_tileRowPass < m_NumPassesForTileReplay; m_tileRowPass++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                PCODEC_ENCODER_SLCDATA slcData = m_slcData;
                uint32_t               slcCount, idx, sliceNumInTile = 0;

                idx = tileRow * numTileColumns + tileCol;

                if ((m_numPipe > 1) && (tileCol != currentPipe))
                {
                    continue;
                }

                MOS_LOCK_PARAMS lockFlags;
                MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
                lockFlags.WriteOnly = true;

                uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(m_tileLevelBatchBuffer[m_tileRowPass][idx].OsResource), &lockFlags);
                CODECHAL_ENCODE_CHK_NULL_RETURN(data);

                MOS_COMMAND_BUFFER tileBatchBuf;
                MOS_ZeroMemory(&tileBatchBuf, sizeof(tileBatchBuf));
                tileBatchBuf.pCmdBase = tileBatchBuf.pCmdPtr = (uint32_t *)data;
                tileBatchBuf.iRemaining = m_tileLevelBatchSize;

                // Add batch buffer start for tile
                HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_tileLevelBatchBuffer[m_tileRowPass][idx].OsResource, 0, true, 0);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_tileLevelBatchBuffer[m_tileRowPass][idx]));

                if (m_numPipe > 1)
                {
                    //wait for last tile row BRC update completion
                    if ((!IsFirstPipe()) && (!IsFirstPassForTileReplay()))
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(&m_resTileRowBRCsyncSemaphore, &tileBatchBuf, 0xFF));
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSemaphoreMem(&m_resTileRowBRCsyncSemaphore, &tileBatchBuf, 0x0));
                    }
                }

                // Add conditional batch buffer end before tile row level second pass
                // To unify the single pipe and multiple pipe cases, add this for each tile

                // To add the sync logic here to make sure the previous tile row BRC update is done

                if (!IsFirstPassForTileReplay())
                {
                    MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS  miEnhancedConditionalBatchBufferEndParams;

                    MOS_ZeroMemory(
                        &miEnhancedConditionalBatchBufferEndParams,
                        sizeof(MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

                    // VDENC uses HuC FW generated semaphore for conditional 2nd pass
                    miEnhancedConditionalBatchBufferEndParams.presSemaphoreBuffer =
                        &m_resPakMmioBuffer;

                    miEnhancedConditionalBatchBufferEndParams.dwParamsType = MHW_MI_ENHANCED_CONDITIONAL_BATCH_BUFFER_END_PARAMS::ENHANCED_PARAMS;
                    miEnhancedConditionalBatchBufferEndParams.enableEndCurrentBatchBuffLevel = true;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
                        &tileBatchBuf,
                        (PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS)(&miEnhancedConditionalBatchBufferEndParams)));
                }

                // Construct the tile batch
                // To be moved to one sub function later
                // HCP Lock for multiple pipe mode
                if (m_numPipe > 1)
                {
                    MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
                    vdControlStateParams.scalableModePipeLock = true;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(&tileBatchBuf, &vdControlStateParams));
                }

                // VDENC_PIPE_MODE_SELECT
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencPipeModeSelectCmd(&tileBatchBuf, pipeModeSelectParams));
                // HCP_PIPE_MODE_SELECT
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&tileBatchBuf, pipeModeSelectParams));

                // 3nd level batch buffer
                if (m_hevcVdencAcqpEnabled || m_brcEnabled)
                {
                    m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize;
                    HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource, 0, true, 0);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&tileBatchBuf, &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx]));

                    if (m_hevcRdoqEnabled)
                    {
                        MHW_VDBOX_HEVC_PIC_STATE_G12 picStateParams;
                        SetHcpPicStateParams(picStateParams);
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(&tileBatchBuf, &picStateParams));
                    }
                }

                // HCP_TILE_CODING commmand
                // Set Tile replay related parameters
                tileParams[idx].IsFirstPass = IsFirstPassForTileReplay();
                tileParams[idx].IsLastPass = IsLastPassForTileReplay();
                tileParams[idx].bTileReplayEnable = m_enableTileReplay;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwVdboxHcpInterfaceG12*>(m_hcpInterface)->AddHcpTileCodingCmd(&tileBatchBuf, &tileParams[idx]));
                
                for (slcCount = 0; slcCount < m_numSlices; slcCount++)
                {
                    bool lastSliceInTile = false, sliceInTile = false;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                        &tileParams[idx],
                        &sliceInTile,
                        &lastSliceInTile));

                    if (!sliceInTile)
                    {
                        continue;
                    }

                    if (m_hevcVdencAcqpEnabled || m_brcEnabled)
                    {
                        // save offset for next 2nd level batch buffer usage
                        // This is because we don't know how many times HCP_WEIGHTOFFSET_STATE & HCP_PAK_INSERT_OBJECT will be inserted for each slice
                        // dwVdencBatchBufferPerSliceConstSize: constant size for each slice
                        // m_vdencBatchBufferPerSliceVarSize:   variable size for each slice

                        // starting location for executing slice level cmds
                        // To do: Improvize to only add current slice wSlcCount
                        m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;

                        for (uint32_t j = 0; j < slcCount; j++)
                        {
                            m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].dwOffset
                                += (m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_vdencBatchBufferPerSliceVarSize[j]);
                        }
                    }

                    SetHcpSliceStateParams(sliceState, slcData, (uint16_t)slcCount, tileParams, lastSliceInTile, idx);

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHwSliceEncodeCommand(&tileBatchBuf, &sliceState));

                    // Send VD_PIPELINE_FLUSH command  for each slice
                    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
                    vdPipelineFlushParams.Flags.bWaitDoneMFX = 1;
                    vdPipelineFlushParams.Flags.bWaitDoneVDENC = 1;
                    vdPipelineFlushParams.Flags.bFlushVDENC = 1;
                    vdPipelineFlushParams.Flags.bFlushHEVC  = 1;
                    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&tileBatchBuf, &vdPipelineFlushParams));

                    sliceNumInTile++;
                } // end of slice

                if (0 == sliceNumInTile)
                {
                    // One tile must have at least one slice
                    CODECHAL_ENCODE_ASSERT(false);
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    break;
                }

                if (sliceNumInTile > 1 && (numTileColumns > 1 || numTileRows > 1))
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Multi-slices in a tile is not supported!");
                    return MOS_STATUS_INVALID_PARAMETER;
                }

                //HCP unLock for multiple pipe mode
                if (m_numPipe > 1)
                {
                    MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
                    vdControlStateParams.scalableModePipeUnlock = true;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(&tileBatchBuf, &vdControlStateParams));
                }

                // Send VD_PIPELINE_FLUSH command
                MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
                vdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
                vdPipelineFlushParams.Flags.bFlushHEVC = 1;
                vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&tileBatchBuf, &vdPipelineFlushParams));

                // Send MI_FLUSH command
                MHW_MI_FLUSH_DW_PARAMS flushDwParams;
                MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
                flushDwParams.bVideoPipelineCacheInvalidate = true;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&tileBatchBuf, &flushDwParams));

                // Add batch buffer end at the end of each tile batch, 2nd level batch buffer
                (&m_tileLevelBatchBuffer[m_tileRowPass][idx])->iCurrent = tileBatchBuf.iOffset;
                (&m_tileLevelBatchBuffer[m_tileRowPass][idx])->iRemaining = tileBatchBuf.iRemaining;
                (&m_tileLevelBatchBuffer[m_tileRowPass][idx])->pData = data;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &m_tileLevelBatchBuffer[m_tileRowPass][idx]));

                if (data)
                {
                    m_osInterface->pfnUnlockResource(m_osInterface, &(m_tileLevelBatchBuffer[m_tileRowPass][idx].OsResource));
                }
            } // end of row tile

            // Set the semaphore for tile row BRC update
            if ((m_numPipe > 1) && (!IsFirstPipe()) && (!IsLastPassForTileReplay()))
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(
                    SetSemaphoreMem(
                        &m_resVdBoxSemaphoreMem[currentPipe].sResource,
                        &cmdBuffer,
                        0xFF));
            }

            // Run tile row based BRC on pipe 0
            if (IsFirstPipe() && (!IsLastPassForTileReplay()))
            {
                m_CurrentTileRow           = tileRow;
                m_CurrentPassForTileReplay = m_tileRowPass;
                m_CurrentPassForOverAll++;

                // Before tile row BRC update, make sure all pipes are complete
                if (m_numPipe > 1)
                {
                    for (uint32_t i = 1; i < m_numPipe; i++)
                    {
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(&m_resVdBoxSemaphoreMem[i].sResource, &cmdBuffer, 0xFF));
                        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSemaphoreMem(&m_resVdBoxSemaphoreMem[i].sResource, &cmdBuffer, 0x0));
                    }
                }

                CODECHAL_ENCODE_CHK_STATUS_RETURN(HuCBrcTileRowUpdate(&cmdBuffer));
            }
        } 

        // Update head pointer for capture mode
        if (m_CaptureModeEnable && IsLastPipe())
        {
            MHW_MI_LOAD_REGISTER_IMM_PARAMS     registerImmParams;
            MOS_ZeroMemory(&registerImmParams, sizeof(registerImmParams));
            registerImmParams.dwData      = 1;
            registerImmParams.dwRegister  = m_VdboxVDENCRegBase[currentPipe] + 0x90;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiLoadRegisterImmCmd(&cmdBuffer, &registerImmParams));
        }
    } 

    m_vdencInterface->ReleaseMhwVdboxPipeModeSelectParams(pipeModeSelectParams);

    // Insert end of sequence/stream if se
    // To be moved to slice level?
    if ((m_lastPicInStream || m_lastPicInSeq) && IsLastPipe())
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInSeq = m_lastPicInSeq;
        pakInsertObjectParams.bLastPicInStream = m_lastPicInStream;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&cmdBuffer, &pakInsertObjectParams));
    }

    // Send VD_CONTROL_STATE (Memory Implict Flush)
    MOS_ZeroMemory(&vdControlStateParams, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdControlStateParams.memoryImplicitFlush = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        static_cast<MhwMiInterfaceG12*>(m_miInterface)->AddMiVdControlStateCmd(&cmdBuffer, &vdControlStateParams));

    // Send VD_PIPELINE_FLUSH command
    MOS_ZeroMemory(&vdPipelineFlushParams, sizeof(vdPipelineFlushParams));
    vdPipelineFlushParams.Flags.bWaitDoneHEVC = 1;
    vdPipelineFlushParams.Flags.bFlushHEVC = 1;
    vdPipelineFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipelineFlushParams));

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Set the HW semaphore to indicate current pipe done
    if (m_numPipe > 1)
    {
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        if (!Mos_ResourceIsNull(&m_resVdBoxSemaphoreMem[currentPipe].sResource))
        {
            flushDwParams.pOsResource = &m_resVdBoxSemaphoreMem[currentPipe].sResource;
            flushDwParams.dwDataDW1   = 0xFF;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));
    }

    if (IsFirstPipe())
    {
        // first pipe needs to ensure all other pipes are ready
        if (m_numPipe > 1)
        {
            for (uint32_t i = 0; i < m_numPipe; i++)
            {
                if (!Mos_ResourceIsNull(&m_resVdBoxSemaphoreMem[i].sResource))
                {
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendHWWaitCommand(&m_resVdBoxSemaphoreMem[i].sResource, &cmdBuffer, 0xFF));
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSemaphoreMem(&m_resVdBoxSemaphoreMem[i].sResource, &cmdBuffer, 0x0));
                }
            }
        }

        // Whenever ACQP/ BRC is enabled with tiling, PAK Integration kernel is needed.
        // ACQP/ BRC need PAK integration kernel to aggregate statistics
        if (m_vdencHucUsed)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(HucPakIntegrate(&cmdBuffer));
        }

        // Use HW stitch commands only in the scalable mode
        // For single pipe with tile replay, stitch also needed
        if (m_enableTileStitchByHW)
        {
            // 2nd level BB buffer for stitching cmd
            // current location to add cmds in 2nd level batch buffer
            m_HucStitchCmdBatchBuffer.iCurrent = 0;
            // reset starting location (offset) executing 2nd level batch buffer for each frame & each pass
            m_HucStitchCmdBatchBuffer.dwOffset = 0;
            HalOcaInterface::OnSubLevelBBStart(cmdBuffer, *m_osInterface->pOsContext, &m_HucStitchCmdBatchBuffer.OsResource, 0, true, 0);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, &m_HucStitchCmdBatchBuffer));
            // This wait cmd is needed to make sure copy command is done as suggested by HW folk in encode cases
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMfxWaitCmd(&cmdBuffer, nullptr, m_osInterface->osCpInterface->IsCpEnabled() ? true : false));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadSseStatistics(&cmdBuffer));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

        if (m_numPipe <= 1)  // single pipe mode can read the info from MMIO register. Otherwise, we have to use the tile size statistic buffer
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadHcpStatus(&cmdBuffer));

            // BRC PAK statistics different for each pass
            if (m_brcEnabled)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStats(&cmdBuffer));
            }
        }

        MHW_MI_STORE_DATA_PARAMS    storeDataParams;
        // Signal HW semaphore for the reference frame dependency (i.e., current coding frame waits for the reference frame being ready)
        if (m_currRefSync && !Mos_ResourceIsNull(&m_currRefSync->resSemaphoreMem.sResource))
        {
            // the reference frame semaphore must be set in each pass because of the conditional BRC batch buffer. Some BRC passes could be skipped.
            MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
            storeDataParams.pOsResource      = &m_currRefSync->resSemaphoreMem.sResource;
            storeDataParams.dwResourceOffset = 0;
            storeDataParams.dwValue = 1;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiStoreDataImmCmd(
                &cmdBuffer,
                &storeDataParams));
        }
    }

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase || (m_numPipe >= 2))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetMiInterface()->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        bool nullRendering = m_videoContextUsesNullHw;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, nullRendering));

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

        if (IsFirstPipe() &&
            IsLastPass() &&
            m_signalEnc &&
            m_currRefSync &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // signal semaphore
            MOS_SYNC_PARAMS syncParams;
            syncParams                  = g_cInitSyncParams;
            syncParams.GpuContext       = m_videoContext;
            syncParams.presSyncResource = &m_currRefSync->resSyncObject;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_currRefSync->uiSemaphoreObjCount++;
            m_currRefSync->bInUsed = true;
            }
    }

    // Reset parameters for next PAK execution
    if (IsLastPipe() &&
        IsLastPass())
    {
        if (!m_singleTaskPhaseSupported)
        {
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

        m_currPakSliceIdx = (m_currPakSliceIdx + 1) % CODECHAL_HEVC_NUM_PAK_SLICE_BATCH_BUFFERS;

        if (m_hevcSeqParams->ParallelBRC)
        {
            m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite =
                (m_vdencBrcBuffers.uiCurrBrcPakStasIdxForWrite + 1) % CODECHAL_ENCODE_RECYCLED_BUFFER_NUM;
        }

        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
        m_frameNum++;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_slcData);
    CODECHAL_ENCODE_CHK_NULL_RETURN(batchBuffer);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, batchBuffer, &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iRemaining = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);

    // 1st Group : PIPE_MODE_SELECT
    // set PIPE_MODE_SELECT command
    // This is not needed for GEN11/GEN12 as single pass SAO is supported
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bVdencEnabled = true;
    pipeModeSelectParams.bAdvancedRateControlEnable = true;
    pipeModeSelectParams.bRdoqEnable                = m_hevcRdoqEnabled;
    pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
    pipeModeSelectParams.PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    pipeModeSelectParams.bStreamOutEnabled = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeModeSelectCmd(&constructedCmdBuf, &pipeModeSelectParams));

    MHW_BATCH_BUFFER  TempBatchBuffer;
    MOS_ZeroMemory(&TempBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    TempBatchBuffer.iSize       = MOS_ALIGN_CEIL(m_hwInterface->m_vdencReadBatchBufferSize, CODECHAL_PAGE_SIZE);
    TempBatchBuffer.pData       = data;

    // set MI_BATCH_BUFFER_END command
    int32_t cmdBufOffset = constructedCmdBuf.iOffset;

    TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
    TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
    constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
    constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
    constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

    m_miBatchBufferEndCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
    CODECHAL_ENCODE_ASSERT(m_hwInterface->m_vdencBatchBuffer1stGroupSize == constructedCmdBuf.iOffset);

    SetAddCommands(CODECHAL_CMD1, &constructedCmdBuf, true, m_roundInterValue, m_roundIntraValue, m_lowDelay);
    m_picStateCmdStartInBytes = constructedCmdBuf.iOffset;

    // set HCP_PIC_STATE command
    MHW_VDBOX_HEVC_PIC_STATE_G12 hevcPicState;
    SetHcpPicStateParams(hevcPicState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(&constructedCmdBuf, &hevcPicState));
    m_cmd2StartInBytes = constructedCmdBuf.iOffset;

    SetAddCommands(CODECHAL_CMD2, &constructedCmdBuf, true, m_roundInterValue, m_roundIntraValue, m_lowDelay, m_refIdxMapping, m_slotForRecNotFiltered);

    // set MI_BATCH_BUFFER_END command
    TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
    TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
    constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
    constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
    constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

    CODECHAL_ENCODE_ASSERT(m_hwInterface->m_vdencBatchBuffer2ndGroupSize + m_hwInterface->m_vdencBatchBuffer1stGroupSize
        == constructedCmdBuf.iOffset);

    // 3rd Group : HCP_WEIGHTSOFFSETS_STATE + HCP_SLICE_STATE + HCP_PAK_INSERT_OBJECT + VDENC_WEIGHT_OFFSETS_STATE
    MHW_VDBOX_HEVC_SLICE_STATE_G12 sliceState;
    SetHcpSliceStateCommonParams(sliceState);

    // slice level cmds for each slice
    PCODEC_ENCODER_SLCDATA slcData = m_slcData;
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];

    for (uint32_t startLCU = 0, slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        bool lastSliceInTile = false, sliceInTile = false;

        if (IsFirstPass())
        {
            slcData[slcCount].CmdOffset = startLCU * (m_hcpInterface->GetHcpPakObjSize()) * sizeof(uint32_t);
        }

        uint32_t  numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
        uint32_t  numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
        uint32_t  idx = 0;
        for (uint32_t tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (uint32_t tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                idx = tileRow * numTileColumns + tileCol;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                    &tileParams[idx],
                    &sliceInTile,
                    &lastSliceInTile));

                if (sliceInTile)
                {
                    break;
                }
            }
            if (sliceInTile)
            {
                break;
            }
        }

        SetHcpSliceStateParams(sliceState, slcData, (uint16_t)slcCount, tileParams, lastSliceInTile, idx);

        m_vdencBatchBufferPerSliceVarSize[slcCount] = 0;

        // set HCP_WEIGHTOFFSET_STATE command
        // This slice level command is issued, if the weighted_pred_flag or weighted_bipred_flag equals one.
        //        If zero, then this command is not issued.
        if (m_hevcVdencWeightedPredEnabled)
        {
            MHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS hcpWeightOffsetParams;
            MOS_ZeroMemory(&hcpWeightOffsetParams, sizeof(hcpWeightOffsetParams));
            // HuC based WP ignores App based weights
            if (!m_hevcPicParams->bEnableGPUWeightedPrediction)
            {
                for (auto k = 0; k < 2; k++) // k=0: LIST_0, k=1: LIST_1
                {
                    // Luma, Chroma Offset
                    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
                    {
                        hcpWeightOffsetParams.LumaOffsets[k][i] = (int16_t)m_hevcSliceParams->luma_offset[k][i];
                        // Cb, Cr
                        for (auto j = 0; j < 2; j++)
                        {
                            hcpWeightOffsetParams.ChromaOffsets[k][i][j] = (int16_t)m_hevcSliceParams->chroma_offset[k][i][j];
                        }
                    }

                    // Luma Weight
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                        &hcpWeightOffsetParams.LumaWeights[k],
                        sizeof(hcpWeightOffsetParams.LumaWeights[k]),
                        &m_hevcSliceParams->delta_luma_weight[k],
                        sizeof(m_hevcSliceParams->delta_luma_weight[k])));
                    // Chroma Weight
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
                        &hcpWeightOffsetParams.ChromaWeights[k],
                        sizeof(hcpWeightOffsetParams.ChromaWeights[k]),
                        &m_hevcSliceParams->delta_chroma_weight[k],
                        sizeof(m_hevcSliceParams->delta_chroma_weight[k])));
                }
            }

            // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
            if (m_hevcSliceParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE || m_hevcSliceParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hcpWeightOffsetParams.ucList = LIST_0;

                cmdBufOffset = constructedCmdBuf.iOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(&constructedCmdBuf, nullptr, &hcpWeightOffsetParams));
                m_hcpWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
                // 1st HcpWeightOffset cmd is not always inserted (except weighted prediction + P, B slices)
                m_vdencBatchBufferPerSliceVarSize[slcCount] += m_hcpWeightOffsetStateCmdSize;
            }

            // 2nd HCP_WEIGHTOFFSET_STATE cmd - B only
            if (m_hevcSliceParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hcpWeightOffsetParams.ucList = LIST_1;

                cmdBufOffset = constructedCmdBuf.iOffset;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpWeightOffsetStateCmd(&constructedCmdBuf, nullptr, &hcpWeightOffsetParams));
                m_hcpWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;
                // 2nd HcpWeightOffset cmd is not always inserted (except weighted prediction + B slices)
                m_vdencBatchBufferPerSliceVarSize[slcCount] += m_hcpWeightOffsetStateCmdSize;
            }
        }

        // set HCP_SLICE_STATE command
        cmdBufOffset = constructedCmdBuf.iOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpSliceStateCmd(&constructedCmdBuf, &sliceState));
        m_hcpSliceStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

        // set 1st HCP_PAK_INSERT_OBJECT command
        // insert AU, SPS, PPS headers before first slice header
        if (sliceState.bInsertBeforeSliceHeaders)
        {
            uint32_t maxBytesInPakInsertObjCmd = ((2 << 11) - 1) * 4; // 12 bits for DwordLength field in PAK_INSERT_OBJ cmd
            m_1stPakInsertObjectCmdSize = 0;

            for (auto i = 0; i < HEVC_MAX_NAL_UNIT_TYPE; i++)
            {
                uint32_t nalUnitPosiSize = sliceState.ppNalUnitParams[i]->uiSize;
                uint32_t nalUnitPosiOffset = sliceState.ppNalUnitParams[i]->uiOffset;

                while (nalUnitPosiSize > 0)
                {
                    uint32_t bitSize = MOS_MIN(maxBytesInPakInsertObjCmd * 8, nalUnitPosiSize * 8);
                    uint32_t offSet = nalUnitPosiOffset;

                    MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
                    MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
                    pakInsertObjectParams.bEmulationByteBitsInsert = sliceState.ppNalUnitParams[i]->bInsertEmulationBytes;
                    pakInsertObjectParams.uiSkipEmulationCheckCount = sliceState.ppNalUnitParams[i]->uiSkipEmulationCheckCount;
                    pakInsertObjectParams.pBsBuffer = sliceState.pBsBuffer;
                    pakInsertObjectParams.dwBitSize = bitSize;
                    pakInsertObjectParams.dwOffset = offSet;

                    if (nalUnitPosiSize > maxBytesInPakInsertObjCmd)
                    {
                        nalUnitPosiSize -= maxBytesInPakInsertObjCmd;
                        nalUnitPosiOffset += maxBytesInPakInsertObjCmd;
                    }
                    else
                    {
                        nalUnitPosiSize = 0;
                    }

                    cmdBufOffset = constructedCmdBuf.iOffset;
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(&constructedCmdBuf, &pakInsertObjectParams));

                    // this info needed again in BrcUpdate HuC FW const
                    m_1stPakInsertObjectCmdSize += (constructedCmdBuf.iOffset - cmdBufOffset);
                }
            }
            // 1st PakInsertObject cmd is not always inserted for each slice
            m_vdencBatchBufferPerSliceVarSize[slcCount] += m_1stPakInsertObjectCmdSize;
        }

        // set 2nd HCP_PAK_INSERT_OBJECT command
        // Insert slice header
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastHeader = true;
        pakInsertObjectParams.bEmulationByteBitsInsert = true;

        // App does the slice header packing, set the skip count passed by the app
        pakInsertObjectParams.uiSkipEmulationCheckCount = sliceState.uiSkipEmulationCheckCount;
        pakInsertObjectParams.pBsBuffer = sliceState.pBsBuffer;
        pakInsertObjectParams.dwBitSize = sliceState.dwLength;
        pakInsertObjectParams.dwOffset = sliceState.dwOffset;

        // For HEVC VDEnc Dynamic Slice
        if (m_hevcSeqParams->SliceSizeControl)
        {
            pakInsertObjectParams.bLastHeader = false;
            pakInsertObjectParams.bEmulationByteBitsInsert = false;
            pakInsertObjectParams.dwBitSize                  = m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            pakInsertObjectParams.bResetBitstreamStartingPos = true;
        }

        uint32_t byteSize = (pakInsertObjectParams.dwBitSize + 7) >> 3;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
            &constructedCmdBuf,
            &pakInsertObjectParams));

        // 2nd PakInsertObject cmd is always inserted for each slice
        // so already reflected in dwVdencBatchBufferPerSliceConstSize
        m_vdencBatchBufferPerSliceVarSize[slcCount] += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t) * 4;

        // set 3rd HCP_PAK_INSERT_OBJECT command
        if (m_hevcSeqParams->SliceSizeControl)
        {
            // Send HCP_PAK_INSERT_OBJ command. For dynamic slice, we are skipping the beginning part of slice header.
            pakInsertObjectParams.bLastHeader = true;
            pakInsertObjectParams.dwBitSize   = sliceState.dwLength - m_hevcSliceParams->BitLengthSliceHeaderStartingPortion;
            pakInsertObjectParams.dwOffset += ((m_hevcSliceParams->BitLengthSliceHeaderStartingPortion + 7) / 8);  // Skips the first 5 bytes which is Start Code + Nal Unit Header
            pakInsertObjectParams.bResetBitstreamStartingPos = true;

            cmdBufOffset = constructedCmdBuf.iOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPakInsertObject(
                &constructedCmdBuf,
                &pakInsertObjectParams));
            // 3rd PakInsertObject cmd is not always inserted for each slice
            m_vdencBatchBufferPerSliceVarSize[slcCount] += (constructedCmdBuf.iOffset - cmdBufOffset);
        }

        // set VDENC_WEIGHT_OFFSETS_STATE command
        MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS vdencWeightOffsetParams;
        MOS_ZeroMemory(&vdencWeightOffsetParams, sizeof(vdencWeightOffsetParams));
        vdencWeightOffsetParams.bWeightedPredEnabled = m_hevcVdencWeightedPredEnabled;
        vdencWeightOffsetParams.isLowDelay = m_lowDelay;

        if (vdencWeightOffsetParams.bWeightedPredEnabled)
        {
            uint8_t lumaLog2WeightDenom = m_hevcPicParams->bEnableGPUWeightedPrediction ? 6 : m_hevcSliceParams->luma_log2_weight_denom;
            vdencWeightOffsetParams.dwDenom = 1 << lumaLog2WeightDenom;

            if (!m_hevcPicParams->bEnableGPUWeightedPrediction)
            {
                // Luma Offsets
                for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
                {
                    vdencWeightOffsetParams.LumaOffsets[0][i] = (int16_t)m_hevcSliceParams->luma_offset[0][i];
                    vdencWeightOffsetParams.LumaOffsets[1][i] = (int16_t)m_hevcSliceParams->luma_offset[1][i];
                }

                // Luma Weights
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                    &vdencWeightOffsetParams.LumaWeights[0],
                    sizeof(vdencWeightOffsetParams.LumaWeights[0]),
                    &m_hevcSliceParams->delta_luma_weight[0],
                    sizeof(m_hevcSliceParams->delta_luma_weight[0])),
                    "Failed to copy luma weight 0 memory.");

                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(MOS_SecureMemcpy(
                    &vdencWeightOffsetParams.LumaWeights[1],
                    sizeof(vdencWeightOffsetParams.LumaWeights[1]),
                    &m_hevcSliceParams->delta_luma_weight[1],
                    sizeof(m_hevcSliceParams->delta_luma_weight[1])),
                    "Failed to copy luma weight 1 memory.");
            }
        }

        cmdBufOffset = constructedCmdBuf.iOffset;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWeightsOffsetsStateCmd(
            &constructedCmdBuf,
            nullptr,
            &vdencWeightOffsetParams));
        m_vdencWeightOffsetStateCmdSize = constructedCmdBuf.iOffset - cmdBufOffset;

        // set MI_BATCH_BUFFER_END command
        TempBatchBuffer.iCurrent    = constructedCmdBuf.iOffset;
        TempBatchBuffer.iRemaining  = constructedCmdBuf.iRemaining;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &TempBatchBuffer));
        constructedCmdBuf.pCmdPtr     += (TempBatchBuffer.iCurrent - constructedCmdBuf.iOffset) / 4;
        constructedCmdBuf.iOffset      = TempBatchBuffer.iCurrent;
        constructedCmdBuf.iRemaining   = TempBatchBuffer.iRemaining;

        m_vdencBatchBufferPerSliceVarSize[slcCount] += ENCODE_VDENC_HEVC_PADDING_DW_SIZE * 4;
        for (auto i = 0; i < ENCODE_VDENC_HEVC_PADDING_DW_SIZE ; i++)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiNoop(&constructedCmdBuf, nullptr));
        }
        startLCU += m_hevcSliceParams[slcCount].NumLCUsInSlice;
    }

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, batchBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ConstructTLB(PMHW_BATCH_BUFFER thirdLevelBatchBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(thirdLevelBatchBuffer);

    MHW_VDBOX_HEVC_PIC_STATE_G12 picStateParams;
    SetHcpPicStateParams(picStateParams);

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(thirdLevelBatchBuffer->OsResource), &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER constructedCmdBuf;
    MOS_ZeroMemory(&constructedCmdBuf, sizeof(constructedCmdBuf));
    constructedCmdBuf.pCmdBase = constructedCmdBuf.pCmdPtr = (uint32_t *)data;
    constructedCmdBuf.iRemaining = m_thirdLBSize;

    SetAddCommands(CODECHAL_CMD1, &constructedCmdBuf, true, m_roundInterValue, m_roundIntraValue, m_lowDelay);

    // HCP_PIC_STATE
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPicStateCmd(&constructedCmdBuf, &picStateParams));

    SetAddCommands(CODECHAL_CMD2, &constructedCmdBuf, true, m_roundInterValue, m_roundIntraValue, m_lowDelay, m_refIdxMapping, m_slotForRecNotFiltered);

    // Send HEVC_VP9_RDOQ_STATE command
    if (m_hevcRdoqEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpHevcVp9RdoqStateCmd(&constructedCmdBuf, &picStateParams));
    }

    thirdLevelBatchBuffer->iCurrent     = constructedCmdBuf.iOffset;
    thirdLevelBatchBuffer->iRemaining   = constructedCmdBuf.iRemaining;
    thirdLevelBatchBuffer->pData        = data;
    // set MI_BATCH_BUFFER_END command
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, thirdLevelBatchBuffer));

    std::string pakPassName = "PAK_PASS[" + std::to_string(GetCurrentPass()) + "]_PIPE[" + std::to_string(GetCurrentPipe()) + "]_TLB";
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &constructedCmdBuf,
            CODECHAL_NUM_MEDIA_STATES,
            pakPassName.data()));)

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &(thirdLevelBatchBuffer->OsResource));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetDmemHuCBrcInitReset()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    // Setup BrcInit DMEM
    auto hucVdencBrcInitDmem = (PCODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12)m_osInterface->pfnLockResource(
        m_osInterface, &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVdencBrcInitDmem);
    MOS_ZeroMemory(hucVdencBrcInitDmem, sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12));

    hucVdencBrcInitDmem->BRCFunc_U32       = (m_enableTileReplay ? 1 : 0) << 7;  //bit0 0: Init; 1: Reset, bit7 0: frame-based; 1: tile-based
    hucVdencBrcInitDmem->UserMaxFrame      = GetProfileLevelMaxFrameSize();
    hucVdencBrcInitDmem->InitBufFull_U32   = MOS_MIN(m_hevcSeqParams->InitVBVBufferFullnessInBit, m_hevcSeqParams->VBVBufferSizeInBit);
    hucVdencBrcInitDmem->BufSize_U32       = m_hevcSeqParams->VBVBufferSizeInBit;
    hucVdencBrcInitDmem->TargetBitrate_U32 = m_hevcSeqParams->TargetBitRate * CODECHAL_ENCODE_BRC_KBPS;  // map DDI params(in Kbits) to huc (in bits)
    hucVdencBrcInitDmem->MaxRate_U32       = m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS;
    hucVdencBrcInitDmem->MinRate_U32 = 0;
    hucVdencBrcInitDmem->FrameRateM_U32    = m_hevcSeqParams->FrameRate.Numerator;
    hucVdencBrcInitDmem->FrameRateD_U32    = m_hevcSeqParams->FrameRate.Denominator;
    hucVdencBrcInitDmem->ACQP_U32          = 0;
    if (m_hevcSeqParams->UserMaxPBFrameSize > 0)
    {
        //Backup CodingType as need to set it as B_Tpye to get MaxFrameSize for P/B frames.
        auto CodingTypeTemp = m_hevcPicParams->CodingType;
        m_hevcPicParams->CodingType = B_TYPE;
        hucVdencBrcInitDmem->ProfileLevelMaxFramePB_U32 = GetProfileLevelMaxFrameSize();
        m_hevcPicParams->CodingType = CodingTypeTemp;
    }
    else
    {
        hucVdencBrcInitDmem->ProfileLevelMaxFramePB_U32 = hucVdencBrcInitDmem->UserMaxFrame;
    }

    if (m_brcEnabled)
    {
        switch (m_hevcSeqParams->RateControlMethod)
        {
        case RATECONTROL_ICQ:
            hucVdencBrcInitDmem->BRCFlag = 0;
            break;
        case RATECONTROL_CBR:
            hucVdencBrcInitDmem->BRCFlag = 1;
            break;
        case RATECONTROL_VBR:
            hucVdencBrcInitDmem->BRCFlag = 2;
            hucVdencBrcInitDmem->ACQP_U32 = 0;
            break;
        case RATECONTROL_VCM:
            hucVdencBrcInitDmem->BRCFlag = 3;
            break;
        case RATECONTROL_QVBR:
            hucVdencBrcInitDmem->BRCFlag = 2;
            hucVdencBrcInitDmem->ACQP_U32 = m_hevcSeqParams->ICQQualityFactor;;
            break;
        default:
            break;
        }

        // Low Delay BRC
        if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
        {
            hucVdencBrcInitDmem->BRCFlag = 5;
        }

        switch (m_hevcSeqParams->MBBRC)
        {
        case mbBrcInternal:
        case mbBrcEnabled:
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 3;
            break;
        case mbBrcDisabled:
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 0;
            break;
        default:
            break;
        }
    }
    else if (m_hevcVdencAcqpEnabled)
    {
        hucVdencBrcInitDmem->BRCFlag = 0;

        // 0=No CUQP; 1=CUQP for I-frame; 2=CUQP for P/B-frame
        // bit operation, bit 1 for I-frame, bit 2 for P/B frame
        // In VDENC mode, the field "Cu_Qp_Delta_Enabled_Flag" should always be set to 1.
        if (m_hevcSeqParams->QpAdjustment)
        {
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 3;  // wPictureCodingType I:0, P:1, B:2
        }
        else
        {
            hucVdencBrcInitDmem->CuQpCtrl_U8 = 0;  // wPictureCodingType I:0, P:1, B:2
        }
    }

    hucVdencBrcInitDmem->SSCFlag = m_hevcSeqParams->SliceSizeControl;

    // LDB case, NumP=0 & NumB=100, but GopP=100 & GopB=0

    hucVdencBrcInitDmem->GopP_U16 = m_hevcSeqParams->GopPicSize - m_hevcSeqParams->NumOfBInGop[0] - 1;
    hucVdencBrcInitDmem->GopB_U16 = (uint16_t)m_hevcSeqParams->NumOfBInGop[0];

    hucVdencBrcInitDmem->FrameWidth_U16 = (uint16_t)m_frameWidth;
    hucVdencBrcInitDmem->FrameHeight_U16 = (uint16_t)m_frameHeight;

    hucVdencBrcInitDmem->GopB1_U16 = (uint16_t)m_hevcSeqParams->NumOfBInGop[1];
    hucVdencBrcInitDmem->GopB2_U16 = (uint16_t)m_hevcSeqParams->NumOfBInGop[2];

    hucVdencBrcInitDmem->MinQP_U8 = m_hevcPicParams->BRCMinQp < 10 ? 10 : m_hevcPicParams->BRCMinQp;                                           // Setting values from arch spec
    hucVdencBrcInitDmem->MaxQP_U8 = m_hevcPicParams->BRCMaxQp < 10 ? 51 : (m_hevcPicParams->BRCMaxQp > 51 ? 51 : m_hevcPicParams->BRCMaxQp);   // Setting values from arch spec

    hucVdencBrcInitDmem->MaxBRCLevel_U8 = 1;
    hucVdencBrcInitDmem->BRCPyramidEnable_U8 = 0;
    //QP modulation settings
    if (m_hevcSeqParams->HierarchicalFlag)
    {
        // Low delay P/B max support Gop 4, layer 3; RA max support Gop 8, layer 4
        hucVdencBrcInitDmem->MaxBRCLevel_U8 = m_hevcSeqParams->LowDelayMode ? 3 : 4;
        hucVdencBrcInitDmem->BRCPyramidEnable_U8 = 1;
    }

    hucVdencBrcInitDmem->LumaBitDepth_U8   = m_hevcSeqParams->bit_depth_luma_minus8 + 8;
    hucVdencBrcInitDmem->ChromaBitDepth_U8 = m_hevcSeqParams->bit_depth_chroma_minus8 + 8;

    if (m_hevcSeqParams->SourceBitDepth == ENCODE_HEVC_BIT_DEPTH_10)
    {
        hucVdencBrcInitDmem->LumaBitDepth_U8 = 10;
        hucVdencBrcInitDmem->ChromaBitDepth_U8 = 10;
    }

    if ((hucVdencBrcInitDmem->LowDelayMode_U8 = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)))
    {
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshPB0_S8, 8 * sizeof(int8_t), (void *)m_lowdelayDevThreshPB, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshVBR0_S8, 8 * sizeof(int8_t), (void*)m_lowdelayDevThreshVBR, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshI0_S8, 8 * sizeof(int8_t), (void*)m_lowdelayDevThreshI, 8 * sizeof(int8_t));
    }
    else
    {
        static int8_t DevThreshPB0_S8[8];
        static int8_t DevThreshVBR0_S8[8];
        static int8_t DevThreshI0_S8[8];

        uint64_t inputbitsperframe = uint64_t(hucVdencBrcInitDmem->MaxRate_U32*100. / (hucVdencBrcInitDmem->FrameRateM_U32 * 100.0 / hucVdencBrcInitDmem->FrameRateD_U32));
        if (m_brcEnabled && !hucVdencBrcInitDmem->BufSize_U32)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("VBV BufSize should not be 0 for BRC case\n");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }
        uint64_t vbvsz = hucVdencBrcInitDmem->BufSize_U32;
        double bps_ratio = inputbitsperframe / (vbvsz / m_devStdFPS);
        if (bps_ratio < m_bpsRatioLow) bps_ratio = m_bpsRatioLow;
        if (bps_ratio > m_bpsRatioHigh) bps_ratio = m_bpsRatioHigh;

        for (int i = 0; i < m_numDevThreshlds / 2; i++) {
            DevThreshPB0_S8[i] = (signed char)(m_negMultPB*pow(m_devThreshPBFPNEG[i], bps_ratio));
            DevThreshPB0_S8[i + m_numDevThreshlds / 2] = (signed char)(m_postMultPB*pow(m_devThreshPBFPPOS[i], bps_ratio));

            DevThreshI0_S8[i] = (signed char)(m_negMultPB*pow(m_devThreshIFPNEG[i], bps_ratio));
            DevThreshI0_S8[i + m_numDevThreshlds / 2] = (signed char)(m_postMultPB*pow(m_devThreshIFPPOS[i], bps_ratio));

            DevThreshVBR0_S8[i] = (signed char)(m_negMultPB*pow(m_devThreshVBRNEG[i], bps_ratio));
            DevThreshVBR0_S8[i + m_numDevThreshlds / 2] = (signed char)(m_posMultVBR*pow(m_devThreshVBRPOS[i], bps_ratio));
        }

        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshPB0_S8, 8 * sizeof(int8_t), (void*)DevThreshPB0_S8, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshVBR0_S8, 8 * sizeof(int8_t), (void*)DevThreshVBR0_S8, 8 * sizeof(int8_t));
        MOS_SecureMemcpy(hucVdencBrcInitDmem->DevThreshI0_S8, 8 * sizeof(int8_t), (void*)DevThreshI0_S8, 8 * sizeof(int8_t));
    }

    MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshP0_S8, 4 * sizeof(int8_t), (void *)m_instRateThreshP0, 4 * sizeof(int8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshB0_S8, 4 * sizeof(int8_t), (void *)m_instRateThreshB0, 4 * sizeof(int8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->InstRateThreshI0_S8, 4 * sizeof(int8_t), (void *)m_instRateThreshI0, 4 * sizeof(int8_t));

    if (m_brcEnabled)
    {
        // initQPIP, initQPB values will be used for BRC in the future
        int32_t initQPIP = 0, initQPB = 0;
        ComputeVDEncInitQP(initQPIP, initQPB);
        hucVdencBrcInitDmem->InitQPIP_U8 = (uint8_t)initQPIP;
        hucVdencBrcInitDmem->InitQPB_U8 = (uint8_t)initQPB;
    }
    else
    {
        hucVdencBrcInitDmem->InitQPIP_U8 = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
        hucVdencBrcInitDmem->InitQPB_U8  = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    }

    hucVdencBrcInitDmem->TopFrmSzThrForAdapt2Pass_U8 = 32;
    hucVdencBrcInitDmem->BotFrmSzThrForAdapt2Pass_U8 = 24;

    MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshP0_U8, 7 * sizeof(uint8_t), (void*)m_estRateThreshP0, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshB0_U8, 7 * sizeof(uint8_t), (void*)m_estRateThreshB0, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcInitDmem->EstRateThreshI0_U8, 7 * sizeof(uint8_t), (void*)m_estRateThreshI0, 7 * sizeof(uint8_t));

    if (m_vdencStreamInEnabled && m_hevcPicParams->NumROI && !m_vdencNativeROIEnabled)
    {
        hucVdencBrcInitDmem->StreamInROIEnable_U8 = 1;
        hucVdencBrcInitDmem->StreamInSurfaceEnable_U8 = 1;
    }

    hucVdencBrcInitDmem->TopQPDeltaThrForAdapt2Pass_U8 = 2;
    hucVdencBrcInitDmem->BotQPDeltaThrForAdapt2Pass_U8 = 1;

    uint32_t framerate = m_hevcSeqParams->FrameRate.Numerator / m_hevcSeqParams->FrameRate.Denominator;
    hucVdencBrcInitDmem->SlidingWindow_Size_U32 = MOS_MIN(framerate, 60);
    hucVdencBrcInitDmem->SLIDINGWINDOW_MaxRateRatio = 120;

    // Tile Row based BRC 
    if (m_enableTileReplay)
    {
        uint32_t shift                        = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
        uint32_t residual                     = (1 << shift) - 1;
        hucVdencBrcInitDmem->SlideWindowRC    = 0;  //Reserved for now
        hucVdencBrcInitDmem->MaxLogCUSize     = m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3;
        hucVdencBrcInitDmem->FrameWidthInLCU  = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1 + residual) >> shift;
        hucVdencBrcInitDmem->FrameHeightInLCU = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1 + residual) >> shift;
    }

    // Long term reference
    hucVdencBrcInitDmem->LongTermRefEnable_U8  = true;
    hucVdencBrcInitDmem->LongTermRefMsdk_U8 = true;
    hucVdencBrcInitDmem->IsLowDelay_U8 = m_lowDelay;

    m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetConstDataHuCBrcUpdate()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    auto hucConstData = (PCODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G12)m_osInterface->pfnLockResource(
        m_osInterface, &m_vdencBrcConstDataBuffer[m_currRecycledBufIdx], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucConstData);

    MOS_SecureMemcpy(hucConstData->SLCSZ_THRDELTAI_U16, sizeof(m_hucConstantData), m_hucConstantData, sizeof(m_hucConstantData));

    MOS_SecureMemcpy(hucConstData->RDQPLambdaI, sizeof(m_rdQpLambdaI), m_rdQpLambdaI, sizeof(m_rdQpLambdaI));
    MOS_SecureMemcpy(hucConstData->RDQPLambdaP, sizeof(m_rdQpLambdaP), m_rdQpLambdaP, sizeof(m_rdQpLambdaP));

    if (m_hevcVisualQualityImprovement)
    {
        MOS_SecureMemcpy(hucConstData->SADQPLambdaI, sizeof(m_sadQpLambdaI), m_sadQpLambdaI_VQI, sizeof(m_sadQpLambdaI_VQI));
        MOS_SecureMemcpy(hucConstData->PenaltyForIntraNonDC32x32PredMode, sizeof(m_penaltyForIntraNonDC32x32PredMode), m_penaltyForIntraNonDC32x32PredMode_VQI, sizeof(m_penaltyForIntraNonDC32x32PredMode_VQI));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->SADQPLambdaI, sizeof(m_sadQpLambdaI), m_sadQpLambdaI, sizeof(m_sadQpLambdaI));
        MOS_SecureMemcpy(hucConstData->PenaltyForIntraNonDC32x32PredMode, sizeof(m_penaltyForIntraNonDC32x32PredMode), m_penaltyForIntraNonDC32x32PredMode, sizeof(m_penaltyForIntraNonDC32x32PredMode));
    }

    MOS_SecureMemcpy(hucConstData->SADQPLambdaP, sizeof(m_sadQpLambdaP), m_sadQpLambdaP, sizeof(m_sadQpLambdaP));

    if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)
    {
        const int numEstrateThreshlds = 7;

        for (int i = 0; i < numEstrateThreshlds + 1; i++)
        {
            for (int j = 0; j < m_numDevThreshlds + 1; j++)
            {
                hucConstData->FrmSzAdjTabI_S8[(numEstrateThreshlds + 1)*j + i] = m_lowdelayDeltaFrmszI[j][i];
                hucConstData->FrmSzAdjTabP_S8[(numEstrateThreshlds + 1)*j + i] = m_lowdelayDeltaFrmszP[j][i];
                hucConstData->FrmSzAdjTabB_S8[(numEstrateThreshlds + 1)*j + i] = m_lowdelayDeltaFrmszB[j][i];
            }
        }
    }

    // ModeCosts depends on frame type
    if (m_pictureCodingType == I_TYPE)
    {
        MOS_SecureMemcpy(hucConstData->ModeCosts, sizeof(m_hucModeCostsIFrame), m_hucModeCostsIFrame, sizeof(m_hucModeCostsIFrame));
    }
    else
    {
        MOS_SecureMemcpy(hucConstData->ModeCosts, sizeof(m_hucModeCostsPbFrame), m_hucModeCostsPbFrame, sizeof(m_hucModeCostsPbFrame));
    }

    // starting location in batch buffer for each slice
    uint32_t baseLocation = m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize;
    uint32_t currentLocation = baseLocation;

    auto slcData = m_slcData;
    // HCP_WEIGHTSOFFSETS_STATE + HCP_SLICE_STATE + HCP_PAK_INSERT_OBJECT + VDENC_WEIGHT_OFFSETS_STATE
    for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        auto hevcSlcParams = &m_hevcSliceParams[slcCount];
        // HuC FW require unit in Bytes
        hucConstData->Slice[slcCount].SizeOfCMDs
            = (uint16_t)(m_hwInterface->m_vdencBatchBufferPerSliceConstSize + m_vdencBatchBufferPerSliceVarSize[slcCount]);

        // HCP_WEIGHTOFFSET_STATE cmd
        if (m_hevcVdencWeightedPredEnabled)
        {
            // 1st HCP_WEIGHTOFFSET_STATE cmd - P & B
            if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_P_SLICE || hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hucConstData->Slice[slcCount].HcpWeightOffsetL0_StartInBytes = (uint16_t)currentLocation;   // HCP_WEIGHTOFFSET_L0 starts in byte from beginning of the SLB. 0xFFFF means unavailable in SLB
                currentLocation += m_hcpWeightOffsetStateCmdSize;
            }

            // 2nd HCP_WEIGHTOFFSET_STATE cmd - B
            if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
            {
                hucConstData->Slice[slcCount].HcpWeightOffsetL1_StartInBytes = (uint16_t)currentLocation; // HCP_WEIGHTOFFSET_L1 starts in byte from beginning of the SLB. 0xFFFF means unavailable in SLB
                currentLocation += m_hcpWeightOffsetStateCmdSize;
            }
        }
        else
        {
            // 0xFFFF means unavailable in SLB
            hucConstData->Slice[slcCount].HcpWeightOffsetL0_StartInBytes = 0xFFFF;
            hucConstData->Slice[slcCount].HcpWeightOffsetL1_StartInBytes = 0xFFFF;
        }

        // HCP_SLICE_STATE cmd
        hucConstData->Slice[slcCount].SliceState_StartInBytes = (uint16_t)currentLocation;  // HCP_WEIGHTOFFSET is not needed
        currentLocation += m_hcpSliceStateCmdSize;

        // VDENC_WEIGHT_OFFSETS_STATE cmd
        hucConstData->Slice[slcCount].VdencWeightOffset_StartInBytes                      // VdencWeightOffset cmd is the last one expect BatchBufferEnd cmd
            = (uint16_t)(baseLocation + hucConstData->Slice[slcCount].SizeOfCMDs - m_vdencWeightOffsetStateCmdSize - m_miBatchBufferEndCmdSize - ENCODE_VDENC_HEVC_PADDING_DW_SIZE * 4);

        // logic from PakInsertObject cmd
        uint32_t bitSize         = (m_hevcSeqParams->SliceSizeControl) ? (hevcSlcParams->BitLengthSliceHeaderStartingPortion) : slcData[slcCount].BitSize;  // 40 for HEVC VDEnc Dynamic Slice
        uint32_t byteSize = (bitSize + 7) >> 3;
        uint32_t sliceHeaderSizeInBytes = (bitSize + 7) >> 3;
        // 1st PakInsertObject cmd with AU, SPS, PPS headers only exists for the first slice
        if (slcCount == 0)
        {
            // assumes that there is no 3rd PakInsertObject cmd for SSC
            currentLocation += m_1stPakInsertObjectCmdSize;
        }

        hucConstData->Slice[slcCount].SliceHeaderPIO_StartInBytes = (uint16_t)currentLocation;

        // HuC FW requires true slice header size in bits without byte alignment
        hucConstData->Slice[slcCount].SliceHeader_SizeInBits = (uint16_t)(sliceHeaderSizeInBytes * 8);
        if (!IsFirstPass())
        {
            PBSBuffer bsBuffer = &m_bsBuffer;
            CODECHAL_ENCODE_CHK_NULL_RETURN(bsBuffer);
            CODECHAL_ENCODE_CHK_NULL_RETURN(bsBuffer->pBase);
            uint8_t *sliceHeaderLastByte = (uint8_t*)(bsBuffer->pBase + slcData[slcCount].SliceOffset + sliceHeaderSizeInBytes - 1);
            for (auto i = 0; i < 8; i++)
            {
                uint8_t mask = 1 << i;
                if (*sliceHeaderLastByte & mask)
                {
                    hucConstData->Slice[slcCount].SliceHeader_SizeInBits -= (i + 1);
                    break;
                }
            }
        }

        if (m_hevcVdencWeightedPredEnabled)
        {
            hucConstData->Slice[slcCount].WeightTable_StartInBits = (uint16_t)hevcSlcParams->PredWeightTableBitOffset;
            hucConstData->Slice[slcCount].WeightTable_EndInBits = (uint16_t)(hevcSlcParams->PredWeightTableBitOffset + (hevcSlcParams->PredWeightTableBitLength));
        }
        else
        {
            // number of bits from beginning of slice header, 0xffff means not awailable
            hucConstData->Slice[slcCount].WeightTable_StartInBits = 0xFFFF;
            hucConstData->Slice[slcCount].WeightTable_EndInBits = 0xFFFF;
        }

        baseLocation += hucConstData->Slice[slcCount].SizeOfCMDs;
        currentLocation = baseLocation;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcConstDataBuffer[m_currRecycledBufIdx]);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetDmemHuCBrcUpdate()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;
    uint32_t currentPass = m_enableTileReplay ? m_CurrentPassForOverAll : GetCurrentPass();

    // Program update DMEM
    auto hucVdencBrcUpdateDmem = (PCODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12)m_osInterface->pfnLockResource(
        m_osInterface, &m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][currentPass], &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucVdencBrcUpdateDmem);
    MOS_ZeroMemory(hucVdencBrcUpdateDmem, sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12));

    hucVdencBrcUpdateDmem->TARGETSIZE_U32 = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)? m_hevcSeqParams->InitVBVBufferFullnessInBit :
                                            MOS_MIN(m_hevcSeqParams->InitVBVBufferFullnessInBit, m_hevcSeqParams->VBVBufferSizeInBit);
    hucVdencBrcUpdateDmem->FrameID_U32 = m_storeData;    // frame number
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjFrame_U16, 4 * sizeof(uint16_t), (void*)m_startGAdjFrame, 4 * sizeof(uint16_t));
    hucVdencBrcUpdateDmem->TargetSliceSize_U16           = (uint16_t)m_hevcPicParams->MaxSliceSizeInBytes;
    auto slbSliceSize = (m_hwInterface->m_vdenc2ndLevelBatchBufferSize - m_hwInterface->m_vdencBatchBuffer1stGroupSize -
        m_hwInterface->m_vdencBatchBuffer2ndGroupSize) / ENCODE_HEVC_VDENC_NUM_MAX_SLICES;
    hucVdencBrcUpdateDmem->SLB_Data_SizeInBytes = (uint16_t)(slbSliceSize * m_numSlices +
        m_hwInterface->m_vdencBatchBuffer1stGroupSize + m_hwInterface->m_vdencBatchBuffer2ndGroupSize);
    hucVdencBrcUpdateDmem->PIPE_MODE_SELECT_StartInBytes = 0xFFFF;    // HuC need not need to modify the pipe mode select command in Gen11+
    hucVdencBrcUpdateDmem->CMD1_StartInBytes = (uint16_t)m_hwInterface->m_vdencBatchBuffer1stGroupSize;
    hucVdencBrcUpdateDmem->PIC_STATE_StartInBytes = (uint16_t)m_picStateCmdStartInBytes;
    hucVdencBrcUpdateDmem->CMD2_StartInBytes = (uint16_t)m_cmd2StartInBytes;

    if (m_prevStoreData != m_storeData) 
    {
        m_prevStoreData = m_storeData;

        int32_t oldestIdx = -1;
        int32_t selectedSlot = -1;
        uint32_t oldestAge = 0;
        for (int i = 0; i < CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER; i++)
        {
            if (slotInfo[i].isUsed == true && slotInfo[i].isRef)
            {
                slotInfo[i].age++;
                if (slotInfo[i].age >= oldestAge)
                {
                    oldestAge = slotInfo[i].age;
                    oldestIdx = i;
                }
            }
            if ((selectedSlot == -1) && (slotInfo[i].isUsed == false || !slotInfo[i].isRef))
            {
                selectedSlot = i;
            }
        }

        if (selectedSlot == -1)
        {
            selectedSlot = oldestIdx;
        }

        slotInfo[selectedSlot].age = 0;
        slotInfo[selectedSlot].poc = m_hevcPicParams->CurrPicOrderCnt;
        slotInfo[selectedSlot].isUsed = true;
        slotInfo[selectedSlot].isRef = m_hevcPicParams->bUsedAsRef;

        m_curPicSlot = selectedSlot;
    }

    hucVdencBrcUpdateDmem->Current_Data_Offset = m_curPicSlot * m_weightHistSize;

    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][refIdx];
        auto refPOC = m_hevcPicParams->RefFramePOCList[refPic.FrameIdx];
        for (int i = 0; i < CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER; i++)
        {
            if (slotInfo[i].poc == refPOC)
            {
                hucVdencBrcUpdateDmem->Ref_Data_Offset[refIdx] = i * m_weightHistSize;
                break;
            }
        }
    }

    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_1][refIdx];
        auto refPOC = m_hevcPicParams->RefFramePOCList[refPic.FrameIdx];
        for (int i = 0; i < CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER; i++)
        {
            if (slotInfo[i].poc == refPOC)
            {
                hucVdencBrcUpdateDmem->Ref_Data_Offset[refIdx + m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1] = i * m_weightHistSize;
                break;
            }
        }
    }

    hucVdencBrcUpdateDmem->MaxNumSliceAllowed_U16 = (uint16_t)GetMaxAllowedSlices(m_hevcSeqParams->Level);

    if (m_FrameLevelBRCForTileRow)
    {
        hucVdencBrcUpdateDmem->OpMode_U8 = 0x4;
    }
    else if (m_TileRowLevelBRC)
    {
        hucVdencBrcUpdateDmem->OpMode_U8 = 0x8;
    }
    else
    {
        hucVdencBrcUpdateDmem->OpMode_U8         // 1: BRC (including ACQP), 2: Weighted prediction (should not be enabled in first pass)
            = (m_hevcVdencWeightedPredEnabled && m_hevcPicParams->bEnableGPUWeightedPrediction && !IsFirstPass()) ? 3 : 1;    // 01: BRC, 10: WP never used,  11: BRC + WP
    }

    if (m_pictureCodingType == I_TYPE)
    {
        hucVdencBrcUpdateDmem->CurrentFrameType_U8 = HEVC_BRC_FRAME_TYPE_I;
    }
    else if (m_hevcSeqParams->HierarchicalFlag)
    {
        if (m_hevcPicParams->HierarchLevelPlus1 > 0)
        {
            std::map<int, HEVC_BRC_FRAME_TYPE> hierchLevelPlus1_to_brclevel{
            {1, HEVC_BRC_FRAME_TYPE_P_OR_LB},
            {2, HEVC_BRC_FRAME_TYPE_B},
            {3, HEVC_BRC_FRAME_TYPE_B1},
            {4, HEVC_BRC_FRAME_TYPE_B2}};
            hucVdencBrcUpdateDmem->CurrentFrameType_U8 = hierchLevelPlus1_to_brclevel.count(m_hevcPicParams->HierarchLevelPlus1) ? hierchLevelPlus1_to_brclevel[m_hevcPicParams->HierarchLevelPlus1] : HEVC_BRC_FRAME_TYPE_INVALID;
            //Invalid HierarchLevelPlus1 or LBD frames at level 3 eror check.
            if ((hucVdencBrcUpdateDmem->CurrentFrameType_U8 == HEVC_BRC_FRAME_TYPE_INVALID) ||
                (m_hevcSeqParams->LowDelayMode && hucVdencBrcUpdateDmem->CurrentFrameType_U8 == HEVC_BRC_FRAME_TYPE_B2))
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("HEVC_BRC_FRAME_TYPE_INVALID or LBD picture doesn't support Level 4\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else if(!m_hevcSeqParams->LowDelayMode) //RA
        {
            //if L0/L1 both points to previous frame, then its LBD otherwise its is level 1 RA B.
            auto B_or_LDB_brclevel = m_lowDelay ? HEVC_BRC_FRAME_TYPE_P_OR_LB : HEVC_BRC_FRAME_TYPE_B;
            std::map<int, HEVC_BRC_FRAME_TYPE> codingtype_to_brclevel{
            {P_TYPE, HEVC_BRC_FRAME_TYPE_P_OR_LB},
            {B_TYPE, B_or_LDB_brclevel},
            {B1_TYPE, HEVC_BRC_FRAME_TYPE_B1},
            {B2_TYPE, HEVC_BRC_FRAME_TYPE_B2}};
             hucVdencBrcUpdateDmem->CurrentFrameType_U8 = codingtype_to_brclevel.count(m_pictureCodingType) ? codingtype_to_brclevel[m_pictureCodingType] : HEVC_BRC_FRAME_TYPE_INVALID;
            //Invalid CodingType.
            if (hucVdencBrcUpdateDmem->CurrentFrameType_U8 == HEVC_BRC_FRAME_TYPE_INVALID)
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid CodingType\n");
                return MOS_STATUS_INVALID_PARAMETER;
            }
        }
        else //LDB
        {
            hucVdencBrcUpdateDmem->CurrentFrameType_U8 = HEVC_BRC_FRAME_TYPE_P_OR_LB; //No Hierarchical info for LDB, treated as flat case
        }
    }
    else
    {
        hucVdencBrcUpdateDmem->CurrentFrameType_U8 = HEVC_BRC_FRAME_TYPE_P_OR_LB;
    }

    // Num_Ref_L1 should be always same as Num_Ref_L0
    hucVdencBrcUpdateDmem->Num_Ref_L0_U8 = m_hevcSliceParams->num_ref_idx_l0_active_minus1 + 1;
    hucVdencBrcUpdateDmem->Num_Ref_L1_U8 = m_hevcSliceParams->num_ref_idx_l1_active_minus1 + 1;
    hucVdencBrcUpdateDmem->Num_Slices    = (uint8_t)m_hevcPicParams->NumSlices;

    // CQP_QPValue_U8 setting is needed since ACQP is also part of ICQ
    hucVdencBrcUpdateDmem->CQP_QPValue_U8 = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    hucVdencBrcUpdateDmem->CQP_FracQP_U8 = 0;
    if (m_hevcPicParams->BRCPrecision == 1)
    {
        hucVdencBrcUpdateDmem->MaxNumPass_U8 = 1;
    }
    else
    {
        hucVdencBrcUpdateDmem->MaxNumPass_U8 = CODECHAL_VDENC_BRC_NUM_OF_PASSES;
    }

    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->gRateRatioThreshold_U8, 7 * sizeof(uint8_t), (void*)m_rateRatioThreshold, 7 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjMult_U8, 5 * sizeof(uint8_t), (void*)m_startGAdjMult, 5 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->startGAdjDiv_U8, 5 * sizeof(uint8_t), (void*)m_startGAdjDiv, 5 * sizeof(uint8_t));
    MOS_SecureMemcpy(hucVdencBrcUpdateDmem->gRateRatioThresholdQP_U8, 8 * sizeof(uint8_t), (void*)m_rateRatioThresholdQP, 8 * sizeof(uint8_t));

    hucVdencBrcUpdateDmem->IPAverageCoeff_U8 = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW) ? 0 : 64;
    hucVdencBrcUpdateDmem->CurrentPass_U8 = (uint8_t)currentPass;

    if ((m_hevcVdencAcqpEnabled && m_hevcSeqParams->QpAdjustment) || (m_brcEnabled && (m_hevcSeqParams->MBBRC != 2)))
    {
        hucVdencBrcUpdateDmem->DeltaQPForSadZone0_S8 = -5;
        hucVdencBrcUpdateDmem->DeltaQPForSadZone1_S8 = -2;
        hucVdencBrcUpdateDmem->DeltaQPForSadZone2_S8 = 2;
        hucVdencBrcUpdateDmem->DeltaQPForSadZone3_S8 = 5;
        hucVdencBrcUpdateDmem->DeltaQPForMvZero_S8   = -4;
        hucVdencBrcUpdateDmem->DeltaQPForMvZone0_S8  = -2;
        hucVdencBrcUpdateDmem->DeltaQPForMvZone1_S8  = 0;
        hucVdencBrcUpdateDmem->DeltaQPForMvZone2_S8  = 2;
    }

    if (m_hevcVdencWeightedPredEnabled)
    {
        hucVdencBrcUpdateDmem->LumaLog2WeightDenom_S8 = 6;
        hucVdencBrcUpdateDmem->ChromaLog2WeightDenom_S8 = 6;
    }

    // chroma weights are not confirmed to be supported from HW team yet
    hucVdencBrcUpdateDmem->DisabledFeature_U8 = 0; // bit mask, 1 (bit0): disable chroma weight setting

    hucVdencBrcUpdateDmem->SlidingWindow_Enable_U8 = (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    hucVdencBrcUpdateDmem->LOG_LCU_Size_U8 = 6;
    hucVdencBrcUpdateDmem->ReEncodePositiveQPDeltaThr_S8    = 4;
    hucVdencBrcUpdateDmem->ReEncodeNegativeQPDeltaThr_S8    = -5;
    hucVdencBrcUpdateDmem->SceneChgPrevIntraPctThreshold_U8 = 96;
    hucVdencBrcUpdateDmem->SceneChgCurIntraPctThreshold_U8  = 192;

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_rsvdState->SetDmemHuCBrcUpdate(hucVdencBrcUpdateDmem));
    }
#endif

    // reset skip frame statistics
    m_numSkipFrames = 0;
    m_sizeSkipFrames = 0;

    // For tile row based BRC
    if (m_TileRowLevelBRC)
    {
        hucVdencBrcUpdateDmem->MaxNumTileHuCCallMinus1 = m_hevcPicParams->num_tile_rows_minus1;
        hucVdencBrcUpdateDmem->TileHucCallIndex        = (uint8_t)m_CurrentTileRow;
        hucVdencBrcUpdateDmem->TileHuCCallPassIndex    = m_CurrentPassForTileReplay + 1;
        hucVdencBrcUpdateDmem->TileHuCCallPassMax      = m_NumPassesForTileReplay;

        // Need change App to pass real max bit rate rather than to enlarge it with 1000
        if (m_hevcSeqParams->FrameRate.Numerator)
        {
            hucVdencBrcUpdateDmem->TxSizeInBitsPerFrame = (uint32_t)(((uint32_t)m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS *
                m_hevcSeqParams->FrameRate.Denominator + (m_hevcSeqParams->FrameRate.Numerator >> 1)) /
                m_hevcSeqParams->FrameRate.Numerator);
        }
        else
        {
            hucVdencBrcUpdateDmem->TxSizeInBitsPerFrame = (uint32_t)(((uint32_t)m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS + 15) / 30);
        }

        uint32_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
        uint32_t startIdx       = m_CurrentTileRow * numTileColumns;
        uint32_t endIdx         = startIdx + numTileColumns - 1;
        uint32_t LCUsInTile     = 0;

        for (uint32_t idx = 0; idx < numTileColumns; idx ++)
        {
            LCUsInTile += m_hevcPicParams->tile_row_height[m_CurrentTileRow] * m_hevcPicParams->tile_column_width[idx];
        }

        hucVdencBrcUpdateDmem->StartTileIdx            = (uint8_t)startIdx;
        hucVdencBrcUpdateDmem->EndTileIdx              = (uint8_t)endIdx;
        hucVdencBrcUpdateDmem->TileSizeInLCU           = (uint16_t)LCUsInTile;
    }
    else if (m_FrameLevelBRCForTileRow)
    {
        hucVdencBrcUpdateDmem->MaxNumTileHuCCallMinus1 = m_hevcPicParams->num_tile_rows_minus1;
        hucVdencBrcUpdateDmem->TileHucCallIndex        = 0;
        hucVdencBrcUpdateDmem->TileHuCCallPassIndex    = 0;
        hucVdencBrcUpdateDmem->TileHuCCallPassMax      = m_NumPassesForTileReplay;

        // Need change App to pass real max bit rate rather than to enlarge it with 1000
        if (m_hevcSeqParams->FrameRate.Numerator)
        {
            hucVdencBrcUpdateDmem->TxSizeInBitsPerFrame = (uint32_t)(((uint32_t)m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS *
                m_hevcSeqParams->FrameRate.Denominator + (m_hevcSeqParams->FrameRate.Numerator >> 1)) /
                m_hevcSeqParams->FrameRate.Numerator);
        }
        else
        {
            hucVdencBrcUpdateDmem->TxSizeInBitsPerFrame = (uint32_t)(((uint32_t)m_hevcSeqParams->MaxBitRate * CODECHAL_ENCODE_BRC_KBPS + 15) / 30);
        }
    }

    // Long term reference
    hucVdencBrcUpdateDmem->IsLongTermRef = CodecHal_PictureIsLongTermRef(m_currReconstructedPic);

    m_osInterface->pfnUnlockResource(m_osInterface, &m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][currentPass]);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetRegionsHuCBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::SetRegionsHuCBrcUpdate(virtualAddrParams));

    // With multiple tiles, ensure that HuC BRC kernel is fed with vdenc frame level statistics from HuC PAK Int kernel
    // Applicable for scalable/ non-scalable mode
    if (m_hevcPicParams->tiles_enabled_flag)
    {
        virtualAddrParams->regionParams[1].presRegion = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 1  VDEnc Statistics Buffer (Input) - VDENC_HEVC_VP9_FRAME_BASED_STATISTICS_STREAMOUT
        virtualAddrParams->regionParams[1].dwOffset   = m_hevcFrameStatsOffset.uiVdencStatistics;
    }

    if (m_numPipe > 1)
    {
        virtualAddrParams->regionParams[2].presRegion = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 2  PAK Statistics Buffer (Input) - MFX_PAK_FRAME_STATISTICS
        virtualAddrParams->regionParams[2].dwOffset   = m_hevcFrameStatsOffset.uiHevcPakStatistics;
        virtualAddrParams->regionParams[7].presRegion = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 7  Slice Stat Streamout (Input)
        virtualAddrParams->regionParams[7].dwOffset   = m_hevcFrameStatsOffset.uiHevcSliceStreamout;
        // In scalable-mode, use PAK Integration kernel output to get bistream size
        virtualAddrParams->regionParams[8].presRegion   = &m_resBrcDataBuffer;
    }

    // Tile reset case, use previous frame BRC data
    if ((m_numPipe != m_numPipePre) && IsFirstPass())
    {
        if (m_numPipePre > 1)
        {
            virtualAddrParams->regionParams[8].presRegion   = &m_resBrcDataBuffer;
        }
        else
        {
            virtualAddrParams->regionParams[8].presRegion   = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, pakInfo);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetRegionsHuCTileRowBrcUpdate(PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::SetRegionsHuCBrcUpdate(virtualAddrParams));

    // For tile replay, the tile based statistics is directly passed to HUC kernel
    virtualAddrParams->regionParams[1].presRegion = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 1 � VDEnc Statistics Buffer (Input) 
    virtualAddrParams->regionParams[1].dwOffset   = m_hevcTileStatsOffset.uiVdencStatistics;

    virtualAddrParams->regionParams[2].presRegion = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 2 � PAK Statistics Buffer (Input) 
    virtualAddrParams->regionParams[2].dwOffset   = m_hevcTileStatsOffset.uiHevcPakStatistics;

    virtualAddrParams->regionParams[7].presRegion = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 7 � Slice Stat Streamout (Input)
    virtualAddrParams->regionParams[7].dwOffset   = m_hevcTileStatsOffset.uiHevcSliceStreamout;

    virtualAddrParams->regionParams[12].presRegion = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;             // Region 12 � Tile encoded information (Input) 

    return eStatus;
}

void CodechalVdencHevcStateG12::SetHcpSliceStateCommonParams(MHW_VDBOX_HEVC_SLICE_STATE& sliceStateParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalVdencHevcState::SetHcpSliceStateCommonParams(sliceStateParams);

    static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12 &>(sliceStateParams).dwNumPipe = m_numPipe;

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetHcpSliceStateCommonParams(sliceStateParams, m_slotForRecNotFiltered);
    }
#endif
}

void CodechalVdencHevcStateG12::SetHcpSliceStateParams(
    MHW_VDBOX_HEVC_SLICE_STATE&           sliceState,
    PCODEC_ENCODER_SLCDATA                slcData,
    uint16_t                              slcCount,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileCodingParams,
    bool                                  lastSliceInTile,
    uint32_t                              idx)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpSliceStateParams(sliceState, slcData, slcCount);

    sliceState.bLastSliceInTile = lastSliceInTile ? true : false;
    sliceState.bLastSliceInTileColumn = (lastSliceInTile & tileCodingParams[idx].IsLastTileofColumn) ? true : false;
    static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12&>(sliceState).pTileCodingParams = tileCodingParams + idx;
    static_cast<MHW_VDBOX_HEVC_SLICE_STATE_G12&>(sliceState).dwTileID = idx;

    // update pass status
    if (m_enableTileReplay && m_FrameLevelBRCForTileRow)
    {
        sliceState.bFirstPass = true;
        sliceState.bLastPass  = false;
    }
    else if (m_enableTileReplay && m_TileRowLevelBRC)
    {
        sliceState.bFirstPass = IsFirstPassForTileReplay();
        sliceState.bLastPass  = IsLastPassForTileReplay();
    }
}

void CodechalVdencHevcStateG12::SetHcpPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& vdboxPipeModeSelectParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpPipeModeSelectParams(vdboxPipeModeSelectParams);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12& pipeModeSelectParams = static_cast<MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12&>(vdboxPipeModeSelectParams);

    if (m_numPipe > 1)
    {
        // Running in the multiple VDBOX mode
        if (IsFirstPipe())
        {
            pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;
        }
        else if (IsLastPipe())
        {
            pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;
        }
        else
        {
            pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;
        }
        pipeModeSelectParams.PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;
    }
    else
    {
        pipeModeSelectParams.MultiEngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;
        pipeModeSelectParams.PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_LEGACY;
    }

    // In single pipe mode, if TileBasedReplayMode is enabled, the bit stream for each tile will not be continuous
    if (m_hevcPicParams->tiles_enabled_flag)
    {
        pipeModeSelectParams.bTileBasedReplayMode = m_enableTileReplay;
    }
    else
    {
        pipeModeSelectParams.bTileBasedReplayMode = 0;
    }

    // To enable VDENC/PAK statistics stream out for BRC only
    // Is stream out needed for ACQP? check this out!
    pipeModeSelectParams.bBRCEnabled = m_hevcVdencAcqpEnabled || m_vdencBrcEnabled;
}

void CodechalVdencHevcStateG12::SetVdencPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS& vdboxPipeModeSelectParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalVdencHevcState::SetVdencPipeModeSelectParams(vdboxPipeModeSelectParams);

    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12& pipeModeSelectParams = static_cast<MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12&>(vdboxPipeModeSelectParams);

    // Enable RGB encoding
    pipeModeSelectParams.bRGBEncodingMode  = m_RGBEncodingEnable;

    // Capture mode enable
    pipeModeSelectParams.bWirelessEncodeEnabled = m_CaptureModeEnable;
    pipeModeSelectParams.ucWirelessSessionId    = 0;

    // Set random access flag
    pipeModeSelectParams.bIsRandomAccess        = !m_lowDelay;

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetVdencPipeModeSelectParams(pipeModeSelectParams);
    }
#endif
}

void CodechalVdencHevcStateG12::SetHcpPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpPipeBufAddrParams(pipeBufAddrParams);

    PCODECHAL_ENCODE_BUFFER tileStatisticsBuffer = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex];
    if (!Mos_ResourceIsNull(&tileStatisticsBuffer->sResource) && (m_numPipe > 1))
    {
        pipeBufAddrParams.presLcuBaseAddressBuffer     = &tileStatisticsBuffer->sResource;
        pipeBufAddrParams.dwLcuStreamOutOffset         = m_hevcTileStatsOffset.uiHevcSliceStreamout;
        pipeBufAddrParams.presFrameStatStreamOutBuffer = &tileStatisticsBuffer->sResource;
        pipeBufAddrParams.dwFrameStatStreamOutOffset   = m_hevcTileStatsOffset.uiHevcPakStatistics;
    }

    // SAO Row Store is GEN12 specific
    pipeBufAddrParams.presSaoRowStoreBuffer = &m_vdencSAORowStoreBuffer;

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetHcpPipeBufAddrParams(pipeBufAddrParams, m_slotForRecNotFiltered);
    }
#endif

}

void CodechalVdencHevcStateG12::SetHcpPicStateParams(MHW_VDBOX_HEVC_PIC_STATE& picStateParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncodeHevcBase::SetHcpPicStateParams(picStateParams);

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetHcpPicStateParams(picStateParams);
    }
#endif
}

MOS_STATUS CodechalVdencHevcStateG12::AddHcpRefIdxCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER batchBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcSliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pEncodeHevcPicParams);

    if (cmdBuffer == nullptr && batchBuffer == nullptr)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        return MOS_STATUS_NULL_POINTER;
    }

    PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = params->pEncodeHevcPicParams;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = params->pEncodeHevcSliceParams;

    if ((hevcPicParams->pps_curr_pic_ref_enabled_flag) || (hevcSlcParams->slice_type != CODECHAL_ENCODE_HEVC_I_SLICE))
    {
        MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 refIdxParams;

        refIdxParams.CurrPic = hevcPicParams->CurrReconstructedPic;
        refIdxParams.isEncode = true;
        refIdxParams.ucList = LIST_0;
        refIdxParams.ucNumRefForList = hevcSlcParams->num_ref_idx_l0_active_minus1 + 1;
        eStatus = MOS_SecureMemcpy(&refIdxParams.RefPicList, sizeof(refIdxParams.RefPicList),
            &hevcSlcParams->RefPicList, sizeof(hevcSlcParams->RefPicList));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        refIdxParams.hevcRefList = (void**)m_refList;
        refIdxParams.poc_curr_pic = hevcPicParams->CurrPicOrderCnt;
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            refIdxParams.poc_list[i] = hevcPicParams->RefFramePOCList[i];
        }

        refIdxParams.pRefIdxMapping = params->pRefIdxMapping;
        refIdxParams.RefFieldPicFlag = 0; // there is no interlaced support in encoder
        refIdxParams.RefBottomFieldFlag = 0; // there is no interlaced support in encoder

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetRefIdxParams(refIdxParams);
    }
#endif

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(cmdBuffer, batchBuffer, &refIdxParams));

        if (hevcSlcParams->slice_type == CODECHAL_ENCODE_HEVC_B_SLICE)
        {
            refIdxParams.ucList = LIST_1;
            refIdxParams.ucNumRefForList = hevcSlcParams->num_ref_idx_l1_active_minus1 + 1;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpRefIdxStateCmd(cmdBuffer, batchBuffer, &refIdxParams));
        }
    }

    return eStatus;
}

void CodechalVdencHevcStateG12::SetVdencPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalVdencHevcState::SetVdencPipeBufAddrParams(pipeBufAddrParams);

    PCODECHAL_ENCODE_BUFFER tileStatisticsBuffer    = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex];
    if (!Mos_ResourceIsNull(&tileStatisticsBuffer->sResource))
    {
        pipeBufAddrParams.presVdencStreamOutBuffer = &tileStatisticsBuffer->sResource;
        pipeBufAddrParams.dwVdencStatsStreamOutOffset = m_hevcTileStatsOffset.uiVdencStatistics;
    }

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetVdencPipeBufAddrParams(pipeBufAddrParams);
    }
#endif

    pipeBufAddrParams.presVdencTileRowStoreBuffer = &m_vdencTileRowStoreBuffer;
    pipeBufAddrParams.presVdencCumulativeCuCountStreamoutSurface = &m_vdencCumulativeCuCountStreamoutSurface;
    pipeBufAddrParams.isLowDelayB = m_lowDelay;
}

MOS_STATUS CodechalVdencHevcStateG12::SetKernelParams(
    EncOperation     operation,
    MHW_KERNEL_PARAM *kernelParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelParams);

    auto curbeAlignment = m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment();

    kernelParams->iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelParams->iIdCount = 1;

    switch (operation)
    {
    case VDENC_ME_P:
    case VDENC_ME_B:
    case VDENC_STREAMIN:
    case VDENC_STREAMIN_HEVC:
    case VDENC_STREAMIN_HEVC_RAB:
        kernelParams->iBTCount = CODECHAL_VDENC_HME_END_G12 - CODECHAL_VDENC_HME_BEGIN_G12;
        kernelParams->iCurbeLength = MOS_ALIGN_CEIL(sizeof(MEDIA_OBJECT_HEVC_VP9_VDENC_ME_CURBE_G12), (size_t)curbeAlignment);
        kernelParams->iBlockWidth = 32;
        kernelParams->iBlockHeight = 32;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetBindingTable(
    EncOperation operation,
    PCODECHAL_ENCODE_BINDING_TABLE_GENERIC bindingTable)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(bindingTable);

    MOS_ZeroMemory(bindingTable, sizeof(*bindingTable));

    switch (operation)
    {
    case VDENC_ME_P:
    case VDENC_ME_B:
    case VDENC_STREAMIN:
    case VDENC_STREAMIN_HEVC:
    case VDENC_STREAMIN_HEVC_RAB:
        bindingTable->dwNumBindingTableEntries = CODECHAL_VDENC_HME_END_G12 - CODECHAL_VDENC_HME_BEGIN_G12;
        bindingTable->dwBindingTableStartOffset = CODECHAL_VDENC_HME_BEGIN_G12;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = 0; i < bindingTable->dwNumBindingTableEntries; i++)
    {
        bindingTable->dwBindingTableEntries[i] = i;
    }
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::EncodeMeKernel(HmeLevel hmeLevel)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    PMHW_KERNEL_STATE kernelState = nullptr;
    if(hmeLevel == HME_LEVEL_4x)
    {
        kernelState = m_lowDelay ? &m_vdencStreaminKernelState : &m_vdencStreaminKernelStateRAB;
    }
    else
    {
        kernelState = m_lowDelay ? &m_vdencMeKernelState : &m_vdencMeKernelStateRAB;
    }
    auto encFunctionType = (hmeLevel == HME_LEVEL_32x) ? CODECHAL_MEDIA_STATE_32X_ME :
        (hmeLevel == HME_LEVEL_16x) ? CODECHAL_MEDIA_STATE_16X_ME : CODECHAL_MEDIA_STATE_4X_ME;

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Set up the DSH/SSH as normal
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

    //Setup curbe for StreamIn Kernel
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetMeCurbe(hmeLevel));

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
        kernelState));
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(hmeLevel, &cmdBuffer));

    // Dump SSH for ME kernel
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState)));

    uint32_t scalingFactor = (hmeLevel == HME_LEVEL_32x) ? SCALE_FACTOR_32x :
        (hmeLevel == HME_LEVEL_16x) ? SCALE_FACTOR_16x : SCALE_FACTOR_4x;

    uint32_t resolutionX = CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / scalingFactor);
    uint32_t resolutionY = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scalingFactor);

    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode = m_walkerMode;
    walkerCodecParams.dwResolutionX = resolutionX;
    walkerCodecParams.dwResolutionY = resolutionY;
    walkerCodecParams.bNoDependency = true;
    walkerCodecParams.bMbaff = false;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
    walkerCodecParams.ucGroupId = m_groupId;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase);

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    MHW_MI_STORE_DATA_PARAMS    storeDataParams;

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetMeCurbe(HmeLevel hmeLevel)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_VDENC_HEVC_ME_CURBE_G12 curbe;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        &curbe,
        sizeof(CODECHAL_VDENC_HEVC_ME_CURBE_G12),
        ME_CURBE_INIT_G12,
        sizeof(CODECHAL_VDENC_HEVC_ME_CURBE_G12)));

    PMHW_KERNEL_STATE kernelState = nullptr;
    if(hmeLevel == HME_LEVEL_4x)
    {
        kernelState = m_lowDelay ? &m_vdencStreaminKernelState : &m_vdencStreaminKernelStateRAB;
    }
    else
    {
        kernelState = m_lowDelay ? &m_vdencMeKernelState : &m_vdencMeKernelStateRAB;
    }
    
    bool useMvFromPrevStep;
    bool writeDistortions;
    uint32_t scaleFactor;
    uint32_t  mvShiftFactor = 0;
    uint32_t  prevMvReadPosFactor = 0;

    switch (hmeLevel)
    {
    case HME_LEVEL_32x:
        useMvFromPrevStep = false;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_32x;
        mvShiftFactor = 1;
        prevMvReadPosFactor = 0;
        break;
    case HME_LEVEL_16x:
        useMvFromPrevStep = (m_b32XMeEnabled) ? true : false;
        writeDistortions = false;
        scaleFactor = SCALE_FACTOR_16x;
        mvShiftFactor = 2;
        prevMvReadPosFactor = 1;
        break;
    case HME_LEVEL_4x:
        useMvFromPrevStep = (m_b16XMeEnabled) ? true : false;
        writeDistortions = true;
        scaleFactor = SCALE_FACTOR_4x;
        mvShiftFactor = 2;
        prevMvReadPosFactor = 0;
        break;
    default:
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
        break;
    }

    curbe.DW3.SubPelMode = 3;
    curbe.DW4.PictureHeightMinus1 = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight / scaleFactor) - 1;
    curbe.DW4.PictureWidth = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth / scaleFactor);
    curbe.DW5.QpPrimeY            = m_hevcPicParams->QpY + m_hevcSliceParams->slice_qp_delta;
    curbe.DW6.WriteDistortions = writeDistortions;
    curbe.DW6.UseMvFromPrevStep = useMvFromPrevStep;
    curbe.DW6.SuperCombineDist = 5;//SuperCombineDist_Generic[pHevcSeqParams->TargetUsage]; Harded coded in KCM
    curbe.DW6.MaxVmvR = 511 * 4;
    curbe.DW15.MvShiftFactor = mvShiftFactor;
    curbe.DW15.PrevMvReadPosFactor = prevMvReadPosFactor;

    if (m_pictureCodingType == B_TYPE)
    {
        // This field is irrelevant since we are not using the bi-direct search.
        curbe.DW1.BiWeight = m_bframeMeBidirectionalWeight;
        curbe.DW13.NumRefIdxL1MinusOne = m_hevcSliceParams->num_ref_idx_l1_active_minus1;
    }

    if (m_pictureCodingType == P_TYPE || m_pictureCodingType == B_TYPE)
    {
        curbe.DW13.NumRefIdxL0MinusOne = m_hevcSliceParams->num_ref_idx_l0_active_minus1;
    }

    if (hmeLevel == HME_LEVEL_4x)
    {
        curbe.DW30.ActualMBHeight = m_frameHeight;
        curbe.DW30.ActualMBWidth = m_frameWidth;
    }
    else
    {
        curbe.DW30.ActualMBHeight = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameFieldHeight);
        curbe.DW30.ActualMBWidth = CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameWidth);
    }

    curbe.DW13.RefStreaminCost = 0;
    // This flag is to indicate the ROI source type instead of indicating ROI is enabled or not
    curbe.DW13.ROIEnable = 0;

    uint8_t meMethod = (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[m_hevcSeqParams->TargetUsage] : m_meMethodGeneric[m_hevcSeqParams->TargetUsage];
    uint8_t tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(&(curbe.SPDelta), 14 * sizeof(uint32_t),
        m_encodeSearchPath[tableIdx][meMethod], 14 * sizeof(uint32_t)));

    if (hmeLevel == HME_LEVEL_4x)
    {
        //StreamIn CURBE
        curbe.DW6.LCUSize            = 1;//Only LCU64 supported by the VDEnc HW
        // Kernel should use driver-prepared stream-in surface during ROI/ Dirty-Rect
        curbe.DW6.InputStreamInEn    = (m_hevcPicParams->NumROI || (m_hevcPicParams->NumDirtyRects > 0 && (B_TYPE == m_hevcPicParams->CodingType)));
        curbe.DW31.MaxCuSize         = 3;
        curbe.DW31.MaxTuSize         = 3;
        switch (m_hevcSeqParams->TargetUsage)
        {
        case 1:
        case 4:
            curbe.DW36.NumMergeCandCu64x64    = 4;
            curbe.DW36.NumMergeCandCu32x32    = 3;
            curbe.DW36.NumMergeCandCu16x16    = 2;
            curbe.DW36.NumMergeCandCu8x8      = 1;
            curbe.DW31.NumImePredictors       = m_imgStateImePredictors;
            break;
        case 7:
            curbe.DW36.NumMergeCandCu64x64    = 2;
            curbe.DW36.NumMergeCandCu32x32    = 2;
            curbe.DW36.NumMergeCandCu16x16    = 2;
            curbe.DW36.NumMergeCandCu8x8      = 0;
            curbe.DW31.NumImePredictors       = 4;
            break;
        }
    }

    curbe.DW40._4xMeMvOutputDataSurfIndex       = CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G12;
    curbe.DW41._16xOr32xMeMvInputDataSurfIndex = (hmeLevel == HME_LEVEL_32x) ? CODECHAL_VDENC_32xME_MV_DATA_SURFACE_CM_G12 : CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G12;
    curbe.DW42._4xMeOutputDistSurfIndex         = CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G12;
    curbe.DW43._4xMeOutputBrcDistSurfIndex      = CODECHAL_VDENC_HME_BRC_DISTORTION_CM_G12;
    curbe.DW44.VMEFwdInterPredictionSurfIndex   = CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G12;
    curbe.DW45.VMEBwdInterPredictionSurfIndex   = CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G12;
    curbe.DW46.VDEncStreamInOutputSurfIndex     = CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G12;
    curbe.DW47.VDEncStreamInInputSurfIndex      = CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G12;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &curbe,
        kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SendMeSurfaces(HmeLevel hmeLevel, PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MOS_SURFACE *meMvDataBuffer;
    uint32_t downscaledWidthInMb;
    uint32_t downscaledHeightInMb;

    if (hmeLevel == HME_LEVEL_32x)
    {
        meMvDataBuffer = &m_s32XMeMvDataBuffer;
        downscaledWidthInMb = m_downscaledWidthInMb32x;
        downscaledHeightInMb = m_downscaledHeightInMb32x;
    }
    else if (hmeLevel == HME_LEVEL_16x)
    {
        meMvDataBuffer = &m_s16XMeMvDataBuffer;
        downscaledWidthInMb = m_downscaledWidthInMb16x;
        downscaledHeightInMb = m_downscaledHeightInMb16x;
    }
    else
    {
        meMvDataBuffer = &m_s4XMeMvDataBuffer;
        downscaledWidthInMb = m_downscaledWidthInMb4x;
        downscaledHeightInMb = m_downscaledHeightInMb4x;
    }

    auto width = MOS_ALIGN_CEIL(downscaledWidthInMb * 32, 64);
    auto height = downscaledHeightInMb * 4 * 10;
    // Force the values
    meMvDataBuffer->dwWidth = width;
    meMvDataBuffer->dwHeight = height;
    meMvDataBuffer->dwPitch = width;

    PMHW_KERNEL_STATE kernelState = nullptr;
    if(hmeLevel == HME_LEVEL_4x)
    {
        kernelState = m_lowDelay ? &m_vdencStreaminKernelState : &m_vdencStreaminKernelStateRAB;
    }
    else
    {
        kernelState = m_lowDelay ? &m_vdencMeKernelState : &m_vdencMeKernelStateRAB;
    }
    auto bindingTable = (hmeLevel == HME_LEVEL_4x) ?
        &m_vdencStreaminKernelBindingTable : &m_vdencMeKernelBindingTable;
    uint32_t meMvBottomFieldOffset = 0;

    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = meMvDataBuffer;
    surfaceCodecParams.dwOffset = meMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_MV_DATA_SURFACE_CM_G12];
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (hmeLevel == HME_LEVEL_16x && m_b32XMeEnabled)
    {
        // Pass 32x MV to 16x ME operation
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_s32XMeMvDataBuffer;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_32xME_MV_DATA_SURFACE_CM_G12];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else if (!(hmeLevel == HME_LEVEL_32x) && m_b16XMeEnabled)
    {
        // Pass 16x MV to 4x ME operation
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_s16XMeMvDataBuffer;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_16xME_MV_DATA_SURFACE_CM_G12];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_s4XMeDistortionBuffer;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_DISTORTION_SURFACE_CM_G12];
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    PMOS_SURFACE currScaledSurface = (hmeLevel == HME_LEVEL_4x) ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) :
        ((hmeLevel == HME_LEVEL_16x) ? m_trackedBuf->Get16xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_trackedBuf->Get32xDsSurface(CODEC_CURR_TRACKED_BUFFER));
    MOS_SURFACE refScaledSurface = *currScaledSurface;
    bool currFieldPicture = CodecHal_PictureIsField(m_currOriginalPic) ? true : false;
    bool currBottomField = CodecHal_PictureIsBottomField(m_currOriginalPic) ? true : false;

    uint8_t currVDirection = (!currFieldPicture) ? CODECHAL_VDIRECTION_FRAME :
        ((currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
    uint32_t currScaledBottomFieldOffset = (hmeLevel == HME_LEVEL_4x) ?
        (uint32_t)m_scaledBottomFieldOffset : ((hmeLevel == HME_LEVEL_16x) ? (uint32_t)m_scaled16xBottomFieldOffset : (uint32_t)m_scaled32xBottomFieldOffset);

    // Setup references 1...n
    // LIST 0 references
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l0_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_0][refIdx];

        if (!CodecHal_PictureIsInvalid(refPic))
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
                surfaceCodecParams.bUseAdvState = true;
                surfaceCodecParams.psSurface = currScaledSurface;
                surfaceCodecParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_CURR_FOR_FWD_REF_CM_G12];
                surfaceCodecParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            bool refFieldPicture = CodecHal_PictureIsField(refPic) ? true : false;
            bool refBottomField = CodecHal_PictureIsBottomField(refPic) ? true : false;
            uint8_t refPicIdx       = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx       = m_refList[refPicIdx]->ucScalingIdx;
            if (hmeLevel == HME_LEVEL_4x)
            {
                refScaledSurface.OsResource = m_trackedBuf->Get4xDsSurface(scaledIdx)->OsResource;
            }
            else if (hmeLevel == HME_LEVEL_16x)
            {
                refScaledSurface.OsResource = m_trackedBuf->Get16xDsSurface(scaledIdx)->OsResource;
            }
            else
            {
                refScaledSurface.OsResource = m_trackedBuf->Get32xDsSurface(scaledIdx)->OsResource;
            }
            uint32_t refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;

            // L0 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &refScaledSurface;
            surfaceCodecParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_FWD_REF_IDX0_CM_G12 + (refIdx * 2)];
            surfaceCodecParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_RESERVED1_CM_G12 + (refIdx * 2)];
            surfaceCodecParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    //List1
    for (uint8_t refIdx = 0; refIdx <= m_hevcSliceParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        CODEC_PICTURE refPic = m_hevcSliceParams->RefPicList[LIST_1][refIdx];

        if (!CodecHal_PictureIsInvalid(refPic))
        {
            if (refIdx == 0)
            {
                // Current Picture Y - VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
                surfaceCodecParams.bUseAdvState = true;
                surfaceCodecParams.psSurface = currScaledSurface;
                surfaceCodecParams.dwOffset = currBottomField ? currScaledBottomFieldOffset : 0;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_CURR_FOR_BWD_REF_CM_G12];
                surfaceCodecParams.ucVDirection = currVDirection;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }

            bool refFieldPicture = CodecHal_PictureIsField(refPic) ? 1 : 0;
            bool refBottomField = CodecHal_PictureIsBottomField(refPic) ? 1 : 0;
            auto    refPicIdx       = m_picIdx[refPic.FrameIdx].ucPicIdx;
            uint8_t scaledIdx       = m_refList[refPicIdx]->ucScalingIdx;

            if (hmeLevel == HME_LEVEL_4x)
            {
                refScaledSurface.OsResource = m_trackedBuf->Get4xDsSurface(scaledIdx)->OsResource;
            }
            else if (hmeLevel == HME_LEVEL_16x)
            {
                refScaledSurface.OsResource = m_trackedBuf->Get16xDsSurface(scaledIdx)->OsResource;
            }
            else
            {
                refScaledSurface.OsResource = m_trackedBuf->Get32xDsSurface(scaledIdx)->OsResource;
            }
            uint32_t refScaledBottomFieldOffset = refBottomField ? currScaledBottomFieldOffset : 0;

            // L1 Reference Picture Y - VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &refScaledSurface;
            surfaceCodecParams.dwOffset = refBottomField ? refScaledBottomFieldOffset : 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_BWD_REF_IDX0_CM_G12 + (refIdx * 2)];
            surfaceCodecParams.ucVDirection = !currFieldPicture ? CODECHAL_VDIRECTION_FRAME :
                ((refBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_RESERVED9_CM_G12 + (refIdx * 2)];
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if (hmeLevel == HME_LEVEL_4x)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(&m_resVdencStreamInBuffer[m_currRecycledBufIdx]);

        auto streamingSize = (MOS_ALIGN_CEIL(m_frameWidth, 64) / 32) * (MOS_ALIGN_CEIL(m_frameHeight, 64) / 32) * CODECHAL_CACHELINE_SIZE;

        // Send driver-prepared stream-in surface as input during ROI/ Dirty-Rect
        if (m_hevcPicParams->NumROI || (m_hevcPicParams->NumDirtyRects > 0 && (B_TYPE == m_hevcPicParams->CodingType)))
        {
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
            surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(streamingSize);
            surfaceCodecParams.bIs2DSurface = false;
            surfaceCodecParams.presBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Value;
            surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_VDENC_STREAMIN_INPUT_CM_G12];
            surfaceCodecParams.bIsWritable = true;
            surfaceCodecParams.bRenderTarget = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
        else    // Clear stream-in surface otherwise
        {
            MOS_LOCK_PARAMS lockFlags;
            MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
            lockFlags.WriteOnly = true;

            auto data = m_osInterface->pfnLockResource(
                m_osInterface,
                &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                &lockFlags);

            CODECHAL_ENCODE_CHK_NULL_RETURN(data);

            MOS_ZeroMemory(
                data,
                streamingSize);

            m_osInterface->pfnUnlockResource(
                m_osInterface,
                &m_resVdencStreamInBuffer[m_currRecycledBufIdx]);
        }

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(streamingSize);
        surfaceCodecParams.bIs2DSurface = false;
        surfaceCodecParams.presBuffer = &m_resVdencStreamInBuffer[m_currRecycledBufIdx];
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Value;
        surfaceCodecParams.dwBindingTableOffset = bindingTable->dwBindingTableEntries[CODECHAL_VDENC_HME_VDENC_STREAMIN_OUTPUT_CM_G12];
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS
CodechalVdencHevcStateG12::GetKernelHeaderAndSize(
    void                           *binary,
    EncOperation                   operation,
    uint32_t                       krnStateIdx,
    void                           *krnHeader,
    uint32_t                       *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommonKernelHeaderAndSizeG12(binary, operation, krnStateIdx, krnHeader, krnSize));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::AddVdencWalkerStateCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 vdencWalkerStateParams;
    vdencWalkerStateParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    vdencWalkerStateParams.pHevcEncSeqParams = params->pEncodeHevcSeqParams;
    vdencWalkerStateParams.pHevcEncPicParams = params->pEncodeHevcPicParams;
    vdencWalkerStateParams.pEncodeHevcSliceParams = params->pEncodeHevcSliceParams;
    vdencWalkerStateParams.pTileCodingParams = static_cast<PMHW_VDBOX_HEVC_SLICE_STATE_G12>(params)->pTileCodingParams;
    vdencWalkerStateParams.dwTileId = static_cast<PMHW_VDBOX_HEVC_SLICE_STATE_G12>(params)->dwTileID;
    switch (static_cast<PMHW_VDBOX_HEVC_SLICE_STATE_G12>(params)->dwNumPipe)
    {
    case 0:
    case 1:
        vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_SINGLE_PIPE;
        break;
    case 2:
        vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_TWO_PIPE;
        break;
    case 4:
        vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_FOUR_PIPE;
        break;
    default:
        vdencWalkerStateParams.dwNumberOfPipes = VDENC_PIPE_INVALID;
        CODECHAL_ENCODE_ASSERT(false);
        break;
    }

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        m_rsvdState->SetVdencWalkerStateParams(vdencWalkerStateParams);
    }
#endif

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencWalkerStateCmd(cmdBuffer, &vdencWalkerStateParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::GetSystemPipeNumberCommon()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENCODE_DISABLE_SCALABILITY_G12,
        &userFeatureData);

    bool disableScalability = false;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = userFeatureData.i32Data ? true : false;
    }

    MEDIA_SYSTEM_INFO *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(gtSystemInfo);

    if (gtSystemInfo && disableScalability == false)
    {
        // Both VE mode and media solo mode should be able to get the VDBOX number via the same interface
        m_numVdbox = (uint8_t)(gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled);
    }
    else
    {
        m_numVdbox = 1;
    }

    CODECHAL_ENCODE_VERBOSEMESSAGE("System VDBOX number = %d.", m_numVdbox);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::Initialize(CodechalSetting * settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    // Tile Replay Enable should be passed from DDI, will change later when DDI is ready
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_TILEREPLAY_ENABLE_ID_G12,
        &userFeatureData);
    m_enableTileReplay = userFeatureData.i32Data ? true : false;

    m_skipFrameBasedHWCounterRead = m_enableTileReplay;

    // RGB Encoding Enable should be passed from DDI, will change later when DDI is ready
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_RGB_ENCODING_ENABLE_ID_G12,
        &userFeatureData);
    m_RGBEncodingEnable = userFeatureData.i32Data ? true : false;

    // Capture mode with display Enable should be passed from DDI, will change later
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_CAPTURE_MODE_ENABLE_ID_G12,
        &userFeatureData);
    m_CaptureModeEnable = userFeatureData.i32Data ? true : false;

    // common initilization
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::Initialize(settings));

    MEDIA_FEATURE_TABLE *skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    if (MEDIA_IS_SKU(skuTable, FtrSimulationMode) && (m_enableTileReplay == true))
    {
        m_frameTrackingEnabled = false;
    }

    // To do: current size assumes 8Kx8K max resolution. Needs to be increased based on Gen12, along with m_maxNumNativeROI.
    m_deltaQpRoiBufferSize = m_deltaQpBufferSize;
    m_brcRoiBufferSize = m_roiStreamInBufferSize;
    m_maxTileNumber = CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, CODECHAL_HEVC_MIN_TILE_SIZE) *
        CODECHAL_GET_HEIGHT_IN_BLOCKS(m_frameHeight, CODECHAL_HEVC_MIN_TILE_SIZE);

    // we need additional buffer for (1) 1 CL for size info at the beginning of each tile column (max of 4 vdbox in scalability mode)
    // (2) CL alignment at end of every tile column
    // as a result, increase the height by 1 for allocation purposes
    m_numLcu = MOS_ROUNDUP_DIVIDE(m_frameWidth, MAX_LCU_SIZE) * (MOS_ROUNDUP_DIVIDE(m_frameHeight, MAX_LCU_SIZE) + 1);
    m_mbCodeSize = MOS_ALIGN_CEIL(2 * sizeof(uint32_t) * (m_numLcu * 5 + m_numLcu * 64 * 8), CODECHAL_PAGE_SIZE);
    m_mbCodeSize += m_mvOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetSystemPipeNumberCommon());

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_scalabilityState = (PCODECHAL_ENCODE_SCALABILITY_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SCALABILITY_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_scalabilityState);
        //scalability initialize
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_InitializeState(m_scalabilityState, m_hwInterface));
    }

    // Caculate the size for 3nd level batch buffer
    // mhw_vdbox_hcp_g12_X::HCP_PIC_STATE_CMD::byteSize
    // As this buffer is going to passed to HuC to generate the command, must be page aligned
    // To add the HW interface get the buffer size later

    m_thirdLBSize = MOS_ALIGN_CEIL(1024, CODECHAL_PAGE_SIZE);

    // Caculate the batch buffer size for each tile
    // To add the MHW interface later, can be fine tuned
    m_tileLevelBatchSize = m_hwInterface->m_vdenc2ndLevelBatchBufferSize;

    // Caculate the size for MV temporal buffer
    uint32_t mvt_size = MOS_ALIGN_CEIL(((m_frameWidth + 63) >> 6)*((m_frameHeight + 15) >> 4), 2) * CODECHAL_CACHELINE_SIZE;
    uint32_t mvtb_size = MOS_ALIGN_CEIL(((m_frameWidth + 31) >> 5)*((m_frameHeight + 31) >> 5), 2) * CODECHAL_CACHELINE_SIZE;
    m_sizeOfMvTemporalBuffer = MOS_MAX(mvt_size, mvtb_size);

    m_sizeOfHcpPakFrameStats = 9 * CODECHAL_CACHELINE_SIZE;

#ifdef _ENCODE_VDENC_RESERVED
    InitReserveState(settings);
#endif
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_HW_STITCH_G12,
        &userFeatureData);
    m_enableTileStitchByHW = userFeatureData.i32Data ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_HW_SEMAPHORE_G12,
        &userFeatureData);
    m_enableHWSemaphore = userFeatureData.i32Data ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VDBOX_HW_SEMAPHORE_G12,
        &userFeatureData);
    m_enableVdBoxHWSemaphore = userFeatureData.i32Data ? true : false;

    // ACQP is now supported on Gen12 for TU1 / TU4
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_ACQP_ENABLE_ID,
        &userFeatureData);
    m_hevcVdencAcqpEnabled = userFeatureData.i32Data ? true : false;

    m_numDelay = 15;

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_ENABLE_VE_DEBUG_OVERRIDE_G12,
        &userFeatureData);
    m_kmdVeOveride.Value = (uint64_t)userFeatureData.i64Data;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_HEVC_VDENC_FORCE_SCALABILITY_ID_G12,
        &userFeatureData);
    m_forceScalability = userFeatureData.i32Data ? true : false;
#endif

    return eStatus;
}

CodechalVdencHevcStateG12::CodechalVdencHevcStateG12(
    CodechalHwInterface* hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo)
    :CodechalVdencHevcState(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_useCommonKernel = true;
    pfnGetKernelHeaderAndSize = GetKernelHeaderAndSize;
    m_useHwScoreboard = false;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#endif
    m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
    m_scalabilityState = nullptr;

    MOS_ZeroMemory(&m_resPakcuLevelStreamoutData, sizeof(m_resPakcuLevelStreamoutData));
    MOS_ZeroMemory(&m_resPakSliceLevelStreamoutData, sizeof(m_resPakSliceLevelStreamoutData));
    MOS_ZeroMemory(m_resTileBasedStatisticsBuffer, sizeof(m_resTileBasedStatisticsBuffer));
    MOS_ZeroMemory(&m_resHuCPakAggregatedFrameStatsBuffer, sizeof(m_resHuCPakAggregatedFrameStatsBuffer));
    MOS_ZeroMemory(m_tileRecordBuffer, sizeof(m_tileRecordBuffer));
    MOS_ZeroMemory(&m_kmdVeOveride, sizeof(m_kmdVeOveride));
    MOS_ZeroMemory(&m_resHcpScalabilitySyncBuffer, sizeof(m_resHcpScalabilitySyncBuffer));

    MOS_ZeroMemory(m_veBatchBuffer, sizeof(m_veBatchBuffer));
    MOS_ZeroMemory(&m_realCmdBuffer, sizeof(m_realCmdBuffer));
    MOS_ZeroMemory(&m_resBrcSemaphoreMem, sizeof(m_resBrcSemaphoreMem));
    MOS_ZeroMemory(&m_resBrcPakSemaphoreMem, sizeof(m_resBrcPakSemaphoreMem));
    MOS_ZeroMemory(m_resVdBoxSemaphoreMem, sizeof(m_resVdBoxSemaphoreMem));
    MOS_ZeroMemory(&m_resPipeStartSemaMem, sizeof(m_resPipeStartSemaMem));

    MOS_ZeroMemory(&m_vdencTileRowStoreBuffer, sizeof(m_vdencTileRowStoreBuffer));
    MOS_ZeroMemory(&m_thirdLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
    MOS_ZeroMemory(&m_vdencSAORowStoreBuffer, sizeof(m_vdencSAORowStoreBuffer));

    for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
    {
        MOS_ZeroMemory(&m_tileLevelBatchBuffer[i], sizeof(PMHW_BATCH_BUFFER));
        MOS_ZeroMemory(&m_TileRowBRCBatchBuffer[i], sizeof(PMHW_BATCH_BUFFER));
    }

    for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
    {
        for (auto i = 0; i < CODECHAL_VDENC_BRC_NUM_OF_PASSES; i++)
        {
            MOS_ZeroMemory(&m_resHucPakStitchDmemBuffer[k][i], sizeof(m_resHucPakStitchDmemBuffer[k][i]));
        }
    }

    MOS_ZeroMemory(&m_resBrcDataBuffer, sizeof(m_resBrcDataBuffer));
    MOS_ZeroMemory(&m_resTileRowBRCsyncSemaphore, sizeof(m_resTileRowBRCsyncSemaphore));
    
    m_vdencBrcInitDmemBufferSize = sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_INIT_DMEM_G12);
    m_vdencBrcUpdateDmemBufferSize = sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_UPDATE_DMEM_G12);
    m_vdencBrcConstDataBufferSize = sizeof(CODECHAL_VDENC_HEVC_HUC_BRC_CONSTANT_DATA_G12);
    m_maxNumSlicesSupported        = CODECHAL_VDENC_HEVC_MAX_SLICE_NUM;

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = CODECHAL_ENCODE_HEVC_NUM_SYNC_TAGS;
    m_hwInterface->GetStateHeapSettings()->dwDshSize = CODECHAL_INIT_DSH_SIZE_HEVC_ENC;
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_kernelBase = (uint8_t*)IGCODECKRN_G12;
#endif

    MOS_STATUS eStatus = CodecHalGetKernelBinaryAndSize(
        m_kernelBase,
        m_kuidCommon,
        &m_kernelBinary,
        &m_combinedKernelSize);
    CODECHAL_ENCODE_ASSERT(eStatus == MOS_STATUS_SUCCESS);

    m_hwInterface->GetStateHeapSettings()->dwIshSize +=
        MOS_ALIGN_CEIL(m_combinedKernelSize, (1 << MHW_KERNEL_OFFSET_SHIFT));

    m_hwInterface->m_hucCommandBufferSize += 64;

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG12, this));
        CreateHevcPar();
    )
}

MOS_STATUS CodechalVdencHevcStateG12::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    
    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CodechalEncoderState::SetGpuCtxCreatOption();
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeScalability_ConstructParmsForGpuCtxCreation(
            m_scalabilityState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetRegionsHuCPakIntegrate(
    PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS  virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = GetCurrentPass();

    if(m_enableTileStitchByHW)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());
    }

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileParams);

    MOS_ZeroMemory(virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));

    // Add Virtual addr
    virtualAddrParams->regionParams[0].presRegion = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 0 - Tile based input statistics from PAK/ VDEnc
    virtualAddrParams->regionParams[0].dwOffset   = 0;
    virtualAddrParams->regionParams[1].presRegion = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 1 - HuC Frame statistics output
    virtualAddrParams->regionParams[1].isWritable = true;
    virtualAddrParams->regionParams[4].presRegion = &m_resBitstreamBuffer;                         // Region 4 - Last Tile bitstream
    virtualAddrParams->regionParams[4].dwOffset   = MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    virtualAddrParams->regionParams[5].presRegion = &m_resBitstreamBuffer;                         // Region 5 - HuC modifies the last tile bitstream before stitch command
    virtualAddrParams->regionParams[5].dwOffset   = MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    virtualAddrParams->regionParams[5].isWritable = true;
    virtualAddrParams->regionParams[6].presRegion = &m_vdencBrcHistoryBuffer;                 // Region 6 History Buffer (Input/Output)
    virtualAddrParams->regionParams[6].isWritable = true;
    virtualAddrParams->regionParams[7].presRegion = &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource;                // Region 7 - HCP PIC state command
    virtualAddrParams->regionParams[9].presRegion = &m_resBrcDataBuffer;                           // Region 9 HuC outputs BRC data
    virtualAddrParams->regionParams[9].isWritable = true;
    if (m_enableTileStitchByHW)
    {
        virtualAddrParams->regionParams[8].presRegion = &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass];  // Region 8 - data buffer read by HUC for stitching cmd generation
        virtualAddrParams->regionParams[10].presRegion = &m_HucStitchCmdBatchBuffer.OsResource;  // Region 10 - SLB for stitching cmd output from Huc
        virtualAddrParams->regionParams[10].isWritable = true;
    }
    virtualAddrParams->regionParams[15].presRegion = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;          // Region 15 [In/Out] - Tile Record Buffer
    virtualAddrParams->regionParams[15].dwOffset   = 0;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ConfigStitchDataBuffer()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 ||
        (currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES && m_brcEnabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = 1;

    HucCommandDataVdencG12 *hucStitchDataBuf = (HucCommandDataVdencG12 *)m_osInterface->pfnLockResource(m_osInterface, &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass], &lockFlagsWriteOnly);

    MOS_ZeroMemory(hucStitchDataBuf, sizeof(HucCommandDataVdencG12));
    hucStitchDataBuf->TotalCommands          = 1;
    hucStitchDataBuf->InputCOM[0].SizeOfData = 0xF;

    HucInputCmdVdencG12 hucInputCmd;
    MOS_ZeroMemory(&hucInputCmd, sizeof(HucInputCmdVdencG12));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);
    hucInputCmd.SelectionForIndData = m_osInterface->osCpInterface->IsCpEnabled() ? 4 : 0;
    hucInputCmd.CmdMode             = HUC_CMD_LIST_MODE;
    hucInputCmd.LengthOfTable       = (uint8_t)(m_numTiles);
    hucInputCmd.CopySize            = m_hwInterface->m_tileRecordSize;

    PMOS_RESOURCE presSrc = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
        m_osInterface,
        presSrc,
        false,
        false));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
        m_osInterface,
        &m_resBitstreamBuffer,
        true,
        true));

    uint64_t srcAddr          = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, presSrc);
    uint64_t destAddr         = m_osInterface->pfnGetResourceGfxAddress(m_osInterface, &m_resBitstreamBuffer);
    hucInputCmd.SrcAddrBottom = (uint32_t)(srcAddr & 0x00000000FFFFFFFF);
    hucInputCmd.SrcAddrTop    = (uint32_t)((srcAddr & 0xFFFFFFFF00000000) >> 32);

    hucInputCmd.DestAddrBottom = (uint32_t)(destAddr & 0x00000000FFFFFFFF);
    hucInputCmd.DestAddrTop    = (uint32_t)((destAddr & 0xFFFFFFFF00000000) >> 32);

    MOS_SecureMemcpy(hucStitchDataBuf->InputCOM[0].data, sizeof(HucInputCmdVdencG12), &hucInputCmd, sizeof(HucInputCmdVdencG12));

    m_osInterface->pfnUnlockResource(m_osInterface, &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass]);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetRegionsHuCPakIntegrateStitch(
    PMHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    int32_t currentPass = GetCurrentPass();

    MOS_ZeroMemory(virtualAddrParams, sizeof(MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS));

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileParams);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConfigStitchDataBuffer());

    // Add Virtual addr
    virtualAddrParams->regionParams[0].presRegion = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;  // Region 0 - Tile based input statistics from PAK/ VDEnc
    virtualAddrParams->regionParams[0].dwOffset   = 0;
    virtualAddrParams->regionParams[1].presRegion = &m_resHuCPakAggregatedFrameStatsBuffer.sResource;  // Region 1 - HuC Frame statistics output
    virtualAddrParams->regionParams[1].isWritable = true;
    virtualAddrParams->regionParams[4].presRegion = &m_resBitstreamBuffer;                         // Region 4 - Last Tile bitstream
    virtualAddrParams->regionParams[4].dwOffset   = MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    virtualAddrParams->regionParams[5].presRegion = &m_resBitstreamBuffer;                         // Region 5 - HuC modifies the last tile bitstream before stitch command
    virtualAddrParams->regionParams[5].dwOffset   = MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
    virtualAddrParams->regionParams[5].isWritable = true;
    virtualAddrParams->regionParams[6].presRegion = &m_vdencBrcHistoryBuffer;  // Region 6  History Buffer (Input/Output)
    virtualAddrParams->regionParams[6].isWritable = true;
    virtualAddrParams->regionParams[7].presRegion = &m_thirdLevelBatchBuffer.OsResource;  //&m_resHucPakStitchReadBatchBuffer;             // Region 7 - HCP PIC state command
    virtualAddrParams->regionParams[8].presRegion  = &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass];  // Region 8 - data buffer read by HUC for stitching cmd generation
    virtualAddrParams->regionParams[9].presRegion  = &m_resBrcDataBuffer;  // Region 9  HuC outputs BRC data
    virtualAddrParams->regionParams[9].isWritable  = true;
    virtualAddrParams->regionParams[10].presRegion = &m_HucStitchCmdBatchBuffer.OsResource;                         // Region 10 - SLB for stitching cmd output from Huc
    virtualAddrParams->regionParams[10].isWritable = true;
    virtualAddrParams->regionParams[15].presRegion = &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource;  // Region 15 [In/Out] - Tile Record Buffer
    virtualAddrParams->regionParams[15].dwOffset   = 0;
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetDmemHuCPakIntegrateStitch(
    PMHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    int32_t currentPass = GetCurrentPass();

    HucPakStitchDmemVdencG12 *hucPakStitchDmem = (HucPakStitchDmemVdencG12 *)m_osInterface->pfnLockResource(
        m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]), &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucPakStitchDmem);

    MOS_ZeroMemory(hucPakStitchDmem, sizeof(HucPakStitchDmemVdencG12));

    // reset all the offsets to -1
    uint32_t TotalOffsetSize = sizeof(hucPakStitchDmem->TileSizeRecord_offset) +
                               sizeof(hucPakStitchDmem->VDENCSTAT_offset) +
                               sizeof(hucPakStitchDmem->HEVC_PAKSTAT_offset) +
                               sizeof(hucPakStitchDmem->HEVC_Streamout_offset) +
                               sizeof(hucPakStitchDmem->VP9_PAK_STAT_offset) +
                               sizeof(hucPakStitchDmem->Vp9CounterBuffer_offset);
    MOS_FillMemory(hucPakStitchDmem, TotalOffsetSize, 0xFF);

    uint16_t numTileRows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    uint16_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    CODECHAL_ENCODE_ASSERT(numTileColumns > 0 && numTileColumns % 2 == 0);                       //numTileColumns is nonzero and even number; 2 or 4
    CODECHAL_ENCODE_ASSERT(m_numPipe > 0 && m_numPipe % 2 == 0 && numTileColumns <= m_numPipe);  //ucNumPipe is nonzero and even number; 2 or 4
    uint16_t numTiles        = numTileRows * numTileColumns;
    uint16_t numTilesPerPipe = m_numTiles / m_numPipe;
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileParams);

    hucPakStitchDmem->PicWidthInPixel          = (uint16_t)m_frameWidth;
    hucPakStitchDmem->PicHeightInPixel         = (uint16_t)m_frameHeight;
    hucPakStitchDmem->TotalNumberOfPAKs        = 0;
    hucPakStitchDmem->Codec                    = 2;  //HEVC DP CQP
    hucPakStitchDmem->MAXPass                  = 1;
    hucPakStitchDmem->CurrentPass              = 1;
    hucPakStitchDmem->MinCUSize                = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    hucPakStitchDmem->CabacZeroWordFlag        = false;
    hucPakStitchDmem->bitdepth_luma            = m_hevcSeqParams->bit_depth_luma_minus8 + 8;    // default: 8
    hucPakStitchDmem->bitdepth_chroma          = m_hevcSeqParams->bit_depth_chroma_minus8 + 8;  // default: 8
    hucPakStitchDmem->ChromaFormatIdc          = m_hevcSeqParams->chroma_format_idc;
    hucPakStitchDmem->TotalSizeInCommandBuffer = m_numTiles * CODECHAL_CACHELINE_SIZE;
    // Last tile length may get modified by HuC. Obtain last Tile Record, Add an offset of 8bytes to skip address field in Tile Record
    hucPakStitchDmem->OffsetInCommandBuffer   = tileParams[m_numTiles - 1].TileSizeStreamoutOffset * CODECHAL_CACHELINE_SIZE + 8;
    hucPakStitchDmem->LastTileBS_StartInBytes = (tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE) & (CODECHAL_PAGE_SIZE - 1);

    hucPakStitchDmem->StitchEnable        = true;
    hucPakStitchDmem->StitchCommandOffset = 0;
    hucPakStitchDmem->BBEndforStitch      = HUC_BATCH_BUFFER_END;

    //Set the kernel output offsets
    hucPakStitchDmem->TileSizeRecord_offset[0] = m_hevcFrameStatsOffset.uiTileSizeRecord;
    hucPakStitchDmem->HEVC_PAKSTAT_offset[0]   = 0xFFFFFFFF;
    hucPakStitchDmem->HEVC_Streamout_offset[0] = 0xFFFFFFFF;
    hucPakStitchDmem->VDENCSTAT_offset[0]      = 0xFFFFFFFF;

    for (auto i = 0; i < m_numPipe; i++)
    {
        hucPakStitchDmem->NumTiles[i] = numTilesPerPipe;

        // Statistics are dumped out at a tile level. Driver shares with kernel starting offset of each pipe statistic.
        // Offset is calculated by adding size of statistics/pipe to the offset in combined statistics region.
        hucPakStitchDmem->TileSizeRecord_offset[i + 1] = (i * numTilesPerPipe * m_hevcStatsSize.uiTileSizeRecord) +
                                                         m_hevcTileStatsOffset.uiTileSizeRecord;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]));

    MOS_ZeroMemory(dmemParams, sizeof(MHW_VDBOX_HUC_DMEM_STATE_PARAMS));
    dmemParams->presHucDataSource = &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]);
    dmemParams->dwDataLength      = MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemVdencG12), CODECHAL_CACHELINE_SIZE);
    dmemParams->dwDmemOffset      = HUC_DMEM_OFFSET_RTOS_GEMS;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetDmemHuCPakIntegrate(
    PMHW_VDBOX_HUC_DMEM_STATE_PARAMS    dmemParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_VDENC_BRC_NUM_OF_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    HucPakStitchDmemVdencG12* hucPakStitchDmem = (HucPakStitchDmemVdencG12*)m_osInterface->pfnLockResource(
        m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]), &lockFlagsWriteOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(hucPakStitchDmem);
    MOS_ZeroMemory(hucPakStitchDmem, sizeof(HucPakStitchDmemVdencG12));

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileParams);

    // Reset all the offsets to be shared in the huc dmem (6*5 DW's)
    MOS_FillMemory(hucPakStitchDmem, 120, 0xFF);

    uint16_t numTileRows                        = m_hevcPicParams->num_tile_rows_minus1 + 1;
    uint16_t numTileColumns                     = m_hevcPicParams->num_tile_columns_minus1 + 1;
    uint16_t numTiles                           = numTileRows * numTileColumns;
    uint16_t numTilesPerPipe                    = m_numTiles / m_numPipe;

    hucPakStitchDmem->TotalSizeInCommandBuffer = m_numTiles * CODECHAL_CACHELINE_SIZE;
    // Last tile length may get modified by HuC. Obtain last Tile Record, Add an offset of 8bytes to skip address field in Tile Record
    hucPakStitchDmem->OffsetInCommandBuffer    = (m_numTiles - 1) * CODECHAL_CACHELINE_SIZE + 8;
    hucPakStitchDmem->PicWidthInPixel          = (uint16_t)m_frameWidth;
    hucPakStitchDmem->PicHeightInPixel         = (uint16_t)m_frameHeight;
    hucPakStitchDmem->TotalNumberOfPAKs        = m_numPipe;
    hucPakStitchDmem->Codec                    = 2;             // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
    hucPakStitchDmem->MAXPass                  = m_brcEnabled ? CODECHAL_VDENC_BRC_NUM_OF_PASSES : 1;
    hucPakStitchDmem->CurrentPass              = (uint8_t) currentPass + 1;      // Current BRC pass [1..MAXPass]
    hucPakStitchDmem->MinCUSize                = m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3;
    hucPakStitchDmem->CabacZeroWordFlag        = false;
    hucPakStitchDmem->bitdepth_luma            = m_hevcSeqParams->bit_depth_luma_minus8 + 8;    // default: 8
    hucPakStitchDmem->bitdepth_chroma          = m_hevcSeqParams->bit_depth_chroma_minus8 + 8;  // default: 8
    hucPakStitchDmem->ChromaFormatIdc          = m_hevcSeqParams->chroma_format_idc;
    hucPakStitchDmem->LastTileBS_StartInBytes  = (tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE) & (CODECHAL_PAGE_SIZE - 1);
    hucPakStitchDmem->PIC_STATE_StartInBytes   = (uint16_t)m_picStateCmdStartInBytes;
    CODECHAL_ENCODE_VERBOSEMESSAGE("last tile offset = 0x%x, LastTileBS_StartInBytes =0x%x, (tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE), hucPakStitchDmem->LastTileBS_StartInBytes");
    if(m_enableTileStitchByHW)
    {
        hucPakStitchDmem->StitchEnable = true;
        hucPakStitchDmem->StitchCommandOffset = 0;
        hucPakStitchDmem->BBEndforStitch = HUC_BATCH_BUFFER_END;
    }

    if (m_numPipe > 1)
    {
        //Set the kernel output offsets
        hucPakStitchDmem->HEVC_PAKSTAT_offset[0]   = m_hevcFrameStatsOffset.uiHevcPakStatistics;
        hucPakStitchDmem->HEVC_Streamout_offset[0] = m_hevcFrameStatsOffset.uiHevcSliceStreamout;
        hucPakStitchDmem->TileSizeRecord_offset[0] = m_hevcFrameStatsOffset.uiTileSizeRecord;
        hucPakStitchDmem->VDENCSTAT_offset[0]      = m_hevcFrameStatsOffset.uiVdencStatistics;

        // Calculate number of slices that execute on a single pipe
        for (auto tileRow = 0; tileRow < numTileRows; tileRow++)
        {
            for (auto tileCol = 0; tileCol < numTileColumns; tileCol++)
            {
                PCODEC_ENCODER_SLCDATA  slcData = m_slcData;
                uint16_t  slcCount, idx, sliceNumInTile = 0;

                idx = tileRow * numTileColumns + tileCol;
                for (slcCount = 0; slcCount < m_numSlices; slcCount++)
                {
                    bool    lastSliceInTile = false, sliceInTile = false;

                    CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                        &tileParams[idx],
                        &sliceInTile,
                        &lastSliceInTile));

                    if (!sliceInTile)
                    {
                        continue;
                    }

                    sliceNumInTile++;
                } // end of slice
                if (0 == sliceNumInTile)
                {
                    // One tile must have at least one slice
                    CODECHAL_ENCODE_ASSERT(false);
                    eStatus = MOS_STATUS_INVALID_PARAMETER;
                    break;
                }

                if (sliceNumInTile > 1 && (numTileColumns > 1 || numTileRows > 1))
                {
                    CODECHAL_ENCODE_ASSERTMESSAGE("Multi-slices in a tile is not supported!");
                    return MOS_STATUS_INVALID_PARAMETER;
                }
                // Set the number of slices per pipe in the Dmem structure
                hucPakStitchDmem->NumSlices[tileCol] += sliceNumInTile;
            }
        }

        for (auto i = 0; i < m_numPipe; i++)
        {
            hucPakStitchDmem->NumTiles[i]   = numTilesPerPipe;
            hucPakStitchDmem->NumSlices[i]  = numTilesPerPipe;      // Assuming 1 slice/ tile. To do: change this later.

            // Statistics are dumped out at a tile level. Driver shares with kernel starting offset of each pipe statistic.
            // Offset is calculated by adding size of statistics/pipe to the offset in combined statistics region.
            hucPakStitchDmem->TileSizeRecord_offset[i + 1] = (i * numTilesPerPipe * m_hevcStatsSize.uiTileSizeRecord) + m_hevcTileStatsOffset.uiTileSizeRecord;
            hucPakStitchDmem->HEVC_PAKSTAT_offset[i + 1]   = (i * numTilesPerPipe * m_hevcStatsSize.uiHevcPakStatistics) + m_hevcTileStatsOffset.uiHevcPakStatistics;
            hucPakStitchDmem->VDENCSTAT_offset[i + 1]      = (i * numTilesPerPipe * m_hevcStatsSize.uiVdencStatistics) + m_hevcTileStatsOffset.uiVdencStatistics;
            hucPakStitchDmem->HEVC_Streamout_offset[i + 1] = (i * hucPakStitchDmem->NumSlices[i] * CODECHAL_CACHELINE_SIZE) + m_hevcTileStatsOffset.uiHevcSliceStreamout;
        }
    }
    else
    {
        hucPakStitchDmem->NumTiles[0]               = numTiles;
        hucPakStitchDmem->TotalNumberOfPAKs         = m_numPipe;

        // non-scalable mode, only VDEnc statistics need to be aggregated
        hucPakStitchDmem->VDENCSTAT_offset[0] = m_hevcFrameStatsOffset.uiVdencStatistics;
        hucPakStitchDmem->VDENCSTAT_offset[1] = m_hevcTileStatsOffset.uiVdencStatistics;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]));

    MOS_ZeroMemory(dmemParams, sizeof(MHW_VDBOX_HUC_DMEM_STATE_PARAMS));
    dmemParams->presHucDataSource = &(m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass]);
    dmemParams->dwDataLength = MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemVdencG12), CODECHAL_CACHELINE_SIZE);
    dmemParams->dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::HucPakIntegrate(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    auto mmioRegisters = m_hwInterface->GetHucInterface()->GetMmioRegisters(m_vdboxIndex);

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = VDBOX_HUC_PAK_INTEGRATION_KERNEL_DESCRIPTOR;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    // DMEM set
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCPakIntegrate(&dmemParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCPakIntegrate(&virtualAddrParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    // Write HUC_STATUS2 mask - bit 6 - valid IMEM loaded
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resHucStatus2Buffer;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue = m_hwInterface->GetHucInterface()->GetHucStatus2ImemLoadedMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    // Store HUC_STATUS2 register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resHucStatus2Buffer;
    storeRegParams.dwOffset = sizeof(uint32_t);
    storeRegParams.dwRegister = mmioRegisters->hucStatus2RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVdencInterface()->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    EncodeStatusBuffer encodeStatusBuf = m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf.wCurrIndex * encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

                                                                                             // Write HUC_STATUS mask
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &encodeStatusBuf.resStatusBuffer;
    storeDataParams.dwResourceOffset = baseOffset + encodeStatusBuf.dwHuCStatusMaskOffset;
    storeDataParams.dwValue = m_hwInterface->GetHucInterface()->GetHucStatusReEncodeMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    // store HUC_STATUS register
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &encodeStatusBuf.resStatusBuffer;
    storeRegParams.dwOffset = baseOffset + encodeStatusBuf.dwHuCStatusRegOffset;
    storeRegParams.dwRegister = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &storeRegParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::HucPakIntegrateStitch(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_COND_RETURN(
        (m_vdboxIndex > m_hwInterface->GetMfxInterface()->GetMaxVdboxIndex()),
        "ERROR - vdbox index exceed the maximum");

    auto mmioRegisters = m_hwInterface->GetHucInterface()->GetMmioRegisters(m_vdboxIndex);

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = VDBOX_HUC_PAK_INTEGRATION_KERNEL_DESCRIPTOR;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucImemStateCmd(cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucPipeModeSelectCmd(cmdBuffer, &pipeModeSelectParams));

    // DMEM set
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCPakIntegrateStitch(&dmemParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucDmemStateCmd(cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCPakIntegrateStitch(&virtualAddrParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucVirtualAddrStateCmd(cmdBuffer, &virtualAddrParams));

    // Write HUC_STATUS2 mask - bit 6 - valid IMEM loaded
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resHucStatus2Buffer;
    storeDataParams.dwResourceOffset = 0;
    storeDataParams.dwValue = m_hwInterface->GetHucInterface()->GetHucStatus2ImemLoadedMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(cmdBuffer, &storeDataParams));

    // Store HUC_STATUS2 register
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resHucStatus2Buffer;
    storeRegParams.dwOffset = sizeof(uint32_t);
    storeRegParams.dwRegister = mmioRegisters->hucStatus2RegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(cmdBuffer, &storeRegParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetHucInterface()->AddHucStartCmd(cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetVdencInterface()->AddVdPipelineFlushCmd(cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));

    EncodeStatusBuffer encodeStatusBuf = m_encodeStatusBuf;

    uint32_t baseOffset =
        (encodeStatusBuf.wCurrIndex * encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2;  // pEncodeStatus is offset by 2 DWs in the resource

    // Write HUC_STATUS mask
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &encodeStatusBuf.resStatusBuffer;
    storeDataParams.dwResourceOffset = baseOffset + encodeStatusBuf.dwHuCStatusMaskOffset;
    storeDataParams.dwValue = m_hwInterface->GetHucInterface()->GetHucStatusReEncodeMask();
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(
        cmdBuffer,
        &storeDataParams));

    // store HUC_STATUS register
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &encodeStatusBuf.resStatusBuffer;
    storeRegParams.dwOffset = baseOffset + encodeStatusBuf.dwHuCStatusRegOffset;
    storeRegParams.dwRegister = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(
        cmdBuffer,
        &storeRegParams));

    return eStatus;
}

void CodechalVdencHevcStateG12::CreateMhwParams()
{
    m_sliceStateParams = MOS_New(MHW_VDBOX_HEVC_SLICE_STATE_G12);
    m_pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12);
    m_pipeBufAddrParams = MOS_New(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12);
}

MOS_STATUS CodechalVdencHevcStateG12::CalculatePictureStateCommandSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MHW_VDBOX_STATE_CMDSIZE_PARAMS_G12 stateCmdSizeParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(
        m_hwInterface->GetHxxStateCommandSize(
            CODECHAL_ENCODE_MODE_HEVC,
            &m_defaultPictureStatesSize,
            &m_defaultPicturePatchListSize,
            &stateCmdSizeParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

#ifdef _MMC_SUPPORTED
    m_mmcState->SetPipeBufAddr(m_pipeBufAddrParams);
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hcpInterface->AddHcpPipeBufAddrCmd(cmdBuffer, m_pipeBufAddrParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetTileData(
    MHW_VDBOX_HCP_TILE_CODING_PARAMS_G12*   tileCodingParams)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        return eStatus;
    }

    uint32_t colBd[100] = { 0 };
    uint32_t numTileColumns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    for (uint32_t i = 0; i < numTileColumns; i++)
    {
        colBd[i + 1] = colBd[i] + m_hevcPicParams->tile_column_width[i];
    }

    uint32_t rowBd[100] = { 0 };
    uint32_t numTileRows = m_hevcPicParams->num_tile_rows_minus1 + 1;
    for (uint32_t i = 0; i < numTileRows; i++)
    {
        rowBd[i + 1] = rowBd[i] + m_hevcPicParams->tile_row_height[i];
    }

    m_numTiles = numTileRows * numTileColumns;
    if (m_numTiles > CODECHAL_GET_WIDTH_IN_BLOCKS(m_frameWidth, CODECHAL_HEVC_VDENC_MIN_TILE_WIDTH_SIZE) *
        CODECHAL_GET_HEIGHT_IN_BLOCKS(m_frameHeight, CODECHAL_HEVC_VDENC_MIN_TILE_HEIGHT_SIZE))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_numTileRows = numTileRows;

    uint32_t const numCuRecordTab[] = { 1, 4, 16, 64 }; //LCU: 8x8->1, 16x16->4, 32x32->16, 64x64->64
    uint32_t       numCuRecord = numCuRecordTab[MOS_MIN(3, m_hevcSeqParams->log2_max_coding_block_size_minus3)];
    uint32_t       maxBytePerLCU = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    maxBytePerLCU = maxBytePerLCU * maxBytePerLCU; // number of pixels per LCU
    maxBytePerLCU = maxBytePerLCU * 3 / (m_is10BitHevc ? 1 : 2);  //assume 4:2:0 format
    uint32_t    bitstreamByteOffset = 0, saoRowstoreOffset = 0, cuLevelStreamoutOffset = 0, sseRowstoreOffset = 0;
    int32_t     frameWidthInMinCb = m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1;
    int32_t     frameHeightInMinCb = m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1;
    int32_t     shift = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
    uint32_t    ctbSize = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t    streamInWidthinLCU = MOS_ROUNDUP_DIVIDE((frameWidthInMinCb << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);
    uint32_t    numLcuInPic = 0;
    uint32_t    tileStartLCUAddr = 0;

    for (uint32_t numLcusInTiles = 0, i = 0; i < numTileRows; i++)
    {
        for (uint32_t j = 0; j < numTileColumns; j++)
        {
            numLcuInPic += m_hevcPicParams->tile_row_height[i] * m_hevcPicParams->tile_column_width[j];
        }
    }

    uint32_t    numSliceInTile = 0;
    uint64_t    activeBitstreamSize = (uint64_t)m_encodeParams.dwBitstreamSize;
    // There would be padding at the end of last tile in CBR, reserve dedicated part in the BS buf
    if (m_hevcSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        // Assume max padding num < target frame size derived from target bit rate and frame rate
        uint32_t actualFrameRate = m_hevcSeqParams->FrameRate.Numerator / m_hevcSeqParams->FrameRate.Denominator;
        uint64_t reservedPart    = (uint64_t)m_hevcSeqParams->TargetBitRate / 8 / (uint64_t)actualFrameRate * 1024;

        if (reservedPart > activeBitstreamSize)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Frame size cal from target Bit rate is larger than BS buf! Issues in CBR paras!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        // Capping the reserved part to 1/10 of bs buf size
        if (reservedPart > activeBitstreamSize / 10)
        {
            reservedPart = activeBitstreamSize / 10;
        }

        activeBitstreamSize -= reservedPart;
    }

    for (uint32_t numLcusInTiles = 0, i = 0; i < numTileRows; i++)
    {
        for (uint32_t j = 0; j < numTileColumns; j++)
        {
            uint32_t idx = i * numTileColumns + j;
            uint32_t numLcuInTile = m_hevcPicParams->tile_row_height[i] * m_hevcPicParams->tile_column_width[j];

            tileCodingParams[idx].TileStartLCUX = colBd[j];
            tileCodingParams[idx].TileStartLCUY = rowBd[i];

            tileCodingParams[idx].TileColumnStoreSelect = j % 2;
            tileCodingParams[idx].TileRowStoreSelect = i % 2;

            if (j != numTileColumns - 1)
            {
                tileCodingParams[idx].TileWidthInMinCbMinus1 = (m_hevcPicParams->tile_column_width[j] << shift) - 1;
                tileCodingParams[idx].IsLastTileofRow = false;
            }
            else
            {
                tileCodingParams[idx].TileWidthInMinCbMinus1 = (frameWidthInMinCb - (colBd[j] << shift)) - 1;
                tileCodingParams[idx].IsLastTileofRow = true;

            }

            if (i != numTileRows - 1)
            {
                tileCodingParams[idx].IsLastTileofColumn = false;
                tileCodingParams[idx].TileHeightInMinCbMinus1 = (m_hevcPicParams->tile_row_height[i] << shift) - 1;
            }
            else
            {
                tileCodingParams[idx].TileHeightInMinCbMinus1 = (frameHeightInMinCb - (rowBd[i] << shift)) - 1;
                tileCodingParams[idx].IsLastTileofColumn = true;
            }

            tileCodingParams[idx].NumOfTilesInFrame = m_numTiles;
            tileCodingParams[idx].NumOfTileColumnsInFrame = numTileColumns;
            tileCodingParams[idx].CuRecordOffset = MOS_ALIGN_CEIL(((numCuRecord * numLcusInTiles) * m_hcpInterface->GetHevcEncCuRecordSize()),
                CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;
            tileCodingParams[idx].NumberOfActiveBePipes = (m_numPipe > 1) ? m_numPipe : 1;

            tileCodingParams[idx].PakTileStatisticsOffset = 9 * idx;
            tileCodingParams[idx].TileSizeStreamoutOffset = idx;
            tileCodingParams[idx].Vp9ProbabilityCounterStreamoutOffset = 0;
            tileCodingParams[idx].presHcpSyncBuffer = &m_resHcpScalabilitySyncBuffer.sResource;
            tileCodingParams[idx].CuLevelStreamoutOffset = cuLevelStreamoutOffset;
            tileCodingParams[idx].SliceSizeStreamoutOffset = numSliceInTile;
            tileCodingParams[idx].SseRowstoreOffset = sseRowstoreOffset;
            tileCodingParams[idx].BitstreamByteOffset = bitstreamByteOffset;
            tileCodingParams[idx].SaoRowstoreOffset = saoRowstoreOffset;

            uint32_t tileHeightInLCU = MOS_ROUNDUP_DIVIDE(((tileCodingParams[idx].TileHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);
            uint32_t tileWidthInLCU = MOS_ROUNDUP_DIVIDE(((tileCodingParams[idx].TileWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);

            //StreamIn data is 4 CLs per LCU
            tileCodingParams[idx].TileStreaminOffset = 4 * (tileCodingParams[idx].TileStartLCUY * streamInWidthinLCU + tileCodingParams[idx].TileStartLCUX * tileHeightInLCU);
            tileCodingParams[idx].SliceSizeStreamoutOffset = tileStartLCUAddr;
            tileStartLCUAddr += (tileWidthInLCU * tileHeightInLCU);

            cuLevelStreamoutOffset += (tileCodingParams[idx].TileWidthInMinCbMinus1 + 1) * (tileCodingParams[idx].TileHeightInMinCbMinus1 + 1) * 16 / CODECHAL_CACHELINE_SIZE;
            sseRowstoreOffset += ((m_hevcPicParams->tile_column_width[j] + 3) * m_sizeOfSseSrcPixelRowStoreBufferPerLcu) / CODECHAL_CACHELINE_SIZE;
            saoRowstoreOffset += (MOS_ALIGN_CEIL(m_hevcPicParams->tile_column_width[j], 4) * CODECHAL_HEVC_SAO_STRMOUT_SIZE_PERLCU) / CODECHAL_CACHELINE_SIZE;

            uint64_t totalSizeTemp = (uint64_t)activeBitstreamSize * (uint64_t)numLcuInTile;
            uint32_t bitStreamSizePerTile = (uint32_t)(totalSizeTemp / (uint64_t)numLcuInPic) + ((totalSizeTemp % (uint64_t)numLcuInPic) ? 1 : 0);
            bitstreamByteOffset += MOS_ALIGN_CEIL(bitStreamSizePerTile, CODECHAL_CACHELINE_SIZE) / CODECHAL_CACHELINE_SIZE;

            numLcusInTiles += numLcuInTile;

            for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
            {
                bool lastSliceInTile = false, sliceInTile = false;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(IsSliceInTile(slcCount,
                    &tileCodingParams[idx],
                    &sliceInTile,
                    &lastSliceInTile));
                numSliceInTile += (sliceInTile ? 1 : 0);
            }
        }

        // same row store buffer for different tile rows.
        saoRowstoreOffset = 0;
        sseRowstoreOffset = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::IsSliceInTile(
    uint32_t                                sliceNumber,
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12   currentTile,
    bool                                   *sliceInTile,
    bool                                   *lastSliceInTile)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(currentTile);
    CODECHAL_ENCODE_CHK_NULL_RETURN(sliceInTile);
    CODECHAL_ENCODE_CHK_NULL_RETURN(lastSliceInTile);

    uint32_t shift            = m_hevcSeqParams->log2_max_coding_block_size_minus3 - m_hevcSeqParams->log2_min_coding_block_size_minus3;
    uint32_t residual = (1 << shift) - 1;
    uint32_t frameWidthInLCU  = (m_hevcSeqParams->wFrameWidthInMinCbMinus1 + 1 + residual) >> shift;
    uint32_t frameHeightInLCU = (m_hevcSeqParams->wFrameHeightInMinCbMinus1 + 1 + residual) >> shift;

    PCODEC_HEVC_ENCODE_SLICE_PARAMS hevcSlcParams = &m_hevcSliceParams[sliceNumber];
    uint32_t sliceStartLCU = hevcSlcParams->slice_segment_address;
    uint32_t sliceLCUx = sliceStartLCU % frameWidthInLCU;
    uint32_t sliceLCUy = sliceStartLCU / frameWidthInLCU;

    uint32_t tileColumnWidth = (currentTile->TileWidthInMinCbMinus1 + 1 + residual) >> shift;
    uint32_t tileRowHeight = (currentTile->TileHeightInMinCbMinus1 + 1 + residual) >> shift;
    if (sliceLCUx <  currentTile->TileStartLCUX ||
        sliceLCUy <  currentTile->TileStartLCUY ||
        sliceLCUx >= currentTile->TileStartLCUX + tileColumnWidth ||
        sliceLCUy >= currentTile->TileStartLCUY + tileRowHeight
        )
    {
        // slice start is not in the tile boundary
        *lastSliceInTile = *sliceInTile = false;
        return eStatus;
    }

    sliceLCUx += (hevcSlcParams->NumLCUsInSlice - 1) % tileColumnWidth;
    sliceLCUy += (hevcSlcParams->NumLCUsInSlice - 1) / tileColumnWidth;

    if (sliceLCUx >= currentTile->TileStartLCUX + tileColumnWidth)
    {
        sliceLCUx -= tileColumnWidth;
        sliceLCUy++;
    }

    if (sliceLCUx <  currentTile->TileStartLCUX ||
        sliceLCUy <  currentTile->TileStartLCUY ||
        sliceLCUx >= currentTile->TileStartLCUX + tileColumnWidth ||
        sliceLCUy >= currentTile->TileStartLCUY + tileRowHeight
        )
    {
        // last LCU of the slice is out of the tile boundary
        *lastSliceInTile = *sliceInTile = false;
        return eStatus;
    }

    *sliceInTile = true;

    sliceLCUx++;
    sliceLCUy++;

    // the end of slice is at the boundary of tile
    *lastSliceInTile = (
        sliceLCUx == currentTile->TileStartLCUX + tileColumnWidth &&
        sliceLCUy == currentTile->TileStartLCUY + tileRowHeight);

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeHevcG12, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

#ifdef _ENCODE_VDENC_RESERVED
MOS_STATUS CodechalVdencHevcStateG12::InitReserveState(CodechalSetting * settings)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_rsvdState = MOS_New(CodechalVdencHevcG12Rsvd, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_rsvdState);
    m_rsvdState->Initialize(settings);
    return MOS_STATUS_SUCCESS;
}
#endif

uint32_t CodechalVdencHevcStateG12::CalculateCommandBufferSize()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    // To be refined later, differentiate BRC and CQP
    uint32_t commandBufferSize =
        m_pictureStatesSize        +
        m_extraPictureStatesSize   +
        (m_sliceStatesSize * m_numSlices) +
        m_hucCommandsSize * 5;

    if (m_singleTaskPhaseSupported)
    {
        commandBufferSize *= (m_numPasses + 1);
    }

    if (m_osInterface->bUsesPatchList && m_hevcPicParams->tiles_enabled_flag)
    {
        commandBufferSize += (m_tileLevelBatchSize * m_numTiles * CODECHAL_VDENC_BRC_NUM_OF_PASSES);
    }

    // 4K align since allocation is in chunks of 4K bytes.
    commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, 0x1000);

    return commandBufferSize;
}

MOS_STATUS CodechalVdencHevcStateG12::VerifyCommandBufferSize()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode & resize CommandBuffer Size for every BRC pass
        if (!m_singleTaskPhaseSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
        }
        return eStatus;
    }

    // virtual engine
    uint32_t requestedSize =
        m_pictureStatesSize +
        m_extraPictureStatesSize +
        (m_sliceStatesSize * m_numSlices);

    requestedSize += (requestedSize * m_numPassesInOnePipe + m_hucCommandsSize);

    // Running in the multiple VDBOX mode
    int currentPipe = GetCurrentPipe();
    if (currentPipe < 0 || currentPipe >= m_numPipe)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (IsFirstPipe() && m_osInterface->bUsesPatchList)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    PMOS_COMMAND_BUFFER pCmdBuffer;
    if (m_osInterface->phasedSubmission)
    {
        m_osInterface->pfnVerifyCommandBufferSize(m_osInterface, requestedSize, 0);
        return eStatus;
    }
    else
    {
        pCmdBuffer = m_singleTaskPhaseSupported ? &m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][0] : &m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][currentPass];
    }

    if (Mos_ResourceIsNull(&pCmdBuffer->OsResource) ||
        m_sizeOfVeBatchBuffer < requestedSize)
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;

        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = requestedSize;
        allocParamsForBufferLinear.pBufName = "Batch buffer for each VDBOX";

        if (!Mos_ResourceIsNull(&pCmdBuffer->OsResource))
        {
            if (pCmdBuffer->pCmdBase)
            {
                m_osInterface->pfnUnlockResource(m_osInterface, &pCmdBuffer->OsResource);
            }
            m_osInterface->pfnFreeResource(m_osInterface, &pCmdBuffer->OsResource);
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &pCmdBuffer->OsResource));

        m_sizeOfVeBatchBuffer = requestedSize;
    }

    if (pCmdBuffer->pCmdBase == nullptr)
    {
        MOS_LOCK_PARAMS lockParams;
        MOS_ZeroMemory(&lockParams, sizeof(lockParams));
        lockParams.WriteOnly = true;
        pCmdBuffer->pCmdPtr = pCmdBuffer->pCmdBase = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &pCmdBuffer->OsResource, &lockParams);
        pCmdBuffer->iRemaining                     = m_sizeOfVeBatchBuffer;
        pCmdBuffer->iOffset = 0;

        if (pCmdBuffer->pCmdBase == nullptr)
        {
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::GetCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode
        m_realCmdBuffer.pCmdBase = m_realCmdBuffer.pCmdPtr = nullptr;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, 0));
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &m_realCmdBuffer, 0));

    int currentPipe = GetCurrentPipe();
    if (currentPipe < 0 || currentPipe >= m_numPipe)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_osInterface->phasedSubmission)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, cmdBuffer, currentPipe + 1));

        CodecHalEncodeScalability_EncodePhaseToSubmissionType(IsFirstPipe(), cmdBuffer);
        if (IsLastPipe())
        {
            cmdBuffer->iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
        }
    }
    else
    {
        *cmdBuffer = m_singleTaskPhaseSupported ? m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][0] : m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][currentPass];
    }

    if (m_osInterface->osCpInterface->IsCpEnabled() && cmdBuffer->iOffset == 0)
    {
        // Insert CP Prolog
        CODECHAL_ENCODE_NORMALMESSAGE("Adding cp prolog for secure scalable encode");
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetCpInterface()->AddProlog(m_osInterface, cmdBuffer));
    }
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ReturnCommandBuffer(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, 0);
        return eStatus;
    }

    int currentPipe = GetCurrentPipe();
    if (currentPipe < 0 || currentPipe >= m_numPipe)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_osInterface->phasedSubmission)
    {
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, cmdBuffer, currentPipe + 1);
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_realCmdBuffer, 0);
    }
    else
    {
        uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
        m_veBatchBuffer[m_virtualEngineBbIndex][currentPipe][passIndex] = *cmdBuffer;
        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &m_realCmdBuffer, 0);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    if (IsLastPass())
    {
        HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface->pOsContext);
    }

    if (UseRenderCommandBuffer() || m_numPipe == 1)
    {
        // legacy mode
        if (!UseRenderCommandBuffer())  // Set VE Hints for video contexts only
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
        return eStatus;
    }

    bool cmdBufferReadyForSubmit = IsLastPipe();

    // In STF, Hold the command buffer submission till last pass
    if (m_singleTaskPhaseSupported)
    {
        cmdBufferReadyForSubmit = cmdBufferReadyForSubmit && IsLastPass();
    }

    if(!cmdBufferReadyForSubmit)
    {
        return eStatus;
    }

    int currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (m_osInterface->phasedSubmission)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_realCmdBuffer, nullRendering));
    }
    else
    {
        uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;

        for (uint32_t i = 0; i < m_numPipe; i++)
        {
            PMOS_COMMAND_BUFFER cmdBuffer = &m_veBatchBuffer[m_virtualEngineBbIndex][i][passIndex];

            if(cmdBuffer->pCmdBase)
            {
                m_osInterface->pfnUnlockResource(m_osInterface, &cmdBuffer->OsResource);
            }

            cmdBuffer->pCmdBase = 0;
            cmdBuffer->iOffset = cmdBuffer->iRemaining = 0;
        }
        m_sizeOfVeBatchBuffer = 0;

        if(eStatus == MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(&m_realCmdBuffer));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &m_realCmdBuffer, nullRendering));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                  frameTrackingRequested,
    MHW_MI_MMIOREGISTERS *mmioRegister)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    MOS_GPU_CONTEXT gpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    if (UseRenderCommandBuffer())
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTrackingRequested, mmioRegister));
        return eStatus;
    }

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SendPrologCmd(m_miInterface, cmdBuffer, gpuContext));
#endif

    if (!IsLastPipe())
    {
        return eStatus;
    }

    PMOS_COMMAND_BUFFER commandBufferInUse;
    if (m_realCmdBuffer.pCmdBase)
    {
        commandBufferInUse = &m_realCmdBuffer;
    }
    else
        if (cmdBuffer && cmdBuffer->pCmdBase)
        {
            commandBufferInUse = cmdBuffer;
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

    // initialize command buffer attributes
    commandBufferInUse->Attributes.bTurboMode = m_hwInterface->m_turboMode;
    commandBufferInUse->Attributes.dwNumRequestedEUSlices = m_hwInterface->m_numRequestedEuSlices;
    commandBufferInUse->Attributes.dwNumRequestedSubSlices = m_hwInterface->m_numRequestedSubSlices;
    commandBufferInUse->Attributes.dwNumRequestedEUs = m_hwInterface->m_numRequestedEus;
    commandBufferInUse->Attributes.bValidPowerGatingRequest = true;

    if (frameTrackingRequested && m_frameTrackingEnabled)
    {
        commandBufferInUse->Attributes.bEnableMediaFrameTracking = true;
        commandBufferInUse->Attributes.resMediaFrameTrackingSurface =
            m_encodeStatusBuf.resStatusBuffer;
        commandBufferInUse->Attributes.dwMediaFrameTrackingTag = m_storeData;
        // Set media frame tracking address offset(the offset from the encoder status buffer page)
        commandBufferInUse->Attributes.dwMediaFrameTrackingAddrOffset = 0;
    }

    MHW_GENERIC_PROLOG_PARAMS  genericPrologParams;
    MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
    genericPrologParams.pOsInterface = m_hwInterface->GetOsInterface();
    genericPrologParams.pvMiInterface = m_hwInterface->GetMiInterface();
    genericPrologParams.bMmcEnabled = CodecHalMmcState::IsMmcEnabled();
    genericPrologParams.dwStoreDataValue = m_storeData - 1;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(commandBufferInUse, &genericPrologParams));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetSliceStructs()
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    eStatus = CodechalEncodeHevcBase::SetSliceStructs();
    m_numPassesInOnePipe                        = m_numPasses;
    m_numPasses                                 = (m_numPasses + 1) * m_numPipe - 1;
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::AllocateTileStatistics()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        return eStatus;
    }

    auto num_tile_rows    = m_hevcPicParams->num_tile_rows_minus1 + 1;
    auto num_tile_columns = m_hevcPicParams->num_tile_columns_minus1 + 1;
    auto num_tiles = num_tile_rows * num_tile_columns;

    MOS_ZeroMemory(&m_hevcFrameStatsOffset, sizeof(HEVC_TILE_STATS_INFO));
    MOS_ZeroMemory(&m_hevcTileStatsOffset, sizeof(HEVC_TILE_STATS_INFO));
    MOS_ZeroMemory(&m_hevcStatsSize, sizeof(HEVC_TILE_STATS_INFO));

    MOS_LOCK_PARAMS lockFlagsWriteOnly;
    MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
    lockFlagsWriteOnly.WriteOnly = true;

    // Set the maximum size based on frame level statistics.
    m_hevcStatsSize.uiTileSizeRecord     = CODECHAL_CACHELINE_SIZE;
    m_hevcStatsSize.uiHevcPakStatistics  = m_sizeOfHcpPakFrameStats;
    m_hevcStatsSize.uiVdencStatistics    = CODECHAL_HEVC_VDENC_STATS_SIZE;
    m_hevcStatsSize.uiHevcSliceStreamout = CODECHAL_CACHELINE_SIZE;

    // Maintain the offsets to use for patching addresses in to the HuC Pak Integration kernel Aggregated Frame Statistics Output Buffer
    // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
    m_hevcFrameStatsOffset.uiTileSizeRecord     = 0;  // Tile Size Record is not present in resHuCPakAggregatedFrameStatsBuffer
    m_hevcFrameStatsOffset.uiHevcPakStatistics  = 0;
    m_hevcFrameStatsOffset.uiVdencStatistics    = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.uiHevcPakStatistics + m_hevcStatsSize.uiHevcPakStatistics, CODECHAL_PAGE_SIZE);
    m_hevcFrameStatsOffset.uiHevcSliceStreamout = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.uiVdencStatistics + m_hevcStatsSize.uiVdencStatistics, CODECHAL_PAGE_SIZE);

    // Frame level statistics
    m_hwInterface->m_pakIntAggregatedFrameStatsSize = MOS_ALIGN_CEIL(m_hevcFrameStatsOffset.uiHevcSliceStreamout + (m_hevcStatsSize.uiHevcSliceStreamout * m_numLcu), CODECHAL_PAGE_SIZE);

    // HEVC Frame Statistics Buffer - Output from HuC PAK Integration kernel
    if (Mos_ResourceIsNull(&m_resHuCPakAggregatedFrameStatsBuffer.sResource))
    {
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = m_hwInterface->m_pakIntAggregatedFrameStatsSize;
        allocParamsForBufferLinear.pBufName = "GEN12 HCP Aggregated Frame Statistics Streamout Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resHuCPakAggregatedFrameStatsBuffer.sResource));
        m_resHuCPakAggregatedFrameStatsBuffer.dwSize = m_hwInterface->m_pakIntAggregatedFrameStatsSize;

        uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resHuCPakAggregatedFrameStatsBuffer.sResource,
            &lockFlagsWriteOnly);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);
        MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resHuCPakAggregatedFrameStatsBuffer.sResource);
    }

    // Maintain the offsets to use for patching addresses in to the Tile Based Statistics Buffer
    // Each offset needs to be page aligned as the combined region is fed into different page aligned HuC regions
    m_hevcTileStatsOffset.uiTileSizeRecord     = 0; // TileReord is in a separated resource
    m_hevcTileStatsOffset.uiHevcPakStatistics  = 0; // PakStaticstics is head of m_resTileBasedStatisticsBuffer;
    m_hevcTileStatsOffset.uiVdencStatistics    = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.uiHevcPakStatistics + (m_hevcStatsSize.uiHevcPakStatistics * num_tiles), CODECHAL_PAGE_SIZE);
    m_hevcTileStatsOffset.uiHevcSliceStreamout = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.uiVdencStatistics + (m_hevcStatsSize.uiVdencStatistics * num_tiles), CODECHAL_PAGE_SIZE);
    // Combined statistics size for all tiles
    m_hwInterface->m_pakIntTileStatsSize = MOS_ALIGN_CEIL(m_hevcTileStatsOffset.uiHevcSliceStreamout + m_hevcStatsSize.uiHevcSliceStreamout * m_numLcu, CODECHAL_PAGE_SIZE);

    // Tile size record size for all tiles
    m_hwInterface->m_tileRecordSize = m_hevcStatsSize.uiTileSizeRecord * num_tiles;

    if (Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource) || m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].dwSize < m_hwInterface->m_pakIntTileStatsSize)
    {
        if (!Mos_ResourceIsNull(&m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource);
        }
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = m_hwInterface->m_pakIntTileStatsSize;
        allocParamsForBufferLinear.pBufName = "GEN12 HCP Tile Level Statistics Streamout Buffer";

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBufferLinear,
            &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource));
        m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].dwSize = m_hwInterface->m_pakIntTileStatsSize;

        uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource,
            &lockFlagsWriteOnly);
        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, allocParamsForBufferLinear.dwBytes);
        m_osInterface->pfnUnlockResource(m_osInterface, &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource);
    }

    // Allocate the updated tile size buffer for PAK integration kernel
    if (Mos_ResourceIsNull(&m_tileRecordBuffer[m_virtualEngineBbIndex].sResource) || m_tileRecordBuffer[m_virtualEngineBbIndex].dwSize < m_hwInterface->m_tileRecordSize)
    {
        if (!Mos_ResourceIsNull(&m_tileRecordBuffer[m_virtualEngineBbIndex].sResource))
        {
            m_osInterface->pfnFreeResource(m_osInterface, &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource);
        }
        MOS_ALLOC_GFXRES_PARAMS  allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = m_hwInterface->m_tileRecordSize;
        allocParamsForBufferLinear.pBufName = "Tile Record buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                                                      m_osInterface,
                                                      &allocParamsForBufferLinear,
                                                      &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource),
            "Failed to create GEN12 Tile Record buffer");

        m_tileRecordBuffer[m_virtualEngineBbIndex].dwSize = allocParamsForBufferLinear.dwBytes;
    }

    // Only needed when tile & BRC is enabled, but the size is not changing at frame level
    // Move to more properiate place later
    if (Mos_ResourceIsNull(&m_resBrcDataBuffer))
    {
        uint8_t* data;
        MOS_ALLOC_GFXRES_PARAMS  allocParamsForBufferLinear;

        // Pak stitch DMEM
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemVdencG12), CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName = "PAK Stitch Dmem Buffer";
        auto numOfPasses = CODECHAL_VDENC_BRC_NUM_OF_PASSES;

        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            for (auto i = 0; i < numOfPasses; i++)
            {
                CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
                    m_osInterface->pfnAllocateResource(
                        m_osInterface,
                        &allocParamsForBufferLinear,
                        &m_resHucPakStitchDmemBuffer[k][i]),
                    "Failed to allocate PAK Stitch Dmem Buffer.");

                MOS_LOCK_PARAMS lockFlagsWriteOnly;
                MOS_ZeroMemory(&lockFlagsWriteOnly, sizeof(MOS_LOCK_PARAMS));
                lockFlagsWriteOnly.WriteOnly = 1;

                data = (uint8_t*)m_osInterface->pfnLockResource(
                    m_osInterface,
                    &m_resHucPakStitchDmemBuffer[k][i],
                    &lockFlagsWriteOnly);

                CODECHAL_ENCODE_CHK_NULL_RETURN(data);

                MOS_ZeroMemory(
                    data,
                    allocParamsForBufferLinear.dwBytes);

                m_osInterface->pfnUnlockResource(m_osInterface, &m_resHucPakStitchDmemBuffer[k][i]);
            }
        }

        // BRC Data Buffer
        allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_numTiles * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);
        allocParamsForBufferLinear.pBufName = "BRC Data Buffer";

        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(
            m_osInterface->pfnAllocateResource(
                m_osInterface,
                &allocParamsForBufferLinear,
                &m_resBrcDataBuffer),
            "Failed to allocate BRC Data Buffer Buffer.");

        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            &m_resBrcDataBuffer,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(
            data,
            allocParamsForBufferLinear.dwBytes);

        m_osInterface->pfnUnlockResource(m_osInterface, &m_resBrcDataBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::ReadSseStatistics(PMOS_COMMAND_BUFFER cmdBuffer)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    // encodeStatus is offset by 2 DWs in the resource
    uint32_t sseOffsetinBytes = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) + sizeof(uint32_t) * 2 + m_encodeStatusBuf.dwSumSquareErrorOffset;
    for (auto i = 0; i < 6; i++)    // 64 bit SSE values for luma/ chroma channels need to be copied
    {
        MHW_MI_COPY_MEM_MEM_PARAMS miCpyMemMemParams;
        MOS_ZeroMemory(&miCpyMemMemParams, sizeof(miCpyMemMemParams));
        miCpyMemMemParams.presSrc     = m_hevcPicParams->tiles_enabled_flag && (m_numPipe > 1) ? &m_resHuCPakAggregatedFrameStatsBuffer.sResource : &m_resFrameStatStreamOutBuffer;
        miCpyMemMemParams.dwSrcOffset = (HEVC_PAK_STATISTICS_SSE_OFFSET + i) * sizeof(uint32_t);    // SSE luma offset is located at DW32 in Frame statistics, followed by chroma
        miCpyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
        miCpyMemMemParams.dwDstOffset = sseOffsetinBytes + i * sizeof(uint32_t);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(cmdBuffer, &miCpyMemMemParams));
    }
    return eStatus;
}

void CodechalVdencHevcStateG12::SetHcpIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams)
{
    PCODECHAL_ENCODE_BUFFER tileRecordBuffer    = &m_tileRecordBuffer[m_virtualEngineBbIndex];
    bool useTileRecordBuffer = !Mos_ResourceIsNull(&tileRecordBuffer->sResource);

    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = CODECHAL_ENCODE_MODE_HEVC;
    indObjBaseAddrParams.presMvObjectBuffer = &m_resMbCodeSurface;
    indObjBaseAddrParams.dwMvObjectOffset = m_mvOffset;
    indObjBaseAddrParams.dwMvObjectSize = m_mbCodeSize - m_mvOffset;
    indObjBaseAddrParams.presPakBaseObjectBuffer = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize = m_bitstreamUpperBound;
    indObjBaseAddrParams.presPakTileSizeStasBuffer = useTileRecordBuffer ? &tileRecordBuffer->sResource : nullptr;
    indObjBaseAddrParams.dwPakTileSizeStasBufferSize = useTileRecordBuffer ? m_hwInterface->m_tileRecordSize : 0;
    indObjBaseAddrParams.dwPakTileSizeRecordOffset   = useTileRecordBuffer ? m_hevcTileStatsOffset.uiTileSizeRecord : 0;
}

MOS_STATUS CodechalVdencHevcStateG12::HuCBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    if ((!m_singleTaskPhaseSupported || m_firstTaskInPhase) && (m_numPipe == 1))
    {
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));
    imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcInitKernelDescriptor;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcInitReset());

    // set HuC DMEM param
    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &m_vdencBrcInitDmemBuffer[m_currRecycledBufIdx];
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(m_vdencBrcInitDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    MOS_ZeroMemory(&virtualAddrParams, sizeof(virtualAddrParams));
    virtualAddrParams.regionParams[0].presRegion = &m_vdencBrcHistoryBuffer;
    virtualAddrParams.regionParams[0].isWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &virtualAddrParams));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    if (!m_singleTaskPhaseSupported && (m_osInterface->bNoParsingAssistanceInKmd) && (m_numPipe == 1))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_MEDIA_STATE_BRC_INIT_RESET,
            nullptr)));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));
    }

    CODECHAL_DEBUG_TOOL(DumpHucBrcInit());
    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::HuCBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetCommandBuffer(&cmdBuffer));

    *m_pipeBufAddrParams = {};
    if (m_pictureCodingType != I_TYPE)
    {
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
        {
            if (!m_picIdx[i].bValid || !m_currUsedRefPic[i])
            {
                continue;
            }

            uint8_t idx = m_picIdx[i].ucPicIdx;
            CodecHalGetResourceInfo(m_osInterface, &(m_refList[idx]->sRefReconBuffer));

            uint8_t frameStoreId = (uint8_t)m_refIdxMapping[i];
            m_pipeBufAddrParams->presReferences[frameStoreId] = &(m_refList[idx]->sRefReconBuffer.OsResource);
        }
    }

#ifdef _ENCODE_VDENC_RESERVED
    if (m_rsvdState)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_rsvdState->HuCBrcUpdate(m_pipeBufAddrParams, m_slotForRecNotFiltered));
    }
#endif

    if (((!m_singleTaskPhaseSupported) || ((m_firstTaskInPhase) && (!m_brcInit))) && (m_numPipe == 1))
    {
        // Send command buffer header at the beginning (OS dependent)
        bool requestFrameTracking = m_singleTaskPhaseSupported ?
            m_firstTaskInPhase : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));
    }

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ConstructBatchBufferHuCBRC(&m_vdencReadBatchBuffer[m_currRecycledBufIdx][currentPass]));

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));

    if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)  // Low Delay BRC
    {
        imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcLowdelayKernelDescriptor;
    }
    else
    {
        imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcUpdateKernelDescriptor;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&cmdBuffer, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    // DMEM set
    m_CurrentPassForOverAll = 0;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcUpdate());

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &(m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][currentPass]);
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&cmdBuffer, &dmemParams));

    // Set Const Data buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetConstDataHuCBrcUpdate());

    // Add Virtual addr
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCBrcUpdate(&m_virtualAddrParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&cmdBuffer, &m_virtualAddrParams));

    // Store HUC_STATUS2 register bit 6 before HUC_Start command
    // BitField: VALID IMEM LOADED - This bit will be cleared by HW at the end of a HUC workload
    // (HUC_Start command with last start bit set).
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(StoreHuCStatus2Register(&cmdBuffer));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    // Write HUC_STATUS mask: DW1 (mask value)
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resPakMmioBuffer;
    storeDataParams.dwResourceOffset = sizeof(uint32_t);
    storeDataParams.dwValue = CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &storeDataParams));

    // store HUC_STATUS register: DW0 (actual value)
    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resPakMmioBuffer;
    storeRegParams.dwOffset = 0;
    storeRegParams.dwRegister = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&cmdBuffer, &storeRegParams));

    // DW0 & DW1 will considered together for conditional batch buffer end cmd later
    if ((!m_singleTaskPhaseSupported) && (m_osInterface->bNoParsingAssistanceInKmd) && (m_numPipe == 1))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    // HuC Input
    CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(true));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReturnCommandBuffer(&cmdBuffer));

    if (!m_singleTaskPhaseSupported)
    {
        bool renderingFlags = m_videoContextUsesNullHw;

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_MEDIA_STATE_BRC_UPDATE,
            nullptr)));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, renderingFlags));
    }

    // HuC Output
    CODECHAL_DEBUG_TOOL(DumpHucBrcUpdate(false));

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::HuCBrcTileRowUpdate(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = true;

    uint8_t *data = (uint8_t *)m_osInterface->pfnLockResource(m_osInterface, &(m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow].OsResource), &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_COMMAND_BUFFER tileRowBRCBatchBuf;
    MOS_ZeroMemory(&tileRowBRCBatchBuf, sizeof(tileRowBRCBatchBuf));
    tileRowBRCBatchBuf.pCmdBase = tileRowBRCBatchBuf.pCmdPtr = (uint32_t *)data;
    tileRowBRCBatchBuf.iRemaining = m_hwInterface->m_hucCommandBufferSize;

    // Add batch buffer start for tile row BRC batch
    HalOcaInterface::OnSubLevelBBStart(*cmdBuffer, *m_osInterface->pOsContext, &m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow].OsResource, 0, true, 0);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow]));

    // load kernel from WOPCM into L2 storage RAM
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS imemParams;
    MOS_ZeroMemory(&imemParams, sizeof(imemParams));

    if (m_hevcSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW)  // Low Delay BRC
    {
        imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcLowdelayKernelDescriptor;
    }
    else
    {
        imemParams.dwKernelDescriptor = m_vdboxHucHevcBrcUpdateKernelDescriptor;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucImemStateCmd(&tileRowBRCBatchBuf, &imemParams));

    // pipe mode select
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucPipeModeSelectCmd(&tileRowBRCBatchBuf, &pipeModeSelectParams));

    // DMEM set
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetDmemHuCBrcUpdate());

    MHW_VDBOX_HUC_DMEM_STATE_PARAMS dmemParams;
    MOS_ZeroMemory(&dmemParams, sizeof(dmemParams));
    dmemParams.presHucDataSource = &(m_vdencBrcUpdateDmemBuffer[m_currRecycledBufIdx][m_CurrentPassForOverAll]);
    dmemParams.dwDataLength = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
    dmemParams.dwDmemOffset = HUC_DMEM_OFFSET_RTOS_GEMS;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucDmemStateCmd(&tileRowBRCBatchBuf, &dmemParams));

    // Set Const Data buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetConstDataHuCBrcUpdate());

    // Add Virtual addr
    MHW_VDBOX_HUC_VIRTUAL_ADDR_PARAMS virtualAddrParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetRegionsHuCTileRowBrcUpdate(&virtualAddrParams));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucVirtualAddrStateCmd(&tileRowBRCBatchBuf, &virtualAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hucInterface->AddHucStartCmd(&tileRowBRCBatchBuf, true));

    // wait Huc completion (use HEVC bit for now)
    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS vdPipeFlushParams;
    MOS_ZeroMemory(&vdPipeFlushParams, sizeof(vdPipeFlushParams));
    vdPipeFlushParams.Flags.bFlushHEVC = 1;
    vdPipeFlushParams.Flags.bWaitDoneHEVC = 1;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdPipelineFlushCmd(&tileRowBRCBatchBuf, &vdPipeFlushParams));

    // Flush the engine to ensure memory written out
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiFlushDwCmd(&tileRowBRCBatchBuf, &flushDwParams));

    // Write HUC_STATUS mask: DW1 (mask value)
    MHW_MI_STORE_DATA_PARAMS storeDataParams;
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resPakMmioBuffer;
    storeDataParams.dwResourceOffset = sizeof(uint32_t);
    storeDataParams.dwValue = CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&tileRowBRCBatchBuf, &storeDataParams));

    // store HUC_STATUS register: DW0 (actual value)
    CODECHAL_ENCODE_CHK_COND_RETURN((m_vdboxIndex > m_mfxInterface->GetMaxVdboxIndex()), "ERROR - vdbox index exceed the maximum");
    auto mmioRegisters = m_hucInterface->GetMmioRegisters(m_vdboxIndex);
    MHW_MI_STORE_REGISTER_MEM_PARAMS storeRegParams;
    MOS_ZeroMemory(&storeRegParams, sizeof(storeRegParams));
    storeRegParams.presStoreBuffer = &m_resPakMmioBuffer;
    storeRegParams.dwOffset = 0;
    storeRegParams.dwRegister = mmioRegisters->hucStatusRegOffset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreRegisterMemCmd(&tileRowBRCBatchBuf, &storeRegParams));

    
    // Set the tile row BRC update sync semaphore 
    MOS_ZeroMemory(&storeDataParams, sizeof(storeDataParams));
    storeDataParams.pOsResource = &m_resTileRowBRCsyncSemaphore;
    storeDataParams.dwValue     = 0xFF;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&tileRowBRCBatchBuf, &storeDataParams));

    (&m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow])->iCurrent = tileRowBRCBatchBuf.iOffset;
    (&m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow])->iRemaining = tileRowBRCBatchBuf.iRemaining;
    (&m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow])->pData = data;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(nullptr, &m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow]));

    if (data)
    {
        m_osInterface->pfnUnlockResource(m_osInterface, &(m_TileRowBRCBatchBuffer[m_CurrentPassForTileReplay][m_CurrentTileRow].OsResource));
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
            (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

        memset(attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
        attriExt->bUseVirtualEngineHint =
            attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    SendKernelCmdsParams *params)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VFE_PARAMS_G12 vfeParams = {};
    vfeParams.pKernelState              = params->pKernelState;
    vfeParams.eVfeSliceDisable          = MHW_VFE_SLICE_ALL;
    vfeParams.dwMaximumNumberofThreads  = m_encodeVfeMaxThreads;
    vfeParams.bFusedEuDispatch          = false; // legacy mode

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

void CodechalVdencHevcStateG12::SetStreaminDataPerLcu(
    PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
    void* streaminData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G12 data = (PCODECHAL_VDENC_HEVC_STREAMIN_STATE_G12)streaminData;
    if (streaminParams->setQpRoiCtrl)
    {
        if (m_vdencNativeROIEnabled)
        {
            data->DW0.RoiCtrl = streaminParams->roiCtrl;
        }
        else
        {
            data->DW7.QpEnable = 0xf;
            data->DW14.ForceQp_0 = data->DW14.ForceQp_1 = data->DW14.ForceQp_2 = data->DW14.ForceQp_3 = streaminParams->forceQp;
        }
    }
    else
    {
        data->DW0.MaxTuSize = streaminParams->maxTuSize;
        data->DW0.MaxCuSize = streaminParams->maxCuSize;
        data->DW0.NumImePredictors = streaminParams->numImePredictors;
        data->DW0.PuTypeCtrl = streaminParams->puTypeCtrl;
        data->DW6.NumMergeCandidateCu64x64 = streaminParams->numMergeCandidateCu64x64;
        data->DW6.NumMergeCandidateCu32x32 = streaminParams->numMergeCandidateCu32x32;
        data->DW6.NumMergeCandidateCu16x16 = streaminParams->numMergeCandidateCu16x16;
        data->DW6.NumMergeCandidateCu8x8 = streaminParams->numMergeCandidateCu8x8;
    }
}

void CodechalVdencHevcStateG12::GetTileInfo(
    uint32_t xPosition,
    uint32_t yPosition,
    uint32_t* tileId,
    uint32_t* tileEndLCUX,
    uint32_t* tileEndLCUY)
{
    *tileId = 0;
    uint32_t ctbSize = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];

    for (uint8_t i = 0; i < m_numTiles; i++)
    {
        uint32_t tileWidthInLCU = MOS_ROUNDUP_DIVIDE(((tileParams[i].TileWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);
        uint32_t tileHeightInLCU = MOS_ROUNDUP_DIVIDE(((tileParams[i].TileHeightInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);
        *tileEndLCUX = tileParams[i].TileStartLCUX + tileWidthInLCU;
        *tileEndLCUY = tileParams[i].TileStartLCUY + tileHeightInLCU;

        if (xPosition >= (tileParams[i].TileStartLCUX * 2) &&
            yPosition >= (tileParams[i].TileStartLCUY * 2) &&
            xPosition < (*tileEndLCUX * 2) &&
            yPosition < (*tileEndLCUY * 2))
        {
            *tileId = i;
            break;
        }
    }
}

MOS_STATUS CodechalVdencHevcStateG12::PrepareVDEncStreamInData()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hevcPicParams->tiles_enabled_flag)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetTileData(m_tileParams[m_virtualEngineBbIndex]));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::PrepareVDEncStreamInData());

    return eStatus;
}

void CodechalVdencHevcStateG12::SetStreaminDataPerRegion(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS streaminParams,
    void* streaminData)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        CodechalVdencHevcState::SetStreaminDataPerRegion(streamInWidth, top, bottom, left, right, streaminParams, streaminData);
        return;
    }

    uint8_t* data = (uint8_t*)streaminData;
    uint32_t tileId = 0, tileEndLCUX = 0, tileEndLCUY = 0;
    uint32_t ctbSize = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    GetTileInfo(left, top, &tileId, &tileEndLCUX, &tileEndLCUY);

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];

    for (auto y = top; y < bottom; y++)
    {
        for (auto x = left; x < right; x++)
        {
            uint32_t streamInBaseOffset = 0, offset = 0, xyOffset = 0;

            if (x < (tileParams[tileId].TileStartLCUX * 2) ||
                y < (tileParams[tileId].TileStartLCUY * 2) ||
                x >= (tileEndLCUX * 2) ||
                y >= (tileEndLCUY * 2))
            {
                GetTileInfo(x, y, &tileId, &tileEndLCUX, &tileEndLCUY);
            }
            streamInBaseOffset = tileParams[tileId].TileStreaminOffset;

            auto xPositionInTile = x - (tileParams[tileId].TileStartLCUX * 2);
            auto yPositionInTile = y - (tileParams[tileId].TileStartLCUY * 2);
            auto tileWidthInLCU = MOS_ROUNDUP_DIVIDE(((tileParams[tileId].TileWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);

            StreaminZigZagToLinearMap(tileWidthInLCU * 2, xPositionInTile, yPositionInTile, &offset, &xyOffset);

            SetStreaminDataPerLcu(streaminParams, data + (streamInBaseOffset + offset + xyOffset) * 64);
        }
    }
}

void CodechalVdencHevcStateG12::SetBrcRoiDeltaQpMap(
    uint32_t streamInWidth,
    uint32_t top,
    uint32_t bottom,
    uint32_t left,
    uint32_t right,
    uint8_t regionId,
    PDeltaQpForROI deltaQpMap)
{

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!m_hevcPicParams->tiles_enabled_flag)
    {
        CodechalVdencHevcState::SetBrcRoiDeltaQpMap(streamInWidth, top, bottom, left, right, regionId, deltaQpMap);
        return;
    }

    uint32_t tileId = 0, tileEndLCUX = 0, tileEndLCUY = 0;
    uint32_t ctbSize = 1 << (m_hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    GetTileInfo(left, top, &tileId, &tileEndLCUX, &tileEndLCUY);

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];

    for (auto y = top; y < bottom; y++)
    {
        for (auto x = left; x < right; x++)
        {
            uint32_t streamInBaseOffset = 0, offset = 0, xyOffset = 0;

            if (x < (tileParams[tileId].TileStartLCUX * 2) ||
                y < (tileParams[tileId].TileStartLCUY * 2) ||
                x >= (tileEndLCUX * 2) ||
                y >= (tileEndLCUY * 2))
            {
                GetTileInfo(x, y, &tileId, &tileEndLCUX, &tileEndLCUY);
            }
            streamInBaseOffset = tileParams[tileId].TileStreaminOffset;

            auto xPositionInTile = x - (tileParams[tileId].TileStartLCUX * 2);
            auto yPositionInTile = y - (tileParams[tileId].TileStartLCUY * 2);
            auto tileWidthInLCU = MOS_ROUNDUP_DIVIDE(((tileParams[tileId].TileWidthInMinCbMinus1 + 1) << (m_hevcSeqParams->log2_min_coding_block_size_minus3 + 3)), ctbSize);

            StreaminZigZagToLinearMap(tileWidthInLCU * 2, xPositionInTile, yPositionInTile, &offset, &xyOffset);

            (deltaQpMap + (streamInBaseOffset + offset + xyOffset))->iDeltaQp = m_hevcPicParams->ROI[regionId].PriorityLevelOrDQp;
        }
    }
}

MOS_STATUS CodechalVdencHevcStateG12::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    CODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS scalSetParms;
    MOS_ZeroMemory(&scalSetParms, sizeof(CODECHAL_ENCODE_SCALABILITY_SETHINT_PARMS));

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        scalSetParms.bNeedSyncWithPrevious = true;
    }

    int32_t currentPass = GetCurrentPass();
    if (currentPass < 0 || currentPass >= CODECHAL_HEVC_MAX_NUM_BRC_PASSES)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }
    uint8_t passIndex = m_singleTaskPhaseSupported ? 0 : currentPass;
    if (m_numPipe >= 2)
    {
        for (auto i = 0; i < m_numPipe; i++)
        {
            scalSetParms.veBatchBuffer[i] = m_veBatchBuffer[m_virtualEngineBbIndex][i][passIndex].OsResource;
        }
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_SetHintParams(this, m_scalabilityState, &scalSetParms));
    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeScalability_PopulateHintParams(m_scalabilityState, cmdBuffer));

    return eStatus;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalVdencHevcStateG12::DumpVdencOutputs()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalVdencHevcState::DumpVdencOutputs());

    if (m_hevcPicParams->tiles_enabled_flag)
    {
        PMOS_RESOURCE presVdencTileStatisticsBuffer = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;
        auto          num_tiles                     = (m_hevcPicParams->num_tile_rows_minus1 + 1) * (m_hevcPicParams->num_tile_columns_minus1 + 1);
        auto          vdencStatsSizeAllTiles        = num_tiles * m_vdencBrcStatsBufferSize;
        auto          vdencStatsOffset              = m_hevcTileStatsOffset.uiVdencStatistics;
        auto          pakStatsSizeAllTiles          = num_tiles * 9 * CODECHAL_CACHELINE_SIZE;
        auto          pakStatsOffset                = m_hevcTileStatsOffset.uiHevcPakStatistics;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            presVdencTileStatisticsBuffer,
            CodechalDbgAttr::attrVdencOutput,
            "_TileVDEncStats",
            vdencStatsSizeAllTiles,
            vdencStatsOffset,
            CODECHAL_NUM_MEDIA_STATES));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            presVdencTileStatisticsBuffer,
            CodechalDbgAttr::attrPakOutput,
            "_TilePAKStats",
            pakStatsSizeAllTiles,
            pakStatsOffset,
            CODECHAL_NUM_MEDIA_STATES));

        // Slice Size Conformance
        if (m_hevcSeqParams->SliceSizeControl)
        {
            PMOS_RESOURCE presLcuBaseAddressBuffer = &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource;
            auto          sliceStreamoutOffset     = m_hevcTileStatsOffset.uiHevcSliceStreamout;
            uint32_t size = m_numLcu * CODECHAL_CACHELINE_SIZE;
            // Slice Size StreamOut Surface
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                presLcuBaseAddressBuffer,
                CodechalDbgAttr::attrVdencOutput,
                "_SliceSize",
                size,
                sliceStreamoutOffset,
                CODECHAL_NUM_MEDIA_STATES));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcStateG12::DumpHucDebugOutputBuffers()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // Virtual Engine does only one submit per pass. Dump all HuC debug outputs
    bool dumpDebugBuffers = IsLastPipe() && (m_numPipe > 1);
    if (m_singleTaskPhaseSupported)
    {
        dumpDebugBuffers = dumpDebugBuffers && IsLastPass();
    }

    if (dumpDebugBuffers)
    {
        CODECHAL_DEBUG_TOOL(
            DumpHucPakIntegrate();
            DumpHucCqp();
           )
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::DumpHucPakIntegrate()
{
    int32_t currentPass = GetCurrentPass();
    // HuC Input
    // HuC DMEM
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucDmem(
        &m_resHucPakStitchDmemBuffer[m_currRecycledBufIdx][currentPass],
        MOS_ALIGN_CEIL(sizeof(HucPakStitchDmemVdencG12), CODECHAL_CACHELINE_SIZE),
        currentPass,
        hucRegionDumpPakIntegrate));

    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].sResource,
        0,
        m_resTileBasedStatisticsBuffer[m_virtualEngineBbIndex].dwSize,
        0,
        "",
        true,
        currentPass,
        hucRegionDumpPakIntegrate));

    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_resHuCPakAggregatedFrameStatsBuffer.sResource,
        0,
        m_resHuCPakAggregatedFrameStatsBuffer.dwSize,
        1,
        "",
        false,
        currentPass,
        hucRegionDumpPakIntegrate));

    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 tileParams = m_tileParams[m_virtualEngineBbIndex];
    CODECHAL_ENCODE_CHK_NULL_RETURN(tileParams);

    auto bitStreamSize = m_encodeParams.dwBitstreamSize - 
        MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE);

    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_resBitstreamBuffer,
        MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE),
        bitStreamSize,
        4,
        "",
        true,
        currentPass,
        hucRegionDumpPakIntegrate));

    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_resBitstreamBuffer,
        MOS_ALIGN_FLOOR(tileParams[m_numTiles - 1].BitstreamByteOffset * CODECHAL_CACHELINE_SIZE, CODECHAL_PAGE_SIZE),
        bitStreamSize,
        5,
        "",
        false,
        currentPass,
        hucRegionDumpPakIntegrate));

    // Region 6 - BRC History buffer
    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_vdencBrcHistoryBuffer,
        0,
        CODECHAL_VDENC_HEVC_BRC_HISTORY_BUF_SIZE,
        6,
        "",
        false,
        currentPass,
        hucRegionDumpPakIntegrate));

    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_thirdLevelBatchBuffer.OsResource,
        0,
        m_thirdLBSize,
        7,
        "",
        true,
        currentPass,
        hucRegionDumpPakIntegrate));

    // Region 8
    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_resHucStitchDataBuffer[m_currRecycledBufIdx][currentPass],
        0,
        MOS_ALIGN_CEIL(sizeof(HucCommandDataVdencG12), CODECHAL_PAGE_SIZE),
        8,
        "",
        true,
        currentPass,
        hucRegionDumpPakIntegrate));

    // Region 9 - HCP BRC Data Output
    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_resBrcDataBuffer,
        0,
        CODECHAL_CACHELINE_SIZE,
        9,
        "",
        false,
        currentPass,
        hucRegionDumpPakIntegrate));

    // Region 10
    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_HucStitchCmdBatchBuffer.OsResource,
        0,
        m_hwInterface->m_HucStitchCmdBatchBufferSize,
        10,
        "",
        false,
        currentPass,
        hucRegionDumpPakIntegrate));

    CODECHAL_DEBUG_CHK_STATUS(m_debugInterface->DumpHucRegion(
        &m_tileRecordBuffer[m_virtualEngineBbIndex].sResource,
        0,
        m_tileRecordBuffer[m_virtualEngineBbIndex].dwSize,
        15,
        "",
        true,
        currentPass,
        hucRegionDumpPakIntegrate));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalVdencHevcStateG12::DumpHucCqp()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
    int32_t currentPass = GetCurrentPass();

    // Region 5 - Output SLB Buffer
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpHucRegion(
        &m_vdenc2ndLevelBatchBuffer[m_currRecycledBufIdx].OsResource,
        0,
        m_hwInterface->m_vdenc2ndLevelBatchBufferSize,
        5,
        "_Out_Slb",
        false,
        currentPass,
        hucRegionDumpUpdate));

    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS CodechalVdencHevcStateG12::SetRoundingValues()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingIntra)
    {
        m_roundIntraValue = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetIntra;
    }
    else
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            m_roundIntraValue = 10;
        }
        else if (m_hevcSeqParams->HierarchicalFlag && m_hevcPicParams->HierarchLevelPlus1 > 0)
        {
            //Hierachical GOP
            if (m_hevcPicParams->HierarchLevelPlus1 == 1)
            {
                m_roundIntraValue = 10;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
            {
                m_roundIntraValue = 9;
            }
            else
            {
                m_roundIntraValue = 8;
            }
        }
        else
        {
            m_roundIntraValue = 10;
        }
    }

    if (m_hevcPicParams->CustomRoundingOffsetsParams.fields.EnableCustomRoudingInter)
    {
        m_roundInterValue = m_hevcPicParams->CustomRoundingOffsetsParams.fields.RoundingOffsetInter;
    }
    else
    {
        if (m_hevcPicParams->CodingType == I_TYPE)
        {
            m_roundInterValue = 4;
        }
        else if (m_hevcSeqParams->HierarchicalFlag && m_hevcPicParams->HierarchLevelPlus1 > 0)
        {
            //Hierachical GOP
            if (m_hevcPicParams->HierarchLevelPlus1 == 1)
            {
                m_roundInterValue = 4;
            }
            else if (m_hevcPicParams->HierarchLevelPlus1 == 2)
            {
                m_roundInterValue = 3;
            }
            else
            {
                m_roundInterValue = 2;
            }
        }
        else
        {
            m_roundInterValue = 4;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalVdencHevcStateG12::SetAddCommands(uint32_t commandType, PMOS_COMMAND_BUFFER cmdBuffer, bool addToBatchBufferHuCBRC, uint32_t roundInterValue, uint32_t roundIntraValue, bool isLowDelayB, int8_t * pRefIdxMapping, int8_t recNotFilteredID)
{
#ifdef _HEVC_ENCODE_VDENC_SUPPORTED
    void *pCmdParams = nullptr;

    if (commandType == CODECHAL_CMD1)
    {
        // Send CMD1 command
        MHW_VDBOX_VDENC_CMD1_PARAMS  cmd1Params;
        MOS_ZeroMemory(&cmd1Params, sizeof(cmd1Params));
        cmd1Params.Mode = CODECHAL_ENCODE_MODE_HEVC;
        cmd1Params.pHevcEncPicParams = m_hevcPicParams;
        cmd1Params.pHevcEncSlcParams = m_hevcSliceParams;
        cmd1Params.pInputParams      = pCmdParams;
        cmd1Params.bHevcVisualQualityImprovement = m_hevcVisualQualityImprovement;
        //down cast?
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencCmd1Cmd(cmdBuffer, nullptr, &cmd1Params));
    }
    else if (commandType == CODECHAL_CMD2)
    {
        PMHW_VDBOX_VDENC_CMD2_STATE cmd2Params(new MHW_VDBOX_VDENC_CMD2_STATE);
        CODECHAL_ENCODE_CHK_NULL_RETURN(cmd2Params);

        // set CMD2 command
        cmd2Params->Mode = CODECHAL_ENCODE_MODE_HEVC;
        cmd2Params->pHevcEncSeqParams = m_hevcSeqParams;
        cmd2Params->pHevcEncPicParams = m_hevcPicParams;
        cmd2Params->pHevcEncSlcParams = m_hevcSliceParams;
        cmd2Params->bRoundingEnabled  = m_hevcVdencRoundingEnabled;
        cmd2Params->bPakOnlyMultipassEnable = m_pakOnlyPass;
        cmd2Params->bUseDefaultQpDeltas     = (m_hevcVdencAcqpEnabled && cmd2Params->pHevcEncSeqParams->QpAdjustment ) || 
                                                      (m_brcEnabled && cmd2Params->pHevcEncSeqParams->MBBRC != mbBrcDisabled);
        cmd2Params->bPanicEnabled           = (m_brcEnabled) && (m_panicEnable) && (IsLastPass()) && !m_pakOnlyPass;
        cmd2Params->bStreamInEnabled        = m_vdencStreamInEnabled;
        cmd2Params->bROIStreamInEnabled     = m_vdencNativeROIEnabled;
        cmd2Params->bTileReplayEnable       = m_enableTileReplay;
        cmd2Params->bIsLowDelayB            = isLowDelayB;
        cmd2Params->bCaptureModeEnable      = m_CaptureModeEnable;
        cmd2Params->m_WirelessSessionID     = 0;
        cmd2Params->pRefIdxMapping          = pRefIdxMapping;
        cmd2Params->recNotFilteredID      = recNotFilteredID;
        cmd2Params->pInputParams            = pCmdParams;
        cmd2Params->ucNumRefIdxL0ActiveMinus1 = cmd2Params->pHevcEncSlcParams->num_ref_idx_l0_active_minus1;
        cmd2Params->bHevcVisualQualityImprovement = m_hevcVisualQualityImprovement;
        cmd2Params->roundInterValue = roundInterValue;
        cmd2Params->roundIntraValue = roundIntraValue;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_vdencInterface->AddVdencCmd2Cmd(cmdBuffer, nullptr, cmd2Params));
    }
#endif
    return MOS_STATUS_SUCCESS;
}
