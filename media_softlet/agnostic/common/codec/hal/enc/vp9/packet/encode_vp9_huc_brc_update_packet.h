/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_vp9_huc_brc_init_packet.h
//! \brief    Defines the implementation of huc/brc update packet for VP9
//!

#ifndef __ENCODE_VP9_HUC_BRC_UPDATE_PACKET_H__
#define __ENCODE_VP9_HUC_BRC_UPDATE_PACKET_H__

#include "media_cmd_packet.h"
#include "encode_huc.h"
#include "media_pipeline.h"
#include "encode_pipeline.h"
#include "encode_utils.h"
#include "encode_vp9_vdenc_pipeline.h"
#include "encode_vp9_basic_feature.h"

namespace encode
{

class Vp9HucBrcUpdatePkt : public EncodeHucPkt
{
public:
    //!
    //! \brief  Vp9HucBrcUpdatePkt constructor
    //!
    Vp9HucBrcUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : EncodeHucPkt(pipeline, task, hwInterface) {}

    //!
    //! \brief  Vp9HucBrcUpdatePkt destructor
    //!
    virtual ~Vp9HucBrcUpdatePkt() {}

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
    MOS_STATUS Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket) override;

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
        return "BrcUpdate_Pass" + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

protected:
    //!
    //! \brief  Allocate resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

    //!
    //! \brief  Set huc dmem buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemBuffer() const;

#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief  Dump input resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpInput() override;
#endif

    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    static constexpr uint32_t m_vdboxHucVp9VdencBrcUpdateKernelDescriptor = 12;  //!< VDBox Huc VDEnc Brc update kernel descriptor
    static const uint32_t m_brcUpdateDmem[64];

    MOS_RESOURCE     m_resVdencBrcUpdateDmemBuffer[3][CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};       //!< VDENC BRC/Update DMEM buffer
    Vp9BasicFeature *m_basicFeature                   = nullptr;  //!< VP9 Basic Feature used in each frame

MEDIA_CLASS_DEFINE_END(encode__Vp9HucBrcUpdatePkt)
};
}  // namespace encode

#endif
