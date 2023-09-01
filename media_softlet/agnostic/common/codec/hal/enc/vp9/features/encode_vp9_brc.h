/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     encode_vp9_brc.h
//! \brief    Defines the common interface for vp9 brc features
//!
#ifndef __ENCODE_VP9_BRC_H__
#define __ENCODE_VP9_BRC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_pipeline.h"
#include "encode_vp9_basic_feature.h"
#include "media_vp9_packet_defs.h"

namespace encode
{
#define CODECHAL_ENCODE_VP9_PIC_STATE_BUFFER_SIZE_PER_PASS 192  // 42 DWORDs for Pic State one uint32_t for BB End + 5 uint32_tS reserved to make it aligned for kernel read
#define CODECHAL_ENCODE_VP9_CQP_NUM_OF_PASSES 2
#define CODECHAL_ENCODE_VP9_BRC_DEFAULT_NUM_OF_PASSES 2  // 2 Passes minimum so HuC is Run twice, second PAK is conditional.
#define CODECHAL_ENCODE_VP9_BRC_CONSTANTSURFACE_SIZE 17792
#define CODECHAL_ENCODE_VP9_SEGMENT_STATE_BUFFER_SIZE 256
#define CODECHAL_ENCODE_VP9_BRC_BITSTREAM_SIZE_BUFFER_SIZE 16
#define CODECHAL_ENCODE_VP9_BRC_MSDK_PAK_BUFFER_SIZE 64
#define CODECHAL_ENCODE_VP9_PAK_INSERT_UNCOMPRESSED_HEADER 80
#define CODECHAL_ENCODE_VP9_VDENC_DATA_EXTENSION_SIZE 32
#define CODECHAL_ENCODE_VP9_HUC_BRC_DATA_BUFFER_SIZE (16*4)
#define CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES 4
//!
//! \struct    BRC_BITSTREAM_SIZE_BUFFER
//! \brief     Brc bitstream size buffer
//!
struct EncodeVp9BSBuffer
{
    uint32_t dwHcpBitstreamByteCountFrame;
    uint32_t dwHcpImageStatusControl;
    uint32_t Reserved[2];
};

//!
//! \struct    HucBrcBuffers
//! \brief     HUC brc buffers
//!
struct HucBrcBuffers
{
    MOS_RESOURCE resBrcHistoryBuffer;
    MOS_RESOURCE resBrcConstantDataBuffer[2]; // 0 == I, 1 == P
    MOS_RESOURCE resBrcMsdkPakBuffer;
    MOS_RESOURCE resBrcMbEncCurbeWriteBuffer;
    MOS_RESOURCE resMbEncAdvancedDsh;
    MOS_RESOURCE resPicStateBrcReadBuffer;
    MOS_RESOURCE resPicStateBrcWriteHucReadBuffer;
    MOS_RESOURCE resPicStateHucWriteBuffer;
    MOS_RESOURCE resSegmentStateBrcReadBuffer;
    MOS_RESOURCE resSegmentStateBrcWriteBuffer;
    MOS_RESOURCE resBrcBitstreamSizeBuffer;
    MOS_RESOURCE resBrcHucDataBuffer;
};

class Vp9EncodeBrc : public MediaFeature, public mhw::vdbox::huc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    Vp9EncodeBrc(MediaFeatureManager *featureManager, EncodeAllocator *allocator, CodechalHwInterfaceNext *hwInterface, void *constSettings);

    virtual ~Vp9EncodeBrc();

    //!
    //! \brief  Init cqp basic features related parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update cqp basic features related parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params) override;

    //!
    //! \brief    Check if BRC enabled
    //! \return   bool
    //!           true if BRC enabled, else BRC disabled.
    //!
    bool IsBrcEnabled() { return m_brcEnabled; }

    //!
    //! \brief    Check if VDENC BRC enabled
    //! \return   bool
    //!           true if VDENC BRC enabled, else VDENC BRC disabled.
    //!
    bool IsVdencBrcEnabled() { return m_vdencBrcEnabled; }

    //!
    //! \brief    Disable Brc Init and Reset after BRC update
    //!
    void DisableBrcInitReset() { m_brcInit = m_brcReset = false; };

    void BrcReset(bool val) { m_brcReset = val; };
    bool IsBrcReset() { return m_brcReset; };

    //!
    //! \brief    Check if BRC Init enabled
    //! \return   bool
    //!           true if BRC Init enabled, else Brc Init disabled.
    //!
    bool IsBrcInit() const { return m_brcInit; }

    //!
    //! \brief    Check if BRC Reset enabled
    //! \return   bool
    //!           true if BRC Reset enabled, else BRC Reset disabled.
    //!
    bool IsBrcInitRequired() { return m_vdencBrcEnabled & (m_brcInit || m_brcReset); }

    //!
    //! \brief    Check if BRC Update enabled
    //! \return   bool
    //!           true if BRC Reset enabled, else BRC Reset disabled.
    //!
    bool IsBrcUpdateRequired() { return m_vdencBrcEnabled; }

    //!
    //! \brief    Help function to check if the rate control method is BRC
    //! \param    [in] rc
    //!           Rate control method
    //! \return   True if using BRC , else return false
    //!
    bool IsRateControlBrc(uint8_t rc)
    {
        return (rc == RATECONTROL_CBR) ||
               (rc == RATECONTROL_VBR) ||
               (rc == RATECONTROL_AVBR) ||
               (rc == RATECONTROL_VCM) ||
               (rc == RATECONTROL_ICQ) ||
               (rc == RATECONTROL_QVBR);
    }

    //!
    //! \brief    Check whether multi-pass brc supported
    //! \return   bool
    //!           true if adaptive repak supported, otherwise false.
    //!
    bool IsMultipassBrcSupported() const { return m_multipassBrcSupported; }

    //!
    //! \brief  Set regions for brc update
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRegionsForBrcUpdate(
        mhw::vdbox::huc::HUC_VIRTUAL_ADDR_STATE_PAR &params) const; 

    //!
    //! \brief  Set Dmem buffer for brc update
    //! \param  [in] params
    //!         Pointer to parameters
    //! \param  [in] isFirstPass
    //!         Indicate is this first pass
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemForUpdate(void *params, bool isFirstPass) const;

    //!
    //! \brief  Set Dmem buffer for brc Init
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemForInit(void *params) const;

    //!
    //! \brief  Get huc brc buffers
    //! \param  [out] buffers
    //!         Reference to the huc brc buffers get from Brc feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHucBrcBuffers(
        HucBrcBuffers *&buffers);

    //!
    //! \brief  Get brc history buffer size
    //! \param  [out] size
    //!         Brc history buffer size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetBrcHistoryBufferSize(
        uint32_t &size);

    //!
    //! \brief  Get vdenc brc stats buffer size
    //! \param  [out] size
    //!         Vdenc brc stats buffer size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVdencBrcStatsBufferSize(
        uint32_t &size);

    //!
    //! \brief  Get vdenc brc pak stats buffer size
    //! \param  [out] size
    //!         Vdenc brc stats buffer size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVdencBrcPakStatsBufferSize(
        uint32_t &size);

    //!
    //! \brief  Initialize brc constant data buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitConstantDataBuffer() const;

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_VP9_PIC_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    // Shared constants
    static constexpr uint32_t m_brcMaxNumPasses        = 3;
    static constexpr uint32_t m_brcStatsBufSize        = ((48 + 256) * sizeof(uint32_t));
    static constexpr uint32_t m_brcPakStatsBufSize     = (64 * sizeof(uint32_t));
    static constexpr uint32_t m_brcConstantSurfaceSize = 1664;

    mutable double m_inputBitsPerFrame = 0.0;
    mutable double m_curTargetFullness = 0.0;

protected:
    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Free resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeResources();

    //!
    //! \brief  Prepare BRC related settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBrcSettings(void *params) const;

    //!
    //! \brief  Calculate VP9 BRC init QP value
    //!
    //! \param  [in, out] initQpI
    //!         Calculated initial QP value for I-frame
    //! \param  [in, out] initQpP
    //!         Calculated initial QP value for P-frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ComputeVDEncInitQP(int32_t &initQpI, int32_t &initQpP) const;

    //!
    //! \brief      Calculate temporal ratios
    //!
    //! \param      [in] numberOfLayers
    //!             Number of layers
    //! \param      [in] maxTemporalBitrate
    //!             Max temporal frame rate
    //! \param      [in] maxTemporalFrameRate
    //!             Frame rate
    //! \param      [in] maxLevelRatios
    //!             Max level ratios
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateTemporalRatios(
        uint16_t   numberOfLayers,
        uint32_t   maxTemporalBitrate,
        FRAME_RATE maxTemporalFrameRate,
        uint8_t *  maxLevelRatios) const;

    //!
    //! \brief      Calculate normalized denominator
    //!
    //! \param      [in] frameRates
    //!             Pointer to frame rate
    //! \param      [in] numberOfLayers
    //!             Number of layers
    //! \param      [in] normalizedDenominator
    //!             Normalized denominator
    //!
    //! \return     uint32_t
    //!             Return 0 if call success, else -1 if fail
    //!
    uint32_t CalculateNormalizedDenominator(
        FRAME_RATE *frameRates,
        uint16_t    numberOfLayers,
        uint32_t    normalizedDenominator) const;

    //!
    //! \brief  Set sequence structures
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSequenceStructs();

    //!
    //! \brief  GetB Brc const buffer accoring to picture coding type
    //! \param  [in] pictureCodingType
    //!         coding type of picture
    //! \return PMOS_RESOURCE
    //!         Pointer to Brc const buffer
    //!
    const MOS_RESOURCE* GetBrcConstantBuffer() const;

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

    static const uint32_t     m_brcConstData[2][416];

    static constexpr float m_devStdFps    = 30.0;
    static constexpr float m_bpsRatioLow  = 0.1f;
    static constexpr float m_bpsRatioHigh = 3.5;

    static constexpr int32_t m_numInstRateThresholds = 4;
    static constexpr int32_t m_numDevThresholds      = 8;
    static constexpr int32_t m_posMultPb             = 50;
    static constexpr int32_t m_negMultPb             = -50;
    static constexpr int32_t m_posMultVbr            = 100;
    static constexpr int32_t m_negMultVbr            = -50;

    static constexpr int8_t m_instRateThresholdI[m_numInstRateThresholds] = {30, 50, 90, 115};
    static constexpr int8_t m_instRateThresholdP[m_numInstRateThresholds] = {30, 50, 70, 120};
    static constexpr double m_devThresholdFpNegI[m_numDevThresholds / 2]  = {0.80, 0.60, 0.34, 0.2};
    static constexpr double m_devThresholdFpPosI[m_numDevThresholds / 2]  = {0.2, 0.4, 0.66, 0.9};
    static constexpr double m_devThresholdFpNegPB[m_numDevThresholds / 2] = {0.90, 0.66, 0.46, 0.3};
    static constexpr double m_devThresholdFpPosPB[m_numDevThresholds / 2] = {0.3, 0.46, 0.70, 0.90};
    static constexpr double m_devThresholdVbrNeg[m_numDevThresholds / 2]  = {0.90, 0.70, 0.50, 0.3};
    static constexpr double m_devThresholdVbrPos[m_numDevThresholds / 2]  = {0.4, 0.5, 0.75, 0.90};

    // VDENC BRC related buffer size
    static constexpr uint32_t m_brcHistoryBufSize = 1152;

    CodechalHwInterfaceNext *   m_hwInterface    = nullptr;
    EncodeAllocator *       m_allocator      = nullptr;
    Vp9BasicFeature *       m_basicFeature   = nullptr;

    std::shared_ptr<mhw::vdbox::hcp::Itf>   m_hcpInterfaceNew   = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencInterfaceNew = nullptr;

    bool m_brcInit         = true;   //!< BRC init flag
    bool m_brcReset        = false;  //!< BRC reset flag
    bool m_brcEnabled      = false;  //!< BRC enable flag
    bool m_vdencEnabled    = false;  //!< Vdenc enabled flag
    bool m_vdencBrcEnabled = false;  //!< Vdenc bitrate control enabled flag

    // BRC Resources/Buffers
    HucBrcBuffers m_brcBuffers                  = {};
    uint32_t      m_brcHistoryBufferSize        = 0;    //!< Bitrate control history buffer size
    uint32_t      m_vdencBrcStatsBufferSize     = 0;    //!< VDENC bitrate control buffer size
    MOS_RESOURCE  m_resFrameStatStreamOutBuffer = {0};  //!< Frame statistics stream out buffer
    uint32_t      m_vdencBrcPakStatsBufferSize  = 0;    //!< VDENC bitrate control PAK buffer size
    bool          m_multipassBrcSupported       = true;  //!< Multi-pass bitrate control supported flag
    bool          m_initBrcConstantDataBuffer   = false; //!< BrcConstantDataBuffer init flag

MEDIA_CLASS_DEFINE_END(encode__Vp9EncodeBrc)
};
}  // namespace encode

#endif
