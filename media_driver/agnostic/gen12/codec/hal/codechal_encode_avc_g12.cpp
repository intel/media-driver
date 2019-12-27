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
//! \file     codechal_encode_avc_g12.cpp
//! \brief    This file implements the C++ class/interface for Gen12 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!
#include "codechal_encode_avc_g12.h"
#include "codechal_mmc_encode_avc_g12.h"
#include "codechal_kernel_header_g12.h"
#include "codechal_kernel_hme_g12.h"
#include "mhw_render_g12_X.h"
#include "codeckrnheader.h"
#include "igcodeckrn_g12.h"
#include "media_user_settings_mgr_g12.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g12.h"
#include "mhw_vdbox_mfx_hwcmd_g12_X.h"
#include "mos_util_user_interface.h"
#endif

enum MbencBindingTableOffset
{
    mbencMfcAvcPakObj            = 0,
    mbencIndMvData               = 1,
    mbencBrcDistortion           = 2,  // For BRC distortion for I
    mbencCurrY                   = 3,
    mbencCurrUv                  = 4,
    mbencMbSpecificData          = 5,
    mbencAuxVmeOut               = 6,
    mbencRefpicselectL0          = 7,
    mbencMvDataFromMe            = 8,
    mbenc4xMeDistortion          = 9,
    mbencSlicemapData            = 10,
    mbencFwdMbData               = 11,
    mbencFwdMvData               = 12,
    mbencMbqp                    = 13,
    mbencMbbrcConstData          = 14,
    mbencVmeInterPredCurrPicIdx0 = 15,
    mbencVmeInterPredFwdPicIDX0  = 16,
    mbencVmeInterPredBwdPicIDX00 = 17,
    mbencVmeInterPredFwdPicIDX1  = 18,
    mbencVmeInterPredBwdPicIDX10 = 19,
    mbencVmeInterPredFwdPicIDX2  = 20,
    mbencReserved0               = 21,
    mbencVmeInterPredFwdPicIDX3  = 22,
    mbencReserved1               = 23,
    mbencVmeInterPredFwdPicIDX4  = 24,
    mbencReserved2               = 25,
    mbencVmeInterPredFwdPicIDX5  = 26,
    mbencReserved3               = 27,
    mbencVmeInterPredFwdPicIDX6  = 28,
    mbencReserved4               = 29,
    mbencVmeInterPredFwdPicIDX7  = 30,
    mbencReserved5               = 31,
    mbencVmeInterPredCurrPicIdx1 = 32,
    mbencVmeInterPredBwdPicIDX01 = 33,
    mbencReserved6               = 34,
    mbencVmeInterPredBwdPicIDX11 = 35,
    mbencReserved7               = 36,
    mbencMbStats                 = 37,
    mbencMadData                 = 38,
    mbencBrcCurbeData            = 39,
    mbencForceNonskipMbMap       = 40,
    mbEncAdv                     = 41,
    mbencSfdCostTable            = 42,
    mbencSwScoreboard            = 43,
    mbencNumSurfaces             = 44
};

enum WpBindingTableOffset
{
    wpInputRefSurface     = 0,
    wpOutputScaledSurface = 1,
    wpNumSurfaces         = 2
};

enum MbencIdOffset
{
    mbencIOffset      = 0,
    mbencPOffset      = 1,
    mbencBOffset      = 2,
    mbencFrameTypeNum = 3
};

// AVC MBEnc CURBE init data for TGL Kernel
const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeNormalIFrame[89] =
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
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeNormalIField[89] =
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
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeNormalPFrame[89] =
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
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeNormalPField[89] =
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
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeNormalBFrame[89] =
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
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeNormalBField[89] =
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
    0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

// AVC I_DIST CURBE init data for TGL Kernel
const uint32_t CodechalEncodeAvcEncG12::MbencCurbe::m_mbEncCurbeIFrameDist[89] =
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
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0xffffffff
};

class CodechalEncodeAvcEncG12::WpCurbe
{
   public:
    WpCurbe()
    {
        memset((void *)&m_wpCurbeCmd, 0, sizeof(WpCurbe));
    }

    struct
    {
        // DW0
        union
        {
            struct
            {
                uint32_t m_defaultWeight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_defaultOffset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t m_roi0XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi0YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t m_roi0XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi0YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t m_roi0Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi0Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t m_roi1XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi1YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t m_roi1XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi1YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t m_roi1Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi1Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t m_roi2XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi2YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW7;

        // DW8
        union
        {
            struct
            {
                uint32_t m_roi2XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi2YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t m_roi2Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi2Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t m_roi3XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi3YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t m_roi3XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi3YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t m_roi3Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi3Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t m_roi4XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi4YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t m_roi4XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi4YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t m_roi4Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi4Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW15;

        // DW16
        union
        {
            struct
            {
                uint32_t m_roi5XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi5YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                uint32_t m_roi5XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi5YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                uint32_t m_roi5Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi5Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                uint32_t m_roi6XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi6YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                uint32_t m_roi6XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi6YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                uint32_t m_roi6Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi6Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                uint32_t m_roi7XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi7YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                uint32_t m_roi7XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi7YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                uint32_t m_roi7Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi7Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                uint32_t m_roi8XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi8YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                uint32_t m_roi8XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi8YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                uint32_t m_roi8Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi8Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                uint32_t m_roi9XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi9YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                uint32_t m_roi9XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi9YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW29;

        // DW30
        union
        {
            struct
            {
                uint32_t m_roi9Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi9Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t m_roi10XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi10YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW31;

        // DW32
        union
        {
            struct
            {
                uint32_t m_roi10XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi10YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t m_roi10Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi10Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t m_roi11XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi11YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t m_roi11XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi11YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t m_roi11Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi11Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t m_roi12XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi12YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t m_roi12XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi12YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t m_roi12Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi12Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t m_roi13XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi13YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t m_roi13XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi13YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t m_roi13Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi13Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t m_roi14XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi14YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t m_roi14XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi14YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t m_roi14Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi14Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t m_roi15XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi15YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t m_roi15XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi15YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW47;

        // DW48
        union
        {
            struct
            {
                uint32_t m_roi15Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi15Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW48;

        // DW49
        union
        {
            struct
            {
                uint32_t m_inputSurface;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW49;

        // DW50
        union
        {
            struct
            {
                uint32_t m_outputSurface;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW50;
    } m_wpCurbeCmd;
};

static const uint32_t trellisQuantizationRounding[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 0, 0, 0, 0, 0, 0
};

const uint8_t CodechalEncodeAvcEncG12::m_QPAdjustmentDistThresholdMaxFrameThresholdIPB[576] =
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

const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD CodechalEncodeAvcEncG12::m_IPCMThresholdTable[5] =
{
    { 2, 3000 },
    { 4, 3600 },
    { 6, 5000 },
    { 10, 7500 },
    { 18, 9000 },
};

const uint32_t CodechalEncodeAvcEncG12::m_intraModeCostForHighTextureMB[CODEC_AVC_NUM_QP]
{
    0x00000303, 0x00000304, 0x00000404, 0x00000405, 0x00000505, 0x00000506, 0x00000607, 0x00000708,
    0x00000809, 0x0000090a, 0x00000a0b, 0x00000b0c, 0x00000c0e, 0x00000e18, 0x00001819, 0x00001918,
    0x00001a19, 0x00001b19, 0x00001d19, 0x00001e18, 0x00002818, 0x00002918, 0x00002a18, 0x00002b19,
    0x00002d18, 0x00002e18, 0x00003818, 0x00003918, 0x00003a18, 0x00003b0f, 0x00003d0e, 0x00003e0e,
    0x0000480e, 0x0000490e, 0x00004a0e, 0x00004b0d, 0x00004d0d, 0x00004e0d, 0x0000580e, 0x0000590e,
    0x00005a0e, 0x00005b0d, 0x00005d0c, 0x00005e0b, 0x0000680a, 0x00006908, 0x00006a09, 0x00006b0a,
    0x00006d0b, 0x00006e0d, 0x0000780e, 0x00007918
};

const uint16_t CodechalEncodeAvcEncG12::m_lambdaData[256] = {
    9,     7,     9,     6,    12,     8,    12,     8,    15,    10,    15,     9,    19,    13,    19,    12,    24,
    17,    24,    15,    30,    21,    30,    19,    38,    27,    38,    24,    48,    34,    48,    31,    60,    43,
    60,    39,    76,    54,    76,    49,    96,    68,    96,    62,   121,    85,   121,    78,   153,   108,   153,
    99,   193,   135,   193,   125,   243,   171,   243,   157,   306,   215,   307,   199,   385,   271,   387,   251,
    485,   342,   488,   317,   612,   431,   616,   400,   771,   543,   777,   505,   971,   684,   981,   638,  1224,
    862,  1237,   806,  1542,  1086,  1562,  1018,  1991,  1402,  1971,  1287,  2534,  1785,  2488,  1626,  3077,  2167,
    3141,  2054,  3982,  2805,  3966,  2596,  4887,  3442,  5007,  3281,  6154,  4335,  6322,  4148,  7783,  5482,  7984,
    5243,  9774,  6885, 10082,  6629, 12489,  8797, 12733,  8382, 15566, 10965, 16082, 10599, 19729, 13897, 20313, 13404,
    24797, 17467, 25660, 16954, 31313, 22057, 32415, 21445, 39458, 27795, 40953, 27129, 49594, 34935, 51742, 34323, 61440,
    43987, 61440, 43428, 61440, 55462, 61440, 54954, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
    61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
    61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440, 61440,
    61440, 61440, 61440, 61440,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
    0,
};

const uint8_t CodechalEncodeAvcEncG12::m_ftQ25[64] = //27 value 4 dummy
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

// AVC MBEnc RefCost tables, index [CodingType][QP]
// QP is from 0 - 51, pad it to 64 since BRC needs each subarray size to be 128bytes
const uint16_t CodechalEncodeAvcEncG12::m_refCostMultiRefQp[NUM_PIC_TYPES][64] =
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

const uint32_t CodechalEncodeAvcEncG12::m_multiPred[NUM_TARGET_USAGE_MODES] =
{
    0, 3, 3, 0, 0, 0, 0, 0
};

const uint32_t CodechalEncodeAvcEncG12::m_multiRefDisableQPCheck[NUM_TARGET_USAGE_MODES] =
{
    0, 1, 0, 0, 0, 0, 0, 0
};
// clang-format on

const int32_t CodechalEncodeAvcEncG12::m_brcBTCounts[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    frameBrcUpdateNumSurfaces,
    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_NUM_SURFACES,
    mbencNumSurfaces,
    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_NUM_SURFACES,
    mbBrcUpdateNumSurfaces
};

const int32_t CodechalEncodeAvcEncG12::m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM] = {
    (sizeof(BrcInitResetCurbe)),
    (sizeof(FrameBrcUpdateCurbe)),
    (sizeof(BrcInitResetCurbe)),
    (sizeof(MbencCurbe)),
    0,
    (sizeof(MbBrcUpdateCurbe))
};

class CodechalEncodeAvcEncG12::EncKernelHeader
{
public:
    int m_kernelCount;
    // Quality mode for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncQltyI;
    CODECHAL_KERNEL_HEADER m_mbEncQltyP;
    CODECHAL_KERNEL_HEADER m_mEncQltyB;
    // Normal mode for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncNormI;
    CODECHAL_KERNEL_HEADER m_mbEncNormP;
    CODECHAL_KERNEL_HEADER m_mbEncNormB;
    // Performance modes for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncPerfI;
    CODECHAL_KERNEL_HEADER m_mbEncPerfP;
    CODECHAL_KERNEL_HEADER m_mbEncPerfB;
    // Modes for Frame/Field
    CODECHAL_KERNEL_HEADER m_mbEncAdvI;
    CODECHAL_KERNEL_HEADER m_mbEncAdvP;
    CODECHAL_KERNEL_HEADER m_mbEncAdvB;
    // BRC Init frame
    CODECHAL_KERNEL_HEADER m_initFrameBrc;
    // FrameBRC Update
    CODECHAL_KERNEL_HEADER m_frameEncUpdate;
    // BRC Reset frame
    CODECHAL_KERNEL_HEADER m_brcResetFrame;
    // BRC I Frame Distortion
    CODECHAL_KERNEL_HEADER m_brcIFrameDist;
    // BRCBlockCopy
    CODECHAL_KERNEL_HEADER m_brcBlockCopy;
    // MbBRC Update
    CODECHAL_KERNEL_HEADER m_mbBrcUpdate;
    //Weighted Prediction Kernel
    CODECHAL_KERNEL_HEADER m_weightedPrediction;
    // SW scoreboard initialization kernel
    CODECHAL_KERNEL_HEADER m_initSwScoreboard;
};

MOS_STATUS CodechalEncodeAvcEncG12::GetKernelHeaderAndSize(
    void                         *binary,
    EncOperation                 operation,
    uint32_t                     krnStateIdx,
    void                         *krnHeader,
    uint32_t                     *krnSize)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    CODECHAL_ENCODE_CHK_NULL_RETURN(binary);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnHeader);
    CODECHAL_ENCODE_CHK_NULL_RETURN(krnSize);

    auto kernelHeaderTable = (EncKernelHeader *)binary;
    auto invalidEntry = &(kernelHeaderTable->m_weightedPrediction) + 1;
    auto nextKrnOffset = *krnSize;

    PCODECHAL_KERNEL_HEADER currKrnHeader = nullptr;

    if (operation == ENC_BRC)
    {
        currKrnHeader = &kernelHeaderTable->m_initFrameBrc;
    }
    else if (operation == ENC_MBENC)
    {
        currKrnHeader = &kernelHeaderTable->m_mbEncQltyI;
    }
    else if (operation == ENC_MBENC_ADV)
    {
        currKrnHeader = &kernelHeaderTable->m_mbEncAdvI;
    }
    else if (operation == ENC_WP)
    {
        currKrnHeader = &kernelHeaderTable->m_weightedPrediction;
    }
    else
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Unsupported ENC mode requested");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    currKrnHeader += krnStateIdx;
    *((PCODECHAL_KERNEL_HEADER)krnHeader) = *currKrnHeader;

    auto nextKrnHeader = (currKrnHeader + 1);
    if (nextKrnHeader < invalidEntry)
    {
        nextKrnOffset = nextKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT;
    }
    *krnSize = nextKrnOffset - (currKrnHeader->KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);

    return eStatus;
}

CodechalEncodeAvcEncG12::CodechalEncodeAvcEncG12(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEnc(hwInterface, debugInterface, standardInfo), 
        m_sinlgePipeVeState(nullptr)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_ASSERT(m_osInterface);

    Mos_CheckVirtualEngineSupported(m_osInterface, false, true);

    // Virtual Engine is enabled in default.
    Mos_SetVirtualEngineSupported(m_osInterface, true);

    bKernelTrellis             = true;
    bExtendedMvCostRange       = true;
    bBrcSplitEnable            = true;
    bDecoupleMbEncCurbeFromBRC = true;
    bHighTextureModeCostEnable = true;
    bMvDataNeededByBRC         = false;

    this->pfnGetKernelHeaderAndSize = CodechalEncodeAvcEncG12::GetKernelHeaderAndSize;

    m_cmKernelEnable         = true;
    m_mbStatsSupported       = true;
    m_useCommonKernel        = true;

    m_kernelBase = (uint8_t *)IGCODECKRN_G12;
    m_kuidCommon = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
    AddIshSize(m_kuid, m_kernelBase);
    AddIshSize(m_kuidCommon, m_kernelBase);

    m_vdboxOneDefaultUsed = true;

    Mos_CheckVirtualEngineSupported(m_osInterface, false, false);

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG12, this));
        CreateAvcPar();
    )
}

CodechalEncodeAvcEncG12::~CodechalEncodeAvcEncG12()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_Delete(m_intraDistKernel);

    if (m_swScoreboardState)
    {
        MOS_Delete(m_swScoreboardState);
        m_swScoreboardState = nullptr;
    }

    if (m_sinlgePipeVeState)
    {
        MOS_FreeMemAndSetNull(m_sinlgePipeVeState);
    }

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

MOS_STATUS CodechalEncodeAvcEncG12::MbEncKernel(bool mbEncIFrameDistInUse)
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t ppsIdx          = m_avcSliceParams->pic_parameter_set_id;
    uint8_t spsIdx          = m_avcPicParams[ppsIdx]->seq_parameter_set_id;
    auto refList            = &m_refList[0];
    auto currRefList        = m_refList[m_currReconstructedPic.FrameIdx];
    bool use45DegreePattern = false;
    bool roiEnabled         = (m_avcPicParams[ppsIdx]->NumROI > 0) ? true : false;
    uint8_t refPicListIdx   = m_avcSliceParams[ppsIdx].RefPicList[0][0].FrameIdx;
    uint8_t refFrameListIdx = m_avcPicParam[ppsIdx].RefFrameList[refPicListIdx].FrameIdx;
    bool dirtyRoiEnabled    = (m_pictureCodingType == P_TYPE
        && m_avcPicParams[ppsIdx]->NumDirtyROI > 0
        && m_prevReconFrameIdx == refFrameListIdx);

    //  Two flags(bMbConstDataBufferNeeded, bMbQpBufferNeeded)
    //  would be used as there are two buffers and not all cases need both the buffers
    //  Constant Data buffer  needed for MBBRC, MBQP, ROI, RollingIntraRefresh
    //  Please note that this surface needs to be programmed for
    //  all usage cases(including CQP cases) because DWord13 includes mode cost for high texture MB?s cost.
    bool mbConstDataBufferInUse = bMbBrcEnabled || bMbQpDataEnabled || roiEnabled || dirtyRoiEnabled ||
        m_avcPicParam->EnableRollingIntraRefresh || bHighTextureModeCostEnable;

    bool mbQpBufferInUse = bMbBrcEnabled || bBrcRoiEnabled || bMbQpDataEnabled;

    if (m_feiEnable)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_avcFeiPicParams);
        mbConstDataBufferInUse |= m_avcFeiPicParams->bMBQp;
        mbQpBufferInUse |= m_avcFeiPicParams->bMBQp;
    }

    // MFE MBEnc kernel handles several frames from different streams in one submission.
    // All the streams use the same HW/OS/StateHeap interfaces during this submssion.
    // All the streams use the kernel state from the first stream.
    // The first stream allocates the DSH and SSH, send the binding table.
    // The last stream sets mfe curbe, prepare and submit the command buffer.
    // All the streams set their own curbe surfaces and surface states.
    CODECHAL_ENCODE_AVC_BINDING_TABLE_MBENC origMbEncBindingTable;
    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        auto mfeEncodeSharedState = m_mfeEncodeSharedState;
        if (m_mfeFirstStream)
        {
            mfeEncodeSharedState->pHwInterface = m_hwInterface;
            mfeEncodeSharedState->pOsInterface = m_osInterface;
            m_hwInterface->GetRenderInterface()->m_stateHeapInterface = m_stateHeapInterface;
            m_osInterface->pfnResetOsStates(m_osInterface);
        }
        else
        {
            m_hwInterface           = mfeEncodeSharedState->pHwInterface;
            m_osInterface           = mfeEncodeSharedState->pOsInterface;
            m_stateHeapInterface    = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;

            m_osInterface = m_osInterface;
        }
        // Set maximum width/height, it is used for initializing media walker parameters
        // during submitting the command buffer at the last stream.
        if (m_picWidthInMb > mfeEncodeSharedState->dwPicWidthInMB)
        {
            mfeEncodeSharedState->dwPicWidthInMB = m_picWidthInMb;
        }
        if (m_frameFieldHeightInMb > mfeEncodeSharedState->dwPicHeightInMB)
        {
            mfeEncodeSharedState->dwPicHeightInMB = m_frameFieldHeightInMb;
        }
        if (m_sliceHeight > mfeEncodeSharedState->sliceHeight)
        {
            mfeEncodeSharedState->sliceHeight = m_sliceHeight;
        }

        m_osInterface->pfnSetGpuContext(m_osInterface, m_renderContext);
        CODECHAL_DEBUG_TOOL(
            m_debugInterface->m_osInterface = m_osInterface;)
        // bookkeeping the original binding table
        origMbEncBindingTable = MbEncBindingTable;
    }

    PerfTagSetting perfTag;
    perfTag.Value             = 0;
    perfTag.Mode              = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType          = (mbEncIFrameDistInUse && !m_singleTaskPhaseSupported) ?
        CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST : CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType;
    if (mbEncIFrameDistInUse)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
    }
    else if (bUseMbEncAdvKernel)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_ADV;
    }
    else if (m_kernelMode == encodeNormalMode)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_NORMAL;
    }
    else if (m_kernelMode == encodePerformanceMode)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_PERFORMANCE;
    }
    else
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_QUALITY;
    }

    // Initialize DSH kernel region
    PMHW_KERNEL_STATE kernelState;
    if (mbEncIFrameDistInUse)
    {
        kernelState = &BrcKernelStates[CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST];
    }
    else if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        kernelState = &mfeMbEncKernelState;
    }
    else
    {
        CodechalEncodeIdOffsetParams idOffsetParams;
        MOS_ZeroMemory(&idOffsetParams, sizeof(idOffsetParams));
        idOffsetParams.Standard           = m_standard;
        idOffsetParams.EncFunctionType    = encFunctionType;
        idOffsetParams.wPictureCodingType = m_pictureCodingType;
        idOffsetParams.ucDmvPredFlag      = m_avcSliceParams->direct_spatial_mv_pred_flag;
        idOffsetParams.interlacedField   = CodecHal_PictureIsField(m_currOriginalPic);

        uint32_t krnStateIdx;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetMbEncKernelStateIdx(
            &idOffsetParams,
            &krnStateIdx));
        kernelState = &pMbEncKernelStates[krnStateIdx];
    }

    // All the streams use the kernel state from the first stream.
    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        if (m_mfeFirstStream)
        {
            m_mfeEncodeSharedState->pMfeMbEncKernelState = kernelState;
        }
        else
        {
            kernelState = m_mfeEncodeSharedState->pMfeMbEncKernelState;
        }
    }

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported ||
        (IsMfeMbEncEnabled(mbEncIFrameDistInUse) && m_mfeFirstStream))
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    // Allocate DSH and SSH for the first stream, which will be passed to other streams through the shared kernel state.
    if ((IsMfeMbEncEnabled(mbEncIFrameDistInUse) && m_mfeFirstStream) ||
        (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) && !bMbEncCurbeSetInBrcUpdate))
    {
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
    }

    if (bMbEncCurbeSetInBrcUpdate)
    {
        if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse))
        {
            // If BRC update was used to set up the DSH & SSH, SSH only needs to
            // be obtained if single task phase is enabled because the same SSH
            // could not be shared between BRC update and MbEnc
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
                m_stateHeapInterface,
                kernelState,
                true,
                0,
                m_singleTaskPhaseSupported,
                m_storeData));
        }
    }
    else
    {
        // Setup AVC Curbe
        CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS mbEncCurbeParams;
        MOS_ZeroMemory(&mbEncCurbeParams, sizeof(mbEncCurbeParams));
        mbEncCurbeParams.pPicParams              = m_avcPicParams[ppsIdx];
        mbEncCurbeParams.pSeqParams              = m_avcSeqParams[spsIdx];
        mbEncCurbeParams.pSlcParams              = m_avcSliceParams;
        mbEncCurbeParams.ppRefList               = &(m_refList[0]);
        mbEncCurbeParams.pPicIdx                 = &(m_picIdx[0]);
        mbEncCurbeParams.bRoiEnabled             = roiEnabled;
        mbEncCurbeParams.bDirtyRoiEnabled        = dirtyRoiEnabled;
        mbEncCurbeParams.bMbEncIFrameDistEnabled = mbEncIFrameDistInUse;
        mbEncCurbeParams.pdwBlockBasedSkipEn     = &dwMbEncBlockBasedSkipEn;
        if (mbEncIFrameDistInUse)
        {
            mbEncCurbeParams.bBrcEnabled           = false;
            mbEncCurbeParams.wPicWidthInMb         = (uint16_t)m_downscaledWidthInMb4x;
            mbEncCurbeParams.wFieldFrameHeightInMb = (uint16_t)m_downscaledFrameFieldHeightInMb4x;
            mbEncCurbeParams.usSliceHeight         = (m_sliceHeight + SCALE_FACTOR_4x - 1) / SCALE_FACTOR_4x;
        }
        else
        {
            mbEncCurbeParams.bBrcEnabled           = bBrcEnabled;
            mbEncCurbeParams.wPicWidthInMb         = m_picWidthInMb;
            mbEncCurbeParams.wFieldFrameHeightInMb = m_frameFieldHeightInMb;
            mbEncCurbeParams.usSliceHeight         = (m_arbitraryNumMbsInSlice) ?
                m_frameFieldHeightInMb : m_sliceHeight;
            mbEncCurbeParams.bUseMbEncAdvKernel    = bUseMbEncAdvKernel;
        }
        mbEncCurbeParams.pKernelState                     = kernelState;
        mbEncCurbeParams.pAvcQCParams                     = m_avcQCParams;
        mbEncCurbeParams.bMbDisableSkipMapEnabled         = bMbDisableSkipMapEnabled;
        mbEncCurbeParams.bStaticFrameDetectionEnabled     = bStaticFrameDetectionEnable && m_hmeEnabled;
        mbEncCurbeParams.bApdatvieSearchWindowSizeEnabled = bApdatvieSearchWindowEnable;
        mbEncCurbeParams.bSquareRollingIEnabled           = bSquareRollingIEnabled;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeAvcMbEnc(
            &mbEncCurbeParams));
    }

    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        // Set MFE specific curbe in the last stream
        // MFE MBEnc specific curbe is different from the normal MBEnc curbe which is passed
        // to MFE MBEnc kernel as a surface.
        if (m_mfeLastStream)
        {
            CODECHAL_ENCODE_AVC_MFE_MBENC_CURBE_PARAMS mfeMbEncCurbeParams;
            MOS_ZeroMemory(&mfeMbEncCurbeParams, sizeof(mfeMbEncCurbeParams));
            mfeMbEncCurbeParams.submitNumber  = m_mfeEncodeParams.submitNumber;
            mfeMbEncCurbeParams.pKernelState  = kernelState;
            mfeMbEncCurbeParams.pBindingTable = &MbEncBindingTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeAvcMfeMbEnc(&mfeMbEncCurbeParams));
        }
        // Change the binding table according to the index during this submission
        UpdateMfeMbEncBindingTable(m_mfeEncodeParams.submitIndex);
    }

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

        for (uint8_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
        {
            if (m_picIdx[i].bValid)
            {
                uint8_t index = m_picIdx[i].ucPicIdx;
                refList[index]->sRefBuffer = m_userFlags.bUseRawPicForRef ?
                    refList[index]->sRefRawBuffer : refList[index]->sRefReconBuffer;

                CodecHalGetResourceInfo(m_osInterface, &refList[index]->sRefBuffer);
            }
        }

    MOS_COMMAND_BUFFER cmdBuffer;
    MOS_ZeroMemory(&cmdBuffer, sizeof(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // For MFE, All the commands are sent in the last stream and can not be sent in different streams
    // since CmdBuffer is zeroed for each stream and cmd buffer pointer is reset.
    if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) || m_mfeLastStream)
    {
        SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
        sendKernelCmdsParams.EncFunctionType = encFunctionType;
        sendKernelCmdsParams.ucDmvPredFlag   =
            m_avcSliceParams->direct_spatial_mv_pred_flag;
        sendKernelCmdsParams.pKernelState    = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));
    }

    // Set up MB BRC Constant Data Buffer if there is QP change within a frame
    if (mbConstDataBufferInUse)
    {
        CODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS initMbBrcConstantDataBufferParams;

        MOS_ZeroMemory(&initMbBrcConstantDataBufferParams, sizeof(initMbBrcConstantDataBufferParams));
        initMbBrcConstantDataBufferParams.pOsInterface                = m_osInterface;
        initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer   =
            &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
        initMbBrcConstantDataBufferParams.dwMbEncBlockBasedSkipEn     = dwMbEncBlockBasedSkipEn;
        initMbBrcConstantDataBufferParams.pPicParams                  = m_avcPicParams[ppsIdx];
        initMbBrcConstantDataBufferParams.wPictureCodingType          = m_pictureCodingType;
        initMbBrcConstantDataBufferParams.bSkipBiasAdjustmentEnable   = m_skipBiasAdjustmentEnable;
        initMbBrcConstantDataBufferParams.bAdaptiveIntraScalingEnable = bAdaptiveIntraScalingEnable;
        initMbBrcConstantDataBufferParams.bOldModeCostEnable          = bOldModeCostEnable;
        initMbBrcConstantDataBufferParams.pAvcQCParams                = m_avcQCParams;
        initMbBrcConstantDataBufferParams.bEnableKernelTrellis        = bKernelTrellis && m_trellisQuantParams.dwTqEnabled;

        // Kernel controlled Trellis Quantization
        if (bKernelTrellis && m_trellisQuantParams.dwTqEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CalcLambdaTable(
                m_pictureCodingType,
                &initMbBrcConstantDataBufferParams.Lambda[0][0]));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMbBrcConstantDataBuffer(&initMbBrcConstantDataBufferParams));

        //dump MbBrcLut
        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            initMbBrcConstantDataBufferParams.presBrcConstantDataBuffer,
            CodechalDbgAttr::attrOutput,
            "MbBrcLut",
            16 * (CODEC_AVC_NUM_QP) * sizeof(uint32_t),
            0,
            CODECHAL_MEDIA_STATE_ENC_QUALITY)));
    }

    // Add binding table
    // For MFE first stream sends binding table since the function zeros the whole SSH.
    // If last stream sends binding table it will clean the surface states from other streams.
    if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) || m_mfeFirstStream)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
            m_stateHeapInterface,
            kernelState));
    }

    //Add surface states
    CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS mbEncSurfaceParams;
    MOS_ZeroMemory(&mbEncSurfaceParams, sizeof(mbEncSurfaceParams));
    mbEncSurfaceParams.MediaStateType        = encFunctionType;
    mbEncSurfaceParams.pAvcSlcParams         = m_avcSliceParams;
    mbEncSurfaceParams.ppRefList             = &m_refList[0];
    mbEncSurfaceParams.pAvcPicIdx            = &m_picIdx[0];
    mbEncSurfaceParams.pCurrOriginalPic      = &m_currOriginalPic;
    mbEncSurfaceParams.pCurrReconstructedPic = &m_currReconstructedPic;
    mbEncSurfaceParams.wPictureCodingType    = m_pictureCodingType;
    mbEncSurfaceParams.psCurrPicSurface      = mbEncIFrameDistInUse ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_rawSurfaceToEnc;
    if (mbEncIFrameDistInUse && CodecHal_PictureIsBottomField(m_currOriginalPic))
    {
        mbEncSurfaceParams.dwCurrPicSurfaceOffset = m_scaledBottomFieldOffset;
    }
    mbEncSurfaceParams.dwMbCodeBottomFieldOffset          = (uint32_t)m_mbcodeBottomFieldOffset;
    mbEncSurfaceParams.dwMvBottomFieldOffset              = (uint32_t)m_mvBottomFieldOffset;
    mbEncSurfaceParams.ps4xMeMvDataBuffer                 = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
    mbEncSurfaceParams.ps4xMeDistortionBuffer             = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer);
    mbEncSurfaceParams.dwMeMvBottomFieldOffset            = m_hmeKernel->Get4xMeMvBottomFieldOffset();
    mbEncSurfaceParams.dwMeDistortionBottomFieldOffset    = m_hmeKernel->GetDistortionBottomFieldOffset();
    mbEncSurfaceParams.psMeBrcDistortionBuffer            = &BrcBuffers.sMeBrcDistortionBuffer;
    mbEncSurfaceParams.dwMeBrcDistortionBottomFieldOffset = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
    mbEncSurfaceParams.dwRefPicSelectBottomFieldOffset    = (uint32_t)ulRefPicSelectBottomFieldOffset;
    mbEncSurfaceParams.dwFrameWidthInMb                   = (uint32_t)m_picWidthInMb;
    mbEncSurfaceParams.dwFrameFieldHeightInMb             = (uint32_t)m_frameFieldHeightInMb;
    mbEncSurfaceParams.dwFrameHeightInMb                  = (uint32_t)m_picHeightInMb;
    // Interleaved input surfaces
    mbEncSurfaceParams.dwVerticalLineStride       = m_verticalLineStride;
    mbEncSurfaceParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;
    // Vertical line stride is not used for the case of scaled surfaces saved as separate fields
    if (!m_fieldScalingOutputInterleaved && mbEncIFrameDistInUse)
    {
        mbEncSurfaceParams.dwVerticalLineStride       = 0;
        mbEncSurfaceParams.dwVerticalLineStrideOffset = 0;
    }
    mbEncSurfaceParams.bHmeEnabled              = m_hmeSupported;
    mbEncSurfaceParams.bMbEncIFrameDistInUse    = mbEncIFrameDistInUse;
    mbEncSurfaceParams.presMbBrcConstDataBuffer = &BrcBuffers.resMbBrcConstDataBuffer[m_currRecycledBufIdx];
    mbEncSurfaceParams.psMbQpBuffer             =
        bMbQpDataEnabled ? &sMbQpDataSurface : &BrcBuffers.sBrcMbQpBuffer;
    mbEncSurfaceParams.dwMbQpBottomFieldOffset  = bMbQpDataEnabled ? 0 : BrcBuffers.dwBrcMbQpBottomFieldOffset;
    mbEncSurfaceParams.bUsedAsRef               =
        m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef;
    mbEncSurfaceParams.presMADDataBuffer        = &m_resMadDataBuffer[m_currMadBufferIdx];
    mbEncSurfaceParams.bMbQpBufferInUse         = mbQpBufferInUse;
    mbEncSurfaceParams.bMbSpecificDataEnabled   = bMbSpecificDataEnabled;
    mbEncSurfaceParams.presMbSpecificDataBuffer = &resMbSpecificDataBuffer[m_currRecycledBufIdx];
    mbEncSurfaceParams.bMbConstDataBufferInUse  = mbConstDataBufferInUse;
    mbEncSurfaceParams.bMADEnabled              = mbEncIFrameDistInUse ? false : m_madEnabled;
    mbEncSurfaceParams.bUseMbEncAdvKernel       = mbEncIFrameDistInUse ? false : bUseMbEncAdvKernel;
    mbEncSurfaceParams.presMbEncCurbeBuffer     =
        (mbEncIFrameDistInUse && bUseMbEncAdvKernel) ? nullptr : &BrcBuffers.resMbEncAdvancedDsh;
    mbEncSurfaceParams.presMbEncBRCBuffer       = &BrcBuffers.resMbEncBrcBuffer;

    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse) || bDecoupleMbEncCurbeFromBRC)
    {
        mbEncSurfaceParams.dwMbEncBRCBufferSize = m_mbencBrcBufferSize;
    }

    mbEncSurfaceParams.bUseAdvancedDsh            = bAdvancedDshInUse;
    mbEncSurfaceParams.bBrcEnabled                 = bBrcEnabled;
    mbEncSurfaceParams.bArbitraryNumMbsInSlice     = m_arbitraryNumMbsInSlice;
    mbEncSurfaceParams.psSliceMapSurface           = &m_sliceMapSurface[m_currRecycledBufIdx];
    mbEncSurfaceParams.dwSliceMapBottomFieldOffset = (uint32_t)m_sliceMapBottomFieldOffset;
    mbEncSurfaceParams.pMbEncBindingTable          = &MbEncBindingTable;
    mbEncSurfaceParams.pKernelState                = kernelState;

    if (m_mbStatsSupported)
    {
        mbEncSurfaceParams.bMBVProcStatsEnabled = m_flatnessCheckEnabled ||
                                                  m_adaptiveTransformDecisionEnabled ||
                                                  bMbBrcEnabled ||
                                                  bMbQpDataEnabled;
        mbEncSurfaceParams.presMBVProcStatsBuffer          = &m_resMbStatsBuffer;
        mbEncSurfaceParams.dwMBVProcStatsBottomFieldOffset = m_mbStatsBottomFieldOffset;
    }
    else
    {
        mbEncSurfaceParams.bFlatnessCheckEnabled            = m_flatnessCheckEnabled;
        mbEncSurfaceParams.psFlatnessCheckSurface           = &m_flatnessCheckSurface;
        mbEncSurfaceParams.dwFlatnessCheckBottomFieldOffset = (uint32_t)m_flatnessCheckBottomFieldOffset;
    }

    // Set up pFeiPicParams
    mbEncSurfaceParams.pFeiPicParams = m_avcFeiPicParams;

    mbEncSurfaceParams.bMbDisableSkipMapEnabled = bMbDisableSkipMapEnabled;
    mbEncSurfaceParams.psMbDisableSkipMapSurface = psMbDisableSkipMapSurface;

    if (bUseWeightedSurfaceForL0 || bUseWeightedSurfaceForL1)
    {
        if (!m_wpUseCommonKernel)
        {
            mbEncSurfaceParams.pWeightedPredOutputPicSelectList = &WeightedPredOutputPicSelectList[0];
        }
        mbEncSurfaceParams.bUseWeightedSurfaceForL0 = bUseWeightedSurfaceForL0;
        mbEncSurfaceParams.bUseWeightedSurfaceForL1 = bUseWeightedSurfaceForL1;
    }

    // Clear the MAD buffer -- the kernel requires it to be 0 as it accumulates the result
    if (mbEncSurfaceParams.bMADEnabled)
    {
        // set lock flag to WRITE_ONLY
        MOS_LOCK_PARAMS lockFlags;
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(
            m_osInterface,
            mbEncSurfaceParams.presMADDataBuffer,
            &lockFlags);

        CODECHAL_ENCODE_CHK_NULL_RETURN(data);

        MOS_ZeroMemory(data, CODECHAL_MAD_BUFFER_SIZE);

        m_osInterface->pfnUnlockResource(
            m_osInterface,
            mbEncSurfaceParams.presMADDataBuffer);

        CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resMadDataBuffer[m_currMadBufferIdx],
            CodechalDbgAttr::attrOutput,
            "MADRead",
            CODECHAL_MAD_BUFFER_SIZE,
            0,
            encFunctionType)));
    }

    // static frame detection buffer
    mbEncSurfaceParams.bStaticFrameDetectionEnabled = bStaticFrameDetectionEnable && m_hmeEnabled;
    mbEncSurfaceParams.presSFDOutputBuffer = &resSFDOutputBuffer[0];
    if (m_pictureCodingType == P_TYPE)
    {
        mbEncSurfaceParams.presSFDCostTableBuffer = &resSFDCostTablePFrameBuffer;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        mbEncSurfaceParams.presSFDCostTableBuffer = &resSFDCostTableBFrameBuffer;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendAvcMbEncSurfaces(&cmdBuffer, &mbEncSurfaceParams));

    // For MFE, only one walker processes frame in parallel through color bits.
    if (!IsMfeMbEncEnabled(mbEncIFrameDistInUse) || m_mfeLastStream)
    {
        uint32_t resolutionX = mbEncIFrameDistInUse ?
            m_downscaledWidthInMb4x : (uint32_t)m_picWidthInMb;
        uint32_t resolutionY = mbEncIFrameDistInUse ?
            m_downscaledFrameFieldHeightInMb4x : (uint32_t)m_frameFieldHeightInMb;

        CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
        MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
        walkerCodecParams.WalkerMode               = m_walkerMode;
        walkerCodecParams.bUseScoreboard           = m_useHwScoreboard;
        walkerCodecParams.wPictureCodingType       = m_pictureCodingType;
        walkerCodecParams.bMbEncIFrameDistInUse    = mbEncIFrameDistInUse;
        walkerCodecParams.bMbaff = m_mbaffEnabled;
        walkerCodecParams.bDirectSpatialMVPredFlag = m_avcSliceParams->direct_spatial_mv_pred_flag;
        walkerCodecParams.bColorbitSupported       = (m_colorbitSupported && !m_arbitraryNumMbsInSlice) ? m_cmKernelEnable : false;

        if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
        {
            walkerCodecParams.dwNumSlices   = m_mfeEncodeParams.submitNumber;  // MFE use color bit to handle frames in parallel
            walkerCodecParams.WalkerDegree  = CODECHAL_26_DEGREE;                        // MFE use 26 degree dependency
            walkerCodecParams.dwResolutionX = m_mfeEncodeSharedState->dwPicWidthInMB;
            walkerCodecParams.dwResolutionY = m_mfeEncodeSharedState->dwPicHeightInMB;
            walkerCodecParams.usSliceHeight = m_mfeEncodeSharedState->sliceHeight;
        }
        else
        {
            walkerCodecParams.dwResolutionX = resolutionX;
            walkerCodecParams.dwResolutionY = resolutionY;
            walkerCodecParams.dwNumSlices   = m_numSlices;
            walkerCodecParams.usSliceHeight = m_sliceHeight;
        }
        walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;
        walkerCodecParams.ucGroupId               = m_groupId;

        MHW_WALKER_PARAMS walkerParams;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
            m_hwInterface,
            &walkerParams,
            &walkerCodecParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaObjectWalkerCmd(
            &cmdBuffer,
            &walkerParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

        // Add dump for MBEnc surface state heap here
        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
                encFunctionType,
                MHW_SSH_TYPE,
                kernelState));
        )

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
                m_stateHeapInterface,
                kernelState));
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
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

        m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

        if ((!m_singleTaskPhaseSupported || m_lastTaskInPhase))
        {
            m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
            m_lastTaskInPhase = false;
        }
    }

    currRefList->ucMADBufferIdx = m_currMadBufferIdx;
    currRefList->bMADEnabled    = m_madEnabled;

    if (IsMfeMbEncEnabled(mbEncIFrameDistInUse))
    {
        m_stateHeapInterface    = m_origStateHeapInterface;
        m_hwInterface           = m_origHwInterface;
        m_osInterface           = m_origOsInterface;

        MbEncBindingTable       = origMbEncBindingTable;

        CODECHAL_DEBUG_TOOL(
            m_debugInterface->m_osInterface = m_osInterface;)
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SetAndPopulateVEHintParams(
    PMOS_COMMAND_BUFFER  cmdBuffer)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_SUPPORTED(m_osInterface))
    {
        return eStatus;
    }

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        MOS_VIRTUALENGINE_SET_PARAMS  vesetParams;
        MOS_ZeroMemory(&vesetParams, sizeof(vesetParams));
        vesetParams.bNeedSyncWithPrevious = true;
        vesetParams.bSFCInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_SetHintParams(m_sinlgePipeVeState, &vesetParams));
    }
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_PopulateHintParams(m_sinlgePipeVeState, cmdBuffer, true));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SubmitCommandBuffer(
    PMOS_COMMAND_BUFFER cmdBuffer,
    int32_t             nullRendering)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetAndPopulateVEHintParams(cmdBuffer));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, cmdBuffer, nullRendering));
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto slcParams = m_avcSliceParams;
    auto slcType   = Slice_Type[slcParams->slice_type];

    CODECHAL_DEBUG_TOOL(
    //    CodecHal_DbgMapSurfaceFormatToDumpFormat(m_rawSurfaceToEnc->Format, &dumpType);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
            m_rawSurfaceToEnc,
            CodechalDbgAttr::attrEncodeRawInputSurface,
            "SrcSurf")));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

    // Scaling, BRC Init/Reset and HME are included in the same task phase
    m_lastEncPhase     = false;
    m_firstTaskInPhase = true;

    // BRC init/reset needs to be called before HME since it will reset the Brc Distortion surface
    if (bBrcEnabled && (bBrcInit || bBrcReset))
    {
        bool cscEnabled            = m_cscDsState->RequireCsc() && m_firstField;
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
        cscScalingKernelParams.bLastTaskInPhase16xDS    = !(m_32xMeSupported || m_hmeEnabled);
        cscScalingKernelParams.bLastTaskInPhase32xDS    = !m_hmeEnabled;
        cscScalingKernelParams.inputColorSpace          = m_avcSeqParam->InputColorSpace;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));
    }

    if (m_hmeKernel && m_hmeKernel->Is4xMeEnabled())
    {
        CodechalKernelHme::CurbeParam curbeParam = {};
        curbeParam.subPelMode = 3;
        curbeParam.currOriginalPic = m_avcPicParam->CurrOriginalPic;
        curbeParam.qpPrimeY = m_avcPicParam->pic_init_qp_minus26 + 26 + m_avcSliceParams->slice_qp_delta;
        curbeParam.targetUsage = m_avcSeqParam->TargetUsage;
        curbeParam.maxMvLen = CodecHalAvcEncode_GetMaxMvLen(m_avcSeqParam->Level);
        curbeParam.numRefIdxL0Minus1 = m_avcSliceParams->num_ref_idx_l0_active_minus1;
        curbeParam.numRefIdxL1Minus1 = m_avcSliceParams->num_ref_idx_l1_active_minus1;

        auto slcParams = m_avcSliceParams;
        curbeParam.list0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        curbeParam.list0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        curbeParam.list0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        curbeParam.list0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        curbeParam.list0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        curbeParam.list0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        curbeParam.list0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        curbeParam.list0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        curbeParam.list1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        curbeParam.list1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);

        CodechalKernelHme::SurfaceParams surfaceParam = {};
        surfaceParam.mbaffEnabled = m_mbaffEnabled;
        surfaceParam.numRefIdxL0ActiveMinus1 = m_avcSliceParams->num_ref_idx_l0_active_minus1;
        surfaceParam.numRefIdxL1ActiveMinus1 = m_avcSliceParams->num_ref_idx_l1_active_minus1;
        surfaceParam.verticalLineStride = m_verticalLineStride;
        surfaceParam.verticalLineStrideOffset = m_verticalLineStrideOffset;
        surfaceParam.meBrcDistortionBottomFieldOffset = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
        surfaceParam.refList = &m_refList[0];
        surfaceParam.picIdx = &m_picIdx[0];
        surfaceParam.currOriginalPic = &m_currOriginalPic;
        surfaceParam.refL0List = &(m_avcSliceParams->RefPicList[LIST_0][0]);
        surfaceParam.refL1List = &(m_avcSliceParams->RefPicList[LIST_1][0]);
        surfaceParam.meBrcDistortionBuffer = &BrcBuffers.sMeBrcDistortionBuffer;

        if (m_hmeKernel->Is16xMeEnabled())
        {
            m_lastTaskInPhase = false;
            if (m_hmeKernel->Is32xMeEnabled())
            {
                surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb32x;
                surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb32x;
                surfaceParam.downScaledBottomFieldOffset = m_scaled32xBottomFieldOffset;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel32x));
            }
            surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb16x;
            surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb16x;
            surfaceParam.downScaledBottomFieldOffset = m_scaled16xBottomFieldOffset;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel16x));
        }
        surfaceParam.downScaledWidthInMb = m_downscaledWidthInMb4x;
        surfaceParam.downScaledHeightInMb = m_downscaledFrameFieldHeightInMb4x;
        surfaceParam.downScaledBottomFieldOffset = m_scaledBottomFieldOffset;

        m_lastTaskInPhase = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Execute(curbeParam, surfaceParam, CodechalKernelHme::HmeLevel::hmeLevel4x));
    }

    // Scaling and HME are not dependent on the output from PAK
    if (m_waitForPak && m_semaphoreObjCount && !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        // Wait on PAK
        auto syncParams             = g_cInitSyncParams;
        syncParams.GpuContext       = m_renderContext;
        syncParams.presSyncResource = &m_resSyncObjectVideoContextInUse;
        syncParams.uiSemaphoreCount = m_semaphoreObjCount;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_semaphoreObjCount = 0;  //reset
    }

    // BRC and MbEnc are included in the same task phase
    m_lastEncPhase     = true;
    m_firstTaskInPhase = true;

    // Initialize software scoreboard used by MBEnc kernel
    // Decide dependency pattern
    CodechalEncodeSwScoreboard::KernelParams swScoreboardKernelParames;
    memset((void *)&swScoreboardKernelParames, 0, sizeof(swScoreboardKernelParames));

    if (m_pictureCodingType == I_TYPE ||
        (m_pictureCodingType == B_TYPE && !m_avcSliceParams->direct_spatial_mv_pred_flag))  // I or B-temporal
    {
        swScoreboardKernelParames.surfaceIndex = dependencyWavefront45Degree;
        m_swScoreboardState->SetDependencyPattern(dependencyWavefront45Degree);
    }
    else  //P or B-spatial
    {
        swScoreboardKernelParames.surfaceIndex = dependencyWavefront26Degree;
        m_swScoreboardState->SetDependencyPattern(dependencyWavefront26Degree);
    }

    m_swScoreboardState->SetCurSwScoreboardSurfaceIndex(swScoreboardKernelParames.surfaceIndex);

    // Call SW scoreboard Init kernel
    swScoreboardKernelParames.scoreboardWidth           = m_picWidthInMb;
    swScoreboardKernelParames.scoreboardHeight          = m_frameFieldHeightInMb;
    swScoreboardKernelParames.swScoreboardSurfaceWidth  = swScoreboardKernelParames.scoreboardWidth * 4;
    swScoreboardKernelParames.swScoreboardSurfaceHeight = swScoreboardKernelParames.scoreboardHeight;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->Execute(&swScoreboardKernelParames));

    // Dump BrcDist 4X_ME buffer here because it will be overwritten in BrcFrameUpdateKernel
    CODECHAL_DEBUG_TOOL(
        if (m_hmeEnabled && bBrcDistortionBufferSupported)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &BrcBuffers.sMeBrcDistortionBuffer.OsResource,
                CodechalDbgAttr::attrOutput,
                "BrcDist",
                BrcBuffers.sMeBrcDistortionBuffer.dwPitch * BrcBuffers.sMeBrcDistortionBuffer.dwHeight,
                BrcBuffers.dwMeBrcDistortionBottomFieldOffset,
                CODECHAL_MEDIA_STATE_4X_ME));
        })

    if (bBrcEnabled)
    {
        if (bMbEncIFrameDistEnabled)
        {
            CodechalKernelIntraDist::CurbeParam curbeParam;
            curbeParam.downScaledWidthInMb4x  = m_downscaledWidthInMb4x;
            curbeParam.downScaledHeightInMb4x = m_downscaledFrameFieldHeightInMb4x;
            CodechalKernelIntraDist::SurfaceParams surfaceParam;
            surfaceParam.input4xDsSurface           =
            surfaceParam.input4xDsVmeSurface        = m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER);
            surfaceParam.intraDistSurface           = &BrcBuffers.sMeBrcDistortionBuffer;
            surfaceParam.intraDistBottomFieldOffset = BrcBuffers.dwMeBrcDistortionBottomFieldOffset;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Execute(curbeParam, surfaceParam));
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

    if (bWeightedPredictionSupported &&
        ((((slcType == SLICE_P) || (slcType == SLICE_SP)) && (m_avcPicParam->weighted_pred_flag)) ||
            (((slcType == SLICE_B)) && (m_avcPicParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))))
    {
        uint8_t idx;
        // Weighted Prediction to be applied for L0
        for (idx = 0; idx < (m_avcPicParam->num_ref_idx_l0_active_minus1 + 1); idx++)
        {
            if ((slcParams->luma_weight_flag[LIST_0] & (1 << idx)) && (idx < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                //Weighted Prediction for ith forward reference frame
                CODECHAL_ENCODE_CHK_STATUS_RETURN(WPKernel(false, idx));
            }
        }

        if (((slcType == SLICE_B)) &&
            (m_avcPicParam->weighted_bipred_idc == EXPLICIT_WEIGHTED_INTER_PRED_MODE))
        {
            for (idx = 0; idx < (m_avcPicParam->num_ref_idx_l1_active_minus1 + 1); idx++)
            {
                // Weighted Pred to be applied for L1
                if ((slcParams->luma_weight_flag[LIST_1] & 1 << idx) && (idx < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
                {
                    //Weighted Prediction for ith backward reference frame
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(WPKernel(false, idx));
                }
            }
        }
    }

#if (_DEBUG || _RELEASE_INTERNAL)

    MOS_USER_FEATURE_VALUE_WRITE_DATA userFeatureWriteData;

    // Weighted prediction for L0 Reporting
    userFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = bUseWeightedSurfaceForL0;
    userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L0_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

    // Weighted prediction for L1 Reporting
    userFeatureWriteData               = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    userFeatureWriteData.Value.i32Data = bUseWeightedSurfaceForL1;
    userFeatureWriteData.ValueID       = __MEDIA_USER_FEATURE_VALUE_WEIGHTED_PREDICTION_L1_IN_USE_ID;
    MOS_UserFeature_WriteValues_ID(nullptr, &userFeatureWriteData, 1);

#endif  // _DEBUG || _RELEASE_INTERNAL

    m_lastTaskInPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MbEncKernel(false));

    // Reset buffer ID used for MbEnc kernel performance reports
    m_osInterface->pfnResetPerfBufferID(m_osInterface);

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        auto syncParams             = g_cInitSyncParams;
        syncParams.GpuContext       = m_renderContext;
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

    CODECHAL_DEBUG_TOOL(KernelDebugDumps());

    if (bBrcEnabled)
    {
        bMbEncCurbeSetInBrcUpdate = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::GenericEncodePictureLevel(PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto trellisQuantParams = &m_trellisQuantParams;

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    MHW_MI_FORCE_WAKEUP_PARAMS forceWakeupParams;
    MOS_ZeroMemory(&forceWakeupParams, sizeof(MHW_MI_FORCE_WAKEUP_PARAMS));
    forceWakeupParams.bMFXPowerWellControl = true;
    forceWakeupParams.bMFXPowerWellControlMask = true;
    forceWakeupParams.bHEVCPowerWellControl = false;
    forceWakeupParams.bHEVCPowerWellControlMask = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiForceWakeupCmd(
        &cmdBuffer,
        &forceWakeupParams));

    // set MFX_PIPE_MODE_SELECT values
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bStreamOutEnabled = (m_currPass != m_numPasses);// Disable Stream Out for final pass; its important for multiple passes, because , next pass will take the qp from stream out

    pipeModeSelectParams.bDeblockerStreamOutEnable = params->bDeblockerStreamOutEnable;
    pipeModeSelectParams.bPostDeblockOutEnable = params->bPostDeblockOutEnable;
    pipeModeSelectParams.bPreDeblockOutEnable = params->bPreDeblockOutEnable;
    pipeModeSelectParams.bDynamicSliceEnable = m_avcSeqParam->EnableSliceLevelRateCtrl;

    // set MFX_PIPE_BUF_ADDR_STATE values
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode = m_mode;
    pipeBufAddrParams.psPreDeblockSurface = params->psPreDeblockSurface;
    pipeBufAddrParams.psPostDeblockSurface = params->psPostDeblockSurface;
    pipeBufAddrParams.psRawSurface = m_rawSurfaceToPak;
    pipeBufAddrParams.presStreamOutBuffer = &m_resStreamOutBuffer[m_currRecycledBufIdx];
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resDeblockingFilterRowStoreScratchBuffer;
    pipeBufAddrParams.presMfdIntraRowStoreScratchBuffer = &m_intraRowStoreScratchBuffer;
    pipeBufAddrParams.presMacroblockIldbStreamOutBuffer1 = params->presMacroblockIldbStreamOutBuffer1;
    pipeBufAddrParams.presMacroblockIldbStreamOutBuffer2 = params->presMacroblockIldbStreamOutBuffer2;

    CODECHAL_DEBUG_TOOL(
    // PAK Input Raw Surface
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToPak,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "PAK_Input_SrcSurf"));
    )

        auto firstValidFrame = &m_reconSurface.OsResource;

    // Setting invalid entries to nullptr
    for (uint32_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        pipeBufAddrParams.presReferences[i] = nullptr;
    }

    uint8_t firstValidFrameId = CODEC_AVC_MAX_NUM_REF_FRAME;

    for (uint32_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t picIdx = m_picIdx[i].ucPicIdx;
            uint8_t frameStoreId = m_refList[picIdx]->ucFrameId;

            CodecHalGetResourceInfo(
                m_osInterface,
                &(m_refList[picIdx]->sRefReconBuffer));
            pipeBufAddrParams.presReferences[frameStoreId] =
                &(m_refList[picIdx]->sRefReconBuffer.OsResource);

            if (picIdx < firstValidFrameId)
            {
                firstValidFrameId = picIdx;
                firstValidFrame = pipeBufAddrParams.presReferences[picIdx];
            }

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);
                MOS_SURFACE refSurface;

                MOS_ZeroMemory(&refSurface, sizeof(refSurface));
                refSurface.Format     = Format_NV12;
                refSurface.OsResource = *(pipeBufAddrParams.presReferences[frameStoreId]);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &refSurface));

                m_debugInterface->m_refIndex = frameStoreId;
                std::string refSurfName      = "RefSurf" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex));
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.data()));)
        }
    }

    for (uint32_t i = 0; i < CODEC_AVC_MAX_NUM_REF_FRAME; i++)
    {
        // error concealment for the unset reference addresses
        if (!pipeBufAddrParams.presReferences[i])
        {
            pipeBufAddrParams.presReferences[i] = firstValidFrame;
        }
    }

    if (m_sliceSizeStreamoutSupported)
    {
        pipeBufAddrParams.presSliceSizeStreamOutBuffer = &m_pakSliceSizeStreamoutBuffer;
    }

    // set MFX_IND_OBJ_BASE_ADDR_STATE values
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = CODECHAL_ENCODE_MODE_AVC;

    indObjBaseAddrParams.presMvObjectBuffer = &m_resMvDataSurface;
    indObjBaseAddrParams.dwMvObjectOffset = m_mvBottomFieldOffset;
    indObjBaseAddrParams.dwMvObjectSize = m_mvDataSize;
    indObjBaseAddrParams.presPakBaseObjectBuffer = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize = m_bitstreamUpperBound;

    // set MFX_BSP_BUF_BASE_ADDR_STATE values
    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resMPCRowStoreScratchBuffer;

    MHW_VDBOX_QM_PARAMS qmParams;
    qmParams.Standard = CODECHAL_AVC;
    qmParams.pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_avcIQWeightScaleLists;

    MHW_VDBOX_QM_PARAMS fqmParams;
    fqmParams.Standard = CODECHAL_AVC;
    fqmParams.pAvcIqMatrix = (PMHW_VDBOX_AVC_QM_PARAMS)m_avcIQWeightScaleLists;

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

    // PAK cmd buffer header insertion for 1) non STF 2) STF (except VDEnc BRC case inserted in HuC cmd buffer)
    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        bool requestFrameTracking = false;

        m_hwInterface->m_numRequestedEuSlices = ((m_frameHeight * m_frameWidth) >= m_ssdResolutionThreshold &&
            m_targetUsage <= m_ssdTargetUsageThreshold) ?
            m_sliceShutdownRequestState : m_sliceShutdownDefaultState;

        // Send command buffer header at the beginning (OS dependent)
        // frame tracking tag is only added in the last command buffer header
        requestFrameTracking = m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

        m_hwInterface->m_numRequestedEuSlices = CODECHAL_SLICE_SHUTDOWN_DEFAULT;
    }

    if (m_currPass)
    {
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
        // Insert conditional batch buffer end
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_encodeStatusBuf.resStatusBuffer;
        miConditionalBatchBufferEndParams.dwOffset =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwImageStatusMaskOffset +
            (sizeof(uint32_t) * 2);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    if (!m_currPass && m_osInterface->bTagResourceSync)
    {
        // This is a short term WA to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.
        MHW_MI_STORE_DATA_PARAMS                        params;
        MOS_RESOURCE                                    globalGpuContextSyncTagBuffer;
        uint32_t                                           value;

        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        value = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.pOsResource = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue = (value > 0) ? (value - 1) : 0;
        CODECHAL_HW_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    // set MFX_SURFACE_STATE values
    // Ref surface
    MHW_VDBOX_SURFACE_PARAMS reconSurfaceParams;
    MOS_ZeroMemory(&reconSurfaceParams, sizeof(reconSurfaceParams));
    reconSurfaceParams.Mode = m_mode;
    reconSurfaceParams.ucSurfaceStateId = CODECHAL_MFX_REF_SURFACE_ID;
    reconSurfaceParams.psSurface = &m_reconSurface;
#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceState(&reconSurfaceParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &reconSurfaceParams));

    // Src surface
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;
    surfaceParams.ucSurfaceStateId = CODECHAL_MFX_SRC_SURFACE_ID;
    surfaceParams.psSurface = m_rawSurfaceToPak;
    surfaceParams.bDisplayFormatSwizzle = m_avcPicParam->bDisplayFormatSwizzle;
#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceState(&surfaceParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));
#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetPipeBufAddr(&pipeBufAddrParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));

    if (params->bBrcEnabled && m_avcSeqParam->RateControlMethod != RATECONTROL_ICQ)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            params->pImgStateBatchBuffer));
    }
    else
    {
        //Set MFX_AVC_IMG_STATE command
        MHW_VDBOX_AVC_IMG_PARAMS imageStateParams;
        imageStateParams.ucCurrPass = m_currPass;
        imageStateParams.pEncodeAvcPicParams = m_avcPicParam;
        imageStateParams.pEncodeAvcSeqParams = m_avcSeqParam;
        imageStateParams.pEncodeAvcSliceParams = m_avcSliceParams;
        if (CodecHalIsFeiEncode(m_codecFunction) && m_avcFeiPicParams && m_avcFeiPicParams->dwMaxFrameSize)
        {
            imageStateParams.pDeltaQp = m_avcFeiPicParams->pDeltaQp;
            imageStateParams.dwMaxFrameSize = m_avcFeiPicParams->dwMaxFrameSize;
        }
        imageStateParams.wPicWidthInMb = m_picWidthInMb;
        imageStateParams.wPicHeightInMb = m_picHeightInMb;
        imageStateParams.ppRefList = &(m_refList[0]);
        imageStateParams.dwTqEnabled = trellisQuantParams->dwTqEnabled;
        imageStateParams.dwTqRounding = trellisQuantParams->dwTqRounding;
        imageStateParams.ucKernelMode = m_kernelMode;
        imageStateParams.wSlcHeightInMb = m_sliceHeight;
        imageStateParams.dwMaxVmvR = CodecHalAvcEncode_GetMaxVmvR(m_avcSeqParam->Level);
        imageStateParams.bSliceSizeStreamOutEnabled = m_sliceSizeStreamoutSupported;

        if (m_currPass && (m_currPass == m_numPasses))
        {
            // Enable IPCM pass, excluding VDENC BRC case
            imageStateParams.bIPCMPass = true;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcImgCmd(&cmdBuffer, nullptr, &imageStateParams));

        CODECHAL_DEBUG_TOOL(
            CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulatePakParam(
                &cmdBuffer,
                nullptr));
        )
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(&cmdBuffer, &qmParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxFqmCmd(&cmdBuffer, &fqmParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxAvcDirectmodeCmd(&cmdBuffer, &directmodeParams));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SceneChangeReport(PMOS_COMMAND_BUFFER    cmdBuffer, PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS params)
{

    MHW_MI_COPY_MEM_MEM_PARAMS                      copyMemMemParams;
    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize)
        + (sizeof(uint32_t) * 2) + m_encodeStatusBuf.dwSceneChangedOffset;

    MOS_ZeroMemory(&copyMemMemParams, sizeof(copyMemMemParams));
    copyMemMemParams.presSrc = params->presBrcHistoryBuffer;
    copyMemMemParams.dwSrcOffset = brcHistoryBufferOffsetSceneChanged;
    copyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
    copyMemMemParams.dwDstOffset = offset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
        &copyMemMemParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG12::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitializeState());

    m_brcHistoryBufferSize  = m_initBrcHistoryBufferSize;
    m_mbencBrcBufferSize    = m_initMbencBrcBufferSize;
    m_forceBrcMbStatsEnabled = true;
    m_useHwScoreboard        = false;

    dwBrcConstantSurfaceWidth  = m_brcConstantsurfaceWidth;
    dwBrcConstantSurfaceHeight = m_brcConstantsurfaceHeight;

    // create intra distortion kernel
    m_intraDistKernel = MOS_New(CodechalKernelIntraDist, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_intraDistKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_intraDistKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));

    // Create SW scoreboard init kernel state
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_swScoreboardState = MOS_New(CodechalEncodeSwScoreboardG12, this));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_swScoreboardState->InitKernelState());

    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        m_sinlgePipeVeState = (PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE)MOS_AllocAndZeroMemory(sizeof(CODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE));
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_sinlgePipeVeState);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_InitInterface(m_hwInterface, m_sinlgePipeVeState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;
#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeAvcG12, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG12::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    for (uint32_t krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM; krnStateIdx++)
    {
        auto kernelStatePtr = &BrcKernelStates[krnStateIdx];
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            ENC_BRC,
            krnStateIdx,
            (void*)&currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount     = m_brcBTCounts[krnStateIdx];
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = m_brcCurbeSize[krnStateIdx];
        kernelStatePtr->KernelParams.iBlockWidth  = CODECHAL_MACROBLOCK_WIDTH;
        kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelStatePtr->KernelParams.iIdCount     = 1;

        kernelStatePtr->dwCurbeOffset        = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelStatePtr->KernelParams.iSize   = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelStatePtr->KernelParams.iBTCount,
            &kernelStatePtr->dwSshSize,
            &kernelStatePtr->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable                                 = &BrcUpdateBindingTable;
    bindingTable->dwFrameBrcHistoryBuffer             = frameBrcUpdateHistory;
    bindingTable->dwFrameBrcPakStatisticsOutputBuffer = frameBrcUpdatePakStatisticsOutput;
    bindingTable->dwFrameBrcImageStateReadBuffer      = frameBrcUpdateImageStateRead;
    bindingTable->dwFrameBrcImageStateWriteBuffer     = frameBrcUpdateImageStateWrite;

    bindingTable->dwFrameBrcMbEncCurbeWriteData = frameBrcUpdateMbencCurbeWrite;
    bindingTable->dwFrameBrcDistortionBuffer    = frameBrcUpdateDistortion;
    bindingTable->dwFrameBrcConstantData        = frameBrcUpdateConstantData;
    bindingTable->dwFrameBrcMbStatBuffer        = frameBrcUpdateMbStat;
    bindingTable->dwFrameBrcMvDataBuffer        = frameBrcUpdateMvStat;

    bindingTable->dwMbBrcHistoryBuffer = mbBrcUpdateHistory;
    bindingTable->dwMbBrcMbQpBuffer    = mbBrcUpdateMbQp;
    bindingTable->dwMbBrcROISurface    = mbBrcUpdateRoi;
    bindingTable->dwMbBrcMbStatBuffer  = mbBrcUpdateMbStat;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::GetTrellisQuantization(
    PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
    PCODECHAL_ENCODE_AVC_TQ_PARAMS       trellisQuantParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(trellisQuantParams);

    trellisQuantParams->dwTqEnabled = TrellisQuantizationEnable[params->ucTargetUsage];
    trellisQuantParams->dwTqRounding =
        trellisQuantParams->dwTqEnabled ? trellisQuantizationRounding[params->ucTargetUsage] : 0;

    // If AdaptiveTrellisQuantization is enabled then disable trellis quantization for
    // B-frames with QP > 26 only in CQP mode
    if (trellisQuantParams->dwTqEnabled && EnableAdaptiveTrellisQuantization[params->ucTargetUsage] &&
        params->wPictureCodingType == B_TYPE && !params->bBrcEnabled && params->ucQP > 26)
    {
        trellisQuantParams->dwTqEnabled  = 0;
        trellisQuantParams->dwTqRounding = 0;
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::GetMbEncKernelStateIdx(CodechalEncodeIdOffsetParams* params, uint32_t* kernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelOffset);

    *kernelOffset = mbencIOffset;

    if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *kernelOffset += m_mbencNumTargetUsages * mbencFrameTypeNum;
    }
    else
    {
        if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_NORMAL)
        {
            *kernelOffset += mbencFrameTypeNum;
        }
        else if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_PERFORMANCE)
        {
            *kernelOffset += mbencFrameTypeNum * 2;
        }
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        *kernelOffset += mbencPOffset;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        *kernelOffset += mbencBOffset;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::InitKernelStateMbEnc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    dwNumMbEncEncKrnStates = m_mbencNumTargetUsages * mbencFrameTypeNum;
    dwNumMbEncEncKrnStates += mbencFrameTypeNum;
    pMbEncKernelStates = MOS_NewArray(MHW_KERNEL_STATE, dwNumMbEncEncKrnStates);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelStates);

    auto kernelStatePtr = pMbEncKernelStates;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));
    CODECHAL_KERNEL_HEADER currKrnHeader;

    for (uint32_t krnStateIdx = 0; krnStateIdx < dwNumMbEncEncKrnStates; krnStateIdx++)
    {
        bool kernelState = (krnStateIdx >= m_mbencNumTargetUsages * mbencFrameTypeNum);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetKernelHeaderAndSize(
            kernelBinary,
            (kernelState ? ENC_MBENC_ADV : ENC_MBENC),
            (kernelState ? krnStateIdx - m_mbencNumTargetUsages * mbencFrameTypeNum : krnStateIdx),
            &currKrnHeader,
            &kernelSize));

        kernelStatePtr->KernelParams.iBTCount     = mbencNumSurfaces;
        kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
        kernelStatePtr->KernelParams.iCurbeLength = sizeof(CodechalEncodeAvcEncG12::MbencCurbe);
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

        kernelStatePtr++;
    }

    // Until a better way can be found, maintain old binding table structures
    auto bindingTable = &MbEncBindingTable;

    bindingTable->dwAvcMBEncMfcAvcPakObj   = mbencMfcAvcPakObj;
    bindingTable->dwAvcMBEncIndMVData      = mbencIndMvData;
    bindingTable->dwAvcMBEncBRCDist        = mbencBrcDistortion;
    bindingTable->dwAvcMBEncCurrY          = mbencCurrY;
    bindingTable->dwAvcMBEncCurrUV         = mbencCurrUv;
    bindingTable->dwAvcMBEncMbSpecificData = mbencMbSpecificData;

    bindingTable->dwAvcMBEncRefPicSelectL0           = mbencRefpicselectL0;
    bindingTable->dwAvcMBEncMVDataFromME             = mbencMvDataFromMe;
    bindingTable->dwAvcMBEncMEDist                   = mbenc4xMeDistortion;
    bindingTable->dwAvcMBEncSliceMapData             = mbencSlicemapData;
    bindingTable->dwAvcMBEncBwdRefMBData             = mbencFwdMbData;
    bindingTable->dwAvcMBEncBwdRefMVData             = mbencFwdMvData;
    bindingTable->dwAvcMBEncMbBrcConstData           = mbencMbbrcConstData;
    bindingTable->dwAvcMBEncMBStats                  = mbencMbStats;
    bindingTable->dwAvcMBEncMADData                  = mbencMadData;
    bindingTable->dwAvcMBEncMbNonSkipMap             = mbencForceNonskipMbMap;
    bindingTable->dwAvcMBEncAdv                      = mbEncAdv;
    bindingTable->dwAvcMbEncBRCCurbeData             = mbencBrcCurbeData;
    bindingTable->dwAvcMBEncStaticDetectionCostTable = mbencSfdCostTable;

    // Frame
    bindingTable->dwAvcMBEncMbQpFrame       = mbencMbqp;
    bindingTable->dwAvcMBEncCurrPicFrame[0] = mbencVmeInterPredCurrPicIdx0;
    bindingTable->dwAvcMBEncFwdPicFrame[0]  = mbencVmeInterPredFwdPicIDX0;
    bindingTable->dwAvcMBEncBwdPicFrame[0]  = mbencVmeInterPredBwdPicIDX00;
    bindingTable->dwAvcMBEncFwdPicFrame[1]  = mbencVmeInterPredFwdPicIDX1;
    bindingTable->dwAvcMBEncBwdPicFrame[1]  = mbencVmeInterPredBwdPicIDX10;
    bindingTable->dwAvcMBEncFwdPicFrame[2]  = mbencVmeInterPredFwdPicIDX2;
    bindingTable->dwAvcMBEncFwdPicFrame[3]  = mbencVmeInterPredFwdPicIDX3;
    bindingTable->dwAvcMBEncFwdPicFrame[4]  = mbencVmeInterPredFwdPicIDX4;
    bindingTable->dwAvcMBEncFwdPicFrame[5]  = mbencVmeInterPredFwdPicIDX5;
    bindingTable->dwAvcMBEncFwdPicFrame[6]  = mbencVmeInterPredFwdPicIDX6;
    bindingTable->dwAvcMBEncFwdPicFrame[7]  = mbencVmeInterPredFwdPicIDX7;
    bindingTable->dwAvcMBEncCurrPicFrame[1] = mbencVmeInterPredCurrPicIdx1;
    bindingTable->dwAvcMBEncBwdPicFrame[2]  = mbencVmeInterPredBwdPicIDX01;
    bindingTable->dwAvcMBEncBwdPicFrame[3]  = mbencVmeInterPredBwdPicIDX11;

    // Field
    bindingTable->dwAvcMBEncMbQpField         = mbencMbqp;
    bindingTable->dwAvcMBEncFieldCurrPic[0]   = mbencVmeInterPredCurrPicIdx0;
    bindingTable->dwAvcMBEncFwdPicTopField[0] = mbencVmeInterPredFwdPicIDX0;
    bindingTable->dwAvcMBEncBwdPicTopField[0] = mbencVmeInterPredBwdPicIDX00;
    bindingTable->dwAvcMBEncFwdPicBotField[0] = mbencVmeInterPredFwdPicIDX0;
    bindingTable->dwAvcMBEncBwdPicBotField[0] = mbencVmeInterPredBwdPicIDX00;
    bindingTable->dwAvcMBEncFwdPicTopField[1] = mbencVmeInterPredFwdPicIDX1;
    bindingTable->dwAvcMBEncBwdPicTopField[1] = mbencVmeInterPredBwdPicIDX10;
    bindingTable->dwAvcMBEncFwdPicBotField[1] = mbencVmeInterPredFwdPicIDX1;
    bindingTable->dwAvcMBEncBwdPicBotField[1] = mbencVmeInterPredBwdPicIDX10;
    bindingTable->dwAvcMBEncFwdPicTopField[2] = mbencVmeInterPredFwdPicIDX2;
    bindingTable->dwAvcMBEncFwdPicBotField[2] = mbencVmeInterPredFwdPicIDX2;
    bindingTable->dwAvcMBEncFwdPicTopField[3] = mbencVmeInterPredFwdPicIDX3;
    bindingTable->dwAvcMBEncFwdPicBotField[3] = mbencVmeInterPredFwdPicIDX3;
    bindingTable->dwAvcMBEncFwdPicTopField[4] = mbencVmeInterPredFwdPicIDX4;
    bindingTable->dwAvcMBEncFwdPicBotField[4] = mbencVmeInterPredFwdPicIDX4;
    bindingTable->dwAvcMBEncFwdPicTopField[5] = mbencVmeInterPredFwdPicIDX5;
    bindingTable->dwAvcMBEncFwdPicBotField[5] = mbencVmeInterPredFwdPicIDX5;
    bindingTable->dwAvcMBEncFwdPicTopField[6] = mbencVmeInterPredFwdPicIDX6;
    bindingTable->dwAvcMBEncFwdPicBotField[6] = mbencVmeInterPredFwdPicIDX6;
    bindingTable->dwAvcMBEncFwdPicTopField[7] = mbencVmeInterPredFwdPicIDX7;
    bindingTable->dwAvcMBEncFwdPicBotField[7] = mbencVmeInterPredFwdPicIDX7;
    bindingTable->dwAvcMBEncFieldCurrPic[1]   = mbencVmeInterPredCurrPicIdx1;
    bindingTable->dwAvcMBEncBwdPicTopField[2] = mbencVmeInterPredBwdPicIDX01;
    bindingTable->dwAvcMBEncBwdPicBotField[2] = mbencVmeInterPredBwdPicIDX01;
    bindingTable->dwAvcMBEncBwdPicTopField[3] = mbencVmeInterPredBwdPicIDX11;
    bindingTable->dwAvcMBEncBwdPicBotField[3] = mbencVmeInterPredBwdPicIDX11;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params)
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

        uint32_t *dataPtr = (uint32_t *)params->pOsInterface->pfnLockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer,
            &lockFlags);
        if (dataPtr == nullptr)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

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

MOS_STATUS CodechalEncodeAvcEncG12::InitKernelStateWP()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    pWPKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);

    auto kernelStatePtr = pWPKernelState;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize));

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        kernelBinary,
        ENC_WP,
        0,
        &currKrnHeader,
        &kernelSize));
    kernelStatePtr->KernelParams.iBTCount          = wpNumSurfaces;
    kernelStatePtr->KernelParams.iThreadCount      = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength      = sizeof(CodechalEncodeAvcEncG12::WpCurbe);
    kernelStatePtr->KernelParams.iBlockWidth       = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight      = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount          = 1;
    kernelStatePtr->KernelParams.iInlineDataLength = 0;

    kernelStatePtr->dwCurbeOffset        = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize   = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::InitBrcConstantBuffer(PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);

    uint8_t tableIdx               = params->wPictureCodingType - 1;

    if (tableIdx >= 3)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid input parameter.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MOS_LOCK_PARAMS lockFlags;
    memset(&lockFlags, 0, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    auto dataPtr         = (uint8_t*)params->pOsInterface->pfnLockResource(
        params->pOsInterface,
        &params->sBrcConstantDataBuffer.OsResource,
        &lockFlags);
    CODECHAL_ENCODE_CHK_NULL_RETURN(dataPtr);

    memset(dataPtr, 0, params->sBrcConstantDataBuffer.dwWidth * params->sBrcConstantDataBuffer.dwHeight);

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table, Distortion QP Adjustment Table
    eStatus = MOS_SecureMemcpy(
        dataPtr,
        sizeof(m_QPAdjustmentDistThresholdMaxFrameThresholdIPB),
        (void*)m_QPAdjustmentDistThresholdMaxFrameThresholdIPB,
        sizeof(m_QPAdjustmentDistThresholdMaxFrameThresholdIPB));
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    dataPtr += sizeof(m_QPAdjustmentDistThresholdMaxFrameThresholdIPB);

    bool    blockBasedSkipEn        = params->dwMbEncBlockBasedSkipEn ? true : false;
    bool    transform_8x8_mode_flag = params->pPicParams->transform_8x8_mode_flag ? true : false;

    // Fill surface with Skip Threshold Table
    switch (params->wPictureCodingType)
    {
    case P_TYPE:
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstantsurfaceEarlySkipTableSize,
            (void*)&SkipVal_P_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            m_brcConstantsurfaceEarlySkipTableSize);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
        break;
    case B_TYPE:
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstantsurfaceEarlySkipTableSize,
            (void*)&SkipVal_B_Common[blockBasedSkipEn][transform_8x8_mode_flag][0],
            m_brcConstantsurfaceEarlySkipTableSize);
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
            *(dataPtr + 1 + (qp * 2)) = (uint8_t)CalcSkipVal((params->dwMbEncBlockBasedSkipEn ? true : false),
                                                                            (params->pPicParams->transform_8x8_mode_flag ? true : false),
                                                                            params->pAvcQCParams->NonFTQSkipThresholdLUT[qp]);
        }
    }

    dataPtr += m_brcConstantsurfaceEarlySkipTableSize;

    // Fill surface with QP list

    // Initialize to -1 (0xff)
    memset(dataPtr, 0xff, m_brcConstantsurfaceQpList0);
    memset(dataPtr + m_brcConstantsurfaceQpList0 + m_brcConstantsurfaceQpList0Reserved,
           0xff, m_brcConstantsurfaceQpList1);

    switch (params->wPictureCodingType)
    {
    case B_TYPE:
        dataPtr += (m_brcConstantsurfaceQpList0 + m_brcConstantsurfaceQpList0Reserved);

        for (uint8_t refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
        {
            CODEC_PICTURE refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                *(dataPtr + refIdx) = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            }
        }
        dataPtr -= (m_brcConstantsurfaceQpList0 + m_brcConstantsurfaceQpList0Reserved);
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

    dataPtr += (m_brcConstantsurfaceQpList0 + m_brcConstantsurfaceQpList0Reserved +
                m_brcConstantsurfaceQpList1 + m_brcConstantsurfaceQpList1Reserved);

    // Fill surface with Mode cost and MV cost
    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstantsurfaceModeMvCostSize,
        (void*)ModeMvCost_Cm[tableIdx],
        m_brcConstantsurfaceModeMvCostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    // If old mode cost is used the update the table
    if (params->wPictureCodingType == I_TYPE && params->bOldModeCostEnable)
    {
        auto dataTemp = (uint32_t *)dataPtr;
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW0 in each sub-array of 16 DWs
            *dataTemp = (uint32_t)OldIntraModeCost_Cm_Common[qp];
            dataTemp += 16;
        }
    }

    if (params->pAvcQCParams)
    {
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            if (params->pAvcQCParams->FTQSkipThresholdLUTInput)
            {
                // clang-format off
                *(dataPtr + (qp * 32) + 24) =
                *(dataPtr + (qp * 32) + 25) =
                *(dataPtr + (qp * 32) + 27) =
                *(dataPtr + (qp * 32) + 28) =
                *(dataPtr + (qp * 32) + 29) =
                *(dataPtr + (qp * 32) + 30) =
                *(dataPtr + (qp * 32) + 31) = params->pAvcQCParams->FTQSkipThresholdLUT[qp];
                // clang-format on
            }
        }
    }

    dataPtr += m_brcConstantsurfaceModeMvCostSize;

    // Fill surface with Refcost
    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstantsurfaceRefcostSize,
        (void*)&m_refCostMultiRefQp[tableIdx][0],
        m_brcConstantsurfaceRefcostSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }
    dataPtr += m_brcConstantsurfaceRefcostSize;

    //Fill surface with Intra cost scaling Factor
    if (params->bAdaptiveIntraScalingEnable)
    {
        eStatus = MOS_SecureMemcpy(
            dataPtr,
            m_brcConstantsurfaceIntracostScalingFactor,
            (void*)&AdaptiveIntraScalingFactor_Cm_Common[0],
            m_brcConstantsurfaceIntracostScalingFactor);
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
            m_brcConstantsurfaceIntracostScalingFactor,
            (void*)&IntraScalingFactor_Cm_Common[0],
            m_brcConstantsurfaceIntracostScalingFactor);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    dataPtr += m_brcConstantsurfaceIntracostScalingFactor;

    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstantsurfaceLambdaSize,
        (void*)&m_lambdaData[0],
        m_brcConstantsurfaceLambdaSize);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
        return eStatus;
    }

    dataPtr += m_brcConstantsurfaceLambdaSize;

    eStatus = MOS_SecureMemcpy(
        dataPtr,
        m_brcConstantsurfaceFtq25Size,
        (void*)&m_ftQ25[0],
        m_brcConstantsurfaceFtq25Size);
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

MOS_STATUS CodechalEncodeAvcEncG12::SetCurbeAvcMbEnc(PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pPicParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSeqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcParams);

    auto picParams = params->pPicParams;
    auto seqParams = params->pSeqParams;
    auto slcParams = params->pSlcParams;

    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    uint8_t meMethod = (m_pictureCodingType == B_TYPE) ? m_bMeMethodGeneric[seqParams->TargetUsage] : m_meMethodGeneric[seqParams->TargetUsage];
    // set sliceQP to MAX_SLICE_QP for  MbEnc kernel, we can use it to verify whether QP is changed or not
    uint8_t sliceQP      = (params->bUseMbEncAdvKernel && params->bBrcEnabled) ? CODECHAL_ENCODE_AVC_MAX_SLICE_QP : picParams->pic_init_qp_minus26 + 26 + slcParams->slice_qp_delta;
    bool    framePicture = CodecHal_PictureIsFrame(picParams->CurrOriginalPic);
    bool    bottomField  = CodecHal_PictureIsBottomField(picParams->CurrOriginalPic);

    CodechalEncodeAvcEncG12::MbencCurbe::MBEncCurbeInitType curbeInitType;
    if (params->bMbEncIFrameDistEnabled)
    {
        curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typeIDist;
    }
    else
    {
        switch (m_pictureCodingType)
        {
        case I_TYPE:
            if (framePicture)
            {
                curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typeIFrame;
            }
            else
            {
                curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typeIField;
            }
            break;

        case P_TYPE:
            if (framePicture)
            {
                curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typePFrame;
            }
            else
            {
                curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typePField;
            }
            break;
        case B_TYPE:
            if (framePicture)
            {
                curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typeBFrame;
            }
            else
            {
                curbeInitType = CodechalEncodeAvcEncG12::MbencCurbe::typeBField;
            }
            break;
        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }
    }

    CodechalEncodeAvcEncG12::MbencCurbe cmd;
    cmd.SetDefaultMbencCurbe(curbeInitType);
    // r1
    cmd.m_curbe.DW0.m_adaptiveEn =
        cmd.m_curbe.DW37.m_adaptiveEn = EnableAdaptiveSearch[seqParams->TargetUsage];
    cmd.m_curbe.DW0.m_t8x8FlagForInterEn =
        cmd.m_curbe.DW37.m_t8x8FlagForInterEn = picParams->transform_8x8_mode_flag;
    cmd.m_curbe.DW2.m_lenSP                   = MaxLenSP[seqParams->TargetUsage];

    cmd.m_curbe.DW1.m_extendedMvCostRange = bExtendedMvCostRange;
    cmd.m_curbe.DW36.m_mbInputEnable = bMbSpecificDataEnabled;

    cmd.m_curbe.DW38.m_lenSP = 0;  // MBZ
    cmd.m_curbe.DW3.m_srcAccess =
        cmd.m_curbe.DW3.m_refAccess = framePicture ? 0 : 1;
    if (m_pictureCodingType != I_TYPE && bFTQEnable)
    {
        if (m_pictureCodingType == P_TYPE)
        {
            cmd.m_curbe.DW3.m_fTEnable = FTQBasedSkip[seqParams->TargetUsage] & 0x01;
        }
        else  // B_TYPE
        {
            cmd.m_curbe.DW3.m_fTEnable = (FTQBasedSkip[seqParams->TargetUsage] >> 1) & 0x01;
        }
    }
    else
    {
        cmd.m_curbe.DW3.m_fTEnable = 0;
    }
    if (picParams->UserFlags.bDisableSubMBPartition)
    {
        cmd.m_curbe.DW3.m_subMbPartMask = CODECHAL_ENCODE_AVC_DISABLE_4X4_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_4X8_SUB_MB_PARTITION | CODECHAL_ENCODE_AVC_DISABLE_8X4_SUB_MB_PARTITION;
    }
    cmd.m_curbe.DW2.m_picWidth        = params->wPicWidthInMb;
    cmd.m_curbe.DW4.m_picHeightMinus1 = params->wFieldFrameHeightInMb - 1;
    cmd.m_curbe.DW4.m_fieldParityFlag = cmd.m_curbe.DW7.m_srcFieldPolarity = bottomField ? 1 : 0;
    cmd.m_curbe.DW4.m_enableFBRBypass                                              = bFBRBypassEnable;
    cmd.m_curbe.DW4.m_enableIntraCostScalingForStaticFrame                         = params->bStaticFrameDetectionEnabled;
    cmd.m_curbe.DW4.m_bCurFldIDR                                                   = framePicture ? 0 : (picParams->bIdrPic || m_firstFieldIdrPic);
    cmd.m_curbe.DW4.m_constrainedIntraPredFlag                                     = picParams->constrained_intra_pred_flag;
    cmd.m_curbe.DW4.m_hmeEnable                                                    = m_hmeEnabled;
    cmd.m_curbe.DW4.m_pictureType                                                  = m_pictureCodingType - 1;
    cmd.m_curbe.DW4.m_useActualRefQPValue                                          = m_hmeEnabled ? (m_multiRefDisableQPCheck[seqParams->TargetUsage] == 0) : false;
    cmd.m_curbe.DW5.m_sliceMbHeight                                                = params->usSliceHeight;
    cmd.m_curbe.DW7.m_intraPartMask                                                = picParams->transform_8x8_mode_flag ? 0 : 0x2;  // Disable 8x8 if flag is not set

    // r2
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.m_curbe.DW6.m_batchBufferEnd = 0;
    }
    else
    {
        uint8_t tableIdx = m_pictureCodingType - 1;
        eStatus          = MOS_SecureMemcpy(&(cmd.m_curbe.ModeMvCost), 8 * sizeof(uint32_t), ModeMvCost_Cm[tableIdx][sliceQP], 8 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }

        if (m_pictureCodingType == I_TYPE && bOldModeCostEnable)
        {
            // Old intra mode cost needs to be used if bOldModeCostEnable is 1
            cmd.m_curbe.ModeMvCost.DW8.m_value = OldIntraModeCost_Cm_Common[sliceQP];
        }
        else if (m_skipBiasAdjustmentEnable)
        {
            // Load different MvCost for P picture when SkipBiasAdjustment is enabled
            // No need to check for P picture as the flag is only enabled for P picture
            cmd.m_curbe.ModeMvCost.DW11.m_value = MvCost_PSkipAdjustment_Cm_Common[sliceQP];
        }
    }

    uint8_t tableIdx;
    // r3 & r4
    if (params->bMbEncIFrameDistEnabled)
    {
        cmd.m_curbe.SPDelta.DW31.m_intraComputeType = 1;
    }
    else
    {
        tableIdx = (m_pictureCodingType == B_TYPE) ? 1 : 0;
        eStatus  = MOS_SecureMemcpy(&(cmd.m_curbe.SPDelta), 16 * sizeof(uint32_t), m_encodeSearchPath[tableIdx][meMethod], 16 * sizeof(uint32_t));
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to copy memory.");
            return eStatus;
        }
    }

    // r5
    if (m_pictureCodingType == P_TYPE)
    {
        cmd.m_curbe.DW32.m_skipVal = SkipVal_P_Common
            [cmd.m_curbe.DW3.m_blockBasedSkipEnable]
            [picParams->transform_8x8_mode_flag]
            [sliceQP];
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        cmd.m_curbe.DW32.m_skipVal = SkipVal_B_Common
            [cmd.m_curbe.DW3.m_blockBasedSkipEnable]
            [picParams->transform_8x8_mode_flag]
            [sliceQP];
    }

    cmd.m_curbe.ModeMvCost.DW13.m_qpPrimeY = sliceQP;
    // m_qpPrimeCb and m_qpPrimeCr are not used by Kernel. Following settings are for CModel matching.
    cmd.m_curbe.ModeMvCost.DW13.m_qpPrimeCb        = sliceQP;
    cmd.m_curbe.ModeMvCost.DW13.m_qpPrimeCr        = sliceQP;
    cmd.m_curbe.ModeMvCost.DW13.m_targetSizeInWord = 0xff;  // hardcoded for BRC disabled

    if (bMultiPredEnable && (m_pictureCodingType != I_TYPE))
    {
        switch (m_multiPred[seqParams->TargetUsage])
        {
        case 0:  // Disable multipred for both P & B picture types
            cmd.m_curbe.DW32.m_multiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_curbe.DW32.m_multiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 1:  // Enable multipred for P pictures only
            cmd.m_curbe.DW32.m_multiPredL0Disable = (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_curbe.DW32.m_multiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 2:  // Enable multipred for B pictures only
            cmd.m_curbe.DW32.m_multiPredL0Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            cmd.m_curbe.DW32.m_multiPredL1Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;

        case 3:  // Enable multipred for both P & B picture types
            cmd.m_curbe.DW32.m_multiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE;
            cmd.m_curbe.DW32.m_multiPredL1Disable = (m_pictureCodingType == B_TYPE) ? CODECHAL_ENCODE_AVC_MULTIPRED_ENABLE : CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
            break;
        }
    }
    else
    {
        cmd.m_curbe.DW32.m_multiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
        cmd.m_curbe.DW32.m_multiPredL1Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;
    }

    if (!framePicture)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.m_curbe.DW34.m_list0RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_0);
            cmd.m_curbe.DW34.m_list0RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_1);
            cmd.m_curbe.DW34.m_list0RefID2FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_2);
            cmd.m_curbe.DW34.m_list0RefID3FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_3);
            cmd.m_curbe.DW34.m_list0RefID4FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_4);
            cmd.m_curbe.DW34.m_list0RefID5FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_5);
            cmd.m_curbe.DW34.m_list0RefID6FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_6);
            cmd.m_curbe.DW34.m_list0RefID7FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        }
        if (m_pictureCodingType == B_TYPE)
        {
            cmd.m_curbe.DW34.m_list1RefID0FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_0);
            cmd.m_curbe.DW34.m_list1RefID1FieldParity = CodecHalAvcEncode_GetFieldParity(slcParams, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        }
    }

    if (m_adaptiveTransformDecisionEnabled)
    {
        if (m_pictureCodingType != I_TYPE)
        {
            cmd.m_curbe.DW34.m_enableAdaptiveTxDecision = true;
        }
        cmd.m_curbe.DW60.m_txDecisonThreshold = m_adaptiveTxDecisionThreshold;
    }

    if (m_adaptiveTransformDecisionEnabled || m_flatnessCheckEnabled)
    {
        cmd.m_curbe.DW60.m_mbTextureThreshold = m_mbTextureThreshold;
    }

    if (m_pictureCodingType == B_TYPE)
    {
        cmd.m_curbe.DW34.m_list1RefID0FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_curbe.DW34.m_list1RefID1FrameFieldFlag = GetRefPicFieldFlag(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
        cmd.m_curbe.DW34.m_bDirectMode               = slcParams->direct_spatial_mv_pred_flag;
    }

    cmd.m_curbe.DW34.m_enablePerMBStaticCheck          = params->bStaticFrameDetectionEnabled;
    cmd.m_curbe.DW34.m_enableAdaptiveSearchWindowSize  = params->bApdatvieSearchWindowSizeEnabled;
    cmd.m_curbe.DW34.m_removeIntraRefreshOverlap       = picParams->bDisableRollingIntraRefreshOverlap;
    cmd.m_curbe.DW34.m_bOriginalBff                    = framePicture ? 0 : ((m_firstField && (bottomField)) || (!m_firstField && (!bottomField)));
    cmd.m_curbe.DW34.m_enableMBFlatnessChkOptimization = m_flatnessCheckEnabled;
    cmd.m_curbe.DW34.m_roiEnableFlag                   = params->bRoiEnabled;
    cmd.m_curbe.DW34.m_madEnableFlag                   = m_madEnabled;
    cmd.m_curbe.DW34.m_mbBrcEnable                     = bMbBrcEnabled || bMbQpDataEnabled;
    cmd.m_curbe.DW34.m_arbitraryNumMbsPerSlice         = m_arbitraryNumMbsInSlice;
    cmd.m_curbe.DW34.m_tqEnable                        = m_trellisQuantParams.dwTqEnabled;  //Enabled for KBL
    cmd.m_curbe.DW34.m_forceNonSkipMbEnable            = params->bMbDisableSkipMapEnabled;
    if (params->pAvcQCParams && !cmd.m_curbe.DW34.m_forceNonSkipMbEnable)  // ignore DisableEncSkipCheck if Mb Disable Skip Map is available
    {
        cmd.m_curbe.DW34.m_disableEncSkipCheck = params->pAvcQCParams->skipCheckDisable;
        }
        cmd.m_curbe.DW34.m_cqpFlag                    = !bBrcEnabled;  // 1 - Rate Control is CQP, 0 - Rate Control is BRC
        cmd.m_curbe.DW36.m_checkAllFractionalEnable   = bCAFEnable;
        cmd.m_curbe.DW38.m_refThreshold               = m_refThreshold;
        cmd.m_curbe.DW39.m_hmeRefWindowsCombThreshold = (m_pictureCodingType == B_TYPE) ? HMEBCombineLen[seqParams->TargetUsage] : HMECombineLen[seqParams->TargetUsage];

        // Default:2 used for MBBRC (MB QP Surface width and height are 4x downscaled picture in MB unit * 4  bytes)
        // 0 used for MBQP data surface (MB QP Surface width and height are same as the input picture size in MB unit * 1bytes)
        // BRC use split kernel, MB QP surface is same size as input picture
        cmd.m_curbe.DW47.m_mbQpReadFactor = (bMbBrcEnabled || bMbQpDataEnabled) ? 0 : 2;

        // Those fields are not really used for I_dist kernel
        if (params->bMbEncIFrameDistEnabled)
        {
            cmd.m_curbe.ModeMvCost.DW13.m_qpPrimeY        = 0;
            cmd.m_curbe.ModeMvCost.DW13.m_qpPrimeCb       = 0;
            cmd.m_curbe.ModeMvCost.DW13.m_qpPrimeCr       = 0;
            cmd.m_curbe.DW33.m_intra16x16NonDCPredPenalty = 0;
            cmd.m_curbe.DW33.m_intra4x4NonDCPredPenalty   = 0;
            cmd.m_curbe.DW33.m_intra8x8NonDCPredPenalty   = 0;
    }

    //r6
    if (cmd.m_curbe.DW4.m_useActualRefQPValue)
    {
        cmd.m_curbe.DW44.m_actualQPValueForRefID0List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_curbe.DW44.m_actualQPValueForRefID1List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_1);
        cmd.m_curbe.DW44.m_actualQPValueForRefID2List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_2);
        cmd.m_curbe.DW44.m_actualQPValueForRefID3List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_3);
        cmd.m_curbe.DW45.m_actualQPValueForRefID4List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_4);
        cmd.m_curbe.DW45.m_actualQPValueForRefID5List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_5);
        cmd.m_curbe.DW45.m_actualQPValueForRefID6List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_6);
        cmd.m_curbe.DW45.m_actualQPValueForRefID7List0 = AVCGetQPValueFromRefList(params, LIST_0, CODECHAL_ENCODE_REF_ID_7);
        cmd.m_curbe.DW46.m_actualQPValueForRefID0List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_0);
        cmd.m_curbe.DW46.m_actualQPValueForRefID1List1 = AVCGetQPValueFromRefList(params, LIST_1, CODECHAL_ENCODE_REF_ID_1);
    }

    tableIdx           = m_pictureCodingType - 1;
    cmd.m_curbe.DW46.m_refCost = m_refCostMultiRefQp[tableIdx][sliceQP];

    // Picture Coding Type dependent parameters
    if (m_pictureCodingType == I_TYPE)
    {
        cmd.m_curbe.DW0.m_skipModeEn                  = 0;
        cmd.m_curbe.DW37.m_skipModeEn                 = 0;
        cmd.m_curbe.DW36.m_hmeCombineOverlap          = 0;
        cmd.m_curbe.DW47.m_intraCostSF                = 16;  // This is not used but recommended to set this to 16 by Kernel team
        cmd.m_curbe.DW34.m_enableDirectBiasAdjustment = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        cmd.m_curbe.DW1.m_maxNumMVs                   = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.m_curbe.DW3.m_bmeDisableFBR               = 1;
        cmd.m_curbe.DW5.m_refWidth                    = SearchX[seqParams->TargetUsage];
        cmd.m_curbe.DW5.m_refHeight                   = SearchY[seqParams->TargetUsage];
        cmd.m_curbe.DW7.m_nonSkipZMvAdded             = 1;
        cmd.m_curbe.DW7.m_nonSkipModeAdded            = 1;
        cmd.m_curbe.DW7.m_skipCenterMask              = 1;
        cmd.m_curbe.DW47.m_intraCostSF                = bAdaptiveIntraScalingEnable ? AdaptiveIntraScalingFactor_Cm_Common[sliceQP] : IntraScalingFactor_Cm_Common[sliceQP];
        cmd.m_curbe.DW47.m_maxVmvR                    = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.m_curbe.DW36.m_hmeCombineOverlap          = 1;
        cmd.m_curbe.DW36.m_numRefIdxL0MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.m_curbe.DW39.m_refWidth                   = SearchX[seqParams->TargetUsage];
        cmd.m_curbe.DW39.m_refHeight                  = SearchY[seqParams->TargetUsage];
        cmd.m_curbe.DW34.m_enableDirectBiasAdjustment = 0;
        if (params->pAvcQCParams)
        {
            cmd.m_curbe.DW34.m_enableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }
    else
    {
        // B_TYPE
        cmd.m_curbe.DW1.m_maxNumMVs      = GetMaxMvsPer2Mb(seqParams->Level) / 2;
        cmd.m_curbe.DW1.m_biWeight       = m_biWeight;
        cmd.m_curbe.DW3.m_searchCtrl     = 7;
        cmd.m_curbe.DW3.m_skipType       = 1;
        cmd.m_curbe.DW5.m_refWidth       = BSearchX[seqParams->TargetUsage];
        cmd.m_curbe.DW5.m_refHeight      = BSearchY[seqParams->TargetUsage];
        cmd.m_curbe.DW7.m_skipCenterMask = 0xFF;
        cmd.m_curbe.DW47.m_intraCostSF =
            bAdaptiveIntraScalingEnable ? AdaptiveIntraScalingFactor_Cm_Common[sliceQP] : IntraScalingFactor_Cm_Common[sliceQP];
        cmd.m_curbe.DW47.m_maxVmvR           = (framePicture) ? CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) * 4 : (CodecHalAvcEncode_GetMaxMvLen(seqParams->Level) >> 1) * 4;
        cmd.m_curbe.DW36.m_hmeCombineOverlap = 1;
        // Checking if the forward frame (List 1 index 0) is a short term reference
        {
            auto codecHalPic = params->pSlcParams->RefPicList[LIST_1][0];
            if (codecHalPic.PicFlags != PICTURE_INVALID &&
                codecHalPic.FrameIdx != CODECHAL_ENCODE_AVC_INVALID_PIC_ID &&
                params->pPicIdx[codecHalPic.FrameIdx].bValid)
            {
                // Although its name is FWD, it actually means the future frame or the backward reference frame
                cmd.m_curbe.DW36.m_isFwdFrameShortTermRef = CodecHal_PictureIsShortTermRef(params->pPicParams->RefFrameList[codecHalPic.FrameIdx]);
            }
            else
            {
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid backward reference frame.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                return eStatus;
            }
        }
        cmd.m_curbe.DW36.m_numRefIdxL0MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l0_active_minus1 : 0;
        cmd.m_curbe.DW36.m_numRefIdxL1MinusOne        = bMultiPredEnable ? slcParams->num_ref_idx_l1_active_minus1 : 0;
        cmd.m_curbe.DW39.m_refWidth                   = BSearchX[seqParams->TargetUsage];
        cmd.m_curbe.DW39.m_refHeight                  = BSearchY[seqParams->TargetUsage];
        cmd.m_curbe.DW40.m_distScaleFactorRefID0List0 = m_distScaleFactorList0[0];
        cmd.m_curbe.DW40.m_distScaleFactorRefID1List0 = m_distScaleFactorList0[1];
        cmd.m_curbe.DW41.m_distScaleFactorRefID2List0 = m_distScaleFactorList0[2];
        cmd.m_curbe.DW41.m_distScaleFactorRefID3List0 = m_distScaleFactorList0[3];
        cmd.m_curbe.DW42.m_distScaleFactorRefID4List0 = m_distScaleFactorList0[4];
        cmd.m_curbe.DW42.m_distScaleFactorRefID5List0 = m_distScaleFactorList0[5];
        cmd.m_curbe.DW43.m_distScaleFactorRefID6List0 = m_distScaleFactorList0[6];
        cmd.m_curbe.DW43.m_distScaleFactorRefID7List0 = m_distScaleFactorList0[7];
        if (params->pAvcQCParams)
        {
            cmd.m_curbe.DW34.m_enableDirectBiasAdjustment = params->pAvcQCParams->directBiasAdjustmentEnable;
            if (cmd.m_curbe.DW34.m_enableDirectBiasAdjustment)
            {
                cmd.m_curbe.DW7.m_nonSkipModeAdded = 1;
                cmd.m_curbe.DW7.m_nonSkipZMvAdded  = 1;
            }

            cmd.m_curbe.DW34.m_enableGlobalMotionBiasAdjustment = params->pAvcQCParams->globalMotionBiasAdjustmentEnable;
        }
    }

    *params->pdwBlockBasedSkipEn = cmd.m_curbe.DW3.m_blockBasedSkipEnable;

    if (picParams->EnableRollingIntraRefresh)
    {
        cmd.m_curbe.DW34.m_IntraRefreshEn = picParams->EnableRollingIntraRefresh;

        /* Multiple predictor should be completely disabled for the RollingI feature. This does not lead to much quality drop for P frames especially for TU as 1 */
        cmd.m_curbe.DW32.m_multiPredL0Disable = CODECHAL_ENCODE_AVC_MULTIPRED_DISABLE;

        /* Pass the same IntraRefreshUnit to the kernel w/o the adjustment by -1, so as to have an overlap of one MB row or column of Intra macroblocks
        across one P frame to another P frame, as needed by the RollingI algo */
        if (ROLLING_I_SQUARE == picParams->EnableRollingIntraRefresh && RATECONTROL_CQP != seqParams->RateControlMethod)
        {
            /*BRC update kernel updates these CURBE to MBEnc*/
            cmd.m_curbe.DW4.m_enableIntraRefresh = false;
            cmd.m_curbe.DW34.m_IntraRefreshEn    = ROLLING_I_DISABLED;
            cmd.m_curbe.DW48.m_IntraRefreshMBx   = 0; /* MB column number */
            cmd.m_curbe.DW61.m_IntraRefreshMBy   = 0; /* MB row number */
        }
        else
        {
            cmd.m_curbe.DW4.m_enableIntraRefresh = true;
            cmd.m_curbe.DW34.m_IntraRefreshEn    = picParams->EnableRollingIntraRefresh;
            cmd.m_curbe.DW48.m_IntraRefreshMBx   = picParams->IntraRefreshMBx; /* MB column number */
            cmd.m_curbe.DW61.m_IntraRefreshMBy   = picParams->IntraRefreshMBy; /* MB row number */
        }
        cmd.m_curbe.DW48.m_IntraRefreshUnitInMBMinus1 = picParams->IntraRefreshUnitinMB;
        cmd.m_curbe.DW48.m_IntraRefreshQPDelta        = picParams->IntraRefreshQPDelta;
    }
    else
    {
        cmd.m_curbe.DW34.m_IntraRefreshEn = 0;
    }

    if (params->bRoiEnabled)
    {
        cmd.m_curbe.DW49.m_roi1XLeft   = picParams->ROI[0].Left;
        cmd.m_curbe.DW49.m_roi1YTop    = picParams->ROI[0].Top;
        cmd.m_curbe.DW50.m_roi1XRight  = picParams->ROI[0].Right;
        cmd.m_curbe.DW50.m_roi1YBottom = picParams->ROI[0].Bottom;

        cmd.m_curbe.DW51.m_roi2XLeft   = picParams->ROI[1].Left;
        cmd.m_curbe.DW51.m_roi2YTop    = picParams->ROI[1].Top;
        cmd.m_curbe.DW52.m_roi2XRight  = picParams->ROI[1].Right;
        cmd.m_curbe.DW52.m_roi2YBottom = picParams->ROI[1].Bottom;

        cmd.m_curbe.DW53.m_roi3XLeft   = picParams->ROI[2].Left;
        cmd.m_curbe.DW53.m_roi3YTop    = picParams->ROI[2].Top;
        cmd.m_curbe.DW54.m_roi3XRight  = picParams->ROI[2].Right;
        cmd.m_curbe.DW54.m_roi3YBottom = picParams->ROI[2].Bottom;

        cmd.m_curbe.DW55.m_roi4XLeft   = picParams->ROI[3].Left;
        cmd.m_curbe.DW55.m_roi4YTop    = picParams->ROI[3].Top;
        cmd.m_curbe.DW56.m_roi4XRight  = picParams->ROI[3].Right;
        cmd.m_curbe.DW56.m_roi4YBottom = picParams->ROI[3].Bottom;

        if (bBrcEnabled == false)
        {
            uint16_t numROI                                                 = picParams->NumROI;
            int8_t   priorityLevelOrDQp[CODECHAL_ENCODE_AVC_MAX_ROI_NUMBER] = {0};

            // cqp case
            for (unsigned int i = 0; i < numROI; i += 1)
            {
                int8_t dQpRoi = picParams->ROI[i].PriorityLevelOrDQp;

                // clip qp roi in order to have (qp + qpY) in range [0, 51]
                priorityLevelOrDQp[i] = (int8_t)CodecHal_Clip3(-sliceQP, CODECHAL_ENCODE_AVC_MAX_SLICE_QP - sliceQP, dQpRoi);
            }

            cmd.m_curbe.DW57.m_roi1dQpPrimeY = priorityLevelOrDQp[0];
            cmd.m_curbe.DW57.m_roi2dQpPrimeY = priorityLevelOrDQp[1];
            cmd.m_curbe.DW57.m_roi3dQpPrimeY = priorityLevelOrDQp[2];
            cmd.m_curbe.DW57.m_roi4dQpPrimeY = priorityLevelOrDQp[3];
        }
        else
        {
            // kernel does not support BRC case
            cmd.m_curbe.DW34.m_roiEnableFlag = 0;
        }
    }
    else if (params->bDirtyRoiEnabled)
    {
        // enable Dirty Rect flag
        cmd.m_curbe.DW4.m_enableDirtyRect = true;

        cmd.m_curbe.DW49.m_roi1XLeft   = params->pPicParams->DirtyROI[0].Left;
        cmd.m_curbe.DW49.m_roi1YTop    = params->pPicParams->DirtyROI[0].Top;
        cmd.m_curbe.DW50.m_roi1XRight  = params->pPicParams->DirtyROI[0].Right;
        cmd.m_curbe.DW50.m_roi1YBottom = params->pPicParams->DirtyROI[0].Bottom;

        cmd.m_curbe.DW51.m_roi2XLeft   = params->pPicParams->DirtyROI[1].Left;
        cmd.m_curbe.DW51.m_roi2YTop    = params->pPicParams->DirtyROI[1].Top;
        cmd.m_curbe.DW52.m_roi2XRight  = params->pPicParams->DirtyROI[1].Right;
        cmd.m_curbe.DW52.m_roi2YBottom = params->pPicParams->DirtyROI[1].Bottom;

        cmd.m_curbe.DW53.m_roi3XLeft   = params->pPicParams->DirtyROI[2].Left;
        cmd.m_curbe.DW53.m_roi3YTop    = params->pPicParams->DirtyROI[2].Top;
        cmd.m_curbe.DW54.m_roi3XRight  = params->pPicParams->DirtyROI[2].Right;
        cmd.m_curbe.DW54.m_roi3YBottom = params->pPicParams->DirtyROI[2].Bottom;

        cmd.m_curbe.DW55.m_roi4XLeft   = params->pPicParams->DirtyROI[3].Left;
        cmd.m_curbe.DW55.m_roi4YTop    = params->pPicParams->DirtyROI[3].Top;
        cmd.m_curbe.DW56.m_roi4XRight  = params->pPicParams->DirtyROI[3].Right;
        cmd.m_curbe.DW56.m_roi4YBottom = params->pPicParams->DirtyROI[3].Bottom;
    }

    if (m_trellisQuantParams.dwTqEnabled)
    {
        // Lambda values for TQ
        if (m_pictureCodingType == I_TYPE)
        {
            cmd.m_curbe.DW58.m_value = TQ_LAMBDA_I_FRAME[sliceQP][0];
            cmd.m_curbe.DW59.m_value = TQ_LAMBDA_I_FRAME[sliceQP][1];
        }
        else if (m_pictureCodingType == P_TYPE)
        {
            cmd.m_curbe.DW58.m_value = TQ_LAMBDA_P_FRAME[sliceQP][0];
            cmd.m_curbe.DW59.m_value = TQ_LAMBDA_P_FRAME[sliceQP][1];
        }
        else
        {
            cmd.m_curbe.DW58.m_value = TQ_LAMBDA_B_FRAME[sliceQP][0];
            cmd.m_curbe.DW59.m_value = TQ_LAMBDA_B_FRAME[sliceQP][1];
        }

        MHW_VDBOX_AVC_SLICE_STATE sliceState;
        MOS_ZeroMemory(&sliceState, sizeof(sliceState));
        sliceState.pEncodeAvcSeqParams   = seqParams;
        sliceState.pEncodeAvcPicParams   = picParams;
        sliceState.pEncodeAvcSliceParams = slcParams;

        // check if Lambda is greater than max value
        CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

        if (cmd.m_curbe.DW58.m_lambda8x8Inter > m_maxLambda)
        {
            cmd.m_curbe.DW58.m_lambda8x8Inter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.m_curbe.DW58.m_lambda8x8Intra > m_maxLambda)
        {
            cmd.m_curbe.DW58.m_lambda8x8Intra = 0xf000 + m_defaultTrellisQuantIntraRounding;
        }

        // check if Lambda is greater than max value
        if (cmd.m_curbe.DW59.m_lambdaInter > m_maxLambda)
        {
            cmd.m_curbe.DW59.m_lambdaInter = 0xf000 + sliceState.dwRoundingValue;
        }

        if (cmd.m_curbe.DW59.m_lambdaIntra > m_maxLambda)
        {
            cmd.m_curbe.DW59.m_lambdaIntra = 0xf000 + m_defaultTrellisQuantIntraRounding;
        }
    }

    //IPCM QP and threshold
    cmd.m_curbe.DW62.m_IPCMQP0 = m_IPCMThresholdTable[0].QP;
    cmd.m_curbe.DW62.m_IPCMQP1 = m_IPCMThresholdTable[1].QP;
    cmd.m_curbe.DW62.m_IPCMQP2 = m_IPCMThresholdTable[2].QP;
    cmd.m_curbe.DW62.m_IPCMQP3 = m_IPCMThresholdTable[3].QP;
    cmd.m_curbe.DW63.m_IPCMQP4 = m_IPCMThresholdTable[4].QP;

    cmd.m_curbe.DW63.m_IPCMThresh0 = m_IPCMThresholdTable[0].Threshold;
    cmd.m_curbe.DW64.m_IPCMThresh1 = m_IPCMThresholdTable[1].Threshold;
    cmd.m_curbe.DW64.m_IPCMThresh2 = m_IPCMThresholdTable[2].Threshold;
    cmd.m_curbe.DW65.m_IPCMThresh3 = m_IPCMThresholdTable[3].Threshold;
    cmd.m_curbe.DW65.m_IPCMThresh4 = m_IPCMThresholdTable[4].Threshold;

    cmd.m_curbe.DW66.m_mbDataSurfIndex               = mbencMfcAvcPakObj;
    cmd.m_curbe.DW67.m_mvDataSurfIndex               = mbencIndMvData;
    cmd.m_curbe.DW68.m_IDistSurfIndex                = mbencBrcDistortion;
    cmd.m_curbe.DW69.m_srcYSurfIndex                 = mbencCurrY;
    cmd.m_curbe.DW70.m_mbSpecificDataSurfIndex       = mbencMbSpecificData;
    cmd.m_curbe.DW71.m_auxVmeOutSurfIndex            = mbencAuxVmeOut;
    cmd.m_curbe.DW72.m_currRefPicSelSurfIndex        = mbencRefpicselectL0;
    cmd.m_curbe.DW73.m_hmeMVPredFwdBwdSurfIndex      = mbencMvDataFromMe;
    cmd.m_curbe.DW74.m_hmeDistSurfIndex              = mbenc4xMeDistortion;
    cmd.m_curbe.DW75.m_sliceMapSurfIndex             = mbencSlicemapData;
    cmd.m_curbe.DW76.m_fwdFrmMBDataSurfIndex         = mbencFwdMbData;
    cmd.m_curbe.DW77.m_fwdFrmMVSurfIndex             = mbencFwdMvData;
    cmd.m_curbe.DW78.m_mbQPBuffer                    = mbencMbqp;
    cmd.m_curbe.DW79.m_mbBRCLut                      = mbencMbbrcConstData;
    cmd.m_curbe.DW80.m_vmeInterPredictionSurfIndex   = mbencVmeInterPredCurrPicIdx0;
    cmd.m_curbe.DW81.m_vmeInterPredictionMRSurfIndex = mbencVmeInterPredCurrPicIdx1;
    cmd.m_curbe.DW82.m_mbStatsSurfIndex              = mbencMbStats;
    cmd.m_curbe.DW83.m_madSurfIndex                  = mbencMadData;
    cmd.m_curbe.DW84.m_brcCurbeSurfIndex             = mbencBrcCurbeData;
    cmd.m_curbe.DW85.m_forceNonSkipMBmapSurface      = mbencForceNonskipMbMap;
    cmd.m_curbe.DW86.m_reservedIndex                 = mbEncAdv;
    cmd.m_curbe.DW87.m_staticDetectionCostTableIndex = mbencSfdCostTable;
    cmd.m_curbe.DW88.m_swScoreboardIndex             = mbencSwScoreboard;

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

MOS_STATUS CodechalEncodeAvcEncG12::SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    int16_t      weight;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto slcParams   = m_avcSliceParams;
    auto seqParams   = m_avcSeqParam;
    auto kernelState = pWPKernelState;
    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    CodechalEncodeAvcEncG12::WpCurbe cmd;

    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    weight = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][0];
        cmd.m_wpCurbeCmd.DW0.m_defaultWeight = (weight << 6) >> (slcParams->luma_log2_weight_denom);
        cmd.m_wpCurbeCmd.DW0.m_defaultOffset = slcParams->Weights[params->RefPicListIdx][0][0][1];

        cmd.m_wpCurbeCmd.DW49.m_inputSurface  = wpInputRefSurface;
        cmd.m_wpCurbeCmd.DW50.m_outputSurface = wpOutputScaledSurface;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
            &cmd,
            kernelState->dwCurbeOffset,
            sizeof(cmd)));

        return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto     seqParams = m_avcSeqParam;
    auto     vuiParams = m_avcVuiParams;
    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    BrcInitResetCurbe curbe;
    curbe.m_brcInitResetCurbeCmd.m_dw0.m_profileLevelMaxFrame = profileLevelMaxFrame;
    curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits    = seqParams->InitVBVBufferFullnessInBit;
    curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits        = seqParams->VBVBufferSizeInBit;
    curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate       = seqParams->TargetBitRate;
    curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate           = seqParams->MaxBitRate;
    curbe.m_brcInitResetCurbeCmd.m_dw8.m_gopP =
        (seqParams->GopRefDist) ? ((seqParams->GopPicSize - 1) / seqParams->GopRefDist) : 0;
    curbe.m_brcInitResetCurbeCmd.m_dw9.m_gopB                = seqParams->GopPicSize - 1 - curbe.m_brcInitResetCurbeCmd.m_dw8.m_gopP;
    curbe.m_brcInitResetCurbeCmd.m_dw9.m_frameWidthInBytes   = m_frameWidth;
    curbe.m_brcInitResetCurbeCmd.m_dw10.m_frameHeightInBytes = m_frameHeight;
    curbe.m_brcInitResetCurbeCmd.m_dw12.m_noSlices           = m_numSlices;

    curbe.m_brcInitResetCurbeCmd.m_dw32.m_surfaceIndexHistorybuffer    = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_HISTORY;
    curbe.m_brcInitResetCurbeCmd.m_dw33.m_surfaceIndexDistortionbuffer = CODECHAL_ENCODE_AVC_BRC_INIT_RESET_DISTORTION;

    // if VUI present, VUI data has high priority
    if (seqParams->vui_parameters_present_flag && seqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate =
            ((vuiParams->bit_rate_value_minus1[0] + 1) << (6 + vuiParams->bit_rate_scale));

        if (seqParams->RateControlMethod == RATECONTROL_CBR)
        {
            curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate = curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate;
        }
    }

    curbe.m_brcInitResetCurbeCmd.m_dw6.m_frameRateM = seqParams->FramesPer100Sec;
    curbe.m_brcInitResetCurbeCmd.m_dw7.m_frameRateD = 100;
    curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag    = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag |= (bMbBrcEnabled && !bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate = curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate;
        curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag    = curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate < curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate)
        {
            curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate = curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate;  // Use max bit rate for HRD compliance
        }
        curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag = curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag = curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate = curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag = curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        curbe.m_brcInitResetCurbeCmd.m_dw23.m_aCQP   = seqParams->ICQQualityFactor;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag = curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate < curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate)
        {
            curbe.m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate = curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate;  // Use max bit rate for HRD compliance
        }
        curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag = curbe.m_brcInitResetCurbeCmd.m_dw8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        curbe.m_brcInitResetCurbeCmd.m_dw23.m_aCQP = seqParams->ICQQualityFactor;
    }

    curbe.m_brcInitResetCurbeCmd.m_dw10.m_avbrAccuracy    = usAVBRAccuracy;
    curbe.m_brcInitResetCurbeCmd.m_dw11.m_avbrConvergence = usAVBRConvergence;

    // Set dynamic thresholds
    double inputBitsPerFrame =
        ((double)(curbe.m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate) * (double)(curbe.m_brcInitResetCurbeCmd.m_dw7.m_frameRateD) /
            (double)(curbe.m_brcInitResetCurbeCmd.m_dw6.m_frameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        inputBitsPerFrame *= 0.5;
    }

    if (curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits == 0)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits = (uint32_t)inputBitsPerFrame * 4;
    }

    if (curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits == 0)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits = 7 * curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits / 8;
    }
    if (curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits < (uint32_t)(inputBitsPerFrame * 2))
    {
        curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits = (uint32_t)(inputBitsPerFrame * 2);
    }
    if (curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits > curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits)
    {
        curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits = curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits;
    }

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits     = 2 * seqParams->TargetBitRate;
        curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits = (uint32_t)(0.75 * curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits);
    }

    double bpsRatio = inputBitsPerFrame / ((double)(curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits) / 30);
    bpsRatio        = (bpsRatio < 0.1) ? 0.1 : (bpsRatio > 3.5) ? 3.5 : bpsRatio;

    curbe.m_brcInitResetCurbeCmd.m_dw16.m_deviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw16.m_deviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw16.m_deviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw16.m_deviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw17.m_deviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw17.m_deviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw17.m_deviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw17.m_deviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw18.m_deviationThreshold0ForVBR   = (uint32_t)(-50 * pow(0.9, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw18.m_deviationThreshold1ForVBR   = (uint32_t)(-50 * pow(0.7, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw18.m_deviationThreshold2ForVBR   = (uint32_t)(-50 * pow(0.5, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw18.m_deviationThreshold3ForVBR   = (uint32_t)(-50 * pow(0.3, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw19.m_deviationThreshold4ForVBR   = (uint32_t)(100 * pow(0.4, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw19.m_deviationThreshold5ForVBR   = (uint32_t)(100 * pow(0.5, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw19.m_deviationThreshold6ForVBR   = (uint32_t)(100 * pow(0.75, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw19.m_deviationThreshold7ForVBR   = (uint32_t)(100 * pow(0.9, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw20.m_deviationThreshold0ForI     = (uint32_t)(-50 * pow(0.8, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw20.m_deviationThreshold1ForI     = (uint32_t)(-50 * pow(0.6, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw20.m_deviationThreshold2ForI     = (uint32_t)(-50 * pow(0.34, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw20.m_deviationThreshold3ForI     = (uint32_t)(-50 * pow(0.2, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw21.m_deviationThreshold4ForI     = (uint32_t)(50 * pow(0.2, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw21.m_deviationThreshold5ForI     = (uint32_t)(50 * pow(0.4, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw21.m_deviationThreshold6ForI     = (uint32_t)(50 * pow(0.66, bpsRatio));
    curbe.m_brcInitResetCurbeCmd.m_dw21.m_deviationThreshold7ForI     = (uint32_t)(50 * pow(0.9, bpsRatio));

    curbe.m_brcInitResetCurbeCmd.m_dw22.m_slidingWindowSize = dwSlidingWindowSize;

    if (bBrcInit)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = curbe.m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits    = curbe.m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = inputBitsPerFrame;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe.m_brcInitResetCurbeCmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe.m_brcInitResetCurbeCmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            &curbe.m_brcInitResetCurbeCmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CodechalEncodeAvcEncG12::FrameBrcUpdateCurbe cmd;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_targetSizeFlag = 0;
    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_targetSizeFlag = 1;
    }

    // skipped frame handling
    if (params->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_numSkipFrames  = params->dwNumSkipFrames;
        cmd.m_frameBrcUpdateCurbeCmd.m_dw7.m_sizeSkipFrames = params->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * params->dwNumSkipFrames;
    }

    cmd.m_frameBrcUpdateCurbeCmd.m_dw0.m_targetSize       = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);
    cmd.m_frameBrcUpdateCurbeCmd.m_dw1.m_frameNumber      = m_storeData - 1;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw2.m_sizeofPicHeaders = m_headerBytesInserted << 3;  // kernel uses how many bits instead of bytes
    cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_currFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_brcFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? brcUpdateIsField : ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (brcUpdateIsField | brcUpdateIsBottomField) : 0);
    cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_brcFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ? brcUpdateIsReference : 0;

    if (bMultiRefQpEnabled)
    {
        cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_brcFlag |= brcUpdateIsActualQp;
        cmd.m_frameBrcUpdateCurbeCmd.m_dw14.m_qpIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;
    auto slcParams = m_avcSliceParams;

    cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_brcFlag |= seqParams->bAutoMaxPBFrameSizeForSceneChange ? brcUpdateAutoPbFrameSize : 0;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw5.m_maxNumPAKs           = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();
    cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_minimumQP            = params->ucMinQP;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_maximumQP            = params->ucMaxQP;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_enableForceToSkip    = (bForceToSkipEnable && !m_avcPicParam->bDisableFrameSkip);
    cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_enableSlidingWindow  = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_enableExtremLowDelay = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);
    cmd.m_frameBrcUpdateCurbeCmd.m_dw6.m_disableVarCompute    = bBRCVarCompuBypass;

    *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_frameBrcUpdateCurbeCmd.m_dw3.m_startGAdjFrame0       = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        cmd.m_frameBrcUpdateCurbeCmd.m_dw3.m_startGAdjFrame1       = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        cmd.m_frameBrcUpdateCurbeCmd.m_dw4.m_startGAdjFrame2       = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        cmd.m_frameBrcUpdateCurbeCmd.m_dw4.m_startGAdjFrame3       = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        cmd.m_frameBrcUpdateCurbeCmd.m_dw11.m_gRateRatioThreshold0 = (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 40)));
        cmd.m_frameBrcUpdateCurbeCmd.m_dw11.m_gRateRatioThreshold1 = (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 75)));
        cmd.m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30) * (100 - 97)));
        cmd.m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (103 - 100)));
        cmd.m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (125 - 100)));
        cmd.m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30) * (160 - 100)));
    }

    cmd.m_frameBrcUpdateCurbeCmd.m_dw15.m_enableROI = params->ucEnableROI;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams   = seqParams;
    sliceState.pEncodeAvcPicParams   = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

    cmd.m_frameBrcUpdateCurbeCmd.m_dw15.m_roundingIntra = 5;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw15.m_roundingInter = sliceState.dwRoundingValue;

    uint32_t profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    cmd.m_frameBrcUpdateCurbeCmd.m_dw19.m_userMaxFrame = profileLevelMaxFrame;

    cmd.m_frameBrcUpdateCurbeCmd.m_dw24.m_surfaceIndexBRCHistorybuffer                  = frameBrcUpdateHistory;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw25.m_surfaceIndexPreciousPAKStatisticsOutputbuffer = frameBrcUpdatePakStatisticsOutput;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw26.m_surfaceIndexAVCIMGStateInputbuffer            = frameBrcUpdateImageStateRead;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw27.m_surfaceIndexAVCIMGStateOutputbuffer           = frameBrcUpdateImageStateWrite;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw28.m_surfaceIndexAVC_Encbuffer                     = frameBrcUpdateMbencCurbeWrite;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw29.m_surfaceIndexAVCDistortionbuffer               = frameBrcUpdateDistortion;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw30.m_surfaceIndexBRCConstdatabuffer                = frameBrcUpdateConstantData;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw31.m_surfaceIndexMBStatsbuffer                     = frameBrcUpdateMbStat;
    cmd.m_frameBrcUpdateCurbeCmd.m_dw32.m_surfaceIndexMotionVectorbuffer                = frameBrcUpdateMvStat;
    auto pStateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd.m_frameBrcUpdateCurbeCmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd.m_frameBrcUpdateCurbeCmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            &cmd.m_frameBrcUpdateCurbeCmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CodechalEncodeAvcEncG12::MbBrcUpdateCurbe curbe;

    // BRC curbe requires: 2 for I-frame, 0 for P-frame, 1 for B-frame
    curbe.m_mbBrcUpdateCurbeCmd.DW0.m_currFrameType = (m_pictureCodingType + 1) % 3;
    if (params->ucEnableROI)
    {
        if (bROIValueInDeltaQP)
        {
            curbe.m_mbBrcUpdateCurbeCmd.DW0.m_enableROI = 2;  // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled
            curbe.m_mbBrcUpdateCurbeCmd.DW0.m_roiRatio  = 0;
        }
        else
        {
            curbe.m_mbBrcUpdateCurbeCmd.DW0.m_enableROI = 1;  // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled

            uint32_t roiSize  = 0;
            uint32_t roiRatio = 0;

            for (uint32_t i = 0; i < m_avcPicParam->NumROI; ++i)
            {
                CODECHAL_ENCODE_VERBOSEMESSAGE("ROI[%d] = {%d, %d, %d, %d} {%d}, size = %d",
                    i,
                    m_avcPicParam->ROI[i].Left,
                    m_avcPicParam->ROI[i].Top,
                    m_avcPicParam->ROI[i].Bottom,
                    m_avcPicParam->ROI[i].Right,
                    m_avcPicParam->ROI[i].PriorityLevelOrDQp,
                    (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top -
                                                          m_avcPicParam->ROI[i].Bottom)) *
                        (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left)));
                roiSize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                           (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left));
            }

            if (roiSize)
            {
                uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
                roiRatio        = 2 * (numMBs * 256 / roiSize - 1);
                roiRatio        = MOS_MIN(51, roiRatio);  // clip QP from 0-51
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("m_roiRatio = %d", roiRatio);
            curbe.m_mbBrcUpdateCurbeCmd.DW0.m_roiRatio = roiRatio;
        }
    }
    else
    {
        curbe.m_mbBrcUpdateCurbeCmd.DW0.m_roiRatio = 0;
    }

    curbe.m_mbBrcUpdateCurbeCmd.DW8.m_historyBufferIndex        = mbBrcUpdateHistory;
    curbe.m_mbBrcUpdateCurbeCmd.DW9.m_mbqpBufferIndex           = mbBrcUpdateMbQp;
    curbe.m_mbBrcUpdateCurbeCmd.DW10.m_roiBufferIndex           = mbBrcUpdateRoi;
    curbe.m_mbBrcUpdateCurbeCmd.DW11.m_mbStatisticalBufferIndex = mbBrcUpdateMbStat;
    auto pStateHeapInterface = m_hwInterface->GetRenderInterface()->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(pStateHeapInterface);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::UserFeatureKeyReport()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::UserFeatureKeyReport());

#if (_DEBUG || _RELEASE_INTERNAL)

    // VE2.0 Reporting
    CodecHalEncode_WriteKey(__MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VE_CTXSCHEDULING_ID, MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface));

#endif // _DEBUG || _RELEASE_INTERNAL
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
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
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    uint32_t refMbCodeBottomFieldOffset =
        params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * 64;
    uint32_t refMvBottomFieldOffset =
        MOS_ALIGN_CEIL(params->dwFrameFieldHeightInMb * params->dwFrameWidthInMb * (32 * 4), 0x1000);

    uint8_t vdirection, refVDirection;
    if (params->bMbEncIFrameDistInUse)
    {
        vdirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        vdirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME : (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer
    uint32_t size                = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    auto     kernelState         = params->pKernelState;
    auto     mbEncBindingTable   = params->pMbEncBindingTable;
    auto     currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto     mbCodeBuffer        = &currPicRefListEntry->resRefMbCodeBuffer;
    auto     mvDataBuffer        = &currPicRefListEntry->resRefMvDataBuffer;

    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = mbCodeBuffer;
    surfaceCodecParams.dwSize                = size;
    surfaceCodecParams.dwOffset              = params->dwMbCodeBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.bRenderTarget         = true;
    surfaceCodecParams.bIsWritable           = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = mvDataBuffer;
    surfaceCodecParams.dwSize                = size;
    surfaceCodecParams.dwOffset              = params->dwMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncIndMVData;
    surfaceCodecParams.bRenderTarget         = true;
    surfaceCodecParams.bIsWritable           = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface               = true;
    surfaceCodecParams.bMediaBlockRW              = true;  // Use media block RW for DP 2D surface access
    surfaceCodecParams.bUseUVPlane                = true;
    surfaceCodecParams.psSurface                  = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset                   = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl      = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset       = mbEncBindingTable->dwAvcMBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset     = mbEncBindingTable->dwAvcMBEncCurrUV;
    surfaceCodecParams.dwVerticalLineStride       = params->dwVerticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
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
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset              = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = params->ps4xMeDistortionBuffer;
        surfaceCodecParams.dwOffset              = params->dwMeDistortionBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncMEDist;
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
        surfaceCodecParams.presBuffer            = params->presMbBrcConstDataBuffer;
        surfaceCodecParams.dwSize                = size;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MB_BRC_CONST_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncMbBrcConstData;
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
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = params->psMbQpBuffer;
        surfaceCodecParams.dwOffset              = params->dwMbQpBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = currFieldPicture ? mbEncBindingTable->dwAvcMBEncMbQpField : mbEncBindingTable->dwAvcMBEncMbQpFrame;
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
        surfaceCodecParams.dwBindingTableOffset = mbEncBindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y - VME
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState          = true;
    surfaceCodecParams.psSurface             = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset              = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = currFieldPicture ? mbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : mbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    surfaceCodecParams.ucVDirection          = vdirection;
#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? mbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : mbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    surfaceCodecParams.ucVDirection         = vdirection;
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
        auto     refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        uint32_t refBindingTableOffset;
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx      = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture)  // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection       = CODECHAL_VDIRECTION_BOT_FIELD;
                    refBindingTableOffset = mbEncBindingTable->dwAvcMBEncFwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection       = CODECHAL_VDIRECTION_TOP_FIELD;
                    refBindingTableOffset = mbEncBindingTable->dwAvcMBEncFwdPicTopField[refIdx];
                }
            }
            else  // if current picture is frame
            {
                refVDirection       = CODECHAL_VDIRECTION_FRAME;
                refBindingTableOffset = mbEncBindingTable->dwAvcMBEncFwdPicFrame[refIdx];
            }

            // Picture Y VME
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if ((0 == refIdx) && (params->bUseWeightedSurfaceForL0))
            {
                surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L0_START + refIdx].sBuffer;
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwWidthInUse  = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

            surfaceCodecParams.dwBindingTableOffset  = refBindingTableOffset;
            surfaceCodecParams.ucVDirection          = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

#ifdef _MMC_SUPPORTED
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

#if 1
    for (refIdx = (params->pAvcSlcParams->num_ref_idx_l0_active_minus1 + 1); refIdx < CODECHAL_ENCODE_NUM_MAX_VME_L0_REF; refIdx++)
    {
        auto     refPic = params->pAvcSlcParams->RefPicList[LIST_0][0];
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        uint32_t refBindingTableOffset;
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture)  // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    refBindingTableOffset = mbEncBindingTable->dwAvcMBEncFwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refBindingTableOffset = mbEncBindingTable->dwAvcMBEncFwdPicTopField[refIdx];
                }
            }
            else  // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refBindingTableOffset = mbEncBindingTable->dwAvcMBEncFwdPicFrame[refIdx];
            }

            // Picture Y VME
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if ((0 == refIdx) && (params->bUseWeightedSurfaceForL0))
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

#ifdef _MMC_SUPPORTED
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    if (params->pAvcSlcParams->num_ref_idx_l1_active_minus1 == 0)
    {
        for (refIdx = 0; refIdx <= MOS_MIN(params->pAvcSlcParams->num_ref_idx_l0_active_minus1, 1); refIdx++)
        {
            auto     refPic = params->pAvcSlcParams->RefPicList[LIST_0][refIdx];
            uint32_t refMbCodeBottomFieldOffsetUsed;
            uint32_t refMvBottomFieldOffsetUsed;
            uint32_t refBindingTableOffset;
            if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
            {
                uint8_t refPicIdx = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
                bool    refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? true : false;
                // Program the surface based on current picture's field/frame mode
                if (currFieldPicture)  // if current picture is field
                {
                    if (refBottomField)
                    {
                        refVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                        refBindingTableOffset = mbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx];
                    }
                    else
                    {
                        refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                        refBindingTableOffset = mbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx];
                    }
                }
                else  // if current picture is frame
                {
                    refVDirection = CODECHAL_VDIRECTION_FRAME;
                    refBindingTableOffset = mbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx];
                }

                // Picture Y VME
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.bUseAdvState = true;
                if ((0 == refIdx) && (params->bUseWeightedSurfaceForL0))
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

#ifdef _MMC_SUPPORTED
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
        }
    }
#endif

    // Setup references 1...n
    // LIST 1 references
    uint32_t curbeSize;
    for (refIdx = 0; refIdx <= params->pAvcSlcParams->num_ref_idx_l1_active_minus1; refIdx++)
    {
        if (!currFieldPicture && refIdx > 0)
        {
            // Only 1 LIST 1 reference required here since only single ref is supported in frame case
            break;
        }

        auto     refPic = params->pAvcSlcParams->RefPicList[LIST_1][refIdx];
        uint32_t refMbCodeBottomFieldOffsetUsed;
        uint32_t refMvBottomFieldOffsetUsed;
        uint32_t refBindingTableOffset;
        bool     refBottomField;
        uint8_t  refPicIdx;
        if (!CodecHal_PictureIsInvalid(refPic) && params->pAvcPicIdx[refPic.FrameIdx].bValid)
        {
            refPicIdx      = params->pAvcPicIdx[refPic.FrameIdx].ucPicIdx;
            refBottomField = (CodecHal_PictureIsBottomField(refPic)) ? 1 : 0;
            // Program the surface based on current picture's field/frame mode
            if (currFieldPicture)  // if current picture is field
            {
                if (refBottomField)
                {
                    refVDirection                = CODECHAL_VDIRECTION_BOT_FIELD;
                    refMbCodeBottomFieldOffsetUsed = refMbCodeBottomFieldOffset;
                    refMvBottomFieldOffsetUsed     = refMvBottomFieldOffset;
                    refBindingTableOffset          = mbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection                = CODECHAL_VDIRECTION_TOP_FIELD;
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed     = 0;
                    refBindingTableOffset          = mbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx];
                }
            }
            else  // if current picture is frame
            {
                refVDirection                = CODECHAL_VDIRECTION_FRAME;
                refMbCodeBottomFieldOffsetUsed = 0;
                refMvBottomFieldOffsetUsed     = 0;
                refBindingTableOffset          = mbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx];
            }

            // Picture Y VME
            memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if ((0 == refIdx) && (params->bUseWeightedSurfaceForL1))
            {
                surfaceCodecParams.psSurface = &params->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L1_START + refIdx].sBuffer;
            }
            else
            {
                surfaceCodecParams.psSurface = &params->ppRefList[refPicIdx]->sRefBuffer;
            }
            surfaceCodecParams.dwWidthInUse  = params->dwFrameWidthInMb * 16;
            surfaceCodecParams.dwHeightInUse = params->dwFrameHeightInMb * 16;

            surfaceCodecParams.dwBindingTableOffset  = refBindingTableOffset;
            surfaceCodecParams.ucVDirection          = refVDirection;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

#ifdef _MMC_SUPPORTED
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));

            if (refIdx == 0)
            {
                // MB data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize                = size;
                surfaceCodecParams.presBuffer            = &params->ppRefList[refPicIdx]->resRefMbCodeBuffer;
                surfaceCodecParams.dwOffset              = refMbCodeBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                // MV data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize                = size;
                surfaceCodecParams.presBuffer            = &params->ppRefList[refPicIdx]->resRefMvDataBuffer;
                surfaceCodecParams.dwOffset              = refMvBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncBwdRefMVData;
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
                        refBindingTableOffset = mbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        refBindingTableOffset = mbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    refBindingTableOffset = mbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.bUseAdvState          = true;
                surfaceCodecParams.dwWidthInUse          = params->dwFrameWidthInMb * 16;
                surfaceCodecParams.dwHeightInUse         = params->dwFrameHeightInMb * 16;
                surfaceCodecParams.psSurface             = &params->ppRefList[refPicIdx]->sRefBuffer;
                surfaceCodecParams.dwBindingTableOffset  = refBindingTableOffset;
                surfaceCodecParams.ucVDirection          = refVDirection;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

#ifdef _MMC_SUPPORTED
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
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
        surfaceCodecParams.bIs2DSurface         = true;
        surfaceCodecParams.bMediaBlockRW        = true;
        surfaceCodecParams.psSurface            = params->psMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset             = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = mbEncBindingTable->dwAvcMBEncBRCDist;
        surfaceCodecParams.bIsWritable          = true;
        surfaceCodecParams.bRenderTarget        = true;
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
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = &currPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        surfaceCodecParams.psSurface->dwHeight   = MOS_ALIGN_CEIL(surfaceCodecParams.psSurface->dwHeight, 8);
        surfaceCodecParams.dwOffset              = params->dwRefPicSelectBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncRefPicSelectL0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.bRenderTarget         = true;
        surfaceCodecParams.bIsWritable           = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMBVProcStatsEnabled)
    {
        size = params->dwFrameWidthInMb *
            (currFieldPicture ? params->dwFrameFieldHeightInMb : params->dwFrameHeightInMb) *
            16 * sizeof(uint32_t);

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize               = size;
        surfaceCodecParams.presBuffer           = params->presMBVProcStatsBuffer;
        surfaceCodecParams.dwOffset             = currBottomField ? params->dwMBVProcStatsBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = mbEncBindingTable->dwAvcMBEncMBStats;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else if (params->bFlatnessCheckEnabled)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = params->psFlatnessCheckSurface;
        surfaceCodecParams.dwOffset              = currBottomField ? params->dwFlatnessCheckBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncFlatnessChk;
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
        surfaceCodecParams.bRawSurface           = true;
        surfaceCodecParams.dwSize                = size;
        surfaceCodecParams.presBuffer            = params->presMADDataBuffer;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncMADData;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MAD_ENCODE].Value;
        surfaceCodecParams.bRenderTarget         = true;
        surfaceCodecParams.bIsWritable           = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->dwMbEncBRCBufferSize > 0)
    {
        // MbEnc BRC buffer - write only
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer            = params->presMbEncBRCBuffer;
        surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMbEncBRCCurbeData;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else
    {
        if (params->bUseMbEncAdvKernel)
        {
            // For BRC the new BRC surface is used
            if (params->bUseAdvancedDsh)
            {
                memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
                curbeSize                     = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMbEncBRCCurbeData;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
            else  // For CQP the DSH CURBE is used
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
                    m_hwInterface->GetRenderInterface()->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMbEncBRCCurbeData;
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
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = params->psSliceMapSurface;
        surfaceCodecParams.bRenderTarget         = false;
        surfaceCodecParams.bIsWritable           = false;
        surfaceCodecParams.dwOffset              = currBottomField ? params->dwSliceMapBottomFieldOffset : 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_SLICE_MAP_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncSliceMapData;

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
            surfaceCodecParams.bIs2DSurface          = true;
            surfaceCodecParams.bMediaBlockRW         = true;
            surfaceCodecParams.psSurface             = params->psMbDisableSkipMapSurface;
            surfaceCodecParams.dwOffset              = 0;
            surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncMbNonSkipMap;
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
            surfaceCodecParams.presBuffer            = params->presSFDCostTableBuffer;
            surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(m_sfdCostTableBufferSize);
            surfaceCodecParams.dwOffset              = 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset  = mbEncBindingTable->dwAvcMBEncStaticDetectionCostTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = m_swScoreboardState->GetCurSwScoreboardSurface();
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_SOFTWARE_SCOREBOARD_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = mbencSwScoreboard;
        surfaceCodecParams.bUse32UINTSurfaceFormat = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SendAvcWPSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psInputRefBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psOutputScaledBuffer);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    memset((void *)&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface               = true;
    surfaceParams.bMediaBlockRW              = true;
    surfaceParams.psSurface                  = params->psInputRefBuffer;  // Input surface
    surfaceParams.bIsWritable                = false;
    surfaceParams.bRenderTarget              = false;
    surfaceParams.dwBindingTableOffset       = wpInputRefSurface;
    surfaceParams.dwCacheabilityControl      = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE].Value;
    surfaceParams.dwVerticalLineStride       = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection               = params->ucVDirection;
#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    memset((void *)&surfaceParams, 0, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface               = true;
    surfaceParams.bMediaBlockRW              = true;
    surfaceParams.psSurface                  = params->psOutputScaledBuffer;  // output surface
    surfaceParams.bIsWritable                = true;
    surfaceParams.bRenderTarget              = true;
    surfaceParams.dwBindingTableOffset       = wpOutputScaledSurface;
    surfaceParams.dwCacheabilityControl      = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE].Value;
    surfaceParams.dwVerticalLineStride       = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection               = params->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SendAvcBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    auto                          kernelState           = params->pKernelState;
    auto                          brcUpdateBindingTable = params->pBrcUpdateBindingTable;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcHistoryBuffer;
    surfaceCodecParams.bIsWritable           = true;
    surfaceCodecParams.bRenderTarget         = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK Statistics buffer
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = &params->pBrcBuffers->resBrcPakStatisticBuffer[0];
    surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(params->dwBrcPakStatisticsSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_PAK_STATS_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t size = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses());
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = &params->pBrcBuffers->resBrcImageStatesReadBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwSize                = size;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcImageStateReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - write only
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = &params->pBrcBuffers->resBrcImageStatesWriteBuffer;
    surfaceCodecParams.dwSize                = size;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcImageStateWriteBuffer;
    surfaceCodecParams.bIsWritable           = true;
    surfaceCodecParams.bRenderTarget         = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->dwMbEncBRCBufferSize > 0)
    {
        // MbEnc BRC buffer - write only
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer            = &params->pBrcBuffers->resMbEncBrcBuffer;
        surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_BRC_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bIsWritable           = true;
        surfaceCodecParams.bRenderTarget         = true;
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
        surfaceCodecParams.dwSize               = MOS_BYTES_TO_DWORDS(size);
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
        surfaceCodecParams.dwSize               = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = brcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bRenderTarget        = true;
        surfaceCodecParams.bIsWritable          = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // AVC_ME BRC Distortion data buffer - input/output
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface          = true;
    surfaceCodecParams.bMediaBlockRW         = true;
    surfaceCodecParams.psSurface             = &params->pBrcBuffers->sMeBrcDistortionBuffer;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE].Value;
    surfaceCodecParams.dwOffset              = params->pBrcBuffers->dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcDistortionBuffer;
    surfaceCodecParams.bRenderTarget         = true;
    surfaceCodecParams.bIsWritable           = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Constant Data Surface
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface          = true;
    surfaceCodecParams.bMediaBlockRW         = true;
    surfaceCodecParams.psSurface             = &params->pBrcBuffers->sBrcConstantDataBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_CONSTANT_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MBStat buffer - input
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = params->presMbStatBuffer;
    surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MB_STATS_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    if (params->psMvDataBuffer)
    {
        memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.psSurface             = params->psMvDataBuffer;
        surfaceCodecParams.dwOffset              = params->dwMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwFrameBrcMvDataBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SendAvcBrcMbUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    auto                          kernelState           = params->pKernelState;
    auto                          brcUpdateBindingTable = params->pBrcUpdateBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.bIsWritable           = true;
    surfaceCodecParams.bRenderTarget         = true;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwMbBrcHistoryBuffer;
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
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.bIsWritable           = true;
        surfaceCodecParams.bRenderTarget         = true;
        surfaceCodecParams.psSurface             = &params->pBrcBuffers->sBrcMbQpBuffer;
        surfaceCodecParams.dwOffset              = params->pBrcBuffers->dwBrcMbQpBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwMbBrcMbQpBuffer;
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
        surfaceCodecParams.bIs2DSurface          = true;
        surfaceCodecParams.bMediaBlockRW         = true;
        surfaceCodecParams.bIsWritable           = false;
        surfaceCodecParams.bRenderTarget         = true;
        surfaceCodecParams.psSurface             = params->psRoiSurface;
        surfaceCodecParams.dwOffset              = 0;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_BRC_ROI_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwMbBrcROISurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // MBStat buffer
    memset((void *)&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer            = params->presMbStatBuffer;
    surfaceCodecParams.dwSize                = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MB_STATS_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset  = brcUpdateBindingTable->dwMbBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SetGpuCtxCreatOption()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(m_osInterface))
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::SetGpuCtxCreatOption());
    }
    else
    {
        m_gpuCtxCreatOpt = MOS_New(MOS_GPUCTX_CREATOPTIONS_ENHANCED);
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_gpuCtxCreatOpt);
         
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalEncodeSinglePipeVE_ConstructParmsForGpuCtxCreation(
            m_sinlgePipeVeState,
            (PMOS_GPUCTX_CREATOPTIONS_ENHANCED)m_gpuCtxCreatOpt));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS readOnly;
    memset(&readOnly, 0, sizeof(readOnly));
    readOnly.ReadOnly = 1;
    uint32_t * dataPtr = (uint32_t *)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource, &readOnly);
    if (!dataPtr)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        return eStatus;
    }

    uint32_t bufferWidthInByte  = BrcBuffers.sBrcRoiSurface.dwPitch;
    uint32_t bufferHeightInByte = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);
    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t uMB = 0; uMB <= numMBs; uMB++)
    {
        int32_t curMbY = uMB / m_picWidthInMb;
        int32_t curMbX = uMB - curMbY * m_picWidthInMb;

        uint32_t outdata = 0;
        for (int32_t roiIdx = (m_avcPicParam->NumROI - 1); roiIdx >= 0; roiIdx--)
        {
            int32_t qpLevel;
            if (bROIValueInDeltaQP)
            {
                qpLevel = -m_avcPicParam->ROI[roiIdx].PriorityLevelOrDQp;
            }
            else
            {
                // QP Level sent to ROI surface is (priority * 6)
                qpLevel = m_avcPicParam->ROI[roiIdx].PriorityLevelOrDQp * 6;
            }

            if (qpLevel == 0)
            {
                continue;
            }

            if ((curMbX >= (int32_t)m_avcPicParam->ROI[roiIdx].Left) && (curMbX < (int32_t)m_avcPicParam->ROI[roiIdx].Right) &&
                (curMbY >= (int32_t)m_avcPicParam->ROI[roiIdx].Top) && (curMbY < (int32_t)m_avcPicParam->ROI[roiIdx].Bottom))
            {
                outdata = 15 | ((qpLevel & 0xFF) << 8);
            }
            else if (bROISmoothEnabled)
            {
                if ((curMbX >= (int32_t)m_avcPicParam->ROI[roiIdx].Left - 1) && (curMbX < (int32_t)m_avcPicParam->ROI[roiIdx].Right + 1) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roiIdx].Top - 1) && (curMbY < (int32_t)m_avcPicParam->ROI[roiIdx].Bottom + 1))
                {
                    outdata = 14 | ((qpLevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[roiIdx].Left - 2) && (curMbX < (int32_t)m_avcPicParam->ROI[roiIdx].Right + 2) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roiIdx].Top - 2) && (curMbY < (int32_t)m_avcPicParam->ROI[roiIdx].Bottom + 2))
                {
                    outdata = 13 | ((qpLevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[roiIdx].Left - 3) && (curMbX < (int32_t)m_avcPicParam->ROI[roiIdx].Right + 3) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[roiIdx].Top - 3) && (curMbY < (int32_t)m_avcPicParam->ROI[roiIdx].Bottom + 3))
                {
                    outdata = 12 | ((qpLevel & 0xFF) << 8);
                }
            }
        }
        dataPtr[(curMbY * (bufferWidthInByte >> 2)) + curMbX] = outdata;
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

MOS_STATUS CodechalEncodeAvcEncG12::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    if (MOS_VE_SUPPORTED(m_osInterface) && cmdBuffer->Attributes.pAttriVe)
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
                (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);
        attriExt->bUseVirtualEngineHint = true;
        attriExt->VEngineHintParams.NeedSyncWithPrevious = 1;
    }

    return CodechalEncodeAvcEnc::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

void CodechalEncodeAvcEncG12::ResizeOnResChange()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::ResizeOnResChange();

    // need to re-allocate surfaces according to resolution
    m_swScoreboardState->ReleaseResources();
}

MOS_STATUS CodechalEncodeAvcEncG12::InitKernelStateMe()
{
    m_hmeKernel = MOS_New(CodechalKernelHmeG12, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hmeKernel);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->Initialize(
        GetCommonKernelHeaderAndSizeG12,
        m_kernelBase,
        m_kuidCommon));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG12::UpdateCmdBufAttribute(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool                renderEngineInUse)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    // should not be there. Will remove it in the next change
    CODECHAL_ENCODE_FUNCTION_ENTER;
    if (MOS_VE_SUPPORTED(m_osInterface))
    {
        PMOS_CMD_BUF_ATTRI_VE attriExt =
            (PMOS_CMD_BUF_ATTRI_VE)(cmdBuffer->Attributes.pAttriVe);

        memset((void *)attriExt, 0, sizeof(MOS_CMD_BUF_ATTRI_VE));
        attriExt->bUseVirtualEngineHint =
            attriExt->VEngineHintParams.NeedSyncWithPrevious = !renderEngineInUse;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::AddMediaVfeCmd(
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

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeAvcEncG12::KernelDebugDumps()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_DEBUG_TOOL(
        if (m_hmeEnabled) {
            CODECHAL_ME_OUTPUT_PARAMS meOutputParams;

            memset((void *)&meOutputParams, 0, sizeof(meOutputParams));
            meOutputParams.psMeMvBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer);
            meOutputParams.psMeDistortionBuffer =
                m_4xMeDistortionBufferSupported ?
                m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer) : nullptr;
            meOutputParams.b16xMeInUse = false;
            meOutputParams.b32xMeInUse = false;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &meOutputParams.psMeMvBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "MvData",
                meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));

            if (meOutputParams.psMeDistortionBuffer)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &meOutputParams.psMeDistortionBuffer->OsResource,
                    CodechalDbgAttr::attrOutput,
                    "MeDist",
                    meOutputParams.psMeDistortionBuffer->dwHeight *meOutputParams.psMeDistortionBuffer->dwPitch,
                    m_hmeKernel ? m_hmeKernel->GetDistortionBottomFieldOffset() : (uint32_t)m_meDistortionBottomFieldOffset,
                    CODECHAL_MEDIA_STATE_4X_ME));
            }

            if (m_16xMeEnabled)
            {
                meOutputParams.psMeMvBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me16xMvDataBuffer);
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
                    meOutputParams.psMeMvBuffer = m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me32xMvDataBuffer);
                    CODECHAL_ENCODE_CHK_STATUS_RETURN(
                        m_debugInterface->DumpBuffer(
                            &meOutputParams.psMeMvBuffer->OsResource,
                            CodechalDbgAttr::attrOutput,
                            "MvData",
                            meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
                            CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb16x * 32), 64) * (m_downscaledFrameFieldHeightInMb16x * 4) : 0,
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
        if (m_mbencBrcBufferSize > 0)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &BrcBuffers.resMbEncBrcBuffer,
                CodechalDbgAttr::attrOutput,
                "MbEncBRCWrite",
                m_mbencBrcBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
        }

        if (m_mbStatsSupported) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resMbStatsBuffer,
                CodechalDbgAttr::attrOutput,
                "MBStatsSurf",
                m_picWidthInMb * m_frameFieldHeightInMb * 16 * sizeof(uint32_t),
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? m_mbStatsBottomFieldOffset : 0,
                CODECHAL_MEDIA_STATE_4X_SCALING));
        }

        else if (m_flatnessCheckEnabled) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_flatnessCheckSurface.OsResource,
                CodechalDbgAttr::attrOutput,
                "FlatnessChkSurf",
                ((CodecHal_PictureIsField(m_currOriginalPic)) ? m_flatnessCheckSurface.dwHeight / 2 : m_flatnessCheckSurface.dwHeight) * m_flatnessCheckSurface.dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? (m_flatnessCheckSurface.dwPitch * m_flatnessCheckSurface.dwHeight >> 1) : 0,
                CODECHAL_MEDIA_STATE_4X_SCALING));
        }
        if (bMbQpDataEnabled) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &sMbQpDataSurface.OsResource,
                CodechalDbgAttr::attrInput,
                "MbQp",
                sMbQpDataSurface.dwHeight*sMbQpDataSurface.dwPitch,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        if (bMbSpecificDataEnabled) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &resMbSpecificDataBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrInput,
                "MbSpecificData",
                m_picWidthInMb*m_frameFieldHeightInMb * 16,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        uint8_t       index;
        CODEC_PICTURE refPic;
        if (bUseWeightedSurfaceForL0) {
            refPic = m_avcSliceParams->RefPicList[LIST_0][0];
            index = m_picIdx[refPic.FrameIdx].ucPicIdx;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "WP_In_L0"));
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &WeightedPredOutputPicSelectList[LIST_0].sBuffer,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "WP_Out_L0"));

        } if (bUseWeightedSurfaceForL1) {

            refPic = m_avcSliceParams->RefPicList[LIST_1][0];
            index = m_picIdx[refPic.FrameIdx].ucPicIdx;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &m_refList[index]->sRefBuffer,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "WP_In_L1"));

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                &WeightedPredOutputPicSelectList[LIST_1].sBuffer,
                CodechalDbgAttr::attrEncodeRawInputSurface,
                "WP_Out_L1"));
        }

        if (m_feiEnable) {
            if (m_avcFeiPicParams->bMBQp) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_avcFeiPicParams->resMBQp,
                    CodechalDbgAttr::attrInput,
                    "MbQp",
                    m_picWidthInMb * m_frameFieldHeightInMb + 3,
                    0,
                    CODECHAL_MEDIA_STATE_ENC_QUALITY));
            }
            if (m_avcFeiPicParams->MVPredictorEnable) {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                    &m_avcFeiPicParams->resMVPredictor,
                    CodechalDbgAttr::attrInput,
                    "MvPredictor",
                    m_picWidthInMb * m_frameFieldHeightInMb * 40,
                    0,
                    CODECHAL_MEDIA_STATE_ENC_QUALITY));
            }
        }
        if (m_arbitraryNumMbsInSlice) {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_sliceMapSurface[m_currRecycledBufIdx].OsResource,
                CodechalDbgAttr::attrInput,
                "SliceMapSurf",
                m_sliceMapSurface[m_currRecycledBufIdx].dwPitch * m_frameFieldHeightInMb,
                0,
                CODECHAL_MEDIA_STATE_ENC_QUALITY));
        }

        // Dump SW scoreboard surface
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &(m_swScoreboardState->GetCurSwScoreboardSurface())->OsResource,
            CodechalDbgAttr::attrOutput,
            "Out",
            (m_swScoreboardState->GetCurSwScoreboardSurface())->dwHeight * (m_swScoreboardState->GetCurSwScoreboardSurface())->dwPitch,
            0,
            CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT));)

        return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG12::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CodechalEncodeAvcEncG12::BrcInitResetCurbe *curbe = (CodechalEncodeAvcEncG12::BrcInitResetCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MBBRCEnable          = bMbBrcEnabled;
        m_avcPar->MBRC                 = bMbBrcEnabled;
        m_avcPar->BitRate              = curbe->m_brcInitResetCurbeCmd.m_dw3.m_averageBitRate;
        m_avcPar->InitVbvFullnessInBit = curbe->m_brcInitResetCurbeCmd.m_dw1.m_initBufFullInBits;
        m_avcPar->MaxBitRate           = curbe->m_brcInitResetCurbeCmd.m_dw4.m_maxBitRate;
        m_avcPar->VbvSzInBit           = curbe->m_brcInitResetCurbeCmd.m_dw2.m_bufSizeInBits;
        m_avcPar->AvbrAccuracy         = curbe->m_brcInitResetCurbeCmd.m_dw10.m_avbrAccuracy;
        m_avcPar->AvbrConvergence      = curbe->m_brcInitResetCurbeCmd.m_dw11.m_avbrConvergence;
        m_avcPar->SlidingWindowSize    = curbe->m_brcInitResetCurbeCmd.m_dw22.m_slidingWindowSize;
        m_avcPar->LongTermInterval     = curbe->m_brcInitResetCurbeCmd.m_dw24.m_longTermInterval;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG12::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CodechalEncodeAvcEncG12::FrameBrcUpdateCurbe *curbe = (CodechalEncodeAvcEncG12::FrameBrcUpdateCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->EnableMultipass     = (curbe->m_frameBrcUpdateCurbeCmd.m_dw5.m_maxNumPAKs > 0) ? 1 : 0;
        m_avcPar->MaxNumPakPasses     = curbe->m_frameBrcUpdateCurbeCmd.m_dw5.m_maxNumPAKs;
        m_avcPar->SlidingWindowEnable = curbe->m_frameBrcUpdateCurbeCmd.m_dw6.m_enableSlidingWindow;
        m_avcPar->FrameSkipEnable     = curbe->m_frameBrcUpdateCurbeCmd.m_dw6.m_enableForceToSkip;
        m_avcPar->UserMaxFrame        = curbe->m_frameBrcUpdateCurbeCmd.m_dw19.m_userMaxFrame;
    }
    else
    {
        m_avcPar->UserMaxFrameP = curbe->m_frameBrcUpdateCurbeCmd.m_dw19.m_userMaxFrame;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG12::PopulateEncParam(
    uint8_t meMethod,
    void    *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    MbencCurbe *curbe = (MbencCurbe *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MRDisableQPCheck = MRDisableQPCheck[m_targetUsage];
        m_avcPar->AllFractional =
            CODECHAL_ENCODE_AVC_AllFractional_Common[m_targetUsage & 0x7];
        m_avcPar->DisableAllFractionalCheckForHighRes =
            CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[m_targetUsage & 0x7];
        m_avcPar->EnableAdaptiveSearch              = curbe->m_curbe.DW37.m_adaptiveEn;
        m_avcPar->EnableFBRBypass                   = curbe->m_curbe.DW4.m_enableFBRBypass;
        m_avcPar->BlockBasedSkip                    = curbe->m_curbe.DW3.m_blockBasedSkipEnable;
        m_avcPar->MADEnableFlag                     = curbe->m_curbe.DW34.m_madEnableFlag;
        m_avcPar->MBTextureThreshold                = curbe->m_curbe.DW60.m_mbTextureThreshold;
        m_avcPar->EnableMBFlatnessCheckOptimization = curbe->m_curbe.DW34.m_enableMBFlatnessChkOptimization;
        m_avcPar->EnableArbitrarySliceSize          = curbe->m_curbe.DW34.m_arbitraryNumMbsPerSlice;
        m_avcPar->RefThresh                         = curbe->m_curbe.DW38.m_refThreshold;
        m_avcPar->EnableWavefrontOptimization       = curbe->m_curbe.DW4.m_enableWavefrontOptimization;
        m_avcPar->MaxLenSP                          = curbe->m_curbe.DW2.m_lenSP;
        m_avcPar->DisableExtendedMvCostRange        = !curbe->m_curbe.DW1.m_extendedMvCostRange;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->MEMethod                             = meMethod;
        m_avcPar->HMECombineLen                        = HMECombineLen[m_targetUsage];
        m_avcPar->FTQBasedSkip                         = FTQBasedSkip[m_targetUsage];
        m_avcPar->MultiplePred                         = MultiPred[m_targetUsage];
        m_avcPar->EnableAdaptiveIntraScaling           = bAdaptiveIntraScalingEnable;
        m_avcPar->StaticFrameIntraCostScalingRatioP    = 240;
        m_avcPar->SubPelMode                           = curbe->m_curbe.DW3.m_subPelMode;
        m_avcPar->HMECombineOverlap                    = curbe->m_curbe.DW36.m_hmeCombineOverlap;
        m_avcPar->SearchX                              = curbe->m_curbe.DW5.m_refWidth;
        m_avcPar->SearchY                              = curbe->m_curbe.DW5.m_refHeight;
        m_avcPar->SearchControl                        = curbe->m_curbe.DW3.m_searchCtrl;
        m_avcPar->EnableAdaptiveTxDecision             = curbe->m_curbe.DW34.m_enableAdaptiveTxDecision;
        m_avcPar->TxDecisionThr                        = curbe->m_curbe.DW60.m_txDecisonThreshold;
        m_avcPar->EnablePerMBStaticCheck               = curbe->m_curbe.DW34.m_enablePerMBStaticCheck;
        m_avcPar->EnableAdaptiveSearchWindowSize       = curbe->m_curbe.DW34.m_enableAdaptiveSearchWindowSize;
        m_avcPar->EnableIntraCostScalingForStaticFrame = curbe->m_curbe.DW4.m_enableIntraCostScalingForStaticFrame;
        m_avcPar->BiMixDisable                         = curbe->m_curbe.DW0.m_biMixDis;
        m_avcPar->SurvivedSkipCost                     = (curbe->m_curbe.DW7.m_nonSkipZMvAdded << 1) + curbe->m_curbe.DW7.m_nonSkipModeAdded;
        m_avcPar->UniMixDisable                        = curbe->m_curbe.DW1.m_uniMixDisable;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        m_avcPar->BMEMethod                         = meMethod;
        m_avcPar->HMEBCombineLen                    = HMEBCombineLen[m_targetUsage];
        m_avcPar->StaticFrameIntraCostScalingRatioB = 200;
        m_avcPar->BSearchX                          = curbe->m_curbe.DW5.m_refWidth;
        m_avcPar->BSearchY                          = curbe->m_curbe.DW5.m_refHeight;
        m_avcPar->BSearchControl                    = curbe->m_curbe.DW3.m_searchCtrl;
        m_avcPar->BSkipType                         = curbe->m_curbe.DW3.m_skipType;
        m_avcPar->DirectMode                        = curbe->m_curbe.DW34.m_bDirectMode;
        m_avcPar->BiWeight                          = curbe->m_curbe.DW1.m_biWeight;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG12::PopulatePakParam(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_BATCH_BUFFER   secondLevelBatchBuffer)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint8_t         *data = nullptr;
    MOS_LOCK_PARAMS lockFlags;
    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.ReadOnly = 1;

    if (cmdBuffer != nullptr)
    {
        data = (uint8_t*)(cmdBuffer->pCmdPtr - (mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD::byteSize / sizeof(uint32_t)));
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

    mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD mfxCmd;
    mfxCmd = *(mhw_vdbox_mfx_g12_X::MFX_AVC_IMG_STATE_CMD *)(data);

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
