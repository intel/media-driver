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
//! \file     encode_jpeg_reference_frames.h
//! \brief    Defines reference list related logic for encode jpeg
//!
#ifndef __ENCODE_JPEG_REFERENCE_FRAMES_H__
#define __ENCODE_JPEG_REFERENCE_FRAMES_H__

#include "mhw_vdbox.h"

namespace encode
{

class JpegBasicFeature;

class JpegReferenceFrames
{
public:

    //!
    //! \brief  JpegReferenceFrames constructor
    //!
    JpegReferenceFrames() {}

    //!
    //! \brief  JpegReferenceFrames deconstructor
    //!
    virtual ~JpegReferenceFrames();

    //!
    //! \brief  Initialize reference frame
    //! \param  [in] params
    //!         Pointer to JpegBasicFeature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(JpegBasicFeature *basicFeature);

    //!
    //! \brief  Get reference list
    //! \return  PCODEC_REF_LIST *
    //!         Pointer of current reference list
    //!
    PCODEC_REF_LIST *GetRefList() { return m_refList; }

    //!
    //! \brief  Get current reference list
    //! \return  PCODEC_REF_LIST
    //!         Pointer of current reference list
    //!
    PCODEC_REF_LIST GetCurrRefList() { return m_currRefList; }

    //!
    //! \brief  Update reference frame for picture
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture();

protected:

    PCODEC_REF_LIST   m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_JPEG] = {}; //!< Pointer to reference pictures
    PCODEC_REF_LIST   m_currRefList = nullptr;                                //!< Current reference list

    JpegBasicFeature *m_basicFeature = nullptr;                               //!<  JPEG paramter

MEDIA_CLASS_DEFINE_END(encode__JpegReferenceFrames)
};

}  // namespace encode

#endif  // !__ENCODE_JPEG_REFERENCE_FRAMES_H__
