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
    CODECHAL_FUNCTION               ExecCodecFunction;          //!< High level codec functionality requested.

    PMOS_SURFACE                    psRawSurface;               //!< Raw surface
    PMOS_SURFACE                    psReconSurface;             //!< reconstructed surface
    PMOS_RESOURCE                   presBitstreamBuffer;        //!< Output buffer for bitstream data.
    PMOS_RESOURCE                   presMetadataBuffer;         //!< Output buffer for meta data.
    PMOS_RESOURCE                   presMbCodeSurface;          //!< PAK objects provided by framework.
    PMOS_SURFACE                    psMbSegmentMapSurface;      //!< [VP9]
    /* \brief [AVC & MPEG2] MB QP data provided by framework.
    *
    *    When in CQP mode, the framework can provide this surface that contains a single QpY value for each macroblock to be used for encoding. If it is not provided, the frame level QpY(QpY + slice_qp_delta) will be used for all macroblocks.
    */
    PMOS_SURFACE                    psMbQpDataSurface;          //!< pointer to surface of Mb QP Data
    PCODEC_ENCODE_MB_CONTROL        pMbCtrlBuffer;              //!< [AVC] MB contrl map provided by framework
    PMOS_SURFACE                    psMbDisableSkipMapSurface;  //!< [AVC] MB disable skip map provided by framework
    PMOS_SURFACE                    psCoeffSurface;             //!< [VP9]
    PMOS_RESOURCE                   presCoeffProbabilityBuffer; //!< [VP9] Coefficient probabilities provided by framework.
    bool                            bNewSeq;                    //!< Indicates the start of a new sequence.
    bool                            bPicQuant;                  //!< Indicates whether the scaling list is for SPS (0) or PPS (1).
    bool                            bNewQmatrixData;            //!< Indicates that new QM data was provided by framework.
    CodechalEncodeSeiData           *pSeiData;                   //!< [AVC & HEVC] Information pertaining to pSeiParamBuffer.
    uint32_t                        dwSEIDataOffset;            //!< [AVC & HEVC] Offset to the first SEI message within pSeiParamBuffer.
    uint8_t                         *pSeiParamBuffer;           //!< [AVC & HEVC] Packed SEI messages provided by the framework
    uint32_t                        dwNumSlices;                //!< Number of slice data structures in pSliceParams.
    uint32_t                        dwAppDataSize;              //!< [JPEG]
    uint32_t                        dwNumHuffBuffers;           //!< [JPEG]
    uint32_t                        dwMbDataBufferSize;         //!< Size of the data contained in presMbCodeSurface
    uint32_t                        dwBitstreamSize;            //!< Maximum amount of data to be output to presBitstreamBuffer.
    bool                            bNewVuiData;                //!< [AVC & MPEG2] Indicates that pVuiParams is present and expected to be valid.
    bool                            bJpegQuantMatrixSent;       //!< [JPEG] Indicates whether a quant matrix was sent by the framework.
    PBSBuffer                       pBSBuffer;                  //!< Packed header data provided by the framework to be inserted in the bitstream.
    PCODECHAL_NAL_UNIT_PARAMS       *ppNALUnitParams = nullptr; //!< Information about the packed header data in pBSBuffer.
    uint32_t                        uiNumNalUnits;              //!< Number of NAL units in ppNALUnitParams.
    void                            *pSlcHeaderData;            //!< [AVC, HEVC, & MPEG2] Packed slice header data provided by the framework.
    bool                            bAcceleratorHeaderPackingCaps;  //!< [AVC] Indicates whether or not the driver is packing the slice headers.
    uint32_t                        uiSlcStructCaps;            //!< [AVC] Slice capability information, formatted as CODEC_SLICE_STRUCTS
    bool                            bMADEnabled;                //!< MAD is enabled
    bool                            bMbQpDataEnabled;           //!< [AVC & MPEG2] Indicates that psMbQpDataSurface is present.
    bool                            bMbDisableSkipMapEnabled;   //!< [AVC] Indicates that psMbDisableSkipMapSurface is present.
    bool                            bReportStatisticsEnabled;   //!< [HEVC] Indicates whether statistic reporting is enabled, disabled by default.
    bool                            bQualityImprovementEnable;  //!< [HEVC] Indicates whether quality improvement is enabled, disabled by default.
    bool                            newSeqHeader;               //!< [AVC] Flag for new Sequence Header.
    bool                            newPpsHeader;               //!< [AVC] Flag for new PPS Header.
    bool                            arbitraryNumMbsInSlice;     //!< [AVC] Flag to indicate if the sliceMapSurface needs to be programmed or not.

    void                            *pSeqParams;                 //!< Sequence parameter set structure defined per standard.
    void                            *pPicParams;                 //!< Picture parameter set structure defined per standard.
    void                            *pVuiParams;                 //!< [AVC & MPEG2] Picture parameter set structure defined per standard.
    void                            *pSliceParams;               //!< Slice data array defined per standard, expect dwNumSlices entries.
    void                            *pSegmentParams;             //!< [VP9]
    void                            *pIQMatrixBuffer;            //!< [AVC, HEVC, & MPEG2] IQ parameter structure defined per standard.

    // AVC Specific Parameters
    void                            *pIQWeightScaleLists;        //!< list of IQ Weight scale
    void                            *pAVCQCParams;               //!< AVC QC parameters
    void                            *pAVCRoundingParams;         //!< AVC rounding parameters

    void                            *pQuantData;
    PMOS_RESOURCE                   presDistortionDataSurface;
    uint32_t                        uiFrameRate;

    bool                            bSegmentMapProvided;        //!< [VP9]

    void                            *pMpeg2UserDataListHead;     //!< [MPEG2]

    void                            *pHuffmanTable;              //!< [JPEG]
    void                            *pQuantizationTable;         //!< [JPEG]
    void                            *pApplicationData;           //!< [JPEG]

    void                            *pFeiPicParams;              //!< [FEI]
    void                            *pPreEncParams;              //!< [FEI]

    // HEVC Specific Parameters
    bool                            bVdencActive;               //!< Indicate if vdenc is active
    bool                            advanced;                       //!< Indicate if vdenc is active

    MOS_SURFACE                     rawSurface;                     //!< Raw surface
    MOS_SURFACE                     reconSurface;                   //!< reconstructed surface
    MOS_RESOURCE                    resBitstreamBuffer;             //!< Output buffer for bitstream data.
    MOS_SURFACE                     mbQpSurface;
    MOS_SURFACE                     disableSkipMapSurface;          //!< [AVC] MB disable skip map provided by framework
    HANDLE                          gpuAppTaskEvent;                // MSDK event handling
    //Call back to application. This informs the application  all ENC kernel workload is submitted(in case of HEVC VME)
    //such that Application can make use of render engine when encoder is working on PAK. this helps in efficient utilisation of
    //Render engine for improving the performance as the render engine will be idle when encoder is working on PAK.
    void *                          plastEncKernelSubmissionCompleteCallback;


    bool                            bStreamOutEnable;
    PMOS_RESOURCE                   pStreamOutBuffer; // StreamOut buffer
    bool                            bCoeffRoundTag;
    uint32_t                        uiRoundIntra;
    uint32_t                        uiRoundInter;

    PMOS_RESOURCE                   presMbInlineData;
    PMOS_RESOURCE                   presMbConstSurface;
    PMOS_RESOURCE                   presVMEOutSurface;
    uint32_t                        uiMVoffset; // App provides PAK objects and MV data in the same surface. This is offset to MV Data.
    bool                            fullHeaderInAppData;         //!< [JPEG]
    uint32_t                        uiOverallNALPayload;
    MetaDataOffset                  metaDataOffset;
    void *                          pSliceHeaderParams;         //!< [HEVC]

    PMOS_RESOURCE                   m_presPredication = nullptr;            //! \brief [Predication] Resource for predication
    uint64_t                        m_predicationResOffset = 0;             //! \brief [Predication] Offset for Predication resource
    bool                            m_predicationNotEqualZero = false;      //! \brief [Predication] Predication mode
    bool                            m_predicationEnabled = false;           //! \brief [Predication] Indicates whether or not Predication is enabled
    PMOS_RESOURCE                   *m_tempPredicationBuffer = nullptr;     //! \brief [Predication] Temp buffer for Predication

    bool                            m_setMarkerEnabled = false;             //! \brief [SetMarker] Indicates whether or not SetMarker is enabled
    PMOS_RESOURCE                   m_presSetMarker = nullptr;              //! \brief [SetMarker] Resource for SetMarker
};

#endif // !__CODEC_DEF_ENCODE_H__
