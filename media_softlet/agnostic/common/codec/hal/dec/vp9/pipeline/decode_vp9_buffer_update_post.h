/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     decode_vp9_buffer_update_post.h
//! \brief    Defines the interface for vp9 decode post-processing buffer update sub pipeline
//! \details  Defines the interface to handle the post-decode buffer update operations
//!           for VP9 decode pipeline when mismatch order programming is enabled
//!

#ifndef __DECODE_VP9_BUFFER_UPDATE_POST_H__
#define __DECODE_VP9_BUFFER_UPDATE_POST_H__

#include "decode_packet_id.h"
#include "decode_resource_array.h"
#include "decode_sub_pipeline.h"
#include "codec_def_decode.h"
#include "media_feature_manager.h"
#include "decode_vp9_pipeline.h"
#include "decode_huc_packet_creator_base.h"

namespace decode {

class DecodeVp9BufferUpdatePost : public DecodeSubPipeline
{
public:
    //!
    //! \brief  Decode VP9 buffer update post constructor
    //!
    DecodeVp9BufferUpdatePost(Vp9Pipeline* pipeline, MediaTask* task, uint8_t numVdbox);

    //!
    //! \brief  Decode VP9 buffer update post destructor
    //!
    virtual ~DecodeVp9BufferUpdatePost();

    //!
    //! \brief  Initialize the buffer update post sub pipeline
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

protected:
    //!
    //! \brief  Reset the buffer update post context for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Begin();

    //!
    //! \brief  Initialize scalability parameters
    //!
    virtual void InitScalabilityPars(PMOS_INTERFACE osInterface) override;

    MOS_STATUS AllocateTempBuffer();
    MOS_STATUS CtxBufDiffInit(uint8_t *ctxBuffer, bool setToKey);

protected:
    Vp9BasicFeature  *m_basicFeature   = nullptr; //!< Vp9 basic feature
    DecodeAllocator  *m_allocator      = nullptr; //!< Resource allocator
    HucCopyPktItf    *m_probbufferResetPostPkt = nullptr;  //!< Segment id reset packet
    PMOS_BUFFER       m_tempDefaultProbBuffer  = nullptr;
    PMOS_BUFFER       m_tempFrameTypeKeyBuffer = nullptr;
    PMOS_BUFFER       m_tempFrameTypeNonKeyBuffer = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodeVp9BufferUpdatePost)
};

}  // namespace decode

#endif  // !__DECODE_VP9_BUFFER_UPDATE_POST_H__