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
//! \file     decode_vvc_reference_frames.h
//! \brief    Defines reference list related logic for VVC decode
//!
#ifndef __DECODE_VVC_REFERENCE_FRAMES_H__
#define __DECODE_VVC_REFERENCE_FRAMES_H__

#include "codec_def_decode_vvc.h"
#include "mhw_vdbox.h"
#include "decode_allocator.h"

namespace decode
{
class VvcBasicFeature;

class VvcReferenceFrames
{
public:
    //!
    //! \brief  VvcReferenceFrames constructor
    //!
    VvcReferenceFrames();

    //!
    //! \brief  VvcReferenceFrames deconstructor
    //!
    ~VvcReferenceFrames();

    //!
    //! \brief  Init Vvc reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Vvc basic feature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(VvcBasicFeature *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CodecVvcPicParams & picParams);

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  std::vector<uint8_t> &
    //!         Active reference list indices for current frame
    //!
    const std::vector<uint8_t> &GetActiveReferenceList(CodecVvcPicParams &picParams);

    //!
    //! \brief  Get frame surface with index equals to frameIndex
    //! \param  [in] frameIndex
    //!         Frame index for reference
    //! \return  PMOS_RESOURCE
    //!         The returned frame surface coresponding to frameIndex
    //!
    PMOS_RESOURCE GetReferenceByFrameIndex(uint8_t frameIndex);

    //!
    //! \brief  Get reference picture attributes
    //! \param  [in] frameIndex
    //!         Frame index for reference
    //! \return  PMOS_RESOURCE
    //!         Attributes for the frame of frameIndex
    //!
    MOS_STATUS GetRefAttrByFrameIndex(uint8_t frameIndex, VvcRefFrameAttributes* attributes);

    //!
    //! \brief  Get valid reference for error concealment
    //! \return  PMOS_RESOURCE
    //!         Valid reference resource
    //!
    PMOS_RESOURCE GetValidReference();

    //!
    //! \brief  Get frame index of the valid reference for error concealment.
    //! \return  PMOS_RESOURCE
    //!         Valid reference resource
    //!
    uint8_t GetValidReferenceFrameIdx() { return m_validRefFrameIdx; }

    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS             UpdateCurFrame(const CodecVvcPicParams &picParams);

    //!
    //! \brief  Update the unavailable ref frames entries on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS             UpdateUnavailableRefFrames(const CodecVvcPicParams &picParams);

    //!
    //! \brief  Calculate RPR constraints active flag
    //! \param  [in] refFrameIdx
    //!         Frame index for reference
    //! \param  [in] flag
    //!         The calculation result
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS             CalcRprConstraintsActiveFlag(uint8_t refFrameIdx, bool& flag);

    //!
    //! \brief  Update the current resource for currnet ref list
    //! \param  [in] pCurRefList
    //!         pointer of current ref list
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS             UpdateCurResource(const PCODEC_REF_LIST_VVC pCurRefList);

    PCODEC_REF_LIST_VVC    m_refList[CODEC_MAX_DPB_NUM_VVC];            //!< Pointer to reference list, actually the DPB
    PCODEC_REF_LIST_VVC    m_currRefList = nullptr;                     //!< Current frame reference list
    uint8_t                m_validRefFrameIdx = CODEC_MAX_DPB_NUM_VVC;  //!< the frame index for the valid reference for error concealment
    bool                   m_curIsIntra       = true;                   //!< Indicate current picture is intra

protected:
    VvcBasicFeature        *m_basicFeature = nullptr;  //!< VVC basic feature
    DecodeAllocator        *m_allocator    = nullptr;  //!< Decode allocator
    std::vector<uint8_t>   m_activeReferenceList;      //!< Active reference list of current picture

MEDIA_CLASS_DEFINE_END(decode__VvcReferenceFrames)
};

}  // namespace decode

#endif  // !__DECODE_VVC_REFERENCE_FRAMES_H__
