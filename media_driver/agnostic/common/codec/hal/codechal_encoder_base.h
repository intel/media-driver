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
//! \file     codechal_encoder_base.h
//! \brief    Defines the encode interface for CodecHal.
//! \details  The encode interface is further sub-divided by standard, this file is for the base interface which is shared by all encode standards.
//!

#ifndef __CODECHAL_ENCODER_BASE_H__
#define __CODECHAL_ENCODER_BASE_H__

#include "codechal.h"
#include "codechal_setting.h"
#include "codechal_hw.h"
#include "codechal_debug.h"
#include "codechal_encode_sfc.h"
#include "codechal_encode_csc_ds.h"
#include "codechal_encode_tracked_buffer.h"
#include "codechal_encode_allocator.h"
#include "codechal_mmc.h"
#include "codechal_utilities.h"
#include "codec_def_encode.h"
#include "cm_rt_umd.h"
#include "media_perf_profiler.h"
#include <algorithm> // std::reverse

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

#define CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(_ptr)                                        \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define CODECHAL_ENCODE_CHK_NULL_RETURN(_ptr)                                                  \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _ptr)

#define CODECHAL_ENCODE_CHK_STATUS_RETURN(_stmt)                                               \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt)

#define CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_ENCODE_CHK_COND_RETURN(_expr, _message, ...)                           \
    MOS_CHK_COND_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE,_expr,_message, ##__VA_ARGS__)

// User Feature Report Writeout
#define CodecHalEncode_WriteKey64(key, value, mosCtx)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i64Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);\
}

#define CodecHalEncode_WriteKey(key, value, mosCtx)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i32Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);\
}

#define CodecHalEncode_WriteStringKey(key, value, len, mosCtx)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.StringData.pStringData = (value);\
    UserFeatureWriteData.Value.StringData.uSize = (len);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);\
}

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
#define CODECHAL_VDENC_BRC_NUM_OF_PASSES_FOR_TILE_REPLAY 22
#define CODECHAL_DP_MAX_NUM_BRC_PASSES                  4
#define CODECHAL_VDENC_NUMIMEPREDICTORS                 0x8
#define CODECHAL_CMDINITIALIZER_MAX_CMD_SIZE            CODECHAL_CACHELINE_SIZE * 4
#define CODECHAL_ENCODE_NUM_MAX_VME_L0_REF              8 // multiref - G7.5+ - used from DDI
#define CODECHAL_ENCODE_NUM_MAX_VME_L1_REF              2 // multiref - G7.5+ - used from DDI
#define CODECHAL_ENCODE_ME_DATA_SIZE_MULTIPLIER         (CODECHAL_ENCODE_NUM_MAX_VME_L0_REF + CODECHAL_ENCODE_NUM_MAX_VME_L1_REF)
#define CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L0_REF        4 // multiref - G7.5+
#define CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L1_REF        1 // multiref - G7.5+

// BRC
#define CODECHAL_ENCODE_BRC_KBPS                        1000     // 1000bps for disk storage, aligned with industry usage
#define CODECHAL_ENCODE_SCENE_CHANGE_DETECTED_MASK      0xffff

typedef enum _CODECHAL_ENCODE_FUNCTION_ID
{
    CODECHAL_ENCODE_ENC_ID           = 0x100,
    CODECHAL_ENCODE_PAK_ID           = 0x101,
    CODECHAL_ENCODE_ENC_PAK_ID       = 0x102,
    CODECHAL_ENCODE_VPP_ID           = 0x103,
    CODECHAL_ENCODE_FORMAT_COUNT_ID  = 0x104,
    CODECHAL_ENCODE_FORMATS_ID       = 0x105,
    CODECHAL_ENCODE_ENC_CTRL_CAPS_ID = 0x106,
    CODECHAL_ENCODE_ENC_CTRL_GET_ID  = 0x107,
    CODECHAL_ENCODE_ENC_CTRL_SET_ID  = 0x108,
    CODECHAL_ENCODE_MBDATA_LAYOUT_ID = 0x109,
    CODECHAL_ENCODE_FEI_PRE_ENC_ID   = 0x10A,
    CODECHAL_ENCODE_FEI_ENC_ID       = 0x10B,
    CODECHAL_ENCODE_FEI_PAK_ID       = 0x10C,
    CODECHAL_ENCODE_FEI_ENC_PAK_ID   = 0x10D,
    CODECHAL_ENCODE_QUERY_STATUS_ID  = 0x121
} CODECHAL_ENCODE_FUNCTION_ID;

typedef enum _CODECHAL_ENCODE_BRC_KERNEL_STATE_IDX
{
    CODECHAL_ENCODE_BRC_IDX_INIT = 0,
    CODECHAL_ENCODE_BRC_IDX_FrameBRC_UPDATE,
    CODECHAL_ENCODE_BRC_IDX_RESET,
    CODECHAL_ENCODE_BRC_IDX_IFRAMEDIST,
    CODECHAL_ENCODE_BRC_IDX_BLOCKCOPY,
    CODECHAL_ENCODE_BRC_IDX_MbBRC_UPDATE, // extra MbBRCUpdate kernel starting GEN9
    CODECHAL_ENCODE_BRC_IDX_NUM
} CODECHAL_ENCODE_BRC_KERNEL_STATE_IDX;

typedef struct KernelHeaderEncode
{
    int nKernelCount;

    union
    {
        struct
        {
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS4X_Frame;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS4X_Field;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS2X_Frame;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_DS2X_Field;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_P;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_B;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_Streamin;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HME_HEVC_Streamin;
            CODECHAL_KERNEL_HEADER Gen10_HEVC_VP9_VDEnc_HMEDetection;
        };
    };

}KernelHeaderEncode;

//!
//! \enum     BindingTableOffsetKernel
//! \brief    Binding table offset kernel
//!
enum BindingTableOffsetKernel
{
    // VDEnc HME kernel
    HmeBegin = 0,
    HmeMvDataSurfaceCm = HmeBegin,
    Hme16xMeMvDataSurfaceCm,
    Hme32xMeMvDataSurfaceCm = Hme16xMeMvDataSurfaceCm,
    HmeDistortionSurfaceCm,
    HmeBrcDistortionCm,
    HmeCurrForFwdRefCm,
    HmeFwdRefIdx0Cm,
    HmeReserved1Cm,
    HmeFwdRefIdx1Cm,
    HmeReserved2Cm,
    HmeFwdRefIdx2Cm,
    HmeReserved3Cm,
    HmeFwdRefIdx3Cm,
    HmeReserved4Cm,
    HmeFwdRefIdx4Cm,
    HmeReserved5Cm,
    HmeFwdRefIdx5Cm,
    HmeReserved6Cm,
    HmeFwdRefIdx6Cm,
    HmeReserved7Cm,
    HmeFwdRefIdx7Cm,
    HmeReserved8Cm,
    HmeCurrForBwdRefCm,
    HmeBwdRefIdx0Cm,
    HmeReserved9Cm,
    HmeBwdRefIdx1Cm,
    HmeReserved10Cm,
    HmeVdencStreaminOutputCm,
    HmeVdencStreaminInputCm,
    HmeEnd,
};

//!
//! \enum   BrcUpdateFlag
//! \brief  Indicate the Brc Update Flag
//!
enum BrcUpdateFlag
{
    brcUpdateIsField         = 0x01,
    brcUpdateIsMbaff         = (0x01 << 1),
    brcUpdateIsBottomField   = (0x01 << 2),
    brcUpdateAutoPbFrameSize = (0x01 << 3),
    brcUpdateIsActualQp      = (0x01 << 6),
    brcUpdateIsReference     = (0x01 << 7)
};

//!
//! \enum   TrellisSetting
//! \brief  Indicate the different Trellis Settings
//!
enum TrellisSetting
{
    trellisInternal = 0,
    trellisDisabled = 1,
    trellisEnabledI = 2,
    trellisEnabledP = 4,
    trellisEnabledB = 8
};

//!
//! \enum   MbBrcSetting
//! \brief  Indicate the MBBRC settings
//!
enum MbBrcSetting
{
    mbBrcInternal = 0,
    mbBrcEnabled  = 1,
    mbBrcDisabled = 2,
};

// User Feature Key Report Writeout
#define CodecHalEncodeWriteKey64(key, value, mosCtx)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i64Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);\
}

#define CodecHalEncodeWriteKey(key, value, mosCtx)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.i32Data  = (value);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);\
}

#define CodecHalEncodeWriteStringKey(key, value, len, mosCtx)\
{\
    MOS_USER_FEATURE_VALUE_WRITE_DATA   UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;\
    UserFeatureWriteData.Value.StringData.pStringData = (value);\
    UserFeatureWriteData.Value.StringData.uSize = (len);\
    UserFeatureWriteData.ValueID        = (key);\
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, mosCtx);\
}

// ---------------------------
// Tables
// ---------------------------

// ---------------------------
// Structures
// ---------------------------

#define CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, type) {                               \
    perfTag.Value               = 0;                                                    \
    perfTag.Mode                = m_mode & CODECHAL_ENCODE_MODE_BIT_MASK;               \
    perfTag.CallType            = type;                                                 \
    perfTag.PictureCodingType   = m_pictureCodingType > 3 ? 0 : m_pictureCodingType;    \
    m_osInterface->pfnSetPerfTag(m_osInterface, perfTag.Value);                         \
    m_osInterface->pfnIncPerfBufferID(m_osInterface);             }

//!
//! \struct    CodechalEncodeMdfKernelResource
//! \brief     Codechal encode mdf kernel resource
//!
struct CodechalEncodeMdfKernelResource
{
    void                            *pCommonISA;
    CmProgram                       *pCmProgram;
    CmKernel                        **ppKernel;
    uint8_t                         *pCurbe;
    CmThreadSpace                   *pTS;
    CmBuffer                        **ppCmBuf;
    CmSurface2D                     **ppCmSurf;
    SurfaceIndex                    **ppCmVmeSurf;
    CmEvent                         *e;

    uint8_t                         KernelNum;
    uint8_t                         BufNum;
    uint8_t                         SurfNum;
    uint8_t                         VmeSurfNum;
    uint16_t                        wCurbeSize;
    uint16_t                        wReserved;
};

//!
//! \struct MfeParams
//! \brief  Mfe encode parameters
//!
struct MfeParams
{
    uint32_t                           streamId;             //!< Unique id
    uint32_t                           submitIndex;          //!< Index during this submission
    uint32_t                           submitNumber;         //!< Total stream number during this submission
    uint32_t                           maxWidth;             //!< Maximum width for all frames
    uint32_t                           maxHeight;            //!< Maximum height for all frames
};

//!
//! \struct MfeSharedState
//! \brief  State shared across multiple streams
//!
struct MfeSharedState
{
    CodechalHwInterface             *pHwInterface = nullptr;
    PMOS_INTERFACE                  pOsInterface;
    PMHW_KERNEL_STATE               pMfeMbEncKernelState;            //!< Set in the first stream, Used by the rest streams
    uint32_t                        dwPicWidthInMB;                  //!< Keep the maximum width
    uint32_t                        dwPicHeightInMB;                 //!< Keep the maximum height
    uint16_t                        sliceHeight;                     //!< Keep the maximum slice height
    uint32_t                        maxTheadWidth = 0;               //!< Keep the maximum width of the threadspace
    uint32_t                        maxTheadHeight = 0;              //!< Keep the maximum Height of the threadspace
    uint32_t                        *maxThreadWidthFrames = nullptr; //!< Keep the thread space width for all frames

    CmDevice                                *pCmDev = nullptr;       //!< Set in the first stream, Used by the rest streams
    CmTask                                  *pCmTask = nullptr;
    CmQueue                                 *pCmQueue = nullptr;
    CodechalEncodeMdfKernelResource         *resMbencKernel = nullptr;
    SurfaceIndex                            *vmeSurface = nullptr;
    SurfaceIndex                            *commonSurface = nullptr;
    std::vector<CodechalEncoderState*>      encoders;
};

//!
//! \struct    FeiPreEncParams
//! \brief     Fei pre-encode parameters
//!
struct FeiPreEncParams
{
    MOS_RESOURCE                    resMvPredBuffer;
    MOS_RESOURCE                    resMbQpBuffer;
    MOS_RESOURCE                    resMvBuffer;
    MOS_RESOURCE                    resStatsBuffer;
    MOS_RESOURCE                    resStatsBotFieldBuffer;

    PMOS_SURFACE                    psCurrOriginalSurface;

    bool                            bInterlaced;
    uint32_t                        dwNumPastReferences;
    uint32_t                        dwNumFutureReferences;

    bool                            bCurPicUpdated;
    CODEC_PICTURE                   CurrOriginalPicture;

    CODEC_PICTURE                   PastRefPicture;
    bool                            bPastRefUpdated;
    MOS_SURFACE                     sPastRefSurface;
    bool                            bPastRefStatsNeeded;
    MOS_RESOURCE                    sPastRefStatsBuffer;
    MOS_RESOURCE                    sPastRefStatsBotFieldBuffer;

    CODEC_PICTURE                   FutureRefPicture;
    bool                            bFutureRefUpdated;
    MOS_SURFACE                     sFutureRefSurface;
    bool                            bFutureRefStatsNeeded;
    MOS_RESOURCE                    sFutureRefStatsBuffer;
    MOS_RESOURCE                    sFutureRefStatsBotFieldBuffer;

    uint32_t                        dwFrameQp;
    uint32_t                        dwLenSP;
    uint32_t                        dwSearchPath;
    uint32_t                        dwSubMBPartMask;
    uint32_t                        dwIntraPartMask;
    uint32_t                        dwSubPelMode;
    uint32_t                        dwInterSAD;
    uint32_t                        dwIntraSAD;
    bool                            bAdaptiveSearch;

    uint32_t                        dwMVPredictorCtrl;
    bool                            bMBQp;
    bool                            bFTEnable;
    uint32_t                        dwRefWidth;
    uint32_t                        dwRefHeight;
    uint32_t                        dwSearchWindow;
    bool                            bDisableMVOutput;
    bool                            bDisableStatisticsOutput;
    bool                            bEnable8x8Statistics;
    bool                            bInputUpdated;
};

//!
//! \enum  EncOperation
//! \brief Encode operations
//!
enum EncOperation
{
    ENC_SCALING4X = 0,
    ENC_SCALING2X,
    ENC_ME,
    ENC_BRC,
    ENC_MBENC,
    ENC_MBENC_ADV,
    ENC_RESETVLINESTRIDE,
    ENC_MC,
    ENC_MBPAK,
    ENC_DEBLOCK,
    ENC_PREPROC,
    VDENC_ME,
    ENC_WP,
    ENC_SFD,                   //!< Static frame detection
    ENC_SCOREBOARD,
    ENC_MBENC_I_LUMA,
    ENC_MPU,
    ENC_TPU,
    ENC_SCALING_CONVERSION,    //!< for HEVC
    ENC_DYS,
    ENC_INTRA_DISTORTION,
    VDENC_ME_P,
    VDENC_ME_B,
    VDENC_STREAMIN,
    VDENC_STREAMIN_HEVC,
    VDENC_STREAMIN_HEVC_RAB
};

//!
//! \enum  HmeLevel
//! \brief Indicate the Hme level
//!
enum HmeLevel
{
    HME_LEVEL_4x  = 0,
    HME_LEVEL_16x = 1,
    HME_LEVEL_32x = 2
};

//!
//! \enum     EncodeMeMode
//! \brief    Encode me mode
//!
enum EncodeMeMode
{
    CODECHAL_ENCODE_ME16X_BEFORE_ME4X = 0,
    CODECHAL_ENCODE_ME16X_ONLY = 1,
    CODECHAL_ENCODE_ME4X_ONLY = 2,
    CODECHAL_ENCODE_ME4X_AFTER_ME16X = 3
};

//!
//! \enum     CodechalEncodeBrcNumPasses
//! \brief    used in GetNumBrcPakPasses
//!
enum CodechalEncodeBrcNumPasses
{
    CODECHAL_ENCODE_BRC_SINGLE_PASS        = 1, // No IPCM case
    CODECHAL_ENCODE_BRC_MINIMUM_NUM_PASSES = 2, // 2 to cover IPCM case
    CODECHAL_ENCODE_BRC_DEFAULT_NUM_PASSES = 4,
    CODECHAL_ENCODE_BRC_MAXIMUM_NUM_PASSES = 7
};

//!
//! \struct HmeParams
//! \brief  Indicate the Hme parameters
//!
struct HmeParams
{
    // ME
    PMOS_SURFACE            ps4xMeMvDataBuffer;
    PMOS_SURFACE            ps16xMeMvDataBuffer;
    PMOS_SURFACE            ps32xMeMvDataBuffer;
    PMOS_SURFACE            ps4xMeDistortionBuffer;
    PMOS_RESOURCE           presMvAndDistortionSumSurface;
    bool                    b4xMeDistortionBufferSupported;
};

//!
//! \brief  Generic binding table
//!
struct GenericBindingTable
{
    uint32_t   dwMediaState;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
    uint32_t   dwBindingTableEntries[64];
};
using PCODECHAL_ENCODE_BINDING_TABLE_GENERIC = GenericBindingTable*;

//!
//! \struct MeKernelBindingTable
//! \brief  The struct of Me kernel binding table
//!
struct MeKernelBindingTable
{
    uint32_t                        dwMEMVDataSurface;
    uint32_t                        dwMECurrY;
    uint32_t                        dwMEFwdRefY;
    uint32_t                        dwMEBwdRefY;
    uint32_t                        dwMECurrForFwdRef;
    uint32_t                        dwMEFwdRef;
    uint32_t                        dwMECurrForBwdRef;
    uint32_t                        dwMEBwdRef;
    uint32_t                        dw16xMEMVDataSurface;
    uint32_t                        dw32xMEMVDataSurface;
    uint32_t                        dwMEDist;
    uint32_t                        dwMEBRCDist;
    uint32_t                        dwMECurrForFwdRefIdx[CODECHAL_ENCODE_NUM_MAX_VME_L0_REF];
    uint32_t                        dwMECurrForBwdRefIdx[CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];

    // Frame Binding Table Entries
    uint32_t                        dwMEFwdRefPicIdx[CODECHAL_ENCODE_NUM_MAX_VME_L0_REF];
    uint32_t                        dwMEBwdRefPicIdx[CODECHAL_ENCODE_NUM_MAX_VME_L1_REF];

    // Field Binding Table Entries
    uint32_t                        dwMEFwdRefPicTopIdx[CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L0_REF];
    uint32_t                        dwMEFwdRefPicBotIdx[CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L0_REF];
    uint32_t                        dwMEBwdRefPicTopIdx[CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L1_REF];
    uint32_t                        dwMEBwdRefPicBotIdx[CODECHAL_ENCODE_FIELD_NUM_MAX_VME_L1_REF];

    uint32_t                        dwVdencStreamInSurface;
    uint32_t                        dwVdencStreamInInputSurface;

    uint32_t                        dwBindingTableStartOffset;
    uint32_t                        dwNumBindingTableEntries;
};

//!
//! \struct MeSurfaceParams
//! \brief  The struct of Me surface parameters
//!
struct MeSurfaceParams
{
    PCODEC_REF_LIST                 *ppRefList;
    PCODEC_PIC_ID                   pPicIdx;
    PCODEC_PICTURE                  pCurrOriginalPic;
    PMOS_SURFACE                    ps4xMeMvDataBuffer;
    uint32_t                        dw4xMeMvBottomFieldOffset;
    PMOS_SURFACE                    ps16xMeMvDataBuffer;
    uint32_t                        dw16xMeMvBottomFieldOffset;
    PMOS_SURFACE                    ps32xMeMvDataBuffer;
    uint32_t                        dw32xMeMvBottomFieldOffset;
    uint32_t                        dw4xScaledBottomFieldOffset;
    uint32_t                        dw16xScaledBottomFieldOffset;
    uint32_t                        dw32xScaledBottomFieldOffset;
    PMOS_SURFACE                    psMeDistortionBuffer;
    uint32_t                        dwMeDistortionBottomFieldOffset;
    PMOS_SURFACE                    psMeBrcDistortionBuffer;
    PMOS_RESOURCE                   psMeVdencStreamInBuffer;
    uint32_t                        dwMeBrcDistortionBottomFieldOffset;
    uint32_t                        dwDownscaledWidthInMb;
    uint32_t                        dwDownscaledHeightInMb;
    uint32_t                        dwVDEncStreamInSurfaceSize;
    uint32_t                        dwVerticalLineStride;
    uint32_t                        dwVerticalLineStrideOffset;
    bool                            b16xMeInUse;
    bool                            b32xMeInUse;
    bool                            b16xMeEnabled;
    bool                            b32xMeEnabled;
    bool                            bVdencStreamInEnabled;
    PCODEC_PICTURE                  pL0RefFrameList;
    PCODEC_PICTURE                  pL1RefFrameList;
    uint32_t                        dwNumRefIdxL0ActiveMinus1;
    uint32_t                        dwNumRefIdxL1ActiveMinus1;
    PMHW_KERNEL_STATE               pKernelState;
    MeKernelBindingTable*           pMeBindingTable;
    bool                            bMbaff;
    bool                            b4xMeDistortionBufferSupported;
};

//!
//! \struct MeCurbeParams
//! \brief  The struct of Me Curbe parameters
//!
struct MeCurbeParams
{
    PMHW_KERNEL_STATE               pKernelState;
    HmeLevel                        hmeLvl;
    bool                            b16xMeEnabled;
    bool                            b32xMeEnabled;
    bool                            segmapProvided;
    uint8_t                         TargetUsage;
    uint8_t                        *pMEMethodTable;
    uint8_t                        *pBMEMethodTable;
    int32_t                         MaxMvLen;

    // Picture parameters
    CODEC_PICTURE                   CurrOriginalPic;  // Source sent in by BeginFrame
    int8_t                          pic_init_qp_minus26;

    // Slice parameters
    int8_t                          slice_qp_delta;
    uint8_t                         num_ref_idx_l0_active_minus1;
    uint8_t                         num_ref_idx_l1_active_minus1;
    bool                            List0RefID0FieldParity;
    bool                            List0RefID1FieldParity;
    bool                            List0RefID2FieldParity;
    bool                            List0RefID3FieldParity;
    bool                            List0RefID4FieldParity;
    bool                            List0RefID5FieldParity;
    bool                            List0RefID6FieldParity;
    bool                            List0RefID7FieldParity;
    bool                            List1RefID0FieldParity;
    bool                            List1RefID1FieldParity;
    uint8_t                         *pCurbeBinary;
};

//!
//! \struct    EncodeReadBrcPakStatsParams
//! \brief     Encode read brc pak states parameters
//!
struct EncodeReadBrcPakStatsParams
{
    CodechalHwInterface    *pHwInterface;
    PMOS_RESOURCE           presBrcPakStatisticBuffer;
    PMOS_RESOURCE           presStatusBuffer;
    uint32_t                dwStatusBufNumPassesOffset;
    uint8_t                 ucPass;
    MOS_GPU_CONTEXT         VideoContext;
};

//!
//! \struct PakIntegrationBrcData
//! \brief  The struct of Huc input command 2
//!
struct PakIntegrationBrcData
{
    uint32_t FrameByteCount;
    uint32_t FrameByteCountNoHeader;
    uint32_t HCP_ImageStatusControl;
};

//!
//! \struct SearchPathDelta
//! \brief  The struct of Search Path Delta
//!
struct SearchPathDelta
{
    uint8_t   SearchPathDelta_X       : 4;
    uint8_t   SearchPathDelta_Y       : 4;
};

//!
//! \struct    HwCounter
//! \brief     Hardware counter
//!
struct HwCounter
{
    uint64_t IV;         // Big-Endian IV
    uint64_t Count;      // Big-Endian Block Count
};

//!
//! \struct PerfTagSetting
//! \brief  Setting of performance tags
//!
struct PerfTagSetting
{
    union
    {
        struct
        {
            uint16_t    PictureCodingType   : 2;
            uint16_t    CallType            : 6;
            uint16_t    Mode                : 4;
            uint16_t                        : 4;
        };
        uint16_t        Value;
    };
};

//!
//! \struct SendKernelCmdsParams
//! \brief  Struct of send kernel commands parameters
//!
struct SendKernelCmdsParams
{
    CODECHAL_MEDIA_STATE_TYPE           EncFunctionType = CODECHAL_NUM_MEDIA_STATES;
    uint32_t                            uiDshIdx = 0;
    uint8_t                             ucDmvPredFlag = 0;
    bool                                bBrcResetRequested = false;
    bool                                bEnable45ZWalkingPattern = false;
    bool                                bEnableCustomScoreBoard = false;
    PMHW_VFE_SCOREBOARD                 pCustomScoreBoard = nullptr;
    PMHW_KERNEL_STATE                   pKernelState = nullptr;
    bool                                bDshInUse = false;
};

//!
//! \struct BrcQpReport
//! \brief  Struct of Query bit rate control and QP Status
//!
struct BrcQpReport
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
};

//!
//! \struct LookaheadStatus
//! \brief  Struct of Query bit rate control and QP Status
//!
struct LookaheadReport
{
    uint32_t StatusReportNumber;
    union
    {
        struct
        {
            uint32_t cqmHint    : 8;
            uint32_t intraHint  : 1;
            uint32_t reserved2  : 22;
            uint32_t isValid    : 1;
        };
        uint32_t encodeHints;
    };
    uint32_t targetFrameSize;
    uint32_t targetBufferFulness;
    uint32_t pyramidDeltaQP;
    uint8_t  miniGopSize;
    uint8_t  reserved1[3];
    uint32_t reserved3[10];
};

//!
//! \struct    EncodeBrcBuffers
//! \brief     Encode brc buffers
//!
struct EncodeBrcBuffers
{
    MOS_RESOURCE            resBrcHistoryBuffer;                                                    // BRC history buffer
    MOS_RESOURCE            resBrcPakStatisticBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    uint32_t                uiCurrBrcPakStasIdxForRead;
    uint32_t                uiCurrBrcPakStasIdxForWrite;
    MOS_RESOURCE            resBrcImageStatesReadBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];      // read only BRC image state buffer
    MOS_RESOURCE            resBrcImageStatesWriteBuffer;                                          // write only BRC image state buffer
    MOS_RESOURCE            resHevcBrcImageStatesWriteBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]; // write only HEVC BRC image state buffers
    MOS_RESOURCE            resPakQPInputTable;
    MOS_RESOURCE            resBrcConstantDataBuffer;
    MOS_RESOURCE            resEncoderCfgCommandReadBuffer;
    MOS_RESOURCE            resEncoderCfgCommandWriteBuffer;
    MOS_RESOURCE            resBrcPakStatsBeforeDumpBuffer;  // These buffers are used to dump all the MMIO read values
    MOS_RESOURCE            resBrcPakStatsAfterDumpBuffer;   // which are super-set of PakStat
    MOS_SURFACE             sBrcConstantDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    MOS_SURFACE             sMeBrcDistortionBuffer;
    uint32_t                dwMeBrcDistortionBottomFieldOffset;
    MOS_RESOURCE            resMbBrcConstDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
    MOS_SURFACE             sBrcMbQpBuffer;
    uint32_t                dwBrcMbQpBottomFieldOffset;
    MOS_RESOURCE            resBrcPicHeaderInputBuffer;
    MOS_RESOURCE            resBrcPicHeaderOutputBuffer;
    MOS_RESOURCE            resMbEncAdvancedDsh;
    MOS_RESOURCE            resMbEncBrcBuffer;
    MOS_SURFACE             sBrcRoiSurface;                 // BRC ROI surface
    PMHW_KERNEL_STATE       pMbEncKernelStateInUse;
};

//!
//! \struct    CODECHAL_ENCODE_BUFFER
//! \brief     Codechal encode buffer
//!
struct CODECHAL_ENCODE_BUFFER
{
    MOS_RESOURCE    sResource;
    uint32_t        dwSize = 0;
};
using PCODECHAL_ENCODE_BUFFER = CODECHAL_ENCODE_BUFFER*;

//!
//! \struct CodechalTileInfo
//! \brief Tile info report to application
//!
struct CodechalTileInfo
{
    uint16_t    TileRowNum;
    uint16_t    TileColNum;
    uint32_t    TileBitStreamOffset;
    uint32_t    TileSizeInBytes;

    uint32_t    reserved;
    HwCounter   HWCounterValue;
};

//!
//! \struct EncodeStatusReport
//! \brief  Encode status report structure
//!
struct EncodeStatusReport
{
    CODECHAL_STATUS                 CodecStatus;            //!< Status for the picture associated with this status report
    uint32_t                        StatusReportNumber;     //!< Status report number associated with the picture in this status report provided in CodechalEncoderState::Execute()
    CODEC_PICTURE                   CurrOriginalPic;        //!< Uncompressed frame information for the picture associated with this status report
    CODECHAL_ENCODE_FUNCTION_ID     Func;                   //!< Encode function requested at CodechalEncoderState::Execute()
    PCODEC_REF_LIST                 pCurrRefList;           //!< Reference list for the current frame, used for dump purposes with CodecHal Debug Tool
                                                            /*! \brief Specifies the order in which the statuses are expected.
                                                            *
                                                            *   The order in which a status is returned is requested at the DDI level and the order itself is determined by StatusReportNumber.
                                                            *       FALSE indicates the statuses should be returned in reverse order.
                                                            *       TRUE indicates the statuses should be returned in sequential order.
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
    int8_t                          QpY;
    /*! \brief Suggested Qp delta value for Y.
    *
    *   Framework can add this delta Qp with the first pass QpY to get the final Qp used for multi-pass.  It is not valid if CQP is set by framework.
    *   Note: Framework can use this reported QpY and suggestedQpYDelta to set QpY in picture parameter to minimize LCU level Qp delta.
    */
    int8_t                          SuggestedQpYDelta;
    uint8_t                         NumberPasses;       //!< Number of PAK passes executed.
    uint8_t                         AverageQp;          //!< The average QP of all MBs or LCUs of the frame.
    HwCounter                       HWCounterValue;
    uint64_t *                      hwctr;
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
            uint32_t                    : 26;
        };
        uint32_t QueryStatusFlags;
    };
    /*! \brief The average MAD (Mean Absolute Difference) across all macroblocks in the Y plane.
    *
    *    The MAD value is the mean of the absolute difference between the pixels in the original block and the corresponding pixels in the block being used for comparison, from motion compensation or intra spatial prediction. MAD reporting is disabled by default.
    */
    uint32_t                        MAD;
    uint32_t                        loopFilterLevel;        //!< [VP9]
    int8_t                          LongTermIndication;     //!< [VP9]
    uint16_t                        NextFrameWidthMinus1;   //!< [VP9]
    uint16_t                        NextFrameHeightMinus1;  //!< [VP9]
    uint8_t                         NumberSlices;           //!< Number of slices generated for the frame.
    uint16_t                        PSNRx100[3];            //!< PSNR for different channels
    uint32_t                        NumberTilesInFrame;     //!< Number of tiles generated for the frame.
    uint8_t                         UsedVdBoxNumber;        //!< Number of vdbox used.
    uint32_t                        SizeOfSliceSizesBuffer; //!< Store the size of slice size buffer
    uint16_t                        *pSliceSizes;           //!< Pointer to the slice size buffer
    uint32_t                        SizeOfTileInfoBuffer;   //!< Store the size of tile info buffer
    CodechalTileInfo*               pHEVCTileinfo;          //!< Pointer to the tile info buffer
    uint32_t                        NumTileReported;        //!< The number of tiles reported in status

    /*! \brief indicate whether it is single stream encoder or MFE.
    *
    *    For single stream encoder (regular), this value should be set to default 0. For Multi-Frame-Encoder (MFE), this value is the StreamId that is set by application.
    */
    uint32_t                        StreamId;

    LookaheadReport*                pLookaheadStatus;       //!< Pointer to the lookahead status buffer. Valid in lookahead pass only.
};

//!
//! \struct EncodeStatusSliceReport
//! \brief  The struct stores Encode status buffers
//!
struct EncodeStatusSliceReport
{
    uint32_t                        SliceSizeOverflow;
    uint8_t                         NumberSlices;
    uint32_t                        SizeOfSliceSizesBuffer;
    PMOS_RESOURCE                   pSliceSize;
    uint32_t                        reserved;
};

//!
//! \struct EncodeStatus
//! \brief  The struct stores Encode status
//!
struct EncodeStatus
{
    uint32_t                        dwStoredDataMfx;    //!< HW requires a QW aligned offset for data storage
    uint32_t                        dwPad;              //!< Pad
    struct
    {
        uint32_t                    dwStoredData;
        uint32_t                    dwPad;
    } qwStoredDataEnc[CODECHAL_NUM_MEDIA_STATES];           //!< Media states of stored encode data
    uint32_t                        dwStoredData;           //!< SW stored data
    uint32_t                        dwMFCBitstreamByteCountPerFrame;         //!< Media fixed function bitstream byte count per frame
    uint32_t                        dwMFCBitstreamSyntaxElementOnlyBitCount; //!< Media fixed function bitstream bit count for syntax element only
    uint32_t                        reserved;
    uint32_t                        dwImageStatusMask;      //!< MUST ENSURE THAT THIS IS QWORD ALIGNED as it's used for the conditional BB end
    MHW_VDBOX_IMAGE_STATUS_CONTROL  ImageStatusCtrl;        //!< Used for storing the control flags for the image status
    uint32_t                        HuCStatusRegMask;       //!< MUST ENSURE THAT THIS IS QWORD ALIGNED as it's used for the conditional BB end
    uint32_t                        HuCStatusReg;           //!< Register value saving HuC Status
    MHW_VDBOX_PAK_NUM_OF_SLICES     NumSlices;              //!< Num of slices for encode
    uint32_t                        dwErrorFlags;           //!< The definition is different on SNB/IVB, hence DWORD
    union
    {
        BrcQpReport                 BrcQPReport;            //!< Query bit rate control and QP Status
        LookaheadReport             lookaheadStatus;        //!< Lookahead status. valid in lookahead pass only
    };
    uint32_t                        dwNumberPasses;         //!< Number of passes
    uint32_t                        dwHeaderBytesInserted;  //!< The size including header, prevention bytes and dummy "0xff" inserted by SW driver
    CodechalQpStatusCount           QpStatusCount;          //!< This is used to obtain the cumulative QP
    EncodeStatusReport              encodeStatusReport;     //!< The detailed encode status report structure
    uint16_t                        wPictureCodingType;     //!< Type of picture coding
    uint32_t                        LoopFilterLevel;        //!< The level of loop filter
    MHW_VDBOX_IMAGE_STATUS_CONTROL  ImageStatusCtrlOfLastBRCPass;   //!< The level of loop filter
    uint32_t                        dwSceneChangedFlag;     //!< The flag indicate if the scene is changed
    uint64_t                        sumSquareError[3];      //!< The list of sum square error
    EncodeStatusSliceReport         sliceReport;
};

//!
//! \struct EncodeStatusBuffer
//! \brief  The sturct of encode status buffer
//!
struct EncodeStatusBuffer
{
    uint8_t                                 *pEncodeStatus;                  //!> needs to be first to ensure alingment of dwStoredDataMfx/Vme
    MOS_RESOURCE                            resStatusBuffer;                //!> Handle of eStatus buffer
    uint32_t                                *pData;                         //!> Pointer of the buffer of actual data
    uint16_t                                wFirstIndex;                    //!> Indicate the first index of status
    uint16_t                                wCurrIndex;                     //!> Indicate current index of status
    uint32_t                                dwStoreDataOffset;              //!> The offset of stored data
    uint32_t                                dwBSByteCountOffset;            //!> The offset of BS byte count
    uint32_t                                dwBSSEBitCountOffset;           //!> The offset of BS SE byte count
    uint32_t                                dwImageStatusMaskOffset;        //!> The offset of image status mask
    uint32_t                                dwImageStatusCtrlOffset;        //!> The offset of status control
    uint32_t                                dwHuCStatusMaskOffset;          //!> The offset of HuC status register mask
    uint32_t                                dwHuCStatusRegOffset;           //!> The offset of HuC status register
    uint32_t                                dwNumSlicesOffset;              //!> The offset of silce num
    uint32_t                                dwErrorFlagOffset;              //!> The offset of error flag
    uint32_t                                dwBRCQPReportOffset;            //!> The offset of bitrate control QP report
    uint32_t                                dwNumPassesOffset;              //!> The offset of passes number
    uint32_t                                dwQpStatusCountOffset;          //!> The offset of QP status count
    uint32_t                                dwImageStatusCtrlOfLastBRCPassOffset; //!> The offset of image status control of last bitrate control pass
    uint32_t                                dwSceneChangedOffset;           //!> The offset of the scene changed flag
    uint32_t                                dwSumSquareErrorOffset;         //!> The offset of list of sum square error
    uint32_t                                dwSliceReportOffset;            //!> The offset of slice size report structure
    uint32_t                                dwLookaheadStatusOffset;        //!> The offset of lookahead status
    uint32_t                                dwSize;                         //!> Size of status buffer
    uint32_t                                dwReportSize;                   //!> Size of report
};

//!
//! \struct AtomicScratchBuffer
//! \brief  The sturct of Atomic Scratch Buffer
//!
struct AtomicScratchBuffer
{
    MOS_RESOURCE                            resAtomicScratchBuffer;     //!> Handle of eStatus buffer
    uint32_t                                *pData;                     //!> Pointer of the buffer of actual data
    uint16_t                                wEncodeUpdateIndex;         //!> used for VDBOX update encode status
    uint16_t                                wTearDownIndex;             //!> Reserved for future extension
    uint32_t                                dwZeroValueOffset;          //!> Store the result of the ATOMIC_CMP
    uint32_t                                dwOperand1Offset;           //!> Operand 1 of the ATOMIC_CMP
    uint32_t                                dwOperand2Offset;           //!> Operand 2 of the ATOMIC_CMP
    uint32_t                                dwOperand3Offset;           //!> Copy of the operand 1

    uint32_t                                dwSize;                     //!> Size of the buffer
    uint32_t                                dwOperandSetSize;           //!> Size of Operand set
};

//!
//! \struct CodechalEncodeBbuf
//! \brief  Struct for Batch buffer
//!
struct CodechalEncodeBbuf
{
    MHW_BATCH_BUFFER        BatchBuffer;
    uint32_t                dwSize;
    uint32_t                dwNumMbsInBBuf;
    bool                    fieldScale;
};

//!
//! \struct CodechalEncodeIdOffsetParams
//! \brief  Indicate the ID Offset parameters
//!
struct CodechalEncodeIdOffsetParams
{
    uint32_t                                Standard;
    CODECHAL_MEDIA_STATE_TYPE               EncFunctionType;
    uint16_t                                wPictureCodingType;
    uint8_t                                 ucDmvPredFlag;
    bool                                    interlacedField;
};

//!
//! \struct VdencBrcPakMmio
//! \brief  MMIO of BRC and PAK
//!
struct VdencBrcPakMmio
{
    uint32_t                dwReEncode[4];
};

//!
//! \struct VdencHucErrorStatus
//! \brief  Huc Error Flags
//!
struct VdencHucErrorStatus
{
    uint32_t                dwErrorFlag[4];
};

//!
//! \enum    EncodeMode
//! \brief   Encode mode
//!
enum EncodeMode
{
    encodeNormalMode = 0,
    encodePerformanceMode = 1,
    encodeQualityMode = 2
};

enum
{
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_PHASE1_KERNEL = CODECHAL_ENCODE_PERFTAG_CALL_MBENC_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_PHASE2_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_SCALING_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_INTRA_DIST,
    CODECHAL_ENCODE_PERFTAG_CALL_ME_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_BRC_INIT_RESET,
    CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE,
    CODECHAL_ENCODE_PERFTAG_CALL_BRC_COPY,
    CODECHAL_ENCODE_PERFTAG_CALL_PAK_ENGINE,
    CODECHAL_ENCODE_PERFTAG_CALL_PAK_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_PAK_PHASE1_KERNEL = CODECHAL_ENCODE_PERFTAG_CALL_PAK_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_PAK_PHASE2_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_UPSCALING,
    CODECHAL_ENCODE_PERFTAG_CALL_DEBLOCKING,
    CODECHAL_ENCODE_PERFTAG_CALL_WP_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_32X32_PU_MD,
    CODECHAL_ENCODE_PERFTAG_CALL_32X32_B_IC,
    CODECHAL_ENCODE_PERFTAG_CALL_16X16_PU_MD,
    CODECHAL_ENCODE_PERFTAG_CALL_16X16_SAD,
    CODECHAL_ENCODE_PERFTAG_CALL_8X8_PU,
    CODECHAL_ENCODE_PERFTAG_CALL_8X8_FMODE,
    CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_LCU,
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_I_32x32,
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_I_16x16,
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_P,
    CODECHAL_ENCODE_PERFTAG_CALL_MBENC_TX,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_RECON_LUMA,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_RECON_CHROMA,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_RECON_LUMA_32x32,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_RECON_INTRA_LUMA,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_RECON_INTRA_CHROMA,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_DEBLOCK_MASK,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_DEBLOCK_LUMA,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_DEBLOCK_CHROMA,
    CODECHAL_ENCODE_PERFTAG_CALL_MBPAK_MC,
    CODECHAL_ENCODE_PERFTAG_CALL_PREPROC_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_DS_CONVERSION_KERNEL,
    CODECHAL_ENCODE_PERFTAG_CALL_SCOREBOARD,
    CODECHAL_ENCODE_PERFTAG_CALL_SFD_KERNEL
};

class CodechalEncodeWP;
#if USE_CODECHAL_DEBUG_TOOL
class CodechalDebugEncodePar;
#endif

//!
//! \class    CodechalEncoderGenState
//! \brief    Codechal encoder gen state
//!
class CodechalEncoderGenState
{
public:
    CodechalEncoderGenState(CodechalEncoderState* encoder);

    virtual ~CodechalEncoderGenState() {}

    CodechalEncoderState*           m_encoder = nullptr;
    // Encoder private data
    CodechalHwInterface*            m_hwInterface = nullptr;
    PMOS_INTERFACE                  m_osInterface = nullptr;
    CodechalDebugInterface*         m_debugInterface = nullptr;
    MhwMiInterface*                 m_miInterface = nullptr;                   //!< Common Mi Interface
    MhwRenderInterface*             m_renderEngineInterface = nullptr;               //!< Render engine interface
    PMHW_STATE_HEAP_INTERFACE       m_stateHeapInterface = nullptr;                  //!< State heap interface

    virtual MOS_STATUS SetCurbeMe(
        MeCurbeParams* params)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SendMeSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MeSurfaceParams* params)
    {
        return MOS_STATUS_SUCCESS;
    }
};

//!
//! \class CodechalEncoderState
//! \brief This base class defines the common member fields, functions etc used by all encoder.
//!
class CodechalEncoderState : public Codechal
{
public:
    //!
    //! \enum     RefId
    //! \brief    Reference id
    //!
    enum RefId
    {
        CODECHAL_ENCODE_REF_ID_0 = 0,
        CODECHAL_ENCODE_REF_ID_1 = 1,
        CODECHAL_ENCODE_REF_ID_2 = 2,
        CODECHAL_ENCODE_REF_ID_3 = 3,
        CODECHAL_ENCODE_REF_ID_4 = 4,
        CODECHAL_ENCODE_REF_ID_5 = 5,
        CODECHAL_ENCODE_REF_ID_6 = 6,
        CODECHAL_ENCODE_REF_ID_7 = 7,
    };

    // this is used for Mpeg2 encoding BRC as well,
    // therefore, the last entry is for Mpeg2
    const uint8_t m_bMeMethodGeneric[NUM_TARGET_USAGE_MODES + 1] = {0, 4, 4, 6, 6, 6, 6, 4, 7};

    const uint8_t m_meMethodGeneric[NUM_TARGET_USAGE_MODES + 1] = {0, 4, 4, 6, 6, 6, 6, 4, 7};

    const uint32_t m_superCombineDistGeneric[NUM_TARGET_USAGE_MODES + 1] ={0, 1, 1, 5, 5, 5, 9, 9, 0};

    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISCBR                      = 0x0010;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISVBR                      = 0x0020;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISAVBR                     = 0x0040;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISCQL                      = 0x0080;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_FIELD_PIC                  = 0x0100;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISICQ                      = 0x0200;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISVCM                      = 0x0400;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_IGNORE_PICTURE_HEADER_SIZE = 0x2000;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_ISQVBR                     = 0x4000;
    static constexpr uint32_t CODECHAL_ENCODE_BRCINIT_DISABLE_MBBRC              = 0x8000;
    static constexpr uint32_t m_numLaDataEntry                                   = 128;  //!< number of entries in lookahead data buffer and lookahead stats buffer

    // SearchPath Table, index [CodingType][MEMethod][]
    const uint32_t m_encodeSearchPath[2][8][16] =
    {
        // I-Frame & P-Frame
        {
            // MEMethod: 0
            {
                0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
                0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            },
        // MEMethod: 1
            {
                0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
                0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            },
        // MEMethod: 2
            {
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
                0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            },
        // MEMethod: 3
            {
                0x01010101, 0x11010101, 0x01010101, 0x11010101, 0x01010101, 0x11010101, 0x01010101, 0x11010101,
                0x01010101, 0x11010101, 0x01010101, 0x00010101, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            },
        // MEMethod: 4
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 5
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 6
            {
                0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
                0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            },
        // MEMethod: 7 used for mpeg2 encoding P frames
            {
                0x1F11F10F, 0x2E22E2FE, 0x20E220DF, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x02F1F1F1, 0x1F201111,
                0xF1EFFF0C, 0xF01104F1, 0x10FF0A50, 0x000FF1C0, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }
        },
        // B-Frame
        {
            // MEMethod: 0
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 1
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 2
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 3
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 4
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 5
            {
                0x0101F00F, 0x0F0F1010, 0xF0F0F00F, 0x01010101, 0x10101010, 0x0F0F0F0F, 0xF0F0F00F, 0x0101F0F0,
                0x01010101, 0x10101010, 0x0F0F1010, 0x0F0F0F0F, 0xF0F0F00F, 0xF0F0F0F0, 0x00000000, 0x00000000
            },
        // MEMethod: 6
            {
                0x120FF10F, 0x1E22E20D, 0x20E2FF10, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x4EF1F1F1, 0xF1F21211,
                0x0DFFFFE0, 0x11201F1F, 0x1105F1CF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            },
        // MEMethod: 7 used for mpeg2 encoding B frames
            {
                0x1F11F10F, 0x2E22E2FE, 0x20E220DF, 0x2EDD06FC, 0x11D33FF1, 0xEB1FF33D, 0x02F1F1F1, 0x1F201111,
                0xF1EFFF0C, 0xF01104F1, 0x10FF0A50, 0x000FF1C0, 0x00000000, 0x00000000, 0x00000000, 0x00000000
            }
        }
    };

    // ME kernel
    enum
    {
        CODECHAL_ENCODE_ME_IDX_P     = 0,
        CODECHAL_ENCODE_ME_IDX_B     = 1,
        CODECHAL_ENCODE_ME_IDX_VDENC = 1,
        CODECHAL_ENCODE_ME_IDX_NUM   = 2
    };

    //!
    //! \struct    MediaObjectInlineData
    //! \brief     Media object inline data
    //!
    struct MediaObjectInlineData
    {
        // uint32_t 0
        union
        {
            struct
            {
                uint32_t   mbHorizontalOrigin : 8;    // in MB unit
                uint32_t   mbVerticalOrigin   : 8;    // in MB unit
                uint32_t                      : 16;
            };
            // For BRC Block Copy kernels
            struct
            {
                uint32_t   blockHeight        : 16;
                uint32_t   bufferOffset       : 16;
            };
            struct
            {
                uint32_t   value;
            };
        } DW0;
    };

    //!
    //! \brief    Constructor
    //!
    CodechalEncoderState(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncoderState();

    // Encoder private data
    MhwVdboxMfxInterface*           m_mfxInterface = nullptr;                       //!< Mfx Interface
    MhwVdboxHcpInterface*           m_hcpInterface = nullptr;                       //!< Hcp Interface
    MhwVdboxHucInterface*           m_hucInterface = nullptr;                       //!< Huc Interface
    MhwVdboxVdencInterface*         m_vdencInterface = nullptr;                     //!< Vdenc Interface
    MhwMiInterface*                 m_miInterface = nullptr;                        //!< Mi Interface
    MhwRenderInterface*             m_renderEngineInterface = nullptr;              //!< Render engine interface
    PMHW_STATE_HEAP_INTERFACE       m_stateHeapInterface = nullptr;                 //!< State heap interface
    CodechalEncodeAllocator*        m_allocator = nullptr;                          //!< Resource allocator
    CodechalEncodeTrackedBuffer*    m_trackedBuf = nullptr;                         //!< Tracked buffer state

    PLATFORM                        m_platform = {};                                //!< The platorm info
    MEDIA_FEATURE_TABLE             *m_skuTable = nullptr;                          //!< SKU table
    MEDIA_WA_TABLE                  *m_waTable = nullptr;                           //!< WA table
    CodecHalMmcState*               m_mmcState = nullptr;                           //!< Memory compression
    MEDIA_SYSTEM_INFO               *m_gtSystemInfo = nullptr;                      //!< GT system infomation
    MOS_GPU_NODE                    m_videoGpuNode = MOS_GPU_NODE_MAX;              //!< GPU node of video
    MOS_GPU_CONTEXT                 m_videoContext = MOS_GPU_CONTEXT_INVALID_HANDLE;              //!< GPU context of video
    MOS_GPU_CONTEXT                 m_videoContextExt[4];                           //!< Extand GPU context
    MOS_GPU_CONTEXT                 m_renderContext = MOS_GPU_CONTEXT_INVALID_HANDLE;             //!< GPU context of render
    bool                            m_pakEnabled = false;                           //!< flag to indicate if PAK is enabled
    bool                            m_encEnabled = false;                           //!< flag to indicate if ENC is enabled
    bool                            m_videoNodeAssociationCreated = false;          //!< flag to indicate if video node association is created
    bool                            m_computeContextEnabled = false;                //!< flag to indicate if compute context is enabled
    bool                            m_needCheckCpEnabled = false;                   //!< Indicate if checking cp is needed when prepare default nodes
    bool                            m_vdboxOneDefaultUsed = false;                  //!< Indicate VDBOX 1 is always used when prepare default nodes

    CodechalEncoderGenState*        m_encoderGenState = nullptr;          //!< Pointer to Gen Specific Encoder State
    CodechalEncodeCscDs*            m_cscDsState = nullptr;               //!< pointer to CSC Downscaling state
    CodechalEncodeWP*               m_wpState = nullptr;                  //!< pointer to weighted prediction state
    CODECHAL_FUNCTION               m_codecFunction = CODECHAL_FUNCTION_INVALID;                  //!< The encode state's codec function used
    uint32_t                        m_standard = 0;                       //!< The encode state's standard
    uint32_t                        m_mode = 0;                           //!< The encode mode
    MHW_WALKER_MODE                 m_walkerMode = MHW_WALKER_MODE_NOT_SET;                       //!< The encode walker's mode
    uint8_t                         m_kernelMode = 0;                     //!< normal, performance, quality.

    bool                            m_mfeEnabled = false;                //!< Mfe enabled
    bool                            m_mfeMbEncEanbled = false;           //!< Mfe MBEnc kernel enabled
    bool                            m_mfeLastStream = false;             //!< Is last stream during this submission
    bool                            m_mfeFirstStream = false;            //!< Is first stream during this submission
    bool                            m_mfeInitialized = false;            //!< Used for initializing MFE resources during first execute
    MfeParams                       m_mfeEncodeParams = {};              //!< Mfe encode params during this submission
    MfeSharedState *                m_mfeEncodeSharedState = nullptr;    //!< shared state from the parent context

    // Common Kernel Parameters
    uint8_t*                        m_kernelBase = nullptr;              //!< Kernel base address
    uint32_t                        m_kuid = 0;                          //!< Kernel unified ID

    // Per-frame Application Settings
    EncoderParams                   m_encodeParams = {};          //!< Encode parameters used in each frame
    uint32_t                        *m_dataHwCount = nullptr;     //!< HW count data
    MOS_RESOURCE                     m_resHwCount = {};                //!< Resource of HW count
    MOS_SURFACE                     m_prevRawSurface = {};        //!< Pointer to MOS_SURFACE of previous raw surface
    MOS_SURFACE                     m_rawSurface = {};            //!< Pointer to MOS_SURFACE of raw surface
    MOS_SURFACE                     m_reconSurface = {};          //!< Pointer to MOS_SURFACE of reconstructed surface
    MOS_RESOURCE                    m_resBitstreamBuffer = {};         //!< Pointer to MOS_SURFACE of bitstream surface
    MOS_RESOURCE                    m_resMbCodeSurface = {};           //!< Pointer to MOS_SURFACE of MbCode surface
    MOS_RESOURCE                    m_resMvDataSurface = {};           //!< Pointer to MOS_SURFACE of MvData surface
    uint32_t                        m_mbDataBufferSize = 0;
    HwCounter                       m_regHwCount[CODECHAL_ENCODE_STATUS_NUM + 1];    //!< HW count register value

    CODEC_PICTURE                   m_currOriginalPic = {};       //!< Raw.
    CODEC_PICTURE                   m_currReconstructedPic = {};  //!< Recon.
    uint16_t                        m_pictureCodingType = 0;      //!< I, P, or B frame
    int16_t                         m_frameNum = 0;               //!< Frame number
    bool                            m_firstFrame = true;          //!< Flag to indicate if it is first frame
    bool                            m_firstTwoFrames = false;     //!< Flag to indicate if they are first two frames
    bool                            m_firstField = true;          //!< Flag to indicate if it is first field
    bool                            m_resolutionChanged = false;  //!< Flag to indicate if resolution is changed
    bool                            m_scalingEnabled = false;     //!< 4x Scaling kernel needs to be called for this frame
    bool                            m_2xScalingEnabled = false;   //!< 2x Scaling kernel only used by HEVC now
    bool                            m_useRawForRef = false;       //!< Flag to indicate if using raw surface for reference
    bool                            m_disableReconMMCD = false;   //!< disable Recon surface's MMC
    bool                            m_repeatFrame = false;        //!< Flag to indicate if current frame is repeat frame
    bool                            m_pollingSyncEnabled = false; //!< Flag to indicate if GPU polling based sync for raw surface copy is enabled
    uint32_t                        m_syncMarkerOffset = 0;       //!< Sync marker offset in raw surface for GPU polling based sync
    uint32_t                        m_syncMarkerValue = 0;        //!< Sync marker value in raw surface for GPU polling based sync
    uint8_t                         m_prevReconFrameIdx = 0;      //!< Previous reconstruct frame index
    uint8_t                         m_currReconFrameIdx = 0;      //!< Current reconstruct frame index

    uint32_t                        m_frameWidth = 0;             //!< Frame width in luma samples
    uint32_t                        m_frameHeight = 0;            //!< Frame height in luma samples
    uint32_t                        m_frameFieldHeight = 0;       //!< Frame height in luma samples
    uint32_t                        m_oriFrameHeight = 0;         //!< Original frame height
    uint32_t                        m_oriFrameWidth = 0;          //!< Original frame width
    uint32_t                        m_createWidth = 0;            //!< Max Frame Width for resolution reset
    uint32_t                        m_createHeight = 0;           //!< Max Frame Height for resolution reset
    uint16_t                        m_picWidthInMb = 0;           //!< Picture Width in MB width count
    uint16_t                        m_picHeightInMb = 0;          //!< Picture Height in MB height count
    uint16_t                        m_frameFieldHeightInMb = 0;   //!< Frame/field Height in MB
    uint32_t                        m_bitstreamUpperBound = 0;    //!< Bitstream upper bound
    uint8_t                         m_oriFieldCodingFlag = 0;     //!< Original field coding flag
    uint32_t                        m_maxBtCount = 0;             //!< Max bt count
    bool                            m_cmKernelEnable = false;     //!< Flag to indicate if cm kernel is enabled
    bool                            m_feiEnable = false;          //!< FEI is enabled

    // Synchronization
    MOS_RESOURCE                    m_resSyncObjectRenderContextInUse = {}; //!< Resource of the sync object to indicate render context is in use
    MOS_RESOURCE                    m_resSyncObjectVideoContextInUse = {};  //!< Resource of the sync object to indicate video context is in use
    bool                            m_waitForPak = false;              //!< Flag to indicate if waiting for PAK
    bool                            m_signalEnc = false;               //!< Flag used to signal ENC
    uint32_t                        m_semaphoreObjCount = 0;           //!< Count of semaphore objects
    uint32_t                        m_semaphoreMaxCount = 0;           //!< Max count of semaphore

    // Status Reporting
    bool                            m_codecGetStatusReportDefined = false;          //!< Need to be set to true by any codec/gen that has their own impleementation.
    uint32_t                        m_storeData = 0;                                //!< Stored data
    bool                            m_statusQueryReportingEnabled = false;                            //!< Flag to indicate if we support eStatus query reporting on current Platform
    EncodeStatusBuffer              m_encodeStatusBuf = {};                         //!< Stores all the status_query related data for PAK engine
    EncodeStatusBuffer              m_encodeStatusBufRcs = {};                      //!< Stores all the status_query related data for render ring (RCS)
    MHW_VDBOX_IMAGE_STATUS_CONTROL  m_imgStatusControlBuffer;                       //!< Stores image eStatus control data
    uint32_t                        m_statusReportFeedbackNumber = 0;               //!< Status report feed back number
    bool                            m_frameTrackingEnabled = false;                 //!< Flag to indicate if we enable KMD frame tracking
    uint32_t                        m_numberTilesInFrame = 0;                       //!< Track number of tiles per frame
    bool                            m_inlineEncodeStatusUpdate = false;             //!< check whether use inline encode status update or seperate BB
    AtomicScratchBuffer             m_atomicScratchBuf = {};                             //!< Stores atomic operands and result
    bool                            m_skipFrameBasedHWCounterRead = false;          //!< Skip reading Frame base HW counter for status report
    bool                            m_disableStatusReport = false;                  //!< Indicate status report is not needed.

    // Shared Parameters
    BSBuffer                        m_bsBuffer = {};                                //!< Bitstream buffer
    bool                            m_newSeqHeader = false;                         //!< New sequence header flag
    bool                            m_newPpsHeader = false;                         //!< New PPS header flag
    bool                            m_newVuiData = false;                           //!< New Vui data flag
    bool                            m_newSeq = false;                               //!< New sequence flag
    bool                            m_lastPicInSeq = false;                         //!< Flag to indicate if it is last picture in sequence
    bool                            m_lastPicInStream = false;                      //!< Flag to indicate if it is last picture in stream
    uint8_t                         m_numRefPair = 0;                               //!< number of reference pair (forward & backward)
    uint8_t                         m_numPasses = 0;                                //!< Number passes
    uint8_t                         m_currPass = 0;                                 //!< Current pass
    bool                            m_forceSinglePakPass = false;                   //!< Flag to enable forcing single pak pass
    bool                            m_useCmScalingKernel = false;                   //!< Flag to use cm scaling kernel
    bool                            m_useMwWlkrForAsmScalingKernel = false;         //!< Use media walker for ASM scaling kernel flag
    bool                            m_combinedDownScaleAndDepthConversion = false;   //!< Combied downscale and depth conversion
    uint32_t                        m_brcPakStatisticsSize = 0;                     //!< Bitrate control PAK statistics size
    uint32_t                        m_brcHistoryBufferSize = 0;                     //!< Bitrate control history buffer size
    uint32_t                        m_mbencBrcBufferSize = 0;                       //!< Mbenc bitrate control buffer size
    uint8_t                         m_numVdbox = 0;                                 //!< Number of vdbox
    uint8_t                         m_numUsedVdbox = 0;                             //!< Number of vdbox used

    // ENC/PAK batch buffer and surface indices
    PCODEC_REF_LIST                 m_currRefList = nullptr;        //!< Current reference list
    uint8_t                         m_currRecycledBufIdx = 0;       //!< Current recycled buffer index
    uint8_t                         m_currEncBbSet = 0;             //!< Current encode bb set
    uint8_t                         m_currMbCodeIdx = 0;            //!< Current mb code index
    uint8_t                         m_currMadBufferIdx = 0;         //!< Current mad buffer
    uint32_t                        m_recycledBufStatusNum[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {0};  //!< Recycled buffer status num list
    uint32_t                        m_recycledBufWaitMs = 0;        //!< Recycled buffer wait (ms)

    // User Feature Key Capabilities
    bool                            m_hmeSupported = false;               //!< Flag to indicate if HME is supported
    bool                            m_noMeKernelForPFrame = false;        //!< HEVC does not have P-frame, no need to load P-frame ME kernel
    bool                            m_16xMeSupported = false;             //!< 16x ME supported
    bool                            m_16xMeUserfeatureControl = false;    //!< User feature control if 16x ME is supported
    bool                            m_32xMeSupported = false;             //!< 32x ME supported
    bool                            m_useNonLegacyStreamin = false;       //!< Use non-legacy stream in
    bool                            m_32xMeUserfeatureControl = false;    //!< User feature control if 32x ME is supported
    bool                            m_2xMeSupported = false;              //!< 2x DS surface, currently only used by Gen10 Hevc Encode
    bool                            m_suppressReconPicSupported = false;  //!< Suppress reconstructed picture supported flag
    bool                            m_useHwScoreboard = true;             //!< Flag to indicate if HW score board is used
    bool                            m_hwWalker = false;                   //!< HW walker used
    bool                            m_panicEnable                = false;                   //!< Rc panic enabled flag
    bool                            m_sliceShutdownEnable = false;        //!< Slice Shutdown Enable
    uint32_t                        m_encodeVfeMaxThreads = 0;            //!< Encode vfe max threads number
    uint32_t                        m_encodeVfeMaxThreadsScaling = 0;     //!< Encode vfe max threads scaling number
    uint32_t                        m_hwScoreboardType = 0;               //!< HW score board type
    bool                            m_flatnessCheckSupported = false;     //!< Flatness check supported flag
    bool                            m_groupIdSelectSupported = false;     //!< Group id select supported flag
    uint8_t                         m_groupId = 0;                        //!< Group id
    bool                            m_multipassBrcSupported = false;      //!< Multi-pass bitrate control supported flag
    uint8_t                         m_targetUsageOverride = 0;            //!< Target usage override
    bool                            m_userFeatureKeyReport = false;       //!< User feature key report flag
    bool                            m_singlePassDys = false;              //!< sungle pass dynamic scaling supported flag

    // CmdGen HuC FW for HEVC/VP9 VDEnc
    MOS_RESOURCE                    m_resVdencCmdInitializerDmemBuffer = {};   //!< Resource of vdenc command initializer DMEM buffer
    MOS_RESOURCE                    m_resVdencCmdInitializerDataBuffer[2];   //!< Resource of vdenc command initializer data buffer

    // VDEnc params
    bool                            m_vdencEnabled = false;               //!< Vdenc enabled flag
    bool                            m_vdencBrcEnabled = false;            //!< Vdenc bitrate control enabled flag
    bool                            m_vdencStreamInEnabled = false;       //!< Vdenc stream in enabled flag
    bool                            m_vdencNoTailInsertion = false;       //!< Vdenc no tail insertion enabled flag
    uint32_t                        m_vdencBrcStatsBufferSize = 0;        //!< Vdenc bitrate control buffer size
    uint32_t                        m_vdencBrcPakStatsBufferSize = 0;     //!< Vdenc bitrate control PAK buffer size
    uint32_t                        m_vdencBrcNumOfSliceOffset = 0;       //!< Vdenc bitrate control number of slice offset
    bool                            m_waReadVDEncOverflowStatus = false;  //!< Read vdenc overflow status used flag
    bool                            m_vdencBrcImgStatAllocated = false;   //!< Vdenc bitrate control image state allocated flag

    // VDEnc dynamic slice control params
    uint32_t                       m_vdencFlushDelayCount = 0;   //!< Vdenc flush delay count

    HMODULE                        m_swBrcMode = nullptr;        //!< Software bitrate control mode

    // IQmatrix params
    bool                            m_picQuant = false;          //!< picture quant
    bool                            m_newQmatrixData = false;    //!< New Qmatrix data
    PCODEC_ENCODER_SLCDATA          m_slcData = nullptr;         //!< record slice header size & position
    uint32_t                        m_numSlices = 0;             //!< Number of slices
    uint32_t                        m_numHuffBuffers = 0;        //!< Number of Huffman buffers

    // ENC input/output buffers
    uint32_t                        m_mbCodeStrideInDW = 0;         //!< Offset + Size of MB + size of MV
    uint32_t                        m_mbCodeOffset = 0;             //!< MB data offset
    uint32_t                        m_mvOffset = 0;                 //!< MV data offset, in 64 byte
    uint32_t                        m_mbCodeSize = 0;               //!< MB code buffer size
    uint32_t                        m_mbcodeBottomFieldOffset = 0;  //!< MB code offset frame/TopField - zero, BottomField - nonzero
    uint32_t                        m_mvDataSize = 0;               //!< MV data size
    uint32_t                        m_mvBottomFieldOffset = 0;      //!< MV data offset frame/TopField - zero, BottomField - nonzero
    MOS_RESOURCE                    m_resDistortionBuffer = {};          //!< MBEnc Distortion Buffer
    MOS_RESOURCE                    m_resMadDataBuffer[CODECHAL_ENCODE_MAX_NUM_MAD_BUFFERS]; //!< Buffers to store Mean of Absolute Differences
    bool                            m_madEnabled = false;                                    //!< Mad enabled flag

    bool                            m_arbitraryNumMbsInSlice = false;                        //!< Flag to indicate if the sliceMapSurface needs to be programmed or not
    MOS_SURFACE                     m_sliceMapSurface[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];  //!< Slice map surface
    uint32_t                        m_sliceMapBottomFieldOffset = 0;                         //!< Slice map bottom field offset

    // VDENC and PAK Data Buffer
    PMOS_RESOURCE                   m_resVdencStatsBuffer = nullptr;               //!< Resource of Vdenc status buffer
    PMOS_RESOURCE                   m_resVdencCuObjStreamOutBuffer = nullptr;      //!< Resource of Vdenc Cu object stream out buffer
    PMOS_RESOURCE                   m_resVdencPakObjCmdStreamOutBuffer = nullptr;  //!< Resource of Vdenc Pak object command stream out buffer
    PMOS_RESOURCE                   m_resPakStatsBuffer = nullptr;                 //!< Resource of Pak status buffer
    PMOS_RESOURCE                   m_resSliceCountBuffer = nullptr;               //!< Resource of slice count buffer
    PMOS_RESOURCE                   m_resVdencModeTimerBuffer = nullptr;           //!< Resource of Vdenc mode timer buffer

    // VDEnc StreamIn Buffer
    MOS_RESOURCE                    m_resVdencStreamInBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];  //!< Resources of Vdenc stream in buffer

    // Maximum number of slices allowed by video spec
    uint32_t                        m_maxNumSlicesAllowed = 0;          //!< Max number of slices allowed

    //VDEnc HuC FW status
    MOS_RESOURCE                    m_resPakMmioBuffer = {};                 //!< Resource of PAK MMIO buffer
    MOS_RESOURCE                    m_resHucErrorStatusBuffer = {};          //!< Resource of Huc Error Status buffer
    MOS_RESOURCE                    m_resHucStatus2Buffer = {};              //!< Resource of HuC status 2 buffer
    MOS_RESOURCE                    m_resHucFwBuffer = {};                   //!< Resource of HuC Fw buffer
    PMOS_RESOURCE                   m_resVdencBrcUpdateDmemBufferPtr[2] = {nullptr, nullptr}; //!< One for 1st pass of next frame, and the other for the next pass of current frame.

    // PAK Scratch Buffers
    MOS_RESOURCE                    m_resDeblockingFilterRowStoreScratchBuffer = {};                 //!< Handle of deblock row store surface
    MOS_RESOURCE                    m_resMPCRowStoreScratchBuffer = {};                              //!< Handle of mpc row store surface
    MOS_RESOURCE                    m_resStreamOutBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];  //!< Handle of streamout data surface

    // Scaling
    MHW_KERNEL_STATE                m_scaling4xKernelStates[CODEC_NUM_FIELDS_PER_FRAME];  //!< Scaling 4x Kernel States
    ScalingBindingTable             m_scaling4xBindingTable = {};                        //!< Scaling 4x Binding Table
    MHW_KERNEL_STATE                m_scaling2xKernelStates[CODEC_NUM_FIELDS_PER_FRAME];  //!< Scaling 2x Kernel States
    ScalingBindingTable             m_scaling2xBindingTable = {};                        //!< Scaling 2x Binding Table
    uint32_t                        m_scalingCurbeSize = 0;                               //!< Scaling curbe size
    bool                            m_interlacedFieldDisabled = false;                    //!< interlaced field disabled flag
    CodechalEncodeBbuf              m_scalingBBUF[CODECHAL_ENCODE_VME_BBUF_NUM];          //!< This Batch Buffer is used for scaling kernel.
    uint32_t                        m_scaledBottomFieldOffset = 0;                        //!< Scaled Bottom Field Offset
    uint32_t                        m_scaled16xBottomFieldOffset = 0;                     //!< Scaled 16x Bottom Field Offset
    uint32_t                        m_scaled32xBottomFieldOffset = 0;                     //!< Scaled 32x Bottom Field Offset
    uint32_t                        m_scalingBBufIdx = 0;                                 //!< Scaling batch buffer index
    uint8_t                         m_minScaledDimension = 0;                             //!< min scaled dimension
    uint8_t                         m_minScaledDimensionInMb = 0;                         //!< min scaled dimension in Mb
    uint32_t                        m_downscaledWidth2x = 0;                              //!< Downscale width 2x
    uint32_t                        m_downscaledHeight2x = 0;                             //!< Downscale height 2x
    uint32_t                        m_downscaledWidth4x = 0;                              //!< Downscale width 4x
    uint32_t                        m_downscaledHeight4x = 0;                             //!< Downscale height 4x
    uint32_t                        m_downscaledWidthInMb4x = 0;                          //!< Downscale width in Mb 4x
    uint32_t                        m_downscaledHeightInMb4x = 0;                         //!< Downscale height in Mb 4x
    uint32_t                        m_downscaledFrameFieldHeightInMb4x = 0;               //!< Downscale frame field height in Mb 4x
    uint32_t                        m_downscaledWidth16x = 0;                             //!< Downscale width 16x
    uint32_t                        m_downscaledHeight16x = 0;                            //!< Downscale height 16x
    uint32_t                        m_downscaledWidthInMb16x = 0;                         //!< Downscale width in Mb 16x
    uint32_t                        m_downscaledHeightInMb16x = 0;                        //!< Downscale height in Mb 16x
    uint32_t                        m_downscaledFrameFieldHeightInMb16x = 0;              //!< Downscale frame field height in Mb 16x
    uint32_t                        m_downscaledWidth32x = 0;                             //!< Downscale width 2x
    uint32_t                        m_downscaledHeight32x = 0;                            //!< Downscale height 2x
    uint32_t                        m_downscaledWidthInMb32x = 0;                         //!< Downscale width 2x
    uint32_t                        m_downscaledHeightInMb32x = 0;                        //!< Downscale height 2x
    uint32_t                        m_downscaledFrameFieldHeightInMb32x = 0;              //!< Downscale frame field height in Mb 32x

    bool                            m_fieldScalingOutputInterleaved = false;              //!< Field scaling output interleaved flag
    MOS_SURFACE                     m_flatnessCheckSurface = {};                          //!< Flatness check surface
    uint32_t                        m_flatnessCheckBottomFieldOffset = 0;                 //!< Flatness check bottom field offset
    bool                            m_flatnessCheckEnabled = false;                       //!< Flatness check enabled flag
    bool                            m_mbStatsEnabled = false;                             //!< MB status enabled flag
    bool                            m_adaptiveTransformDecisionEnabled = false;                             //!< Adaptive Transform Decision Enabled flag
    bool                            m_forceBrcMbStatsEnabled           = false;                             //!< Force Brc Mb statistics Enabled flag
    uint32_t                        m_mbvProcStatsBottomFieldOffset    = 0;                                 //!< MB VProc statistics Bottom Field Offset
    CODECHAL_ENCODE_BUFFER          m_resMbStatisticsSurface;                                               //!< Resource of Mb statistics surface
    bool                            m_mbStatsSupported = false;                           //!< Mb statistics supported flag
    MOS_RESOURCE                    m_resMbStatsBuffer = {};                                   //!< Resource of Mb statistics buffer
    uint32_t                        m_mbStatsBottomFieldOffset = 0;                       //!< Mb statistics bottom field offset

    // ME
    MHW_KERNEL_STATE                m_meKernelStates[CODECHAL_ENCODE_ME_IDX_NUM];     //!< ME kernel states
    MeKernelBindingTable            m_meBindingTable = {};                            //!< ME binding table
    bool                            bStreamOutEnable = false;
    MOS_RESOURCE                    StreamOutBuffer = {};               // StreamOut buffer

    //GVA paramters to change inter and intra rounding
    bool                            bCoeffRoundTag = false;
    uint32_t                        uiRoundIntra = 0;
    uint32_t                        uiRoundInter = 0;

    // Ds+Copy kernel optimization
    uint8_t                         m_outputChromaFormat = (uint8_t)HCP_CHROMA_FORMAT_YUV420;     //!< 1: 420 2: 422 3: 444
    bool                            m_gopIsIdrFrameOnly = false;                                  //!< GOP structure contains I-frame only
    PMOS_SURFACE                    m_rawSurfaceToEnc = nullptr;                                  //!< raw surf to enc
    PMOS_SURFACE                    m_rawSurfaceToPak = nullptr;                                  //!< raw surf to pak

    // HME VDEnc
    GenericBindingTable             m_vdencMeKernelBindingTable = {};   //!< Vdenc ME kernel binding table
    MHW_KERNEL_STATE                m_vdencMeKernelState;               //!< Vdenc ME kernel state for Low Delay B
    MHW_KERNEL_STATE                m_vdencMeKernelStateRAB = {};       //!< Vdenc ME kernel state for Random Access B

    GenericBindingTable             m_vdencStreaminKernelBindingTable = {};  //!< Vdenc stream in kernel binding table
    MHW_KERNEL_STATE                m_vdencStreaminKernelState;         //!< Vdenc stream in kernel state for Low Delay B
    MHW_KERNEL_STATE                m_vdencStreaminKernelStateRAB;      //!< Vdenc stream in kernel state for Random Access B

    // Common kernel
    uint32_t                        m_kuidCommon = 0;                    //!< Common kernel UID
    bool                            m_useCommonKernel = false;           //!< Use common kernel
    bool                            m_wpUseCommonKernel = false;         //!< WP uses common kernel

    // Generic ENC parameters
    uint32_t                        m_verticalLineStride = 0;            //!< vertical line stride
    uint32_t                        m_verticalLineStrideOffset = 0;      //!< vertical line stride offset

    // CMD buffer sizes
    uint32_t                        m_pictureStatesSize = 0;             //!< Picture states size
    uint32_t                        m_extraPictureStatesSize = 0;        //!< Picture states size extra
    uint32_t                        m_sliceStatesSize = 0;               //!< Slice states size
    uint32_t                        m_vmeStatesSize = 0;                 //!< VME states size
    uint32_t                        m_hucCommandsSize = 0;               //!< HuC command size

    // Patch List Size
    uint32_t                        m_picturePatchListSize = 0;          //!< Picture patch list size
    uint32_t                        m_extraPicturePatchListSize = 0;     //!< Picture patch list size extra
    uint32_t                        m_slicePatchListSize = 0;            //!< Slice patch list size
    uint32_t                        m_vmePatchListSize = 0;              //!< not used yet, for future development

    // Single Task Phase parameters
    bool                            m_singleTaskPhaseSupported = false;      //!< Single task phase supported flag
    bool                            m_firstTaskInPhase = false;              //!< first task in phase flag
    bool                            m_lastTaskInPhase = false;               //!< last task in phase flag
    bool                            m_lastEncPhase = false;                  //!< first enc phase flag
    bool                            m_singleTaskPhaseSupportedInPak = false; //!< Single task phase supported in pak flag
    uint32_t                        m_headerBytesInserted = 0;               //!< Header bytes inserted flag

    // Null Rendering Flags
    bool                            m_videoContextUsesNullHw = false;        //!< Using null HW flags for video context
    bool                            m_renderContextUsesNullHw = false;       //!< Using null HW flags for render context

    // Slice Shutdown parameters
    bool                            m_setRequestedEUSlices = false;          //!< Flag to indicate the need to set requested EU slices
    bool                            m_setRequestedSubSlices = false;         //!< Flag to indicate the need to set requested EU sub-slices
    bool                            m_setRequestedEUs = false;               //!< Flag to indicate the need to set requested EUs
    uint32_t                        m_sliceShutdownDefaultState = 0;         //!< Default state of slice shutdown
    uint32_t                        m_sliceShutdownRequestState = 0;         //!< Request state of slice shutdown
    uint32_t                        m_ssdResolutionThreshold = 0;            //!< Resolution threshold for slice shutdown
    uint32_t                        m_ssdTargetUsageThreshold = 0;           //!< Target usage threshold for slice shutdown
    uint32_t                        m_targetUsage = 0;                       //!< Target usage

    // Skip frame params
    uint8_t                         m_skipFrameFlag = 0;        //!< Skip frame flag
    uint32_t                        m_numSkipFrames = 0;        //!< Number of skip frame
    uint32_t                        m_sizeSkipFrames = 0;       //!< acccumulative size of skipped frames for skipflag = 2
    uint32_t                        m_sizeCurrSkipFrame = 0;    //!< size of curr skipped frame for skipflag = 2

    // Lookahead
    uint8_t                         m_lookaheadDepth = 0;       //!< Number of frames to lookahead
    uint8_t                         m_currLaDataIdx = 0;        //!< Current lookahead data index
    uint32_t                        m_averageFrameSize = 0;     //!< Average frame size based on targed bitrate and frame rate, in unit of bits
    uint32_t                        m_prevTargetFrameSize = 0;  //!< Target frame size of previous frame.
    uint32_t                        m_targetBufferFulness = 0;  //!< Target encode buffer fulness in bits, used by BRC and calculated from initial buffer fulness, target frame size (from DDI) and average frame size

    MHW_VDBOX_NODE_IND              m_vdboxIndex = MHW_VDBOX_NODE_MAX;               //!< Index of vdbox
    MediaPerfProfiler               *m_perfProfiler = nullptr;  //!< Performance data profiler
    PMOS_GPUCTX_CREATOPTIONS        m_gpuCtxCreatOpt = nullptr; //!< Used for creating GPU context
    bool                            intraModeMaskControl = false;
    uint32_t                        intraModeMask = 0;  // to disable intra mode
    bool                            interMbTransformSizeControl = false;
    bool                            interMbTransform8x8Enabled = false;

    PMOS_RESOURCE                   presMbInlineData = nullptr;
    PMOS_RESOURCE                   presMbConstSurface = nullptr;
    PMOS_RESOURCE                   presVMEOutSurface = nullptr;

#if (_DEBUG || _RELEASE_INTERNAL)
    bool m_mmcUserFeatureUpdated;  //!< indicate if the user feature is updated with MMC state
#endif

    CmDevice *m_cmDev     = nullptr;
    CmTask *  m_cmTask    = nullptr;
    CmQueue * m_cmQueue   = nullptr;
    CmDevice *m_origCmDev = nullptr;

#define CM_EVENT_NUM 128
    CmEvent *m_cmEvent[CM_EVENT_NUM] = {nullptr};
    short    m_cmEventIdx = 0;  // current  event idx
    short    m_cmEventCheckIdx = 0;

#ifdef FEI_ENABLE_CMRT
    CodechalEncodeMdfKernelResource resDSKernel;
#endif

    bool m_colorbitSupported = false;

#if USE_CODECHAL_DEBUG_TOOL
    CodechalDebugEncodePar          *m_encodeParState = nullptr;         //!< Encode Par state
#endif

    //!
    //! \brief  Entry to allocate and intialize the encode instance
    //! \param  [in] codecHalSettings
    //!         The settings to inialize the encode instance
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Allocate(CodechalSetting * codecHalSettings) override;

    //!
    //! \brief  The entry to encode each frame.
    //! \param  [in] params
    //!         Pointer to encode parameters of this frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute(void *params) override;

    //!
    //! \brief  The entry to get status report.
    //! \param  [out] status
    //!         The point to encode status
    //! \param  [in] numStatus
    //!         The requested number of status reports
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

    //!
    //! \brief  Read counter value for encode.
    //! \param  [in] index
    //!         The index of status report number
    //! \param  [in, out] encodeStatusReport
    //!         The address of encodeStatusReport
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadCounterValue(uint16_t index, EncodeStatusReport* encodeStatusReport);

    //!
    //! \brief  Initialize the encoder state
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(
        CodechalSetting * settings);

    //!
    //! \brief  Allocate resources for encoder
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief  Free resources in encoder
    //! \return void
    //!
    virtual void FreeResources();

    //!
    //! \brief  Initialize the picture
    //! \param  [in] params
    //!         Encoder parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializePicture(const EncoderParams& params) = 0;

    //!
    //! \brief  Execute kernel functions
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteKernelFunctions() = 0;

    //!
    //! \brief  Execute picture level in encoder
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecutePictureLevel() = 0;

    //!
    //! \brief  Execute slice level in encoder
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteSliceLevel() = 0;

    //!
    //! \brief  Get Status Report
    //! \details Each Codec need to define its own GetStatusReport
    //! \param  [out] encodeStatus
    //!         Encoder status
    //! \param  [out] encodeStatusReport
    //!         Encoder status report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(
        EncodeStatus* encodeStatus,
        EncodeStatusReport* encodeStatusReport) = 0;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureKeyReport();

    //!
    //! \brief    Help function to submit a command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] nullRendering
    //!           Null rendering flag
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        int32_t             nullRendering);

    //!
    //! \brief  Check Supported Format
    //! \param  [in] surface
    //!         Input surface to check
    //! \return bool
    //!         true if supported, false if not
    //!
    virtual bool CheckSupportedFormat(
        PMOS_SURFACE surface);

    //!
    //! \brief  Encode Copy Skip Frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EncodeCopySkipFrame()
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Read Image Status
    //! \param  [in, out] cmdBuffer
    //!         Input and output cmdbuffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadImageStatus(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Read Mfc Status
    //! \param  [in, out] cmdBuffer
    //!         Input and output cmdbuffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReadMfcStatus(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief  Send Prolog With Frame Tracking
    //! \param  [in, out] cmdBuffer
    //!         Input and output cmdbuffer
    //! \param  [in] frameTrackingRequested
    //!         frame Tracking Requested
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool frameTrackingRequested,
        MHW_MI_MMIOREGISTERS* mmioRegister = nullptr);

    //!
    //! \brief  Calculate Command Buffer Size
    //!
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual uint32_t CalculateCommandBufferSize();

    //!
    //! \brief  Prepare Nodes
    //! \param  [in, out] videoGpuNode
    //!         GPU node prepared
    //! \param  [in, out] setVideoNode
    //!         flag indicates if node needs to set
    //! \return void
    //!
    virtual void PrepareNodes(
        MOS_GPU_NODE& videoGpuNode,
        bool&         setVideoNode);

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetGpuCtxCreatOption();

    //!
    //! \brief  Sets video gpu context
    //!
    void SetVideoContext(MOS_GPU_CONTEXT videoContext) { m_videoContext = videoContext; }

    //!
    //! \brief  Create Gpu Contexts
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateGpuContexts();

    //!
    //! \brief  Verify Space Available
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS VerifySpaceAvailable();

    //!
    //! \brief  Add MEDIA_VFE command to command buffer
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] params
    //!         Parameters for send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params);

    //!
    //! \brief  Send Generic Kernel Cmds
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] params
    //!         Parameters for send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendGenericKernelCmds(
        PMOS_COMMAND_BUFFER   cmdBuffer,
        SendKernelCmdsParams *params);

    //!
    //! \brief  Start Status Report
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] encFunctionType
    //!         encFunctionType for send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StartStatusReport(
        PMOS_COMMAND_BUFFER cmdBuffer,
        CODECHAL_MEDIA_STATE_TYPE encFunctionType);

    //!
    //! \brief  End Status Report
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] encFunctionType
    //!         encFunctionType for send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EndStatusReport(
        PMOS_COMMAND_BUFFER cmdBuffer,
        CODECHAL_MEDIA_STATE_TYPE encFunctionType);

    //!
    //! \brief  Set Status Report Parameters
    //! \param  [in] currRefList
    //!         current RefList used to set
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetStatusReportParams(
        PCODEC_REF_LIST currRefList);

    //!
    //! \brief  Motion Estimation Disable Check
    //!
    //! \return void
    //!
    virtual void MotionEstimationDisableCheck();

    //!
    //! \brief  Execute the encode
    //!
    //! \param  [in] encodeParams
    //!         Encode parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteEnc(
        EncoderParams* encodeParams);

    //! \brief  Execute FEI PreEnc
    //!
    //! \param  [in] encodeParams
    //!         Encode parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecutePreEnc(
            EncoderParams* encodeParams) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief  Destroy Me Resources
    //!
    //! \param  [in] param
    //!         Hme parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyMeResources(
        HmeParams* param);

    //!
    //! \brief  Clean Up Resource
    //!
    //! \param  [in, out] resource
    //!         Resource to clean up
    //! \param  [in] allocParams
    //!         allocParams used to clean
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CleanUpResource(
        PMOS_RESOURCE            resource,
        PMOS_ALLOC_GFXRES_PARAMS allocParams);

    //!
    //! \brief  Allocate Resources 4x Me
    //!
    //! \param  [in] param
    //!         Hme parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources4xMe(
        HmeParams* param);

    //!
    //! \brief  Allocate Resources 16x Me
    //!
    //! \param  [in] param
    //!         Hme parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources16xMe(
        HmeParams* param);

    //!
    //! \brief  Allocate Resources 32x Me
    //!
    //! \param  [in] param
    //!         Hme parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources32xMe(
        HmeParams* param);

    //!
    //! \brief  Initialize Common
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitCommon();

    //!
    //! \brief  Resize Internal Buffer on Resolution Change
    //!
    virtual void ResizeOnResChange();

    //!
    //! \brief   Check Resolution Change and CSC
    //!
    //! \details On resolution change, resize internal buffer
    //!          Check raw surface to set flag for CSC operation
    //!
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckResChangeAndCsc();

    //!
    //! \brief  Destroy encode state
    //!
    //! \return void
    //!
    void Destroy() override;

    //!
    //! \brief  Allocate Scaling Resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateScalingResources();

    //!
    //! \brief  Execute Me Kernel
    //!
    //! \param  [in] meParams
    //!         meParams used to execute kernel
    //! \param  [in] meSurfaceParams
    //!         meSurfaceParams used to execute kernel
    //! \param  [in] hmeLevel
    //!         hme Level used to execute kernel
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteMeKernel(
        MeCurbeParams *meParams,
        MeSurfaceParams *meSurfaceParams,
        HmeLevel hmeLevel);

    //!
    //! \brief  Initialize Status Report
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitStatusReport();

    //!
    //! \brief  Update Encode Status
    //!
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] forceOperation
    //!         forceOperation flag used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateEncodeStatus(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                forceOperation);

    //!
    //! \brief  Reset Encode Status
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResetStatusReport();

    //!
    //! \brief  Read Brc Pak Statistics
    //!
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] params
    //!         parameters used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadBrcPakStatistics(
        PMOS_COMMAND_BUFFER cmdBuffer,
        EncodeReadBrcPakStatsParams* params);

    //!
    //! \brief  Update command buffer attribute
    //!
    //! \param  [in] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \param  [in] renderEngineInUse
    //!         renderEngineInUse flag used
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse);

    //!
    //! \brief    Function pointer of Get Kernel Header And Size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*pfnGetKernelHeaderAndSize) (
        void                            *binary,
        EncOperation                    operation,
        uint32_t                        krnStateIdx,
        void                            *krnHeader,
        uint32_t                        *krnSize);

    //!
    //! \brief    Function to allocate MDF required resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateMDFResources();

    //!
    //! \brief    Function to destroy MDF required resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DestroyMDFResources();

    //!
    //! \brief    Function to set MFE Shared State
    //!
    //! \details  Pointer on passed object will be saved in the local field,
    //!           content of source object might be changed later
    //!           (for example, CmDevice might be set or chagned)
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetMfeSharedState(MfeSharedState *pMfeSharedState);

    //!
    //! \brief  Function to add MDF kernel
    //!
    //! \param  [in] device
    //!         pointer to CmDevice
    //! \param  [in] queue
    //!         pointer to CmQueue
    //! \param  [in] kernel
    //!         pointer to CmKernel
    //! \param  [in] task
    //!         pointer to CmTask
    //! \param  [in] threadspace
    //!         pointer to CmThreadSpace
    //! \param  [in] event
    //!         reference to CmEvent*
    //! \param  [in] isEnqueue
    //!         bool
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddKernelMdf(
        CmDevice      *device,
        CmQueue       *queue,
        CmKernel      *kernel,
        CmTask        *task,
        CmThreadSpace *threadspace,
        CmEvent       *&event,
        bool           isEnqueue);

    //!
    //! \brief  Function to create Mdf Kernel resource
    //!
    //! \param  [out] resource
    //!         pointer to CodechalEncodeMdfKernelResource
    //! \param  [in] kernelNum
    //!         uint8_t, kernel number
    //! \param  [in] bufNum
    //!         uint8_t, buffer number
    //! \param  [in] surfNum
    //!         uint8_t, surface number
    //! \param  [in] vmeSurfNum
    //!         uint8_t, vme surface number
    //! \param  [in] curbeSize
    //!         uint16_t, curbe structure size
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateMDFKernelResource(
        CodechalEncodeMdfKernelResource *resource,
        uint8_t                          kernelNum,
        uint8_t                          bufNum,
        uint8_t                          surfNum,
        uint8_t                          vmeSurfNum,
        uint16_t                         curbeSize);

    //!
    //! \brief    Function to destroy Mdf kernel resource
    //!
    //! \param    [in] resource
    //!           pointer to CodechalEncodeMdfKernelResource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestroyMDFKernelResource(
        CodechalEncodeMdfKernelResource *resource);

    //!
    //! \brief    Function to free Mdf kernel surfaces
    //!
    //! \param    [in] resource
    //!           pointer to CodechalEncodeMdfKernelResource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FreeMDFKernelSurfaces(
        CodechalEncodeMdfKernelResource *resource);

    //!
    //! \brief  Returns number of PAK Passes based on BRC Precision flag
    //!
    //! \param  [in] usBRCPrecision
    //!         refer to CodechalEncodeBrcNumPasses
    //!
    //! \return uint8_t
    //!         number of pak passes
    //!
    uint8_t GetNumBrcPakPasses(uint16_t usBRCPrecision);

    //!
    //! \brief  Setup Walker Context
    //! \param  [in, out] cmdBuffer
    //!         Input and output cmdbuffer
    //! \param  [in] Pointer to kernel state
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupWalkerContext(
        MOS_COMMAND_BUFFER* cmdBuffer,
        SendKernelCmdsParams* params);

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpMbEncPakOutput(PCODEC_REF_LIST currRefList, CodechalDebugInterface* debugInterface);
    virtual MOS_STATUS DumpFrameStatsBuffer(CodechalDebugInterface* debugInterface) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief  Add/Subtract a value to specified gfx memory
    //!
    //! \param  [in] cmdBuffer
    //!         command buffer
    //! \param  [in] presStoreBuffer
    //!         buffer to modify
    //! \param  [in] offset
    //!         member offset in the buffer
    //! \param  [in] value
    //!         value to add/subtract
    //! \param  [in] bAdd
    //!         add or subtract
    //!
    //! \return MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddBufferWithIMMValue(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        PMOS_RESOURCE               presStoreBuffer,
        uint32_t                    offset,
        uint32_t                    value,
        bool                        bAdd);

    bool          m_enableFakeHrdSize   = false;
    int32_t       m_fakeIFrameHrdSize   = 0;
    int32_t       m_fakePBFrameHrdSize  = 0;
#endif
};

static void PutBit(BSBuffer *bsbuffer, uint32_t code)
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

static void PutBitsSub(BSBuffer *bsbuffer, uint32_t code, uint32_t length)
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

static void PutBits(BSBuffer *bsbuffer, uint32_t code, uint32_t length)
{
    uint32_t code1, code2;

    // temp solution, only support up to 32 bits based on current usage
    CODECHAL_ENCODE_ASSERT(length <= 32);

    if (length >= 24)
    {
        code1 = code & 0xFFFF;
        code2 = code >> 16;

        // high bits go first
        PutBitsSub(bsbuffer, code2, length - 16);
        PutBitsSub(bsbuffer, code1, 16);
    }
    else
    {
        PutBitsSub(bsbuffer, code, length);
    }
}

template<typename ValueType>
static ValueType SwapEndianness(ValueType value)
{
    uint8_t*    startLocation = reinterpret_cast<uint8_t*>(&value);
    uint8_t*    endLocation = startLocation + sizeof(ValueType);
    std::reverse(startLocation, endLocation);
    return value;
}
#endif  // __CODECHAL_ENCODER_BASE_H__
