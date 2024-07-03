/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     encode_av1_vdenc_packet.h
//! \brief    Defines the interface to adapt to AV1 VDENC packet
//!

#ifndef __CODECHAL_AV1_VDENC_PACKET_H__
#define __CODECHAL_AV1_VDENC_PACKET_H__

#include <vector>
#include "media_cmd_packet.h"
#include "encode_av1_vdenc_pipeline.h"
#include "encode_utils.h"
#include "encode_av1_basic_feature.h"
#include "encode_status_report.h"
#include "codec_def_encode_av1.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"
#include "mhw_mi_itf.h"

namespace encode
{
#define CODECHAL_ENCODE_RECYCLED_BUFFER_NUM 6
#define CODECHAL_PAGE_SIZE 0x1000
#define CODECHAL_AV1_PAK_STREAMOUT_SIZE 0x500000  //size is accounted for 4Kx4K with all 8x8 CU,based on streamout0 and streamout1 requirements
//(4096*4096)/64 *16 (streamout0) + 1MB(streamout 1). there is scope to reduce streamout1 size. Need to check with HW team.
// 8K is just an estimation

//!
//! \struct AtomicScratchBuffer
//! \brief  The sturct of Atomic Scratch Buffer
//!
struct AtomicScratchBufferAv1
{
    PMOS_RESOURCE resAtomicScratchBuffer;  //!> Handle of eStatus buffer
    uint32_t     *pData;                   //!> Pointer of the buffer of actual data
    uint16_t      encodeUpdateIndex;       //!> used for VDBOX update encode status
    uint16_t      tearDownIndex;           //!> Reserved for future extension
    uint32_t      zeroValueOffset;         //!> Store the result of the ATOMIC_CMP
    uint32_t      operand1Offset;          //!> Operand 1 of the ATOMIC_CMP
    uint32_t      operand2Offset;          //!> Operand 2 of the ATOMIC_CMP
    uint32_t      operand3Offset;          //!> Copy of the operand 1
    uint32_t      size;                    //!> Size of the buffer
    uint32_t      operandSetSize;          //!> Size of Operand set
};

inline uint32_t GetUseInterVsSkipSADWinner(uint32_t MrgCand8x8DepEn, uint32_t MrgCand16x16DepEn)
{
    uint32_t useInterVsSkipSADWinner = 0;

    useInterVsSkipSADWinner =
        ((MrgCand8x8DepEn & 1) << 0) | (1 << 1) |
        ((MrgCand16x16DepEn & 1) << 2) | (1 << 3);

    if (MrgCand8x8DepEn == 3)
    {
        useInterVsSkipSADWinner = (1 << 3) | (MrgCand16x16DepEn & 0x1) << 2;
    }
    if (MrgCand16x16DepEn == 3)
    {
        useInterVsSkipSADWinner = (1 << 1) | (MrgCand8x8DepEn & 0x1);
    }
    if (MrgCand16x16DepEn == 3 && MrgCand8x8DepEn == 3)
    {
        useInterVsSkipSADWinner = 0;
    }

    return useInterVsSkipSADWinner;
}

class Av1VdencPkt : public CmdPacket, public MediaStatusReportObserver, public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:
    Av1VdencPkt(MediaPipeline* pipeline, MediaTask* task, CodechalHwInterfaceNext* hwInterface);
    virtual ~Av1VdencPkt() {}

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
    MOS_STATUS CalculateCommandSize(
        uint32_t &commandBufferSize,
        uint32_t &requestedPatchListSize) override;

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS PerformSwStitch(
        const EncodeReportTileData* tileReportData,
        PakHwTileSizeRecord* tileStatusReport,
        EncodeStatusReportData* statusReportData);
#endif

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "AV1VDENC_PASS" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

protected:
#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief  Dump input resources or infomation before submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpInput();

    virtual MOS_STATUS DumpStatistics();
#endif

    virtual MOS_STATUS AllocateResources();

    virtual MOS_STATUS StartStatusReport(
        uint32_t            srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    virtual MOS_STATUS EndStatusReport(
        uint32_t           srType,
        MOS_COMMAND_BUFFER *cmdBuffer) override;

    virtual MOS_STATUS ReadAvpStatus(
        MHW_VDBOX_NODE_IND vdboxIndex,
        MediaStatusReport  *statusReport,
        MOS_COMMAND_BUFFER &cmdBuffer);
    
    virtual MOS_STATUS AddOneTileCommands(
        MOS_COMMAND_BUFFER  &cmdBuffer,
        uint32_t            tileRow,
        uint32_t            tileCol,
        uint32_t            tileRowPass = 0) = 0;

    virtual MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer) = 0;

    virtual MOS_STATUS RegisterPostCdef() = 0;

    virtual MOS_STATUS PatchTileLevelCommands(MOS_COMMAND_BUFFER &cmdBuffer, uint8_t packetPhase);

    MOS_STATUS AddCondBBEndFor2ndPass(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS Construct3rdLevelBatch();

    virtual MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface);

    virtual MOS_STATUS UpdateStatusReport(uint32_t srType, MOS_COMMAND_BUFFER *cmdBuffer) override;

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
    //! \brief    Calculate avp picture state command size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalculateAvpPictureStateCommandSize(uint32_t * commandsSize, uint32_t * patchListSize);

    //!
    //! \brief    get  SliceStatesSize and SlicePatchListSize,
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CalculateAvpCommandsSize();
    virtual MOS_STATUS AddPictureVdencCommands(MOS_COMMAND_BUFFER &cmdBuffer);
    virtual MOS_STATUS PatchPictureLevelCommands(const uint8_t &packetPhase, MOS_COMMAND_BUFFER &cmdBuffer);

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
        EncodeStatusMfx        *encodeStatusMfx,
        EncodeStatusReportData *statusReportData);

#endif  // USE_CODECHAL_DEBUG_TOOL

    Av1VdencPipeline *m_pipeline = nullptr;

    // Interfaces
    EncodeAllocator                 *m_allocator        = nullptr;
    CodechalHwInterfaceNext         *m_hwInterface      = nullptr;
    CodechalHwInterfaceNext         *m_hwInterfaceNext  = nullptr;
    EncodeMemComp                   *m_mmcState         = nullptr;
    Av1BasicFeature                 *m_basicFeature     = nullptr;              //!< Encode parameters used in each frame
    MHW_VDBOX_NODE_IND              m_vdboxIndex        = MHW_VDBOX_NODE_1;
    uint32_t                        m_mvOffset          = 0;                    //!< MV data offset, in 64 byte
    std::shared_ptr<mhw::vdbox::vdenc::Itf>   m_vdencItf           = nullptr;
    std::shared_ptr<mhw::vdbox::avp::Itf>     m_avpItf             = nullptr;
    const CODEC_AV1_ENCODE_PICTURE_PARAMS    *m_av1PicParams       = nullptr;   //!< Pointer to picture parameter
    const CODEC_AV1_ENCODE_SEQUENCE_PARAMS   *m_av1SeqParams       = nullptr;   //!< Pointer to sequence parameter
    const PCODECHAL_NAL_UNIT_PARAMS          *m_nalUnitParams      = nullptr;   //!< Pointer to NAL unit parameters
    uint8_t                                   m_prevFrameType      = keyFrame;  //!< Previous frame type

    std::shared_ptr<MediaFeatureManager::ManagerLite> m_featureManager = nullptr;

    mutable uint8_t m_curAvpSurfStateId = 0;

    AtomicScratchBufferAv1 m_atomicScratchBuf = {};  //!< Stores atomic operands and result

    bool m_userFeatureUpdated_post_cdef                 = false;    //!< Inidate if mmc user feature key for post cdef is updated
    bool m_vdencPakObjCmdStreamOutEnabled               = false;    //!< Pakobj stream out enable flag
    PMOS_RESOURCE m_resCumulativeCuCountStreamoutBuffer = nullptr;  //!< Cumulative CU count stream out buffer
    PMOS_RESOURCE m_vdencIntraRowStoreScratch           = nullptr;
    PMOS_RESOURCE m_vdencTileRowStoreBuffer             = nullptr;  //!< Tile row store buffer
    PMOS_RESOURCE m_resVDEncPakObjCmdStreamOutBuffer    = nullptr;  //!< Resource of Vdenc Pak object command stream out buffer
    PMOS_RESOURCE m_resVDEncStatsBuffer                 = nullptr;  //!< Resource of Vdenc status buffer
    PMOS_RESOURCE m_resVDEncCuObjStreamOutBuffer        = nullptr;  //!< Resource of Vdenc Cu object stream out buffer

    bool     m_usePatchList                = false;  //!< Use Ptach List or not
    uint32_t m_pictureStatesSize           = 0;  //!< Picture states size
    uint32_t m_picturePatchListSize        = 0;  //!< Picture patch list size
    uint32_t m_tileStatesSize              = 0;  //!< Slice states size
    uint32_t m_tilePatchListSize           = 0;  //!< Slice patch list size

    MOS_STATUS SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        MHW_VDBOX_SURFACE_PARAMS &      srcSurfaceParams,
        MHW_VDBOX_SURFACE_PARAMS &      reconSurfaceParams);

    MOS_STATUS SetSurfaceState(
        PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams);

    void SetPerfTag();
    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS SendPrologCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS SetRowstoreCachingOffsets();

    virtual void UpdateParameters();

    MOS_STATUS ReadPakMmioRegisters(PMOS_COMMAND_BUFFER cmdBuf, bool firstTile);

    MOS_STATUS ReadPakMmioRegistersAtomic(PMOS_COMMAND_BUFFER cmdBuf);

    MHW_SETPAR_DECL_HDR(VD_PIPELINE_FLUSH);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_HEVC_VP9_TILE_SLICE_STATE);

    MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_MODE_SELECT);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_IND_OBJ_BASE_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_TILE_CODING);

    virtual MOS_STATUS AddAllCmds_AVP_SURFACE_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    virtual MOS_STATUS AddAllCmds_AVP_PAK_INSERT_OBJECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

    virtual MOS_STATUS AddAllCmds_AVP_PIPE_MODE_SELECT(PMOS_COMMAND_BUFFER cmdBuffer) const;

    virtual MOS_STATUS AddAllCmds_AVP_SEGMENT_STATE(PMOS_COMMAND_BUFFER cmdBuffer) const;

    virtual MOS_STATUS GetVdencStateCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const;

    virtual MOS_STATUS GetVdencPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const;

    virtual MOS_STATUS GetAvpPrimitiveCommandsDataSize(uint32_t *commandsSize, uint32_t *patchListSize) const;

    virtual MOS_STATUS PrepareHWMetaData(MOS_COMMAND_BUFFER *cmdBuffer);

    virtual MOS_STATUS PrepareHWMetaDataFromStreamout(MOS_COMMAND_BUFFER *cmdBuffer, const MetaDataOffset resourceOffset, const AV1MetaDataOffset AV1ResourceOffset);

    virtual MOS_STATUS PrepareHWMetaDataFromRegister(MOS_COMMAND_BUFFER *cmdBuffer, const MetaDataOffset resourceOffset);

    virtual MOS_STATUS PrepareHWMetaDataFromDriver(MOS_COMMAND_BUFFER *cmdBuffer, const MetaDataOffset resourceOffset, const AV1MetaDataOffset AV1ResourceOffset);

    virtual MOS_STATUS readBRCMetaDataFromSLBB(MOS_COMMAND_BUFFER *cmdBuffer, PMOS_RESOURCE presDst, uint32_t dstOffset, PMOS_RESOURCE presSrc, uint32_t srcOffset, uint32_t significantBits);

    virtual MOS_STATUS PrepareHWMetaDataFromStreamoutTileLevel(MOS_COMMAND_BUFFER *cmdBuffer, uint32_t tileCol, uint32_t tileRow);

    inline MOS_STATUS CalAtomic(PMOS_RESOURCE presDst, uint32_t dstOffset, PMOS_RESOURCE presSrc, uint32_t srcOffset, mhw::mi::MHW_COMMON_MI_ATOMIC_OPCODE opCode, MOS_COMMAND_BUFFER *cmdBuffer);

    MEDIA_CLASS_DEFINE_END(encode__Av1VdencPkt)
};

}  // namespace encode
#endif
