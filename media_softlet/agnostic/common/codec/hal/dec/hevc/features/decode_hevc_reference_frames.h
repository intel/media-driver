/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_hevc_reference_frames.h
//! \brief    Defines reference list related logic for hevc decode
//!
#ifndef __DECODE_HEVC_REFERENCE_FRAMES_H__
#define __DECODE_HEVC_REFERENCE_FRAMES_H__

#include "codec_def_decode_hevc.h"
#include "mhw_vdbox.h"
#include "decode_allocator.h"

namespace decode
{
class HevcBasicFeature;

class HevcReferenceFrames
{
public:
    //!
    //! \brief  HevcReferenceFrames constructor
    //!
    HevcReferenceFrames();

    //!
    //! \brief  HevcReferenceFrames deconstructor
    //!
    ~HevcReferenceFrames();

    //!
    //! \brief  Init Hevc reference frames
    //! \param  [in] basicFeature
    //!         Pointer to Hevc basic feature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(HevcBasicFeature *basicFeature, DecodeAllocator& allocator);

    //!
    //! \brief  Update reference frames for picture
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] isSCCIBCMode
    //!         Flag to indicate SCC IBC mode
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture(CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode);

    //!
    //! \brief  Update current resource for reference list 
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] isSCCIBCMode
    //!         Flag to indicate SCC IBC mode
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurResource(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode);

    //!
    //! \brief  Get active reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  std::vector<uint8_t> &
    //!         Active reference list indices for current frame
    //!
    const std::vector<uint8_t> & GetActiveReferenceList(const CODEC_HEVC_PIC_PARAMS & picParams);

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
    //! \brief  Fix reference list for slice
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] slc
    //!         Slice parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS FixSliceRefList(const CODEC_HEVC_PIC_PARAMS & picParams, CODEC_HEVC_SLICE_PARAMS & slc);

    int8_t              m_refIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC];      //!< Map table of indices of references
    PCODEC_REF_LIST     m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];  //!< Pointer to reference list
    bool                m_curIsIntra = true;                                //!< Indicate current picture is intra
    uint8_t             m_IBCRefIdx = 0;                                    //!< Reference ID for IBC mode

protected:
    //!
    //! \brief  Update the current frame entry on m_refList
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] isSCCIBCMode
    //!         Flag to indicate SCC IBC mode
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurFrame(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode);

    //!
    //! \brief  Update the reference list for current frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] isSCCIBCMode
    //!         Flag to indicate SCC IBC mode
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateCurRefList(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode);

    //!
    //! \brief  Update the reference index mapping
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [in] isSCCIBCMode
    //!         Flag to indicate SCC IBC mode
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateRefIdxMapping(const CODEC_HEVC_PIC_PARAMS & picParams, bool isSCCIBCMode);

    //!
    //! \brief  Update the reference cache policy
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateRefCachePolicy(const CODEC_HEVC_PIC_PARAMS &picParams);

    //!
    //! \brief  Detect if current frame has refrence frame
    //! \param  [in] picParams
    //!         Picture parameters
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    bool IsCurFrameUseReference(const CODEC_HEVC_PIC_PARAMS & picParams);

    //!
    //! \brief  Detect POC duplication and save status to m_duplicationPocMap
    //! \param  [in] picParams
    //!         Picture parameters
    //! \param  [out] refFrameList
    //!         Reference frame list
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DetectPocDuplication(const int32_t (&picOrderCntValList)[CODEC_MAX_NUM_REF_FRAME_HEVC],
                                    CODEC_PICTURE (&refFrameList)[CODEC_MAX_NUM_REF_FRAME_HEVC]);

    static constexpr int32_t m_invalidPocValue = 0XFFFFFFFF;

    HevcBasicFeature*    m_basicFeature = nullptr;                           //!< HEVC paramter
    DecodeAllocator*     m_allocator    = nullptr;                           //!< Decode allocator

    bool                 m_frameUsedAsCurRef[CODEC_MAX_NUM_REF_FRAME_HEVC];  //!< Indicate frames used as reference of current picture
    std::vector<uint8_t> m_activeReferenceList;                             //!< Active reference list of current picture

    std::vector<int8_t>  m_duplicationPocMap[CODEC_MAX_NUM_REF_FRAME_HEVC];  //!< duplication POC map
    PMOS_INTERFACE       m_osInterface = nullptr;                            //!< Os interface

MEDIA_CLASS_DEFINE_END(decode__HevcReferenceFrames)
};

}  // namespace encode

#endif  // !__DECODE_HEVC_REFERENCE_FRAMES_H__
