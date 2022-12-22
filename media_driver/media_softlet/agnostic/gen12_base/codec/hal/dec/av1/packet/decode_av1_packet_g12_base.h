/*
* Copyright (c) 2019-2021, Intel Corporation
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
//! \file     decode_av1_packet_g12_base.h
//! \brief    Defines the implementation of gen12 base av1 decode packet
//!

#ifndef __DECODE_AV1_PACKET_G12_BASE_H__
#define __DECODE_AV1_PACKET_G12_BASE_H__

#include "media_cmd_packet.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_utils.h"
#include "decode_av1_basic_feature_g12.h"
#include "decode_status_report.h"
#include "decode_av1_picture_packet_g12_base.h"
#include "decode_av1_tile_packet_g12_base.h"

namespace decode
{

class Av1DecodePkt_G12_Base : public CmdPacket, public MediaStatusReportObserver
{
public:
    Av1DecodePkt_G12_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
        : CmdPacket(task)
    {
        if (pipeline != nullptr)
        {
            m_statusReport = pipeline->GetStatusReportInstance();
            m_featureManager = pipeline->GetFeatureManager();
            m_av1Pipeline = dynamic_cast<Av1PipelineG12_Base*>(pipeline);
        }
        if (hwInterface != nullptr)
        {
            m_hwInterface    = hwInterface;
            m_miInterface    = hwInterface->GetMiInterface();
            m_osInterface    = hwInterface->GetOsInterface();
            m_vdencInterface = hwInterface->GetVdencInterface();
        }
    }
    virtual ~Av1DecodePkt_G12_Base(){};

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Prepare interal parameters, should be invoked for each frame
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
    //! \brief  One frame is completed
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for mfx
    //! \param  [in] rcsStatus
    //!         pointer to status buffer which for RCS
    //! \param  [in, out] statusReport
    //!         pointer of DecoderStatusReport
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
    MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "AV1_DECODE";
    }

    PMHW_BATCH_BUFFER GetSecondLvlBB() { return m_batchBuf; };

protected:
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

    void SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType);

    bool IsPrologRequired();

    MOS_STATUS SendPrologWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested);

    MOS_STATUS VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer);

    MOS_STATUS MiFlush(MOS_COMMAND_BUFFER & cmdBuffer);

    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS ReadAvpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer);

    virtual MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
    virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
    MOS_STATUS InitPicLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase);

    MediaFeatureManager    *m_featureManager   = nullptr;
    Av1PipelineG12_Base    *m_av1Pipeline      = nullptr;
    DecodeAllocator        *m_allocator        = nullptr;
    Av1BasicFeatureG12     *m_av1BasicFeature  = nullptr;
    MhwVdboxVdencInterface *m_vdencInterface   = nullptr;
    CodechalHwInterface    *m_hwInterface      = nullptr;
    DecodeMemComp          *m_mmcState         = nullptr;

    Av1DecodePicPkt_G12_Base  *m_picturePkt    = nullptr;
    Av1DecodeTilePkt_G12_Base *m_tilePkt       = nullptr;
    bool                   m_isLastTileInPartialFrm  = false;
    bool                   m_isFirstTileInPartialFrm = false;

    // Parameters passed from application
    const CodecAv1PicParams *m_av1PicParams   = nullptr;  //!< Pointer to picture parameter

    uint32_t m_pictureStatesSize    = 0;
    uint32_t m_picturePatchListSize = 0;
    uint32_t m_tileStatesSize       = 0;
    uint32_t m_tilePatchListSize    = 0;

    BatchBufferArray     *m_secondLevelBBArray = nullptr; //!< Point to second level batch buffer
    MOS_COMMAND_BUFFER    m_picCmdBuffer;
    PMHW_BATCH_BUFFER     m_batchBuf = nullptr;
MEDIA_CLASS_DEFINE_END(decode__Av1DecodePkt_G12_Base)
};

}  // namespace decode
#endif // !__DECODE_AV1_PACKET_G12_BASE_H__

