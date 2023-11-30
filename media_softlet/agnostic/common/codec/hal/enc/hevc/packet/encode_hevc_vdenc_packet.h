/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     encode_hevc_vdenc_packet.h
//! \brief    Defines the interface to adapt to hevc vdenc encode pipeline
//!

#ifndef __CODECHAL_HEVC_VDENC_PACKET_H__
#define __CODECHAL_HEVC_VDENC_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_utils.h"
#include "encode_hevc_vdenc_pipeline.h"
#include "encode_hevc_basic_feature.h"
#include "encode_status_report.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#if _ENCODE_RESERVED
#include "encode_hevc_vdenc_par_dump.h"
#endif  // _ENCODE_RESERVED

namespace encode
{
    class HevcVdencPkt : public CmdPacket, public MediaStatusReportObserver, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
    {
        //!
        //! \struct AtomicScratchBuffer
        //! \brief  The sturct of Atomic Scratch Buffer
        //!
        struct AtomicScratchBuffer
        {
            PMOS_RESOURCE resAtomicScratchBuffer;  //!> Handle of eStatus buffer
            uint32_t *    pData;                   //!> Pointer of the buffer of actual data
            uint16_t      encodeUpdateIndex;       //!> used for VDBOX update encode status
            uint16_t      tearDownIndex;           //!> Reserved for future extension
            uint32_t      zeroValueOffset;         //!> Store the result of the ATOMIC_CMP
            uint32_t      operand1Offset;          //!> Operand 1 of the ATOMIC_CMP
            uint32_t      operand2Offset;          //!> Operand 2 of the ATOMIC_CMP
            uint32_t      operand3Offset;          //!> Copy of the operand 1
            uint32_t      size;                    //!> Size of the buffer
            uint32_t      operandSetSize;          //!> Size of Operand set
        };

#define CODECHAL_ENCODE_RECYCLED_BUFFER_NUM 6
#define CODECHAL_PAGE_SIZE 0x1000

        //!
        //! \struct   CodechalEncodeHevcPakStatesBuffer
        //! \brief    Codechal encode HEVC PAK States buffer
        //!
        struct CodechalEncodeHevcPakStatesBuffer
        {
            uint32_t hcpBitstreamByteCountFrame;
            uint32_t hcpBitstreamByteCountFrameNoheader;
            uint32_t hcpImageStatusControl;
            uint32_t reserved0;
            uint32_t hcpImageStatusControlForLastPass;
            uint32_t reserved1[3];
        };

    public:
        enum SubmitState
        {
            submitFrameByDefault = 0,
            submitPic,
            submitTile,
            submitInvalid
        };
        enum FlushCmd
        {
            waitHevc = 0,
            waitVdenc,
            waitHevcVdenc
        };
        HevcVdencPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : CmdPacket(task),
                                                                                                            m_pipeline(dynamic_cast<HevcVdencPipeline *>(pipeline)),
                                                                                                            m_hwInterface(dynamic_cast<CodechalHwInterfaceNext *>(hwInterface))
        {
            ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);

            m_osInterface    = hwInterface->GetOsInterface();
            m_statusReport   = m_pipeline->GetStatusReportInstance();
            m_featureManager = m_pipeline->GetPacketLevelFeatureManager(HevcPipeline::hevcVdencPacket);
            m_encodecp       = m_pipeline->GetEncodeCp();
            m_vdencItf       = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
            m_hcpItf         = hwInterface->GetHcpInterfaceNext();
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpItf);
            m_miItf          = m_hwInterface->GetMiInterfaceNext();
            ENCODE_CHK_NULL_NO_STATUS_RETURN(m_miItf);
        }

        virtual ~HevcVdencPkt() 
        {
            FreeResources();
        }

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \param  [in] params
        //!         Pointer to the input parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare() override;

        //!
        //! \brief  Destroy the media packet and release the resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Destroy() override;

        //!
        //! \brief  Add the command sequence into the commandBuffer and
        //!         and return to the caller task
        //! \param  [in] commandBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Submit(
            MOS_COMMAND_BUFFER* commandBuffer,
            uint8_t packetPhase = otherPacket) override;

        //!
        //! \brief  One frame is completed
        //! \param  [in] mfxStatus
        //!         pointer to status buffer which for MFX
        //! \param  [in] rcsStatus
        //!         pointer to status buffer which for RCS
        //! \param  [in, out] statusReport
        //!         pointer of EncoderStatusReport
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

        //!
        //! \brief    Calculate picture state command size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculatePictureStateCommandSize();

        virtual MOS_STATUS SendHwSliceEncodeCommand(const PCODEC_ENCODER_SLCDATA slcData, const uint32_t currSlcIdx, MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief    get vdenc state command size and patch size
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS GetVdencStateCommandsDataSize(uint32_t &vdencPictureStatesSize, uint32_t &vdencPicturePatchListSize);

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

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "VDENC_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
        }

        MOS_STATUS SetSubmitState(SubmitState state)
        {
            if (state >= submitFrameByDefault && state < submitInvalid)
            {
                m_submitState = state;
                return MOS_STATUS_SUCCESS;
            }
            else
            {
                return MOS_STATUS_UNINITIALIZED;
            }
        }

        MOS_STATUS PrepareHWMetaData(MOS_COMMAND_BUFFER *cmdBuffer);

    protected:
#if USE_CODECHAL_DEBUG_TOOL
        //!
        //! \brief  Dump input resources or infomation before submit
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS DumpInput();
#endif
        //!
        //! \brief    get  SliceStatesSize and SlicePatchListSize,
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS GetHxxPrimitiveCommandSize();

        //!
        //! \brief  Calculate Command Buffer Size
        //!
        //! \return uint32_t
        //!         Command buffer size calculated
        //!
        virtual uint32_t CalculateCommandBufferSize();

        //!
        //! \brief  Calculate Patch List Size
        //!
        //! \return uint32_t
        //!         Patchlist size calculated
        //!
        virtual uint32_t CalculatePatchListSize();

        //!
        //! \brief  Submit slice level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] packetPhase
        //!         Packet phase
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SubmitPictureLevel(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase);

        //!
        //! \brief  Submit tile level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] packetPhase
        //!         Packet phase
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SubmitTileLevel(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase);

        //!
        //! \brief  Patch slice level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] packetPhase
        //!         Packet phase
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);

        //!
        //! \brief  Patch tile level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] packetPhase
        //!         Packet phase
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase) ;

        //!
        //! \brief  Add one tile commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \param  [in] tileRow
        //!         tile row
        //! \param  [in] tileCol
        //!         tile column
        //! \param  [in] tileRowPass
        //!         tile row pass
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AddOneTileCommands(
            MOS_COMMAND_BUFFER  &cmdBuffer,
            uint32_t            tileRow,
            uint32_t            tileCol,
            uint32_t            tileRowPass);

        //!
        //! \brief  Add slice commands in tile
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddSlicesCommandsInTile(
            MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Update params
        //! \return void
        //!         No return value
        //!
        void UpdateParameters();

        virtual MOS_STATUS AddPicStateWithNoTile(
            MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Add picture state with tile
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddPicStateWithTile(
            MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Add HCP picture level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddPictureHcpCommands(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Add VDENC picture level commands
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Ensure all commands have been executed
        //! \param  [in] packetPhase
        //!         Packet phase
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer);

        //!
        //! \brief  Ensure all commands have been executed
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS InsertSeqStreamEnd(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief  Ensure all commands have been executed
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS AddHcpPipeModeSelect(
            MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS UpdateStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

        //!
        //! \brief  Construct 3rd level batch buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Construct3rdLevelBatch();

        virtual MOS_STATUS AllocateResources();

        //!
        //! \brief  Add conditional batch buffer end command
        //! \param  [in] cmdBuffer
        //!         Pointer to the command buffer which is allocated by caller
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS AddCondBBEndForLastPass(MOS_COMMAND_BUFFER &cmdBuffer);

        virtual MOS_STATUS StartStatusReport(
            uint32_t            srType,
            MOS_COMMAND_BUFFER *cmdBuffer) override;

        virtual MOS_STATUS EndStatusReport(
            uint32_t            srType,
            MOS_COMMAND_BUFFER *cmdBuffer) override;

        //!
        //! \brief    Add command to read the HCP status
        //!
        //! \param    [in] vdboxIndex
        //!           Index of vdbox
        //! \param    [in] statusReport
        //!           Encode status report
        //! \param    [in, out] cmdBuffer
        //!           Command buffer
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ReadHcpStatus(
            MHW_VDBOX_NODE_IND  vdboxIndex,
            MediaStatusReport * statusReport,
            MOS_COMMAND_BUFFER &cmdBuffer);

        void SetPakPassType();

        MOS_STATUS ReadSliceSizeForSinglePipe(MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS ReadSliceSize(MOS_COMMAND_BUFFER &cmdBuffer);

        // Inline functions
        MOS_STATUS ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex);

        void SetPerfTag();

        MOS_STATUS SetSemaphoreMem(
            MOS_RESOURCE &      semaphoreMem,
            uint32_t            value,
            MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS SendPrologCmds(
            MOS_COMMAND_BUFFER &cmdBuffer);

        MOS_STATUS AllocateBatchBufferForPakSlices(
            uint32_t numSlices,
            uint16_t numPakPasses);

        MOS_STATUS SetBatchBufferForPakSlices();

        virtual MOS_STATUS ReadExtStatistics(MOS_COMMAND_BUFFER &cmdBuffer);

        //!
        //! \brief    Retreive BRC Pak statistics
        //!
        //! \param    [in] cmdBuffer
        //!           Pointer to command buffer
        //! \param    [in] params
        //!           BRC pak statistics parameters
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS ReadBrcPakStatistics(
            PMOS_COMMAND_BUFFER          cmdBuffer,
            EncodeReadBrcPakStatsParams *params);

        virtual MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);

        //! \brief    Report extended statistics
        //!
        //! \param    [in] encodeStatusMfx
        //!           Reference to encoder status for vdbox
        //! \param    [in, out] statusReportData
        //!           Reference to encoder status report data
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
       virtual MOS_STATUS ReportExtStatistics(
            EncodeStatusMfx        &encodeStatusMfx,
            EncodeStatusReportData &statusReportData);

        //! \brief    Set Rowstore Cache offset
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SetRowstoreCachingOffsets();

        //!
        //! \brief  Free resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS FreeResources();

        MHW_SETPAR_DECL_HDR(VDENC_CONTROL_STATE);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);

        MHW_SETPAR_DECL_HDR(HCP_TILE_CODING);

        MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);

        MHW_SETPAR_DECL_HDR(HCP_SLICE_STATE);

        MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);

        MOS_STATUS AddAllCmds_HCP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_PAK_INSERT_OBJECT_BRC(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_REF_IDX_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_FQM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        MOS_STATUS AddAllCmds_HCP_WEIGHTOFFSET_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

        // 3rd Level Batch buffer
        AtomicScratchBuffer         m_atomicScratchBuf = {};               //!< Stores atomic operands and result

        MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

#if USE_CODECHAL_DEBUG_TOOL
        //! \brief    Dump the output resources in status report callback function
        //!
        //! \param    [in] encodeStatusMfx
        //!           Pointer to encoder status for vdbox
        //! \param    [in] statusReportData
        //!           Pointer to encoder status report datas
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpResources(
            EncodeStatusMfx *       encodeStatusMfx,
            EncodeStatusReportData *statusReportData);
#endif
        HevcVdencPipeline *m_pipeline = nullptr;

        // Interfaces
        EncodeAllocator *         m_allocator         = nullptr;
        CodechalHwInterfaceNext *     m_hwInterface       = nullptr;
        HevcBasicFeature *        m_basicFeature      = nullptr;  //!< Encode parameters used in each frame
        EncodeMemComp *           m_mmcState          = nullptr;
        EncodeCp *                m_encodecp          = nullptr;
        PacketUtilities *         m_packetUtilities   = nullptr;

        SubmitState m_submitState = submitFrameByDefault;

        std::shared_ptr<mhw::vdbox::vdenc::Itf>           m_vdencItf       = nullptr;
        std::shared_ptr<mhw::vdbox::hcp::Itf>             m_hcpItf         = nullptr;
        std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

        mutable uint8_t m_curHcpSurfStateId = 0;

        // Parameters passed from application
        const CODEC_HEVC_ENCODE_PICTURE_PARAMS * m_hevcPicParams      = nullptr;  //!< Pointer to picture parameter
        const CODEC_HEVC_ENCODE_SEQUENCE_PARAMS *m_hevcSeqParams      = nullptr;  //!< Pointer to sequence parameter
        const CODEC_HEVC_ENCODE_SLICE_PARAMS *   m_hevcSliceParams    = nullptr;  //!< Pointer to slice parameter
        const CODECHAL_HEVC_IQ_MATRIX_PARAMS *   m_hevcIqMatrixParams = nullptr;  //!< Pointer to IQ matrix parameter
        const PCODECHAL_NAL_UNIT_PARAMS *        m_nalUnitParams      = nullptr;  //!< Pointer to NAL unit parameters

        bool m_pakOnlyPass     = false;
        bool m_streamInEnabled = false;  //!< Vdenc stream in enabled flag

        uint8_t m_currRecycledBufIdx = 0;  //!< Current recycled buffer index

        uint32_t      m_mvOffset                            = 0;        //!< MV data offset, in 64 byte
        uint32_t      m_mbCodeSize                          = 0;        //!< MB code buffer size
        PMOS_RESOURCE m_resMetadataLineBuffer               = nullptr;  //!< Metadata line data buffer
        PMOS_RESOURCE m_resMetadataTileLineBuffer           = nullptr;  //!< Metadata tile line data buffer
        PMOS_RESOURCE m_resMetadataTileColumnBuffer         = nullptr;  //!< Metadata tile column data buffer
        PMOS_RESOURCE m_resLCUIldbStreamOutBuffer           = nullptr;  //!< LCU ILDB stream-out buffer
        PMOS_RESOURCE m_resSSESrcPixelRowStoreBuffer        = nullptr;  //!< SSE Src pixel row store buffer
        PMOS_RESOURCE m_resCumulativeCuCountStreamoutBuffer = nullptr;  //!< Cumulative CU count stream out buffer
        PMOS_RESOURCE m_vdencTileRowStoreBuffer             = nullptr;  //!< Tile row store buffer
        PMOS_RESOURCE m_resPakcuLevelStreamOutData          = nullptr;  //!< PAK LCU level stream out data buffer

        MHW_VDBOX_NODE_IND m_vdboxIndex = MHW_VDBOX_NODE_1;  //!< Index of VDBOX

        uint32_t m_sliceStatesSize = 0;  //!< Slice states size

        bool m_lastTaskInPhase  = false;  //!< last task in phase flag

        bool m_useBatchBufferForPakSlices = false;

        int32_t  m_batchBufferForPakSlicesStartOffset    = 0;
        uint32_t m_sizeOfSseSrcPixelRowStoreBufferPerLcu = 0;  //!< Size of SSE row store buffer per LCU

        MHW_BATCH_BUFFER m_batchBufferForPakSlices[HevcBasicFeature::m_codecHalHevcNumPakSliceBatchBuffers] = {};  //!< Batch buffer for pak slice commands

        PMOS_RESOURCE m_vdencIntraRowStoreScratch = nullptr;

        bool     m_usePatchList                = 0;  //!< Use Ptach List or not
        uint32_t m_defaultPictureStatesSize    = 0;  //!< Picture state command size
        uint32_t m_pictureStatesSize           = 0;  //!< Picture states size
        uint32_t m_defaultSliceStatesSize      = 0;  //!< Slice state command size
        uint32_t m_defaultPicturePatchListSize = 0;  //!< Picture state patch list size
        uint32_t m_picturePatchListSize        = 0;  //!< Picture patch list size
        uint32_t m_defaultSlicePatchListSize   = 0;  //!< Slice state patch list size
        uint32_t m_slicePatchListSize          = 0;  //!< Slice patch list size

        bool m_vdencPakObjCmdStreamOutForceEnabled = false;

        bool m_lastSliceInTile = false;

        FlushCmd m_flushCmd = waitHevc;

#if USE_CODECHAL_DEBUG_TOOL && _ENCODE_RESERVED
        std::shared_ptr<HevcVdencParDump> m_hevcParDump = nullptr;
#endif  // _ENCODE_RESERVED

        bool m_enableVdencStatusReport = false;

    MEDIA_CLASS_DEFINE_END(encode__HevcVdencPkt)
    };

}

#endif
