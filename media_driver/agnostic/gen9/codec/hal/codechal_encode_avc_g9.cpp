/*
* Copyright (c) 2011-2017, Intel Corporation
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
#include "codechal_encoder_g9.h"
#include "igcodeckrn_g9.h"
#if USE_CODECHAL_DEBUG_TOOL
#include "codechal_debug_encode_par_g9.h"
#endif

#define CODECHAL_ENCODE_AVC_NUM_MBENC_CURBE_SIZE_G9                         88
#define CODECHAL_ENCODE_AVC_SFD_OUTPUT_BUFFER_SIZE_G9                       128

#define CODECHAL_VDENC_AVC_I_SLICE_SIZE_MINUS_G9                            500
#define CODECHAL_VDENC_AVC_P_SLICE_SIZE_MINUS_G9                            500

#define CODECHAL_ENCODE_AVC_SEI_BUFFER_SIZE                                 10240   // 10K is just estimation

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
        MOS_Delete(m_encodeParState);
        DestroyAvcPar();
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

MOS_STATUS CodechalEncodeAvcEncG9::InitMbBrcConstantDataBuffer(PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS pParams)

{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pOsInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->presBrcConstantDataBuffer);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncodeAvcEnc::InitMbBrcConstantDataBuffer(pParams));

    if (pParams->wPictureCodingType == I_TYPE)
    {
        MOS_LOCK_PARAMS LockFlags;
        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
        LockFlags.WriteOnly = 1;

        uint32_t* pData = (uint32_t*)pParams->pOsInterface->pfnLockResource(
            pParams->pOsInterface,
            pParams->presBrcConstantDataBuffer,
            &LockFlags);
        if (pData == nullptr)
        {
            eStatus = MOS_STATUS_UNKNOWN;
            return eStatus;
        }

        // Update MbBrcConstantDataBuffer with high texture cost
        for (uint8_t ucQp = 0; ucQp < CODEC_AVC_NUM_QP; ucQp++)
        {
            // Writing to DW13 in each sub-array of 16 DWs
            *(pData + 13) = (uint32_t)IntraModeCostForHighTextureMB[ucQp];
            // 16 DWs per QP value
            pData += 16;
        }

        pParams->pOsInterface->pfnUnlockResource(
            pParams->pOsInterface,
            pParams->presBrcConstantDataBuffer);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::InitKernelStateWP()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    pWPKernelState = MOS_New(MHW_KERNEL_STATE);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pWPKernelState);

    auto pKernelStatePtr = pWPKernelState;

    uint8_t* kernelBinary;
    uint32_t kernelSize;

    MOS_STATUS status = CodecHal_GetKernelBinaryAndSize(m_kernelBase, m_kuid, &kernelBinary, &kernelSize);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(status);

    CODECHAL_KERNEL_HEADER CurrKrnHeader;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
        kernelBinary,
        ENC_WP,
        0,
        &CurrKrnHeader,
        &kernelSize));
    pKernelStatePtr->KernelParams.iBTCount = CODECHAL_ENCODE_AVC_WP_NUM_SURFACES_G9;
    pKernelStatePtr->KernelParams.iThreadCount = m_renderEngineInterface->GetHwCaps()->dwMaxThreads;
    pKernelStatePtr->KernelParams.iCurbeLength = sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9);
    pKernelStatePtr->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
    pKernelStatePtr->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
    pKernelStatePtr->KernelParams.iIdCount = 1;
    pKernelStatePtr->KernelParams.iInlineDataLength = 0;

    pKernelStatePtr->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
    pKernelStatePtr->KernelParams.pBinary = kernelBinary + (CurrKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
    pKernelStatePtr->KernelParams.iSize = kernelSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
        m_stateHeapInterface,
        pKernelStatePtr->KernelParams.iBTCount,
        &pKernelStatePtr->dwSshSize,
        &pKernelStatePtr->dwBindingTableSize));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_MhwInitISH(m_stateHeapInterface, pKernelStatePtr));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::GetMbEncKernelStateIdx(PCODECHAL_ENCODE_ID_OFFSET_PARAMS pParams, uint32_t* pdwKernelOffset)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pdwKernelOffset);

    *pdwKernelOffset = MBENC_I_OFFSET_CM;

    if (pParams->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_ADV)
    {
        *pdwKernelOffset +=
            MBENC_TARGET_USAGE_CM * m_mbencNumTargetUsages;
    }
    else
    {
        if (pParams->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_NORMAL)
        {
            *pdwKernelOffset += MBENC_TARGET_USAGE_CM;
        }
        else if (pParams->EncFunctionType == CODECHAL_MEDIA_STATE_ENC_PERFORMANCE)
        {
            *pdwKernelOffset += MBENC_TARGET_USAGE_CM * 2;
        }
    }

    if (pParams->wPictureCodingType == P_TYPE)
    {
        *pdwKernelOffset += MBENC_P_OFFSET_CM;
    }
    else if (pParams->wPictureCodingType == B_TYPE)
    {
        *pdwKernelOffset += MBENC_B_OFFSET_CM;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcWP(PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);

    auto pSlcParams = m_avcSliceParams;
    auto pSeqParams = m_avcSeqParam;
    auto pKernelState = pWPKernelState;
    CODECHAL_ENCODE_ASSERT(pSeqParams->TargetUsage < NUM_TARGET_USAGE_MODES);

    CODECHAL_ENCODE_AVC_WP_CURBE_G9 Cmd;
    MOS_ZeroMemory(&Cmd, sizeof(CODECHAL_ENCODE_AVC_WP_CURBE_G9));

    /* Weights[i][j][k][m] is interpreted as:

    i refers to reference picture list 0 or 1;
    j refers to reference list entry 0-31;
    k refers to data for the luma (Y) component when it is 0, the Cb chroma component when it is 1 and the Cr chroma component when it is 2;
    m refers to weight when it is 0 and offset when it is 1
    */
    Cmd.DW0.DefaultWeight = pSlcParams->Weights[pParams->RefPicListIdx][pParams->WPIdx][0][0];
    Cmd.DW0.DefaultOffset = pSlcParams->Weights[pParams->RefPicListIdx][pParams->WPIdx][0][1];

    Cmd.DW49.Log2WeightDenom = pSlcParams->luma_log2_weight_denom;
    Cmd.DW49.ROI_enabled = 0;

    Cmd.DW50.InputSurface = CODECHAL_ENCODE_AVC_WP_INPUT_REF_SURFACE_G9;
    Cmd.DW51.OutputSurface = CODECHAL_ENCODE_AVC_WP_OUTPUT_SCALED_SURFACE_G9;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pKernelState->m_dshRegion.AddData(
        &Cmd,
        pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcBrcInitReset(PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);

    auto pPicParams = m_avcPicParam;
    auto pSeqParams = m_avcSeqParam;
    auto pVuiParams = m_avcVuiParams;
    uint32_t dwProfileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        pSeqParams,
        this,
        (uint32_t*)&dwProfileLevelMaxFrame));

    CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9 Cmd = g_cInit_CODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_G9;
    Cmd.DW0.ProfileLevelMaxFrame = dwProfileLevelMaxFrame;
    Cmd.DW1.InitBufFullInBits = pSeqParams->InitVBVBufferFullnessInBit;
    Cmd.DW2.BufSizeInBits = pSeqParams->VBVBufferSizeInBit;
    Cmd.DW3.AverageBitRate = pSeqParams->TargetBitRate;
    Cmd.DW4.MaxBitRate = pSeqParams->MaxBitRate;
    Cmd.DW8.GopP =
        (pSeqParams->GopRefDist) ? ((pSeqParams->GopPicSize - 1) / pSeqParams->GopRefDist) : 0;
    Cmd.DW9.GopB = pSeqParams->GopPicSize - 1 - Cmd.DW8.GopP;
    Cmd.DW9.FrameWidthInBytes = m_frameWidth;
    Cmd.DW10.FrameHeightInBytes = m_frameHeight;
    Cmd.DW12.NoSlices = m_numSlices;

    // if VUI present, VUI data has high priority
    if (pSeqParams->vui_parameters_present_flag && pSeqParams->RateControlMethod != RATECONTROL_AVBR)
    {
        Cmd.DW4.MaxBitRate =
            ((pVuiParams->bit_rate_value_minus1[0] + 1) << (6 + pVuiParams->bit_rate_scale));

        if (pSeqParams->RateControlMethod == RATECONTROL_CBR)
        {
            Cmd.DW3.AverageBitRate = Cmd.DW4.MaxBitRate;
        }
    }

    Cmd.DW6.FrameRateM = pSeqParams->FramesPer100Sec;
    Cmd.DW7.FrameRateD = 100;
    Cmd.DW8.BRCFlag = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? 0 : CODECHAL_ENCODE_BRCINIT_FIELD_PIC;
    // MBBRC should be skipped when BRC ROI is on
    Cmd.DW8.BRCFlag |= (bMbBrcEnabled && !bBrcRoiEnabled) ? 0 : CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC;

    if (pSeqParams->RateControlMethod == RATECONTROL_CBR)
    {
        Cmd.DW4.MaxBitRate = Cmd.DW3.AverageBitRate;
        Cmd.DW8.BRCFlag = Cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (pSeqParams->RateControlMethod == RATECONTROL_VBR)
    {
        if (Cmd.DW4.MaxBitRate < Cmd.DW3.AverageBitRate)
        {
            Cmd.DW3.AverageBitRate = Cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        Cmd.DW8.BRCFlag = Cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (pSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        Cmd.DW8.BRCFlag = Cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, max bitrate = target bitrate,
        Cmd.DW4.MaxBitRate = Cmd.DW3.AverageBitRate;
    }
    else if (pSeqParams->RateControlMethod == RATECONTROL_ICQ)
    {
        Cmd.DW8.BRCFlag = Cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISICQ;
        Cmd.DW23.ACQP = pSeqParams->ICQQualityFactor;
    }
    else if (pSeqParams->RateControlMethod == RATECONTROL_VCM)
    {
        Cmd.DW8.BRCFlag = Cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISVCM;
    }
    else if (pSeqParams->RateControlMethod == RATECONTROL_QVBR)
    {
        if (Cmd.DW4.MaxBitRate < Cmd.DW3.AverageBitRate)
        {
            Cmd.DW3.AverageBitRate = Cmd.DW4.MaxBitRate; // Use max bit rate for HRD compliance
        }
        Cmd.DW8.BRCFlag = Cmd.DW8.BRCFlag | CODECHAL_ENCODE_BRCINIT_ISQVBR;
        // use ICQQualityFactor to determine the larger Qp for each MB
        Cmd.DW23.ACQP = pSeqParams->ICQQualityFactor;
    }

    Cmd.DW10.AVBRAccuracy = usAVBRAccuracy;
    Cmd.DW11.AVBRConvergence = usAVBRConvergence;

    // Set dynamic thresholds
    double dInputBitsPerFrame =
        ((double)(Cmd.DW4.MaxBitRate) * (double)(Cmd.DW7.FrameRateD) /
        (double)(Cmd.DW6.FrameRateM));
    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        dInputBitsPerFrame *= 0.5;
    }

    if (Cmd.DW2.BufSizeInBits == 0)
    {
        Cmd.DW2.BufSizeInBits = (uint32_t)dInputBitsPerFrame * 4;
    }

    if (Cmd.DW1.InitBufFullInBits == 0)
    {
        Cmd.DW1.InitBufFullInBits = 7 * Cmd.DW2.BufSizeInBits / 8;
    }
    if (Cmd.DW1.InitBufFullInBits < (uint32_t)(dInputBitsPerFrame * 2))
    {
        Cmd.DW1.InitBufFullInBits = (uint32_t)(dInputBitsPerFrame * 2);
    }
    if (Cmd.DW1.InitBufFullInBits > Cmd.DW2.BufSizeInBits)
    {
        Cmd.DW1.InitBufFullInBits = Cmd.DW2.BufSizeInBits;
    }

    if (pSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        // For AVBR, Buffer size =  2*Bitrate, InitVBV = 0.75 * BufferSize
        Cmd.DW2.BufSizeInBits = 2 * pSeqParams->TargetBitRate;
        Cmd.DW1.InitBufFullInBits = (uint32_t)(0.75 * Cmd.DW2.BufSizeInBits);
    }

    double dBpsRatio = dInputBitsPerFrame / ((double)(Cmd.DW2.BufSizeInBits) / 30);
    dBpsRatio = (dBpsRatio < 0.1) ? 0.1 : (dBpsRatio > 3.5) ? 3.5 : dBpsRatio;

    Cmd.DW16.DeviationThreshold0ForPandB = (uint32_t)(-50 * pow(0.90, dBpsRatio));
    Cmd.DW16.DeviationThreshold1ForPandB = (uint32_t)(-50 * pow(0.66, dBpsRatio));
    Cmd.DW16.DeviationThreshold2ForPandB = (uint32_t)(-50 * pow(0.46, dBpsRatio));
    Cmd.DW16.DeviationThreshold3ForPandB = (uint32_t)(-50 * pow(0.3, dBpsRatio));
    Cmd.DW17.DeviationThreshold4ForPandB = (uint32_t)(50 * pow(0.3, dBpsRatio));
    Cmd.DW17.DeviationThreshold5ForPandB = (uint32_t)(50 * pow(0.46, dBpsRatio));
    Cmd.DW17.DeviationThreshold6ForPandB = (uint32_t)(50 * pow(0.7, dBpsRatio));
    Cmd.DW17.DeviationThreshold7ForPandB = (uint32_t)(50 * pow(0.9, dBpsRatio));
    Cmd.DW18.DeviationThreshold0ForVBR = (uint32_t)(-50 * pow(0.9, dBpsRatio));
    Cmd.DW18.DeviationThreshold1ForVBR = (uint32_t)(-50 * pow(0.7, dBpsRatio));
    Cmd.DW18.DeviationThreshold2ForVBR = (uint32_t)(-50 * pow(0.5, dBpsRatio));
    Cmd.DW18.DeviationThreshold3ForVBR = (uint32_t)(-50 * pow(0.3, dBpsRatio));
    Cmd.DW19.DeviationThreshold4ForVBR = (uint32_t)(100 * pow(0.4, dBpsRatio));
    Cmd.DW19.DeviationThreshold5ForVBR = (uint32_t)(100 * pow(0.5, dBpsRatio));
    Cmd.DW19.DeviationThreshold6ForVBR = (uint32_t)(100 * pow(0.75, dBpsRatio));
    Cmd.DW19.DeviationThreshold7ForVBR = (uint32_t)(100 * pow(0.9, dBpsRatio));
    Cmd.DW20.DeviationThreshold0ForI = (uint32_t)(-50 * pow(0.8, dBpsRatio));
    Cmd.DW20.DeviationThreshold1ForI = (uint32_t)(-50 * pow(0.6, dBpsRatio));
    Cmd.DW20.DeviationThreshold2ForI = (uint32_t)(-50 * pow(0.34, dBpsRatio));
    Cmd.DW20.DeviationThreshold3ForI = (uint32_t)(-50 * pow(0.2, dBpsRatio));
    Cmd.DW21.DeviationThreshold4ForI = (uint32_t)(50 * pow(0.2, dBpsRatio));
    Cmd.DW21.DeviationThreshold5ForI = (uint32_t)(50 * pow(0.4, dBpsRatio));
    Cmd.DW21.DeviationThreshold6ForI = (uint32_t)(50 * pow(0.66, dBpsRatio));
    Cmd.DW21.DeviationThreshold7ForI = (uint32_t)(50 * pow(0.9, dBpsRatio));

    Cmd.DW22.SlidingWindowSize = dwSlidingWindowSize;

    if (bBrcInit)
    {
        *pParams->pdBrcInitCurrentTargetBufFullInBits = Cmd.DW1.InitBufFullInBits;
    }

    *pParams->pdwBrcInitResetBufSizeInBits = Cmd.DW2.BufSizeInBits;
    *pParams->pdBrcInitResetInputBitsPerFrame = dInputBitsPerFrame;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcInitParam(
            &Cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcFrameBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);

    auto pSeqParams = m_avcSeqParam;
    auto pPicParams = m_avcPicParam;
    auto pSlcParams = m_avcSliceParams;

    MHW_VDBOX_AVC_SLICE_STATE                       SliceState;
    SliceState.pEncodeAvcSeqParams = pSeqParams;
    SliceState.pEncodeAvcPicParams = pPicParams;
    SliceState.pEncodeAvcSliceParams = pSlcParams;

    CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9   Cmd = g_cInit_CODECHAL_ENCODE_AVC_FRAME_BRC_UPDATE_CURBE_G9;
    Cmd.DW5.TargetSizeFlag = 0;
    if (*pParams->pdBrcInitCurrentTargetBufFullInBits > (double)dwBrcInitResetBufSizeInBits)
    {
        *pParams->pdBrcInitCurrentTargetBufFullInBits -= (double)dwBrcInitResetBufSizeInBits;
        Cmd.DW5.TargetSizeFlag = 1;
    }

    // skipped frame handling
    if (pParams->dwNumSkipFrames)
    {
        // pass num/size of skipped frames to update BRC
        Cmd.DW6.NumSkipFrames = pParams->dwNumSkipFrames;
        Cmd.DW7.SizeSkipFrames = pParams->dwSizeSkipFrames;

        // account for skipped frame in calculating CurrentTargetBufFullInBits
        *pParams->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame * pParams->dwNumSkipFrames;
    }

    Cmd.DW0.TargetSize = (uint32_t)(*pParams->pdBrcInitCurrentTargetBufFullInBits);
    Cmd.DW1.FrameNumber = m_storeData - 1;
    Cmd.DW2.SizeofPicHeaders = m_headerBytesInserted << 3;   // kernel uses how many bits instead of bytes
    Cmd.DW5.CurrFrameType =
        ((m_pictureCodingType - 2) < 0) ? 2 : (m_pictureCodingType - 2);
    Cmd.DW5.BRCFlag =
        (CodecHal_PictureIsTopField(m_currOriginalPic)) ? CODECHAL_ENCODE_BRCUPDATE_IS_FIELD :
        ((CodecHal_PictureIsBottomField(m_currOriginalPic)) ? (CODECHAL_ENCODE_BRCUPDATE_IS_FIELD | CODECHAL_ENCODE_BRCUPDATE_IS_BOTTOM_FIELD) : 0);
    Cmd.DW5.BRCFlag |= (m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) ?
    CODECHAL_ENCODE_BRCUPDATE_IS_REFERENCE : 0;

    if (bMultiRefQpEnabled)
    {
        Cmd.DW5.BRCFlag |= CODECHAL_ENCODE_BRCUPDATE_IS_ACTUALQP;
        Cmd.DW14.QPIndexOfCurPic = m_currOriginalPic.FrameIdx;
    }

    Cmd.DW5.BRCFlag |= pSeqParams->bAutoMaxPBFrameSizeForSceneChange?
        CODECHAL_ENCODE_BRCUPDATE_AUTO_PB_FRAME_SIZE : 0;

    Cmd.DW5.MaxNumPAKs = m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses();

    Cmd.DW6.MinimumQP               = pParams->ucMinQP;
    Cmd.DW6.MaximumQP               = pParams->ucMaxQP;
    Cmd.DW6.EnableForceToSkip       = bForceToSkipEnable;
    Cmd.DW6.EnableSlidingWindow     = (pSeqParams->FrameSizeTolerance == EFRAMESIZETOL_LOW);
    Cmd.DW6.EnableExtremLowDelay    = (pSeqParams->FrameSizeTolerance == EFRAMESIZETOL_EXTREMELY_LOW);

    *pParams->pdBrcInitCurrentTargetBufFullInBits += dBrcInitResetInputBitsPerFrame;

    if (pSeqParams->RateControlMethod == RATECONTROL_AVBR)
    {
        Cmd.DW3.startGAdjFrame0 = (uint32_t)((10 * usAVBRConvergence) / (double)150);
        Cmd.DW3.startGAdjFrame1 = (uint32_t)((50 * usAVBRConvergence) / (double)150);
        Cmd.DW4.startGAdjFrame2 = (uint32_t)((100 * usAVBRConvergence) / (double)150);
        Cmd.DW4.startGAdjFrame3 = (uint32_t)((150 * usAVBRConvergence) / (double)150);
        Cmd.DW11.gRateRatioThreshold0 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 40)));
        Cmd.DW11.gRateRatioThreshold1 =
            (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 75)));
        Cmd.DW12.gRateRatioThreshold2 = (uint32_t)((100 - (usAVBRAccuracy / (double)30)*(100 - 97)));
        Cmd.DW12.gRateRatioThreshold3 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(103 - 100)));
        Cmd.DW12.gRateRatioThreshold4 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(125 - 100)));
        Cmd.DW12.gRateRatioThreshold5 = (uint32_t)((100 + (usAVBRAccuracy / (double)30)*(160 - 100)));
    }

    Cmd.DW15.EnableROI = pParams->ucEnableROI;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(GetInterRounding(&SliceState));

    Cmd.DW15.RoundingIntra = 5;
    Cmd.DW15.RoundingInter = SliceState.dwRoundingValue;

    uint32_t                                           dwProfileLevelMaxFrame;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalAvcEncode_GetProfileLevelMaxFrameSize(
        pSeqParams,
        this,
        (uint32_t*)&dwProfileLevelMaxFrame));

    Cmd.DW19.UserMaxFrame = dwProfileLevelMaxFrame;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PopulateBrcUpdateParam(
            &Cmd));
    )

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcMbBrcUpdate(PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);

    auto Curbe = g_cInit_CODECHAL_ENCODE_AVC_MB_BRC_UPDATE_CURBE_G9;

    // BRC Curbe requires: 2 for I-frame, 0 for P-frame, 1 for B-frame
    Curbe.DW0.CurrFrameType = (m_pictureCodingType + 1) % 3;
    if( pParams->ucEnableROI )
    {
        if (bROIValueInDeltaQP)
        {
            Curbe.DW0.EnableROI = 2; // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled
            Curbe.DW0.ROIRatio  = 0;
        }
        else
        {
            Curbe.DW0.EnableROI = 1; // 1-Enabled ROI priority, 2-Enable ROI QP Delta,  0- disabled

            uint32_t dwROISize = 0;
            uint32_t dwROIRatio = 0;

            for (uint32_t i = 0 ; i < m_avcPicParam->NumROI ; ++i)
            {
                CODECHAL_ENCODE_VERBOSEMESSAGE("ROI[%d] = {%d, %d, %d, %d} {%d}, size = %d", i,
                    m_avcPicParam->ROI[i].Left, m_avcPicParam->ROI[i].Top,
                    m_avcPicParam->ROI[i].Bottom, m_avcPicParam->ROI[i].Right,
                    m_avcPicParam->ROI[i].PriorityLevelOrDQp,
                    (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                    (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left)));
                dwROISize += (CODECHAL_MACROBLOCK_HEIGHT * MOS_ABS(m_avcPicParam->ROI[i].Top - m_avcPicParam->ROI[i].Bottom)) *
                                (CODECHAL_MACROBLOCK_WIDTH * MOS_ABS(m_avcPicParam->ROI[i].Right - m_avcPicParam->ROI[i].Left));
            }
            
            if (dwROISize)
            {
                uint32_t dwNumMBs = m_picWidthInMb * m_picHeightInMb;
                dwROIRatio = 2 * (dwNumMBs * 256 / dwROISize - 1);
                dwROIRatio = MOS_MIN(51, dwROIRatio); // clip QP from 0-51
            }
            CODECHAL_ENCODE_VERBOSEMESSAGE("ROIRatio = %d", dwROIRatio);
            Curbe.DW0.ROIRatio  = dwROIRatio;
        }
    }
    else
    {
        Curbe.DW0.ROIRatio = 0;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Curbe,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetCurbeAvcBrcBlockCopy(PCODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_PARAMS pParams)
{
    MOS_STATUS								        eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);

    CODECHAL_ENCODE_AVC_BRC_BLOCK_COPY_CURBE_CM_G9 Cmd;
    MOS_ZeroMemory(&Cmd, sizeof(Cmd));
    Cmd.DW0.BufferOffset = pParams->dwBufferOffset;
    Cmd.DW0.BlockHeight = pParams->dwBlockHeight;
    Cmd.DW1.SrcSurfaceIndex = 0x00;
    Cmd.DW2.DstSurfaceIndex = 0x01;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(pParams->pKernelState->m_dshRegion.AddData(
        &Cmd,
        pParams->pKernelState->dwCurbeOffset,
        sizeof(Cmd)));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcMbEncSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pAvcSlcParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ppRefList);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrOriginalPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pCurrReconstructedPic);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->psCurrPicSurface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pAvcPicIdx);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pMbEncBindingTable);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);

    auto pKernelState = pParams->pKernelState;
    auto pAvcMbEncBindingTable = pParams->pMbEncBindingTable;
    bool bCurrFieldPicture = CodecHal_PictureIsField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    bool bCurrBottomField = CodecHal_PictureIsBottomField(*(pParams->pCurrOriginalPic)) ? 1 : 0;
    auto CurrPicRefListEntry = pParams->ppRefList[pParams->pCurrReconstructedPic->FrameIdx];
    auto presMbCodeBuffer = &CurrPicRefListEntry->resRefMbCodeBuffer;
    auto presMvDataBuffer = &CurrPicRefListEntry->resRefMvDataBuffer;
    uint32_t dwRefMbCodeBottomFieldOffset =
        pParams->dwFrameFieldHeightInMb * pParams->dwFrameWidthInMb * 64;
    uint32_t dwRefMvBottomFieldOffset =
        MOS_ALIGN_CEIL(pParams->dwFrameFieldHeightInMb * pParams->dwFrameWidthInMb * (32 * 4), 0x1000);

    uint8_t ucVDirection, ucRefVDirection;
    if (pParams->bMbEncIFrameDistInUse)
    {
        ucVDirection = CODECHAL_VDIRECTION_FRAME;
    }
    else
    {
        ucVDirection = (CodecHal_PictureIsFrame(*(pParams->pCurrOriginalPic))) ? CODECHAL_VDIRECTION_FRAME :
            (bCurrBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;
    }

    // PAK Obj command buffer
    uint32_t dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * 4;  // 11DW + 5DW padding
    CODECHAL_SURFACE_CODEC_PARAMS               SurfaceCodecParams;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = presMbCodeBuffer;
    SurfaceCodecParams.dwSize = dwSize;
    SurfaceCodecParams.dwOffset = pParams->dwMbCodeBottomFieldOffset;
    SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMfcAvcPakObj;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    SurfaceCodecParams.bRenderTarget = true;
    SurfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // MV data buffer
    dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = presMvDataBuffer;
    SurfaceCodecParams.dwSize = dwSize;
    SurfaceCodecParams.dwOffset = pParams->dwMvBottomFieldOffset;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncIndMVData;
    SurfaceCodecParams.bRenderTarget = true;
    SurfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // Current Picture Y
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bIs2DSurface = true;
    SurfaceCodecParams.bMediaBlockRW = true; // Use media block RW for DP 2D surface access
    SurfaceCodecParams.bUseUVPlane = true;
    SurfaceCodecParams.psSurface = pParams->psCurrPicSurface;
    SurfaceCodecParams.dwOffset = pParams->dwCurrPicSurfaceOffset;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncCurrY;
    SurfaceCodecParams.dwUVBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncCurrUV;
    SurfaceCodecParams.dwVerticalLineStride = pParams->dwVerticalLineStride;
    SurfaceCodecParams.dwVerticalLineStrideOffset = pParams->dwVerticalLineStrideOffset;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // AVC_ME MV data buffer
    if (pParams->bHmeEnabled)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeMvDataBuffer);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->ps4xMeMvDataBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeMvBottomFieldOffset;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMVDataFromME;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->ps4xMeDistortionBuffer);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->ps4xMeDistortionBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeDistortionBottomFieldOffset;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMEDist;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bMbConstDataBufferInUse)
    {
        // 16 DWs per QP value
        dwSize = 16 * 52 * sizeof(uint32_t);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = pParams->presMbBrcConstDataBuffer;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMbBrcConstData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bMbQpBufferInUse)
    {
        // AVC MB BRC QP buffer
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psMbQpBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMbQpBottomFieldOffset;
        SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ? pAvcMbEncBindingTable->dwAvcMBEncMbQpField :
            pAvcMbEncBindingTable->dwAvcMBEncMbQpFrame;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bMbSpecificDataEnabled)
    {
        dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * sizeof(CODECHAL_ENCODE_AVC_MB_SPECIFIC_PARAMS);
        CODECHAL_ENCODE_VERBOSEMESSAGE("Send MB specific surface, size = %d", dwSize);
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = pParams->presMbSpecificDataBuffer;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMbSpecificData;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // Current Picture Y - VME
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bUseAdvState = true;
    SurfaceCodecParams.psSurface = pParams->psCurrPicSurface;
    SurfaceCodecParams.dwOffset = pParams->dwCurrPicSurfaceOffset;
    SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ?
        pAvcMbEncBindingTable->dwAvcMBEncFieldCurrPic[0] : pAvcMbEncBindingTable->dwAvcMBEncCurrPicFrame[0];
    SurfaceCodecParams.ucVDirection = ucVDirection;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    SurfaceCodecParams.dwBindingTableOffset = bCurrFieldPicture ?
        pAvcMbEncBindingTable->dwAvcMBEncFieldCurrPic[1] : pAvcMbEncBindingTable->dwAvcMBEncCurrPicFrame[1];
    SurfaceCodecParams.ucVDirection = ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // Setup references 1...n
    // LIST 0 references
    uint8_t ucRefIdx;
    for (ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l0_active_minus1; ucRefIdx++)
    {
        auto RefPic = pParams->pAvcSlcParams->RefPicList[LIST_0][ucRefIdx];
        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t ucRefPicIdx = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            bool  bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            uint32_t dwRefBindingTableOffset;
            // Program the surface based on current picture's field/frame mode
            if (bCurrFieldPicture) // if current picture is field
            {
                if (bRefBottomField)
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFwdPicBotField[ucRefIdx];
                }
                else
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFwdPicTopField[ucRefIdx];
                }
            }
            else // if current picture is frame
            {
                ucRefVDirection = CODECHAL_VDIRECTION_FRAME;
                dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFwdPicFrame[ucRefIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            SurfaceCodecParams.bUseAdvState = true;
            if((pParams->bUseWeightedSurfaceForL0) &&
               (pParams->pAvcSlcParams->luma_weight_flag[LIST_0] & (1 << ucRefIdx)) &&
               (ucRefIdx < CODEC_AVC_MAX_FORWARD_WP_FRAME))
            {
                SurfaceCodecParams.psSurface = &pParams->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L0_START + ucRefIdx].sBuffer;
            }
            else
            {
                SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
            }
            SurfaceCodecParams.dwWidthInUse = pParams->dwFrameWidthInMb * 16;
            SurfaceCodecParams.dwHeightInUse = pParams->dwFrameHeightInMb * 16;

            SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
            SurfaceCodecParams.ucVDirection = ucRefVDirection;
            SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));
        }
    }

    // Setup references 1...n
    // LIST 1 references
    for (ucRefIdx = 0; ucRefIdx <= pParams->pAvcSlcParams->num_ref_idx_l1_active_minus1; ucRefIdx++)
    {
        if (!bCurrFieldPicture && ucRefIdx > 0)
        {
            // Only 1 LIST 1 reference required here since only single ref is supported in frame case
            break;
        }

        auto RefPic = pParams->pAvcSlcParams->RefPicList[LIST_1][ucRefIdx];
        uint32_t dwRefMbCodeBottomFieldOffsetUsed;
        uint32_t dwRefMvBottomFieldOffsetUsed;
        uint32_t dwRefBindingTableOffset;
        if (!CodecHal_PictureIsInvalid(RefPic) && pParams->pAvcPicIdx[RefPic.FrameIdx].bValid)
        {
            uint8_t ucRefPicIdx = pParams->pAvcPicIdx[RefPic.FrameIdx].ucPicIdx;
            bool bRefBottomField = (CodecHal_PictureIsBottomField(RefPic)) ? 1 : 0;
            // Program the surface based on current picture's field/frame mode
            if (bCurrFieldPicture) // if current picture is field
            {
                if (bRefBottomField)
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_BOT_FIELD;
                    dwRefMbCodeBottomFieldOffsetUsed = dwRefMbCodeBottomFieldOffset;
                    dwRefMvBottomFieldOffsetUsed = dwRefMvBottomFieldOffset;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicBotField[ucRefIdx];
                }
                else
                {
                    ucRefVDirection = CODECHAL_VDIRECTION_TOP_FIELD;
                    dwRefMbCodeBottomFieldOffsetUsed = 0;
                    dwRefMvBottomFieldOffsetUsed = 0;
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicTopField[ucRefIdx];
                }
            }
            else // if current picture is frame
            {
                ucRefVDirection = CODECHAL_VDIRECTION_FRAME;
                dwRefMbCodeBottomFieldOffsetUsed = 0;
                dwRefMvBottomFieldOffsetUsed = 0;
                dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicFrame[ucRefIdx];
            }

            // Picture Y VME
            MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            SurfaceCodecParams.bUseAdvState = true;
            if((pParams->bUseWeightedSurfaceForL1) &&
               (pParams->pAvcSlcParams->luma_weight_flag[LIST_1] & (1 << ucRefIdx)) &&
               (ucRefIdx < CODEC_AVC_MAX_BACKWARD_WP_FRAME))
            {
                SurfaceCodecParams.psSurface = &pParams->pWeightedPredOutputPicSelectList[CODEC_AVC_WP_OUTPUT_L1_START + ucRefIdx].sBuffer;
            }
            else
            {
                SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
            }

            SurfaceCodecParams.dwWidthInUse = pParams->dwFrameWidthInMb * 16;
            SurfaceCodecParams.dwHeightInUse = pParams->dwFrameHeightInMb * 16;
            SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
            SurfaceCodecParams.ucVDirection = ucRefVDirection;
            SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));

            if (ucRefIdx == 0)
            {
                if(bCurrFieldPicture && (pParams->ppRefList[ucRefPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_FRAME || pParams->ppRefList[ucRefPicIdx]->ucAvcPictureCodingType == CODEC_AVC_PIC_CODING_TYPE_INVALID))
                {
                    dwRefMbCodeBottomFieldOffsetUsed = 0;
                    dwRefMvBottomFieldOffsetUsed     = 0;
                }
                // MB data buffer
                dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * 4;
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.dwSize = dwSize;
                SurfaceCodecParams.presBuffer = &pParams->ppRefList[ucRefPicIdx]->resRefMbCodeBuffer;
                SurfaceCodecParams.dwOffset = dwRefMbCodeBottomFieldOffsetUsed;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
                SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdRefMBData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));

                // MV data buffer
                dwSize = pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 32 * 4;
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.dwSize = dwSize;
                SurfaceCodecParams.presBuffer = &pParams->ppRefList[ucRefPicIdx]->resRefMvDataBuffer;
                SurfaceCodecParams.dwOffset = dwRefMvBottomFieldOffsetUsed;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
                SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdRefMVData;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));
            }

            if (ucRefIdx < CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
            {
                if (bCurrFieldPicture)
                {
                    // The binding table contains multiple entries for IDX0 backwards references
                    if (bRefBottomField)
                    {
                        dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicBotField[ucRefIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                    else
                    {
                        dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicTopField[ucRefIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                    }
                }
                else
                {
                    dwRefBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBwdPicFrame[ucRefIdx + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];
                }

                // Picture Y VME
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.bUseAdvState = true;
                SurfaceCodecParams.dwWidthInUse = pParams->dwFrameWidthInMb * 16;
                SurfaceCodecParams.dwHeightInUse = pParams->dwFrameHeightInMb * 16;
                SurfaceCodecParams.psSurface = &pParams->ppRefList[ucRefPicIdx]->sRefBuffer;
                SurfaceCodecParams.dwBindingTableOffset = dwRefBindingTableOffset;
                SurfaceCodecParams.ucVDirection = ucRefVDirection;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));
            }
        }
    }

    // BRC distortion data buffer for I frame
    if (pParams->bMbEncIFrameDistInUse)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psMeBrcDistortionBuffer;
        SurfaceCodecParams.dwOffset = pParams->dwMeBrcDistortionBottomFieldOffset;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncBRCDist;
        SurfaceCodecParams.bIsWritable = true;
        SurfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // RefPicSelect of Current Picture
    if (pParams->bUsedAsRef)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = &CurrPicRefListEntry->pRefPicSelectListEntry->sBuffer;
        SurfaceCodecParams.psSurface->dwHeight = MOS_ALIGN_CEIL(SurfaceCodecParams.psSurface->dwHeight, 8);
        SurfaceCodecParams.dwOffset = pParams->dwRefPicSelectBottomFieldOffset;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncRefPicSelectL0;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bMBVProcStatsEnabled)
    {
        dwSize = (bCurrFieldPicture ? 1 : 2) * pParams->dwFrameWidthInMb * pParams->dwFrameFieldHeightInMb * 16 * sizeof(uint32_t);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = pParams->presMBVProcStatsBuffer;
        SurfaceCodecParams.dwOffset = bCurrBottomField ? pParams->dwMBVProcStatsBottomFieldOffset : 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMBStats;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }
    else if (pParams->bFlatnessCheckEnabled)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psFlatnessCheckSurface;
        SurfaceCodecParams.dwOffset = bCurrBottomField ? pParams->dwFlatnessCheckBottomFieldOffset : 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncFlatnessChk;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->bMADEnabled)
    {
        dwSize = CODECHAL_MAD_BUFFER_SIZE;

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bRawSurface = true;
        SurfaceCodecParams.dwSize = dwSize;
        SurfaceCodecParams.presBuffer = pParams->presMADDataBuffer;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMADData;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (pParams->dwMbEncBRCBufferSize > 0)
    {
        //Started from GEN95, separated Mbenc curbe from BRC update kernel. BRC update kernel will generate a 128 bytes surface for mbenc.
        //The new surface contains the updated data for mbenc. MBenc kernel has been changed to use the new BRC update output surface 
        //to update its curbe internally.
        // MbEnc BRC buffer - write only
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = pParams->presMbEncBRCBuffer;
        SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(pParams->dwMbEncBRCBufferSize);
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
        SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }
    else
    {
        uint32_t dwCurbeSize;
        if (pParams->bUseMbEncAdvKernel)
        {
            // For BRC the new BRC surface is used
            if (pParams->bUseAdvancedDsh)
            {
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                SurfaceCodecParams.presBuffer = pParams->presMbEncCurbeBuffer;
                dwCurbeSize = MOS_ALIGN_CEIL(
                    pParams->pKernelState->KernelParams.iCurbeLength,
                    m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(dwCurbeSize);
                SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));
            }
            else // For CQP the DSH CURBE is used
            {
                MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
                MOS_RESOURCE *dsh = nullptr;
                CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = pParams->pKernelState->m_dshRegion.GetResource());
                SurfaceCodecParams.presBuffer = dsh;
                SurfaceCodecParams.dwOffset =
                    pParams->pKernelState->m_dshRegion.GetOffset() +
                    pParams->pKernelState->dwCurbeOffset;
                dwCurbeSize = MOS_ALIGN_CEIL(
                    pParams->pKernelState->KernelParams.iCurbeLength,
                    m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
                SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(dwCurbeSize);
                SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMbEncBRCCurbeData;
                SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE].Value;
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                    m_hwInterface,
                    pCmdBuffer,
                    &SurfaceCodecParams,
                    pKernelState));
            }
        }
    }

    if (pParams->bArbitraryNumMbsInSlice)
    {
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.psSurface = pParams->psSliceMapSurface;
        SurfaceCodecParams.bRenderTarget = false;
        SurfaceCodecParams.bIsWritable = false;
        SurfaceCodecParams.dwOffset = bCurrBottomField ? pParams->dwSliceMapBottomFieldOffset : 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncSliceMapData;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    if (!pParams->bMbEncIFrameDistInUse)
    {
        if (pParams->bMbDisableSkipMapEnabled)
        {
            MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            SurfaceCodecParams.bIs2DSurface = true;
            SurfaceCodecParams.bMediaBlockRW = true;
            SurfaceCodecParams.psSurface = pParams->psMbDisableSkipMapSurface;
            SurfaceCodecParams.dwOffset = 0;
            SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncMbNonSkipMap;
            SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC].Value;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));
        }

        if (pParams->bStaticFrameDetectionEnabled)
        {
            // static frame cost table surface
            MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            SurfaceCodecParams.presBuffer = pParams->presSFDCostTableBuffer;
            SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(CODECHAL_ENCODE_AVC_SFD_COST_TABLE_BUFFER_SIZE_G9);
            SurfaceCodecParams.dwOffset = 0;
            SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE].Value;
            SurfaceCodecParams.dwBindingTableOffset = pAvcMbEncBindingTable->dwAvcMBEncStaticDetectionCostTable;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcWPSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pKernelState);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->psInputRefBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->psOutputScaledBuffer);

    CODECHAL_SURFACE_CODEC_PARAMS SurfaceParams;
    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.bIs2DSurface = true;
    SurfaceParams.bMediaBlockRW = true;
    SurfaceParams.psSurface = pParams->psInputRefBuffer;// Input surface
    SurfaceParams.bIsWritable = false;
    SurfaceParams.bRenderTarget = false;
    SurfaceParams.dwBindingTableOffset = CODECHAL_ENCODE_AVC_WP_INPUT_REF_SURFACE_G9;
    SurfaceParams.dwVerticalLineStride = pParams->dwVerticalLineStride;
    SurfaceParams.dwVerticalLineStrideOffset = pParams->dwVerticalLineStrideOffset;
    SurfaceParams.ucVDirection = pParams->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceParams,
        pParams->pKernelState));

    MOS_ZeroMemory(&SurfaceParams, sizeof(SurfaceParams));
    SurfaceParams.bIs2DSurface = true;
    SurfaceParams.bMediaBlockRW = true;
    SurfaceParams.psSurface = pParams->psOutputScaledBuffer;// output surface
    SurfaceParams.bIsWritable = true;
    SurfaceParams.bRenderTarget = true;
    SurfaceParams.dwBindingTableOffset = CODECHAL_ENCODE_AVC_WP_OUTPUT_SCALED_SURFACE_G9;
    SurfaceParams.dwVerticalLineStride = pParams->dwVerticalLineStride;
    SurfaceParams.dwVerticalLineStrideOffset = pParams->dwVerticalLineStrideOffset;
    SurfaceParams.ucVDirection = pParams->ucVDirection;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceParams,
        pParams->pKernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcBrcFrameUpdateSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pBrcBuffers);

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS SurfaceCodecParams;
    auto pKernelState = pParams->pKernelState;
    auto pAvcBrcUpdateBindingTable = pParams->pBrcUpdateBindingTable;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = &pParams->pBrcBuffers->resBrcHistoryBuffer;
    SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(pParams->dwBrcHistoryBufferSize);
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcHistoryBuffer;
    SurfaceCodecParams.bIsWritable = true;
    SurfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // PAK Statistics buffer
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = &pParams->pBrcBuffers->resBrcPakStatisticBuffer[0];
    SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(pParams->dwBrcPakStatisticsSize);
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcPakStatisticsOutputBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t dwSize = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses());
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer =
        &pParams->pBrcBuffers->resBrcImageStatesReadBuffer[pParams->ucCurrRecycledBufIdx];
    SurfaceCodecParams.dwSize = dwSize;
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcImageStateReadBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // PAK IMG_STATEs buffer - write only
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = &pParams->pBrcBuffers->resBrcImageStatesWriteBuffer;
    SurfaceCodecParams.dwSize = dwSize;
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcImageStateWriteBuffer;
    SurfaceCodecParams.bIsWritable = true;
    SurfaceCodecParams.bRenderTarget = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    if (pParams->dwMbEncBRCBufferSize > 0)
    {
        //Started from GEN95, separated Mbenc curbe from BRC update kernel. BRC update kernel will generate a 128 bytes surface for mbenc.
        //The new surface contains the updated data for mbenc. MBenc kernel has been changed to use the new BRC update output surface 
        //to update its curbe internally.
        // MbEnc BRC buffer - write only
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.presBuffer = &pParams->pBrcBuffers->resMbEncBrcBuffer;
        SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(pParams->dwMbEncBRCBufferSize);

        if (IsMfeMbEncEnabled(false))
        {
            SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
                m_hwInterface,
                pCmdBuffer,
                &SurfaceCodecParams,
                pKernelState));
        }

        SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        SurfaceCodecParams.bIsWritable = true;
        SurfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }
    else
    {
        PMHW_KERNEL_STATE pMbEncKernelState;
        CODECHAL_ENCODE_CHK_NULL_RETURN(pMbEncKernelState = pParams->pBrcBuffers->pMbEncKernelStateInUse);

        // BRC ENC CURBE Buffer - read only
        dwSize = MOS_ALIGN_CEIL(
            pMbEncKernelState->KernelParams.iCurbeLength,
            m_renderEngineInterface->m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        MOS_RESOURCE *dsh = nullptr;
        CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = pMbEncKernelState->m_dshRegion.GetResource());
        SurfaceCodecParams.presBuffer = dsh;
        SurfaceCodecParams.dwOffset =
            pMbEncKernelState->m_dshRegion.GetOffset() +
            pMbEncKernelState->dwCurbeOffset;
        SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(dwSize);
        SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeReadBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));

        // BRC ENC CURBE Buffer - write only
        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        if (pParams->bUseAdvancedDsh)
        {
            SurfaceCodecParams.presBuffer = pParams->presMbEncCurbeBuffer;
        }
        else
        {
            MOS_RESOURCE *dsh = nullptr;
            CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = pMbEncKernelState->m_dshRegion.GetResource());
            SurfaceCodecParams.presBuffer = dsh;
            SurfaceCodecParams.dwOffset =
                pMbEncKernelState->m_dshRegion.GetOffset() +
                pMbEncKernelState->dwCurbeOffset;
        }
        SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(dwSize);
        SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcMbEncCurbeWriteData;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // AVC_ME BRC Distortion data buffer - input/output
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bIs2DSurface = true;
    SurfaceCodecParams.bMediaBlockRW = true;
    SurfaceCodecParams.psSurface = &pParams->pBrcBuffers->sMeBrcDistortionBuffer;
    SurfaceCodecParams.dwOffset = pParams->pBrcBuffers->dwMeBrcDistortionBottomFieldOffset;
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcDistortionBuffer;
    SurfaceCodecParams.bRenderTarget = true;
    SurfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // BRC Constant Data Surface
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.bIs2DSurface = true;
    SurfaceCodecParams.bMediaBlockRW = true;
    SurfaceCodecParams.psSurface =
        &pParams->pBrcBuffers->sBrcConstantDataBuffer[pParams->ucCurrRecycledBufIdx];
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // MBStat buffer - input
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = pParams->presMbStatBuffer;
    SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

	// MV data buffer
	if (pParams->psMvDataBuffer)
	{
		memset(&SurfaceCodecParams, 0, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
		SurfaceCodecParams.bIs2DSurface = true;
		SurfaceCodecParams.bMediaBlockRW = true;
		SurfaceCodecParams.psSurface = pParams->psMvDataBuffer;
		SurfaceCodecParams.dwOffset = pParams->dwMvBottomFieldOffset;
		SurfaceCodecParams.dwCacheabilityControl = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE].Value;
		SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwFrameBrcMvDataBuffer;
		CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
			m_hwInterface,
			pCmdBuffer,
			&SurfaceCodecParams,
			pKernelState));
	}
	
    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SendAvcBrcMbUpdateSurfaces(PMOS_COMMAND_BUFFER pCmdBuffer, PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS pParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(pCmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(pParams->pBrcBuffers);

    // BRC history buffer
    auto pKernelState = pParams->pKernelState;
    auto pAvcBrcUpdateBindingTable = pParams->pBrcUpdateBindingTable;
    CODECHAL_SURFACE_CODEC_PARAMS                   SurfaceCodecParams;
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = &pParams->pBrcBuffers->resBrcHistoryBuffer;
    SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(pParams->dwBrcHistoryBufferSize);
    SurfaceCodecParams.bIsWritable = true;
    SurfaceCodecParams.bRenderTarget = true;
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwMbBrcHistoryBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    // AVC MB QP data buffer
    if (pParams->bMbBrcEnabled)
    {
        pParams->pBrcBuffers->sBrcMbQpBuffer.dwHeight = MOS_ALIGN_CEIL((pParams->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.bIsWritable = true;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.psSurface = &pParams->pBrcBuffers->sBrcMbQpBuffer;
        SurfaceCodecParams.dwOffset = pParams->pBrcBuffers->dwBrcMbQpBottomFieldOffset;
        SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwMbBrcMbQpBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // BRC ROI feature
    if (pParams->bBrcRoiEnabled)
    {
        pParams->psRoiSurface->dwHeight = MOS_ALIGN_CEIL((pParams->dwDownscaledFrameFieldHeightInMb4x << 2), 8);

        MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        SurfaceCodecParams.bIs2DSurface = true;
        SurfaceCodecParams.bMediaBlockRW = true;
        SurfaceCodecParams.bIsWritable = false;
        SurfaceCodecParams.bRenderTarget = true;
        SurfaceCodecParams.psSurface = pParams->psRoiSurface;
        SurfaceCodecParams.dwOffset = 0;
        SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwMbBrcROISurface;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
            m_hwInterface,
            pCmdBuffer,
            &SurfaceCodecParams,
            pKernelState));
    }

    // MBStat buffer
    MOS_ZeroMemory(&SurfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    SurfaceCodecParams.presBuffer = pParams->presMbStatBuffer;
    SurfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_hwInterface->m_avcMbStatBufferSize);
    SurfaceCodecParams.dwBindingTableOffset = pAvcBrcUpdateBindingTable->dwMbBrcMbStatBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_SetRcsSurfaceState(
        m_hwInterface,
        pCmdBuffer,
        &SurfaceCodecParams,
        pKernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeAvcEncG9::SetupROISurface()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_LOCK_PARAMS ReadOnly;
    MOS_ZeroMemory(&ReadOnly, sizeof(ReadOnly));
    ReadOnly.ReadOnly = 1;
    uint32_t* pU32Data = (uint32_t*)m_osInterface->pfnLockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource, &ReadOnly);
    if (!pU32Data)
    {
        eStatus = MOS_STATUS_INVALID_HANDLE;
        return eStatus;
    }

    uint32_t dwBufferWidthInByte = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x << 4), 64);//(m_picWidthInMb * 4 + 63) & ~63;
    uint32_t dwBufferHeightInByte = MOS_ALIGN_CEIL((m_downscaledHeightInMb4x << 2), 8);//(m_picHeightInMb + 7) & ~7;
    uint32_t uNumMBs = m_picWidthInMb * m_picHeightInMb;
    for (uint32_t uMB = 0; uMB <= uNumMBs; uMB++)
    {
        int32_t iCurMbY = uMB / m_picWidthInMb;
        int32_t iCurMbX = uMB - iCurMbY * m_picWidthInMb;

        uint32_t outdata = 0;
        for (int32_t iRoi = (m_avcPicParam->NumROI - 1); iRoi >= 0; iRoi--)
        {
            int32_t iQPLevel;
            if (bROIValueInDeltaQP)
            {
                iQPLevel = -m_avcPicParam->ROI[iRoi].PriorityLevelOrDQp;
            }
            else
            {
                // QP Level sent to ROI surface is (priority * 6)
                iQPLevel = m_avcPicParam->ROI[iRoi].PriorityLevelOrDQp * 6;
            }

            if (iQPLevel == 0)
            {
                continue;
            }

            if ((iCurMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left) && (iCurMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right) &&
                (iCurMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top) && (iCurMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom))
            {
                outdata = 15 | ((iQPLevel & 0xFF) << 8);
            }
            else if (bROISmoothEnabled)
            {
                if ((iCurMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left - 1) && (iCurMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right + 1) &&
                    (iCurMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top - 1) && (iCurMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom + 1))
                {
                    outdata = 14 | ((iQPLevel & 0xFF) << 8);
                }
                else if ((iCurMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left - 2) && (iCurMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right + 2) &&
                    (iCurMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top - 2) && (iCurMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom + 2))
                {
                    outdata = 13 | ((iQPLevel & 0xFF) << 8);
                }
                else if ((iCurMbX >= (int32_t)m_avcPicParam->ROI[iRoi].Left - 3) && (iCurMbX < (int32_t)m_avcPicParam->ROI[iRoi].Right + 3) &&
                    (iCurMbY >= (int32_t)m_avcPicParam->ROI[iRoi].Top - 3) && (iCurMbY < (int32_t)m_avcPicParam->ROI[iRoi].Bottom + 3))
                {
                    outdata = 12 | ((iQPLevel & 0xFF) << 8);
                }
            }
        }
        pU32Data[(iCurMbY * (dwBufferWidthInByte>>2)) + iCurMbX] = outdata;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, &BrcBuffers.sBrcRoiSurface.OsResource);

    uint32_t dwBufferSize = dwBufferWidthInByte * dwBufferHeightInByte;
    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
        &BrcBuffers.sBrcRoiSurface.OsResource,
        CodechalDbgAttr::attrInput,
        "ROI",
        dwBufferSize,
        0,
        CODECHAL_MEDIA_STATE_MB_BRC_UPDATE)));
    return eStatus;
}

bool CodechalEncodeAvcEncG9::IsMfeMbEncEnabled(bool bMbEncIFrameDistInUse)
{
    return bMbEncIFrameDistInUse ? false : m_mfeMbEncEanbled;
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
        if (pCmEvent[nCmEventCheckIdx] != nullptr)
        {
            pCmEvent[nCmEventCheckIdx]->WaitForTaskFinished();
            pCmQueue->DestroyEvent(pCmEvent[nCmEventCheckIdx]);
            pCmEvent[nCmEventCheckIdx] = nullptr;
            nCmEventCheckIdx = (nCmEventCheckIdx + 1 ) % CM_EVENT_NUM;
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
        avcPar->MBBRCEnable          = bMbBrcEnabled;
        avcPar->MBRC                 = bMbBrcEnabled;
        avcPar->BitRate              = curbe->DW3.AverageBitRate;
        avcPar->InitVbvFullnessInBit = curbe->DW1.InitBufFullInBits;
        avcPar->MaxBitRate           = curbe->DW4.MaxBitRate;
        avcPar->VbvSzInBit           = curbe->DW2.BufSizeInBits;
        avcPar->AvbrAccuracy         = curbe->DW10.AVBRAccuracy;
        avcPar->AvbrConvergence      = curbe->DW11.AVBRConvergence;
        avcPar->SlidingWindowSize    = curbe->DW22.SlidingWindowSize;
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
        avcPar->EnableMultipass     = (curbe->DW5.MaxNumPAKs > 0) ? 1 : 0;
        avcPar->MaxNumPakPasses     = curbe->DW5.MaxNumPAKs;
        avcPar->SlidingWindowEnable = curbe->DW6.EnableSlidingWindow;
        avcPar->FrameSkipEnable     = curbe->DW6.EnableForceToSkip;
        avcPar->UserMaxFrame        = curbe->DW19.UserMaxFrame;
    }
    else
    {
        avcPar->UserMaxFrameP       = curbe->DW19.UserMaxFrame;
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
        avcPar->MRDisableQPCheck                    = MRDisableQPCheck[m_targetUsage];
        avcPar->AllFractional = 
            CODECHAL_ENCODE_AVC_AllFractional_Common[m_targetUsage & 0x7];
        avcPar->DisableAllFractionalCheckForHighRes = 
            CODECHAL_ENCODE_AVC_DisableAllFractionalCheckForHighRes_Common[m_targetUsage & 0x7];
        avcPar->EnableAdaptiveSearch                = curbe->common.DW37.AdaptiveEn;
        avcPar->EnableFBRBypass                     = curbe->common.DW4.EnableFBRBypass;
        avcPar->BlockBasedSkip                      = curbe->common.DW3.BlockBasedSkipEnable;
        avcPar->MADEnableFlag                       = curbe->common.DW34.MADEnableFlag;
        avcPar->MBTextureThreshold                  = curbe->common.DW58.MBTextureThreshold;
        avcPar->EnableMBFlatnessCheckOptimization   = curbe->common.DW34.EnableMBFlatnessChkOptimization;
        avcPar->EnableArbitrarySliceSize            = curbe->common.DW34.ArbitraryNumMbsPerSlice;
        avcPar->RefThresh                           = curbe->common.DW38.RefThreshold;
        avcPar->EnableWavefrontOptimization         = curbe->common.DW4.EnableWavefrontOptimization;
        avcPar->MaxLenSP                            = curbe->common.DW2.LenSP;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        avcPar->MEMethod                            = meMethod;
        avcPar->HMECombineLen                       = HMECombineLen[m_targetUsage];
        avcPar->FTQBasedSkip                        = FTQBasedSkip[m_targetUsage];
        avcPar->MultiplePred                        = MultiPred[m_targetUsage];
        avcPar->EnableAdaptiveIntraScaling          = bAdaptiveIntraScalingEnable;
        avcPar->StaticFrameIntraCostScalingRatioP   = 240;
        avcPar->SubPelMode                          = curbe->common.DW3.SubPelMode;
        avcPar->HMECombineOverlap                   = curbe->common.DW36.HMECombineOverlap;
        avcPar->SearchX                             = curbe->common.DW5.RefWidth;
        avcPar->SearchY                             = curbe->common.DW5.RefHeight;
        avcPar->SearchControl                       = curbe->common.DW3.SearchCtrl;
        avcPar->EnableAdaptiveTxDecision            = curbe->common.DW34.EnableAdaptiveTxDecision;
        avcPar->TxDecisionThr                       = curbe->common.DW58.TxDecisonThreshold;
        avcPar->EnablePerMBStaticCheck              = curbe->common.DW34.EnablePerMBStaticCheck;
        avcPar->EnableAdaptiveSearchWindowSize      = curbe->common.DW34.EnableAdaptiveSearchWindowSize;
        avcPar->EnableIntraCostScalingForStaticFrame = curbe->common.DW4.EnableIntraCostScalingForStaticFrame;
        avcPar->BiMixDisable                        = curbe->common.DW0.BiMixDis;
        avcPar->SurvivedSkipCost                    = (curbe->common.DW7.NonSkipZMvAdded << 1) + curbe->common.DW7.NonSkipModeAdded;
        avcPar->UniMixDisable                       = curbe->common.DW1.UniMixDisable;
    }
    else if (m_pictureCodingType == B_TYPE)
    {
        avcPar->BMEMethod                           = meMethod;
        avcPar->HMEBCombineLen                      = HMEBCombineLen[m_targetUsage];
        avcPar->StaticFrameIntraCostScalingRatioB   = 200;
        avcPar->BSearchX                            = curbe->common.DW5.RefWidth;
        avcPar->BSearchY                            = curbe->common.DW5.RefHeight;
        avcPar->BSearchControl                      = curbe->common.DW3.SearchCtrl;
        avcPar->BSkipType                           = curbe->common.DW3.SkipType;
        avcPar->DirectMode                          = curbe->common.DW34.bDirectMode;
        avcPar->BiWeight                            = curbe->common.DW1.BiWeight;
    }

    return MOS_STATUS_SUCCESS;
}
#endif
