/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     encode_vp9_basic_feature.h
//! \brief    Defines the common interface for encode vp9 basic feature
//!
#ifndef __ENCODE_VP9_BASIC_FEATURE_H__
#define __ENCODE_VP9_BASIC_FEATURE_H__

#include "encode_basic_feature.h"
#include "encode_mem_compression.h"

#include "codec_def_encode_vp9.h"

#include "encode_vp9_reference_frames.h"
#include "media_vp9_packet_defs.h"

#include "mhw_vdbox_huc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "mhw_vdbox_mfx_itf.h"

#include "mhw_vdbox_vdenc_cmdpar.h"

#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"

#include "media_packet.h"

namespace encode
{
#define CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_WIDTH  256
#define CODECHAL_ENCODE_VP9_MIN_TILE_SIZE_HEIGHT 128
#define CODECHAL_ENCODE_MIN_SCALED_SURFACE_SIZE  48


namespace TargetUsage
{
//!
//! \enum  MODE
//! \brief TargetUsage mode
//!
enum MODE
{
    QUALITY     = 1,
    QUALITY_2   = 2,
    NORMAL      = 4,
    PERFORMANCE = 7
};

//! \brief  Check whether the target usage mode is valid.
//! \return bool
//!         'true' if the target usage mode is valid and 'false' otherwise.
//!
inline bool isValid(uint8_t targetUsage)
{
    bool res = true;

    switch (targetUsage)
    {
    case QUALITY:
    case QUALITY_2:
    case NORMAL:
    case PERFORMANCE:
        break;
    default:
        MHW_NORMALMESSAGE("Invalid TU provided!");
        res = false;
        break;
    }

    return res;
}

//!
//! \brief  Check whether the target usage mode is the TU1 or TU2 quality mode.
//! \return bool
//!         'true' if the target usage mode is the TU1 or TU2 quality mode and 'false' otherwise.
//!
inline bool isQuality(uint8_t targetUsage)
{
    if (QUALITY == targetUsage || QUALITY_2 == targetUsage)
    {
        return true;
    }
    return false;
}

//!
//! \brief  Check whether the target usage mode is the TU4 normal mode.
//! \return bool
//!         'true' if the target usage mode is the TU4 normal mode and 'false' otherwise.
//!
inline bool isNormal(uint8_t targetUsage)
{
    if (NORMAL == targetUsage)
    {
        return true;
    }
    return false;
}

//!
//! \brief  Check whether the target usage mode is the TU7 speed mode.
//! \return bool
//!         'true' if the target usage mode is the TU7 speed mode and 'false' otherwise.
//!
inline bool isSpeed(uint8_t targetUsage)
{
    if (PERFORMANCE == targetUsage)
    {
        return true;
    }
    return false;
}
};  // namespace TargetUsage


class Vp9BasicFeature : public EncodeBasicFeature, 
    public mhw::vdbox::huc::Itf::ParSetting,
    public mhw::vdbox::hcp::Itf::ParSetting,
    public mhw::vdbox::vdenc::Itf::ParSetting,
    public mhw::vdbox::mfx::Itf::ParSetting,
    public mhw::mi::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9BasicFeature constructor
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] trackedBuf
    //!         Pointer to TrackedBuffer
    //! \param  [in] recycleBuf
    //!         Pointer to RecycleResource
    //!
    Vp9BasicFeature(EncodeAllocator *allocator,
                    CodechalHwInterfaceNext *hwInterface,
                    TrackedBuffer *trackedBuf,
                    RecycleResource *recycleBuf,
                    void *constSettings = nullptr) : EncodeBasicFeature(allocator, hwInterface, trackedBuf, recycleBuf) { m_constSettings = constSettings; };

    //!
    //! \brief  Vp9BasicFeature destructor
    //!
    virtual ~Vp9BasicFeature(){};

    //!
    //! \brief  Init VP9 encode basic feature's parameter setting
    //! \param  [in] setting
    //!         Pointer to CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update VP9 encode basic feature parameters
    //! \param  [in] params
    //!         Pointer to EncoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Get profile level max frame size
    //! \return uint32_t
    //!         Profile level max frame size
    //!
    virtual uint32_t GetProfileLevelMaxFrameSize() override;

    //!
    //! \brief    Get Os interface
    //! \details  Get Os interface in codechal hw interface
    //! \return   [out] PMOS_INTERFACE
    //!           Interface got.
    //!
    inline PMOS_INTERFACE GetOsInterface() { return m_osInterface; }

    //!
    //! \brief    Get encode allocator interfacae
    //! \details  Get encode allocator interface
    //! \return   [out] EncodeAllocator
    //!           Encode allocator interface
    //!
    inline EncodeAllocator* GetAllocator() { return m_allocator; }

    //!
    //! \brief      Resize 4x and 8x DS recon Surfaces to VDEnc
    //! \param      [in] bufIdx
    //!             Index of the surface
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Resize4x8xforDS(uint8_t bufIdx);

    //!
    //! \brief  Update parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateParameters();

    //!
    //! \brief  Convert to Sign Magnitude
    //! \return uint32_t
    //!         value calculated
    //!
    uint32_t Convert2SignMagnitude(int32_t val, uint32_t signBitPos) const;

    //!
    //! \brief  Enable Decode mode for MFX
    //!
    void SetDecodeInUse(bool decodeInUse) { m_decodeInUse = decodeInUse; }

    //!
    //! \brief  Check whether HME is enabled for the target usage mode.
    //! \return bool
    //!         'true' if HME is enabled for the target usage mode and 'false' otherwise.
    //!
    virtual bool isHmeEnabledForTargetUsage(uint8_t targetUsage) const;

    //!
    //! \brief MHW parameters declaration
    //! 
    MHW_SETPAR_DECL_HDR(HCP_VP9_PIC_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_CMD1);
    MHW_SETPAR_DECL_HDR(VDENC_SRC_SURFACE_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_DS_REF_SURFACE_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(MFX_WAIT);
    MHW_SETPAR_DECL_HDR(MFX_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    EncodeMemComp *m_mmcState = nullptr;

    // Parameters passed from application
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS m_vp9SeqParams     = nullptr;  //!< Pointer to sequence parameters
    PCODEC_VP9_ENCODE_PIC_PARAMS      m_vp9PicParams     = nullptr;  //!< Pointer to picture parameters
    PCODEC_VP9_ENCODE_SEGMENT_PARAMS  m_vp9SegmentParams = nullptr;  //!< Pointer to segment parameters
    Vp9ReferenceFrames                m_ref              = {};       //!< Reference List

    bool m_pakEnabled = false;  //!< flag to indicate if PAK is enabled
    bool m_encEnabled = false;  //!< flag to indicate if ENC is enabled

    bool m_dysCqp = false;
    bool m_dysBrc = false;
    bool m_tsEnabled                      = false;
    bool m_vdencPakonlyMultipassEnabled   = false;  //!< Flag to signal (VDEnc + PAK) vs. (PAK Only)

    uint8_t m_txMode = 0;  //!< TX mode

    // User feature key capabilities
    bool m_hucEnabled     = true;   //!< HUC usage flag
    bool m_hmeEnabled     = false;  //!< Flag indicate if HME is enabled
    bool m_hmeSupported   = false;  //!< Flag indicate if HME is supported
    bool m_16xMeEnabled   = false;  //!< Flag indicate if 16x ME is enabled
    bool m_16xMeSupported = false;  //!< Flag indicate if 16x ME is supported
    bool m_32xMeSupported = false;  //!< Flag indicate if 32x ME is supported

    uint32_t m_maxPicWidth      = 0;  //!< Max picture width
    uint32_t m_maxPicHeight     = 0;  //!< Max picture height
    uint32_t m_maxPicWidthInSb  = 0;  //!< Max picture width in SB unit
    uint32_t m_maxPicHeightInSb = 0;  //!< Max picture height in SB unit
    uint32_t m_maxPicSizeInSb   = 0;  //!< Max picture size in SB unit

    uint32_t m_maxTileNumber = 1;  //!< max number of tile
    uint32_t m_picWidthInSb  = 0;  //!< Picture width in SB unit
    uint32_t m_picHeightInSb = 0;  //!< Picture height in SB unit
    uint32_t m_picSizeInSb   = 0;  //!< Picture size in SB unit

    uint16_t m_slbbImgStateOffset = 0;
    uint16_t m_hucPicStateOffset  = 0;
    uint16_t m_hucSlbbSize        = 0;

    uint32_t m_bitstreamUpperBound = 0;  //!< Bitstream upper bound

    uint32_t m_sizeOfSseSrcPixelRowStoreBufferPerLcu = 0;  //!< Size of SSE row store buffer per LCU

    uint8_t  m_minScaledDimension                    = 0;  //!< min scaled dimension
    uint8_t  m_minScaledDimensionInMb                = 0;  //!< min scaled dimension in Mb

    uint32_t m_downscaledFrameFieldHeightInMb4x  = 0;  //!< Downscale frame field height in Mb 4x

    uint32_t m_downscaledWidth16x                = 0;  //!< Downscale width 16x
    uint32_t m_downscaledHeight16x               = 0;  //!< Downscale height 16x
    uint32_t m_downscaledWidthInMb16x            = 0;  //!< Downscale width in Mb 16x
    uint32_t m_downscaledHeightInMb16x           = 0;  //!< Downscale height in Mb 16x
    uint32_t m_downscaledFrameFieldHeightInMb16x = 0;  //!< Downscale frame field height in Mb 16x

    uint32_t m_downscaledWidth32x      = 0;  //!< Downscale width 32x
    uint32_t m_downscaledHeight32x     = 0;  //!< Downscale height 32x
    uint32_t m_downscaledWidthInMb32x  = 0;  //!< Downscale width 32x
    uint32_t m_downscaledHeightInMb32x = 0;  //!< Downscale height 32x

    PMOS_RESOURCE m_resMvTemporalBuffer    = nullptr;  //!< Pointer to MOS_RESOURCE of MvTemporal Buffer
    PMOS_RESOURCE m_hucPakIntBrcDataBuffer = nullptr;  //!< HuC PAK integration BRC data buffer
    PMOS_RESOURCE m_resSegmentIdBuffer     = nullptr;  //!< Segment ID buffer

    uint8_t m_contextFrameTypes[CODEC_VP9_NUM_CONTEXTS] = {0};
    bool    m_prevFrameSegEnabled                       = false;

    bool m_scalableMode             = false;  //!< Flag indicate scalable mode
    bool m_lastFrameScalableMode    = false;  //!< Flag indicate last frame scalable mode
    bool m_dysVdencMultiPassEnabled = false;  //!< Flag DYS vdenc multiple pass enabled
    bool m_pakOnlyModeEnabledForLastPass = false;  //!< Pak only mode enaled for last pass

    uint8_t m_currMvTemporalBufferIndex = 0;  //!< Current MV temporal buffer index
    uint8_t m_lastMvTemporalBufferIndex = 0;  //!< Last MV temporal buffer index

    uint8_t m_currRecycledBufIdx = 0;  //!< Current recycled buffer index update from EncodePipeline::ResetParams()

    uint32_t m_mvOffset = 0;  //!< MV data offset, in 64 byte

    HucPrevFrameInfo m_prevFrameInfo = {0};

    bool     m_adaptiveRepakSupported                 = false;
    uint32_t m_rePakThreshold[CODEC_VP9_QINDEX_RANGE] = {0};

    uint8_t m_oriTargetUsage = 0; //!< Store original target usage pass from app. Use for CalculateRePakThresholds()

protected:
    //!
    //! \brief  Update the parameters of tracked buffers
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateTrackedBufferParameters() override;

    //!
    //! \brief  Get the buffers from tracked buffer manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetTrackedBuffers() override;

    //!
    //! \brief  Set sequence structures
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSequenceStructs();

    //!
    //! \brief  Set the picture structs
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPictureStructs();

    //!
    //! \brief  Motion estimation disable check
    //!
    void MotionEstimationDisableCheck();

    //!
    //! \brief      Resize DS recon surface for vdenc
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResizeDsReconSurfacesVdenc();

    //!
    //! \brief    Calculate rePak thresholds
    //! \details
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateRePakThresholds();

    static constexpr uint32_t m_maxLCUSize = 64;  //!< Max LCU size
    static constexpr uint32_t m_minLCUSize = 8;   //!< Min LCU size

    bool m_decodeInUse = false;

    enum CodecSelect
    {
        decoderCodec = 0,
        encoderCodec = 1
    };

MEDIA_CLASS_DEFINE_END(encode__Vp9BasicFeature)
};

}  // namespace encode

#endif  // !__ENCODE_VP9_BASIC_FEATURE_H__
