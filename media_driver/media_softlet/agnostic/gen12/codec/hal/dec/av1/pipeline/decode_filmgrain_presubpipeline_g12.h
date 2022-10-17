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
//! \file     decode_filmgrain_presubpipeline_g12.h
//! \brief    Defines the common interface for decode input bitstream
//! \details  Defines the interface to handle the decode input bitstream in
//!           both single execution call mode and multiple excution call mode.
//!

#ifndef __DECODE_FILMGRAIN_PRESUBPIPELINE_G12_H__
#define __DECODE_FILMGRAIN_PRESUBPIPELINE_G12_H__

#include "decode_av1_pipeline_g12.h"
#include "decode_filmgrain_gennoise_grv_packet_g12.h"
#include "decode_filmgrain_gennoise_rp1_packet_g12.h"
#include "decode_filmgrain_gennoise_rp2_packet_g12.h"
#include "decode_av1_filmgrain_feature_g12.h"

namespace decode {
class FilmGrainGrvPacket;

class FilmGrainPreSubPipeline : public DecodeSubPipeline
{
public:
    //!
    //! \brief  AV1 Decode Film Grain constructor
    //!
    FilmGrainPreSubPipeline(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface *hwInterface);

    //!
    //! \brief  AV1 Decode Film Grain destructor
    //!
    virtual ~FilmGrainPreSubPipeline(){};

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

protected:
    //!
    //! \brief  Reset the bitstream context for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Begin();

    //!
    //! \brief  Film Grain Generate Noise
    //! \param  [in] decodeParams
    //!         Reference to codechal decode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DoFilmGrainGenerateNoise(const CodechalDecodeParams &decodeParams);

    //!
    //! \brief  Film Grain Get Random Values kernel
    //! \param  [in] decodeParams
    //!         Reference to codechal decode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetRandomValuesKernel(const CodechalDecodeParams &decodeParams);

    //!
    //! \brief  Film Grain RegressPhase1 kernel
    //! \param  [in] decodeParams
    //!         Reference to codechal decode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegressPhase1Kernel(const CodechalDecodeParams &decodeParams);

    //!
    //! \brief  Film Grain RegressPhase2 kernel
    //! \param  [in] decodeParams
    //!         Reference to codechal decode parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RegressPhase2Kernel(const CodechalDecodeParams &decodeParams);

    //!
    //! \brief  Initialize scalability parameters
    //!
    virtual void InitScalabilityPars(PMOS_INTERFACE osInterface) override;

private:
    DecodeBasicFeature*  m_basicFeature         = nullptr;        //!< Decode basic feature
    DecodeAllocator *    m_allocator            = nullptr;        //!< Resource allocator

    FilmGrainGrvPacket *    m_filmGrainGrvPkt   = nullptr;        //!< GetRandomValues kenrel packet
    FilmGrainRp1Packet *    m_filmGrainRp1Pkt   = nullptr;        //!< RegressPhase1 kernel packet
    FilmGrainRp2Packet *    m_filmGrainRp2Pkt   = nullptr;        //!< RegressPhase2 kernel packet
    Av1DecodeFilmGrainG12 * m_filmGrainFeature  = nullptr;        //!< Film Grain feature
    CodechalHwInterface    *m_hwInterface       = nullptr;
MEDIA_CLASS_DEFINE_END(decode__FilmGrainPreSubPipeline)
};

}  // namespace decode

#endif
