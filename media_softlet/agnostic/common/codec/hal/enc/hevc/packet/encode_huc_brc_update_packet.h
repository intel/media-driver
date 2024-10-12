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
//! \file     encode_huc_brc_update_packet.h
//! \brief    Defines the implementation of huc update packet 
//!

#ifndef __CODECHAL_HUC_BRC_UPDATE_PACKET_H__
#define __CODECHAL_HUC_BRC_UPDATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "encode_utils.h"
#include "encode_hevc_vdenc_pipeline.h"
#include "encode_hevc_basic_feature.h"
#if _ENCODE_RESERVED
#include "encode_huc_brc_update_packet_ext.h"
#endif // _ENCODE_RESERVED

namespace encode
{
    struct VdencHevcHucBrcUpdateDmem
    {
        uint32_t    TARGETSIZE_U32 = 0;
        uint32_t    FrameID_U32    = 0;  // frame number
        uint32_t    Ref_L0_FrameID_U32[8] = {};
        uint32_t    Ref_L1_FrameID_U32[8] = {};
        uint16_t    startGAdjFrame_U16[4] = {};  // 10, 50, 100, 150
        uint16_t    TargetSliceSize_U16    = 0;
        uint16_t    SLB_Data_SizeInBytes   = 0;
        uint16_t    PIC_STATE_StartInBytes = 0;  // PIC_STATE starts in byte. 0xFFFF means not available in SLB
        uint16_t    CMD2_StartInBytes      = 0;
        uint16_t    CMD1_StartInBytes      = 0;
        uint16_t    PIPE_MODE_SELECT_StartInBytes = 0;  // PIPE Mode select starts in byte. 0xFFFF means not available in SLB
        uint16_t    Current_Data_Offset           = 0;  // Data block offset of current picture from beginning of the data buffer (region 9)
        uint16_t    Ref_Data_Offset[5]            = {};  // Data block offset of ref pictures from beginning of the data buffer (region 9)
        uint16_t    MaxNumSliceAllowed_U16 = 0;
        uint8_t     OpMode_U8              = 0;     // 1: frame-based BRC (including ACQP), 2: Weighted prediction, Weighted prediction should not be enabled in first pass.
                                                    // Same as other common flags, this is a bit operation. Each bit is zero for disabling and 1 for enabling. i.e. 01: BRC, 10: WP - never used; 11: BRC+WP, 4: tile-based BRC (frame level), 8: tile-based BRC (tile level)
        uint8_t     CurrentFrameType_U8 = 0;
        uint8_t     Num_Ref_L0_U8       = 0;
        uint8_t     Num_Ref_L1_U8       = 0;
        uint8_t     Num_Slices          = 0;
        uint8_t     CQP_QPValue_U8      = 0;  // CQP QP value (needed for ICQ and ACQP)
        uint8_t     CQP_FracQP_U8       = 0;
        uint8_t     MaxNumPass_U8       = 0;  // max number of BRC passes (SAO second pass is not included.)
        uint8_t     gRateRatioThreshold_U8[7]        = {};
        uint8_t     startGAdjMult_U8[5]              = {};
        uint8_t     startGAdjDiv_U8[5]               = {};
        uint8_t     gRateRatioThresholdQP_U8[8]      = {};
        uint8_t     SceneChgPrevIntraPctThreshold_U8 = 0;
        uint8_t     SceneChgCurIntraPctThreshold_U8  = 0;
        uint8_t     IPAverageCoeff_U8                = 0;
        uint8_t     CurrentPass_U8                   = 0;
        int8_t      DeltaQPForMvZero_S8              = 0;
        int8_t      DeltaQPForMvZone0_S8             = 0;
        int8_t      DeltaQPForMvZone1_S8             = 0;
        int8_t      DeltaQPForMvZone2_S8             = 0;
        int8_t      DeltaQPForSadZone0_S8            = 0;
        int8_t      DeltaQPForSadZone1_S8            = 0;
        int8_t      DeltaQPForSadZone2_S8            = 0;
        int8_t      DeltaQPForSadZone3_S8            = 0;
        int8_t      DeltaQPForROI0_S8                = 0;
        int8_t      DeltaQPForROI1_S8                = 0;
        int8_t      DeltaQPForROI2_S8                = 0;
        int8_t      DeltaQPForROI3_S8                = 0;
        int8_t      LumaLog2WeightDenom_S8           = 0;  // default: 6
        int8_t      ChromaLog2WeightDenom_S8         = 0;  // default: 6
        uint8_t     DisabledFeature_U8               = 0;
        uint8_t     SlidingWindow_Enable_U8          = 0;  // 0-disabled, 1-enabled
        uint8_t     LOG_LCU_Size_U8                  = 0;  // 6
        uint16_t    NetworkTraceEntry_U16            = 0;  // default: 0
        uint16_t    LowDelaySceneChangeXFrameSize_U16 = 0;  // default: 0
        int8_t      ReEncodePositiveQPDeltaThr_S8     = 0;  // default: 4
        int8_t      ReEncodeNegativeQPDeltaThr_S8     = 0;  // default: -10

        // tile-based BRC
        uint8_t     MaxNumTileHuCCallMinus1 = 0;  // maximal tile row
        uint8_t     TileHucCallIndex        = 0;
        uint8_t     TileHuCCallPassIndex    = 0;  // Start from 1
        uint8_t     TileHuCCallPassMax      = 0;  // Reserved now
        uint16_t    TileSizeInLCU           = 0;

        uint32_t    TxSizeInBitsPerFrame = 0;

        uint8_t     StartTileIdx = 0;
        uint8_t     EndTileIdx   = 0;

        uint16_t    NumFrameSkipped = 0;
        uint32_t    SkipFrameSize   = 0;

        uint32_t    SliceHeaderSize = 0;
        uint8_t     IsLongTermRef   = 0;
        uint8_t     FrameSizeBoostForSceneChange = 0;  // UPD_TCBRC_SCENARIO_U8
        uint8_t     ROMCurrent                   = 0;  // ROM average of current frame
        uint8_t     ROMZero                      = 0;  // ROM zero percentage
        uint32_t    TargetFrameSize              = 0;  // TR_BRC
        uint32_t    TargetFulness                = 0;
        uint8_t     Delta                        = 0;
        uint8_t     CqmEnable                    = 0;
        uint8_t     UPD_TempCurrentlayer         = 0;
        uint8_t     UPD_TempScalable             = 0;
        uint32_t    UPD_UserMaxFrame             = 0;
        uint32_t    UPD_UserMaxFramePB           = 0;
        uint8_t     UPD_Randomaccess             = 0;
        uint8_t     UPD_AdaptiveTUEnabled        = 0;

        uint8_t     RSVD[42] = {};  // 64 bytes aligned
    };
    C_ASSERT(256 == sizeof(VdencHevcHucBrcUpdateDmem));

#define CODECHAL_VDENC_HEVC_MAX_SLICE_NUM   70

    struct VdencHevcHucBrcConstantData
    {
        uint16_t    VdencHevcHucBrcConstantData_0[52] = {};
        uint16_t    VdencHevcHucBrcConstantData_1[52] = {};
        uint16_t    VdencHevcHucBrcConstantData_2[52] = {};
        uint16_t    VdencHevcHucBrcConstantData_3[52] = {};
        uint16_t    VdencHevcHucBrcConstantData_4[52] = {};
        uint16_t    RSVD0[52];
        uint8_t     RSVD1[27];
        int8_t      RSVD2[243];
        int8_t      VdencHevcHucBrcConstantData_5[72] = {};
        int8_t      VdencHevcHucBrcConstantData_6[72] = {};
        int8_t      VdencHevcHucBrcConstantData_7[72] = {};
        uint8_t     RSVD3[54];
        int8_t      RSVD4[135];
        struct
        {
            uint8_t RSVD5[28];
        } VdencHevcHucBrcConstantData_8[52];
        struct
        {
            // Unit in Bytes
            uint16_t    SizeOfCMDs                     = 0;
            uint16_t    HcpWeightOffsetL0_StartInBytes = 0;         // HCP_WEIGHTOFFSET_L0 starts in bytes from beginning of the SLB. 0xFFFF means unavailable in SLB
            uint16_t    HcpWeightOffsetL1_StartInBytes = 0;         // HCP_WEIGHTOFFSET_L1 starts in bytes from beginning of the SLB. 0xFFFF means unavailable in SLB
            uint16_t    SliceState_StartInBytes        = 0;
            uint16_t    SliceHeaderPIO_StartInBytes    = 0;
            uint16_t    VdencWeightOffset_StartInBytes = 0;
            // Unit in Bits
            uint16_t    SliceHeader_SizeInBits         = 0;
            uint16_t    WeightTable_StartInBits        = 0;         // number of bits from beginning of slice header for weight table first bit, 0xffff means not awailable
            uint16_t    WeightTable_EndInBits          = 0;         // number of bits from beginning of slice header for weight table last bit, 0xffff means not awailable
        } Slice[CODECHAL_VDENC_HEVC_MAX_SLICE_NUM];

        uint8_t  RSVD6[61];
    };
#define  CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER 6

    class HucBrcUpdatePkt : public EncodeHucPkt, public mhw::vdbox::hcp::Itf::ParSetting
    {
    public:
        HucBrcUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) :
            EncodeHucPkt(pipeline, task, hwInterface)
        {
            m_featureManager = m_pipeline->GetPacketLevelFeatureManager(HevcPipeline::HucBrcUpdate);
            m_hcpItf         = hwInterface->GetHcpInterfaceNext();
            m_vdencItf       = hwInterface->GetVdencInterfaceNext();
            m_miItf          = hwInterface->GetMiInterfaceNext();
        }

        virtual ~HucBrcUpdatePkt() {}

        virtual MOS_STATUS Init() override;

        MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

        //!
        //! \brief  Calculate Command Size
        //!
        //! \param  [in, out] commandBufferSize
        //!         requested size
        //! \param  [in, out] requestedPatchListSize
        //!         requested size
        //! \return MOS_STATUS
        //!         status
        //!
        MOS_STATUS CalculateCommandSize(
            uint32_t &commandBufferSize,
            uint32_t &requestedPatchListSize) override;

        virtual MOS_STATUS DumpOutput() override;

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "BRCUPDATE_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
        }

    protected:
        MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);
        MHW_SETPAR_DECL_HDR(HCP_PIC_STATE);

        virtual MOS_STATUS AllocateResources() override;

        virtual MOS_STATUS ConstructBatchBufferHuCBRC(PMOS_RESOURCE batchBuffer);

        virtual MOS_STATUS ConstructGroup1Cmds();
        virtual MOS_STATUS ConstructGroup2Cmds();
        virtual MOS_STATUS ConstructGroup3Cmds();

        HevcBasicFeature *m_basicFeature = nullptr;  //!< Hevc Basic Feature used in each frame

        virtual MOS_STATUS SetExtDmemBuffer(VdencHevcHucBrcUpdateDmem *hucVdencBrcUpdateDmem) const;
        virtual MOS_STATUS SetCommonDmemBuffer(VdencHevcHucBrcUpdateDmem *hucVdencBrcUpdateDmem);
        virtual MOS_STATUS SetDmemBuffer() const;

        virtual MOS_STATUS SetConstLambdaHucBrcUpdate(void *params) const;
        virtual MOS_STATUS SetConstDataHuCBrcUpdate() const;

        MOS_STATUS SetTcbrcMode();

        uint32_t GetMaxAllowedSlices(uint8_t levelIdc) const;

        MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

        MOS_STATUS AddAllCmds_HCP_WEIGHTOFFSET_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_PAK_INSERT_OBJECT_SLICE(PMOS_COMMAND_BUFFER cmdBuffer) const;

#if USE_CODECHAL_DEBUG_TOOL
        MOS_STATUS DumpHucBrcUpdate(bool isInput);

        virtual MOS_STATUS DumpInput() override;
#endif
        uint32_t m_alignSize[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = {0};
        static constexpr uint32_t               m_vdboxHucHevcBrcUpdateKernelDescriptor = 9;//!< Huc HEVC Brc init kernel descriptor
        static constexpr uint32_t               m_vdboxHucHevcBrcLowdelayKernelDescriptor = 10;//!< Huc HEVC Brc low delay kernel descriptor

        // Batch Buffer for VDEnc
        MOS_RESOURCE                            m_vdencReadBatchBufferOrigin[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = {};  //!< VDEnc read batch buffer
        MOS_RESOURCE                            m_vdencReadBatchBufferTU7[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES]    = {};  //!< VDEnc read batch buffer
        MOS_RESOURCE                            m_vdencBrcConstDataBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};                        //!< VDEnc brc constant data buffer

        MOS_RESOURCE                            m_dataFromPicsBuffer = {}; //!< Data Buffer of Current and Reference Pictures for Weighted Prediction
        uint32_t                                m_vdenc2ndLevelBatchBufferSize[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = { 0 };
        MOS_RESOURCE                            m_vdencBrcUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM][VDENC_BRC_NUM_OF_PASSES] = { 0 };  //!< VDEnc BrcUpdate DMEM buffer

        mutable uint32_t                        m_1stPakInsertObjectCmdSize = 0;                   //!< Size of 1st PAK_INSERT_OBJ cmd
        mutable uint32_t                        m_hcpWeightOffsetStateCmdSize   = 0;               //!< Size of HCP_WEIGHT_OFFSET_STATE cmd
        uint32_t                                m_hcpSliceStateCmdSize = 0;                        //!< Size of HCP_SLICE_STATE cmd
        uint32_t                                m_vdencWeightOffsetStateCmdSize = 0;               //!< Size of VDENC_WEIGHT_OFFSET_STATE cmd
        uint32_t                                m_miBatchBufferEndCmdSize = 0;                     //!< Size of MI_BATCH_BUFFER_END cmd
        uint32_t                                m_cmd2StartInBytes = 0;
        uint32_t                                m_vdencBrcInitDmemBufferSize = 0;                  //!< Offset of BRC init DMEM buffer
        uint32_t                                m_vdencBrcUpdateDmemBufferSize = sizeof(VdencHevcHucBrcUpdateDmem);                //!< Offset of BRC update DMEM buffer
        uint32_t                                m_vdencBrcConstDataBufferSize = sizeof(VdencHevcHucBrcConstantData);                 //!< Offset of BRC const data buffer
        uint32_t                                m_slbDataSizeInBytes = 0;                          //!< Size of SLB Data
        uint8_t                                 m_tcbrcQualityBoost = 0;
        bool                                    m_bufConstSizeFlagForAdaptiveTU = false;           //!< VdencBatchBufferPerSliceConstSize flag for adaptiveTU

        MOS_RESOURCE m_vdencBrcInitDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {}; //!< VDEnc BrcInit DMEM buffer

        // Information for entry/slot in the picture data
        struct SlotInfo {
            uint32_t age;
            int32_t poc;
            bool isUsed;
            bool isRef;
        } slotInfo[CODECHAL_ENCODE_HEVC_VDENC_WP_DATA_BLOCK_NUMBER] = { { 0, 0, false, false } };

        uint8_t *m_batchbufferAddr = nullptr;
        int32_t m_curPicSlot = -1;        //!< Slot selected to store current Picutre data
        static constexpr uint32_t m_weightHistSize = 1024;                  //!< Weight Histogram (part of VDEnc Statistic): 256 DWs (16CLs) of Histogram Stats = 1024

        std::shared_ptr<mhw::vdbox::hcp::Itf>   m_hcpItf   = nullptr;
        std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;
        std::shared_ptr<mhw::mi::Itf>           m_miItf    = nullptr;
        std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__HucBrcUpdatePkt)
    };

}  // namespace encode
#endif
