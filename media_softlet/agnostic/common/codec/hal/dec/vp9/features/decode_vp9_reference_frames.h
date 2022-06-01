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
//! \file     decode_vp9_reference_frames.h
//! \brief    Defines reference list related logic for vp9 decode
//!
#ifndef __DECODE_VP9_REFERENCE_FRAMES_H__
#define __DECODE_VP9_REFERENCE_FRAMES_H__

#include "codec_def_decode_vp9.h" 
#include "mhw_vdbox.h"
#include "decode_allocator.h"

namespace decode
{
class Vp9BasicFeature;

class Vp9ReferenceFrames
{
public:
    //!
    //! \brief  Vp9ReferenceFrames constructor
    //!
    Vp9ReferenceFrames();

    //!
    //! \brief  Vp9ReferenceFrames deconstructor
    //!
    ~Vp9ReferenceFrames();

    //!
    //! \brief  Init Vp9 reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Vp9 basic feature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(Vp9BasicFeature *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CODEC_VP9_PIC_PARAMS &picParams); // use CODEC_VP9_PIC_PARAMS? CodecAv1PicParams

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  std::vector<uint8_t> &
    //!         Active reference list indices for current frame
    //!
    const std::vector<uint8_t> &GetActiveReferenceList(CODEC_VP9_PIC_PARAMS &picParams);

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] frameIndex
    //!         Frame index for reference
    //! \return  PMOS_RESOURCE
    //!         Active reference list for current frame
    //!
    PMOS_RESOURCE GetReferenceByFrameIndex(uint8_t frameIndex);

    //!
    //! \brief  Get valid reference for error concealment.
    //! \return  PMOS_RESOURCE
    //!         Valid reference resource
    //!
    PMOS_RESOURCE GetValidReference();  
    
    MOS_STATUS UpdateCurResource(const CODEC_VP9_PIC_PARAMS &picParams);

    //!
    //! \brief    Get Primary Reference Frame Index
    //! \return   Frame Index
    //!           Primary Reference Frame Index
    //!
    uint8_t GetPrimaryRefIdx() {return m_prevFrameIdx;};

    PCODEC_REF_LIST m_vp9RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9];  //!< Pointer to reference list
    PCODEC_REF_LIST m_currRefList = nullptr;                              //!< Current frame reference list
                                                                              //???ADD BY LI
  protected:
    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurFrame(const CODEC_VP9_PIC_PARAMS &picParams);

    uint8_t m_prevFrameIdx = 0;                 //!< primary reference frame index
    Vp9BasicFeature *m_basicFeature = nullptr;  //!< vp9 basic feature
    DecodeAllocator *m_allocator    = nullptr;  //!< Decode allocator
    std::vector<uint8_t> m_activeReferenceList; //!< Active reference list of current picture   

  MEDIA_CLASS_DEFINE_END(decode__Vp9ReferenceFrames)
};

}  // namespace decode

#endif  // !__DECODE_VP9_REFERENCE_FRAMES_H__
