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
//! \file     decode_vp9_buffer_update.h
//! \brief    Defines the interface for vp9 buffer update
//! \details  Defines the interface to handle the vp9 buffer update for
//!           segment id buffer and probability buffer.
//!

#ifndef __DECODE_VP9_BUFFER_UPDATE_H__
#define __DECODE_VP9_BUFFER_UPDATE_H__

#include "decode_packet_id.h"
#include "decode_resource_array.h"
#include "decode_sub_pipeline.h"
#include "codec_def_decode.h"
#include "media_feature_manager.h"
#include "decode_vp9_pipeline.h"
#include "decode_huc_packet_creator_base.h" 

namespace decode {

class DecodeVp9BufferUpdate : public DecodeSubPipeline
{
public:
    //!
    //! \brief  Decode input bitstream constructor
    //!
    DecodeVp9BufferUpdate(Vp9Pipeline* pipeline, MediaTask* task, uint8_t numVdbox);

    //!
    //! \brief  Decode input bitstream destructor
    //!
    virtual ~DecodeVp9BufferUpdate();

    //!
    //! \brief  Initialize the bitstream context
    //!
    //! \param  [in] settings
    //!         Reference to the Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(CodechalSetting& settings) override;

    //!
    //! \brief  Prepare interal parameters
    //! \param  [in] params
    //!         Reference to decode pipeline parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(DecodePipelineParams& params) override;

    //!
    //! \brief  Get media function for context switch
    //! \return MediaFunction
    //!         Return the media function
    //!
    MediaFunction GetMediaFunction() override;

    DeclareDecodePacketId(HucVp9ProbUpdatePktId);
   
protected:
    //!
    //! \brief  Reset the bitstream context for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Begin();

    //!
    //! \brief  Initialize scalability parameters
    //!
    virtual void InitScalabilityPars(PMOS_INTERFACE osInterface) override;

    //!
    //! \brief  Allocate segment id and probability init buffer
    //! \param  [in] allocSize
    //!         Allocate size for segment id init buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateSegmentInitBuffer(uint32_t allocSize);

    MOS_STATUS ProbBufFullUpdatewithDrv();
    MOS_STATUS ProbBufferPartialUpdatewithDrv();
    MOS_STATUS ContextBufferInit(uint8_t *ctxBuffer, bool setToKey);
    MOS_STATUS CtxBufDiffInit(uint8_t *ctxBuffer, bool setToKey);

protected:
    Vp9BasicFeature  *m_basicFeature   = nullptr; //!< Vp9 basic feature
    DecodeAllocator  *m_allocator      = nullptr; //!< Resource allocator

    HucCopyPktItf     *m_sgementbufferResetPkt = nullptr;  //!< Segment id reset packet
    PMOS_BUFFER        m_segmentInitBuffer     = nullptr; //!< Segment id init buffer

MEDIA_CLASS_DEFINE_END(decode__DecodeVp9BufferUpdate)
};

}

#endif
