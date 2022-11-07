/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_hevc_brc.h
//! \brief    Defines the common interface for hevc brc features
//!
#ifndef __ENCODE_HEVC_BRC_H__
#define __ENCODE_HEVC_BRC_H__

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "encode_recycle_resource.h"
#include "encode_basic_feature.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "mhw_vdbox_huc_itf.h"

namespace encode
{
    //!
    //! \struct    CodechalVdencHevcPakInfo
    //! \brief     Codechal Vdenc HEVC Pak info
    //!
    struct CodechalVdencHevcPakInfo
    {
        uint32_t  FrameByteCount;
        uint8_t   PAKPassNum;
    };

    //!
    //! \struct    HevcVdencBrcBuffers
    //! \brief     Hevc Vdenc brc buffers
    //!
    struct HevcVdencBrcBuffers
    {
        PMOS_RESOURCE resBrcPakStatisticBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM];
        uint32_t     currBrcPakStasIdxForRead;
        uint32_t     currBrcPakStasIdxForWrite;
    };

    struct VdencHevcHucBrcInitDmem
    {
        uint32_t BRCFunc_U32;   // 0: Init; 2: Reset, bit7 0: frame-based; 1: tile-based
        uint32_t UserMaxFrame;  // ProfileLevelMaxFrame_U32
        uint32_t InitBufFull_U32;
        uint32_t BufSize_U32;
        uint32_t TargetBitrate_U32;
        uint32_t MaxRate_U32;
        uint32_t MinRate_U32;
        uint32_t FrameRateM_U32;
        uint32_t FrameRateD_U32;
        uint32_t LumaLog2WeightDenom_U32;
        uint32_t ChromaLog2WeightDenom_U32;
        uint8_t  BRCFlag : 7;  // ACQP/ICQ=0, CBR=1, VBR=2, VCM=3, LOWDELAY=4
        uint8_t  SSCFlag : 1;  // SSC: 0x80
        uint8_t  Reserved;
        uint16_t GopP_U16;
        uint16_t GopB_U16;
        uint16_t FrameWidth_U16;
        uint16_t FrameHeight_U16;
        uint16_t GopB1_U16;
        uint16_t GopB2_U16;
        uint8_t  MinQP_U8;
        uint8_t  MaxQP_U8;
        uint8_t  MaxBRCLevel_U8;
        uint8_t  LumaBitDepth_U8;
        uint8_t  ChromaBitDepth_U8;
        uint8_t  CuQpCtrl_U8;  // 0=No CUQP; 1=CUQP for I-frame; 2=CUQP for P/B-frame

        uint8_t RSVD0[4];
        int8_t  DevThreshPB0_S8[8];
        int8_t  DevThreshVBR0_S8[8];
        int8_t  DevThreshI0_S8[8];
        int8_t  InstRateThreshP0_S8[4];
        int8_t  InstRateThreshB0_S8[4];
        int8_t  InstRateThreshI0_S8[4];
        uint8_t LowDelayMode_U8;
        uint8_t InitQPIP_U8;
        uint8_t InitQPB_U8;  // In CQP mode, InitQPB_U8= InitQPIP_U8
        uint8_t QPDeltaThrForAdapt2Pass_U8;
        uint8_t TopFrmSzThrForAdapt2Pass_U8;
        uint8_t BotFrmSzThrForAdapt2Pass_U8;
        uint8_t QPSelectForFirstPass_U8;
        uint8_t MBHeaderCompensation_U8;
        uint8_t OverShootCarryFlag_U8;
        uint8_t OverShootSkipFramePct_U8;
        uint8_t EstRateThreshP0_U8[7];
        uint8_t EstRateThreshB0_U8[7];

        uint8_t  EstRateThreshI0_U8[7];
        uint8_t  QPP_U8;
        uint8_t  StreamInSurfaceEnable_U8;       // 0-disabled, 1-enabled
        uint8_t  StreamInROIEnable_U8;           // 0-disabled, 1-enabled
        uint8_t  TimingBudget_Enable_U8;         // 0-disabled, 1-enabled
        uint8_t  TopQPDeltaThrForAdapt2Pass_U8;  // 2
        uint8_t  BotQPDeltaThrForAdapt2Pass_U8;  // 1
        uint8_t  RESERVED;
        uint8_t  NetworkTraceEnable_U8;                   // 0-disabled, 1-enabled
        uint8_t  LowDelaySceneChangeXFrameSizeEnable_U8;  // 0-disabled, 1-enabled
        uint32_t ACQP_U32;                                // 1
        uint32_t SlidingWindow_Size_U32;                  // 30

        uint8_t SLIDINGWINDOW_MaxRateRatio;
        uint8_t LookaheadDepth_U8;
        int8_t  CbQPOffset;
        int8_t  CrQPOffset;

        uint32_t ProfileLevelMaxFramePB_U32;

        // tile-based BRC
        uint16_t SlideWindowRC;  // Reserved now
        uint16_t MaxLogCUSize;

        uint16_t FrameWidthInLCU;
        uint16_t FrameHeightInLCU;

        uint8_t  BRCPyramidEnable_U8;
        uint8_t  LongTermRefEnable_U8;
        uint16_t LongTermRefInterval_U16;
        uint8_t  LongTermRefMsdk_U8;
        uint8_t  IsLowDelay_U8;
        uint16_t RSVD3;
        uint32_t RSVD1[4];  // 64 bytes aligned
    };
    C_ASSERT(192 == sizeof(VdencHevcHucBrcInitDmem));

    //!
    //! \struct    EncodeReadBrcPakStatsParams
    //! \brief     Encode read brc pak states parameters
    //!
    struct EncodeReadBrcPakStatsParams
    {
        CodechalHwInterfaceNext    *pHwInterface;
        PMOS_RESOURCE           presBrcPakStatisticBuffer;
        PMOS_RESOURCE           presStatusBuffer;
        uint32_t                dwStatusBufNumPassesOffset;
        uint8_t                 ucPass;
        MOS_GPU_CONTEXT         VideoContext;
    };

    class HEVCEncodeBRC : public MediaFeature, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting, public mhw::vdbox::huc::Itf::ParSetting
    {
    public:
        HEVCEncodeBRC(MediaFeatureManager *featureManager, EncodeAllocator *allocator, CodechalHwInterfaceNext *hwInterface, void *constSettings);

        virtual ~HEVCEncodeBRC();

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
        //! \brief    Set ACQP status
        //!
        //! \param    [in]bool
        //!           expected ACQP status
        //!
        void SetACQPStatus(bool isEnabled) { m_hevcVDEncAcqpEnabled = isEnabled; }

        //!
        //! \brief    Check if ACQP enabled
        //!
        //! \return   bool
        //!           true if ACQP enabled, else ACQP disabled.
        //!
        bool IsACQPEnabled() { return m_hevcVDEncAcqpEnabled; }

        uint8_t RateControlMethod(){ return m_rcMode; }

        //!
        //! \brief    Check if BRC enabled
        //!
        //! \return   bool
        //!           true if BRC enabled, else BRC disabled.
        //!
        bool IsBRCEnabled() { return m_brcEnabled; }

        //!
        //! \brief    Disable Brc Init and Reset after BRC update
        //!
        void DisableBrcInitReset() { m_brcInit = m_brcReset = false; };

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
        virtual bool IsBRCInitRequired()        { return m_vdencHucUsed & (m_brcInit || m_brcReset); }

        //!
        //! \brief    Check if BRC Update enabled
        //!
        //! \return   bool
        //!           true if BRC Reset enabled, else BRC Reset disabled.
        //!
        virtual bool IsBRCUpdateRequired() { return m_vdencHucUsed; }

        //!
        //! \brief    Check whether VDEnc HuC using
        //!
        //! \return   bool
        //!           true if VDEnc HuC using, otherwise false.
        //!
        bool IsVdencHucUsed() const { return m_vdencHucUsed; }

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
                   (rc == RATECONTROL_VCM) ||
                   (rc == RATECONTROL_QVBR) ||
                   (rc == RATECONTROL_ICQ);
        }

        PMHW_BATCH_BUFFER GetVdenc2ndLevelBatchBuffer(uint32_t currRecycledBufIdx) {
            return &m_vdenc2ndLevelBatchBuffer[currRecycledBufIdx];
        };

        MOS_STATUS GetBrcDataBuffer(MOS_RESOURCE *&buffer);

        //!
        //! \brief  Set Dmem buffer for brc update
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetDmemForUpdate(void *params);

        //!
        //! \brief  Set Const data for brc update
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetConstForUpdate(void *params);

        //!
        //! \brief  Set Const Lambda data for brc update
        //! \param  [in] params
        //!         Pointer to parameters
        //! \param  [in] lambdaType
        //!         Indicate whether to use depth based calculation
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetConstLambdaForUpdate(void *params, bool lambdaType = false);

        MOS_STATUS SetHevcDepthBasedLambda(
            PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams,
            PCODEC_HEVC_ENCODE_PICTURE_PARAMS  hevcPicParams,
            uint8_t                            qp,
            uint16_t &                         SADQPLambda,
            uint16_t &                         RDQPLambda);

        //!
        //! \brief  Set Dmem buffer for brc Init
        //! \param  [in] params
        //!         Pointer to parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetDmemForInit(void *params);

        //!
        //! \brief    Read stats for BRC from PAK
        //!
        //! \param    [in] ucPass
        //!            current pass number
        //! \param    [in] offset
        //!            number pass offset in status buffer
        //! \param    [in] osResource
        //!            Pointer to resource of status buffer
        //! \param    [in] cmdBuffer
        //!            Pointer to command buffer
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SetReadBrcPakStatsParams(
            uint8_t                      ucPass,
            uint32_t                     offset,
            PMOS_RESOURCE                osResource,
            EncodeReadBrcPakStatsParams &readBrcPakStatsParams);

        HevcVdencBrcBuffers* GetHevcVdencBrcBuffers()
        {
            return &m_vdencBrcBuffers;
        };

        PMOS_RESOURCE GetHevcVdenc2ndLevelBatchBuffer(uint32_t currRecycledBufIdx)
        {
            return &m_vdenc2ndLevelBatchBuffer[currRecycledBufIdx].OsResource;
        };

        MOS_STATUS SetVdencBatchBufferState(
            const uint32_t    currRecycledBufIdx,
            const uint32_t    slcIdx,
            PMHW_BATCH_BUFFER &vdencBatchBuffer,
            bool &            vdencHucUsed);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(VDENC_CMD2);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

        MOS_STATUS SetCurrRecycledBufIdx(const uint8_t index);

    protected:
        //! \brief  Allocate feature related resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AllocateResources() override;

        //!
        //! \brief  Update feature related resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS UpdateBrcResources(void *params);

        //!
        //! \brief  Free resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS FreeBrcResources();

        MOS_STATUS SetBrcSettings(void *params);

        MOS_STATUS SetAcqpSettings(void *params);

        void ComputeVDEncInitQP(int32_t& initQPIP, int32_t& initQPB);

        void SetLcuBrc();

        MOS_STATUS SetSequenceStructs();

        // const data
        static constexpr uint32_t m_brcHistoryBufSize       = 6092;  //!< BRC history buffer size
        static constexpr uint32_t m_vdencBRCStatsBufferSize = 1216;  //!< Vdenc bitrate control buffer size
        static constexpr uint32_t m_deltaQpRoiBufferSize    = 65536;
        static constexpr uint32_t m_brcDebugBufSize         = 0x1000;                          //!< BRC debug buffer size
        static constexpr uint32_t m_roiStreamInBufferSize   = 65536 * CODECHAL_CACHELINE_SIZE; //!< ROI Streamin buffer size (part of BRC Update)

        const uint32_t m_brc_kbps = 1000;     // 1000bps for disk storage, aligned with industry usage

        CodechalHwInterfaceNext  *m_hwInterface  = nullptr;
        EncodeAllocator      *m_allocator    = nullptr;
        HevcBasicFeature     *m_basicFeature = nullptr;  //!< EncodeBasicFeature

        uint8_t  m_rcMode         = 0;

        bool m_brcInit              = true;   //!< BRC init flag
        bool m_brcReset             = false;  //!< BRC reset flag
        bool m_brcEnabled           = false;  //!< BRC enable flag
        bool m_lcuBrcEnabled        = false;  //!< LCU BRC enable flag
        bool m_hevcVDEncAcqpEnabled = false;  //!< ACQP enable flag
        bool m_vdencBrcEnabled      = false;  //!< Vdenc bitrate control enabled flag
        bool m_vdencHucUsed         = false;  //!< HUC usage flag
        bool m_fastPakEnable        = true;

        //Resources
        MHW_BATCH_BUFFER m_vdenc2ndLevelBatchBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};  //!< VDEnc 2nd level batch buffer
        MOS_RESOURCE     m_vdencOutputROIStreaminBuffer                                  = {};  //!< VDEnc Output ROI Streamin Buffer
        MOS_RESOURCE     m_vdencBrcDbgBuffer                                             = {};  //!< VDEnc brc debug buffer
        MOS_RESOURCE        m_resBrcDataBuffer                                           = {};  //!< Resource of bitrate control data buffer, only as an output of PAKintegrate Kernel
        HevcVdencBrcBuffers m_vdencBrcBuffers                                            = {};  //!< VDEnc Brc buffers
        uint16_t           *m_rdLambdaArray                                              = nullptr;
        uint16_t           *m_sadLambdaArray                                             = nullptr;

        MHW_VDBOX_NODE_IND m_vdboxIndex = MHW_VDBOX_NODE_1;
        uint32_t           m_currRecycledBufIdx = 0;

    MEDIA_CLASS_DEFINE_END(encode__HEVCEncodeBRC)
    };

}  // namespace encode

#endif  // !__ENCODE_HEVC_BRC_H__
