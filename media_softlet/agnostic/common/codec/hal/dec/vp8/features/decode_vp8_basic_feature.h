/*
* Copyright (c) 2021-2022, Intel Corporation
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
//! \file     decode_vp8_basic_feature.h
//! \brief    Defines the common interface for decode vp8 basic feature
//!
#ifndef __DECODE_VP8_BASIC_FEATURE_H__
#define __DECODE_VP8_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_vp8.h"
#include "codec_def_vp8_probs.h"
#include "decode_vp8_reference_frames.h"
#include "decode_vp8_entropy_state.h"

namespace decode
{

class Vp8BasicFeature : public DecodeBasicFeature
{
public:
    //!
    //! \brief  Vp8BasicFeature constructor
    //!
    Vp8BasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface);

    //!
    //! \brief  Vp8BasicFeature deconstructor
    //!
    virtual ~Vp8BasicFeature();

    //!
    //! \brief  Initialize vp8 basic feature CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting) override;

    //!
    //! \brief  Update vp8 decodeParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params) override;

    MOS_STATUS SetPictureStructs(CodechalDecodeParams *decodeParams);
    
    MOS_STATUS AllocateCoefProbBuffer();

    MOS_STATUS ParseFrameHead(uint8_t* bitstreamBuffer, uint32_t bitstreamBufferSize);

    // Parameters passed by application
    uint16_t                    picWidthInMB                = 0;
    uint16_t                    picHeightInMB               = 0;

    PCODEC_VP8_PIC_PARAMS       m_vp8PicParams              = nullptr;              //!< Pointer to VP8 Pic Params
    PCODEC_VP8_SLICE_PARAMS     m_vp8SliceParams            = nullptr;              //!< Pointer to VP8 Slice Params
    PCODEC_VP8_IQ_MATRIX_PARAMS m_vp8IqMatrixParams         = nullptr;              //!< Pointer to VP8 IQ Matrix Params
    PMOS_RESOURCE               m_LastRefSurface            = nullptr;              //!< Pointer to resource of Last Reference Surface
    PMOS_RESOURCE               m_GoldenRefSurface          = nullptr;              //!< Pointer to resource of Golden Reference Surface
    PMOS_RESOURCE               m_AltRefSurface             = nullptr;              //!< Pointer to resource of Alternate Reference Surface
    PMOS_RESOURCE               m_streamOutBuffer           = nullptr;              //!< Stream out buffer from HW
    MOS_RESOURCE                m_resCoefProbBufferExternal;                        //!< Graphics resource of Coefficient Probability data surface, when m_bitstreamLockingInUse is false, pass from app
    PMOS_BUFFER                 m_resCoefProbBufferInternal = nullptr;                  //!< Graphics resource of Coefficient Probability data surface, when m_bitstreamLockingInUse is true
    PCODEC_REF_LIST             m_vp8RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8];    //!< VP8 Reference List
    uint32_t                    m_privateInputBufferSize    = 0;                        //!< Size of private surface
    uint32_t                    m_coeffProbTableOffset      = 0;                        //!< Coefficient Probability Table Offset
    uint32_t                    m_coefProbSize              = 0;                        //!< VP8 Size of the data contained in m_coefProbBuffer

    bool                        m_shortFormatInUse          = false;                    //!< Short Format Indicator
    bool                        m_deblockingEnabled         = false;                    //!< VP8 Loop Filter Enable Indicator
    bool                        m_streamOutEnabled          = false;                    //!< Stream Out enable flag
    bool                        m_bitstreamLockingInUse     = false;                    //!< Indicates whether or not the driver is expected to parse parameters from the frame header

    // VP8 Frame Head
    Vp8EntropyState                 m_vp8EntropyState;                  //!< VP8 Entropy State class to parse frame head
    CODECHAL_DECODE_VP8_FRAME_HEAD  m_vp8FrameHead;                     //!< VP8 Frame Head
    Vp8ReferenceFrames              m_refFrames;                        //!< Reference frames

protected:

    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;
    
    //! \brief OS Interface
    PMOS_INTERFACE                  m_osInterface           = nullptr;

MEDIA_CLASS_DEFINE_END(decode__Vp8BasicFeature)
};

}  // namespace decode

#endif  // !__DECODE_VP8_BASIC_FEATURE_H__
