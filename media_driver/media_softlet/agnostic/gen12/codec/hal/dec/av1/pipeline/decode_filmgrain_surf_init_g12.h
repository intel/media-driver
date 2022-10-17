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
//! \file     decode_filmgrain_surf_init_g12.h
//! \brief    Defines the common interface for decode input bitstream
//! \details  Defines the interface to handle the decode input bitstream in
//!           both single execution call mode and multiple excution call mode.
//!

#ifndef __DECODE_FILMGRAIN_SURF_INIT_H__
#define __DECODE_FILMGRAIN_SURF_INIT_H__

#include "decode_av1_pipeline_g12.h"
#include "decode_av1_filmgrain_feature_g12.h"
#include "decode_huc_copy_packet_g12.h"


namespace decode {

class FilmGrainSurfaceInit : public DecodeSubPipeline
{
public:
    //!
    //! \brief  AV1 Decode Film Grain constructor
    //!
    FilmGrainSurfaceInit(DecodePipeline *pipeline, MediaTask *task, uint8_t numVdbox, CodechalHwInterface* hwInterface);

    //!
    //! \brief  AV1 Decode Film Grain destructor
    //!
    virtual ~FilmGrainSurfaceInit();

    //!
    //! \brief  Initialize the bitstream context
    //!
    //! \param  [in] settings
    //!         Reference to the Codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(CodechalSetting &settings) override;

    //!
    //! \brief  Prepare interal parameters
    //! \param  [in] params
    //!         Reference to decode pipeline parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Prepare(DecodePipelineParams &params) override;

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
    //! \brief  Film Grain Surface Init
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitCoordinateSurface();

    //!
    //! \brief  Initialize scalability parameters
    //!
    virtual void InitScalabilityPars(PMOS_INTERFACE osInterface) override;

private:
    Av1DecodeFilmGrainG12 * m_filmGrainFeature   = nullptr;        //!< Film Grain feature
    DecodeAllocator *       m_allocator            = nullptr;        //!< Resource allocator
    HucCopyPktG12 *         m_surfInitPkt          = nullptr;        //!< Surface init packet
    PMOS_BUFFER             m_tmpInitBuf           = nullptr;        //!< Init buffer filled with 0
    CodechalHwInterface    *m_hwInterface          = nullptr;
    MEDIA_CLASS_DEFINE_END(decode__FilmGrainSurfaceInit)
};
}  // namespace decode
#endif
