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
//! \file     decode_avc_reference_frames.h
//! \brief    Defines reference list related logic for avc decode
//!
#ifndef __DECODE_AVC_REFERENCE_FRAMES_H__
#define __DECODE_AVC_REFERENCE_FRAMES_H__

#include "codec_def_decode_avc.h"
#include "mhw_vdbox.h"
#include "decode_allocator.h"

namespace decode
{
class AvcBasicFeature;

class AvcReferenceFrames
{
public:
    //!
    //! \brief  AvcReferenceFrames constructor
    //!
    AvcReferenceFrames();

    //!
    //! \brief  AvcReferenceFrames deconstructor
    //!
    ~AvcReferenceFrames();

    //!
    //! \brief  Init Avc reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Avc basic feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(AvcBasicFeature *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CODEC_AVC_PIC_PARAMS & picParams);

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return std::vector<uint8_t> &
    //!         Active reference list indices for current frame
    //!
    const std::vector<uint8_t> & GetActiveReferenceList(const CODEC_AVC_PIC_PARAMS & picParams);

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] frameIndex
    //!         Frame index for reference
    //! \return PMOS_RESOURCE
    //!         Active reference list for current frame
    //!
    PMOS_RESOURCE GetReferenceByFrameIndex(uint8_t frameIndex);

    //!
    //! \brief  Get valid reference for error concealment.
    //! \return PMOS_RESOURCE
    //!         Valid reference resource
    //!
    PMOS_RESOURCE GetValidReference();

    //!
    //! \brief  Update current resource for reference list
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurResource(const CODEC_AVC_PIC_PARAMS &picParams);

    CODEC_PIC_ID        m_avcPicIdx[CODEC_AVC_MAX_NUM_REF_FRAME];          //!< Picture Index
    PCODEC_REF_LIST     m_refList[CODEC_AVC_NUM_UNCOMPRESSED_SURFACE];     //!< Pointer to reference list

protected:
    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurFrame(const CODEC_AVC_PIC_PARAMS & picParams);

    //!
    //! \brief  Update the reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurRefList(const CODEC_AVC_PIC_PARAMS & picParams);

    //!
    //! \brief  Update the reference cache policy
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateRefCachePolicy(const CODEC_AVC_PIC_PARAMS &picParams);

    //!
    //! \brief    Set frame store Id for avc decode.
    //! \details
    //! \param    [in] frameIdx
    //!           frame index
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetFrameStoreIds(uint8_t frameIdx);

    AvcBasicFeature*         m_basicFeature = nullptr;                       //!< AVC basic feature
    DecodeAllocator*         m_allocator    = nullptr;                       //!< Decode allocator
    std::vector<uint8_t>     m_activeReferenceList;                          //!< Active reference list of current picture
    CODEC_PICTURE            m_prevPic;                                      //!< Current Picture Struct
    CODEC_AVC_FRAME_STORE_ID m_avcFrameStoreId[CODEC_AVC_MAX_NUM_REF_FRAME]; //!< Avc Frame Store ID
    PMOS_INTERFACE           m_osInterface = nullptr;                        //!< Os interface

MEDIA_CLASS_DEFINE_END(decode__AvcReferenceFrames)
};

}  // namespace decode

#endif  // !__DECODE_AVC_REFERENCE_FRAMES_H__
