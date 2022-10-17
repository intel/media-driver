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
//! \file     encode_huc_la_update_packet.h
//! \brief    Defines the interface of Huc Lookahead update packet

#ifndef __CODECHAL_HUC_LA_UPDATE_PACKET_H__
#define __CODECHAL_HUC_LA_UPDATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "encode_hevc_vdenc_pipeline.h"

namespace encode
{
class HucLaUpdatePkt : public EncodeHucPkt
{
public:
    HucLaUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : EncodeHucPkt(pipeline, task, hwInterface)
    {
        m_miItf = m_hwInterface->GetMiInterfaceNext();
    }

    virtual ~HucLaUpdatePkt() {}

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

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

    virtual MOS_STATUS DumpOutput() override;

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpInput() override;
#endif

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
        return "LAUPDATE";
    }

protected:
    virtual MOS_STATUS AllocateResources() override;

    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    virtual MOS_STATUS ReadLPLAData(MOS_COMMAND_BUFFER *commandBuffer);

    HevcBasicFeature              *m_basicFeature = nullptr;  //!< Hevc Basic Feature used in each frame
    std::shared_ptr<mhw::mi::Itf> m_miItf         = nullptr;

MEDIA_CLASS_DEFINE_END(encode__HucLaUpdatePkt)
};
}  // namespace encode

#endif
