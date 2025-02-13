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
//! \file     encode_parameter.h
//! \brief    Defines the common interface for encode parameter
//!
#ifndef __ENCODE_BASIC_FEATURE_H__
#define __ENCODE_BASIC_FEATURE_H__

#include "codec_hw_next.h"
#include "codec_def_encode.h"
#include "codechal_setting.h"
#include "encode_tracked_buffer.h"
#include "encode_recycle_resource.h"
#include "media_feature.h"
#include "mhw_vdbox.h"

namespace encode
{
class EncodeBasicFeature:public MediaFeature
{
public:
    EncodeBasicFeature(EncodeAllocator *allocator,
                        CodechalHwInterfaceNext *hwInterface,
                        TrackedBuffer *trackedBuf,
                        RecycleResource *recycleBuf);
    virtual ~EncodeBasicFeature() { }

    //!
    //! \brief  Init encode parameter
    //! \param  [in] setting
    //!         Pointer to CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting);

    //!
    //! \brief  Update encode basic feature
    //! \param  [in] params
    //!         Pointer to EncoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params);

    //!
    //! \brief  Update encode basic format
    //! \param  [in] params
    //!         Pointer to EncoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateFormat(void *params);

    //!
    //! \brief  Reset encode basic feature.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Reset(CODEC_REF_LIST *refList);

    //!
    //! \brief  Get the waTable
    //! \return The active WA table
    //!
    MEDIA_WA_TABLE *GetWaTable() { return m_osInterface? m_osInterface->pfnGetWaTable(m_osInterface):nullptr; }

    virtual uint32_t GetProfileLevelMaxFrameSize() = 0;

    static constexpr uint32_t m_vdencBrcPassNum = 3;  //!< Vdenc Brc Number of passes
    static constexpr uint32_t m_uncompressedSurfaceNum = 127;  //!< Hevc/Av1 uncompressed surface number, 7 bits, 0x7f is invalid one
    static constexpr uint32_t m_sizeOfHcpPakFrameStats = 9 * CODECHAL_CACHELINE_SIZE;  //!> Size of HEVC PAK frame statistics
    static constexpr uint32_t m_hevcPakStatsSSEOffset = 32;
    static constexpr uint32_t m_codecHalHevcNumPakSliceBatchBuffers = 3;

    uint32_t                    m_frameWidth = 0;             //!< Frame width in luma samples
    uint32_t                    m_frameHeight = 0;            //!< Frame height in luma samples
    uint32_t                    m_frameFieldHeight = 0;       //!< Frame height in luma samples
    uint32_t                    m_oriFrameHeight = 0;         //!< Original frame height
    uint32_t                    m_oriFrameWidth = 0;          //!< Original frame width
    uint16_t                    m_frame_crop_bottom_offset = 0;                       //!< frame_crop_bottom_offset
    uint16_t                    m_frame_mbs_only_flag      = 0;                       //!< frame_mbs_only_flag
    uint16_t                    m_frame_cropping_flag    = 0;                         //!< frame_cropping_flag
    uint16_t                    m_picWidthInMb = 0;           //!< Picture Width in MB width count
    uint16_t                    m_picHeightInMb = 0;          //!< Picture Height in MB height count
    uint16_t                    m_frameFieldHeightInMb = 0;   //!< Frame/field Height in MB
    uint32_t                    m_downscaledWidthInMb4x = 0;                          //!< Downscale width in Mb 4x
    uint32_t                    m_downscaledHeightInMb4x = 0;                         //!< Downscale height in Mb 4x

    CODEC_PICTURE               m_currOriginalPic = {};       //!< Raw.
    CODEC_PICTURE               m_currReconstructedPic = {};  //!< RECON.
    uint16_t                    m_pictureCodingType = 0;      //!< I, P, or B frame
    uint32_t                    m_frameNum             = 0;   //!< Frame number
    bool                        m_firstField = true;          //!< Flag to indicate if it is first field
    bool                        m_resolutionChanged = false;  //!< Flag to indicate if resolution is changed

    uint32_t                    m_standard = 0;               //!< The encode state's standard
    uint32_t                    m_mode = 0;                   //!< The encode mode
    CODECHAL_FUNCTION           m_codecFunction = CODECHAL_FUNCTION_INVALID;           //!< The encode state's codec function used

    PCODECHAL_NAL_UNIT_PARAMS   *m_nalUnitParams = nullptr;   //!< The encode nal unit
    uint32_t                     m_NumNalUnits = 0;           //!< Number of NAL units in ppNALUnitParams.

    bool                        m_panicEnable = false;        //!< Indicate if panic is enabled

    bool                        m_newSeqHeader = false;       //!< New sequence header flag
    bool                        m_newPpsHeader = false;       //!< New PPS header flag
    bool                        m_newVuiData = false;         //!< New VUI data flag
    bool                        m_newSeq = false;             //!< New sequence flag
    bool                        m_lastPicInSeq = false;       //!< Flag to indicate if it is last picture in sequence
    bool                        m_lastPicInStream = false;    //!< Flag to indicate if it is last picture in stream

    bool                        m_picQuant = false;           //!< picture quant
    bool                        m_newQmatrixData = false;     //!< New Qmatrix data
    PCODEC_ENCODER_SLCDATA      m_slcData = nullptr;          //!< record slice header size & position
    uint32_t                    m_numSlices = 0;              //!< Number of slices

    uint32_t                    m_mbDataBufferSize = 0;

    TrackedBuffer               *m_trackedBuf         = nullptr;  //!< Encode tracked buffer
    RecycleResource             *m_recycleBuf         = nullptr;  //!< Recycle resources

    uint32_t                    m_mbCodeSize          = 0;        //!< MB code buffer size
    uint32_t                    m_mvDataSize          = 0;        //!< MV data size
    PMOS_RESOURCE               m_resMbCodeBuffer     = nullptr;  //!< Pointer to MOS_RESOURCE of MbCode buffer
    PMOS_RESOURCE               m_resMvDataBuffer     = nullptr;  //!< Pointer to MOS_RESOURCE of MvData buffer
    bool                        m_isMbCodeRegistered  = false;

    uint32_t                    m_downscaledWidth4x   = 0;
    uint32_t                    m_downscaledHeight4x  = 0;
    PMOS_SURFACE                m_4xDSSurface         = nullptr;
    PMOS_SURFACE                m_8xDSSurface         = nullptr;

    PMOS_SURFACE                m_rawSurfaceToPak = nullptr;
    PMOS_SURFACE                m_rawSurfaceToEnc = nullptr;       //!< raw surf to enc
    MOS_SURFACE                 m_rawSurface = {};                 //!< Pointer to MOS_SURFACE of raw surface
    MOS_SURFACE                 m_reconSurface = {};               //!< Pointer to MOS_SURFACE of reconstructed surface
    MOS_RESOURCE                m_resBitstreamBuffer = {};         //!< Pointer to MOS_SURFACE of bitstream surface
    PMOS_RESOURCE               m_resMetadataBuffer = nullptr;
    MetaDataOffset              m_metaDataOffset = {};

    BSBuffer                    m_bsBuffer = {};                   //!< Bit-stream buffer

    uint32_t                    m_bitstreamSize = 0;               //!< Maximum amount of data to be output to presBitstreamBuffer.
    bool                        m_mbQpDataEnabled = false;         //!< [AVC & MPEG2] Indicates that psMbQpDataSurface is present.
    bool                        m_mbDisableSkipMapEnabled = false; //!< [AVC] Indicates that psMbDisableSkipMapSurface is present.
    MOS_SURFACE                 m_mbDisableSkipMapSurface = {};    //!< [AVC] MB disable skip map provided by framework
    MOS_SURFACE                 m_mbQpDataSurface = {};            //!< pointer to surface of Mb QP Data
    bool                        m_rgbEncodingEnable = false;       //!< Enable RGB encoding
    bool                        m_captureModeEnable = false;       //!< Enable Capture mode with display
    bool                        m_predicationNotEqualZero = false;       //!< [Predication] Predication mode
    bool                        m_predicationEnabled      = false;       //!< [Predication] Indicates whether or not Predication is enabled
    bool                        m_setMarkerEnabled        = false;       //!< [SetMarker] Indicates whether or not SetMarker is enabled
    uint64_t                    m_predicationResOffset    = 0;           //!< [Predication] Offset for Predication resource
    PMOS_RESOURCE               m_presPredication         = nullptr;     //!< [Predication] Resource for predication
    PMOS_RESOURCE              *m_tempPredicationBuffer   = nullptr;     //!< [Predication] Temp buffer for Predication
    PMOS_RESOURCE               m_predicationBuffer       = nullptr;     //!< [Predication] Internal buffer for predication

    uint8_t                     m_targetUsage =0;

    uint8_t  m_chromaFormat       = HCP_CHROMA_FORMAT_YUV420;           //!< Chroma format(420, 422 etc)
    uint8_t  m_bitDepth           = 8;                                  //!< Bit depth
    bool     m_is10Bit            = false;
    uint8_t  m_outputChromaFormat = (uint8_t)HCP_CHROMA_FORMAT_YUV420;  //!< 1: 420 2: 422 3: 444

    uint32_t m_currPakSliceIdx = 0;                                     //!< Current pak slice index

    bool m_enableTileStitchByHW = false;                                //!< Enable HW to stitch commands in scalable mode

    bool m_lockableResource = false;                                    //!< disable / enable lockable surface

    uint8_t m_par65Inter = 0;
    uint8_t m_par65Intra = 0;

    /*! \brief Specifies motion search modes that will be used.
    *
    *    SubPelMode is only valid when bEnableSubPelMode is true. Following are valid values of SubPelMode:
    *    0:Integer mode searching
    *    1:Half-pel mode searching
    *    2:Reserved
    *    3:Quarter-pel mode searching
    */
    bool    m_bEnableSubPelMode = false;
    uint8_t m_SubPelMode        = 3;
    bool    m_dualEncEnable     = false;

protected:
    //!
    //! \brief  Update the parameters of tracked buffers
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateTrackedBufferParameters();

    //!
    //! \brief  Get the buffers from tracked buffer manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrackedBuffers() { return MOS_STATUS_SUCCESS; }

    EncodeAllocator *m_allocator = nullptr;
    MOS_INTERFACE   *m_osInterface = nullptr;

MEDIA_CLASS_DEFINE_END(encode__EncodeBasicFeature)
};

}  // namespace encode

#endif  // !__ENCODE_BASIC_FEATURE_H__
