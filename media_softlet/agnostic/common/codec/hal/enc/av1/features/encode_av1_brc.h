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
//! \file     encode_av1_brc.h
//! \brief    Defines the common interface for av1 brc features
//!
#ifndef __ENCODE_AV1_BRC_H__
#define __ENCODE_AV1_BRC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "encode_pipeline.h"
#include "encode_recycle_resource.h"
#include "encode_av1_basic_feature.h"

namespace encode
{
#define AV1_BRC_HUC_STATUS_REENCODE_MASK     (1<<31)
static const uint32_t BRC_KBPS = 1000;     // 1000bps for disk storage, aligned with industry usage
static const uint32_t BRC_DATA_SIZE = 16*4;

enum AV1_BRC_FRAME_TYPE
{
    AV1_BRC_FRAME_TYPE_P_OR_LB = 0,
    AV1_BRC_FRAME_TYPE_B = 1,
    AV1_BRC_FRAME_TYPE_I = 2,
    AV1_BRC_FRAME_TYPE_B1 = 3,
    AV1_BRC_FRAME_TYPE_B2 = 4,
    AV1_BRC_FRAME_TYPE_B3 = 5,
    AV1_BRC_FRAME_TYPE_INVALID

};

    //!
    //! \struct    Av1VdencPakInfo
    //! \brief     AV1 Vdenc Pak info
    //!
    struct Av1VdencPakInfo
    {
        uint32_t  frameByteCount;
        uint8_t   pakPassNum;
    };
    struct SlbData
    {
        //SLB related fields.
        uint16_t slbSize = 0;
        uint16_t avpPicStateCmdNum = 1;
        uint16_t avpSegmentStateOffset = 0;
        uint16_t avpInloopFilterStateOffset = 0;
        uint16_t vdencCmd1Offset = 0;
        uint16_t vdencCmd2Offset = 0;
        uint16_t avpPicStateOffset = 0;
        uint16_t secondAvpPicStateOffset = 0;
        uint16_t pakInsertSlbSize = 0;
        uint16_t vdencTileSliceStateOffset = 0;
        uint16_t tileNum = 1;
    };
    //!
    //! \struct Av1BrcPakMmio
    //! \brief  MMIO of BRC and PAK
    //!
    struct Av1BrcPakMmio
    {
        uint32_t reEncode[4];
    };

    struct VdencAv1HucBrcInitDmem;
    struct VdencAv1HucBrcUpdateDmem;
    struct VdencAv1HucBrcConstantData;

    class Av1Brc : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::huc::Itf::ParSetting
    {
    public:
        Av1Brc(MediaFeatureManager *featureManager, EncodeAllocator *allocator, CodechalHwInterfaceNext *hwInterface, void *constSettings);

        virtual ~Av1Brc();

        //!
        //! \brief  Init av1 brc features related parameter
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
        //!
        //! \return   bool
        //!           true if BRC enabled, else BRC disabled.
        //!
        virtual bool IsBRCEnabled() const { return m_brcEnabled; }

        //!
        //! \brief    Disable Brc Init and Reset after BRC update
        //!
        void DisableBrcInitReset() { m_brcInit = m_brcReset = false; };

        //!
        //! \brief    Check if BRC Init or Reset enabled
        //!
        //! \return   bool
        //!           true if BRC Init or Reset enabled.
        //!
        virtual bool IsBRCInitRequired() const { return m_brcEnabled & (m_brcInit || m_brcReset); }

        //!
        //! \brief    Check if BRC Reset enabled
        //!
        //! \return   bool
        //!           true if BRC Reset enabled.
        //!
        virtual bool IsBRCResetRequired() const { return m_brcEnabled & m_brcReset; }

        PMHW_BATCH_BUFFER GetVdenc2ndLevelBatchBuffer(uint32_t currRecycledBufIdx) {
            return &m_vdenc2ndLevelBatchBuffer[currRecycledBufIdx];
        };

        PMHW_BATCH_BUFFER GetPakInsertOutputBatchBuffer(uint32_t currRecycledBufIdx)
        {
            return &m_pakInsertOutputBatchBuffer[currRecycledBufIdx];
        };

        MOS_STATUS GetBrcDataBuffer(MOS_RESOURCE *&buffer);

        //!
        //! \brief  Set Dmem buffer for brc update
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetDmemForUpdate(VdencAv1HucBrcUpdateDmem *params) const;

        //!
        //! \brief  Set Const data for brc update
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetConstForUpdate(VdencAv1HucBrcConstantData *params) const;

        //!
        //! \brief  Set Dmem buffer for brc Init
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetDmemForInit(VdencAv1HucBrcInitDmem *params) const;

        const SlbData& GetSLBData() { return m_slbData; };
        void SetSLBData(const SlbData& input) { m_slbData = input; };

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

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);
        MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

        // const data
        static constexpr uint32_t m_brcHistoryBufSize       = 6080;   //!< BRC history buffer size

        CodechalHwInterfaceNext  *m_hwInterface  = nullptr;
        EncodeAllocator          *m_allocator    = nullptr;
        Av1BasicFeature          *m_basicFeature = nullptr;  //!< EncodeBasicFeature

        SlbData  m_slbData = {};
        uint8_t  m_rcMode  = 0;

        bool m_brcInit              = true;   //!< BRC init flag
        bool m_brcReset             = false;  //!< BRC reset flag
        bool m_brcEnabled           = false;  //!< BRC enable flag

        //Resources
        MHW_BATCH_BUFFER   m_vdenc2ndLevelBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM]   = {};  //!< VDEnc 2nd level batch buffer
        MHW_BATCH_BUFFER   m_pakInsertOutputBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};  //!< PAK insert output batch buffer
        MOS_RESOURCE       m_vdencBrcDbgBuffer                                               = {};  //!< VDEnc brc debug buffer
        MOS_RESOURCE       m_resBrcDataBuffer                                                = {};  //!< Resource of bitrate control data buffer, only as an output of PAKintegrate Kernel

        MHW_VDBOX_NODE_IND m_vdboxIndex = MHW_VDBOX_NODE_1;

        mutable double m_curTargetFullness = 0;
        int32_t  m_delay = 0;

        int32_t m_vbvSize      = 0;
        int32_t m_frameRate    = 0;
        double m_inputbitsperframe = 0;

    MEDIA_CLASS_DEFINE_END(encode__Av1Brc)
    };

}  // namespace encode

#endif  // !__ENCODE_AV1_BRC_H__
