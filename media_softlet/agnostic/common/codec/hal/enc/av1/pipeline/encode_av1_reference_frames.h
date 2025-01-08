/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     encode_av1_reference_frames.h
//! \brief    Defines reference list related logic for encode av1
//!
#ifndef __ENCODE_AV1_REFERENCE_FRAMES_H__
#define __ENCODE_AV1_REFERENCE_FRAMES_H__

#include "codec_def_encode_av1.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_vdenc_itf.h"
#include "mhw_vdbox_avp_itf.h"
#include "encode_tracked_buffer.h"

namespace encode
{
#define AV1_ENCODE_GET_REF_FALG(i) (0x1 << i)

class Av1BasicFeature;
class Av1VdencPipeline;

class Av1ReferenceFrames : public mhw::vdbox::vdenc::Itf::ParSetting, public mhw::vdbox::avp::Itf::ParSetting
{
public:

    //!
    //! \brief  Av1ReferenceFrames constructor
    //!
    Av1ReferenceFrames() {};

    //!
    //! \brief  Av1ReferenceFrames deconstructor
    //!
    ~Av1ReferenceFrames();

    //!
    //! \brief  Initialize reference frame
    //! \param  [in] params
    //!         Pointer to Av1BasicFeature
    //! \return  MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(Av1BasicFeature *basicFeature);

    //! \brief   Update reference frame
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update();

    MOS_STATUS UpdateRefFrameSize(uint32_t width, uint32_t height);

    //! \brief   Check if low delay mode
    //! \return  bool
    //!          true if it's low delay, false random access mode
    //!
    bool IsLowDelay() const { return m_lowDelay; };

    //! \brief   Check if is p frame
    //! \return  bool
    //!          true if it's p frame, false random access mode
    //!
    bool IsPFrame() const { return m_PFrame; }

    //!
    //! \brief  Get current reference list
    //! \return  PCODEC_REF_LIST
    //!         Pointer of current reference list
    //!
    CODEC_REF_LIST_AV1 *GetCurrRefList() const { return m_currRefList; };

    //!
    //! \brief  Get reference list
    //! \return  PCODEC_REF_LIST *
    //!         Pointer of current reference list
    //!
    PCODEC_REF_LIST_AV1 *GetRefList() { return m_refList; };

    //!
    //! \brief  Get picture idx
    //! \param  [in] idx
    //!         id of RefFrameList 
    //! \return  CODEC_PIC_ID
    //!         CODEC_PIC_ID refer to the FrameIdx
    //!
    CODEC_PIC_ID GetPicId(uint8_t idx) { return m_picIdx[idx]; };

    //!
    //! \brief  Get Prime frame's reference list
    //! \return  PCODEC_REF_LIST
    //!         Pointer of Prime frame's reference list
    //!
    CODEC_REF_LIST_AV1 *GetPrimaryRefList() const { return m_primaryRefList; };

    //!
    //! \brief  Set whether to use postcdef as ENC ref
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetPostCdefAsEncRef(bool flag);

    //!
    //! \brief  Get reference scaling idx
    //! \return std::vector<uint8_t>
    //!         A vector of reference scaling idx
    //!
    std::vector<uint8_t> GetRefScalingIdx() const;

    //!
    //! \brief  Get ENC reference surface
    //! \return std::vector<PMOS_SURFACE>
    //!         A vector of ENC reference surface
    //!
    std::vector<PMOS_SURFACE> GetEncRefSurface() const;

    //!
    //! \brief  Get ENC 4x reference surface
    //! \return std::vector<PMOS_SURFACE>
    //!         A vector of ENC 4x reference surface
    //!
    std::vector<PMOS_SURFACE> GetEnc4xRefSurface() const;

    //!
    //! \brief  Get ENC 8x reference surface
    //! \return std::vector<PMOS_SURFACE>
    //!         A vector of ENC 8x reference surface
    //!
    std::vector<PMOS_SURFACE> GetEnc8xRefSurface() const;

    //!
    //! \brief  Get PAK reference surface
    //! \return std::vector<PMOS_SURFACE>
    //!         A vector of PAK reference surface
    //!
    std::vector<PMOS_SURFACE> GetPakRefSurface() const;

    //!
    //! \brief  Get  get the forward and backward reference surface
    //! \param  [in] refsPicList
    //!         A vector contain forward and backward reference surface
    //!
    void GetFwdBwdRefPicList(CODEC_PICTURE (&refsPicList)[2][15]);

    //! \brief  Get  get current frame display order
    //! \return int32_t
    //!         frame display order
    //!
    int32_t GetFrameDisplayOrder();

    //!
    //! \brief  Get  get the Picture Order Count values of reference pictures 
    //!          corresponding to the entries of RefFrameList[]. 
    //! \param  [in] refsPOCList
    //!         A vector contain reference frame POC
    //!         [in] orderHint
    //!         frame display order
    //!
    void GetRefFramePOC(int32_t (&refsPOCList)[15], int32_t const orderHint);

    bool CheckSegmentForPrimeFrame();
    uint8_t RefFrameL0L1(CODEC_Ref_Frame_Ctrl_AV1 const &ref_frame_ctrl) const;

     //!
    //! \brief  Dump input resources or infomation before submit
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpInput(Av1VdencPipeline *pipeline);

    MHW_SETPAR_DECL_HDR(VDENC_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

    MHW_SETPAR_DECL_HDR(AVP_PIC_STATE);

    MHW_SETPAR_DECL_HDR(AVP_PIPE_BUF_ADDR_STATE);

    MHW_SETPAR_DECL_HDR(AVP_INTER_PRED_STATE);

    MHW_SETPAR_DECL_HDR(AVP_SURFACE_STATE);

    bool m_lowDelay = true;//!< Low delay flag

protected:
    static const uint32_t      m_av1ScalingFactor       = (1 << 14);    //!< AV1 Scaling factor
    //!
    //! \brief   Set up internal reference frame flag and reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupRefFlag();

    //!
    //! \brief   Set up internal reference frame flag and reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupCurrRefPic();

    //!
    //! \brief   Set up internal reference frame flag and reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupRefIdx();

    //!
    //! \brief   Set up internal reference frame flag and reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateLowDelayBFrame();

    //!
    //! \brief   Set up internal reference frame flag and reference picture
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidatePFrame();

    int32_t GetRelativeDist(int32_t a, int32_t b) const;

    //!
    //! \brief  Get FWD and BWD reference number
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetFwdBwdRefNum(uint8_t &fwdRefNum, uint8_t &bwdRefNum) const;

    Av1BasicFeature        *m_basicFeature = nullptr;                           //!< AV1 paramter
    uint8_t                 m_primaryRefFrame = av1PrimaryRefNone;              //!< AV1 primary reference frame
    PCODEC_REF_LIST_AV1     m_refList[CODEC_AV1_NUM_UNCOMPRESSED_SURFACE] = {}; //!< Pointer to all reference pictures
    PCODEC_REF_LIST_AV1     m_currRefList = nullptr;                            //!< Current reference list
    PCODEC_REF_LIST_AV1     m_primaryRefList = nullptr;                            //!< prime frame's reference list

    // m_refFrameFlags & 0x01 != 0: Last ref frames used as reference
    // m_refFrameFlags & 0x02 != 0: Last2 ref frames used as reference
    // m_refFrameFlags & 0x04 != 0: Last3 ref frames used as reference
    // m_refFrameFlags & 0x08 != 0: Golden ref frames used as reference
    // m_refFrameFlags & 0x10 != 0: Bwd ref frames used as reference
    // m_refFrameFlags & 0x20 != 0: Alt ref frames used as reference
    // m_refFrameFlags & 0x40 != 0: Alt2 ref frames used as reference
    uint8_t                 m_refFrameFlags = 0;                     //!< m_refFrameFlags is to indicate which frames to be used as reference
    uint8_t                 m_numRefFrames  = 0;                     //!< number of current used reference surfaces
    PMOS_SURFACE            m_currRefPic[av1NumInterRefFrames] = {}; //!< pointer to the current used reference surfaces
    PMOS_SURFACE            m_firstValidRefPic = nullptr;            //!< pointer to the first valid reference surfaces
    MOS_MEMCOMP_STATE       m_refMmcState[av1TotalRefsPerFrame] = { MOS_MEMCOMP_DISABLED };
    uint32_t                m_refCompressionFormat = 0;
    CODEC_PIC_ID            m_picIdx[CODEC_AV1_NUM_REF_FRAMES] = {}; //!< keep a map of frame index
    bool                    m_PFrame   = true;                       //!< P frame flag
    bool                    m_enable_order_hint = false;
    uint8_t                 m_orderHintBitsMinus1 = 0;
    uint8_t                 m_orderHintCount[ENCODE_AV1_ORDER_HINT_SIZE];
    int32_t                 m_frameOut = 0;                        //!<frame output number
    int32_t                 m_prevFrameOffset = 0;
    int32_t                 m_prevFrameDisplayerOrder = 0;

    bool       m_encUsePostCdefAsRef = false;
    BufferType m_encRefBufType       = BufferType::postCdefReconSurface;
    BufferType m_enc4xRefBufType     = BufferType::ds4xSurface;
    BufferType m_enc8xRefBufType     = BufferType::ds8xSurface;
    uint32_t   m_refWidth            = 0;
    uint32_t   m_refHeight           = 0;

    union
    {
        struct
        {
            uint8_t LAST_FRAME    : 1;
            uint8_t LAST2_FRAME   : 1;
            uint8_t LAST3_FRAME   : 1;
            uint8_t GOLDEN_FRAME  : 1;
            uint8_t BWDREF_FRAME  : 1;
            uint8_t ALTREF_FRAME  : 1;
            uint8_t ALTREF2_FRAME : 1;
            uint8_t Reserved1     : 1;
        } fields;
        uint8_t value;
    } m_refFrameBiasFlagsForPak, m_refFrameBiasFlagsForRefManagement;

MEDIA_CLASS_DEFINE_END(encode__Av1ReferenceFrames)
};

}  // namespace encode

#endif  // !__ENCODE_AV1_REFERENCE_FRAMES_H__
