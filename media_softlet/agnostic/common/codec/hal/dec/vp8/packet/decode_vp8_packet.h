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
//! \file     decode_vp8_packet.h
//! \brief    Defines the implementation of vp8 decode packet
//!

#ifndef __DECODE_VP8_PACKET_H__
#define __DECODE_VP8_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_vp8_pipeline.h"
#include "decode_utils.h"
#include "decode_vp8_basic_feature.h"
#include "decode_status_report.h"
#include "decode_vp8_picture_packet.h"
#include "decode_vp8_slice_packet.h"

namespace decode
{

class Vp8DecodePkt : public CmdPacket, public MediaStatusReportObserver
{
public:
    Vp8DecodePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

    virtual ~Vp8DecodePkt(){};

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
    //! \brief Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "VP8_DECODE";
    }

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

    MOS_STATUS SendPrologWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested);
    MOS_STATUS ReadMfxStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer);

    MOS_STATUS MiFlush(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER &cmdBuffer);
    bool IsPrologRequired();

    virtual MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
    virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;

    MediaFeatureManager         *m_featureManager   = nullptr;
    Vp8Pipeline                 *m_vp8Pipeline      = nullptr;
    DecodeAllocator             *m_allocator        = nullptr;
    Vp8BasicFeature             *m_vp8BasicFeature  = nullptr;
    CodechalHwInterfaceNext     *m_hwInterface      = nullptr;
    Vp8DecodePicPkt             *m_picturePkt       = nullptr;
    Vp8DecodeSlcPkt             *m_slicePkt         = nullptr;
    // Parameters passed from application
    const CODEC_VP8_PIC_PARAMS  *m_vp8PicParams     = nullptr;  //!< Pointer to picture parameter

    DecodePhase                 *m_phase            = nullptr;  //!< Phase for current packet
    DecodeMemComp               *m_mmcState         = nullptr;


    uint32_t m_pictureStatesSize                    = 0;
    uint32_t m_picturePatchListSize                 = 0;
    uint32_t m_sliceStatesSize                      = 0;
    uint32_t m_slicePatchListSize                   = 0;

MEDIA_CLASS_DEFINE_END(decode__Vp8DecodePkt)
};

}  // namespace decode
#endif // !__DECODE_VP8_PACKET_H__

