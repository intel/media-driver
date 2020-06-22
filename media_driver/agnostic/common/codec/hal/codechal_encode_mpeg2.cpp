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
//! \file     codechal_encode_mpeg2.cpp
//! \brief    Defines base class for MPEG2 dual-pipe encoder.
//!
#include "codechal_encode_mpeg2.h"
#include "codechal_mmc_encode_mpeg2.h"
#include "codechal_kernel_hme.h"
#include "codeckrnheader.h"

#define CODECHAL_ENCODE_MPEG2_FCODE_X(width) ((width < 200) ? 3 : (width < 500) ? 4 : (width < 1400) ? 5 : 6)
#define CODECHAL_ENCODE_MPEG2_FCODE_Y(fcodeX) ((fcodeX > 5) ? 5 : fcodeX)

//!
//! \enum     ProfileIdc
//! \brief    Profile idc
//!
enum ProfileIdc
{
    highProfile      = 0x10,
    spatialProfile   = 0x20,
    snrProfile       = 0x30,
    mainProfile      = 0x40,
    simpleProfile    = 0x50
} ;

//!
//! \enum     LevelIdc
//! \brief    Level idc
//!
enum LevelIdc
{
    levelHigh        = 4,
    levelHigh1440    = 6,
    levelMain        = 8,
    levelLow         = 10
} ;

//!
//! \enum     StartCode
//! \brief    Start code
//!
enum StartCode
{
    startCodePrefix         = 0x000001, // bit string 0000 0000 0000 0000 0000 0001
    startCodePicture        = 0x00,
    // slice_start_code = 0x01..0xAF, it is the slice_vertical_position for the slice
    // 0xB0, 0xB1, 0xB6 - Reserved
    startCodeUserData       = 0xB2,
    startCodeSequenceHeader = 0xB3,
    startCodeSequenceError  = 0xB4,
    startCodeExtension      = 0xB5,
    startCodeSequenceEnd    = 0xB7,
    startCodeGroupStart     = 0xB8
    // system start codes = 0xB9..0xFF
} ;

//!
//! \enum     BingdingTableOffsetBrcInitReset
//! \brief    Bingding table offset brc init reset
//!
enum BingdingTableOffsetBrcInitReset
{
    brcInitResetHistory                 = 0,
    brcInitResetDistortion              = 1,
    brcInitResetNumBindingTableEntries  = 2
} ;

//!
//! \enum     BindingTableOffsetBrcUpdate
//! \brief    Binding table offset brc update
//!
enum BindingTableOffsetBrcUpdate
{
    brcUpdateHistory                    = 0,
    brcUpdatePakStaticOutput            = 1,
    brcUpdatePictureStateRead           = 2,
    brcUpdatePictureStateWrite          = 3,
    brcUpdateMbencCurbeRead             = 4,
    brcUpdateMbencCurbeWrite            = 5,
    brcUpdateDistortion                 = 6,
    brcUpdateConstantData               = 7,
    brcUpdatePicHeaderInputData         = 8,
    brcUpdateOutputData                 = 9,
    brcUpdateNumBindingTableEntries     = 10
} ;

//!
//! \class    BrcInitResetCurbe
//! \brief    Brc initialization reset curbe
//!
class BrcInitResetCurbe
{
public:
    //!
    //! \struct    CurbeData
    //! \brief     Curbe data
    //!
    struct CurbeData
    {
        union
        {
            struct
            {
                uint32_t m_profileLevelMaxFrame         : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        union
        {
            struct
            {
                uint32_t m_initBufFullInBits            : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        union
        {
            struct
            {
                uint32_t m_bufSizeInBits               : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        union
        {
            struct
            {
                uint32_t m_averageBitRate               : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        union
        {
            struct
            {
                uint32_t m_maxBitRate                   : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        union
        {
            struct
            {
                uint32_t m_minBitRate                   : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW5;

        union
        {
            struct
            {
                uint32_t m_frameRateM                     : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW6;

        union
        {
            struct
            {
                uint32_t m_frameRateD                   : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW7;

        union
        {
            struct
            {
                uint32_t m_brcFlag                        : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_gopP                           : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW8;

        union
        {
            struct
            {
                uint32_t m_gopB                            : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_frameWidthInBytes               : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW9;

        union
        {
            struct
            {
                uint32_t m_frameHeightInBytes             : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_avbrAccuracy                   : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW10;

        union
        {
            struct
            {
                uint32_t m_avbrConvergence                : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_minQP                          : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW11;

        union
        {
            struct
            {
                uint32_t m_maxQP                          : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_noSlices                       : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW12;

        union
        {
            struct
            {
                uint32_t m_instantRateThreshold0ForP      : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_instantRateThreshold1ForP      : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_instantRateThreshold2ForP      : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_instantRateThreshold3ForP      : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW13;

        union
        {
            struct
            {
                uint32_t m_instantRateThreshold0ForB      : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_instantRateThreshold1ForB      : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_instantRateThreshold2ForB      : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_instantRateThreshold3ForB      : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW14;

        union
        {
            struct
            {
                uint32_t m_instantRateThreshold0ForI      : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_instantRateThreshold1ForI      : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_instantRateThreshold2ForI      : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_instantRateThreshold3ForI      : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW15;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold0ForPandB    : MOS_BITFIELD_RANGE(0,7);       // Signed byte
                uint32_t m_deviationThreshold1ForPandB    : MOS_BITFIELD_RANGE(8,15);      // Signed byte
                uint32_t m_deviationThreshold2ForPandB    : MOS_BITFIELD_RANGE(16,23);     // Signed byte
                uint32_t m_deviationThreshold3ForPandB    : MOS_BITFIELD_RANGE(24,31);     // Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } DW16;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold4ForPandB    : MOS_BITFIELD_RANGE(0,7);     // Signed byte
                uint32_t m_deviationThreshold5ForPandB    : MOS_BITFIELD_RANGE(8,15);     // Signed byte
                uint32_t m_deviationThreshold6ForPandB    : MOS_BITFIELD_RANGE(16,23);     // Signed byte
                uint32_t m_deviationThreshold7ForPandB    : MOS_BITFIELD_RANGE(24,31);     // Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } DW17;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold0ForVBR      : MOS_BITFIELD_RANGE(0,7);     // Signed byte
                uint32_t m_deviationThreshold1ForVBR      : MOS_BITFIELD_RANGE(8,15);     // Signed byte
                uint32_t m_deviationThreshold2ForVBR      : MOS_BITFIELD_RANGE(16,23);     // Signed byte
                uint32_t m_deviationThreshold3ForVBR      : MOS_BITFIELD_RANGE(24,31);     // Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } DW18;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold4ForVBR      : MOS_BITFIELD_RANGE(0,7);     // Signed byte
                uint32_t m_deviationThreshold5ForVBR      : MOS_BITFIELD_RANGE(8,15);     // Signed byte
                uint32_t m_deviationThreshold6ForVBR      : MOS_BITFIELD_RANGE(16,23);     // Signed byte
                uint32_t m_deviationThreshold7ForVBR      : MOS_BITFIELD_RANGE(24,31);     // Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } DW19;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold0ForI        : MOS_BITFIELD_RANGE(0,7);     // Signed byte
                uint32_t m_deviationThreshold1ForI        : MOS_BITFIELD_RANGE(8,15);     // Signed byte
                uint32_t m_deviationThreshold2ForI        : MOS_BITFIELD_RANGE(16,23);     // Signed byte
                uint32_t m_deviationThreshold3ForI        : MOS_BITFIELD_RANGE(24,31);     // Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } DW20;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold4ForI        : MOS_BITFIELD_RANGE(0,7);     // Signed byte
                uint32_t m_deviationThreshold5ForI        : MOS_BITFIELD_RANGE(8,15);     // Signed byte
                uint32_t m_deviationThreshold6ForI        : MOS_BITFIELD_RANGE(16,23);     // Signed byte
                uint32_t m_deviationThreshold7ForI        : MOS_BITFIELD_RANGE(24,31);     // Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } DW21;

        union
        {
            struct
            {
                uint32_t m_value;
            };
        } DW22;

        union
        {
            struct
            {
                uint32_t m_value;
            };
        } DW23;

        union
        {
            struct
            {
                uint32_t m_value;
            };
        } DW24;

        union
        {
            struct
            {
                uint32_t m_value;
            };
        } DW25;
    }m_curbeData;

    //!
    //! \brief    Constructor
    //!
    BrcInitResetCurbe();

    //!
    //! \brief    Destructor
    //!
    ~BrcInitResetCurbe(){};

    static const size_t m_byteSize = sizeof(CurbeData);

};

//!
//! \struct    BrcUpdateCurbe
//! \brief     BRC update curbe
//!
class BrcUpdateCurbe
{
public:
    //!
    //! \struct    CurbeData
    //! \brief     Curbe data
    //!
    struct CurbeData
    {
        union
        {
            struct
            {
                uint32_t m_targetSize                 : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        union
        {
            struct
            {
                uint32_t m_frameNumber                : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        union
        {
            struct
            {
                uint32_t m_sliceNumber                : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        union
        {
            struct
            {
                uint32_t m_startGAdjFrame0            : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_startGAdjFrame1            : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        union
        {
            struct
            {
                uint32_t m_startGAdjFrame2            : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_startGAdjFrame3            : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        union
        {
            struct
            {
                uint32_t m_targetSizeFlag             : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_brcFlag                    : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_maxNumPAKs                 : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_currFrameType              : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW5;

        // This offset indicates the byte position of the q_scale_type bit
        // in the 2nd level batch buffer containing the INSERT_OBJ command
        // for inserting the picture header data into the bitstream.
        // This offset includes the 8 bytes of the INSERT command at the
        // beginning of the buffer.
        union
        {
            struct
            {
                uint32_t m_qScaleTypeOffset           : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_vbvDelay                   : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW6;

        // This size is the size of the entire 2nd level batch buffer
        // containing the INSERT_OBJ command for inserting the
        // picture header data into the bitstream. It includes the batch buffer end
        // command at the end of the buffer.
        union
        {
            struct
            {
                uint32_t m_picHeaderDataBufferSize    :MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };

        } DW7;

        union
        {
            struct
            {
                uint32_t m_startGlobalAdjustMult0     : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_startGlobalAdjustMult1     : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_startGlobalAdjustMult2     : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_startGlobalAdjustMult3     : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW8;

        union
        {
            struct
            {
                uint32_t m_startGlobalAdjustMult4     : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_startGlobalAdjustDiv0      : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_startGlobalAdjustDiv1      : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_startGlobalAdjustDiv2      : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW9;

        union
        {
            struct
            {
                uint32_t m_startGlobalAdjustDiv3      : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_startGlobalAdjustDiv4      : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_qpThreshold0               : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_qpThreshold1               : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW10;

        union
        {
            struct
            {
                uint32_t m_qpThreshold2               : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_qpThreshold3               : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_gRateRatioThreshold0       : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_gRateRatioThreshold1       : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW11;

        union
        {
            struct
            {
                uint32_t m_gRateRatioThreshold2       : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_gRateRatioThreshold3       : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_gRateRatioThreshold4       : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_gRateRatioThreshold5       : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW12;

        union
        {
            struct
            {
                uint32_t m_gRateRatioThresholdQP0     : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_gRateRatioThresholdQP1     : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_gRateRatioThresholdQP2     : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_gRateRatioThresholdQP3     : MOS_BITFIELD_RANGE(24,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW13;

        union
        {
            struct
            {
                uint32_t m_gRateRatioThresholdQP4     : MOS_BITFIELD_RANGE(0,7);
                uint32_t m_gRateRatioThresholdQP5     : MOS_BITFIELD_RANGE(8,15);
                uint32_t m_gRateRatioThresholdQP6     : MOS_BITFIELD_RANGE(16,23);
                uint32_t m_forceToSkip                : MOS_BITFIELD_RANGE(24,24);
                uint32_t m_reserved25                 : MOS_BITFIELD_RANGE(25,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW14;

        union
        {
            struct
            {
                uint32_t m_extraHeaders               : MOS_BITFIELD_RANGE(0,15);
                uint32_t m_intraDcPrecisionOffset     : MOS_BITFIELD_RANGE(16,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW15;

        union
        {
            struct
            {
                uint32_t m_value;
            };
        } DW16[16];

        union
        {
            struct
            {
                uint32_t m_bindingTableIndex          : MOS_BITFIELD_RANGE(0,31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW32[10];
    }m_curbeData;

    //!
    //! \brief    Constructor
    //!
    BrcUpdateCurbe();

    //!
    //! \brief    Destructor
    //!
    ~BrcUpdateCurbe(){};

    static const size_t m_byteSize = sizeof(CurbeData);

 } ;

//!
//! \struct    VLCode
//! \brief     VL code
//!
struct VLCode{
    uint32_t m_code;
    uint32_t m_len;
};

//!
//! \struct MediaObjectInlineDataMpeg2
//! \brief  Media object inline data
//!
struct MediaObjectInlineDataMpeg2
{
    // DW0
    union
    {
        struct
        {
            uint32_t   m_mbX        : 8;    //<! in MB unit
            uint32_t   m_mbY        : 8;    //<! in MB unit
            uint32_t   m_reserved   : 16;
        };
        struct
        {
            uint32_t   m_value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   m_lastMbInSliceGroup : 8;
            uint32_t   m_firstMbInSliceGroup : 8;
            uint32_t   m_lastMbInSlice : 8;
            uint32_t   m_firstMbInSlice : 8;
        };
        struct
        {
            uint32_t   m_value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t   m_batchBufferEnd : 32;
        };
        struct
        {
            uint32_t   m_value;
        };
    } DW2;
} ;

//!
//! \struct    SliceRecord
//! \brief     Slice record
//!
struct SliceRecord
{
    // DW 1
    union
    {
        struct
        {
            uint32_t   m_lastMbInSliceGroup  : 8;
            uint32_t   m_firstMbInSliceGroup : 8;
            uint32_t   m_lastMbInSlice       : 8;
            uint32_t   m_firstMbInSlice      : 8;
        };
        struct
        {
            uint32_t   m_value;
        };
    } DW1;

    // DW 2
    union
    {
        struct
        {
            uint32_t   m_batchBufferEnd      : 32;
        };
        struct
        {
            uint32_t   m_value;
        };
    } DW2;
} ;

 /* VL codes for macroblock_address_increment ISO/IEC 13818-2, B.1, Table B-1. */
static const VLCode mpeg2AddrIncreamentTbl[35] =
{
    { 0x00, 0 }, // forbidden m_value
    { 0x01, 1 },
    { 0x03, 3 }, { 0x02, 3 },
    { 0x03, 4 }, { 0x02, 4 },
    { 0x03, 5 }, { 0x02, 5 },
    { 0x07, 7 }, { 0x06, 7 },
    { 0x0b, 8 }, { 0x0a, 8 }, { 0x09, 8 }, { 0x08, 8 }, { 0x07, 8 }, { 0x06, 8 },
    { 0x17, 10 }, { 0x16, 10 }, { 0x15, 10 }, { 0x14, 10 }, { 0x13, 10 }, { 0x12, 10 },
    { 0x23, 11 }, { 0x22, 11 }, { 0x21, 11 }, { 0x20, 11 }, { 0x1f, 11 }, { 0x1e, 11 }, { 0x1d, 11 }, { 0x1c, 11 }, { 0x1b, 11 }, { 0x1a, 11 }, { 0x19, 11 }, { 0x18, 11 },
    { 0x08, 11 } // macroblock_escape
};

/* VL codes for macroblock_type ISO/IEC 13818-2, B.2, Tables B-2, B-3, B-4. */
static const VLCode mpeg2MbTypeTbl[3][32] =
{
    /* I */
    {
        { 0x00, 0 }, { 0x01, 1 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x01, 2 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }
    },
    /* P */
    {
        { 0x00, 0 }, { 0x03, 5 }, { 0x01, 2 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 },
        { 0x01, 3 }, { 0x00, 0 }, { 0x01, 1 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x01, 6 }, { 0x01, 5 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x00, 0 }, { 0x02, 5 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }
    },
    /* B */
    {
        { 0x00, 0 }, { 0x03, 5 }, { 0x00, 0 }, { 0x00, 0 }, { 0x02, 3 }, { 0x00, 0 }, { 0x03, 3 }, { 0x00, 0 },
        { 0x02, 4 }, { 0x00, 0 }, { 0x03, 4 }, { 0x00, 0 }, { 0x02, 2 }, { 0x00, 0 }, { 0x03, 2 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x01, 6 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x02, 6 }, { 0x00, 0 },
        { 0x00, 0 }, { 0x00, 0 }, { 0x03, 6 }, { 0x00, 0 }, { 0x00, 0 }, { 0x00, 0 }, { 0x02, 5 }, { 0x00, 0 }
    }
};

/* VL codes for motion_code+16 ISO/IEC 13818-2, B.4, Table B-10. */
static const VLCode mpeg2MvVlcTbl[33] =
{
    // negative motion_code
    { 0x19, 11 }, { 0x1b, 11 }, { 0x1d, 11 }, { 0x1f, 11 }, { 0x21, 11 }, { 0x23, 11 },
    { 0x13, 10 }, { 0x15, 10 }, { 0x17, 10 },
    { 0x07, 8 }, { 0x09, 8 }, { 0x0b, 8 },
    { 0x07, 7 },
    { 0x03, 5 },
    { 0x03, 4 },
    { 0x03, 3 },
    // zero motion_code
    { 0x01, 1 },
    // positive motion_code
    { 0x02, 3 },
    { 0x02, 4 },
    { 0x02, 5 },
    { 0x06, 7 },
    { 0x0a, 8 }, { 0x08, 8 }, { 0x06, 8 },
    { 0x16, 10 }, { 0x14, 10 }, { 0x12, 10 },
    { 0x22, 11 }, { 0x20, 11 }, { 0x1e, 11 }, { 0x1c, 11 }, { 0x1a, 11 }, { 0x18, 11 }
};

BrcInitResetCurbe::BrcInitResetCurbe()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_curbeData, m_byteSize);

    m_curbeData.DW10.m_avbrAccuracy                   = 30;
    m_curbeData.DW11.m_avbrConvergence                = 150;
    m_curbeData.DW11.m_minQP                          = 1;
    m_curbeData.DW12.m_maxQP                          = 112;
    m_curbeData.DW12.m_noSlices                       = 1;
    m_curbeData.DW13.m_instantRateThreshold0ForP      = 30;
    m_curbeData.DW13.m_instantRateThreshold1ForP      = 50;
    m_curbeData.DW13.m_instantRateThreshold2ForP      = 70;
    m_curbeData.DW13.m_instantRateThreshold3ForP      = 120;
    m_curbeData.DW14.m_instantRateThreshold0ForB      = 25;
    m_curbeData.DW14.m_instantRateThreshold1ForB      = 50;
    m_curbeData.DW14.m_instantRateThreshold2ForB      = 70;
    m_curbeData.DW14.m_instantRateThreshold3ForB      = 120;
    m_curbeData.DW15.m_instantRateThreshold0ForI      = 30;
    m_curbeData.DW15.m_instantRateThreshold1ForI      = 50;
    m_curbeData.DW15.m_instantRateThreshold2ForI      = 90;
    m_curbeData.DW15.m_instantRateThreshold3ForI      = 115;
    m_curbeData.DW16.m_deviationThreshold0ForPandB    = MOS_BITFIELD_VALUE((uint32_t)-45, 8);
    m_curbeData.DW16.m_deviationThreshold1ForPandB    = MOS_BITFIELD_VALUE((uint32_t)-33, 8);
    m_curbeData.DW16.m_deviationThreshold2ForPandB    = MOS_BITFIELD_VALUE((uint32_t)-23, 8);
    m_curbeData.DW16.m_deviationThreshold3ForPandB    = MOS_BITFIELD_VALUE((uint32_t)-15, 8);
    m_curbeData.DW17.m_deviationThreshold4ForPandB    = 15;
    m_curbeData.DW17.m_deviationThreshold5ForPandB    = 23;
    m_curbeData.DW17.m_deviationThreshold6ForPandB    = 35;
    m_curbeData.DW17.m_deviationThreshold7ForPandB    = 45;
    m_curbeData.DW18.m_deviationThreshold0ForVBR      = MOS_BITFIELD_VALUE((uint32_t)-45, 8);
    m_curbeData.DW18.m_deviationThreshold1ForVBR      = MOS_BITFIELD_VALUE((uint32_t)-35, 8);
    m_curbeData.DW18.m_deviationThreshold2ForVBR      = MOS_BITFIELD_VALUE((uint32_t)-25, 8);
    m_curbeData.DW18.m_deviationThreshold3ForVBR      = MOS_BITFIELD_VALUE((uint32_t)-15, 8);
    m_curbeData.DW19.m_deviationThreshold4ForVBR      = 40;
    m_curbeData.DW19.m_deviationThreshold5ForVBR      = 50;
    m_curbeData.DW19.m_deviationThreshold6ForVBR      = 75;
    m_curbeData.DW19.m_deviationThreshold7ForVBR      = 90;
    m_curbeData.DW20.m_deviationThreshold0ForI        = MOS_BITFIELD_VALUE((uint32_t)-40, 8);
    m_curbeData.DW20.m_deviationThreshold1ForI        = MOS_BITFIELD_VALUE((uint32_t)-30, 8);
    m_curbeData.DW20.m_deviationThreshold2ForI        = MOS_BITFIELD_VALUE((uint32_t)-17, 8);
    m_curbeData.DW20.m_deviationThreshold3ForI        = MOS_BITFIELD_VALUE((uint32_t)-10, 8);
    m_curbeData.DW21.m_deviationThreshold4ForI        = 10;
    m_curbeData.DW21.m_deviationThreshold5ForI        = 20;
    m_curbeData.DW21.m_deviationThreshold6ForI        = 33;
    m_curbeData.DW21.m_deviationThreshold7ForI        = 45;
    m_curbeData.DW25.m_value                          = 1;
}

BrcUpdateCurbe::BrcUpdateCurbe()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_ZeroMemory(&m_curbeData, m_byteSize);

    m_curbeData.DW3.m_startGAdjFrame0             = 10;
    m_curbeData.DW3.m_startGAdjFrame1             = 50;
    m_curbeData.DW4.m_startGAdjFrame2             = 100;
    m_curbeData.DW4.m_startGAdjFrame3             = 150;
    m_curbeData.DW7.m_picHeaderDataBufferSize     = 0;
    m_curbeData.DW8.m_startGlobalAdjustMult0      = 1;
    m_curbeData.DW8.m_startGlobalAdjustMult1      = 1;
    m_curbeData.DW8.m_startGlobalAdjustMult2      = 3;
    m_curbeData.DW8.m_startGlobalAdjustMult3      = 2;
    m_curbeData.DW9.m_startGlobalAdjustMult4      = 1;
    m_curbeData.DW9.m_startGlobalAdjustDiv0       = 40;
    m_curbeData.DW9.m_startGlobalAdjustDiv1       = 5;
    m_curbeData.DW9.m_startGlobalAdjustDiv2       = 5;
    m_curbeData.DW10.m_startGlobalAdjustDiv3      = 3;
    m_curbeData.DW10.m_startGlobalAdjustDiv4      = 1;
    m_curbeData.DW10.m_qpThreshold0               = 7;
    m_curbeData.DW10.m_qpThreshold1               = 18;
    m_curbeData.DW11.m_qpThreshold2               = 25;
    m_curbeData.DW11.m_qpThreshold3               = 37;
    m_curbeData.DW11.m_gRateRatioThreshold0       = 40;
    m_curbeData.DW11.m_gRateRatioThreshold1       = 75;
    m_curbeData.DW12.m_gRateRatioThreshold2       = 97;
    m_curbeData.DW12.m_gRateRatioThreshold3       = 103;
    m_curbeData.DW12.m_gRateRatioThreshold4       = 125;
    m_curbeData.DW12.m_gRateRatioThreshold5       = 160;
    m_curbeData.DW13.m_gRateRatioThresholdQP0     = MOS_BITFIELD_VALUE((uint32_t)-3, 8);
    m_curbeData.DW13.m_gRateRatioThresholdQP1     = MOS_BITFIELD_VALUE((uint32_t)-2, 8);
    m_curbeData.DW13.m_gRateRatioThresholdQP2     = MOS_BITFIELD_VALUE((uint32_t)-1, 8);
    m_curbeData.DW13.m_gRateRatioThresholdQP3     = 0;
    m_curbeData.DW14.m_gRateRatioThresholdQP4     = 1;
    m_curbeData.DW14.m_gRateRatioThresholdQP5     = 2;
    m_curbeData.DW14.m_gRateRatioThresholdQP6     = 3;
    m_curbeData.DW14.m_forceToSkip                = 1;
    m_curbeData.DW15.m_value                      = 0;
    m_curbeData.DW16[0].m_value                   = 0x06040200;
    m_curbeData.DW16[1].m_value                   = 0x0e0c0a08;
    m_curbeData.DW16[2].m_value                   = 0x16141210;
    m_curbeData.DW16[3].m_value                   = 0x1e1c1a18;
    m_curbeData.DW16[4].m_value                   = 0x26242220;
    m_curbeData.DW16[5].m_value                   = 0x2e2c2a28;
    m_curbeData.DW16[6].m_value                   = 0x36343230;
    m_curbeData.DW16[7].m_value                   = 0x3e3c3a38;
    m_curbeData.DW16[8].m_value                   = 0x03020100;
    m_curbeData.DW16[9].m_value                   = 0x07060504;
    m_curbeData.DW16[10].m_value                   = 0x0e0c0a08;
    m_curbeData.DW16[11].m_value                   = 0x16141210;
    m_curbeData.DW16[12].m_value                   = 0x24201c18;
    m_curbeData.DW16[13].m_value                   = 0x34302c28;
    m_curbeData.DW16[14].m_value                   = 0x50484038;
    m_curbeData.DW16[15].m_value                   = 0x70686058;

    for (uint8_t idx = 0; idx < 10 ; idx++)
    {
        m_curbeData.DW32[idx].m_bindingTableIndex  = idx;
    }
}

const uint8_t CodechalEncodeMpeg2::m_qpAdjustmentDistThresholdMaxFrameThresholdI[] = {
    0x01,   0x02,   0x03,   0x04,   0x05,   0x01,   0x01,   0x02,   0x03,   0x04,
    0x00,   0x00,   0x01,   0x02,   0x03,   0x00,   0x00,   0x00,   0x01,   0x02,
    0xff,   0x00,   0x00,   0x00,   0x01,   0xfe,   0xfe,   0xff,   0x00,   0x00,
    0xfd,   0xfd,   0xff,   0xff,   0x00,   0xfc,   0xfd,   0xfe,   0xff,   0xff,
    0xfb,   0xfc,   0xfd,   0xfe,   0xff,   0x00,   0x04,   0x1e,   0x3c,   0x50,
    0x78,   0x8c,   0xc8,   0xff,   0x0a,   0x0b,   0x0c,   0x0c,   0x0d,   0x00,
    0x00,   0x00,   0x00,   0x00};

const uint8_t CodechalEncodeMpeg2::m_qpAdjustmentDistThresholdMaxFrameThresholdP[] = {
    0x01,   0x02,   0x03,   0x04,   0x05,   0x01,   0x01,   0x02,   0x03,   0x04,
    0x00,   0x01,   0x01,   0x02,   0x03,   0x00,   0x00,   0x00,   0x01,   0x02,
    0xff,   0x00,   0x00,   0x00,   0x01,   0xff,   0xff,   0xff,   0x00,   0x00,
    0xfe,   0xff,   0xff,   0xff,   0x00,   0xfc,   0xfe,   0xff,   0xff,   0x00,
    0xfc,   0xfd,   0xfe,   0xff,   0xff,   0x00,   0x04,   0x1e,   0x3c,   0x50,
    0x78,   0x8c,   0xc8,   0xff,   0x04,   0x05,   0x06,   0x06,   0x07,   0x00,
    0x00,   0x00,   0x00,   0x00 };

const uint8_t CodechalEncodeMpeg2::m_qpAdjustmentDistThresholdMaxFrameThresholdB[] = {
    0x01,   0x01,   0x02,   0x03,   0x04,   0x01,   0x01,   0x01,   0x02,   0x03,
    0x00,   0x00,   0x01,   0x01,   0x02,   0x00,   0x00,   0x00,   0x01,   0x01,
    0xff,   0x00,   0x00,   0x00,   0x00,   0xff,   0xff,   0xff,   0x00,   0x00,
    0xfe,   0xff,   0xff,   0xff,   0x00,   0xfd,   0xfe,   0xff,   0xff,   0x01,
    0xfc,   0xfd,   0xfe,   0xff,   0xff,   0x00,   0x02,   0x14,   0x28,   0x46,
    0x82,   0xa0,   0xc8,   0xff,   0x04,   0x05,   0x06,   0x06,   0x07,   0x00,
    0x00,   0x00,   0x00,   0x00 };

const uint8_t CodechalEncodeMpeg2::m_distQpAdjustmentI[] = {
    0x00,   0x01,   0x01,   0x02,   0x03,   0x03,   0x04,   0x05,   0x06,   0x00,
    0x00,   0x01,   0x01,   0x02,   0x02,   0x03,   0x04,   0x05,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x02,   0x02,   0x04,   0x05,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x02,   0x03,   0x04,   0xfd,   0xfe,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x02,   0x04,   0xfe,   0xfe,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x03,   0xfd,   0xfe,   0xff,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x02,   0xfc,   0xfd,   0xfd,   0xfe,   0xfe,   0xff,   0xff,
    0x00,   0x01,   0xfb,   0xfc,   0xfd,   0xfe,   0xfe,   0xff,   0xff,   0xff,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00};

const uint8_t CodechalEncodeMpeg2::m_distQpAdjustmentP[] = {
    0x00,   0x01,   0x01,   0x01,   0x02,   0x02,   0x03,   0x04,   0x05,   0x00,
    0x00,   0x01,   0x01,   0x01,   0x02,   0x02,   0x03,   0x04,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x01,   0x02,   0x02,   0x04,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x01,   0x01,   0x03,   0xfe,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x01,   0x03,   0xfe,   0xfe,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x01,   0x02,   0xfd,   0xfe,   0xff,   0xff,   0xff,   0x00,
    0x00,   0x00,   0x02,   0xfc,   0xfd,   0xfd,   0xfe,   0xfe,   0xff,   0xff,
    0x00,   0x01,   0xfb,   0xfc,   0xfd,   0xfe,   0xfe,   0xff,   0xff,   0xff,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00};

const uint8_t CodechalEncodeMpeg2::m_distQpAdjustmentB[] = {
    0x00,   0x01,   0x01,   0x01,   0x02,   0x02,   0x03,   0x04,   0x04,   0x00,
    0x00,   0x01,   0x01,   0x01,   0x02,   0x02,   0x03,   0x03,   0x00,   0x00,
    0x00,   0x00,   0x01,   0x01,   0x02,   0x02,   0x03,   0xff,   0x00,   0x00,
    0x00,   0x00,   0x01,   0x01,   0x01,   0x02,   0xfe,   0xff,   0x00,   0x00,
    0x00,   0x00,   0x01,   0x01,   0x02,   0xfe,   0xfe,   0xff,   0x00,   0x00,
    0x00,   0x00,   0x01,   0x01,   0xfd,   0xfe,   0xff,   0xff,   0x00,   0x00,
    0x00,   0x00,   0x01,   0xfc,   0xfd,   0xfd,   0xfe,   0xff,   0xff,   0x00,
    0x00,   0x00,   0xfb,   0xfc,   0xfd,   0xfe,   0xfe,   0xff,   0xff,   0xff,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,   0x00,
    0x00,   0x00,   0x00,   0x00,   0x00,   0x00};

const uint8_t CodechalEncodeMpeg2::m_targetUsageToKernelMode[] = {
    encodeNormalMode, encodeNormalMode,
    encodeNormalMode, encodeNormalMode,
    encodeNormalMode, encodeNormalMode,
    encodeNormalMode, encodeNormalMode };

const uint32_t CodechalEncodeMpeg2::m_vmeLutXyP[] = { 0x34262410, 0x46454436 };

const uint32_t CodechalEncodeMpeg2::m_vmeLutXyB[] = { 0x44363410, 0x56555446 };

const uint32_t CodechalEncodeMpeg2::m_vmeSPathP0[] = {
    0x1F11F10F, 0x2E22E2FE, 0x20E220DF, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x02F1F1F1, 0x1F201111,
    0xF1EFFF0C, 0xF01104F1, 0x10FF0A50, 0x000FF1C0, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeMpeg2::m_vmeSPathP1[] = {
    0x1F11F10F, 0x2E22E2FE, 0x20E220DF, 0xF1FB06FC, 0x0000D33F, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeMpeg2::m_vmeSPathB0[] = {
    0x120FF10F, 0x20E20F1F, 0x201EE2FD, 0x000D02D1, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t CodechalEncodeMpeg2::m_vmeSPathB1[] = {
    0x120FF10F, 0x20E20F1F, 0x0000E2FD, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

MOS_STATUS CodechalEncodeMpeg2::InitMmcState()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

#ifdef _MMC_SUPPORTED
    m_mmcState = MOS_New(CodechalMmcEncodeMpeg2, m_hwInterface, this);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
#endif
    return MOS_STATUS_SUCCESS;
}

CodechalEncodeMpeg2::CodechalEncodeMpeg2(
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
    auto renderInterface = m_hwInterface->GetRenderInterface();
    CODECHAL_ENCODE_ASSERT(renderInterface);
    m_stateHeapInterface = renderInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_ASSERT(m_stateHeapInterface);

    MOS_ZeroMemory(&m_picIdx, sizeof(m_picIdx));
    MOS_ZeroMemory(&m_refList, sizeof(m_refList));
    MOS_ZeroMemory(&m_4xMEMVDataBuffer, sizeof(m_4xMEMVDataBuffer));
    MOS_ZeroMemory(&m_batchBufForMEDistBuffer, sizeof(m_batchBufForMEDistBuffer));
    MOS_ZeroMemory(&m_mbEncBindingTable, sizeof(m_mbEncBindingTable));
    MOS_ZeroMemory(&m_4xMEDistortionBuffer, sizeof(m_4xMEDistortionBuffer));
    MOS_ZeroMemory(&m_brcBuffers, sizeof(m_brcBuffers));
    MOS_ZeroMemory(&m_mbQpDataSurface, sizeof(m_mbQpDataSurface));

    uint8_t i;
    for (i = 0; i < CODECHAL_ENCODE_BRC_IDX_NUM; i++)
    {
        m_brcKernelStates[i] = MHW_KERNEL_STATE();
    }
    for (i = 0; i < mbEncKernelIdxNum; i++)
    {
        m_mbEncKernelStates[i] = MHW_KERNEL_STATE();
    }

    m_interlacedFieldDisabled       = true;

    // Always true since interlaced field is no longer supported.
    m_firstField                    = true;
    m_hwWalker                      = true;
    m_fieldScalingOutputInterleaved = true;
    m_hmeSupported                  = true;
    m_kuid                          = IDR_CODEC_AllMPEG2Enc;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SINGLE_TASK_PHASE_ENABLE_ID,
        &userFeatureData);
    m_singleTaskPhaseSupported = (userFeatureData.i32Data) ? true : false;

    m_hwInterface->GetStateHeapSettings()->dwNumSyncTags = m_numSyncTags;
    m_hwInterface->GetStateHeapSettings()->dwDshSize     = m_initDshSize;

    m_useCmScalingKernel           = true;

}

CodechalEncodeMpeg2::~CodechalEncodeMpeg2()
{
    MOS_Delete(m_hmeKernel);
}

MOS_STATUS CodechalEncodeMpeg2::Initialize(CodechalSetting * codecHalSettings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::Initialize(codecHalSettings));

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_hwInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_miInterface);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_stateHeapInterface);

    m_frameNumB = 0;

    // Offset + Size of MB + size of MV
    m_mbCodeStrideInDW = 16;
    uint32_t fieldNumMBs = m_picWidthInMb * ((m_picHeightInMb + 1) >> 1);
    // 12 DW for MB + 4 DW for MV
    m_mbCodeSize = fieldNumMBs * 2 * 16 * sizeof(uint32_t);

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MPEG2_ENCODE_BRC_DISTORTION_BUFFER_ENABLE_ID,
        &userFeatureData);
    m_brcDistortionBufferSupported = (userFeatureData.i32Data) ? true : false;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MPEG2_SLICE_STATE_ENABLE_ID,
        &userFeatureData);
    m_sliceStateEnable = (userFeatureData.i32Data) ? true : false;

#endif
    // Initialize kernel State
    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitKernelState());

    if (m_singleTaskPhaseSupported)
    {
        m_maxBtCount = GetMaxBtCount();
    }

    // Picture Level Commands
    m_hwInterface->GetMfxStateCommandsDataSize(
        CODECHAL_ENCODE_MODE_MPEG2,
        &m_pictureStatesSize,
        &m_picturePatchListSize,
        0);

    // Slice Level Commands (cannot be placed in 2nd level batch)
    m_hwInterface->GetMfxPrimitiveCommandsDataSize(
        CODECHAL_ENCODE_MODE_MPEG2,
        &m_sliceStatesSize,
        &m_slicePatchListSize,
        0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitMmcState());

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::AllocateBuffer(
    PMOS_RESOURCE               buffer,
    uint32_t                    bufSize,
    PCCHAR                      name)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(buffer);

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    allocParams.Type = MOS_GFXRES_BUFFER;
    allocParams.TileType = MOS_TILE_LINEAR;
    allocParams.Format = Format_Buffer;
    allocParams.dwBytes = bufSize;
    allocParams.pBufName = name;

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &allocParams,
        buffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return eStatus;
    }

    CodechalResLock bufLock(m_osInterface, buffer);
    auto data = bufLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, bufSize);

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::AllocateBuffer2D(
    PMOS_SURFACE         surface,
    uint32_t             surfWidth,
    uint32_t             surfHeight,
    PCCHAR               name)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(surface);

    MOS_ZeroMemory(surface, sizeof(*surface));

    surface->TileType = MOS_TILE_LINEAR;
    surface->bArraySpacing = true;
    surface->Format = Format_Buffer_2D;
    surface->dwWidth = MOS_ALIGN_CEIL(surfWidth, 64);
    surface->dwHeight = surfHeight;
    surface->dwPitch = surface->dwWidth;

    MOS_ALLOC_GFXRES_PARAMS AllocParamsForBuffer2D;
    MOS_ZeroMemory(&AllocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
    AllocParamsForBuffer2D.Type = MOS_GFXRES_2D;
    AllocParamsForBuffer2D.TileType = surface->TileType;
    AllocParamsForBuffer2D.Format = surface->Format;
    AllocParamsForBuffer2D.dwWidth = surface->dwWidth;
    AllocParamsForBuffer2D.dwHeight = surface->dwHeight;
    AllocParamsForBuffer2D.pBufName = name;

    eStatus = (MOS_STATUS)m_osInterface->pfnAllocateResource(
        m_osInterface,
        &AllocParamsForBuffer2D,
        &surface->OsResource);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return eStatus;
    }

    CodechalResLock bufLock(m_osInterface, &surface->OsResource);
    auto data = bufLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, surface->dwWidth * surface->dwHeight);

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::AllocateBatchBuffer(
    PMHW_BATCH_BUFFER            batchBuffer,
    uint32_t                     bufSize,
    PCCHAR                       name)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_CHK_NULL_RETURN(batchBuffer);

    MOS_ZeroMemory(
        batchBuffer,
        sizeof(MHW_BATCH_BUFFER));

    batchBuffer->bSecondLevel = true;

    eStatus = Mhw_AllocateBb(
        m_osInterface,
        batchBuffer,
        nullptr,
        bufSize);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate %s.", name);
        return eStatus;
    }

     eStatus = Mhw_LockBb(m_osInterface, batchBuffer);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to lock %s.", name);
        return eStatus;
    }

    MOS_ZeroMemory(batchBuffer->pData, bufSize);

    eStatus = Mhw_UnlockBb(
        m_osInterface,
        batchBuffer,
        false);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to unlock  %s.", name);
        return eStatus;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::AllocateBrcResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // BRC history buffer
    uint32_t bufSize = m_brcHistoryBufferSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_brcBuffers.resBrcHistoryBuffer,
        bufSize,
        "BRC History Buffer"));

    // PAK Statistics buffer
    bufSize = m_brcPakStatisticsSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_brcBuffers.resBrcPakStatisticBuffer[0],
        bufSize,
        "BRC PAK Statistics Buffer"));

    // PAK IMG_STATEs buffer
    bufSize = BRC_IMG_STATE_SIZE_PER_PASS * m_mfxInterface->GetBrcNumPakPasses();
    for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
            &m_brcBuffers.resBrcImageStatesReadBuffer[i],
            bufSize,
            "PAK IMG State Read Buffer"));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_brcBuffers.resBrcImageStatesWriteBuffer,
        bufSize,
        "PAK IMG State Write Buffer"));

    // Picture header input and output buffers
    bufSize = m_brcPicHeaderSurfaceSize;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_brcBuffers.resBrcPicHeaderInputBuffer,
        bufSize,
        "Picture Header Input Buffer"));
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer(
        &m_brcBuffers.resBrcPicHeaderOutputBuffer,
        bufSize,
        "Picture Header Output Buffer"));

    uint32_t surfWidth = m_hwInterface->m_mpeg2BrcConstantSurfaceWidth;
    uint32_t surfHeight = m_hwInterface->m_mpeg2BrcConstantSurfaceHeight;
    for (uint8_t i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        //BRC Constant Data Surfaces
        CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
            &m_brcBuffers.sBrcConstantDataBuffer[i],
            surfWidth,
            surfHeight,
            "BRC Constant Data Buffer"));
    }

    // BRC Distortion Surface
    uint32_t downscaledFieldHeightInMB4x =
        (m_downscaledHeightInMb4x + 1) >> 1;
    surfWidth = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
    surfHeight = 2 * MOS_ALIGN_CEIL((downscaledFieldHeightInMB4x * 4), 8);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
        &m_brcBuffers.sMeBrcDistortionBuffer,
        surfWidth,
        surfHeight,
        "BRC Distortion Surface Buffer"));

    // VME batch buffer for distortion surface
    for (uint8_t i = 0; i < NUM_ENCODE_BB_TYPE; i++)
    {
        uint32_t currNumMBs;
        if (i == MB_ENC_Frame_BB)
        {
            currNumMBs = m_downscaledWidthInMb4x * m_downscaledHeightInMb4x;
        }
        else
        {
            currNumMBs = m_downscaledWidthInMb4x * downscaledFieldHeightInMB4x;
        }

        bufSize = m_hwInterface->GetMediaObjectBufferSize(
            currNumMBs,
            sizeof(MediaObjectInlineDataMpeg2));

        AllocateBatchBuffer(&m_batchBufForMEDistBuffer[i], bufSize, "ME Distortion Buffer");
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::AllocateEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t downscaledFieldHeightInMB4x = (m_downscaledHeightInMb4x + 1) >> 1;

    if (m_hmeSupported)
    {
        if (m_hmeKernel)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hmeKernel->AllocateResources());
        }
        else
        {
            uint32_t bufWidth = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64); // MediaBlockRW requires pitch multiple of 64 bytes when linear.
            uint32_t bufHeight = (m_downscaledHeightInMb4x * 2 * 4 * CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_4xMEMVDataBuffer,
                bufWidth,
                bufHeight,
                "4xME MV Data Buffer"));

            bufWidth = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
            bufHeight = 2 * MOS_ALIGN_CEIL((downscaledFieldHeightInMB4x * 4 * 10), 8);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateBuffer2D(
                &m_4xMEDistortionBuffer,
                bufWidth,
                bufHeight,
                "4xME Distortion Buffer"));
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::AllocateResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodechalEncoderState::AllocateResources());

    // Allocate Ref Lists
    CodecHalAllocateDataList(
        m_refList,
        CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate PAK resources.");
        return eStatus;
    }

    if (m_encEnabled)
    {
        eStatus = AllocateEncResources();
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate ENC resources.");
            return eStatus;
        }

        eStatus = AllocateBrcResources();
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            CODECHAL_ENCODE_ASSERTMESSAGE("Failed to allocate BRC resources.");
            return eStatus;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::FreeBrcResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcHistoryBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcHistoryBuffer);
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcPakStatisticBuffer[0]))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcPakStatisticBuffer[0]);
    }

    uint32_t i;
    for (i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcImageStatesReadBuffer[i]))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_brcBuffers.resBrcImageStatesReadBuffer[i]);
        }
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.resBrcImageStatesWriteBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcImageStatesWriteBuffer);
    }

    for (i = 0; i < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; i++)
    {
        if (!Mos_ResourceIsNull(&m_brcBuffers.sBrcConstantDataBuffer[i].OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_brcBuffers.sBrcConstantDataBuffer[i].OsResource);
        }
    }

    if (!Mos_ResourceIsNull(&m_brcBuffers.sMeBrcDistortionBuffer.OsResource))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.sMeBrcDistortionBuffer.OsResource);
    }

    if(!Mos_ResourceIsNull(&m_brcBuffers.resBrcPicHeaderInputBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcPicHeaderInputBuffer);
    }
    if(!Mos_ResourceIsNull(&m_brcBuffers.resBrcPicHeaderOutputBuffer))
    {
        m_osInterface->pfnFreeResource(
            m_osInterface,
            &m_brcBuffers.resBrcPicHeaderOutputBuffer);
    }

    for (i = 0; i < NUM_ENCODE_BB_TYPE; i++)
    {
        if (!Mos_ResourceIsNull(&m_batchBufForMEDistBuffer[i].OsResource))
        {
            Mhw_FreeBb(m_osInterface, &m_batchBufForMEDistBuffer[i], nullptr);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::FreeEncResources()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if (m_hmeSupported)
    {
        // 4xME ME MV data buffer
        if (!Mos_ResourceIsNull(&m_4xMEMVDataBuffer.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_4xMEMVDataBuffer.OsResource);
        }

        // 4xME distortion buffer
        if (!Mos_ResourceIsNull(&m_4xMEDistortionBuffer.OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_4xMEDistortionBuffer.OsResource);
        }
    }

    return eStatus;

}

void CodechalEncodeMpeg2::FreeResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    CodechalEncoderState::FreeResources();

    // Release Ref Lists
    CodecHalFreeDataList(m_refList, CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2);

    if (m_encEnabled)
    {
        FreeBrcResources();

        FreeEncResources();
    }
}

MOS_STATUS CodechalEncodeMpeg2::CheckProfileAndLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_INVALID_PARAMETER;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    switch(m_seqParams->m_profile)
    {
        case highProfile:
        case mainProfile:
        case simpleProfile:
            break;
        default:
            return eStatus;
            break;
    }

    switch(m_seqParams->m_level)
    {
        case levelHigh:
        case levelHigh1440:
        case levelMain:
        case levelLow:
            break;
        default:
            return eStatus;
            break;
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SetSequenceStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_oriFrameHeight = m_seqParams->m_frameHeight;
    m_oriFrameWidth = m_seqParams->m_frameWidth;
    if (m_seqParams->m_progressiveSequence)
    {
        m_picHeightInMb =
            (uint16_t)(CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_oriFrameHeight));
    }
    else
    {
        // For interlaced frame, align to 32 pixels.
        m_picHeightInMb =
            (uint16_t)((CODECHAL_GET_WIDTH_IN_BLOCKS(m_oriFrameHeight, (CODECHAL_MACROBLOCK_WIDTH << 1))) << 1);
    }
    m_picWidthInMb =
        (uint16_t)(CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_oriFrameWidth));
    m_frameWidth = m_picWidthInMb * CODECHAL_MACROBLOCK_WIDTH;
    m_frameHeight = m_picHeightInMb * CODECHAL_MACROBLOCK_HEIGHT;

    // HME Scaling WxH
    m_downscaledWidthInMb4x =
        CODECHAL_GET_WIDTH_IN_MACROBLOCKS(m_frameWidth / SCALE_FACTOR_4x);
    m_downscaledHeightInMb4x =
        CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(m_frameHeight / SCALE_FACTOR_4x);
    m_downscaledWidth4x =
        m_downscaledWidthInMb4x * CODECHAL_MACROBLOCK_WIDTH;
    m_downscaledHeight4x =
        m_downscaledHeightInMb4x * CODECHAL_MACROBLOCK_HEIGHT;

    MotionEstimationDisableCheck();

    m_targetUsage = m_seqParams->m_targetUsage & 0x7;
    m_kernelMode = m_targetUsageToKernelMode[m_targetUsage];

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CheckProfileAndLevel());

    m_brcEnabled = CodecHalIsRateControlBrc(m_seqParams->m_rateControlMethod, CODECHAL_MPEG2);

    // Mb Qp data is only enabled for CQP
    if (m_brcEnabled)
    {
        m_mbQpDataEnabled = false;
    }

    m_brcReset = m_seqParams->m_resetBRC;

    m_avbrAccuracy = 30;
    m_avbrConvergence = 150;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SetPictureStructs()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    if ((m_picParams->m_pictureCodingType < I_TYPE) ||
        (m_picParams->m_pictureCodingType > B_TYPE))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    if (Mos_ResourceIsNull(&m_reconSurface.OsResource) &&
        (!m_picParams->m_useRawPicForRef || m_pakEnabled))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // Sync initialize
    if ((m_firstFrame) ||
        (m_codecFunction == CODECHAL_FUNCTION_ENC) ||
        (!m_brcEnabled && m_picParams->m_useRawPicForRef) ||
        (!m_brcEnabled && (m_picParams->m_pictureCodingType == I_TYPE)))
    {
        m_waitForPak = false;
    }
    else
    {
        m_waitForPak = true;
    }

    if (m_codecFunction != CODECHAL_FUNCTION_ENC)
    {
        m_signalEnc = true;
    }
    else
    {
        m_signalEnc = false;
    }
    m_pictureCodingType = m_picParams->m_pictureCodingType;
    m_mbEncForcePictureCodingType = 0;

    // f_code checking.
    uint32_t fcodeX = CODECHAL_ENCODE_MPEG2_FCODE_X(m_frameWidth);
    uint32_t fcodeY = CODECHAL_ENCODE_MPEG2_FCODE_Y(fcodeX);

    if (m_pictureCodingType == I_TYPE)
    {
        if ((m_picParams->m_fcode00 > fcodeX) ||
            (m_picParams->m_fcode01 > fcodeY) ||
            (m_picParams->m_fcode00 == 0) ||
            (m_picParams->m_fcode01 == 0))
        {
            m_picParams->m_fcode00 = fcodeX;
            m_picParams->m_fcode01 = fcodeY;
        }
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        if ((m_picParams->m_fcode00 > fcodeX) ||
            (m_picParams->m_fcode01 > fcodeY) ||
            (m_picParams->m_fcode00 == 0) ||
            (m_picParams->m_fcode01 == 0))
        {
            m_picParams->m_fcode00 = fcodeX;
            m_picParams->m_fcode01 = fcodeY;
        }
    }
    else // B picture
    {
        if ((m_picParams->m_fcode00 > fcodeX) ||
            (m_picParams->m_fcode01 > fcodeY) ||
            (m_picParams->m_fcode10 > fcodeX) ||
            (m_picParams->m_fcode11 > fcodeY) ||
            (m_picParams->m_fcode00 == 0) ||
            (m_picParams->m_fcode01 == 0) ||
            (m_picParams->m_fcode10 == 0) ||
            (m_picParams->m_fcode11 == 0))
        {
            m_picParams->m_fcode00 = fcodeX;
            m_picParams->m_fcode01 = fcodeY;
            m_picParams->m_fcode10 = fcodeX;
            m_picParams->m_fcode11 = fcodeY;
        }
    }

    if (m_picParams->m_fieldCodingFlag == 0)
    {
        m_frameFieldHeight = m_frameHeight;
        m_frameFieldHeightInMb = m_picHeightInMb;
        m_downscaledFrameFieldHeightInMb4x = m_downscaledHeightInMb4x;
    }
    else
    {
        m_frameFieldHeight = ((m_frameHeight + 1) >> 1);
        m_frameFieldHeightInMb = ((m_picHeightInMb + 1) >> 1);
        m_downscaledFrameFieldHeightInMb4x = ((m_downscaledHeightInMb4x + 1) >> 1);
    }

    m_statusReportFeedbackNumber = m_picParams->m_statusReportFeedbackNumber;
    m_lastPicInStream = m_picParams->m_lastPicInStream;
    m_currOriginalPic = m_picParams->m_currOriginalPic;
    m_currReconstructedPic = m_picParams->m_currReconstructedPic;

    uint8_t currRefIdx = m_picParams->m_currReconstructedPic.FrameIdx;

    m_refList[currRefIdx]->sRefRawBuffer = m_rawSurface;
    m_refList[currRefIdx]->sRefReconBuffer = m_reconSurface;
    m_refList[currRefIdx]->resBitstreamBuffer = m_resBitstreamBuffer;

    if (m_pictureCodingType == I_TYPE)
    {
        m_picIdx[0].bValid = m_picIdx[1].bValid = 0;
        m_refList[currRefIdx]->bUsedAsRef = true;
        m_refList[currRefIdx]->ucNumRef = 0;
    }
    else if (m_pictureCodingType == P_TYPE)
    {
        if (m_picParams->m_refFrameList[0].PicFlags != PICTURE_INVALID)
        {
            m_picIdx[0].bValid = 1;
            m_picIdx[0].ucPicIdx = m_picParams->m_refFrameList[0].FrameIdx;
        }
        m_picIdx[1].bValid = 0;
        m_refList[currRefIdx]->bUsedAsRef = true;
        m_refList[currRefIdx]->RefList[0] = m_picParams->m_refFrameList[0];
        m_refList[currRefIdx]->ucNumRef = 1;
    }
    else// B_TYPE
    {
        if (m_picParams->m_refFrameList[0].PicFlags != PICTURE_INVALID)
        {
            m_picIdx[0].bValid = 1;
            m_picIdx[0].ucPicIdx = m_picParams->m_refFrameList[0].FrameIdx;
        }

        if (m_picParams->m_refFrameList[1].PicFlags != PICTURE_INVALID)
        {
            m_picIdx[1].bValid = 1;
            m_picIdx[1].ucPicIdx = m_picParams->m_refFrameList[1].FrameIdx;
        }
        m_refList[currRefIdx]->bUsedAsRef = false;
    }
    m_currRefList = m_refList[currRefIdx];

    if (m_codecFunction == CODECHAL_FUNCTION_ENC)
    {
        CODECHAL_ENCODE_CHK_NULL_RETURN(m_encodeParams.presMbCodeSurface);
        m_resMbCodeSurface = *(m_encodeParams.presMbCodeSurface);
    }
    else if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
    {
        // the actual MbCode/MvData surface to be allocated later
        m_trackedBuf->SetAllocationFlag(true);
    }

    m_hmeEnabled = m_hmeSupported && m_pictureCodingType != I_TYPE;

    if (m_brcEnabled)
    {
        m_numPasses = (uint8_t)(m_mfxInterface->GetBrcNumPakPasses() - 1);  // 1 original plus extra to handle BRC
    }

    // if GOP structure is I-frame only, we use 3 non-ref slots for tracked buffer
    m_gopIsIdrFrameOnly = (m_picParams->m_gopPicSize == 1 && m_picParams->m_gopRefDist == 0);

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SetSliceGroups()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    uint32_t mbCount = 0;
    auto bsBuffer = &m_bsBuffer;
    auto slcParams = m_sliceParams;
    auto slcData = m_slcData;
    PCODEC_ENCODER_SLCDATA slcDataPrevStart = nullptr;

    for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        // Slice width should equal picture width, MBs should be on same row
        CODECHAL_ENCODE_CHK_NULL_RETURN(slcData);
        CODECHAL_ENCODE_CHK_NULL_RETURN(slcParams);
        CODECHAL_ENCODE_ASSERT((slcParams->m_numMbsForSlice % m_picWidthInMb) == 0);
        CODECHAL_ENCODE_ASSERT(slcParams->m_numMbsForSlice <= m_picWidthInMb);

        if ((slcParams->m_quantiserScaleCode < 1) ||
            (slcParams->m_quantiserScaleCode > 31))
        {
            slcParams->m_quantiserScaleCode = 1;
        }

        // Determine slice groups
        if (slcCount == 0)
        {
            // First slice
            slcDataPrevStart = slcData;
            slcData->SliceGroup |= SLICE_GROUP_START;

            if (m_codecFunction == (CODECHAL_FUNCTION_ENC | CODECHAL_FUNCTION_PAK))
            {
                slcData->SliceOffset = bsBuffer->SliceOffset;
                // Make slice header uint8_t aligned, all start codes are uint8_t aligned
                while (bsBuffer->BitOffset)
                {
                    PutBit(bsBuffer, 0);
                }
                for (uint32_t i = 0; i < 8; i++)
                {
                    PutBit(bsBuffer, 0);
                }

                slcData->BitSize = bsBuffer->BitSize =
                    (uint32_t)((bsBuffer->pCurrent - bsBuffer->SliceOffset - bsBuffer->pBase) * 8 + bsBuffer->BitOffset);
                bsBuffer->SliceOffset =
                    (uint32_t)(bsBuffer->pCurrent - bsBuffer->pBase + (bsBuffer->BitOffset != 0)); // start at next byte
            }
            else
            {
                slcData->SliceOffset = bsBuffer->SliceOffset;
                slcData->BitSize = bsBuffer->BitSize;
            }
        }
        else
        {
            // Compare with prev slice to see if curr slice is start of new slice group
            PCODEC_ENCODER_SLCDATA slcDataPrev = slcData - 1;
            CodecEncodeMpeg2SliceParmas *slcParamsPrev = slcParams - 1;

            if (!slcDataPrev || !slcParamsPrev)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                CODECHAL_ENCODE_ASSERTMESSAGE("Invalid slice pointer.");
                return eStatus;
            }

            // Start of a new slice group if gap in slices or quantiser_scale_code/IntraSlice changes
            uint32_t mbPrevEnd =
                (slcParamsPrev->m_firstMbY * m_picWidthInMb) +
                slcParamsPrev->m_firstMbX +
                slcParamsPrev->m_numMbsForSlice;
            uint32_t mbCurrStart = (slcParams->m_firstMbY * m_picWidthInMb) + slcParams->m_firstMbX;

            if ((mbPrevEnd != mbCurrStart) ||
                (slcParamsPrev->m_quantiserScaleCode != slcParams->m_quantiserScaleCode) ||
                (slcParamsPrev->m_intraSlice != slcParams->m_intraSlice))
            {
                slcDataPrev->SliceGroup |= SLICE_GROUP_END;
                slcData->SliceGroup |= SLICE_GROUP_START;

                slcDataPrevStart->NextSgMbXCnt = slcParams->m_firstMbX;
                slcDataPrevStart->NextSgMbYCnt = slcParams->m_firstMbY;
                slcDataPrevStart = slcData;

                slcData->SliceOffset = bsBuffer->SliceOffset;
                // Make slice header uint8_t aligned, all start codes are uint8_t aligned
                while (bsBuffer->BitOffset)
                {
                    PutBit(bsBuffer, 0);
                }
                for (uint32_t i = 0; i < 8; i++)
                {
                    PutBit(bsBuffer, 0);
                }

                slcData->BitSize = bsBuffer->BitSize =
                    (uint32_t)((bsBuffer->pCurrent - bsBuffer->SliceOffset - bsBuffer->pBase) * 8 + bsBuffer->BitOffset);
                bsBuffer->SliceOffset =
                    (uint32_t)(bsBuffer->pCurrent - bsBuffer->pBase + (bsBuffer->BitOffset != 0)); // start at next byte
            }
        }

        if (slcCount == (m_numSlices - 1))
        {
            // Last slice
            slcData->SliceGroup |= SLICE_GROUP_END;
            slcDataPrevStart->SliceGroup |= SLICE_GROUP_LAST;
            slcDataPrevStart->NextSgMbXCnt = 0;
            slcDataPrevStart->NextSgMbYCnt = m_frameFieldHeightInMb;
        }

        slcData->CmdOffset = mbCount * m_mbCodeStrideInDW * sizeof(uint32_t);

        mbCount += slcParams->m_numMbsForSlice;
        slcParams++;
        slcData++;
    }

    return eStatus;

}

uint32_t CodechalEncodeMpeg2::GetCurByteOffset(BSBuffer* bsBuffer)
{
    return (uint32_t)(bsBuffer->pCurrent - bsBuffer->pBase);
}

MOS_STATUS CodechalEncodeMpeg2::PackDisplaySeqExtension()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    // Make start code uint8_t aligned
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }

    // extension_start_code
    PutBits(bsBuffer, startCodePrefix, 24);
    PutBits(bsBuffer, startCodeExtension, 8);

    // extension_start_code_identifier
    PutBits(bsBuffer, Mpeg2sequenceDisplayExtension, 4);

    // video_format
    PutBits(bsBuffer, m_vuiParams->m_videoFormat, 3);

    // colour_description
    PutBit(bsBuffer, m_vuiParams->m_colourDescription);

    if (m_vuiParams->m_colourDescription)
    {
        // colour_primaries
        PutBits(bsBuffer, m_vuiParams->m_colourPrimaries, 8);

        // transfer_characteristics
        PutBits(bsBuffer, m_vuiParams->m_transferCharacteristics, 8);

        // matrix_coefficients
        PutBits(bsBuffer, m_vuiParams->m_matrixCoefficients, 8);
    }

    // display_horizontal_size
    PutBits(bsBuffer, m_vuiParams->m_displayHorizontalSize, 14);

    // marker_bit
    PutBit(bsBuffer, 1);

    // display_vertical_size
    PutBits(bsBuffer, m_vuiParams->m_displayVerticalSize, 14);

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::PackSeqExtension()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    // Make start code uint8_t aligned
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }

    // extension_start_code
    PutBits(bsBuffer, startCodePrefix, 24);
    PutBits(bsBuffer, startCodeExtension, 8);

    // extension_start_code_identifier
    PutBits(bsBuffer, Mpeg2sequenceExtension, 4);

    // profile_and_level_indication
    PutBits(bsBuffer, ((m_seqParams->m_profile & 0x70) | (m_seqParams->m_level & 0xF)), 8);

    // progressive_sequence
    PutBit(bsBuffer, m_seqParams->m_progressiveSequence);

    // chroma_format
    PutBits(bsBuffer, m_seqParams->m_chromaFormat, 2);

    // horizontal_size_extension
    PutBits(bsBuffer, ((m_seqParams->m_frameWidth >> 12) & 0x3), 2);

    // vertical_size_extension
    PutBits(bsBuffer, ((m_seqParams->m_frameHeight >> 12) & 0x3), 2);

    // bit_rate_extension
    PutBits(bsBuffer, ((MOS_ROUNDUP_DIVIDE(m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS, 400)) >> 18) & 0xFFF, 12);

    // marker_bit
    PutBit(bsBuffer, 1);

    // vbv_buffer_size_extension 8 uimsbf
    PutBits(bsBuffer, ((m_seqParams->m_vbvBufferSize >> 10) & 0xFF), 8);

    // low_delay
    PutBit(bsBuffer, m_seqParams->m_lowDelay);

    // frame_rate_extension_n
    PutBits(bsBuffer, m_seqParams->m_frameRateExtN, 2);

    // frame_rate_extension_d
    PutBits(bsBuffer, m_seqParams->m_frameRateExtD, 5);

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::PackSeqHeader()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    // Make start code uint8_t aligned
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }

    // sequence_start_code
    PutBits(bsBuffer, startCodePrefix, 24);
    PutBits(bsBuffer, startCodeSequenceHeader, 8);

    // horizontal_size_value
    CODECHAL_ENCODE_ASSERT((m_seqParams->m_frameWidth & 0xFFF) != 0); // Avoid start code emulation
    PutBits(bsBuffer, (m_seqParams->m_frameWidth & 0xFFF), 12);

    // vertical_size_value
    CODECHAL_ENCODE_ASSERT((m_seqParams->m_frameHeight & 0xFFF) != 0); // Avoid start code emulation
    PutBits(bsBuffer, (m_seqParams->m_frameHeight & 0xFFF), 12);

    // aspect_ratio_information
    CODECHAL_ENCODE_ASSERT((m_seqParams->m_aspectRatio > 0) && (m_seqParams->m_aspectRatio < 5));
    PutBits(bsBuffer, m_seqParams->m_aspectRatio, 4);

    // frame_rate_code
    CODECHAL_ENCODE_ASSERT((m_seqParams->m_frameRateCode > 0) & (m_seqParams->m_frameRateCode < 15));
    PutBits(bsBuffer, m_seqParams->m_frameRateCode, 4);

    // bit_rate_value
    if (m_seqParams->m_rateControlMethod == RATECONTROL_VBR)
    {
        // In Architecture prototype, the bit_rate_value of sequence header is set to m_maxBitRate not the target bit-rate for VBR case.
        PutBits(bsBuffer, ((MOS_ROUNDUP_DIVIDE(m_seqParams->m_maxBitRate * CODECHAL_ENCODE_BRC_KBPS, 400)) & 0x3FFFF), 18);
    }
    else
    {
        PutBits(bsBuffer, ((MOS_ROUNDUP_DIVIDE(m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS, 400)) & 0x3FFFF), 18);
    }

    // marker_bit
    PutBit(bsBuffer, 1);

    // vbv_buffer_size_value
    PutBits(bsBuffer, (m_seqParams->m_vbvBufferSize & 0x3FF), 10);

    // constrained_parameters_flag
    PutBit(bsBuffer, 0);

    // m_loadIntraQuantiserMatrix
    PutBit(bsBuffer, m_qMatrixParams->m_newQmatrix[0]);
    if (m_qMatrixParams->m_newQmatrix[0])
    {
        // m_intraQuantiserMatrix[64]
        for (uint8_t i = 0; i < 64; i++)
        {
            // Already in zig-zag scan order
            PutBits(bsBuffer, m_qMatrixParams->m_qmatrix[0][i], 8);
        }
    }

    // m_loadNonIntraQuantiserMatrix
    PutBit(bsBuffer, m_qMatrixParams->m_newQmatrix[1]);
    if (m_qMatrixParams->m_newQmatrix[1])
    {
        // m_nonIntraQuantiserMatrix[64]
        for (uint8_t i = 0; i < 64; i++)
        {
            // Already in zig-zag scan order
            PutBits(bsBuffer, m_qMatrixParams->m_qmatrix[1][i], 8);
        }
    }

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::PackSequenceParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // picture header
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackSeqHeader());

    // picture coding extension
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackSeqExtension());

    // optional sequence display extension (& user data)
    if (m_newVuiData)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PackDisplaySeqExtension());

        m_newVuiData = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::PackPicCodingExtension()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    // All start codes are uint8_t aligned
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }

    // extension_start_code
    PutBits(bsBuffer, startCodePrefix, 24);
    PutBits(bsBuffer, startCodeExtension, 8);

    // extension_start_code_identifier
    PutBits(bsBuffer, Mpeg2pictureCodingExtension, 4);

    // f_codes values 1-9 or 15; 0 or 1-14 are reserved
    if ((m_picParams->m_pictureCodingType == I_TYPE) && !m_picParams->m_concealmentMotionVectors)
    {
        // f_code[0][0], forward horizontal
        PutBits(bsBuffer, 0xF, 4);
        // f_code[0][1], forward vertical
        PutBits(bsBuffer, 0xF, 4);
        // f_code[1][0], backward horizontal
        PutBits(bsBuffer, 0xF, 4);
        // f_code[1][1], backward vertical
        PutBits(bsBuffer, 0xF, 4);
    }
    else
    {
        // f_code[0][0], forward horizontal
        PutBits(bsBuffer, m_picParams->m_fcode00, 4);
        // f_code[0][1], forward vertical
        PutBits(bsBuffer, m_picParams->m_fcode01, 4);

        if ((m_picParams->m_pictureCodingType == I_TYPE) || (m_picParams->m_pictureCodingType == P_TYPE))
        {
            // f_code[1][0], backward horizontal
            PutBits(bsBuffer, 0xF, 4);
            // f_code[1][1], backward vertical
            PutBits(bsBuffer, 0xF, 4);
        }
        else
        {
            // f_code[1][0], backward horizontal
            PutBits(bsBuffer, m_picParams->m_fcode10, 4);
            // f_code[1][1], backward vertical
            PutBits(bsBuffer, m_picParams->m_fcode11, 4);
        }
    }

    // store byte offset of intra_dc_precision
    m_intraDcPrecisionOffset = GetCurByteOffset(bsBuffer);
    // intra_dc_precision
    PutBits(bsBuffer, m_picParams->m_intraDCprecision, 2);

    // picture_structure
    PutBits(bsBuffer, (!m_picParams->m_fieldCodingFlag) ? 3 : ((m_picParams->m_interleavedFieldBFF) ? 2 : 1), 2);

    bool progressiveSequence = m_seqParams->m_progressiveSequence & 0x1;
    bool actual_tff = (!m_picParams->m_fieldCodingFlag && !progressiveSequence) || (m_picParams->m_repeatFirstField != 0);

    // top_field_first
    PutBit(bsBuffer, (actual_tff ) ? (!m_picParams->m_interleavedFieldBFF) : 0);
    bool progressive = true;
    if (m_picParams->m_fieldCodingFlag || m_picParams->m_fieldFrameCodingFlag)
    {
        progressive = false;
    }

    // frame_pred_frame_dct
    if (progressive)
    {
        PutBit(bsBuffer, 1);
    }
    else if (m_picParams->m_fieldCodingFlag)
    {
        PutBit(bsBuffer, 0);
    }
    else
    {
        PutBit(bsBuffer, m_picParams->m_framePredFrameDCT);
    }

    // concealment_motion_vectors
    PutBit(bsBuffer, m_picParams->m_concealmentMotionVectors);

    // Store the byte offset of the q_scale_type
    m_qScaleTypeByteOffse = GetCurByteOffset(bsBuffer);
    // q_scale_type
    PutBit(bsBuffer, m_picParams->m_qscaleType);

    // intra_vlc_format
    PutBit(bsBuffer, m_picParams->m_intraVlcFormat);

    // alternate_scan
    PutBit(bsBuffer, m_picParams->m_alternateScan);

    // repeat_first_field
    PutBit(bsBuffer, (!m_picParams->m_fieldCodingFlag) ? m_picParams->m_repeatFirstField : 0);

    // chroma_420_type
    PutBit(bsBuffer, progressive);

    // progressive_frame
    PutBit(bsBuffer, progressive);

    // composite_display_flag
    PutBit(bsBuffer, m_picParams->m_compositeDisplayFlag);

    if (m_picParams->m_compositeDisplayFlag)
    {
        // v_axis
        PutBit(bsBuffer, m_picParams->m_vaxis);
        // field_sequence
        PutBits(bsBuffer, m_picParams->m_fieldSequence, 3);
        // sub_carrier
        PutBit(bsBuffer, m_picParams->m_subCarrier);
        // burst_amplitude
        PutBits(bsBuffer, m_picParams->m_burstAmplitude, 7);
        // sub_carrier_phase
        PutBits(bsBuffer, m_picParams->m_subCarrierPhase, 8);
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::PackPicUserData()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto userDataListHead = (CodecEncodeMpeg2UserDataList *)m_encodeParams.pMpeg2UserDataListHead;
    CODECHAL_ENCODE_CHK_NULL_RETURN(userDataListHead);

    auto bsBuffer = &m_bsBuffer;

    for (auto p = userDataListHead; p; p = p->m_nextItem)
    {
        auto userData = (uint8_t*)p->m_userData;

        while (bsBuffer->BitOffset)
        {
            PutBit(bsBuffer, 0);
        }

        for(unsigned int i = 0; i < p->m_userDataSize; ++i)
        {
            PutBits(bsBuffer, (uint32_t) (userData[i]), 8);
        }
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::PackPicHeader()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    // All start codes are uint8_t aligned
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }

    // picture_start_code
    PutBits(bsBuffer, startCodePrefix, 24);
    PutBits(bsBuffer, startCodePicture, 8);

    // temporal_reference
    PutBits(bsBuffer, m_picParams->m_temporalReference, 10);

    // picture_coding_type
    PutBits(bsBuffer, m_picParams->m_pictureCodingType, 3);

    // Store the byte offset of the q_scale_type
    m_vbvDelayOffset = GetCurByteOffset(bsBuffer);
    // vbv_delay
    PutBits(bsBuffer, m_picParams->m_vbvDelay, 16);

    if ((m_picParams->m_pictureCodingType == P_TYPE) || (m_picParams->m_pictureCodingType == B_TYPE))
    {
        // full_pel_forward_vector, '0'
        PutBit(bsBuffer, 0);
        // forward_f_code,  '111'
        PutBits(bsBuffer, 0x7, 3);
    }

    if (m_picParams->m_pictureCodingType == B_TYPE)
    {
        // full_pel_backward_vector, '0'
        PutBit(bsBuffer, 0);
        // backward_f_code '111'
        PutBits(bsBuffer, 0x7, 3);
    }

    // extra_bit_picture, '0'
    PutBit(bsBuffer, 0);

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::PackGroupOfPicHeader()
{
    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    // All start codes are uint8_t aligned
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }

    // group_start_code
    PutBits(bsBuffer, startCodePrefix, 24);
    PutBits(bsBuffer, startCodeGroupStart, 8);

    // time_code, 25 bits total
    // drop_flag
    PutBit(bsBuffer, ((m_picParams->m_timeCode >> 24) & 1));
    // hour
    PutBits(bsBuffer, ((m_picParams->m_timeCode >> 19) & 0x1F), 5);
    // minute
    PutBits(bsBuffer, ((m_picParams->m_timeCode >> 13) & 0x3F), 6);
    // marker_bit
    PutBit(bsBuffer, 1);
    // sec
    PutBits(bsBuffer, ((m_picParams->m_timeCode >> 6) & 0x3F), 6);
    // frame
    PutBits(bsBuffer, ((m_picParams->m_timeCode) & 0x3F), 6);

    // closed_gop
    PutBit(bsBuffer, m_picParams->m_gopOptFlag & 1);
    // broken_link, used in editing
    PutBit(bsBuffer, 0);

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::PackPictureParams()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    // optional GOP header (& user data)
    if (m_picParams->m_newGop)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PackGroupOfPicHeader());
    }

    // picture header
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackPicHeader());

    // picture coding extension
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackPicCodingExtension());

    // user data
    if(m_encodeParams.pMpeg2UserDataListHead)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PackPicUserData());
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::PackPictureHeader()
{
    MOS_STATUS  eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;

    *(bsBuffer->pBase) = 0; // init first byte to 0
    bsBuffer->pCurrent = bsBuffer->pBase;
    bsBuffer->SliceOffset = 0;
    bsBuffer->BitOffset = 0;
    bsBuffer->BitSize = 0;

    // If this is a new sequence, write the seq set
    if (m_newSeq)
    {
        // Pack SPS
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PackSequenceParams());
    }

    // Pack PPS
    CODECHAL_ENCODE_CHK_STATUS_RETURN(PackPictureParams());

    // HW will insert next slice start code, but need to byte align for HW
    while (bsBuffer->BitOffset)
    {
        PutBit(bsBuffer, 0);
    }
    bsBuffer->BitSize = (uint32_t)(bsBuffer->pCurrent - bsBuffer->SliceOffset - bsBuffer->pBase) * 8 + bsBuffer->BitOffset;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::InitializePicture(const EncoderParams& params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    m_seqParams = (CodecEncodeMpeg2SequenceParams *)(params.pSeqParams);
    m_vuiParams = (CodecEncodeMpeg2VuiParams *)(params.pVuiParams);
    m_picParams = (CodecEncodeMpeg2PictureParams *)(params.pPicParams);
    m_sliceParams = (CodecEncodeMpeg2SliceParmas *)(params.pSliceParams);
    m_qMatrixParams = (CodecEncodeMpeg2QmatixParams *)(params.pIQMatrixBuffer);

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_seqParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_vuiParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_picParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_sliceParams);
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_qMatrixParams);

    // Mb Qp data
    m_mbQpDataEnabled = params.bMbQpDataEnabled;
    if (m_mbQpDataEnabled)
    {
        m_mbQpDataSurface = *(params.psMbQpDataSurface);
    }
    m_skipFrameFlag = m_picParams->m_skipFrameFlag;

    m_verticalLineStride = CODECHAL_VLINESTRIDE_FRAME;
    m_verticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_TOP_FIELD;
    m_mbcodeBottomFieldOffset = 0;
    m_mvBottomFieldOffset = 0;
    m_scaledBottomFieldOffset = 0;
    m_scaled16xBottomFieldOffset = 0;

    if (m_newSeq)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSequenceStructs());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetPictureStructs());

    // 4x downscaled surface needed by MbEnc IDist (for BRC) kernel or HME kernel
    m_scalingEnabled = (m_hmeSupported || m_brcEnabled);

    if (CodecHal_PictureIsField(m_currOriginalPic))
    {
        m_verticalLineStride = CODECHAL_VLINESTRIDE_FIELD;
        m_frameHeight = m_frameFieldHeightInMb * 2 * 16;
        m_picHeightInMb = (uint16_t)(m_frameHeight / 16);
        if (CodecHal_PictureIsBottomField(m_currOriginalPic))
        {
            m_verticalLineStrideOffset = CODECHAL_VLINESTRIDEOFFSET_BOT_FIELD;
            m_mbcodeBottomFieldOffset = m_frameFieldHeightInMb * m_picWidthInMb * 64;
            m_mvBottomFieldOffset = MOS_ALIGN_CEIL(m_frameFieldHeightInMb * m_picWidthInMb * (32 * 4), 0x1000);
        }
    }

    if (m_pictureCodingType == B_TYPE)
    {
        m_frameNumB += 1;
    }
    else
    {
        m_frameNumB = 0;
    }

    if (m_pakEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(PackPictureHeader());

        if (m_brcEnabled)
        {
            MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
            uint32_t dwPicHeaderDataStartOffset,dwPicHeaderDataBufferSize;

            MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
            pakInsertObjectParams.pBsBuffer = &m_bsBuffer;
            pakInsertObjectParams.pdwMpeg2PicHeaderDataStartOffset = &dwPicHeaderDataStartOffset;
            pakInsertObjectParams.pdwMpeg2PicHeaderTotalBufferSize = &dwPicHeaderDataBufferSize;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfcMpeg2PakInsertBrcBuffer(
                &m_brcBuffers.resBrcPicHeaderInputBuffer,
                &pakInsertObjectParams));

            // The q_scale_type offset is relative to the beginning of the picture header buffer.
            // Since it starts off with the INSERT command, include its size in the offset for the
            // q_scale_type. Do the same for the vbv_delay offset.
            m_picHeaderDataBufferSize = dwPicHeaderDataBufferSize;
            m_qScaleTypeByteOffse += dwPicHeaderDataStartOffset;
            m_vbvDelayOffset += dwPicHeaderDataStartOffset;
            m_intraDcPrecisionOffset += dwPicHeaderDataStartOffset;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetSliceGroups());
    }

    CODECHAL_DEBUG_TOOL(
        m_debugInterface->m_currPic            = m_picParams->m_currOriginalPic;
        m_debugInterface->m_bufferDumpFrameNum = m_storeData;
        m_debugInterface->m_frameType          = m_pictureCodingType;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpVuiParams(
            m_vuiParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpPicParams(
            m_picParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSeqParams(
            m_seqParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(DumpSliceParams(
            m_sliceParams));)

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetStatusReportParams(
        m_refList[m_currReconstructedPic.FrameIdx]));

    m_bitstreamUpperBound = m_encodeParams.dwBitstreamSize;

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::InitKernelStateBrc()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;
    uint32_t brcBtCount[CODECHAL_ENCODE_BRC_IDX_NUM] = {
        brcInitResetNumBindingTableEntries,
        brcUpdateNumBindingTableEntries,
        brcInitResetNumBindingTableEntries,
        0,                                                                  // IFrameDist uses MBEnc I kernel
        0,                                                                  // BlockCopy kernel is not needed
        0                                                                   // MbBRCUpdate kernel is not needed
    };

    uint32_t brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM] = {
        BrcInitResetCurbe::m_byteSize,
        BrcUpdateCurbe::m_byteSize,
        BrcInitResetCurbe::m_byteSize,
        0,                                                                // IFrameDist uses MBEnc I kernel
        0,                                                                // BlockCopy kernel is not needed
        0                                                                 // MbBRCUpdate kernel is not needed
    };

    CODECHAL_KERNEL_HEADER currKrnHeader;
    // CODECHAL_ENCODE_BRC_IDX_NUM - 2: BlockCopy and MbBRCUpdate kernel not needed
    for (uint8_t krnStateIdx = 0; krnStateIdx < CODECHAL_ENCODE_BRC_IDX_NUM - 2; krnStateIdx++)
    {
        // IFrameDist doesn't have separate kernel for MPEG2, needs to use MbEnc I kernel
        if (krnStateIdx == CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST)
        {
            m_brcKernelStates[krnStateIdx] = m_mbEncKernelStates[mbEncKernelIdxI];
            continue;
        }

        auto kernelState = &m_brcKernelStates[krnStateIdx];
        uint32_t kernelSize = m_combinedKernelSize;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(pfnGetKernelHeaderAndSize(
            m_kernelBinary,
            ENC_BRC,
            krnStateIdx,
            &currKrnHeader,
            &kernelSize));

        kernelState->KernelParams.iBTCount = brcBtCount[krnStateIdx];
        kernelState->KernelParams.iThreadCount = m_hwInterface->GetRenderInterface()->GetHwCaps()->dwMaxThreads;
        kernelState->KernelParams.iCurbeLength = brcCurbeSize[krnStateIdx];
        kernelState->KernelParams.iBlockWidth = CODECHAL_MACROBLOCK_WIDTH;
        kernelState->KernelParams.iBlockHeight = CODECHAL_MACROBLOCK_HEIGHT;
        kernelState->KernelParams.iIdCount = 1;

        kernelState->dwCurbeOffset = m_stateHeapInterface->pStateHeapInterface->GetSizeofCmdInterfaceDescriptorData();
        kernelState->KernelParams.pBinary = m_kernelBinary + (currKrnHeader.KernelStartPointer << MHW_KERNEL_OFFSET_SHIFT);
        kernelState->KernelParams.iSize = kernelSize;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnCalculateSshAndBtSizesRequested(
            m_stateHeapInterface,
            kernelState->KernelParams.iBTCount,
            &kernelState->dwSshSize,
            &kernelState->dwBindingTableSize));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->MhwInitISH(m_stateHeapInterface, kernelState));
    }

    return eStatus;
}

uint32_t CodechalEncodeMpeg2::GetMaxBtCount()
{
    uint32_t scalingBtCount = MOS_ALIGN_CEIL(
        m_scaling4xKernelStates[0].KernelParams.iBTCount,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    uint32_t meBtCount = MOS_ALIGN_CEIL(
        m_hmeKernel ? m_hmeKernel->GetBTCount() : m_meKernelStates[0].KernelParams.iBTCount,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    uint32_t mbEncBtCount = MOS_ALIGN_CEIL(
        m_mbEncKernelStates[0].KernelParams.iBTCount,
        m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());

    uint32_t brcBtCount = 0;
    for (uint32_t i = 0; i < CODECHAL_ENCODE_BRC_IDX_NUM; i++)
    {
        brcBtCount += MOS_ALIGN_CEIL(
            m_brcKernelStates[i].KernelParams.iBTCount,
            m_stateHeapInterface->pStateHeapInterface->GetBtIdxAlignment());
    }

   return MOS_MAX(scalingBtCount + meBtCount, mbEncBtCount + brcBtCount);
}

MOS_STATUS CodechalEncodeMpeg2::EncodeMeKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value               = 0;
    perfTag.Mode                = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType            = CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL;
    perfTag.PictureCodingType   = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    uint32_t krnStateIdx =
        (m_pictureCodingType == P_TYPE) ? CODECHAL_ENCODE_ME_IDX_P : CODECHAL_ENCODE_ME_IDX_B;

    if (m_pictureCodingType == B_TYPE && CodecHal_PictureIsInvalid(m_picParams->m_refFrameList[1]))
    {
        krnStateIdx = CODECHAL_ENCODE_ME_IDX_P;
    }

    auto kernelState = &m_meKernelStates[krnStateIdx];

    if (m_firstTaskInPhase || !m_singleTaskPhaseSupported)
    {
        uint32_t maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnRequestSshSpaceForCmdBuf(
            m_stateHeapInterface,
            maxBtCount));
        m_vmeStatesSize =
            m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(VerifySpaceAvailable());
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS interfaceParams;
    MOS_ZeroMemory(&interfaceParams, sizeof(interfaceParams));
    interfaceParams.pKernelState = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &interfaceParams));

    // This parameter is used to select correct mode mv cost
    // and search path from the predefined tables specifically
    // for Mpeg2 BRC encoding path
    m_seqParams->m_targetUsage = 8;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMe());

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_4X_ME;

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
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(
        m_osInterface,
        &cmdBuffer,
        0));

    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams.EncFunctionType    = encFunctionType;
    sendKernelCmdsParams.pKernelState       = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(
        &cmdBuffer,
        &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMeSurfaces(&cmdBuffer));

    // Dump SSH for ME kernel
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState)));

    // HW walker
    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode            = m_walkerMode;
    walkerCodecParams.dwResolutionX         = m_downscaledWidthInMb4x;
    walkerCodecParams.dwResolutionY         = m_downscaledFrameFieldHeightInMb4x;
    walkerCodecParams.bNoDependency         = true;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(
        &cmdBuffer,
        m_singleTaskPhaseSupported,
        m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            m_renderContextUsesNullHw));
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

uint32_t CodechalEncodeMpeg2::CalcFrameRateValue(
    uint16_t frameRateCode,
    uint32_t factor)
{
    uint32_t ret;
    switch(frameRateCode)
    {
        // Note a frame rate code of 0 is forbidden according to MPEG-2 spec
    case 0x1:
        ret = (uint32_t)((24000/1001.0)*factor);
        break;
    case 0x2:
        ret = 24 * factor;
        break;
    case 0x3:
        ret = 25*factor;
        break;
    case 0x4:
        ret = (uint32_t)((30000/1001.0)*factor);
        break;
    case 0x5:
        ret = 30 * factor;
        break;
    case 0x6:
        ret = 50 * factor;
        break;
    case 0x7:
        ret = (uint32_t)((60000/1001.0)*factor);
        break;
    case 0x8:
        ret = 60*factor;
        break;
    default:
        ret = 0xdeadbeef;
    }

    return ret;
}

MOS_STATUS CodechalEncodeMpeg2::SetCurbeBrcInitReset()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    BrcInitResetCurbe cmd;

    cmd.m_curbeData.DW1.m_initBufFullInBits = m_seqParams->m_initVBVBufferFullnessInBit;
    cmd.m_curbeData.DW2.m_bufSizeInBits = m_seqParams->m_vbvBufferSize * CODEC_ENCODE_MPEG2_VBV_BUFFER_SIZE_UNITS;
    cmd.m_curbeData.DW3.m_averageBitRate = m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS;
    cmd.m_curbeData.DW4.m_maxBitRate = m_seqParams->m_maxBitRate * CODECHAL_ENCODE_BRC_KBPS;

    if (m_picParams->m_gopPicSize == 1)
    {
        cmd.m_curbeData.DW8.m_gopP = 0;
        cmd.m_curbeData.DW9.m_gopB = 0;
    }
    else
    {
        cmd.m_curbeData.DW8.m_gopP = (m_picParams->m_gopRefDist) ? ((m_picParams->m_gopPicSize - 1) / m_picParams->m_gopRefDist) : 0;
        cmd.m_curbeData.DW9.m_gopB = (m_picParams->m_gopRefDist - 1) * cmd.m_curbeData.DW8.m_gopP;
    }
    cmd.m_curbeData.DW9.m_frameWidthInBytes = m_frameWidth;
    cmd.m_curbeData.DW10.m_frameHeightInBytes = m_frameHeight;
    cmd.m_curbeData.DW11.m_minQP = 1;
    cmd.m_curbeData.DW12.m_noSlices = ((m_frameHeight + 31) >> 5) << 1;

    // Frame Rate m_value Scaled by m_frameRateDenom
    uint32_t scaledFrameRateValue = CalcFrameRateValue(m_seqParams->m_frameRateCode, m_frameRateDenom);

    if (CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic))
    {
        cmd.m_curbeData.DW6.m_frameRateM = scaledFrameRateValue;
    }
    else // This else clause will only be taken when interlaced field support is added to MPEG-2.
    {
        cmd.m_curbeData.DW6.m_frameRateM = scaledFrameRateValue * 2;
    }

    cmd.m_curbeData.DW7.m_frameRateD = m_frameRateDenom;
    cmd.m_curbeData.DW8.m_brcFlag = (CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) ? (0) : (CODECHAL_ENCODE_BRCINIT_FIELD_PIC);

    if (m_seqParams->m_rateControlMethod == RATECONTROL_CBR)
    {
        cmd.m_curbeData.DW4.m_maxBitRate = cmd.m_curbeData.DW3.m_averageBitRate;
        cmd.m_curbeData.DW8.m_brcFlag = cmd.m_curbeData.DW8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (m_seqParams->m_rateControlMethod == RATECONTROL_VBR)
    {
        cmd.m_curbeData.DW8.m_brcFlag = cmd.m_curbeData.DW8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (m_seqParams->m_rateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_curbeData.DW10.m_avbrAccuracy = m_avbrAccuracy;
        cmd.m_curbeData.DW11.m_avbrConvergence = m_avbrConvergence;
        cmd.m_curbeData.DW8.m_brcFlag = cmd.m_curbeData.DW8.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
        // For AVBR, only honor bitrate from app => InitVBV = Bitrate, Buffer size =  2*Bitrate, max bitrate = target bitrate,
        cmd.m_curbeData.DW1.m_initBufFullInBits = m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS; //m_seqParams->bit_rate * ENCODE_BRC_KBPS;
        cmd.m_curbeData.DW2.m_bufSizeInBits = 2 * m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS; //m_seqParams->bit_rate * ENCODE_BRC_KBPS;
        cmd.m_curbeData.DW3.m_averageBitRate = m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS;
        cmd.m_curbeData.DW4.m_maxBitRate = m_seqParams->m_bitrate * CODECHAL_ENCODE_BRC_KBPS;
    }

    // Profile & level max frame size
    uint32_t defaultFrameSize = m_frameWidth * m_frameHeight;
    if (m_seqParams->m_userMaxFrameSize > 0)
    {
        cmd.m_curbeData.DW0.m_profileLevelMaxFrame = MOS_MIN(m_seqParams->m_userMaxFrameSize, defaultFrameSize);
    }
    else
    {
        cmd.m_curbeData.DW0.m_profileLevelMaxFrame = defaultFrameSize;
    }

    uint32_t brcKernelIdx = (m_brcInit) ? CODECHAL_ENCODE_BRC_IDX_INIT : CODECHAL_ENCODE_BRC_IDX_RESET;
    PMHW_KERNEL_STATE kernelState        = &m_brcKernelStates[brcKernelIdx];
    if (m_brcInit)
    {
        m_brcInitCurrentTargetBufFullInBits = cmd.m_curbeData.DW1.m_initBufFullInBits;
    }
    m_brcInitResetBufSizeInBits = (double)cmd.m_curbeData.DW2.m_bufSizeInBits;
    m_brcInitResetInputBitsPerFrame =
        ((double)(cmd.m_curbeData.DW4.m_maxBitRate) * (double)(cmd.m_curbeData.DW7.m_frameRateD) /(double)(cmd.m_curbeData.DW6.m_frameRateM));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        cmd.m_byteSize));

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::SendBrcInitResetSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    uint32_t brcKernelIdx = (m_brcInit) ? CODECHAL_ENCODE_BRC_IDX_INIT : CODECHAL_ENCODE_BRC_IDX_RESET;
    PMHW_KERNEL_STATE kernelState = &m_brcKernelStates[brcKernelIdx];

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.presBuffer = &m_brcBuffers.resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = m_brcHistoryBufferSize;
    surfaceCodecParams.dwBindingTableOffset = brcInitResetHistory;
    surfaceCodecParams.bIsWritable = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // AVC_ME BRC Distortion data buffer - output
    m_brcBuffers.sMeBrcDistortionBuffer.dwWidth = MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64);
    m_brcBuffers.sMeBrcDistortionBuffer.dwHeight = MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8);
    m_brcBuffers.sMeBrcDistortionBuffer.dwPitch = m_brcBuffers.sMeBrcDistortionBuffer.dwWidth;

    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.psSurface = &m_brcBuffers.sMeBrcDistortionBuffer;
    surfaceCodecParams.dwOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwBindingTableOffset = brcInitResetDistortion;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::EncodeBrcInitResetKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value               = 0;
    perfTag.Mode                = (uint32_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType            = CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET;
    perfTag.PictureCodingType   = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    uint32_t brcKernelIdx = (m_brcInit) ? CODECHAL_ENCODE_BRC_IDX_INIT : CODECHAL_ENCODE_BRC_IDX_RESET;
    PMHW_KERNEL_STATE kernelState        = &m_brcKernelStates[brcKernelIdx];

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

    // Setup Mpeg2 Curbe
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS interfaceParams;
    MOS_ZeroMemory(&interfaceParams, sizeof(interfaceParams));
    interfaceParams.pKernelState  = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &interfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcInitReset());

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_INIT_RESET;
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

    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams.EncFunctionType    = encFunctionType;
    sendKernelCmdsParams.bBrcResetRequested = m_brcReset;
    sendKernelCmdsParams.pKernelState        = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    //Add surface states
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcInitResetSurfaces(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    MediaObjectInlineDataMpeg2 mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    // add end of commands here for eStatus report
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
        &cmdBuffer,
        encFunctionType,
        nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(
        &cmdBuffer,
        m_singleTaskPhaseSupported,
        m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface, &cmdBuffer,
            m_renderContextUsesNullHw));
        m_lastTaskInPhase = false;
    }
    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::EncodeMbEncKernel(bool mbEncIFrameDistEnabled)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value               = 0;
    perfTag.Mode                = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType            =
        (mbEncIFrameDistEnabled) ? CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST : CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL;
    perfTag.PictureCodingType   = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    CODECHAL_MEDIA_STATE_TYPE encFunctionType;
    if (mbEncIFrameDistEnabled)
    {
        encFunctionType = CODECHAL_MEDIA_STATE_ENC_I_FRAME_DIST;
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

    PMHW_KERNEL_STATE kernelState;
    uint8_t           codingType = m_mbEncForcePictureCodingType ?
        m_mbEncForcePictureCodingType : (uint8_t)m_pictureCodingType;
    // Initialize DSH kernel region
    if (mbEncIFrameDistEnabled)
    {
        kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST];
    }
    else
    {
        // wPictureCodingType: I_TYPE = 1, P_TYPE = 2, B_TYPE = 3
        // KernelStates are I: 0, P: 1, B: 2
        // m_mbEncKernelStates: I: m_mbEncKernelStates[0], P: m_mbEncKernelStates[1], B: m_mbEncKernelStates[2]
        uint32_t krnStateIdx = codingType - 1;

        kernelState = &m_mbEncKernelStates[krnStateIdx];
    }

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

    if (m_mbEncCurbeSetInBrcUpdate)
    {
        // single task phase disabled for MPEG2 MbEnc
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            m_stateHeapInterface,
            kernelState,
            true,
            0,
            m_singleTaskPhaseSupported,
            m_storeData));
    }
    else
    {
        // Set up the DSH as normal
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
            m_stateHeapInterface,
            kernelState,
            false,
            0,
            false,
            m_storeData));

        MHW_INTERFACE_DESCRIPTOR_PARAMS interfaceParams;
        MOS_ZeroMemory(&interfaceParams, sizeof(interfaceParams));
        interfaceParams.pKernelState  = kernelState;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
            m_stateHeapInterface,
            1,
            &interfaceParams));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMbEnc(mbEncIFrameDistEnabled, m_mbQpDataEnabled));

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
    }

    for (uint8_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
    {
        if (m_picIdx[i].bValid)
        {
            auto Index = m_picIdx[i].ucPicIdx;
            m_refList[Index]->sRefBuffer = m_picParams->m_useRawPicForRef ?
                m_refList[Index]->sRefRawBuffer : m_refList[Index]->sRefReconBuffer;

            if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
            {
                auto pResRefMbCodeBuffer = (MOS_RESOURCE*)m_allocator->GetResource(m_standard, mbCodeBuffer, m_refList[Index]->ucMbCodeIdx);

                if (pResRefMbCodeBuffer)
                {
                    m_refList[Index]->resRefMbCodeBuffer = *pResRefMbCodeBuffer;
                }
            }

            CodecHalGetResourceInfo(m_osInterface, &m_refList[Index]->sRefBuffer);
        }
    }

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams.EncFunctionType       = encFunctionType;
    sendKernelCmdsParams.pKernelState          = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

   CODECHAL_ENCODE_CHK_STATUS_RETURN(SendMbEncSurfaces(&cmdBuffer, mbEncIFrameDistEnabled));

    if ((codingType != B_TYPE) && (!mbEncIFrameDistEnabled))
    {
        m_prevMBCodeIdx = m_currReconstructedPic.FrameIdx;
    }

    // HW walker
    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode                = m_walkerMode;
    walkerCodecParams.bUseScoreboard            = m_useHwScoreboard;
    walkerCodecParams.dwResolutionX             = mbEncIFrameDistEnabled ?
        m_downscaledWidthInMb4x : (uint32_t)m_picWidthInMb;
    walkerCodecParams.dwResolutionY             = mbEncIFrameDistEnabled ?
        m_downscaledFrameFieldHeightInMb4x : (uint32_t)m_frameFieldHeightInMb;

    if (codingType == I_TYPE)
    {
        walkerCodecParams.bUseScoreboard            = false;
        walkerCodecParams.bNoDependency             = true;     /* Enforce no dependency dispatch order for I frame */
    }
    else if (codingType == P_TYPE)
    {
        // walkerCodecParams.wPictureCodingType can be different from m_pictureCodingType
        walkerCodecParams.wPictureCodingType        = I_TYPE;   /* Enforce 45 degree dispatch order for P frame, as by default it's 26 degree */
    }
    else// B_TYPE
    {
        walkerCodecParams.bUseVerticalRasterScan    = true;
    }

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(
        m_hwInterface,
        &walkerParams,
        &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObjectWalkerCmd(
        &cmdBuffer,
        &walkerParams));

    // add end of commands here for eStatus report
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr));
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

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(&cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SendBrcUpdateSurfaces(
    PMOS_COMMAND_BUFFER                                 cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    auto kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_FrameBRC_UPDATE];
    auto mbEncKernelState = m_brcBuffers.pMbEncKernelStateInUse;

    // BRC history buffer
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.presBuffer = &m_brcBuffers.resBrcHistoryBuffer;
    surfaceCodecParams.dwSize = m_brcHistoryBufferSize;
    surfaceCodecParams.dwBindingTableOffset = brcUpdateHistory;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK Statistics buffer
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = &m_brcBuffers.resBrcPakStatisticBuffer[0];
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(m_brcPakStatisticsSize);
    surfaceCodecParams.dwBindingTableOffset = brcUpdatePakStaticOutput;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - read only
    uint32_t bufSize = MOS_BYTES_TO_DWORDS(BRC_IMG_STATE_SIZE_PER_PASS * m_mfxInterface->GetBrcNumPakPasses());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer =
        &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx];
    surfaceCodecParams.dwSize = bufSize;
    surfaceCodecParams.dwBindingTableOffset = brcUpdatePictureStateRead;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // PAK IMG_STATEs buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.presBuffer = &m_brcBuffers.resBrcImageStatesWriteBuffer;
    surfaceCodecParams.dwSize = bufSize;
    surfaceCodecParams.dwBindingTableOffset = brcUpdatePictureStateWrite;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC ENC CURBE Buffer - read only
    MOS_RESOURCE *dsh = nullptr;
    CODECHAL_ENCODE_CHK_NULL_RETURN(dsh = mbEncKernelState->m_dshRegion.GetResource());
    bufSize = MOS_ALIGN_CEIL(
        mbEncKernelState->KernelParams.iCurbeLength,
        m_stateHeapInterface->pStateHeapInterface->GetCurbeAlignment());
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        mbEncKernelState->m_dshRegion.GetOffset() +
        mbEncKernelState->dwCurbeOffset;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(bufSize);
    surfaceCodecParams.dwBindingTableOffset = brcUpdateMbencCurbeRead;
    // If the protection DSH isn't used, the same DSH is used for both the MbEnc CURBE read and write
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // BRC ENC CURBE Buffer - write only
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = dsh;
    surfaceCodecParams.dwOffset =
        mbEncKernelState->m_dshRegion.GetOffset() +
        mbEncKernelState->dwCurbeOffset;
    surfaceCodecParams.dwSize = MOS_BYTES_TO_DWORDS(bufSize);
    surfaceCodecParams.dwBindingTableOffset = brcUpdateMbencCurbeWrite;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // MPEG2_ME BRC Distortion data buffer - input
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.bMediaBlockRW = true;
    surfaceCodecParams.psSurface = &m_brcBuffers.sMeBrcDistortionBuffer;
    surfaceCodecParams.dwOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
    surfaceCodecParams.dwSize = bufSize;
    surfaceCodecParams.dwBindingTableOffset = brcUpdateDistortion;
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
        &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];
    surfaceCodecParams.dwBindingTableOffset = brcUpdateConstantData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Picture header input surface
    bufSize = m_picHeaderDataBufferSize;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer =
        &m_brcBuffers.resBrcPicHeaderInputBuffer;
    surfaceCodecParams.dwSize = bufSize;
    surfaceCodecParams.dwBindingTableOffset = brcUpdatePicHeaderInputData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Picture header output surface
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIsWritable = true;
    surfaceCodecParams.presBuffer = &m_brcBuffers.resBrcPicHeaderOutputBuffer;
    surfaceCodecParams.dwSize = bufSize;
    surfaceCodecParams.dwBindingTableOffset = brcUpdateOutputData;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SetCurbeBrcUpdate()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    BrcUpdateCurbe cmd;

    cmd.m_curbeData.DW5.m_targetSizeFlag = 0;
    if (m_brcInitCurrentTargetBufFullInBits > m_brcInitResetBufSizeInBits)
    {
        m_brcInitCurrentTargetBufFullInBits -= m_brcInitResetBufSizeInBits;
        cmd.m_curbeData.DW5.m_targetSizeFlag = 1;
    }
    cmd.m_curbeData.DW0.m_targetSize = (uint32_t)m_brcInitCurrentTargetBufFullInBits;
    cmd.m_curbeData.DW5.m_currFrameType = m_pictureCodingType - 1;

    cmd.m_curbeData.DW5.m_brcFlag = (CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) ? (0) : (CODECHAL_ENCODE_BRCINIT_FIELD_PIC);

    if (m_seqParams->m_rateControlMethod == RATECONTROL_CBR)
    {
        cmd.m_curbeData.DW5.m_brcFlag = cmd.m_curbeData.DW5.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISCBR;
    }
    else if (m_seqParams->m_rateControlMethod == RATECONTROL_VBR)
    {
        cmd.m_curbeData.DW5.m_brcFlag = cmd.m_curbeData.DW5.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISVBR;
    }
    else if (m_seqParams->m_rateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_curbeData.DW5.m_brcFlag = cmd.m_curbeData.DW5.m_brcFlag | CODECHAL_ENCODE_BRCINIT_ISAVBR;
    }

    cmd.m_curbeData.DW6.m_qScaleTypeOffset = m_qScaleTypeByteOffse;
    cmd.m_curbeData.DW6.m_vbvDelay = m_vbvDelayOffset;
    cmd.m_curbeData.DW7.m_picHeaderDataBufferSize = m_picHeaderDataBufferSize;
    cmd.m_curbeData.DW15.m_extraHeaders = 1;
    cmd.m_curbeData.DW15.m_intraDcPrecisionOffset = m_intraDcPrecisionOffset;

    m_brcInitCurrentTargetBufFullInBits += m_brcInitResetInputBitsPerFrame;

    if (m_seqParams->m_rateControlMethod == RATECONTROL_AVBR)
    {
        cmd.m_curbeData.DW3.m_startGAdjFrame0 = (uint32_t)((10 * m_avbrConvergence) / (double)150);
        cmd.m_curbeData.DW3.m_startGAdjFrame1 = (uint32_t)((50 * m_avbrConvergence) / (double)150);
        cmd.m_curbeData.DW4.m_startGAdjFrame2 = (uint32_t)((100 * m_avbrConvergence) / (double)150);
        cmd.m_curbeData.DW4.m_startGAdjFrame3 = (uint32_t)((150 * m_avbrConvergence) / (double)150);
        cmd.m_curbeData.DW11.m_gRateRatioThreshold0 = (uint32_t)((100 - (m_avbrAccuracy / (double)30) * (100 - 40)));
        cmd.m_curbeData.DW11.m_gRateRatioThreshold1 = (uint32_t)((100 - (m_avbrAccuracy / (double)30) * (100 - 75)));
        cmd.m_curbeData.DW12.m_gRateRatioThreshold2 = (uint32_t)((100 - (m_avbrAccuracy / (double)30) * (100 - 97)));
        cmd.m_curbeData.DW12.m_gRateRatioThreshold3 = (uint32_t)((100 + (m_avbrAccuracy / (double)30) * (103 - 100)));
        cmd.m_curbeData.DW12.m_gRateRatioThreshold4 = (uint32_t)((100 + (m_avbrAccuracy / (double)30) * (125 - 100)));
        cmd.m_curbeData.DW12.m_gRateRatioThreshold5 = (uint32_t)((100 + (m_avbrAccuracy / (double)30) * (160 - 100)));
    }

    if (m_seqParams->m_forcePanicModeControl == 1) {
        cmd.m_curbeData.DW14.m_forceToSkip = m_seqParams->m_panicModeDisable ? 0 : 1;
    } else {
        cmd.m_curbeData.DW14.m_forceToSkip = m_panicEnable ? 1 : 0;
    }
    auto kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_FrameBRC_UPDATE];
    CODECHAL_ENCODE_CHK_STATUS_RETURN(kernelState->m_dshRegion.AddData(
        &cmd,
        kernelState->dwCurbeOffset,
        cmd.m_byteSize));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::InitBrcConstantBuffer()
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto brcConstantDataBuffer  =
        m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx];

    CodechalResLock bufLock(m_osInterface, &brcConstantDataBuffer.OsResource);
    auto data = (uint8_t *)bufLock.Lock(CodechalResLock::writeOnly);
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    MOS_ZeroMemory(data, brcConstantDataBuffer.dwWidth * brcConstantDataBuffer.dwHeight);

    uint8_t *maxFrameThresholdArray = nullptr;
    uint8_t *distQPAdjustmentArray  = nullptr;
    switch(m_pictureCodingType)
    {
    case I_TYPE:
        maxFrameThresholdArray = (uint8_t *)m_qpAdjustmentDistThresholdMaxFrameThresholdI;
        distQPAdjustmentArray  = (uint8_t *)m_distQpAdjustmentI;
        break;
    case P_TYPE:
        maxFrameThresholdArray = (uint8_t *)m_qpAdjustmentDistThresholdMaxFrameThresholdP;
        distQPAdjustmentArray  = (uint8_t *)m_distQpAdjustmentP;
        break;
    case B_TYPE:
        maxFrameThresholdArray = (uint8_t *)m_qpAdjustmentDistThresholdMaxFrameThresholdB;
        distQPAdjustmentArray  = (uint8_t *)m_distQpAdjustmentB;
        break;
    default:
        CODECHAL_ENCODE_ASSERTMESSAGE("Invalid picture coding type.");
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        return eStatus;
    }

    // Fill surface with QP Adjustment table, Distortion threshold table, MaxFrame threshold table for I frame
    // The surface width happens to be the size of the array.
    CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
        data,
        m_frameThresholdArraySize,
        maxFrameThresholdArray,
        m_frameThresholdArraySize));

    data += m_frameThresholdArraySize;

    for (uint32_t i = 0; i < m_distQpAdjustmentArraySize; i += m_brcConstantSurfaceWidth)
    {
        uint32_t copySize;
        if ((m_distQpAdjustmentArraySize - i) > m_brcConstantSurfaceWidth)
        {
            copySize = m_brcConstantSurfaceWidth;
        }
        else
        {
            copySize = m_distQpAdjustmentArraySize - i;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(MOS_SecureMemcpy(
            data + i,
            copySize,
            distQPAdjustmentArray + i,
            copySize));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::EncodeBrcUpdateKernel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value               = 0;
    perfTag.Mode                = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType            = CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE;
    perfTag.PictureCodingType   = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    auto kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_FrameBRC_UPDATE];

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

    // wPictureCodingType: I_TYPE = 1, P_TYPE = 2, B_TYPE = 3
    // KernelStates are I: 0, P: 1, B: 2
    // m_mbEncKernelStates: I: m_mbEncKernelStates[0], P: m_mbEncKernelStates[1], B: m_mbEncKernelStates[2]
    uint32_t krnStateIdx = m_pictureCodingType - 1;

    if (m_mbEncForcePictureCodingType)
    {
        krnStateIdx = m_mbEncForcePictureCodingType - 1;
    }

    auto mbEncKernelState = &m_mbEncKernelStates[krnStateIdx];

    // Setup MbEnc Curbe
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        mbEncKernelState,
        false,
        0,
        !m_singleTaskPhaseSupported,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS interfaceParams;
    MOS_ZeroMemory(&interfaceParams, sizeof(interfaceParams));
    interfaceParams.pKernelState  = mbEncKernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &interfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeMbEnc(0, 0));

    // Brc Update
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        m_stateHeapInterface,
        kernelState,
        false,
        0,
        false,
        m_storeData));

    MOS_ZeroMemory(&interfaceParams, sizeof(interfaceParams));
    interfaceParams.pKernelState  = kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetInterfaceDescriptor(
        m_stateHeapInterface,
        1,
        &interfaceParams));

    m_mbEncCurbeSetInBrcUpdate = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbeBrcUpdate());

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_BRC_UPDATE;
    CODECHAL_MEDIA_STATE_TYPE mbEncFunctionType;
    if (m_kernelMode == encodeNormalMode)
    {
        mbEncFunctionType = CODECHAL_MEDIA_STATE_ENC_NORMAL;
    }
    else if (m_kernelMode == encodePerformanceMode)
    {
        mbEncFunctionType = CODECHAL_MEDIA_STATE_ENC_PERFORMANCE;
    }
    else
    {
        mbEncFunctionType = CODECHAL_MEDIA_STATE_ENC_QUALITY;
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

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams;
    sendKernelCmdsParams.EncFunctionType    = encFunctionType;
    sendKernelCmdsParams.pKernelState       = kernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // Add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSetBindingTable(
        m_stateHeapInterface,
        kernelState));

    m_brcBuffers.pMbEncKernelStateInUse = mbEncKernelState;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(InitBrcConstantBuffer());

    //Set MFX_MPEG2_PIC_STATE command
    MHW_VDBOX_MPEG2_PIC_STATE mpeg2PicState;
    MOS_ZeroMemory(&mpeg2PicState, sizeof(mpeg2PicState));
    mpeg2PicState.pEncodeMpeg2PicParams  = m_picParams;
    mpeg2PicState.pEncodeMpeg2SeqParams  = m_seqParams;
    mpeg2PicState.wPicWidthInMb          = m_picWidthInMb;
    mpeg2PicState.wPicHeightInMb         = m_picHeightInMb;
    mpeg2PicState.ppRefList              = &(m_refList[0]);
    mpeg2PicState.bBrcEnabled            = true;
    mpeg2PicState.bTrellisQuantEnable    = false;
    mpeg2PicState.ucKernelMode           = m_kernelMode;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxMpeg2PicBrcBuffer(
        &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx],
        &mpeg2PicState));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendBrcUpdateSurfaces(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            kernelState));
    )

    MHW_MEDIA_OBJECT_PARAMS mediaObjectParams;
    MOS_ZeroMemory(&mediaObjectParams, sizeof(mediaObjectParams));
    MediaObjectInlineData mediaObjectInlineData;
    MOS_ZeroMemory(&mediaObjectInlineData, sizeof(mediaObjectInlineData));
    mediaObjectParams.pInlineData = &mediaObjectInlineData;
    mediaObjectParams.dwInlineDataSize = sizeof(mediaObjectInlineData);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->GetRenderInterface()->AddMediaObject(
        &cmdBuffer,
        nullptr,
        &mediaObjectParams));

    // add end of commands here for eStatus report
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcImageStatesReadBuffer[m_currRecycledBufIdx],
            CodechalDbgAttr::attrInput,
            "ImgStateRead",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].OsResource,
            CodechalDbgAttr::attrInput,
            "ConstData",
            m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].dwPitch * m_brcBuffers.sBrcConstantDataBuffer[m_currRecycledBufIdx].dwHeight,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        // PAK statistics buffer is only dumped for BrcUpdate kernel input
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcPakStatisticBuffer[0],
            CodechalDbgAttr::attrInput,
            "PakStats",
            m_brcPakStatisticsSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.sMeBrcDistortionBuffer.OsResource,
            CodechalDbgAttr::attrInput,
            "BrcDist",
            m_brcBuffers.sMeBrcDistortionBuffer.dwPitch * m_brcBuffers.sMeBrcDistortionBuffer.dwHeight,
            m_brcBuffers.dwMeBrcDistortionBottomFieldOffset,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrInput,
            "HistoryRead",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_resMbStatsBuffer,
            CodechalDbgAttr::attrInput,
            "MBStatsSurf",
            m_hwInterface->m_avcMbStatBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcPicHeaderInputBuffer,
            CodechalDbgAttr::attrInput,
            "PicHeaderRead",
            CODEC_ENCODE_MPEG2_BRC_PIC_HEADER_SURFACE_SIZE,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnSubmitBlocks(
        m_stateHeapInterface,
        kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->pfnUpdateGlobalCmdBufId(
            m_stateHeapInterface));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(
        &cmdBuffer,
        m_singleTaskPhaseSupported,
        m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(
            m_osInterface,
            &cmdBuffer,
            m_renderContextUsesNullHw));
        m_lastTaskInPhase = false;
    }

    return eStatus;
}
MOS_STATUS CodechalEncodeMpeg2::ExecuteKernelFunctions()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
        m_rawSurfaceToEnc,
        CodechalDbgAttr::attrEncodeRawInputSurface,
        "SrcSurf")));

    m_firstTaskInPhase = true;
    m_lastTaskInPhase  = !m_singleTaskPhaseSupported;
    m_lastEncPhase     = false;

    UpdateSSDSliceCount();

    // Csc, Downscaling, and/or 10-bit to 8-bit conversion
    // Scaling is only used to calculate distortions in case of Mpeg2
    CODECHAL_ENCODE_CHK_NULL_RETURN(m_cscDsState);

    CodechalEncodeCscDs::KernelParams cscScalingKernelParams;
    MOS_ZeroMemory(&cscScalingKernelParams, sizeof(cscScalingKernelParams));
    cscScalingKernelParams.bLastTaskInPhaseCSC =
        cscScalingKernelParams.bLastTaskInPhase4xDS = m_pictureCodingType == I_TYPE;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_cscDsState->KernelFunctions(&cscScalingKernelParams));

    // P and B frames distortion calculations
    if (m_hmeSupported && (m_pictureCodingType != I_TYPE))
    {
        m_firstTaskInPhase = !m_singleTaskPhaseSupported;
        m_lastTaskInPhase  = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMeKernel());
    }

    MOS_SYNC_PARAMS syncParams;

    // Scaling and HME are not dependent on the output from PAK
    if (m_waitForPak &&
        m_semaphoreObjCount &&
        !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
    {
        // Wait on PAK
        syncParams                          = g_cInitSyncParams;
        syncParams.GpuContext               = m_renderContext;
        syncParams.presSyncResource         = &m_resSyncObjectVideoContextInUse;
        syncParams.uiSemaphoreCount         = m_semaphoreObjCount;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
        m_semaphoreObjCount         = 0; //reset
    }

    m_firstTaskInPhase = true;
    if (m_brcEnabled)
    {
        if (m_pictureCodingType == I_TYPE)
        {
            // The reset/init is only valid for I frames
            if (m_brcInit || m_brcReset)
            {
                CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcInitResetKernel());
                m_firstTaskInPhase = !m_singleTaskPhaseSupported;
            }

            CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(true));
            m_firstTaskInPhase = !m_singleTaskPhaseSupported;
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeBrcUpdateKernel());
        m_firstTaskInPhase = !m_singleTaskPhaseSupported;
    }

    m_lastTaskInPhase = true;
    m_lastEncPhase = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EncodeMbEncKernel(false));

    if (!Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_renderContext;
        syncParams.presSyncResource     = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
    }

 CODECHAL_DEBUG_TOOL(
    if (m_hmeEnabled && m_brcEnabled)
    {
        CODECHAL_ME_OUTPUT_PARAMS meOutputParams;
        MOS_ZeroMemory(&meOutputParams, sizeof(CODECHAL_ME_OUTPUT_PARAMS));
        meOutputParams.psMeMvBuffer = m_hmeKernel ?
            m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xMvDataBuffer) : &m_4xMEMVDataBuffer;
        meOutputParams.psMeBrcDistortionBuffer =
            m_brcDistortionBufferSupported ? &m_brcBuffers.sMeBrcDistortionBuffer : nullptr;
        meOutputParams.psMeDistortionBuffer = m_hmeKernel ?
            m_hmeKernel->GetSurface(CodechalKernelHme::SurfaceId::me4xDistortionBuffer) : &m_4xMEDistortionBuffer;
        meOutputParams.b16xMeInUse = false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &meOutputParams.psMeMvBuffer->OsResource,
            CodechalDbgAttr::attrOutput,
            "MvData",
            meOutputParams.psMeMvBuffer->dwHeight *meOutputParams.psMeMvBuffer->dwPitch,
            CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 32), 64) * (m_downscaledFrameFieldHeightInMb4x * 4) : 0,
            CODECHAL_MEDIA_STATE_4X_ME));
        if (!m_vdencStreamInEnabled && meOutputParams.psMeBrcDistortionBuffer)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &meOutputParams.psMeBrcDistortionBuffer->OsResource,
                CodechalDbgAttr::attrOutput,
                "BrcDist",
                meOutputParams.psMeBrcDistortionBuffer->dwHeight *meOutputParams.psMeBrcDistortionBuffer->dwPitch,
                CodecHal_PictureIsBottomField(m_currOriginalPic) ? MOS_ALIGN_CEIL((m_downscaledWidthInMb4x * 8), 64) * MOS_ALIGN_CEIL((m_downscaledFrameFieldHeightInMb4x * 4), 8) : 0,
                CODECHAL_MEDIA_STATE_4X_ME));
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
        }
        // dump VDEncStreamin
        if (m_vdencStreamInEnabled)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_resVdencStreamInBuffer[m_currRecycledBufIdx],
                CodechalDbgAttr::attrOutput,
                "MvData",
                m_picWidthInMb * m_picHeightInMb* CODECHAL_CACHELINE_SIZE,
                0,
                CODECHAL_MEDIA_STATE_ME_VDENC_STREAMIN));
        }
    }

    if(m_mbQpDataEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_mbQpDataSurface.OsResource,
            CodechalDbgAttr::attrInput,
            "MbQp",
            m_mbQpDataSurface.dwHeight*m_mbQpDataSurface.dwPitch,
            0,
            CODECHAL_MEDIA_STATE_ENC_QUALITY));
    }
    if (m_brcEnabled)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcImageStatesWriteBuffer,
            CodechalDbgAttr::attrOutput,
            "ImgStateWrite",
            BRC_IMG_STATE_SIZE_PER_PASS * m_hwInterface->GetMfxInterface()->GetBrcNumPakPasses(),
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
            &m_brcBuffers.resBrcHistoryBuffer,
            CodechalDbgAttr::attrOutput,
            "HistoryWrite",
            m_brcHistoryBufferSize,
            0,
            CODECHAL_MEDIA_STATE_BRC_UPDATE));
        if (m_brcBuffers.pMbEncKernelStateInUse)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
                CODECHAL_MEDIA_STATE_BRC_UPDATE,
                m_brcBuffers.pMbEncKernelStateInUse));
        }
        if (m_mbencBrcBufferSize > 0)
        {
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpBuffer(
                &m_brcBuffers.resMbEncBrcBuffer,
                CodechalDbgAttr::attrOutput,
                "MbEncBRCWrite",
                m_mbencBrcBufferSize,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
        }

        CODECHAL_ENCODE_CHK_STATUS_RETURN(
            m_debugInterface->DumpBuffer(
                &m_brcBuffers.resBrcPicHeaderOutputBuffer,
                CodechalDbgAttr::attrOutput,
                "PicHeaderWrite",
                CODEC_ENCODE_MPEG2_BRC_PIC_HEADER_SURFACE_SIZE,
                0,
                CODECHAL_MEDIA_STATE_BRC_UPDATE));
    }
    )

    // Reset after BRC Init has been processed
    m_brcInit = false;

    m_setRequestedEUSlices = false;

    // Reset indices for next frame
    if (m_brcEnabled)
    {
        m_mbEncCurbeSetInBrcUpdate = false;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::ExecutePictureLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PerfTagSetting perfTag;
    perfTag.Value = 0;
    perfTag.Mode = (uint16_t)m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;
    perfTag.CallType = CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE;
    perfTag.PictureCodingType = m_pictureCodingType;
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);

    // set MFX_PIPE_MODE_SELECT values
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams;
    pipeModeSelectParams.Mode = m_mode;
    pipeModeSelectParams.bStreamOutEnabled = true;
    bool suppressReconPic =
        (!m_refList[m_currReconstructedPic.FrameIdx]->bUsedAsRef) &&
        m_suppressReconPicSupported;
    pipeModeSelectParams.bPreDeblockOutEnable  = !suppressReconPic;
    pipeModeSelectParams.bPostDeblockOutEnable = 0;

    // set MFX_PIPE_BUF_ADDR_STATE values
    MHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams;
    pipeBufAddrParams.Mode = m_mode;
    pipeBufAddrParams.psPreDeblockSurface = &m_reconSurface;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_mmcState);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetPipeBufAddr(&pipeBufAddrParams));

    pipeBufAddrParams.psPostDeblockSurface = &m_reconSurface;
    pipeBufAddrParams.psRawSurface = m_rawSurfaceToPak;
    pipeBufAddrParams.presStreamOutBuffer = &m_resStreamOutBuffer[m_currRecycledBufIdx];
    pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer  = &m_resDeblockingFilterRowStoreScratchBuffer;

    // Setting invalid entries to nullptr
    for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        pipeBufAddrParams.presReferences[i]= nullptr;
    }

    //divide by two to account for interlace. for now only 0 and 1 will be valid.
    for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC / 2; i++)
    {
        if (m_picIdx[i].bValid)
        {
            uint8_t picIdx = m_picIdx[i].ucPicIdx;
            CodecHalGetResourceInfo(
                m_osInterface,
                &(m_refList[picIdx]->sRefReconBuffer));
            pipeBufAddrParams.presReferences[i] = &(m_refList[picIdx]->sRefReconBuffer.OsResource);

            //HSW MPEG2 Interlaced VME refine, need extra references
            pipeBufAddrParams.presReferences[i + 2] = &(m_refList[picIdx]->sRefReconBuffer.OsResource);

            CODECHAL_DEBUG_TOOL(
                CODECHAL_ENCODE_CHK_NULL_RETURN(m_debugInterface);

                MOS_SURFACE refSurface;
                MOS_ZeroMemory(&refSurface, sizeof(refSurface));
                refSurface.Format     = Format_NV12;
                refSurface.OsResource = *(pipeBufAddrParams.presReferences[i]);
                CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(
                    m_osInterface,
                    &refSurface));

                m_debugInterface->m_refIndex = (uint16_t)i;
                std::string refSurfName      = "RefSurf[" + std::to_string(static_cast<uint32_t>(m_debugInterface->m_refIndex)) + "]";
                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpYUVSurface(
                    &refSurface,
                    CodechalDbgAttr::attrReferenceSurfaces,
                    refSurfName.c_str()));)
        }
    }

    // set MFX_SURFACE_STATE values
    MHW_VDBOX_SURFACE_PARAMS surfaceParams;
    MOS_ZeroMemory(&surfaceParams, sizeof(surfaceParams));
    surfaceParams.Mode = m_mode;

    // set MFX_IND_OBJ_BASE_ADDR_STATE values
    // MPEG2 doesn't really use presMvObjectBuffer, different from AVC, the MV Data portion of the bitstream is loaded as part of MB control data
    MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
    MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
    indObjBaseAddrParams.Mode = CODECHAL_ENCODE_MODE_MPEG2;
    indObjBaseAddrParams.presMvObjectBuffer = &m_resMbCodeSurface;
    indObjBaseAddrParams.dwMvObjectOffset = m_mvOffset + m_mvBottomFieldOffset;
    indObjBaseAddrParams.dwMvObjectSize = m_mbCodeSize - m_mvOffset;
    indObjBaseAddrParams.presPakBaseObjectBuffer = &m_resBitstreamBuffer;
    indObjBaseAddrParams.dwPakBaseObjectSize = m_bitstreamUpperBound;

    // set MFX_BSP_BUF_BASE_ADDR_STATE values
    MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS bspBufBaseAddrParams;
    MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
    bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resMPCRowStoreScratchBuffer;

    //Set MFX_MPEG2_PIC_STATE command
    MHW_VDBOX_MPEG2_PIC_STATE mpeg2PicState;
    MOS_ZeroMemory(&mpeg2PicState, sizeof(mpeg2PicState));
    mpeg2PicState.pEncodeMpeg2PicParams  = m_picParams;
    mpeg2PicState.pEncodeMpeg2SeqParams  = m_seqParams;
    mpeg2PicState.wPicWidthInMb          = m_picWidthInMb;
    mpeg2PicState.wPicHeightInMb         = m_picHeightInMb;
    mpeg2PicState.ppRefList              = &(m_refList[0]);
    mpeg2PicState.bBrcEnabled            = m_brcEnabled;
    mpeg2PicState.bTrellisQuantEnable    = false;
    mpeg2PicState.ucKernelMode           = m_kernelMode;

    m_hwInterface->m_numRequestedEuSlices    = (m_brcEnabled &&
                                               m_sliceStateEnable &&
                                               ((m_frameHeight * m_frameWidth) >= m_hwInterface->m_mpeg2SSDResolutionThreshold)) ?
                                               m_sliceShutdownRequestState : m_sliceShutdownDefaultState;

    MHW_VDBOX_QM_PARAMS qmParams;
    qmParams.Standard = CODECHAL_MPEG2;
    qmParams.Mode = CODECHAL_ENCODE_MODE_MPEG2;
    qmParams.pMpeg2IqMatrix = (CodecMpeg2IqMatrix *)m_qMatrixParams;

    MHW_VDBOX_QM_PARAMS fqmParams;
    fqmParams.Standard = CODECHAL_MPEG2;
    fqmParams.Mode = CODECHAL_ENCODE_MODE_MPEG2;
    fqmParams.pMpeg2IqMatrix  = (CodecMpeg2IqMatrix *)m_qMatrixParams;

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    // Send command buffer header at the beginning (OS dependent)
    if (!m_singleTaskPhaseSupported || m_firstTaskInPhase)
    {
        // frame tracking tag is only added in the last command buffer header
        auto requestFrameTracking =
            m_singleTaskPhaseSupported ? m_firstTaskInPhase : m_lastTaskInPhase;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendPrologWithFrameTracking(&cmdBuffer, requestFrameTracking));

        m_hwInterface->m_numRequestedEuSlices    = CODECHAL_SLICE_SHUTDOWN_DEFAULT;
    }

    if (m_currPass)
    {
        // Insert conditional batch buffer end
        MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS miConditionalBatchBufferEndParams;
        MOS_ZeroMemory(
            &miConditionalBatchBufferEndParams,
            sizeof(MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS));

        miConditionalBatchBufferEndParams.presSemaphoreBuffer =
            &m_encodeStatusBuf.resStatusBuffer;
        miConditionalBatchBufferEndParams.dwOffset  =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwImageStatusMaskOffset                       +
            (sizeof(uint32_t) * 2);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiConditionalBatchBufferEndCmd(
            &cmdBuffer,
            &miConditionalBatchBufferEndParams));
    }

    if (!m_currPass && m_osInterface->bTagResourceSync)
    {
        // This is a solution to solve the sync tag issue: the sync tag write for PAK is inserted at the end of 2nd pass PAK BB
        // which may be skipped in multi-pass PAK enabled case. The idea here is to insert the previous frame's tag at the beginning
        // of the BB and keep the current frame's tag at the end of the BB. There will be a delay for tag update but it should be fine
        // as long as Dec/VP/Enc won't depend on this PAK so soon.
        MOS_RESOURCE globalGpuContextSyncTagBuffer;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetGpuStatusBufferResource(
            m_osInterface,
            &globalGpuContextSyncTagBuffer));

        uint32_t statusTag = m_osInterface->pfnGetGpuStatusTag(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        MHW_MI_STORE_DATA_PARAMS params;
        params.pOsResource      = &globalGpuContextSyncTagBuffer;
        params.dwResourceOffset = m_osInterface->pfnGetGpuStatusTagOffset(m_osInterface, m_osInterface->CurrentGpuContextOrdinal);
        params.dwValue          = (statusTag > 0)? (statusTag - 1) : 0;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiStoreDataImmCmd(&cmdBuffer, &params));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));

    // Ref surface
    surfaceParams.ucSurfaceStateId = CODECHAL_MFX_REF_SURFACE_ID;
    surfaceParams.psSurface = &m_reconSurface;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));
    // Src surface
    surfaceParams.ucSurfaceStateId = CODECHAL_MFX_SRC_SURFACE_ID;
    surfaceParams.psSurface = m_rawSurfaceToPak;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &surfaceParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPipeBufAddrCmd(&cmdBuffer, &pipeBufAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &bspBufBaseAddrParams));

    if (m_brcEnabled)
    {
        MHW_BATCH_BUFFER batchBuffer;
        MOS_ZeroMemory(&batchBuffer, sizeof(batchBuffer));
        batchBuffer.OsResource      = m_brcBuffers.resBrcImageStatesWriteBuffer;
        batchBuffer.dwOffset        = m_currPass * BRC_IMG_STATE_SIZE_PER_PASS;
        batchBuffer.bSecondLevel    = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(
            &cmdBuffer,
            &batchBuffer));
    }
    else
    {
        auto picStateCmdStart = cmdBuffer.pCmdPtr;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxMpeg2PicCmd(&cmdBuffer, &mpeg2PicState));

        CODECHAL_DEBUG_TOOL(
            auto picStateCmdEnd = cmdBuffer.pCmdPtr;
            uint32_t picStateCmdSize = ((uint32_t)(picStateCmdEnd - picStateCmdStart))*sizeof(uint32_t);
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpData(
                (void *)picStateCmdStart,
                picStateCmdSize,
                CodechalDbgAttr::attrPicParams,
                "PicState")));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxQmCmd(&cmdBuffer, &qmParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxFqmCmd(&cmdBuffer, &fqmParams));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    return eStatus;

}

MOS_STATUS CodechalEncodeMpeg2::SendSliceParams(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_MPEG2_SLICE_STATE    params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->presDataBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pBsBuffer);
    CODECHAL_ENCODE_CHK_NULL_RETURN(params->pSlcData);

    if (params->pSlcData->SliceGroup & SLICE_GROUP_START)
    {
        // add Mpeg2 Slice group commands
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfcMpeg2SliceGroupCmd(cmdBuffer, params));
        MHW_BATCH_BUFFER secondLevelBatchBuffer;
        if (params->bBrcEnabled && params->dwSliceIndex == 0)
        {
            MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
            secondLevelBatchBuffer.OsResource = *(params->presPicHeaderBBSurf);
            secondLevelBatchBuffer.dwOffset = 0;
            secondLevelBatchBuffer.bSecondLevel = true;
            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &secondLevelBatchBuffer));
        }
        else
        {
            // Insert pre-slice headers
            MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
            MOS_ZeroMemory(&pakInsertObjectParams, sizeof(pakInsertObjectParams));
            pakInsertObjectParams.bLastHeader = true;
            pakInsertObjectParams.pBsBuffer = params->pBsBuffer;
            pakInsertObjectParams.dwBitSize = params->dwLength;
            pakInsertObjectParams.dwOffset = params->dwOffset;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(cmdBuffer, nullptr, &pakInsertObjectParams));
        }

        // Insert Batch Buffer Start command to send Mpeg2_PAK_OBJ data for MBs in this slice
        MOS_ZeroMemory(&secondLevelBatchBuffer, sizeof(MHW_BATCH_BUFFER));
        secondLevelBatchBuffer.OsResource = *params->presDataBuffer;
        secondLevelBatchBuffer.dwOffset = params->dwDataBufferOffset;
        secondLevelBatchBuffer.bSecondLevel = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferStartCmd(cmdBuffer, &secondLevelBatchBuffer));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::ExecuteSliceLevel()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    auto cpInterface = m_hwInterface->GetCpInterface();

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    if (m_osInterface->osCpInterface->IsCpEnabled())
    {
        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.bLastPass = (m_currPass == m_numPasses) ? true : false;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->SetMfxProtectionState(m_mfxInterface->IsDecodeInUse(), &cmdBuffer, nullptr, &sliceInfoParam));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(cpInterface->UpdateParams(false));
    }

    MHW_VDBOX_MPEG2_SLICE_STATE sliceState;
    MOS_ZeroMemory(&sliceState, sizeof(sliceState));
    sliceState.presDataBuffer               = &m_resMbCodeSurface;
    sliceState.pMpeg2PicIdx                 = &(m_picIdx[0]);
    sliceState.ppMpeg2RefList               = &(m_refList[0]);
    sliceState.pEncodeMpeg2SeqParams        = m_seqParams;
    sliceState.pEncodeMpeg2PicParams        = m_picParams;
    sliceState.pEncodeMpeg2SliceParams      = m_sliceParams;
    sliceState.pBsBuffer                    = &m_bsBuffer;
    sliceState.bBrcEnabled                  = m_brcEnabled;
    if (m_seqParams->m_forcePanicModeControl == 1) {
        sliceState.bRCPanicEnable               = !m_seqParams->m_panicModeDisable;
    } else {
        sliceState.bRCPanicEnable = m_panicEnable;
    }
    sliceState.presPicHeaderBBSurf          = &m_brcBuffers.resBrcPicHeaderOutputBuffer;

    for (uint16_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        //we should not need to call pfnPackSliceHeader this it's done by hw
        PCODEC_ENCODER_SLCDATA  slcData        = m_slcData;
        CODECHAL_ENCODE_CHK_NULL_RETURN(slcData);
        sliceState.dwDataBufferOffset           =
            m_slcData[slcCount].CmdOffset + m_mbcodeBottomFieldOffset;
        sliceState.dwOffset                     = slcData[slcCount].SliceOffset;
        sliceState.dwLength                     = slcData[slcCount].BitSize;
        sliceState.dwSliceIndex                 = slcCount;
        sliceState.bFirstPass                   = true;
        sliceState.bLastPass                    = false;
        sliceState.pSlcData                     = &slcData[slcCount];
        sliceState.bFirstPass                   = (m_currPass == 0);
        sliceState.bLastPass                    = (m_currPass == m_numPasses);

        CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSliceParams(&cmdBuffer, &sliceState));
    }

    // Insert end of stream if set
    if (m_lastPicInStream)
    {
        MHW_VDBOX_PAK_INSERT_PARAMS pakInsertObjectParams;
        MOS_ZeroMemory(&pakInsertObjectParams,sizeof(pakInsertObjectParams));
        pakInsertObjectParams.bLastPicInStream  = true;
        if (m_codecFunction == CODECHAL_FUNCTION_ENC_PAK)
        {
            pakInsertObjectParams.bSetLastPicInStreamData       = true;
            pakInsertObjectParams.dwBitSize                     = 32;   // use dwBitSize for SrcDataEndingBitInclusion
            pakInsertObjectParams.dwLastPicInStreamData         = (uint32_t)((1 << 16) | startCodeSequenceEnd << 24);
        }
        else
        {
            pakInsertObjectParams.bSetLastPicInStreamData       = false;
            pakInsertObjectParams.dwBitSize                     = 8;    // use dwBitSize for SrcDataEndingBitInclusion
            pakInsertObjectParams.dwLastPicInStreamData         = 0;
        }
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mfxInterface->AddMfxPakInsertObject(&cmdBuffer, nullptr, &pakInsertObjectParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadMfcStatus(&cmdBuffer));

    // BRC PAK statistics different for each pass
    if (m_brcEnabled)
    {
        uint32_t frameOffset =
            (m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize) +
            m_encodeStatusBuf.dwNumPassesOffset                              +   // Num passes offset
            sizeof(uint32_t) * 2;                                                          // pEncodeStatus is offset by 2 DWs in the resource

        EncodeReadBrcPakStatsParams   readBrcPakStatsParams;
        readBrcPakStatsParams.pHwInterface                  = m_hwInterface;
        readBrcPakStatsParams.presBrcPakStatisticBuffer     = &m_brcBuffers.resBrcPakStatisticBuffer[0];
        readBrcPakStatsParams.presStatusBuffer              = &m_encodeStatusBuf.resStatusBuffer;
        readBrcPakStatsParams.dwStatusBufNumPassesOffset    = frameOffset;
        readBrcPakStatsParams.ucPass                        = m_currPass;
        readBrcPakStatsParams.VideoContext                  = m_videoContext;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(ReadBrcPakStatistics(
            &cmdBuffer,
            &readBrcPakStatsParams));
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(
        &cmdBuffer,
        CODECHAL_NUM_MEDIA_STATES));

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(
            &cmdBuffer,
            nullptr));
    }

    std::string Pak_pass = "PAK_PASS[" + std::to_string(static_cast<uint32_t>(m_currPass))+"]";
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            CODECHAL_NUM_MEDIA_STATES,
            Pak_pass.c_str()));

        //CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHal_DbgReplaceAllCommands(
        //    m_debugInterface,
        //    &cmdBuffer));
    )

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    MOS_SYNC_PARAMS syncParams;
    if ((m_currPass == 0) &&
        !Mos_ResourceIsNull(&m_resSyncObjectRenderContextInUse))
    {
        syncParams                      = g_cInitSyncParams;
        syncParams.GpuContext           = m_videoContext;
        syncParams.presSyncResource     = &m_resSyncObjectRenderContextInUse;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
    }

    if (!m_singleTaskPhaseSupported ||
        m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(SubmitCommandBuffer(&cmdBuffer, m_videoContextUsesNullHw));

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState)
            {
                m_mmcState->UpdateUserFeatureKey(&m_reconSurface);
            }
        )

        if ((m_currPass == m_numPasses)  &&
            m_signalEnc                            &&
            !Mos_ResourceIsNull(&m_resSyncObjectVideoContextInUse))
        {
            // Check if the signal obj count exceeds max m_value
            if (m_semaphoreObjCount == MOS_MIN(m_semaphoreMaxCount, MOS_MAX_OBJECT_SIGNALED))
            {
                syncParams                      = g_cInitSyncParams;
                syncParams.GpuContext           = m_renderContext;
                syncParams.presSyncResource     = &m_resSyncObjectVideoContextInUse;

                CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));
                m_semaphoreObjCount--;
            }

            // signal semaphore
            syncParams                      = g_cInitSyncParams;
            syncParams.GpuContext           = m_videoContext;
            syncParams.presSyncResource     = &m_resSyncObjectVideoContextInUse;

            CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));
            m_semaphoreObjCount++;
        }
    }

    // Reset parameters for next PAK execution
    if (m_currPass == m_numPasses)
    {
        m_newPpsHeader = 0;
        m_newSeqHeader = 0;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::PackSkippedMB(uint32_t mbIncrement)
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto bsBuffer = &m_bsBuffer;
    //macroblock_escap: The macroblock_escap`e is a fixed bit-string "0000 0001 000" which is used
    //when the difference between macroblock_address and previous_macroblock_address is greater than 33
    while(mbIncrement > 33)
    {
        PutBits(bsBuffer,0x08,11);
        mbIncrement -= 33;
    }
    // macroblock_address_increment:   This is a variable length coded integer
    //which indicates the difference between macroblock_address and previous_macroblock_address
    PutBits(bsBuffer, mpeg2AddrIncreamentTbl[mbIncrement].m_code, mpeg2AddrIncreamentTbl[mbIncrement].m_len);
    // macroblock_modes()
    //macroblock_type: Variable length coded indicator which indicates the method of coding and
    //content of the macroblock according to the Tables B-2 through B-8 in spec ISO 13818-2,
    //for skip mb, we should choose "MC, NotCoded" for P frame which means there are no quant, backward mv, mb pattern ...
    //choose "Bwd,Not Coded" for B frame
    if(m_pictureCodingType == P_TYPE)
    {
        PutBits(bsBuffer, mpeg2MbTypeTbl[1][8].m_code, mpeg2MbTypeTbl[1][8].m_len);
    }
    else if(m_pictureCodingType == B_TYPE)
    {
        PutBits(bsBuffer, mpeg2MbTypeTbl[2][4].m_code, mpeg2MbTypeTbl[2][4].m_len);
    }
    // frame_motion_type  This is a two bit code indicating the macroblock prediction, 0b10 -- frame-based
    // 0b01 --field based  0b11---Dual-Prime
    // attention: currently, mpeg2 encode only support frame encoding and field frame encoding , so Picture_Struct should be 3
    if(m_picParams->m_framePredFrameDCT == 0)
    {
        PutBits(bsBuffer, 2, 2);
    }
    // motion_vectors   // motion_vector ( 0, 0 )   //
    // set the MV to zero for skip MB.
    PutBits(bsBuffer, mpeg2MvVlcTbl[16 + 0].m_code, mpeg2MvVlcTbl[16 + 0].m_len);
    PutBits(bsBuffer, mpeg2MvVlcTbl[16 + 0].m_code, mpeg2MvVlcTbl[16 + 0].m_len);

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS CodechalEncodeMpeg2::PackSkipSliceData()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    auto slcParams      = m_sliceParams;
    auto bsBuffer       = &m_bsBuffer;

    while (bsBuffer->BitOffset)
    {
        PutBits(bsBuffer, 0, 1);
    }

    for (uint32_t slcCount = 0; slcCount < m_numSlices; slcCount++)
    {
        //slice start code
        PutBits(bsBuffer,0x000001,24);
        PutBits(bsBuffer,slcParams->m_firstMbY + 1,8);
        // quantiser_scale_code
        PutBits(bsBuffer,slcParams->m_quantiserScaleCode, 5);
        // intra_slice_flag
        PutBits(bsBuffer, 1, 1);
        // intra_slice
        PutBits(bsBuffer, slcParams->m_intraSlice, 1);
        // reserved_bits
        PutBits(bsBuffer, 0, 7);
        // extra_bit_slice
        PutBits(bsBuffer, 0, 1);

        PackSkippedMB(1);
        PackSkippedMB(slcParams->m_numMbsForSlice - 1);
        while (bsBuffer->BitOffset)
        {
            PutBits(bsBuffer, 0, 1);
        }
        slcParams++;
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::EncodeCopySkipFrame()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    PackSkipSliceData();

    CodechalResLock bufLock(m_osInterface, &m_resBitstreamBuffer);
    auto data = bufLock.Lock(CodechalResLock::writeOnly);;
    CODECHAL_ENCODE_CHK_NULL_RETURN(data);

    auto bsBuffer           = &m_bsBuffer;
    auto bsSize = (uint32_t)(bsBuffer->pCurrent - bsBuffer->pBase);

    //copy skipped frame
    MOS_SecureMemcpy(data, bsSize, bsBuffer->pBase, bsSize);
    //unlock bitstream buffer
    m_osInterface->pfnUnlockResource( m_osInterface, &m_resBitstreamBuffer );

    //get cmd buffer
    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));
    //start status report
    CODECHAL_ENCODE_CHK_STATUS_RETURN(StartStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    //fill status report
    auto encodeStatus = (EncodeStatus*)(m_encodeStatusBuf.pEncodeStatus +
                    m_encodeStatusBuf.wCurrIndex * m_encodeStatusBuf.dwReportSize);
    encodeStatus->dwMFCBitstreamByteCountPerFrame  = bsSize;
    encodeStatus->dwHeaderBytesInserted            = 0;  // set dwHeaderBytesInserted to 0

    //end status report
    CODECHAL_ENCODE_CHK_STATUS_RETURN(EndStatusReport(&cmdBuffer, CODECHAL_NUM_MEDIA_STATES));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw));

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SendMbEncSurfaces(
    PMOS_COMMAND_BUFFER cmdBuffer,
    bool mbEncIFrameDistEnabled)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    PMHW_KERNEL_STATE kernelState;
    if (mbEncIFrameDistEnabled)
    {
        kernelState = &m_brcKernelStates[CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST];
    }
    else
    {
        // wPictureCodingType: I_TYPE = 1, P_TYPE = 2, B_TYPE = 3
        // KernelStates are I: 0, P: 1, B: 2
        // m_mbEncKernelStates: I: m_mbEncKernelStates[0], P: m_mbEncKernelStates[1], B: m_mbEncKernelStates[2]
        uint32_t krnStateIdx = m_pictureCodingType - 1;

        if (m_mbEncForcePictureCodingType)
        {
            krnStateIdx = m_mbEncForcePictureCodingType - 1;
        }

        kernelState = &m_mbEncKernelStates[krnStateIdx];
    }

    auto presMbCodeBuffer = &m_refList[m_currReconstructedPic.FrameIdx]->resRefMbCodeBuffer;
    auto presPrevMbCodeBuffer = &m_refList[m_prevMBCodeIdx]->resRefMbCodeBuffer;

    // Caution: if PAFF supports added, need to make sure each field get correct surface pointer
    // PAK Obj command buffer
    uint32_t pakSize = (uint32_t)m_picWidthInMb * m_frameFieldHeightInMb * 16 * 4;  // 12 DW for MB + 4 DW for MV
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;

    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.presBuffer = presMbCodeBuffer;
    surfaceCodecParams.dwSize = pakSize;
    surfaceCodecParams.dwOffset = (uint32_t)m_mbcodeBottomFieldOffset;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncPakObj;
    surfaceCodecParams.bRenderTarget = true;
    surfaceCodecParams.bIsWritable = true;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    // Prev PAK Obj command buffer
    pakSize = (uint32_t)m_picWidthInMb * m_frameFieldHeightInMb * 16 * 4;  // 12 DW for MB + 4 DW for MV

    // verify if the current frame is not the first frame
    if (!Mos_ResourceIsNull(presPrevMbCodeBuffer) &&
        !m_firstFrame)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.presBuffer = presPrevMbCodeBuffer;
        surfaceCodecParams.dwSize = pakSize;
        surfaceCodecParams.dwOffset = (uint32_t)m_mbcodeBottomFieldOffset;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PAK_OBJECT_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncPakObjPrev;
        surfaceCodecParams.bRenderTarget = true;
        surfaceCodecParams.bIsWritable = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }
    auto currPicSurface = mbEncIFrameDistEnabled ? m_trackedBuf->Get4xDsSurface(CODEC_CURR_TRACKED_BUFFER) : m_rawSurfaceToEnc;

    // Current Picture Y
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bIs2DSurface = true;
    surfaceCodecParams.psSurface = currPicSurface;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncCurrentY;
    surfaceCodecParams.dwVerticalLineStride = m_verticalLineStride;
    surfaceCodecParams.dwVerticalLineStrideOffset = m_verticalLineStrideOffset;

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    bool currBottomField = CodecHal_PictureIsBottomField(m_currOriginalPic) ? 1 : 0;
    uint8_t vDirection = (CodecHal_PictureIsFrame(m_currOriginalPic)) ? CODECHAL_VDIRECTION_FRAME :
        (currBottomField) ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD;

    // Current Picture
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
    surfaceCodecParams.bUseAdvState = true;
    surfaceCodecParams.psSurface = currPicSurface;
    surfaceCodecParams.dwCacheabilityControl =
        m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncCurrentPic;
    surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        kernelState));

    uint8_t picIdx0 = CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2;
    uint8_t picIdx1 = CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2;
    bool refL0BottomField = false;
    bool refL1BottomField = false;

    if (m_picIdx[0].bValid)
    {
        picIdx0 = m_picIdx[0].ucPicIdx;
        refL0BottomField = (CodecHal_PictureIsBottomField(m_currOriginalPic)) ? 1 : 0;
    }

    if (m_picIdx[1].bValid)
    {
        picIdx1 = m_picIdx[1].ucPicIdx;
        refL1BottomField = (CodecHal_PictureIsBottomField(m_currOriginalPic)) ? 1 : 0;
    }

    // forward reference
    if (picIdx0 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        if (m_verticalLineStride == CODECHAL_VLINESTRIDE_FIELD)
        {
            vDirection = (refL0BottomField ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        }

        // Picture Y
        CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx0]->sRefBuffer);

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = &m_refList[picIdx0]->sRefBuffer;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncForwardPic;
        surfaceCodecParams.ucVDirection = vDirection;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;

#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // backward reference
    if (picIdx1 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
    {
        if (m_verticalLineStride == CODECHAL_VLINESTRIDE_FIELD)
        {
            vDirection = (refL1BottomField ? CODECHAL_VDIRECTION_BOT_FIELD : CODECHAL_VDIRECTION_TOP_FIELD);
        }

        CodecHalGetResourceInfo(m_osInterface, &m_refList[picIdx1]->sRefBuffer);

        // Picture Y VME
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = &m_refList[picIdx1]->sRefBuffer;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncBackwardPic;
        surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // Interlace Frame
    if ((CodecHal_PictureIsFrame(m_picParams->m_currOriginalPic)) &&
        (m_picParams->m_fieldCodingFlag || m_picParams->m_fieldFrameCodingFlag))
    {
        // Current Picture Interlace
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bUseAdvState = true;
        surfaceCodecParams.psSurface = currPicSurface;
        surfaceCodecParams.dwCacheabilityControl =
            m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_CURR_ENCODE].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncInterlaceFrameCurrentPic;
        surfaceCodecParams.ucVDirection = vDirection;

#ifdef _MMC_SUPPORTED
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_mmcState->SetSurfaceParams(&surfaceCodecParams));
#endif
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));

        if (picIdx1 < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            // Picture Y VME
            MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
            surfaceCodecParams.bUseAdvState = true;
            surfaceCodecParams.psSurface = &m_refList[picIdx1]->sRefBuffer;
            surfaceCodecParams.dwCacheabilityControl =
                m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_REF_ENCODE].Value;
            surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncInterlaceFrameBackwardPic;
            surfaceCodecParams.ucVDirection = vDirection;

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

    // BRC distortion data buffer for I frame
    if (mbEncIFrameDistEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_brcBuffers.sMeBrcDistortionBuffer;
        surfaceCodecParams.dwOffset = m_brcBuffers.dwMeBrcDistortionBottomFieldOffset;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncBrcDistortionSurface;
        surfaceCodecParams.bRenderTarget = true;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    // MB-control surface for MB level QP, SkipEnable and NonSkipEnable
    if (m_mbQpDataEnabled)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(CODECHAL_SURFACE_CODEC_PARAMS));
        surfaceCodecParams.bIs2DSurface = true;
        surfaceCodecParams.bMediaBlockRW = true;
        surfaceCodecParams.psSurface = &m_mbQpDataSurface;
        surfaceCodecParams.dwCacheabilityControl =
           m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_MB_QP_CODEC].Value;
        surfaceCodecParams.dwBindingTableOffset = m_mbEncBindingTable.m_mbEncMbControl;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeMpeg2::SendPrologWithFrameTracking(
    PMOS_COMMAND_BUFFER         cmdBuffer,
    bool                        frameTracking,
    MHW_MI_MMIOREGISTERS       *mmioRegister)
{
    return CodechalEncoderState::SendPrologWithFrameTracking(cmdBuffer, frameTracking, mmioRegister);
}

void CodechalEncodeMpeg2::UpdateSSDSliceCount()
{
    m_setRequestedEUSlices = (m_brcEnabled         &&
                                        m_sliceStateEnable &&
                                       (m_frameHeight * m_frameWidth) >= m_hwInterface->m_mpeg2SSDResolutionThreshold) ? true : false;

    m_hwInterface->m_numRequestedEuSlices = (m_setRequestedEUSlices) ?
       m_sliceShutdownRequestState : m_sliceShutdownDefaultState;
}

MOS_STATUS CodechalEncodeMpeg2::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    SendKernelCmdsParams *params)
{
    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    MHW_VFE_PARAMS vfeParams = {};
    vfeParams.pKernelState                      = params->pKernelState;
    vfeParams.eVfeSliceDisable                  = MHW_VFE_SLICE_ALL;
    vfeParams.dwMaximumNumberofThreads          = m_encodeVfeMaxThreads;

    if (!m_useHwScoreboard)
    {
        vfeParams.Scoreboard.ScoreboardMask = 0;
    }
    else
    {
        vfeParams.Scoreboard.ScoreboardEnable     = true;
        vfeParams.Scoreboard.ScoreboardType       = m_hwScoreboardType;
        vfeParams.Scoreboard.ScoreboardMask       = 0xFF;

        // Scoreboard 0
        vfeParams.Scoreboard.ScoreboardDelta[0].x = 0xF;
        vfeParams.Scoreboard.ScoreboardDelta[0].y = 0;

        // Scoreboard 1
        vfeParams.Scoreboard.ScoreboardDelta[1].x = 0;
        vfeParams.Scoreboard.ScoreboardDelta[1].y = 0xF;

        // Scoreboard 2
        vfeParams.Scoreboard.ScoreboardDelta[2].x = 0xE;
        vfeParams.Scoreboard.ScoreboardDelta[2].y = 0;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderEngineInterface->AddMediaVfeCmd(cmdBuffer, &vfeParams));

    return MOS_STATUS_SUCCESS;
}

#if USE_CODECHAL_DEBUG_TOOL
MOS_STATUS CodechalEncodeMpeg2::DumpSeqParams(
    CodecEncodeMpeg2SequenceParams *seqParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSeqParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(seqParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_frameWidth: " << +seqParams->m_frameWidth << std::endl;
    oss << "m_frameHeight: " << +seqParams->m_frameHeight << std::endl;
    oss << "m_profile: " << +seqParams->m_profile << std::endl;
    oss << "m_profile: " << +seqParams->m_level << std::endl;
    oss << "m_chromaFormat: " << +seqParams->m_chromaFormat << std::endl;
    oss << "m_targetUsage: " << +seqParams->m_targetUsage << std::endl;
    oss << "m_aratioFrate: " << +seqParams->m_aratioFrate << std::endl;
    oss << "m_aspectRatio: " << +seqParams->m_aspectRatio << std::endl;
    oss << "m_frameRateCode: " << +seqParams->m_frameRateCode << std::endl;
    oss << "m_frameRateExtN: " << +seqParams->m_frameRateExtN << std::endl;
    oss << "m_frameRateExtD: " << +seqParams->m_frameRateExtD << std::endl;
    oss << "m_bitrate: " << +seqParams->m_bitrate << std::endl;
    oss << "m_vbvBufferSize: " << +seqParams->m_vbvBufferSize << std::endl;
    oss << "m_progressiveSequence: " << +seqParams->m_progressiveSequence << std::endl;
    oss << "m_lowDelay: " << +seqParams->m_lowDelay << std::endl;
    oss << "m_resetBRC: " << +seqParams->m_resetBRC << std::endl;
    oss << "m_noAcceleratorSPSInsertion: " << +seqParams->m_noAcceleratorSPSInsertion << std::endl;
    oss << "m_reserved0: " << +seqParams->m_reserved0 << std::endl;
    oss << "m_rateControlMethod: " << +seqParams->m_rateControlMethod << std::endl;
    oss << "m_reserved1: " << +seqParams->m_reserved1 << std::endl;
    oss << "m_maxBitRate: " << +seqParams->m_maxBitRate << std::endl;
    oss << "m_minBitRate: " << +seqParams->m_minBitRate << std::endl;
    oss << "m_userMaxFrameSize: " << +seqParams->m_userMaxFrameSize << std::endl;
    oss << "m_initVBVBufferFullnessInBit: " << +seqParams->m_initVBVBufferFullnessInBit << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSeqParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeMpeg2::DumpPicParams(
    CodecEncodeMpeg2PictureParams *picParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrPicParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(picParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_currOriginalPic: " << +picParams->m_currOriginalPic.FrameIdx << std::endl;
    oss << "m_currReconstructedPic: " << +picParams->m_currReconstructedPic.FrameIdx << std::endl;
    oss << "m_pictureCodingType: " << +picParams->m_pictureCodingType << std::endl;
    oss << "m_fieldCodingFlag: " << +picParams->m_fieldCodingFlag << std::endl;
    oss << "m_fieldFrameCodingFlag: " << +picParams->m_fieldFrameCodingFlag << std::endl;
    oss << "m_interleavedFieldBFF: " << +picParams->m_interleavedFieldBFF << std::endl;
    oss << "m_progressiveField: " << +picParams->m_progressiveField << std::endl;
    oss << "m_numSlice: " << +picParams->m_numSlice << std::endl;
    oss << "m_picBackwardPrediction: " << +picParams->m_picBackwardPrediction << std::endl;
    oss << "m_bidirectionalAveragingMode: " << +picParams->m_bidirectionalAveragingMode << std::endl;
    oss << "m_pic4MVallowed: " << +picParams->m_pic4MVallowed << std::endl;
    oss << "m_refFrameList: " << +picParams->m_refFrameList[0].FrameIdx << " " << +picParams->m_refFrameList[1].FrameIdx << std::endl;
    oss << "m_useRawPicForRef: " << +picParams->m_useRawPicForRef << std::endl;
    oss << "m_statusReportFeedbackNumber: " << +picParams->m_statusReportFeedbackNumber << std::endl;
    oss << "m_alternateScan: " << +picParams->m_alternateScan << std::endl;
    oss << "m_intraVlcFormat: " << +picParams->m_intraVlcFormat << std::endl;
    oss << "m_qscaleType: " << +picParams->m_qscaleType << std::endl;
    oss << "m_concealmentMotionVectors: " << +picParams->m_concealmentMotionVectors << std::endl;
    oss << "m_framePredFrameDCT: " << +picParams->m_framePredFrameDCT << std::endl;
    oss << "m_disableMismatchControl: " << +picParams->m_disableMismatchControl << std::endl;
    oss << "m_intraDCprecision: " << +picParams->m_intraDCprecision << std::endl;
    oss << "m_fcode00: " << +picParams->m_fcode00 << std::endl;
    oss << "m_fcode01: " << +picParams->m_fcode01 << std::endl;
    oss << "m_fcode10: " << +picParams->m_fcode10 << std::endl;
    oss << "m_fcode11: " << +picParams->m_fcode11 << std::endl;
    oss << "m_lastPicInStream: " << +picParams->m_lastPicInStream << std::endl;
    oss << "m_newGop: " << +picParams->m_newGop << std::endl;
    oss << "m_gopPicSize: " << +picParams->m_gopPicSize << std::endl;
    oss << "m_gopRefDist: " << +picParams->m_gopRefDist << std::endl;
    oss << "m_gopOptFlag: " << +picParams->m_gopOptFlag << std::endl;
    oss << "m_timeCode: " << +picParams->m_timeCode << std::endl;
    oss << "m_temporalReference: " << +picParams->m_temporalReference << std::endl;
    oss << "m_vbvDelay: " << +picParams->m_vbvDelay << std::endl;
    oss << "m_repeatFirstField: " << +picParams->m_repeatFirstField << std::endl;
    oss << "m_compositeDisplayFlag: " << +picParams->m_compositeDisplayFlag << std::endl;
    oss << "m_vaxis: " << +picParams->m_vaxis << std::endl;
    oss << "m_fieldSequence: " << +picParams->m_fieldSequence << std::endl;
    oss << "m_subCarrier: " << +picParams->m_subCarrier << std::endl;
    oss << "m_burstAmplitude: " << +picParams->m_burstAmplitude << std::endl;
    oss << "m_subCarrierPhase: " << +picParams->m_subCarrierPhase << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufPicParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeMpeg2::DumpSliceParams(
    CodecEncodeMpeg2SliceParmas *sliceParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrSlcParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    CODECHAL_DEBUG_CHK_NULL(sliceParams);

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_numMbsForSlice: " << +sliceParams->m_numMbsForSlice << std::endl;
    oss << "m_firstMbX: " << +sliceParams->m_firstMbX << std::endl;
    oss << "m_firstMbY: " << +sliceParams->m_firstMbY << std::endl;
    oss << "m_intraSlice: " << +sliceParams->m_intraSlice << std::endl;
    oss << "m_quantiserScaleCode: " << +sliceParams->m_quantiserScaleCode << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufSlcParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalEncodeMpeg2::DumpVuiParams(
    CodecEncodeMpeg2VuiParams *vuiParams)
{
    CODECHAL_DEBUG_FUNCTION_ENTER;
    CODECHAL_DEBUG_CHK_NULL(vuiParams);

    if (!m_debugInterface->DumpIsEnabled(CodechalDbgAttr::attrVuiParams))
    {
        return MOS_STATUS_SUCCESS;
    }

    std::ostringstream oss;
    oss.setf(std::ios::showbase | std::ios::uppercase);

    oss << "m_videoFormat: " << +vuiParams->m_videoFormat << std::endl;
    oss << "m_colourDescription: " << +vuiParams->m_colourDescription << std::endl;
    oss << "m_colourPrimaries: " << +vuiParams->m_colourPrimaries << std::endl;
    oss << "m_transferCharacteristics: " << +vuiParams->m_transferCharacteristics << std::endl;
    oss << "m_matrixCoefficients: " << +vuiParams->m_matrixCoefficients << std::endl;
    oss << "m_displayHorizontalSize: " << +vuiParams->m_displayHorizontalSize << std::endl;
    oss << "m_displayVerticalSize: " << +vuiParams->m_displayVerticalSize << std::endl;

    const char *fileName = m_debugInterface->CreateFileName(
        "_DDIEnc",
        CodechalDbgBufferType::bufVuiParams,
        CodechalDbgExtType::txt);

    std::ofstream ofs(fileName, std::ios::out);
    ofs << oss.str();
    ofs.close();
    return MOS_STATUS_SUCCESS;
}
#endif

/**************************************************************************
* MB sequence in a non-MBAFF picture with WidthInMB = 5 & HeightInMB = 6
* 26 degree walking pattern
*        0    1    2    3    4
*       -----------------------
*   0  | 0    1    2    4    6
*   1  | 3    5    7    9    11
*   2  | 8    10   12   14   16
*   3  | 13   15   17   19   21
*   4  | 18   20   22   24   26
*   5  | 23   25   27   28   29
***************************************************************************/
void CodechalEncodeMpeg2::MBWalker(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint16_t i, j;
    uint16_t numMBs = picWidthInMB * picHeightInMB;
    uint16_t curX = 0, curY = 0, wflenX, wflenY;
    uint16_t wfstart = 0, wflen;
    uint16_t walkX, walkY;

    wflenY = picHeightInMB - 1;
    for (i = 0; i < numMBs; i++)
    {
        // Get current region length
        wflenX = curX / 2;
        wflen = (wflenX < wflenY) ? wflenX : wflenY;

        mbmap[wfstart] = curY * picWidthInMB + curX;

        if (wfstart == (numMBs - 1))
            break;

        walkX = curX - 2;
        walkY = curY + 1;
        for (j = 0; j < wflen; j++)
        {
            mbmap[wfstart + j + 1] = walkY * picWidthInMB + walkX;
            walkX -= 2;
            walkY++;
        }

        // Start new region
        if (curX == (picWidthInMB - 1))
        {
            // Right picture boundary reached, adjustment required
            curX--;
            curY++;
            wflenY--;
        }
        else
        {
            curX++;
        }
        wfstart += (wflen + 1);
    }
}

/**************************************************************************
* MB sequence in a non-MBAFF picture with WidthInMB = 5 & HeightInMB = 6
* 45 degree pattern
*        0    1    2    3    4
*       -----------------------
*   0  | 0    1    3    6    10
*   1  | 2    4    7    11   15
*   2  | 5    8    12   16   20
*   3  | 9    13   17   21   24
*   4  | 14   18   22   25   27
*   5  | 19   23   26   28   29
***************************************************************************/
void CodechalEncodeMpeg2::MBWalker45Degree(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint16_t i, j;
    uint16_t numMBs = picWidthInMB * picHeightInMB;
    uint16_t curX = 0, curY = 0, wflenX, wflenY;
    uint16_t wfstart = 0, wflen;
    uint16_t walkX, walkY;

    wflenY = picHeightInMB - 1;
    for (i = 0; i < numMBs; i++)
    {
        // Get current region length
        wflenX = curX;
        wflen = (wflenX < wflenY) ? wflenX : wflenY;

        mbmap[wfstart] = curY * picWidthInMB + curX;

        if (wfstart == (numMBs - 1))
            break;

        walkX = curX - 1;
        walkY = curY + 1;
        for (j = 0; j < wflen; j++)
        {
            mbmap[wfstart + j + 1] = walkY * picWidthInMB + walkX;
            walkX -= 1;
            walkY++;
        }

        // Start new region
        if (curX == (picWidthInMB - 1))
        {
            // Right picture boundary reached, adjustment required
            curY++;
            wflenY--;
        }
        else
        {
            curX++;
        }
        wfstart += (wflen + 1);
    }
}

/**************************************************************************
* MB sequence in a MBAFF picture with WidthInMB = 5 & HeightInMB = 6
* 26 degree walking pattern
*        0    1    2    3    4
*       -----------------------
*   0  | 0    2    4    8    12
*   1  | 1    3    6    10   15
*   2  | 5    9    13   18   22
*   3  | 7    11   16   20   24
*   4  | 14   19   23   26   28
*   5  | 17   21   25   27   29
***************************************************************************/
void CodechalEncodeMpeg2::MBWalkerMBAFF(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint16_t i, j, k;
    uint16_t numMBs = picWidthInMB * picHeightInMB;
    uint16_t curX = 0, curY = 0, wflenX, wflenY;
    uint16_t wfstart = 0, wflen;
    uint16_t walkX, walkY;

    wflenY = picHeightInMB / 2 - 1;
    for (i = 0; i < numMBs; i++)
    {
        // Get current region length
        wflenX = curX / 2;
        wflen = (wflenX < wflenY) ? wflenX : wflenY;

        mbmap[wfstart] = curY * picWidthInMB + curX * 2;
        mbmap[wfstart + wflen + 1] = mbmap[wfstart] + 1;

        if ((wfstart + wflen + 1) == (numMBs - 1))
            break;

        walkX = curX - 2;
        walkY = curY + 2;
        for (j = 0; j < wflen; j++)
        {
            k = wfstart + j + 1;
            mbmap[k] = walkY * picWidthInMB + walkX * 2;
            mbmap[k + wflen + 1] = mbmap[k] + 1;
            walkX -= 2;
            walkY += 2;
        }

        // Start new region
        if (curX == (picWidthInMB - 1))
        {
            // Right picture boundary reached, adjustment required
            curX--;
            curY += 2;
            wflenY--;
        }
        else
        {
            curX++;
        }
        wfstart += ((wflen + 1) * 2);
    }
}

/**************************************************************************
* MB sequence in a non-MBAFF picture with WidthInMB = 5 & HeightInMB = 5
* Raster scan pattern
*        0    1    2    3    4
*       -----------------------
*   0  | 0    1    2    3    4
*   1  | 5    6    7    8    9
*   2  | 10   11   12   13   14
*   3  | 15   16   17   18   19
*   4  | 20   21   22   23   24
***************************************************************************/
void CodechalEncodeMpeg2::MBWalkerRasterScan(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint32_t i;
    uint32_t numMBs = picWidthInMB * picHeightInMB;

    for (i = 0; i < numMBs; i++)
    {
        mbmap[i] = (uint16_t)i;
    }
}

/**************************************************************************
* MB sequence in a non-MBAFF picture with WidthInMB = 5 & HeightInMB = 5
* Vertical scan pattern
*        0    1    2    3    4
*       -----------------------
*   0  | 0    5    10   15   20
*   1  | 1    6    11   16   21
*   2  | 2    7    12   17   22
*   3  | 3    8    13   18   23
*   4  | 4    9    14   19   24
***************************************************************************/
void CodechalEncodeMpeg2::MBWalkerVerticalScan(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint32_t i, j, k;

    for (i = 0, k = 0; i < picWidthInMB; i++)
    {
        for (j = 0; j < picHeightInMB; j++)
        {
            mbmap[k++] = (uint16_t)(i + j * picWidthInMB);
        }
    }
}
