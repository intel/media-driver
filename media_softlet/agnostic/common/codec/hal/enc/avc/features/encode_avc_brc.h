/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     encode_avc_brc.h
//! \brief    Defines the common interface for avc brc features
//!
#ifndef __ENCODE_AVC_BRC_H__
#define __ENCODE_AVC_BRC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_avc_basic_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_mi_itf.h"

#if (_SW_BRC)
#include "encode_sw_brc.h"
#endif  // !_SW_BRC

namespace encode
{
struct VdencAvcHucBrcUpdateDmem;

class AvcEncodeBRC : public MediaFeature,
    public mhw::vdbox::vdenc::Itf::ParSetting,
    public mhw::vdbox::huc::Itf::ParSetting
{
    enum AvcBrcFrameType
    {
        P_FRAME  = 0,
        B_FRAME  = 1,
        I_FRAME  = 2,
        B1_FRAME = 3,
        B2_FRAME = 4,
    };

public:
    AvcEncodeBRC(MediaFeatureManager *featureManager,
                 EncodeAllocator *allocator,
                 CodechalHwInterfaceNext *hwInterface,
                 void *constSettings);

    virtual ~AvcEncodeBRC();

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
    //! \brief    Disable Brc Init and Reset after BRC update
    //!
    void DisableBrcInitReset() { m_brcInit = m_brcReset = false; }

    //!
    //! \brief    Enable VDEnc single pass
    //!
    void EnableVdencSinglePass() { m_vdencSinglePassEnable = true; }

    //!
    //! \brief    Check if BRC Init enabled
    //!
    //! \return   bool
    //!           true if BRC Init enabled, else Brc Init disabled.
    //!
    bool IsBRCInit() { return m_brcInit; }

    //!
    //! \brief    Check if BRC Reset enabled
    //!
    //! \return   bool
    //!           true if BRC Reset enabled, else BRC Reset disabled.
    //!
    virtual bool IsBRCInitRequired() { return m_vdencBrcEnabled & (m_brcInit || m_brcReset); }

    //!
    //! \brief    Check if BRC Update enabled
    //!
    //! \return   bool
    //!           true if BRC Reset enabled, else BRC Reset disabled.
    //!
    virtual bool IsBRCUpdateRequired() { return m_vdencBrcEnabled; }

    //!
    //! \brief    Check whether VDEnc BRC enabled
    //!
    //! \return   bool
    //!           true if VDEnc BRC enabled, otherwise false.
    //!
    virtual bool IsVdencBrcEnabled() { return m_vdencBrcEnabled; }

    //!
    //! \brief    Check whether VDEnc BRC is supported
    //!
    //! \param    [in] params
    //!           Pointer to AVC Sequence parameters
    //! \return   bool
    //!           true if VDEnc BRC is supported, otherwise false.
    //!
    bool IsVdencBrcSupported(PCODEC_AVC_ENCODE_SEQUENCE_PARAMS avcSeqParams);

    //!
    //! \brief    Check whether MBBRC enabled
    //!
    //! \return   bool
    //!           true if MBBRC enabled, otherwise false.
    //!
    virtual bool IsMbBrcEnabled()  { return m_mbBrcEnabled; }

    //!
    //! \brief    Help function to check if the rate control method is BRC
    //!
    //! \param    [in] rc
    //!           Rate control method
    //!
    //! \return   True if using BRC , else return false
    //!
    bool IsRateControlBrc(uint8_t rc)
    {
        return (rc == RATECONTROL_CBR) ||
               (rc == RATECONTROL_VBR) ||
               (rc == RATECONTROL_AVBR) ||
               (rc == RATECONTROL_CQL) ||
               (rc == RATECONTROL_VCM) ||
               (rc == RATECONTROL_ICQ) ||
               (rc == RATECONTROL_QVBR) ||
               (rc == RATECONTROL_IWD_VBR);
    }

    PMHW_BATCH_BUFFER GetBatchBufferForVdencImgStat()  { return &m_batchBufferForVdencImgStat; }

    //!
    //! \brief  Set Dmem buffer for brc Init
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemForInit(void *params);

    //!
    //! \brief  Set Dmem buffer for brc update
    //! \param  [in] params
    //!         Pointer to parameters
    //! \param  [in] currPass
    //!         Current pass number
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemForUpdate(void *params, uint16_t currPass, bool bIsLastPass);

    virtual MOS_STATUS FillHucConstData(uint8_t *data, uint8_t pictureType);

    //!
    //! \brief    Add params to read the MFC status
    //!
    //! \param    params
    //!           [out] the parameters for Mfc status read
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetMfcStatusParams(EncodeStatusReadParams &params);

    virtual MOS_STATUS SaveBrcUpdateDmemBufferPtr(
        PMOS_RESOURCE vdencBrcUpdateDmemBuffer0,
        PMOS_RESOURCE vdencBrcUpdateDmemBuffer1);

    virtual MOS_STATUS SaveHucStatus2Buffer(PMOS_RESOURCE hucStatus2Buffer);

    virtual PMOS_RESOURCE GetHucStatus2Buffer() { return m_hucStatus2BufferPtr; }

    uint16_t GetAdaptiveRoundingNumSlices() const { return static_cast<uint16_t>(m_basicFeature->m_numSlices); }

    uint32_t GetVdencBRCImgStateBufferSize();

    uint32_t GetVdencOneSliceStateSize();

    bool IsBPyramidWithGoldenBGOP();

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

#if _SW_BRC
    std::shared_ptr<EncodeSwBrc> m_swBrc = nullptr;
#endif

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
    virtual MOS_STATUS FreeBrcResources();

    MOS_STATUS SetSequenceStructs();

    void SetMbBrc();

    virtual MOS_STATUS LoadConstTable0(uint8_t constTable[8][42]);
    virtual MOS_STATUS LoadConstTable3(uint8_t pictureType, uint8_t ConstTable3[42]);
    virtual MOS_STATUS LoadConstTable5(uint8_t pictureType, uint16_t ConstTable5[42]);
    virtual MOS_STATUS LoadConstTable6(uint8_t pictureType, uint16_t ConstTable6[42]);
    virtual MOS_STATUS LoadConstTable7(uint8_t pictureType, uint8_t ConstTable7[42]);
    virtual MOS_STATUS LoadConstTable8(uint8_t pictureType, uint8_t ConstTable8[42]);

    //!
    //! \brief    VDENC Compute BRC Init QP..
    //! \return   initQP
    //!
    int32_t ComputeBRCInitQP();

    MOS_STATUS DeltaQPUpdate(uint8_t qpModulationStrength, bool bIsLastPass);

    void SetFrameTypeForUpdate(VdencAvcHucBrcUpdateDmem *dmem, uint16_t currPass);

    void CalculateCurLvlInBGop(uint16_t curFrameIdxInBGop, uint16_t begin, uint16_t end, uint16_t curLvl, uint16_t &curOrder, uint16_t &retLvl);

    CodechalHwInterfaceNext    *m_hwInterface    = nullptr;
    EncodeAllocator        *m_allocator      = nullptr;
    AvcBasicFeature        *m_basicFeature   = nullptr;  //!< EncodeBasicFeature

    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;
    std::shared_ptr<mhw::vdbox::mfx::Itf>   m_mfxItf   = nullptr;
    std::shared_ptr<mhw::mi::Itf>           m_miItf    = nullptr;

    // Batch Buffers
    MHW_BATCH_BUFFER       m_batchBufferForVdencImgStat = {}; //!< VDEnc image state batch buffers

    //!< One for 1st pass of next frame, and the other for the next pass of current frame.
    PMOS_RESOURCE m_vdencBrcUpdateDmemBufferPtr[2] = {nullptr, nullptr};
    PMOS_RESOURCE m_hucStatus2BufferPtr            = nullptr;

    uint8_t  m_rcMode = 0;

    bool m_brcInit                     = true;   //!< BRC init flag
    bool m_brcReset                    = false;  //!< BRC reset flag
    bool m_mbBrcEnabled                = false;  //!< MBBrc enable flag.
    bool m_mbBrcUserFeatureKeyControl  = false;  //!< MBBRC user feature control enable flag.
    bool m_vdencBrcEnabled             = false;  //!< Vdenc bitrate control enabled flag
    bool m_vdencSinglePassEnable       = false;  //!< Enable VDEnc single pass
    bool m_isFirstDeltaQPCalculation   = true;   //!< Check if it's first time delta qp modulation calculation

    double   m_dBrcInitCurrentTargetBufFullInBits = 0;  //!< BRC init current target buffer full in bits
    double   m_dBrcInitResetInputBitsPerFrame     = 0;  //!< BrcInitReset Input Bits Per Frame
    uint32_t m_brcInitPreviousTargetBufFullInBits = 0;  //!< BRC Init Previous Target Buffer Full In Bits
    double   m_dBrcTargetSize                     = 0;  //!< BRC target size.
    uint8_t  m_qpModulationStrength               = 0;  //!< Current QP modulation strength

    uint16_t m_frameIdxInBGop                     = 0;  //!< Current frame index in BGOP in encoding order

#if (_DEBUG || _RELEASE_INTERNAL)
    static const uint32_t m_bufferFulnessDataSize = 600;
    uint32_t       m_bufferFulnessData_csv[m_bufferFulnessDataSize] = {};     //!< bufFulness for lpla. Read from file
    bool           m_useBufferFulnessData       = false; //!< To force using bufFulness for lpla from input file
#endif

MEDIA_CLASS_DEFINE_END(encode__AvcEncodeBRC)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_BRC_H__
