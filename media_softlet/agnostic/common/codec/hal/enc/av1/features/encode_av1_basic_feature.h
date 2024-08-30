/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     encode_av1_basic_feature.h
//! \brief    Defines the common interface for encode av1 basic feature
//!
#ifndef __ENCODE_AV1_BASIC_FEATURE_H__
#define __ENCODE_AV1_BASIC_FEATURE_H__

#include "encode_basic_feature.h"
#include "codec_def_encode_av1.h"
#include "encode_av1_reference_frames.h"
#include "encode_av1_stream_in.h"
#include "encode_av1_vdenc_feature_defs.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_vdbox_huc_itf.h"
#include "encode_mem_compression.h"
#if _MEDIA_RESERVED
#include "codec_def_encode_av1_ext.h"
#endif

namespace encode
{

inline bool DeltaQIsZero(CODEC_AV1_ENCODE_PICTURE_PARAMS const &picPar)
{
    return (picPar.y_dc_delta_q == 0) &&
           (picPar.u_ac_delta_q == 0) && (picPar.u_dc_delta_q == 0) &&
           (picPar.v_ac_delta_q == 0) && (picPar.v_dc_delta_q == 0);
}

inline bool IsFrameLossless(CODEC_AV1_ENCODE_PICTURE_PARAMS const &picPar)
{
    return (picPar.base_qindex == 0) && DeltaQIsZero(picPar);
}

MOS_STATUS InitDefaultFrameContextBuffer(
    uint16_t                 *ctxBuffer,
    uint8_t                  index,
    Av1CdfTableSyntaxElement begin = partition8x8,
    Av1CdfTableSyntaxElement end   = syntaxElementMax);

MOS_STATUS SyntaxElementCdfTableInit(
    uint16_t                    *ctxBuffer,
    SyntaxElementCdfTableLayout SyntaxElement);

class Av1BasicFeature : public EncodeBasicFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting, public mhw::vdbox::huc::Itf::ParSetting, public mhw::mi::Itf::ParSetting
{
public:
    Av1BasicFeature(EncodeAllocator * allocator,
                     CodechalHwInterfaceNext *hwInterface,
                     TrackedBuffer *trackedBuf,
                     RecycleResource *recycleBuf,
                     void *constSettings) :
                     EncodeBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf){m_constSettings = constSettings;};

    virtual ~Av1BasicFeature() {};

    virtual MOS_STATUS Init(void *setting) override;

    virtual MOS_STATUS Update(void *params) override;

    MOS_STATUS UpdateTrackedBufferParameters() override;

    virtual MOS_STATUS UpdateFormat(void *params) override;

    virtual MOS_STATUS UpdateDefaultCdfTable();

    virtual MOS_STATUS GetTrackedBuffers() override;

    Av1StreamIn *GetStreamIn();

    MOS_STATUS GetSurfaceMmcInfo(PMOS_SURFACE surface, MOS_MEMCOMP_STATE& mmcState, uint32_t& compressionFormat) const;

    virtual uint32_t GetProfileLevelMaxFrameSize() override;

    uint32_t GetAppHdrSizeInBytes(bool excludeFrameHdr = false) const;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_SRC_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WEIGHTSOFFSETS_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD1);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_INLOOP_FILTER_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(AVP_TILE_CODING);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_INTER_PRED_STATE);

    MHW_SETPAR_DECL_HDR(AVP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(HUC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(MFX_WAIT);

    EncodeMemComp* m_mmcState = nullptr;

    // Parameters passed from application
    PCODEC_AV1_ENCODE_SEQUENCE_PARAMS   m_av1SeqParams          = nullptr;                      //!< Pointer to sequence parameter
    PCODEC_AV1_ENCODE_PICTURE_PARAMS    m_av1PicParams          = nullptr;                      //!< Pointer to picture parameter
    Av1ReferenceFrames                  m_ref                   = {};                           //!< Reference List
    Av1StreamIn                         m_streamIn              = {};                           //!< Stream In
    AV1MetaDataOffset                   m_AV1metaDataOffset     = {};                           //!< AV1 Metadata Offset

    uint32_t            m_sizeOfSseSrcPixelRowStoreBufferPerLcu = 0;                            //!< Size of SSE row store buffer per LCU
    static constexpr uint32_t  m_vdencBrcStatsBufferSize        = 1216;                         //!< Vdenc bitrate control buffer size
    static constexpr uint32_t  m_vdencBrcPakStatsBufferSize     = 512;

    int16_t                            m_miCols = 0;
    int16_t                            m_miRows = 0;
    int32_t                            m_picWidthInSb = 0;
    int32_t                            m_picHeightInSb = 0;
    bool                               m_isSb128x128 = false;
    static const uint32_t              m_cdfMaxNumBytes = 15104;                                //!< Max number of bytes for CDF tables buffer, which equals to 236*64 (236 Cache Lines)
    PMOS_RESOURCE                      m_defaultCdfBuffers  = nullptr;                          //!< 4 default frame contexts per base_qindex
    PMOS_RESOURCE                      m_defaultCdfBufferInUse = nullptr;                       //!< default cdf table used base on current base_qindex
    uint32_t                           m_defaultCdfBufferInUseOffset = 0;
    bool                               m_defaultFcInitialized = false;

    bool                               m_enableSWBackAnnotation = true;                        //!< indicate whether SW back annotation enabled or not
    bool                               m_enableSWStitching = false;                             //!< indicate whether SW bitstream stitching enabled or not
    bool                               m_enableNonDefaultMapping = false;                       //!< indicate whether Non-default mapping enabled or not

    RoundingMethod                     m_roundingMethod = fixedRounding;
    bool                               m_enableCDEF = false;
    bool                               m_postCdefReconSurfaceFlag = false;

    uint32_t                           m_vdencTileSliceStart[av1MaxTileNum] = { 0 };           //!< VDEnc TILE_SLICE buffer offset array for every tile

    enum FlushCmd
    {
        waitVdenc = 0,
        waitAvp
    };
    FlushCmd                           m_flushCmd = waitVdenc;

#define ASYNC_NUM 32

    uint32_t                           m_frameHdrOBUSizeByteOffset = 0;  //!< indicate current frame OBUFrame offset
    uint16_t                           m_tileGroupHeaderSize = 0;
    uint32_t                           m_encodedFrameNum = 0;                                   //!< Currently encoded frame number

    PMOS_RESOURCE                      m_resMvTemporalBuffer = nullptr;                         //!< Pointer to MOS_RESOURCE of MvTemporal Buffer

    PMOS_RESOURCE m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer     = nullptr;  //!< Handle of Bitstream Decode Line Rowstore buffer,
    PMOS_RESOURCE m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< Handle of Bitstream Decode Tile Line buffer
    PMOS_RESOURCE m_intraPredictionTileLineRowstoreReadWriteBuffer         = nullptr;  //!< Handle of Intra Prediction Tile Line Rowstore Read/Write Buffer
    PMOS_RESOURCE m_spatialMotionVectorLineReadWriteBuffer                 = nullptr;  //!< Handle of Spatial Motion Vector Line rowstore buffer, can be programmed to use Local Media Storage VMM instead of Memory
    PMOS_RESOURCE m_spatialMotionVectorCodingTileLineReadWriteBuffer       = nullptr;  //!< Handle of Spatial Motion Vector Tile Line buffer
    PMOS_RESOURCE m_loopRestorationMetaTileColumnReadWriteBuffer           = nullptr;  //!< DW80..81, Loop Restoration Meta Tile Column Read/Write Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileReadWriteLineYBuffer          = nullptr;  //!< DW83..84, Loop Restoration Filter Tile Read/Write Line Y Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileReadWriteLineUBuffer          = nullptr;  //!< DW86..87, Loop Restoration Filter Tile Read/Write Line U Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileReadWriteLineVBuffer          = nullptr;  //!< DW89..90, Loop Restoration Filter Tile Read/Write Line V Buffer Address
    PMOS_RESOURCE m_deblockerFilterLineReadWriteYBuffer                    = nullptr;  //!< DW92..93, Deblocker Filter Line Read/Write Y Buffer Address
    PMOS_RESOURCE m_deblockerFilterLineReadWriteUBuffer                    = nullptr;  //!< DW95..96, Deblocker Filter Line Read/Write U Buffer Address
    PMOS_RESOURCE m_deblockerFilterLineReadWriteVBuffer                    = nullptr;  //!< DW98..99, Deblocker Filter Line Read/Write V Buffer Address
    PMOS_RESOURCE m_deblockerFilterTileLineReadWriteYBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW101..102, Deblocker Filter Tile Line Read/Write Y Buffer Address
    PMOS_RESOURCE m_deblockerFilterTileLineReadWriteVBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW104..105, Deblocker Filter Tile Line Read/Write V Buffer Address
    PMOS_RESOURCE m_deblockerFilterTileLineReadWriteUBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW107..108, Deblocker Filter Tile Line Read/Write U Buffer Address
    PMOS_RESOURCE m_deblockerFilterTileColumnReadWriteYBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW110..111, Deblocker Filter Tile Column Read/Write Y Buffer Address
    PMOS_RESOURCE m_deblockerFilterTileColumnReadWriteUBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW113..114, Deblocker Filter Tile Column Read/Write U Buffer Address
    PMOS_RESOURCE m_deblockerFilterTileColumnReadWriteVBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW116..117, Deblocker Filter Tile Column Read/Write V Buffer Address
    PMOS_RESOURCE m_cdefFilterLineReadWriteBuffer[AV1_NUM_OF_DUAL_CTX]           = {};  //!< DW119..120, CDEF Filter Line Read/Write Y Buffer Address
    PMOS_RESOURCE m_cdefFilterTileLineReadWriteBuffer[AV1_NUM_OF_DUAL_CTX]       = {};       //!< DW128..129, CDEF Filter Tile Line Read/Write Y Buffer Address
    PMOS_RESOURCE m_cdefFilterTileColumnReadWriteBuffer[AV1_NUM_OF_DUAL_CTX]     = {};  //!< DW137..138, CDEF Filter Tile Column Read/Write Y Buffer Address
    PMOS_RESOURCE m_cdefFilterMetaTileLineReadWriteBuffer[AV1_NUM_OF_DUAL_CTX]   = {};       //!< DW140..141, CDEF Filter Meta Tile Line Read/Write Buffer Address
    PMOS_RESOURCE m_cdefFilterMetaTileColumnReadWriteBuffer[AV1_NUM_OF_DUAL_CTX] = {};  //!< DW143..144, CDEF Filter Meta Tile Column Read/Write Buffer Address
    PMOS_RESOURCE m_cdefFilterTopLeftCornerReadWriteBuffer[AV1_NUM_OF_DUAL_CTX]  = {};       //!< DW146..147, CDEF Filter Top-Left Corner Read/Write Buffer Address
    PMOS_RESOURCE m_superResTileColumnReadWriteYBuffer                     = nullptr;  //!< DW149..150, Super-Res Tile Column Read/Write Y Buffer Address
    PMOS_RESOURCE m_superResTileColumnReadWriteUBuffer                     = nullptr;  //!< DW152..153, Super-Res Tile Column Read/Write U Buffer Address
    PMOS_RESOURCE m_superResTileColumnReadWriteVBuffer                     = nullptr;  //!< DW155..156, Super-Res Tile Column Read/Write V Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileColumnReadWriteYBuffer        = nullptr;  //!< DW158..159, Loop Restoration Filter Tile Column Read/Write Y Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileColumnReadWriteUBuffer        = nullptr;  //!< DW161..162, Loop Restoration Filter Tile Column Read/Write U Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileColumnReadWriteVBuffer        = nullptr;  //!< DW164..165, Loop Restoration Filter Tile Column Read/Write V Buffer Address
    PMOS_RESOURCE m_loopRestorationFilterTileColumnAlignmentBuf            = nullptr;  //!< DW170..171, Loop Restoration filter tile column alignment read/write buffer
    PMOS_RESOURCE m_decodedFrameStatusErrorBuffer                          = nullptr;  //!< DW176..177, Decoded Frame Status/Error Buffer Base Address
    PMOS_RESOURCE m_decodedBlockDataStreamoutBuffer                        = nullptr;  //!< DW179..180, Decoded Block Data Streamout Buffer Address
    PMOS_RESOURCE m_tileStatisticsPakStreamoutBuffer                       = nullptr;  //!< Tile Statistics Pak Streamout Buffer
    PMOS_RESOURCE m_cuStreamoutBuffer                                      = nullptr;  //!< CU Streamout Buffer
    PMOS_RESOURCE m_sseLineReadWriteBuffer                                 = nullptr;  //!< SSE Line Read / Write Buffer
    PMOS_RESOURCE m_sseTileLineReadWriteBuffer                             = nullptr;  //!< SSE Tile Line Read/Write Buffer
    PMOS_RESOURCE m_resMfdIntraRowStoreScratchBuffer                       = nullptr;

protected:
    uint32_t m_appHdrSize                = 0;
    uint32_t m_appHdrSizeExcludeFrameHdr = 0;

MEDIA_CLASS_DEFINE_END(encode__Av1BasicFeature)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_BASIC_FEATURE_H__
