/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_hevc_reference_frames.h
//! \brief    Defines reference list related logic for encode hevc
//!
#ifndef __ENCODE_HEVC_REFERENCE_FRAMES_H__
#define __ENCODE_HEVC_REFERENCE_FRAMES_H__

#include "codec_def_encode_hevc.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_hcp_itf.h"
#include "encode_mem_compression.h"

namespace encode
{
class HevcBasicFeature;

class HevcReferenceFrames : public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::hcp::Itf::ParSetting
{
public:

    //!
    //! \brief  HevcReferenceFrames constructor
    //!
    HevcReferenceFrames() {};

    //!
    //! \brief  HevcReferenceFrames deconstructor
    //!
    ~HevcReferenceFrames();

    //!
    //! \brief  Initialize reference frame
    //! \param  [in] params
    //!         Pointer to HevcBasicFeature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(HevcBasicFeature *basicFeature, EncodeAllocator *allocator);

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
    //! \brief  Update pic params rollingI reference location.
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateRollingIReferenceLocation();

    //!
    //! \brief  Set SlotForRecNotFiltered.
    //! \param  [in, out] slotForRecNotFiltered
    //!         slot for recnotfiltered surface
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSlotForRecNotFiltered(unsigned char &slotForRecNotFiltered);

    //!
    //! \brief  Get picture index
    //! \param  [in] idx
    //!         Index of pic index
    //! \return  CODEC_PIC_ID
    //!         CODEC_PIC_ID refer to the picure index
    //!
    CODEC_PIC_ID GetPicIndex(uint8_t idx) { return m_picIdx[idx]; };

    //!
    //! \brief  Get reference list index
    //! \param  [in] idx
    //!         Index of pic index
    //! \return  uint8_t
    //!         reference list index
    //!
    uint8_t GetRefListIndex(uint8_t idx) { return m_picIdx[idx].ucPicIdx; };

    //!
    //! \brief  Get frame store index
    //! \param  [in] idx
    //!         Index of reference index mapping
    //!
    uint8_t GetFrameRestoreIndex(uint8_t idx) { return m_refIdxMapping[idx]; };

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
    //! \brief  Get reference index mapping
    //! \return  uint8_t
    //!         reference index mapping
    //!
    int8_t *GetRefIdxMapping() { return m_refIdxMapping; };
    const int8_t *GetRefIdxMapping() const { return m_refIdxMapping; };

    //!
    //! \brief  Get picture coding type
    //! \return  uint16_t
    //!         Picture coding type
    //!
    uint16_t GetPictureCodingType() { return m_pictureCodingType; };

    //!
    //! \brief  Is current used as reference
    //! \param  [in] idx
    //!         Index of reference list
    //! \return  bool
    //!         true if used as reference, else false
    //!
    bool IsCurrentUsedAsRef(uint8_t idx);
    //!
    //! \brief  Is low delay
    //! \return  bool
    //!         true if low delay,, else false
    //!
    bool IsLowDelay() const { return m_lowDelay; };

    //!
    //! \brief  Is same ref list
    //! \return bool
    //!         true if same ref list, else false
    //!
    bool IsSameRefList() const { return m_sameRefList; };

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(HCP_SURFACE_STATE);

    EncodeMemComp *m_mmcState = nullptr;

    static constexpr uint8_t m_numMaxVdencL0Ref = 3;                                //!< Max number of reference frame list0
    static constexpr uint8_t m_numMaxVdencL1Ref = 3;                                //!< Max number of reference frame list1

protected:

    //!
    //! \brief  Validate low delay B frame
    //! \param  [in] slcParams
    //!         Pointer to CODEC_HEVC_ENCODE_SLICE_PARAMS
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateLowDelayBFrame(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);
    
    //!
    //! \brief  Validate same reference in L0 and L1
    //! \param  [in] slcParams
    //!         Pointer to CODEC_HEVC_ENCODE_SLICE_PARAMS
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!    
    MOS_STATUS ValidateSameRefInL0L1(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);

    //!
    //! \brief  Validate temporal mvp enabling flag
    //! \param  [in] slcParams
    //!         Pointer to CODEC_HEVC_ENCODE_SLICE_PARAMS
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //! 
    MOS_STATUS ValidateTmvp(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);

    ////!
    ////! \brief  Validate radndom access
    ////! \param  [in] slcParams
    ////!         Pointer to CODEC_HEVC_ENCODE_SLICE_PARAMS
    ////! \return  MOS_STATUS
    ////!         MOS_STATUS_SUCCESS if success, else fail reason
    ////!
    //MOS_STATUS ValidateRandomAccess(PCODEC_HEVC_ENCODE_SLICE_PARAMS slcParams);

    uint16_t                m_pictureCodingType = 0;                                //!< I, P, or B frame
    uint8_t                 m_idxForTempMVP = 0;                                    //!< Temp MVP index;
    CODEC_PIC_ID            m_picIdx[CODEC_MAX_NUM_REF_FRAME_HEVC] = {};            //!< Reference picture index array
    PCODEC_REF_LIST         m_refList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC] = {}; //!< Pointer to reference pictures
    PCODEC_REF_LIST         m_currRefList = nullptr;                                //!< Current reference list
    uint8_t                 m_currRefIdx = 0;                                       //!< Current reference list Index
    int8_t                  m_refIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC] = {};     //!< Reference Index mapping
    bool                    m_currUsedRefPic[CODEC_MAX_NUM_REF_FRAME_HEVC] = {};    //!< Reference picture usage array
    bool                    m_lowDelay = false;                                     //!< Low delay flag
    bool                    m_sameRefList = false;                                  //!< Flag to specify if ref list L0 and L1 are same
    int32_t                 m_currGopIFramePOC = -1;                                //!< record POC for I frame in current GoP for temporal MVP verification

    HevcBasicFeature        *m_basicFeature = nullptr;                              //!<  HEVC paramter
    EncodeAllocator         *m_allocator = nullptr;                                 //!< Encode allocator

MEDIA_CLASS_DEFINE_END(encode__HevcReferenceFrames)
};

}  // namespace encode

#endif  // !__ENCODE_HEVC_REFERENCE_FRAMES_H__
