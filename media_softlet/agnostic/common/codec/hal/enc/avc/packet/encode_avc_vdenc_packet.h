/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     encode_avc_vdenc_packet.h
//! \brief    Defines the interface to adapt to AVC VDENC pipeline
//!

#ifndef __CODECHAL_AVC_VDENC_PACKET_H__
#define __CODECHAL_AVC_VDENC_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_avc_vdenc_pipeline.h"
#include "encode_avc_basic_feature.h"
#include "encode_status_report.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_mfx_itf.h"
#include "mhw_mi_itf.h"

namespace encode
{

class AvcVdencPkt : public CmdPacket, 
                    public MediaStatusReportObserver, 
                    public mhw::vdbox::vdenc::Itf::ParSetting,
                    public mhw::vdbox::mfx::Itf::ParSetting, 
                    public mhw::mi::Itf::ParSetting
{
public:

    AvcVdencPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

    virtual ~AvcVdencPkt();

    void fill_pad_with_value(PMOS_SURFACE psSurface, uint32_t real_height, uint32_t aligned_height) const;

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
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

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
    //! \brief  Calculate Command Size
    //!
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         status
    //!
    virtual MOS_STATUS CalculateCommandSize(
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

protected:
    MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER  &cmdBuffer);
    MOS_STATUS PatchSliceLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);
    MOS_STATUS InsertSeqStreamEnd(MOS_COMMAND_BUFFER &cmdBuffer);

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
    //! \brief    Calculate Vdenc Commands Size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS CalculateVdencCommandsSize();

    //!
    //! \brief    Calculate Mfx Commands Size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS CalculateMfxCommandsSize();

    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isModeSpecific);

    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isShortFormat);

    MmioRegistersMfx *SelectVdboxAndGetMmioRegister(
        MHW_VDBOX_NODE_IND  index,
        PMOS_COMMAND_BUFFER pCmdBuffer);

    virtual MOS_STATUS StartStatusReport(
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    virtual MOS_STATUS EndStatusReport(
        uint32_t srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    //!
    //! \brief  Ensure all commands have been executed
    //! \param  [in] cmdBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Add command to read Mfc status
    //!
    //! \param    [in, out] cmdBuffer
    //!           Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadMfcStatus(MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Read Image status for status report
    //! \param    vdboxIndex
    //!           [in] the vdbox index
    //! \param    params
    //!           [in] the parameters for Image status read
    //! \param    cmdBuffer
    //!           [in, out] the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadImageStatus(
        const EncodeStatusReadParams &params,
        PMOS_COMMAND_BUFFER           cmdBuffer);

    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Add picture-level MFX commands to command buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddPictureMfxCommands(MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Add picture-level VDEnc commands to command buffer
    //!
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer);

    void UpdateParameters();

    // Inline functions
    MOS_STATUS ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex);

    void SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType);

    MOS_STATUS SendPrologCmds(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Allocate Batch Buffer For PakSlice.
    //!
    //! \param    [in] numSlices
    //!           Number of Slice
    //! \param    [in] numPakPasses
    //!           Number of PAK pass.
    //! \param    [in] currRecycledBufIdx
    //!           Current Recycle Buffer Index.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateBatchBufferForPakSlices(
        uint32_t numSlices,
        uint16_t numPakPasses,
        uint8_t currRecycledBufIdx);

    //!
    //! \brief    Release Batch Buffer For PakSlice.
    //!
    //! \param    [in] currRecycledBufIdx
    //!           Current Recycle Buffer Index.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ReleaseBatchBufferForPakSlices(uint8_t currRecycledBufIdx);

    //!
    //! \brief    Build slices with header insertion
    //! \param    [in] cmdBuffer
    //!           command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendSlice(PMOS_COMMAND_BUFFER cmdBuffer);

    MOS_STATUS SetSliceStateCommonParams(MHW_VDBOX_AVC_SLICE_STATE &sliceState);

    //!
    //! \brief    Use to pack slice header related params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PackSliceHeader(uint16_t slcCount);

    MOS_STATUS SetSliceStateParams(
        MHW_VDBOX_AVC_SLICE_STATE   &sliceState,
        PCODEC_ENCODER_SLCDATA      slcData,
        uint16_t                    slcCount);

    //!
    //! \brief    Prepare HW MetaData buffer
    //! \details  Prepare HW MetaData buffer.
    //! \param    [in] cmdBuffer
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS PrepareHWMetaData(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief      Store number passes
    //!
    //! \param      [in, out] cmdBuffer
    //!             Command buffer
    //!
    //! \return     MOS_STATUS
    //!             MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreNumPasses(MOS_COMMAND_BUFFER &cmdBuffer);

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
    MOS_STATUS FreeResources();

    //! \brief    Update and lock BatchBufferForPakSlices
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS LockBatchBufferForPakSlices();

    //! \brief    Unlock BatchBufferForPakSlices
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UnlockBatchBufferForPakSlices();

    //!
    //! \brief    Report Slice Size to MetaData buffer
    //! \details  Report Slice Size to MetaData buffer.
    //! \param    [in] cmdBuffer
    //!               Pointer to primary cmd buffer
    //!           [in] slcCount
    //!               Current slice count
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReportSliceSizeMetaData(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            slcCount);

    //! \brief    Get AVC VDenc frame level status extention
    //!
    //! \param    [in] cmdBuffer
    //!           Point to MOS_COMMAND_BUFFER
    //!           [in] StatusReportFeedbackNumber
    //!           Status report number
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetAvcVdencFrameLevelStatusExt(uint32_t StatusReportFeedbackNumber, MOS_COMMAND_BUFFER *cmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    };

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
        EncodeStatusReportData &statusReportData)
    {
        return MOS_STATUS_SUCCESS;
    };

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    MOS_STATUS AddAllCmds_MFX_QM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFX_FQM_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFX_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFX_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFX_AVC_REF_IDX_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MOS_STATUS AddAllCmds_MFX_AVC_WEIGHTOFFSET_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    MHW_SETPAR_DECL_HDR(MFX_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(MFX_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(MFX_BSP_BUF_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(MFX_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(MI_CONDITIONAL_BATCH_BUFFER_END);

    MHW_SETPAR_DECL_HDR(MI_STORE_REGISTER_MEM);

    MHW_SETPAR_DECL_HDR(MI_STORE_DATA_IMM);

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the output resources in status report callback function
    //!
    //! \param    [in] encodeStatusMfx
    //!           Pointer to encoder status for vdbox
    //! \param    [in] statusReportData
    //!           Pointer to encoder status report data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpResources(
        EncodeStatusMfx *       encodeStatusMfx,
        EncodeStatusReportData *statusReportData);

    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer) { return MOS_STATUS_SUCCESS; }

    virtual MOS_STATUS PopulateEncParam(uint8_t meMethod, void *cmd) { return MOS_STATUS_SUCCESS; }

    virtual MOS_STATUS DumpEncodeImgStats(
        PMOS_COMMAND_BUFFER cmdbuffer);
#endif

    AvcVdencPipeline         *m_pipeline        = nullptr;

    // Interfaces
    EncodeAllocator          *m_allocator       = nullptr;
    CodechalHwInterfaceNext  *m_hwInterface     = nullptr;
    AvcBasicFeature          *m_basicFeature    = nullptr;
    EncodeMemComp            *m_mmcState        = nullptr;
    EncodeCp                 *m_encodecp        = nullptr;

    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;
    std::shared_ptr<mhw::vdbox::mfx::Itf>   m_mfxItf   = nullptr;

    std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

    MediaFeatureManager *m_legacyFeatureManager = nullptr;

    mutable uint8_t m_curMfxSurfStateId              = 0;
    PMOS_RESOURCE   m_pResource                      = nullptr;
    uint32_t        m_dwOffset                       = 0;
    uint32_t        m_dwValue                        = 0;

    // Parameters passed from application
    CODEC_AVC_ENCODE_PIC_PARAMS *      m_picParam    = nullptr;  //!< Pointer to picture parameter
    CODEC_AVC_ENCODE_SEQUENCE_PARAMS * m_seqParam    = nullptr;  //!< Pointer to sequence parameter
    CODEC_AVC_ENCODE_SLICE_PARAMS *    m_sliceParams = nullptr;  //!< Pointer to slice parameter

    MHW_VDBOX_NODE_IND     m_vdboxIndex = MHW_VDBOX_NODE_1;     //!< Index of VDBOX

    // CMD buffer sizes
    uint32_t               m_pictureStatesSize = 0;             //!< Picture states size
    uint32_t               m_sliceStatesSize = 0;               //!< Slice states size

    // Patch List
    bool                   m_usePatchList = 0;                  //!< Use Ptach List or not
    uint32_t               m_picturePatchListSize = 0;          //!< Picture patch list size
    uint32_t               m_slicePatchListSize = 0;            //!< Slice patch list size

    // Batch Buffers
    MHW_BATCH_BUFFER       m_batchBufferForVdencImgStat[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {}; //!< VDEnc image state batch buffers

    // PAK Scratch Buffers
    MHW_BATCH_BUFFER       m_batchBufferForPakSlices[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};  //!< PAK Slice batch buffers
    bool                   m_useBatchBufferForPakSlices = false;                   //!< use PAK Slice batch buffers
    uint32_t               m_pakSliceSize = 0;                                     //!< PAK Slice Size
    uint32_t               m_pakSlicePatchListSize = 0;                            //!< PAK Slice patch list size

    PMOS_RESOURCE          m_resDeblockingFilterRowStoreScratchBuffer = nullptr;   //!< Handle of De-block row store surface
    PMOS_RESOURCE          m_intraRowStoreScratchBuffer = nullptr;                 //!< Handle of intra row store surface
    PMOS_RESOURCE          m_vdencIntraRowStoreScratch = nullptr;                  //!< Handle of intra row store surface
    PMOS_RESOURCE          m_resMPCRowStoreScratchBuffer = nullptr;                //!< Handle of mpc row store surface

    bool                   m_vdencBrcImgStatAllocated = false;  //!< Vdenc bitrate control image state allocated flag

    bool                   m_lastSlice = false;
    bool                   m_lastPic   = false;

MEDIA_CLASS_DEFINE_END(encode__AvcVdencPkt)
};

}  // namespace encode

#endif  // !__CODECHAL_AVC_VDENC_PACKET_H__
