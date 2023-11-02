/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_avc_basic_feature.h
//! \brief    Defines the common interface for encode avc basic feature
//!
#ifndef __ENCODE_AVC_BASIC_FEATURE_H__
#define __ENCODE_AVC_BASIC_FEATURE_H__

#include "encode_basic_feature.h"
#include "encode_avc_reference_frames.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_vdbox_huc_itf.h"
#include "encode_mem_compression.h"
#include "media_copy_wrapper.h"
#include "codechal_debug.h"

namespace encode
{
//!
//! \enum     AvcImgStructure
//! \brief    Average image structure
//!
enum AvcImgStructure
{
    avcFrame        = 0,
    avcTopField     = 1,
    avcBottomField  = 3
};

//!
//! \enum     CodecSelect
//! \brief    Codec select
//!
enum CodecSelect
{
    decoderCodec    = 0,
    encoderCodec    = 1
};

//!
//! \enum     AvcQmTypes
//! \brief    Average qm types
//!
enum AvcQmTypes
{
    avcQmIntra4x4 = 0,
    avcQmInter4x4 = 1,
    avcQmIntra8x8 = 2,
    avcQmInter8x8 = 3
};

//!
//! \struct   AvcRefListWrite
//! \brief    Average reference list write
//!
struct AvcRefListWrite
{
    union
    {
        struct
        {
            uint8_t bottomField     : 1;
            uint8_t frameStoreID    : 4;
            uint8_t fieldPicFlag    : 1;
            uint8_t longTermFlag    : 1;
            uint8_t nonExisting     : 1;
        };
        struct
        {
            uint8_t value;
        };
    } UC[32];
};

class AvcBasicFeature : public EncodeBasicFeature,
    public mhw::vdbox::vdenc::Itf::ParSetting,
    public mhw::vdbox::mfx::Itf::ParSetting,
    public mhw::vdbox::huc::Itf::ParSetting,
    public mhw::mi::Itf::ParSetting
{
public:
    AvcBasicFeature(EncodeAllocator * allocator,
                    CodechalHwInterfaceNext *hwInterface,
                    TrackedBuffer *trackedBuf,
                    RecycleResource *recycleBuf,
                    MediaCopyWrapper *mediaCopyWrapper,
                    void *constSettings = nullptr) :
                    EncodeBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf),
                    m_mediaCopyWrapper(mediaCopyWrapper) { m_constSettings = constSettings; }

    virtual ~AvcBasicFeature();

    virtual MOS_STATUS Init(void *setting) override;

    virtual MOS_STATUS Update(void *params) override;

    virtual uint32_t GetProfileLevelMaxFrameSize() override;

    bool IsAvcPSlice(uint8_t sliceType) const;
    bool IsAvcBSlice(uint8_t sliceType) const;

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_SRC_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD3);

    MHW_SETPAR_DECL_HDR(VDENC_AVC_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

    MHW_SETPAR_DECL_HDR(MFX_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(MFX_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(MFX_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(MFX_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_REF_IDX_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_DIRECTMODE_STATE);

    MHW_SETPAR_DECL_HDR(MI_FORCE_WAKEUP);

    MHW_SETPAR_DECL_HDR(MFX_WAIT);

    MHW_SETPAR_DECL_HDR(HUC_PIPE_MODE_SELECT);

    EncodeMemComp *m_mmcState = nullptr;

    // Parameters passed from application
    PCODEC_AVC_ENCODE_PIC_PARAMS                m_picParams[CODEC_AVC_MAX_PPS_NUM] = {};  //!< Pointer to array of picture parameter, could be removed
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS           m_seqParams[CODEC_AVC_MAX_SPS_NUM] = {};  //!< Pointer to array of sequence parameter, could be removed
    PCODEC_AVC_ENCODE_PIC_PARAMS                m_picParam           = nullptr;           //!< Pointer to AVC picture parameter
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS           m_seqParam           = nullptr;           //!< Pointer to AVC sequence parameter
    PCODECHAL_ENCODE_AVC_VUI_PARAMS             m_vuiParams          = nullptr;           //!< Pointer to AVC Uvi parameter
    PCODEC_AVC_ENCODE_SLICE_PARAMS              m_sliceParams        = nullptr;           //!< Pointer to AVC slice parameter
    PCODEC_AVC_IQ_MATRIX_PARAMS                 m_iqMatrixParams     = nullptr;           //!< Pointer to IQMaxtrix parameter
    PCODEC_AVC_ENCODE_IQ_WEIGTHSCALE_LISTS      m_iqWeightScaleLists = nullptr;           //!< Pointer to IQWidght ScaleLists
    CODEC_AVC_ENCODE_USER_FLAGS                 m_userFlags;                              //!< Encoder user flag settings

    std::shared_ptr<AvcReferenceFrames>  m_ref = nullptr;             //! Reference List

    uint32_t                          m_curNumSlices = 0;             //!< Number of current slice

#if USE_CODECHAL_DEBUG_TOOL
    MHW_VDBOX_AVC_SLICE_STATE sliceState{};
#endif  // USE_CODECHAL_DEBUG_TOOL

    CODECHAL_ENCODE_AVC_NAL_UNIT_TYPE m_nalUnitType         = CODECHAL_ENCODE_AVC_NAL_UT_RESERVED;      //!< Nal unit type
    
    uint16_t                          m_sliceHeight         = 0;      //!< Slice height
    bool                              m_deblockingEnabled   = false;  //!< Enable deblocking flag
    bool                              m_mbaffEnabled        = false;  //!< Enable MBAFF flag
    uint32_t                          m_sliceStructCaps     = 0;      //!< Slice struct
    uint8_t                           m_targetUsageOverride = 0;      //!< Target usage override

    bool                              m_useRawForRef = false;         //!< Flag to indicate if using raw surface for reference
    uint8_t                           m_prevReconFrameIdx = 0;        //!< Previous reconstruct frame index
    uint8_t                           m_currReconFrameIdx = 0;        //!< Current reconstruct frame index

    bool                              m_adaptiveRoundingInterEnable = false;  //!< Adaptive Rounding Inter Enable Flag.

    // VDEnc params
    bool                            m_vdencNoTailInsertion = false;  //!< Vdenc no tail insertion enabled flag

    bool                            m_perMBStreamOutEnable = false;

    // ENC input/output buffers
    bool                            m_madEnabled = false;            //!< Mad enabled flag

    // Maximum number of slices allowed by video spec
    uint32_t                        m_maxNumSlicesAllowed = 0;       //!< Max number of slices allowed

    // CMD buffer sizes
    uint32_t                        m_extraPictureStatesSize = 0;    //!< Picture states size extra

    // Skip frame params
    uint8_t                         m_skipFrameFlag = 0;             //!< Skip frame flag

    bool                            m_acceleratorHeaderPackingCaps = false;  //!< Flag set by driver from driver caps.
    bool                            m_suppressReconPicSupported    = false;  //!< Suppress reconstructed picture supported flag

    // Lookahead
    uint8_t  m_lookaheadDepth      = 0;  //!< Number of frames to lookahead
    uint32_t m_averageFrameSize    = 0;  //!< Average frame size based on targed bitrate and frame rate, in unit of bits
    uint32_t m_prevTargetFrameSize = 0;  //!< Target frame size of previous frame.
    uint32_t m_targetBufferFulness = 0;  //!< Target encode buffer fulness in bits, used by BRC and calculated from initial buffer fulness, target frame size (from DDI) and average frame size

    // Below values will be set if qp control params are sent by app
    bool                            m_minMaxQpControlEnabled = false;  //!< Flag to indicate if min/max QP feature is enabled or not.
    uint8_t                         m_iMinQp                 = 0;      //!< I frame Minimum QP.
    uint8_t                         m_iMaxQp                 = 0;      //!< I frame Maximum QP.
    uint8_t                         m_pMinQp                 = 0;      //!< P frame Minimum QP.
    uint8_t                         m_pMaxQp                 = 0;      //!< P frame Maximum QP.
    uint8_t                         m_bMinQp                 = 0;      //!< B frame Minimum QP.
    uint8_t                         m_bMaxQp                 = 0;      //!< B frame Maximum QP.
    bool                            m_pFrameMinMaxQpControl  = false;  //!< Indicates min/max QP values for P-frames are set separately or not.
    bool                            m_bFrameMinMaxQpControl  = false;  //!< Indicates min/max QP values for B-frames are set separately or not.

    uint32_t                        m_colocatedMVBufferSize       = 0;
    PMOS_RESOURCE                   m_colocatedMVBufferForIFrames = nullptr;

    // TCBRC related flags
    bool                            m_forcedTCBRC                     = false;  //!< TCBRC forced instead of LowDelayBRC
    bool                            m_brcAdaptiveRegionBoostSupported = false;  //!< Adaptive Region Boost supported flag
    bool                            m_brcAdaptiveRegionBoostEnabled   = false;  //!< Adaptive Region Boost enabled flag

protected:
    MOS_STATUS SetSequenceStructs();
    MOS_STATUS SetPictureStructs();
    MOS_STATUS UpdateSeiParameters(EncoderParams* params);
    void UpdateMinMaxQp();
    int32_t GetMaxMBPS(uint8_t levelIdc);
    MOS_STATUS SetSliceStructs();
    void CheckResolutionChange();
    MOS_STATUS PackPictureHeader();
    virtual bool InputSurfaceNeedsExtraCopy(const MOS_SURFACE &input);

    virtual MOS_STATUS UpdateTrackedBufferParameters() override;
    virtual MOS_STATUS GetTrackedBuffers() override;

    //!
    //! \brief    Initialize reference frames class
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitRefFrames();

    //!
    //! \brief    Calculate scaling list
    //!
    //! \return   void
    //!
    void ScalingListFlat();

    //!
    //! \brief    Calculate scaling list
    //!
    //! \return   void
    //!
    void ScalingListFallbackRuleA();

    //! \brief    Get the max number of allowed slice
    //! \param    [in] profileIdc
    //!           AVC profile idc
    //! \param    [in] levelIdc
    //!           AVC level idc
    //! \param    [in] framesPer100Sec
    //!           frame Per 100Sec
    //! \return   uint16_t
    //!           return uiMaxAllowedNumSlices
    //!
    uint16_t GetMaxNumSlicesAllowed(
        CODEC_AVC_PROFILE_IDC profileIdc,
        CODEC_AVC_LEVEL_IDC   levelIdc,
        uint32_t              framesPer100Sec);

    // SEI
    CodechalEncodeSeiData m_seiData        = {};       //!< Encode SEI data parameter.
    uint32_t              m_seiDataOffset  = false;    //!< Encode SEI data offset.
    uint8_t *             m_seiParamBuffer = nullptr;  //!< Encode SEI data buffer.

    MediaCopyWrapper *m_mediaCopyWrapper = nullptr;

MEDIA_CLASS_DEFINE_END(encode__AvcBasicFeature)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_BASIC_FEATURE_H__
