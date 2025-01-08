/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_reference_frames_g12.h
//! \brief    Defines reference list related logic for av1 decode
//!
#ifndef __DECODE_AV1_REFERENCE_FRAMES_G12_H__
#define __DECODE_AV1_REFERENCE_FRAMES_G12_H__

#include "codec_def_decode_av1.h"
#include "mhw_vdbox.h"
#include "decode_allocator.h"

namespace decode
{
class Av1BasicFeatureG12;

class Av1ReferenceFramesG12
{
public:
    //!
    //! \brief  Av1ReferenceFramesG12 constructor
    //!
    Av1ReferenceFramesG12();

    //!
    //! \brief  Av1ReferenceFramesG12 deconstructor
    //!
    ~Av1ReferenceFramesG12();

    //!
    //! \brief  Init Av1 reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Av1 basic feature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(Av1BasicFeatureG12 *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CodecAv1PicParams & picParams);

    //!
    //! \brief  Insert one anchor frame for Large Scale Tile decoding
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InsertAnchorFrame(CodecAv1PicParams & picParams);

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] tileParams
    //!         Tile parameters
    //! \return  std::vector<uint8_t> &
    //!         Active reference list indices for current frame
    //!
    const std::vector<uint8_t> &GetActiveReferenceList(CodecAv1PicParams &picParams, CodecAv1TileParams &tileParams);

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

    //!
    //! \brief  Get valid reference index for error concealment.
    //! \param  [out] uint8_t
    //!         Valid reference resource index
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS GetValidReferenceIndex(uint8_t *validRefIndex);

    //! \brief    Identify the first nearest reference frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Identify1stNearRef(const CodecAv1PicParams & picParams,
        int32_t curFrameOffset,int32_t* refFrameOffset,int32_t* refIdx);

    //! \brief    Identify the second nearest reference frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Identify2ndNearRef(const CodecAv1PicParams & picParams,
        int32_t curFrameOffset,int32_t* refFrameOffset,int32_t* refIdx);

    //!
    //! \brief    Calculate the relative distance between a and b
    //! \return   Distance
    //!           Relative distance between a and b
    //!
    int32_t GetRelativeDist(const CodecAv1PicParams & picParams, int32_t a, int32_t b);

    //!
    //! \brief    Setup Motion Field for each reference
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupMotionFieldProjection(CodecAv1PicParams & picParams);

    //!
    //! \brief    Setup Motion Field for each reference
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    bool MotionFieldProjection(CodecAv1PicParams & picParams, int32_t ref, int32_t dir);

    //!
    //! \brief    Check Segmentation for primary reference frame
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    bool CheckSegForPrimFrame(CodecAv1PicParams & picParams);

    //!
    //! \brief    Get Primary Reference Frame Index
    //! \return   Frame Index
    //!           Primary Reference Frame Index
    //!
    uint8_t GetPrimaryRefIdx() {return m_prevFrameIdx;};

    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurFrame(const CodecAv1PicParams &picParams);

    //!
    //! \brief  Error detect and concealment for reference list for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ErrorConcealment(CodecAv1PicParams &picParams);

    //!
    //! \brief  Update the reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurRefList(const CodecAv1PicParams &picParams);

    //!
    //! \brief  Update the current resource for currnet ref list
    //! \param  [in] pCurRefList
    //!         pointer of current ref list
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS             UpdateCurResource(const PCODEC_REF_LIST_AV1 pCurRefList);

    PCODEC_REF_LIST_AV1    m_refList[CODECHAL_MAX_DPB_NUM_LST_AV1]; //!< Pointer to reference list, actually the DPB
    PCODEC_REF_LIST_AV1    m_currRefList = nullptr;                 //!< Current frame reference list

protected:

    uint8_t m_prevFrameIdx = 0;                    //!< primary reference frame index
    Av1BasicFeatureG12 *m_basicFeature = nullptr;  //!< AV1 basic feature
    DecodeAllocator *m_allocator = nullptr;        //!< Decode allocator
    std::vector<uint8_t> m_activeReferenceList;    //!< Active reference list of current picture
    PMOS_INTERFACE       m_osInterface = nullptr;  //!< Os interface
MEDIA_CLASS_DEFINE_END(decode__Av1ReferenceFramesG12)
};

}  // namespace decode

#endif  // !__DECODE_AV1_REFERENCE_FRAMES_G12_H__
