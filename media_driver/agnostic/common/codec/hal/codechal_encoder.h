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
//! \file     codechal_encoder.h
//! \brief    Defines the encode interface for CodecHal.
//! \details  The encode interface is further sub-divided by standard, this file is for the base interface which is shared by all encode standards.
//!

#ifndef __CODECHAL_ENCODER_H__
#define __CODECHAL_ENCODER_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_debug.h"
#include "codechal_encode_sfc.h"
#include "codechal_encode_csc_ds.h"
#include "codechal_mmc.h"
#include "cm_def.h"
#include "cm_wrapper.h"

//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_ENCODE sub-comp
//------------------------------------------------------------------------------
#define CODECHAL_ENCODE_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _expr)

#define CODECHAL_ENCODE_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE)

#define CODECHAL_ENCODE_CHK_STATUS(_stmt)                                               \
    MOS_CHK_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt)

#define CODECHAL_ENCODE_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_CHK_NULL(_ptr)                                                  \
    MOS_CHK_NULL(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define CODECHAL_ENCODE_CHK_NULL_NO_STATUS(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define CODECHAL_ENCODE_CHK_COND(_expr, _message, ...)                           \
    MOS_CHK_COND(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE,_expr,_message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_CHK_NULL_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define CODECHAL_ENCODE_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt)

#define CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_MODS_CHK_COND(_expr, _message, ...)                           \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE,_expr,_message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_MODS_CHK_COND_RETURN(_expr, retVal, _message, ...)            \
    MOS_CHK_COND_RETURN_VALUE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE,_expr, retVal, _message, ##__VA_ARGS__)

//!
//! \brief Recycled buffers are buffers which are locked and populated each execute
//!        to be consumed by HW. The driver loops through these buffers, if for the
//!        current index it is detected that the buffers for that index are still in
//!        use by HW, the driver waits until the buffers are available. 6 of each
//!        recycled buffer type are allocated to achieve performance parity with the
//!        old method of not having any waits.
//!
#define CODECHAL_ENCODE_RECYCLED_BUFFER_NUM             6

// Encode Sizes
#define CODECHAL_ENCODE_STATUS_NUM                      512
#define CODECHAL_ENCODE_VME_BBUF_NUM                    2
#define CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE         48
#define CODECHAL_ENCODE_BRC_PAK_STATISTICS_SIZE         64
#define CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS             CODEC_MAX_NUM_REF_FIELD
#define CODECHAL_ENCODE_SLICESIZE_BUF_SIZE              (4960 * sizeof(uint16_t))
#define CODECHAL_VDENC_BRC_NUM_OF_PASSES                2
#define CODECHAL_VDENC_NUMIMEPREDICTORS                 0x8
#define CODECHAL_DP_MAX_NUM_BRC_PASSES                  4
#define CODECHAL_CMDINITIALIZER_MAX_CMD_SIZE            CODECHAL_CACHELINE_SIZE * 4
#define CODECHAL_ENCODE_NUM_MAX_VME_L0_REF              8 // multiref - G7.5+ - used from DDI
#define CODECHAL_ENCODE_NUM_MAX_VME_L1_REF              2 // multiref - G7.5+ - used from DDI
#define CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER         (CODECHAL_ENCODE_NUM_MAX_VME_L0_REF + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
#define CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L0_REF        4 // multiref - G7.5+
#define CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L1_REF        1 // multiref - G7.5+
#define CODECHAL_HEVC_MAX_LCU_SIZE_G10                  64

// BRC
#define CODECHAL_ENCODE_BRC_KBPS                        1000     // 1000bps for disk storage, aligned with industry usage
#define CODECHAL_ENCODE_SCENE_CHANGE_DETECTED_MASK      0xffff


// User Feature Report Writeout
#define CodecHalEncode_WriteKey64(key, value)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i64Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    CodecHal_UserFeature_WriteValue(nullptr, &UserFeatureWriteData);\
}

#define CodecHalEncode_WriteKey(key, value)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i32Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    CodecHal_UserFeature_WriteValue(nullptr, &UserFeatureWriteData);\
}

#define CodecHalEncode_WriteStringKey(key, value, len)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.StringData.pStringData = (value);\
    UserFeatureWriteData.Value.StringData.uSize = (len);\
    UserFeatureWriteData.ValueID        = (key);\
    CodecHal_UserFeature_WriteValue(nullptr, &UserFeatureWriteData);\
}

// BRC Flag in BRC Init Kernel
typedef enum _CODECHAL_ENCODE_BRCINIT_FLAG
{
    CODECHAL_ENCODE_BRCINIT_ISCBR                       = 0x0010,
    CODECHAL_ENCODE_BRCINIT_ISVBR                       = 0x0020,
    CODECHAL_ENCODE_BRCINIT_ISAVBR                      = 0x0040,
    CODECHAL_ENCODE_BRCINIT_ISCQL                       = 0x0080,
    CODECHAL_ENCODE_BRCINIT_FIELD_PIC                   = 0x0100,
    CODECHAL_ENCODE_BRCINIT_ISICQ                       = 0x0200,
    CODECHAL_ENCODE_BRCINIT_ISVCM                       = 0x0400,
    CODECHAL_ENCODE_BRCINIT_IGNORE_PICTURE_HEADER_SIZE  = 0x2000,
    CODECHAL_ENCODE_BRCINIT_ISQVBR                      = 0x4000,
    CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC               = 0x8000
} CODECHAL_ENCODE_BRCINIT_FLAG;

// BRC Flag in BRC Update Kernel
typedef enum _CODECHAL_ENCODE_BRCUPDATE_FLAG
{
    CODECHAL_ENCODE_BRCUPDATE_IS_FIELD                  = 0x01,
    CODECHAL_ENCODE_BRCUPDATE_IS_MBAFF                  = (0x01 << 1),
    CODECHAL_ENCODE_BRCUPDATE_IS_BOTTOM_FIELD           = (0x01 << 2),
    CODECHAL_ENCODE_BRCUPDATE_AUTO_PB_FRAME_SIZE        = (0x01 << 3),
    CODECHAL_ENCODE_BRCUPDATE_IS_ACTUALQP               = (0x01 << 6),
    CODECHAL_ENCODE_BRCUPDATE_IS_REFERENCE              = (0x01 << 7)
} CODECHAL_ENCODE_BRCUPDATE_FLAG;

// ---------------------------
// Structures
// ---------------------------
typedef struct _CODECHAL_ENCODE_HW_COUNTER
{
	uint64_t   IV;         // Big-Endian IV
	uint64_t   Count;      // Big-Endian Block Count
}CODECHAL_ENCODE_HW_COUNTER, *PCODECHAL_ENCODE_HW_COUNTER;

typedef struct _CODECHAL_ENCODE_BBUF
{
    MHW_BATCH_BUFFER        BatchBuffer;
    uint32_t                dwSize;
    uint32_t                dwNumMbsInBBuf;
    bool                    bFieldScale;
} CODECHAL_ENCODE_BBUF, *PCODECHAL_ENCODE_BBUF;

// ---------------------------
// Enums
// ---------------------------
typedef enum _CODECHAL_ENCODE_REF_ID
{
	CODECHAL_ENCODE_REF_ID_0 = 0,
	CODECHAL_ENCODE_REF_ID_1 = 1,
	CODECHAL_ENCODE_REF_ID_2 = 2,
	CODECHAL_ENCODE_REF_ID_3 = 3,
	CODECHAL_ENCODE_REF_ID_4 = 4,
	CODECHAL_ENCODE_REF_ID_5 = 5,
	CODECHAL_ENCODE_REF_ID_6 = 6,
	CODECHAL_ENCODE_REF_ID_7 = 7,
} CODECHAL_ENCODE_REF_ID;

// supported usage
typedef enum _CODECHAL_ENCODER_MODE
{
    CODECHAL_ENCODE_NORMAL_MODE             = 0,
    CODECHAL_ENCODE_PERFORMANCE_MODE        = 1,
    CODECHAL_ENCODE_QUALITY_MODE            = 2
} CODECHAL_ENCODER_MODE;

typedef enum _CODECHAL_ENCODE_MBBRC_SETTING
{
    CODECHAL_ENCODE_MBBRC_INTERNAL          = 0,
    CODECHAL_ENCODE_MBBRC_ENABLED           = 1,
    CODECHAL_ENCODE_MBBRC_DISABLED          = 2,
} CODECHAL_ENCODE_MBBRC_SETTING;

// used in GetNumBrcPakPasses
typedef enum _CODECHAL_ENCODE_BRC_NUM_PASSES
{
    CODECHAL_ENCODE_BRC_SINGLE_PASS         = 1, // No IPCM case
    CODECHAL_ENCODE_BRC_MINIMUM_NUM_PASSES  = 2, // 2 to cover IPCM case
    CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES  = 4,
    CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES  = 7
} CODECHAL_ENCODE_BRC_NUM_PASSES;

typedef enum _CODECHAL_ENCODE_TRELLIS_SETTING
{
    CODECHAL_ENCODE_TRELLIS_INTERNAL        = 0,
    CODECHAL_ENCODE_TRELLIS_DISABLED        = 1,
    CODECHAL_ENCODE_TRELLIS_ENABLED_I       = 2,
    CODECHAL_ENCODE_TRELLIS_ENABLED_P       = 4,
    CODECHAL_ENCODE_TRELLIS_ENABLED_B       = 8
} CODECHAL_ENCODE_TRELLIS_SETTING;


typedef struct _CODECHAL_ENCODE_MEDIA_OBJECT_INLINE_DATA
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   MBHorizontalOrigin  : 8;    // in MB unit
            uint32_t   MBVerticalOrigin    : 8;    // in MB unit
            uint32_t                       : 16;
        };
        // For BRC Block Copy kernels
        struct
        {
            uint32_t   BlockHeight     : 16;
            uint32_t   BufferOffset    : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;
} CODECHAL_ENCODE_MEDIA_OBJECT_INLINE_DATA;

typedef struct _CODECHAL_ENCODE_ID_OFFSET_PARAMS
{
    uint32_t                                Standard;
    CODECHAL_MEDIA_STATE_TYPE               EncFunctionType;
    uint16_t                                wPictureCodingType;
    uint8_t                                 ucDmvPredFlag;
    bool                                    bInterlacedField;
} CODECHAL_ENCODE_ID_OFFSET_PARAMS, *PCODECHAL_ENCODE_ID_OFFSET_PARAMS;

typedef struct _CODECHAL_ENCODE_SEND_KERNEL_CMDS_PARAMS
{
    CODECHAL_MEDIA_STATE_TYPE           EncFunctionType;
    uint32_t                            uiDshIdx;
    uint8_t                             ucDmvPredFlag;
    bool                                bBrcResetRequested;
    bool                                bEnableScoreboardSettings;
    bool                                bEnable45ZWalkingPattern;
    bool                                bEnableCustomScoreBoard;
    PMHW_VFE_SCOREBOARD                 pCustomScoreBoard;
    bool                                bEnableVp9HybridPakScoreboardDependency;
    PMHW_KERNEL_STATE                   pKernelState;
    bool                                bAdvancedDshInUse;
} CODECHAL_ENCODE_SEND_KERNEL_CMDS_PARAMS, *PCODECHAL_ENCODE_SEND_KERNEL_CMDS_PARAMS;

/********************************************************************
    ------------------------------------------
    |  bit 3      | bit 2   | bit 1  | bit 0  |
    ------------------------------------------
    |Conversion   | 16x bit | 4x bit | 2x bit |
    ------------------------------------------
*********************************************************************/

typedef struct _CODECHAL_ENCODE_BINDING_TABLE_GENERIC
{
    uint32_t   dwMediaState;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
    uint32_t   dwBindingTableEntries[64];
} CODECHAL_ENCODE_BINDING_TABLE_GENERIC, *PCODECHAL_ENCODE_BINDING_TABLE_GENERIC;

typedef struct _CODECHAL_ENCODE_BUFFER
{
    MOS_RESOURCE    sResource;
    uint32_t        dwSize;
} CODECHAL_ENCODE_BUFFER, *PCODECHAL_ENCODE_BUFFER;

//Query Status struct
typedef struct _CODECHAL_ENCODE_BRC_QP_REPORT
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   QPPrimeY                    : 8;
            uint32_t   QPPrimeCb                   : 8;
            uint32_t   QPPrimeCr                   : 8;
            uint32_t   Reserved                    : 8;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1 ~ 15
    struct
    {
        uint32_t   Value[15];
    };
} CODECHAL_ENCODE_BRC_QP_REPORT, *PCODECHAL_ENCODE_BRC_QP_REPORT;

typedef struct _CODECHAL_ENCODE_BINDING_TABLE_RESET_VLINE_STRIDE
{
    uint32_t   dwResetVLineStrideBuffer;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
} CODECHAL_ENCODE_BINDING_TABLE_RESET_VLINE_STRIDE, *PCODECHAL_ENCODE_BINDING_TABLE_RESET_VLINE_STRIDE;

/* \brief Tile info report to application
*/
typedef struct _CODECHAL_TILE_INFO
{
    uint16_t    TileRowNum;
    uint16_t    TileColNum;
    uint32_t    TileBitStreamOffset;
    uint32_t    TileSizeInBytes;

    uint32_t    reserved;
    uint32_t    HWCounterValue;
} CODECHAL_TILE_INFO, *PCODECHAL_TILE_INFO;

/*! \brief Information pertaining to a particular picture's encode operation.
*/
typedef struct _CODECHAL_ENCODE_STATUS_REPORT
{
    CODECHAL_STATUS                 CodecStatus;            //!< Status for the picture associated with this status report
    uint32_t                        StatusReportNumber;     //!< Status report number associated with the picture in this status report provided in Codechal::Execute()
    CODEC_PICTURE                   CurrOriginalPic;        //!< Uncompressed frame information for the picture associated with this status report
    CODECHAL_ENCODE_FUNCTION_ID     Func;                   //!< Encode function requested at Codechal::Execute()
    PCODEC_REF_LIST                 pCurrRefList;           //!< Reference list for the current frame, used for dump purposes with CodecHal Debug Tool
    /*! \brief Specifies the order in which the statuses are expected.
    *
    *   The order in which a status is returned is requested at the DDI level and the order itself is determined by StatusReportNumber.
    *       false indicates the statuses should be returned in reverse order.
    *       true indicates the statuses should be returned in sequential order.
    */
    bool                            bSequential;
    /*! \brief Coded bitstream size reported by HW.
    *
    *   The size reported by HW is the total bitstream size that is encoded by HW including any bitstream buffer overrun.  That is, HW continues counting the encoded bytes past the programmed upperbound based on the allocated bitstream buffer size.  The framework can compare this value to the allocated buffer size to determine if there was overflow for this frame and can act accordingly.
    */
    uint32_t                        bitstreamSize;
    /*! \brief Qp value for Y used for the first PAK pass.
    *
    *   It is not valid if CQP is set by framework.
    */
    char                            QpY;
    /*! \brief Suggested Qp delta value for Y.
    *
    *   Framework can add this delta Qp with the first pass QpY to get the final Qp used for multi-pass.  It is not valid if CQP is set by framework.
    *   Note: Framework can use this reported QpY and suggestedQpYDelta to set QpY in picture parameter to minimize LCU level Qp delta.
    */
    char                            SuggestedQpYDelta;
    uint8_t                         NumberPasses;       //!< Number of PAK passes executed.
    uint8_t                         AverageQp;          //!< The average QP of all MBs or LCUs of the frame.
    CODECHAL_ENCODE_HW_COUNTER      HWCounterValue;

    union
    {
        struct
        {
            uint32_t PanicMode              : 1;    //!< Indicates that panic mode was triggered by HW for this frame.
            uint32_t SliceSizeOverflow      : 1;    //!< When SliceLevelRateCtrl is used, indicates the requested slice size was not met for one or more generated slices.
            uint32_t NumSlicesNonCompliant  : 1;    //!< When SliceLevelRateCtrl is used, indicates whether or not the number of generated slices exceeds specification limits.
            uint32_t LongTermReference      : 1;
            uint32_t FrameSkipped           : 1;
            uint32_t SceneChangeDetected    : 1;
            uint32_t                        : 26;
        };
        uint32_t QueryStatusFlags;
    };
    /*! \brief The average MAD (Mean Absolute Difference) across all macroblocks in the Y plane.
    *
    *    The MAD value is the mean of the absolute difference between the pixels in the original block and the corresponding pixels in the block being used for comparison, from motion compensation or intra spatial prediction. MAD reporting is disabled by default.
    */
    uint32_t                        MAD;
    uint32_t                        loopFilterLevel;        //!< [VP9]
    char                            LongTermIndication;     //!< [VP9]
    uint16_t                        NextFrameWidthMinus1;   //!< [VP9]
    uint16_t                        NextFrameHeightMinus1;  //!< [VP9]
    uint8_t                         NumberSlices;           //!< Number of slices generated for the frame.
    uint16_t                        PSNRx100[3];            //!< PSNR for different channels
    uint32_t                        NumberTilesInFrame;     //!< Number of tiles generated for the frame.
    uint8_t                         UsedVdBoxNumber;
    uint32_t                        SizeOfSliceSizesBuffer;
    uint16_t                       *pSliceSizes;
    uint32_t                        SizeOfTileInfoBuffer;
    PCODECHAL_TILE_INFO             pHEVCTileinfo;
    uint32_t                        NumTileReported;
} CODECHAL_ENCODE_STATUS_REPORT, *PCODECHAL_ENCODE_STATUS_REPORT;

typedef struct _CODECHAL_ENCODE_STATUS
{
    uint32_t                        dwStoredDataMfx;    // HW requires a QW aligned offset for data storage
    uint32_t                        dwPad;
    struct
    {
        uint32_t                    dwStoredData;
        uint32_t                    dwPad;
    } qwStoredDataEnc[CODECHAL_NUM_MEDIA_STATES];
    uint32_t                        dwStoredData;       // SW
    uint32_t                        dwMFCBitstreamByteCountPerFrame;
    uint32_t                        dwMFCBitstreamSyntaxElementOnlyBitCount;
    uint32_t                        dwPad2;
    uint32_t                        dwImageStatusMask;  // MUST ENSURE THAT THIS IS uint64_t ALIGNED as it's used for the conditional BB end
    MHW_VDBOX_IMAGE_STATUS_CONTROL  ImageStatusCtrl;
    uint32_t                        HuCStatusRegMask;  // MUST ENSURE THAT THIS IS uint64_t ALIGNED as it's used for the conditional BB end
    uint32_t                        HuCStatusReg;
    MHW_VDBOX_PAK_NUM_OF_SLICES     NumSlices;
    uint32_t                        dwErrorFlags;       // The definition is different on SNB/IVB, hence uint32_t
    CODECHAL_ENCODE_BRC_QP_REPORT   BrcQPReport;
    uint32_t                        dwNumberPasses;
    uint32_t                        dwHeaderBytesInserted;  // The size including header, prevention bytes and dummy "0xff" inserted by SW driver
    CodechalQpStatusCount           QpStatusCount;          // This is used to obtain the cumulative QP
    CODECHAL_ENCODE_STATUS_REPORT   EncodeStatusReport;
    uint16_t                        wPictureCodingType;
    uint32_t                        LoopFilterLevel;
    MHW_VDBOX_IMAGE_STATUS_CONTROL  ImageStatusCtrlOfLastBRCPass;
    uint32_t                        dwSceneChangedFlag;
    uint64_t                        sumSquareError[3];
}CODECHAL_ENCODE_STATUS, *PCODECHAL_ENCODE_STATUS;

typedef struct _CODECHAL_ENCODE_STATUS_BUFFER
{
    uint8_t*                                pEncodeStatus;                  // needs to be first to ensure alingment of dwStoredDataMfx/Vme
    MOS_RESOURCE                            resStatusBuffer;                // Handle of eStatus buffer
    uint32_t*                               pData;
    uint16_t                                wFirstIndex;
    uint16_t                                wCurrIndex;
    uint32_t                                dwStoreDataOffset;
    uint32_t                                dwBSByteCountOffset;
    uint32_t                                dwBSSEBitCountOffset;
    uint32_t                                dwImageStatusMaskOffset;
    uint32_t                                dwImageStatusCtrlOffset;
    uint32_t                                dwHuCStatusMaskOffset;
    uint32_t                                dwHuCStatusRegOffset;
    uint32_t                                dwNumSlicesOffset;
    uint32_t                                dwErrorFlagOffset;
    uint32_t                                dwBRCQPReportOffset;
    uint32_t                                dwNumPassesOffset;
    uint32_t                                dwQpStatusCountOffset;
    uint32_t                                dwImageStatusCtrlOfLastBRCPassOffset;
    uint32_t                                dwSceneChangedOffset;
    uint32_t                                dwSumSquareErrorOffset;
    uint32_t                                dwSize;
    uint32_t                                dwReportSize;
} CODECHAL_ENCODE_STATUS_BUFFER, *PCODECHAL_ENCODE_STATUS_BUFFER;

typedef struct _CODECHAL_ENCODE_ATOMIC_SCRATCH_BUFFER
{
    MOS_RESOURCE                            resAtomicScratchBuffer;     // Handle of eStatus buffer
    uint32_t*                               pData;
    uint16_t                                wEncodeUpdateIndex;         // used for VDBOX update encode status
    uint16_t                                wReservedIndex;             // Reserved for future extension
    uint32_t                                dwZeroValueOffset;          // Store the result of the ATOMIC_CMP
    uint32_t                                dwOperand1Offset;           // Operand 1 of the ATOMIC_CMP
    uint32_t                                dwOperand2Offset;           // Operand 2 of the ATOMIC_CMP
    uint32_t                                dwOperand3Offset;           // Copy of the operand 1

    uint32_t                                dwSize;
    uint32_t                                dwOperandSetSize;
}CODECHAL_ENCODE_ATOMIC_SCRATCH_BUFFER, *PCODECHAL_ENCODE_ATOMIC_SCRATCH_BUFFER;

typedef struct _CODECHAL_ENCODE_READ_BRC_PAK_STATS_PARAMS
{
    CodechalHwInterface    *pHwInterface;
    PMOS_RESOURCE           presBrcPakStatisticBuffer;
    PMOS_RESOURCE           presStatusBuffer;
    uint32_t                dwStatusBufNumPassesOffset;
    uint8_t                 ucPass;
    MOS_GPU_CONTEXT         VideoContext;
} CODECHAL_ENCODE_READ_BRC_PAK_STATS_PARAMS, *PCODECHAL_ENCODE_READ_BRC_PAK_STATS_PARAMS;

typedef struct _CODECHAL_VDENC_BRC_PAKMMIO
{
    uint32_t                dwReEncode[4];
} CODECHAL_VDENC_BRC_PAKMMIO, *PCODECHAL_VDENC_BRC_PAKMMIO;

typedef struct _CODECHAL_ENCODE_SEI_DATA
{
	bool       bNewSEIData;
	uint32_t   dwSEIDataSize;
	uint32_t   dwSEIBufSize;
	uint8_t*   pSEIBuffer;
} CODECHAL_ENCODE_SEI_DATA, *PCODECHAL_ENCODE_SEI_DATA;

/* \brief Parameters passed in via Execute() to perform encoding.
*/

typedef struct HUC_COM_DMEM
{
    uint32_t    OutputSize;               // Total size in byte of the Output SLB
    uint32_t    TotalOutputCommands;      // Total Commands in the output SLB
    uint8_t     TargetUsage;              // TU number
    uint8_t     Codec;                    // 0-HEVC VDEnc; 1-VP9 VDEnc; 2-AVC VDEnc
    uint8_t     FrameType;                // 0-I Frame; 1-P Frame; 2-B Frame
    uint8_t     Reserved[37];             //
    struct
    {
        uint16_t    StartInBytes;        // Command starts offset in bytes in Output SLB
        uint8_t     ID;                  // Command ID
        uint8_t     Type;                // Command Type
        uint32_t    BBEnd;
    } OutputCOM[50];
} HUC_COM_DMEM,*PHUC_COM_DMEM;

typedef struct _HUC_COM_DATA
{
    uint32_t        TotalCommands;       // Total Commands in the Data buffer
    struct
    {
        uint16_t    ID;              // Command ID, defined and order must be same as that in DMEM
        uint16_t    SizeOfData;      // data size in uint32_t
        uint32_t    data[40];
    } InputCOM[50];
} HUC_COM_DATA, *PHUC_COM_DATA;

typedef struct _HUC_INPUT_CMD1
{
    // Shared
    uint32_t FrameWidthInMinCbMinus1;
    uint32_t FrameHeightInMinCbMinus1;
    uint32_t log2_min_coding_block_size_minus3;
    uint8_t  VdencStreamInEnabled;
    uint8_t  PakOnlyMultipassEnable;
    uint16_t num_ref_idx_l0_active_minus1;
    uint16_t SADQPLambda;
    uint16_t RDQPLambda;

    // HEVC
    uint16_t num_ref_idx_l1_active_minus1;
    uint8_t  RSVD0;
    uint8_t  ROIStreamInEnabled;
    int8_t   ROIDeltaQp[8]; // [-3..3] or [-51..51]
    uint8_t  FwdPocNumForRefId0inL0;
    uint8_t  FwdPocNumForRefId0inL1;
    uint8_t  FwdPocNumForRefId1inL0;
    uint8_t  FwdPocNumForRefId1inL1;
    uint8_t  FwdPocNumForRefId2inL0;
    uint8_t  FwdPocNumForRefId2inL1;
    uint8_t  FwdPocNumForRefId3inL0;
    uint8_t  FwdPocNumForRefId3inL1;
    uint8_t  EnableRollingIntraRefresh;
    int8_t   QpDeltaForInsertedIntra;
    uint16_t IntraInsertionSize;
    uint16_t IntraInsertionLocation;
    int8_t   QpY;
    uint8_t  RoundingEnabled;
    uint8_t  UseDefaultQpDeltas;
    uint8_t  PanicEnabled;
    uint8_t  RSVD[2];

    // VP9
    uint16_t DstFrameWidthMinus1;
    uint16_t DstFrameHeightMinus1;
    uint8_t  SegmentationEnabled;
    uint8_t  PrevFrameSegEnabled;
    uint8_t  SegMapStreamInEnabled;
    uint8_t  LumaACQIndex;
    int8_t   LumaDCQIndexDelta;
    uint8_t  RESERVED[3];
    int16_t  SegmentQIndexDelta[8];
} HUC_INPUT_CMD1, *PHUC_INPUT_CMD1;

typedef struct _HUC_INPUT_CMD2
{
    uint32_t SADQPLambda;     // HEVC/VP9
} HUC_INPUT_CMD2, *PHUC_INPUT_CMD2;

typedef struct _HUC_PAK_STITCH_DMEM
{
    uint32_t     TileSizeRecord_offset[5];  // Tile Size Records, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VDENCSTAT_offset[5];	  // needed for HEVC VDEnc, VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_PAKSTAT_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     HEVC_Streamout_offset[5]; //needed for HEVC VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     VP9_PAK_STAT_offset[5]; //needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     Vp9CounterBuffer_offset[5];	//needed for VP9 VDEnc, start offset  in byte, 0xffffffff means unavailable
    uint32_t     LastTileBS_StartInBytes;// last tile in bitstream for region 4 and region 5
    uint16_t     TotalSizeInCommandBuffer; // Total size in bytes of valid data in the command buffer
    uint16_t     OffsetInCommandBuffer; // Byte  offset of the to-be-updated Length (uint32_t) in the command buffer, 0xffff means unavailable
    uint16_t     PicWidthInPixel;   // Picture width in pixel
    uint16_t     PicHeightInPixel;  // Picture hieght in pixel
    uint16_t     TotalNumberOfPAKs; // [2..4]
    uint16_t     NumSlices[4];  // this is number of slices from each PAK
    uint16_t     NumTiles[4];  // this is number of tiles from each PAK
    uint16_t     PIC_STATE_StartInBytes;// offset for  region 7 and region 8
    uint8_t      Codec;             // 1: HEVC DP; 2: HEVC VDEnc; 3: VP9 VDEnc
    uint8_t      MAXPass;           // Max number of BRC pass >=1
    uint8_t      CurrentPass;       // Current BRC pass [1..MAXPass]
    uint8_t      MinCUSize;      // Minimum CU size (3: 8x8, 4:16x16), HEVC only.
    uint8_t      CabacZeroWordFlag; // cabac zero flag, HEVC only
    uint8_t      bitdepth_luma;     // luma bitdepth, HEVC only
    uint8_t      bitdepth_chroma;   // chroma bitdepth, HEVC only
    uint8_t      ChromaFormatIdc;   // chroma format idc, HEVC only
    uint8_t      RSVD[30];	//mbz
} HUC_PAK_STITCH_DMEM, *PHUC_PAK_STITCH_DMEM;

class CodechalEncodeWP;

//-----------------------------------------------------------------------------
// CODECHAL encoder state
//-----------------------------------------------------------------------------
struct _CODECHAL_ENCODER
{
    _CODECHAL_ENCODER()
    {
        pHwInterface = nullptr;
        pOsInterface = nullptr;
        pvStandardState = nullptr;
    }
    // Encoder private data
    CodechalHwInterface            *pHwInterface;
    PMOS_INTERFACE                  pOsInterface;
    CodechalDebugInterface*         pDebugInterface;

    PLATFORM                        Platform;
    MEDIA_FEATURE_TABLE             *pSkuTable;
    MEDIA_WA_TABLE                  *pWaTable;
    CodecHalMmcState                *pMmc;
    MEDIA_SYSTEM_INFO               *pGtSystemInfo;
    MOS_GPU_NODE                    VideoGpuNode;
    MOS_GPU_CONTEXT                 VideoContext;
    MOS_GPU_CONTEXT                 VideoContextExt[4];
    MOS_GPU_CONTEXT                 RenderContext;
    bool                            bPakEnabled;
    bool                            bEncEnabled;
    bool                            bVideoNodeAssociationCreated;
    bool                            ComputeContextEnabled;

    void*                           pvStandardState;        // Standard Specific State
    CodechalEncodeCscDs*            m_cscDsState;           // pointer to CSC Downscaling state
    CodechalEncodeWP*               m_wpState;              // pointer to weighted prediction state
    CODECHAL_FUNCTION               CodecFunction;
    uint32_t                        Standard;
    uint32_t                        Mode;
    MHW_WALKER_MODE                 WalkerMode;
    uint8_t                         ucKernelMode;           // normal, performance, quality.
    bool                            bModsEnabled;

    bool                            bMfeEnabled;                // Mfe enabled
    bool                            bMfeMbEncEanbled;           // Mfe MBEnc kernel enabled
    bool                            bMfeLastStream;             // Is last stream during this submission
    bool                            bMfeFirstStream;            // Is first stream during this submission
    bool                            bMfeInitialized;            // Used for initializing MFE resources during first execute

                                                                // Combined Kernel Parameters
    uint8_t*                        pui8KernelBase;
    uint32_t                        dwKUID;

    // Per-frame Application Settings
    uint32_t*                       pdwDataHWCount;
    MOS_RESOURCE                    resHwCount;
    MOS_SURFACE                     sRawSurface;                // Pointer to MOS_SURFACE of raw surface
    MOS_SURFACE                     sReconSurface;              // Pointer to MOS_SURFACE of reconstructed surface
    MOS_RESOURCE                    resBitstreamBuffer;         // Pointer to MOS_SURFACE of bitstream surface
    MOS_RESOURCE                    resMbCodeSurface;           // Pointer to MOS_SURFACE of MbCode surface
    MOS_RESOURCE                    resMvDataSurface;           // Pointer to MOS_SURFACE of MvData surface
    uint32_t                        dwMbDataBufferSize;

    CODEC_PICTURE                   CurrOriginalPic;        // Raw.
    CODEC_PICTURE                   CurrReconstructedPic;   // Recon.
    uint16_t                        wPictureCodingType;     // I, P, or B frame
    int16_t                         sFrameNum;
    bool                            bFirstFrame;
    bool                            bFirstTwoFrames;
    bool                            bFirstField;
    bool                            bResolutionChanged;
    bool                            bScalingEnabled;        // 4x Scaling kernel needs to be called for this frame
    bool                            b2xScalingEnabled;      // 2x Scaling kernel only used by HEVC now
    bool                            bUseRawForRef;
    bool                            bDisableReconMMCD;      // disable Recon surface's MMC
    uint8_t                         ucPrevReconFrameIdx;
    uint8_t                         ucCurrReconFrameIdx;

    uint32_t                        dwFrameWidth;           // Frame width in luma samples
    uint32_t                        dwFrameHeight;          // Frame height in luma samples
    uint32_t                        dwFrameFieldHeight;     // Frame height in luma samples
    uint32_t                        dwOriFrameHeight;
    uint32_t                        dwOriFrameWidth;
    uint16_t                        wPicWidthInMB;          // Picture Width in MB width count
    uint16_t                        wPicHeightInMB;         // Picture Height in MB height count
    uint16_t                        wFrameFieldHeightInMB;  // Frame/field Height in MB
    uint32_t                        dwBitstreamUpperBound;
    uint8_t                         ucOriFieldCodingFlag;
    uint32_t                        dwMaxBtCount;
    bool                            bCmKernelEnable;
    bool                            bFeiEnable;

    // Synchronization
    MOS_RESOURCE                    resSyncObjectRenderContextInUse;
    MOS_RESOURCE                    resSyncObjectVideoContextInUse;
    bool                            bWaitForPAK;
    bool                            bSignalENC;
    uint32_t                        uiSemaphoreObjCount;
    uint32_t                        uiSemaphoreMaxCount;

    // Status Reporting
    uint32_t                        dwStoreData;
    bool                            bStatusQueryReportingEnabled;       // Flag to indicate if we support eStatus query reporting on current Platform
    CODECHAL_ENCODE_STATUS_BUFFER   encodeStatusBuf;                    // Stores all the status_query related data for PAK engine
    CODECHAL_ENCODE_STATUS_BUFFER   encodeStatusBufRcs;                 // Stores all the status_query related data for render ring (RCS)
    MHW_VDBOX_IMAGE_STATUS_CONTROL  ImgStatusControlBuffer;             // Stores image eStatus control register data
    uint32_t                        uiStatusReportFeedbackNumber;
    bool                            bFrameTrackingEnabled;           // Flag to indicate if we enable frame tracking
    uint32_t                        uiNumberTilesInFrame;
    bool                            bInlineEncodeStatusUpdate;          // check whether use inline encode status update or seperate BB
    CODECHAL_ENCODE_ATOMIC_SCRATCH_BUFFER atomicScratchBuf;             // Stores atomic operands and result

    // Shared Parameters
    BSBuffer                        bsBuffer;
    bool                            bNewSeqHeader;
    bool                            bNewPPSHeader;
    bool                            bNewVuiData;
    bool                            bNewSeq;
    bool                            bLastPicInSeq;
    bool                            bLastPicInStream;
    uint8_t                         ucNumRefPair;               // number of reference pair (forward & backward)
    uint8_t                         ucNumPasses;
    uint8_t                         ucCurrPass;
    bool                            ForceSinglePakPass;
    bool                            bUseCmScalingKernel;
    bool                            bUseMwWlkrForAsmScalingKernel;
    bool                            bCombinedDownScaleAndDepthConversion;
    uint32_t                        dwBrcPakStatisticsSize;
    uint32_t                        dwBrcHistoryBufferSize;
    uint32_t                        dwMbEncBRCBufferSize;
    uint8_t                         ucNumVDBOX;
    uint8_t                         ucNumUsedVDBOX;

    // ENC/PAK batch buffer and surface indices
    PCODEC_REF_LIST                 pCurrRefList;
    uint8_t                         ucCurrRecycledBufIdx;
    uint8_t                         ucCurrEncBbSet;
    uint8_t                         ucCurrMbCodeIdx;
    uint8_t                         ucCurrScalingIdx;
    uint8_t                         ucCurrMADBufferIdx;
    uint32_t                        dwRecycledBufStatusNum[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    uint32_t                        dwRecycledBufWaitMs;

    // User Feature Key Capabilities
    bool                            bHmeSupported;
    bool                            bNoMEKernelForPFrame;   // HEVC does not have P-frame, no need to load P-frame ME kernel
    bool                            b16xMeSupported;
    bool                            b16xMeUserFeatureKeyControl;
    bool                            b32xMeSupported;
    bool                            b32xMeUserFeatureKeyControl;
    bool                            b2xMeSupported;         // 2x DS surface, currently only used by Gen10 Hevc Encode
    bool                            bSuppressReconPicSupported;
    bool                            bUseHwScoreboard;
    bool                            bHWWalker;
    bool                            bRCPanicEnable;
    bool                            bSliceShutdownEnable;   // HSW Slice Shutdown Enable
    uint32_t                        dwEncodeVFEMaxThreads;
    uint32_t                        dwEncodeVFEMaxThreadsScaling;
    uint32_t                        dwHalfSliceSelect;
    uint32_t                        dwHWScoreboardType;
    bool                            bFlatnessCheckSupported;
    bool                            bColorbitSupported;
    bool                            bGroupIdSelectSupported;
    uint8_t                         ucGroupId;
    bool                            bRepakSupported;
    bool                            bMultipassBrcSupported;
    uint8_t                         ucTargetUsageOverride;
    bool                            bUserFeatureKeyReport;

    // CmdInitializer HuC FW for HEVC/VP9 VDEnc
    bool                            bCmdInitializerHucUsed;
    MHW_BATCH_BUFFER                CmdInitializer2ndLevelBatchBufferPass1;
    MHW_BATCH_BUFFER                CmdInitializer2ndLevelBatchBufferPass2;

    MOS_RESOURCE                    resVdencCmdInitializerDmemBuffer;
    MOS_RESOURCE                    resVdencCmdInitializerDataBuffer;

    // VDEnc params
    bool                            bVdencEnabled;
    bool                            bVdencBrcEnabled;
    bool                            bVdencStreamInEnabled;
    bool                            bVdencNoTailInsertion;
    uint32_t                        dwVdencBrcStatsBufferSize;
    uint32_t                        dwVdencBrcPakStatsBufferSize;
    uint32_t                        dwVdencBrcNumOfSliceOffset;
    bool                            bWaReadVDEncOverflowStatus;
    bool                            bVdencBrcImgStatAllocated;

    // VDEnc dynamic slice control params
    uint32_t                        dwMbSlcThresholdValue;
    uint32_t                        dwSliceThresholdTable;
    uint32_t                        dwVdencFlushDelayCount;
    uint32_t                        dwVdencSliceMinusI;
    uint32_t                        dwVdencSliceMinusP;

    HMODULE                         hSwBrcMode;

    // IQmatrix params
    bool                            bPicQuant;
    bool                            bNewQmatrixData;
    PCODEC_ENCODER_SLCDATA          pslcData;      // record slice header size & position
    uint32_t                        dwNumSlices;
    uint32_t                        dwNumHuffBuffers;

    // ENC input/output buffers
    uint32_t                        ulMbCodeStrideInDW;         // Offset + Size of MB + size of MV
    uint32_t                        ulMbCodeOffset;             // MB data offset
    uint32_t                        ulMVOffset;                 // MV data offset, in 64 byte
    uint32_t                        ulMBCodeSize;               // MB code buffer size
    uint32_t                        ulMBcodeBottomFieldOffset;  // MB code offset frame/TopField - zero, BottomField - nonzero
    uint32_t                        ulMVDataSize;               // MV data size
    uint32_t                        ulMVBottomFieldOffset;      // MV data offset frame/TopField - zero, BottomField - nonzero
    MOS_RESOURCE                    resDistortionBuffer;        // MBEnc Distortion Buffer
    MOS_RESOURCE                    resMADDataBuffer[CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS]; // Buffers to store Mean of Absolute Differences
    bool                            bMADEnabled;

    bool                            bArbitraryNumMbsInSlice;                        // Flag to indicate if the sliceMapSurface needs to be programmed or not
    MOS_SURFACE                     sSliceMapSurface[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    uint32_t                        ulSliceMapBottomFieldOffset;

    MHW_KERNEL_STATE                ResetVLineStrideKernelState;
    bool                            bResetVLineStrideForMbEnc;
    MOS_SURFACE                     sResetVLineStrideBuffer;  // Buffer allocated to clear vertical line stride stuck in Render cache

    // VDENC and PAK Data Buffer
    PMOS_RESOURCE                   presVdencStatsBuffer;
    PMOS_RESOURCE                   presVdencCuObjStreamOutBuffer;
    PMOS_RESOURCE                   presVdencPakObjCmdStreamOutBuffer;
    PMOS_RESOURCE                   presPakStatsBuffer;
    PMOS_RESOURCE                   presSliceCountBuffer;
    PMOS_RESOURCE                   presVdencModeTimerBuffer;

    // VDEnc StreamIn Buffer
    MOS_RESOURCE                    resVDEncStreamInBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];

    // Maximum number of slices allowed by video spec
    uint32_t                        dwMaxNumSlicesAllowed;

    //VDEnc HuC FW status
    MOS_RESOURCE                    resPakMmioBuffer;
    MOS_RESOURCE                    resHuCStatus2Buffer;
    MOS_RESOURCE                    resHuCFwBuffer;
    PMOS_RESOURCE                   presVdencBrcUpdateDmemBuffer[2]; // One for 1st pass of next frame, and the other for the next pass of current frame.

    // PAK Scratch Buffers
    MOS_RESOURCE                    resDeblockingFilterRowStoreScratchBuffer;               // Handle of deblock row store surface
    MOS_RESOURCE                    resMPCRowStoreScratchBuffer;                            // Handle of mpc row store surface
    MOS_RESOURCE                    resStreamOutBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];    // Handle of streamout data surface

    // Scaling
    MHW_KERNEL_STATE                Scaling4xKernelStates[CODEC_NUM_FIELDS_PER_FRAME];
    ScalingBindingTable             Scaling4xBindingTable;
    MHW_KERNEL_STATE                Scaling2xKernelStates[CODEC_NUM_FIELDS_PER_FRAME];
    ScalingBindingTable             Scaling2xBindingTable;
    uint32_t                        dwScalingCurbeSize;
    bool                            bInterlacedFieldDisabled;
    CODECHAL_ENCODE_BBUF            scalingBBUF[CODECHAL_ENCODE_VME_BBUF_NUM];          // This Batch Buffer is used for scaling kernel.
    uint32_t                        ulScaledBottomFieldOffset;
    uint32_t                        ulScaled16xBottomFieldOffset;
    uint32_t                        ulScaled32xBottomFieldOffset;
    uint32_t                        dwScalingBBufIdx;
    uint8_t                         uiMinScaledDimension;
    uint8_t                         uiMinScaledDimensionInMb;
    uint32_t                        dwDownscaledWidth2x;    // currently only used by Gen10 HEVC
    uint32_t                        dwDownscaledHeight2x;
    uint32_t                        dwDownscaledWidth4x;
    uint32_t                        dwDownscaledHeight4x;
    uint32_t                        dwDownscaledWidthInMb4x;
    uint32_t                        dwDownscaledHeightInMb4x;
    uint32_t                        dwDownscaledFrameFieldHeightInMb4x;
    uint32_t                        dwDownscaledWidth16x;
    uint32_t                        dwDownscaledHeight16x;
    uint32_t                        dwDownscaledWidthInMb16x;
    uint32_t                        dwDownscaledHeightInMb16x;
    uint32_t                        dwDownscaledFrameFieldHeightInMb16x;
    uint32_t                        dwDownscaledWidth32x;
    uint32_t                        dwDownscaledHeight32x;
    uint32_t                        dwDownscaledWidthInMb32x;
    uint32_t                        dwDownscaledHeightInMb32x;
    uint32_t                        dwDownscaledFrameFieldHeightInMb32x;

    bool                            bFieldScalingOutputInterleaved;
    MOS_SURFACE                     sFlatnessCheckSurface;
    uint32_t                        ulFlatnessCheckBottomFieldOffset;
    bool                            bFlatnessCheckEnabled;
    bool                            bMbStatsEnabled;
    uint8_t                         bAdaptiveTransformDecisionEnabled;
    bool                            bForceBrcMbStatsEnabled;
    uint32_t                        dwMBVProcStatsBottomFieldOffset;
    CODECHAL_ENCODE_BUFFER          resMbStatisticsSurface;
    bool                            bMbStatsSupported;
    MOS_RESOURCE                    resMbStatsBuffer;
    uint32_t                        dwMbStatsBottomFieldOffset;

    // ME
    bool                            bUseNonLegacyStreamin;

    // tracked buffer
    uint8_t                         ucTrackedBufCurrIdx;        // current tracked buffer index when ring buffer is used
    uint8_t                         ucTrackedBufCountNonRef;    // counting number of tracked buffer when ring buffer is used
    uint8_t                         ucTrackedBufCountResize;    // 3 buffers to be delay-destructed during res change
    uint8_t                         ucTrackedBufLastIdx;        // last tracked buffer index
    uint8_t                         ucTrackedBufPenuIdx;        // 2nd-to-last tracked buffer index
    uint8_t                         ucTrackedBufAnteIdx;        // 3rd-to-last tracked buffer index
    bool                            bWaitForTrackedBuffer;      // wait to re-use tracked buffer
    CODEC_TRACKED_BUFFER            trackedBuffer[CODEC_NUM_TRACKED_BUFFERS];  // Csc/Ds/MbCode/MvData surface/buffer

    // Ds+Copy kernel optimization
    uint8_t                         outputChromaFormat;     // 1: 420 2: 422 3: 444
    bool                            bGopIsIdrFrameOnly;     // GOP structure contains I-frame only
    uint32_t                        dwRawSurfAlignment;     // raw surf alignment for HW pipeline to work properly
    PMOS_SURFACE                    psRawSurfaceToENC;
    PMOS_SURFACE                    psRawSurfaceToPAK;

    // HME VDEnc
    CODECHAL_ENCODE_BINDING_TABLE_GENERIC      VDEncMeKernelBindingTable;
    MHW_KERNEL_STATE                           VDEncMeKernelState;

    CODECHAL_ENCODE_BINDING_TABLE_GENERIC      VDEncStreaminKernelBindingTable;
    MHW_KERNEL_STATE                           VDEncStreaminKernelState;

    //Weighted Prediction Kernel
    bool                                        WPuseCommonKernel;

    // Common kernel
    bool                                        bUseCommonKernel;
    uint32_t                                    dwKUIDCommonKernel;

    // Generic ENC parameters
    uint32_t                        dwVerticalLineStride;
    uint32_t                        dwVerticalLineStrideOffset;

    // CMD buffer sizes
    uint32_t                        dwPictureStatesSize;
    uint32_t                        dwExtraPictureStatesSize;
    uint32_t                        dwSliceStatesSize;
    uint32_t                        dwVMEStatesSize;
    uint32_t                        dwHucCommandsSize;

    // Patch List Size
    uint32_t                        dwPicturePatchListSize;
    uint32_t                        dwExtraPicturePatchListSize;
    uint32_t                        dwSlicePatchListSize;
    uint32_t                        dwVMEPatchListSize; // not used yet, for future development

    // Single Task Phase parameters
    bool                            bSingleTaskPhaseSupported;
    bool                            bFirstTaskInPhase;
    bool                            bLastTaskInPhase;
    bool                            bLastEncPhase;
    bool                            bSingleTaskPhaseSupportedInPak;
    uint32_t                        dwHeaderBytesInserted;

    // Null Rendering Flags
    bool                            bVideoContextUsesNullHw;
    bool                            bRenderContextUsesNullHw;

    // Slice Shutdown parameters
    bool                            bSetRequestedEUSlices;
    bool                            bSetRequestedSubSlices;
    bool                            bSetRequestedEUs;
    uint32_t                        dwSliceShutdownDefaultState;
    uint32_t                        dwSliceShutdownRequestState;
    uint32_t                        dwSSDResolutionThreshold;
    uint32_t                        dwSSDTargetUsageThreshold;
    uint32_t                        dwTargetUsage;

    // Skip frame params
    uint8_t                         ucSkipFrameFlag;
    uint32_t                        dwNumSkipFrames;
    uint32_t                        dwSizeSkipFrames;       // acccumulative size of skipped frames for skipflag = 2
    uint32_t                        dwSizeCurrSkipFrame;    // size of curr skipped frame for skipflag = 2

    MHW_VDBOX_NODE_IND              VdboxIndex;

#if (_DEBUG || _RELEASE_INTERNAL)
    bool                            bMmcUserFeatureUpdated;     // indicate if the user feature is updated with MMC state
#endif

    MOS_STATUS (* pfnReadImageStatus) (
        PCODECHAL_ENCODER               pEncoder,
        PMOS_COMMAND_BUFFER             pCmdBuffer);

    MOS_STATUS (* pfnReadMfcStatus) (
        PCODECHAL_ENCODER               pEncoder,
        PMOS_COMMAND_BUFFER             pCmdBuffer);

    MOS_STATUS (* pfnResetStatusReport) (
        PCODECHAL_ENCODER               pEncoder);

    MOS_STATUS (* pfnUpdateEncodeStatus) (
        PCODECHAL_ENCODER               pEncoder,
        PMOS_COMMAND_BUFFER             pCmdBuffer,
        bool                            bForceOperation);

    MOS_STATUS (* pfnInitializeStandard) (
        PCODECHAL_ENCODER               pEncoder,
        void*                           pvStandardState,
        PCODECHAL_SETTINGS              pSettings);

    void (* pfnDestroyStandard) (
        PCODECHAL_ENCODER               pEncoder,
        void*                           pvStandardState);

    MOS_STATUS (* pfnEncodeInitialize) (
        PCODECHAL_ENCODER               pEncoder,
        void*                           pvStandardState);

    MOS_STATUS (* pfnEncodeKernelFunctions) (
        PCODECHAL_ENCODER               pEncoder,
        void*                           pvStandardState);

    MOS_STATUS (* pfnEncodePictureLevel) (
        PCODECHAL_ENCODER               pEncoder,
        void*                           pvStandardState);

    MOS_STATUS (* pfnEncodeSliceLevel) (
        PCODECHAL_ENCODER               pEncoder,
        void*                           pvStandardState);

    MOS_STATUS (* pfnReadBrcPakStatistics) (
        PCODECHAL_ENCODER                       pEncoder,
        PMOS_COMMAND_BUFFER                     pCmdBuffer,
        PCODECHAL_ENCODE_READ_BRC_PAK_STATS_PARAMS  pParams);

    MOS_STATUS (*pfnEncodeGetStatusReport) (
        PCODECHAL_ENCODER               pEncoder,
        PCODECHAL_ENCODE_STATUS         pEncodeStatus,
        PCODECHAL_ENCODE_STATUS_REPORT  pStatusReport);

    MOS_STATUS(*pfnInitializeMmcState)(
        PCODECHAL_ENCODER               pEncoder,
        void                            *standardState);

    MOS_STATUS(*pfnAddMediaVfeCmd)(
        PCODECHAL_ENCODER                        pEncoder,
        PMOS_COMMAND_BUFFER                      pCmdBuffer,
        PCODECHAL_ENCODE_SEND_KERNEL_CMDS_PARAMS pParams);
};

typedef struct _CODECHAL_PAK_INTEGRATION_BRCData
{
    uint32_t FrameByteCount;
    uint32_t FrameByteCountNoHeader;
    uint32_t HCP_ImageStatusControl;
} CODECHAL_PAK_INTEGRATION_BRCData;

// ---------------------------
// Functions
// ---------------------------
// For software scoreboard
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
static __inline void CodecHalEncode_MBWalker(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
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
        wflenX = curX/2;
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
static __inline void CodecHalEncode_MBWalker_45(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
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
static __inline void CodecHalEncode_MBAFFWalker(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint16_t i, j, k;
    uint16_t numMBs = picWidthInMB * picHeightInMB;
    uint16_t curX = 0, curY = 0, wflenX, wflenY;
    uint16_t wfstart = 0, wflen;
    uint16_t walkX, walkY;

    wflenY = picHeightInMB/2 - 1;
    for (i = 0; i < numMBs; i++)
    {
        // Get current region length
        wflenX = curX/2;
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

static __inline void CodecHalEncode_MBWalker_RasterScan(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
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

static __inline void CodecHalEncode_MBWalker_VerticalScan(uint16_t picWidthInMB, uint16_t picHeightInMB, uint16_t *mbmap)
{
    uint32_t i, j, k;

    for (i = 0, k = 0; i < picWidthInMB; i++)
    {
        for (j = 0; j < picHeightInMB; j++)
        {
            mbmap[k++] = (uint16_t)(i + j * picWidthInMB) ;
        }
    }
}

static __inline void PutBit(BSBuffer *bsbuffer, uint32_t code)
{
    if (code & 1)
    {
        *(bsbuffer->pCurrent) = (*(bsbuffer->pCurrent) | (uint8_t)(0x01 << (7 - bsbuffer->BitOffset)));
    }

    bsbuffer->BitOffset++;
    if (bsbuffer->BitOffset == 8)
    {
        bsbuffer->BitOffset = 0;
        bsbuffer->pCurrent++;
        *(bsbuffer->pCurrent) = 0;
    }
}

static __inline void PutBitsSub(BSBuffer *bsbuffer, uint32_t code, uint32_t length)
{
    uint8_t *byte = bsbuffer->pCurrent;

    // make sure that the number of bits given is <= 24
    CODECHAL_ENCODE_ASSERT(length <= 24);

    code <<= (32 - length);

    // shift field so that the given code begins at the current bit
    // offset in the most significant byte of the 32-bit word
    length += bsbuffer->BitOffset;
    code >>= bsbuffer->BitOffset;

    // write bytes back into memory, big-endian
    byte[0] = (uint8_t)((code >> 24) | byte[0]);
    byte[1] = (uint8_t)(code >> 16);
    if (length > 16)
    {
        byte[2] = (uint8_t)(code >> 8);
        byte[3] = (uint8_t)code;
    }
    else
    {
        byte[2] = 0;
    }

    // update bitstream pointer and bit offset
    bsbuffer->pCurrent += (length >> 3);
    bsbuffer->BitOffset = (length & 7);
}

static __inline void PutBits(BSBuffer *bsbuffer, uint32_t code, uint32_t length)
{
    uint32_t code1, code2;

    // temp solution, only support up to 32 bits based on current usage
    CODECHAL_ENCODE_ASSERT(length <= 32);

    if (length >= 24)
    {
        code1 = code & 0xFFFF;
        code2 = code >> 16;

        // high bits go first
        PutBitsSub(bsbuffer, code2, length-16);
        PutBitsSub(bsbuffer, code1, 16);
    }
    else
    {
        PutBitsSub(bsbuffer, code, length);
    }
}

/*----------------------------------------------------------------------------
| Name      : GetNumBrcPakPasses
| Purpose   : Returns number of PAK Passes based on BRC Precision flag
\---------------------------------------------------------------------------*/
static __inline uint8_t CodecHalEncode_GetNumBrcPakPasses(uint16_t usBRCPrecision)
{
    uint8_t numBRCPAKPasses = CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES;

    switch (usBRCPrecision)
    {
    case 0:
    case 2:     numBRCPAKPasses = CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES;
                break;

    case 1:     numBRCPAKPasses = CODECHAL_ENCODE_BRC_MINIMUM_NUM_PASSES;
                break;

    case 3:     numBRCPAKPasses = CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES;
                break;

    default:    CODECHAL_ENCODE_ASSERT("Invalid BRC Precision value in Pic Params.");
                numBRCPAKPasses = CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES;
                break;
    }

    return numBRCPAKPasses;
}

#endif  // __CODECHAL_ENCODER_H__
