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
//! \file     decode_sfc_histogram_postsubpipeline.h
//! \brief    Defines the postSubpipeline for decode SFC histogram
//! \details  Defines the postSubpipline for decode SFC histogram
//!

#ifndef __DECODE_SFC_HISTOGRAM_POSTSUBPIPELINE_H__
#define __DECODE_SFC_HISTOGRAM_POSTSUBPIPELINE_H__

#include "decode_sub_pipeline.h"
#include "decode_basic_feature.h"
#include "decode_huc_copy_packet_itf.h"
#include "decode_downsampling_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode {

class DecodeSfcHistogramSubPipeline : public DecodeSubPipeline
{
public:
    //!
    //! \brief  Decode SFC histogram constructor
    //!
    DecodeSfcHistogramSubPipeline(DecodePipeline* pipeline, MediaTask* task, uint8_t numVdbox);

    //!
    //! \brief  Decode SFC histogram destructor
    //!
    virtual ~DecodeSfcHistogramSubPipeline() {}

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
    //! \brief  
    //! \param  [in] Check it the input bitstream is complete
    //!         bitstream size
    //! \return bool
    //!         Ture if input bitstream is complete, else return false
    //!
    //bool IsComplete();

    //!
    //! \brief  Get media function for context switch
    //! \return MediaFunction
    //!         Return the media function
    //!
    MediaFunction GetMediaFunction() override;

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

    MOS_STATUS CopyHistogramToDestBuf(MOS_RESOURCE* src, MOS_RESOURCE* dest, uint32_t destOffset);

protected:
    DecodeBasicFeature*             m_basicFeature      = nullptr; //!< Decode basic feature
    DecodeAllocator*                m_allocator         = nullptr; //!< Resource allocator
    HucCopyPktItf *                 m_copyPkt           = nullptr;  //!< Bitstream concat packet
    PMOS_INTERFACE                  m_osInterface       = nullptr; //!< MOS interface
    DecodeDownSamplingFeature*      m_downsampFeature   = nullptr; //!< Downsampling feature

MEDIA_CLASS_DEFINE_END(decode__DecodeSfcHistogramSubPipeline)
};

}  // namespace decode

#endif // !<__DECODE_PROCESSING_SUPPORTED

#endif // !<__DECODE_SFC_HISTOGRAM_POSTSUBPIPELINE_H__
