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
//! \file     encode_vp9_vdenc_packet.h
//! \brief    Defines the interface to adapt to VP9 VDENC packet
//!

#ifndef __ENCODE_VP9_VDENC_PACKET_H__
#define __ENCODE_VP9_VDENC_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_vp9_vdenc_pipeline.h"
#include "encode_status_report.h"
#include "encodecp.h"

namespace encode
{

#define CODECHAL_ENCODE_VP9_BRC_MAX_NUM_OF_PASSES 4

class Vp9VdencPkt : public CmdPacket, public MediaStatusReportObserver, public mhw::vdbox::hcp::Itf::ParSetting, public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9VdencPkt constructor
    //!
    Vp9VdencPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : CmdPacket(task),
          m_pipeline(dynamic_cast<Vp9VdencPipeline *>(pipeline)),
          m_hwInterface(dynamic_cast<CodechalHwInterfaceNext *>(hwInterface))
    {
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeline);

        m_osInterface    = m_hwInterface->GetOsInterface();
        m_statusReport   = m_pipeline->GetStatusReportInstance();
        m_featureManager = m_pipeline->GetFeatureManager();
        m_encodecp       = m_pipeline->GetEncodeCp();

        m_hcpInterfaceNew   = std::static_pointer_cast<mhw::vdbox::hcp::Itf>(m_hwInterface->GetHcpInterfaceNext());
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_hcpInterfaceNew);

        m_vdencInterfaceNew = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(m_hwInterface->GetVdencInterfaceNext());
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_vdencInterfaceNew);

        m_miItf             = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_miItf);
    }

    //!
    //! \brief  Vp9VdencPkt destructor
    //!
    virtual ~Vp9VdencPkt();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
    //! \param  [in] params
    //!         Pointer to the input parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Prepare() override;

    //!
    //! \brief  Destroy the media packet and release the resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Destroy() override;

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override
    {
        return MOS_STATUS_SUCCESS;
    }

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
    //! \param  [in, out] commandBufferSize
    //!         requested size
    //! \param  [in, out] requestedPatchListSize
    //!         requested size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Dump output resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpOutput() override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "PAK_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

    void fill_pad_with_value(PMOS_SURFACE psSurface, uint32_t real_height, uint32_t aligned_height);

    //!
    //! \brief  Add HCP_SURFACE_STATE
    //! \param  [in] cmdBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAllCmds_HCP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(HCP_IND_OBJ_BASE_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_REF_SURFACE_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);
    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(VDENC_CONTROL_STATE);
    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    enum FlushCmd
    {
        waitVp9 = 0,
        waitVdenc,
        waitVp9Vdenc
    };

protected:
    //!
    //! \brief  Calculate picture state command size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalculatePictureStateCommandSize();

    //!
    //! \brief  Get vdenc state command size and patch size
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVdencStateCommandsDataSize(uint32_t &vdencPictureStatesSize, uint32_t &vdencPicturePatchListSize);

    //!
    //! \brief  Get  SliceStatesSize and SlicePatchListSize,
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHxxPrimitiveCommandSize();

    //!
    //! \brief  Calculate Command Buffer Size
    //! \return uint32_t
    //!         Command buffer size calculated
    //!
    virtual uint32_t CalculateCommandBufferSize();

    //!
    //! \brief  Calculate Patch List Size
    //! \return uint32_t
    //!         Patchlist size calculated
    //!
    virtual uint32_t CalculatePatchListSize();

    //!
    //! \brief  Allocate resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief  Free resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS        FreeResources();

    // Inline functions
    //!
    //! \brief  Validate Vdbox index
    //! \param  [in] vdboxIndex
    //!         Index of Vdbox
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateVdboxIdx(const MHW_VDBOX_NODE_IND &vdboxIndex);

    //!
    //! \brief  Set perf tag
    //! \param  [in] type
    //!         Perf tag call type
    //! \param  [in] mode
    //!         Encoding mode
    //! \param  [in] picCodingType
    //!         Picture coding type
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void SetPerfTag(uint16_t type, uint16_t mode, uint16_t picCodingType);

    //!
    //! \brief    Add force wake-up command
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Sends prolog commands
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendPrologCmds(MOS_COMMAND_BUFFER &cmdBuffer);

    //! \brief    Set Rowstore Cache offset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetRowstoreCachingOffsets();

    //!
    //! \brief  Add command to set hcp pipe mode select values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeModeSelectCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) = 0;

    //!
    //! \brief    Set HCP surfaces' parameters
    //! \param    [in, out] surfacesParams
    //!           Pointer to surfaces' parameters structures
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHcpSurfacesParams(
        MHW_VDBOX_SURFACE_PARAMS *surfacesParams);

    virtual MOS_STATUS SetHcpSurfaceMMCState();

    //!
    //! \brief    Add command to set Hcp Pipe Buffer Address values
    //! \param    [in, out] cmdBuffer
    //!           Command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpPipeBufAddrCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add command to set Hcp Indirect Object Base Address values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHcpIndObjBaseAddrCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add command to set vdenc pipe mode select values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencPipeModeSelectCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Set vdenc DS surfaces parameters
    //! \param  [in] dsSurfaceParams
    //!         DS surfaces parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVdencDsSurfaceParams(
        MHW_VDBOX_SURFACE_PARAMS *dsSurfaceParams);

    //!
    //! \brief  Add command to set vdenc surfaces state
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVdencSurfacesStateCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Set vdenc pipe mode select parameter
    //! \param  [in] pipeModeSelectParams
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencPipeModeSelectParams(
        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS &vdboxPipeModeSelectParams);

    //!
    //! \brief  Add command to set Vdenc Pipe Buffer Address values
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencPipeBufAddrCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Set vdenc pipe buffer address parameter
    //! \param  [in] pipeBufAddrParams
    //!         Pipeline buffer address parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetVdencPipeBufAddrParams(
        MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams);

    //!
    //! \brief  Set Hcp Pipe Buffer Address parameter (MMC)
    //! \param  [in, out] pipeBufAddrParams
    //!         Pointer to pipe buffer address parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPipeBufAddrMmcState(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams);

    //!
    //! \brief  Add command to set vdenc second level batch buffer command
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencSecondLevelBatchBufferCmd(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief    Set surface's state (MMC)
    //! \param    [in, out] surfacesParams
    //!           Pointer to surfaces' parameters structures
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSurfaceMmcState(
        PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams);

    //!
    //! \brief    Add VDENC_WALKER_STATE commands to command buffer
    //! \param    [in, out] cmdBuffer
    //!           Pointer to the command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddVdencWalkerStateCmd(
        MOS_COMMAND_BUFFER &cmdBuffer) = 0;

    //!
    //! \brief  Start Status Report
    //! \param  [in] srType
    //!         status report type for send cmds
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS StartStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    //!
    //! \brief  End Status Report
    //! \param  [in] srType
    //!         status report type for send cmds
    //! \param  [in, out] cmdBuffer
    //!         cmdbuffer to send cmds
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EndStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    //!
    //! \brief  Ensure all commands memory written out
    //! \param  [in] cmdBuffer
    //!         Command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EnsureAllCommandsExecuted(
        MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Add command to read the HCP status
    //! \param  [in] vdboxIndex
    //!         Index of vdbox
    //! \param  [in] statusReport
    //!         Encode status report
    //! \param  [in, out] cmdBuffer
    //!         Command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ReadHcpStatus(MHW_VDBOX_NODE_IND vdboxIndex, MediaStatusReport *statusReport, MOS_COMMAND_BUFFER &cmdBuffer);

    //!
    //! \brief  Update parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateParameters();

    //!
    //! \brief  Dump input resources or infomation before submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpInput();

    //!
    //! \brief  Get value of MultiEngineMode
    //! \return MHW_VDBOX_HCP_MULTI_ENGINE_MODE
    //!         Multi Engine Mode
    //!
    MHW_VDBOX_HCP_MULTI_ENGINE_MODE getMultiEngineMode() const;

    //!
    //! \brief  Get value of PipeWorkMode
    //! \return MHW_VDBOX_HCP_PIPE_WORK_MODE
    //!         Pipe Work Mode
    //!
    MHW_VDBOX_HCP_PIPE_WORK_MODE getPipeWorkMode() const;

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief  Dump the output resources in status report callback function
    //! \param  [in] encodeStatusMfx
    //!         Pointer to encoder status for vdbox
    //! \param  [in] statusReportData
    //!         Pointer to encoder status report data
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpResources(
        EncodeStatusMfx *       encodeStatusMfx,
        EncodeStatusReportData *statusReportData);
#endif

    //!
    //! \brief  Add HCP_SURFACE_STATE
    //! \param  [in] cmdBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Add_HCP_SURFACE_STATE(PMOS_COMMAND_BUFFER &cmdBuffer);

    enum CODEC_SELECT
    {
        CODEC_SELECT_DECODE = 0,
        CODEC_SELECT_ENCODE = 1,
    };

    enum CODEC_STANDARD_SELECT
    {
        CODEC_STANDARD_SELECT_HEVC = 0,
        CODEC_STANDARD_SELECT_VP9  = 1,
        CODEC_STANDARD_SELECT_AV1  = 2,
    };

    Vp9VdencPipeline *m_pipeline = nullptr;

    // Interfaces
    EncodeAllocator *         m_allocator         = nullptr;  //!< Interface of encode allocator
    CodechalHwInterfaceNext * m_hwInterface       = nullptr;  //!< Interface of Codec HAL HW
    MediaFeatureManager *     m_featureManager    = nullptr;  //!< Interface of feature manager
    Vp9BasicFeature *         m_basicFeature      = nullptr;  //!< Encode parameters used in each frame
    EncodeMemComp *           m_mmcState          = nullptr;  //!< Interface of codec media memory comp
    EncodeCp *                m_encodecp          = nullptr;  //!< Interface of encode CP

    std::shared_ptr<mhw::vdbox::hcp::Itf> m_hcpInterfaceNew     = nullptr;  //!< Interface of MHW
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencInterfaceNew = nullptr;  //!< Interface of MHW

    static constexpr uint32_t m_maxNumPipes = 4;

    // Parameters passed from application
    const CODEC_VP9_ENCODE_SEQUENCE_PARAMS *m_vp9SeqParams     = nullptr;  //!< Pointer to sequence parameters
    const CODEC_VP9_ENCODE_PIC_PARAMS *     m_vp9PicParams     = nullptr;  //!< Pointer to picture parameters
    const CODEC_VP9_ENCODE_SEGMENT_PARAMS * m_vp9SegmentParams = nullptr;  //!< Pointer to segment parameters

    MHW_VDBOX_NODE_IND m_vdboxIndex = MHW_VDBOX_NODE_1;  //!< Index of VDBOX

    bool m_usePatchList = false;  //!< Use Patch List or not

    mutable uint8_t m_curHcpSurfStateId = 0;

    uint32_t m_defaultPictureStatesSize    = 0;  //!< Picture state command size
    uint32_t m_defaultPicturePatchListSize = 0;  //!< Picture state patch list size

    uint32_t m_defaultSliceStatesSize    = 0;  //!< Slice state command size
    uint32_t m_defaultSlicePatchListSize = 0;  //!< Slice state patch list size

    // CMD buffer sizes
    uint32_t m_pictureStatesSize = 0;  //!< Picture states size
    uint32_t m_sliceStatesSize   = 0;  //!< Slice states size

    // Patch list sizes
    uint32_t m_picturePatchListSize = 0;  //!< Picture patch list size
    uint32_t m_slicePatchListSize   = 0;  //!< Slice patch list size

    MHW_VDBOX_SURFACE_PARAMS m_surfacesParams[CODECHAL_HCP_ALTREF_SURFACE_ID + 1] = {0};

    // VDENC and PAK data buffers
    bool          m_vdencPakObjCmdStreamOutEnabled   = false;    //!< Pak Obj stream out enable flag
    PMOS_RESOURCE m_resVdencPakObjCmdStreamOutBuffer = nullptr;  //!< Resource of Vdenc Pak object command stream out buffer
    MOS_RESOURCE  m_resPakcuLevelStreamoutData       = {0};      //!< PAK CU level Ssreamout data buffer

    MOS_SURFACE m_output16X16InterModes = {0};
    // ME
    MOS_SURFACE m_4xMeMvDataBuffer     = {0};  //!< 4x ME MV data buffer
    MOS_SURFACE m_4xMeDistortionBuffer = {0};  //!< 4x ME distortion buffer
    MOS_SURFACE m_16xMeMvDataBuffer    = {0};  //!< 16x ME MV data buffer

    MOS_RESOURCE m_resVdencIntraRowStoreScratchBuffer     = {0};  //!< VDENC Intra row store scratch buffer
    MOS_RESOURCE m_resHvcTileRowStoreBuffer               = {0};  //!< HVC tile row store buffer
    MOS_RESOURCE m_vdencCumulativeCuCountStreamoutSurface = {0};  //!< Cumulative CU count stream out surface
    MOS_RESOURCE m_vdencTileRowStoreBuffer                = {0};  //!< Tile row store buffer
    // Segments
    MOS_RESOURCE m_resVdencSegmentMapStreamOut  = {0};  //!< VDENC Segment map stream out buffer
    MOS_RESOURCE m_resSseSrcPixelRowStoreBuffer = {0};  //!< SSE source pixel row store buffer

    mutable uint8_t m_curVdencSurfStateId = 0;

    FlushCmd m_flushCmd = waitVp9;

MEDIA_CLASS_DEFINE_END(encode__Vp9VdencPkt)
};
}  // namespace encode

#endif  // !__ENCODE_VP9_VDENC_PACKET_H__
