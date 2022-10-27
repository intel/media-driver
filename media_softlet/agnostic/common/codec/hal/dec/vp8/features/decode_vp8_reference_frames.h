/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_vp8_reference_frames.h
//! \brief    Defines reference list related logic for vp8 decode
//!
#ifndef __DECODE_VP8_REFERENCE_FRAMES_H__
#define __DECODE_VP8_REFERENCE_FRAMES_H__

#include "codec_def_decode_vp8.h" 
#include "mhw_vdbox.h"
#include "decode_allocator.h"
#include "decode_vp8_basic_feature.h"

namespace decode
{
class Vp8BasicFeature;

class Vp8ReferenceFrames
{
public:
    //!
    //! \brief  Vp8ReferenceFrames constructor
    //!
    Vp8ReferenceFrames();

    //!
    //! \brief  Vp8ReferenceFrames deconstructor
    //!
    ~Vp8ReferenceFrames();

    //!
    //! \brief  Init Vp8 reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Vp8 basic feature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(Vp8BasicFeature *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CODEC_VP8_PIC_PARAMS &picParams);


    PCODEC_REF_LIST m_vp8RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP8];  //!< Pointer to reference list
    PCODEC_REF_LIST m_currRefList = nullptr;                              //!< Current frame reference list

  protected:
    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurFrame(const CODEC_VP8_PIC_PARAMS &picParams);

    Vp8BasicFeature *m_basicFeature = nullptr;  //!< vp8 basic feature
    DecodeAllocator *m_allocator    = nullptr;  //!< Decode allocator
    std::vector<uint8_t> m_activeReferenceList; //!< Active reference list of current picture   

  MEDIA_CLASS_DEFINE_END(decode__Vp8ReferenceFrames)
};

}  // namespace decode

#endif  // !__DECODE_VP8_REFERENCE_FRAMES_H__
