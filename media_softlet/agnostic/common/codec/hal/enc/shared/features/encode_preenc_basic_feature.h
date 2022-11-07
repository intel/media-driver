/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_preenc_basic_feature.h
//! \brief    Defines the common interface for encode preenc basic feature
//!
#ifndef __ENCODE_PREENC_BASIC_FEATURE_H__
#define __ENCODE_PREENC_BASIC_FEATURE_H__

#include "encode_basic_feature.h"
#include "codec_def_encode_hevc.h"
#include "encode_hevc_dfs.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "encode_mem_compression.h"
#include "encode_preenc_const_settings.h"
#include "codechal_debug.h"
#include "encode_preenc_defs.h"
#if _MEDIA_RESERVED
#include "encode_preenc_basic_feature_ext.h"
#endif // _MEDIA_RESERVED

namespace encode
{
class PreEncBasicFeature : public EncodeBasicFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
{
public:
    PreEncBasicFeature(MediaFeatureManager *featureManager,
        EncodeAllocator *                   allocator,
        CodechalHwInterfaceNext *               hwInterface,
        TrackedBuffer *                     trackedBuf,
        RecycleResource *                   recycleBuf,
        void *                              constSettings = nullptr) : EncodeBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf)
    {
        m_hcpItf = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(hwInterface->GetHcpInterfaceNext());
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpItf);
        m_preEncConstSettings = MOS_New(EncodePreEncConstSettings);
    };

    virtual ~PreEncBasicFeature();

    virtual uint32_t GetProfileLevelMaxFrameSize() override { return 0; };

    virtual MOS_STATUS Init(void *setting) override;

    virtual MOS_STATUS Update(void *params) override;

    PCODEC_REF_LIST *GetRefList() { return m_preEncConfig.RefList; };

    int8_t *GetRefIdxMapping() { return m_refIdxMapping; };

    uint16_t GetPictureCodingType() { return m_pictureCodingType; };

    bool IsLowDelay() const { return m_lowDelay; };

    virtual MOS_STATUS CalculatePreEncInfo(uint32_t width, uint32_t height, PreEncInfo &preEncInfo);

    virtual MOS_STATUS GetPreEncInfo(PreEncInfo &preEncInfo);

    MOS_STATUS EncodePreencBasicFuntion0(PMOS_RESOURCE& Buffer0, PMOS_RESOURCE& Buffer1);

    virtual MOS_STATUS IsEnabled(bool &enabled)
    {
        enabled = m_enabled;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetEncodeMode(uint32_t &encodeMode)
    {
        encodeMode = m_encodeMode;
        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS EncodePreencBasicFuntion1();

    FILE *pfile0 = nullptr;
    FILE *pfile1 = nullptr;
#endif

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_SRC_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD1);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(VDENC_WALKER_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_WEIGHTSOFFSETS_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(HEVC_VP9_RDOQ_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);

    EncodeMemComp *m_mmcState = nullptr;

    static constexpr uint32_t m_maxLCUSize = 64;  //!< Max LCU size 64

    CODECHAL_HEVC_IQ_MATRIX_PARAMS m_hevcIqMatrixParams = {};  //!< Pointer to IQ matrix parameter

    CODEC_PRE_ENC_PARAMS m_preEncConfig = {};

    PMOS_SURFACE m_rawDsSurface     = nullptr;
    PMOS_SURFACE m_preEncRawSurface = nullptr;

protected:
    virtual MOS_STATUS PreparePreEncConfig(void *params) = 0;

    bool IsCurrentUsedAsRef(uint8_t idx) const;

    virtual MOS_STATUS UpdateTrackedBufferParameters() override;
    virtual MOS_STATUS GetTrackedBuffers() override;
    virtual MOS_STATUS AllocateResources() override;
    virtual MOS_STATUS GetRecycleBuffers();
    virtual MOS_STATUS InitPreEncSize();

    MOS_STATUS SetSliceStructs();
    MOS_STATUS SetPictureStructs();
    MOS_STATUS ValidateLowDelayBFrame();
    MOS_STATUS ValidateSameRefInL0L1();

    int8_t          m_refIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC]     = {};       //!< Reference Index mapping
    bool            m_currUsedRefPic[CODEC_MAX_NUM_REF_FRAME_HEVC]    = {};       //!< Reference picture usage array
    bool            m_lowDelay                                        = false;    //!< Low delay flag
    bool            m_sameRefList                                     = false;    //!< Flag to specify if ref list L0 and L1 are same
    PreEncInfo      m_preEncInfo                                      = {};

    //Deblocking
    bool          m_SliceDeblockingFilterDisabled                  = false;
    uint32_t      m_SliceTcOffsetDiv2                              = 0;
    uint32_t      m_SliceBettaOffsetDiv2                           = 0;
    PMOS_RESOURCE m_resDeblockingFilterTileRowStoreScratchBuffer   = nullptr;  //!< De-blocking filter tile row store Scratch data buffer
    PMOS_RESOURCE m_resDeblockingFilterColumnRowStoreScratchBuffer = nullptr;  //!< De-blocking filter column row Store scratch data buffer
    PMOS_RESOURCE m_resDeblockingFilterRowStoreScratchBuffer       = nullptr;  //!< Handle of De-block row store surface

    uint32_t rawCTUBits = 0;
    uint8_t  m_QP       = 22;

    uint32_t m_encodeMode             = 0;
    uint8_t  EncodePreencBasicMember0 = 0;
    uint8_t  EncodePreencBasicMember1 = 0;
    uint32_t EncodePreencBasicMember2 = 0;
    uint8_t  EncodePreencBasicMember5 = 0;  
    uint8_t  EncodePreencBasicMember6 = 0;
    uint32_t m_preEncSrcWidth         = 0;
    uint32_t m_preEncSrcHeight        = 0;

    PMOS_RESOURCE EncodePreencBasicMember3 = nullptr;
    PMOS_RESOURCE EncodePreencBasicMember4 = nullptr;

    EncodePreEncConstSettings *m_preEncConstSettings = nullptr;

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__PreEncBasicFeature)
};
}  // namespace encode

#endif  // !__ENCODE_PREENC_BASIC_FEATURE_H__
