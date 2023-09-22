/*
* Copyright (c) 2018-2023, Intel Corporation
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
//! \file     codec_def_encode.h
//! \brief    Defines encode types and macros shared by CodecHal and DDI layer
//! \details  Applies to encode only. Should not contain any DDI specific code.
//!

#ifndef __CODEC_DEF_ENCODE_H__
#define __CODEC_DEF_ENCODE_H__
#include "mos_os.h"
#include "codec_def_common.h"

#define CODECHAL_VDENC_BRC_NUM_OF_PASSES        2
#define CODECHAL_PAK_OBJ_EACH_CU                66
#define CODECHAL_LPLA_NUM_OF_PASSES             2
#define CODECHAL_ENCODE_BRC_KBPS                1000  // 1000bps for disk storage, aligned with industry usage

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
//! \struct CodechalEncodeSeiData
//! \brief  Indicate the SeiData parameters
//!
struct CodechalEncodeSeiData
{
    bool       newSEIData;
    uint32_t   dwSEIDataSize;
    uint32_t   dwSEIBufSize;
    uint8_t*   pSEIBuffer;
};

typedef struct _CODEC_ENCODE_MB_CONTROL
{
    union
    {
        struct
        {
            uint32_t        bForceIntra : 1;
            uint32_t        Reserved : 31;
        };
        uint32_t            value;
    } MBParams;
} CODEC_ENCODE_MB_CONTROL, *PCODEC_ENCODE_MB_CONTROL;

struct MetaDataOffset
{
    uint32_t dwEncodeErrorFlags                         = 0;
    uint32_t dwEncodedBitstreamWrittenBytesCount        = 0;
    uint32_t dwWrittenSubregionsCount                   = 0;

    uint32_t dwEncodeStats                       = 0;
    uint32_t dwAverageQP                         = 0;
    uint32_t dwIntraCodingUnitsCount             = 0;
    uint32_t dwInterCodingUnitsCount             = 0;
    uint32_t dwSkipCodingUnitsCount              = 0;
    uint32_t dwAverageMotionEstimationXDirection = 0;
    uint32_t dwAverageMotionEstimationYDirection = 0;

    uint32_t dwbSize        = 0;
    uint32_t dwbStartOffset = 0;
    uint32_t dwbHeaderSize  = 0;

    uint32_t dwMetaDataSize             = 0;
    uint32_t dwMetaDataSubRegionSize    = 0;

    uint32_t dwTilePartitionSize = 0;
    uint32_t dwPostFeatueSize    = 0;
};

//!
//! \struct EncoderParams
//! \brief  Encoder parameters
//!
struct EncoderParams
{
    CODECHAL_FUNCTION               ExecCodecFunction       = CODECHAL_FUNCTION_INVALID;    //!< High level codec functionality requested.

    PMOS_SURFACE                    psRawSurface            = nullptr;                      //!< Raw surface
    PMOS_SURFACE                    psReconSurface          = nullptr;                      //!< reconstructed surface
    PMOS_RESOURCE                   presBitstreamBuffer     = nullptr;                      //!< Output buffer for bitstream data.
    PMOS_RESOURCE                   presMetadataBuffer      = nullptr;                      //!< Output buffer for meta data.
    PMOS_RESOURCE                   presMbCodeSurface       = nullptr;                      //!< PAK objects provided by framework.
    PMOS_SURFACE                    psMbSegmentMapSurface   = nullptr;                      //!< [VP9]
    /* \brief [AVC & MPEG2] MB QP data provided by framework.
    *
    *    When in CQP mode, the framework can provide this surface that contains a single QpY value for each macroblock to be used for encoding. If it is not provided, the frame level QpY(QpY + slice_qp_delta) will be used for all macroblocks.
    */
    PMOS_SURFACE                    psMbQpDataSurface               = nullptr;                  //!< pointer to surface of Mb QP Data
    PCODEC_ENCODE_MB_CONTROL        pMbCtrlBuffer                   = nullptr;                  //!< [AVC] MB contrl map provided by framework
    PMOS_SURFACE                    psMbDisableSkipMapSurface       = nullptr;                  //!< [AVC] MB disable skip map provided by framework
    PMOS_SURFACE                    psCoeffSurface                  = nullptr;                  //!< [VP9]
    PMOS_RESOURCE                   presCoeffProbabilityBuffer      = nullptr;                  //!< [VP9] Coefficient probabilities provided by framework.
    bool                            bNewSeq                         = false;                    //!< Indicates the start of a new sequence.
    bool                            bPicQuant                       = false;                    //!< Indicates whether the scaling list is for SPS (0) or PPS (1).
    bool                            bNewQmatrixData                 = false;                    //!< Indicates that new QM data was provided by framework.
    CodechalEncodeSeiData           *pSeiData                       = nullptr;                  //!< [AVC & HEVC] Information pertaining to pSeiParamBuffer.
    uint32_t                        dwSEIDataOffset                 = 0;                        //!< [AVC & HEVC] Offset to the first SEI message within pSeiParamBuffer.
    uint8_t                         *pSeiParamBuffer                = nullptr;                  //!< [AVC & HEVC] Packed SEI messages provided by the framework
    uint32_t                        dwNumSlices                     = 0;                        //!< Number of slice data structures in pSliceParams.
    uint32_t                        dwAppDataSize                   = 0;                        //!< [JPEG]
    uint32_t                        dwNumHuffBuffers                = 0;                        //!< [JPEG]
    uint32_t                        dwMbDataBufferSize              = 0;                        //!< Size of the data contained in presMbCodeSurface
    uint32_t                        dwBitstreamSize                 = 0;                        //!< Maximum amount of data to be output to presBitstreamBuffer.
    bool                            bNewVuiData                     = false;                    //!< [AVC & MPEG2] Indicates that pVuiParams is present and expected to be valid.
    bool                            bJpegQuantMatrixSent            = false;                    //!< [JPEG] Indicates whether a quant matrix was sent by the framework.
    PBSBuffer                       pBSBuffer                       = nullptr;                  //!< Packed header data provided by the framework to be inserted in the bitstream.
    PCODECHAL_NAL_UNIT_PARAMS       *ppNALUnitParams                = nullptr;                  //!< Information about the packed header data in pBSBuffer.
    uint32_t                        uiNumNalUnits                   = 0;                        //!< Number of NAL units in ppNALUnitParams.
    void                            *pSlcHeaderData                 = nullptr;                  //!< [AVC, HEVC, & MPEG2] Packed slice header data provided by the framework.
    bool                            bAcceleratorHeaderPackingCaps   = false;                    //!< [AVC] Indicates whether or not the driver is packing the slice headers.
    uint32_t                        uiSlcStructCaps                 = 0;                        //!< [AVC] Slice capability information, formatted as CODEC_SLICE_STRUCTS
    bool                            bMADEnabled                     = false;                    //!< MAD is enabled
    bool                            bMbQpDataEnabled                = false;                    //!< [AVC & MPEG2] Indicates that psMbQpDataSurface is present.
    bool                            bMbDisableSkipMapEnabled        = false;                    //!< [AVC] Indicates that psMbDisableSkipMapSurface is present.
    bool                            bReportStatisticsEnabled        = false;                    //!< [HEVC] Indicates whether statistic reporting is enabled, disabled by default.
    bool                            bQualityImprovementEnable       = false;                    //!< [HEVC] Indicates whether quality improvement is enabled, disabled by default.
    bool                            newSeqHeader                    = false;                    //!< [AVC] Flag for new Sequence Header.
    bool                            newPpsHeader                    = false;                    //!< [AVC] Flag for new PPS Header.
    bool                            arbitraryNumMbsInSlice          = false;                    //!< [AVC] Flag to indicate if the sliceMapSurface needs to be programmed or not.

    void                            *pSeqParams                     = nullptr;                  //!< Sequence parameter set structure defined per standard.
    void                            *pPicParams                     = nullptr;                  //!< Picture parameter set structure defined per standard.
    void                            *pVuiParams                     = nullptr;                  //!< [AVC & MPEG2] Picture parameter set structure defined per standard.
    void                            *pSliceParams                   = nullptr;                  //!< Slice data array defined per standard, expect dwNumSlices entries.
    void                            *pSegmentParams                 = nullptr;                  //!< [VP9]
    void                            *pIQMatrixBuffer                = nullptr;                  //!< [AVC, HEVC, & MPEG2] IQ parameter structure defined per standard.

    // AVC Specific Parameters
    void                            *pIQWeightScaleLists            = nullptr;                  //!< list of IQ Weight scale
    void                            *pAVCQCParams                   = nullptr;                  //!< AVC QC parameters
    void                            *pAVCRoundingParams             = nullptr;                  //!< AVC rounding parameters

    void                            *pQuantData                     = nullptr;
    PMOS_RESOURCE                   presDistortionDataSurface       = nullptr;
    uint32_t                        uiFrameRate                     = 0;

    bool                            bSegmentMapProvided             = false;                    //!< [VP9]

    void                            *pMpeg2UserDataListHead         = nullptr;                  //!< [MPEG2]

    void                            *pHuffmanTable                  = nullptr;                  //!< [JPEG]
    void                            *pQuantizationTable             = nullptr;                  //!< [JPEG]
    void                            *pApplicationData               = nullptr;                  //!< [JPEG]

    void                            *pFeiPicParams                  = nullptr;                  //!< [FEI]
    void                            *pPreEncParams                  = nullptr;                  //!< [FEI]

    // HEVC Specific Parameters
    bool                            bVdencActive                    = false;                    //!< Indicate if vdenc is active
    bool                            advanced                        = false;                    //!< Indicate if vdenc is active

    MOS_SURFACE                     rawSurface                      = {};                       //!< Raw surface
    MOS_SURFACE                     reconSurface                    = {};                       //!< reconstructed surface
    MOS_RESOURCE                    resBitstreamBuffer              = {};                       //!< Output buffer for bitstream data.
    MOS_SURFACE                     mbQpSurface                     = {};
    MOS_SURFACE                     disableSkipMapSurface           = {};                       //!< [AVC] MB disable skip map provided by framework
    HANDLE                          gpuAppTaskEvent                 = nullptr;                  // MSDK event handling
    //Call back to application. This informs the application  all ENC kernel workload is submitted(in case of HEVC VME)
    //such that Application can make use of render engine when encoder is working on PAK. this helps in efficient utilisation of
    //Render engine for improving the performance as the render engine will be idle when encoder is working on PAK.
    void *                          plastEncKernelSubmissionCompleteCallback = nullptr;


    bool                            bStreamOutEnable    = false;
    PMOS_RESOURCE                   pStreamOutBuffer    = nullptr;          // StreamOut buffer
    bool                            bCoeffRoundTag      = false;
    uint32_t                        uiRoundIntra        = 0;
    uint32_t                        uiRoundInter        = 0;

    PMOS_RESOURCE                   presMbInlineData    = nullptr;
    PMOS_RESOURCE                   presMbConstSurface  = nullptr;
    PMOS_RESOURCE                   presVMEOutSurface   = nullptr;
    uint32_t                        uiMVoffset          = 0;                // App provides PAK objects and MV data in the same surface. This is offset to MV Data.
    bool                            fullHeaderInAppData = false;            //!< [JPEG]
    uint32_t                        uiOverallNALPayload = 0;
    MetaDataOffset                  metaDataOffset      = {};
    void *                          pSliceHeaderParams  = nullptr;          //!< [HEVC]

    /*! \brief Specifies motion search modes that will be used.
    *
    *    SubPelMode is only valid when bEnableSubPelMode is true. Following are valid values of SubPelMode:
    *    0:Integer mode searching
    *    1:Half-pel mode searching
    *    2:Reserved
    *    3:Quarter-pel mode searching
    */
    bool                            bEnableSubPelMode = false;
    uint8_t                         SubPelMode        = 0;

    PMOS_RESOURCE                   m_presPredication           = nullptr;      //! \brief [Predication] Resource for predication
    uint64_t                        m_predicationResOffset      = 0;            //! \brief [Predication] Offset for Predication resource
    bool                            m_predicationNotEqualZero   = false;        //! \brief [Predication] Predication mode
    bool                            m_predicationEnabled        = false;        //! \brief [Predication] Indicates whether or not Predication is enabled
    PMOS_RESOURCE                   *m_tempPredicationBuffer    = nullptr;      //! \brief [Predication] Temp buffer for Predication

    bool                            m_setMarkerEnabled          = false;        //! \brief [SetMarker] Indicates whether or not SetMarker is enabled
    PMOS_RESOURCE                   m_presSetMarker             = nullptr;      //! \brief [SetMarker] Resource for SetMarker
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
//! \enum   MbBrcSetting
//! \brief  Indicate the MBBRC settings
//!
enum MbBrcSetting
{
    mbBrcInternal = 0,
    mbBrcEnabled  = 1,
    mbBrcDisabled = 2,
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
//! \struct    EncodeStatusReadParams
//! \brief     Read encode states parameters
//!
struct EncodeStatusReadParams
{
    bool          vdencBrcEnabled                          = false;
    bool          waReadVDEncOverflowStatus                = false;
    uint32_t      mode                                     = 0;

    uint32_t      vdencBrcNumOfSliceOffset                 = 0;
    PMOS_RESOURCE *resVdencBrcUpdateDmemBufferPtr          = nullptr;

    PMOS_RESOURCE resBitstreamByteCountPerFrame            = nullptr;
    uint32_t      bitstreamByteCountPerFrameOffset         = 0;

    PMOS_RESOURCE resBitstreamSyntaxElementOnlyBitCount    = nullptr;
    uint32_t      bitstreamSyntaxElementOnlyBitCountOffset = 0;

    PMOS_RESOURCE resQpStatusCount                         = nullptr;
    uint32_t      qpStatusCountOffset                      = 0;

    PMOS_RESOURCE resNumSlices                             = nullptr;
    uint32_t      numSlicesOffset                          = 0;

    PMOS_RESOURCE resImageStatusMask                       = nullptr;
    uint32_t      imageStatusMaskOffset                    = 0;

    PMOS_RESOURCE resImageStatusCtrl                       = nullptr;
    uint32_t      imageStatusCtrlOffset                    = 0;
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

#endif // !__CODEC_DEF_ENCODE_H__
