/*
* Copyright (c) 2018-2021, Intel Corporation
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

#define CODECHAL_VDENC_BRC_NUM_OF_PASSES        2

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
    bool          vdencBrcEnabled;
    bool          waReadVDEncOverflowStatus;
    uint32_t      mode ;

    uint32_t      vdencBrcNumOfSliceOffset;
    PMOS_RESOURCE *resVdencBrcUpdateDmemBufferPtr;

    PMOS_RESOURCE resBitstreamByteCountPerFrame;
    uint32_t      bitstreamByteCountPerFrameOffset;

    PMOS_RESOURCE resBitstreamSyntaxElementOnlyBitCount;
    uint32_t      bitstreamSyntaxElementOnlyBitCountOffset;

    PMOS_RESOURCE resQpStatusCount;
    uint32_t      qpStatusCountOffset;

    PMOS_RESOURCE resNumSlices;
    uint32_t      numSlicesOffset;

    PMOS_RESOURCE resImageStatusMask;
    uint32_t      imageStatusMaskOffset;

    PMOS_RESOURCE resImageStatusCtrl;
    uint32_t      imageStatusCtrlOffset;
};

#endif // !__CODEC_DEF_ENCODE_H__
