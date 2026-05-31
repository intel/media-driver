/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vvc_packet.h
//! \brief    Defines the implementation of VVC decode packet
//!

#ifndef __DECODE_VVC_PACKET_H__
#define __DECODE_VVC_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vvc_pipeline.h"
#include "decode_utils.h"
#include "decode_status_report.h"
#include "decode_vvc_basic_feature.h"
#include "decode_vvc_picture_packet.h"
#include "decode_vvc_slice_packet.h"
#include "mhw_vdbox_vvcp_itf.h"

namespace decode
{
class VvcPipeline;
class VvcDecodePicPkt;
class VvcDecodePkt : public CmdPacket, public MediaStatusReportObserver
{
public:
    VvcDecodePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : CmdPacket(task)
    {
        if (pipeline != nullptr)
        {
            m_statusReport = pipeline->GetStatusReportInstance();
            m_featureManager = pipeline->GetFeatureManager();
            m_vvcPipeline = dynamic_cast<VvcPipeline*>(pipeline);
        }
        if (hwInterface != nullptr)
        {
            m_hwInterface    = hwInterface;
            m_osInterface    = hwInterface->GetOsInterface();
            m_miItf          = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
            m_vdencItf       = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(hwInterface->GetVdencInterfaceNext());
            if (m_hwInterface != nullptr)
            {
                m_vvcpItf = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(m_hwInterface->GetVvcpInterfaceNext());
            }
        }
    }
    virtual ~VvcDecodePkt(){};

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
        return "VVC_DECODE";
    }

    PMHW_BATCH_BUFFER GetPictureLvlBB() { return m_picBatchBuf; };

    //!
    //! \brief  Add the command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

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

    //!
    //! \brief  Calculate pic-level command size
    //! \return uint32_t
    //!         Pic-level command size calculated
    //!
    virtual uint32_t CalculatePicCommandSize();

    //!
    //! \brief  Calculate pic-level patch list size
    //! \return uint32_t
    //!         Pic-level patch list size calculated
    //!
    virtual uint32_t CalculatePicPatchListSize();

    void CalculateVvcSliceLvlCmdSize();

    void        SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType);
    bool        IsPrologRequired();
    MOS_STATUS  SendPrologWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested);
    MOS_STATUS  VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer);
    MOS_STATUS  MiFlush(MOS_COMMAND_BUFFER & cmdBuffer);
    MOS_STATUS  AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS ReadVvcpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer);
    virtual MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
    virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;

    MOS_STATUS InitPicLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase);

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the output resources in status report callback function
    //!
    //! \param    [in] decodeStatusMfx
    //!           Pointer to decoder status for vdbox
    //! \param    [in] statusReportData
    //!           Pointer to decoder status report data
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpResources(
        DecodeStatusMfx         *decodeStatusMfx,
        DecodeStatusReportData  *statusReportData);
#endif

    MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS PackS2LSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer);

    MediaFeatureManager             *m_featureManager       = nullptr;
    VvcPipeline                     *m_vvcPipeline          = nullptr;
    DecodeAllocator                 *m_allocator            = nullptr;
    VvcBasicFeature                 *m_vvcBasicFeature      = nullptr;
    DecodeMemComp                   *m_mmcState             = nullptr;
    VvcDecodePicPkt                 *m_picturePkt           = nullptr;
    VvcDecodeSlicePkt               *m_slicePkt             = nullptr;
    bool                            m_isFirstSliceInFrame   = true;
    bool                            m_isLastSliceInFrame    = false;
    const CodecVvcPicParams         *m_vvcPicParams         = nullptr;  //!< Pointer to picture parameter

    uint32_t                        m_pictureStatesSize     = 0;
    uint32_t                        m_picturePatchListSize  = 0;
    uint32_t                        m_sliceStatesSize       = 0;
    uint32_t                        m_slicePatchListSize    = 0;
    uint32_t                        m_tileStateSize         = 0;
    uint32_t                        m_tilePatchListSize     = 0;
    uint32_t                        m_vvcpSliceCmdSize      = 0; //Slice Lvl Command Size

    MOS_COMMAND_BUFFER              m_picCmdBuffer;                   //!< picture command buffer
    PMHW_BATCH_BUFFER               m_picBatchBuf          = nullptr; //!< Pointer to batch buffer
    bool                            m_picCmdSizeCalculated = false;   //!< flag to indicate if pic-level cmd size is included

    PMHW_BATCH_BUFFER                       m_tileLevelBB          = nullptr;
    CodechalHwInterfaceNext                *m_hwInterface = nullptr;
    std::shared_ptr<mhw::vdbox::vdenc::Itf> m_vdencItf = nullptr;
    std::shared_ptr<mhw::vdbox::vvcp::Itf>  m_vvcpItf  = nullptr;


MEDIA_CLASS_DEFINE_END(decode__VvcDecodePkt)
};

}  // namespace decode
#endif // !__DECODE_VVC_PACKET_H__

