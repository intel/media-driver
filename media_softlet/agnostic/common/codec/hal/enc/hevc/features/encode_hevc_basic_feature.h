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
//! \file     encode_hevc_basic_feature.h
//! \brief    Defines the common interface for encode hevc basic feature
//!
#ifndef __ENCODE_HEVC_BASIC_FEATURE_H__
#define __ENCODE_HEVC_BASIC_FEATURE_H__

#include <deque>
#include "encode_basic_feature.h"
#include "codec_def_encode_hevc.h"
#include "encode_hevc_reference_frames.h"
#include "media_hevc_feature_defs.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "encode_mem_compression.h"
#include "encode_hevc_basic_feature_422.h"
namespace encode
{
#define CODECHAL_HEVC_VDENC_LCU_SIZE           64
#define CODECHAL_HEVC_MIN_LCU_SIZE             16
#define CODECHAL_HEVC_MIN_CU_SIZE              8
#define CODECHAL_HEVC_MIN_TILE_SIZE            128
#define CODECHAL_ENCODE_HEVC_MIN_ICQ_QUALITYFACTOR      1
#define CODECHAL_ENCODE_HEVC_MAX_ICQ_QUALITYFACTOR      51

class HevcBasicFeature : public EncodeBasicFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    HevcBasicFeature(EncodeAllocator *allocator,
                     CodechalHwInterfaceNext *hwInterface,
                     TrackedBuffer *trackedBuf,
                     RecycleResource *recycleBuf,
                     void *constSettings = nullptr) :
                     EncodeBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf){ m_constSettings = constSettings;};

    virtual ~HevcBasicFeature();

    virtual MOS_STATUS Init(void *setting) override;

    virtual MOS_STATUS Update(void *params) override;

    virtual uint32_t GetProfileLevelMaxFrameSize() override;

    MOS_STATUS GetSurfaceMmcInfo(PMOS_SURFACE surface, MOS_MEMCOMP_STATE &mmcState, uint32_t &compressionFormat) const;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_SRC_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD1);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(HEVC_VP9_RDOQ_STATE);

    MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

    EncodeMemComp *m_mmcState = nullptr;

    static constexpr uint32_t                   m_maxSliceQP   = 52;          //!< Max QP
    static constexpr uint32_t                   m_maxLCUSize   = 64;          //!< Max LCU size 64
    static constexpr uint32_t                   m_qpNum        = 52;          //!< Number of QP values
    static constexpr uint32_t                   m_maxSyncDepth = 10;

    // Parameters passed from application
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS  m_hevcSeqParams = nullptr;          //!< Pointer to sequence parameter
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS   m_hevcPicParams = nullptr;          //!< Pointer to picture parameter
    PCODEC_HEVC_ENCODE_SLICE_PARAMS     m_hevcSliceParams = nullptr;        //!< Pointer to slice parameter
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS     m_hevcIqMatrixParams = nullptr;     //!< Pointer to IQ matrix parameter
    HevcReferenceFrames                 m_ref = {};                      //! Reference List

    uint32_t            m_lambdaType = 1;
    std::vector<double> m_qpFactors;

    uint32_t m_numTiles     = 1;
    uint32_t m_curNumSlices = 0;

    bool m_lastSliceInTile = false;

    uint32_t m_maxTileNumber          = 0;
    uint32_t m_picWidthInMinLCU       = 0;
    uint32_t m_picHeightInMinLCU      = 0;
    uint32_t m_widthAlignedMaxLCU     = 0;
    uint32_t m_heightAlignedMaxLCU    = 0;
    uint32_t m_sizeOfMvTemporalBuffer = 0;
    bool     m_hevcRDOQPerfDisabled   = false;
    PMOS_RESOURCE m_resMvTemporalBuffer = nullptr;                                  //!< Pointer to MOS_RESOURCE of MvTemporal Buffer

    uint32_t m_sizeOfSseSrcPixelRowStoreBufferPerLcu = 0;  //!< Size of SSE row store buffer per LCU
                                                                                    // VDENC Display interface related
    uint8_t m_slotForRecNotFiltered = 0;
    bool m_enableLBCOnly = false;
    bool m_enablePartialFrameUpdate = false;
    bool m_pakOnlyPass = false;
    bool m_captureModeEnable = false;  //!< Enable Capture mode with display
    bool m_hevcVdencRoundingPrecisionEnabled  = true;  //!<  Roinding Precision enabled
    uint8_t m_roundingIntra = 0;
    uint8_t m_roundingInter = 0;
    bool m_useDefaultRoundingForHcpSliceState = false;  //!< use default rounding for HCP_SLICE_STATE
    bool m_hevcVdencWeightedPredEnabled = false;
    uint32_t m_prevStoreData = -1;  // Change to -1 since FrameIdx starts from 0; Legacy path initialized to be 0 since FrameIdx starts from 1;
    uint32_t m_vdencBatchBufferPerSliceVarSize[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = { 0 };    //!< VDEnc batch buffer slice size array
    uint32_t m_vdencBatchBufferPerSlicePart2Start[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = {0};  //!< VDEnc batch buffer slice size array
    uint32_t m_vdencBatchBufferPerSlicePart2Size[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = {0};  //!< VDEnc batch buffer slice size array

    uint32_t m_picStateCmdStartInBytes = 0;       //!< Offset of PIC_STATE cmd in batch buffer

    HevcBasicFeature422 *m_422State= nullptr;
    MOS_STATUS            Init422State();

    std::deque<uint32_t> m_recycleBufferIdxes;

protected:
    MOS_STATUS SetPictureStructs();
    virtual MOS_STATUS UpdateTrackedBufferParameters() override;
    MOS_STATUS GetMaxMBPS(uint32_t levelIdc, uint32_t* maxMBPS, uint64_t* maxBytePerPic);
    virtual MOS_STATUS GetTrackedBuffers() override;
    MOS_STATUS SetSliceStructs();
    MOS_STATUS SetRoundingValues();
    //!
    //! \brief    Calculate maximum bitsize allowed for LCU
    //! \details  Calculate LCU max coding size according to log2_max_coding_block_size_minus3
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalcLCUMaxCodingSize();
    virtual MOS_STATUS GetRecycleBuffers();

    void CreateDefaultScalingList();
    void CreateFlatScalingList();

MEDIA_CLASS_DEFINE_END(encode__HevcBasicFeature)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_BASIC_FEATURE_H__
