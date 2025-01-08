/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     encode_status_report_defs.h
//! \brief    Defines the common struture for encode status report
//! \details  
//!
#ifndef __ENCODE_STATUS_REPORT_DEFS_H__
#define __ENCODE_STATUS_REPORT_DEFS_H__

#include "mos_defs.h"
#include "codec_def_common.h"
#include "media_status_report.h"

namespace encode
{
enum EncodeStatusReportType
{
    statusReportGlobalCount = STATUS_REPORT_GLOBAL_COUNT,
    //RCS
    statusReportRCSStart,
    statusReportEncNormal = statusReportRCSStart,
    statusReportEncPerformance,
    statusReportEncQuality,
    statusReportEncIFrameDist,
    statusReport32xScaling,
    statusReport16xScaling,
    statusReport4xScaling,
    statusReport32xME,
    statusReport16xME,
    statusReport4xME,
    statusReportBRCInitReset,
    statusReportBRCUpdate,
    statusReportBRCBlockCopy,
    statusReportHybridPakP1,
    statusReportHybridPakP2,
    statusReportEncIFrameChroma,
    statusReportEncIFrameLuma,
    statusReportMPUFHB,
    statusReportTPUFHB,
    statusReportPACopy,
    statusReportPl2Copy,
    statusReportEvcAdv,
    statusReport2xScaling,
    statusReport32x32PUModeDecision,
    statusReport16x16PUSAD,
    statusReport16x16PUModeDecision,
    statusReport8x8PU,
    statusReport8x8PUFmode,
    statusReport32x32BIntraCheck,
    statusReportHevcBMbEnc,
    statusReportResetVLineStride,
    statusReportHevcBPak,
    statusReportHevcBRCLCUUpdate,
    statusReportMeVDEncStreamIn,
    statusReportVP9EncI32x32,
    statusReportVP9EncI16x16,
    statusReportVP9EncP,
    statusReportVP9EncTX,
    statusReportVP9DYS,
    statusReportVP9PakLumaRecon,
    statusReportVP9PakChromaRecon,
    statusReportVP9PakDeblockMask,
    statusReportVP9PakLumaDeblock,
    statusReportVP9PakChromaDeblock,
    statusReportVP9PakMcPred,
    statusReportVP9PakPFrameLumaRecon,
    statusReportVP9PakPFrameLumaRecon32x32,
    statusReportVP9PakPFrameChromaRecon,
    statusReportVP9PakPFrameIntraLumaRecon,
    statusReportVP9PakPFrameIntraChromaRecon,
    statusReportPreProc,
    statusReportEncWP,
    statusReportHevcIMbEnc,
    statusReportCSCDSCopy,
    statusReport2x4xScaling,
    statusReportHevcLCU64BMbEnc,
    statusReportMbBRCUpdate,
    statusReportStaticFrameDetection,
    statusReportHevcROI,
    statusReportSwScoreBoardInit,
    statusReportFrameStatus,
    statusReportRcsMaxNum,

    //MFX
    statusReportMfx,
    statusReportMfxBitstreamByteCountPerFrame,
    statusReportMfxBitstreamSyntaxElementOnlyBitCount,
    statusReportImageStatusMask,
    statusReportImageStatusCtrl,
    statusReportHucStatusRegMask,
    statusReportHucStatusReg,
    statusReportNumSlices,
    statusReportErrorFlags,
    statusReportBRCQPReport,
    statusReportNumberPasses,
    statusReportHeaderBytesInserted,
    statusReportQPStatusCount,
    statusReportPictureCodingType,
    statusReportLoopFilterLevel,
    statusReportImageStatusCtrlOfLastBRCPass,
    statusReportSceneChangedFlag,
    statusReportSumSquareError,
    statusReportSadLuma,
    statusReportNumIntra4x4Block,
    statusReportNumInterSkip4x4Block,
    statusReportNumSkip8x8Block,
    statusReportSliceReport,
    statusReportLpla,
    statusReportHucStatus2Reg,
    statusReportMfxMaxNum,

    statusReportMaxNum
};

struct EncoderStatusParameters
{
    uint32_t           statusReportFeedbackNumber;
    uint32_t           numberTilesInFrame;
    uint16_t           pictureCodingType;
    CODEC_PICTURE      currOriginalPic;
    CODECHAL_FUNCTION  codecFunction;
    uint8_t            numUsedVdbox;
    const void         *currRefList;
    bool               hwWalker;
    uint16_t           picWidthInMb;
    uint16_t           frameFieldHeightInMb;
    uint32_t           maxNumSlicesAllowed;
    uint32_t           av1EnableFrameObu;
    uint32_t           av1FrameHdrOBUSizeByteOffset;
    uint32_t           frameWidth;
    uint32_t           frameHeight;
    void               *pBlkQualityInfo;
};

struct ImageStatusControl
{
    union
    {
        struct
        {
            uint32_t   maxMbConformanceFlag : 1;
            uint32_t   frameBitcountFlag : 1;
            uint32_t   panic : 1;
            uint32_t   missingHuffmanCode : 1; // new addition for JPEG encode
            uint32_t   : 4;
            uint32_t   totalNumPass : 4;
            uint32_t   vdencSliceOverflowErrorOccurred : 1;
            uint32_t   numPassPolarityChange : 2;
            uint32_t   cumulativeSliceQPPolarityChange : 1;
            uint32_t   suggestedSliceQPDelta : 8;
            uint32_t   cumulativeSliceDeltaQP: 8;
        };

        struct
        {
            uint32_t   hcpLCUMaxSizeViolate : 1;
            uint32_t   hcpFrameBitCountViolateOverRun : 1;
            uint32_t   hcpFrameBitCountViolateUnderRun : 1;
            uint32_t   : 5;
            uint32_t   hcpTotalPass : 4;
            uint32_t   : 4;
            uint32_t   hcpCumulativeFrameDeltaLF: 7;
            uint32_t   : 1;
            uint32_t   hcpCumulativeFrameDeltaQP : 8;
        };

        struct
        {
            uint32_t                                 : 1;
            uint32_t avpFrameBitCountViolateOverrun  : 1;
            uint32_t avpFrameBitCountViolateUnderrun : 1;
            uint32_t                                 : 5;
            uint32_t avpTotalNumPass                 : 4;
            uint32_t                                 : 4;
            uint32_t avpCumulativeFrameDeltaLf       : 7;
            uint32_t                                 : 1;
            uint32_t avpCumulativeFrameDeltaQIndex   : 8;
        };

        struct
        {
            uint32_t   value;
        };
    };
};

struct PakNumberOfSlices
{
    // Num Slices
    union
    {
        struct
        {
            uint32_t   numberOfSlices : 16;
            uint32_t   reserved : 16;
        };

        struct
        {
            uint32_t   value;
        };
    };
};

struct BrcQpReport
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   qpPrimeY                    : 8;
            uint32_t   qpPrimeCb                   : 8;
            uint32_t   qpPrimeCr                   : 8;
            uint32_t   reserved                    : 8;
        };
        struct
        {
            uint32_t   value;
        };
    } DW0;

    // uint32_t 1 ~ 15
    struct
    {
        uint32_t   value[15];
    };
};

struct QpStatusCount
{
    union{
        struct{
            uint32_t   cumulativeQP : 24;
            uint32_t   cumulativeQPAdjust : 8;
        };

        struct
        {
            // DW0
            uint32_t   hcpCumulativeQP : 32;

            // DW1
            uint32_t   hcpFrameMinCUQp : 6;
            uint32_t   hcpFrameMaxCUQp : 6;
            uint32_t                   : 20;
        };

        struct
        {
            uint32_t avpCumulativeQP : 24;
            uint32_t                 : 8;
        };

        struct
        {
            uint32_t value[2];
        };
    };
};

struct EncodeStatusSliceReport
{
    uint32_t                        sliceSizeOverflow;
    uint8_t                         numberSlices;
    uint32_t                        sizeOfSliceSizesBuffer;
    PMOS_RESOURCE                   sliceSize;
    uint32_t                        reserved;
};

struct LookaheadReport
{
    uint32_t StatusReportNumber = 0;
    union
    {
        struct
        {
            uint32_t cqmHint : 8;
            uint32_t intraHint : 1;
            uint32_t reserved2 : 22;
            uint32_t isValid : 1;
        };
        uint32_t encodeHints = 0;
    };
    uint32_t targetFrameSize = 0;
    uint32_t targetBufferFulness = 0;
    uint32_t pyramidDeltaQP = 0;
    uint8_t  adaptive_rounding = 0;
    uint8_t  miniGopSize = 0;
    uint8_t  reserved1[2];
    uint32_t reserved3[10];
};

// the tile size record is streamed out serving 2 purposes
// in vp9 for back annotation of tile size into the bitstream
struct PakHwTileSizeRecord
{
    //DW0
    uint32_t
        Address_31_0;

    //DW1
    uint32_t
        Address_63_32;

    //DW2
    uint32_t
        Length;  // Bitstream length per tile; includes header len in first tile, and tail len in last tile

    //DW3
    uint32_t
        TileSize;  // In Vp9, it is used for back annotation, In Hevc, it is the mmio register bytecountNoHeader

    //DW4
    uint32_t
        AddressOffset;  // Cacheline offset

    //DW5
    uint32_t
        ByteOffset : 6,  //[5:0] // Byte offset within cacheline
        Res_95_70 : 26;  //[31:6]

    //DW6
    uint32_t
        Hcp_Bs_SE_Bitcount_Tile;  // bitstream size for syntax element per tile

    //DW7
    uint32_t
        Hcp_Cabac_BinCnt_Tile;  // bitstream size for syntax element per tile

    //DW8
    uint32_t
        Res_DW8_31_0;

    //DW9
    uint32_t
        Hcp_Image_Status_Ctrl;  // image status control per tile

    //DW10
    uint32_t
        Hcp_Qp_Status_Count;  // Qp status count per tile

    //DW11
    uint32_t
        Hcp_Slice_Count_Tile;  // number of slices per tile

    //DW12-15
    uint32_t
        Res_DW12_DW15[4];  // reserved bits added so that QwordDisables are set correctly
};

struct EncodeStatusMfx
{
    uint32_t                        status;                 //!< HW requires a QW aligned offset for data storage
    uint32_t                        pad;                    //!< Pad

    uint32_t                        mfcBitstreamByteCountPerFrame;         //!< Media fixed function bitstream byte count per frame
    uint32_t                        mfcBitstreamSyntaxElementOnlyBitCount; //!< Media fixed function bitstream bit count for syntax element only
    uint32_t                        imageStatusMask;        //!< MUST ENSURE THAT THIS IS QWORD ALIGNED as it's used for the conditional BB end
    ImageStatusControl              imageStatusCtrl;        //!< Used for storing the control flags for the image status
    uint32_t                        hucStatusRegMask;       //!< MUST ENSURE THAT THIS IS QWORD ALIGNED as it's used for the conditional BB end
    uint32_t                        hucStatusReg;           //!< Register value saving HuC Status
    PakNumberOfSlices               numSlices;              //!< Num of slices for encode
    uint32_t                        errorFlags;             //!< The definition is different on SNB/IVB, hence DWORD
    union
    {
        BrcQpReport     brcQPReport;      //!< Query bit rate control and QP Status
        LookaheadReport lookaheadStatus;  //!< Lookahead status. valid in lookahead pass only
    };
    uint32_t                        numberPasses;           //!< Number of passes
    uint32_t                        headerBytesInserted;    //!< The size including header, prevention bytes and dummy "0xff" inserted by SW driver
    QpStatusCount                   qpStatusCount;          //!< This is used to obtain the cumulative QP
    uint16_t                        pictureCodingType;      //!< Type of picture coding
    uint32_t                        loopFilterLevel;        //!< The level of loop filter
    ImageStatusControl              imageStatusCtrlOfLastBRCPass; //!< The level of loop filter
    uint32_t                        sceneChangedFlag;       //!< The flag indicate if the scene is changed
    uint64_t                        sumSquareError[3];      //!< The list of sum square error, luma, Cb, Cr respectively
    uint32_t                        sadLuma;                //!< Luma SAD
    uint32_t                        numIntra4x4Block;       //!< Number of intra 4x4 blocks
    uint32_t                        numInterSkip4x4Block;   //!< Number of inter and skipped 4x4 blocks
    uint32_t                        numSkip8x8Block;        //!< Number of skipped 8x8 blocks
    EncodeStatusSliceReport         sliceReport;
    uint32_t                        hucStatus2Reg;          //!< Register value saving HuC Status2
};

struct EncodeStatusRcs
{
    struct
    {
        uint32_t                    status;
        uint32_t                    pad;        //!< Pad
    } executingStatus[statusReportRcsMaxNum];   //!< Media states of stored encode data
};

struct VDEncStatusReportParam
{
    bool          vdEncBRCEnabled;
    bool          waReadVDEncOverflowStatus;
    uint32_t      mode ;
    uint32_t      vDEncBRCNumOfSliceOffset;
    PMOS_RESOURCE *resVDEncBRCUpdateDmemBufferPtr;
};
};

#endif // !__ENCODE_STATUS_REPORT_DEFS_H__