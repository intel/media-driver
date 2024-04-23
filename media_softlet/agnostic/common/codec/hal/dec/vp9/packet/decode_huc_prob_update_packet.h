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
//! \file     decode_huc_prob_update_packet.h
//! \brief    Defines the implementation of huc prob update packet for VP9 decode
//!

#ifndef __DECODE_HUC_PROB_UPDATE_PACKET_H__
#define __DECODE_HUC_PROB_UPDATE_PACKET_H__

#include "media_cmd_packet.h"
#include "decode_huc.h"
#include "media_pipeline.h"
#include "codec_hw_next.h"
#include "decode_utils.h"
#include "decode_vp9_pipeline.h"
#include "decode_vp9_basic_feature.h"
#include "decode_huc_packet_creator.h"
#include "mhw_vdbox_huc_cmdpar.h"
#include "mhw_vdbox_huc_itf.h"
#include "mhw_cmdpar.h"

namespace decode
{
struct HucVp9ProbBss
{
    int32_t bSegProbCopy;      //!< seg tree and pred prob update with values from App.
    int32_t bProbSave;         //!< Save prob buffer
    int32_t bProbRestore;      //!< Restore prob buffer
    int32_t bProbReset;        //!<  1: reset; 0: not reset
    int32_t bResetFull;        //!< full reset or partial (section A) reset
    int32_t bResetKeyDefault;  //!<  reset to key or inter default
    uint8_t SegTreeProbs[7];   //!< Segment tree prob buffers
    uint8_t SegPredProbs[3];   //!< Segment predict prob buffers
};

class HucVp9ProbUpdatePkt : public DecodeHucBasic, public mhw::vdbox::huc::Itf::ParSetting
{
public:
    HucVp9ProbUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface)
        : DecodeHucBasic(pipeline, task, hwInterface){}

    virtual ~HucVp9ProbUpdatePkt();

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
    //! \brief  Get Packet Name
    //! \return std::string
    //!
    virtual std::string GetPacketName() override
    {
        return "VP9_PROB_UPDATE";
    }

    MOS_STATUS VdPipelineFlush(MOS_COMMAND_BUFFER &cmdBuffer);

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

    virtual MOS_STATUS AllocateResources() override;

    virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded) override;

    virtual MOS_STATUS PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS SetDmemBuffer();
    virtual MOS_STATUS AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer);
    MOS_STATUS AddCmd_HUC_PIPE_MODE_SELECT(MOS_COMMAND_BUFFER &cmdBuffer);

    virtual MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);
    virtual MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);
    virtual MHW_SETPAR_DECL_HDR(HUC_START);

    static constexpr uint32_t m_vdboxHucVp9ProbUpdateKernelDescriptor = 6;     //!< Huc Vp9 prob update kernel descriptor
    static constexpr uint32_t m_numVp9ProbUpdateDmem                  = 8;     //!< Huc Vp9 prob update Dmem number
    static constexpr uint32_t m_mediaResetCounter                     = 2400;  //!< Media reset counter for Huc Vp9 prob update

    Vp9BasicFeature *m_vp9BasicFeature = nullptr;  //!< Pointer to vp9 basic feature

    BufferArray *m_probUpdateDmemBufferArray = nullptr;  //!< Vp9 prob update DMEM buffer array
    MOS_BUFFER * m_probUpdateDmemBuffer      = nullptr;  //!< Resource of current DMEM buffer
    uint32_t     m_dmemBufferSize            = 0;        //!< Size of DMEM buffer

    MOS_BUFFER *m_interProbSaveBuffer = nullptr;  //!< Vp9 inter prob save buffer

    uint32_t                              m_pictureStatesSize    = 0;
    uint32_t                              m_picturePatchListSize = 0;
    uint32_t                              m_sliceStatesSize      = 0;
    uint32_t                              m_slicePatchListSize   = 0;
    
    MEDIA_CLASS_DEFINE_END(decode__HucVp9ProbUpdatePkt)
};

}  // namespace decode
#endif
