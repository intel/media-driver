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
//! \file     encode_avc_reference_frames.h
//! \brief    Defines reference list related logic for encode avc
//!
#ifndef __ENCODE_AVC_REFERENCE_FRAMES_H__
#define __ENCODE_AVC_REFERENCE_FRAMES_H__

#include "codec_def_encode_avc.h"
#include "mhw_vdbox.h"
#include "encode_allocator.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_mfx_itf.h"

namespace encode
{

class AvcBasicFeature;

class AvcReferenceFrames : public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::mfx::Itf::ParSetting
{
public:

    //!
    //! \brief  AvcReferenceFrames constructor
    //!
    AvcReferenceFrames() {}

    //!
    //! \brief  AvcReferenceFrames deconstructor
    //!
    virtual ~AvcReferenceFrames();

    //!
    //! \brief  Initialize reference frame
    //! \param  [in] params
    //!         Pointer to AvcBasicFeature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(AvcBasicFeature *basicFeature, EncodeAllocator *allocator);

    //!
    //! \brief  Update reference frame for picture
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicture();

    //!
    //! \brief  Update reference frame for slice
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateSlice();

    //!
    //! \brief  Get picture index
    //! \param  [in] idx
    //!         Index of pic index
    //! \return  CODEC_PIC_ID
    //!         CODEC_PIC_ID refer to the picure index
    //!
    PCODEC_PIC_ID GetPicIndex() { return m_picIdx; };

    //!
    //! \brief  Get reference list index
    //! \param  [in] idx
    //!         Index of pic index
    //! \return  uint8_t
    //!         reference list index
    //!
    uint8_t GetRefListIndex(uint8_t idx) { return m_picIdx[idx].ucPicIdx; };

    //!
    //! \brief  Get current reference list
    //! \return  PCODEC_REF_LIST
    //!         Pointer of current reference list
    //!
    PCODEC_REF_LIST GetCurrRefList() { return m_currRefList; };

    //!
    //! \brief  Get reference list
    //! \return  PCODEC_REF_LIST *
    //!         Pointer of current reference list
    //!
    PCODEC_REF_LIST *GetRefList() { return m_refList; };

    //!
    //! \brief  Get picture coding type
    //! \return  uint16_t
    //!         Picture coding type
    //!
    uint16_t GetPictureCodingType() { return m_pictureCodingType; };

    //!
    //! \brief    Get bidirectional weight
    //!
    //! \return   int32_t
    //!           Bidirectional weight
    //!
    int32_t GetBiWeight() { return m_biWeight; };

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_AVC_IMG_STATE);

    MHW_SETPAR_DECL_HDR(MFX_PIPE_BUF_ADDR_STATE);

protected:

    //!
    //! \brief    Set frame store Id for avc codec.
    //! \details
    //! \return   frameIdx
    //!           [in] frame index
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetFrameStoreIds(uint8_t frameIdx);

    //!
    //! \brief    Validate reference list L0 and L1.
    //!
    //! \param    [in] params
    //!           Pointer to CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateNumReferences(PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params);

    //!
    //! \brief    Get Dist Scale factor
    //!
    //! \return   void
    //!
    void GetDistScaleFactor();

    //!
    //! \brief    Get bidirectional weight
    //!
    //! \param    [in] distScaleFactorRefID0List0
    //!           DistScaleFactorRefID0List0
    //! \param    [in] weightedBiPredIdc
    //!           Same as AVC syntax element.
    //!
    //! \return   int32_t
    //!           Bidirectional weight
    //!
    int32_t GetBiWeight(
        uint32_t distScaleFactorRefID0List0,
        uint16_t weightedBiPredIdc);

    uint16_t                   m_pictureCodingType = 0;                                //!< I, P, or B frame
    CODEC_PIC_ID               m_picIdx[CODEC_AVC_MAX_NUM_REF_FRAME] = {};             //!< Picture index
    PCODEC_REF_LIST            m_refList[CODEC_AVC_NUM_UNCOMPRESSED_SURFACE] = {};     //!< Pointer to reference pictures
    CODEC_AVC_FRAME_STORE_ID   m_frameStoreID[CODEC_AVC_MAX_NUM_REF_FRAME] = {};             //!< Refer to CODEC_AVC_FRAME_STORE_ID
    PCODEC_REF_LIST            m_currRefList = nullptr;                                //!< Current reference list

    // B-frame
    uint32_t                   m_distScaleFactorList0[CODEC_AVC_MAX_NUM_REF_FRAME * 2] = {};  //!< the DistScaleFactor used to derive temporal direct motion vector
    uint32_t                   m_biWeight = 0;                     //!< Bidirectional Weight

    AvcBasicFeature *m_basicFeature = nullptr;  //!<  AVC paramter
    EncodeAllocator *m_allocator    = nullptr;  //!< Encode allocator

MEDIA_CLASS_DEFINE_END(encode__AvcReferenceFrames)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_REFERENCE_FRAMES_H__
