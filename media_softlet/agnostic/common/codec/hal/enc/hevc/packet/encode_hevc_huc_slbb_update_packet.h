/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_hevc_huc_slbb_update_packet.h
//! \brief    Defines the implementation of HEVC HuC SLBB update packet
//!

#ifndef __ENCODE_HEVC_HUC_SLBB_UPDATE_PACKET_H__
#define __ENCODE_HEVC_HUC_SLBB_UPDATE_PACKET_H__

#include "encode_huc_slbb_update_pkt.h"
#include "encode_hevc_basic_feature.h"
#include "encode_hevc_vdenc_pipeline.h"
#include "media_hevc_feature_defs.h"

namespace encode
{
    //!
    //! \class HEVCHucSLBBUpdatePkt
    //! \brief HEVC HuC second level batch buffer update packet class
    //!
    class HEVCHucSLBBUpdatePkt : public HucSLBBUpdatePkt, public mhw::vdbox::hcp::Itf::ParSetting
    {
    public:
        //!
        //! \brief  Constructor
        //! \param  [in] pipeline
        //!         Pointer to media pipeline
        //! \param  [in] task
        //!         Pointer to media task
        //! \param  [in] hwInterface
        //!         Pointer to hardware interface
        //!
        HEVCHucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

        //!
        //! \brief  Destructor
        //!
        virtual ~HEVCHucSLBBUpdatePkt() = default;

        //!
        //! \brief  Initialize the HEVC HuC SLBB update packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Submit HEVC HuC SLBB update packet
        //! \param  [in] commandBuffer
        //!         Pointer to command buffer
        //! \param  [in] packetPhase
        //!         Packet phase flag
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

        //!
        //! \brief  Calculate Command Size for HEVC HuC SLBB update
        //! \param  [in, out] commandBufferSize
        //!         requested size
        //! \param  [in, out] requestedPatchListSize
        //!         requested patch list size
        //! \return MOS_STATUS
        //!         status
        //!
        virtual MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    protected:
        //!
        //! \brief  Allocate Resources for HEVC HuC SLBB update
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AllocateResources() override;

        //!
        //! \brief  Construct batch buffer for HEVC-specific SLBB updates
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS ConstructBatchBuffer() override;

        //!
        //! \brief  Set HUC_IMEM_STATE command parameters
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

        //!
        //! \brief  Returns true on extended platforms where the driver pre-fills
        //!         conflict-surface CMD2 fields and HuC must preserve them.
        //!         Base class returns false (base platforms). Subclasses override to return true.
        //!         Mirrors AV1 IsExtendedPlatform() at encode_av1_huc_slbb_update_packet.h.
        //!
        virtual bool IsExtendedPlatform() const { return false; }

        //!
        //! \brief  Set DMEM buffer for HuC HEVC SLBB Update
        //! \details Populates m_slbbUpdateDmemBuffer with HEVC-specific SLBB update parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SetDmem() const override;

#if USE_CODECHAL_DEBUG_TOOL
        virtual MOS_STATUS DumpInput() override;
        virtual MOS_STATUS DumpOutput() override;
#endif

        //!
        //! \brief  HuC HEVC SLBB Update DMEM structure
        //! \details Compact structure layout matching hevc_slbb_update.h specification (68 bytes total):
        //!          Section 1: SLBB Size (2 bytes)
        //!          Section 2: Group1 Command Offsets (6 bytes)
        //!          Section 3: Group2 Command Offsets (8 bytes)
        //!          Section 4: Group3 Command Offsets (12 bytes)
        //!          Section 5: Command Metadata (2 bytes)
        //!          Section 6: Reserved Fields (30 bytes)
        //!          Section 7: Encoding Parameters (8 bytes)
        //!
        struct HucHevcSlbbUpdateDmem
        {
                // Section 1: SLBB Size (2 bytes)
                uint16_t slbbSize;                          //!< Total size of second level batch buffer in bytes

                // Section 2: Group1 Command Offsets (6 bytes)
                uint16_t mfxWaitOffset;                     //!< Byte offset to first MFX_WAIT command in Group1
                uint16_t hcpPipeModeSelectOffset;           //!< Byte offset to HCP_PIPE_MODE_SELECT command in Group1
                uint16_t group1EndOffset;                   //!< Byte offset to end of Group1 commands

                // Section 3: Group2 Command Offsets (8 bytes)
                uint16_t vdencCmd1Offset;                   //!< Byte offset to VDENC_CMD1 command in Group2
                uint16_t hcpPicStateOffset;                 //!< Byte offset to HCP_PIC_STATE command in Group2
                uint16_t vdencCmd2Offset;                   //!< Byte offset to VDENC_CMD2 command in Group2
                uint16_t group2EndOffset;                   //!< Byte offset to end of Group2 commands

                // Section 4: Group3 Command Offsets (12 bytes)
                uint16_t hcpWeightOffsetStateOffset;        //!< Byte offset to HCP_WEIGHTOFFSET_STATE command in Group3
                uint16_t hcpSliceStateOffset;               //!< Byte offset to HCP_SLICE_STATE command in Group3
                uint16_t hcpPakInsertObjectOffset;          //!< Byte offset to HCP_PAK_INSERT_OBJECT command in Group3
                uint16_t vdencWeightsOffsetsStateOffset;    //!< Byte offset to VDENC_WEIGHTSOFFSETS_STATE command in Group3
                uint16_t vdencHevcVp9TileSliceStateOffset;  //!< Byte offset to VDENC_HEVC_VP9_TILE_SLICE_STATE command in Group3
                uint16_t group3EndOffset;                   //!< Byte offset to end of Group3 commands

                // Section 5: Command Metadata (2 bytes)
                uint16_t tileNum;                           //!< Total number of tiles in the frame

                // Section 6: Reserved + CMD2 Parameters (28 bytes)
                uint16_t reserved[4];                       //!< Reserved for future expansion
                uint8_t  Wa_16025947269;                    //!< Wa_16025947269: gate the protected VDENC_CMD2 DW52 bit
                                                            //!< (TU1/TU2 only). 1 = WA active, HuC keeps TU-table value;
                                                            //!< 0 = HW fix in place, HuC forces field to 0.
                uint8_t  reservedPad;                       //!< Pad to preserve DMEM size and DWORD alignment
                uint8_t  seqLowDelayMode;                   //!< Sequence-level LowDelayMode (m_hevcSeqParams->LowDelayMode)
                uint8_t  roundingFlags;                     //!< bit0: EnableCustomRoundingIntra, bit1: EnableCustomRoundingInter, bit2: HierarchicalFlag
                uint8_t  customRoundingOffsetIntra;         //!< CustomRoundingOffsetsParams.RoundingOffsetIntra (valid when roundingFlags bit0=1)
                uint8_t  customRoundingOffsetInter;         //!< CustomRoundingOffsetsParams.RoundingOffsetInter (valid when roundingFlags bit1=1)
                uint8_t  rdoqEnabled;                       //!< m_hevcRdoqEnabled flag
                uint8_t  ppsCurrPicRefEnabled;              //!< pps_curr_pic_ref_enabled_flag
                uint8_t  paletteModeEnabled;                //!< palette_mode_enabled_flag
                uint8_t  chromaFormatIdc;                   //!< chroma_format_idc (0/1/2/3)
                uint8_t  roundingPrecisionEnabled;          //!< m_hevcVdencRoundingPrecisionEnabled
                uint8_t  numRefL0;                          //!< num_ref_idx_l0_active
                uint8_t  numRefL1;                          //!< num_ref_idx_l1_active
                uint8_t  bitDepthLumaMinus8;                //!< bit_depth_luma_minus8

                uint8_t  sliceQpDelta;                      //!< int8_t cast: slice_qp_delta (signed, cast to uint8_t for transport)
                uint8_t  AdaptiveTUEnabled;                 //!< AdaptiveTU mode flag: 0=disabled, 1=enabled (process TU7 SLBB via Region 2->3)
                uint8_t  lookAheadPhase;                    //!< 1 = LA pass: HuC keeps Region 0 (driver wrote LA values)
                uint8_t  ExtendedPlatform;                  //!< 0 = base platform: HuC writes the vendor-specific CMD2 bit from TU table.
                                                            //!< 1 = extended platform: HuC preserves driver pre-fill.
                                                            //!< Mirrors AV1 ExtendedPlatform semantics.
                uint8_t  numROI;                            //!< ROI flag: 1 if NumROI > 0, else 0
                uint8_t  rateControlMethod;                 //!< Rate control method (from SeqParams)

                // Section 7: Encoding Parameters (8 bytes)
                uint8_t codingType;                         //!< Frame coding type (I/P/B/B1/B2)
                uint8_t qpValue;                            //!< Quantization parameter value (0-51)
                uint8_t lowDelayMode;                       //!< Low delay encoding mode flag (0/1)
                uint8_t targetUsage;                        //!< Performance/quality tradeoff (1-7)
                uint16_t bGopSize;                          //!< Group of pictures size
                uint8_t depth;                              //!< Encoding depth/hierarchy level
                uint8_t  scenarioInfo;                      //!< Encoding scenario (from SeqParams)
        };

        //!
        //! \brief  Set HUC_VIRTUAL_ADDR_STATE command parameters
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

        //!
        //! \brief  Construct Group 1 commands for batch buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ConstructGroup1Cmds();

        //!
        //! \brief  Construct Group 2 commands for batch buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ConstructGroup2Cmds();

        //!
        //! \brief  Construct Group 3 commands for batch buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ConstructGroup3Cmds();

        //!
        //! \brief  HEVC HuC SLBB Update kernel descriptor
        //!
        static constexpr uint32_t m_vdboxHucHevcSlbbUpdateKernelDescriptor = 23;

        //!
        //! \brief  Pointer to HEVC basic feature
        //!
        HevcBasicFeature *m_hevcBasicFeature = nullptr;

        //!
        //! \brief  Pointer to feature manager
        //!
        std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

        //!
        //! \brief  HEVC-specific SLBB buffer resources for tile operations
        //!
        PMOS_RESOURCE m_hevcTileSlbbBuffer = nullptr;

        //!
        //! \brief  HEVC CTB parameters buffer for batch buffer operations
        //!
        PMOS_RESOURCE m_hevcCtbParamsBuffer = nullptr;

        //!
        //! \brief  Number of tiles in current frame
        //!
        uint32_t m_numTiles = 1;

        //!
        //! \brief  Current CTB size for HEVC encoding
        //!
        uint32_t m_ctbSize = 64;

        //!
        //! \brief  WPP (Wavefront Parallel Processing) mode flag
        //!
        bool m_wppMode = false;

        //!
        //! \brief  SCC (Screen Content Coding) mode flag
        //!
        bool m_sccMode = false;

        //!
        //! \brief  Pointer to batch buffer address
        //!
        uint8_t *m_batchbufferAddr = nullptr;

        //!
        //! \brief  Size of MI_BATCH_BUFFER_END command
        //!
        uint32_t m_miBatchBufferEndCmdSize = 0;

        //!
        //! \brief  Size of HCP_SLICE_STATE command
        //!
        uint32_t m_hcpSliceStateCmdSize = 0;

        //!
        //! \brief  Size of HCP_WEIGHTOFFSET_STATE command (single L0 or L1)
        //!
        uint32_t m_hcpWeightOffsetStateCmdSize = 0;

        //!
        //! \brief  Size of VDENC_WEIGHT_OFFSET_STATE command
        //!
        uint32_t m_vdencWeightOffsetStateCmdSize = 0;

        //!
        //! \brief  Start offset for command 2
        //!
        uint32_t m_cmd2StartInBytes = 0;

        //!
        //! \brief  Actual end offset of Group1 commands in the SLBB (recorded during ConstructGroup1Cmds)
        //!
        uint32_t m_group1EndOffset = 0;

        //!
        //! \brief  Actual end offset of Group2 commands in the SLBB (recorded during ConstructGroup2Cmds)
        //!
        uint32_t m_group2EndOffset = 0;

        //!
        //! \brief  Actual byte offset of VDENC_WEIGHTSOFFSETS_STATE in SLBB (recorded during ConstructGroup3Cmds)
        //!
        uint32_t m_weightsOffsetsStateOffsetInBytes = 0;

        //!
        //! \brief  Actual byte offset of VDENC_HEVC_VP9_TILE_SLICE_STATE in SLBB (recorded during ConstructGroup3Cmds)
        //!
        uint32_t m_tileSliceStateOffsetInBytes = 0;

        //!
        //! \brief  Size of SLB data
        //!
        uint32_t m_slbDataSizeInBytes = 0;

        //!
        //! \brief  Alignment sizes per slice
        //!
        uint32_t m_alignSize[ENCODE_HEVC_VDENC_NUM_MAX_SLICES] = {0};

        //!
        //! \brief  HCP interface for batch buffer construction
        //!
        std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpItf = nullptr;

        //!
        //! \brief  VDENC interface for batch buffer construction
        //!
        std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;

        //!
        //! \brief  MI interface for batch buffer construction
        //!
        std::shared_ptr<mhw::mi::Itf> m_miItf = nullptr;

    MEDIA_CLASS_DEFINE_END(encode__HEVCHucSLBBUpdatePkt)
    };
}

#endif // __ENCODE_HEVC_HUC_SLBB_UPDATE_PACKET_H__