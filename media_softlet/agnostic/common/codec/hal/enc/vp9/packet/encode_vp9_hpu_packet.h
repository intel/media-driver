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
//! \file     encode_vp9_hpu_packet.h
//! \brief    Defines the interface for HPU (header's probability update) packet for VP9
//!

#ifndef __ENCODE_VP9_HPU_PACKET_H__
#define __ENCODE_VP9_HPU_PACKET_H__

#include "encode_huc.h"
#include "encode_vp9_basic_feature.h"
#include "encode_vp9_brc.h"

namespace encode
{
#define CODECHAL_ENCODE_VP9_HUC_SUPERFRAME_PASS 2
#define CODECHAL_ENCODE_VP9_BRC_SUPER_FRAME_BUFFER_SIZE MOS_ALIGN_CEIL(3 + 2 * sizeof(uint32_t), sizeof(uint32_t))
#define CODECHAL_ENCODE_VP9_REF_SEGMENT_DISABLED 0xFF

class Vp9HpuPkt : public EncodeHucPkt
{
public:
    //!
    //! \brief  Vp9HucProbPkt constructor
    //! \param  [in] pipeline
    //!         Pointer to the media pipeline
    //! \param  [in] task
    //!         Pointer to media task
    //! \param  [in] hwInterface
    //!         Pointer to HW interface
    //!
    Vp9HpuPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : EncodeHucPkt(pipeline, task, hwInterface) {}

    //!
    //! \brief  Vp9HucProbPkt destructor
    //!
    virtual ~Vp9HpuPkt() {}

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
    //! \brief  Patch hpu command sequence into the commandBuffer and
    //!         and return to the caller task
    //! \param  [in] commandBuffer
    //!         Pointer to the command buffer which is allocated by caller
    //! \param  [in] packetPhase
    //!         Indicate packet phase stage
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PatchHucProbCommands(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase = otherPacket);

    //!
    //! \brief  Calculate Command Size
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
        return (m_superFrameHucPass ? "HPU_SuperFramePass" : "HPU_Pass") + std::to_string((uint32_t)m_pipeline->GetCurrentPass());
    }

    //!
    //! \brief  Set super frame huc pass on/off
    //! \param  [in, out] superFrameHucPass
    //!         Super frame huc pass flag
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSuperFrameHucPass(bool superFrameHucPass)
    {
        m_superFrameHucPass = superFrameHucPass;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

protected:
    //!
    //! \brief  Allocate resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() override;

public:
    //!
    //! \brief  Set huc dmem buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmemBuffer() const;

    //!
    //! \brief      Get reference buffer slot index
    //! \param      [in] refreshFlags
    //!             Refresh flags
    //! \return     uint8_t
    //!             Return 0 if call success, else -1 if fail
    //!
    uint8_t GetReferenceBufferSlotIndex(
        uint8_t refreshFlags) const;

 protected:

 #if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief  Dump input resources or infomation after submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpInput() override;
#endif

    static constexpr uint32_t m_vdboxHucVp9VdencProbKernelDescriptor = 13;  //!< VDBox Huc VDEnc prob kernel descriptor

    Vp9BasicFeature *m_basicFeature = nullptr;  //!< VP9 Basic Feature used in each frame

    MOS_RESOURCE m_resHucDefaultProbBuffer = {};
    bool         m_superFrameHucPass = false;  //!< Huc super frame pass enable/disabl flags

 MEDIA_CLASS_DEFINE_END(encode__Vp9HpuPkt)
};

}  // namespace encode

#endif
