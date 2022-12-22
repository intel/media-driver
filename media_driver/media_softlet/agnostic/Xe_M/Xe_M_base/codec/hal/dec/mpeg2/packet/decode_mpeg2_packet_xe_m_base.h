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
//! \file     decode_mpeg2_packet_xe_m_base.h
//! \brief    Defines the interface for mpeg2 decode packet for Xe_M_Base.
//!

#ifndef __DECODE_MPEG2_PACKET_XE_M_BASE_H__
#define __DECODE_MPEG2_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "codechal_hw.h"
#include "decode_vdbox_mfx_common.h"
#include "decode_mpeg2_basic_feature.h"
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "decode_mpeg2_picture_packet_xe_m_base.h"
#include "decode_mpeg2_slice_packet_xe_m_base.h"
#include "decode_mpeg2_mb_packet_xe_m_base.h"

namespace decode {

    class Mpeg2DecodePktXe_M_Base : public CmdPacket, public MediaStatusReportObserver
    {
    public:
        Mpeg2DecodePktXe_M_Base(MediaPipeline* pipeline, MediaTask* task, CodechalHwInterface* hwInterface)
            : CmdPacket(task)
        {
            if (pipeline != nullptr)
            {
                m_statusReport = pipeline->GetStatusReportInstance();
                m_featureManager = pipeline->GetFeatureManager();
                m_mpeg2Pipeline = dynamic_cast<Mpeg2Pipeline*>(pipeline);
            }
            if (hwInterface != nullptr)
            {
                m_hwInterface = hwInterface;
                m_miInterface = hwInterface->GetMiInterface();
                m_osInterface = hwInterface->GetOsInterface();
            }
        }
        virtual ~Mpeg2DecodePktXe_M_Base() {};

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
        virtual MOS_STATUS Completed(void* mfxStatus, void* rcsStatus, void* statusReport) override;

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
        MOS_STATUS CalculateCommandSize(uint32_t& commandBufferSize, uint32_t& requestedPatchListSize) override;

        //!
        //! \brief  Get Packet Name
        //! \return std::string
        //!
        virtual std::string GetPacketName() override
        {
            return "MPEG2_DECODE";
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

        bool IsPrologRequired();

        MOS_STATUS SendPrologWithFrameTracking(MOS_COMMAND_BUFFER& cmdBuffer, bool frameTrackingRequested);

        MOS_STATUS MiFlush(MOS_COMMAND_BUFFER& cmdBuffer);

        MOS_STATUS AddForceWakeup(MOS_COMMAND_BUFFER& cmdBuffer);

        MOS_STATUS ReadMfxStatus(MediaStatusReport* statusReport, MOS_COMMAND_BUFFER& cmdBuffer);

        virtual MOS_STATUS StartStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;
        virtual MOS_STATUS EndStatusReport(uint32_t srType, MOS_COMMAND_BUFFER* cmdBuffer) override;

        MediaFeatureManager* m_featureManager = nullptr;
        Mpeg2Pipeline* m_mpeg2Pipeline = nullptr;
        DecodeAllocator* m_allocator = nullptr;
        Mpeg2BasicFeature* m_mpeg2BasicFeature = nullptr;
        CodechalHwInterface* m_hwInterface = nullptr;
        DecodeMemComp* m_mmcState = nullptr;

        Mpeg2DecodePicPktXe_M_Base* m_picturePkt = nullptr;
        Mpeg2DecodeSlcPktXe_M_Base* m_slicePkt = nullptr;
        Mpeg2DecodeMbPktXe_M_Base* m_mbPkt = nullptr;

        // Parameters passed from application
        const CodecDecodeMpeg2PicParams* m_mpeg2PicParams = nullptr;  //!< Pointer to picture parameter

        BatchBufferArray* m_secondLevelBBArray = nullptr; //!< Point to second level batch buffer array

        uint32_t m_pictureStatesSize = 0;
        uint32_t m_picturePatchListSize = 0;
        uint32_t m_sliceStatesSize = 0;
        uint32_t m_slicePatchListSize = 0;
        uint32_t m_mbStatesSize = 0;
        uint32_t m_mbPatchListSize = 0;
    MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodePktXe_M_Base)
    };

}
#endif // !__DECODE_MPEG2_PACKET_XE_M_BASE_H__
