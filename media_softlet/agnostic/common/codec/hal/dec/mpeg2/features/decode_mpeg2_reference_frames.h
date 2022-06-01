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
//! \file     decode_mpeg2_reference_frames.h
//! \brief    Defines reference list related logic for mpeg2 decode
//!
#ifndef __DECODE_MPEG2_REFERENCE_FRAMES_H__
#define __DECODE_MPEG2_REFERENCE_FRAMES_H__

#include "codec_def_decode_mpeg2.h"
#include "mhw_vdbox.h"
#include "decode_allocator.h"

namespace decode
{
class Mpeg2BasicFeature;

class Mpeg2ReferenceFrames
{
public:
    //!
    //! \brief  Mpeg2ReferenceFrames constructor
    //!
    Mpeg2ReferenceFrames();

    //!
    //! \brief  Mpeg2ReferenceFrames deconstructor
    //!
    ~Mpeg2ReferenceFrames();

    //!
    //! \brief  Init Mpeg2 reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Mpeg2 basic feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(Mpeg2BasicFeature *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CodecDecodeMpeg2PicParams &picParams);

    //!
    //! \brief  Update current resource for reference list
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurResource(const CodecDecodeMpeg2PicParams &picParams);

    PCODEC_REF_LIST     m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2];     //!< Pointer to reference list

protected:
    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurFrame(const CodecDecodeMpeg2PicParams &picParams);

    //!
    //! \brief  Update the reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurRefList(const CodecDecodeMpeg2PicParams &picParams);

    Mpeg2BasicFeature*       m_basicFeature = nullptr;                       //!< MPEG2 basic feature
    DecodeAllocator*         m_allocator    = nullptr;                       //!< Decode allocator
    std::vector<uint8_t>     m_activeReferenceList;                          //!< Active reference list of current picture

MEDIA_CLASS_DEFINE_END(decode__Mpeg2ReferenceFrames)
};

}  // namespace decode

#endif  // !__DECODE_MPEG2_REFERENCE_FRAMES_H__
