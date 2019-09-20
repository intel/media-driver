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
//! \file     codechal_encode_avc_g9.cpp
//! \brief    This file implements the C++ class/interface for Gen9 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!

#include "codechal_encode_avc_g9.h"
#include "igcodeckrn_g9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g9.h"
#endif

#define CODECHAL_ENCODE_AVC_NUM_MBENC_CURBE_SIZE_G9                         88
#define CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE_G9                       128

#define CODECHAL_VDENC_AVC_I_SLICE_SIZE_MINUS_G9                            500
#define CODECHAL_VDENC_AVC_P_SLICE_SIZE_MINUS_G9                            500

#define CODECHAL_ENCODE_AVC_SEI_BUFFER_SIZE                                 10240   // 10K is just estimation
#define CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_OFFSET_SCENE_CHANGED         0x2F8   // (368 + 12)*2 = 760

typedef enum _CODECHAL_BINDING_TABLE_OFFSET_2xSCALING_CM_G9
{
    CODECHAL_2xSCALING_FRAME_SRC_Y_CM_G9 = 0,
    CODECHAL_2xSCALING_FRAME_DST_Y_CM_G9 = 1,
    CODECHAL_2xSCALING_FIELD_TOP_SRC_Y_CM_G9 = 0,
    CODECHAL_2xSCALING_FIELD_TOP_DST_Y_CM_G9 = 1,
    CODECHAL_2xSCALING_FIELD_BOT_SRC_Y_CM_G9 = 2,
    CODECHAL_2xSCALING_FIELD_BOT_DST_Y_CM_G9 = 3,
    CODECHAL_2xSCALING_NUM_SURFACES_CM_G9 = 4
}CODECHAL_BINDING_TABLE_OFFSET_2xSCALING_CM_G9;

typedef enum _CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_BRC_UPDATE_G9
{
    CODECHAL_ENCODE_AVC_BRC_UPDATE_HISTORY_G9 = 0,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_PAK_STATISTICS_OUTPUT_G9 = 1,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_IMAGE_STATE_READ_G9 = 2,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_IMAGE_STATE_WRITE_G9 = 3,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_MBENC_CURBE_READ_G9 = 4,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_MBENC_CURBE_WRITE_G9 = 5,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_DISTORTION_G9 = 6,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_CONSTANT_DATA_G9 = 7,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_MB_QP_G9 = 8,
    CODECHAL_ENCODE_AVC_BRC_UPDATE_NUM_SURFACES_G9 = 9
} CODECHAL_ENCODE_AVC_BINDING_TABLE_OFFSET_BRC_UPDATE_G9;

const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD CodechalEncodeAvcEncG9::IPCM_Threshold_Table[5] =
{
    { 2, 3000 },
    { 4, 3600 },
    { 6, 5000 },
    { 10, 7500 },
    { 18, 9000 },
};

typedef struct _CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_CM_G9
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

    // uint64_t PADDING
    struct
    {
        uint32_t  Reserved;
    } PADDING;
} CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_CM_G9, *PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_CM_G9;

static const CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9 g_cInit_CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9 =
{
    // uint32_t 0
    {
        {
            0
        }
    },

        // uint32_t 1
    {
        {
            0
        }
    },

        // uint32_t 2
    {
        {
            0
        }
    },

        // uint32_t 3
    {
        {
            0
        }
    },

        // uint32_t 4
    {
        {
            0
        }
    },

        // uint32_t 5
    {
        {
            0
        }
    },

        // uint32_t 6
    {
        {
            0
        }
    },

        // uint32_t 7
    {
        {
            0
        }
    },

        // uint32_t 8
    {
        {
            0,
            0
        }
    },

        // uint32_t 9
    {
        {
            0,
            0
        }
    },

        // uint32_t 10
    {
        {
            0,
            0
        }
    },

        // uint32_t 11
    {
        {
            0,
            1
        }
    },

        // uint32_t 12
    {
        {
            51,
            0
        }
    },

        // uint32_t 13
    {
        {
            40,
            60,
            80,
            120
        }
    },

        // uint32_t 14
    {
        {
            35,
            60,
            80,
            120
        }
    },

        // uint32_t 15
    {
        {
            40,
            60,
            90,
            115
        }
    },

        // uint32_t 16
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 17
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 18
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 19
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 20
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 21
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 22
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 23
    {
        {
            0
        }
    }
};

typedef struct _CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9
{
    union
    {
        struct
        {
            uint32_t   TargetSize : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    union
    {
        struct
        {
            uint32_t   FrameNumber : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    union
    {
        struct
        {
            uint32_t   SizeofPicHeaders : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

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
    } DW3;

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
    } DW4;

    union
    {
        struct
        {
            uint32_t   TargetSizeFlag : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   BRCFlag : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaxNumPAKs : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   CurrFrameType : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    union
    {
        struct
        {
            uint32_t   NumSkipFrames : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   MinimumQP : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   MaximumQP : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   EnableForceToSkip : MOS_BITFIELD_BIT(24);
            uint32_t   EnableSlidingWindow : MOS_BITFIELD_BIT(25);
            uint32_t   Reserved : MOS_BITFIELD_RANGE(26, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    union
    {
        struct
        {
            uint32_t    SizeSkipFrames : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

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
    } DW8;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustMult4 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDiv0 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   StartGlobalAdjustDiv1 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   StartGlobalAdjustDiv2 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW9;

    union
    {
        struct
        {
            uint32_t   StartGlobalAdjustDiv3 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   StartGlobalAdjustDiv4 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   QPThreshold0 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   QPThreshold1 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW10;

    union
    {
        struct
        {
            uint32_t   QPThreshold2 : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   QPThreshold3 : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   gRateRatioThreshold0 : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   gRateRatioThreshold1 : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW11;

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
    } DW12;

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
    } DW13;

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
    } DW14;

    union
    {
        struct
        {
            uint32_t   QPIntraRefresh         : MOS_BITFIELD_RANGE(0, 7);
            uint32_t   IntraRefreshMode       : MOS_BITFIELD_RANGE(8, 15);
            uint32_t   Reserved1              : MOS_BITFIELD_RANGE(16, 23);
            uint32_t   Reserved2              : MOS_BITFIELD_RANGE(24, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW15;

    union
    {
        struct
        {
            uint32_t   IntraRefreshYPos      : MOS_BITFIELD_RANGE(0, 15);
            uint32_t   IntraRefreshXPos      : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW16;

    union
    {
        struct
        {
            uint32_t    IntraRefreshHeight   : MOS_BITFIELD_RANGE(0, 15);
            uint32_t    IntraRefreshWidth    : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW17;

    union
    {
        struct
        {
            uint32_t    IntraRefreshOffFrames : MOS_BITFIELD_RANGE(0, 15);
            uint32_t    Reserved              : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW18;

    union
    {
        struct
        {
            uint32_t   UserMaxFrame : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW19;

    union
    {
        struct
        {
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW20;

    union
    {
        struct
        {
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW21;

    union
    {
        struct
        {
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW22;

    union
    {
        struct
        {
            uint32_t   Reserved : MOS_BITFIELD_RANGE(0, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW23;
} CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9, *PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9;
C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9)) == 24);

static const CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9 g_cInit_CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9 =
{
    // uint32_t 0
    {
        {
            0
        }
    },

        // uint32_t 1
    {
        {
            0
        }
    },

        // uint32_t 2
    {
        {
            0
        }
    },

        // uint32_t 3
    {
        {
            10,
            50
        }
    },

        // uint32_t 4
    {
        {
            100,
            150
        }
    },

        // uint32_t 5
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 6
    {
        {
            0,
            0,
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 7
    {
        {
            0
        }
    },

        // uint32_t 8
    {
        {
            1,
            1,
            3,
            2
        }
    },

        // uint32_t 9
    {
        {
            1,
            40,
            5,
            5
        }
    },

        // uint32_t 10
    {
        {
            3,
            1,
            7,
            18
        }
    },

        // uint32_t 11
    {
        {
            25,
            37,
            40,
            75
        }
    },

        // uint32_t 12
    {
        {
            97,
            103,
            125,
            160
        }
    },

        // uint32_t 13
    {
        {
            MOS_BITFIELD_VALUE((uint32_t)-3, 8),
            MOS_BITFIELD_VALUE((uint32_t)-2, 8),
            MOS_BITFIELD_VALUE((uint32_t)-1, 8),
            0
        }
    },

        // uint32_t 14
    {
        {
            1,
            2,
            3,
            0xff
        }
    },

        // uint32_t 15
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 16
    {
        {
            0,
            0
        }
    },

        // uint32_t 17
    {
        {
            0,
            0
        }
    },

        // uint32_t 18
    {
        {
            0,
            0
        }
    },

        // uint32_t 19
    {
        {
            0
        }
    },

        // uint32_t 20
    {
        {
            0
        }
    },

        // uint32_t 21
    {
        {
            0
        }
    },

        // uint32_t 22
    {
        {
            0
        }
    },

        // uint32_t 23
    {
        {
            0
        }
    },
};

const uint32_t CodechalEncodeAvcEncG9::IntraModeCostForHighTextureMB[CODEC_AVC_NUM_QP]
{
    0x00000303, 0x00000304, 0x00000404, 0x00000405, 0x00000505, 0x00000506, 0x00000607, 0x00000708,
    0x00000809, 0x0000090a, 0x00000a0b, 0x00000b0c, 0x00000c0e, 0x00000e18, 0x00001819, 0x00001918,
    0x00001a19, 0x00001b19, 0x00001d19, 0x00001e18, 0x00002818, 0x00002918, 0x00002a18, 0x00002b19,
    0x00002d18, 0x00002e18, 0x00003818, 0x00003918, 0x00003a18, 0x00003b0f, 0x00003d0e, 0x00003e0e,
    0x0000480e, 0x0000490e, 0x00004a0e, 0x00004b0d, 0x00004d0d, 0x00004e0d, 0x0000580e, 0x0000590e,
    0x00005a0e, 0x00005b0d, 0x00005d0c, 0x00005e0b, 0x0000680a, 0x00006908, 0x00006a09, 0x00006b0a,
    0x00006d0b, 0x00006e0d, 0x0000780e, 0x00007918
};

static const CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9 g_cInit_CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9 =
{
    // uint32_t 0
    {
        {
            0
        }
    },

        // uint32_t 1
    {
        {
            0
        }
    },

        // uint32_t 2
    {
        {
            0
        }
    },

        // uint32_t 3
    {
        {
            10,
            50
        }
    },

        // uint32_t 4
    {
        {
            100,
            150
        }
    },

        // uint32_t 5
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 6
    {
        {
            0,
            0,
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 7
    {
        {
            0
        }
    },

        // uint32_t 8
    {
        {
            1,
            1,
            3,
            2
        }
    },

        // uint32_t 9
    {
        {
            1,
            40,
            5,
            5
        }
    },

        // uint32_t 10
    {
        {
            3,
            1,
            7,
            18
        }
    },

        // uint32_t 11
    {
        {
            25,
            37,
            40,
            75
        }
    },

        // uint32_t 12
    {
        {
            97,
            103,
            125,
            160
        }
    },

        // uint32_t 13
    {
        {
            MOS_BITFIELD_VALUE((uint32_t)-3, 8),
            MOS_BITFIELD_VALUE((uint32_t)-2, 8),
            MOS_BITFIELD_VALUE((uint32_t)-1, 8),
            0
        }
    },

        // uint32_t 14
    {
        {
            1,
            2,
            3,
            0xff
        }
    },

        // uint32_t 15
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 16
    {
        {
            0
        }
    },

        // uint32_t 17
    {
        {
            0
        }
    },

        // uint32_t 18
    {
        {
            0
        }
    },

        // uint32_t 19
    {
        {
            0
        }
    },

        // uint32_t 20
    {
        {
            0
        }
    },

        // uint32_t 21
    {
        {
            0
        }
    },

        // uint32_t 22
    {
        {
            0
        }
    },

        // uint32_t 23
    {
        {
            0
        }
    },
};

static const CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9 g_cInit_CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9 =
{
    // uint32_t 0
    {
        {
            0,
            0,
            0,
            0
        }
    },

        // uint32_t 1
    {
        {
            0
        }
    },

        // uint32_t 2
    {
        {
            0
        }
    },

        // uint32_t 3
    {
        {
            0
        }
    },

        // uint32_t 4
    {
        {
            0
        }
    },

        // uint32_t 5
    {
        {
            0
        }
    },

        // uint32_t 6
    {
        {
            0
        }
    }
};

CodechalEncodeAvcEncG9::CodechalEncodeAvcEncG9(
    CodechalHwInterface *   hwInterface,
    CodechalDebugInterface *debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) : CodechalEncodeAvcEnc(hwInterface, debugInterface, standardInfo)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_needCheckCpEnabled = true;

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(m_encodeParState = MOS_New(CodechalDebugEncodeParG9, this));
        CreateAvcPar();
    )
}

CodechalEncodeAvcEncG9::~CodechalEncodeAvcEncG9()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(
        DestroyAvcPar();
        MOS_Delete(m_encodeParState);
    )
}

MOS_STATUS CodechalEncodeAvcEncG9::InitializeState()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitializeState());

    m_brcHistoryBufferSize = CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_SIZE_G9;
    dwBrcConstantSurfaceWidth = CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_WIDTH_G9;
    dwBrcConstantSurfaceHeight = CODECHAL_ENCODE_AVC_BRC_CONSTANTSURFACE_HEIGHT_G9;

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params)

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
        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;

        uint32_t* pData = (uint32_t*)params->pOsInterface->pfnLockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer,
            &lockFlags);
        if (pData == nullptr)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        // Update MbBrcConstantDataBuffer with high texture cost
        for (uint8_t qp = 0; qp < CODEC_AVC_NUM_QP; qp++)
        {
            // Writing to DW13 in each sub-array of 16 DWs
            *(pData + 13) = (uint32_t)IntraModeCostForHighTextureMB[qp];
            // 16 DWs per QP value
            pData += 16;
        }

        params->pOsInterface->pfnUnlockResource(
            params->pOsInterface,
            params->presBrcConstantDataBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::InitKernelStateWP()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    pWPKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);

    auto kernelStatePtr = pWPKernelState;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHalGetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER currKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        kernelBinary,
        ENC_WP,
        0,
        &currKrnHeader,
        &kernelSize));
    kernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_WP_NUM_SURFACES_G9;
    kernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    kernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9);
    kernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    kernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    kernelStatePtr->KernelParams.iIdCount = 1;
    kernelStatePtr->KernelParams.iInlineDataLength = 0;

    kernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    kernelStatePtr->KernelParams.pBinary = kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    kernelStatePtr->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        kernelStatePtr->KernelParams.iBTCount,
        &kernelStatePtr->dwSshSize,
        &kernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::GetMbEncKernelStateIdx(CodechalEncodeIdOffsetParams* params, uint32_t* kernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(kernelOffset);

    *kernelOffset = MBENC_I_OFFSET_CM;

    if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *kernelOffset +=
            MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsages;
    }
    else
    {
        if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_NORMAL)
        {
            *kernelOffset += MBENC_TARGET_USAGE_CM;
        }
        else if (params->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_PERFORMANCE)
        {
            *kernelOffset += MBENC_TARGET_USAGE_CM * 2;
        }
    }

    if (params->wPictureCodingType == P_TYPE)
    {
        *kernelOffset += MBENC_P_OFFSET_CM;
    }
    else if (params->wPictureCodingType == B_TYPE)
    {
        *kernelOffset += MBENC_B_OFFSET_CM;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    auto slcParams = m_avcSliceParams;
    auto seqParams = m_avcSeqParam;
    auto kernelState = pWPKernelState;
    CODECHAL_ENCODE_ASSERT(seqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    CODECHAL_ENCODE_AVC_WP_CURBE_G9 cmd;
    MOS_ZeroMemory(&cmd, sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9));

    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    cmd.DW0.DefaultWeight = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][0];
    cmd.DW0.DefaultOffset = slcParams->Weights[params->RefPicListIdx][params->WPIdx][0][1];

    cmd.DW49.Log2WeightDenom = slcParams->luma_log2_weight_denom;
    cmd.DW49.ROI_enabled = 0;

    cmd.DW50.InputSurface = CODECHAL_ENCODE_AVC_WP_INPUT_REF_SURFACE_G9;
    cmd.DW51.OutputSurface = CODECHAL_ENCODE_AVC_WP_OUTPUT_SCALED_SURFACE_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params)
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

    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9 cmd = g_cInit_CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9;
    cmd.DW0.ProfileLevelMaxFrame = profileLevelMaxFrame;
    cmd.DW1.InitBufFullInBits = seqParams->InitVBVBufferFullnessInBit;
    cmd.DW2.BufSizeInBits = seqParams->VBVBufferSizeInBit;
    cmd.DW3.AverageBitRate = seqParams->TargetBitRate;
    cmd.DW4.MaxBitRate = seqParams->MaxBitRate;
    cmd.DW8.GopP =
        (seqParams->GopRefDist) ? ((seqParams->GopPicSize - 1) / seqParams->GopRefDist) : 0;
    cmd.DW9.GopB = seqParams->GopPicSize - 1 - cmd.DW8.GopP;
    cmd.DW9.FrameWidthInBytes = m_frameWidth;
    cmd.DW10.FrameHeightInBytes = m_frameHeight;
    cmd.DW12.NoSlices = m_numSlices;

    // if VUI present, VUI data has high priority
    if (seqParams->vui_parameters_present_flag && seqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        cmd.DW4.MaxBitRate =
            ((vuiParams->bit_rate_value_minus1[0] + 1) << (6 + vuiParams->bit_rate_scale));

        if (seqParams->RateControlMethod == RATECONTROL_CBR)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate;
        }
    }

    cmd.DW6.FrameRateM = seqParams->FramesPer100Sec;
    cmd.DW7.FrameRateD = 100;
    cmd.DW8.BRCFlag = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    cmd.DW8.BRCFlag |= (bMbBrcEnabled && !bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (seqParams->RateControlMethod == RATECONTROL_CBR)
    {
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        cmd.DW4.MaxBitRate = cmd.DW3.AverageBitRate;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        cmd.DW23.ACQP = seqParams->ICQQualityFactor;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_VCM)
    {
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (seqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (cmd.DW4.MaxBitRate < cmd.DW3.AverageBitRate)
        {
            cmd.DW3.AverageBitRate = cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        cmd.DW8.BRCFlag = cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        cmd.DW23.ACQP = seqParams->ICQQualityFactor;
    }

    cmd.DW10.AVBRAccuracy = usAVBRAccuracy;
    cmd.DW11.AVBRConvergence = usAVBRConvergence;

    // Set dynamic thresholds
    double dInputBitsPerFrame =
        ((double)(cmd.DW4.MaxBitRate) * (double)(cmd.DW7.FrameRateD) /
        (double)(cmd.DW6.FrameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        dInputBitsPerFrame *= 0.5;
    }

    if (cmd.DW2.BufSizeInBits == 0)
    {
        cmd.DW2.BufSizeInBits = (uint32_t)dInputBitsPerFrame * 4;
    }

    if (cmd.DW1.InitBufFullInBits == 0)
    {
        cmd.DW1.InitBufFullInBits = 7 * cmd.DW2.BufSizeInBits / 8;
    }
    if (cmd.DW1.InitBufFullInBits < (uint32_t)(dInputBitsPerFrame * 2))
    {
        cmd.DW1.InitBufFullInBits = (uint32_t)(dInputBitsPerFrame * 2);
    }
    if (cmd.DW1.InitBufFullInBits > cmd.DW2.BufSizeInBits)
    {
        cmd.DW1.InitBufFullInBits = cmd.DW2.BufSizeInBits;
    }

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        cmd.DW2.BufSizeInBits = 2 * seqParams->TargetBitRate;
        cmd.DW1.InitBufFullInBits = (uint32_t)(0.75 * cmd.DW2.BufSizeInBits);
    }

    double dBpsRatio = dInputBitsPerFrame / ((double)(cmd.DW2.BufSizeInBits) / 30);
    dBpsRatio = (dBpsRatio < 0.1) ? 0.1 : (dBpsRatio > 3.5) ? 3.5 : dBpsRatio;

    cmd.DW16.DeviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, dBpsRatio));
    cmd.DW16.DeviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, dBpsRatio));
    cmd.DW16.DeviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, dBpsRatio));
    cmd.DW16.DeviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, dBpsRatio));
    cmd.DW17.DeviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, dBpsRatio));
    cmd.DW17.DeviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, dBpsRatio));
    cmd.DW17.DeviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, dBpsRatio));
    cmd.DW17.DeviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, dBpsRatio));
    cmd.DW18.DeviationThreshold0ForVBR = (uint32_t)(-50 * pow(0.9, dBpsRatio));
    cmd.DW18.DeviationThreshold1ForVBR = (uint32_t)(-50 * pow(0.7, dBpsRatio));
    cmd.DW18.DeviationThreshold2ForVBR = (uint32_t)(-50 * pow(0.5, dBpsRatio));
    cmd.DW18.DeviationThreshold3ForVBR = (uint32_t)(-50 * pow(0.3, dBpsRatio));
    cmd.DW19.DeviationThreshold4ForVBR = (uint32_t)(100 * pow(0.4, dBpsRatio));
    cmd.DW19.DeviationThreshold5ForVBR = (uint32_t)(100 * pow(0.5, dBpsRatio));
    cmd.DW19.DeviationThreshold6ForVBR = (uint32_t)(100 * pow(0.75, dBpsRatio));
    cmd.DW19.DeviationThreshold7ForVBR = (uint32_t)(100 * pow(0.9, dBpsRatio));
    cmd.DW20.DeviationThreshold0ForI = (uint32_t)(-50 * pow(0.8, dBpsRatio));
    cmd.DW20.DeviationThreshold1ForI = (uint32_t)(-50 * pow(0.6, dBpsRatio));
    cmd.DW20.DeviationThreshold2ForI = (uint32_t)(-50 * pow(0.34, dBpsRatio));
    cmd.DW20.DeviationThreshold3ForI = (uint32_t)(-50 * pow(0.2, dBpsRatio));
    cmd.DW21.DeviationThreshold4ForI = (uint32_t)(50 * pow(0.2, dBpsRatio));
    cmd.DW21.DeviationThreshold5ForI = (uint32_t)(50 * pow(0.4, dBpsRatio));
    cmd.DW21.DeviationThreshold6ForI = (uint32_t)(50 * pow(0.66, dBpsRatio));
    cmd.DW21.DeviationThreshold7ForI = (uint32_t)(50 * pow(0.9, dBpsRatio));

    cmd.DW22.SlidingWindowSize = dwSlidingWindowSize;

    if (bBrcInit)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits = cmd.DW1.InitBufFullInBits;
    }

    *params->pdwBrcInitResetBufSizeInBits = cmd.DW2.BufSizeInBits;
    *params->pdBrcInitResetInputBitsPerFrame = dInputBitsPerFrame;

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

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto seqParams = m_avcSeqParam;
    auto picParams = m_avcPicParam;
    auto slcParams = m_avcSliceParams;

    MHW_VDBOX_AVC_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.pEncodeAvcSeqParams = seqParams;
    sliceState.pEncodeAvcPicParams = picParams;
    sliceState.pEncodeAvcSliceParams = slcParams;

    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9   cmd = g_cInit_CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9;
    cmd.DW5.TargetSizeFlag = 0;
    if (*params->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *params->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        cmd.DW5.TargetSizeFlag = 1;
    }

    // skipped frame handling
    if (params->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        cmd.DW6.NumSkipFrames = params->dwNumSkipFrames;
        cmd.DW7.SizeSkipFrames = params->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * params->dwNumSkipFrames;
    }

    cmd.DW0.TargetSize = (uint32_t)(*params->pdBrcInitCurrentTargetBufFullInBits);
    cmd.DW1.FrameNumber = m_storeData - 1;
    cmd.DW2.SizeofPicHeaders = m_headerBytesInserted << 3;   // kernel uses how many bits instead of bytes
    cmd.DW5.CurrFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    cmd.DW5.BRCFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? brcUpdateIsField :
        ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (brcUpdateIsField | brcUpdateIsBottomField) : 0);
    cmd.DW5.BRCFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ?
        brcUpdateIsReference : 0;

    if (bMultiRefQpEnabled)
    {
        cmd.DW5.BRCFlag |= brcUpdateIsActualQp;
        cmd.DW14.QPIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    cmd.DW5.BRCFlag |= seqParams->bAutoMaxPBFrameSizeForSceneChange?
        brcUpdateAutoPbFrameSize : 0;

    cmd.DW5.MaxNumPAKs = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    cmd.DW6.MinimumQP               = params->ucMinQP;
    cmd.DW6.MaximumQP               = params->ucMaxQP;
    cmd.DW6.EnableForceToSkip       = (bForceToSkipEnable && !m_avcPicParam->bDisableFrameSkip);
    cmd.DW6.EnableSlidingWindow     = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    cmd.DW6.EnableExtremLowDelay    = (seqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);

    *params->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (seqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        cmd.DW3.startGAdjFrame0 = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        cmd.DW3.startGAdjFrame1 = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        cmd.DW4.startGAdjFrame2 = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        cmd.DW4.startGAdjFrame3 = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        cmd.DW11.gRateRatioThreshold0 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 40)));
        cmd.DW11.gRateRatioThreshold1 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 75)));
        cmd.DW12.gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 97)));
        cmd.DW12.gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(103 - 100)));
        cmd.DW12.gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(125 - 100)));
        cmd.DW12.gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(160 - 100)));
    }

    cmd.DW15.EnableROI = params->ucEnableROI;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&sliceState));

    cmd.DW15.RoundingIntra = 5;
    cmd.DW15.RoundingInter = sliceState.dwRoundingValue;

    uint32_t                                           profileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        seqParams,
        this,
        (uint32_t*)&profileLevelMaxFrame));

    cmd.DW19.UserMaxFrame = profileLevelMaxFrame;

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

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    auto curbe = g_cInit_CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9;

    // BRC curbe requires: 2 for I-frame, 0 for P-frame, 1 for B-frame
    curbe.DW0.CurrFrameType = (m_pictureCodingType + 1) % 3;
    if( params->ucEnableROI )
    {
        if (bROIValueInDeltaQP)
        {
            curbe.DW0.EnableROI = 2; // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled
            curbe.DW0.ROIRatio  = 0;
        }
        else
        {
            curbe.DW0.EnableROI = 1; // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled

            uint32_t roisize = 0;
            uint32_t roiratio = 0;

            for (uint32_t i = 0 ; i < m_avcPicParam->NumROI ; ++i)
            {
                CODECHAL_ENCODE_VERBOSEMESSAGE("ROI[%d] = {%d, %d, %d, %d} {%d}, size = %d", i,
                    m_avcPicParam->ROI[i].Left, m_avcPicParam->ROI[i].Top,
                    m_avcPicParam->ROI[i].Bottom, m_avcPicParam->ROI[i].Right,
                    m_avcPicParam->ROI[i].PriorityLevelOrDQp,
                    (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left)));
                roisize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                                (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left));
            }

            if (roisize)
            {
                uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
                roiratio = 2 * (numMBs * 256 / roisize - 1);
                roiratio = MOS_MIN(51, roiratio); // clip QP from 0-51
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("ROIRatio = %d", roiratio);
            curbe.DW0.ROIRatio  = roiratio;
        }
    }
    else
    {
        curbe.DW0.ROIRatio = 0;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &curbe,
        params->pKernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcBrcBlockCopy(PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS params)
{
    MOS_STATUS                                        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);

    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_CM_G9 cmd;
    MOS_ZeroMemory(&cmd, sizeof(cmd));
    cmd.DW0.BufferOffset = params->dwBufferOffset;
    cmd.DW0.BlockHeight = params->dwBlockHeight;
    cmd.DW1.SrcSurfaceIndex = 0x00;
    cmd.DW2.DstSurfaceIndex = 0x01;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(params->pKernelState->m_dshRegion.AddData(
        &cmd,
        params->pKernelState->dwCurbeOffset,
        sizeof(cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params)
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

    auto kernelState = params->pKernelState;
    auto avcMbEncBindingTable = params->pMbEncBindingTable;
    bool currFieldPicture = CodecHal_PictureIsField(*(params->pCurrOriginalPic)) ? 1 : 0;
    bool currBottomField = CodecHal_PictureIsBottomField(*(params->pCurrOriginalPic)) ? 1 : 0;
    auto currPicRefListEntry = params->ppRefList[params->pCurrReconstructedPic->FrameIdx];
    auto mbCodeBuffer = &currPicRefListEntry->resRefMbCodeBuffer;
    auto mvDataBuffer = &currPicRefListEntry->resRefMvDataBuffer;
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
        vdirection = (CodecHal_PictureIsFrame(*(params->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
            (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer
    uint32_t size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    CODECHAL_SURFACE_CODEC_PARAMS               surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mbCodeBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMbCodeBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
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
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = mvDataBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncIndMVData;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Current Picture Y
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    surfaceCodecParams.bUseUVPlane = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrY;
    surfaceCodecParams.dwUVBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncCurrUV;
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

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMeMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(params->ps4xMeDistortionBuffer);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->ps4xMeDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeDistortionBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMEDist;
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

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbBrcConstDataBuffer;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    if (params->bMbQpBufferInUse)
    {
        // AVC MB BRC QP buffer
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMbQpBuffer;
        surfaceCodecParams.dwOffset = params->dwMbQpBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = currFieldPicture ? avcMbEncBindingTable->dwAvcMBEncMbQpField :
            avcMbEncBindingTable->dwAvcMBEncMbQpFrame;
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
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMbSpecificDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = params->psCurrPicSurface;
    surfaceCodecParams.dwOffset = params->dwCurrPicSurfaceOffset;
    surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    surfaceCodecParams.ucVDirection = vdirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    surfaceCodecParams.dwBindingTableOffset = currFieldPicture ?
        avcMbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : avcMbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    surfaceCodecParams.ucVDirection = vdirection;
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
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFwdPicFrame[refIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if((params->bUseWeightedSurfaceForL0) &&
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
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx];
                }
                else
                {
                    refVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed = 0;
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx];
                }
            }
            else // if current picture is frame
            {
                refVDirection = CODECHAL_VDIRECTION_FRAME;
                refMbCodeBottomFieldOffsetUsed = 0;
                refMvBottomFieldOffsetUsed = 0;
                refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            if((params->bUseWeightedSurfaceForL1) &&
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
                if(currFieldPicture && (params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_FRAME || params->ppRefList[refPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_INVALID))
                {
                    refMbCodeBottomFieldOffsetUsed = 0;
                    refMvBottomFieldOffsetUsed     = 0;
                }
                // MB data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 16 * 4;
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMbCodeBuffer;
                surfaceCodecParams.dwOffset = refMbCodeBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));

                // MV data buffer
                size = params->dwFrameWidthInMb * params->dwFrameFieldHeightInMb * 32 * 4;
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.dwSize = size;
                surfaceCodecParams.presBuffer = &params->ppRefList[refPicIdx]->resRefMvDataBuffer;
                surfaceCodecParams.dwOffset = refMvBottomFieldOffsetUsed;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdRefMVData;
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
                        refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicBotField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicTopField[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    refBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBwdPicFrame[refIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
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
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = params->dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncBRCDist;
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
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &currPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        surfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(surfaceCodecParams.psSurface->dwHeight, 8);
        surfaceCodecParams.dwOffset = params->dwRefPicSelectBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncRefPicSelectL0;
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

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMBVProcStatsBuffer;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwMBVProcStatsBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMBStats;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    else if (params->bFlatnessCheckEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psFlatnessCheckSurface;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwFlatnessCheckBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncFlatnessChk;
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

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bRawSurface = true;
        surfaceCodecParams.dwSize = size;
        surfaceCodecParams.presBuffer = params->presMADDataBuffer;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMADData;
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
        //Started from GEN95, separated Mbenc curbe from BRC update kernel. BRC update kernel will generate a 128 bytes surface for mbenc.
        //The new surface contains the updated data for mbenc. MBenc kernel has been changed to use the new BRC update output surface
        //to update its curbe internally.
        // MbEnc BRC buffer - write only
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = params->presMbEncBRCBuffer;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
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
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
                curbeSize = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
                surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                    m_hwInterface,
                    cmdBuffer,
                    &surfaceCodecParams,
                    kernelState));
            }
            else // For CQP the DSH CURBE is used
            {
                MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                MOS_RESOURCE *dsh = nullptr;
                CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = params->pKernelState->m_dshRegion.GetResource());
                surfaceCodecParams.presBuffer = dsh;
                surfaceCodecParams.dwOffset =
                    params->pKernelState->m_dshRegion.GetOffset() +
                    params->pKernelState->dwCurbeOffset;
                curbeSize = MOS_ALIGN_CEIL(
                    params->pKernelState->KernelParams.iCurbeLength,
                    m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(curbeSize);
                surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
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
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psSliceMapSurface;
        surfaceCodecParams.bRenderTarget = false;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.dwOffset = currBottomField ? params->dwSliceMapBottomFieldOffset : 0;
        surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncSliceMapData;

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
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bIs2DSurface = true;
            surfaceCodecParams.bMediaBlockRW = true;
            surfaceCodecParams.psSurface = params->psMbDisableSkipMapSurface;
            surfaceCodecParams.dwOffset = 0;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncMbNonSkipMap;
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
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.presBuffer = params->presSFDCostTableBuffer;
            surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE_G9);
            surfaceCodecParams.dwOffset = 0;
            surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = avcMbEncBindingTable->dwAvcMBEncStaticDetectionCostTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcWPSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psInputRefBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->psOutputScaledBuffer);

    CODECHAL_SURFACE_CODEC_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psInputRefBuffer;// Input surface
    surfaceParams.bIsWritable = false;
    surfaceParams.bRenderTarget = false;
    surfaceParams.dwBindingTableOffset = CODECHAL_ENCODE_AVC_WP_INPUT_REF_SURFACE_G9;
    surfaceParams.dwVerticalLineStride = params->dwVerticalLineStride;
    surfaceParams.dwVerticalLineStrideOffset = params->dwVerticalLineStrideOffset;
    surfaceParams.ucVDirection = params->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceParams,
        params->pKernelState));

    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.bIs2DSurface = true;
    surfaceParams.bMediaBlockRW = true;
    surfaceParams.psSurface = params->psOutputScaledBuffer;// output surface
    surfaceParams.bIsWritable = true;
    surfaceParams.bRenderTarget = true;
    surfaceParams.dwBindingTableOffset = CODECHAL_ENCODE_AVC_WP_OUTPUT_SCALED_SURFACE_G9;
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

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    auto kernelState = params->pKernelState;
    auto avcBrcUpdateBindingTable = params->pBrcUpdateBindingTable;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcHistoryBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK Statistics buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcPakStatisticBuffer[0];
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcPakStatisticsSize);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t size = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer =
        &params->pBrcBuffers->resBrcImageStatesReadBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcImageStateReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcImageStatesWriteBuffer;
    surfaceCodecParams.dwSize = size;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcImageStateWriteBuffer;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    if (params->dwMbEncBRCBufferSize > 0)
    {
        //Started from GEN95, separated Mbenc curbe from BRC update kernel. BRC update kernel will generate a 128 bytes surface for mbenc.
        //The new surface contains the updated data for mbenc. MBenc kernel has been changed to use the new BRC update output surface
        //to update its curbe internally.
        // MbEnc BRC buffer - write only
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = &params->pBrcBuffers->resMbEncBrcBuffer;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwMbEncBRCBufferSize);

        if (IsMfeMbEncEnabled(false))
        {
            surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
                m_hwInterface,
                cmdBuffer,
                &surfaceCodecParams,
                kernelState));
        }

        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
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

        // BRC ENC CURBE Buffer - read only
        size = MOS_ALIGN_CEIL(
            mbEncKernelState->KernelParams.iCurbeLength,
            m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        MOS_RESOURCE *dsh = nullptr;
        CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = mbEncKernelState->m_dshRegion.GetResource());
        surfaceCodecParams.presBuffer = dsh;
        surfaceCodecParams.dwOffset =
            mbEncKernelState->m_dshRegion.GetOffset() +
            mbEncKernelState->dwCurbeOffset;
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        // BRC ENC CURBE Buffer - write only
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        if (params->bUseAdvancedDsh)
        {
            surfaceCodecParams.presBuffer = params->presMbEncCurbeBuffer;
        }
        else
        {
            MOS_RESOURCE *dsh = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = mbEncKernelState->m_dshRegion.GetResource());
            surfaceCodecParams.presBuffer = dsh;
            surfaceCodecParams.dwOffset =
                mbEncKernelState->m_dshRegion.GetOffset() +
                mbEncKernelState->dwCurbeOffset;
        }
        surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(size);
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // AVC_ME BRC Distortion data buffer - input/output
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = &params->pBrcBuffers->sMeBrcDistortionBuffer;
    surfaceCodecParams.dwOffset = params->pBrcBuffers->dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcDistortionBuffer;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC Constant Data Surface
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface =
        &params->pBrcBuffers->sBrcConstantDataBuffer[params->ucCurrRecycledBufIdx];
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MBStat buffer - input
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presMbStatBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MV data buffer
    if (params->psMvDataBuffer)
    {
        memset(&surfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = params->psMvDataBuffer;
        surfaceCodecParams.dwOffset = params->dwMvBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwFrameBrcMvDataBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcBrcMbUpdateSurfaces(PMOS_COMMAND_BUFFER cmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBrcBuffers);

    // BRC history buffer
    auto kernelState = params->pKernelState;
    auto avcBrcUpdateBindingTable = params->pBrcUpdateBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS                   surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &params->pBrcBuffers->resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(params->dwBrcHistoryBufferSize);
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcHistoryBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC MB QP data buffer
    if (params->bMbBrcEnabled)
    {
        params->pBrcBuffers->sBrcMbQpBuffer.dwHeight = MOS_ALIGN_CEIL((params->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.bIsWritable = true;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.psSurface = &params->pBrcBuffers->sBrcMbQpBuffer;
        surfaceCodecParams.dwOffset = params->pBrcBuffers->dwBrcMbQpBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcMbQpBuffer;
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

        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.bIsWritable = false;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.psSurface = params->psRoiSurface;
        surfaceCodecParams.dwOffset = 0;
        surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcROISurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // MBStat buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = params->presMbStatBuffer;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    surfaceCodecParams.dwBindingTableOffset = avcBrcUpdateBindingTable->dwMbBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS readOnly;
    MOS_ZeroMemory(&readOnly, sizeof(readOnly));
    readOnly.ReadOnly = 1;
    uint32_t* data = (uint32_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource, &readOnly);
    if (!data)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        return eStatus;
    }

    uint32_t bufferWidthInByte = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 4), 64);//(m_picWidthInMb * 4 + 63) & ~63;
    uint32_t bufferHeightInByte = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);//(m_picHeightInMb + 7) & ~7;
    uint32_t numMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t mb = 0; mb <= numMBs; mb++)
    {
        int32_t curMbY = mb / m_picWidthInMb;
        int32_t curMbX = mb - curMbY * m_picWidthInMb;

        uint32_t outdata = 0;
        for (int32_t iRoi = (m_avcPicParam->NumROI - 1); iRoi >= 0; iRoi--)
        {
            int32_t qplevel;
            if (bROIValueInDeltaQP)
            {
                qplevel = -m_avcPicParam->ROI[iRoi].PriorityLevelOrDQp;
            }
            else
            {
                // QP Level sent to ROI surface is (priority * 6)
                qplevel = m_avcPicParam->ROI[iRoi].PriorityLevelOrDQp * 6;
            }

            if (qplevel == 0)
            {
                continue;
            }

            if ((curMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left) && (curMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right) &&
                (curMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top) && (curMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom))
            {
                outdata = 15 | ((qplevel & 0xFF) << 8);
            }
            else if (bROISmoothEnabled)
            {
                if ((curMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left - 1) && (curMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right + 1) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top - 1) && (curMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom + 1))
                {
                    outdata = 14 | ((qplevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left - 2) && (curMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right + 2) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top - 2) && (curMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom + 2))
                {
                    outdata = 13 | ((qplevel & 0xFF) << 8);
                }
                else if ((curMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left - 3) && (curMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right + 3) &&
                    (curMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top - 3) && (curMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom + 3))
                {
                    outdata = 12 | ((qplevel & 0xFF) << 8);
                }
            }
        }
        data[(curMbY * (bufferWidthInByte>>2)) + curMbX] = outdata;
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

bool CodechalEncodeAvcEncG9::IsMfeMbEncEnabled(bool mbEncIFrameDistInUse)
{
    return mbEncIFrameDistInUse ? false : m_mfeMbEncEanbled;
}

MOS_STATUS CodechalEncodeAvcEncG9::GetStatusReport(
    void *status,
    uint16_t numStatus)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(status);
    EncodeStatusReport *codecStatus = (EncodeStatusReport *)status;

    if ((m_mfeEnabled && m_codecFunction == CODECHAL_FUNCTION_FEI_ENC)
#ifdef FEI_ENABLE_CMRT
        || (m_codecFunction == CODECHAL_FUNCTION_FEI_PRE_ENC)
#endif
        )
    {
        if (m_cmEvent[m_cmEventCheckIdx] != nullptr)
        {
            if (!m_mfeEnabled)
            {
                m_cmEvent[m_cmEventCheckIdx]->WaitForTaskFinished();
                m_cmQueue->DestroyEvent(m_cmEvent[m_cmEventCheckIdx]);
            }
            m_cmEvent[m_cmEventCheckIdx] = nullptr;
            m_cmEventCheckIdx            = (m_cmEventCheckIdx + 1) % CM_EVENT_NUM;
            codecStatus[0].CodecStatus = CODECHAL_STATUS_SUCCESSFUL;

            return MOS_STATUS_SUCCESS;
        }
        else
        {
            codecStatus[0].CodecStatus = CODECHAL_STATUS_UNAVAILABLE;
            return MOS_STATUS_SUCCESS;
        }
    }
    else
        return CodechalEncoderState::GetStatusReport(status, numStatus);
}

MOS_STATUS CodechalEncodeAvcEncG9::SceneChangeReport(PMOS_COMMAND_BUFFER    cmdBuffer, PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS params)
{

    MHW_MI_COPY_MEM_MEM_PARAMS                      copyMemMemParams;
    uint32_t offset = (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize)
        + (sizeof(uint32_t) * 2) + m_encodeStatusBuf.dwSceneChangedOffset;

    MOS_ZeroMemory(&copyMemMemParams, sizeof(copyMemMemParams));
    copyMemMemParams.presSrc = params->presBrcHistoryBuffer;
    copyMemMemParams.dwSrcOffset = CODECHAL_ENCODE_AVC_BRC_HISTORY_BUFFER_OFFSET_SCENE_CHANGED;
    copyMemMemParams.presDst = &m_encodeStatusBuf.resStatusBuffer;
    copyMemMemParams.dwDstOffset = offset;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiCopyMemMemCmd(
        cmdBuffer,
        &copyMemMemParams));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeAvcEncG9::PopulateBrcInitParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9 *curbe = (CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9 *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MBBRCEnable          = bMbBrcEnabled;
        m_avcPar->MBRC                 = bMbBrcEnabled;
        m_avcPar->BitRate              = curbe->DW3.AverageBitRate;
        m_avcPar->InitVbvFullnessInBit = curbe->DW1.InitBufFullInBits;
        m_avcPar->MaxBitRate           = curbe->DW4.MaxBitRate;
        m_avcPar->VbvSzInBit           = curbe->DW2.BufSizeInBits;
        m_avcPar->AvbrAccuracy         = curbe->DW10.AVBRAccuracy;
        m_avcPar->AvbrConvergence      = curbe->DW11.AVBRConvergence;
        m_avcPar->SlidingWindowSize    = curbe->DW22.SlidingWindowSize;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9::PopulateBrcUpdateParam(
    void *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9 *curbe = (CODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_G9 *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->EnableMultipass     = (curbe->DW5.MaxNumPAKs > 0) ? 1 : 0;
        m_avcPar->MaxNumPakPasses     = curbe->DW5.MaxNumPAKs;
        m_avcPar->SlidingWindowEnable = curbe->DW6.EnableSlidingWindow;
        m_avcPar->FrameSkipEnable     = curbe->DW6.EnableForceToSkip;
        m_avcPar->UserMaxFrame        = curbe->DW19.UserMaxFrame;
    }
    else
    {
        m_avcPar->UserMaxFrameP = curbe->DW19.UserMaxFrame;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeAvcEncG9::PopulateEncParam(
    uint8_t meMethod,
    void    *cmd)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    CODECHAL_DEBUG_CHK_NULL(m_debugInterface);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrDumpEncodePar))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_ENCODE_AVC_MBENC_CURBE_G9 *curbe = (CODECHAL_ENCODE_AVC_MBENC_CURBE_G9 *)cmd;

    if (m_pictureCodingType == I_TYPE)
    {
        m_avcPar->MRDisableQPCheck = MRDisableQPCheck[m_targetUsage];
        m_avcPar->AllFractional =
            CODECHAL_ENCODE_AVC_AllFractional_Common[m_targetUsage & 0x7];
        m_avcPar->DisableAllFractionalCheckForHighRes =
            CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[m_targetUsage & 0x7];
        m_avcPar->EnableAdaptiveSearch              = curbe->common.DW37.AdaptiveEn;
        m_avcPar->EnableFBRBypass                   = curbe->common.DW4.EnableFBRBypass;
        m_avcPar->BlockBasedSkip                    = curbe->common.DW3.BlockBasedSkipEnable;
        m_avcPar->MADEnableFlag                     = curbe->common.DW34.MADEnableFlag;
        m_avcPar->MBTextureThreshold                = curbe->common.DW58.MBTextureThreshold;
        m_avcPar->EnableMBFlatnessCheckOptimization = curbe->common.DW34.EnableMBFlatnessChkOptimization;
        m_avcPar->EnableArbitrarySliceSize          = curbe->common.DW34.ArbitraryNumMbsPerSlice;
        m_avcPar->RefThresh                         = curbe->common.DW38.RefThreshold;
        m_avcPar->EnableWavefrontOptimization       = curbe->common.DW4.EnableWavefrontOptimization;
        m_avcPar->MaxLenSP                          = curbe->common.DW2.LenSP;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        m_avcPar->MEMethod                             = meMethod;
        m_avcPar->HMECombineLen                        = HMECombineLen[m_targetUsage];
        m_avcPar->FTQBasedSkip                         = FTQBasedSkip[m_targetUsage];
        m_avcPar->MultiplePred                         = MultiPred[m_targetUsage];
        m_avcPar->EnableAdaptiveIntraScaling           = bAdaptiveIntraScalingEnable;
        m_avcPar->StaticFrameIntraCostScalingRatioP    = 240;
        m_avcPar->SubPelMode                           = curbe->common.DW3.SubPelMode;
        m_avcPar->HMECombineOverlap                    = curbe->common.DW36.HMECombineOverlap;
        m_avcPar->SearchX                              = curbe->common.DW5.RefWidth;
        m_avcPar->SearchY                              = curbe->common.DW5.RefHeight;
        m_avcPar->SearchControl                        = curbe->common.DW3.SearchCtrl;
        m_avcPar->EnableAdaptiveTxDecision             = curbe->common.DW34.EnableAdaptiveTxDecision;
        m_avcPar->TxDecisionThr                        = curbe->common.DW58.TxDecisonThreshold;
        m_avcPar->EnablePerMBStaticCheck               = curbe->common.DW34.EnablePerMBStaticCheck;
        m_avcPar->EnableAdaptiveSearchWindowSize       = curbe->common.DW34.EnableAdaptiveSearchWindowSize;
        m_avcPar->EnableIntraCostScalingForStaticFrame = curbe->common.DW4.EnableIntraCostScalingForStaticFrame;
        m_avcPar->BiMixDisable                         = curbe->common.DW0.BiMixDis;
        m_avcPar->SurvivedSkipCost                     = (curbe->common.DW7.NonSkipZMvAdded << 1) + curbe->common.DW7.NonSkipModeAdded;
        m_avcPar->UniMixDisable                        = curbe->common.DW1.UniMixDisable;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        m_avcPar->BMEMethod                         = meMethod;
        m_avcPar->HMEBCombineLen                    = HMEBCombineLen[m_targetUsage];
        m_avcPar->StaticFrameIntraCostScalingRatioB = 200;
        m_avcPar->BSearchX                          = curbe->common.DW5.RefWidth;
        m_avcPar->BSearchY                          = curbe->common.DW5.RefHeight;
        m_avcPar->BSearchControl                    = curbe->common.DW3.SearchCtrl;
        m_avcPar->BSkipType                         = curbe->common.DW3.SkipType;
        m_avcPar->DirectMode                        = curbe->common.DW34.bDirectMode;
        m_avcPar->BiWeight                          = curbe->common.DW1.BiWeight;
    }

    return MOS_STATUS_SUCCESS;
}
#endif
