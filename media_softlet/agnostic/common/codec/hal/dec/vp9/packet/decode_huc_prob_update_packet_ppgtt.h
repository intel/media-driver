/*
* Copyright (c) 2024-2026, Intel Corporation
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
//! \file     decode_huc_prob_update_packet_ppgtt.h
//! \brief    Defines the implementation of huc prob update ppgtt packet for VP9 decode
//!

#ifndef __DECODE_HUC_PROB_UPDATE_PACKET_PPGTT_H__
#define __DECODE_HUC_PROB_UPDATE_PACKET_PPGTT_H__

#include "decode_huc_prob_update_packet.h"
#include "mhw_vdbox_huc_ppgtt_itf.h"
#include "huc_kernel_source.h"

namespace decode
{

class HucVp9ProbUpdatePktPpgtt : public HucVp9ProbUpdatePkt, public mhw::vdbox::huc::ItfPPGTT::ParSetting
{
public:
    HucVp9ProbUpdatePktPpgtt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : HucVp9ProbUpdatePkt(pipeline, task, hwInterface){}

    virtual ~HucVp9ProbUpdatePktPpgtt();

    //!
    //! \brief  Initialize the media packet, allocate required resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init() override;

    //!
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "VP9_PROB_UPDATE_PPGTT";
    }

protected:

    virtual MOS_STATUS AllocateResources() override;

    virtual MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MOS_STATUS AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer) override;

    virtual MHW_SETPAR_DECL_HDR(HUC_IMEM_ADDR);

    HucKernelSource *m_hucKernelSource = nullptr;

    MOS_BUFFER *m_kernelBinBuffer = nullptr;

    bool m_isPpgttMode = false;

    MEDIA_CLASS_DEFINE_END(decode__HucVp9ProbUpdatePktPpgtt)
};

}  // namespace decode
#endif

