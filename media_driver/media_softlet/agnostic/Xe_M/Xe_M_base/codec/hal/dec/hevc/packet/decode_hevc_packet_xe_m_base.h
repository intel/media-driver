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
//! \file     decode_hevc_packet_xe_m_base.h
//! \brief    Defines the implementation of hevc decode packet
//!

#ifndef __DECODE_HEVC_PACKET_XE_M_BASE_H__
#define __DECODE_HEVC_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "decode_hevc_pipeline.h"
#include "decode_utils.h"
#include "decode_hevc_basic_feature.h"
#include "decode_status_report.h"
#include "decode_hevc_picture_packet_xe_m_base.h"
#include "decode_hevc_slice_packet_xe_m_base.h"

namespace decode
{

class HevcDecodePktXe_M_Base : public CmdPacket, public MediaStatusReportObserver
{
public:
    HevcDecodePktXe_M_Base(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterface *hwInterface)
        : CmdPacket(task)
    {
        if (pipeline != nullptr)
        {
            m_statusReport   = pipeline->GetStatusReportInstance();
            m_featureManager = pipeline->GetFeatureManager();
            m_hevcPipeline   = dynamic_cast<HevcPipeline *>(pipeline);
        }
        if (hwInterface != nullptr)
        {
            m_hwInterface    = hwInterface;
            m_miInterface    = hwInterface->GetMiInterface();
            m_osInterface    = hwInterface->GetOsInterface();
            m_vdencInterface = hwInterface->GetVdencInterface();
        }
    }
    virtual ~HevcDecodePktXe_M_Base() {}

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
    //! \brief  One frame is completed
    //! \param  [in] mfxStatus
    //!         pointer to status buffer which for MFX
    //! \param  [in] rcsStatus
    //!         pointer to status buffer which for RCS
    //! \param  [in, out] statusReport
    //!         pointer of DecodeStatusReport
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Completed(void *mfxStatus, void *rcsStatus, void *statusReport) override;

protected:
    void SetPerfTag(CODECHAL_MODE mode, uint16_t picCodingType);

    MOS_STATUS SendPrologWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested);

    MOS_STATUS MiFlush(MOS_COMMAND_BUFFER & cmdBuffer);
    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);

    MOS_STATUS ReadVdboxId(MOS_COMMAND_BUFFER &cmdBuffer);

    MOS_STATUS ReadHcpStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer);
    virtual MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
    virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpSecondaryCommandBuffer(MOS_COMMAND_BUFFER &cmdBuffer);
#endif

    MediaFeatureManager *    m_featureManager   = nullptr;
    HevcPipeline *           m_hevcPipeline     = nullptr;
    DecodeAllocator *        m_allocator        = nullptr;
    HevcBasicFeature *       m_hevcBasicFeature = nullptr;
    MhwVdboxVdencInterface * m_vdencInterface   = nullptr;
    CodechalHwInterface*     m_hwInterface      = nullptr;

    // Parameters passed from application
    const CODEC_HEVC_PIC_PARAMS *m_hevcPicParams = nullptr; //!< Pointer to picture parameter

    DecodePhase                 *m_phase         = nullptr; //!< Phase for current packet

    uint32_t m_pictureStatesSize    = 0;
    uint32_t m_picturePatchListSize = 0;
    uint32_t m_sliceStatesSize      = 0;
    uint32_t m_slicePatchListSize   = 0;
MEDIA_CLASS_DEFINE_END(decode__HevcDecodePktXe_M_Base)
};

}  // namespace decode
#endif
