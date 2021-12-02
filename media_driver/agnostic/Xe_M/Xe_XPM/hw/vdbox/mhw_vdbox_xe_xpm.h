/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_xe_xpm.h
//! \brief    Defines structures used for constructing VDBox commands on XeHP
//!

#ifndef _MHW_VDBOX_XE_XPM_H_
#define _MHW_VDBOX_XE_XPM_H_

#include "mhw_vdbox_g12_X.h"

static constexpr uint32_t RD_MODE_INTRA_MPM    = 0;
static constexpr uint32_t RD_MODE_INTRA_16X16  = 1;
static constexpr uint32_t RD_MODE_INTRA_8X8    = 2;
static constexpr uint32_t RD_MODE_INTRA_4X4    = 3;
static constexpr uint32_t RD_MODE_INTER_16X8   = 4;
static constexpr uint32_t RD_MODE_INTER_8X8    = 5;
static constexpr uint32_t RD_MODE_INTER_8X4    = 6;
static constexpr uint32_t RD_MODE_INTER_16X16  = 7;
static constexpr uint32_t RD_MODE_INTER_BWD    = 8;
static constexpr uint32_t RD_MODE_REF_ID       = 9;
static constexpr uint32_t RD_MODE_INTRA_CHROMA = 10;
static constexpr uint32_t RD_MODE_SKIP_16X16   = 11;
static constexpr uint32_t RD_MODE_DIRECT_16X16 = 12;
static constexpr uint32_t NUM_RD_MODE_COST     = 13;

static constexpr uint32_t LUTMODE_INTRA_NONPRED = 0;
static constexpr uint32_t LUTMODE_INTRA         = 1;
static constexpr uint32_t LUTMODE_INTRA_16x16   = 1;
static constexpr uint32_t LUTMODE_INTRA_8x8     = 2;
static constexpr uint32_t LUTMODE_INTRA_4x4     = 3;
static constexpr uint32_t LUTMODE_INTER_BWD     = 9;
static constexpr uint32_t LUTMODE_REF_ID        = 10;
static constexpr uint32_t LUTMODE_INTRA_CHROMA  = 11;
static constexpr uint32_t LUTMODE_INTER         = 8;
static constexpr uint32_t LUTMODE_INTER_16x16   = 8;
static constexpr uint32_t LUTMODE_INTER_16x8    = 4;
static constexpr uint32_t LUTMODE_INTER_8x16    = 4;
static constexpr uint32_t LUTMODE_INTER_8x8q    = 5;
static constexpr uint32_t LUTMODE_INTER_8x4q    = 6;
static constexpr uint32_t LUTMODE_INTER_4x8q    = 6;
static constexpr uint32_t LUTMODE_INTER_4x4q    = 7;
static constexpr uint32_t LUTMODE_SKIP_16x16    = 12;
static constexpr uint32_t LUTMODE_DIRECT_16x16  = 13;

struct VDENC_COST_TABLE_XE_XPM
{
    const uint16_t HmeCost[8][52];
    const uint16_t VDEnc_MV_Cost[3][12];
    const double AVC_RD_Mode_Cost[3][NUM_RD_MODE_COST];
    const double VDEnc_Mode_Cost[3][12][52];
    const uint8_t qp_lambda[42];
};

struct MHW_VDBOX_AVC_IMG_PARAMS_XE_XPM : public MHW_VDBOX_AVC_IMG_PARAMS_G12
{
    const VDENC_COST_TABLE_XE_XPM   *vdencCostTable   = nullptr;
    bool                                colMVReadEnable  = false;
    bool                                colMVWriteEnable = false;
};
using PMHW_VDBOX_AVC_IMG_PARAMS_XE_XPM = MHW_VDBOX_AVC_IMG_PARAMS_XE_XPM *;

enum VDENC_XE_XPM_MAXTUSIZE
{
    VDENC_XE_XPM_MAXTUSIZE_4x4 = 0,
    VDENC_XE_XPM_MAXTUSIZE_8x8 = 1,
    VDENC_XE_XPM_MAXTUSIZE_16x16 = 2,
    VDENC_XE_XPM_MAXTUSIZE_32x32 = 3
};

enum VDENC_XE_XPM_MAXCUSIZE
{
    VDENC_XE_XPM_MAXCUSIZE_8x8 = 0,
    VDENC_XE_XPM_MAXCUSIZE_16x16 = 1,
    VDENC_XE_XPM_MAXCUSIZE_32x32 = 2,
    VDENC_XE_XPM_MAXCUSIZE_64x64 = 3
};


#endif
